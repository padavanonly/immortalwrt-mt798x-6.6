/* device_rw.h
 *
 * Driver Framework, Device API, Read/Write functions
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

#ifndef INCLUDE_GUARD_DEVICE_RW_H
#define INCLUDE_GUARD_DEVICE_RW_H

// Driver Framework Basic Defs API
#include "basic_defs.h"     // uint32_t, inline

// Driver Framework Device API
#include "device_types.h"   // Device_Handle_t


/*----------------------------------------------------------------------------
 * Device_Read32
 *
 * This function can be used to read one static 32bit resource inside a device
 * (typically a register or memory location). Since reading registers can have
 * side effects, the implementation must guarantee that the resource will be
 * read only once and no neighboring resources will be accessed.
 *
 * If required (decided based on internal configuration), on the fly endianness
 * swapping of the value read will be performed before it is returned to the
 * caller.
 *
 * Device (input)
 *     Handle for the device instance as returned by Device_Find.
 *
 * ByteOffset (input)
 *     The byte offset within the device for the resource to read.
 *
 * Return Value
 *     The value read.
 *
 * When the Device or Offset parameters are invalid, or a read error occurs,
 * the implementation will return an unspecified value.
 */
uint32_t
Device_Read32(
        const Device_Handle_t Device,
        const unsigned int ByteOffset);


/*----------------------------------------------------------------------------
 * Device_Read32Check
 *
 * This function can be used to read one static 32-bit resource inside a device
 * (typically a register or memory location). Since reading registers can have
 * side effects, the implementation must guarantee that the resource will be
 * read only once and no neighboring resources will be accessed.
 *
 * If required (decided based on internal configuration), on the fly endianess
 * swapping of the value read will be performed before it is returned to the
 * caller.
 *
 * Device (input)
 *     Handle for the device instance as returned by Device_Find.
 *
 * ByteOffset (input)
 *     The byte offset within the device for the resource to read.
 *
 * Value_p (output)
 *     The 32-bit value read from the register.
 *
 * Return Value
 *     0 for success, non-zero value for read error.
 *
 * When the Device or Offset parameters are invalid, or a read error occurs,
 * the implementation will return an unspecified value.
 */
int
Device_Read32Check(
        const Device_Handle_t Device,
        const unsigned int ByteOffset,
        uint32_t * const Value_p);


/*----------------------------------------------------------------------------
 * Device_Write32
 *
 * This function can be used to write one static 32bit resource inside a
 * device (typically a register or memory location). Since writing registers
 * can have side effects, the implementation must guarantee that the resource
 * will be written exactly once and no neighboring resources will be
 * accessed.
 *
 * If required (decided based on internal configuration), on the fly endianness
 * swapping of the value to be written will be performed.
 *
 * Device (input)
 *     Handle for the device instance as returned by Device_Find.
 *
 * ByteOffset (input)
 *     The byte offset within the device for the resource to write.
 *
 * Value (input)
 *     The 32bit value to write.
 *
 * Return Value
 *     0 for success, non-zero value for write error.
 *
 * The write can only be successful when the Device and ByteOffset parameters
 * are valid.
 */
int
Device_Write32(
        const Device_Handle_t Device,
        const unsigned int ByteOffset,
        const uint32_t Value);


/*----------------------------------------------------------------------------
 * Device_Read32Array
 *
 * This function perform the same task as Device_Read32 for an array of
 * consecutive 32bit words, allowing the implementation to use a more optimal
 * burst-read (if available).
 *
 * See Device_Read32 for pre-conditions and a more detailed description.
 *
 * Device (input)
 *     Handle for the device instance as returned by Device_Find.
 *
 * StartByteOffset (input)
 *     Byte offset of the first resource to read.
 *     This value is incremented by 4 for each following resource.
 *
 * MemoryDst_p (output)
 *     Pointer to the memory where the retrieved words will be stored.
 *
 * Count (input)
 *     The number of 32bit words to transfer.
 *
 * Return Value
 *     0 for success, non-zero value for read error.
 *
 * When the Device or Offset parameters are invalid, or a read error occurs,
 * the implementation will return unspecified array values.
 */
int
Device_Read32Array(
        const Device_Handle_t Device,
        const unsigned int StartByteOffset,
        uint32_t * MemoryDst_p,
        const int Count);


/*----------------------------------------------------------------------------
 * Device_Write32Array
 *
 * This function perform the same task as Device_Write32 for an array of
 * consecutive 32bit words, allowing the implementation to use a more optimal
 * burst-write (if available).
 *
 * See Device_Write32 for pre-conditions and a more detailed description.
 *
 * Device (input)
 *     Handle for the device instance as returned by Device_Find.
 *
 * StartByteOffset (input)
 *     Byte offset of the first resource to write.
 *     This value is incremented by 4 for each following resource.
 *
 * MemorySrc_p (input)
 *     Pointer to the memory where the values to be written are located.
 *
 * Count (input)
 *     The number of 32bit words to transfer.
 *
 * Return Value
 *     0 for success, non-zero value for write error.
 */
int
Device_Write32Array(
        const Device_Handle_t Device,
        const unsigned int StartByteOffset,
        const uint32_t * MemorySrc_p,
        const int Count);


/* The defines below specify the possible return codes of the implementations
   of these API functions */

/* Device error return code for invalid parameter */
#define DEVICE_RW_PARAM_ERROR 0x00000010

/* Device error return code for a not initialized situation */
#define DEVICE_NOT_INITIALIZED_ERROR 0xEEEEEEEE

#endif /* Include Guard */

/* end of file device_rw.h */
