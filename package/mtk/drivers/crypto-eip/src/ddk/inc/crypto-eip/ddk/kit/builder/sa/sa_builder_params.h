/* sa_builder_params.h
 *
 * Type definitions for the parameter structure for the SA Builder.
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


#ifndef SA_BUILDER_PARAMS_H_
#define SA_BUILDER_PARAMS_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* This specifies one of the supported protocols. For each of the
 * supported protocols, the SABuilder_Params_t record contains a
 * protocol-specific extension, defined in a separate header file.
 */
typedef enum
{
    SAB_PROTO_BASIC,
    SAB_PROTO_IPSEC,
    SAB_PROTO_SSLTLS,
    SAB_PROTO_MACSEC,
    SAB_PROTO_SRTP
} SABuilder_Protocol_t;

/* Specify direction: outbound (encrypt) or inbound (decrypt) */
typedef enum
{
    SAB_DIRECTION_OUTBOUND,
    SAB_DIRECTION_INBOUND
} SABuilder_Direction_t;

/* Values for the flags field. Combine any values using a bitwise or.
   If no flags apply, use the value zero. */
#define SAB_FLAG_GATHER            BIT_0
#define SAB_FLAG_SCATTER           BIT_1
#define SAB_FLAG_SUPPRESS_HDRPROC  BIT_2
#define SAB_FLAG_SUPPRESS_HEADER   BIT_3
#define SAB_FLAG_SUPPRESS_PAYLOAD  BIT_4

#define SAB_FLAG_IV_SAVE           BIT_5 /* Save IV back into SA */
#define SAB_FLAG_ARC4_STATE_SAVE   BIT_6 /* Save ARCFOUR state back */
#define SAB_FLAG_ARC4_STATE_LOAD   BIT_7 /* Load ARCFOUR state */
#define SAB_FLAG_HASH_LOAD         BIT_8 /* Load digest value from SA */
#define SAB_FLAG_HASH_SAVE         BIT_9 /* Save digest value into SA */
#define SAB_FLAG_HASH_INTERMEDIATE BIT_10 /* hash message crosses multiple
                                   packets: load/store intermediate digest */
#define SAB_FLAG_COPY_IV           BIT_11 /* Insert the IV into the output packet (basic crypto) */
#define SAB_FLAG_REDIRECT          BIT_12 /* Redirect packets to other interface*/


/* Specify one of the crypto algorithms */
typedef enum
{
    SAB_CRYPTO_NULL,
    SAB_CRYPTO_DES,
    SAB_CRYPTO_3DES,
    SAB_CRYPTO_AES,
    SAB_CRYPTO_ARCFOUR,
    SAB_CRYPTO_KASUMI,
    SAB_CRYPTO_SNOW,
    SAB_CRYPTO_ZUC,
    SAB_CRYPTO_CHACHA20,
    SAB_CRYPTO_SM4,
    SAB_CRYPTO_BC0, /* Auxiliary block cipher algorithm */
} SABuilder_Crypto_t;

/* Specify one of the crypto modes */
typedef enum
{
    SAB_CRYPTO_MODE_ECB,
    SAB_CRYPTO_MODE_CBC,
    SAB_CRYPTO_MODE_OFB,
    SAB_CRYPTO_MODE_CFB,
    SAB_CRYPTO_MODE_CFB1,
    SAB_CRYPTO_MODE_CFB8,
    SAB_CRYPTO_MODE_CTR,
    SAB_CRYPTO_MODE_ICM,
    SAB_CRYPTO_MODE_CCM,       /* Only use with AES, set SAB_AUTH_AES_CCM */
    SAB_CRYPTO_MODE_GCM,       /* Only use with AES, set SAB_AUTH_AES_GCM */
    SAB_CRYPTO_MODE_GMAC,      /* Only use with AES, set SAB_AUTH_AES_GMAC */
    SAB_CRYPTO_MODE_STATELESS, /* For ARC4 */
    SAB_CRYPTO_MODE_STATEFUL, /* For ARC4 */
    SAB_CRYPTO_MODE_XTS,      /* Only use with AES, no authentication */
    SAB_CRYPTO_MODE_XTS_STATEFUL, /* Only use with AES, no authentication.
                                   Stateful operation, allow multiple
                                   operations on the same 'sector' */
    SAB_CRYPTO_MODE_BASIC,
    SAB_CRYPTO_MODE_F8,       /* Only with Kasumi */
    SAB_CRYPTO_MODE_UEA2,     /* Only with SNOW */
    SAB_CRYPTO_MODE_EEA3,     /* Only with ZUC */
    SAB_CRYPTO_MODE_CHACHA_CTR32, /* Only with Chacha20 */
    SAB_CRYPTO_MODE_CHACHA_CTR64, /* Only with Chacha20 */w
} SABuilder_Crypto_Mode_t;

/* Specify one of the IV sources. Not all methods are supported with all
 protocols.*/
typedef enum
{
    SAB_IV_SRC_DEFAULT, /* Default mode for the protocol */
    SAB_IV_SRC_SA,      /* IV is loaded from SA */
    SAB_IV_SRC_PRNG,    /* IV is derived from PRNG */
    SAB_IV_SRC_INPUT,   /* IV is prepended to the input packet. */
    SAB_IV_SRC_TOKEN,   /* IV is included in the packet token. */
    SAB_IV_SRC_SEQ,     /* IV is derived from packet sequence number. */
    SAB_IV_SRC_XORSEQ,  /* IV is derived from packet sequence number XOR-ed
                           with fixed value. */
    SAB_IV_SRC_IMPLICIT, /* IV is derived from packet sequence number,
                            not included in packet data (RFC8750 for ESP) */
} SABuilder_IV_Src_t;

/* Specify one of the hash or authentication methods */
typedef enum
{
    SAB_AUTH_NULL,
    SAB_AUTH_HASH_MD5,
    SAB_AUTH_HASH_SHA1,
    SAB_AUTH_HASH_SHA2_224,
    SAB_AUTH_HASH_SHA2_256,
    SAB_AUTH_HASH_SHA2_384,
    SAB_AUTH_HASH_SHA2_512,
    SAB_AUTH_SSLMAC_MD5,
    SAB_AUTH_SSLMAC_SHA1,
    SAB_AUTH_HMAC_MD5,
    SAB_AUTH_HMAC_SHA1,
    SAB_AUTH_HMAC_SHA2_224,
    SAB_AUTH_HMAC_SHA2_256,
    SAB_AUTH_HMAC_SHA2_384,
    SAB_AUTH_HMAC_SHA2_512,
    SAB_AUTH_AES_XCBC_MAC,
    SAB_AUTH_AES_CMAC_128,  /* Identical to AES_XCBC_MAC */
    SAB_AUTH_AES_CMAC_192,
    SAB_AUTH_AES_CMAC_256,
    SAB_AUTH_AES_CCM,      /* Set matching crypto algorithm and mode */
    SAB_AUTH_AES_GCM,      /* Set matching crypto algorithm and mode */
    SAB_AUTH_AES_GMAC,     /* Set matching crypto algorithm and mode */
    SAB_AUTH_KASUMI_F9,
    SAB_AUTH_SNOW_UIA2,
    SAB_AUTH_ZUC_EIA3,
    SAB_AUTH_HASH_SHA3_224,
    SAB_AUTH_HASH_SHA3_256,
    SAB_AUTH_HASH_SHA3_384,
    SAB_AUTH_HASH_SHA3_512,
    SAB_AUTH_KEYED_HASH_SHA3_224,
    SAB_AUTH_KEYED_HASH_SHA3_256,
    SAB_AUTH_KEYED_HASH_SHA3_384,
    SAB_AUTH_KEYED_HASH_SHA3_512,
    SAB_AUTH_HMAC_SHA3_224,
    SAB_AUTH_HMAC_SHA3_256,
    SAB_AUTH_HMAC_SHA3_384,
    SAB_AUTH_HMAC_SHA3_512,
    SAB_AUTH_POLY1305,
    SAB_AUTH_KEYED_HASH_POLY1305,
    SAB_AUTH_HASH_SM3,
    SAB_AUTH_HMAC_SM3,
} SABuilder_Auth_t;

/* This is the main SA parameter structure.
 *
 * This contains the common fields for all protocol families.
 * Each protocol has a special extension.
 *
 * The entire data structure (including protocol-specific extension) must be
 * prepared before calling SABuilder_GetSizes() and SABuilder_Build_SA().
 *
 * See the SABuilder_Init* functions in the protocol specific headers
 * (sa_builder_ipsec.h etc.) for helper functions to prepare it.
 * All these initialization functions will provide sensible defaults for all
 * fields, but both the crypto algorithm and the authentication algorithm
 * are set to NULL.
 *
 * For a practical use, at least the following fields must be filled in
 * after calling the initialization functions:
 * - CryptoAlgo, Key_p and KeyByteCount if encryption is required.
 *   - CryptoMode if anything other than CBC is required.
 *   - Nonce_p if counter mode is used.
 * - AuthAlgo if authentication is desired and (depending on the
 *   authentication algorithm), one or more of
 *   AuthKey1_p, AuthKey2_p or AuthKey3_p.
 */
typedef struct
{
    /* Protocol related fields */
    SABuilder_Protocol_t protocol;
    SABuilder_Direction_t direction;
    void * ProtocolExtension_p; /* Pointer to the extension record */
    uint32_t flags; /* Generic flags */

    /* Crypto related fields */
    SABuilder_Crypto_t CryptoAlgo; /* Cryptographic algorithm */
    SABuilder_Crypto_Mode_t CryptoMode;
    uint8_t CryptoParameter; /* Additional algorithm parameter,
                                for example number of rounds */
    SABuilder_IV_Src_t IVSrc; /* source of IV */
    uint8_t KeyByteCount; /* Key length in bytes */
    uint8_t *Key_p;    /* pointer to crypto key */
    uint8_t *IV_p;     /* pointer to initialization vector.
                          IV size is implied by crypto algorithm and mode */
    uint8_t *Nonce_p;    /* Cryptographic nonce, e.g. for counter mode */
    /* Note: for ARCFOUR, the stream cipher state will be loaded if the
       SAB_FLAG_ARC4_STATE_LOAD flag is set.
       Nonce_p[0] is the I variable, Nonce_p[1] is the J variable and
       IV_p points to the 256-byte S-array.

       If this flag is not set, the stream cipher state will not be
       loaded at SA build time and the first packet has to specify
       the ARCFOUR state to be initialized from the key.

       For AES-XTS, the Nonce represents Key2, */

    /* Authentication related fields */
    SABuilder_Auth_t AuthAlgo; /* Authentication algorithm */
    uint8_t AuthKeyByteCount; /* Number of bytes in authentication key
                                 (only used for keyed SHA3 and HMAC-SHA3) */
    uint8_t *AuthKey1_p;
    uint8_t *AuthKey2_p;
    uint8_t *AuthKey3_p;
    /* The SA Builder expects authentication keys in their
       preprocessed form. The sizes of the authentication keys are
       implied by AuthAlgo. For details on the preprocessed
       authentication keys, see the Implementation Notes document.

       Plain hash functions with no digest loading:
                   No authentication keys are used.
       Plain hash functions with digest loading:
                   AuthKey1 is the digest to be loaded.
       Any of the HMAC functions:
                   AuthKey1 is precomputed inner digest.
                   AuthKey2 is precomputed outer digest.
       SSL-MAC-MD$:
                   AuthKey1 is precomputed inner digest.
                   AuthKey2 is precomputed outer digest.
       SSL-HAC-SHA1:
                   AuthKey1 is the authentication key (not pre-processed).
       GCM or GMAC:
                   AuthKey1 is H = E(K,0). This is an all-zero block encrypted
                   with the encryption key.
       XCBC_MAC (RFC3566):
                   AuthKey1 = K1
                   AuthKey2 = K2
                   AuthKey3 = K3
       CMAC (RFC4493,RFC4494):
                   AuthKey1 = K
                   AuthKey2 = K1
                   AuthKey3 = K2
       CCM:
                   No authentication keys required: the SA builder will
                   use the encryption key.

       Unused AuthKey fields may be NULL.
    */
    uint8_t RedirectInterface;
    /* Interface ID to which packets must be redirected */

    uint32_t OffsetARC4StateRecord;
    /* Offset of the ARC4 State record with respect to the start of
       the SA record. The application can set this to specify a
       desired offset for this record. If left 0, the SA Builder will
       put the ARC4 state right after the SA.

       This parameter is only used when SAB_ARC4_STATE_IN_SA is defined.
    */

    /* The following values reflect control words. The application shall
       not touch those */
    uint32_t CW0, CW1;

    /* The following values reflect. offsets of certain fields in the SA
     buffer. The application shall not touch these.*/
    uint8_t OffsetDigest0; /* Word-offset of Digest 0 */
    uint8_t OffsetDigest1; /* Word-offset of Digest 1 */
    uint8_t OffsetSeqNum;  /* Word-offset of Sequence Number  */
    uint8_t OffsetSeqMask; /* Word-offset of Sequence Number Mask */
    uint8_t OffsetIV;      /* Word-offset of IV */
    uint8_t OffsetIJPtr;   /* Word-offset of IJ Pointer for ARC4 */
    uint8_t OffsetARC4State; /* Word-offset of ARC4 state */
    /* The following values reflect the width of certain fields in the SA
       (those that may be updated and may be read back)*/
    uint8_t SeqNumWord32Count; /* Width of the sequence number */
    uint8_t SeqMaskWord32Count;/* Width of the sequence number masks*/
    uint8_t IVWord32Count;  /* Width of the IV */
} SABuilder_Params_t;


#endif /* SA_BUILDER_PARAMS_H_ */


/* end of file sa_builder_params.h */
