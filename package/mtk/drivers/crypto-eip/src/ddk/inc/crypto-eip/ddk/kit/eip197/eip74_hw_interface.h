/* eip74_hw_interface.h
 *
 * EIP74 register internal interface.
 */

/*****************************************************************************
* Copyright (c) 2017-2020 by Rambus, Inc. and/or its subsidiaries.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#ifndef EIP74_HW_INTERFACE_H_
#define EIP74_HW_INTERFACE_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip74.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // BIT definitions, bool, uint32_t

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t
#include "device_rw.h"          // Read32, Write32


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// EIP-74 TRNG device EIP number (0x4A) and complement (0xB5)
#define EIP74_SIGNATURE                   ((uint16_t)0xB54A)

// Byte offsets of the EIP74 registers
#define EIP74_REG_OFFS    4

#define EIP74_REG_INPUT_0         (0x00 * EIP74_REG_OFFS)
#define EIP74_REG_INPUT_1         (0x01 * EIP74_REG_OFFS)
#define EIP74_REG_INPUT_2         (0x02 * EIP74_REG_OFFS)
#define EIP74_REG_INPUT_3         (0x03 * EIP74_REG_OFFS)
#define EIP74_REG_OUTPUT_0        (0x00 * EIP74_REG_OFFS)
#define EIP74_REG_OUTPUT_1        (0x01 * EIP74_REG_OFFS)
#define EIP74_REG_OUTPUT_2        (0x02 * EIP74_REG_OFFS)
#define EIP74_REG_OUTPUT_3        (0x03 * EIP74_REG_OFFS)
#define EIP74_REG_STATUS          (0x04 * EIP74_REG_OFFS)
#define EIP74_REG_INTACK          (0x04 * EIP74_REG_OFFS)
#define EIP74_REG_CONTROL         (0x05 * EIP74_REG_OFFS)
#define EIP74_REG_GENERATE_CNT    (0x08 * EIP74_REG_OFFS)
#define EIP74_REG_RESEED_THR_EARLY (0x09 * EIP74_REG_OFFS)
#define EIP74_REG_RESEED_THR      (0x0A * EIP74_REG_OFFS)
#define EIP74_REG_GEN_BLK_SIZE    (0x0B * EIP74_REG_OFFS)
#define EIP74_REG_KEY_0           (0x10 * EIP74_REG_OFFS)
#define EIP74_REG_KEY_1           (0x11 * EIP74_REG_OFFS)
#define EIP74_REG_KEY_2           (0x12 * EIP74_REG_OFFS)
#define EIP74_REG_KEY_3           (0x13 * EIP74_REG_OFFS)
#define EIP74_REG_KEY_4           (0x14 * EIP74_REG_OFFS)
#define EIP74_REG_KEY_5           (0x15 * EIP74_REG_OFFS)
#define EIP74_REG_KEY_6           (0x16 * EIP74_REG_OFFS)
#define EIP74_REG_KEY_7           (0x17 * EIP74_REG_OFFS)
#define EIP74_REG_PS_AI_0         (0x10 * EIP74_REG_OFFS)
#define EIP74_REG_TEST            (0x1C * EIP74_REG_OFFS)
#define EIP74_REG_OPTIONS         (0x1E * EIP74_REG_OFFS)
#define EIP74_REG_VERSION         (0x1F * EIP74_REG_OFFS)

// Default (reset) register values
// Note: when updating the default register values from 0 to something else
//       check how this value is used in the corresponding level0 macro,
//       it may need to be updated too for the boolean operations!
#define EIP74_REG_INTACK_DEFAULT  0x00000000
#define EIP74_REG_CONTROL_DEFAULT 0x00000000
#define EIP74_REG_GEN_BLK_CNT_DEFAULT 0x00000000
#define EIP74_REG_TEST_DEFAULT    0x00000000

#endif /* EIP74_HW_INTERFACE_H_*/

/* end of file eip74_hw_interface.h */
