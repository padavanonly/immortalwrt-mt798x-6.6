/* cs_driver.h
 *
 * Top-level Product Configuration Settings.
 */

/*****************************************************************************
* Copyright (c) 2011-2021 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_CS_DRIVER_H
#define INCLUDE_GUARD_CS_DRIVER_H

#include "cs_systemtestconfig.h"      // defines SYSTEMTEST_CONFIG_Cnn

// Host hardware platform specific extensions
#include "cs_driver_ext.h"


// Driver name used for reporting
#define DRIVER_NAME     "Security_IP-197"

// activate in case of endianness difference between CPU and EIP
// to ask driver to swap byte order of all control words
// we assume that if ARCH is not x86, then CPU is big endian
#ifdef ARCH_POWERPC
#define DRIVER_SWAPENDIAN
#endif //ARCH_POWERPC

// Maximum number of Classification Engines (EIP-207c) to use
// Note: one Processing Engine includes one Classification Engine
#define DRIVER_CS_MAX_NOF_CE_TO_USE                    DRIVER_MAX_NOF_PE_TO_USE

#define DRIVER_MAX_NOF_PE_TO_USE                       12

// Size (in entries) of the Flow Hash Table (FHT)
// used by the EIP-207 Classification Engine
#define DRIVER_PCL_FLOW_HASH_ENTRIES_COUNT             512

// Enable use of UMDevXS and SHDevXS API's
// Note: This parameter must be used for the Driver PEC-PCL build!
#define DRIVER_USE_SHDEVXS

// Some applications allocate network packets at unaligned addresses.
// To avoid bouncing these buffers, you can allow unaligned buffers if
// caching is properly handled in hardware.
//#define DRIVER_ALLOW_UNALIGNED_DMA

// C0 = ARM, Separate, Interrupt coalescing, BB=Y, Perf=N, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C0
//#define DRIVER_PE_TCM
#define DRIVER_PE_ARM_SEPARATE
#define DRIVER_INTERRUPTS
#define DRIVER_INTERRUPT_COALESCING
#define DRIVER_BOUNCEBUFFERS
//#define DRIVER_PERFORMANCE
#define DRIVER_SCATTERGATHER
#define DRIVER_PEC_MAX_SAS         64
#define DRIVER_PEC_MAX_PACKETS     32
#define DRIVER_MAX_PECLOGICDESCR   32
#endif

// C1 = ARM, Overlapped, Polling, BB=N, Perf=N, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C1
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_BOUNCEBUFFERS
//#define DRIVER_PERFORMANCE
//#define DRIVER_SCATTERGATHER
#endif

// C2 = ARM, Overlapped, Interrupts, BB=N, Perf=N, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C2
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_BOUNCEBUFFERS
//#define DRIVER_PERFORMANCE
//#define DRIVER_SCATTERGATHER
#endif

// C3 = ARM, Separate, Polling, BB=N, Perf=N, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C3
//#define DRIVER_PE_TCM
#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_BOUNCEBUFFERS
//#define DRIVER_PERFORMANCE
//#define DRIVER_SCATTERGATHER
#endif

// C4 = ARM, Overlapped, Polling, BB=Yes, Perf=N, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C4
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
#define DRIVER_BOUNCEBUFFERS
//#define DRIVER_PERFORMANCE
//#define DRIVER_SCATTERGATHER
#endif

// C5 = ARM, Overlapped, Interrupts, BB=Yes, Perf=N, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C5
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
#define DRIVER_BOUNCEBUFFERS
//#define DRIVER_PERFORMANCE
//#define DRIVER_SCATTERGATHER
#endif

// C6 = ARM, Separate, Polling, BB=Yes, Perf=N, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C6
//#define DRIVER_PE_TCM
#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
#define DRIVER_BOUNCEBUFFERS
//#define DRIVER_PERFORMANCE
//#define DRIVER_SCATTERGATHER
#endif

// C8 = ARM, Polling, Separate, BB=N, Perf=Yes, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C8
//#define DRIVER_PE_TCM
#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_BOUNCEBUFFERS
#define DRIVER_PERFORMANCE
//#define DRIVER_SCATTERGATHER
#define DRIVER_PEC_MAX_SAS         64
#define DRIVER_PEC_MAX_PACKETS     32
#define DRIVER_MAX_PECLOGICDESCR   32
#endif

// C9 = ARM, Interrupts, Overlapped, BB=N, Perf=Yes, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C9
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_BOUNCEBUFFERS
#define DRIVER_PERFORMANCE
//#define DRIVER_SCATTERGATHER
#define DRIVER_PEC_MAX_SAS         64
#define DRIVER_PEC_MAX_PACKETS     32
#define DRIVER_MAX_PECLOGICDESCR   32
#endif

// C10 = ARM, Polling, Overlapped, BB=N, Perf=Yes, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C10
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_BOUNCEBUFFERS
#define DRIVER_PERFORMANCE
//#define DRIVER_SCATTERGATHER
#endif

// C11 = ARM, Polling, Overlapped, BB=Yes, Perf=Yes, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C11
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
#define DRIVER_BOUNCEBUFFERS
#define DRIVER_PERFORMANCE
//#define DRIVER_SCATTERGATHER
#define DRIVER_PEC_MAX_SAS         64
#define DRIVER_PEC_MAX_PACKETS     32
#define DRIVER_MAX_PECLOGICDESCR   32
#endif

// C12 = ARM, Interrupts, Overlapped, BB=Yes, Perf=Yes, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C12
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
#define DRIVER_BOUNCEBUFFERS
#define DRIVER_PERFORMANCE
//#define DRIVER_SCATTERGATHER
#endif

// C13 = ARM, Polling, Separate, BB=Yes, Perf=Yes, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C13
//#define DRIVER_PE_TCM
#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
#define DRIVER_BOUNCEBUFFERS
#define DRIVER_PERFORMANCE
//#define DRIVER_SCATTERGATHER
#endif

// C14 = ARM, Polling, Overlapped, BB=N, Perf=N, SG=Yes
#ifdef SYSTEMTEST_CONFIGURATION_C14
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_BOUNCEBUFFERS
//#define DRIVER_PERFORMANCE
#define DRIVER_SCATTERGATHER
#define DRIVER_PEC_MAX_PACKETS     64
#define DRIVER_MAX_PECLOGICDESCR   64
#endif

// C15 = ARM, Interrupts, Overlapped, BB=N, Perf=N, SG=Yes
#ifdef SYSTEMTEST_CONFIGURATION_C15
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_BOUNCEBUFFERS
//#define DRIVER_PERFORMANCE
#define DRIVER_SCATTERGATHER
#define DRIVER_PEC_MAX_PACKETS     64
#define DRIVER_MAX_PECLOGICDESCR   64
#endif

// C16 = ARM, Polling, Separate, BB=N, Perf=N, SG=Yes
#ifdef SYSTEMTEST_CONFIGURATION_C16
//#define DRIVER_PE_TCM
#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_BOUNCEBUFFERS
//#define DRIVER_PERFORMANCE
#define DRIVER_SCATTERGATHER
#define DRIVER_PEC_MAX_PACKETS     64
#define DRIVER_MAX_PECLOGICDESCR   64
#endif

// C17 = ARM, Interrupt + Coalescing, Overlapped, BB=N, Perf=Yes, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C17
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
#define DRIVER_INTERRUPTS
#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_BOUNCEBUFFERS
#define DRIVER_PERFORMANCE
//#define DRIVER_SCATTERGATHER
#define DRIVER_PEC_MAX_SAS           64
#define DRIVER_PEC_MAX_PACKETS       32
#define DRIVER_MAX_PECLOGICDESCR     32
#endif

// C18 = ARM, Polling, Overlapped, BB=N, Perf=No, SG=N Byteswap on
#ifdef SYSTEMTEST_CONFIGURATION_C18
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_BOUNCEBUFFERS
//#define DRIVER_PERFORMANCE
//#define DRIVER_SCATTERGATHER
// PLB FPGA does not allow for endianness conversion
// in the Board Control device slave interface required on x86,
// so just disable the test
#ifndef EIP197_BUS_VERSION_PLB
#define DRIVER_SWAPENDIAN         // Host and device have different endianness
#ifdef ARCH_POWERPC
#undef  DRIVER_SWAPENDIAN         // Switch off byte swap by the host processor
#endif //ARCH_POWERPC
// Enable byte swap by the Engine slave interface
#define DRIVER_ENABLE_SWAP_SLAVE
// Enable byte swap by the Engine master interface
#define DRIVER_ENABLE_SWAP_MASTER
#endif // not EIP197_BUS_VERSION_PLB
#endif // SYSTEMTEST_CONFIGURATION_C18

#ifndef DRIVER_PEC_MAX_SAS
#define DRIVER_PEC_MAX_SAS                      60
#endif


// EIP-197 hardware specific extensions
#include "cs_driver_ext2.h"


#endif /* Include Guard */


/* end of file cs_driver.h */
