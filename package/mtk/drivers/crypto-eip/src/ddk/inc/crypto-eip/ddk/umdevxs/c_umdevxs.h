/* c_umdevxs.h
 *
 * Configuration options for UMDevXS driver.
 *
 * This file includes cs_umdevxs.h (from the product-level) and then provides
 * defaults for missing configuration switches.
 */

/*****************************************************************************
* Copyright (c) 2009-2022 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_C_UMDEVXS_H
#define INCLUDE_GUARD_C_UMDEVXS_H

#define HWPAL_LOCK_SLEEPABLE
#define HWPAL_TRACE_DMARESOURCE_LEAKS

// get the product-level configuration
#include "cs_umdevxs.h"

// Use non-cached DMA buffer mapping to user-space
//#define UMDEVXS_SMBUF_UNCACHED_MAPPING

// Use non-cached DMA buffer mapping to user-space for PCI
//#define UMDEVXS_PCI_UNCACHED_MAPPING

// Maximum size of the PCI device aperture (bar) to map
#ifndef UMDEVXS_PCI_DEVICE_RESOURCE_BYTE_COUNT
#define UMDEVXS_PCI_DEVICE_RESOURCE_BYTE_COUNT     0
#endif

// Use user-space buffer address for CPU Data Cache control (flush/invalidate)
//#define UMDEVXS_DCACHE_CTRL_UADDR

// Implement CPU Data Cache control entirely in user space
//#define UMDEVXS_DCACHE_CTRL_USERMODE

// if UMDEVXS_SMBUF_CACHE_COHERENT is defined then cache-coherent buffers
// will be mapped to the user-space and
// the high-level cache control functions will be used
#ifdef UMDEVXS_SMBUF_UNCACHED_MAPPING
#define UMDEVXS_SMBUF_CACHE_COHERENT
#endif

//#define UMDEVXS_ARCH_COHERENT

// if UMDEVXS_SMBUF_DIRECT_DCACHE_CONTROL is defined then
// the low-level UMDevXS functions will be used for
// the direct D-Cache control (flush/invalidate)
// unless UMDEVXS_SMBUF_CACHE_COHERENT is defined
//#define UMDEVXS_SMBUF_DIRECT_DCACHE_CONTROL

#if defined(UMDEVXS_DCACHE_CTRL_USERMODE) && \
    defined (UMDEVXS_SMBUF_DIRECT_DCACHE_CONTROL)
#error "Error: UMDEVXS_DCACHE_CTRL_USERMODE cannot be used together with" \
       "UMDEVXS_SMBUF_DIRECT_DCACHE_CONTROL"
#endif

// if UMDEVXS_SMBUF_DIRECT_DCACHE_CONTROL is defined then the ARM architecture
// specific Store Buffer control functionality can be used for which
// UMDEVXS_SMBUF_DSB_ENABLE must be defined
//#define UMDEVXS_SMBUF_DSB_ENABLE

// if UMDEVXS_SMBUF_DCACHE_WRITE_TROUGH is defined then no cache line flush
// (clean) operations will be done
//#define UMDEVXS_SMBUF_DCACHE_WRITE_TROUGH

// provide backup values

#ifndef UMDEVXS_LICENSE
#define UMDEVXS_LICENSE "GPL"
#endif

#ifndef UMDEVXS_DMARESOURCE_HANDLES_MAX
#define UMDEVXS_DMARESOURCE_HANDLES_MAX 128
#endif

#ifndef UMDEVXS_SMBUF_DCDUS
#define UMDEVXS_SMBUF_DCDUS 32
#endif

#ifndef UMDEVXS_PCI_VENDOR_ID
#define UMDEVXS_PCI_VENDOR_ID  0
#endif

#ifndef UMDEVXS_PCI_DEVICE_ID
#define UMDEVXS_PCI_DEVICE_ID  0
#endif

#ifndef UMDEVXS_PCI_BAR0_SUBSET_START
#define UMDEVXS_PCI_BAR0_SUBSET_START  0
#endif
#ifndef UMDEVXS_PCI_BAR1_SUBSET_START
#define UMDEVXS_PCI_BAR1_SUBSET_START  0
#endif
#ifndef UMDEVXS_PCI_BAR2_SUBSET_START
#define UMDEVXS_PCI_BAR2_SUBSET_START  0
#endif
#ifndef UMDEVXS_PCI_BAR3_SUBSET_START
#define UMDEVXS_PCI_BAR3_SUBSET_START  0
#endif

#ifndef UMDEVXS_PCI_BAR0_SUBSET_SIZE
#define UMDEVXS_PCI_BAR0_SUBSET_SIZE   1*1024*1024
#endif
#ifndef UMDEVXS_PCI_BAR1_SUBSET_SIZE
#define UMDEVXS_PCI_BAR1_SUBSET_SIZE   1*1024*1024
#endif
#ifndef UMDEVXS_PCI_BAR2_SUBSET_SIZE
#define UMDEVXS_PCI_BAR2_SUBSET_SIZE   1*1024*1024
#endif
#ifndef UMDEVXS_PCI_BAR3_SUBSET_SIZE
#define UMDEVXS_PCI_BAR3_SUBSET_SIZE   1*1024*1024
#endif

#ifndef UMDEVXS_SHMEM_BANK_STATIC_OFFSET
#define UMDEVXS_SHMEM_BANK_STATIC_OFFSET      0
#endif

#ifndef UMDEVXS_INTERRUPT_STATIC_IRQ
#define UMDEVXS_INTERRUPT_STATIC_IRQ -1
#endif

// Maximum number of interrupt controllers (IC's) to support
#ifndef UMDEVXS_INTERRUPT_IC_DEVICE_COUNT
#define UMDEVXS_INTERRUPT_IC_DEVICE_COUNT      1
#endif

// Default index of the interrupt controller to use
#ifndef UMDEVXS_INTERRUPT_IC_DEVICE_IDX
#define UMDEVXS_INTERRUPT_IC_DEVICE_IDX        0
#endif

#ifndef UMDEVXS_INTERRUPT_TRACE_FILTER
#define UMDEVXS_INTERRUPT_TRACE_FILTER 0
#endif

// logging level
#ifndef LOG_SEVERITY_MAX
#define LOG_SEVERITY_MAX LOG_SEVERITY_CRIT
#endif

// logging prefix
#ifndef UMDEVXS_LOG_PREFIX
#define UMDEVXS_LOG_PREFIX "UMDevXS: "
#endif

#ifndef UMDEVXS_MODULENAME
#define UMDEVXS_MODULENAME "umdevxs"
#endif

// if UMDEVXS_REMOVE_DEVICE is defined, make sure that the PCI and Simulation
// devices are removed as well
#ifdef UMDEVXS_REMOVE_DEVICE
#define UMDEVXS_REMOVE_PCI
#define UMDEVXS_REMOVE_SIMULATION
#define UMDEVXS_REMOVE_DEVICE_PCICFG
#define UMDEVXS_REMOVE_DEVICE_OF
#endif

// if UMDEVXS_REMOVE_PCI is defined, make sure that the PCI Config Space
// device is removed as well
#ifdef UMDEVXS_REMOVE_PCI
#define UMDEVXS_REMOVE_DEVICE_PCICFG
#endif

// Enable when using MSI interrupts on PCI
//#define UMDEVXS_USE_MSI

// Enable support for kernel modules to access the PCI device
// or interrupt
//#define UMDEVXS_ENABLE_KERNEL_SUPPORT

// Enable locking support for mapped devices, so each device can be used by
// at most one application.
//#define UMDEVXS_ENABLE_DEVICE_LOCK

// Enable checking that a mapped buffer really belongs to the application
// requesting it. This prohibits sharing of buffers between applications.
//#define UMDEVXS_ENABLE_BUFFER_APPCHECK

// Device DMA mask: maximum addressable bits for DMA memory
#ifndef UMDEVXS_SMBUF_ADDR_MASK
#define UMDEVXS_SMBUF_ADDR_MASK         0xffffffffULL
#endif

// Enable use of runtime power management functionality
//#define UMDEVXS_USE_RPM

// Maximum device map size for lock operations
#ifndef UMDEVXS_DEVICE_LOCK_MAX_SIZE
#define UMDEVXS_DEVICE_LOCK_MAX_SIZE 0x400000
#endif

// Some kernels before 5.0 contain backports of the 2-parameter access_ok
//#define UMDEVX_ACCESS_OK_BACKPORT

#endif /* INCLUDE_GUARD_C_UMDEVXS_H */


/* end of file c_umdevxs.h */
