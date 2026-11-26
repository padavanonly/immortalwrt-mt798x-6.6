/* api_global_eip97.h
 *
 * Security-IP-97 Global Control API
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

#ifndef API_GLOBAL_EIP97_H_
#define API_GLOBAL_EIP97_H_

// The status part of the API
#include "api_global_status_eip97.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint8_t, uint32_t, bool

// EIP-97 Driver Library Global API
#include "eip97_global_init.h"  // Initialization
#include "eip97_global_prng.h"  // PRNG


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define GLOBAL_CONTROL_MAXLEN_TEXT      128

// Ring PE assignment map
typedef EIP97_Global_Ring_PE_Map_t      GlobalControl97_Ring_PE_Map_t;

// Zero-terminated descriptive text of the available services.
typedef struct
{
    char szTextDescription[GLOBAL_CONTROL_MAXLEN_TEXT];
} GlobalControl97_Capabilities_t;


/*----------------------------------------------------------------------------
 * GlobalControl97_Capabilities_Get
 *
 * This routine returns a structure that describes the capabilities of the
 * implementation. See description of GlobalControl97_Capabilities_t
 * for details.
 *
 * Capabilities_p
 *     Pointer to the capabilities structure to fill in.
 *
 * This function is re-entrant.
 */
GlobalControl97_Error_t
GlobalControl97_Capabilities_Get(
        GlobalControl97_Capabilities_t * const Capabilities_p);


/*----------------------------------------------------------------------------
 * GlobalControl97_Init
 *
 * This function performs the initialization of the EIP-97 Global Control
 * functionality.
 *
 * fHWResetDone (input)
 *     Flag indicates whether the HW Reset operation was performed
 *     for the Security-IP-97 hardware before calling this function
 *
 * This function is not re-entrant.
 *
 * Return value
 *     GLOBAL_CONTROL_NO_ERROR : initialization performed successfully
 *     GLOBAL_CONTROL_ERROR_INTERNAL : initialization failed
 *     GLOBAL_CONTROL_ERROR_BAD_USE_ORDER : initialization is already done
 */
GlobalControl97_Error_t
GlobalControl97_Init(
        const bool fHWResetDone);


/*----------------------------------------------------------------------------
 * GlobalControl97_UnInit
 *
 * This function performs the initialization of the EIP-97 Global Control
 * functionality.
 *
 * This function is not re-entrant.
 *
 * Return value
 *     GLOBAL_CONTROL_NO_ERROR : un-initialization performed successfully
 *     GLOBAL_CONTROL_ERROR_INTERNAL : un-initialization failed
 *     GLOBAL_CONTROL_ERROR_BAD_USE_ORDER : un-initialization is already done
 */
GlobalControl97_Error_t
GlobalControl97_UnInit(void);


/*----------------------------------------------------------------------------
 * GlobalControl97_Configure
 *
 * This function performs the Ring to PE assignment and configures
 * the Ring priority. The EIP-97 device supports multiple Ring interfaces
 * as well as multiple PE's. One ring can be assigned to the same or different
 * PE's. Multiple rings can be assigned to the same PE.
 *
 * PE_Number (input)
 *     Number of the EIP-97 Processing Engine for which the status information
 *     must be retrieved
 *
 * RingPEMap_p (input)
 *     Pointer to the data structure that contains the Ring PE assignment map.
 *
 * This function is not re-entrant.
 *
 * Return value
 *     GLOBAL_CONTROL_NO_ERROR : PE configured successfully
 *     GLOBAL_CONTROL_ERROR_INTERNAL : operation failed
 *     GLOBAL_CONTROL_ERROR_BAD_PARAMETER : invalid parameter
 */
GlobalControl97_Error_t
GlobalControl97_Configure(
        const unsigned int PE_Number,
        const GlobalControl97_Ring_PE_Map_t * const RingPEMap_p);


#endif /* API_GLOBAL_EIP97_H_ */


/* end of file api_global_eip97.h */
