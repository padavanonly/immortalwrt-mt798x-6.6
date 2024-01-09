/* adapter_sglist.c
 *
 * Packet Engine Control (PEC) Scatter Gather list API implementation.
 *
 */

/*****************************************************************************
* Copyright (c) 2008-2022 by Rambus, Inc. and/or its subsidiaries.
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


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "api_pec_sg.h"         // PEC_SG_* (the API we implement here)


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default Adapter PEC configuration
#include "c_adapter_pec.h"

// DMABuf API
#include "api_dmabuf.h"         // DMABuf_*

// Adapter DMABuf internal API
#include "adapter_dmabuf.h"

// Logging API
#include "log.h"

// Driver Framework DMAResource API
#include "dmares_types.h"       // DMAResource_Handle_t
#include "dmares_mgmt.h"        // DMAResource management functions
#include "dmares_rw.h"          // DMAResource buffer access.
#include "dmares_addr.h"        // DMAResource addr translation functions.
#include "dmares_buf.h"         // DMAResource buffer allocations

// Driver Framework C Run-Time Library API
#include "clib.h"               // memcpy, memset

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // bool, uint32_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER

// our internal definition of a scatter/gather list
// the DMA buffer for SGList store this (variable-length) structure
typedef struct
{
    unsigned int ListCapacity;

    struct
    {
        DMABuf_Handle_t Handle;
        unsigned int ByteCount;  // set by PEC_SGList_Write and PEC_Packet_Get
    } Entries[1];       // variable-size allocated!!!

} PEC_SGList_t;


/*----------------------------------------------------------------------------
 * Adapter_Handle2SGList
 *
 * This function validates that the DMAResource handle is indeed an
 * SGList handle
 * and then returns the pointer to the PEC_SGList_t structure contained in
 * the buffer related to this handle.
 */
static PEC_SGList_t *
Adapter_Handle2SGListPtr(
        DMABuf_Handle_t SGList_Handle)
{
    DMAResource_Handle_t * DMA_Handle =
        Adapter_DMABuf_Handle2DMAResourceHandle(SGList_Handle);
    DMAResource_Record_t * Rec_p;
    if (DMAResource_IsValidHandle(DMA_Handle))
        Rec_p = DMAResource_Handle2RecordPtr(DMA_Handle);
    else
        return NULL;

    // ensure it is an SGList
    if (Rec_p->sg.IsSGList != true)
        return NULL;

    return Adapter_DMAResource_HostAddr(
        Adapter_DMABuf_Handle2DMAResourceHandle(SGList_Handle));

}


/*----------------------------------------------------------------------------
 * Adapter_SGList_CalcAllocSize
 *
 * This function calculates the size of the buffer to be allocated to hold
 * an PEC_SGList_t with a given capacity.
 */
static inline unsigned int
Adapter_SGList_CalcAllocSize(
        const unsigned int ListCapacity)
{
    PEC_SGList_t SG;
    unsigned int S = sizeof(PEC_SGList_t);

    S += (ListCapacity - 1) * sizeof(SG.Entries[1]);

    return S;
}
#endif /* ADAPTER_PEC_ENABLE_SCATTERGATHER */


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
        DMABuf_Handle_t * const SGList_Handle_p)
{
#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
    DMAResource_AddrPair_t HostAddr;
    int dmares;
    DMAResource_Properties_t Props;

    ZEROINIT(Props);

    if (ListCapacity == 0 ||
        SGList_Handle_p == NULL)
    {
        return PEC_ERROR_BAD_PARAMETER;
    }

    // initialize the output parameter
    *SGList_Handle_p = DMABuf_NULLHandle;

    Props.Size      = Adapter_SGList_CalcAllocSize(ListCapacity);
    Props.Alignment = Adapter_DMAResource_Alignment_Get();

    dmares = DMAResource_Alloc(Props, &HostAddr, &SGList_Handle_p->p);
    if (dmares != 0)
        return PEC_ERROR_INTERNAL;

    // set the special flag in the DMA Resource record for SGLists
    {
        DMAResource_Handle_t DMAHandle;
        DMAResource_Record_t * Rec_p;

        DMAHandle = SGList_Handle_p->p;
        Rec_p = DMAResource_Handle2RecordPtr(DMAHandle);

        if (!Rec_p)
        {
            // corner case - avoid memory leak
            DMAResource_Release(Adapter_DMABuf_Handle2DMAResourceHandle(
                                    *SGList_Handle_p));
            *SGList_Handle_p = DMABuf_NULLHandle;

            return PEC_ERROR_INTERNAL;
        }

        Rec_p->sg.IsSGList = true;
    }

    // initialize the SGList
    {
        PEC_SGList_t * const p = HostAddr.Address_p;
        memset(p, 0, Props.Size);
        p->ListCapacity = ListCapacity;
    }

    return PEC_STATUS_OK;
#else
    IDENTIFIER_NOT_USED(ListCapacity);
    IDENTIFIER_NOT_USED(SGList_Handle_p);

    return PEC_ERROR_NOT_IMPLEMENTED;
#endif /* ADAPTER_PEC_ENABLE_SCATTERGATHER */
}


/*----------------------------------------------------------------------------
 * PEC_SGList_Destroy
 *
 * This function must be used to destroy a SGList that was previously created
 * with PEC_SGList_Create. The potentially referred fragments in the list are
 * not freed by the implementation!
 *
 * SGList_Handle (input)
 *     The handle to the SGList as returned by PEC_SGList_Create.
 *
 * DMABuf_Release may be called instead of this function.
 */
PEC_Status_t
PEC_SGList_Destroy(
        DMABuf_Handle_t SGList_Handle)
{
#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
    int dmares;

    dmares = DMAResource_Release(
        Adapter_DMABuf_Handle2DMAResourceHandle(SGList_Handle));

    if (dmares == 0)
        return PEC_STATUS_OK;

    return PEC_ERROR_BAD_HANDLE;
#else
    IDENTIFIER_NOT_USED(SGList_Handle.p);

    return PEC_ERROR_NOT_IMPLEMENTED;
#endif /* ADAPTER_PEC_ENABLE_SCATTERGATHER */
}


/*----------------------------------------------------------------------------
 * PEC_SGList_Write
 */
PEC_Status_t
PEC_SGList_Write(
        DMABuf_Handle_t SGList_Handle,
        const unsigned int Index,
        DMABuf_Handle_t FragmentHandle,
        const unsigned int FragmentByteCount)
{
#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
    PEC_SGList_t * SGList_p;

    SGList_p = Adapter_Handle2SGListPtr(SGList_Handle);
    if (SGList_p == NULL)
        return PEC_ERROR_BAD_HANDLE;

    if (Index >= SGList_p->ListCapacity)
        return PEC_ERROR_BAD_PARAMETER;

    SGList_p->Entries[Index].Handle = FragmentHandle;
    SGList_p->Entries[Index].ByteCount = FragmentByteCount;

    return PEC_STATUS_OK;
#else
    IDENTIFIER_NOT_USED(SGList_Handle.p);
    IDENTIFIER_NOT_USED(Index);
    IDENTIFIER_NOT_USED(FragmentHandle.p);
    IDENTIFIER_NOT_USED(FragmentByteCount);

    return PEC_ERROR_NOT_IMPLEMENTED;
#endif /* ADAPTER_PEC_ENABLE_SCATTERGATHER */
}


/*----------------------------------------------------------------------------
 * PEC_SGList_Read
 */
PEC_Status_t
PEC_SGList_Read(
        DMABuf_Handle_t SGList_Handle,
        const unsigned int Index,
        DMABuf_Handle_t * const FragmentHandle_p,
        unsigned int * const FragmentSizeInBytes_p,
        uint8_t ** const FragmentPtr_p)
{
#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
    PEC_SGList_t * SGList_p;

    // initialize the output parameters
    {
        if (FragmentHandle_p)
            *FragmentHandle_p = DMABuf_NULLHandle;

        if (FragmentSizeInBytes_p)
            *FragmentSizeInBytes_p = 0;

        if (FragmentPtr_p)
            *FragmentPtr_p = NULL;
    }

    SGList_p = Adapter_Handle2SGListPtr(SGList_Handle);
    if (SGList_p == NULL)
        return PEC_ERROR_BAD_HANDLE;

    if (Index >= SGList_p->ListCapacity)
        return PEC_ERROR_BAD_PARAMETER;

    // fill in the output parameters
    {
        if (FragmentHandle_p)
            *FragmentHandle_p = SGList_p->Entries[Index].Handle;

        if (FragmentSizeInBytes_p)
            *FragmentSizeInBytes_p = SGList_p->Entries[Index].ByteCount;

        if (FragmentPtr_p)
        {
            // retrieve the host address from the DMA resource record
            DMAResource_Handle_t DMAHandle;
            DMAResource_Record_t * Rec_p;

            DMAHandle = Adapter_DMABuf_Handle2DMAResourceHandle(
                SGList_p->Entries[Index].Handle);
            Rec_p = DMAResource_Handle2RecordPtr(DMAHandle);
            if (Rec_p)
            {
                // it is a valid handle
                *FragmentPtr_p = Adapter_DMAResource_HostAddr(DMAHandle);
            }
        }
    }

    return PEC_STATUS_OK;
#else
    IDENTIFIER_NOT_USED(SGList_Handle.p);
    IDENTIFIER_NOT_USED(Index);
    IDENTIFIER_NOT_USED(FragmentHandle_p);
    IDENTIFIER_NOT_USED(FragmentSizeInBytes_p);
    IDENTIFIER_NOT_USED(FragmentPtr_p);

    return PEC_ERROR_NOT_IMPLEMENTED;
#endif /* ADAPTER_PEC_ENABLE_SCATTERGATHER */
}


/*----------------------------------------------------------------------------
 * PEC_SGList_GetCapacity
 */
PEC_Status_t
PEC_SGList_GetCapacity(
        DMABuf_Handle_t SGList_Handle,
        unsigned int * const ListCapacity_p)
{
#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
    PEC_SGList_t * SGList_p;

    if (ListCapacity_p == NULL)
        return PEC_ERROR_BAD_PARAMETER;

    // initialize the output parameter
    *ListCapacity_p = 0;

    SGList_p = Adapter_Handle2SGListPtr(SGList_Handle);
    if (SGList_p != NULL)
        *ListCapacity_p = SGList_p->ListCapacity;

    return PEC_STATUS_OK;
#else
    IDENTIFIER_NOT_USED(SGList_Handle.p);
    IDENTIFIER_NOT_USED(ListCapacity_p);

    return PEC_ERROR_NOT_IMPLEMENTED;
#endif /* ADAPTER_PEC_ENABLE_SCATTERGATHER */
}



/* end of file adapter_pec_sglist.c */
