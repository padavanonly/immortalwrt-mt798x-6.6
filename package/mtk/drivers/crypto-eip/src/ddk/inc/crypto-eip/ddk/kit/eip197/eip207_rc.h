/* eip207_rc.h
 *
 * EIP-207 Record Cache (RC) interface
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

#ifndef EIP207_RC_H_
#define EIP207_RC_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip207_global.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // uint8_t, uint32_t, MASK_2_BITS

// Driver Framework Device API
#include "device_types.h"               // Device_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Record Cache commands

// Select record but do not fetch if it is not in cache
#define EIP207_RC_CMD_SELECT_NO_FETCH     0x2
#define EIP207_RC_CMD_WRITE_DATA          0x7
#define EIP207_RC_CMD_SET_BITS            0xA

#define EIP207_RC_REG_DATA_BYTE_OFFSET    0x400
#define EIP207_FRC_REG_DATA_BYTE_OFFSET   EIP207_RC_REG_DATA_BYTE_OFFSET
#define EIP207_TRC_REG_DATA_BYTE_OFFSET   EIP207_RC_REG_DATA_BYTE_OFFSET

#define EIP207_RC_HDR_WORD_3_RELOAD_BIT   BIT_15


/*----------------------------------------------------------------------------
 * EIP207_RC_BaseAddr_Set
 *
 * Set the memory base address for an EIP-207 Record Cache.
 *
 * Device (input)
 *      Device handle that identifies the Record Cache hardware
 *
 * CacheBase (input)
 *      Base address of the Record Cache, must be one of the following:
 *      EIP207_FRC_REG_BASE    - for the Flow Record Cache,
 *      EIP207_TRC_REG_BASE    - for the Transform Record Cache,
 *      EIP207_ARC4RC_REG_BASE - for the ARC4 Record Cache
 *
 * CacheNr (input)
 *      Cache set number. There can be multiple cache sets used.
 *
 * Address (input)
 *      Lower half (32 bits) of the base address.
 *
 * UpperAddress (input)
 *      Upper half (32 bits) of the base address.
 *
 * Return code
 *      None
 */
void
EIP207_RC_BaseAddr_Set(
            const Device_Handle_t Device,
            const uint32_t CacheBase,
            const unsigned int CacheNr,
            const uint32_t Address,
            const uint32_t UpperAddress);


/*----------------------------------------------------------------------------
 * EIP207_RC_Record_Update
 *
 * This function updates a requested record in the EIP-207 Record Cache.
 *
 * Device (input)
 *      Device handle that identifies the Record Cache hardware
 *
 * CacheBase (input)
 *      Base address of the Record Cache, must be one of the following:
 *      EIP207_FRC_REG_BASE    - for the Flow Record Cache,
 *      EIP207_TRC_REG_BASE    - for the Transform Record Cache,
 *      EIP207_ARC4RC_REG_BASE - for the ARC4 Record Cache
 *
 * CacheNr (input)
 *      Cache set number. There can be multiple cache sets used.
 *
 * Rec_DMA_Addr (input)
 *      DMA address of the record to be updated in the record cache.
 *
 * Command (input)
 *      Cache command that must be used for the update operation.
 *
 * ByteOffset (input)
 *      Byte offset in the record where the Value32 must be written
 *
 * Value32 (input)
 *      Value that must be written to the record in the cache.
 *
 * Return code
 *      None
 */
void
EIP207_RC_Record_Update(
        const Device_Handle_t Device,
        const uint32_t CacheBase,
        const unsigned int CacheNr,
        const uint32_t Rec_DMA_Addr,
        const uint8_t Command,
        const unsigned int ByteOffset,
        const uint32_t Value32);


/*----------------------------------------------------------------------------
 * EIP207_RC_Record_Dummy_Addr_Get
 *
 * This function returns the dummy record address.
 */
unsigned int
EIP207_RC_Record_Dummy_Addr_Get(void);


/* end of file eip207_rc.h */


#endif /* EIP207_RC_H_ */
