/* sa_builder_params_ssltls.h
 *
 * SSL/TLS/DTLS specific extension to the SABuilder_Params_t type.
 */

/*****************************************************************************
* Copyright (c) 2011-2021 by Rambus, Inc. and/or its subsidiaries.
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


#ifndef SA_BUILDER_PARAMS_SSLTLS_H_
#define SA_BUILDER_PARAMS_SSLTLS_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "sa_builder_params.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Flag bits for the SSLTLSFlags field. Combine any values using a
   bitwise or.
 */
#define SAB_DTLS_NO_ANTI_REPLAY  BIT_0 /* Disable anti-replay protection */
#define SAB_DTLS_MASK_128        BIT_1 /* Use 128-bit anti-replay mask instead of 64-bit (for downward compatibility)*/
#define SAB_DTLS_MASK_32         BIT_2 /* Use 32-bit anti-replay mask instead of 64-bit (for downward compatibility)*/
#define SAB_DTLS_IPV4            BIT_3 /* DLTS transported over UDP IPv4 */
#define SAB_DTLS_IPV6            BIT_4 /* DTLS transported over UDP IPv6 */
#define SAB_DTLS_UDPLITE         BIT_5 /* Use UDPLite instead of UDP */
#define SAB_DTLS_CAPWAP          BIT_6 /* Use CAPWAP/DTLS */
#define SAB_DTLS_PROCESS_IP_HEADERS BIT_7 /* Perform header processing */
#define SAB_DTLS_PLAINTEXT_HEADERS BIT_8 /* Expect DTLS headers in plaintext packets */
#define SAB_DTLS_FIXED_SEQ_OFFSET BIT_9 /* Use fixed sequence number offset
                                             for 64 or 128 bit masks. */
#define SAB_DTLS_EXT_PROCESSING  BIT_10 /* Extended processing for DTLS
                                            in stand-alone token builder */

/* SSL, TLS and DTLS versions */
#define SAB_SSL_VERSION_3_0   0x0300
#define SAB_TLS_VERSION_1_0   0x0301
#define SAB_TLS_VERSION_1_1   0x0302
#define SAB_TLS_VERSION_1_2   0x0303
#define SAB_TLS_VERSION_1_3   0x0304
#define SAB_DTLS_VERSION_1_0  0xFEFF
#define SAB_DTLS_VERSION_1_2  0xFEFD

/* Extension record for SAParams_t. Protocol_Extension_p must point
   to this structure when the SSL, TLS or DTLS  protocol is used.

   SABuilder_Iinit_SSLTLS() will fill all fields in this structure  with
   sensible defaults.
 */
typedef struct
{
    uint32_t SSLTLSFlags;  /* See SAB_{SSL,TLS,DTLS}_* flag bits above*/
    uint16_t version;
    uint16_t epoch;        /* for DTLS only */

    uint32_t SeqNum;       /* Least significant part of sequence number.*/
    uint32_t SeqNumHi;     /* Most significant part of sequence number. */

    uint32_t SeqMask[12];   /* Up to 384-bit mask window
                              Only used with inbound DTLS.
                              Initialize first word with 1, others with 0. */

    uint32_t PadAlignment; /* Align padding to specified multiple of bytes.
                              This must be a power of two between 4 and 256.
                              If zero, default pad alignment is used.*/
    uint32_t ContextRef; /* Reference to application context */
    uint16_t SequenceMaskBitCount; /* Number of bits in sequence number mask.
                                      Default is 64 */
    uint16_t ICVByteCount; /* Length of ICV in bytes. If left zero, a default
                              value is used, compatible with the authentication
                              algorithm, */
} SABuilder_Params_SSLTLS_t;


#endif /* SA_BUILDER_PARAMS_TLS_H_ */


/* end of file sa_builder_params_ssltls.h */
