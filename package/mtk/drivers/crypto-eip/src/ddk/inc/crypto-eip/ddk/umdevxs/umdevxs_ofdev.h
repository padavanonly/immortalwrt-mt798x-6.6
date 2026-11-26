/* umdevxs_ofdev.h
 *
 * Exported OpenFirmware (OF) Device API of the UMDexXS driver.
 */

/*****************************************************************************
* Copyright (c) 2014-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_UMDEVXS_OFDEV_H
#define INCLUDE_GUARD_UMDEVXS_OFDEV_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Linux Kernel API
#include <linux/of_platform.h>      // of_*,


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * UMDevXS_OFDev_GetDevice
 *
 * Obtain a reference to the OF device structure and the mapped address
 * of the OF device registered by this driver.
 *
 * DeviceID (input)
 *   Selection of a specific (sub)-device.
 *
 * Device_pp (output)
 *   Reference to the struct platform_device of the registered OF device.
 *
 * MappedBaseAddr_pp (output)
 *   Base address of the memory-mapped I/O.
 *
 * Return 0 on success.
 */
int
UMDevXS_OFDev_GetDevice(
              unsigned int DeviceID,
              struct platform_device ** OF_Device_pp,
              void __iomem ** MappedBaseAddr_pp);


#endif /* INCLUDE_GUARD_UMDEVXS_OFDEV_H */


/* umdevxs_ofdev.h */
