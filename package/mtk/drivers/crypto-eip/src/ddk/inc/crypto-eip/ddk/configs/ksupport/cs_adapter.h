/* cs_adapter.h
 *
 * Configuration Settings for the SLAD Adapter module.
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


#include "cs_driver.h"
#include "cs_umdevxs.h"

/****************************************************************************
 * Adapter Global configuration parameters
 */

#ifndef LOG_SEVERITY_MAX
#define LOG_SEVERITY_MAX LOG_SEVERITY_WARN
#endif

#ifdef UMDEVXS_MODULENAME
#define ADAPTER_DRIVER_NAME             UMDEVXS_MODULENAME
#endif

// filter for printing interrupts
//#define ADAPTER_INTERRUPTS_TRACEFILTER 0x0007FFFF

// Is host platform 64-bit?
#ifdef DRIVER_64BIT_HOST
#define ADAPTER_64BIT_HOST
#endif  // DRIVER_64BIT_HOST

// Is device 64-bit?
#ifdef DRIVER_64BIT_DEVICE
#define ADAPTER_64BIT_DEVICE
#endif  // DRIVER_64BIT_DEVICE

// Maximum number of descriptors that can be submitted in one go.
#ifdef DRIVER_MAX_PECLOGICDESCR
#define ADAPTER_MAX_PECLOGICDESCR       DRIVER_MAX_PECLOGICDESCR
#else
#define ADAPTER_MAX_PECLOGICDESCR       10
#endif

// switch to remove support for bounce buffers
#ifndef DRIVER_BOUNCEBUFFERS
#define ADAPTER_REMOVE_BOUNCEBUFFERS
#endif

#define ADAPTER_GLOBAL_DEVICE_NAME      "EIP197_GLOBAL"

// the number of packets in the result ring before the related interrupt
// is activated. This reduces the number of interrupts and allows the
// host to get many results in one call to PEC_Packet_Get.
#ifdef DRIVER_INTERRUPT_COALESCING
#define ADAPTER_DESCRIPTORDONECOUNT     4
#else
#define ADAPTER_DESCRIPTORDONECOUNT     1
#endif

// maximum delay until activating an interrupt
// after writing a result descriptor to the result ring
// desired maximum time: T in seconds
// calculate configuration value N as follows:
//   N = T (seconds) * Engine frequency (Hz) / 1024
// example: T = 100 microseconds,
//          Engine frequency = 100 MHz
// N = (1 / 10 000) sec * 100 000 000 Hz / 1024 ~= 10
#ifdef DRIVER_INTERRUPT_COALESCING
#define ADAPTER_DESCRIPTORDONETIMEOUT   20
#else
#define ADAPTER_DESCRIPTORDONETIMEOUT   0
#endif

#ifdef DRIVER_DMARESOURCE_BANKS_ENABLE
#define ADAPTER_DMARESOURCE_BANKS_ENABLE
#endif


/****************************************************************************
 * Adapter PEC configuration parameters
 */

// enable debug checks
#ifndef DRIVER_PERFORMANCE
#define ADAPTER_PEC_DBG
#define ADAPTER_PEC_STRICT_ARGS
#endif

// Maximum number of supported Packet Engine devices
#ifdef DRIVER_MAX_NOF_RING_TO_USE
#define ADAPTER_PEC_DEVICE_COUNT       DRIVER_MAX_NOF_RING_TO_USE
#else
#define ADAPTER_PEC_DEVICE_COUNT       2
#endif

// the number of SA records the driver can register
// if State record or ARC4 State record are used together with SA record
// then this number should be increased by factor of 2 or 3 respectively
#ifndef DRIVER_PEC_MAX_SAS
#define ADAPTER_PEC_MAX_SAS         60
#else
#define ADAPTER_PEC_MAX_SAS         DRIVER_PEC_MAX_SAS
#endif

// the number of packets the driver can buffer either in its input buffer
// a.k.a. Packet Descriptor Ring (PDR) or its output buffer
// a.k.a. Result Descriptor Ring (RDR)
#ifndef DRIVER_PEC_MAX_PACKETS
#define ADAPTER_PEC_MAX_PACKETS     10
#else
#define ADAPTER_PEC_MAX_PACKETS     DRIVER_PEC_MAX_PACKETS
#endif

// one for commands, the other for results
#ifdef DRIVER_PE_ARM_SEPARATE
#define ADAPTER_PEC_SEPARATE_RINGS
#endif

#ifdef ADAPTER_MAX_PECLOGICDESCR
#define ADAPTER_PEC_MAX_LOGICDESCR  ADAPTER_MAX_PECLOGICDESCR
#endif

#if ADAPTER_PEC_MAX_LOGICDESCR > ADAPTER_PEC_MAX_PACKETS
#error "Error: ADAPTER_PEC_MAX_LOGICDESCR > ADAPTER_PEC_MAX_PACKETS"
#endif

#ifndef DRIVER_BOUNCEBUFFERS
#define ADAPTER_PEC_REMOVE_BOUNCEBUFFERS
#endif

#ifdef DRIVER_SCATTERGATHER

#define ADAPTER_PEC_ENABLE_SCATTERGATHER

// the maximum number of fragments per packet
#define ADAPTER_PEC_MAX_FRAGMENTS_PER_PACKET    4

// the maximum number of descriptors in the gather and scatter rings
#define ADAPTER_PEC_SC_RING_MAX_DESCRIPTORS \
        (ADAPTER_PEC_MAX_FRAGMENTS_PER_PACKET * ADAPTER_PEC_MAX_PACKETS)

#endif /* scatter/gather */

#ifdef DRIVER_INTERRUPTS
#define ADAPTER_PEC_INTERRUPTS_ENABLE
#endif

// enable for big-endian CPU
#ifdef DRIVER_SWAPENDIAN
#define ADAPTER_PEC_ARMRING_ENABLE_SWAP
#endif //DRIVER_SWAPENDIAN

// DMA resource bank for SLAD PEC Adapter, used for bouncing SA buffers
#ifdef DRIVER_PEC_BANK_SA
#define ADAPTER_PEC_BANK_SA                     DRIVER_PEC_BANK_SA
#endif


/****************************************************************************
 * Adapter EIP-202 configuration parameters
 */

// enable debug checks
#ifndef DRIVER_PERFORMANCE
#define ADAPTER_EIP202_STRICT_ARGS
#endif

#ifdef ADAPTER_DRIVER_NAME
#define ADAPTER_EIP202_DRIVER_NAME          ADAPTER_DRIVER_NAME
#endif

#ifdef DRIVER_USE_SHDEVXS
// Use Shared Device Access API for EIP-207 Record Cache access
#define ADAPTER_EIP202_USE_SHDEVXS
#endif

#ifdef ADAPTER_GLOBAL_DEVICE_NAME
#define ADAPTER_EIP202_GLOBAL_DEVICE_NAME    ADAPTER_GLOBAL_DEVICE_NAME
#endif

#ifdef DRIVER_64BIT_DEVICE
#define ADAPTER_EIP202_64BIT_DEVICE
#endif

#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
#define ADAPTER_EIP202_ENABLE_SCATTERGATHER
#endif

#ifdef ADAPTER_PEC_SEPARATE_RINGS
#define ADAPTER_EIP202_SEPARATE_RINGS
#endif

#ifdef ADAPTER_PEC_MAX_PACKETS
#define ADAPTER_EIP202_MAX_PACKETS      ADAPTER_PEC_MAX_PACKETS
#endif

#ifdef ADAPTER_DESCRIPTORDONECOUNT
#define ADAPTER_EIP202_DESCRIPTORDONECOUNT  ADAPTER_DESCRIPTORDONECOUNT
#endif

#ifdef ADAPTER_DESCRIPTORDONETIMEOUT
#define ADAPTER_EIP202_DESCRIPTORDONETIMEOUT  ADAPTER_DESCRIPTORDONETIMEOUT
#endif

#ifdef ADAPTER_PEC_MAX_LOGICDESCR
#define ADAPTER_EIP202_MAX_LOGICDESCR   ADAPTER_PEC_MAX_LOGICDESCR
#endif

// switch to remove support for bounce buffers
#ifndef DRIVER_BOUNCEBUFFERS
#define ADAPTER_EIP202_REMOVE_BOUNCEBUFFERS
#endif

// descriptor spacing in words, allowing cache line alignment
// ring memory start alignment will use same value
#define ADAPTER_SYSTEM_DCACHE_LINE_SIZE_BYTES  32
#define ADAPTER_EIP97_DESCRIPTORSPACING_WORDS \
                (ADAPTER_SYSTEM_DCACHE_LINE_SIZE_BYTES / 4)

// Threshold for DMA input (read) and output (write)
// The transfer is initiated when the number of positions in
// the engine buffer are used (output) or free (input).
// It also affects the maximum length of the burst.
#define ADAPTER_EIP97_DMATHRESHOLD_INPUT   0x08
#define ADAPTER_EIP97_DMATHRESHOLD_OUTPUT  0x08

#ifdef DRIVER_ENABLE_SWAP_MASTER
// This parameter enables the byte swap in 32-bit words
// for the EIP-202 CD Manager master interface
#define ADAPTER_EIP202_CDR_BYTE_SWAP_ENABLE

// This parameter enables the byte swap in 32-bit words
// for the EIP-202 RD Manager master interface
#define ADAPTER_EIP202_RDR_BYTE_SWAP_ENABLE
#endif // DRIVER_ENABLE_SWAP_MASTER

#ifdef DRIVER_SWAPENDIAN
// This parameter enables the endianness conversion by the Host CPU
// for the ring descriptors
#define ADAPTER_EIP202_ARMRING_ENABLE_SWAP
#endif //DRIVER_SWAPENDIAN

#ifdef DRIVER_INTERRUPTS
#define ADAPTER_EIP202_INTERRUPTS_ENABLE
#endif

#ifdef ADAPTER_INTERRUPTS_TRACEFILTER
#define ADAPTER_EIP202_INTERRUPTS_TRACEFILTER   ADAPTER_INTERRUPTS_TRACEFILTER
#endif

#ifdef ADAPTER_PEC_BANK_RING
#define ADAPTER_EIP202_BANK_RING        ADAPTER_PEC_BANK_RING
#endif

// Maximum number of supported EIP-202 devices
// (one device includes CDR Manager and RDR Manager)
#ifdef DRIVER_MAX_NOF_RING_TO_USE
#define ADAPTER_EIP202_DEVICE_COUNT     DRIVER_MAX_NOF_RING_TO_USE
#else
#define ADAPTER_EIP202_DEVICE_COUNT     2
#endif


//Set this if the IRQs for the second ring (Ring 1) are used.
#define ADAPTER_EIP202_HAVE_RING1_IRQ
//Set this if the IRQs for the third ring (Ring 2) are used.
#define ADAPTER_EIP202_HAVE_RING2_IRQ
//Set this if the IRQs for the fourth ring (Ring 3) are used.
#define ADAPTER_EIP202_HAVE_RING3_IRQ

#define ADAPTER_EIP202_DEVICES                                      \
    {                                                               \
         3,                /* CDR IRQ */                            \
         "EIP202_CDR0",    /* CDR Device Name */                    \
         4,                /* RDR IRQ */                            \
         "EIP202_RDR0"     /* RDR Dev Name */                       \
    },                                                              \
    {                                                               \
         5,                /* CDR IRQ */                            \
         "EIP202_CDR1",    /* CDR Device Name */                    \
         6,                /* RDR IRQ */                            \
         "EIP202_RDR1"     /* RDR Dev Name */                       \
    }

#if ADAPTER_PEC_DEVICE_COUNT != ADAPTER_EIP202_DEVICE_COUNT
#error "Adapter PEC device configuration is invalid"
#endif

#ifdef ADAPTER_PEC_BANK_SA
#define ADAPTER_EIP202_BANK_SA          ADAPTER_PEC_BANK_SA
#endif

#ifdef DRIVER_USE_EXTENDED_DESCRIPTOR
// Define if the hardware uses extended command and result descriptors.
#define ADAPTER_EIP202_USE_EXTENDED_DESCRIPTOR
#endif


/****************************************************************************
 * Adapter DMAResource handles maximum calculation
 */

// the number of buffers that can be tracked by the driver-internal
// administration: ADAPTER_MAX_DMARESOURCE_HANDLES
#ifndef ADAPTER_REMOVE_BOUNCEBUFFERS
#define ADAPTER_BOUNCE_FACTOR   2
#else
#define ADAPTER_BOUNCE_FACTOR   1
#endif

// internal DMA-safe buffers are used for rings, they are never bounced
#ifdef ADAPTER_PEC_SEPARATE_RINGS
#define ADAPTER_PEC_MAX_RINGS_DMARESOURCE_HANDLES   2
#else
#define ADAPTER_PEC_MAX_RINGS_DMARESOURCE_HANDLES   1
#endif

// SC buffers must be allocated DMA-safe by applications,
// they are never bounced
#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
// 2 separate rings are used, one for gather and one for scatter
// 1 DMA resource is used for SG list and 1 is extra
#define ADAPTER_PEC_MAX_SC_DMARESOURCE_HANDLES   \
    (2+1+ADAPTER_PEC_SC_RING_MAX_DESCRIPTORS+1)
#else
#define ADAPTER_PEC_MAX_SC_DMARESOURCE_HANDLES   0
#endif

#define ADAPTER_MAX_DMARESOURCE_HANDLES \
    (ADAPTER_BOUNCE_FACTOR * \
     (ADAPTER_PEC_MAX_SAS + \
      ADAPTER_PEC_DEVICE_COUNT * ADAPTER_PEC_MAX_PACKETS) + \
     ADAPTER_PEC_MAX_RINGS_DMARESOURCE_HANDLES +         \
     ADAPTER_PEC_MAX_SC_DMARESOURCE_HANDLES)

// Request the IRQ through the UMDevXS driver.
#define ADAPTER_EIP202_USE_UMDEVXS_IRQ

// Adapter configuration extensions
#include "cs_adapter_ext.h"


/* end of file cs_adapter.h */
