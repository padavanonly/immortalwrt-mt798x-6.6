/* device_platform.h
 *
 * Driver Framework platform-specific interface,
 * Linux user-space UMDevXS.
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

#ifndef DEVICE_PLATFORM_H_
#define DEVICE_PLATFORM_H_

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"             // uint32_t

// Linux kernel API
#include <linux/compiler.h>         // __iomem
#include <linux/pci.h>              // pci_*


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Internal global device administration data
typedef struct
{
    // Device data
    struct platform_device * Platform_Device_p;

    // Physical base address of the device resource (MMIO space)
    void * PhysBaseAddr;

    // Mapped (virtual) address of the device resource (MMIO space)
    uint32_t __iomem * MappedBaseAddr_p;

} Device_Platform_Global_t;

// Internal device administration data
typedef struct
{
    unsigned int Reserved; // not used

} Device_Platform_t;


/*----------------------------------------------------------------------------
 * Local variables
 */


#endif // DEVICE_PLATFORM_H_


/* end of file device_platform.h */

