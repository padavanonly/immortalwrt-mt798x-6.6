/* cs_hwpal_ext.h
 *
 * Security-IP-197 (FPGA) PCI chip specific configuration parameters
 */

/*****************************************************************************
* Copyright (c) 2011-2022 by Rambus, Inc. and/or its subsidiaries.
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

// Max number of IRQ's supported by device
#define HWPAL_PLATFORM_IRQ_COUNT    1

// Index of the IRQ in the "interrupts" property of the Open Firmware device
// tree entry. 0 is the first IRQ listed, 1 is the second IRQ listed, etc.
#define HWPAL_PLATFORM_IRQ_IDX      0

#define HWPAL_REMAP_ADDRESSES       ;

// definition of static resources inside the PCI device
// Refer to the data sheet of device for the correct values
//                       Name         DevNr  Start    Last  Flags (see below)
#define HWPAL_DEVICES \
        HWPAL_DEVICE_ADD("EIP197_GLOBAL", 0, 0,       0xfffff, 7),  \
        HWPAL_DEVICE_ADD("EIP207_FLUE0",  0, 0x00000, 0x01fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR0",   0, 0x80000, 0x80fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR0",   0, 0x80000, 0x80fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR1",   0, 0x81000, 0x81fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR1",   0, 0x81000, 0x81fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR2",   0, 0x82000, 0x82fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR2",   0, 0x82000, 0x82fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR3",   0, 0x83000, 0x83fff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR3",   0, 0x83000, 0x83fff, 7),  \
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
        HWPAL_DEVICE_ADD("EIP202_CDR12",   0, 0x8c000, 0x8cfff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR12",   0, 0x8c000, 0x8cfff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_CDR13",   0, 0x8d000, 0x8dfff, 7),  \
        HWPAL_DEVICE_ADD("EIP202_RDR13",   0, 0x8d000, 0x8dfff, 7),  \
        HWPAL_DEVICE_ADD("EIP201_GLOBAL", 0, 0x9f800, 0x9f8ff, 7),  \
        HWPAL_DEVICE_ADD("EIP201_RING0",  0, 0x9e800, 0x9e8ff,  7),  \
        HWPAL_DEVICE_ADD("EIP201_RING1",  0, 0x9d800, 0x9d8ff,  7),  \
        HWPAL_DEVICE_ADD("EIP201_RING2",  0, 0x9c800, 0x9c8ff,  7),  \
        HWPAL_DEVICE_ADD("EIP201_RING3",  0, 0x9b800, 0x9b8ff,  7),  \
        HWPAL_DEVICE_ADD("EIP74",         0, 0xf7000, 0xf707f,  7),  \
        HWPAL_DEVICE_ADD("EIP201_CS",     0, 0xf7800, 0xf78ff,  7)

// Flags:
//   bit0 = Trace reads (requires HWPAL_TRACE_DEVICE_READ)
//   bit1 = Trace writes (requires HWPAL_TRACE_DEVICE_WRITE)
//   bit2 = Swap word endianness (requires HWPAL_DEVICE_ENABLE_SWAP)

// Note: HWPAL_DEVICES must be aligned with UMDEVXS_DEVICES in cs_umdevxs.h

// Enables DMA resources banks so that different memory regions can be used
// for DMA buffer allocation
#ifdef DRIVER_DMARESOURCE_BANKS_ENABLE
#define HWPAL_DMARESOURCE_BANKS_ENABLE
#endif // DRIVER_DMARESOURCE_BANKS_ENABLE

#ifdef HWPAL_DMARESOURCE_BANKS_ENABLE
// Definition of DMA banks, one dynamic and 1 static
//                                 Bank    Type   Shared  Cached  Addr  Blocks   Block Size
#define HWPAL_DMARESOURCE_BANKS                                                              \
        HWPAL_DMARESOURCE_BANK_ADD (0,       0,     0,      1,      0,    0,         0),     \
        HWPAL_DMARESOURCE_BANK_ADD (1,       1,     1,      1,      0,                       \
                                    DRIVER_DMA_BANK_ELEMENT_COUNT,                           \
                                    DRIVER_DMA_BANK_ELEMENT_BYTE_COUNT)
#endif // HWPAL_DMARESOURCE_BANKS_ENABLE


#endif /* CS_HWPAL_EXT_H_ */


/* end of file cs_hwpal_ext.h */
