/* eip202_global_init.h
 *
 * EIP-202 Global Init interface
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

#ifndef EIP202_GLOBAL_INIT_H_
#define EIP202_GLOBAL_INIT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"             // BIT definitions, bool, uint32_t

// Driver Framework Device API
#include "device_types.h"           // Device_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

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

} EIP202_Global_Ring_PE_Map_t;

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

} EIP202_Capabilities_t;


/*----------------------------------------------------------------------------
 * Local variables
 */



/*----------------------------------------------------------------------------
 * EIP202 Global Functions
 *
 */

/*----------------------------------------------------------------------------
 * EIP202_Global_Detect
 *
 * Checks the presence of EIP-202 HIA hardware. Returns true when found.
 *
 * Device (input)
 *     Device handle of the hardware.
 *
 * Return value
 *     true  : Success
 *     false : Failure
 */
bool
EIP202_Global_Detect(
        const Device_Handle_t Device);


/*----------------------------------------------------------------------------
 * EIP202_Global_Endianness_Slave_Configure
 *
 * Configure Endianness Conversion method
 * for the EIP-202 slave (MMIO) interface
 *
 * Device (input)
 *     Device handle of the hardware.
 *
 * Return value
 *     true  : Success
 *     false : Failure
 */
bool
EIP202_Global_Endianness_Slave_Configure(
        const Device_Handle_t Device);


/*----------------------------------------------------------------------------
 * EIP202_Global_Init
 *
 * Initialize the ring, LA-FIFO and inline interfaces of the EIP202 hardware.
 *
 * Device (input)
 *     Device handle of the hardware.
 * NofPE (input)
 *     Number of packet engines in the device.
 * NofLA (input)
 *     Number of look-aside FIFOs of the device.
 * ipbuf_min (input)
 *     Minimum input packet burst size.
 * ipbuf_max (input)
 *     Maximum input packet burst size.
 * itbuf_min (input)
 *     Minimum input token burst size.
 * itbuf_max (input)
 *     Maximum input token burst size.
 * opbuf_min (input)
 *     Minimum output packet burst size.
 * opbuf_max (input)
 *     Maximum output packet burst size.
 */
void
EIP202_Global_Init(
        const Device_Handle_t Device,
        unsigned int NofPE,
        unsigned int NofLA,
        uint8_t ipbuf_min,
        uint8_t ipbuf_max,
        uint8_t itbuf_min,
        uint8_t itbuf_max,
        uint8_t opbuf_min,
        uint8_t opbuf_max);


/*----------------------------------------------------------------------------
 * EIP202_Global_Reset
 *
 * Reset the EIP202 hardware.
 *
 * Device (input)
 *     Device handle of the hardware.
 * NofPE (input)
 *     Number of packet engines in the hardware.
 *
 * Return value
 *     true  : Success
 *     false : Failure
 */
bool
EIP202_Global_Reset(
        const Device_Handle_t Device,
        const unsigned int NofPE);


/*----------------------------------------------------------------------------
 * EIP202_Global_Reset_IsDone
 *
 * Check if a reset operation (started with EIP202_Global_Reset) is completed
 * for a specified Packet Engine.
 *
 * Device (input)
 *     Device handle of the hardware.
 * PEnr (input)
 *     Number of the packet engine to check.
 *
 * Return value
 *     true  : Reset is completed
 *     false : Reset is not completed
 */
bool
EIP202_Global_Reset_IsDone(
        const Device_Handle_t Device,
        const unsigned int PEnr);


/*----------------------------------------------------------------------------
 * EIP202_Global_HWRevision_Get
 *
 * Read hardware revision and capabilities of the EIP202 hardware.
 *
 * Device (input)
 *     Device handle of the hardware.
 * Capabilities_p (output)
 *     Hardware options of the EIP202.
 */
void
EIP202_Global_HWRevision_Get(
        const Device_Handle_t Device,
        EIP202_Capabilities_t * const Capabilities_p);


/*----------------------------------------------------------------------------
 * EIP202_Global_Configure
 *
 * Configure the EIP202 for a single Packet Engine.
 *
 * Device (input)
 *     Device handle of the hardware.
 * PE_Number (input)
 *     Number of the Packet Engine to configure.
 * RingPEMap_p (input)
 *     Structure containing the configuration.
 */
void
EIP202_Global_Configure(
        const Device_Handle_t Device,
        const unsigned int PE_Number,
        const EIP202_Global_Ring_PE_Map_t * const RingPEMap_p);


#endif /* EIP202_GLOBAL_INIT_H_ */


/* end of file eip202_global_init.h */
