/* cs_sa_builder.h
 *
 * Product-specific configuration file for the SA Builder.
 */

/*****************************************************************************
* Copyright (c) 2011-2022 by Rambus, Inc. and/or its subsidiaries.
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

/* DDK configuration */
#include "cs_ddk197.h"

/* Specify a specific version of the EIP-96, specify exactly one */
#define SAB_ENABLE_CRYPTO_STANDARD

/* Enable SHA2-512 algorithms */
#define SAB_ENABLE_AUTH_SHA2_512

/* Collectively enable all wireless algorithms */
#define SAB_ENABLE_CRYPTO_WIRELESS

/* Collectively enable SM4, SM3 and related algorithms */
#define SAB_ENABLE_CRYPTO_SM4_SM3

/* Enable AES-XTS */
#define SAB_ENABLE_CRYPTO_XTS

/* Enable ARCFOUR */
#define SAB_ENABLE_CRYPTO_ARCFOUR

/* Enable ChaCha20 and Poly1305 */
#define SAB_ENABLE_CRYPTO_CHACHAPOLY

/* Enable SHA3 */
#define SAB_ENABLE_AUTH_SHA3

/* Which protcol families are enabled? */
#define SAB_ENABLE_PROTO_BASIC
#define SAB_ENABLE_PROTO_IPSEC
#define SAB_ENABLE_PROTO_SSLTLS
//#define SAB_ENABLE_PROTO_MACSEC
//#define SAB_ENABLE_PROTO_SRTP

/* Which protcol-specific options are enabled? */
#define SAB_ENABLE_IPSEC_ESP
//#define SAB_ENABLE_IPSEC_AH

/* Enable if the SA Builder must support extended use case for IPsec
   processing */
/* TODO: check missing header firmware_eip207_api_flow_cs.h */
#define SAB_ENABLE_IPSEC_EXTENDED

/* Enable if the SA Builder must support extended use case for DTLS
   processing */
//#define SAB_ENABLE_DTLS_EXTENDED

/* Enable if the SA Builder must support extended use case for Basic
   processing */
#define SAB_ENABLE_BASIC_EXTENDED

// Set this if tunnel header fields are to be copied into the transform.
// for extended use case.
#define SAB_ENABLE_EXTENDED_TUNNEL_HEADER

/* Enable if the SA Builder must support an engine with fixed SA records
   (e.g. for a record cache) */
//#define SAB_ENABLE_FIXED_RECORD_SIZE
#define SAB_ENABLE_TWO_FIXED_RECORD_SIZES

/* Enable this if you desire to include the ARC4 state in the SA
   record. This requires the hardware to be configured for relative
   ARC4 state offsets */
//#define SAB_ARC4_STATE_IN_SA

#define SAB_ENABLE_384BIT_SEQMASK
/* Enable this if the hardware supports 384-bitsequence number
   masks. If SAB_ENABLE_256BIT_SEQMASK not also set, support 256-bit
   masks via a work-around.*/
#ifdef DDK_EIP197_EIP96_SEQMASK_256

/* Enable this if the hardware supports 256-bit sequence
   number masks. */
#define SAB_ENABLE_256BIT_SEQMASK

/* Enable this to use fixed offsets for sequence number masks regardless of
   size. This only works on hardware that supports 384-bit masks. */
#define SAB_ENABLE_DEFAULT_FIXED_OFFSETS
#endif
/* IF the HW supports 1024-bit masks, the firmware needs larger offsets
   in its large transform record */
#ifdef DDK_EIP197_EIP96_SEQMASK_1024
#define SAB_LARGE_TRANSFORM_OFFSET 40
#define SAB_ENABLE_1024BIT_SEQMASK
#endif

/* Strict checking of function arguments if enabled */
#define SAB_STRICT_ARGS_CHECK

/* log level for the token builder.
   choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT */
#define LOG_SEVERITY_MAX LOG_SEVERITY_WARN


/* end of file cs_sa_builder.h */
