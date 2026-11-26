/* iotoken.c
 *
 * IOToken API implementation for the EIP-197 Server with ICE only
 *
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

// Extended IOToken API
#include "iotoken_ext.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_iotoken.h"              // IOTOKEN_STRICT_ARGS

// Driver Framework Basic Definitions API
#include "basic_defs.h"             // IDENTIFIER_NOT_USED, bool, uint32_t

// Firmware packet flow codes
#include "firmware_eip207_api_cmd.h"

// ZEROINIT
#include "clib.h"

#include "log.h"

// Packet buffer access
#include "adapter_pec_pktbuf.h"

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define IOTOKEN_ARGUMENT_ERROR      -1
#define IOTOKEN_INTERNAL_ERROR      -2

#ifdef IOTOKEN_STRICT_ARGS
#define IOTOKEN_CHECK_POINTER(_p) \
    if (NULL == (_p)) \
        return IOTOKEN_ARGUMENT_ERROR;
#define IOTOKEN_CHECK_INT_INRANGE(_i, _min, _max) \
    if ((_i) < (_min) || (_i) > (_max)) \
        return IOTOKEN_ARGUMENT_ERROR;
#define IOTOKEN_CHECK_INT_ATLEAST(_i, _min) \
    if ((_i) < (_min)) \
        return IOTOKEN_ARGUMENT_ERROR;
#define IOTOKEN_CHECK_INT_ATMOST(_i, _max) \
    if ((_i) > (_max)) \
        return IOTOKEN_ARGUMENT_ERROR;
#else
/* IOTOKEN_STRICT_ARGS undefined */
#define IOTOKEN_CHECK_POINTER(_p)
#define IOTOKEN_CHECK_INT_INRANGE(_i, _min, _max)
#define IOTOKEN_CHECK_INT_ATLEAST(_i, _min)
#define IOTOKEN_CHECK_INT_ATMOST(_i, _max)
#endif /*end of IOTOKEN_STRICT_ARGS */

// Input Token words offsets
#define IOTOKEN_HDR_IN_WORD_OFFS           0
#define IOTOKEN_APP_ID_IN_WORD_OFFS        1
#define IOTOKEN_SA_ADDR_LO_IN_WORD_OFFS    2
#define IOTOKEN_SA_ADDR_HI_IN_WORD_OFFS    3
#define IOTOKEN_HW_SERVICES_IN_WORD_OFFS   4
#define IOTOKEN_NH_OFFSET_IN_WORD_OFFS     5
#define IOTOKEN_BP_DATA_IN_WORD_OFFS       6

// Output Token words offsets
#define IOTOKEN_HDR_OUT_WORD_OFFS          0
#define IOTOKEN_BP_LEN_OUT_WORD_OFFS       1
#define IOTOKEN_APP_ID_OUT_WORD_OFFS       2
#define IOTOKEN_PAD_NH_OUT_WORD_OFFS       3
#define IOTOKEN_SA_ADDR_LO_OUT_WORD_OFFS   4
#define IOTOKEN_SA_ADDR_HI_OUT_WORD_OFFS   5
#define IOTOKEN_NPH_CTX_OUT_WORD_OFFS      6
#define IOTOKEN_PREV_NH_OFFS_OUT_WORD_OFFS 7
#define IOTOKEN_BP_DATA_OUT_WORD_OFFS      8

#define IOTOKEN_MARK                       0xEC00


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * IOToken_InWordCount_Get
 */
unsigned int
IOToken_InWordCount_Get(void)
{
    return IOTOKEN_IN_WORD_COUNT - 4 + IOTOKEN_BYPASS_WORD_COUNT;
}


/*----------------------------------------------------------------------------
 * IOToken_OutWordCount_Get
 */
unsigned int
IOToken_OutWordCount_Get(void)
{
    return IOTOKEN_OUT_WORD_COUNT - 4 + IOTOKEN_BYPASS_WORD_COUNT;
}


/*----------------------------------------------------------------------------
 * IOToken_Create
 */
int
IOToken_Create(
        const IOToken_Input_Dscr_t * const Dscr_p,
        uint32_t * Data_p)
{
    IOToken_Input_Dscr_Ext_t * DscrExt_p;
#if IOTOKEN_BYPASS_WORD_COUNT > 0
    unsigned int bypass_gap = 0;
#endif
    unsigned int i;

    IOTOKEN_CHECK_POINTER(Dscr_p);
    IOTOKEN_CHECK_POINTER(Dscr_p->Ext_p);
    IOTOKEN_CHECK_POINTER(Data_p);

    DscrExt_p = (IOToken_Input_Dscr_Ext_t *)Dscr_p->Ext_p;

    // Input Token word: EIP-96 Token Header Word
    {
        i = IOTOKEN_HDR_IN_WORD_OFFS;

        // Set initialization value in the Token Header Word excluding packet
        // size field
        Data_p[i] = Dscr_p->TknHdrWordInit & ~MASK_16_BITS;

        // Input packet size
        Data_p[i] |= Dscr_p->InPacket_ByteCount & MASK_16_BITS;

        if (DscrExt_p->HW_Services == IOTOKEN_CMD_PKT_LAC)
        {
            // Options, ARC4 pre-fetch
            if (DscrExt_p->fARC4Prefetch)
                Data_p[i] |= BIT_16;

            Data_p[i] |= BIT_17; // Set token header format to EIP-(1)97

            Data_p[i] |= BIT_18; // Only 64-bit Context (SA) pointer is supported

            // Enable Context Reuse auto detect if no new SA
            if (Dscr_p->Options.fReuseSA)
                Data_p[i] |= BIT_21;
        }
        // Mask size for inbound ESP
        if (DscrExt_p->SequenceMaskBitCount != 0)
        {
            switch(DscrExt_p->SequenceMaskBitCount)
            {
            case 64:
                Data_p[i] |= 0x00200000;
                break;
            case 128:
                Data_p[i] |= 0x00400000;
                break;
            case 256:
                Data_p[i] |= 0x00600000;
                break;
            case 512:
                Data_p[i] |= 0x00800000;
                break;
            }
        }
        // Extended options for LIP
        if (DscrExt_p->fEncLastDest)
            Data_p[i] |= BIT_25;
        // Extended options for in-line DTLS packet flow
        {
            if (DscrExt_p->Options.fCAPWAP)
                Data_p[i] |= BIT_26;

            if (DscrExt_p->Options.fInbound)
                Data_p[i] |= BIT_27;

            Data_p[i] |= DscrExt_p->Options.ContentType << 28;
        }

        Data_p[i] |= IOTOKEN_FLOW_TYPE << 30; // Implemented flow type
    }

    // Input Token word: Application ID
    {
        i = IOTOKEN_APP_ID_IN_WORD_OFFS;
        Data_p[i] = (Dscr_p->AppID & MASK_7_BITS) << 9;
        Data_p[i] |= (DscrExt_p->AAD_ByteCount & MASK_8_BITS);
        if (DscrExt_p->fInline)
            Data_p[i] |= (32 + IOTOKEN_BYPASS_WORD_COUNT*4) << 16;
    }

    // Input Token word: Context (SA) low 32 bits physical address
    {
        i = IOTOKEN_SA_ADDR_LO_IN_WORD_OFFS;
        Data_p[i] = Dscr_p->SA_PhysAddr.Lo;
    }

    // Input Token word: Context (SA) high 32 bits physical address
    {
        i = IOTOKEN_SA_ADDR_HI_IN_WORD_OFFS;
        //if (DscrExt_p->Options.f64bitSA) // Only 64-bit SA address supported
            Data_p[i] = Dscr_p->SA_PhysAddr.Hi;
    }

    // Input Token word: HW services, e,g, packet flow selection
    {
        i = IOTOKEN_HW_SERVICES_IN_WORD_OFFS;
        Data_p[i] = ((DscrExt_p->HW_Services & MASK_8_BITS) << 24) |
            (DscrExt_p->UserDef & MASK_16_BITS);
        if (DscrExt_p->fStripPadding != IOTOKEN_PADDING_DEFAULT_ON)
            Data_p[i] |= BIT_22;
        if (DscrExt_p->fAllowPadding != IOTOKEN_PADDING_DEFAULT_ON)
            Data_p[i] |= BIT_23;
    }

    // Input Token word: Offset and Next Header
    {
        i = IOTOKEN_NH_OFFSET_IN_WORD_OFFS;
        Data_p[i] = (DscrExt_p->Offset_ByteCount & MASK_8_BITS) << 8;
        Data_p[i] |= (DscrExt_p->NextHeader & MASK_8_BITS) << 16;
        if (DscrExt_p->fFL)
            Data_p[i] |= BIT_24;
        if (DscrExt_p->fIPv4Chksum)
            Data_p[i] |= BIT_25;
        if (DscrExt_p->fL4Chksum)
            Data_p[i] |= BIT_26;
        if (DscrExt_p->fParseEther)
            Data_p[i] |= BIT_27;
        if (DscrExt_p->fKeepOuter)
            Data_p[i] |= BIT_28;

    }

    if (DscrExt_p->fInline)
    {
        Data_p[IOTOKEN_BP_DATA_IN_WORD_OFFS]=0;
        Data_p[IOTOKEN_BP_DATA_IN_WORD_OFFS+1]=0;
#if IOTOKEN_BYPASS_WORD_COUNT > 0
        bypass_gap = 2;
#endif
    }

#if IOTOKEN_BYPASS_WORD_COUNT > 0
    // Input Token word: Bypass Data
    {
        unsigned int j;

        if (DscrExt_p->BypassData_p)
        {
            for (j = 0; j < IOTOKEN_BYPASS_WORD_COUNT; j++)
                Data_p[j+IOTOKEN_BP_DATA_IN_WORD_OFFS + bypass_gap] = DscrExt_p->BypassData_p[j];
        }
        else
        {
            for (j = 0; j < IOTOKEN_BYPASS_WORD_COUNT; j++)
                Data_p[j+IOTOKEN_BP_DATA_IN_WORD_OFFS + bypass_gap] = 0xf0f0f0f0;
        }
        i += bypass_gap + j;
    }
#endif

    if (i  >= IOTOKEN_IN_WORD_COUNT_IL)
        return IOTOKEN_INTERNAL_ERROR;
    else
        return i+1;
}


/*----------------------------------------------------------------------------
 * IOToken_Fixup
 */
int
IOToken_Fixup(
        uint32_t * const Data_p,
        const DMABuf_Handle_t PacketHandle)
{
    uint32_t AppendFlags;
    if (Data_p == NULL)
        return 0;

    AppendFlags = Data_p[IOTOKEN_BP_LEN_OUT_WORD_OFFS] & (BIT_31|BIT_30|BIT_29);
    if (AppendFlags != 0)
    {
        uint8_t AppendData[12];
        uint8_t *Append_p;
        unsigned int Offset;
        unsigned int PrevNH_Offset;
        unsigned int UpdateOffset;
        unsigned int AppendLen = 0;
        if ((AppendFlags & BIT_31) != 0)
            AppendLen+=4;
        if ((AppendFlags & BIT_30) != 0)
            AppendLen+=4;
        if ((AppendFlags & BIT_29) != 0)
            AppendLen+=4;
        UpdateOffset = Data_p[IOTOKEN_HDR_OUT_WORD_OFFS] & MASK_17_BITS;
        UpdateOffset = 4*((UpdateOffset + 3)/4); // Align to word boundary.
        Offset = Data_p[IOTOKEN_PAD_NH_OUT_WORD_OFFS] >> 16 & MASK_8_BITS;
        // Get first byte of header
        Append_p = Adapter_PEC_PktData_Get(PacketHandle, AppendData, UpdateOffset, AppendLen);
        if (AppendFlags == (BIT_31|BIT_30))
        { // IPv6 header update, NH plus length
            PrevNH_Offset = Data_p[IOTOKEN_PREV_NH_OFFS_OUT_WORD_OFFS] & MASK_16_BITS;
            Adapter_PEC_PktByte_Put(PacketHandle, PrevNH_Offset, Append_p[4]);
            Adapter_PEC_PktByte_Put(PacketHandle, Offset + 4, Append_p[0]);
            Adapter_PEC_PktByte_Put(PacketHandle, Offset + 5, Append_p[1]);
            LOG_INFO("IOToken_Fixup: IPv6 inbound transport\n");
        }
        else if (AppendFlags == BIT_29)
        { // IPv4 checksum only update
            Adapter_PEC_PktByte_Put(PacketHandle, Offset + 10, Append_p[0]);
            Adapter_PEC_PktByte_Put(PacketHandle, Offset + 11, Append_p[1]);
            LOG_INFO("IOToken_Fixup: IPv4 inbound tunnel\n");
        }
        else if (AppendFlags == (BIT_31|BIT_30|BIT_29))
        { // IPv4 header update, proto + length + checksum.
            Adapter_PEC_PktByte_Put(PacketHandle, Offset + 2, Append_p[0]);
            Adapter_PEC_PktByte_Put(PacketHandle, Offset + 3, Append_p[1]);
            Adapter_PEC_PktByte_Put(PacketHandle, Offset + 9, Append_p[5]);
            Adapter_PEC_PktByte_Put(PacketHandle, Offset + 10, Append_p[8]);
            Adapter_PEC_PktByte_Put(PacketHandle, Offset + 11, Append_p[9]);
            LOG_INFO("IOToken_Fixup: IPv4 inbound transport\n");
        }
        else
        {
            LOG_CRIT("IOToken_Fixup: Unexpected flags combination 0x%08x\n",
                     AppendFlags);
            return -1;
        }
    }
    return 0;
}

/*----------------------------------------------------------------------------
 * IOToken_SAAddr_Update
 */
int
IOToken_SAAddr_Update(
        const IOToken_PhysAddr_t * const SA_PhysAddr_p,
        uint32_t * InTokenData_p)
{
    IOTOKEN_CHECK_POINTER(SA_PhysAddr_p);
    IOTOKEN_CHECK_POINTER(InTokenData_p);

    // Input Token word: Context (SA) low 32 bits physical address
    InTokenData_p[IOTOKEN_SA_ADDR_LO_IN_WORD_OFFS] = SA_PhysAddr_p->Lo;

    // Input Token word: Context (SA) high 32 bits physical address
    InTokenData_p[IOTOKEN_SA_ADDR_HI_IN_WORD_OFFS] = SA_PhysAddr_p->Hi;

    return 2; // updated 32-bit token words
}


/*----------------------------------------------------------------------------
 * IOToken_SAReuse_Update
 */
int
IOToken_SAReuse_Update(
        const bool fReuseSA,
        uint32_t * InTokenData_p)
{
    IOTOKEN_CHECK_POINTER(InTokenData_p);

    // Enable Context Reuse auto detect if no new SA
    if (fReuseSA)
        InTokenData_p[IOTOKEN_HDR_IN_WORD_OFFS] |= BIT_21;
    else
        InTokenData_p[IOTOKEN_HDR_IN_WORD_OFFS] &= ~BIT_21;

    return 1;
}


/*----------------------------------------------------------------------------
 * IOToken_Mark_Set
 */
int
IOToken_Mark_Set(
        uint32_t * InTokenData_p)
{
    IOTOKEN_CHECK_POINTER(InTokenData_p);

    InTokenData_p[IOTOKEN_APP_ID_IN_WORD_OFFS] &= ~(MASK_7_BITS << 9);
    InTokenData_p[IOTOKEN_APP_ID_IN_WORD_OFFS] |= IOTOKEN_MARK;

    return 1;
}


/*----------------------------------------------------------------------------
 * IOToken_Mark_Offset_Get
 */
int
IOToken_OutMarkOffset_Get(void)
{
    return IOTOKEN_APP_ID_OUT_WORD_OFFS;
}


/*----------------------------------------------------------------------------
 * IOToken_Mark_Check
 */
int
IOToken_Mark_Check(
        uint32_t * OutTokenData_p)
{
    uint32_t Mark;

    IOTOKEN_CHECK_POINTER(OutTokenData_p);

    Mark = OutTokenData_p[IOTOKEN_APP_ID_OUT_WORD_OFFS] & IOTOKEN_MARK;

    if (Mark == IOTOKEN_MARK)
        return 0;
    else
        return 1;
}


/*----------------------------------------------------------------------------
 * IOToken_Parse
 */
int
IOToken_Parse(
        const uint32_t * Data_p,
        IOToken_Output_Dscr_t * const Dscr_p)
{
    unsigned int i, j = 0;
    IOToken_Output_Dscr_Ext_t * Ext_p;

    IOTOKEN_CHECK_POINTER(Data_p);
    IOTOKEN_CHECK_POINTER(Dscr_p);

    Ext_p = (IOToken_Output_Dscr_Ext_t *)Dscr_p->Ext_p;

    // Output Token word: EIP-96 Output Token Header Word
    {
        i = IOTOKEN_HDR_OUT_WORD_OFFS;
        Dscr_p->OutPacket_ByteCount = Data_p[i]       & MASK_17_BITS;
        Dscr_p->ErrorCode           = Data_p[i] >> 17 & MASK_15_BITS;
    }

    // Output Token word: EIP-96 Output Token Bypass Data Length Word
    {
        i = IOTOKEN_BP_LEN_OUT_WORD_OFFS;
        Dscr_p->BypassData_ByteCount =  Data_p[i] & MASK_4_BITS;
        Dscr_p->fHashAppended        = (Data_p[i] & BIT_21) != 0;

        Dscr_p->Hash_ByteCount       = Data_p[i] >> 22 & MASK_6_BITS;

        Dscr_p->fBytesAppended       = (Data_p[i] & BIT_28) != 0;
        Dscr_p->fChecksumAppended    = (Data_p[i] & BIT_29) != 0;
        Dscr_p->fNextHeaderAppended  = (Data_p[i] & BIT_30) != 0;
        Dscr_p->fLengthAppended      = (Data_p[i] & BIT_31) != 0;
    }

    // Output Token word: EIP-96 Output Token Application ID Word
    {
        i = IOTOKEN_APP_ID_OUT_WORD_OFFS;
        Dscr_p->AppID = Data_p[i] >> 9 & MASK_7_BITS;
    }

    // Output Token word: Pad Length and Next Header
    {
        i = IOTOKEN_PAD_NH_OUT_WORD_OFFS;
        Dscr_p->NextHeader    = Data_p[i]      & MASK_8_BITS;
        Dscr_p->Pad_ByteCount = Data_p[i] >> 8 & MASK_8_BITS;
    }

    // Parse Output Token descriptor extension if requested
    if (Ext_p)
    {
        // Output Token word: ToS/TC, DF, Firmware errors
        {
            j = IOTOKEN_BP_LEN_OUT_WORD_OFFS;
            Ext_p->TOS_TC    = Data_p[j] >> 5  & MASK_8_BITS;

            Ext_p->fDF       = (Data_p[j] & BIT_13) != 0;

            Ext_p->CfyErrors = Data_p[j] >> 16 & MASK_5_BITS;
        }

#ifdef IOTOKEN_EXTENDED_ERRORS_ENABLE
        if ((Data_p[IOTOKEN_HDR_OUT_WORD_OFFS] & BIT_31) != 0)
        {
            Ext_p->ExtErrors = (Data_p[IOTOKEN_HDR_OUT_WORD_OFFS] >> 17) & MASK_8_BITS;
        }
        else
#endif
        {
            Ext_p->ExtErrors = 0;
        }

        // Output Token word: IP Length Delta and Offset
        {
            j = IOTOKEN_PAD_NH_OUT_WORD_OFFS;
            Ext_p->Offset_ByteCount  = Data_p[j] >> 16 & MASK_8_BITS;
            Ext_p->IPDelta_ByteCount = Data_p[j] >> 24 & MASK_8_BITS;
        }

        // Output Token word: SA physical address low/high words
        {
            j = IOTOKEN_SA_ADDR_LO_OUT_WORD_OFFS;
            Ext_p->SA_PhysAddr.Lo = Data_p[j];

            j = IOTOKEN_SA_ADDR_HI_OUT_WORD_OFFS;
            Ext_p->SA_PhysAddr.Hi = Data_p[j];
        }

        // Output Token word: Application header processing context
        {
            j = IOTOKEN_NPH_CTX_OUT_WORD_OFFS;
            Ext_p->NPH_Context = Data_p[j];
        }

        // Output Token word: Previous Next Header Offset
        {
            j = IOTOKEN_PREV_NH_OFFS_OUT_WORD_OFFS;
            Ext_p->NextHeaderOffset = Data_p[j] & MASK_16_BITS;
            Ext_p->fInIPv6 = ((Data_p[j] & BIT_16) != 0);
            Ext_p->fFromEther = ((Data_p[j] & BIT_17) != 0);
            Ext_p->fOutIPv6 = ((Data_p[j] & BIT_22) != 0);
            Ext_p->fInbTunnel = ((Data_p[j] & BIT_23) != 0);
        }

#if IOTOKEN_BYPASS_WORD_COUNT > 0
        // Output Token word: Bypass Data
        {
            unsigned int k = 0;
            if (Ext_p->BypassData_p)
            {
                for (k = 0; k < IOTOKEN_BYPASS_WORD_COUNT; k++)
                    Ext_p->BypassData_p[k] =
                        Data_p[k+IOTOKEN_BP_DATA_OUT_WORD_OFFS];
            }
            j += k;
        }
#endif
    }

    // Output Token word: Reserved word not used
    // i = IOTOKEN_RSVD_OUT_WORD_OFFS;

    return MAX(i, j);
}


/*----------------------------------------------------------------------------
 * IOToken_PacketLegth_Get
 */
int
IOToken_PacketLegth_Get(
        const uint32_t * Data_p,
        unsigned int * Pkt_ByteCount_p)
{
    IOTOKEN_CHECK_POINTER(Data_p);
    IOTOKEN_CHECK_POINTER(Pkt_ByteCount_p);

    *Pkt_ByteCount_p = Data_p[IOTOKEN_HDR_OUT_WORD_OFFS] & MASK_17_BITS;

    return 1;
}


/*----------------------------------------------------------------------------
 * IOToken_BypassLegth_Get
 */
int
IOToken_BypassLegth_Get(
        const uint32_t * Data_p,
        unsigned int * BD_ByteCount_p)
{
    IOTOKEN_CHECK_POINTER(Data_p);
    IOTOKEN_CHECK_POINTER(BD_ByteCount_p);

    *BD_ByteCount_p = Data_p[IOTOKEN_BP_LEN_OUT_WORD_OFFS] & MASK_4_BITS;

    return 1;
}


/*----------------------------------------------------------------------------
 * IOToken_ErrorCode_Get
 */
int
IOToken_ErrorCode_Get(
        const uint32_t * Data_p,
        unsigned int * ErrorCode_p)
{
    IOTOKEN_CHECK_POINTER(Data_p);
    IOTOKEN_CHECK_POINTER(ErrorCode_p);

    *ErrorCode_p = (Data_p[IOTOKEN_HDR_OUT_WORD_OFFS] >> 17) & MASK_15_BITS;
#ifdef IOTOKEN_EXTENDED_ERRORS_ENABLE
    if ((Data_p[IOTOKEN_HDR_OUT_WORD_OFFS] & BIT_31) != 0)
    {
        *ErrorCode_p &= ~0x40ff; // Remove the detailed error code and related bits.
    }
#endif

    return 1;
}


/* end of file iotoken.c */
