/* adapter_pec_dma.c
 *
 * Packet Engine Control (PEC) API Implementation
 * using DMA mode.
 */

/*****************************************************************************
* Copyright (c) 2011-2022 by Rambus, Inc. and/or its subsidiaries.
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

#include "api_pec.h"            // PEC_* (the API we implement here)


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default Adapter PEC configuration
#include "c_adapter_pec.h"

// DMABuf API
#include "api_dmabuf.h"         // DMABuf_*

// Adapter DMABuf internal API
#include "adapter_dmabuf.h"

// Adapter PEC device API
#include "adapter_pecdev_dma.h" // Adapter_PECDev_*

// Adapter Locking internal API
#include "adapter_lock.h"       // Adapter_Lock_*

#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
#include "api_pec_sg.h"         // PEC_SG_* (the API we implement here)
#endif

// Runtime Power Management Device Macros API
#include "rpm_device_macros.h"  // RPM_*

// Logging API
#include "log.h"

// Driver Framework DMAResource API
#include "dmares_types.h"       // DMAResource_Handle_t
#include "dmares_mgmt.h"        // DMAResource management functions
#include "dmares_rw.h"          // DMAResource buffer access.
#include "dmares_addr.h"        // DMAResource addr translation functions.
#include "dmares_buf.h"         // DMAResource buffer allocations

// Standard IOToken API
#include "iotoken.h"

// Driver Framework C Run-Time Library API
#include "clib.h"               // memcpy, memset

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // bool, uint32_t


#ifndef ADAPTER_PE_MODE_DHM
/*----------------------------------------------------------------------------
 * Definitions and macros
 */

typedef struct
{
    void * User_p;
    DMABuf_Handle_t SrcPkt_Handle;
    DMABuf_Handle_t DstPkt_Handle;
    DMABuf_Handle_t Token_Handle;
    unsigned int Bypass_WordCount;
} Adapter_SideChannelRecord_t;


/* Side channel FIFO
   - Normal operation: PEC_Packet_Put adds a record containing several
                                      words for each packet.
                       PEC_Packet_Get pops one record for each packet, fills
                                      fields into result descriptor.
   - Contiuous scatter mode: PEC_Scatter_Preload adds a record
                                      with DestPkt_Handle only for each scatter
                                      buffer.
                             PEC_Packet_Get pops a record for each scatter
                                      buffer used. Can do PostDMA for each
                                      scatter buffer, will not fill in fields
                                      in result descriptor.
*/
typedef struct
{
    int Size;
    int ReadIndex;
    int WriteIndex;
    Adapter_SideChannelRecord_t Records[1 + ADAPTER_PEC_MAX_PACKETS +
                                        ADAPTER_PEC_MAX_LOGICDESCR];
} AdapterPEC_SideChannelFIFO_t;


/*----------------------------------------------------------------------------
 * Local variables
 */
static volatile bool PEC_IsInitialized[ADAPTER_PEC_DEVICE_COUNT];
static volatile bool PEC_ContinuousScatter[ADAPTER_PEC_DEVICE_COUNT];

// Lock and critical section for PEC_Init/Uninit()
static ADAPTER_LOCK_DEFINE(AdapterPEC_InitLock);
static Adapter_Lock_CS_t AdapterPEC_InitCS;

// Locks and critical sections for PEC_Packet_Put()
static Adapter_Lock_t AdapterPEC_PutLock[ADAPTER_PEC_DEVICE_COUNT];
static Adapter_Lock_CS_t AdapterPEC_PutCS[ADAPTER_PEC_DEVICE_COUNT];

// Locks and critical sections for PEC_Packet_Get()
static Adapter_Lock_t AdapterPEC_GetLock[ADAPTER_PEC_DEVICE_COUNT];
static Adapter_Lock_CS_t AdapterPEC_GetCS[ADAPTER_PEC_DEVICE_COUNT];

static AdapterPEC_SideChannelFIFO_t
Adapter_SideChannelFIFO[ADAPTER_PEC_DEVICE_COUNT];

static struct
{
    volatile PEC_NotifyFunction_t ResultNotifyCB_p;
    volatile unsigned int ResultsCount;

    volatile PEC_NotifyFunction_t CommandNotifyCB_p;
    volatile unsigned int CommandsCount;

} PEC_Notify[ADAPTER_PEC_DEVICE_COUNT];


#ifdef ADAPTER_PEC_INTERRUPTS_ENABLE
/*----------------------------------------------------------------------------
 * AdapterPEC_InterruptHandlerResultNotify
 *
 * This function is the interrupt handler for the PEC interrupt
 * sources that indicate the arrival of a a result descriptor..There
 * may be several interrupt sources.
 *
 * This function is used to invoke the PEC result notification callback.
 */
static void
AdapterPEC_InterruptHandlerResultNotify(
        const int nIRQ,
        const unsigned int flags)
{
    unsigned int InterfaceId = Adapter_PECDev_IRQToInferfaceId(nIRQ);

    IDENTIFIER_NOT_USED(flags);

    if (InterfaceId >= ADAPTER_PEC_DEVICE_COUNT)
    {
        LOG_CRIT("AdapterPEC_InterruptHandlerResultNotify"
                 "InterfaceId out of range\n");
        return;
    }

    Adapter_PECDev_Disable_ResultIRQ(InterfaceId);

    LOG_INFO("AdapterPEC_InterruptHandlerResultNotify: Enter\n");

    if (PEC_Notify[InterfaceId].ResultNotifyCB_p != NULL)
    {
        PEC_NotifyFunction_t CBFunc_p;

        // Keep the callback on stack to allow registration
        // of another result notify request from callback
        CBFunc_p = PEC_Notify[InterfaceId].ResultNotifyCB_p;

        PEC_Notify[InterfaceId].ResultNotifyCB_p = NULL;
        PEC_Notify[InterfaceId].ResultsCount = 0;

        LOG_INFO(
            "AdapterPEC_InterruptHandlerResultNotify: "
            "Invoking PEC result notify callback for interface %d\n",
            InterfaceId);

        CBFunc_p();
    }
}


/*----------------------------------------------------------------------------
 * AdapterPEC_InterruptHandlerCommandNotify
 *
 * This function is the interrupt handler for the PEC interrupt sources.that
 * indicate that there is again freee space for new command descriptors.
 *
 * This function is used to invoke the PEC command notification callback.
 */
static void
AdapterPEC_InterruptHandlerCommandNotify(
        const int nIRQ,
        const unsigned int flags)
{
    unsigned int InterfaceId = Adapter_PECDev_IRQToInferfaceId(nIRQ);

    IDENTIFIER_NOT_USED(flags);

    if (InterfaceId >= ADAPTER_PEC_DEVICE_COUNT)
    {
        LOG_CRIT("AdapterPEC_InterruptHandlerCommandNotify"
                 "InterfaceId out of range\n");
        return;
    }

    Adapter_PECDev_Disable_CommandIRQ(InterfaceId);

    LOG_INFO("AdapterPEC_InterruptHandlerCommandNotify: Enter\n");

    if (PEC_Notify[InterfaceId].CommandNotifyCB_p != NULL)
    {
        PEC_NotifyFunction_t CBFunc_p;

        // Keep the callback on stack to allow registration
        // of another command notify request from callback
        CBFunc_p = PEC_Notify[InterfaceId].CommandNotifyCB_p;

        PEC_Notify[InterfaceId].CommandNotifyCB_p = NULL;
        PEC_Notify[InterfaceId].CommandsCount = 0;

        LOG_INFO(
            "AdapterPEC_InterruptHandlerCommandNotify: "
            "Invoking PEC command notify callback interface=%d\n",
            InterfaceId);

        CBFunc_p();
    }
}
#endif /* ADAPTER_PEC_INTERRUPTS_ENABLE */


/*----------------------------------------------------------------------------
 * Adapter_MakeCommandNotify_CallBack
 */
static inline void
Adapter_MakeCommandNotify_CallBack(unsigned int InterfaceId)
{
    unsigned int PacketSlotsEmptyCount;

    if (InterfaceId >= ADAPTER_PEC_DEVICE_COUNT)
        return;

    if (PEC_Notify[InterfaceId].CommandNotifyCB_p != NULL)
    {
        PacketSlotsEmptyCount = Adapter_PECDev_GetFreeSpace(InterfaceId);

        if (PEC_Notify[InterfaceId].CommandsCount <= PacketSlotsEmptyCount)
        {
            PEC_NotifyFunction_t CBFunc_p;

            // Keep the callback on stack to allow registeration
            // of another result notify request from callback
            CBFunc_p = PEC_Notify[InterfaceId].CommandNotifyCB_p;

            PEC_Notify[InterfaceId].CommandNotifyCB_p = NULL;
            PEC_Notify[InterfaceId].CommandsCount = 0;

            LOG_INFO(
                "PEC_Packet_Get: "
                "Invoking command notify callback\n");

            CBFunc_p();
        }
    }
}


/*----------------------------------------------------------------------------
 * Adapter_PECResgisterSA_BounceIfRequired
 *
 * Returns false in case of error.
 * Allocate a bounce buffer and copy the data in case this if required.
 */
#ifndef ADAPTER_PEC_REMOVE_BOUNCEBUFFERS
static bool
Adapter_PECRegisterSA_BounceIfRequired(
        DMAResource_Handle_t *DMAHandle_p)
{
    DMAResource_Handle_t DMAHandle = *DMAHandle_p;
    DMAResource_Record_t * Rec_p;
    DMAResource_AddrPair_t BounceHostAddr;
    void * HostAddr;
    int dmares;

    // skip null handles
    if (!DMAResource_IsValidHandle(DMAHandle))
        return true;    // no error

    Rec_p = DMAResource_Handle2RecordPtr(DMAHandle);


    // skip proper buffers
    if (!Adapter_DMAResource_IsForeignAllocated(DMAHandle))
    {
        Rec_p->bounce.Bounce_Handle = NULL;
        return true;    // no error
    }

    {
        DMAResource_Properties_t BounceProperties;

        // used as uint32_t array
        BounceProperties.Alignment  = Adapter_DMAResource_Alignment_Get();
        BounceProperties.Bank       = ADAPTER_PEC_BANK_SA;
        BounceProperties.fCached    = false;
        BounceProperties.Size       = Rec_p->Props.Size;

        HostAddr = Adapter_DMAResource_HostAddr(DMAHandle);

        dmares = DMAResource_Alloc(
                     BounceProperties,
                     &BounceHostAddr,
                     &Rec_p->bounce.Bounce_Handle);

        // bounce buffer handle is stored in the DMA Resource Record
        // of the original buffer, which links the two
        // this will be used when freeing the buffer
        // but also when the SA is referenced in packet put

        if (dmares != 0)
        {
            LOG_CRIT(
                "PEC_SA_Register: "
                "Failed to alloc bounce buffer (error %d)\n",
                dmares);
            return false;   // error!
        }
        LOG_INFO(
            "PEC_SA_Register: "
            "Bouncing SA: %p to %p\n",
            DMAHandle,
            Rec_p->bounce.Bounce_Handle);
#ifdef ADAPTER_PEC_ARMRING_ENABLE_SWAP
        DMAResource_SwapEndianness_Set(Rec_p->bounce.Bounce_Handle, true);
#endif

    }

    // copy the data to the bounce buffer
    memcpy(
        BounceHostAddr.Address_p,
        HostAddr,
        Rec_p->Props.Size);

    *DMAHandle_p = Rec_p->bounce.Bounce_Handle;
    return true;        // no error
}
#endif /* ADAPTER_PEC_REMOVE_BOUNCEBUFFERS */


/*----------------------------------------------------------------------------
 * Adapter_FIFO_Put
 *
 * Put packet information into the side channel FIFO
 */
static bool
Adapter_FIFO_Put(AdapterPEC_SideChannelFIFO_t *FIFO,
                 void *User_p,
                 DMABuf_Handle_t SrcPkt_Handle,
                 DMABuf_Handle_t DstPkt_Handle,
                 DMABuf_Handle_t Token_Handle,
                 unsigned int Bypass_WordCount)
{
    int WriteIndex = FIFO->WriteIndex;
    int ReadIndex = FIFO->ReadIndex;
    if (WriteIndex == ReadIndex - 1 ||
        (ReadIndex == 0 && WriteIndex == FIFO->Size - 1))
    {
        LOG_CRIT("Side channel FIFO full\n");
        return false;
    }
    FIFO->Records[WriteIndex].User_p = User_p;
    FIFO->Records[WriteIndex].SrcPkt_Handle = SrcPkt_Handle;
    FIFO->Records[WriteIndex].DstPkt_Handle = DstPkt_Handle;

    FIFO->Records[WriteIndex].Token_Handle = Token_Handle;
    if (!DMABuf_Handle_IsSame(&Token_Handle, &DMABuf_NULLHandle))
    {
        FIFO->Records[WriteIndex].Bypass_WordCount = Bypass_WordCount;
    }

    WriteIndex += 1;
    if (WriteIndex == FIFO->Size)
        WriteIndex = 0;
    FIFO->WriteIndex = WriteIndex;
    return true;
}


/*----------------------------------------------------------------------------
 * Adapter_FIFO_Get
 *
 * Get and remove the oldest entry from the side channel FIFO.
 */
static bool
Adapter_FIFO_Get(AdapterPEC_SideChannelFIFO_t *FIFO,
                 void **User_p,
                 DMABuf_Handle_t *SrcPkt_Handle_p,
                 DMABuf_Handle_t *DstPkt_Handle_p,
                 DMABuf_Handle_t *Token_Handle_p,
                 unsigned int *Bypass_WordCount_p)
{
    int WriteIndex = FIFO->WriteIndex;
    int ReadIndex = FIFO->ReadIndex;
    if (WriteIndex == ReadIndex)
    {
        LOG_CRIT("Trying to read from empty FIFO\n");
        return false;
    }
    if (User_p)
        *User_p = FIFO->Records[ReadIndex].User_p;
    if (SrcPkt_Handle_p)
        *SrcPkt_Handle_p = FIFO->Records[ReadIndex].SrcPkt_Handle;
    *DstPkt_Handle_p = FIFO->Records[ReadIndex].DstPkt_Handle;

    if (Token_Handle_p)
        *Token_Handle_p = FIFO->Records[ReadIndex].Token_Handle;
    if (Token_Handle_p != NULL &&
        !DMABuf_Handle_IsSame(Token_Handle_p, &DMABuf_NULLHandle) &&
        Bypass_WordCount_p != NULL)
        *Bypass_WordCount_p = FIFO->Records[ReadIndex].Bypass_WordCount;

    ReadIndex += 1;
    if (ReadIndex == FIFO->Size)
        ReadIndex = 0;
    FIFO->ReadIndex = ReadIndex;
    return true;
}


/*----------------------------------------------------------------------------
 * Adapter_FIFO_Withdraw
 *
 * Withdraw the most recently added record from the side channel FIFO.
 */
static void
Adapter_FIFO_Withdraw(
        AdapterPEC_SideChannelFIFO_t *FIFO)
{
    int WriteIndex = FIFO->WriteIndex;
    if (WriteIndex == FIFO->ReadIndex)
    {
        LOG_CRIT("Adapter_FIFO_Withdraw: FIFO is empty\n");
    }
    if (WriteIndex == 0)
        WriteIndex = FIFO->Size - 1;
    else
        WriteIndex -= 1;
    FIFO->WriteIndex = WriteIndex;
}


/* Adapter_Packet_Prepare
 *
 * In case of bounce buffers, allocate bounce buffers for the packet and
 * the packet token.
 * Copy source packet and token into the bounce buffers.
 * Perform PreDMA on all packet buffers (source, destination and token).
 */
static PEC_Status_t
Adapter_Packet_Prepare(
        const unsigned int InterfaceId,
        const PEC_CommandDescriptor_t *Cmd_p)
{
    DMAResource_Handle_t SrcPkt_Handle, DstPkt_Handle, Token_Handle;
#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
    unsigned int ParticleCount;
    unsigned int i;
    DMABuf_Handle_t ParticleHandle;
    DMAResource_Handle_t DMARes_Handle;
    uint8_t * DummyPtr;
    unsigned int ParticleSize;
#endif

    SrcPkt_Handle =
        Adapter_DMABuf_Handle2DMAResourceHandle(Cmd_p->SrcPkt_Handle);
    DstPkt_Handle =
        Adapter_DMABuf_Handle2DMAResourceHandle(Cmd_p->DstPkt_Handle);
    Token_Handle = Adapter_DMABuf_Handle2DMAResourceHandle(Cmd_p->Token_Handle);

    if (!DMAResource_IsValidHandle(SrcPkt_Handle) &&
        !DMAResource_IsValidHandle(DstPkt_Handle))
        return PEC_STATUS_OK; // For record invalidation in the Record Cache
    else if (!DMAResource_IsValidHandle(SrcPkt_Handle) ||
             (!DMAResource_IsValidHandle(DstPkt_Handle) &&
              !PEC_ContinuousScatter[InterfaceId]))
    {
        LOG_CRIT("PEC_Packet_Put: invalid source or destination handle\n");
        return PEC_ERROR_BAD_PARAMETER;
    }

    // Token handle
    if (DMAResource_IsValidHandle(Token_Handle))
    {
#ifndef ADAPTER_PEC_REMOVE_BOUNCEBUFFERS
        DMAResource_Record_t * Rec_p =
            DMAResource_Handle2RecordPtr(Token_Handle);
        if (Adapter_DMAResource_IsForeignAllocated(Token_Handle))
        {
            // Bounce buffer required.
            DMAResource_AddrPair_t BounceHostAddr;
            void * HostAddr;
            int dmares;
            DMAResource_Properties_t BounceProperties;

            // used as uint32_t array
            BounceProperties.Alignment  = Adapter_DMAResource_Alignment_Get();
            BounceProperties.Bank       = ADAPTER_PEC_BANK_TOKEN;
            BounceProperties.fCached    = false;
            BounceProperties.Size       = Rec_p->Props.Size;

            HostAddr = Adapter_DMAResource_HostAddr(Token_Handle);

            dmares = DMAResource_Alloc(
                BounceProperties,
                &BounceHostAddr,
                &Rec_p->bounce.Bounce_Handle);

            // bounce buffer handle is stored in the DMA Resource Record
            // of the original buffer, which links the two
            // this will be used when freeing the buffer
            // but also when obtaining the bus address.

            if (dmares != 0)
            {
                LOG_CRIT(
                    "PEC_Packet_Put: "
                    "Failed to alloc bounce buffer (error %d)\n",
                dmares);
                return PEC_ERROR_INTERNAL;   // error!
            }

            LOG_INFO(
                "PEC_Packet_Putr: "
                "Bouncing Token: %p to %p\n",
                Token_Handle,
                Rec_p->bounce.Bounce_Handle);

            // copy the data to the bounce buffer
            memcpy(
                BounceHostAddr.Address_p,
                HostAddr,
                Rec_p->Props.Size);

            Token_Handle = Rec_p->bounce.Bounce_Handle;
        }
        else
        {
            Rec_p->bounce.Bounce_Handle = NULL;
        }
#endif // !ADAPTER_PEC_REMOVE_BOUNCEBUFFERS

#ifdef ADAPTER_PEC_ARMRING_ENABLE_SWAP
        // Convert token data to packet engine endianness format
        DMAResource_SwapEndianness_Set(Token_Handle, true);

        DMAResource_Write32Array(
            Token_Handle,
            0,
            Cmd_p->Token_WordCount,
            Adapter_DMAResource_HostAddr(Token_Handle));
#endif // ADAPTER_PEC_ARMRING_ENABLE_SWAP

        DMAResource_PreDMA(Token_Handle, 0, 0);
    }

    // Source packet handle
#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
    PEC_SGList_GetCapacity(Cmd_p->SrcPkt_Handle, &ParticleCount);

    if (ParticleCount > 0)
    {
        for (i=0; i<ParticleCount; i++)
        {
            PEC_SGList_Read(Cmd_p->SrcPkt_Handle,
                            i,
                            &ParticleHandle,
                            &ParticleSize,
                            &DummyPtr);
            DMARes_Handle =
                Adapter_DMABuf_Handle2DMAResourceHandle(ParticleHandle);
            DMAResource_PreDMA(DMARes_Handle, 0, 0);
        }
    }
    else
#endif
    { // Not a gather packet,
#ifndef ADAPTER_PEC_REMOVE_BOUNCEBUFFERS
        DMAResource_Record_t * Rec_p =
            DMAResource_Handle2RecordPtr(SrcPkt_Handle);
        DMAResource_Record_t * Dst_Rec_p =
            DMAResource_Handle2RecordPtr(DstPkt_Handle);
        if (Adapter_DMAResource_IsForeignAllocated(SrcPkt_Handle) ||
            Adapter_DMAResource_IsForeignAllocated(DstPkt_Handle))
        {
            // Bounce buffer required. Use a single bounce buffer for
            // both the source and the destination packet.
            DMAResource_AddrPair_t BounceHostAddr;
            void * HostAddr;
            int dmares;
            DMAResource_Properties_t BounceProperties;

            // used as uint32_t array
            BounceProperties.Alignment  = Adapter_DMAResource_Alignment_Get();
            BounceProperties.Bank       = ADAPTER_PEC_BANK_PACKET;
            BounceProperties.fCached    = false;
            BounceProperties.Size       = MAX(Rec_p->Props.Size,
                                              Dst_Rec_p->Props.Size);

            HostAddr = Adapter_DMAResource_HostAddr(SrcPkt_Handle);

            dmares = DMAResource_Alloc(
                BounceProperties,
                &BounceHostAddr,
                &Rec_p->bounce.Bounce_Handle);

            // bounce buffer handle is stored in the DMA Resource Record
            // of the original buffer, which links the two
            // this will be used when freeing the buffer
            // but also when obtaining the bus address.

            if (dmares != 0)
            {
                LOG_CRIT(
                    "PEC_Packet_Put: "
                    "Failed to alloc bounce buffer (error %d)\n",
                dmares);
                return PEC_ERROR_INTERNAL;   // error!
            }
            LOG_INFO(
                "PEC_Packet_Putr: "
                "Bouncing Packet: %p to %p\n",
                SrcPkt_Handle,
                Rec_p->bounce.Bounce_Handle);


            // copy the data to the bounce buffer
            memcpy(
                BounceHostAddr.Address_p,
                HostAddr,
                Rec_p->Props.Size);

            DstPkt_Handle = SrcPkt_Handle = Rec_p->bounce.Bounce_Handle;

            Dst_Rec_p->bounce.Bounce_Handle = Rec_p->bounce.Bounce_Handle;
        }
        else
        {
            Rec_p->bounce.Bounce_Handle = NULL;
            Dst_Rec_p->bounce.Bounce_Handle = NULL;
        }
#endif
        DMAResource_PreDMA(SrcPkt_Handle, 0, 0);
    }
    // Destination packet handle, not for continuous scatter.
    if (PEC_ContinuousScatter[InterfaceId])
        return PEC_STATUS_OK;
#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
    PEC_SGList_GetCapacity(Cmd_p->DstPkt_Handle, &ParticleCount);

    if (ParticleCount > 0)
    {
        for (i=0; i<ParticleCount; i++)
        {
            PEC_SGList_Read(Cmd_p->DstPkt_Handle,
                            i,
                            &ParticleHandle,
                            &ParticleSize,
                            &DummyPtr);
            DMARes_Handle =
                Adapter_DMABuf_Handle2DMAResourceHandle(ParticleHandle);
            DMAResource_PreDMA(DMARes_Handle, 0, 0);
        }
    }
    else
#endif
    if (SrcPkt_Handle != DstPkt_Handle)
    {
        // Only if source and destination are distinct.
        // When bounce buffers were used, these are not distinct.
        DMAResource_PreDMA(DstPkt_Handle, 0, 0);
    }
    return PEC_STATUS_OK;
}


/* Adapter_Packet_Finalize
 *
 * Perform PostDMA on all DMA buffers (source, destination and token).
 * Copy the destination packet from the bounce buffer into the final location.
 * Deallocate any bounce buffers (packet and token).
 */
static PEC_Status_t
Adapter_Packet_Finalize(
        DMABuf_Handle_t DMABuf_SrcPkt_Handle,
        DMABuf_Handle_t DMABuf_DstPkt_Handle,
        DMABuf_Handle_t DMABuf_Token_Handle)
{
    DMAResource_Handle_t SrcPkt_Handle, DstPkt_Handle, Token_Handle;

#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
    unsigned int ParticleCount;
    unsigned int i;
    DMABuf_Handle_t ParticleHandle;
    DMAResource_Handle_t DMARes_Handle;
    uint8_t * DummyPtr;
    unsigned int ParticleSize;
#endif

    SrcPkt_Handle =
        Adapter_DMABuf_Handle2DMAResourceHandle(DMABuf_SrcPkt_Handle);
    DstPkt_Handle =
        Adapter_DMABuf_Handle2DMAResourceHandle(DMABuf_DstPkt_Handle);

    if (!DMAResource_IsValidHandle(SrcPkt_Handle) &&
        !DMAResource_IsValidHandle(DstPkt_Handle))
        return PEC_STATUS_OK; // For record invalidation in the Record Cache

    Token_Handle = Adapter_DMABuf_Handle2DMAResourceHandle(DMABuf_Token_Handle);

    // Token Handle.
    if (DMAResource_IsValidHandle(Token_Handle))
    {
#ifndef ADAPTER_PEC_REMOVE_BOUNCEBUFFERS
        DMAResource_Record_t * Rec_p =
            DMAResource_Handle2RecordPtr(Token_Handle);
        if (Rec_p->bounce.Bounce_Handle != NULL)
        {
            // Post DMA and release the bounce buffer.
            DMAResource_PostDMA(Rec_p->bounce.Bounce_Handle, 0, 0);
            DMAResource_Release(Rec_p->bounce.Bounce_Handle);
        }
        else
#endif
        {
            DMAResource_PostDMA(Token_Handle, 0, 0);
        }
    }
    // Destination packet handle

#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
    PEC_SGList_GetCapacity(DMABuf_DstPkt_Handle, &ParticleCount);

    if (ParticleCount > 0)
    {
        for (i=0; i<ParticleCount; i++)
        {
            PEC_SGList_Read(DMABuf_DstPkt_Handle,
                            i,
                            &ParticleHandle,
                            &ParticleSize,
                            &DummyPtr);
            DMARes_Handle =
                Adapter_DMABuf_Handle2DMAResourceHandle(ParticleHandle);
            DMAResource_PostDMA(DMARes_Handle, 0, 0);
        }
    }
    else
#endif
    {
#ifndef ADAPTER_PEC_REMOVE_BOUNCEBUFFERS
        DMAResource_Record_t * Rec_p =
            DMAResource_Handle2RecordPtr(DstPkt_Handle);
        void * HostAddr = Adapter_DMAResource_HostAddr(DstPkt_Handle);
        if (Rec_p->bounce.Bounce_Handle != NULL)
        {
            void * BounceHostAddr =
                Adapter_DMAResource_HostAddr(Rec_p->bounce.Bounce_Handle);
            // Post DMA, copy and release the bounce buffer.
            DMAResource_PostDMA(Rec_p->bounce.Bounce_Handle, 0, 0);

            memcpy( HostAddr, BounceHostAddr, Rec_p->Props.Size);

            DMAResource_Release(Rec_p->bounce.Bounce_Handle);
            SrcPkt_Handle = DstPkt_Handle;
        }
        else
#endif
        {
            DMAResource_PostDMA(DstPkt_Handle, 0, 0);
        }

    }
    // Source packet handle
#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
    PEC_SGList_GetCapacity(DMABuf_SrcPkt_Handle, &ParticleCount);

    if (ParticleCount > 0)
    {
        for (i=0; i<ParticleCount; i++)
        {
            PEC_SGList_Read(DMABuf_SrcPkt_Handle,
                            i,
                            &ParticleHandle,
                            &ParticleSize,
                            &DummyPtr);
            DMARes_Handle =
                Adapter_DMABuf_Handle2DMAResourceHandle(ParticleHandle);
            DMAResource_PostDMA(DMARes_Handle, 0, 0);
        }
    }
    else
#endif
    if (SrcPkt_Handle != DstPkt_Handle)
    {
        // Only if source and destination are distinct.
        // When bounce buffers were used, these are not distinct.
        DMAResource_PostDMA(SrcPkt_Handle, 0, 0);
    }

    return PEC_STATUS_OK;
}


#ifdef ADAPTER_PEC_RPM_EIP202_DEVICE0_ID
/*----------------------------------------------------------------------------
 * AdapterPEC_Resume
 */
static int
AdapterPEC_Resume(void * p)
{
    int InterfaceId = *(int *)p;

    if (InterfaceId < 0 || InterfaceId < ADAPTER_PEC_RPM_EIP202_DEVICE0_ID)
        return -3; // error

    InterfaceId -= ADAPTER_PEC_RPM_EIP202_DEVICE0_ID;

    return Adapter_PECDev_Resume(InterfaceId);
}


/*----------------------------------------------------------------------------
 * AdapterPEC_Suspend
 */
static int
AdapterPEC_Suspend(void * p)
{
    int InterfaceId = *(int *)p;

    if (InterfaceId < 0 || InterfaceId < ADAPTER_PEC_RPM_EIP202_DEVICE0_ID)
        return -3; // error

    InterfaceId -= ADAPTER_PEC_RPM_EIP202_DEVICE0_ID;

    return Adapter_PECDev_Suspend(InterfaceId);
}
#endif


/*----------------------------------------------------------------------------
 * PEC_Capabilities_Get
 */
PEC_Status_t
PEC_Capabilities_Get(
        PEC_Capabilities_t * const Capabilities_p)
{
    return Adapter_PECDev_Capabilities_Get(Capabilities_p);
}


/*----------------------------------------------------------------------------
 * PEC_Init
 */
PEC_Status_t
PEC_Init(
        const unsigned int InterfaceId,
        const PEC_InitBlock_t * const InitBlock_p)
{
    LOG_INFO("\n\t PEC_Init \n");

    if (!InitBlock_p)
        return PEC_ERROR_BAD_PARAMETER;

    if (InterfaceId >= ADAPTER_PEC_DEVICE_COUNT)
        return PEC_ERROR_BAD_PARAMETER;

    Adapter_Lock_CS_Set(&AdapterPEC_InitCS, &AdapterPEC_InitLock);

    if (!Adapter_Lock_CS_Enter(&AdapterPEC_InitCS))
        return PEC_STATUS_BUSY;

    // ensure we init only once
    if (PEC_IsInitialized[InterfaceId])
    {
        Adapter_Lock_CS_Leave(&AdapterPEC_InitCS);
        return PEC_STATUS_OK;
    }
    PEC_ContinuousScatter[InterfaceId] = InitBlock_p->fContinuousScatter;

    // Allocate the Put lock
    AdapterPEC_PutLock[InterfaceId] = Adapter_Lock_Alloc();
    if (AdapterPEC_PutLock[InterfaceId] == NULL)
    {
        LOG_CRIT("PEC_Init: PutLock allocation failed\n");
        Adapter_Lock_CS_Leave(&AdapterPEC_InitCS);
        return PEC_ERROR_INTERNAL;
    }
    Adapter_Lock_CS_Set(&AdapterPEC_PutCS[InterfaceId],
                         AdapterPEC_PutLock[InterfaceId]);

    // Allocate the Get lock
    AdapterPEC_GetLock[InterfaceId] = Adapter_Lock_Alloc();
    if (AdapterPEC_GetLock[InterfaceId] == NULL)
    {
        LOG_CRIT("PEC_Init: GetLock allocation failed\n");
        Adapter_Lock_Free(AdapterPEC_PutLock[InterfaceId]);
        Adapter_Lock_CS_Leave(&AdapterPEC_InitCS);
        return PEC_ERROR_INTERNAL;
    }
    Adapter_Lock_CS_Set(&AdapterPEC_GetCS[InterfaceId],
                         AdapterPEC_GetLock[InterfaceId]);

    ZEROINIT(PEC_Notify[InterfaceId]);

    if (RPM_DEVICE_INIT_START_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID + InterfaceId,
                                    AdapterPEC_Suspend,
                                    AdapterPEC_Resume) != RPM_SUCCESS)
        return PEC_ERROR_INTERNAL;

    // Init the device
    if (Adapter_PECDev_Init(InterfaceId, InitBlock_p) != PEC_STATUS_OK)
    {
        (void)RPM_DEVICE_INIT_STOP_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID + InterfaceId);
        LOG_CRIT("PEC_Init: Adapter_PECDev_Init failed\n");
        Adapter_Lock_Free(AdapterPEC_PutLock[InterfaceId]);
        Adapter_Lock_Free(AdapterPEC_GetLock[InterfaceId]);
        Adapter_Lock_CS_Leave(&AdapterPEC_InitCS);
        return PEC_ERROR_INTERNAL;
    }

    (void)RPM_DEVICE_INIT_STOP_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID + InterfaceId);

    Adapter_SideChannelFIFO[InterfaceId].Size =
        sizeof(Adapter_SideChannelFIFO[InterfaceId].Records) /
        sizeof(Adapter_SideChannelFIFO[InterfaceId].Records[0]);
    Adapter_SideChannelFIFO[InterfaceId].WriteIndex = 0;
    Adapter_SideChannelFIFO[InterfaceId].ReadIndex = 0;

#ifdef ADAPTER_PEC_INTERRUPTS_ENABLE
    // enable the descriptor done interrupt
    LOG_INFO("PEC_Init: Registering interrupt handler\n");

    Adapter_PECDev_SetResultHandler(
            InterfaceId,
            AdapterPEC_InterruptHandlerResultNotify);

    Adapter_PECDev_SetCommandHandler(
            InterfaceId,
            AdapterPEC_InterruptHandlerCommandNotify);
#endif /* ADAPTER_PEC_INTERRUPTS_ENABLE */

    PEC_IsInitialized[InterfaceId] = true;

    LOG_INFO("\n\t PEC_Init done \n");

    Adapter_Lock_CS_Leave(&AdapterPEC_InitCS);

    return PEC_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * PEC_UnInit
 */
PEC_Status_t
PEC_UnInit(
        const unsigned int InterfaceId)
{
    LOG_INFO("\n\t PEC_UnInit \n");

    if (InterfaceId >= ADAPTER_PEC_DEVICE_COUNT)
        return PEC_ERROR_BAD_PARAMETER;

    Adapter_Lock_CS_Set(&AdapterPEC_InitCS, &AdapterPEC_InitLock);

    if (!Adapter_Lock_CS_Enter(&AdapterPEC_InitCS))
        return PEC_STATUS_BUSY;

    // ensure we uninit only once
    if (!PEC_IsInitialized[InterfaceId])
    {
        Adapter_Lock_CS_Leave(&AdapterPEC_InitCS);
        return PEC_STATUS_OK;
    }

    if (!Adapter_Lock_CS_Enter(&AdapterPEC_PutCS[InterfaceId]))
    {
        Adapter_Lock_CS_Leave(&AdapterPEC_InitCS);
        return PEC_STATUS_BUSY;
    }

    if (!Adapter_Lock_CS_Enter(&AdapterPEC_GetCS[InterfaceId]))
    {
        Adapter_Lock_CS_Leave(&AdapterPEC_PutCS[InterfaceId]);
        Adapter_Lock_CS_Leave(&AdapterPEC_InitCS);
        return PEC_STATUS_BUSY;
    }

    if (RPM_DEVICE_UNINIT_START_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID + InterfaceId,
                                true) != RPM_SUCCESS)
    {
        Adapter_Lock_CS_Leave(&AdapterPEC_PutCS[InterfaceId]);
        Adapter_Lock_CS_Leave(&AdapterPEC_InitCS);
        return PEC_ERROR_INTERNAL;
    }

#ifdef ADAPTER_PEC_INTERRUPTS_ENABLE
    Adapter_PECDev_Disable_ResultIRQ(InterfaceId);
    Adapter_PECDev_Disable_CommandIRQ(InterfaceId);
#endif

    Adapter_PECDev_UnInit(InterfaceId);

    PEC_IsInitialized[InterfaceId] = false;

    (void)RPM_DEVICE_UNINIT_STOP_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID + InterfaceId);

    Adapter_Lock_CS_Leave(&AdapterPEC_GetCS[InterfaceId]);
    Adapter_Lock_CS_Leave(&AdapterPEC_PutCS[InterfaceId]);

    // Free Get lock
    Adapter_Lock_Free(Adapter_Lock_CS_Get(&AdapterPEC_GetCS[InterfaceId]));
    Adapter_Lock_CS_Set(&AdapterPEC_GetCS[InterfaceId], Adapter_Lock_NULL);

    // Free Put lock
    Adapter_Lock_Free(Adapter_Lock_CS_Get(&AdapterPEC_PutCS[InterfaceId]));
    Adapter_Lock_CS_Set(&AdapterPEC_PutCS[InterfaceId], Adapter_Lock_NULL);

    LOG_INFO("\n\t PEC_UnInit done \n");

    Adapter_Lock_CS_Leave(&AdapterPEC_InitCS);

    return PEC_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * PEC_SA_Register
 */
PEC_Status_t
PEC_SA_Register(
        const unsigned int InterfaceId,
        DMABuf_Handle_t SA_Handle1,
        DMABuf_Handle_t SA_Handle2,
        DMABuf_Handle_t SA_Handle3)
{
    DMAResource_Handle_t DMAHandle1, DMAHandle2, DMAHandle3;
    PEC_Status_t res;

    LOG_INFO("\n\t PEC_SA_Register \n");

    IDENTIFIER_NOT_USED(InterfaceId);

    DMAHandle1 = Adapter_DMABuf_Handle2DMAResourceHandle(SA_Handle1);
    DMAHandle2 = Adapter_DMABuf_Handle2DMAResourceHandle(SA_Handle2);
    DMAHandle3 = Adapter_DMABuf_Handle2DMAResourceHandle(SA_Handle3);

    // The SA, State Record and ARC4 State Record are arrays of uint32_t.
    // The caller provides them in host-native format.
    // This function converts them to device-native format
    // using DMAResource and in-place operations.

    // Endianness conversion for the 1st SA memory block (Main SA Record)
#ifdef ADAPTER_PEC_ARMRING_ENABLE_SWAP
    {
        DMAResource_Record_t * const Rec_p =
            DMAResource_Handle2RecordPtr(DMAHandle1);

        if (Rec_p == NULL)
            return PEC_ERROR_INTERNAL;

        DMAResource_SwapEndianness_Set(DMAHandle1, true);

        DMAResource_Write32Array(
            DMAHandle1,
            0,
            Rec_p->Props.Size / 4,
            Adapter_DMAResource_HostAddr(DMAHandle1));
    }

    // Endianness conversion for the 2nd SA memory block (State Record)
    if (DMAHandle2 != NULL)
    {
        DMAResource_Record_t * const Rec_p =
            DMAResource_Handle2RecordPtr(DMAHandle2);

        if (Rec_p == NULL)
            return PEC_ERROR_INTERNAL;

        // The 2nd SA memory block can never be a subset of
        // the 1st SA memory block so it is safe to perform
        // the endianness conversion
        DMAResource_SwapEndianness_Set(DMAHandle2, true);

        DMAResource_Write32Array(
            DMAHandle2,
            0,
            Rec_p->Props.Size / 4,
            Adapter_DMAResource_HostAddr(DMAHandle2));
    }

    // Endianness conversion for the 3d SA memory block (ARC4 State Record)
    if (DMAHandle3 != NULL)
    {
        DMAResource_Record_t * const Rec_p =
            DMAResource_Handle2RecordPtr(DMAHandle3);

        if (Rec_p == NULL)
            return PEC_ERROR_INTERNAL;

        // The 3d SA memory block can never be a subset of
        // the 2nd SA memory block.

        // Check if the 3d SA memory block is not a subset of the 1st one
        if (!Adapter_DMAResource_IsSubRangeOf(DMAHandle3, DMAHandle1))
        {
            // The 3d SA memory block is a separate buffer and does not
            // overlap with the 1st SA memory block,
            // so the endianness conversion must be done
            DMAResource_SwapEndianness_Set(DMAHandle3, true);

            DMAResource_Write32Array(
                    DMAHandle3,
                    0,
                    Rec_p->Props.Size / 4,
                    Adapter_DMAResource_HostAddr(DMAHandle3));
        }
    }
#endif // ADAPTER_PEC_ARMRING_ENABLE_SWAP

#ifndef ADAPTER_PEC_REMOVE_BOUNCEBUFFERS
    // Bounce the SA buffers if required
    // Check if the 3d SA memory block is not a subset of the 1st one
    if (DMAHandle3 != NULL &&
        !Adapter_DMAResource_IsSubRangeOf(DMAHandle3, DMAHandle1))
    {
        if (!Adapter_PECRegisterSA_BounceIfRequired(&DMAHandle3))
            return PEC_ERROR_INTERNAL;
    }

    if (!Adapter_PECRegisterSA_BounceIfRequired(&DMAHandle1))
        return PEC_ERROR_INTERNAL;

    if (!Adapter_PECRegisterSA_BounceIfRequired(&DMAHandle2))
        return PEC_ERROR_INTERNAL;
#endif

    res = Adapter_PECDev_SA_Prepare(SA_Handle1, SA_Handle2, SA_Handle3);
    if (res != PEC_STATUS_OK)
    {
        LOG_WARN(
            "PEC_SA_Register: "
            "Adapter_PECDev_PrepareSA returned %d\n",
            res);
        return PEC_ERROR_INTERNAL;
    }

    // now use DMAResource to ensure the engine
    // can read the memory blocks using DMA
    DMAResource_PreDMA(DMAHandle1, 0, 0);     // 0,0 = "entire buffer"

    if (DMAHandle2 != NULL)
        DMAResource_PreDMA(DMAHandle2, 0, 0);

    // Check if the 3d SA memory block is not a subset of the 1st one
    if (DMAHandle3 != NULL &&
        !Adapter_DMAResource_IsSubRangeOf(DMAHandle3, DMAHandle1))
        DMAResource_PreDMA(DMAHandle3, 0, 0);

    LOG_INFO("\n\t PEC_SA_Register done \n");

    return PEC_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * PEC_SA_UnRegister
 */
PEC_Status_t
PEC_SA_UnRegister(
        const unsigned int InterfaceId,
        DMABuf_Handle_t SA_Handle1,
        DMABuf_Handle_t SA_Handle2,
        DMABuf_Handle_t SA_Handle3)
{
    DMAResource_Handle_t SA_Handle[3];
    PEC_Status_t res;
    int i, MaxHandles;

    LOG_INFO("\n\t PEC_SA_UnRegister \n");

    IDENTIFIER_NOT_USED(InterfaceId);

    res = Adapter_PECDev_SA_Remove(SA_Handle1, SA_Handle2, SA_Handle3);
    if (res != PEC_STATUS_OK)
    {
        LOG_CRIT(
            "PEC_SA_UnRegister: "
            "Adapter_PECDev_SA_Remove returned %d\n",
            res);
        return PEC_ERROR_INTERNAL;
    }

    SA_Handle[0] = Adapter_DMABuf_Handle2DMAResourceHandle(SA_Handle1);
    SA_Handle[1] = Adapter_DMABuf_Handle2DMAResourceHandle(SA_Handle2);
    SA_Handle[2] = Adapter_DMABuf_Handle2DMAResourceHandle(SA_Handle3);

    // Check if the 3d SA memory block is not a subset of the 1st one
    if (SA_Handle[0] != NULL &&
        SA_Handle[2] != NULL &&
        Adapter_DMAResource_IsSubRangeOf(SA_Handle[2], SA_Handle[0]))
        MaxHandles = 2;
    else
        MaxHandles = 3;

    for (i = 0; i < MaxHandles; i++)
    {
        if (DMAResource_IsValidHandle(SA_Handle[i]))
        {
            DMAResource_Handle_t DMAHandle = SA_Handle[i];
            void *HostAddr;
            DMAResource_Record_t * Rec_p =
                DMAResource_Handle2RecordPtr(DMAHandle);

            // Check if a bounce buffer is in use
#ifndef ADAPTER_PEC_REMOVE_BOUNCEBUFFERS
            void * OrigHostAddr;
            DMAResource_Record_t * HostRec_p = Rec_p;

            OrigHostAddr = Adapter_DMAResource_HostAddr(DMAHandle);

            if (Adapter_DMAResource_IsForeignAllocated(SA_Handle[i]))
            {
                // Get bounce buffer handle and its record
                DMAHandle = HostRec_p->bounce.Bounce_Handle;
                Rec_p = DMAResource_Handle2RecordPtr(DMAHandle);
            }
#endif /* ADAPTER_PEC_REMOVE_BOUNCEBUFFERS */

            HostAddr = Adapter_DMAResource_HostAddr(DMAHandle);
            // ensure we look at valid engine-written data
            // 0,0 = "entire buffer"
            DMAResource_PostDMA(DMAHandle, 0, 0);

            // convert to host format
            if (Rec_p != NULL)
                DMAResource_Read32Array(
                    DMAHandle,
                    0,
                    Rec_p->Props.Size / 4,
                    HostAddr);

            // copy from bounce buffer to original buffer
#ifndef ADAPTER_PEC_REMOVE_BOUNCEBUFFERS
            if (Adapter_DMAResource_IsForeignAllocated(SA_Handle[i]) &&
                HostRec_p != NULL)
            {
                // copy the data from bounce to original buffer
                memcpy(
                    OrigHostAddr,
                    HostAddr,
                    HostRec_p->Props.Size);

                // free the bounce handle
                DMAResource_Release(HostRec_p->bounce.Bounce_Handle);
                HostRec_p->bounce.Bounce_Handle = NULL;
            }
#endif /* ADAPTER_PEC_REMOVE_BOUNCEBUFFERS */
        } // if handle valid
    } // for

    LOG_INFO("\n\t PEC_SA_UnRegister done\n");

    return PEC_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * PEC_Packet_Put
 */
PEC_Status_t
PEC_Packet_Put(
        const unsigned int InterfaceId,
        const PEC_CommandDescriptor_t * Commands_p,
        const unsigned int CommandsCount,
        unsigned int * const PutCount_p)
{
    unsigned int CmdLp;
    unsigned int PktCnt;
    unsigned int CmdDescriptorCount;
    PEC_Status_t res = 0, res2, PEC_Rc = PEC_STATUS_OK;
    unsigned int FreeSlots;

    LOG_INFO("\n\t PEC_Packet_Put \n");

    if (InterfaceId >= ADAPTER_PEC_DEVICE_COUNT)
        return PEC_ERROR_BAD_PARAMETER;

#ifdef ADAPTER_PEC_STRICT_ARGS
    if (Commands_p == NULL ||
        CommandsCount == 0 ||
        PutCount_p == NULL)
    {
        return PEC_ERROR_BAD_PARAMETER;
    }
#endif

    // initialize the output parameters
    *PutCount_p = 0;

#ifdef ADAPTER_PEC_STRICT_ARGS
    // validate the descriptors
    // (error out before bounce buffer allocation)
    for (CmdLp = 0; CmdLp < CommandsCount; CmdLp++)
        if (Commands_p[CmdLp].Bypass_WordCount > 255)
            return PEC_ERROR_BAD_PARAMETER;
#endif /* ADAPTER_PEC_STRICT_ARGS */

    if (!Adapter_Lock_CS_Enter(&AdapterPEC_PutCS[InterfaceId]))
        return PEC_STATUS_BUSY;

    if (!PEC_IsInitialized[InterfaceId])
    {
        Adapter_Lock_CS_Leave(&AdapterPEC_PutCS[InterfaceId]);
        return PEC_ERROR_BAD_USE_ORDER;
    }

    CmdDescriptorCount = MIN(ADAPTER_PEC_MAX_LOGICDESCR, CommandsCount);
    FreeSlots = 0;
    CmdLp = 0;
    while (CmdLp < CmdDescriptorCount)
    {
        unsigned int j;
        unsigned int count;
        unsigned int NonSGPackets;

#ifndef ADAPTER_PEC_ENABLE_SCATTERGATHER
        NonSGPackets = CmdDescriptorCount - CmdLp;
        // All remaining packets are non-SG.
#else
        unsigned int GatherParticles;
        unsigned int ScatterParticles;
        unsigned int i;

        for (i = CmdLp; i < CmdDescriptorCount; i++)
        {
            PEC_SGList_GetCapacity(Commands_p[i].SrcPkt_Handle,
                                   &GatherParticles);
            if (PEC_ContinuousScatter[InterfaceId])
                ScatterParticles = 0;
            else
                PEC_SGList_GetCapacity(Commands_p[i].DstPkt_Handle,
                                       &ScatterParticles);
            if ( GatherParticles > 0 || ScatterParticles > 0)
                break;
        }
        NonSGPackets = i - CmdLp;

        if (NonSGPackets == 0)
        {
            bool fSuccess;

            if (RPM_DEVICE_IO_START_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID +
                                                                   InterfaceId,
                                          RPM_FLAG_SYNC) != RPM_SUCCESS)
            {
                PEC_Rc = PEC_ERROR_INTERNAL;
                break;
            }

            // First packet found is scatter gather.
            fSuccess = Adapter_PECDev_TestSG(InterfaceId,
                                             GatherParticles,
                                             ScatterParticles);

            (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID +
                                                           InterfaceId,
                                           RPM_FLAG_ASYNC);

            if (!fSuccess)
            {
                PEC_Rc = PEC_ERROR_INTERNAL;
                break;
            }

            // Process a single SG packet in this iteration.
            FreeSlots = 1;
        }
        else
#endif
        if (FreeSlots == 0)
        {
            if (RPM_DEVICE_IO_START_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID +
                                                                   InterfaceId,
                                          RPM_FLAG_SYNC) != RPM_SUCCESS)
            {
                PEC_Rc = PEC_ERROR_INTERNAL;
                break;
            }

            // Allow all non-SG packets to be processed in this iteration,
            // but limited by the number of free slots in the ring(s).
            FreeSlots = Adapter_PECDev_GetFreeSpace(InterfaceId);

            (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID +
                                                           InterfaceId,
                                           RPM_FLAG_ASYNC);

            if (FreeSlots > NonSGPackets)
                FreeSlots = NonSGPackets;

            if (FreeSlots == 0)
                break;
        }

        for (PktCnt=0; PktCnt<FreeSlots; PktCnt++)
        {
            res = Adapter_Packet_Prepare(InterfaceId,
                                         Commands_p + CmdLp + PktCnt);
            if (res != PEC_STATUS_OK)
            {
                LOG_CRIT("%s: Adapter_Packet_Prepare error %d\n", __func__, res);
                PEC_Rc = res;
                break;
            }

            if (!PEC_ContinuousScatter[InterfaceId])
            {
                Adapter_FIFO_Put(&(Adapter_SideChannelFIFO[InterfaceId]),
                                 Commands_p[CmdLp+PktCnt].User_p,
                                 Commands_p[CmdLp+PktCnt].SrcPkt_Handle,
                                 Commands_p[CmdLp+PktCnt].DstPkt_Handle,
                                 Commands_p[CmdLp+PktCnt].Token_Handle,
                                 Commands_p[CmdLp+PktCnt].Bypass_WordCount);
            }
        }

        // RPM_Device_IO_Start() must be called for each successfully submitted
        // packet
        for (j = 0; j < PktCnt; j++)
        {
            // Skipped error checking to reduce code complexity
            (void)RPM_DEVICE_IO_START_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID +
                                                                InterfaceId,
                                            RPM_FLAG_SYNC);
        }

        res2 = Adapter_PECDev_Packet_Put(InterfaceId,
                                         Commands_p + CmdLp,
                                         PktCnt,
                                         &count);
        if (res2 != PEC_STATUS_OK)
        {
            LOG_CRIT("%s: Adapter_PECDev_Packet_Put error %d\n", __func__, res2);
            PEC_Rc = res2;
        }

        FreeSlots -= count;
        *PutCount_p += count;

        if (count <  PktCnt)
        {
            LOG_WARN("PEC_Packet_Put: withdrawing %d prepared packets\n",
                     PktCnt - count);

            for (j = count; j < PktCnt; j++)
            {
                if (!PEC_ContinuousScatter[InterfaceId])
                {
                    Adapter_FIFO_Withdraw(&(Adapter_SideChannelFIFO[InterfaceId]));
                    Adapter_Packet_Finalize(Commands_p[CmdLp + j].SrcPkt_Handle,
                                            Commands_p[CmdLp + j].DstPkt_Handle,
                                            Commands_p[CmdLp + j].Token_Handle);
                }

                // RPM_DEVICE_IO_STOP_MACRO() must be called here for packets
                // which could not be successfully submitted,
                // for example because device queue was full
                (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID +
                                                               InterfaceId,
                                               RPM_FLAG_ASYNC);
            }
            break;
        }

        CmdLp += count;

        if (res != PEC_STATUS_OK || res2 != PEC_STATUS_OK)
        {
            PEC_Rc = PEC_ERROR_INTERNAL;
            break;
        }
    } // while

    LOG_INFO("\n\t PEC_Packet_Put done \n");

    Adapter_Lock_CS_Leave(&AdapterPEC_PutCS[InterfaceId]);

    return PEC_Rc;
}


/*----------------------------------------------------------------------------
 * PEC_Packet_Get
 */
PEC_Status_t
PEC_Packet_Get(
        const unsigned int InterfaceId,
        PEC_ResultDescriptor_t * Results_p,
        const unsigned int ResultsLimit,
        unsigned int * const GetCount_p)
{
    LOG_INFO("\n\t PEC_Packet_Get \n");

    if (InterfaceId >= ADAPTER_PEC_DEVICE_COUNT)
        return PEC_ERROR_BAD_PARAMETER;

#ifdef ADAPTER_PEC_STRICT_ARGS
    if (Results_p == NULL ||
        GetCount_p == NULL ||
        ResultsLimit == 0)
    {
        return PEC_ERROR_BAD_PARAMETER;
    }
#endif

    // initialize the output parameter
    *GetCount_p = 0;

    if (!Adapter_Lock_CS_Enter(&AdapterPEC_GetCS[InterfaceId]))
        return PEC_STATUS_BUSY;

    if (!PEC_IsInitialized[InterfaceId])
    {
        Adapter_Lock_CS_Leave(&AdapterPEC_GetCS[InterfaceId]);
        return PEC_ERROR_BAD_USE_ORDER;
    }

    // read descriptors from PEC device
    {
        PEC_Status_t res;
        unsigned int ResLp;
        unsigned int Limit = MIN(ResultsLimit, ADAPTER_PEC_MAX_LOGICDESCR);
        unsigned int count;
        DMABuf_Handle_t Token_Handle = DMABuf_NULLHandle;

        res=Adapter_PECDev_Packet_Get(InterfaceId,
                                      Results_p,
                                      Limit,
                                      &count);
        if (res != PEC_STATUS_OK)
        {
            LOG_CRIT("PEC_Packet_Get() returned error: %d\n", res);
            Adapter_Lock_CS_Leave(&AdapterPEC_GetCS[InterfaceId]);
            return res;
        }

        for (ResLp = 0; ResLp < count; ResLp++)
        {
            // To help CommandNotifyCB
            if (ResLp == count-1)
                Adapter_MakeCommandNotify_CallBack(InterfaceId);

            (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID +
                                                        InterfaceId,
                                           RPM_FLAG_ASYNC);
            if (PEC_ContinuousScatter[InterfaceId])
            {
                unsigned int i;
                DMABuf_Handle_t DestHandle = DMABuf_NULLHandle;
                DMAResource_Handle_t DMARes_Handle;
                if (Results_p[ResLp].NumParticles == 1)
                {
                    /* No real scatter, can fixup using single destination
                       handle */
                    Adapter_FIFO_Get(&Adapter_SideChannelFIFO[InterfaceId],
                                     NULL,
                                     NULL,
                                     &DestHandle,
                                     NULL,
                                     NULL);
                    DMARes_Handle =
                        Adapter_DMABuf_Handle2DMAResourceHandle(DestHandle);
                    DMAResource_PostDMA(DMARes_Handle, 0, 0);
#ifdef ADAPTER_AUTO_FIXUP
                    IOToken_Fixup(Results_p[ResLp].OutputToken_p,
                                  DestHandle);
#endif
                    Results_p[ResLp].User_p = NULL;
                    Results_p[ResLp].SrcPkt_Handle = DMABuf_NULLHandle;
                    Results_p[ResLp].DstPkt_Handle = DestHandle;
                    Results_p[ResLp].Bypass_WordCount = 0;
                    if (!DMABuf_Handle_IsSame(&Results_p[ResLp].DstPkt_Handle,
                                              &DMABuf_NULLHandle))
                    {
                        Results_p[ResLp].DstPkt_p =
                            Adapter_DMAResource_HostAddr(
                                Adapter_DMABuf_Handle2DMAResourceHandle(
                                    Results_p[ResLp].DstPkt_Handle));
                    }
                    else
                    {
                        Results_p[ResLp].DstPkt_p = NULL;
                    }
                }
                else
                {
#if defined(ADAPTER_PEC_ENABLE_SCATTERGATHER) && defined(ADAPTER_AUTO_FIXUP)
                    DMAResource_Record_t * Rec_p;
                    PEC_Status_t PEC_Rc;
                    DMABuf_Handle_t Fixup_SGList; /* scatter list for Fixup in continuous scatter mode */
                    PEC_Rc = PEC_SGList_Create(Results_p[ResLp].NumParticles,
                                               &Fixup_SGList);
#endif
                    for (i = 0; i < Results_p[ResLp].NumParticles; i++)
                    {
                        Adapter_FIFO_Get(&Adapter_SideChannelFIFO[InterfaceId],
                                         NULL,
                                         NULL,
                                         &DestHandle,
                                         NULL,
                                     NULL);
                        DMARes_Handle =
                            Adapter_DMABuf_Handle2DMAResourceHandle(DestHandle);
                        DMAResource_PostDMA(DMARes_Handle, 0, 0);
#if defined(ADAPTER_PEC_ENABLE_SCATTERGATHER) && defined(ADAPTER_AUTO_FIXUP)
                        if (PEC_Rc == PEC_STATUS_OK)
                        {
                            Rec_p = DMAResource_Handle2RecordPtr(DMARes_Handle);

                            PEC_SGList_Write(Fixup_SGList,
                                             i,
                                             DestHandle,
                                             Rec_p->Props.Size);
                        }
#endif
                    }
#if defined(ADAPTER_PEC_ENABLE_SCATTERGATHER) && defined(ADAPTER_AUTO_FIXUP)
                    if (PEC_Rc == PEC_STATUS_OK)
                    {
                        IOToken_Fixup(Results_p[ResLp].OutputToken_p,
                              Fixup_SGList);
                        PEC_SGList_Destroy(Fixup_SGList);
                    }

#endif
                    Results_p[ResLp].User_p = NULL;
                    Results_p[ResLp].SrcPkt_Handle = DMABuf_NULLHandle;
                    Results_p[ResLp].DstPkt_Handle = DMABuf_NULLHandle;
                    Results_p[ResLp].Bypass_WordCount = 0;
                }
            }
            else
            {
                Adapter_FIFO_Get(&(Adapter_SideChannelFIFO[InterfaceId]),
                                 &(Results_p[ResLp].User_p),
                                 &(Results_p[ResLp].SrcPkt_Handle),
                                 &(Results_p[ResLp].DstPkt_Handle),
                                 &Token_Handle,
                                 &(Results_p[ResLp].Bypass_WordCount));

                Adapter_Packet_Finalize(Results_p[ResLp].SrcPkt_Handle,
                                    Results_p[ResLp].DstPkt_Handle,
                                    Token_Handle);
#ifdef ADAPTER_AUTO_FIXUP
                IOToken_Fixup(Results_p[ResLp].OutputToken_p,
                              Results_p[ResLp].DstPkt_Handle);
#endif

                if (!DMABuf_Handle_IsSame(&Results_p[ResLp].DstPkt_Handle,
                                          &DMABuf_NULLHandle))
                {
                    Results_p[ResLp].DstPkt_p =
                        Adapter_DMAResource_HostAddr(
                            Adapter_DMABuf_Handle2DMAResourceHandle(
                                Results_p[ResLp].DstPkt_Handle));
                }
                else
                {
                    Results_p[ResLp].DstPkt_p = NULL;
                }
            }
            *GetCount_p += 1;
        } // for
    }

    LOG_INFO("\n\t PEC_Packet_Get done \n");

    Adapter_Lock_CS_Leave(&AdapterPEC_GetCS[InterfaceId]);

    return PEC_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * PEC_CD_Control_Write
 *
 * Write the Control1 and Control2 engine-specific fields in the
 * Command Descriptor The other fields (such as SrcPkt_ByteCount and
 * Bypass_WordCount must have been filled in already.
 *
 * Command_p (input, output)
 *     Command descriptor whose Control1 and Control2 fields must be filled in.
 *
 * PacketParams_p (input)
 *     Per-packet parameters.
 *
 * This function is not implemented for all engine types.
 */
PEC_Status_t
PEC_CD_Control_Write(
    PEC_CommandDescriptor_t *Command_p,
    const PEC_PacketParams_t *PacketParams_p)
{
    return Adapter_PECDev_CD_Control_Write(Command_p, PacketParams_p);
}


/*----------------------------------------------------------------------------
 * PEC_RD_Status_Read
 */
PEC_Status_t
PEC_RD_Status_Read(
        const PEC_ResultDescriptor_t * const Result_p,
        PEC_ResultStatus_t * const ResultStatus_p)
{
    return Adapter_PECDev_RD_Status_Read(Result_p, ResultStatus_p);
}


/*----------------------------------------------------------------------------
 * PEC_CommandNotify_Request
 */
PEC_Status_t
PEC_CommandNotify_Request(
        const unsigned int InterfaceId,
        PEC_NotifyFunction_t CBFunc_p,
        const unsigned int CommandsCount)
{
    unsigned int PacketSlotsEmptyCount;

    LOG_INFO("\n\t PEC_CommandNotify_Request \n");

    if (InterfaceId >= ADAPTER_PEC_DEVICE_COUNT)
        return PEC_ERROR_BAD_PARAMETER;

    if (CBFunc_p == NULL ||
        CommandsCount == 0 ||
        CommandsCount > ADAPTER_PEC_MAX_PACKETS)
    {
        return PEC_ERROR_BAD_PARAMETER;
    }

    if (!PEC_IsInitialized[InterfaceId])
        return PEC_ERROR_BAD_USE_ORDER;

    if (RPM_DEVICE_IO_START_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID + InterfaceId,
                                  RPM_FLAG_SYNC) != RPM_SUCCESS)
        return PEC_ERROR_INTERNAL;

    PacketSlotsEmptyCount = Adapter_PECDev_GetFreeSpace(InterfaceId);

    (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID + InterfaceId,
                                   RPM_FLAG_ASYNC);

    if (CommandsCount <= PacketSlotsEmptyCount)
    {
        LOG_INFO(
            "PEC_CommandNotify_Request: "
            "Invoking command notify callback immediately\n");

        CBFunc_p();
    }
    else
    {
        PEC_Notify[InterfaceId].CommandsCount = CommandsCount;
        PEC_Notify[InterfaceId].CommandNotifyCB_p = CBFunc_p;

#ifdef ADAPTER_PEC_INTERRUPTS_ENABLE
        if (RPM_DEVICE_IO_START_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID + InterfaceId,
                                      RPM_FLAG_SYNC) != RPM_SUCCESS)
            return PEC_ERROR_INTERNAL;

        /* Note that space for new commands may have become available before
         * the call to PEC_CommandNotify_Request and the associated interrupt
         * may already be pending. In this case the interrupt will occur
         * immediately.
         */
        Adapter_PECDev_Enable_CommandIRQ(InterfaceId);

        (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID + InterfaceId,
                                       RPM_FLAG_ASYNC);
#endif /* ADAPTER_PEC_INTERRUPTS_ENABLE */
    }

    LOG_INFO("\n\t PEC_CommandNotify_Request done \n");

    return PEC_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * PEC_ResultNotify_Request
 */
PEC_Status_t
PEC_ResultNotify_Request(
        const unsigned int InterfaceId,
        PEC_NotifyFunction_t CBFunc_p,
        const unsigned int ResultsCount)
{
    LOG_INFO("\n\t PEC_ResultNotify_Request \n");

    if (InterfaceId >= ADAPTER_PEC_DEVICE_COUNT)
        return PEC_ERROR_BAD_PARAMETER;

    if (CBFunc_p == NULL ||
        ResultsCount == 0 ||
        ResultsCount > ADAPTER_PEC_MAX_PACKETS)
    {
        return PEC_ERROR_BAD_PARAMETER;
    }

    if (!PEC_IsInitialized[InterfaceId])
        return PEC_ERROR_BAD_USE_ORDER;

    // install it
    PEC_Notify[InterfaceId].ResultsCount = ResultsCount;
    PEC_Notify[InterfaceId].ResultNotifyCB_p = CBFunc_p;

#ifdef ADAPTER_PEC_INTERRUPTS_ENABLE
    if (RPM_DEVICE_IO_START_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID + InterfaceId,
                                  RPM_FLAG_SYNC) != RPM_SUCCESS)
        return PEC_ERROR_INTERNAL;

    /* Note that results may have become available before the call
       to PEC_ResultNotify_Request and the associated interrupts may already
       be pending. In this case the interrupt will occur immediately.
     */
    Adapter_PECDev_Enable_ResultIRQ(InterfaceId);

    (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_PEC_RPM_EIP202_DEVICE0_ID + InterfaceId,
                                   RPM_FLAG_ASYNC);
#endif /* ADAPTER_PEC_INTERRUPTS_ENABLE */

    LOG_INFO("\n\t PEC_ResultNotify_Request done\n");

    return PEC_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * PEC_Scatter_Preload
 */
PEC_Status_t
PEC_Scatter_Preload(
        const unsigned int InterfaceId,
        DMABuf_Handle_t * Handles_p,
        const unsigned int HandlesCount,
        unsigned int * const AcceptedCount_p)
{
    PEC_Status_t rc;
    unsigned int i;
    unsigned int HandlesToUse,ri,wi;
        LOG_INFO("\n\t PEC_Scatter_Preload\n");

    if (InterfaceId >= ADAPTER_PEC_DEVICE_COUNT)
        return PEC_ERROR_BAD_PARAMETER;

    if (!Adapter_Lock_CS_Enter(&AdapterPEC_PutCS[InterfaceId]))
        return PEC_STATUS_BUSY;

    if (!PEC_IsInitialized[InterfaceId] || !PEC_ContinuousScatter[InterfaceId])
    {
        Adapter_Lock_CS_Leave(&AdapterPEC_PutCS[InterfaceId]);
        return PEC_ERROR_BAD_USE_ORDER;
    }
    ri = Adapter_SideChannelFIFO[InterfaceId].ReadIndex;
    wi = Adapter_SideChannelFIFO[InterfaceId].WriteIndex;
    // Compute the number of free slots in the ring from the read/write index
    // of the side channel FIFO (which is larger than the ring)
    if (wi >= ri)
        HandlesToUse = ADAPTER_PEC_MAX_PACKETS - 1 - wi + ri ;
    else
        HandlesToUse = ADAPTER_PEC_MAX_PACKETS - 1
            - Adapter_SideChannelFIFO[InterfaceId].Size - wi + ri;
    if (HandlesToUse > ADAPTER_PEC_MAX_LOGICDESCR)
        HandlesToUse = ADAPTER_PEC_MAX_LOGICDESCR;
    if (HandlesToUse > HandlesCount)
        HandlesToUse = HandlesCount;
    for (i=0; i < HandlesToUse; i++)
    {
        DMAResource_Handle_t DMARes_Handle =
            Adapter_DMABuf_Handle2DMAResourceHandle(Handles_p[i]);
        DMAResource_PreDMA(DMARes_Handle, 0, 0);
        Adapter_FIFO_Put(&Adapter_SideChannelFIFO[InterfaceId],
                         NULL,
                         DMABuf_NULLHandle,
                         Handles_p[i],
                         DMABuf_NULLHandle,
                         0);
    }
    rc = Adapter_PECDev_Scatter_Preload(InterfaceId,
                                        Handles_p,
                                        HandlesToUse);

    *AcceptedCount_p = HandlesToUse;
    Adapter_Lock_CS_Leave(&AdapterPEC_PutCS[InterfaceId]);
    return rc;
}


/*----------------------------------------------------------------------------
 * PEC_Scatter_PreloadNotify_Request
 */
PEC_Status_t
PEC_Scatter_PreloadNotify_Request(
        const unsigned int InterfaceId,
        PEC_NotifyFunction_t CBFunc_p,
        const unsigned int ConsumedCount)
{
    IDENTIFIER_NOT_USED(InterfaceId);
    IDENTIFIER_NOT_USED(CBFunc_p);
    IDENTIFIER_NOT_USED(ConsumedCount);

    return PEC_ERROR_NOT_IMPLEMENTED;
}


/*----------------------------------------------------------------------------
 * PEC_Put_Dump
 */
void
PEC_Put_Dump(
        const unsigned int InterfaceId,
        const unsigned int FirstSlotId,
        const unsigned int LastSlotId,
        const bool fDumpRDRAdmin,
        const bool fDumpRDRCache)
{
    Adapter_PECDev_Put_Dump(InterfaceId,
                            FirstSlotId,
                            LastSlotId,
                            fDumpRDRAdmin,
                            fDumpRDRCache);
}




/*----------------------------------------------------------------------------
 * PEC_Get_Dump
 */
void
PEC_Get_Dump(
        const unsigned int InterfaceId,
        const unsigned int FirstSlotId,
        const unsigned int LastSlotId,
        const bool fDumpRDRAdmin,
        const bool fDumpRDRCache)
{
    Adapter_PECDev_Get_Dump(InterfaceId,
                            FirstSlotId,
                            LastSlotId,
                            fDumpRDRAdmin,
                            fDumpRDRCache);
}


#endif /* ADAPTER_PE_MODE_DHM */


/* end of file adapter_pec_dma.c */
