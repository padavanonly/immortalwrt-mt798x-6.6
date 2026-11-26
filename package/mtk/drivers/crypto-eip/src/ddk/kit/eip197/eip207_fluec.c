/* eip207_fluec.c
 *
 * EIP-207 Flow Look-Up Engine Cache (FLUEC) interface implementation
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

// EIP-207s Flow Look-Up Engine Cache (FLUEC) interface
#include "eip207_fluec.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // uint8_t, uint32_t,
                                        // IDENTIFIER_NOT_USED

// Driver Framework Device API
#include "device_types.h"               // Device_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * EIP207_FLUEC_Invalidate
 */
void
EIP207_FLUEC_Invalidate(
        const Device_Handle_t Device,
        const uint8_t InvTable,
        const uint32_t FlowID_W0,
        const uint32_t FlowID_W1,
        const uint32_t FlowID_W2,
        const uint32_t FlowID_W3)
{
    // Not implemented
    IDENTIFIER_NOT_USED(Device);
    IDENTIFIER_NOT_USED(InvTable);
    IDENTIFIER_NOT_USED(FlowID_W0);
    IDENTIFIER_NOT_USED(FlowID_W1);
    IDENTIFIER_NOT_USED(FlowID_W2);
    IDENTIFIER_NOT_USED(FlowID_W3);
}


/* end of file eip207_fluec.c */
