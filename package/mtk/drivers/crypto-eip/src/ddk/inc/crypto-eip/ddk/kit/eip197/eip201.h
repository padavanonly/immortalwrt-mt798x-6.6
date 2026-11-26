/* eip201.h
 *
 * EIP201 Driver Library API
 *
 * Security-IP-201 is the Advanced Interrupt Controller (AIC)
 */

/*****************************************************************************
* Copyright (c) 2007-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_EIP201_H
#define INCLUDE_GUARD_EIP201_H

// Basic definitions
#include "basic_defs.h"          // uint32_t, bool, static inline, BIT_* etc.
// Device API definitions
#include "device_types.h"        // Device_Handle_t

// The API's table of contents:
//
//  EIP201_Status_t - error codes data type
//  EIP201_Source_t - interrupt source (irq) data type
//  EIP201_Config API - to control individual configurations
//  EIP201_SourceMask API - to control individual interrupt masks (masking)
//  EIP201_SourceStatus API - to retrieve individual interrupt statuses
//  EIP201_Initialize API - to initialize the EIP201 device by providing
//                          initial interrupt polarities and masks
//                          just in one call
//  EIP201_Acknowledge - to acknowledge individual interrupts


/*----------------------------------------------------------------------------
 * List of EIP-201 error codes that API functions can return.
 * EIP201_STATUS_UNSUPPORTED_IRQ can be returned if a concrete EIP device
 * does not support an interrupt source number provided in a bitmap
 * as a function argument.
 *
 * Any other integer value: Error return from device read or write function.
 */
enum
{
    EIP201_STATUS_SUCCESS = 0,
    EIP201_STATUS_UNSUPPORTED_IRQ,
    EIP201_STATUS_UNSUPPORTED_HARDWARE_VERSION,
};
typedef int EIP201_Status_t;


/*----------------------------------------------------------------------------
 * List of EIP-201 interrupt sources
 * Maximum number of sources is 32.
 */

// a single EIP201 interrupt source, typically BIT_xx
typedef uint32_t EIP201_Source_t;

// An OR-ed combination of EIP201 interrupt sources
typedef uint32_t EIP201_SourceBitmap_t;

// A handy macro for all interrupt sources mask
#define EIP201_SOURCE_ALL  (EIP201_SourceBitmap_t)(~0)

//Note1:
//  In API functions to follow the first parameter often is
//  Device_Handle_t Device.
//  This is a context object for the Driver Framework Device API
//  implementation.
//  This context must be unique for each instance of each driver
//  to allow selection of a specific EIP device (instance).
//  It is important that the Device_Handle_t instance describes a
//  valid EIP-201 device (HW block).

//Note2:
//  In API functions to follow the second parameter sometimes is
//  const EIP201_SourceBitmap_t Sources.
//  This is always a set of interrupt sources, for which some operation
//  has to be performed.
//  If an interrupt source is not included in the EIP201_SourceBitmap_t
//  instance, then the operation will not be performed for this source
//  (corresponding bit is not changed in a HW register).

//Note3:
//  If not stated otherwise, all API functions are re-entrant and can be
//  called concurrently with other API functions.

/*----------------------------------------------------------------------------
 * EIP201_Config API
 *
 * Controls configuration of individual interrupt sources:
 * - Falling Edge or Rising Edge (edge based)
 * - Active High or Active Low (level based)
 *
 * Usage:
 *     EIP201_Config_t Source0Config;
 *
 *     EIP201_Config_Change(
 *             Device,
 *             BIT_0,
 *             EIP201_CONFIG_ACTIVE_LOW);
 *
 *     EIP201_Config_Change(
 *             Device,
 *             BIT_1 | BIT_2,
 *             EIP201_CONFIG_RISING_EDGE);
 *
 *     Source0Config = EIP201_Config_Read(Device, BIT_0);
 */
typedef enum
{
    EIP201_CONFIG_ACTIVE_LOW = 0,
    EIP201_CONFIG_ACTIVE_HIGH,
    EIP201_CONFIG_FALLING_EDGE,
    EIP201_CONFIG_RISING_EDGE
} EIP201_Config_t;

/* EIP201_Config_Change function is not re-entrant and
   cannot be called concurrently with API functions:
        EIP201_Initialize
*/
EIP201_Status_t
EIP201_Config_Change(
        Device_Handle_t Device,
        const EIP201_SourceBitmap_t Sources,
        const EIP201_Config_t NewConfig);

// Source can only have one bit set
EIP201_Config_t
EIP201_Config_Read(
        Device_Handle_t Device,
        const EIP201_Source_t Source);


/*----------------------------------------------------------------------------
 * EIP201_SourceMask API
 *
 * Allows masking/unmasking individual interrupt sources.
 *
 * Usage:
 *     bool Source0IsEnabled;
 *     EIP201_SourceBitmap_t AllEnabledSources;
 *     EIP201_SourceMask_EnableSource(Device, BIT_1 + BIT_0);
 *     EIP201_SourceMask_DisableSource(Device, BIT_5 + BIT_2);
 *     Source0IsEnabled = EIP201_SourceMask_SourceIsEnabled(Device, BIT_0);
 *     AllEnabledSources = EIP201_SourceMask_ReadAll(Device);
 */

/* EIP201_SourceMask_EnableSource
*/
EIP201_Status_t
EIP201_SourceMask_EnableSource(
        Device_Handle_t Device,
        const EIP201_SourceBitmap_t Sources);

/* EIP201_SourceMask_DisableSource
*/
EIP201_Status_t
EIP201_SourceMask_DisableSource(
        Device_Handle_t Device,
        const EIP201_SourceBitmap_t Sources);

// Source can only have one bit set
bool
EIP201_SourceMask_SourceIsEnabled(
        Device_Handle_t Device,
        const EIP201_Source_t Source);

// Returns a bitmask for all enabled sources.
// In this bitmask:
//      0 - an interrupt source is disabled
//      1 - an interrupt source is enabled
EIP201_SourceBitmap_t
EIP201_SourceMask_ReadAll(
        Device_Handle_t Device);


/*----------------------------------------------------------------------------
 * EIP201_SourceStatus API
 *
 * Allows reading status of individual interrupt sources.
 * _IsEnabledSourcePending -
 *     reads the status of the source after a source mask is applied.
 * _IsRawSourcePending -
 *     reads the status of the raw source (no source mask is applied).
 *
 * Usage:
 *     bool Source0EnabledStatus;
 *     bool Source0RawStatus;
 *     EIP201_SourceBitmap_t AllEnabledStatuses;
 *     EIP201_SourceBitmap_t AllRawStatuses;
 *     Source0EnabledStatus =
 *         EIP201_SourceStatus_IsEnabledSourcePending(Device, BIT_0);
 *     Source0RawStatus =
 *         EIP201_SourceStatus_IsRawSourcePending(Device, BIT_0);
 *     AllEnabledStatuses = EIP201_SourceStatus_ReadAllEnabled(Device);
 *     AllRawStatuses = EIP201_SourceStatus_ReadAllRaw(Device);
 */
// Source can only have one bit set
bool
EIP201_SourceStatus_IsEnabledSourcePending(
        Device_Handle_t Device,
        const EIP201_Source_t Source);

// Source can only have one bit set
bool
EIP201_SourceStatus_IsRawSourcePending(
        Device_Handle_t Device,
        const EIP201_Source_t Source);

// Returns a bitmask for current statuses of all enabled sources.
// (after a source mask is applied)
// In this bitmask:
//      0 - an enabled interrupt source is not pending (inactive).
//      1 - an enabled interrupt source is pending (active).
EIP201_SourceBitmap_t
EIP201_SourceStatus_ReadAllEnabled(
        Device_Handle_t Device);

// Returns a bitmask for current statuses of all raw sources.
// (no source mask is applied)
// In this bitmask:
//      0 - a raw interrupt source is not pending (inactive).
//      1 - a raw interrupt source is pending (active).
EIP201_SourceBitmap_t
EIP201_SourceStatus_ReadAllRaw(
        Device_Handle_t Device);


// Returns a bitmask for current statuses of all enabled sources.
// (after a source mask is applied). Also return a status.
// In this bitmask:
//      0 - an enabled interrupt source is not pending (inactive).
//      1 - an enabled interrupt source is pending (active).
EIP201_Status_t
EIP201_SourceStatus_ReadAllEnabledCheck(
        Device_Handle_t Device,
        EIP201_SourceBitmap_t * const Statuses_p);

// Returns a bitmask for current statuses of all raw sources.
// (no source mask is applied). Also return a status.
// In this bitmask:
//      0 - a raw interrupt source is not pending (inactive).
//      1 - a raw interrupt source is pending (active).
EIP201_Status_t
EIP201_SourceStatus_ReadAllRawCheck(
        Device_Handle_t Device,
        EIP201_SourceBitmap_t * const Statuses_p);


/*----------------------------------------------------------------------------
 * EIP201_Initialize API
 *
 * Initializes the EIP201 interrupt controller device.
 *
 *     SettingsArray_p
 *         Initial interrupt settings for a number of interrupt sources.
 *         Can be NULL. If NULL, all settings are device default.
 *
 *     SettingsCount
 *         Number of interrupt sources, for which settings are given.
 *         Can be 0. If 0, all settings are device default.
 *
 * Usage:
 *     EIP201_SourceSettings_t MySettings[] =
 *     {
 *         {BIT_1, EIP201_CONFIG_ACTIVE_LOW,  false},
 *         {BIT_2, EIP201_CONFIG_ACTIVE_HIGH, true}
 *     };
 *
 *     EIP201_Initialize(Device, MySettings, 2);
 */
typedef struct
{
    EIP201_Source_t Source;  // for which interrupt source the settings are
    EIP201_Config_t Config;
    bool fEnable;
} EIP201_SourceSettings_t;

/* EIP201_Initialize function is not re-entrant and
   cannot be called concurrently with API functions:
        EIP201_Config_Change
*/
EIP201_Status_t
EIP201_Initialize(
        Device_Handle_t Device,
        const EIP201_SourceSettings_t * SettingsArray_p,
        const unsigned int SettingsCount);


/*----------------------------------------------------------------------------
 * EIP201_Acknowledge
 *
 * Acknowledges the EIP201 interrupts.
 *
 * Usage:
 *     EIP201_Acknowledge(Device, BIT_0 | BIT_1);
 */
EIP201_Status_t
EIP201_Acknowledge(
        Device_Handle_t Device,
        const EIP201_SourceBitmap_t Sources);

#endif /* INCLUDE_GUARD_EIP201_H */

/* end of file eip201.h */
