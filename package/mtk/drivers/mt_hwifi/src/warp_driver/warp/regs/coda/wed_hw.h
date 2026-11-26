#ifndef __WED_HW_
#define __WED_HW_


#ifndef REG_FLD
#define REG_FLD(width, shift) (shift)
#endif /*REG_FLD*/

#define WED0_REG_BASE 0

#define WED_ID                                       (WED0_REG_BASE + 0x00000000) // 0000
#define WED_REV_ID                                   (WED0_REG_BASE + 0x00000004) // 0004
#define WED_MOD_RST                                  (WED0_REG_BASE + 0x00000008) // 0008
#define WED_CTRL                                     (WED0_REG_BASE + 0x0000000C) // 000C
#define WED_AXI_CTRL                                 (WED0_REG_BASE + 0x00000010) // 0010
#define WED_CTRL2                                    (WED0_REG_BASE + 0x0000001C) // 001C
#define WED_EX_INT_STA                               (WED0_REG_BASE + 0x00000020) // 0020
#define WED_EX_INT_MSK                               (WED0_REG_BASE + 0x00000028) // 0028
#define WED_EX_INT_MSK1                              (WED0_REG_BASE + 0x0000002c) // 002C
#define WED_EX_INT_MSK2                              (WED0_REG_BASE + 0x00000030) // 0030
#define WED_EX_INT_MSK3                              (WED0_REG_BASE + 0x00000034) // 0034
#define WED_EX_INT_STA_POST_MSK0                     (WED0_REG_BASE + 0x00000040) // 0040
#define WED_EX_INT_STA_POST_MSK1                     (WED0_REG_BASE + 0x00000044) // 0044
#define WED_EX_INT_STA_POST_MSK2                     (WED0_REG_BASE + 0x00000048) // 0048
#define WED_EX_INT_STA_POST_MSK3                     (WED0_REG_BASE + 0x0000004C) // 004C
#define WED_IRQ_MON                                  (WED0_REG_BASE + 0x00000050) // 0050
#define WED_ST                                       (WED0_REG_BASE + 0x00000060) // 0060
#define WED_WPDMA_ST                                 (WED0_REG_BASE + 0x00000064) // 0064
#define WED_WDMA_ST                                  (WED0_REG_BASE + 0x00000068) // 0068
#define WED_BM_ST                                    (WED0_REG_BASE + 0x0000006c) // 006C
#define WED_QM_ST                                    (WED0_REG_BASE + 0x00000070) // 0070
#define WED_WPDMA_D_ST                               (WED0_REG_BASE + 0x00000074) // 0074
#define WED_TX_BM_CTRL                               (WED0_REG_BASE + 0x00000080) // 0080
#define WED_TX_BM_BASE                               (WED0_REG_BASE + 0x00000084) // 0084
#define WED_TX_BM_BLEN                               (WED0_REG_BASE + 0x0000008C) // 008C
#define WED_TX_BM_STS                                (WED0_REG_BASE + 0x00000090) // 0090
#define WED_TX_BM_INTF                               (WED0_REG_BASE + 0x0000009C) // 009C
#define WED_TX_BM_DYN_TH                             (WED0_REG_BASE + 0x000000A0) // 00A0
#define WED_TX_BM_RECYC                              (WED0_REG_BASE + 0x000000A8) // 00A8
#define WED_TX_BM_MON_CTRL                           (WED0_REG_BASE + 0x000000AC) // 00AC
#define WED_TX_BM_VB_FREE_0_31                       (WED0_REG_BASE + 0x000000B0) // 00B0
#define WED_TX_BM_VB_FREE_32_63                      (WED0_REG_BASE + 0x000000B4) // 00B4
#define WED_TX_BM_VB_USED_0_31                       (WED0_REG_BASE + 0x000000B8) // 00B8
#define WED_TX_BM_VB_USED_32_63                      (WED0_REG_BASE + 0x000000BC) // 00BC
#define WED_TX_TKID_CTRL                             (WED0_REG_BASE + 0x000000C0) // 00C0
#define WED_TX_TKID_BASE                             (WED0_REG_BASE + 0x000000C4) // 00C4
#define WED_TX_TKID_TKID                             (WED0_REG_BASE + 0x000000C8) // 00C8
#define WED_TX_TKID_INTF                             (WED0_REG_BASE + 0x000000DC) // 00DC
#define WED_TX_TKID_DYN_TH                           (WED0_REG_BASE + 0x000000E0) // 00E0
#define WED_TX_TKID_RECYC                            (WED0_REG_BASE + 0x000000E8) // 00E8
#define WED_TX_TKID_ALI                              (WED0_REG_BASE + 0x000000EC) // 00EC
#define WED_TX_TKID_VB_FREE_0_31                     (WED0_REG_BASE + 0x000000F0) // 00F0
#define WED_TX_TKID_VB_FREE_32_63                    (WED0_REG_BASE + 0x000000F4) // 00F4
#define WED_TX_TKID_VB_USED_0_31                     (WED0_REG_BASE + 0x000000F8) // 00F8
#define WED_TX_TKID_VB_USED_32_63                    (WED0_REG_BASE + 0x000000FC) // 00FC
#define WED_TXD_DW0                                  (WED0_REG_BASE + 0x00000100) // 0100
#define WED_TXD_DW1                                  (WED0_REG_BASE + 0x00000104) // 0104
#define WED_TXD_DW2                                  (WED0_REG_BASE + 0x00000108) // 0108
#define WED_TXD_DW3                                  (WED0_REG_BASE + 0x0000010C) // 010C
#define WED_TXD_DW4                                  (WED0_REG_BASE + 0x00000110) // 0110
#define WED_TXD_DW5                                  (WED0_REG_BASE + 0x00000114) // 0114
#define WED_TXD_DW6                                  (WED0_REG_BASE + 0x00000118) // 0118
#define WED_TXD_DW7                                  (WED0_REG_BASE + 0x0000011C) // 011C
#define WED_TXP_DW0                                  (WED0_REG_BASE + 0x00000120) // 0120
#define WED_TXP_DW1                                  (WED0_REG_BASE + 0x00000124) // 0124
#define WED_TXDP_CTRL                                (WED0_REG_BASE + 0x00000130) // 0130
#define WED_DBG_CTRL                                 (WED0_REG_BASE + 0x00000180) // 0180
#define WED_DBG_BUS_ADDR_L                           (WED0_REG_BASE + 0x00000184) // 0184
#define WED_DBG_BUS_ADDR_H                           (WED0_REG_BASE + 0x00000188) // 0188
#define WED_DBG_BUS_MON                              (WED0_REG_BASE + 0x0000018C) // 018C
#define WED_DBG_PRB0                                 (WED0_REG_BASE + 0x00000190) // 0190
#define WED_DBG_PRB1                                 (WED0_REG_BASE + 0x00000194) // 0194
#define WED_DBG_PRB2                                 (WED0_REG_BASE + 0x00000198) // 0198
#define WED_DBG_PRB3                                 (WED0_REG_BASE + 0x0000019c) // 019C
#define WED_TX_FREE_TO_TX_TKID_TKID_MIB              (WED0_REG_BASE + 0x000001c0) // 01C0
#define WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB           (WED0_REG_BASE + 0x000001c4) // 01C4
#define WED_RX_BM_TO_WPDMA_RX_D_DRV_TKID_MIB         (WED0_REG_BASE + 0x000001cc) // 01CC
#define WED_TX_TKID_TO_TX_BM_FREE_SKBID_MIB          (WED0_REG_BASE + 0x000001D0) // 01D0
#define WED_PMTR_CTRL                                (WED0_REG_BASE + 0x000001DC) // 01DC
#define WED_PMTR_TGT                                 (WED0_REG_BASE + 0X000001E0) // 01E0
#define WED_PMTR_TGT_ST                              (WED0_REG_BASE + 0X000001E4) // 01E4
#define WED_PMTR_LTCY_MAX0_1                         (WED0_REG_BASE + 0X000001E8) // 01E8
#define WED_PMTR_LTCY_MAX2_3                         (WED0_REG_BASE + 0X000001EC) // 01EC
#define WED_PMTR_LTCY_ACC0                           (WED0_REG_BASE + 0X000001F0) // 01F0
#define WED_PMTR_LTCY_ACC1                           (WED0_REG_BASE + 0X000001F4) // 01F4
#define WED_PMTR_LTCY_ACC2                           (WED0_REG_BASE + 0X000001F8) // 01F8
#define WED_PMTR_LTCY_ACC3                           (WED0_REG_BASE + 0X000001FC) // 01FC
#define WED_INT_STA                                  (WED0_REG_BASE + 0x00000200) // 0200
#define WED_INT_MSK                                  (WED0_REG_BASE + 0x00000204) // 0204
#define WED_GLO_CFG                                  (WED0_REG_BASE + 0x00000208) // 0208
#define WED_RST_IDX                                  (WED0_REG_BASE + 0x0000020C) // 020C
#define WED_DLY_INT_CFG                              (WED0_REG_BASE + 0x00000210) // 0210
#define WED_DLY_INT_CFG1                             (WED0_REG_BASE + 0x00000214) // 0214
#define WED_SPR                                      (WED0_REG_BASE + 0x0000021C) // 021C
#define WED_INT_MSK1                                 (WED0_REG_BASE + 0x00000224) // 0224
#define WED_INT_MSK2                                 (WED0_REG_BASE + 0x00000228) // 0228
#define WED_INT_MSK3                                 (WED0_REG_BASE + 0x0000022C) // 022C
#define WED_INT_STA_POST_MSK0                        (WED0_REG_BASE + 0x00000230) // 0230
#define WED_INT_STA_POST_MSK1                        (WED0_REG_BASE + 0x00000234) // 0234
#define WED_INT_STA_POST_MSK2                        (WED0_REG_BASE + 0x00000238) // 0238
#define WED_INT_STA_POST_MSK3                        (WED0_REG_BASE + 0x0000023C) // 023C
#define WED_SPR_0X24C                                (WED0_REG_BASE + 0x0000024C) // 024C
#define WED_SPR_0X25C                                (WED0_REG_BASE + 0x0000025C) // 025C
#define WED_TX0_MIB                                  (WED0_REG_BASE + 0x000002a0) // 02A0
#define WED_TX1_MIB                                  (WED0_REG_BASE + 0x000002a4) // 02A4
#define WED_TX_COHERENT_MIB                          (WED0_REG_BASE + 0x000002d0) // 02D0
#define WED_RX0_MIB                                  (WED0_REG_BASE + 0x000002e0) // 02E0
#define WED_RX1_MIB                                  (WED0_REG_BASE + 0x000002e4) // 02E4
#define WED_TX0_CTRL0                                (WED0_REG_BASE + 0x00000300) // 0300
#define WED_TX0_CTRL1                                (WED0_REG_BASE + 0x00000304) // 0304
#define WED_TX0_CTRL2                                (WED0_REG_BASE + 0x00000308) // 0308
#define WED_TX0_CTRL3                                (WED0_REG_BASE + 0x0000030c) // 030C
#define WED_TX1_CTRL0                                (WED0_REG_BASE + 0x00000310) // 0310
#define WED_TX1_CTRL1                                (WED0_REG_BASE + 0x00000314) // 0314
#define WED_TX1_CTRL2                                (WED0_REG_BASE + 0x00000318) // 0318
#define WED_TX1_CTRL3                                (WED0_REG_BASE + 0x0000031c) // 031C
#define WED_SCR0                                     (WED0_REG_BASE + 0x000003c0) // 03C0
#define WED_SCR1                                     (WED0_REG_BASE + 0x000003c4) // 03C4
#define WED_SCR2                                     (WED0_REG_BASE + 0x000003c8) // 03C8
#define WED_SCR3                                     (WED0_REG_BASE + 0x000003cc) // 03CC
#define WED_SCR4                                     (WED0_REG_BASE + 0x000003d0) // 03D0
#define WED_SCR5                                     (WED0_REG_BASE + 0x000003d4) // 03D4
#define WED_SCR6                                     (WED0_REG_BASE + 0x000003d8) // 03D8
#define WED_SCR7                                     (WED0_REG_BASE + 0x000003dc) // 03DC
#define WED_RX0_CTRL0                                (WED0_REG_BASE + 0x00000400) // 0400
#define WED_RX0_CTRL1                                (WED0_REG_BASE + 0x00000404) // 0404
#define WED_RX0_CTRL2                                (WED0_REG_BASE + 0x00000408) // 0408
#define WED_RX0_CTRL3                                (WED0_REG_BASE + 0x0000040c) // 040C
#define WED_RX1_CTRL0                                (WED0_REG_BASE + 0x00000410) // 0410
#define WED_RX1_CTRL1                                (WED0_REG_BASE + 0x00000414) // 0414
#define WED_RX1_CTRL2                                (WED0_REG_BASE + 0x00000418) // 0418
#define WED_RX1_CTRL3                                (WED0_REG_BASE + 0x0000041c) // 041C
#define WED_RX_BASE_PTR_0                            (WED0_REG_BASE + 0x00000420) // 0420
#define WED_RX_MAX_CNT_0                             (WED0_REG_BASE + 0x00000424) // 0424
#define WED_RX_CRX_IDX_0                             (WED0_REG_BASE + 0x00000428) // 0428
#define WED_RX_DRX_IDX_0                             (WED0_REG_BASE + 0x0000042c) // 042C
#define WED_RX_BASE_PTR_1                            (WED0_REG_BASE + 0x00000430) // 0430
#define WED_RX_MAX_CNT_1                             (WED0_REG_BASE + 0x00000434) // 0434
#define WED_RX_CRX_IDX_1                             (WED0_REG_BASE + 0x00000438) // 0438
#define WED_RX_DRX_IDX_1                             (WED0_REG_BASE + 0x0000043c) // 043C
#define WED_WPDMA_INT_STA_REC                        (WED0_REG_BASE + 0x00000500) // 0500
#define WED_WPDMA_INT_TRIG                           (WED0_REG_BASE + 0x00000504) // 0504
#define WED_WPDMA_GLO_CFG                            (WED0_REG_BASE + 0x00000508) // 0508
#define WED_WPDMA_RST_IDX                            (WED0_REG_BASE + 0x0000050C) // 050C
#define WED_WPDMA_CTRL                               (WED0_REG_BASE + 0x00000518) // 0518
#define WED_WPDMA_FORCE_PROC                         (WED0_REG_BASE + 0x0000051C) // 051C
#define WED_WPDMA_INT_CTRL                           (WED0_REG_BASE + 0x00000520) // 0520
#define WED_WPDMA_INT_MSK                            (WED0_REG_BASE + 0x00000524) // 0524
#define WED_WPDMA_INT_CLR                            (WED0_REG_BASE + 0x00000528) // 0528
#define WED_WPDMA_INT_MON                            (WED0_REG_BASE + 0x0000052C) // 052C
#define WED_WPDMA_INT_CTRL_TX                        (WED0_REG_BASE + 0x00000530) // 0530
#define WED_WPDMA_INT_CTRL_RX                        (WED0_REG_BASE + 0x00000534) // 0534
#define WED_WPDMA_INT_CTRL_TX_FREE                   (WED0_REG_BASE + 0x00000538) // 0538
#define WED_PCIE_MSIS_TRIG                           (WED0_REG_BASE + 0x00000540) // 0540
#define WED_PCIE_MSIS_REC                            (WED0_REG_BASE + 0x00000544) // 0544
#define WED_PCIE_MSIS_CLR                            (WED0_REG_BASE + 0x00000548) // 0548
#define WED_WPDMA_SPR                                (WED0_REG_BASE + 0x0000054C) // 054C
#define WED_PCIE_INTS_CLR                            (WED0_REG_BASE + 0x00000550) // 0550
#define WED_PCIE_EP_INTS_CLR                         (WED0_REG_BASE + 0x00000554) // 0554
#define WED_PCIE_CFG_ADDR_INTS                       (WED0_REG_BASE + 0x00000560) // 0560
#define WED_PCIE_CFG_ADDR_INTM                       (WED0_REG_BASE + 0x00000564) // 0564
#define WED_PCIE_CFG_ADDR_MSIS                       (WED0_REG_BASE + 0x00000568) // 0568
#define WED_PCIE_CFG_ADDR_INTS_EP                    (WED0_REG_BASE + 0x0000056C) // 056C
#define WED_PCIE_INTS_TRIG                           (WED0_REG_BASE + 0x00000570) // 0570
#define WED_PCIE_INTS_REC                            (WED0_REG_BASE + 0x00000574) // 0574
#define WED_PCIE_INTM_REC                            (WED0_REG_BASE + 0x00000578) // 0578
#define WED_PCIE_INT_CTRL                            (WED0_REG_BASE + 0x0000057C) // 057C
#define WED_WPDMA_CFG_ADDR_INTS                      (WED0_REG_BASE + 0x00000580) // 0580
#define WED_WPDMA_CFG_ADDR_INTM                      (WED0_REG_BASE + 0x00000584) // 0584
#define WED_WPDMA_CFG_ADDR_TX                        (WED0_REG_BASE + 0x00000588) // 0588
#define WED_WPDMA_CFG_ADDR_TX_FREE                   (WED0_REG_BASE + 0x0000058C) // 058C
#define WED_WPDMA_CFG_ADDR_RX                        (WED0_REG_BASE + 0x00000590) // 0590
#define WED_WPDMA_TX0_MIB                            (WED0_REG_BASE + 0x000005a0) // 05A0
#define WED_WPDMA_TX1_MIB                            (WED0_REG_BASE + 0x000005a4) // 05A4
#define WED_WPDMA_TX_COHERENT_MIB                    (WED0_REG_BASE + 0x000005D0) // 05D0
#define WED_WPDMA_RX0_MIB                            (WED0_REG_BASE + 0x000005e0) // 05E0
#define WED_WPDMA_RX1_MIB                            (WED0_REG_BASE + 0x000005e4) // 05E4
#define WED_WPDMA_RX_COHERENT_MIB                    (WED0_REG_BASE + 0x000005F0) // 05F0
#define WED_WPDMA_RX_EXTC_FREE_TKID_MIB              (WED0_REG_BASE + 0x000005F8) // 05F8
#define WED_WPDMA_TX0_CTRL0                          (WED0_REG_BASE + 0x00000600) // 0600
#define WED_WPDMA_TX0_CTRL1                          (WED0_REG_BASE + 0x00000604) // 0604
#define WED_WPDMA_TX0_CTRL2                          (WED0_REG_BASE + 0x00000608) // 0608
#define WED_WPDMA_TX0_CTRL3                          (WED0_REG_BASE + 0x0000060c) // 060C
#define WED_WPDMA_TX1_CTRL0                          (WED0_REG_BASE + 0x00000610) // 0610
#define WED_WPDMA_TX1_CTRL1                          (WED0_REG_BASE + 0x00000614) // 0614
#define WED_WPDMA_TX1_CTRL2                          (WED0_REG_BASE + 0x00000618) // 0618
#define WED_WPDMA_TX1_CTRL3                          (WED0_REG_BASE + 0x0000061c) // 061C
#define WED_WPDMA_RX0_CTRL0                          (WED0_REG_BASE + 0x00000700) // 0700
#define WED_WPDMA_RX0_CTRL1                          (WED0_REG_BASE + 0x00000704) // 0704
#define WED_WPDMA_RX0_CTRL2                          (WED0_REG_BASE + 0x00000708) // 0708
#define WED_WPDMA_RX0_CTRL3                          (WED0_REG_BASE + 0x0000070c) // 070C
#define WED_WPDMA_RX1_CTRL0                          (WED0_REG_BASE + 0x00000710) // 0710
#define WED_WPDMA_RX1_CTRL1                          (WED0_REG_BASE + 0x00000714) // 0714
#define WED_WPDMA_RX1_CTRL2                          (WED0_REG_BASE + 0x00000718) // 0718
#define WED_WPDMA_RX1_CTRL3                          (WED0_REG_BASE + 0x0000071c) // 071C
#define WED_WPDMA_RX_D_RX0_BASE                      (WED0_REG_BASE + 0x00000730) // 0730
#define WED_WPDMA_RX_D_RX0_CNT                       (WED0_REG_BASE + 0x00000734) // 0734
#define WED_WPDMA_RX_D_RX0_CRX_IDX                   (WED0_REG_BASE + 0x00000738) // 0738
#define WED_WPDMA_RX_D_RX0_DRX_IDX                   (WED0_REG_BASE + 0x0000073C) // 073C
#define WED_WPDMA_RX_D_RX1_BASE                      (WED0_REG_BASE + 0x00000740) // 0740
#define WED_WPDMA_RX_D_RX1_CNT                       (WED0_REG_BASE + 0x00000744) // 0744
#define WED_WPDMA_RX_D_RX1_CRX_IDX                   (WED0_REG_BASE + 0x00000748) // 0748
#define WED_WPDMA_RX_D_RX1_DRX_IDX                   (WED0_REG_BASE + 0x0000074C) // 074C
#define WED_WPDMA_RX_D_RX0_THRES_CFG                 (WED0_REG_BASE + 0x00000750) // 0750
#define WED_WPDMA_RX_D_RX1_THRES_CFG                 (WED0_REG_BASE + 0x00000754) // 0754
#define WED_WPDMA_RX_D_INFO                          (WED0_REG_BASE + 0x00000758) // 0758
#define WED_WPDMA_RX_D_GLO_CFG                       (WED0_REG_BASE + 0x0000075C) // 075C
#define WED_WPDMA_RX_D_RST_IDX                       (WED0_REG_BASE + 0x00000760) // 0760
#define WED_WPDMA_RX_D_LOAD_DRV_IDX                  (WED0_REG_BASE + 0x00000764) // 0764
#define WED_WPDMA_RX_D_LOAD_CRX_IDX                  (WED0_REG_BASE + 0x00000768) // 0768
#define WED_WPDMA_RX_D_GLO_CFG_ADDR                  (WED0_REG_BASE + 0x0000076C) // 076C
#define WED_WPDMA_RX_D_RING_CFG_ADDR                 (WED0_REG_BASE + 0x00000770) // 0770
#define WED_WPDMA_RX_D_RX0_MIB                       (WED0_REG_BASE + 0x00000774) // 0774
#define WED_WPDMA_RX_D_RX1_MIB                       (WED0_REG_BASE + 0x00000778) // 0778
#define WED_WPDMA_RX_D_RX0_RECYCLE_MIB               (WED0_REG_BASE + 0x0000077C) // 077C
#define WED_WPDMA_RX_D_RX1_RECYCLE_MIB               (WED0_REG_BASE + 0x00000780) // 0780
#define WED_WPDMA_RX_D_RX0_PROCESSED_MIB             (WED0_REG_BASE + 0x00000784) // 0784
#define WED_WPDMA_RX_D_RX1_PROCESSED_MIB             (WED0_REG_BASE + 0x00000788) // 0788
#define WED_WPDMA_RX_D_RX_COHERENT_MIB               (WED0_REG_BASE + 0x0000078C) // 078C
#define WED_WPDMA_RX_D_ERR_STS                       (WED0_REG_BASE + 0x00000790) // 0790
#define WED_WPDMA_RX_D_TKID_CFG                      (WED0_REG_BASE + 0x000007A0) // 07A0
#define WED_WPDMA_RX_D_TKID_RX0_BASE                 (WED0_REG_BASE + 0x000007A4) // 07A4
#define WED_WPDMA_RX_D_TKID_RX1_BASE                 (WED0_REG_BASE + 0x000007A8) // 07A8
#define WED_WPDMA_RX_D_BUS_CFG                       (WED0_REG_BASE + 0x000007B0) // 07B0
#define WED_WPDMA_RX_D_PREF_CFG                      (WED0_REG_BASE + 0x000007B4) // 07B4
#define WED_WPDMA_RX_D_PREF_RX0_SIDX                 (WED0_REG_BASE + 0x000007B8) // 07B8
#define WED_WPDMA_RX_D_PREF_RX1_SIDX                 (WED0_REG_BASE + 0x000007BC) // 07BC
#define WED_WPDMA_RX_D_PREF_FIFO_CFG                 (WED0_REG_BASE + 0x000007C0) // 07C0
#define WED_WDMA_TX0_BASE                            (WED0_REG_BASE + 0x00000800) // 0800
#define WED_WDMA_TX0_CNT                             (WED0_REG_BASE + 0x00000804) // 0804
#define WED_WDMA_TX0_CTX_IDX                         (WED0_REG_BASE + 0x00000808) // 0808
#define WED_WDMA_TX0_DTX_IDX                         (WED0_REG_BASE + 0x0000080c) // 080C
#define WED_WDMA_TX0_MIB                             (WED0_REG_BASE + 0x00000810) // 0810
#define WED_WDMA_RX0_BASE                            (WED0_REG_BASE + 0x00000900) // 0900
#define WED_WDMA_RX0_CNT                             (WED0_REG_BASE + 0x00000904) // 0904
#define WED_WDMA_RX0_CRX_IDX                         (WED0_REG_BASE + 0x00000908) // 0908
#define WED_WDMA_RX0_DRX_IDX                         (WED0_REG_BASE + 0x0000090c) // 090C
#define WED_WDMA_RX1_BASE                            (WED0_REG_BASE + 0x00000910) // 0910
#define WED_WDMA_RX1_CNT                             (WED0_REG_BASE + 0x00000914) // 0914
#define WED_WDMA_RX1_CRX_IDX                         (WED0_REG_BASE + 0x00000918) // 0918
#define WED_WDMA_RX1_DRX_IDX                         (WED0_REG_BASE + 0x0000091c) // 091C
#define WED_WDMA_RX0_THRES_CFG                       (WED0_REG_BASE + 0x00000940) // 0940
#define WED_WDMA_RX1_THRES_CFG                       (WED0_REG_BASE + 0x00000944) // 0944
#define WED_WDMA_INFO                                (WED0_REG_BASE + 0x00000a00) // 0A00
#define WED_WDMA_GLO_CFG                             (WED0_REG_BASE + 0x00000a04) // 0A04
#define WED_WDMA_RST_IDX                             (WED0_REG_BASE + 0x00000a08) // 0A08
#define WED_WDMA_LOAD_DRV_IDX                        (WED0_REG_BASE + 0x00000a10) // 0A10
#define WED_WDMA_LOAD_CRX_IDX                        (WED0_REG_BASE + 0x00000a14) // 0A14
#define WED_WDMA_SPR                                 (WED0_REG_BASE + 0x00000a1c) // 0A1C
#define WED_WDMA_INT_STA_REC                         (WED0_REG_BASE + 0x00000a20) // 0A20
#define WED_WDMA_INT_CLR                             (WED0_REG_BASE + 0x00000a24) // 0A24
#define WED_WDMA_INT_TRIG                            (WED0_REG_BASE + 0x00000a28) // 0A28
#define WED_WDMA_INT_CTRL                            (WED0_REG_BASE + 0x00000A2C) // 0A2C
#define WED_WDMA_INT_MON                             (WED0_REG_BASE + 0x00000A3C) // 0A3C
#define WED_WDMA_CFG_BASE                            (WED0_REG_BASE + 0x00000aa0) // 0AA0
#define WED_WDMA_OFST0                               (WED0_REG_BASE + 0x00000aa4) // 0AA4
#define WED_WDMA_OFST1                               (WED0_REG_BASE + 0x00000aa8) // 0AA8
#define WED_WDMA_RX0_MIB                             (WED0_REG_BASE + 0x00000ae0) // 0AE0
#define WED_WDMA_RX1_MIB                             (WED0_REG_BASE + 0x00000ae4) // 0AE4
#define WED_WDMA_RX0_RECYCLE_MIB                     (WED0_REG_BASE + 0x00000ae8) // 0AE8
#define WED_WDMA_RX1_RECYCLE_MIB                     (WED0_REG_BASE + 0x00000aec) // 0AEC
#define WED_WDMA_RX0_PROCESSED_MIB                   (WED0_REG_BASE + 0x00000af0) // 0AF0
#define WED_WDMA_RX1_PROCESSED_MIB                   (WED0_REG_BASE + 0x00000af4) // 0AF4
#define WED_WDMA_RX_COHERENT_MIB                     (WED0_REG_BASE + 0x00000af8) // 0AF8
#define WED_RTQM_GLO_CFG                             (WED0_REG_BASE + 0x00000b00) // 0B00
#define WED_RTQM_HIDX_CFG                            (WED0_REG_BASE + 0x00000b04) // 0B04
#define WED_RTQM_TIDX_CFG                            (WED0_REG_BASE + 0x00000b08) // 0B08
#define WED_RTQM_RIDX_CFG                            (WED0_REG_BASE + 0x00000b0c) // 0B0C
#define WED_RTQM_DIDX_CFG                            (WED0_REG_BASE + 0x00000b10) // 0B10
#define WED_RTQM_WRR_CFG0                            (WED0_REG_BASE + 0x00000b14) // 0B14
#define WED_RTQM_WRR_CFG1                            (WED0_REG_BASE + 0x00000b18) // 0B18
#define WED_RTQM_FIFO_CFG                            (WED0_REG_BASE + 0x00000b1c) // 0B1C
#define WED_RTQM_RD_CFG                              (WED0_REG_BASE + 0x00000b20) // 0B20
#define WED_RTQM_RD_DATA0                            (WED0_REG_BASE + 0x00000b24) // 0B24
#define WED_RTQM_RD_DATA1                            (WED0_REG_BASE + 0x00000b28) // 0B28
#define WED_RTQM_RD_DATA2                            (WED0_REG_BASE + 0x00000b2c) // 0B2C
#define WED_RTQM_RD_DATA3                            (WED0_REG_BASE + 0x00000b30) // 0B30
#define WED_RTQM_RD_DATA4                            (WED0_REG_BASE + 0x00000b34) // 0B34
#define WED_RTQM_DQRGN_CNT                           (WED0_REG_BASE + 0x00000b38) // 0B38
#define WED_RTQM_ERR_STS                             (WED0_REG_BASE + 0x00000b3c) // 0B3C
#define WED_RTQM_DBG_CFG                             (WED0_REG_BASE + 0x00000b40) // 0B40
#define WED_RTQM_DBG_MON0                            (WED0_REG_BASE + 0x00000b44) // 0B44
#define WED_RTQM_DBG_MON1                            (WED0_REG_BASE + 0x00000b48) // 0B48
#define WED_RTQM_DBG_MON2                            (WED0_REG_BASE + 0x00000b4c) // 0B4C
#define WED_RTQM_DBG_MON3                            (WED0_REG_BASE + 0x00000b50) // 0B50
#define WED_RTQM_DBG_PFDBK_SEQ_CFG                   (WED0_REG_BASE + 0x00000b54) // 0B54
#define WED_RTQM_DBG_PFDBK_SEQ_ERR                   (WED0_REG_BASE + 0x00000b58) // 0B58
#define WED_RTQM_DBG_PFDBK_SEQ_OW                    (WED0_REG_BASE + 0x00000b5C) // 0B5C
#define WED_RTQM_FIFO_PPE_CFG                        (WED0_REG_BASE + 0x00000b60) // 0B60
#define WED_RTQM_R2H_MIB_0                           (WED0_REG_BASE + 0x00000b70) // 0B70
#define WED_RTQM_R2H_MIB_1                           (WED0_REG_BASE + 0x00000b74) // 0B74
#define WED_RTQM_R2Q_MIB_0                           (WED0_REG_BASE + 0x00000b78) // 0B78
#define WED_RTQM_R2Q_MIB_1                           (WED0_REG_BASE + 0x00000b7C) // 0B7C
#define WED_RTQM_Q2N_MIB                             (WED0_REG_BASE + 0x00000b80) // 0B80
#define WED_RTQM_Q2H_MIB_0                           (WED0_REG_BASE + 0x00000b84) // 0B84
#define WED_RTQM_Q2H_MIB_1                           (WED0_REG_BASE + 0x00000b88) // 0B88
#define WED_RTQM_Q2B_MIB                             (WED0_REG_BASE + 0x00000b8C) // 0B8C
#define WED_RTQM_PFDBK_MIB                           (WED0_REG_BASE + 0x00000b90) // 0B90
#define WED_RTQM_PFDBK_SER_SEQ_CFG0                  (WED0_REG_BASE + 0x00000bA0) // 0BA0
#define WED_RTQM_PFDBK_SER_SEQ_CFG1                  (WED0_REG_BASE + 0x00000bA4) // 0BA4
#define WED_RTQM_PFDBK_SER_SEQ_CFG2                  (WED0_REG_BASE + 0x00000bA8) // 0BA8
#define WED_RTQM_PFDBK_SER_DFDBK                     (WED0_REG_BASE + 0x00000bAC) // 0BAC
#define WED_RTQM_PFDBK_SER_SEQ_ID0                   (WED0_REG_BASE + 0x00000bB0) // 0BB0
#define WED_RTQM_PFDBK_SER_SEQ_ID1                   (WED0_REG_BASE + 0x00000bB4) // 0BB4
#define WED_RTQM_PFDBK_SER_SEQ_OW0                   (WED0_REG_BASE + 0x00000bB8) // 0BB8
#define WED_RTQM_PFDBK_SER_SEQ_OW1                   (WED0_REG_BASE + 0x00000bBC) // 0BBC
#define WED_RTQM_PFDBK_SER_SEQ_EID                   (WED0_REG_BASE + 0x00000bC0) // 0BC0
#define WED_RTQM_PFDBK_SER_SEQ_MIB                   (WED0_REG_BASE + 0x00000bC4) // 0BC4
#define WED_RROQM_GLO_CFG                            (WED0_REG_BASE + 0x00000c04) // 0C04
#define WED_RROQM_RST_IDX                            (WED0_REG_BASE + 0x00000c08) // 0C08
#define WED_RROQM_DLY_INT                            (WED0_REG_BASE + 0x00000c0c) // 0C0C
#define WED_RROQM_MIOD_CTRL0                         (WED0_REG_BASE + 0x00000c40) // 0C40
#define WED_RROQM_MIOD_CTRL1                         (WED0_REG_BASE + 0x00000c44) // 0C44
#define WED_RROQM_MIOD_CTRL2                         (WED0_REG_BASE + 0x00000c48) // 0C48
#define WED_RROQM_MIOD_CTRL3                         (WED0_REG_BASE + 0x00000c4c) // 0C4C
#define WED_RROQM_FDBK_CTRL0                         (WED0_REG_BASE + 0x00000c50) // 0C50
#define WED_RROQM_FDBK_CTRL1                         (WED0_REG_BASE + 0x00000c54) // 0C54
#define WED_RROQM_FDBK_CTRL2                         (WED0_REG_BASE + 0x00000c58) // 0C58
#define WED_RROQM_FDBK_CTRL3                         (WED0_REG_BASE + 0x00000c5c) // 0C5C
#define WED_RROQ_BASE_L                              (WED0_REG_BASE + 0x00000c80) // 0C80
#define WED_RROQ_BASE_H                              (WED0_REG_BASE + 0x00000c84) // 0C84
#define WED_RROQM_MIOD_CFG                           (WED0_REG_BASE + 0x00000c8c) // 0C8C
#define WED_RROQM_FDBK_REC_CTRL                      (WED0_REG_BASE + 0x00000cac) // 0CAC
#define WED_RROQM_FDBK_REC0                          (WED0_REG_BASE + 0x00000cb0) // 0CB0
#define WED_RROQM_FDBK_REC1                          (WED0_REG_BASE + 0x00000cb4) // 0CB4
#define WED_RROQM_FDBK_REC2                          (WED0_REG_BASE + 0x00000cb8) // 0CB8
#define WED_RROQM_FDBK_REC3                          (WED0_REG_BASE + 0x00000cbc) // 0CBC
#define WED_RROQM_MID_MIB                            (WED0_REG_BASE + 0x00000cc0) // 0CC0
#define WED_RROQM_MOD_MIB                            (WED0_REG_BASE + 0x00000cc4) // 0CC4
#define WED_RROQM_MOD_COH_MIB                        (WED0_REG_BASE + 0x00000cc8) // 0CC8
#define WED_RROQM_FDBK_MIB                           (WED0_REG_BASE + 0x00000cd0) // 0CD0
#define WED_RROQM_FDBK_COH_MIB                       (WED0_REG_BASE + 0x00000cd4) // 0CD4
#define WED_RROQM_FDBK_IND_MIB                       (WED0_REG_BASE + 0x00000ce0) // 0CE0
#define WED_RROQM_FDBK_ENQ_MIB                       (WED0_REG_BASE + 0x00000ce4) // 0CE4
#define WED_RROQM_FDBK_ANC_MIB                       (WED0_REG_BASE + 0x00000ce8) // 0CE8
#define WED_RROQM_FDBK_ANC2H_MIB                     (WED0_REG_BASE + 0x00000cec) // 0CEC
#define WED_RX_BM_RX_DMAD                            (WED0_REG_BASE + 0x00000D80) // 0D80
#define WED_RX_BM_BASE                               (WED0_REG_BASE + 0x00000D84) // 0D84
#define WED_RX_BM_INIT_PTR                           (WED0_REG_BASE + 0x00000D88) // 0D88
#define WED_RX_BM_PTR                                (WED0_REG_BASE + 0x00000D8C) // 0D8C
#define WED_RX_BM_BLEN                               (WED0_REG_BASE + 0x00000D90) // 0D90
#define WED_RX_BM_STS                                (WED0_REG_BASE + 0x00000D94) // 0D94
#define WED_RX_BM_INTF2                              (WED0_REG_BASE + 0x00000D98) // 0D98
#define WED_RX_BM_INTF                               (WED0_REG_BASE + 0x00000D9C) // 0D9C
#define WED_RX_BM_ERR_STS                            (WED0_REG_BASE + 0x00000DA8) // 0DA8
#define WED_RX_BM_OPT_CTRL                           (WED0_REG_BASE + 0x00000DAC) // 0DAC
#define WED_RX_DMA_MIB                               (WED0_REG_BASE + 0x00000DB0) // 0DB0
#define WED_RX_BM_DYN_ALLOC_TH                       (WED0_REG_BASE + 0x00000DB4) // 0DB4
#define WED_RX_BM_DYN_ALLOC_CFG                      (WED0_REG_BASE + 0x00000DB8) // 0DB8
#define WED_RX_BM_ADD_BASE                           (WED0_REG_BASE + 0x00000DBC) // 0DBC
#define WED_RX_BM_ADD_PTR                            (WED0_REG_BASE + 0x00000DC0) // 0DC0
#define WED_RX_BM_TOTAL_DMAD_IDX                     (WED0_REG_BASE + 0x00000DC4) // 0DC4


#define WED_ID_FLD_MODE                                        REG_FLD(32, 0)

#define WED_REV_ID_FLD_MAJOR                                   REG_FLD(4, 28)
#define WED_REV_ID_FLD_MINOR                                   REG_FLD(12, 16)
#define WED_REV_ID_FLD_BRANCH                                  REG_FLD(8, 8)
#define WED_REV_ID_FLD_ECO                                     REG_FLD(8, 0)

#define WED_MOD_RST_FLD_WED                                    REG_FLD(1, 31)
#define WED_MOD_RST_FLD_RX_ROUTE_QM                            REG_FLD(1, 21)
#define WED_MOD_RST_FLD_RX_RRO_QM                              REG_FLD(1, 20)
#define WED_MOD_RST_FLD_WDMA_INT_AGT                           REG_FLD(1, 19)
#define WED_MOD_RST_FLD_WDMA_RX_DRV                            REG_FLD(1, 17)
#define WED_MOD_RST_FLD_WDMA_TX_DRV                            REG_FLD(1, 16)
#define WED_MOD_RST_FLD_WED_RX_DMA                             REG_FLD(1, 13)
#define WED_MOD_RST_FLD_WED_TX_DMA                             REG_FLD(1, 12)
#define WED_MOD_RST_FLD_WPDMA_INT_AGT                          REG_FLD(1, 11)
#define WED_MOD_RST_FLD_WPDMA_RX_D_DRV                         REG_FLD(1, 10)
#define WED_MOD_RST_FLD_WPDMA_RX_DRV                           REG_FLD(1, 9)
#define WED_MOD_RST_FLD_WPDMA_TX_DRV                           REG_FLD(1, 8)
#define WED_MOD_RST_FLD_TX_FREE_AGT                            REG_FLD(1, 4)
#define WED_MOD_RST_FLD_RX_BM                                  REG_FLD(1, 1)
#define WED_MOD_RST_FLD_TX_BM                                  REG_FLD(1, 0)

#define WED_CTRL_FLD_MIB_RD_CLR                                REG_FLD(1, 28)
#define WED_CTRL_FLD_ETH_DMAD_FMT                              REG_FLD(1, 25)
#define WED_CTRL_FLD_FINAL_DIDX_RD                             REG_FLD(1, 24)
#define WED_CTRL_FLD_WED_TX_TKID_BUSY                          REG_FLD(1, 18)
#define WED_CTRL_FLD_WED_RX_ROUTE_QM_BUSY                      REG_FLD(1, 17)
#define WED_CTRL_FLD_WED_RX_ROUTE_QM_EN                        REG_FLD(1, 16)
#define WED_CTRL_FLD_WED_RX_RRO_QM_BUSY                        REG_FLD(1, 15)
#define WED_CTRL_FLD_WED_RX_RRO_QM_EN                          REG_FLD(1, 14)
#define WED_CTRL_FLD_WED_RX_BM_BUSY                            REG_FLD(1, 13)
#define WED_CTRL_FLD_WED_RX_BM_EN                              REG_FLD(1, 12)
#define WED_CTRL_FLD_WED_TX_FREE_AGT_BUSY                      REG_FLD(1, 11)
#define WED_CTRL_FLD_WED_TX_FREE_AGT_EN                        REG_FLD(1, 10)
#define WED_CTRL_FLD_WED_TX_BM_BUSY                            REG_FLD(1, 9)
#define WED_CTRL_FLD_WED_TX_BM_EN                              REG_FLD(1, 8)
#define WED_CTRL_FLD_WDMA_INT_AGT_BUSY                         REG_FLD(1, 3)
#define WED_CTRL_FLD_WDMA_INT_AGT_EN                           REG_FLD(1, 2)
#define WED_CTRL_FLD_WPDMA_INT_AGT_BUSY                        REG_FLD(1, 1)
#define WED_CTRL_FLD_WPDMA_INT_AGT_EN                          REG_FLD(1, 0)

#define WED_AXI_CTRL_FLD_CSR_ERRMID_SET_BIRQ                   REG_FLD(1, 4)
#define WED_AXI_CTRL_FLD_CSR_ERRMID_SET_RIRQ                   REG_FLD(1, 3)
#define WED_AXI_CTRL_FLD_R_BUSY                                REG_FLD(1, 2)
#define WED_AXI_CTRL_FLD_W_BUSY                                REG_FLD(1, 1)
#define WED_AXI_CTRL_FLD_CSR_CG_DISABLE                        REG_FLD(1, 0)

#define WED_CTRL2_FLD_COHERENT_WAIT_MAXIMUM                    REG_FLD(8, 0)

#define WED_EX_INT_STA_FLD_FDBK_LEAVE_FULL                     REG_FLD(1, 31)
#define WED_EX_INT_STA_FLD_MID_TAKE_IDX_ERR                    REG_FLD(1, 30)
#define WED_EX_INT_STA_FLD_RX_ROUTE_QM_ERR                     REG_FLD(1, 29)
#define WED_EX_INT_STA_FLD_BUS_MON                             REG_FLD(1, 28)
#define WED_EX_INT_STA_FLD_MID_RDY                             REG_FLD(1, 27)
#define WED_EX_INT_STA_FLD_WPDMA_RX_D_DRV_ERR                  REG_FLD(1, 26)
#define WED_EX_INT_STA_FLD_RX_DRV_GET_BM_DMAD_SKIP             REG_FLD(1, 25)
#define WED_EX_INT_STA_FLD_RX_DRV_DMAD_RECYCLE                 REG_FLD(1, 24)
#define WED_EX_INT_STA_FLD_TX_DMA_W_RESP_ERR                   REG_FLD(1, 23)
#define WED_EX_INT_STA_FLD_TX_DMA_R_RESP_ERR                   REG_FLD(1, 22)
#define WED_EX_INT_STA_FLD_RX_DRV_BM_DMAD_ERROR                REG_FLD(1, 21)
#define WED_EX_INT_STA_FLD_RX_DRV_BM_DMAD_COHERENT             REG_FLD(1, 20)
#define WED_EX_INT_STA_FLD_RX_DRV_INTI_WDMA_ENABLED            REG_FLD(1, 19)
#define WED_EX_INT_STA_FLD_RX_DRV_COHERENT                     REG_FLD(1, 18)
#define WED_EX_INT_STA_FLD_RX_DRV_W_RESP_ERR                   REG_FLD(1, 17)
#define WED_EX_INT_STA_FLD_RX_DRV_R_RESP_ERR                   REG_FLD(1, 16)
#define WED_EX_INT_STA_FLD_RX_BM_H_BUF					 	   REG_FLD(1, 15)
#define WED_EX_INT_STA_FLD_RX_BM_L_BUF					 	   REG_FLD(1, 14)
#define WED_EX_INT_STA_FLD_RX_BM_FREE_AT_EMPTY                 REG_FLD(1, 12)
#define WED_EX_INT_STA_FLD_RX_BM_DMAD_RD_ERR                   REG_FLD(1, 13)
#define WED_EX_INT_STA_FLD_RX_BM_FREE_AT_EMPTY                 REG_FLD(1, 12)
#define WED_EX_INT_STA_FLD_TX_TKID_HTH                         REG_FLD(1, 11)
#define WED_EX_INT_STA_FLD_TX_TKID_LTH                         REG_FLD(1, 10)
#define WED_EX_INT_STA_FLD_TX_BM_HTH                           REG_FLD(1, 9)
#define WED_EX_INT_STA_FLD_TX_BM_LTH                           REG_FLD(1, 8)
#define WED_EX_INT_STA_FLD_TF_TKID_FIFO_INVLD                  REG_FLD(1, 4)
#define WED_EX_INT_STA_FLD_TF_TKID_WO_PYLD                     REG_FLD(1, 1)
#define WED_EX_INT_STA_FLD_TF_LEN_ERR                          REG_FLD(1, 0)

#define WED_EX_INT_MSK_FLD_EN                                  REG_FLD(32, 0)

#define WED_EX_INT_MSK1_FLD_EN                                 REG_FLD(32, 0)

#define WED_EX_INT_MSK2_FLD_EN                                 REG_FLD(32, 0)

#define WED_EX_INT_MSK3_FLD_EN                                 REG_FLD(32, 0)

#define WED_EX_INT_STA_POST_MSK0_FLD_EVENT                     REG_FLD(32, 0)

#define WED_EX_INT_STA_POST_MSK1_FLD_EVENT                     REG_FLD(32, 0)

#define WED_EX_INT_STA_POST_MSK2_FLD_EVENT                     REG_FLD(32, 0)

#define WED_EX_INT_STA_POST_MSK3_FLD_EVENT                     REG_FLD(32, 0)

#define WED_IRQ_MON_FLD_WED_IRQ                                REG_FLD(3, 28)
#define WED_IRQ_MON_FLD_WDMA1_IRQ                              REG_FLD(1, 9)
#define WED_IRQ_MON_FLD_WDMA0_IRQ                              REG_FLD(1, 8)
#define WED_IRQ_MON_FLD_PCIE1_IRQ                              REG_FLD(1, 1)
#define WED_IRQ_MON_FLD_PCIE0_IRQ                              REG_FLD(1, 0)

#define WED_ST_FLD_TX_ST                                       REG_FLD(8, 8)

#define WED_WPDMA_ST_FLD_INT_AGT_ST                            REG_FLD(8, 16)
#define WED_WPDMA_ST_FLD_TX_DRV_ST                             REG_FLD(8, 8)
#define WED_WPDMA_ST_FLD_RX_DRV_ST                             REG_FLD(8, 0)

#define WED_WDMA_ST_FLD_AXI_W_AFTER_AW_ST                      REG_FLD(2, 24)
#define WED_WDMA_ST_FLD_INT_AGT_ST                             REG_FLD(8, 16)
#define WED_WDMA_ST_FLD_TX_DRV_ST                              REG_FLD(8, 8)
#define WED_WDMA_ST_FLD_RX_DRV_ST                              REG_FLD(8, 0)

#define WED_BM_ST_FLD_FREE_AGT_ST                              REG_FLD(8, 24)
#define WED_BM_ST_FLD_TX_TKID_ST                               REG_FLD(8, 16)
#define WED_BM_ST_FLD_TX_BM_ST                                 REG_FLD(8, 8)
#define WED_BM_ST_FLD_RX_BM_ST                                 REG_FLD(8, 0)

#define WED_QM_ST_FLD_RTQM_ST                                  REG_FLD(9, 8)
#define WED_QM_ST_FLD_RROQM_ST                                 REG_FLD(8, 0)

#define WED_WPDMA_D_ST_FLD_AXI_W_AFTER_AW_ST                   REG_FLD(2, 8)
#define WED_WPDMA_D_ST_FLD_RX_DRV_ST                           REG_FLD(8, 0)

#define WED_TX_BM_CTRL_FLD_SCAN                                REG_FLD(1, 29)
#define WED_TX_BM_CTRL_FLD_PAUSE                               REG_FLD(1, 28)
#define WED_TX_BM_CTRL_FLD_RSV_GRP_NUM                         REG_FLD(9, 16)
#define WED_TX_BM_CTRL_FLD_VLD_GRP_NUM                         REG_FLD(9, 0)
#define WED_TX_BM_CTRL_FLD_VLD_GRP_NUM_MASK						(0x1ff)


#define WED_TX_BM_BASE_FLD_PTR                                 REG_FLD(32, 0)

#define WED_TX_BM_BLEN_FLD_BYTE_LEN                            REG_FLD(14, 0)

#define WED_TX_BM_STS_FLD_DMAD_RD                              REG_FLD(1, 25)
#define WED_TX_BM_STS_FLD_DMAD_VLD                             REG_FLD(1, 24)
#define WED_TX_BM_STS_FLD_DMAD_NUM                             REG_FLD(5, 16)
#define WED_TX_BM_STS_FLD_FREE_NUM                             REG_FLD(16, 0)

#define WED_TX_BM_INTF_FLD_FREE_SKBID_RD                       REG_FLD(1, 29)
#define WED_TX_BM_INTF_FLD_FREE_SKBID_VLD                      REG_FLD(1, 28)
#define WED_TX_BM_INTF_FLD_FREE_SKBID                          REG_FLD(16, 0)

#define WED_TX_BM_DYN_TH_FLD_HI_GRP_NUM                        REG_FLD(9, 16)
#define WED_TX_BM_DYN_TH_FLD_LOW_GRP_NUM                       REG_FLD(9, 0)

#define WED_TX_BM_RECYC_FLD_START_GRP_IDX                      REG_FLD(9, 0)

#define WED_TX_BM_MON_CTRL_FLD_VB_PGSEL                        REG_FLD(2, 0)

#define WED_TX_BM_VB_FREE_0_31_FLD_GRP                         REG_FLD(32, 0)

#define WED_TX_BM_VB_FREE_32_63_FLD_GRP                        REG_FLD(32, 0)

#define WED_TX_BM_VB_USED_0_31_FLD_GRP                         REG_FLD(32, 0)

#define WED_TX_BM_VB_USED_32_63_FLD_GRP                        REG_FLD(32, 0)

#define WED_TX_TKID_CTRL_FLD_SCAN                              REG_FLD(1, 29)
#define WED_TX_TKID_CTRL_FLD_PAUSE                             REG_FLD(1, 28)
#define WED_TX_TKID_CTRL_FLD_RSV_GRP_NUM                       REG_FLD(7, 16)
#define WED_TX_TKID_CTRL_FLD_VLD_GRP_NUM                       REG_FLD(7, 0)

#define WED_TX_TKID_BASE_FLD_PTR                               REG_FLD(32, 0)

#define WED_TX_TKID_TKID_FLD_END_ID                            REG_FLD(16, 16)
#define WED_TX_TKID_TKID_FLD_START_ID                          REG_FLD(16, 0)

#define WED_TX_TKID_INTF_FLD_FREE_TKID_RD                      REG_FLD(1, 29)
#define WED_TX_TKID_INTF_FLD_FREE_TKID_VLD                     REG_FLD(1, 28)
#define WED_TX_TKID_INTF_FLD_FREE_TKFIFO_FDEP                  REG_FLD(7, 16)
#define WED_TX_TKID_INTF_FLD_FREE_TKID                         REG_FLD(16, 0)

#define WED_TX_TKID_DYN_TH_FLD_HI_GRP_NUM                      REG_FLD(7, 16)
#define WED_TX_TKID_DYN_TH_FLD_LOW_GRP_NUM                     REG_FLD(7, 0)

#define WED_TX_TKID_RECYC_FLD_START_GRP_IDX                    REG_FLD(7, 0)

#define WED_TX_TKID_ALI_FIFO_RD_ERR                            REG_FLD(1, 29)
#define WED_TX_TKID_ALI_FIFO_WR_ERR                            REG_FLD(1, 28)
#define WED_TX_TKID_ALI_FIFO_CNT                               REG_FLD(12, 16)
#define WED_TX_TKID_ALI_FIFO_FULL                              REG_FLD(4, 8)
#define WED_TX_TKID_ALI_FIFO_EMPTY                             REG_FLD(4, 4)
#define WED_TX_TKID_ALI_FIFO_RST                               REG_FLD(1, 1)

#define WED_TX_TKID_VB_FREE_0_31_FLD_GRP                       REG_FLD(32, 0)

#define WED_TX_TKID_VB_FREE_32_63_FLD_GRP                      REG_FLD(32, 0)

#define WED_TX_TKID_VB_USED_0_31_FLD_GRP                       REG_FLD(32, 0)

#define WED_TX_TKID_VB_USED_32_63_FLD_GRP                      REG_FLD(32, 0)

#define WED_TXD_DW0_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXD_DW1_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXD_DW2_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXD_DW3_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXD_DW4_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXD_DW5_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXD_DW6_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXD_DW7_FLD_VALUE                                  REG_FLD(32, 0)

#define WED_TXP_DW0_FLD_VALUE                                  REG_FLD(16, 0)

#define WED_TXP_DW1_FLD_VALUE                                  REG_FLD(16, 16)

#define WED_TXDP_CTRL_FLD_DW_OVERWR                            REG_FLD(10, 0)

#define WED_DBG_CTRL_FLD_TX_TKID_BM_MEM_DBG                    REG_FLD(1, 28)
#define WED_DBG_CTRL_FLD_TX_BM_MEM_PGSEL                       REG_FLD(2, 26)
#define WED_DBG_CTRL_FLD_BUS_MON_AR_EN                         REG_FLD(1, 25)
#define WED_DBG_CTRL_FLD_BUS_MON_AW_EN                         REG_FLD(1, 24)
#define WED_DBG_CTRL_FLD_PRB_DW_SEL_TO_CHIP                    REG_FLD(2, 8)
#define WED_DBG_CTRL_FLD_PRB_SEL                               REG_FLD(6, 0)

#define WED_DBG_BUS_MON_FLD_ADDRESS                            REG_FLD(32, 0)

#define WED_DBG_BUS_MON1_FLD_STATUS                            REG_FLD(32, 0)

#define WED_DBG_PRB0_FLD_SIGNAL                                REG_FLD(32, 0)

#define WED_DBG_PRB1_FLD_SIGNAL                                REG_FLD(32, 0)

#define WED_DBG_PRB2_FLD_SIGNAL                                REG_FLD(32, 0)

#define WED_DBG_PRB3_FLD_SIGNAL                                REG_FLD(32, 0)

#define WED_TX_FREE_TO_TX_TKID_TKID_MIB_FLD_CNT                REG_FLD(32, 0)

#define WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB_FLD_CNT             REG_FLD(32, 0)

#define WED_RX_BM_TO_WPDMA_RX_D_DRV_TKID_MIB_FLD_CNT           REG_FLD(32, 0)

#define WED_TX_TKID_TO_TX_BM_FREE_SKBID_MIB_FLD_CNT            REG_FLD(32, 0)

#define WED_PMTR_CTRL_FLD_EN                                   REG_FLD(1, 31)
#define WED_PMTR_CTRL_FLD_ACC_DIV                              REG_FLD(4, 24)
#define WED_PMTR_CTRL_FLD_MAX_DIV                              REG_FLD(4, 16)

#define WED_PMTR_TGT_FLD_MST3                                  REG_FLD(5, 24)
#define WED_PMTR_TGT_FLD_MST2                                  REG_FLD(5, 16)
#define WED_PMTR_TGT_FLD_MST1                                  REG_FLD(5, 8)
#define WED_PMTR_TGT_FLD_MST0                                  REG_FLD(5, 0)

#define WED_PMTR_TGT_ST_FLD_MST3                               REG_FLD(8, 24)
#define WED_PMTR_TGT_ST_FLD_MST2                               REG_FLD(8, 16)
#define WED_PMTR_TGT_ST_FLD_MST1                               REG_FLD(8, 8)
#define WED_PMTR_TGT_ST_FLD_MST0                               REG_FLD(8, 0)

#define WED_PMTR_LTCY_MAX0_1_FLD_MST1_CNT                      REG_FLD(16, 16)
#define WED_PMTR_LTCY_MAX0_1_FLD_MST0_CNT                      REG_FLD(16, 0)

#define WED_PMTR_LTCY_MAX2_3_FLD_MST3_CNT                      REG_FLD(16, 16)
#define WED_PMTR_LTCY_MAX2_3_FLD_MST2_CNT                      REG_FLD(16, 0)

#define WED_PMTR_LTCY_ACC0_FLD_CNT                             REG_FLD(32, 0)

#define WED_PMTR_LTCY_ACC0_FLD_CNT                             REG_FLD(32, 0)

#define WED_PMTR_LTCY_ACC1_FLD_CNT                             REG_FLD(32, 0)

#define WED_PMTR_LTCY_ACC2_FLD_CNT                             REG_FLD(32, 0)

#define WED_PMTR_LTCY_ACC3_FLD_CNT                             REG_FLD(32, 0)

#define WED_INT_STA_FLD_EVENT_31                               REG_FLD(1, 31)
#define WED_INT_STA_FLD_EVENT_30                               REG_FLD(1, 30)
#define WED_INT_STA_FLD_EVENT_29                               REG_FLD(1, 29)
#define WED_INT_STA_FLD_EVENT_28                               REG_FLD(1, 28)
#define WED_INT_STA_FLD_EVENT_27                               REG_FLD(1, 27)
#define WED_INT_STA_FLD_EVENT_26                               REG_FLD(1, 26)
#define WED_INT_STA_FLD_EVENT_25                               REG_FLD(1, 25)
#define WED_INT_STA_FLD_EVENT_24                               REG_FLD(1, 24)
#define WED_INT_STA_FLD_EVENT_23                               REG_FLD(1, 23)
#define WED_INT_STA_FLD_EVENT_22                               REG_FLD(1, 22)
#define WED_INT_STA_FLD_EVENT_21                               REG_FLD(1, 21)
#define WED_INT_STA_FLD_EVENT_20                               REG_FLD(1, 20)
#define WED_INT_STA_FLD_EVENT_19                               REG_FLD(1, 19)
#define WED_INT_STA_FLD_EVENT_18                               REG_FLD(1, 18)
#define WED_INT_STA_FLD_EVENT_17                               REG_FLD(1, 17)
#define WED_INT_STA_FLD_EVENT_16                               REG_FLD(1, 16)
#define WED_INT_STA_FLD_EVENT_15                               REG_FLD(1, 15)
#define WED_INT_STA_FLD_EVENT_14                               REG_FLD(1, 14)
#define WED_INT_STA_FLD_EVENT_13                               REG_FLD(1, 13)
#define WED_INT_STA_FLD_EVENT_12                               REG_FLD(1, 12)
#define WED_INT_STA_FLD_EVENT_11                               REG_FLD(1, 11)
#define WED_INT_STA_FLD_EVENT_10                               REG_FLD(1, 10)
#define WED_INT_STA_FLD_EVENT_9                                REG_FLD(1, 9)
#define WED_INT_STA_FLD_EVENT_8                                REG_FLD(1, 8)
#define WED_INT_STA_FLD_EVENT_7                                REG_FLD(1, 7)
#define WED_INT_STA_FLD_EVENT_6                                REG_FLD(1, 6)
#define WED_INT_STA_FLD_EVENT_5                                REG_FLD(1, 5)
#define WED_INT_STA_FLD_EVENT_4                                REG_FLD(1, 4)
#define WED_INT_STA_FLD_EVENT_3                                REG_FLD(1, 3)
#define WED_INT_STA_FLD_EVENT_2                                REG_FLD(1, 2)
#define WED_INT_STA_FLD_EVENT_1                                REG_FLD(1, 1)
#define WED_INT_STA_FLD_EVENT_0                                REG_FLD(1, 0)

#define WED_INT_MSK_FLD_EN                                     REG_FLD(32, 0)
#define WED_GLO_CFG_FLD_CSR_CLKGATE_BYP                        REG_FLD(1, 30)
#define WED_GLO_CFG_FLD_BYTE_SWAP                              REG_FLD(1, 29)
#define WED_GLO_CFG_FLD_WCOMPLETE_SEL                          REG_FLD(1, 9)
#define WED_GLO_CFG_FLD_DESC_32B_E                             REG_FLD(1, 8)
#define WED_GLO_CFG_FLD_BIG_ENDIAN                             REG_FLD(1, 7)
#define WED_GLO_CFG_FLD_TX_WB_DDONE                            REG_FLD(1, 6)
#define WED_GLO_CFG_FLD_RX_DMA_BUSY                            REG_FLD(1, 3)
#define WED_GLO_CFG_FLD_RX_DMA_EN                              REG_FLD(1, 2)
#define WED_GLO_CFG_FLD_TX_DMA_BUSY                            REG_FLD(1, 1)
#define WED_GLO_CFG_FLD_TX_DMA_EN                              REG_FLD(1, 0)

#define WED_RST_IDX_FLD_WPDMA_DRX_IDX1                         REG_FLD(1, 31)
#define WED_RST_IDX_FLD_WPDMA_DRX_IDX0                         REG_FLD(1, 30)
#define WED_RST_IDX_FLD_DRX_IDX1                               REG_FLD(1, 7)
#define WED_RST_IDX_FLD_DRX_IDX0                               REG_FLD(1, 6)
#define WED_RST_IDX_FLD_DTX_IDX1                               REG_FLD(1, 1)
#define WED_RST_IDX_FLD_DTX_IDX0                               REG_FLD(1, 0)

#define WED_DLY_INT_CFG_FLD_TX_DLY_INT_EN                      REG_FLD(1, 31)
#define WED_DLY_INT_CFG_FLD_TX_MAX_PINT                        REG_FLD(7, 24)
#define WED_DLY_INT_CFG_FLD_TX_MAX_PTIME                       REG_FLD(8, 16)
#define WED_DLY_INT_CFG_FLD_RX_DLY_INT_EN                      REG_FLD(1, 15)
#define WED_DLY_INT_CFG_FLD_RX_MAX_PINT                        REG_FLD(7, 8)
#define WED_DLY_INT_CFG_FLD_RX_MAX_PTIME                       REG_FLD(8, 0)

#define WED_DLY_INT_CFG1_FLD_TX_FREE_DLY_INT_EN                REG_FLD(1, 15)
#define WED_DLY_INT_CFG1_FLD_TX_FREE_MAX_PINT                  REG_FLD(7, 8)
#define WED_DLY_INT_CFG1_FLD_TX_FREE_MAX_PTIME                 REG_FLD(8, 0)
#define WED_SPR_FLD_RSV_REG                                    REG_FLD(32, 0)

#define WED_INT_MSK1_FLD_EN                                    REG_FLD(32, 0)

#define WED_INT_MSK2_FLD_EN                                    REG_FLD(32, 0)

#define WED_INT_MSK3_FLD_EN                                    REG_FLD(32, 0)

#define WED_INT_STA_POST_MSK0_FLD_EVENT                        REG_FLD(32, 0)

#define WED_INT_STA_POST_MSK1_FLD_EVENT                        REG_FLD(32, 0)

#define WED_INT_STA_POST_MSK2_FLD_EVENT                        REG_FLD(32, 0)

#define WED_INT_STA_POST_MSK3_FLD_EVENT                        REG_FLD(32, 0)

#define WED_SPR_0X24C_FLD_RSV_REG                              REG_FLD(32, 0)

#define WED_SPR_0X25C_FLD_RSV_REG                              REG_FLD(32, 0)

#define WED_TX0_MIB_FLD_PKT_CNT                                REG_FLD(32, 0)

#define WED_TX1_MIB_FLD_PKT_CNT                                REG_FLD(32, 0)

#define WED_TX_COHERENT_MIB_FLD_CNT                            REG_FLD(32, 0)

#define WED_RX0_MIB_FLD_PKT_CNT                                REG_FLD(32, 0)

#define WED_RX1_MIB_FLD_PKT_CNT                                REG_FLD(32, 0)

#define WED_TX0_CTRL0_FLD_BASE_PTR                             REG_FLD(32, 0)

#define WED_TX0_CTRL1_FLD_MAX_CNT                              REG_FLD(12, 0)

#define WED_TX0_CTRL2_FLD_CPU_IDX                              REG_FLD(12, 0)

#define WED_TX0_CTRL3_FLD_DMA_IDX                              REG_FLD(12, 0)

#define WED_TX1_CTRL0_FLD_BASE_PTR                             REG_FLD(32, 0)

#define WED_TX1_CTRL1_FLD_MAX_CNT                              REG_FLD(12, 0)

#define WED_TX1_CTRL2_FLD_CPU_IDX                              REG_FLD(12, 0)

#define WED_TX1_CTRL3_FLD_DMA_IDX                              REG_FLD(12, 0)

#define WED_SCR0_FLD_REG                                       REG_FLD(32, 0)

#define WED_SCR1_FLD_REG                                       REG_FLD(32, 0)

#define WED_SCR2_FLD_REG                                       REG_FLD(32, 0)

#define WED_SCR3_FLD_REG                                       REG_FLD(32, 0)

#define WED_SCR4_FLD_REG                                       REG_FLD(32, 0)

#define WED_SCR5_FLD_REG                                       REG_FLD(32, 0)

#define WED_SCR6_FLD_REG                                       REG_FLD(32, 0)

#define WED_SCR7_FLD_REG                                       REG_FLD(32, 0)

#define WED_RX0_CTRL0_FLD_BASE_PTR                             REG_FLD(32, 0)

#define WED_RX0_CTRL1_FLD_MAX_CNT                              REG_FLD(12, 0)

#define WED_RX0_CTRL2_FLD_CPU_IDX                              REG_FLD(12, 0)

#define WED_RX0_CTRL3_FLD_DMA_IDX                              REG_FLD(12, 0)

#define WED_RX1_CTRL0_FLD_BASE_PTR                             REG_FLD(32, 0)

#define WED_RX1_CTRL1_FLD_MAX_CNT                              REG_FLD(12, 0)

#define WED_RX1_CTRL2_FLD_CPU_IDX                              REG_FLD(12, 0)

#define WED_RX1_CTRL3_FLD_DMA_IDX                              REG_FLD(12, 0)

#define WED_RX_BASE_PTR_0_FLD_BASE_PTR                         REG_FLD(32, 0)

#define WED_RX_MAX_CNT_0_FLD_MAX_CNT                           REG_FLD(12, 0)

#define WED_RX_CRX_IDX_0_FLD_CRX_IDX                           REG_FLD(12, 0)

#define WED_RX_DRX_IDX_0_FLD_DRX_IDX                           REG_FLD(12, 0)

#define WED_RX_BASE_PTR_1_FLD_BASE_PTR                         REG_FLD(32, 0)

#define WED_RX_MAX_CNT_1_FLD_MAX_CNT                           REG_FLD(12, 0)

#define WED_RX_CRX_IDX_1_FLD_CRX_IDX                           REG_FLD(12, 0)

#define WED_RX_DRX_IDX_1_FLD_DRX_IDX                           REG_FLD(12, 0)

#define WED_WPDMA_INT_STA_REC_FLD_INT_STA                      REG_FLD(32, 0)

#define WED_WPDMA_INT_TRIG_FLD_INT_TRIG                        REG_FLD(32, 0)

#define WED_WPDMA_GLO_CFG_FLD_RX_2B_OFFSET                     REG_FLD(1, 31)
#define WED_WPDMA_GLO_CFG_FLD_TX_DDONE_CHK                     REG_FLD(1, 30)
#define WED_WPDMA_GLO_CFG_FLD_BYTE_SWAP                        REG_FLD(1, 29)
#define WED_WPDMA_GLO_CFG_FLD_TX_DMAD_DW3_PREV                 REG_FLD(1, 28)
#define WED_WPDMA_GLO_CFG_FLD_RSV2                             REG_FLD(3, 25)
#define WED_WPDMA_GLO_CFG_FLD_TX_TKID_KEEP                     REG_FLD(1, 24)
#define WED_WPDMA_GLO_CFG_FLD_RSV1                             REG_FLD(2, 22)
#define WED_WPDMA_GLO_CFG_FLD_RX_DDONE2_WR                     REG_FLD(1, 21)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_EVENT_PKT_FMT_CHK         REG_FLD(1, 20)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_UNSUPPORT_FMT_BYP         REG_FLD(1, 19)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_EVENT_PKT_FMT_VER         REG_FLD(3, 16)
#define WED_WPDMA_GLO_CFG_FLD_RSV0                             REG_FLD(8, 8)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_RING1_CRX_SYNC            REG_FLD(1, 7)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_RING0_CRX_SYNC            REG_FLD(1, 6)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_RING1_PKT_PROC            REG_FLD(1, 5)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_RING0_PKT_PROC            REG_FLD(1, 4)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_BUSY                      REG_FLD(1, 3)
#define WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN                        REG_FLD(1, 2)
#define WED_WPDMA_GLO_CFG_FLD_TX_DRV_BUSY                      REG_FLD(1, 1)
#define WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN                        REG_FLD(1, 0)

#define WED_WPDMA_RST_IDX_FLD_CRX_IDX1                         REG_FLD(1, 17)
#define WED_WPDMA_RST_IDX_FLD_CRX_IDX0                         REG_FLD(1, 16)
#define WED_WPDMA_RST_IDX_FLD_CTX_IDX1                         REG_FLD(1, 1)
#define WED_WPDMA_RST_IDX_FLD_CTX_IDX0                         REG_FLD(1, 0)

#define WED_WPDMA_CTRL_FLD_SDL1_FIXED                          REG_FLD(1, 31)
#define WED_WPDMA_CTRL_FLD_SDL1_MAX                            REG_FLD(14, 0)

#define WED_WPDMA_INT_CTRL_FLD_MSK_EN                          REG_FLD(1, 28)
#define WED_WPDMA_INT_CTRL_FLD_POLL_MODE                       REG_FLD(2, 24)
#define WED_WPDMA_INT_CTRL_FLD_MSK_ON_DLY                      REG_FLD(1, 23)
#define WED_WPDMA_INT_CTRL_FLD_SIG_SRC                         REG_FLD(1, 22)
#define WED_WPDMA_INT_CTRL_FLD_SUBRT_ADV                       REG_FLD(1, 21)
#define WED_WPDMA_INT_CTRL_FLD_MSK_EN_POLA                     REG_FLD(1, 20)
#define WED_WPDMA_INT_CTRL_FLD_SRC_SEL                         REG_FLD(2, 16)
#define WED_WPDMA_INT_CTRL_FLD_POLL_PRD                        REG_FLD(8, 8)
#define WED_WPDMA_INT_CTRL_FLD_MSK_ON_DLY_PRD                  REG_FLD(8, 0)

#define WED_WPDMA_INT_MSK_FLD_VALUE                            REG_FLD(32, 0)

#define WED_WPDMA_INT_CLR_FLD_INT_CLR                          REG_FLD(32, 0)

#define WED_WPDMA_INT_MON_FLD_TX_FREE_DONE_INT1                REG_FLD(1, 5)
#define WED_WPDMA_INT_MON_FLD_TX_FREE_DONE_INT0                REG_FLD(1, 4)
#define WED_WPDMA_INT_MON_FLD_RX_DONE_INT1                     REG_FLD(1, 3)
#define WED_WPDMA_INT_MON_FLD_RX_DONE_INT0                     REG_FLD(1, 2)
#define WED_WPDMA_INT_MON_FLD_TX_DONE_INT1                     REG_FLD(1, 1)
#define WED_WPDMA_INT_MON_FLD_TX_DONE_INT0                     REG_FLD(1, 0)

#define WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_TRIG1                REG_FLD(5, 10)
#define WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_CLR1                 REG_FLD(1, 9)
#define WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_EN1                  REG_FLD(1, 8)
#define WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_TRIG0                REG_FLD(5, 2)
#define WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_CLR0                 REG_FLD(1, 1)
#define WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_EN0                  REG_FLD(1, 0)

#define WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_TRIG1                REG_FLD(5, 10)
#define WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_CLR1                 REG_FLD(1, 9)
#define WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_EN1                  REG_FLD(1, 8)
#define WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_TRIG0                REG_FLD(5, 2)
#define WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_CLR0                 REG_FLD(1, 1)
#define WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_EN0                  REG_FLD(1, 0)

#define WED_WPDMA_INT_CTRL_TX_FREE_FLD_TX_FREE_DONE_TRIG1      REG_FLD(5, 10)
#define WED_WPDMA_INT_CTRL_TX_FREE_FLD_TX_FREE_DONE_CLR1       REG_FLD(1, 9)
#define WED_WPDMA_INT_CTRL_TX_FREE_FLD_TX_FREE_DONE_EN1        REG_FLD(1, 8)
#define WED_WPDMA_INT_CTRL_TX_FREE_FLD_TX_FREE_DONE_TRIG0      REG_FLD(5, 2)
#define WED_WPDMA_INT_CTRL_TX_FREE_FLD_TX_FREE_DONE_CLR0       REG_FLD(1, 1)
#define WED_WPDMA_INT_CTRL_TX_FREE_FLD_TX_FREE_DONE_EN0        REG_FLD(1, 0)

#define WED_WPDMA_SPR_FLD_RSV_REG                              REG_FLD(32, 0)

#define WED_PCIE_CFG_ADDR_INTS_FLD_PTR                         REG_FLD(32, 0)

#define WED_PCIE_CFG_ADDR_INTM_FLD_PTR                         REG_FLD(32, 0)

#define WED_PCIE_INTS_TRIG_FLD_INTS_TRIG                       REG_FLD(32, 0)

#define WED_PCIE_INTS_REC_FLD_INTS                             REG_FLD(32, 0)

#define WED_PCIE_INTM_REC_FLD_INTM                             REG_FLD(32, 0)
#define WED_PCIE_INT_CTRL_FLD_IRQ_MSI_SEL                      REG_FLD(1, 25)
#define WED_PCIE_INT_CTRL_FLD_MSI_EN                           REG_FLD(1, 24)
#define WED_PCIE_INT_CTRL_FLD_CLR_PCIE_BF_WPDMA                REG_FLD(1, 22)
#define WED_PCIE_INT_CTRL_FLD_MSK_EN_POLA                      REG_FLD(1, 20)
#define WED_PCIE_INT_CTRL_FLD_EXT_INCPT_DIS                    REG_FLD(1, 18)
#define WED_PCIE_INT_CTRL_FLD_SRC_SEL                          REG_FLD(2, 16)
#define WED_PCIE_INT_CTRL_FLD_POLL_EN                          REG_FLD(2, 12)
#define WED_PCIE_INT_CTRL_FLD_POLL_PRD                         REG_FLD(8, 0)

#define WED_PCIE_MSIS_CLR_FLD_CFG                              REG_FLD(1, 0)

#define WED_WPDMA_CFG_ADDR_INTS_FLD_PTR                        REG_FLD(32, 0)

#define WED_WPDMA_CFG_ADDR_INTM_FLD_PTR                        REG_FLD(32, 0)

#define WED_WPDMA_CFG_ADDR_TX_FLD_PTR                          REG_FLD(32, 0)

#define WED_WPDMA_CFG_ADDR_TX_FREE_FLD_PTR                     REG_FLD(32, 0)

#define WED_WPDMA_CFG_ADDR_RX_FLD_PTR                          REG_FLD(32, 0)

#define WED_WPDMA_TX0_MIB_FLD_PKT_CNT                          REG_FLD(32, 0)

#define WED_WPDMA_TX1_MIB_FLD_PKT_CNT                          REG_FLD(32, 0)

#define WED_WPDMA_TX_COHERENT_MIB_FLD_CNT                      REG_FLD(32, 0)

#define WED_WPDMA_RX0_MIB_FLD_PKT_CNT                          REG_FLD(32, 0)

#define WED_WPDMA_RX1_MIB_FLD_PKT_CNT                          REG_FLD(32, 0)

#define WED_WPDMA_RX_COHERENT_MIB_FLD_CNT                      REG_FLD(32, 0)

#define WED_WPDMA_RX_EXTC_FREE_TKID_MIB_FLD_CNT                REG_FLD(32, 0)

#define WED_WPDMA_TX0_CTRL0_FLD_BASE_PTR                       REG_FLD(32, 0)

#define WED_WPDMA_TX0_CTRL1_FLD_MAX_CNT                        REG_FLD(12, 0)

#define WED_WPDMA_TX0_CTRL2_FLD_RLS_IDX                        REG_FLD(12, 16)
#define WED_WPDMA_TX0_CTRL2_FLD_CPU_IDX                        REG_FLD(12, 0)

#define WED_WPDMA_TX0_CTRL3_FLD_DMA_IDX_MIRO                   REG_FLD(12, 0)

#define WED_WPDMA_TX1_CTRL0_FLD_BASE_PTR                       REG_FLD(32, 0)

#define WED_WPDMA_TX1_CTRL1_FLD_MAX_CNT                        REG_FLD(12, 0)

#define WED_WPDMA_TX1_CTRL2_FLD_RLS_IDX                        REG_FLD(12, 16)
#define WED_WPDMA_TX1_CTRL2_FLD_CPU_IDX                        REG_FLD(12, 0)

#define WED_WPDMA_TX1_CTRL3_FLD_DMA_IDX_MIRO                   REG_FLD(12, 0)

#define WED_WPDMA_RX0_CTRL0_FLD_BASE_PTR                       REG_FLD(32, 0)

#define WED_WPDMA_RX0_CTRL1_FLD_MAX_CNT                        REG_FLD(12, 0)

#define WED_WPDMA_RX0_CTRL2_FLD_DRV_IDX                        REG_FLD(12, 16)
#define WED_WPDMA_RX0_CTRL2_FLD_CPU_IDX                        REG_FLD(12, 0)

#define WED_WPDMA_RX0_CTRL3_FLD_DMA_IDX_MIRO                   REG_FLD(12, 0)

#define WED_WPDMA_RX1_CTRL0_FLD_BASE_PTR                       REG_FLD(32, 0)

#define WED_WPDMA_RX1_CTRL1_FLD_MAX_CNT                        REG_FLD(12, 0)

#define WED_WPDMA_RX1_CTRL2_FLD_DRV_IDX                        REG_FLD(12, 16)
#define WED_WPDMA_RX1_CTRL2_FLD_CPU_IDX                        REG_FLD(12, 0)

#define WED_WPDMA_RX1_CTRL3_FLD_DMA_IDX_MIRO                   REG_FLD(12, 0)

#define WED_WPDMA_RX_D_RX0_BASE_FLD_PTR                        REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX0_CNT_FLD_MAX                         REG_FLD(12, 0)

#define WED_WPDMA_RX_D_RX0_CRX_IDX_FLD_DRV_IDX                 REG_FLD(12, 16)
#define WED_WPDMA_RX_D_RX0_CRX_IDX_FLD_CRX_IDX                 REG_FLD(12, 0)

#define WED_WPDMA_RX_D_RX0_DRX_IDX_FLD_DRX_IDX_MIRO            REG_FLD(12, 0)

#define WED_WPDMA_RX_D_RX1_BASE_FLD_PTR                        REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX1_CNT_FLD_MAX                         REG_FLD(12, 0)

#define WED_WPDMA_RX_D_RX1_CRX_IDX_FLD_DRV_IDX                 REG_FLD(12, 16)
#define WED_WPDMA_RX_D_RX1_CRX_IDX_FLD_CRX_IDX                 REG_FLD(12, 0)

#define WED_WPDMA_RX_D_RX1_DRX_IDX_FLD_DRX_IDX_MIRO            REG_FLD(12, 0)

#define WED_WPDMA_RX_D_RX0_THRES_CFG_FLD_DRX_CRX_DISTANCE_THRES REG_FLD(12, 16)
#define WED_WPDMA_RX_D_RX0_THRES_CFG_FLD_WAIT_BM_CNT_MAX       REG_FLD(16, 0)

#define WED_WPDMA_RX_D_RX1_THRES_CFG_FLD_DRX_CRX_DISTANCE_THRES REG_FLD(12, 16)
#define WED_WPDMA_RX_D_RX1_THRES_CFG_FLD_WAIT_BM_CNT_MAX       REG_FLD(16, 0)

#define WED_WPDMA_RX_D_INFO_FLD_INDEX_WIDTH                    REG_FLD(4, 24)
#define WED_WPDMA_RX_D_INFO_FLD_BASE_PTR_WIDTH                 REG_FLD(8, 16)
#define WED_WPDMA_RX_D_INFO_FLD_RX_RING_NUM                    REG_FLD(8, 8)

#define WED_WPDMA_RX_D_GLO_CFG_FLD_RXD_READ_LEN                REG_FLD(8, 24)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_THRES_REACH_METHOD REG_FLD(1, 23)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_CNT_SRC   REG_FLD(1, 22)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_DYNAMIC_DMAD_RECYCLE        REG_FLD(1, 21)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_PREPARE   REG_FLD(1, 20)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_IDLE_STATE_DMAD_SUPPLY_EN   REG_FLD(1, 19)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_IDLE_STATE_DMAD_SUPPLY_SINGLE_DMAD_WRITE REG_FLD(1, 18)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_AXI_W_AFTER_AW_EN           REG_FLD(1, 17)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_WAIT_COHERENT_EN            REG_FLD(1, 16)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_WCOMPLETE_SEL               REG_FLD(1, 15)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_DISABLE_DMAD_HANDLE_PROC    REG_FLD(1, 14)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_DISABLE_IRQ_HANDLE_PROC     REG_FLD(1, 13)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_RXDRV_DISABLED_FSM_AUTO_IDLE REG_FLD(1, 12)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_INIT_PHASE_RXEN_SEL         REG_FLD(5, 7)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_INIT_PHASE_ENABLING_RXDMA_BYPASS REG_FLD(1, 6)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_INIT_PHASE_BYPASS           REG_FLD(1, 5)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_RST_INIT_COMPLETE_FLAG      REG_FLD(1, 4)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_FSM_RETURN_IDLE             REG_FLD(1, 3)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_RXDRV_CLKGATE_BYP           REG_FLD(1, 2)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_RX_DRV_BUSY                 REG_FLD(1, 1)
#define WED_WPDMA_RX_D_GLO_CFG_FLD_RX_DRV_EN                   REG_FLD(1, 0)

#define WED_WPDMA_RX_D_RST_IDX_FLD_DRV_IDX1                    REG_FLD(1, 25)
#define WED_WPDMA_RX_D_RST_IDX_FLD_DRV_IDX0                    REG_FLD(1, 24)
#define WED_WPDMA_RX_D_RST_IDX_FLD_CRX_IDX1                    REG_FLD(1, 17)
#define WED_WPDMA_RX_D_RST_IDX_FLD_CRX_IDX0                    REG_FLD(1, 16)

#define WED_WPDMA_RX_D_LOAD_DRV_IDX_FLD_LOAD_DRV_IDX1          REG_FLD(1, 31)
#define WED_WPDMA_RX_D_LOAD_DRV_IDX_FLD_DRV_IDX1               REG_FLD(12, 16)
#define WED_WPDMA_RX_D_LOAD_DRV_IDX_FLD_LOAD_DRV_IDX0          REG_FLD(1, 15)
#define WED_WPDMA_RX_D_LOAD_DRV_IDX_FLD_DRV_IDX0               REG_FLD(12, 0)

#define WED_WPDMA_RX_D_LOAD_CRX_IDX_FLD_LOAD_CRX_IDX1          REG_FLD(1, 31)
#define WED_WPDMA_RX_D_LOAD_CRX_IDX_FLD_CRX_IDX1               REG_FLD(12, 16)
#define WED_WPDMA_RX_D_LOAD_CRX_IDX_FLD_LOAD_CRX_IDX0          REG_FLD(1, 15)
#define WED_WPDMA_RX_D_LOAD_CRX_IDX_FLD_CRX_IDX0               REG_FLD(12, 0)

#define WED_WPDMA_RX_D_GLO_CFG_ADDR_FLD_ADDRESS                REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RING_CFG_ADDR_FLD_ADDRESS               REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX0_MIB_FLD_PKT_CNT                     REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX1_MIB_FLD_PKT_CNT                     REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX0_RECYCLE_MIB_FLD_CNT                 REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX1_RECYCLE_MIB_FLD_CNT                 REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX0_PROCESSED_MIB_FLD_PKT_CNT           REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX1_PROCESSED_MIB_FLD_PKT_CNT           REG_FLD(32, 0)

#define WED_WPDMA_RX_D_RX_COHERENT_MIB_FLD_CNT                 REG_FLD(32, 0)

#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_GET_BM_DMAD_SKIP     REG_FLD(1, 7)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_DMAD_RECYCLE         REG_FLD(1, 6)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_BM_DMAD_ERROR        REG_FLD(1, 5)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_BM_DMAD_COHERENT     REG_FLD(1, 4)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_INTI_WDMA_ENABLED    REG_FLD(1, 3)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_COHERENT             REG_FLD(1, 2)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_W_RESP_ERR           REG_FLD(1, 1)
#define WED_WPDMA_RX_D_ERR_STS_FLD_RX_DRV_R_RESP_ERR           REG_FLD(1, 0)

#define WED_WPDMA_RX_D_TKID_CFG_FLD_TKID_RING_EN               REG_FLD(1, 0)

#define WED_WPDMA_RX_D_TKID_RX0_BASE_FLD_PTR                   REG_FLD(32, 0)

#define WED_WPDMA_RX_D_TKID_RX1_BASE_FLD_PTR                   REG_FLD(32, 0)

#define WED_WPDMA_RX_D_BUS_CFG_FLD_RD_BND_4KB_BST              REG_FLD(1, 1)
#define WED_WPDMA_RX_D_BUS_CFG_FLD_WR_BND_4KB_BST              REG_FLD(1, 0)
#define WED_WDMA_TX0_BASE_FLD_PTR                              REG_FLD(32, 0)

#define WED_WDMA_TX0_CNT_FLD_MAX                               REG_FLD(12, 0)

#define WED_WDMA_TX0_CTX_IDX_FLD_CTX_IDX                       REG_FLD(12, 0)

#define WED_WDMA_TX0_DTX_IDX_FLD_DTX_IDX                       REG_FLD(12, 0)

#define WED_WDMA_TX0_MIB_FLD_RD_CNT                            REG_FLD(16, 16)
#define WED_WDMA_TX0_MIB_FLD_WR_CNT                            REG_FLD(16, 0)
#define WED_WDMA_RX0_BASE_FLD_PTR                              REG_FLD(32, 0)

#define WED_WDMA_RX0_CNT_FLD_MAX                               REG_FLD(12, 0)

#define WED_WDMA_RX0_CRX_IDX_FLD_DRV_IDX                       REG_FLD(12, 16)
#define WED_WDMA_RX0_CRX_IDX_FLD_CRX_IDX                       REG_FLD(12, 0)

#define WED_WDMA_RX0_DRX_IDX_FLD_DRX_IDX_MIRO                  REG_FLD(12, 0)

#define WED_WDMA_RX1_BASE_FLD_PTR                              REG_FLD(32, 0)

#define WED_WDMA_RX1_CNT_FLD_MAX                               REG_FLD(12, 0)

#define WED_WDMA_RX1_CRX_IDX_FLD_DRV_IDX                       REG_FLD(12, 16)
#define WED_WDMA_RX1_CRX_IDX_FLD_CRX_IDX                       REG_FLD(12, 0)

#define WED_WDMA_RX1_DRX_IDX_FLD_DRX_IDX_MIRO                  REG_FLD(12, 0)

#define WED_WDMA_RX0_THRES_CFG_FLD_DRX_CRX_DISTANCE_THRES      REG_FLD(12, 16)
#define WED_WDMA_RX0_THRES_CFG_FLD_WAIT_BM_CNT_MAX             REG_FLD(16, 0)

#define WED_WDMA_RX1_THRES_CFG_FLD_DRX_CRX_DISTANCE_THRES      REG_FLD(12, 16)
#define WED_WDMA_RX1_THRES_CFG_FLD_WAIT_BM_CNT_MAX             REG_FLD(16, 0)

#define WED_WDMA_INFO_FLD_INDEX_WIDTH                          REG_FLD(4, 24)
#define WED_WDMA_INFO_FLD_BASE_PTR_WIDTH                       REG_FLD(8, 16)
#define WED_WDMA_INFO_FLD_RX_RING_NUM                          REG_FLD(8, 8)
#define WED_WDMA_INFO_FLD_TX_RING_NUM                          REG_FLD(8, 0)

#define WED_WDMA_GLO_CFG_FLD_RXDRV_CLKGATE_BYP                 REG_FLD(1, 30)
#define WED_WDMA_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_THRES_REACH_METHOD REG_FLD(1, 28)
#define WED_WDMA_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_CNT_SRC         REG_FLD(1, 27)
#define WED_WDMA_GLO_CFG_FLD_RST_INIT_COMPLETE_FLAG            REG_FLD(1, 26)
#define WED_WDMA_GLO_CFG_FLD_DYNAMIC_DMAD_RECYCLE              REG_FLD(1, 25)
#define WED_WDMA_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_PREPARE         REG_FLD(1, 24)
#define WED_WDMA_GLO_CFG_FLD_IDLE_STATE_DMAD_SUPPLY_EN         REG_FLD(1, 23)
#define WED_WDMA_GLO_CFG_FLD_IDLE_STATE_DMAD_SUPPLY_SINGLE_DMAD_WRITE REG_FLD(1, 22)
#define WED_WDMA_GLO_CFG_FLD_AXI_W_AFTER_AW_EN                 REG_FLD(1, 21)
#define WED_WDMA_GLO_CFG_FLD_WAIT_COHERENT_EN                  REG_FLD(1, 20)
#define WED_WDMA_GLO_CFG_FLD_FSM_RETURN_IDLE                   REG_FLD(1, 19)
#define WED_WDMA_GLO_CFG_FLD_INIT_PHASE_BYPASS                 REG_FLD(1, 18)
#define WED_WDMA_GLO_CFG_FLD_INIT_PHASE_ENABLING_RXDMA_BYPASS  REG_FLD(1, 17)
#define WED_WDMA_GLO_CFG_FLD_WCOMPLETE_SEL                     REG_FLD(1, 16)
#define WED_WDMA_GLO_CFG_FLD_DISABLE_DMAD_HANDLE_PROC          REG_FLD(1, 15)
#define WED_WDMA_GLO_CFG_FLD_DISABLE_IRQ_HANDLE_PROC           REG_FLD(1, 14)
#define WED_WDMA_GLO_CFG_FLD_RXDRV_DISABLED_FSM_AUTO_IDLE      REG_FLD(1, 13)
#define WED_WDMA_GLO_CFG_FLD_RSV0                              REG_FLD(5, 8)
#define WED_WDMA_GLO_CFG_FLD_BIG_ENDIAN                        REG_FLD(1, 7)
#define WED_WDMA_GLO_CFG_FLD_TX_WB_DDONE                       REG_FLD(1, 6)
#define WED_WDMA_GLO_CFG_FLD_WDMA_BT_SIZE                      REG_FLD(2, 4)
#define WED_WDMA_GLO_CFG_FLD_RX_DRV_BUSY                       REG_FLD(1, 3)
#define WED_WDMA_GLO_CFG_FLD_RX_DRV_EN                         REG_FLD(1, 2)
#define WED_WDMA_GLO_CFG_FLD_TX_DDONE_CHK                      REG_FLD(1, 1)
#define WED_WDMA_GLO_CFG_FLD_TX_DRV_EN                         REG_FLD(1, 0)

#define WED_WDMA_RST_IDX_FLD_DRV_IDX1                          REG_FLD(1, 25)
#define WED_WDMA_RST_IDX_FLD_DRV_IDX0                          REG_FLD(1, 24)
#define WED_WDMA_RST_IDX_FLD_CTX_IDX1                          REG_FLD(1, 19)
#define WED_WDMA_RST_IDX_FLD_CTX_IDX0                          REG_FLD(1, 18)
#define WED_WDMA_RST_IDX_FLD_CRX_IDX1                          REG_FLD(1, 17)
#define WED_WDMA_RST_IDX_FLD_CRX_IDX0                          REG_FLD(1, 16)

#define WED_WDMA_LOAD_DRV_IDX_FLD_LOAD_DRV_IDX1                REG_FLD(1, 31)
#define WED_WDMA_LOAD_DRV_IDX_FLD_DRV_IDX1                     REG_FLD(12, 16)
#define WED_WDMA_LOAD_DRV_IDX_FLD_LOAD_DRV_IDX0                REG_FLD(1, 15)
#define WED_WDMA_LOAD_DRV_IDX_FLD_DRV_IDX0                     REG_FLD(12, 0)

#define WED_WDMA_LOAD_CRX_IDX_FLD_LOAD_CRX_IDX1                REG_FLD(1, 31)
#define WED_WDMA_LOAD_CRX_IDX_FLD_CRX_IDX1                     REG_FLD(12, 16)
#define WED_WDMA_LOAD_CRX_IDX_FLD_LOAD_CRX_IDX0                REG_FLD(1, 15)
#define WED_WDMA_LOAD_CRX_IDX_FLD_CRX_IDX0                     REG_FLD(12, 0)

#define WED_WDMA_SPR_FLD_RSV_REG                               REG_FLD(32, 0)

#define WED_WDMA_INT_STA_REC_FLD_RX_COHERENT                   REG_FLD(1, 31)
#define WED_WDMA_INT_STA_REC_FLD_RX_DLY_INT                    REG_FLD(1, 30)
#define WED_WDMA_INT_STA_REC_FLD_TX_COHERENT                   REG_FLD(1, 29)
#define WED_WDMA_INT_STA_REC_FLD_TX_DLY_INT                    REG_FLD(1, 28)
#define WED_WDMA_INT_STA_REC_FLD_RX_DONE1                      REG_FLD(1, 17)
#define WED_WDMA_INT_STA_REC_FLD_RX_DONE0                      REG_FLD(1, 16)
#define WED_WDMA_INT_STA_REC_FLD_TX_DONE3                      REG_FLD(1, 3)
#define WED_WDMA_INT_STA_REC_FLD_TX_DONE2                      REG_FLD(1, 2)
#define WED_WDMA_INT_STA_REC_FLD_TX_DONE1                      REG_FLD(1, 1)
#define WED_WDMA_INT_STA_REC_FLD_TX_DONE0                      REG_FLD(1, 0)

#define WED_WDMA_INT_CLR_FLD_INT_CLR                           REG_FLD(32, 0)

#define WED_WDMA_INT_TRIG_FLD_INT_TRIG                         REG_FLD(32, 0)

#define WED_WDMA_INT_CTRL_FLD_SUBRT_ADV                        REG_FLD(1, 21)
#define WED_WDMA_INT_CTRL_FLD_EXT_INCPT_DIS                    REG_FLD(1, 18)
#define WED_WDMA_INT_CTRL_FLD_SRC_SEL                          REG_FLD(2, 16)
#define WED_WDMA_INT_CTRL_FLD_POLL_MODE                        REG_FLD(2, 12)
#define WED_WDMA_INT_CTRL_FLD_POLL_PRD                         REG_FLD(8, 0)

#define WED_WDMA_INT_MON_FLD_RX_DONE_INT1                      REG_FLD(1, 17)
#define WED_WDMA_INT_MON_FLD_RX_DONE_INT0                      REG_FLD(1, 16)

#define WED_WDMA_CFG_BASE_FLD_PTR                              REG_FLD(32, 0)

#define WED_WDMA_OFST0_FLD_GLO_CFG                             REG_FLD(16, 16)
#define WED_WDMA_OFST0_FLD_INTS                                REG_FLD(16, 0)

#define WED_WDMA_OFST1_FLD_RX0_CTRL                            REG_FLD(16, 16)
#define WED_WDMA_OFST1_FLD_TX0_CTRL                            REG_FLD(16, 0)

#define WED_WDMA_RX0_MIB_FLD_PKT_CNT                           REG_FLD(32, 0)

#define WED_WDMA_RX1_MIB_FLD_PKT_CNT                           REG_FLD(32, 0)

#define WED_WDMA_RX0_RECYCLE_MIB_FLD_CNT                       REG_FLD(32, 0)

#define WED_WDMA_RX1_RECYCLE_MIB_FLD_CNT                       REG_FLD(32, 0)

#define WED_WDMA_RX0_PROCESSED_MIB_FLD_PKT_CNT                 REG_FLD(32, 0)

#define WED_WDMA_RX1_PROCESSED_MIB_FLD_PKT_CNT                 REG_FLD(32, 0)

#define WED_WDMA_RX_COHERENT_MIB_FLD_CNT                       REG_FLD(32, 0)

#define WED_RTQM_GLO_CFG_FLD_DMAD_PVLD_UPD                     REG_FLD(1, 24)
#define WED_RTQM_GLO_CFG_FLD_TXDMAD_FPORT                      REG_FLD(4, 20)
#define WED_RTQM_GLO_CFG_FLD_TXDMAD_OFLD                       REG_FLD(4, 16)
#define WED_RTQM_GLO_CFG_FLD_Q_DQCON                           REG_FLD(1, 15)
#define WED_RTQM_GLO_CFG_FLD_Q_EQCON                           REG_FLD(1, 14)
#define WED_RTQM_GLO_CFG_FLD_Q_PEMPTY                          REG_FLD(1, 13)
#define WED_RTQM_GLO_CFG_FLD_Q_AEMPTY                          REG_FLD(1, 12)
#define WED_RTQM_GLO_CFG_FLD_Q_EMPTY                           REG_FLD(1, 11)
#define WED_RTQM_GLO_CFG_FLD_Q_FULL                            REG_FLD(1, 10)
#define WED_RTQM_GLO_CFG_FLD_PFDBK_BUF_DIS                     REG_FLD(1, 9)
#define WED_RTQM_GLO_CFG_FLD_HWEQ_BUF_DIS                      REG_FLD(1, 8)
#define WED_RTQM_GLO_CFG_FLD_RXDMAD_OFST_UNIT                  REG_FLD(2, 6)
#define WED_RTQM_GLO_CFG_FLD_DBG_BYPASS                        REG_FLD(1, 5)
#define WED_RTQM_GLO_CFG_FLD_BSY_RESP_IM                       REG_FLD(1, 4)
#define WED_RTQM_GLO_CFG_FLD_CG_DIS                            REG_FLD(1, 3)
#define WED_RTQM_GLO_CFG_FLD_Q_RST                             REG_FLD(1, 2)
#define WED_RTQM_GLO_CFG_FLD_BUSY                              REG_FLD(1, 1)

#define WED_RTQM_HIDX_CFG_FLD_OW                               REG_FLD(1, 31)
#define WED_RTQM_HIDX_CFG_FLD_OW_VAL                           REG_FLD(9, 16)
#define WED_RTQM_HIDX_CFG_FLD_CLR                              REG_FLD(1, 15)
#define WED_RTQM_HIDX_CFG_FLD_IDX                              REG_FLD(9, 0)

#define WED_RTQM_TIDX_CFG_FLD_OW                               REG_FLD(1, 31)
#define WED_RTQM_TIDX_CFG_FLD_OW_VAL                           REG_FLD(9, 16)
#define WED_RTQM_TIDX_CFG_FLD_CLR                              REG_FLD(1, 15)
#define WED_RTQM_TIDX_CFG_FLD_IDX                              REG_FLD(9, 0)

#define WED_RTQM_RIDX_CFG_FLD_OW                               REG_FLD(1, 31)
#define WED_RTQM_RIDX_CFG_FLD_OW_VAL                           REG_FLD(9, 16)
#define WED_RTQM_RIDX_CFG_FLD_CLR                              REG_FLD(1, 15)
#define WED_RTQM_RIDX_CFG_FLD_IDX                              REG_FLD(9, 0)

#define WED_RTQM_DIDX_CFG_FLD_OW                               REG_FLD(1, 31)
#define WED_RTQM_DIDX_CFG_FLD_OW_VAL                           REG_FLD(9, 16)
#define WED_RTQM_DIDX_CFG_FLD_CLR                              REG_FLD(1, 15)
#define WED_RTQM_DIDX_CFG_FLD_IDX                              REG_FLD(9, 0)

#define WED_RTQM_WRR_CFG0_FLD_RING1_WT_R2H                     REG_FLD(4, 28)
#define WED_RTQM_WRR_CFG0_FLD_RING1_WT_Q2H                     REG_FLD(4, 24)
#define WED_RTQM_WRR_CFG0_FLD_RING1_WFQ_RST                    REG_FLD(1, 18)
#define WED_RTQM_WRR_CFG0_FLD_RING1_WFQ_SP                     REG_FLD(1, 17)
#define WED_RTQM_WRR_CFG0_FLD_RING1_SCH_MODE                   REG_FLD(1, 16)
#define WED_RTQM_WRR_CFG0_FLD_RING0_WT_R2H                     REG_FLD(4, 12)
#define WED_RTQM_WRR_CFG0_FLD_RING0_WT_Q2H                     REG_FLD(4, 8)
#define WED_RTQM_WRR_CFG0_FLD_RING0_WFQ_RST                    REG_FLD(1, 2)
#define WED_RTQM_WRR_CFG0_FLD_RING0_WFQ_SP                     REG_FLD(1, 1)
#define WED_RTQM_WRR_CFG0_FLD_RING0_SCH_MODE                   REG_FLD(1, 0)

#define WED_RTQM_WRR_CFG1_FLD_WT_CSR_RD                        REG_FLD(4, 28)
#define WED_RTQM_WRR_CFG1_FLD_WT_DQ_RXBM                       REG_FLD(4, 24)
#define WED_RTQM_WRR_CFG1_FLD_WT_DQ_HOST_RING1                 REG_FLD(4, 20)
#define WED_RTQM_WRR_CFG1_FLD_WT_DQ_HOST_RING0                 REG_FLD(4, 16)
#define WED_RTQM_WRR_CFG1_FLD_WT_DRV_PUSH                      REG_FLD(4, 12)

#define WED_RTQM_FIFO_CFG_FLD_Q2H1_CNT                         REG_FLD(3, 28)
#define WED_RTQM_FIFO_CFG_FLD_Q2H1_EMPTY                       REG_FLD(1, 26)
#define WED_RTQM_FIFO_CFG_FLD_Q2H1_FULL                        REG_FLD(1, 25)
#define WED_RTQM_FIFO_CFG_FLD_Q2H1_CLEAR                       REG_FLD(1, 24)
#define WED_RTQM_FIFO_CFG_FLD_Q2H0_CNT                         REG_FLD(3, 20)
#define WED_RTQM_FIFO_CFG_FLD_Q2H0_EMPTY                       REG_FLD(1, 18)
#define WED_RTQM_FIFO_CFG_FLD_Q2H0_FULL                        REG_FLD(1, 17)
#define WED_RTQM_FIFO_CFG_FLD_Q2H0_CLEAR                       REG_FLD(1, 16)
#define WED_RTQM_FIFO_CFG_FLD_Q2B_CNT                          REG_FLD(3, 12)
#define WED_RTQM_FIFO_CFG_FLD_Q2B_EMPTY                        REG_FLD(1, 10)
#define WED_RTQM_FIFO_CFG_FLD_Q2B_FULL                         REG_FLD(1, 9)
#define WED_RTQM_FIFO_CFG_FLD_Q2B_CLEAR                        REG_FLD(1, 8)
#define WED_RTQM_FIFO_CFG_FLD_R2Q_CNT                          REG_FLD(4, 4)
#define WED_RTQM_FIFO_CFG_FLD_R2Q_EMPTY                        REG_FLD(1, 2)
#define WED_RTQM_FIFO_CFG_FLD_R2Q_FULL                         REG_FLD(1, 1)
#define WED_RTQM_FIFO_CFG_FLD_R2Q_CLEAR                        REG_FLD(1, 0)

#define WED_RTQM_RD_CFG_FLD_ADDRESS                            REG_FLD(9, 16)
#define WED_RTQM_RD_CFG_FLD_DONE                               REG_FLD(1, 1)
#define WED_RTQM_RD_CFG_FLD_RD                                 REG_FLD(1, 0)

#define WED_RTQM_RD_DATA0_FLD_DATA                             REG_FLD(32, 0)

#define WED_RTQM_RD_DATA1_FLD_DATA                             REG_FLD(32, 0)

#define WED_RTQM_RD_DATA2_FLD_DATA                             REG_FLD(32, 0)

#define WED_RTQM_RD_DATA3_FLD_DATA                             REG_FLD(32, 0)

#define WED_RTQM_RD_DATA4_FLD_DQ_DONE                          REG_FLD(1, 3)
#define WED_RTQM_RD_DATA4_FLD_DQ_DESN                          REG_FLD(1, 2)
#define WED_RTQM_RD_DATA4_FLD_DQ_RINF                          REG_FLD(2, 0)

#define WED_RTQM_DQRGN_CNT_FLD_TAR_101_CNT                     REG_FLD(9, 22)
#define WED_RTQM_DQRGN_CNT_FLD_TAR_011_CNT                     REG_FLD(9, 13)
#define WED_RTQM_DQRGN_CNT_FLD_TAR_XX0_CNT                     REG_FLD(9, 4)
#define WED_RTQM_DQRGN_CNT_FLD_TAR_101_CNT_CLR                 REG_FLD(1, 2)
#define WED_RTQM_DQRGN_CNT_FLD_TAR_011_CNT_CLR                 REG_FLD(1, 1)
#define WED_RTQM_DQRGN_CNT_FLD_TAR_XX0_CNT_CLR                 REG_FLD(1, 0)

#define WED_RTQM_ERR_STS_FLD_PFDBK_SEQ_ERR                     REG_FLD(1, 18)
#define WED_RTQM_ERR_STS_FLD_Q_DMEM_RW_ERR                     REG_FLD(1, 17)
#define WED_RTQM_ERR_STS_FLD_Q_IMEM_RW_ERR                     REG_FLD(1, 16)
#define WED_RTQM_ERR_STS_FLD_PPE_FDBK_UEXP_ERR                 REG_FLD(1, 15)
#define WED_RTQM_ERR_STS_FLD_Q_RD_ERR                          REG_FLD(1, 14)
#define WED_RTQM_ERR_STS_FLD_PSH_DRV_ERR                       REG_FLD(1, 13)
#define WED_RTQM_ERR_STS_FLD_DQ_REQ_ERR                        REG_FLD(1, 12)
#define WED_RTQM_ERR_STS_FLD_UPD_RLS_ERR                       REG_FLD(1, 11)
#define WED_RTQM_ERR_STS_FLD_EQ_REQ_ERR                        REG_FLD(1, 10)
#define WED_RTQM_ERR_STS_FLD_DQ_PEMPTY_ERR                     REG_FLD(1, 9)
#define WED_RTQM_ERR_STS_FLD_DQ_EMPTY_ERR                      REG_FLD(1, 8)
#define WED_RTQM_ERR_STS_FLD_Q2B_FIFO_R_ERR                    REG_FLD(1, 7)
#define WED_RTQM_ERR_STS_FLD_Q2B_FIFO_W_ERR                    REG_FLD(1, 6)
#define WED_RTQM_ERR_STS_FLD_Q2H_R1_FIFO_R_ERR                 REG_FLD(1, 5)
#define WED_RTQM_ERR_STS_FLD_Q2H_R1_FIFO_W_ERR                 REG_FLD(1, 4)
#define WED_RTQM_ERR_STS_FLD_Q2H_R0_FIFO_R_ERR                 REG_FLD(1, 3)
#define WED_RTQM_ERR_STS_FLD_Q2H_R0_FIFO_W_ERR                 REG_FLD(1, 2)
#define WED_RTQM_ERR_STS_FLD_R2Q_FIFO_R_ERR                    REG_FLD(1, 1)
#define WED_RTQM_ERR_STS_FLD_R2Q_FIFO_W_ERR                    REG_FLD(1, 0)

#define WED_RTQM_DBG_CFG_FLD_Q2HB_FORCE                        REG_FLD(2, 22)
#define WED_RTQM_DBG_CFG_FLD_R2HQ_FORCE                        REG_FLD(2, 20)
#define WED_RTQM_DBG_CFG_FLD_R2Q_DROP                          REG_FLD(1, 17)
#define WED_RTQM_DBG_CFG_FLD_PPE_FDBK_DROP                     REG_FLD(1, 16)
#define WED_RTQM_DBG_CFG_FLD_Q_MON_SEL                         REG_FLD(8, 0)

#define WED_RTQM_DBG_MON0_FLD_DATA                             REG_FLD(32, 0)

#define WED_RTQM_DBG_MON1_FLD_DATA                             REG_FLD(32, 0)

#define WED_RTQM_DBG_MON2_FLD_DATA                             REG_FLD(32, 0)

#define WED_RTQM_DBG_MON3_FLD_DATA                             REG_FLD(32, 0)

#define WED_RTQM_DBG_PFDBK_SEQ_CFG_FLD_RX_SEQ_ID_OW            REG_FLD(1, 11)
#define WED_RTQM_DBG_PFDBK_SEQ_CFG_FLD_TX_SEQ_ID_OW            REG_FLD(1, 10)
#define WED_RTQM_DBG_PFDBK_SEQ_CFG_FLD_ERR_ID_CLR              REG_FLD(1, 9)
#define WED_RTQM_DBG_PFDBK_SEQ_CFG_FLD_ERR_ID_VLD              REG_FLD(1, 8)
#define WED_RTQM_DBG_PFDBK_SEQ_CFG_FLD_BIT_SEL                 REG_FLD(4, 4)
#define WED_RTQM_DBG_PFDBK_SEQ_CFG_FLD_DBG_RST                 REG_FLD(1, 1)
#define WED_RTQM_DBG_PFDBK_SEQ_CFG_FLD_DBG_EN                  REG_FLD(1, 0)

#define WED_RTQM_DBG_PFDBK_SEQ_ERR_FLD_CORRECT_ID              REG_FLD(16, 16)
#define WED_RTQM_DBG_PFDBK_SEQ_ERR_FLD_ERROR_ID                REG_FLD(16, 0)

#define WED_RTQM_DBG_PFDBK_SEQ_OW_FLD_RX_SEQ_ID_OW_VAL         REG_FLD(16, 16)
#define WED_RTQM_DBG_PFDBK_SEQ_OW_FLD_TX_SEQ_ID_OW_VAL         REG_FLD(16, 0)

#define WED_RTQM_FIFO_PPE_CFG_FLD_P2Q_CNT                      REG_FLD(3, 4)
#define WED_RTQM_FIFO_PPE_CFG_FLD_P2Q_EMPTY                    REG_FLD(1, 2)
#define WED_RTQM_FIFO_PPE_CFG_FLD_P2Q_FULL                     REG_FLD(1, 1)
#define WED_RTQM_FIFO_PPE_CFG_FLD_P2Q_CLEAR                    REG_FLD(1, 0)

#define WED_RTQM_R2H_MIB_0_FLD_CNT                             REG_FLD(32, 0)

#define WED_RTQM_R2H_MIB_1_FLD_CNT                             REG_FLD(32, 0)

#define WED_RTQM_R2Q_MIB_0_FLD_CNT                             REG_FLD(32, 0)

#define WED_RTQM_R2Q_MIB_1_FLD_CNT                             REG_FLD(32, 0)

#define WED_RTQM_Q2N_MIB_FLD_CNT                               REG_FLD(32, 0)

#define WED_RTQM_Q2H_MIB_0_FLD_CNT                             REG_FLD(32, 0)

#define WED_RTQM_Q2H_MIB_1_FLD_CNT                             REG_FLD(32, 0)

#define WED_RTQM_Q2B_MIB_FLD_CNT                               REG_FLD(32, 0)

#define WED_RTQM_PFDBK_MIB_FLD_CNT                             REG_FLD(32, 0)
#define WED_RROQM_GLO_CFG_FLD_RROQM_BYPASS                     REG_FLD(1, 24)
#define WED_RROQM_GLO_CFG_FLD_WAIT_FDBK_COH_EN                 REG_FLD(1, 17)
#define WED_RROQM_GLO_CFG_FLD_WAIT_MOD_COH_EN                  REG_FLD(1, 16)
#define WED_RROQM_GLO_CFG_FLD_RROQM_BUSY                       REG_FLD(1, 1)

#define WED_RROQM_RST_IDX_FLD_FDBK                             REG_FLD(1, 4)
#define WED_RROQM_RST_IDX_FLD_MIOD                             REG_FLD(1, 0)

#define WED_RROQM_DLY_INT_FLD_MID_RDY_EN                       REG_FLD(1, 15)
#define WED_RROQM_DLY_INT_FLD_MID_RDY_MAX_PINT                 REG_FLD(7, 8)
#define WED_RROQM_DLY_INT_FLD_MID_RDY_MAX_PTIME                REG_FLD(8, 0)

#define WED_RROQM_MIOD_CTRL0_FLD_BASE_PTR                      REG_FLD(32, 0)

#define WED_RROQM_MIOD_CTRL1_FLD_BASE_PTR_EXT                  REG_FLD(4, 16)
#define WED_RROQM_MIOD_CTRL1_FLD_MAX_CNT                       REG_FLD(12, 0)

#define WED_RROQM_MIOD_CTRL2_FLD_TAKE_IDX                      REG_FLD(12, 0)

#define WED_RROQM_MIOD_CTRL3_FLD_PUT_IDX                       REG_FLD(12, 0)

#define WED_RROQM_FDBK_CTRL0_FLD_BASE_PTR                      REG_FLD(32, 0)

#define WED_RROQM_FDBK_CTRL1_FLD_BASE_PTR_EXT                  REG_FLD(4, 16)
#define WED_RROQM_FDBK_CTRL1_FLD_MAX_CNT                       REG_FLD(12, 0)

#define WED_RROQM_FDBK_CTRL2_FLD_CPU_IDX                       REG_FLD(12, 0)

#define WED_RROQM_FDBK_CTRL3_FLD_PREFETCH_IDX                  REG_FLD(12, 16)
#define WED_RROQM_FDBK_CTRL3_FLD_DMA_IDX                       REG_FLD(12, 0)

#define WED_RROQ_BASE_L_FLD_PTR                                REG_FLD(32, 0)

#define WED_RROQ_BASE_H_FLD_PTR                                REG_FLD(4, 0)

#define WED_RROQM_MIOD_CFG_FLD_MIOD_ENTRY_DW                   REG_FLD(7, 16)
#define WED_RROQM_MIOD_CFG_FLD_MOD_DW                          REG_FLD(6, 8)
#define WED_RROQM_MIOD_CFG_FLD_MID_DW                          REG_FLD(6, 0)

#define WED_RROQM_FDBK_REC_CTRL_FLD_PAGE                       REG_FLD(2, 8)
#define WED_RROQM_FDBK_REC_CTRL_FLD_CMD_EN                     REG_FLD(4, 0)

#define WED_RROQM_FDBK_REC0_FLD_CMD                            REG_FLD(32, 0)

#define WED_RROQM_FDBK_REC1_FLD_CMD                            REG_FLD(32, 0)

#define WED_RROQM_FDBK_REC2_FLD_CMD                            REG_FLD(32, 0)

#define WED_RROQM_FDBK_REC3_FLD_CMD                            REG_FLD(32, 0)

#define WED_RROQM_MID_MIB_FLD_CNT                              REG_FLD(32, 0)

#define WED_RROQM_MOD_MIB_FLD_CNT                              REG_FLD(32, 0)

#define WED_RROQM_MOD_COH_MIB_FLD_CNT                          REG_FLD(32, 0)

#define WED_RROQM_FDBK_MIB_FLD_CNT                             REG_FLD(32, 0)

#define WED_RROQM_FDBK_COH_MIB_FLD_CNT                         REG_FLD(32, 0)

#define WED_RROQM_FDBK_IND_MIB_FLD_CNT                         REG_FLD(32, 0)

#define WED_RROQM_FDBK_ENQ_MIB_FLD_CNT                         REG_FLD(32, 0)

#define WED_RROQM_FDBK_ANC_MIB_FLD_CNT                         REG_FLD(32, 0)

#define WED_RROQM_FDBK_ANC2H_MIB_FLD_CNT                       REG_FLD(32, 0)
#define WED_RX_BM_RX_DMAD_FLD_SDL0                             REG_FLD(14, 0)

#define WED_RX_BM_BASE_FLD_PTR                                 REG_FLD(32, 0)

#define WED_RX_BM_INIT_PTR_FLD_INIT_SW_TAIL_IDX                REG_FLD(1, 16)
#define WED_RX_BM_INIT_PTR_FLD_SW_TAIL_IDX                     REG_FLD(16, 0)

#define WED_RX_BM_PTR_FLD_HEAD_IDX                             REG_FLD(16, 16)
#define WED_RX_BM_PTR_FLD_TAIL_IDX                             REG_FLD(16, 0)

#define WED_RX_BM_BLEN_FLD_BYTE_LEN                            REG_FLD(14, 0)

#define WED_RX_BM_STS_FLD_DMAD_RD                              REG_FLD(1, 25)
#define WED_RX_BM_STS_FLD_DMAD_VLD                             REG_FLD(1, 24)
#define WED_RX_BM_STS_FLD_DMAD_NUM                             REG_FLD(5, 16)

#define WED_RX_BM_INTF2_FLD_DMAD_EXCHG_RD                      REG_FLD(1, 29)
#define WED_RX_BM_INTF2_FLD_DMAD_EXCHG_VLD                     REG_FLD(1, 28)
#define WED_RX_BM_INTF2_FLD_DMAD_EXCHG                         REG_FLD(16, 0)

#define WED_RX_BM_INTF_FLD_DMAD_FREE_RD                        REG_FLD(1, 29)
#define WED_RX_BM_INTF_FLD_DMAD_FREE_VLD                       REG_FLD(1, 28)
#define WED_RX_BM_INTF_FLD_DMAD_FREE                           REG_FLD(16, 0)

#define WED_RX_BM_ERR_STS_FLD_FREE_AT_EMPTY                    REG_FLD(1, 7)
#define WED_RX_BM_ERR_STS_FLD_DMAD_AXI_RD_OVER_TAIL            REG_FLD(1, 3)
#define WED_RX_BM_ERR_STS_FLD_DMAD_FULL_AXI_RD                 REG_FLD(1, 2)
#define WED_RX_BM_ERR_STS_FLD_DMAD_AXI_RD_MORE                 REG_FLD(1, 1)
#define WED_RX_BM_ERR_STS_FLD_DMAD_AXI_RD_LESS                 REG_FLD(1, 0)

#define WED_RX_BM_OPT_CTRL_FLD_FREE_AXI_RESP_SEL               REG_FLD(1, 0)
#define WED_RX_DMA_MIB_FLD_RD_CNT                              REG_FLD(16, 16)
#define WED_RX_DMA_MIB_FLD_WR_CNT                              REG_FLD(16, 0)

#define WED_RX_BM_DYN_ALLOC_TH_H							   REG_FLD(16, 16)
#define WED_RX_BM_DYN_ALLOC_TH_L							   REG_FLD(16, 0)

#define WED_RX_BM_DYN_ALLOC_CFG_HW_SUB_PAUSE				   REG_FLD(1, 31)
#define WED_RX_BM_DYN_ALLOC_CFG_HW_ADDSUB_ACK				   REG_FLD(1, 30)
#define WED_RX_BM_DYN_ALLOC_CFG_ERR_ADD_OVERFLOW			   REG_FLD(1, 29)
#define WED_RX_BM_DYN_ALLOC_CFG_ERR_SUB_UNDERFLOW			   REG_FLD(1, 28)
#define WED_RX_BM_DYN_ALLOC_CFG_SW_SUB_RDY					   REG_FLD(1, 18)
#define WED_RX_BM_DYN_ALLOC_CFG_SW_ADD_BUF_REQ				   REG_FLD(1, 17)
#define WED_RX_BM_DYN_ALLOC_CFG_SW_SUB_BUF_REQ				   REG_FLD(1, 16)
#define WED_RX_BM_DYN_ALLOC_CFG_SW_ADDSUB_IDX_CNT			   REG_FLD(16, 0)

#define WED_RX_BM_ADD_PTR_HEAD2_IDX							   REG_FLD(16, 16)
#define WED_RX_BM_ADD_PTR_SW_ADD_IDX_REMAIN					   REG_FLD(16, 0)

/*copy from CR description*/
#define WED_BMF_VALID_TABLE_START (WED0_REG_BASE + 0x00000C00)
#define WED_BMF_VALID_TABLE_END (WED0_REG_BASE + 0x00000FFC)
#define WED_WDMA_GLO_CFG_FLD_WDMA_BT_SIZE_MASK  0x3
#define WED_WDMA_INT_CTRL_FLD_SRC_SEL_MASK 0x3
#define WED_TX_BM_DYN_TH_FLD_HI_GRP_MASK 0x7f
#define WED_TX_BM_DYN_TH_FLD_LOW_GRP_MASK 0x7f
#define WED_TX_TKID_DYN_TH_FLD_HI_GRP_MASK  0x7f
#define WED_TX_TKID_DYN_TH_FLD_LOW_GRP_MASK  0x7f
#define WED_TX_BM_BLEN_FLD_BYTE_LEN_MASK    0x3fff

#endif /*__WED_HW_*/
