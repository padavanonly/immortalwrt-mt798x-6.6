/* eip207_rc_level0_ext.h
 *
 * EIP-207 Record Cache Level0 Internal interface High-Performance (HP)
 * extensions
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

#ifndef EIP207_RC_LEVEL0_EXT_H_
#define EIP207_RC_LEVEL0_EXT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"             // BIT definitions, bool, uint32_t

// EIP-207 HW interface
#include "eip207_hw_interface.h"    // EIP207_RC_REG_*

// Driver Framework Device API
#include "device_types.h"           // Device_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP207 Global Register Write/Read Functions
 *
 */

static inline void
EIP207_RC_PARAMS_WR(
        const Device_Handle_t Device,
        const uint32_t CacheBase,
        const unsigned int CacheNr,
        const bool fSWReset,
        const bool fBlock,
        const bool fDataAccess,
        const uint8_t HashTableMask,
        const uint8_t BlockClockCount,
        const bool fMaxRecSize,
        const uint16_t RecordWordCount)
{
    uint32_t RegVal = EIP207_RC_REG_PARAMS_DEFAULT;

    IDENTIFIER_NOT_USED(fMaxRecSize);

    if (!fSWReset)
        RegVal &= (~BIT_0);

    if (fBlock)
        RegVal |= BIT_1;

    if (fDataAccess)
        RegVal |= BIT_2;

    RegVal &= (~((uint32_t)(MASK_9_BITS << 18)));
    RegVal |= (uint32_t)((((uint32_t)RecordWordCount) & MASK_9_BITS) << 18);

    RegVal &= (~((uint32_t)(MASK_3_BITS << 10)));
    RegVal |= (uint32_t)((((uint32_t)BlockClockCount) & MASK_3_BITS) << 10);

    RegVal |= (uint32_t)((((uint32_t)HashTableMask)   & MASK_3_BITS) << 4);

    EIP207_Write32(Device, EIP207_RC_REG_PARAMS(CacheBase, CacheNr), RegVal);
}


static inline void
EIP207_RC_PARAMS_RD(
        const Device_Handle_t Device,
        const uint32_t CacheBase,
        const unsigned int CacheNr,
        bool * const fDMAReadError_p,
        bool * const fDMAWriteError_p)
{
    uint32_t RegVal;

    RegVal = EIP207_Read32(Device, EIP207_RC_REG_PARAMS(CacheBase, CacheNr));

    *fDMAReadError_p  = ((RegVal & BIT_30) != 0);
    *fDMAWriteError_p = ((RegVal & BIT_31) != 0);
}


static inline void
EIP207_RC_FREECHAIN_WR(
        const Device_Handle_t Device,
        const uint32_t CacheBase,
        const unsigned int CacheNr,
        const uint16_t HeadOffset,
        const uint16_t TailOffset)
{
    uint32_t RegVal = EIP207_RC_REG_PARAMS2_DEFAULT;

    RegVal |= (uint32_t)((((uint32_t)TailOffset) & MASK_10_BITS) << 16);
    RegVal |= (uint32_t)((((uint32_t)HeadOffset) & MASK_10_BITS));

    EIP207_Write32(Device, EIP207_RC_REG_FREECHAIN(CacheBase,CacheNr), RegVal);
}


static inline void
EIP207_RC_PARAMS2_WR(
        const Device_Handle_t Device,
        const uint32_t CacheBase,
        const unsigned int CacheNr,
        const uint16_t HashTableStart,
        const uint16_t Record2WordCount,
        const uint8_t DMAWrCombDly)
{
    uint32_t RegVal = EIP207_RC_REG_PARAMS2_DEFAULT;

    RegVal |= (uint32_t)((((uint32_t)Record2WordCount) & MASK_9_BITS) << 18);
    RegVal |= (uint32_t)((((uint32_t)DMAWrCombDly)     & MASK_8_BITS) << 10);
    RegVal |= (uint32_t)((((uint32_t)HashTableStart)   & MASK_10_BITS));

    EIP207_Write32(Device, EIP207_RC_REG_PARAMS2(CacheBase, CacheNr), RegVal);
}


static inline void
EIP207_RC_REGINDEX_WR(
        const Device_Handle_t Device,
        const uint32_t CacheBase,
        const unsigned int CacheNr,
        const uint8_t Regindex)
{
    uint32_t RegVal = EIP207_RC_REG_REGINDEX_DEFAULT;

    RegVal |= (uint32_t)((((uint32_t)Regindex) & MASK_6_BITS));

    EIP207_Write32(Device, EIP207_RC_REG_REGINDEX(CacheBase, CacheNr), RegVal);
}


static inline void
EIP207_RC_ECCCTRL_WR(
        const Device_Handle_t Device,
        const uint32_t CacheBase,
        const unsigned int CacheNr,
        const bool fDataEccOflo,
        const bool fDataEccErr,
        const bool fAdminEccErr)
{
    uint32_t RegVal = EIP207_RC_REG_ECCCRTL_DEFAULT;

    if (fDataEccOflo)
        RegVal |= BIT_29;

    if (fDataEccErr)
        RegVal |= BIT_30;

    if (fAdminEccErr)
        RegVal |= BIT_31;

    EIP207_Write32(Device, EIP207_RC_REG_ECCCTRL(CacheBase, CacheNr), RegVal);
}


static inline void
EIP207_RC_ECCCTRL_RD_CLEAR(
        const Device_Handle_t Device,
        const uint32_t CacheBase,
        const unsigned int CacheNr,
        bool * const fDataEccOflo_p,
        bool * const fDataEccErr_p,
        bool * const fAdminEccErr_p)
{
    uint32_t RegVal;

    RegVal = EIP207_Read32(Device, EIP207_RC_REG_ECCCTRL(CacheBase, CacheNr));

    *fDataEccOflo_p  = ((RegVal & BIT_29) != 0);
    *fDataEccErr_p   = ((RegVal & BIT_30) != 0);
    *fAdminEccErr_p  = ((RegVal & BIT_31) != 0);

    // Clear non-correctable (but non-fatal) RC Data RAM ECC error if detected
    if (*fDataEccErr_p)
    {
        // *fDataEccOflo_p:
        // Do not clear fatal RC Data RAM non-correctable error here

        // *fAdminEccErr_p:
        // Do not clear fatal RC Administration RAM non-correctable error here

        // Do not clear anything in admin_corr_mask
        RegVal &= (~MASK_12_BITS);

        if(*fDataEccErr_p)
            RegVal &= (~BIT_30); // Write 0 to clear the error '1' status bit

        EIP207_Write32(Device,
                       EIP207_RC_REG_ECCCTRL(CacheBase, CacheNr),
                       RegVal);
    }
}

static inline void
EIP207_RC_LONGCTR_RD(
        const Device_Handle_t Device,
        unsigned int RegOffset,
        uint64_t *Counter)
{
    uint32_t val_lo, val_hi;
    val_lo = EIP207_Read32(Device, RegOffset);
    val_hi = EIP207_Read32(Device, RegOffset + 4) & MASK_8_BITS;

    *Counter = (((uint64_t) val_hi) << 32) | val_lo;
}

static inline void
EIP207_RC_SHORTCTR_RD(
        const Device_Handle_t Device,
        unsigned int RegOffset,
        uint32_t *Counter)
{
    *Counter = EIP207_Read32(Device, RegOffset) & MASK_24_BITS;
}

static inline void
EIP207_RC_RDMAERRFLGS_RD(
        const Device_Handle_t Device,
        const uint32_t CacheBase,
        const unsigned int CacheNr,
        uint32_t *ErrFlags)
{
    *ErrFlags = EIP207_Read32(Device,
                              EIP207_RC_REG_RDMAERRFLGS_0(CacheBase,CacheNr)) &
                             MASK_8_BITS;
}

#endif /* EIP207_RC_LEVEL0_EXT_H_ */

/* end of file eip207_rc_level0_ext.h */
