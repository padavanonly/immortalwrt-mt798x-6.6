/* cs_eip202_global_ext.h
 *
 * Top-level EIP-202 Driver Library Global Control configuration extensions
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

#ifndef CS_EIP202_GLOBAL_EXT_H_
#define CS_EIP202_GLOBAL_EXT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Read/Write register constants

/*****************************************************************************
 * Byte offsets of the EIP-202 HIA registers
 *****************************************************************************/

// HIA DFE all threads
#define EIP202_DFE_BASE           0x8C000

// HIA DFE thread n (n - number of the DFE thread)
#define EIP202_DFE_TRD_BASE       0x8C040

// HIA DSE all threads
#define EIP202_DSE_BASE           0x8D000

// HIA DSE thread n (n - number of the DSE thread)
#define EIP202_DSE_TRD_BASE       0x8D040

// HIA Ring Arbiter
#define EIP202_RA_BASE            0x90000

// HIA Global
#define EIP202_G_BASE             0x9FFF0


#endif /* CS_EIP202_GLOBAL_EXT_H_ */


/* end of file cs_eip202_global_ext.h */
