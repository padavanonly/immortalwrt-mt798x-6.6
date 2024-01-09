/* SPDX-License-Identifier: GPL */
/* cs_token_builder.h
 *
 * Product-specific configuration file for the token builder.
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

/* Uncomment this option if the EIP-96 does not use an input packet pointer.
 *  This is the case when it is used in a system in which packet data is fetched
 *  outside the control of the EIP-96. Whether this setting is needed depends
 *  on the hardware environment in which the EIP-96 is integrated.
 */
#define TKB_NO_INPUT_PACKET_POINTER

/* Uncomment this option if context reuse must be auto-detected. This
 *  is only supported in EIP-96 HW2.1 and higher.
 */
#define TKB_AUTODETECT_CONTEXT_REUSE


#define TKB_ENABLE_CRYPTO_WIRELESS
#define TKB_ENABLE_CRYPTO_XTS
#define TKB_ENABLE_CRYPTO_CHACHAPOLY


/* Which protocol families are enabled? */
#define TKB_ENABLE_PROTO_BASIC
#define TKB_ENABLE_PROTO_IPSEC
#define TKB_ENABLE_PROTO_SSLTLS
/* #define TKB_ENABLE_PROTO_MACSEC */
/* #define TKB_ENABLE_PROTO_SRTP */

/* Which protocol-specific options are enabled? */
#define TKB_ENABLE_IPSEC_ESP
/* #define TKB_ENABLE_IPSEC_AH */

/* Strict checking of function arguments if enabled */
#define TKB_STRICT_ARGS_CHECK

#define TKB_ENABLE_CRYPTO_WIRELESS

/* log level for the token builder.
 *  choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT
 */
#define LOG_SEVERITY_MAX LOG_SEVERITY_WARN

/* end of file cs_token_builder.h */
