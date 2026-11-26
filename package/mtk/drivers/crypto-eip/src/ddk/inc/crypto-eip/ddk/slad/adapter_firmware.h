/* adapter_firmware.h
 *
 * Interface for obtaining the firmware image.
 */

/*****************************************************************************
* Copyright (c) 2016-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_ADAPTER_FIRMWARE_H
#define INCLUDE_GUARD_ADAPTER_FIRMWARE_H


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Defs API
#include "basic_defs.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// This data type represents a firmware resource.
// use Adapter_Firmware_NULL to set to a known uninitialized value
typedef void * Adapter_Firmware_t;

/*----------------------------------------------------------------------------
 * Adapter_Firmware_NULL
 *
 * This handle can be assigned to a variable of type Adapter_Firmware_t.
 *
 */
extern const Adapter_Firmware_t Adapter_Firmware_NULL;


/*----------------------------------------------------------------------------
 * Adapter_Firmware_Acquire
 *
 * Obtain access to a specific firmware image in the form of an array of 32-bit
 * words. This function allocates any required buffers to store the
 * firmware. Multiple calls to this function are possible and multiple
 * firmware images remain valid at the same time. Access to the firmware
 * image remains valid until Adapter_Firmware_Release is called.
 *
 * Firmware_Name_p (input)
 *       Null terminated string that indicates which firmware to load.
 *       This is typically a file name under a implementation-defined
 *       fixed directory, but not all implementations are required to
 *       load firmware from a file system.
 *
 * Firmware_p (output)
 *       Pointer to array of 32-bit words that represents the loaded firmware.
 *
 * Firmware_Word32Count (output)
 *       Size of the array in 32-bit words.
 *
 * Return: Adapter_Firmware_NULL if firmware failed to load.
 *         any other value on success, can be passed to
 *                          Adapter_Firmware_Release
 */
Adapter_Firmware_t
Adapter_Firmware_Acquire(
    const char * Firmware_Name_p,
    const uint32_t ** Firmware_p,
    unsigned int  * Firmware_Word32Count);

/*----------------------------------------------------------------------------
 * Adapter_Firmware_Release
 *
 * Release any resources that were allocated by a single call to
 * Adapter_Firmware_Acquire. It is illegal to call this function multiple
 * times for the same handle.
 *
 * FirmwareHandle (input)
 *         Handle as returned by Adapter_Firmware_Acquire().
 */
void
Adapter_Firmware_Release(
   Adapter_Firmware_t FirmwareHandle);



#endif /* Include Guard */

/* end of file adapter_firmware.h */
