/* umdevxs_device.h
 *
 * Exported Device API of the UMDexXS/UMPCI driver.
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

#ifndef INCLUDE_GUARD_UMDEVXS_DEVICE_H
#define INCLUDE_GUARD_UMDEVXS_DEVICE_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#define UMDEVXS_KERNEL_APPID ((void*)-1)

/*----------------------------------------------------------------------------
 * UMDevXS_Device_LockRange
 *
 * Lock the specified device resource, so no other application will be
 * allowed to lock it through this function. Locking is bypassed if
 * the current application is the privileged application.
 *
 * DeviceNr (input)
 *   Selection of a specific (sub)-device.
 * offset (input)
 *   Byte offset of the range to lock. Must be a multiple of PAGE_SIZE.
 * size (input)
 *   Size in bytes of the range to lock. Must be a multiple of PAGE_SIZE.
 * AppID (input)
 *   ID of the application that requests the lock. Use UMDEVXS_KERNEL_APPID
 *   when calling this function from the kernel.
 *
 * Return 0 on success, -1 on failure.
 */
int
UMDevXS_Device_LockRange(
        unsigned int DeviceNr,
        unsigned int offset,
        unsigned int size,
        void * AppID);


/*----------------------------------------------------------------------------
 * UMDevXS_Device_SetPrivileged
 *
 * Mark the indicated application as the privileged application. Only one
 * application is allowed to claim this status.
 *
 * AppID (input)
 *   ID of the application that requests the lock. Use UMDEVXS_KERNEL_APPID
 *   when calling this function from the kernel.
 *
 * Return 0 on success, -1 on failure
 */
int
UMDevXS_Device_SetPrivileged(
        void *AppID);

/*----------------------------------------------------------------------------
 * UMDevXS_Device_Unlock
 *
 * Release all device ranges locked by the indicated application.  If
 * the indicated application was the privileged application, release
 * this status as well.
 *
 * AppID (input)
 *    ID of the application that releases the resources.
 *    Use UMDEVXS_KERNEL_APPID when calling this function from the kernel.
*/
void
UMDevXS_Device_Unlock(
        void *AppID);

#endif /* INCLUDE_GUARD_UMDEVXS_DEVICE_H */

/* umdevxs_device.h */
