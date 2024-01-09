/* device_swap.h
 *
 * Driver Framework, Device API, Swap-Endianness function
 *
 * The document "Driver Framework Porting Guide" contains the detailed
 * specification of this API. The information contained in this header file
 * is for reference only.
 */

/*****************************************************************************
* Copyright (c) 2007-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_DEVICE_SWAP_H
#define INCLUDE_GUARD_DEVICE_SWAP_H

// Driver Framework Basic Defs API
#include "basic_defs.h"     // uint32_t, inline

/*----------------------------------------------------------------------------
 * Device_SwapEndian32
 *
 * This function can be used to swap the byte order of a 32bit integer. The
 * implementation could use custom CPU instructions, if available.
 */
static inline uint32_t
Device_SwapEndian32(
        const uint32_t Value)
{
#ifdef DEVICE_SWAP_SAFE
    return (((Value & 0x000000FFU) << 24) |
            ((Value & 0x0000FF00U) <<  8) |
            ((Value & 0x00FF0000U) >>  8) |
            ((Value & 0xFF000000U) >> 24));
#else
    // reduces typically unneeded AND operations
    return ((Value << 24) |
            ((Value & 0x0000FF00U) <<  8) |
            ((Value & 0x00FF0000U) >>  8) |
            (Value >> 24));
#endif
}

#endif /* Include Guard */

/* end of file device_swap.h */
