/* cs_eip202_ring.h
 *
 * Top-level configuration parameters
 * for the EIP-202 Ring Control Driver Library
 */

/*****************************************************************************
* Copyright (c) 2011-2021 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef CS_EIP202_RING_H_
#define CS_EIP202_RING_H_

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "cs_driver.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Is device 64-bit?
#ifdef DRIVER_64BIT_DEVICE
#define EIP202_64BIT_DEVICE
#endif // DRIVER_64BIT_DEVICE

// Disable clustered write operations, e.g. every write operation to
// an EIP-202 RD register will be followed by one read operation to
// a pre-defined EIP-202 register
#define EIP202_CLUSTERED_WRITES_DISABLE

#ifdef DRIVER_PEC_CD_PROT_VALUE
#define EIP202_RING_CD_PROT_VALUE               DRIVER_PEC_CD_PROT_VALUE
#endif

#ifdef DRIVER_PEC_RD_PROT_VALUE
#define EIP202_RING_RD_PROT_VALUE               DRIVER_PEC_RD_PROT_VALUE
#endif

#ifdef DRIVER_PEC_DATA_PROT_VALUE
#define EIP202_RING_DATA_PROT_VALUE             DRIVER_PEC_DATA_PROT_VALUE
#endif

#ifdef DRIVER_PEC_ACD_PROT_VALUE
#define EIP202_RING_ACD_PROT_VALUE              DRIVER_PEC_ACD_PROT_VALUE
#endif

#ifdef EIP197_BUS_VERSION_AXI3
// For AXI v3
#define EIP202_RING_CD_RD_CACHE_CTRL            2
#define EIP202_RING_CD_WR_CACHE_CTRL            2
#define EIP202_RING_RD_RD_CACHE_CTRL            2
#define EIP202_RING_RD_WR_CACHE_CTRL            2
#endif

#ifdef EIP197_BUS_VERSION_AXI4
// For AXI v4
#define EIP202_RING_CD_RD_CACHE_CTRL            5
#define EIP202_RING_CD_WR_CACHE_CTRL            3
#define EIP202_RING_RD_RD_CACHE_CTRL            5
#define EIP202_RING_RD_WR_CACHE_CTRL            3
#endif

// Strict argument checking for the input parameters
// If required then define this parameter in the top-level configuration
#ifndef DRIVER_PERFORMANCE
#define EIP202_RING_STRICT_ARGS
#endif

// Finite State Machine that can be used for verifying that the Driver Library
// API is called in a correct order
#ifndef DRIVER_PERFORMANCE
#define EIP202_RING_DEBUG_FSM
#endif

// Performance optimization: minimize CD and RD words writes and reads
#ifdef DRIVER_PERFORMANCE
#define EIP202_CDR_OPT1
#endif

#if defined(DRIVER_PE_ARM_SEPARATE)
#ifndef DRIVER_SCATTERGATHER
#if defined(DRIVER_64BIT_DEVICE) && !defined(DRIVER_PERFORMANCE)
// Enables padding the result descriptor to its full programmed offset,
// 0 - disabled, 1 - enabled
// Note: when enabled EIP97_GLOBAL_DSE_ENABLE_SINGLE_WR_FLAG must be defined too
#define EIP202_RDR_PAD_TO_OFFSET                1
#endif

// Enable RDR ownership word mechanism
#define EIP202_RDR_OWNERSHIP_WORD_ENABLE

#endif // !DRIVER_SCATTERGATHER
#else // DRIVER_PE_ARM_SEPARATE
#ifndef DRIVER_SCATTERGATHER
// Application ID field in the result token cannot be used when scatter/gather
// is enabled because this ID field is only written in the last descriptor
// in the chain!
#define EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS
#endif // !DRIVER_SCATTERGATHER
#endif // !DRIVER_PE_ARM_SEPARATE


// Bufferability control for result token DMA writes:
// 0b = do not buffer, 1b = allow buffering
#define EIP202_RING_RD_RES_BUF                  0

// Bufferability control for descriptor control word DMA writes:
// 0b = do not buffer, 1b = allow buffering.
#define EIP202_RING_RD_CTRL_BUF                 0

#include "cs_eip202_ring_ext.h"

#endif /* CS_EIP202_RING_H_ */


/* end of file cs_eip202_ring.h */
