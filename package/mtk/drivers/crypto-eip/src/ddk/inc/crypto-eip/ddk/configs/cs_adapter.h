/* cs_adapter.h
 *
 * Configuration Settings for the SLAD Adapter Combined module.
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : ddk197                                                   */
/*   Version       : 5.6.1                                                    */
/*   Configuration : DDK-197-GPL                                              */
/*                                                                            */
/*   Date          : 2022-Dec-16                                              */
/*                                                                            */
/* Copyright (c) 2008-2022 by Rambus, Inc. and/or its subsidiaries.           */
/*                                                                            */
/* This program is free software: you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation, either version 2 of the License, or          */
/* any later version.                                                         */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               */
/* GNU General Public License for more details.                               */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program. If not, see <http://www.gnu.org/licenses/>.       */
/* -------------------------------------------------------------------------- */

// we accept a few settings from the top-level configuration file
#include "cs_driver.h"

// Adapter extensions
#include "cs_adapter_ext.h"

// DDK configuration
#include "cs_ddk197.h"

/****************************************************************************
 * Adapter Global configuration parameters
 */

// log level for the entire adapter (for now)
// choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT
#ifndef LOG_SEVERITY_MAX
#ifdef DRIVER_PERFORMANCE
#define LOG_SEVERITY_MAX  LOG_SEVERITY_CRITICAL
#else
#define LOG_SEVERITY_MAX  LOG_SEVERITY_WARN
#endif
#endif

// Do a register dump after ring initialization
//#define ADAPTER_EIP202_ADD_INIT_DIAGNOSTICS


#define ADAPTER_VERSION_STRING DDK_EIP197_VERSION_STRING

#ifdef DRIVER_NAME
#define ADAPTER_DRIVER_NAME     DRIVER_NAME
#else
#define ADAPTER_DRIVER_NAME     "Security"
#endif

#ifdef DRIVER_LICENSE
#define ADAPTER_LICENSE         DRIVER_LICENSE
#else
#define ADAPTER_LICENSE         "GPL"
#endif

// Allow longer firmware path names.
#define ADAPTER_FIRMWARE_NAMELEN_MAX  256

// PCI configuration value: Cache Line Size, in 32bit words
// Advised value: 1
#define ADAPTER_PCICONFIG_CACHELINESIZE   1

// PCI configuration value: Master Latency Timer, in PCI bus clocks
// Advised value: 0xf8
#define ADAPTER_PCICONFIG_MASTERLATENCYTIMER 0xf8

// filter for printing interrupts
#ifdef DRIVER_INTERRUPTS
#define ADAPTER_INTERRUPTS_TRACEFILTER 0x00
#else
#define ADAPTER_INTERRUPTS_TRACEFILTER 0x00
#endif

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

// Fix up the packet automatically. (inserting appended fields into header)
#define ADAPTER_AUTO_FIXUP

#define ADAPTER_GLOBAL_DEVICE_NAME          "EIP197_GLOBAL"

#define ADAPTER_GLOBAL_DBG_STATISTICS

// the number of packets in the result ring before the related interrupt
// is activated. This reduces the number of interrupts and allows the
// host to get many results in one call to PEC_Packet_Get.
#ifdef DRIVER_INTERRUPT_COALESCING
#define ADAPTER_DESCRIPTORDONECOUNT  4
#else
#define ADAPTER_DESCRIPTORDONECOUNT  1
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
#define ADAPTER_DESCRIPTORDONETIMEOUT  20
#else
#define ADAPTER_DESCRIPTORDONETIMEOUT  0
#endif

#ifdef DRIVER_DMARESOURCE_BANKS_ENABLE
#define ADAPTER_DMARESOURCE_BANKS_ENABLE
#endif

// DMA buffer allocation alignment
#ifdef DRIVER_DMA_ALIGNMENT_BYTE_COUNT
#define ADAPTER_PCL_DMA_ALIGNMENT_BYTE_COUNT DRIVER_DMA_ALIGNMENT_BYTE_COUNT
#endif

// Define if the hardware does not use large transform records
#ifndef ADAPTER_EIP202_INVALIDATE_NULL_PKT_POINTER
#define ADAPTER_USE_LARGE_TRANSFORM_DISABLE
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
#define ADAPTER_PEC_DEVICE_COUNT       1
#endif

#define ADAPTER_GLOBAL_EIP97_NOF_PES DRIVER_MAX_NOF_PE_TO_USE
#define ADAPTER_GLOBAL_EIP97_RINGMASK ((1 << ADAPTER_PEC_DEVICE_COUNT) - 1)

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
#define ADAPTER_EIP74_INTERRUPTS_ENABLE
#define ADAPTER_EIP74_ERR_IRQ IRQ_DRBG_ERR // DRBG error interrupt
#define ADAPTER_EIP74_RES_IRQ IRQ_DRBG_RES // DRBG reseed early interrupt
#endif

// enable for big-endian CPU
#ifdef DRIVER_SWAPENDIAN
#define ADAPTER_PEC_ARMRING_ENABLE_SWAP
#endif //DRIVER_SWAPENDIAN

#ifdef DRIVER_PEC_BANK_RING
#define ADAPTER_PEC_BANK_RING                   DRIVER_PEC_BANK_RING
#endif

#ifdef DRIVER_PEC_BANK_PACKET
#define ADAPTER_PEC_BANK_PACKET                 DRIVER_PEC_BANK_PACKET
#endif

#ifdef DRIVER_PEC_BANK_TOKEN
#define ADAPTER_PEC_BANK_TOKEN                  DRIVER_PEC_BANK_TOKEN
#endif

// DMA resource bank for SLAD PEC Adapter, used for bouncing SA buffers
#ifdef DRIVER_PEC_BANK_SA
#define ADAPTER_PEC_BANK_SA                     DRIVER_PEC_BANK_SA
#endif


/****************************************************************************
 * Adapter Classification (Global Control) configuration parameters
 */

#define ADAPTER_CS_GLOBAL_DEVICE_NAME       ADAPTER_GLOBAL_DEVICE_NAME

// Maximum number of EIP-207c Classification Engines that can be used
#ifdef DRIVER_CS_MAX_NOF_CE_TO_USE
#define ADAPTER_CS_GLOBAL_MAX_NOF_CE_TO_USE DRIVER_CS_MAX_NOF_CE_TO_USE
#else
#define ADAPTER_CS_GLOBAL_MAX_NOF_CE_TO_USE            1
#endif

// Enable per-transform redirect for all interfaces
#define ADAPTER_CS_GLOBAL_TRANSFORM_REDIRECT_ENABLE 0xffff

// Maximum supported number of flow hash tables
#ifdef DRIVER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE
#define ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE  \
                        DRIVER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE
#else
#define ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE     1
#endif


/****************************************************************************
 * Adapter PCL (Flow Control) configuration parameters
 */

#define ADAPTER_PCL_DEVICE_NAME             ADAPTER_GLOBAL_DEVICE_NAME

// Flow hash table size.
#ifdef DRIVER_PCL_FLOW_HASH_ENTRIES_COUNT
#define ADAPTER_PCL_FLOW_HASH_ENTRIES_COUNT DRIVER_PCL_FLOW_HASH_ENTRIES_COUNT
#else
#define ADAPTER_PCL_FLOW_HASH_ENTRIES_COUNT 1024
#endif

// Number of overflow buckets associated with hash table
#ifdef DRIVER_PCL_FLOW_HASH_OVERFLOW_COUNT
#define ADAPTER_PCL_FLOW_HASH_OVERFLOW_COUNT    \
                DRIVER_PCL_FLOW_HASH_OVERFLOW_COUNT
#endif

#ifdef DRIVER_PCL_FLOW_REMOVE_MAX_TRIES
#define ADAPTER_PCL_FLOW_REMOVE_MAX_TRIES     DRIVER_PCL_FLOW_REMOVE_MAX_TRIES
#endif

#ifdef DRIVER_PCL_FLOW_REMOVE_DELAY_MS
#define ADAPTER_PCL_FLOW_REMOVE_DELAY         DRIVER_PCL_FLOW_REMOVE_DELAY_MS
#endif

#ifdef DRIVER_PCL_MAX_FLUE_DEVICES
#define ADAPTER_PCL_MAX_FLUE_DEVICES          DRIVER_PCL_MAX_FLUE_DEVICES
#endif

// enable for big-endian CPU
#ifdef DRIVER_SWAPENDIAN
#define ADAPTER_PCL_ENABLE_SWAP
#endif //DRIVER_SWAPENDIAN

#define ADAPTER_PCL_ENABLE

#ifdef DRIVER_DMARESOURCE_BANKS_ENABLE
#define ADAPTER_PCL_DMARESOURCE_BANKS_ENABLE
#endif // DRIVER_DMARESOURCE_BANKS_ENABLE

// DMA resource bank for PCL transform records
#ifdef DRIVER_PCL_BANK_TRANSFORM
#define ADAPTER_PCL_BANK_TRANSFORM              DRIVER_PCL_BANK_TRANSFORM
#endif

// DMA resource bank for flow records
#ifdef DRIVER_PCL_BANK_FLOW
#define ADAPTER_PCL_BANK_FLOW                   DRIVER_PCL_BANK_FLOW
#endif

// DMA resource bank for flow table (also for overflow hash buckets)
#ifdef DRIVER_PCL_BANK_FLOWTABLE
#define ADAPTER_PCL_BANK_FLOWTABLE              DRIVER_PCL_BANK_FLOWTABLE
#endif

#ifdef DRIVER_LIST_PCL_OFFSET
#define ADAPTER_PCL_LIST_ID_OFFSET              DRIVER_LIST_PCL_OFFSET
#endif

// Define if the hardware does not (need to) use large transform records
#ifdef ADAPTER_USE_LARGE_TRANSFORM_DISABLE
#define ADAPTER_PCL_USE_LARGE_TRANSFORM_DISABLE
#define ADAPTER_EIP202_USE_LARGE_TRANSFORM_DISABLE
#endif


/****************************************************************************
 * Adapter EIP-202 configuration parameters
 */

// enable debug checks
#ifndef DRIVER_PERFORMANCE
#define ADAPTER_EIP202_STRICT_ARGS
#endif

#ifdef ADAPTER_DRIVER_NAME
#define ADAPTER_EIP202_DRIVER_NAME ADAPTER_DRIVER_NAME
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

#ifdef ADAPTER_PEC_BANK_SA
#define ADAPTER_EIP202_BANK_SA          ADAPTER_PEC_BANK_SA
#endif

// Maximum number of supported EIP-202 devices
// (one device includes CDR Manager and RDR Manager)
#ifdef DRIVER_MAX_NOF_RING_TO_USE
#define ADAPTER_EIP202_DEVICE_COUNT     DRIVER_MAX_NOF_RING_TO_USE
#else
#define ADAPTER_EIP202_DEVICE_COUNT     1
#endif

#define ADAPTER_EIP202_DEVICE_CONF(n) \
    { \
        IRQ_CDR##n,       /* CDR IRQ */                \
        0,                /* CDR IRQ flags */          \
        "EIP202_CDR" #n,  /* CDR Device Name */        \
        IRQ_RDR##n,       /* RDR IRQ */                \
        0,                /* RDR IRQ flags */          \
        "EIP202_RDR" #n,  /* RDR Device Name */        \
    }

#define ADAPTER_EIP202_DEVICES \
 ADAPTER_EIP202_DEVICE_CONF(0), \
 ADAPTER_EIP202_DEVICE_CONF(1), \
 ADAPTER_EIP202_DEVICE_CONF(2), \
 ADAPTER_EIP202_DEVICE_CONF(3), \
 ADAPTER_EIP202_DEVICE_CONF(4), \
 ADAPTER_EIP202_DEVICE_CONF(5), \
 ADAPTER_EIP202_DEVICE_CONF(6), \
 ADAPTER_EIP202_DEVICE_CONF(7), \
 ADAPTER_EIP202_DEVICE_CONF(8), \
 ADAPTER_EIP202_DEVICE_CONF(9), \
 ADAPTER_EIP202_DEVICE_CONF(10), \
 ADAPTER_EIP202_DEVICE_CONF(11), \
 ADAPTER_EIP202_DEVICE_CONF(12), \
 ADAPTER_EIP202_DEVICE_CONF(13)


#if ADAPTER_PEC_DEVICE_COUNT != ADAPTER_EIP202_DEVICE_COUNT
#error "Adapter PEC device configuration is invalid"
#endif

#ifdef ADAPTER_EIP202_INVALIDATE_NULL_PKT_POINTER
// Define if the hardware uses bits in record pointers to distinguish types.
#define ADAPTER_EIP202_USE_POINTER_TYPES
#endif

#ifdef DRIVER_USE_INVALIDATE_COMMANDS
// Define if record invalidate commands are implemented without extended
// command and result descriptors.
#define ADAPTER_EIP202_USE_INVALIDATE_COMMANDS
// Define if the hardware uses bits in record pointers to distinguish types.
#define ADAPTER_EIP202_USE_POINTER_TYPES
#endif


// DMA buffer allocation alignment
#ifdef DRIVER_DMA_ALIGNMENT_BYTE_COUNT
#define ADAPTER_EIP202_DMA_ALIGNMENT_BYTE_COUNT DRIVER_DMA_ALIGNMENT_BYTE_COUNT
#endif

// Allow unaligned DMA buffers.
#ifdef DRIVER_ALLOW_UNALIGNED_DMA
#define ADAPTER_EIP202_ALLOW_UNALIGNED_DMA
#endif

// Enable this parameter to switch off the automatic calculation of the global
// and ring data transfer size and threshold values
//#define ADAPTER_EIP202_AUTO_THRESH_DISABLE

#ifdef ADAPTER_EIP202_AUTO_THRESH_DISABLE
#define ADAPTER_EIP202_CDR_DSCR_FETCH_WORD_COUNT    16
#define ADAPTER_EIP202_CDR_DSCR_THRESH_WORD_COUNT   12
#define ADAPTER_EIP202_RDR_DSCR_FETCH_WORD_COUNT    80
#define ADAPTER_EIP202_RDR_DSCR_THRESH_WORD_COUNT   20
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
     ADAPTER_PEC_MAX_SC_DMARESOURCE_HANDLES + \
     ADAPTER_PCL_FLOW_HASH_ENTRIES_COUNT)


/* end of file cs_adapter.h */
