/* api_global_status_eip207.h
 *
 * Classification (EIP-207) Global Control Status API
 *
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

#ifndef API_GLOBAL_STATUS_EIP207_H_
#define API_GLOBAL_STATUS_EIP207_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint32_t, bool

// EIP-207 Driver Library Global Classification Control API
#include "eip207_global_init.h"

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// GlobalControl207 Error Codes
typedef enum
{
    EIP207_GLOBAL_CONTROL_NO_ERROR = 0,
    EIP207_GLOBAL_CONTROL_ERROR_BAD_PARAMETER,
    EIP207_GLOBAL_CONTROL_ERROR_BAD_USE_ORDER,
    EIP207_GLOBAL_CONTROL_ERROR_INTERNAL,
    EIP207_GLOBAL_CONTROL_ERROR_NOT_IMPLEMENTED
} GlobalControl207_Error_t;

// EIP-207s FRC/TRC/ARC4RC status information
typedef struct
{
    // True when a Host bus master read access from this cache
    // resulted in an error.
    bool fDMAReadError;

    // True when a Host bus master write access from this cache
    // resulted in an error.
    bool fDMAWriteError;

} GlobalControl207_CacheStatus_t;

// Classification Engine Status information
typedef EIP207_Global_Status_t GlobalControl207_Status_t;

// The Classification hardware global statistics
typedef EIP207_Global_GlobalStats_t GlobalControl207_GlobalStats_t;

// The Classification Engine clock count
typedef EIP207_Global_Clock_t GlobalControl207_Clock_t;

// The Classification Engine firmware configuration.
typedef EIP207_Firmware_Config_t GlobalControl_Firmware_Config_t;


/*----------------------------------------------------------------------------
 * GlobalControl207_Status_Get
 *
 * This function retrieves the global status information
 * from the EIP-207 Classification Engine hardware.
 *
 * This function can detect the EIP-207 Fatal Error condition requiring
 * the Global SW or HW Reset!
 *
 * CE_Number (input)
 *     Number of the EIP-207 Classification Engine for which the status
 *     information must be retrieved
 *
 * Status_p (output)
 *     Pointer to the data structure where the engine status will be stored
 *
 * This function is re-entrant.
 *
 * Return value
 *     EIP207_GLOBAL_CONTROL_NO_ERROR : status retrieved successfully
 *     EIP207_GLOBAL_CONTROL_ERROR_BAD_PARAMETER : invalid parameter
 */
GlobalControl207_Error_t
GlobalControl207_Status_Get(
        const unsigned int CE_Number,
        GlobalControl207_Status_t * const Status_p);


/*----------------------------------------------------------------------------
 * GlobalControl207_GlobalStats_Get
 *
 * This function obtains global statistics for the Classification Engine.
 *
 * CE_Number (input)
 *     Number of the EIP-207 Classification Engine for which the global
 *     statistics must be retrieved
 *
 * GlobalStats_p (output)
 *     Pointer to the data structure where the global statistics will be stored.
 *
 * Return value
 *     EIP207_GLOBAL_CONTROL_NO_ERROR : statistics retrieved successfully
 *     EIP207_GLOBAL_CONTROL_ERROR_BAD_PARAMETER : invalid parameter
 */
GlobalControl207_Error_t
GlobalControl207_GlobalStats_Get(
        const unsigned int CE_Number,
        GlobalControl207_GlobalStats_t * const GlobalStats_p);


/*--------------------- -------------------------------------------------------
 * GlobalControl207_ClockCount_Get
 *
 * Retrieve the current clock count as used by the Classification Engine.
 *
 * CE_Number (input)
 *     Number of the EIP-207 Classification Engine for which the clock
 *     count must be retrieved
 *
 * Clock_p (output)
 *     Pointer to the data structure where the current clock count used by
 *     the Classification Engine will be stored.
 *
 * Return value
 *     EIP207_GLOBAL_CONTROL_NO_ERROR : status retrieved successfully
 *     EIP207_GLOBAL_CONTROL_ERROR_BAD_PARAMETER : invalid parameter
 */
GlobalControl207_Error_t
GlobalControl207_ClockCount_Get(
        const unsigned int CE_Number,
        GlobalControl207_Clock_t * const Clock_p);


/*--------------------- -------------------------------------------------------
 * GlobalControl207_Firmware_Configure
 *
 * This function configures firmware settings.
 *
 * FWConfig_p (input)
 *     Configuration parameters for the firmware.
 *
 * Return value
 *     EIP207_GLOBAL_CONTROL_NO_ERROR : status retrieved successfully
 *     EIP207_GLOBAL_CONTROL_ERROR_BAD_PARAMETER : invalid parameter
 */
GlobalControl207_Error_t
GlobalControl207_Firmware_Configure(
        GlobalControl_Firmware_Config_t * const FWConfig_p);


#endif /* API_GLOBAL_STATUS_EIP207_H_ */


/* end of file api_global_status_eip207.h */
