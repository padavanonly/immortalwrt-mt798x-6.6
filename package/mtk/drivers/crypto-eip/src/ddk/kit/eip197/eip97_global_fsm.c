/* eip97_global_fsm.c
 *
 * EIP-97 Global Control Driver Library API State Machine Internal Interface
 * implementation
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

#include "eip97_global_fsm.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// EIP-97 Driver Library Types API
#include "eip97_global_types.h"        // EIP97_Global_* types


/*----------------------------------------------------------------------------
 * EIP97_Global_State_Set
 *
 */
EIP97_Global_Error_t
EIP97_Global_State_Set(
        volatile EIP97_Global_State_t * const CurrentState,
        const EIP97_Global_State_t NewState)
{
    switch(*CurrentState)
    {
        case EIP97_GLOBAL_STATE_UNKNOWN:
            switch(NewState)
            {
                case EIP97_GLOBAL_STATE_SW_RESET_START:
                    *CurrentState = NewState;
                    break;
                case EIP97_GLOBAL_STATE_SW_RESET_DONE:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP97_GLOBAL_ILLEGAL_IN_STATE;
            }
            break;

         case EIP97_GLOBAL_STATE_HW_RESET_DONE:
            switch(NewState)
            {
                case EIP97_GLOBAL_STATE_INITIALIZED:
                   *CurrentState = NewState;
                   break;
                default:
                    return EIP97_GLOBAL_ILLEGAL_IN_STATE;
            }
            break;

        case EIP97_GLOBAL_STATE_SW_RESET_START:
            switch(NewState)
            {
                case EIP97_GLOBAL_STATE_SW_RESET_DONE:
                    *CurrentState = NewState;
                    break;
                case EIP97_GLOBAL_STATE_SW_RESET_START:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP97_GLOBAL_ILLEGAL_IN_STATE;
            }
            break;

        case EIP97_GLOBAL_STATE_SW_RESET_DONE:
            switch(NewState)
            {
                case EIP97_GLOBAL_STATE_SW_RESET_DONE:
                case EIP97_GLOBAL_STATE_INITIALIZED:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP97_GLOBAL_ILLEGAL_IN_STATE;
            }
            break;

        case EIP97_GLOBAL_STATE_INITIALIZED:
            switch(NewState)
            {
                case EIP97_GLOBAL_STATE_ENABLED:
                    *CurrentState = NewState;
                    break;
                case EIP97_GLOBAL_STATE_SW_RESET_DONE:
                    *CurrentState = NewState;
                    break;
                case EIP97_GLOBAL_STATE_INITIALIZED:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP97_GLOBAL_ILLEGAL_IN_STATE;
            }
            break;

        case EIP97_GLOBAL_STATE_ENABLED:
            switch(NewState)
            {
                case EIP97_GLOBAL_STATE_SW_RESET_START:
                    *CurrentState = NewState;
                    break;
                case EIP97_GLOBAL_STATE_SW_RESET_DONE:
                    *CurrentState = NewState;
                    break;
                case EIP97_GLOBAL_STATE_FATAL_ERROR:
                    break;
                case EIP97_GLOBAL_STATE_ENABLED:
                    break;
                default:
                    return EIP97_GLOBAL_ILLEGAL_IN_STATE;
            }
            break;

        case EIP97_GLOBAL_STATE_FATAL_ERROR:
            switch(NewState)
            {
                case EIP97_GLOBAL_STATE_SW_RESET_START:
                    *CurrentState = NewState;
                    break;
                case EIP97_GLOBAL_STATE_SW_RESET_DONE:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP97_GLOBAL_ILLEGAL_IN_STATE;
            }
            break;

        default:
            return EIP97_GLOBAL_ILLEGAL_IN_STATE;
    }

    return EIP97_GLOBAL_NO_ERROR;
}


/* end of file eip97_global_fsm.c */
