/* iotoken_ext.h
 *
 * IOToken API for Extended Server with Input Classification Engine (ICE) only
 *
 * All the functions in this API are re-entrant for different tokens.
 * All the functions in this API can be used concurrently for different tokens.
 *
 */

/*****************************************************************************
* Copyright (c) 2016-2022 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef IOTOKEN_EXT_H_
#define IOTOKEN_EXT_H_

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Standard IOToken API
#include "iotoken.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Length of the EIP-197 Server extended command token (in 32-bit words)
// Note: Based on 64-bit Context (SA) address only!
// Take the maximum possible value, including 4 optional bypass data words.
#define IOTOKEN_IN_WORD_COUNT        (6 + 4)
// The inline token may be larger.
#define IOTOKEN_IN_WORD_COUNT_IL     (8 + 4)

// Length of the EIP-197 Server extended result token (in 32-bit words)
// Take the maximum possible value, including 4 optional bypass data words.
#define IOTOKEN_OUT_WORD_COUNT       (8 + 4)
// The inline token may be larger.
#define IOTOKEN_OUT_WORD_COUNT_IL    (8 + 4)

#define IOTOKEN_FLOW_TYPE            3 // Extended packet flow

// DTLS content type
typedef enum
{
    IOTOKEN_DTLS_CTYPE_CHANGE_CIPHER,
    IOTOKEN_DTLS_CTYPE_ALERT,
    IOTOKEN_DTLS_CTYPE_HANDSHAKE,
    IOTOKEN_DTLS_CTYPE_APP_DATA
} IOToken_DTLS_ContentType_t;

// Input token header word extended options
typedef struct
{
    // true - DTLS CAPWAP header is present in the input DTLS packet.
    //        Used only for inbound (Direction = 1) and ignored for outbound
    //        processing.
    bool fCAPWAP;

    // Direction for the packet flow. true - inbound, false - outbound
    bool fInbound;

    // DTLS content type
    IOToken_DTLS_ContentType_t ContentType;

} IOToken_Options_Ext_t;

// Input Token descriptor extension
typedef struct
{
    // ARC4 state record pre-fetch control
    // true - ARC4 state pre-fetch is used, otherwise (false) not
    bool fARC4Prefetch;

    // Input token header extended options
    IOToken_Options_Ext_t Options;

    // Packet flow selection (see the EIP-197 firmware API specification)
    unsigned int HW_Services;

    // Offset to the end of the bypass data (start of packet data) or
    // length (in bytes) of bypass data
    unsigned int Offset_ByteCount;

    // Next header to be inserted with pad data during the ESP Outbound
    // encapsulation, otherwise zero
    unsigned int NextHeader;

    // Number of bytes for additional authentication data in combined
    // encyrpt-authenticate operations.
    unsigned int AAD_ByteCount;

    // User defined field for flow lookup.
    uint16_t UserDef;

    // Strip padding after IP datagrams
    bool fStripPadding;

    // Allow padding after IP datagrams
    bool fAllowPadding;

    // Copy Flow Label
    bool fFL;

    // Verify IPv4 checksum
    bool fIPv4Chksum;

    // Verify L4 checksum;
    bool fL4Chksum;

    // Parse Ethernet header
    bool fParseEther;

    // Keep outer header fter inbound tunnel operation
    bool fKeepOuter;

    // Always encapsulate last destination header (IPv6 ESP outbound transport).
    bool fEncLastDest;

    // Set to specify inline interface.
    bool fInline;

    // Reduced mask size for inbound SA with 1024-bit mask.
    unsigned int SequenceMaskBitCount;

    // Bypass data which will be passed unmodified from Input Token
    // to Output Token
    // Buffer size must IOTOKEN_BYPASS_WORD_COUNT (32-bit words) at least
    // If not used set to NULL (IOTOKEN_BYPASS_WORD_COUNT must be set to 0)
    unsigned int * BypassData_p;

} IOToken_Input_Dscr_Ext_t;

// Output Token descriptor extension
typedef struct
{
    // For tunnel ESP processing
    // IPv4 Type of Service or IPv6 Traffic Class field from inner IP header
    unsigned int TOS_TC;

    // For tunnel ESP processing
    // IPv4 DontFragment field from inner IP header
    bool fDF;

    // Classification errors (CLE), see firmware specification for details
    unsigned int CfyErrors;

    // Extended errors, see firmware specification for details.
    unsigned int ExtErrors;

    // Copy of the Offset_ByteCount from the Input Token
    unsigned int Offset_ByteCount;

    // Valid only when fLengthAppended is true and both fChecksumAppended and
    // fNextHeaderAppended are false (see Output Token Descriptor)
    // Delta between the result packet length and the length value that
    // must be inserted in the IP header
    unsigned int IPDelta_ByteCount;

    // Context record (SA) physical address
    IOToken_PhysAddr_t SA_PhysAddr;

    // Application-specific reference for Network Packet Header processing
    // Comes to the Engine via the Context (SA) Record
    unsigned int NPH_Context;

    // Offset within the IPv6 header needed to complete the header processing
    unsigned int NextHeaderOffset;

    // Bypass data which will be passed unmodified from Input Token
    // to Output Token

    // Input packet was IPv6
    bool fInIPv6;

    // Ethernet parsing performed
    bool fFromEther;

    // Output packet is IPv6
    bool fOutIPv6;

    // Inbound tunnel operation
    bool fInbTunnel;


    // Buffer size must IOTOKEN_BYPASS_WORD_COUNT (32-bit words) at least
    // If not used set to NULL (IOTOKEN_BYPASS_WORD_COUNT must be set to 0)
    unsigned int * BypassData_p;

} IOToken_Output_Dscr_Ext_t;

// Lookaside Crypto packet Flow
#define IOTOKEN_CMD_PKT_LAC       0x04

// Lookaside IPsec packet flow
#define IOTOKEN_CMD_PKT_LIP       0x02

// Inline IPsec packet flow
#define IOTOKEN_CMD_PKT_IIP       0x02

// Lookaside IPsec packet flow, custom classification
#define IOTOKEN_CMD_PKT_LIP_CUST  0x03

// Inline IPsec packet flow, custom classification
#define IOTOKEN_CMD_PKT_IIP_CUST  0x03

// Lookaside IPsec with Token Builder offload packet flow
#define IOTOKEN_CMD_PKT_LAIP_TB   0x18

// Lookaside Basic with Token Builder offload packet flow
#define IOTOKEN_CMD_PKT_LAB_TB    0x20

// Inline Basic with Token Builder offload packet flow
#define IOTOKEN_CMD_PKT_IB_TB    0x20

// Lookaside DTLS (including CAPWAP)
#define IOTOKEN_CMD_PKT_LDT       0x28

// Inline DTLS (including CAPWAP)
#define IOTOKEN_CMD_PKT_IDT       0x28

// Inline MACsec
#define IOTOKEN_CMD_PKT_IMA       0x38

// Invalidate Transform Record command
#define IOTOKEN_CMD_INV_TR        0x06

// Invalidate Flow Record command
#define IOTOKEN_CMD_INV_FR        0x05

// Bypass operation.
#define IOTOKEN_CMD_BYPASS        0x08

// Bit mask that enables the flow lookup. Otherwise the transform lookup is performed.
#define IOTOKEN_CMD_FLOW_LOOKUP   0x80

#define IOTOKEN_USE_HW_SERVICE
#define IOTOKEN_USE_TRANSFORM_INVALIDATE


/* end of file iotoken_ext.h */


#endif /* IOTOKEN_EXT_H_ */
