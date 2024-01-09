/* eip97_global_event.c
 *
 * EIP-97 Global Control Driver Library
 * Event Management Module
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

#include "eip97_global_event.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip97_global.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"                // uint32_t

// Driver Framework Device API
#include "device_types.h"              // Device_Handle_t

// EIP-97 Global Control Driver Library Internal interfaces
#include "eip97_global_internal.h"
#include "eip202_global_level0.h"      // EIP-202 Level 0 macros
#include "eip96_level0.h"              // EIP-96 Level 0 macros
#include "eip97_global_fsm.h"          // State machine
#include "eip97_global_level0.h"

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */

#ifdef EIP97_REG_DBG_BASE
/*----------------------------------------------------------------------------
 * EIP97_Global_Debug_Statistics_Get
 */
EIP97_Global_Error_t
EIP97_Global_Debug_Statistics_Get(
        EIP97_Global_IOArea_t * const IOArea_p,
        EIP97_Global_Debug_Statistics_t * const Debug_Statistics_p)
{
    Device_Handle_t Device;
    unsigned int i;
    volatile EIP97_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP97_GLOBAL_CHECK_POINTER(IOArea_p);
    EIP97_GLOBAL_CHECK_POINTER(Debug_Statistics_p);

    Device = TrueIOArea_p->Device;

    for (i = 0; i < 16; i++)
    {
        EIP97_DBG_RING_IN_COUNT_RD(Device,
                                   i,
                                   &Debug_Statistics_p->Ifc_Packets_In[i]);
        EIP97_DBG_RING_OUT_COUNT_RD(Device,
                                    i,
                                    &Debug_Statistics_p->Ifc_Packets_Out[i]);

    }
    for (i = 0; i < EIP97_GLOBAL_MAX_NOF_PE_TO_USE; i++)
    {
        EIP97_DBG_PIPE_COUNT_RD(Device,
                                i,
                                &Debug_Statistics_p->Pipe_Total_Packets[i],
                                &Debug_Statistics_p->Pipe_Current_Packets[i],
                                &Debug_Statistics_p->Pipe_Max_Packets[i]);
        EIP97_DBG_PIPE_DCOUNT_RD(Device,
                                 i,
                                 &Debug_Statistics_p->Pipe_Data_Count[i]);
    }
    for (i = EIP97_GLOBAL_MAX_NOF_PE_TO_USE; i < 16; i++)
    {
        Debug_Statistics_p->Pipe_Total_Packets[i] = 0;
        Debug_Statistics_p->Pipe_Current_Packets[i] = 0;
        Debug_Statistics_p->Pipe_Max_Packets[i] = 0;
        Debug_Statistics_p->Pipe_Data_Count[i] = 0;
    }
    return 0;
}
#endif

/*----------------------------------------------------------------------------
 * EIP97_Global_DFE_Status_Get
 */
EIP97_Global_Error_t
EIP97_Global_DFE_Status_Get(
        EIP97_Global_IOArea_t * const IOArea_p,
        const unsigned int PE_Number,
        EIP97_Global_DFE_Status_t * const DFE_Status_p)
{
    Device_Handle_t Device;
    volatile EIP97_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);
    unsigned int DFEDSEOffset;
    DFEDSEOffset = EIP97_DFEDSE_Offset_Get();

    EIP97_GLOBAL_CHECK_POINTER(IOArea_p);
    EIP97_GLOBAL_CHECK_POINTER(DFE_Status_p);

    if(PE_Number >= EIP97_GLOBAL_MAX_NOF_PE_TO_USE)
        return EIP97_GLOBAL_ARGUMENT_ERROR;

    Device = TrueIOArea_p->Device;

    EIP202_DFE_TRD_STAT_RD(Device,
                           DFEDSEOffset,
                           PE_Number,
                           &DFE_Status_p->CDFifoWord32Count,
                           &DFE_Status_p->CDR_ID,
                           &DFE_Status_p->DMASize,
                           &DFE_Status_p->fAtDMABusy,
                           &DFE_Status_p->fDataDMABusy,
                           &DFE_Status_p->fDMAError);

#ifdef EIP97_GLOBAL_DEBUG_FSM
    {
        EIP97_Global_Error_t rv;

        if(DFE_Status_p->fDMAError)
            rv = EIP97_Global_State_Set(
                    (volatile EIP97_Global_State_t*)&TrueIOArea_p->State,
                    EIP97_GLOBAL_STATE_FATAL_ERROR);
        else
            // Remain in the current state
            rv = EIP97_Global_State_Set(
                    (volatile EIP97_Global_State_t*)&TrueIOArea_p->State,
                    (EIP97_Global_State_t)TrueIOArea_p->State);
        if(rv != EIP97_GLOBAL_NO_ERROR)
            return EIP97_GLOBAL_ILLEGAL_IN_STATE;
    }
#endif // EIP97_GLOBAL_DEBUG_FSM

    return EIP97_GLOBAL_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP97_Global_DSE_Status_Get
 */
EIP97_Global_Error_t
EIP97_Global_DSE_Status_Get(
        EIP97_Global_IOArea_t * const IOArea_p,
        const unsigned int PE_Number,
        EIP97_Global_DSE_Status_t * const DSE_Status_p)
{
    Device_Handle_t Device;
    volatile EIP97_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);
    unsigned int DFEDSEOffset;
    DFEDSEOffset = EIP97_DFEDSE_Offset_Get();

    EIP97_GLOBAL_CHECK_POINTER(IOArea_p);
    EIP97_GLOBAL_CHECK_POINTER(DSE_Status_p);

    if(PE_Number >= EIP97_GLOBAL_MAX_NOF_PE_TO_USE)
        return EIP97_GLOBAL_ARGUMENT_ERROR;

    Device = TrueIOArea_p->Device;

    EIP202_DSE_TRD_STAT_RD(Device,
                           DFEDSEOffset,
                           PE_Number,
                           &DSE_Status_p->RDFifoWord32Count,
                           &DSE_Status_p->RDR_ID,
                           &DSE_Status_p->DMASize,
                           &DSE_Status_p->fDataFlushBusy,
                           &DSE_Status_p->fDataDMABusy,
                           &DSE_Status_p->fDMAError);

#ifdef EIP97_GLOBAL_DEBUG_FSM
    {
        EIP97_Global_Error_t rv;

        if(DSE_Status_p->fDMAError)
            rv = EIP97_Global_State_Set(
                    (volatile EIP97_Global_State_t*)&TrueIOArea_p->State,
                    EIP97_GLOBAL_STATE_FATAL_ERROR);
        else
            // Remain in the current state
            rv = EIP97_Global_State_Set(
                    (volatile EIP97_Global_State_t*)&TrueIOArea_p->State,
                    (EIP97_Global_State_t)TrueIOArea_p->State);
        if(rv != EIP97_GLOBAL_NO_ERROR)
            return EIP97_GLOBAL_ILLEGAL_IN_STATE;
    }
#endif // EIP97_GLOBAL_DEBUG_FSM

    return EIP97_GLOBAL_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP97_Global_EIP96_Token_Status_Get
 */
EIP97_Global_Error_t
EIP97_Global_EIP96_Token_Status_Get(
        EIP97_Global_IOArea_t * const IOArea_p,
        const unsigned int PE_Number,
        EIP96_Token_Status_t * const Token_Status_p)
{
    Device_Handle_t Device;
    volatile EIP97_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP97_GLOBAL_CHECK_POINTER(IOArea_p);
    EIP97_GLOBAL_CHECK_POINTER(Token_Status_p);

    if(PE_Number >= EIP97_GLOBAL_MAX_NOF_PE_TO_USE)
        return EIP97_GLOBAL_UNSUPPORTED_FEATURE_ERROR;

    Device = TrueIOArea_p->Device;

#ifdef EIP97_GLOBAL_DEBUG_FSM
    {
        EIP97_Global_Error_t rv;

        // Remain in the current state
        rv = EIP97_Global_State_Set(
                (volatile EIP97_Global_State_t*)&TrueIOArea_p->State,
                (EIP97_Global_State_t)TrueIOArea_p->State);
        if(rv != EIP97_GLOBAL_NO_ERROR)
            return EIP97_GLOBAL_ILLEGAL_IN_STATE;
    }
#endif // EIP97_GLOBAL_DEBUG_FSM

    EIP96_TOKEN_CTRL_STAT_RD(Device,
                             PE_Number,
                             &Token_Status_p->ActiveTokenCount,
                             &Token_Status_p->fTokenLocationAvailable,
                             &Token_Status_p->fResultTokenAvailable,
                             &Token_Status_p->fTokenReadActive,
                             &Token_Status_p->fContextCacheActive,
                             &Token_Status_p->fContextFetch,
                             &Token_Status_p->fResultContext,
                             &Token_Status_p->fProcessingHeld,
                             &Token_Status_p->fBusy);

    return EIP97_GLOBAL_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP97_Global_EIP96_Context_Status_Get
 */
EIP97_Global_Error_t
EIP97_Global_EIP96_Context_Status_Get(
        EIP97_Global_IOArea_t * const IOArea_p,
        const unsigned int PE_Number,
        EIP96_Context_Status_t * const Context_Status_p)
{
    Device_Handle_t Device;
    volatile EIP97_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP97_GLOBAL_CHECK_POINTER(IOArea_p);
    EIP97_GLOBAL_CHECK_POINTER(Context_Status_p);

    if(PE_Number >= EIP97_GLOBAL_MAX_NOF_PE_TO_USE)
        return EIP97_GLOBAL_ARGUMENT_ERROR;

    Device = TrueIOArea_p->Device;

    EIP96_CONTEXT_STAT_RD(Device,
                          PE_Number,
                          &Context_Status_p->Error,
                          &Context_Status_p->AvailableTokenCount,
                          &Context_Status_p->fActiveContext,
                          &Context_Status_p->fNextContext,
                          &Context_Status_p->fResultContext,
                          &Context_Status_p->fErrorRecovery);

#ifdef EIP97_GLOBAL_DEBUG_FSM
    {
        EIP97_Global_Error_t rv;

        if((Context_Status_p->Error & EIP96_TIMEOUT_FATAL_ERROR_MASK) != 0)
            rv = EIP97_Global_State_Set(
                    (volatile EIP97_Global_State_t*)&TrueIOArea_p->State,
                    EIP97_GLOBAL_STATE_FATAL_ERROR);
        else
            // Remain in the current state
            rv = EIP97_Global_State_Set(
                    (volatile EIP97_Global_State_t*)&TrueIOArea_p->State,
                    (EIP97_Global_State_t)TrueIOArea_p->State);
        if(rv != EIP97_GLOBAL_NO_ERROR)
            return EIP97_GLOBAL_ILLEGAL_IN_STATE;
    }
#endif // EIP97_GLOBAL_DEBUG_FSM

    return EIP97_GLOBAL_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP97_Global_EIP96_OutXfer_Status_Get
 */
EIP97_Global_Error_t
EIP97_Global_EIP96_OutXfer_Status_Get(
        EIP97_Global_IOArea_t * const IOArea_p,
        const unsigned int PE_Number,
        EIP96_Output_Transfer_Status_t * const OutXfer_Status_p)
{
    Device_Handle_t Device;
    volatile EIP97_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP97_GLOBAL_CHECK_POINTER(IOArea_p);
    EIP97_GLOBAL_CHECK_POINTER(OutXfer_Status_p);

    if(PE_Number >= EIP97_GLOBAL_MAX_NOF_PE_TO_USE)
        return EIP97_GLOBAL_ARGUMENT_ERROR;

    Device = TrueIOArea_p->Device;

#ifdef EIP97_GLOBAL_DEBUG_FSM
    {
        EIP97_Global_Error_t rv;

        // Remain in the current state
        rv = EIP97_Global_State_Set((volatile EIP97_Global_State_t*)&TrueIOArea_p->State,
                                    (EIP97_Global_State_t)TrueIOArea_p->State);
        if(rv != EIP97_GLOBAL_NO_ERROR)
            return EIP97_GLOBAL_ILLEGAL_IN_STATE;
    }
#endif // EIP97_GLOBAL_DEBUG_FSM

    EIP96_OUT_TRANS_CTRL_STAT_RD(Device,
                                 PE_Number,
                                 &OutXfer_Status_p->AvailableWord32Count,
                                 &OutXfer_Status_p->MinTransferWordCount,
                                 &OutXfer_Status_p->MaxTransferWordCount,
                                 &OutXfer_Status_p->TransferSizeMask);

    return EIP97_GLOBAL_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP97_Global_EIP96_PRNG_Status_Get
 */
EIP97_Global_Error_t
EIP97_Global_EIP96_PRNG_Status_Get(
        EIP97_Global_IOArea_t * const IOArea_p,
        const unsigned int PE_Number,
        EIP96_PRNG_Status_t * const PRNG_Status_p)
{
    Device_Handle_t Device;
    volatile EIP97_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP97_GLOBAL_CHECK_POINTER(IOArea_p);
    EIP97_GLOBAL_CHECK_POINTER(PRNG_Status_p);

    if(PE_Number >= EIP97_GLOBAL_MAX_NOF_PE_TO_USE)
        return EIP97_GLOBAL_ARGUMENT_ERROR;

    Device = TrueIOArea_p->Device;

#ifdef EIP97_GLOBAL_DEBUG_FSM
    {
        EIP97_Global_Error_t rv;

        // Remain in the current state
        rv = EIP97_Global_State_Set((volatile EIP97_Global_State_t*)&TrueIOArea_p->State,
                                    (EIP97_Global_State_t)TrueIOArea_p->State);
        if(rv != EIP97_GLOBAL_NO_ERROR)
            return EIP97_GLOBAL_ILLEGAL_IN_STATE;
    }
#endif // EIP97_GLOBAL_DEBUG_FSM

    EIP96_PRNG_STAT_RD(Device,
                       PE_Number,
                       &PRNG_Status_p->fBusy,
                       &PRNG_Status_p->fResultReady);

    return EIP97_GLOBAL_NO_ERROR;
}


/* end of file eip97_global_event.c */
