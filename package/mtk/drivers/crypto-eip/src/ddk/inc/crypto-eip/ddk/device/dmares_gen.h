/* dmares_gen.h
 *
 * Declare functions exported by "dmares_gen.c" that implements large parts
 * of the DMAResource API. The exported functions are to be used by module(s)
 * that implement the remaining parts of the DMAResource API.
 *
 */

/*****************************************************************************
* Copyright (c) 2012-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef DMARES_GEN_H_
#define DMARES_GEN_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // bool

// Driver Framework DMAResource Types API
#include "dmares_types.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * DMAResourceLib_IsSaneInput
 */
bool
DMAResourceLib_IsSaneInput(
        const DMAResource_AddrPair_t * AddrPair_p,
        const char * const AllocatorRef_p,
        const DMAResource_Properties_t * Props_p);


/*----------------------------------------------------------------------------
 * DMAResourceLib_AlignForSize
 */
unsigned int
DMAResourceLib_AlignForSize(
        const unsigned int ByteCount,
        const unsigned int AlignTo);


/*----------------------------------------------------------------------------
 * DMAResourceLib_AlignForAddress
 */
unsigned int
DMAResourceLib_AlignForAddress(
        const unsigned int ByteCount,
        const unsigned int AlignTo);


/*----------------------------------------------------------------------------
 * DMAResourceLib_LookupDomain
 */
DMAResource_AddrPair_t *
DMAResourceLib_LookupDomain(
        const DMAResource_Record_t * Rec_p,
        const DMAResource_AddrDomain_t Domain);


#endif // DMARES_GEN_H_


/* end of file dmares_gen.h */
