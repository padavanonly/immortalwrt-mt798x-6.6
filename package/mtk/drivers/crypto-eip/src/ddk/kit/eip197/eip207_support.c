/* eip207_support.c
 *
 * EIP-207 Support interface implementation
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

// EIP-207 Support interface
#include "eip207_support.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip207_global.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // uint32_t, bool

// Driver Framework Device API
#include "device_types.h"               // Device_Handle_t
#include "device_rw.h"                  // Read32, Write32


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * EIP207_Global_Read64
 */
EIP207_Global_Error_t
EIP207_Global_Read64(
        const Device_Handle_t Device,
        const unsigned int Value64_OffsetLo,
        const unsigned int Value64_OffsetHi,
        EIP207_Global_Value64_t * const Value64_p)
{
    uint32_t Value32;
    unsigned int i;

    for (i = 0; i < EIP207_VALUE_64BIT_MAX_NOF_READ_ATTEMPTS; i++)
    {
        Value32 = Device_Read32(Device, Value64_OffsetHi);

        Value64_p->Value64_Lo = Device_Read32(Device, Value64_OffsetLo);
        Value64_p->Value64_Hi = Device_Read32(Device, Value64_OffsetHi);

        if (Value32 == Value64_p->Value64_Hi)
            return EIP207_GLOBAL_NO_ERROR;
    }

    return EIP207_GLOBAL_INTERNAL_ERROR;
}


/* end of file eip207_support.c */
