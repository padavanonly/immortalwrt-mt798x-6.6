/* c_eip97_global.h
 *
 * Default configuration parameters
 * for the EIP-97 Global Control Driver Library
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

#ifndef C_EIP97_GLOBAL_H_
#define C_EIP97_GLOBAL_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level configuration
#include "cs_eip97_global.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// EIP-97 supported HW version
#define EIP97_GLOBAL_MAJOR_VERSION           1
#define EIP97_GLOBAL_MINOR_VERSION           0
#define EIP97_GLOBAL_PATCH_LEVEL             0

// EIP-202 supported HW version
#define EIP202_GLOBAL_MAJOR_VERSION          1
#define EIP202_GLOBAL_MINOR_VERSION          0
#define EIP202_GLOBAL_PATCH_LEVEL            0

// EIP-96 supported HW version
#define EIP96_GLOBAL_MAJOR_VERSION           2
#define EIP96_GLOBAL_MINOR_VERSION           0
#define EIP96_GLOBAL_PATCH_LEVEL             3

// Enables aggressive DMA mode. Set to 1 for optimal performance.
#ifndef EIP97_GLOBAL_DFE_AGGRESSIVE_DMA_FLAG
#define EIP97_GLOBAL_DFE_AGGRESSIVE_DMA_FLAG   1
#endif

// Enables DSE aggressive DMA mode. Set to 1 for optimal performance.
#ifndef EIP97_GLOBAL_DSE_AGGRESSIVE_DMA_FLAG
#define EIP97_GLOBAL_DSE_AGGRESSIVE_DMA_FLAG   1
#endif

// Bus burst size (bus type specific, AHB, AXI, PLB)
// AHB:
// Determines the maximum burst size that will be used on the AHB bus,
// values „n‟ in the range 0...4 select 2^n beat maximum bursts (i.e. from
// 1 to 16 beats) – other values are reserved and must not be used. The
// system reset value of 4 selects 16 beat bursts.
// AXI:
// Determines the maximum burst size that will be used on the AXI
// interface, values „n‟ in the range 1...4 select 2^n beat maximum bursts
// (i.e. from 1 to 16 beats) – other values are reserved and must not be
// used. The system reset value of 4 selects 16 beat bursts.
// Maximum values:
//    0100b - AXI v3 and AXI v4
// PLB (YYZZb):
// ZZ: Selects the maximum burst size on the PLB bus, value in range 00b – 11b:
// 00b = 2 words, 01b = 4 words, 10b2 = 8 words, 11b = 16 words.
// YY: Set data swap, value in range 00b – 11b:
//     00b - swap 8-bit chunks within each 16-bit word
//     01b - swap 8-bit chunks within each 32-bit word
//     10b - swap 8-bit chunks within each 64-bit word (only if HDW>=001b)
//     11b - swap 8-bit chunks within each 128-bit word (only if HDW>=010b)
// This field cannot be changed when PLB Timeout, Write or Read error occurs
#ifndef EIP97_GLOBAL_BUS_BURST_SIZE
#define EIP97_GLOBAL_BUS_BURST_SIZE          7 // 7 for PLB
#endif

// Determines the maximum burst size that will be used on the receive side
// of the AXI interface or
// secondary requesting and priority for the PLB interface
#ifndef EIP97_GLOBAL_RX_BUS_BURST_SIZE
#define EIP97_GLOBAL_RX_BUS_BURST_SIZE       5 // 5 for PLB
#endif

// For AXI bus only, for non-AXI bus must be 0!
// This field controls when an AXI read channel Master timeout Irq will be
// generated. The minimum value is 1. The actual timeout fires when the last
// data for a read transfer has not arrived within Timeout*(2^26) clock
// cycles after the read command has been transferred.
#ifndef EIP97_GLOBAL_TIMEOUT_VALUE
#define EIP97_GLOBAL_TIMEOUT_VALUE           0xF
#endif

// Set this option to enable swapping of bytes in 32-bit words written via
// the Packet Engine slave interface, e.g. device registers.
//#define EIP97_GLOBAL_ENABLE_SWAP_REG_DATA

// Set this option to disable the endianness conversion by the host
// processor of the first words written to the Packet Engine registers
// during its initialization.
// If endianness conversion is configured for the Packet Engine Slave
// interface (by defining the EIP97_GLOBAL_ENABLE_SWAP_REG_DATA parameter)
// then also configure whether the endianness conversion is required
// for the very first registers written during the Packet Engine
// initialization, e.g. the Packet Engine registers used to reset
// the engine and activate the endianness conversion for the Slave interface.
//#define EIP97_GLOBAL_DISABLE_HOST_SWAP_INIT

// Enable Flow Lookup Data Endianness Conversion
// by the Classification Engine hardware master interface
//#define EIP97_GLOBAL_BYTE_SWAP_FLUE_DATA

// Enable Flow Record Data Endianness Conversion
// by the Classification Engine hardware master interface
//#define EIP97_GLOBAL_BYTE_SWAP_FLOW_DATA

// Enable Context Data Endianness Conversion
// by the Processing Engine hardware master interface
//#define EIP97_GLOBAL_BYTE_SWAP_CONTEXT_DATA

// Enable ARC4 Context Data Endianness Conversion
// by the PE hardware master interface
//#define EIP97_GLOBAL_BYTE_SWAP_ARC4_CONTEXT_DATA

// One or several of the following methods must be configured:
// Swap bytes within each 32 bit word
//#define EIP97_GLOBAL_BYTE_SWAP_METHOD_32
// Swap 32 bit chunks within each 64 bit chunk
//#define EIP97_GLOBAL_BYTE_SWAP_METHOD_64
// Swap 64 bit chunks within each 128 bit chunk
//#define EIP97_GLOBAL_BYTE_SWAP_METHOD_128
// Swap 128 bit chunks within each 256 bit chunk
//#define EIP97_GLOBAL_BYTE_SWAP_METHOD_256

#if defined(EIP97_GLOBAL_BYTE_SWAP_METHOD_32)
#define EIP97_GLOBAL_BYTE_SWAP_METHOD   1
#elif defined(EIP97_GLOBAL_BYTE_SWAP_METHOD_64)
#define EIP97_GLOBAL_BYTE_SWAP_METHOD   2
#elif defined(EIP97_GLOBAL_BYTE_SWAP_METHOD_128)
#define EIP97_GLOBAL_BYTE_SWAP_METHOD   4
#elif defined(EIP97_GLOBAL_BYTE_SWAP_METHOD_256)
#define EIP97_GLOBAL_BYTE_SWAP_METHOD   8
#else
#define EIP97_GLOBAL_BYTE_SWAP_METHOD   0
#endif

// if nonzero, packet update information (checksum, length, protocol) will
// not be appended to packet data. Indicate in result token flags
// which fields have to be updated.
// If zero, such update information can be appended to the packet, if the
// packet cannot be updated in-place by the hardware.
#ifndef EIP97_GLOBAL_EIP96_BLOCK_UPDATE_APPEND
#define EIP97_GLOBAL_EIP96_BLOCK_UPDATE_APPEND          0
#endif

// If nonzero, add an IP length delta field to the result token.
#ifndef EIP97_GLOBAL_EIP96_LEN_DELTA_ENABLE
#define EIP97_GLOBAL_EIP96_LEN_DELTA_ENABLE             0
#endif

// EIP-96 Packet Engine Enable extended errors
// When set to 1 E14 indicates the presence of extended errors. If an extended
//               error is present the error code is in E7..E0.
// When set to 0 errors will always be reported one-hot in E14..E0
#ifndef EIP97_GLOBAL_EIP96_EXTENDED_ERRORS_ENABLE
#define EIP97_GLOBAL_EIP96_EXTENDED_ERRORS_ENABLE   0
#endif

// EIP-96 Packet Engine Timeout Counter
// Enables the time-out counter that generates an error in case of a „hang‟
// situation. If this bit is not set, the time-out error can never occur.
#ifndef EIP97_GLOBAL_EIP96_TIMEOUT_CNTR_FLAG
#define EIP97_GLOBAL_EIP96_TIMEOUT_CNTR_FLAG     1
#endif

// EIP-96 Packet Engine hold output data
#ifndef EIP97_GLOBAL_EIP96_PE_HOLD_OUTPUT_DATA
#define EIP97_GLOBAL_EIP96_PE_HOLD_OUTPUT_DATA      0
#endif // EIP97_GLOBAL_EIP96_PE_HOLD_OUTPUT_DATA

// Outbound sequence number threshold for reporting imminent rollover (32-bit)
#ifndef EIP97_GLOBAL_EIP96_NUM32_THR
#define EIP97_GLOBAL_EIP96_NUM32_THR                0
#endif

// Outbound sequence number threshold for reporting imminent rollover (64-bit)
#ifndef EIP97_GLOBAL_EIP96_NUM64_THR_L
#define EIP97_GLOBAL_EIP96_NUM64_THR_L              0
#endif
#ifndef EIP97_GLOBAL_EIP96_NUM64_THR_H
#define EIP97_GLOBAL_EIP96_NUM64_THR_H              0
#endif


// Define this parameter to enable the EIP-97 HW version check
//#define EIP97_GLOBAL_VERSION_CHECK_ENABLE

// Define this parameter in order to configure the DFE and DSE ring priorities
// Note: Some EIP-97 HW version do not have the DFE and DSE ring priority
//       configuration registers.
//#define EIP97_GLOBAL_DFE_DSE_PRIO_CONFIGURE

// Set this parameter to a correct value in top-level configuration file
// EIP-207s Classification Support, DMA Control base address
//#define EIP97_RC_BASE      0x37000

// Set this parameter to a correct value in top-level configuration file
// EIP-207s Classification Support, DMA Control base address
//#define EIP97_BASE         0x3FFF4

// Read cache type control (EIP197_MST_CTRL rd_cache field)
#ifndef EIP97_GLOBAL_RD_CACHE_VALUE
#define EIP97_GLOBAL_RD_CACHE_VALUE                 0
#endif

// Write cache type control (EIP197_MST_CTRL wr_cache field)
// Note: the buffering is enabled here by default
#ifndef EIP97_GLOBAL_WR_CACHE_VALUE
#define EIP97_GLOBAL_WR_CACHE_VALUE                 1
#endif

// Data read cache type control (data_cache_ctrl field in HIA_DFE_n_CFG reg)
#ifndef EIP97_GLOBAL_DFE_DATA_CACHE_CTRL
#define EIP97_GLOBAL_DFE_DATA_CACHE_CTRL            0
#endif

// Control read cache type control (ctrl_cache_ctrl field in HIA_DFE_n_CFG reg)
#ifndef EIP97_GLOBAL_DFE_CTRL_CACHE_CTRL
#define EIP97_GLOBAL_DFE_CTRL_CACHE_CTRL            0
#endif

// Data read cache type control (data_cache_ctrl field in HIA_DSE_n_CFG reg)
#ifndef EIP97_GLOBAL_DSE_DATA_CACHE_CTRL
#define EIP97_GLOBAL_DSE_DATA_CACHE_CTRL            0
#endif

// Data write bufferability control (buffer_ctrl field in HIA_DSE_n_CFG reg)
// for the Look-Aside FIFO streaming interface only
// Note: when the LA FIFO is not used then set this to 3 for optimal performance
#ifndef EIP97_GLOBAL_DSE_BUFFER_CTRL
#define EIP97_GLOBAL_DSE_BUFFER_CTRL                2
#endif

// Enables DSE single write mode. Set to 1 for optimal performance.
// Note: This cannot be set to 1 when
//       - 32-bit DMA address EIP-202 ring descriptor format is used
//       - overlapping EIP-202 CDR/RDR rings are used
#ifndef EIP97_GLOBAL_DSE_ENABLE_SINGLE_WR_FLAG
#define EIP97_GLOBAL_DSE_ENABLE_SINGLE_WR_FLAG      0
#endif

// Number of the first ring used as a look-aside FIFO
#ifndef EIP97_GLOBAL_LAFIFO_RING_ID
#define EIP97_GLOBAL_LAFIFO_RING_ID                 1
#endif

// Number of Processing Engines to use
// Maximum number of processing that should be used
// Should not exceed the number of engines physically available
#ifndef EIP97_GLOBAL_MAX_NOF_PE_TO_USE
#define EIP97_GLOBAL_MAX_NOF_PE_TO_USE              1
#endif

// Number of the look-aside FIFO queues
// EIP-197 configuration:
//           Server b - 1, c - 3, d - 5, e - 1.
//           Mobile 0
#ifndef EIP97_GLOBAL_MAX_NOF_LAFIFO_TO_USE
#define EIP97_GLOBAL_MAX_NOF_LAFIFO_TO_USE          0
#endif

// Number of the Inline FIFO queues
// Server 1 Mobile 0
#ifndef EIP97_GLOBAL_MAX_NOF_INFIFO_TO_USE
#define EIP97_GLOBAL_MAX_NOF_INFIFO_TO_USE          0
#endif

// AXI bus protection value, refer to HW documentation
#ifndef EIP97_GLOBAL_SUPPORT_PROTECT_VALUE
#define EIP97_GLOBAL_SUPPORT_PROTECT_VALUE          0
#endif

// Look-aside FIFO descriptor data swap
#ifndef EIP202_LASIDE_DSCR_BYTE_SWAP_METHOD
#define EIP202_LASIDE_DSCR_BYTE_SWAP_METHOD      EIP97_GLOBAL_BYTE_SWAP_METHOD
#endif

// Look-aside FIFO input packet data swap
#ifndef EIP202_LASIDE_IN_PKT_BYTE_SWAP_METHOD
#define EIP202_LASIDE_IN_PKT_BYTE_SWAP_METHOD    EIP97_GLOBAL_BYTE_SWAP_METHOD
#endif

// Look-aside FIFO output packet data swap
#ifndef EIP202_LASIDE_OUT_PKT_BYTE_SWAP_METHOD
#define EIP202_LASIDE_OUT_PKT_BYTE_SWAP_METHOD   EIP97_GLOBAL_BYTE_SWAP_METHOD
#endif

// Look-aside FIFO token data swap
#ifndef EIP202_LASIDE_TOKEN_BYTE_SWAP_METHOD
#define EIP202_LASIDE_TOKEN_BYTE_SWAP_METHOD     EIP97_GLOBAL_BYTE_SWAP_METHOD
#endif

// Look-aside FIFO input packet data AXI PROT value
#ifndef EIP202_LASIDE_IN_PKT_PROTO
#define EIP202_LASIDE_IN_PKT_PROTO                  0
#endif

// Look-aside FIFO output packet data AXI PROT value
#ifndef EIP202_LASIDE_OUT_PKT_PROTO
#define EIP202_LASIDE_OUT_PKT_PROTO                 0
#endif

// Look-aside FIFO token data AXI PROT value
#ifndef EIP202_LASIDE_TOKEN_PROTO
#define EIP202_LASIDE_TOKEN_PROTO                   0
#endif

// Inline FIFO: burst size 3 = 8 beats = 128 bytes
#ifndef EIP202_INLINE_BURST_SIZE
#define EIP202_INLINE_BURST_SIZE                    3
#endif

// Inline FIFO: result packets in order - 1, out of order - 0
#ifndef EIP202_INLINE_FORCE_INORDER
#define EIP202_INLINE_FORCE_INORDER                 0  // out of order
#endif

// Inline FIFO input packet data swap
#ifndef EIP202_INLINE_IN_PKT_BYTE_SWAP_METHOD
#define EIP202_INLINE_IN_PKT_BYTE_SWAP_METHOD    EIP97_GLOBAL_BYTE_SWAP_METHOD
#endif

// Inline FIFO output packet data swap
#ifndef EIP202_INLINE_OUT_PKT_BYTE_SWAP_METHOD
#define EIP202_INLINE_OUT_PKT_BYTE_SWAP_METHOD   EIP97_GLOBAL_BYTE_SWAP_METHOD
#endif

//Define if the hardware ECN registers are included
//#define EIP97_GLOBAL_HAVE_ECN_FIXUP

// Do not wait for token in EIP96
#ifndef EIP97_GLOBAL_EIP96_NO_TOKEN_WAIT
#define EIP97_GLOBAL_EIP96_NO_TOKEN_WAIT false
#endif

#ifndef EIP97_GLOBAL_EIP96_ECN_CONTROL
#define EIP97_GLOBAL_EIP96_ECN_CONTROL 0x1f
#endif

#if EIP97_GLOBAL_EIP96_ECN_CONTROL & 0x1
#define EIP96_ECN_CLE0 20
#else
#define EIP96_ECN_CLE0 0
#endif
#if EIP97_GLOBAL_EIP96_ECN_CONTROL & 0x2
#define EIP96_ECN_CLE1 21
#else
#define EIP96_ECN_CLE1 0
#endif
#if EIP97_GLOBAL_EIP96_ECN_CONTROL & 0x4
#define EIP96_ECN_CLE2 22
#else
#define EIP96_ECN_CLE2 0
#endif
#if EIP97_GLOBAL_EIP96_ECN_CONTROL & 0x8
#define EIP96_ECN_CLE3 23
#else
#define EIP96_ECN_CLE3 0
#endif
#if EIP97_GLOBAL_EIP96_ECN_CONTROL & 0x10
#define EIP96_ECN_CLE4 24
#else
#define EIP96_ECN_CLE4 0
#endif

// DFE packet threshold
#ifndef EIP97_GLOBAL_IN_TBUF_PKT_THR
#define EIP97_GLOBAL_IN_TBUF_PKT_THR 0
#endif

// Send context done pulses to record cache.
#ifndef EIP97_GLOBAL_EIP96_CTX_DONE_PULSE
#define EIP97_GLOBAL_EIP96_CTX_DONE_PULSE false
#endif

#ifndef EIP97_GLOBAL_META_DATA_ENABLE
#define EIP97_GLOBAL_META_DATA_ENABLE 0
#endif

// Strict argument checking for the input parameters
// If required then define this parameter in the top-level configuration
//#define EIP97_GLOBAL_STRICT_ARGS

// Finite State Machine that can be used for verifying that the Driver Library
// API is called in a correct order
//#define EIP97_GLOBAL_DEBUG_FSM

#ifndef EIP97_EIP96_CTX_SIZE
#define EIP97_EIP96_CTX_SIZE 0x3e
#endif

#endif /* C_EIP97_GLOBAL_H_ */


/* end of file c_eip97_global.h */
