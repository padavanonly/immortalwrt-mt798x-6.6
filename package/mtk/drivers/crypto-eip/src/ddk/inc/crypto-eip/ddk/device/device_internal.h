/* device_internal.h
 *
 * Driver Framework Device Internal interface.
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

#ifndef DEVICE_INTERNAL_H_
#define DEVICE_INTERNAL_H_

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_device_generic.h"

// Driver Framework Device API
#include "device_types.h"           // Device_Handle_t

// Driver Framework Device Platform interface
#include "device_platform.h"        // Device_Platform_*


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Global administration data
typedef struct
{
    // Initialization flag
    bool fInitialized;

    Device_Platform_Global_t Platform; // platform-specific global data
} Device_Global_Admin_t;

// Internal statically configured device administration data
typedef struct
{
    const char * DevName;
    unsigned int DeviceNr;
    unsigned int FirstOfs;
    unsigned int LastOfs;
    unsigned int Flags; // swapping, tracing
} Device_Admin_Static_t;

// Internal device administration data
typedef struct
{
    char * DevName;
    unsigned int DeviceNr;
    unsigned int FirstOfs;
    unsigned int LastOfs;
    unsigned int Flags; // swapping, tracing

#ifdef HWPAL_DEVICE_MAGIC
    unsigned int Magic;
#endif

    Device_Platform_t Platform; // platform-specific device data

    unsigned int DeviceId; // index in the device list

} Device_Admin_t;

#define HWPAL_DEVICE_ADD(_name, _devnr, _firstofs, _lastofs, _flags) \
                        {_name, _devnr, _firstofs, _lastofs, _flags}


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * Device_Internal_Static_Count_Get
 *
 * Returns number of statically configured devices in the HWPAL_DEVICES device
 * list.
 *
 */
unsigned int
Device_Internal_Static_Count_Get(void);


/*----------------------------------------------------------------------------
 * Device_Internal_Count_Get
 *
 * Returns number of maximum supported devices in the device list.
 *
 */
unsigned int
Device_Internal_Count_Get(void);


/*----------------------------------------------------------------------------
 * Device_Internal_Admin_Static_Get
 *
 * Returns pointer to the memory location where the statically configured
 * device list is stored.
 *
 */
const Device_Admin_Static_t *
Device_Internal_Admin_Static_Get(void);


/*----------------------------------------------------------------------------
 * Device_Internal_Admin_Get
 *
 * Returns pointer to the memory location where the device list is stored.
 *
 */
Device_Admin_t **
Device_Internal_Admin_Get(void);


/*----------------------------------------------------------------------------
 * Device_Internal_Admin_Global_Get
 *
 * Returns pointer to the memory location where the global device
 * administration data is stored.
 *
 */
Device_Global_Admin_t *
Device_Internal_Admin_Global_Get(void);


/*----------------------------------------------------------------------------
 * Device_Internal_Alloc
 *
 * Returns a pointer to a newly allocated block ByteCount bytes long, or a null
 * pointer if the block could not be allocated.
 *
 */
void *
Device_Internal_Alloc(
        unsigned int ByteCount);


/*----------------------------------------------------------------------------
 * Device_Internal_Free
 *
 * Deallocates the block of memory pointed at by Ptr.
 *
 */
void
Device_Internal_Free(
        void * Ptr);


/*----------------------------------------------------------------------------
 * Device_Internal_Initialize
 *
 * See Device_Initialize() description.
 *
 */
int
Device_Internal_Initialize(
        void * CustomInitData_p);


/*----------------------------------------------------------------------------
 * Device_Internal_UnInitialize
 *
 * See Device_UnInitialize() description.
 *
 */
void
Device_Internal_UnInitialize(void);


/*-----------------------------------------------------------------------------
 * Device_Internal_Find
 *
 * See Device_Find() description.
 *
 * Index (input)
 *      Device  index in the device list.
 *
 */
Device_Handle_t
Device_Internal_Find(
        const char * DeviceName_p,
        const unsigned int Index);


/*-----------------------------------------------------------------------------
 * Device_Internal_GetIndex
 *
 * See Device_GetIndex() description.
 *
 */
int
Device_Internal_GetIndex(
        const Device_Handle_t Device);


#endif // DEVICE_INTERNAL_H_


/* end of file device_internal.h */

