/* umdevxs_pcidev.h
 *
 * Exported PCIDev API of the UMDexXS/UMPCI driver.
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

#ifndef INCLUDE_GUARD_UMDEVXS_PCIDEV_H
#define INCLUDE_GUARD_UMDEVXS_PCIDEV_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include <linux/pci.h>

/*----------------------------------------------------------------------------
 * Definitions and macros
 */



/*----------------------------------------------------------------------------
 * UMDevXS_PCIDev_Get
 *
 * Obtain a reference to the PCI device structure and the mapped address
 * of the first BAR of the PCI device registered by this driver.
 *
 * DeviceID (input)
 *   Selection of a specific (sub)-device.
 * Device_p (output)
 *   Reference to the struct pci_dev of the registered PCI device.
 * MappedBaseAddr_p (output)
 *   Base address of the memory-mapped I/O.
 *
 * Return 0 on success.
 */
int
UMDevXS_PCIDev_Get(
              unsigned int DeviceID,
              struct pci_dev ** Device_pp,
              void __iomem ** MappedBaseAddr_pp);


#endif /* INCLUDE_GUARD_UMDEVXS_PCIDEV_H */


/* umdevxs_pcidev.h */
