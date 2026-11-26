/* eip206_hw_interface.h
 *
 * EIP-206 Processing Engine HW interface
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

#ifndef EIP206_HW_INTERFACE_H_
#define EIP206_HW_INTERFACE_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip206.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint16_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Read/Write register constants

/*****************************************************************************
 * Byte offsets of the EIP-206 Processing Engine registers
 *****************************************************************************/
// Processing Engine EIP number (0xCE) and complement (0x31)
#define EIP206_SIGNATURE              ((uint16_t)0x31CE)

#define EIP206_REG_OFFS               4
#define EIP206_REG_MAP_SIZE           8192

// Processing Packet Engine n (n - number of the PE)
// Input Side
#define EIP206_IN_REG_DBUF_TRESH(n)   ((EIP206_REG_MAP_SIZE * n) + \
                                       ((EIP206_IN_DBUF_BASE) + \
                                        (0x00 * EIP206_REG_OFFS)))

#define EIP206_IN_REG_TBUF_TRESH(n)   ((EIP206_REG_MAP_SIZE * n) + \
                                       ((EIP206_IN_TBUF_BASE) + \
                                        (0x00 * EIP206_REG_OFFS)))

// Output Side
#define EIP206_OUT_REG_DBUF_TRESH(n)  ((EIP206_REG_MAP_SIZE * n) + \
                                       ((EIP206_OUT_DBUF_BASE) + \
                                        (0x00 * EIP206_REG_OFFS)))

#define EIP206_OUT_REG_TBUF_TRESH(n)  ((EIP206_REG_MAP_SIZE * n) + \
                                       ((EIP206_OUT_TBUF_BASE) + \
                                        (0x00 * EIP206_REG_OFFS)))

// PE Options and Version
#define EIP206_REG_DEBUG(n)           ((EIP206_REG_MAP_SIZE * n) + \
                                       ((EIP206_VER_BASE) - \
                                        (0x01 * EIP206_REG_OFFS)))
#define EIP206_REG_OPTIONS(n)         ((EIP206_REG_MAP_SIZE * n) + \
                                       ((EIP206_VER_BASE) + \
                                        (0x00 * EIP206_REG_OFFS)))
#define EIP206_REG_VERSION(n)         ((EIP206_REG_MAP_SIZE * n) + \
                                       ((EIP206_VER_BASE) + \
                                        (0x01 * EIP206_REG_OFFS)))

// Default EIP206_IN_REG_DBUF_TRESH register value
#define EIP206_IN_REG_DBUF_TRESH_DEFAULT        0x00000000

// Default EIP206_IN_REG_TBUF_TRESH register value
#define EIP206_IN_REG_TBUF_TRESH_DEFAULT        0x00000000

// Default EIP206_OUT_REG_DBUF_TRESH register value
#define EIP206_OUT_REG_DBUF_TRESH_DEFAULT       0x00000000

// Default EIP206_OUT_REG_TBUF_TRESH register value
#define EIP206_OUT_REG_TBUF_TRESH_DEFAULT       0x00000000


// ARC4 Size Small/Large
#define EIP206_ARC4_REG_SIZE(n)       ((EIP206_REG_MAP_SIZE * n) + \
                                       ((EIP206_ARC4_BASE) + \
                                        (0x00 * EIP206_REG_OFFS)))


// Default EIP206_ARC4_REG_SIZE_DEFAULT register value
#define EIP206_ARC4_REG_SIZE_DEFAULT            0x00000000

#endif /* EIP206_HW_INTERFACE_H_ */


/* end of file eip206_hw_interface.h */
