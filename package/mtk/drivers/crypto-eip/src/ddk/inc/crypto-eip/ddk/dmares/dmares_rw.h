/* dmares_rw.h
 *
 * Driver Framework, DMAResource API, Read/Write and Pre/Post-DMA functions
 *
 * The document "Driver Framework Porting Guide" contains the detailed
 * specification of this API. The information contained in this header file
 * is for reference only.
 */

/*****************************************************************************
* Copyright (c) 2007-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_DMARES_RW_H
#define INCLUDE_GUARD_DMARES_RW_H

#include "basic_defs.h"         // bool, uint32_t, inline
#include "dmares_types.h"       // DMAResource_Handle_t


/*****************************************************************************
 * DMA Resource API
 *
 * This API is related to management of memory buffers that can be accessed by
 * the host as well as a device, using Direct Memory Access (DMA). A driver
 * must typically support many of these shared resources. This API helps with
 * the administration of these resources.
 *
 * This API maintains administration records that the caller can read and
 * write directly. A handle is also provided, to abstract the record.
 * The handle cannot be used to directly access the resource and is therefore
 * considered safe to pass around in the system, even to applications.
 *
 * Another important aspect of this API is to provide a point where resources
 * can be handed over between the host and the device. The driver library or
 * adapter must call the PreDMA and PostDMA functions to indicate the hand over
 * of access right between the host and the device for an entire resource or
 * part thereof. The implementation can use these calls to manage the
 * data coherence for the resource, for example in a cached system.
 *
 * Dynamic DMA resources such as DMA-safe buffers covered by this API
 * are different from static Device resources (see the Device API)
 * which represent device-internal resources with possible read and write
 * side-effects.
 *
 * Memory buffers are different from HWPAL_Device resources, which represent
 * static device-internal resources with possible read and write side-effects.
 *
 * On the fly endianness swapping for words is supported for DMA Resources by
 * means of the Read32 and Write32 functions.
 *
 * Note: If multiple devices with a different memory view need to use the same
 * DMA resource then the caller should consider separate records for each
 * device (for the same buffer).
 */

/*----------------------------------------------------------------------------
 * DMAResource_Read32
 *
 * This function can be used to read one 32bit word from the DMA Resource
 * buffer.
 * If required (decided by DMAResource_Record_t.device.fSwapEndianness),
 * on the fly endianness conversion of the value read will be performed before
 * it is returned to the caller.
 *
 * Handle (input)
 *     Handle for the DMA Resource to access.
 *
 * WordOffset (input)
 *     Offset in 32bit words, from the start of the DMA Resource to read from.
 *
 * Return Value
 *     The value read.
 *
 * When the Handle and WordOffset parameters are not valid, the implementation
 * will return an unspecified value.
 */
uint32_t
DMAResource_Read32(
        const DMAResource_Handle_t Handle,
        const unsigned int WordOffset);


/*----------------------------------------------------------------------------
 * DMAResource_Write32
 *
 * This function can be used to write one 32bit word to the DMA Resource.
 * If required (decided by DMAResource_Record_t.device.fSwapEndianness),
 * on the fly endianness conversion of the value to be written will be
 * performed.
 *
 * Handle (input)
 *     Handle for the DMA Resource to access.
 *
 * WordOffset (input)
 *     Offset in 32bit words, from the start of the DMA Resource to write to.
 *
 * Value (input)
 *     The 32bit value to write.
 *
 * Return Value
 *     None
 *
 * The write can only be successful when the Handle and WordOffset
 * parameters are valid.
 */
void
DMAResource_Write32(
        const DMAResource_Handle_t Handle,
        const unsigned int WordOffset,
        const uint32_t Value);


/*----------------------------------------------------------------------------
 * DMAResource_Read32Array
 *
 * This function perform the same task as DMAResource_Read32 for an array of
 * consecutive 32bit words.
 *
 * See DMAResource_Read32 for a more detailed description.
 *
 * Handle (input)
 *     Handle for the DMA Resource to access.
 *
 * StartWordOffset (input)
 *     Offset in 32bit words, from the start of the DMA Resource to start
 *     reading from. The word offset in incremented for every word.
 *
 * WordCount (input)
 *     The number of 32bit words to transfer.
 *
 * Values_p (input)
 *     Memory location to write the retrieved values to.
 *     Note the ability to let Values_p point inside the DMAResource that is
 *     being read from, allowing for in-place endianness conversion.
 *
 * Return Value
 *     None.
 *
 * The read can only be successful when the Handle and StartWordOffset
 * parameters are valid.
 */
void
DMAResource_Read32Array(
        const DMAResource_Handle_t Handle,
        const unsigned int StartWordOffset,
        const unsigned int WordCount,
        uint32_t * Values_p);


/*----------------------------------------------------------------------------
 * DMAResource_Write32Array
 *
 * This function perform the same task as DMAResource_Write32 for an array of
 * consecutive 32bit words.
 *
 * See DMAResource_Write32 for a more detailed description.
 *
 * Handle (input)
 *     Handle for the DMA Resource to access.
 *
 * StartWordOffset (input)
 *     Offset in 32bit words, from the start of the DMA Resource to start
 *     writing from. The word offset in incremented for every word.
 *
 * WordCount (input)
 *     The number of 32bit words to transfer.
 *
 * Values_p (input)
 *     Pointer to the memory where the values to be written are located.
 *     Note the ability to let Values_p point inside the DMAResource that is
 *     being written to, allowing for in-place endianness conversion.
 *
 * Return Value
 *     None.
 *
 * The write can only be successful when the Handle and StartWordOffset
 * parameters are valid.
 */
void
DMAResource_Write32Array(
        const DMAResource_Handle_t Handle,
        const unsigned int StartWordOffset,
        const unsigned int WordCount,
        const uint32_t * Values_p);


/*----------------------------------------------------------------------------
 * DMAResource_PreDMA
 *
 * This function must be called when the host has finished accessing the
 * DMA resource and the device (using its DMA) is the next to access it.
 * It is possible to hand off the entire DMA resource, or only a selected part
 * of it by describing the part with a start offset and count.
 *
 * Handle (input)
 *     Handle for the DMA Resource to (partially) hand off.
 *
 * ByteOffset (input)
 *     Start offset within the DMA resource for the selected part to hand off
 *     to the device.
 *
 * ByteCount (input)
 *     Number of bytes from ByteOffset for the part of the DMA Resource to
 *     hand off to the device.
 *     Set to zero to hand off the entire DMA Resource.
 *
 * The driver framework implementation must use this call to ensure the data
 * coherence of the DMA Resource.
 */
void
DMAResource_PreDMA(
        const DMAResource_Handle_t Handle,
        const unsigned int ByteOffset,
        const unsigned int ByteCount);


/*----------------------------------------------------------------------------
 * DMAResource_PostDMA
 *
 * This function must be called when the device has finished accessing the
 * DMA resource and the host can reclaim ownership and access it.
 * It is possible to reclaim ownership for the entire DMA resource, or only a
 * selected part of it by describing the part with a start offset and count.
 *
 * Handle (input)
 *     Handle for the DMA resource to (partially) hand off.
 *
 * ByteOffset (input)
 *     Start offset within the DMA resource for the selected part to reclaim.
 *
 * ByteCount (input)
 *     Number of bytes from ByteOffset for the part of the DMA Resource to
 *     reclaim.
 *     Set to zero to reclaim the entire DMA resource.
 *
 * The driver framework implementation must use this call to ensure the data
 * coherence of the DMA resource.
 */
void
DMAResource_PostDMA(
        const DMAResource_Handle_t Handle,
        const unsigned int ByteOffset,
        const unsigned int ByteCount);


#endif /* Include Guard */

/* end of file dmares_rw.h */
