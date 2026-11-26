/* eip207_rc.c
 *
 * EIP-207 Record Cache (RC) interface High-Performance (HP) implementation
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

// EIP-207 Record Cache (RC) interface
#include "eip207_rc.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // uint32_t

// EIP-207 Global Control Driver Library Internal interfaces
#include "eip207_level0.h"              // EIP-207 Level 0 macros

// Driver Framework Device API
#include "device_types.h"               // Device_Handle_t


/*----------------------------------------------------------------------------
 * EIP207_RC_BaseAddr_Set
 */
void
EIP207_RC_BaseAddr_Set(
            const Device_Handle_t Device,
            const uint32_t CacheBase,
            const unsigned int CacheNr,
            const uint32_t Address,
            const uint32_t UpperAddress)
{
    // Not implemented
    IDENTIFIER_NOT_USED(Device);
    IDENTIFIER_NOT_USED(CacheBase);
    IDENTIFIER_NOT_USED(CacheNr);
    IDENTIFIER_NOT_USED(Address);
    IDENTIFIER_NOT_USED(UpperAddress);
}


/*----------------------------------------------------------------------------
 * EIP207_RC_Record_Update
 */
void
EIP207_RC_Record_Update(
        const Device_Handle_t Device,
        const uint32_t CacheBase,
        const unsigned int CacheNr,
        const uint32_t Rec_DMA_Addr,
        const uint8_t Command,
        const unsigned int ByteOffset,
        const uint32_t Value32)
{
    // Not implemented
    IDENTIFIER_NOT_USED(Command);
    IDENTIFIER_NOT_USED(Device);
    IDENTIFIER_NOT_USED(CacheBase);
    IDENTIFIER_NOT_USED(Device);
    IDENTIFIER_NOT_USED(CacheNr);
    IDENTIFIER_NOT_USED(Rec_DMA_Addr);
    IDENTIFIER_NOT_USED(ByteOffset);
    IDENTIFIER_NOT_USED(Value32);
}


/*----------------------------------------------------------------------------
 * EIP207_RC_Record_Dummy_Addr_Get
 */
unsigned int
EIP207_RC_Record_Dummy_Addr_Get(void)
{
    return EIP207_RC_RECORD_DUMMY_ADDRESS;
}


/* end of file eip207_rc.c */
