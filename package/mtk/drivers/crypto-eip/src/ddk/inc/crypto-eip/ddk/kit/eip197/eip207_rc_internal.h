/* eip207_rc_internal.h
 *
 * EIP-207s Record Cache (RC) internal interface
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

#ifndef EIP207_RC_INTERNAL_H_
#define EIP207_RC_INTERNAL_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// EIP-207 Global Control Init API
#include "eip207_global_init.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // bool, uint32_t

// Driver Framework Device API
#include "device_types.h"               // Device_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Record cache combination type
typedef enum
{
    EIP207_RC_INTERNAL_NOT_COMBINED,        // Not combined
    EIP207_RC_INTERNAL_FRC_TRC_COMBINED,    // TRC combined with FRC
    EIP207_RC_INTERNAL_FRC_ARC4_COMBINED,   // ARC4RC combined with FRC
    EIP207_RC_INTERNAL_TRC_ARC4_COMBINED    // ARC4RC combined with TRC
} EIP207_RC_Internal_Combination_Type_t;


/*----------------------------------------------------------------------------
 * EIP207_RC_Internal_Init
 *
 * Initialize the EIP-207 Record Cache.
 *
 * Device (input)
 *      Device handle that identifies the Record Cache hardware
 *
 * CombinationType (input)
 *      Specifies the Record Cache combination type with another one,
 *      see EIP207_RC_Internal_Combination_Type_t
 *
 * CacheBase (input)
 *      Base address of the Record Cache, must be one of the following:
 *      EIP207_FRC_REG_BASE    - for the Flow Record Cache,
 *      EIP207_TRC_REG_BASE    - for the Transform Record Cache,
 *      EIP207_ARC4RC_REG_BASE - for the ARC4 Record Cache
 *
 * RC_Params_p (input, output)
 *      Pointer to the memory location containing the Record Cache
 *      initialization parameters.
 *
 * RecordWordCount (input)
 *      Record Cache Record size in 32-bit words.
 *
 * Return code
 *     EIP97_GLOBAL_NO_ERROR : operation is completed
 *     EIP97_GLOBAL_ARGUMENT_ERROR : Passed wrong argument
 */
EIP207_Global_Error_t
EIP207_RC_Internal_Init(
        const Device_Handle_t Device,
        const EIP207_RC_Internal_Combination_Type_t CombinationType,
        const uint32_t CacheBase,
        EIP207_Global_CacheParams_t * RC_Params_p,
        const unsigned int RecordWordCount);


/*----------------------------------------------------------------------------
 * EIP207_RC_Internal_Status_Get
 *
 * Get the EIP-207 Record Cache status.
 *
 * Device (input)
 *      Device handle that identifies the Record Cache hardware
 *
 * CacheSetId (input)
 *      Cache set identifier.
 *
 * FRC_Status_p (output)
 *      Pointer to the memory location to store the Flow Record Cache
 *      status information.
 *
 * TRC_Status_p (output)
 *      Pointer to the memory location to store the Transform Record Cache
 *      status information.
 *
 * ARC4RC_Status_p (output)
 *      Pointer to the memory location to store the ARC4 Record Cache
 *      status information.
 *
 * Return code
 *     None
 */
void
EIP207_RC_Internal_Status_Get(
        const Device_Handle_t Device,
        const unsigned int CacheSetId,
        EIP207_Global_CacheStatus_t * const FRC_Status_p,
        EIP207_Global_CacheStatus_t * const TRC_Status_p,
        EIP207_Global_CacheStatus_t * const ARC4RC_Status_p);


/*----------------------------------------------------------------------------
 * EIP207_RC_Internal_DebugStatistics_Get
 *
 * Get the EIP-207 Record Cache debug statistics.
 *
 * Device (input)
 *      Device handle that identifies the Record Cache hardware
 *
 * CacheSetId (input)
 *      Cache set identifier.
 *
 * FRC_Stats_p (output)
 *      Pointer to the memory location to store the Flow Record Cache
 *      status information.
 *
 * TRC_Stats_p (output)
 *      Pointer to the memory location to store the Transform Record Cache
 *      statistics information.
 *
 * Return code
 *     None
 */
void
EIP207_RC_Internal_DebugStatistics_Get(
        const Device_Handle_t Device,
        const unsigned int CacheSetId,
        EIP207_Global_CacheDebugStatistics_t * const FRC_Stats_p,
        EIP207_Global_CacheDebugStatistics_t * const TRC_Stats_p);

/* end of file eip207_rc_internal.h */


#endif /* EIP207_RC_INTERNAL_H_ */
