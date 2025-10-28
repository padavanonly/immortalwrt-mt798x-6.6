/* SPDX-License-Identifier: <SPDX License Expression> */
#ifndef __WF_DS_AGG_TXS_REGS_H__
#define __WF_DS_AGG_TXS_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif








#define WF_DS_AGG_TXS_BASE                                     0x0

#define WF_DS_AGG_TXS_DW00_ADDR                                (WF_DS_AGG_TXS_BASE + 0x0000)
#define WF_DS_AGG_TXS_DW01_ADDR                                (WF_DS_AGG_TXS_BASE + 0x0020)
#define WF_DS_AGG_TXS_DW04_ADDR                                (WF_DS_AGG_TXS_BASE + 0x0080)
#define WF_DS_AGG_TXS_VERSION_CODE_ADDR                        (WF_DS_AGG_TXS_BASE + 0xFFFF)





#define WF_DS_AGG_TXS_DW00_PACKET_TYPE_ADDR                    WF_DS_AGG_TXS_DW00_ADDR
#define WF_DS_AGG_TXS_DW00_PACKET_TYPE_MASK                    0xF8000000
#define WF_DS_AGG_TXS_DW00_PACKET_TYPE_SHIFT                    27
#define WF_DS_AGG_TXS_DW00_TXS_CNT_ADDR                        WF_DS_AGG_TXS_DW00_ADDR
#define WF_DS_AGG_TXS_DW00_TXS_CNT_MASK                        0x001F0000
#define WF_DS_AGG_TXS_DW00_TXS_CNT_SHIFT                        16
#define WF_DS_AGG_TXS_DW00_RX_BYTE_COUNT_ADDR                  WF_DS_AGG_TXS_DW00_ADDR
#define WF_DS_AGG_TXS_DW00_RX_BYTE_COUNT_MASK                  0x000007FF
#define WF_DS_AGG_TXS_DW00_RX_BYTE_COUNT_SHIFT                  0


#define WF_DS_AGG_TXS_DW01_PSE_FID_ADDR                        WF_DS_AGG_TXS_DW01_ADDR
#define WF_DS_AGG_TXS_DW01_PSE_FID_MASK                        0x1FFF0000
#define WF_DS_AGG_TXS_DW01_PSE_FID_SHIFT                        16


#define WF_DS_AGG_TXS_DW04_TXS_ADDR                            WF_DS_AGG_TXS_DW04_ADDR
#define WF_DS_AGG_TXS_DW04_TXS_MASK                            0xFFFFFFFF
#define WF_DS_AGG_TXS_DW04_TXS_SHIFT                            0


#define WF_DS_AGG_TXS_VERSION_CODE_VERSION_CODE_ADDR           WF_DS_AGG_TXS_VERSION_CODE_ADDR
#define WF_DS_AGG_TXS_VERSION_CODE_VERSION_CODE_MASK           0xFFFFFFFF
#define WF_DS_AGG_TXS_VERSION_CODE_VERSION_CODE_SHIFT           0

#ifdef __cplusplus
}
#endif

#endif // __WF_DS_AGG_TXS_REGS_H__
