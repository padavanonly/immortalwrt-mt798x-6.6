/* cs_eip207_flow.h
 *
 * Default configuration parameters
 * for the EIP-207 Flow Control Driver Library
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

#ifndef CS_EIP207_FLOW_H_
#define CS_EIP207_FLOW_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level configuration
#include "cs_driver.h"
#include "cs_adapter.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Is device 64-bit?
#ifdef DRIVER_64BIT_DEVICE
#define EIP207_64BIT_DEVICE
#endif  // DRIVER_64BIT_DEVICE

// Maximum number of EIP-207c Classification Engines that can be used
// Should not exceed the number of engines physically available
#ifdef ADAPTER_CS_GLOBAL_MAX_NOF_CE_TO_USE
#define EIP207_FLOW_MAX_NOF_CE_TO_USE  ADAPTER_CS_GLOBAL_MAX_NOF_CE_TO_USE
#else
#define EIP207_FLOW_MAX_NOF_CE_TO_USE            1
#endif

// Maximum supported number of flow hash tables
#ifdef ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE
#define EIP207_FLOW_MAX_NOF_FLOW_HASH_TABLES_TO_USE \
                           ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE
#else
#define EIP207_FLOW_MAX_NOF_FLOW_HASH_TABLES_TO_USE     1
#endif

// enable debug checks
#ifndef DRIVER_PERFORMANCE
// Define this parameter for additional consistency checks in the flow
// control functionality
#define EIP207_FLOW_CONSISTENCY_CHECK

// Strict argument checking for the input parameters
// If required then define this parameter in the top-level configuration
#define EIP207_FLOW_STRICT_ARGS

// Finite State Machine that can be used for verifying that the Driver Library
// API is called in a correct order
#define EIP207_FLOW_DEBUG_FSM
#endif // DRIVER_PERFORMANCE


// Configuration parameter Extensions
#include "cs_eip207_flow_ext.h"


#endif /* CS_EIP207_FLOW_H_ */


/* end of file cs_eip207_flow.h */
