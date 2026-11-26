/* sa_builder_extended_basic.c
 *
 * Basic opaeration specific functions (for initialization of
 * SABuilder_Params_t structures and for building the Basic specific
 * part of an SA) in the Extended use case.
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */
#include "c_sa_builder.h"
#ifdef SAB_ENABLE_BASIC_EXTENDED
#include "sa_builder_extended_internal.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "basic_defs.h"
#include "log.h"
#include "sa_builder_internal.h" /* SABuilder_SetBasicParams */
#include "sa_builder_basic.h"

#include "firmware_eip207_api_cs.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * SABuilder_SetExtendedBasicParams
 *
 * Fill in Basic-specific extensions into the SA.for Extended.
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
SABuilder_SetExtendedBasicParams(SABuilder_Params_t *const SAParams_p,
                                 SABuilder_State_t * const SAState_p,
                                 uint32_t * const SABuffer_p)
{
    SABuilder_Params_Basic_t *SAParamsBasic_p =
        (SABuilder_Params_Basic_t *)(SAParams_p->ProtocolExtension_p);

    IDENTIFIER_NOT_USED(SAState_p);

#ifdef FIRMWARE_EIP207_CS_TR_IV_WORD_OFFSET
    /* These operations are specific to PDCP firmware */
    if (SAParams_p->AuthAlgo == SAB_AUTH_SNOW_UIA2 ||
        SAParams_p->AuthAlgo == SAB_AUTH_KASUMI_F9)
    {
        if(SABuffer_p != NULL)
            SABuffer_p[FIRMWARE_EIP207_CS_TR_IV_WORD_OFFSET] =
                SAParamsBasic_p->fresh;
    }
#endif
#ifdef FIRMWARE_EIP207_CS_FLOW_TR_BYTE_PARAM_WORD_OFFSET
    /* These operations are specific to non-PDCP firmware */
    if ((SAParams_p->CryptoAlgo != SAB_CRYPTO_NULL &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_ARCFOUR &&
         SAParams_p->CryptoAlgo != SAB_CRYPTO_CHACHA20 &&
         SAParams_p->AuthAlgo != SAB_AUTH_NULL &&
         SAParams_p->AuthAlgo != SAB_AUTH_AES_GCM &&
         SAParams_p->AuthAlgo != SAB_AUTH_AES_GMAC &&
         SAParams_p->AuthAlgo != SAB_AUTH_AES_CCM) ||
        (SAParams_p->CryptoAlgo == SAB_CRYPTO_NULL &&
         SAParams_p->AuthAlgo == SAB_AUTH_NULL))
    {
        /* Only combined crypto + hash and not AES-GCM/AES-GMAC/AES-CCM are
           supported */
        uint32_t TokenHeaderWord = SAB_HEADER_DEFAULT;
        SABuilder_ESPProtocol_t ESPProto;
        SABuilder_HeaderProtocol_t HeaderProto;
        uint8_t PadBlockByteCount = 1;
        uint8_t IVByteCount = 0;
        uint8_t ICVByteCount = 0;
        uint32_t flags = 0;
        uint32_t VerifyInstructionWord, CtxInstructionWord;
        uint32_t IVInstructionWord = 0;

        if (SAParams_p->CryptoAlgo == SAB_CRYPTO_NULL)
        {
            ESPProto = SAB_ESP_PROTO_NONE;
            if ((SAParamsBasic_p->BasicFlags & SAB_BASIC_FLAG_XFRM_API) != 0)

                HeaderProto = SAB_HDR_BASIC_IN_NO_PAD;
            else
                HeaderProto = SAB_HDR_BYPASS;
            VerifyInstructionWord = SAB_VERIFY_NONE;
        }
        else
        {
            if ((SAParamsBasic_p->BasicFlags & SAB_BASIC_FLAG_ENCRYPT_AFTER_HASH) != 0)
            {
                if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
                {
                    ESPProto = SAB_BASIC_PROTO_OUT_HASHENC;
                    HeaderProto = SAB_HDR_BASIC_OUT_TPAD;
                }
                else
                {
                    ESPProto = SAB_BASIC_PROTO_IN_DECHASH;
                    TokenHeaderWord |= SAB_HEADER_PAD_VERIFY;
                    HeaderProto = SAB_HDR_BASIC_IN_PAD;
                }
            }
            else
            {
                if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
                {
                    ESPProto = SAB_BASIC_PROTO_OUT_ENCHASH;
                    HeaderProto = SAB_HDR_BASIC_OUT_ZPAD;
                }
                else
                {
                    ESPProto = SAB_BASIC_PROTO_IN_HASHDEC;
                    HeaderProto = SAB_HDR_BASIC_IN_NO_PAD;
                }
        }

            if ((SAParams_p->flags & SAB_FLAG_SUPPRESS_HEADER) == 0)
                flags |= BIT_29;

            switch(SAParams_p->CryptoAlgo)
            {
            case SAB_CRYPTO_DES:
            case SAB_CRYPTO_3DES:
                IVByteCount = 8;
                PadBlockByteCount = 8;
            break;
            case SAB_CRYPTO_AES:
            case SAB_CRYPTO_SM4:
            case SAB_CRYPTO_BC0:
                IVByteCount = 16;
                PadBlockByteCount = 16;
                break;
            default:
                LOG_CRIT("SABuilder_BuildSA: unsupported crypto algorithm\n");
                return SAB_UNSUPPORTED_FEATURE;
            }

            switch(SAParams_p->CryptoMode)
            {
            case SAB_CRYPTO_MODE_ECB:
                IVByteCount = 0;
                IVInstructionWord = 0x20000004; /* NOP instruction */
                break;
            case SAB_CRYPTO_MODE_CBC:
                if (SAParams_p->IVSrc == SAB_IV_SRC_DEFAULT ||
                    SAParams_p->IVSrc == SAB_IV_SRC_INPUT)
                {
                    IVInstructionWord = SA_RETR_HASH_IV0 + IVByteCount;
                }
                else
                {
                    IVInstructionWord = SA_INS_NONE_IV0 + IVByteCount;
                    IVByteCount = 0;
                }
                if (SAParams_p->IVSrc == SAB_IV_SRC_PRNG)
                    TokenHeaderWord |= SAB_HEADER_IV_PRNG;
                if ((SAParams_p->flags & SAB_FLAG_COPY_IV) != 0)
                {
                    IVInstructionWord |= BIT_25|BIT_24; /* IV to output & hash */
                }
                if ((SAParamsBasic_p->BasicFlags & SAB_BASIC_FLAG_ENCRYPT_AFTER_HASH) != 0)
                {
                    IVInstructionWord &= ~BIT_25; /* Do not hash IV for HASHENC */
                }
                break;
            case SAB_CRYPTO_MODE_CTR:
                IVByteCount = 8;
                PadBlockByteCount = 1;
                if (SAParams_p->IVSrc == SAB_IV_SRC_DEFAULT ||
                    SAParams_p->IVSrc == SAB_IV_SRC_INPUT)
                {
                    IVInstructionWord = SA_RETR_HASH_IV1 + IVByteCount;
                }
                else
                {
                    IVInstructionWord = SA_INS_NONE_IV1 + IVByteCount;
                    IVByteCount = 0;
                }
                if ((SAParams_p->flags & SAB_FLAG_COPY_IV) != 0)
                {
                    IVInstructionWord |= BIT_25|BIT_24; /* IV to output & hash */
                }
                break;
            case SAB_CRYPTO_MODE_ICM:
                IVByteCount = 16;
                PadBlockByteCount = 1;
                if (SAParams_p->IVSrc == SAB_IV_SRC_DEFAULT ||
                    SAParams_p->IVSrc == SAB_IV_SRC_INPUT)
                {
                    IVInstructionWord = SA_RETR_HASH_IV0 + IVByteCount;
                }
                else
                {
                    IVInstructionWord = SA_INS_NONE_IV0 + IVByteCount;
                    IVByteCount = 0;
                }
                if ((SAParams_p->flags & SAB_FLAG_COPY_IV) != 0)
                {
                    IVInstructionWord |= BIT_25|BIT_24; /* IV to output & hash */
                }
                break;
            default:
                LOG_CRIT("SABuilder_BuildSA: unsupported crypto mode\n");
                return SAB_UNSUPPORTED_FEATURE;
            }

            switch(SAParams_p->AuthAlgo)
            {
            case SAB_AUTH_HASH_MD5:
            case SAB_AUTH_SSLMAC_MD5:
            case SAB_AUTH_HMAC_MD5:
                ICVByteCount = 16;
                break;
            case SAB_AUTH_HASH_SHA1:
            case SAB_AUTH_SSLMAC_SHA1:
            case SAB_AUTH_HMAC_SHA1:
                ICVByteCount = 20;
                break;
            case SAB_AUTH_HASH_SHA3_224:
            case SAB_AUTH_KEYED_HASH_SHA3_224:
            case SAB_AUTH_HMAC_SHA3_224:
                ICVByteCount = 28;
                break;
            case SAB_AUTH_HASH_SHA2_224:
            case SAB_AUTH_HMAC_SHA2_224:
            case SAB_AUTH_HASH_SHA2_256:
            case SAB_AUTH_HMAC_SHA2_256:
            case SAB_AUTH_HMAC_SM3:
            case SAB_AUTH_HASH_SM3:
            case SAB_AUTH_HASH_SHA3_256:
            case SAB_AUTH_KEYED_HASH_SHA3_256:
            case SAB_AUTH_HMAC_SHA3_256:
                ICVByteCount = 32;
                break;
            case SAB_AUTH_HASH_SHA3_384:
            case SAB_AUTH_KEYED_HASH_SHA3_384:
            case SAB_AUTH_HMAC_SHA3_384:
                ICVByteCount = 48;
                break;
            case SAB_AUTH_HASH_SHA2_384:
            case SAB_AUTH_HMAC_SHA2_384:
                if ((SAParamsBasic_p->BasicFlags & SAB_BASIC_FLAG_ENCRYPT_AFTER_HASH) != 0)
                    ICVByteCount = 48;
                else
                    ICVByteCount = 64;
                break;
            case SAB_AUTH_HASH_SHA3_512:
            case SAB_AUTH_KEYED_HASH_SHA3_512:
            case SAB_AUTH_HMAC_SHA3_512:
            case SAB_AUTH_HASH_SHA2_512:
            case SAB_AUTH_HMAC_SHA2_512:
                ICVByteCount = 64;
                break;
            case SAB_AUTH_AES_XCBC_MAC:
            case SAB_AUTH_AES_CMAC_128:
            case SAB_AUTH_AES_CMAC_192:
            case SAB_AUTH_AES_CMAC_256:
                ICVByteCount = 16;
                break;
            default:
                LOG_CRIT("SABuilder_BuildSA: unsupported authentication algorithm\n");
                return SAB_UNSUPPORTED_FEATURE;
            }
            if (SAParamsBasic_p->ICVByteCount != 0 &&
                SAParamsBasic_p->ICVByteCount < ICVByteCount)
                ICVByteCount = SAParamsBasic_p->ICVByteCount;

            /* Take care of the VERIFY and CTX token instructions */
            if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
            {
                VerifyInstructionWord = SAB_VERIFY_NONE;
            }
            else
            {
                if ((SAParamsBasic_p->BasicFlags & SAB_BASIC_FLAG_ENCRYPT_AFTER_HASH) != 0)
                {
                    VerifyInstructionWord = SAB_VERIFY_PAD;
                }
                else
                {
                    VerifyInstructionWord = SAB_VERIFY_NONE;
                }
                VerifyInstructionWord += SAB_VERIFY_BIT_H + ICVByteCount;
            }
        }

#ifdef SAB_ENABLE_TWO_FIXED_RECORD_SIZES
        if (SAState_p->fLarge)
        {
            CtxInstructionWord = SAB_CTX_NONE + FIRMWARE_EIP207_CS_FLOW_TRC_RECORD_WORD_COUNT + LargeTransformOffset - 1;
        }
        else
#endif
        {
            CtxInstructionWord = SAB_CTX_NONE + FIRMWARE_EIP207_CS_FLOW_TRC_RECORD_WORD_COUNT - 1;
        }

        /* Write all parameters to their respective offsets */
        if (SABuffer_p != NULL)
        {
#ifdef SAB_ENABLE_TWO_FIXED_RECORD_SIZES
            if (SAState_p->fLarge)
            {
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_FLAGS_WORD_OFFSET +
                           LargeTransformOffset] = flags;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_HDRPROC_CTX_WORD_OFFSET +
                    LargeTransformOffset] = SAParamsBasic_p->ContextRef;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_BYTE_PARAM_WORD_OFFSET +
                           LargeTransformOffset] =
                    SAB_PACKBYTES(IVByteCount,ICVByteCount,HeaderProto,ESPProto);
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TK_HDR_WORD_OFFSET +
                           LargeTransformOffset] = TokenHeaderWord;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_PAD_ALIGN_WORD_OFFSET +
                           LargeTransformOffset] =
                    SAB_PACKBYTES(PadBlockByteCount/2, 0, 0, 0);
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_CCM_SALT_WORD_OFFSET +
                    LargeTransformOffset] =IVInstructionWord;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TK_VFY_INST_WORD_OFFSET +
                    LargeTransformOffset] = VerifyInstructionWord;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TK_CTX_INST_WORD_OFFSET +
                    LargeTransformOffset] = CtxInstructionWord;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_LO_WORD_OFFSET +
                           LargeTransformOffset] = 0;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_HI_WORD_OFFSET +
                    LargeTransformOffset] = 0;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_LO_WORD_OFFSET +
                    LargeTransformOffset] = 0;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_HI_WORD_OFFSET +
                    LargeTransformOffset] = 0;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_STAT_PKT_WORD_OFFSET +
                    LargeTransformOffset] = 0;
            }
            else
#endif /* SAB_ENABLE_TWO_FIXED_RECORD_SIZES */
            {
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_FLAGS_WORD_OFFSET] = flags;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_HDRPROC_CTX_WORD_OFFSET] =
                    SAParamsBasic_p->ContextRef;
;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_BYTE_PARAM_WORD_OFFSET] =
                    SAB_PACKBYTES(IVByteCount,ICVByteCount,HeaderProto,ESPProto);
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TK_HDR_WORD_OFFSET] = TokenHeaderWord;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_PAD_ALIGN_WORD_OFFSET] =
                    SAB_PACKBYTES(PadBlockByteCount/2, 0, 0, 0);
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_CCM_SALT_WORD_OFFSET] = IVInstructionWord;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TK_VFY_INST_WORD_OFFSET] =
                    VerifyInstructionWord;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TK_CTX_INST_WORD_OFFSET] =
                    CtxInstructionWord;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_LO_WORD_OFFSET] = 0;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_HI_WORD_OFFSET] = 0;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_LO_WORD_OFFSET] = 0;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_HI_WORD_OFFSET] = 0;
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_STAT_PKT_WORD_OFFSET] = 0;
            }
        }
    }
#endif
    return SAB_STATUS_OK;
}

#endif /* SAB_ENABLE_BASIC_EXTENDED */


/* end of file sa_builder_extended_basic.c */
