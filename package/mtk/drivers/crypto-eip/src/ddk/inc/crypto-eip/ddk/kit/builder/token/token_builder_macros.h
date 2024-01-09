/* token_builder_macros.h
 *
 * Macros to be used inside the Token Builder. Main purpose is to evaluate
 * variables that are used inside generated code.
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


#ifndef TOKEN_BUILDER_MACROS_H_
#define TOKEN_BUILDER_MACROS_H_

#include "log.h"

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/* Values to be added to Context Control Word 0 for per-packet options */
#define TKB_PERPKT_HASH_FIRST     0x00000010
#define TKB_PERPKT_HASH_NO_FINAL  0x00000020
#define TKB_PERPKT_CTR_INIT       0x00000040
#define TKB_PERPKT_ARC4_INIT      0x00000080
#define TKB_PERPKT_XTS_INIT       0x00000080

#define TKB_PERPKT_HASH_STORE     0x00000040
#define TKB_PERPKT_HASH_CMPRKEY   0x00000080

#define TKB_RESFLAG_INIPV6        BIT_0
#define TKB_RESFLAG_FROMETHER     BIT_1
#define TKB_RESFLAG_OUTIPV6       BIT_2
#define TKB_RESFLAG_INBTUNNEL     BIT_3

#define EVAL_TokenHeaderWord() (EVAL_packetsize() | \
             TokenContext_Internal_p->TokenHeaderWord | \
             (EVAL_per_packet_options() != 0?TKB_HEADER_C:0) | \
             (EVAL_u_word() != 0 ? TKB_HEADER_U:0))

#define EVAL_per_packet_options() PacketOptions(TokenContext_Internal_p,\
                                                TKBParams_p->PacketFlags, PacketByteCount - TKBParams_p->BypassByteCount)

#define EVAL_u_word() UWord(TokenContext_Internal_p, TKBParams_p)

#define EVAL_appendhash() ((unsigned)((TKBParams_p->PacketFlags &   \
                                       TKB_PACKET_FLAG_HASHAPPEND) != 0 || \
                               TokenContext_Internal_p->SeqOffset==0))

/* The following values are each a parameter, either in the Token Context,
   or in the Packet Parameters or a parameter to the token builder function */
#define EVAL_cw0() ((unsigned)TokenContext_Internal_p->u.generic.CW0)
#define EVAL_cw1() ((unsigned)TokenContext_Internal_p->u.generic.CW1)
#define EVAL_proto() ((unsigned)TokenContext_Internal_p->protocol)
#define EVAL_hproto() ((unsigned)TokenContext_Internal_p->hproto)
#define EVAL_ivlen() ((unsigned)TokenContext_Internal_p->IVByteCount)
#define EVAL_icvlen() ((unsigned)TokenContext_Internal_p->ICVByteCount)
#define EVAL_hstatelen() ((unsigned)TokenContext_Internal_p->DigestWordCount)
#define EVAL_hstatelen_bytes() ((unsigned)TokenContext_Internal_p->DigestWordCount*4)
#define EVAL_cipher_is_aes() ((unsigned)(TokenContext_Internal_p->PadBlockByteCount==16))
#define EVAL_pad_blocksize()                               \
    PadBlockSize(TokenContext_Internal_p->PadBlockByteCount, 0)
#define EVAL_pad_blocksize_out()                               \
    PadBlockSize(TokenContext_Internal_p->PadBlockByteCount,  \
                 TokenContext_Internal_p->hproto==0?TKBParams_p->AdditionalValue:0)
#define EVAL_seq_offset() ((unsigned)TokenContext_Internal_p->SeqOffset)
#define EVAL_iv_offset() ((unsigned)TokenContext_Internal_p->IVOffset)
#define EVAL_digest_offset() ((unsigned)TokenContext_Internal_p->DigestOffset)
#define EVAL_capwap_out() ((TokenContext_Internal_p->u.generic.ESPFlags & TKB_DTLS_FLAG_CAPWAP)?4:0)
#define EVAL_capwap_in() ((TokenContext_Internal_p->u.generic.ESPFlags & TKB_DTLS_FLAG_CAPWAP) && IsCAPWAP(Packet_p+bypass+hdrlen)?4:0)
#define EVAL_packetsize() ((unsigned)PacketByteCount)
#define EVAL_aadlen_pkt() (EVAL_aad()==NULL?(unsigned)TKBParams_p->AdditionalValue:0)
#define EVAL_aadlen_tkn() (EVAL_aad()==NULL?0:(unsigned)TKBParams_p->AdditionalValue)
#define EVAL_aadlen_out() (EVAL_extseq()==0?0:(unsigned)TKBParams_p->AdditionalValue)
#define EVAL_swap_j() ByteSwap32(TKBParams_p->AdditionalValue)
#define EVAL_paylen_ccm() (EVAL_packetsize()-EVAL_bypass() - \
                                        EVAL_ivlen()-EVAL_aadlen_pkt()- \
                   (EVAL_proto()==TKB_PROTO_BASIC_CCM_IN || \
                    EVAL_proto()==TKB_PROTO_BASIC_CHACHAPOLY_IN?EVAL_icvlen():0))
#define EVAL_basic_swaplen() ByteSwap32(EVAL_paylen_ccm())
#define EVAL_aadlen_swap() ByteSwap16(TKBParams_p->AdditionalValue)
#define EVAL_aadlen_swap32() ByteSwap32(TKBParams_p->AdditionalValue)
#define EVAL_aadpad() (PaddedSize(TKBParams_p->AdditionalValue + 18 ,16) - \
                       TKBParams_p->AdditionalValue - 18)
#define EVAL_aadpadpoly() (PaddedSize(TKBParams_p->AdditionalValue, 16) -  \
                           TKBParams_p->AdditionalValue)
#define EVAL_basic_hashpad() (PaddedSize( EVAL_paylen_ccm(), 16) - EVAL_paylen_ccm())

#define EVAL_bypass() ((unsigned)TKBParams_p->BypassByteCount)
#define EVAL_ivhandling() ((unsigned)TokenContext_Internal_p->IVHandling)
#define EVAL_upd_handling() ((unsigned)TokenContext_Internal_p->u.generic.UpdateHandling)
#define EVAL_extseq() ((unsigned)TokenContext_Internal_p->ExtSeq)
#define EVAL_antireplay() ((unsigned)TokenContext_Internal_p->AntiReplay)
#define EVAL_salt() ((unsigned)TokenContext_Internal_p->u.generic.CCMSalt)
#define EVAL_basic_salt() (EVAL_salt() - (TKBParams_p->AdditionalValue==0?0x40:0))
#define EVAL_swaplen() (EVAL_proto()==TKB_PROTO_SSLTLS_OUT||    \
                        EVAL_proto()==TKB_PROTO_SSLTLS_IN||     \
                        EVAL_proto()==TKB_PROTO_SSLTLS_GCM_OUT||\
                        EVAL_proto()==TKB_PROTO_SSLTLS_GCM_IN||  \
                        EVAL_proto()==TKB_PROTO_SSLTLS_CCM_OUT||\
                        EVAL_proto()==TKB_PROTO_SSLTLS_CCM_IN||  \
                        EVAL_proto()==TKB_PROTO_SSLTLS_CHACHAPOLY_OUT||\
                        EVAL_proto()==TKB_PROTO_SSLTLS_CHACHAPOLY_IN?  \
                         ByteSwap16(EVAL_paylen()):             \
                         ByteSwap32(EVAL_paylen()))
#define EVAL_swap_fraglen() ByteSwap16(EVAL_packetsize() - EVAL_bypass() + \
       EVAL_ivlen() + EVAL_icvlen() - hdrlen + \
           (unsigned int)(EVAL_upd_handling() >= TKB_UPD_IV2) * \
                                       EVAL_pad_bytes())
#define EVAL_swap_fraglen_tls13() ByteSwap16(EVAL_packetsize() - EVAL_bypass() + \
                                             EVAL_icvlen() + 1 + EVAL_count())
#define EVAL_hashpad() (PaddedSize( EVAL_paylen(), 16) - EVAL_paylen())

#define EVAL_paylen() PayloadSize(TokenContext_Internal_p, \
                                  EVAL_packetsize() - EVAL_bypass() - hdrlen, \
                                  TKBParams_p->AdditionalValue)
#define EVAL_iv() TKBParams_p->IV_p
#define EVAL_aad() TKBParams_p->AAD_p

#define EVAL_paylen_tls13_ccm_out() (EVAL_packetsize() - EVAL_bypass() + \
                                     1 + EVAL_count())
#define EVAL_paylen_tls13_ccm_in() (EVAL_packetsize() - EVAL_bypass() - \
                                    EVAL_icvlen() - 5)
#define EVAL_swaplen3() ByteSwap24(EVAL_paylen())
#define EVAL_swaplen3_tls13_out() ByteSwap24(EVAL_paylen_tls13_ccm_out())
#define EVAL_swaplen3_tls13_in() ByteSwap24(EVAL_paylen_tls13_ccm_in())
#define EVAL_hashpad_tls13_out() (PaddedSize( EVAL_paylen_tls13_ccm_out(), 16) - EVAL_paylen_tls13_ccm_out())
#define EVAL_hashpad_tls13_in() (PaddedSize( EVAL_paylen_tls13_ccm_in(), 16) - EVAL_paylen_tls13_ccm_in())


/* EVAL_pad_remainder() is used to check whether the packet payload
   on inbound packets is a multiple of the pad block size. If not,
   the packet is invalid and shall not be processed.
 */
#define EVAL_pad_remainder() PadRemainder(EVAL_packetsize() - 8 - \
                                           EVAL_ivlen() - EVAL_icvlen() - \
                                           EVAL_bypass() - hdrlen,     \
                                           EVAL_pad_blocksize())

/* EVAL_pad_bytes() is used to compute the number of bytes that must
   be added to outbound packets.

   For IPsec, two bytes (next header and the number of padded bytes)
   must be added in any case. It is the difference between the padded
   payload size and the unpadded payload size.

   For SSL and TLS, one byte (number of padded bytes) must be added in
   any case.  Padding has to be applied to the payload plus MAC.
*/
#define EVAL_pad_bytes() ComputePadBytes(TokenContext_Internal_p,\
                            EVAL_packetsize()-EVAL_bypass()-hdrlen,     \
                            EVAL_pad_blocksize_out())
#define EVAL_pad_bytes_basic() (PaddedSize(EVAL_packetsize() - EVAL_bypass() -EVAL_antireplay() - EVAL_aadlen_pkt(), \
                  TokenContext_Internal_p->PadBlockByteCount) -  \
                                EVAL_packetsize()+EVAL_bypass()+EVAL_antireplay()+EVAL_aadlen_pkt())
#define EVAL_pad_bytes_hashenc() ComputePadBytes(TokenContext_Internal_p, \
            EVAL_packetsize() - EVAL_bypass() + \
             TokenContext_Internal_p->PadBlockByteCount - EVAL_aadlen_pkt(), \
            PadBlockSize(TokenContext_Internal_p->PadBlockByteCount,TKBParams_p->PadByte + 1))


#define EVAL_srtp_iv0() (TokenContext_Internal_p->u.srtp.SaltKey0)
#define EVAL_srtp_iv1() SRTP_IV1(TokenContext_Internal_p,\
                                 Packet_p + EVAL_bypass())
#define EVAL_srtp_iv2() SRTP_IV2(TokenContext_Internal_p, \
                                 Packet_p + EVAL_bypass(), \
                                 PacketByteCount - EVAL_bypass(), \
                                 TKBParams_p->AdditionalValue)
#define EVAL_srtp_iv3() SRTP_IV3(TokenContext_Internal_p, \
                                 Packet_p + EVAL_bypass(), \
                                 PacketByteCount - EVAL_bypass(), \
                                 TKBParams_p->AdditionalValue)
#define EVAL_srtp_offset() SRTP_Offset(TokenContext_Internal_p, \
                                       Packet_p + EVAL_bypass(),  \
                                       PacketByteCount - EVAL_bypass(), \
                                       TKBParams_p->AdditionalValue)
#define EVAL_srtp_swaproc() ByteSwap32(TKBParams_p->AdditionalValue)


#define EVAL_ssltls_lastblock() (Packet_p + EVAL_packetsize() - 16 - 16*EVAL_cipher_is_aes())

#define EVAL_ssltls_lastword() (Packet_p + EVAL_packetsize() - 4)

#define EVAL_count() TKBParams_p->AdditionalValue
#define EVAL_bearer_dir_fresh() TokenContext_Internal_p->u.generic.CCMSalt

#if TKB_HAVE_PROTO_IPSEC==1|TKB_HAVE_PROTO_SSLTLS==1
/*----------------------------------------------------------------------------
 * PadRemainder
 *
 * Compute the remainder of ByteCount when devided by BlockByteCount..
 * BlockByteCount must be a power of 2.
 */
static unsigned int
PadRemainder(unsigned int ByteCount,
             unsigned int BlockByteCount)
{
    unsigned int val;
    val = ByteCount & (BlockByteCount - 1);
    LOG_INFO("TokenBuilder_BuildToken: Input message %u pad align=%u "
             "remainder=%u\n",
             ByteCount, BlockByteCount,val);
    return val;
}
#endif

#if TKB_HAVE_PROTO_IPSEC==1||TKB_HAVE_PROTO_SSLTLS==1||TKB_HAVE_PROTO_BASIC==1
/*----------------------------------------------------------------------------
 * PaddedSize
 *
 * Compute the size of a packet of length ByteCount when padded to a multiple
 * of BlockByteCount.
 *
 * BlockByteCount must be a power of 2.
 */
static unsigned int
PaddedSize(unsigned int ByteCount,
           unsigned int BlockByteCount)
{
    return (ByteCount + BlockByteCount - 1) & ~(BlockByteCount - 1);
}
#endif

#if TKB_HAVE_PROTO_IPSEC==1||TKB_HAVE_PROTO_SSLTLS==1||TKB_HAVE_PROTO_BASIC==1
/*----------------------------------------------------------------------------
 * PadBlockSize
 *
 * Select the pad block size: select the override value if it is appropriate,
 * else select the default value.
 *
 * The override value is appropriate if it is a power of two and if it is
 * greater than the default value and at most 256.
 */
static unsigned int
PadBlockSize(unsigned int DefaultVal,
             unsigned int OverrideVal)
{
    if (OverrideVal > DefaultVal  &&
        OverrideVal <= 256 &&
        (OverrideVal  & (OverrideVal - 1)) == 0) // Check power of two.
    {
        LOG_INFO("TokenBuilder_Buildtoken: Override pad alignment from %u to %u\n",
                 DefaultVal, OverrideVal);
        return OverrideVal;
    }
    else
    {
        return DefaultVal;
    }
}
#endif

#if TKB_HAVE_PROTO_SSLTLS==1
/*----------------------------------------------------------------------------
 * IsCAPWAP
 *
 * Check if there is a CAPWAP header at the given location in the packet.
 */
static inline bool
IsCAPWAP(const uint8_t * const Packet_p)
{
    return Packet_p[0]==1;
}

#endif
/*----------------------------------------------------------------------------
 * ByteSwap32
 *
 * Return a 32-bit value byte-swapped.
 */
static inline unsigned int
ByteSwap32(unsigned int val)
{
    return ((val << 24) & 0xff000000) |
           ((val << 8)  & 0x00ff0000) |
           ((val >> 8)  & 0x0000ff00) |
           ((val >> 24) & 0x000000ff);
}

#if TKB_HAVE_PROTO_IPSEC==1||TKB_HAVE_PROTO_SSLTLS==1||TKB_HAVE_PROTO_BASIC==1
/*----------------------------------------------------------------------------
 * ByteSwap16
 *
 * Return a 16-bit value byte-swapped.
 */
static inline unsigned int
ByteSwap16(unsigned int val)
{
    return ((val << 8) & 0xff00) |
           ((val >> 8) & 0x00ff);
}


/*----------------------------------------------------------------------------
 * ByteSwap24
 *
 * Return a 24-bit value byte-swapped.
 */
static inline unsigned int
ByteSwap24(unsigned int val)
{
    return ((val << 16) & 0x00ff0000) |
           ( val        & 0x0000ff00) |
           ((val >> 16) & 0x000000ff);
}

#endif

/*----------------------------------------------------------------------------
 * PacketOptions
 *
 * Return the per-packet options depending on the packet flags and the protocol.
 * In this implementaiton only the TKB_PACKET_FLAG_ARC4_INIT flag is
 * honored for the ARC4 SSL/TLS case.
 */
static inline unsigned int
PacketOptions(const TokenBuilder_Context_t * TokenContext_p,
              unsigned int flags, unsigned int PacketByteCount)
{
    unsigned int val = 0;
    unsigned int protocol = TokenContext_p->protocol;

    if ((protocol == TKB_PROTO_SSLTLS_OUT || protocol == TKB_PROTO_SSLTLS_IN ||
            protocol == TKB_PROTO_BASIC_CRYPTO) &&
        (flags & TKB_PACKET_FLAG_ARC4_INIT) != 0)
        val |= TKB_PERPKT_ARC4_INIT;

    if (protocol == TKB_PROTO_BASIC_XTS_CRYPTO &&
        (flags & TKB_PACKET_FLAG_XTS_INIT) != 0)
        val |= TKB_PERPKT_XTS_INIT;

    if (protocol == TKB_PROTO_BASIC_HASH &&
        ((TokenContext_p->SeqOffset != 0 &&
         TokenContext_p->DigestWordCount >= 16) ||
         (TokenContext_p->u.generic.CW0 & 0x07800000) == 0x07800000))
    {
        if ((flags & TKB_PACKET_FLAG_HASHFIRST) != 0)
            val |= TKB_PERPKT_HASH_FIRST;
        if ((flags & TKB_PACKET_FLAG_HASHFINAL) == 0)
            val |= TKB_PERPKT_HASH_NO_FINAL;
    }

    if (protocol == TKB_PROTO_BASIC_HMAC_PRECOMPUTE ||
        protocol == TKB_PROTO_BASIC_HMAC_CTXPREPARE)
    {
        unsigned int KeyLimit = 64;
        if (TokenContext_p->DigestWordCount==16)
            KeyLimit = 128;

        val |= TKB_PERPKT_HASH_FIRST;
        if (PacketByteCount > KeyLimit)
            val |= TKB_PERPKT_HASH_CMPRKEY;

        if (protocol == TKB_PROTO_BASIC_HMAC_CTXPREPARE)
            val |= TKB_PERPKT_HASH_STORE;
    }

    if (val != 0)
    {
        LOG_INFO("TokenBuilder_BuildToken: non-zero per-packet options: %x\n",
                 val);
    }

    return val;
}

/*----------------------------------------------------------------------------
 * UWord
 *
 * Return the U-word that is optionally present in the Token. It can be used
 * to specify non-byte aligned packet sizes in SNOW and ZUC authentication
 * algorithms or to specify the per-packet window size for inbound IPSsec ESP.
 */
static inline unsigned int
UWord(const TokenBuilder_Context_t * TokenContext_p,
      const TokenBuilder_Params_t *TKBParams_p)
{
    switch (TokenContext_p->protocol)
    {
    case TKB_PROTO_ESP_IN:
    case TKB_PROTO_ESP_CCM_IN:
    case TKB_PROTO_ESP_GCM_IN:
    case TKB_PROTO_ESP_GMAC_IN:
    case TKB_PROTO_ESP_CHACHAPOLY_IN:
        if (TokenContext_p->AntiReplay < 32)
            return 0;
        switch(TKBParams_p->AdditionalValue)
        {
        case 64:
            return 0x00200000;
        case 128:
            return 0x00400000;
        case 256:
            return 0x00600000;
        case 512:
            return 0x00800000;
        default:
            return 0;
        }
    case TKB_PROTO_BASIC_SNOW_HASH:
    case TKB_PROTO_BASIC_ZUC_HASH:
        return TKBParams_p->PadByte;
    default:
        return 0;
    }
}


#if TKB_HAVE_PROTO_IPSEC==1||TKB_HAVE_PROTO_SSLTLS==1|TKB_HAVE_PROTO_BASIC==1
/*----------------------------------------------------------------------------
 * ComputePadBytes
 *
 * TokenContext_p (input)
 *     Token context.
 * PayloadByteCount (input)
 *    Size of message to be padded in bytes.
 * PadBlockByteCount (input)
 *    Block size to which the payload must be padded.
 */
static unsigned int
ComputePadBytes(const TokenBuilder_Context_t *TokenContext_p,
                unsigned int PayloadByteCount,
                unsigned int PadBlockByteCount)
{
    unsigned int PadByteCount;
    if(TokenContext_p->protocol == TKB_PROTO_SSLTLS_OUT ||
        TokenContext_p->protocol == TKB_PROTO_BASIC_HASHENC)
    {
        PadByteCount = PaddedSize(PayloadByteCount + 1 +
                                  TokenContext_p->IVByteCount +
                                  TokenContext_p->ICVByteCount,
                                  PadBlockByteCount) -
          PayloadByteCount - TokenContext_p->ICVByteCount -
          TokenContext_p->IVByteCount;
        LOG_INFO("TokenBuilder_BuildToken: SSL/TLS padding message %u\n"
                 "  with mac %u pad bytes=%u align=%u\n",
                 PayloadByteCount,
                 TokenContext_p->ICVByteCount,
                 PadByteCount, PadBlockByteCount);
    }
    else
    {
        PadByteCount =  PaddedSize(PayloadByteCount + 2, PadBlockByteCount) -
            PayloadByteCount;
        LOG_INFO("TokenBuilder_BuildToken: IPsec padding message %u "
                 "pad bytes=%u align=%u\n",
                 PayloadByteCount, PadByteCount, PadBlockByteCount);
    }
    return PadByteCount;
}
#endif

#if TKB_HAVE_PROTO_IPSEC==1||TKB_HAVE_PROTO_SSLTLS==1
/*----------------------------------------------------------------------------
 * PayloadSize
 *
 * TokenContext_p (input)
 *     Token context.
 * MessageByteCount (input)
 *     Size of the input packet, excluding bypass.
 * AdditionalValue
 *     Additional value supplied in pacekt parameters. This may be a pad
 *     block size or the number of pad bytes.
 *
 * Compute the payload length in bytes for all SSL/TLS and for ESP with AES-CCM.
 * This is not used for other ESP modes.
 */
static unsigned int
PayloadSize(const TokenBuilder_Context_t *TokenContext_p,
            unsigned int MessageByteCount,
            unsigned int AdditionalValue)
{
    int size;
    switch(TokenContext_p->protocol)
    {
    case TKB_PROTO_ESP_CCM_OUT:
        /* Need paddded payload size, pad message */
        return PaddedSize(MessageByteCount + 2,
           PadBlockSize(TokenContext_p->PadBlockByteCount, AdditionalValue));
    case TKB_PROTO_ESP_CCM_IN:
        /* Need paddded payload size, derive from message length, subtract
           headers and trailers.*/
        return MessageByteCount - TokenContext_p->IVByteCount -
            TokenContext_p->ICVByteCount - 8;
    case TKB_PROTO_SSLTLS_OUT:
    case TKB_PROTO_SSLTLS_GCM_OUT:
    case TKB_PROTO_SSLTLS_CCM_OUT:
    case TKB_PROTO_SSLTLS_CHACHAPOLY_OUT:
        /* Need fragment length */
        return MessageByteCount;
    case TKB_PROTO_SSLTLS_IN:
    case TKB_PROTO_SSLTLS_GCM_IN:
    case TKB_PROTO_SSLTLS_CCM_IN:
    case TKB_PROTO_SSLTLS_CHACHAPOLY_IN:
        /* Need fragment length. Must remove any headers, padding and trailers
         */
        size = (int)MessageByteCount;
        if ((TokenContext_p->u.generic.ESPFlags & TKB_DTLS_FLAG_CAPWAP) != 0)
            size -= 4;
        // Deduce CAPWAP/DTLS header.
        size -= TokenContext_p->ICVByteCount;
        size -= TokenContext_p->IVByteCount;
        if (TokenContext_p->ExtSeq != 0)
            size -= 8; /* Deduce epoch and sequence number for DTLS */
        size -= 5;   /* Deduce type, version and fragment length. */
        if (size < 0)
            return 0xffffffffu;
        if(TokenContext_p->u.generic.UpdateHandling != TKB_UPD_NULL &&
           TokenContext_p->u.generic.UpdateHandling != TKB_UPD_ARC4)
        {
            if (PadRemainder((unsigned int)size + TokenContext_p->ICVByteCount,
                             TokenContext_p->PadBlockByteCount) != 0)
                /* Padded payload + ICV must be a multiple of block size */
                return 0xffffffffu;
/* Report negative payload size due to subtraction of pad bytes as 0
 * rather than 0xffffffff, so the the operation will still be
 * execcuted. This to avoid timing attacks. See RFC5246, section
 * 6.2.3.2. */
        }
        return (unsigned int)size;
    default:
        LOG_CRIT("Token Builder: PayloadSize used with unsupported protocol\n");
        return 0;
    }
}
#endif

/* SRTP_IV1

   Form second word of IV.  (Exclusive or of salt key and SSRC).
*/
static unsigned int
SRTP_IV1(
        const TokenBuilder_Context_t *TokenContext_p,
        const uint8_t *Packet_p)
{
    unsigned int SSRCOffset;
    if (TokenContext_p->ExtSeq != 0)
    { // SRTCP
        SSRCOffset = 4;
    }
    else
    { // SRTP
        SSRCOffset = 8;
    }

    if (Packet_p == NULL)
        return 0;

    return TokenContext_p->u.srtp.SaltKey1 ^ ((Packet_p[SSRCOffset]) |
                                              (Packet_p[SSRCOffset+1]<<8) |
                                              (Packet_p[SSRCOffset+2]<<16) |
                                              (Packet_p[SSRCOffset+3]<<24));
}

/* SRTP_IV2

   Form third word of IV (SRTCP: exclusive or with MSB of SRTCP index.
   SRTP: Exclusive or with ROC).
*/
static unsigned int
SRTP_IV2(
        const TokenBuilder_Context_t *TokenContext_p,
        const uint8_t *Packet_p,
        unsigned int MessageByteCount,
        unsigned int AdditionalValue)
{
    unsigned int ByteOffset;

    if (Packet_p == NULL)
        return 0;

    if (TokenContext_p->ExtSeq != 0)
    { // SRTCP
        uint32_t SRTCP_Index;
        if (TokenContext_p->protocol == TKB_PROTO_SRTP_OUT)
        {   // For outbound packet, SRTCP index is supplied parameter.
            SRTCP_Index = AdditionalValue & 0x7fffffff;
        }
        else
        {   // For inbound, extract it from the packet.
            ByteOffset = MessageByteCount - 4 - TokenContext_p->AntiReplay -
                TokenContext_p->ICVByteCount;
            SRTCP_Index = ((Packet_p[ByteOffset]&0x7f)<<24) |
                (Packet_p[ByteOffset+1]<<16) ;
        }
        return TokenContext_p->u.srtp.SaltKey2 ^ ByteSwap32(SRTCP_Index>>16);
    }
    else
    { // SRTP
        return TokenContext_p->u.srtp.SaltKey2 ^ ByteSwap32(AdditionalValue);
    }
}



/* SRTP_IV3

   Form fourth word of IV (SRTCP: exclusive or with LSB of SRTCP index.
   SRTP: Exclusive or with seq no in packet).
*/
static unsigned int
SRTP_IV3(
        const TokenBuilder_Context_t *TokenContext_p,
        const uint8_t *Packet_p,
        unsigned int MessageByteCount,
        unsigned int AdditionalValue)
{
    unsigned int ByteOffset;

    if (Packet_p == NULL)
        return 0;

    if (TokenContext_p->ExtSeq != 0)
    { // SRTCP
        uint32_t SRTCP_Index;
        if (TokenContext_p->protocol == TKB_PROTO_SRTP_OUT)
        {   // For outbound packet, SRTCP index is supplied parameter.
            SRTCP_Index = AdditionalValue;
        }
        else
        {   // For inbound, extract it from the packet.
            ByteOffset = MessageByteCount - 4 - TokenContext_p->AntiReplay -
                TokenContext_p->ICVByteCount;
            SRTCP_Index = (Packet_p[ByteOffset+2]<<8) |
                (Packet_p[ByteOffset+3]) ;
        }
        return TokenContext_p->u.srtp.SaltKey3 ^ ByteSwap32(SRTCP_Index<<16);
    }
    else
    {
        return TokenContext_p->u.srtp.SaltKey3 ^ (Packet_p[2] |
                                                  (Packet_p[3]<<8));
    }
}

#if TKB_HAVE_PROTO_SRTP==1
/* SRTP_Offset

   Compute the crypto offset (in bytes) of an SRTP packet.

   If no crypto used: return 0.
   If packet is invalid: return offset larger than message size.
*/
static unsigned int
SRTP_Offset(
        const TokenBuilder_Context_t *TokenContext_p,
        const uint8_t *Packet_p,
        unsigned int MessageByteCount,
        unsigned int AdditionalValue)
{
    unsigned int ByteOffset;
    if (TokenContext_p->IVByteCount == 0)
        return 0;

    if (Packet_p == NULL)
        return 0;

    if (TokenContext_p->ExtSeq != 0)
    { // SRTCP
        uint32_t SRTCP_Index;
        if (TokenContext_p->protocol == TKB_PROTO_SRTP_OUT)
        {   // For outbound packet, SRTCP index is supplied parameter.
            SRTCP_Index = AdditionalValue;
        }
        else
        {   // For inbound, extract it from the packet.
            ByteOffset = MessageByteCount - 4 - TokenContext_p->AntiReplay -
                TokenContext_p->ICVByteCount;
            if (ByteOffset > MessageByteCount)
            {
                LOG_INFO("TokenBuilder_BuildToken: Short packet\n");
                return MessageByteCount + 1;

            }
            SRTCP_Index = Packet_p[ByteOffset]<<24; // Only get MSB.
        }
        if ((SRTCP_Index & BIT_31) != 0) // Test the E bit.
            return 8; // SRTCP always has crypto offset 8.
        else
            return 0; // Return 0 if no encryption is used.
    }
    else
    { // SRTP
        unsigned int NCSRC = Packet_p[0] & 0xf; // Number of CSRC fields.
        ByteOffset = 12 + 4*NCSRC;
        if ( (Packet_p[0] & BIT_4) != 0) // Extension header present.
        {
            if (ByteOffset + 4 > MessageByteCount)
            {
                LOG_INFO("TokenBuilder_BuildToken: Short packet\n");
                return MessageByteCount + 1;
            }
            ByteOffset += 4 + 4*(Packet_p[ByteOffset+2]<<8) +
                4*Packet_p[ByteOffset+3]; // Add length field from extension.
        }
        return ByteOffset;
    }
}
#endif



/*-----------------------------------------------------------------------------
 * TokenBuilder_CopyBytes
 *
 * Copy a number of bytes from src to dst. src is a byte pointer, which is
 * not necessarily byte aligned. Always write a whole number of words,
 * filling the last word with null-bytes if necessary.
 */
static void
TokenBuilder_CopyBytes(uint32_t *dest,
                       const uint8_t *src,
                       unsigned int ByteCount)
{
    unsigned int i,j;
    uint32_t w;
    if (src == NULL)
    {
        LOG_CRIT("TokenBuilder trying copy data from NULL pointer\n");
        return;
    }
    for (i=0; i<ByteCount/sizeof(uint32_t); i++)
    {
        w=0;
        for (j=0; j<sizeof(uint32_t); j++)
        {
            w = (w>>8) | (*src++ << 24);
        }
        *dest++ = w;
    }
    if (ByteCount % sizeof(uint32_t) != 0)
    {
        w=0;
        for (j=0; j<ByteCount % sizeof(uint32_t); j++)
        {
            w = w | (*src++ << (8*j));
        }
        *dest++ = w;
    }
}

#if TKB_HAVE_EXTENDED_IPSEC==1 || TKB_HAVE_EXTENDED_DTLS==1


/*-----------------------------------------------------------------------------
 * TokenBuilder_ParseIPHeader
 *
 * Parse ths IP header and provide values for the following variables
 * - hdrlen (returned). Number of bytes to skip in input packet before
 *                      inserting/removing ESP header.
 * - ohdrlen            Number of bytes in output packet before
 *                      ESP header or decrypted payload.
 * - outlen             Length of output IP packet (outbound only).
 * - nh                 Next header byte in IPv6 header (if applicable)
 *                      For tunnel outbound modes (IPv4 or IPv6), it is the
 *                      DSCP+ECN byte instead.
 * - nextheader         Next header byte to insert in ESP trailer.
 * - prev_nhoffset      Offset in packet where next header byte is replaced.
 *                      For IPv6 tunnel mode, it is the flow label instead.
 * - ecn_fixup_instr    ECN fixup instruction for inbound tunnel mode.
 */
static inline uint32_t
TokenBuilder_ParseIPHeader(
        const uint8_t *Packet_p,
        const unsigned int PacketByteCount,
        const TokenBuilder_Context_t *TokenContext_Internal_p,
        TokenBuilder_Params_t * TKBParams_p,
        uint32_t *ohdrlen_p,
        uint32_t *outlen_p,
        uint32_t *nh_p,
        uint32_t *nextheader_p,
        uint32_t *prev_nhoffset_p,
        uint32_t *ecn_fixup_instr_p)
{
    uint32_t nh = 0;
    uint32_t ohdrlen = 0;
    uint32_t outlen = 0;
    uint8_t nextheader = TKBParams_p->PadByte;
    uint32_t prev_nhoffset = 0;
    uint32_t hdrlen = 0;
    uint32_t ecn_fixup_instr = 0x20000000;
    uint8_t hproto=TokenContext_Internal_p->hproto;
    bool fIPv6;
    uint32_t MaxHdrLen = TKBParams_p->Prev_NH_Offset;

    TKBParams_p->CLE = 0;
    TKBParams_p->Prev_NH_Offset = 0;
    TKBParams_p->TOS_TC_DF = 0;
    TKBParams_p->ResultFlags = 0;

    if (hproto)
    {
        /* Check whether the packet is IPv6. For outbound tunnel modes
           this is the only form of header parsing done. For non-protocol
           modes, do not look at the header at all

           Also collect the TOS bytes (DSCP+ECN) and the IPv6 flow label.*/
        if (PacketByteCount < 20)
        {
            TKBParams_p->CLE = 3;
            return 0xffffffff;
        }
        fIPv6 = (Packet_p[0] & 0xf0) == 0x60;
        TKBParams_p->ResultFlags |= fIPv6;
        if ((fIPv6 && PacketByteCount < 40) ||
            (!fIPv6 && (Packet_p[0] & 0xf0) != 0x40))
        {
            TKBParams_p->CLE = 3;
            return 0xffffffff;
        }
        if (!fIPv6)
        {
            nh = Packet_p[1]; /* DSCP+ECN byte */
            TKBParams_p->TOS_TC_DF = nh | ((Packet_p[6]&0x40)?0x100:0);
        }
        else
        {
            uint32_t w = (Packet_p[0]<<24) | (Packet_p[1]<<16) |
                (Packet_p[2]<<8) |  Packet_p[3];
            nh = (w>>20) & 0xff; /* DSCP+ECN byte */
            TKBParams_p->TOS_TC_DF = nh;
            if (TKBParams_p->PacketFlags & TKB_PACKET_FLAG_COPY_FLOWLABEL)
                prev_nhoffset = w & 0xfffff; /* Flow label */
        }
    }

    switch (hproto)
    {
    case 0:
        break;
    case TKB_HDR_IPV4_OUT_TUNNEL:
        ohdrlen = 20;
        nextheader = fIPv6 ? 41 : 4;
        break;
    case TKB_HDR_IPV6_OUT_TUNNEL:
        ohdrlen = 40;
        nextheader = fIPv6 ? 41 : 4;
        break;
    case TKB_HDR_IPV4_OUT_TUNNEL_NATT:
        ohdrlen = 28;
        nextheader = fIPv6 ? 41 : 4;
        nh = Packet_p[1]; /* DSCP+ECN byte */
        break;
    case TKB_HDR_IPV6_OUT_TUNNEL_NATT:
        ohdrlen = 48;
        nextheader = fIPv6 ? 41 : 4;
        break;
    default:
        /* Parse the IP header, sufficiently to find the location of the
           ESP header or where it should be inserted. */
        if (fIPv6)
        {
            int ChainState = 0;
            // 0 no specifics encountered.
            // 1 routing header encountered, immediately exit when
            //   destination opts header found (before parsing that header).

            if (MaxHdrLen == 0 || MaxHdrLen > PacketByteCount)
            {
                MaxHdrLen = PacketByteCount;
            }
            nh = Packet_p[6];
            prev_nhoffset = 6;
            hdrlen = 40;
            nextheader = nh;
            while (nextheader == 0 || nextheader == 43 ||
                   nextheader == 44 || nextheader == 60)
            {
                if (MaxHdrLen < hdrlen + 8) {
                    TKBParams_p->CLE = 3;
                    return 0xffffffff;
                }
                /* Assume we want to encapsulate any destination options
                   after the routine header */
                if (nextheader == 43 &&
                    (hproto == TKB_HDR_IPV6_OUT_TRANSP ||
                     hproto == TKB_HDR_IPV6_OUT_TRANSP_HDRBYPASS ||
                     hproto == TKB_HDR_IPV6_OUT_TRANSP_NATT ||
                     hproto == TKB_HDR_IPV6_OUT_TRANSP_HDRBYPASS_NATT))
                {
                    ChainState=1;
                }
                /* Fragment header encountered, these packets cannot be
                   processed.*/
                if (nextheader == 44)
                {
                    TKBParams_p->CLE = 1;
                    return 0xffffffff;
                }
                nextheader = Packet_p[hdrlen];
                prev_nhoffset = hdrlen;
                hdrlen += Packet_p[hdrlen+1] * 8 + 8;
                if ((nextheader == 60 && ChainState == 1) ||
                    ChainState == 2)
                {
                    break;
                }
            }
        }
        else
        {
            hdrlen = (Packet_p[0] & 0xf) * 4;
            nextheader = Packet_p[9];
            /* Special protocol 254 for Firmware tests, encode the
               desired nextheader byte in PktId field */
            if (nextheader == 254)
                nextheader = Packet_p[5];
            /* Check for fragment, reject fragments  */
            if ((Packet_p[6] & ~0xc0) != 0 || Packet_p[7] != 0)
            {
                    TKBParams_p->CLE = 1;
                    return 0xffffffff;
            }
        }

        switch (hproto)
        {
        case TKB_HDR_IPV4_OUT_TRANSP:
        case TKB_HDR_IPV4_OUT_TRANSP_HDRBYPASS:
            if (fIPv6)
            {
                TKBParams_p->CLE = 3;
                return 0xffffffff;
            }
            ohdrlen = hdrlen;
            break;
        case TKB_HDR_IPV6_OUT_TRANSP:
        case TKB_HDR_IPV6_OUT_TRANSP_HDRBYPASS:
            if (!fIPv6)
            {
                TKBParams_p->CLE = 3;
                return 0xffffffff;
            }
            TKBParams_p->ResultFlags |= TKB_RESFLAG_OUTIPV6;
            ohdrlen = hdrlen;
            break;
        case TKB_HDR_IPV4_IN_TRANSP:
        case TKB_HDR_IPV4_IN_TRANSP_HDRBYPASS:
            if (nextheader != 50 || fIPv6)
            {

                TKBParams_p->CLE = 3;
                return 0xffffffff;
            }
            ohdrlen = hdrlen;
            break;
        case TKB_HDR_IPV6_IN_TRANSP:
        case TKB_HDR_IPV6_IN_TRANSP_HDRBYPASS:
            if (nextheader != 50 || !fIPv6)
            {

                TKBParams_p->CLE = 3;
                return 0xffffffff;
            }
            TKBParams_p->ResultFlags |= TKB_RESFLAG_OUTIPV6;
            TKBParams_p->Prev_NH_Offset = prev_nhoffset +
                TKBParams_p->BypassByteCount ;
            ohdrlen = hdrlen;
            break;
        case TKB_HDR_IPV4_OUT_TRANSP_NATT:
        case TKB_HDR_IPV4_OUT_TRANSP_HDRBYPASS_NATT:
            if (fIPv6)
            {
                TKBParams_p->CLE = 3;
                return 0xffffffff;
            }
            ohdrlen = hdrlen + 8;
            break;
        case TKB_HDR_IPV6_OUT_TRANSP_NATT:
        case TKB_HDR_IPV6_OUT_TRANSP_HDRBYPASS_NATT:
            if (!fIPv6)
            {
                TKBParams_p->CLE = 3;
                return 0xffffffff;
            }
            TKBParams_p->ResultFlags |= TKB_RESFLAG_OUTIPV6;
            ohdrlen = hdrlen + 8;
            break;
        case TKB_HDR_IPV4_IN_TRANSP_NATT:
        case TKB_HDR_IPV4_IN_TRANSP_HDRBYPASS_NATT:
            if (nextheader != 17 || fIPv6)
            {

                TKBParams_p->CLE = 3;
                return 0xffffffff;
            }
            ohdrlen = hdrlen;
            hdrlen += 8;
            break;
        case TKB_HDR_IPV6_IN_TRANSP_NATT:
        case TKB_HDR_IPV6_IN_TRANSP_HDRBYPASS_NATT:
            if (nextheader != 17 || !fIPv6)
            {

                TKBParams_p->CLE = 3;
                return 0xffffffff;
            }
            TKBParams_p->Prev_NH_Offset = prev_nhoffset  +
                TKBParams_p->BypassByteCount;
            TKBParams_p->ResultFlags |= TKB_RESFLAG_OUTIPV6;
            ohdrlen = hdrlen;
            hdrlen += 8;
            break;
        case TKB_HDR_IPV4_IN_TUNNEL:
        case TKB_HDR_IPV6_IN_TUNNEL:
            if (nextheader != 50)
            {

                TKBParams_p->CLE = 3;
                return 0xffffffff;
            }
            if ((TKBParams_p->PacketFlags & TKB_PACKET_FLAG_KEEP_OUTER) != 0)
                ohdrlen = hdrlen;
            TKBParams_p->ResultFlags |= TKB_RESFLAG_INBTUNNEL;
            ecn_fixup_instr = 0xa6000000 +
                ((TKBParams_p->TOS_TC_DF & 0x3)<<19)  +
                ohdrlen + TKBParams_p->BypassByteCount;
            break;
        case TKB_HDR_IPV4_IN_TUNNEL_NATT:
        case TKB_HDR_IPV6_IN_TUNNEL_NATT:
            if (nextheader != 17)
            {

                TKBParams_p->CLE = 3;
                return 0xffffffff;
            }
            if ((TKBParams_p->PacketFlags & TKB_PACKET_FLAG_KEEP_OUTER) != 0)
                ohdrlen = hdrlen;
            hdrlen += 8;
            TKBParams_p->ResultFlags |= TKB_RESFLAG_INBTUNNEL;
            ecn_fixup_instr = 0xa6000000 +
                ((TKBParams_p->TOS_TC_DF & 0x3)<<19)  +
                ohdrlen + TKBParams_p->BypassByteCount;
            break;
        case TKB_HDR_IPV4_OUT_DTLS:
        case TKB_HDR_IPV6_OUT_DTLS:
            hdrlen += 8;
            ohdrlen = hdrlen;
            /* Must be UDP or UDPlite */
            if (nextheader != 17 && nextheader != 136)
            {

                TKBParams_p->CLE = 3;
                return 0xffffffff;
            }
            nh = nextheader;
            prev_nhoffset = Packet_p[6];
            nextheader = TKBParams_p->PadByte;
            if (fIPv6) TKBParams_p->ResultFlags |= TKB_RESFLAG_OUTIPV6;
            break;
        case TKB_HDR_DTLS_UDP_IN:
            hdrlen += 8;
            ohdrlen = hdrlen;
            /* Must be UDP or UDPlite */
            if (nextheader != 17 && nextheader != 136)
            {

                TKBParams_p->CLE = 3;
                return 0xffffffff;
            }
            if ((TokenContext_Internal_p->u.generic.ESPFlags &
                 TKB_DTLS_FLAG_PLAINTEXT_HDR) != 0)
            {
                ohdrlen += 5;
                if (TKBParams_p->PadByte >= 8)
                {
                    ohdrlen += (TKBParams_p->PadByte - 5);
                }
            }
            if (fIPv6) TKBParams_p->ResultFlags |= TKB_RESFLAG_OUTIPV6;
            break;
        case TKB_HDR_IPV6_OUT_TUNNEL:
        case TKB_HDR_IPV6_OUT_TUNNEL_NATT:
            TKBParams_p->ResultFlags |= TKB_RESFLAG_OUTIPV6;
            break;
        }
    }

    if (hproto == TKB_HDR_IPV4_OUT_TRANSP ||
        hproto == TKB_HDR_IPV6_OUT_TRANSP ||
        hproto == TKB_HDR_IPV4_OUT_TRANSP_NATT ||
        hproto == TKB_HDR_IPV6_OUT_TRANSP_NATT ||
        hproto == TKB_HDR_IPV4_OUT_TUNNEL ||
        hproto == TKB_HDR_IPV6_OUT_TUNNEL ||
        hproto == TKB_HDR_IPV4_OUT_TUNNEL_NATT ||
        hproto == TKB_HDR_IPV6_OUT_TUNNEL_NATT)
    {
        outlen = ohdrlen + 8 +
            TokenContext_Internal_p->IVByteCount +
            TokenContext_Internal_p->ICVByteCount +
            PaddedSize(PacketByteCount - hdrlen + 2,
                       TokenContext_Internal_p->PadBlockByteCount);
    }
    else if (hproto == TKB_HDR_IPV4_OUT_DTLS ||
             hproto == TKB_HDR_IPV6_OUT_DTLS)
    {
        if (TokenContext_Internal_p->protocol == TKB_PROTO_SSLTLS_OUT &&
            TokenContext_Internal_p->IVByteCount > 0)
            outlen = 13 + TokenContext_Internal_p->IVByteCount +
                PaddedSize(PacketByteCount - hdrlen + 1 + TokenContext_Internal_p->ICVByteCount,
                           TokenContext_Internal_p->PadBlockByteCount);
        else
            outlen = PacketByteCount - hdrlen + 13 + TokenContext_Internal_p->IVByteCount +
                TokenContext_Internal_p->ICVByteCount;
        if ((TokenContext_Internal_p->u.generic.ESPFlags & TKB_DTLS_FLAG_CAPWAP) != 0)
            outlen += 4;
    }

    *nh_p = nh;
    *ohdrlen_p = ohdrlen;
    *outlen_p = outlen;
    *nextheader_p = nextheader;
    *prev_nhoffset_p = prev_nhoffset;
    *ecn_fixup_instr_p = ecn_fixup_instr;
    return hdrlen;
}

/*-----------------------------------------------------------------------------
 * FormDSCP_ECN
 *
 * Form the DSCP/ECN byte in the tunnel header. Use the value provided in the
 * input packet, but optionally replace the DSCP field or clear the ECN field.
 */
static uint8_t
FormDSCP_ECN(
        const TokenBuilder_Context_t *TokenContext_Internal_p,
        const uint8_t DSCP_ECN_in)
{
    uint8_t DSCP_ECN_out = DSCP_ECN_in;
    if (TokenContext_Internal_p->u.generic.ESPFlags & TKB_ESP_FLAG_REPLACE_DSCP)
    {
        DSCP_ECN_out = (DSCP_ECN_out & 0x3) | \
            ((TokenContext_Internal_p->u.generic.DSCP)<<2);
    }
    if (TokenContext_Internal_p->u.generic.ESPFlags & TKB_ESP_FLAG_CLEAR_ECN)
    {
        DSCP_ECN_out &= 0xfc;
    }
    return DSCP_ECN_out;
}

/*-----------------------------------------------------------------------------
 * Form_W0_IP4
 *
 * Form the first word of the IPv4 tunnel header.
 */
static uint32_t
Form_W0_IP4(
    const TokenBuilder_Context_t *TokenContext_Internal_p,
    const uint16_t PktLen,
    const uint8_t DSCP_ECN_in)
{
    return 0x45 |
        (FormDSCP_ECN(TokenContext_Internal_p, DSCP_ECN_in) << 8) |
        (ByteSwap16(PktLen) << 16);
}

/*-----------------------------------------------------------------------------
 * Form_W1_IP4
 *
 * Form the second word of the IPv4 tunnel header. Contains DF bit.
 */
static uint32_t
Form_W1_IP4(
        const TokenBuilder_Context_t *TokenContext_Internal_p,
        const uint8_t FlagByte,
        const uint32_t PktId)
{
    bool DF = (FlagByte & 0x40) != 0;
    if (TokenContext_Internal_p->u.generic.ESPFlags & TKB_ESP_FLAG_SET_DF)
        DF=true;
    else if (TokenContext_Internal_p->u.generic.ESPFlags & TKB_ESP_FLAG_CLEAR_DF)
        DF=false;
    return (DF ? 0x00400000 : 0x00000000) |
        ((PktId >> 8) & 0xff) | ((PktId & 0xff) << 8);
}

/*-----------------------------------------------------------------------------
 * Form_W2_IP4
 *
 * Form the third word of the IPv4 tunnel header. Contains header checksum,
 * which is computed over all other fields of the header.
 */
static uint32_t
Form_W2_IP4(
    const TokenBuilder_Context_t *TokenContext_Internal_p,
    const uint32_t w0,
    const uint32_t w1)
{
    uint32_t w2;
    uint32_t sum;
    unsigned int i;
    w2 = TokenContext_Internal_p->u.generic.TTL |
        (TokenContext_Internal_p->hproto==TKB_HDR_IPV4_OUT_TUNNEL_NATT ?
         0x1100 : 0x3200);
    sum = (w0 >> 16)+(w0 & 0xffff) +
        (w1 >> 16)+(w1 & 0xffff) + w2;
    for (i=0; i<4; i++)
    {
        sum = sum + TokenContext_Internal_p->TunnelIP[2*i] +
            256 * TokenContext_Internal_p->TunnelIP[2*i + 1];
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum = (sum >> 16) + (sum & 0xffff);
    return (sum ^ 0xffff) << 16 | w2;
}


/*-----------------------------------------------------------------------------
 * Form_W0_IP6
 *
 * Form the first word of the IPv6 tunnel header.
 */
static uint32_t
Form_W0_IP6(
    const TokenBuilder_Context_t *TokenContext_Internal_p,
    const uint8_t DSCP_ECN_in,
    const uint32_t FlowLabel)
{
    return ByteSwap32(0x60000000 |
            (FormDSCP_ECN(TokenContext_Internal_p,DSCP_ECN_in) << 20) |
             FlowLabel);
}

/*-----------------------------------------------------------------------------
 * Form_W1_IP6
 *
 * Form the second word of the IPv6 tunnel header.
 */
static uint32_t
Form_W1_IP6(
    const TokenBuilder_Context_t *TokenContext_Internal_p,
    const uint16_t PktLen)
{
    return ByteSwap16(PktLen - 40) |
        (TokenContext_Internal_p->u.generic.TTL << 24) |
        (TokenContext_Internal_p->hproto==TKB_HDR_IPV6_OUT_TUNNEL_NATT ?
         0x110000 : 0x320000);
}


#define EVAL_prev_nhoffset() prev_nhoffset
#define EVAL_hdrlen() TokenBuilder_ParseIPHeader(Packet_p + bypass, \
                                                 PacketByteCount - bypass, \
                                                 TokenContext_Internal_p, \
                                                 TKBParams_p, \
                                                 &ohdrlen, \
                                                 &outlen, \
                                                 &nh, \
                                                 &nextheader, \
                                                 &prev_nhoffset, \
                                                 &ecn_fixup_instr)
#define EVAL_ohdrlen() ohdrlen
#define EVAL_outlen() outlen
#define EVAL_nh() nh
#define EVAL_ecn_fixup_instr() ecn_fixup_instr
#define EVAL_outer_ecn()  (TKBParams_p->TOS_TC_DF & 0x3)
#define EVAL_tunnel_w0_ip4() Form_W0_IP4(TokenContext_Internal_p, \
                                         outlen, nh)
#define EVAL_tunnel_w1_ip4() Form_W1_IP4(TokenContext_Internal_p, \
                                         nextheader==4?Packet_p[bypass+6]:0, \
                                         TKBParams_p->AdditionalValue)
#define EVAL_tunnel_w2_ip4() Form_W2_IP4(TokenContext_Internal_p, \
                                         tunnel_w0_ip4, tunnel_w1_ip4)
#define EVAL_tunnel_w0_ip6() Form_W0_IP6(TokenContext_Internal_p, nh,\
                                         prev_nhoffset)
#define EVAL_tunnel_w1_ip6() Form_W1_IP6(TokenContext_Internal_p, outlen)

#define EVAL_ports_natt() TokenContext_Internal_p->NATT_Ports
#define EVAL_nextheader() nextheader
#define EVAL_tunnel_ip_addr() TokenContext_Internal_p->TunnelIP
#define EVAL_dst_ip_addr() (TokenContext_Internal_p->TunnelIP + 4)
#define EVAL_is_nat() ((TokenContext_Internal_p->u.generic.ESPFlags & TKB_ESP_FLAG_NAT) != 0)
#else
#define EVAL_hdrlen() 0
#define EVAL_ohdrlen() 0
#define EVAL_nextheader() ((unsigned)TKBParams_p->PadByte)
#define EVAL_ecn_fixup_instr() 0x20000000
#endif

/*-----------------------------------------------------------------------------
 * Switch_Proto
 *
 * Switch the Token Context back from HMAC precompute to the original
 * protocol.
 *
 * Return true if protocol was switched.
 */
static inline bool
Switch_Proto(TokenBuilder_Context_t *ctx)
{
    if(ctx->protocol == TKB_PROTO_BASIC_HMAC_CTXPREPARE)
    {
        ctx->protocol = ctx->protocol_next;
        ctx->hproto = ctx->hproto_next;
        ctx->IVHandling = ctx->IVHandling_next;
        ctx->TokenHeaderWord |= ctx->HeaderWordFields_next << 22;
        return true;
    }
    else
    {
        return false;
    }
}

#endif /* TOKEN_BUILDER_MACROS_H_ */

/* end of file token_builder_macros.h */
