/* eip97_global_fsm.h
 *
 * EIP-97 Global Control Driver Library API State Machine Internal Interface
 *
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

#ifndef EIP97_GLOBAL_FSM_H_
#define EIP97_GLOBAL_FSM_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "eip97_global_types.h"            // EIP97_Global_Error_t

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// EIP-97 Global Control Driver Library API States
typedef enum
{
    EIP97_GLOBAL_STATE_UNKNOWN = 1,
    EIP97_GLOBAL_STATE_SW_RESET_START,
    EIP97_GLOBAL_STATE_SW_RESET_DONE,
    EIP97_GLOBAL_STATE_HW_RESET_DONE,
    EIP97_GLOBAL_STATE_INITIALIZED,
    EIP97_GLOBAL_STATE_ENABLED,
    EIP97_GLOBAL_STATE_FATAL_ERROR
} EIP97_Global_State_t;


/*----------------------------------------------------------------------------
 * EIP97_Global_State_Set
 *
 * This function check whether the transition from the "CurrentState" to the
 * "NewState" is allowed and if yes changes the former to the latter.
 *
  * Return value
 *     EIP97_GLOBAL_NO_ERROR : operation is completed
 *     EIP97_GLOBAL_ILLEGAL_IN_STATE : state transition is not allowed
 */
EIP97_Global_Error_t
EIP97_Global_State_Set(
        volatile EIP97_Global_State_t * const CurrentState,
        const EIP97_Global_State_t NewState);


#endif /* EIP97_GLOBAL_FSM_H_ */


/* end of file eip97_global_fsm.h */
