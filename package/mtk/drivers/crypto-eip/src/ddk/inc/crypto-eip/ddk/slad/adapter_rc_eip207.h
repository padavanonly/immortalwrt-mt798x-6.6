/* adapter_rc_eip207.h
 *
 * Interface to Security-IP-207 Record Cache functionality.
 */

/*****************************************************************************
* Copyright (c) 2011-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef ADAPTER_RC_EIP207_H_
#define ADAPTER_RC_EIP207_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // bool


/*----------------------------------------------------------------------------
 * Adapter_RC_EIP207_Configure
 *
 * This routine configures the Security-IP-207 Record Cache functionality
 * with EIP-207 HW parameters that can be obtained via the HW datasheet or
 * the Security-IP-207 Global Control interface, for example.
 *
 * fEnabledTRC (input)
 *         True when the EIP-207 Transform Record Cache is enabled
 *
 * fEnabledARC4RC (input)
 *         True when the EIP-207 ARC4 Record Cache is enabled
 *
 * fCombined (input)
 *         True when the EIP-207 TRC and ARC4RC are combined
 *
 * This function is re-entrant.
 */
void
Adapter_RC_EIP207_Configure(
        const bool fEnabledTRC,
        const bool fEnabledARC4RC,
        const bool fCombined);


#endif /* ADAPTER_RC_EIP207_H_ */


/* end of file adapter_rc_eip207.h */
