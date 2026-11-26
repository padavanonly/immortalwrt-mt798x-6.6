/* sa_builder_srtp.c
 *
 * SRTP specific functions (for initialization of
 * SABuilder_Params_t structures and for building the SRTP
 * specifc part of an SA.).
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
 * This module implements (provides) the following interface(s):
 */
#include "sa_builder_srtp.h"
#include "sa_builder_internal.h" /* SABuilder_SetSSLTLSParams */

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_sa_builder.h"
#include "basic_defs.h"
#include "log.h"

#ifdef SAB_ENABLE_PROTO_SRTP
/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */

/*----------------------------------------------------------------------------
 * SABuilder_Init_SRTP
 *
 * This function initializes the SABuilder_Params_t data structure and
 * its SABuilder_Params_SRTP_t extension with sensible defaults for
 * SRTP processing..
 *
 * SAParams_p (output)
 *   Pointer to SA parameter structure to be filled in.
 * SAParamsSRTP_p (output)
 *   Pointer to SRTP parameter extension to be filled in
 * IsSRTCP (input)
 *   true if the SA is for SRTCP.
 * direction (input)
 *   Must be one of SAB_DIRECTION_INBOUND or SAB_DIRECTION_OUTBOUND.
 *
 * Tis function initializes the authentication algorithm to HMAC_SHA1.
 * The application has to fill in the appropriate keys. The crypto algorithm
 * is initialized to NULL. It can be changed to AES ICM and then a crypto
 * key has to be added as well.
 *
 * Both the SAParams_p and SAParamsSRTP_p input parameters must point
 * to valid storage where variables of the appropriate type can be
 * stored. This function initializes the link from SAParams_p to
 * SAParamsSRTP_p.
 *
 * Return:
 * SAB_STATUS_OK on success
 * SAB_INVALID_PARAMETER when one of the pointer parameters is NULL
 *   or the remaining parameters have illegal values.
 */
SABuilder_Status_t
SABuilder_Init_SRTP(
        SABuilder_Params_t * const SAParams_p,
        SABuilder_Params_SRTP_t * const SAParamsSRTP_p,
        const bool IsSRTCP,
        const SABuilder_Direction_t direction)
{
#ifdef SAB_STRICT_ARGS_CHECK
    if (SAParams_p == NULL || SAParamsSRTP_p == NULL)
    {
        LOG_CRIT("SABuilder_Init_SSLTLS: NULL pointer parameter supplied.\n");
        return SAB_INVALID_PARAMETER;
    }

    if (direction != SAB_DIRECTION_OUTBOUND &&
        direction != SAB_DIRECTION_INBOUND)
    {
        LOG_CRIT("SABuilder_Init_ESP: Invalid direction.\n");
        return SAB_INVALID_PARAMETER;
    }
#endif

    SAParams_p->protocol = SAB_PROTO_SRTP;
    SAParams_p->direction = direction;
    SAParams_p->ProtocolExtension_p = (void*)SAParamsSRTP_p;
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

    SAParams_p->AuthAlgo = SAB_AUTH_HMAC_SHA1;
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

    SAParamsSRTP_p->SRTPFlags = 0;
    if (IsSRTCP)
        SAParamsSRTP_p->SRTPFlags |= SAB_SRTP_FLAG_SRTCP;
    SAParamsSRTP_p->MKI = 0;
    SAParamsSRTP_p->ICVByteCount = 10;

    return SAB_STATUS_OK;
}

/*----------------------------------------------------------------------------
 * SABuilder_SetSRTPParams
 *
 * Fill in SRTP-specific extensions into the SA.
 *
 * SAParams_p (input)
 *   The SA parameters structure from which the SA is derived.
 * SAState_p (input, output)
 *   Variables containing information about the SA being generated.
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
SABuilder_SetSRTPParams(SABuilder_Params_t *const SAParams_p,
                        SABuilder_State_t * const SAState_p,
                        uint32_t * const SABuffer_p)
{
    SABuilder_Params_SRTP_t *SAParamsSRTP_p;
    SAParamsSRTP_p = (SABuilder_Params_SRTP_t *)
        (SAParams_p->ProtocolExtension_p);
    if (SAParamsSRTP_p == NULL)
    {
        LOG_CRIT("SABuilder: SRTP extension pointer is null\n");
        return SAB_INVALID_PARAMETER;
    }

    if (SAParams_p->CryptoAlgo == SAB_CRYPTO_AES)
    {
        SAState_p->CW1 &= ~0x7; // Clear crypto mode (CTR or ICM);
        SAState_p->CW1 |= SAB_CW1_CRYPTO_MODE_CTR_LOAD | SAB_CW1_IV_CTR;
    }
    else if (SAParams_p->CryptoAlgo != SAB_CRYPTO_NULL)
    {
        LOG_CRIT("SABuilder: I: crypto algorithm not supported\n");
        return SAB_INVALID_PARAMETER;
    }

    if (SAParams_p->AuthAlgo != SAB_AUTH_HMAC_SHA1)
    {
        LOG_CRIT("SABuilder: I: authentication algorithm not supported\n");
        return SAB_INVALID_PARAMETER;
    }

    /* Add MKI (as the SPI field) */
    if ((SAParamsSRTP_p->SRTPFlags & SAB_SRTP_FLAG_INCLUDE_MKI) != 0)
    {
        SAState_p->CW0 |= SAB_CW0_SPI;
        if (SABuffer_p != NULL)
            SABuffer_p[SAState_p->CurrentOffset] = SAParamsSRTP_p->MKI;
        SAState_p->CurrentOffset += 1;
    }

    if (SAParams_p->direction == SAB_DIRECTION_OUTBOUND)
    {
        if (SAParams_p->CryptoAlgo==SAB_CRYPTO_NULL)
            SAState_p->CW0 |= SAB_CW0_TOP_HASH_OUT;
        else
            SAState_p->CW0 |= SAB_CW0_TOP_ENCRYPT_HASH;
    }
    else
    {
        if (SAParams_p->CryptoAlgo==SAB_CRYPTO_NULL)
            SAState_p->CW0 |= SAB_CW0_TOP_HASH_IN;
        else
            SAState_p->CW0 |= SAB_CW0_TOP_HASH_DECRYPT;
    }

    return SAB_STATUS_OK;
}

#endif /* SAB_ENABLE_PROTO_SRTP */


/* end of file sa_builder_srtp.c */
