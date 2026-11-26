/* cs_ddk197.h
 *
 * Top-level DDK-197 product configuration
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

#ifndef CS_DDK197_H_
#define CS_DDK197_H_

// Version string to show at startup
#define DDK_EIP197_VERSION_STRING "DDK-197-GPL v5.6.1"

// Enable if hardware supports 256-bit sequence number masks natively.
#define DDK_EIP197_EIP96_SEQMASK_256

// Enable if hardware supports 1024-bit sequence number masks natively.
//#define DDK_EIP197_EIP96_SEQMASK_1024

// Prevent update information from being appended to the output packet.
// Update information is used for inbound ESP transport: length, protocol,
// checksum.
//#define DDK_EIP197_EIP96_BLOCK_UPDATE_APPEND

// Enable extended error codes from the EIP-96
#define DDK_EIP197_EXTENDED_ERRORS_ENABLE

// Enable use of meta-data in input and output tokens with the EIP-197 firmware
// Note: The EIP-197 firmware must support this before in order to be enabled
#define DDK_EIP197_FW_IOTOKEN_METADATA_ENABLE

// Ring Control: Command Descriptor offset (maximum size), in bytes
//               16 32-bit words, 64 bytes
// Note: consider CPU d-cache line size alignment
//       if the Command Ring buffer is in cached memory!
#define DDK_EIP202_CD_OFFSET_BYTE_COUNT     128

// Ring Control: Result Descriptor offset (maximum size), in bytes
//               16 32-bit words, 64 bytes
// Note: consider CPU d-cache line size alignment
//       if the Result Ring buffer is in cached memory!
#define DDK_EIP202_RD_OFFSET_BYTE_COUNT     128


/*----------------------------------------------------------------------------
 * CAUTION: Do not change the parameters below unless
 *          the EIP-197 hardware configuration changes!
 */

// EIP-197 Server with ICE configuration and optional OCE
#define DDK_EIP197_SRV_ICE

// Test Firmware 3.3 features.
#define DDK_EIP197_FW33_FEATURES

// Firmware supports XFRM API
#define DDK_EIP197_FW_XFRM_API_

#endif // CS_DDK197_H_


/* end of file cs_ddk197.h */
