/* cs_eip206_ext.h
 *
 * Top-level EIP-206 Driver Library configuration extensions
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

#ifndef CS_EIP206_EXT_H_
#define CS_EIP206_EXT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Read/Write register constants

/*****************************************************************************
 * Byte offsets of the EIP-206 Processing Engine registers
 *****************************************************************************/

// Processing Packet Engine n (n - number of the PE)
// Input Side
#define EIP206_IN_DBUF_BASE           0xA0000
#define EIP206_IN_TBUF_BASE           0xA0100

// Output Side
#define EIP206_OUT_DBUF_BASE          0xA1C00
#define EIP206_OUT_TBUF_BASE          0xA1D00

// PE Options and Version
#define EIP206_ARC4_BASE              0xA1FEC
#define EIP206_VER_BASE               0xA1FF8


#endif /* CS_EIP206_EXT_H_ */


/* end of file cs_eip206_ext.h */
