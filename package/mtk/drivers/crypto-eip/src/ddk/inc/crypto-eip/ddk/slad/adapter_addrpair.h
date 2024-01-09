/* adapter_addrpair.h
 *
 * Convert an address (pointer) to a pair of 32-bit words.
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

#ifndef ADAPTER_ADDDRPAIR_H_
#define ADAPTER_ADDRPAIR_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Defs API
#include "basic_defs.h" // Type definitions uint32_t.

/*----------------------------------------------------------------------------
 * Adapter_AddrToWordPair
 *
 * Convert address (void *) to a pair of 32-bit words.
 *
 * Address_p (input)
 *     The address to be converted.
 * Offset (input)
 *     Integer offset to be added to the address.
 * WordLow_p (output)
 *     Least significant address word.
 * WordHigh_p (output)
 *     Most significant address word.
 */
static inline void
Adapter_AddrToWordPair(
    const void *Address_p,
    const int Offset,
    uint32_t *WordLow_p,
    uint32_t *WordHigh_p)
{
#ifdef ADAPTER_64BIT_HOST
    uint64_t IntAddr;
    IntAddr = (uint64_t)Address_p + Offset;
    *WordLow_p = IntAddr & 0xFFFFFFFF;
    *WordHigh_p = IntAddr >> 32;
#else
    *WordLow_p = (uint32_t)Address_p + Offset;
    *WordHigh_p = 0;
#endif
}


#endif /* ADAPTER_ADDRPAIR_H */


/* end of file adapter_addrpair.h */
