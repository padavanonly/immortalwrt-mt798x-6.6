/* eip207_flow_hw_interface.h
 *
 * EIP-207 Flow HW interface
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

#ifndef EIP207_FLOW_HW_INTERFACE_H_
#define EIP207_FLOW_HW_INTERFACE_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip207_flow.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint16_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Read/Write register constants

/*****************************************************************************
 * Byte offsets of the EIP-207 HIA registers
 *****************************************************************************/

// EIP-207c FLUE EIP number (0xCF) and complement (0x30)
#define EIP207_FLUE_SIGNATURE           ((uint16_t)0x30CF)

#define EIP207_REG_OFFS                 4


// EIP-207 Flow HW interface extensions
#include "eip207_flow_hw_interface_ext.h"


#endif /* EIP207_FLOW_HW_INTERFACE_H_ */


/* end of file eip207_flow_hw_interface.h */
