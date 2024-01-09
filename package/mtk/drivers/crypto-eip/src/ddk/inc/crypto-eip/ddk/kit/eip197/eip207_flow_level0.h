/* eip207_flow_level0.h
 *
 * EIP-207 Flow Level0 Internal interface
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

#ifndef EIP207_FLOW_LEVEL0_H_
#define EIP207_FLOW_LEVEL0_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip207_flow.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // BIT definitions, bool, uint32_t

// EIP-207 Flow HW interface
#include "eip207_flow_hw_interface.h"

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
 * EIP207 Global Register Write/Read Functions
 *
 */

/*----------------------------------------------------------------------------
 * EIP207_Flow_Read32
 *
 * This routine writes to a Register location in the EIP-207.
 */
static inline uint32_t
EIP207_Flow_Read32(
        Device_Handle_t Device,
        const unsigned int Offset)
{
    return Device_Read32(Device, Offset);
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_Write32
 *
 * This routine writes to a Register location in the EIP-207.
 */
static inline void
EIP207_Flow_Write32(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t Value)
{
    Device_Write32(Device, Offset, Value);

#ifdef EIP207_FLOW_CLUSTERED_WRITES_DISABLE
    // Prevent clustered write operations, break them with a read operation
    // Note: Reading the EIP207_CS_REG_VERSION register has no side effects!
    EIP207_Flow_Read32(Device, EIP207_CS_REG_VERSION);
#endif
}


static inline bool
EIP207_FLUE_SIGNATURE_MATCH(
        const uint32_t Rev)
{
    return (((uint16_t)Rev) == EIP207_FLUE_SIGNATURE);
}


static inline void
EIP207_FLUE_HASHBASE_LO_WR(
        const Device_Handle_t Device,
        const unsigned int HashTableNr,
        const uint32_t Addr32Lo)
{
    uint32_t RegVal = EIP207_FLUE_REG_HASHBASE_LO_DEFAULT;

    RegVal |= (uint32_t)((((uint32_t)Addr32Lo) & (~MASK_2_BITS)));

    EIP207_Flow_Write32(Device,
                        EIP207_FLUE_FHT_REG_HASHBASE_LO(HashTableNr),
                        RegVal);
}


static inline void
EIP207_FLUE_HASHBASE_HI_WR(
        const Device_Handle_t Device,
        const unsigned int HashTableNr,
        const uint32_t Addr32Hi)
{
    EIP207_Flow_Write32(Device,
                        EIP207_FLUE_FHT_REG_HASHBASE_HI(HashTableNr),
                        Addr32Hi);
}


static inline void
EIP207_FHASH_IV_RD(
        const Device_Handle_t Device,
        const unsigned int HashTableNr,
        uint32_t * const IV_0,
        uint32_t * const IV_1,
        uint32_t * const IV_2,
        uint32_t * const IV_3)
{
    *IV_0 = EIP207_Flow_Read32(Device, EIP207_FHASH_REG_IV_0(HashTableNr));
    *IV_1 = EIP207_Flow_Read32(Device, EIP207_FHASH_REG_IV_1(HashTableNr));
    *IV_2 = EIP207_Flow_Read32(Device, EIP207_FHASH_REG_IV_2(HashTableNr));
    *IV_3 = EIP207_Flow_Read32(Device, EIP207_FHASH_REG_IV_3(HashTableNr));
}


// EIP-207 Flow Level0 interface extensions
#include "eip207_flow_level0_ext.h"


#endif /* EIP207_FLOW_LEVEL0_H_ */


/* end of file eip207_flow_level0.h */
