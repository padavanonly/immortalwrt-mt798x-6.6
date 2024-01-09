/* eip202_cdr_dscr.c
 *
 * EIP-202 Ring Control Driver Library
 * 1) Descriptor I/O Driver Library API implementation
 * 2) Internal Command Descriptor interface implementation
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
#include "eip202_cdr.h"

// Internal Command Descriptor interface
#include "eip202_cdr_dscr.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip202_ring.h"

// EIP-202 Ring Control Driver Library Internal interfaces
#include "eip202_ring_internal.h"
#include "eip202_cdr_level0.h"         // EIP-202 Level 0 macros
#include "eip202_cdr_fsm.h"             // CDR State machine
#include "eip202_cd_format.h"           // CD Format API

// Driver Framework Basic Definitions API
#include "basic_defs.h"                // IDENTIFIER_NOT_USED, bool, uint32_t

// Driver Framework DMA Resource API
#include "dmares_types.h"         // types of the DMA resource API
#include "dmares_rw.h"            // read/write of the DMA resource API.


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP202Lib_CDR_FillLevel_Finalize
 *
 */
static EIP202_Ring_Error_t
EIP202Lib_CDR_FillLevel_Finalize(
        const unsigned int CDWordCount,
        volatile EIP202_CDR_True_IOArea_t * const TrueIOArea_p,
        unsigned int * const FillLevelDscrCount_p)
{
#ifdef EIP202_RING_DEBUG_FSM
    EIP202_Ring_Error_t rv;

    if(CDWordCount == 0)
        // CD Ring is empty
        rv = EIP202_CDR_State_Set((volatile EIP202_CDR_State_t*)&TrueIOArea_p->State,
                                EIP202_CDR_STATE_INITIALIZED);
    else if(CDWordCount > 0 &&
            CDWordCount < (unsigned int)TrueIOArea_p->RingSizeWordCount)
        // CD Ring is free
        rv = EIP202_CDR_State_Set((volatile EIP202_CDR_State_t*)&TrueIOArea_p->State,
                                 EIP202_CDR_STATE_FREE);
    else if(CDWordCount == (unsigned int)TrueIOArea_p->RingSizeWordCount)
        // CD Ring is full
        rv = EIP202_CDR_State_Set((volatile EIP202_CDR_State_t*)&TrueIOArea_p->State,
                                 EIP202_CDR_STATE_FULL);
    else
        rv = EIP202_RING_ILLEGAL_IN_STATE;

    if(rv != EIP202_RING_NO_ERROR)
        return EIP202_RING_ILLEGAL_IN_STATE;
#endif

    // Store actual fill level
    *FillLevelDscrCount_p = CDWordCount /
                            (unsigned int)TrueIOArea_p->DescOffsWordCount;

    // Return actual fill level plus one descriptor to distinguish
    // ring full from ring empty
    if(CDWordCount < (unsigned int)TrueIOArea_p->RingSizeWordCount)
        (*FillLevelDscrCount_p)++;

    return EIP202_RING_NO_ERROR;
}


/*----------------------------------------------------------------------------
  Internal Command Descriptor interface
  ---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * EIP202_CDR_WriteCB
 */
int
EIP202_CDR_WriteCB(
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
    volatile EIP202_CDR_True_IOArea_t * const TrueIOArea_p =
                                            CDRIOAREA(CallbackParam1_p);
    if(CallbackParam1_p == NULL || Descriptors_p == NULL)
        return -1;

    IDENTIFIER_NOT_USED(CallbackParam2);
    IDENTIFIER_NOT_USED(DescriptorCount);
    IDENTIFIER_NOT_USED(TotalWriteLimit);

    Device = TrueIOArea_p->Device;

    DescOffsetWordCount = (unsigned int)TrueIOArea_p->DescOffsWordCount;

    // Write descriptors to CDR
    for(i = WriteIndex; i < WriteIndex + WriteCount; i++)
    {
        EIP202_CD_Write(
            TrueIOArea_p->RingHandle,
            i * DescOffsetWordCount,
            ((const EIP202_ARM_CommandDescriptor_t *)Descriptors_p) +
                                            DescriptorSkipCount + nWritten,
            TrueIOArea_p->fATP);

        nWritten++;
    }

    if (nWritten > 0)
    {
        // Call PreDMA to prepared descriptors in Ring DMA buffer for handover
        // to the Device (the EIP-202 DMA Master)
        DMAResource_PreDMA(TrueIOArea_p->RingHandle,
                           WriteIndex * DescOffsetWordCount *
                             (unsigned int)sizeof(uint32_t),
                           nWritten * DescOffsetWordCount *
                             (unsigned int)sizeof(uint32_t));

        // CDS point: hand over written Command Descriptors to the Device
        EIP202_CDR_COUNT_WR(Device,
                            (uint16_t)(nWritten * DescOffsetWordCount),
                            false);
    }

    return (int) nWritten;
}


/*----------------------------------------------------------------------------
 * EIP202_CDR_ReadCB
 */
int
EIP202_CDR_ReadCB(
        void * const CallbackParam1_p,
        const int CallbackParam2,
        const unsigned int ReadIndex,
        const unsigned int ReadLimit,
        void * Descriptors_p,
        const unsigned int DescriptorSkipCount)
{
    IDENTIFIER_NOT_USED(CallbackParam1_p);
    IDENTIFIER_NOT_USED(CallbackParam2);
    IDENTIFIER_NOT_USED(ReadIndex);
    IDENTIFIER_NOT_USED(ReadLimit);
    IDENTIFIER_NOT_USED(Descriptors_p);
    IDENTIFIER_NOT_USED(DescriptorSkipCount);

    // Not used for CDR

    return -1;
}


/*----------------------------------------------------------------------------
 * EIP202_CDR_StatusCB
 */
int
EIP202_CDR_StatusCB(
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
 * EIP202_CDR_FillLevel_Get
 */
EIP202_Ring_Error_t
EIP202_CDR_FillLevel_Get(
        EIP202_Ring_IOArea_t * const IOArea_p,
        unsigned int * FillLevelDscrCount_p)
{
    Device_Handle_t Device;
    unsigned int CDWordCount;
    volatile EIP202_CDR_True_IOArea_t * const TrueIOArea_p = CDRIOAREA(IOArea_p);

    EIP202_RING_CHECK_POINTER(IOArea_p);
    EIP202_RING_CHECK_POINTER(FillLevelDscrCount_p);

    Device = TrueIOArea_p->Device;

    {
        uint32_t Value32;

        EIP202_CDR_COUNT_RD(Device, &Value32);

        TrueIOArea_p->CountRD       = Value32;
        TrueIOArea_p->fValidCountRD = true;

        CDWordCount = (unsigned int)Value32;
    }

    return EIP202Lib_CDR_FillLevel_Finalize(CDWordCount,
                                            TrueIOArea_p,
                                            FillLevelDscrCount_p);
}


/*----------------------------------------------------------------------------
 * EIP202_CDR_Write_ControlWord
 */
uint32_t
EIP202_CDR_Write_ControlWord(
        const EIP202_CDR_Control_t * const  CommandCtrl_p)
{
    return EIP202_CD_Make_ControlWord(CommandCtrl_p->TokenWordCount,
                                     CommandCtrl_p->SegmentByteCount,
                                     CommandCtrl_p->fFirstSegment,
                                     CommandCtrl_p->fLastSegment,
                                     CommandCtrl_p->fForceEngine,
                                     CommandCtrl_p->EngineId);
}


/*----------------------------------------------------------------------------
 * EIP202_CDR_Descriptor_Put
 *
 */
EIP202_Ring_Error_t
EIP202_CDR_Descriptor_Put(
        EIP202_Ring_IOArea_t * const IOArea_p,
        const EIP202_ARM_CommandDescriptor_t * CommandDscr_p,
        const unsigned int DscrRequestedCount,
        unsigned int * const DscrDoneCount_p,
        unsigned int * FillLevelDscrCount_p)
{
    Device_Handle_t Device;
    int res;
    unsigned int CDWordCount, CDFreeCount, CDNewRequestedCount;
    volatile EIP202_CDR_True_IOArea_t * const TrueIOArea_p = CDRIOAREA(IOArea_p);

    EIP202_RING_CHECK_POINTER(IOArea_p);
    EIP202_RING_CHECK_POINTER(CommandDscr_p);
    EIP202_RING_CHECK_POINTER(DscrDoneCount_p);
    EIP202_RING_CHECK_POINTER(FillLevelDscrCount_p);

    Device = TrueIOArea_p->Device;

    // Check how many descriptors can be put
    {
        uint32_t Value32;

        if (TrueIOArea_p->fValidCountRD)
        {
            Value32 = TrueIOArea_p->CountRD;
            TrueIOArea_p->fValidCountRD = false;
        }
        else
        {
            EIP202_CDR_COUNT_RD(Device, &Value32);
            TrueIOArea_p->CountRD = Value32;
        }

        CDWordCount = (unsigned int)Value32;
    }

    // Check if CDR is full
    if(CDWordCount == (unsigned int)TrueIOArea_p->RingSizeWordCount)
    {
        // CD Ring is full
        *FillLevelDscrCount_p = CDWordCount /
                            (unsigned int)TrueIOArea_p->DescOffsWordCount;
        *DscrDoneCount_p = 0;

        return EIP202_CDR_State_Set((volatile EIP202_CDR_State_t*)&TrueIOArea_p->State,
                                 EIP202_CDR_STATE_FULL);
    }

    CDFreeCount =
            ((unsigned int)TrueIOArea_p->RingSizeWordCount - CDWordCount) /
                    (unsigned int)TrueIOArea_p->DescOffsWordCount;

    CDNewRequestedCount = MIN(CDFreeCount, DscrRequestedCount);

    // Put command descriptors to CDR
    res = RingHelper_Put((volatile RingHelper_t*)&TrueIOArea_p->RingHelper,
                         CommandDscr_p,
                         (int)CDNewRequestedCount);
    if(res >= 0)
        *DscrDoneCount_p = (unsigned int)res;
    else
        return EIP202_RING_ARGUMENT_ERROR;

    // Increase the fill level by the number of successfully put descriptors
    CDWordCount += ((unsigned int)res *
                         (unsigned int)TrueIOArea_p->DescOffsWordCount);

    return EIP202Lib_CDR_FillLevel_Finalize(CDWordCount,
                                           TrueIOArea_p,
                                           FillLevelDscrCount_p);
}


/* end of file eip202_cdr_dscr.c */
