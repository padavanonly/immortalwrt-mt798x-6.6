
#ifndef __CONN_MCU_CFG_LS_REGS_H__
#define __CONN_MCU_CFG_LS_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
//
//                     WOX_MCU_CFG_LS CR Definitions
//
//****************************************************************************

#define WOX_MCU_CFG_LS_BASE                                   0x15590000

#define WOX_MCU_CFG_LS_HW_VER_ADDR                            0x00000000
#define WOX_MCU_CFG_LS_FW_VER_ADDR                            0x00000004
#define WOX_MCU_CFG_LS_WO0_APSRC_ADDR                         0x00000070
#define WOX_MCU_CFG_LS_WOX_GP_IRQ_ADDR                        0x00000080
#define WOX_MCU_CFG_LS_WOX_WED_CTL_ADDR                       0x00000084
#define WOX_MCU_CFG_LS_FPGA_RSV_ADDR                          0x00000100
#define WOX_MCU_CFG_LS_RESERVE0_ADDR                          0x00000150
#define WOX_MCU_CFG_LS_WOX_COM_REG0_ADDR                      0x00000200
#define WOX_MCU_CFG_LS_WOX_COM_REG1_ADDR                      0x00000204
#define WOX_MCU_CFG_LS_WOX_COM_REG2_ADDR                      0x00000208
#define WOX_MCU_CFG_LS_WOX_COM_REG3_ADDR                      0x0000020C
#define WOX_MCU_CFG_LS_WOX_COM_REG4_ADDR                      0x00000210
#define WOX_MCU_CFG_LS_WOX_COM_REG5_ADDR                      0x00000214
#define WOX_MCU_CFG_LS_WOX_COM_REG6_ADDR                      0x00000218
#define WOX_MCU_CFG_LS_WOX_COM_REG7_ADDR                      0x0000021C
#define WOX_MCU_CFG_LS_WOX_COM_REG8_ADDR                      0x00000220
#define WOX_MCU_CFG_LS_WOX_COM_REG9_ADDR                      0x00000224
#define WOX_MCU_CFG_LS_WOX_COM_REG10_ADDR                     0x00000228
#define WOX_MCU_CFG_LS_WOX_COM_REG11_ADDR                     0x0000022C
#define WOX_MCU_CFG_LS_WOX_COM_REG12_ADDR                     0x00000230
#define WOX_MCU_CFG_LS_WOX_COM_REG13_ADDR                     0x00000234
#define WOX_MCU_CFG_LS_WOX_COM_REG14_ADDR                     0x00000238
#define WOX_MCU_CFG_LS_WOX_COM_REG15_ADDR                     0x0000023C
#define WOX_MCU_CFG_LS_WOX_SEMA0_ADDR                         0x00000320
#define WOX_MCU_CFG_LS_WOX_SEMA1_ADDR                         0x00000324
#define WOX_MCU_CFG_LS_WOX_SEMA2_ADDR                         0x00000328
#define WOX_MCU_CFG_LS_WOX_SEMA3_ADDR                         0x0000032C
#define WOX_MCU_CFG_LS_WOX_SEMA4_ADDR                         0x00000330
#define WOX_MCU_CFG_LS_WOX_SEMA5_ADDR                         0x00000334
#define WOX_MCU_CFG_LS_WOX_SEMA6_ADDR                         0x00000338
#define WOX_MCU_CFG_LS_WOX_SEMA7_ADDR                         0x0000033C
#define WOX_MCU_CFG_LS_WOX_SEMA8_ADDR                         0x00000340
#define WOX_MCU_CFG_LS_WOX_SEMA9_ADDR                         0x00000344
#define WOX_MCU_CFG_LS_WOX_SEMA10_ADDR                        0x00000348
#define WOX_MCU_CFG_LS_WOX_SEMA11_ADDR                        0x0000034C
#define WOX_MCU_CFG_LS_WOX_SEMA12_ADDR                        0x00000350
#define WOX_MCU_CFG_LS_WOX_SEMA13_ADDR                        0x00000354
#define WOX_MCU_CFG_LS_WOX_SEMA14_ADDR                        0x00000358
#define WOX_MCU_CFG_LS_WOX_SEMA15_ADDR                        0x0000035C
#define WOX_MCU_CFG_LS_WOX_SEMA16_ADDR                        0x00000360
#define WOX_MCU_CFG_LS_WOX_SEMA17_ADDR                        0x00000364
#define WOX_MCU_CFG_LS_WOX_SEMA18_ADDR                        0x00000368
#define WOX_MCU_CFG_LS_WOX_SEMA19_ADDR                        0x0000036C
#define WOX_MCU_CFG_LS_WOX_SEMA20_ADDR                        0x00000370
#define WOX_MCU_CFG_LS_WOX_SEMA21_ADDR                        0x00000374
#define WOX_MCU_CFG_LS_WOX_SEMA22_ADDR                        0x00000378
#define WOX_MCU_CFG_LS_WOX_SEMA23_ADDR                        0x0000037C
#define WOX_MCU_CFG_LS_WOX_SEMA24_ADDR                        0x00000380
#define WOX_MCU_CFG_LS_WOX_SEMA25_ADDR                        0x00000384
#define WOX_MCU_CFG_LS_WOX_SEMA26_ADDR                        0x00000388
#define WOX_MCU_CFG_LS_WOX_SEMA27_ADDR                        0x0000038C
#define WOX_MCU_CFG_LS_WOX_SEMA28_ADDR                        0x00000390
#define WOX_MCU_CFG_LS_WOX_SEMA29_ADDR                        0x00000394
#define WOX_MCU_CFG_LS_WOX_SEMA30_ADDR                        0x00000398
#define WOX_MCU_CFG_LS_WOX_SEMA31_ADDR                        0x0000039C
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_ADDR                      0x00000430
#define WOX_MCU_CFG_LS_BUSHANGCR_ADDR                         0x00000440
#define WOX_MCU_CFG_LS_BUSHANGADDR_ADDR                       0x00000444
#define WOX_MCU_CFG_LS_BUSHANGID_ADDR                         0x0000044C
#define WOX_MCU_CFG_LS_BUSHANGBUS_ADDR                        0x00000450
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_MON_SEL_ADDR            0x00000910
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_0_ADDR             0x00000914
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_1_ADDR             0x00000918

/* =====================================================================================

  ---HW_VER (0x15590000 + 0x000u)---

    HW_VER[31..0]                - (RO) CONNSYS Version

 =====================================================================================*/
#define WOX_MCU_CFG_LS_HW_VER_HW_VER_ADDR                     WOX_MCU_CFG_LS_HW_VER_ADDR
#define WOX_MCU_CFG_LS_HW_VER_HW_VER_MASK                     0xFFFFFFFF                // HW_VER[31..0]
#define WOX_MCU_CFG_LS_HW_VER_HW_VER_SHFT                     0

/* =====================================================================================

  ---FW_VER (0x15590000 + 0x004u)---

    FW_VER[31..0]                - (RO) Firmware Version

 =====================================================================================*/
#define WOX_MCU_CFG_LS_FW_VER_FW_VER_ADDR                     WOX_MCU_CFG_LS_FW_VER_ADDR
#define WOX_MCU_CFG_LS_FW_VER_FW_VER_MASK                     0xFFFFFFFF                // FW_VER[31..0]
#define WOX_MCU_CFG_LS_FW_VER_FW_VER_SHFT                     0

/* =====================================================================================

  ---WO0_APSRC (0x15590000 + 0x0070u)---

    WO0_APSRC_IDLE[0]            - (RW) WO0 APSRC idle
    WO0_APSRC_REQ[1]             - (RW) WO0 APSRC request
    WO0_APSRC_POWOFF[2]          - (RW) WO0 APSRC power off
    WO0_APSRC_DDREN[3]           - (RW) WO0 APSRC DDREN
    RESERVED4[31..4]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WO0_APSRC_WO0_APSRC_DDREN_ADDR         WOX_MCU_CFG_LS_WO0_APSRC_ADDR
#define WOX_MCU_CFG_LS_WO0_APSRC_WO0_APSRC_DDREN_MASK         0x00000008                // WO0_APSRC_DDREN[3]
#define WOX_MCU_CFG_LS_WO0_APSRC_WO0_APSRC_DDREN_SHFT         3
#define WOX_MCU_CFG_LS_WO0_APSRC_WO0_APSRC_POWOFF_ADDR        WOX_MCU_CFG_LS_WO0_APSRC_ADDR
#define WOX_MCU_CFG_LS_WO0_APSRC_WO0_APSRC_POWOFF_MASK        0x00000004                // WO0_APSRC_POWOFF[2]
#define WOX_MCU_CFG_LS_WO0_APSRC_WO0_APSRC_POWOFF_SHFT        2
#define WOX_MCU_CFG_LS_WO0_APSRC_WO0_APSRC_REQ_ADDR           WOX_MCU_CFG_LS_WO0_APSRC_ADDR
#define WOX_MCU_CFG_LS_WO0_APSRC_WO0_APSRC_REQ_MASK           0x00000002                // WO0_APSRC_REQ[1]
#define WOX_MCU_CFG_LS_WO0_APSRC_WO0_APSRC_REQ_SHFT           1
#define WOX_MCU_CFG_LS_WO0_APSRC_WO0_APSRC_IDLE_ADDR          WOX_MCU_CFG_LS_WO0_APSRC_ADDR
#define WOX_MCU_CFG_LS_WO0_APSRC_WO0_APSRC_IDLE_MASK          0x00000001                // WO0_APSRC_IDLE[0]
#define WOX_MCU_CFG_LS_WO0_APSRC_WO0_APSRC_IDLE_SHFT          0

/* =====================================================================================

  ---WOX_GP_IRQ (0x15590000 + 0x080u)---

    WO_GP_IRQ_B[0]               - (RW) WO0 CPU GP_IRQ_B (low active)
                                     0: active
                                     1: inactive
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_GP_IRQ_WO_GP_IRQ_B_ADDR            WOX_MCU_CFG_LS_WOX_GP_IRQ_ADDR
#define WOX_MCU_CFG_LS_WOX_GP_IRQ_WO_GP_IRQ_B_MASK            0x00000001                // WO_GP_IRQ_B[0]
#define WOX_MCU_CFG_LS_WOX_GP_IRQ_WO_GP_IRQ_B_SHFT            0

/* =====================================================================================

  ---WOX_WED_CTL (0x15590000 + 0x084u)---

    WO_WED_IRQ_POL[0]            - (RW) WO0 CPU WED IRQ polarity configure
                                     0: low active
                                     1: high active
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_WED_CTL_WO_WED_IRQ_POL_ADDR        WOX_MCU_CFG_LS_WOX_WED_CTL_ADDR
#define WOX_MCU_CFG_LS_WOX_WED_CTL_WO_WED_IRQ_POL_MASK        0x00000001                // WO_WED_IRQ_POL[0]
#define WOX_MCU_CFG_LS_WOX_WED_CTL_WO_WED_IRQ_POL_SHFT        0

/* =====================================================================================

  ---FPGA_RSV (0x15590000 + 0x100u)---

    FPGA_RSV[31..0]              - (RW) FPGA reserve register

 =====================================================================================*/
#define WOX_MCU_CFG_LS_FPGA_RSV_FPGA_RSV_ADDR                 WOX_MCU_CFG_LS_FPGA_RSV_ADDR
#define WOX_MCU_CFG_LS_FPGA_RSV_FPGA_RSV_MASK                 0xFFFFFFFF                // FPGA_RSV[31..0]
#define WOX_MCU_CFG_LS_FPGA_RSV_FPGA_RSV_SHFT                 0

/* =====================================================================================

  ---RESERVE0 (0x15590000 + 0x150u)---

    RESERVE0[31..0]              - (RW) RESERVE0

 =====================================================================================*/
#define WOX_MCU_CFG_LS_RESERVE0_RESERVE0_ADDR                 WOX_MCU_CFG_LS_RESERVE0_ADDR
#define WOX_MCU_CFG_LS_RESERVE0_RESERVE0_MASK                 0xFFFFFFFF                // RESERVE0[31..0]
#define WOX_MCU_CFG_LS_RESERVE0_RESERVE0_SHFT                 0

/* =====================================================================================

  ---WOX_COM_REG0 (0x15590000 + 0x200u)---

    WOX_CON_REG0[31..0]          - (RW) WOX_MCUSYS COMMON register no.0

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG0_WOX_CON_REG0_ADDR         WOX_MCU_CFG_LS_WOX_COM_REG0_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG0_WOX_CON_REG0_MASK         0xFFFFFFFF                // WOX_CON_REG0[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG0_WOX_CON_REG0_SHFT         0

/* =====================================================================================

  ---WOX_COM_REG1 (0x15590000 + 0x204u)---

    WOX_CON_REG1[31..0]          - (RW) WOX_MCUSYS COMMON register no.1

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG1_WOX_CON_REG1_ADDR         WOX_MCU_CFG_LS_WOX_COM_REG1_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG1_WOX_CON_REG1_MASK         0xFFFFFFFF                // WOX_CON_REG1[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG1_WOX_CON_REG1_SHFT         0

/* =====================================================================================

  ---WOX_COM_REG2 (0x15590000 + 0x208u)---

    WOX_CON_REG2[31..0]          - (RW) WOX_MCUSYS COMMON register no.2

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG2_WOX_CON_REG2_ADDR         WOX_MCU_CFG_LS_WOX_COM_REG2_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG2_WOX_CON_REG2_MASK         0xFFFFFFFF                // WOX_CON_REG2[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG2_WOX_CON_REG2_SHFT         0

/* =====================================================================================

  ---WOX_COM_REG3 (0x15590000 + 0x20Cu)---

    WOX_CON_REG3[31..0]          - (RW) WOX_MCUSYS COMMON register no.3

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG3_WOX_CON_REG3_ADDR         WOX_MCU_CFG_LS_WOX_COM_REG3_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG3_WOX_CON_REG3_MASK         0xFFFFFFFF                // WOX_CON_REG3[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG3_WOX_CON_REG3_SHFT         0

/* =====================================================================================

  ---WOX_COM_REG4 (0x15590000 + 0x210u)---

    WOX_CON_REG4[31..0]          - (RW) WOX_MCUSYS COMMON register no.4

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG4_WOX_CON_REG4_ADDR         WOX_MCU_CFG_LS_WOX_COM_REG4_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG4_WOX_CON_REG4_MASK         0xFFFFFFFF                // WOX_CON_REG4[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG4_WOX_CON_REG4_SHFT         0

/* =====================================================================================

  ---WOX_COM_REG5 (0x15590000 + 0x214u)---

    WOX_CON_REG5[31..0]          - (RW) WOX_MCUSYS COMMON register no.5

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG5_WOX_CON_REG5_ADDR         WOX_MCU_CFG_LS_WOX_COM_REG5_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG5_WOX_CON_REG5_MASK         0xFFFFFFFF                // WOX_CON_REG5[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG5_WOX_CON_REG5_SHFT         0

/* =====================================================================================

  ---WOX_COM_REG6 (0x15590000 + 0x218u)---

    WOX_CON_REG6[31..0]          - (RW) WOX_MCUSYS COMMON register no.6

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG6_WOX_CON_REG6_ADDR         WOX_MCU_CFG_LS_WOX_COM_REG6_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG6_WOX_CON_REG6_MASK         0xFFFFFFFF                // WOX_CON_REG6[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG6_WOX_CON_REG6_SHFT         0

/* =====================================================================================

  ---WOX_COM_REG7 (0x15590000 + 0x21Cu)---

    WOX_CON_REG7[31..0]          - (RW) WOX_MCUSYS COMMON register no.7

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG7_WOX_CON_REG7_ADDR         WOX_MCU_CFG_LS_WOX_COM_REG7_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG7_WOX_CON_REG7_MASK         0xFFFFFFFF                // WOX_CON_REG7[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG7_WOX_CON_REG7_SHFT         0

/* =====================================================================================

  ---WOX_COM_REG8 (0x15590000 + 0x220u)---

    WOX_CON_REG8[31..0]          - (RW) WOX_MCUSYS COMMON register no.8

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG8_WOX_CON_REG8_ADDR         WOX_MCU_CFG_LS_WOX_COM_REG8_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG8_WOX_CON_REG8_MASK         0xFFFFFFFF                // WOX_CON_REG8[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG8_WOX_CON_REG8_SHFT         0

/* =====================================================================================

  ---WOX_COM_REG9 (0x15590000 + 0x224u)---

    WOX_CON_REG9[31..0]          - (RW) WOX_MCUSYS COMMON register no.9

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG9_WOX_CON_REG9_ADDR         WOX_MCU_CFG_LS_WOX_COM_REG9_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG9_WOX_CON_REG9_MASK         0xFFFFFFFF                // WOX_CON_REG9[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG9_WOX_CON_REG9_SHFT         0

/* =====================================================================================

  ---WOX_COM_REG10 (0x15590000 + 0x228u)---

    WOX_CON_REG10[31..0]         - (RW) WOX_MCUSYS COMMON register no.10

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG10_WOX_CON_REG10_ADDR       WOX_MCU_CFG_LS_WOX_COM_REG10_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG10_WOX_CON_REG10_MASK       0xFFFFFFFF                // WOX_CON_REG10[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG10_WOX_CON_REG10_SHFT       0

/* =====================================================================================

  ---WOX_COM_REG11 (0x15590000 + 0x22Cu)---

    WOX_CON_REG11[31..0]         - (RW) WOX_MCUSYS COMMON register no.11

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG11_WOX_CON_REG11_ADDR       WOX_MCU_CFG_LS_WOX_COM_REG11_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG11_WOX_CON_REG11_MASK       0xFFFFFFFF                // WOX_CON_REG11[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG11_WOX_CON_REG11_SHFT       0

/* =====================================================================================

  ---WOX_COM_REG12 (0x15590000 + 0x230u)---

    WOX_CON_REG12[31..0]         - (RW) WOX_MCUSYS COMMON register no.12

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG12_WOX_CON_REG12_ADDR       WOX_MCU_CFG_LS_WOX_COM_REG12_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG12_WOX_CON_REG12_MASK       0xFFFFFFFF                // WOX_CON_REG12[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG12_WOX_CON_REG12_SHFT       0

/* =====================================================================================

  ---WOX_COM_REG13 (0x15590000 + 0x234u)---

    WOX_CON_REG13[31..0]         - (RW) WOX_MCUSYS COMMON register no.13

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG13_WOX_CON_REG13_ADDR       WOX_MCU_CFG_LS_WOX_COM_REG13_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG13_WOX_CON_REG13_MASK       0xFFFFFFFF                // WOX_CON_REG13[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG13_WOX_CON_REG13_SHFT       0

/* =====================================================================================

  ---WOX_COM_REG14 (0x15590000 + 0x238u)---

    WOX_CON_REG14[31..0]         - (RW) WOX_MCUSYS COMMON register no.14

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG14_WOX_CON_REG14_ADDR       WOX_MCU_CFG_LS_WOX_COM_REG14_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG14_WOX_CON_REG14_MASK       0xFFFFFFFF                // WOX_CON_REG14[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG14_WOX_CON_REG14_SHFT       0

/* =====================================================================================

  ---WOX_COM_REG15 (0x15590000 + 0x23Cu)---

    WOX_CON_REG15[31..0]         - (RW) WOX_MCUSYS COMMON register no.15

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG15_WOX_CON_REG15_ADDR       WOX_MCU_CFG_LS_WOX_COM_REG15_ADDR
#define WOX_MCU_CFG_LS_WOX_COM_REG15_WOX_CON_REG15_MASK       0xFFFFFFFF                // WOX_CON_REG15[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG15_WOX_CON_REG15_SHFT       0

/* =====================================================================================

  ---WOX_SEMA0 (0x15590000 + 0x320u)---

    WOX_SEMA0[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA0_WOX_SEMA0_ADDR               WOX_MCU_CFG_LS_WOX_SEMA0_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA0_WOX_SEMA0_MASK               0x00000001                // WOX_SEMA0[0]
#define WOX_MCU_CFG_LS_WOX_SEMA0_WOX_SEMA0_SHFT               0

/* =====================================================================================

  ---WOX_SEMA1 (0x15590000 + 0x324u)---

    WOX_SEMA1[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA1_WOX_SEMA1_ADDR               WOX_MCU_CFG_LS_WOX_SEMA1_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA1_WOX_SEMA1_MASK               0x00000001                // WOX_SEMA1[0]
#define WOX_MCU_CFG_LS_WOX_SEMA1_WOX_SEMA1_SHFT               0

/* =====================================================================================

  ---WOX_SEMA2 (0x15590000 + 0x328u)---

    WOX_SEMA2[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA2_WOX_SEMA2_ADDR               WOX_MCU_CFG_LS_WOX_SEMA2_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA2_WOX_SEMA2_MASK               0x00000001                // WOX_SEMA2[0]
#define WOX_MCU_CFG_LS_WOX_SEMA2_WOX_SEMA2_SHFT               0

/* =====================================================================================

  ---WOX_SEMA3 (0x15590000 + 0x32Cu)---

    WOX_SEMA3[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA3_WOX_SEMA3_ADDR               WOX_MCU_CFG_LS_WOX_SEMA3_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA3_WOX_SEMA3_MASK               0x00000001                // WOX_SEMA3[0]
#define WOX_MCU_CFG_LS_WOX_SEMA3_WOX_SEMA3_SHFT               0

/* =====================================================================================

  ---WOX_SEMA4 (0x15590000 + 0x330u)---

    WOX_SEMA4[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA4_WOX_SEMA4_ADDR               WOX_MCU_CFG_LS_WOX_SEMA4_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA4_WOX_SEMA4_MASK               0x00000001                // WOX_SEMA4[0]
#define WOX_MCU_CFG_LS_WOX_SEMA4_WOX_SEMA4_SHFT               0

/* =====================================================================================

  ---WOX_SEMA5 (0x15590000 + 0x334u)---

    WOX_SEMA5[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA5_WOX_SEMA5_ADDR               WOX_MCU_CFG_LS_WOX_SEMA5_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA5_WOX_SEMA5_MASK               0x00000001                // WOX_SEMA5[0]
#define WOX_MCU_CFG_LS_WOX_SEMA5_WOX_SEMA5_SHFT               0

/* =====================================================================================

  ---WOX_SEMA6 (0x15590000 + 0x338u)---

    WOX_SEMA6[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA6_WOX_SEMA6_ADDR               WOX_MCU_CFG_LS_WOX_SEMA6_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA6_WOX_SEMA6_MASK               0x00000001                // WOX_SEMA6[0]
#define WOX_MCU_CFG_LS_WOX_SEMA6_WOX_SEMA6_SHFT               0

/* =====================================================================================

  ---WOX_SEMA7 (0x15590000 + 0x33Cu)---

    WOX_SEMA7[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA7_WOX_SEMA7_ADDR               WOX_MCU_CFG_LS_WOX_SEMA7_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA7_WOX_SEMA7_MASK               0x00000001                // WOX_SEMA7[0]
#define WOX_MCU_CFG_LS_WOX_SEMA7_WOX_SEMA7_SHFT               0

/* =====================================================================================

  ---WOX_SEMA8 (0x15590000 + 0x340u)---

    WOX_SEMA8[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA8_WOX_SEMA8_ADDR               WOX_MCU_CFG_LS_WOX_SEMA8_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA8_WOX_SEMA8_MASK               0x00000001                // WOX_SEMA8[0]
#define WOX_MCU_CFG_LS_WOX_SEMA8_WOX_SEMA8_SHFT               0

/* =====================================================================================

  ---WOX_SEMA9 (0x15590000 + 0x344u)---

    WOX_SEMA9[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA9_WOX_SEMA9_ADDR               WOX_MCU_CFG_LS_WOX_SEMA9_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA9_WOX_SEMA9_MASK               0x00000001                // WOX_SEMA9[0]
#define WOX_MCU_CFG_LS_WOX_SEMA9_WOX_SEMA9_SHFT               0

/* =====================================================================================

  ---WOX_SEMA10 (0x15590000 + 0x348u)---

    WOX_SEMA10[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA10_WOX_SEMA10_ADDR             WOX_MCU_CFG_LS_WOX_SEMA10_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA10_WOX_SEMA10_MASK             0x00000001                // WOX_SEMA10[0]
#define WOX_MCU_CFG_LS_WOX_SEMA10_WOX_SEMA10_SHFT             0

/* =====================================================================================

  ---WOX_SEMA11 (0x15590000 + 0x34Cu)---

    WOX_SEMA11[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA11_WOX_SEMA11_ADDR             WOX_MCU_CFG_LS_WOX_SEMA11_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA11_WOX_SEMA11_MASK             0x00000001                // WOX_SEMA11[0]
#define WOX_MCU_CFG_LS_WOX_SEMA11_WOX_SEMA11_SHFT             0

/* =====================================================================================

  ---WOX_SEMA12 (0x15590000 + 0x350u)---

    WOX_SEMA12[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA12_WOX_SEMA12_ADDR             WOX_MCU_CFG_LS_WOX_SEMA12_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA12_WOX_SEMA12_MASK             0x00000001                // WOX_SEMA12[0]
#define WOX_MCU_CFG_LS_WOX_SEMA12_WOX_SEMA12_SHFT             0

/* =====================================================================================

  ---WOX_SEMA13 (0x15590000 + 0x354u)---

    WOX_SEMA13[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA13_WOX_SEMA13_ADDR             WOX_MCU_CFG_LS_WOX_SEMA13_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA13_WOX_SEMA13_MASK             0x00000001                // WOX_SEMA13[0]
#define WOX_MCU_CFG_LS_WOX_SEMA13_WOX_SEMA13_SHFT             0

/* =====================================================================================

  ---WOX_SEMA14 (0x15590000 + 0x358u)---

    WOX_SEMA14[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA14_WOX_SEMA14_ADDR             WOX_MCU_CFG_LS_WOX_SEMA14_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA14_WOX_SEMA14_MASK             0x00000001                // WOX_SEMA14[0]
#define WOX_MCU_CFG_LS_WOX_SEMA14_WOX_SEMA14_SHFT             0

/* =====================================================================================

  ---WOX_SEMA15 (0x15590000 + 0x35Cu)---

    WOX_SEMA15[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA15_WOX_SEMA15_ADDR             WOX_MCU_CFG_LS_WOX_SEMA15_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA15_WOX_SEMA15_MASK             0x00000001                // WOX_SEMA15[0]
#define WOX_MCU_CFG_LS_WOX_SEMA15_WOX_SEMA15_SHFT             0

/* =====================================================================================

  ---WOX_SEMA16 (0x15590000 + 0x360u)---

    WOX_SEMA16[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA16_WOX_SEMA16_ADDR             WOX_MCU_CFG_LS_WOX_SEMA16_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA16_WOX_SEMA16_MASK             0x00000001                // WOX_SEMA16[0]
#define WOX_MCU_CFG_LS_WOX_SEMA16_WOX_SEMA16_SHFT             0

/* =====================================================================================

  ---WOX_SEMA17 (0x15590000 + 0x364u)---

    WOX_SEMA17[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA17_WOX_SEMA17_ADDR             WOX_MCU_CFG_LS_WOX_SEMA17_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA17_WOX_SEMA17_MASK             0x00000001                // WOX_SEMA17[0]
#define WOX_MCU_CFG_LS_WOX_SEMA17_WOX_SEMA17_SHFT             0

/* =====================================================================================

  ---WOX_SEMA18 (0x15590000 + 0x368u)---

    WOX_SEMA18[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA18_WOX_SEMA18_ADDR             WOX_MCU_CFG_LS_WOX_SEMA18_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA18_WOX_SEMA18_MASK             0x00000001                // WOX_SEMA18[0]
#define WOX_MCU_CFG_LS_WOX_SEMA18_WOX_SEMA18_SHFT             0

/* =====================================================================================

  ---WOX_SEMA19 (0x15590000 + 0x36Cu)---

    WOX_SEMA19[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA19_WOX_SEMA19_ADDR             WOX_MCU_CFG_LS_WOX_SEMA19_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA19_WOX_SEMA19_MASK             0x00000001                // WOX_SEMA19[0]
#define WOX_MCU_CFG_LS_WOX_SEMA19_WOX_SEMA19_SHFT             0

/* =====================================================================================

  ---WOX_SEMA20 (0x15590000 + 0x370u)---

    WOX_SEMA20[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA20_WOX_SEMA20_ADDR             WOX_MCU_CFG_LS_WOX_SEMA20_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA20_WOX_SEMA20_MASK             0x00000001                // WOX_SEMA20[0]
#define WOX_MCU_CFG_LS_WOX_SEMA20_WOX_SEMA20_SHFT             0

/* =====================================================================================

  ---WOX_SEMA21 (0x15590000 + 0x374u)---

    WOX_SEMA21[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA21_WOX_SEMA21_ADDR             WOX_MCU_CFG_LS_WOX_SEMA21_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA21_WOX_SEMA21_MASK             0x00000001                // WOX_SEMA21[0]
#define WOX_MCU_CFG_LS_WOX_SEMA21_WOX_SEMA21_SHFT             0

/* =====================================================================================

  ---WOX_SEMA22 (0x15590000 + 0x378u)---

    WOX_SEMA22[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA22_WOX_SEMA22_ADDR             WOX_MCU_CFG_LS_WOX_SEMA22_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA22_WOX_SEMA22_MASK             0x00000001                // WOX_SEMA22[0]
#define WOX_MCU_CFG_LS_WOX_SEMA22_WOX_SEMA22_SHFT             0

/* =====================================================================================

  ---WOX_SEMA23 (0x15590000 + 0x37Cu)---

    WOX_SEMA23[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA23_WOX_SEMA23_ADDR             WOX_MCU_CFG_LS_WOX_SEMA23_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA23_WOX_SEMA23_MASK             0x00000001                // WOX_SEMA23[0]
#define WOX_MCU_CFG_LS_WOX_SEMA23_WOX_SEMA23_SHFT             0

/* =====================================================================================

  ---WOX_SEMA24 (0x15590000 + 0x380u)---

    WOX_SEMA24[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA24_WOX_SEMA24_ADDR             WOX_MCU_CFG_LS_WOX_SEMA24_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA24_WOX_SEMA24_MASK             0x00000001                // WOX_SEMA24[0]
#define WOX_MCU_CFG_LS_WOX_SEMA24_WOX_SEMA24_SHFT             0

/* =====================================================================================

  ---WOX_SEMA25 (0x15590000 + 0x384u)---

    WOX_SEMA25[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA25_WOX_SEMA25_ADDR             WOX_MCU_CFG_LS_WOX_SEMA25_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA25_WOX_SEMA25_MASK             0x00000001                // WOX_SEMA25[0]
#define WOX_MCU_CFG_LS_WOX_SEMA25_WOX_SEMA25_SHFT             0

/* =====================================================================================

  ---WOX_SEMA26 (0x15590000 + 0x388u)---

    WOX_SEMA26[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA26_WOX_SEMA26_ADDR             WOX_MCU_CFG_LS_WOX_SEMA26_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA26_WOX_SEMA26_MASK             0x00000001                // WOX_SEMA26[0]
#define WOX_MCU_CFG_LS_WOX_SEMA26_WOX_SEMA26_SHFT             0

/* =====================================================================================

  ---WOX_SEMA27 (0x15590000 + 0x38Cu)---

    WOX_SEMA27[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA27_WOX_SEMA27_ADDR             WOX_MCU_CFG_LS_WOX_SEMA27_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA27_WOX_SEMA27_MASK             0x00000001                // WOX_SEMA27[0]
#define WOX_MCU_CFG_LS_WOX_SEMA27_WOX_SEMA27_SHFT             0

/* =====================================================================================

  ---WOX_SEMA28 (0x15590000 + 0x390u)---

    WOX_SEMA28[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA28_WOX_SEMA28_ADDR             WOX_MCU_CFG_LS_WOX_SEMA28_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA28_WOX_SEMA28_MASK             0x00000001                // WOX_SEMA28[0]
#define WOX_MCU_CFG_LS_WOX_SEMA28_WOX_SEMA28_SHFT             0

/* =====================================================================================

  ---WOX_SEMA29 (0x15590000 + 0x394u)---

    WOX_SEMA29[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA29_WOX_SEMA29_ADDR             WOX_MCU_CFG_LS_WOX_SEMA29_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA29_WOX_SEMA29_MASK             0x00000001                // WOX_SEMA29[0]
#define WOX_MCU_CFG_LS_WOX_SEMA29_WOX_SEMA29_SHFT             0

/* =====================================================================================

  ---WOX_SEMA30 (0x15590000 + 0x398u)---

    WOX_SEMA30[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA30_WOX_SEMA30_ADDR             WOX_MCU_CFG_LS_WOX_SEMA30_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA30_WOX_SEMA30_MASK             0x00000001                // WOX_SEMA30[0]
#define WOX_MCU_CFG_LS_WOX_SEMA30_WOX_SEMA30_SHFT             0

/* =====================================================================================

  ---WOX_SEMA31 (0x15590000 + 0x39Cu)---

    WOX_SEMA31[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA31_WOX_SEMA31_ADDR             WOX_MCU_CFG_LS_WOX_SEMA31_ADDR
#define WOX_MCU_CFG_LS_WOX_SEMA31_WOX_SEMA31_MASK             0x00000001                // WOX_SEMA31[0]
#define WOX_MCU_CFG_LS_WOX_SEMA31_WOX_SEMA31_SHFT             0

/* =====================================================================================

  ---BUSHANGCTRLA (0x15590000 + 0x0430u)---

    RESERVED0[3..0]              - (RO) Reserved bits
    BUS_HANG_HBURST[6..4]        - (RO) bus hang hburst
    BUS_HANG_WRITE[7]            - (RO) bus hang hwrite/pwrite
    BUS_HANG_HTRANS[9..8]        - (RO) bus hang htrans
    RESERVED10[31..10]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_HTRANS_ADDR      WOX_MCU_CFG_LS_BUSHANGCTRLA_ADDR
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_HTRANS_MASK      0x00000300                // BUS_HANG_HTRANS[9..8]
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_HTRANS_SHFT      8
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_WRITE_ADDR       WOX_MCU_CFG_LS_BUSHANGCTRLA_ADDR
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_WRITE_MASK       0x00000080                // BUS_HANG_WRITE[7]
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_WRITE_SHFT       7
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_HBURST_ADDR      WOX_MCU_CFG_LS_BUSHANGCTRLA_ADDR
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_HBURST_MASK      0x00000070                // BUS_HANG_HBURST[6..4]
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_HBURST_SHFT      4

/* =====================================================================================

  ---BUSHANGCR (0x15590000 + 0x0440u)---

    BUS_HANG_TIME_LIMIT[7..0]    - (RW) Internal bus debug time limit
                                     The total hanging cycle count is a 32-bit value. BUS_HANG_TIME_LIMIT specifies the MSB of total hanging cycle count, that is:
                                     Total hanging cycle count = { BUS_HANG_TIME_LIMIT, 0x80000}
                                     For example:
                                     If BUS_HANG_TIME_LIMIT was set to 0x10, then total hanging cycle count will be 0x1080000, which means when bus was hanging for 0x1080000 cycles, an interrupt will be triggered
    RESERVED8[28..8]             - (RO) Reserved bits
    BUS_HANG_IRQ_CLR[29]         - (RW)  xxx
    BUS_HANG_SLV_HREADY_SEL[30]  - (RW) Internal bus hang force slv_hready selection
                                     1'b0: revise ahb slave hready
                                     1'b1: force bus layer
    BUS_HANG_DEBUG_EN[31]        - (RW) Internal bus debug enable

 =====================================================================================*/
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_DEBUG_EN_ADDR       WOX_MCU_CFG_LS_BUSHANGCR_ADDR
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_DEBUG_EN_MASK       0x80000000                // BUS_HANG_DEBUG_EN[31]
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_DEBUG_EN_SHFT       31
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_SLV_HREADY_SEL_ADDR WOX_MCU_CFG_LS_BUSHANGCR_ADDR
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_SLV_HREADY_SEL_MASK 0x40000000                // BUS_HANG_SLV_HREADY_SEL[30]
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_SLV_HREADY_SEL_SHFT 30
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_IRQ_CLR_ADDR        WOX_MCU_CFG_LS_BUSHANGCR_ADDR
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_IRQ_CLR_MASK        0x20000000                // BUS_HANG_IRQ_CLR[29]
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_IRQ_CLR_SHFT        29
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_TIME_LIMIT_ADDR     WOX_MCU_CFG_LS_BUSHANGCR_ADDR
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_TIME_LIMIT_MASK     0x000000FF                // BUS_HANG_TIME_LIMIT[7..0]
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_TIME_LIMIT_SHFT     0

/* =====================================================================================

  ---BUSHANGADDR (0x15590000 + 0x0444u)---

    AHB_TIMEOUT_HADDR_LATCH[31..0] - (RO) Internal bus hang address

 =====================================================================================*/
#define WOX_MCU_CFG_LS_BUSHANGADDR_AHB_TIMEOUT_HADDR_LATCH_ADDR WOX_MCU_CFG_LS_BUSHANGADDR_ADDR
#define WOX_MCU_CFG_LS_BUSHANGADDR_AHB_TIMEOUT_HADDR_LATCH_MASK 0xFFFFFFFF                // AHB_TIMEOUT_HADDR_LATCH[31..0]
#define WOX_MCU_CFG_LS_BUSHANGADDR_AHB_TIMEOUT_HADDR_LATCH_SHFT 0

/* =====================================================================================

  ---BUSHANGID (0x15590000 + 0x044Cu)---

    BUS_HANG_ID[9..0]            - (RO) bus master id record when bus hang
    RESERVED10[31..10]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_BUSHANGID_BUS_HANG_ID_ADDR             WOX_MCU_CFG_LS_BUSHANGID_ADDR
#define WOX_MCU_CFG_LS_BUSHANGID_BUS_HANG_ID_MASK             0x000003FF                // BUS_HANG_ID[9..0]
#define WOX_MCU_CFG_LS_BUSHANGID_BUS_HANG_ID_SHFT             0

/* =====================================================================================

  ---BUSHANGBUS (0x15590000 + 0x0450u)---

    BUS_HANG_BUS[31..0]          - (RO) indicate which bus is hang,
                                     [0]: s33, aximon
                                     [1]: s35, axidma
                                     [2]: s18, WOX_TOP_MISC_OFF
                                     [3]: s10, mcu_cfg_on
                                     [4]: s11, WO0_cirq
                                     [5]: s12, WO0_gpt
                                     [6]: s13, WO0_wdt
                                     [7]: s14, WO1_cirq
                                     [8]: s15, WO1_gpt
                                     [9]: s16, WO1_wdt

 =====================================================================================*/
#define WOX_MCU_CFG_LS_BUSHANGBUS_BUS_HANG_BUS_ADDR           WOX_MCU_CFG_LS_BUSHANGBUS_ADDR
#define WOX_MCU_CFG_LS_BUSHANGBUS_BUS_HANG_BUS_MASK           0xFFFFFFFF                // BUS_HANG_BUS[31..0]
#define WOX_MCU_CFG_LS_BUSHANGBUS_BUS_HANG_BUS_SHFT           0

/* =====================================================================================

  ---VDNR_BUS_DEBUG_MON_SEL (0x15590000 + 0x0910u)---

    VDNR_BUS_DEBUG_MON_SEL[31..0] - (RW) BUS VDNR debug mon select control

 =====================================================================================*/
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_MON_SEL_VDNR_BUS_DEBUG_MON_SEL_ADDR WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_MON_SEL_ADDR
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_MON_SEL_VDNR_BUS_DEBUG_MON_SEL_MASK 0xFFFFFFFF                // VDNR_BUS_DEBUG_MON_SEL[31..0]
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_MON_SEL_VDNR_BUS_DEBUG_MON_SEL_SHFT 0

/* =====================================================================================

  ---VDNR_BUS_DEBUG_CTRL_0 (0x15590000 + 0x0914u)---

    VDNR_DEBUG_CTRL_AO_DEBUGSYS_CTRL[31..0] - (RW) VDNR_DEBUG_CTRL_AO_DEBUGSYS_CTRL

 =====================================================================================*/
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_0_VDNR_DEBUG_CTRL_AO_DEBUGSYS_CTRL_ADDR WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_0_ADDR
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_0_VDNR_DEBUG_CTRL_AO_DEBUGSYS_CTRL_MASK 0xFFFFFFFF                // VDNR_DEBUG_CTRL_AO_DEBUGSYS_CTRL[31..0]
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_0_VDNR_DEBUG_CTRL_AO_DEBUGSYS_CTRL_SHFT 0

/* =====================================================================================

  ---VDNR_BUS_DEBUG_CTRL_1 (0x15590000 + 0x0918u)---

    VDNR_DEBUG_CTRL_AO_IO_CTRL[7..0] - (RW) VDNR_DEBUG_CTRL_AO_IO_CTRL
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_1_VDNR_DEBUG_CTRL_AO_IO_CTRL_ADDR WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_1_ADDR
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_1_VDNR_DEBUG_CTRL_AO_IO_CTRL_MASK 0x000000FF                // VDNR_DEBUG_CTRL_AO_IO_CTRL[7..0]
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_1_VDNR_DEBUG_CTRL_AO_IO_CTRL_SHFT 0

#ifdef __cplusplus
}
#endif

#endif // __CONN_MCU_CFG_LS_REGS_H__
