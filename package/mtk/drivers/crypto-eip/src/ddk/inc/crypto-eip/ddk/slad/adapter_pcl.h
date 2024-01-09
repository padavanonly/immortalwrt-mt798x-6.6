/* adapter_pcl.h
 *
 * Adapter PCL Internal interface
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

#ifndef INCLUDE_GUARD_ADAPTER_PCL_INTERNAL_H
#define INCLUDE_GUARD_ADAPTER_PCL_INTERNAL_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "basic_defs.h"         // bool

// EIP-207 Driver Library
#include "eip207_flow_generic.h"

// List API
#include "list.h"               // List_*

// DMABuf API
#include "api_dmabuf.h"         // DMABuf_*

// Adapter Locking internal API
#include "adapter_lock.h"       // Adapter_Lock_*


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

typedef struct
{
    volatile bool PCL_IsInitialized;
    EIP207_Flow_IOArea_t * EIP207_IOArea_p;

    void * EIP207_Descriptor_Area_p;
    DMAResource_Handle_t EIP207_Hashtable_DMA_Handle;

    unsigned int FreeListID;
    List_Element_t * ElementPool_p;
    unsigned char * RecDscrPool_p;

    // Pointer to pool list
    void * List_p;

    // Pointer to elements for pool of lists
    List_Element_t * ListElementPool_p;

    // Pointer to a pool of lists
    unsigned char * ListPool_p;

    // Device-specific lock and critical section
    Adapter_Lock_t AdapterPCL_DevLock;
    Adapter_Lock_CS_t AdapterPCL_DevCS;

} AdapterPCL_Device_Instance_Data_t;


/*-----------------------------------------------------------------------------
 * AdapterPCL_Device_Get
 *
 * Obtain PCL Device pointer for the provided interface id
 */
AdapterPCL_Device_Instance_Data_t *
AdapterPCL_Device_Get(
        const unsigned int InterfaceId);


/*-----------------------------------------------------------------------------
  AdapterPCL_DMABuf_To_TRDscr

  Convert DMABuf handle to Transform record descriptor

  TransformHandle (input)
    DMABuf handle representing the transform.

  TR_Dscr_p (output)
    Pointer to TR descriptor.

  Rec_pp (output)
    Pointer to memory location where pointer to DMAesource record descriptor
    will be stored.

  Return:
      PCL_STATUS_OK if succeeded.
      PCL_ERROR
*/
PCL_Status_t
AdapterPCL_DMABuf_To_TRDscr(
        const DMABuf_Handle_t TransformHandle,
        EIP207_Flow_TR_Dscr_t * const TR_Dscr_p,
        DMAResource_Record_t ** Rec_pp);


/*-----------------------------------------------------------------------------
 * AdapterPCL_ListID_Get
 *
 * Obtain List pointer for the provided interface id
 *
 * Return:
 *     PCL_STATUS_OK if succeeded.
 *     PCL_ERROR
 */
PCL_Status_t
AdapterPCL_ListID_Get(
        const unsigned int InterfaceId,
        unsigned int * const ListID_p);


#endif /* Include Guard */


/* end of file adapter_pcl.h */
