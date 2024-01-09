/* api_pec_sg.h
 *
 * Packet Engine Control API (PEC)
 * Extension for Scatter/Gather (fragmented packet buffers)
 *
 * This API can be used to perform transforms on security protocol packets
 * for a set of security network protocols like IPSec, MACSec, sRTP, SSL,
 * DTLS, etc.
 *
 * Please note that this is a generic API that can be used for many transform
 * engines. A separate document will detail the exact fields and parameters
 * required for the SA and Descriptors.
 */

/*****************************************************************************
* Copyright (c) 2007-2022 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_API_PEC_SG_H
#define INCLUDE_GUARD_API_PEC_SG_H

#include "basic_defs.h"
#include "api_dmabuf.h"         // DMABuf_Handle_t
#include "api_pec.h"            // PEC_Status_t


/*----------------------------------------------------------------------------
 * PEC_SGList_Create
 *
 * This function must be used to create a list that can hold references to
 * packet buffer fragments. The returned handle can be used in PEC_Packet_Put
 * instead of a normal (contiguous) buffers.
 *
 * ListCapacity (input)
 *     The number of scatter and/or gather fragments that this list can hold.
 *
 * SGList_Handle_p (output)
 *     Pointer to the output parameter that will be filled in with the handle
 *     that represents the newly created SGList.
 */
PEC_Status_t
PEC_SGList_Create(
        const unsigned int ListCapacity,
        DMABuf_Handle_t * const SGList_Handle_p);


/*----------------------------------------------------------------------------
 * PEC_SGList_Destroy
 *
 * This function must be used to destroy a SGList that was previously created
 * with PEC_SGList_Create. The potentially referred fragments in the list are
 * not freed by the implementation!
 *
 * SGList_Handle (input)
 *     The handle to the SGList as returned by PEC_SGList_Create.
 */
PEC_Status_t
PEC_SGList_Destroy(
        DMABuf_Handle_t SGList_Handle);


/*----------------------------------------------------------------------------
 * PEC_SGList_Write
 *
 * This function can be used to write a specific entry in the SGList with a
 * packet fragment buffer information (handle, bytes used)
 *
 * SGList_Handle (input)
 *     The handle to the SGList as returned by PEC_SGList_Create.
 *
 * Index (input)
 *     Position in the SGList to write. This value must be in the range from
 *     0..ListCapacity-1.
 *
 * FragmentHandle (input)
 *     Handle for the fragment buffer.
 *
 * FragmentByteCount (input)
 *     Number of bytes used in this fragment. Only used for gather.
 */
PEC_Status_t
PEC_SGList_Write(
        DMABuf_Handle_t SGList_Handle,
        const unsigned int Index,
        DMABuf_Handle_t FragmentHandle,
        const unsigned int FragmentByteCount);


/*----------------------------------------------------------------------------
 * PEC_SGList_Read
 *
 * This function can be used to read one entry in the SGList. The function
 * returns the handle together with the host address and size of the buffer.
 *
 * SGList_Handle (input)
 *     The handle to the SGList as returned by PEC_SGList_Create.
 *
 * Index (input)
 *     Position in the SGList to write. This value must be in the range from
 *     0..ListCapacity-1.
 *
 * FragmentHandle_p (output)
 *     Pointer to the output parameter that will receive the DMABuf handle
 *     stored in this position of the list.
 *     This parameter is optional and may be set to NULL.
 *
 * FragmentSizeInBytes_p (output)
 *     Pointer to the output parameter that will receive the size of the
 *     buffer represented by the FragmentHandle.
 *     This parameter is optional and may be set to NULL.
 *
 * FragmentPtr_p (output)
 *     Pointer to the output parameter (of type uint8_t *) that will receive
 *     the address to the start of th ebuffer represented by FragmentHandle.
 *     This parameter is optional and may be set to NULL.
 */
PEC_Status_t
PEC_SGList_Read(
        DMABuf_Handle_t SGList_Handle,
        const unsigned int Index,
        DMABuf_Handle_t * const FragmentHandle_p,
        unsigned int * const FragmentSizeInBytes_p,
        uint8_t ** const FragmentPtr_p);


/*----------------------------------------------------------------------------
 * PEC_SGList_GetCapacity
 *
 * This helper function can be used to retrieve the capacity of an SGList,
 * so the caller does not have to remember this.
 *
 * SGList_Handle (input)
 *     The handle to the SGList as returned by PEC_SGList_Create.
 *
 * ListCapacity_p (output)
 *     Pointer to the output variable that will receive the capacity of the
 *     SGList, assumingn SGList_Handle is valid.
 */
PEC_Status_t
PEC_SGList_GetCapacity(
        DMABuf_Handle_t SGList_Handle,
        unsigned int * const ListCapacity_p);


#endif /* Include Guard */

/* end of file api_pec_sg.h */
