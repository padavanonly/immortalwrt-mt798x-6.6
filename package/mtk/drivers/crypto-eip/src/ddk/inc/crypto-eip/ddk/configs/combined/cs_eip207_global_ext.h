/* cs_eip207_global_ext.h
 *
 * Top-level configuration parameters extensions
 * for the EIP-207 Global Control
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

#ifndef CS_EIP207_GLOBAL_EXT_H_
#define CS_EIP207_GLOBAL_EXT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define EIP207_FLUE_HAVE_VIRTUALIZATION

// Number of interfaces for virtualization.
#define EIP207_FLUE_MAX_NOF_INTERFACES_TO_USE 15

// Read/Write register constants

/*****************************************************************************
 * Byte offsets of the EIP-207 registers
 *****************************************************************************/


// EIP-207c Classification Engine n (n - number of the CE)
// Input Side

// EIP-207c Classification Engine n (n - number of the CE)
// Input Side

#define EIP207_ICE_REG_SCRATCH_BASE    0xA0800
#define EIP207_ICE_REG_ADAPT_CTRL_BASE 0xA0C00
#define EIP207_ICE_REG_PUE_CTRL_BASE   0xA0C80
#define EIP207_ICE_REG_PUTF_CTRL_BASE  0xA0D00
#define EIP207_ICE_REG_FPP_CTRL_BASE   0xA0D80
#define EIP207_ICE_REG_PPTF_CTRL_BASE  0xA0E00
#define EIP207_ICE_REG_RAM_CTRL_BASE   0xA0FF0

// EIP-207c Classification Engine n (n - number of the CE)
// Output Side

#define EIP207_OCE_REG_SCRATCH_BASE    0xA1400
#define EIP207_OCE_REG_ADAPT_CTRL_BASE 0xA1860
#define EIP207_OCE_REG_PUE_CTRL_BASE   0xA1880
#define EIP207_OCE_REG_PUTF_CTRL_BASE  0xA1900
#define EIP207_OCE_REG_FPP_CTRL_BASE   0xA1980
#define EIP207_OCE_REG_PPTF_CTRL_BASE  0xA1A00
#define EIP207_OCE_REG_RAM_CTRL_BASE   0xA1BF0

// Output Side: reserved

// Access space for Classification RAM, base offset
#define EIP207_CS_RAM_XS_SPACE_BASE    0xE0000

// EIP-207s Classification Support module (p - number of the cache set)

// General Record Cache (FRC/TRC/ARC4RC) register interface
#define EIP207_FRC_REG_BASE            0xF0000
#define EIP207_TRC_REG_BASE            0xF0800
#define EIP207_ARC4RC_REG_BASE         0xF1000

// EIP-207s Classification Support, Flow Hash Engine (FHASH)
#define EIP207_FHASH_REG_BASE          0xF68C0

// EIP-207s Classification Support, Flow Look-Up Engine (FLUE), VM-specific
#define EIP207_FLUE_CONFIG_REG_BASE     0xF6010

// EIP-207s Classification Support, Flow Look-Up Engine (FLUE)
#define EIP207_FLUE_REG_BASE            0xF6808

#ifdef EIP207_FLUE_HAVE_VIRTUALIZATION
#define EIP207_FLUE_IFC_LUT_REG_BASE    0xF6820
#endif

#define EIP207_FLUE_ENABLED_REG_BASE    0xF6840

// EIP-207s Classification Support, Flow Look-Up Engine Cache (FLUEC) Control
#define EIP207_FLUEC_REG_BASE           0xF6880

// Flow Look-Up Engine Cache (FLUEC) Control
#define EIP207_FLUEC_REG_INV_BASE       0xF688C

// EIP-207s Classification Support, DMA Control
#define EIP207_CS_DMA_REG_BASE          0xF7000

// EIP-207s Classification Support, Options and Versions
#define EIP207_CS_REG_BASE              0xF7FF0

// Size of the Flow Record Cache (administration RAM) in 32-bit words
#define EIP207_FRC_ADMIN_RAM_WORD_COUNT             320

// Size of the Flow Record Cache (data RAM) in 32-bit words
#define EIP207_FRC_RAM_WORD_COUNT                   768


// Size of the Transform Record Cache (administration RAM) in 32-bit words
// 80 * 4 = 320 32-bit words for EIP-197 configurations b and c
// 320 * 4 = 1280 32-bit words for EIP-197 configurations d and e
//
// Size of the Transform Record Cache (data RAM) in 32-bit words
// 480 * 8 = 3840 32-bit words for EIP-197 configurations b and c
// 1920 * 4 = 15360 32-bit words for EIP-197 configurations d and e
#if EIP207_MAX_NOF_PE_TO_USE > 1
#define EIP207_TRC_ADMIN_RAM_WORD_COUNT             16384
#define EIP207_TRC_RAM_WORD_COUNT                   131072
#else
#define EIP207_TRC_ADMIN_RAM_WORD_COUNT             3840
#define EIP207_TRC_RAM_WORD_COUNT                   15360
#endif

// Size of the ARC4 Record Cache (administration RAM) in 32-bit words
#define EIP207_ARC4RC_ADMIN_RAM_WORD_COUNT          0

// Size of the ARC4 Record Cache (data RAM) in 32-bit words
#define EIP207_ARC4RC_RAM_WORD_COUNT                0

// Input Pull-Up Engine Program RAM size (in 32-bit words), 16KB
#define EIP207_IPUE_PROG_RAM_WORD_COUNT             4096

// Input Flow Post-Processor Engine Program RAM size (in 32-bit words), 16KB
#define EIP207_IFPP_PROG_RAM_WORD_COUNT             4096

// Output Pull-Up Engine Program RAM size (in 32-bit words), 16KB
#define EIP207_OPUE_PROG_RAM_WORD_COUNT             4096

// Output Flow Post-Processor Engine Program RAM size (in 32-bit words), 16KB
#define EIP207_OFPP_PROG_RAM_WORD_COUNT             4096


#endif /* CS_EIP207_GLOBAL_EXT_H_ */


/* end of file cs_eip207_global_ext.h */
