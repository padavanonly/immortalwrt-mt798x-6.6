
#ifndef __WOX_MCU_CFG_LS_H__
#define __WOX_MCU_CFG_LS_H__

/* CONST definition */
#ifndef REG_CONST
#define REG_CONST
#endif
typedef unsigned int uint32_t;

typedef volatile struct wo2_mcu_cfg_ls_REG {
    /* 0x000 */
    union {
        struct {
            REG_CONST uint32_t _HW_VER : 32;
        };
        REG_CONST uint32_t DATA;
    } HW_VER;

    /* 0x004 */
    union {
        struct {
            REG_CONST uint32_t _FW_VER : 32;
        };
        REG_CONST uint32_t DATA;
    } FW_VER;

    /* 0x008 ~ 0x008 */
    REG_CONST uint32_t          RSV_008_008[1];

    /* 0x00c */
    union {
        struct {
            REG_CONST uint32_t _DEBUG_N0_EN : 1;
            REG_CONST uint32_t _DEBUG_N1_EN : 1;
            REG_CONST uint32_t _DEBUG_N2_EN : 1;
            REG_CONST uint32_t _DEBUG_N3_EN : 1;
            REG_CONST uint32_t _JTAG_SEL_EN : 1;
            REG_CONST uint32_t _JTAG_SEL : 1;
            REG_CONST uint32_t _RESERVED_6 : 26;
        };
        REG_CONST uint32_t DATA;
    } CFG_DBG1;

    /* 0x010 */
    union {
        struct {
            REG_CONST uint32_t _DEBUG_L1_N0_MOD_SEL : 8;
            REG_CONST uint32_t _DEBUG_L1_N1_MOD_SEL : 8;
            REG_CONST uint32_t _DEBUG_L1_N2_MOD_SEL : 8;
            REG_CONST uint32_t _DEBUG_L1_N3_MOD_SEL : 8;
        };
        REG_CONST uint32_t DATA;
    } CFG_DBG2;

    /* 0x014 */
    union {
        struct {
            REG_CONST uint32_t _RESERVED_0 : 10;
            REG_CONST uint32_t _UART_DBG_STOP : 2;
            REG_CONST uint32_t _RESERVED_2 : 20;
        };
        REG_CONST uint32_t DATA;
    } WOX_MCCR;

    /* 0x018 */
    union {
        struct {
            REG_CONST uint32_t _RESERVED_0 : 10;
            REG_CONST uint32_t _UART_DBG : 2;
            REG_CONST uint32_t _RESERVED_2 : 20;
        };
        REG_CONST uint32_t DATA;
    } WOX_MCCR_SET;

    /* 0x01c */
    union {
        struct {
            REG_CONST uint32_t _RESERVED_0 : 10;
            REG_CONST uint32_t _UART_DBG : 2;
            REG_CONST uint32_t _RESERVED_2 : 20;
        };
        REG_CONST uint32_t DATA;
    } WOX_MCCR_CLR;

    /* 0x020 ~ 0x04c */
    REG_CONST uint32_t          RSV_020_04c[12];

    /* 0x050 */
    union {
        struct {
            REG_CONST uint32_t _RESERVED_0 : 5;
            REG_CONST uint32_t _WOX_CPU_RSTB : 1;
            REG_CONST uint32_t _RESERVED_2 : 26;
        };
        REG_CONST uint32_t DATA;
    } WOX_MCU_CFG_WO0_WO1;

    /* 0x054 ~ 0x05c */
    REG_CONST uint32_t          RSV_054_05c[3];

    /* 0x060 */
    union {
        struct {
            REG_CONST uint32_t _WOX_BOOT_ADDR : 16;
            REG_CONST uint32_t _RESERVED_1 : 16;
        };
        REG_CONST uint32_t DATA;
    } WOX_BOOT_ADDR;

    /* 0x064 ~ 0x068 */
    REG_CONST uint32_t          RSV_064_068[2];

    /* 0x06c */
    union {
        struct {
            REG_CONST uint32_t _WO0_ILM_BOOT : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_CFG_CTL2;

    /* 0x070 ~ 0x07c */
    REG_CONST uint32_t          RSV_070_07c[4];

    /* 0x080 */
    union {
        struct {
            REG_CONST uint32_t _WO_GP_IRQ_B : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_GP_IRQ;

    /* 0x084 */
    union {
        struct {
            REG_CONST uint32_t _WO_WED_IRQ_POL : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_WED_CTL;

    /* 0x088 ~ 0x0fc */
    REG_CONST uint32_t          RSV_088_0fc[30];

    /* 0x100 */
    union {
        struct {
            REG_CONST uint32_t _FPGA_RSV : 32;
        };
        REG_CONST uint32_t DATA;
    } FPGA_RSV;

    /* 0x104 ~ 0x14c */
    REG_CONST uint32_t          RSV_104_14c[19];

    /* 0x150 */
    union {
        struct {
            REG_CONST uint32_t _RESERVE0 : 32;
        };
        REG_CONST uint32_t DATA;
    } RESERVE0;

    /* 0x154 ~ 0x1fc */
    REG_CONST uint32_t          RSV_154_1fc[43];

    /* 0x200 */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG0 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG0;

    /* 0x204 */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG1 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG1;

    /* 0x208 */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG2 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG2;

    /* 0x20c */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG3 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG3;

    /* 0x210 */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG4 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG4;

    /* 0x214 */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG5 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG5;

    /* 0x218 */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG6 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG6;

    /* 0x21c */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG7 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG7;

    /* 0x220 */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG8 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG8;

    /* 0x224 */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG9 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG9;

    /* 0x228 */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG10 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG10;

    /* 0x22c */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG11 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG11;

    /* 0x230 */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG12 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG12;

    /* 0x234 */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG13 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG13;

    /* 0x238 */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG14 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG14;

    /* 0x23c */
    union {
        struct {
            REG_CONST uint32_t _WOX_CON_REG15 : 32;
        };
        REG_CONST uint32_t DATA;
    } WOX_COM_REG15;

    /* 0x240 ~ 0x31c */
    REG_CONST uint32_t          RSV_240_31c[56];

    /* 0x320 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA0 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA0;

    /* 0x324 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA1 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA1;

    /* 0x328 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA2 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA2;

    /* 0x32c */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA3 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA3;

    /* 0x330 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA4 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA4;

    /* 0x334 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA5 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA5;

    /* 0x338 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA6 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA6;

    /* 0x33c */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA7 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA7;

    /* 0x340 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA8 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA8;

    /* 0x344 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA9 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA9;

    /* 0x348 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA10 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA10;

    /* 0x34c */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA11 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA11;

    /* 0x350 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA12 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA12;

    /* 0x354 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA13 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA13;

    /* 0x358 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA14 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA14;

    /* 0x35c */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA15 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA15;

    /* 0x360 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA16 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA16;

    /* 0x364 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA17 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA17;

    /* 0x368 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA18 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA18;

    /* 0x36c */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA19 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA19;

    /* 0x370 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA20 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA20;

    /* 0x374 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA21 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA21;

    /* 0x378 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA22 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA22;

    /* 0x37c */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA23 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA23;

    /* 0x380 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA24 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA24;

    /* 0x384 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA25 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA25;

    /* 0x388 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA26 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA26;

    /* 0x38c */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA27 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA27;

    /* 0x390 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA28 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA28;

    /* 0x394 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA29 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA29;

    /* 0x398 */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA30 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA30;

    /* 0x39c */
    union {
        struct {
            REG_CONST uint32_t _WOX_SEMA31 : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WOX_SEMA31;

    /* 0x3a0 ~ 0x3fc */
    REG_CONST uint32_t          RSV_3a0_3fc[24];

    /* 0x400 */
    union {
        struct {
            REG_CONST uint32_t _DEBUG_SELECT : 24;
            REG_CONST uint32_t _RESERVED_1 : 8;
        };
        REG_CONST uint32_t DATA;
    } DBGSR;

    /* 0x404 */
    union {
        struct {
            REG_CONST uint32_t _DEBUG_SELECT1 : 8;
            REG_CONST uint32_t _RESERVED_1 : 24;
        };
        REG_CONST uint32_t DATA;
    } DBGSR1;

    /* 0x408 */
    union {
        struct {
            REG_CONST uint32_t _DEBUG_SELECT2 : 32;
        };
        REG_CONST uint32_t DATA;
    } DBGSR2;

    /* 0x40c ~ 0x42c */
    REG_CONST uint32_t          RSV_40c_42c[9];

    /* 0x430 */
    union {
        struct {
            REG_CONST uint32_t _RESERVED_0 : 4;
            REG_CONST uint32_t _BUS_HANG_HBURST : 3;
            REG_CONST uint32_t _BUS_HANG_WRITE : 1;
            REG_CONST uint32_t _BUS_HANG_HTRANS : 2;
            REG_CONST uint32_t _RESERVED_4 : 22;
        };
        REG_CONST uint32_t DATA;
    } BUSHANGCTRLA;

    /* 0x434 ~ 0x43c */
    REG_CONST uint32_t          RSV_434_43c[3];

    /* 0x440 */
    union {
        struct {
            REG_CONST uint32_t _BUS_HANG_TIME_LIMIT : 8;
            REG_CONST uint32_t _RESERVED_1 : 21;
            REG_CONST uint32_t _BUS_HANG_IRQ_CLR : 1;
            REG_CONST uint32_t _BUS_HANG_SLV_HREADY_SEL : 1;
            REG_CONST uint32_t _BUS_HANG_DEBUG_EN : 1;
        };
        REG_CONST uint32_t DATA;
    } BUSHANGCR;

    /* 0x444 */
    union {
        struct {
            REG_CONST uint32_t _AHB_TIMEOUT_HADDR_LATCH : 32;
        };
        REG_CONST uint32_t DATA;
    } BUSHANGADDR;

    /* 0x448 ~ 0x448 */
    REG_CONST uint32_t          RSV_448_448[1];

    /* 0x44c */
    union {
        struct {
            REG_CONST uint32_t _BUS_HANG_ID : 10;
            REG_CONST uint32_t _RESERVED_1 : 22;
        };
        REG_CONST uint32_t DATA;
    } BUSHANGID;

    /* 0x450 */
    union {
        struct {
            REG_CONST uint32_t _BUS_HANG_BUS : 32;
        };
        REG_CONST uint32_t DATA;
    } BUSHANGBUS;

    /* 0x454 ~ 0x90c */
    REG_CONST uint32_t          RSV_454_90c[303];

    /* 0x910 */
    union {
        struct {
            REG_CONST uint32_t _VDNR_BUS_DEBUG_MON_SEL : 32;
        };
        REG_CONST uint32_t DATA;
    } VDNR_BUS_DEBUG_MON_SEL;

    /* 0x914 */
    union {
        struct {
            REG_CONST uint32_t _VDNR_DEBUG_CTRL_AO_DEBUGSYS_CTRL : 32;
        };
        REG_CONST uint32_t DATA;
    } VDNR_BUS_DEBUG_CTRL_0;

    /* 0x918 */
    union {
        struct {
            REG_CONST uint32_t _VDNR_DEBUG_CTRL_AO_IO_CTRL : 8;
            REG_CONST uint32_t _RESERVED_1 : 24;
        };
        REG_CONST uint32_t DATA;
    } VDNR_BUS_DEBUG_CTRL_1;
} wo2_mcu_cfg_ls_REG_T;

#define WOX_MCU_CFG_LS_HW_VER_OFFSET                           (0x000) // 4000
#define WOX_MCU_CFG_LS_FW_VER_OFFSET                           (0x004) // 4004
#define WOX_MCU_CFG_LS_CFG_DBG1_OFFSET                         (0x00C) // 400C
#define WOX_MCU_CFG_LS_CFG_DBG2_OFFSET                         (0x010) // 4010
#define WOX_MCU_CFG_LS_WOX_MCCR_OFFSET                         (0x014) // 4014
#define WOX_MCU_CFG_LS_WOX_MCCR_SET_OFFSET                     (0x018) // 4018
#define WOX_MCU_CFG_LS_WOX_MCCR_CLR_OFFSET                     (0x01C) // 401C
#define WOX_MCU_CFG_LS_WOX_MCU_CFG_WO0_WO1_OFFSET              (0x050) // 4050
#define WOX_MCU_CFG_LS_WOX_BOOT_ADDR_OFFSET                    (0x060) // 4060
#define WOX_MCU_CFG_LS_WOX_CFG_CTL2_OFFSET                     (0x06C) // 406C
#define WOX_MCU_CFG_LS_WOX_GP_IRQ_OFFSET                       (0x080) // 4080
#define WOX_MCU_CFG_LS_WOX_WED_CTL_OFFSET                      (0x084) // 4084
#define WOX_MCU_CFG_LS_FPGA_RSV_OFFSET                         (0x100) // 4100
#define WOX_MCU_CFG_LS_RESERVE0_OFFSET                         (0x150) // 4150
#define WOX_MCU_CFG_LS_WOX_COM_REG0_OFFSET                     (0x200) // 4200
#define WOX_MCU_CFG_LS_WOX_COM_REG1_OFFSET                     (0x204) // 4204
#define WOX_MCU_CFG_LS_WOX_COM_REG2_OFFSET                     (0x208) // 4208
#define WOX_MCU_CFG_LS_WOX_COM_REG3_OFFSET                     (0x20C) // 420C
#define WOX_MCU_CFG_LS_WOX_COM_REG4_OFFSET                     (0x210) // 4210
#define WOX_MCU_CFG_LS_WOX_COM_REG5_OFFSET                     (0x214) // 4214
#define WOX_MCU_CFG_LS_WOX_COM_REG6_OFFSET                     (0x218) // 4218
#define WOX_MCU_CFG_LS_WOX_COM_REG7_OFFSET                     (0x21C) // 421C
#define WOX_MCU_CFG_LS_WOX_COM_REG8_OFFSET                     (0x220) // 4220
#define WOX_MCU_CFG_LS_WOX_COM_REG9_OFFSET                     (0x224) // 4224
#define WOX_MCU_CFG_LS_WOX_COM_REG10_OFFSET                    (0x228) // 4228
#define WOX_MCU_CFG_LS_WOX_COM_REG11_OFFSET                    (0x22C) // 422C
#define WOX_MCU_CFG_LS_WOX_COM_REG12_OFFSET                    (0x230) // 4230
#define WOX_MCU_CFG_LS_WOX_COM_REG13_OFFSET                    (0x234) // 4234
#define WOX_MCU_CFG_LS_WOX_COM_REG14_OFFSET                    (0x238) // 4238
#define WOX_MCU_CFG_LS_WOX_COM_REG15_OFFSET                    (0x23C) // 423C
#define WOX_MCU_CFG_LS_WOX_SEMA0_OFFSET                        (0x320) // 4320
#define WOX_MCU_CFG_LS_WOX_SEMA1_OFFSET                        (0x324) // 4324
#define WOX_MCU_CFG_LS_WOX_SEMA2_OFFSET                        (0x328) // 4328
#define WOX_MCU_CFG_LS_WOX_SEMA3_OFFSET                        (0x32C) // 432C
#define WOX_MCU_CFG_LS_WOX_SEMA4_OFFSET                        (0x330) // 4330
#define WOX_MCU_CFG_LS_WOX_SEMA5_OFFSET                        (0x334) // 4334
#define WOX_MCU_CFG_LS_WOX_SEMA6_OFFSET                        (0x338) // 4338
#define WOX_MCU_CFG_LS_WOX_SEMA7_OFFSET                        (0x33C) // 433C
#define WOX_MCU_CFG_LS_WOX_SEMA8_OFFSET                        (0x340) // 4340
#define WOX_MCU_CFG_LS_WOX_SEMA9_OFFSET                        (0x344) // 4344
#define WOX_MCU_CFG_LS_WOX_SEMA10_OFFSET                       (0x348) // 4348
#define WOX_MCU_CFG_LS_WOX_SEMA11_OFFSET                       (0x34C) // 434C
#define WOX_MCU_CFG_LS_WOX_SEMA12_OFFSET                       (0x350) // 4350
#define WOX_MCU_CFG_LS_WOX_SEMA13_OFFSET                       (0x354) // 4354
#define WOX_MCU_CFG_LS_WOX_SEMA14_OFFSET                       (0x358) // 4358
#define WOX_MCU_CFG_LS_WOX_SEMA15_OFFSET                       (0x35C) // 435C
#define WOX_MCU_CFG_LS_WOX_SEMA16_OFFSET                       (0x360) // 4360
#define WOX_MCU_CFG_LS_WOX_SEMA17_OFFSET                       (0x364) // 4364
#define WOX_MCU_CFG_LS_WOX_SEMA18_OFFSET                       (0x368) // 4368
#define WOX_MCU_CFG_LS_WOX_SEMA19_OFFSET                       (0x36C) // 436C
#define WOX_MCU_CFG_LS_WOX_SEMA20_OFFSET                       (0x370) // 4370
#define WOX_MCU_CFG_LS_WOX_SEMA21_OFFSET                       (0x374) // 4374
#define WOX_MCU_CFG_LS_WOX_SEMA22_OFFSET                       (0x378) // 4378
#define WOX_MCU_CFG_LS_WOX_SEMA23_OFFSET                       (0x37C) // 437C
#define WOX_MCU_CFG_LS_WOX_SEMA24_OFFSET                       (0x380) // 4380
#define WOX_MCU_CFG_LS_WOX_SEMA25_OFFSET                       (0x384) // 4384
#define WOX_MCU_CFG_LS_WOX_SEMA26_OFFSET                       (0x388) // 4388
#define WOX_MCU_CFG_LS_WOX_SEMA27_OFFSET                       (0x38C) // 438C
#define WOX_MCU_CFG_LS_WOX_SEMA28_OFFSET                       (0x390) // 4390
#define WOX_MCU_CFG_LS_WOX_SEMA29_OFFSET                       (0x394) // 4394
#define WOX_MCU_CFG_LS_WOX_SEMA30_OFFSET                       (0x398) // 4398
#define WOX_MCU_CFG_LS_WOX_SEMA31_OFFSET                       (0x39C) // 439C
#define WOX_MCU_CFG_LS_DBGSR_OFFSET                            (0x400) // 4400
#define WOX_MCU_CFG_LS_DBGSR1_OFFSET                           (0x404) // 4404
#define WOX_MCU_CFG_LS_DBGSR2_OFFSET                           (0x408) // 4408
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_OFFSET                     (0x0430) // 4430
#define WOX_MCU_CFG_LS_BUSHANGCR_OFFSET                        (0x0440) // 4440
#define WOX_MCU_CFG_LS_BUSHANGADDR_OFFSET                      (0x0444) // 4444
#define WOX_MCU_CFG_LS_BUSHANGID_OFFSET                        (0x044C) // 444C
#define WOX_MCU_CFG_LS_BUSHANGBUS_OFFSET                       (0x0450) // 4450
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_MON_SEL_OFFSET           (0x0910) // 4910
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_0_OFFSET            (0x0914) // 4914
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_1_OFFSET            (0x0918) // 4918

/* =====================================================================================

  ---HW_VER (0x15394000 + 0x000)---

    HW_VER[31..0]                - (RO) CONNSYS Version

 =====================================================================================*/
#define WOX_MCU_CFG_LS_HW_VER_HW_VER_OFFSET                    WOX_MCU_CFG_LS_HW_VER_OFFSET
#define WOX_MCU_CFG_LS_HW_VER_HW_VER_MASK                      0xFFFFFFFF                // HW_VER[31..0]
#define WOX_MCU_CFG_LS_HW_VER_HW_VER_SHFT                      0

/* =====================================================================================

  ---FW_VER (0x15394000 + 0x004)---

    FW_VER[31..0]                - (RO) Firmware Version

 =====================================================================================*/
#define WOX_MCU_CFG_LS_FW_VER_FW_VER_OFFSET                    WOX_MCU_CFG_LS_FW_VER_OFFSET
#define WOX_MCU_CFG_LS_FW_VER_FW_VER_MASK                      0xFFFFFFFF                // FW_VER[31..0]
#define WOX_MCU_CFG_LS_FW_VER_FW_VER_SHFT                      0

/* =====================================================================================

  ---CFG_DBG1 (0x15394000 + 0x00C)---

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
#define WOX_MCU_CFG_LS_CFG_DBG1_JTAG_SEL_OFFSET                WOX_MCU_CFG_LS_CFG_DBG1_OFFSET
#define WOX_MCU_CFG_LS_CFG_DBG1_JTAG_SEL_MASK                  0x00000020                // JTAG_SEL[5]
#define WOX_MCU_CFG_LS_CFG_DBG1_JTAG_SEL_SHFT                  5
#define WOX_MCU_CFG_LS_CFG_DBG1_JTAG_SEL_EN_OFFSET             WOX_MCU_CFG_LS_CFG_DBG1_OFFSET
#define WOX_MCU_CFG_LS_CFG_DBG1_JTAG_SEL_EN_MASK               0x00000010                // JTAG_SEL_EN[4]
#define WOX_MCU_CFG_LS_CFG_DBG1_JTAG_SEL_EN_SHFT               4
#define WOX_MCU_CFG_LS_CFG_DBG1_DEBUG_N3_EN_OFFSET             WOX_MCU_CFG_LS_CFG_DBG1_OFFSET
#define WOX_MCU_CFG_LS_CFG_DBG1_DEBUG_N3_EN_MASK               0x00000008                // DEBUG_N3_EN[3]
#define WOX_MCU_CFG_LS_CFG_DBG1_DEBUG_N3_EN_SHFT               3
#define WOX_MCU_CFG_LS_CFG_DBG1_DEBUG_N2_EN_OFFSET             WOX_MCU_CFG_LS_CFG_DBG1_OFFSET
#define WOX_MCU_CFG_LS_CFG_DBG1_DEBUG_N2_EN_MASK               0x00000004                // DEBUG_N2_EN[2]
#define WOX_MCU_CFG_LS_CFG_DBG1_DEBUG_N2_EN_SHFT               2
#define WOX_MCU_CFG_LS_CFG_DBG1_DEBUG_N1_EN_OFFSET             WOX_MCU_CFG_LS_CFG_DBG1_OFFSET
#define WOX_MCU_CFG_LS_CFG_DBG1_DEBUG_N1_EN_MASK               0x00000002                // DEBUG_N1_EN[1]
#define WOX_MCU_CFG_LS_CFG_DBG1_DEBUG_N1_EN_SHFT               1
#define WOX_MCU_CFG_LS_CFG_DBG1_DEBUG_N0_EN_OFFSET             WOX_MCU_CFG_LS_CFG_DBG1_OFFSET
#define WOX_MCU_CFG_LS_CFG_DBG1_DEBUG_N0_EN_MASK               0x00000001                // DEBUG_N0_EN[0]
#define WOX_MCU_CFG_LS_CFG_DBG1_DEBUG_N0_EN_SHFT               0

/* =====================================================================================

  ---CFG_DBG2 (0x15394000 + 0x010)---

    DEBUG_L1_N0_MOD_SEL[7..0]    - (RW) Debug flag selection
    DEBUG_L1_N1_MOD_SEL[15..8]   - (RW) Debug flag selection
    DEBUG_L1_N2_MOD_SEL[23..16]  - (RW) Debug flag selection
    DEBUG_L1_N3_MOD_SEL[31..24]  - (RW) Debug flag selection

 =====================================================================================*/
#define WOX_MCU_CFG_LS_CFG_DBG2_DEBUG_L1_N3_MOD_SEL_OFFSET     WOX_MCU_CFG_LS_CFG_DBG2_OFFSET
#define WOX_MCU_CFG_LS_CFG_DBG2_DEBUG_L1_N3_MOD_SEL_MASK       0xFF000000                // DEBUG_L1_N3_MOD_SEL[31..24]
#define WOX_MCU_CFG_LS_CFG_DBG2_DEBUG_L1_N3_MOD_SEL_SHFT       24
#define WOX_MCU_CFG_LS_CFG_DBG2_DEBUG_L1_N2_MOD_SEL_OFFSET     WOX_MCU_CFG_LS_CFG_DBG2_OFFSET
#define WOX_MCU_CFG_LS_CFG_DBG2_DEBUG_L1_N2_MOD_SEL_MASK       0x00FF0000                // DEBUG_L1_N2_MOD_SEL[23..16]
#define WOX_MCU_CFG_LS_CFG_DBG2_DEBUG_L1_N2_MOD_SEL_SHFT       16
#define WOX_MCU_CFG_LS_CFG_DBG2_DEBUG_L1_N1_MOD_SEL_OFFSET     WOX_MCU_CFG_LS_CFG_DBG2_OFFSET
#define WOX_MCU_CFG_LS_CFG_DBG2_DEBUG_L1_N1_MOD_SEL_MASK       0x0000FF00                // DEBUG_L1_N1_MOD_SEL[15..8]
#define WOX_MCU_CFG_LS_CFG_DBG2_DEBUG_L1_N1_MOD_SEL_SHFT       8
#define WOX_MCU_CFG_LS_CFG_DBG2_DEBUG_L1_N0_MOD_SEL_OFFSET     WOX_MCU_CFG_LS_CFG_DBG2_OFFSET
#define WOX_MCU_CFG_LS_CFG_DBG2_DEBUG_L1_N0_MOD_SEL_MASK       0x000000FF                // DEBUG_L1_N0_MOD_SEL[7..0]
#define WOX_MCU_CFG_LS_CFG_DBG2_DEBUG_L1_N0_MOD_SEL_SHFT       0

/* =====================================================================================

  ---WOX_MCCR (0x15394000 + 0x014)---

    RESERVED0[9..0]              - (RO) Reserved bits
    UART_DBG_STOP[11..10]        - (RO) UART_DBG STOP control
                                     Triggered by CR set and released by CR clear
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_MCCR_UART_DBG_STOP_OFFSET           WOX_MCU_CFG_LS_WOX_MCCR_OFFSET
#define WOX_MCU_CFG_LS_WOX_MCCR_UART_DBG_STOP_MASK             0x00000C00                // UART_DBG_STOP[11..10]
#define WOX_MCU_CFG_LS_WOX_MCCR_UART_DBG_STOP_SHFT             10

/* =====================================================================================

  ---WOX_MCCR_SET (0x15394000 + 0x018)---

    RESERVED0[9..0]              - (RO) Reserved bits
    UART_DBG[11..10]             - (W1S) UART_DBG_STOP Set. Write 1 to set stop
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_MCCR_SET_UART_DBG_OFFSET            WOX_MCU_CFG_LS_WOX_MCCR_SET_OFFSET
#define WOX_MCU_CFG_LS_WOX_MCCR_SET_UART_DBG_MASK              0x00000C00                // UART_DBG[11..10]
#define WOX_MCU_CFG_LS_WOX_MCCR_SET_UART_DBG_SHFT              10

/* =====================================================================================

  ---WOX_MCCR_CLR (0x15394000 + 0x01C)---

    RESERVED0[9..0]              - (RO) Reserved bits
    UART_DBG[11..10]             - (W1C) UART_DBG_STOP Clear. Write 1 to set clear
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_MCCR_CLR_UART_DBG_OFFSET            WOX_MCU_CFG_LS_WOX_MCCR_CLR_OFFSET
#define WOX_MCU_CFG_LS_WOX_MCCR_CLR_UART_DBG_MASK              0x00000C00                // UART_DBG[11..10]
#define WOX_MCU_CFG_LS_WOX_MCCR_CLR_UART_DBG_SHFT              10

/* =====================================================================================

  ---WOX_MCU_CFG_WO0_WO1 (0x15394000 + 0x050)---

    RESERVED0[4..0]              - (RO) Reserved bits
    WOX_CPU_RSTB[5]              - (RW) WO0 CPU software reset (low active)
    RESERVED6[31..6]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_MCU_CFG_WO0_WO1_WOX_CPU_RSTB_OFFSET WOX_MCU_CFG_LS_WOX_MCU_CFG_WO0_WO1_OFFSET
#define WOX_MCU_CFG_LS_WOX_MCU_CFG_WO0_WO1_WOX_CPU_RSTB_MASK   0x00000020                // WOX_CPU_RSTB[5]
#define WOX_MCU_CFG_LS_WOX_MCU_CFG_WO0_WO1_WOX_CPU_RSTB_SHFT   5

/* =====================================================================================

  ---WOX_BOOT_ADDR (0x15394000 + 0x060)---

    WOX_BOOT_ADDR[15..0]         - (RW) WO0 CPU Boot address MSB
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_BOOT_ADDR_WOX_BOOT_ADDR_OFFSET      WOX_MCU_CFG_LS_WOX_BOOT_ADDR_OFFSET
#define WOX_MCU_CFG_LS_WOX_BOOT_ADDR_WOX_BOOT_ADDR_MASK        0x0000FFFF                // WOX_BOOT_ADDR[15..0]
#define WOX_MCU_CFG_LS_WOX_BOOT_ADDR_WOX_BOOT_ADDR_SHFT        0

/* =====================================================================================

  ---WOX_CFG_CTL2 (0x15394000 + 0x06C)---

    WO0_ILM_BOOT[0]              - (RW) WO0 CPU ILM_BOOT enable
                                     0: Disable
                                     1: Enable
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_CFG_CTL2_WO0_ILM_BOOT_OFFSET        WOX_MCU_CFG_LS_WOX_CFG_CTL2_OFFSET
#define WOX_MCU_CFG_LS_WOX_CFG_CTL2_WO0_ILM_BOOT_MASK          0x00000001                // WO0_ILM_BOOT[0]
#define WOX_MCU_CFG_LS_WOX_CFG_CTL2_WO0_ILM_BOOT_SHFT          0

/* =====================================================================================

  ---WOX_GP_IRQ (0x15394000 + 0x080)---

    WO_GP_IRQ_B[0]               - (RW) WO0 CPU GP_IRQ_B (low active)
                                     0: active
                                     1: inactive
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_GP_IRQ_WO_GP_IRQ_B_OFFSET           WOX_MCU_CFG_LS_WOX_GP_IRQ_OFFSET
#define WOX_MCU_CFG_LS_WOX_GP_IRQ_WO_GP_IRQ_B_MASK             0x00000001                // WO_GP_IRQ_B[0]
#define WOX_MCU_CFG_LS_WOX_GP_IRQ_WO_GP_IRQ_B_SHFT             0

/* =====================================================================================

  ---WOX_WED_CTL (0x15394000 + 0x084)---

    WO_WED_IRQ_POL[0]            - (RW) WO0 CPU WED IRQ polarity configure
                                     0: low active
                                     1: high active
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_WED_CTL_WO_WED_IRQ_POL_OFFSET       WOX_MCU_CFG_LS_WOX_WED_CTL_OFFSET
#define WOX_MCU_CFG_LS_WOX_WED_CTL_WO_WED_IRQ_POL_MASK         0x00000001                // WO_WED_IRQ_POL[0]
#define WOX_MCU_CFG_LS_WOX_WED_CTL_WO_WED_IRQ_POL_SHFT         0

/* =====================================================================================

  ---FPGA_RSV (0x15394000 + 0x100)---

    FPGA_RSV[31..0]              - (RW) FPGA reserve register

 =====================================================================================*/
#define WOX_MCU_CFG_LS_FPGA_RSV_FPGA_RSV_OFFSET                WOX_MCU_CFG_LS_FPGA_RSV_OFFSET
#define WOX_MCU_CFG_LS_FPGA_RSV_FPGA_RSV_MASK                  0xFFFFFFFF                // FPGA_RSV[31..0]
#define WOX_MCU_CFG_LS_FPGA_RSV_FPGA_RSV_SHFT                  0

/* =====================================================================================

  ---RESERVE0 (0x15394000 + 0x150)---

    RESERVE0[31..0]              - (RW) RESERVE0

 =====================================================================================*/
#define WOX_MCU_CFG_LS_RESERVE0_RESERVE0_OFFSET                WOX_MCU_CFG_LS_RESERVE0_OFFSET
#define WOX_MCU_CFG_LS_RESERVE0_RESERVE0_MASK                  0xFFFFFFFF                // RESERVE0[31..0]
#define WOX_MCU_CFG_LS_RESERVE0_RESERVE0_SHFT                  0

/* =====================================================================================

  ---WOX_COM_REG0 (0x15394000 + 0x200)---

    WOX_CON_REG0[31..0]          - (RW) WOX_MCUSYS COMMON register no.0

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG0_WOX_CON_REG0_OFFSET        WOX_MCU_CFG_LS_WOX_COM_REG0_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG0_WOX_CON_REG0_MASK          0xFFFFFFFF                // WOX_CON_REG0[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG0_WOX_CON_REG0_SHFT          0

/* =====================================================================================

  ---WOX_COM_REG1 (0x15394000 + 0x204)---

    WOX_CON_REG1[31..0]          - (RW) WOX_MCUSYS COMMON register no.1

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG1_WOX_CON_REG1_OFFSET        WOX_MCU_CFG_LS_WOX_COM_REG1_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG1_WOX_CON_REG1_MASK          0xFFFFFFFF                // WOX_CON_REG1[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG1_WOX_CON_REG1_SHFT          0

/* =====================================================================================

  ---WOX_COM_REG2 (0x15394000 + 0x208)---

    WOX_CON_REG2[31..0]          - (RW) WOX_MCUSYS COMMON register no.2

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG2_WOX_CON_REG2_OFFSET        WOX_MCU_CFG_LS_WOX_COM_REG2_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG2_WOX_CON_REG2_MASK          0xFFFFFFFF                // WOX_CON_REG2[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG2_WOX_CON_REG2_SHFT          0

/* =====================================================================================

  ---WOX_COM_REG3 (0x15394000 + 0x20C)---

    WOX_CON_REG3[31..0]          - (RW) WOX_MCUSYS COMMON register no.3

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG3_WOX_CON_REG3_OFFSET        WOX_MCU_CFG_LS_WOX_COM_REG3_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG3_WOX_CON_REG3_MASK          0xFFFFFFFF                // WOX_CON_REG3[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG3_WOX_CON_REG3_SHFT          0

/* =====================================================================================

  ---WOX_COM_REG4 (0x15394000 + 0x210)---

    WOX_CON_REG4[31..0]          - (RW) WOX_MCUSYS COMMON register no.4

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG4_WOX_CON_REG4_OFFSET        WOX_MCU_CFG_LS_WOX_COM_REG4_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG4_WOX_CON_REG4_MASK          0xFFFFFFFF                // WOX_CON_REG4[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG4_WOX_CON_REG4_SHFT          0

/* =====================================================================================

  ---WOX_COM_REG5 (0x15394000 + 0x214)---

    WOX_CON_REG5[31..0]          - (RW) WOX_MCUSYS COMMON register no.5

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG5_WOX_CON_REG5_OFFSET        WOX_MCU_CFG_LS_WOX_COM_REG5_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG5_WOX_CON_REG5_MASK          0xFFFFFFFF                // WOX_CON_REG5[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG5_WOX_CON_REG5_SHFT          0

/* =====================================================================================

  ---WOX_COM_REG6 (0x15394000 + 0x218)---

    WOX_CON_REG6[31..0]          - (RW) WOX_MCUSYS COMMON register no.6

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG6_WOX_CON_REG6_OFFSET        WOX_MCU_CFG_LS_WOX_COM_REG6_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG6_WOX_CON_REG6_MASK          0xFFFFFFFF                // WOX_CON_REG6[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG6_WOX_CON_REG6_SHFT          0

/* =====================================================================================

  ---WOX_COM_REG7 (0x15394000 + 0x21C)---

    WOX_CON_REG7[31..0]          - (RW) WOX_MCUSYS COMMON register no.7

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG7_WOX_CON_REG7_OFFSET        WOX_MCU_CFG_LS_WOX_COM_REG7_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG7_WOX_CON_REG7_MASK          0xFFFFFFFF                // WOX_CON_REG7[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG7_WOX_CON_REG7_SHFT          0

/* =====================================================================================

  ---WOX_COM_REG8 (0x15394000 + 0x220)---

    WOX_CON_REG8[31..0]          - (RW) WOX_MCUSYS COMMON register no.8

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG8_WOX_CON_REG8_OFFSET        WOX_MCU_CFG_LS_WOX_COM_REG8_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG8_WOX_CON_REG8_MASK          0xFFFFFFFF                // WOX_CON_REG8[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG8_WOX_CON_REG8_SHFT          0

/* =====================================================================================

  ---WOX_COM_REG9 (0x15394000 + 0x224)---

    WOX_CON_REG9[31..0]          - (RW) WOX_MCUSYS COMMON register no.9

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG9_WOX_CON_REG9_OFFSET        WOX_MCU_CFG_LS_WOX_COM_REG9_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG9_WOX_CON_REG9_MASK          0xFFFFFFFF                // WOX_CON_REG9[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG9_WOX_CON_REG9_SHFT          0

/* =====================================================================================

  ---WOX_COM_REG10 (0x15394000 + 0x228)---

    WOX_CON_REG10[31..0]         - (RW) WOX_MCUSYS COMMON register no.10

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG10_WOX_CON_REG10_OFFSET      WOX_MCU_CFG_LS_WOX_COM_REG10_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG10_WOX_CON_REG10_MASK        0xFFFFFFFF                // WOX_CON_REG10[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG10_WOX_CON_REG10_SHFT        0

/* =====================================================================================

  ---WOX_COM_REG11 (0x15394000 + 0x22C)---

    WOX_CON_REG11[31..0]         - (RW) WOX_MCUSYS COMMON register no.11

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG11_WOX_CON_REG11_OFFSET      WOX_MCU_CFG_LS_WOX_COM_REG11_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG11_WOX_CON_REG11_MASK        0xFFFFFFFF                // WOX_CON_REG11[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG11_WOX_CON_REG11_SHFT        0

/* =====================================================================================

  ---WOX_COM_REG12 (0x15394000 + 0x230)---

    WOX_CON_REG12[31..0]         - (RW) WOX_MCUSYS COMMON register no.12

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG12_WOX_CON_REG12_OFFSET      WOX_MCU_CFG_LS_WOX_COM_REG12_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG12_WOX_CON_REG12_MASK        0xFFFFFFFF                // WOX_CON_REG12[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG12_WOX_CON_REG12_SHFT        0

/* =====================================================================================

  ---WOX_COM_REG13 (0x15394000 + 0x234)---

    WOX_CON_REG13[31..0]         - (RW) WOX_MCUSYS COMMON register no.13

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG13_WOX_CON_REG13_OFFSET      WOX_MCU_CFG_LS_WOX_COM_REG13_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG13_WOX_CON_REG13_MASK        0xFFFFFFFF                // WOX_CON_REG13[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG13_WOX_CON_REG13_SHFT        0

/* =====================================================================================

  ---WOX_COM_REG14 (0x15394000 + 0x238)---

    WOX_CON_REG14[31..0]         - (RW) WOX_MCUSYS COMMON register no.14

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG14_WOX_CON_REG14_OFFSET      WOX_MCU_CFG_LS_WOX_COM_REG14_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG14_WOX_CON_REG14_MASK        0xFFFFFFFF                // WOX_CON_REG14[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG14_WOX_CON_REG14_SHFT        0

/* =====================================================================================

  ---WOX_COM_REG15 (0x15394000 + 0x23C)---

    WOX_CON_REG15[31..0]         - (RW) WOX_MCUSYS COMMON register no.15

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_COM_REG15_WOX_CON_REG15_OFFSET      WOX_MCU_CFG_LS_WOX_COM_REG15_OFFSET
#define WOX_MCU_CFG_LS_WOX_COM_REG15_WOX_CON_REG15_MASK        0xFFFFFFFF                // WOX_CON_REG15[31..0]
#define WOX_MCU_CFG_LS_WOX_COM_REG15_WOX_CON_REG15_SHFT        0

/* =====================================================================================

  ---WOX_SEMA0 (0x15394000 + 0x320)---

    WOX_SEMA0[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA0_WOX_SEMA0_OFFSET              WOX_MCU_CFG_LS_WOX_SEMA0_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA0_WOX_SEMA0_MASK                0x00000001                // WOX_SEMA0[0]
#define WOX_MCU_CFG_LS_WOX_SEMA0_WOX_SEMA0_SHFT                0

/* =====================================================================================

  ---WOX_SEMA1 (0x15394000 + 0x324)---

    WOX_SEMA1[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA1_WOX_SEMA1_OFFSET              WOX_MCU_CFG_LS_WOX_SEMA1_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA1_WOX_SEMA1_MASK                0x00000001                // WOX_SEMA1[0]
#define WOX_MCU_CFG_LS_WOX_SEMA1_WOX_SEMA1_SHFT                0

/* =====================================================================================

  ---WOX_SEMA2 (0x15394000 + 0x328)---

    WOX_SEMA2[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA2_WOX_SEMA2_OFFSET              WOX_MCU_CFG_LS_WOX_SEMA2_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA2_WOX_SEMA2_MASK                0x00000001                // WOX_SEMA2[0]
#define WOX_MCU_CFG_LS_WOX_SEMA2_WOX_SEMA2_SHFT                0

/* =====================================================================================

  ---WOX_SEMA3 (0x15394000 + 0x32C)---

    WOX_SEMA3[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA3_WOX_SEMA3_OFFSET              WOX_MCU_CFG_LS_WOX_SEMA3_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA3_WOX_SEMA3_MASK                0x00000001                // WOX_SEMA3[0]
#define WOX_MCU_CFG_LS_WOX_SEMA3_WOX_SEMA3_SHFT                0

/* =====================================================================================

  ---WOX_SEMA4 (0x15394000 + 0x330)---

    WOX_SEMA4[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA4_WOX_SEMA4_OFFSET              WOX_MCU_CFG_LS_WOX_SEMA4_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA4_WOX_SEMA4_MASK                0x00000001                // WOX_SEMA4[0]
#define WOX_MCU_CFG_LS_WOX_SEMA4_WOX_SEMA4_SHFT                0

/* =====================================================================================

  ---WOX_SEMA5 (0x15394000 + 0x334)---

    WOX_SEMA5[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA5_WOX_SEMA5_OFFSET              WOX_MCU_CFG_LS_WOX_SEMA5_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA5_WOX_SEMA5_MASK                0x00000001                // WOX_SEMA5[0]
#define WOX_MCU_CFG_LS_WOX_SEMA5_WOX_SEMA5_SHFT                0

/* =====================================================================================

  ---WOX_SEMA6 (0x15394000 + 0x338)---

    WOX_SEMA6[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA6_WOX_SEMA6_OFFSET              WOX_MCU_CFG_LS_WOX_SEMA6_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA6_WOX_SEMA6_MASK                0x00000001                // WOX_SEMA6[0]
#define WOX_MCU_CFG_LS_WOX_SEMA6_WOX_SEMA6_SHFT                0

/* =====================================================================================

  ---WOX_SEMA7 (0x15394000 + 0x33C)---

    WOX_SEMA7[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA7_WOX_SEMA7_OFFSET              WOX_MCU_CFG_LS_WOX_SEMA7_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA7_WOX_SEMA7_MASK                0x00000001                // WOX_SEMA7[0]
#define WOX_MCU_CFG_LS_WOX_SEMA7_WOX_SEMA7_SHFT                0

/* =====================================================================================

  ---WOX_SEMA8 (0x15394000 + 0x340)---

    WOX_SEMA8[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA8_WOX_SEMA8_OFFSET              WOX_MCU_CFG_LS_WOX_SEMA8_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA8_WOX_SEMA8_MASK                0x00000001                // WOX_SEMA8[0]
#define WOX_MCU_CFG_LS_WOX_SEMA8_WOX_SEMA8_SHFT                0

/* =====================================================================================

  ---WOX_SEMA9 (0x15394000 + 0x344)---

    WOX_SEMA9[0]                 - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA9_WOX_SEMA9_OFFSET              WOX_MCU_CFG_LS_WOX_SEMA9_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA9_WOX_SEMA9_MASK                0x00000001                // WOX_SEMA9[0]
#define WOX_MCU_CFG_LS_WOX_SEMA9_WOX_SEMA9_SHFT                0

/* =====================================================================================

  ---WOX_SEMA10 (0x15394000 + 0x348)---

    WOX_SEMA10[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA10_WOX_SEMA10_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA10_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA10_WOX_SEMA10_MASK              0x00000001                // WOX_SEMA10[0]
#define WOX_MCU_CFG_LS_WOX_SEMA10_WOX_SEMA10_SHFT              0

/* =====================================================================================

  ---WOX_SEMA11 (0x15394000 + 0x34C)---

    WOX_SEMA11[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA11_WOX_SEMA11_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA11_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA11_WOX_SEMA11_MASK              0x00000001                // WOX_SEMA11[0]
#define WOX_MCU_CFG_LS_WOX_SEMA11_WOX_SEMA11_SHFT              0

/* =====================================================================================

  ---WOX_SEMA12 (0x15394000 + 0x350)---

    WOX_SEMA12[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA12_WOX_SEMA12_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA12_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA12_WOX_SEMA12_MASK              0x00000001                // WOX_SEMA12[0]
#define WOX_MCU_CFG_LS_WOX_SEMA12_WOX_SEMA12_SHFT              0

/* =====================================================================================

  ---WOX_SEMA13 (0x15394000 + 0x354)---

    WOX_SEMA13[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA13_WOX_SEMA13_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA13_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA13_WOX_SEMA13_MASK              0x00000001                // WOX_SEMA13[0]
#define WOX_MCU_CFG_LS_WOX_SEMA13_WOX_SEMA13_SHFT              0

/* =====================================================================================

  ---WOX_SEMA14 (0x15394000 + 0x358)---

    WOX_SEMA14[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA14_WOX_SEMA14_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA14_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA14_WOX_SEMA14_MASK              0x00000001                // WOX_SEMA14[0]
#define WOX_MCU_CFG_LS_WOX_SEMA14_WOX_SEMA14_SHFT              0

/* =====================================================================================

  ---WOX_SEMA15 (0x15394000 + 0x35C)---

    WOX_SEMA15[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA15_WOX_SEMA15_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA15_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA15_WOX_SEMA15_MASK              0x00000001                // WOX_SEMA15[0]
#define WOX_MCU_CFG_LS_WOX_SEMA15_WOX_SEMA15_SHFT              0

/* =====================================================================================

  ---WOX_SEMA16 (0x15394000 + 0x360)---

    WOX_SEMA16[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA16_WOX_SEMA16_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA16_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA16_WOX_SEMA16_MASK              0x00000001                // WOX_SEMA16[0]
#define WOX_MCU_CFG_LS_WOX_SEMA16_WOX_SEMA16_SHFT              0

/* =====================================================================================

  ---WOX_SEMA17 (0x15394000 + 0x364)---

    WOX_SEMA17[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA17_WOX_SEMA17_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA17_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA17_WOX_SEMA17_MASK              0x00000001                // WOX_SEMA17[0]
#define WOX_MCU_CFG_LS_WOX_SEMA17_WOX_SEMA17_SHFT              0

/* =====================================================================================

  ---WOX_SEMA18 (0x15394000 + 0x368)---

    WOX_SEMA18[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA18_WOX_SEMA18_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA18_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA18_WOX_SEMA18_MASK              0x00000001                // WOX_SEMA18[0]
#define WOX_MCU_CFG_LS_WOX_SEMA18_WOX_SEMA18_SHFT              0

/* =====================================================================================

  ---WOX_SEMA19 (0x15394000 + 0x36C)---

    WOX_SEMA19[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA19_WOX_SEMA19_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA19_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA19_WOX_SEMA19_MASK              0x00000001                // WOX_SEMA19[0]
#define WOX_MCU_CFG_LS_WOX_SEMA19_WOX_SEMA19_SHFT              0

/* =====================================================================================

  ---WOX_SEMA20 (0x15394000 + 0x370)---

    WOX_SEMA20[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA20_WOX_SEMA20_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA20_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA20_WOX_SEMA20_MASK              0x00000001                // WOX_SEMA20[0]
#define WOX_MCU_CFG_LS_WOX_SEMA20_WOX_SEMA20_SHFT              0

/* =====================================================================================

  ---WOX_SEMA21 (0x15394000 + 0x374)---

    WOX_SEMA21[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA21_WOX_SEMA21_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA21_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA21_WOX_SEMA21_MASK              0x00000001                // WOX_SEMA21[0]
#define WOX_MCU_CFG_LS_WOX_SEMA21_WOX_SEMA21_SHFT              0

/* =====================================================================================

  ---WOX_SEMA22 (0x15394000 + 0x378)---

    WOX_SEMA22[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA22_WOX_SEMA22_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA22_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA22_WOX_SEMA22_MASK              0x00000001                // WOX_SEMA22[0]
#define WOX_MCU_CFG_LS_WOX_SEMA22_WOX_SEMA22_SHFT              0

/* =====================================================================================

  ---WOX_SEMA23 (0x15394000 + 0x37C)---

    WOX_SEMA23[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA23_WOX_SEMA23_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA23_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA23_WOX_SEMA23_MASK              0x00000001                // WOX_SEMA23[0]
#define WOX_MCU_CFG_LS_WOX_SEMA23_WOX_SEMA23_SHFT              0

/* =====================================================================================

  ---WOX_SEMA24 (0x15394000 + 0x380)---

    WOX_SEMA24[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA24_WOX_SEMA24_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA24_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA24_WOX_SEMA24_MASK              0x00000001                // WOX_SEMA24[0]
#define WOX_MCU_CFG_LS_WOX_SEMA24_WOX_SEMA24_SHFT              0

/* =====================================================================================

  ---WOX_SEMA25 (0x15394000 + 0x384)---

    WOX_SEMA25[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA25_WOX_SEMA25_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA25_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA25_WOX_SEMA25_MASK              0x00000001                // WOX_SEMA25[0]
#define WOX_MCU_CFG_LS_WOX_SEMA25_WOX_SEMA25_SHFT              0

/* =====================================================================================

  ---WOX_SEMA26 (0x15394000 + 0x388)---

    WOX_SEMA26[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA26_WOX_SEMA26_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA26_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA26_WOX_SEMA26_MASK              0x00000001                // WOX_SEMA26[0]
#define WOX_MCU_CFG_LS_WOX_SEMA26_WOX_SEMA26_SHFT              0

/* =====================================================================================

  ---WOX_SEMA27 (0x15394000 + 0x38C)---

    WOX_SEMA27[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA27_WOX_SEMA27_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA27_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA27_WOX_SEMA27_MASK              0x00000001                // WOX_SEMA27[0]
#define WOX_MCU_CFG_LS_WOX_SEMA27_WOX_SEMA27_SHFT              0

/* =====================================================================================

  ---WOX_SEMA28 (0x15394000 + 0x390)---

    WOX_SEMA28[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA28_WOX_SEMA28_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA28_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA28_WOX_SEMA28_MASK              0x00000001                // WOX_SEMA28[0]
#define WOX_MCU_CFG_LS_WOX_SEMA28_WOX_SEMA28_SHFT              0

/* =====================================================================================

  ---WOX_SEMA29 (0x15394000 + 0x394)---

    WOX_SEMA29[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA29_WOX_SEMA29_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA29_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA29_WOX_SEMA29_MASK              0x00000001                // WOX_SEMA29[0]
#define WOX_MCU_CFG_LS_WOX_SEMA29_WOX_SEMA29_SHFT              0

/* =====================================================================================

  ---WOX_SEMA30 (0x15394000 + 0x398)---

    WOX_SEMA30[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA30_WOX_SEMA30_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA30_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA30_WOX_SEMA30_MASK              0x00000001                // WOX_SEMA30[0]
#define WOX_MCU_CFG_LS_WOX_SEMA30_WOX_SEMA30_SHFT              0

/* =====================================================================================

  ---WOX_SEMA31 (0x15394000 + 0x39C)---

    WOX_SEMA31[0]                - (W1SRC) WOX_MCUSYS SEMAPHORE register (Write 1 set, read clear)
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_WOX_SEMA31_WOX_SEMA31_OFFSET            WOX_MCU_CFG_LS_WOX_SEMA31_OFFSET
#define WOX_MCU_CFG_LS_WOX_SEMA31_WOX_SEMA31_MASK              0x00000001                // WOX_SEMA31[0]
#define WOX_MCU_CFG_LS_WOX_SEMA31_WOX_SEMA31_SHFT              0

/* =====================================================================================

  ---DBGSR (0x15394000 + 0x400)---

    DEBUG_SELECT[23..0]          - (RW) Internal debug selection
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_DBGSR_DEBUG_SELECT_OFFSET               WOX_MCU_CFG_LS_DBGSR_OFFSET
#define WOX_MCU_CFG_LS_DBGSR_DEBUG_SELECT_MASK                 0x00FFFFFF                // DEBUG_SELECT[23..0]
#define WOX_MCU_CFG_LS_DBGSR_DEBUG_SELECT_SHFT                 0

/* =====================================================================================

  ---DBGSR1 (0x15394000 + 0x404)---

    DEBUG_SELECT1[7..0]          - (RW) Internal debug selection1
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_DBGSR1_DEBUG_SELECT1_OFFSET             WOX_MCU_CFG_LS_DBGSR1_OFFSET
#define WOX_MCU_CFG_LS_DBGSR1_DEBUG_SELECT1_MASK               0x000000FF                // DEBUG_SELECT1[7..0]
#define WOX_MCU_CFG_LS_DBGSR1_DEBUG_SELECT1_SHFT               0

/* =====================================================================================

  ---DBGSR2 (0x15394000 + 0x408)---

    DEBUG_SELECT2[31..0]         - (RW) Internal debug selection2

 =====================================================================================*/
#define WOX_MCU_CFG_LS_DBGSR2_DEBUG_SELECT2_OFFSET             WOX_MCU_CFG_LS_DBGSR2_OFFSET
#define WOX_MCU_CFG_LS_DBGSR2_DEBUG_SELECT2_MASK               0xFFFFFFFF                // DEBUG_SELECT2[31..0]
#define WOX_MCU_CFG_LS_DBGSR2_DEBUG_SELECT2_SHFT               0

/* =====================================================================================

  ---BUSHANGCTRLA (0x15394000 + 0x0430)---

    RESERVED0[3..0]              - (RO) Reserved bits
    BUS_HANG_HBURST[6..4]        - (RO) bus hang hburst
    BUS_HANG_WRITE[7]            - (RO) bus hang hwrite/pwrite
    BUS_HANG_HTRANS[9..8]        - (RO) bus hang htrans
    RESERVED10[31..10]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_HTRANS_OFFSET     WOX_MCU_CFG_LS_BUSHANGCTRLA_OFFSET
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_HTRANS_MASK       0x00000300                // BUS_HANG_HTRANS[9..8]
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_HTRANS_SHFT       8
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_WRITE_OFFSET      WOX_MCU_CFG_LS_BUSHANGCTRLA_OFFSET
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_WRITE_MASK        0x00000080                // BUS_HANG_WRITE[7]
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_WRITE_SHFT        7
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_HBURST_OFFSET     WOX_MCU_CFG_LS_BUSHANGCTRLA_OFFSET
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_HBURST_MASK       0x00000070                // BUS_HANG_HBURST[6..4]
#define WOX_MCU_CFG_LS_BUSHANGCTRLA_BUS_HANG_HBURST_SHFT       4

/* =====================================================================================

  ---BUSHANGCR (0x15394000 + 0x0440)---

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
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_DEBUG_EN_OFFSET      WOX_MCU_CFG_LS_BUSHANGCR_OFFSET
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_DEBUG_EN_MASK        0x80000000                // BUS_HANG_DEBUG_EN[31]
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_DEBUG_EN_SHFT        31
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_SLV_HREADY_SEL_OFFSET WOX_MCU_CFG_LS_BUSHANGCR_OFFSET
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_SLV_HREADY_SEL_MASK  0x40000000                // BUS_HANG_SLV_HREADY_SEL[30]
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_SLV_HREADY_SEL_SHFT  30
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_IRQ_CLR_OFFSET       WOX_MCU_CFG_LS_BUSHANGCR_OFFSET
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_IRQ_CLR_MASK         0x20000000                // BUS_HANG_IRQ_CLR[29]
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_IRQ_CLR_SHFT         29
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_TIME_LIMIT_OFFSET    WOX_MCU_CFG_LS_BUSHANGCR_OFFSET
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_TIME_LIMIT_MASK      0x000000FF                // BUS_HANG_TIME_LIMIT[7..0]
#define WOX_MCU_CFG_LS_BUSHANGCR_BUS_HANG_TIME_LIMIT_SHFT      0

/* =====================================================================================

  ---BUSHANGADDR (0x15394000 + 0x0444)---

    AHB_TIMEOUT_HADDR_LATCH[31..0] - (RO) Internal bus hang address

 =====================================================================================*/
#define WOX_MCU_CFG_LS_BUSHANGADDR_AHB_TIMEOUT_HADDR_LATCH_OFFSET WOX_MCU_CFG_LS_BUSHANGADDR_OFFSET
#define WOX_MCU_CFG_LS_BUSHANGADDR_AHB_TIMEOUT_HADDR_LATCH_MASK 0xFFFFFFFF                // AHB_TIMEOUT_HADDR_LATCH[31..0]
#define WOX_MCU_CFG_LS_BUSHANGADDR_AHB_TIMEOUT_HADDR_LATCH_SHFT 0

/* =====================================================================================

  ---BUSHANGID (0x15394000 + 0x044C)---

    BUS_HANG_ID[9..0]            - (RO) bus master id record when bus hang
    RESERVED10[31..10]           - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_BUSHANGID_BUS_HANG_ID_OFFSET            WOX_MCU_CFG_LS_BUSHANGID_OFFSET
#define WOX_MCU_CFG_LS_BUSHANGID_BUS_HANG_ID_MASK              0x000003FF                // BUS_HANG_ID[9..0]
#define WOX_MCU_CFG_LS_BUSHANGID_BUS_HANG_ID_SHFT              0

/* =====================================================================================

  ---BUSHANGBUS (0x15394000 + 0x0450)---

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
#define WOX_MCU_CFG_LS_BUSHANGBUS_BUS_HANG_BUS_OFFSET          WOX_MCU_CFG_LS_BUSHANGBUS_OFFSET
#define WOX_MCU_CFG_LS_BUSHANGBUS_BUS_HANG_BUS_MASK            0xFFFFFFFF                // BUS_HANG_BUS[31..0]
#define WOX_MCU_CFG_LS_BUSHANGBUS_BUS_HANG_BUS_SHFT            0

/* =====================================================================================

  ---VDNR_BUS_DEBUG_MON_SEL (0x15394000 + 0x0910)---

    VDNR_BUS_DEBUG_MON_SEL[31..0] - (RW) BUS VDNR debug mon select control

 =====================================================================================*/
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_MON_SEL_VDNR_BUS_DEBUG_MON_SEL_OFFSET WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_MON_SEL_OFFSET
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_MON_SEL_VDNR_BUS_DEBUG_MON_SEL_MASK 0xFFFFFFFF                // VDNR_BUS_DEBUG_MON_SEL[31..0]
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_MON_SEL_VDNR_BUS_DEBUG_MON_SEL_SHFT 0

/* =====================================================================================

  ---VDNR_BUS_DEBUG_CTRL_0 (0x15394000 + 0x0914)---

    VDNR_DEBUG_CTRL_AO_DEBUGSYS_CTRL[31..0] - (RW) VDNR_DEBUG_CTRL_AO_DEBUGSYS_CTRL

 =====================================================================================*/
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_0_VDNR_DEBUG_CTRL_AO_DEBUGSYS_CTRL_OFFSET WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_0_OFFSET
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_0_VDNR_DEBUG_CTRL_AO_DEBUGSYS_CTRL_MASK 0xFFFFFFFF                // VDNR_DEBUG_CTRL_AO_DEBUGSYS_CTRL[31..0]
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_0_VDNR_DEBUG_CTRL_AO_DEBUGSYS_CTRL_SHFT 0

/* =====================================================================================

  ---VDNR_BUS_DEBUG_CTRL_1 (0x15394000 + 0x0918)---

    VDNR_DEBUG_CTRL_AO_IO_CTRL[7..0] - (RW) VDNR_DEBUG_CTRL_AO_IO_CTRL
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_1_VDNR_DEBUG_CTRL_AO_IO_CTRL_OFFSET WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_1_OFFSET
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_1_VDNR_DEBUG_CTRL_AO_IO_CTRL_MASK 0x000000FF                // VDNR_DEBUG_CTRL_AO_IO_CTRL[7..0]
#define WOX_MCU_CFG_LS_VDNR_BUS_DEBUG_CTRL_1_VDNR_DEBUG_CTRL_AO_IO_CTRL_SHFT 0

#endif
