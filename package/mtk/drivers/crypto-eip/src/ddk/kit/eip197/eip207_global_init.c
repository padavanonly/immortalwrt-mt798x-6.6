/* eip207_global_init.c
 *
 * EIP-207 Global Control Driver Library
 * Initialization and status retrieval Module
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : ddk197                                                   */
/*   Version       : 5.6.1                                                    */
/*   Configuration : DDK-197-GPL                                              */
/*                                                                            */
/*   Date          : 2022-Dec-16                                              */
/*                                                                            */
/* Copyright (c) 2008-2022 by Rambus, Inc. and/or its subsidiaries.           */
/*                                                                            */
/* This program is free software: you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation, either version 2 of the License, or          */
/* any later version.                                                         */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               */
/* GNU General Public License for more details.                               */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program. If not, see <http://www.gnu.org/licenses/>.       */
/* -------------------------------------------------------------------------- */

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// EIP-207 Global Control API
#include "eip207_global_init.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip207_global.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // uint32_t

// Driver Framework C Run-time Library API
#include "clib.h"                       // ZEROINIT

// Driver Framework Device API
#include "device_types.h"               // Device_Handle_t

// EIP-207s Record Cache (RC) internal interface
#include "eip207_rc_internal.h"

// EIP-207c Input Classification Engine (ICE) interface
#include "eip207_ice.h"

// EIP-207c Output Classification Engine (OCE) interface
#include "eip207_oce.h"

// EIP-207s Flow Look-Up Engine (FLUE) interface
#include "eip207_flue.h"

// EIP-207 Global Control Driver Library Internal interfaces
#include "eip207_level0.h"              // EIP-207 Level 0 macros

// EIP-207s Flow Look-Up Engine Cache (FLUEC) Level0 interface
#include "eip207_fluec_level0.h"

// EIP-207c Firmware Classification API
#include "firmware_eip207_api_cs.h"     // Classification API: General

// EIP-207c Firmware Download API
#include "firmware_eip207_api_dwld.h"   // Classification API: FW download

// EIP97 Global init API
#include "eip97_global_init.h"

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Maximum number of EIP-207c Classification Engines that should be used
// Should not exceed the number of engines physically available
#ifndef EIP207_GLOBAL_MAX_NOF_CE_TO_USE
#error "EIP207_GLOBAL_MAX_NOF_CE_TO_USE is not defined"
#endif

// Maximum number of flow hash tables that should be used
#ifndef EIP207_MAX_NOF_FLOW_HASH_TABLES_TO_USE
#error "EIP207_MAX_NOF_FLOW_HASH_TABLES_TO_USE is not defined"
#endif

// Default Classification Engine number
#define CE_DEFAULT_NR                                   0

#if (CE_DEFAULT_NR >= EIP207_GLOBAL_MAX_NOF_CE_TO_USE)
#error "Error: CE_DEFAULT_NR must be less than EIP207_GLOBAL_MAX_NOF_CE_TO_USE"
#endif

// Size of the ARC4 State Record in 32-bit words
#define EIP207_CS_ARC4RC_RECORD_WORD_COUNT              64

#ifndef FIRMWARE_EIP207_CS_ARC4RC_RECORD_WORD_COUNT
#define FIRMWARE_EIP207_CS_ARC4RC_RECORD_WORD_COUNT     64
#endif

// I/O Area, used internally
typedef struct
{
    Device_Handle_t Device;
    uint32_t State;
} EIP207_True_IOArea_t;

#define IOAREA(_p) ((volatile EIP207_True_IOArea_t *)_p)

#ifdef EIP207_GLOBAL_STRICT_ARGS
#define EIP207_GLOBAL_CHECK_POINTER(_p) \
    if (NULL == (_p)) \
        return EIP207_GLOBAL_ARGUMENT_ERROR;
#define EIP207_GLOBAL_CHECK_INT_INRANGE(_i, _min, _max) \
    if ((_i) < (_min) || (_i) > (_max)) \
        return EIP207_GLOBAL_ARGUMENT_ERROR;
#define EIP207_GLOBAL_CHECK_INT_ATLEAST(_i, _min) \
    if ((_i) < (_min)) \
        return EIP207_GLOBAL_ARGUMENT_ERROR;
#define EIP207_GLOBAL_CHECK_INT_ATMOST(_i, _max) \
    if ((_i) > (_max)) \
        return EIP207_GLOBAL_ARGUMENT_ERROR;
#else
/* EIP207_GLOBAL_STRICT_ARGS undefined */
#define EIP207_GLOBAL_CHECK_POINTER(_p)
#define EIP207_GLOBAL_CHECK_INT_INRANGE(_i, _min, _max)
#define EIP207_GLOBAL_CHECK_INT_ATLEAST(_i, _min)
#define EIP207_GLOBAL_CHECK_INT_ATMOST(_i, _max)
#endif /*end of EIP207_GLOBAL_STRICT_ARGS */

#define TEST_SIZEOF(type, size) \
    extern int size##_must_bigger[1 - 2*((int)(sizeof(type) > size))]

// validate the size of the fake and real IOArea structures
TEST_SIZEOF(EIP207_True_IOArea_t, EIP207_GLOBAL_IOAREA_REQUIRED_SIZE);

#ifdef EIP207_GLOBAL_DEBUG_FSM
// EIP-207 Global Control API States
typedef enum
{
    EIP207_GLOBAL_STATE_INITIALIZED  = 5,
    EIP207_GLOBAL_STATE_FATAL_ERROR  = 7,
    EIP207_GLOBAL_STATE_RC_ENABLED   = 8,
    EIP207_GLOBAL_STATE_FW_LOADED    = 9
} EIP207_Global_State_t;
#endif // EIP207_GLOBAL_DEBUG_FSM


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP207Lib_Detect
 *
 * Checks the presence of EIP-207 hardware. Returns true when found.
 */
static bool
EIP207Lib_Detect(
        const Device_Handle_t Device,
        const unsigned int CEnr)
{
    uint32_t Value;

    IDENTIFIER_NOT_USED(CEnr);

    Value = EIP207_Read32(Device, EIP207_CS_REG_VERSION);
    if (!EIP207_CS_SIGNATURE_MATCH( Value ))
        return false;

    return true;
}


/*----------------------------------------------------------------------------
 * EIP207Lib_HWRevision_Get
 */
static void
EIP207Lib_HWRevision_Get(
        const Device_Handle_t Device,
        EIP207_Options_t * const Options_p,
        EIP207_Version_t * const Version_p)
{
    EIP207_CS_VERSION_RD(
                      Device,
                      &Version_p->EipNumber,
                      &Version_p->ComplmtEipNumber,
                      &Version_p->HWPatchLevel,
                      &Version_p->MinHWRevision,
                      &Version_p->MajHWRevision);

    EIP207_CS_OPTIONS_RD(Device,
                         &Options_p->NofLookupTables,
                         &Options_p->fLookupCached,
                         &Options_p->NofLookupClients,
                         &Options_p->fCombinedTRC_ARC4,
                         &Options_p->fCombinedFRC_ARC4,
                         &Options_p->fARC4Present,
                         &Options_p->NofARC4_Clients,
                         &Options_p->fCombinedFRC_TRC,
                         &Options_p->NofTRC_Clients,
                         &Options_p->NofFRC_Clients,
                         &Options_p->NofCacheSets);
}


#ifdef EIP207_GLOBAL_DEBUG_FSM
/*----------------------------------------------------------------------------
 * EIP207Lib_Global_State_Set
 *
 */
static EIP207_Global_Error_t
EIP207Lib_Global_State_Set(
        EIP207_Global_State_t * const CurrentState,
        const EIP207_Global_State_t NewState)
{
    switch(*CurrentState)
    {
        case EIP207_GLOBAL_STATE_INITIALIZED:
            switch(NewState)
            {
                case EIP207_GLOBAL_STATE_RC_ENABLED:
                    *CurrentState = NewState;
                    break;
                case EIP207_GLOBAL_STATE_FATAL_ERROR:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP207_GLOBAL_ILLEGAL_IN_STATE;
            }
            break;

         case EIP207_GLOBAL_STATE_RC_ENABLED:
            switch(NewState)
            {
                case EIP207_GLOBAL_STATE_FW_LOADED:
                   *CurrentState = NewState;
                   break;
                case EIP207_GLOBAL_STATE_FATAL_ERROR:
                    *CurrentState = NewState;
                    break;
                default:
                    return EIP207_GLOBAL_ILLEGAL_IN_STATE;
            }
            break;

        default:
            return EIP207_GLOBAL_ILLEGAL_IN_STATE;
    }

    return EIP207_GLOBAL_NO_ERROR;
}
#endif // EIP207_GLOBAL_DEBUG_FSM


/*----------------------------------------------------------------------------
 * EIP207_Global_Init
 */
EIP207_Global_Error_t
EIP207_Global_Init(
        EIP207_Global_IOArea_t * const IOArea_p,
        const Device_Handle_t Device,
        EIP207_Global_CacheConfig_t * const CacheConf_p,
        const EIP207_Global_FLUEConfig_t * const FLUEConf_p)
{
    unsigned int FLUE_NofLookupTables;
    EIP207_Global_Capabilities_t Capabilities;
    volatile EIP207_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);
    unsigned int i;

    EIP207_GLOBAL_CHECK_POINTER(IOArea_p);
    EIP207_GLOBAL_CHECK_POINTER(CacheConf_p);
    EIP207_GLOBAL_CHECK_POINTER(FLUEConf_p);

    // Detect presence of EIP-207 HW hardware
    if (!EIP207Lib_Detect(Device, CE_DEFAULT_NR))
        return EIP207_GLOBAL_UNSUPPORTED_FEATURE_ERROR;

    // Initialize the IO Area
    TrueIOArea_p->Device = Device;

#ifdef EIP207_GLOBAL_DEBUG_FSM
    TrueIOArea_p->State = (uint32_t)EIP207_GLOBAL_STATE_INITIALIZED;
#endif // EIP207_GLOBAL_DEBUG_FSM

    ZEROINIT(Capabilities);

    EIP207Lib_HWRevision_Get(Device,
                             &Capabilities.EIP207_Options,
                             &Capabilities.EIP207_Version);

    FLUE_NofLookupTables = Capabilities.EIP207_Options.NofLookupTables;

    // 0 hash tables has a special meaning for the EIP-207 FLUE HW
    if (FLUE_NofLookupTables == 0)
        FLUE_NofLookupTables = EIP207_GLOBAL_MAX_HW_NOF_FLOW_HASH_TABLES;

    // Check actual configuration HW against capabilities
    // Number of configured cache sets and hash tables
    if ((Capabilities.EIP207_Options.NofCacheSets <
            EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE) ||
        (FLUE_NofLookupTables <
            EIP207_MAX_NOF_FLOW_HASH_TABLES_TO_USE)  ||
        (FLUEConf_p->HashTablesCount >
            EIP207_MAX_NOF_FLOW_HASH_TABLES_TO_USE))
        return EIP207_GLOBAL_UNSUPPORTED_FEATURE_ERROR;

    // Configure EIP-207 Classification Engine

    // Initialize Record Caches
    {
        EIP207_Global_Error_t rv;

        // Initialize Flow Record Cache
#ifndef EIP207_GLOBAL_FRC_DISABLE
        if ((EIP97_SupportedFuncs_Get() & BIT_3) != 0)
        {
            for (i=0; i < EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE; i++)
            {
                CacheConf_p->FRC[i].DataWordCount = EIP207_FRC_RAM_WORD_COUNT;
                CacheConf_p->FRC[i].AdminWordCount =
                    EIP207_FRC_ADMIN_RAM_WORD_COUNT;
            }

            rv = EIP207_RC_Internal_Init(
                Device,
                EIP207_RC_INTERNAL_NOT_COMBINED,
                EIP207_FRC_REG_BASE,
                CacheConf_p->FRC,
                FIRMWARE_EIP207_CS_FRC_RECORD_WORD_COUNT);
            if (rv != EIP207_GLOBAL_NO_ERROR)
                return rv;
        }
#endif // EIP207_GLOBAL_FRC_DISABLE
        for (i=0; i < EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE; i++)
        {
            CacheConf_p->TRC[i].DataWordCount = EIP207_TRC_RAM_WORD_COUNT;
            CacheConf_p->TRC[i].AdminWordCount =
                EIP207_TRC_ADMIN_RAM_WORD_COUNT;
        }

        // Initialize Transform Record Cache
        rv = EIP207_RC_Internal_Init(
                 Device,
                 Capabilities.EIP207_Options.fCombinedFRC_TRC ?
                         EIP207_RC_INTERNAL_FRC_TRC_COMBINED :
                                EIP207_RC_INTERNAL_NOT_COMBINED,
                 EIP207_TRC_REG_BASE,
                 CacheConf_p->TRC,
                 FIRMWARE_EIP207_CS_TRC_RECORD_WORD_COUNT);
        if (rv != EIP207_GLOBAL_NO_ERROR)
            return rv;

        // Check if ARC4 Record Cache is available
        if ( Capabilities.EIP207_Options.fARC4Present )
        {
            for (i=0; i < EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE; i++)
            {
                CacheConf_p->ARC4[i].DataWordCount =
                    EIP207_ARC4RC_RAM_WORD_COUNT;
                CacheConf_p->ARC4[i].AdminWordCount =
                    EIP207_ARC4RC_ADMIN_RAM_WORD_COUNT;
            }
            // Initialize ARC4 Record Cache
            rv = EIP207_RC_Internal_Init(
                    Device,
                    Capabilities.EIP207_Options.fCombinedFRC_ARC4 ?
                        EIP207_RC_INTERNAL_FRC_ARC4_COMBINED :
                    (Capabilities.EIP207_Options.fCombinedTRC_ARC4 ?
                          EIP207_RC_INTERNAL_TRC_ARC4_COMBINED :
                                    EIP207_RC_INTERNAL_NOT_COMBINED),
                    EIP207_ARC4RC_REG_BASE,
                    CacheConf_p->ARC4,
                    FIRMWARE_EIP207_CS_ARC4RC_RECORD_WORD_COUNT);
            if (rv != EIP207_GLOBAL_NO_ERROR)
                return rv;
        }
    }

    // Initialize Flow Hash Engine, set IV values
    EIP207_FHASH_IV_WR(Device,
                       FLUEConf_p->IV.IV_Word32[0],
                       FLUEConf_p->IV.IV_Word32[1],
                       FLUEConf_p->IV.IV_Word32[2],
                       FLUEConf_p->IV.IV_Word32[3]);

    // Initialize FLUE Hash Tables
    {
        unsigned int i;

        for (i = 0; i < FLUEConf_p->HashTablesCount; i++)
            EIP207_FLUE_Init(Device,
                             i,
                             FLUEConf_p,
                             Capabilities.EIP207_Options.fARC4Present,
                             Capabilities.EIP207_Options.fLookupCached);
    }

    // Initialize optional FLUE Cache
    if (Capabilities.EIP207_Options.fLookupCached)
    {
        EIP207_FLUEC_CTRL_WR(Device,
                             false,        // Disable cache RAM access
                             FLUEConf_p->fDelayMemXS,
                             EIP207_FLUEC_TABLE_SIZE,
                             EIP207_FLUEC_GROUP_SIZE,
                             FLUEConf_p->CacheChain);  // Cache chain
    }

#ifdef EIP207_GLOBAL_DEBUG_FSM
        {
            EIP207_Global_Error_t rv;
            uint32_t State = TrueIOArea_p->State;

            // Transit to a new state
            rv = EIP207Lib_Global_State_Set(
                    (EIP207_Global_State_t* const)&State,
                    EIP207_GLOBAL_STATE_RC_ENABLED);

            TrueIOArea_p->State = State;

            if (rv != EIP207_GLOBAL_NO_ERROR)
                return EIP207_GLOBAL_ILLEGAL_IN_STATE;
        }
#endif // EIP207_GLOBAL_DEBUG_FSM

    return EIP207_GLOBAL_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP207_Global_Firmware_Load
 */
EIP207_Global_Error_t
EIP207_Global_Firmware_Load(
        EIP207_Global_IOArea_t * const IOArea_p,
        const unsigned int TimerPrescaler,
        EIP207_Firmware_t * const IPUE_Firmware_p,
        EIP207_Firmware_t * const IFPP_Firmware_p,
        EIP207_Firmware_t * const OPUE_Firmware_p,
        EIP207_Firmware_t * const OFPP_Firmware_p)
{
    EIP207_Global_Error_t EIP207_Rc;
    Device_Handle_t Device;
    volatile EIP207_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP207_GLOBAL_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    // Download EIP-207c Input Classification Engine (ICE) firmware,
    // use the same images for all the instances of the engine
    EIP207_Rc = EIP207_ICE_Firmware_Load(Device,
                                         TimerPrescaler,
                                         IPUE_Firmware_p,
                                         IFPP_Firmware_p);
    if (EIP207_Rc != EIP207_GLOBAL_NO_ERROR)
        return EIP207_Rc;

    // Download EIP-207c Output Classification Engine (OCE) firmware,
    // use the same images for all the instances of the engine
    // Download Input Classification Engine (OCE) firmware
    EIP207_Rc = EIP207_OCE_Firmware_Load(Device,
                                             TimerPrescaler,
                                             OPUE_Firmware_p,
                                             OFPP_Firmware_p);
        // OCE is not supported by this EIP-207 HW version
    if (EIP207_Rc != EIP207_GLOBAL_UNSUPPORTED_FEATURE_ERROR &&
        EIP207_Rc != EIP207_GLOBAL_NO_ERROR) // OCE is supported
        return EIP207_Rc; // OCE FW download error, abort!

#ifdef EIP207_GLOBAL_DEBUG_FSM
        {
            EIP207_Global_Error_t rv;
            uint32_t State = TrueIOArea_p->State;

            // Transit to a new state
            rv = EIP207Lib_Global_State_Set(
                    (EIP207_Global_State_t* const)&State,
                    EIP207_GLOBAL_STATE_FW_LOADED);

            TrueIOArea_p->State = State;

            if (rv != EIP207_GLOBAL_NO_ERROR)
                return EIP207_GLOBAL_ILLEGAL_IN_STATE;
        }
#endif // EIP207_GLOBAL_DEBUG_FSM

    return EIP207_GLOBAL_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP207_Global_HWRevision_Get
 */
EIP207_Global_Error_t
EIP207_Global_HWRevision_Get(
        EIP207_Global_IOArea_t * const IOArea_p,
        EIP207_Global_Capabilities_t * const Capabilities_p)
{
    Device_Handle_t Device;
    volatile EIP207_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP207_GLOBAL_CHECK_POINTER(IOArea_p);
    EIP207_GLOBAL_CHECK_POINTER(Capabilities_p);

    Device = TrueIOArea_p->Device;

    EIP207Lib_HWRevision_Get(Device,
                             &Capabilities_p->EIP207_Options,
                             &Capabilities_p->EIP207_Version);

    return EIP207_GLOBAL_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP207_Global_GlobalStats_Get
 */
EIP207_Global_Error_t
EIP207_Global_GlobalStats_Get(
        EIP207_Global_IOArea_t * const IOArea_p,
        const unsigned int CE_Number,
        EIP207_Global_GlobalStats_t * const GlobalStats_p)
{
    Device_Handle_t Device;
    EIP207_Global_Error_t rv;
    volatile EIP207_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP207_GLOBAL_CHECK_POINTER(IOArea_p);
    EIP207_GLOBAL_CHECK_POINTER(GlobalStats_p);

    if(CE_Number >= EIP207_GLOBAL_MAX_NOF_CE_TO_USE)
        return EIP207_GLOBAL_ARGUMENT_ERROR;

    Device = TrueIOArea_p->Device;

    rv = EIP207_ICE_GlobalStats_Get(Device, CE_Number, &GlobalStats_p->ICE);
    if (rv != EIP207_GLOBAL_NO_ERROR)
        return rv;

    rv = EIP207_OCE_GlobalStats_Get(Device, CE_Number, &GlobalStats_p->OCE);
    if (rv == EIP207_GLOBAL_UNSUPPORTED_FEATURE_ERROR)
        return EIP207_GLOBAL_NO_ERROR; // OCE is not supported
    else                               // by this EIP-207 HW version
        return rv;
}


/*----------------------------------------------------------------------------
 * EIP207_Global_ClockCount_Get
 */
EIP207_Global_Error_t
EIP207_Global_ClockCount_Get(
        EIP207_Global_IOArea_t * const IOArea_p,
        const unsigned int CE_Number,
        EIP207_Global_Clock_t * const Clock_p)
{
    Device_Handle_t Device;
    EIP207_Global_Error_t rv;
    volatile EIP207_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP207_GLOBAL_CHECK_POINTER(IOArea_p);
    EIP207_GLOBAL_CHECK_POINTER(Clock_p);

    if(CE_Number >= EIP207_GLOBAL_MAX_NOF_CE_TO_USE)
        return EIP207_GLOBAL_ARGUMENT_ERROR;

    Device = TrueIOArea_p->Device;

    rv = EIP207_ICE_ClockCount_Get(Device, CE_Number, &Clock_p->ICE);
    if (rv != EIP207_GLOBAL_NO_ERROR)
        return rv;

    rv = EIP207_OCE_ClockCount_Get(Device, CE_Number, &Clock_p->OCE);
    if (rv == EIP207_GLOBAL_UNSUPPORTED_FEATURE_ERROR)
        return EIP207_GLOBAL_NO_ERROR; // OCE is not supported
    else                               // by this EIP-207 HW version
        return rv;
}


/*-----------------------------------------------------------------------------
 * EIP207_Global_Status_Get
 */
EIP207_Global_Error_t
EIP207_Global_Status_Get(
        EIP207_Global_IOArea_t * const IOArea_p,
        const unsigned int CE_Number,
        EIP207_Global_Status_t * const Status_p,
        bool * const fFatalError_p)
{
    unsigned int i;
    Device_Handle_t Device;
    EIP207_Global_Error_t rv;
    bool fFatalError = false;
    volatile EIP207_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP207_GLOBAL_CHECK_POINTER(IOArea_p);
    EIP207_GLOBAL_CHECK_POINTER(Status_p);
    EIP207_GLOBAL_CHECK_POINTER(fFatalError_p);

    if(CE_Number >= EIP207_GLOBAL_MAX_NOF_CE_TO_USE)
        return EIP207_GLOBAL_ARGUMENT_ERROR;

    Device = TrueIOArea_p->Device;

    rv = EIP207_ICE_Status_Get(Device, CE_Number, &Status_p->ICE);
    if (rv != EIP207_GLOBAL_NO_ERROR)
        return rv;

    rv = EIP207_OCE_Status_Get(Device, CE_Number, &Status_p->OCE);
    if (rv != EIP207_GLOBAL_NO_ERROR &&
        rv != EIP207_GLOBAL_UNSUPPORTED_FEATURE_ERROR)
        return rv;

    EIP207_FLUE_Status_Get(Device, &Status_p->FLUE);

    if (Status_p->ICE.fPUE_EccDerr   ||
        Status_p->ICE.fFPP_EccDerr   ||
        Status_p->OCE.fPUE_EccDerr ||
        Status_p->OCE.fFPP_EccDerr ||
        Status_p->ICE.fTimerOverflow ||
        Status_p->OCE.fTimerOverflow ||
        Status_p->FLUE.Error1 != 0   ||
        Status_p->FLUE.Error2 != 0)
        fFatalError = true;

    for (i = 0; i < EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE; i++)
    {
        EIP207_RC_Internal_Status_Get(Device,
                                      i,
                                      &Status_p->FRC[i],
                                      &Status_p->TRC[i],
                                      &Status_p->ARC4RC[i]);
        if (Status_p->FRC[i].fDMAReadError ||
            Status_p->FRC[i].fDMAWriteError ||
            Status_p->FRC[i].fAdminEccErr ||
            Status_p->FRC[i].fDataEccOflo ||
            Status_p->TRC[i].fDMAReadError ||
            Status_p->TRC[i].fDMAWriteError ||
            Status_p->TRC[i].fAdminEccErr ||
            Status_p->TRC[i].fDataEccOflo ||
            Status_p->ARC4RC[i].fDMAReadError ||
            Status_p->ARC4RC[i].fDMAWriteError ||
            Status_p->ARC4RC[i].fAdminEccErr ||
            Status_p->ARC4RC[i].fDataEccOflo)
            fFatalError = true;

        EIP207_RC_Internal_DebugStatistics_Get(Device,
                                               i,
                                               &Status_p->FRCStats[i],
                                               &Status_p->TRCStats[i]);
    }

    *fFatalError_p = fFatalError;

#ifdef EIP207_GLOBAL_DEBUG_FSM
    if (fFatalError)
    {
        uint32_t State = TrueIOArea_p->State;

        // Transit to a new state
        rv = EIP207Lib_Global_State_Set(
                (EIP207_Global_State_t* const)&State,
                EIP207_GLOBAL_STATE_FATAL_ERROR);

        TrueIOArea_p->State = State;

        if (rv != EIP207_GLOBAL_NO_ERROR)
            return EIP207_GLOBAL_ILLEGAL_IN_STATE;
    }
#endif // EIP207_GLOBAL_DEBUG_FSM

    return EIP207_GLOBAL_NO_ERROR;
}


/* end of file eip207_global_init.c */
