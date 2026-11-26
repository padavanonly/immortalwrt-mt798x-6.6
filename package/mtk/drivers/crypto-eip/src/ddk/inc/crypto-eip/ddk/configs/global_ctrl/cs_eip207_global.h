/* cs_eip207_global.h
 *
 * Top-level configuration parameters
 * for the EIP-207 Global Control
 *
 */

/*****************************************************************************
* Copyright (c) 2011-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef CS_EIP207_GLOBAL_H_
#define CS_EIP207_GLOBAL_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level configuration
#include "cs_adapter.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define EIP207_MAX_NOF_PE_TO_USE DRIVER_MAX_NOF_PE_TO_USE

// Maximum number of EIP-207c Classification Engines that can be used
#ifdef ADAPTER_CS_GLOBAL_MAX_NOF_CE_TO_USE
#define EIP207_GLOBAL_MAX_NOF_CE_TO_USE  ADAPTER_CS_GLOBAL_MAX_NOF_CE_TO_USE
#else
#define EIP207_GLOBAL_MAX_NOF_CE_TO_USE            1
#endif

// Maximum supported number of flow hash tables
#ifdef ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE
#define EIP207_MAX_NOF_FLOW_HASH_TABLES_TO_USE \
                           ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE
#else
#define EIP207_MAX_NOF_FLOW_HASH_TABLES_TO_USE     1
#endif

// Maximum supported number of FRC/TRC/ARC4 cache sets
#define EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE    1

// Enable firmware version check after the download
#define EIP207_GLOBAL_FIRMWARE_DOWNLOAD_VERSION_CHECK

// ICE Scratchpad RAM size in 128-byte blocks (512 bytes)
#define EIP207_ICE_SCRATCH_RAM_128B_BLOCK_COUNT         4

// OCE Scratchpad RAM size in 128-byte blocks (512 bytes)
#define EIP207_OCE_SCRATCH_RAM_128B_BLOCK_COUNT         4

// Disable clustered write operations, e.g. every write operation to
// an EIP-207 register will be followed by one read operation to
// a pre-defined EIP-207 register
#define EIP207_CLUSTERED_WRITES_DISABLE

// Strict argument checking for the input parameters
// If required then define this parameter in the top-level configuration
#define EIP207_GLOBAL_STRICT_ARGS

// Finite State Machine that can be used for verifying that the Driver Library
// API is called in a correct order
#define EIP207_GLOBAL_DEBUG_FSM


// Configuration parameter extensions
#include "cs_eip207_global_ext.h"

// Configuration parameter extensions
#include "cs_eip207_global_ext2.h"


#endif /* CS_EIP207_GLOBAL_H_ */


/* end of file cs_eip207_global.h */
