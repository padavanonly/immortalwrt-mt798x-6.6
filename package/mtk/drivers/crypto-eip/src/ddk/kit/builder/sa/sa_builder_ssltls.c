/* sa_builder_ssltls.c
 *
 * SSL/TLS/DTLS specific functions (for initialization of SABuilder_Params_t
 * structures and for building the SSL/TLS/DTLS specifc part of an SA.).
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */
#include "sa_builder_ssltls.h"
#include "sa_builder_internal.h" /* SABuilder_SetSSLTLSParams */

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_sa_builder.h"
#include "basic_defs.h"
#include "log.h"

#ifdef SAB_ENABLE_PROTO_SSLTLS

/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#ifdef SAB_ENABLE_384BIT_SEQMASK
#define SAB_SEQUENCE_MAXBITS 384
#else
#define SAB_SEQUENCE_MAXBITS 128
#endif


/*----------------------------------------------------------------------------
 * Local variables
 */

/*----------------------------------------------------------------------------
 * SABuilder_Init_SSLTLS
 *
 * This function initializes the SABuilder_Params_t data structure and its
 * SABuilder_Params_SSLTLS_t extension with sensible defaults for SSL, TLS
 * and DTLS processing.
 *
 * SAParams_p (output)
 *   Pointer to SA parameter structure to be filled in.
 * SAParamsSSLTLS_p (output)
 *   Pointer to SSLTLS parameter extension to be filled in
 * version (input)
 *   Version code for the desired protcol (choose one of the SAB_*_VERSION_*
 *   constants from sa_builder_params_ssltls.h).
 * direction (input)
 *   Must be one of SAB_DIRECTION_INBOUND or SAB_DIRECTION_OUTBOUND.
 *
 * Both the crypto and the authentication algorithm are initialized to
 * NULL. The crypto algorithm (which may remain NULL) must be set to
 * one of the algorithms supported by the protocol. The authentication
 * algorithm must also be set to one of the algorithms supported by
 * the protocol..Any required keys have to be specified as well.
 *
 * Both the SAParams_p and SAParamsSSLTLS_p input parameters must point
 * to valid storage where variables of the appropriate type can be
 * stored. This function initializes the link from SAParams_p to
 * SAParamsSSSLTLS_p.
 *
 * Return:
 * SAB_STATUS_OK on success
 * SAB_INVALID_PARAMETER when one of the pointer parameters is NULL
 *   or the remaining parameters have illegal values.
 */
SABuilder_Status_t
SABuilder_Init_SSLTLS(
    SABuilder_Params_t * const SAParams_p,
    SABuilder_Params_SSLTLS_t * const SAParamsSSLTLS_p,
    const uint16_t version,
    const SABuilder_Direction_t direction)
{
    int i;
#ifdef SAB_STRICT_ARGS_CHECK
    if (SAParams_p == NULL || SAParamsSSLTLS_p == NULL)
    {
        LOG_CRIT("SABuilder_Init_SSLTLS: NULL pointer parameter supplied.\n");
        return SAB_INVALID_PARAMETER;
    }

    if (version != SAB_SSL_VERSION_3_0 &&
        version != SAB_TLS_VERSION_1_0 &&
        version != SAB_TLS_VERSION_1_1 &&
        version != SAB_TLS_VERSION_1_2 &&
        version != SAB_TLS_VERSION_1_3 &&
        version != SAB_DTLS_VERSION_1_0 &&
        version != SAB_DTLS_VERSION_1_2)
    {
        LOG_CRIT("SABuilder_Init_SSLTLS: Invalid protocol version.\n");
        return SAB_INVALID_PARAMETER;
    }

    if (direction != SAB_DIRECTION_OUTBOUND &&
        direction != SAB_DIRECTION_INBOUND)
    {
        LOG_CRIT("SABuilder_Init_ESP: Invalid direction.\n");
        return SAB_INVALID_PARAMETER;
    }
#endif

    SAParams_p->protocol = SAB_PROTO_SSLTLS;
    SAParams_p->direction = direction;
    SAParams_p->ProtocolExtension_p = (void*)SAParamsSSLTLS_p;
    SAParams_p->flags = 0;
    SAParams_p->RedirectInterface = 0;

    SAParams_p->CryptoAlgo = SAB_CRYPTO_NULL;
    SAParams_p->CryptoMode = SAB_CRYPTO_MODE_CBC;
    SAParams_p->IVSrc = SAB_IV_SRC_DEFAULT;
    SAParams_p->CryptoParameter = 0;
    SAParams_p->KeyByteCount = 0;
    SAParams_p->Key_p = NULL;
    SAParams_p->IV_p = NULL;
    SAParams_p->Nonce_p = NULL;

    SAParams_p->AuthAlgo = SAB_AUTH_NULL;
    SAParams_p->AuthKey1_p = NULL;
    SAParams_p->AuthKey2_p = NULL;
    SAParams_p->AuthKey3_p = NULL;
    SAParams_p->AuthKeyByteCount = 0;

    SAParams_p->OffsetARC4StateRecord = 0;
    SAParams_p->CW0 = 0;
    SAParams_p->CW1 = 0;
    SAParams_p->OffsetDigest0 = 0;
    SAParams_p->OffsetDigest1 = 0;
    SAParams_p->OffsetSeqNum = 0;
    SAParams_p->OffsetSeqMask = 0;
    SAParams_p->OffsetIV = 0;
    SAParams_p->OffsetIJPtr = 0;
    SAParams_p->OffsetARC4State = 0;
    SAParams_p->SeqNumWord32Count = 0;
    SAParams_p->SeqMaskWord32Count = 0;
    SAParams_p->IVWord32Count = 0;

    SAParamsSSLTLS_p->SSLTLSFlags = 0;
    SAParamsSSLTLS_p->version = version;
    SAParamsSSLTLS_p->epoch = 0;
    SAParamsSSLTLS_p->SeqNum = 0;
    SAParamsSSLTLS_p->SeqNumHi = 0;
    for (i=0; i<12; i++)
        SAParamsSSLTLS_p->SeqMask[i] = 0;
    SAParamsSSLTLS_p->PadAlignment = 0;
    SAParamsSSLTLS_p->ContextRef = 0;
    SAParamsSSLTLS_p->SequenceMaskBitCount = 0;
    SAParamsSSLTLS_p->ICVByteCount = 0;
    return SAB_STATUS_OK;
}

/*----------------------------------------------------------------------------
 * SABuilder_SetSSLTLSParams
 *
 * Fill in SSL/TLS/DTLS-specific extensions into the SA.
 *
 * SAParams_p (input)
 *   The SA parameters structure from which the SA is derived.
 * SAState_p (input, output)
 *   Variables containing information about the SA being generated/
 * SABuffer_p (input, output).
 *   The buffer in which the SA is built. If NULL, no SA will be built, but
 *   state variables in SAState_p will still be updated.
 *
 * Return:
 * SAB_STATUS_OK on success
 * SAB_INVALID_PARAMETER when SAParams_p is invalid, or if any of
 *    the buffer arguments  is a null pointer while the corresponding buffer
 *    would be required for the operation.
 * SAB_UNSUPPORTED_FEATURE when SAParams_p describes an operations that
 *    is not supported on the hardware for which this SA builder
 *    is configured.
 */
SABuilder_Status_t
SABuilder_SetSSLTLSParams(SABuilder_Params_t *const SAParams_p,
                          SABuilder_State_t * const SAState_p,
                          uint32_t * const SABuffer_p)
{
    unsigned int IVOffset = 0;
    SABuilder_Params_SSLTLS_t *SAParamsSSLTLS_p;
    bool fFixedSeqOffset = false;
    SAParamsSSLTLS_p = (SABuilder_Params_SSLTLS_t *)
        (SAParams_p->ProtocolExtension_p);
    if (SAParamsSSLTLS_p == NULL)
    {
        LOG_CRIT("SABuilder: SSLTLS extension pointer is null\n");
        return SAB_INVALID_PARAMETER;
    }

    /* Prohibit ARC4 in DTLS and TLS1.3 */
    if (SAParams_p->CryptoAlgo == SAB_CRYPTO_ARCFOUR &&
        (SAParamsSSLTLS_p->version == SAB_TLS_VERSION_1_3||
         SAParamsSSLTLS_p->version == SAB_DTLS_VERSION_1_0||
         SAParamsSSLTLS_p->version == SAB_DTLS_VERSION_1_2))
    {
        LOG_CRIT("SABuilder: ARC4 not allowed with DTLS/TLS1.3\n");
        return SAB_INVALID_PARAMETER;
    }

    /* Prohibit CBC mode or NULL crypto in TLS1.3 */
    if (SAParamsSSLTLS_p->version == SAB_TLS_VERSION_1_3 &&
        (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CBC ||
         SAParams_p->CryptoAlgo == SAB_CRYPTO_NULL))
    {
        LOG_CRIT("SABuilder: CBC or nullcrypto  not allowed with TLS1.3\n");
        return SAB_INVALID_PARAMETER;
    }

    /* AES GCM/CCM/Chacha20 is only allowed with TLS1.2/TLS1.3/DTLS1.2 */
    if ((SAParams_p->CryptoAlgo == SAB_CRYPTO_CHACHA20 ||
         SAParams_p->CryptoMode == SAB_CRYPTO_MODE_GCM||
         SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CCM) &&
        SAParamsSSLTLS_p->version != SAB_TLS_VERSION_1_2 &&
        SAParamsSSLTLS_p->version != SAB_TLS_VERSION_1_3 &&
        SAParamsSSLTLS_p->version != SAB_DTLS_VERSION_1_2)
    {
        LOG_CRIT("SABuilder: AES-GCM/CCM/ChaCha20 only allowed with TLS/DTLS 1.2 and 1.3\n");
        return SAB_INVALID_PARAMETER;
    }

    /* Prohibit stateless ARC4*/
    if (SAParams_p->CryptoAlgo == SAB_CRYPTO_ARCFOUR &&
        SAParams_p->CryptoMode != SAB_CRYPTO_MODE_STATEFUL)
    {
        LOG_CRIT("SABuilder: ARC4 must be stateful\n");
        return SAB_INVALID_PARAMETER;
    }

    /* Check for supported algorithms and crypto modes in SSL/TLS */
    if ((SAParams_p->CryptoAlgo != SAB_CRYPTO_NULL &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_ARCFOUR &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_DES &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_3DES &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_AES &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_CHACHA20 &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_SM4 &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_BC0) ||
        (SAParams_p->CryptoAlgo != SAB_CRYPTO_NULL &&
         SAParams_p->CryptoMode != SAB_CRYPTO_MODE_STATEFUL &&
         SAParams_p->CryptoMode != SAB_CRYPTO_MODE_CBC &&
         SAParams_p->CryptoMode != SAB_CRYPTO_MODE_GCM &&
         SAParams_p->CryptoMode != SAB_CRYPTO_MODE_CCM &&
         SAParams_p->CryptoMode != SAB_CRYPTO_MODE_CHACHA_CTR32))
    {
        LOG_CRIT("SABuilder: SSLTLS crypto algorithm/mode not supported\n");
        return SAB_INVALID_PARAMETER;
    }

    /* Check for supported authentication algorithms in SSL/TLS */
    if (SAParams_p->AuthAlgo != SAB_AUTH_HMAC_MD5 &&
        SAParams_p->AuthAlgo != SAB_AUTH_SSLMAC_MD5 &&
        SAParams_p->AuthAlgo != SAB_AUTH_HMAC_SHA1 &&
        SAParams_p->AuthAlgo != SAB_AUTH_SSLMAC_SHA1 &&
        SAParams_p->AuthAlgo != SAB_AUTH_HMAC_SHA2_256 &&
        SAParams_p->AuthAlgo != SAB_AUTH_HMAC_SHA2_384 &&
        SAParams_p->AuthAlgo != SAB_AUTH_HMAC_SHA2_512 &&
        SAParams_p->AuthAlgo != SAB_AUTH_AES_GCM &&
        SAParams_p->AuthAlgo != SAB_AUTH_AES_CCM &&
        SAParams_p->AuthAlgo != SAB_AUTH_POLY1305 &&
        SAParams_p->AuthAlgo != SAB_AUTH_HMAC_SM3)
    {
        LOG_CRIT("SABuilder: SSLTLS: auth algorithm not supported\n");
        return SAB_INVALID_PARAMETER;
    }

    /* Add version to SA record */
    if (SABuffer_p != NULL)
    {
        if (SAParamsSSLTLS_p->version == SAB_TLS_VERSION_1_3)
        {
            SABuffer_p[SAState_p->CurrentOffset] = SAB_TLS_VERSION_1_2<<16;
            /* fixed type & version to be put in record for TLS1.3 */
        }
        else
        {
            SABuffer_p[SAState_p->CurrentOffset] = SAParamsSSLTLS_p->version<<16;
        }
    }
    SAState_p->CurrentOffset += 1;

    /* Determine whether we will have a fixed sequence number offset */
    if (SAParams_p->direction == SAB_DIRECTION_INBOUND &&
        (SAParamsSSLTLS_p->version == SAB_DTLS_VERSION_1_0||
         SAParamsSSLTLS_p->version == SAB_DTLS_VERSION_1_2))
    {
        /* Determine size of sequence number mask in bits */
        if (SAParamsSSLTLS_p->SequenceMaskBitCount == 0)
        {
            if ((SAParamsSSLTLS_p->SSLTLSFlags & SAB_DTLS_MASK_128) != 0)
            {
                SAParamsSSLTLS_p->SequenceMaskBitCount = 128;
            }
            else if ((SAParamsSSLTLS_p->SSLTLSFlags & SAB_DTLS_MASK_32) != 0)
            {
                SAParamsSSLTLS_p->SequenceMaskBitCount = 32;
            }
            else
            {
                SAParamsSSLTLS_p->SequenceMaskBitCount = 64;
            }
        }
        if (SAParamsSSLTLS_p->SequenceMaskBitCount > SAB_SEQUENCE_MAXBITS ||
            (SAParamsSSLTLS_p->SequenceMaskBitCount & 0x1f) != 0)
        {
            LOG_CRIT("SABuilder: Illegal sequence mask size.\n");
            return SAB_INVALID_PARAMETER;
        }
#ifdef SAB_ENABLE_DEFAULT_FIXED_OFFSETS
        fFixedSeqOffset = true;
#else
        if (SAParamsSSLTLS_p->SequenceMaskBitCount > 128 ||
            (SAParamsSSLTLS_p->SSLTLSFlags & SAB_DTLS_FIXED_SEQ_OFFSET) != 0)
        {
            fFixedSeqOffset = true;
        }
#endif
        if (SAParamsSSLTLS_p->SequenceMaskBitCount == 32)
        {
            fFixedSeqOffset = false; /* not supported for 32-bit mask */
        }
    }


    if (fFixedSeqOffset)
    {
        /* Use a fixed sequence number offset for inbound if the hardware
           supports it. */
        /* Take care to insert the IV (nonce) just after the SPI. */
        IVOffset = SAState_p->CurrentOffset;

        /* Select one of two fixed offsets for the sequence number */
        if (SAState_p->CurrentOffset < SAB_SEQNUM_LO_FIX_OFFSET)
        {
            SAState_p->CurrentOffset = SAB_SEQNUM_LO_FIX_OFFSET;
        }
        else
        {
            SAState_p->CurrentOffset = SAB_SEQNUM_HI_FIX_OFFSET;
        }

        /* Add sequence number */
        SAParams_p->OffsetSeqNum = SAState_p->CurrentOffset;
        SAParams_p->SeqNumWord32Count = 2;
        SAState_p->CW1 |= SAB_CW1_SEQNUM_STORE;
        SAState_p->CW0 |= SAB_CW0_SPI | SAB_CW0_SEQNUM_48_FIX;
        if (SABuffer_p != NULL)
        {
            SABuffer_p[SAState_p->CurrentOffset] = SAParamsSSLTLS_p->SeqNum;
            SABuffer_p[SAState_p->CurrentOffset + 1] =
                    (SAParamsSSLTLS_p->SeqNumHi & 0xffff) |
                    (SAParamsSSLTLS_p->epoch << 16);
        }
        SAState_p->CurrentOffset += 2;
    }
    else
    {
        /* Add sequence number */
        SAParams_p->OffsetSeqNum = SAState_p->CurrentOffset;
        SAParams_p->SeqNumWord32Count = 2;
        SAState_p->CW1 |= SAB_CW1_SEQNUM_STORE;

        if (SABuffer_p != NULL)
            SABuffer_p[SAState_p->CurrentOffset] = SAParamsSSLTLS_p->SeqNum;
        SAState_p->CurrentOffset += 1;
        if (SAParamsSSLTLS_p->version != SAB_DTLS_VERSION_1_0 &&
            SAParamsSSLTLS_p->version != SAB_DTLS_VERSION_1_2)
        {
            SAState_p->CW0 |= SAB_CW0_SPI | SAB_CW0_SEQNUM_64;
            if (SABuffer_p != NULL)
                SABuffer_p[SAState_p->CurrentOffset] =
                    SAParamsSSLTLS_p->SeqNumHi;
        }
        else
        {
            SAState_p->CW0 |= SAB_CW0_SPI | SAB_CW0_SEQNUM_48;
            if (SABuffer_p != NULL)
                SABuffer_p[SAState_p->CurrentOffset] =
                    (SAParamsSSLTLS_p->SeqNumHi & 0xffff) |
                    (SAParamsSSLTLS_p->epoch << 16);
        }
        SAState_p->CurrentOffset += 1;
    }

    if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
    {

        if (SAParams_p->CryptoAlgo==SAB_CRYPTO_NULL)
            SAState_p->CW0 |= SAB_CW0_TOP_HASH_OUT;
        else if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_GCM||
                 SAParams_p->CryptoAlgo == SAB_CRYPTO_CHACHA20)
            SAState_p->CW0 |= SAB_CW0_TOP_ENCRYPT_HASH;
        else
            SAState_p->CW0 |= SAB_CW0_TOP_HASH_ENCRYPT;

        /* Some versions of the hardware can update the sequence number
           early, so multiple engines can operate in parallel. */
        SAState_p->CW1 |= SAB_CW1_EARLY_SEQNUM_UPDATE;
        SAState_p->CW1 |= SAParams_p->OffsetSeqNum << 24;

        /* Take care of IV  */
        if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CCM)
        {
            if (SAParamsSSLTLS_p->version == SAB_TLS_VERSION_1_3)
            {
                SAState_p->CW1 = (SAState_p->CW1 & ~SAB_CW1_IV_MODE_MASK) |
                    SAB_CW1_IV_ORIG_SEQ|SAB_CW1_CRYPTO_NONCE_XOR;
                SAState_p->CW1 |= SAB_CW1_IV0|SAB_CW1_IV1|SAB_CW1_IV2|
                    SAB_CW1_IV3 | SAB_CW1_CCM_IV_SHIFT;
                if (SABuffer_p != NULL)
                {
                    SABuilderLib_CopyKeyMat(SABuffer_p,
                                            SAState_p->CurrentOffset,
                                            SAParams_p->Nonce_p,
                                            3 * sizeof(uint32_t));
                    SABuffer_p[SAState_p->CurrentOffset + 3] =
                        SAB_CCM_FLAG_L3 << 24;
                }
                SAState_p->CurrentOffset += 4;
            }
            else
            {
                SAState_p->CW1 = (SAState_p->CW1 & ~SAB_CW1_IV_MODE_MASK) |
                    SAB_CW1_IV_ORIG_SEQ;
                SAState_p->CW1 |= SAB_CW1_IV0|SAB_CW1_IV3 | SAB_CW1_CCM_IV_SHIFT;
                if (SABuffer_p != NULL)
                {
                    SABuilderLib_CopyKeyMat(SABuffer_p,
                                            SAState_p->CurrentOffset,
                                            SAParams_p->Nonce_p,
                                            sizeof(uint32_t));
                    SABuffer_p[SAState_p->CurrentOffset + 1] =
                        SAB_CCM_FLAG_L3 << 24;
                }
                SAState_p->CurrentOffset += 2;
            }
        }
        else if (SAParamsSSLTLS_p->version == SAB_TLS_VERSION_1_3 ||
                 SAParams_p->CryptoAlgo == SAB_CRYPTO_CHACHA20)
        {
            SAState_p->CW1 = (SAState_p->CW1 & ~SAB_CW1_IV_MODE_MASK) |
                SAB_CW1_IV_ORIG_SEQ|SAB_CW1_CRYPTO_NONCE_XOR;
            SAState_p->CW1 |= SAB_CW1_IV0|SAB_CW1_IV1|SAB_CW1_IV2;
            /* Always store the nonce (implicit salt) with TLS1.3 */
            SABuilderLib_CopyKeyMat(SABuffer_p,
                                    SAState_p->CurrentOffset,
                                    SAParams_p->Nonce_p, 3*sizeof(uint32_t));
            SAState_p->CurrentOffset +=3;
        }
        else if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_GCM)
        {
            if (SAParams_p->IVSrc == SAB_IV_SRC_DEFAULT)
                SAParams_p->IVSrc = SAB_IV_SRC_SEQ;

            if (SAParams_p->IVSrc == SAB_IV_SRC_SEQ)
            {
                SAState_p->CW1 |= SAB_CW1_IV_ORIG_SEQ;
            }
            else if (SAParams_p->IVSrc == SAB_IV_SRC_XORSEQ)
            {
                SAState_p->CW1 |= SAB_CW1_IV_CTR|SAB_CW1_CRYPTO_NONCE_XOR;
                SAState_p->CW1 |= SAB_CW1_IV0|SAB_CW1_IV1|SAB_CW1_IV2;
            }
            else if (SAParams_p->IVSrc == SAB_IV_SRC_SA)
            {
                SAState_p->CW1 |= SAB_CW1_IV_CTR | SAB_CW1_IV1 | SAB_CW1_IV2;
#ifdef SAB_STRICT_ARGS_CHECK
                if (SAParams_p->IV_p == NULL)
                {
                    LOG_CRIT("SABuilder: NULL pointer IV.\n");
                    return SAB_INVALID_PARAMETER;
                }
#endif
                SAParams_p->OffsetIV = SAState_p->CurrentOffset;
                SAParams_p->IVWord32Count = 2;

                SABuilderLib_CopyKeyMat(SABuffer_p, SAState_p->CurrentOffset,
                                        SAParams_p->IV_p, 8);
                SAState_p->CurrentOffset += 2;
            }
            else
            {
                SAState_p->CW1 |= SAB_CW1_IV_CTR;
            }
            SAState_p->CW1 |= SAB_CW1_IV0;
            /* Always store the nonce (implicit salt) with AES-GCM */
            SABuilderLib_CopyKeyMat(SABuffer_p,
                                    SAState_p->CurrentOffset,
                                    SAParams_p->Nonce_p,
                                    sizeof(uint32_t));
            SAState_p->CurrentOffset +=1;
            if (SAParams_p->IVSrc == SAB_IV_SRC_XORSEQ)
            {
                /* Store a fixed 8-byte value to XOR with sequence number */
                SABuilderLib_CopyKeyMat(SABuffer_p,
                                    SAState_p->CurrentOffset,
                                    SAParams_p->IV_p,
                                        2*sizeof(uint32_t));
                SAState_p->CurrentOffset +=2;
            }
        }
        else if (SAState_p->IVWords > 0)
        { /* CBC mode, non-null */
            if (SAParamsSSLTLS_p->version == SAB_SSL_VERSION_3_0 ||
                SAParamsSSLTLS_p->version == SAB_TLS_VERSION_1_0)
            {
                SAParams_p->IVSrc = SAB_IV_SRC_SA;
                SAState_p->CW1 |= SAB_CW1_CRYPTO_STORE;
            }
            else if (SAParams_p->IVSrc == SAB_IV_SRC_DEFAULT)
            {
                SAParams_p->IVSrc = SAB_IV_SRC_PRNG;
            }
            SAState_p->CW1 |= SAB_CW1_IV_FULL;
            if (SAParams_p->IVSrc == SAB_IV_SRC_SA)
            {
                SAState_p->CW1 |= SAB_CW1_IV0 | SAB_CW1_IV1;
                if(SAState_p->IVWords == 4)
                    SAState_p->CW1 |= SAB_CW1_IV2 | SAB_CW1_IV3;
#ifdef SAB_STRICT_ARGS_CHECK
                if (SAParams_p->IV_p == NULL)
                {
                    LOG_CRIT("SABuilder: NULL pointer IV.\n");
                    return SAB_INVALID_PARAMETER;
                }
#endif
                SAParams_p->OffsetIV = SAState_p->CurrentOffset;
                SAParams_p->IVWord32Count = SAState_p->IVWords;

                SABuilderLib_CopyKeyMat(SABuffer_p,
                                        SAState_p->CurrentOffset,
                                        SAParams_p->IV_p,
                                        SAState_p->IVWords * sizeof(uint32_t));
                SAState_p->CurrentOffset += SAState_p->IVWords;
            }
        }
    }
    else
    {   /* Inbound */
        if (SAParams_p->CryptoAlgo==SAB_CRYPTO_NULL)
            SAState_p->CW0 |= SAB_CW0_TOP_HASH_IN;
        else if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_GCM ||
                 SAParams_p->CryptoAlgo == SAB_CRYPTO_CHACHA20)
            SAState_p->CW0 |= SAB_CW0_TOP_HASH_DECRYPT;
        else
            SAState_p->CW0 |= SAB_CW0_TOP_DECRYPT_HASH;

        if (SAParams_p->CryptoMode != SAB_CRYPTO_MODE_GCM &&
            SAParams_p->CryptoMode != SAB_CRYPTO_MODE_CCM &&
            SAParams_p->CryptoAlgo != SAB_CRYPTO_CHACHA20  &&
            SAState_p->IVWords > 0)
        {
            SAState_p->CW1 |= SAB_CW1_PREPKT_OP;
            if (SAParamsSSLTLS_p->version == SAB_SSL_VERSION_3_0)
            {
                SAState_p->CW1 |= SAB_CW1_PAD_SSL;
            }
            else
            {
                SAState_p->CW1 |= SAB_CW1_PAD_TLS;
            }
        }
        if (SAParamsSSLTLS_p->version == SAB_TLS_VERSION_1_3)
        {
            SAState_p->CW1 |= SAB_CW1_PAD_TLS | SAB_CW1_CRYPTO_AEAD;
        }

        /* Add sequence mask for DTLS only. */
        if ((SAParamsSSLTLS_p->version == SAB_DTLS_VERSION_1_0 ||
             SAParamsSSLTLS_p->version == SAB_DTLS_VERSION_1_2))
        {
            unsigned int InputMaskWordCount =
                SAParamsSSLTLS_p->SequenceMaskBitCount / 32;
            unsigned int AllocMaskWordCount;
            SAParams_p->OffsetSeqMask = SAState_p->CurrentOffset;
            /* Determine the required hardware mask size in words and set
               control words accordingly. */
            if (InputMaskWordCount  == 1)
            {
                AllocMaskWordCount = 1;
                SAState_p->CW0 |= SAB_CW0_MASK_32;
            }
            else if (InputMaskWordCount  == 2)
            {
                AllocMaskWordCount = 2;
                if (fFixedSeqOffset)
                {
                    SAState_p->CW0 |= SAB_CW0_MASK_64_FIX;
                }
                else
                {
                    SAState_p->CW0 |= SAB_CW0_MASK_64;
                }
            }
            else if (InputMaskWordCount <= 4)
            {
                AllocMaskWordCount = 4;
                if (fFixedSeqOffset)
                {
                SAState_p->CW0 |= SAB_CW0_MASK_128_FIX;
                }
                else
                {
                SAState_p->CW0 |= SAB_CW0_MASK_128;
                }
            }
#ifdef SAB_ENABLE_256BIT_SEQMASK
            else if (InputMaskWordCount <= 8)
            {
                AllocMaskWordCount = 8;
                SAState_p->CW0 |= SAB_CW0_MASK_256_FIX;
            }
#endif
            else
            {
                AllocMaskWordCount = 12;
                SAState_p->CW0 |= SAB_CW0_MASK_384_FIX;
            }
            if(SABuffer_p != NULL)
            {
                unsigned int i;
                for (i = 0; i < InputMaskWordCount; i++)
                    SABuffer_p[SAState_p->CurrentOffset+i] =
                        SAParamsSSLTLS_p->SeqMask[i];
                /* If the input mask is smaller than the one picked by the
                   hardware, fill the remaining words with all-one, the
                   hardware will treat these words as invalid.
                */
                for (i= InputMaskWordCount; i < AllocMaskWordCount; i++)
                    SABuffer_p[SAState_p->CurrentOffset+i] = 0xffffffff;
            }
            SAState_p->CurrentOffset += AllocMaskWordCount;
            SAParams_p->SeqMaskWord32Count = InputMaskWordCount;
        }

        if (IVOffset == 0)
            IVOffset = SAState_p->CurrentOffset;
        if (SAState_p->IVWords > 0 &&
            (SAParamsSSLTLS_p->version == SAB_SSL_VERSION_3_0 ||
             SAParamsSSLTLS_p->version == SAB_TLS_VERSION_1_0))
        {
            SAParams_p->IVSrc = SAB_IV_SRC_SA;
            SAState_p->CW1 |= SAB_CW1_IV_FULL;

            SAState_p->CW1 |= SAB_CW1_IV0 | SAB_CW1_IV1 |
                SAB_CW1_CRYPTO_STORE;
            if(SAState_p->IVWords == 4)
                    SAState_p->CW1 |= SAB_CW1_IV2 | SAB_CW1_IV3;
#ifdef SAB_STRICT_ARGS_CHECK
            if (SAParams_p->IV_p == NULL)
            {
                LOG_CRIT("SABuilder: NULL pointer IV.\n");
                    return SAB_INVALID_PARAMETER;
            }
#endif
            SAParams_p->OffsetIV = IVOffset;
            SAParams_p->IVWord32Count = SAState_p->IVWords;

            SABuilderLib_CopyKeyMat(SABuffer_p,
                                    IVOffset,
                                    SAParams_p->IV_p,
                                    SAState_p->IVWords * sizeof(uint32_t));
            IVOffset += SAState_p->IVWords;
        }

        if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CCM)
        {
            if (SAParamsSSLTLS_p->version == SAB_TLS_VERSION_1_3)
            {
                SAState_p->CW1 = (SAState_p->CW1 & ~SAB_CW1_IV_MODE_MASK) |
                    SAB_CW1_IV_ORIG_SEQ|SAB_CW1_CRYPTO_NONCE_XOR;
                SAState_p->CW1 |= SAB_CW1_IV0|SAB_CW1_IV1|SAB_CW1_IV2|
                    SAB_CW1_IV3 | SAB_CW1_CCM_IV_SHIFT;
                if (SABuffer_p != NULL)
                {
                    SABuilderLib_CopyKeyMat(SABuffer_p,
                                            SAState_p->CurrentOffset,
                                            SAParams_p->Nonce_p,
                                            3 * sizeof(uint32_t));
                    SABuffer_p[SAState_p->CurrentOffset + 3] =
                        SAB_CCM_FLAG_L3 << 24;
                }
                SAState_p->CurrentOffset += 4;
            }
            else
            {
                SAState_p->CW1 = (SAState_p->CW1 & ~SAB_CW1_IV_MODE_MASK);
                SAState_p->CW1 |= SAB_CW1_IV0| SAB_CW1_IV3 | SAB_CW1_CCM_IV_SHIFT;
                if (SABuffer_p != NULL)
                {
                    SABuilderLib_CopyKeyMat(SABuffer_p,
                                            SAState_p->CurrentOffset,
                                            SAParams_p->Nonce_p,
                                            sizeof(uint32_t));
                    SABuffer_p[SAState_p->CurrentOffset + 1] =
                        SAB_CCM_FLAG_L3 << 24;
                }
                SAState_p->CurrentOffset += 2;
            }
        }
        else if (SAParamsSSLTLS_p->version == SAB_TLS_VERSION_1_3 ||
            SAParams_p->CryptoAlgo == SAB_CRYPTO_CHACHA20)
        {

            if (SAParamsSSLTLS_p->version == SAB_DTLS_VERSION_1_2)
            {
                /* DTLS inbound extracts the sequence number from the packet, use delayed OTK mode */
                SAState_p->CW1 |= SAB_CW1_IV_CTR|SAB_CW1_CRYPTO_NONCE_XOR;
            } else {
                /* regular TLS inbound internally increments the sequence number, so can save some cycles by starting OTK calc early */
                SAState_p->CW1 = (SAState_p->CW1 & ~SAB_CW1_IV_MODE_MASK) |
                    SAB_CW1_IV_ORIG_SEQ|SAB_CW1_CRYPTO_NONCE_XOR;
            }
            SAState_p->CW1 |= SAB_CW1_IV0|SAB_CW1_IV1|SAB_CW1_IV2;
            /* Always store the nonce (implicit salt) with TLS1.3 */
            SABuilderLib_CopyKeyMat(SABuffer_p,
                                    IVOffset,
                                    SAParams_p->Nonce_p, 3*sizeof(uint32_t));
            IVOffset +=3;
        }
        else if(SAParams_p->CryptoMode == SAB_CRYPTO_MODE_GCM)
        {
            SAState_p->CW1 |= SAB_CW1_IV_CTR|SAB_CW1_IV0;
            /* Always store the nonce (implicit salt) with AES-GCM */
            SABuilderLib_CopyKeyMat(SABuffer_p,
                                    IVOffset,
                                    SAParams_p->Nonce_p, sizeof(uint32_t));
            IVOffset +=1;
        }
        if (IVOffset > SAState_p->CurrentOffset)
        {
            SAState_p->CurrentOffset = IVOffset;
        }

    }

    return SAB_STATUS_OK;
}


#endif /* SAB_ENABLE_PROTO_SSLTLS */

/* end of file sa_builder_ssltls.c */
