/* token_builder_interna.h
 *
 * Internal APIs for the Token Builder implementation.
 * This includes thee actual definition of the Token Context Record.
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


#ifndef TOKEN_BUILDER_INTERNAL_H_
#define TOKEN_BUILDER_INTERNAL_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "token_builder.h"
#include "basic_defs.h"

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Various fields in the token header word.
 */
#define TKB_HEADER_RC_NO_REUSE       0x00000000
#define TKB_HEADER_RC_REUSE          0x00100000
#define TKB_HEADER_RC_AUTO_REUSE     0x00200000

#define TKB_HEADER_IP                0x00020000

#ifdef TKB_NO_INPUT_PACKET_POINTER
#define TKB_HEADER_IP_OPTION TKB_HEADER_IP
#else
#define TKB_HEADER_IP_OPTION 0
#endif

#ifdef TKB_AUTODETECT_CONTEXT_REUSE
#define TKB_HEADER_RC_OPTION TKB_HEADER_RC_AUTO_REUSE
#else
#define TKB_HEADER_RC_OPTION TKB_HEADER_RC_NO_REUSE
#endif

#define TKB_HEADER_DEFAULT           (TKB_HEADER_RC_OPTION | TKB_HEADER_IP_OPTION)

#define TKB_HEADER_C                 0x02000000


#define TKB_HEADER_UPD_HDR           0x00400000
#define TKB_HEADER_APP_RES           0x00800000

#define TKB_HEADER_PAD_VERIFY        0x01000000
#define TKB_HEADER_IV_DEFAULT        0x00000000
#define TKB_HEADER_IV_PRNG           0x04000000
#define TKB_HEADER_IV_TOKEN_2WORDS   0x18000000
#define TKB_HEADER_IV_TOKEN_4WORDS   0x1c000000

#define TKB_HEADER_U                 0x20000000

/* CCM flag byte, includes Adata and L=4, M has to be filled in */
#define TKB_CCM_FLAG_ADATA_L4        0x43
/* CCM flag byte, includes Adata and L=3, M has to be filled in */
#define TKB_CCM_FLAG_ADATA_L3        0x42

#define TKB_ESP_FLAG_CLEAR_DF      BIT_0
#define TKB_ESP_FLAG_SET_DF       BIT_1
#define TKB_ESP_FLAG_REPLACE_DSCP  BIT_2
#define TKB_ESP_FLAG_CLEAR_ECN     BIT_3
#define TKB_ESP_FLAG_NAT           BIT_6
#define TKB_DTLS_FLAG_PLAINTEXT_HDR BIT_4
#define TKB_DTLS_FLAG_CAPWAP       BIT_5

/* The protocol values have to agree with those used in token_builder_core.c
 */
typedef enum
{
    TKB_PROTO_ESP_OUT = 0,
    TKB_PROTO_ESP_IN = 1,
    TKB_PROTO_ESP_CCM_OUT = 2,
    TKB_PROTO_ESP_CCM_IN = 3,
    TKB_PROTO_ESP_GCM_OUT = 4,
    TKB_PROTO_ESP_GCM_IN = 5,
    TKB_PROTO_ESP_GMAC_OUT = 6,
    TKB_PROTO_ESP_GMAC_IN = 7,
    TKB_PROTO_SSLTLS_OUT = 8,
    TKB_PROTO_SSLTLS_IN = 9,
    TKB_PROTO_BASIC_CRYPTO = 10,
    TKB_PROTO_BASIC_HASH = 11,
    TKB_PROTO_SRTP_OUT = 12,
    TKB_PROTO_SRTP_IN = 13,
    TKB_PROTO_BASIC_CRYPTHASH = 14,
    TKB_PROTO_BASIC_CCM_OUT = 15,
    TKB_PROTO_BASIC_CCM_IN = 16,
    TKB_PROTO_BASIC_GCM_OUT = 17,
    TKB_PROTO_BASIC_GCM_IN = 18,
    TKB_PROTO_BASIC_GMAC_OUT = 19,
    TKB_PROTO_BASIC_GMAC_IN = 20,
    TKB_PROTO_SSLTLS_GCM_OUT = 21,
    TKB_PROTO_SSLTLS_GCM_IN = 22,
    TKB_PROTO_BASIC_XTS_CRYPTO = 23,
    TKB_PROTO_BASIC_KASUMI_HASH = 24,
    TKB_PROTO_BASIC_SNOW_HASH = 25,
    TKB_PROTO_BASIC_ZUC_HASH = 26,
    TKB_PROTO_BASIC_HASHENC = 27,
    TKB_PROTO_BASIC_DECHASH = 28,
    TKB_PROTO_BASIC_CHACHAPOLY_OUT = 29,
    TKB_PROTO_BASIC_CHACHAPOLY_IN = 30,
    TKB_PROTO_TLS13_GCM_OUT = 31,
    TKB_PROTO_TLS13_GCM_IN = 32,
    TKB_PROTO_TLS13_CHACHAPOLY_OUT = 33,
    TKB_PROTO_TLS13_CHACHAPOLY_IN = 34,
    TKB_PROTO_ESP_CHACHAPOLY_OUT = 35,
    TKB_PROTO_ESP_CHACHAPOLY_IN = 36,
    TKB_PROTO_SSLTLS_CHACHAPOLY_OUT = 37,
    TKB_PROTO_SSLTLS_CHACHAPOLY_IN = 38,
    TKB_PROTO_SSLTLS_CCM_OUT = 39,
    TKB_PROTO_SSLTLS_CCM_IN = 40,
    TKB_PROTO_TLS13_CCM_OUT = 41,
    TKB_PROTO_TLS13_CCM_IN = 42,
    TKB_PROTO_BASIC_HMAC_PRECOMPUTE = 43,
    TKB_PROTO_BASIC_HMAC_CTXPREPARE = 44,
    TKB_PROTO_BASIC_BYPASS = 45,
} TokenBuilder_Protocol_t;

/* The header protocol values have to agree with those used in
 * token_builder_core.c.
 *
 * Header protocol operations in the token builder attempt to achieve the
 * same results as firmware using the extended IPsec operations.
 *
 * Values are chosen the same as in sa_builder_extended_internal.h
 */
typedef enum
{
    TKB_HDR_BYPASS = 0,
    TKB_HDR_IPV4_OUT_TRANSP_HDRBYPASS = 1,
    TKB_HDR_IPV4_OUT_TUNNEL = 2,
    TKB_HDR_IPV4_IN_TRANSP_HDRBYPASS = 3,
    TKB_HDR_IPV4_IN_TUNNEL = 4,
    TKB_HDR_IPV4_OUT_TRANSP = 5,
    TKB_HDR_IPV4_IN_TRANSP = 6,
    TKB_HDR_IPV6_OUT_TUNNEL = 7,
    TKB_HDR_IPV6_IN_TUNNEL = 8,
    TKB_HDR_IPV6_OUT_TRANSP_HDRBYPASS = 9,
    TKB_HDR_IPV6_IN_TRANSP_HDRBYPASS = 10,
    TKB_HDR_IPV6_OUT_TRANSP = 11,
    TKB_HDR_IPV6_IN_TRANSP = 12,

    /* DTLS and DLTS-CAPWAP */
    TKB_HDR_IPV4_OUT_DTLS = 13,
    TKB_HDR_IPV6_OUT_DTLS = 33,
    TKB_HDR_DTLS_UDP_IN = 14,

    /* IPsec with NAT-T. Must be in the same order as the non NAT-T
       counterparts */
    TKB_HDR_IPV4_OUT_TRANSP_HDRBYPASS_NATT = 21,
    TKB_HDR_IPV4_OUT_TUNNEL_NATT = 22,
    TKB_HDR_IPV4_IN_TRANSP_HDRBYPASS_NATT = 23,
    TKB_HDR_IPV4_IN_TUNNEL_NATT = 24,
    TKB_HDR_IPV4_OUT_TRANSP_NATT = 25,
    TKB_HDR_IPV4_IN_TRANSP_NATT = 26,
    TKB_HDR_IPV6_OUT_TUNNEL_NATT = 27,
    TKB_HDR_IPV6_IN_TUNNEL_NATT = 28,
    TKB_HDR_IPV6_OUT_TRANSP_HDRBYPASS_NATT = 29,
    TKB_HDR_IPV6_IN_TRANSP_HDRBYPASS_NATT = 30,
    TKB_HDR_IPV6_OUT_TRANSP_NATT = 31,
    TKB_HDR_IPV6_IN_TRANSP_NATT = 32,

} TokenBuilder_HdrProto_t;

/* The IV handling values have to agree with those used in token_builder_core.c
 */
typedef enum
{
    TKB_IV_INBOUND_CTR = 0,
    TKB_IV_INBOUND_CBC = 1,
    TKB_IV_OUTBOUND_CTR = 2,
    TKB_IV_OUTBOUND_CBC = 3,
    TKB_IV_COPY_CBC = 4,
    TKB_IV_COPY_CTR = 5,
    TKB_IV_OUTBOUND_2WORDS = 6,
    TKB_IV_OUTBOUND_4WORDS = 7,
    TKB_IV_OUTBOUND_TOKEN_2WORDS = 8,
    TKB_IV_COPY_TOKEN_2WORDS = 9,
    TKB_IV_OUTBOUND_TOKEN_SRTP = 10,
    TKB_IV_KASUMI_F8 = 11,
    TKB_IV_SNOW_UEA2 = 12,
    TKB_IV_ZUC_EEA3 = 13,
    TKB_IV_OUTBOUND_TOKEN_4WORDS = 14,
    TKB_IV_COPY_TOKEN_4WORDS = 15,
} TokenBuilder_IVHandling_t;


/* Context update handling for SSL/TLS. The values have to agree with
   those used in token_builder_core.c
*/
typedef enum
{
    TKB_UPD_NULL = 0,
    TKB_UPD_ARC4 = 1,
    TKB_UPD_IV2 = 2,
    TKB_UPD_IV4 = 3,
    TKB_UPD_BLK = 4,
} TokenBuilder_UpdateHandling_t;


/* The TokenBuilder_Context_t (Token Context Record) contains all
   information that the Token Builder requires for each SA.
*/
typedef struct
{
    uint32_t TokenHeaderWord;
    uint8_t protocol;
    uint8_t hproto;
    uint8_t IVByteCount;
    uint8_t ICVByteCount;
    uint8_t PadBlockByteCount;
    uint8_t ExtSeq;
    uint8_t AntiReplay;
    uint8_t SeqOffset;
    uint8_t IVOffset;
    uint8_t DigestWordCount;
    uint8_t IVHandling;
    uint8_t DigestOffset;
    uint8_t protocol_next;
    uint8_t hproto_next;
    uint8_t IVHandling_next;
    uint8_t HeaderWordFields_next;
    uint32_t NATT_Ports;
    union {
        struct {
            uint8_t UpdateHandling;
            uint8_t ESPFlags;
            uint8_t TTL;
            uint8_t DSCP;
            uint32_t CCMSalt;
            uint32_t CW0,CW1; /* Context control words */
        } generic;
        struct { // Make the SRTP salt key overlap with fields not used in SRTP.
            uint32_t SaltKey0,SaltKey1,SaltKey2,SaltKey3;
        } srtp;
    } u;
#ifdef TKB_ENABLE_EXTENDED_IPSEC
    uint8_t TunnelIP[32];
#endif
} TokenBuilder_Context_t;


#endif /* TOKEN_BUILDER_INTERNAL_H_ */

/* end of file token_builder_internal.h */
