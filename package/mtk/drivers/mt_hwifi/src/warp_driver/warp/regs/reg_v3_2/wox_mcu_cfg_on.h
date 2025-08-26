
#ifndef __WOX_MCU_CFG_ON_REGS_H__
#define __WOX_MCU_CFG_ON_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
//
//                     WOX_MCU_CFG_ON CR Definitions
//
//****************************************************************************

#define WOX_MCU_CFG_ON_BASE                                   0x15596000

#define WOX_MCU_CFG_ON_DBG_CTL_ADDR                           0x0000008
#define WOX_MCU_CFG_ON_CFG_DBG1_ADDR                          0x000000C
#define WOX_MCU_CFG_ON_CFG_DBG2_ADDR                          0x0000010
#define WOX_MCU_CFG_ON_WOX_MCCR_ADDR                          0x0000014
#define WOX_MCU_CFG_ON_WOX_MCCR_SET_ADDR                      0x0000018
#define WOX_MCU_CFG_ON_WOX_MCCR_CLR_ADDR                      0x000001C
#define WOX_MCU_CFG_ON_DBG_EINT_ADDR                          0x0000020
#define WOX_MCU_CFG_ON_WOX_MCU_CFG_WO0_WO1_ADDR               0x0000050
#define WOX_MCU_CFG_ON_MCSR_ADDR                              0x0000010C
#define WOX_MCU_CFG_ON_WOX_MCU_CG_ADDR                        0x0000029C
#define WOX_MCU_CFG_ON_DBGSR_ADDR                             0x0000400
#define WOX_MCU_CFG_ON_DBGSR1_ADDR                            0x0000404
#define WOX_MCU_CFG_ON_DBGSR2_ADDR                            0x0000408
#define WOX_MCU_CFG_ON_WO2NETSYS_AID_ADDR                     0x0000500

/* =====================================================================================

  ---DBG_CTL (0x15596000 + 0x008u)---

    MCUSYS_ON_DBG_SEL[2..0]      - (RW) wox mcusys on debug selection
    RESERVED3[31..3]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_ON_DBG_CTL_MCUSYS_ON_DBG_SEL_ADDR         WOX_MCU_CFG_ON_DBG_CTL_ADDR
#define WOX_MCU_CFG_ON_DBG_CTL_MCUSYS_ON_DBG_SEL_MASK         0x00000007                // MCUSYS_ON_DBG_SEL[2..0]
#define WOX_MCU_CFG_ON_DBG_CTL_MCUSYS_ON_DBG_SEL_SHFT         0

/* =====================================================================================

  ---CFG_DBG1 (0x15596000 + 0x00Cu)---

    DEBUG_N0_EN[0]               - (RW) 0: Disable
                                     1: Enable
    DEBUG_N1_EN[1]               - (RW) 0: Disable
                                     1: Enable
    DEBUG_N2_EN[2]               - (RW) 0: Disable
                                     1: Enable
    DEBUG_N3_EN[3]               - (RW) 0: Disable
                                     1: Enable
    JTAG_SEL_EN[4]               - (RW) 0: Disable
                                     1: Enable
    JTAG_SEL[5]                  - (RW) 0: 5 wire
                                     1: 2 wire
    RESERVED6[31..6]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_ON_CFG_DBG1_JTAG_SEL_ADDR                 WOX_MCU_CFG_ON_CFG_DBG1_ADDR
#define WOX_MCU_CFG_ON_CFG_DBG1_JTAG_SEL_MASK                 0x00000020                // JTAG_SEL[5]
#define WOX_MCU_CFG_ON_CFG_DBG1_JTAG_SEL_SHFT                 5
#define WOX_MCU_CFG_ON_CFG_DBG1_JTAG_SEL_EN_ADDR              WOX_MCU_CFG_ON_CFG_DBG1_ADDR
#define WOX_MCU_CFG_ON_CFG_DBG1_JTAG_SEL_EN_MASK              0x00000010                // JTAG_SEL_EN[4]
#define WOX_MCU_CFG_ON_CFG_DBG1_JTAG_SEL_EN_SHFT              4
#define WOX_MCU_CFG_ON_CFG_DBG1_DEBUG_N3_EN_ADDR              WOX_MCU_CFG_ON_CFG_DBG1_ADDR
#define WOX_MCU_CFG_ON_CFG_DBG1_DEBUG_N3_EN_MASK              0x00000008                // DEBUG_N3_EN[3]
#define WOX_MCU_CFG_ON_CFG_DBG1_DEBUG_N3_EN_SHFT              3
#define WOX_MCU_CFG_ON_CFG_DBG1_DEBUG_N2_EN_ADDR              WOX_MCU_CFG_ON_CFG_DBG1_ADDR
#define WOX_MCU_CFG_ON_CFG_DBG1_DEBUG_N2_EN_MASK              0x00000004                // DEBUG_N2_EN[2]
#define WOX_MCU_CFG_ON_CFG_DBG1_DEBUG_N2_EN_SHFT              2
#define WOX_MCU_CFG_ON_CFG_DBG1_DEBUG_N1_EN_ADDR              WOX_MCU_CFG_ON_CFG_DBG1_ADDR
#define WOX_MCU_CFG_ON_CFG_DBG1_DEBUG_N1_EN_MASK              0x00000002                // DEBUG_N1_EN[1]
#define WOX_MCU_CFG_ON_CFG_DBG1_DEBUG_N1_EN_SHFT              1
#define WOX_MCU_CFG_ON_CFG_DBG1_DEBUG_N0_EN_ADDR              WOX_MCU_CFG_ON_CFG_DBG1_ADDR
#define WOX_MCU_CFG_ON_CFG_DBG1_DEBUG_N0_EN_MASK              0x00000001                // DEBUG_N0_EN[0]
#define WOX_MCU_CFG_ON_CFG_DBG1_DEBUG_N0_EN_SHFT              0

/* =====================================================================================

  ---CFG_DBG2 (0x15596000 + 0x010u)---

    DEBUG_L1_N0_MOD_SEL[7..0]    - (RW) Debug flag selection
    DEBUG_L1_N1_MOD_SEL[15..8]   - (RW) Debug flag selection
    DEBUG_L1_N2_MOD_SEL[23..16]  - (RW) Debug flag selection
    DEBUG_L1_N3_MOD_SEL[31..24]  - (RW) Debug flag selection

 =====================================================================================*/
#define WOX_MCU_CFG_ON_CFG_DBG2_DEBUG_L1_N3_MOD_SEL_ADDR      WOX_MCU_CFG_ON_CFG_DBG2_ADDR
#define WOX_MCU_CFG_ON_CFG_DBG2_DEBUG_L1_N3_MOD_SEL_MASK      0xFF000000                // DEBUG_L1_N3_MOD_SEL[31..24]
#define WOX_MCU_CFG_ON_CFG_DBG2_DEBUG_L1_N3_MOD_SEL_SHFT      24
#define WOX_MCU_CFG_ON_CFG_DBG2_DEBUG_L1_N2_MOD_SEL_ADDR      WOX_MCU_CFG_ON_CFG_DBG2_ADDR
#define WOX_MCU_CFG_ON_CFG_DBG2_DEBUG_L1_N2_MOD_SEL_MASK      0x00FF0000                // DEBUG_L1_N2_MOD_SEL[23..16]
#define WOX_MCU_CFG_ON_CFG_DBG2_DEBUG_L1_N2_MOD_SEL_SHFT      16
#define WOX_MCU_CFG_ON_CFG_DBG2_DEBUG_L1_N1_MOD_SEL_ADDR      WOX_MCU_CFG_ON_CFG_DBG2_ADDR
#define WOX_MCU_CFG_ON_CFG_DBG2_DEBUG_L1_N1_MOD_SEL_MASK      0x0000FF00                // DEBUG_L1_N1_MOD_SEL[15..8]
#define WOX_MCU_CFG_ON_CFG_DBG2_DEBUG_L1_N1_MOD_SEL_SHFT      8
#define WOX_MCU_CFG_ON_CFG_DBG2_DEBUG_L1_N0_MOD_SEL_ADDR      WOX_MCU_CFG_ON_CFG_DBG2_ADDR
#define WOX_MCU_CFG_ON_CFG_DBG2_DEBUG_L1_N0_MOD_SEL_MASK      0x000000FF                // DEBUG_L1_N0_MOD_SEL[7..0]
#define WOX_MCU_CFG_ON_CFG_DBG2_DEBUG_L1_N0_MOD_SEL_SHFT      0

/* =====================================================================================

  ---WOX_MCCR (0x15596000 + 0x014u)---

    RESERVED0[9..0]              - (RO) Reserved bits
    UART_DBG_STOP[11..10]        - (RO) UART_DBG STOP control
                                     Triggered by CR set and released by CR clear
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_ON_WOX_MCCR_UART_DBG_STOP_ADDR            WOX_MCU_CFG_ON_WOX_MCCR_ADDR
#define WOX_MCU_CFG_ON_WOX_MCCR_UART_DBG_STOP_MASK            0x00000C00                // UART_DBG_STOP[11..10]
#define WOX_MCU_CFG_ON_WOX_MCCR_UART_DBG_STOP_SHFT            10

/* =====================================================================================

  ---WOX_MCCR_SET (0x15596000 + 0x018u)---

    RESERVED0[9..0]              - (RO) Reserved bits
    UART_DBG[11..10]             - (W1S) UART_DBG_STOP Set. Write 1 to set stop
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_ON_WOX_MCCR_SET_UART_DBG_ADDR             WOX_MCU_CFG_ON_WOX_MCCR_SET_ADDR
#define WOX_MCU_CFG_ON_WOX_MCCR_SET_UART_DBG_MASK             0x00000C00                // UART_DBG[11..10]
#define WOX_MCU_CFG_ON_WOX_MCCR_SET_UART_DBG_SHFT             10

/* =====================================================================================

  ---WOX_MCCR_CLR (0x15596000 + 0x01Cu)---

    RESERVED0[9..0]              - (RO) Reserved bits
    UART_DBG[11..10]             - (W1C) UART_DBG_STOP Clear. Write 1 to set clear
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_ON_WOX_MCCR_CLR_UART_DBG_ADDR             WOX_MCU_CFG_ON_WOX_MCCR_CLR_ADDR
#define WOX_MCU_CFG_ON_WOX_MCCR_CLR_UART_DBG_MASK             0x00000C00                // UART_DBG[11..10]
#define WOX_MCU_CFG_ON_WOX_MCCR_CLR_UART_DBG_SHFT             10

/* =====================================================================================

  ---DBG_EINT (0x15596000 + 0x020u)---

    SW_DBG_EINT[0]               - (RW) sw debug eint
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_ON_DBG_EINT_SW_DBG_EINT_ADDR              WOX_MCU_CFG_ON_DBG_EINT_ADDR
#define WOX_MCU_CFG_ON_DBG_EINT_SW_DBG_EINT_MASK              0x00000001                // SW_DBG_EINT[0]
#define WOX_MCU_CFG_ON_DBG_EINT_SW_DBG_EINT_SHFT              0

/* =====================================================================================

  ---WOX_MCU_CFG_WO0_WO1 (0x15596000 + 0x050u)---

    RESERVED0[4..0]              - (RO) Reserved bits
    WO0_CPU_RSTB[5]              - (RW) WO0 CPU software reset (low active)
    RESERVED6[31..6]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_ON_WOX_MCU_CFG_WO0_WO1_WO0_CPU_RSTB_ADDR  WOX_MCU_CFG_ON_WOX_MCU_CFG_WO0_WO1_ADDR
#define WOX_MCU_CFG_ON_WOX_MCU_CFG_WO0_WO1_WO0_CPU_RSTB_MASK  0x00000020                // WO0_CPU_RSTB[5]
#define WOX_MCU_CFG_ON_WOX_MCU_CFG_WO0_WO1_WO0_CPU_RSTB_SHFT  5

/* =====================================================================================

  ---MCSR (0x15596000 + 0x010Cu)---

    RESERVED0[19..0]             - (RO) Reserved bits
    STBY2RESUM_CYC[24..20]       - (RW) Standby to Resume Cycle Setting
    RESERVED25[31..25]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_ON_MCSR_STBY2RESUM_CYC_ADDR               WOX_MCU_CFG_ON_MCSR_ADDR
#define WOX_MCU_CFG_ON_MCSR_STBY2RESUM_CYC_MASK               0x01F00000                // STBY2RESUM_CYC[24..20]
#define WOX_MCU_CFG_ON_MCSR_STBY2RESUM_CYC_SHFT               20

/* =====================================================================================

  ---WOX_MCU_CG (0x15596000 + 0x029Cu)---

    WO0CPU_GT_CKEN[0]            - (RW)  xxx
    WO0ULM_CLK_EN2[1]            - (RW)  xxx
    BUS_CLK_EN2[2]               - (RW)  xxx
    AXI_CLK_EN[3]                - (RW)  xxx
    CPU2AXI_BUS_DIV_EN[4]        - (RW)  xxx
    AXI2LS_BUS_DIV_EN[5]         - (RW)  xxx
    AXI_GT_CKEN[6]               - (RW)  xxx
    AXI_DIVFR_CKEN[7]            - (RW)  xxx
    RESERVED8[10..8]             - (RO) Reserved bits
    MCUSYS_CLK_EN[11]            - (RW)  xxx
    RESERVED12[12]               - (RO) Reserved bits
    CFG_HS_PCLK_GT_EN[13]        - (RW)  xxx
    EXCP_CON_PCLK_GT_EN[14]      - (RW)  xxx
    AXIMON_PCLK_GT_EN[15]        - (RW)  xxx
    WO1CPU_GT_CKEN[16]           - (WO)  xxx
    WO1ULM_CLK_EN2[17]           - (WO)  xxx
    RESERVED18[31..18]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_ON_WOX_MCU_CG_WO1ULM_CLK_EN2_ADDR         WOX_MCU_CFG_ON_WOX_MCU_CG_ADDR
#define WOX_MCU_CFG_ON_WOX_MCU_CG_WO1ULM_CLK_EN2_MASK         0x00020000                // WO1ULM_CLK_EN2[17]
#define WOX_MCU_CFG_ON_WOX_MCU_CG_WO1ULM_CLK_EN2_SHFT         17
#define WOX_MCU_CFG_ON_WOX_MCU_CG_WO1CPU_GT_CKEN_ADDR         WOX_MCU_CFG_ON_WOX_MCU_CG_ADDR
#define WOX_MCU_CFG_ON_WOX_MCU_CG_WO1CPU_GT_CKEN_MASK         0x00010000                // WO1CPU_GT_CKEN[16]
#define WOX_MCU_CFG_ON_WOX_MCU_CG_WO1CPU_GT_CKEN_SHFT         16
#define WOX_MCU_CFG_ON_WOX_MCU_CG_AXIMON_PCLK_GT_EN_ADDR      WOX_MCU_CFG_ON_WOX_MCU_CG_ADDR
#define WOX_MCU_CFG_ON_WOX_MCU_CG_AXIMON_PCLK_GT_EN_MASK      0x00008000                // AXIMON_PCLK_GT_EN[15]
#define WOX_MCU_CFG_ON_WOX_MCU_CG_AXIMON_PCLK_GT_EN_SHFT      15
#define WOX_MCU_CFG_ON_WOX_MCU_CG_EXCP_CON_PCLK_GT_EN_ADDR    WOX_MCU_CFG_ON_WOX_MCU_CG_ADDR
#define WOX_MCU_CFG_ON_WOX_MCU_CG_EXCP_CON_PCLK_GT_EN_MASK    0x00004000                // EXCP_CON_PCLK_GT_EN[14]
#define WOX_MCU_CFG_ON_WOX_MCU_CG_EXCP_CON_PCLK_GT_EN_SHFT    14
#define WOX_MCU_CFG_ON_WOX_MCU_CG_CFG_HS_PCLK_GT_EN_ADDR      WOX_MCU_CFG_ON_WOX_MCU_CG_ADDR
#define WOX_MCU_CFG_ON_WOX_MCU_CG_CFG_HS_PCLK_GT_EN_MASK      0x00002000                // CFG_HS_PCLK_GT_EN[13]
#define WOX_MCU_CFG_ON_WOX_MCU_CG_CFG_HS_PCLK_GT_EN_SHFT      13
#define WOX_MCU_CFG_ON_WOX_MCU_CG_MCUSYS_CLK_EN_ADDR          WOX_MCU_CFG_ON_WOX_MCU_CG_ADDR
#define WOX_MCU_CFG_ON_WOX_MCU_CG_MCUSYS_CLK_EN_MASK          0x00000800                // MCUSYS_CLK_EN[11]
#define WOX_MCU_CFG_ON_WOX_MCU_CG_MCUSYS_CLK_EN_SHFT          11
#define WOX_MCU_CFG_ON_WOX_MCU_CG_AXI_DIVFR_CKEN_ADDR         WOX_MCU_CFG_ON_WOX_MCU_CG_ADDR
#define WOX_MCU_CFG_ON_WOX_MCU_CG_AXI_DIVFR_CKEN_MASK         0x00000080                // AXI_DIVFR_CKEN[7]
#define WOX_MCU_CFG_ON_WOX_MCU_CG_AXI_DIVFR_CKEN_SHFT         7
#define WOX_MCU_CFG_ON_WOX_MCU_CG_AXI_GT_CKEN_ADDR            WOX_MCU_CFG_ON_WOX_MCU_CG_ADDR
#define WOX_MCU_CFG_ON_WOX_MCU_CG_AXI_GT_CKEN_MASK            0x00000040                // AXI_GT_CKEN[6]
#define WOX_MCU_CFG_ON_WOX_MCU_CG_AXI_GT_CKEN_SHFT            6
#define WOX_MCU_CFG_ON_WOX_MCU_CG_AXI2LS_BUS_DIV_EN_ADDR      WOX_MCU_CFG_ON_WOX_MCU_CG_ADDR
#define WOX_MCU_CFG_ON_WOX_MCU_CG_AXI2LS_BUS_DIV_EN_MASK      0x00000020                // AXI2LS_BUS_DIV_EN[5]
#define WOX_MCU_CFG_ON_WOX_MCU_CG_AXI2LS_BUS_DIV_EN_SHFT      5
#define WOX_MCU_CFG_ON_WOX_MCU_CG_CPU2AXI_BUS_DIV_EN_ADDR     WOX_MCU_CFG_ON_WOX_MCU_CG_ADDR
#define WOX_MCU_CFG_ON_WOX_MCU_CG_CPU2AXI_BUS_DIV_EN_MASK     0x00000010                // CPU2AXI_BUS_DIV_EN[4]
#define WOX_MCU_CFG_ON_WOX_MCU_CG_CPU2AXI_BUS_DIV_EN_SHFT     4
#define WOX_MCU_CFG_ON_WOX_MCU_CG_AXI_CLK_EN_ADDR             WOX_MCU_CFG_ON_WOX_MCU_CG_ADDR
#define WOX_MCU_CFG_ON_WOX_MCU_CG_AXI_CLK_EN_MASK             0x00000008                // AXI_CLK_EN[3]
#define WOX_MCU_CFG_ON_WOX_MCU_CG_AXI_CLK_EN_SHFT             3
#define WOX_MCU_CFG_ON_WOX_MCU_CG_BUS_CLK_EN2_ADDR            WOX_MCU_CFG_ON_WOX_MCU_CG_ADDR
#define WOX_MCU_CFG_ON_WOX_MCU_CG_BUS_CLK_EN2_MASK            0x00000004                // BUS_CLK_EN2[2]
#define WOX_MCU_CFG_ON_WOX_MCU_CG_BUS_CLK_EN2_SHFT            2
#define WOX_MCU_CFG_ON_WOX_MCU_CG_WO0ULM_CLK_EN2_ADDR         WOX_MCU_CFG_ON_WOX_MCU_CG_ADDR
#define WOX_MCU_CFG_ON_WOX_MCU_CG_WO0ULM_CLK_EN2_MASK         0x00000002                // WO0ULM_CLK_EN2[1]
#define WOX_MCU_CFG_ON_WOX_MCU_CG_WO0ULM_CLK_EN2_SHFT         1
#define WOX_MCU_CFG_ON_WOX_MCU_CG_WO0CPU_GT_CKEN_ADDR         WOX_MCU_CFG_ON_WOX_MCU_CG_ADDR
#define WOX_MCU_CFG_ON_WOX_MCU_CG_WO0CPU_GT_CKEN_MASK         0x00000001                // WO0CPU_GT_CKEN[0]
#define WOX_MCU_CFG_ON_WOX_MCU_CG_WO0CPU_GT_CKEN_SHFT         0

/* =====================================================================================

  ---DBGSR (0x15596000 + 0x400u)---

    DEBUG_SELECT[23..0]          - (RW) Internal debug selection
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_ON_DBGSR_DEBUG_SELECT_ADDR                WOX_MCU_CFG_ON_DBGSR_ADDR
#define WOX_MCU_CFG_ON_DBGSR_DEBUG_SELECT_MASK                0x00FFFFFF                // DEBUG_SELECT[23..0]
#define WOX_MCU_CFG_ON_DBGSR_DEBUG_SELECT_SHFT                0

/* =====================================================================================

  ---DBGSR1 (0x15596000 + 0x404u)---

    DEBUG_SELECT1[7..0]          - (RW) Internal debug selection1
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_ON_DBGSR1_DEBUG_SELECT1_ADDR              WOX_MCU_CFG_ON_DBGSR1_ADDR
#define WOX_MCU_CFG_ON_DBGSR1_DEBUG_SELECT1_MASK              0x000000FF                // DEBUG_SELECT1[7..0]
#define WOX_MCU_CFG_ON_DBGSR1_DEBUG_SELECT1_SHFT              0

/* =====================================================================================

  ---DBGSR2 (0x15596000 + 0x408u)---

    DEBUG_SELECT2[31..0]         - (RW) Internal debug selection2

 =====================================================================================*/
#define WOX_MCU_CFG_ON_DBGSR2_DEBUG_SELECT2_ADDR              WOX_MCU_CFG_ON_DBGSR2_ADDR
#define WOX_MCU_CFG_ON_DBGSR2_DEBUG_SELECT2_MASK              0xFFFFFFFF                // DEBUG_SELECT2[31..0]
#define WOX_MCU_CFG_ON_DBGSR2_DEBUG_SELECT2_SHFT              0

/* =====================================================================================

  ---WO2NETSYS_AID (0x15596000 + 0x500u)---

    WO2NETSYS_AXDOMAIN[3..0]     - (W1) WOCPUSYS to NETSYS AXI domain
    RESERVED4[31..4]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_ON_WO2NETSYS_AID_WO2NETSYS_AXDOMAIN_ADDR  WOX_MCU_CFG_ON_WO2NETSYS_AID_ADDR
#define WOX_MCU_CFG_ON_WO2NETSYS_AID_WO2NETSYS_AXDOMAIN_MASK  0x0000000F                // WO2NETSYS_AXDOMAIN[3..0]
#define WOX_MCU_CFG_ON_WO2NETSYS_AID_WO2NETSYS_AXDOMAIN_SHFT  0

#ifdef __cplusplus
}
#endif

#endif // __WOX_MCU_CFG_ON_REGS_H__
