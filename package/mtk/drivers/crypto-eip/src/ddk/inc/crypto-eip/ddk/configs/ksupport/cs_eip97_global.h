/* cs_eip97_global.h
 *
 * Top-level configuration parameters
 * for the EIP-97 Global Control Driver Library
 *
 */

/*****************************************************************************
* Copyright (c) 2011-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef CS_EIP97_GLOBAL_H_
#define CS_EIP97_GLOBAL_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
//#include "cs_driver.h"

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Number of Ring interfaces
// Maximum number of Ring interfaces that can be used
// Should not exceed the number of rings physically available
#define EIP97_GLOBAL_MAX_NOF_RING_TO_USE     1

#ifdef DRIVER_ENABLE_SWAP_SLAVE
#define EIP97_GLOBAL_ENABLE_SWAP_REG_DATA
#endif

//#define EIP97_GLOBAL_BUS_BURST_SIZE          4

#ifdef DRIVER_ENABLE_SWAP_MASTER
// Enable Flow Lookup Data Endianness Conversion
// by the Classification Engine hardware master interface
#define EIP97_GLOBAL_BYTE_SWAP_FLUE_DATA

// Enable Flow Record Data Endianness Conversion
// by the Classification Engine hardware master interface
#define EIP97_GLOBAL_BYTE_SWAP_FLOW_DATA

// Enable Context Data Endianness Conversion
// by the Processing Engine hardware master interface
#define EIP97_GLOBAL_BYTE_SWAP_CONTEXT_DATA

// Enable ARC4 Context Data Endianness Conversion
// by the PE hardware master interface
#define EIP97_GLOBAL_BYTE_SWAP_ARC4_CONTEXT_DATA

// One or several of the following methods must be configured:
// Swap bytes within each 32 bit word
#define EIP97_GLOBAL_BYTE_SWAP_METHOD_32
// Swap 32 bit chunks within each 64 bit chunk
//#define EIP97_GLOBAL_BYTE_SWAP_METHOD_64
// Swap 64 bit chunks within each 128 bit chunk
//#define EIP97_GLOBAL_BYTE_SWAP_METHOD_128
// Swap 128 bit chunks within each 256 bit chunk
//#define EIP97_GLOBAL_BYTE_SWAP_METHOD_256
#endif

// Packet Engine hold output data.
// This parameter can be used for the in-place packet transformations when
// the transformed result packet is larger than the original packet.
// In-place means that the same packet buffer is used to store the original
// as well as the transformed packet data.
// This value of this parameter defines the number of last the 8-byte blocks
// that the packet engine will hold in its internal result packet buffer
// until the packet processing is completed.
#ifdef DRIVER_PE_HOLD_OUTPUT_DATA
#define EIP97_GLOBAL_EIP96_PE_HOLD_OUTPUT_DATA      \
                                DRIVER_PE_HOLD_OUTPUT_DATA
#endif // EIP97_GLOBAL_EIP96_PE_HOLD_OUTPUT_DATA

// if nonzero, packet update information (checksum, length, protocol) will
// not be appended to packet data. Indicate in result token flags
// which fields have to be updated.
#define EIP97_GLOBAL_EIP96_BLOCK_UPDATE_APPEND          1

// If nonzero, add an IP length delta field to the result token.
#define EIP97_GLOBAL_EIP96_LEN_DELTA_ENABLE             1


// Enable advance threshold mode for optimal performance and
// latency compensation in the internal engine buffers
#define EIP97_GLOBAL_DFE_ADV_THRESH_MODE_FLAG           1

// Define this parameter for automatic calculation of the EIP-202 and EIP-206
// Global Control threshold values.
// If not defined then the statically configured values will be used.
//#define EIP97_GLOBAL_THRESH_CONFIG_AUTO

#ifndef EIP97_GLOBAL_THRESH_CONFIG_AUTO
// Example static EIP-202 and EIP-206 Global Control threshold configuration
// for optimal performance
#define EIP97_GLOBAL_DFE_MIN_DATA_XFER_SIZE     5
#define EIP97_GLOBAL_DFE_MAX_DATA_XFER_SIZE     9

#define EIP97_GLOBAL_DSE_MIN_DATA_XFER_SIZE     7
#define EIP97_GLOBAL_DSE_MAX_DATA_XFER_SIZE     8

#define EIP97_GLOBAL_DFE_MIN_TOKEN_XFER_SIZE    5
#define EIP97_GLOBAL_DFE_MAX_TOKEN_XFER_SIZE    EIP97_GLOBAL_MAX_TOKEN_SIZE
#endif

// Strict argument checking for the input parameters
// If required then define this parameter in the top-level configuration
#define EIP97_GLOBAL_STRICT_ARGS

// Finite State Machine that can be used for verifying that the Driver Library
// API is called in a correct order
#define EIP97_GLOBAL_DEBUG_FSM

// Configuration parameter extensions
#include "cs_eip97_global_ext.h"


#endif /* CS_EIP97_GLOBAL_H_ */


/* end of file cs_eip97_global.h */
