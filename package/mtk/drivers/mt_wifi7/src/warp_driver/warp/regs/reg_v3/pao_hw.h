#pragma once


/* CONST definition */
#ifndef REG_CONST
#define REG_CONST
#endif

typedef unsigned int uint32_t;
typedef volatile struct pao2_REG {
    /* 0x000 */
    union {
        struct {
            REG_CONST uint32_t _MIN_SDL_SIZE : 10;
            REG_CONST uint32_t _IS_PRIOR0_RING : 1;
            REG_CONST uint32_t _IS_PRIOR1_RING : 1;
            REG_CONST uint32_t _ETHER_TYPE_MRG : 1;
            REG_CONST uint32_t _ETHER_TYPE_CFG : 1;
            REG_CONST uint32_t _IS_JUMBO_MRG : 1;
            REG_CONST uint32_t _RESERVED_6 : 1;
            REG_CONST uint32_t _JUMBO_TIMEOUT_DROP_EN : 1;
            REG_CONST uint32_t _JUMBO_BUF_NUM_EXCEED_DROP_EN : 1;
            REG_CONST uint32_t _JUMBO_ID_ERR_1_DROP_EN : 1;
            REG_CONST uint32_t _JUMBO_ID_ERR_0_DROP_EN : 1;
            REG_CONST uint32_t _JUMBO_ERR_STOP_EN : 1;
            REG_CONST uint32_t _LATCH_HIFTXD_OFF : 1;
            REG_CONST uint32_t _DMAD_LAT_OPT : 4;
            REG_CONST uint32_t _RESERVED_14 : 2;
            REG_CONST uint32_t _JUMBO_MAX_BUF_NUM : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU_FIFO;

    /* 0x004 ~ 0x004 */
    REG_CONST uint32_t          RSV_004_004[1];

    /* 0x008 */
    union {
        struct {
            REG_CONST uint32_t _DIV : 3;
            REG_CONST uint32_t _RESERVED_1 : 1;
            REG_CONST uint32_t _EN : 1;
            REG_CONST uint32_t _RESERVED_3 : 27;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_TIMER;

    /* 0x00c */
    union {
        struct {
            REG_CONST uint32_t _CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_TIMER_JUMBO;

    /* 0x010 */
    union {
        struct {
            REG_CONST uint32_t _DO_INIT : 1;
            REG_CONST uint32_t _DO_SET : 1;
            REG_CONST uint32_t _DO_GET : 1;
            REG_CONST uint32_t _RESERVED_3 : 29;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_STA_INFO;

    /* 0x014 */
    union {
        struct {
            REG_CONST uint32_t _CFG_WTBL_HDRT_MODE : 1;
            REG_CONST uint32_t _RMVL : 1;
            REG_CONST uint32_t _MAX_AMSDU_LEN : 6;
            REG_CONST uint32_t _MAX_AMSDU_NUM : 4;
            REG_CONST uint32_t _RESERVED_4 : 20;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_STA_INFO_INIT;

    /* 0x018 */
    union {
        struct {
            REG_CONST uint32_t _CFG_WTBL_HDRT_MODE : 1;
            REG_CONST uint32_t _RMVL : 1;
            REG_CONST uint32_t _MAX_AMSDU_LEN : 6;
            REG_CONST uint32_t _MAX_AMSDU_NUM : 4;
            REG_CONST uint32_t _RESERVED_4 : 4;
            REG_CONST uint32_t _NUMBER : 12;
            REG_CONST uint32_t _RESERVED_6 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_STA_INFO_SET;

    /* 0x01c */
    union {
        struct {
            REG_CONST uint32_t _INIT_BUSY : 1;
            REG_CONST uint32_t _SET_BUSY : 1;
            REG_CONST uint32_t _GET_BUSY : 1;
            REG_CONST uint32_t _INIT_DONE : 1;
            REG_CONST uint32_t _CURR_STAT : 5;
            REG_CONST uint32_t _DOUT : 12;
            REG_CONST uint32_t _RESERVED_6 : 11;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_STA_INFO_STS;

    /* 0x020 */
    union {
        struct {
            REG_CONST uint32_t _MASK : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_COMP0_MASK0;

    /* 0x024 */
    union {
        struct {
            REG_CONST uint32_t _MASK : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_COMP0_MASK1;

    /* 0x028 */
    union {
        struct {
            REG_CONST uint32_t _MASK : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_COMP1_MASK0;

    /* 0x02c */
    union {
        struct {
            REG_CONST uint32_t _MASK : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_COMP1_MASK1;

    /* 0x030 */
    union {
        struct {
            REG_CONST uint32_t _CONST01 : 6;
            REG_CONST uint32_t _CONST00 : 6;
            REG_CONST uint32_t _TXD_LEN : 6;
            REG_CONST uint32_t _SUB_VAL : 6;
            REG_CONST uint32_t _ADD_VAL : 6;
            REG_CONST uint32_t _AMSDU_ENQ_OPT : 1;
            REG_CONST uint32_t _RESERVED_6 : 1;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_TX_BYTE_CNT_CFG0;

    /* 0x034 */
    union {
        struct {
            REG_CONST uint32_t _CONST06 : 6;
            REG_CONST uint32_t _CONST05 : 6;
            REG_CONST uint32_t _CONST04 : 6;
            REG_CONST uint32_t _CONST03 : 6;
            REG_CONST uint32_t _CONST02 : 6;
            REG_CONST uint32_t _RESERVED_5 : 2;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_TX_BYTE_CNT_CFG1;

    /* 0x038 */
    union {
        struct {
            REG_CONST uint32_t _CONST11 : 6;
            REG_CONST uint32_t _CONST10 : 6;
            REG_CONST uint32_t _CONST09 : 6;
            REG_CONST uint32_t _CONST08 : 6;
            REG_CONST uint32_t _CONST07 : 6;
            REG_CONST uint32_t _RESERVED_5 : 2;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_TX_BYTE_CNT_CFG2;

    /* 0x03c */
    union {
        struct {
            REG_CONST uint32_t _PADING3 : 2;
            REG_CONST uint32_t _PADING2 : 2;
            REG_CONST uint32_t _PADING1 : 2;
            REG_CONST uint32_t _PADING0 : 2;
            REG_CONST uint32_t _CONST15 : 6;
            REG_CONST uint32_t _CONST14 : 6;
            REG_CONST uint32_t _CONST13 : 6;
            REG_CONST uint32_t _CONST12 : 6;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_TX_BYTE_CNT_CFG3;

    /* 0x040 */
    union {
        struct {
            REG_CONST uint32_t _FAST_DEQ_OPT : 5;
            REG_CONST uint32_t _TXP_NUM_MAX : 5;
            REG_CONST uint32_t _PL_CNT_MAX : 11;
            REG_CONST uint32_t _SEARCH_MAX : 11;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU0_CFG0;

    /* 0x044 */
    union {
        struct {
            REG_CONST uint32_t _WAIT_DMAD_CYCLE : 14;
            REG_CONST uint32_t _WAIT_CYCLE : 14;
            REG_CONST uint32_t _RESERVED_2 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU0_CFG1;

    /* 0x048 ~ 0x04c */
    REG_CONST uint32_t          RSV_048_04c[2];

    /* 0x050 */
    union {
        struct {
            REG_CONST uint32_t _FAST_DEQ_OPT : 5;
            REG_CONST uint32_t _TXP_NUM_MAX : 5;
            REG_CONST uint32_t _PL_CNT_MAX : 11;
            REG_CONST uint32_t _SEARCH_MAX : 11;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU1_CFG0;

    /* 0x054 */
    union {
        struct {
            REG_CONST uint32_t _WAIT_DMAD_CYCLE : 14;
            REG_CONST uint32_t _WAIT_CYCLE : 14;
            REG_CONST uint32_t _RESERVED_2 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU1_CFG1;

    /* 0x058 ~ 0x05c */
    REG_CONST uint32_t          RSV_058_05c[2];

    /* 0x060 */
    union {
        struct {
            REG_CONST uint32_t _FAST_DEQ_OPT : 5;
            REG_CONST uint32_t _TXP_NUM_MAX : 5;
            REG_CONST uint32_t _PL_CNT_MAX : 11;
            REG_CONST uint32_t _SEARCH_MAX : 11;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU2_CFG0;

    /* 0x064 */
    union {
        struct {
            REG_CONST uint32_t _WAIT_DMAD_CYCLE : 14;
            REG_CONST uint32_t _WAIT_CYCLE : 14;
            REG_CONST uint32_t _RESERVED_2 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU2_CFG1;

    /* 0x068 ~ 0x06c */
    REG_CONST uint32_t          RSV_068_06c[2];

    /* 0x070 */
    union {
        struct {
            REG_CONST uint32_t _FAST_DEQ_OPT : 5;
            REG_CONST uint32_t _TXP_NUM_MAX : 5;
            REG_CONST uint32_t _PL_CNT_MAX : 11;
            REG_CONST uint32_t _SEARCH_MAX : 11;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU3_CFG0;

    /* 0x074 */
    union {
        struct {
            REG_CONST uint32_t _WAIT_DMAD_CYCLE : 14;
            REG_CONST uint32_t _WAIT_CYCLE : 14;
            REG_CONST uint32_t _RESERVED_2 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU3_CFG1;

    /* 0x078 ~ 0x07c */
    REG_CONST uint32_t          RSV_078_07c[2];

    /* 0x080 */
    union {
        struct {
            REG_CONST uint32_t _FAST_DEQ_OPT : 5;
            REG_CONST uint32_t _TXP_NUM_MAX : 5;
            REG_CONST uint32_t _PL_CNT_MAX : 11;
            REG_CONST uint32_t _SEARCH_MAX : 11;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU4_CFG0;

    /* 0x084 */
    union {
        struct {
            REG_CONST uint32_t _WAIT_DMAD_CYCLE : 14;
            REG_CONST uint32_t _WAIT_CYCLE : 14;
            REG_CONST uint32_t _RESERVED_2 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU4_CFG1;

    /* 0x088 ~ 0x08c */
    REG_CONST uint32_t          RSV_088_08c[2];

    /* 0x090 */
    union {
        struct {
            REG_CONST uint32_t _FAST_DEQ_OPT : 5;
            REG_CONST uint32_t _TXP_NUM_MAX : 5;
            REG_CONST uint32_t _PL_CNT_MAX : 11;
            REG_CONST uint32_t _SEARCH_MAX : 11;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU5_CFG0;

    /* 0x094 */
    union {
        struct {
            REG_CONST uint32_t _WAIT_DMAD_CYCLE : 14;
            REG_CONST uint32_t _WAIT_CYCLE : 14;
            REG_CONST uint32_t _RESERVED_2 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU5_CFG1;

    /* 0x098 ~ 0x09c */
    REG_CONST uint32_t          RSV_098_09c[2];

    /* 0x0a0 */
    union {
        struct {
            REG_CONST uint32_t _FAST_DEQ_OPT : 5;
            REG_CONST uint32_t _TXP_NUM_MAX : 5;
            REG_CONST uint32_t _PL_CNT_MAX : 11;
            REG_CONST uint32_t _SEARCH_MAX : 11;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU6_CFG0;

    /* 0x0a4 */
    union {
        struct {
            REG_CONST uint32_t _WAIT_DMAD_CYCLE : 14;
            REG_CONST uint32_t _WAIT_CYCLE : 14;
            REG_CONST uint32_t _RESERVED_2 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU6_CFG1;

    /* 0x0a8 ~ 0x0ac */
    REG_CONST uint32_t          RSV_0a8_0ac[2];

    /* 0x0b0 */
    union {
        struct {
            REG_CONST uint32_t _FAST_DEQ_OPT : 5;
            REG_CONST uint32_t _TXP_NUM_MAX : 5;
            REG_CONST uint32_t _PL_CNT_MAX : 11;
            REG_CONST uint32_t _SEARCH_MAX : 11;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU7_CFG0;

    /* 0x0b4 */
    union {
        struct {
            REG_CONST uint32_t _WAIT_DMAD_CYCLE : 14;
            REG_CONST uint32_t _WAIT_CYCLE : 14;
            REG_CONST uint32_t _RESERVED_2 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU7_CFG1;

    /* 0x0b8 ~ 0x0bc */
    REG_CONST uint32_t          RSV_0b8_0bc[2];

    /* 0x0c0 */
    union {
        struct {
            REG_CONST uint32_t _FAST_DEQ_OPT : 5;
            REG_CONST uint32_t _TXP_NUM_MAX : 5;
            REG_CONST uint32_t _PL_CNT_MAX : 11;
            REG_CONST uint32_t _SEARCH_MAX : 11;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU8_CFG0;

    /* 0x0c4 */
    union {
        struct {
            REG_CONST uint32_t _WAIT_DMAD_CYCLE : 14;
            REG_CONST uint32_t _WAIT_CYCLE : 14;
            REG_CONST uint32_t _RESERVED_2 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU8_CFG1;

    /* 0x0c8 ~ 0x0cc */
    REG_CONST uint32_t          RSV_0c8_0cc[2];

    /* 0x0d0 */
    union {
        struct {
            REG_CONST uint32_t _SP_EN : 9;
            REG_CONST uint32_t _RESERVED_1 : 7;
            REG_CONST uint32_t _THRESHOLD : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_SCHED;

    /* 0x0d4 */
    union {
        struct {
            REG_CONST uint32_t _Q3_WEIGHT : 7;
            REG_CONST uint32_t _Q2_WEIGHT : 7;
            REG_CONST uint32_t _Q1_WEIGHT : 7;
            REG_CONST uint32_t _Q0_WEIGHT : 7;
            REG_CONST uint32_t _RESERVED_4 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_SCHED_CFG0;

    /* 0x0d8 */
    union {
        struct {
            REG_CONST uint32_t _Q7_WEIGHT : 7;
            REG_CONST uint32_t _Q6_WEIGHT : 7;
            REG_CONST uint32_t _Q5_WEIGHT : 7;
            REG_CONST uint32_t _Q4_WEIGHT : 7;
            REG_CONST uint32_t _RESERVED_4 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_SCHED_CFG1;

    /* 0x0dc */
    union {
        struct {
            REG_CONST uint32_t _Q8_WEIGHT : 7;
            REG_CONST uint32_t _RESERVED_1 : 25;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_SCHED_CFG2;

    /* 0x0e0 */
    union {
        struct {
            REG_CONST uint32_t _CYCLE : 16;
            REG_CONST uint32_t _RESERVED_1 : 16;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_SCHED_TIMER;

    /* 0x0e4 ~ 0x0fc */
    REG_CONST uint32_t          RSV_0e4_0fc[7];

    /* 0x100 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 11;
            REG_CONST uint32_t _RESERVED_1 : 21;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_FQ_MAX_PCNT;

    /* 0x104 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 11;
            REG_CONST uint32_t _RESERVED_1 : 21;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_FQ_DROP;

    /* 0x108 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 11;
            REG_CONST uint32_t _RESERVED_1 : 21;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_FQ_LOW;

    /* 0x10c */
    union {
        struct {
            REG_CONST uint32_t _VAL : 11;
            REG_CONST uint32_t _RESERVED_1 : 21;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_FQ_HIGH;

    /* 0x110 */
    union {
        struct {
            REG_CONST uint32_t _SP_ARB_EN : 1;
            REG_CONST uint32_t _TDM_ARB_EN : 1;
            REG_CONST uint32_t _TDM_TBL_NUM : 7;
            REG_CONST uint32_t _RESERVED_3 : 7;
            REG_CONST uint32_t _RESET : 1;
            REG_CONST uint32_t _RESERVED_5 : 15;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE;

    /* 0x114 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL0;

    /* 0x118 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL1;

    /* 0x11c */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL2;

    /* 0x120 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL3;

    /* 0x124 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL4;

    /* 0x128 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL5;

    /* 0x12c */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL6;

    /* 0x130 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL7;

    /* 0x134 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL8;

    /* 0x138 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL9;

    /* 0x13c */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL10;

    /* 0x140 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL11;

    /* 0x144 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL12;

    /* 0x148 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL13;

    /* 0x14c */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL14;

    /* 0x150 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_TDM_TBL15;

    /* 0x154 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_SP_CFG0;

    /* 0x158 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PSE_SP_CFG1;

    /* 0x15c ~ 0x15c */
    REG_CONST uint32_t          RSV_15c_15c[1];

    /* 0x160 */
    union {
        struct {
            REG_CONST uint32_t _SDP1_ZERO : 1;
            REG_CONST uint32_t _SDP0_ZERO : 1;
            REG_CONST uint32_t _BURST : 1;
            REG_CONST uint32_t _LS1 : 1;
            REG_CONST uint32_t _LS0 : 1;
            REG_CONST uint32_t _RESERVED_5 : 27;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_WPDMAD_CFG0;

    /* 0x164 */
    union {
        struct {
            REG_CONST uint32_t _SDL1 : 14;
            REG_CONST uint32_t _SDL0 : 14;
            REG_CONST uint32_t _RESERVED_2 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_WPDMAD_CFG1;

    /* 0x168 */
    union {
        struct {
            REG_CONST uint32_t _SIZE : 4;
            REG_CONST uint32_t _RESERVED_1 : 6;
            REG_CONST uint32_t _IS_NON_CIPHER : 1;
            REG_CONST uint32_t _BC_MC_FLAG : 2;
            REG_CONST uint32_t _IS_FR_HOST : 1;
            REG_CONST uint32_t _IS_MCAST_CLONE : 1;
            REG_CONST uint32_t _SRC : 2;
            REG_CONST uint32_t _HIF_VERSION : 4;
            REG_CONST uint32_t _ETHER_TYPE_BSWAP : 1;
            REG_CONST uint32_t _RESERVED_9 : 10;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD;

    /* 0x16c ~ 0x17c */
    REG_CONST uint32_t          RSV_16c_17c[5];

    /* 0x180 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE00_L;

    /* 0x184 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE01_L;

    /* 0x188 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE02_L;

    /* 0x18c */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE03_L;

    /* 0x190 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE04_L;

    /* 0x194 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE05_L;

    /* 0x198 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE06_L;

    /* 0x19c */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE07_L;

    /* 0x1a0 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE08_L;

    /* 0x1a4 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE09_L;

    /* 0x1a8 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE10_L;

    /* 0x1ac */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE11_L;

    /* 0x1b0 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE12_L;

    /* 0x1b4 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE13_L;

    /* 0x1b8 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE14_L;

    /* 0x1bc */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE15_L;

    /* 0x1c0 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE16_L;

    /* 0x1c4 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE17_L;

    /* 0x1c8 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE18_L;

    /* 0x1cc */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE19_L;

    /* 0x1d0 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE20_L;

    /* 0x1d4 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE21_L;

    /* 0x1d8 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE22_L;

    /* 0x1dc */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE23_L;

    /* 0x1e0 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE24_L;

    /* 0x1e4 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE25_L;

    /* 0x1e8 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE26_L;

    /* 0x1ec */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE27_L;

    /* 0x1f0 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE28_L;

    /* 0x1f4 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE29_L;

    /* 0x1f8 */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE30_L;

    /* 0x1fc */
    union {
        struct {
            REG_CONST uint32_t _PTR : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE31_L;

    /* 0x200 */
    union {
        struct {
            REG_CONST uint32_t _BASE07 : 4;
            REG_CONST uint32_t _BASE06 : 4;
            REG_CONST uint32_t _BASE05 : 4;
            REG_CONST uint32_t _BASE04 : 4;
            REG_CONST uint32_t _BASE03 : 4;
            REG_CONST uint32_t _BASE02 : 4;
            REG_CONST uint32_t _BASE01 : 4;
            REG_CONST uint32_t _BASE00 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE_00_07_H;

    /* 0x204 */
    union {
        struct {
            REG_CONST uint32_t _BASE15 : 4;
            REG_CONST uint32_t _BASE14 : 4;
            REG_CONST uint32_t _BASE13 : 4;
            REG_CONST uint32_t _BASE12 : 4;
            REG_CONST uint32_t _BASE11 : 4;
            REG_CONST uint32_t _BASE10 : 4;
            REG_CONST uint32_t _BASE09 : 4;
            REG_CONST uint32_t _BASE08 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE_08_15_H;

    /* 0x208 */
    union {
        struct {
            REG_CONST uint32_t _BASE23 : 4;
            REG_CONST uint32_t _BASE22 : 4;
            REG_CONST uint32_t _BASE21 : 4;
            REG_CONST uint32_t _BASE20 : 4;
            REG_CONST uint32_t _BASE19 : 4;
            REG_CONST uint32_t _BASE18 : 4;
            REG_CONST uint32_t _BASE17 : 4;
            REG_CONST uint32_t _BASE16 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE_16_23_H;

    /* 0x20c */
    union {
        struct {
            REG_CONST uint32_t _BASE31 : 4;
            REG_CONST uint32_t _BASE30 : 4;
            REG_CONST uint32_t _BASE29 : 4;
            REG_CONST uint32_t _BASE28 : 4;
            REG_CONST uint32_t _BASE27 : 4;
            REG_CONST uint32_t _BASE26 : 4;
            REG_CONST uint32_t _BASE25 : 4;
            REG_CONST uint32_t _BASE24 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_HIFTXD_BASE_24_31_H;

    /* 0x210 ~ 0x22c */
    REG_CONST uint32_t          RSV_210_22c[8];

    /* 0x230 */
    union {
        struct {
            REG_CONST uint32_t _FSM : 8;
            REG_CONST uint32_t _RESERVED_1 : 24;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_FIFO;

    /* 0x234 */
    union {
        struct {
            REG_CONST uint32_t _DMAD : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_FIFO_CNT0;

    /* 0x238 */
    union {
        struct {
            REG_CONST uint32_t _JUMBO_MSDU_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_FIFO_CNT1;

    /* 0x23c */
    union {
        struct {
            REG_CONST uint32_t _IS_SP_DMAD_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_FIFO_CNT2;

    /* 0x240 */
    union {
        struct {
            REG_CONST uint32_t _AMSDU_EN_0_CNT0 : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_FIFO_CNT3;

    /* 0x244 */
    union {
        struct {
            REG_CONST uint32_t _AMSDU_EN_0_CNT1 : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_FIFO_CNT4;

    /* 0x248 */
    union {
        struct {
            REG_CONST uint32_t _AMSDU_EN_0_CNT2 : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_FIFO_CNT5;

    /* 0x24c */
    union {
        struct {
            REG_CONST uint32_t _AMSDU_EN_0_CNT3 : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_FIFO_CNT6;

    /* 0x250 */
    union {
        struct {
            REG_CONST uint32_t _MSDU_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_FIFO_CNT7;

    /* 0x254 */
    union {
        struct {
            REG_CONST uint32_t _ETHER_TYPE_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_FIFO_CNT8;

    /* 0x258 ~ 0x258 */
    REG_CONST uint32_t          RSV_258_258[1];

    /* 0x25c */
    union {
        struct {
            REG_CONST uint32_t _DATA : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_FIFO_DMAD_0;

    /* 0x260 */
    union {
        struct {
            REG_CONST uint32_t _DATA : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_FIFO_DMAD_1;

    /* 0x264 */
    union {
        struct {
            REG_CONST uint32_t _DATA : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_FIFO_DMAD_2;

    /* 0x268 */
    union {
        struct {
            REG_CONST uint32_t _DATA : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_FIFO_DMAD_3;

    /* 0x26c */
    union {
        struct {
            REG_CONST uint32_t _DATA : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_FIFO_DMAD_4;

    /* 0x270 */
    union {
        struct {
            REG_CONST uint32_t _FSM : 17;
            REG_CONST uint32_t _RESERVED_1 : 15;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0;

    /* 0x274 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_ADD_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_CNT0;

    /* 0x278 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MRG_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_CNT1;

    /* 0x27c */
    union {
        struct {
            REG_CONST uint32_t _DEQ_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_CNT2;

    /* 0x280 */
    union {
        struct {
            REG_CONST uint32_t _DMAD_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_CNT3;

    /* 0x284 */
    union {
        struct {
            REG_CONST uint32_t _QFPL_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_CNT4;

    /* 0x288 */
    union {
        struct {
            REG_CONST uint32_t _QENI_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_CNT5;

    /* 0x28c */
    union {
        struct {
            REG_CONST uint32_t _QENO_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_CNT6;

    /* 0x290 */
    union {
        struct {
            REG_CONST uint32_t _MERG_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_CNT7;

    /* 0x294 */
    union {
        struct {
            REG_CONST uint32_t _MAX_QGPP_CNT : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _MAX_PL_CNT : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_CNT8;

    /* 0x298 */
    union {
        struct {
            REG_CONST uint32_t _PORTQ_CNT : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _MAX_BUF_NUM : 5;
            REG_CONST uint32_t _RESERVED_3 : 3;
            REG_CONST uint32_t _MAX_MSDU_CNT : 5;
            REG_CONST uint32_t _RESERVED_5 : 3;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_CNT9;

    /* 0x29c */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_CNT10;

    /* 0x2a0 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_HEAD_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_CNT11;

    /* 0x2a4 */
    union {
        struct {
            REG_CONST uint32_t _MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_CNT12;

    /* 0x2a8 */
    union {
        struct {
            REG_CONST uint32_t _WCID_HIT_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_CNT13;

    /* 0x2ac */
    union {
        struct {
            REG_CONST uint32_t _PORT_END : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _PORT_START : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_PTR0;

    /* 0x2b0 */
    union {
        struct {
            REG_CONST uint32_t _CUR_HEAD_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_WRIT_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_PTR1;

    /* 0x2b4 */
    union {
        struct {
            REG_CONST uint32_t _CUR_QGPP_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_TAIL_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG0_PTR2;

    /* 0x2b8 ~ 0x2bc */
    REG_CONST uint32_t          RSV_2b8_2bc[2];

    /* 0x2c0 */
    union {
        struct {
            REG_CONST uint32_t _FSM : 17;
            REG_CONST uint32_t _RESERVED_1 : 15;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1;

    /* 0x2c4 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_ADD_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_CNT0;

    /* 0x2c8 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MRG_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_CNT1;

    /* 0x2cc */
    union {
        struct {
            REG_CONST uint32_t _DEQ_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_CNT2;

    /* 0x2d0 */
    union {
        struct {
            REG_CONST uint32_t _DMAD_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_CNT3;

    /* 0x2d4 */
    union {
        struct {
            REG_CONST uint32_t _QFPL_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_CNT4;

    /* 0x2d8 */
    union {
        struct {
            REG_CONST uint32_t _QENI_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_CNT5;

    /* 0x2dc */
    union {
        struct {
            REG_CONST uint32_t _QENO_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_CNT6;

    /* 0x2e0 */
    union {
        struct {
            REG_CONST uint32_t _MERG_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_CNT7;

    /* 0x2e4 */
    union {
        struct {
            REG_CONST uint32_t _MAX_QGPP_CNT : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _MAX_PL_CNT : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_CNT8;

    /* 0x2e8 */
    union {
        struct {
            REG_CONST uint32_t _PORTQ_CNT : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _MAX_BUF_NUM : 5;
            REG_CONST uint32_t _RESERVED_3 : 3;
            REG_CONST uint32_t _MAX_MSDU_CNT : 5;
            REG_CONST uint32_t _RESERVED_5 : 3;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_CNT9;

    /* 0x2ec */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_CNT10;

    /* 0x2f0 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_HEAD_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_CNT11;

    /* 0x2f4 */
    union {
        struct {
            REG_CONST uint32_t _MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_CNT12;

    /* 0x2f8 */
    union {
        struct {
            REG_CONST uint32_t _WCID_HIT_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_CNT13;

    /* 0x2fc */
    union {
        struct {
            REG_CONST uint32_t _PORT_END : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _PORT_START : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_PTR0;

    /* 0x300 */
    union {
        struct {
            REG_CONST uint32_t _CUR_HEAD_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_WRIT_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_PTR1;

    /* 0x304 */
    union {
        struct {
            REG_CONST uint32_t _CUR_QGPP_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_TAIL_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG1_PTR2;

    /* 0x308 ~ 0x30c */
    REG_CONST uint32_t          RSV_308_30c[2];

    /* 0x310 */
    union {
        struct {
            REG_CONST uint32_t _FSM : 17;
            REG_CONST uint32_t _RESERVED_1 : 15;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2;

    /* 0x314 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_ADD_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_CNT0;

    /* 0x318 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MRG_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_CNT1;

    /* 0x31c */
    union {
        struct {
            REG_CONST uint32_t _DEQ_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_CNT2;

    /* 0x320 */
    union {
        struct {
            REG_CONST uint32_t _DMAD_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_CNT3;

    /* 0x324 */
    union {
        struct {
            REG_CONST uint32_t _QFPL_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_CNT4;

    /* 0x328 */
    union {
        struct {
            REG_CONST uint32_t _QENI_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_CNT5;

    /* 0x32c */
    union {
        struct {
            REG_CONST uint32_t _QENO_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_CNT6;

    /* 0x330 */
    union {
        struct {
            REG_CONST uint32_t _MERG_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_CNT7;

    /* 0x334 */
    union {
        struct {
            REG_CONST uint32_t _MAX_QGPP_CNT : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _MAX_PL_CNT : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_CNT8;

    /* 0x338 */
    union {
        struct {
            REG_CONST uint32_t _PORTQ_CNT : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _MAX_BUF_NUM : 5;
            REG_CONST uint32_t _RESERVED_3 : 3;
            REG_CONST uint32_t _MAX_MSDU_CNT : 5;
            REG_CONST uint32_t _RESERVED_5 : 3;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_CNT9;

    /* 0x33c */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_CNT10;

    /* 0x340 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_HEAD_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_CNT11;

    /* 0x344 */
    union {
        struct {
            REG_CONST uint32_t _MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_CNT12;

    /* 0x348 */
    union {
        struct {
            REG_CONST uint32_t _WCID_HIT_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_CNT13;

    /* 0x34c */
    union {
        struct {
            REG_CONST uint32_t _PORT_END : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _PORT_START : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_PTR0;

    /* 0x350 */
    union {
        struct {
            REG_CONST uint32_t _CUR_HEAD_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_WRIT_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_PTR1;

    /* 0x354 */
    union {
        struct {
            REG_CONST uint32_t _CUR_QGPP_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_TAIL_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG2_PTR2;

    /* 0x358 ~ 0x35c */
    REG_CONST uint32_t          RSV_358_35c[2];

    /* 0x360 */
    union {
        struct {
            REG_CONST uint32_t _FSM : 17;
            REG_CONST uint32_t _RESERVED_1 : 15;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3;

    /* 0x364 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_ADD_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_CNT0;

    /* 0x368 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MRG_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_CNT1;

    /* 0x36c */
    union {
        struct {
            REG_CONST uint32_t _DEQ_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_CNT2;

    /* 0x370 */
    union {
        struct {
            REG_CONST uint32_t _DMAD_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_CNT3;

    /* 0x374 */
    union {
        struct {
            REG_CONST uint32_t _QFPL_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_CNT4;

    /* 0x378 */
    union {
        struct {
            REG_CONST uint32_t _QENI_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_CNT5;

    /* 0x37c */
    union {
        struct {
            REG_CONST uint32_t _QENO_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_CNT6;

    /* 0x380 */
    union {
        struct {
            REG_CONST uint32_t _MERG_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_CNT7;

    /* 0x384 */
    union {
        struct {
            REG_CONST uint32_t _MAX_QGPP_CNT : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _MAX_PL_CNT : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_CNT8;

    /* 0x388 */
    union {
        struct {
            REG_CONST uint32_t _PORTQ_CNT : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _MAX_BUF_NUM : 5;
            REG_CONST uint32_t _RESERVED_3 : 3;
            REG_CONST uint32_t _MAX_MSDU_CNT : 5;
            REG_CONST uint32_t _RESERVED_5 : 3;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_CNT9;

    /* 0x38c */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_CNT10;

    /* 0x390 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_HEAD_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_CNT11;

    /* 0x394 */
    union {
        struct {
            REG_CONST uint32_t _MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_CNT12;

    /* 0x398 */
    union {
        struct {
            REG_CONST uint32_t _WCID_HIT_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_CNT13;

    /* 0x39c */
    union {
        struct {
            REG_CONST uint32_t _PORT_END : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _PORT_START : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_PTR0;

    /* 0x3a0 */
    union {
        struct {
            REG_CONST uint32_t _CUR_HEAD_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_WRIT_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_PTR1;

    /* 0x3a4 */
    union {
        struct {
            REG_CONST uint32_t _CUR_QGPP_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_TAIL_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG3_PTR2;

    /* 0x3a8 ~ 0x3ac */
    REG_CONST uint32_t          RSV_3a8_3ac[2];

    /* 0x3b0 */
    union {
        struct {
            REG_CONST uint32_t _FSM : 17;
            REG_CONST uint32_t _RESERVED_1 : 15;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4;

    /* 0x3b4 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_ADD_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_CNT0;

    /* 0x3b8 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MRG_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_CNT1;

    /* 0x3bc */
    union {
        struct {
            REG_CONST uint32_t _DEQ_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_CNT2;

    /* 0x3c0 */
    union {
        struct {
            REG_CONST uint32_t _DMAD_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_CNT3;

    /* 0x3c4 */
    union {
        struct {
            REG_CONST uint32_t _QFPL_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_CNT4;

    /* 0x3c8 */
    union {
        struct {
            REG_CONST uint32_t _QENI_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_CNT5;

    /* 0x3cc */
    union {
        struct {
            REG_CONST uint32_t _QENO_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_CNT6;

    /* 0x3d0 */
    union {
        struct {
            REG_CONST uint32_t _MERG_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_CNT7;

    /* 0x3d4 */
    union {
        struct {
            REG_CONST uint32_t _MAX_QGPP_CNT : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _MAX_PL_CNT : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_CNT8;

    /* 0x3d8 */
    union {
        struct {
            REG_CONST uint32_t _PORTQ_CNT : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _MAX_BUF_NUM : 5;
            REG_CONST uint32_t _RESERVED_3 : 3;
            REG_CONST uint32_t _MAX_MSDU_CNT : 5;
            REG_CONST uint32_t _RESERVED_5 : 3;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_CNT9;

    /* 0x3dc */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_CNT10;

    /* 0x3e0 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_HEAD_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_CNT11;

    /* 0x3e4 */
    union {
        struct {
            REG_CONST uint32_t _MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_CNT12;

    /* 0x3e8 */
    union {
        struct {
            REG_CONST uint32_t _WCID_HIT_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_CNT13;

    /* 0x3ec */
    union {
        struct {
            REG_CONST uint32_t _PORT_END : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _PORT_START : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_PTR0;

    /* 0x3f0 */
    union {
        struct {
            REG_CONST uint32_t _CUR_HEAD_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_WRIT_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_PTR1;

    /* 0x3f4 */
    union {
        struct {
            REG_CONST uint32_t _CUR_QGPP_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_TAIL_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG4_PTR2;

    /* 0x3f8 ~ 0x3fc */
    REG_CONST uint32_t          RSV_3f8_3fc[2];

    /* 0x400 */
    union {
        struct {
            REG_CONST uint32_t _FSM : 17;
            REG_CONST uint32_t _RESERVED_1 : 15;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5;

    /* 0x404 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_ADD_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_CNT0;

    /* 0x408 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MRG_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_CNT1;

    /* 0x40c */
    union {
        struct {
            REG_CONST uint32_t _DEQ_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_CNT2;

    /* 0x410 */
    union {
        struct {
            REG_CONST uint32_t _DMAD_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_CNT3;

    /* 0x414 */
    union {
        struct {
            REG_CONST uint32_t _QFPL_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_CNT4;

    /* 0x418 */
    union {
        struct {
            REG_CONST uint32_t _QENI_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_CNT5;

    /* 0x41c */
    union {
        struct {
            REG_CONST uint32_t _QENO_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_CNT6;

    /* 0x420 */
    union {
        struct {
            REG_CONST uint32_t _MERG_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_CNT7;

    /* 0x424 */
    union {
        struct {
            REG_CONST uint32_t _MAX_QGPP_CNT : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _MAX_PL_CNT : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_CNT8;

    /* 0x428 */
    union {
        struct {
            REG_CONST uint32_t _PORTQ_CNT : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _MAX_BUF_NUM : 5;
            REG_CONST uint32_t _RESERVED_3 : 3;
            REG_CONST uint32_t _MAX_MSDU_CNT : 5;
            REG_CONST uint32_t _RESERVED_5 : 3;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_CNT9;

    /* 0x42c */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_CNT10;

    /* 0x430 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_HEAD_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_CNT11;

    /* 0x434 */
    union {
        struct {
            REG_CONST uint32_t _MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_CNT12;

    /* 0x438 */
    union {
        struct {
            REG_CONST uint32_t _WCID_HIT_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_CNT13;

    /* 0x43c */
    union {
        struct {
            REG_CONST uint32_t _PORT_END : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _PORT_START : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_PTR0;

    /* 0x440 */
    union {
        struct {
            REG_CONST uint32_t _CUR_HEAD_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_WRIT_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_PTR1;

    /* 0x444 */
    union {
        struct {
            REG_CONST uint32_t _CUR_QGPP_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_TAIL_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG5_PTR2;

    /* 0x448 ~ 0x44c */
    REG_CONST uint32_t          RSV_448_44c[2];

    /* 0x450 */
    union {
        struct {
            REG_CONST uint32_t _FSM : 17;
            REG_CONST uint32_t _RESERVED_1 : 15;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6;

    /* 0x454 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_ADD_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_CNT0;

    /* 0x458 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MRG_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_CNT1;

    /* 0x45c */
    union {
        struct {
            REG_CONST uint32_t _DEQ_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_CNT2;

    /* 0x460 */
    union {
        struct {
            REG_CONST uint32_t _DMAD_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_CNT3;

    /* 0x464 */
    union {
        struct {
            REG_CONST uint32_t _QFPL_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_CNT4;

    /* 0x468 */
    union {
        struct {
            REG_CONST uint32_t _QENI_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_CNT5;

    /* 0x46c */
    union {
        struct {
            REG_CONST uint32_t _QENO_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_CNT6;

    /* 0x470 */
    union {
        struct {
            REG_CONST uint32_t _MERG_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_CNT7;

    /* 0x474 */
    union {
        struct {
            REG_CONST uint32_t _MAX_QGPP_CNT : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _MAX_PL_CNT : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_CNT8;

    /* 0x478 */
    union {
        struct {
            REG_CONST uint32_t _PORTQ_CNT : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _MAX_BUF_NUM : 5;
            REG_CONST uint32_t _RESERVED_3 : 3;
            REG_CONST uint32_t _MAX_MSDU_CNT : 5;
            REG_CONST uint32_t _RESERVED_5 : 3;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_CNT9;

    /* 0x47c */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_CNT10;

    /* 0x480 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_HEAD_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_CNT11;

    /* 0x484 */
    union {
        struct {
            REG_CONST uint32_t _MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_CNT12;

    /* 0x488 */
    union {
        struct {
            REG_CONST uint32_t _WCID_HIT_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_CNT13;

    /* 0x48c */
    union {
        struct {
            REG_CONST uint32_t _PORT_END : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _PORT_START : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_PTR0;

    /* 0x490 */
    union {
        struct {
            REG_CONST uint32_t _CUR_HEAD_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_WRIT_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_PTR1;

    /* 0x494 */
    union {
        struct {
            REG_CONST uint32_t _CUR_QGPP_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_TAIL_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG6_PTR2;

    /* 0x498 ~ 0x49c */
    REG_CONST uint32_t          RSV_498_49c[2];

    /* 0x4a0 */
    union {
        struct {
            REG_CONST uint32_t _FSM : 17;
            REG_CONST uint32_t _RESERVED_1 : 15;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7;

    /* 0x4a4 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_ADD_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_CNT0;

    /* 0x4a8 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MRG_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_CNT1;

    /* 0x4ac */
    union {
        struct {
            REG_CONST uint32_t _DEQ_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_CNT2;

    /* 0x4b0 */
    union {
        struct {
            REG_CONST uint32_t _DMAD_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_CNT3;

    /* 0x4b4 */
    union {
        struct {
            REG_CONST uint32_t _QFPL_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_CNT4;

    /* 0x4b8 */
    union {
        struct {
            REG_CONST uint32_t _QENI_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_CNT5;

    /* 0x4bc */
    union {
        struct {
            REG_CONST uint32_t _QENO_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_CNT6;

    /* 0x4c0 */
    union {
        struct {
            REG_CONST uint32_t _MERG_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_CNT7;

    /* 0x4c4 */
    union {
        struct {
            REG_CONST uint32_t _MAX_QGPP_CNT : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _MAX_PL_CNT : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_CNT8;

    /* 0x4c8 */
    union {
        struct {
            REG_CONST uint32_t _PORTQ_CNT : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _MAX_BUF_NUM : 5;
            REG_CONST uint32_t _RESERVED_3 : 3;
            REG_CONST uint32_t _MAX_MSDU_CNT : 5;
            REG_CONST uint32_t _RESERVED_5 : 3;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_CNT9;

    /* 0x4cc */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_CNT10;

    /* 0x4d0 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_HEAD_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_CNT11;

    /* 0x4d4 */
    union {
        struct {
            REG_CONST uint32_t _MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_CNT12;

    /* 0x4d8 */
    union {
        struct {
            REG_CONST uint32_t _WCID_HIT_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_CNT13;

    /* 0x4dc */
    union {
        struct {
            REG_CONST uint32_t _PORT_END : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _PORT_START : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_PTR0;

    /* 0x4e0 */
    union {
        struct {
            REG_CONST uint32_t _CUR_HEAD_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_WRIT_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_PTR1;

    /* 0x4e4 */
    union {
        struct {
            REG_CONST uint32_t _CUR_QGPP_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_TAIL_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG7_PTR2;

    /* 0x4e8 ~ 0x4ec */
    REG_CONST uint32_t          RSV_4e8_4ec[2];

    /* 0x4f0 */
    union {
        struct {
            REG_CONST uint32_t _FSM : 17;
            REG_CONST uint32_t _RESERVED_1 : 15;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8;

    /* 0x4f4 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_ADD_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_CNT0;

    /* 0x4f8 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MRG_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_CNT1;

    /* 0x4fc */
    union {
        struct {
            REG_CONST uint32_t _DEQ_CYCLE : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_CNT2;

    /* 0x500 */
    union {
        struct {
            REG_CONST uint32_t _DMAD_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_CNT3;

    /* 0x504 */
    union {
        struct {
            REG_CONST uint32_t _QFPL_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_CNT4;

    /* 0x508 */
    union {
        struct {
            REG_CONST uint32_t _QENI_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_CNT5;

    /* 0x50c */
    union {
        struct {
            REG_CONST uint32_t _QENO_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_CNT6;

    /* 0x510 */
    union {
        struct {
            REG_CONST uint32_t _MERG_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_CNT7;

    /* 0x514 */
    union {
        struct {
            REG_CONST uint32_t _MAX_QGPP_CNT : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _MAX_PL_CNT : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_CNT8;

    /* 0x518 */
    union {
        struct {
            REG_CONST uint32_t _PORTQ_CNT : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _MAX_BUF_NUM : 5;
            REG_CONST uint32_t _RESERVED_3 : 3;
            REG_CONST uint32_t _MAX_MSDU_CNT : 5;
            REG_CONST uint32_t _RESERVED_5 : 3;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_CNT9;

    /* 0x51c */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_CNT10;

    /* 0x520 */
    union {
        struct {
            REG_CONST uint32_t _SEARCH_HEAD_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_CNT11;

    /* 0x524 */
    union {
        struct {
            REG_CONST uint32_t _MAX_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_CNT12;

    /* 0x528 */
    union {
        struct {
            REG_CONST uint32_t _WCID_HIT_ENQ_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_CNT13;

    /* 0x52c */
    union {
        struct {
            REG_CONST uint32_t _PORT_END : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _PORT_START : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_PTR0;

    /* 0x530 */
    union {
        struct {
            REG_CONST uint32_t _CUR_HEAD_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_WRIT_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_PTR1;

    /* 0x534 */
    union {
        struct {
            REG_CONST uint32_t _CUR_QGPP_PTR : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _CUR_TAIL_PTR : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_ENG8_PTR2;

    /* 0x538 ~ 0x5fc */
    REG_CONST uint32_t          RSV_538_5fc[50];

    /* 0x600 */
    union {
        struct {
            REG_CONST uint32_t _STS : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_STS0;

    /* 0x604 */
    union {
        struct {
            REG_CONST uint32_t _STS : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_STS1;

    /* 0x608 */
    union {
        struct {
            REG_CONST uint32_t _STS : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_STS2;

    /* 0x60c */
    union {
        struct {
            REG_CONST uint32_t _Q0_PCNT : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _FQ_PCNT : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_CNT0;

    /* 0x610 */
    union {
        struct {
            REG_CONST uint32_t _Q2_PCNT : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _Q1_PCNT : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_CNT1;

    /* 0x614 */
    union {
        struct {
            REG_CONST uint32_t _Q4_PCNT : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _Q3_PCNT : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_CNT2;

    /* 0x618 */
    union {
        struct {
            REG_CONST uint32_t _Q6_PCNT : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _Q5_PCNT : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_CNT3;

    /* 0x61c */
    union {
        struct {
            REG_CONST uint32_t _Q8_PCNT : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _Q7_PCNT : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_CNT4;

    /* 0x620 */
    union {
        struct {
            REG_CONST uint32_t _Q0_START : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _FQ_START : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_PTR0;

    /* 0x624 */
    union {
        struct {
            REG_CONST uint32_t _Q2_START : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _Q1_START : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_PTR1;

    /* 0x628 */
    union {
        struct {
            REG_CONST uint32_t _Q4_START : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _Q3_START : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_PTR2;

    /* 0x62c */
    union {
        struct {
            REG_CONST uint32_t _Q6_START : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _Q5_START : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_PTR3;

    /* 0x630 */
    union {
        struct {
            REG_CONST uint32_t _Q8_START : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _Q7_START : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_PTR4;

    /* 0x634 */
    union {
        struct {
            REG_CONST uint32_t _Q0_END : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _FQ_END : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_PTR5;

    /* 0x638 */
    union {
        struct {
            REG_CONST uint32_t _Q2_END : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _Q1_END : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_PTR6;

    /* 0x63c */
    union {
        struct {
            REG_CONST uint32_t _Q4_END : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _Q3_END : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_PTR7;

    /* 0x640 */
    union {
        struct {
            REG_CONST uint32_t _Q6_END : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _Q5_END : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_PTR8;

    /* 0x644 */
    union {
        struct {
            REG_CONST uint32_t _Q8_END : 12;
            REG_CONST uint32_t _RESERVED_1 : 4;
            REG_CONST uint32_t _Q7_END : 12;
            REG_CONST uint32_t _RESERVED_3 : 4;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_PTR9;

    /* 0x648 */
    union {
        struct {
            REG_CONST uint32_t _Q0_CPP : 11;
            REG_CONST uint32_t _RESERVED_1 : 21;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_PTR10;

    /* 0x64c */
    union {
        struct {
            REG_CONST uint32_t _Q2_CPP : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _Q1_CPP : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_PTR11;

    /* 0x650 */
    union {
        struct {
            REG_CONST uint32_t _Q4_CPP : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _Q3_CPP : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_PTR12;

    /* 0x654 */
    union {
        struct {
            REG_CONST uint32_t _Q6_CPP : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _Q5_CPP : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_PTR13;

    /* 0x658 */
    union {
        struct {
            REG_CONST uint32_t _Q8_CPP : 11;
            REG_CONST uint32_t _RESERVED_1 : 5;
            REG_CONST uint32_t _Q7_CPP : 11;
            REG_CONST uint32_t _RESERVED_3 : 5;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_QMEM_PTR14;

    /* 0x65c */
    union {
        struct {
            REG_CONST uint32_t _STS : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_AMSDU_SCHED;

    /* 0x660 ~ 0x66c */
    REG_CONST uint32_t          RSV_660_66c[4];

    /* 0x670 */
    union {
        struct {
            REG_CONST uint32_t _STS : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH;

    /* 0x674 */
    union {
        struct {
            REG_CONST uint32_t _HIFTXD_IN_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_CNT0;

    /* 0x678 */
    union {
        struct {
            REG_CONST uint32_t _HIFTXD_OU1_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_CNT1;

    /* 0x67c */
    union {
        struct {
            REG_CONST uint32_t _HIFTXD_OU2_CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_CNT2;

    /* 0x680 */
    union {
        struct {
            REG_CONST uint32_t _CURQ_CNT : 12;
            REG_CONST uint32_t _RESERVED_1 : 20;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_CNT3;

    /* 0x684 ~ 0x68c */
    REG_CONST uint32_t          RSV_684_68c[3];

    /* 0x690 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1;

    /* 0x694 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_2;

    /* 0x698 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_3;

    /* 0x69c */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_4;

    /* 0x6a0 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_5;

    /* 0x6a4 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_6;

    /* 0x6a8 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_7;

    /* 0x6ac */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_8;

    /* 0x6b0 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_9;

    /* 0x6b4 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_10;

    /* 0x6b8 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_11;

    /* 0x6bc */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_12;

    /* 0x6c0 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_13;

    /* 0x6c4 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1;

    /* 0x6c8 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_2;

    /* 0x6cc */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_3;

    /* 0x6d0 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_4;

    /* 0x6d4 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_5;

    /* 0x6d8 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_6;

    /* 0x6dc */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_7;

    /* 0x6e0 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_8;

    /* 0x6e4 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_9;

    /* 0x6e8 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_10;

    /* 0x6ec */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_11;

    /* 0x6f0 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_12;

    /* 0x6f4 */
    union {
        struct {
            REG_CONST uint32_t _CNT : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_13;

    /* 0x6f8 ~ 0x7cc */
    REG_CONST uint32_t          RSV_6f8_7cc[54];

    /* 0x7d0 */
    union {
        struct {
            REG_CONST uint32_t _JUMBO_BUF_NUM_ERR : 1;
            REG_CONST uint32_t _JUMBO_TIMEOUT_ERR : 1;
            REG_CONST uint32_t _JUMBO_ID_ERR_0 : 1;
            REG_CONST uint32_t _JUMBO_ID_ERR_1 : 1;
            REG_CONST uint32_t _RESERVED_4 : 27;
            REG_CONST uint32_t _CLR : 1;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_AMSDU_FIFO_ERR;

    /* 0x7d4 ~ 0x7dc */
    REG_CONST uint32_t          RSV_7d4_7dc[3];

    /* 0x7e0 */
    union {
        struct {
            REG_CONST uint32_t _SEL : 8;
            REG_CONST uint32_t _QSEL : 4;
            REG_CONST uint32_t _MSEL : 4;
            REG_CONST uint32_t _PSEL : 2;
            REG_CONST uint32_t _EN : 1;
            REG_CONST uint32_t _RESERVED_5 : 13;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_DBG_CFG;

    /* 0x7e4 */
    union {
        struct {
            REG_CONST uint32_t _CLR : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_DBG_CNT;

    /* 0x7e8 */
    union {
        struct {
            REG_CONST uint32_t _INFO : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_DBG;

    /* 0x7ec */
    union {
        struct {
            REG_CONST uint32_t _EN : 1;
            REG_CONST uint32_t _RESERVED_1 : 31;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PKTMEM_TEST;

    /* 0x7f0 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 11;
            REG_CONST uint32_t _RESERVED_1 : 21;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PKTMEM_ADDR;

    /* 0x7f4 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 5;
            REG_CONST uint32_t _RESERVED_1 : 27;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PKTMEM_DWSEL;

    /* 0x7f8 */
    union {
        struct {
            REG_CONST uint32_t _VAL : 32;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PKTMEM_DATA;

    /* 0x7fc */
    union {
        struct {
            REG_CONST uint32_t _DATA_SET : 1;
            REG_CONST uint32_t _RD : 1;
            REG_CONST uint32_t _WR : 1;
            REG_CONST uint32_t _RESERVED_3 : 29;
        };
        REG_CONST uint32_t DATA;
    } WED_PAO_PKTMEM_TRIG;
} pao2_REG_T;

#define WED_PAO_AMSDU_FIFO_OFFSET                         0x00001800
#define WED_PAO_TIMER_OFFSET                              0x00001808
#define WED_PAO_TIMER_JUMBO_OFFSET                        0x0000180C
#define WED_PAO_STA_INFO_OFFSET                           0x00001810
#define WED_PAO_STA_INFO_INIT_OFFSET                      0x00001814
#define WED_PAO_STA_INFO_SET_OFFSET                       0x00001818
#define WED_PAO_STA_INFO_STS_OFFSET                       0x0000181C
#define WED_PAO_COMP0_MASK0_OFFSET                        0x00001820
#define WED_PAO_COMP0_MASK1_OFFSET                        0x00001824
#define WED_PAO_COMP1_MASK0_OFFSET                        0x00001828
#define WED_PAO_COMP1_MASK1_OFFSET                        0x0000182C
#define WED_PAO_TX_BYTE_CNT_CFG0_OFFSET                   0x00001830
#define WED_PAO_TX_BYTE_CNT_CFG1_OFFSET                   0x00001834
#define WED_PAO_TX_BYTE_CNT_CFG2_OFFSET                   0x00001838
#define WED_PAO_TX_BYTE_CNT_CFG3_OFFSET                   0x0000183C
#define WED_PAO_AMSDU0_CFG0_OFFSET                        0x00001840
#define WED_PAO_AMSDU0_CFG1_OFFSET                        0x00001844
#define WED_PAO_AMSDU1_CFG0_OFFSET                        0x00001850
#define WED_PAO_AMSDU1_CFG1_OFFSET                        0x00001854
#define WED_PAO_AMSDU2_CFG0_OFFSET                        0x00001860
#define WED_PAO_AMSDU2_CFG1_OFFSET                        0x00001864
#define WED_PAO_AMSDU3_CFG0_OFFSET                        0x00001870
#define WED_PAO_AMSDU3_CFG1_OFFSET                        0x00001874
#define WED_PAO_AMSDU4_CFG0_OFFSET                        0x00001880
#define WED_PAO_AMSDU4_CFG1_OFFSET                        0x00001884
#define WED_PAO_AMSDU5_CFG0_OFFSET                        0x00001890
#define WED_PAO_AMSDU5_CFG1_OFFSET                        0x00001894
#define WED_PAO_AMSDU6_CFG0_OFFSET                        0x000018A0
#define WED_PAO_AMSDU6_CFG1_OFFSET                        0x000018A4
#define WED_PAO_AMSDU7_CFG0_OFFSET                        0x000018B0
#define WED_PAO_AMSDU7_CFG1_OFFSET                        0x000018B4
#define WED_PAO_AMSDU8_CFG0_OFFSET                        0x000018C0
#define WED_PAO_AMSDU8_CFG1_OFFSET                        0x000018C4
#define WED_PAO_SCHED_OFFSET                              0x000018D0
#define WED_PAO_SCHED_CFG0_OFFSET                         0x000018D4
#define WED_PAO_SCHED_CFG1_OFFSET                         0x000018D8
#define WED_PAO_SCHED_CFG2_OFFSET                         0x000018DC
#define WED_PAO_SCHED_TIMER_OFFSET                        0x000018E0
#define WED_PAO_FQ_MAX_PCNT_OFFSET                        0x00001900
#define WED_PAO_FQ_DROP_OFFSET                            0x00001904
#define WED_PAO_FQ_LOW_OFFSET                             0x00001908
#define WED_PAO_FQ_HIGH_OFFSET                            0x0000190C
#define WED_PAO_PSE_OFFSET                                0x00001910
#define WED_PAO_PSE_TDM_TBL0_OFFSET                       0x00001914
#define WED_PAO_PSE_TDM_TBL1_OFFSET                       0x00001918
#define WED_PAO_PSE_TDM_TBL2_OFFSET                       0x0000191C
#define WED_PAO_PSE_TDM_TBL3_OFFSET                       0x00001920
#define WED_PAO_PSE_TDM_TBL4_OFFSET                       0x00001924
#define WED_PAO_PSE_TDM_TBL5_OFFSET                       0x00001928
#define WED_PAO_PSE_TDM_TBL6_OFFSET                       0x0000192C
#define WED_PAO_PSE_TDM_TBL7_OFFSET                       0x00001930
#define WED_PAO_PSE_TDM_TBL8_OFFSET                       0x00001934
#define WED_PAO_PSE_TDM_TBL9_OFFSET                       0x00001938
#define WED_PAO_PSE_TDM_TBL10_OFFSET                      0x0000193C
#define WED_PAO_PSE_TDM_TBL11_OFFSET                      0x00001940
#define WED_PAO_PSE_TDM_TBL12_OFFSET                      0x00001944
#define WED_PAO_PSE_TDM_TBL13_OFFSET                      0x00001948
#define WED_PAO_PSE_TDM_TBL14_OFFSET                      0x0000194C
#define WED_PAO_PSE_TDM_TBL15_OFFSET                      0x00001950
#define WED_PAO_PSE_SP_CFG0_OFFSET                        0x00001954
#define WED_PAO_PSE_SP_CFG1_OFFSET                        0x00001958
#define WED_PAO_WPDMAD_CFG0_OFFSET                        0x00001960
#define WED_PAO_WPDMAD_CFG1_OFFSET                        0x00001964
#define WED_PAO_HIFTXD_OFFSET                             0x00001968
#define WED_PAO_HIFTXD_BASE00_L_OFFSET                    0x00001980
#define WED_PAO_HIFTXD_BASE01_L_OFFSET                    0x00001984
#define WED_PAO_HIFTXD_BASE02_L_OFFSET                    0x00001988
#define WED_PAO_HIFTXD_BASE03_L_OFFSET                    0x0000198C
#define WED_PAO_HIFTXD_BASE04_L_OFFSET                    0x00001990
#define WED_PAO_HIFTXD_BASE05_L_OFFSET                    0x00001994
#define WED_PAO_HIFTXD_BASE06_L_OFFSET                    0x00001998
#define WED_PAO_HIFTXD_BASE07_L_OFFSET                    0x0000199C
#define WED_PAO_HIFTXD_BASE08_L_OFFSET                    0x000019A0
#define WED_PAO_HIFTXD_BASE09_L_OFFSET                    0x000019A4
#define WED_PAO_HIFTXD_BASE10_L_OFFSET                    0x000019A8
#define WED_PAO_HIFTXD_BASE11_L_OFFSET                    0x000019AC
#define WED_PAO_HIFTXD_BASE12_L_OFFSET                    0x000019B0
#define WED_PAO_HIFTXD_BASE13_L_OFFSET                    0x000019B4
#define WED_PAO_HIFTXD_BASE14_L_OFFSET                    0x000019B8
#define WED_PAO_HIFTXD_BASE15_L_OFFSET                    0x000019BC
#define WED_PAO_HIFTXD_BASE16_L_OFFSET                    0x000019C0
#define WED_PAO_HIFTXD_BASE17_L_OFFSET                    0x000019C4
#define WED_PAO_HIFTXD_BASE18_L_OFFSET                    0x000019C8
#define WED_PAO_HIFTXD_BASE19_L_OFFSET                    0x000019CC
#define WED_PAO_HIFTXD_BASE20_L_OFFSET                    0x000019D0
#define WED_PAO_HIFTXD_BASE21_L_OFFSET                    0x000019D4
#define WED_PAO_HIFTXD_BASE22_L_OFFSET                    0x000019D8
#define WED_PAO_HIFTXD_BASE23_L_OFFSET                    0x000019DC
#define WED_PAO_HIFTXD_BASE24_L_OFFSET                    0x000019E0
#define WED_PAO_HIFTXD_BASE25_L_OFFSET                    0x000019E4
#define WED_PAO_HIFTXD_BASE26_L_OFFSET                    0x000019E8
#define WED_PAO_HIFTXD_BASE27_L_OFFSET                    0x000019EC
#define WED_PAO_HIFTXD_BASE28_L_OFFSET                    0x000019F0
#define WED_PAO_HIFTXD_BASE29_L_OFFSET                    0x000019F4
#define WED_PAO_HIFTXD_BASE30_L_OFFSET                    0x000019F8
#define WED_PAO_HIFTXD_BASE31_L_OFFSET                    0x000019FC
#define WED_PAO_HIFTXD_BASE_00_07_H_OFFSET                0x00001A00
#define WED_PAO_HIFTXD_BASE_08_15_H_OFFSET                0x00001A04
#define WED_PAO_HIFTXD_BASE_16_23_H_OFFSET                0x00001A08
#define WED_PAO_HIFTXD_BASE_24_31_H_OFFSET                0x00001A0C
#define WED_PAO_MON_AMSDU_FIFO_OFFSET                     0x00001A30
#define WED_PAO_MON_AMSDU_FIFO_CNT0_OFFSET                0x00001A34
#define WED_PAO_MON_AMSDU_FIFO_CNT1_OFFSET                0x00001A38
#define WED_PAO_MON_AMSDU_FIFO_CNT2_OFFSET                0x00001A3C
#define WED_PAO_MON_AMSDU_FIFO_CNT3_OFFSET                0x00001A40
#define WED_PAO_MON_AMSDU_FIFO_CNT4_OFFSET                0x00001A44
#define WED_PAO_MON_AMSDU_FIFO_CNT5_OFFSET                0x00001A48
#define WED_PAO_MON_AMSDU_FIFO_CNT6_OFFSET                0x00001A4C
#define WED_PAO_MON_AMSDU_FIFO_CNT7_OFFSET                0x00001A50
#define WED_PAO_MON_AMSDU_FIFO_CNT8_OFFSET                0x00001A54
#define WED_PAO_MON_AMSDU_FIFO_DMAD_0_OFFSET              0x00001A5C
#define WED_PAO_MON_AMSDU_FIFO_DMAD_1_OFFSET              0x00001A60
#define WED_PAO_MON_AMSDU_FIFO_DMAD_2_OFFSET              0x00001A64
#define WED_PAO_MON_AMSDU_FIFO_DMAD_3_OFFSET              0x00001A68
#define WED_PAO_MON_AMSDU_FIFO_DMAD_4_OFFSET              0x00001A6C
#define WED_PAO_MON_AMSDU_ENG0_OFFSET                     0x00001A70
#define WED_PAO_MON_AMSDU_ENG0_CNT0_OFFSET                0x00001A74
#define WED_PAO_MON_AMSDU_ENG0_CNT1_OFFSET                0x00001A78
#define WED_PAO_MON_AMSDU_ENG0_CNT2_OFFSET                0x00001A7C
#define WED_PAO_MON_AMSDU_ENG0_CNT3_OFFSET                0x00001A80
#define WED_PAO_MON_AMSDU_ENG0_CNT4_OFFSET                0x00001A84
#define WED_PAO_MON_AMSDU_ENG0_CNT5_OFFSET                0x00001A88
#define WED_PAO_MON_AMSDU_ENG0_CNT6_OFFSET                0x00001A8C
#define WED_PAO_MON_AMSDU_ENG0_CNT7_OFFSET                0x00001A90
#define WED_PAO_MON_AMSDU_ENG0_CNT8_OFFSET                0x00001A94
#define WED_PAO_MON_AMSDU_ENG0_CNT9_OFFSET                0x00001A98
#define WED_PAO_MON_AMSDU_ENG0_CNT10_OFFSET               0x00001A9C
#define WED_PAO_MON_AMSDU_ENG0_CNT11_OFFSET               0x00001AA0
#define WED_PAO_MON_AMSDU_ENG0_CNT12_OFFSET               0x00001AA4
#define WED_PAO_MON_AMSDU_ENG0_CNT13_OFFSET               0x00001AA8
#define WED_PAO_MON_AMSDU_ENG0_PTR0_OFFSET                0x00001AAC
#define WED_PAO_MON_AMSDU_ENG0_PTR1_OFFSET                0x00001AB0
#define WED_PAO_MON_AMSDU_ENG0_PTR2_OFFSET                0x00001AB4
#define WED_PAO_MON_AMSDU_ENG1_OFFSET                     0x00001AC0
#define WED_PAO_MON_AMSDU_ENG1_CNT0_OFFSET                0x00001AC4
#define WED_PAO_MON_AMSDU_ENG1_CNT1_OFFSET                0x00001AC8
#define WED_PAO_MON_AMSDU_ENG1_CNT2_OFFSET                0x00001ACC
#define WED_PAO_MON_AMSDU_ENG1_CNT3_OFFSET                0x00001AD0
#define WED_PAO_MON_AMSDU_ENG1_CNT4_OFFSET                0x00001AD4
#define WED_PAO_MON_AMSDU_ENG1_CNT5_OFFSET                0x00001AD8
#define WED_PAO_MON_AMSDU_ENG1_CNT6_OFFSET                0x00001ADC
#define WED_PAO_MON_AMSDU_ENG1_CNT7_OFFSET                0x00001AE0
#define WED_PAO_MON_AMSDU_ENG1_CNT8_OFFSET                0x00001AE4
#define WED_PAO_MON_AMSDU_ENG1_CNT9_OFFSET                0x00001AE8
#define WED_PAO_MON_AMSDU_ENG1_CNT10_OFFSET               0x00001AEC
#define WED_PAO_MON_AMSDU_ENG1_CNT11_OFFSET               0x00001AF0
#define WED_PAO_MON_AMSDU_ENG1_CNT12_OFFSET               0x00001AF4
#define WED_PAO_MON_AMSDU_ENG1_CNT13_OFFSET               0x00001AF8
#define WED_PAO_MON_AMSDU_ENG1_PTR0_OFFSET                0x00001AFC
#define WED_PAO_MON_AMSDU_ENG1_PTR1_OFFSET                0x00001B00
#define WED_PAO_MON_AMSDU_ENG1_PTR2_OFFSET                0x00001B04
#define WED_PAO_MON_AMSDU_ENG2_OFFSET                     0x00001B10
#define WED_PAO_MON_AMSDU_ENG2_CNT0_OFFSET                0x00001B14
#define WED_PAO_MON_AMSDU_ENG2_CNT1_OFFSET                0x00001B18
#define WED_PAO_MON_AMSDU_ENG2_CNT2_OFFSET                0x00001B1C
#define WED_PAO_MON_AMSDU_ENG2_CNT3_OFFSET                0x00001B20
#define WED_PAO_MON_AMSDU_ENG2_CNT4_OFFSET                0x00001B24
#define WED_PAO_MON_AMSDU_ENG2_CNT5_OFFSET                0x00001B28
#define WED_PAO_MON_AMSDU_ENG2_CNT6_OFFSET                0x00001B2C
#define WED_PAO_MON_AMSDU_ENG2_CNT7_OFFSET                0x00001B30
#define WED_PAO_MON_AMSDU_ENG2_CNT8_OFFSET                0x00001B34
#define WED_PAO_MON_AMSDU_ENG2_CNT9_OFFSET                0x00001B38
#define WED_PAO_MON_AMSDU_ENG2_CNT10_OFFSET               0x00001B3C
#define WED_PAO_MON_AMSDU_ENG2_CNT11_OFFSET               0x00001B40
#define WED_PAO_MON_AMSDU_ENG2_CNT12_OFFSET               0x00001B44
#define WED_PAO_MON_AMSDU_ENG2_CNT13_OFFSET               0x00001B48
#define WED_PAO_MON_AMSDU_ENG2_PTR0_OFFSET                0x00001B4C
#define WED_PAO_MON_AMSDU_ENG2_PTR1_OFFSET                0x00001B50
#define WED_PAO_MON_AMSDU_ENG2_PTR2_OFFSET                0x00001B54
#define WED_PAO_MON_AMSDU_ENG3_OFFSET                     0x00001B60
#define WED_PAO_MON_AMSDU_ENG3_CNT0_OFFSET                0x00001B64
#define WED_PAO_MON_AMSDU_ENG3_CNT1_OFFSET                0x00001B68
#define WED_PAO_MON_AMSDU_ENG3_CNT2_OFFSET                0x00001B6C
#define WED_PAO_MON_AMSDU_ENG3_CNT3_OFFSET                0x00001B70
#define WED_PAO_MON_AMSDU_ENG3_CNT4_OFFSET                0x00001B74
#define WED_PAO_MON_AMSDU_ENG3_CNT5_OFFSET                0x00001B78
#define WED_PAO_MON_AMSDU_ENG3_CNT6_OFFSET                0x00001B7C
#define WED_PAO_MON_AMSDU_ENG3_CNT7_OFFSET                0x00001B80
#define WED_PAO_MON_AMSDU_ENG3_CNT8_OFFSET                0x00001B84
#define WED_PAO_MON_AMSDU_ENG3_CNT9_OFFSET                0x00001B88
#define WED_PAO_MON_AMSDU_ENG3_CNT10_OFFSET               0x00001B8C
#define WED_PAO_MON_AMSDU_ENG3_CNT11_OFFSET               0x00001B90
#define WED_PAO_MON_AMSDU_ENG3_CNT12_OFFSET               0x00001B94
#define WED_PAO_MON_AMSDU_ENG3_CNT13_OFFSET               0x00001B98
#define WED_PAO_MON_AMSDU_ENG3_PTR0_OFFSET                0x00001B9C
#define WED_PAO_MON_AMSDU_ENG3_PTR1_OFFSET                0x00001BA0
#define WED_PAO_MON_AMSDU_ENG3_PTR2_OFFSET                0x00001BA4
#define WED_PAO_MON_AMSDU_ENG4_OFFSET                     0x00001BB0
#define WED_PAO_MON_AMSDU_ENG4_CNT0_OFFSET                0x00001BB4
#define WED_PAO_MON_AMSDU_ENG4_CNT1_OFFSET                0x00001BB8
#define WED_PAO_MON_AMSDU_ENG4_CNT2_OFFSET                0x00001BBC
#define WED_PAO_MON_AMSDU_ENG4_CNT3_OFFSET                0x00001BC0
#define WED_PAO_MON_AMSDU_ENG4_CNT4_OFFSET                0x00001BC4
#define WED_PAO_MON_AMSDU_ENG4_CNT5_OFFSET                0x00001BC8
#define WED_PAO_MON_AMSDU_ENG4_CNT6_OFFSET                0x00001BCC
#define WED_PAO_MON_AMSDU_ENG4_CNT7_OFFSET                0x00001BD0
#define WED_PAO_MON_AMSDU_ENG4_CNT8_OFFSET                0x00001BD4
#define WED_PAO_MON_AMSDU_ENG4_CNT9_OFFSET                0x00001BD8
#define WED_PAO_MON_AMSDU_ENG4_CNT10_OFFSET               0x00001BDC
#define WED_PAO_MON_AMSDU_ENG4_CNT11_OFFSET               0x00001BE0
#define WED_PAO_MON_AMSDU_ENG4_CNT12_OFFSET               0x00001BE4
#define WED_PAO_MON_AMSDU_ENG4_CNT13_OFFSET               0x00001BE8
#define WED_PAO_MON_AMSDU_ENG4_PTR0_OFFSET                0x00001BEC
#define WED_PAO_MON_AMSDU_ENG4_PTR1_OFFSET                0x00001BF0
#define WED_PAO_MON_AMSDU_ENG4_PTR2_OFFSET                0x00001BF4
#define WED_PAO_MON_AMSDU_ENG5_OFFSET                     0x00001C00
#define WED_PAO_MON_AMSDU_ENG5_CNT0_OFFSET                0x00001C04
#define WED_PAO_MON_AMSDU_ENG5_CNT1_OFFSET                0x00001C08
#define WED_PAO_MON_AMSDU_ENG5_CNT2_OFFSET                0x00001C0C
#define WED_PAO_MON_AMSDU_ENG5_CNT3_OFFSET                0x00001C10
#define WED_PAO_MON_AMSDU_ENG5_CNT4_OFFSET                0x00001C14
#define WED_PAO_MON_AMSDU_ENG5_CNT5_OFFSET                0x00001C18
#define WED_PAO_MON_AMSDU_ENG5_CNT6_OFFSET                0x00001C1C
#define WED_PAO_MON_AMSDU_ENG5_CNT7_OFFSET                0x00001C20
#define WED_PAO_MON_AMSDU_ENG5_CNT8_OFFSET                0x00001C24
#define WED_PAO_MON_AMSDU_ENG5_CNT9_OFFSET                0x00001C28
#define WED_PAO_MON_AMSDU_ENG5_CNT10_OFFSET               0x00001C2C
#define WED_PAO_MON_AMSDU_ENG5_CNT11_OFFSET               0x00001C30
#define WED_PAO_MON_AMSDU_ENG5_CNT12_OFFSET               0x00001C34
#define WED_PAO_MON_AMSDU_ENG5_CNT13_OFFSET               0x00001C38
#define WED_PAO_MON_AMSDU_ENG5_PTR0_OFFSET                0x00001C3C
#define WED_PAO_MON_AMSDU_ENG5_PTR1_OFFSET                0x00001C40
#define WED_PAO_MON_AMSDU_ENG5_PTR2_OFFSET                0x00001C44
#define WED_PAO_MON_AMSDU_ENG6_OFFSET                     0x00001C50
#define WED_PAO_MON_AMSDU_ENG6_CNT0_OFFSET                0x00001C54
#define WED_PAO_MON_AMSDU_ENG6_CNT1_OFFSET                0x00001C58
#define WED_PAO_MON_AMSDU_ENG6_CNT2_OFFSET                0x00001C5C
#define WED_PAO_MON_AMSDU_ENG6_CNT3_OFFSET                0x00001C60
#define WED_PAO_MON_AMSDU_ENG6_CNT4_OFFSET                0x00001C64
#define WED_PAO_MON_AMSDU_ENG6_CNT5_OFFSET                0x00001C68
#define WED_PAO_MON_AMSDU_ENG6_CNT6_OFFSET                0x00001C6C
#define WED_PAO_MON_AMSDU_ENG6_CNT7_OFFSET                0x00001C70
#define WED_PAO_MON_AMSDU_ENG6_CNT8_OFFSET                0x00001C74
#define WED_PAO_MON_AMSDU_ENG6_CNT9_OFFSET                0x00001C78
#define WED_PAO_MON_AMSDU_ENG6_CNT10_OFFSET               0x00001C7C
#define WED_PAO_MON_AMSDU_ENG6_CNT11_OFFSET               0x00001C80
#define WED_PAO_MON_AMSDU_ENG6_CNT12_OFFSET               0x00001C84
#define WED_PAO_MON_AMSDU_ENG6_CNT13_OFFSET               0x00001C88
#define WED_PAO_MON_AMSDU_ENG6_PTR0_OFFSET                0x00001C8C
#define WED_PAO_MON_AMSDU_ENG6_PTR1_OFFSET                0x00001C90
#define WED_PAO_MON_AMSDU_ENG6_PTR2_OFFSET                0x00001C94
#define WED_PAO_MON_AMSDU_ENG7_OFFSET                     0x00001CA0
#define WED_PAO_MON_AMSDU_ENG7_CNT0_OFFSET                0x00001CA4
#define WED_PAO_MON_AMSDU_ENG7_CNT1_OFFSET                0x00001CA8
#define WED_PAO_MON_AMSDU_ENG7_CNT2_OFFSET                0x00001CAC
#define WED_PAO_MON_AMSDU_ENG7_CNT3_OFFSET                0x00001CB0
#define WED_PAO_MON_AMSDU_ENG7_CNT4_OFFSET                0x00001CB4
#define WED_PAO_MON_AMSDU_ENG7_CNT5_OFFSET                0x00001CB8
#define WED_PAO_MON_AMSDU_ENG7_CNT6_OFFSET                0x00001CBC
#define WED_PAO_MON_AMSDU_ENG7_CNT7_OFFSET                0x00001CC0
#define WED_PAO_MON_AMSDU_ENG7_CNT8_OFFSET                0x00001CC4
#define WED_PAO_MON_AMSDU_ENG7_CNT9_OFFSET                0x00001CC8
#define WED_PAO_MON_AMSDU_ENG7_CNT10_OFFSET               0x00001CCC
#define WED_PAO_MON_AMSDU_ENG7_CNT11_OFFSET               0x00001CD0
#define WED_PAO_MON_AMSDU_ENG7_CNT12_OFFSET               0x00001CD4
#define WED_PAO_MON_AMSDU_ENG7_CNT13_OFFSET               0x00001CD8
#define WED_PAO_MON_AMSDU_ENG7_PTR0_OFFSET                0x00001CDC
#define WED_PAO_MON_AMSDU_ENG7_PTR1_OFFSET                0x00001CE0
#define WED_PAO_MON_AMSDU_ENG7_PTR2_OFFSET                0x00001CE4
#define WED_PAO_MON_AMSDU_ENG8_OFFSET                     0x00001CF0
#define WED_PAO_MON_AMSDU_ENG8_CNT0_OFFSET                0x00001CF4
#define WED_PAO_MON_AMSDU_ENG8_CNT1_OFFSET                0x00001CF8
#define WED_PAO_MON_AMSDU_ENG8_CNT2_OFFSET                0x00001CFC
#define WED_PAO_MON_AMSDU_ENG8_CNT3_OFFSET                0x00001D00
#define WED_PAO_MON_AMSDU_ENG8_CNT4_OFFSET                0x00001D04
#define WED_PAO_MON_AMSDU_ENG8_CNT5_OFFSET                0x00001D08
#define WED_PAO_MON_AMSDU_ENG8_CNT6_OFFSET                0x00001D0C
#define WED_PAO_MON_AMSDU_ENG8_CNT7_OFFSET                0x00001D10
#define WED_PAO_MON_AMSDU_ENG8_CNT8_OFFSET                0x00001D14
#define WED_PAO_MON_AMSDU_ENG8_CNT9_OFFSET                0x00001D18
#define WED_PAO_MON_AMSDU_ENG8_CNT10_OFFSET               0x00001D1C
#define WED_PAO_MON_AMSDU_ENG8_CNT11_OFFSET               0x00001D20
#define WED_PAO_MON_AMSDU_ENG8_CNT12_OFFSET               0x00001D24
#define WED_PAO_MON_AMSDU_ENG8_CNT13_OFFSET               0x00001D28
#define WED_PAO_MON_AMSDU_ENG8_PTR0_OFFSET                0x00001D2C
#define WED_PAO_MON_AMSDU_ENG8_PTR1_OFFSET                0x00001D30
#define WED_PAO_MON_AMSDU_ENG8_PTR2_OFFSET                0x00001D34
#define WED_PAO_MON_QMEM_STS0_OFFSET                      0x00001E00
#define WED_PAO_MON_QMEM_STS1_OFFSET                      0x00001E04
#define WED_PAO_MON_QMEM_STS2_OFFSET                      0x00001E08
#define WED_PAO_MON_QMEM_CNT0_OFFSET                      0x00001E0C
#define WED_PAO_MON_QMEM_CNT1_OFFSET                      0x00001E10
#define WED_PAO_MON_QMEM_CNT2_OFFSET                      0x00001E14
#define WED_PAO_MON_QMEM_CNT3_OFFSET                      0x00001E18
#define WED_PAO_MON_QMEM_CNT4_OFFSET                      0x00001E1C
#define WED_PAO_MON_QMEM_PTR0_OFFSET                      0x00001E20
#define WED_PAO_MON_QMEM_PTR1_OFFSET                      0x00001E24
#define WED_PAO_MON_QMEM_PTR2_OFFSET                      0x00001E28
#define WED_PAO_MON_QMEM_PTR3_OFFSET                      0x00001E2C
#define WED_PAO_MON_QMEM_PTR4_OFFSET                      0x00001E30
#define WED_PAO_MON_QMEM_PTR5_OFFSET                      0x00001E34
#define WED_PAO_MON_QMEM_PTR6_OFFSET                      0x00001E38
#define WED_PAO_MON_QMEM_PTR7_OFFSET                      0x00001E3C
#define WED_PAO_MON_QMEM_PTR8_OFFSET                      0x00001E40
#define WED_PAO_MON_QMEM_PTR9_OFFSET                      0x00001E44
#define WED_PAO_MON_QMEM_PTR10_OFFSET                     0x00001E48
#define WED_PAO_MON_QMEM_PTR11_OFFSET                     0x00001E4C
#define WED_PAO_MON_QMEM_PTR12_OFFSET                     0x00001E50
#define WED_PAO_MON_QMEM_PTR13_OFFSET                     0x00001E54
#define WED_PAO_MON_QMEM_PTR14_OFFSET                     0x00001E58
#define WED_PAO_MON_AMSDU_SCHED_OFFSET                    0x00001E5C
#define WED_PAO_MON_HIFTXD_FETCH_OFFSET                   0x00001E70
#define WED_PAO_MON_HIFTXD_FETCH_CNT0_OFFSET              0x00001E74
#define WED_PAO_MON_HIFTXD_FETCH_CNT1_OFFSET              0x00001E78
#define WED_PAO_MON_HIFTXD_FETCH_CNT2_OFFSET              0x00001E7C
#define WED_PAO_MON_HIFTXD_FETCH_CNT3_OFFSET              0x00001E80
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1_OFFSET         0x00001E90
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_2_OFFSET         0x00001E94
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_3_OFFSET         0x00001E98
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_4_OFFSET         0x00001E9C
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_5_OFFSET         0x00001EA0
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_6_OFFSET         0x00001EA4
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_7_OFFSET         0x00001EA8
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_8_OFFSET         0x00001EAC
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_9_OFFSET         0x00001EB0
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_10_OFFSET        0x00001EB4
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_11_OFFSET        0x00001EB8
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_12_OFFSET        0x00001EBC
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_13_OFFSET        0x00001EC0
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1_OFFSET        0x00001EC4
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_2_OFFSET        0x00001EC8
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_3_OFFSET        0x00001ECC
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_4_OFFSET        0x00001ED0
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_5_OFFSET        0x00001ED4
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_6_OFFSET        0x00001ED8
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_7_OFFSET        0x00001EDC
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_8_OFFSET        0x00001EE0
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_9_OFFSET        0x00001EE4
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_10_OFFSET       0x00001EE8
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_11_OFFSET       0x00001EEC
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_12_OFFSET       0x00001EF0
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_13_OFFSET       0x00001EF4
#define WED_PAO_AMSDU_FIFO_ERR_OFFSET                     0x00001FD0
#define WED_PAO_DBG_CFG_OFFSET                            0x00001FE0
#define WED_PAO_DBG_CNT_OFFSET                            0x00001FE4
#define WED_PAO_DBG_OFFSET                                0x00001FE8
#define WED_PAO_PKTMEM_TEST_OFFSET                        0x00001FEC
#define WED_PAO_PKTMEM_ADDR_OFFSET                        0x00001FF0
#define WED_PAO_PKTMEM_DWSEL_OFFSET                       0x00001FF4
#define WED_PAO_PKTMEM_DATA_OFFSET                        0x00001FF8
#define WED_PAO_PKTMEM_TRIG_OFFSET                        0x00001FFC

/* =====================================================================================

  ---WED_PAO_AMSDU_FIFO (0x15015800 + 0x000)---

    MIN_SDL_SIZE[9..0]           - (RW) IF SDL < MIN_SDL_SIZE, PAO change AMSDU_EN to 1'b0  (i.e.  By default could be merged packet SDL at least ETH header 14 bytes +  payload 1 byte)
    IS_PRIOR0_RING[10]           - (RW) Set IS_PRIOR=0 to WFDMA ring#
    IS_PRIOR1_RING[11]           - (RW) Set IS_PRIOR=1 to WFDMA ring#
    ETHER_TYPE_MRG[12]           - (RW) Change AMSDU_EN when EtherType<0x0600
    ETHER_TYPE_CFG[13]           - (RW) Change EtherType value when EtherType<0x0600
    IS_JUMBO_MRG[14]             - (RW) Change AMSDU_EN when jumbo gather/scatter
    RESERVED15[15]               - (RO) Reserved bits
    JUMBO_TIMEOUT_DROP_EN[16]    - (RW) When receive jumbo DMAD, if the latter coming DMAD is waiting too long time, then drop current merged jumbo DMAD, for debug mode only
    JUMBO_BUF_NUM_EXCEED_DROP_EN[17] - (RW) When receive jumbo DMAD, if the coming DMAD exceeds max, then drop current merged jumbo DMAD, for debug mode only
    JUMBO_ID_ERR_1_DROP_EN[18]   - (RW) When receive jumbo DMAD, if the coming DMAD ID mismatch, then drop current merged jumbo DMAD, for debug mode only
    JUMBO_ID_ERR_0_DROP_EN[19]   - (RW) When receive jumbo DMAD, if the coming DMAD ID mismatch, then drop current merged jumbo DMAD, for debug mode only
    JUMBO_ERR_STOP_EN[20]        - (RW) Stop AMSDU_FIFO FSM when Error occur, send clr to release FSM to IDLE, for debug mode only
    LATCH_HIFTXD_OFF[21]         - (RW) Latch the latest partial HIFTXD when dispatch to AMSDU_ENG
    DMAD_LAT_OPT[25..22]         - (RW) Latch DMAD when dispatch to AMSDU_ENG
    RESERVED26[27..26]           - (RO) Reserved bits
    JUMBO_MAX_BUF_NUM[31..28]    - (RW) Set the maximum jumbo DMAD counts, for debug mode only

 =====================================================================================*/
#define WED_PAO_AMSDU_FIFO_JUMBO_MAX_BUF_NUM_OFFSET       WED_PAO_AMSDU_FIFO_OFFSET
#define WED_PAO_AMSDU_FIFO_JUMBO_MAX_BUF_NUM_MASK         0xF0000000                // JUMBO_MAX_BUF_NUM[31..28]
#define WED_PAO_AMSDU_FIFO_JUMBO_MAX_BUF_NUM_SHFT         28
#define WED_PAO_AMSDU_FIFO_DMAD_LAT_OPT_OFFSET            WED_PAO_AMSDU_FIFO_OFFSET
#define WED_PAO_AMSDU_FIFO_DMAD_LAT_OPT_MASK              0x03C00000                // DMAD_LAT_OPT[25..22]
#define WED_PAO_AMSDU_FIFO_DMAD_LAT_OPT_SHFT              22
#define WED_PAO_AMSDU_FIFO_LATCH_HIFTXD_OFF_OFFSET        WED_PAO_AMSDU_FIFO_OFFSET
#define WED_PAO_AMSDU_FIFO_LATCH_HIFTXD_OFF_MASK          0x00200000                // LATCH_HIFTXD_OFF[21]
#define WED_PAO_AMSDU_FIFO_LATCH_HIFTXD_OFF_SHFT          21
#define WED_PAO_AMSDU_FIFO_JUMBO_ERR_STOP_EN_OFFSET       WED_PAO_AMSDU_FIFO_OFFSET
#define WED_PAO_AMSDU_FIFO_JUMBO_ERR_STOP_EN_MASK         0x00100000                // JUMBO_ERR_STOP_EN[20]
#define WED_PAO_AMSDU_FIFO_JUMBO_ERR_STOP_EN_SHFT         20
#define WED_PAO_AMSDU_FIFO_JUMBO_ID_ERR_0_DROP_EN_OFFSET  WED_PAO_AMSDU_FIFO_OFFSET
#define WED_PAO_AMSDU_FIFO_JUMBO_ID_ERR_0_DROP_EN_MASK    0x00080000                // JUMBO_ID_ERR_0_DROP_EN[19]
#define WED_PAO_AMSDU_FIFO_JUMBO_ID_ERR_0_DROP_EN_SHFT    19
#define WED_PAO_AMSDU_FIFO_JUMBO_ID_ERR_1_DROP_EN_OFFSET  WED_PAO_AMSDU_FIFO_OFFSET
#define WED_PAO_AMSDU_FIFO_JUMBO_ID_ERR_1_DROP_EN_MASK    0x00040000                // JUMBO_ID_ERR_1_DROP_EN[18]
#define WED_PAO_AMSDU_FIFO_JUMBO_ID_ERR_1_DROP_EN_SHFT    18
#define WED_PAO_AMSDU_FIFO_JUMBO_BUF_NUM_EXCEED_DROP_EN_OFFSET WED_PAO_AMSDU_FIFO_OFFSET
#define WED_PAO_AMSDU_FIFO_JUMBO_BUF_NUM_EXCEED_DROP_EN_MASK 0x00020000                // JUMBO_BUF_NUM_EXCEED_DROP_EN[17]
#define WED_PAO_AMSDU_FIFO_JUMBO_BUF_NUM_EXCEED_DROP_EN_SHFT 17
#define WED_PAO_AMSDU_FIFO_JUMBO_TIMEOUT_DROP_EN_OFFSET   WED_PAO_AMSDU_FIFO_OFFSET
#define WED_PAO_AMSDU_FIFO_JUMBO_TIMEOUT_DROP_EN_MASK     0x00010000                // JUMBO_TIMEOUT_DROP_EN[16]
#define WED_PAO_AMSDU_FIFO_JUMBO_TIMEOUT_DROP_EN_SHFT     16
#define WED_PAO_AMSDU_FIFO_IS_JUMBO_MRG_OFFSET            WED_PAO_AMSDU_FIFO_OFFSET
#define WED_PAO_AMSDU_FIFO_IS_JUMBO_MRG_MASK              0x00004000                // IS_JUMBO_MRG[14]
#define WED_PAO_AMSDU_FIFO_IS_JUMBO_MRG_SHFT              14
#define WED_PAO_AMSDU_FIFO_ETHER_TYPE_CFG_OFFSET          WED_PAO_AMSDU_FIFO_OFFSET
#define WED_PAO_AMSDU_FIFO_ETHER_TYPE_CFG_MASK            0x00002000                // ETHER_TYPE_CFG[13]
#define WED_PAO_AMSDU_FIFO_ETHER_TYPE_CFG_SHFT            13
#define WED_PAO_AMSDU_FIFO_ETHER_TYPE_MRG_OFFSET          WED_PAO_AMSDU_FIFO_OFFSET
#define WED_PAO_AMSDU_FIFO_ETHER_TYPE_MRG_MASK            0x00001000                // ETHER_TYPE_MRG[12]
#define WED_PAO_AMSDU_FIFO_ETHER_TYPE_MRG_SHFT            12
#define WED_PAO_AMSDU_FIFO_IS_PRIOR1_RING_OFFSET          WED_PAO_AMSDU_FIFO_OFFSET
#define WED_PAO_AMSDU_FIFO_IS_PRIOR1_RING_MASK            0x00000800                // IS_PRIOR1_RING[11]
#define WED_PAO_AMSDU_FIFO_IS_PRIOR1_RING_SHFT            11
#define WED_PAO_AMSDU_FIFO_IS_PRIOR0_RING_OFFSET          WED_PAO_AMSDU_FIFO_OFFSET
#define WED_PAO_AMSDU_FIFO_IS_PRIOR0_RING_MASK            0x00000400                // IS_PRIOR0_RING[10]
#define WED_PAO_AMSDU_FIFO_IS_PRIOR0_RING_SHFT            10
#define WED_PAO_AMSDU_FIFO_MIN_SDL_SIZE_OFFSET            WED_PAO_AMSDU_FIFO_OFFSET
#define WED_PAO_AMSDU_FIFO_MIN_SDL_SIZE_MASK              0x000003FF                // MIN_SDL_SIZE[9..0]
#define WED_PAO_AMSDU_FIFO_MIN_SDL_SIZE_SHFT              0

/* =====================================================================================

  ---WED_PAO_TIMER (0x15015800 + 0x008)---

    DIV[2..0]                    - (RW) Timer Tick Freq. DIV(x1 125M)
    RESERVED3[3]                 - (RO) Reserved bits
    EN[4]                        - (RW) Timer for AMSDU_ENG
    RESERVED5[31..5]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_TIMER_EN_OFFSET                           WED_PAO_TIMER_OFFSET
#define WED_PAO_TIMER_EN_MASK                             0x00000010                // EN[4]
#define WED_PAO_TIMER_EN_SHFT                             4
#define WED_PAO_TIMER_DIV_OFFSET                          WED_PAO_TIMER_OFFSET
#define WED_PAO_TIMER_DIV_MASK                            0x00000007                // DIV[2..0]
#define WED_PAO_TIMER_DIV_SHFT                            0

/* =====================================================================================

  ---WED_PAO_TIMER_JUMBO (0x15015800 + 0x00C)---

    CYCLE[31..0]                 - (RW) Timer for AMSDU_FIFO (jumbo error detect), maximum cycle

 =====================================================================================*/
#define WED_PAO_TIMER_JUMBO_CYCLE_OFFSET                  WED_PAO_TIMER_JUMBO_OFFSET
#define WED_PAO_TIMER_JUMBO_CYCLE_MASK                    0xFFFFFFFF                // CYCLE[31..0]
#define WED_PAO_TIMER_JUMBO_CYCLE_SHFT                    0

/* =====================================================================================

  ---WED_PAO_STA_INFO (0x15015800 + 0x010)---

    DO_INIT[0]                   - (A0) Write 1 to trigger per-station info initialization sequence, then this bit will return to 0 automatically.
    DO_SET[1]                    - (A0) Write 1 to set one per-station info, then this bit will return to 0 automatically.
    DO_GET[2]                    - (A0) Write 1 to get one per-station info, then this bit will return to 0 automatically.
    RESERVED3[31..3]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_STA_INFO_DO_GET_OFFSET                    WED_PAO_STA_INFO_OFFSET
#define WED_PAO_STA_INFO_DO_GET_MASK                      0x00000004                // DO_GET[2]
#define WED_PAO_STA_INFO_DO_GET_SHFT                      2
#define WED_PAO_STA_INFO_DO_SET_OFFSET                    WED_PAO_STA_INFO_OFFSET
#define WED_PAO_STA_INFO_DO_SET_MASK                      0x00000002                // DO_SET[1]
#define WED_PAO_STA_INFO_DO_SET_SHFT                      1
#define WED_PAO_STA_INFO_DO_INIT_OFFSET                   WED_PAO_STA_INFO_OFFSET
#define WED_PAO_STA_INFO_DO_INIT_MASK                     0x00000001                // DO_INIT[0]
#define WED_PAO_STA_INFO_DO_INIT_SHFT                     0

/* =====================================================================================

  ---WED_PAO_STA_INFO_INIT (0x15015800 + 0x014)---

    CFG_WTBL_HDRT_MODE[0]        - (RW) for tx_byte_count calculation
    RMVL[1]                      - (RW) for tx_byte_count calculation
    MAX_AMSDU_LEN[7..2]          - (RW) When current AMSDU tx_byte_cnt (partial-HIFTXD tx_byte_cnt field) exceed MAX_AMSDU_LEN, PAO will not merge
    MAX_AMSDU_NUM[11..8]         - (RW) When current AMSDU_NUM exceed MAX_AMSDU_NUM, PAO will not merge
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_STA_INFO_INIT_MAX_AMSDU_NUM_OFFSET        WED_PAO_STA_INFO_INIT_OFFSET
#define WED_PAO_STA_INFO_INIT_MAX_AMSDU_NUM_MASK          0x00000F00                // MAX_AMSDU_NUM[11..8]
#define WED_PAO_STA_INFO_INIT_MAX_AMSDU_NUM_SHFT          8
#define WED_PAO_STA_INFO_INIT_MAX_AMSDU_LEN_OFFSET        WED_PAO_STA_INFO_INIT_OFFSET
#define WED_PAO_STA_INFO_INIT_MAX_AMSDU_LEN_MASK          0x000000FC                // MAX_AMSDU_LEN[7..2]
#define WED_PAO_STA_INFO_INIT_MAX_AMSDU_LEN_SHFT          2
#define WED_PAO_STA_INFO_INIT_RMVL_OFFSET                 WED_PAO_STA_INFO_INIT_OFFSET
#define WED_PAO_STA_INFO_INIT_RMVL_MASK                   0x00000002                // RMVL[1]
#define WED_PAO_STA_INFO_INIT_RMVL_SHFT                   1
#define WED_PAO_STA_INFO_INIT_CFG_WTBL_HDRT_MODE_OFFSET   WED_PAO_STA_INFO_INIT_OFFSET
#define WED_PAO_STA_INFO_INIT_CFG_WTBL_HDRT_MODE_MASK     0x00000001                // CFG_WTBL_HDRT_MODE[0]
#define WED_PAO_STA_INFO_INIT_CFG_WTBL_HDRT_MODE_SHFT     0

/* =====================================================================================

  ---WED_PAO_STA_INFO_SET (0x15015800 + 0x018)---

    CFG_WTBL_HDRT_MODE[0]        - (RW) For tx_byte_count calculation
    RMVL[1]                      - (RW) For tx_byte_count calculation
    MAX_AMSDU_LEN[7..2]          - (RW) When current AMSDU tx_byte_cnt (partial-HIFTXD tx_byte_cnt field) exceed MAX_AMSDU_LEN, PAO will not merge
    MAX_AMSDU_NUM[11..8]         - (RW) When current AMSDU_NUM exceed MAX_AMSDU_NUM, PAO will not merge
    RESERVED12[15..12]           - (RO) Reserved bits
    NUMBER[27..16]               - (RW) WCID number
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_STA_INFO_SET_NUMBER_OFFSET                WED_PAO_STA_INFO_SET_OFFSET
#define WED_PAO_STA_INFO_SET_NUMBER_MASK                  0x0FFF0000                // NUMBER[27..16]
#define WED_PAO_STA_INFO_SET_NUMBER_SHFT                  16
#define WED_PAO_STA_INFO_SET_MAX_AMSDU_NUM_OFFSET         WED_PAO_STA_INFO_SET_OFFSET
#define WED_PAO_STA_INFO_SET_MAX_AMSDU_NUM_MASK           0x00000F00                // MAX_AMSDU_NUM[11..8]
#define WED_PAO_STA_INFO_SET_MAX_AMSDU_NUM_SHFT           8
#define WED_PAO_STA_INFO_SET_MAX_AMSDU_LEN_OFFSET         WED_PAO_STA_INFO_SET_OFFSET
#define WED_PAO_STA_INFO_SET_MAX_AMSDU_LEN_MASK           0x000000FC                // MAX_AMSDU_LEN[7..2]
#define WED_PAO_STA_INFO_SET_MAX_AMSDU_LEN_SHFT           2
#define WED_PAO_STA_INFO_SET_RMVL_OFFSET                  WED_PAO_STA_INFO_SET_OFFSET
#define WED_PAO_STA_INFO_SET_RMVL_MASK                    0x00000002                // RMVL[1]
#define WED_PAO_STA_INFO_SET_RMVL_SHFT                    1
#define WED_PAO_STA_INFO_SET_CFG_WTBL_HDRT_MODE_OFFSET    WED_PAO_STA_INFO_SET_OFFSET
#define WED_PAO_STA_INFO_SET_CFG_WTBL_HDRT_MODE_MASK      0x00000001                // CFG_WTBL_HDRT_MODE[0]
#define WED_PAO_STA_INFO_SET_CFG_WTBL_HDRT_MODE_SHFT      0

/* =====================================================================================

  ---WED_PAO_STA_INFO_STS (0x15015800 + 0x01C)---

    INIT_BUSY[0]                 - (RO) Initialization busy
    SET_BUSY[1]                  - (RO) Set busy
    GET_BUSY[2]                  - (RO) Get busy
    INIT_DONE[3]                 - (RO) Initialization done flag
    CURR_STAT[8..4]              - (RO) Station information FSM
    DOUT[20..9]                  - (RO) Read per-station information
    RESERVED21[31..21]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_STA_INFO_STS_DOUT_OFFSET                  WED_PAO_STA_INFO_STS_OFFSET
#define WED_PAO_STA_INFO_STS_DOUT_MASK                    0x001FFE00                // DOUT[20..9]
#define WED_PAO_STA_INFO_STS_DOUT_SHFT                    9
#define WED_PAO_STA_INFO_STS_CURR_STAT_OFFSET             WED_PAO_STA_INFO_STS_OFFSET
#define WED_PAO_STA_INFO_STS_CURR_STAT_MASK               0x000001F0                // CURR_STAT[8..4]
#define WED_PAO_STA_INFO_STS_CURR_STAT_SHFT               4
#define WED_PAO_STA_INFO_STS_INIT_DONE_OFFSET             WED_PAO_STA_INFO_STS_OFFSET
#define WED_PAO_STA_INFO_STS_INIT_DONE_MASK               0x00000008                // INIT_DONE[3]
#define WED_PAO_STA_INFO_STS_INIT_DONE_SHFT               3
#define WED_PAO_STA_INFO_STS_GET_BUSY_OFFSET              WED_PAO_STA_INFO_STS_OFFSET
#define WED_PAO_STA_INFO_STS_GET_BUSY_MASK                0x00000004                // GET_BUSY[2]
#define WED_PAO_STA_INFO_STS_GET_BUSY_SHFT                2
#define WED_PAO_STA_INFO_STS_SET_BUSY_OFFSET              WED_PAO_STA_INFO_STS_OFFSET
#define WED_PAO_STA_INFO_STS_SET_BUSY_MASK                0x00000002                // SET_BUSY[1]
#define WED_PAO_STA_INFO_STS_SET_BUSY_SHFT                1
#define WED_PAO_STA_INFO_STS_INIT_BUSY_OFFSET             WED_PAO_STA_INFO_STS_OFFSET
#define WED_PAO_STA_INFO_STS_INIT_BUSY_MASK               0x00000001                // INIT_BUSY[0]
#define WED_PAO_STA_INFO_STS_INIT_BUSY_SHFT               0

/* =====================================================================================

  ---WED_PAO_COMP0_MASK0 (0x15015800 + 0x020)---

    MASK[31..0]                  - (RW) 1st stage compare mask, for compare WCID ([31:0])

 =====================================================================================*/
#define WED_PAO_COMP0_MASK0_MASK_OFFSET                   WED_PAO_COMP0_MASK0_OFFSET
#define WED_PAO_COMP0_MASK0_MASK_MASK                     0xFFFFFFFF                // MASK[31..0]
#define WED_PAO_COMP0_MASK0_MASK_SHFT                     0

/* =====================================================================================

  ---WED_PAO_COMP0_MASK1 (0x15015800 + 0x024)---

    MASK[31..0]                  - (RW) 1st stage compare mask, for compare WCID (63:32])
                                     1st compare hit && 2nd compare miss => En-queue (no search)
                                     1st compare miss && 2nd compare miss => Search and Compare to HEAD

 =====================================================================================*/
#define WED_PAO_COMP0_MASK1_MASK_OFFSET                   WED_PAO_COMP0_MASK1_OFFSET
#define WED_PAO_COMP0_MASK1_MASK_MASK                     0xFFFFFFFF                // MASK[31..0]
#define WED_PAO_COMP0_MASK1_MASK_SHFT                     0

/* =====================================================================================

  ---WED_PAO_COMP1_MASK0 (0x15015800 + 0x028)---

    MASK[31..0]                  - (RW) 2nd stage compare mask, for compare other information ([31:0])

 =====================================================================================*/
#define WED_PAO_COMP1_MASK0_MASK_OFFSET                   WED_PAO_COMP1_MASK0_OFFSET
#define WED_PAO_COMP1_MASK0_MASK_MASK                     0xFFFFFFFF                // MASK[31..0]
#define WED_PAO_COMP1_MASK0_MASK_SHFT                     0

/* =====================================================================================

  ---WED_PAO_COMP1_MASK1 (0x15015800 + 0x02C)---

    MASK[31..0]                  - (RW) 2nd stage compare mask, for compare other information ([63:32])
                                     1st compare hit && 2nd compare miss => En-queue (no search)
                                     1st compare miss && 2nd compare miss => Search and Compare to HEAD

 =====================================================================================*/
#define WED_PAO_COMP1_MASK1_MASK_OFFSET                   WED_PAO_COMP1_MASK1_OFFSET
#define WED_PAO_COMP1_MASK1_MASK_MASK                     0xFFFFFFFF                // MASK[31..0]
#define WED_PAO_COMP1_MASK1_MASK_SHFT                     0

/* =====================================================================================

  ---WED_PAO_TX_BYTE_CNT_CFG0 (0x15015800 + 0x030)---

    CONST01[5..0]                - (RW) Constant for TX_BYTE_CNT calculation
    CONST00[11..6]               - (RW) Constant for TX_BYTE_CNT calculation
    TXD_LEN[17..12]              - (RW) Constant for TX_BYTE_CNT calculation
    SUB_VAL[23..18]              - (RW) Constant for TX_BYTE_CNT calculation
    ADD_VAL[29..24]              - (RW) Constant for TX_BYTE_CNT calculation
    AMSDU_ENQ_OPT[30]            - (RW) AMSDU_ENG en-queue option
    RESERVED31[31]               - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_TX_BYTE_CNT_CFG0_AMSDU_ENQ_OPT_OFFSET     WED_PAO_TX_BYTE_CNT_CFG0_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG0_AMSDU_ENQ_OPT_MASK       0x40000000                // AMSDU_ENQ_OPT[30]
#define WED_PAO_TX_BYTE_CNT_CFG0_AMSDU_ENQ_OPT_SHFT       30
#define WED_PAO_TX_BYTE_CNT_CFG0_ADD_VAL_OFFSET           WED_PAO_TX_BYTE_CNT_CFG0_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG0_ADD_VAL_MASK             0x3F000000                // ADD_VAL[29..24]
#define WED_PAO_TX_BYTE_CNT_CFG0_ADD_VAL_SHFT             24
#define WED_PAO_TX_BYTE_CNT_CFG0_SUB_VAL_OFFSET           WED_PAO_TX_BYTE_CNT_CFG0_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG0_SUB_VAL_MASK             0x00FC0000                // SUB_VAL[23..18]
#define WED_PAO_TX_BYTE_CNT_CFG0_SUB_VAL_SHFT             18
#define WED_PAO_TX_BYTE_CNT_CFG0_TXD_LEN_OFFSET           WED_PAO_TX_BYTE_CNT_CFG0_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG0_TXD_LEN_MASK             0x0003F000                // TXD_LEN[17..12]
#define WED_PAO_TX_BYTE_CNT_CFG0_TXD_LEN_SHFT             12
#define WED_PAO_TX_BYTE_CNT_CFG0_CONST00_OFFSET           WED_PAO_TX_BYTE_CNT_CFG0_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG0_CONST00_MASK             0x00000FC0                // CONST00[11..6]
#define WED_PAO_TX_BYTE_CNT_CFG0_CONST00_SHFT             6
#define WED_PAO_TX_BYTE_CNT_CFG0_CONST01_OFFSET           WED_PAO_TX_BYTE_CNT_CFG0_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG0_CONST01_MASK             0x0000003F                // CONST01[5..0]
#define WED_PAO_TX_BYTE_CNT_CFG0_CONST01_SHFT             0

/* =====================================================================================

  ---WED_PAO_TX_BYTE_CNT_CFG1 (0x15015800 + 0x034)---

    CONST06[5..0]                - (RW)  xxx 
    CONST05[11..6]               - (RW) Constant for TX_BYTE_CNT calculation
    CONST04[17..12]              - (RW) Constant for TX_BYTE_CNT calculation
    CONST03[23..18]              - (RW) Constant for TX_BYTE_CNT calculation
    CONST02[29..24]              - (RW) Constant for TX_BYTE_CNT calculation
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_TX_BYTE_CNT_CFG1_CONST02_OFFSET           WED_PAO_TX_BYTE_CNT_CFG1_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG1_CONST02_MASK             0x3F000000                // CONST02[29..24]
#define WED_PAO_TX_BYTE_CNT_CFG1_CONST02_SHFT             24
#define WED_PAO_TX_BYTE_CNT_CFG1_CONST03_OFFSET           WED_PAO_TX_BYTE_CNT_CFG1_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG1_CONST03_MASK             0x00FC0000                // CONST03[23..18]
#define WED_PAO_TX_BYTE_CNT_CFG1_CONST03_SHFT             18
#define WED_PAO_TX_BYTE_CNT_CFG1_CONST04_OFFSET           WED_PAO_TX_BYTE_CNT_CFG1_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG1_CONST04_MASK             0x0003F000                // CONST04[17..12]
#define WED_PAO_TX_BYTE_CNT_CFG1_CONST04_SHFT             12
#define WED_PAO_TX_BYTE_CNT_CFG1_CONST05_OFFSET           WED_PAO_TX_BYTE_CNT_CFG1_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG1_CONST05_MASK             0x00000FC0                // CONST05[11..6]
#define WED_PAO_TX_BYTE_CNT_CFG1_CONST05_SHFT             6
#define WED_PAO_TX_BYTE_CNT_CFG1_CONST06_OFFSET           WED_PAO_TX_BYTE_CNT_CFG1_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG1_CONST06_MASK             0x0000003F                // CONST06[5..0]
#define WED_PAO_TX_BYTE_CNT_CFG1_CONST06_SHFT             0

/* =====================================================================================

  ---WED_PAO_TX_BYTE_CNT_CFG2 (0x15015800 + 0x038)---

    CONST11[5..0]                - (RW) Constant for TX_BYTE_CNT calculation
    CONST10[11..6]               - (RW) Constant for TX_BYTE_CNT calculation
    CONST09[17..12]              - (RW) Constant for TX_BYTE_CNT calculation
    CONST08[23..18]              - (RW) Constant for TX_BYTE_CNT calculation
    CONST07[29..24]              - (RW) Constant for TX_BYTE_CNT calculation
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_TX_BYTE_CNT_CFG2_CONST07_OFFSET           WED_PAO_TX_BYTE_CNT_CFG2_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG2_CONST07_MASK             0x3F000000                // CONST07[29..24]
#define WED_PAO_TX_BYTE_CNT_CFG2_CONST07_SHFT             24
#define WED_PAO_TX_BYTE_CNT_CFG2_CONST08_OFFSET           WED_PAO_TX_BYTE_CNT_CFG2_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG2_CONST08_MASK             0x00FC0000                // CONST08[23..18]
#define WED_PAO_TX_BYTE_CNT_CFG2_CONST08_SHFT             18
#define WED_PAO_TX_BYTE_CNT_CFG2_CONST09_OFFSET           WED_PAO_TX_BYTE_CNT_CFG2_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG2_CONST09_MASK             0x0003F000                // CONST09[17..12]
#define WED_PAO_TX_BYTE_CNT_CFG2_CONST09_SHFT             12
#define WED_PAO_TX_BYTE_CNT_CFG2_CONST10_OFFSET           WED_PAO_TX_BYTE_CNT_CFG2_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG2_CONST10_MASK             0x00000FC0                // CONST10[11..6]
#define WED_PAO_TX_BYTE_CNT_CFG2_CONST10_SHFT             6
#define WED_PAO_TX_BYTE_CNT_CFG2_CONST11_OFFSET           WED_PAO_TX_BYTE_CNT_CFG2_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG2_CONST11_MASK             0x0000003F                // CONST11[5..0]
#define WED_PAO_TX_BYTE_CNT_CFG2_CONST11_SHFT             0

/* =====================================================================================

  ---WED_PAO_TX_BYTE_CNT_CFG3 (0x15015800 + 0x03C)---

    PADING3[1..0]                - (RW) Constant for TX_BYTE_CNT calculation
    PADING2[3..2]                - (RW) Constant for TX_BYTE_CNT calculation
    PADING1[5..4]                - (RW) Constant for TX_BYTE_CNT calculation
    PADING0[7..6]                - (RW) Constant for TX_BYTE_CNT calculation
    CONST15[13..8]               - (RW) Constant for TX_BYTE_CNT calculation
    CONST14[19..14]              - (RW) Constant for TX_BYTE_CNT calculation
    CONST13[25..20]              - (RW) Constant for TX_BYTE_CNT calculation
    CONST12[31..26]              - (RW) Constant for TX_BYTE_CNT calculation

 =====================================================================================*/
#define WED_PAO_TX_BYTE_CNT_CFG3_CONST12_OFFSET           WED_PAO_TX_BYTE_CNT_CFG3_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG3_CONST12_MASK             0xFC000000                // CONST12[31..26]
#define WED_PAO_TX_BYTE_CNT_CFG3_CONST12_SHFT             26
#define WED_PAO_TX_BYTE_CNT_CFG3_CONST13_OFFSET           WED_PAO_TX_BYTE_CNT_CFG3_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG3_CONST13_MASK             0x03F00000                // CONST13[25..20]
#define WED_PAO_TX_BYTE_CNT_CFG3_CONST13_SHFT             20
#define WED_PAO_TX_BYTE_CNT_CFG3_CONST14_OFFSET           WED_PAO_TX_BYTE_CNT_CFG3_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG3_CONST14_MASK             0x000FC000                // CONST14[19..14]
#define WED_PAO_TX_BYTE_CNT_CFG3_CONST14_SHFT             14
#define WED_PAO_TX_BYTE_CNT_CFG3_CONST15_OFFSET           WED_PAO_TX_BYTE_CNT_CFG3_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG3_CONST15_MASK             0x00003F00                // CONST15[13..8]
#define WED_PAO_TX_BYTE_CNT_CFG3_CONST15_SHFT             8
#define WED_PAO_TX_BYTE_CNT_CFG3_PADING0_OFFSET           WED_PAO_TX_BYTE_CNT_CFG3_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG3_PADING0_MASK             0x000000C0                // PADING0[7..6]
#define WED_PAO_TX_BYTE_CNT_CFG3_PADING0_SHFT             6
#define WED_PAO_TX_BYTE_CNT_CFG3_PADING1_OFFSET           WED_PAO_TX_BYTE_CNT_CFG3_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG3_PADING1_MASK             0x00000030                // PADING1[5..4]
#define WED_PAO_TX_BYTE_CNT_CFG3_PADING1_SHFT             4
#define WED_PAO_TX_BYTE_CNT_CFG3_PADING2_OFFSET           WED_PAO_TX_BYTE_CNT_CFG3_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG3_PADING2_MASK             0x0000000C                // PADING2[3..2]
#define WED_PAO_TX_BYTE_CNT_CFG3_PADING2_SHFT             2
#define WED_PAO_TX_BYTE_CNT_CFG3_PADING3_OFFSET           WED_PAO_TX_BYTE_CNT_CFG3_OFFSET
#define WED_PAO_TX_BYTE_CNT_CFG3_PADING3_MASK             0x00000003                // PADING3[1..0]
#define WED_PAO_TX_BYTE_CNT_CFG3_PADING3_SHFT             0

/* =====================================================================================

  ---WED_PAO_AMSDU0_CFG0 (0x15015800 + 0x040)---

    FAST_DEQ_OPT[4..0]           - (RW) AMSDU_ENG Control Setting : 
                                     Current queue is ready for de-queue option
    TXP_NUM_MAX[9..5]            - (RW) Set AMSDU_ENG Max merge TXP count
    PL_CNT_MAX[20..10]           - (RW) Set AMSDU_ENG queue entry usage
    SEARCH_MAX[31..21]           - (RW) Set AMSDU_ENG search times for one MSDU

 =====================================================================================*/
#define WED_PAO_AMSDU0_CFG0_SEARCH_MAX_OFFSET             WED_PAO_AMSDU0_CFG0_OFFSET
#define WED_PAO_AMSDU0_CFG0_SEARCH_MAX_MASK               0xFFE00000                // SEARCH_MAX[31..21]
#define WED_PAO_AMSDU0_CFG0_SEARCH_MAX_SHFT               21
#define WED_PAO_AMSDU0_CFG0_PL_CNT_MAX_OFFSET             WED_PAO_AMSDU0_CFG0_OFFSET
#define WED_PAO_AMSDU0_CFG0_PL_CNT_MAX_MASK               0x001FFC00                // PL_CNT_MAX[20..10]
#define WED_PAO_AMSDU0_CFG0_PL_CNT_MAX_SHFT               10
#define WED_PAO_AMSDU0_CFG0_TXP_NUM_MAX_OFFSET            WED_PAO_AMSDU0_CFG0_OFFSET
#define WED_PAO_AMSDU0_CFG0_TXP_NUM_MAX_MASK              0x000003E0                // TXP_NUM_MAX[9..5]
#define WED_PAO_AMSDU0_CFG0_TXP_NUM_MAX_SHFT              5
#define WED_PAO_AMSDU0_CFG0_FAST_DEQ_OPT_OFFSET           WED_PAO_AMSDU0_CFG0_OFFSET
#define WED_PAO_AMSDU0_CFG0_FAST_DEQ_OPT_MASK             0x0000001F                // FAST_DEQ_OPT[4..0]
#define WED_PAO_AMSDU0_CFG0_FAST_DEQ_OPT_SHFT             0

/* =====================================================================================

  ---WED_PAO_AMSDU0_CFG1 (0x15015800 + 0x044)---

    WAIT_DMAD_CYCLE[13..0]       - (RW) Set AMSDU_ENG wait new MSDU time
    WAIT_CYCLE[27..14]           - (RW) Set per MSDU live time in QMEM (wait for merge)
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_AMSDU0_CFG1_WAIT_CYCLE_OFFSET             WED_PAO_AMSDU0_CFG1_OFFSET
#define WED_PAO_AMSDU0_CFG1_WAIT_CYCLE_MASK               0x0FFFC000                // WAIT_CYCLE[27..14]
#define WED_PAO_AMSDU0_CFG1_WAIT_CYCLE_SHFT               14
#define WED_PAO_AMSDU0_CFG1_WAIT_DMAD_CYCLE_OFFSET        WED_PAO_AMSDU0_CFG1_OFFSET
#define WED_PAO_AMSDU0_CFG1_WAIT_DMAD_CYCLE_MASK          0x00003FFF                // WAIT_DMAD_CYCLE[13..0]
#define WED_PAO_AMSDU0_CFG1_WAIT_DMAD_CYCLE_SHFT          0

/* =====================================================================================

  ---WED_PAO_AMSDU1_CFG0 (0x15015800 + 0x050)---

    FAST_DEQ_OPT[4..0]           - (RW) AMSDU_ENG Control Setting : 
                                     Current queue is ready for de-queue option
    TXP_NUM_MAX[9..5]            - (RW) Set AMSDU_ENG Max merge TXP count
    PL_CNT_MAX[20..10]           - (RW) Set AMSDU_ENG queue entry usage
    SEARCH_MAX[31..21]           - (RW) Set AMSDU_ENG search times for one MSDU

 =====================================================================================*/
#define WED_PAO_AMSDU1_CFG0_SEARCH_MAX_OFFSET             WED_PAO_AMSDU1_CFG0_OFFSET
#define WED_PAO_AMSDU1_CFG0_SEARCH_MAX_MASK               0xFFE00000                // SEARCH_MAX[31..21]
#define WED_PAO_AMSDU1_CFG0_SEARCH_MAX_SHFT               21
#define WED_PAO_AMSDU1_CFG0_PL_CNT_MAX_OFFSET             WED_PAO_AMSDU1_CFG0_OFFSET
#define WED_PAO_AMSDU1_CFG0_PL_CNT_MAX_MASK               0x001FFC00                // PL_CNT_MAX[20..10]
#define WED_PAO_AMSDU1_CFG0_PL_CNT_MAX_SHFT               10
#define WED_PAO_AMSDU1_CFG0_TXP_NUM_MAX_OFFSET            WED_PAO_AMSDU1_CFG0_OFFSET
#define WED_PAO_AMSDU1_CFG0_TXP_NUM_MAX_MASK              0x000003E0                // TXP_NUM_MAX[9..5]
#define WED_PAO_AMSDU1_CFG0_TXP_NUM_MAX_SHFT              5
#define WED_PAO_AMSDU1_CFG0_FAST_DEQ_OPT_OFFSET           WED_PAO_AMSDU1_CFG0_OFFSET
#define WED_PAO_AMSDU1_CFG0_FAST_DEQ_OPT_MASK             0x0000001F                // FAST_DEQ_OPT[4..0]
#define WED_PAO_AMSDU1_CFG0_FAST_DEQ_OPT_SHFT             0

/* =====================================================================================

  ---WED_PAO_AMSDU1_CFG1 (0x15015800 + 0x054)---

    WAIT_DMAD_CYCLE[13..0]       - (RW) Set AMSDU_ENG wait new MSDU time
    WAIT_CYCLE[27..14]           - (RW) Set per MSDU live time in QMEM (wait for merge)
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_AMSDU1_CFG1_WAIT_CYCLE_OFFSET             WED_PAO_AMSDU1_CFG1_OFFSET
#define WED_PAO_AMSDU1_CFG1_WAIT_CYCLE_MASK               0x0FFFC000                // WAIT_CYCLE[27..14]
#define WED_PAO_AMSDU1_CFG1_WAIT_CYCLE_SHFT               14
#define WED_PAO_AMSDU1_CFG1_WAIT_DMAD_CYCLE_OFFSET        WED_PAO_AMSDU1_CFG1_OFFSET
#define WED_PAO_AMSDU1_CFG1_WAIT_DMAD_CYCLE_MASK          0x00003FFF                // WAIT_DMAD_CYCLE[13..0]
#define WED_PAO_AMSDU1_CFG1_WAIT_DMAD_CYCLE_SHFT          0

/* =====================================================================================

  ---WED_PAO_AMSDU2_CFG0 (0x15015800 + 0x060)---

    FAST_DEQ_OPT[4..0]           - (RW) AMSDU_ENG Control Setting : 
                                     Current queue is ready for de-queue option
    TXP_NUM_MAX[9..5]            - (RW) Set AMSDU_ENG Max merge TXP count
    PL_CNT_MAX[20..10]           - (RW) Set AMSDU_ENG queue entry usage
    SEARCH_MAX[31..21]           - (RW) Set AMSDU_ENG search times for one MSDU

 =====================================================================================*/
#define WED_PAO_AMSDU2_CFG0_SEARCH_MAX_OFFSET             WED_PAO_AMSDU2_CFG0_OFFSET
#define WED_PAO_AMSDU2_CFG0_SEARCH_MAX_MASK               0xFFE00000                // SEARCH_MAX[31..21]
#define WED_PAO_AMSDU2_CFG0_SEARCH_MAX_SHFT               21
#define WED_PAO_AMSDU2_CFG0_PL_CNT_MAX_OFFSET             WED_PAO_AMSDU2_CFG0_OFFSET
#define WED_PAO_AMSDU2_CFG0_PL_CNT_MAX_MASK               0x001FFC00                // PL_CNT_MAX[20..10]
#define WED_PAO_AMSDU2_CFG0_PL_CNT_MAX_SHFT               10
#define WED_PAO_AMSDU2_CFG0_TXP_NUM_MAX_OFFSET            WED_PAO_AMSDU2_CFG0_OFFSET
#define WED_PAO_AMSDU2_CFG0_TXP_NUM_MAX_MASK              0x000003E0                // TXP_NUM_MAX[9..5]
#define WED_PAO_AMSDU2_CFG0_TXP_NUM_MAX_SHFT              5
#define WED_PAO_AMSDU2_CFG0_FAST_DEQ_OPT_OFFSET           WED_PAO_AMSDU2_CFG0_OFFSET
#define WED_PAO_AMSDU2_CFG0_FAST_DEQ_OPT_MASK             0x0000001F                // FAST_DEQ_OPT[4..0]
#define WED_PAO_AMSDU2_CFG0_FAST_DEQ_OPT_SHFT             0

/* =====================================================================================

  ---WED_PAO_AMSDU2_CFG1 (0x15015800 + 0x064)---

    WAIT_DMAD_CYCLE[13..0]       - (RW) Set AMSDU_ENG wait new MSDU time
    WAIT_CYCLE[27..14]           - (RW) Set per MSDU live time in QMEM (wait for merge)
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_AMSDU2_CFG1_WAIT_CYCLE_OFFSET             WED_PAO_AMSDU2_CFG1_OFFSET
#define WED_PAO_AMSDU2_CFG1_WAIT_CYCLE_MASK               0x0FFFC000                // WAIT_CYCLE[27..14]
#define WED_PAO_AMSDU2_CFG1_WAIT_CYCLE_SHFT               14
#define WED_PAO_AMSDU2_CFG1_WAIT_DMAD_CYCLE_OFFSET        WED_PAO_AMSDU2_CFG1_OFFSET
#define WED_PAO_AMSDU2_CFG1_WAIT_DMAD_CYCLE_MASK          0x00003FFF                // WAIT_DMAD_CYCLE[13..0]
#define WED_PAO_AMSDU2_CFG1_WAIT_DMAD_CYCLE_SHFT          0

/* =====================================================================================

  ---WED_PAO_AMSDU3_CFG0 (0x15015800 + 0x070)---

    FAST_DEQ_OPT[4..0]           - (RW) AMSDU_ENG Control Setting : 
                                     Current queue is ready for de-queue option
    TXP_NUM_MAX[9..5]            - (RW) Set AMSDU_ENG Max merge TXP count
    PL_CNT_MAX[20..10]           - (RW) Set AMSDU_ENG queue entry usage
    SEARCH_MAX[31..21]           - (RW) Set AMSDU_ENG search times for one MSDU

 =====================================================================================*/
#define WED_PAO_AMSDU3_CFG0_SEARCH_MAX_OFFSET             WED_PAO_AMSDU3_CFG0_OFFSET
#define WED_PAO_AMSDU3_CFG0_SEARCH_MAX_MASK               0xFFE00000                // SEARCH_MAX[31..21]
#define WED_PAO_AMSDU3_CFG0_SEARCH_MAX_SHFT               21
#define WED_PAO_AMSDU3_CFG0_PL_CNT_MAX_OFFSET             WED_PAO_AMSDU3_CFG0_OFFSET
#define WED_PAO_AMSDU3_CFG0_PL_CNT_MAX_MASK               0x001FFC00                // PL_CNT_MAX[20..10]
#define WED_PAO_AMSDU3_CFG0_PL_CNT_MAX_SHFT               10
#define WED_PAO_AMSDU3_CFG0_TXP_NUM_MAX_OFFSET            WED_PAO_AMSDU3_CFG0_OFFSET
#define WED_PAO_AMSDU3_CFG0_TXP_NUM_MAX_MASK              0x000003E0                // TXP_NUM_MAX[9..5]
#define WED_PAO_AMSDU3_CFG0_TXP_NUM_MAX_SHFT              5
#define WED_PAO_AMSDU3_CFG0_FAST_DEQ_OPT_OFFSET           WED_PAO_AMSDU3_CFG0_OFFSET
#define WED_PAO_AMSDU3_CFG0_FAST_DEQ_OPT_MASK             0x0000001F                // FAST_DEQ_OPT[4..0]
#define WED_PAO_AMSDU3_CFG0_FAST_DEQ_OPT_SHFT             0

/* =====================================================================================

  ---WED_PAO_AMSDU3_CFG1 (0x15015800 + 0x074)---

    WAIT_DMAD_CYCLE[13..0]       - (RW) Set AMSDU_ENG wait new MSDU time
    WAIT_CYCLE[27..14]           - (RW) Set per MSDU live time in QMEM (wait for merge)
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_AMSDU3_CFG1_WAIT_CYCLE_OFFSET             WED_PAO_AMSDU3_CFG1_OFFSET
#define WED_PAO_AMSDU3_CFG1_WAIT_CYCLE_MASK               0x0FFFC000                // WAIT_CYCLE[27..14]
#define WED_PAO_AMSDU3_CFG1_WAIT_CYCLE_SHFT               14
#define WED_PAO_AMSDU3_CFG1_WAIT_DMAD_CYCLE_OFFSET        WED_PAO_AMSDU3_CFG1_OFFSET
#define WED_PAO_AMSDU3_CFG1_WAIT_DMAD_CYCLE_MASK          0x00003FFF                // WAIT_DMAD_CYCLE[13..0]
#define WED_PAO_AMSDU3_CFG1_WAIT_DMAD_CYCLE_SHFT          0

/* =====================================================================================

  ---WED_PAO_AMSDU4_CFG0 (0x15015800 + 0x080)---

    FAST_DEQ_OPT[4..0]           - (RW) AMSDU_ENG Control Setting : 
                                     Current queue is ready for de-queue option
    TXP_NUM_MAX[9..5]            - (RW) Set AMSDU_ENG Max merge TXP count
    PL_CNT_MAX[20..10]           - (RW) Set AMSDU_ENG queue entry usage
    SEARCH_MAX[31..21]           - (RW) Set AMSDU_ENG search times for one MSDU

 =====================================================================================*/
#define WED_PAO_AMSDU4_CFG0_SEARCH_MAX_OFFSET             WED_PAO_AMSDU4_CFG0_OFFSET
#define WED_PAO_AMSDU4_CFG0_SEARCH_MAX_MASK               0xFFE00000                // SEARCH_MAX[31..21]
#define WED_PAO_AMSDU4_CFG0_SEARCH_MAX_SHFT               21
#define WED_PAO_AMSDU4_CFG0_PL_CNT_MAX_OFFSET             WED_PAO_AMSDU4_CFG0_OFFSET
#define WED_PAO_AMSDU4_CFG0_PL_CNT_MAX_MASK               0x001FFC00                // PL_CNT_MAX[20..10]
#define WED_PAO_AMSDU4_CFG0_PL_CNT_MAX_SHFT               10
#define WED_PAO_AMSDU4_CFG0_TXP_NUM_MAX_OFFSET            WED_PAO_AMSDU4_CFG0_OFFSET
#define WED_PAO_AMSDU4_CFG0_TXP_NUM_MAX_MASK              0x000003E0                // TXP_NUM_MAX[9..5]
#define WED_PAO_AMSDU4_CFG0_TXP_NUM_MAX_SHFT              5
#define WED_PAO_AMSDU4_CFG0_FAST_DEQ_OPT_OFFSET           WED_PAO_AMSDU4_CFG0_OFFSET
#define WED_PAO_AMSDU4_CFG0_FAST_DEQ_OPT_MASK             0x0000001F                // FAST_DEQ_OPT[4..0]
#define WED_PAO_AMSDU4_CFG0_FAST_DEQ_OPT_SHFT             0

/* =====================================================================================

  ---WED_PAO_AMSDU4_CFG1 (0x15015800 + 0x084)---

    WAIT_DMAD_CYCLE[13..0]       - (RW) Set AMSDU_ENG wait new MSDU time
    WAIT_CYCLE[27..14]           - (RW) Set per MSDU live time in QMEM (wait for merge)
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_AMSDU4_CFG1_WAIT_CYCLE_OFFSET             WED_PAO_AMSDU4_CFG1_OFFSET
#define WED_PAO_AMSDU4_CFG1_WAIT_CYCLE_MASK               0x0FFFC000                // WAIT_CYCLE[27..14]
#define WED_PAO_AMSDU4_CFG1_WAIT_CYCLE_SHFT               14
#define WED_PAO_AMSDU4_CFG1_WAIT_DMAD_CYCLE_OFFSET        WED_PAO_AMSDU4_CFG1_OFFSET
#define WED_PAO_AMSDU4_CFG1_WAIT_DMAD_CYCLE_MASK          0x00003FFF                // WAIT_DMAD_CYCLE[13..0]
#define WED_PAO_AMSDU4_CFG1_WAIT_DMAD_CYCLE_SHFT          0

/* =====================================================================================

  ---WED_PAO_AMSDU5_CFG0 (0x15015800 + 0x090)---

    FAST_DEQ_OPT[4..0]           - (RW) AMSDU_ENG Control Setting : 
                                     Current queue is ready for de-queue option
    TXP_NUM_MAX[9..5]            - (RW) Set AMSDU_ENG Max merge TXP count
    PL_CNT_MAX[20..10]           - (RW) Set AMSDU_ENG queue entry usage
    SEARCH_MAX[31..21]           - (RW) Set AMSDU_ENG search times for one MSDU

 =====================================================================================*/
#define WED_PAO_AMSDU5_CFG0_SEARCH_MAX_OFFSET             WED_PAO_AMSDU5_CFG0_OFFSET
#define WED_PAO_AMSDU5_CFG0_SEARCH_MAX_MASK               0xFFE00000                // SEARCH_MAX[31..21]
#define WED_PAO_AMSDU5_CFG0_SEARCH_MAX_SHFT               21
#define WED_PAO_AMSDU5_CFG0_PL_CNT_MAX_OFFSET             WED_PAO_AMSDU5_CFG0_OFFSET
#define WED_PAO_AMSDU5_CFG0_PL_CNT_MAX_MASK               0x001FFC00                // PL_CNT_MAX[20..10]
#define WED_PAO_AMSDU5_CFG0_PL_CNT_MAX_SHFT               10
#define WED_PAO_AMSDU5_CFG0_TXP_NUM_MAX_OFFSET            WED_PAO_AMSDU5_CFG0_OFFSET
#define WED_PAO_AMSDU5_CFG0_TXP_NUM_MAX_MASK              0x000003E0                // TXP_NUM_MAX[9..5]
#define WED_PAO_AMSDU5_CFG0_TXP_NUM_MAX_SHFT              5
#define WED_PAO_AMSDU5_CFG0_FAST_DEQ_OPT_OFFSET           WED_PAO_AMSDU5_CFG0_OFFSET
#define WED_PAO_AMSDU5_CFG0_FAST_DEQ_OPT_MASK             0x0000001F                // FAST_DEQ_OPT[4..0]
#define WED_PAO_AMSDU5_CFG0_FAST_DEQ_OPT_SHFT             0

/* =====================================================================================

  ---WED_PAO_AMSDU5_CFG1 (0x15015800 + 0x094)---

    WAIT_DMAD_CYCLE[13..0]       - (RW) Set AMSDU_ENG wait new MSDU time
    WAIT_CYCLE[27..14]           - (RW) Set per MSDU live time in QMEM (wait for merge)
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_AMSDU5_CFG1_WAIT_CYCLE_OFFSET             WED_PAO_AMSDU5_CFG1_OFFSET
#define WED_PAO_AMSDU5_CFG1_WAIT_CYCLE_MASK               0x0FFFC000                // WAIT_CYCLE[27..14]
#define WED_PAO_AMSDU5_CFG1_WAIT_CYCLE_SHFT               14
#define WED_PAO_AMSDU5_CFG1_WAIT_DMAD_CYCLE_OFFSET        WED_PAO_AMSDU5_CFG1_OFFSET
#define WED_PAO_AMSDU5_CFG1_WAIT_DMAD_CYCLE_MASK          0x00003FFF                // WAIT_DMAD_CYCLE[13..0]
#define WED_PAO_AMSDU5_CFG1_WAIT_DMAD_CYCLE_SHFT          0

/* =====================================================================================

  ---WED_PAO_AMSDU6_CFG0 (0x15015800 + 0x0A0)---

    FAST_DEQ_OPT[4..0]           - (RW) AMSDU_ENG Control Setting : 
                                     Current queue is ready for de-queue option
    TXP_NUM_MAX[9..5]            - (RW) Set AMSDU_ENG Max merge TXP count
    PL_CNT_MAX[20..10]           - (RW) Set AMSDU_ENG queue entry usage
    SEARCH_MAX[31..21]           - (RW) Set AMSDU_ENG search times for one MSDU

 =====================================================================================*/
#define WED_PAO_AMSDU6_CFG0_SEARCH_MAX_OFFSET             WED_PAO_AMSDU6_CFG0_OFFSET
#define WED_PAO_AMSDU6_CFG0_SEARCH_MAX_MASK               0xFFE00000                // SEARCH_MAX[31..21]
#define WED_PAO_AMSDU6_CFG0_SEARCH_MAX_SHFT               21
#define WED_PAO_AMSDU6_CFG0_PL_CNT_MAX_OFFSET             WED_PAO_AMSDU6_CFG0_OFFSET
#define WED_PAO_AMSDU6_CFG0_PL_CNT_MAX_MASK               0x001FFC00                // PL_CNT_MAX[20..10]
#define WED_PAO_AMSDU6_CFG0_PL_CNT_MAX_SHFT               10
#define WED_PAO_AMSDU6_CFG0_TXP_NUM_MAX_OFFSET            WED_PAO_AMSDU6_CFG0_OFFSET
#define WED_PAO_AMSDU6_CFG0_TXP_NUM_MAX_MASK              0x000003E0                // TXP_NUM_MAX[9..5]
#define WED_PAO_AMSDU6_CFG0_TXP_NUM_MAX_SHFT              5
#define WED_PAO_AMSDU6_CFG0_FAST_DEQ_OPT_OFFSET           WED_PAO_AMSDU6_CFG0_OFFSET
#define WED_PAO_AMSDU6_CFG0_FAST_DEQ_OPT_MASK             0x0000001F                // FAST_DEQ_OPT[4..0]
#define WED_PAO_AMSDU6_CFG0_FAST_DEQ_OPT_SHFT             0

/* =====================================================================================

  ---WED_PAO_AMSDU6_CFG1 (0x15015800 + 0x0A4)---

    WAIT_DMAD_CYCLE[13..0]       - (RW) Set AMSDU_ENG wait new MSDU time
    WAIT_CYCLE[27..14]           - (RW) Set per MSDU live time in QMEM (wait for merge)
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_AMSDU6_CFG1_WAIT_CYCLE_OFFSET             WED_PAO_AMSDU6_CFG1_OFFSET
#define WED_PAO_AMSDU6_CFG1_WAIT_CYCLE_MASK               0x0FFFC000                // WAIT_CYCLE[27..14]
#define WED_PAO_AMSDU6_CFG1_WAIT_CYCLE_SHFT               14
#define WED_PAO_AMSDU6_CFG1_WAIT_DMAD_CYCLE_OFFSET        WED_PAO_AMSDU6_CFG1_OFFSET
#define WED_PAO_AMSDU6_CFG1_WAIT_DMAD_CYCLE_MASK          0x00003FFF                // WAIT_DMAD_CYCLE[13..0]
#define WED_PAO_AMSDU6_CFG1_WAIT_DMAD_CYCLE_SHFT          0

/* =====================================================================================

  ---WED_PAO_AMSDU7_CFG0 (0x15015800 + 0x0B0)---

    FAST_DEQ_OPT[4..0]           - (RW) AMSDU_ENG Control Setting : 
                                     Current queue is ready for de-queue option
    TXP_NUM_MAX[9..5]            - (RW) Set AMSDU_ENG Max merge TXP count
    PL_CNT_MAX[20..10]           - (RW) Set AMSDU_ENG queue entry usage
    SEARCH_MAX[31..21]           - (RW) Set AMSDU_ENG search times for one MSDU

 =====================================================================================*/
#define WED_PAO_AMSDU7_CFG0_SEARCH_MAX_OFFSET             WED_PAO_AMSDU7_CFG0_OFFSET
#define WED_PAO_AMSDU7_CFG0_SEARCH_MAX_MASK               0xFFE00000                // SEARCH_MAX[31..21]
#define WED_PAO_AMSDU7_CFG0_SEARCH_MAX_SHFT               21
#define WED_PAO_AMSDU7_CFG0_PL_CNT_MAX_OFFSET             WED_PAO_AMSDU7_CFG0_OFFSET
#define WED_PAO_AMSDU7_CFG0_PL_CNT_MAX_MASK               0x001FFC00                // PL_CNT_MAX[20..10]
#define WED_PAO_AMSDU7_CFG0_PL_CNT_MAX_SHFT               10
#define WED_PAO_AMSDU7_CFG0_TXP_NUM_MAX_OFFSET            WED_PAO_AMSDU7_CFG0_OFFSET
#define WED_PAO_AMSDU7_CFG0_TXP_NUM_MAX_MASK              0x000003E0                // TXP_NUM_MAX[9..5]
#define WED_PAO_AMSDU7_CFG0_TXP_NUM_MAX_SHFT              5
#define WED_PAO_AMSDU7_CFG0_FAST_DEQ_OPT_OFFSET           WED_PAO_AMSDU7_CFG0_OFFSET
#define WED_PAO_AMSDU7_CFG0_FAST_DEQ_OPT_MASK             0x0000001F                // FAST_DEQ_OPT[4..0]
#define WED_PAO_AMSDU7_CFG0_FAST_DEQ_OPT_SHFT             0

/* =====================================================================================

  ---WED_PAO_AMSDU7_CFG1 (0x15015800 + 0x0B4)---

    WAIT_DMAD_CYCLE[13..0]       - (RW) Set AMSDU_ENG wait new MSDU time
    WAIT_CYCLE[27..14]           - (RW) Set per MSDU live time in QMEM (wait for merge)
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_AMSDU7_CFG1_WAIT_CYCLE_OFFSET             WED_PAO_AMSDU7_CFG1_OFFSET
#define WED_PAO_AMSDU7_CFG1_WAIT_CYCLE_MASK               0x0FFFC000                // WAIT_CYCLE[27..14]
#define WED_PAO_AMSDU7_CFG1_WAIT_CYCLE_SHFT               14
#define WED_PAO_AMSDU7_CFG1_WAIT_DMAD_CYCLE_OFFSET        WED_PAO_AMSDU7_CFG1_OFFSET
#define WED_PAO_AMSDU7_CFG1_WAIT_DMAD_CYCLE_MASK          0x00003FFF                // WAIT_DMAD_CYCLE[13..0]
#define WED_PAO_AMSDU7_CFG1_WAIT_DMAD_CYCLE_SHFT          0

/* =====================================================================================

  ---WED_PAO_AMSDU8_CFG0 (0x15015800 + 0x0C0)---

    FAST_DEQ_OPT[4..0]           - (RW) AMSDU_ENG Control Setting : 
                                     Current queue is ready for de-queue option
    TXP_NUM_MAX[9..5]            - (RW) Set AMSDU_ENG Max merge TXP count
    PL_CNT_MAX[20..10]           - (RW) Set AMSDU_ENG queue entry usage
    SEARCH_MAX[31..21]           - (RW) Set AMSDU_ENG search times for one MSDU

 =====================================================================================*/
#define WED_PAO_AMSDU8_CFG0_SEARCH_MAX_OFFSET             WED_PAO_AMSDU8_CFG0_OFFSET
#define WED_PAO_AMSDU8_CFG0_SEARCH_MAX_MASK               0xFFE00000                // SEARCH_MAX[31..21]
#define WED_PAO_AMSDU8_CFG0_SEARCH_MAX_SHFT               21
#define WED_PAO_AMSDU8_CFG0_PL_CNT_MAX_OFFSET             WED_PAO_AMSDU8_CFG0_OFFSET
#define WED_PAO_AMSDU8_CFG0_PL_CNT_MAX_MASK               0x001FFC00                // PL_CNT_MAX[20..10]
#define WED_PAO_AMSDU8_CFG0_PL_CNT_MAX_SHFT               10
#define WED_PAO_AMSDU8_CFG0_TXP_NUM_MAX_OFFSET            WED_PAO_AMSDU8_CFG0_OFFSET
#define WED_PAO_AMSDU8_CFG0_TXP_NUM_MAX_MASK              0x000003E0                // TXP_NUM_MAX[9..5]
#define WED_PAO_AMSDU8_CFG0_TXP_NUM_MAX_SHFT              5
#define WED_PAO_AMSDU8_CFG0_FAST_DEQ_OPT_OFFSET           WED_PAO_AMSDU8_CFG0_OFFSET
#define WED_PAO_AMSDU8_CFG0_FAST_DEQ_OPT_MASK             0x0000001F                // FAST_DEQ_OPT[4..0]
#define WED_PAO_AMSDU8_CFG0_FAST_DEQ_OPT_SHFT             0

/* =====================================================================================

  ---WED_PAO_AMSDU8_CFG1 (0x15015800 + 0x0C4)---

    WAIT_DMAD_CYCLE[13..0]       - (RW) Set AMSDU_ENG wait new MSDU time
    WAIT_CYCLE[27..14]           - (RW) Set per MSDU live time in QMEM (wait for merge)
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_AMSDU8_CFG1_WAIT_CYCLE_OFFSET             WED_PAO_AMSDU8_CFG1_OFFSET
#define WED_PAO_AMSDU8_CFG1_WAIT_CYCLE_MASK               0x0FFFC000                // WAIT_CYCLE[27..14]
#define WED_PAO_AMSDU8_CFG1_WAIT_CYCLE_SHFT               14
#define WED_PAO_AMSDU8_CFG1_WAIT_DMAD_CYCLE_OFFSET        WED_PAO_AMSDU8_CFG1_OFFSET
#define WED_PAO_AMSDU8_CFG1_WAIT_DMAD_CYCLE_MASK          0x00003FFF                // WAIT_DMAD_CYCLE[13..0]
#define WED_PAO_AMSDU8_CFG1_WAIT_DMAD_CYCLE_SHFT          0

/* =====================================================================================

  ---WED_PAO_SCHED (0x15015800 + 0x0D0)---

    SP_EN[8..0]                  - (RW)  xxx 
    RESERVED9[15..9]             - (RO) Reserved bits
    THRESHOLD[27..16]            - (RW)  xxx 
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_SCHED_THRESHOLD_OFFSET                    WED_PAO_SCHED_OFFSET
#define WED_PAO_SCHED_THRESHOLD_MASK                      0x0FFF0000                // THRESHOLD[27..16]
#define WED_PAO_SCHED_THRESHOLD_SHFT                      16
#define WED_PAO_SCHED_SP_EN_OFFSET                        WED_PAO_SCHED_OFFSET
#define WED_PAO_SCHED_SP_EN_MASK                          0x000001FF                // SP_EN[8..0]
#define WED_PAO_SCHED_SP_EN_SHFT                          0

/* =====================================================================================

  ---WED_PAO_SCHED_CFG0 (0x15015800 + 0x0D4)---

    Q3_WEIGHT[6..0]              - (RW) Set max De-queue count of one queue
    Q2_WEIGHT[13..7]             - (RW) Set max De-queue count of one queue
    Q1_WEIGHT[20..14]            - (RW) Set max De-queue count of one queue
    Q0_WEIGHT[27..21]            - (RW) Set max De-queue count of one queue
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_SCHED_CFG0_Q0_WEIGHT_OFFSET               WED_PAO_SCHED_CFG0_OFFSET
#define WED_PAO_SCHED_CFG0_Q0_WEIGHT_MASK                 0x0FE00000                // Q0_WEIGHT[27..21]
#define WED_PAO_SCHED_CFG0_Q0_WEIGHT_SHFT                 21
#define WED_PAO_SCHED_CFG0_Q1_WEIGHT_OFFSET               WED_PAO_SCHED_CFG0_OFFSET
#define WED_PAO_SCHED_CFG0_Q1_WEIGHT_MASK                 0x001FC000                // Q1_WEIGHT[20..14]
#define WED_PAO_SCHED_CFG0_Q1_WEIGHT_SHFT                 14
#define WED_PAO_SCHED_CFG0_Q2_WEIGHT_OFFSET               WED_PAO_SCHED_CFG0_OFFSET
#define WED_PAO_SCHED_CFG0_Q2_WEIGHT_MASK                 0x00003F80                // Q2_WEIGHT[13..7]
#define WED_PAO_SCHED_CFG0_Q2_WEIGHT_SHFT                 7
#define WED_PAO_SCHED_CFG0_Q3_WEIGHT_OFFSET               WED_PAO_SCHED_CFG0_OFFSET
#define WED_PAO_SCHED_CFG0_Q3_WEIGHT_MASK                 0x0000007F                // Q3_WEIGHT[6..0]
#define WED_PAO_SCHED_CFG0_Q3_WEIGHT_SHFT                 0

/* =====================================================================================

  ---WED_PAO_SCHED_CFG1 (0x15015800 + 0x0D8)---

    Q7_WEIGHT[6..0]              - (RW) Set max De-queue count of one queue
    Q6_WEIGHT[13..7]             - (RW) Set max De-queue count of one queue
    Q5_WEIGHT[20..14]            - (RW) Set max De-queue count of one queue
    Q4_WEIGHT[27..21]            - (RW) Set max De-queue count of one queue
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_SCHED_CFG1_Q4_WEIGHT_OFFSET               WED_PAO_SCHED_CFG1_OFFSET
#define WED_PAO_SCHED_CFG1_Q4_WEIGHT_MASK                 0x0FE00000                // Q4_WEIGHT[27..21]
#define WED_PAO_SCHED_CFG1_Q4_WEIGHT_SHFT                 21
#define WED_PAO_SCHED_CFG1_Q5_WEIGHT_OFFSET               WED_PAO_SCHED_CFG1_OFFSET
#define WED_PAO_SCHED_CFG1_Q5_WEIGHT_MASK                 0x001FC000                // Q5_WEIGHT[20..14]
#define WED_PAO_SCHED_CFG1_Q5_WEIGHT_SHFT                 14
#define WED_PAO_SCHED_CFG1_Q6_WEIGHT_OFFSET               WED_PAO_SCHED_CFG1_OFFSET
#define WED_PAO_SCHED_CFG1_Q6_WEIGHT_MASK                 0x00003F80                // Q6_WEIGHT[13..7]
#define WED_PAO_SCHED_CFG1_Q6_WEIGHT_SHFT                 7
#define WED_PAO_SCHED_CFG1_Q7_WEIGHT_OFFSET               WED_PAO_SCHED_CFG1_OFFSET
#define WED_PAO_SCHED_CFG1_Q7_WEIGHT_MASK                 0x0000007F                // Q7_WEIGHT[6..0]
#define WED_PAO_SCHED_CFG1_Q7_WEIGHT_SHFT                 0

/* =====================================================================================

  ---WED_PAO_SCHED_CFG2 (0x15015800 + 0x0DC)---

    Q8_WEIGHT[6..0]              - (RW) Set max De-queue count of one queue
    RESERVED7[31..7]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_SCHED_CFG2_Q8_WEIGHT_OFFSET               WED_PAO_SCHED_CFG2_OFFSET
#define WED_PAO_SCHED_CFG2_Q8_WEIGHT_MASK                 0x0000007F                // Q8_WEIGHT[6..0]
#define WED_PAO_SCHED_CFG2_Q8_WEIGHT_SHFT                 0

/* =====================================================================================

  ---WED_PAO_SCHED_TIMER (0x15015800 + 0X0E0)---

    CYCLE[15..0]                 - (RW) Set timer max cycle
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_SCHED_TIMER_CYCLE_OFFSET                  WED_PAO_SCHED_TIMER_OFFSET
#define WED_PAO_SCHED_TIMER_CYCLE_MASK                    0x0000FFFF                // CYCLE[15..0]
#define WED_PAO_SCHED_TIMER_CYCLE_SHFT                    0

/* =====================================================================================

  ---WED_PAO_FQ_MAX_PCNT (0x15015800 + 0x100)---

    VAL[10..0]                   - (RW) Set Free queue max entry count
    RESERVED11[31..11]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_FQ_MAX_PCNT_VAL_OFFSET                    WED_PAO_FQ_MAX_PCNT_OFFSET
#define WED_PAO_FQ_MAX_PCNT_VAL_MASK                      0x000007FF                // VAL[10..0]
#define WED_PAO_FQ_MAX_PCNT_VAL_SHFT                      0

/* =====================================================================================

  ---WED_PAO_FQ_DROP (0x15015800 + 0x104)---

    VAL[10..0]                   - (RW)  xxx 
    RESERVED11[31..11]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_FQ_DROP_VAL_OFFSET                        WED_PAO_FQ_DROP_OFFSET
#define WED_PAO_FQ_DROP_VAL_MASK                          0x000007FF                // VAL[10..0]
#define WED_PAO_FQ_DROP_VAL_SHFT                          0

/* =====================================================================================

  ---WED_PAO_FQ_LOW (0x15015800 + 0x108)---

    VAL[10..0]                   - (RW) Set Free queue low level
    RESERVED11[31..11]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_FQ_LOW_VAL_OFFSET                         WED_PAO_FQ_LOW_OFFSET
#define WED_PAO_FQ_LOW_VAL_MASK                           0x000007FF                // VAL[10..0]
#define WED_PAO_FQ_LOW_VAL_SHFT                           0

/* =====================================================================================

  ---WED_PAO_FQ_HIGH (0x15015800 + 0x10C)---

    VAL[10..0]                   - (RW) Set Free queue high level
    RESERVED11[31..11]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_FQ_HIGH_VAL_OFFSET                        WED_PAO_FQ_HIGH_OFFSET
#define WED_PAO_FQ_HIGH_VAL_MASK                          0x000007FF                // VAL[10..0]
#define WED_PAO_FQ_HIGH_VAL_SHFT                          0

/* =====================================================================================

  ---WED_PAO_PSE (0x15015800 + 0x110)---

    SP_ARB_EN[0]                 - (RW)  xxx 
    TDM_ARB_EN[1]                - (RW)  xxx 
    TDM_TBL_NUM[8..2]            - (RW) Set QMEM PKTMEM arbitration
    RESERVED9[15..9]             - (RO) Reserved bits
    RESET[16]                    - (A0) PAO QMEM SW reset
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_PSE_RESET_OFFSET                          WED_PAO_PSE_OFFSET
#define WED_PAO_PSE_RESET_MASK                            0x00010000                // RESET[16]
#define WED_PAO_PSE_RESET_SHFT                            16
#define WED_PAO_PSE_TDM_TBL_NUM_OFFSET                    WED_PAO_PSE_OFFSET
#define WED_PAO_PSE_TDM_TBL_NUM_MASK                      0x000001FC                // TDM_TBL_NUM[8..2]
#define WED_PAO_PSE_TDM_TBL_NUM_SHFT                      2
#define WED_PAO_PSE_TDM_ARB_EN_OFFSET                     WED_PAO_PSE_OFFSET
#define WED_PAO_PSE_TDM_ARB_EN_MASK                       0x00000002                // TDM_ARB_EN[1]
#define WED_PAO_PSE_TDM_ARB_EN_SHFT                       1
#define WED_PAO_PSE_SP_ARB_EN_OFFSET                      WED_PAO_PSE_OFFSET
#define WED_PAO_PSE_SP_ARB_EN_MASK                        0x00000001                // SP_ARB_EN[0]
#define WED_PAO_PSE_SP_ARB_EN_SHFT                        0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL0 (0x15015800 + 0x114)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL0_VAL_OFFSET                   WED_PAO_PSE_TDM_TBL0_OFFSET
#define WED_PAO_PSE_TDM_TBL0_VAL_MASK                     0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL0_VAL_SHFT                     0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL1 (0x15015800 + 0x118)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL1_VAL_OFFSET                   WED_PAO_PSE_TDM_TBL1_OFFSET
#define WED_PAO_PSE_TDM_TBL1_VAL_MASK                     0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL1_VAL_SHFT                     0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL2 (0x15015800 + 0x11C)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL2_VAL_OFFSET                   WED_PAO_PSE_TDM_TBL2_OFFSET
#define WED_PAO_PSE_TDM_TBL2_VAL_MASK                     0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL2_VAL_SHFT                     0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL3 (0x15015800 + 0x120)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL3_VAL_OFFSET                   WED_PAO_PSE_TDM_TBL3_OFFSET
#define WED_PAO_PSE_TDM_TBL3_VAL_MASK                     0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL3_VAL_SHFT                     0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL4 (0x15015800 + 0x124)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL4_VAL_OFFSET                   WED_PAO_PSE_TDM_TBL4_OFFSET
#define WED_PAO_PSE_TDM_TBL4_VAL_MASK                     0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL4_VAL_SHFT                     0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL5 (0x15015800 + 0x128)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL5_VAL_OFFSET                   WED_PAO_PSE_TDM_TBL5_OFFSET
#define WED_PAO_PSE_TDM_TBL5_VAL_MASK                     0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL5_VAL_SHFT                     0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL6 (0x15015800 + 0x12C)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL6_VAL_OFFSET                   WED_PAO_PSE_TDM_TBL6_OFFSET
#define WED_PAO_PSE_TDM_TBL6_VAL_MASK                     0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL6_VAL_SHFT                     0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL7 (0x15015800 + 0x130)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL7_VAL_OFFSET                   WED_PAO_PSE_TDM_TBL7_OFFSET
#define WED_PAO_PSE_TDM_TBL7_VAL_MASK                     0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL7_VAL_SHFT                     0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL8 (0x15015800 + 0x134)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL8_VAL_OFFSET                   WED_PAO_PSE_TDM_TBL8_OFFSET
#define WED_PAO_PSE_TDM_TBL8_VAL_MASK                     0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL8_VAL_SHFT                     0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL9 (0x15015800 + 0x138)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL9_VAL_OFFSET                   WED_PAO_PSE_TDM_TBL9_OFFSET
#define WED_PAO_PSE_TDM_TBL9_VAL_MASK                     0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL9_VAL_SHFT                     0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL10 (0x15015800 + 0x13C)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL10_VAL_OFFSET                  WED_PAO_PSE_TDM_TBL10_OFFSET
#define WED_PAO_PSE_TDM_TBL10_VAL_MASK                    0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL10_VAL_SHFT                    0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL11 (0x15015800 + 0x140)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL11_VAL_OFFSET                  WED_PAO_PSE_TDM_TBL11_OFFSET
#define WED_PAO_PSE_TDM_TBL11_VAL_MASK                    0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL11_VAL_SHFT                    0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL12 (0x15015800 + 0x144)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL12_VAL_OFFSET                  WED_PAO_PSE_TDM_TBL12_OFFSET
#define WED_PAO_PSE_TDM_TBL12_VAL_MASK                    0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL12_VAL_SHFT                    0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL13 (0x15015800 + 0x148)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL13_VAL_OFFSET                  WED_PAO_PSE_TDM_TBL13_OFFSET
#define WED_PAO_PSE_TDM_TBL13_VAL_MASK                    0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL13_VAL_SHFT                    0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL14 (0x15015800 + 0x14C)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL14_VAL_OFFSET                  WED_PAO_PSE_TDM_TBL14_OFFSET
#define WED_PAO_PSE_TDM_TBL14_VAL_MASK                    0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL14_VAL_SHFT                    0

/* =====================================================================================

  ---WED_PAO_PSE_TDM_TBL15 (0x15015800 + 0x150)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_TDM_TBL15_VAL_OFFSET                  WED_PAO_PSE_TDM_TBL15_OFFSET
#define WED_PAO_PSE_TDM_TBL15_VAL_MASK                    0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_TDM_TBL15_VAL_SHFT                    0

/* =====================================================================================

  ---WED_PAO_PSE_SP_CFG0 (0x15015800 + 0x154)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_SP_CFG0_VAL_OFFSET                    WED_PAO_PSE_SP_CFG0_OFFSET
#define WED_PAO_PSE_SP_CFG0_VAL_MASK                      0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_SP_CFG0_VAL_SHFT                      0

/* =====================================================================================

  ---WED_PAO_PSE_SP_CFG1 (0x15015800 + 0x158)---

    VAL[31..0]                   - (RW) Set QMEM PKTMEM arbitration

 =====================================================================================*/
#define WED_PAO_PSE_SP_CFG1_VAL_OFFSET                    WED_PAO_PSE_SP_CFG1_OFFSET
#define WED_PAO_PSE_SP_CFG1_VAL_MASK                      0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PSE_SP_CFG1_VAL_SHFT                      0

/* =====================================================================================

  ---WED_PAO_WPDMAD_CFG0 (0x15015800 + 0x160)---

    SDP1_ZERO[0]                 - (RW) WPDMA_TXDMAD.SDP1 force zero
    SDP0_ZERO[1]                 - (RW) WPDMA_TXDMAD.SDP0 force zero
    BURST[2]                     - (RW) WPDMA_TXDMAD.BURST value config
    LS1[3]                       - (RW) WPDMA_TXDMAD.LS1 value config
    LS0[4]                       - (RW) WPDMA_TXDMAD.LS0 value config
    RESERVED5[31..5]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_WPDMAD_CFG0_LS0_OFFSET                    WED_PAO_WPDMAD_CFG0_OFFSET
#define WED_PAO_WPDMAD_CFG0_LS0_MASK                      0x00000010                // LS0[4]
#define WED_PAO_WPDMAD_CFG0_LS0_SHFT                      4
#define WED_PAO_WPDMAD_CFG0_LS1_OFFSET                    WED_PAO_WPDMAD_CFG0_OFFSET
#define WED_PAO_WPDMAD_CFG0_LS1_MASK                      0x00000008                // LS1[3]
#define WED_PAO_WPDMAD_CFG0_LS1_SHFT                      3
#define WED_PAO_WPDMAD_CFG0_BURST_OFFSET                  WED_PAO_WPDMAD_CFG0_OFFSET
#define WED_PAO_WPDMAD_CFG0_BURST_MASK                    0x00000004                // BURST[2]
#define WED_PAO_WPDMAD_CFG0_BURST_SHFT                    2
#define WED_PAO_WPDMAD_CFG0_SDP0_ZERO_OFFSET              WED_PAO_WPDMAD_CFG0_OFFSET
#define WED_PAO_WPDMAD_CFG0_SDP0_ZERO_MASK                0x00000002                // SDP0_ZERO[1]
#define WED_PAO_WPDMAD_CFG0_SDP0_ZERO_SHFT                1
#define WED_PAO_WPDMAD_CFG0_SDP1_ZERO_OFFSET              WED_PAO_WPDMAD_CFG0_OFFSET
#define WED_PAO_WPDMAD_CFG0_SDP1_ZERO_MASK                0x00000001                // SDP1_ZERO[0]
#define WED_PAO_WPDMAD_CFG0_SDP1_ZERO_SHFT                0

/* =====================================================================================

  ---WED_PAO_WPDMAD_CFG1 (0x15015800 + 0x164)---

    SDL1[13..0]                  - (RW) WPDMA_TXDMAD.SDL1 valud config.
    SDL0[27..14]                 - (RW) WPDMA_TXDMAD.SDL0 value config. Default=128 (HIF_TXD_V2 length)
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_WPDMAD_CFG1_SDL0_OFFSET                   WED_PAO_WPDMAD_CFG1_OFFSET
#define WED_PAO_WPDMAD_CFG1_SDL0_MASK                     0x0FFFC000                // SDL0[27..14]
#define WED_PAO_WPDMAD_CFG1_SDL0_SHFT                     14
#define WED_PAO_WPDMAD_CFG1_SDL1_OFFSET                   WED_PAO_WPDMAD_CFG1_OFFSET
#define WED_PAO_WPDMAD_CFG1_SDL1_MASK                     0x00003FFF                // SDL1[13..0]
#define WED_PAO_WPDMAD_CFG1_SDL1_SHFT                     0

/* =====================================================================================

  ---WED_PAO_HIFTXD (0x15015800 + 0x168)---

    SIZE[3..0]                   - (RW) HIFTXD size, for calculate HIFTXD physical address pointer
    RESERVED4[9..4]              - (RO) Reserved bits
    IS_NON_CIPHER[10]            - (RW) HIFTXD.IS_NON_CIPHER value config.
    BC_MC_FLAG[12..11]           - (RW) HIFTXD.BC_MC_FLAG value config.
    IS_FR_HOST[13]               - (RW) HIFTXD.IS_FR_HOST value config.
    IS_MCAST_CLONE[14]           - (RW) HIFTXD.IS_MCAST_CLONE value config.
    SRC[16..15]                  - (RW) HIFTXD.SRC value config.
    HIF_VERSION[20..17]          - (RW) HIFTXD.HIF_VERSION value config.
    ETHER_TYPE_BSWAP[21]         - (RW) HIFTXD.ETHER_TYPE byte swap
    RESERVED22[31..22]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_HIFTXD_ETHER_TYPE_BSWAP_OFFSET            WED_PAO_HIFTXD_OFFSET
#define WED_PAO_HIFTXD_ETHER_TYPE_BSWAP_MASK              0x00200000                // ETHER_TYPE_BSWAP[21]
#define WED_PAO_HIFTXD_ETHER_TYPE_BSWAP_SHFT              21
#define WED_PAO_HIFTXD_HIF_VERSION_OFFSET                 WED_PAO_HIFTXD_OFFSET
#define WED_PAO_HIFTXD_HIF_VERSION_MASK                   0x001E0000                // HIF_VERSION[20..17]
#define WED_PAO_HIFTXD_HIF_VERSION_SHFT                   17
#define WED_PAO_HIFTXD_SRC_OFFSET                         WED_PAO_HIFTXD_OFFSET
#define WED_PAO_HIFTXD_SRC_MASK                           0x00018000                // SRC[16..15]
#define WED_PAO_HIFTXD_SRC_SHFT                           15
#define WED_PAO_HIFTXD_IS_MCAST_CLONE_OFFSET              WED_PAO_HIFTXD_OFFSET
#define WED_PAO_HIFTXD_IS_MCAST_CLONE_MASK                0x00004000                // IS_MCAST_CLONE[14]
#define WED_PAO_HIFTXD_IS_MCAST_CLONE_SHFT                14
#define WED_PAO_HIFTXD_IS_FR_HOST_OFFSET                  WED_PAO_HIFTXD_OFFSET
#define WED_PAO_HIFTXD_IS_FR_HOST_MASK                    0x00002000                // IS_FR_HOST[13]
#define WED_PAO_HIFTXD_IS_FR_HOST_SHFT                    13
#define WED_PAO_HIFTXD_BC_MC_FLAG_OFFSET                  WED_PAO_HIFTXD_OFFSET
#define WED_PAO_HIFTXD_BC_MC_FLAG_MASK                    0x00001800                // BC_MC_FLAG[12..11]
#define WED_PAO_HIFTXD_BC_MC_FLAG_SHFT                    11
#define WED_PAO_HIFTXD_IS_NON_CIPHER_OFFSET               WED_PAO_HIFTXD_OFFSET
#define WED_PAO_HIFTXD_IS_NON_CIPHER_MASK                 0x00000400                // IS_NON_CIPHER[10]
#define WED_PAO_HIFTXD_IS_NON_CIPHER_SHFT                 10
#define WED_PAO_HIFTXD_SIZE_OFFSET                        WED_PAO_HIFTXD_OFFSET
#define WED_PAO_HIFTXD_SIZE_MASK                          0x0000000F                // SIZE[3..0]
#define WED_PAO_HIFTXD_SIZE_SHFT                          0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE00_L (0x15015800 + 0x180)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 00 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE00_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE00_L_OFFSET
#define WED_PAO_HIFTXD_BASE00_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE00_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE01_L (0x15015800 + 0x184)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 01 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE01_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE01_L_OFFSET
#define WED_PAO_HIFTXD_BASE01_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE01_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE02_L (0x15015800 + 0x188)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 02 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE02_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE02_L_OFFSET
#define WED_PAO_HIFTXD_BASE02_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE02_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE03_L (0x15015800 + 0x18C)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 03 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE03_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE03_L_OFFSET
#define WED_PAO_HIFTXD_BASE03_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE03_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE04_L (0x15015800 + 0x190)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 04 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE04_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE04_L_OFFSET
#define WED_PAO_HIFTXD_BASE04_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE04_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE05_L (0x15015800 + 0x194)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 05 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE05_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE05_L_OFFSET
#define WED_PAO_HIFTXD_BASE05_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE05_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE06_L (0x15015800 + 0x198)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 06 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE06_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE06_L_OFFSET
#define WED_PAO_HIFTXD_BASE06_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE06_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE07_L (0x15015800 + 0x19C)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 07 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE07_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE07_L_OFFSET
#define WED_PAO_HIFTXD_BASE07_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE07_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE08_L (0x15015800 + 0x1A0)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 08 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE08_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE08_L_OFFSET
#define WED_PAO_HIFTXD_BASE08_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE08_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE09_L (0x15015800 + 0x1A4)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 09 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE09_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE09_L_OFFSET
#define WED_PAO_HIFTXD_BASE09_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE09_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE10_L (0x15015800 + 0x1A8)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 10 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE10_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE10_L_OFFSET
#define WED_PAO_HIFTXD_BASE10_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE10_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE11_L (0x15015800 + 0x1AC)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 11 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE11_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE11_L_OFFSET
#define WED_PAO_HIFTXD_BASE11_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE11_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE12_L (0x15015800 + 0x1B0)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 12 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE12_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE12_L_OFFSET
#define WED_PAO_HIFTXD_BASE12_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE12_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE13_L (0x15015800 + 0x1B4)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 13 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE13_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE13_L_OFFSET
#define WED_PAO_HIFTXD_BASE13_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE13_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE14_L (0x15015800 + 0x1B8)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 14 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE14_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE14_L_OFFSET
#define WED_PAO_HIFTXD_BASE14_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE14_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE15_L (0x15015800 + 0x1BC)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 15 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE15_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE15_L_OFFSET
#define WED_PAO_HIFTXD_BASE15_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE15_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE16_L (0x15015800 + 0x1C0)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 16 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE16_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE16_L_OFFSET
#define WED_PAO_HIFTXD_BASE16_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE16_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE17_L (0x15015800 + 0x1C4)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 17 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE17_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE17_L_OFFSET
#define WED_PAO_HIFTXD_BASE17_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE17_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE18_L (0x15015800 + 0x1C8)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 18 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE18_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE18_L_OFFSET
#define WED_PAO_HIFTXD_BASE18_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE18_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE19_L (0x15015800 + 0x1CC)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 19 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE19_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE19_L_OFFSET
#define WED_PAO_HIFTXD_BASE19_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE19_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE20_L (0x15015800 + 0x1D0)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 20 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE20_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE20_L_OFFSET
#define WED_PAO_HIFTXD_BASE20_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE20_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE21_L (0x15015800 + 0x1D4)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 21 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE21_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE21_L_OFFSET
#define WED_PAO_HIFTXD_BASE21_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE21_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE22_L (0x15015800 + 0x1D8)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 22 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE22_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE22_L_OFFSET
#define WED_PAO_HIFTXD_BASE22_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE22_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE23_L (0x15015800 + 0x1DC)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 23 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE23_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE23_L_OFFSET
#define WED_PAO_HIFTXD_BASE23_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE23_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE24_L (0x15015800 + 0x1E0)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 24 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE24_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE24_L_OFFSET
#define WED_PAO_HIFTXD_BASE24_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE24_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE25_L (0x15015800 + 0x1E4)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 25 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE25_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE25_L_OFFSET
#define WED_PAO_HIFTXD_BASE25_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE25_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE26_L (0x15015800 + 0x1E8)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 26 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE26_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE26_L_OFFSET
#define WED_PAO_HIFTXD_BASE26_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE26_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE27_L (0x15015800 + 0x1EC)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 27 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE27_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE27_L_OFFSET
#define WED_PAO_HIFTXD_BASE27_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE27_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE28_L (0x15015800 + 0x1F0)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 28 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE28_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE28_L_OFFSET
#define WED_PAO_HIFTXD_BASE28_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE28_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE29_L (0x15015800 + 0x1F4)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 29 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE29_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE29_L_OFFSET
#define WED_PAO_HIFTXD_BASE29_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE29_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE30_L (0x15015800 + 0x1F8)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 30 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE30_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE30_L_OFFSET
#define WED_PAO_HIFTXD_BASE30_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE30_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE31_L (0x15015800 + 0x1FC)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 31 [31:0]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE31_L_PTR_OFFSET                WED_PAO_HIFTXD_BASE31_L_OFFSET
#define WED_PAO_HIFTXD_BASE31_L_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_PAO_HIFTXD_BASE31_L_PTR_SHFT                  0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE_00_07_H (0x15015800 + 0x200)---

    BASE07[3..0]                 - (RW) HIFTXD base pointer 07 [35:32]
    BASE06[7..4]                 - (RW) HIFTXD base pointer 06 [35:32]
    BASE05[11..8]                - (RW) HIFTXD base pointer 05 [35:32]
    BASE04[15..12]               - (RW) HIFTXD base pointer 04 [35:32]
    BASE03[19..16]               - (RW) HIFTXD base pointer 03 [35:32]
    BASE02[23..20]               - (RW) HIFTXD base pointer 02 [35:32]
    BASE01[27..24]               - (RW) HIFTXD base pointer 01 [35:32]
    BASE00[31..28]               - (RW) HIFTXD base pointer 00 [35:32]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE00_OFFSET         WED_PAO_HIFTXD_BASE_00_07_H_OFFSET
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE00_MASK           0xF0000000                // BASE00[31..28]
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE00_SHFT           28
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE01_OFFSET         WED_PAO_HIFTXD_BASE_00_07_H_OFFSET
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE01_MASK           0x0F000000                // BASE01[27..24]
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE01_SHFT           24
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE02_OFFSET         WED_PAO_HIFTXD_BASE_00_07_H_OFFSET
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE02_MASK           0x00F00000                // BASE02[23..20]
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE02_SHFT           20
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE03_OFFSET         WED_PAO_HIFTXD_BASE_00_07_H_OFFSET
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE03_MASK           0x000F0000                // BASE03[19..16]
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE03_SHFT           16
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE04_OFFSET         WED_PAO_HIFTXD_BASE_00_07_H_OFFSET
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE04_MASK           0x0000F000                // BASE04[15..12]
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE04_SHFT           12
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE05_OFFSET         WED_PAO_HIFTXD_BASE_00_07_H_OFFSET
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE05_MASK           0x00000F00                // BASE05[11..8]
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE05_SHFT           8
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE06_OFFSET         WED_PAO_HIFTXD_BASE_00_07_H_OFFSET
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE06_MASK           0x000000F0                // BASE06[7..4]
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE06_SHFT           4
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE07_OFFSET         WED_PAO_HIFTXD_BASE_00_07_H_OFFSET
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE07_MASK           0x0000000F                // BASE07[3..0]
#define WED_PAO_HIFTXD_BASE_00_07_H_BASE07_SHFT           0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE_08_15_H (0x15015800 + 0x204)---

    BASE15[3..0]                 - (RW) HIFTXD base pointer 15 [35:32]
    BASE14[7..4]                 - (RW) HIFTXD base pointer 14 [35:32]
    BASE13[11..8]                - (RW) HIFTXD base pointer 13 [35:32]
    BASE12[15..12]               - (RW) HIFTXD base pointer 12 [35:32]
    BASE11[19..16]               - (RW) HIFTXD base pointer 11 [35:32]
    BASE10[23..20]               - (RW) HIFTXD base pointer 10 [35:32]
    BASE09[27..24]               - (RW) HIFTXD base pointer 09 [35:32]
    BASE08[31..28]               - (RW) HIFTXD base pointer 08 [35:32]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE08_OFFSET         WED_PAO_HIFTXD_BASE_08_15_H_OFFSET
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE08_MASK           0xF0000000                // BASE08[31..28]
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE08_SHFT           28
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE09_OFFSET         WED_PAO_HIFTXD_BASE_08_15_H_OFFSET
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE09_MASK           0x0F000000                // BASE09[27..24]
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE09_SHFT           24
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE10_OFFSET         WED_PAO_HIFTXD_BASE_08_15_H_OFFSET
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE10_MASK           0x00F00000                // BASE10[23..20]
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE10_SHFT           20
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE11_OFFSET         WED_PAO_HIFTXD_BASE_08_15_H_OFFSET
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE11_MASK           0x000F0000                // BASE11[19..16]
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE11_SHFT           16
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE12_OFFSET         WED_PAO_HIFTXD_BASE_08_15_H_OFFSET
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE12_MASK           0x0000F000                // BASE12[15..12]
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE12_SHFT           12
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE13_OFFSET         WED_PAO_HIFTXD_BASE_08_15_H_OFFSET
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE13_MASK           0x00000F00                // BASE13[11..8]
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE13_SHFT           8
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE14_OFFSET         WED_PAO_HIFTXD_BASE_08_15_H_OFFSET
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE14_MASK           0x000000F0                // BASE14[7..4]
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE14_SHFT           4
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE15_OFFSET         WED_PAO_HIFTXD_BASE_08_15_H_OFFSET
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE15_MASK           0x0000000F                // BASE15[3..0]
#define WED_PAO_HIFTXD_BASE_08_15_H_BASE15_SHFT           0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE_16_23_H (0x15015800 + 0x208)---

    BASE23[3..0]                 - (RW) HIFTXD base pointer 23 [35:32]
    BASE22[7..4]                 - (RW) HIFTXD base pointer 22 [35:32]
    BASE21[11..8]                - (RW) HIFTXD base pointer 21 [35:32]
    BASE20[15..12]               - (RW) HIFTXD base pointer 20 [35:32]
    BASE19[19..16]               - (RW) HIFTXD base pointer 19 [35:32]
    BASE18[23..20]               - (RW) HIFTXD base pointer 18 [35:32]
    BASE17[27..24]               - (RW) HIFTXD base pointer 17 [35:32]
    BASE16[31..28]               - (RW) HIFTXD base pointer 16 [35:32]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE16_OFFSET         WED_PAO_HIFTXD_BASE_16_23_H_OFFSET
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE16_MASK           0xF0000000                // BASE16[31..28]
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE16_SHFT           28
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE17_OFFSET         WED_PAO_HIFTXD_BASE_16_23_H_OFFSET
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE17_MASK           0x0F000000                // BASE17[27..24]
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE17_SHFT           24
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE18_OFFSET         WED_PAO_HIFTXD_BASE_16_23_H_OFFSET
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE18_MASK           0x00F00000                // BASE18[23..20]
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE18_SHFT           20
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE19_OFFSET         WED_PAO_HIFTXD_BASE_16_23_H_OFFSET
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE19_MASK           0x000F0000                // BASE19[19..16]
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE19_SHFT           16
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE20_OFFSET         WED_PAO_HIFTXD_BASE_16_23_H_OFFSET
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE20_MASK           0x0000F000                // BASE20[15..12]
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE20_SHFT           12
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE21_OFFSET         WED_PAO_HIFTXD_BASE_16_23_H_OFFSET
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE21_MASK           0x00000F00                // BASE21[11..8]
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE21_SHFT           8
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE22_OFFSET         WED_PAO_HIFTXD_BASE_16_23_H_OFFSET
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE22_MASK           0x000000F0                // BASE22[7..4]
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE22_SHFT           4
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE23_OFFSET         WED_PAO_HIFTXD_BASE_16_23_H_OFFSET
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE23_MASK           0x0000000F                // BASE23[3..0]
#define WED_PAO_HIFTXD_BASE_16_23_H_BASE23_SHFT           0

/* =====================================================================================

  ---WED_PAO_HIFTXD_BASE_24_31_H (0x15015800 + 0x20C)---

    BASE31[3..0]                 - (RW) HIFTXD base pointer 31 [35:32]
    BASE30[7..4]                 - (RW) HIFTXD base pointer 30 [35:32]
    BASE29[11..8]                - (RW) HIFTXD base pointer 29 [35:32]
    BASE28[15..12]               - (RW) HIFTXD base pointer 28 [35:32]
    BASE27[19..16]               - (RW) HIFTXD base pointer 27 [35:32]
    BASE26[23..20]               - (RW) HIFTXD base pointer 26 [35:32]
    BASE25[27..24]               - (RW) HIFTXD base pointer 25 [35:32]
    BASE24[31..28]               - (RW) HIFTXD base pointer 24 [35:32]

 =====================================================================================*/
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE24_OFFSET         WED_PAO_HIFTXD_BASE_24_31_H_OFFSET
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE24_MASK           0xF0000000                // BASE24[31..28]
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE24_SHFT           28
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE25_OFFSET         WED_PAO_HIFTXD_BASE_24_31_H_OFFSET
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE25_MASK           0x0F000000                // BASE25[27..24]
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE25_SHFT           24
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE26_OFFSET         WED_PAO_HIFTXD_BASE_24_31_H_OFFSET
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE26_MASK           0x00F00000                // BASE26[23..20]
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE26_SHFT           20
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE27_OFFSET         WED_PAO_HIFTXD_BASE_24_31_H_OFFSET
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE27_MASK           0x000F0000                // BASE27[19..16]
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE27_SHFT           16
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE28_OFFSET         WED_PAO_HIFTXD_BASE_24_31_H_OFFSET
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE28_MASK           0x0000F000                // BASE28[15..12]
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE28_SHFT           12
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE29_OFFSET         WED_PAO_HIFTXD_BASE_24_31_H_OFFSET
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE29_MASK           0x00000F00                // BASE29[11..8]
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE29_SHFT           8
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE30_OFFSET         WED_PAO_HIFTXD_BASE_24_31_H_OFFSET
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE30_MASK           0x000000F0                // BASE30[7..4]
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE30_SHFT           4
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE31_OFFSET         WED_PAO_HIFTXD_BASE_24_31_H_OFFSET
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE31_MASK           0x0000000F                // BASE31[3..0]
#define WED_PAO_HIFTXD_BASE_24_31_H_BASE31_SHFT           0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO (0x15015800 + 0x230)---

    FSM[7..0]                    - (RO) AMSDU_FIFO module FSM
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_FSM_OFFSET                 WED_PAO_MON_AMSDU_FIFO_OFFSET
#define WED_PAO_MON_AMSDU_FIFO_FSM_MASK                   0x000000FF                // FSM[7..0]
#define WED_PAO_MON_AMSDU_FIFO_FSM_SHFT                   0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT0 (0x15015800 + 0x234)---

    DMAD[31..0]                  - (RO) AMSDU_FIFO DMAD counter, how many DMAD input

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT0_DMAD_OFFSET           WED_PAO_MON_AMSDU_FIFO_CNT0_OFFSET
#define WED_PAO_MON_AMSDU_FIFO_CNT0_DMAD_MASK             0xFFFFFFFF                // DMAD[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT0_DMAD_SHFT             0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT1 (0x15015800 + 0x238)---

    JUMBO_MSDU_CNT[31..0]        - (RO) AMSDU_FIFO MSDU counter, count jumbo MSDU (partial HIFTXD, BUF_NUM!=0, MSDU_CNT==1)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT1_JUMBO_MSDU_CNT_OFFSET WED_PAO_MON_AMSDU_FIFO_CNT1_OFFSET
#define WED_PAO_MON_AMSDU_FIFO_CNT1_JUMBO_MSDU_CNT_MASK   0xFFFFFFFF                // JUMBO_MSDU_CNT[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT1_JUMBO_MSDU_CNT_SHFT   0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT2 (0x15015800 + 0x23C)---

    IS_SP_DMAD_CNT[31..0]        - (RO) AMSDU_FIFO DMAD counter, count IS_SP MSDU (partial HIFTXD)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT2_IS_SP_DMAD_CNT_OFFSET WED_PAO_MON_AMSDU_FIFO_CNT2_OFFSET
#define WED_PAO_MON_AMSDU_FIFO_CNT2_IS_SP_DMAD_CNT_MASK   0xFFFFFFFF                // IS_SP_DMAD_CNT[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT2_IS_SP_DMAD_CNT_SHFT   0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT3 (0x15015800 + 0x240)---

    AMSDU_EN_0_CNT0[31..0]       - (RO) AMSDU_FIFO DMAD counter, count AMSDU_EN=0 MSDU (partial HIFTXD) (original field value in WDMA Rx DMAD)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT3_AMSDU_EN_0_CNT0_OFFSET WED_PAO_MON_AMSDU_FIFO_CNT3_OFFSET
#define WED_PAO_MON_AMSDU_FIFO_CNT3_AMSDU_EN_0_CNT0_MASK  0xFFFFFFFF                // AMSDU_EN_0_CNT0[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT3_AMSDU_EN_0_CNT0_SHFT  0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT4 (0x15015800 + 0x244)---

    AMSDU_EN_0_CNT1[31..0]       - (RO) AMSDU_FIFO DMAD counter, count AMSDU_EN=0 MSDU (partial HIFTXD) (overwrite if SDL< MIN_SDL, always check)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT4_AMSDU_EN_0_CNT1_OFFSET WED_PAO_MON_AMSDU_FIFO_CNT4_OFFSET
#define WED_PAO_MON_AMSDU_FIFO_CNT4_AMSDU_EN_0_CNT1_MASK  0xFFFFFFFF                // AMSDU_EN_0_CNT1[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT4_AMSDU_EN_0_CNT1_SHFT  0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT5 (0x15015800 + 0x248)---

    AMSDU_EN_0_CNT2[31..0]       - (RO) AMSDU_FIFO DMAD counter, count AMSDU_EN=0 MSDU (partial HIFTXD) (overwrite if ETHER_TYPE<16'h0600, has CR option)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT5_AMSDU_EN_0_CNT2_OFFSET WED_PAO_MON_AMSDU_FIFO_CNT5_OFFSET
#define WED_PAO_MON_AMSDU_FIFO_CNT5_AMSDU_EN_0_CNT2_MASK  0xFFFFFFFF                // AMSDU_EN_0_CNT2[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT5_AMSDU_EN_0_CNT2_SHFT  0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT6 (0x15015800 + 0x24C)---

    AMSDU_EN_0_CNT3[31..0]       - (RO) AMSDU_FIFO DMAD counter, count AMSDU_EN=0 MSDU (partial HIFTXD) (overwrite if JUMBO_MSDU, has CR option)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT6_AMSDU_EN_0_CNT3_OFFSET WED_PAO_MON_AMSDU_FIFO_CNT6_OFFSET
#define WED_PAO_MON_AMSDU_FIFO_CNT6_AMSDU_EN_0_CNT3_MASK  0xFFFFFFFF                // AMSDU_EN_0_CNT3[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT6_AMSDU_EN_0_CNT3_SHFT  0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT7 (0x15015800 + 0x250)---

    MSDU_CNT[31..0]              - (RO) AMSDU_FIFO MSDU counter, count MSDU (partial HIFTXD)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT7_MSDU_CNT_OFFSET       WED_PAO_MON_AMSDU_FIFO_CNT7_OFFSET
#define WED_PAO_MON_AMSDU_FIFO_CNT7_MSDU_CNT_MASK         0xFFFFFFFF                // MSDU_CNT[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT7_MSDU_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT8 (0x15015800 + 0x254)---

    ETHER_TYPE_CNT[31..0]        - (RO) AMSDU_FIFO MSDU counter, count ETHER_TYPE<16'h0600

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT8_ETHER_TYPE_CNT_OFFSET WED_PAO_MON_AMSDU_FIFO_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_FIFO_CNT8_ETHER_TYPE_CNT_MASK   0xFFFFFFFF                // ETHER_TYPE_CNT[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT8_ETHER_TYPE_CNT_SHFT   0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_DMAD_0 (0x15015800 + 0x25C)---

    DATA[31..0]                  - (RO) AMSDU_FIFO latch DMAD data

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_DMAD_0_DATA_OFFSET         WED_PAO_MON_AMSDU_FIFO_DMAD_0_OFFSET
#define WED_PAO_MON_AMSDU_FIFO_DMAD_0_DATA_MASK           0xFFFFFFFF                // DATA[31..0]
#define WED_PAO_MON_AMSDU_FIFO_DMAD_0_DATA_SHFT           0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_DMAD_1 (0x15015800 + 0x260)---

    DATA[31..0]                  - (RO) AMSDU_FIFO latch DMAD data

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_DMAD_1_DATA_OFFSET         WED_PAO_MON_AMSDU_FIFO_DMAD_1_OFFSET
#define WED_PAO_MON_AMSDU_FIFO_DMAD_1_DATA_MASK           0xFFFFFFFF                // DATA[31..0]
#define WED_PAO_MON_AMSDU_FIFO_DMAD_1_DATA_SHFT           0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_DMAD_2 (0x15015800 + 0x264)---

    DATA[31..0]                  - (RO) AMSDU_FIFO latch DMAD data

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_DMAD_2_DATA_OFFSET         WED_PAO_MON_AMSDU_FIFO_DMAD_2_OFFSET
#define WED_PAO_MON_AMSDU_FIFO_DMAD_2_DATA_MASK           0xFFFFFFFF                // DATA[31..0]
#define WED_PAO_MON_AMSDU_FIFO_DMAD_2_DATA_SHFT           0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_DMAD_3 (0x15015800 + 0x268)---

    DATA[31..0]                  - (RO) AMSDU_FIFO latch DMAD data

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_DMAD_3_DATA_OFFSET         WED_PAO_MON_AMSDU_FIFO_DMAD_3_OFFSET
#define WED_PAO_MON_AMSDU_FIFO_DMAD_3_DATA_MASK           0xFFFFFFFF                // DATA[31..0]
#define WED_PAO_MON_AMSDU_FIFO_DMAD_3_DATA_SHFT           0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_DMAD_4 (0x15015800 + 0x26C)---

    DATA[31..0]                  - (RO) AMSDU_FIFO latch DMAD data

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_DMAD_4_DATA_OFFSET         WED_PAO_MON_AMSDU_FIFO_DMAD_4_OFFSET
#define WED_PAO_MON_AMSDU_FIFO_DMAD_4_DATA_MASK           0xFFFFFFFF                // DATA[31..0]
#define WED_PAO_MON_AMSDU_FIFO_DMAD_4_DATA_SHFT           0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0 (0x15015800 + 0x270)---

    FSM[16..0]                   - (RO) AMSDU_ENG FSM
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_FSM_OFFSET                 WED_PAO_MON_AMSDU_ENG0_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_FSM_MASK                   0x0001FFFF                // FSM[16..0]
#define WED_PAO_MON_AMSDU_ENG0_FSM_SHFT                   0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_CNT0 (0x15015800 + 0x274)---

    SEARCH_ADD_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Add cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_CNT0_SEARCH_ADD_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG0_CNT0_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT0_SEARCH_ADD_CYCLE_MASK 0xFFFFFFFF                // SEARCH_ADD_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG0_CNT0_SEARCH_ADD_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_CNT1 (0x15015800 + 0x278)---

    SEARCH_MRG_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Merge cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_CNT1_SEARCH_MRG_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG0_CNT1_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT1_SEARCH_MRG_CYCLE_MASK 0xFFFFFFFF                // SEARCH_MRG_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG0_CNT1_SEARCH_MRG_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_CNT2 (0x15015800 + 0x27C)---

    DEQ_CYCLE[31..0]             - (RO) AMSDU_ENG do De-queue cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_CNT2_DEQ_CYCLE_OFFSET      WED_PAO_MON_AMSDU_ENG0_CNT2_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT2_DEQ_CYCLE_MASK        0xFFFFFFFF                // DEQ_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG0_CNT2_DEQ_CYCLE_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_CNT3 (0x15015800 + 0x280)---

    DMAD_CNT[31..0]              - (RO) AMSDU_ENG DMAD counter, how many DMAD input

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_CNT3_DMAD_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG0_CNT3_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT3_DMAD_CNT_MASK         0xFFFFFFFF                // DMAD_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG0_CNT3_DMAD_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_CNT4 (0x15015800 + 0x284)---

    QFPL_CNT[31..0]              - (RO) AMSDU_ENG QFPL counter, requeus free page times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_CNT4_QFPL_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG0_CNT4_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT4_QFPL_CNT_MASK         0xFFFFFFFF                // QFPL_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG0_CNT4_QFPL_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_CNT5 (0x15015800 + 0x288)---

    QENI_CNT[31..0]              - (RO) AMSDU_ENG QENI counter, en-queue to input queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_CNT5_QENI_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG0_CNT5_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT5_QENI_CNT_MASK         0xFFFFFFFF                // QENI_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG0_CNT5_QENI_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_CNT6 (0x15015800 + 0x28C)---

    QENO_CNT[31..0]              - (RO) AMSDU_ENG QENO counter, en-queue to output queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_CNT6_QENO_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG0_CNT6_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT6_QENO_CNT_MASK         0xFFFFFFFF                // QENO_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG0_CNT6_QENO_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_CNT7 (0x15015800 + 0x290)---

    MERG_CNT[31..0]              - (RO) AMSDU_ENG MERG counter, how many DMADs are merged

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_CNT7_MERG_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG0_CNT7_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT7_MERG_CNT_MASK         0xFFFFFFFF                // MERG_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG0_CNT7_MERG_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_CNT8 (0x15015800 + 0x294)---

    MAX_QGPP_CNT[10..0]          - (RO) AMSDU_ENG Max search times
    RESERVED11[15..11]           - (RO) Reserved bits
    MAX_PL_CNT[27..16]           - (RO) AMSDU_ENG Max entry usage
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_CNT8_MAX_PL_CNT_OFFSET     WED_PAO_MON_AMSDU_ENG0_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT8_MAX_PL_CNT_MASK       0x0FFF0000                // MAX_PL_CNT[27..16]
#define WED_PAO_MON_AMSDU_ENG0_CNT8_MAX_PL_CNT_SHFT       16
#define WED_PAO_MON_AMSDU_ENG0_CNT8_MAX_QGPP_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG0_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT8_MAX_QGPP_CNT_MASK     0x000007FF                // MAX_QGPP_CNT[10..0]
#define WED_PAO_MON_AMSDU_ENG0_CNT8_MAX_QGPP_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_CNT9 (0x15015800 + 0x298)---

    PORTQ_CNT[11..0]             - (RO) AMSDU_ENG current entry usage
    RESERVED12[15..12]           - (RO) Reserved bits
    MAX_BUF_NUM[20..16]          - (RO) AMSDU_ENG Max merge TXPs count
    RESERVED21[23..21]           - (RO) Reserved bits
    MAX_MSDU_CNT[28..24]         - (RO) AMSDU_ENG Max merge MSDUs count
    RESERVED29[31..29]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_CNT9_MAX_MSDU_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG0_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT9_MAX_MSDU_CNT_MASK     0x1F000000                // MAX_MSDU_CNT[28..24]
#define WED_PAO_MON_AMSDU_ENG0_CNT9_MAX_MSDU_CNT_SHFT     24
#define WED_PAO_MON_AMSDU_ENG0_CNT9_MAX_BUF_NUM_OFFSET    WED_PAO_MON_AMSDU_ENG0_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT9_MAX_BUF_NUM_MASK      0x001F0000                // MAX_BUF_NUM[20..16]
#define WED_PAO_MON_AMSDU_ENG0_CNT9_MAX_BUF_NUM_SHFT      16
#define WED_PAO_MON_AMSDU_ENG0_CNT9_PORTQ_CNT_OFFSET      WED_PAO_MON_AMSDU_ENG0_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT9_PORTQ_CNT_MASK        0x00000FFF                // PORTQ_CNT[11..0]
#define WED_PAO_MON_AMSDU_ENG0_CNT9_PORTQ_CNT_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_CNT10 (0x15015800 + 0x29C)---

    SEARCH_MAX_ENQ_CNT[31..0]    - (RO) AMSDU_ENG search to MAX and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_CNT10_SEARCH_MAX_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG0_CNT10_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT10_SEARCH_MAX_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG0_CNT10_SEARCH_MAX_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_CNT11 (0x15015800 + 0x2A0)---

    SEARCH_HEAD_ENQ_CNT[31..0]   - (RO) AMSDU_ENG search to HEAD and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_CNT11_SEARCH_HEAD_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG0_CNT11_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT11_SEARCH_HEAD_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_HEAD_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG0_CNT11_SEARCH_HEAD_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_CNT12 (0x15015800 + 0x2A4)---

    MAX_ENQ_CNT[31..0]           - (RO) AMSDU_ENG Max en-queue ccount (BUF_NUM or MSDU_CNT or TX_BYTE_CNT reach max)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_CNT12_MAX_ENQ_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG0_CNT12_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT12_MAX_ENQ_CNT_MASK     0xFFFFFFFF                // MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG0_CNT12_MAX_ENQ_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_CNT13 (0x15015800 + 0x2A8)---

    WCID_HIT_ENQ_CNT[31..0]      - (RO) AMSDU_ENG WCID hit (other fields miss) en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_CNT13_WCID_HIT_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG0_CNT13_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_CNT13_WCID_HIT_ENQ_CNT_MASK 0xFFFFFFFF                // WCID_HIT_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG0_CNT13_WCID_HIT_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_PTR0 (0x15015800 + 0x2AC)---

    PORT_END[10..0]              - (RO) AMSDU_ENG current TAIL pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    PORT_START[26..16]           - (RO) AMSDU_ENG current HEAD pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_PTR0_PORT_START_OFFSET     WED_PAO_MON_AMSDU_ENG0_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_PTR0_PORT_START_MASK       0x07FF0000                // PORT_START[26..16]
#define WED_PAO_MON_AMSDU_ENG0_PTR0_PORT_START_SHFT       16
#define WED_PAO_MON_AMSDU_ENG0_PTR0_PORT_END_OFFSET       WED_PAO_MON_AMSDU_ENG0_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_PTR0_PORT_END_MASK         0x000007FF                // PORT_END[10..0]
#define WED_PAO_MON_AMSDU_ENG0_PTR0_PORT_END_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_PTR1 (0x15015800 + 0x2B0)---

    CUR_HEAD_PTR[10..0]          - (RO) AMSDU_ENG current HEAD pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_WRIT_PTR[26..16]         - (RO) AMSDU_ENG current Write pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_PTR1_CUR_WRIT_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG0_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_PTR1_CUR_WRIT_PTR_MASK     0x07FF0000                // CUR_WRIT_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG0_PTR1_CUR_WRIT_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG0_PTR1_CUR_HEAD_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG0_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_PTR1_CUR_HEAD_PTR_MASK     0x000007FF                // CUR_HEAD_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG0_PTR1_CUR_HEAD_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG0_PTR2 (0x15015800 + 0x2B4)---

    CUR_QGPP_PTR[10..0]          - (RO) AMSDU_ENG current SEARCH pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_TAIL_PTR[26..16]         - (RO) AMSDU_ENG current TAIL pointer (internal)
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG0_PTR2_CUR_TAIL_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG0_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_PTR2_CUR_TAIL_PTR_MASK     0x07FF0000                // CUR_TAIL_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG0_PTR2_CUR_TAIL_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG0_PTR2_CUR_QGPP_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG0_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG0_PTR2_CUR_QGPP_PTR_MASK     0x000007FF                // CUR_QGPP_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG0_PTR2_CUR_QGPP_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1 (0x15015800 + 0x2C0)---

    FSM[16..0]                   - (RO) AMSDU_ENG FSM
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_FSM_OFFSET                 WED_PAO_MON_AMSDU_ENG1_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_FSM_MASK                   0x0001FFFF                // FSM[16..0]
#define WED_PAO_MON_AMSDU_ENG1_FSM_SHFT                   0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_CNT0 (0x15015800 + 0x2C4)---

    SEARCH_ADD_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Add cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_CNT0_SEARCH_ADD_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG1_CNT0_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT0_SEARCH_ADD_CYCLE_MASK 0xFFFFFFFF                // SEARCH_ADD_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG1_CNT0_SEARCH_ADD_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_CNT1 (0x15015800 + 0x2C8)---

    SEARCH_MRG_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Merge cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_CNT1_SEARCH_MRG_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG1_CNT1_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT1_SEARCH_MRG_CYCLE_MASK 0xFFFFFFFF                // SEARCH_MRG_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG1_CNT1_SEARCH_MRG_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_CNT2 (0x15015800 + 0x2CC)---

    DEQ_CYCLE[31..0]             - (RO) AMSDU_ENG do De-queue cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_CNT2_DEQ_CYCLE_OFFSET      WED_PAO_MON_AMSDU_ENG1_CNT2_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT2_DEQ_CYCLE_MASK        0xFFFFFFFF                // DEQ_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG1_CNT2_DEQ_CYCLE_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_CNT3 (0x15015800 + 0x2D0)---

    DMAD_CNT[31..0]              - (RO) AMSDU_ENG DMAD counter, how many DMAD input

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_CNT3_DMAD_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG1_CNT3_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT3_DMAD_CNT_MASK         0xFFFFFFFF                // DMAD_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG1_CNT3_DMAD_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_CNT4 (0x15015800 + 0x2D4)---

    QFPL_CNT[31..0]              - (RO) AMSDU_ENG QFPL counter, requeus free page times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_CNT4_QFPL_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG1_CNT4_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT4_QFPL_CNT_MASK         0xFFFFFFFF                // QFPL_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG1_CNT4_QFPL_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_CNT5 (0x15015800 + 0x2D8)---

    QENI_CNT[31..0]              - (RO) AMSDU_ENG QENI counter, en-queue to input queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_CNT5_QENI_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG1_CNT5_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT5_QENI_CNT_MASK         0xFFFFFFFF                // QENI_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG1_CNT5_QENI_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_CNT6 (0x15015800 + 0x2DC)---

    QENO_CNT[31..0]              - (RO) AMSDU_ENG QENO counter, en-queue to output queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_CNT6_QENO_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG1_CNT6_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT6_QENO_CNT_MASK         0xFFFFFFFF                // QENO_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG1_CNT6_QENO_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_CNT7 (0x15015800 + 0x2E0)---

    MERG_CNT[31..0]              - (RO) AMSDU_ENG MERG counter, how many DMADs are merged

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_CNT7_MERG_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG1_CNT7_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT7_MERG_CNT_MASK         0xFFFFFFFF                // MERG_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG1_CNT7_MERG_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_CNT8 (0x15015800 + 0x2E4)---

    MAX_QGPP_CNT[10..0]          - (RO) AMSDU_ENG Max search times
    RESERVED11[15..11]           - (RO) Reserved bits
    MAX_PL_CNT[27..16]           - (RO) AMSDU_ENG Max entry usage
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_CNT8_MAX_PL_CNT_OFFSET     WED_PAO_MON_AMSDU_ENG1_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT8_MAX_PL_CNT_MASK       0x0FFF0000                // MAX_PL_CNT[27..16]
#define WED_PAO_MON_AMSDU_ENG1_CNT8_MAX_PL_CNT_SHFT       16
#define WED_PAO_MON_AMSDU_ENG1_CNT8_MAX_QGPP_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG1_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT8_MAX_QGPP_CNT_MASK     0x000007FF                // MAX_QGPP_CNT[10..0]
#define WED_PAO_MON_AMSDU_ENG1_CNT8_MAX_QGPP_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_CNT9 (0x15015800 + 0x2E8)---

    PORTQ_CNT[11..0]             - (RO) AMSDU_ENG current entry usage
    RESERVED12[15..12]           - (RO) Reserved bits
    MAX_BUF_NUM[20..16]          - (RO) AMSDU_ENG Max merge TXPs count
    RESERVED21[23..21]           - (RO) Reserved bits
    MAX_MSDU_CNT[28..24]         - (RO) AMSDU_ENG Max merge MSDUs count
    RESERVED29[31..29]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_CNT9_MAX_MSDU_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG1_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT9_MAX_MSDU_CNT_MASK     0x1F000000                // MAX_MSDU_CNT[28..24]
#define WED_PAO_MON_AMSDU_ENG1_CNT9_MAX_MSDU_CNT_SHFT     24
#define WED_PAO_MON_AMSDU_ENG1_CNT9_MAX_BUF_NUM_OFFSET    WED_PAO_MON_AMSDU_ENG1_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT9_MAX_BUF_NUM_MASK      0x001F0000                // MAX_BUF_NUM[20..16]
#define WED_PAO_MON_AMSDU_ENG1_CNT9_MAX_BUF_NUM_SHFT      16
#define WED_PAO_MON_AMSDU_ENG1_CNT9_PORTQ_CNT_OFFSET      WED_PAO_MON_AMSDU_ENG1_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT9_PORTQ_CNT_MASK        0x00000FFF                // PORTQ_CNT[11..0]
#define WED_PAO_MON_AMSDU_ENG1_CNT9_PORTQ_CNT_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_CNT10 (0x15015800 + 0x2EC)---

    SEARCH_MAX_ENQ_CNT[31..0]    - (RO) AMSDU_ENG search to MAX and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_CNT10_SEARCH_MAX_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG1_CNT10_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT10_SEARCH_MAX_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG1_CNT10_SEARCH_MAX_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_CNT11 (0x15015800 + 0x2F0)---

    SEARCH_HEAD_ENQ_CNT[31..0]   - (RO) AMSDU_ENG search to HEAD and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_CNT11_SEARCH_HEAD_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG1_CNT11_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT11_SEARCH_HEAD_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_HEAD_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG1_CNT11_SEARCH_HEAD_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_CNT12 (0x15015800 + 0x2F4)---

    MAX_ENQ_CNT[31..0]           - (RO) AMSDU_ENG Max en-queue ccount (BUF_NUM or MSDU_CNT or TX_BYTE_CNT reach max)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_CNT12_MAX_ENQ_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG1_CNT12_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT12_MAX_ENQ_CNT_MASK     0xFFFFFFFF                // MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG1_CNT12_MAX_ENQ_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_CNT13 (0x15015800 + 0x2F8)---

    WCID_HIT_ENQ_CNT[31..0]      - (RO) AMSDU_ENG WCID hit (other fields miss) en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_CNT13_WCID_HIT_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG1_CNT13_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_CNT13_WCID_HIT_ENQ_CNT_MASK 0xFFFFFFFF                // WCID_HIT_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG1_CNT13_WCID_HIT_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_PTR0 (0x15015800 + 0x2FC)---

    PORT_END[10..0]              - (RO) AMSDU_ENG current TAIL pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    PORT_START[26..16]           - (RO) AMSDU_ENG current HEAD pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_PTR0_PORT_START_OFFSET     WED_PAO_MON_AMSDU_ENG1_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_PTR0_PORT_START_MASK       0x07FF0000                // PORT_START[26..16]
#define WED_PAO_MON_AMSDU_ENG1_PTR0_PORT_START_SHFT       16
#define WED_PAO_MON_AMSDU_ENG1_PTR0_PORT_END_OFFSET       WED_PAO_MON_AMSDU_ENG1_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_PTR0_PORT_END_MASK         0x000007FF                // PORT_END[10..0]
#define WED_PAO_MON_AMSDU_ENG1_PTR0_PORT_END_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_PTR1 (0x15015800 + 0x300)---

    CUR_HEAD_PTR[10..0]          - (RO) AMSDU_ENG current HEAD pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_WRIT_PTR[26..16]         - (RO) AMSDU_ENG current Write pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_PTR1_CUR_WRIT_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG1_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_PTR1_CUR_WRIT_PTR_MASK     0x07FF0000                // CUR_WRIT_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG1_PTR1_CUR_WRIT_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG1_PTR1_CUR_HEAD_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG1_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_PTR1_CUR_HEAD_PTR_MASK     0x000007FF                // CUR_HEAD_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG1_PTR1_CUR_HEAD_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG1_PTR2 (0x15015800 + 0x304)---

    CUR_QGPP_PTR[10..0]          - (RO) AMSDU_ENG current SEARCH pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_TAIL_PTR[26..16]         - (RO) AMSDU_ENG current TAIL pointer (internal)
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG1_PTR2_CUR_TAIL_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG1_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_PTR2_CUR_TAIL_PTR_MASK     0x07FF0000                // CUR_TAIL_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG1_PTR2_CUR_TAIL_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG1_PTR2_CUR_QGPP_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG1_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG1_PTR2_CUR_QGPP_PTR_MASK     0x000007FF                // CUR_QGPP_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG1_PTR2_CUR_QGPP_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2 (0x15015800 + 0x310)---

    FSM[16..0]                   - (RO) AMSDU_ENG FSM
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_FSM_OFFSET                 WED_PAO_MON_AMSDU_ENG2_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_FSM_MASK                   0x0001FFFF                // FSM[16..0]
#define WED_PAO_MON_AMSDU_ENG2_FSM_SHFT                   0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_CNT0 (0x15015800 + 0x314)---

    SEARCH_ADD_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Add cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_CNT0_SEARCH_ADD_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG2_CNT0_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT0_SEARCH_ADD_CYCLE_MASK 0xFFFFFFFF                // SEARCH_ADD_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG2_CNT0_SEARCH_ADD_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_CNT1 (0x15015800 + 0x318)---

    SEARCH_MRG_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Merge cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_CNT1_SEARCH_MRG_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG2_CNT1_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT1_SEARCH_MRG_CYCLE_MASK 0xFFFFFFFF                // SEARCH_MRG_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG2_CNT1_SEARCH_MRG_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_CNT2 (0x15015800 + 0x31C)---

    DEQ_CYCLE[31..0]             - (RO) AMSDU_ENG do De-queue cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_CNT2_DEQ_CYCLE_OFFSET      WED_PAO_MON_AMSDU_ENG2_CNT2_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT2_DEQ_CYCLE_MASK        0xFFFFFFFF                // DEQ_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG2_CNT2_DEQ_CYCLE_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_CNT3 (0x15015800 + 0x320)---

    DMAD_CNT[31..0]              - (RO) AMSDU_ENG DMAD counter, how many DMAD input

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_CNT3_DMAD_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG2_CNT3_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT3_DMAD_CNT_MASK         0xFFFFFFFF                // DMAD_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG2_CNT3_DMAD_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_CNT4 (0x15015800 + 0x324)---

    QFPL_CNT[31..0]              - (RO) AMSDU_ENG QFPL counter, requeus free page times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_CNT4_QFPL_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG2_CNT4_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT4_QFPL_CNT_MASK         0xFFFFFFFF                // QFPL_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG2_CNT4_QFPL_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_CNT5 (0x15015800 + 0x328)---

    QENI_CNT[31..0]              - (RO) AMSDU_ENG QENI counter, en-queue to input queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_CNT5_QENI_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG2_CNT5_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT5_QENI_CNT_MASK         0xFFFFFFFF                // QENI_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG2_CNT5_QENI_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_CNT6 (0x15015800 + 0x32C)---

    QENO_CNT[31..0]              - (RO) AMSDU_ENG QENO counter, en-queue to output queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_CNT6_QENO_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG2_CNT6_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT6_QENO_CNT_MASK         0xFFFFFFFF                // QENO_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG2_CNT6_QENO_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_CNT7 (0x15015800 + 0x330)---

    MERG_CNT[31..0]              - (RO) AMSDU_ENG MERG counter, how many DMADs are merged

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_CNT7_MERG_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG2_CNT7_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT7_MERG_CNT_MASK         0xFFFFFFFF                // MERG_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG2_CNT7_MERG_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_CNT8 (0x15015800 + 0x334)---

    MAX_QGPP_CNT[10..0]          - (RO) AMSDU_ENG Max search times
    RESERVED11[15..11]           - (RO) Reserved bits
    MAX_PL_CNT[27..16]           - (RO) AMSDU_ENG Max entry usage
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_CNT8_MAX_PL_CNT_OFFSET     WED_PAO_MON_AMSDU_ENG2_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT8_MAX_PL_CNT_MASK       0x0FFF0000                // MAX_PL_CNT[27..16]
#define WED_PAO_MON_AMSDU_ENG2_CNT8_MAX_PL_CNT_SHFT       16
#define WED_PAO_MON_AMSDU_ENG2_CNT8_MAX_QGPP_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG2_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT8_MAX_QGPP_CNT_MASK     0x000007FF                // MAX_QGPP_CNT[10..0]
#define WED_PAO_MON_AMSDU_ENG2_CNT8_MAX_QGPP_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_CNT9 (0x15015800 + 0x338)---

    PORTQ_CNT[11..0]             - (RO) AMSDU_ENG current entry usage
    RESERVED12[15..12]           - (RO) Reserved bits
    MAX_BUF_NUM[20..16]          - (RO) AMSDU_ENG Max merge TXPs count
    RESERVED21[23..21]           - (RO) Reserved bits
    MAX_MSDU_CNT[28..24]         - (RO) AMSDU_ENG Max merge MSDUs count
    RESERVED29[31..29]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_CNT9_MAX_MSDU_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG2_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT9_MAX_MSDU_CNT_MASK     0x1F000000                // MAX_MSDU_CNT[28..24]
#define WED_PAO_MON_AMSDU_ENG2_CNT9_MAX_MSDU_CNT_SHFT     24
#define WED_PAO_MON_AMSDU_ENG2_CNT9_MAX_BUF_NUM_OFFSET    WED_PAO_MON_AMSDU_ENG2_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT9_MAX_BUF_NUM_MASK      0x001F0000                // MAX_BUF_NUM[20..16]
#define WED_PAO_MON_AMSDU_ENG2_CNT9_MAX_BUF_NUM_SHFT      16
#define WED_PAO_MON_AMSDU_ENG2_CNT9_PORTQ_CNT_OFFSET      WED_PAO_MON_AMSDU_ENG2_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT9_PORTQ_CNT_MASK        0x00000FFF                // PORTQ_CNT[11..0]
#define WED_PAO_MON_AMSDU_ENG2_CNT9_PORTQ_CNT_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_CNT10 (0x15015800 + 0x33C)---

    SEARCH_MAX_ENQ_CNT[31..0]    - (RO) AMSDU_ENG search to MAX and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_CNT10_SEARCH_MAX_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG2_CNT10_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT10_SEARCH_MAX_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG2_CNT10_SEARCH_MAX_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_CNT11 (0x15015800 + 0x340)---

    SEARCH_HEAD_ENQ_CNT[31..0]   - (RO) AMSDU_ENG search to HEAD and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_CNT11_SEARCH_HEAD_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG2_CNT11_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT11_SEARCH_HEAD_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_HEAD_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG2_CNT11_SEARCH_HEAD_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_CNT12 (0x15015800 + 0x344)---

    MAX_ENQ_CNT[31..0]           - (RO) AMSDU_ENG Max en-queue ccount (BUF_NUM or MSDU_CNT or TX_BYTE_CNT reach max)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_CNT12_MAX_ENQ_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG2_CNT12_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT12_MAX_ENQ_CNT_MASK     0xFFFFFFFF                // MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG2_CNT12_MAX_ENQ_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_CNT13 (0x15015800 + 0x348)---

    WCID_HIT_ENQ_CNT[31..0]      - (RO) AMSDU_ENG WCID hit (other fields miss) en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_CNT13_WCID_HIT_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG2_CNT13_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_CNT13_WCID_HIT_ENQ_CNT_MASK 0xFFFFFFFF                // WCID_HIT_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG2_CNT13_WCID_HIT_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_PTR0 (0x15015800 + 0x34C)---

    PORT_END[10..0]              - (RO) AMSDU_ENG current TAIL pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    PORT_START[26..16]           - (RO) AMSDU_ENG current HEAD pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_PTR0_PORT_START_OFFSET     WED_PAO_MON_AMSDU_ENG2_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_PTR0_PORT_START_MASK       0x07FF0000                // PORT_START[26..16]
#define WED_PAO_MON_AMSDU_ENG2_PTR0_PORT_START_SHFT       16
#define WED_PAO_MON_AMSDU_ENG2_PTR0_PORT_END_OFFSET       WED_PAO_MON_AMSDU_ENG2_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_PTR0_PORT_END_MASK         0x000007FF                // PORT_END[10..0]
#define WED_PAO_MON_AMSDU_ENG2_PTR0_PORT_END_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_PTR1 (0x15015800 + 0x350)---

    CUR_HEAD_PTR[10..0]          - (RO) AMSDU_ENG current HEAD pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_WRIT_PTR[26..16]         - (RO) AMSDU_ENG current Write pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_PTR1_CUR_WRIT_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG2_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_PTR1_CUR_WRIT_PTR_MASK     0x07FF0000                // CUR_WRIT_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG2_PTR1_CUR_WRIT_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG2_PTR1_CUR_HEAD_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG2_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_PTR1_CUR_HEAD_PTR_MASK     0x000007FF                // CUR_HEAD_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG2_PTR1_CUR_HEAD_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG2_PTR2 (0x15015800 + 0x354)---

    CUR_QGPP_PTR[10..0]          - (RO) AMSDU_ENG current SEARCH pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_TAIL_PTR[26..16]         - (RO) AMSDU_ENG current TAIL pointer (internal)
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG2_PTR2_CUR_TAIL_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG2_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_PTR2_CUR_TAIL_PTR_MASK     0x07FF0000                // CUR_TAIL_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG2_PTR2_CUR_TAIL_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG2_PTR2_CUR_QGPP_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG2_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG2_PTR2_CUR_QGPP_PTR_MASK     0x000007FF                // CUR_QGPP_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG2_PTR2_CUR_QGPP_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3 (0x15015800 + 0x360)---

    FSM[16..0]                   - (RO) AMSDU_ENG FSM
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_FSM_OFFSET                 WED_PAO_MON_AMSDU_ENG3_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_FSM_MASK                   0x0001FFFF                // FSM[16..0]
#define WED_PAO_MON_AMSDU_ENG3_FSM_SHFT                   0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_CNT0 (0x15015800 + 0x364)---

    SEARCH_ADD_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Add cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_CNT0_SEARCH_ADD_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG3_CNT0_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT0_SEARCH_ADD_CYCLE_MASK 0xFFFFFFFF                // SEARCH_ADD_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG3_CNT0_SEARCH_ADD_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_CNT1 (0x15015800 + 0x368)---

    SEARCH_MRG_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Merge cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_CNT1_SEARCH_MRG_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG3_CNT1_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT1_SEARCH_MRG_CYCLE_MASK 0xFFFFFFFF                // SEARCH_MRG_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG3_CNT1_SEARCH_MRG_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_CNT2 (0x15015800 + 0x36C)---

    DEQ_CYCLE[31..0]             - (RO) AMSDU_ENG do De-queue cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_CNT2_DEQ_CYCLE_OFFSET      WED_PAO_MON_AMSDU_ENG3_CNT2_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT2_DEQ_CYCLE_MASK        0xFFFFFFFF                // DEQ_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG3_CNT2_DEQ_CYCLE_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_CNT3 (0x15015800 + 0x370)---

    DMAD_CNT[31..0]              - (RO) AMSDU_ENG DMAD counter, how many DMAD input

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_CNT3_DMAD_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG3_CNT3_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT3_DMAD_CNT_MASK         0xFFFFFFFF                // DMAD_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG3_CNT3_DMAD_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_CNT4 (0x15015800 + 0x374)---

    QFPL_CNT[31..0]              - (RO) AMSDU_ENG QFPL counter, requeus free page times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_CNT4_QFPL_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG3_CNT4_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT4_QFPL_CNT_MASK         0xFFFFFFFF                // QFPL_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG3_CNT4_QFPL_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_CNT5 (0x15015800 + 0x378)---

    QENI_CNT[31..0]              - (RO) AMSDU_ENG QENI counter, en-queue to input queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_CNT5_QENI_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG3_CNT5_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT5_QENI_CNT_MASK         0xFFFFFFFF                // QENI_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG3_CNT5_QENI_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_CNT6 (0x15015800 + 0x37C)---

    QENO_CNT[31..0]              - (RO) AMSDU_ENG QENO counter, en-queue to output queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_CNT6_QENO_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG3_CNT6_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT6_QENO_CNT_MASK         0xFFFFFFFF                // QENO_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG3_CNT6_QENO_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_CNT7 (0x15015800 + 0x380)---

    MERG_CNT[31..0]              - (RO) AMSDU_ENG MERG counter, how many DMADs are merged

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_CNT7_MERG_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG3_CNT7_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT7_MERG_CNT_MASK         0xFFFFFFFF                // MERG_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG3_CNT7_MERG_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_CNT8 (0x15015800 + 0x384)---

    MAX_QGPP_CNT[10..0]          - (RO) AMSDU_ENG Max search times
    RESERVED11[15..11]           - (RO) Reserved bits
    MAX_PL_CNT[27..16]           - (RO) AMSDU_ENG Max entry usage
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_CNT8_MAX_PL_CNT_OFFSET     WED_PAO_MON_AMSDU_ENG3_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT8_MAX_PL_CNT_MASK       0x0FFF0000                // MAX_PL_CNT[27..16]
#define WED_PAO_MON_AMSDU_ENG3_CNT8_MAX_PL_CNT_SHFT       16
#define WED_PAO_MON_AMSDU_ENG3_CNT8_MAX_QGPP_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG3_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT8_MAX_QGPP_CNT_MASK     0x000007FF                // MAX_QGPP_CNT[10..0]
#define WED_PAO_MON_AMSDU_ENG3_CNT8_MAX_QGPP_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_CNT9 (0x15015800 + 0x388)---

    PORTQ_CNT[11..0]             - (RO) AMSDU_ENG current entry usage
    RESERVED12[15..12]           - (RO) Reserved bits
    MAX_BUF_NUM[20..16]          - (RO) AMSDU_ENG Max merge TXPs count
    RESERVED21[23..21]           - (RO) Reserved bits
    MAX_MSDU_CNT[28..24]         - (RO) AMSDU_ENG Max merge MSDUs count
    RESERVED29[31..29]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_CNT9_MAX_MSDU_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG3_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT9_MAX_MSDU_CNT_MASK     0x1F000000                // MAX_MSDU_CNT[28..24]
#define WED_PAO_MON_AMSDU_ENG3_CNT9_MAX_MSDU_CNT_SHFT     24
#define WED_PAO_MON_AMSDU_ENG3_CNT9_MAX_BUF_NUM_OFFSET    WED_PAO_MON_AMSDU_ENG3_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT9_MAX_BUF_NUM_MASK      0x001F0000                // MAX_BUF_NUM[20..16]
#define WED_PAO_MON_AMSDU_ENG3_CNT9_MAX_BUF_NUM_SHFT      16
#define WED_PAO_MON_AMSDU_ENG3_CNT9_PORTQ_CNT_OFFSET      WED_PAO_MON_AMSDU_ENG3_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT9_PORTQ_CNT_MASK        0x00000FFF                // PORTQ_CNT[11..0]
#define WED_PAO_MON_AMSDU_ENG3_CNT9_PORTQ_CNT_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_CNT10 (0x15015800 + 0x38C)---

    SEARCH_MAX_ENQ_CNT[31..0]    - (RO) AMSDU_ENG search to MAX and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_CNT10_SEARCH_MAX_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG3_CNT10_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT10_SEARCH_MAX_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG3_CNT10_SEARCH_MAX_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_CNT11 (0x15015800 + 0x390)---

    SEARCH_HEAD_ENQ_CNT[31..0]   - (RO) AMSDU_ENG search to HEAD and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_CNT11_SEARCH_HEAD_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG3_CNT11_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT11_SEARCH_HEAD_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_HEAD_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG3_CNT11_SEARCH_HEAD_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_CNT12 (0x15015800 + 0x394)---

    MAX_ENQ_CNT[31..0]           - (RO) AMSDU_ENG Max en-queue ccount (BUF_NUM or MSDU_CNT or TX_BYTE_CNT reach max)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_CNT12_MAX_ENQ_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG3_CNT12_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT12_MAX_ENQ_CNT_MASK     0xFFFFFFFF                // MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG3_CNT12_MAX_ENQ_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_CNT13 (0x15015800 + 0x398)---

    WCID_HIT_ENQ_CNT[31..0]      - (RO) AMSDU_ENG WCID hit (other fields miss) en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_CNT13_WCID_HIT_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG3_CNT13_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_CNT13_WCID_HIT_ENQ_CNT_MASK 0xFFFFFFFF                // WCID_HIT_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG3_CNT13_WCID_HIT_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_PTR0 (0x15015800 + 0x39C)---

    PORT_END[10..0]              - (RO) AMSDU_ENG current TAIL pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    PORT_START[26..16]           - (RO) AMSDU_ENG current HEAD pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_PTR0_PORT_START_OFFSET     WED_PAO_MON_AMSDU_ENG3_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_PTR0_PORT_START_MASK       0x07FF0000                // PORT_START[26..16]
#define WED_PAO_MON_AMSDU_ENG3_PTR0_PORT_START_SHFT       16
#define WED_PAO_MON_AMSDU_ENG3_PTR0_PORT_END_OFFSET       WED_PAO_MON_AMSDU_ENG3_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_PTR0_PORT_END_MASK         0x000007FF                // PORT_END[10..0]
#define WED_PAO_MON_AMSDU_ENG3_PTR0_PORT_END_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_PTR1 (0x15015800 + 0x3A0)---

    CUR_HEAD_PTR[10..0]          - (RO) AMSDU_ENG current HEAD pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_WRIT_PTR[26..16]         - (RO) AMSDU_ENG current Write pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_PTR1_CUR_WRIT_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG3_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_PTR1_CUR_WRIT_PTR_MASK     0x07FF0000                // CUR_WRIT_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG3_PTR1_CUR_WRIT_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG3_PTR1_CUR_HEAD_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG3_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_PTR1_CUR_HEAD_PTR_MASK     0x000007FF                // CUR_HEAD_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG3_PTR1_CUR_HEAD_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG3_PTR2 (0x15015800 + 0x3A4)---

    CUR_QGPP_PTR[10..0]          - (RO) AMSDU_ENG current SEARCH pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_TAIL_PTR[26..16]         - (RO) AMSDU_ENG current TAIL pointer (internal)
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG3_PTR2_CUR_TAIL_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG3_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_PTR2_CUR_TAIL_PTR_MASK     0x07FF0000                // CUR_TAIL_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG3_PTR2_CUR_TAIL_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG3_PTR2_CUR_QGPP_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG3_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG3_PTR2_CUR_QGPP_PTR_MASK     0x000007FF                // CUR_QGPP_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG3_PTR2_CUR_QGPP_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4 (0x15015800 + 0x3B0)---

    FSM[16..0]                   - (RO) AMSDU_ENG FSM
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_FSM_OFFSET                 WED_PAO_MON_AMSDU_ENG4_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_FSM_MASK                   0x0001FFFF                // FSM[16..0]
#define WED_PAO_MON_AMSDU_ENG4_FSM_SHFT                   0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_CNT0 (0x15015800 + 0x3B4)---

    SEARCH_ADD_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Add cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_CNT0_SEARCH_ADD_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG4_CNT0_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT0_SEARCH_ADD_CYCLE_MASK 0xFFFFFFFF                // SEARCH_ADD_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG4_CNT0_SEARCH_ADD_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_CNT1 (0x15015800 + 0x3B8)---

    SEARCH_MRG_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Merge cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_CNT1_SEARCH_MRG_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG4_CNT1_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT1_SEARCH_MRG_CYCLE_MASK 0xFFFFFFFF                // SEARCH_MRG_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG4_CNT1_SEARCH_MRG_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_CNT2 (0x15015800 + 0x3BC)---

    DEQ_CYCLE[31..0]             - (RO) AMSDU_ENG do De-queue cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_CNT2_DEQ_CYCLE_OFFSET      WED_PAO_MON_AMSDU_ENG4_CNT2_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT2_DEQ_CYCLE_MASK        0xFFFFFFFF                // DEQ_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG4_CNT2_DEQ_CYCLE_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_CNT3 (0x15015800 + 0x3C0)---

    DMAD_CNT[31..0]              - (RO) AMSDU_ENG DMAD counter, how many DMAD input

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_CNT3_DMAD_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG4_CNT3_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT3_DMAD_CNT_MASK         0xFFFFFFFF                // DMAD_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG4_CNT3_DMAD_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_CNT4 (0x15015800 + 0x3C4)---

    QFPL_CNT[31..0]              - (RO) AMSDU_ENG QFPL counter, requeus free page times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_CNT4_QFPL_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG4_CNT4_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT4_QFPL_CNT_MASK         0xFFFFFFFF                // QFPL_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG4_CNT4_QFPL_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_CNT5 (0x15015800 + 0x3C8)---

    QENI_CNT[31..0]              - (RO) AMSDU_ENG QENI counter, en-queue to input queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_CNT5_QENI_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG4_CNT5_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT5_QENI_CNT_MASK         0xFFFFFFFF                // QENI_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG4_CNT5_QENI_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_CNT6 (0x15015800 + 0x3CC)---

    QENO_CNT[31..0]              - (RO) AMSDU_ENG QENO counter, en-queue to output queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_CNT6_QENO_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG4_CNT6_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT6_QENO_CNT_MASK         0xFFFFFFFF                // QENO_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG4_CNT6_QENO_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_CNT7 (0x15015800 + 0x3D0)---

    MERG_CNT[31..0]              - (RO) AMSDU_ENG MERG counter, how many DMADs are merged

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_CNT7_MERG_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG4_CNT7_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT7_MERG_CNT_MASK         0xFFFFFFFF                // MERG_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG4_CNT7_MERG_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_CNT8 (0x15015800 + 0x3D4)---

    MAX_QGPP_CNT[10..0]          - (RO) AMSDU_ENG Max search times
    RESERVED11[15..11]           - (RO) Reserved bits
    MAX_PL_CNT[27..16]           - (RO) AMSDU_ENG Max entry usage
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_CNT8_MAX_PL_CNT_OFFSET     WED_PAO_MON_AMSDU_ENG4_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT8_MAX_PL_CNT_MASK       0x0FFF0000                // MAX_PL_CNT[27..16]
#define WED_PAO_MON_AMSDU_ENG4_CNT8_MAX_PL_CNT_SHFT       16
#define WED_PAO_MON_AMSDU_ENG4_CNT8_MAX_QGPP_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG4_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT8_MAX_QGPP_CNT_MASK     0x000007FF                // MAX_QGPP_CNT[10..0]
#define WED_PAO_MON_AMSDU_ENG4_CNT8_MAX_QGPP_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_CNT9 (0x15015800 + 0x3D8)---

    PORTQ_CNT[11..0]             - (RO) AMSDU_ENG current entry usage
    RESERVED12[15..12]           - (RO) Reserved bits
    MAX_BUF_NUM[20..16]          - (RO) AMSDU_ENG Max merge TXPs count
    RESERVED21[23..21]           - (RO) Reserved bits
    MAX_MSDU_CNT[28..24]         - (RO) AMSDU_ENG Max merge MSDUs count
    RESERVED29[31..29]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_CNT9_MAX_MSDU_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG4_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT9_MAX_MSDU_CNT_MASK     0x1F000000                // MAX_MSDU_CNT[28..24]
#define WED_PAO_MON_AMSDU_ENG4_CNT9_MAX_MSDU_CNT_SHFT     24
#define WED_PAO_MON_AMSDU_ENG4_CNT9_MAX_BUF_NUM_OFFSET    WED_PAO_MON_AMSDU_ENG4_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT9_MAX_BUF_NUM_MASK      0x001F0000                // MAX_BUF_NUM[20..16]
#define WED_PAO_MON_AMSDU_ENG4_CNT9_MAX_BUF_NUM_SHFT      16
#define WED_PAO_MON_AMSDU_ENG4_CNT9_PORTQ_CNT_OFFSET      WED_PAO_MON_AMSDU_ENG4_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT9_PORTQ_CNT_MASK        0x00000FFF                // PORTQ_CNT[11..0]
#define WED_PAO_MON_AMSDU_ENG4_CNT9_PORTQ_CNT_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_CNT10 (0x15015800 + 0x3DC)---

    SEARCH_MAX_ENQ_CNT[31..0]    - (RO) AMSDU_ENG search to MAX and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_CNT10_SEARCH_MAX_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG4_CNT10_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT10_SEARCH_MAX_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG4_CNT10_SEARCH_MAX_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_CNT11 (0x15015800 + 0x3E0)---

    SEARCH_HEAD_ENQ_CNT[31..0]   - (RO) AMSDU_ENG search to HEAD and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_CNT11_SEARCH_HEAD_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG4_CNT11_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT11_SEARCH_HEAD_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_HEAD_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG4_CNT11_SEARCH_HEAD_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_CNT12 (0x15015800 + 0x3E4)---

    MAX_ENQ_CNT[31..0]           - (RO) AMSDU_ENG Max en-queue ccount (BUF_NUM or MSDU_CNT or TX_BYTE_CNT reach max)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_CNT12_MAX_ENQ_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG4_CNT12_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT12_MAX_ENQ_CNT_MASK     0xFFFFFFFF                // MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG4_CNT12_MAX_ENQ_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_CNT13 (0x15015800 + 0x3E8)---

    WCID_HIT_ENQ_CNT[31..0]      - (RO) AMSDU_ENG WCID hit (other fields miss) en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_CNT13_WCID_HIT_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG4_CNT13_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_CNT13_WCID_HIT_ENQ_CNT_MASK 0xFFFFFFFF                // WCID_HIT_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG4_CNT13_WCID_HIT_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_PTR0 (0x15015800 + 0x3EC)---

    PORT_END[10..0]              - (RO) AMSDU_ENG current TAIL pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    PORT_START[26..16]           - (RO) AMSDU_ENG current HEAD pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_PTR0_PORT_START_OFFSET     WED_PAO_MON_AMSDU_ENG4_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_PTR0_PORT_START_MASK       0x07FF0000                // PORT_START[26..16]
#define WED_PAO_MON_AMSDU_ENG4_PTR0_PORT_START_SHFT       16
#define WED_PAO_MON_AMSDU_ENG4_PTR0_PORT_END_OFFSET       WED_PAO_MON_AMSDU_ENG4_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_PTR0_PORT_END_MASK         0x000007FF                // PORT_END[10..0]
#define WED_PAO_MON_AMSDU_ENG4_PTR0_PORT_END_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_PTR1 (0x15015800 + 0x3F0)---

    CUR_HEAD_PTR[10..0]          - (RO) AMSDU_ENG current HEAD pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_WRIT_PTR[26..16]         - (RO) AMSDU_ENG current Write pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_PTR1_CUR_WRIT_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG4_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_PTR1_CUR_WRIT_PTR_MASK     0x07FF0000                // CUR_WRIT_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG4_PTR1_CUR_WRIT_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG4_PTR1_CUR_HEAD_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG4_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_PTR1_CUR_HEAD_PTR_MASK     0x000007FF                // CUR_HEAD_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG4_PTR1_CUR_HEAD_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG4_PTR2 (0x15015800 + 0x3F4)---

    CUR_QGPP_PTR[10..0]          - (RO) AMSDU_ENG current SEARCH pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_TAIL_PTR[26..16]         - (RO) AMSDU_ENG current TAIL pointer (internal)
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG4_PTR2_CUR_TAIL_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG4_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_PTR2_CUR_TAIL_PTR_MASK     0x07FF0000                // CUR_TAIL_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG4_PTR2_CUR_TAIL_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG4_PTR2_CUR_QGPP_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG4_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG4_PTR2_CUR_QGPP_PTR_MASK     0x000007FF                // CUR_QGPP_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG4_PTR2_CUR_QGPP_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5 (0x15015800 + 0x400)---

    FSM[16..0]                   - (RO) AMSDU_ENG FSM
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_FSM_OFFSET                 WED_PAO_MON_AMSDU_ENG5_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_FSM_MASK                   0x0001FFFF                // FSM[16..0]
#define WED_PAO_MON_AMSDU_ENG5_FSM_SHFT                   0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_CNT0 (0x15015800 + 0x404)---

    SEARCH_ADD_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Add cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_CNT0_SEARCH_ADD_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG5_CNT0_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT0_SEARCH_ADD_CYCLE_MASK 0xFFFFFFFF                // SEARCH_ADD_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG5_CNT0_SEARCH_ADD_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_CNT1 (0x15015800 + 0x408)---

    SEARCH_MRG_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Merge cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_CNT1_SEARCH_MRG_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG5_CNT1_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT1_SEARCH_MRG_CYCLE_MASK 0xFFFFFFFF                // SEARCH_MRG_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG5_CNT1_SEARCH_MRG_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_CNT2 (0x15015800 + 0x40C)---

    DEQ_CYCLE[31..0]             - (RO) AMSDU_ENG do De-queue cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_CNT2_DEQ_CYCLE_OFFSET      WED_PAO_MON_AMSDU_ENG5_CNT2_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT2_DEQ_CYCLE_MASK        0xFFFFFFFF                // DEQ_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG5_CNT2_DEQ_CYCLE_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_CNT3 (0x15015800 + 0x410)---

    DMAD_CNT[31..0]              - (RO) AMSDU_ENG DMAD counter, how many DMAD input

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_CNT3_DMAD_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG5_CNT3_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT3_DMAD_CNT_MASK         0xFFFFFFFF                // DMAD_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG5_CNT3_DMAD_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_CNT4 (0x15015800 + 0x414)---

    QFPL_CNT[31..0]              - (RO) AMSDU_ENG QFPL counter, requeus free page times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_CNT4_QFPL_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG5_CNT4_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT4_QFPL_CNT_MASK         0xFFFFFFFF                // QFPL_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG5_CNT4_QFPL_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_CNT5 (0x15015800 + 0x418)---

    QENI_CNT[31..0]              - (RO) AMSDU_ENG QENI counter, en-queue to input queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_CNT5_QENI_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG5_CNT5_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT5_QENI_CNT_MASK         0xFFFFFFFF                // QENI_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG5_CNT5_QENI_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_CNT6 (0x15015800 + 0x41C)---

    QENO_CNT[31..0]              - (RO) AMSDU_ENG QENO counter, en-queue to output queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_CNT6_QENO_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG5_CNT6_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT6_QENO_CNT_MASK         0xFFFFFFFF                // QENO_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG5_CNT6_QENO_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_CNT7 (0x15015800 + 0x420)---

    MERG_CNT[31..0]              - (RO) AMSDU_ENG MERG counter, how many DMADs are merged

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_CNT7_MERG_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG5_CNT7_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT7_MERG_CNT_MASK         0xFFFFFFFF                // MERG_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG5_CNT7_MERG_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_CNT8 (0x15015800 + 0x424)---

    MAX_QGPP_CNT[10..0]          - (RO) AMSDU_ENG Max search times
    RESERVED11[15..11]           - (RO) Reserved bits
    MAX_PL_CNT[27..16]           - (RO) AMSDU_ENG Max entry usage
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_CNT8_MAX_PL_CNT_OFFSET     WED_PAO_MON_AMSDU_ENG5_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT8_MAX_PL_CNT_MASK       0x0FFF0000                // MAX_PL_CNT[27..16]
#define WED_PAO_MON_AMSDU_ENG5_CNT8_MAX_PL_CNT_SHFT       16
#define WED_PAO_MON_AMSDU_ENG5_CNT8_MAX_QGPP_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG5_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT8_MAX_QGPP_CNT_MASK     0x000007FF                // MAX_QGPP_CNT[10..0]
#define WED_PAO_MON_AMSDU_ENG5_CNT8_MAX_QGPP_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_CNT9 (0x15015800 + 0x428)---

    PORTQ_CNT[11..0]             - (RO) AMSDU_ENG current entry usage
    RESERVED12[15..12]           - (RO) Reserved bits
    MAX_BUF_NUM[20..16]          - (RO) AMSDU_ENG Max merge TXPs count
    RESERVED21[23..21]           - (RO) Reserved bits
    MAX_MSDU_CNT[28..24]         - (RO) AMSDU_ENG Max merge MSDUs count
    RESERVED29[31..29]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_CNT9_MAX_MSDU_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG5_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT9_MAX_MSDU_CNT_MASK     0x1F000000                // MAX_MSDU_CNT[28..24]
#define WED_PAO_MON_AMSDU_ENG5_CNT9_MAX_MSDU_CNT_SHFT     24
#define WED_PAO_MON_AMSDU_ENG5_CNT9_MAX_BUF_NUM_OFFSET    WED_PAO_MON_AMSDU_ENG5_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT9_MAX_BUF_NUM_MASK      0x001F0000                // MAX_BUF_NUM[20..16]
#define WED_PAO_MON_AMSDU_ENG5_CNT9_MAX_BUF_NUM_SHFT      16
#define WED_PAO_MON_AMSDU_ENG5_CNT9_PORTQ_CNT_OFFSET      WED_PAO_MON_AMSDU_ENG5_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT9_PORTQ_CNT_MASK        0x00000FFF                // PORTQ_CNT[11..0]
#define WED_PAO_MON_AMSDU_ENG5_CNT9_PORTQ_CNT_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_CNT10 (0x15015800 + 0x42C)---

    SEARCH_MAX_ENQ_CNT[31..0]    - (RO) AMSDU_ENG search to MAX and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_CNT10_SEARCH_MAX_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG5_CNT10_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT10_SEARCH_MAX_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG5_CNT10_SEARCH_MAX_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_CNT11 (0x15015800 + 0x430)---

    SEARCH_HEAD_ENQ_CNT[31..0]   - (RO) AMSDU_ENG search to HEAD and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_CNT11_SEARCH_HEAD_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG5_CNT11_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT11_SEARCH_HEAD_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_HEAD_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG5_CNT11_SEARCH_HEAD_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_CNT12 (0x15015800 + 0x434)---

    MAX_ENQ_CNT[31..0]           - (RO) AMSDU_ENG Max en-queue ccount (BUF_NUM or MSDU_CNT or TX_BYTE_CNT reach max)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_CNT12_MAX_ENQ_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG5_CNT12_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT12_MAX_ENQ_CNT_MASK     0xFFFFFFFF                // MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG5_CNT12_MAX_ENQ_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_CNT13 (0x15015800 + 0x438)---

    WCID_HIT_ENQ_CNT[31..0]      - (RO) AMSDU_ENG WCID hit (other fields miss) en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_CNT13_WCID_HIT_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG5_CNT13_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_CNT13_WCID_HIT_ENQ_CNT_MASK 0xFFFFFFFF                // WCID_HIT_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG5_CNT13_WCID_HIT_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_PTR0 (0x15015800 + 0x43C)---

    PORT_END[10..0]              - (RO) AMSDU_ENG current TAIL pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    PORT_START[26..16]           - (RO) AMSDU_ENG current HEAD pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_PTR0_PORT_START_OFFSET     WED_PAO_MON_AMSDU_ENG5_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_PTR0_PORT_START_MASK       0x07FF0000                // PORT_START[26..16]
#define WED_PAO_MON_AMSDU_ENG5_PTR0_PORT_START_SHFT       16
#define WED_PAO_MON_AMSDU_ENG5_PTR0_PORT_END_OFFSET       WED_PAO_MON_AMSDU_ENG5_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_PTR0_PORT_END_MASK         0x000007FF                // PORT_END[10..0]
#define WED_PAO_MON_AMSDU_ENG5_PTR0_PORT_END_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_PTR1 (0x15015800 + 0x440)---

    CUR_HEAD_PTR[10..0]          - (RO) AMSDU_ENG current HEAD pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_WRIT_PTR[26..16]         - (RO) AMSDU_ENG current Write pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_PTR1_CUR_WRIT_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG5_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_PTR1_CUR_WRIT_PTR_MASK     0x07FF0000                // CUR_WRIT_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG5_PTR1_CUR_WRIT_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG5_PTR1_CUR_HEAD_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG5_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_PTR1_CUR_HEAD_PTR_MASK     0x000007FF                // CUR_HEAD_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG5_PTR1_CUR_HEAD_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG5_PTR2 (0x15015800 + 0x444)---

    CUR_QGPP_PTR[10..0]          - (RO) AMSDU_ENG current SEARCH pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_TAIL_PTR[26..16]         - (RO) AMSDU_ENG current TAIL pointer (internal)
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG5_PTR2_CUR_TAIL_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG5_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_PTR2_CUR_TAIL_PTR_MASK     0x07FF0000                // CUR_TAIL_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG5_PTR2_CUR_TAIL_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG5_PTR2_CUR_QGPP_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG5_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG5_PTR2_CUR_QGPP_PTR_MASK     0x000007FF                // CUR_QGPP_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG5_PTR2_CUR_QGPP_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6 (0x15015800 + 0x450)---

    FSM[16..0]                   - (RO) AMSDU_ENG FSM
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_FSM_OFFSET                 WED_PAO_MON_AMSDU_ENG6_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_FSM_MASK                   0x0001FFFF                // FSM[16..0]
#define WED_PAO_MON_AMSDU_ENG6_FSM_SHFT                   0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_CNT0 (0x15015800 + 0x454)---

    SEARCH_ADD_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Add cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_CNT0_SEARCH_ADD_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG6_CNT0_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT0_SEARCH_ADD_CYCLE_MASK 0xFFFFFFFF                // SEARCH_ADD_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG6_CNT0_SEARCH_ADD_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_CNT1 (0x15015800 + 0x458)---

    SEARCH_MRG_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Merge cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_CNT1_SEARCH_MRG_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG6_CNT1_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT1_SEARCH_MRG_CYCLE_MASK 0xFFFFFFFF                // SEARCH_MRG_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG6_CNT1_SEARCH_MRG_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_CNT2 (0x15015800 + 0x45C)---

    DEQ_CYCLE[31..0]             - (RO) AMSDU_ENG do De-queue cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_CNT2_DEQ_CYCLE_OFFSET      WED_PAO_MON_AMSDU_ENG6_CNT2_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT2_DEQ_CYCLE_MASK        0xFFFFFFFF                // DEQ_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG6_CNT2_DEQ_CYCLE_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_CNT3 (0x15015800 + 0x460)---

    DMAD_CNT[31..0]              - (RO) AMSDU_ENG DMAD counter, how many DMAD input

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_CNT3_DMAD_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG6_CNT3_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT3_DMAD_CNT_MASK         0xFFFFFFFF                // DMAD_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG6_CNT3_DMAD_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_CNT4 (0x15015800 + 0x464)---

    QFPL_CNT[31..0]              - (RO) AMSDU_ENG QFPL counter, requeus free page times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_CNT4_QFPL_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG6_CNT4_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT4_QFPL_CNT_MASK         0xFFFFFFFF                // QFPL_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG6_CNT4_QFPL_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_CNT5 (0x15015800 + 0x468)---

    QENI_CNT[31..0]              - (RO) AMSDU_ENG QENI counter, en-queue to input queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_CNT5_QENI_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG6_CNT5_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT5_QENI_CNT_MASK         0xFFFFFFFF                // QENI_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG6_CNT5_QENI_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_CNT6 (0x15015800 + 0x46C)---

    QENO_CNT[31..0]              - (RO) AMSDU_ENG QENO counter, en-queue to output queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_CNT6_QENO_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG6_CNT6_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT6_QENO_CNT_MASK         0xFFFFFFFF                // QENO_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG6_CNT6_QENO_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_CNT7 (0x15015800 + 0x470)---

    MERG_CNT[31..0]              - (RO) AMSDU_ENG MERG counter, how many DMADs are merged

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_CNT7_MERG_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG6_CNT7_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT7_MERG_CNT_MASK         0xFFFFFFFF                // MERG_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG6_CNT7_MERG_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_CNT8 (0x15015800 + 0x474)---

    MAX_QGPP_CNT[10..0]          - (RO) AMSDU_ENG Max search times
    RESERVED11[15..11]           - (RO) Reserved bits
    MAX_PL_CNT[27..16]           - (RO) AMSDU_ENG Max entry usage
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_CNT8_MAX_PL_CNT_OFFSET     WED_PAO_MON_AMSDU_ENG6_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT8_MAX_PL_CNT_MASK       0x0FFF0000                // MAX_PL_CNT[27..16]
#define WED_PAO_MON_AMSDU_ENG6_CNT8_MAX_PL_CNT_SHFT       16
#define WED_PAO_MON_AMSDU_ENG6_CNT8_MAX_QGPP_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG6_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT8_MAX_QGPP_CNT_MASK     0x000007FF                // MAX_QGPP_CNT[10..0]
#define WED_PAO_MON_AMSDU_ENG6_CNT8_MAX_QGPP_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_CNT9 (0x15015800 + 0x478)---

    PORTQ_CNT[11..0]             - (RO) AMSDU_ENG current entry usage
    RESERVED12[15..12]           - (RO) Reserved bits
    MAX_BUF_NUM[20..16]          - (RO) AMSDU_ENG Max merge TXPs count
    RESERVED21[23..21]           - (RO) Reserved bits
    MAX_MSDU_CNT[28..24]         - (RO) AMSDU_ENG Max merge MSDUs count
    RESERVED29[31..29]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_CNT9_MAX_MSDU_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG6_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT9_MAX_MSDU_CNT_MASK     0x1F000000                // MAX_MSDU_CNT[28..24]
#define WED_PAO_MON_AMSDU_ENG6_CNT9_MAX_MSDU_CNT_SHFT     24
#define WED_PAO_MON_AMSDU_ENG6_CNT9_MAX_BUF_NUM_OFFSET    WED_PAO_MON_AMSDU_ENG6_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT9_MAX_BUF_NUM_MASK      0x001F0000                // MAX_BUF_NUM[20..16]
#define WED_PAO_MON_AMSDU_ENG6_CNT9_MAX_BUF_NUM_SHFT      16
#define WED_PAO_MON_AMSDU_ENG6_CNT9_PORTQ_CNT_OFFSET      WED_PAO_MON_AMSDU_ENG6_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT9_PORTQ_CNT_MASK        0x00000FFF                // PORTQ_CNT[11..0]
#define WED_PAO_MON_AMSDU_ENG6_CNT9_PORTQ_CNT_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_CNT10 (0x15015800 + 0x47C)---

    SEARCH_MAX_ENQ_CNT[31..0]    - (RO) AMSDU_ENG search to MAX and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_CNT10_SEARCH_MAX_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG6_CNT10_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT10_SEARCH_MAX_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG6_CNT10_SEARCH_MAX_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_CNT11 (0x15015800 + 0x480)---

    SEARCH_HEAD_ENQ_CNT[31..0]   - (RO) AMSDU_ENG search to HEAD and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_CNT11_SEARCH_HEAD_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG6_CNT11_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT11_SEARCH_HEAD_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_HEAD_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG6_CNT11_SEARCH_HEAD_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_CNT12 (0x15015800 + 0x484)---

    MAX_ENQ_CNT[31..0]           - (RO) AMSDU_ENG Max en-queue ccount (BUF_NUM or MSDU_CNT or TX_BYTE_CNT reach max)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_CNT12_MAX_ENQ_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG6_CNT12_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT12_MAX_ENQ_CNT_MASK     0xFFFFFFFF                // MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG6_CNT12_MAX_ENQ_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_CNT13 (0x15015800 + 0x488)---

    WCID_HIT_ENQ_CNT[31..0]      - (RO) AMSDU_ENG WCID hit (other fields miss) en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_CNT13_WCID_HIT_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG6_CNT13_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_CNT13_WCID_HIT_ENQ_CNT_MASK 0xFFFFFFFF                // WCID_HIT_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG6_CNT13_WCID_HIT_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_PTR0 (0x15015800 + 0x48C)---

    PORT_END[10..0]              - (RO) AMSDU_ENG current TAIL pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    PORT_START[26..16]           - (RO) AMSDU_ENG current HEAD pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_PTR0_PORT_START_OFFSET     WED_PAO_MON_AMSDU_ENG6_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_PTR0_PORT_START_MASK       0x07FF0000                // PORT_START[26..16]
#define WED_PAO_MON_AMSDU_ENG6_PTR0_PORT_START_SHFT       16
#define WED_PAO_MON_AMSDU_ENG6_PTR0_PORT_END_OFFSET       WED_PAO_MON_AMSDU_ENG6_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_PTR0_PORT_END_MASK         0x000007FF                // PORT_END[10..0]
#define WED_PAO_MON_AMSDU_ENG6_PTR0_PORT_END_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_PTR1 (0x15015800 + 0x490)---

    CUR_HEAD_PTR[10..0]          - (RO) AMSDU_ENG current HEAD pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_WRIT_PTR[26..16]         - (RO) AMSDU_ENG current Write pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_PTR1_CUR_WRIT_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG6_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_PTR1_CUR_WRIT_PTR_MASK     0x07FF0000                // CUR_WRIT_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG6_PTR1_CUR_WRIT_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG6_PTR1_CUR_HEAD_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG6_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_PTR1_CUR_HEAD_PTR_MASK     0x000007FF                // CUR_HEAD_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG6_PTR1_CUR_HEAD_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG6_PTR2 (0x15015800 + 0x494)---

    CUR_QGPP_PTR[10..0]          - (RO) AMSDU_ENG current SEARCH pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_TAIL_PTR[26..16]         - (RO) AMSDU_ENG current TAIL pointer (internal)
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG6_PTR2_CUR_TAIL_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG6_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_PTR2_CUR_TAIL_PTR_MASK     0x07FF0000                // CUR_TAIL_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG6_PTR2_CUR_TAIL_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG6_PTR2_CUR_QGPP_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG6_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG6_PTR2_CUR_QGPP_PTR_MASK     0x000007FF                // CUR_QGPP_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG6_PTR2_CUR_QGPP_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7 (0x15015800 + 0x4A0)---

    FSM[16..0]                   - (RO) AMSDU_ENG FSM
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_FSM_OFFSET                 WED_PAO_MON_AMSDU_ENG7_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_FSM_MASK                   0x0001FFFF                // FSM[16..0]
#define WED_PAO_MON_AMSDU_ENG7_FSM_SHFT                   0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_CNT0 (0x15015800 + 0x4A4)---

    SEARCH_ADD_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Add cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_CNT0_SEARCH_ADD_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG7_CNT0_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT0_SEARCH_ADD_CYCLE_MASK 0xFFFFFFFF                // SEARCH_ADD_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG7_CNT0_SEARCH_ADD_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_CNT1 (0x15015800 + 0x4A8)---

    SEARCH_MRG_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Merge cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_CNT1_SEARCH_MRG_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG7_CNT1_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT1_SEARCH_MRG_CYCLE_MASK 0xFFFFFFFF                // SEARCH_MRG_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG7_CNT1_SEARCH_MRG_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_CNT2 (0x15015800 + 0x4AC)---

    DEQ_CYCLE[31..0]             - (RO) AMSDU_ENG do De-queue cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_CNT2_DEQ_CYCLE_OFFSET      WED_PAO_MON_AMSDU_ENG7_CNT2_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT2_DEQ_CYCLE_MASK        0xFFFFFFFF                // DEQ_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG7_CNT2_DEQ_CYCLE_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_CNT3 (0x15015800 + 0x4B0)---

    DMAD_CNT[31..0]              - (RO) AMSDU_ENG DMAD counter, how many DMAD input

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_CNT3_DMAD_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG7_CNT3_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT3_DMAD_CNT_MASK         0xFFFFFFFF                // DMAD_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG7_CNT3_DMAD_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_CNT4 (0x15015800 + 0x4B4)---

    QFPL_CNT[31..0]              - (RO) AMSDU_ENG QFPL counter, requeus free page times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_CNT4_QFPL_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG7_CNT4_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT4_QFPL_CNT_MASK         0xFFFFFFFF                // QFPL_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG7_CNT4_QFPL_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_CNT5 (0x15015800 + 0x4B8)---

    QENI_CNT[31..0]              - (RO) AMSDU_ENG QENI counter, en-queue to input queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_CNT5_QENI_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG7_CNT5_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT5_QENI_CNT_MASK         0xFFFFFFFF                // QENI_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG7_CNT5_QENI_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_CNT6 (0x15015800 + 0x4BC)---

    QENO_CNT[31..0]              - (RO) AMSDU_ENG QENO counter, en-queue to output queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_CNT6_QENO_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG7_CNT6_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT6_QENO_CNT_MASK         0xFFFFFFFF                // QENO_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG7_CNT6_QENO_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_CNT7 (0x15015800 + 0x4C0)---

    MERG_CNT[31..0]              - (RO) AMSDU_ENG MERG counter, how many DMADs are merged

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_CNT7_MERG_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG7_CNT7_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT7_MERG_CNT_MASK         0xFFFFFFFF                // MERG_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG7_CNT7_MERG_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_CNT8 (0x15015800 + 0x4C4)---

    MAX_QGPP_CNT[10..0]          - (RO) AMSDU_ENG Max search times
    RESERVED11[15..11]           - (RO) Reserved bits
    MAX_PL_CNT[27..16]           - (RO) AMSDU_ENG Max entry usage
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_CNT8_MAX_PL_CNT_OFFSET     WED_PAO_MON_AMSDU_ENG7_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT8_MAX_PL_CNT_MASK       0x0FFF0000                // MAX_PL_CNT[27..16]
#define WED_PAO_MON_AMSDU_ENG7_CNT8_MAX_PL_CNT_SHFT       16
#define WED_PAO_MON_AMSDU_ENG7_CNT8_MAX_QGPP_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG7_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT8_MAX_QGPP_CNT_MASK     0x000007FF                // MAX_QGPP_CNT[10..0]
#define WED_PAO_MON_AMSDU_ENG7_CNT8_MAX_QGPP_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_CNT9 (0x15015800 + 0x4C8)---

    PORTQ_CNT[11..0]             - (RO) AMSDU_ENG current entry usage
    RESERVED12[15..12]           - (RO) Reserved bits
    MAX_BUF_NUM[20..16]          - (RO) AMSDU_ENG Max merge TXPs count
    RESERVED21[23..21]           - (RO) Reserved bits
    MAX_MSDU_CNT[28..24]         - (RO) AMSDU_ENG Max merge MSDUs count
    RESERVED29[31..29]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_CNT9_MAX_MSDU_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG7_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT9_MAX_MSDU_CNT_MASK     0x1F000000                // MAX_MSDU_CNT[28..24]
#define WED_PAO_MON_AMSDU_ENG7_CNT9_MAX_MSDU_CNT_SHFT     24
#define WED_PAO_MON_AMSDU_ENG7_CNT9_MAX_BUF_NUM_OFFSET    WED_PAO_MON_AMSDU_ENG7_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT9_MAX_BUF_NUM_MASK      0x001F0000                // MAX_BUF_NUM[20..16]
#define WED_PAO_MON_AMSDU_ENG7_CNT9_MAX_BUF_NUM_SHFT      16
#define WED_PAO_MON_AMSDU_ENG7_CNT9_PORTQ_CNT_OFFSET      WED_PAO_MON_AMSDU_ENG7_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT9_PORTQ_CNT_MASK        0x00000FFF                // PORTQ_CNT[11..0]
#define WED_PAO_MON_AMSDU_ENG7_CNT9_PORTQ_CNT_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_CNT10 (0x15015800 + 0x4CC)---

    SEARCH_MAX_ENQ_CNT[31..0]    - (RO) AMSDU_ENG search to MAX and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_CNT10_SEARCH_MAX_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG7_CNT10_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT10_SEARCH_MAX_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG7_CNT10_SEARCH_MAX_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_CNT11 (0x15015800 + 0x4D0)---

    SEARCH_HEAD_ENQ_CNT[31..0]   - (RO) AMSDU_ENG search to HEAD and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_CNT11_SEARCH_HEAD_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG7_CNT11_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT11_SEARCH_HEAD_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_HEAD_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG7_CNT11_SEARCH_HEAD_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_CNT12 (0x15015800 + 0x4D4)---

    MAX_ENQ_CNT[31..0]           - (RO) AMSDU_ENG Max en-queue ccount (BUF_NUM or MSDU_CNT or TX_BYTE_CNT reach max)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_CNT12_MAX_ENQ_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG7_CNT12_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT12_MAX_ENQ_CNT_MASK     0xFFFFFFFF                // MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG7_CNT12_MAX_ENQ_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_CNT13 (0x15015800 + 0x4D8)---

    WCID_HIT_ENQ_CNT[31..0]      - (RO) AMSDU_ENG WCID hit (other fields miss) en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_CNT13_WCID_HIT_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG7_CNT13_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_CNT13_WCID_HIT_ENQ_CNT_MASK 0xFFFFFFFF                // WCID_HIT_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG7_CNT13_WCID_HIT_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_PTR0 (0x15015800 + 0x4DC)---

    PORT_END[10..0]              - (RO) AMSDU_ENG current TAIL pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    PORT_START[26..16]           - (RO) AMSDU_ENG current HEAD pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_PTR0_PORT_START_OFFSET     WED_PAO_MON_AMSDU_ENG7_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_PTR0_PORT_START_MASK       0x07FF0000                // PORT_START[26..16]
#define WED_PAO_MON_AMSDU_ENG7_PTR0_PORT_START_SHFT       16
#define WED_PAO_MON_AMSDU_ENG7_PTR0_PORT_END_OFFSET       WED_PAO_MON_AMSDU_ENG7_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_PTR0_PORT_END_MASK         0x000007FF                // PORT_END[10..0]
#define WED_PAO_MON_AMSDU_ENG7_PTR0_PORT_END_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_PTR1 (0x15015800 + 0x4E0)---

    CUR_HEAD_PTR[10..0]          - (RO) AMSDU_ENG current HEAD pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_WRIT_PTR[26..16]         - (RO) AMSDU_ENG current Write pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_PTR1_CUR_WRIT_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG7_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_PTR1_CUR_WRIT_PTR_MASK     0x07FF0000                // CUR_WRIT_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG7_PTR1_CUR_WRIT_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG7_PTR1_CUR_HEAD_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG7_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_PTR1_CUR_HEAD_PTR_MASK     0x000007FF                // CUR_HEAD_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG7_PTR1_CUR_HEAD_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG7_PTR2 (0x15015800 + 0x4E4)---

    CUR_QGPP_PTR[10..0]          - (RO) AMSDU_ENG current SEARCH pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_TAIL_PTR[26..16]         - (RO) AMSDU_ENG current TAIL pointer (internal)
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG7_PTR2_CUR_TAIL_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG7_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_PTR2_CUR_TAIL_PTR_MASK     0x07FF0000                // CUR_TAIL_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG7_PTR2_CUR_TAIL_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG7_PTR2_CUR_QGPP_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG7_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG7_PTR2_CUR_QGPP_PTR_MASK     0x000007FF                // CUR_QGPP_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG7_PTR2_CUR_QGPP_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8 (0x15015800 + 0x4F0)---

    FSM[16..0]                   - (RO) AMSDU_ENG FSM
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_FSM_OFFSET                 WED_PAO_MON_AMSDU_ENG8_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_FSM_MASK                   0x0001FFFF                // FSM[16..0]
#define WED_PAO_MON_AMSDU_ENG8_FSM_SHFT                   0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_CNT0 (0x15015800 + 0x4F4)---

    SEARCH_ADD_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Add cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_CNT0_SEARCH_ADD_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG8_CNT0_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT0_SEARCH_ADD_CYCLE_MASK 0xFFFFFFFF                // SEARCH_ADD_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG8_CNT0_SEARCH_ADD_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_CNT1 (0x15015800 + 0x4F8)---

    SEARCH_MRG_CYCLE[31..0]      - (RO) AMSDU_ENG do Search -> Merge cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_CNT1_SEARCH_MRG_CYCLE_OFFSET WED_PAO_MON_AMSDU_ENG8_CNT1_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT1_SEARCH_MRG_CYCLE_MASK 0xFFFFFFFF                // SEARCH_MRG_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG8_CNT1_SEARCH_MRG_CYCLE_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_CNT2 (0x15015800 + 0x4FC)---

    DEQ_CYCLE[31..0]             - (RO) AMSDU_ENG do De-queue cycle count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_CNT2_DEQ_CYCLE_OFFSET      WED_PAO_MON_AMSDU_ENG8_CNT2_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT2_DEQ_CYCLE_MASK        0xFFFFFFFF                // DEQ_CYCLE[31..0]
#define WED_PAO_MON_AMSDU_ENG8_CNT2_DEQ_CYCLE_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_CNT3 (0x15015800 + 0x500)---

    DMAD_CNT[31..0]              - (RO) AMSDU_ENG DMAD counter, how many DMAD input

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_CNT3_DMAD_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG8_CNT3_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT3_DMAD_CNT_MASK         0xFFFFFFFF                // DMAD_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG8_CNT3_DMAD_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_CNT4 (0x15015800 + 0x504)---

    QFPL_CNT[31..0]              - (RO) AMSDU_ENG QFPL counter, requeus free page times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_CNT4_QFPL_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG8_CNT4_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT4_QFPL_CNT_MASK         0xFFFFFFFF                // QFPL_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG8_CNT4_QFPL_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_CNT5 (0x15015800 + 0x508)---

    QENI_CNT[31..0]              - (RO) AMSDU_ENG QENI counter, en-queue to input queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_CNT5_QENI_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG8_CNT5_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT5_QENI_CNT_MASK         0xFFFFFFFF                // QENI_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG8_CNT5_QENI_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_CNT6 (0x15015800 + 0x50C)---

    QENO_CNT[31..0]              - (RO) AMSDU_ENG QENO counter, en-queue to output queue times

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_CNT6_QENO_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG8_CNT6_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT6_QENO_CNT_MASK         0xFFFFFFFF                // QENO_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG8_CNT6_QENO_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_CNT7 (0x15015800 + 0x510)---

    MERG_CNT[31..0]              - (RO) AMSDU_ENG MERG counter, how many DMADs are merged

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_CNT7_MERG_CNT_OFFSET       WED_PAO_MON_AMSDU_ENG8_CNT7_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT7_MERG_CNT_MASK         0xFFFFFFFF                // MERG_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG8_CNT7_MERG_CNT_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_CNT8 (0x15015800 + 0x514)---

    MAX_QGPP_CNT[10..0]          - (RO) AMSDU_ENG Max search times
    RESERVED11[15..11]           - (RO) Reserved bits
    MAX_PL_CNT[27..16]           - (RO) AMSDU_ENG Max entry usage
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_CNT8_MAX_PL_CNT_OFFSET     WED_PAO_MON_AMSDU_ENG8_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT8_MAX_PL_CNT_MASK       0x0FFF0000                // MAX_PL_CNT[27..16]
#define WED_PAO_MON_AMSDU_ENG8_CNT8_MAX_PL_CNT_SHFT       16
#define WED_PAO_MON_AMSDU_ENG8_CNT8_MAX_QGPP_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG8_CNT8_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT8_MAX_QGPP_CNT_MASK     0x000007FF                // MAX_QGPP_CNT[10..0]
#define WED_PAO_MON_AMSDU_ENG8_CNT8_MAX_QGPP_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_CNT9 (0x15015800 + 0x518)---

    PORTQ_CNT[11..0]             - (RO) AMSDU_ENG current entry usage
    RESERVED12[15..12]           - (RO) Reserved bits
    MAX_BUF_NUM[20..16]          - (RO) AMSDU_ENG Max merge TXPs count
    RESERVED21[23..21]           - (RO) Reserved bits
    MAX_MSDU_CNT[28..24]         - (RO) AMSDU_ENG Max merge MSDUs count
    RESERVED29[31..29]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_CNT9_MAX_MSDU_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG8_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT9_MAX_MSDU_CNT_MASK     0x1F000000                // MAX_MSDU_CNT[28..24]
#define WED_PAO_MON_AMSDU_ENG8_CNT9_MAX_MSDU_CNT_SHFT     24
#define WED_PAO_MON_AMSDU_ENG8_CNT9_MAX_BUF_NUM_OFFSET    WED_PAO_MON_AMSDU_ENG8_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT9_MAX_BUF_NUM_MASK      0x001F0000                // MAX_BUF_NUM[20..16]
#define WED_PAO_MON_AMSDU_ENG8_CNT9_MAX_BUF_NUM_SHFT      16
#define WED_PAO_MON_AMSDU_ENG8_CNT9_PORTQ_CNT_OFFSET      WED_PAO_MON_AMSDU_ENG8_CNT9_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT9_PORTQ_CNT_MASK        0x00000FFF                // PORTQ_CNT[11..0]
#define WED_PAO_MON_AMSDU_ENG8_CNT9_PORTQ_CNT_SHFT        0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_CNT10 (0x15015800 + 0x51C)---

    SEARCH_MAX_ENQ_CNT[31..0]    - (RO) AMSDU_ENG search to MAX and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_CNT10_SEARCH_MAX_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG8_CNT10_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT10_SEARCH_MAX_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG8_CNT10_SEARCH_MAX_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_CNT11 (0x15015800 + 0x520)---

    SEARCH_HEAD_ENQ_CNT[31..0]   - (RO) AMSDU_ENG search to HEAD and en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_CNT11_SEARCH_HEAD_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG8_CNT11_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT11_SEARCH_HEAD_ENQ_CNT_MASK 0xFFFFFFFF                // SEARCH_HEAD_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG8_CNT11_SEARCH_HEAD_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_CNT12 (0x15015800 + 0x524)---

    MAX_ENQ_CNT[31..0]           - (RO) AMSDU_ENG Max en-queue ccount (BUF_NUM or MSDU_CNT or TX_BYTE_CNT reach max)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_CNT12_MAX_ENQ_CNT_OFFSET   WED_PAO_MON_AMSDU_ENG8_CNT12_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT12_MAX_ENQ_CNT_MASK     0xFFFFFFFF                // MAX_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG8_CNT12_MAX_ENQ_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_CNT13 (0x15015800 + 0x528)---

    WCID_HIT_ENQ_CNT[31..0]      - (RO) AMSDU_ENG WCID hit (other fields miss) en-queue count

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_CNT13_WCID_HIT_ENQ_CNT_OFFSET WED_PAO_MON_AMSDU_ENG8_CNT13_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_CNT13_WCID_HIT_ENQ_CNT_MASK 0xFFFFFFFF                // WCID_HIT_ENQ_CNT[31..0]
#define WED_PAO_MON_AMSDU_ENG8_CNT13_WCID_HIT_ENQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_PTR0 (0x15015800 + 0x52C)---

    PORT_END[10..0]              - (RO) AMSDU_ENG current TAIL pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    PORT_START[26..16]           - (RO) AMSDU_ENG current HEAD pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_PTR0_PORT_START_OFFSET     WED_PAO_MON_AMSDU_ENG8_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_PTR0_PORT_START_MASK       0x07FF0000                // PORT_START[26..16]
#define WED_PAO_MON_AMSDU_ENG8_PTR0_PORT_START_SHFT       16
#define WED_PAO_MON_AMSDU_ENG8_PTR0_PORT_END_OFFSET       WED_PAO_MON_AMSDU_ENG8_PTR0_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_PTR0_PORT_END_MASK         0x000007FF                // PORT_END[10..0]
#define WED_PAO_MON_AMSDU_ENG8_PTR0_PORT_END_SHFT         0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_PTR1 (0x15015800 + 0x530)---

    CUR_HEAD_PTR[10..0]          - (RO) AMSDU_ENG current HEAD pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_WRIT_PTR[26..16]         - (RO) AMSDU_ENG current Write pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_PTR1_CUR_WRIT_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG8_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_PTR1_CUR_WRIT_PTR_MASK     0x07FF0000                // CUR_WRIT_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG8_PTR1_CUR_WRIT_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG8_PTR1_CUR_HEAD_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG8_PTR1_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_PTR1_CUR_HEAD_PTR_MASK     0x000007FF                // CUR_HEAD_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG8_PTR1_CUR_HEAD_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_ENG8_PTR2 (0x15015800 + 0x534)---

    CUR_QGPP_PTR[10..0]          - (RO) AMSDU_ENG current SEARCH pointer (internal)
    RESERVED11[15..11]           - (RO) Reserved bits
    CUR_TAIL_PTR[26..16]         - (RO) AMSDU_ENG current TAIL pointer (internal)
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_ENG8_PTR2_CUR_TAIL_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG8_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_PTR2_CUR_TAIL_PTR_MASK     0x07FF0000                // CUR_TAIL_PTR[26..16]
#define WED_PAO_MON_AMSDU_ENG8_PTR2_CUR_TAIL_PTR_SHFT     16
#define WED_PAO_MON_AMSDU_ENG8_PTR2_CUR_QGPP_PTR_OFFSET   WED_PAO_MON_AMSDU_ENG8_PTR2_OFFSET
#define WED_PAO_MON_AMSDU_ENG8_PTR2_CUR_QGPP_PTR_MASK     0x000007FF                // CUR_QGPP_PTR[10..0]
#define WED_PAO_MON_AMSDU_ENG8_PTR2_CUR_QGPP_PTR_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_STS0 (0x15015800 + 0x600)---

    STS[31..0]                   - (RO) PAO QMEM status

 =====================================================================================*/
#define WED_PAO_MON_QMEM_STS0_STS_OFFSET                  WED_PAO_MON_QMEM_STS0_OFFSET
#define WED_PAO_MON_QMEM_STS0_STS_MASK                    0xFFFFFFFF                // STS[31..0]
#define WED_PAO_MON_QMEM_STS0_STS_SHFT                    0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_STS1 (0x15015800 + 0x604)---

    STS[31..0]                   - (RO) PAO QMEM status

 =====================================================================================*/
#define WED_PAO_MON_QMEM_STS1_STS_OFFSET                  WED_PAO_MON_QMEM_STS1_OFFSET
#define WED_PAO_MON_QMEM_STS1_STS_MASK                    0xFFFFFFFF                // STS[31..0]
#define WED_PAO_MON_QMEM_STS1_STS_SHFT                    0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_STS2 (0x15015800 + 0x608)---

    STS[31..0]                   - (RO) PAO QMEM status

 =====================================================================================*/
#define WED_PAO_MON_QMEM_STS2_STS_OFFSET                  WED_PAO_MON_QMEM_STS2_OFFSET
#define WED_PAO_MON_QMEM_STS2_STS_MASK                    0xFFFFFFFF                // STS[31..0]
#define WED_PAO_MON_QMEM_STS2_STS_SHFT                    0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_CNT0 (0x15015800 + 0x60C)---

    Q0_PCNT[11..0]               - (RO) PAO QMEM (IS_SP) queue entry count
    RESERVED12[15..12]           - (RO) Reserved bits
    FQ_PCNT[27..16]              - (RO) PAO QMEM Free queue entry count
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_CNT0_FQ_PCNT_OFFSET              WED_PAO_MON_QMEM_CNT0_OFFSET
#define WED_PAO_MON_QMEM_CNT0_FQ_PCNT_MASK                0x0FFF0000                // FQ_PCNT[27..16]
#define WED_PAO_MON_QMEM_CNT0_FQ_PCNT_SHFT                16
#define WED_PAO_MON_QMEM_CNT0_Q0_PCNT_OFFSET              WED_PAO_MON_QMEM_CNT0_OFFSET
#define WED_PAO_MON_QMEM_CNT0_Q0_PCNT_MASK                0x00000FFF                // Q0_PCNT[11..0]
#define WED_PAO_MON_QMEM_CNT0_Q0_PCNT_SHFT                0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_CNT1 (0x15015800 + 0x610)---

    Q2_PCNT[11..0]               - (RO) PAO QMEM (TID=1) queue entry count
    RESERVED12[15..12]           - (RO) Reserved bits
    Q1_PCNT[27..16]              - (RO) PAO QMEM (TID=0) queue entry count
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_CNT1_Q1_PCNT_OFFSET              WED_PAO_MON_QMEM_CNT1_OFFSET
#define WED_PAO_MON_QMEM_CNT1_Q1_PCNT_MASK                0x0FFF0000                // Q1_PCNT[27..16]
#define WED_PAO_MON_QMEM_CNT1_Q1_PCNT_SHFT                16
#define WED_PAO_MON_QMEM_CNT1_Q2_PCNT_OFFSET              WED_PAO_MON_QMEM_CNT1_OFFSET
#define WED_PAO_MON_QMEM_CNT1_Q2_PCNT_MASK                0x00000FFF                // Q2_PCNT[11..0]
#define WED_PAO_MON_QMEM_CNT1_Q2_PCNT_SHFT                0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_CNT2 (0x15015800 + 0x614)---

    Q4_PCNT[11..0]               - (RO) PAO QMEM (TID=3) queue entry count
    RESERVED12[15..12]           - (RO) Reserved bits
    Q3_PCNT[27..16]              - (RO) PAO QMEM (TID=2) queue entry count
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_CNT2_Q3_PCNT_OFFSET              WED_PAO_MON_QMEM_CNT2_OFFSET
#define WED_PAO_MON_QMEM_CNT2_Q3_PCNT_MASK                0x0FFF0000                // Q3_PCNT[27..16]
#define WED_PAO_MON_QMEM_CNT2_Q3_PCNT_SHFT                16
#define WED_PAO_MON_QMEM_CNT2_Q4_PCNT_OFFSET              WED_PAO_MON_QMEM_CNT2_OFFSET
#define WED_PAO_MON_QMEM_CNT2_Q4_PCNT_MASK                0x00000FFF                // Q4_PCNT[11..0]
#define WED_PAO_MON_QMEM_CNT2_Q4_PCNT_SHFT                0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_CNT3 (0x15015800 + 0x618)---

    Q6_PCNT[11..0]               - (RO) PAO QMEM (TID=5) queue entry count
    RESERVED12[15..12]           - (RO) Reserved bits
    Q5_PCNT[27..16]              - (RO) PAO QMEM (TID=4) queue entry count
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_CNT3_Q5_PCNT_OFFSET              WED_PAO_MON_QMEM_CNT3_OFFSET
#define WED_PAO_MON_QMEM_CNT3_Q5_PCNT_MASK                0x0FFF0000                // Q5_PCNT[27..16]
#define WED_PAO_MON_QMEM_CNT3_Q5_PCNT_SHFT                16
#define WED_PAO_MON_QMEM_CNT3_Q6_PCNT_OFFSET              WED_PAO_MON_QMEM_CNT3_OFFSET
#define WED_PAO_MON_QMEM_CNT3_Q6_PCNT_MASK                0x00000FFF                // Q6_PCNT[11..0]
#define WED_PAO_MON_QMEM_CNT3_Q6_PCNT_SHFT                0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_CNT4 (0x15015800 + 0x61C)---

    Q8_PCNT[11..0]               - (RO) PAO QMEM (TID=7) queue entry count
    RESERVED12[15..12]           - (RO) Reserved bits
    Q7_PCNT[27..16]              - (RO) PAO QMEM (TID=6) queue entry count
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_CNT4_Q7_PCNT_OFFSET              WED_PAO_MON_QMEM_CNT4_OFFSET
#define WED_PAO_MON_QMEM_CNT4_Q7_PCNT_MASK                0x0FFF0000                // Q7_PCNT[27..16]
#define WED_PAO_MON_QMEM_CNT4_Q7_PCNT_SHFT                16
#define WED_PAO_MON_QMEM_CNT4_Q8_PCNT_OFFSET              WED_PAO_MON_QMEM_CNT4_OFFSET
#define WED_PAO_MON_QMEM_CNT4_Q8_PCNT_MASK                0x00000FFF                // Q8_PCNT[11..0]
#define WED_PAO_MON_QMEM_CNT4_Q8_PCNT_SHFT                0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_PTR0 (0x15015800 + 0x620)---

    Q0_START[10..0]              - (RO) PAO QMEM (IS_SP) queue HEAD pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    FQ_START[26..16]             - (RO) PAO QMEM Free queue HEAD pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_PTR0_FQ_START_OFFSET             WED_PAO_MON_QMEM_PTR0_OFFSET
#define WED_PAO_MON_QMEM_PTR0_FQ_START_MASK               0x07FF0000                // FQ_START[26..16]
#define WED_PAO_MON_QMEM_PTR0_FQ_START_SHFT               16
#define WED_PAO_MON_QMEM_PTR0_Q0_START_OFFSET             WED_PAO_MON_QMEM_PTR0_OFFSET
#define WED_PAO_MON_QMEM_PTR0_Q0_START_MASK               0x000007FF                // Q0_START[10..0]
#define WED_PAO_MON_QMEM_PTR0_Q0_START_SHFT               0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_PTR1 (0x15015800 + 0x624)---

    Q2_START[10..0]              - (RO) PAO QMEM (TID=1) queue HEAD pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    Q1_START[26..16]             - (RO) PAO QMEM (TID=0) queue HEAD pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_PTR1_Q1_START_OFFSET             WED_PAO_MON_QMEM_PTR1_OFFSET
#define WED_PAO_MON_QMEM_PTR1_Q1_START_MASK               0x07FF0000                // Q1_START[26..16]
#define WED_PAO_MON_QMEM_PTR1_Q1_START_SHFT               16
#define WED_PAO_MON_QMEM_PTR1_Q2_START_OFFSET             WED_PAO_MON_QMEM_PTR1_OFFSET
#define WED_PAO_MON_QMEM_PTR1_Q2_START_MASK               0x000007FF                // Q2_START[10..0]
#define WED_PAO_MON_QMEM_PTR1_Q2_START_SHFT               0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_PTR2 (0x15015800 + 0x628)---

    Q4_START[10..0]              - (RO) PAO QMEM (TID=3) queue HEAD pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    Q3_START[26..16]             - (RO) PAO QMEM (TID=2) queue HEAD pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_PTR2_Q3_START_OFFSET             WED_PAO_MON_QMEM_PTR2_OFFSET
#define WED_PAO_MON_QMEM_PTR2_Q3_START_MASK               0x07FF0000                // Q3_START[26..16]
#define WED_PAO_MON_QMEM_PTR2_Q3_START_SHFT               16
#define WED_PAO_MON_QMEM_PTR2_Q4_START_OFFSET             WED_PAO_MON_QMEM_PTR2_OFFSET
#define WED_PAO_MON_QMEM_PTR2_Q4_START_MASK               0x000007FF                // Q4_START[10..0]
#define WED_PAO_MON_QMEM_PTR2_Q4_START_SHFT               0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_PTR3 (0x15015800 + 0x62C)---

    Q6_START[10..0]              - (RO) PAO QMEM (TID=5) queue HEAD pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    Q5_START[26..16]             - (RO) PAO QMEM (TID=4) queue HEAD pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_PTR3_Q5_START_OFFSET             WED_PAO_MON_QMEM_PTR3_OFFSET
#define WED_PAO_MON_QMEM_PTR3_Q5_START_MASK               0x07FF0000                // Q5_START[26..16]
#define WED_PAO_MON_QMEM_PTR3_Q5_START_SHFT               16
#define WED_PAO_MON_QMEM_PTR3_Q6_START_OFFSET             WED_PAO_MON_QMEM_PTR3_OFFSET
#define WED_PAO_MON_QMEM_PTR3_Q6_START_MASK               0x000007FF                // Q6_START[10..0]
#define WED_PAO_MON_QMEM_PTR3_Q6_START_SHFT               0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_PTR4 (0x15015800 + 0x630)---

    Q8_START[10..0]              - (RO) PAO QMEM (TID=7) queue HEAD pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    Q7_START[26..16]             - (RO) PAO QMEM (TID=6) queue HEAD pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_PTR4_Q7_START_OFFSET             WED_PAO_MON_QMEM_PTR4_OFFSET
#define WED_PAO_MON_QMEM_PTR4_Q7_START_MASK               0x07FF0000                // Q7_START[26..16]
#define WED_PAO_MON_QMEM_PTR4_Q7_START_SHFT               16
#define WED_PAO_MON_QMEM_PTR4_Q8_START_OFFSET             WED_PAO_MON_QMEM_PTR4_OFFSET
#define WED_PAO_MON_QMEM_PTR4_Q8_START_MASK               0x000007FF                // Q8_START[10..0]
#define WED_PAO_MON_QMEM_PTR4_Q8_START_SHFT               0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_PTR5 (0x15015800 + 0x634)---

    Q0_END[11..0]                - (RO) PAO QMEM (IS_SP) queue TAIL pointer
    RESERVED12[15..12]           - (RO) Reserved bits
    FQ_END[27..16]               - (RO) PAO QMEM Free queue TAIL pointer
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_PTR5_FQ_END_OFFSET               WED_PAO_MON_QMEM_PTR5_OFFSET
#define WED_PAO_MON_QMEM_PTR5_FQ_END_MASK                 0x0FFF0000                // FQ_END[27..16]
#define WED_PAO_MON_QMEM_PTR5_FQ_END_SHFT                 16
#define WED_PAO_MON_QMEM_PTR5_Q0_END_OFFSET               WED_PAO_MON_QMEM_PTR5_OFFSET
#define WED_PAO_MON_QMEM_PTR5_Q0_END_MASK                 0x00000FFF                // Q0_END[11..0]
#define WED_PAO_MON_QMEM_PTR5_Q0_END_SHFT                 0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_PTR6 (0x15015800 + 0x638)---

    Q2_END[11..0]                - (RO) PAO QMEM (TID=1) queue TAIL pointer
    RESERVED12[15..12]           - (RO) Reserved bits
    Q1_END[27..16]               - (RO) PAO QMEM (TID=0) queue TAIL pointer
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_PTR6_Q1_END_OFFSET               WED_PAO_MON_QMEM_PTR6_OFFSET
#define WED_PAO_MON_QMEM_PTR6_Q1_END_MASK                 0x0FFF0000                // Q1_END[27..16]
#define WED_PAO_MON_QMEM_PTR6_Q1_END_SHFT                 16
#define WED_PAO_MON_QMEM_PTR6_Q2_END_OFFSET               WED_PAO_MON_QMEM_PTR6_OFFSET
#define WED_PAO_MON_QMEM_PTR6_Q2_END_MASK                 0x00000FFF                // Q2_END[11..0]
#define WED_PAO_MON_QMEM_PTR6_Q2_END_SHFT                 0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_PTR7 (0x15015800 + 0x63C)---

    Q4_END[11..0]                - (RO) PAO QMEM (TID=3) queue TAIL pointer
    RESERVED12[15..12]           - (RO) Reserved bits
    Q3_END[27..16]               - (RO) PAO QMEM (TID=2) queue TAIL pointer
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_PTR7_Q3_END_OFFSET               WED_PAO_MON_QMEM_PTR7_OFFSET
#define WED_PAO_MON_QMEM_PTR7_Q3_END_MASK                 0x0FFF0000                // Q3_END[27..16]
#define WED_PAO_MON_QMEM_PTR7_Q3_END_SHFT                 16
#define WED_PAO_MON_QMEM_PTR7_Q4_END_OFFSET               WED_PAO_MON_QMEM_PTR7_OFFSET
#define WED_PAO_MON_QMEM_PTR7_Q4_END_MASK                 0x00000FFF                // Q4_END[11..0]
#define WED_PAO_MON_QMEM_PTR7_Q4_END_SHFT                 0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_PTR8 (0x15015800 + 0x640)---

    Q6_END[11..0]                - (RO) PAO QMEM (TID=5) queue TAIL pointer
    RESERVED12[15..12]           - (RO) Reserved bits
    Q5_END[27..16]               - (RO) PAO QMEM (TID=4) queue TAIL pointer
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_PTR8_Q5_END_OFFSET               WED_PAO_MON_QMEM_PTR8_OFFSET
#define WED_PAO_MON_QMEM_PTR8_Q5_END_MASK                 0x0FFF0000                // Q5_END[27..16]
#define WED_PAO_MON_QMEM_PTR8_Q5_END_SHFT                 16
#define WED_PAO_MON_QMEM_PTR8_Q6_END_OFFSET               WED_PAO_MON_QMEM_PTR8_OFFSET
#define WED_PAO_MON_QMEM_PTR8_Q6_END_MASK                 0x00000FFF                // Q6_END[11..0]
#define WED_PAO_MON_QMEM_PTR8_Q6_END_SHFT                 0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_PTR9 (0x15015800 + 0x644)---

    Q8_END[11..0]                - (RO) PAO QMEM (TID=7) queue TAIL pointer
    RESERVED12[15..12]           - (RO) Reserved bits
    Q7_END[27..16]               - (RO) PAO QMEM (TID=6) queue TAIL pointer
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_PTR9_Q7_END_OFFSET               WED_PAO_MON_QMEM_PTR9_OFFSET
#define WED_PAO_MON_QMEM_PTR9_Q7_END_MASK                 0x0FFF0000                // Q7_END[27..16]
#define WED_PAO_MON_QMEM_PTR9_Q7_END_SHFT                 16
#define WED_PAO_MON_QMEM_PTR9_Q8_END_OFFSET               WED_PAO_MON_QMEM_PTR9_OFFSET
#define WED_PAO_MON_QMEM_PTR9_Q8_END_MASK                 0x00000FFF                // Q8_END[11..0]
#define WED_PAO_MON_QMEM_PTR9_Q8_END_SHFT                 0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_PTR10 (0x15015800 + 0x648)---

    Q0_CPP[10..0]                - (RO) PAO QMEM (IS_SP) queue Pre-pointer
    RESERVED11[31..11]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_PTR10_Q0_CPP_OFFSET              WED_PAO_MON_QMEM_PTR10_OFFSET
#define WED_PAO_MON_QMEM_PTR10_Q0_CPP_MASK                0x000007FF                // Q0_CPP[10..0]
#define WED_PAO_MON_QMEM_PTR10_Q0_CPP_SHFT                0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_PTR11 (0x15015800 + 0x64C)---

    Q2_CPP[10..0]                - (RO) PAO QMEM (TID=1) queue Pre-pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    Q1_CPP[26..16]               - (RO) PAO QMEM (TID=0) queue Pre-pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_PTR11_Q1_CPP_OFFSET              WED_PAO_MON_QMEM_PTR11_OFFSET
#define WED_PAO_MON_QMEM_PTR11_Q1_CPP_MASK                0x07FF0000                // Q1_CPP[26..16]
#define WED_PAO_MON_QMEM_PTR11_Q1_CPP_SHFT                16
#define WED_PAO_MON_QMEM_PTR11_Q2_CPP_OFFSET              WED_PAO_MON_QMEM_PTR11_OFFSET
#define WED_PAO_MON_QMEM_PTR11_Q2_CPP_MASK                0x000007FF                // Q2_CPP[10..0]
#define WED_PAO_MON_QMEM_PTR11_Q2_CPP_SHFT                0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_PTR12 (0x15015800 + 0x650)---

    Q4_CPP[10..0]                - (RO) PAO QMEM (TID=3) queue Pre-pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    Q3_CPP[26..16]               - (RO) PAO QMEM (TID=2) queue Pre-pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_PTR12_Q3_CPP_OFFSET              WED_PAO_MON_QMEM_PTR12_OFFSET
#define WED_PAO_MON_QMEM_PTR12_Q3_CPP_MASK                0x07FF0000                // Q3_CPP[26..16]
#define WED_PAO_MON_QMEM_PTR12_Q3_CPP_SHFT                16
#define WED_PAO_MON_QMEM_PTR12_Q4_CPP_OFFSET              WED_PAO_MON_QMEM_PTR12_OFFSET
#define WED_PAO_MON_QMEM_PTR12_Q4_CPP_MASK                0x000007FF                // Q4_CPP[10..0]
#define WED_PAO_MON_QMEM_PTR12_Q4_CPP_SHFT                0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_PTR13 (0x15015800 + 0x654)---

    Q6_CPP[10..0]                - (RO) PAO QMEM (TID=5) queue Pre-pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    Q5_CPP[26..16]               - (RO) PAO QMEM (TID=4) queue Pre-pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_PTR13_Q5_CPP_OFFSET              WED_PAO_MON_QMEM_PTR13_OFFSET
#define WED_PAO_MON_QMEM_PTR13_Q5_CPP_MASK                0x07FF0000                // Q5_CPP[26..16]
#define WED_PAO_MON_QMEM_PTR13_Q5_CPP_SHFT                16
#define WED_PAO_MON_QMEM_PTR13_Q6_CPP_OFFSET              WED_PAO_MON_QMEM_PTR13_OFFSET
#define WED_PAO_MON_QMEM_PTR13_Q6_CPP_MASK                0x000007FF                // Q6_CPP[10..0]
#define WED_PAO_MON_QMEM_PTR13_Q6_CPP_SHFT                0

/* =====================================================================================

  ---WED_PAO_MON_QMEM_PTR14 (0x15015800 + 0x658)---

    Q8_CPP[10..0]                - (RO) PAO QMEM (TID=7) queue Pre-pointer
    RESERVED11[15..11]           - (RO) Reserved bits
    Q7_CPP[26..16]               - (RO) PAO QMEM (TID=6) queue Pre-pointer
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_QMEM_PTR14_Q7_CPP_OFFSET              WED_PAO_MON_QMEM_PTR14_OFFSET
#define WED_PAO_MON_QMEM_PTR14_Q7_CPP_MASK                0x07FF0000                // Q7_CPP[26..16]
#define WED_PAO_MON_QMEM_PTR14_Q7_CPP_SHFT                16
#define WED_PAO_MON_QMEM_PTR14_Q8_CPP_OFFSET              WED_PAO_MON_QMEM_PTR14_OFFSET
#define WED_PAO_MON_QMEM_PTR14_Q8_CPP_MASK                0x000007FF                // Q8_CPP[10..0]
#define WED_PAO_MON_QMEM_PTR14_Q8_CPP_SHFT                0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_SCHED (0x15015800 + 0x65C)---

    STS[31..0]                   - (RO) PAO SCHED status

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_SCHED_STS_OFFSET                WED_PAO_MON_AMSDU_SCHED_OFFSET
#define WED_PAO_MON_AMSDU_SCHED_STS_MASK                  0xFFFFFFFF                // STS[31..0]
#define WED_PAO_MON_AMSDU_SCHED_STS_SHFT                  0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH (0x15015800 + 0x670)---

    STS[31..0]                   - (RO) PAO HIFTXD_FETCH status

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_STS_OFFSET               WED_PAO_MON_HIFTXD_FETCH_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_STS_MASK                 0xFFFFFFFF                // STS[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_STS_SHFT                 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_CNT0 (0x15015800 + 0x674)---

    HIFTXD_IN_CNT[31..0]         - (RO) PAO HIFTX_FETCH input Partial HIFTXD count

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_CNT0_HIFTXD_IN_CNT_OFFSET WED_PAO_MON_HIFTXD_FETCH_CNT0_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_CNT0_HIFTXD_IN_CNT_MASK  0xFFFFFFFF                // HIFTXD_IN_CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_CNT0_HIFTXD_IN_CNT_SHFT  0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_CNT1 (0x15015800 + 0x678)---

    HIFTXD_OU1_CNT[31..0]        - (RO) PAO HIFTX_FETCH output HIFTXD to Ring0 count

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_CNT1_HIFTXD_OU1_CNT_OFFSET WED_PAO_MON_HIFTXD_FETCH_CNT1_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_CNT1_HIFTXD_OU1_CNT_MASK 0xFFFFFFFF                // HIFTXD_OU1_CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_CNT1_HIFTXD_OU1_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_CNT2 (0x15015800 + 0x67C)---

    HIFTXD_OU2_CNT[31..0]        - (RO) PAO HIFTX_FETCH output HIFTXD to Ring1 count

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_CNT2_HIFTXD_OU2_CNT_OFFSET WED_PAO_MON_HIFTXD_FETCH_CNT2_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_CNT2_HIFTXD_OU2_CNT_MASK 0xFFFFFFFF                // HIFTXD_OU2_CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_CNT2_HIFTXD_OU2_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_CNT3 (0x15015800 + 0x680)---

    CURQ_CNT[11..0]              - (RO) PAO QMEM output-queue entry count
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_CNT3_CURQ_CNT_OFFSET     WED_PAO_MON_HIFTXD_FETCH_CNT3_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_CNT3_CURQ_CNT_MASK       0x00000FFF                // CURQ_CNT[11..0]
#define WED_PAO_MON_HIFTXD_FETCH_CNT3_CURQ_CNT_SHFT       0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1 (0x15015800 + 0x690)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=1 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1_CNT_OFFSET     WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1_CNT_MASK       0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1_CNT_SHFT       0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_2 (0x15015800 + 0x694)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=2 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_2_CNT_OFFSET     WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_2_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_2_CNT_MASK       0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_2_CNT_SHFT       0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_3 (0x15015800 + 0x698)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=3 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_3_CNT_OFFSET     WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_3_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_3_CNT_MASK       0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_3_CNT_SHFT       0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_4 (0x15015800 + 0x69C)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=4 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_4_CNT_OFFSET     WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_4_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_4_CNT_MASK       0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_4_CNT_SHFT       0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_5 (0x15015800 + 0x6A0)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=5 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_5_CNT_OFFSET     WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_5_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_5_CNT_MASK       0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_5_CNT_SHFT       0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_6 (0x15015800 + 0x6A4)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=6 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_6_CNT_OFFSET     WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_6_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_6_CNT_MASK       0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_6_CNT_SHFT       0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_7 (0x15015800 + 0x6A8)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=7 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_7_CNT_OFFSET     WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_7_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_7_CNT_MASK       0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_7_CNT_SHFT       0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_8 (0x15015800 + 0x6AC)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=8 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_8_CNT_OFFSET     WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_8_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_8_CNT_MASK       0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_8_CNT_SHFT       0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_9 (0x15015800 + 0x6B0)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=9 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_9_CNT_OFFSET     WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_9_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_9_CNT_MASK       0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_9_CNT_SHFT       0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_10 (0x15015800 + 0x6B4)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=10 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_10_CNT_OFFSET    WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_10_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_10_CNT_MASK      0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_10_CNT_SHFT      0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_11 (0x15015800 + 0x6B8)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=11 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_11_CNT_OFFSET    WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_11_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_11_CNT_MASK      0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_11_CNT_SHFT      0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_12 (0x15015800 + 0x6BC)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=12 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_12_CNT_OFFSET    WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_12_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_12_CNT_MASK      0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_12_CNT_SHFT      0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_13 (0x15015800 + 0x6C0)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=13 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_13_CNT_OFFSET    WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_13_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_13_CNT_MASK      0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_13_CNT_SHFT      0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1 (0x15015800 + 0x6C4)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=1 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1_CNT_OFFSET    WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1_CNT_MASK      0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1_CNT_SHFT      0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_2 (0x15015800 + 0x6C8)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=2 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_2_CNT_OFFSET    WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_2_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_2_CNT_MASK      0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_2_CNT_SHFT      0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_3 (0x15015800 + 0x6CC)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=3 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_3_CNT_OFFSET    WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_3_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_3_CNT_MASK      0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_3_CNT_SHFT      0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_4 (0x15015800 + 0x6D0)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=4 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_4_CNT_OFFSET    WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_4_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_4_CNT_MASK      0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_4_CNT_SHFT      0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_5 (0x15015800 + 0x6D4)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=5 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_5_CNT_OFFSET    WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_5_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_5_CNT_MASK      0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_5_CNT_SHFT      0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_6 (0x15015800 + 0x6D8)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=6 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_6_CNT_OFFSET    WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_6_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_6_CNT_MASK      0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_6_CNT_SHFT      0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_7 (0x15015800 + 0x6DC)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=7 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_7_CNT_OFFSET    WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_7_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_7_CNT_MASK      0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_7_CNT_SHFT      0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_8 (0x15015800 + 0x6E0)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=8 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_8_CNT_OFFSET    WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_8_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_8_CNT_MASK      0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_8_CNT_SHFT      0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_9 (0x15015800 + 0x6E4)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=9 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_9_CNT_OFFSET    WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_9_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_9_CNT_MASK      0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_9_CNT_SHFT      0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_10 (0x15015800 + 0x6E8)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=10 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_10_CNT_OFFSET   WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_10_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_10_CNT_MASK     0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_10_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_11 (0x15015800 + 0x6EC)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=11 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_11_CNT_OFFSET   WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_11_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_11_CNT_MASK     0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_11_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_12 (0x15015800 + 0x6F0)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=12 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_12_CNT_OFFSET   WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_12_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_12_CNT_MASK     0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_12_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_13 (0x15015800 + 0x6F4)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=13 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_13_CNT_OFFSET   WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_13_OFFSET
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_13_CNT_MASK     0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_13_CNT_SHFT     0

/* =====================================================================================

  ---WED_PAO_AMSDU_FIFO_ERR (0x15015800 + 0x7D0)---

    JUMBO_BUF_NUM_ERR[0]         - (W1C) When receive jumbo (gather/scatter) MSDU, the merged DMADs exceed MAX, HW will set this flag to 1'b1, SW write 1 clear
    JUMBO_TIMEOUT_ERR[1]         - (W1C) When receive jumbo (gather/scatter) MSDU, the latter DMADs long time no comming, HW will set this flag to 1'b1, SW write 1 clear
    JUMBO_ID_ERR_0[2]            - (W1C) When receive jumbo (gather/scatter) MSDU, the JUMBO ID mismatch, HW will set this flag to 1'b1, SW write 1 clear
    JUMBO_ID_ERR_1[3]            - (W1C) When receive jumbo (gather/scatter) MSDU, the JUMBO ID mismatch, HW will set this flag to 1'b1, SW write 1 clear
    RESERVED4[30..4]             - (RO) Reserved bits
    CLR[31]                      - (A0) Clear AMSDU_FIFO Error Status

 =====================================================================================*/
#define WED_PAO_AMSDU_FIFO_ERR_CLR_OFFSET                 WED_PAO_AMSDU_FIFO_ERR_OFFSET
#define WED_PAO_AMSDU_FIFO_ERR_CLR_MASK                   0x80000000                // CLR[31]
#define WED_PAO_AMSDU_FIFO_ERR_CLR_SHFT                   31
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_ID_ERR_1_OFFSET      WED_PAO_AMSDU_FIFO_ERR_OFFSET
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_ID_ERR_1_MASK        0x00000008                // JUMBO_ID_ERR_1[3]
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_ID_ERR_1_SHFT        3
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_ID_ERR_0_OFFSET      WED_PAO_AMSDU_FIFO_ERR_OFFSET
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_ID_ERR_0_MASK        0x00000004                // JUMBO_ID_ERR_0[2]
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_ID_ERR_0_SHFT        2
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_TIMEOUT_ERR_OFFSET   WED_PAO_AMSDU_FIFO_ERR_OFFSET
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_TIMEOUT_ERR_MASK     0x00000002                // JUMBO_TIMEOUT_ERR[1]
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_TIMEOUT_ERR_SHFT     1
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_BUF_NUM_ERR_OFFSET   WED_PAO_AMSDU_FIFO_ERR_OFFSET
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_BUF_NUM_ERR_MASK     0x00000001                // JUMBO_BUF_NUM_ERR[0]
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_BUF_NUM_ERR_SHFT     0

/* =====================================================================================

  ---WED_PAO_DBG_CFG (0x15015800 + 0x7E0)---

    SEL[7..0]                    - (RW) PAO Debug Interface, PAO debug signal selection
    QSEL[11..8]                  - (RW) PAO Debug Interface, PAO AMSDU engine selection
    MSEL[15..12]                 - (RW) PAO Debug Interface, PAO module selection
    PSEL[17..16]                 - (RW) PAO Debug Interface, PAO instance selection
    EN[18]                       - (RW) PAO Debug Interface, Enable
    RESERVED19[31..19]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_DBG_CFG_EN_OFFSET                         WED_PAO_DBG_CFG_OFFSET
#define WED_PAO_DBG_CFG_EN_MASK                           0x00040000                // EN[18]
#define WED_PAO_DBG_CFG_EN_SHFT                           18
#define WED_PAO_DBG_CFG_PSEL_OFFSET                       WED_PAO_DBG_CFG_OFFSET
#define WED_PAO_DBG_CFG_PSEL_MASK                         0x00030000                // PSEL[17..16]
#define WED_PAO_DBG_CFG_PSEL_SHFT                         16
#define WED_PAO_DBG_CFG_MSEL_OFFSET                       WED_PAO_DBG_CFG_OFFSET
#define WED_PAO_DBG_CFG_MSEL_MASK                         0x0000F000                // MSEL[15..12]
#define WED_PAO_DBG_CFG_MSEL_SHFT                         12
#define WED_PAO_DBG_CFG_QSEL_OFFSET                       WED_PAO_DBG_CFG_OFFSET
#define WED_PAO_DBG_CFG_QSEL_MASK                         0x00000F00                // QSEL[11..8]
#define WED_PAO_DBG_CFG_QSEL_SHFT                         8
#define WED_PAO_DBG_CFG_SEL_OFFSET                        WED_PAO_DBG_CFG_OFFSET
#define WED_PAO_DBG_CFG_SEL_MASK                          0x000000FF                // SEL[7..0]
#define WED_PAO_DBG_CFG_SEL_SHFT                          0

/* =====================================================================================

  ---WED_PAO_DBG_CNT (0x15015800 + 0x7E4)---

    CLR[0]                       - (A0) PAO Debug Interface, clear counter
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_DBG_CNT_CLR_OFFSET                        WED_PAO_DBG_CNT_OFFSET
#define WED_PAO_DBG_CNT_CLR_MASK                          0x00000001                // CLR[0]
#define WED_PAO_DBG_CNT_CLR_SHFT                          0

/* =====================================================================================

  ---WED_PAO_DBG (0x15015800 + 0x7E8)---

    INFO[31..0]                  - (RO) PAO Debug Interface, read debug info.

 =====================================================================================*/
#define WED_PAO_DBG_INFO_OFFSET                           WED_PAO_DBG_OFFSET
#define WED_PAO_DBG_INFO_MASK                             0xFFFFFFFF                // INFO[31..0]
#define WED_PAO_DBG_INFO_SHFT                             0

/* =====================================================================================

  ---WED_PAO_PKTMEM_TEST (0x15015800 + 0x7EC)---

    EN[0]                        - (RW) PAO PKETMEM Test Interface, enable
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_PKTMEM_TEST_EN_OFFSET                     WED_PAO_PKTMEM_TEST_OFFSET
#define WED_PAO_PKTMEM_TEST_EN_MASK                       0x00000001                // EN[0]
#define WED_PAO_PKTMEM_TEST_EN_SHFT                       0

/* =====================================================================================

  ---WED_PAO_PKTMEM_ADDR (0x15015800 + 0x7F0)---

    VAL[10..0]                   - (RW) PAO PKETMEM Test Interface, SRAM address
    RESERVED11[31..11]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_PKTMEM_ADDR_VAL_OFFSET                    WED_PAO_PKTMEM_ADDR_OFFSET
#define WED_PAO_PKTMEM_ADDR_VAL_MASK                      0x000007FF                // VAL[10..0]
#define WED_PAO_PKTMEM_ADDR_VAL_SHFT                      0

/* =====================================================================================

  ---WED_PAO_PKTMEM_DWSEL (0x15015800 + 0x7F4)---

    VAL[4..0]                    - (RW) PAO PKETMEM Test Interface, SRAM data dword select
    RESERVED5[31..5]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_PKTMEM_DWSEL_VAL_OFFSET                   WED_PAO_PKTMEM_DWSEL_OFFSET
#define WED_PAO_PKTMEM_DWSEL_VAL_MASK                     0x0000001F                // VAL[4..0]
#define WED_PAO_PKTMEM_DWSEL_VAL_SHFT                     0

/* =====================================================================================

  ---WED_PAO_PKTMEM_DATA (0x15015800 + 0x7F8)---

    VAL[31..0]                   - (RW) PAO PKETMEM Test Interface, SRAM data value

 =====================================================================================*/
#define WED_PAO_PKTMEM_DATA_VAL_OFFSET                    WED_PAO_PKTMEM_DATA_OFFSET
#define WED_PAO_PKTMEM_DATA_VAL_MASK                      0xFFFFFFFF                // VAL[31..0]
#define WED_PAO_PKTMEM_DATA_VAL_SHFT                      0

/* =====================================================================================

  ---WED_PAO_PKTMEM_TRIG (0x15015800 + 0x7FC)---

    DATA_SET[0]                  - (A0) PAO PKEMEM Test Interface, set data
    RD[1]                        - (A0) PAO PKEMEM Test Interface, trigger read SRAM
    WR[2]                        - (A0) PAO PKEMEM Test Interface, trigger write SRAM
    RESERVED3[31..3]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_PKTMEM_TRIG_WR_OFFSET                     WED_PAO_PKTMEM_TRIG_OFFSET
#define WED_PAO_PKTMEM_TRIG_WR_MASK                       0x00000004                // WR[2]
#define WED_PAO_PKTMEM_TRIG_WR_SHFT                       2
#define WED_PAO_PKTMEM_TRIG_RD_OFFSET                     WED_PAO_PKTMEM_TRIG_OFFSET
#define WED_PAO_PKTMEM_TRIG_RD_MASK                       0x00000002                // RD[1]
#define WED_PAO_PKTMEM_TRIG_RD_SHFT                       1
#define WED_PAO_PKTMEM_TRIG_DATA_SET_OFFSET               WED_PAO_PKTMEM_TRIG_OFFSET
#define WED_PAO_PKTMEM_TRIG_DATA_SET_MASK                 0x00000001                // DATA_SET[0]
#define WED_PAO_PKTMEM_TRIG_DATA_SET_SHFT                 0

