/* eip74_level0.h
 *
 * This file contains all the macros and functions that allow
 * access to the EIP74 registers and to build the values
 * read or written to the registers.
 *
 */

/*****************************************************************************
* Copyright (c) 2017-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef EIP74_LEVEL0_H_
#define EIP74_LEVEL0_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip74.h"

// Register addresses
#include "eip74_hw_interface.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // BIT definitions, bool, uint32_t

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t
#include "device_rw.h"          // Read32, Write32


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * EIP74_Read32
 *
 * This routine writes to a Register location in the EIP74.
 */
static inline uint32_t
EIP74_Read32(
        Device_Handle_t Device,
        const unsigned int Offset)
{
    return Device_Read32(Device, Offset);
}


/*----------------------------------------------------------------------------
 * EIP74_Write32
 *
 * This routine writes to a Register location in the EIP74.
 */
static inline void
EIP74_Write32(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t Value)
{
    Device_Write32(Device, Offset, Value);
}


static inline void
EIP74_INPUT_WR(
        Device_Handle_t Device,
        const uint32_t * const Input_p)
{
    unsigned i;
    for (i=0; i<4; i++)
    {
        EIP74_Write32(Device,
                      EIP74_REG_INPUT_0 + i*sizeof(uint32_t),
                      Input_p[i]);
    }
}

static inline void
EIP74_OUTPUT_RD(
        Device_Handle_t Device,
        uint32_t * const Output_p)
{
    unsigned i;
    for (i=0; i<4; i++)
    {
        Output_p[i] =
            EIP74_Read32(Device, EIP74_REG_OUTPUT_0 + i*sizeof(uint32_t));
    }
}


static inline void
EIP74_CONTROL_WR(
        Device_Handle_t Device,
        const bool fReadyMask,
        const bool fStuckOutMask,
        const bool fTestMode,
        const bool fHostMode,
        const bool fEnableDRBG,
        const bool fForceStuckOut,
        const bool fRequestData,
        const uint16_t DataBlocks)
{
    uint32_t RegVal = EIP74_REG_CONTROL_DEFAULT;

    if (fRequestData)
    {   // RequestData bit leaves all other control settings unchanged.
        RegVal |= BIT_16;
        RegVal |= (DataBlocks & MASK_12_BITS) << 20;
    }
    else
    {
        if (fReadyMask)
            RegVal |= BIT_0;

        if (fStuckOutMask)
            RegVal |= BIT_2;

        if (fTestMode)
            RegVal |= BIT_8;

        if (fHostMode)
            RegVal |= BIT_9;

        if (fEnableDRBG)
            RegVal |= BIT_10;

        if (fForceStuckOut)
            RegVal |= BIT_11;
    }


    EIP74_Write32(Device, EIP74_REG_CONTROL, RegVal);
}


static inline void
EIP74_CONTROL_RD(
        Device_Handle_t Device,
        bool * const fReseed)
{
    uint32_t RegVal = EIP74_Read32(Device, EIP74_REG_CONTROL);

    *fReseed = (RegVal & BIT_15) != 0;
}


static inline void
EIP74_STATUS_RD(
        Device_Handle_t Device,
        bool * const fReady,
        bool * const fPSAIWriteOK,
        bool * const fStuckOut,
        bool * const fEarlyReseed,
        bool * const fTestReady,
        bool * const fGenPktError,
        bool * const fInstantiated,
        bool * const fTestStuckOut,
        uint8_t * const BlocksAvailable,
        bool * const fNeedClock)
{
    uint32_t RegVal =  EIP74_Read32(Device, EIP74_REG_STATUS);

    *fReady          = (RegVal & BIT_0) != 0;
    *fPSAIWriteOK    = (RegVal & BIT_1) != 0;
    *fStuckOut       = (RegVal & BIT_2) != 0;
    *fEarlyReseed    = (RegVal & BIT_7) != 0;
    *fTestReady      = (RegVal & BIT_8) != 0;
    *fGenPktError    = (RegVal & BIT_9) != 0;
    *fInstantiated   = (RegVal & BIT_10) != 0;
    *fTestStuckOut   = (RegVal & BIT_15) != 0;

    *BlocksAvailable = (RegVal >> 16) & MASK_8_BITS;

    *fNeedClock      = (RegVal & BIT_31) != 0;
}


/* Write TRNG_INTACK register (write-only) */
static inline void
EIP74_INTACK_WR(
        Device_Handle_t Device,
        const bool fReadyAck,
        const bool fStuckOutAck,
        const bool fTestStuckOut)
{
    uint32_t RegVal = EIP74_REG_INTACK_DEFAULT;

    if (fReadyAck)
        RegVal |= BIT_0;

    if (fStuckOutAck)
        RegVal |= BIT_2;

    if (fTestStuckOut)
        RegVal |= BIT_15;

    EIP74_Write32(Device, EIP74_REG_INTACK, RegVal);
}


static inline void
EIP74_GENERATE_CNT_RD(
        Device_Handle_t Device,
        uint32_t * const Value_p)
{
    *Value_p = EIP74_Read32(Device, EIP74_REG_GENERATE_CNT);
}


static inline void
EIP74_RESEED_THR_WR(
        Device_Handle_t Device,
        const uint32_t Value)
{
    EIP74_Write32(Device, EIP74_REG_RESEED_THR, Value);
}


static inline void
EIP74_RESEED_THR_RD(
        Device_Handle_t Device,
        uint32_t * const Value)
{
    *Value = EIP74_Read32(Device, EIP74_REG_RESEED_THR);
}


static inline void
EIP74_RESEED_THR_EARLY_WR(
        Device_Handle_t Device,
        const uint32_t Value)
{
    EIP74_Write32(Device, EIP74_REG_RESEED_THR_EARLY, Value);
}


static inline void
EIP74_RESEED_THR_EARLY_RD(
        Device_Handle_t Device,
        uint32_t * const Value)
{
    *Value = EIP74_Read32(Device, EIP74_REG_RESEED_THR_EARLY);
}


static inline void
EIP74_GEN_BLK_SIZE_WR(
        Device_Handle_t Device,
        const uint32_t Value)
{
    uint32_t RegVal = EIP74_REG_GEN_BLK_CNT_DEFAULT;

    RegVal |= Value & MASK_12_BITS;
    EIP74_Write32(Device, EIP74_REG_GEN_BLK_SIZE, RegVal);
}


static inline bool
EIP74_REV_SIGNATURE_MATCH(
        const uint32_t Rev)
{
    return (((uint16_t)Rev) == EIP74_SIGNATURE);
}


static inline void
EIP74_VERSION_RD(
        Device_Handle_t Device,
        uint8_t * const EipNumber,
        uint8_t * const ComplmtEipNumber,
        uint8_t * const HWPatchLevel,
        uint8_t * const MinHWRevision,
        uint8_t * const MajHWRevision)
{
    uint32_t RevRegVal;

    RevRegVal = EIP74_Read32(Device, EIP74_REG_VERSION);

    *MajHWRevision     = (uint8_t)((RevRegVal >> 24) & 0x0f);
    *MinHWRevision     = (uint8_t)((RevRegVal >> 20) & 0x0f);
    *HWPatchLevel      = (uint8_t)((RevRegVal >> 16) & 0x0f);
    *ComplmtEipNumber  = (uint8_t)((RevRegVal >> 8)  & 0xff);
    *EipNumber         = (uint8_t)((RevRegVal)       & 0xff);
}


static inline void
EIP74_OPTIONS_RD(
        Device_Handle_t Device,
        uint8_t * const ClientCount,
        uint8_t * const AESCoreCount,
        uint8_t * const AESCoreSpeed,
        uint8_t * const FIFODepth)
{
    uint32_t RegVal;

    RegVal = EIP74_Read32(Device, EIP74_REG_OPTIONS);

    *ClientCount = RegVal & MASK_6_BITS;
    *AESCoreCount = (RegVal >> 8) & MASK_2_BITS;
    *AESCoreSpeed = (RegVal >> 10) & MASK_2_BITS;
    *FIFODepth = (RegVal >> 16) & MASK_8_BITS;
}


/* Write TRNG_TEST register */
static inline void
EIP74_TEST_WR(
        Device_Handle_t Device,
        const bool fTestAES256,
        const bool fTestSP80090,
        const bool fTestIRQ)
{
    uint32_t Value = EIP74_REG_TEST_DEFAULT;

    if (fTestAES256)
        Value |= BIT_6;

    if (fTestSP80090)
        Value |= BIT_7;

    if (fTestIRQ)
        Value |= BIT_31;

    EIP74_Write32(Device, EIP74_REG_TEST, Value);
}


/* Read TRNG_TEST register */
static inline void
EIP74_TEST_RD(
        Device_Handle_t Device,
        bool * const fTestAES256,
        bool * const fTestSP80090,
        bool * const fTestIRQ)
{
    uint32_t Value = EIP74_Read32(Device, EIP74_REG_TEST);

    *fTestAES256 = (Value & BIT_6) != 0;
    *fTestSP80090 = (Value & BIT_7) != 0;
    *fTestIRQ = (Value & BIT_31) != 0;
}


/* Write Key registers */
static inline void
EIP74_KEY_WR(
        Device_Handle_t Device,
        const uint32_t * Data_p,
        const unsigned int WordCount)
{
    unsigned int i;

    for(i = 0; i < WordCount; i++)
        EIP74_Write32(Device,
                      EIP74_REG_KEY_0 + i * sizeof(uint32_t),
                      Data_p[i]);
}


/* Write PS_AI registers */
static inline void
EIP74_PS_AI_WR(
        Device_Handle_t Device,
        const uint32_t * Data_p,
        const unsigned int WordCount)
{
    unsigned int i;

    for(i = 0; i < WordCount; i++)
        EIP74_Write32(Device,
                      EIP74_REG_PS_AI_0 + i * sizeof(uint32_t),
                      Data_p[i]);
}


#endif /* EIP74_LEVEL0_H_ */

/* end of file eip74_level0.h */
