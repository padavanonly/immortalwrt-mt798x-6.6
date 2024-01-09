/* sa_builder_ipsec.c
 *
 * IPsec specific functions (for initialization of SABuilder_Params_t
 * structures and for building the IPSec specifc part of an SA.).
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
#include "sa_builder_ipsec.h"
#include "sa_builder_internal.h" /* SABuilder_SetIpsecParams */

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_sa_builder.h"
#include "basic_defs.h"
#include "log.h"

#ifdef SAB_ENABLE_PROTO_IPSEC


/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#ifdef SAB_ENABLE_1024BIT_SEQMASK
#define SAB_SEQUENCE_MAXBITS 1024
#elif defined(SAB_ENABLE_384BIT_SEQMASK)
#define SAB_SEQUENCE_MAXBITS 384
#else
#define SAB_SEQUENCE_MAXBITS 128
#endif

/*----------------------------------------------------------------------------
 * Local variables
 */

/*----------------------------------------------------------------------------
 * SABuilder_Init_ESP
 *
 * This function initializes the SABuilder_Params_t data structure and its
 * SABuilder_Params_IPsec_t extension with sensible defaults for ESP
 * processing.
 *
 * SAParams_p (output)
 *   Pointer to SA parameter structure to be filled in.
 * SAParamsIPsec_p (output)
 *   Pointer to IPsec parameter extension to be filled in
 * spi (input)
 *   SPI of the newly created parameter structure (must not be zero).
 * TunnelTransport (input)
 *   Must be one of SAB_IPSEC_TUNNEL or SAB_IPSEC_TRANSPORT.
 * IPMode (input)
 *   Must be one of SAB_IPSEC_IPV4 or SAB_IPSEC_IPV6.
 * direction (input)
 *   Must be one of SAB_DIRECTION_INBOUND or SAB_DIRECTION_OUTBOUND.
 *
 * Both the crypto and the authentication algorithm are initialized to
 * NULL, which is illegal according to the IPsec standards, but it is
 * possible to use this setting for debug purposes.
 *
 * Both the SAParams_p and SAParamsIPsec_p input parameters must point
 * to valid storage where variables of the appropriate type can be
 * stored. This function initializes the link from SAParams_p to
 * SAParamsIPsec_p.
 *
 * Return:
 * SAB_STATUS_OK on success
 * SAB_INVALID_PARAMETER when one of the pointer parameters is NULL
 *   or the remaining parameters have illegal values.
 */
SABuilder_Status_t
SABuilder_Init_ESP(
    SABuilder_Params_t * const SAParams_p,
    SABuilder_Params_IPsec_t * const SAParamsIPsec_p,
    const uint32_t spi,
    const uint32_t TunnelTransport,
    const uint32_t IPMode,
    const SABuilder_Direction_t direction)
{
    int i;
#ifdef SAB_STRICT_ARGS_CHECK
    if (SAParams_p == NULL || SAParamsIPsec_p == NULL)
    {
        LOG_CRIT("SABuilder_Init_ESP: NULL pointer parameter supplied.\n");
        return SAB_INVALID_PARAMETER;
    }

    if (spi == 0)
    {
        LOG_CRIT("SABuilder_Init_ESP: SPI may not be 0.\n");
        return SAB_INVALID_PARAMETER;
    }

    if (TunnelTransport != SAB_IPSEC_TUNNEL &&
        TunnelTransport != SAB_IPSEC_TRANSPORT)
    {
        LOG_CRIT("SABuilder_Init_ESP: Invalid TunnelTransport.\n");
        return SAB_INVALID_PARAMETER;
    }

    if (IPMode != SAB_IPSEC_IPV4 && IPMode != SAB_IPSEC_IPV6)
    {
        LOG_CRIT("SABuilder_Init_ESP: Invalid IPMode.\n");
        return SAB_INVALID_PARAMETER;
    }

    if (direction != SAB_DIRECTION_OUTBOUND &&
        direction != SAB_DIRECTION_INBOUND)
    {
        LOG_CRIT("SABuilder_Init_ESP: Invalid direction.\n");
        return SAB_INVALID_PARAMETER;
    }
#endif

    SAParams_p->protocol = SAB_PROTO_IPSEC;
    SAParams_p->direction = direction;
    SAParams_p->ProtocolExtension_p = (void*)SAParamsIPsec_p;
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

    SAParamsIPsec_p->spi = spi;
    SAParamsIPsec_p->IPsecFlags = SAB_IPSEC_ESP | TunnelTransport | IPMode;
    SAParamsIPsec_p->SeqNum = 0;
    SAParamsIPsec_p->SeqNumHi = 0;
    SAParamsIPsec_p->SeqMask[0] = 1;
    for (i=1; i<SA_SEQ_MASK_WORD_COUNT; i++)
        SAParamsIPsec_p->SeqMask[i] = 0;
    SAParamsIPsec_p->PadAlignment = 0;
    SAParamsIPsec_p->ICVByteCount = 0;
    SAParamsIPsec_p->SrcIPAddr_p = NULL;
    SAParamsIPsec_p->DestIPAddr_p = NULL;
    SAParamsIPsec_p->OrigSrcIPAddr_p = NULL;
    SAParamsIPsec_p->OrigDestIPAddr_p = NULL;
    SAParamsIPsec_p->NATTSrcPort = 4500;
    SAParamsIPsec_p->NATTDestPort = 4500;
    SAParamsIPsec_p->ContextRef = 0;
    SAParamsIPsec_p->TTL = 240;
    SAParamsIPsec_p->DSCP = 0;
    SAParamsIPsec_p->SequenceMaskBitCount = 0;
    return SAB_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SABuilder_SetIPsecParams
 *
 * Fill in IPsec-specific extensions into the SA.
 *
 * SAParams_p (input, updated)
 *   The SA parameters structure from which the SA is derived.
 * SAState_p (input, output)
 *   Variables containing information about the SA being generated/
 * SABuffer_p (output).
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
SABuilder_SetIPsecParams(SABuilder_Params_t *const SAParams_p,
                         SABuilder_State_t * const SAState_p,
                         uint32_t * const SABuffer_p)
{
    unsigned int IVOffset = 0;
    SABuilder_Params_IPsec_t *SAParamsIPsec_p;
    bool fFixedSeqOffset = false;
    SAParamsIPsec_p = (SABuilder_Params_IPsec_t *)
        (SAParams_p->ProtocolExtension_p);
    if (SAParamsIPsec_p == NULL)
    {
        LOG_CRIT("SABuilder: IPsec extension pointer is null\n");
        return SAB_INVALID_PARAMETER;
    }

    /* First check whether AH or ESP flags are correct */

    if ( (SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_AH) != 0)
    {
#ifdef SAB_ENABLE_IPSEC_AH
        if (SAParams_p->CryptoAlgo != SAB_CRYPTO_NULL ||
            (SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_ESP) != 0)
        {
            LOG_CRIT("SABuilder: AH does not support crypto.\n");
            return SAB_INVALID_PARAMETER;
        }
#else
        LOG_CRIT("SABuilder: AH unsupported..\n");
        return SAB_INVALID_PARAMETER;
#endif
    }

#ifndef SAB_ENABLE_IPSEC_ESP
    if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_ESP) != 0)
    {
        LOG_CRIT("SABuilder: ESP unsupported.\n");
        return SAB_INVALID_PARAMETER;
    }
#endif

    /* Check for supported algorithms and crypto modes in IPsec */
    if ((SAParams_p->CryptoAlgo != SAB_CRYPTO_NULL &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_DES &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_3DES &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_AES &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_CHACHA20 &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_SM4 &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_BC0) ||
        (SAParams_p->CryptoAlgo != SAB_CRYPTO_NULL && (
            SAParams_p->CryptoMode != SAB_CRYPTO_MODE_CBC &&
            SAParams_p->CryptoMode != SAB_CRYPTO_MODE_CTR &&
            SAParams_p->CryptoMode != SAB_CRYPTO_MODE_GCM &&
            SAParams_p->CryptoMode != SAB_CRYPTO_MODE_GMAC &&
            SAParams_p->CryptoMode != SAB_CRYPTO_MODE_CCM &&
            SAParams_p->CryptoMode != SAB_CRYPTO_MODE_CHACHA_CTR32)))
    {
        LOG_CRIT("SABuilder: IPsec: crypto algorithm/mode not supported\n");
        return SAB_INVALID_PARAMETER;
    }

    /* Check for supported authentication algorithms in IPsec */
    if (SAParams_p->AuthAlgo != SAB_AUTH_NULL &&
        SAParams_p->AuthAlgo != SAB_AUTH_HMAC_MD5 &&
        SAParams_p->AuthAlgo != SAB_AUTH_HMAC_SHA1 &&
        SAParams_p->AuthAlgo != SAB_AUTH_HMAC_SHA2_256 &&
        SAParams_p->AuthAlgo != SAB_AUTH_HMAC_SHA2_384 &&
        SAParams_p->AuthAlgo != SAB_AUTH_HMAC_SHA2_512 &&
        SAParams_p->AuthAlgo != SAB_AUTH_AES_XCBC_MAC &&
        SAParams_p->AuthAlgo != SAB_AUTH_AES_CMAC_128 &&
        SAParams_p->AuthAlgo != SAB_AUTH_AES_GCM &&
        SAParams_p->AuthAlgo != SAB_AUTH_AES_GMAC &&
        SAParams_p->AuthAlgo != SAB_AUTH_AES_CCM &&
        SAParams_p->AuthAlgo != SAB_AUTH_POLY1305 &&
        SAParams_p->AuthAlgo != SAB_AUTH_HMAC_SM3)
    {
        LOG_CRIT("SABuilder: IPsec: auth algorithm not supported\n");
        return SAB_INVALID_PARAMETER;
    }

    /* Add SPI to SA record */
    if (SABuffer_p != NULL)
        SABuffer_p[SAState_p->CurrentOffset] = SAParamsIPsec_p->spi;
    SAState_p->CurrentOffset += 1;

    /* Determine whether we will have a fixed sequence number offset */
    if (SAParams_p->direction == SAB_DIRECTION_INBOUND)
    {
        /* Determine size of sequence number mask in bits */
        if (SAParamsIPsec_p->SequenceMaskBitCount == 0)
        {
            /* Some flags indicate specific mask sizes */
            if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_MASK_384) != 0)
            {
                SAParamsIPsec_p->SequenceMaskBitCount = 384;
            }
            else if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_MASK_256) != 0)
            {
                SAParamsIPsec_p->SequenceMaskBitCount = 256;
            }
            else if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_MASK_128) != 0)
            {
                SAParamsIPsec_p->SequenceMaskBitCount = 128;
            }
            else if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_MASK_32) != 0)
            {
                SAParamsIPsec_p->SequenceMaskBitCount = 32;
            }
            else
            {
                SAParamsIPsec_p->SequenceMaskBitCount = 64;
            }
        }
        if (SAParamsIPsec_p->SequenceMaskBitCount > SAB_SEQUENCE_MAXBITS ||
            (SAParamsIPsec_p->SequenceMaskBitCount & 0x1f) != 0)
        {
            LOG_CRIT("SABuilder: Illegal sequence mask size.\n");
            return SAB_INVALID_PARAMETER;
        }
#ifdef SAB_ENABLE_DEFAULT_FIXED_OFFSETS
        fFixedSeqOffset = true;
#else
        if (SAParamsIPsec_p->SequenceMaskBitCount > 128 ||
            (SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_FIXED_SEQ_OFFSET) != 0)
        {
            fFixedSeqOffset = true;
        }
#endif
        if (SAParamsIPsec_p->SequenceMaskBitCount ==32)
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

        if (SABuffer_p != NULL)
            SABuffer_p[SAState_p->CurrentOffset] = SAParamsIPsec_p->SeqNum;

        if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_LONG_SEQ) != 0)
        {
            if (SABuffer_p != NULL)
                SABuffer_p[SAState_p->CurrentOffset+1] = SAParamsIPsec_p->SeqNumHi;
            SAState_p->CW0 |= SAB_CW0_SPI | SAB_CW0_SEQNUM_64_FIX;
            SAParams_p->SeqNumWord32Count = 2;
        }
        else
        {
            SAState_p->CW0 |= SAB_CW0_SPI | SAB_CW0_SEQNUM_32_FIX;
            SAParams_p->SeqNumWord32Count = 1;
        }
        // Always reserve 2 words for the sequence number.
        SAState_p->CurrentOffset += 2;
    }
    else
    {
        /* Add sequence number */
        SAParams_p->OffsetSeqNum = SAState_p->CurrentOffset;

        if (SABuffer_p != NULL)
            SABuffer_p[SAState_p->CurrentOffset] = SAParamsIPsec_p->SeqNum;
        SAState_p->CurrentOffset += 1;
        if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_LONG_SEQ) != 0)
        {
            if (SABuffer_p != NULL)
                SABuffer_p[SAState_p->CurrentOffset] = SAParamsIPsec_p->SeqNumHi;
            SAState_p->CW0 |= SAB_CW0_SPI | SAB_CW0_SEQNUM_64;
            SAState_p->CurrentOffset += 1;

            SAParams_p->SeqNumWord32Count = 2;
        }
        else
        {
            SAState_p->CW0 |= SAB_CW0_SPI | SAB_CW0_SEQNUM_32;
            SAParams_p->SeqNumWord32Count = 1;
        }
    }

    if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
    {

        if (SAParams_p->CryptoAlgo==SAB_CRYPTO_NULL &&
            SAParams_p->AuthAlgo==SAB_AUTH_NULL)
            SAState_p->CW0 |= SAB_CW0_TOP_NULL_OUT;
        else if (SAParams_p->CryptoAlgo==SAB_CRYPTO_NULL)
            SAState_p->CW0 |= SAB_CW0_TOP_HASH_OUT;
        else if (SAParams_p->AuthAlgo==SAB_AUTH_NULL)
            SAState_p->CW0 |= SAB_CW0_TOP_ENCRYPT;
        else if (SAParams_p->AuthAlgo==SAB_AUTH_AES_CCM ||
                 SAParams_p->AuthAlgo==SAB_AUTH_AES_GMAC)
            SAState_p->CW0 |= SAB_CW0_TOP_HASH_ENCRYPT;
        else
            SAState_p->CW0 |= SAB_CW0_TOP_ENCRYPT_HASH;

        /* Some versions of the hardware can update the sequence number
           early, so multiple engines can operate in parallel. */
        SAState_p->CW1 |= SAB_CW1_EARLY_SEQNUM_UPDATE;
        SAState_p->CW1 |= SAParams_p->OffsetSeqNum << 24;

        SAState_p->CW1 |= SAB_CW1_SEQNUM_STORE;
        if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_NO_ANTI_REPLAY)!=0 &&
            (SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_LONG_SEQ) == 0)
        {
            /* Disable outbound sequence number rollover checking by putting
               an 64-sequence number in the SA. This will not be
               used in authentication (no ESN) */
            if (SABuffer_p != NULL)
                SABuffer_p[SAState_p->CurrentOffset] = 0;
            SAState_p->CW0 |= SAB_CW0_SEQNUM_64;
            SAState_p->CurrentOffset += 1;
        }

        /* Take care of IV and nonce */
        if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CTR ||
            SAParams_p->CryptoMode == SAB_CRYPTO_MODE_GCM ||
            SAParams_p->CryptoMode == SAB_CRYPTO_MODE_GMAC ||
            SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CCM ||
            SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CHACHA_CTR32)
        {
            if (SAParams_p->IVSrc == SAB_IV_SRC_DEFAULT)
                SAParams_p->IVSrc = SAB_IV_SRC_SEQ;

            /* Add nonce, always present */
            SAState_p->CW1 |= SAB_CW1_IV0;
            if (SABuffer_p != NULL)
            {
                if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CCM)
                    SABuffer_p[SAState_p->CurrentOffset] =
                        (SAParams_p->Nonce_p[0] << 8)  |
                        (SAParams_p->Nonce_p[1] << 16) |
                        (SAParams_p->Nonce_p[2] << 24) | SAB_CCM_FLAG_L4;
                else
                    SABuilderLib_CopyKeyMat(SABuffer_p,
                                            SAState_p->CurrentOffset,
                                            SAParams_p->Nonce_p,
                                            sizeof(uint32_t));
            }
            SAState_p->CurrentOffset +=1;

            if (SAParams_p->IVSrc == SAB_IV_SRC_SEQ)
            {
                SAState_p->CW1 |= SAB_CW1_IV_ORIG_SEQ;
            }
            else if (SAParams_p->IVSrc == SAB_IV_SRC_IMPLICIT)
            {
                SAState_p->CW1 |= SAB_CW1_IV_INCR_SEQ;
            }
            else if (SAParams_p->IVSrc == SAB_IV_SRC_XORSEQ)
            {
                SAState_p->CW1 |= SAB_CW1_IV_CTR|SAB_CW1_CRYPTO_NONCE_XOR;
                SAState_p->CW1 |= SAB_CW1_IV0|SAB_CW1_IV1|SAB_CW1_IV2;
                SABuilderLib_CopyKeyMat(SABuffer_p,
                                        SAState_p->CurrentOffset,
                                        SAParams_p->IV_p,
                                        2*sizeof(uint32_t));
                SAState_p->CurrentOffset +=2;
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
            if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CCM)
            {
                /* Add 0 counter field (IV3) */
                SAState_p->CW1 |= SAB_CW1_IV3;
                if(SABuffer_p != NULL)
                    SABuffer_p[SAState_p->CurrentOffset] = 0;
                SAState_p->CurrentOffset+=1;
            }
        }
        else if (SAState_p->IVWords > 0)
        { /* CBC mode, non-null */
            if (SAParams_p->IVSrc == SAB_IV_SRC_DEFAULT)
                SAParams_p->IVSrc = SAB_IV_SRC_PRNG;
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
        unsigned int InputMaskWordCount =
            SAParamsIPsec_p->SequenceMaskBitCount / 32;
        unsigned int AllocMaskWordCount;

        if (SAParams_p->CryptoAlgo==SAB_CRYPTO_NULL &&
            SAParams_p->AuthAlgo==SAB_AUTH_NULL)
            SAState_p->CW0 |= SAB_CW0_TOP_NULL_IN;
        else if (SAParams_p->CryptoAlgo==SAB_CRYPTO_NULL)
            SAState_p->CW0 |= SAB_CW0_TOP_HASH_IN;
        else if (SAParams_p->AuthAlgo==SAB_AUTH_NULL)
            SAState_p->CW0 |= SAB_CW0_TOP_DECRYPT;
        else if (SAParams_p->AuthAlgo==SAB_AUTH_AES_CCM)
            SAState_p->CW0 |= SAB_CW0_TOP_DECRYPT_HASH;
        else
            SAState_p->CW0 |= SAB_CW0_TOP_HASH_DECRYPT;

        SAState_p->CW1 |= SAB_CW1_PAD_IPSEC;

        /* Add sequence mask  Always add one even with no anti-replay*/
        SAParams_p->OffsetSeqMask = SAState_p->CurrentOffset;

        if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_APPEND_SEQNUM) != 0)
            SAState_p->CW0 |= SAB_CW0_SEQNUM_APPEND;

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
        else if (InputMaskWordCount <= 12)
        {
            AllocMaskWordCount = 12;
            SAState_p->CW0 |= SAB_CW0_MASK_384_FIX;
        }
        else
        {
            AllocMaskWordCount = 32;
            SAState_p->CW0 |= SAB_CW0_MASK_1024_FIX;
            SAState_p->fLargeMask = true;
            SAState_p->fLarge = true;
        }
        if(SABuffer_p != NULL)
        {
            unsigned int i;
            if (AllocMaskWordCount <= SA_SEQ_MASK_WORD_COUNT)
            {
                for (i = 0; i < InputMaskWordCount; i++)
                    SABuffer_p[SAState_p->CurrentOffset+i] =
                        SAParamsIPsec_p->SeqMask[i];
                /* If the input mask is smaller than the one picked by the
                   hardware, fill the remaining words with all-one, the
                   hardware will treat these words as invalid.
                */
                for (i= InputMaskWordCount; i < AllocMaskWordCount; i++)
                    SABuffer_p[SAState_p->CurrentOffset+i] = 0xffffffff;
            }
            else
            {
                /* Mask too big to store in parameter structure.
                   Also need to shift the '1' bit to correct position */
                uint32_t WordIdx, BitMask;
                for (i= 0; i < AllocMaskWordCount; i++)
                    SABuffer_p[SAState_p->CurrentOffset+i] = 0;
                WordIdx = (SAParamsIPsec_p->SeqNum & MASK_10_BITS) >> 5;
                BitMask = 1 << (SAParamsIPsec_p->SeqNum & MASK_5_BITS);
                SABuffer_p[SAState_p->CurrentOffset+WordIdx] = BitMask;
            }
        }
        SAState_p->CurrentOffset += AllocMaskWordCount;
        SAParams_p->SeqMaskWord32Count = InputMaskWordCount;

        /* Add nonce for CTR and related modes */
        if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CTR ||
            SAParams_p->CryptoMode == SAB_CRYPTO_MODE_GCM ||
            SAParams_p->CryptoMode == SAB_CRYPTO_MODE_GMAC ||
            SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CCM||
            SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CHACHA_CTR32)
        {
            if (IVOffset == 0)
                IVOffset = SAState_p->CurrentOffset;

            SAState_p->CW1 |= SAB_CW1_IV0;

            /* For Poly/Chacha, we need to run in XOR IV mode with
              delayed OTK in order to make the OTK derivation from the
              extracted IV work */
            if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CHACHA_CTR32)
            {
                SAState_p->CW1 |= SAB_CW1_CRYPTO_NONCE_XOR|
                    SAB_CW1_CRYPTO_MODE_CHACHA_POLY_OTK;
                /* need all 3 IV double words - for IV=seqno these need to be
                   zeroized */
                SAState_p->CW1 |= SAB_CW1_IV1|SAB_CW1_IV2;
                if (SAParams_p->IVSrc == SAB_IV_SRC_IMPLICIT)
                    SAState_p->CW1 |= SAB_CW1_IV_CTR;
            }
            else if (SAParams_p->IVSrc == SAB_IV_SRC_IMPLICIT)
                SAState_p->CW1 |= SAB_CW1_IV_ORIG_SEQ;

            if (SABuffer_p != NULL)
            {
                if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CCM)
                {
                    SABuffer_p[IVOffset] =
                        (SAParams_p->Nonce_p[0] << 8)  |
                        (SAParams_p->Nonce_p[1] << 16) |
                        (SAParams_p->Nonce_p[2] << 24) | SAB_CCM_FLAG_L4;
                }
                else
                    SABuilderLib_CopyKeyMat(SABuffer_p,
                                            IVOffset,
                                            SAParams_p->Nonce_p,
                                            sizeof(uint32_t));
            }
            IVOffset += 1;
            if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CCM)
            {
                /* Add 0 counter field (IV3) */
                SAState_p->CW1 |= SAB_CW1_IV3;
                if(SABuffer_p != NULL)
                    SABuffer_p[IVOffset] = 0;
                IVOffset += 1;
            }
            else if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CHACHA_CTR32)
            {
                /* For ChaCha20 the IV1 and IV2 words are required to be 0 */
                SABuilderLib_ZeroFill(SABuffer_p, IVOffset, 2*sizeof(uint32_t));
                IVOffset +=2;
            }
            if (IVOffset > SAState_p->CurrentOffset)
            {
                SAState_p->CurrentOffset = IVOffset;
            }
         }
    }
    return SAB_STATUS_OK;
}


#endif /* SAB_ENABLE_PROTO_IPSEC */

/* end of file sa_builder_ipsec.c */
