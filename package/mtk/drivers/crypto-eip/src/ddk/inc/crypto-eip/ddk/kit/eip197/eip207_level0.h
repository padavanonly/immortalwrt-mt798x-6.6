/* eip207_level0.h
 *
 * EIP-207 Level0 Internal interface
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

#ifndef EIP207_LEVEL0_H_
#define EIP207_LEVEL0_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip207_global.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // BIT definitions, bool, uint32_t

// EIP-207 HW interface
#include "eip207_hw_interface.h"

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t
#include "device_rw.h"          // Read32, Write32
#include "device_swap.h"        // Device_SwapEndian32


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

/*----------------------------------------------------------------------------
 * EIP207_Read32
 *
 * This routine writes to a Register location in the EIP-207.
 */
static inline uint32_t
EIP207_Read32(
        Device_Handle_t Device,
        const unsigned int Offset)
{
    return Device_Read32(Device, Offset);
}


/*----------------------------------------------------------------------------
 * EIP207_Write32
 *
 * This routine writes to a Register location in the EIP-207.
 */
static inline void
EIP207_Write32(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t Value)
{
    Device_Write32(Device, Offset, Value);

#ifdef EIP207_CLUSTERED_WRITES_DISABLE
    // Prevent clustered write operations, break them with a read operation
    // Note: Reading the EIP207_CS_REG_VERSION register has no side effects!
    EIP207_Read32(Device, EIP207_CS_REG_VERSION);
#endif
}


static inline bool
EIP207_ICE_SIGNATURE_MATCH(
        const uint32_t Rev)
{
    return (((uint16_t)Rev) == EIP207_ICE_SIGNATURE);
}


static inline bool
EIP207_CS_SIGNATURE_MATCH(
        const uint32_t Rev)
{
    return (((uint16_t)Rev) == EIP207_CS_SIGNATURE);
}


static inline void
EIP207_CS_VERSION_RD(
        Device_Handle_t Device,
        uint8_t * const EipNumber,
        uint8_t * const ComplmtEipNumber,
        uint8_t * const HWPatchLevel,
        uint8_t * const MinHWRevision,
        uint8_t * const MajHWRevision)
{
    uint32_t RevRegVal;

    RevRegVal = EIP207_Read32(Device, EIP207_CS_REG_VERSION);

    *MajHWRevision     = (uint8_t)((RevRegVal >> 24) & MASK_4_BITS);
    *MinHWRevision     = (uint8_t)((RevRegVal >> 20) & MASK_4_BITS);
    *HWPatchLevel      = (uint8_t)((RevRegVal >> 16) & MASK_4_BITS);
    *ComplmtEipNumber  = (uint8_t)((RevRegVal >> 8)  & MASK_8_BITS);
    *EipNumber         = (uint8_t)((RevRegVal)       & MASK_8_BITS);
}


static inline void
EIP207_CS_OPTIONS_RD(
        Device_Handle_t Device,
        uint8_t * const NofLookupTables,
        bool * const fLookupCached,
        uint8_t * const NofLookupClients,
        bool * const fARC4TrcCombined,
        bool * const fARC4FrcCombined,
        bool * const fARC4Present,
        uint8_t * const NofArc4Clients,
        bool * const fTrcFrcCombined,
        uint8_t * const NofTrcClients,
        uint8_t * const NofFrcClients,
        uint8_t * const NofCacheSets)
{
    uint32_t RevRegVal;

    RevRegVal = EIP207_Read32(Device, EIP207_CS_REG_OPTIONS);

    *NofLookupTables  = (uint8_t)((RevRegVal >> 28) & MASK_3_BITS);
    *fLookupCached    = ((RevRegVal & BIT_27) != 0);
    *NofLookupClients = (uint8_t)((RevRegVal >> 22) & MASK_5_BITS);
    *fARC4TrcCombined = ((RevRegVal & BIT_21) != 0);
    *fARC4FrcCombined = ((RevRegVal & BIT_20) != 0);
    *fARC4Present     = ((RevRegVal & BIT_19) != 0);
    *NofArc4Clients   = (uint8_t)((RevRegVal >> 14) & MASK_5_BITS);
    *fTrcFrcCombined  = ((RevRegVal & BIT_13) != 0);
    *NofTrcClients    = (uint8_t)((RevRegVal >> 8)  & MASK_5_BITS);
    *NofFrcClients    = (uint8_t)((RevRegVal >> 3)  & MASK_5_BITS);
    *NofCacheSets     = (uint8_t)((RevRegVal >> 1)  & MASK_2_BITS);
}

static inline void
EIP207_ICE_ADAPT_CTRL_WR(
        Device_Handle_t Device,
        const unsigned int CEnr,
        uint32_t MaxPacketSize)
{
    uint32_t RegVal = 0xc0de0000;
    RegVal |= (MaxPacketSize & 0xfffc);
    EIP207_Write32(Device, EIP207_ICE_REG_ADAPT_CTRL(CEnr), RegVal);
}

static inline void
EIP207_ICE_VERSION_RD(
        Device_Handle_t Device,
        const unsigned int CEnr,
        uint8_t * const EipNumber,
        uint8_t * const ComplmtEipNumber,
        uint8_t * const HWPatchLevel,
        uint8_t * const MinHWRevision,
        uint8_t * const MajHWRevision)
{
    uint32_t RevRegVal;

    RevRegVal = EIP207_Read32(Device, EIP207_ICE_REG_VERSION(CEnr));

    *MajHWRevision     = (uint8_t)((RevRegVal >> 24) & MASK_4_BITS);
    *MinHWRevision     = (uint8_t)((RevRegVal >> 20) & MASK_4_BITS);
    *HWPatchLevel      = (uint8_t)((RevRegVal >> 16) & MASK_4_BITS);
    *ComplmtEipNumber  = (uint8_t)((RevRegVal >> 8)  & MASK_8_BITS);
    *EipNumber         = (uint8_t)((RevRegVal)       & MASK_8_BITS);
}


static inline void
EIP207_CS_RAM_CTRL_WR(
        const Device_Handle_t Device,
        const bool fFrc0Enable,
        const bool fFrc1Enable,
        const bool fFrc2Enable,
        const bool fTrc0Enable,
        const bool fTrc1Enable,
        const bool fTrc2Enable,
        const bool fARC4rc0Enable,
        const bool fARC4rc1Enable,
        const bool fARC4rc2Enable,
        const bool fFLUECEnable)
{
    uint32_t RegVal = EIP207_CS_REG_RAM_CTRL_DEFAULT;

    if(fFrc0Enable)
        RegVal |= BIT_0;

    if(fFrc1Enable)
        RegVal |= BIT_1;

    if(fFrc2Enable)
        RegVal |= BIT_2;

    if(fTrc0Enable)
        RegVal |= BIT_4;

    if(fTrc1Enable)
        RegVal |= BIT_5;

    if(fTrc2Enable)
        RegVal |= BIT_6;

    if(fARC4rc0Enable)
        RegVal |= BIT_8;

    if(fARC4rc1Enable)
        RegVal |= BIT_9;

    if(fARC4rc2Enable)
        RegVal |= BIT_10;

    if(fFLUECEnable)
        RegVal |= BIT_12;

    EIP207_Write32(Device, EIP207_CS_REG_RAM_CTRL, RegVal);
}


static inline void
EIP207_CS_RAM_CTRL_DEFAULT_WR(
        const Device_Handle_t Device)
{
    EIP207_Write32(Device,
                   EIP207_CS_REG_RAM_CTRL,
                   EIP207_CS_REG_RAM_CTRL_DEFAULT);
}


static inline void
EIP207_RC_CTRL_WR(
        const Device_Handle_t Device,
        const uint32_t CacheBase,
        const unsigned int CacheNr,
        const uint8_t Command)
{
    uint32_t RegVal = EIP207_RC_REG_CTRL_DEFAULT;

    RegVal &= (~MASK_4_BITS);
    RegVal |= (uint32_t)((((uint32_t)Command)   & MASK_4_BITS));

    EIP207_Write32(Device, EIP207_RC_REG_CTRL(CacheBase, CacheNr), RegVal);
}


static inline void
EIP207_RC_DATA_WR(
        const Device_Handle_t Device,
        const uint32_t CacheBase,
        const unsigned int CacheNr,
        const unsigned int ByteOffset,
        const uint32_t Value32)
{
    EIP207_Write32(Device,
                   EIP207_RC_REG_DATA(CacheBase + ByteOffset, CacheNr),
                   Value32);
}


static inline void
EIP207_ICE_PUTF_CTRL_WR(
        const Device_Handle_t Device,
        const unsigned int CENr,
        const uint8_t TokenLimit,
        const bool fFIFOReset)
{
    uint32_t RegVal = 0;
    RegVal |= TokenLimit & MASK_8_BITS;

    if (fFIFOReset)
        RegVal |= BIT_15;

    EIP207_Write32(Device, EIP207_ICE_REG_PUTF_CTRL(CENr), RegVal);
}


static inline void
EIP207_ICE_PPTF_CTRL_WR(
        const Device_Handle_t Device,
        const unsigned int CENr,
        const uint8_t TokenLimit,
        const bool fFIFOReset)
{
    uint32_t RegVal = 0;
    RegVal |= TokenLimit & MASK_8_BITS;

    if (fFIFOReset)
        RegVal |= BIT_15;

    EIP207_Write32(Device, EIP207_ICE_REG_PPTF_CTRL(CENr), RegVal);
}


static inline void
EIP207_ICE_SCRATCH_CTRL_WR(
        const Device_Handle_t Device,
        const unsigned int CENr,
        const bool fChangeTimer,
        const bool fTimerEnable,
        const uint16_t TimerPrescaler,
        const uint8_t TimerOfloBit,
        const bool fChangeAccess,
        const uint8_t ScratchAccess)
{
    uint32_t RegVal = EIP207_ICE_REG_SCRATCH_CTRL_DEFAULT;

    if(fChangeTimer)
        RegVal |= BIT_2;

    if(fTimerEnable)
        RegVal |= BIT_3;

    if(fChangeAccess)
        RegVal |= BIT_24;

    RegVal |= (uint32_t)((((uint32_t)ScratchAccess)  & MASK_4_BITS)  << 25);

    RegVal &= (~((uint32_t)(MASK_5_BITS << 16)));
    RegVal |= (uint32_t)((((uint32_t)TimerOfloBit)   & MASK_5_BITS)  << 16);

    RegVal &= (~((uint32_t)(MASK_12_BITS << 4)));
    RegVal |= (uint32_t)((((uint32_t)TimerPrescaler) & MASK_12_BITS) << 4);

    EIP207_Write32(Device, EIP207_ICE_REG_SCRATCH_CTRL(CENr), RegVal);
}


static inline void
EIP207_ICE_SCRATCH_CTRL_IRQ_CLEAR(
        const Device_Handle_t Device,
        const unsigned int CENr)
{
    uint32_t RegVal = EIP207_Read32(Device, EIP207_ICE_REG_SCRATCH_CTRL(CENr));

    // Clear Time Overflow IRQ
    RegVal |= BIT_0;

    EIP207_Write32(Device, EIP207_ICE_REG_SCRATCH_CTRL(CENr), RegVal);
}


static inline void
EIP207_ICE_SCRATCH_CTRL_RD(
        const Device_Handle_t Device,
        const unsigned int CENr,
        bool * const fTimerOfloIrq)
{
    uint32_t RegVal = EIP207_Read32(Device, EIP207_ICE_REG_SCRATCH_CTRL(CENr));

    *fTimerOfloIrq = ((RegVal & BIT_0) != 0);
}


static inline void
EIP207_ICE_RAM_CTRL_WR(
        const Device_Handle_t Device,
        const unsigned int CENr,
        const bool fPueProgEnable,
        const bool fFppProgEnable)
{
    uint32_t RegVal = EIP207_ICE_REG_RAM_CTRL_DEFAULT;

    if(fPueProgEnable)
        RegVal |= BIT_0;

    if(fFppProgEnable)
        RegVal |= BIT_1;

    EIP207_Write32(Device, EIP207_ICE_REG_RAM_CTRL(CENr), RegVal);
}


static inline void
EIP207_FHASH_IV_WR(
        const Device_Handle_t Device,
        const uint32_t IV_0,
        const uint32_t IV_1,
        const uint32_t IV_2,
        const uint32_t IV_3)
{
    EIP207_Write32(Device, EIP207_FHASH_REG_IV_0, IV_0);
    EIP207_Write32(Device, EIP207_FHASH_REG_IV_1, IV_1);
    EIP207_Write32(Device, EIP207_FHASH_REG_IV_2, IV_2);
    EIP207_Write32(Device, EIP207_FHASH_REG_IV_3, IV_3);
}


// EIP-207 Level0 interface extensions
#include "eip207_level0_ext.h"

// EIP-207 Record Cache Level0 interface extensions
#include "eip207_rc_level0_ext.h"


#endif /* EIP207_LEVEL0_H_ */


/* end of file eip207_level0.h */
