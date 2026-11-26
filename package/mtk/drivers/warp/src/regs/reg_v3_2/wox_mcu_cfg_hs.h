
#ifndef __WOX_MCU_CFG_HS_REGS_H__
#define __WOX_MCU_CFG_HS_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
//
//                     WOX_MCU_CFG_HS CR Definitions
//
//****************************************************************************

#define WOX_MCU_CFG_HS_WO0_BOOT_ADDR_ADDR                     0x0000060
#define WOX_MCU_CFG_HS_WOX_CFG_CTL2_ADDR                      0x000006C

/* =====================================================================================

  ---WO0_BOOT_ADDR (0x15591000 + 0x060u)---

    WO0_BOOT_ADDR[15..0]         - (RW) WO0 CPU Boot address MSB
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_HS_WO0_BOOT_ADDR_WO0_BOOT_ADDR_ADDR       WOX_MCU_CFG_HS_WO0_BOOT_ADDR_ADDR
#define WOX_MCU_CFG_HS_WO0_BOOT_ADDR_WO0_BOOT_ADDR_MASK       0x0000FFFF                // WO0_BOOT_ADDR[15..0]
#define WOX_MCU_CFG_HS_WO0_BOOT_ADDR_WO0_BOOT_ADDR_SHFT       0

/* =====================================================================================

  ---WOX_CFG_CTL2 (0x15591000 + 0x06Cu)---

    WO0_ILM_BOOT[0]              - (RW) WO0 CPU ILM_BOOT enable
                                     0: Disable
                                     1: Enable
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_HS_WOX_CFG_CTL2_WO0_ILM_BOOT_ADDR         WOX_MCU_CFG_HS_WOX_CFG_CTL2_ADDR
#define WOX_MCU_CFG_HS_WOX_CFG_CTL2_WO0_ILM_BOOT_MASK         0x00000001                // WO0_ILM_BOOT[0]
#define WOX_MCU_CFG_HS_WOX_CFG_CTL2_WO0_ILM_BOOT_SHFT         0

#ifdef __cplusplus
}
#endif

#endif // __WOX_MCU_CFG_HS_REGS_H__
