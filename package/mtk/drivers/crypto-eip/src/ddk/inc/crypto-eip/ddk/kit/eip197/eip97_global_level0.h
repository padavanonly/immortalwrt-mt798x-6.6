/* eip97_global_level0.h
 *
 * EIP-97 Global Control Level0 Internal interface
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

#ifndef EIP97_GLOBAL_LEVEL0_H_
#define EIP97_GLOBAL_LEVEL0_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip97_global.h"     // EIP97_RC_BASE, EIP97_BASE

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // BIT definitions, bool, uint32_t

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t
#include "device_rw.h"          // Read32, Write32

#define EIP97_REG_MST_CTRL_DEFAULT      0x00000000

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Read/Write register constants

// Byte offsets of the EIP-97 Engine registers

// EIP-97 Engine EIP number (0x61) and complement (0x9E)
#define EIP97_SIGNATURE    ((uint16_t)0x9E61)

#define EIP97_REG_OFFS     4

#define EIP97_REG_CS_DMA_CTRL2  ((EIP97_RC_BASE)+(0x01 * EIP97_REG_OFFS))

#define EIP97_REG_MST_CTRL ((EIP97_BASE)+(0x00 * EIP97_REG_OFFS))
#define EIP97_REG_OPTIONS  ((EIP97_BASE)+(0x01 * EIP97_REG_OFFS))
#define EIP97_REG_VERSION  ((EIP97_BASE)+(0x02 * EIP97_REG_OFFS))

#define EIP97_REG_CS_DMA_CTRL2_DEFAULT  0x00000000


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP97_Read32
 *
 * This routine writes to a Register location in the EIP-97.
 */
static inline uint32_t
EIP97_Read32(
        Device_Handle_t Device,
        const unsigned int Offset)
{
    return Device_Read32(Device, Offset);
}


/*----------------------------------------------------------------------------
 * EIP97_Write32
 *
 * This routine writes to a Register location in the EIP97.
 */
static inline void
EIP97_Write32(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t Value)
{
    Device_Write32(Device, Offset, Value);
}


static inline bool
EIP97_REV_SIGNATURE_MATCH(
        const uint32_t Rev)
{
    return (((uint16_t)Rev) == EIP97_SIGNATURE);
}


static inline void
EIP97_EIP_REV_RD(
        Device_Handle_t Device,
        uint8_t * const EipNumber,
        uint8_t * const ComplmtEipNumber,
        uint8_t * const HWPatchLevel,
        uint8_t * const MinHWRevision,
        uint8_t * const MajHWRevision)
{
    uint32_t RevRegVal;

    RevRegVal = EIP97_Read32(Device, EIP97_REG_VERSION);

    *MajHWRevision     = (uint8_t)((RevRegVal >> 24) & MASK_4_BITS);
    *MinHWRevision     = (uint8_t)((RevRegVal >> 20) & MASK_4_BITS);
    *HWPatchLevel      = (uint8_t)((RevRegVal >> 16) & MASK_4_BITS);
    *ComplmtEipNumber  = (uint8_t)((RevRegVal >> 8)  & MASK_8_BITS);
    *EipNumber         = (uint8_t)((RevRegVal)       & MASK_8_BITS);
}


static inline void
EIP97_MST_CTRL_WR(
        const Device_Handle_t Device,
        const unsigned int PENr,
        const uint8_t RdCache,
        const uint8_t WrCache,
        const uint8_t SwapMethod,
        const uint8_t Protection,
        const uint8_t CtxCacheAlign,
        const bool fWriteCacheNoBuf)
{
    uint32_t RegVal = EIP97_REG_MST_CTRL_DEFAULT;

    IDENTIFIER_NOT_USED(PENr);

    RegVal |= (uint32_t)((((uint32_t)fWriteCacheNoBuf) & MASK_1_BIT) << 18);
    RegVal |= (uint32_t)((((uint32_t)CtxCacheAlign) & MASK_2_BITS) << 16);
    RegVal |= (uint32_t)((((uint32_t)Protection) & MASK_4_BITS) << 12);
    RegVal |= (uint32_t)((((uint32_t)SwapMethod) & MASK_4_BITS) << 8);
    RegVal |= (uint32_t)((((uint32_t)WrCache)    & MASK_4_BITS) << 4);
    RegVal |= (uint32_t)((((uint32_t)RdCache)    & MASK_4_BITS));

    EIP97_Write32(Device, EIP97_REG_MST_CTRL, RegVal);
}


static inline void
EIP97_CS_DMA_CTRL2_WR(
        const Device_Handle_t Device,
        const uint8_t FLUE_Swap_Mask,
        const uint8_t FRC_Swap_Mask,
        const uint8_t TRC_Swap_Mask,
        const uint8_t ARC4_Swap_Mask)
{
    uint32_t RegVal = EIP97_REG_CS_DMA_CTRL2_DEFAULT;

    RegVal |= (uint32_t)((((uint32_t)FLUE_Swap_Mask) & MASK_4_BITS) << 24);
    RegVal |= (uint32_t)((((uint32_t)ARC4_Swap_Mask) & MASK_4_BITS) << 16);
    RegVal |= (uint32_t)((((uint32_t)TRC_Swap_Mask)  & MASK_4_BITS) << 8);
    RegVal |= (uint32_t)((((uint32_t)FRC_Swap_Mask)  & MASK_4_BITS));

    EIP97_Write32(Device, EIP97_REG_CS_DMA_CTRL2, RegVal);
}


// EIP-97 Global Control Level0 Internal API extensions
#define EIP97_REG_DBG_BASE 0xffb00
#define EIP97_REG_DBG_RING_IN_COUNT_LO(n)  (EIP97_REG_DBG_BASE + 0x00 + 16*(n))
#define EIP97_REG_DBG_RING_IN_COUNT_HI(n)  (EIP97_REG_DBG_BASE + 0x04 + 16*(n))
#define EIP97_REG_DBG_RING_OUT_COUNT_LO(n) (EIP97_REG_DBG_BASE + 0x08 + 16*(n))
#define EIP97_REG_DBG_RING_OUT_COUNT_HI(n) (EIP97_REG_DBG_BASE + 0x0c + 16*(n))
#define EIP97_REG_DBG_PIPE_COUNT(n)  (EIP97_REG_DBG_BASE + 0x100 + 16*(n))
#define EIP97_REG_DBG_PIPE_STATE(n)  (EIP97_REG_DBG_BASE + 0x104 + 16*(n))
#define EIP97_REG_DBG_PIPE_DCOUNT_LO(n)  (EIP97_REG_DBG_BASE + 0x108 + 16*(n))
#define EIP97_REG_DBG_PIPE_DCOUNT_HI(n)  (EIP97_REG_DBG_BASE + 0x10c + 16*(n))

/*----------------------------------------------------------------------------
 * Local variables
 */

static inline void
EIP97_OPTIONS_RD(
        Device_Handle_t Device,
        uint8_t * const NofPes,
        uint8_t * const InTbufSize,
        uint8_t * const InDbufSize,
        uint8_t * const OutTbufSize,
        uint8_t * const OutDbufSize,
        bool * const fCentralPRNG,
        bool * const fTG,
        bool * const fTRC)
{
    uint32_t RevRegVal;

    RevRegVal = EIP97_Read32(Device, EIP97_REG_OPTIONS);

    *fTRC         = ((RevRegVal & BIT_31) != 0);
    *fTG          = ((RevRegVal & BIT_27) != 0);
    *fCentralPRNG = ((RevRegVal & BIT_26) != 0);
    *OutDbufSize  = (uint8_t)((RevRegVal >> 16) & MASK_4_BITS);
    *OutTbufSize  = (uint8_t)((RevRegVal >> 13) & MASK_3_BITS) + 3;
    *InDbufSize   = (uint8_t)((RevRegVal >> 9)  & MASK_4_BITS);
    *InTbufSize   = (uint8_t)((RevRegVal >> 6)  & MASK_3_BITS) + 3;
    *NofPes       = (uint8_t)((RevRegVal)       & MASK_5_BITS);
}

static inline void
EIP97_DBG_RING_IN_COUNT_RD(
        Device_Handle_t Device,
        unsigned int PENr,
        uint64_t * const Counter)
{
    uint32_t val_lo,val_hi;
    val_lo = EIP97_Read32(Device, EIP97_REG_DBG_RING_IN_COUNT_LO(PENr));
    val_hi = EIP97_Read32(Device, EIP97_REG_DBG_RING_IN_COUNT_HI(PENr));

    *Counter = ((uint64_t)val_hi << 32) | ((uint64_t)val_lo);
}

static inline void
EIP97_DBG_RING_OUT_COUNT_RD(
        Device_Handle_t Device,
        unsigned int PENr,
        uint64_t * const Counter)
{
    uint32_t val_lo,val_hi;
    val_lo = EIP97_Read32(Device, EIP97_REG_DBG_RING_OUT_COUNT_LO(PENr));
    val_hi = EIP97_Read32(Device, EIP97_REG_DBG_RING_OUT_COUNT_HI(PENr));

    *Counter = ((uint64_t)val_hi << 32) | ((uint64_t)val_lo);
}


static inline void
EIP97_DBG_PIPE_COUNT_RD(
        Device_Handle_t Device,
        unsigned int PENr,
        uint64_t * const TotalPackets,
        uint8_t * const CurrentPackets,
        uint8_t * const MaxPackets)
{
    uint32_t val_lo,val_hi;
    val_lo = EIP97_Read32(Device, EIP97_REG_DBG_PIPE_COUNT(PENr));
    val_hi = EIP97_Read32(Device, EIP97_REG_DBG_PIPE_STATE(PENr));

    *TotalPackets = (((uint64_t)val_hi & MASK_16_BITS )<< 32) | ((uint64_t)val_lo);
    *CurrentPackets = (val_hi >> 16) & MASK_8_BITS;
    *MaxPackets = (val_hi >> 24) & MASK_8_BITS;
}


static inline void
EIP97_DBG_PIPE_DCOUNT_RD(
        Device_Handle_t Device,
        unsigned int PENr,
        uint64_t * const Counter)
{
    uint32_t val_lo,val_hi;
    val_lo = EIP97_Read32(Device, EIP97_REG_DBG_PIPE_DCOUNT_LO(PENr));
    val_hi = EIP97_Read32(Device, EIP97_REG_DBG_PIPE_DCOUNT_HI(PENr));

    *Counter = ((uint64_t)val_hi << 32) | ((uint64_t)val_lo);
}

#endif /* EIP97_GLOBAL_LEVEL0_H_ */


/* end of file eip97_global_level0.h */
