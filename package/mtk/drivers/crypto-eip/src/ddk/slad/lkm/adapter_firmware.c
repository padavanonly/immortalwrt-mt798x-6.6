/* adapter_firmware.c
 *
 * Kernel-space implementation of Interface for obtaining the firmware image.
 * Read from file system using Linux firmware API.
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


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "adapter_firmware.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_adapter_firmware.h"

// Driver Framework Basic Defs API
#include "basic_defs.h"

#include <linux/firmware.h>

#include "adapter_alloc.h"

// Logging API
#include "log.h"

#include "device_mgmt.h"  // Device_GetReference()

/*----------------------------------------------------------------------------
 * Adapter_Firmware_NULL
 *
 */
const Adapter_Firmware_t Adapter_Firmware_NULL = NULL;


/*----------------------------------------------------------------------------
 * Adapter_Firmware_Acquire
 */
Adapter_Firmware_t
Adapter_Firmware_Acquire(
    const char * Firmware_Name_p,
    const uint32_t ** Firmware_p,
    unsigned int  * Firmware_Word32Count)
{
    uint32_t * Firmware_Data_p;
    const struct firmware  *firmware;
    int rc;
    unsigned int i;

    LOG_CRIT("Adapter_Firmware_Acquire for %s\n",Firmware_Name_p);

    // Initialize output parameters.
    *Firmware_p = NULL;
    *Firmware_Word32Count = 0;

    // Request firmware via kernel API.
    rc = request_firmware(&firmware,
                          Firmware_Name_p,
                          Device_GetReference(NULL, NULL));
    if (rc < 0)
    {
        LOG_CRIT("Adapter_Firmware_Acquire request of %s failed, rc=%d\n",
                 Firmware_Name_p, rc);
        return NULL;
    }

    if (firmware->data == NULL ||
        firmware->size == 0 ||
        firmware->size >= 256*1024 ||
        firmware->size % sizeof(uint32_t) != 0)
    {
        LOG_CRIT("Adapter_Firmware_Acquire: Invalid image size %d\n",
                 (int)firmware->size);
        release_firmware(firmware);
        return NULL;
    }

    // Allocate buffer for data
    Firmware_Data_p = Adapter_Alloc(firmware->size);
    if (Firmware_Data_p == NULL)
    {
        LOG_CRIT("Adapter_Firmware_Acquire: Failed to allocate\n");
        release_firmware(firmware);
        return NULL;
    }

    // Convert bytes from file to array of 32-bit words.
    {
        const uint8_t *p = firmware->data;
        for (i=0; i<firmware->size / sizeof(uint32_t); i++)
        {
            Firmware_Data_p[i] = p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24);
            p += sizeof(uint32_t);
        }
    }

    // Pass results to caller
    *Firmware_p = Firmware_Data_p;
    *Firmware_Word32Count = firmware->size / sizeof(uint32_t);

    // Release firmware data structure.
    release_firmware(firmware);

    return (Adapter_Firmware_t)Firmware_Data_p;
}

/*----------------------------------------------------------------------------
 * Adapter_Firmware_Release
 */
void
Adapter_Firmware_Release(
    Adapter_Firmware_t FirmwareHandle)
{
    LOG_CRIT("Adapter_Firmware_Release\n");
    Adapter_Free((void*)FirmwareHandle);
}



/* end of file adapter_firmware.c */
