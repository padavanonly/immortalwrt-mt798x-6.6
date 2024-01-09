/* firmware_eip207_api_flow_cs.h
 *
 * EIP-207 Firmware Classification API:
 * Flow Control functionality,
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

#ifndef FIRMWARE_EIP207_API_FLOW_CS_H_
#define FIRMWARE_EIP207_API_FLOW_CS_H_


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */



/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "basic_defs.h"     // uint32_t

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Size of the Flow Record in 32-bit words
#define FIRMWARE_EIP207_CS_FLOW_FRC_RECORD_WORD_COUNT                       16

// Size of the Transform Record in 32-bit words
#define FIRMWARE_EIP207_CS_FLOW_TRC_RECORD_WORD_COUNT                       64
#define FIRMWARE_EIP207_CS_FLOW_TRC_RECORD_WORD_COUNT_LARGE                 80

// Size of the ARC4 Record in 32-bit words
#define FIRMWARE_EIP207_CS_FLOW_ARC4RC_RECORD_WORD_COUNT                    64
#define FIRMWARE_EIP207_CS_FLOW_ARC4RC_RECORD_WORD_COUNT_LARGE              64

// Word offset for ARC4 state record physical address
#define FIRMWARE_EIP207_CS_FLOW_FR_ARC4_ADDR_WORD_OFFSET                    6

// Flow ID field length in 32-bit words
#define FIRMWARE_EIP207_CS_FLOW_HASH_ID_FIELD_WORD_COUNT                    4

/*
 * Flow Record field offsets
 */

 // Word offset for transform record physical address offset relative to cache base address
#define FIRMWARE_EIP207_CS_FLOW_FR_XFORM_OFFS_WORD_OFFSET                   4

// Word offset for transform record physical address
#define FIRMWARE_EIP207_CS_FLOW_FR_XFORM_ADDR_WORD_OFFSET                   5

// Word offset for transform record physical address
#define FIRMWARE_EIP207_CS_FLOW_FR_XFORM_ADDR_HI_WORD_OFFSET                6

// Word offset for software flow record host (virtual) address
#define FIRMWARE_EIP207_CS_FLOW_FR_SW_ADDR_WORD_OFFSET                      8

// Word offset for Flags field
#define FIRMWARE_EIP207_CS_FLOW_FR_FLAGS_WORD_OFFSET                        9

// Word offset for MTU/Interface ID field
#define FIRMWARE_EIP207_CS_FLOW_FR_MTU_IFC_WORD_OFFSET                      3

// Word offset for Next Hop MAC field (size of field is 3 words).
#define FIRMWARE_EIP207_CS_FLOW_FR_NEXTHOP_MAC_WORD_OFFSET                  7

// Word offset for NAT ports
#define FIRMWARE_EIP207_CS_FLOW_FR_NAT_PORTS_WORD_OFFSET                    8

// Word offset for NAT source address
#define FIRMWARE_EIP207_CS_FLOW_FR_NAT_SRC_WORD_OFFSET                      9

// Word offset for NAT destination address
#define FIRMWARE_EIP207_CS_FLOW_FR_NAT_DST_WORD_OFFSET                      3

// Word offset for time stamp 64-bit value, low 32-bits
#define FIRMWARE_EIP207_CS_FLOW_FR_TIME_STAMP_LO_WORD_OFFSET                12

// Word offset for time stamp 64-bit value, high 32-bits
#define FIRMWARE_EIP207_CS_FLOW_FR_TIME_STAMP_HI_WORD_OFFSET                \
                          (FIRMWARE_EIP207_CS_FLOW_FR_TIME_STAMP_LO_WORD_OFFSET + 1)

// Word offset for octets statistics 64-bit value, low 32-bits
#define FIRMWARE_EIP207_CS_FLOW_FR_STAT_OCT_LO_WORD_OFFSET                  14

// Word offset for octets statistics 64-bit value, high 32-bits
#define FIRMWARE_EIP207_CS_FLOW_FR_STAT_OCT_HI_WORD_OFFSET                  \
                      (FIRMWARE_EIP207_CS_FLOW_FR_STAT_OCT_LO_WORD_OFFSET + 1)

// Word offset for packet statistics 32-bit value
#define FIRMWARE_EIP207_CS_FLOW_FR_STAT_PKT_WORD_OFFSET                     10

#define FIRMWARE_EIP207_CS_FLOW_FR_LAST_WORD_OFFSET                         \
                       (FIRMWARE_EIP207_CS_FLOW_FR_STAT_OCT_HI_WORD_OFFSET)

#if (FIRMWARE_EIP207_CS_FLOW_FR_LAST_WORD_OFFSET + 1) != \
                        FIRMWARE_EIP207_CS_FLOW_FRC_RECORD_WORD_COUNT
#error "Error: Firmware EIP-207 flow record offsets do not match its size"
#endif

/*
 * Transform Record field offsets
 */
// Maximum offset after storing all EIP96 context information, beyond which the
// large record size and associated offsets must be used.
#define FIRMWARE_EIP207_CS_FLOW_TR_LARGE_THRESHOLD_OFFSET                  56

// Offset of extension data from start of transform record.
#define FIRMWARE_EIP207_CS_FLOW_TR_EXTENSION_WORD_OFFSET                   56

// Word offset for CCM salt value
#define FIRMWARE_EIP207_CS_FLOW_TR_CCM_SALT_WORD_OFFSET                    56

// Word offset for pad aligment.
#define FIRMWARE_EIP207_CS_FLOW_TR_PAD_ALIGN_WORD_OFFSET                   50

// Word offset for transform flags.
#define FIRMWARE_EIP207_CS_FLOW_TR_FLAGS_WORD_OFFSET                       49

// Word offset for Token Verify Instruction field, 32-bit word
#define FIRMWARE_EIP207_CS_FLOW_TR_TK_VFY_INST_WORD_OFFSET                 54

// Word offset for Token Context Instruction field, 32-bit word
#define FIRMWARE_EIP207_CS_FLOW_TR_TK_CTX_INST_WORD_OFFSET                 55

// Word offset for NAT-T ports
#define FIMRWARE_EIP207_CS_FLOW_TR_NATT_PORTS_WORD_OFFSET                  53

// Word offset for time stamp 64-bit value, low 32-bits
#define FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_LO_WORD_OFFSET               58

// Word offset for time stamp 64-bit value, high 32-bits
#define FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_HI_WORD_OFFSET                \
                      (FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_LO_WORD_OFFSET + 1)

// Word offset for octets statistics 64-bit value, low 32-bits
#define FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_LO_WORD_OFFSET                 62

// Word offset for octets statistics 64-bit value, high 32-bits
#define FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_HI_WORD_OFFSET                  \
                      (FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_LO_WORD_OFFSET + 1)

// Word offset for packets statistics 32-bit value
#define FIRMWARE_EIP207_CS_FLOW_TR_STAT_PKT_WORD_OFFSET                    60

// Word offset for token header word.
#define FIRMWARE_EIP207_CS_FLOW_TR_TK_HDR_WORD_OFFSET                      48

// Word offset for various byte-sized parameters.
#define FIRMWARE_EIP207_CS_FLOW_TR_BYTE_PARAM_WORD_OFFSET                  52

// Word offset for header proc context pointer.
#define FIRMWARE_EIP207_CS_FLOW_TR_HDRPROC_CTX_WORD_OFFSET                 51

// Word offset for Tunnel IP source address
#define FIRMWARE_EIP207_CS_FLOW_TR_TUNNEL_SRC_WORD_OFFSET                  40

// Word offset for Tunnel IP destination address
#define FIRMWARE_EIP207_CS_FLOW_TR_TUNNEL_DST_WORD_OFFSET                  44 

// Word offset for Tunnel IPv4 checksum
#define FIRMWARE_EIP207_CS_FLOW_TR_CHECKSUM_WORD_OFFSET                    41

// Word offset for Path MTU field
#define FIRMWARE_EIP207_CS_FLOW_TR_PATH_MTU_WORD_OFFSET                    57

#define FIRMWARE_EIP207_CS_FLOW_TR_LAST_WORD_OFFSET                        FIRMWARE_EIP207_CS_FLOW_TR_HDRPROC_CTX_WORD_OFFSET

#if (FIRMWARE_EIP207_CS_FLOW_TR_LAST_WORD_OFFSET + 1) > \
                        FIRMWARE_EIP207_CS_FLOW_TRC_RECORD_WORD_COUNT
#error "Error: Firmware EIP-207 transform record offsets do not match its size"
#endif

/*
 * Transform Record field offsets for large records
 */

// Offset of extension data from start of transform record.
#define FIRMWARE_EIP207_CS_FLOW_TR_EXTENSION_WORD_OFFSET_LARGE          72

// Word offset for CCM salt value
#define FIRMWARE_EIP207_CS_FLOW_TR_CCM_SALT_WORD_OFFSET_LARGE           72

// Word offset for pad aligment.
#define FIRMWARE_EIP207_CS_FLOW_TR_PAD_ALIGN_WORD_OFFSET_LARGE          66

// Word offset for transform flags.
#define FIRMWARE_EIP207_CS_FLOW_TR_FLAGS_WORD_OFFSET_LARGE              65

// Word offset for Token Verify Instruction field, 32-bit word
#define FIRMWARE_EIP207_CS_FLOW_TR_TK_VFY_INST_WORD_OFFSET_LARGE        70

// Word offset for Token Context Instruction field, 32-bit word
#define FIRMWARE_EIP207_CS_FLOW_TR_TK_CTX_INST_WORD_OFFSET_LARGE       71

// Word offset for NAT-T ports
#define FIMRWARE_EIP207_CS_FLOW_TR_NATT_PORTS_WORD_OFFSET_LARGE        69

// Word offset for time stamp 64-bit value, low 32-bits
#define FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_LO_WORD_OFFSET_LARGE     74

// Word offset for time stamp 64-bit value, high 32-bits
#define FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_HI_WORD_OFFSET_LARGE     \
         (FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_LO_WORD_OFFSET_LARGE + 1)

// Word offset for octets statistics 64-bit value, low 32-bits
#define FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_LO_WORD_OFFSET_LARGE       78

// Word offset for octets statistics 64-bit value, high 32-bits
#define FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_HI_WORD_OFFSET_LARGE       \
         (FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_LO_WORD_OFFSET_LARGE + 1)

// Word offset for packets statistics 32-bit value
#define FIRMWARE_EIP207_CS_FLOW_TR_STAT_PKT_WORD_OFFSET_LARGE          76

// Word offset for token header word.
#define FIRMWARE_EIP207_CS_FLOW_TR_TK_HDR_WORD_OFFSET_LARGE            64

// Word offset for various byte-sized parameters.
#define FIRMWARE_EIP207_CS_FLOW_TR_BYTE_PARAM_WORD_OFFSET_LARGE        68

// Word offset for header proc context pointer.
#define FIRMWARE_EIP207_CS_FLOW_TR_HDRPROC_CTX_WORD_OFFSET_LARGE       67

// Word offset for Tunnel IP source address
#define FIRMWARE_EIP207_CS_FLOW_TR_TUNNEL_SRC_WORD_OFFSET_LARGE        56

// Word offset for Tunnel IP destination address
#define FIRMWARE_EIP207_CS_FLOW_TR_TUNNEL_DST_WORD_OFFSET_LARGE        60

// Word offset for Tunnel IPv4 checksum
#define FIRMWARE_EIP207_CS_FLOW_TR_CHECKSUM_WORD_OFFSET_LARGE          \
                      (FIRMWARE_EIP207_CS_FLOW_TR_TUNNEL_SRC_WORD_OFFSET_LARGE + 1)

// Word offset for Path MTU field
#define FIRMWARE_EIP207_CS_FLOW_TR_PATH_MTU_WORD_OFFSET_LARGE          73

#define FIRMWARE_EIP207_CS_FLOW_TR_LAST_WORD_OFFSET_LARGE                   \
                       (FIRMWARE_EIP207_CS_FLOW_TR_HDRPROC_CTX_WORD_OFFSET_LARGE)

#if (FIRMWARE_EIP207_CS_FLOW_TR_LAST_WORD_OFFSET_LARGE + 1) > \
                        FIRMWARE_EIP207_CS_FLOW_TRC_RECORD_WORD_COUNT_LARGE
#error "Error: Firmware EIP-207 large transform record offsets do not match its size"
#endif


/*
 * Flow hash ID calculation
 */

// The maximum size of the 32-bit word array that is used as data input for
// the flow hash ID calculation
#define FIRMWARE_EIP207_CS_FLOW_HASH_ID_INPUT_WORD_COUNT               13

// Flags
#define FIRMWARE_EIP207_CS_FLOW_SELECT_IPV4                            BIT_0
#define FIRMWARE_EIP207_CS_FLOW_SELECT_IPV6                            BIT_1
#define FIMRWARE_EIP207_CD_FLOW_SELECT_CUSTOM                          BIT_2
#define FIMRWARE_EIP207_CD_FLOW_ESP_WITH_SRC                           BIT_3

#define FIRMWARE_EIP207_CS_FLOW_DTLS_SUPPORTED                         1
// This data structure represents the packet parameters (such as IP addresses
// and ports) that select a particular flow.
typedef struct
{
    // Flags, see FIRMWARE_EIP207_CS_FLOW_SELECT_*
    uint32_t Flags;

    // IP protocol number
    uint8_t IpProto;

    // IP source address
    uint8_t * SrcIp_p;

    // IP destination address
    uint8_t * DstIp_p;

    // Source port for UDP
    uint16_t  SrcPort;

    // Destination port for UDP
    uint16_t  DstPort;

    // Custom selection ID.
    uint16_t  CustomID;

    // SPI in IPsec
    uint32_t  SPI;

    // Epoch for inbound DTLS
    uint16_t Epoch;

} FIRMWARE_EIP207_CS_Flow_SelectorParams_t;


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * FIRMWARE_EIP207_CS_Flow_Selectors_Reorder
 *
 * This function re-orders the selectors for the flow hash ID calculation.
 *
 * Selectors_p (input)
 *     Pointer to the data structure that contains the selectors from the
 *     packet header that can be used for the flow hash ID calculation.
 *
 * OutData_p (output)
 *     Pointer to the memory where the data arrays of 32-bit words
 *     will be stored. The buffer for the array must be of size
 *     FW207_CS_FLOW_HASH_ID_INPUT_WORD_COUNT.
 *
 * OutDataWordCount_p (output)
 *     Pointer to the memory where the data arrays size in 32-bit words
 *     will be stored.
 *
 * Return value
 *     None
 */
static inline void
FIRMWARE_EIP207_CS_Flow_Selectors_Reorder(
        const FIRMWARE_EIP207_CS_Flow_SelectorParams_t * const Selectors_p,
        uint32_t * OutData_p,
        unsigned int * const OutDataWordCount_p)
{
    unsigned int i = 0;

    // Word 0
    OutData_p[i++] = 0;

    // Flags and IP protocol number
    if ( Selectors_p->Flags & FIRMWARE_EIP207_CS_FLOW_SELECT_IPV6 )
        OutData_p[i++] = (uint32_t)(Selectors_p->IpProto << 8) |
                         (uint32_t)(BIT_25);
    else
        OutData_p[i++] = (uint32_t)(Selectors_p->IpProto << 8);

    // SPI
    OutData_p[i++] = Selectors_p->SPI;

    // Epoch
    OutData_p[i++] = Selectors_p->Epoch;

    if (Selectors_p->Flags & FIMRWARE_EIP207_CD_FLOW_SELECT_CUSTOM)
    {
        // Custom ID (used instead of port numbers)
        OutData_p[i++] = (uint32_t)Selectors_p->CustomID;
    }
    else if (Selectors_p->SPI == 0)
    {
        // L4 (TCP or UDP) destination and source port numbers
        OutData_p[i++] = (uint32_t)Selectors_p->SrcPort |
        (uint32_t)(Selectors_p->DstPort << 16);
    }
    else
    {
        OutData_p[i++] = 0;
    }

    // Destination IP address
    OutData_p[i++] = (uint32_t)Selectors_p->DstIp_p[0]         |
                     (uint32_t)(Selectors_p->DstIp_p[1] << 8)  |
                     (uint32_t)(Selectors_p->DstIp_p[2] << 16) |
                     (uint32_t)(Selectors_p->DstIp_p[3] << 24);

    if ( Selectors_p->Flags & FIRMWARE_EIP207_CS_FLOW_SELECT_IPV6 )
    {
        OutData_p[i++] = (uint32_t)Selectors_p->DstIp_p[4]         |
                         (uint32_t)(Selectors_p->DstIp_p[5] << 8)  |
                         (uint32_t)(Selectors_p->DstIp_p[6] << 16) |
                         (uint32_t)(Selectors_p->DstIp_p[7] << 24);
        OutData_p[i++] = (uint32_t)Selectors_p->DstIp_p[8]         |
                         (uint32_t)(Selectors_p->DstIp_p[9] << 8)  |
                         (uint32_t)(Selectors_p->DstIp_p[10] << 16)|
                         (uint32_t)(Selectors_p->DstIp_p[11] << 24);
        OutData_p[i++] = (uint32_t)Selectors_p->DstIp_p[12]        |
                         (uint32_t)(Selectors_p->DstIp_p[13] << 8) |
                         (uint32_t)(Selectors_p->DstIp_p[14] << 16)|
                         (uint32_t)(Selectors_p->DstIp_p[15] << 24);
    }
    else
    {
        OutData_p[i++] = 0;
        OutData_p[i++] = 0;
        OutData_p[i++] = 0;
    }

    // Source IP address
    if (Selectors_p->SPI != 0 && (Selectors_p->Flags & FIMRWARE_EIP207_CD_FLOW_ESP_WITH_SRC) == 0)
    {
        OutData_p[i++] = 0;
        OutData_p[i++] = 0;
        OutData_p[i++] = 0;
        OutData_p[i++] = 0;
    }
    else
    {
        OutData_p[i++] = (uint32_t)Selectors_p->SrcIp_p[0]         |
                         (uint32_t)(Selectors_p->SrcIp_p[1] << 8)  |
                         (uint32_t)(Selectors_p->SrcIp_p[2] << 16) |
                         (uint32_t)(Selectors_p->SrcIp_p[3] << 24);
        if ( Selectors_p->Flags & FIRMWARE_EIP207_CS_FLOW_SELECT_IPV6 )
        {
            OutData_p[i++] = (uint32_t)Selectors_p->SrcIp_p[4]         |
                             (uint32_t)(Selectors_p->SrcIp_p[5] << 8)  |
                             (uint32_t)(Selectors_p->SrcIp_p[6] << 16) |
                             (uint32_t)(Selectors_p->SrcIp_p[7] << 24);
            OutData_p[i++] = (uint32_t)Selectors_p->SrcIp_p[8]         |
                             (uint32_t)(Selectors_p->SrcIp_p[9] << 8)  |
                             (uint32_t)(Selectors_p->SrcIp_p[10] << 16)|
                             (uint32_t)(Selectors_p->SrcIp_p[11] << 24);
            OutData_p[i++] = (uint32_t)Selectors_p->SrcIp_p[12]        |
                             (uint32_t)(Selectors_p->SrcIp_p[13] << 8) |
                             (uint32_t)(Selectors_p->SrcIp_p[14] << 16)|
                             (uint32_t)(Selectors_p->SrcIp_p[15] << 24);
        }
        else
        {
            OutData_p[i++] = 0;
            OutData_p[i++] = 0;
            OutData_p[i++] = 0;
        }
    }

    *OutDataWordCount_p = i;
}


/*----------------------------------------------------------------------------
 * FIRMWARE_EIP207_CS_Flow_SeqNum_Offset_Read
 *
 * This function reads the word offset of the 32-bit Sequence Number field
 * from the provided Token Context Instruction 32-bit word.
 *
 * Value (input)
 *     Token Context Instruction 32-bit word.
 *
 * WordOffset_p (output)
 *     Pointer to the memory where the word offset of the 32-bit Sequence Number
 *     field will be stored.
 *
 * Return value
 *     None
 */
static inline void
FIRMWARE_EIP207_CS_Flow_SeqNum_Offset_Read(
        const uint32_t Value,
        unsigned int * const WordOffset_p)
{
    *WordOffset_p = (unsigned int)(Value & MASK_8_BITS);
}


#endif /* FIRMWARE_EIP207_API_FLOW_CS_H_ */


/* end of file firmware_eip207_api_flow_cs.h */
