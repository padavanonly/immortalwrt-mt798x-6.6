/* adapter_pcl_dtl.c
 *
 * Packet Classification (PCL) DTL API implementation.
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
******************************************************************************/

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// PCL API
#include "api_pcl.h"                // PCL_*

// PCL DTL API
#include "api_pcl_dtl.h"            // PCL_DTL_*


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default Adapter configuration
#include "c_adapter_pcl.h"

// DMABuf API
#include "api_dmabuf.h"         // DMABuf_*

// Adapter DMABuf internal API
#include "adapter_dmabuf.h"

// Convert address to pair of 32-bit words.
#include "adapter_addrpair.h"

// Buffer allocation (non-DMA) API
#include "adapter_alloc.h"

// Adapter PCL Internal API
#include "adapter_pcl.h"

// Adapter Locking internal API
#include "adapter_lock.h"       // Adapter_Lock_*

// EIP-207 Driver Library Flow Control Generic API
#include "eip207_flow_generic.h"

// EIP-207 Driver Library Flow Control DTL API
#include "eip207_flow_dtl.h"

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t
#include "device_mgmt.h"        // Device_find

// Driver Framework DMAResource API
#include "dmares_types.h"       // DMAResource_Handle_t
#include "dmares_mgmt.h"        // DMAResource management functions
#include "dmares_rw.h"          // DMAResource buffer access.
#include "dmares_addr.h"        // DMAResource addr translation functions.
#include "dmares_buf.h"         // DMAResource buffer allocations

// List API
#include "list.h"               // List_*

// Runtime Power Management Device Macros API
#include "rpm_device_macros.h"  // RPM_*

// Logging API
#include "log.h"

// Driver Framework C Run-Time Library API
#include "clib.h"               // memcpy, memset

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // bool, uint32_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// this implementation requires DMA resource banks
#ifndef ADAPTER_PCL_DMARESOURCE_BANKS_ENABLE
#error "Adapter DTL: ADAPTER_PCL_DMARESOURCE_BANKS_ENABLE not defined"
#endif

// Bit to indicate whether transformr record is large.
#define ADAPTER_PCL_TR_ISLARGE              BIT_4


/*----------------------------------------------------------------------------
 * Global constants
 */


/*----------------------------------------------------------------------------
 * PCL_DTL_NULLHandle
 *
 */
const PCL_DTL_Hash_Handle_t PCL_DTL_NULLHandle = { NULL };


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * Local prototypes
 */


/*-----------------------------------------------------------------------------
 * PCL API functions implementation
 *
 */

/*-----------------------------------------------------------------------------
 * PCL_Flow_Remove
 */
PCL_Status_t
PCL_Flow_Remove(
        const unsigned int InterfaceId,
        const unsigned int FlowHashTableId,
        const PCL_FlowHandle_t FlowHandle)
{
    PCL_Status_t PCL_Rc = PCL_STATUS_OK;
    EIP207_Flow_Error_t EIP207_Rc;
    EIP207_Flow_IOArea_t * ioarea_p;
    List_Element_t * Element_p = (List_Element_t*)FlowHandle;
    EIP207_Flow_FR_Dscr_t * FlowDescriptor_p;
    AdapterPCL_Device_Instance_Data_t * Dev_p;

    LOG_INFO("\n\t PCL_Flow_Remove \n");

    // validate input parameters
    if (InterfaceId >= ADAPTER_PCL_MAX_FLUE_DEVICES ||
        FlowHashTableId >= ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE ||
        Element_p == NULL)
    {
        return PCL_INVALID_PARAMETER;
    }

    FlowDescriptor_p = (EIP207_Flow_FR_Dscr_t *)Element_p->DataObject_p;

    if (FlowDescriptor_p == NULL)
    {
        LOG_CRIT("PCL_Flow_Remove: failed, invalid flow handle\n");
        return PCL_INVALID_PARAMETER;
    }

    // get interface ioarea
    Dev_p = AdapterPCL_Device_Get(InterfaceId);
    ioarea_p = Dev_p->EIP207_IOArea_p;

    if (Adapter_Lock_CS_Get(&Dev_p->AdapterPCL_DevCS) == Adapter_Lock_NULL)
    {
        LOG_CRIT("PCL_Flow_Add: no device lock, not initialized?\n");
        return PCL_ERROR;
    }

    if (!Adapter_Lock_CS_Enter(&Dev_p->AdapterPCL_DevCS))
        return PCL_STATUS_BUSY;

    if (RPM_DEVICE_IO_START_MACRO(ADAPTER_PCL_RPM_EIP207_DEVICE_ID,
                                  RPM_FLAG_SYNC) != RPM_SUCCESS)
    {
        Adapter_Lock_CS_Leave(&Dev_p->AdapterPCL_DevCS);
        return PCL_ERROR;
    }

    LOG_INFO("\n\t\t EIP207_Flow_DTL_FR_Remove \n");

    EIP207_Rc = EIP207_Flow_DTL_FR_Remove(ioarea_p,
                                          FlowHashTableId,
                                          FlowDescriptor_p);
    if (EIP207_Rc != EIP207_FLOW_NO_ERROR)
    {
        LOG_CRIT("PCL_Flow_Remove: failed to remove FR, err=%d\n", EIP207_Rc);
        PCL_Rc = PCL_ERROR;
    }

    (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_PCL_RPM_EIP207_DEVICE_ID,
                                   RPM_FLAG_ASYNC);

    Adapter_Lock_CS_Leave(&Dev_p->AdapterPCL_DevCS);

    return PCL_Rc;
}


/*-----------------------------------------------------------------------------
 * PCL_Transform_Register
 */
PCL_Status_t
PCL_Transform_Register(
        const DMABuf_Handle_t TransformHandle)
{
    unsigned int TR_WordCount;
    DMAResource_Handle_t DMAHandle =
                Adapter_DMABuf_Handle2DMAResourceHandle(TransformHandle);
    DMAResource_Record_t * const Rec_p =
                                DMAResource_Handle2RecordPtr(DMAHandle);

    LOG_INFO("\n\t PCL_Transform_Register \n");

    // validate parameter
    if (Rec_p == NULL)
        return PCL_INVALID_PARAMETER;

    {
#ifndef ADAPTER_PCL_USE_LARGE_TRANSFORM_DISABLE
        uint32_t *TR_p = (uint32_t*)Adapter_DMAResource_HostAddr(DMAHandle);

        Rec_p->fIsLargeTransform = false;

        // Check whether the transform record is large.
        // Register that in the DMA resource record.
        if ((*TR_p & ADAPTER_PCL_TR_ISLARGE) != 0)
        {
            Rec_p->fIsLargeTransform = true;

            TR_WordCount = EIP207_Flow_DTL_TR_Large_WordCount_Get();
            *TR_p = *TR_p & ~ADAPTER_PCL_TR_ISLARGE;
            // Clear that bit in the transform record.
        }
        else
#endif // !ADAPTER_PCL_USE_LARGE_TRANSFORM_DISABLE
        {
            TR_WordCount = EIP207_Flow_TR_WordCount_Get();
        }
    }

#ifdef ADAPTER_PEC_ARMRING_ENABLE_SWAP
    DMAResource_SwapEndianness_Set(DMAHandle, true);
#endif

#ifdef ADAPTER_PCL_DMARESOURCE_BANKS_ENABLE
    if (Rec_p->Props.Bank != ADAPTER_PCL_BANK_TRANSFORM)
    {
        LOG_CRIT("PCL Adapter: Invalid bank for Transform\n");
        return PCL_ERROR;
    }
#endif

    if (Rec_p->Props.Size < (sizeof(uint32_t) * TR_WordCount))
    {
        LOG_CRIT("PCL_Transform_Register: supplied buffer too small\n");
        return PCL_ERROR;
    }

    DMAResource_Write32Array(
            DMAHandle,
            0,
            TR_WordCount,
            Adapter_DMAResource_HostAddr(DMAHandle));

    DMAResource_PreDMA(DMAHandle, 0, sizeof(uint32_t) * TR_WordCount);

    return PCL_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * PCL_Transform_UnRegister
 */
PCL_Status_t
PCL_Transform_UnRegister(
        const DMABuf_Handle_t TransformHandle)
{
    IDENTIFIER_NOT_USED(TransformHandle.p);

    LOG_INFO("\n\t PCL_Transform_UnRegister \n");

    // Kept for backwards-compatibility, nothing to do here

    return PCL_STATUS_OK;
}

/*-----------------------------------------------------------------------------
 * PCL_Transform_Get_ReadOnly
 */
PCL_Status_t
PCL_Transform_Get_ReadOnly(
        const DMABuf_Handle_t TransformHandle,
        PCL_TransformParams_t * const TransformParams_p)
{
    EIP207_Flow_TR_Dscr_t TransformDescriptor;
    EIP207_Flow_TR_OutputData_t TransformData;
    EIP207_Flow_Error_t res;
    EIP207_Flow_IOArea_t * ioarea_p;
    AdapterPCL_Device_Instance_Data_t * Dev_p;
    DMAResource_Record_t * Rec_p = NULL;

    LOG_INFO("\n\t PCL_Transform_Get_ReadOnly \n");

    if( AdapterPCL_DMABuf_To_TRDscr(
            TransformHandle, &TransformDescriptor, &Rec_p) != PCL_STATUS_OK)
        return PCL_ERROR;

    // get interface ioarea
    Dev_p = AdapterPCL_Device_Get(0);
    ioarea_p = Dev_p->EIP207_IOArea_p;
    if (ioarea_p == NULL)
    {
        LOG_CRIT("PCL_Transform_Get_ReadOnly: failed, not initialized\n");
        return PCL_ERROR;
    }

    {
#ifndef ADAPTER_PCL_USE_LARGE_TRANSFORM_DISABLE
        if (Rec_p->fIsLargeTransform)
        {
            res = EIP207_Flow_DTL_TR_Large_Read(
                ioarea_p,
                0,
                &TransformDescriptor,
                &TransformData);
        }
        else
#else
        IDENTIFIER_NOT_USED(Rec_p);
#endif // !ADAPTER_PCL_USE_LARGE_TRANSFORM_DISABLE
        {
            res = EIP207_Flow_TR_Read(
                ioarea_p,
                0,
                &TransformDescriptor,
                &TransformData);
        }
        if (res != EIP207_FLOW_NO_ERROR)
        {
            LOG_CRIT("PCL_Transform_Get_ReadOnly: "
                    "Failed to remove transform record\n");
            return PCL_ERROR;
        }
    }

    TransformParams_p->SequenceNumber   = TransformData.SequenceNumber;
    TransformParams_p->PacketsCounterLo = TransformData.PacketsCounter;
    TransformParams_p->PacketsCounterHi = 0;
    TransformParams_p->OctetsCounterLo  = TransformData.OctetsCounterLo;
    TransformParams_p->OctetsCounterHi  = TransformData.OctetsCounterHi;

    return PCL_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * PCL DTL API functions implementation
 *
 */


/*-----------------------------------------------------------------------------
 * PCL_DTL_Transform_Add
 */
PCL_Status_t
PCL_DTL_Transform_Add(
        const unsigned int InterfaceId,
        const unsigned int FlowHashTableId,
        const PCL_DTL_TransformParams_t * const TransformParams,
        const DMABuf_Handle_t XformDMAHandle,
        PCL_DTL_Hash_Handle_t * const HashHandle_p)
{
    PCL_Status_t PCL_Rc = PCL_ERROR;
    PCL_Status_t PCL_Rc2;
    EIP207_Flow_Error_t EIP207_Rc;
    EIP207_Flow_IOArea_t * ioarea_p;
    EIP207_Flow_TR_Dscr_t * TR_Dscr_p;
    EIP207_Flow_TR_InputData_t TR_inputdata;
    AdapterPCL_Device_Instance_Data_t * Dev_p;
    void * HashList_p = NULL;
    List_Status_t List_Rc;
    DMAResource_Record_t * Rec_p = NULL;
    List_Element_t * Element_p = NULL;

    LOG_INFO("\n\t %s \n", __func__);

    // Validate input parameters
    if (InterfaceId >= ADAPTER_PCL_MAX_FLUE_DEVICES ||
        FlowHashTableId >= ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE)
        return PCL_INVALID_PARAMETER;

    // Get interface ioarea
    Dev_p = AdapterPCL_Device_Get(InterfaceId);
    ioarea_p = Dev_p->EIP207_IOArea_p;

    if (Dev_p->List_p == NULL)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return PCL_ERROR;
    }

    if (Adapter_Lock_CS_Get(&Dev_p->AdapterPCL_DevCS) == Adapter_Lock_NULL)
    {
        LOG_CRIT("%s: failed, no device lock, not initialized?\n", __func__);
        return PCL_ERROR;
    }

    if (!Adapter_Lock_CS_Enter(&Dev_p->AdapterPCL_DevCS))
        return PCL_STATUS_BUSY;

    // Try to add new hash value to Flow Hash Table
    {
        unsigned int ListID;

        PCL_Rc2 = AdapterPCL_ListID_Get(InterfaceId, &ListID);
        if (PCL_Rc2 != PCL_STATUS_OK)
        {
            LOG_CRIT("%s: failed to get free list\n", __func__);
            goto error_exit;
        }

        List_Rc = List_RemoveFromTail(ListID, NULL, &Element_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT(
                "%s: failed to get free element from list\n", __func__);
            goto error_exit;
        }

        // Note: Element_p and TR_Dscr_p must be valid by implementation!
        TR_Dscr_p = (EIP207_Flow_TR_Dscr_t*)Element_p->DataObject_p;

        // Set list element with the transform record descriptor data
        PCL_Rc2 = AdapterPCL_DMABuf_To_TRDscr(XformDMAHandle, TR_Dscr_p, &Rec_p);
        if (PCL_Rc2 != PCL_STATUS_OK)
        {
            LOG_CRIT("%s: failed, invalid transform\n", __func__);
            PCL_Rc = PCL_INVALID_PARAMETER;
            goto error_exit;
        }

        // Convert transform parameters into EIP-207 transform parameters
        ZEROINIT(TR_inputdata);
        TR_inputdata.HashID.Word32[0] = TransformParams->HashID[0];
        TR_inputdata.HashID.Word32[1] = TransformParams->HashID[1];
        TR_inputdata.HashID.Word32[2] = TransformParams->HashID[2];
        TR_inputdata.HashID.Word32[3] = TransformParams->HashID[3];

#ifndef ADAPTER_PCL_USE_LARGE_TRANSFORM_DISABLE
        TR_inputdata.fLarge = Rec_p->fIsLargeTransform;
#else
        TR_inputdata.fLarge = false;
#endif

        LOG_INFO("\n\t\t EIP207_Flow_DTL_TR_Add \n");

        // Add new hash value for this transform record to Flow Hash Table
        EIP207_Rc = EIP207_Flow_DTL_TR_Add(ioarea_p,
                                           FlowHashTableId,
                                           TR_Dscr_p,
                                           &TR_inputdata);
        if (EIP207_Rc == EIP207_FLOW_OUT_OF_MEMORY_ERROR)
        {
            LOG_CRIT("%s: failed to install transform, "
                     "out of memory\n", __func__);
            PCL_Rc = PCL_OUT_OF_MEMORY_ERROR;
        }
        else if (EIP207_Rc == EIP207_FLOW_NO_ERROR)
        {
            PCL_Rc = PCL_STATUS_OK;
        }
        else
        {
            LOG_CRIT("%s: failed to install transform, "
                     "EIP207_Flow_DTL_TR_Add() error %d\n",
                     __func__,
                     EIP207_Rc);
        }

        if (PCL_Rc != PCL_STATUS_OK)
        {
            LOG_CRIT("%s: failed to add hash value for transform record\n",
                     __func__);

            // Return not added element to free list
            List_Rc = List_AddToHead(ListID, NULL, Element_p);
            if (List_Rc != LIST_STATUS_OK)
            {
                LOG_CRIT("%s: failed to update free list\n", __func__);
            }

            PCL_Rc = PCL_INVALID_PARAMETER;
            goto error_exit;
        }
    } // Done adding new hash value to Flow Hash Table

    if (Rec_p->Context_p)
    {
        // Get list element
        List_Element_t * TmpElement_p = Rec_p->Context_p;

        // Get hash list that contains all transform record descriptors
        HashList_p = TmpElement_p->DataObject_p;
    }

    if(HashList_p == NULL)
    {
        // Create hash list for transform record

        List_Element_t * TmpElement_p = NULL;

        // Get a free list from pool of lists
        List_Rc = List_RemoveFromTail(LIST_DUMMY_LIST_ID,
                                      Dev_p->List_p,
                                      &TmpElement_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT("%s: failed to get free element from pool list\n",
                     __func__);
            goto error_exit;
        }

        // Note: TmpElement_p must be valid by implementation!
        HashList_p = TmpElement_p->DataObject_p;

        // Initialize list instance
        if (List_Init(LIST_DUMMY_LIST_ID, HashList_p) != LIST_STATUS_OK)
        {
            LOG_CRIT("%s: list initialization failed\n", __func__);
            goto error_exit;
        }

        // Store list element in the transform record descriptor
        Rec_p->Context_p = TmpElement_p;
    } // Created hash list for transform record

    // Add new hash value to list of hash values for this transform record
    List_Rc = List_AddToHead(LIST_DUMMY_LIST_ID, HashList_p, Element_p);
    if (List_Rc != LIST_STATUS_OK)
    {
        LOG_CRIT("%s: failed to update hash list for transform record\n",
                 __func__);
    }

    HashHandle_p->p = Element_p;

error_exit:

    Adapter_Lock_CS_Leave(&Dev_p->AdapterPCL_DevCS);
    return PCL_Rc;
}


/*-----------------------------------------------------------------------------
 * PCL_DTL_Transform_Remove
 */
PCL_Status_t
PCL_DTL_Transform_Remove(
        const unsigned int InterfaceId,
        const unsigned int FlowHashTableId,
        const DMABuf_Handle_t XformDMAHandle)
{
    PCL_Status_t PCL_Rc = PCL_ERROR;
    PCL_Status_t PCL_Rc2;
    AdapterPCL_Device_Instance_Data_t * Dev_p;
    EIP207_Flow_Error_t EIP207_Rc;
    EIP207_Flow_IOArea_t * ioarea_p;
    EIP207_Flow_TR_Dscr_t * TR_Dscr_p;
    List_Status_t List_Rc;
    List_Element_t * Element_p;
    List_Element_t * ListElement_p;
    unsigned int ListID;
    void * HashList_p;

    DMAResource_Handle_t DMAHandle =
        Adapter_DMABuf_Handle2DMAResourceHandle(XformDMAHandle);

    DMAResource_Record_t * Rec_p = DMAResource_Handle2RecordPtr(DMAHandle);

    LOG_INFO("\n\t %s \n", __func__);

    // Validate input parameters
    if (InterfaceId >= ADAPTER_PCL_MAX_FLUE_DEVICES ||
        FlowHashTableId >= ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE ||
        Rec_p == NULL)
        return PCL_INVALID_PARAMETER;

    // Get interface ioarea
    Dev_p = AdapterPCL_Device_Get(InterfaceId);
    ioarea_p = Dev_p->EIP207_IOArea_p;

    if (Dev_p->List_p == NULL)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return PCL_ERROR;
    }

    if (Adapter_Lock_CS_Get(&Dev_p->AdapterPCL_DevCS) == Adapter_Lock_NULL)
    {
        LOG_CRIT("%s: no device lock, not initialized?\n", __func__);
        return PCL_ERROR;
    }

    if (!Adapter_Lock_CS_Enter(&Dev_p->AdapterPCL_DevCS))
        return PCL_STATUS_BUSY;

    PCL_Rc2 = AdapterPCL_ListID_Get(InterfaceId, &ListID);
    if (PCL_Rc2 != PCL_STATUS_OK)
    {
        LOG_CRIT("%s: failed to get free list\n", __func__);
        goto error_exit;
    }

    // Get list element
    ListElement_p = Rec_p->Context_p;
    if (ListElement_p == NULL || ListElement_p->DataObject_p == NULL)
    {
        PCL_Rc = PCL_INVALID_PARAMETER;
        goto error_exit;
    }

    // Get hash list that contains all transform record descriptors
    // for this transform
    HashList_p = ListElement_p->DataObject_p;

    while (HashList_p)
    {
        // Get the element from hash list that references record descriptor
        List_Rc = List_RemoveFromTail(LIST_DUMMY_LIST_ID,
                                      HashList_p,
                                      &Element_p);
        if (List_Rc != LIST_STATUS_OK || Element_p == NULL)
        {
            LOG_CRIT(
                "%s: failed to get element from hash list\n", __func__);
            goto error_exit;
        }

        // Retrieve transform record descriptor
        TR_Dscr_p = Element_p->DataObject_p;
        if (TR_Dscr_p == NULL)
        {
            LOG_CRIT("%s: failed, invalid transform handle\n", __func__);
            PCL_Rc = PCL_INVALID_PARAMETER;
            goto error_exit;
        }

        if (RPM_DEVICE_IO_START_MACRO(ADAPTER_PCL_RPM_EIP207_DEVICE_ID + InterfaceId,
                                      RPM_FLAG_SYNC) != RPM_SUCCESS)
        {
            PCL_Rc = PCL_ERROR;
            goto error_exit;
        }

        LOG_INFO("\n\t\t EIP207_Flow_DTL_TR_Remove \n");

        EIP207_Rc = EIP207_Flow_DTL_TR_Remove(ioarea_p,
                                              FlowHashTableId,
                                              TR_Dscr_p);

        (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_PCL_RPM_EIP207_DEVICE_ID + InterfaceId,
                                       RPM_FLAG_ASYNC);

        if (EIP207_Rc != EIP207_FLOW_NO_ERROR)
        {
            LOG_CRIT("%s: failed to remove TR, "
                     "EIP207_Flow_DTL_TR_Remove() error %d\n",
                    __func__,
                    EIP207_Rc);
            // Return element to hash list
            List_AddToHead(LIST_DUMMY_LIST_ID, HashList_p, Element_p);
            goto error_exit;
        }

        {
            unsigned int Count;

            // Return the element to the free list
            List_Rc = List_AddToHead(ListID, NULL, Element_p);
            if (List_Rc != LIST_STATUS_OK)
            {
                LOG_CRIT(
                    "%s: failed to get free element from list\n", __func__);
                goto error_exit;
            }

            // Check if transform record has any hash values still referencing it
            List_GetListElementCount(LIST_DUMMY_LIST_ID, HashList_p, &Count);
            if (Count == 0)
            {
                // Return the element to the free list
                List_Rc = List_AddToHead(LIST_DUMMY_LIST_ID,
                                         Dev_p->List_p,
                                         ListElement_p);
                if (List_Rc != LIST_STATUS_OK)
                {
                    LOG_CRIT("%s: failed to return element to pool list\n",
                              __func__);
                    goto error_exit;
                }

                Rec_p->Context_p = HashList_p = NULL;
            }
        }
    } // while

    PCL_Rc = PCL_STATUS_OK;

error_exit:
    Adapter_Lock_CS_Leave(&Dev_p->AdapterPCL_DevCS);
    return PCL_Rc;
}


/*-----------------------------------------------------------------------------
 * PCL_DTL_Hash_Remove
 */
PCL_Status_t
PCL_DTL_Hash_Remove(
        const unsigned int InterfaceId,
        const unsigned int FlowHashTableId,
        PCL_DTL_Hash_Handle_t * const HashHandle_p)
{
    PCL_Status_t PCL_Rc = PCL_ERROR;
    PCL_Status_t PCL_Rc2;
    EIP207_Flow_Error_t EIP207_Rc;
    EIP207_Flow_IOArea_t * ioarea_p;
    EIP207_Flow_TR_Dscr_t * TR_Dscr_p;
    List_Element_t * Element_p;
    AdapterPCL_Device_Instance_Data_t * Dev_p;
    DMAResource_Record_t * Rec_p;

    LOG_INFO("\n\t %s \n", __func__);

    // Validate input parameters
    if (InterfaceId >= ADAPTER_PCL_MAX_FLUE_DEVICES ||
        FlowHashTableId >= ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE)
        return PCL_INVALID_PARAMETER;

    // Get interface ioarea
    Dev_p = AdapterPCL_Device_Get(InterfaceId);
    ioarea_p = Dev_p->EIP207_IOArea_p;

    if (Dev_p->List_p == NULL)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return PCL_ERROR;
    }

    if (Adapter_Lock_CS_Get(&Dev_p->AdapterPCL_DevCS) == Adapter_Lock_NULL)
    {
        LOG_CRIT("%s: no device lock, not initialized?\n", __func__);
        return PCL_ERROR;
    }

    if (!Adapter_Lock_CS_Enter(&Dev_p->AdapterPCL_DevCS))
        return PCL_STATUS_BUSY;

    // Retrieve transform record descriptor
    Element_p = HashHandle_p->p;
    if (Element_p == NULL)
    {
        LOG_CRIT("%s: failed, invalid hash handle\n", __func__);
        PCL_Rc = PCL_INVALID_PARAMETER;
        goto error_exit;
    }

    TR_Dscr_p = Element_p->DataObject_p;
    if (TR_Dscr_p == NULL)
    {
        LOG_CRIT("%s: failed, invalid hash handle for transform descriptor\n",
                 __func__);
        PCL_Rc = PCL_INVALID_PARAMETER;
        goto error_exit;
    }

    Rec_p = DMAResource_Handle2RecordPtr(TR_Dscr_p->DMA_Handle);
    if (Rec_p == NULL || Rec_p->Context_p == NULL)
    {
        LOG_CRIT("%s: failed, invalid hash handle for DMA resource\n",
                 __func__);
        PCL_Rc = PCL_INVALID_PARAMETER;
        goto error_exit;
    }

    if (RPM_DEVICE_IO_START_MACRO(ADAPTER_PCL_RPM_EIP207_DEVICE_ID + InterfaceId,
                                  RPM_FLAG_SYNC) != RPM_SUCCESS)
    {
        PCL_Rc = PCL_ERROR;
        goto error_exit;
    }

    LOG_INFO("\n\t\t EIP207_Flow_DTL_TR_Remove \n");

    EIP207_Rc = EIP207_Flow_DTL_TR_Remove(ioarea_p, FlowHashTableId, TR_Dscr_p);

    (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_PCL_RPM_EIP207_DEVICE_ID + InterfaceId,
                                   RPM_FLAG_ASYNC);

    if (EIP207_Rc != EIP207_FLOW_NO_ERROR)
    {
        LOG_CRIT("%s: failed to remove TR, "
                 "EIP207_Flow_DTL_TR_Remove() error %d\n",
                __func__,
                EIP207_Rc);
        goto error_exit;
    }

    {
        unsigned int ListID, Count;
        List_Status_t List_Rc;
        List_Element_t * ListElement_p = Rec_p->Context_p;
        void * HashList_p = ListElement_p->DataObject_p;

        // Remove the hash value from the transform record list
        List_Rc = List_RemoveAnywhere(LIST_DUMMY_LIST_ID,
                                      HashList_p,
                                      Element_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT("%s: failed to remove hash from record list\n", __func__);
        }

        PCL_Rc2 = AdapterPCL_ListID_Get(InterfaceId, &ListID);
        if (PCL_Rc2 != PCL_STATUS_OK)
        {
            LOG_CRIT("%s: failed to get free list\n", __func__);
            goto error_exit;
        }

        // Return the element to the free list
        List_Rc = List_AddToHead(ListID, NULL, Element_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT(
                "%s: failed to get free element from list\n", __func__);
            goto error_exit;
        }

        // Check if transform record has any hash values still referencing it
        List_GetListElementCount(LIST_DUMMY_LIST_ID, HashList_p, &Count);
        if (Count == 0)
        {
            // Return the element to the free list
            List_Rc = List_AddToHead(LIST_DUMMY_LIST_ID,
                                     Dev_p->List_p,
                                     ListElement_p);
            if (List_Rc != LIST_STATUS_OK)
            {
                LOG_CRIT("%s: failed to return element to pool list\n",
                          __func__);
                goto error_exit;
            }

            Rec_p->Context_p = NULL;
        }
    }

    // Invalidate hash handle
    HashHandle_p->p = NULL;

    PCL_Rc = PCL_STATUS_OK;

error_exit:
    Adapter_Lock_CS_Leave(&Dev_p->AdapterPCL_DevCS);
    return PCL_Rc;
}


/*-----------------------------------------------------------------------------
 * PCL_DTL_Init
 */
PCL_Status_t
PCL_DTL_Init(
        const unsigned int InterfaceId)
{
    PCL_Status_t PCL_Rc = PCL_ERROR;
    AdapterPCL_Device_Instance_Data_t * Dev_p;

    LOG_INFO("\n\t %s \n", __func__);

    // Validate input parameters.
    if (InterfaceId >= ADAPTER_PCL_MAX_FLUE_DEVICES)
        return PCL_INVALID_PARAMETER;

    // Get interface ioarea
    Dev_p = AdapterPCL_Device_Get(InterfaceId);

    if (Dev_p->List_p != NULL)
    {
        LOG_CRIT("%s: failed, already initialized\n", __func__);
        return PCL_ERROR;
    }

    if (Adapter_Lock_CS_Get(&Dev_p->AdapterPCL_DevCS) == Adapter_Lock_NULL)
    {
        LOG_CRIT("%s: failed, no device lock, not initialized?\n", __func__);
        return PCL_ERROR;
    }

    if (!Adapter_Lock_CS_Enter(&Dev_p->AdapterPCL_DevCS))
        return PCL_STATUS_BUSY;

    // Create pool of lists for record descriptors
    {
        unsigned int i;
        void * List_p;
        List_Element_t * ListElementPool_p;
        List_Element_t * Element_p;
        unsigned char * ListPool_p;
        unsigned int ListInstanceByteCount = List_GetInstanceByteCount();

        // Allocate PCL DTL list instance that will chain other lists
        List_p = Adapter_Alloc(ListInstanceByteCount);
        if (List_p == NULL)
        {
            LOG_CRIT("%s: list pool allocation failed\n", __func__);
            goto error_exit;
        }
        memset(List_p, 0, ListInstanceByteCount);

        // Initialize PCL DTL list instance
        if (List_Init(LIST_DUMMY_LIST_ID,
                      List_p) != LIST_STATUS_OK)
        {
            LOG_CRIT("%s: list pool initialization failed\n", __func__);
            goto error_exit;
        }

        // Allocate a pool of list elements
        ListElementPool_p = Adapter_Alloc(sizeof(List_Element_t) *
                                            ADAPTER_PCL_FLOW_RECORD_COUNT);
        if (ListElementPool_p == NULL)
        {
            LOG_CRIT("%s: pool elements allocation failed\n", __func__);
            Adapter_Free(List_p);
            goto error_exit;
        }

        // Allocate a pool of lists,
        // one list for one transform record
        ListPool_p = Adapter_Alloc(ListInstanceByteCount *
                                           ADAPTER_PCL_FLOW_RECORD_COUNT);
        if (ListPool_p == NULL)
        {
            LOG_CRIT("%s:  pool lists allocation failed\n", __func__);
            Adapter_Free(List_p);
            Adapter_Free(ListElementPool_p);
            goto error_exit;
        }
        memset(ListPool_p,
               0,
               ListInstanceByteCount * ADAPTER_PCL_FLOW_RECORD_COUNT);
        Dev_p->ListPool_p = ListPool_p;

        // Populate the pool list with the elements (lists)
        Element_p = ListElementPool_p;
        Element_p->DataObject_p = ListPool_p;
        for (i = 0; i < ADAPTER_PCL_FLOW_RECORD_COUNT; i++)
        {
            if (List_AddToHead(LIST_DUMMY_LIST_ID,
                               List_p,
                               Element_p) == LIST_STATUS_OK)
            {
                if (i + 1 < ADAPTER_PCL_FLOW_RECORD_COUNT)
                {
                    Element_p++;
                    ListPool_p += ListInstanceByteCount;
                    Element_p->DataObject_p = ListPool_p;
                }
            }
            else
            {
                LOG_CRIT("%s: pool list population failed\n", __func__);
                Dev_p->ListPool_p = NULL;
                Adapter_Free(List_p);
                Adapter_Free(ListElementPool_p);
                Adapter_Free(ListPool_p);
                goto error_exit;
            }
        } // for

        Dev_p->List_p            = List_p;
        Dev_p->ListElementPool_p = ListElementPool_p;

        PCL_Rc = PCL_STATUS_OK;
    }  // Created pool of lists for record descriptors

error_exit:

    Adapter_Lock_CS_Leave(&Dev_p->AdapterPCL_DevCS);
    return PCL_Rc;
}


/*-----------------------------------------------------------------------------
 * PCL_DTL_UnInit
 */
PCL_Status_t
PCL_DTL_UnInit(
        const unsigned int InterfaceId)
{
    AdapterPCL_Device_Instance_Data_t * Dev_p;

    LOG_INFO("\n\t %s \n", __func__);

    // Validate input parameters.
    if (InterfaceId >= ADAPTER_PCL_MAX_FLUE_DEVICES)
        return PCL_INVALID_PARAMETER;

    // Get interface ioarea
    Dev_p = AdapterPCL_Device_Get(InterfaceId);
    if (Dev_p->List_p == NULL)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return PCL_ERROR;
    }

    if (Adapter_Lock_CS_Get(&Dev_p->AdapterPCL_DevCS) == Adapter_Lock_NULL)
    {
        LOG_CRIT("%s: failed, no device lock, not initialized?\n", __func__);
        return PCL_ERROR;
    }

    if (!Adapter_Lock_CS_Enter(&Dev_p->AdapterPCL_DevCS))
        return PCL_STATUS_BUSY;

    // pool list data structures
    Adapter_Free(Dev_p->ListElementPool_p);
    Adapter_Free(Dev_p->ListPool_p);
    List_Uninit(LIST_DUMMY_LIST_ID, Dev_p->List_p);
    Adapter_Free(Dev_p->List_p);

    Dev_p->List_p               = NULL;
    Dev_p->ListElementPool_p    = NULL;
    Dev_p->ListPool_p           = NULL;

    Adapter_Lock_CS_Leave(&Dev_p->AdapterPCL_DevCS);

    return PCL_STATUS_OK;
}


/* end of file adapter_pcl_dtl.c */
