/* api_dmabuf.h
 *
 * Management of buffers that can be shared between the host and hardware
 * devices that utilize Direct Memory Access (DMA).
 *
 * Issues to take into account for these buffers:
 * - Start address alignment
 * - Cache line sharing for buffer head and tail
 * - Cache coherence
 * - Address translation to device memory view
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

#ifndef INCLUDE_GUARD_API_DMABUF_H
#define INCLUDE_GUARD_API_DMABUF_H

#include "basic_defs.h"


/*----------------------------------------------------------------------------
 * DMABuf_Handle_t
 *
 * This handle is a reference to a DMA buffer. It is returned when a buffer
 * is allocated or registered and it remains valid until the buffer is freed
 * or de-registered.
 *
 * The handle is set to NULL when DMABuf_Handle_t handle.p is equal to NULL.
 *
 */
typedef struct
{
    void * p;
} DMABuf_Handle_t;


/*----------------------------------------------------------------------------
 * DMABuf_HostAddress_t
 *
 * Buffer address that can be used by the host to access the buffer. This
 * address has been put in a data structure to make it type-safe.
 */
typedef struct
{
    void * p;
} DMABuf_HostAddress_t;


/*----------------------------------------------------------------------------
 * DMABuf_Status_t
 *
 * Return values for all the API functions.
 */
typedef enum
{
    DMABUF_STATUS_OK,
    DMABUF_ERROR_BAD_ARGUMENT,
    DMABUF_ERROR_INVALID_HANDLE,
    DMABUF_ERROR_OUT_OF_MEMORY
} DMABuf_Status_t;


/*----------------------------------------------------------------------------
 * DMABuf_Properties_t
 *
 * Buffer properties. When allocating a buffer, these are the requested
 * properties for the buffer. When registering an externally allocated buffer,
 * these properties describe the buffer.
 *
 * For both uses, the data structure should be initialized to all zeros
 * before filling in some or all of the fields. This ensures forward
 * compatibility in case this structure is extended with more fields.
 *
 * Example usage:
 *     DMABuf_Properties_t Props = {0};
 *     Props.fIsCached = true;
 */
typedef struct
{
    uint32_t Size;       // size of the buffer
    uint8_t Alignment;   // buffer start address alignment, for example
                         // 4 for 32bit
    uint8_t Bank;        // can be used to indicate on-chip memory
    bool fCached;        // true = SW needs to do coherency management
} DMABuf_Properties_t;


/*----------------------------------------------------------------------------
 * DMABuf_NULLHandle
 *
 * This handle can be assigned to a variable of type DMABuf_Handle_t.
 *
 */
extern const DMABuf_Handle_t DMABuf_NULLHandle;


/*----------------------------------------------------------------------------
 * DMABuf_Handle_IsSame
 *
 * Check whether provided Handle1 is equal to provided Handle2.
 *
 * Handle1_p
 *      First handle
 *
 * Handle2_p
 *      Second handle
 *
 * Return values
 *      true:  provided handles are equal
 *      false: provided handles are not equal
 *
 */
bool
DMABuf_Handle_IsSame(
        const DMABuf_Handle_t * const Handle1_p,
        const DMABuf_Handle_t * const Handle2_p);


/*----------------------------------------------------------------------------
 * DMABuf_Alloc
 *
 * Allocate a buffer of requested size that can be used for device DMA.
 *
 * RequestedProperties
 *     Requested properties of the buffer that will be allocated, including
 *     the size, start address alignment, etc. See above.
 *
 * Buffer_p (output)
 *     Pointer to the memory location where the address of the buffer will be
 *     written by this function when allocation is successful. This address
 *     can then be used to access the driver on the host in the domain of the
 *     driver.
 *
 * Handle_p (output)
 *     Pointer to the memory location when the handle will be returned.
 *
 * Return Values
 *     DMABUF_STATUS_OK: Success, Handle_p was written.
 *     DMABUF_ERROR_BAD_ARGUMENT
 *     DMABUF_ERROR_OUT_OF_MEMORY: Failed to allocate a buffer or handle.
 */
DMABuf_Status_t
DMABuf_Alloc(
        const DMABuf_Properties_t RequestedProperties,
        DMABuf_HostAddress_t * const Buffer_p,
        DMABuf_Handle_t * const Handle_p);


/*----------------------------------------------------------------------------
 * DMABuf_Register
 *
 * This function must be used to register an "alien" buffer that was allocated
 * somewhere else.
 *
 * ActualProperties (input)
 *     Properties that describe the buffer that is being registered.
 *
 * Buffer_p (input)
 *     Pointer to the buffer. This pointer must be valid to use on the host
 *     in the domain of the driver.
 *
 * Alternative_p (input)
 *     Some allocators return two addresses. This parameter can be used to
 *     pass this second address to the driver. The type is pointer to ensure
 *     it is always large enough to hold a system address, also in LP64
 *     architecture. Set to NULL if not used.
 *
 * AllocatorRef (input)
 *     Number to describe the source of this buffer. The exact numbers
 *     supported is implementation specific. This provides some flexibility
 *     for a specific implementation to support a number of "alien" buffers
 *     from different allocator and properly interpret and use the
 *     Alternative_p parameter when translating the address to the device
 *     memory map. Set to zero when a default allocator is used. The type
 *     of the default allocator is implementation specific,
 *     refer to the DMABuf API Implementation Notes for details.
 *
 * Handle_p (output)
 *     Pointer to the memory location when the handle will be returned.
 *
 * Return Values
 *     DMABUF_STATUS_OK: Success, Handle_p was written.
 *     DMABUF_ERROR_BAD_ARGUMENT
 *     DMABUF_ERROR_OUT_OF_MEMORY: Failed to allocate a handle.
 */
DMABuf_Status_t
DMABuf_Register(
        const DMABuf_Properties_t ActualProperties,
        void * Buffer_p,
        void * Alternative_p,
        const char AllocatorRef,
        DMABuf_Handle_t * const Handle_p);


/*----------------------------------------------------------------------------
 * DMABuf_Release
 *
 * This function will close the handle that was returned by DMABuf_Alloc or
 * DMABuf_Register, meaning it must not be used anymore.
 * If the buffer was allocated through DMABuf_Alloc, this function will also
 * free the buffer, meaning it must not be accessed anymore.
 *
 * Handle (input)
 *     The handle that may be released.
 *
 * Return Values
 *     DMABUF_STATUS_OK
 *     DMABUF_ERROR_INVALID_HANDLE
 */
DMABuf_Status_t
DMABuf_Release(
        DMABuf_Handle_t Handle);


#endif /* Include Guard */

/* end of file api_dmabuf.h */

