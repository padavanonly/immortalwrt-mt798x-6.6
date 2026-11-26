/* eip207_flow_level0_ext.h
 *
 * EIP-207 Flow Level0 Internal interface extensions
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

#ifndef EIP207_FLOW_LEVEL0_EXT_H_
#define EIP207_FLOW_LEVEL0_EXT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP207 Flow Functions
 *
 */

static inline void
EIP207_FLUE_CACHEBASE_LO_WR(
        const Device_Handle_t Device,
        const unsigned int HashTableNr,
        const uint32_t Addr32Lo)
{
    uint32_t RegVal = EIP207_FLUE_REG_CACHEBASE_LO_DEFAULT;

    RegVal |= (uint32_t)((((uint32_t)Addr32Lo) & (~MASK_2_BITS)));

    EIP207_Flow_Write32(Device,
                        EIP207_FLUE_FHT_REG_CACHEBASE_LO(HashTableNr),
                        RegVal);
}


static inline void
EIP207_FLUE_CACHEBASE_HI_WR(
        const Device_Handle_t Device,
        const unsigned int HashTableNr,
        const uint32_t Addr32Hi)
{
    EIP207_Flow_Write32(Device,
                        EIP207_FLUE_FHT_REG_CACHEBASE_HI(HashTableNr),
                        Addr32Hi);
}


static inline void
EIP207_FLUE_SIZE_UPDATE(
        const Device_Handle_t Device,
        const unsigned int HashTableNr,
        const uint8_t TableSize)
{
    uint32_t RegVal =
                EIP207_Flow_Read32(Device,
                                   EIP207_FLUE_FHT_REG_SIZE(HashTableNr));

    RegVal &= (~(MASK_4_BITS << 4));
    RegVal |=
          (uint32_t)((((uint32_t)TableSize) & MASK_4_BITS) << 4);

    EIP207_Flow_Write32(Device,
                        EIP207_FLUE_FHT_REG_SIZE(HashTableNr),
                        RegVal);
}


#endif /* EIP207_FLOW_LEVEL0_EXT_H_ */


/* end of file eip207_flow_level0_ext.h */
