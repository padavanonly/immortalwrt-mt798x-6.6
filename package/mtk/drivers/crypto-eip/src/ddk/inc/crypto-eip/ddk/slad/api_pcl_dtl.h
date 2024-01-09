/* api_pcl_dtl.h
 *
 * Packet Classification API extensions for the Direct Transform Lookup (DTL).
 *
 * The PCL DTL API functions may be used only for those Transform
 * Records which can be looked up directly by the Classification Engine
 * in the Flow Hash Table (without using a Flow Record).
 *
 * A Transform Record for direct lookup must be allocated using
 * the DMABuf API DMABuf_Alloc() function. In order to determine the required
 * size of of the DMA-buffer the SA Builder API function SABuilder_GetSizes()
 * can be used. The allocated DMA-safe buffer subsequently must be filled in
 * with the transform data, for example by using the SA Builder API, namely
 * the SABuilder_BuildSA() function. Finally the allocated and filled in
 * Transform Record must be added to the Flow Hash Table by means of
 * the PCL_DTL_Transform_Add() function.
 *
 * Several different hash values can reference the same transform record.
 * When the PCL_DTL_Transform_Add() function adds a transform record to
 * the Flow Hash Table it also adds a hash value and returns its hash handle.
 * In order to add a new hash value for the same transform record
 * the PCL_DTL_Transform_Add() function can be called again with another hash
 * ID value in the input transform parameters. If successful then this function
 * stores the hash handle in its output parameter for the hash ID added to
 * the Flow Hash Table for the requested transform record. In order to remove
 * the hash value from the the Flow Hash Table the PCL_DTL_Transform_Remove()
 * function must be called using the hash handle obtained in the previous call
 * to the PCL_DTL_Transform_Add() function. If the hash handle is set to NULL
 * then the PCL_DTL_Transform_Remove() function will removed all the hash values
 * associated with the requested transform record from the Flow Hash Table.
 *
 * When a Transform Record for direct lookup must be deleted then, first,
 * the PCL_DTL_Transform_Remove() function must be used. Second, the PEC API
 * PEC_Packet_Put() function must be called for the Transform Record entry
 * invalidation in the Record Cache. Finally, in order to release the dynamic
 * resource storage the DMABuf API DMABuf_Release() function must be used.
 *
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


#ifndef API_PCL_DTL_H_
#define API_PCL_DTL_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"

// Dependency on DMA Buffer Allocation API (api_dmabuf.h)
#include "api_dmabuf.h"         // DMABuf_Handle_t

// Main PCL API
#include "api_pcl.h"            // PCL_Status_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/*----------------------------------------------------------------------------
 * PCL_DTL_TransformParams_t
 *
 * This data structure contains extended transform parameters for the transform
 * record that must be used for direct lookup.
 */
typedef struct
{
    // The 128-bit transform ID computed by the hash function
    // from the selector parameters. This uniquely identifies the hash entry.
    // One or multiple hash ID's (entries) can co-exist for the same transform.
    // This can be calculated by the PCL_Flow_Hash() function.
    uint32_t HashID[4];

    // Reference to a generic transform parameters, set to NULL if not used
    PCL_TransformParams_t * Params_p;

} PCL_DTL_TransformParams_t;


/*----------------------------------------------------------------------------
 * PCL_DTL_Hash_Handle_t
 *
 * This handle is a reference to a hash ID in the Flow Hash Table.
 *
 * The handle is set to NULL when PCL_DTL_Hash_Handle_t handle.p is equal
 * to NULL (or use HashHandle = PCL_DTL_NULLHandle).
 *
 */
typedef struct
{
    void * p;
} PCL_DTL_Hash_Handle_t;


/*----------------------------------------------------------------------------
 * PCL_DTL_NULLHandle
 *
 * This handle can be assigned to a variable of type PCL_DTL_Hash_Handle_t.
 *
 */
extern const PCL_DTL_Hash_Handle_t PCL_DTL_NULLHandle;


/*-----------------------------------------------------------------------------
 * PCL_DTL_Init
 *
 * Initialize the PCL DTL functionality.
 *
 * API use order:
 *     This function must be called before any other PCL DTL API functions
 *     are called.
 *
 * InterfaceId (input)
 *     Identifier of the Classification Engine interface
 *
 * Return (see PCL_Status_t in api_pcl.h):
 *     PCL_STATUS_OK
 *     PCL_STATUS_BUSY
 *     PCL_INVALID_PARAMETER
 *     PCL_ERROR
 */
PCL_Status_t
PCL_DTL_Init(
        const unsigned int InterfaceId);


/*-----------------------------------------------------------------------------
 * PCL_DTL_UnInit
 *
 * Uninitialize the PCL DTL functionality.
 *
 * API use order:
 *     After this function is called none of the PCL DTL API functions
 *     can be called called except the PCL_DTL_Init() function.
 *
 * InterfaceId (input)
 *     Identifier of the Classification Engine interface
 *
 * Return (see PCL_Status_t in api_pcl.h):
 *     PCL_STATUS_OK
 *     PCL_STATUS_BUSY
 *     PCL_INVALID_PARAMETER
 *     PCL_ERROR
 */
PCL_Status_t
PCL_DTL_UnInit(
        const unsigned int InterfaceId);


/*-----------------------------------------------------------------------------
 * PCL_DTL_Transform_Add
 *
 * Create a hash value (ID) for the provided Transform Record from
 * the provided transform parameters and add it to the Flow Hash Table.
 * One transform record may have multiple hash ID's associated with it. This
 * function can be called several times to add different hash ID's for
 * the same Transform Record in the Flow Hash Table.
 *
 * Note:  The same hash ID cannot be used for different Transform Records.
 *
 * Note:  This function will return PCL_INVALID_PARAMETER for the transform
 *        parameters that were already successfully used to add a hash value
 *        to the Flow Hash Table for this record in an previous call
 *        to PCL_DTL_Transform_Add().
 *
 * InterfaceId (input)
 *     Identifier of the Classification Engine interface
 *
 * FlowHashTableId (input)
 *     Flow Hash Table identifier where the hash ID should be added
 *
 * TransformParams (input)
 *     Contents of the hash ID that is to be added.
 *
 * XformDMAHandle (input)
 *     Handle representing the Transform Record as returned by DMABuf_Alloc().
 *     If this function returns with PCL_STATUS_OK, the new hash value
 *     for this Transform Record will be in use.
 *
 * HashHandle_p (output)
 *     Pointer to the memory location when the hash handle will be returned.
 *
 * Return (see PCL_Status_t in api_pcl.h):
 *     PCL_STATUS_OK
 *     PCL_STATUS_BUSY
 *     PCL_INVALID_PARAMETER
 *     PCL_ERROR
 *     PCL_OUT_OF_MEMORY_ERROR
 */
PCL_Status_t
PCL_DTL_Transform_Add(
        const unsigned int InterfaceId,
        const unsigned int FlowHashTableId,
        const PCL_DTL_TransformParams_t * const TransformParams,
        const DMABuf_Handle_t XformDMAHandle,
        PCL_DTL_Hash_Handle_t * const HashHandle_p);


/*-----------------------------------------------------------------------------
 * PCL_DTL_Transform_Remove
 *
 * Remove all the hash ID's associated with the Transform Record,
 * the dynamic storage resource represented by XformDMAHandle remains allocated
 * and it can be reused by another call to PCL_DTL_Transform_Add().
 * The application is responsible for releasing Transform Record buffer
 * via the DMABuf_Release() function.
 *
 * The XformDMAHandle must represent a Transform Record that is currently
 * in the Flow Hash Table (as added by the PCL_DTL_Transform_Add() function).
 *
 * Note:  This function will return PCL_INVALID_PARAMETER for the transform
 *        record that has no more hash ID's in the Flow Hash Table associated
 *        with it.
 *
 * InterfaceId (input)
 *     Identifier of the Classification Engine interface
 *
 * FlowHashTableId (input)
 *     Flow Hash Table identifier where the Transform Record should be removed
 *
 * XformDMAHandle (input)
 *     Handle of the Transform Record to be removed
 *
 * Return (see PCL_Status_t in api_pcl.h):
 *     PCL_STATUS_OK
 *     PCL_STATUS_BUSY
 *     PCL_INVALID_PARAMETER
 *     PCL_ERROR
 */
PCL_Status_t
PCL_DTL_Transform_Remove(
        const unsigned int InterfaceId,
        const unsigned int FlowHashTableId,
        const DMABuf_Handle_t XformDMAHandle);


/*-----------------------------------------------------------------------------
 * PCL_DTL_Hash_Remove
 *
 * Remove the hash ID referenced by the HashHandle from the Flow Hash Table.
 *
 * The HashHandle must reference the hash ID that is currently
 * in the Flow Hash Table (as returned by the PCL_DTL_Transform_Add()
 * function) for the specified Transform Record handle.
 *
 * InterfaceId (input)
 *     Identifier of the Classification Engine interface
 *
 * FlowHashTableId (input)
 *     Flow Hash Table identifier where the Transform Record should be removed
 *
 * HashHandle (input/output)
 *     Handle for the hash ID to be removed
 *
 * Return (see PCL_Status_t in api_pcl.h):
 *     PCL_STATUS_OK
 *     PCL_STATUS_BUSY
 *     PCL_INVALID_PARAMETER
 *     PCL_ERROR
 */
PCL_Status_t
PCL_DTL_Hash_Remove(
        const unsigned int InterfaceId,
        const unsigned int FlowHashTableId,
        PCL_DTL_Hash_Handle_t * const HashHandle_p);


#endif /* API_PCL_DTL_H_ */


/* end of file api_pcl_dtl.h */
