/* device_generic.c
 *
 * This is the generic Driver Framework v4 Device API implementation.
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// Driver Framework Device API
#include "device_mgmt.h"            // API to implement


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Defs API
#include "basic_defs.h"             // IDENTIFIER_NOT_USED, NULL

// Driver Framework C Run-time Library API
#include "clib.h"                   // strlen, strcpy

// Driver Framework Device Internal interface
#include "device_internal.h"        // Device_Internal_*


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Default configuration
#include "c_device_generic.h"

// Logging API
#include "log.h"


/*----------------------------------------------------------------------------
 * Local variables
 */


/*-----------------------------------------------------------------------------
 * DeviceLib_Device_Exists
 *
 * Checks if a device with DeviceName_p is already present in the device list
 *
 */
static bool
DeviceLib_Device_Exists(
        const char * DeviceName_p)
{
    unsigned int i = 0;
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();

    if (DeviceName_p == NULL)
        return false;

    while(DevAdmin_pp[i])
    {
        if (strcmp(DevAdmin_pp[i]->DevName, DeviceName_p) == 0)
            return true;
        i++;
    }

    return false;
}


/*-----------------------------------------------------------------------------
 * device_mgmt API
 *
 * These functions support finding a device given its name.
 * A handle is returned that is needed in the device_rw API
 * to read or write the device
 */

/*-----------------------------------------------------------------------------
 * Device_Initialize
 */
int
Device_Initialize(
        void * CustomInitData_p)
{
    unsigned int res;
    unsigned int DevStatCount = Device_Internal_Static_Count_Get();
    unsigned int DevCount = Device_Internal_Count_Get();
    const Device_Admin_Static_t * DevStatAdmin_p =
                            Device_Internal_Admin_Static_Get();
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();
    Device_Global_Admin_t * DevGlobalAdmin_p =
                            Device_Internal_Admin_Global_Get();

    if (DevGlobalAdmin_p->fInitialized)
        return 0; // already initialized, success

    if (DevStatCount > DevCount)
    {
        LOG_CRIT("%s: Invalid number of static devices (%d), max %d\n",
                 __func__,
                 (int)DevStatCount,
                 DevCount);
        return -1; // failed
    }

    // Copy static devices
    for (res = 0; res < DevStatCount; res++)
    {
        if (DeviceLib_Device_Exists(DevStatAdmin_p[res].DevName))
        {
            LOG_CRIT("%s: failed, device (index %d) with name %s exists\n",
                     __func__,
                     res,
                     DevStatAdmin_p[res].DevName);
            goto error_exit;
        }

        // Allocate memory for device administration data
        DevAdmin_pp[res] = Device_Internal_Alloc(sizeof(Device_Admin_t));
        if (DevAdmin_pp[res] == NULL)
        {
            LOG_CRIT("%s: failed to allocate device (index %d, name %s)\n",
                     __func__,
                     res,
                     DevStatAdmin_p[res].DevName);
            goto error_exit;
        }

        // Allocate and copy device name
        DevAdmin_pp[res]->DevName =
                Device_Internal_Alloc((unsigned int)strlen(DevStatAdmin_p[res].DevName)+1);
        if (DevAdmin_pp[res]->DevName == NULL)
        {
            LOG_CRIT("%s: failed to allocate device (index %d) name %s\n",
                     __func__,
                     res,
                     DevStatAdmin_p[res].DevName);
            goto error_exit;
        }
        strcpy(DevAdmin_pp[res]->DevName, DevStatAdmin_p[res].DevName);

        // Copy the rest of device data
        DevAdmin_pp[res]->DeviceNr = DevStatAdmin_p[res].DeviceNr;
        DevAdmin_pp[res]->FirstOfs = DevStatAdmin_p[res].FirstOfs;
        DevAdmin_pp[res]->LastOfs  = DevStatAdmin_p[res].LastOfs;
        DevAdmin_pp[res]->Flags    = DevStatAdmin_p[res].Flags;

#ifdef HWPAL_DEVICE_MAGIC
        DevAdmin_pp[res]->Magic    = HWPAL_DEVICE_MAGIC;
#endif
        DevAdmin_pp[res]->DeviceId = res;
    }

    res = (unsigned int)Device_Internal_Initialize(CustomInitData_p);
    if (res != 0)
    {
        LOG_CRIT("%s: failed, error %d\n", __func__, res);
        goto error_exit;
    }

    DevGlobalAdmin_p->fInitialized = true;
    return 0; // success

error_exit:
    // Free all allocated memory
    for (res = 0; res < DevStatCount; res++)
        if (DevAdmin_pp[res])
        {
            Device_Internal_Free(DevAdmin_pp[res]->DevName);
            Device_Internal_Free(DevAdmin_pp[res]);
            DevAdmin_pp[res] = NULL;
        }

    return -1; // failed
}


/*-----------------------------------------------------------------------------
 * Device_UnInitialize
 */
void
Device_UnInitialize(void)
{
    unsigned int DevCount = Device_Internal_Count_Get();
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();
    Device_Global_Admin_t * DevGlobalAdmin_p =
                        Device_Internal_Admin_Global_Get();

    LOG_INFO("%s: unregister driver\n", __func__);

    if (DevGlobalAdmin_p->fInitialized)
    {
        unsigned int i;

        Device_Internal_UnInitialize();

        // Free all allocated memory
        for (i = 0; i < DevCount; i++)
            if (DevAdmin_pp[i])
            {
                Device_Internal_Free(DevAdmin_pp[i]->DevName);
                Device_Internal_Free(DevAdmin_pp[i]);
                DevAdmin_pp[i] = NULL;
            }

        DevGlobalAdmin_p->fInitialized = false;
    }
}


/*-----------------------------------------------------------------------------
 * Device_Find
 */
Device_Handle_t
Device_Find(
        const char * DeviceName_p)
{
    unsigned int i;

    unsigned int DevCount = Device_Internal_Count_Get();
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();
    Device_Global_Admin_t * DevGlobalAdmin_p =
                        Device_Internal_Admin_Global_Get();

    if (!DevGlobalAdmin_p->fInitialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return NULL;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (DeviceName_p == NULL)
    {
        LOG_CRIT("%s: failed, invalid name\n", __func__);
        return NULL; // not supported, thus not found
    }
#endif

    // walk through the device list and compare the device name
    for (i = 0; i < DevCount; i++)
        if (DevAdmin_pp[i] &&
            strcmp(DeviceName_p, DevAdmin_pp[i]->DevName) == 0)
            return Device_Internal_Find(DeviceName_p, i);

    LOG_CRIT("%s: failed to find device '%s'\n", __func__, DeviceName_p);

    return NULL;
}


/*----------------------------------------------------------------------------
 * Device_GetProperties
 */
int
Device_GetProperties(
        const unsigned int Index,
        Device_Properties_t * const Props_p,
        bool * const fValid_p)
{
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();
    Device_Global_Admin_t * DevGlobalAdmin_p =
                        Device_Internal_Admin_Global_Get();

    if (!DevGlobalAdmin_p->fInitialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return -1;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (Index >= Device_Internal_Count_Get())
    {
        LOG_CRIT("%s: failed, invalid index %d, max device count %d\n",
                 __func__,
                 Index,
                 Device_Internal_Count_Get());
        return -1;
    }

    if (Props_p == NULL || fValid_p == NULL)
    {
        LOG_CRIT("%s: failed, invalid pointers for device index %d\n",
                 __func__,
                 Index);
        return -1;
    }
#endif

    if (!DevAdmin_pp[Index])
    {
        LOG_INFO("%s: device with index %d not present\n",
                 __func__,
                 Index);
        *fValid_p = false;
    }
    else
    {
        Props_p->Name_p          = DevAdmin_pp[Index]->DevName;
        Props_p->StartByteOffset = DevAdmin_pp[Index]->FirstOfs;
        Props_p->LastByteOffset  = DevAdmin_pp[Index]->LastOfs;
        Props_p->Flags           = (char)DevAdmin_pp[Index]->Flags;
        *fValid_p = true;
    }
    return 0;
}


/*----------------------------------------------------------------------------
 * Device_Add
 */
int
Device_Add(
        const unsigned int Index,
        const Device_Properties_t * const Props_p)
{
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();
    Device_Global_Admin_t * DevGlobalAdmin_p =
                        Device_Internal_Admin_Global_Get();

    if (!DevGlobalAdmin_p->fInitialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return -1;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (Index >= Device_Internal_Count_Get())
    {
        LOG_CRIT("%s: failed, invalid index %d, max device count %d\n",
                 __func__,
                 Index,
                 Device_Internal_Count_Get());
        return -1;
    }

    if (Props_p == NULL || Props_p->Name_p == NULL)
    {
        LOG_CRIT("%s: failed, invalid properties for device index %d\n",
                 __func__,
                 Index);
        return -1;
    }
#endif

    if (DevAdmin_pp[Index])
    {
        LOG_CRIT("%s: device with index %d already added\n",
                 __func__,
                 Index);
        return -1;
    }

    if (DeviceLib_Device_Exists(Props_p->Name_p))
    {
        LOG_CRIT("%s: device with name %s already added\n",
                 __func__,
                 Props_p->Name_p);
        return -1;
    }

    // Allocate memory for device administration data
    DevAdmin_pp[Index] = Device_Internal_Alloc(sizeof(Device_Admin_t));
    if (DevAdmin_pp[Index] == NULL)
    {
        LOG_CRIT("%s: failed to allocate device (index %d, name %s)\n",
                 __func__,
                 Index,
                 Props_p->Name_p);
        return -1;
    }

    // Allocate and copy device name
    DevAdmin_pp[Index]->DevName =
                    Device_Internal_Alloc((unsigned int)strlen(Props_p->Name_p)+1);
    if (DevAdmin_pp[Index]->DevName == NULL)
    {
        LOG_CRIT("%s: failed to allocate device (index %d) name %s\n",
                 __func__,
                 Index,
                 Props_p->Name_p);
        Device_Internal_Free(DevAdmin_pp[Index]);
        DevAdmin_pp[Index] = NULL;
        return -1;
    }
    strcpy(DevAdmin_pp[Index]->DevName, Props_p->Name_p);

    // Copy the rest
    DevAdmin_pp[Index]->FirstOfs  = Props_p->StartByteOffset;
    DevAdmin_pp[Index]->LastOfs   = Props_p->LastByteOffset;
    DevAdmin_pp[Index]->Flags     = (unsigned int)Props_p->Flags;

    // Set default values
    DevAdmin_pp[Index]->DeviceNr  = 0;
#ifdef HWPAL_DEVICE_MAGIC
    DevAdmin_pp[Index]->Magic     = HWPAL_DEVICE_MAGIC;
#endif

    DevAdmin_pp[Index]->DeviceId  = Index;

    return 0; // success
}


/*----------------------------------------------------------------------------
 * Device_Remove
 */
int
Device_Remove(
        const unsigned int Index)
{
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();
    Device_Global_Admin_t * DevGlobalAdmin_p =
                        Device_Internal_Admin_Global_Get();

    if (!DevGlobalAdmin_p->fInitialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return -1;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (Index >= Device_Internal_Count_Get())
    {
        LOG_CRIT("%s: failed, invalid index %d, max device count %d\n",
                 __func__,
                 Index,
                 Device_Internal_Count_Get());
        return -1;
    }
#endif

    if (!DevAdmin_pp[Index])
    {
        LOG_CRIT("%s: device with index %d already removed\n",
                 __func__,
                 Index);
        return -1;
    }
    else
    {
        // Free device memory
        Device_Internal_Free(DevAdmin_pp[Index]->DevName);
        Device_Internal_Free(DevAdmin_pp[Index]);
        DevAdmin_pp[Index] = NULL;
    }

    return 0; // success
}


/*-----------------------------------------------------------------------------
 * Device_GetName
 */
char *
Device_GetName(
        const unsigned int Index)
{
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();
    Device_Global_Admin_t * DevGlobalAdmin_p =
                        Device_Internal_Admin_Global_Get();

    if (!DevGlobalAdmin_p->fInitialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return NULL;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (Index >= Device_Internal_Count_Get())
    {
        LOG_CRIT("%s: failed, invalid index %d, max device count %d\n",
                 __func__,
                 Index,
                 Device_Internal_Count_Get());
        return NULL;
    }
#endif

    if (!DevAdmin_pp[Index])
    {
        LOG_CRIT("%s: device with index %d already removed\n",
                 __func__,
                 Index);
        return NULL;
    }

    return DevAdmin_pp[Index]->DevName; // success
}


/*-----------------------------------------------------------------------------
 * Device_GetIndex
 */
int
Device_GetIndex(
        const Device_Handle_t Device)
{
    Device_Global_Admin_t * DevGlobalAdmin_p =
                        Device_Internal_Admin_Global_Get();

    if (!DevGlobalAdmin_p->fInitialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return -1;
    }

    return Device_Internal_GetIndex(Device);
}


/*-----------------------------------------------------------------------------
 * Device_GetCount
 */
unsigned int
Device_GetCount(void)
{
    return Device_Internal_Count_Get();
}


/* end of file device_generic.c */
