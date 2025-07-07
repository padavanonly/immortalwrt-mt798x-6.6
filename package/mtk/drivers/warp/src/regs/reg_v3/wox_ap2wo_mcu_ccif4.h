
#ifndef __WOX_AP2WO_MCU_CCIF4_H__
#define __WOX_AP2WO_MCU_CCIF4_H__

/* CONST definition */
#ifndef REG_CONST
#define REG_CONST
#endif
typedef unsigned int uint32_t;

typedef volatile struct WOX_AP2WO_MCU_CCIF4_REG {
    /* 0x000 ~ 0x000 */
    REG_CONST uint32_t          RSV_000_000[1];

    /* 0x004 */
    union {
        struct {
            REG_CONST uint32_t _BUSY : 8;
            REG_CONST uint32_t _RESERVED_1 : 24;
        };
        REG_CONST uint32_t DATA;
    } CPUA_PCCIF_BUSY;

    /* 0x008 */
    union {
        struct {
            REG_CONST uint32_t _START : 8;
            REG_CONST uint32_t _RESERVED_1 : 24;
        };
        REG_CONST uint32_t DATA;
    } CPUA_PCCIF_START;

    /* 0x00c */
    union {
        struct {
            REG_CONST uint32_t _TCHNUM : 3;
            REG_CONST uint32_t _RESERVED_1 : 29;
        };
        REG_CONST uint32_t DATA;
    } CPUA_PCCIF_TCHNUM;

    /* 0x010 */
    union {
        struct {
            REG_CONST uint32_t _RCHNUM : 8;
            REG_CONST uint32_t _RESERVED_1 : 24;
        };
        REG_CONST uint32_t DATA;
    } CPUA_PCCIF_RCHNUM;

    /* 0x014 */
    union {
        struct {
            REG_CONST uint32_t _ACK : 8;
            REG_CONST uint32_t _RESERVED_1 : 24;
        };
        REG_CONST uint32_t DATA;
    } CPUA_PCCIF_ACK;

    /* 0x018 */
    union {
        struct {
            REG_CONST uint32_t _IRQ0_MASK : 8;
            REG_CONST uint32_t _RESERVED_1 : 24;
        };
        REG_CONST uint32_t DATA;
    } CPUA_PCCIF_IRQ0_MASK;

    /* 0x01c */
    union {
        struct {
            REG_CONST uint32_t _IRQ1_MASK : 8;
            REG_CONST uint32_t _RESERVED_1 : 24;
        };
        REG_CONST uint32_t DATA;
    } CPUA_PCCIF_IRQ1_MASK;

    /* 0x020 */
    union {
        struct {
            REG_CONST uint32_t _CPUA_PCCIF_DUMMY1 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUA_PCCIF_DUMMY1;

    /* 0x024 */
    union {
        struct {
            REG_CONST uint32_t _CPUA_PCCIF_DUMMY2 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUA_PCCIF_DUMMY2;

    /* 0x028 */
    union {
        struct {
            REG_CONST uint32_t _CPUA_PCCIF_DUMMY2 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUA_PCCIF_DUMMY3;

    /* 0x02c */
    union {
        struct {
            REG_CONST uint32_t _CPUA_PCCIF_DUMMY2 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUA_PCCIF_DUMMY4;

    /* 0x030 */
    union {
        struct {
            REG_CONST uint32_t _CPUB2CPUA_SHADOW1 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUB2CPUA_SHADOW1;

    /* 0x034 */
    union {
        struct {
            REG_CONST uint32_t _CPUB2CPUA_SHADOW2 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUB2CPUA_SHADOW2;

    /* 0x038 */
    union {
        struct {
            REG_CONST uint32_t _CPUB2CPUA_SHADOW2 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUB2CPUA_SHADOW3;

    /* 0x03c */
    union {
        struct {
            REG_CONST uint32_t _CPUB2CPUA_SHADOW2 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUB2CPUA_SHADOW4;

    /* 0x040 ~ 0x04c */
    REG_CONST uint32_t          RSV_040_04c[4];

    /* 0x050 */
    union {
        struct {
            REG_CONST uint32_t _CPUA_PCCIF_DUMMY1 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUA_PCCIF_DUMMY5;

    /* 0x054 */
    union {
        struct {
            REG_CONST uint32_t _CPUA_PCCIF_DUMMY2 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUA_PCCIF_DUMMY6;

    /* 0x058 */
    union {
        struct {
            REG_CONST uint32_t _CPUA_PCCIF_DUMMY2 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUA_PCCIF_DUMMY7;

    /* 0x05c */
    union {
        struct {
            REG_CONST uint32_t _CPUA_PCCIF_DUMMY2 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUA_PCCIF_DUMMY8;

    /* 0x060 */
    union {
        struct {
            REG_CONST uint32_t _CPUB2CPUA_SHADOW1 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUB2CPUA_SHADOW5;

    /* 0x064 */
    union {
        struct {
            REG_CONST uint32_t _CPUB2CPUA_SHADOW2 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUB2CPUA_SHADOW6;

    /* 0x068 */
    union {
        struct {
            REG_CONST uint32_t _CPUB2CPUA_SHADOW2 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUB2CPUA_SHADOW7;

    /* 0x06c */
    union {
        struct {
            REG_CONST uint32_t _CPUB2CPUA_SHADOW2 : 32;
        };
        REG_CONST uint32_t DATA;
    } CPUB2CPUA_SHADOW8;
} WOX_AP2WO_MCU_CCIF4_REG_T;

#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_BUSY_OFFSET             (0x0004) // 5004
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_START_OFFSET            (0x0008) // 5008
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_TCHNUM_OFFSET           (0x000C) // 500C
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_RCHNUM_OFFSET           (0x0010) // 5010
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_ACK_OFFSET              (0x0014) // 5014
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ0_MASK_OFFSET        (0x0018) // 5018
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ1_MASK_OFFSET        (0x001C) // 501C
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY1_OFFSET           (0x0020) // 5020
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY2_OFFSET           (0x0024) // 5024
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY3_OFFSET           (0x0028) // 5028
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY4_OFFSET           (0x002C) // 502C
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW1_OFFSET           (0x0030) // 5030
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW2_OFFSET           (0x0034) // 5034
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW3_OFFSET           (0x0038) // 5038
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW4_OFFSET           (0x003C) // 503C
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY5_OFFSET           (0x0050) // 5050
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY6_OFFSET           (0x0054) // 5054
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY7_OFFSET           (0x0058) // 5058
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY8_OFFSET           (0x005C) // 505C
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW5_OFFSET           (0x0060) // 5060
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW6_OFFSET           (0x0064) // 5064
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW7_OFFSET           (0x0068) // 5068
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW8_OFFSET           (0x006C) // 506C

/* =====================================================================================

  ---CPUA_PCCIF_BUSY (0x153A5000 + 0x0004)---

    BUSY[7..0]                   - (RW) Indicates which channel operation is in process
                                     It will de-assert when CPUB finishes fetching channel data and then write acknowledgement for according channel. BUSY [7] refers to the busy status of channel 7 and so on. DO NOT use the read-modified-write procedure to set up the BUSY register. It will result in some channels being busy all the time and cannot be cleared by the other side. The root cause is that the processing channels may be cleared during the read-modified-write procedure. The safe CPUBy to set up the BUSY register is to only set up the channel you would like to occupy, e.g. if you are to use channel 3 for data transfer, set the BUSY register to 0x4 no matter what the current BUSY register is. (write bit n to 1 to assert busy for channel n; write 0 has no effect)
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_BUSY_BUSY_OFFSET        WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_BUSY_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_BUSY_BUSY_MASK          0x000000FF                // BUSY[7..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_BUSY_BUSY_SHFT          0

/* =====================================================================================

  ---CPUA_PCCIF_START (0x153A5000 + 0x0008)---

    START[7..0]                  - (RU) Indicates the state of completed transmitted channel number, not receiving acknowledgement. It will assert when writing TCHNUM by CPUA.
                                     It will de-assert when writing acknowledgement for according channel by CPUB. START [0] represents the start status of channel 0 and so on.
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_START_START_OFFSET      WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_START_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_START_START_MASK        0x000000FF                // START[7..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_START_START_SHFT        0

/* =====================================================================================

  ---CPUA_PCCIF_TCHNUM (0x153A5000 + 0x000C)---

    TCHNUM[2..0]                 - (WO) The 3-bit channel number represents which channel (channel 7-0) is to be used for transmitting data to CPUB.
    RESERVED3[31..3]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_TCHNUM_TCHNUM_OFFSET    WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_TCHNUM_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_TCHNUM_TCHNUM_MASK      0x00000007                // TCHNUM[2..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_TCHNUM_TCHNUM_SHFT      0

/* =====================================================================================

  ---CPUA_PCCIF_RCHNUM (0x153A5000 + 0x0010)---

    RCHNUM[7..0]                 - (RO) Bit0 to Bit7 will report the read status of each channel.
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_RCHNUM_RCHNUM_OFFSET    WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_RCHNUM_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_RCHNUM_RCHNUM_MASK      0x000000FF                // RCHNUM[7..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_RCHNUM_RCHNUM_SHFT      0

/* =====================================================================================

  ---CPUA_PCCIF_ACK (0x153A5000 + 0x0014)---

    ACK[7..0]                    - (W1C) Acknowledgment
                                     It is write-cleared and set by CPUA_MCU for clearing interrupt status and BUSY/START states of CPUB CPU-CPU interface according channel. Writing ACK is the last step to end one channel transfer.
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_ACK_ACK_OFFSET          WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_ACK_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_ACK_ACK_MASK            0x000000FF                // ACK[7..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_ACK_ACK_SHFT            0

/* =====================================================================================

  ---CPUA_PCCIF_IRQ0_MASK (0x153A5000 + 0x0018)---

    IRQ0_MASK[7..0]              - (RW) IRQ0 mask is used to mask the channel start event to signal pin IRQ0.
                                     If the register is set to 8'hff, all the 8ch start event will generate the IRQ0.
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ0_MASK_IRQ0_MASK_OFFSET WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ0_MASK_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ0_MASK_IRQ0_MASK_MASK 0x000000FF                // IRQ0_MASK[7..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ0_MASK_IRQ0_MASK_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_IRQ1_MASK (0x153A5000 + 0x001C)---

    IRQ1_MASK[7..0]              - (RW) IRQ1 mask is used to mask the channel start event to signal pin IRQ1.
                                     If the register is set to  8'hff, all the 8ch start event will generate the IRQ1.
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ1_MASK_IRQ1_MASK_OFFSET WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ1_MASK_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ1_MASK_IRQ1_MASK_MASK 0x000000FF                // IRQ1_MASK[7..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ1_MASK_IRQ1_MASK_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY1 (0x153A5000 + 0x0020)---

    CPUA_PCCIF_DUMMY1[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY1_CPUA_PCCIF_DUMMY1_OFFSET WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY1_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY1_CPUA_PCCIF_DUMMY1_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY1[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY1_CPUA_PCCIF_DUMMY1_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY2 (0x153A5000 + 0x0024)---

    CPUA_PCCIF_DUMMY2[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY2_CPUA_PCCIF_DUMMY2_OFFSET WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY2_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY2_CPUA_PCCIF_DUMMY2_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY2_CPUA_PCCIF_DUMMY2_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY3 (0x153A5000 + 0x0028)---

    CPUA_PCCIF_DUMMY2[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY3_CPUA_PCCIF_DUMMY2_OFFSET WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY3_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY3_CPUA_PCCIF_DUMMY2_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY3_CPUA_PCCIF_DUMMY2_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY4 (0x153A5000 + 0x002C)---

    CPUA_PCCIF_DUMMY2[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY4_CPUA_PCCIF_DUMMY2_OFFSET WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY4_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY4_CPUA_PCCIF_DUMMY2_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY4_CPUA_PCCIF_DUMMY2_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW1 (0x153A5000 + 0x0030)---

    CPUB2CPUA_SHADOW1[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW1_CPUB2CPUA_SHADOW1_OFFSET WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW1_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW1_CPUB2CPUA_SHADOW1_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW1[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW1_CPUB2CPUA_SHADOW1_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW2 (0x153A5000 + 0x0034)---

    CPUB2CPUA_SHADOW2[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW2_CPUB2CPUA_SHADOW2_OFFSET WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW2_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW2_CPUB2CPUA_SHADOW2_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW2_CPUB2CPUA_SHADOW2_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW3 (0x153A5000 + 0x0038)---

    CPUB2CPUA_SHADOW2[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW3_CPUB2CPUA_SHADOW2_OFFSET WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW3_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW3_CPUB2CPUA_SHADOW2_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW3_CPUB2CPUA_SHADOW2_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW4 (0x153A5000 + 0x003C)---

    CPUB2CPUA_SHADOW2[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW4_CPUB2CPUA_SHADOW2_OFFSET WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW4_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW4_CPUB2CPUA_SHADOW2_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW4_CPUB2CPUA_SHADOW2_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY5 (0x153A5000 + 0x0050)---

    CPUA_PCCIF_DUMMY1[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY5_CPUA_PCCIF_DUMMY1_OFFSET WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY5_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY5_CPUA_PCCIF_DUMMY1_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY1[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY5_CPUA_PCCIF_DUMMY1_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY6 (0x153A5000 + 0x0054)---

    CPUA_PCCIF_DUMMY2[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY6_CPUA_PCCIF_DUMMY2_OFFSET WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY6_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY6_CPUA_PCCIF_DUMMY2_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY6_CPUA_PCCIF_DUMMY2_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY7 (0x153A5000 + 0x0058)---

    CPUA_PCCIF_DUMMY2[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY7_CPUA_PCCIF_DUMMY2_OFFSET WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY7_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY7_CPUA_PCCIF_DUMMY2_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY7_CPUA_PCCIF_DUMMY2_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY8 (0x153A5000 + 0x005C)---

    CPUA_PCCIF_DUMMY2[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY8_CPUA_PCCIF_DUMMY2_OFFSET WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY8_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY8_CPUA_PCCIF_DUMMY2_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY8_CPUA_PCCIF_DUMMY2_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW5 (0x153A5000 + 0x0060)---

    CPUB2CPUA_SHADOW1[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW5_CPUB2CPUA_SHADOW1_OFFSET WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW5_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW5_CPUB2CPUA_SHADOW1_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW1[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW5_CPUB2CPUA_SHADOW1_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW6 (0x153A5000 + 0x0064)---

    CPUB2CPUA_SHADOW2[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW6_CPUB2CPUA_SHADOW2_OFFSET WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW6_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW6_CPUB2CPUA_SHADOW2_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW6_CPUB2CPUA_SHADOW2_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW7 (0x153A5000 + 0x0068)---

    CPUB2CPUA_SHADOW2[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW7_CPUB2CPUA_SHADOW2_OFFSET WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW7_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW7_CPUB2CPUA_SHADOW2_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW7_CPUB2CPUA_SHADOW2_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW8 (0x153A5000 + 0x006C)---

    CPUB2CPUA_SHADOW2[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW8_CPUB2CPUA_SHADOW2_OFFSET WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW8_OFFSET
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW8_CPUB2CPUA_SHADOW2_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW8_CPUB2CPUA_SHADOW2_SHFT 0

#endif
