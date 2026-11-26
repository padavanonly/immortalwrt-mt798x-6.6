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

// FPGA board device ID.
#define HWPAL_DEVICE_ID             0x6018

// Xilinx PCI vendor ID
#define HWPAL_VENDOR_ID             0x10EE

#define HWPAL_MAGIC_PCICONFIGSPACE  0xFF434647      // 43 46 47 = C F G

#define HWPAL_REMAP_ADDRESSES   ;

#define HWPAL_DEVICE_TO_FIND       "PCI.0" // PCI Bar 0

#define HWPAL_DEVICE_ADD_PCI    HWPAL_DEVICE_ADD("PCI_CONFIG_SPACE", 0,  \
                                  HWPAL_MAGIC_PCICONFIGSPACE,            \
                                  HWPAL_MAGIC_PCICONFIGSPACE + 1024,     \
                                  7)

// definition of static resources inside the PCI device
// Refer to the data sheet of device for the correct values
//                       Name         DevNr  Start    Last  Flags (see below)
#define HWPAL_DEVICES \
        HWPAL_DEVICE_ADD_PCI, \
        HWPAL_DEVICE_ADD("EIP197_GLOBAL", 0, 0,        0xfffff,  7),  \
        HWPAL_DEVICE_ADD("EIP207_FLUE0",  0, 0x00000,  0x01fff,  7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR0",   0, 0x80000,  0x80fff,  7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR0",   0, 0x80000,  0x80fff,  7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR1",   0, 0x81000,  0x81fff,  7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR1",   0, 0x81000,  0x81fff,  7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR2",   0, 0x82000,  0x82fff,  7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR2",   0, 0x82000,  0x82fff,  7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR3",   0, 0x83000,  0x83fff,  7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR3",   0, 0x83000,  0x83fff,  7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR4",   0, 0x84000, 0x84fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR4",   0, 0x84000, 0x84fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR5",   0, 0x85000, 0x85fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR5",   0, 0x85000, 0x85fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR6",   0, 0x86000, 0x86fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR6",   0, 0x86000, 0x86fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR7",   0, 0x87000, 0x87fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR7",   0, 0x87000, 0x87fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR8",   0, 0x88000, 0x88fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR8",   0, 0x88000, 0x88fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR9",   0, 0x89000, 0x89fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR9",   0, 0x89000, 0x89fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR10",   0, 0x8a000, 0x8afff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR10",   0, 0x8a000, 0x8afff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR11",   0, 0x8b000, 0x8bfff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR11",   0, 0x8b000, 0x8bfff, 7),  \
        HWPAL_DEVICE_ADD("EIP201_GLOBAL", 0, 0x9f800,  0x9f8ff,  7),  \
        HWPAL_DEVICE_ADD("EIP201_RING0",  0, 0x9e800,  0x9e8ff,  7),  \
        HWPAL_DEVICE_ADD("EIP201_RING1",  0, 0x9d800,  0x9d8ff,  7),  \
        HWPAL_DEVICE_ADD("EIP201_RING2",  0, 0x9c800,  0x9c8ff,  7),  \
        HWPAL_DEVICE_ADD("EIP201_RING3",  0, 0x9b800,  0x9b8ff,  7),  \
        HWPAL_DEVICE_ADD("EIP201_CS",     0, 0xf7800,  0xf78ff,  7),  \
        HWPAL_DEVICE_ADD("BOARD_CTRL",    0, 0x100000, 0x102fff, 7)

// Flags:
//   bit0 = Trace reads (requires HWPAL_TRACE_DEVICE_READ)
//   bit1 = Trace writes (requires HWPAL_TRACE_DEVICE_WRITE)
//   bit2 = Swap word endianness (requires HWPAL_DEVICE_ENABLE_SWAP)

#define HWPAL_USE_MSI


#endif /* CS_HWPAL_EXT_H_ */


/* end of file cs_hwpal_ext.h */
