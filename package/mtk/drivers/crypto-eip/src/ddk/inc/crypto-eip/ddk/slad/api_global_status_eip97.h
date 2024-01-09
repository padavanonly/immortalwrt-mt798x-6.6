/* api_global_status_eip97.h
 *
 * Security-IP-97 Global Control Get Status API
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

#ifndef API_GLOBAL_STATUS_EIP97_H_
#define API_GLOBAL_STATUS_EIP97_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint8_t, uint32_t, bool

// EIP-97 Driver Library API
#include "eip97_global_event.h" // Event Control
#include "eip97_global_prng.h"  // PRNG


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// GlobalControl97 Error Codes
typedef enum
{
    GLOBAL_CONTROL_NO_ERROR = 0,
    GLOBAL_CONTROL_ERROR_BAD_PARAMETER,
    GLOBAL_CONTROL_ERROR_BAD_USE_ORDER,
    GLOBAL_CONTROL_ERROR_INTERNAL,
    GLOBAL_CONTROL_ERROR_NOT_IMPLEMENTED
} GlobalControl97_Error_t;

// EIP-97 Data Fetch Engine (DFE) thread status,
// 1 DFE thread corresponds to 1 Processing Engine (PE)
typedef EIP97_Global_DFE_Status_t GlobalControl97_DFE_Status_t;

// EIP-97 Data Store Engine (DSE) thread status,
// 1 DSE thread corresponds to 1 Processing Engine (PE)
typedef EIP97_Global_DSE_Status_t GlobalControl97_DSE_Status_t;

// EIP-96 Token Status
typedef EIP96_Token_Status_t GlobalControl97_Token_Status_t;

// EIP-96 Context Status
typedef EIP96_Context_Status_t GlobalControl97_Context_Status_t;

// EIP-96 Interrupt Status
typedef EIP96_Interrupt_Status_t GlobalControl97_Interrupt_Status_t;

// EIP-96 Output Transfer Status
typedef EIP96_Output_Transfer_Status_t GlobalControl97_Output_Transfer_Status_t;

// EIP-96 PRNG Status
typedef EIP96_PRNG_Status_t GlobalControl97_PRNG_Status_t;

// EIP-96 PRNG Re-seed data
typedef EIP96_PRNG_Reseed_t             GlobalControl97_PRNG_Reseed_t;

// EIP-197 Debug statistics
typedef EIP97_Global_Debug_Statistics_t GlobalControl97_Debug_Statistics_t;


/*----------------------------------------------------------------------------
 * GlobalControl97_DFE_Status_Get
 *
 * This function retrieves the global status information
 * from the EIP-97 HIA DFE hardware.
 *
 * This function can detect the EIP-97 Fatal Error condition requiring
 * the EIP-97 Global SW or HW Reset!
 *
 * PE_Number (input)
 *     Number of the EIP-97 Processing Engine for which the status information
 *     must be retrieved
 *
 * DFE_Status_p (output)
 *     Pointer to the data structure where the DFE status will be stored
 *
 * This function is re-entrant.
 *
 * Return value
 *     GLOBAL_CONTROL_NO_ERROR : status retrieved successfully
 *     GLOBAL_CONTROL_ERROR_BAD_PARAMETER : invalid parameter
 */
GlobalControl97_Error_t
GlobalControl97_DFE_Status_Get(
        const unsigned int PE_Number,
        GlobalControl97_DFE_Status_t * const DFE_Status_p);


/*----------------------------------------------------------------------------
 * GlobalControl97_DSE_Status_Get
 *
 * This function can detect the EIP-97 Fatal Error condition requiring
 * the EIP-97 Global SW or HW Reset!
 *
 * This function can detect the EIP-97 Fatal Error condition requiring
 * the EIP-97 Global SW or HW Reset!
 *
 * PE_Number (input)
 *     Number of the EIP-97 Processing Engine for which the status information
 *     must be retrieved
 *
 * DSE_Status_p (output)
 *     Pointer to the data structure where the DSE status will be stored
 *
 * This function is re-entrant.
 *
 * Return value
 *     GLOBAL_CONTROL_NO_ERROR : status retrieved successfully
 *     GLOBAL_CONTROL_ERROR_BAD_PARAMETER : invalid parameter
 */
GlobalControl97_Error_t
GlobalControl97_DSE_Status_Get(
        const unsigned int PE_Number,
        GlobalControl97_DSE_Status_t * const DSE_Status_p);


/*----------------------------------------------------------------------------
 * GlobalControl97_Token_Status_Get
 *
 * This function retrieves the EIP-96 Token status information
 * from the EIP-97 hardware (includes HIA DFE, HIA DSE and EIP-96 PE)
 *
 * PE_Number (input)
 *     Number of the EIP-97 Processing Engine for which the status information
 *     must be retrieved
 *
 * Token_Status_p (output)
 *     Pointer to the data structure where the EIP-96 Token status
 *     will be stored
 *
 * This function is re-entrant.
 *
 * Return value
 *     GLOBAL_CONTROL_NO_ERROR : status retrieved successfully
 *     GLOBAL_CONTROL_ERROR_BAD_PARAMETER : invalid parameter
 */
GlobalControl97_Error_t
GlobalControl97_Token_Status_Get(
        const unsigned int PE_Number,
        GlobalControl97_Token_Status_t * const Token_Status_p);


/*----------------------------------------------------------------------------
 * GlobalControl97_Context_Status_Get
 *
 * This function retrieves the EIP-96 Context status information
 * from the EIP-97 hardware (includes HIA DFE, HIA DSE and EIP-96 PE)
 *
 * This function can detect the EIP-97 Fatal Error condition requiring
 * the EIP-97 Global SW or HW Reset!
 *
 * PE_Number (input)
 *     Number of the EIP-97 Processing Engine for which the status information
 *     must be retrieved
 *
 * Context_Status_p (output)
 *     Pointer to the data structure where the EIP-96 Context status
 *     will be stored
 *
 * This function is re-entrant.
 *
 * Return value
 *     GLOBAL_CONTROL_NO_ERROR : status retrieved successfully
 *     GLOBAL_CONTROL_ERROR_BAD_PARAMETER : invalid parameter
 */
GlobalControl97_Error_t
GlobalControl97_Context_Status_Get(
        const unsigned int PE_Number,
        GlobalControl97_Context_Status_t * const Context_Status_p);


/*----------------------------------------------------------------------------
 * GlobalControl97_Interrupt_Status_Get
 *
 * This function retrieves the EIP-96 Interrupt status information
 * from the EIP-97 hardware (includes HIA DFE, HIA DSE and EIP-96 PE)
 *
 * This function can detect the EIP-97 Fatal Error condition requiring
 * the EIP-97 Global SW or HW Reset!
 *
 * PE_Number (input)
 *     Number of the EIP-97 Processing Engine for which the status information
 *     must be retrieved
 *
 * Interrupt_Status_p (output)
 *     Pointer to the data structure where the EIP-96 Interrupt status
 *     will be stored
 *
 * This function is re-entrant.
 *
 * Return value
 *     GLOBAL_CONTROL_NO_ERROR : status retrieved successfully
 *     GLOBAL_CONTROL_ERROR_BAD_PARAMETER : invalid parameter
 */
GlobalControl97_Error_t
GlobalControl97_Interrupt_Status_Get(
        const unsigned int PE_Number,
        GlobalControl97_Interrupt_Status_t * const Interrupt_Status_p);


/*----------------------------------------------------------------------------
 * GlobalControl97_OutXfer_Status_Get
 *
 * This function retrieves the EIP-96 Output Transfer status information
 * from the EIP-97 hardware (includes HIA DFE, HIA DSE and EIP-96 PE)
 *
 * PE_Number (input)
 *     Number of the EIP-97 Processing Engine for which the status information
 *     must be retrieved
 *
 * OutXfer_Status_p (output)
 *     Pointer to the data structure where the EIP-96 Output Transfer status
 *     will be stored
 *
 * This function is re-entrant.
 *
 * Return value
 *     GLOBAL_CONTROL_NO_ERROR : status retrieved successfully
 *     GLOBAL_CONTROL_ERROR_BAD_PARAMETER : invalid parameter
 */
GlobalControl97_Error_t
GlobalControl97_OutXfer_Status_Get(
        const unsigned int PE_Number,
        GlobalControl97_Output_Transfer_Status_t * const OutXfer_Status_p);


/*----------------------------------------------------------------------------
 * GlobalControl97_PRNG_Status_Get
 *
 * This function retrieves the EIP-96 PRNG status information
 * from the EIP-97 hardware (includes HIA DFE, HIA DSE and EIP-96 PE)
 *
 * PE_Number (input)
 *     Number of the EIP-97 Processing Engine for which the status information
 *     must be retrieved
 *
 * PRNG_Status_p (output)
 *     Pointer to the data structure where the EIP-96 PRNG status
 *     will be stored
 *
 * This function is re-entrant.
 *
 * Return value
 *     GLOBAL_CONTROL_NO_ERROR : status retrieved successfully
 *     GLOBAL_CONTROL_ERROR_BAD_PARAMETER : invalid parameter
 */
GlobalControl97_Error_t
GlobalControl97_PRNG_Status_Get(
        const unsigned int PE_Number,
        GlobalControl97_PRNG_Status_t * const PRNG_Status_p);

/*----------------------------------------------------------------------------
 * GlobalControl97_PRNG_Reseed
 *
 * This function performs the Packet Engine re-seed operation
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * PE_Number (input)
 *     Number of the PE that must be re-seed.
 *
 * ReseedData_p (input)
 *     Pointer to the PRNG seed and key data.
 *
 * This function is not re-entrant.
 *
 * Return value
 *     GLOBAL_CONTROL_NO_ERROR : re-seed done successfully
 *     GLOBAL_CONTROL_ERROR_INTERNAL : operation failed
 *     GLOBAL_CONTROL_ERROR_BAD_PARAMETER : invalid parameter
 */
GlobalControl97_Error_t
GlobalControl97_PRNG_Reseed(
        const unsigned int PE_Number,
        const GlobalControl97_PRNG_Reseed_t * const ReseedData_p);


/*----------------------------------------------------------------------------
 * GlobalControl97_Debug_Statistics_Get
 *
 * This function retrieves the debug statistics
 * from the EIP-197
 *
 * Debug_Statistics_p (output)
 *     Pointer to the data structure where the debug statistics will be stored
 *
 * This function is re-entrant.
 *
 * Return value
 *     GLOBAL_CONTROL_NO_ERROR : status retrieved successfully
 *     GLOBAL_CONTROL_ERROR_BAD_PARAMETER : invalid parameter
 */
GlobalControl97_Error_t
GlobalControl97_Debug_Statistics_Get(
        GlobalControl97_Debug_Statistics_t * const Debug_Statistics_p);


/*----------------------------------------------------------------------------
 * GlobalControl97_Interfaces_Get
 *
 * Read the number of available packet interfaces.
 *
 * NofPEs_p (output)
 *    Number of processing engines.
 * NofRings_p (output)
 *    Number of available ring pairs.
 * NofLAInterfaces_p (output)
 *    Number of available Look-Aside FIFO interfaces.
 * NofInlineInterfaces_p (output)
 *    Number of available Inline interfaces.
 *
 * If all returned values are zero, the device has not been initialized.
 * This is considered an error.
 */
void
GlobalControl97_Interfaces_Get(
    unsigned int * const NofPEs_p,
    unsigned int * const NofRings_p,
    unsigned int * const NofLAInterfaces_p,
    unsigned int * const NofInlineInterfaces_p);


#endif /* API_GLOBAL_STATUS_EIP97_H_ */


/* end of file api_global_status_eip97.h */
