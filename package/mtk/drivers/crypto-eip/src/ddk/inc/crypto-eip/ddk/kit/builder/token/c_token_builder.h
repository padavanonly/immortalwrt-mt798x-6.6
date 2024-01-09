/* c_token_builder.h
 *
 * Default configuration file for the Token Builder
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


/*----------------------------------------------------------------------------
 * Import the product specific configuration.
 */
#include "cs_token_builder.h"

/* Uncomment this option if the EIP-96 does not use an input packet pointer.
   This is the case when it is used in a system in which packet data is fetched
   outside the control of the EIP-96. Whether this setting is needed depends
   on the hardware environment in which the EIP-96 is integrated.
 */
//#define TKB_NO_INPUT_PACKET_POINTER

/* Uncomment this option if context reuse must be auto-detected. This
   is only supported in EIP-96 HW2.1 and higher.
 */
//#define TKB_AUTODETECT_CONTEXT_REUSE

/* Define this to a nonzero value if the generated token must contain a header
   that includes the Token Header Word. Any other fields are filled with zero.
   This parameter specifies the header size in words.
 */
#ifndef TKB_TOKEN_HEADER_WORD_COUNT
#define TKB_TOKEN_HEADER_WORD_COUNT 0
#endif


/* Which protocol families are enabled? */
//#define TKB_ENABLE_PROTO_BASIC
//#define TKB_ENABLE_PROTO_IPSEC
//#define TKB_ENABLE_PROTO_SSLTLS
//#define TKB_ENABLE_PROTO_MACSEC
//#define TKB_ENABLE_PROTO_SRTP

/* Which protocol-specific options are enabled? */
//#define TKB_ENABLE_IPSEC_ESP
//#define TKB_ENABLE_IPSEC_AH

/* Token builder supports extended IPsec operations that include
 * header processing for ESP tunnel and transport operations.
*/
#define TKB_ENABLE_EXTENDED_IPSEC

/* Token builder inserts special instructions to fix up the ECN bits
   for inbound tunnel protocols. */
#define TKB_ENABLE_ECN_FIXUP

/* Token builder supports extended DTLS operations that include
 * IP and UDP header processing.
*/
//#define TKB_ENABLE_EXTENDED_DTLS

//#define TKB_ENABLE_CRYPTO_WIRELESS
//#define TKB_ENABLE_CRYPTO_XTS
//#define TKB_ENABLE_CRYPTO_CHACHAPOLY

#ifdef TKB_ENABLE_PROTO_BASIC
#define TKB_HAVE_PROTO_BASIC 1
#else
#define TKB_HAVE_PROTO_BASIC 0
#endif

#ifdef TKB_ENABLE_PROTO_IPSEC
#define TKB_HAVE_PROTO_IPSEC 1
#else
#define TKB_HAVE_PROTO_IPSEC 0
#undef TKB_ENABLE_EXTENDEDN_IPSEC
#endif

#ifdef TKB_ENABLE_EXTENDED_IPSEC
#define TKB_HAVE_EXTENDED_IPSEC 1
#else
#define TKB_HAVE_EXTENDED_IPSEC 0
#endif

#ifdef TKB_ENABLE_ECN_FIXUP
#define TKB_HAVE_ECN_FIXUP 1
#else
#define TKB_HAVE_ECN_FIXUP 0
#endif

#ifdef TKB_ENABLE_EXTENDED_DTLS
#define TKB_HAVE_EXTENDED_DTLS 1
#else
#define TKB_HAVE_EXTENDED_DTLS 0
#endif

#ifdef TKB_ENABLE_PROTO_SSLTLS
#define TKB_HAVE_PROTO_SSLTLS 1
#else
#define TKB_HAVE_PROTO_SSLTLS 0
#endif

#ifdef TKB_ENABLE_PROTO_SRTP
#define TKB_HAVE_PROTO_SRTP 1
#else
#define TKB_HAVE_PROTO_SRTP 0
#endif

#ifdef TKB_ENABLE_CRYPTO_WIRELESS
#define TKB_HAVE_CRYPTO_WIRELESS 1
#else
#define TKB_HAVE_CRYPTO_WIRELESS 0
#endif

#ifdef TKB_ENABLE_CRYPTO_CHACHAPOLY
#define TKB_HAVE_CRYPTO_CHACHAPOLY 1
#else
#define TKB_HAVE_CRYPTO_CHACHAPOLY 0
#endif

#ifndef TKB_HAVE_CHACHAPOLY_HW30
#define TKB_HAVE_CHACHAPOLY_HW30 1
#endif



#ifdef TKB_ENABLE_CRYPTO_XTS
#define TKB_HAVE_CRYPTO_XTS 1
#else
#define TKB_HAVE_CRYPTO_XTS 0
#endif


/* Strict checking of function arguments if enabled */
//#define TKB_STRICT_ARGS_CHECK

/* log level for the token builder.
   choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT */
#ifndef LOG_SEVERITY_MAX
#define LOG_SEVERITY_MAX LOG_SEVERITY_CRIT
#endif


/* end of file c_sa_builder.h */
