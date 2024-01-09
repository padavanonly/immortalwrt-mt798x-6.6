/* eip206_level0.h
 *
 * EIP-206 Processing Engine Level0 Internal interface
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

#ifndef EIP206_LEVEL0_H_
#define EIP206_LEVEL0_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // BIT definitions, bool, uint32_t

// EIP-206 HW interface
#include "eip206_hw_interface.h"

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t
#include "device_rw.h"          // Read32, Write32


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP206_Read32
 *
 * This routine writes to a Register location in the EIP-206.
 */
static inline uint32_t
EIP206_Read32(
        Device_Handle_t Device,
        const unsigned int Offset)
{
    return Device_Read32(Device, Offset);
}


/*----------------------------------------------------------------------------
 * EIP206_Write32
 *
 * This routine writes to a Register location in the EIP-206.
 */
static inline void
EIP206_Write32(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t Value)
{
    Device_Write32(Device, Offset, Value);
}


static inline bool
EIP206_REV_SIGNATURE_MATCH(
        const uint32_t Rev)
{
    return (((uint16_t)Rev) == EIP206_SIGNATURE);
}


static inline void
EIP206_EIP_REV_RD(
        Device_Handle_t Device,
        const unsigned int PEnr,
        uint8_t * const EipNumber,
        uint8_t * const ComplmtEipNumber,
        uint8_t * const HWPatchLevel,
        uint8_t * const MinHWRevision,
        uint8_t * const MajHWRevision)
{
    uint32_t RevRegVal;

    RevRegVal = EIP206_Read32(Device, EIP206_REG_VERSION(PEnr));

    *MajHWRevision     = (uint8_t)((RevRegVal >> 24) & MASK_4_BITS);
    *MinHWRevision     = (uint8_t)((RevRegVal >> 20) & MASK_4_BITS);
    *HWPatchLevel      = (uint8_t)((RevRegVal >> 16) & MASK_4_BITS);
    *ComplmtEipNumber  = (uint8_t)((RevRegVal >> 8)  & MASK_8_BITS);
    *EipNumber         = (uint8_t)((RevRegVal)       & MASK_8_BITS);
}


static inline void
EIP206_OPTIONS_RD(
        Device_Handle_t Device,
        const unsigned int PEnr,
        uint8_t * const PE_Type,
        uint8_t * const InClassifier,
        uint8_t * const OutClassifier,
        uint8_t * const NofMAC_Channels,
        uint8_t * const InDbufSizeKB,
        uint8_t * const InTbufSizeKB,
        uint8_t * const OutDbufSizeKB,
        uint8_t * const OutTbufSizeKB)
{
    uint32_t RevRegVal;

    RevRegVal = EIP206_Read32(Device, EIP206_REG_OPTIONS(PEnr));

    *OutTbufSizeKB   = (uint8_t)((RevRegVal >> 28) & MASK_4_BITS);
    *OutDbufSizeKB   = (uint8_t)((RevRegVal >> 24) & MASK_4_BITS);
    *InTbufSizeKB    = (uint8_t)((RevRegVal >> 20) & MASK_4_BITS);
    *InDbufSizeKB    = (uint8_t)((RevRegVal >> 16) & MASK_4_BITS);
    *NofMAC_Channels = (uint8_t)((RevRegVal >> 12) & MASK_4_BITS);
    *OutClassifier   = (uint8_t)((RevRegVal >> 10) & MASK_2_BITS);
    *InClassifier    = (uint8_t)((RevRegVal >> 8)  & MASK_2_BITS);
    *PE_Type         = (uint8_t)((RevRegVal)       & MASK_8_BITS);
}


static inline void
EIP206_IN_DEBUG_WR(
        Device_Handle_t Device,
        const unsigned int PEnr,
        const bool fBypass,
        const bool fInFlightClear,
        const bool fMetaEnb)
{
    uint32_t RegVal = 0;

    if (fBypass)
        RegVal |= BIT_0;

    if (fInFlightClear)
        RegVal |= BIT_16;

    if (fMetaEnb)
        RegVal |= BIT_31;
    EIP206_Write32(Device,EIP206_REG_DEBUG(PEnr),RegVal);
}


static inline void
EIP206_IN_DBUF_THRESH_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int PEnr)
{
    EIP206_Write32(Device,
                   EIP206_IN_REG_DBUF_TRESH(PEnr),
                   EIP206_IN_REG_DBUF_TRESH_DEFAULT);
}


static inline void
EIP206_IN_DBUF_THRESH_WR(
        Device_Handle_t Device,
        const unsigned int PEnr,
        const uint8_t PktTresh,
        const uint8_t MinTresh,
        const uint8_t MaxTresh)
{
    uint32_t RegVal = EIP206_IN_REG_DBUF_TRESH_DEFAULT;

    RegVal |= (uint32_t)((((uint32_t)PktTresh) & MASK_8_BITS));
    RegVal |= (uint32_t)((((uint32_t)MaxTresh) & MASK_4_BITS) << 12);
    RegVal |= (uint32_t)((((uint32_t)MinTresh) & MASK_4_BITS) << 8);

    EIP206_Write32(Device, EIP206_IN_REG_DBUF_TRESH(PEnr), RegVal);
}


static inline void
EIP206_IN_TBUF_THRESH_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int PEnr)
{
    EIP206_Write32(Device,
                   EIP206_IN_REG_TBUF_TRESH(PEnr),
                   EIP206_IN_REG_TBUF_TRESH_DEFAULT);
}


static inline void
EIP206_IN_TBUF_THRESH_WR(
        Device_Handle_t Device,
        const unsigned int PEnr,
        const uint8_t PktTresh,
        const uint8_t MinTresh,
        const uint8_t MaxTresh)
{
    uint32_t RegVal = EIP206_IN_REG_TBUF_TRESH_DEFAULT;

    RegVal |= (uint32_t)((((uint32_t)PktTresh) & MASK_8_BITS));
    RegVal |= (uint32_t)((((uint32_t)MaxTresh) & MASK_4_BITS) << 12);
    RegVal |= (uint32_t)((((uint32_t)MinTresh) & MASK_4_BITS) << 8);

    EIP206_Write32(Device, EIP206_IN_REG_TBUF_TRESH(PEnr), RegVal);
}


static inline void
EIP206_OUT_DBUF_THRESH_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int PEnr)
{
    EIP206_Write32(Device,
                   EIP206_OUT_REG_DBUF_TRESH(PEnr),
                   EIP206_OUT_REG_DBUF_TRESH_DEFAULT);
}


static inline void
EIP206_OUT_DBUF_THRESH_WR(
        Device_Handle_t Device,
        const unsigned int PEnr,
        const uint8_t MinTresh,
        const uint8_t MaxTresh)
{
    uint32_t RegVal = EIP206_OUT_REG_DBUF_TRESH_DEFAULT;

    RegVal |= (uint32_t)((((uint32_t)MaxTresh) & MASK_4_BITS) << 4);
    RegVal |= (uint32_t)((((uint32_t)MinTresh) & MASK_4_BITS));

    EIP206_Write32(Device, EIP206_OUT_REG_DBUF_TRESH(PEnr), RegVal);
}


static inline void
EIP206_OUT_TBUF_THRESH_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int PEnr)
{
    EIP206_Write32(Device,
                   EIP206_OUT_REG_TBUF_TRESH(PEnr),
                   EIP206_OUT_REG_TBUF_TRESH_DEFAULT);
}


static inline void
EIP206_OUT_TBUF_THRESH_WR(
        Device_Handle_t Device,
        const unsigned int PEnr,
        const uint8_t MinTresh,
        const uint8_t MaxTresh)
{
    uint32_t RegVal = EIP206_OUT_REG_TBUF_TRESH_DEFAULT;

    RegVal |= (uint32_t)((((uint32_t)MaxTresh) & MASK_4_BITS) << 4);
    RegVal |= (uint32_t)((((uint32_t)MinTresh) & MASK_4_BITS));

    EIP206_Write32(Device, EIP206_OUT_REG_TBUF_TRESH(PEnr), RegVal);
}

static inline void
EIP206_ARC4_SIZE_WR(
        Device_Handle_t Device,
        const unsigned int PEnr,
        const bool fLarge)
{
    uint32_t RegVal = EIP206_ARC4_REG_SIZE_DEFAULT;

    if(fLarge)
        RegVal |= BIT_0;

    EIP206_Write32(Device, EIP206_ARC4_REG_SIZE(PEnr), RegVal);
}



#endif /* EIP206_LEVEL0_H_ */


/* end of file eip206_level0.h */
