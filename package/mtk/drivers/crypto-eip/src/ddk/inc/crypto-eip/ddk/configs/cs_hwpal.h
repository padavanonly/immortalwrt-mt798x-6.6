/* cs_hwpal.h
 *
 * Configuration Settings for Driver Framework Implementation
 * for the Security-IP-197 Driver.
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

#ifndef INCLUDE_GUARD_CS_HWPAL_H
#define INCLUDE_GUARD_CS_HWPAL_H

// we accept a few settings from the top-level configuration file
#include "cs_driver.h"
#include "cs_adapter.h"

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

// maximum allowed length for a device name
#define HWPAL_MAX_DEVICE_NAME_LENGTH 64

#define HWPAL_DEVICE_MAGIC   54333

#ifdef DRIVER_NAME
#define HWPAL_DRIVER_NAME DRIVER_NAME
#endif

#ifdef DRIVER_ALLOW_UNALIGNED_DMA
#define HWPAL_DRMARESOURCE_ALLOW_UNALIGNED_ADDRESS
#endif

// Trace memory leaks for DMAResource API implementation
#define HWPAL_TRACE_DMARESOURCE_LEAKS

// Is host platform 64-bit?
#ifdef DRIVER_64BIT_HOST
#define HWPAL_64BIT_HOST
// Is device 64-bit? Only makes sense on 64-bit host.
#ifdef DRIVER_64BIT_DEVICE
#define HWPAL_DMARESOURCE_64BIT
#endif  // DRIVER_64BIT_DEVICE
#endif  // DRIVER_64BIT_HOST

// only define this if the platform hardware guarantees cache coherence of
// DMA buffers, i.e. when SW does not need to do coherence management.
#ifdef ARCH_X86
#define HWPAL_ARCH_COHERENT
#else
#undef HWPAL_ARCH_COHERENT
#if defined(ARCH_ARM) || defined(ARCH_ARM64)
//#define HWPAL_DMARESOURCE_ARM_DCACHE_CTRL
//#define HWPAL_DMARESOURCE_CACHE_LINE_BYTE_COUNT 32
#endif // defined(ARCH_ARM) || defined(ARCH_ARM64)
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
//#define HWPAL_TRACE_DMARESOURCE_WRITE
//#define HWPAL_TRACE_DMARESOURCE_READ
//#define HWPAL_TRACE_DMARESOURCE_PREPOSTDMA
//#define HWPAL_TRACE_DMARESOURCE_BUF
#endif

// Use sleepable or non-sleepable lock ?
//#define HWPAL_LOCK_SLEEPABLE

#define HWPAL_DMA_NRESOURCES     ADAPTER_MAX_DMARESOURCE_HANDLES

// Use direct I/O bypassing the OS functions,
// I/O device must swap bytes in words
#ifdef DRIVER_ENABLE_SWAP_SLAVE
#define HWPAL_DEVICE_DIRECT_MEMIO
#endif

// Enable use of UMDevXS device
// Note: This parameter must be used for the Driver GC, PEC or PEC-PCL build!
#ifdef DRIVER_USE_SHDEVXS_DEVICE
#define HWPAL_USE_UMDEVXS_DEVICE
#endif

// DMA buffer allocation alignment
#ifdef DRIVER_DMA_ALIGNMENT_BYTE_COUNT
#define HWPAL_DMARESOURCE_DMA_ALIGNMENT_BYTE_COUNT      \
                                        DRIVER_DMA_ALIGNMENT_BYTE_COUNT
#endif

// Performance optimization: disable _Write32/Read32() resource handle validation
#ifdef DRIVER_PERFORMANCE
#define HWPAL_DMARESOURCE_OPT1
#endif

// Performance optimization: disable _Write32/Read32() endianness conversion branch
#ifdef DRIVER_PERFORMANCE
#ifndef DRIVER_SWAPENDIAN
#define HWPAL_DMARESOURCE_OPT2
#endif
#endif


#endif // INCLUDE_GUARD_CS_HWPAL_H


/* end of file cs_hwpal.h */
