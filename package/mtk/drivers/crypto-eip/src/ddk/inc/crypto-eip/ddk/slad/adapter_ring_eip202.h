/* adapter_ring_eip202.h
 *
 * Interface to EIP-202 ring-specific functionality.
 */

/*****************************************************************************
* Copyright (c) 2011-2022 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef ADAPTER_RING_EIP202_H_
#define ADAPTER_RING_EIP202_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // bool


/*----------------------------------------------------------------------------
 * Adapter_Ring_EIP202_Configure
 *
 * This routine configures the Security-IP-202 ring-specific functionality
 * with parameters that are obtained via the Global Control
 * interface.
 *
 * HostDataWidth (input)
 *         Host interface data width:
 *              0 = 32 bits, 1 = 64 bits, 2 = 128 bits, 3 = 256 bits
 *
 * CF_Size (input)
 *         Command Descriptor FIFO size, the actual size is 2^CF_Size 32-bit
 *         words.
 *
 * RF_Size (input)
 *         Result Descriptor FIFO size, the actual size is 2^RF_Size 32-bit
 *         words.
 *
 * This function is re-entrant.
 */
void
Adapter_Ring_EIP202_Configure(
        const uint8_t HostDataWidth,
        const uint8_t CF_Size,
        const uint8_t RF_Size);


#endif /* ADAPTER_RING_EIP202_H_ */


/* end of file adapter_ring_eip202.h */
