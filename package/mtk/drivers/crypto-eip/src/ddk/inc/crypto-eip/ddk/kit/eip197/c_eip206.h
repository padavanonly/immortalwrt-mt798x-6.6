/* c_eip206.h
 *
 * Default configuration EIP-206 Driver Library
 *
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

#ifndef C_EIP206_H_
#define C_EIP206_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level configuration
#include "cs_eip206.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Processing Packet Engine n (n - number of the PE)
// Input Side
//#define EIP206_IN_DBUF_BASE           0xA0000
//#define EIP206_IN_TBUF_BASE           0xA0100

// Output Side
//#define EIP206_OUT_DBUF_BASE          0xA1C00
//#define EIP206_OUT_TBUF_BASE          0xA1D00

// PE Options and Version
//#define EIP206_ARC4_BASE              0xA1FEC
//#define EIP206_VER_BASE               0xA1FF8


#endif /* C_EIP206_H_ */


/* end of file c_eip206.h */
