/* sa_builder_params_ipsec.h
 *
 * IPsec specific extension to the SABuilder_Params_t type.
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


#ifndef SA_BUILDER_PARAMS_IPSEC_H_
#define SA_BUILDER_PARAMS_IPSEC_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "sa_builder_params.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"

#define SA_SEQ_MASK_WORD_COUNT 12 /* Maximum number of words in sequence mask */

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Flag bits for the IPsecFlags field. Combine any values using a
   bitwise or.
   Of SAB_IPSEC_ESP and SAB_AH, exactly one must be set.
   Of SAB_IPSEC_TUNNEL and SAB_IPSEC_TRANSPORT, exactly one must be set.
   Of SAB_IPSEC_IPV4 and SAB_IPSEC_IPV6, exactly one must be set.
*/

#define SAB_IPSEC_ESP             BIT_0
#define SAB_IPSEC_AH              BIT_1

#define SAB_IPSEC_TUNNEL          BIT_2
#define SAB_IPSEC_TRANSPORT       BIT_3

#define SAB_IPSEC_IPV4            BIT_4
#define SAB_IPSEC_IPV6            BIT_5

#define SAB_IPSEC_LONG_SEQ        BIT_6 /* Use 64-bit extended seq. number */
#define SAB_IPSEC_NO_ANTI_REPLAY  BIT_7 /* Disable anti-replay protection */
#define SAB_IPSEC_MASK_128        BIT_8 /* Use 128-bit anti-replay mask instead of 64-bit (for downward compatibility) */
#define SAB_IPSEC_MASK_32         BIT_9 /* Use 32-bit anti-replay mask instead of 64-bit (for downward compatibility) */

#define SAB_IPSEC_PROCESS_IP_HEADERS BIT_10 /* Perform header processing */
#define SAB_IPSEC_CLEAR_DF        BIT_11 /* Clear DF on outer tunnel header */
#define SAB_IPSEC_SET_DF          BIT_12 /* Set DF on outer tunnel header */
#define SAB_IPSEC_NATT            BIT_13 /* Encapsulate ESP in UDP for nat traversal */
#define SAB_IPSEC_REPLACE_DSCP    BIT_14 /* Copy DSCP from transform record */
#define SAB_IPSEC_MASK_384        BIT_15 /* Use 384-bit anti-replay mask instead of 64-bit (for downward compatibility) */
#define SAB_IPSEC_APPEND_SEQNUM   BIT_16 /* Cause the hardware to append sequence number to output */
#define SAB_IPSEC_CLEAR_ECN       BIT_17 /* Clear ECN bits in tunnel header for compatibility mode */
#define SAB_IPSEC_MASK_256        BIT_18 /* Use 256-bit anti-replay mask instead of 64-bit (for downward compatibility) */
#define SAB_IPSEC_FIXED_SEQ_OFFSET BIT_19 /* Use fixed sequence number offset
                                             for 64 or 128 bit masks. */
#define SAB_IPSEC_EXT_PROCESSING  BIT_20 /* Extended processing for IPsec
                                            in stand-alone token builder */
#define SAB_IPSEC_TRANSPORT_NAT     BIT_21 /* Use additional NAT with transport NATT */
#define SAB_IPSEC_CHECKSUM_FIX      BIT_22 /* Fix checksum for inbound transport NAT-T */
#define SAB_IPSEC_DEC_TTL           BIT_23 /* Decrement TTL/hop limit field */

#define SAB_IPSEC_XFRM_API          BIT_24 /* Use this transform with Linux kernel XFRM API */

/* Extension record for SAParams_t. Protocol_Extension_p must point
   to this structure when the IPsec protocol is used.

   SABuilder_Iinit_ESP() will fill all fields in this structure  with
   sensible defaults.
 */
typedef struct
{
    uint32_t spi;
    uint32_t IPsecFlags;    /* See SAB_IPSEC_* flag bits above */

    uint32_t SeqNum;       /* Initialize with zero */
    uint32_t SeqNumHi;     /* Only valid if SAB_IPSEC_LONG_SEQ is set */
    uint32_t SeqMask[SA_SEQ_MASK_WORD_COUNT];
                           /* Mask window Only used with inbound operations.
                              By default, set first word to 1, all others to 0.

                              The mask can be programmed to resume an existing
                              SA operation, This field is unused for
                              mask sizes that cannot be accommodated in
                              this array. For these, only the default mask is
                              possible,
                           */
    uint32_t PadAlignment; /* Align padding to specified multiple of bytes.
                              This must be a power of two between 4 and 256.
                              If zero, default pad alignment is used.*/
    uint32_t ICVByteCount; /* Length of ICV in bytes. If left zero, a default
                              value is used, compatible with the authentication
                              algorithm, */
    uint8_t *SrcIPAddr_p;  /* Source IP address for tunnel header.
                                 4 bytes for IPv4, 16 bytes for IPv6
                              Also used as the translate-to address for NAT.*/
    uint8_t *DestIPAddr_p; /* Destination IP address for tunnel header.
                              4 bytes for IPv4, 16 bytes for IPv6
                              Also used as the translate-to address for NAT.*/
    uint8_t *OrigSrcIPAddr_p; /* Original NAT source address (translate-from)
                                 used in checksum delta calculations */
    uint8_t *OrigDestIPAddr_p; /* Original NAT destination address (translate-from)
                                 used in checksum delta calculations */
    uint16_t NATTSrcPort;  /* UDP source port when using NAT-T */
    uint16_t NATTDestPort; /* UDP destination port when using NAT-T */
    uint32_t ContextRef; /* Reference to application context */
    uint8_t  TTL;        /* Time-to-live/Hop Limit in outer header */
    uint8_t  DSCP;       /* DSCP/traffic class field in outer header
                            if copied from SA */
    uint16_t SequenceMaskBitCount; /* Number of bits in sequence number mask.
                                      Default is 64 */
} SABuilder_Params_IPsec_t;


#endif /* SA_BUILDER_PARAMS_IPSEC_H_ */


/* end of file sa_builder_params_ipsec.h */
