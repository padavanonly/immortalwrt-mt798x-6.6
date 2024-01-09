/* adapter_ring_eip202.c
 *
 * Adapter EIP-202 implementation: EIP-202 specific layer.
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

// SLAD Adapter PEC Device-specific API
#include "adapter_pecdev_dma.h"

// Ring Control configuration API
#include "adapter_ring_eip202.h"

#if defined(ADAPTER_EIP202_RC_DIRECT_INVALIDATE_SUPPORT) || \
    defined(ADAPTER_EIP202_RC_DMA_BANK_SUPPORT)
// Record Cache configuration API
#include "adapter_rc_eip207.h"
#endif


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_adapter_eip202.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // bool
#include "api_pec.h"
#include "api_dmabuf.h"

// Adapter DMABuf internal API
#include "adapter_dmabuf.h"

// Convert address to pair of 32-bit words.
#include "adapter_addrpair.h"

// Adapter interrupts API
#include "adapter_interrupts.h" // Adapter_Interrupt_*

// Adapter memory allocation API
#include "adapter_alloc.h"      // Adapter_Alloc/Free

// Driver Framework DMAResource API
#include "dmares_addr.h"      // AddrTrans_*
#include "dmares_buf.h"       // DMAResource_Alloc/Release
#include "dmares_rw.h"        // DMAResource_PreDMA/PostDMA
#include "dmares_mgmt.h"      // DMAResource_Alloc/Release

#include "device_types.h"  // Device_Handle_t
#include "device_mgmt.h" // Device_find
#include "device_rw.h" // Device register read/write

#ifdef ADAPTER_EIP202_ENABLE_SCATTERGATHER
#include "api_pec_sg.h"         // PEC_SG_* (the API we implement here)
#endif

// EIP97 Ring Control
#include "eip202_ring_types.h"
#include "eip202_cdr.h"
#include "eip202_rdr.h"

// Standard IOToken API
#include "iotoken.h"


#ifdef ADAPTER_EIP202_USE_SHDEVXS
#include "shdevxs_prng.h"
#else
#include "eip97_global_init.h"
#endif

#if defined(ADAPTER_EIP202_RC_DIRECT_INVALIDATE_SUPPORT) || \
    defined(ADAPTER_EIP202_RC_DMA_BANK_SUPPORT)
#ifdef ADAPTER_EIP202_USE_SHDEVXS
// EIP-207 Record Cache (RC) interface
#include "shdevxs_dmapool.h"
#include "shdevxs_rc.h"
#else
// EIP-207 Record Cache (RC) interface
#include "eip207_rc.h"                  // EIP207_RC_*
#endif // ADAPTER_EIP202_USE_SHDEVXS
#endif

#include "clib.h"  // memcpy, ZEROINIT

// Log API
#include "log.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#if defined(ADAPTER_EIP202_RC_DIRECT_INVALIDATE_SUPPORT) || \
    defined(ADAPTER_EIP202_RC_DMA_BANK_SUPPORT)
#ifndef ADAPTER_EIP202_USE_SHDEVXS
// The default Record Cache set number to be used
// Note: Only one cache set is supported by this implementation!
#define EIP207_RC_SET_NR_DEFAULT              0
#endif
#endif // ADAPTER_EIP202_RC_DIRECT_INVALIDATE_SUPPORT || ADAPTER_EIP202_RC_DMA_BANK_SUPPORT

#if defined(ADAPTER_EIP202_RC_DIRECT_INVALIDATE_SUPPORT) || \
    defined(ADAPTER_EIP202_RC_DMA_BANK_SUPPORT)
#ifdef ADAPTER_EIP202_USE_SHDEVXS
#define ADAPTER_EIP202_DUMMY_DMA_ADDRESS_LO   SHDevXS_RC_Record_Dummy_Addr_Get()
#define ADAPTER_EIP202_DUMMY_DMA_ADDRESS_HI   0
#else
#define ADAPTER_EIP202_DUMMY_DMA_ADDRESS_LO   EIP207_RC_Record_Dummy_Addr_Get()
#define ADAPTER_EIP202_DUMMY_DMA_ADDRESS_HI   0
#endif // ADAPTER_EIP202_USE_SHDEVXS
#else

#ifdef ADAPTER_EIP202_USE_POINTER_TYPES
#define ADAPTER_EIP202_DUMMY_DMA_ADDRESS_LO       0xfffffffc
#define ADAPTER_EIP202_DUMMY_DMA_ADDRESS_HI       0xffffffff
#else
#define ADAPTER_EIP202_DUMMY_DMA_ADDRESS_LO       3
#define ADAPTER_EIP202_DUMMY_DMA_ADDRESS_HI       0
#endif

#endif // ADAPTER_EIP202_RC_DIRECT_INVALIDATE_SUPPORT || ADAPTER_EIP202_RC_DMA_BANK_SUPPORT

// Move bit position src in value v to dst.
#define BIT_MOVE(v, src, dst)  ( ((src) < (dst)) ? \
    ((v) << ((dst) - (src))) & (1 << (dst)) :  \
    ((v) >> ((src) - (dst))) & (1 << (dst)))

#ifndef ADAPTER_EIP202_SEPARATE_RINGS
#define ADAPTER_EIP202_CDR_BYTE_OFFSET              \
                    (EIP202_RD_CTRL_DATA_MAX_WORD_COUNT * sizeof(uint32_t))
#endif


typedef struct
{
    unsigned int CDR_IRQ_ID;

    unsigned int CDR_IRQ_Flags;

    const char * CDR_DeviceName_p;

    unsigned int RDR_IRQ_ID;

    unsigned int RDR_IRQ_Flags;

    const char * RDR_DeviceName_p;

} Adapter_Ring_EIP202_Device_t;


#ifdef ADAPTER_EIP202_USE_POINTER_TYPES
#define ADAPTER_EIP202_TR_ADDRESS       2
#ifndef ADAPTER_EIP202_USE_LARGE_TRANSFORM_DISABLE
#define ADAPTER_EIP202_TR_LARGE_ADDRESS 3
// Bit in first word of SA record to indicate it is large.
#define ADAPTER_EIP202_TR_ISLARGE              BIT_4
#endif
#endif


/*----------------------------------------------------------------------------
 * Local variables
 */

static const Adapter_Ring_EIP202_Device_t EIP202_Devices [] =
{
    ADAPTER_EIP202_DEVICES
};

// number of devices supported calculated on ADAPTER_EIP202_DEVICES defined
// in c_adapter_eip202.h
#define ADAPTER_EIP202_DEVICE_COUNT_ACTUAL \
        (sizeof(EIP202_Devices) / sizeof(Adapter_Ring_EIP202_Device_t))

static EIP202_CDR_Settings_t  CDR_Settings;
static EIP202_RDR_Settings_t  RDR_Settings;

#ifdef ADAPTER_EIP202_INTERRUPTS_ENABLE
static unsigned int EIP202_Interrupts[ADAPTER_EIP202_DEVICE_COUNT];
#endif

static EIP202_Ring_IOArea_t CDR_IOArea[ADAPTER_EIP202_DEVICE_COUNT];
static EIP202_Ring_IOArea_t RDR_IOArea[ADAPTER_EIP202_DEVICE_COUNT];

static DMAResource_Handle_t CDR_Handle[ADAPTER_EIP202_DEVICE_COUNT];
static DMAResource_Handle_t RDR_Handle[ADAPTER_EIP202_DEVICE_COUNT];

static  EIP202_ARM_CommandDescriptor_t
    EIP202_CDR_Entries[ADAPTER_EIP202_DEVICE_COUNT][ADAPTER_EIP202_MAX_LOGICDESCR];

static  EIP202_ARM_PreparedDescriptor_t
    EIP202_RDR_Prepared[ADAPTER_EIP202_DEVICE_COUNT][ADAPTER_EIP202_MAX_LOGICDESCR];

static  EIP202_ARM_ResultDescriptor_t
    EIP202_RDR_Entries[ADAPTER_EIP202_DEVICE_COUNT][ADAPTER_EIP202_MAX_LOGICDESCR];

static bool EIP202_ContinuousScatter[ADAPTER_EIP202_DEVICE_COUNT];

static const PEC_Capabilities_t CapabilitiesString =
{
    "EIP-202 Packet Engine rings=__ (ARM,"        // szTextDescription
#ifdef ADAPTER_EIP202_ENABLE_SCATTERGATHER
    "SG,"
#endif
#ifndef ADAPTER_EIP202_REMOVE_BOUNCEBUFFERS
    "BB,"
#endif
#ifdef ADAPTER_EIP202_INTERRUPTS_ENABLE
    "Int)",
#else
    "Poll)",
#endif
    0
};

// Static variables used by the adapter_ring_eip202.h interface implementation
static bool    Adapter_Ring_EIP202_Configured;
static uint8_t Adapter_Ring_EIP202_HDW;
static uint8_t Adapter_Ring_EIP202_CFSize;
static uint8_t Adapter_Ring_EIP202_RFSize;

#ifdef ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE
#ifdef ADAPTER_EIP202_RC_DMA_BANK_SUPPORT
static uint32_t EIP202_SABaseAddr;
static uint32_t EIP202_SABaseUpperAddr;
#endif // ADAPTER_EIP202_RC_DMA_BANK_SUPPORT
#endif // ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE

#ifdef ADAPTER_EIP202_RC_DIRECT_INVALIDATE_SUPPORT
// Static variables used by the adapter_rc_eip207.h interface implementation
static bool Adapter_RC_EIP207_Configured = true;
static bool Adapter_RC_EIP207_TRC_Enabled = true;
static bool Adapter_RC_EIP207_ARC4RC_Enabled;
static bool Adapter_RC_EIP207_Combined;
#endif // ADAPTER_EIP202_RC_DIRECT_INVALIDATE_SUPPORT

// DMA buffer allocation alignment in bytes
static int Adapter_Ring_EIP202_DMA_Alignment_ByteCount =
                                        ADAPTER_DMABUF_ALIGNMENT_INVALID;


#ifdef ADAPTER_EIP202_ADD_INIT_DIAGNOSTICS
static void
Adapter_Register_Dump(void)
{
    Device_Handle_t Device=Device_Find("EIP197_GLOBAL");
    unsigned int i,j,v[256];
    LOG_CRIT("REGISTER DUMP\n");
    for (i=0; i<0x100000; i+=1024)
    {
        Device_Read32Array(Device, i, v, 256);
        for (j=0; j<256; j++)
        {
            if (v[j])
            {
                LOG_CRIT("Addr 0x%08x = 0x%08x\n",i+j*4,v[j]);
            }
        }
    }
    LOG_CRIT("END REGISTER DUMP\n");
}
#endif

/*----------------------------------------------------------------------------
 * AdapterLib_RD_OutTokens_Free
 *
 * Free Output Tokens in the EIP-202 result descriptors.
 */
static void
AdapterLib_RD_OutTokens_Free(
        const unsigned int InterfaceId)
{
    unsigned int i;

    for (i = 0; i < ADAPTER_EIP202_MAX_LOGICDESCR; i++)
        Adapter_Free(EIP202_RDR_Entries[InterfaceId][i].Token_p);
}


/*----------------------------------------------------------------------------
 * AdapterLib_RD_OutTokens_Alloc
 *
 * Allocate Output Tokens in the EIP-202 result descriptors.
 */
static bool
AdapterLib_RD_OutTokens_Alloc(
        const unsigned int InterfaceId)
{
    unsigned int i;

    for (i = 0; i < ADAPTER_EIP202_MAX_LOGICDESCR; i++)
    {
        uint32_t * p =
               Adapter_Alloc(IOToken_OutWordCount_Get() * sizeof (uint32_t));

        if (p)
            EIP202_RDR_Entries[InterfaceId][i].Token_p = p;
        else
            goto exit_error;
    }

    return true;

exit_error:
    AdapterLib_RD_OutTokens_Free(InterfaceId);

    return false;
}


/*----------------------------------------------------------------------------
 * Adapter_GetPhysAddr
 *
 * Obtain the physical address from a DMABuf handle.
 * Take the bounce handle into account.
 *
 */
static void
Adapter_GetPhysAddr(
        const DMABuf_Handle_t Handle,
        EIP202_DeviceAddress_t *PhysAddr_p)
{
    DMAResource_Handle_t DMAHandle =
        Adapter_DMABuf_Handle2DMAResourceHandle(Handle);
    DMAResource_Record_t * Rec_p = NULL;
    DMAResource_AddrPair_t PhysAddr_Pair;

    if (PhysAddr_p == NULL)
    {
        LOG_CRIT("Adapter_GetPhysAddr: PANIC\n");
        return;
    }

    PhysAddr_p->Addr        = ADAPTER_EIP202_DUMMY_DMA_ADDRESS_LO;
    PhysAddr_p->UpperAddr   = ADAPTER_EIP202_DUMMY_DMA_ADDRESS_HI;

    // Note: this function is sometimes invoked with invalid DMA handle
    //       to obtain a dummy address, not an error.
    if (!DMAResource_IsValidHandle(DMAHandle))
        return; // success!

    Rec_p = DMAResource_Handle2RecordPtr(DMAHandle);

    if (Rec_p == NULL)
    {
        LOG_CRIT("Adapter_GetPhysAddr: failed\n");
        return;
    }

#ifndef ADAPTER_EIP202_REMOVE_BOUNCEBUFFERS
    if (Rec_p->bounce.Bounce_Handle != NULL)
        DMAHandle = Rec_p->bounce.Bounce_Handle;
#endif

    if (DMAResource_Translate(DMAHandle, DMARES_DOMAIN_BUS,
                              &PhysAddr_Pair) == 0)
    {
        Adapter_AddrToWordPair(PhysAddr_Pair.Address_p, 0, &PhysAddr_p->Addr,
                              &PhysAddr_p->UpperAddr);
        // success!
    }
    else
    {
        LOG_CRIT("Adapter_GetPhysAddr: failed\n");
    }
}


/*----------------------------------------------------------------------------
 * BoolToString()
 *
 * Convert boolean value to string.
 */
static const char *
BoolToString(
        bool b)
{
    if (b)
        return "TRUE";
    else
        return "false";
}


/*----------------------------------------------------------------------------
 * AdapterLib_Ring_EIP202_CDR_Status_Report
 *
 * Report the status of the CDR interface
 *
 */
static void
AdapterLib_Ring_EIP202_CDR_Status_Report(
        unsigned int InterfaceId)
{
    EIP202_Ring_Error_t res;
    EIP202_CDR_Status_t CDR_Status;

    LOG_CRIT("Status of CDR interface %d\n",InterfaceId);

    LOG_INFO("\n\t\t\t EIP202_CDR_Status_Get \n");

    ZEROINIT(CDR_Status);
    res = EIP202_CDR_Status_Get(CDR_IOArea + InterfaceId, &CDR_Status);
    if (res != EIP202_RING_NO_ERROR)
    {
        LOG_CRIT("EIP202_CDR_Status_Get returned error\n");
        return;
    }

    // Report CDR status
    LOG_CRIT("CDR Status: CD prep/proc count %d/%d, proc pkt count %d\n",
             CDR_Status.CDPrepWordCount,
             CDR_Status.CDProcWordCount,
             CDR_Status.CDProcPktWordCount);

    if (CDR_Status.fDMAError    ||
        CDR_Status.fError       ||
        CDR_Status.fOUFlowError ||
        CDR_Status.fTresholdInt ||
        CDR_Status.fTimeoutInt)
    {
        // Report CDR errors
        LOG_CRIT("CDR Status: error(s) detected\n");
        LOG_CRIT("\tDMA err:       %s\n"
                 "\tErr:           %s\n"
                 "\tOvf/under err: %s\n"
                 "\tThreshold int: %s\n"
                 "\tTimeout int:   %s\n"
                 "\tFIFO count:    %d\n",
                 BoolToString(CDR_Status.fDMAError),
                 BoolToString(CDR_Status.fError),
                 BoolToString(CDR_Status.fOUFlowError),
                 BoolToString(CDR_Status.fTresholdInt),
                 BoolToString(CDR_Status.fTimeoutInt),
                 CDR_Status.CDFIFOWordCount);
    }
    else
        LOG_CRIT("CDR Status: all OK, FIFO count: %d\n",
                 CDR_Status.CDFIFOWordCount);
}


/*----------------------------------------------------------------------------
 * AdapterLib_Ring_EIP202_RDR_Status_Report
 *
 * Report the status of the RDR interface
 *
 */
static void
AdapterLib_Ring_EIP202_RDR_Status_Report(
        unsigned int InterfaceId)
{
    EIP202_Ring_Error_t res;
    EIP202_RDR_Status_t RDR_Status;

    LOG_CRIT("Status of RDR interface %d\n",InterfaceId);
    if (EIP202_ContinuousScatter[InterfaceId])
        LOG_CRIT("Ring is in continuous scatter mode\n");

    LOG_INFO("\n\t\t\t EIP202_RDR_Status_Get \n");

    ZEROINIT(RDR_Status);
    res = EIP202_RDR_Status_Get(RDR_IOArea + InterfaceId, &RDR_Status);
    if (res != EIP202_RING_NO_ERROR)
    {
        LOG_CRIT("EIP202_RDR_Status_Get returned error\n");
        return;
    }

    // Report RDR status
    LOG_CRIT("RDR Status: RD prep/proc count %d/%d, proc pkt count %d\n",
             RDR_Status.RDPrepWordCount,
             RDR_Status.RDProcWordCount,
             RDR_Status.RDProcPktWordCount);

    if (RDR_Status.fDMAError         ||
        RDR_Status.fError            ||
        RDR_Status.fOUFlowError      ||
        RDR_Status.fRDBufOverflowInt ||
        RDR_Status.fRDOverflowInt    ||
        RDR_Status.fTresholdInt      ||
        RDR_Status.fTimeoutInt)
    {
        // Report RDR errors
        LOG_CRIT("RDR Status: error(s) detected\n");
        LOG_CRIT("\tDMA err:        %s\n"
                 "\tErr:            %s\n"
                 "\tOvf/under err:  %s\n"
                 "\tBuf ovf:        %s\n"
                 "\tDescriptor ovf: %s\n"
                 "\tThreshold int:  %s\n"
                 "\tTimeout int:    %s\n"
                 "\tFIFO count:     %d\n",
                 BoolToString(RDR_Status.fDMAError),
                 BoolToString(RDR_Status.fError),
                 BoolToString(RDR_Status.fOUFlowError),
                 BoolToString(RDR_Status.fRDBufOverflowInt),
                 BoolToString(RDR_Status.fRDOverflowInt),
                 BoolToString(RDR_Status.fTresholdInt),
                 BoolToString(RDR_Status.fTimeoutInt),
                 RDR_Status.RDFIFOWordCount);
    }
    else
        LOG_CRIT("RDR Status: all OK, FIFO count: %d\n",
                 RDR_Status.RDFIFOWordCount);
}


/*----------------------------------------------------------------------------
 * AdapterLib_Ring_EIP202_Status_Report
 *
 * Report the status of the CDR and RDR interface
 *
 */
static void
AdapterLib_Ring_EIP202_Status_Report(
        unsigned int InterfaceId)
{
    AdapterLib_Ring_EIP202_CDR_Status_Report(InterfaceId);

    AdapterLib_Ring_EIP202_RDR_Status_Report(InterfaceId);
}


/*----------------------------------------------------------------------------
 * AdapterLib_Ring_EIP202_AlignForSize
 */
static unsigned int
AdapterLib_Ring_EIP202_AlignForSize(
        const unsigned int ByteCount,
        const unsigned int AlignTo)
{
    unsigned int AlignedByteCount = ByteCount;

    // Check if alignment and padding for length alignment is required
    if (AlignTo > 1 && ByteCount % AlignTo)
        AlignedByteCount = ByteCount / AlignTo * AlignTo + AlignTo;

    return AlignedByteCount;
}


/*----------------------------------------------------------------------------
 * AdapterLib_Ring_EIP202_DMA_Alignment_Determine
 *
 * Determine the required EIP-202 DMA alignment value
 *
 */
static bool
AdapterLib_Ring_EIP202_DMA_Alignment_Determine(void)
{
#ifdef ADAPTER_EIP202_ALLOW_UNALIGNED_DMA
    Adapter_DMAResource_Alignment_Set(1);
#else
    // Default alignment value is invalid
    Adapter_Ring_EIP202_DMA_Alignment_ByteCount =
                                ADAPTER_DMABUF_ALIGNMENT_INVALID;

    if (Adapter_Ring_EIP202_Configured)
    {
        int AlignTo = Adapter_Ring_EIP202_HDW;

        // Determine the EIP-202 master interface hardware data width
        switch (AlignTo)
        {
            case EIP202_RING_DMA_ALIGNMENT_4_BYTES:
                Adapter_DMAResource_Alignment_Set(4);
                break;
            case EIP202_RING_DMA_ALIGNMENT_8_BYTES:
                Adapter_DMAResource_Alignment_Set(8);
                break;
            case EIP202_RING_DMA_ALIGNMENT_16_BYTES:
                Adapter_DMAResource_Alignment_Set(16);
                break;
            case EIP202_RING_DMA_ALIGNMENT_32_BYTES:
                Adapter_DMAResource_Alignment_Set(32);
                break;
            default:
                // Not supported, the alignment value cannot be determined
                LOG_CRIT("AdapterLib_Ring_EIP202_DMA_Alignment_Determine: "
                         "EIP-202 master interface HW data width "
                         "(%d) is unsupported\n",
                         AlignTo);
                return false; // Error
        } // switch
    }
    else
    {
#if ADAPTER_EIP202_DMA_ALIGNMENT_BYTE_COUNT == 0
        // The alignment value cannot be determined
        return false; // Error
#else
        // Set the configured non-zero alignment value
        Adapter_DMAResource_Alignment_Set(
                ADAPTER_EIP202_DMA_ALIGNMENT_BYTE_COUNT);
#endif
    }
#endif

    Adapter_Ring_EIP202_DMA_Alignment_ByteCount =
                            Adapter_DMAResource_Alignment_Get();

    return true; // Success
}


/*----------------------------------------------------------------------------
 * AdapterLib_Ring_EIP202_DMA_Alignment_Get
 *
 * Get the required EIP-202 DMA alignment value
 *
 */
inline static int
AdapterLib_Ring_EIP202_DMA_Alignment_Get(void)
{
    return Adapter_Ring_EIP202_DMA_Alignment_ByteCount;
}


/*----------------------------------------------------------------------------
 * AdapterLib_Ring_EIP202_DMAAddr_IsSane
 *
 * Validate the DMA address for the EIP-202 device
 *
 */
inline static bool
AdapterLib_Ring_EIP202_DMAAddr_IsSane(
        const EIP202_DeviceAddress_t * const DevAddr_p,
        const char * BufferName_p)
{
#ifndef ADAPTER_EIP202_64BIT_DEVICE
    // Check if 64-bit DMA address is used for EIP-202 configuration
    // that supports 32-bit addresses only
    if (DevAddr_p->UpperAddr)
    {
        LOG_CRIT("%s: failed, "
                 "%s bus address (low/high 32 bits 0x%08x/0x%08x) too big\n",
                 __func__,
                 BufferName_p,
                 DevAddr_p->Addr,
                 DevAddr_p->UpperAddr);
        return false;
    }
    else
        return true;
#else
    IDENTIFIER_NOT_USED(DevAddr_p);
    IDENTIFIER_NOT_USED(BufferName_p);
    return true;
#endif
}


/*----------------------------------------------------------------------------
 * Implementation of the adapter_ring_eip202.h interface
 *
 */

/*----------------------------------------------------------------------------
 * Adapter_Ring_EIP202_Configure
 */
void
Adapter_Ring_EIP202_Configure(
        const uint8_t HostDataWidth,
        const uint8_t CF_Size,
        const uint8_t RF_Size)
{
    Adapter_Ring_EIP202_HDW    = HostDataWidth;
    Adapter_Ring_EIP202_CFSize = CF_Size;
    Adapter_Ring_EIP202_RFSize = RF_Size;

    Adapter_Ring_EIP202_Configured = true;
}

/*----------------------------------------------------------------------------
 * Implementation of the adapter_rc_eip207.h interface
 *
 */

#ifdef ADAPTER_EIP202_RC_DIRECT_INVALIDATE_SUPPORT
/*----------------------------------------------------------------------------
 * Adapter_RC_EIP207_Configure
 */
void
Adapter_RC_EIP207_Configure(
        const bool fEnabledTRC,
        const bool fEnabledARC4RC,
        const bool fCombined)
{
    Adapter_RC_EIP207_Combined       = fCombined;
    Adapter_RC_EIP207_TRC_Enabled    = fEnabledTRC;
    Adapter_RC_EIP207_ARC4RC_Enabled = fEnabledARC4RC;

    Adapter_RC_EIP207_Configured     = true;

}
#endif // ADAPTER_EIP202_RC_DIRECT_INVALIDATE_SUPPORT


/*----------------------------------------------------------------------------
 * Implementation of the adapter_pecdev_dma.h interface
 *
 */

/*----------------------------------------------------------------------------
 * Adapter_PECDev_Capabilities_Get
 */
PEC_Status_t
Adapter_PECDev_Capabilities_Get(
        PEC_Capabilities_t * const Capabilities_p)
{
    uint8_t Versions[2];

    if (Capabilities_p == NULL)
        return PEC_ERROR_BAD_PARAMETER;

    memcpy(Capabilities_p, &CapabilitiesString, sizeof(CapabilitiesString));

    // now fill in the number of rings.
    {
        Versions[0] = ADAPTER_EIP202_DEVICE_COUNT / 10;
        Versions[1] = ADAPTER_EIP202_DEVICE_COUNT % 10;
    }

    {
        char * p = Capabilities_p->szTextDescription;
        int VerIndex = 0;
        int i = 0;

        if (p[sizeof(CapabilitiesString)-1] != 0)
            return PEC_ERROR_INTERNAL;

        while(p[i])
        {
            if (p[i] == '_')
            {
                if (VerIndex == sizeof(Versions)/sizeof(Versions[0]))
                    return PEC_ERROR_INTERNAL;
                if (Versions[VerIndex] > 9)
                    p[i] = '?';
                else
                    p[i] = '0' + Versions[VerIndex++];
            }

            i++;
        }
    }
#ifdef ADAPTER_EIP202_USE_SHDEVXS
    Capabilities_p->SupportedFuncs = SHDevXS_SupportedFuncs_Get();
#else
    Capabilities_p->SupportedFuncs = EIP97_SupportedFuncs_Get();
#endif
    return PEC_STATUS_OK;
}

/*----------------------------------------------------------------------------
 * Adapter_PECDev_Control_Write
 */
PEC_Status_t
Adapter_PECDev_CD_Control_Write(
    PEC_CommandDescriptor_t *Command_p,
    const PEC_PacketParams_t *PacketParams_p)
{
    IDENTIFIER_NOT_USED(Command_p);
    IDENTIFIER_NOT_USED(PacketParams_p);

    return PEC_ERROR_NOT_IMPLEMENTED;
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_RD_Status_Read
 */
PEC_Status_t
Adapter_PECDev_RD_Status_Read(
        const PEC_ResultDescriptor_t * const Result_p,
        PEC_ResultStatus_t * const ResultStatus_p)
{
    unsigned int s;

#ifdef ADAPTER_EIP202_STRICT_ARGS
    if (ResultStatus_p == NULL)
        return PEC_ERROR_BAD_PARAMETER;
#endif

    if (IOToken_ErrorCode_Get(Result_p->OutputToken_p, &s) < 0)
        return PEC_ERROR_BAD_PARAMETER;

    // Translate the EIP-96 error codes to the PEC error codes
    ResultStatus_p->errors =
        BIT_MOVE(s,  0, 11) | // EIP-96 Packet length error
        BIT_MOVE(s,  1, 17) | // EIP-96 Token error
        BIT_MOVE(s,  2, 18) | // EIP-96 Bypass error
        BIT_MOVE(s,  3,  9) | // EIP-96 Block size error
        BIT_MOVE(s,  4, 16) | // EIP-96 Hash block size error.
        BIT_MOVE(s,  5, 10) | // EIP-96 Invalid combo
        BIT_MOVE(s,  6,  5) | // EIP-96 Prohibited algo
        BIT_MOVE(s,  7, 19) | // EIP-96 Hash overflow
#ifndef ADAPTER_EIP202_RING_TTL_ERROR_WA
        BIT_MOVE(s,  8, 20) | // EIP-96 TTL/Hop limit underflow.
#endif
        BIT_MOVE(s,  9,  0) | // EIP-96 Authentication failed
        BIT_MOVE(s, 10,  2) | // EIP-96 Sequence number check failed.
        BIT_MOVE(s, 11,  8) | // EIP-96 SPI check failed.
        BIT_MOVE(s, 12, 21) | // EIP-96 Incorrect checksum
        BIT_MOVE(s, 12,  1) | // EIP-96 Pad verification.
        BIT_MOVE(s, 14, 22);  // EIP-96 Timeout

    // Translate the EIP-202 error codes to the PEC error codes
    s = Result_p->Status1;
    ResultStatus_p->errors  |=
        BIT_MOVE(s, 21, 26) | // EIP-202 Buffer overflow
        BIT_MOVE(s, 20, 25);  // EIP-202 Descriptor overflow

    return PEC_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Init
 */
PEC_Status_t
Adapter_PECDev_Init(
        const unsigned int InterfaceId,
        const PEC_InitBlock_t * const InitBlock_p)
{
    Device_Handle_t CDR_Device, RDR_Device;
    EIP202_Ring_Error_t res;
    DMAResource_AddrPair_t PhysAddr_Pair;
    unsigned int CDWordCount, RDWordCount; // Command and Result Descriptor size
    unsigned int CDOffset, RDOffset; // Command and Result Descriptor Offsets
    unsigned int RDTokenOffsWordCount;

    IDENTIFIER_NOT_USED(InitBlock_p);

    LOG_INFO("\n\t\t Adapter_PECDev_Init \n");

    if (ADAPTER_EIP202_DEVICE_COUNT > ADAPTER_EIP202_DEVICE_COUNT_ACTUAL)
    {
        LOG_CRIT("Adapter_PECDev_Init: "
                 "Adapter EIP-202 devices configuration is invalid\n");
        return PEC_ERROR_BAD_PARAMETER;
    }

    if (InterfaceId >= ADAPTER_EIP202_DEVICE_COUNT)
        return PEC_ERROR_BAD_PARAMETER;

    CDR_Device = Device_Find(EIP202_Devices[InterfaceId].CDR_DeviceName_p);
    RDR_Device = Device_Find(EIP202_Devices[InterfaceId].RDR_DeviceName_p);

    if (CDR_Device == NULL || RDR_Device == NULL)
        return PEC_ERROR_INTERNAL;
    EIP202_ContinuousScatter[InterfaceId] = InitBlock_p->fContinuousScatter;

    LOG_INFO("\n\t\t\t EIP202_CDR_Reset \n");

    // Reset both the CDR and RDR devives.
    res = EIP202_CDR_Reset(CDR_IOArea + InterfaceId, CDR_Device);
    if (res != EIP202_RING_NO_ERROR)
        return PEC_ERROR_INTERNAL;

    LOG_INFO("\n\t\t\t EIP202_RDR_Reset \n");

    res = EIP202_RDR_Reset(RDR_IOArea + InterfaceId, RDR_Device);
    if (res != EIP202_RING_NO_ERROR)
        return PEC_ERROR_INTERNAL;

    // Determine the DMA buffer allocation alignment
    if (!AdapterLib_Ring_EIP202_DMA_Alignment_Determine())
        return PEC_ERROR_INTERNAL;

#ifdef ADAPTER_EIP202_RING_LOCAL_CONFIGURE
    // Configure the EIP-202 Ring Managers with configuration
    // parameters obtained from local CDR device.
    {
      EIP202_Ring_Options_t options;

      res = EIP202_CDR_Options_Get(CDR_Device, &options);
      if (res != EIP202_RING_NO_ERROR)
      {
          LOG_CRIT("%s: EIP202_CDR_Options_Get failed, error %d\n",
                   __func__,
                   res);
          return PEC_ERROR_INTERNAL;
      }
      Adapter_Ring_EIP202_Configure(options.HDW,
                                    options.CF_Size,
                                    options.RF_Size);
    }
#endif
    // Determine required command and result descriptor size and offset
    {
        unsigned int ByteCount;

        CDWordCount = EIP202_CD_CTRL_DATA_MAX_WORD_COUNT +
                                            IOToken_InWordCount_Get();
        ByteCount = MAX(AdapterLib_Ring_EIP202_DMA_Alignment_Get(),
                        ADAPTER_EIP202_CD_OFFSET_BYTE_COUNT);
        ByteCount = AdapterLib_Ring_EIP202_AlignForSize(
                                CDWordCount * sizeof(uint32_t),
                                ByteCount);
        CDOffset = ByteCount / sizeof(uint32_t);

        LOG_CRIT("%s: EIP-202 CD size/offset %d/%d (32-bit words)\n",
                 __func__,
                 CDWordCount,
                 CDOffset);

        if (Adapter_Ring_EIP202_HDW == 3)
        {
            RDTokenOffsWordCount = 8;
        }
        else
        {
            RDTokenOffsWordCount = EIP202_RD_CTRL_DATA_MAX_WORD_COUNT;
        }
        RDWordCount = RDTokenOffsWordCount +
                                            IOToken_OutWordCount_Get();
        ByteCount = MAX(AdapterLib_Ring_EIP202_DMA_Alignment_Get(),
                        ADAPTER_EIP202_RD_OFFSET_BYTE_COUNT);
        ByteCount = AdapterLib_Ring_EIP202_AlignForSize(
                                RDWordCount * sizeof(uint32_t),
                                ByteCount);
        RDOffset = ByteCount / sizeof(uint32_t);

        LOG_CRIT("%s: EIP-202 RD size/offset %d/%d (32-bit words)\n",
                 __func__,
                 RDWordCount,
                 RDOffset);

#ifndef ADAPTER_EIP202_SEPARATE_RINGS
        {
            unsigned int RDMaxTokenWordCount = RDOffset -
                                           EIP202_RD_CTRL_DATA_MAX_WORD_COUNT;

            if (CDWordCount > RDMaxTokenWordCount)
            {
                LOG_CRIT("%s: failed, EIP-202 CD (%d 32-bit words) "
                         "exceeds max RD token space (%d 32-bit words)\n",
                         __func__,
                         CDWordCount,
                         RDMaxTokenWordCount);
                return PEC_ERROR_INTERNAL;
            }
        }
#endif
    }

#ifdef ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE
#ifdef ADAPTER_EIP202_RC_DMA_BANK_SUPPORT
#ifdef ADAPTER_EIP202_USE_SHDEVXS
    {
        void *SABaseAddr;

        SHDevXS_DMAPool_GetBase(&SABaseAddr);

        LOG_INFO("Adapter_PECDev_Init: got SA base %p\n",SABaseAddr);

        Adapter_AddrToWordPair(SABaseAddr,
                               0,
                               &EIP202_SABaseAddr,
                               &EIP202_SABaseUpperAddr);
    }
#else
    { // Set the SA pool base address.
        int dmares;
        Device_Handle_t Device;
        DMAResource_Handle_t DMAHandle;
        DMAResource_Properties_t DMAProperties;
        DMAResource_AddrPair_t DMAAddr;

        // Perform a dummy allocation in bank 1 to obtain the pool base
        // address.
        DMAProperties.Alignment = AdapterLib_Ring_EIP202_DMA_Alignment_Get();
        DMAProperties.Bank      = ADAPTER_EIP202_BANK_SA;
        DMAProperties.fCached   = false;
        DMAProperties.Size      = ADAPTER_EIP202_TRANSFORM_RECORD_COUNT *
                                    ADAPTER_EIP202_TRANSFORM_RECORD_BYTE_COUNT;

        dmares = DMAResource_Alloc(DMAProperties, &DMAAddr, &DMAHandle);
        if (dmares != 0)
            return PEC_ERROR_INTERNAL;

        // Derive the physical address from the DMA resource.
        if (DMAResource_Translate(DMAHandle,
                                  DMARES_DOMAIN_BUS,
                                  &DMAAddr) < 0)
        {
            DMAResource_Release(DMAHandle);
            return PEC_ERROR_INTERNAL;
        }

        Adapter_AddrToWordPair(DMAAddr.Address_p,
                               0,
                               &EIP202_SABaseAddr,
                               &EIP202_SABaseUpperAddr);

        // Set the cache base address.
        Device = Device_Find(ADAPTER_EIP202_GLOBAL_DEVICE_NAME);
        if (Device == NULL)
        {
            LOG_CRIT("Adapter_PECDev_UnInit: Could not find device\n");
            return PEC_ERROR_INTERNAL;
        }

        LOG_INFO("\n\t\t\t EIP207_RC_BaseAddr_Set \n");

        EIP207_RC_BaseAddr_Set(
            Device,
            EIP207_TRC_REG_BASE,
            EIP207_RC_SET_NR_DEFAULT,
            EIP202_SABaseAddr,
            EIP202_SABaseUpperAddr);

        // Release the DMA resource.
        DMAResource_Release(DMAHandle);
    }
#endif // ADAPTER_EIP202_USE_SHDEVXS
#endif // ADAPTER_EIP202_RC_DMA_BANK_SUPPORT
#endif // ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE

    // Allocate the ring buffers(s).
    {
        int dmares;
#ifdef ADAPTER_EIP202_SEPARATE_RINGS
        unsigned int CDRByteCount = 4 * ADAPTER_EIP202_MAX_PACKETS * CDOffset;
        unsigned int RDRByteCount = 4 * ADAPTER_EIP202_MAX_PACKETS * RDOffset;
#else
        unsigned int RDRByteCount = 4 * ADAPTER_EIP202_MAX_PACKETS * CDOffset +
                                                ADAPTER_EIP202_CDR_BYTE_OFFSET;
#endif

        DMAResource_Properties_t RingProperties;
        DMAResource_AddrPair_t RingHostAddr;

        // used as uint32_t array
        RingProperties.Alignment = AdapterLib_Ring_EIP202_DMA_Alignment_Get();
        RingProperties.Bank      = ADAPTER_EIP202_BANK_RING;
        RingProperties.fCached   = false;
        RingProperties.Size      = RDRByteCount;

        dmares = DMAResource_Alloc(RingProperties,
                                   &RingHostAddr,
                                   RDR_Handle + InterfaceId);
        if (dmares != 0)
            return PEC_ERROR_INTERNAL;

#ifdef ADAPTER_EIP202_ARMRING_ENABLE_SWAP
        DMAResource_SwapEndianness_Set(RDR_Handle[InterfaceId],true);
#endif

        memset (RingHostAddr.Address_p, 0, RDRByteCount);

#ifdef ADAPTER_EIP202_SEPARATE_RINGS

        RingProperties.Size = CDRByteCount;

        dmares = DMAResource_Alloc(RingProperties,
                                   &RingHostAddr,
                                   CDR_Handle + InterfaceId);
        if (dmares != 0)
        {
            DMAResource_Release(RDR_Handle[InterfaceId]);
            return PEC_ERROR_INTERNAL;
        }

#ifdef ADAPTER_EIP202_ARMRING_ENABLE_SWAP
        DMAResource_SwapEndianness_Set(CDR_Handle[InterfaceId], true);
#endif

        memset (RingHostAddr.Address_p, 0, CDRByteCount);

#else
        RingProperties.Size -= ADAPTER_EIP202_CDR_BYTE_OFFSET;
        RingProperties.fCached = false;

        {
            uint8_t * Byte_p = (uint8_t*)RingHostAddr.Address_p;

            Byte_p += ADAPTER_EIP202_CDR_BYTE_OFFSET;

            RingHostAddr.Address_p = Byte_p;
        }

        dmares = DMAResource_CheckAndRegister(RingProperties,
                                              RingHostAddr,
                                              'R',
                                              CDR_Handle + InterfaceId);
        if (dmares != 0)
        {
            DMAResource_Release(RDR_Handle[InterfaceId]);
            return PEC_ERROR_INTERNAL;
        }

#ifdef ADAPTER_EIP202_ARMRING_ENABLE_SWAP
        DMAResource_SwapEndianness_Set(CDR_Handle[InterfaceId], true);
#endif

#endif

        LOG_CRIT("%s: CDR/RDR byte count %d/%d, %s, CDR offset %d bytes\n",
                 __func__,
#ifdef ADAPTER_EIP202_SEPARATE_RINGS
                 CDRByteCount,
                 RDRByteCount,
                 "non-overlapping",
                 0);
#else
                 0,
                 RDRByteCount,
                 "overlapping",
                 (int)ADAPTER_EIP202_CDR_BYTE_OFFSET);
#endif
    }

    // Initialize the CDR and RDR devices
    {
        ZEROINIT(CDR_Settings);

#ifdef ADAPTER_EIP202_RING_MANUAL_CONFIGURE
        // Configure the EIP-202 Ring Managers with manually set
        // configuration parameters
        Adapter_Ring_EIP202_Configure(ADAPTER_EIP202_HOST_DATA_WIDTH,
                                      ADAPTER_EIP202_CF_SIZE,
                                      ADAPTER_EIP202_RF_SIZE);
#endif

        CDR_Settings.fATP        = (ADAPTER_EIP202_CDR_ATP_PRESENT > 0) ?
                                                                true : false;
        CDR_Settings.fATPtoToken = false;

        CDR_Settings.Params.DataBusWidthWordCount = 1;

        // Not used for CDR
        CDR_Settings.Params.ByteSwap_DataType_Mask      = 0;
        CDR_Settings.Params.ByteSwap_Packet_Mask        = 0;

        // Enable endianess conversion for the RDR master interface
        // if configured
#ifdef ADAPTER_EIP202_CDR_BYTE_SWAP_ENABLE
        CDR_Settings.Params.ByteSwap_Token_Mask         =
        CDR_Settings.Params.ByteSwap_Descriptor_Mask    =
                                            EIP202_RING_BYTE_SWAP_METHOD_32;
#else
        CDR_Settings.Params.ByteSwap_Token_Mask         = 0;
        CDR_Settings.Params.ByteSwap_Descriptor_Mask    = 0;
#endif

        CDR_Settings.Params.Bufferability = 0;
#ifdef ADAPTER_EIP202_64BIT_DEVICE
        CDR_Settings.Params.DMA_AddressMode = EIP202_RING_64BIT_DMA_DSCR_PTR;
#else
        CDR_Settings.Params.DMA_AddressMode = EIP202_RING_64BIT_DMA_DISABLED;
#endif
        CDR_Settings.Params.RingSizeWordCount =
                                        CDOffset * ADAPTER_EIP202_MAX_PACKETS;
        CDR_Settings.Params.RingDMA_Handle = CDR_Handle[InterfaceId];

#ifdef ADAPTER_EIP202_SEPARATE_RINGS
        if (DMAResource_Translate(CDR_Handle[InterfaceId], DMARES_DOMAIN_BUS,
                                  &PhysAddr_Pair) < 0)
        {
            Adapter_PECDev_UnInit(InterfaceId);
            return PEC_ERROR_INTERNAL;
        }
        Adapter_AddrToWordPair(PhysAddr_Pair.Address_p, 0,
                               &CDR_Settings.Params.RingDMA_Address.Addr,
                               &CDR_Settings.Params.RingDMA_Address.UpperAddr);
#else
        if (DMAResource_Translate(RDR_Handle[InterfaceId], DMARES_DOMAIN_BUS,
                                  &PhysAddr_Pair) < 0)
        {
            Adapter_PECDev_UnInit(InterfaceId);
            return PEC_ERROR_INTERNAL;
        }
        Adapter_AddrToWordPair(PhysAddr_Pair.Address_p,
                               ADAPTER_EIP202_CDR_BYTE_OFFSET,
                               &CDR_Settings.Params.RingDMA_Address.Addr,
                               &CDR_Settings.Params.RingDMA_Address.UpperAddr);
#endif

        CDR_Settings.Params.DscrSizeWordCount = CDWordCount;
        CDR_Settings.Params.DscrOffsWordCount = CDOffset;

#ifndef ADAPTER_EIP202_AUTO_THRESH_DISABLE
        if(Adapter_Ring_EIP202_Configured)
        {
            uint32_t cd_size_rndup;
            int cfcount;

            // Use configuration parameters received via
            // the Ring 97 Configuration (adapter_ring_eip202.h) interface
            if(CDR_Settings.Params.DscrOffsWordCount &
                    ((1 << Adapter_Ring_EIP202_HDW) - 1))
            {
                LOG_CRIT("Adapter_PECDev_Init: Error, "
                         "Command Descriptor Offset %d"
                         " is not an integer multiple of Host Data Width %d\n",
                         CDR_Settings.Params.DscrOffsWordCount,
                         Adapter_Ring_EIP202_HDW);

                Adapter_PECDev_UnInit(InterfaceId);
                return PEC_ERROR_INTERNAL;
            }

            // Round up to the next multiple of HDW words
            cd_size_rndup = (CDR_Settings.Params.DscrOffsWordCount +
                                ((1 << Adapter_Ring_EIP202_HDW) - 1)) >>
                                         Adapter_Ring_EIP202_HDW;

            // Half of number of full descriptors that fit FIFO
            // Note: Adapter_Ring_EIP202_CFSize is in HDW words
            cfcount = (1<<(Adapter_Ring_EIP202_CFSize-1)) / cd_size_rndup;

            // Check if command descriptor fits in fetch FIFO
            if(cfcount <= 0)
                cfcount = 1; // does not fit, adjust the count

            // Note: cfcount must be also checked for not exceeding
            //       max DMA length

            // Convert to 32-bits word counts
            CDR_Settings.Params.DscrFetchSizeWordCount =
                (cfcount-1)* (cd_size_rndup << Adapter_Ring_EIP202_HDW);
            CDR_Settings.Params.DscrThresholdWordCount =
                     cfcount * (cd_size_rndup << Adapter_Ring_EIP202_HDW);
        }
        else
#endif // #ifndef ADAPTER_EIP202_AUTO_THRESH_DISABLE
        {
            // Use default static (user-defined) configuration parameters
#ifdef ADAPTER_EIP202_CDR_DSCR_FETCH_WORD_COUNT
            CDR_Settings.Params.DscrFetchSizeWordCount =
                                    ADAPTER_EIP202_CDR_DSCR_FETCH_WORD_COUNT;
#else
            CDR_Settings.Params.DscrFetchSizeWordCount = CDOffset;
#endif
            CDR_Settings.Params.DscrThresholdWordCount =
                                    ADAPTER_EIP202_CDR_DSCR_THRESH_WORD_COUNT;
        }

        LOG_CRIT("Adapter_PECDev_Init: CDR fetch size %d, threshold %d, "
                 "HDW=%d, CFsize=%d\n",
                 CDR_Settings.Params.DscrFetchSizeWordCount,
                 CDR_Settings.Params.DscrThresholdWordCount,
                 Adapter_Ring_EIP202_HDW,
                 Adapter_Ring_EIP202_CFSize);

        // CDR Interrupts will be enabled via the Event Mgmt API functions
        CDR_Settings.Params.IntThresholdDscrCount = 0;
        CDR_Settings.Params.IntTimeoutDscrCount = 0;
        if ((CDR_Settings.Params.DscrFetchSizeWordCount /
             CDR_Settings.Params.DscrOffsWordCount) >
            (CDR_Settings.Params.DscrThresholdWordCount /
             CDR_Settings.Params.DscrSizeWordCount))
        {
            LOG_CRIT("Adapter_PECDev_Init: CDR Threshold lower than fetch size"
                     " incorrect setting\n");
            Adapter_PECDev_UnInit(InterfaceId);
            return PEC_ERROR_BAD_PARAMETER;
        }


        LOG_INFO("\n\t\t\t EIP202_CDR_Init \n");

        res = EIP202_CDR_Init(CDR_IOArea + InterfaceId,
                              CDR_Device,
                              &CDR_Settings);
        if (res != EIP202_RING_NO_ERROR)
        {
            Adapter_PECDev_UnInit(InterfaceId);
            return PEC_ERROR_INTERNAL;
        }
    }

    {
        ZEROINIT(RDR_Settings);

        RDR_Settings.Params.DataBusWidthWordCount = 1;

        // Not used for RDR
        RDR_Settings.Params.ByteSwap_DataType_Mask = 0;
        RDR_Settings.Params.ByteSwap_Token_Mask = 0;

        RDR_Settings.fContinuousScatter = InitBlock_p->fContinuousScatter;

        // Enable endianess conversion for the RDR master interface
        // if configured
        RDR_Settings.Params.ByteSwap_Packet_Mask        = 0;
#ifdef ADAPTER_EIP202_RDR_BYTE_SWAP_ENABLE
        RDR_Settings.Params.ByteSwap_Descriptor_Mask    =
                                            EIP202_RING_BYTE_SWAP_METHOD_32;
#else
        RDR_Settings.Params.ByteSwap_Descriptor_Mask    = 0;
#endif

        RDR_Settings.Params.Bufferability = 0;

#ifdef ADAPTER_EIP202_64BIT_DEVICE
        RDR_Settings.Params.DMA_AddressMode = EIP202_RING_64BIT_DMA_DSCR_PTR;
#else
        RDR_Settings.Params.DMA_AddressMode = EIP202_RING_64BIT_DMA_DISABLED;
#endif

        RDR_Settings.Params.RingSizeWordCount =
                                    RDOffset * ADAPTER_EIP202_MAX_PACKETS;

        RDR_Settings.Params.RingDMA_Handle = RDR_Handle[InterfaceId];

        if (DMAResource_Translate(RDR_Handle[InterfaceId], DMARES_DOMAIN_BUS,
                                  &PhysAddr_Pair) < 0)
        {
            Adapter_PECDev_UnInit(InterfaceId);
            return PEC_ERROR_INTERNAL;
        }
        Adapter_AddrToWordPair(PhysAddr_Pair.Address_p, 0,
                               &RDR_Settings.Params.RingDMA_Address.Addr,
                               &RDR_Settings.Params.RingDMA_Address.UpperAddr);

        RDR_Settings.Params.DscrSizeWordCount = RDWordCount;
        RDR_Settings.Params.DscrOffsWordCount = RDOffset;
        RDR_Settings.Params.TokenOffsetWordCount = RDTokenOffsWordCount;

#ifndef ADAPTER_EIP202_AUTO_THRESH_DISABLE
        if(Adapter_Ring_EIP202_Configured)
        {
            uint32_t rd_size_rndup;
            int rfcount;

            // Use configuration parameters received via
            // the Ring 97 Configuration (adapter_ring_eip202.h) interface
            if(RDR_Settings.Params.DscrOffsWordCount &
                    ((1 << Adapter_Ring_EIP202_HDW) - 1))
            {
                LOG_CRIT("Adapter_PECDev_Init: Error, "
                         "Result Descriptor Offset %d"
                         " is not an integer multiple of Host Data Width %d\n",
                         RDR_Settings.Params.DscrOffsWordCount,
                         Adapter_Ring_EIP202_HDW);

                Adapter_PECDev_UnInit(InterfaceId);
                return PEC_ERROR_INTERNAL;
            }

            // Round up to the next multiple of HDW words
            rd_size_rndup = (RDR_Settings.Params.DscrOffsWordCount +
                                ((1 << Adapter_Ring_EIP202_HDW) - 1)) >>
                                         Adapter_Ring_EIP202_HDW;

            // Half of number of full descriptors that fit FIFO
            // Note: Adapter_Ring_EIP202_RFSize is in HDW words
            rfcount = (1 << (Adapter_Ring_EIP202_RFSize - 1)) / rd_size_rndup;

            // Check if prepared result descriptor fits in fetch FIFO
            if(rfcount <= 0)
                rfcount = 1; // does not fit, adjust the count

            // Note: rfcount must be also checked for not exceeding
            //       max DMA length

            // Convert to 32-bit words counts
            RDR_Settings.Params.DscrFetchSizeWordCount =
                (rfcount - 1) * (rd_size_rndup << Adapter_Ring_EIP202_HDW);
            RDR_Settings.Params.DscrThresholdWordCount =
                     rfcount * (rd_size_rndup << Adapter_Ring_EIP202_HDW);
        }
        else
#endif // #ifndef ADAPTER_EIP202_AUTO_THRESH_DISABLE
        {
            // Use default static (user-defined) configuration parameters
            RDR_Settings.Params.DscrFetchSizeWordCount =
                                    ADAPTER_EIP202_RDR_DSCR_FETCH_WORD_COUNT;
            RDR_Settings.Params.DscrThresholdWordCount =
                                    ADAPTER_EIP202_RDR_DSCR_THRESH_WORD_COUNT;
        }

        LOG_CRIT("Adapter_PECDev_Init: RDR fetch size %d, threshold %d, "
                 "RFsize=%d\n",
                 RDR_Settings.Params.DscrFetchSizeWordCount,
                 RDR_Settings.Params.DscrThresholdWordCount,
                 Adapter_Ring_EIP202_RFSize);

        // RDR Interrupts will be enabled via the Event Mgmt API functions
        RDR_Settings.Params.IntThresholdDscrCount = 0;
        RDR_Settings.Params.IntTimeoutDscrCount = 0;

        if ((RDR_Settings.Params.DscrFetchSizeWordCount /
             RDR_Settings.Params.DscrOffsWordCount) >
            (RDR_Settings.Params.DscrThresholdWordCount /
#ifdef ADAPTER_EIP202_64BIT_DEVICE
             4 /* RDR prepared descriptor size for 64-bit */
#else
             2 /* RDR prepared descriptor size for 32-bit */
#endif
                ))
        {
            LOG_CRIT("Adapter_PECDev_Init: RDR Threshold lower than fetch size"
                     " incorrect setting\n");
            Adapter_PECDev_UnInit(InterfaceId);
            return PEC_ERROR_BAD_PARAMETER;
        }
        LOG_INFO("\n\t\t\t EIP202_RDR_Init \n");

        res = EIP202_RDR_Init(RDR_IOArea + InterfaceId,
                              RDR_Device,
                              &RDR_Settings);
        if (res != EIP202_RING_NO_ERROR)
        {
            Adapter_PECDev_UnInit(InterfaceId);
            return PEC_ERROR_INTERNAL;
        }
    }

    if (!AdapterLib_RD_OutTokens_Alloc(InterfaceId))
    {
        LOG_CRIT("Adapter_PECDev_Init: failed to allocate output tokens\n");
        Adapter_PECDev_UnInit(InterfaceId);
        return PEC_ERROR_INTERNAL; // Out of memory
    }

    AdapterLib_Ring_EIP202_Status_Report(InterfaceId);

#ifdef ADAPTER_EIP202_ADD_INIT_DIAGNOSTICS
    Adapter_Register_Dump();
#endif

    return PEC_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_UnInit
 */
PEC_Status_t
Adapter_PECDev_UnInit(
        const unsigned int InterfaceId)
{
    Device_Handle_t CDR_Device, RDR_Device;

    LOG_INFO("\n\t\t Adapter_PECDev_UnInit \n");

    if (InterfaceId >= ADAPTER_EIP202_DEVICE_COUNT)
        return PEC_ERROR_BAD_PARAMETER;

    AdapterLib_Ring_EIP202_Status_Report(InterfaceId);

#ifdef ADAPTER_EIP202_ENABLE_SCATTERGATHER
    {
        // Make a last attempt to get rid of any remaining result descriptors
        // belonging to unused scatter particles.
        uint32_t DscrDoneCount,DscrCount;

        LOG_INFO("\n\t\t\t EIP202_RDR_Descriptor_Get \n");

        EIP202_RDR_Descriptor_Get(RDR_IOArea + InterfaceId,
                                 EIP202_RDR_Entries[InterfaceId],
                                 ADAPTER_EIP202_MAX_LOGICDESCR,
                                 ADAPTER_EIP202_MAX_LOGICDESCR,
                                 &DscrDoneCount,
                                 &DscrCount);
    }
#endif

    AdapterLib_RD_OutTokens_Free(InterfaceId);

#ifdef ADAPTER_EIP202_RC_DMA_BANK_SUPPORT
#ifndef ADAPTER_EIP202_USE_SHDEVXS
    {
        // Reset the TRC base address to 0.
        Device_Handle_t Device;

        Device = Device_Find(ADAPTER_EIP202_GLOBAL_DEVICE_NAME);
        if (Device == NULL)
        {
            LOG_CRIT("Adapter_PECDev_UnInit: Could not find device\n");
            return PEC_ERROR_INTERNAL;
        }

        LOG_INFO("\n\t\t\t EIP207_RC_BaseAddr_Set \n");

        EIP207_RC_BaseAddr_Set(
            Device,
            EIP207_TRC_REG_BASE,
            EIP207_RC_SET_NR_DEFAULT,
            0,
            0);
    }
#endif
#endif // ADAPTER_EIP202_RC_DMA_BANK_SUPPORT

    CDR_Device = Device_Find(EIP202_Devices[InterfaceId].CDR_DeviceName_p);
    RDR_Device = Device_Find(EIP202_Devices[InterfaceId].RDR_DeviceName_p);

    if (CDR_Device == NULL || RDR_Device == NULL)
        return PEC_ERROR_INTERNAL;

    LOG_INFO("\n\t\t\t EIP202_CDR_Reset \n");

    EIP202_CDR_Reset(CDR_IOArea + InterfaceId,
                    CDR_Device);

    LOG_INFO("\n\t\t\t EIP202_RDR_Reset \n");

    EIP202_RDR_Reset(RDR_IOArea + InterfaceId,
                    RDR_Device);

    if (RDR_Handle[InterfaceId] != NULL)
    {
        DMAResource_Release(RDR_Handle[InterfaceId]);
        RDR_Handle[InterfaceId] = NULL;
    }

    if (CDR_Handle[InterfaceId] != NULL)
    {
        DMAResource_Release(CDR_Handle[InterfaceId]);
        CDR_Handle[InterfaceId] = NULL;
    }

#ifdef ADAPTER_EIP202_INTERRUPTS_ENABLE
    Adapter_Interrupt_SetHandler(EIP202_Devices[InterfaceId].RDR_IRQ_ID, NULL);
    Adapter_Interrupt_SetHandler(EIP202_Devices[InterfaceId].CDR_IRQ_ID, NULL);
#endif

    return PEC_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Resume
 */
int
Adapter_PECDev_Resume(
        const unsigned int InterfaceId)
{
    EIP202_Ring_Error_t res;

    LOG_INFO("\n\t\t %s \n", __func__);

    LOG_INFO("\n\t\t\t EIP202_CDR_Init \n");
    // Restore EIP-202 CDR
    res = EIP202_CDR_Init(CDR_IOArea + InterfaceId,
                          Device_Find(EIP202_Devices[InterfaceId].CDR_DeviceName_p),
                          &CDR_Settings);
    if (res != EIP202_RING_NO_ERROR)
    {
        LOG_CRIT("%s: EIP202_CDR_Init() error %d", __func__, res);
        return -1;
    }

    LOG_INFO("\n\t\t\t EIP202_RDR_Init \n");
    // Restore EIP-202 RDR
    res = EIP202_RDR_Init(RDR_IOArea + InterfaceId,
                          Device_Find(EIP202_Devices[InterfaceId].RDR_DeviceName_p),
                          &RDR_Settings);
    if (res != EIP202_RING_NO_ERROR)
    {
        LOG_CRIT("%s: EIP202_CDR_Init() error %d", __func__, res);
        return -2;
    }

#ifdef ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE
#ifdef ADAPTER_EIP202_RC_DMA_BANK_SUPPORT
#ifndef ADAPTER_EIP202_USE_SHDEVXS
    LOG_INFO("\n\t\t\t EIP207_RC_BaseAddr_Set \n");
    // Restore EIP-207 Record Cache base address
    EIP207_RC_BaseAddr_Set(
        Device_Find(ADAPTER_EIP202_GLOBAL_DEVICE_NAME),
        EIP207_TRC_REG_BASE,
        EIP207_RC_SET_NR_DEFAULT,
        EIP202_SABaseAddr,
        EIP202_SABaseUpperAddr);
#endif
#endif // ADAPTER_EIP202_RC_DMA_BANK_SUPPORT
#endif // ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE

#ifdef ADAPTER_EIP202_INTERRUPTS_ENABLE
#ifndef ADAPTER_EIP202_USE_SHDEVXS
    // Restore RDR interrupt
    if (EIP202_Interrupts[InterfaceId] & BIT_0)
        Adapter_PECDev_Enable_ResultIRQ(InterfaceId);

    // Restore CDR interrupt
    if (EIP202_Interrupts[InterfaceId] & BIT_1)
        Adapter_PECDev_Enable_CommandIRQ(InterfaceId);
#endif // ADAPTER_EIP202_USE_SHDEVXS
#endif // ADAPTER_EIP202_INTERRUPTS_ENABLE

    return 0; // success
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Suspend
 */
int
Adapter_PECDev_Suspend(
        const unsigned int InterfaceId)
{
    LOG_INFO("\n\t\t %s \n", __func__);

#ifdef ADAPTER_EIP202_INTERRUPTS_ENABLE
#ifndef ADAPTER_EIP202_USE_SHDEVXS
    // Disable RDR interrupt
    if (EIP202_Interrupts[InterfaceId] & BIT_0)
    {
        Adapter_PECDev_Disable_ResultIRQ(InterfaceId);

        // Remember that interrupt was enabled
        EIP202_Interrupts[InterfaceId] |= BIT_0;
    }

    // Disable CDR interrupt
    if (EIP202_Interrupts[InterfaceId] & BIT_1)
    {
        Adapter_PECDev_Disable_CommandIRQ(InterfaceId);

        // Remember that interrupt was enabled
        EIP202_Interrupts[InterfaceId] |= BIT_1;
    }
#endif // ADAPTER_EIP202_USE_SHDEVXS
#endif // ADAPTER_EIP202_INTERRUPTS_ENABLE

#ifdef ADAPTER_EIP202_RC_DMA_BANK_SUPPORT
#ifndef ADAPTER_EIP202_USE_SHDEVXS
    {
        LOG_INFO("\n\t\t\t EIP207_RC_BaseAddr_Set \n");

        // Reset the TRC base address to 0
        EIP207_RC_BaseAddr_Set(
            Device_Find(ADAPTER_EIP202_GLOBAL_DEVICE_NAME),
            EIP207_TRC_REG_BASE,
            EIP207_RC_SET_NR_DEFAULT,
            0,
            0);
    }
#endif
#endif // ADAPTER_EIP202_RC_DMA_BANK_SUPPORT

    LOG_INFO("\n\t\t\t EIP202_CDR_Reset \n");

    EIP202_CDR_Reset(CDR_IOArea + InterfaceId,
                     Device_Find(EIP202_Devices[InterfaceId].CDR_DeviceName_p));

    LOG_INFO("\n\t\t\t EIP202_RDR_Reset \n");

    EIP202_RDR_Reset(RDR_IOArea + InterfaceId,
                     Device_Find(EIP202_Devices[InterfaceId].RDR_DeviceName_p));

    return 0; // success
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_SA_Prepare
 */
PEC_Status_t
Adapter_PECDev_SA_Prepare(
        const DMABuf_Handle_t SAHandle,
        const DMABuf_Handle_t StateHandle,
        const DMABuf_Handle_t ARC4Handle)
{
    DMAResource_Handle_t SA_DMAHandle;

    IDENTIFIER_NOT_USED(StateHandle.p);
    IDENTIFIER_NOT_USED(ARC4Handle.p);

    if (DMABuf_Handle_IsSame(&SAHandle, &DMABuf_NULLHandle))
        return PEC_ERROR_BAD_PARAMETER;
    else
    {
        DMAResource_Record_t * Rec_p;

        SA_DMAHandle = Adapter_DMABuf_Handle2DMAResourceHandle(SAHandle);
        Rec_p = DMAResource_Handle2RecordPtr(SA_DMAHandle);

        if (Rec_p == NULL)
            return PEC_ERROR_INTERNAL;

#ifdef ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE
        if (Rec_p->Props.Bank != ADAPTER_EIP202_BANK_SA)
        {
            LOG_CRIT("PEC_SA_Register: Invalid bank for SA\n");
            return PEC_ERROR_BAD_PARAMETER;
        }
#endif

#ifndef ADAPTER_EIP202_USE_LARGE_TRANSFORM_DISABLE
        {
            uint32_t FirstWord = DMAResource_Read32(SA_DMAHandle, 0);
            // Register in the DMA resource record whether the transform
            // is large.
            if ( (FirstWord & ADAPTER_EIP202_TR_ISLARGE) != 0)
            {
                Rec_p->fIsLargeTransform = true;
                DMAResource_Write32(SA_DMAHandle,
                                    0,
                                    FirstWord & ~ADAPTER_EIP202_TR_ISLARGE);
                // Clear that bit in the SA record itself.
            }
            else
            {
                Rec_p->fIsLargeTransform = false;
            }
        }
#endif

        Rec_p->fIsNewSA = true;
    }

    return PEC_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_SA_Remove
 */
PEC_Status_t
Adapter_PECDev_SA_Remove(
        const DMABuf_Handle_t SAHandle,
        const DMABuf_Handle_t StateHandle,
        const DMABuf_Handle_t ARC4Handle)
{
    IDENTIFIER_NOT_USED(StateHandle.p);

    if (DMABuf_Handle_IsSame(&SAHandle, &DMABuf_NULLHandle))
        return PEC_ERROR_BAD_PARAMETER;

#ifdef ADAPTER_EIP202_RC_DIRECT_INVALIDATE_SUPPORT
    // Invalidate the record in the EIP-207 Transform Record Cache
    // or/and ARC4 State Record Cache
    // Not configured = disabled
    if (Adapter_RC_EIP207_Configured)
    {
#ifndef ADAPTER_EIP202_USE_SHDEVXS
        Device_Handle_t Device;

        Device = Device_Find(ADAPTER_EIP202_GLOBAL_DEVICE_NAME);
        if (Device == NULL)
        {
            LOG_CRIT("Adapter_PECDev_SA_Remove: Could not find device\n");
            return PEC_ERROR_INTERNAL;
        }
#endif

        if (Adapter_RC_EIP207_TRC_Enabled)
        {
            EIP202_DeviceAddress_t DMA_Addr;

            Adapter_GetPhysAddr(SAHandle, &DMA_Addr);

#ifdef ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE
            DMA_Addr.Addr -= EIP202_SABaseAddr;
#endif

            // Invalidate the SA Record in the TRC
#ifdef ADAPTER_EIP202_USE_SHDEVXS
            LOG_INFO("\n\t\t\t SHDevXS_TRC_Record_Invalidate \n");

            SHDevXS_TRC_Record_Invalidate(DMA_Addr.Addr);
#else
            LOG_INFO("\n\t\t\t EIP207_RC_Record_Update \n");

            EIP207_RC_Record_Update(
                    Device,
                    EIP207_TRC_REG_BASE,
                    EIP207_RC_SET_NR_DEFAULT,
                    DMA_Addr.Addr,
                    EIP207_RC_CMD_SET_BITS,
                    EIP207_RC_REG_DATA_BYTE_OFFSET - 3 * sizeof(uint32_t),
                    EIP207_RC_HDR_WORD_3_RELOAD_BIT);
#endif // ADAPTER_EIP202_USE_SHDEVXS
        }

        if (!DMABuf_Handle_IsSame(&ARC4Handle, &DMABuf_NULLHandle) &&
            Adapter_RC_EIP207_ARC4RC_Enabled)
        {
            EIP202_DeviceAddress_t DMA_Addr;

            Adapter_GetPhysAddr(ARC4Handle, &DMA_Addr);

            // Invalidate the ARC4 State record in the TRC or ARC4RC
#ifdef ADAPTER_EIP202_USE_SHDEVXS
            LOG_INFO("\n\t\t\t SHDevXS_ARC4RC_Record_Invalidate \n");

            SHDevXS_ARC4RC_Record_Invalidate(DMA_Addr.Addr);
#else
            LOG_INFO("\n\t\t\t EIP207_RC_Record_Update \n");

            EIP207_RC_Record_Update(
                    Device,
                    EIP207_ARC4RC_REG_BASE,
                    EIP207_RC_SET_NR_DEFAULT,
                    DMA_Addr.Addr,
                    EIP207_RC_CMD_SET_BITS,
                    EIP207_RC_REG_DATA_BYTE_OFFSET - 3 * sizeof(uint32_t),
                    EIP207_RC_HDR_WORD_3_RELOAD_BIT);
#endif // ADAPTER_EIP202_USE_SHDEVXS
        }
    }
#else
    IDENTIFIER_NOT_USED(ARC4Handle.p);
#endif // ADAPTER_EIP202_RC_DIRECT_INVALIDATE_SUPPORT

    return PEC_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_GetFreeSpace
 */
unsigned int
Adapter_PECDev_GetFreeSpace(
        const unsigned int InterfaceId)
{
    unsigned int FreeCDR, FreeRDR, FilledCDR, FilledRDR;
    EIP202_Ring_Error_t res;

    LOG_INFO("\n\t\t Adapter_PECDev_GetFreeSpace \n");

    if (InterfaceId >= ADAPTER_EIP202_DEVICE_COUNT)
        return PEC_ERROR_BAD_PARAMETER;

    LOG_INFO("\n\t\t\t EIP202_CDR_FillLevel_Get \n");

    res = EIP202_CDR_FillLevel_Get(CDR_IOArea + InterfaceId,
                                  &FilledCDR);
    if (res != EIP202_RING_NO_ERROR)
        return 0;

    if (FilledCDR > ADAPTER_EIP202_MAX_PACKETS)
        return 0;

    FreeCDR = ADAPTER_EIP202_MAX_PACKETS - FilledCDR;

    if (EIP202_ContinuousScatter[InterfaceId])
    {
        return FreeCDR;
    }
    else
    {
        LOG_INFO("\n\t\t\t EIP202_RDR_FillLevel_Get \n");

        res = EIP202_RDR_FillLevel_Get(RDR_IOArea + InterfaceId,
                                       &FilledRDR);
        if (res != EIP202_RING_NO_ERROR)
            return 0;

        if (FilledRDR > ADAPTER_EIP202_MAX_PACKETS)
            return 0;

        FreeRDR = ADAPTER_EIP202_MAX_PACKETS - FilledRDR;

        return MIN(FreeCDR, FreeRDR);
    }
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_PacketPut
 */
PEC_Status_t
Adapter_PECDev_Packet_Put(
        const unsigned int InterfaceId,
        const PEC_CommandDescriptor_t * Commands_p,
        const unsigned int CommandsCount,
        unsigned int * const PutCount_p)
{
    unsigned int CmdLp;
#ifdef ADAPTER_EIP202_STRICT_ARGS
    unsigned int FreeCDR,FreeRDR;
#endif
    unsigned int FilledCDR, FilledRDR, CDRIndex=0, RDRIndex=0;
    unsigned int Submitted = 0;
    EIP202_Ring_Error_t res;
    EIP202_CDR_Control_t CDR_Control;
    EIP202_RDR_Prepared_Control_t RDR_Control;

    LOG_INFO("\n\t\t Adapter_PECDev_Packet_Put \n");

    *PutCount_p = 0;
    if (InterfaceId >= ADAPTER_EIP202_DEVICE_COUNT)
        return PEC_ERROR_BAD_PARAMETER;

#ifdef ADAPTER_EIP202_STRICT_ARGS
    LOG_INFO("\n\t\t\t EIP202_CDR_FillLevel_Get \n");

    res = EIP202_CDR_FillLevel_Get(CDR_IOArea + InterfaceId,
                                  &FilledCDR);
    if (res != EIP202_RING_NO_ERROR)
        return PEC_ERROR_INTERNAL;

    if (FilledCDR > ADAPTER_EIP202_MAX_PACKETS)
        return PEC_ERROR_INTERNAL;

    FreeCDR = ADAPTER_EIP202_MAX_PACKETS - FilledCDR;

    LOG_INFO("\n\t\t\t EIP202_RDR_FillLevel_Get \n");

    if (!EIP202_ContinuousScatter[InterfaceId])
    {
        res = EIP202_RDR_FillLevel_Get(RDR_IOArea + InterfaceId,
                                       &FilledRDR);
        if (res != EIP202_RING_NO_ERROR)
            return PEC_ERROR_INTERNAL;

        if (FilledRDR > ADAPTER_EIP202_MAX_PACKETS)
        return PEC_ERROR_INTERNAL;

        FreeRDR = ADAPTER_EIP202_MAX_PACKETS - FilledRDR;
        FreeRDR = MIN(ADAPTER_EIP202_MAX_LOGICDESCR, FreeRDR);
    }
    else
    {
        FreeRDR = 1;
    }
    FreeCDR = MIN(ADAPTER_EIP202_MAX_LOGICDESCR, FreeCDR);
#endif

    for (CmdLp = 0; CmdLp < CommandsCount; CmdLp++)
    {
        uint8_t TokenWordCount;
        EIP202_DeviceAddress_t SA_PhysAddr;

#ifdef ADAPTER_EIP202_STRICT_ARGS
        if (CDRIndex == FreeCDR || (!EIP202_ContinuousScatter[InterfaceId] && RDRIndex == FreeRDR))
            break; // Run out of free descriptors in any of the rings.
#endif

        if (!Commands_p[CmdLp].InputToken_p)
        {
            LOG_CRIT("Adapter_PECDev_Packet_Put: failed, missing input token "
                     "for command descriptor %d\n",
                     CmdLp);
            return PEC_ERROR_BAD_PARAMETER;
        }

        // Prepare (first) descriptor, except for source pointer/size.
        EIP202_CDR_Entries[InterfaceId][CDRIndex].SrcPacketByteCount =
                                            Commands_p[CmdLp].SrcPkt_ByteCount;

        if (DMABuf_Handle_IsSame(&Commands_p[CmdLp].Token_Handle,
                                 &DMABuf_NULLHandle))
        {
            TokenWordCount = 0;
        }
        else
        {
            // Look-aside use case. token is created by the caller
            if (Commands_p[CmdLp].Token_WordCount > 255)
                return PEC_ERROR_INTERNAL;

            TokenWordCount = (uint8_t)Commands_p[CmdLp].Token_WordCount;
        }

        CDR_Control.TokenWordCount = TokenWordCount;

        Adapter_GetPhysAddr(Commands_p[CmdLp].Token_Handle,
              &(EIP202_CDR_Entries[InterfaceId][CDRIndex].TokenDataAddr));

        if (!AdapterLib_Ring_EIP202_DMAAddr_IsSane(
                &EIP202_CDR_Entries[InterfaceId][CDRIndex].TokenDataAddr,
                "token buffer"))
            return PEC_ERROR_INTERNAL;

        CDR_Control.fFirstSegment = true;
        CDR_Control.fLastSegment  = false;
        CDR_Control.fForceEngine = (Commands_p[CmdLp].Control2 & BIT_5) != 0;
        CDR_Control.EngineId = Commands_p[CmdLp].Control2 & MASK_5_BITS;

        EIP202_CDR_Entries[InterfaceId][CDRIndex].Token_p =
                                            Commands_p[CmdLp].InputToken_p;

        Adapter_GetPhysAddr(Commands_p[CmdLp].SA_Handle1, &SA_PhysAddr);

        if (SA_PhysAddr.Addr != ADAPTER_EIP202_DUMMY_DMA_ADDRESS_LO ||
            SA_PhysAddr.UpperAddr != ADAPTER_EIP202_DUMMY_DMA_ADDRESS_HI)
        {
            DMAResource_Handle_t DMAHandle =
                Adapter_DMABuf_Handle2DMAResourceHandle(
                                        Commands_p[CmdLp].SA_Handle1);
            DMAResource_Record_t * Rec_p =
                                    DMAResource_Handle2RecordPtr(DMAHandle);
            if (Rec_p == NULL)
                return PEC_ERROR_INTERNAL;

#ifdef ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE
            if (Rec_p->Props.Bank != ADAPTER_EIP202_BANK_SA)
            {
                LOG_CRIT("PEC_Packet_Put: Invalid bank for SA\n");
                return PEC_ERROR_BAD_PARAMETER;
            }
#endif

            if (IOToken_SAReuse_Update(!Rec_p->fIsNewSA,
                                       Commands_p[CmdLp].InputToken_p) < 0)
                return PEC_ERROR_INTERNAL;

            if (Rec_p->fIsNewSA)
                Rec_p->fIsNewSA = false;
        }

#ifdef ADAPTER_EIP202_USE_POINTER_TYPES
        if (SA_PhysAddr.Addr != ADAPTER_EIP202_DUMMY_DMA_ADDRESS_LO ||
            SA_PhysAddr.UpperAddr != ADAPTER_EIP202_DUMMY_DMA_ADDRESS_HI)
        {
#ifndef ADAPTER_EIP202_USE_LARGE_TRANSFORM_DISABLE
            DMAResource_Handle_t DMAHandle =
                Adapter_DMABuf_Handle2DMAResourceHandle(
                                        Commands_p[CmdLp].SA_Handle1);
            DMAResource_Record_t * Rec_p =
                            DMAResource_Handle2RecordPtr(DMAHandle);

            if (Rec_p->fIsLargeTransform)
                SA_PhysAddr.Addr |= ADAPTER_EIP202_TR_LARGE_ADDRESS;
            else
#endif
                SA_PhysAddr.Addr |= ADAPTER_EIP202_TR_ADDRESS;
        }
#endif

#ifdef ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE
#ifdef ADAPTER_EIP202_RC_DMA_BANK_SUPPORT
        if (SA_PhysAddr.Addr != ADAPTER_EIP202_DUMMY_DMA_ADDRESS ||
            SA_PhysAddr.UpperAddr != ADAPTER_EIP202_DUMMY_DMA_ADDRESS_HI)
            SA_PhysAddr.Addr -= EIP202_SABaseAddr;

        SA_PhysAddr.UpperAddr = 0;
#endif // ADAPTER_EIP202_RC_DMA_BANKS_SUPPORT
#endif // ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE

        EIP202_CDR_Entries[InterfaceId][CDRIndex].ContextDataAddr = SA_PhysAddr;

        if (!AdapterLib_Ring_EIP202_DMAAddr_IsSane(&SA_PhysAddr, "SA buffer"))
            return PEC_ERROR_INTERNAL;

        {
            IOToken_PhysAddr_t tkn_pa;

            tkn_pa.Lo = SA_PhysAddr.Addr;
            tkn_pa.Hi = SA_PhysAddr.UpperAddr;

            if (IOToken_SAAddr_Update(&tkn_pa,
                                      Commands_p[CmdLp].InputToken_p) < 0)
                return PEC_ERROR_INTERNAL;
        }

        RDR_Control.fFirstSegment           = true;
        RDR_Control.fLastSegment            = false;
        RDR_Control.ExpectedResultWordCount = 0;

#ifdef ADAPTER_EIP202_ENABLE_SCATTERGATHER
        {
            unsigned int GatherParticles;
            unsigned int ScatterParticles;
            unsigned int RequiredCDR, RequiredRDR;
            unsigned int i;
            unsigned int GatherByteCount;

            PEC_SGList_GetCapacity(Commands_p[CmdLp].SrcPkt_Handle,
                                   &GatherParticles);
            if (EIP202_ContinuousScatter[InterfaceId])
                ScatterParticles = 1;
            else
                PEC_SGList_GetCapacity(Commands_p[CmdLp].DstPkt_Handle,
                                       &ScatterParticles);

            if (GatherParticles == 0)
                RequiredCDR = 1;
            else
                RequiredCDR = GatherParticles;

            if (ScatterParticles == 0)
                RequiredRDR = 1;
            else
                RequiredRDR = ScatterParticles;

#ifndef ADAPTER_EIP202_SEPARATE_RINGS
            // If using overlapping rings, require an equal number of CDR
            // and RDR entries for the packet, the maximum of both.
            RequiredCDR = MAX(RequiredCDR,RequiredRDR);
            RequiredRDR = RequiredCDR;
#endif
            /* Check whether it will fit into the rings and the
             * prepared descriptor arrays.*/
#ifdef ADAPTER_EIP202_STRICT_ARGS
            if (CDRIndex + RequiredCDR > FreeCDR ||
                RDRIndex + RequiredRDR > FreeRDR)
                break;
#endif

            if (GatherParticles > 0)
            {
                GatherByteCount = Commands_p[CmdLp].SrcPkt_ByteCount;
                for (i=0; i<GatherParticles; i++)
                {
                    DMABuf_Handle_t ParticleHandle;
                    uint8_t * DummyPtr;
                    unsigned int ParticleSize;

                    PEC_SGList_Read(Commands_p[CmdLp].SrcPkt_Handle,
                                    i,
                                    &ParticleHandle,
                                    &ParticleSize,
                                    &DummyPtr);

                    Adapter_GetPhysAddr(ParticleHandle,
                      &(EIP202_CDR_Entries[InterfaceId][CDRIndex+i].SrcPacketAddr));

                    if (!AdapterLib_Ring_EIP202_DMAAddr_IsSane(
                            &EIP202_CDR_Entries[InterfaceId][CDRIndex+i].SrcPacketAddr,
                            "source packet buffer"))
                        return PEC_ERROR_INTERNAL;

                    if (ParticleSize > GatherByteCount)
                        ParticleSize = GatherByteCount;
                    GatherByteCount -= ParticleSize;
                    // Limit the total size of the gather particles to the
                    // actual packet length.

                    CDR_Control.fLastSegment = (RequiredCDR == i + 1);
                    CDR_Control.SegmentByteCount = ParticleSize;
                    EIP202_CDR_Entries[InterfaceId][CDRIndex+i].ControlWord =
                        EIP202_CDR_Write_ControlWord(&CDR_Control);
                    CDR_Control.fFirstSegment = false;
                    CDR_Control.TokenWordCount = 0;
                }
            }
            else
            { /* No gather, use single source buffer */

                Adapter_GetPhysAddr(Commands_p[CmdLp].SrcPkt_Handle,
                 &(EIP202_CDR_Entries[InterfaceId][CDRIndex].SrcPacketAddr));

                if (!AdapterLib_Ring_EIP202_DMAAddr_IsSane(
                        &EIP202_CDR_Entries[InterfaceId][CDRIndex].SrcPacketAddr,
                        "source packet buffer"))
                    return PEC_ERROR_INTERNAL;

                CDR_Control.fLastSegment = (RequiredCDR == 1);
                CDR_Control.SegmentByteCount =
                    Commands_p[CmdLp].SrcPkt_ByteCount;
                EIP202_CDR_Entries[InterfaceId][CDRIndex].ControlWord =
                    EIP202_CDR_Write_ControlWord(&CDR_Control);

                CDR_Control.fFirstSegment = false;
                CDR_Control.TokenWordCount = 0;
                i = 1;
            }

            /* Add any dummy segments for overlapping rings */
            for ( ; i<RequiredCDR; i++)
            {

                CDR_Control.fLastSegment = (RequiredCDR == i + 1);
                CDR_Control.SegmentByteCount = 0;
                EIP202_CDR_Entries[InterfaceId][CDRIndex+i].ControlWord =
                    EIP202_CDR_Write_ControlWord(&CDR_Control);
            }

            if (!EIP202_ContinuousScatter[InterfaceId])
            {
                if (ScatterParticles > 0)
                {
                    for (i=0; i<ScatterParticles; i++)
                    {
                        DMABuf_Handle_t ParticleHandle;
                        uint8_t * DummyPtr;
                        unsigned int ParticleSize;

                        PEC_SGList_Read(Commands_p[CmdLp].DstPkt_Handle,
                                        i,
                                        &ParticleHandle,
                                        &ParticleSize,
                                        &DummyPtr);

                        Adapter_GetPhysAddr(ParticleHandle,
                                            &(EIP202_RDR_Prepared[InterfaceId][RDRIndex+i].DstPacketAddr));

                        if (!AdapterLib_Ring_EIP202_DMAAddr_IsSane(
                                &EIP202_RDR_Prepared[InterfaceId][RDRIndex+i].DstPacketAddr,
                                "destination packet buffer"))
                            return PEC_ERROR_INTERNAL;

                        RDR_Control.fLastSegment = (RequiredRDR == i + 1);
                        RDR_Control.PrepSegmentByteCount = ParticleSize;
                        EIP202_RDR_Prepared[InterfaceId][RDRIndex+i].PrepControlWord =
                            EIP202_RDR_Write_Prepared_ControlWord(&RDR_Control);

                        RDR_Control.fFirstSegment = false;
                    }
                }
                else
                { /* No scatter, use single destination buffer */
                    DMAResource_Handle_t *DMAHandle =
                    Adapter_DMABuf_Handle2DMAResourceHandle(
                        Commands_p[CmdLp].DstPkt_Handle);
                    DMAResource_Record_t * Rec_p;

                    if (DMAResource_IsValidHandle(DMAHandle))
                        Rec_p  = DMAResource_Handle2RecordPtr(DMAHandle);
                    else
                        Rec_p = NULL;

                    // Check if NULL packet pointers are allowed
                    // for record invalidation commands
#ifndef ADAPTER_EIP202_INVALIDATE_NULL_PKT_POINTER
                    if (Rec_p == NULL)
                        return PEC_ERROR_INTERNAL;
#endif

                    Adapter_GetPhysAddr(Commands_p[CmdLp].DstPkt_Handle,
                                        &(EIP202_RDR_Prepared[InterfaceId][RDRIndex].DstPacketAddr));

                    if (!AdapterLib_Ring_EIP202_DMAAddr_IsSane(
                      &EIP202_RDR_Prepared[InterfaceId][RDRIndex].DstPacketAddr,
                      "destination packet buffer"))
                    return PEC_ERROR_INTERNAL;

                    RDR_Control.fLastSegment = (RequiredRDR==1);

#ifdef ADAPTER_EIP202_INVALIDATE_NULL_PKT_POINTER
                    // For NULL packet pointers only, record cache invalidation
                    if (Rec_p == NULL)
                        RDR_Control.PrepSegmentByteCount = 0;
                    else
#endif
                        RDR_Control.PrepSegmentByteCount = Rec_p->Props.Size;

                    EIP202_RDR_Prepared[InterfaceId][RDRIndex].PrepControlWord =
                        EIP202_RDR_Write_Prepared_ControlWord(&RDR_Control);

                    RDR_Control.fFirstSegment = false;
                    i = 1;
                }

                /* Add any dummy segments for overlapping rings */
                for ( ; i<RequiredRDR; i++)
                {
                    RDR_Control.fLastSegment = (RequiredRDR == i + 1);
                    RDR_Control.PrepSegmentByteCount = 0;
                    EIP202_RDR_Prepared[InterfaceId][RDRIndex+i].PrepControlWord =
                        EIP202_RDR_Write_Prepared_ControlWord(&RDR_Control);
                }
                RDRIndex += RequiredRDR;
            }
            CDRIndex += RequiredCDR;
        }
#else
        {
            // Prepare source and destination buffer in non-SG case.
            DMAResource_Handle_t *DMAHandle =
                Adapter_DMABuf_Handle2DMAResourceHandle(
                    Commands_p[CmdLp].DstPkt_Handle);
            DMAResource_Record_t * Rec_p;

            if (DMAResource_IsValidHandle(DMAHandle))
                Rec_p  = DMAResource_Handle2RecordPtr(DMAHandle);
            else
                Rec_p = NULL;

            // Check if NULL packet pointers are allowed
            // for record invalidation commands
#ifndef ADAPTER_EIP202_INVALIDATE_NULL_PKT_POINTER
            if (Rec_p == NULL)
                return PEC_ERROR_INTERNAL;
#endif

            Adapter_GetPhysAddr(Commands_p[CmdLp].SrcPkt_Handle,
                 &(EIP202_CDR_Entries[InterfaceId][CDRIndex].SrcPacketAddr));

            if (!AdapterLib_Ring_EIP202_DMAAddr_IsSane(
                  &EIP202_CDR_Entries[InterfaceId][CDRIndex].SrcPacketAddr,
                  "source packet buffer"))
                return PEC_ERROR_INTERNAL;

            CDR_Control.fLastSegment     = true;
            CDR_Control.SegmentByteCount = Commands_p[CmdLp].SrcPkt_ByteCount;

            EIP202_CDR_Entries[InterfaceId][CDRIndex].ControlWord =
                                EIP202_CDR_Write_ControlWord(&CDR_Control);

            if (!EIP202_ContinuousScatter[InterfaceId])
            {
                Adapter_GetPhysAddr(Commands_p[CmdLp].DstPkt_Handle,
                                    &(EIP202_RDR_Prepared[InterfaceId][RDRIndex].DstPacketAddr));

                if (!AdapterLib_Ring_EIP202_DMAAddr_IsSane(
                        &EIP202_RDR_Prepared[InterfaceId][RDRIndex].DstPacketAddr,
                        "destination packet buffer"))
                    return PEC_ERROR_INTERNAL;

                RDR_Control.fLastSegment = true;

#ifdef ADAPTER_EIP202_INVALIDATE_NULL_PKT_POINTER
                // For NULL packet pointers only, record cache invalidation
                if (Rec_p == NULL)
                    RDR_Control.PrepSegmentByteCount = 0;
                else
#endif
                    RDR_Control.PrepSegmentByteCount = Rec_p->Props.Size;

                EIP202_RDR_Prepared[InterfaceId][RDRIndex].PrepControlWord =
                    EIP202_RDR_Write_Prepared_ControlWord(&RDR_Control);

                RDRIndex +=1;
            }
            CDRIndex +=1;
        }
#endif
        *PutCount_p += 1;
    } // for, CommandsCount, CmdLp


    if (!EIP202_ContinuousScatter[InterfaceId])
    {
        LOG_INFO("\n\t\t\t EIP202_RDR_Descriptor_Prepare \n");
        res = EIP202_RDR_Descriptor_Prepare(RDR_IOArea + InterfaceId,
                                            EIP202_RDR_Prepared[InterfaceId],
                                            RDRIndex,
                                            &Submitted,
                                            &FilledRDR);
        if (res != EIP202_RING_NO_ERROR || Submitted != RDRIndex)
        {
            LOG_CRIT("Adapter_PECDev_Packet_Put: writing prepared descriptors"
                     "error code %d count=%d expected=%d\n",
                     res, Submitted, RDRIndex);
            return PEC_ERROR_INTERNAL;
        }
    }
    LOG_INFO("\n\t\t\t EIP202_CDR_Descriptor_Put \n");

    res = EIP202_CDR_Descriptor_Put(CDR_IOArea + InterfaceId,
                                   EIP202_CDR_Entries[InterfaceId],
                                   CDRIndex,
                                   &Submitted,
                                   &FilledCDR);
    if (res != EIP202_RING_NO_ERROR || Submitted != CDRIndex)
    {
        LOG_CRIT("Adapter_PECDev_Packet_Put: writing command descriptors"
                 "error code %d count=%d expected=%d\n",
                 res, Submitted, CDRIndex);
        return PEC_ERROR_INTERNAL;
    }

    return PEC_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Packet_Get
 */
PEC_Status_t
Adapter_PECDev_Packet_Get(
        const unsigned int InterfaceId,
        PEC_ResultDescriptor_t * Results_p,
        const unsigned int ResultsLimit,
        unsigned int * const GetCount_p)
{
    unsigned int ResLp;
    unsigned int DscrCount;
    unsigned int DscrDoneCount;
    unsigned int ResIndex;

    EIP202_Ring_Error_t res;

    LOG_INFO("\n\t\t Adapter_PECDev_Packet_Get \n");

    *GetCount_p = 0;

    if (ResultsLimit == 0)
        return PEC_STATUS_OK;

    if (InterfaceId >= ADAPTER_EIP202_DEVICE_COUNT)
        return PEC_ERROR_BAD_PARAMETER;

    LOG_INFO("\n\t\t\t EIP202_RDR_Descriptor_Get \n");

    // Assume that we do get the requested number of descriptors
    // as they were reported available.
    res = EIP202_RDR_Descriptor_Get(RDR_IOArea + InterfaceId,
                                    EIP202_RDR_Entries[InterfaceId],
                                    ResultsLimit, // Max number of packets.
                                    ADAPTER_EIP202_MAX_LOGICDESCR,
                                    &DscrDoneCount,
                                    &DscrCount);
    if (res != EIP202_RING_NO_ERROR)
        return PEC_ERROR_INTERNAL;

    ResIndex = 0;
    for (ResLp = 0; ResLp < ResultsLimit; ResLp++)
    {
        EIP202_RDR_Result_Control_t ControlWord;
        bool fEncounteredFirst = false;

        if (ResIndex >= DscrDoneCount)
            break;

        for (;;)
        {
            LOG_INFO("\n\t\t\t EIP202_RDR_Read_Processed_ControlWord \n");

            EIP202_RDR_Read_Processed_ControlWord(
                                EIP202_RDR_Entries[InterfaceId] + ResIndex,
                                &ControlWord,
                                NULL);

            if ( ControlWord.fFirstSegment)
            {
                fEncounteredFirst = true;
                Results_p[ResLp].NumParticles = 0;
            }
            Results_p[ResLp].NumParticles++;

            if ( ControlWord.fLastSegment && fEncounteredFirst)
                break; // Last segment of packet, only valid when
                       // first segment was encountered.
            ResIndex++;

            // There may be unused scatter particles after the last segment
            // that must be skipped.
            if (ResIndex >= DscrDoneCount)
                return PEC_STATUS_OK;
        }

        // Presence of Output Token placeholder is optional
        if (Results_p[ResLp].OutputToken_p)
        {
            // Copy Output Token from EIP-202 to PEC result descriptor
            memcpy(Results_p[ResLp].OutputToken_p,
                   EIP202_RDR_Entries[InterfaceId][ResIndex].Token_p,
                   IOToken_OutWordCount_Get() * sizeof (uint32_t));

            IOToken_PacketLegth_Get(Results_p[ResLp].OutputToken_p,
                                    &Results_p[ResLp].DstPkt_ByteCount);

            IOToken_BypassLegth_Get(Results_p[ResLp].OutputToken_p,
                                    &Results_p[ResLp].Bypass_WordCount);
        }

        // Copy the first EIP-202 result descriptor word, contains
        // - Particle byte count,
        // - Buffer overflow (BIT_21) and Descriptor overflow (BIT_20) errors
        // - First segment (BIT_23) and Last segment (BIT_22) indicators
        // - Descriptor overflow word count if BIT_20 is set
        Results_p[ResLp].Status1 =
              EIP202_RDR_Entries[InterfaceId][ResIndex].ProcControlWord;

        *GetCount_p += 1;
        ResIndex++;
    }

    return PEC_STATUS_OK;
}


#ifdef ADAPTER_EIP202_ENABLE_SCATTERGATHER
/*----------------------------------------------------------------------------
 * Adapter_PECDev_TestSG
 */
bool
Adapter_PECDev_TestSG(
    const unsigned int InterfaceId,
    const unsigned int GatherParticleCount,
    const unsigned int ScatterParticleCount)
{
    unsigned int GCount = GatherParticleCount;
    unsigned int SCount = ScatterParticleCount;
    unsigned int FreeCDR, FreeRDR, FilledCDR, FilledRDR;
    EIP202_Ring_Error_t res;

    if (InterfaceId >= ADAPTER_EIP202_DEVICE_COUNT)
        return false;

    if (GCount == 0)
        GCount = 1;

    if (SCount == 0)
        SCount = 1;

#ifndef ADAPTER_EIP202_SEPARATE_RINGS
    GCount = MAX(GCount, SCount);
    SCount = GCount;
#endif

    if (GCount > ADAPTER_EIP202_MAX_LOGICDESCR ||
        SCount > ADAPTER_EIP202_MAX_LOGICDESCR)
        return false;

    LOG_INFO("\n\t\t\t EIP202_CDR_FillLevel_Get \n");

    res = EIP202_CDR_FillLevel_Get(CDR_IOArea + InterfaceId,
                                  &FilledCDR);
    if (res != EIP202_RING_NO_ERROR)
        return false;

    if (FilledCDR > ADAPTER_EIP202_MAX_PACKETS)
        return false;

    FreeCDR = ADAPTER_EIP202_MAX_PACKETS - FilledCDR;

    LOG_INFO("\n\t\t\t EIP202_RDR_FillLevel_Get \n");

    if (EIP202_ContinuousScatter[InterfaceId])
    {
        return (FreeCDR >= GCount);
    }
    else
    {
        res = EIP202_RDR_FillLevel_Get(RDR_IOArea + InterfaceId,
                                           &FilledRDR);
        if (res != EIP202_RING_NO_ERROR)
            return false;

        if (FilledRDR > ADAPTER_EIP202_MAX_PACKETS)
            return false;

        FreeRDR = ADAPTER_EIP202_MAX_PACKETS - FilledRDR;

        return (FreeCDR >= GCount && FreeRDR >= SCount);
    }
}
#endif // ADAPTER_EIP202_ENABLE_SCATTERGATHER


#ifdef ADAPTER_EIP202_INTERRUPTS_ENABLE
/* Adapter_PECDev_IRQToInteraceID
 */
unsigned int
Adapter_PECDev_IRQToInferfaceId(
        const int nIRQ)
{
    unsigned int i, IRQ_Nr;

    if (nIRQ < 0)
        return 0;

    IRQ_Nr = (unsigned int)nIRQ;

    for (i = 0; i < ADAPTER_EIP202_DEVICE_COUNT; i++)
    {
        if (IRQ_Nr == EIP202_Devices[i].RDR_IRQ_ID ||
            IRQ_Nr == EIP202_Devices[i].CDR_IRQ_ID)
        {
            return i;
        }
    }

    LOG_CRIT("Adapter_PECDev_IRQToInterfaceId: unknown interrupt %d\n",nIRQ);

    return 0;
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Enable_ResultIRQ
 */
void
Adapter_PECDev_Enable_ResultIRQ(
        const unsigned int InterfaceId)
{
    if (InterfaceId >= ADAPTER_EIP202_DEVICE_COUNT)
        return;

    LOG_INFO(
      "\n\t\t\t EIP202_RDR_Processed_FillLevel_High_INT_ClearAndDisable \n");

    EIP202_RDR_Processed_FillLevel_High_INT_ClearAndDisable (
        RDR_IOArea + InterfaceId,
        false);

    LOG_INFO("\n\t\t\t EIP202_RDR_Processed_FillLevel_High_INT_Enable \n");

    EIP202_RDR_Processed_FillLevel_High_INT_Enable(
        RDR_IOArea + InterfaceId,
        ADAPTER_EIP202_DESCRIPTORDONECOUNT,
        ADAPTER_EIP202_DESCRIPTORDONETIMEOUT,
        true);

    Adapter_Interrupt_Enable(EIP202_Devices[InterfaceId].RDR_IRQ_ID,
                             EIP202_Devices[InterfaceId].RDR_IRQ_Flags);

    EIP202_Interrupts[InterfaceId] |= BIT_0;
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Disable_ResultIRQ
 */
void
Adapter_PECDev_Disable_ResultIRQ(
        const unsigned int InterfaceId)
{
    if (InterfaceId >= ADAPTER_EIP202_DEVICE_COUNT)
        return;

    LOG_INFO(
       "\n\t\t\t EIP202_RDR_Processed_FillLevel_High_INT_ClearAndDisable \n");

    EIP202_RDR_Processed_FillLevel_High_INT_ClearAndDisable (
        RDR_IOArea + InterfaceId,
        false);

    Adapter_Interrupt_Disable(EIP202_Devices[InterfaceId].RDR_IRQ_ID,
                              EIP202_Devices[InterfaceId].RDR_IRQ_Flags);

    EIP202_Interrupts[InterfaceId] &= ~BIT_0;
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Enable_CommandIRQ
 */
void
Adapter_PECDev_Enable_CommandIRQ(
        const unsigned int InterfaceId)
{
    if (InterfaceId >= ADAPTER_EIP202_DEVICE_COUNT)
        return;

    LOG_INFO("\n\t\t\t EIP202_CDR_FillLevel_Low_INT_Enable \n");

    EIP202_CDR_FillLevel_Low_INT_Enable(
        CDR_IOArea + InterfaceId,
        ADAPTER_EIP202_DESCRIPTORDONECOUNT,
        ADAPTER_EIP202_DESCRIPTORDONETIMEOUT);

    Adapter_Interrupt_Enable(EIP202_Devices[InterfaceId].CDR_IRQ_ID,
                             EIP202_Devices[InterfaceId].CDR_IRQ_Flags);

    EIP202_Interrupts[InterfaceId] |= BIT_1;
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Disable_CommandIRQ
 */
void
Adapter_PECDev_Disable_CommandIRQ(
        const unsigned int InterfaceId)
{
    if (InterfaceId >= ADAPTER_EIP202_DEVICE_COUNT)
        return;

    LOG_INFO("\n\t\t\t EIP202_CDR_FillLevel_Low_INT_ClearAndDisable \n");

    EIP202_CDR_FillLevel_Low_INT_ClearAndDisable(
        CDR_IOArea + InterfaceId);

    Adapter_Interrupt_Disable(EIP202_Devices[InterfaceId].CDR_IRQ_ID,
                              EIP202_Devices[InterfaceId].CDR_IRQ_Flags);

    EIP202_Interrupts[InterfaceId] &= ~BIT_1;
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_SetResultHandler
 */
void Adapter_PECDev_SetResultHandler(
        const unsigned int InterfaceId,
        Adapter_InterruptHandler_t HandlerFunction)
{
    if (InterfaceId >= ADAPTER_EIP202_DEVICE_COUNT)
        return;

    Adapter_Interrupt_SetHandler(EIP202_Devices[InterfaceId].RDR_IRQ_ID,
                                 HandlerFunction);
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_SetCommandHandler
 */
void Adapter_PECDev_SetCommandHandler(
        const unsigned int InterfaceId,
        Adapter_InterruptHandler_t HandlerFunction)
{
    if (InterfaceId >= ADAPTER_EIP202_DEVICE_COUNT)
        return;

    Adapter_Interrupt_SetHandler(EIP202_Devices[InterfaceId].CDR_IRQ_ID,
                                 HandlerFunction);
}
#endif // ADAPTER_EIP202_INTERRUPTS_ENABLE


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Scatter_Preload
 */
PEC_Status_t
Adapter_PECDev_Scatter_Preload(
        const unsigned int InterfaceId,
        DMABuf_Handle_t * Handles_p,
        const unsigned int HandlesCount)
{
    unsigned int i;
    EIP202_RDR_Prepared_Control_t RDR_Control;
    EIP202_Ring_Error_t res;
    unsigned int Submitted,FilledRDR;
    for (i = 0; i <  HandlesCount; i++)
    {
        DMABuf_Handle_t ParticleHandle = Handles_p[i];
        DMAResource_Handle_t DMAHandle = Adapter_DMABuf_Handle2DMAResourceHandle(ParticleHandle);
        DMAResource_Record_t * Rec_p = DMAResource_Handle2RecordPtr(DMAHandle);

        RDR_Control.fFirstSegment           = true;
#ifdef ADAPTER_EIP202_ENABLE_SCATTERGATHER
        RDR_Control.fLastSegment            = false;
#else
        RDR_Control.fLastSegment            = true;
        // When No scatter gather supported, packets must fit into single buffer.
#endif
        RDR_Control.PrepSegmentByteCount = Rec_p->Props.Size;
        RDR_Control.ExpectedResultWordCount = 0;
        EIP202_RDR_Prepared[InterfaceId][i].PrepControlWord =
            EIP202_RDR_Write_Prepared_ControlWord(&RDR_Control);
        Adapter_GetPhysAddr(ParticleHandle,
                            &(EIP202_RDR_Prepared[InterfaceId][i].DstPacketAddr));
    }
    res = EIP202_RDR_Descriptor_Prepare(RDR_IOArea + InterfaceId,
                                       EIP202_RDR_Prepared[InterfaceId],
                                       HandlesCount,
                                       &Submitted,
                                       &FilledRDR);
    if (res != EIP202_RING_NO_ERROR || Submitted != HandlesCount)
    {
        LOG_CRIT("Adapter_PECDev_Packet_Put: writing prepared descriptors"
                 "error code %d count=%d expected=%d\n",
                 res, Submitted, HandlesCount);
        return PEC_ERROR_INTERNAL;
    }

    return PEC_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Put_Dump
 */
void
Adapter_PECDev_Put_Dump(
        const unsigned int InterfaceId,
        const unsigned int FirstSlotId,
        const unsigned int LastSlotId,
        const bool fDumpCDRAdmin,
        const bool fDumpCDRCache)
{
    void * va;
    unsigned int i, CDOffset;
    uint32_t Word32;
    EIP202_RingAdmin_t RingAdmin;
    DMAResource_AddrPair_t Addr_Pair;

    if (InterfaceId >= ADAPTER_EIP202_DEVICE_COUNT)
        return;

    if (FirstSlotId >= ADAPTER_EIP202_MAX_PACKETS ||
        LastSlotId >= ADAPTER_EIP202_MAX_PACKETS)
        return;

    if (FirstSlotId > LastSlotId)
        return;

    AdapterLib_Ring_EIP202_CDR_Status_Report(InterfaceId);

    ZEROINIT(RingAdmin);
    EIP202_CDR_Dump(CDR_IOArea + InterfaceId, &RingAdmin);
    CDOffset = RingAdmin.DescOffsWordCount;

    ZEROINIT(Addr_Pair);
    DMAResource_Translate(CDR_Handle[InterfaceId],
                          DMARES_DOMAIN_HOST,
                          &Addr_Pair);
    va = Addr_Pair.Address_p;

    if (fDumpCDRAdmin)
    {
        void * pa;

        ZEROINIT(Addr_Pair);
        DMAResource_Translate(CDR_Handle[InterfaceId],
                              DMARES_DOMAIN_BUS,
                              &Addr_Pair);
        pa = Addr_Pair.Address_p;

        LOG_CRIT("\n\tCDR admin data, all sizes in 32-bit words:\n"
                 "\tSeparate:         %s\n"
                 "\tRing size:        %d\n"
                 "\tDescriptor size:  %d\n"
                 "\tInput ring size:  %d\n"
                 "\tInput ring tail:  %d\n"
                 "\tOutput ring size: %d\n"
                 "\tOutput ring head: %d\n"
                 "\tRing phys addr:   %p\n"
                 "\tRing host addr:   %p\n",
                 RingAdmin.fSeparate ? "yes" : "no",
                 RingAdmin.RingSizeWordCount,
                 RingAdmin.DescOffsWordCount,
                 RingAdmin.IN_Size,
                 RingAdmin.IN_Tail,
                 RingAdmin.OUT_Size,
                 RingAdmin.OUT_Head,
                 pa,
                 va);
    }

    LOG_CRIT("\n\tCDR dump, first slot %d, last slot %d\n",
             FirstSlotId,
             LastSlotId);

    for (i = FirstSlotId; i <= LastSlotId; i++)
    {
        unsigned int j;

        LOG_CRIT("\tDescriptor %d:\n", i);
        for (j = 0; j < CDOffset; j++)
        {
            Word32 = DMAResource_Read32(CDR_Handle[InterfaceId],
                                        i * CDOffset + j);

            LOG_CRIT("\tCD[%02d] word[%02d] 0x%08x\n",
                     i,
                     j,
                     Word32);
        }
    }

    if (!fDumpCDRCache)
        return;

    LOG_CRIT("\n\tCDR cache dump, size %d entries\n",
             ADAPTER_EIP202_MAX_LOGICDESCR);

    for (i = 0; i < ADAPTER_EIP202_MAX_LOGICDESCR; i++)
    {
        unsigned int j;

        LOG_CRIT("\tDescriptor %d cache:\n", i);

        Word32 = EIP202_CDR_Entries[InterfaceId][i].ControlWord;
        LOG_CRIT("\tCDC[%02d] word[00] 0x%08x\n", i, Word32);

        Word32 = EIP202_CDR_Entries[InterfaceId][i].SrcPacketAddr.Addr;
        LOG_CRIT("\tCDC[%02d] word[02] 0x%08x\n", i, Word32);

        Word32 = EIP202_CDR_Entries[InterfaceId][i].SrcPacketAddr.UpperAddr;
        LOG_CRIT("\tCDC[%02d] word[03] 0x%08x\n", i, Word32);

        Word32 = EIP202_CDR_Entries[InterfaceId][i].TokenDataAddr.Addr;
        LOG_CRIT("\tCDC[%02d] word[04] 0x%08x\n", i, Word32);

        Word32 = EIP202_CDR_Entries[InterfaceId][i].TokenDataAddr.UpperAddr;
        LOG_CRIT("\tCDC[%02d] word[05] 0x%08x\n", i, Word32);

        if (EIP202_CDR_Entries[InterfaceId][i].Token_p)
            for (j = 0; j < IOToken_InWordCount_Get(); j++)
            {
                Word32 = EIP202_CDR_Entries[InterfaceId][i].Token_p[j];
                LOG_CRIT("\tCDC[%02d] word[%02d] 0x%08x\n", i, 6 + j, Word32);
            }
    }

    return;
}


/*----------------------------------------------------------------------------
 * Adapter_PECDev_Get_Dump
 */
void
Adapter_PECDev_Get_Dump(
        const unsigned int InterfaceId,
        const unsigned int FirstSlotId,
        const unsigned int LastSlotId,
        const bool fDumpRDRAdmin,
        const bool fDumpRDRCache)
{
    void * va;
    unsigned int i, RDOffset;
    uint32_t Word32;
    EIP202_RingAdmin_t RingAdmin;
    DMAResource_AddrPair_t Addr_Pair;

    if (InterfaceId >= ADAPTER_EIP202_DEVICE_COUNT)
        return;

    if (FirstSlotId >= ADAPTER_EIP202_MAX_PACKETS ||
        LastSlotId >= ADAPTER_EIP202_MAX_PACKETS)
        return;

    if (FirstSlotId > LastSlotId)
        return;

    AdapterLib_Ring_EIP202_RDR_Status_Report(InterfaceId);

    ZEROINIT(RingAdmin);
    EIP202_RDR_Dump(RDR_IOArea + InterfaceId, &RingAdmin);
    RDOffset = RingAdmin.DescOffsWordCount;

    ZEROINIT(Addr_Pair);
    DMAResource_Translate(RDR_Handle[InterfaceId],
                          DMARES_DOMAIN_HOST,
                          &Addr_Pair);
    va = Addr_Pair.Address_p;

    if (fDumpRDRAdmin)
    {
        void * pa;

        ZEROINIT(Addr_Pair);
        DMAResource_Translate(RDR_Handle[InterfaceId],
                              DMARES_DOMAIN_BUS,
                              &Addr_Pair);
        pa = Addr_Pair.Address_p;

        LOG_CRIT("\n\tRDR admin data, all sizes in 32-bit words:\n"
                 "\tSeparate:         %s\n"
                 "\tRing size:        %d\n"
                 "\tDescriptor size:  %d\n"
                 "\tInput ring size:  %d\n"
                 "\tInput ring tail:  %d\n"
                 "\tOutput ring size: %d\n"
                 "\tOutput ring head: %d\n"
                 "\tRing phys addr:   %p\n"
                 "\tRing host addr:   %p\n",
                 RingAdmin.fSeparate ? "yes" : "no",
                 RingAdmin.RingSizeWordCount,
                 RingAdmin.DescOffsWordCount,
                 RingAdmin.IN_Size,
                 RingAdmin.IN_Tail,
                 RingAdmin.OUT_Size,
                 RingAdmin.OUT_Head,
                 pa,
                 va);
    }

    LOG_CRIT("\n\tRDR dump, first slot %d, last slot %d\n",
             FirstSlotId,
             LastSlotId);

    for (i = FirstSlotId; i <= LastSlotId; i++)
    {
        unsigned int j;

        LOG_CRIT("\tDescriptor %d:\n", i);
        for (j = 0; j < RDOffset; j++)
        {
            Word32 = DMAResource_Read32(RDR_Handle[InterfaceId],
                                        i * RDOffset + j);

            LOG_CRIT("\tRD[%02d] word[%02d] 0x%08x\n",
                     i,
                     j,
                     Word32);
        }
    }

    if (!fDumpRDRCache)
        return;

    LOG_CRIT("\n\tRDR cache dump, size %d entries\n",
             ADAPTER_EIP202_MAX_LOGICDESCR);

    for (i = 0; i < ADAPTER_EIP202_MAX_LOGICDESCR; i++)
    {
        unsigned int j;

        LOG_CRIT("\tDescriptor %d cache:\n", i);

        Word32 = EIP202_RDR_Entries[InterfaceId][i].ProcControlWord;
        LOG_CRIT("\tRDC[%02d] word[00] 0x%08x\n", i, Word32);

        Word32 = EIP202_RDR_Entries[InterfaceId][i].DstPacketAddr.Addr;
        LOG_CRIT("\tRDC[%02d] word[02] 0x%08x\n", i, Word32);

        Word32 = EIP202_RDR_Entries[InterfaceId][i].DstPacketAddr.UpperAddr;
        LOG_CRIT("\tRDC[%02d] word[03] 0x%08x\n", i, Word32);

        for (j = 0; j < IOToken_OutWordCount_Get(); j++)
        {
            Word32 = EIP202_RDR_Entries[InterfaceId][i].Token_p[j];
            LOG_CRIT("\tRDC[%02d] word[%02d] 0x%08x\n", i, 4 + j, Word32);
        }
    }

    return;
}


/* end of file adapter_ring_eip202.c */
