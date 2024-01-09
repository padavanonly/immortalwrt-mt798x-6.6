/* eip207_flow_generic.c
 *
 * Partial EIP-207 Flow Control Generic API implementation
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

// EIP-207 Driver Library Flow Control Generic API
#include "eip207_flow_generic.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip207_flow.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // uint32_t

// Driver Framework C Run-time Library API
#include "clib.h"                       // ZEROINIT, memset, memcpy

// Driver Framework Device API
#include "device_types.h"               // Device_Handle_t

// Driver Framework DMA Resource API
#include "dmares_types.h"
#include "dmares_rw.h"

// EIP-207 Flow Control Driver Library Internal interfaces
#include "eip207_flow_level0.h"         // EIP-207 Level 0 macros
#include "eip207_flow_internal.h"

// EIP-207 Firmware API
#include "firmware_eip207_api_flow_cs.h" // Classification API: Flow Control
#include "firmware_eip207_api_cs.h"      // Classification API: General

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_Detect
 *
 * Checks the presence of EIP-207c hardware. Returns true when found.
 */
static bool
EIP207Lib_Flow_Detect(
        const Device_Handle_t Device)
{
    uint32_t Value;

    Value = EIP207_Flow_Read32(Device, EIP207_FLUE_FHT_REG_VERSION);
    if (!EIP207_FLUE_SIGNATURE_MATCH( Value ))
        return false;

    // read-write test one of the registers

    // Set MASK_31_BITS bits of the EIP207_FLUE_REG_HASHBASE_LO register
    EIP207_Flow_Write32( Device,
                         EIP207_FLUE_FHT_REG_HASHBASE_LO(0),
                         ~MASK_2_BITS);
    Value = EIP207_Flow_Read32(Device, EIP207_FLUE_FHT_REG_HASHBASE_LO(0));
    if ((Value) != ~MASK_2_BITS)
        return false;

    // Clear MASK_31_BITS bits of the EIP207_FLUE_REG_HASHBASE_LO register
    EIP207_Flow_Write32(Device, EIP207_FLUE_FHT_REG_HASHBASE_LO(0), 0);
    Value = EIP207_Flow_Read32(Device, EIP207_FLUE_FHT_REG_HASHBASE_LO(0));
    if (Value != 0)
       return false;

    return true;
}


#ifdef EIP207_FLOW_DEBUG_FSM
/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_State_Set
 *
 */
EIP207_Flow_Error_t
EIP207_Flow_State_Set(
        EIP207_Flow_State_t * const CurrentState,
        const EIP207_Flow_State_t NewState)
{
    switch(*CurrentState)
    {
        case EIP207_FLOW_STATE_INITIALIZED:
            switch(NewState)
            {
                case EIP207_FLOW_STATE_ENABLED:
                    *CurrentState = NewState;
                    break;
                case EIP207_FLOW_STATE_FATAL_ERROR:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP207_FLOW_ILLEGAL_IN_STATE;
            }
            break;

        case EIP207_FLOW_STATE_ENABLED:
            switch(NewState)
            {
                case EIP207_FLOW_STATE_INSTALLED:
                    *CurrentState = NewState;
                    break;
                case EIP207_FLOW_STATE_FATAL_ERROR:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP207_FLOW_ILLEGAL_IN_STATE;
            }
            break;

        case EIP207_FLOW_STATE_INSTALLED:
            switch(NewState)
            {
                case EIP207_FLOW_STATE_INSTALLED:
                    *CurrentState = NewState;
                    break;
                case EIP207_FLOW_STATE_ENABLED:
                    *CurrentState = NewState;
                    break;
                case EIP207_FLOW_STATE_FATAL_ERROR:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP207_FLOW_ILLEGAL_IN_STATE;
            }
            break;

        default:
            return EIP207_FLOW_ILLEGAL_IN_STATE;
    }

    return EIP207_FLOW_NO_ERROR;
}
#endif // EIP207_FLOW_DEBUG_FSM


/*----------------------------------------------------------------------------
 * EIP207_Flow_IOArea_ByteCount_Get
 */
unsigned int
EIP207_Flow_IOArea_ByteCount_Get(void)
{
    return sizeof(EIP207_Flow_True_IOArea_t);
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_HT_Entry_WordCount_Get
 */
unsigned int
EIP207_Flow_HT_Entry_WordCount_Get(void)
{
    return EIP207_FLOW_HT_ENTRY_WORD_COUNT;
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_Init
 */
EIP207_Flow_Error_t
EIP207_Flow_Init(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const Device_Handle_t Device)
{
    volatile EIP207_Flow_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP207_FLOW_CHECK_POINTER(IOArea_p);

    // Detect presence of EIP-207c HW hardware
    if (!EIP207Lib_Flow_Detect(Device))
        return EIP207_FLOW_UNSUPPORTED_FEATURE_ERROR;

    // Initialize the IO Area
    TrueIOArea_p->Device = Device;

#ifdef EIP207_FLOW_DEBUG_FSM
    {
        TrueIOArea_p->Rec_InstalledCounter = 0;

        TrueIOArea_p->State = (uint32_t)EIP207_FLOW_STATE_INITIALIZED;
    }
#endif // EIP207_FLOW_DEBUG_FSM

    return EIP207_FLOW_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_ID_Compute
 *
 */
EIP207_Flow_Error_t
EIP207_Flow_ID_Compute(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        const EIP207_Flow_SelectorParams_t * const SelectorParams_p,
        EIP207_Flow_ID_t * const FlowID)
{
    uint32_t h1, h2, h3, h4;
    uint32_t w;
    const uint32_t * p;
    uint32_t count, data [FIRMWARE_EIP207_CS_FLOW_HASH_ID_INPUT_WORD_COUNT];
    Device_Handle_t Device;
    FIRMWARE_EIP207_CS_Flow_SelectorParams_t Selectors;
    volatile EIP207_Flow_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP207_FLOW_CHECK_POINTER(IOArea_p);
    EIP207_FLOW_CHECK_POINTER(SelectorParams_p);
    EIP207_FLOW_CHECK_POINTER(FlowID);

    Device = TrueIOArea_p->Device;

    // Read the IV from the flow hash engine in the classification engine
    // The IV must have been already installed in the flow hash engine
    // via the Global Classification Control API
    EIP207_FHASH_IV_RD(Device, HashTableId, &h1, &h2, &h3, &h4);

    // Install the selectors for the flow hash ID calculation
    ZEROINIT(Selectors);
    Selectors.Flags   = SelectorParams_p->Flags;
    Selectors.DstIp_p = SelectorParams_p->DstIp_p;
    Selectors.DstPort = SelectorParams_p->DstPort;
    Selectors.IpProto = SelectorParams_p->IpProto;
    Selectors.SPI     = SelectorParams_p->SPI;
#ifdef FIRMWARE_EIP207_CS_FLOW_DTLS_SUPPORTED
    Selectors.Epoch     = SelectorParams_p->Epoch;
#endif
    Selectors.SrcIp_p = SelectorParams_p->SrcIp_p;
    Selectors.SrcPort = SelectorParams_p->SrcPort;
    FIRMWARE_EIP207_CS_Flow_Selectors_Reorder(&Selectors, data, &count);

    p = data;

    while (p < &data[count - 3])
    {
        w = *p++;
        h2 ^= w;
        h1 += w;
        h1 += h1 << 10;
        h1 ^= h1 >> 6;

        w = *p++;
        h3 ^= w;
        h1 += w;
        h1 += h1 << 10;
        h1 ^= h1 >> 6;

        w = *p++;
        h4 ^= w;
        h1 += w;
        h1 += h1 << 10;
        h1 ^= h1 >> 6;

        /* Mixing step for the 96 bits in h2-h4.  The code comes from a
           hash table lookup function by Robert J. Jenkins, and has been
           presented on numerous web pages and in a Dr. Dobbs Journal
           sometimes in late 90's.

           h1 is computed according to the one-at-a-time hash function,
           presented in the same article. */
        h2 -= h3;  h2 -= h4;  h2 ^= h4 >> 13;
        h3 -= h4;  h3 -= h2;  h3 ^= h2 << 8;
        h4 -= h2;  h4 -= h3;  h4 ^= h3 >> 13;
        h2 -= h3;  h2 -= h4;  h2 ^= h4 >> 12;
        h3 -= h4;  h3 -= h2;  h3 ^= h2 << 16;
        h4 -= h2;  h4 -= h3;  h4 ^= h3 >> 5;
        h2 -= h3;  h2 -= h4;  h2 ^= h4 >> 3;
        h3 -= h4;  h3 -= h2;  h3 ^= h2 << 10;
        h4 -= h2;  h4 -= h3;  h4 ^= h3 >> 15;
    } // while

    w = *p++;
    h1 += w;
    h1 += h1 << 10;
    h1 ^= h1 >> 6;
    h2 ^= w;

    if (p < data + count)
    {
        w = *p++;
        h1 += w;
        h1 += h1 << 10;
        h1 ^= h1 >> 6;
        h3 ^= w;

        if (p < data + count)
        {
            w = *p++;
            h1 += w;
            h1 += h1 << 10;
            h1 ^= h1 >> 6;
            h4 ^= w;
        }
    }

    FlowID->Word32[0] = h1;
    FlowID->Word32[1] = h2;
    FlowID->Word32[2] = h3;
    FlowID->Word32[3] = h4;

    return EIP207_FLOW_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_FR_WordCount_Get
 */
unsigned int
EIP207_Flow_FR_WordCount_Get(void)
{
    return FIRMWARE_EIP207_CS_FRC_RECORD_WORD_COUNT;
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_FR_Read
 */
EIP207_Flow_Error_t
EIP207_Flow_FR_Read(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        EIP207_Flow_FR_Dscr_t * const FR_Dscr_p,
        EIP207_Flow_FR_OutputData_t * const FlowData_p)
{
    EIP207_Flow_Error_t rv;

    EIP207_FLOW_CHECK_POINTER(IOArea_p);
    EIP207_FLOW_CHECK_POINTER(FR_Dscr_p);
    EIP207_FLOW_CHECK_POINTER(FlowData_p);
    EIP207_FLOW_CHECK_INT_ATMOST(HashTableId + 1,
                                 EIP207_FLOW_MAX_NOF_FLOW_HASH_TABLES_TO_USE);

    IDENTIFIER_NOT_USED(HashTableId);

#ifdef EIP207_FLOW_CONSISTENCY_CHECK
    // Consistency check for the provided flow descriptor
    if (FR_Dscr_p->DMA_Addr.Addr == EIP207_FLOW_RECORD_DUMMY_ADDRESS)
        return EIP207_FLOW_INTERNAL_ERROR;
#endif // EIP207_FLOW_CONSISTENCY_CHECK

    // Prepare the flow record for reading
    DMAResource_PostDMA(FR_Dscr_p->DMA_Handle, 0, 0);

    // Read the flow record data

    // Recent record Packets Counter
    FlowData_p->PacketsCounter =
            DMAResource_Read32(FR_Dscr_p->DMA_Handle,
                               FIRMWARE_EIP207_CS_FLOW_FR_STAT_PKT_WORD_OFFSET);

    // Recent record time stamp
    rv = EIP207_Flow_Internal_Read64(
                       FR_Dscr_p->DMA_Handle,
                       FIRMWARE_EIP207_CS_FLOW_FR_TIME_STAMP_LO_WORD_OFFSET,
                       FIRMWARE_EIP207_CS_FLOW_FR_TIME_STAMP_HI_WORD_OFFSET,
                       &FlowData_p->LastTimeLo,
                       &FlowData_p->LastTimeHi);
    if (rv != EIP207_FLOW_NO_ERROR)
        return rv;

    // Recent record Octets Counter
    rv = EIP207_Flow_Internal_Read64(
                       FR_Dscr_p->DMA_Handle,
                       FIRMWARE_EIP207_CS_FLOW_FR_STAT_OCT_LO_WORD_OFFSET,
                       FIRMWARE_EIP207_CS_FLOW_FR_STAT_OCT_HI_WORD_OFFSET,
                       &FlowData_p->OctetsCounterLo,
                       &FlowData_p->OctetsCounterHi);
    if (rv != EIP207_FLOW_NO_ERROR)
        return rv;

#ifdef EIP207_FLOW_DEBUG_FSM
    {
        volatile EIP207_Flow_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);
        uint32_t State = TrueIOArea_p->State;

        // Remain in the current state
        rv = EIP207_Flow_State_Set(
                (EIP207_Flow_State_t* const)&State,
                (EIP207_Flow_State_t)TrueIOArea_p->State);

        TrueIOArea_p->State = State;

        if (rv != EIP207_FLOW_NO_ERROR)
            return EIP207_FLOW_ILLEGAL_IN_STATE;
    }
#else
    IDENTIFIER_NOT_USED(IOArea_p);
#endif // EIP207_FLOW_DEBUG_FSM

    return EIP207_FLOW_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_TR_WordCount_Get
 */
unsigned int
EIP207_Flow_TR_WordCount_Get(void)
{
    return FIRMWARE_EIP207_CS_TRC_RECORD_WORD_COUNT;
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_TR_Read
 */
EIP207_Flow_Error_t
EIP207_Flow_TR_Read(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        const EIP207_Flow_TR_Dscr_t * const TR_Dscr_p,
        EIP207_Flow_TR_OutputData_t * const XformData_p)
{
    EIP207_Flow_Error_t rv;
    uint32_t Value32, SeqNrWordOffset;

    EIP207_FLOW_CHECK_POINTER(IOArea_p);
    EIP207_FLOW_CHECK_POINTER(TR_Dscr_p);
    EIP207_FLOW_CHECK_POINTER(XformData_p);
    EIP207_FLOW_CHECK_INT_ATMOST(HashTableId + 1,
                                 EIP207_FLOW_MAX_NOF_FLOW_HASH_TABLES_TO_USE);

    IDENTIFIER_NOT_USED(IOArea_p);
    IDENTIFIER_NOT_USED(HashTableId);

    // Prepare the transform record for reading
    DMAResource_PostDMA(TR_Dscr_p->DMA_Handle, 0, 0);

    // Read the transform record data

    // Recent record Packets Counter
    XformData_p->PacketsCounter =
            DMAResource_Read32(TR_Dscr_p->DMA_Handle,
                               FIRMWARE_EIP207_CS_FLOW_TR_STAT_PKT_WORD_OFFSET);

    // Read Token Context Instruction word
    Value32 =
        DMAResource_Read32(TR_Dscr_p->DMA_Handle,
                           FIRMWARE_EIP207_CS_FLOW_TR_TK_CTX_INST_WORD_OFFSET);

    // Extract the Sequence Number word offset from the read value
    FIRMWARE_EIP207_CS_Flow_SeqNum_Offset_Read(Value32, &SeqNrWordOffset);

    // Read the sequence number
    XformData_p->SequenceNumber =
            DMAResource_Read32(TR_Dscr_p->DMA_Handle, SeqNrWordOffset);

    // Recent record time stamp
    rv = EIP207_Flow_Internal_Read64(
                   TR_Dscr_p->DMA_Handle,
                   FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_LO_WORD_OFFSET,
                   FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_HI_WORD_OFFSET,
                   &XformData_p->LastTimeLo,
                   &XformData_p->LastTimeHi);
    if (rv != EIP207_FLOW_NO_ERROR)
        return rv;

    // Recent record Octets Counter
    rv = EIP207_Flow_Internal_Read64(
                   TR_Dscr_p->DMA_Handle,
                   FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_LO_WORD_OFFSET,
                   FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_HI_WORD_OFFSET,
                   &XformData_p->OctetsCounterLo,
                   &XformData_p->OctetsCounterHi);
    if (rv != EIP207_FLOW_NO_ERROR)
        return rv;

    return EIP207_FLOW_NO_ERROR;
}


/* end of file eip207_flow_generic.c */
