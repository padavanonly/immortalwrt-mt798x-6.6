
#ifndef __WOX_AP2WO_MCU_CCIF4_REGS_H__
#define __WOX_AP2WO_MCU_CCIF4_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
//
//                     WO0_AP2WO_MCU_CCIF4 CR Definitions
//
//****************************************************************************

#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_BUSY_ADDR               0x00000004
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_START_ADDR              0x00000008
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_TCHNUM_ADDR             0x0000000C
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_RCHNUM_ADDR             0x00000010
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_ACK_ADDR                0x00000014
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ0_MASK_ADDR          0x00000018
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ1_MASK_ADDR          0x0000001C
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY1_ADDR             0x00000020
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY2_ADDR             0x00000024
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY3_ADDR             0x00000028
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY4_ADDR             0x0000002C
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW1_ADDR             0x00000030
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW2_ADDR             0x00000034
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW3_ADDR             0x00000038
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW4_ADDR             0x0000003C
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY5_ADDR             0x00000050
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY6_ADDR             0x00000054
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY7_ADDR             0x00000058
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY8_ADDR             0x0000005C
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW5_ADDR             0x00000060
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW6_ADDR             0x00000064
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW7_ADDR             0x00000068
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW8_ADDR             0x0000006C

/* =====================================================================================

  ---CPUA_PCCIF_BUSY (0x1559B000 + 0x0004u)---

    BUSY[7..0]                   - (RW) Indicates which channel operation is in process
                                     It will de-assert when CPUB finishes fetching channel data and then write acknowledgement for according channel. BUSY [7] refers to the busy status of channel 7 and so on. DO NOT use the read-modified-write procedure to set up the BUSY register. It will result in some channels being busy all the time and cannot be cleared by the other side. The root cause is that the processing channels may be cleared during the read-modified-write procedure. The safe CPUBy to set up the BUSY register is to only set up the channel you would like to occupy, e.g. if you are to use channel 3 for data transfer, set the BUSY register to 0x4 no matter what the current BUSY register is. (write bit n to 1 to assert busy for channel n; write 0 has no effect)
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_BUSY_BUSY_ADDR          WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_BUSY_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_BUSY_BUSY_MASK          0x000000FF                // BUSY[7..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_BUSY_BUSY_SHFT          0

/* =====================================================================================

  ---CPUA_PCCIF_START (0x1559B000 + 0x0008u)---

    START[7..0]                  - (RU) Indicates the state of completed transmitted channel number, not receiving acknowledgement. It will assert when writing TCHNUM by CPUA.
                                     It will de-assert when writing acknowledgement for according channel by CPUB. START [0] represents the start status of channel 0 and so on.
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_START_START_ADDR        WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_START_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_START_START_MASK        0x000000FF                // START[7..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_START_START_SHFT        0

/* =====================================================================================

  ---CPUA_PCCIF_TCHNUM (0x1559B000 + 0x000Cu)---

    TCHNUM[2..0]                 - (WO) The 3-bit channel number represents which channel (channel 7-0) is to be used for transmitting data to CPUB.
    RESERVED3[31..3]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_TCHNUM_TCHNUM_ADDR      WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_TCHNUM_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_TCHNUM_TCHNUM_MASK      0x00000007                // TCHNUM[2..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_TCHNUM_TCHNUM_SHFT      0

/* =====================================================================================

  ---CPUA_PCCIF_RCHNUM (0x1559B000 + 0x0010u)---

    RCHNUM[7..0]                 - (RO) Bit0 to Bit7 will report the read status of each channel.
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_RCHNUM_RCHNUM_ADDR      WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_RCHNUM_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_RCHNUM_RCHNUM_MASK      0x000000FF                // RCHNUM[7..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_RCHNUM_RCHNUM_SHFT      0

/* =====================================================================================

  ---CPUA_PCCIF_ACK (0x1559B000 + 0x0014u)---

    ACK[7..0]                    - (W1C) Acknowledgment
                                     It is write-cleared and set by CPUA_MCU for clearing interrupt status and BUSY/START states of CPUB CPU-CPU interface according channel. Writing ACK is the last step to end one channel transfer.
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_ACK_ACK_ADDR            WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_ACK_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_ACK_ACK_MASK            0x000000FF                // ACK[7..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_ACK_ACK_SHFT            0

/* =====================================================================================

  ---CPUA_PCCIF_IRQ0_MASK (0x1559B000 + 0x0018u)---

    IRQ0_MASK[7..0]              - (RW) IRQ0 mask is used to mask the channel start event to signal pin IRQ0.
                                     If the register is set to 8'hff, all the 8ch start event will generate the IRQ0.
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ0_MASK_IRQ0_MASK_ADDR WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ0_MASK_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ0_MASK_IRQ0_MASK_MASK 0x000000FF                // IRQ0_MASK[7..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ0_MASK_IRQ0_MASK_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_IRQ1_MASK (0x1559B000 + 0x001Cu)---

    IRQ1_MASK[7..0]              - (RW) IRQ1 mask is used to mask the channel start event to signal pin IRQ1.
                                     If the register is set to  8'hff, all the 8ch start event will generate the IRQ1.
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ1_MASK_IRQ1_MASK_ADDR WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ1_MASK_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ1_MASK_IRQ1_MASK_MASK 0x000000FF                // IRQ1_MASK[7..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ1_MASK_IRQ1_MASK_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY1 (0x1559B000 + 0x0020u)---

    CPUA_PCCIF_DUMMY1[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY1_CPUA_PCCIF_DUMMY1_ADDR WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY1_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY1_CPUA_PCCIF_DUMMY1_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY1[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY1_CPUA_PCCIF_DUMMY1_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY2 (0x1559B000 + 0x0024u)---

    CPUA_PCCIF_DUMMY2[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY2_CPUA_PCCIF_DUMMY2_ADDR WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY2_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY2_CPUA_PCCIF_DUMMY2_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY2_CPUA_PCCIF_DUMMY2_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY3 (0x1559B000 + 0x0028u)---

    CPUA_PCCIF_DUMMY2[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY3_CPUA_PCCIF_DUMMY2_ADDR WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY3_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY3_CPUA_PCCIF_DUMMY2_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY3_CPUA_PCCIF_DUMMY2_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY4 (0x1559B000 + 0x002Cu)---

    CPUA_PCCIF_DUMMY2[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY4_CPUA_PCCIF_DUMMY2_ADDR WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY4_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY4_CPUA_PCCIF_DUMMY2_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY4_CPUA_PCCIF_DUMMY2_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW1 (0x1559B000 + 0x0030u)---

    CPUB2CPUA_SHADOW1[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW1_CPUB2CPUA_SHADOW1_ADDR WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW1_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW1_CPUB2CPUA_SHADOW1_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW1[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW1_CPUB2CPUA_SHADOW1_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW2 (0x1559B000 + 0x0034u)---

    CPUB2CPUA_SHADOW2[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW2_CPUB2CPUA_SHADOW2_ADDR WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW2_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW2_CPUB2CPUA_SHADOW2_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW2_CPUB2CPUA_SHADOW2_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW3 (0x1559B000 + 0x0038u)---

    CPUB2CPUA_SHADOW2[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW3_CPUB2CPUA_SHADOW2_ADDR WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW3_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW3_CPUB2CPUA_SHADOW2_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW3_CPUB2CPUA_SHADOW2_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW4 (0x1559B000 + 0x003Cu)---

    CPUB2CPUA_SHADOW2[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW4_CPUB2CPUA_SHADOW2_ADDR WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW4_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW4_CPUB2CPUA_SHADOW2_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW4_CPUB2CPUA_SHADOW2_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY5 (0x1559B000 + 0x0050u)---

    CPUA_PCCIF_DUMMY1[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY5_CPUA_PCCIF_DUMMY1_ADDR WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY5_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY5_CPUA_PCCIF_DUMMY1_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY1[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY5_CPUA_PCCIF_DUMMY1_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY6 (0x1559B000 + 0x0054u)---

    CPUA_PCCIF_DUMMY2[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY6_CPUA_PCCIF_DUMMY2_ADDR WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY6_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY6_CPUA_PCCIF_DUMMY2_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY6_CPUA_PCCIF_DUMMY2_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY7 (0x1559B000 + 0x0058u)---

    CPUA_PCCIF_DUMMY2[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY7_CPUA_PCCIF_DUMMY2_ADDR WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY7_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY7_CPUA_PCCIF_DUMMY2_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY7_CPUA_PCCIF_DUMMY2_SHFT 0

/* =====================================================================================

  ---CPUA_PCCIF_DUMMY8 (0x1559B000 + 0x005Cu)---

    CPUA_PCCIF_DUMMY2[31..0]     - (RW) This register is CPUA side dummy register  for SW debug purpose only, CPUA can R/W and CPUB can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY8_CPUA_PCCIF_DUMMY2_ADDR WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY8_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY8_CPUA_PCCIF_DUMMY2_MASK 0xFFFFFFFF                // CPUA_PCCIF_DUMMY2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY8_CPUA_PCCIF_DUMMY2_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW5 (0x1559B000 + 0x0060u)---

    CPUB2CPUA_SHADOW1[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW5_CPUB2CPUA_SHADOW1_ADDR WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW5_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW5_CPUB2CPUA_SHADOW1_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW1[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW5_CPUB2CPUA_SHADOW1_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW6 (0x1559B000 + 0x0064u)---

    CPUB2CPUA_SHADOW2[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW6_CPUB2CPUA_SHADOW2_ADDR WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW6_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW6_CPUB2CPUA_SHADOW2_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW6_CPUB2CPUA_SHADOW2_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW7 (0x1559B000 + 0x0068u)---

    CPUB2CPUA_SHADOW2[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW7_CPUB2CPUA_SHADOW2_ADDR WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW7_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW7_CPUB2CPUA_SHADOW2_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW7_CPUB2CPUA_SHADOW2_SHFT 0

/* =====================================================================================

  ---CPUB2CPUA_SHADOW8 (0x1559B000 + 0x006Cu)---

    CPUB2CPUA_SHADOW2[31..0]     - (RO) This register is CPUB side register shadow by CPUA side , CPUA can read only.

 =====================================================================================*/
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW8_CPUB2CPUA_SHADOW2_ADDR WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW8_ADDR
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW8_CPUB2CPUA_SHADOW2_MASK 0xFFFFFFFF                // CPUB2CPUA_SHADOW2[31..0]
#define WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW8_CPUB2CPUA_SHADOW2_SHFT 0

#ifdef __cplusplus
}
#endif

#endif // __WOX_AP2WO_MCU_CCIF4_REGS_H__
