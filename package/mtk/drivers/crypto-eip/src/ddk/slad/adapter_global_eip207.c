/* adapter_global_eip207.c
 *
 * Security-IP-207 Global Control Adapter
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

// Classification (EIP-207) Global Control Initialization API
#include "api_global_eip207.h"

// Classification (EIP-207) Global Control Status API
#include "api_global_status_eip207.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_adapter_cs.h"

#ifndef GLOBALCONTROL_BUILD
#include "adapter_rc_eip207.h"  // Record Cache EIP-207 interface to pass
                                // config params from Global Control
#endif

// Global Control API
#include "api_global_eip97.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"          // uint8_t, uint32_t, bool

// Driver Framework C Library API
#include "clib.h"                // memcpy, ZEROINIT

// EIP-207 Driver Library Global Control API
#include "eip207_global_init.h"  // Init/Uninit/Status/FW download

// EIP-207 Driver Library Global Control API: Configuration
#include "eip207_global_config.h" // EIP207_Global_MetaData_Configure

#include "device_types.h"        // Device_Handle_t
#include "device_mgmt.h"         // Device_find

// Logging API
#include "log.h"                 // Log_*, LOG_*

// Firmware load API.
#include "adapter_firmware.h"
#include "firmware_eip207_api_dwld.h"

// Runtime Power Management Device Macros API
#include "rpm_device_macros.h"  // RPM_*

// EIP97_Supported_Funcs_Get()
#include "eip97_global_init.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Support legacy firmware packages without name parameters in download API */
#ifndef FIRMWARE_EIP207_IPUE_NAME
#define FIRMWARE_EIP207_IPUE_NAME "firmware_eip207_ipue.bin"
#define FIRMWARE_EIP207_IFPP_NAME "firmware_eip207_ifpp.bin"
#endif
#ifndef FIRMWARE_EIP207_OPUE_NAME
#define FIRMWARE_EIP207_OPUE_NAME "firmware_eip207_opue.bin"
#define FIRMWARE_EIP207_OFPP_NAME "firmware_eip207_ofpp.bin"
#endif

/*----------------------------------------------------------------------------
 * Local variables
 */

static EIP207_Global_IOArea_t Global_IOArea;
static bool Global_IsInitialized;

// Cached values during initialization will be used for RPM device resume
static EIP207_Global_CacheConfig_t RC_Conf;
static EIP207_Global_FLUEConfig_t FLUE_Conf;

static const  GlobalControl207_Capabilities_t Global_CapabilitiesString =
{
    "EIP-207 v_._p_  #cache sets=__ #lookup tables=__" // szTextDescription
};


/*----------------------------------------------------------------------------
 * YesNo
 */
static const char *
YesNo(
        const bool b)
{
    if (b)
        return "Yes";
    else
        return "No";
}


/*----------------------------------------------------------------------------
 * GlobalControl207Lib_Init
 *
 */
static int
GlobalControl207Lib_Init(void)
{
    EIP207_Global_Error_t rc;
    unsigned int i;

    LOG_INFO("\n\t\t\t\t EIP207_Global_Init \n");

    rc = EIP207_Global_Init(&Global_IOArea,
                            Device_Find(ADAPTER_CS_GLOBAL_DEVICE_NAME),
                            &RC_Conf,
                            &FLUE_Conf);

    for (i = 0; i < EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE; i++)
    {
        Log_FormattedMessage("GlobalControl_EIP207_Init cache set %d:\n"
                             "\t\tFRC  AdminWords=%5d DataWords=%5d\n"
                             "\t\tTRC  AdminWords=%5d DataWords=%5d\n"
                             "\t\tARC4 AdminWords=%5d DataWords=%5d\n",
                             i,
                             RC_Conf.FRC[i].AdminWordCount,
                             RC_Conf.FRC[i].DataWordCount,
                             RC_Conf.TRC[i].AdminWordCount,
                             RC_Conf.TRC[i].DataWordCount,
                             RC_Conf.ARC4[i].AdminWordCount,
                             RC_Conf.ARC4[i].DataWordCount);
    }

    if (rc != EIP207_GLOBAL_NO_ERROR)
    {
        LOG_CRIT("%s: EIP207_Global_Init() returned error %d\n", __func__, rc);
        return -1; // error
    }

    return 0; // success
}


/*----------------------------------------------------------------------------
 * GlobalControl207Lib_Firmware_Load
 *
 */
static int
GlobalControl207Lib_Firmware_Load(bool fVerbose)
{
    Adapter_Firmware_t IPUE_Handle, IFPP_Handle,OPUE_Handle, OFPP_Handle;
    EIP207_Firmware_t IPUE_Firmware, IFPP_Firmware;
    EIP207_Firmware_t OPUE_Firmware, OFPP_Firmware;
    EIP207_Global_Error_t rc;

    ZEROINIT(IPUE_Firmware);
    ZEROINIT(IFPP_Firmware);
    ZEROINIT(OPUE_Firmware);
    ZEROINIT(OFPP_Firmware);

#ifdef FIRMWARE_EIP207_VERSION_MAJOR
    // If version numbers are provided, then fill them in, so they
    // will be checked against the actual firmware, else leave them
    // at zero.
    IPUE_Firmware.Major = FIRMWARE_EIP207_VERSION_MAJOR;
    IPUE_Firmware.Minor = FIRMWARE_EIP207_VERSION_MINOR;
    IPUE_Firmware.PatchLevel = FIRMWARE_EIP207_VERSION_PATCH;
    IFPP_Firmware.Major = FIRMWARE_EIP207_VERSION_MAJOR;
    IFPP_Firmware.Minor = FIRMWARE_EIP207_VERSION_MINOR;
    IFPP_Firmware.PatchLevel = FIRMWARE_EIP207_VERSION_PATCH;
    OPUE_Firmware.Major = FIRMWARE_EIP207_VERSION_MAJOR;
    OPUE_Firmware.Minor = FIRMWARE_EIP207_VERSION_MINOR;
    OPUE_Firmware.PatchLevel = FIRMWARE_EIP207_VERSION_PATCH;
    OFPP_Firmware.Major = FIRMWARE_EIP207_VERSION_MAJOR;
    OFPP_Firmware.Minor = FIRMWARE_EIP207_VERSION_MINOR;
    OFPP_Firmware.PatchLevel = FIRMWARE_EIP207_VERSION_PATCH;
#endif
    IPUE_Handle = Adapter_Firmware_Acquire(FIRMWARE_EIP207_IPUE_NAME,
                                           &IPUE_Firmware.Image_p,
                                           &IPUE_Firmware.ImageWordCount);
    IFPP_Handle = Adapter_Firmware_Acquire(FIRMWARE_EIP207_IFPP_NAME,
                                           &IFPP_Firmware.Image_p,
                                           &IFPP_Firmware.ImageWordCount);
    if ((EIP97_SupportedFuncs_Get() & BIT_1) != 0)
    {
        OPUE_Handle = Adapter_Firmware_Acquire(FIRMWARE_EIP207_OPUE_NAME,
                                               &OPUE_Firmware.Image_p,
                                               &OPUE_Firmware.ImageWordCount);
        OFPP_Handle = Adapter_Firmware_Acquire(FIRMWARE_EIP207_OFPP_NAME,
                                               &OFPP_Firmware.Image_p,
                                               &OFPP_Firmware.ImageWordCount);
    }
    else
    {
        OPUE_Handle = Adapter_Firmware_NULL;
        OFPP_Handle = Adapter_Firmware_NULL;
    }

    LOG_INFO("\n\t\t\t\t EIP207_Global_Firmware_Load \n");

    rc = EIP207_Global_Firmware_Load(&Global_IOArea,
                                     ADAPTER_CS_TIMER_PRESCALER,
                                     &IPUE_Firmware,
                                     &IFPP_Firmware,
                                     &OPUE_Firmware,
                                     &OFPP_Firmware);
    Adapter_Firmware_Release(IPUE_Handle);
    Adapter_Firmware_Release(IFPP_Handle);
    Adapter_Firmware_Release(OPUE_Handle);
    Adapter_Firmware_Release(OFPP_Handle);
    if (rc != EIP207_GLOBAL_NO_ERROR)
    {
        LOG_CRIT("GlobalControl207_Init: "
                 "EIP207_Global_Firmware_Load() failed\n");
        return -3; // error
    }
    else if (fVerbose)
    {
        LOG_CRIT("GlobalControl207_Init: firmware "
                 "downloaded successfully\n");

        LOG_CRIT("\tIPUE firmware v%d.%d.%d, image byte count %d\n",
                 IPUE_Firmware.Major,
                 IPUE_Firmware.Minor,
                 IPUE_Firmware.PatchLevel,
                 (int)(IPUE_Firmware.ImageWordCount * sizeof(uint32_t)));

        LOG_CRIT("\tIFPP firmware v%d.%d.%d, image byte count %d\n\n",
                 IFPP_Firmware.Major,
                     IFPP_Firmware.Minor,
                 IFPP_Firmware.PatchLevel,
                 (int)(IFPP_Firmware.ImageWordCount * sizeof(uint32_t)));

        LOG_CRIT("\tOPUE firmware v%d.%d.%d, image byte count %d\n",
                 OPUE_Firmware.Major,
                 OPUE_Firmware.Minor,
                 OPUE_Firmware.PatchLevel,
                     (int)(OPUE_Firmware.ImageWordCount * sizeof(uint32_t)));

        LOG_CRIT("\tOFPP firmware v%d.%d.%d, image byte count %d\n\n",
                 OFPP_Firmware.Major,
                 OFPP_Firmware.Minor,
                 OFPP_Firmware.PatchLevel,
                 (int)(OFPP_Firmware.ImageWordCount * sizeof(uint32_t)));
    }
    return 0;
}

#ifdef ADAPTER_GLOBAL_RPM_EIP207_DEVICE_ID
/*----------------------------------------------------------------------------
 * GlobalControl207Lib_Resume
 *
 */
static int
GlobalControl207Lib_Resume(void * p)
{
    EIP207_Global_Error_t rc;
    EIP207_Firmware_t IPUE_Firmware, IFPP_Firmware;
    EIP207_Firmware_t OPUE_Firmware, OFPP_Firmware;

    IDENTIFIER_NOT_USED(p);

    if (GlobalControl207Lib_Init() != 0)
        return -1; // error

    if (GlobalControl207Lib_Firmware_Load(false) != 0)
        return -2; // error

    return 0; // success
}
#endif


/*----------------------------------------------------------------------------
 * GlobalControl207_Capabilities_Get
 */
void
GlobalControl207_Capabilities_Get(
        GlobalControl207_Capabilities_t * const Capabilities_p)
{
    uint8_t Versions[7];

    LOG_INFO("\n\t\t\t %s \n", __func__);

    memcpy(Capabilities_p, &Global_CapabilitiesString,
           sizeof(Global_CapabilitiesString));

    {
        EIP207_Global_Error_t rc;
        EIP207_Global_Capabilities_t Capabilities;

        if (RPM_DEVICE_IO_START_MACRO(ADAPTER_GLOBAL_RPM_EIP207_DEVICE_ID,
                                RPM_FLAG_SYNC) != RPM_SUCCESS)
            return;

        LOG_INFO("\n\t\t\t\t EIP207_Global_HWRevision_Get \n");

        rc = EIP207_Global_HWRevision_Get(&Global_IOArea, &Capabilities);

        (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP207_DEVICE_ID,
                                       RPM_FLAG_ASYNC);

        if (rc != EIP207_GLOBAL_NO_ERROR)
        {
            LOG_CRIT("GlobalControl207_Capabilities_Get: "
                     "EIP207_Global_HWRevision_Get() failed\n");
            return;
        }

        // Show those capabilities not propagated to higher layer.
        LOG_CRIT("EIP-207 capabilities\n");
        LOG_CRIT("\tLookup cached:            %s\n"
                 "\tFRC combined with TRC:    %s\n"
                 "\tARC4RC present:           %s\n"
                 "\tFRC combined with ARC4RC: %s\n"
                 "\tTRC combined with ARC4RC: %s\n"
                 "\tFRC clients:              %d\n"
                 "\tTRC clients:              %d\n"
                 "\tARC4RC clients:           %d\n"
                 "\tLookup clients:           %d\n\n",
                 YesNo(Capabilities.EIP207_Options.fLookupCached),
                 YesNo(Capabilities.EIP207_Options.fCombinedFRC_TRC),
                 YesNo(Capabilities.EIP207_Options.fARC4Present),
                 YesNo(Capabilities.EIP207_Options.fCombinedFRC_ARC4),
                 YesNo(Capabilities.EIP207_Options.fCombinedTRC_ARC4),
                 Capabilities.EIP207_Options.NofFRC_Clients,
                 Capabilities.EIP207_Options.NofTRC_Clients,
                 Capabilities.EIP207_Options.NofARC4_Clients,
                 Capabilities.EIP207_Options.NofLookupClients);

        Versions[0] = Capabilities.EIP207_Version.MajHWRevision;
        Versions[1] = Capabilities.EIP207_Version.MinHWRevision;
        Versions[2] = Capabilities.EIP207_Version.HWPatchLevel;

        Versions[3] = Capabilities.EIP207_Options.NofCacheSets / 10;
        Versions[4] = Capabilities.EIP207_Options.NofCacheSets % 10;

        Versions[5] = Capabilities.EIP207_Options.NofLookupTables / 10;
        Versions[6] = Capabilities.EIP207_Options.NofLookupTables % 10;
    }

    {
        char * p = Capabilities_p->szTextDescription;
        int VerIndex = 0;
        int i = 0;

        while(p[i])
        {
            if (p[i] == '_')
            {
                if (Versions[VerIndex] > 9)
                    p[i] = '?';
                else
                    p[i] = '0' + Versions[VerIndex++];

                if (VerIndex >= 7)
                    break;
            }

            i++;
        }
    }

    return;
}

/*----------------------------------------------------------------------------
 * GlobalControl207_Init
 */
GlobalControl207_Error_t
GlobalControl207_Init(
        const bool fLoadFirmware,
        const GlobalControl207_IV_t * const IV_p)
{
    unsigned int i;
    GlobalControl207_Error_t GC207_Rc = EIP207_GLOBAL_CONTROL_ERROR_INTERNAL;
    EIP207_Global_Error_t rc;
    Device_Handle_t Device;
    unsigned int NofCEs,NofRings,NofLAInterfaces,NofInlineInterfaces;

    LOG_INFO("\n\t\t\t %s \n", __func__);

    if (Global_IsInitialized)
    {
        LOG_CRIT("GlobalControl207_Init: called while already initialized\n");
        return EIP207_GLOBAL_CONTROL_ERROR_BAD_USE_ORDER;
    }

    Device = Device_Find(ADAPTER_CS_GLOBAL_DEVICE_NAME);
    if (Device == NULL)
    {
        LOG_CRIT("GlobalControl207_Init: Could not find device\n");
        return EIP207_GLOBAL_CONTROL_ERROR_INTERNAL;
    }

    ZEROINIT(RC_Conf);
    ZEROINIT(FLUE_Conf);

    // Record Caches initialization parameters
    for (i = 0; i < EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE; i++)
    {
        RC_Conf.FRC[i].fEnable = (ADAPTER_CS_FRC_ENABLED != 0);
        RC_Conf.FRC[i].fNonBlock = false;
        RC_Conf.FRC[i].BlockClockCount = ADAPTER_CS_RC_BLOCK_CLOCK_COUNT;
        RC_Conf.FRC[i].RecBaseAddr.Value64_Lo = 0;
        RC_Conf.FRC[i].RecBaseAddr.Value64_Hi = 0;

        RC_Conf.TRC[i].fEnable = (ADAPTER_CS_TRC_ENABLED != 0);
        RC_Conf.TRC[i].fNonBlock = false;
        RC_Conf.TRC[i].BlockClockCount = ADAPTER_CS_RC_BLOCK_CLOCK_COUNT;
        RC_Conf.TRC[i].RecBaseAddr.Value64_Lo = 0;
        RC_Conf.TRC[i].RecBaseAddr.Value64_Hi = 0;

        RC_Conf.ARC4[i].fEnable = (ADAPTER_CS_ARC4RC_ENABLED != 0);
        RC_Conf.ARC4[i].fNonBlock = false;
        RC_Conf.ARC4[i].BlockClockCount = ADAPTER_CS_RC_BLOCK_CLOCK_COUNT;
        RC_Conf.ARC4[i].RecBaseAddr.Value64_Lo = 0;
        RC_Conf.ARC4[i].RecBaseAddr.Value64_Hi = 0;
    }

    // Flow Look-Up Engine initialization parameters
    FLUE_Conf.CacheChain = ADAPTER_CS_FLUE_CACHE_CHAIN;
    FLUE_Conf.fDelayMemXS = (ADAPTER_CS_FLUE_MEMXS_DELAY != 0);
    FLUE_Conf.IV.IV_Word32[0] = IV_p->IV[0];
    FLUE_Conf.IV.IV_Word32[1] = IV_p->IV[1];
    FLUE_Conf.IV.IV_Word32[2] = IV_p->IV[2];
    FLUE_Conf.IV.IV_Word32[3] = IV_p->IV[3];

    // Hash table initialization parameters
    FLUE_Conf.HashTablesCount = ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE;
    for (i = 0; i < FLUE_Conf.HashTablesCount; i++)
    {
        FLUE_Conf.HashTable[i].fLookupCached =
                                    (ADAPTER_CS_FLUE_LOOKUP_CACHED != 0);
        FLUE_Conf.HashTable[i].fPrefetchXform =
                                    (ADAPTER_CS_FLUE_PREFETCH_XFORM != 0);
        FLUE_Conf.HashTable[i].fPrefetchARC4State =
                                    (ADAPTER_CS_FLUE_PREFETCH_ARC4 != 0);
    }

    GlobalControl97_Interfaces_Get(&NofCEs, &NofRings, &NofLAInterfaces, &NofInlineInterfaces);
    LOG_CRIT("GlobalControl_EIP207_Init:\n"
             "Number of Rings: %u, LA Interfaces: %u, Inline interfaces: %u\n",
             NofRings,NofLAInterfaces,NofInlineInterfaces);

    FLUE_Conf.InterfacesCount = NofRings + NofLAInterfaces +
                                                    NofInlineInterfaces;
    if (FLUE_Conf.InterfacesCount == 0)
    {
        LOG_CRIT("GlobalControl207_Init: Device not initialized\n");
        return EIP207_GLOBAL_CONTROL_ERROR_INTERNAL;
    }

    for (i = 0; i < FLUE_Conf.InterfacesCount; i++)
        FLUE_Conf.InterfaceIndex[i] = 0;

    if (RPM_DEVICE_INIT_START_MACRO(ADAPTER_GLOBAL_RPM_EIP207_DEVICE_ID,
                                    NULL, // Suspend callback not used
                                    GlobalControl207Lib_Resume) != RPM_SUCCESS)
        return EIP207_GLOBAL_CONTROL_ERROR_INTERNAL;

    if (GlobalControl207Lib_Init() != 0)
        goto exit; // error

    // Configure the Record Cache functionality at the Ring Control
    {
        EIP207_Global_Capabilities_t Capabilities;

        LOG_INFO("\n\t\t\t\t EIP207_Global_HWRevision_Get \n");

        rc = EIP207_Global_HWRevision_Get(&Global_IOArea, &Capabilities);
        if (rc != EIP207_GLOBAL_NO_ERROR)
        {
            LOG_CRIT("GlobalControl207_Init: "
                     "EIP207_Global_HWRevision_Get() failed\n");
            goto exit; // error
        }

#ifndef GLOBALCONTROL_BUILD
#ifdef ADAPTER_CS_RC_SUPPORT
        Adapter_RC_EIP207_Configure(
                (ADAPTER_CS_TRC_ENABLED != 0),
                (ADAPTER_CS_ARC4RC_ENABLED != 0) &&
                                 Capabilities.EIP207_Options.fARC4Present,
                Capabilities.EIP207_Options.fCombinedTRC_ARC4);
#endif // ADAPTER_CS_RC_SUPPORT
#endif // GLOBALCONTROL_BUILD
    }

    if (fLoadFirmware)
    {
        if (GlobalControl207Lib_Firmware_Load(true) != 0)
            goto exit;
    }

    {
        EIP207_Firmware_Config_t FWConfig;
        ZEROINIT(FWConfig);
#if defined(ADAPTER_CS_GLOBAL_IOTOKEN_METADATA_ENABLE) || \
    defined(ADAPTER_CS_GLOBAL_CFH_ENABLE)
        FWConfig.fTokenExtensionsEnable = true;
#else
        FWConfig.fTokenExtensionsEnable = false;
#endif
#ifdef ADAPTER_CS_GLOBAL_INCREMENT_PKTID
        FWConfig.fIncrementPktID = true;
#else
        FWConfig.fIncrementPktID = false;
#endif
#ifdef ADAPTER_CS_GLOBAL_ECN_CONTROL
        FWConfig.ECNControl = ADAPTER_CS_GLOBAL_ECN_CONTROL;
#endif
#ifdef ADAPTER_CS_GLOBAL_DTLS_DEFER_CCS
        FWConfig.fDTLSDeferCCS = true;
#endif
#ifdef ADAPTER_CS_GLOBAL_DTLS_DEFER_ALERT
        FWConfig.fDTLSDeferAlert = true;
#endif
#ifdef ADAPTER_CS_GLOBAL_DTLS_DEFER_HANDSHAKE
        FWConfig.fDTLSDeferHandshake = true;
#endif
#ifdef ADAPTER_CS_GLOBAL_DTLS_DEFER_APPDATA
        FWConfig.fDTLSDeferAppData = true;
#endif
#ifdef ADAPTER_CS_GLOBAL_DTLS_DEFER_CAPWAP
        FWConfig.fDTLSDeferCAPWAP = true;
#endif
#ifdef ADAPTER_CS_GLOBAL_DTLS_HDR_ALIGN
        FWConfig.DTLSRecordHeaderAlign = ADAPTER_CS_GLOBAL_DTLS_HDR_ALIGN;
#endif
        FWConfig.TransformRedirEnable = ADAPTER_CS_GLOBAL_TRANSFORM_REDIRECT_ENABLE;
#ifdef ADAPTER_CS_GLOBAL_REDIR_RING
        FWConfig.fRedirRingEnable = true;
        FWConfig.RedirRing = ADAPTER_CS_GLOBAL_REDIR_RING;
#endif
        // Configure the EIP-207 Firmware meta-data or CFH presence
        // in the ICE scratch-path RAM
        for (i = 0; i < NofCEs; i++)
        {
            EIP207_Global_Firmware_Configure(Device, i, &FWConfig);
            if (FWConfig.fIncrementPktID)
            {   /* Give each engine its own range of PktID values */
                FWConfig.PktID += 4096;
            }
        }
    }
    Global_IsInitialized = true;
    GC207_Rc = EIP207_GLOBAL_CONTROL_NO_ERROR; // success

exit:
    (void)RPM_DEVICE_INIT_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP207_DEVICE_ID);

    return GC207_Rc;
}


/*----------------------------------------------------------------------------
 * GlobalControl207_UnInit
 */
GlobalControl207_Error_t
GlobalControl207_UnInit(void)
{
    LOG_INFO("\n\t\t\t %s \n", __func__);

    if (!Global_IsInitialized)
    {
        LOG_CRIT("GlobalControl207_UnInit: called while not initialized\n");
        return EIP207_GLOBAL_CONTROL_ERROR_BAD_USE_ORDER;
    }

    (void)RPM_DEVICE_UNINIT_START_MACRO(ADAPTER_GLOBAL_RPM_EIP207_DEVICE_ID, false);
    (void)RPM_DEVICE_UNINIT_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP207_DEVICE_ID);

    Global_IsInitialized = false;

    return EIP207_GLOBAL_CONTROL_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * GlobalControl207_Status_Get
 */
GlobalControl207_Error_t
GlobalControl207_Status_Get(
        const unsigned int CE_Number,
        GlobalControl207_Status_t * const Status_p)
{
    EIP207_Global_Error_t rc;
    bool fFatalError;

    LOG_INFO("\n\t\t\t %s \n", __func__);

    if (RPM_DEVICE_IO_START_MACRO(ADAPTER_GLOBAL_RPM_EIP207_DEVICE_ID,
                            RPM_FLAG_SYNC) != RPM_SUCCESS)
        return EIP207_GLOBAL_CONTROL_ERROR_INTERNAL;

    LOG_INFO("\n\t\t\t\t EIP207_Global_Status_Get \n");

    rc = EIP207_Global_Status_Get(&Global_IOArea,
                                  CE_Number,
                                  Status_p,
                                  &fFatalError);

    (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP207_DEVICE_ID,
                                   RPM_FLAG_ASYNC);

    if (rc == EIP207_GLOBAL_NO_ERROR)
    {
        if (fFatalError)
            LOG_CRIT("GlobalControl207_Status_Get: Fatal Error detected, "
                     "reset required!\n");

        return EIP207_GLOBAL_CONTROL_NO_ERROR;
    }
    else if (rc == EIP207_GLOBAL_ARGUMENT_ERROR)
        return EIP207_GLOBAL_CONTROL_ERROR_BAD_PARAMETER;
    else
        return EIP207_GLOBAL_CONTROL_ERROR_INTERNAL;
}


/*----------------------------------------------------------------------------
 * GlobalControl207_GlobalStats_Get
 */
GlobalControl207_Error_t
GlobalControl207_GlobalStats_Get(
        const unsigned int CE_Number,
        GlobalControl207_GlobalStats_t * const GlobalStats_p)
{
    EIP207_Global_Error_t rc;

    LOG_INFO("\n\t\t\t %s \n", __func__);

    if (RPM_DEVICE_IO_START_MACRO(ADAPTER_GLOBAL_RPM_EIP207_DEVICE_ID,
                            RPM_FLAG_SYNC) != RPM_SUCCESS)
        return EIP207_GLOBAL_CONTROL_ERROR_INTERNAL;

    LOG_INFO("\n\t\t\t\t EIP207_Global_GlobalStats_Get \n");

    rc = EIP207_Global_GlobalStats_Get(&Global_IOArea,
                                       CE_Number,
                                       GlobalStats_p);

    (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP207_DEVICE_ID,
                                   RPM_FLAG_ASYNC);

    if (rc == EIP207_GLOBAL_NO_ERROR)
        return EIP207_GLOBAL_CONTROL_NO_ERROR;
    else if (rc == EIP207_GLOBAL_ARGUMENT_ERROR)
        return EIP207_GLOBAL_CONTROL_ERROR_BAD_PARAMETER;
    else
        return EIP207_GLOBAL_CONTROL_ERROR_INTERNAL;
}


/*----------------------------------------------------------------------------
 * GlobalControl207_ClockCount_Get
 */
GlobalControl207_Error_t
GlobalControl207_ClockCount_Get(
        const unsigned int CE_Number,
        GlobalControl207_Clock_t * const Clock_p)
{
    EIP207_Global_Error_t rc;

    LOG_INFO("\n\t\t\t %s \n", __func__);

    if (RPM_DEVICE_IO_START_MACRO(ADAPTER_GLOBAL_RPM_EIP207_DEVICE_ID,
                            RPM_FLAG_SYNC) != RPM_SUCCESS)
        return EIP207_GLOBAL_CONTROL_ERROR_INTERNAL;

    LOG_INFO("\n\t\t\t\t EIP207_Global_ClockCount_Get \n");

    rc = EIP207_Global_ClockCount_Get(&Global_IOArea,
                                      CE_Number,
                                      Clock_p);

    (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP207_DEVICE_ID,
                                   RPM_FLAG_ASYNC);

    if (rc == EIP207_GLOBAL_NO_ERROR)
        return EIP207_GLOBAL_CONTROL_NO_ERROR;
    else if (rc == EIP207_GLOBAL_ARGUMENT_ERROR)
        return EIP207_GLOBAL_CONTROL_ERROR_BAD_PARAMETER;
    else
        return EIP207_GLOBAL_CONTROL_ERROR_INTERNAL;
}


/*--------------------- -------------------------------------------------------
 * GlobalControl207_Firmware_Configure
 */
GlobalControl207_Error_t
GlobalControl207_Firmware_Configure(
        GlobalControl_Firmware_Config_t * const FWConfig_p)
{
    unsigned i;
    Device_Handle_t Device;
    unsigned int NofCEs;
    Device = Device_Find(ADAPTER_CS_GLOBAL_DEVICE_NAME);
    GlobalControl97_Interfaces_Get(&NofCEs, NULL, NULL, NULL);
    if (FWConfig_p == NULL)
        return EIP207_GLOBAL_CONTROL_ERROR_BAD_PARAMETER;
    FWConfig_p->PktID = 0;
    // Configure the EIP-207 Firmware meta-data or CFH presence
    // in the ICE scratch-path RAM
    for (i = 0; i < NofCEs; i++)
    {
        EIP207_Global_Firmware_Configure(Device, i, FWConfig_p);
        if (FWConfig_p->fIncrementPktID)
        {   /* Give each engine its own range of PktID values */
            FWConfig_p->PktID += 4096;
        }
    }

    return EIP207_GLOBAL_CONTROL_NO_ERROR;

}


/* end of file adapter_global_eip207.c */
