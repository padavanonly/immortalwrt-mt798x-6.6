/* adapter_memxs.c
 *
 * Low-level Memory Access API implementation.
 */

/*****************************************************************************
* Copyright (c) 2012-2020 by Rambus, Inc. and/or its subsidiaries.
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// MemXS API
#include "api_memxs.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_adapter_memxs.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t
#include "device_rw.h"         // Device_Read/Write
#include "device_mgmt.h"

// Driver Framework C Run-Time Library API
#include "clib.h"               // memcmp


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/*----------------------------------------------------------------------------
 * MemXS_DeviceAdmin_t
 *
 * MemXS device record
 */
typedef struct
{
    const char *        DevName;

    unsigned int        FirstOfs;

    unsigned int        LastOfs;

    Device_Handle_t     MemXS_Device;

} MemXS_DeviceAdmin_t;


/*----------------------------------------------------------------------------
 * Local variables
 */

// Here is the dependency on the Driver Framework configuration
// via the MemXS configuration
#ifndef HWPAL_DEVICES
#error "Expected HWPAL_DEVICES defined by cs_memxs.h"
#endif

// Here is the dependency on the Driver Framework configuration
// via the MemXS configuration
#undef HWPAL_DEVICE_ADD
#define HWPAL_DEVICE_ADD(_name, _devnr, _firstofs, _lastofs, _flags) \
           { _name, _firstofs, _lastofs, NULL }

static MemXS_DeviceAdmin_t MemXS_Devices[] =
{
    HWPAL_DEVICES
};

static const unsigned int MemXS_Device_Count =
        (sizeof(MemXS_Devices) / sizeof(MemXS_DeviceAdmin_t));


/*----------------------------------------------------------------------------
 * MemXS_NULLHandle
 *
 */
const MemXS_Handle_t MemXS_NULLHandle = { 0 };


/*----------------------------------------------------------------------------
 * MemXS_Init
 */
bool
MemXS_Init (void)
{
    unsigned int i;

    for(i = 0; i < MemXS_Device_Count; i++)
    {
        MemXS_Devices[i].MemXS_Device = NULL;

        MemXS_Devices[i].MemXS_Device =
                Device_Find (MemXS_Devices[i].DevName);

        if (MemXS_Devices[i].MemXS_Device == NULL)
        {
            return false;
        }
    } // for

    return true;
}


/*-----------------------------------------------------------------------------
 * MemXS_Handle_IsSame
 */
bool
MemXS_Handle_IsSame(
        const MemXS_Handle_t * const Handle1_p,
        const MemXS_Handle_t * const Handle2_p)
{
    return Handle1_p->p == Handle2_p->p;
}


/*-----------------------------------------------------------------------------
 * MemXS_Device_Count_Get
 */
MemXS_Status_t
MemXS_Device_Count_Get(
        unsigned int * const DeviceCount_p)
{
    if (DeviceCount_p == NULL)
        return MEMXS_INVALID_PARAMETER;

    *DeviceCount_p = MemXS_Device_Count;

    return MEMXS_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * MemXS_Device_Info_Get
 */
MemXS_Status_t
MemXS_Device_Info_Get(
        const unsigned int DeviceIndex,
        MemXS_DevInfo_t * const DeviceInfo_p)
{
    if (DeviceInfo_p == NULL)
        return MEMXS_INVALID_PARAMETER;

    if (DeviceIndex > MemXS_Device_Count)
        return MEMXS_INVALID_PARAMETER;

    DeviceInfo_p->Index    = DeviceIndex;
    DeviceInfo_p->Handle.p = MemXS_Devices[DeviceIndex].MemXS_Device;
    DeviceInfo_p->Name_p   = MemXS_Devices[DeviceIndex].DevName;
    DeviceInfo_p->FirstOfs = MemXS_Devices[DeviceIndex].FirstOfs;
    DeviceInfo_p->LastOfs  = MemXS_Devices[DeviceIndex].LastOfs;

    return MEMXS_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * MemXS_Read32
 */
uint32_t
MemXS_Read32(
        const MemXS_Handle_t Handle,
        const unsigned int ByteOffset)
{
    Device_Handle_t Device = (Device_Handle_t)Handle.p;

    return Device_Read32(Device, ByteOffset);
}


/*----------------------------------------------------------------------------
 * MemXS_Write32
 */
void
MemXS_Write32(
        const MemXS_Handle_t Handle,
        const unsigned int ByteOffset,
        const uint32_t Value)
{
    Device_Handle_t Device = (Device_Handle_t)Handle.p;

    Device_Write32(Device, ByteOffset, Value);
}


/*----------------------------------------------------------------------------
 * MemXS_Read32Array
 */
void
MemXS_Read32Array(
        const MemXS_Handle_t Handle,
        const unsigned int ByteOffset,
        uint32_t * MemoryDst_p,
        const int Count)
{
    Device_Handle_t Device = (Device_Handle_t)Handle.p;

    Device_Read32Array(Device, ByteOffset, MemoryDst_p, Count);
}


/*----------------------------------------------------------------------------
 * MemXS_Write32Array
 */
void
MemXS_Write32Array(
        const MemXS_Handle_t Handle,
        const unsigned int ByteOffset,
        const uint32_t * MemorySrc_p,
        const int Count)
{
    Device_Handle_t Device = (Device_Handle_t)Handle.p;

    Device_Write32Array(Device, ByteOffset, MemorySrc_p, Count);
}


/* end of file adapter_memxs.c */
