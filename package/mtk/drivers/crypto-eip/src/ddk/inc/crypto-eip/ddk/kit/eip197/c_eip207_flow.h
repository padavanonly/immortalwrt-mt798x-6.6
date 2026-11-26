/* c_eip207_flow.h
 *
 * Default configuration parameters
 * for the EIP-207 Flow Control Driver Library
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

#ifndef C_EIP207_FLOW_H_
#define C_EIP207_FLOW_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level configuration
#include "cs_eip207_flow.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Maximum number of EIP-207c Classification Engines that can be used
// Should not exceed the number of engines physically available
#ifndef EIP207_FLOW_MAX_NOF_CE_TO_USE
#define EIP207_FLOW_MAX_NOF_CE_TO_USE                   1
#endif

// Maximum supported number of flow hash tables
#ifndef EIP207_FLOW_MAX_NOF_FLOW_HASH_TABLES_TO_USE
#define EIP207_FLOW_MAX_NOF_FLOW_HASH_TABLES_TO_USE     1
#endif

// Define this parameter for additional consistency checks in the flow
// control functionality
//#define EIP207_FLOW_CONSISTENCY_CHECK

// Strict argument checking for the input parameters
// If required then define this parameter in the top-level configuration
//#define EIP207_FLOW_STRICT_ARGS

// Disable clustered write operations, e.g. every write operation to
// an EIP-207 register will be followed by one read operation to
// a pre-defined EIP-207 register
//#define EIP207_FLOW_CLUSTERED_WRITES_DISABLE

// EIP-207s Classification Support,
// Flow Look-Up Engine (FLUE) Record Cache type:
// If defined then use the  Hihg-Performance (HP) Record Cache,
// otherwise use the standard Record Cache
//#define EIP207_FLUE_RC_HP

// EIP-207s Classification Support,
// Flow Look-Up Engine (FLUE) register bank size
#ifndef EIP207_FLUE_FHT_REG_MAP_SIZE
#define EIP207_FLUE_FHT_REG_MAP_SIZE                     8192
#endif

// EIP-207s Classification Support,
// Flow Look-Up Engine (FLUE) CACHEBASE registers
#ifndef EIP207_FLUE_FHT1_REG_BASE
#define EIP207_FLUE_FHT1_REG_BASE                        0x00000
#endif // EIP207_FLUE_FHT1_REG_BASE

// EIP-207s Classification Support,
// Flow Look-Up Engine (FLUE) HASHBASE registers
#ifndef EIP207_FLUE_FHT2_REG_BASE
#define EIP207_FLUE_FHT2_REG_BASE                        0x00000
#endif // EIP207_FLUE_FHT2_REG_BASE

// EIP-207s Classification Support,
// Flow Look-Up Engine (FLUE) SIZE register
#ifndef EIP207_FLUE_FHT3_REG_BASE
#define EIP207_FLUE_FHT3_REG_BASE                        0x00000
#endif // EIP207_FLUE_FHT3_REG_BASE

// EIP-207s Classification Support, options and version registers
#ifndef EIP207_FLUE_OPTVER_REG_BASE
#define EIP207_FLUE_OPTVER_REG_BASE                      0x00000
#endif // EIP207_FLUE_OPTVER_REG_BASE

// EIP-207s Classification Support, Flow Hash Engine (FHASH) registers
#ifndef EIP207_FLUE_HASH_REG_BASE
#define EIP207_FLUE_HASH_REG_BASE                        0x00000
#endif // EIP207_FLUE_HASH_REG_BASE

// Finite State Machine that can be used for verifying that the Driver Library
// API is called in a correct order
//#define EIP207_FLOW_DEBUG_FSM

// Enable 64-bit DMA address support in the device
//#define EIP207_64BIT_DEVICE

// Define to disable the large transform record support
//#define EIP207_FLOW_REMOVE_TR_LARGE_SUPPORT

// Define to disable type bits in pointers inside flow record.
//#define EIP207_FLOW_NO_TYPE_BITS_IN_FLOW_RECORD


// Flow or transform record (FLUE_STD), Hash bucket (FLUE_DTL)
// remove wait loop count
#ifndef EIP207_FLOW_RECORD_REMOVE_WAIT_COUNT
#define EIP207_FLOW_RECORD_REMOVE_WAIT_COUNT            1000000
#endif


#endif /* C_EIP207_FLOW_H_ */


/* end of file c_eip207_flow.h */
