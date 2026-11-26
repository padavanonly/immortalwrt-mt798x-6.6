/* adapter_pec_pktbuf.c
 *
 * Helper functions to access packet data via DMABuf handles, possibly
 * in scatter-gather lists.
 */

/*****************************************************************************
* Copyright (c) 2020-2021 by Rambus, Inc. and/or its subsidiaries.
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

#include "adapter_pec_pktbuf.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default Adapter PEC configuration
#include "c_adapter_pec.h"

#include "api_pec_sg.h"         // PEC_SG_* (the API we implement here)


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

/*----------------------------------------------------------------------------
 * Adapter_PEC_PktData_Get
 */
uint8_t *
Adapter_PEC_PktData_Get(
        DMABuf_Handle_t PacketHandle,
        uint8_t *CopyBuffer_p,
        unsigned int StartOffs,
        unsigned int ByteCount)
{
#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
    unsigned int NofParticles;
    PEC_SGList_GetCapacity(PacketHandle, &NofParticles);
    if (NofParticles != 0)
    {
        unsigned int TotalByteCount = 0;
        unsigned int BytesCopied = 0;
        unsigned int BytesSkip;
        unsigned int i;
        unsigned int FragmentByteCount;
        DMABuf_Handle_t FragmentHandle;
        uint8_t *FragmentPtr;
        for (i=0; i<NofParticles; i++)
        {
            PEC_SGList_Read(PacketHandle, i, &FragmentHandle,
                            &FragmentByteCount, &FragmentPtr);
            if (TotalByteCount + FragmentByteCount >=
                StartOffs + ByteCount)
            {
                // This is the last fragment to visit (possibly the first).
                if (BytesCopied == 0)
                {
                    // No need to copy everything is in single fragment.
                    return FragmentPtr + StartOffs - TotalByteCount;
                }
                else
                {
                    // Copy the final fragment.
                    memcpy(CopyBuffer_p + BytesCopied,
                           FragmentPtr, ByteCount - BytesCopied);
                    return CopyBuffer_p;
                }
            }
            else if (TotalByteCount + FragmentByteCount >
                     StartOffs)
            {
                // This fragment contains data that must be copied
                if (BytesCopied == 0)
                {
                    // First fragment containing data to copy, may need to skip
                    // bytes at start of fragment.
                    BytesSkip = StartOffs - TotalByteCount;
                }
                else
                {   // Later fragments, copy from start of fragment.
                    BytesSkip = 0;
                }
                if (CopyBuffer_p == NULL)
                    return NULL; // Skip copying altogether.
                memcpy(CopyBuffer_p + BytesCopied,
                       FragmentPtr + BytesSkip,
                       FragmentByteCount - BytesSkip);
                BytesCopied += FragmentByteCount - BytesSkip;
            }
            TotalByteCount += FragmentByteCount;
        }
        // We haven't collected enough data here, return NULL pointer.
        return NULL;
    }
    else
#endif
    {
        DMAResource_Handle_t DMAHandle =
            Adapter_DMABuf_Handle2DMAResourceHandle(PacketHandle);
        uint8_t *Packet_p;
        IDENTIFIER_NOT_USED(CopyBuffer_p);
        IDENTIFIER_NOT_USED(ByteCount);
        if (DMAResource_IsValidHandle(DMAHandle))
        {
            Packet_p = (uint8_t *)Adapter_DMAResource_HostAddr(DMAHandle) + StartOffs;
        }
        else
        {
            Packet_p = NULL;
        }
        return Packet_p;
    }
}

/*----------------------------------------------------------------------------
 * Adapter_PEC_PktByte_Put
 */
void
Adapter_PEC_PktByte_Put(
        DMABuf_Handle_t PacketHandle,
        unsigned int Offset,
        unsigned int Byte)
{
#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
    unsigned int NofParticles;
    PEC_SGList_GetCapacity(PacketHandle, &NofParticles);
    if (NofParticles != 0)
    {
        unsigned int TotalByteCount = 0;
        unsigned int i;
        unsigned int FragmentByteCount;
        DMABuf_Handle_t FragmentHandle;
        uint8_t *FragmentPtr;
        for (i=0; i<NofParticles; i++)
        {
            PEC_SGList_Read(PacketHandle, i, &FragmentHandle,
                            &FragmentByteCount, &FragmentPtr);
            if (TotalByteCount + FragmentByteCount > Offset)
            {
                // Found the fragment where the byte must be changed..
                FragmentPtr[Offset - TotalByteCount] = Byte;
                return;
            }
            TotalByteCount += FragmentByteCount;
        }
    }
    else
#endif
    {
        DMAResource_Handle_t DMAHandle =
            Adapter_DMABuf_Handle2DMAResourceHandle(PacketHandle);
        uint8_t *Packet_p;
        if (DMAResource_IsValidHandle(DMAHandle))
        {
            Packet_p = (uint8_t *)Adapter_DMAResource_HostAddr(DMAHandle) + Offset;
            Packet_p[0] = Byte;
        }
    }
}




/* end of file adapter_pec_dmabuf.c */
