/* api_global_eip207.h
 *
 * Classification (EIP-207) Global Control Initialization API
 *
 * This API is not re-entrant.
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

#ifndef API_GLOBAL_EIP207_H_
#define API_GLOBAL_EIP207_H_


// The status part of the API
#include "api_global_status_eip207.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint32_t, bool


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define EIP207_GLOBAL_CONTROL_MAXLEN_TEXT           128

// IV to use for the Flow Hash ID calculations by the Classification hardware
typedef struct
{
    uint32_t IV[4]; // must be written with a true random value
} GlobalControl207_IV_t;

// Zero-terminated descriptive text of the available services.
typedef struct
{
    char szTextDescription[EIP207_GLOBAL_CONTROL_MAXLEN_TEXT];
} GlobalControl207_Capabilities_t;


/*----------------------------------------------------------------------------
 * GlobalControl207_Capabilities_Get
 *
 * This functions retrieves info about the capabilities of the
 * implementation.
 *
 * Capabilities_p
 *     Pointer to the capabilities structure to fill in.
 *
 * Return value
 *     None
 */
void
GlobalControl207_Capabilities_Get(
        GlobalControl207_Capabilities_t * const Capabilities_p);


/*----------------------------------------------------------------------------
 * GlobalControl207_Init
 *
 * This function performs the initialization of the EIP-207 Classification
 * Engine.
 *
 * fLoadFirmware (input)
 *     Flag indicates whether the EIP-207 Classification Engine firmware
 *     must be loaded
 *
 * IV_p (input)
 *     Pointer to the Initialization Vector data.
 *
 * Return value
 *     EIP207_GLOBAL_CONTROL_NO_ERROR : initialization performed successfully
 *     EIP207_GLOBAL_CONTROL_ERROR_INTERNAL : initialization failed
 *     EIP207_GLOBAL_CONTROL_ERROR_BAD_USE_ORDER : initialization is already
 *                                                 done
 */
GlobalControl207_Error_t
GlobalControl207_Init(
        const bool fLoadFirmware,
        const GlobalControl207_IV_t * const IV_p);


/*----------------------------------------------------------------------------
 * GlobalControl207_UnInit
 *
 * This function performs the un-initialization of the EIP-207 Classification
 * Engine.
 *
 * Return value
 *     EIP207_GLOBAL_CONTROL_NO_ERROR : un-initialization performed successfully
 *     EIP207_GLOBAL_CONTROL_ERROR_INTERNAL : un-initialization failed
 *     EIP207_GLOBAL_CONTROL_ERROR_BAD_USE_ORDER : un-initialization is already
 *                                                 done
 */
GlobalControl207_Error_t
GlobalControl207_UnInit(void);


#endif /* API_GLOBAL_EIP207_H_ */


/* end of file api_global_eip207.h */

