/* c_eip202_ring.h
 *
 * Default configuration of the EIP-202 Ring Control Driver Library
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

#ifndef C_EIP202_RING_H_
#define C_EIP202_RING_H_

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// Top-level configuration, can override default configuration
// parameters specified in this file
#include "cs_eip202_ring.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Strict argument checking for the input parameters
// If required then define this parameter in the top-level configuration
//#define EIP202_RING_STRICT_ARGS

// Finite State Machine that can be used for verifying that the Driver Library
// API is called in a correct order
//#define EIP202_RING_DEBUG_FSM

// Set to 1 to enable RDR interrupt generation per packet
// instead of interrupt generation per result descriptor
#ifndef EIP202_RING_RD_INTERRUPTS_PER_PACKET_FLAG
#define EIP202_RING_RD_INTERRUPTS_PER_PACKET_FLAG  1
#endif

// Enables the 64-bit DMA addresses support in the device
//#define EIP202_64BIT_DEVICE

// Enable anti DMA race condition CDS mechanism.
// When enabled the Application ID field in the descriptors cannot be used
// by the Ring Control Driver Library users.
//#define EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS

// Disable clustered write operations, e.g. every write operation to
// an EIP-202 RD register will be followed by one read operation to
// a pre-defined EIP-202 register
//#define EIP202_CLUSTERED_WRITES_DISABLE

// CDR Read cache type control (rd_cache field in HIA_CDR_y_DMA_CFG register)
#ifndef EIP202_RING_CD_RD_CACHE_CTRL
#define EIP202_RING_CD_RD_CACHE_CTRL            0
#endif

// CDR Write cache type control (wr_cache field in HIA_CDR_y_DMA_CFG register)
#ifndef EIP202_RING_CD_WR_CACHE_CTRL
#define EIP202_RING_CD_WR_CACHE_CTRL            0
#endif

// RDR Read cache type control (rd_cache field in HIA_RDR_y_DMA_CFG register)
#ifndef EIP202_RING_RD_RD_CACHE_CTRL
#define EIP202_RING_RD_RD_CACHE_CTRL            0
#endif

// RDR Write cache type control (wr_cache field in HIA_RDR_y_DMA_CFG register)
#ifndef EIP202_RING_RD_WR_CACHE_CTRL
#define EIP202_RING_RD_WR_CACHE_CTRL            0
#endif

// AXI bus specific protection configuration: command descriptor data
#ifndef EIP202_RING_CD_PROT_VALUE
#define EIP202_RING_CD_PROT_VALUE               0
#endif

// AXI bus specific protection configuration: result descriptor data
#ifndef EIP202_RING_RD_PROT_VALUE
#define EIP202_RING_RD_PROT_VALUE               0
#endif

// AXI bus specific protection configuration: packet data
#ifndef EIP202_RING_DATA_PROT_VALUE
#define EIP202_RING_DATA_PROT_VALUE             0
#endif

// AXI bus specific protection configuration: token data
#ifndef EIP202_RING_ACD_PROT_VALUE
#define EIP202_RING_ACD_PROT_VALUE              0
#endif

// Enables padding the result descriptor to its full programmed
// offset with 0xAAAAAAAA, which is the ownership word magic
// number, to avoid having to do a separate ownership word write. This
// opposed to just writing the actual result descriptor data. This may be
// useful in certain systems where partial cache line writes cause read-
// modify-write operations and therefore filling the whole cacheline may be
// beneficial even if that means writing out more data than necessary.
//     0 - disabled
//     1 - enabled
// Note: when enabled EIP97_GLOBAL_DSE_ENABLE_SINGLE_WR_FLAG must be defined too
#ifndef EIP202_RDR_PAD_TO_OFFSET
#define EIP202_RDR_PAD_TO_OFFSET                0
#endif

// When defined the command descriptor write optimization will be applied,
// e.g. only required words will be written
//#define EIP202_CDR_OPT1 /* skip writing unused length word */
//#define EIP202_CDR_OPT2 /* skip writing token buffer address */
//#define EIP202_CDR_OPT3 /* skip writing SA buffer address */

// When defined the RDR ownership word support will be enabled
//#define EIP202_RDR_OWNERSHIP_WORD_ENABLE

#if defined(EIP202_RDR_OWNERSHIP_WORD_ENABLE) && \
    defined(EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS)
#error "Error: EIP202_RDR_OWNERSHIP_WORD_ENABLE excludes \
           EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS"
#endif

// Bufferability control for result token DMA writes. Driven on cache type
// bus bit [0] to Host Interface for all result token DMA writes to control
// write bufferability:
// 0b = do not buffer, 1b = allow buffering
#ifndef EIP202_RING_RD_RES_BUF
#define EIP202_RING_RD_RES_BUF                  0
#endif

// Bufferability control for descriptor control word DMA writes. Driven on
// cache type bus bit [0] to Host Interface for all descriptor control word
// DMA writes to control write bufferability:
// 0b = do not buffer, 1b = allow buffering.
#ifndef EIP202_RING_RD_CTRL_BUF
#define EIP202_RING_RD_CTRL_BUF                 0
#endif

// Bufferability control for ownership word DMA writes. Driven on cache
// type bus bit [0] to Host Interface for all ownership word DMA writes to
// control write bufferability:
// 0b = do not buffer, 1b = allow buffering.
#ifndef EIP202_RING_RD_OWN_BUF
#define EIP202_RING_RD_OWN_BUF                  1
#endif


#endif /* C_EIP202_RING_H_ */


/* end of file c_eip202_ring.h */
