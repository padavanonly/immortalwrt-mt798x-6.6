/* eip207_flow_hw_interface_ext.h
 *
 * EIP-207 Flow HW interface extensions
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : ddk197                                                   */
/*   Version       : 5.6.1                                                    */
/*   Configuration : DDK-197-GPL                                              */
/*                                                                            */
/*   Date          : 2022-Dec-16                                              */
/*                                                                            */
/* Copyright (c) 2008-2022 by Rambus, Inc. and/or its subsidiaries.           */
/*                                                                            */
/* This program is free software: you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation, either version 2 of the License, or          */
/* any later version.                                                         */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               */
/* GNU General Public License for more details.                               */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program. If not, see <http://www.gnu.org/licenses/>.       */
/* -------------------------------------------------------------------------- */

#ifndef EIP207_FLOW_HW_INTERFACE_EXT_H_
#define EIP207_FLOW_HW_INTERFACE_EXT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// One entry size (in 32-bit words) in the Hash Table
// One HT entry = one hash bucket, 16 32-bit words
#define EIP207_FLOW_HT_ENTRY_WORD_COUNT          16

// Maximum number of records one Hash Bucket can refer to
#define EIP207_FLOW_HTE_BKT_NOF_REC_MAX          3

#define EIP207_FLOW_RECORD_ADDRESS_TYPE_BITS     MASK_2_BITS
#define EIP207_FLOW_RECORD_DUMMY_ADDRESS         0
#define EIP207_FLOW_RECORD_FR_ADDRESS            1
#define EIP207_FLOW_RECORD_TR_ADDRESS            2
#define EIP207_FLOW_RECORD_TR_LARGE_ADDRESS      3

// Hash ID fields offsets in the hash bucket
#define EIP207_FLOW_HB_HASH_ID_1_WORD_OFFSET     0
#define EIP207_FLOW_HB_HASH_ID_2_WORD_OFFSET                                 \
                                   (EIP207_FLOW_HB_HASH_ID_1_WORD_OFFSET +   \
                                      EIP207_FLOW_HASH_ID_WORD_COUNT)
#define EIP207_FLOW_HB_HASH_ID_3_WORD_OFFSET                                 \
                                   (EIP207_FLOW_HB_HASH_ID_2_WORD_OFFSET +   \
                                      EIP207_FLOW_HASH_ID_WORD_COUNT)
#define EIP207_FLOW_HB_REC_1_WORD_OFFSET                                     \
                                   (EIP207_FLOW_HB_HASH_ID_3_WORD_OFFSET +   \
                                      EIP207_FLOW_HASH_ID_WORD_COUNT)
#define EIP207_FLOW_HB_REC_2_WORD_OFFSET                                     \
                                   (EIP207_FLOW_HB_REC_1_WORD_OFFSET + 1)
#define EIP207_FLOW_HB_REC_3_WORD_OFFSET                                     \
                                   (EIP207_FLOW_HB_REC_2_WORD_OFFSET + 1)
#define EIP207_FLOW_HB_OVFL_BUCKET_WORD_OFFSET                               \
                                   (EIP207_FLOW_HB_REC_3_WORD_OFFSET + 1)

// Read/Write register constants


/*****************************************************************************
 * Byte offsets of the EIP-207 FLUE and FHASH registers
 *****************************************************************************/

// EIP-207s Classification Support,
// Flow Look-Up Engine (FLUE) CACHEBASE registers
#define EIP207_FLUE_FHT_REG_CACHEBASE_LO(f)  \
                                     (EIP207_FLUE_FHT_REG_MAP_SIZE * f + \
                                       EIP207_FLUE_FHT1_REG_BASE + \
                                        (0x00 * EIP207_REG_OFFS))

#define EIP207_FLUE_FHT_REG_CACHEBASE_HI(f)  \
                                     (EIP207_FLUE_FHT_REG_MAP_SIZE * f + \
                                       EIP207_FLUE_FHT1_REG_BASE + \
                                        (0x01 * EIP207_REG_OFFS))

// EIP-207s Classification Support,
// Flow Look-Up Engine (FLUE) HASHBASE registers
#define EIP207_FLUE_FHT_REG_HASHBASE_LO(f)  \
                                     (EIP207_FLUE_FHT_REG_MAP_SIZE * f + \
                                       EIP207_FLUE_FHT2_REG_BASE + \
                                        (0x00 * EIP207_REG_OFFS))

#define EIP207_FLUE_FHT_REG_HASHBASE_HI(f)  \
                                     (EIP207_FLUE_FHT_REG_MAP_SIZE * f + \
                                       EIP207_FLUE_FHT2_REG_BASE + \
                                        (0x01 * EIP207_REG_OFFS))

// EIP-207s Classification Support,
// Flow Look-Up Engine (FLUE) SIZE register
#define EIP207_FLUE_FHT_REG_SIZE(f)  \
                                     (EIP207_FLUE_FHT_REG_MAP_SIZE * f + \
                                       EIP207_FLUE_FHT3_REG_BASE + \
                                        (0x00 * EIP207_REG_OFFS))

// EIP-207s Classification Support, options and version registers
#define EIP207_FLUE_FHT_REG_OPTIONS  \
                                     (EIP207_FLUE_OPTVER_REG_BASE + \
                                       (0x00 * EIP207_REG_OFFS))

#define EIP207_FLUE_FHT_REG_VERSION  \
                                     (EIP207_FLUE_OPTVER_REG_BASE + \
                                       (0x01 * EIP207_REG_OFFS))


// EIP-207s Classification Support, Flow Hash Engine (FHASH) registers
#define EIP207_FHASH_REG_IV_0(f)  \
                                     (EIP207_FLUE_FHT_REG_MAP_SIZE * f + \
                                       EIP207_FLUE_HASH_REG_BASE + \
                                        (0x00 * EIP207_REG_OFFS))

#define EIP207_FHASH_REG_IV_1(f)  \
                                     (EIP207_FLUE_FHT_REG_MAP_SIZE * f + \
                                       EIP207_FLUE_HASH_REG_BASE + \
                                        (0x01 * EIP207_REG_OFFS))

#define EIP207_FHASH_REG_IV_2(f)  \
                                     (EIP207_FLUE_FHT_REG_MAP_SIZE * f + \
                                       EIP207_FLUE_HASH_REG_BASE + \
                                        (0x02 * EIP207_REG_OFFS))

#define EIP207_FHASH_REG_IV_3(f)  \
                                     (EIP207_FLUE_FHT_REG_MAP_SIZE * f + \
                                       EIP207_FLUE_HASH_REG_BASE + \
                                        (0x03 * EIP207_REG_OFFS))


// Register default value
#define EIP207_FLUE_REG_CACHEBASE_LO_DEFAULT   0x00000000
#define EIP207_FLUE_REG_CACHEBASE_HI_DEFAULT   0x00000000
#define EIP207_FLUE_REG_HASHBASE_LO_DEFAULT    0x00000000
#define EIP207_FLUE_REG_HASHBASE_HI_DEFAULT    0x00000000
#define EIP207_FLUE_REG_SIZE_DEFAULT           0x00000000
#define EIP207_FLUE_REG_IV_DEFAULT             0x00000000


#endif /* EIP207_FLOW_HW_INTERFACE_EXT_H_ */


/* end of file eip207_flow_hw_interface_ext.h */
