/* eip97_global_init.h
 *
 * EIP-97 Global Control Driver Library API:
 * Initialization, Un-initialization, Configuration use case
 *
 * Refer to the EIP-97 Driver Library User Guide for information about
 * re-entrance and usage from concurrent execution contexts of this API
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

#ifndef EIP97_GLOBAL_INIT_H_
#define EIP97_GLOBAL_INIT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint8_t, uint32_t, bool

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t

// EIP-97 Global Control Driver Library Types API
#include "eip97_global_types.h" // EIP97_* types


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Generic EIP HW version
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
} EIP_Version_t;

// EIP-202 HW options
typedef struct
{
    // Number of statically configured Descriptor Rings
    uint8_t NofRings;

    // Number of statically configured Processing Engines,
    // value 0 indicates 8 PEs
    uint8_t NofPes;

    // If true then 64 bit descriptors will contain a particle
    // size/fill level extension word to allow particle sizes larger
    // than 1 MB.
    bool fExpPlf;

    // Command Descriptor FIFO size, the actual size is 2^CF_Size 32-bit
    // words.
    uint8_t CF_Size;

    // Result Descriptor FIFO size, the actual size is 2^RF_Size 32-bit
    // words.
    uint8_t RF_Size;

    // Host interface type:
    // 0 = PLB, 1 = AHB, 2 = TCM, 3 = AXI
    uint8_t HostIfc;

    // Maximum supported DMA length is 2^(DMA_Len+1) – 1 bytes
    uint8_t DMA_Len;

    // Host interface data width:
    // 0 = 32 bits, 1 = 64 bits, 2 = 128 bits, 3 = 256 bits
    uint8_t HDW;

    // Target access block alignment. If this value is larger than 0,
    // the distance between 2 rings, 2 AICs and between the DFE and the DSE
    // in the slave memory map is increased by a factor of 2^TgtAlign.
    // This means that ring control registers start at 2^(TgtAlign+11) Byte
    // boundaries (or a 2^(TgtAlign+12) Byte boundary for a combined
    // CDR/RDR block), the DFE and DSE will start at a 2^(TgtAlign+10) Byte
    // boundary and AICs will start at 2^(TgtAlign+8) Byte boundaries.
    uint8_t TgtAlign;

    // 64-bit addressing mode:
    // false = 32-bit addressing,
    // true = 64-bit addressing
    bool fAddr64;
} EIP202_Options_t;

// EIP-202 HW options2
typedef struct
{
    // Number of statically configured Look-aside interfaces
    uint8_t NofLA_Ifs;

    // Number of statically configured Inline interfaces
    uint8_t NofIN_Ifs;

    // AXI master interface only
    // Number of write channels for a high-performance AXI master interface
    // minus 1
    uint8_t NofAXI_WrChs;

    // AXI master interface only
    // Number of read clusters for a high-performance AXI master interface.
    uint8_t NofAXI_RdClusters;

    // AXI master interface only
    // Number of read channels per read cluster for a high-performance AXI
    // master interface, minus 1
    uint8_t NofAXI_RdCPC;

} EIP202_Options2_t;

// EIP-206 HW options
typedef struct
{
    // These bits encode the EIP number for the EIP-96 Packet Engine.
    // This field contains the value 96 (decimal) or 0x60.
    uint8_t PE_Type;

    // Input-side classifier configuration:
    //    0 - no input classifier present
    //    1 - EIP-207 input classifier present
    uint8_t InClassifier;

    // Output-side classifier configuration:
    //    0 - no input classifier present
    //    1 - EIP-207 output classifier present
    uint8_t OutClassifier;

    // Number of MAC [9:8] media interface RX/TX channels multiplexed and
    // demultiplexed here, in range 0-8.
    uint8_t NofMAC_Channels;

    // Size of the Input Data Buffer in kilobytes.
    uint8_t InDbufSizeKB;

    // Size of the Input Token Buffer in kilobytes.
    uint8_t InTbufSizeKB;

    // Size of the Output Data Buffer in kilobytes.
    uint8_t OutDbufSizeKB;

    // Size of the Output Token Buffer in kilobytes.
    uint8_t OutTbufSizeKB;

} EIP206_Options_t;

// EIP-96 HW options
typedef struct
{
    // If true AES is available
    bool fAES;

    // If true AES-CFB-128 and AES-OFB-128 are available
    bool fAESfb;

    // If true fast AES core is integrated (12.8 bits/cycle).
    // If false medium speed AES core is integrated (4.2 bits/cycle).
    bool fAESspeed;

    // If true DES and 3-DES are available
    bool fDES;

    // If true (3-)DES-CFB-64 and (3-)DES-OFB-64 are available.
    bool fDESfb;

    // If true fast (4-round) DES core is integrated.
    // If false slow (3-round) DES core is integrated.
    bool fDESspeed;

    // ARC4 availability
    // 0 - no ARC4 is available
    // 1 - a slow speed ARC4 core is integrated (3.5 bits/cycle)
    // 2 - a medium speed ARC4 core is integrated (6.4 bits/cycle)
    // 3 - a high speed ARC4 core is integrated (8.0 bits/cycle)
    uint8_t ARC4;

    // If true AES-XTS is available
    bool fAES_XTS;

    // If true Wireless crypto algorithms (Kasumi, SNOW, ZUC) are available
    bool fWireless;

    // If true MD5 is available
    bool fMD5;

    // If true SHA-1 is available
    bool fSHA1;

    // If true fast SHA-1 core is integrated (12.8 bits/cycle)
    // If false slow SHA-1 core is integrated (6.4 bits/cycle)
    bool fSHA1speed;

    // If true SHA-224/256 is available
    bool fSHA224_256;

    // If true SHA-384/512 is available
    bool fSHA384_512;

    // If true AES-XCBC-MAC is available.
    // This also supports CBC-MAC and CMAC operations
    bool fXCBC_MAC;

    // If true fast AES-CBC-MAC core is integrated (12.8 bits/cycle)
    // If false slow AES-CBC-MAC core is integrated (4.2 bits/cycle)
    bool fCBC_MACspeed;

    // If true AES-CBC-MAC core accepts all key lengths (128/192/256 bits)
    // If false AES-CBC-MAC core accepts only keys with a length of 128 bits
    bool fCBC_MACkeylens;

    // If true GHASH core is available
    bool fGHASH;

} EIP96_Options_t;

// EIP-97 HW options
typedef struct
{
    // Number of statically configured Processing Engines
    uint8_t NofPes;

    // Size of statically configured Input Token Buffer
    // The actual size is 2^in_tbuf_size in 32-bit words
    uint8_t in_tbuf_size;

    // Size of statically configured Input Data Buffer
    // The actual size is 2^in_dbuf_size in 32-bit words
    uint8_t in_dbuf_size;

    // Size of statically configured Output Token Buffer
    // The actual size is 2^in_tbuf_size in 32-bit words
    uint8_t out_tbuf_size;

    // Size of statically configured Output Data Buffer
    // The actual size is 2^in_dbuf_size in 32-bit words
    uint8_t out_dbuf_size;

    // If true then the EIP(197) has a single central EIP-74 type DRBG.
    // If false then each EIP-96 has its own local PRNG.
    bool central_prng;

    // If true then a Token Generator is available in EIP-97 HW
    // If false the Host must supply the Token for the Processing Engine
    bool tg;

    // If true then a Transform Record Cache is available in EIP-97 HW
    bool trc;
} EIP97_Options_t;

// Capabilities structure for EIP-97 HW
typedef struct
{
    // HIA
    EIP202_Options2_t   EIP202_Options2;
    EIP202_Options_t    EIP202_Options;
    EIP_Version_t       EIP202_Version;

    // Processing Engine
    EIP206_Options_t    EIP206_Options;
    EIP_Version_t       EIP206_Version;

    // Packet Engine
    EIP96_Options_t     EIP96_Options;
    EIP_Version_t       EIP96_Version;

    // EIP-97 HW shell
    EIP97_Options_t     EIP97_Options;
    EIP_Version_t       EIP97_Version;

} EIP97_Global_Capabilities_t;

// Ring PE assignment map
typedef struct
{
    // PE is selected via the index in the RingPE_Mask array,
    // index i selects PE i
    // Bit N:
    //     0 - ring N is not assigned (cleared) to this PE
    //     1 - ring N is assigned to this PE
    uint32_t RingPE_Mask;

    // PE is selected via the index in the RingPrio_Mask array,
    // index i selects PE i
    // Bit N:
    //     0 - ring N is low priority
    //     1 - ring N is high priority
    uint32_t RingPrio_Mask;

    // CDR Slots
    // CDR0: bits 3-0,   CDR1: bits 7-4,   CDR2: bits 11-8,  CDR3: bits 15-12
    // CDR4: bits 19-16, CDR5: bits 23-20, CDR6: bits 27-24, CDR7: bits 31-28
    uint32_t RingSlots0;

    // CDR Slots
    // CDR8:  bits 3-0,   CDR9:  bits 7-4,   CDR10: bits 11-8, CDR11: bits 15-12
    // CDR12: bits 19-16, CDR13: bits 23-20, CDR14: bits 27-24
    uint32_t RingSlots1;

} EIP97_Global_Ring_PE_Map_t;


/*----------------------------------------------------------------------------
 * EIP97_Global_Init
 *
 * This function performs the initialization of the EIP-97 Global Control HW
 * interface and transits the API to the Initialized state.
 *
 * This function returns the EIP97_GLOBAL_UNSUPPORTED_FEATURE_ERROR error code
 * when it detects a mismatch in the Global Control Driver Library configuration
 * and the use EIP-97 HW revision or configuration.
 *
 * Note: This function should be called either after the EIP-97 HW Reset or
 *       after the Global SW Reset.
 *
 * IOArea_p (output)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * Device (input)
 *     Handle for the Global Control device instance returned by Device_Find.
 *
 * Return value
 *     EIP97_GLOBAL_NO_ERROR : operation is completed
 *     EIP97_GLOBAL_UNSUPPORTED_FEATURE_ERROR : not supported by the device.
 *     EIP97_GLOBAL_ARGUMENT_ERROR : Passed wrong argument
 *     EIP97_GLOBAL_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP97_Global_Error_t
EIP97_Global_Init(
        EIP97_Global_IOArea_t * const IOArea_p,
        const Device_Handle_t Device);


/*----------------------------------------------------------------------------
 * EIP97_Global_Reset
 *
 * This function starts the Global SW Reset operation. If the reset operation
 * can be done immediately this function returns EIP97_GLOBAL_NO_ERROR.
 * Otherwise it will return EIP97_GLOBAL_BUSY_RETRY_LATER indicating
 * that the reset operation has been started and is ongoing.
 * The EIP97_Global_Reset_IsDone() function can be used to poll the device
 * for the completion of the reset operation.
 *
 * Note: This function must be called before calling the EIP97_Global_Init()
 *       function only if the EIP-97 HW Reset was not done. Otherwise it still
 *       is can be called but it is not necessary.
 *
 * IOArea_p (output)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * Device (input)
 *     Handle for the Global Control device instance returned by Device_Find.
 *
 * Return value
 *     EIP97_GLOBAL_NO_ERROR : Global SW Reset is done
 *     EIP97_GLOBAL_UNSUPPORTED_FEATURE_ERROR : not supported by the device.
 *     EIP97_GLOBAL_BUSY_RETRY_LATER: Global SW Reset is started but
 *                                    not completed yet
 *     EIP97_GLOBAL_ARGUMENT_ERROR : Passed wrong argument
 *     EIP97_GLOBAL_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP97_Global_Error_t
EIP97_Global_Reset(
        EIP97_Global_IOArea_t * const IOArea_p,
        const Device_Handle_t Device);


/*----------------------------------------------------------------------------
 * EIP97_Global_Reset_IsDone
 *
 * This function checks the status of the started by the EIP97_Global_Reset()
 * function Global SW Reset operation for the EIP-97 device.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * Return value
 *     EIP97_GLOBAL_NO_ERROR : Global SW Reset operation is done
 *     EIP97_GLOBAL_BUSY_RETRY_LATER: Global SW Reset is started but
 *                                    not completed yet
 *     EIP97_GLOBAL_ARGUMENT_ERROR : Passed wrong argument
 *     EIP97_GLOBAL_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP97_Global_Error_t
EIP97_Global_Reset_IsDone(
        EIP97_Global_IOArea_t * const IOArea_p);


/*----------------------------------------------------------------------------
 * EIP97_Global_HWRevision_Get
 *
 * This function returns EIP-97, EIP202 HIA and EIP-96 PE hardware revision
 * information in the Capabilities_p data structure.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * Capabilities_p (output)
 *     Pointer to the place holder in memory where the device capability
 *     information will be stored.
 *
 * Return value
 *     EIP97_GLOBAL_NO_ERROR : operation is completed
 *     EIP97_GLOBAL_ARGUMENT_ERROR : Passed wrong argument
 */
EIP97_Global_Error_t
EIP97_Global_HWRevision_Get(
        EIP97_Global_IOArea_t * const IOArea_p,
        EIP97_Global_Capabilities_t * const Capabilities_p);


/*----------------------------------------------------------------------------
 * EIP97_Global_Configure
 *
 * This function performs the Ring to PE assignment and configures
 * the Ring priority. The EIP-97 device supports multiple Ring interfaces
 * as well as multiple PE's. One ring can be assigned to the same or different
 * PE's. Multiple rings can be assigned to the same PE.
 *
 * This function transits the API from the Initialized to the Enabled state
 * when the ring(s) assignment to the PE(s) is performed successfully.
 *
 * This function transits the API from the Enabled to the Initialized state
 * when the ring(s) assignment to the PE(s) is cleared.
 *
 * This function keeps the API in the Enabled state when the ring(s) assignment
 * to the PE(s) is changed but not cleared completely.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * PE_Number (input)
 *     Number of the PE that must be configured.
 *
 * RingPEMap_p (input)
 *     Pointer to the data structure that contains the Ring PE assignment map.
 *
 * Return value
 *     EIP97_GLOBAL_NO_ERROR : operation is completed
 *     EIP97_GLOBAL_ARGUMENT_ERROR : Passed wrong argument
 *     EIP97_GLOBAL_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP97_Global_Error_t
EIP97_Global_Configure(
        EIP97_Global_IOArea_t * const IOArea_p,
        const unsigned int PE_Number,
        const EIP97_Global_Ring_PE_Map_t * const RingPEMap_p);


/*----------------------------------------------------------------------------
 * EIP97_SupportedFuncs_Get
 *
 * Return a bitmask of the supported functions (EIP96 options register).
 */
unsigned int
EIP97_SupportedFuncs_Get(void);


#endif /* EIP97_GLOBAL_INIT_H_ */


/* end of file eip97_global_init.h */
