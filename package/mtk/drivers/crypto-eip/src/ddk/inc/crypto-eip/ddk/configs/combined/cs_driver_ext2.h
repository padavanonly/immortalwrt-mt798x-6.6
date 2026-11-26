/* cs_driver_ext2.h
 *
 * Top-level Product EIP-197 hardware specific Configuration Settings.
 */

/*****************************************************************************
* Copyright (c) 2012-2022 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_CS_DRIVER_EXT2_H
#define INCLUDE_GUARD_CS_DRIVER_EXT2_H

// Disable usage of the EIP-202 Ring Arbiter
//#define DRIVER_EIP202_RA_DISABLE

// Number of Ring interfaces to use
#define DRIVER_MAX_NOF_RING_TO_USE                     14

// Maximum number of Flow Look-Up Engines (FLUE in EIP-207s) to use
#define DRIVER_PCL_MAX_FLUE_DEVICES                 DRIVER_CS_MAX_NOF_CE_TO_USE

// Maximum number of Flow Hash Tables to use in the Classification (PCL) Driver
#define DRIVER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE      1

// Number of overflow buckets associated with hash table
#define DRIVER_PCL_FLOW_HASH_OVERFLOW_COUNT  DRIVER_PCL_FLOW_HASH_ENTRIES_COUNT

#ifdef DRIVER_DMARESOURCE_BANKS_ENABLE

// Maximum number of flow records the driver can allocate
#define DRIVER_FLOW_RECORD_COUNT            \
                        (DRIVER_PCL_FLOW_HASH_ENTRIES_COUNT + \
                                    DRIVER_PCL_FLOW_HASH_OVERFLOW_COUNT)

// Flow record size is determined by the EIP-207 Firmware API
#define DRIVER_FLOW_RECORD_BYTE_COUNT                  64

// Maximum number of SA or transform records the driver can allocate
#define DRIVER_TRANSFORM_RECORD_COUNT                  DRIVER_PEC_MAX_SAS

// Maximum Transform record size for the static DMA bank
// For the Look-Aside Basic Crypto flow the transform record size is determined
// by the maximum used SA size.
// For all the other flows the transform record size is determined
// by the EIP-207 Firmware API.
// The static DMA bank is used only for the Flow and Transform records
// that can be looked up, e.g. NOT for the Look-Aside Basic Crypto flow.
#define DRIVER_TRANSFORM_RECORD_BYTE_COUNT             512

// Each static bank for DMA resources must have 2 lists plus
// each classification engine requires 1 list
#define DRIVER_LIST_PCL_MAX_NOF_INSTANCES           DRIVER_PCL_MAX_FLUE_DEVICES
#define DRIVER_LIST_MAX_NOF_INSTANCES              \
                                (DRIVER_LIST_HWPAL_MAX_NOF_INSTANCES +  \
                                        DRIVER_LIST_PCL_MAX_NOF_INSTANCES)
#define DRIVER_LIST_PCL_OFFSET             DRIVER_LIST_HWPAL_MAX_NOF_INSTANCES

// Determine maximum DMA bank element size
#define DRIVER_DMA_BANK_ELEMENT_BYTE_COUNT      \
        (DRIVER_FLOW_RECORD_BYTE_COUNT > DRIVER_TRANSFORM_RECORD_BYTE_COUNT ? \
           DRIVER_FLOW_RECORD_BYTE_COUNT : DRIVER_TRANSFORM_RECORD_BYTE_COUNT)

// Determine maximum number of elements in DMA bank
#define DRIVER_DMA_BANK_ELEMENT_COUNT           \
                (DRIVER_FLOW_RECORD_COUNT + DRIVER_TRANSFORM_RECORD_COUNT)

#endif // DRIVER_DMARESOURCE_BANKS_ENABLE

// Driver uses record invalidate commands (with non-extended descriptor format).
//#define DRIVER_USE_INVALIDATE_COMMANDS


#endif /* Include Guard */


/* end of file cs_driver_ext2.h */
