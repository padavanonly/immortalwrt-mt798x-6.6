/* shdevxs_prng.h
 *
 * API to access EIP-96 Pseudo Random Number Generator.
 */

/*****************************************************************************
* Copyright (c) 2012-2022 by Rambus, Inc. and/or its subsidiaries.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#ifndef SHDEVXS_PRNG_H_
#define SHDEVXS_PRNG_H_

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "shdevxs_prng.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Types Definitions API
#include "basic_defs.h"


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
} SHDevXS_PRNG_Reseed_t;

/*----------------------------------------------------------------------------
 * SHDevXS_PRNG_Ressed
 *
 * Reseed the internal PRNG of the EIP-96 to obtain predicatble IV's in
 * known answer tests.
 *
 * Return 0 on success, -1 on failure.
 */
int
SHDevXS_PRNG_Reseed(
        SHDevXS_PRNG_Reseed_t * const Reseed_p);

/*----------------------------------------------------------------------------
 * SHDevXS_SupportedFuncs_Get
 *
 * Return a bitmask of the functions supported by the device.
 */
unsigned int
SHDevXS_SupportedFuncs_Get(void);

#endif /* SHDEVXS_PRNG_H_ */


/* end of file shdevxs_prng.h */
