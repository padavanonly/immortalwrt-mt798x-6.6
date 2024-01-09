/* eip207_flow_internal.c
 *
 *  EIP-207 Flow Control Internal interface implementation
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

// EIP-207 Flow Control Driver Library Internal interfaces
#include "eip207_flow_internal.h"
#include "eip207_flow_generic.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "eip207_flow_level0.h"         // EIP-207 Level 0 macros

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // BIT definitions, bool, uint32_t

// Driver Framework DMA Resource API
#include "dmares_types.h"       // DMAResource_Handle_t
#include "dmares_rw.h"          // DMAResource_Write32()/_Read32()/_PreDMA()


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP207_Flow_Internal_Read64
 */
EIP207_Flow_Error_t
EIP207_Flow_Internal_Read64(
        const DMAResource_Handle_t Handle,
        const unsigned int Value64_WordOffsetLo,
        const unsigned int Value64_WordOffsetHi,
        uint32_t * const Value64_Lo,
        uint32_t * const Value64_Hi)
{
    uint32_t Value32;
    unsigned int i;

    for (i = 0; i < EIP207_FLOW_VALUE_64BIT_MAX_NOF_READ_ATTEMPTS; i++)
    {
        Value32     = DMAResource_Read32(Handle, Value64_WordOffsetHi);
        *Value64_Lo = DMAResource_Read32(Handle, Value64_WordOffsetLo);

        // Prepare the flow record for reading
        DMAResource_PostDMA(Handle, 0, 0);

        *Value64_Hi = DMAResource_Read32(Handle, Value64_WordOffsetHi);

        if (Value32 == (*Value64_Hi))
            return EIP207_FLOW_NO_ERROR;
    }

    return EIP207_FLOW_INTERNAL_ERROR;
}


/* end of file eip207_flow_hte_dscr_dtl.c */
