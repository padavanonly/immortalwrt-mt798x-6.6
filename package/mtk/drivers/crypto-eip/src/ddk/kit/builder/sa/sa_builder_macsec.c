/* sa_builder_macsec.c
 *
 * MACsec specific functions (for initialization of SABuilder_Params_t
 * structures and for building the MACsec specifc part of an SA.).
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
#include "sa_builder_macsec.h"
#include "sa_builder_internal.h" /* SABuilder_SetMACsecParams */

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_sa_builder.h"
#include "basic_defs.h"
#include "log.h"

#ifdef SAB_ENABLE_PROTO_MACSEC

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */

/*----------------------------------------------------------------------------
 * SABuilder_Init_MACsec
 *
 * This function initializes the SABuilder_Params_t data structure and its
 * SABuilder_Params_MACsec_t extension with sensible defaults for MACsec
 * processing.
 *
 * SAParams_p (output)
 *   Pointer to SA parameter structure to be filled in.
 * SAParamsMACsec_p (output)
 *   Pointer to MACsec parameter extension to be filled in
 * SCI_p (input)
 *   Pointer to Secure Channel Identifier, 8 bytes.
 * AN (input)
 *   Association number, a number for 0 to 3.
 * direction (input)
 *   Must be one of SAB_DIRECTION_INBOUND or SAB_DIRECTION_OUTBOUND.
 *
 * Both the crypto and the authentication algorithm are initialized to
 * NULL. The crypto algorithm (which may remain NULL) must be set to
 * one of the algorithms supported by the protocol. The authentication
 * algorithm must also be set to one of the algorithms supported by
 * the protocol..Any required keys have to be specified as well.
 *
 * Both the SAParams_p and SAParamsMACsec_p input parameters must point
 * to valid storage where variables of the appropriate type can be
 * stored. This function initializes the link from SAParams_p to
 * SAParamsMACsec_p.
 *
 * Return:
 * SAB_STATUS_OK on success
 * SAB_INVALID_PARAMETER when one of the pointer parameters is NULL
 *   or the remaining parameters have illegal values.
 */
SABuilder_Status_t
SABuilder_Init_MACsec(
    SABuilder_Params_t * const SAParams_p,
    SABuilder_Params_MACsec_t * const SAParamsMACsec_p,
    const uint8_t *SCI_p,
    const uint8_t AN,
    const SABuilder_Direction_t direction)
{
#ifdef SAB_STRICT_ARGS_CHECK
    if (SAParams_p == NULL || SAParamsMACsec_p == NULL || SCI_p == NULL)
    {
        LOG_CRIT("SABuilder_Init_MACsec: NULL pointer parameter supplied.\n");
        return SAB_INVALID_PARAMETER;
    }

    if (AN > 3)
    {
        LOG_CRIT("SABuilder_Init_MACsec: Invalid Association Number.\n");
        return SAB_INVALID_PARAMETER;
    }

    if (direction != SAB_DIRECTION_OUTBOUND &&
        direction != SAB_DIRECTION_INBOUND)
    {
        LOG_CRIT("SABuilder_Init_ESP: Invalid direction.\n");
        return SAB_INVALID_PARAMETER;
    }
#endif

    SAParams_p->protocol = SAB_PROTO_MACSEC;
    SAParams_p->direction = direction;
    SAParams_p->ProtocolExtension_p = (void*)SAParamsMACsec_p;
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

    SAParamsMACsec_p->MACsecFlags = 0;
    SAParamsMACsec_p->SCI_p = SCI_p;
    SAParamsMACsec_p->AN = AN;
    SAParamsMACsec_p->SeqNum = 0;
    SAParamsMACsec_p->ReplayWindow = 0;
    SAParamsMACsec_p->ConfOffset = 0;
    SAParamsMACsec_p->ContextRef = 0;

    return SAB_STATUS_OK;
}

/*----------------------------------------------------------------------------
 * SABuilder_SetMACsecParams
 *
 * Fill in MACsec-specific extensions into the SA.
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
SABuilder_SetMACsecParams(SABuilder_Params_t *const SAParams_p,
                          SABuilder_State_t * const SAState_p,
                          uint32_t * const SABuffer_p)
{
    SABuilder_Params_MACsec_t *SAParamsMACsec_p;
    SAParamsMACsec_p = (SABuilder_Params_MACsec_t *)
        (SAParams_p->ProtocolExtension_p);
    if (SAParamsMACsec_p == NULL)
    {
        LOG_CRIT("SABuilder: MACsec extension pointer is null\n");
        return SAB_INVALID_PARAMETER;
    }

    /* Allow only AES-GMAC and AES-GCM */
    if (SAParams_p->AuthAlgo != SAB_AUTH_AES_GCM &&
        SAParams_p->AuthAlgo != SAB_AUTH_AES_GMAC)
    {
        LOG_CRIT("SABuilder: Only AES-GCM and GMAC allowed wtih MACsec\n");
        return SAB_INVALID_PARAMETER;
    }

    if ( (SAParamsMACsec_p->MACsecFlags & SAB_MACSEC_ES) != 0 &&
         (SAParamsMACsec_p->MACsecFlags & SAB_MACSEC_SC) != 0)
    {
        LOG_CRIT("SABuilder: MACSEC if ES is set, then SC must be zero,\n");
        return SAB_INVALID_PARAMETER;
    }

    /* Add sequence number */
    SAState_p->CW0 |= SAB_CW0_SEQNUM_32;
    SAParams_p->OffsetSeqNum = SAState_p->CurrentOffset;
    SAParams_p->SeqNumWord32Count = 1;
    SAState_p->CW1 |= SAB_CW1_SEQNUM_STORE;

    if (SABuffer_p != NULL)
        SABuffer_p[SAState_p->CurrentOffset] = SAParamsMACsec_p->SeqNum;
    SAState_p->CurrentOffset += 1;

    if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
    {
        if (SAParams_p->CryptoMode == SAB_CRYPTO_MODE_GCM)
            SAState_p->CW0 |= SAB_CW0_TOP_ENCRYPT_HASH;
        else
            SAState_p->CW0 |= SAB_CW0_TOP_HASH_ENCRYPT;
        /* Some versions of the hardware can update the sequence number
           early, so multiple engines can operate in parallel. */
        SAState_p->CW1 |= SAB_CW1_EARLY_SEQNUM_UPDATE;
        SAState_p->CW1 |= SAParams_p->OffsetSeqNum << 24;
    }
    else
    {
        SAState_p->CW0 |= SAB_CW0_TOP_HASH_DECRYPT;

        /* Add 'sequence number mask' parameter, which is the replay
           window size */
        SAParams_p->OffsetSeqMask = SAState_p->CurrentOffset;
        if(SABuffer_p != NULL)
        {
            SABuffer_p[SAState_p->CurrentOffset] =
                SAParamsMACsec_p->ReplayWindow;
            SABuffer_p[SAState_p->CurrentOffset+1] = 0; // Add dummy mask word.
        }
        SAParams_p->SeqMaskWord32Count = 1;
        SAState_p->CurrentOffset += 2;
        SAState_p->CW0 |= SAB_CW0_MASK_32;
        SAState_p->CW1 |= SAB_CW1_MACSEC_SEQCHECK|SAB_CW1_NO_MASK_UPDATE;
    }

    /* Add SCI (IV0 and IV1) */
    SAState_p->CW1 |= SAB_CW1_IV_CTR | SAB_CW1_IV0 | SAB_CW1_IV1 | SAB_CW1_IV2;
#ifdef SAB_STRICT_ARGS_CHECK
    if (SAParamsMACsec_p->SCI_p == NULL)
    {
        LOG_CRIT("SABuilder: NULL pointer SCI.\n");
        return SAB_INVALID_PARAMETER;
    }
#endif
    SAParams_p->OffsetIV = SAState_p->CurrentOffset;
    SAParams_p->IVWord32Count = 2;

    SABuilderLib_CopyKeyMat(SABuffer_p, SAState_p->CurrentOffset,
                            SAParamsMACsec_p->SCI_p, 8);
    SAState_p->CurrentOffset += 2;

    /* Add sequence number once more (IV2) */
    if (SABuffer_p != NULL)
        SABuffer_p[SAState_p->CurrentOffset] = SAParamsMACsec_p->SeqNum;
    SAState_p->CurrentOffset += 1;

    return SAB_STATUS_OK;
}


#endif /* SAB_ENABLE_PROTO_MACSEC */

/* end of file sa_builder_macsec.c */
