/* eip207_fluec.h
 *
 * EIP-207 Flow Look-Up Engine Cache (FLUEC) interface
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

#ifndef EIP207_FLUEC_H_
#define EIP207_FLUEC_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip207_global.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // uint8_t, uint32_t

// Driver Framework Device API
#include "device_types.h"               // Device_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * EIP207_FLUEC_Invalidate
 *
 * This function invalidates a cached lookup result in the lookup cache
 * based on the flow ID.
 *
 * Device (input)
 *      Device handle that identifies the Record Cache hardware
 *
 * InvTable (input)
 *      Lookup table where the invalidation must be done.
 *
 * FlowID_Wn (input)
 *      32-bit word n of the 128-bit Flow hash ID.
 *
 * Return code
 *      None
 */
void
EIP207_FLUEC_Invalidate(
        const Device_Handle_t Device,
        const uint8_t InvTable,
        const uint32_t FlowID_W0,
        const uint32_t FlowID_W1,
        const uint32_t FlowID_W2,
        const uint32_t FlowID_W3);


/* end of file eip207_fluec.h */


#endif /* EIP207_FLUEC_H_ */
