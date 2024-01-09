/* eip96_hw_interface.h
 *
 * EIP-96 Packet Engine HW interface
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

#ifndef EIP96_HW_INTERFACE_H_
#define EIP96_HW_INTERFACE_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip96.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // BIT definitions


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Internal Packet Engine time-out – a fatal error requiring a complete reset.
#define EIP96_TIMEOUT_FATAL_ERROR_MASK     BIT_14

// Read/Write register constants

/*****************************************************************************
 * Byte offsets of the EIP-96 Packet Engine registers
 *****************************************************************************/
#define EIP96_REG_OFFS                     4
#define EIP96_REG_MAP_SIZE                 8192

// Processing Packet Engine n (n - number of the DSE thread)
#define EIP96_REG_TOKEN_CTRL_STAT(n)       ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_CONF_BASE) + \
                                             (0x00 * EIP96_REG_OFFS)))
#define EIP96_REG_FUNCTION_EN(n)           ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_CONF_BASE) + \
                                             (0x01 * EIP96_REG_OFFS)))
#define EIP96_REG_CONTEXT_CTRL(n)          ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_CONF_BASE) + \
                                             (0x02 * EIP96_REG_OFFS)))
#define EIP96_REG_CONTEXT_STAT(n)          ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_CONF_BASE) + \
                                             (0x03 * EIP96_REG_OFFS)))

#define EIP96_REG_OUT_TRANS_CTRL_STAT(n)   ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_CONF_BASE) + \
                                             (0x06 * EIP96_REG_OFFS)))
#define EIP96_REG_OUT_BUF_CTRL(n)          ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_CONF_BASE) + \
                                             (0x07 * EIP96_REG_OFFS)))
#define EIP96_REG_CTX_NUM32_THR(n)         ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_CONF_BASE) + \
                                             (0x08 * EIP96_REG_OFFS)))
#define EIP96_REG_CTX_NUM64_THR_L(n)       ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_CONF_BASE) + \
                                             (0x09 * EIP96_REG_OFFS)))
#define EIP96_REG_CTX_NUM64_THR_H(n)       ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_CONF_BASE) + \
                                             (0x0a * EIP96_REG_OFFS)))
#define EIP96_REG_TOKEN_CTRL2(n)           ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_CONF_BASE) + \
                                             (0x0b * EIP96_REG_OFFS)))

// EIP-96 PRNG
#define EIP96_REG_PRNG_STAT(n)             ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_PRNG_BASE) + \
                                             (0x00 * EIP96_REG_OFFS)))
#define EIP96_REG_PRNG_CTRL(n)             ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_PRNG_BASE) + \
                                             (0x01 * EIP96_REG_OFFS)))
#define EIP96_REG_PRNG_SEED_L(n)           ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_PRNG_BASE) + \
                                             (0x02 * EIP96_REG_OFFS)))
#define EIP96_REG_PRNG_SEED_H(n)           ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_PRNG_BASE) + \
                                             (0x03 * EIP96_REG_OFFS)))
#define EIP96_REG_PRNG_KEY_0_L(n)          ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_PRNG_BASE) + \
                                             (0x04 * EIP96_REG_OFFS)))
#define EIP96_REG_PRNG_KEY_0_H(n)          ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_PRNG_BASE) + \
                                             (0x05 * EIP96_REG_OFFS)))
#define EIP96_REG_PRNG_KEY_1_L(n)          ((EIP96_REG_MAP_SIZE * n) + \
                                             ((EIP96_PRNG_BASE) + \
                                              (0x06 * EIP96_REG_OFFS)))
#define EIP96_REG_PRNG_KEY_1_H(n)          ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_PRNG_BASE) + \
                                             (0x07 * EIP96_REG_OFFS)))
#define EIP96_REG_PRNG_LFSR_L(n)           ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_PRNG_BASE) + \
                                             (0x0c * EIP96_REG_OFFS)))
#define EIP96_REG_PRNG_LFSR_H(n)           ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_PRNG_BASE) + \
                                             (0x0d * EIP96_REG_OFFS)))

#define EIP96_REG_ECN_TABLE(n,k)           ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_CONF_BASE) + 0x3e0 + 4*(k)))

// EIP-96 Options and Version
// New registers to must still be added to the HW,
// do not use these registers yet
#define EIP96_REG_OPTIONS(n)               ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_VER_BASE) + \
                                             (0x00 * EIP96_REG_OFFS)))
#define EIP96_REG_VERSION(n)               ((EIP96_REG_MAP_SIZE * n) + \
                                            ((EIP96_VER_BASE) + \
                                             (0x01 * EIP96_REG_OFFS)))

// Default EIP96_REG_TOKEN_CTRL_STAT register value
#define EIP96_REG_TOKEN_CTRL_STAT_DEFAULT   0x00004004

// Default EIP96_REG_PRNG_CTRL register value
#define EIP96_REG_PRNG_CTRL_DEFAULT         0x00000000

// Default EIP96_REG_OUT_BUF_CTRL register value
#define EIP96_REG_OUT_BUF_CTRL_DEFAULT      0x00000000


#endif /* EIP96_HW_INTERFACE_H_ */


/* end of file eip96_hw_interface.h */
