/* eip207_level0_ext.h
 *
 * EIP-207 Level0 Internal interface extensions
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

#ifndef EIP207_LEVEL0_EXT_H_
#define EIP207_LEVEL0_EXT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"             // BIT definitions, bool, uint32_t

// EIP-207 HW interface
#include "eip207_hw_interface.h"    // EIP207_FLUE_REG_*

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
EIP207_FLUE_CONFIG_WR(
        const Device_Handle_t Device,
        const unsigned int HashTableId,
        const uint8_t Function,
        const uint8_t Generation,
        const bool fTableEnable,
        const bool fXSEnable)
{
    uint32_t RegVal = EIP207_FLUE_REG_CONFIG_DEFAULT;

    if (fTableEnable)
        RegVal |= BIT_30;

    if (fXSEnable)
        RegVal |= BIT_31;

    RegVal |= (uint32_t)((((uint32_t)Generation) & MASK_3_BITS) << 24);
    RegVal |= (uint32_t)((((uint32_t)Function)   & MASK_8_BITS) << 16);

    EIP207_Write32(Device, EIP207_FLUE_REG_CONFIG(HashTableId), RegVal);
}


static inline void
EIP207_FLUE_OFFSET_WR(
        const Device_Handle_t Device,
        const bool fPrefetchXform,
        const bool fLookupCached,
        const uint8_t XformRecordWordOffset)
{
    uint32_t RegVal = EIP207_FLUE_REG_OFFSETS_DEFAULT;

    if (fPrefetchXform)
        RegVal |= BIT_2;

    if (fLookupCached)
        RegVal |= BIT_3;

    RegVal |=
          (uint32_t)((((uint32_t)XformRecordWordOffset) & MASK_8_BITS) << 24);

    EIP207_Write32(Device, EIP207_FLUE_REG_OFFSETS, RegVal);
}


static inline void
EIP207_FLUE_ARC4_OFFSET_WR(
        const Device_Handle_t Device,
        const bool fPrefetchARC4,
        const bool f3EntryLookup,
        const uint8_t ARC4RecordWordOffset)
{
    uint32_t RegVal = EIP207_FLUE_REG_ARC4_OFFSET_DEFAULT;

    if (fPrefetchARC4)
        RegVal |= BIT_2;

    if (f3EntryLookup)
        RegVal |= BIT_3;

    RegVal |=
          (uint32_t)((((uint32_t)ARC4RecordWordOffset) & MASK_8_BITS) << 24);

    EIP207_Write32(Device, EIP207_FLUE_REG_ARC4_OFFSET, RegVal);
}


static inline void
EIP207_ICE_PUE_CTRL_WR(
        const Device_Handle_t Device,
        const unsigned int CENr,
        const uint16_t CurrentPc,
        const bool fEccCorr,
        const bool fEccDerr,
        const bool fDebugReset,
        const bool fSWReset)
{
    uint32_t RegVal = EIP207_ICE_REG_PUE_CTRL_DEFAULT;

    if (!fSWReset)
        RegVal &= (~BIT_0);

    if(fDebugReset)
        RegVal |= BIT_3;

    if(fEccDerr)
        RegVal |= BIT_14;

    if(fEccCorr)
        RegVal |= BIT_15;

    RegVal |= (uint32_t)((((uint32_t)CurrentPc) & MASK_15_BITS) << 16);

    EIP207_Write32(Device, EIP207_ICE_REG_PUE_CTRL(CENr), RegVal);
}


static inline void
EIP207_ICE_PUE_CTRL_RD_CLEAR(
        const Device_Handle_t Device,
        const unsigned int CENr,
        bool * const fEccCorr_p,
        bool * const fEccDerr_p)
{
    uint32_t RegVal;

    RegVal = EIP207_Read32(Device, EIP207_ICE_REG_PUE_CTRL(CENr));

    *fEccCorr_p = ((RegVal & BIT_15) != 0);
    *fEccDerr_p = ((RegVal & BIT_14) != 0);

    // Clear correctable ECC error is detected
    if (*fEccCorr_p )
    {
        if(*fEccDerr_p)
            RegVal &= (~BIT_14);

        EIP207_Write32(Device, EIP207_ICE_REG_PUE_CTRL(CENr), RegVal);
    }
}


static inline void
EIP207_ICE_FPP_CTRL_WR(
        const Device_Handle_t Device,
        const unsigned int CENr,
        const uint16_t CurrentPc,
        const bool fEccCorr,
        const bool fEccDerr,
        const bool fDebugReset,
        const bool fSWReset)
{
    uint32_t RegVal = EIP207_ICE_REG_FPP_CTRL_DEFAULT;

    if (!fSWReset)
        RegVal &= (~BIT_0);

    if (fDebugReset)
        RegVal |= BIT_3;

    if(fEccDerr)
        RegVal |= BIT_14;

    if(fEccCorr)
        RegVal |= BIT_15;

    RegVal |= (uint32_t)((((uint32_t)CurrentPc) & MASK_15_BITS) << 16);

    EIP207_Write32(Device, EIP207_ICE_REG_FPP_CTRL(CENr), RegVal);
}


static inline void
EIP207_ICE_FPP_CTRL_RD_CLEAR(
        const Device_Handle_t Device,
        const unsigned int CENr,
        bool * const fEccCorr_p,
        bool * const fEccDerr_p)
{
    uint32_t RegVal;

    RegVal = EIP207_Read32(Device, EIP207_ICE_REG_FPP_CTRL(CENr));

    *fEccCorr_p = ((RegVal & BIT_15) != 0);
    *fEccDerr_p = ((RegVal & BIT_14) != 0);

    // Clear correctable ECC error is detected
    if (*fEccCorr_p )
    {
        if(*fEccDerr_p)
            RegVal &= (~BIT_14);

        EIP207_Write32(Device, EIP207_ICE_REG_FPP_CTRL(CENr), RegVal);
    }
}


static inline void
EIP207_OCE_SCRATCH_CTRL_WR(
        const Device_Handle_t Device,
        const unsigned int CENr,
        const bool fChangeTimer,
        const bool fTimerEnable,
        const uint16_t TimerPrescaler,
        const uint8_t TimerOfloBit,
        const bool fChangeAccess,
        const uint8_t ScratchAccess)
{
    uint32_t RegVal = EIP207_OCE_REG_SCRATCH_CTRL_DEFAULT;

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

    EIP207_Write32(Device, EIP207_OCE_REG_SCRATCH_CTRL(CENr), RegVal);
}


static inline void
EIP207_OCE_SCRATCH_CTRL_RD(
        const Device_Handle_t Device,
        const unsigned int CENr,
        bool * const fTimerOfloIrq)
{
    uint32_t RegVal = EIP207_Read32(Device, EIP207_OCE_REG_SCRATCH_CTRL(CENr));

    *fTimerOfloIrq = ((RegVal & BIT_0) != 0);
}


static inline void
EIP207_OCE_PUE_CTRL_WR(
        const Device_Handle_t Device,
        const unsigned int CENr,
        const uint16_t CurrentPc,
        const bool fEccCorr,
        const bool fEccDerr,
        const bool fDebugReset,
        const bool fSWReset)
{
    uint32_t RegVal = EIP207_OCE_REG_PUE_CTRL_DEFAULT;

    if (!fSWReset)
        RegVal &= (~BIT_0);

    if(fDebugReset)
        RegVal |= BIT_3;

    if(fEccDerr)
        RegVal |= BIT_14;

    if(fEccCorr)
        RegVal |= BIT_15;

    RegVal |= (uint32_t)((((uint32_t)CurrentPc) & MASK_15_BITS) << 16);

    EIP207_Write32(Device, EIP207_OCE_REG_PUE_CTRL(CENr), RegVal);
}


static inline void
EIP207_OCE_PUE_CTRL_RD_CLEAR(
        const Device_Handle_t Device,
        const unsigned int CENr,
        bool * const fEccCorr_p,
        bool * const fEccDerr_p)
{
    uint32_t RegVal;

    RegVal = EIP207_Read32(Device, EIP207_OCE_REG_PUE_CTRL(CENr));

    *fEccCorr_p = ((RegVal & BIT_15) != 0);
    *fEccDerr_p = ((RegVal & BIT_14) != 0);

    // Clear correctable ECC error is detected
    if (*fEccCorr_p )
    {
        if(*fEccDerr_p)
            RegVal &= (~BIT_14);

        EIP207_Write32(Device, EIP207_OCE_REG_PUE_CTRL(CENr), RegVal);
    }
}


static inline void
EIP207_OCE_FPP_CTRL_WR(
        const Device_Handle_t Device,
        const unsigned int CENr,
        const uint16_t CurrentPc,
        const bool fEccCorr,
        const bool fEccDerr,
        const bool fDebugReset,
        const bool fSWReset)
{
    uint32_t RegVal = EIP207_OCE_REG_FPP_CTRL_DEFAULT;

    if (!fSWReset)
        RegVal &= (~BIT_0);

    if (fDebugReset)
        RegVal |= BIT_3;

    if(fEccDerr)
        RegVal |= BIT_14;

    if(fEccCorr)
        RegVal |= BIT_15;

    RegVal |= (uint32_t)((((uint32_t)CurrentPc) & MASK_15_BITS) << 16);

    EIP207_Write32(Device, EIP207_OCE_REG_FPP_CTRL(CENr), RegVal);
}


static inline void
EIP207_OCE_FPP_CTRL_RD_CLEAR(
        const Device_Handle_t Device,
        const unsigned int CENr,
        bool * const fEccCorr_p,
        bool * const fEccDerr_p)
{
    uint32_t RegVal;

    RegVal = EIP207_Read32(Device, EIP207_OCE_REG_FPP_CTRL(CENr));

    *fEccCorr_p = ((RegVal & BIT_15) != 0);
    *fEccDerr_p = ((RegVal & BIT_14) != 0);

    // Clear correctable ECC error is detected
    if (*fEccCorr_p )
    {
        if(*fEccDerr_p)
            RegVal &= (~BIT_14);

        EIP207_Write32(Device, EIP207_OCE_REG_FPP_CTRL(CENr), RegVal);
    }
}


static inline void
EIP207_OCE_RAM_CTRL_WR(
        const Device_Handle_t Device,
        const unsigned int CENr,
        const bool fPueProgEnable,
        const bool fFppProgEnable)
{
    uint32_t RegVal = EIP207_OCE_REG_RAM_CTRL_DEFAULT;

    if(fPueProgEnable)
        RegVal |= BIT_0;

    if(fFppProgEnable)
        RegVal |= BIT_1;

    EIP207_Write32(Device, EIP207_OCE_REG_RAM_CTRL(CENr), RegVal);
}


#ifdef EIP207_FLUE_HAVE_VIRTUALIZATION

static inline void
EIP207_FLUE_IFC_LUT_WR(
        const Device_Handle_t Device,
        const unsigned int RegNr,
        const unsigned int TableId0,
        const unsigned int TableId1,
        const unsigned int TableId2,
        const unsigned int TableId3)
{
    uint32_t RegVal = EIP207_FLUE_REG_IFC_LUT_DEFAULT;

    RegVal |= (TableId0 & MASK_4_BITS);
    RegVal |= (TableId1 & MASK_4_BITS) << 8;
    RegVal |= (TableId2 & MASK_4_BITS) << 16;
    RegVal |= (TableId3 & MASK_4_BITS) << 24;

    EIP207_Write32(Device, EIP207_FLUE_REG_IFC_LUT(RegNr), RegVal);
}

#endif


#endif /* EIP207_LEVEL0_EXT_H_ */


/* end of file eip207_level0_ext.h */
