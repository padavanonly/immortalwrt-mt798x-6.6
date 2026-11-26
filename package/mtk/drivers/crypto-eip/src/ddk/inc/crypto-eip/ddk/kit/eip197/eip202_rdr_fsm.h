/* eip202_rdr_fsm.h
 *
 * EIP-202 Ring Control Driver Library API State Machine Internal Interface
 * for Result Descriptor Ring (RDR)
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

#ifndef EIP202_RDR_FSM_H_
#define EIP202_RDR_FSM_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "eip202_ring_types.h"            // EIP202_Ring_Error_t

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// EIP-202 Ring Control Driver Library API States for RDR
typedef enum
{
    EIP202_RDR_STATE_UNKNOWN = 1,
    EIP202_RDR_STATE_UNINITIALIZED,
    EIP202_RDR_STATE_INITIALIZED,
    EIP202_RDR_STATE_FREE,
    EIP202_RDR_STATE_FULL,
    EIP202_RDR_STATE_FATAL_ERROR
} EIP202_RDR_State_t;


/*----------------------------------------------------------------------------
 * EIP202_RDR_State_Set
 *
 * This function check whether the transition from the "CurrentState" to the
 * "NewState" is allowed and if yes changes the former to the latter.
 *
  * Return value
 *     EIP202_RING_NO_ERROR : operation is completed
 *     EIP202_RING_ILLEGAL_IN_STATE : state transition is not allowed
 */
EIP202_Ring_Error_t
EIP202_RDR_State_Set(
        volatile EIP202_RDR_State_t * const CurrentState,
        const EIP202_RDR_State_t NewState);


#endif /* EIP202_RDR_FSM_H_ */


/* end of file eip202_rdr_fsm.h */
