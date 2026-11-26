/* cs_hwpal_ext.h
 *
 * Security-IP-197 (FPGA) PCI chip specific configuration parameters
 */

/*****************************************************************************
* Copyright (c) 2011-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef CS_HWPAL_EXT_H_
#define CS_HWPAL_EXT_H_


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// For obtaining the IRQ number
#ifdef DRIVER_INTERRUPTS
#define HWPAL_INTERRUPTS
#endif

// Disable PCI Configuration Space support support
#define HWPAL_REMOVE_DEVICE_PCICONFIGSPACE

// Device name in the Device Tree Structure
#define HWPAL_PLATFORM_DEVICE_NAME  "security-ip-197-srv"

#define HWPAL_REMAP_ADDRESSES   ;

// definition of static resources inside the PCI device
// Refer to the data sheet of device for the correct values
//                       Name         DevNr  Start    Last  Flags (see below)
#define HWPAL_DEVICES \
        HWPAL_DEVICE_ADD("EIP197_GLOBAL", 0, 0,        0xfffff,  7),  \
        HWPAL_DEVICE_ADD("EIP201_GLOBAL", 0, 0x9f800,  0x9f8ff,  7),  \
        HWPAL_DEVICE_ADD("EIP74",         0, 0xf7000, 0xf707f,  7),  \
        HWPAL_DEVICE_ADD("EIP201_CS",     0, 0xf7800,  0xf78ff,  7)

// Flags:
//   bit0 = Trace reads (requires HWPAL_TRACE_DEVICE_READ)
//   bit1 = Trace writes (requires HWPAL_TRACE_DEVICE_WRITE)
//   bit2 = Swap word endianness (requires HWPAL_DEVICE_ENABLE_SWAP)

// Note: HWPAL_DEVICES must be aligned with UMDEVXS_DEVICES in cs_umdevxs.h


#endif /* CS_HWPAL_EXT_H_ */


/* end of file cs_hwpal_ext.h */
