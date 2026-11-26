/* eip207_rc_hw_interface_ext.h
 *
 * EIP-207 HW interface extensions
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

#ifndef EIP207_RC_HW_INTERFACE_EXT_H_
#define EIP207_RC_HW_INTERFACE_EXT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Invalid (dummy) address
#define EIP207_RC_RECORD_DUMMY_ADDRESS    0


// Read/Write register constants

// EIP-207s Classification Support module (p - number of the cache set)
// Record Cache (FRC/TRC/ARC4RC) register interface
#define EIP207_RC_REG_REGINDEX(base,p)  ((EIP207_REG_MAP_SIZE * p) + \
                                         (base + \
                                          (0x00 * EIP207_REG_OFFS)))
#define EIP207_RC_REG_PARAMS(base,p)    ((EIP207_REG_MAP_SIZE * p) + \
                                         ((base + EIP207_RC_REG_PARAMS_BASE) + \
                                          (0x00 * EIP207_REG_OFFS)))
#define EIP207_RC_REG_FREECHAIN(base,p) ((EIP207_REG_MAP_SIZE * p) + \
                                         ((base + EIP207_RC_REG_PARAMS_BASE) + \
                                          (0x01 * EIP207_REG_OFFS)))
#define EIP207_RC_REG_PARAMS2(base,p)   ((EIP207_REG_MAP_SIZE * p) + \
                                         ((base + EIP207_RC_REG_PARAMS_BASE) + \
                                          (0x02 * EIP207_REG_OFFS)))
#define EIP207_RC_REG_ECCCTRL(base,p)   ((EIP207_REG_MAP_SIZE * p) + \
                                         ((base + EIP207_RC_REG_PARAMS_BASE) + \
                                          (0x04 * EIP207_REG_OFFS)))

// Debug counter registers (40 bit each, 2 consecutive words)
#define EIP207_RC_REG_PREFEXEC(base,p) ((EIP207_REG_MAP_SIZE * p) + base + 0x80)
#define EIP207_RC_REG_PREFBLCK(base,p) ((EIP207_REG_MAP_SIZE * p) + base + 0x88)
#define EIP207_RC_REG_PREFDMA(base,p) ((EIP207_REG_MAP_SIZE * p) + base + 0x90)
#define EIP207_RC_REG_SELOPS(base,p)  ((EIP207_REG_MAP_SIZE * p) + base + 0x98)
#define EIP207_RC_REG_SELDMA(base,p)  ((EIP207_REG_MAP_SIZE * p) + base + 0xA0)
#define EIP207_RC_REG_IDMAWR(base,p)  ((EIP207_REG_MAP_SIZE * p) + base + 0xA8)
#define EIP207_RC_REG_XDMAWR(base,p)  ((EIP207_REG_MAP_SIZE * p) + base + 0xB0)
#define EIP207_RC_REG_INVCMD(base,p)  ((EIP207_REG_MAP_SIZE * p) + base + 0xB8)
// Debug register, DMA read error status
#define EIP207_RC_REG_RDMAERRFLGS_0(base,p) \
    ((EIP207_REG_MAP_SIZE * p) + base + 0xD0)
// Debug counter registers, (24 bits each)
#define EIP207_RC_REG_RDMAERR(base,p) ((EIP207_REG_MAP_SIZE * p) + base + 0xE0)
#define EIP207_RC_REG_WDMAERR(base,p) ((EIP207_REG_MAP_SIZE * p) + base + 0xE4)
#define EIP207_RC_REG_INVECC(base,p)  ((EIP207_REG_MAP_SIZE * p) + base + 0xF0)
#define EIP207_RC_REG_DATECC_CORR(base,p) ((EIP207_REG_MAP_SIZE * p) + base + 0xF4)
#define EIP207_RC_REG_ADMECC_CORR(base,p) ((EIP207_REG_MAP_SIZE * p) + base + 0xE8)



// Register default value
#define EIP207_RC_REG_PARAMS_DEFAULT        0x00200401
#define EIP207_RC_REG_PARAMS2_DEFAULT       0x00000000
#define EIP207_RC_REG_REGINDEX_DEFAULT      0x00000000
#define EIP207_RC_REG_ECCCRTL_DEFAULT       0x00000000




#endif /* EIP207_RC_HW_INTERFACE_EXT_H_ */


/* end of file eip207_rc_hw_interface_ext.h */
