/* sa_builder_extended_ipsec.c
 *
 * IPsec specific functions (for initialization of SABuilder_Params_t
 * structures and for building the IPSec specifc part of an SA.) in the
 * Extended use case.
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */
#include "c_sa_builder.h"

#ifdef SAB_ENABLE_IPSEC_EXTENDED
#include "sa_builder_extended_internal.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "basic_defs.h"
#include "log.h"
#include "sa_builder_internal.h" /* SABuilder_SetIpsecParams */
#include "sa_builder_ipsec.h"

/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#define ESP_HDR_LEN 8
#define IPV4_HDR_LEN 20
#define IPV6_HDR_LEN 40

/*----------------------------------------------------------------------------
 * Local variables
 */


#ifdef SAB_ENABLE_EXTENDED_TUNNEL_HEADER
/*----------------------------------------------------------------------------
 * get16
 *
 * Read 16-bit value from byte array not changing the byte order.
 */
static uint16_t
get16no(
        uint8_t *p,
        unsigned int offs)
{
    return (p[offs+1]<<8) | p[offs];
}
#endif

/*----------------------------------------------------------------------------
 * SABuilder_SetExtendedIPsecParams
 *
 * Fill in IPsec-specific extensions into the SA.for Extended.
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
SABuilder_SetExtendedIPsecParams(SABuilder_Params_t *const SAParams_p,
                         SABuilder_State_t * const SAState_p,
                         uint32_t * const SABuffer_p)
{
    SABuilder_Params_IPsec_t *SAParamsIPsec_p =
        (SABuilder_Params_IPsec_t *)(SAParams_p->ProtocolExtension_p);
    uint32_t TokenHeaderWord = SAB_HEADER_DEFAULT;
    SABuilder_ESPProtocol_t ESPProto;
    SABuilder_HeaderProtocol_t HeaderProto;
    uint8_t PadBlockByteCount;
    uint8_t IVByteCount;
    uint8_t ICVByteCount;
    uint8_t SeqOffset;
    uint8_t ExtSeq = 0;
    uint8_t AntiReplay;
    uint32_t CCMSalt = 0;
    uint32_t flags = 0;
    uint32_t VerifyInstructionWord, CtxInstructionWord;
#ifdef SAB_ENABLE_EXTENDED_TUNNEL_HEADER
    uint32_t MTUDiscount = 0;
    uint32_t CheckSum = 0;
#endif
    IDENTIFIER_NOT_USED(SAState_p);

    if (SAParamsIPsec_p == NULL)
    {
        LOG_CRIT("SABuilder: IPsec extension pointer is null\n");
        return SAB_INVALID_PARAMETER;
    }

    if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_ESP) == 0)
    {
        LOG_CRIT("SABuilder: IPsec only supports ESP.\n");
        return SAB_INVALID_PARAMETER;
    }

    if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_XFRM_API) != 0)
    {
        if(SAParams_p->CryptoMode != SAB_CRYPTO_MODE_CBC &&
           SAParams_p->CryptoMode != SAB_CRYPTO_MODE_GCM)
        {
            LOG_CRIT("SABuilder: IPsec for XFRM only supports CBC and GCM modes.\n");
            return SAB_INVALID_PARAMETER;
        }
        if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_NATT) != 0)
        {
            LOG_CRIT("SABuilder: IPsec for XFRM does not support NATT\n");
            return SAB_INVALID_PARAMETER;
        }

    }
    if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_NO_ANTI_REPLAY) != 0)
        AntiReplay = 0;
    else
        AntiReplay = 1;

    if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
    {
        if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_XFRM_API) != 0)
            ESPProto = SAB_ESP_PROTO_OUT_XFRM_CBC;
        else
            ESPProto = SAB_ESP_PROTO_OUT_CBC;
        PadBlockByteCount = 4;
        if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_IPV6) !=0)
        {
            if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_XFRM_API) != 0)
            {
                    HeaderProto = SAB_HDR_IPV6_OUT_XFRM;
            }
            else if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_PROCESS_IP_HEADERS) !=0)
            {
                if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_TUNNEL) !=0)
                {
                    HeaderProto = SAB_HDR_IPV6_OUT_TUNNEL;
                }
                else
                {
                    HeaderProto = SAB_HDR_IPV6_OUT_TRANSP;
                }
            }
            else
            {
                HeaderProto = SAB_HDR_IPV6_OUT_TRANSP_HDRBYPASS;
            }
        }
        else
        {
            if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_XFRM_API) != 0)
            {
                    HeaderProto = SAB_HDR_IPV4_OUT_XFRM;
            }
            else if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_PROCESS_IP_HEADERS) !=0)
            {
                if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_TUNNEL) !=0)
                {
                    HeaderProto = SAB_HDR_IPV4_OUT_TUNNEL;
                }
                else
                {
                    HeaderProto = SAB_HDR_IPV4_OUT_TRANSP;
                }
            }
            else
            {
                HeaderProto = SAB_HDR_IPV4_OUT_TRANSP_HDRBYPASS;
            }
        }

        if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_LONG_SEQ) != 0)
            ExtSeq = 1;
    }
    else
    {
        if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_XFRM_API) != 0)
            ESPProto = SAB_ESP_PROTO_IN_XFRM_CBC;
        else
            ESPProto = SAB_ESP_PROTO_IN_CBC;
        PadBlockByteCount = 4;
        TokenHeaderWord |= SAB_HEADER_PAD_VERIFY;

        if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_IPV6) !=0)
        {
            if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_XFRM_API) != 0)
            {
                    HeaderProto = SAB_HDR_IPV6_IN_XFRM;
            }
            else if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_PROCESS_IP_HEADERS) !=0)
            {
                if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_TUNNEL) !=0)
                {
                    HeaderProto = SAB_HDR_IPV6_IN_TUNNEL;
                }
                else
                {
                    HeaderProto = SAB_HDR_IPV6_IN_TRANSP;
                    TokenHeaderWord |= SAB_HEADER_UPD_HDR;
                }
            }
            else
            {
                HeaderProto = SAB_HDR_IPV6_IN_TRANSP_HDRBYPASS;
            }
        }
        else
        {
            if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_XFRM_API) != 0)
            {
                    HeaderProto = SAB_HDR_IPV4_IN_XFRM;
            }
            else if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_PROCESS_IP_HEADERS) !=0)
            {
                if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_TUNNEL) !=0)
                {
                    HeaderProto = SAB_HDR_IPV4_IN_TUNNEL;
                }
                else
                {
                    HeaderProto = SAB_HDR_IPV4_IN_TRANSP;
                    TokenHeaderWord |= SAB_HEADER_UPD_HDR;
                }
            }
            else
            {
                HeaderProto = SAB_HDR_IPV4_IN_TRANSP_HDRBYPASS;
            }
        }

        if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_LONG_SEQ) != 0)
            ExtSeq = 1;
        AntiReplay *= SAParamsIPsec_p->SequenceMaskBitCount / 32;
    }
    SeqOffset = SAParams_p->OffsetSeqNum;

    switch (SAParams_p->CryptoAlgo)
    {
    case SAB_CRYPTO_NULL:
        IVByteCount = 0;
                break;
    case SAB_CRYPTO_DES:
    case SAB_CRYPTO_3DES:
        IVByteCount = 8;
        PadBlockByteCount = 8;
        break;
    case SAB_CRYPTO_AES:
    case SAB_CRYPTO_SM4:
    case SAB_CRYPTO_BC0:
        if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CBC)
        {
            IVByteCount = 16;
            PadBlockByteCount = 16;
        }
        else
        {
            if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
                ESPProto = SAB_ESP_PROTO_OUT_CTR;
            else
                ESPProto = SAB_ESP_PROTO_IN_CTR;

            if (SAParams_p->IVSrc == SAB_IV_SRC_IMPLICIT)
                IVByteCount = 0;
            else
                IVByteCount = 8;
        }
        break;
    case SAB_CRYPTO_CHACHA20:
        if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
            ESPProto = SAB_ESP_PROTO_OUT_CHACHAPOLY;
        else
            ESPProto = SAB_ESP_PROTO_IN_CHACHAPOLY;

        if (SAParams_p->IVSrc == SAB_IV_SRC_IMPLICIT)
            IVByteCount = 0;
        else
            IVByteCount = 8;
        break;
    default:
            LOG_CRIT("SABuilder_BuildSA:"
                     "Unsupported Crypto algorithm\n");
            return SAB_INVALID_PARAMETER;
        ;
    }

    /* For all inbound and CTR mode outbound packets there is
       only one supported way to obtain the IV, which is already
       taken care of. Now handle outbound CBC. */
    if(SAParams_p->CryptoMode == SAB_CRYPTO_MODE_CBC &&
       SAParams_p->direction == SAB_DIRECTION_OUTBOUND &&
       SAParams_p->CryptoAlgo != SAB_CRYPTO_NULL)
    {
        switch (SAParams_p->IVSrc)
        {
        case SAB_IV_SRC_PRNG:
            TokenHeaderWord |=
                SAB_HEADER_IV_PRNG;
            break;
        case SAB_IV_SRC_DEFAULT:
        case SAB_IV_SRC_SA: /* No action required */
        case SAB_IV_SRC_TOKEN:
            break;
        default:
            LOG_CRIT("SABuilder_BuildSA:"
                     "Unsupported IV source\n");
            return SAB_INVALID_PARAMETER;
        }
    }

    if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND &&
        SAParamsIPsec_p->PadAlignment >
        PadBlockByteCount &&
        SAParamsIPsec_p->PadAlignment <= 256)
        PadBlockByteCount =
            SAParamsIPsec_p->PadAlignment;

    switch(SAParams_p->AuthAlgo)
    {
    case SAB_AUTH_NULL:
        ICVByteCount = 0;
        ExtSeq = 0;
        if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
            ESPProto = SAB_ESP_PROTO_OUT_NULLAUTH;
        else
            ESPProto = SAB_ESP_PROTO_IN_NULLAUTH;

        break;
            case SAB_AUTH_HMAC_MD5:
    case SAB_AUTH_HMAC_SHA1:
    case SAB_AUTH_AES_XCBC_MAC:
    case SAB_AUTH_AES_CMAC_128:
        ICVByteCount = 12;
        break;
    case SAB_AUTH_HMAC_SHA2_224:
    case SAB_AUTH_HMAC_SHA2_256:
    case SAB_AUTH_HMAC_SM3:
        ICVByteCount = 16;
        break;
    case SAB_AUTH_HMAC_SHA2_384:
        ICVByteCount = 24;
        break;
    case SAB_AUTH_HMAC_SHA2_512:
        ICVByteCount = 32;
        break;
    case SAB_AUTH_AES_CCM:
    case SAB_AUTH_AES_GCM:
    case SAB_AUTH_AES_GMAC:
        // All these protocols have a selectable ICV length.
        if (SAParamsIPsec_p->ICVByteCount == 8 ||
            SAParamsIPsec_p->ICVByteCount == 12 ||
            SAParamsIPsec_p->ICVByteCount == 16)
        {
            ICVByteCount =
                        SAParamsIPsec_p->ICVByteCount;
        }
        else
        {
            ICVByteCount = 16;
        }
        switch (SAParams_p->AuthAlgo)
        {
            /* These protocols need specialized protocol codes
               for the token generator.*/
        case SAB_AUTH_AES_CCM:
            if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
                ESPProto = SAB_ESP_PROTO_OUT_CCM;
            else
                ESPProto = SAB_ESP_PROTO_IN_CCM;

            CCMSalt =
                (SAParams_p->Nonce_p[0] << 8) |
                (SAParams_p->Nonce_p[1] << 16) |
                (SAParams_p->Nonce_p[2] << 24) |
                SAB_CCM_FLAG_ADATA_L4 |
                ((ICVByteCount-2)*4);
            break;
        case SAB_AUTH_AES_GCM:
            if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
            {
                if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_XFRM_API) != 0)
                    ESPProto = SAB_ESP_PROTO_OUT_XFRM_GCM;
                else
                    ESPProto = SAB_ESP_PROTO_OUT_GCM;
            }
            else
            {
                if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_XFRM_API) != 0)
                    ESPProto = SAB_ESP_PROTO_IN_XFRM_GCM;
                else
                    ESPProto = SAB_ESP_PROTO_IN_GCM;
            }
            break;
        case SAB_AUTH_AES_GMAC:
            if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
                        ESPProto = SAB_ESP_PROTO_OUT_GMAC;
            else
                ESPProto = SAB_ESP_PROTO_IN_GMAC;
            break;
        default:
            ;
        }
        break;
    case SAB_AUTH_POLY1305:
        ICVByteCount = 16;
        break;
    default:
        LOG_CRIT("SABuilder_BuildSA: unsupported authentication algorithm\n");
        return SAB_UNSUPPORTED_FEATURE;
    }


    /* Flags variable */
    if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_IPV6) !=0)
        flags |= BIT_8;
    if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_PROCESS_IP_HEADERS) !=0)
        flags |= BIT_19;
    if (ExtSeq !=0)
        flags |= BIT_29;
    if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_DEC_TTL) != 0)
        flags |= BIT_27;
    if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_CLEAR_DF) != 0)
        flags |= BIT_20;
    if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_SET_DF) != 0)
        flags |= BIT_21;
    if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_REPLACE_DSCP) != 0)
        flags |= BIT_22;
    if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_CLEAR_ECN) != 0)
        flags |= BIT_23;
    if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_APPEND_SEQNUM) != 0)
    {
        if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_LONG_SEQ) != 0)
            flags |= BIT_25;
        else
            flags |= BIT_24;
    }
    if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_TRANSPORT_NAT) != 0)
    {
        if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_TUNNEL) != 0)
        {
            LOG_CRIT("NAT only for transport\n");
            return SAB_INVALID_PARAMETER;
        }
        if (SAParams_p->direction==SAB_DIRECTION_INBOUND &&
            SAParamsIPsec_p->SequenceMaskBitCount > 128)
        {
            if (SAState_p->fLarge && LargeTransformOffset == 16)
            {
                LOG_CRIT(
                    "SABuilder_BuildSA: Inbound NAT cannot be combined with \n"
                    " anti-replay mask > 128\n and HMAC-SHA384/512\n");
                return SAB_UNSUPPORTED_FEATURE;
            }
            else if (SAParams_p->OffsetSeqNum == SAB_SEQNUM_HI_FIX_OFFSET &&
                     SAParamsIPsec_p->SequenceMaskBitCount > 384)
            {
                LOG_CRIT(
                    "SABuilder_BuildSA: Inbound NAT cannot be combined with \n"
                    " anti-replay mask > 384\n and HMAC-SHA384/512\n");
                return SAB_UNSUPPORTED_FEATURE;
            }
            else
            {
                SAState_p->fLarge = true;
            }
        }
        flags |= BIT_28;
    }

    /* Take care of the VERIFY and CTX token instructions */
    if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
    {
        VerifyInstructionWord = SAB_VERIFY_NONE;
#ifdef SAB_ENABLE_TWO_FIXED_RECORD_SIZES
        if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_XFRM_API) != 0)
        {
            if (SAState_p->fLarge)
            {
                CtxInstructionWord = SAB_CTX_NONE + FIRMWARE_EIP207_CS_FLOW_TRC_RECORD_WORD_COUNT + LargeTransformOffset - 1;
            }
            else
#endif
            {
                CtxInstructionWord = SAB_CTX_NONE + FIRMWARE_EIP207_CS_FLOW_TRC_RECORD_WORD_COUNT - 1;
            }
        }
        else
        {
            CtxInstructionWord = SAB_CTX_OUT_SEQNUM +
                ((unsigned int)(ExtSeq+1)<<24) + SeqOffset;
        }
    }
    else
    {
        VerifyInstructionWord = SAB_VERIFY_PADSPI;
        if (ICVByteCount > 0)
        {
            VerifyInstructionWord += SAB_VERIFY_BIT_H + ICVByteCount;
        }
        if (AntiReplay > 0 &&
            (SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_APPEND_SEQNUM) == 0 &&
            (SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_XFRM_API) == 0)
        {
            /* Skip verification of sequence number in sequence number append
               mode. */
            VerifyInstructionWord += SAB_VERIFY_BIT_SEQ;
        }
        if (ICVByteCount == 0 || AntiReplay == 0 ||
            (SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_XFRM_API) != 0)
        {
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
        }
        else if (ExtSeq != 0 ||
                 (AntiReplay != 0 &&
                  SAParams_p->OffsetSeqNum + 2 == SAParams_p->OffsetSeqMask))
        {
            if (AntiReplay > 12)
                CtxInstructionWord = SAB_CTX_SEQNUM +
                     + SeqOffset;
            else
                CtxInstructionWord = SAB_CTX_SEQNUM +
                    ((unsigned int)(2+AntiReplay)<<24) + SeqOffset;
        }
        else
        {
            CtxInstructionWord = SAB_CTX_INSEQNUM +
                ((unsigned int)(1+AntiReplay)<<24) + SeqOffset;
        }
    }

#ifdef SAB_ENABLE_EXTENDED_TUNNEL_HEADER
    /* Compute the maximum amount by which the packet can be enlarged,
       so discount that from the output MTU to judge whether a packet can
       be processed without fragmentation. */
    if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
    {
        MTUDiscount = ESP_HDR_LEN + 1 + PadBlockByteCount +
            IVByteCount + ICVByteCount;

        if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_TUNNEL) !=0)
        {
            if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_IPV4) !=0)
                MTUDiscount += IPV4_HDR_LEN;
            else
                MTUDiscount += IPV6_HDR_LEN;

            // for IPv4 tunnel, pre-calculate checksum on IP addresses and store them in the transform record
            // this checksum does not include the final inversion and is performed on data
            // as they stored in the memory
            if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_IPV4) !=0)
            {
                // protection against NULL pointers
                if ((SAParamsIPsec_p->SrcIPAddr_p != NULL)&&
                    (SAParamsIPsec_p->DestIPAddr_p != NULL))
                {
                    // add the addresses (in order they are stored in the memory)
                    CheckSum += get16no(SAParamsIPsec_p->SrcIPAddr_p, 0);
                    CheckSum += get16no(SAParamsIPsec_p->SrcIPAddr_p, 2);
                    CheckSum += get16no(SAParamsIPsec_p->DestIPAddr_p, 0);
                    CheckSum += get16no(SAParamsIPsec_p->DestIPAddr_p, 2);

                    // process the carries
                    while ((CheckSum>>16) != 0)
                        CheckSum = (CheckSum>>16) + (CheckSum & 0xffff);
                }
            }
        }
    }
    /* Compute the checksum delta for internal NAT operations and for inbound
       transport NAT-T checksum fixup */
    if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_TUNNEL) == 0 &&
        (SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_CHECKSUM_FIX) != 0)
    {
        uint8_t IPLen = SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_IPV4?4:16;
        unsigned int i;
        // Compute source address delta only if both original and new source
        // addresses are provided, otherwise assume source address is unchanged.
        if (SAParamsIPsec_p->SrcIPAddr_p != NULL &&
            SAParamsIPsec_p->OrigSrcIPAddr_p != NULL)
        {
            for (i=0; i<IPLen; i+=2)
            {
                CheckSum += get16no(SAParamsIPsec_p->SrcIPAddr_p, i);
                CheckSum += get16no(SAParamsIPsec_p->OrigSrcIPAddr_p, i) ^ 0xffff;
            }
        }
        // Compute destination address delta only if both original and
        // new destination addresses are provided, otherwise assume
        // destination address is unchanged.
        if (SAParamsIPsec_p->DestIPAddr_p != NULL &&
            SAParamsIPsec_p->OrigDestIPAddr_p != NULL)
        {
            for (i=0; i<IPLen; i+=2)
            {
                CheckSum += get16no(SAParamsIPsec_p->DestIPAddr_p, i);
                CheckSum += get16no(SAParamsIPsec_p->OrigDestIPAddr_p, i) ^ 0xffff;
            }
        }
        // process the carries
        while ((CheckSum>>16) != 0)
            CheckSum = (CheckSum>>16) + (CheckSum & 0xffff);
    }

#endif

    /* If NAT-T selected, select other header protocol range */
    if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_NATT) != 0)
        HeaderProto += (SAB_HDR_IPV4_OUT_TRANSP_HDRBYPASS_NATT -
                        SAB_HDR_IPV4_OUT_TRANSP_HDRBYPASS);

    /* Write all parameters to their respective offsets */
    if (SABuffer_p != NULL)
    {
#ifdef SAB_ENABLE_TWO_FIXED_RECORD_SIZES
        if (SAState_p->fLarge)
        {
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_FLAGS_WORD_OFFSET +
                       LargeTransformOffset] = flags;
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_HDRPROC_CTX_WORD_OFFSET +
                       LargeTransformOffset] =
                SAParamsIPsec_p->ContextRef;
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_BYTE_PARAM_WORD_OFFSET +
                       LargeTransformOffset] =
                SAB_PACKBYTES(IVByteCount,ICVByteCount,HeaderProto,ESPProto);
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TK_HDR_WORD_OFFSET +
                       LargeTransformOffset] = TokenHeaderWord;
            if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_CHECKSUM_FIX) != 0 &&
                (SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_TUNNEL) == 0)
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_PAD_ALIGN_WORD_OFFSET +
                    LargeTransformOffset] =
                    SAB_PACKBYTES(PadBlockByteCount/2,
                                  0,
                                  CheckSum & 0xff,
                                  CheckSum >> 8);
            else if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_TUNNEL) != 0)
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_PAD_ALIGN_WORD_OFFSET +
                           LargeTransformOffset] =
                    SAB_PACKBYTES(PadBlockByteCount/2,
                                  0,
                                  SAParamsIPsec_p->TTL,
                                  SAParamsIPsec_p->DSCP);
            else
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_PAD_ALIGN_WORD_OFFSET
                           + LargeTransformOffset] =
                    SAB_PACKBYTES(PadBlockByteCount/2,
                                  0,
                                  0,
                                  0);

            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_CCM_SALT_WORD_OFFSET +
                       LargeTransformOffset] = CCMSalt;
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TK_VFY_INST_WORD_OFFSET +
                       LargeTransformOffset] =
                VerifyInstructionWord;
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TK_CTX_INST_WORD_OFFSET +
                       LargeTransformOffset] =
                CtxInstructionWord;
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
#ifdef SAB_ENABLE_EXTENDED_TUNNEL_HEADER
            SABuffer_p[FIMRWARE_EIP207_CS_FLOW_TR_NATT_PORTS_WORD_OFFSET +
                       LargeTransformOffset] =
                SAB_PACKBYTES(SAParamsIPsec_p->NATTSrcPort >> 8,
                              SAParamsIPsec_p->NATTSrcPort & 0xff,
                              SAParamsIPsec_p->NATTDestPort >> 8,
                              SAParamsIPsec_p->NATTDestPort & 0xff);

            if (HeaderProto == SAB_HDR_IPV4_OUT_TUNNEL ||
                HeaderProto == SAB_HDR_IPV6_OUT_TUNNEL ||
                HeaderProto == SAB_HDR_IPV4_OUT_TUNNEL_NATT ||
                HeaderProto == SAB_HDR_IPV6_OUT_TUNNEL_NATT ||
                (SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_TRANSPORT_NAT) != 0)
            {
#ifdef SAB_STRICT_ARGS_CHECK
                if (SAParamsIPsec_p->SrcIPAddr_p == NULL ||
                    SAParamsIPsec_p->DestIPAddr_p == NULL)
                {
                    LOG_CRIT("SABuilder: NULL pointer tunnel address.\n");
                    return SAB_INVALID_PARAMETER;
                }
#endif
                SABuilderLib_CopyKeyMat(SABuffer_p,
                                        FIRMWARE_EIP207_CS_FLOW_TR_TUNNEL_SRC_WORD_OFFSET + LargeTransformOffset,
                                        SAParamsIPsec_p->SrcIPAddr_p,
                                        (SAParamsIPsec_p->IPsecFlags&SAB_IPSEC_IPV4)!=0?4:16);
                SABuilderLib_CopyKeyMat(SABuffer_p,
                                        FIRMWARE_EIP207_CS_FLOW_TR_TUNNEL_DST_WORD_OFFSET + LargeTransformOffset,
                                        SAParamsIPsec_p->DestIPAddr_p,
                                        (SAParamsIPsec_p->IPsecFlags&SAB_IPSEC_IPV4)!=0?4:16);

#ifdef FIRMWARE_EIP207_CS_FLOW_TR_CHECKSUM_WORD_OFFSET
                // checksum (only for IPv4)
                if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_IPV4) !=0)
                    SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_CHECKSUM_WORD_OFFSET +
                        LargeTransformOffset] = CheckSum;
#endif
            }

            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_PATH_MTU_WORD_OFFSET +
                       LargeTransformOffset] =  MTUDiscount;
#endif /* SAB_ENABLE_EXTENDED_TUNNEL_HEADER */
        }
        else
#endif /* SAB_ENABLE_TWO_FIXED_RECORD_SIZES */
        {
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_FLAGS_WORD_OFFSET] = flags;
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_HDRPROC_CTX_WORD_OFFSET] =
                SAParamsIPsec_p->ContextRef;
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_BYTE_PARAM_WORD_OFFSET] =
                SAB_PACKBYTES(IVByteCount,ICVByteCount,HeaderProto,ESPProto);
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TK_HDR_WORD_OFFSET] = TokenHeaderWord;
            if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_CHECKSUM_FIX) != 0 &&
                (SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_TUNNEL) == 0)
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_PAD_ALIGN_WORD_OFFSET] =
                    SAB_PACKBYTES(PadBlockByteCount/2,
                                  0,
                                  CheckSum & 0xff,
                                  CheckSum >> 8);
            else if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_TUNNEL) != 0)
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_PAD_ALIGN_WORD_OFFSET] =
                    SAB_PACKBYTES(PadBlockByteCount/2,
                                  0,
                                  SAParamsIPsec_p->TTL,
                                  SAParamsIPsec_p->DSCP);
            else
                SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_PAD_ALIGN_WORD_OFFSET] =
                    SAB_PACKBYTES(PadBlockByteCount/2,
                                  0,
                                  0,
                                  0);
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_CCM_SALT_WORD_OFFSET] = CCMSalt;
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TK_VFY_INST_WORD_OFFSET] =
                VerifyInstructionWord;
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TK_CTX_INST_WORD_OFFSET] =
                CtxInstructionWord;
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_LO_WORD_OFFSET] = 0;
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_HI_WORD_OFFSET] = 0;
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_LO_WORD_OFFSET] = 0;
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_HI_WORD_OFFSET] = 0;
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_STAT_PKT_WORD_OFFSET] = 0;
#ifdef SAB_ENABLE_EXTENDED_TUNNEL_HEADER
            SABuffer_p[FIMRWARE_EIP207_CS_FLOW_TR_NATT_PORTS_WORD_OFFSET] =
                SAB_PACKBYTES(SAParamsIPsec_p->NATTSrcPort >> 8,
                              SAParamsIPsec_p->NATTSrcPort & 0xff,
                              SAParamsIPsec_p->NATTDestPort >> 8,
                              SAParamsIPsec_p->NATTDestPort & 0xff);

            if (HeaderProto == SAB_HDR_IPV4_OUT_TUNNEL ||
                HeaderProto == SAB_HDR_IPV6_OUT_TUNNEL ||
                HeaderProto == SAB_HDR_IPV4_OUT_TUNNEL_NATT ||
                HeaderProto == SAB_HDR_IPV6_OUT_TUNNEL_NATT ||
                (SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_TRANSPORT_NAT) != 0)
            {
#ifdef SAB_STRICT_ARGS_CHECK
                if (SAParamsIPsec_p->SrcIPAddr_p == NULL ||
                    SAParamsIPsec_p->DestIPAddr_p == NULL)
                {
                    LOG_CRIT("SABuilder: NULL pointer tunnel address.\n");
                    return SAB_INVALID_PARAMETER;
                }
#endif
                SABuilderLib_CopyKeyMat(SABuffer_p,
                                        FIRMWARE_EIP207_CS_FLOW_TR_TUNNEL_SRC_WORD_OFFSET,
                                        SAParamsIPsec_p->SrcIPAddr_p,
                                        (SAParamsIPsec_p->IPsecFlags&SAB_IPSEC_IPV4)!=0?4:16);
                SABuilderLib_CopyKeyMat(SABuffer_p,
                                        FIRMWARE_EIP207_CS_FLOW_TR_TUNNEL_DST_WORD_OFFSET,
                                        SAParamsIPsec_p->DestIPAddr_p,
                                        (SAParamsIPsec_p->IPsecFlags&SAB_IPSEC_IPV4)!=0?4:16);

#ifdef FIRMWARE_EIP207_CS_FLOW_TR_CHECKSUM_WORD_OFFSET
                // checksum (only for IPv4)
                if ((SAParamsIPsec_p->IPsecFlags & SAB_IPSEC_IPV4) !=0)
                    SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_CHECKSUM_WORD_OFFSET] = CheckSum;
#endif

            }
            SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_PATH_MTU_WORD_OFFSET] =  MTUDiscount;
#endif /* SAB_ENABLE_EXTENDED_TUNNEL_HEADER */
        }
    }
    return SAB_STATUS_OK;
}


#endif /* SAB_ENABLE_IPSEC_EXTENDED */


/* end of file sa_builder_extended_ipsec.c */
