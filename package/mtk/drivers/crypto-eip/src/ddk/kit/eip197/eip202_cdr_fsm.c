/* eip202_cdr_fsm.c
 *
 * EIP-202 Ring Control Driver Library API
 * State Machine Internal Interface implementation
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

#include "eip202_cdr_fsm.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip202_ring.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"              // IDENTIFIER_NOT_USED

// EIP-202 Ring Control Driver Library Types API
#include "eip202_ring_types.h"        // EIP202_Ring_* types


/*----------------------------------------------------------------------------
 * EIP202_CDR_State_Set
 *
 */
EIP202_Ring_Error_t
EIP202_CDR_State_Set(
        volatile EIP202_CDR_State_t * const CurrentState,
        const EIP202_CDR_State_t NewState)
{
#ifdef EIP202_RING_DEBUG_FSM
    switch(*CurrentState)
    {
        case EIP202_CDR_STATE_UNKNOWN:
            switch(NewState)
            {
                case EIP202_CDR_STATE_UNINITIALIZED:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP202_RING_ILLEGAL_IN_STATE;
            }
            break;

         case EIP202_CDR_STATE_UNINITIALIZED:
            switch(NewState)
            {
                case EIP202_CDR_STATE_INITIALIZED:
                   *CurrentState = NewState;
                   break;
                default:
                    return EIP202_RING_ILLEGAL_IN_STATE;
            }
            break;

        case EIP202_CDR_STATE_INITIALIZED:
            switch(NewState)
            {
                case EIP202_CDR_STATE_UNINITIALIZED:
                    *CurrentState = NewState;
                    break;
                case EIP202_CDR_STATE_FREE:
                    *CurrentState = NewState;
                    break;
                case EIP202_CDR_STATE_FULL:
                    *CurrentState = NewState;
                    break;
                case EIP202_CDR_STATE_INITIALIZED:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP202_RING_ILLEGAL_IN_STATE;
            }
            break;

        case EIP202_CDR_STATE_FREE:
            switch(NewState)
            {
                case EIP202_CDR_STATE_UNINITIALIZED:
                    *CurrentState = NewState;
                    break;
                case EIP202_CDR_STATE_INITIALIZED:
                    *CurrentState = NewState;
                    break;
                case EIP202_CDR_STATE_FULL:
                    *CurrentState = NewState;
                    break;
                case EIP202_CDR_STATE_FATAL_ERROR:
                    *CurrentState = NewState;
                    break;
                case EIP202_CDR_STATE_FREE:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP202_RING_ILLEGAL_IN_STATE;
            }
            break;

        case EIP202_CDR_STATE_FULL:
            switch(NewState)
            {
                case EIP202_CDR_STATE_UNINITIALIZED:
                    *CurrentState = NewState;
                    break;
                case EIP202_CDR_STATE_INITIALIZED:
                    *CurrentState = NewState;
                    break;
                case EIP202_CDR_STATE_FREE:
                    *CurrentState = NewState;
                    break;
                case EIP202_CDR_STATE_FATAL_ERROR:
                    *CurrentState = NewState;
                    break;
                case EIP202_CDR_STATE_FULL:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP202_RING_ILLEGAL_IN_STATE;
            }
            break;

        case EIP202_CDR_STATE_FATAL_ERROR:
            switch(NewState)
            {
                case EIP202_CDR_STATE_UNINITIALIZED:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP202_RING_ILLEGAL_IN_STATE;
            }
            break;

        default:
            return EIP202_RING_ILLEGAL_IN_STATE;
    }
#else
    IDENTIFIER_NOT_USED(CurrentState);
    IDENTIFIER_NOT_USED(NewState);
#endif // EIP202_RING_DEBUG_FSM

    return EIP202_RING_NO_ERROR;
}


/* end of file eip202_cdr_fsm.c */
