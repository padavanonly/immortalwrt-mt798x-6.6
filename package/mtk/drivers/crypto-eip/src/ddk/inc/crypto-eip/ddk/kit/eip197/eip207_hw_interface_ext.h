/* eip207_hw_interface_ext.h
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

#ifndef EIP207_HW_INTERFACE_EXT_H_
#define EIP207_HW_INTERFACE_EXT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// VM-specific register sets in 32-byte blocks
#define EIP207_FLUE_VM_REG_MAP_SIZE       32


// Read/Write register constants

// EIP-207s Classification Support, Flow Look-Up Engine (FLUE), VM-specific
// (f - number of VM)
#define EIP207_FLUE_REG_CONFIG(f)       ((EIP207_FLUE_VM_REG_MAP_SIZE * f) + \
                                         ((EIP207_FLUE_CONFIG_REG_BASE) + \
                                          (0x00 * EIP207_REG_OFFS)))

// EIP-207s Classification Support, Flow Look-Up Engine (FLUE)
#define EIP207_FLUE_REG_OFFSETS         (EIP207_FLUE_REG_BASE + \
                                          (0x00 * EIP207_REG_OFFS))
#define EIP207_FLUE_REG_ARC4_OFFSET     (EIP207_FLUE_REG_BASE + \
                                          (0x01 * EIP207_REG_OFFS))

#define EIP207_FLUE_REG_ENABLED_LO      (EIP207_FLUE_ENABLED_REG_BASE + \
                                          (0x02 * EIP207_REG_OFFS))
#define EIP207_FLUE_REG_ENABLED_HI      (EIP207_FLUE_ENABLED_REG_BASE + \
                                          (0x03 * EIP207_REG_OFFS))
#define EIP207_FLUE_REG_ERROR_LO        (EIP207_FLUE_ENABLED_REG_BASE + \
                                          (0x04 * EIP207_REG_OFFS))
#define EIP207_FLUE_REG_ERROR_HI        (EIP207_FLUE_ENABLED_REG_BASE + \
                                          (0x05 * EIP207_REG_OFFS))

#define EIP207_FLUE_REG_IFC_LUT(f)       (EIP207_FLUE_IFC_LUT_REG_BASE + \
                                          ((f) * EIP207_REG_OFFS))

// EIP-207c Classification Engine n (n - number of the CE)
// Output Side

#define EIP207_OCE_REG_SCRATCH_RAM(n)  ((EIP207_REG_MAP_SIZE * n) + \
                                        ((EIP207_OCE_REG_SCRATCH_BASE) + \
                                         (0x00 * EIP207_REG_OFFS)))

#define EIP207_OCE_REG_ADAPT_CTRL(n)   ((EIP207_REG_MAP_SIZE * n) + \
                                        ((EIP207_OCE_REG_ADAPT_CTRL_BASE) + \
                                         (0x00 * EIP207_REG_OFFS)))

#define EIP207_OCE_REG_PUE_CTRL(n)     ((EIP207_REG_MAP_SIZE * n) + \
                                        ((EIP207_OCE_REG_PUE_CTRL_BASE) + \
                                         (0x00 * EIP207_REG_OFFS)))
#define EIP207_OCE_REG_PUE_DEBUG(n)    ((EIP207_REG_MAP_SIZE * n) + \
                                        ((EIP207_OCE_REG_PUE_CTRL_BASE) + \
                                         (0x01 * EIP207_REG_OFFS)))

#define EIP207_OCE_REG_PUTF_CTRL(n)    ((EIP207_REG_MAP_SIZE * n) + \
                                        ((EIP207_OCE_REG_PUTF_CTRL_BASE) + \
                                         (0x00 * EIP207_REG_OFFS)))
#define EIP207_OCE_REG_SCRATCH_CTRL(n) ((EIP207_REG_MAP_SIZE * n) + \
                                        ((EIP207_OCE_REG_PUTF_CTRL_BASE) + \
                                         (0x01 * EIP207_REG_OFFS)))
#define EIP207_OCE_REG_TIMER_LO(n)     ((EIP207_REG_MAP_SIZE * n) + \
                                        ((EIP207_OCE_REG_PUTF_CTRL_BASE) + \
                                         (0x02 * EIP207_REG_OFFS)))
#define EIP207_OCE_REG_TIMER_HI(n)     ((EIP207_REG_MAP_SIZE * n) + \
                                        ((EIP207_OCE_REG_PUTF_CTRL_BASE) + \
                                         (0x03 * EIP207_REG_OFFS)))
#define EIP207_OCE_REG_UENG_STAT(n)    ((EIP207_REG_MAP_SIZE * n) + \
                                        ((EIP207_OCE_REG_PUTF_CTRL_BASE) + \
                                         (0x04 * EIP207_REG_OFFS)))

#define EIP207_OCE_REG_FPP_CTRL(n)     ((EIP207_REG_MAP_SIZE * n) + \
                                        ((EIP207_OCE_REG_FPP_CTRL_BASE) + \
                                         (0x00 * EIP207_REG_OFFS)))
#define EIP207_OCE_REG_FPP_DEBUG(n)    ((EIP207_REG_MAP_SIZE * n) + \
                                        ((EIP207_OCE_REG_FPP_CTRL_BASE) + \
                                         (0x01 * EIP207_REG_OFFS)))

#define EIP207_OCE_REG_PPTF_CTRL(n)    ((EIP207_REG_MAP_SIZE * n) + \
                                        ((EIP207_OCE_REG_PPTF_CTRL_BASE) + \
                                         (0x00 * EIP207_REG_OFFS)))

#define EIP207_OCE_REG_RAM_CTRL(n)      ((EIP207_REG_MAP_SIZE * n) + \
                                         ((EIP207_OCE_REG_RAM_CTRL_BASE) + \
                                          (0x00 * EIP207_REG_OFFS)))
#define EIP207_OCE_REG_RAM_CTRL_RSV1(n) ((EIP207_REG_MAP_SIZE * n) + \
                                         ((EIP207_OCE_REG_RAM_CTRL_BASE) + \
                                          (0x01 * EIP207_REG_OFFS)))
#define EIP207_OCE_REG_OPTIONS(n)       ((EIP207_REG_MAP_SIZE * n) + \
                                         ((EIP207_OCE_REG_RAM_CTRL_BASE) + \
                                          (0x02 * EIP207_REG_OFFS)))
#define EIP207_OCE_REG_VERSION(n)       ((EIP207_REG_MAP_SIZE * n) + \
                                         ((EIP207_OCE_REG_RAM_CTRL_BASE) + \
                                          (0x03 * EIP207_REG_OFFS)))

// Output Side: reserved

// Register default value
#define EIP207_FLUE_REG_CONFIG_DEFAULT      0x00000000

#define EIP207_FLUE_REG_IFC_LUT_DEFAULT     0x00000000

#define EIP207_OCE_REG_RAM_CTRL_DEFAULT     0x00000000
#define EIP207_OCE_REG_PUE_CTRL_DEFAULT     0x00000001
#define EIP207_OCE_REG_FPP_CTRL_DEFAULT     0x00000001
#define EIP207_OCE_REG_SCRATCH_CTRL_DEFAULT 0x001F0200


#endif /* EIP207_HW_INTERFACE_EXT_H_ */


/* end of file eip207_hw_interface_ext.h */
