/* api_global_eip74.h
 *
 * Deterministic Random Bit Generator (EIP-74) Global Control Initialization
 * API. The EIP-74 is used to generate pseudo-random IVs for outbound
 * operations in CBC mode.
 *
 * This API is not re-entrant.
 */

/*****************************************************************************
* Copyright (c) 2017-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef API_GLOBAL_EIP74_H_
#define API_GLOBAL_EIP74_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // bool, uint8_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// GlobalControl74 Error Codes
typedef enum
{
    GLOBAL_CONTROL_EIP74_NO_ERROR = 0,
    GLOBAL_CONTROL_EIP74_ERROR_BAD_PARAMETER,
    GLOBAL_CONTROL_EIP74_ERROR_BAD_USE_ORDER,
    GLOBAL_CONTROL_EIP74_ERROR_INTERNAL,
    GLOBAL_CONTROL_EIP74_ERROR_NOT_IMPLEMENTED
} GlobalControl74_Error_t;


#define GLOBAL_CONTROL_EIP74_MAXLEN_TEXT           128

// Zero-terminated descriptive text of the available services.
typedef struct
{
    char szTextDescription[GLOBAL_CONTROL_EIP74_MAXLEN_TEXT];
} GlobalControl74_Capabilities_t;


// Configuration parameters for EIP-74.
typedef struct
{
    // Number of IVs generated for each Generate operation.
    // Value can be set to zero to request default value.
    unsigned int GenerateBlockSize;

    // Number of Generate operations after which it is an error to continue
    // without reseed
    // Value can be set to zero to request default value.
    unsigned int ReseedThr;

    // Number of Generate operations after which to notify that reseed is
    // required
    // Value can be set to zero to request default value.
    unsigned int ReseedThrEarly;

    // Detect stuck-out faults
    bool fStuckOut;
} GlobalControl74_Configuration_t;


// Status information of the EIP-74.
typedef struct
{
    // Number of generate operations since last initialize or reseed
    unsigned int GenerateBlockCount;

    // Stuck-out fault detected
    bool fStuckOut;

    // Not-initialized error detected
    bool fNotInitialized;

    // Reseed error  detected, GenerateBlockCount passed threshold
    bool fReseedError;

    // Reseed warning detected, GenerateBlockCount passed early  threshold
    bool fReseedWarning;

    // DRBG was instantiated successfully
    bool fInstantiated;

    // Number of IVs available
    unsigned int AvailableCount;
} GlobalControl74_Status_t;


/*----------------------------------------------------------------------------
 * GlobalControl74_NotifyFunction_t
 *
 * This type specifies the callback function prototype for the function
 * GlobalControl74_Notify_Request. The notification will occur only once.
 *
 * NOTE: The exact context in which the callback function is invoked and the
 *       allowed actions in that callback are implementation specific. The
 *       intention is that all API functions can be used, except
 *       GlobalControl74_UnInit.
 */
typedef void (* GlobalControl74_NotifyFunction_t)(void);


/*----------------------------------------------------------------------------
 * GlobalControl74_Capabilities_Get
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
GlobalControl74_Capabilities_Get(
        GlobalControl74_Capabilities_t * const Capabilities_p);


/*----------------------------------------------------------------------------
 * GlobalControl74_Init
 *
 * This function performs the initialization of the EIP-74 Deterministic
 * Random Bit Generator.
 *
 * Configuration_p (input)
 *     Configuration parameters of the DRBG.
 *
 * Entropy_p (input)
 *     Pointer to a string of exactly 48 bytes that serves as the entropy.
 *     to initialize the DRBG.
 *
 * Return value
 *     GLOBAL_CONTROL_EIP74_NO_ERROR : initialization performed successfully
 *     GLOBAL_CONTROL_EIP74_ERROR_BAD_PARAMETER : Invalid parameters supplied
 *     GLOBAL_CONTROL_EIP74_ERROR_INTERNAL : initialization failed
 *     GLOBAL_CONTROL_EIP74_ERROR_BAD_USE_ORDER : initialization is already
 *                                                done
 */
GlobalControl74_Error_t
GlobalControl74_Init(
        const GlobalControl74_Configuration_t * const Configuration_p,
        const uint8_t * const Entropy_p);


/*----------------------------------------------------------------------------
 * GlobalControl74_UnInit
 *
 * This function performs the un-initialization of the EIP-74 Deterministic
 * Random Bit Generator.
 *
 * Return value
 *     GLOBAL_CONTROL_EIP74_NO_ERROR : un-initialization performed successfully
 *     GLOBAL_CONTROL_EIP74_ERROR_INTERNAL : un-initialization failed
 *     GLOBAL_CONTROL_EIP74_ERROR_BAD_USE_ORDER : un-initialization is already
 *                                                done
 */
GlobalControl74_Error_t
GlobalControl74_UnInit(void);


/*----------------------------------------------------------------------------
 * GlobalControl74_Reseed
 *
 * This function performs a reseed of the EIP-74 Deterministic
 * Random Bit Generator.
 *
 * Entropy_p (input)
 *     Pointer to a string of exactly 48 bytes that serves as the entropy.
 *     to reseed the DRBG.
 *
 * Return value
 *     GLOBAL_CONTROL_EIP74_NO_ERROR : operation performed successfully
 *     GLOBAL_CONTROL_EIP74_ERROR_BAD_PARAMETER : Invalid parameter supplied
 *     GLOBAL_CONTROL_EIP74_ERROR_INTERNAL : operation failed
 *     GLOBAL_CONTROL_EIP74_ERROR_BAD_USE_ORDER : device not initialized
 */
GlobalControl74_Error_t
GlobalControl74_Reseed(
        const uint8_t * const Entropy_p);


/*----------------------------------------------------------------------------
 * GlobalControl74_Status_Get
 *
 * This function reads the status of the EIP-74 Deterministic Random
 * Bit Generator.
 *
 * Status_p (output)
 *     Status information obtained from the EIP-74.
 *
 * Return value
 *     GLOBAL_CONTROL_EIP74_NO_ERROR : operation performed successfully
 *     GLOBAL_CONTROL_EIP74_ERROR_BAD_PARAMETER : Invalid parameter supplied
 *     GLOBAL_CONTROL_EIP74_ERROR_INTERNAL : operation failed
 *     GLOBAL_CONTROL_EIP74_ERROR_BAD_USE_ORDER : device not initialized
 */
GlobalControl74_Error_t
GlobalControl74_Status_Get(
        GlobalControl74_Status_t * const Status_p);


/*----------------------------------------------------------------------------
 * GlobalControl74_Clear
 *
 * This function clears any stuck-out condition of the EIP-74 Deterministic
 * Random Bit Generator.
 *
 * Return value
 *     GLOBAL_CONTROL_EIP74_NO_ERROR : operation performed successfully
 *     GLOBAL_CONTROL_EIP74_ERROR_INTERNAL : operation failed
 *     GLOBAL_CONTROL_EIP74_ERROR_BAD_USE_ORDER : device not initialized
 */
GlobalControl74_Error_t
GlobalControl74_Clear(void);


/*----------------------------------------------------------------------------
 * GlobalControl74_Notify_Request
 *
 * This routine can be used to request a one-time notification of
 * EIP-74 related events. When any error, fault or warning condition
 * occurs, the implementation will invoke the callback once to notify
 * the user. The callback function can then call
 * GlobalControl74_Status_Get to find out what event occurred and it
 * can take any actions to rectify the situation, to log the event or
 * to stop processing. The callback can call this function again to
 * request future notifications of future events.
 *
 * CBFunc_p (input)
 *     Address of the callback function.
 *
 * Return value
 *     GLOBAL_CONTROL_EIP74_NO_ERROR : operation performed successfully
 *     GLOBAL_CONTROL_EIP74_ERROR_BAD_PARAMETER : Invalid parameter supplied
 *     GLOBAL_CONTROL_EIP74_ERROR_INTERNAL : operation failed
 *     GLOBAL_CONTROL_EIP74_ERROR_BAD_USE_ORDER : device not initialized
 */
GlobalControl74_Error_t
GlobalControl74_Notify_Request(
        GlobalControl74_NotifyFunction_t CBFunc_p);


#endif /* API_GLOBAL_EIP74_H_ */


/* end of file api_global_eip74.h */
