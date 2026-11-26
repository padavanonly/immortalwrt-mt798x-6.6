/* c_adapter_pcl.h
 *
 * Default PCL Adapter configuration.
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

#ifndef INCLUDE_GUARD_C_ADAPTER_PCL_H
#define INCLUDE_GUARD_C_ADAPTER_PCL_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level Adapter configuration
#include "cs_adapter.h"

#ifndef ADAPTER_PCL_DEVICE_NAME
#define ADAPTER_PCL_DEVICE_NAME                         "PCL_Device"
#endif

#ifndef ADAPTER_PCL_BANK_FLOWTABLE
#define ADAPTER_PCL_BANK_FLOWTABLE                      0
#endif

#ifndef ADAPTER_PCL_BANK_TRANSFORM
#define ADAPTER_PCL_BANK_TRANSFORM                      0
#endif

#ifndef ADAPTER_PCL_BANK_FLOW
#define ADAPTER_PCL_BANK_FLOW                           0
#endif

//#define ADAPTER_PCL_ENABLE

// This parameters enables use of the DMA banks
//#define ADAPTER_PCL_DMARESOURCE_BANKS_ENABLE

#ifndef ADAPTER_PCL_ENABLE_FLUE_CACHE
#define ADAPTER_PCL_ENABLE_FLUE_CACHE false
#endif

#ifndef ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE
#define ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE      1
#endif

#ifndef ADAPTER_PCL_FLOW_HASH_ENTRIES_COUNT
#define ADAPTER_PCL_FLOW_HASH_ENTRIES_COUNT             1024
#endif

#if defined(ADAPTER_PCL_DMARESOURCE_BANKS_ENABLE) && \
    !defined(ADAPTER_PCL_FLOW_RECORD_COUNT)
#define ADAPTER_PCL_FLOW_RECORD_COUNT     ADAPTER_PCL_FLOW_HASH_ENTRIES_COUNT
#endif

#if defined(ADAPTER_PCL_DMARESOURCE_BANKS_ENABLE) && \
    !defined(ADAPTER_PCL_FLOW_RECORD_BYTE_COUNT)
#error "ADAPTER_PCL_FLOW_RECORD_BYTE_COUNT is not defined"
#endif

#if defined(ADAPTER_PCL_DMARESOURCE_BANKS_ENABLE) && \
    !defined(ADAPTER_TRANSFORM_RECORD_COUNT)
#error "ADAPTER_TRANSFORM_RECORD_COUNT is not defined"
#endif

#if defined(ADAPTER_PCL_DMARESOURCE_BANKS_ENABLE) && \
    !defined(ADAPTER_TRANSFORM_RECORD_BYTE_COUNT)
#error "ADAPTER_TRANSFORM_RECORD_BYTE_COUNT is not defined"
#endif

// Number of tries to wait until a flow can be ssafely removed.
#ifndef ADAPTER_PCL_FLOW_REMOVE_MAX_TRIES
#define ADAPTER_PCL_FLOW_REMOVE_MAX_TRIES               100
#endif

// Delay between tries to check that a flow can be safely removed.
// Delay is in milliseconds. If it is specified as zero, no delay is inserted.
#ifndef ADAPTER_PCL_FLOW_REMOVE_DELAY
#define ADAPTER_PCL_FLOW_REMOVE_DELAY                   10
#endif

// Number of overflow buckets associated with hash table
// (defaults here to: number of hash table entries / 16)
#ifndef ADAPTER_PCL_FLOW_HASH_OVERFLOW_COUNT
#define ADAPTER_PCL_FLOW_HASH_OVERFLOW_COUNT \
    (ADAPTER_PCL_FLOW_HASH_ENTRIES_COUNT / 16)
#endif

// Default FLUE device name
#ifndef ADAPTER_PCL_FLUE_DEFAULT_DEVICE_NAME
#define ADAPTER_PCL_FLUE_DEFAULT_DEVICE_NAME            "EIP207_FLUE0"
#endif

// Maximum number of FLUE device instances to be supported
#ifndef ADAPTER_PCL_MAX_FLUE_DEVICES
#define ADAPTER_PCL_MAX_FLUE_DEVICES                    1
#endif

// Device numbering: max number of final digits in device instance name
// eg. value of 1 allows 0-9, value of 2 allows 0-99
#ifndef ADAPTER_PCL_MAX_DEVICE_DIGITS
#define ADAPTER_PCL_MAX_DEVICE_DIGITS                   1
#endif

#ifndef ADAPTER_PCL_LIST_ID_OFFSET
#error "ADAPTER_PCL_LIST_ID_OFFSET is not defined"
#endif

// Default DMA buffer allocation alignment is 4 bytes
#ifndef ADAPTER_PCL_DMA_ALIGNMENT_BYTE_COUNT
#define ADAPTER_PCL_DMA_ALIGNMENT_BYTE_COUNT            4
#endif

// EIP-207 flow control device ID, keep undefined if RPM for EIP-207 not used
//#define ADAPTER_PCL_RPM_EIP207_DEVICE_ID                0


#endif /* Include Guard */


/* end of file c_adapter_pcl.h */
