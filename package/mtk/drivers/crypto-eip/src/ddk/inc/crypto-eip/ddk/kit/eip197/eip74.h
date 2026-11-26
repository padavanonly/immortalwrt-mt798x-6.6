/* eip74.h
 *
 * EIP-74 Driver Library API:
 *
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

#ifndef EIP74_H_
#define EIP74_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint8_t, uint32_t, bool

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t

/*----------------------------------------------------------------------------
 * Example API Use Order:
 *
 * 1 Call EIP74_Init to initialize the IOArea of the Driver Library.
 *
 * 2 Call EIP74_Configure to set the configuration parameters of the device:
 *   the generate block size and the reseed thresholds. This function can be
 *   called anywhere between step 1 and step 5. Typically these configuration
 *   parameters are set only once.
 *
 * 3 Call EIP74_Reset to reset the device. It it returns EIP74_NO_ERROR
 *   go to step 5, if it returns EIP74_BUSY_RETRY_LATER, go to step 4.
 *
 * 4 Call EIP74_Reset_Isdone until the result is EIP74_NO_ERROR.
 *
 * 5 Call EIP74_Instantiate. When this function returns, the device is
 *   operational.
 *
 * Steps 3 through 5 can be repeated to re-instantiate the DRBG, but typically
 * these steps are performed only once,
 *
 * 6 Periodically, or upon receiving a device interrupt, check the
 *   status of the device and take necessary actions.
 *
 * 6.1 call EIP74_Status_Get to find out what events occurred.
 *
 * 6.2 If a stuck-output fault occurred, call EIP74_Clear to clear the fault
 *     condition. Stuck-output faults are very rare but not impossible during
 *     normal operation.
 *
 * 6.3 If a reseed warning occurred, call EIP74_Reseed
 *
 * 7 Optionally call EIP74_Reset to stop operation of the device.
 */

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define EIP74_IOAREA_REQUIRED_SIZE       (2 * sizeof(void*))

/*----------------------------------------------------------------------------
 * EIP74_Error_t
 *
 * Status (error) code type returned by these API functions
 * See each function "Return value" for details.
 *
 * EIP74_NO_ERROR : successful completion of the call.
 * EIP74_UNSUPPORTED_FEATURE_ERROR : not supported by the device.
 * EIP74_ARGUMENT_ERROR :  invalid argument for a function parameter.
 * EIP74_BUSY_RETRY_LATER : Device is busy.
 * EIP74_ILLEGAL_IN_STATE : illegal state transition
 */
typedef enum
{
    EIP74_NO_ERROR = 0,
    EIP74_UNSUPPORTED_FEATURE_ERROR,
    EIP74_ARGUMENT_ERROR,
    EIP74_BUSY_RETRY_LATER,
    EIP74_ILLEGAL_IN_STATE
} EIP74_Error_t;

// Generic EIP HW version
typedef struct
{
    // The basic EIP number.
    uint8_t EipNumber;

    // The complement of the basic EIP number.
    uint8_t ComplmtEipNumber;

    // Hardware Patch Level.
    uint8_t HWPatchLevel;

    // Minor Hardware revision.
    uint8_t MinHWRevision;

    // Major Hardware revision.
    uint8_t MajHWRevision;
} EIP_Version_t;

// EIP-74 HW options
typedef struct
{
    // Number of client interfaces
    uint8_t ClientCount;

    // Number of AES cores
    uint8_t AESCoreCount;

    // Speed grade of AES core
    uint8_t AESSpeed;

    // Depth of output FIFO
    uint8_t FIFODepth;
} EIP74_Options_t;


// EIP-74 Hardware capabilities.
typedef struct
{
    EIP_Version_t HW_Revision;
    EIP74_Options_t HW_Options;
} EIP74_Capabilities_t;

// Configuration parameters for EIP-74.
typedef struct
{
    // Number of IVs generated for each Generate operation.
    unsigned int GenerateBlockSize;

    // Number of Generate operations after which it is an error to continue
    // without reseed
    unsigned int ReseedThr;

    // Number of Generate operations after which to notify that reseed is
    // required
    unsigned int ReseedThrEarly;
} EIP74_Configuration_t;


// Status information of the EIP-74.
typedef struct
{
    // Number of generate operations since last initialize or reseed
    unsigned int GenerateBlockCount;

    // Stuck-output fault detected
    bool fStuckOut;

    // Not-initialized error detected
    bool fNotInitialized;

    // Reseed error  detected, GenerateBlockCount passed threshold
    bool fReseedError;

    // Reseed warning detected, GenerateBlockCount passed early threshold
    bool fReseedWarning;

    // DRBG was instantiated successfully
    bool fInstantiated;

    // Number of IVs available
    unsigned int AvailableCount;
} EIP74_Status_t;


// place holder for device specific internal data
typedef struct
{
    uint32_t placeholder[EIP74_IOAREA_REQUIRED_SIZE];
} EIP74_IOArea_t;


/*----------------------------------------------------------------------------
 * EIP74_Init
 *
 * This function performs the initialization of the EIP-74 HW
 * interface and transits the API to the Initialized state.
 *
 * This function returns the EIP74_UNSUPPORTED_FEATURE_ERROR error code
 * when it detects a mismatch in the Driver Library configuration
 * and the EIP-74 HW revision or configuration.
 *
 * Note: This function should be called first.
 *
 * IOArea_p (output)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * Device (input)
 *     Handle for the Global Control device instance returned by Device_Find.
 *
 * Return value
 *     EIP74_NO_ERROR : operation is completed
 *     EIP74_UNSUPPORTED_FEATURE_ERROR : not supported by the device.
 *     EIP74_ARGUMENT_ERROR : Passed wrong argument
 *     EIP74_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP74_Error_t
EIP74_Init(
        EIP74_IOArea_t * const IOArea_p,
        const Device_Handle_t Device);


/*----------------------------------------------------------------------------
 * EIP74_Reset
 *
 * This function starts the EIP-74 Reset operation. If the reset operation
 * can be done immediately this function returns EIP74_NO_ERROR.
 * Otherwise it will return EIP74_BUSY_RETRY_LATER indicating
 * that the reset operation has been started and is ongoing.
 * The EIP74_Reset_IsDone() function can be used to poll the device
 * for the completion of the reset operation.
 *
 * Note: This function must be called and the operation must be complete
 *       before calling the EIP74_Instantiate() function
 * Note: This function can be used to terminate operation of the DRBG. This
 *       will also clear the internal state of the hardware.
 *
 * IOArea_p (output)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * This function can be called once the driver library is initialized.
 *
 * This function is NOT re-entrant. It can be called concurrently with
 * EIP74_HWRevision_Get, EIP74_Configure, EIP174_Status_Get and EIP74_Clear.
 *
 * Return value
 *     EIP74_NO_ERROR : Global SW Reset is done
 *     EIP74_UNSUPPORTED_FEATURE_ERROR : not supported by the device.
 *     EIP74_BUSY_RETRY_LATER: Global SW Reset is started but
 *                                    not completed yet
 *     EIP74_ARGUMENT_ERROR : Passed wrong argument
 *     EIP74_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP74_Error_t
EIP74_Reset(
        EIP74_IOArea_t * const IOArea_p);


/*----------------------------------------------------------------------------
 * EIP74_Reset_IsDone
 *
 * This function checks the status of the started by the EIP74_Reset()
 * function for the EIP-74 device.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * This function can be called after a call to EIP74_Reset and before any
 * subsequent call to EIP74_Instantiate
 *
 * This function is NOT re-entrant. It can be called concurrently with
 * EIP74_HWRevision_Get, EIP74_Configure, EIP174_Status_Get and EIP74_Clear.
 *
 * Return value
 *     EIP74_NO_ERROR : Global SW Reset operation is done
 *     EIP74_BUSY_RETRY_LATER: Global SW Reset is started but
 *                                    not completed yet
 *     EIP74_ARGUMENT_ERROR : Passed wrong argument
 *     EIP74_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP74_Error_t
EIP74_Reset_IsDone(
        EIP74_IOArea_t * const IOArea_p);


/*----------------------------------------------------------------------------
 * EIP74_HWRevision_Get
 *
 * This function returns EIP-74 hardware revision
 * information in the Capabilities_p data structure.
 *
 * Device (input)
 *     Handle of the device to be used.
 *
 * Capabilities_p (output)
 *     Pointer to the place holder in memory where the device capability
 *     information will be stored.
 *
 * This function can be called at all times, even when the device is
 * not initialized.
 *
 * This function is re-entrant and can be called
 * concurrently with other driver library functions..
 *
 * Return value
 *     EIP74_NO_ERROR : operation is completed
 *     EIP74_ARGUMENT_ERROR : Passed wrong argument
 */
EIP74_Error_t
EIP74_HWRevision_Get(
        const Device_Handle_t Device,
        EIP74_Capabilities_t * const Capabilities_p);


/*----------------------------------------------------------------------------
 * EIP74_Configure
 *
 * This function configures the block size and the reseed thresholds of the
 * EIP-74 hardware. If called, it should be called before the first
 * call to EIP74_Reset or between EIP74_Reset and EIP74_Instantiate.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * Configuration_p (input)
 *     Pointer to the data structure that contains the EIP74 configuration
 *
 * This function can be called once the driver library is initialized.
 *
 * This function is NOT re-entrant. It can be called concurrently
 * with other driver library functions.
 *
 * Return value
 *     EIP74_NO_ERROR : operation is completed
 *     EIP74_ARGUMENT_ERROR : Passed wrong argument
 *     EIP74_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP74_Error_t
EIP74_Configure(
        EIP74_IOArea_t * const IOArea_p,
        const EIP74_Configuration_t * const Configuration_p);


/*----------------------------------------------------------------------------
 * EIP74_Instantiate
 *
 * This function instantiates the AES-256-CTR SP800-90 DRBG on the EIP-74
 * hardware. It can be called after a reset operation that is completed.
 * After instantiation, the EIP-74 will generate pseudo-random data blocks
 * of the configured size autonomously.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * Entropy_p (input)
 *     Pointer to a string of exactly 12 32-bit worfs that serves as the
 *     entropy to instantiate the DRBG.
 *
 * fStuckOut (input)
 *     Enable stuck-output detection (next output is the same as previous)
 *     during IV generation.
 *
 * This function can be called after EIP74_Reset or EIP74_Reset_IsDone when
 * such function returns EIP74_NO_ERROR.
 *
 * This function is NOT re-entrant. It can be called concurrently with
 * EIP74_HWRevision_Get, EIP74_Configure, EIP174_Status_Get and EIP74_Clear.
 *
 * Return value
 *     EIP74_NO_ERROR : operation is completed
 *     EIP74_ARGUMENT_ERROR : Passed wrong argument
 *     EIP74_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP74_Error_t
EIP74_Instantiate(
        EIP74_IOArea_t * const IOArea_p,
        const uint32_t * const Entropy_p,
        bool fStuckOut);


/*----------------------------------------------------------------------------
 * EIP74_Reseed
 *
 * This function initiates a reseed operation of the DRBG. The reseed
 * will take effect when the next Generate operation is started.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * Entropy_p (input)
 *     Pointer to a string of exactly 12 32-bit worfs that serves as the
 *     entropy to reseed the DRBG.
 *
 * This function can be called after EIP7_Instantiate and before any
 * subsequent call to EIP74_Reset.
 *
 * This function is NOT re-entrant. It can be called concurrently with
 * EIP74_HWRevision_Get, EIP74_Configure, EIP174_Status_Get and EIP74_Clear.
 *
 * Return value
 *     EIP74_NO_ERROR : operation is completed
 *     EIP74_ARGUMENT_ERROR : Passed wrong argument
 *     EIP74_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP74_Error_t
EIP74_Reseed(
        EIP74_IOArea_t * const IOArea_p,
        const uint32_t * const Entropy_p);


/*----------------------------------------------------------------------------
 * EIP74_Status_Get
 *
 * This function reads the status of the EIP-74.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * Status_p (output)
 *     Status information obtained from the EIP-74.
 *
 * This function can be called once the driver library is initialized.
 *
 * This function is re-entrant and can be called
 * concurrently with other driver library functions.
 *
 * Return value
 *     EIP74_NO_ERROR : operation is completed
 *     EIP74_ARGUMENT_ERROR : Passed wrong argument
 *     EIP74_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP74_Error_t
EIP74_Status_Get(
        EIP74_IOArea_t * const IOArea_p,
        EIP74_Status_t * const Status_p);


/*----------------------------------------------------------------------------
 * EIP74_Clear
 *
 * This function clears any stuck-output fault condition that may have
 * been detected by the hardware.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * This function can be called once the driver library is initialized.
 *
 * This function is re-entrant and can be called
 * concurrently with other driver library functions..
 *
 * Return value
 *     EIP74_NO_ERROR : operation is completed
 *     EIP74_ARGUMENT_ERROR : Passed wrong argument
 *     EIP74_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP74_Error_t
EIP74_Clear(
        EIP74_IOArea_t * const IOArea_p);


#endif /* EIP74_H_ */


/* end of file eip74.h */
