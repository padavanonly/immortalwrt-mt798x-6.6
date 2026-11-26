/* cs_driver.h
 *
 * Top-level Product Configuration Settings.
 */

/*****************************************************************************
* Copyright (c) 2012-2021 by Rambus, Inc. and/or its subsidiaries.
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
#include "cs_driver_ext.h"            // extensions

// Driver name used for reporting
#define DRIVER_NAME     "Security_IP-197_GlobalControl"

// activate in case of endianness difference between CPU and EIP
// to ask driver to swap byte order of all control words
// we assume that if ARCH is PowerPC, then CPU is big endian
#ifdef ARCH_POWERPC
#define DRIVER_SWAPENDIAN
#endif //ARCH_POWERPC

#define DRIVER_MAX_NOF_PE_TO_USE                       12

// Maximum number of Classification Engines (EIP-207c) to use
// Note: one Processing Engine includes one Classification Engine
#define DRIVER_CS_MAX_NOF_CE_TO_USE                    DRIVER_MAX_NOF_PE_TO_USE

// Maximum number of Flow Hash Tables to use in the Classification (PCL) Driver
#define DRIVER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE      1


// C0 = ARM, Separate, Interrupt coalescing, BB=Y, Perf=N, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C0
//#define DRIVER_PE_TCM
#define DRIVER_PE_ARM_SEPARATE
#define DRIVER_INTERRUPTS
#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_PERFORMANCE
#endif

// C1 = ARM, Overlapped, Polling, BB=N, Perf=N, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C1
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_PERFORMANCE
#endif

// C2 = ARM, Overlapped, Interrupts, BB=N, Perf=N, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C2
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_PERFORMANCE
#endif

// C3 = ARM, Separate, Polling, BB=N, Perf=N, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C3
//#define DRIVER_PE_TCM
#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_PERFORMANCE
#endif

// C4 = ARM, Overlapped, Polling, BB=Yes, Perf=N, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C4
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_PERFORMANCE
#endif

// C5 = ARM, Overlapped, Interrupts, BB=Yes, Perf=N, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C5
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_PERFORMANCE
#endif

// C6 = ARM, Separate, Polling, BB=Yes, Perf=N, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C6
//#define DRIVER_PE_TCM
#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_PERFORMANCE
#endif

// C8 = ARM, Polling, Separate, BB=N, Perf=Yes, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C8
//#define DRIVER_PE_TCM
#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
#define DRIVER_PERFORMANCE
#endif

// C9 = ARM, Interrupts, Overlapped, BB=N, Perf=Yes, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C9
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
#define DRIVER_PERFORMANCE
#endif

// C10 = ARM, Polling, Overlapped, BB=N, Perf=Yes, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C10
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
#define DRIVER_PERFORMANCE
#endif

// C11 = ARM, Polling, Overlapped, BB=Yes, Perf=Yes, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C11
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
#define DRIVER_PERFORMANCE
#endif

// C12 = ARM, Interrupts, Overlapped, BB=Yes, Perf=Yes, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C12
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
#define DRIVER_PERFORMANCE
#endif

// C13 = ARM, Polling, Separate, BB=Yes, Perf=Yes, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C13
//#define DRIVER_PE_TCM
#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
#define DRIVER_PERFORMANCE
#endif

// C14 = ARM, Polling, Overlapped, BB=N, Perf=N, SG=Yes
#ifdef SYSTEMTEST_CONFIGURATION_C14
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_PERFORMANCE
#endif

// C15 = ARM, Interrupts, Overlapped, BB=N, Perf=N, SG=Yes
#ifdef SYSTEMTEST_CONFIGURATION_C15
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_PERFORMANCE
#endif

// C16 = ARM, Polling, Separate, BB=N, Perf=N, SG=Yes
#ifdef SYSTEMTEST_CONFIGURATION_C16
//#define DRIVER_PE_TCM
#define DRIVER_PE_ARM_SEPARATE
//#define DRIVER_INTERRUPTS
//#define DRIVER_INTERRUPT_COALESCING
//#define DRIVER_PERFORMANCE
#endif

// C17 = ARM, Interrupt + Coalescing, Overlapped, BB=N, Perf=Yes, SG=N
#ifdef SYSTEMTEST_CONFIGURATION_C17
//#define DRIVER_PE_TCM
//#define DRIVER_PE_ARM_SEPARATE
#define DRIVER_INTERRUPTS
#define DRIVER_INTERRUPT_COALESCING
#define DRIVER_PERFORMANCE
#endif

#ifdef SYSTEMTEST_CONFIGURATION_C18
//#define DRIVER_PERFORMANCE
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

// EIP-197 hardware specific extensions
#include "cs_driver_ext2.h"


#endif /* Include Guard */


/* end of file cs_driver.h */
