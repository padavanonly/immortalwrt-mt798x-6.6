/* cs_eip96_ext.h
 *
 * Top-level EIP-96 Driver Library configuration extensions
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

#ifndef CS_EIP96_EXT_H_
#define CS_EIP96_EXT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Read/Write register constants

/*****************************************************************************
 * Byte offsets of the EIP-96 Packet Engine registers
 *****************************************************************************/

// Processing Packet Engine n (n - number of the DSE thread)
#define EIP96_CONF_BASE                    0xA1000

// EIP-96 PRNG
#define EIP96_PRNG_BASE                    0xA1040

// EIP-96 Options and Version
#define EIP96_VER_BASE                     0xA13F8


#endif /* CS_EIP96_EXT_H_ */


/* end of file cs_eip96_ext.h */
