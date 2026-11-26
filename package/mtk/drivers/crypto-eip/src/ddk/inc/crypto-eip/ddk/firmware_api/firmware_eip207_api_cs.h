/* firmware_eip207_api_cs.h
 *
 * EIP-207 Firmware Classification API:
 * Initialization functionality,
 *
 * This API is defined by the EIP-207 Classification Firmware
 *
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : firmware_eip197                                          */
/*   Version       : 3.5                                                      */
/*   Configuration : FIRMWARE-GENERIC                                         */
/*                                                                            */
/*   Date          : 2022-Dec-21                                              */
/*                                                                            */
/* Copyright (c) 2012-2022 by Rambus, Inc. and/or its subsidiaries.           */
/* All rights reserved. Unauthorized use (including, without limitation,      */
/* distribution and copying) is strictly prohibited. All use requires,        */
/* and is subject to, explicit written authorization and nondisclosure        */
/* agreements with Rambus, Inc. and/or its subsidiaries.                      */
/*                                                                            */
/* For more information or support, please go to our online support system at */
/* https://sipsupport.rambus.com.                                             */
/* In case you do not have an account for this system, please send an e-mail  */
/* to sipsupport@rambus.com.                                                  */
/* -------------------------------------------------------------------------- */

#ifndef FIRMWARE_EIP207_API_CS_H_
#define FIRMWARE_EIP207_API_CS_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"       // uint32_t, MASK_16_BITS

// Firmware EIP-207 Classification API, Flow Control
#include "firmware_eip207_api_flow_cs.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// the recommended value for combining the write command in the cache client
#define FIRMWARE_EIP207_RC_DMA_WR_COMB_DLY       0x07


// Size of the Flow Record in 32-bit words
#define FIRMWARE_EIP207_CS_FRC_RECORD_WORD_COUNT                \
                                    FIRMWARE_EIP207_CS_FLOW_FRC_RECORD_WORD_COUNT

// Size of the Small Transform Record in 32-bit words
#define FIRMWARE_EIP207_CS_TRC_RECORD_WORD_COUNT                \
                                    FIRMWARE_EIP207_CS_FLOW_TRC_RECORD_WORD_COUNT

// Size of the Large Transform Record in 32-bit words
#define FIRMWARE_EIP207_CS_TRC_RECORD_WORD_COUNT_LARGE          \
                                    FIRMWARE_EIP207_CS_FLOW_TRC_RECORD_WORD_COUNT_LARGE

// Size of the Small ARC4 State Record in 32-bit words
#define FIRMWARE_EIP207_CS_ARC4RC_RECORD_WORD_COUNT                \
                                    FIRMWARE_EIP207_CS_FLOW_ARC4RC_RECORD_WORD_COUNT

// Size of the Large ARC4 State Record in 32-bit words
#define FIRMWARE_EIP207_CS_ARC4RC_RECORD_WORD_COUNT_LARGE          \
                                    FIRMWARE_EIP207_CS_FLOW_ARC4RC_RECORD_WORD_COUNT_LARGE

// Word offset of the Flow ID field in the flow record
#define FIRMWARE_EIP207_CS_HASH_ID_WORD_OFFSET                  \
                                    FIRMWARE_EIP207_CS_FLOW_FR_FLOW_ID_WORD_OFFSET

// Flow ID field length in 32-bit words
#define FIRMWARE_EIP207_CS_HASH_ID_FIELD_WORD_COUNT             \
                                    FIRMWARE_EIP207_CS_FLOW_HASH_ID_FIELD_WORD_COUNT

// Word offset of the Next Record Pointer field in the flow record
#define FIRMWARE_EIP207_CS_NEXT_RECORD_WORD_OFFSET              \
                                    FIRMWARE_EIP207_CS_FLOW_FR_NEXT_ADDR_WORD_OFFSET

// Word offset of the Transform Record Pointer field in the flow record
#define FIRMWARE_EIP207_CS_XFORM_RECORD_WORD_OFFSET             \
                                    FIRMWARE_EIP207_CS_FLOW_FR_XFORM_OFFS_WORD_OFFSET

// Word offset of the ARC4 State Record Pointer field in the flow record
#define FIRMWARE_EIP207_CS_ARC4_RECORD_WORD_OFFSET              \
                                    FIRMWARE_EIP207_CS_FLOW_FR_ARC4_ADDR_WORD_OFFSET

// Classification Engine clocks per one tick for
// the blocking next command logic
typedef enum
{
    FIRMWARE_EIP207_CS_BLOCK_CLOCKS_16 = 0,
    FIRMWARE_EIP207_CS_BLOCK_CLOCKS_32,
    FIRMWARE_EIP207_CS_BLOCK_CLOCKS_64,
    FIRMWARE_EIP207_CS_BLOCK_CLOCKS_128,
    FIRMWARE_EIP207_CS_BLOCK_CLOCKS_256,
    FIRMWARE_EIP207_CS_BLOCK_CLOCKS_512,
    FIRMWARE_EIP207_CS_BLOCK_CLOCKS_1024,
    FIRMWARE_EIP207_CS_BLOCK_CLOCKS_2048,
} FIRMWARE_EIP207_CS_BlockClocks_t;

// Disable the "block next command" logic
#define FIRMWARE_EIP207_CS_BLOCK_NEXT_COMMAND_LOGIC_DISABLE

// A blocked Record Cache command will be released automatically after 3 ticks
// of a free-running timer whose speed is set by this field.
// Value 0 ticks every engine clock (for debugging), other values M
// in the range 1...7 generate one tick every 2^(M+4) engine clocks.
// Default value: 1 tick per 32 engine clocks, see also FIRMWARE_EIP207_CS_BlockClocks_t
#ifndef FIRMWARE_EIP207_CS_BLOCK_NEXT_COMMAND_LOGIC_DISABLE
#define FIRMWARE_EIP207_CS_FRC_BLOCK_TIMEBASE                   FIRMWARE_EIP207_CS_BLOCK_CLOCKS_32
#define FIRMWARE_EIP207_CS_TRC_BLOCK_TIMEBASE                   FIRMWARE_EIP207_CS_BLOCK_CLOCKS_32
#define FIRMWARE_EIP207_CS_ARC4RC_BLOCK_TIMEBASE                FIRMWARE_EIP207_CS_BLOCK_CLOCKS_32
#endif // FIRMWARE_EIP207_CS_BLOCK_NEXT_COMMAND_LOGIC_DISABLE

// Administration RAM byte offsets as opposed to PE_n_ICE_SCRATCH_RAM
// 1 KB memory area base address in Classification Engine n
#define FIRMWARE_EIP207_CS_WORD_OFFS                            4

#define FIRMWARE_EIP207_CS_VERSION_BASE                          \
                    ((0 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_IPUE_VER_CAP_BYTE_OFFSET    \
                    ((0 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_IPUE_CAP_BYTE_OFFSET        \
                    ((1 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_IFPP_VER_CAP_BYTE_OFFSET    \
                    ((2 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_IFPP_CAP_BYTE_OFFSET        \
                    ((3 * FIRMWARE_EIP207_CS_WORD_OFFS))

// Administration RAM byte offsets as opposed to PE_n_OCE_SCRATCH_RAM
// 1 KB memory area base address in Classification Engine n
#define FIRMWARE_EIP207_CS_ADMIN_RAM_OPUE_VER_CAP_BYTE_OFFSET    \
                    ((0 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_OPUE_CAP_BYTE_OFFSET        \
                    ((1 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_OFPP_VER_CAP_BYTE_OFFSET    \
                    ((2 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_OFPP_CAP_BYTE_OFFSET        \
                    ((3 * FIRMWARE_EIP207_CS_WORD_OFFS))

// Trace windows in PE_n_ICE_SCRATCH_RAM
#define FIRMWARE_EIP207_CS_TRACE_WINDOW_BASE                         \
                    ((64 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_IPUE_TRACE_WINDOW_BYTE_OFFSET   \
                    ((64 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_IFPP_TRACE_WINDOW_BYTE_OFFSET   \
                    ((80 * FIRMWARE_EIP207_CS_WORD_OFFS))

// Trace windows in PE_n_OCE_SCRATCH_RAM
#define FIRMWARE_EIP207_CS_ADMIN_RAM_OPUE_TRACE_WINDOW_BYTE_OFFSET   \
                    ((64 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_OFPP_TRACE_WINDOW_BYTE_OFFSET   \
                    ((80 * FIRMWARE_EIP207_CS_WORD_OFFS))

// Flow and Transform record size word byte offset
#define FIRMWARE_EIP207_CS_ADMIN_RAM_REC_SIZE_BYTE_OFFSET       \
                    ((4 * FIRMWARE_EIP207_CS_WORD_OFFS))

// Global statistics
#define FIRMWARE_EIP207_CS_GLOBAL_STAT_BASE                     \
                    ((12 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_TIME_LO_BYTE_OFFSET        \
                    ((12 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_TIME_HI_BYTE_OFFSET        \
                    (FIRMWARE_EIP207_CS_ADMIN_RAM_TIME_LO_BYTE_OFFSET + (0x01 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_STAT_DROP_LO_BYTE_OFFSET   \
                    ((14 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_STAT_DROP_HI_BYTE_OFFSET   \
                    (FIRMWARE_EIP207_CS_ADMIN_RAM_STAT_DROP_LO_BYTE_OFFSET + (0x01 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_STAT_IN_OCT_LO_BYTE_OFFSET \
                    ((16 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_STAT_IN_OCT_HI_BYTE_OFFSET \
                    (FIRMWARE_EIP207_CS_ADMIN_RAM_STAT_IN_OCT_LO_BYTE_OFFSET + (0x01 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_STAT_OUT_OCT_LO_BYTE_OFFSET \
                    ((18 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_STAT_OUT_OCT_HI_BYTE_OFFSET \
                    (FIRMWARE_EIP207_CS_ADMIN_RAM_STAT_OUT_OCT_LO_BYTE_OFFSET + (0x01 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_STAT_IN_PKT_BYTE_OFFSET    \
                    ((20 * FIRMWARE_EIP207_CS_WORD_OFFS))
#define FIRMWARE_EIP207_CS_ADMIN_RAM_STAT_OUT_PKT_BYTE_OFFSET   \
                    ((21 * FIRMWARE_EIP207_CS_WORD_OFFS))

#define FIRMWARE_EIP207_CS_ADMIN_RAM_INPUT_LAST_BYTE_COUNT \
                    (FIRMWARE_EIP207_CS_ADMIN_RAM_STAT_OUT_PKT_BYTE_OFFSET + 4)

#define FIRMWARE_EIP207_CS_ADMIN_RAM_OUTPUT_LAST_BYTE_COUNT \
                    (FIRMWARE_EIP207_CS_ADMIN_RAM_REC_SIZE_BYTE_OFFSET + 4)

// token format
#define FIRMWARE_EIP207_CS_ADMIN_RAM_TOKEN_FORMAT_BYTE_OFFSET   (24 * FIRMWARE_EIP207_CS_WORD_OFFS)

// DTLS record alignment (ICE admin RAM)
#define FIRMWARE_EIP207_CS_ADMIN_RAM_DTLS_HDR_SIZE_BYTE_OFFSET  (11 * FIRMWARE_EIP207_CS_WORD_OFFS)

// Redir (OCE admin RAM)
#define FIRMWARE_EIP207_CS_ADMIN_RAM_REDIR_BYTE_OFFSET  (8 * FIRMWARE_EIP207_CS_WORD_OFFS)

// ECN control (OCE admin RAM)
#define FIRMWARE_EIP207_CS_ADMIN_RAM_ECN_CONTROL_BYTE_OFFSET  (9 * FIRMWARE_EIP207_CS_WORD_OFFS)

// PKTID (OCE admin RAM)
#define FIRMWARE_EIP207_CS_ADMIN_RAM_PKTID_BYTE_OFFSET  (7 * FIRMWARE_EIP207_CS_WORD_OFFS)

// ARC4 offset control
#define FIRMWARE_EIP207_CS_ADMIN_RAM_ARC4_OFFSET_OFFSET         (25 * FIRMWARE_EIP207_CS_WORD_OFFS)

/*----------------------------------------------------------------------------
 * Firmware helper functions
 */

/*----------------------------------------------------------------------------
 * FIRMWARE_EIP207_CS_RecordSize_Read
 *
 * This function reads the EIP-207 firmware flow, transform and ARC4 state
 * record size from the provided 32-bit value
 *
 * Value (input)
 *     32-bit value that can be read from the Administration RAM byte offset
 *     FIRMWARE_EIP207_CS_ADMIN_RAM_REC_SIZE_BYTE_OFFSET
 *
 * FlowRec_ByteCount_p (output)
 *     Pointer to the memory where the flow record size in 32-bit words
 *     will be stored
 *
 * XformRec_ByteCount_p (output)
 *     Pointer to the memory where the transform record size in 32-bit words
 *     will be stored
 *
 * Return value
 *     None
 */
static inline void
FIRMWARE_EIP207_CS_RecordSize_Read(
        const uint32_t Value,
        unsigned int * const FlowRec_ByteCount_p,
        unsigned int * const XformRec_ByteCount_p)
{
    *XformRec_ByteCount_p = (unsigned int)((Value >> 16) & MASK_16_BITS);
    *FlowRec_ByteCount_p  = (unsigned int)((Value)       & MASK_16_BITS);
}


#endif /* FIRMWARE_EIP207_API_CS_H_ */


/* end of file firmware_eip207_api_cs.h */
