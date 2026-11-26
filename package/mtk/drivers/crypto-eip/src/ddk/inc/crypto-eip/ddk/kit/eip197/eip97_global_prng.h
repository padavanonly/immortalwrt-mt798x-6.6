/* eip97_global_prng.h
 *
 * EIP-97 Global Control Driver Library API:
 * PRNG Re-Seed use case
 *
 * Refer to the EIP-97 Driver Library User Guide for information about
 * re-entrance and usage from concurrent execution contexts of this API
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

#ifndef EIP97_GLOBAL_PRNG_H_
#define EIP97_GLOBAL_PRNG_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint32_t

// EIP-97 Global Control Driver Library Types API
#include "eip97_global_types.h" // EIP97_* types


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// EIP-96 PRNG Re-seed data
typedef struct
{
    // Seed value low 32-bit word
    uint32_t SeedLo;

    // Seed value high 32-bit word
    uint32_t SeedHi;

    // Key register 0 value low 32-bit word
    uint32_t Key0Lo;

    // Key register 0 value high 32-bit word
    uint32_t Key0Hi;

    // Key register 1 value low 32-bit word
    uint32_t Key1Lo;

    // Key register 1 value high 32-bit word
    uint32_t Key1Hi;

    // Seed value low 32-bit word
    uint32_t LFSRLo;

    // Seed value high 32-bit word
    uint32_t LFSRHi;
} EIP96_PRNG_Reseed_t;


/*----------------------------------------------------------------------------
 * EIP97_Global_PRNG_Reseed
 *
 * This function returns hardware revision information in the Capabilities_p
 * data structure.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * PE_Number (input)
 *     Number of the PE that must be re-seed.
 *
 * ReseedData_p (input)
 *     Pointer to the PRNG seed and key data.
 *
 * Return value
 *     EIP97_GLOBAL_NO_ERROR : operation is completed
 *     EIP97_GLOBAL_ARGUMENT_ERROR : Passed wrong argument
 *     EIP97_GLOBAL_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP97_Global_Error_t
EIP97_Global_PRNG_Reseed(
        EIP97_Global_IOArea_t * const IOArea_p,
        const unsigned int PE_Number,
        const EIP96_PRNG_Reseed_t * const ReseedData_p);


#endif /* EIP97_GLOBAL_PRNG_H_ */


/* end of file eip97_global_prng.h */
