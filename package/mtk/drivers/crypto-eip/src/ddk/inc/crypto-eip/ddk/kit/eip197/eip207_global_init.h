/* eip207_global_init.h
 *
 * EIP-207 Global Control API:
 * Record Cache, FLUE, optional FLUEC, ICE and optional OCE initialization
 * as well as ICE and optional OCE firmware download.
 *
 * This API is not re-entrant.
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

#ifndef EIP207_GLOBAL_INIT_H_
#define EIP207_GLOBAL_INIT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip207_global.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint8_t, uint32_t, bool

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define EIP207_GLOBAL_IOAREA_REQUIRED_SIZE       (2 * sizeof(void*))

// place holder for device specific internal data
typedef struct
{
    uint32_t placeholder[EIP207_GLOBAL_IOAREA_REQUIRED_SIZE];
} EIP207_Global_IOArea_t;

/*----------------------------------------------------------------------------
 * EIP207_Global_Error_t
 *
 * Status (error) code type returned by these API functions
 * See each function "Return value" for details.
 *
 * EIP207_GLOBAL_NO_ERROR : successful completion of the call.
 * EIP207_GLOBAL_UNSUPPORTED_FEATURE_ERROR : not supported by the device.
 * EIP207_GLOBAL_ARGUMENT_ERROR :  invalid argument for a function parameter.
 * EIP207_GLOBAL_ILLEGAL_IN_STATE : illegal state transition
 * EIP207_GLOBAL_INTERNAL_ERROR : failure due to internal error
 */
typedef enum
{
    EIP207_GLOBAL_NO_ERROR = 0,
    EIP207_GLOBAL_UNSUPPORTED_FEATURE_ERROR,
    EIP207_GLOBAL_ARGUMENT_ERROR,
    EIP207_GLOBAL_ILLEGAL_IN_STATE,
    EIP207_GLOBAL_INTERNAL_ERROR
} EIP207_Global_Error_t;

// 64-bit data structure
typedef struct
{
    uint32_t Value64_Lo; // Lowest 32 bits of the 64-bit value
    uint32_t Value64_Hi; // Highest 32 bits of the 64-bit value
} EIP207_Global_Value64_t;

typedef struct
{
    // Version information
    unsigned int Major;
    unsigned int Minor;
    unsigned int PatchLevel;

    // Pointer to the memory location where the firmware image resides
    const uint32_t * Image_p;

    // Firmware image size in 32-bit words
    unsigned int ImageWordCount;

} EIP207_Firmware_t;

// EIP-207 HW version
typedef struct
{
    // The basic EIP number.
    uint8_t EipNumber;

    // The complement of the basic EIP number.
    uint8_t ComplmtEipNumber;

    // Hardware Patch Level.
    uint8_t HWPatchLevel;

    // Minor Hardware revision.
    uint8_t MinHWRevision;

    // Major Hardware revision.
    uint8_t MajHWRevision;
} EIP207_Version_t;

// EIP-207 HW options
typedef struct
{
    // Number of cache sets implemented, in range 1...3
    uint8_t NofCacheSets;

    // Number of external (non-Host) clients on the Flow Record Cache
    // (for each of the cache sets if more than one of those are present)
    uint8_t NofFRC_Clients;

    // Number of external (non-Host) clients on the Transform Record Cache
    // (for each of the cache sets if more than one of those are present)
    uint8_t NofTRC_Clients;

    // True when Transform Record Cache logic is combined with the Flow Record
    // Cache logic (i.e. they share the processing pipeline, administration
    // control logic and RAM, but have separate client states and Host client
    // access interfaces).
    bool fCombinedFRC_TRC;

    // Number of external (non-Host) clients on the ARC4 State Record Cache
    // (for each of the cache sets if more than one of those are present)
    // Set to 0 when the ARC4 State Record Cache is not present
    uint8_t NofARC4_Clients;

    // True when the ARC4 Record Cache is present
    bool fARC4Present;

    // True when ARC4 State Record Cache logic is combined with the Flow Record
    // Cache logic (i.e. they share the processing pipeline, administration
    // control logic and RAM, but have separate client states and Host client
    // access interfaces).
    // Only valid when NofARC4_Clients is not 0.
    bool fCombinedFRC_ARC4;

    // True when ARC4 State Record Cache logic is combined with the Transform
    // Record Cache logic (i.e. they share the processing pipeline,
    // administration control logic and RAM, but have separate client states
    // and Host client access interfaces).
    // Only valid when NofARC4_Clients is not 0.
    bool fCombinedTRC_ARC4;

    // Number of clients to the flow hash and lookup logic, independent from
    // the number of cache sets.
    uint8_t NofLookupClients;

    // True when the Flow lookup logic is fitted with a cache RAM.
    bool fLookupCached;

    // Number of lookup tables supported by the flow lookup logic
    uint8_t NofLookupTables;

} EIP207_Options_t;

// Capabilities structure for EIP-207 HW
typedef struct
{
    // EIP-207 HW shell
    EIP207_Options_t     EIP207_Options;
    EIP207_Version_t     EIP207_Version;

} EIP207_Global_Capabilities_t;

// Classification Engine clocks per one tick for
// the blocking next command logic
typedef enum
{
    EIP207_BLOCK_CLOCKS_16 = 0,
    EIP207_BLOCK_CLOCKS_32,
    EIP207_BLOCK_CLOCKS_64,
    EIP207_BLOCK_CLOCKS_128,
    EIP207_BLOCK_CLOCKS_256,
    EIP207_BLOCK_CLOCKS_512,
    EIP207_BLOCK_CLOCKS_1024,
    EIP207_BLOCK_CLOCKS_2048,
} EIP207_BlockClocks_t;

// EIP-207s FRC/TRC/ARC4RC configuration
typedef struct
{
    // True when the record cache must be enabled
    bool fEnable;

    // True when the block next command logic is disabled
    bool fNonBlock;

    // A blocked command will be released automatically after 3 ticks of
    // a free-running timer whose speed is set by this field.
    // This parameter configures the number of the Classification Engine
    // clocks per one tick, see EIP207_BlockClocks_t values
    uint8_t BlockClockCount;

    // Record base address
    EIP207_Global_Value64_t RecBaseAddr;

    // Number of words in Admin RAM.
    unsigned int AdminWordCount;

    // Number of words in Data RAM.
    unsigned int DataWordCount;

} EIP207_Global_CacheParams_t;

// EIP-207s FRC/TRC/ARC4RC status information
typedef struct
{
    // True when a Host bus master read access from this cache
    // resulted in an error.
    bool fDMAReadError;

    // True when a Host bus master write access from this cache
    // resulted in an error.
    bool fDMAWriteError;

    // True when a non-correctable ECC error is detected in the Record Cache
    // Administration RAM. In case of this error the Record Cache will enter the
    // software reset state.
    bool fAdminEccErr;

    // True when a non-correctable ECC error is detected in the Record Cache
    // Data RAM. In case of this error the Record Cache will remain
    // operational, only the packets for the corrupted record will be flushed.
    bool fDataEccErr;

    // True when a non-correctable ECC error is detected in the Record Cache
    // Data RAM. In case of this error the Record Cache will enter the
    // software reset state.
    bool fDataEccOflo;

} EIP207_Global_CacheStatus_t;

// EIP207 Cache statistics.
typedef struct
{
    // Number of prefetches executed
    uint64_t PrefetchExec;
    // Number of prefetches blocked
    uint64_t PrefetchBlock;
    // Number of pretetches with DMA
    uint64_t PrefetchDMA;
    // Number of select operations
    uint64_t SelectOps;
    // Number of select operations with DMA
    uint64_t SelectDMA;
    // Number of internal DMA writes
    uint64_t IntDMAWrite;
    // Number of external DMA writes
    uint64_t ExtDMAWrite;
    // Number of invalidate operations
    uint64_t InvalidateOps;
    // Read DMA error flags
    uint32_t ReadDMAErrFlags;
    // Number of read DMA errors
    uint32_t ReadDMAErrors;
    // Number of write DMA errors
    uint32_t WriteDMAErrors;
    // Number of invalidates caused by ECC errors
    uint32_t InvalidateECC;
    // Number of correctable ECC errors in Data RAM
    uint32_t DataECCCorr;
    // Number of correctable ECC errors in Admin RAM
    uint32_t AdminECCCorr;
} EIP207_Global_CacheDebugStatistics_t;

typedef struct
{
    EIP207_Global_CacheParams_t FRC [EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE];
    EIP207_Global_CacheParams_t TRC [EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE];
    EIP207_Global_CacheParams_t ARC4[EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE];
} EIP207_Global_CacheConfig_t;

// Flow Hash IV
// Although there are as many flow hash engines in the Classification Support
// module (EIP-207s) as there are Pull-up Micro-engines in the Classification
// Engines (EIP-207c), they all use the same IV value to start their
// calculations from.
typedef struct
{
    uint32_t IV_Word32[4]; // must be written with a true random value
} EIP207_Hash_IV_t;

// Flow hash table configuration parameters,
// there can be multiple (f) flow hash tables
typedef struct
{
    // True when transform record pre-fetch triggering must be done in the Flow
    // Record Cache for flow lookups done via this hash table.
    bool fPrefetchXform;

    // True when ARC4 state record pre-fetch triggering must be done
    // in the Flow Record Cache for flow lookups done via this hash table.
    bool fPrefetchARC4State;

    // True when caching of the flow lookups is enabled for this hash table.
    bool fLookupCached;

} EIP207_Global_FlowHashTable_t;

// Flow Look-Up Engine (FLUE) configuration
typedef struct
{
    // Flow hash calculation IV data
    EIP207_Hash_IV_t IV;

    // When true the lookup in Host memory is only started on a failed flow
    // lookup cache access. When false the read from the hash table in Host
    // memory is started in parallel with the flow lookup cache access.
    bool fDelayMemXS;

    // Number of intermediate entries cached when performing a flow lookup
    uint8_t CacheChain;

    // Number of configured tables in the HashTable parameter,
    // may not exceed EIP207_MAX_NOF_FLUE_HASH_TABLES
    unsigned int HashTablesCount;

    EIP207_Global_FlowHashTable_t
                HashTable[EIP207_MAX_NOF_FLOW_HASH_TABLES_TO_USE];

    // Number of packet interfaces in the InterfaceIndex parameter.
    // may not exceed EIP207_FLUE_MAX_NOF_FLOW_HASH_TABLES_TO_USE
    unsigned int InterfacesCount;

    unsigned int InterfaceIndex[EIP207_FLUE_MAX_NOF_INTERFACES_TO_USE];

} EIP207_Global_FLUEConfig_t;

// This data structure represents EIP-207s Input Classification Engine (ICE)
// global statistics.
typedef struct
{
    EIP207_Global_Value64_t DroppedPacketsCounter;

    uint32_t                OutboundPacketCounter;

    EIP207_Global_Value64_t OutboundOctetsCounter;

    uint32_t                InboundPacketsCounter;

    EIP207_Global_Value64_t InboundOctetsCounter;

} EIP207_Global_ICE_GlobalStats_t;

// This data structure represents EIP-207s Output Classification Engine (OCE)
// global statistics.
typedef struct
{
    // For future extensions:
    //   to be filled in with the actual OCE global statistics
    EIP207_Global_Value64_t DroppedPacketsCounter;

} EIP207_Global_OCE_GlobalStats_t;

// This data structure represents global statistics.
typedef struct
{
    // EIP-207c ICE Global Statistics
    EIP207_Global_ICE_GlobalStats_t ICE;

    // EIP-207c OCE Global Statistics
    // Note: Not used in some EIP-207 HW versions and
    //       for these versions these counters will not be updated.
    EIP207_Global_OCE_GlobalStats_t OCE;

} EIP207_Global_GlobalStats_t;

// This data structure represents the clock count data as generated by
// the Classification Engine timer which speed is configured by
// the TimerPrescaler parameter in the EIP207_Global_Firmware_Load() function.
// The clock count is updated when a packet is processed and represents
// the timestamp of the last packet.
typedef struct
{
    // EIP-207c ICE clock count
    EIP207_Global_Value64_t ICE;

    // EIP-207c OCE clock count
    // Note: Not used in some EIP-207c HW versions and
    //       for these versions the OCE timestamp will not be updated.
    EIP207_Global_Value64_t OCE;

} EIP207_Global_Clock_t;

// EIP-207c Classification Engine (CE) status information
typedef struct
{
    // True when a correctable ECC error has been detected
    // by the Pull-Up micro-engine
    bool fPUE_EccCorr;

    // True when a non-correctable ECC error has been detected
    // by the Pull-Up micro-engine
    bool fPUE_EccDerr;

    // True when a correctable ECC error has been detected
    // by the PostProcessor micro-engine
    bool fFPP_EccCorr;

    // True when a non-correctable ECC error has been detected
    // by the PostProcessor micro-engine
    bool fFPP_EccDerr;

    // The Classification Engine timer overflow
    bool fTimerOverflow;

} EIP207_Global_CE_Status_t;

// Flow Look-Up Engine (FLUE) status information
typedef struct
{
    // Error bit mask 1, bits definition is implementation-specific
    uint32_t Error1;

    // Error bit mask 2, bits definition is implementation-specific
    uint32_t Error2;

} EIP207_Global_FLUE_Status_t;

// Classification Engine status information
typedef struct
{
    // The EIP-207c Input Classification Engine (ICE)
    EIP207_Global_CE_Status_t ICE;

    // The EIP-207c Output Classification Engine (OCE)
    // Note: OCE is not used in some EIP-207 HW versions and
    //        these flags are set to false in this case.
    EIP207_Global_CE_Status_t OCE;

    // Debug statistics counters.
    EIP207_Global_CacheDebugStatistics_t FRCStats[EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE];
    EIP207_Global_CacheDebugStatistics_t TRCStats[EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE];

    // Fatal DMA errors reported by the Record Caches (FRC/TRC/ARC4RC)
    // The engine must be recovered by means of the Global SW Reset or
    // the HW Reset
    EIP207_Global_CacheStatus_t FRC [EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE];
    EIP207_Global_CacheStatus_t TRC [EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE];
    EIP207_Global_CacheStatus_t ARC4RC [EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE];

    // Errors reported by the EIP-207s Flow Look-Up Engine (FLUE).
    EIP207_Global_FLUE_Status_t FLUE;

} EIP207_Global_Status_t;

// Firmware configuration data structure.
typedef struct {
    bool fTokenExtensionsEnable; /* Use extensions to command/result tokens */
    bool fIncrementPktID; /* increment packet ID in tunnel headers */
    uint8_t DTLSRecordHeaderAlign; /* Size of DTLS record headers in decrypted
                                      output */
    uint8_t ECNControl; /* Bit mask controlling ECN packet drops */
    uint16_t PktID; /* Initial value of packet ID */

    bool fDTLSDeferCCS; /* Defer DTLS packets of type CCS to slow path */
    bool fDTLSDeferAlert; /* Defer DTLS packets of type Alert to slow path */
    bool fDTLSDeferHandshake; /* Defer DTLS packets of type Handshake to slow path */
    bool fDTLSDeferAppData; /* Defer DTLS packets of type AppData to slow path */
    bool fDTLSDeferCAPWAP; /* Defer DTLS packets with CAPWAP to slow path */

    bool fRedirRingEnable; /* Enable the redirection ring */
    bool fAlwaysRedirect; /* Always redirect inline to redirection ring */
    uint8_t RedirRing;     /* Ring ID of redirection ring */
    uint16_t TransformRedirEnable; /* Enable per-interface the per-transform redirect. */
} EIP207_Firmware_Config_t;

/*----------------------------------------------------------------------------
 * EIP207_Global_Init
 *
 * This function performs the initialization of the EIP-207 Global Control HW
 * interface and transits the API to the "Caches Enabled" state.
 *
 * This function returns the EIP207_GLOBAL_UNSUPPORTED_FEATURE_ERROR error code
 * when it detects a mismatch in the Global Control Driver Library configuration
 * and the use EIP-207 HW revision or configuration.
 *
 * Note: This function should be called either after the HW Reset or
 *       after the Global SW Reset.
 *       It must be called before any other functions of this API are called
 *       for this Classification Engine instance.
 *
 * IOArea_p (output)
 *     Pointer to the place holder in memory for the IO Area data for
 *     the Classification Engine instance identified by the Device parameter.
 *
 * Device (input)
 *     Handle for the EIP-207 Global Control device instance returned
 *     by Device_Find(). There can be only one device instance for the
 *     EIP-207 Global HW interface.
 *
 * CacheConf_p (input, output)
 *     EIP-207s FRC/TRC/ARC4RC configuration parameters
 *
 * FLUEConf_p (input)
 *     EIP-207s Flow Hash & Look-Up Engine configuration parameters
 *
 * Return value
 *     EIP207_GLOBAL_NO_ERROR
 *     EIP207_GLOBAL_UNSUPPORTED_FEATURE_ERROR
 *     EIP207_GLOBAL_ARGUMENT_ERROR
 *     EIP207_GLOBAL_ILLEGAL_IN_STATE
 */
EIP207_Global_Error_t
EIP207_Global_Init(
        EIP207_Global_IOArea_t * const IOArea_p,
        const Device_Handle_t Device,
        EIP207_Global_CacheConfig_t * const CacheConf_p,
        const EIP207_Global_FLUEConfig_t * const FLUEConf_p);


/*----------------------------------------------------------------------------
 * EIP207_Global_Firmware_Load
 *
 * This function performs the initialization of the EIP-207 Classification
 * Engine internal RAM, performs the firmware download and transits the API to
 * the "Firmware Loaded" state. The function also clears the global statistics.
 *
 * Note: This function should be called after EIP207_Global_Init()
 *
 * IOArea_p (output)
 *     Pointer to the IO Area for the Classification Engine.
 *
 * TimerPrescaler (input)
 *     Prescaler setting to let the 64 bit timer increment at a fixed rate
 *
 * IPUE_Firmware_p (input, output)
 *     Pointer to the firmware data structure for the Input Pull-up
 *     micro-engine.
 *     The adapter has to pass a valid image pointer and size.
 *     The adapter can optionally set the Major, Minor and PatchLevel fields
 *     to 0, in which case this function returns the acutal version fields
 *     of the firmware loaded. If the adapter sets one of them to a nonzero
 *     value, they have to match with the actual values of the loaded firmware.
 *
 * IFPP_Firmware_p (input, output)
 *     Pointer to the firmware data structure for the Input Flow
 *     The adapter has to pass a valid image pointer and size.
 *     The adapter can optionally set the Major, Minor and PatchLevel fields
 *     to 0, in which case this function returns the acutal version fields
 *     of the firmware loaded. If the adapter sets one of them to a nonzero
 *     value, they have to match with the actual values of the loaded firmware.
 *     Post-processor micro-engine.
 *
 * OPUE_Firmware_p (input, output)
 *     Pointer to the firmware data structure for the Output Pull-up
 *     micro-engine.
 *     The adapter has to pass a valid image pointer and size.
 *     The adapter can optionally set the Major, Minor and PatchLevel fields
 *     to 0, in which case this function returns the acutal version fields
 *     of the firmware loaded. If the adapter sets one of them to a nonzero
 *     value, they have to match with the actual values of the loaded firmware.
 *     Note: Not used in some EIP-207 HW versions.
 *
 * OFPP_Firmware_p (input, output)
 *     Pointer to the firmware data structure for the Output Flow
 *     Post-processor micro-engine.
 *     The adapter has to pass a valid image pointer and size.
 *     The adapter can optionally set the Major, Minor and PatchLevel fields
 *     to 0, in which case this function returns the acutal version fields
 *     of the firmware loaded. If the adapter sets one of them to a nonzero
 *     value, they have to match with the actual values of the loaded firmware.
 *     Note: Not used in some EIP-207 HW versions.
 *
 * Return value
 *     EIP207_GLOBAL_NO_ERROR
 *     EIP207_GLOBAL_UNSUPPORTED_FEATURE_ERROR
 *     EIP207_GLOBAL_ARGUMENT_ERROR
 *     EIP207_GLOBAL_ILLEGAL_IN_STATE
 *     EIP207_GLOBAL_INTERNAL_ERROR : firmware download failed
 */
EIP207_Global_Error_t
EIP207_Global_Firmware_Load(
        EIP207_Global_IOArea_t * const IOArea_p,
        const unsigned int TimerPrescaler,
        EIP207_Firmware_t * const IPUE_Firmware_p,
        EIP207_Firmware_t * const IFPP_Firmware_p,
        EIP207_Firmware_t * const OPUE_Firmware_p,
        EIP207_Firmware_t * const OFPP_Firmware_p);


/*----------------------------------------------------------------------------
 * EIP207_Global_HWRevision_Get
 *
 * This function returns EIP-207 hardware revision
 * information in the Capabilities_p data structure.
 *
 * IOArea_p (output)
 *     Pointer to the IO Area for the Classification Engine.
 *
 * Capabilities_p (output)
 *     Pointer to the place holder in memory where the device capability
 *     information will be stored.
 *
 * Return value
 *     EIP207_GLOBAL_NO_ERROR
 *     EIP207_GLOBAL_ARGUMENT_ERROR
 *     EIP207_GLOBAL_ILLEGAL_IN_STATE
 */
EIP207_Global_Error_t
EIP207_Global_HWRevision_Get(
        EIP207_Global_IOArea_t * const IOArea_p,
        EIP207_Global_Capabilities_t * const Capabilities_p);


/*----------------------------------------------------------------------------
 * EIP207_Global_GlobalStats_Get
 *
 * This function obtains global statistics for the Classification Engine.
 *
 * IOArea_p (output)
 *     Pointer to the IO Area for the Classification Engine.
 *
 * CE_Number (input)
 *     Number of the CE for which the status must be obtained.
 *
 * GlobalStats_p (output)
 *     Pointer to the data structure where the global statistics will be stored.
 *
 * Return value
 *     EIP207_GLOBAL_NO_ERROR
 *     EIP207_GLOBAL_ARGUMENT_ERROR
 *     EIP207_GLOBAL_ILLEGAL_IN_STATE
 *     EIP207_GLOBAL_INTERNAL_ERROR : failed to read 64-bit counter value
 */
EIP207_Global_Error_t
EIP207_Global_GlobalStats_Get(
        EIP207_Global_IOArea_t * const IOArea_p,
        const unsigned int CE_Number,
        EIP207_Global_GlobalStats_t * const GlobalStats_p);


/*--------------------- -------------------------------------------------------
 * EIP207_Global_ClockCount_Get
 *
 * Retrieve the current clock count as used by the Classification Engine.
 *
 * IOArea_p (output)
 *     Pointer to the IO Area for the Classification Engine.
 *
 * CE_Number (input)
 *     Number of the CE for which the status must be obtained.
 *
 * Clock_p (output)
 *     Current clock count used by the engine (least significant word).
 *
 * Return value
 *     EIP207_GLOBAL_NO_ERROR
 *     EIP207_GLOBAL_ARGUMENT_ERROR
 *     EIP207_GLOBAL_ILLEGAL_IN_STATE
 *     EIP207_GLOBAL_INTERNAL_ERROR : failed to read 64-bit counter value
 */
EIP207_Global_Error_t
EIP207_Global_ClockCount_Get(
        EIP207_Global_IOArea_t * const IOArea_p,
        const unsigned int CE_Number,
        EIP207_Global_Clock_t * const Clock_p);


/*--------------------- -------------------------------------------------------
 * EIP207_Global_Status_Get
 *
 * Retrieve the Classification Engine status information.
 *
 * IOArea_p (output)
 *     Pointer to the IO Area for the Classification Engine.
 *
 * CE_Number (input)
 *     Number of the CE for which the status must be obtained.
 *
 * Status_p (output)
 *     Pointer to the data structure where the Classification Engine
 *     status information will be stored.
 *
 * fFatalError_p (output)
 *     Pointer to memory where the boolean flag will be stored. The flag
 *     is set to true when this function reports one or several fatal errors.
 *     A fatal error requires the engine Global SW or HW Reset.
 *
 * Return value
 *     EIP207_GLOBAL_NO_ERROR
 *     EIP207_GLOBAL_ARGUMENT_ERROR
 *     EIP207_GLOBAL_ILLEGAL_IN_STATE
 */
EIP207_Global_Error_t
EIP207_Global_Status_Get(
        EIP207_Global_IOArea_t * const IOArea_p,
        const unsigned int CE_Number,
        EIP207_Global_Status_t * const Status_p,
        bool * const fFatalError_p);


#endif /* EIP207_GLOBAL_INIT_H_ */


/* end of file eip207_global_init.h */
