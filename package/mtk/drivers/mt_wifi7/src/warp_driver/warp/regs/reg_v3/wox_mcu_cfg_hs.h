
#ifndef __WOX_MCU_CFG_HS_H__
#define __WOX_MCU_CFG_HS_H__

/* CONST definition */
#ifndef REG_CONST
#define REG_CONST
#endif
typedef unsigned int uint32_t;

typedef volatile struct wo2_mcu_cfg_hs_REG {
    /* 0x000 ~ 0x06c */
    REG_CONST uint32_t          RSV_000_06c[28];

    /* 0x070 */
    union {
        struct {
            REG_CONST uint32_t _WOX_APSRC_IDLE : 1;
            REG_CONST uint32_t _WOX_APSRC_REQ : 1;
            REG_CONST uint32_t _WOX_APSRC_POWOFF : 1;
            REG_CONST uint32_t _WOX_APSRC_DDREN : 1;
            REG_CONST uint32_t _RESERVED_4 : 28;
        };
        REG_CONST uint32_t DATA;
    } WOX_APSRC;

    /* 0x074 ~ 0x108 */
    REG_CONST uint32_t          RSV_074_108[38];

    /* 0x10c */
    union {
        struct {
            REG_CONST uint32_t _RESERVED_0 : 20;
            REG_CONST uint32_t _STBY2RESUM_CYC : 5;
            REG_CONST uint32_t _RESERVED_2 : 7;
        };
        REG_CONST uint32_t DATA;
    } MCSR;

    /* 0x110 ~ 0x298 */
    REG_CONST uint32_t          RSV_110_298[99];

    /* 0x29c */
    union {
        struct {
            REG_CONST uint32_t _WO0CPU_GT_CKEN : 1;
            REG_CONST uint32_t _WO0ULM_CLK_EN2 : 1;
            REG_CONST uint32_t _BUS_CLK_EN2 : 1;
            REG_CONST uint32_t _AXI_CLK_EN : 1;
            REG_CONST uint32_t _CPU2AXI_BUS_DIV_EN : 1;
            REG_CONST uint32_t _AXI2LS_BUS_DIV_EN : 1;
            REG_CONST uint32_t _AXI_GT_CKEN : 1;
            REG_CONST uint32_t _AXI_DIVFR_CKEN : 1;
            REG_CONST uint32_t _RESERVED_8 : 3;
            REG_CONST uint32_t _MCUSYS_CLK_EN : 1;
            REG_CONST uint32_t _RESERVED_10 : 1;
            REG_CONST uint32_t _CFG_HS_PCLK_GT_EN : 1;
            REG_CONST uint32_t _EXCP_CON_PCLK_GT_EN : 1;
            REG_CONST uint32_t _AXIMON_PCLK_GT_EN : 1;
            REG_CONST uint32_t _WO1CPU_GT_CKEN : 1;
            REG_CONST uint32_t _WO1ULM_CLK_EN2 : 1;
            REG_CONST uint32_t _RESERVED_16 : 14;
        };
        REG_CONST uint32_t DATA;
    } WOX_MCU_CG;
} wo2_mcu_cfg_hs_REG_T;

#define WOX_MCU_CFG_HS_WOX_APSRC_OFFSET                        (0x0070) // 5070
#define WOX_MCU_CFG_HS_MCSR_OFFSET                             (0x010C) // 510C
#define WOX_MCU_CFG_HS_WOX_MCU_CG_OFFSET                       (0x029C) // 529C

/* =====================================================================================

  ---WOX_APSRC (0x15395000 + 0x0070)---

    WOX_APSRC_IDLE[0]            - (RW) WO0 APSRC idle
    WOX_APSRC_REQ[1]             - (RW) WO0 APSRC request
    WOX_APSRC_POWOFF[2]          - (RW) WO0 APSRC power off
    WOX_APSRC_DDREN[3]           - (RW) WO0 APSRC DDREN
    RESERVED4[31..4]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_HS_WOX_APSRC_WOX_APSRC_DDREN_OFFSET        WOX_MCU_CFG_HS_WOX_APSRC_OFFSET
#define WOX_MCU_CFG_HS_WOX_APSRC_WOX_APSRC_DDREN_MASK          0x00000008                // WOX_APSRC_DDREN[3]
#define WOX_MCU_CFG_HS_WOX_APSRC_WOX_APSRC_DDREN_SHFT          3
#define WOX_MCU_CFG_HS_WOX_APSRC_WOX_APSRC_POWOFF_OFFSET       WOX_MCU_CFG_HS_WOX_APSRC_OFFSET
#define WOX_MCU_CFG_HS_WOX_APSRC_WOX_APSRC_POWOFF_MASK         0x00000004                // WOX_APSRC_POWOFF[2]
#define WOX_MCU_CFG_HS_WOX_APSRC_WOX_APSRC_POWOFF_SHFT         2
#define WOX_MCU_CFG_HS_WOX_APSRC_WOX_APSRC_REQ_OFFSET          WOX_MCU_CFG_HS_WOX_APSRC_OFFSET
#define WOX_MCU_CFG_HS_WOX_APSRC_WOX_APSRC_REQ_MASK            0x00000002                // WOX_APSRC_REQ[1]
#define WOX_MCU_CFG_HS_WOX_APSRC_WOX_APSRC_REQ_SHFT            1
#define WOX_MCU_CFG_HS_WOX_APSRC_WOX_APSRC_IDLE_OFFSET         WOX_MCU_CFG_HS_WOX_APSRC_OFFSET
#define WOX_MCU_CFG_HS_WOX_APSRC_WOX_APSRC_IDLE_MASK           0x00000001                // WOX_APSRC_IDLE[0]
#define WOX_MCU_CFG_HS_WOX_APSRC_WOX_APSRC_IDLE_SHFT           0

/* =====================================================================================

  ---MCSR (0x15395000 + 0x010C)---

    RESERVED0[19..0]             - (RO) Reserved bits
    STBY2RESUM_CYC[24..20]       - (RW) Standby to Resume Cycle Setting
    RESERVED25[31..25]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_HS_MCSR_STBY2RESUM_CYC_OFFSET              WOX_MCU_CFG_HS_MCSR_OFFSET
#define WOX_MCU_CFG_HS_MCSR_STBY2RESUM_CYC_MASK                0x01F00000                // STBY2RESUM_CYC[24..20]
#define WOX_MCU_CFG_HS_MCSR_STBY2RESUM_CYC_SHFT                20

/* =====================================================================================

  ---WOX_MCU_CG (0x15395000 + 0x029C)---

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
#define WOX_MCU_CFG_HS_WOX_MCU_CG_WO1ULM_CLK_EN2_OFFSET        WOX_MCU_CFG_HS_WOX_MCU_CG_OFFSET
#define WOX_MCU_CFG_HS_WOX_MCU_CG_WO1ULM_CLK_EN2_MASK          0x00020000                // WO1ULM_CLK_EN2[17]
#define WOX_MCU_CFG_HS_WOX_MCU_CG_WO1ULM_CLK_EN2_SHFT          17
#define WOX_MCU_CFG_HS_WOX_MCU_CG_WO1CPU_GT_CKEN_OFFSET        WOX_MCU_CFG_HS_WOX_MCU_CG_OFFSET
#define WOX_MCU_CFG_HS_WOX_MCU_CG_WO1CPU_GT_CKEN_MASK          0x00010000                // WO1CPU_GT_CKEN[16]
#define WOX_MCU_CFG_HS_WOX_MCU_CG_WO1CPU_GT_CKEN_SHFT          16
#define WOX_MCU_CFG_HS_WOX_MCU_CG_AXIMON_PCLK_GT_EN_OFFSET     WOX_MCU_CFG_HS_WOX_MCU_CG_OFFSET
#define WOX_MCU_CFG_HS_WOX_MCU_CG_AXIMON_PCLK_GT_EN_MASK       0x00008000                // AXIMON_PCLK_GT_EN[15]
#define WOX_MCU_CFG_HS_WOX_MCU_CG_AXIMON_PCLK_GT_EN_SHFT       15
#define WOX_MCU_CFG_HS_WOX_MCU_CG_EXCP_CON_PCLK_GT_EN_OFFSET   WOX_MCU_CFG_HS_WOX_MCU_CG_OFFSET
#define WOX_MCU_CFG_HS_WOX_MCU_CG_EXCP_CON_PCLK_GT_EN_MASK     0x00004000                // EXCP_CON_PCLK_GT_EN[14]
#define WOX_MCU_CFG_HS_WOX_MCU_CG_EXCP_CON_PCLK_GT_EN_SHFT     14
#define WOX_MCU_CFG_HS_WOX_MCU_CG_CFG_HS_PCLK_GT_EN_OFFSET     WOX_MCU_CFG_HS_WOX_MCU_CG_OFFSET
#define WOX_MCU_CFG_HS_WOX_MCU_CG_CFG_HS_PCLK_GT_EN_MASK       0x00002000                // CFG_HS_PCLK_GT_EN[13]
#define WOX_MCU_CFG_HS_WOX_MCU_CG_CFG_HS_PCLK_GT_EN_SHFT       13
#define WOX_MCU_CFG_HS_WOX_MCU_CG_MCUSYS_CLK_EN_OFFSET         WOX_MCU_CFG_HS_WOX_MCU_CG_OFFSET
#define WOX_MCU_CFG_HS_WOX_MCU_CG_MCUSYS_CLK_EN_MASK           0x00000800                // MCUSYS_CLK_EN[11]
#define WOX_MCU_CFG_HS_WOX_MCU_CG_MCUSYS_CLK_EN_SHFT           11
#define WOX_MCU_CFG_HS_WOX_MCU_CG_AXI_DIVFR_CKEN_OFFSET        WOX_MCU_CFG_HS_WOX_MCU_CG_OFFSET
#define WOX_MCU_CFG_HS_WOX_MCU_CG_AXI_DIVFR_CKEN_MASK          0x00000080                // AXI_DIVFR_CKEN[7]
#define WOX_MCU_CFG_HS_WOX_MCU_CG_AXI_DIVFR_CKEN_SHFT          7
#define WOX_MCU_CFG_HS_WOX_MCU_CG_AXI_GT_CKEN_OFFSET           WOX_MCU_CFG_HS_WOX_MCU_CG_OFFSET
#define WOX_MCU_CFG_HS_WOX_MCU_CG_AXI_GT_CKEN_MASK             0x00000040                // AXI_GT_CKEN[6]
#define WOX_MCU_CFG_HS_WOX_MCU_CG_AXI_GT_CKEN_SHFT             6
#define WOX_MCU_CFG_HS_WOX_MCU_CG_AXI2LS_BUS_DIV_EN_OFFSET     WOX_MCU_CFG_HS_WOX_MCU_CG_OFFSET
#define WOX_MCU_CFG_HS_WOX_MCU_CG_AXI2LS_BUS_DIV_EN_MASK       0x00000020                // AXI2LS_BUS_DIV_EN[5]
#define WOX_MCU_CFG_HS_WOX_MCU_CG_AXI2LS_BUS_DIV_EN_SHFT       5
#define WOX_MCU_CFG_HS_WOX_MCU_CG_CPU2AXI_BUS_DIV_EN_OFFSET    WOX_MCU_CFG_HS_WOX_MCU_CG_OFFSET
#define WOX_MCU_CFG_HS_WOX_MCU_CG_CPU2AXI_BUS_DIV_EN_MASK      0x00000010                // CPU2AXI_BUS_DIV_EN[4]
#define WOX_MCU_CFG_HS_WOX_MCU_CG_CPU2AXI_BUS_DIV_EN_SHFT      4
#define WOX_MCU_CFG_HS_WOX_MCU_CG_AXI_CLK_EN_OFFSET            WOX_MCU_CFG_HS_WOX_MCU_CG_OFFSET
#define WOX_MCU_CFG_HS_WOX_MCU_CG_AXI_CLK_EN_MASK              0x00000008                // AXI_CLK_EN[3]
#define WOX_MCU_CFG_HS_WOX_MCU_CG_AXI_CLK_EN_SHFT              3
#define WOX_MCU_CFG_HS_WOX_MCU_CG_BUS_CLK_EN2_OFFSET           WOX_MCU_CFG_HS_WOX_MCU_CG_OFFSET
#define WOX_MCU_CFG_HS_WOX_MCU_CG_BUS_CLK_EN2_MASK             0x00000004                // BUS_CLK_EN2[2]
#define WOX_MCU_CFG_HS_WOX_MCU_CG_BUS_CLK_EN2_SHFT             2
#define WOX_MCU_CFG_HS_WOX_MCU_CG_WO0ULM_CLK_EN2_OFFSET        WOX_MCU_CFG_HS_WOX_MCU_CG_OFFSET
#define WOX_MCU_CFG_HS_WOX_MCU_CG_WO0ULM_CLK_EN2_MASK          0x00000002                // WO0ULM_CLK_EN2[1]
#define WOX_MCU_CFG_HS_WOX_MCU_CG_WO0ULM_CLK_EN2_SHFT          1
#define WOX_MCU_CFG_HS_WOX_MCU_CG_WO0CPU_GT_CKEN_OFFSET        WOX_MCU_CFG_HS_WOX_MCU_CG_OFFSET
#define WOX_MCU_CFG_HS_WOX_MCU_CG_WO0CPU_GT_CKEN_MASK          0x00000001                // WO0CPU_GT_CKEN[0]
#define WOX_MCU_CFG_HS_WOX_MCU_CG_WO0CPU_GT_CKEN_SHFT          0

#endif
