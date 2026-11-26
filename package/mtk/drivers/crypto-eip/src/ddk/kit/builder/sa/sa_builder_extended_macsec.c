/* sa_builder_extended_macsec.c
 *
 * MACsec specific functions (for initialization of SABuilder_Params_t
 * structures and for building the MACsec specific part of an SA) in the
 * Extended use case.
 */

/*****************************************************************************
* Copyright (c) 2013-2020 by Rambus, Inc. and/or its subsidiaries.
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
#ifdef SAB_ENABLE_MACSEC_EXTENDED
#include "sa_builder_extended_internal.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "basic_defs.h"
#include "log.h"
#include "sa_builder_internal.h" /* SABuilder_SetMACsecParams */
#include "sa_builder_macsec.h"

/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#define SAB_MACSEC_ETHER_TYPE 0x88e5
/* Various bits in the TCI byte */
#define SAB_MACSEC_TCI_ES  BIT_6
#define SAB_MACSEC_TCI_SC  BIT_5
#define SAB_MACSEC_TCI_SCB BIT_4
#define SAB_MACSEC_TCI_E   BIT_3
#define SAB_MACSEC_TCI_C   BIT_2

/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * SABuilder_SetExtendedMACsecParams
 *
 * Fill in MACsec-specific extensions into the SA.for Extended.
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
SABuilder_SetExtendedMACsecParams(SABuilder_Params_t *const SAParams_p,
                         SABuilder_State_t * const SAState_p,
                         uint32_t * const SABuffer_p)
{
    SABuilder_Params_MACsec_t *SAParamsMACsec_p =
        (SABuilder_Params_MACsec_t *)(SAParams_p->ProtocolExtension_p);
    uint32_t TokenHeaderWord = SAB_HEADER_DEFAULT;
    SABuilder_ESPProtocol_t ESPProto;
    SABuilder_HeaderProtocol_t HeaderProto;
    uint8_t IVByteCount;
    uint8_t ICVByteCount;
    uint8_t SeqOffset;
    uint8_t TCI; /* TCI byte in SECtag */
    uint32_t flags = 0;
    uint32_t VerifyInstructionWord, CtxInstructionWord;

    IDENTIFIER_NOT_USED(SAState_p);

    if (SAParamsMACsec_p == NULL)
    {
        LOG_CRIT("SABuilder: MACsec extension pointer is null\n");
        return SAB_INVALID_PARAMETER;
    }

    SeqOffset = SAParams_p->OffsetSeqNum;
    ICVByteCount = 16;
    TCI = SAParamsMACsec_p->AN;
    if ((SAParamsMACsec_p->MACsecFlags & SAB_MACSEC_ES) != 0)
    {
        TCI |= SAB_MACSEC_TCI_ES;
    }
    if ((SAParamsMACsec_p->MACsecFlags & SAB_MACSEC_SC) != 0)
    {
        IVByteCount = 8;
        TCI |= SAB_MACSEC_TCI_SC;
    }
    else
    {
        IVByteCount = 0;
    }
    if ((SAParamsMACsec_p->MACsecFlags & SAB_MACSEC_SCB) != 0)
    {
        TCI |= SAB_MACSEC_TCI_SCB;
    }

    if (SAParams_p->AuthAlgo == SAB_AUTH_AES_GCM)
        TCI |= SAB_MACSEC_TCI_E | SAB_MACSEC_TCI_C;

    if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
    {
        HeaderProto = SAB_HDR_MACSEC_OUT;
        if (SAParams_p->AuthAlgo == SAB_AUTH_AES_GCM)
            ESPProto = SAB_MACSEC_PROTO_OUT_GCM;
        else
            ESPProto = SAB_MACSEC_PROTO_OUT_GMAC;
        VerifyInstructionWord = SAB_VERIFY_NONE;
        CtxInstructionWord = SAB_CTX_OUT_SEQNUM +
            ((unsigned int)(1<<24)) + SeqOffset;
    }
    else
    {
        HeaderProto = SAB_HDR_MACSEC_IN;
        if (SAParams_p->AuthAlgo == SAB_AUTH_AES_GCM)
            ESPProto = SAB_MACSEC_PROTO_IN_GCM;
        else
            ESPProto = SAB_MACSEC_PROTO_IN_GMAC;
        VerifyInstructionWord = SAB_VERIFY_NONE + SAB_VERIFY_BIT_H +
            SAB_VERIFY_BIT_SEQ + ICVByteCount;
        CtxInstructionWord = SAB_CTX_SEQNUM +
            ((unsigned int)(1<<24)) + SeqOffset;
    }

    /* Write all parameters to their respective offsets */
    if (SABuffer_p != NULL)
    {
        /* Do not support large transform records as Macsec will never
           use HMAC-SHA512 */
        SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_FLAGS_WORD_OFFSET] = flags;
        SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_HDRPROC_CTX_WORD_OFFSET] =
            SAParamsMACsec_p->ContextRef;
        SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_BYTE_PARAM_WORD_OFFSET] =
            SAB_PACKBYTES(IVByteCount,ICVByteCount,HeaderProto,ESPProto);
        SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TK_HDR_WORD_OFFSET] = TokenHeaderWord;
        SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_PAD_ALIGN_WORD_OFFSET] =
            SAB_PACKBYTES(SAB_MACSEC_ETHER_TYPE>>8,
                          SAB_MACSEC_ETHER_TYPE &0xff, TCI, 0);
        SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_CCM_SALT_WORD_OFFSET] =
            SAParamsMACsec_p->ConfOffset;
        SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TK_VFY_INST_WORD_OFFSET] =
                VerifyInstructionWord;
        SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TK_CTX_INST_WORD_OFFSET] =
            CtxInstructionWord;
        SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_LO_WORD_OFFSET] = 0;
        SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_HI_WORD_OFFSET] = 0;
        SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_LO_WORD_OFFSET] = 0;
        SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_HI_WORD_OFFSET] = 0;
        SABuffer_p[FIRMWARE_EIP207_CS_FLOW_TR_STAT_PKT_WORD_OFFSET] = 0;

        SABuilderLib_CopyKeyMat(SABuffer_p,
                                FIRMWARE_EIP207_CS_FLOW_TR_TUNNEL_SRC_WORD_OFFSET,
                                SAParamsMACsec_p->SCI_p, 8);
    }
    return SAB_STATUS_OK;
}

#endif /* SAB_ENABLE_MACSEC_EXTENDED */


/* end of file sa_builder_extended_dtls.c */
