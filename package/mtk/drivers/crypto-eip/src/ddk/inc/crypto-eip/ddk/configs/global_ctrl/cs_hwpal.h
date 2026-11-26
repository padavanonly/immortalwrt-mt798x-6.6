/* cs_hwpal.h
 *
 * Configuration Settings for Driver Framework Implementation
 * for the Security-IP-197 Global Control Driver.
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

#ifndef INCLUDE_GUARD_CS_HWPAL_H
#define INCLUDE_GUARD_CS_HWPAL_H

// we accept a few settings from the top-level configuration file
#include "cs_driver.h"

// Host hardware platform specific extensions
#include "cs_hwpal_ext.h"

// Total number of devices allowed, must be at least as last as static list.
#define HWPAL_DEVICE_COUNT 40

// logging level for HWPAL Device
// Choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT
#undef LOG_SEVERITY_MAX
#ifdef DRIVER_PERFORMANCE
#define LOG_SEVERITY_MAX  LOG_SEVERITY_CRITICAL
#else // DRIVER_PERFORMANCE
#define LOG_SEVERITY_MAX  LOG_SEVERITY_WARN
#endif // DRIVER_PERFORMANCE

#ifdef INFO_DEBUG
#define HWPAL_LOG_SEVERITY  LOG_SEVERITY_INFO
#endif


// maximum allowed length for a device name
#define HWPAL_MAX_DEVICE_NAME_LENGTH 64

#define HWPAL_DEVICE_MAGIC   54333

#ifdef DRIVER_NAME
#define HWPAL_DRIVER_NAME DRIVER_NAME
#endif

// Trace memory leaks for DMAResource API implementation
#define HWPAL_TRACE_DMARESOURCE_LEAKS

// Is host platform 64-bit?
#ifdef DRIVER_64BIT_HOST
#define HWPAL_64BIT_HOST
#endif  // DRIVER_64BIT_HOST

// only define this if the platform hardware guarantees cache coherence of
// DMA buffers, i.e. when SW does not need to do coherence management.
#ifdef ARCH_X86
#define HWPAL_ARCH_COHERENT
#else
#undef HWPAL_ARCH_COHERENT
#if  defined(ARCH_ARM) || defined(ARCH_ARM64)
//#define HWPAL_DMARESOURCE_ARM_DCACHE_CTRL
//#define HWPAL_DMARESOURCE_CACHE_LINE_BYTE_COUNT 32
#endif //  defined(ARCH_ARM) || defined(ARCH_ARM64)
#endif

// Logging / tracing control
#ifndef DRIVER_PERFORMANCE
#define HWPAL_STRICT_ARGS_CHECK
#define HWPAL_DMARESOURCE_STRICT_ARGS_CHECKS
#ifdef ARCH_X86
// Enabled for DMAResource API implementation on x86 debugging purposes only
#undef HWPAL_ARCH_COHERENT
#endif
//#define HWPAL_TRACE_DEVICE_FIND
//#define HWPAL_TRACE_DEVICE_READ
//#define HWPAL_TRACE_DEVICE_WRITE
#endif

// enable code that looks at flag bit2 and performs actual endianness swap
//#define HWPAL_DEVICE_ENABLE_SWAP

// Use sleepable or non-sleepable lock ?
//#define HWPAL_LOCK_SLEEPABLE

// Use direct I/O bypassing the OS functions,
// I/O device must swap bytes in words
#ifdef DRIVER_ENABLE_SWAP_SLAVE
#define HWPAL_DEVICE_DIRECT_MEMIO
#endif

// Enable use of UMDevXS device
// Note: This parameter must be used for the Driver GC, PEC or PEC-PCL build!
#define HWPAL_USE_UMDEVXS_DEVICE


#endif // INCLUDE_GUARD_CS_HWPAL_H


/* end of file cs_hwpal.h */
