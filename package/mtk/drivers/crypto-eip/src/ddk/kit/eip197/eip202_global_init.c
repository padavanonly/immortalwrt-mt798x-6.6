/* eip202_global_init.c
 *
 * EIP-202 Global Control Driver Library
 * Initialization Module
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

#include "eip202_global_init.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip97_global.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"             // uint32_t

// Driver Framework C Run-time Library API
#include "clib.h"                   // ZEROINIT

// Driver Framework Device API
#include "device_types.h"           // Device_Handle_t

#include "eip202_global_level0.h"   // EIP-202 Level 0 macros

// EIP97_Interfaces_Get
#include "eip97_global_internal.h"

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP202_Global_Detect
 */
bool
EIP202_Global_Detect(
        const Device_Handle_t Device)
{
    uint32_t Value;

    Value = EIP202_Read32(Device, EIP202_G_REG_VERSION);
    if (!EIP202_REV_SIGNATURE_MATCH( Value ))
        return false;

    return true;
}




/*----------------------------------------------------------------------------
 * EIP202_Global_HWRevision_Get
 */
void
EIP202_Global_HWRevision_Get(
        const Device_Handle_t Device,
        EIP202_Capabilities_t * const Capabilities_p)
{
    EIP202_EIP_REV_RD(Device,
                      &Capabilities_p->EipNumber,
                      &Capabilities_p->ComplmtEipNumber,
                      &Capabilities_p->HWPatchLevel,
                      &Capabilities_p->MinHWRevision,
                      &Capabilities_p->MajHWRevision);

    EIP202_OPTIONS_RD(Device,
                      &Capabilities_p->NofRings,
                      &Capabilities_p->NofPes,
                      &Capabilities_p->fExpPlf,
                      &Capabilities_p->CF_Size,
                      &Capabilities_p->RF_Size,
                      &Capabilities_p->HostIfc,
                      &Capabilities_p->DMA_Len,
                      &Capabilities_p->HDW,
                      &Capabilities_p->TgtAlign,
                      &Capabilities_p->fAddr64);

    EIP202_OPTIONS2_RD(Device,
                       &Capabilities_p->NofLA_Ifs,
                       &Capabilities_p->NofIN_Ifs,
                       &Capabilities_p->NofAXI_WrChs,
                       &Capabilities_p->NofAXI_RdClusters,
                       &Capabilities_p->NofAXI_RdCPC);
}


/*----------------------------------------------------------------------------
 * EIP202_Global_Endianness_Slave_Configure
 *
 * Configure Endianness Conversion method
 * for the EIP-202 slave (MMIO) interface
 *
 */
bool
EIP202_Global_Endianness_Slave_Configure(
        const Device_Handle_t Device)
{
#ifdef EIP97_GLOBAL_ENABLE_SWAP_REG_DATA
    uint32_t Value;

    // Read and check the revision register
    Value = EIP202_Read32(Device, EIP202_G_REG_VERSION);
    if (!EIP202_REV_SIGNATURE_MATCH( Value ))
    {
        // No match, try to enable the Slave interface byte swap
        // Must be done via EIP-202 HIA GLobal
        EIP202_MST_CTRL_BYTE_SWAP_UPDATE(Device, true);

        // Read and check the revision register again
        Value = EIP202_Read32(Device, EIP202_G_REG_VERSION);
        if (!EIP202_REV_SIGNATURE_MATCH( Value ))
            // Bail out if still not OK
            return false;
    }

    return true;
#else
    IDENTIFIER_NOT_USED(Device);
    return true;
#endif // EIP97_GLOBAL_ENABLE_SWAP_REG_DATA
}


/*----------------------------------------------------------------------------
 * EIP202_Global_Init
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
        uint8_t opbuf_max)
{
    unsigned int i;
    uint8_t BufferCtrl;
    unsigned int NofPEs,NofRings,NofLAs,NofIN,DFEDSEOffset;
    EIP97_Interfaces_Get(&NofPEs,&NofRings,&NofLAs,&NofIN);
    DFEDSEOffset = EIP97_DFEDSE_Offset_Get();

    // Configure EIP-202 HIA Global
    EIP202_MST_CTRL_BUS_BURST_SIZE_UPDATE(Device,
                                          EIP97_GLOBAL_BUS_BURST_SIZE,
                                          EIP97_GLOBAL_RX_BUS_BURST_SIZE);
    EIP202_MST_CTRL_BUS_TIMEOUT_UPDATE(Device,
                                       EIP97_GLOBAL_TIMEOUT_VALUE);
    if (NofLA)
        // User-configured value
        BufferCtrl = EIP97_GLOBAL_DSE_BUFFER_CTRL;
    else
        // Default register reset value
        BufferCtrl = (uint8_t)EIP202_DSE_BUFFER_CTRL;

    for (i = 0; i < NofPE; i++)
    {
        // Configure EIP-202 HIA DFE Global
        EIP202_DFE_CFG_WR(Device,
                          DFEDSEOffset,
                          i,
                          ipbuf_min,
                          EIP97_GLOBAL_DFE_DATA_CACHE_CTRL,
                          ipbuf_max,
                          itbuf_min,
                          EIP97_GLOBAL_DFE_CTRL_CACHE_CTRL,
                          itbuf_max,
                          (EIP97_GLOBAL_DFE_ADV_THRESH_MODE_FLAG == 1),
                          (EIP97_GLOBAL_DFE_AGGRESSIVE_DMA_FLAG == 1));

        // Configure EIP-202 HIA DSE Global
        EIP202_DSE_CFG_WR(Device,
                          DFEDSEOffset,
                          i,
                          opbuf_min,
                          EIP97_GLOBAL_DSE_DATA_CACHE_CTRL,
                          opbuf_max,
                          BufferCtrl,
                          (EIP97_GLOBAL_DSE_ENABLE_SINGLE_WR_FLAG == 1),
                          (EIP97_GLOBAL_DSE_AGGRESSIVE_DMA_FLAG == 1));

    }

    // Configure HIA Look-aside FIFO
    EIP202_LASIDE_BASE_ADDR_LO_WR(Device,
                                  EIP202_LASIDE_DSCR_BYTE_SWAP_METHOD);

    for (i = EIP97_GLOBAL_LAFIFO_RING_ID;
         i < NofLAs +
             EIP97_GLOBAL_LAFIFO_RING_ID;
         i++)
    {
        EIP202_LASIDE_SLAVE_CTRL_WR(Device,
                                    i,
                                    EIP202_LASIDE_IN_PKT_BYTE_SWAP_METHOD,
                                    EIP202_LASIDE_IN_PKT_PROTO,
                                    EIP202_LASIDE_TOKEN_BYTE_SWAP_METHOD,
                                    EIP202_LASIDE_TOKEN_PROTO,
                                    true); // Clear cmd descriptor error

        EIP202_LASIDE_MASTER_CTRL_WR(Device,
                                     i,
                                     EIP202_LASIDE_OUT_PKT_BYTE_SWAP_METHOD,
                                     EIP202_LASIDE_OUT_PKT_PROTO,
                                     true); // Clear res descriptor error
    }
    // Configure HIA Inline FIFO
    for (i = 0; i < NofIN; i++)
        EIP202_INLINE_CTRL_WR(Device,
                              i,
                              EIP202_INLINE_IN_PKT_BYTE_SWAP_METHOD,
                              false, // Clear protocol error
                              EIP202_INLINE_OUT_PKT_BYTE_SWAP_METHOD,
                              opbuf_min,
                              opbuf_max,
                              EIP202_INLINE_BURST_SIZE,
                              EIP202_INLINE_FORCE_INORDER);

}


/*----------------------------------------------------------------------------
 * EIP202_Global_Reset
 */
bool
EIP202_Global_Reset(
        const Device_Handle_t Device,
        const unsigned int NofPE)
{
    unsigned int i;
    unsigned int DFEDSEOffset;
    DFEDSEOffset = EIP97_DFEDSE_Offset_Get();

    // Restore the EIP-202 default configuration
    // Resets DFE thread and clears ring assignment
    for (i = 0; i < NofPE; i++)
        EIP202_DFE_TRD_CTRL_WR(Device, DFEDSEOffset, i, 0, false, true);

    // HIA DFE defaults
    for (i = 0; i < NofPE; i++)
        EIP202_DFE_CFG_DEFAULT_WR(Device, DFEDSEOffset, i);

#ifndef EIP202_RA_DISABLE
    EIP202_RA_PRIO_0_DEFAULT_WR(Device);
    EIP202_RA_PRIO_1_DEFAULT_WR(Device);
    EIP202_RA_PRIO_2_DEFAULT_WR(Device);
    EIP202_RA_PRIO_3_DEFAULT_WR(Device);

    // Resets ring assignment
    for (i = 0; i < NofPE; i++)
        EIP202_RA_PE_CTRL_WR(Device, i, 0, false, true);
#endif // #ifndef EIP202_RA_DISABLE

    // Resets DSE thread and clears ring assignment
    for (i = 0; i < NofPE; i++)
        EIP202_DSE_TRD_CTRL_WR(Device, DFEDSEOffset, i, 0, false, true);

    // HIA DSE defaults
    for (i = 0; i < NofPE; i++)
        EIP202_DSE_CFG_DEFAULT_WR(Device, DFEDSEOffset, i);


    // HIA LASIDE defaults
    EIP202_LASIDE_BASE_ADDR_LO_DEFAULT_WR(Device);
    EIP202_LASIDE_BASE_ADDR_HI_DEFAULT_WR(Device);
    for (i = EIP97_GLOBAL_LAFIFO_RING_ID;
         i < EIP97_GLOBAL_MAX_NOF_LAFIFO_TO_USE + EIP97_GLOBAL_LAFIFO_RING_ID;
         i++)
    {
        EIP202_LASIDE_MASTER_CTRL_DEFAULT_WR(Device, i);
        EIP202_LASIDE_SLAVE_CTRL_DEFAULT_WR(Device, i);
    }

    // HIA INLINE defaults
    for (i = 0; i < NofPE; i++)
        EIP202_INLINE_CTRL_DEFAULT_WR(Device, i);
    return true;
}


/*----------------------------------------------------------------------------
 * EIP202_Global_Reset_IsDone
 */
bool
EIP202_Global_Reset_IsDone(
        const Device_Handle_t Device,
        const unsigned int PEnr)
{
    uint8_t RingId;
    unsigned int DFEDSEOffset;
    DFEDSEOffset = EIP97_DFEDSE_Offset_Get();

    // Check for completion of all DMA transfers
    EIP202_DFE_TRD_STAT_RINGID_RD(Device, DFEDSEOffset, PEnr, &RingId);
    if(RingId == EIP202_DFE_TRD_REG_STAT_IDLE)
    {
        EIP202_DSE_TRD_STAT_RINGID_RD(Device, DFEDSEOffset, PEnr, &RingId);
        if(RingId == EIP202_DFE_TRD_REG_STAT_IDLE)
        {
            // Take DFE thread out of reset
            EIP202_DFE_TRD_CTRL_DEFAULT_WR(Device, DFEDSEOffset, PEnr);

            // Take DSE thread out of reset
            EIP202_DSE_TRD_CTRL_DEFAULT_WR(Device, DFEDSEOffset, PEnr);

            // Do not restore the EIP-202 Master Control default configuration
            // so this will not change the endianness conversion configuration
            // for the Slave interface
        }
        return true;
    }
    else
    {
        return false;
    }
}


/*----------------------------------------------------------------------------
 * EIP202_Global_Configure
 */
void
EIP202_Global_Configure(
        const Device_Handle_t Device,
        const unsigned int PE_Number,
        const EIP202_Global_Ring_PE_Map_t * const RingPEMap_p)
{
    unsigned int DFEDSEOffset;
    DFEDSEOffset = EIP97_DFEDSE_Offset_Get();
    // Disable EIP-202 HIA DFE thread(s)
    EIP202_DFE_TRD_CTRL_WR(Device,
                           DFEDSEOffset,
                           PE_Number,   // Thread Nr
                           0,
                           false,       // Disable thread
                           false);      // Do not reset thread

    // Disable EIP-202 HIA DSE thread(s)
    EIP202_DSE_TRD_CTRL_WR(Device,
                           DFEDSEOffset,
                           PE_Number,   // Thread Nr
                           0,
                           false,       // Disable thread
                           false);      // Do not reset thread

#ifndef EIP202_RA_DISABLE
    // Configure the HIA Ring Arbiter
    EIP202_RA_PRIO_0_WR(
            Device,
            (RingPEMap_p->RingPrio_Mask & BIT_0) == 0 ? false : true,
            (uint8_t)(RingPEMap_p->RingSlots0 & MASK_4_BITS),
            (RingPEMap_p->RingPrio_Mask & BIT_1) == 0 ? false : true,
            (uint8_t)((RingPEMap_p->RingSlots0 >> 4) & MASK_4_BITS),
            (RingPEMap_p->RingPrio_Mask & BIT_2) == 0 ? false : true,
            (uint8_t)((RingPEMap_p->RingSlots0 >> 8) & MASK_4_BITS),
            (RingPEMap_p->RingPrio_Mask & BIT_3) == 0 ? false : true,
            (uint8_t)((RingPEMap_p->RingSlots0 >> 12) & MASK_4_BITS));

    EIP202_RA_PRIO_1_WR(
            Device,
            (RingPEMap_p->RingPrio_Mask & BIT_4) == 0 ? false : true,
            (uint8_t)((RingPEMap_p->RingSlots0 >> 16) & MASK_4_BITS),
            (RingPEMap_p->RingPrio_Mask & BIT_5) == 0 ? false : true,
            (uint8_t)((RingPEMap_p->RingSlots0 >> 20) & MASK_4_BITS),
            (RingPEMap_p->RingPrio_Mask & BIT_6) == 0 ? false : true,
            (uint8_t)((RingPEMap_p->RingSlots0 >> 24) & MASK_4_BITS),
            (RingPEMap_p->RingPrio_Mask & BIT_7) == 0 ? false : true,
            (uint8_t)((RingPEMap_p->RingSlots0 >> 28) & MASK_4_BITS));

    EIP202_RA_PRIO_2_WR(
            Device,
            (RingPEMap_p->RingPrio_Mask & BIT_8) == 0 ? false : true,
            (uint8_t)(RingPEMap_p->RingSlots1 & MASK_4_BITS),
            (RingPEMap_p->RingPrio_Mask & BIT_9) == 0 ? false : true,
            (uint8_t)((RingPEMap_p->RingSlots1 >> 4) & MASK_4_BITS),
            (RingPEMap_p->RingPrio_Mask & BIT_10) == 0 ? false : true,
            (uint8_t)((RingPEMap_p->RingSlots1 >> 8) & MASK_4_BITS),
            (RingPEMap_p->RingPrio_Mask & BIT_11) == 0 ? false : true,
            (uint8_t)((RingPEMap_p->RingSlots1 >> 12) & MASK_4_BITS));

    EIP202_RA_PRIO_3_WR(
            Device,
            (RingPEMap_p->RingPrio_Mask & BIT_12) == 0 ? false : true,
            (uint8_t)((RingPEMap_p->RingSlots1 >> 16) & MASK_4_BITS),
            (RingPEMap_p->RingPrio_Mask & BIT_13) == 0 ? false : true,
            (uint8_t)((RingPEMap_p->RingSlots1 >> 20) & MASK_4_BITS),
            (RingPEMap_p->RingPrio_Mask & BIT_14) == 0 ? false : true,
            (uint8_t)((RingPEMap_p->RingSlots1 >> 24) & MASK_4_BITS));

    // Ring assignment in the Ring Arbiter
    EIP202_RA_PE_CTRL_WR(Device,
                         PE_Number,
                         RingPEMap_p->RingPE_Mask,
                         true,
                         false);
#endif // #ifndef EIP202_RA_DISABLE

    {
        // Assign Rings to this DFE thread
        // Enable EIP-202 HIA DFE thread(s)
        EIP202_DFE_TRD_CTRL_WR(Device,
                               DFEDSEOffset,
                               PE_Number,   // Thread Nr
                               RingPEMap_p->RingPE_Mask,  // Rings to assign
                               true,        // Enable thread
                               false);      // Do not reset thread

        // Assign Rings to this DSE thread
        // Enable EIP-202 HIA DSE thread(s)
        EIP202_DSE_TRD_CTRL_WR(Device,
                               DFEDSEOffset,
                               PE_Number,   // Thread Nr
                               RingPEMap_p->RingPE_Mask,   // Rings to assign
                               true,        // Enable thread
                               false);      // Do not reset thread
    }
}

/* end of file eip202_global_init.c */
