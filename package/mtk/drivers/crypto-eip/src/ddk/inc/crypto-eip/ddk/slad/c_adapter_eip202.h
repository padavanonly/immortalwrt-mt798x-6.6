/* c_adapter_eip202.h
 *
 * Default Adapter EIP-202 configuration
 */

/*****************************************************************************
* Copyright (c) 2011-2022 by Rambus, Inc. and/or its subsidiaries.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#ifndef INCLUDE_GUARD_C_ADAPTER_EIP202_H
#define INCLUDE_GUARD_C_ADAPTER_EIP202_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level Adapter configuration
#include "cs_adapter.h"


// This parameter enables strict argument checking
//#define ADAPTER_EIP202_STRICT_ARGS

#ifndef ADAPTER_EIP202_GLOBAL_DEVICE_NAME
#define ADAPTER_EIP202_GLOBAL_DEVICE_NAME    "EIP202_GLOBAL"
#endif // ADAPTER_EIP202_GLOBAL_DEVICE_NAME

#ifndef ADAPTER_EIP202_DRIVER_NAME
#define ADAPTER_EIP202_DRIVER_NAME           "Security-IP"
#endif

// This parameter enables the byte swap in 32-bit words
// for the EIP-202 CD Manager master interface
//#define ADAPTER_EIP202_CDR_BYTE_SWAP_ENABLE

// This parameter enables the byte swap in 32-bit words
// for the EIP-202 RD Manager master interface
//#define ADAPTER_EIP202_RDR_BYTE_SWAP_ENABLE

// Enables the EIP-207 Record Cache interface for DMA banks
//#define ADAPTER_EIP202_RC_DMA_BANK_SUPPORT

// Enables the EIP-207 Record Cache interface for record cache invalidation
//#define ADAPTER_EIP202_RC_DIRECT_INVALIDATE_SUPPORT

// This parameter enables the EIP-207 Record Cache interface
// for the record invalidation via the SHDevXS API.
// When not defined the Record Cache will be accessed directly
// via the EIP-207 Record Cache Driver Library API.
// This parameter must be defined together with ADAPTER_EIP202_RC_SUPPORT
// for the Record Cache access via the SHDevXS API.
//#define ADAPTER_EIP202_USE_SHDEVXS

// This parameter enables the EIP-202 64-bit DMA address support
//#define ADAPTER_EIP202_64BIT_DEVICE

// This parameter enables the DMA banks support
//#define ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE

// This parameter enables the EIP-202 separate rings
// (CDR used separately from RDR)
//#define ADAPTER_EIP202_SEPARATE_RINGS

// This parameter enables the EIP-202 scatter-gather support
//#define ADAPTER_EIP202_ENABLE_SCATTERGATHER

// This parameter disables bounce buffers
//#define ADAPTER_EIP202_REMOVE_BOUNCEBUFFERS

#ifdef ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE

// This parameter configures the maximum number of transform records
#ifndef ADAPTER_EIP202_TRANSFORM_RECORD_COUNT
#error "ADAPTER_EIP202_TRANSFORM_RECORD_COUNT is not defined"
#endif

// This parameter configures the maximum byte count of one transform record
#ifndef ADAPTER_EIP202_TRANSFORM_RECORD_BYTE_COUNT
#error "ADAPTER_EIP202_TRANSFORM_RECORD_BYTE_COUNT is not defined"
#endif

#endif // ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE

// This parameter enables the EIP-202 interrupt support
//#define ADAPTER_EIP202_INTERRUPTS_ENABLE

#ifndef ADAPTER_EIP202_INTERRUPTS_TRACEFILTER
#define ADAPTER_EIP202_INTERRUPTS_TRACEFILTER   0
#endif

#ifndef ADAPTER_EIP202_PHY_CDR0_IRQ
#define ADAPTER_EIP202_PHY_CDR0_IRQ     0
#endif

#ifndef ADAPTER_EIP202_CDR0_INT_NAME
#define ADAPTER_EIP202_CDR0_INT_NAME    "EIP202-CDR0"
#endif

#ifndef ADAPTER_EIP202_PHY_RDR0_IRQ
#define ADAPTER_EIP202_PHY_RDR0_IRQ     1
#endif

#ifndef ADAPTER_EIP202_RDR0_INT_NAME
#define ADAPTER_EIP202_RDR0_INT_NAME    "EIP202-RDR0"
#endif

#ifndef ADAPTER_PHY_EIP202_DFE0_IRQ
#define ADAPTER_PHY_EIP202_DFE0_IRQ     0
#endif

#ifndef ADAPTER_EIP202_DFE0_INT_NAME
#define ADAPTER_EIP202_DFE0_INT_NAME    "EIP202-DFE0"
#endif

#ifndef ADAPTER_PHY_EIP202_DSE0_IRQ
#define ADAPTER_PHY_EIP202_DSE0_IRQ     1
#endif

#ifndef ADAPTER_EIP202_DSE0_INT_NAME
#define ADAPTER_EIP202_DSE0_INT_NAME    "EIP202-DSE0"
#endif

#ifndef ADAPTER_PHY_EIP202_RING0_IRQ
#define ADAPTER_PHY_EIP202_RING0_IRQ    16
#endif

#ifndef ADAPTER_EIP202_RING0_INT_NAME
#define ADAPTER_EIP202_RING0_INT_NAME   "EIP202-RING0"
#endif

#ifndef ADAPTER_PHY_EIP202_PE0_IRQ
#define ADAPTER_PHY_EIP202_PE0_IRQ      24
#endif

#ifndef ADAPTER_EIP202_PE0_INT_NAME
#define ADAPTER_EIP202_PE0_INT_NAME     "EIP202-PE0"
#endif

#ifndef ADAPTER_EIP202_MAX_PACKETS
#define ADAPTER_EIP202_MAX_PACKETS      32
#endif

#ifndef ADAPTER_EIP202_MAX_LOGICDESCR
#define ADAPTER_EIP202_MAX_LOGICDESCR   32
#endif

#ifndef ADAPTER_EIP202_BANK_SA
#define ADAPTER_EIP202_BANK_SA          0
#endif

#ifndef ADAPTER_EIP202_BANK_RING
#define ADAPTER_EIP202_BANK_RING        0
#endif

// This parameter enables the endianness conversion by the Host CPU
// for the ring descriptors
//#define ADAPTER_EIP202_ARMRING_ENABLE_SWAP

#ifndef ADAPTER_EIP202_DESCRIPTORDONECOUNT
#define ADAPTER_EIP202_DESCRIPTORDONECOUNT      1
#endif

#ifndef ADAPTER_EIP202_DESCRIPTORDONETIMEOUT
#define ADAPTER_EIP202_DESCRIPTORDONETIMEOUT    0
#endif

#ifndef ADAPTER_EIP202_DEVICE_COUNT
#define ADAPTER_EIP202_DEVICE_COUNT     1
#endif

#ifndef ADAPTER_EIP202_DEVICES
#error "ADAPTER_EIP202_DEVICES not defined"
#endif

#ifndef ADAPTER_EIP202_IRQS
#error "ADAPTER_EIP202_IRQS not defined"
#endif

//Set this if the IRQs for the second ring (Ring 1) are used.
//#define ADAPTER_EIP202_HAVE_RING1_IRQ

// Request the IRQ through the UMDevXS driver.
//#define ADAPTER_EIP202_USE_UMDEVXS_IRQ

// Enable this parameter to switch off the automatic calculation of the global
// and ring data transfer size and threshold values
//#define ADAPTER_EIP202_AUTO_THRESH_DISABLE

#ifdef ADAPTER_EIP202_AUTO_THRESH_DISABLE
//#define ADAPTER_EIP202_CDR_DSCR_FETCH_WORD_COUNT    16
//#define ADAPTER_EIP202_CDR_DSCR_THRESH_WORD_COUNT   12
//#define ADAPTER_EIP202_RDR_DSCR_FETCH_WORD_COUNT    80
//#define ADAPTER_EIP202_RDR_DSCR_THRESH_WORD_COUNT   20
#endif

// Provide manually CDR and RDR configuration parameters
//#define ADAPTER_EIP202_RING_MANUAL_CONFIGURE

// Derive CDR and RDR configuration parameters from local CDR options register
//#define ADAPTER_EIP202_RING_LOCAL_CONFIGURE

// Provide manually CDR and RDR configuration parameters
#ifdef ADAPTER_EIP202_RING_MANUAL_CONFIGURE
// Host interface data width:
//   0 = 32 bits, 1 = 64 bits, 2 = 128 bits, 3 = 256 bits
#ifndef ADAPTER_EIP202_HOST_DATA_WIDTH
#define ADAPTER_EIP202_HOST_DATA_WIDTH      0
#endif

// Command Descriptor FIFO size, the actual size is 2^CF_Size 32-bit words
#ifndef ADAPTER_EIP202_CF_SIZE
#define ADAPTER_EIP202_CF_SIZE              5
#endif

// Result Descriptor FIFO size, the actual size is 2^RF_Size 32-bit words
#ifndef ADAPTER_EIP202_RF_SIZE
#define ADAPTER_EIP202_RF_SIZE              5
#endif

#endif // ADAPTER_EIP202_RING_CONFIGURE

#ifndef ADAPTER_EIP202_CDR_DSCR_THRESH_WORD_COUNT
#define ADAPTER_EIP202_CDR_DSCR_THRESH_WORD_COUNT   6
#endif

#ifndef ADAPTER_EIP202_RDR_DSCR_FETCH_WORD_COUNT
#define ADAPTER_EIP202_RDR_DSCR_FETCH_WORD_COUNT    16
#endif

#ifndef ADAPTER_EIP202_RDR_DSCR_THRESH_WORD_COUNT
#define ADAPTER_EIP202_RDR_DSCR_THRESH_WORD_COUNT   2
#endif

// Define if the hardware uses bits in record pointers to distinguish types.
//#define ADAPTER_EIP202_USE_POINTER_TYPES

// Define if record invalidate commands allow NULL packet pointers
//#define ADAPTER_EIP202_INVALIDATE_NULL_PKT_POINTER

// Define if the hardware does not use large transforms.
//#define ADAPTER_EIP202_USE_LARGE_TRANSFORM_DISABLE

// Default DMA buffer allocation alignment is 4 bytes
#ifndef ADAPTER_EIP202_DMA_ALIGNMENT_BYTE_COUNT
#define ADAPTER_EIP202_DMA_ALIGNMENT_BYTE_COUNT     4
#endif

// Some applications allocate network packets at unaligned addresses.
// To avoid bouncing these buffers, you can allow unaligned buffers if
// caching is properly handled in hardware.
//#define ADAPTER_EIP202_ALLOW_UNALIGNED_DMA

// EIP-202 Ring manager device ID, keep undefined if RPM for EIP-202 not used
//#define ADAPTER_PEC_RPM_EIP202_DEVICE0_ID  0

// Default Command Descriptor offset (1 32-bit words)
// This value must be set to the CPU Data Cache line size if the Command
// Descriptor Ring is allocated in the cached DMA memory and configured as
// non-overlapping (separate) with the Result Descriptor Ring.
// Otherwise the Adapter will set the offset value equal to the Command
// Descriptor Size aligned for the required DMA buffer alignment and the
// DMA HW interface alignment (the latter only if configured).
#ifndef ADAPTER_EIP202_CD_OFFSET_BYTE_COUNT
#define ADAPTER_EIP202_CD_OFFSET_BYTE_COUNT     \
                        ADAPTER_EIP202_DMA_ALIGNMENT_BYTE_COUNT
#endif

// Default Result Descriptor offset (1 32-bit words)
// This value must be set to the CPU Data Cache line size if the Result
// Descriptor Ring is allocated in the cached DMA memory and configured as
// non-overlapping (separate) with the Command Descriptor Ring.
// Otherwise the Adapter will set the offset value equal to the Command
// Descriptor Size aligned for the required DMA buffer alignment and the
// DMA HW interface alignment (the latter only if configured).
#ifndef ADAPTER_EIP202_RD_OFFSET_BYTE_COUNT
#define ADAPTER_EIP202_RD_OFFSET_BYTE_COUNT     \
                        ADAPTER_EIP202_DMA_ALIGNMENT_BYTE_COUNT
#endif

// Additional Token Pointer Descriptor Mode, e.g. EIP-202 Command Descriptor
// will contain an address of the DMA buffer where the EIP-96 Instruction Token
// is stored.
// When set to 1 the token data can be stored in a separate from the descriptor
// DMA buffer, otherwise must be set to 0
#ifndef ADAPTER_EIP202_CDR_ATP_PRESENT
#define ADAPTER_EIP202_CDR_ATP_PRESENT      1
#endif


#endif /* Include Guard */


/* end of file c_adapter_eip202.h */
