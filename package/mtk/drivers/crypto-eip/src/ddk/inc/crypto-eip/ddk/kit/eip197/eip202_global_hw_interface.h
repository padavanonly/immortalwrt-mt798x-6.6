/* eip202_global_hw_interface.h
 *
 * EIP-202 HIA Global Control HW Internal interface
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

#ifndef EIP202_GLOBAL_HW_INTERFACE_H_
#define EIP202_GLOBAL_HW_INTERFACE_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip202_global.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint16_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define EIP202_DFE_TRD_REG_STAT_IDLE    0xF

// Read/Write register constants

/*****************************************************************************
 * Byte offsets of the EIP-202 HIA registers
 *****************************************************************************/
// EIP-202 HIA EIP number (0xCA) and complement (0x35)
#define EIP202_SIGNATURE                ((uint16_t)0x35CA)

#define EIP202_REG_OFFS                 4

#define EIP202_FE_REG_MAP_SIZE          128

// HIA Look-aside FIFO base offset
#define EIP202_LASIDE_BASE              0x9FF00
#define EIP202_REG_LASIDE_MAP_SIZE      8

// HIA Inline FIFO base offset
#define EIP202_INLINE_BASE              0x9FF80
#define EIP202_REG_INLINE_MAP_SIZE      4

// HIA Ring Arbiter map size
#define EIP202_REG_RA_MAP_SIZE          8


// HIA Ring Arbiter
#define EIP202_RA_REG_PRIO_0     ((EIP202_RA_BASE)+(0x00 * EIP202_REG_OFFS))
#define EIP202_RA_REG_PRIO_1     ((EIP202_RA_BASE)+(0x01 * EIP202_REG_OFFS))
#define EIP202_RA_REG_PRIO_2     ((EIP202_RA_BASE)+(0x02 * EIP202_REG_OFFS))
#define EIP202_RA_REG_PRIO_3     ((EIP202_RA_BASE)+(0x03 * EIP202_REG_OFFS))

// HIA Ring Arbiter control for PE n (n - number of the PE)
#define EIP202_RA_PE_REG_CTRL(n)   ((EIP202_REG_RA_MAP_SIZE * n) + \
                                      ((EIP202_RA_BASE) + \
                                       (0x04 * EIP202_REG_OFFS)))

// HIA DFE all threads
#define EIP202_DFE_REG_CFG(n)        ((EIP202_FE_REG_MAP_SIZE * n) + \
                                      EIP202_DFE_BASE + \
                                       (0x00 * EIP202_REG_OFFS))

// HIA DFE thread n (n - number of the DFE thread)
#define EIP202_DFE_TRD_REG_CTRL(n)   ((EIP202_FE_REG_MAP_SIZE * n) + \
                                      ((EIP202_DFE_TRD_BASE) + \
                                       (0x00 * EIP202_REG_OFFS)))
#define EIP202_DFE_TRD_REG_STAT(n)   ((EIP202_FE_REG_MAP_SIZE * n) + \
                                      ((EIP202_DFE_TRD_BASE) + \
                                       (0x01 * EIP202_REG_OFFS)))

// HIA DSE all threads
#define EIP202_DSE_REG_CFG(n)        ((EIP202_FE_REG_MAP_SIZE * n) + \
                                      (EIP202_DSE_BASE) + \
                                       (0x00 * EIP202_REG_OFFS))

// HIA DSE thread n (n - number of the DSE thread)
#define EIP202_DSE_TRD_REG_CTRL(n)   ((EIP202_FE_REG_MAP_SIZE * n) + \
                                      ((EIP202_DSE_TRD_BASE) + \
                                       (0x00 * EIP202_REG_OFFS)))
#define EIP202_DSE_TRD_REG_STAT(n)   ((EIP202_FE_REG_MAP_SIZE * n) + \
                                      ((EIP202_DSE_TRD_BASE) + \
                                       (0x01 * EIP202_REG_OFFS)))


// HIA Global
#define EIP202_G_REG_OPTIONS2     ((EIP202_G_BASE)+(0x00 * EIP202_REG_OFFS))
#define EIP202_G_REG_MST_CTRL     ((EIP202_G_BASE)+(0x01 * EIP202_REG_OFFS))
#define EIP202_G_REG_OPTIONS      ((EIP202_G_BASE)+(0x02 * EIP202_REG_OFFS))
#define EIP202_G_REG_VERSION      ((EIP202_G_BASE)+(0x03 * EIP202_REG_OFFS))

// HIA Look-aside (LA) FIFO, k - LA FIFO number, must be from 1 to 5
#define EIP202_REG_LASIDE_BASE_ADDR_LO      ((EIP202_LASIDE_BASE) + \
                                                (0x00 * EIP202_REG_OFFS))
#define EIP202_REG_LASIDE_BASE_ADDR_HI      ((EIP202_LASIDE_BASE) + \
                                                (0x01 * EIP202_REG_OFFS))
#define EIP202_REG_LASIDE_SLAVE_CTRL(k)  ((EIP202_REG_LASIDE_MAP_SIZE * k) + \
                                           ((EIP202_LASIDE_BASE) + \
                                            (0x00 * EIP202_REG_OFFS)))
#define EIP202_REG_LASIDE_MASTER_CTRL(k) ((EIP202_REG_LASIDE_MAP_SIZE * k) + \
                                           ((EIP202_LASIDE_BASE) + \
                                            (0x01 * EIP202_REG_OFFS)))

// HIA Inline (IN) FIFO base offset, l - IN FIFO number
#define EIP202_REG_INLINE_CTRL(k)         ((EIP202_REG_INLINE_MAP_SIZE * k) + \
                                           ((EIP202_INLINE_BASE) + \
                                            (0x00 * EIP202_REG_OFFS)))

// Default EIP202_DFE_REG_CFG register value
#define EIP202_DFE_REG_CFG_DEFAULT          0x00000000

// Default EIP202_DFE_TRD_REG_CTRL register value
#define EIP202_DFE_TRD_REG_CTRL_DEFAULT     0x00000000

// Default EIP202_DSE_REG_CFG register value
#define EIP202_DSE_REG_CFG_DEFAULT          0x80008000

// Default EIP202_DSE_TRD_REG_CTRL register value
#define EIP202_DSE_TRD_REG_CTRL_DEFAULT     0x00000000

// Default EIP202_RA_REG_PRIO_x register values
#define EIP202_RA_REG_PRIO_0_DEFAULT        0x00000000
#define EIP202_RA_REG_PRIO_1_DEFAULT        0x00000000
#define EIP202_RA_REG_PRIO_2_DEFAULT        0x00000000
#define EIP202_RA_REG_PRIO_3_DEFAULT        0x00000000
#define EIP202_RA_PE_REG_CTRL_DEFAULT       0x00000000

// Default HIA Look-aside (LA) FIFO registers values
#define EIP202_REG_LASIDE_BASE_ADDR_LO_DEFAULT     0x00000000
#define EIP202_REG_LASIDE_BASE_ADDR_HI_DEFAULT     0x00000000
#define EIP202_REG_LASIDE_MASTER_CTRL_DEFAULT      0x00000000
#define EIP202_REG_LASIDE_SLAVE_CTRL_DEFAULT       0x00000000

// Default HIA Inline (IN) FIFO registers values
#define EIP202_REG_INLINE_CTRL_DEFAULT             0x00000000

// Default value of the BufferCtrl field in the EIP202_DSE_REG_CFG register
#define EIP202_DSE_BUFFER_CTRL            ((EIP202_DSE_REG_CFG_DEFAULT >> 14) \
                                           & MASK_2_BITS)


#endif /* EIP202_GLOBAL_HW_INTERFACE_H_ */


/* end of file eip202_global_hw_interface.h */
