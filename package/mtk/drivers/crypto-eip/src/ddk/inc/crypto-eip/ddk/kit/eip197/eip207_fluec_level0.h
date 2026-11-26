/* eip207_fluec_level0.h
 *
 * EIP-207 Flow Look-Up Engine Cache (FLUEC) Level0 Internal interface
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

#ifndef EIP207_FLUEC_LEVEL0_H_
#define EIP207_FLUEC_LEVEL0_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // bool, uint32_t,
                                // IDENTIFIER_NOT_USED

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP207 FLUEC Functions
 *
 */

static inline void
EIP207_FLUEC_CTRL_WR(
        const Device_Handle_t Device,
        const bool fSWReset,
        const bool fDelayMemXS,
        const uint8_t TableSize,
        const uint8_t GroupSize,
        const uint8_t CacheChain)
{
    // Not implemented
    IDENTIFIER_NOT_USED(Device);
    IDENTIFIER_NOT_USED(fSWReset);
    IDENTIFIER_NOT_USED(fDelayMemXS);
    IDENTIFIER_NOT_USED(TableSize);
    IDENTIFIER_NOT_USED(GroupSize);
    IDENTIFIER_NOT_USED(CacheChain);
}


#endif /* EIP207_FLUEC_LEVEL0_H_ */


/* end of file eip207_fluec_level0.h */
