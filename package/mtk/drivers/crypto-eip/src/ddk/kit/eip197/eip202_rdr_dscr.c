/* eip202_rdr_dscr.c
 *
 * EIP-202 Ring Control Driver Library
 * 1) Descriptor I/O Driver Library API implementation
 * 2) Internal Result Descriptor interface implementation
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : ddk197                                                   */
/*   Version       : 5.6.1                                                    */
/*   Configuration : DDK-197-GPL                                              */
/*                                                                            */
/*   Date          : 2022-Dec-16                                              */
/*                                                                            */
/* Copyright (c) 2008-2022 by Rambus, Inc. and/or its subsidiaries.           */
/*                                                                            */
/* This program is free software: you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation, either version 2 of the License, or          */
/* any later version.                                                         */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               */
/* GNU General Public License for more details.                               */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program. If not, see <http://www.gnu.org/licenses/>.       */
/* -------------------------------------------------------------------------- */

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// Descriptor I/O Driver Library API implementation
#include "eip202_rdr.h"

// Internal Result Descriptor interface
#include "eip202_rdr_dscr.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip202_ring.h"

// EIP-202 Ring Control Driver Library Internal interfaces
#include "eip202_ring_internal.h"
#include "eip202_rdr_level0.h"         // EIP-202 Level 0 macros
#include "eip202_rdr_fsm.h"             // RDR State machine
#include "eip202_rd_format.h"           // RD Format API

// Driver Framework Basic Definitions API
#include "basic_defs.h"                // IDENTIFIER_NOT_USED, bool, uint32_t

// Driver Framework DMA Resource API
#include "dmares_types.h"         // types of the DMA resource API
#include "dmares_rw.h"            // read/write of the DMA resource API.

// Standard IOToken API
#include "iotoken.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// EIP-202 HW limit for the number of packets to acknowledge at once
#define EIP202_RING_MAX_RD_PACKET_COUNT      127


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP202Lib_RDR_Prepared_FillLevel_Get
 */
static EIP202_Ring_Error_t
EIP202Lib_RDR_Prepared_FillLevel_Get(
        const Device_Handle_t Device,
        volatile EIP202_RDR_True_IOArea_t * const TrueIOArea_p,
        unsigned int * const FillLevelDscrCount_p)
{
    unsigned int RDWordCount;
    EIP202_Ring_Error_t rv;

    EIP202_RDR_PREP_COUNT_RD(Device, (uint32_t*)&RDWordCount);

    // Remain in the current state
    rv = EIP202_RDR_State_Set((volatile EIP202_RDR_State_t*)&TrueIOArea_p->State,
                             (EIP202_RDR_State_t)TrueIOArea_p->State);
    if(rv != EIP202_RING_NO_ERROR)
        return EIP202_RING_ILLEGAL_IN_STATE;

    *FillLevelDscrCount_p = RDWordCount /
                            (unsigned int)TrueIOArea_p->DescOffsWordCount;

    if(RDWordCount < (unsigned int)TrueIOArea_p->RingSizeWordCount)
        (*FillLevelDscrCount_p)++;

    return EIP202_RING_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP202Lib_RDR_Processed_FillLevel_Finalize
 */
static EIP202_Ring_Error_t
EIP202Lib_RDR_Processed_FillLevel_Finalize(
        const unsigned int RDWordCount,
        const unsigned int PktCount,
        volatile EIP202_RDR_True_IOArea_t * const TrueIOArea_p,
        unsigned int * const FillLevelDscrCount_p,
        unsigned int * const FillLevelPktCount_p)
{
#ifdef EIP202_RING_DEBUG_FSM
    EIP202_Ring_Error_t rv;

    if(RDWordCount == 0)
        // CD Ring is empty
        rv = EIP202_RDR_State_Set((volatile EIP202_RDR_State_t*)&TrueIOArea_p->State,
                                EIP202_RDR_STATE_INITIALIZED);
    else if(RDWordCount > 0 &&
            RDWordCount < (unsigned int)TrueIOArea_p->RingSizeWordCount)
        // CD Ring is free
        rv = EIP202_RDR_State_Set((volatile EIP202_RDR_State_t*)&TrueIOArea_p->State,
                                 EIP202_RDR_STATE_FREE);
    else if(RDWordCount == (unsigned int)TrueIOArea_p->RingSizeWordCount)
        // CD Ring is full
        rv = EIP202_RDR_State_Set((volatile EIP202_RDR_State_t*)&TrueIOArea_p->State,
                                 EIP202_RDR_STATE_FULL);
    else
        rv = EIP202_RING_ILLEGAL_IN_STATE;

    if(rv != EIP202_RING_NO_ERROR)
        return EIP202_RING_ILLEGAL_IN_STATE;
#endif

    *FillLevelDscrCount_p = RDWordCount /
                            (unsigned int)TrueIOArea_p->DescOffsWordCount;

    *FillLevelPktCount_p = PktCount;

    return EIP202_RING_NO_ERROR;
}


/*----------------------------------------------------------------------------
  Internal Result Descriptor interface
  ---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * EIP202_RDR_WriteCB
 */
int
EIP202_RDR_WriteCB(
        void * const CallbackParam1_p,
        const int CallbackParam2,
        const unsigned int WriteIndex,
        const unsigned int WriteCount,
        const unsigned int TotalWriteLimit,
        const void * Descriptors_p,
        const int DescriptorCount,
        const unsigned int DescriptorSkipCount)
{
    Device_Handle_t Device;
    unsigned int i, DescOffsetWordCount;
    unsigned int nWritten = 0;
    volatile EIP202_RDR_True_IOArea_t * const TrueIOArea_p =
                                            RDRIOAREA(CallbackParam1_p);
    if(CallbackParam1_p == NULL || Descriptors_p == NULL)
        return -1;

    IDENTIFIER_NOT_USED(CallbackParam2);
    IDENTIFIER_NOT_USED(DescriptorCount);
    IDENTIFIER_NOT_USED(TotalWriteLimit);

    Device = TrueIOArea_p->Device;

    DescOffsetWordCount = (unsigned int)TrueIOArea_p->DescOffsWordCount;

    // Write descriptors to RDR
    for(i = WriteIndex; i < WriteIndex + WriteCount; i++)
    {
        EIP202_Prepared_Write(
            TrueIOArea_p->RingHandle,
            i * DescOffsetWordCount,
            ((const EIP202_ARM_PreparedDescriptor_t *)Descriptors_p) +
                    DescriptorSkipCount + nWritten);

        nWritten++;
    }

    if (nWritten > 0)
    {
        // Call PreDMA to prepared descriptors in Ring DMA buffer for handover
        // to the Device (the EIP-202 DMA Master)
        DMAResource_PreDMA(TrueIOArea_p->RingHandle,
                           WriteIndex *
                              DescOffsetWordCount *
                               (unsigned int)sizeof(uint32_t),
                           nWritten *
                             DescOffsetWordCount *
                               (unsigned int)sizeof(uint32_t));

        // CDS point: hand over written Prepared Descriptors to the Device
        EIP202_RDR_PREP_COUNT_WR(Device,
                                 (uint16_t)(nWritten * DescOffsetWordCount),
                                 false);
    }

    return (int)nWritten;
}


/*----------------------------------------------------------------------------
 * EIP202_RDR_ReadCB
 */
int
EIP202_RDR_ReadCB(
        void * const CallbackParam1_p,
        const int CallbackParam2,
        const unsigned int ReadIndex,
        const unsigned int ReadLimit,
        void * Descriptors_p,
        const unsigned int DescriptorSkipCount)
{
    Device_Handle_t Device;
    unsigned int i, DescOffsetWordCount;
    bool fGotDescriptor, fLastSegment = false, fFirstSegment = false;
    unsigned int GotDscrCount = 0, GotPktCount = 0;
    volatile EIP202_RDR_True_IOArea_t * const TrueIOArea_p =
                                       RDRIOAREA(CallbackParam1_p);

    if(CallbackParam1_p == NULL || Descriptors_p == NULL)
        return -1;

    IDENTIFIER_NOT_USED(CallbackParam2);

    Device = TrueIOArea_p->Device;

    DescOffsetWordCount = (unsigned int )TrueIOArea_p->DescOffsWordCount;

    // Read descriptors from RDR
    for(i = ReadIndex; i < ReadIndex + ReadLimit; i++)
    {
        EIP202_ARM_ResultDescriptor_t * CurrentResultDesc_p =
            ((EIP202_ARM_ResultDescriptor_t *)Descriptors_p) +
                    DescriptorSkipCount + GotDscrCount;

#define EIP202_RING_RDR_ALL_DESCRIPTORS_TO_GET_DONE             \
         (TrueIOArea_p->AcknowledgedRDCount + GotDscrCount >=  \
              TrueIOArea_p->RDToGetCount)

// This can be true only if the PktRequestedCount parameter in
// the EIP202_RDR_Descriptor_Get() function is set to a non-zero value
#define EIP202_RING_RDR_ALL_PACKETS_TO_GET_DONE                 \
         (TrueIOArea_p->PktToGetCount > 0 &&                   \
          TrueIOArea_p->AcknowledgedPktCount + GotPktCount >=  \
              TrueIOArea_p->PktToGetCount)

        // Stop reading the descriptors if all the requested
        // descriptors and packet chains have been read
        if(EIP202_RING_RDR_ALL_PACKETS_TO_GET_DONE)
        {
            if(EIP202_RING_RDR_ALL_DESCRIPTORS_TO_GET_DONE)
                break; // for
        }

        // Call PostDMA before reading descriptors from
        // the EIP-202 DMA Master
        DMAResource_PostDMA(TrueIOArea_p->RingHandle,
                            i * DescOffsetWordCount *
                              (unsigned int)sizeof(uint32_t),
                            DescOffsetWordCount *
                              (unsigned int)sizeof(uint32_t));

        // Check if a processed result descriptor is received
#if defined(EIP202_RDR_OWNERSHIP_WORD_ENABLE)
        {
            uint32_t OwnershipWord =
                        DMAResource_Read32(TrueIOArea_p->RingHandle,
                                           i * DescOffsetWordCount +
                                                 DescOffsetWordCount - 1);
            fGotDescriptor =
                    (OwnershipWord == EIP202_RDR_OWNERSHIP_WORD_PATTERN);
#ifdef EIP202_RING_BUS_KEEPALIVE_WORKAROUND
            // Read from the device to solve a keep-alive problem in some
            // bus environments.
            Device_Read32(Device,0);
#endif
        }
#elif defined(EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS)
        CurrentResultDesc_p->Token_p[IOToken_OutMarkOffset_Get()] =
                DMAResource_Read32(TrueIOArea_p->RingHandle,
                                   i * DescOffsetWordCount +
                                   TrueIOArea_p->TokenOffsetWordCount +
                                   IOToken_OutMarkOffset_Get());
        fGotDescriptor = (IOToken_Mark_Check(CurrentResultDesc_p->Token_p) == 0);
#else
        fGotDescriptor = true; // according to EIP202_RDR_PROC_COUNT_RD()
#endif

        if (fGotDescriptor)
        {
            // Read Result Descriptor
            EIP202_ReadDescriptor(CurrentResultDesc_p,
                                  TrueIOArea_p->RingHandle,
                                  i * DescOffsetWordCount,
                                  DescOffsetWordCount,
                                  TrueIOArea_p->TokenOffsetWordCount,
                                  &fLastSegment,
                                  &fFirstSegment);

            // Stop reading the descriptors if all the requested
            // packet chains have been read and a new processed packet descriptor
            // chain is detected
            if(fFirstSegment && EIP202_RING_RDR_ALL_PACKETS_TO_GET_DONE)
                break; // for

            if (fFirstSegment)
                TrueIOArea_p->PacketFound = true;

            GotDscrCount++;

            if(TrueIOArea_p->PacketFound && fLastSegment)
            {
                GotPktCount++;
                TrueIOArea_p->PacketFound = false;
            }

#if defined(EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS) || \
    defined(EIP202_RDR_OWNERSHIP_WORD_ENABLE)
            // Clear this descriptor
            EIP202_ClearDescriptor(CurrentResultDesc_p,
                                   TrueIOArea_p->RingHandle,
                                   i * DescOffsetWordCount,
                                   TrueIOArea_p->TokenOffsetWordCount,
                                   DescOffsetWordCount);

            // Ensure next PostDMA does not undo the clear operation above
            DMAResource_PreDMA(
                TrueIOArea_p->RingHandle,
                i * DescOffsetWordCount * (unsigned int)sizeof(uint32_t),
                DescOffsetWordCount * (unsigned int)sizeof(uint32_t));
#endif
        }
        else
        {
            // The fGotDescriptor is set in EIP202_ReadDescriptor() and
            // depends on the Application ID field in the result descriptor
            // when EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS is defined.
            // In case of a packet descriptor chain the Engine writes the
            // Application ID field for the last segment descriptor only
            // but we need to acknowledge all the descriptors of the chain
            break; // for
        }
    } // for

    // Check if there are processed result descriptors and packets
    // that must be acknowledged
    if (GotDscrCount > 0)
    {
        unsigned int NewGotPktCount;

#if EIP202_RING_RD_INTERRUPTS_PER_PACKET_FLAG != 1
        GotPktCount = 0;
#endif

        // EIP-202 HW limits the number of packets to acknowledge at once to
        // EIP202_RING_MAX_RD_PACKET_COUNT packets
        NewGotPktCount = MIN(GotPktCount, EIP202_RING_MAX_RD_PACKET_COUNT);

        // CDS point: hand over read Result Descriptors to the Device
        EIP202_RDR_PROC_COUNT_WR(
                Device,
                (uint16_t)(GotDscrCount * DescOffsetWordCount),
                (uint8_t)NewGotPktCount,
                false);
    }

    // Update acknowledged packets counter
    TrueIOArea_p->AcknowledgedPktCount += GotPktCount;

    // Update acknowledged descriptors counter
    TrueIOArea_p->AcknowledgedRDCount += GotDscrCount;

    return (int)GotDscrCount;
}


/*----------------------------------------------------------------------------
 * EIP202_RDR_StatusCB
 */
int
EIP202_RDR_StatusCB(
        void * const CallbackParam1_p,
        const int CallbackParam2,
        int * const DeviceReadPos_p)
{
    IDENTIFIER_NOT_USED(CallbackParam1_p);
    IDENTIFIER_NOT_USED(CallbackParam2);

    *DeviceReadPos_p = -1;  // not used

    return 0;
}


/*----------------------------------------------------------------------------
  Descriptor I/O Driver Library API implementation
  ---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * EIP202_RDR_FillLevel_Get
 */
EIP202_Ring_Error_t
EIP202_RDR_FillLevel_Get(
        EIP202_Ring_IOArea_t * const IOArea_p,
        unsigned int * FillLevelDscrCount_p)
{
    int FillLevel;
    volatile EIP202_RDR_True_IOArea_t * const TrueIOArea_p = RDRIOAREA(IOArea_p);

    EIP202_RING_CHECK_POINTER(IOArea_p);
    EIP202_RING_CHECK_POINTER(FillLevelDscrCount_p);

    FillLevel = RingHelper_FillLevel_Get(
            (volatile RingHelper_t*)&TrueIOArea_p->RingHelper);

    if(FillLevel < 0)
        return EIP202_RING_UNSUPPORTED_FEATURE_ERROR;
    else
    {
        *FillLevelDscrCount_p = (unsigned int)FillLevel;
        return EIP202_RING_NO_ERROR;
    }
}


/*----------------------------------------------------------------------------
 * EIP202_RDR_Prepared_FillLevel_Get
 */
EIP202_Ring_Error_t
EIP202_RDR_Prepared_FillLevel_Get(
        EIP202_Ring_IOArea_t * const IOArea_p,
        unsigned int * FillLevelDscrCount_p)
{
    Device_Handle_t Device;
    volatile EIP202_RDR_True_IOArea_t * const TrueIOArea_p = RDRIOAREA(IOArea_p);

    EIP202_RING_CHECK_POINTER(IOArea_p);
    EIP202_RING_CHECK_POINTER(FillLevelDscrCount_p);

    Device = TrueIOArea_p->Device;

    return EIP202Lib_RDR_Prepared_FillLevel_Get(Device,
                                               TrueIOArea_p,
                                               FillLevelDscrCount_p);
}


/*----------------------------------------------------------------------------
 * EIP202_RDR_Write_Prepared_ControlWord
 *
 * This helper function returns the control word that can be written to
 * the logical prepared descriptor.
 *
 * This function is re-entrant.
 *
 */
uint32_t
EIP202_RDR_Write_Prepared_ControlWord(
        const EIP202_RDR_Prepared_Control_t * const  PreparedCtrl_p)
{
    return EIP202_RD_Make_ControlWord(PreparedCtrl_p->ExpectedResultWordCount,
                                     PreparedCtrl_p->PrepSegmentByteCount,
                                     PreparedCtrl_p->fFirstSegment,
                                     PreparedCtrl_p->fLastSegment);
}


/*----------------------------------------------------------------------------
 * EIP202_RDR_Descriptor_Prepare
 */
EIP202_Ring_Error_t
EIP202_RDR_Descriptor_Prepare(
        EIP202_Ring_IOArea_t * const IOArea_p,
        const EIP202_ARM_PreparedDescriptor_t * PreparedDscr_p,
        const unsigned int DscrRequestedCount,
        unsigned int * const DscrPreparedCount_p,
        unsigned int * FillLevelDscrCount_p)
{
    int res, FillLevel;
    unsigned int RDWordCount, RDFreeCount, DscrNewRequestedCount;
    volatile EIP202_RDR_True_IOArea_t * const TrueIOArea_p = RDRIOAREA(IOArea_p);

    EIP202_RING_CHECK_POINTER(IOArea_p);
    EIP202_RING_CHECK_POINTER(PreparedDscr_p);
    EIP202_RING_CHECK_POINTER(DscrPreparedCount_p);
    EIP202_RING_CHECK_POINTER(FillLevelDscrCount_p);

    // Check how many more descriptors can be added (prepared) to RDR
    FillLevel = RingHelper_FillLevel_Get(
            (volatile RingHelper_t*)&TrueIOArea_p->RingHelper);
    if(FillLevel < 0)
        return EIP202_RING_ARGUMENT_ERROR; // Error, RDR admin corrupted

    // Check if RDR is full
    RDWordCount = ((unsigned int)FillLevel *
            (unsigned int)TrueIOArea_p->DescOffsWordCount);
    if(RDWordCount == (unsigned int)TrueIOArea_p->RingSizeWordCount)
    {
        // RD Ring is full
        *FillLevelDscrCount_p = (unsigned int)FillLevel;
        *DscrPreparedCount_p = 0;

        // Remain in the current state
        return EIP202_RDR_State_Set((volatile EIP202_RDR_State_t*)&TrueIOArea_p->State,
                                   (EIP202_RDR_State_t)TrueIOArea_p->State);
    }

    // Calculate the maximum number of descriptors that can be added to RDR
    RDFreeCount =
            ((unsigned int)TrueIOArea_p->RingSizeWordCount - RDWordCount) /
                    (unsigned int)TrueIOArea_p->DescOffsWordCount;

    DscrNewRequestedCount = MIN(RDFreeCount, DscrRequestedCount);

    res = RingHelper_Put((volatile RingHelper_t*)&TrueIOArea_p->RingHelper,
                         PreparedDscr_p,
                         (int)DscrNewRequestedCount);
    if(res >= 0)
        *DscrPreparedCount_p = (unsigned int)res;
    else
        return EIP202_RING_ARGUMENT_ERROR;

    // Get the current RDR fill level
    FillLevel = RingHelper_FillLevel_Get(
            (volatile RingHelper_t*)&TrueIOArea_p->RingHelper);
    if(FillLevel < 0)
        return EIP202_RING_ARGUMENT_ERROR; // Error, RDR admin corrupted
    else
    {
        *FillLevelDscrCount_p = (unsigned int)FillLevel;
        return EIP202_RING_NO_ERROR;
    }
}


/*----------------------------------------------------------------------------
 * EIP202_RDR_Processed_FillLevel_Get
 */
EIP202_Ring_Error_t
EIP202_RDR_Processed_FillLevel_Get(
        EIP202_Ring_IOArea_t * const IOArea_p,
        unsigned int * FillLevelDscrCount_p,
        unsigned int * FillLevelPktCount_p)
{
// If configured then the driver cannot rely on the register counter
#if defined(EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS) || \
    defined(EIP202_RDR_OWNERSHIP_WORD_ENABLE)
    IDENTIFIER_NOT_USED(FillLevelDscrCount_p);
    IDENTIFIER_NOT_USED(FillLevelPktCount_p);
    IDENTIFIER_NOT_USED(IOArea_p);

    return EIP202_RING_UNSUPPORTED_FEATURE_ERROR;
#else
    Device_Handle_t Device;
    unsigned int RDWordCount, PktCount;
    volatile EIP202_RDR_True_IOArea_t * const TrueIOArea_p = RDRIOAREA(IOArea_p);

    EIP202_RING_CHECK_POINTER(IOArea_p);
    EIP202_RING_CHECK_POINTER(FillLevelDscrCount_p);
    EIP202_RING_CHECK_POINTER(FillLevelPktCount_p);

    Device = TrueIOArea_p->Device;

    {
        uint8_t Value8;
        uint32_t Value32;

        EIP202_RDR_PROC_COUNT_RD(Device, &Value32, &Value8);

        RDWordCount = (unsigned int)Value32;
        PktCount = (unsigned int)Value8;
    }

    return EIP202Lib_RDR_Processed_FillLevel_Finalize(RDWordCount,
                                                     PktCount,
                                                     TrueIOArea_p,
                                                     FillLevelDscrCount_p,
                                                     FillLevelPktCount_p);
#endif // EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS
}


/*----------------------------------------------------------------------------
 * EIP202_RDR_Read_Processed_ControlWord
 */
void
EIP202_RDR_Read_Processed_ControlWord(
        EIP202_ARM_ResultDescriptor_t * const  ResDscr_p,
        EIP202_RDR_Result_Control_t * const RDControl_p,
        EIP202_RDR_Result_Token_t * const ResToken_p)
{
    IDENTIFIER_NOT_USED(ResToken_p);

    EIP202_RD_Read_ControlWord(ResDscr_p->ProcControlWord,
                               NULL,
                               RDControl_p,
                               NULL);

    return;
}


/*----------------------------------------------------------------------------
 * EIP202_RDR_Read_Processed_BypassData
 */
void
EIP202_RDR_Read_Processed_BypassData(
        const EIP202_RDR_Result_Token_t * const  ResToken_p,
        EIP202_RDR_BypassData_t * const BD_p)
{
    EIP202_RD_Read_BypassData(ResToken_p->BypassData_p,
                              ResToken_p->BypassWordCount,
                              BD_p);

    return;
}


/*----------------------------------------------------------------------------
 * EIP202_RDR_Descriptor_Get
 */
EIP202_Ring_Error_t
EIP202_RDR_Descriptor_Get(
        EIP202_Ring_IOArea_t * const IOArea_p,
        EIP202_ARM_ResultDescriptor_t * ResultDscr_p,
        const unsigned int PktRequestedCount,
        const unsigned int DscrRequestedCount,
        unsigned int * const DscrDoneCount_p,
        unsigned int * FillLevelDscrCount_p)
{
    Device_Handle_t Device;
    int res;
    unsigned int RDWordCount, ProcDsrcCount, ProcPktCount;
    volatile EIP202_RDR_True_IOArea_t * const TrueIOArea_p = RDRIOAREA(IOArea_p);

    EIP202_RING_CHECK_POINTER(IOArea_p);
    EIP202_RING_CHECK_POINTER(ResultDscr_p);
    EIP202_RING_CHECK_POINTER(DscrDoneCount_p);
    EIP202_RING_CHECK_POINTER(FillLevelDscrCount_p);

    if(DscrRequestedCount == 0)
        return EIP202_RING_ARGUMENT_ERROR;

    Device = TrueIOArea_p->Device;

    // Check how many descriptors can be obtained
    {
        uint8_t Value8;
        uint32_t Value32;

#ifdef EIP202_RDR_OWNERSHIP_WORD_ENABLE
        Value32 = MIN(PktRequestedCount, DscrRequestedCount);
        Value8  = MIN(EIP202_RING_MAX_RD_PACKET_COUNT, Value32);
        Value32 = Value8 * TrueIOArea_p->DescOffsWordCount;
        IDENTIFIER_NOT_USED(Device);
#else
        EIP202_RDR_PROC_COUNT_RD(Device, &Value32, &Value8);
#endif
        RDWordCount = (unsigned int)Value32;
        ProcPktCount = (unsigned int)Value8;
    }

    // Check if RDR is empty or
    // if RDR has no fully processed packet descriptor chain
    if(RDWordCount == 0 ||
       (PktRequestedCount != 0 && ProcPktCount == 0))
    {
        // Nothing to do
        *FillLevelDscrCount_p = 0;
        *DscrDoneCount_p = 0;

        return EIP202_RDR_State_Set((volatile EIP202_RDR_State_t*)&TrueIOArea_p->State,
                                   EIP202_RDR_STATE_INITIALIZED);
    }

    ProcDsrcCount = RDWordCount /
                    (unsigned int)TrueIOArea_p->DescOffsWordCount;

    TrueIOArea_p->AcknowledgedRDCount = 0;
    TrueIOArea_p->AcknowledgedPktCount = 0;
    TrueIOArea_p->RDToGetCount = MIN(ProcDsrcCount, DscrRequestedCount);
    TrueIOArea_p->PktToGetCount = MIN(ProcPktCount, PktRequestedCount);

    // Get processed (result) descriptors
    res = RingHelper_Get((volatile RingHelper_t*)&TrueIOArea_p->RingHelper,
// If configured then the driver cannot rely on the register counter
#if defined(EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS) || \
    defined(EIP202_RDR_OWNERSHIP_WORD_ENABLE)
                         -1, // Certainly available RD count unknown
#else
                         (int)TrueIOArea_p->RDToGetCount,
#endif // EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS
                         ResultDscr_p,
                         (int)DscrRequestedCount);
    if(res >= 0)
        *DscrDoneCount_p = (unsigned int)res;
    else
        return EIP202_RING_ARGUMENT_ERROR;

    // Increase the fill level by the number of successfully got descriptors
    RDWordCount -= ((unsigned int)res *
                         (unsigned int)TrueIOArea_p->DescOffsWordCount);

    // Increase the fill level by the number of acknowledged packets
    ProcPktCount -= TrueIOArea_p->AcknowledgedPktCount;

    return EIP202Lib_RDR_Processed_FillLevel_Finalize(RDWordCount,
                                                     ProcPktCount,
                                                     TrueIOArea_p,
                                                     FillLevelDscrCount_p,
                                                     &ProcPktCount);
}


/* end of file eip202_rdr_dscr.c */
