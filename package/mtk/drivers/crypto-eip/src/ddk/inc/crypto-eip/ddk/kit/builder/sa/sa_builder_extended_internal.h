/* sa_builder_extended_internal.h
 *
 * Internal API of the EIP-96 SA Builder.
 * - layout of the control words.
 * - Data structure that represents the SA builder state.
 * - Headers for shared functions and protocol-specific functions.
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


#ifndef SA_BUILDER_EXTENDED_INTERNAL_H_
#define SA_BUILDER_EXTENDED_INTERNAL_H_

#include "c_sa_builder.h"
#include "sa_builder_internal.h"
#include "firmware_eip207_api_flow_cs.h"


// In the extended use case, take the record word count from the firmware
// package instead of any local definition.
#undef SAB_RECORD_WORD_COUNT
#define SAB_RECORD_WORD_COUNT FIRMWARE_EIP207_CS_FLOW_TRC_RECORD_WORD_COUNT
#ifdef SAB_ENABLE_TWO_FIXED_RECORD_SIZES
#define SAB_RECORD_WORD_COUNT_LARGE FIRMWARE_EIP207_CS_FLOW_TRC_RECORD_WORD_COUNT_LARGE
#define SAB_LARGE_RECORD_THRESHOLD_WORD_COUNT FIRMWARE_EIP207_CS_FLOW_TR_LARGE_THRESHOLD_OFFSET
#endif

#define SAB_PACKBYTES(b0,b1,b2,b3) (((b3)<<24)|((b2)<<16)|((b1)<<8)|(b0))


/* The protocol values have to agree with those used by the EIP-207 firmware.
 */
typedef enum
{
    SAB_ESP_PROTO_NONE = 0,
    SAB_ESP_PROTO_OUT_CBC = 1,
    SAB_ESP_PROTO_OUT_NULLAUTH = 2,
    SAB_ESP_PROTO_OUT_CTR = 3,
    SAB_ESP_PROTO_OUT_CCM = 4,
    SAB_ESP_PROTO_OUT_GCM = 5,
    SAB_ESP_PROTO_OUT_GMAC = 6,
    SAB_ESP_PROTO_IN_CBC = 7,
    SAB_ESP_PROTO_IN_NULLAUTH = 8,
    SAB_ESP_PROTO_IN_CTR = 9,
    SAB_ESP_PROTO_IN_CCM = 10,
    SAB_ESP_PROTO_IN_GCM = 11,
    SAB_ESP_PROTO_IN_GMAC = 12,
    SAB_DTLS_PROTO_OUT_CBC = 13,
    SAB_DTLS_PROTO_OUT_GCM = 14,
    SAB_DTLS_PROTO_IN_CBC = 15,
    SAB_DTLS_PROTO_IN_GCM = 16,
    SAB_MACSEC_PROTO_OUT_GCM = 17,
    SAB_MACSEC_PROTO_OUT_GMAC = 18,
    SAB_MACSEC_PROTO_IN_GCM = 19,
    SAB_MACSEC_PROTO_IN_GMAC = 20,
    SAB_BASIC_PROTO_OUT_ENCHASH = 21,
    SAB_BASIC_PROTO_IN_HASHDEC = 22,
    SAB_BASIC_PROTO_OUT_HASHENC = 23,
    SAB_BASIC_PROTO_IN_DECHASH = 24,
    SAB_ESP_PROTO_OUT_CHACHAPOLY = 25,
    SAB_ESP_PROTO_IN_CHACHAPOLY = 26,
    SAB_DTLS_PROTO_OUT_CHACHAPOLY = 27,
    SAB_DTLS_PROTO_IN_CHACHAPOLY = 28,
    SAB_ESP_PROTO_OUT_XFRM_CBC = 31,
    SAB_ESP_PROTO_IN_XFRM_CBC = 32,
    SAB_ESP_PROTO_OUT_XFRM_GCM = 33,
    SAB_ESP_PROTO_IN_XFRM_GCM = 34,
} SABuilder_ESPProtocol_t;

typedef enum
{
    SAB_HDR_BYPASS = 0,
    SAB_HDR_IPV4_OUT_TRANSP_HDRBYPASS = 1,
    SAB_HDR_IPV4_OUT_TUNNEL = 2,
    SAB_HDR_IPV4_IN_TRANSP_HDRBYPASS = 3,
    SAB_HDR_IPV4_IN_TUNNEL = 4,
    SAB_HDR_IPV4_OUT_TRANSP = 5,
    SAB_HDR_IPV4_IN_TRANSP = 6,
    SAB_HDR_IPV6_OUT_TUNNEL = 7,
    SAB_HDR_IPV6_IN_TUNNEL = 8,
    SAB_HDR_IPV6_OUT_TRANSP_HDRBYPASS = 9,
    SAB_HDR_IPV6_IN_TRANSP_HDRBYPASS = 10,
    SAB_HDR_IPV6_OUT_TRANSP = 11,
    SAB_HDR_IPV6_IN_TRANSP = 12,

    /* DTLS and DLTS-CAPWAP */
    SAB_HDR_IPV4_OUT_DTLS = 13,
    SAB_HDR_IPV4_IN_DTLS = 14,
    SAB_HDR_IPV6_OUT_DTLS = 15,
    SAB_HDR_IPV6_IN_DTLS = 16,
    SAB_HDR_IPV4_OUT_DTLS_CAPWAP = 17,
    SAB_HDR_IPV4_IN_DTLS_CAPWAP = 18,
    SAB_HDR_IPV6_OUT_DTLS_CAPWAP = 19,
    SAB_HDR_IPV6_IN_DTLS_CAPWAP = 20,

    /* IPsec with NAT-T. Must be in the same order as the non NAT-T
       counterparts */
    SAB_HDR_IPV4_OUT_TRANSP_HDRBYPASS_NATT = 21,
    SAB_HDR_IPV4_OUT_TUNNEL_NATT = 22,
    SAB_HDR_IPV4_IN_TRANSP_HDRBYPASS_NATT = 23,
    SAB_HDR_IPV4_IN_TUNNEL_NATT = 24,
    SAB_HDR_IPV4_OUT_TRANSP_NATT = 25,
    SAB_HDR_IPV4_IN_TRANSP_NATT = 26,
    SAB_HDR_IPV6_OUT_TUNNEL_NATT = 27,
    SAB_HDR_IPV6_IN_TUNNEL_NATT = 28,
    SAB_HDR_IPV6_OUT_TRANSP_HDRBYPASS_NATT = 29,
    SAB_HDR_IPV6_IN_TRANSP_HDRBYPASS_NATT = 30,
    SAB_HDR_IPV6_OUT_TRANSP_NATT = 31,
    SAB_HDR_IPV6_IN_TRANSP_NATT = 32,

    /* MACsec */
    SAB_HDR_MACSEC_OUT = 33,
    SAB_HDR_MACSEC_IN = 34,

    /* BASIC */
    SAB_HDR_BASIC_OUT_ZPAD  = 35,
    SAB_HDR_BASIC_IN_NO_PAD = 36,
    SAB_HDR_BASIC_OUT_TPAD  = 37,
    SAB_HDR_BASIC_IN_PAD    = 38,

    /* ESP with XFRM API */
    SAB_HDR_IPV4_OUT_XFRM   = 39,
    SAB_HDR_IPV6_OUT_XFRM   = 40,
    SAB_HDR_IPV4_IN_XFRM   = 41,
    SAB_HDR_IPV6_IN_XFRM   = 42,

} SABuilder_HeaderProtocol_t;


/* Values to construct the Token Header Word */
#define SAB_HEADER_RC_NO_REUSE       0x00000000
#define SAB_HEADER_RC_REUSE          0x00100000
#define SAB_HEADER_RC_AUTO_REUSE     0x00200000

#define SAB_HEADER_IP                0x00020000

#define SAB_HEADER_DEFAULT           (SAB_HEADER_RC_NO_REUSE | SAB_HEADER_IP)

#define SAB_HEADER_C                 0x02000000

#define SAB_HEADER_UPD_HDR           0x00400000
#define SAB_HEADER_APP_RES           0x00800000
#define SAB_HEADER_PAD_VERIFY        0x01000000

#define SAB_HEADER_IV_DEFAULT        0x00000000
#define SAB_HEADER_IV_PRNG           0x04000000


/* Used to construct the CCM Salt */
#define SAB_CCM_FLAG_ADATA_L4        0x43


/* Used to construct the VERIFY instruction */
#define SAB_VERIFY_NONE     0xd0060000 /* No options set, use for outbound */
#define SAB_VERIFY_PAD      0xd1060000 /* Verify padding only */
#define SAB_VERIFY_PADSPI   0xd5060000 /* Verify padding and SPI */
#define SAB_VERIFY_BIT_H    0x00010000 /* Add this bit to verify hash */
#define SAB_VERIFY_BIT_SEQ  0x08000000 /* Add this bit to verify sequence number*/

/* Used to construct the CTX instruction */
/* Inbound */
#define SAB_CTX_SEQNUM      0xe0561800 /* Update context from sequence number */
#define SAB_CTX_INSEQNUM    0xe02e1800 /* Update context from sequence
                                          number, skip seqnumhi, then
                                          mask, so use on inbound without
                                          exteded sequence number */
/* Outbound */
#define SAB_CTX_OUT_SEQNUM      0xe0560800 /* Update context from sequence number */
#define SAB_CTX_OUT_INSEQNUM    0xe02e0800 /* Update context from sequence
                                              number, skip seqnumhi, then
                                              mask, so use on inbound without
                                              exteded sequence number */

#define SAB_CTX_NONE        0xe1560000 /* Use on confidentiality-only
                                          protocols, combined with unused
                                          offset in record */


/* Used to construct RETR/INS instructions for IV */
#define SA_RETR_HASH_IV0 0x42a00000
#define SA_INS_NONE_IV0  0x20a00000
#define SA_RETR_HASH_IV1 0x42a80000
#define SA_INS_NONE_IV1  0x20a80000


/* This variable contains the difference between the offsets of firmware fields
   in large transform records compared to small transform records.
   This is normally 16, but it can be larger when anti-replay masks of
   1024 or more are supported.*/
extern unsigned int LargeTransformOffset;

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
                               uint32_t * const SABuffer_p);


/*----------------------------------------------------------------------------
 * SABuilder_SetExtendedDTLSParams
 *
 * Fill in DTLS-specific extensions into the SA.for Extended.
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
SABuilder_SetExtendedDTLSParams(SABuilder_Params_t *const SAParams_p,
                                SABuilder_State_t * const SAState_p,
                                uint32_t * const SABuffer_p);


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
                                uint32_t * const SABuffer_p);

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
                                 uint32_t * const SABuffer_p);


#endif /* SA_BUILDER_EXTENDED_INTERNAL_H_ */


/* end of file sa_builder_extended_internal.h */
