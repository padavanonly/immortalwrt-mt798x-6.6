/* eip202_cd_format.c
 *
 * EIP-202 Ring Control Driver Library
 * Command Descriptor Internal interface
 *
 * This module contains the EIP-202 Command Descriptor specific functionality
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "eip202_cd_format.h"

// Descriptor I/O Driver Library API implementation
#include "eip202_cdr.h"                 // EIP202_ARM_CommandDescriptor_t


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip202_ring.h"

// EIP-202 Ring Control Driver Library Internal interfaces
#include "eip202_ring_internal.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"                // bool, uint32_t, uint8_t

// Driver Framework DMA Resource API
#include "dmares_types.h"              // DMAResource_Handle_t
#include "dmares_rw.h"                 // DMAResource_Write/Read

// Standard IOToken API
#include "iotoken.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP202_CD_Make_ControlWord
 */
uint32_t
EIP202_CD_Make_ControlWord(
        const uint8_t TokenWordCount,
        const uint32_t SegmentByteCount,
        const bool fFirstSegment,
        const bool fLastSegment,
        const bool fForceEngine,
        const uint8_t EngineId)
{
    uint32_t Value = 0;

    if(fFirstSegment)
        Value |= BIT_23;

    if(fLastSegment)
        Value |= BIT_22;

    Value |= ((((uint32_t)TokenWordCount) & MASK_8_BITS) << 24);
    Value |= ((((uint32_t)SegmentByteCount) & MASK_16_BITS));
    if (fForceEngine)
        Value |= BIT_21 | (((uint32_t)EngineId & MASK_5_BITS) << 16);

    return Value;
}


/*----------------------------------------------------------------------------
 * EIP202_CD_Write
 */
void
EIP202_CD_Write(
        DMAResource_Handle_t Handle,
        const unsigned int WordOffset,
        const EIP202_ARM_CommandDescriptor_t * const Descr_p,
        const bool fATP)

{
    unsigned int InTokenWordOffset;

#ifdef EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS
    IOToken_Mark_Set(Descr_p->Token_p);
#endif

#ifdef EIP202_64BIT_DEVICE
    // Write Control Word
    DMAResource_Write32(Handle, WordOffset, Descr_p->ControlWord);

    // Lengths greater than 20 bits not supported yet.
#ifndef EIP202_CDR_OPT1
    DMAResource_Write32(Handle, WordOffset + 1, 0);
#endif

    // Write Source Packet Data address
    DMAResource_Write32(Handle, WordOffset + 2, Descr_p->SrcPacketAddr.Addr);
    DMAResource_Write32(Handle, WordOffset + 3, Descr_p->SrcPacketAddr.UpperAddr);

    if (fATP)
    {
#ifndef EIP202_CDR_OPT2
        // Write Token Data address
        DMAResource_Write32(Handle,
                            WordOffset + 4,
                            Descr_p->TokenDataAddr.Addr);
        DMAResource_Write32(Handle,
                            WordOffset + 5,
                            Descr_p->TokenDataAddr.UpperAddr);
#endif
        InTokenWordOffset = WordOffset + 6;
    }
    else
        InTokenWordOffset = WordOffset + 4;
#else // EIP202_64BIT_DEVICE
    // Write Control Word
    DMAResource_Write32(Handle, WordOffset, Descr_p->ControlWord);

    // Write Source Packet Data address
    DMAResource_Write32(Handle, WordOffset + 1, Descr_p->SrcPacketAddr.Addr);

    if (fATP)
    {
#ifndef EIP202_CDR_OPT2
        // Write Token Data address
        DMAResource_Write32(Handle, WordOffset + 2, Descr_p->TokenDataAddr.Addr);
#endif
        InTokenWordOffset = WordOffset + 3;
    }
    else
        InTokenWordOffset = WordOffset + 2;
#endif // !EIP202_64BIT_DEVICE

    // Write Input Token (only for the first segment and if token is available)
    if (Descr_p->ControlWord & BIT_23 && Descr_p->Token_p)
    {
        unsigned int i, offset = InTokenWordOffset;

        // Write Application ID
#ifdef EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS
        IOToken_Mark_Set(Descr_p->Token_p);
#endif

        for (i = 0; i < IOToken_InWordCount_Get(); i++)
            DMAResource_Write32(Handle, offset + i, Descr_p->Token_p[i]);
    }

    return;
}


/* end of file eip202_cd_format.c */
