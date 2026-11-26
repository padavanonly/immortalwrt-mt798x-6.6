/* cs_driver_ext.h
 *
 * Top-level Product Configuration Settings specific for FPGA.
 */

/*****************************************************************************
* Copyright (c) 2010-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_CS_DRIVER_EXT_H
#define INCLUDE_GUARD_CS_DRIVER_EXT_H

// Driver license for the module registration with the Linux kernel
#define DRIVER_LICENSE                                 "GPL"

// FPGA DMA buffer alignment is 8 bytes
#define DRIVER_DMA_ALIGNMENT_BYTE_COUNT                16

// Enables DMA banks
#define DRIVER_DMARESOURCE_BANKS_ENABLE

// DMA bank to use for SA bouncing by the PEC API implementation
#define DRIVER_PEC_BANK_SA                             1 // Static bank

// DMA resource bank for transform records allocation
// by the PCL API implementation, static bank
#define DRIVER_PCL_BANK_TRANSFORM                      DRIVER_PEC_BANK_SA

// DMA resource bank for flow records allocation by the PCL API implementation,
// static bank
#define DRIVER_PCL_BANK_FLOW                           DRIVER_PCL_BANK_TRANSFORM

// Each static bank for DMA resources must have 2 lists
#define DRIVER_LIST_HWPAL_MAX_NOF_INSTANCES            2

// Define this it all AIC devices have a separate external IRQ line.
//#define DRIVER_AIC_SEPARATE_IRQ


#endif /* Include Guard */


/* end of file cs_driver_ext.h */
