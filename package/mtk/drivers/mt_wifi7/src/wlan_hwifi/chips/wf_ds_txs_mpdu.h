/* SPDX-License-Identifier: <SPDX License Expression> */
#ifndef __WF_DS_TXS_MPDU_REGS_H__
#define __WF_DS_TXS_MPDU_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif








#define WF_DS_TXS_MPDU_BASE                                    0x0

#define WF_DS_TXS_MPDU_DW00_ADDR                               (WF_DS_TXS_MPDU_BASE + 0x0000)
#define WF_DS_TXS_MPDU_DW01_ADDR                               (WF_DS_TXS_MPDU_BASE + 0x0020)
#define WF_DS_TXS_MPDU_DW02_ADDR                               (WF_DS_TXS_MPDU_BASE + 0x0040)
#define WF_DS_TXS_MPDU_DW03_ADDR                               (WF_DS_TXS_MPDU_BASE + 0x0060)
#define WF_DS_TXS_MPDU_DW04_ADDR                               (WF_DS_TXS_MPDU_BASE + 0x0080)
#define WF_DS_TXS_MPDU_DW05_ADDR                               (WF_DS_TXS_MPDU_BASE + 0x00a0)
#define WF_DS_TXS_MPDU_DW06_ADDR                               (WF_DS_TXS_MPDU_BASE + 0x00c0)
#define WF_DS_TXS_MPDU_DW07_ADDR                               (WF_DS_TXS_MPDU_BASE + 0x00e0)
#define WF_DS_TXS_MPDU_VERSION_CODE_ADDR                       (WF_DS_TXS_MPDU_BASE + 0xFFFF)





#define WF_DS_TXS_MPDU_DW00_TBW_ADDR                           WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_TBW_MASK                           0xE0000000
#define WF_DS_TXS_MPDU_DW00_TBW_SHIFT                           29
#define WF_DS_TXS_MPDU_DW00_TID_ADDR                           WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_TID_MASK                           0x1C000000
#define WF_DS_TXS_MPDU_DW00_TID_SHIFT                           26
#define WF_DS_TXS_MPDU_DW00_AM_ADDR                            WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_AM_MASK                            0x02000000
#define WF_DS_TXS_MPDU_DW00_AM_SHIFT                            25
#define WF_DS_TXS_MPDU_DW00_TXSFM_ADDR                         WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_TXSFM_MASK                         0x01800000
#define WF_DS_TXS_MPDU_DW00_TXSFM_SHIFT                         23
#define WF_DS_TXS_MPDU_DW00_BAF_ADDR                           WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_BAF_MASK                           0x00400000
#define WF_DS_TXS_MPDU_DW00_BAF_SHIFT                           22
#define WF_DS_TXS_MPDU_DW00_PS_ADDR                            WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_PS_MASK                            0x00200000
#define WF_DS_TXS_MPDU_DW00_PS_SHIFT                            21
#define WF_DS_TXS_MPDU_DW00_TXOP_ADDR                          WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_TXOP_MASK                          0x00100000
#define WF_DS_TXS_MPDU_DW00_TXOP_SHIFT                          20
#define WF_DS_TXS_MPDU_DW00_BF_BFE_ADDR                        WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_BF_BFE_MASK                        0x00080000
#define WF_DS_TXS_MPDU_DW00_BF_BFE_SHIFT                        19
#define WF_DS_TXS_MPDU_DW00_LE_ADDR                            WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_LE_MASK                            0x00040000
#define WF_DS_TXS_MPDU_DW00_LE_SHIFT                            18
#define WF_DS_TXS_MPDU_DW00_RE_ADDR                            WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_RE_MASK                            0x00020000
#define WF_DS_TXS_MPDU_DW00_RE_SHIFT                            17
#define WF_DS_TXS_MPDU_DW00_ME_ADDR                            WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_ME_MASK                            0x00010000
#define WF_DS_TXS_MPDU_DW00_ME_SHIFT                            16
#define WF_DS_TXS_MPDU_DW00_TXS2H_ADDR                         WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_TXS2H_MASK                         0x00008000
#define WF_DS_TXS_MPDU_DW00_TXS2H_SHIFT                         15
#define WF_DS_TXS_MPDU_DW00_TXS2M_ADDR                         WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_TXS2M_MASK                         0x00004000
#define WF_DS_TXS_MPDU_DW00_TXS2M_SHIFT                         14
#define WF_DS_TXS_MPDU_DW00_RATE_NSTS_ADDR                     WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_RATE_NSTS_MASK                     0x00003C00
#define WF_DS_TXS_MPDU_DW00_RATE_NSTS_SHIFT                     10
#define WF_DS_TXS_MPDU_DW00_RATE_TX_MODE_ADDR                  WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_RATE_TX_MODE_MASK                  0x000003C0
#define WF_DS_TXS_MPDU_DW00_RATE_TX_MODE_SHIFT                  6
#define WF_DS_TXS_MPDU_DW00_RATE_TX_RATE_ADDR                  WF_DS_TXS_MPDU_DW00_ADDR
#define WF_DS_TXS_MPDU_DW00_RATE_TX_RATE_MASK                  0x0000003F
#define WF_DS_TXS_MPDU_DW00_RATE_TX_RATE_SHIFT                  0


#define WF_DS_TXS_MPDU_DW01_SN_ADDR                            WF_DS_TXS_MPDU_DW01_ADDR
#define WF_DS_TXS_MPDU_DW01_SN_MASK                            0xFFF00000
#define WF_DS_TXS_MPDU_DW01_SN_SHIFT                            20
#define WF_DS_TXS_MPDU_DW01_RESP_LEGACY_RATE_ADDR              WF_DS_TXS_MPDU_DW01_ADDR
#define WF_DS_TXS_MPDU_DW01_RESP_LEGACY_RATE_MASK              0x000F0000
#define WF_DS_TXS_MPDU_DW01_RESP_LEGACY_RATE_SHIFT              16
#define WF_DS_TXS_MPDU_DW01_RXV_SN_ADDR                        WF_DS_TXS_MPDU_DW01_ADDR
#define WF_DS_TXS_MPDU_DW01_RXV_SN_MASK                        0x0000FF00
#define WF_DS_TXS_MPDU_DW01_RXV_SN_SHIFT                        8
#define WF_DS_TXS_MPDU_DW01_TX_POWER_ADDR                      WF_DS_TXS_MPDU_DW01_ADDR
#define WF_DS_TXS_MPDU_DW01_TX_POWER_MASK                      0x000000FF
#define WF_DS_TXS_MPDU_DW01_TX_POWER_SHIFT                      0


#define WF_DS_TXS_MPDU_DW02_BF_STS_ADDR                        WF_DS_TXS_MPDU_DW02_ADDR
#define WF_DS_TXS_MPDU_DW02_BF_STS_MASK                        0xC0000000
#define WF_DS_TXS_MPDU_DW02_BF_STS_SHIFT                        30
#define WF_DS_TXS_MPDU_DW02_BAND_ADDR                          WF_DS_TXS_MPDU_DW02_ADDR
#define WF_DS_TXS_MPDU_DW02_BAND_MASK                          0x30000000
#define WF_DS_TXS_MPDU_DW02_BAND_SHIFT                          28
#define WF_DS_TXS_MPDU_DW02_MLD_ID_ADDR                        WF_DS_TXS_MPDU_DW02_ADDR
#define WF_DS_TXS_MPDU_DW02_MLD_ID_MASK                        0x0FFF0000
#define WF_DS_TXS_MPDU_DW02_MLD_ID_SHIFT                        16
#define WF_DS_TXS_MPDU_DW02_TRANSMISSION_DELAY_ADDR            WF_DS_TXS_MPDU_DW02_ADDR
#define WF_DS_TXS_MPDU_DW02_TRANSMISSION_DELAY_MASK            0x0000FFFF
#define WF_DS_TXS_MPDU_DW02_TRANSMISSION_DELAY_SHIFT            0


#define WF_DS_TXS_MPDU_DW03_PID_ADDR                           WF_DS_TXS_MPDU_DW03_ADDR
#define WF_DS_TXS_MPDU_DW03_PID_MASK                           0xFF000000
#define WF_DS_TXS_MPDU_DW03_PID_SHIFT                           24
#define WF_DS_TXS_MPDU_DW03_RATE_STBC_ADDR                     WF_DS_TXS_MPDU_DW03_ADDR
#define WF_DS_TXS_MPDU_DW03_RATE_STBC_MASK                     0x00000080
#define WF_DS_TXS_MPDU_DW03_RATE_STBC_SHIFT                     7
#define WF_DS_TXS_MPDU_DW03_FR_ADDR                            WF_DS_TXS_MPDU_DW03_ADDR
#define WF_DS_TXS_MPDU_DW03_FR_MASK                            0x00000040
#define WF_DS_TXS_MPDU_DW03_FR_SHIFT                            6
#define WF_DS_TXS_MPDU_DW03_SRC_ADDR                           WF_DS_TXS_MPDU_DW03_ADDR
#define WF_DS_TXS_MPDU_DW03_SRC_MASK                           0x00000030
#define WF_DS_TXS_MPDU_DW03_SRC_SHIFT                           4
#define WF_DS_TXS_MPDU_DW03_CO_ANT_ADDR                        WF_DS_TXS_MPDU_DW03_ADDR
#define WF_DS_TXS_MPDU_DW03_CO_ANT_MASK                        0x00000008
#define WF_DS_TXS_MPDU_DW03_CO_ANT_SHIFT                        3
#define WF_DS_TXS_MPDU_DW03_LAST_TX_RATE_IDX_ADDR              WF_DS_TXS_MPDU_DW03_ADDR
#define WF_DS_TXS_MPDU_DW03_LAST_TX_RATE_IDX_MASK              0x00000007
#define WF_DS_TXS_MPDU_DW03_LAST_TX_RATE_IDX_SHIFT              0


#define WF_DS_TXS_MPDU_DW04_TIMESTAMP_ADDR                     WF_DS_TXS_MPDU_DW04_ADDR
#define WF_DS_TXS_MPDU_DW04_TIMESTAMP_MASK                     0xFFFFFFFF
#define WF_DS_TXS_MPDU_DW04_TIMESTAMP_SHIFT                     0


#define WF_DS_TXS_MPDU_DW05_FM_ADDR                            WF_DS_TXS_MPDU_DW05_ADDR
#define WF_DS_TXS_MPDU_DW05_FM_MASK                            0x80000000
#define WF_DS_TXS_MPDU_DW05_FM_SHIFT                            31
#define WF_DS_TXS_MPDU_DW05_QOS_ADDR                           WF_DS_TXS_MPDU_DW05_ADDR
#define WF_DS_TXS_MPDU_DW05_QOS_MASK                           0x40000000
#define WF_DS_TXS_MPDU_DW05_QOS_SHIFT                           30
#define WF_DS_TXS_MPDU_DW05_MPDU_TX_CNT_ADDR                   WF_DS_TXS_MPDU_DW05_ADDR
#define WF_DS_TXS_MPDU_DW05_MPDU_TX_CNT_MASK                   0x3E000000
#define WF_DS_TXS_MPDU_DW05_MPDU_TX_CNT_SHIFT                   25
#define WF_DS_TXS_MPDU_DW05_FRONTTIME_ADDR                     WF_DS_TXS_MPDU_DW05_ADDR
#define WF_DS_TXS_MPDU_DW05_FRONTTIME_MASK                     0x01FFFFFF
#define WF_DS_TXS_MPDU_DW05_FRONTTIME_SHIFT                     0


#define WF_DS_TXS_MPDU_DW06_NOISE3_ADDR                        WF_DS_TXS_MPDU_DW06_ADDR
#define WF_DS_TXS_MPDU_DW06_NOISE3_MASK                        0xFF000000
#define WF_DS_TXS_MPDU_DW06_NOISE3_SHIFT                        24
#define WF_DS_TXS_MPDU_DW06_NOISE2_ADDR                        WF_DS_TXS_MPDU_DW06_ADDR
#define WF_DS_TXS_MPDU_DW06_NOISE2_MASK                        0x00FF0000
#define WF_DS_TXS_MPDU_DW06_NOISE2_SHIFT                        16
#define WF_DS_TXS_MPDU_DW06_NOISE1_ADDR                        WF_DS_TXS_MPDU_DW06_ADDR
#define WF_DS_TXS_MPDU_DW06_NOISE1_MASK                        0x0000FF00
#define WF_DS_TXS_MPDU_DW06_NOISE1_SHIFT                        8
#define WF_DS_TXS_MPDU_DW06_NOISE0_ADDR                        WF_DS_TXS_MPDU_DW06_ADDR
#define WF_DS_TXS_MPDU_DW06_NOISE0_MASK                        0x000000FF
#define WF_DS_TXS_MPDU_DW06_NOISE0_SHIFT                        0


#define WF_DS_TXS_MPDU_DW07_RCPI3_ADDR                         WF_DS_TXS_MPDU_DW07_ADDR
#define WF_DS_TXS_MPDU_DW07_RCPI3_MASK                         0xFF000000
#define WF_DS_TXS_MPDU_DW07_RCPI3_SHIFT                         24
#define WF_DS_TXS_MPDU_DW07_RCPI2_ADDR                         WF_DS_TXS_MPDU_DW07_ADDR
#define WF_DS_TXS_MPDU_DW07_RCPI2_MASK                         0x00FF0000
#define WF_DS_TXS_MPDU_DW07_RCPI2_SHIFT                         16
#define WF_DS_TXS_MPDU_DW07_RCPI1_ADDR                         WF_DS_TXS_MPDU_DW07_ADDR
#define WF_DS_TXS_MPDU_DW07_RCPI1_MASK                         0x0000FF00
#define WF_DS_TXS_MPDU_DW07_RCPI1_SHIFT                         8
#define WF_DS_TXS_MPDU_DW07_RCPI0_ADDR                         WF_DS_TXS_MPDU_DW07_ADDR
#define WF_DS_TXS_MPDU_DW07_RCPI0_MASK                         0x000000FF
#define WF_DS_TXS_MPDU_DW07_RCPI0_SHIFT                         0


#define WF_DS_TXS_MPDU_VERSION_CODE_VERSION_CODE_ADDR          WF_DS_TXS_MPDU_VERSION_CODE_ADDR
#define WF_DS_TXS_MPDU_VERSION_CODE_VERSION_CODE_MASK          0xFFFFFFFF
#define WF_DS_TXS_MPDU_VERSION_CODE_VERSION_CODE_SHIFT          0

#ifdef __cplusplus
}
#endif

#endif // __WF_DS_TXS_MPDU_REGS_H__
