/* cs_eip207_flow_ext.h
 *
 * Top-level configuration parameters extensions
 * for the EIP-207 Flow Control Driver Library
 *
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

#ifndef CS_EIP207_FLOW_EXT_H_
#define CS_EIP207_FLOW_EXT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


// Read/Write register constants


/*****************************************************************************
 * Byte offsets of the EIP-207 HIA registers
 *****************************************************************************/


// Direct Transform Lookup (DTL) hardware is present
#define EIP207_FLOW_HAVE_DTL

// General Configuration and version, in-flight counters
#define EIP207_REG_IN_FLIGHT_BASE           0xA1FF0

// Flow Hash Engine (FHASH)
#define EIP207_FHASH_REG_BASE               0xF68C0

// Flow Look-Up Engine (FLUE) register bank size
#define EIP207_FLUE_FHT_REG_MAP_SIZE        8192

// EIP-207s Classification Support, FLUE CACHEBASE registers base offset
#define EIP207_FLUE_FHT1_REG_BASE           0x00000

// EIP-207s Classification Support,
// Flow Hash Engine (FHASH) registers base offset
#define EIP207_FLUE_HASH_REG_BASE           0x00100

// EIP-207s Classification Support, FLUE HASHBASE registers base offset
#define EIP207_FLUE_FHT2_REG_BASE           0x00008

// EIP-207s Classification Support, FLUE SIZE register base offset
#define EIP207_FLUE_FHT3_REG_BASE           0x00010

// EIP-207s Classification Support, options and version registers base offset
#define EIP207_FLUE_OPTVER_REG_BASE         0x01FF8

// EIP-207s Classification Support, FLUE Record Cache type - High-Performance
#define EIP207_FLUE_RC_HP


#endif /* CS_EIP207_FLOW_EXT_H_ */


/* end of file cs_eip207_flow_ext.h */
