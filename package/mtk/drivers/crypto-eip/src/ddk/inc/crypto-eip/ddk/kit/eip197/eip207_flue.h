/* eip207_flue.h
 *
 * EIP-207s Flow Look-Up Engine (FLUE) interface
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

#ifndef EIP207_FLUE_H_
#define EIP207_FLUE_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// EIP-207 Global Control Init API
#include "eip207_global_init.h"         // EIP207_*


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // bool, uint32_t

// Driver Framework Device API
#include "device_types.h"               // Device_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * EIP207_FLUE_Init
 */
void
EIP207_FLUE_Init(
        const Device_Handle_t Device,
        const unsigned int HashTableId,
        const EIP207_Global_FLUEConfig_t * const FLUEConf_p,
        const bool fARC4Present,
        const bool fLookupCachePresent);


/*----------------------------------------------------------------------------
 * EIP207_FLUE_Status_Get
 */
void
EIP207_FLUE_Status_Get(
        const Device_Handle_t Device,
        EIP207_Global_FLUE_Status_t * const FLUE_Status_p);


/* end of file eip207_flue.h */


#endif /* EIP207_FLUE_H_ */
