/* eip202_ring_types.h
 *
 * EIP-202 Ring Control Driver Library Public Interface:
 * Common Type Definitions for CDR and RDR
 *
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : ddk197                                                   */
/*   Version       : 5.6.1                                                    */
/*   Configuration : DDK-197-GPL                                              */
/*                                                                            */
/*   Date          : 2022-Dec-16                                              */
/*                                                                            */
/* Copyright (c) 2008-2022 by Rambus, Inc. and/or its subsidiaries.           */
/*                                                                            */
/* This program is free software: you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation, either version 2 of the License, or          */
/* any later version.                                                         */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               */
/* GNU General Public License for more details.                               */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program. If not, see <http://www.gnu.org/licenses/>.       */
/* -------------------------------------------------------------------------- */

#ifndef INCLUDE_GUARD_EIP202_RING_TYPES_H
#define INCLUDE_GUARD_EIP202_RING_TYPES_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"             // uint32_t

// Driver Framework DMA Resource API
#include "dmares_types.h"         // types of the DMA resource API


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// I/O Area size for one CDR or RDR instance
#define EIP202_IOAREA_REQUIRED_SIZE           300


/*----------------------------------------------------------------------------
 * EIP202_Ring_Error_t
 *
 * Status (error) code type returned by these API functions
 * See each function "Return value" for details.
 *
 * EIP202_RING_NO_ERROR : successful completion of the call.
 * EIP202_RING_UNSUPPORTED_FEATURE_ERROR : not supported by the device.
 * EIP202_RING_ARGUMENT_ERROR :  invalid argument for a function parameter.
 * EIP202_RING_BUSY_RETRY_LATER : Device is busy.
 * EIP202_RING_ILLEGAL_IN_STATE : illegal state transition
 */
typedef enum
{
    EIP202_RING_NO_ERROR = 0,
    EIP202_RING_UNSUPPORTED_FEATURE_ERROR,
    EIP202_RING_ARGUMENT_ERROR,
    EIP202_RING_ILLEGAL_IN_STATE
} EIP202_Ring_Error_t;

// place holder for device specific internal data
typedef struct
{
    uint32_t placeholder[EIP202_IOAREA_REQUIRED_SIZE];
} EIP202_Ring_IOArea_t;

typedef enum
{
    // Endianness Conversion (EC) in the EIP-202 HW master interface setting
    // for different types of DMA data.
    // Default setting is 0 which means that the EC in the EIP-202 hardware
    // is disabled.
    EIP202_RING_BYTESWAP_TOKEN           = BIT_0, // Enables Token Data EC
    EIP202_RING_BYTESWAP_DESCRIPTOR      = BIT_1, // Enables Descriptor Data EC
    EIP202_RING_BYTESWAP_PACKET          = BIT_2  // Enables Packet Data EC
} EIP202_Ring_DMA_ByteSwap_DataType_t;

// Bit-mask for the events defined by EIP202_Ring_DMA_ByteSwap_DataType_t
typedef uint32_t EIP202_Ring_DMA_ByteSwap_DataType_Mask_t;

// Endianness Conversion method
// One (or several) of the following methods must be configured in case
// the endianness conversion is enabled for one (or several) of
// the DMA data types:
typedef enum
{
    // Swap bytes within each 32 bit word
    EIP202_RING_BYTE_SWAP_METHOD_32    = BIT_0,

    // Swap 32 bit chunks within each 64 bit chunk
    EIP202_RING_BYTE_SWAP_METHOD_64    = BIT_1,

    // Swap 64 bit chunks within each 128 bit chunk
    EIP202_RING_BYTE_SWAP_METHOD_128   = BIT_2,

    // Swap 128 bit chunks within each 256 bit chunk
    EIP202_RING_BYTE_SWAP_METHOD_256   = BIT_3
} EIP202_Ring_DMA_ByteSwap_Method_t;

// Bit-mask for the events defined by EIP202_Ring_DMA_ByteSwap_DataType_t
typedef uint32_t EIP202_Ring_DMA_ByteSwap_Method_Mask_t;

// Physical (bus) address that can be used for DMA by the device
typedef struct
{
    // 32-bit physical bus address
    uint32_t Addr;

    // upper 32-bit part of a 64-bit physical address
    // Note: this value has to be provided only for 64-bit addresses,
    // in this case Addr field provides the lower 32-bit part
    // of the 64-bit address, for 32bit addresses this field is ignored,
    // and should be set to 0.
    uint32_t UpperAddr;

} EIP202_DeviceAddress_t;

// 64-bit DMA address support
typedef enum
{
    EIP202_RING_64BIT_DMA_DISABLED,  // 32-bit DMA addresses in descriptors only
    EIP202_RING_64BIT_DMA_DSCR_PTR,  // 64-bit DMA addresses in descriptors
    EIP202_RING_64BIT_DMA_EXT_ADDR   // Enables extended DMA addresses, e.g.
                                    // 32-bit DMA addresses in descriptors
                                    // are added to a 64-bit base address
                                    // (only one 64-bit base address can be
                                    // defined for tokens and one for
                                    // packet data )
} EIP202_Ring_DMA_Address_Mode_t;

// The EIP-202 HW master interface hardware data width
typedef enum
{
    EIP202_RING_DMA_ALIGNMENT_4_BYTES   = 0, // 4 bytes
    EIP202_RING_DMA_ALIGNMENT_8_BYTES   = 1, // 8 bytes
    EIP202_RING_DMA_ALIGNMENT_16_BYTES  = 2, // 16 bytes
    EIP202_RING_DMA_ALIGNMENT_32_BYTES  = 3  // 32 bytes
} EIP202_Ring_DMA_Alignment_t;

// Bufferability control
typedef enum
{
    // Enables bufferability control for ownership word DMA writes
    EIP202_RING_BUF_CTRL_OWN_DMA_WRITES              = BIT_0,

    // Enables bufferability control for write/Read cache type control
    EIP202_RING_BUF_CTRL_WR_RD_CACHE                 = BIT_1,

    // Enables bufferability control for Result Token DMA writes
    // Note: this can be enabled for RDR only
    EIP202_RING_BUF_CTRL_RESULT_TOKEN_DMA_WRITES     = BIT_2,

    // Enables bufferability control for Descriptor Control Word DMA writes
    // Note: this can be enabled for RDR only
    EIP202_RING_BUF_CTRL_RDR_CONTROL_DMA_WRITES      = BIT_3

} EIP202_Ring_Bufferability_Control_t;

// Bit-mask for the events defined by EIP202_Ring_Bufferability_Control_t
typedef uint32_t    EIP202_Ring_Bufferability_Settings_t;

// Autonomous Ring Mode (ARM) descriptor ring settings
typedef struct
{
    // Master Data Bus Width (in 32-bit words)
    uint32_t                                DataBusWidthWordCount;

    // Endianness Conversion (EC) settings
    // Mask enables the EC per DMA data type:
    // 1) Token, 2) Descriptor and 3) Packet data.
    EIP202_Ring_DMA_ByteSwap_DataType_Mask_t ByteSwap_DataType_Mask;

    // Endianness Conversion method per DMA data type
    EIP202_Ring_DMA_ByteSwap_Method_Mask_t   ByteSwap_Token_Mask;
    EIP202_Ring_DMA_ByteSwap_Method_Mask_t   ByteSwap_Descriptor_Mask;
    EIP202_Ring_DMA_ByteSwap_Method_Mask_t   ByteSwap_Packet_Mask;

    // Bufferability control
    EIP202_Ring_Bufferability_Settings_t     Bufferability;

    // Enable 64-bit DMA address support
    // Two different 64-bit DMA modes exist:
    //      1) 64-bit descriptor pointer and
    //      2) extended addressing.
    EIP202_Ring_DMA_Address_Mode_t           DMA_AddressMode;

    // Ring Size, in 32-bit words:
    //      The minimum value is the “Command Descriptor Offset”, see below.
    //      The maximum value is 4194303
    //      (setting a ring size of one word below 16M Byte).
    uint32_t                                RingSizeWordCount;

    // Ring DMA resource handle
    DMAResource_Handle_t                    RingDMA_Handle;

    // Ring physical address that can be used for DMA
    EIP202_DeviceAddress_t                   RingDMA_Address;

    // Command/Result Descriptor Size (in 32-bit words)
    // 1) For CDR:
    // The minimum value in 32-bit address mode is 3 without the Additional
    // Token Pointer (ATP) and 4 with ATP.
    // The minimum value in 64-bit address mode is 5 without ATP and 7 with ATP.
    // The maximum value is the “Command Descriptor FIFO Size”,
    // independent of the addressing mode.
    // 2) For RDR:
    // The minimum value is 3 for 32-bit addressable systems and
    // 5 for 64-bit addressable systems.
    // The maximum value is the “Result Descriptor FIFO Size”,
    // independent of the addressing mode.
    uint32_t                                DscrSizeWordCount;

    // Descriptor Offset (in 32-bit words)
    // Can be used to align descriptor size for cache-line size, for example)
    uint32_t                                DscrOffsWordCount;

    // Offset of result token with respect to start of descriptor.
    uint32_t                                TokenOffsetWordCount;

    // Command/Result Descriptor Fetch Size (in 32-bit words)
    // The number of 32-bit words of Descriptor data that
    // the Ring Manager attempts to fetch from the ring in Host memory in one
    // contiguous block to the internal FIFO. The Ring Manager waits until
    // the “Descriptor Fetch Threshold” (see below) is exceeded.
    // Then it fetches “Descriptor Fetch Size” 32-bit words or as many
    // words of descriptor data as available, from the ring.
    // This value must be an integer multiple of the “Descriptor Offset”
    // and should not exceed the “Descriptor FIFO Size”.
    uint32_t                                DscrFetchSizeWordCount;

    // Command/Result Descriptor Fetch Threshold (in 32-bit words)
    // In Autonomous Ring Mode this is the number of 32-bit words that must be
    // free in the Descriptor FIFO before it fetches a new block of
    // Descriptors. This value must be an integer multiple of
    // the “Descriptor Size” rounded up to a whole multiple of
    // the “Master Data Bus Width”, and should not exceed the
    // “Descriptor FIFO Size”.
    // Example: if the “Command Descriptor FIFO Size” is 5 words and the master
    //          data bus width is 128 bits (4 words), then this register must
    //          be programmed to at least value 8, since that is the next
    //          integer multiple of 4 after 5.
    uint32_t                                DscrThresholdWordCount;

    // Command/Result Descriptor Interrupt Threshold
    // 1) For CDR  (in Descriptors):
    // The value in this field is compared with the number of prepared Command
    // Descriptors pending in the CDR. When that number is less than or equal
    // to the value set in this field the cd_thresh_irq interrupt fires.
    // 2) For RDR  (in Descriptors or in Packets):
    // The rd_proc_thresh_irq interrupt is fired when the number of
    // the Processed Result Descriptors/Packets for the RDR exceeds the value
    // set by this parameter. The maximum number of packets is 127.
    uint32_t                                IntThresholdDscrCount;

    // Command/Result Descriptor Interrupt Timeout
    // (units of 256 clock cycles of the HIA clock)
    // 1) For CDR:
    // The cd_timeout_irq interrupt is fired when the number of prepared
    // command descriptors pending in the CDR is non-zero and not decremented
    // for the amount of time specified by this value.
    // 2) For RDR:
    // The rd_proc_timeout_irq interrupt is fired when the number of Processed
    // Result Descriptors pending in the RDR is non-zero, remains constant and
    // has not reached the value specified by the “Result Descriptor Interrupt
    // Threshold” parameter for the amount of time specified by this value.
    // 3) For CDR and RDR:
    // A value of zero disables the interrupt and stops the timeout counter
    // (useful to reduce power consumption).
    uint32_t                                IntTimeoutDscrCount;

} EIP202_ARM_Ring_Settings_t;

// EIP-202 HW options
typedef struct
{
    // Number of statically configured Descriptor Rings
    uint8_t NofRings;

    // Number of statically configured Processing Engines,
    // value 0 indicates 8 PEs
    uint8_t NofPes;

    // If true then 64 bit descriptors will contain a particle
    // size/fill level extension word to allow particle sizes larger
    // than 1 MB.
    bool fExpPlf;

    // Command Descriptor FIFO size, the actual size is 2^CF_Size 32-bit
    // words.
    uint8_t CF_Size;

    // Result Descriptor FIFO size, the actual size is 2^RF_Size 32-bit
    // words.
    uint8_t RF_Size;

    // Host interface type:
    // 0 = PLB, 1 = AHB, 2 = TCM, 3 = AXI
    uint8_t HostIfc;

    // Maximum supported DMA length is 2^(DMA_Len+1) – 1 bytes
    uint8_t DMA_Len;

    // Host interface data width:
    // 0 = 32 bits, 1 = 64 bits, 2 = 128 bits, 3 = 256 bits
    uint8_t HDW;

    // Target access block alignment. If this value is larger than 0,
    // the distance between 2 rings, 2 AICs and between the DFE and the DSE
    // in the slave memory map is increased by a factor of 2^TgtAlign.
    // This means that ring control registers start at 2^(TgtAlign+11) Byte
    // boundaries (or a 2^(TgtAlign+12) Byte boundary for a combined
    // CDR/RDR block), the DFE and DSE will start at a 2^(TgtAlign+10) Byte
    // boundary and AICs will start at 2^(TgtAlign+8) Byte boundaries.
    uint8_t TgtAlign;

    // 64-bit addressing mode:
    // false = 32-bit addressing,
    // true = 64-bit addressing
    bool fAddr64;
} EIP202_Ring_Options_t;

// Ring Admin data
typedef struct
{
    unsigned int RingSizeWordCount;
    unsigned int DescOffsWordCount;

    unsigned int IN_Size;
    unsigned int IN_Tail;

    unsigned int OUT_Size;
    unsigned int OUT_Head;

    bool fSeparate;
} EIP202_RingAdmin_t;


#endif /* INCLUDE_GUARD_EIP202_RING_TYPES_H */


/* end of file eip202_ring_types.h */
