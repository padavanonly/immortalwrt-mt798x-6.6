/* eip202_global_level0.h
 *
 * EIP-202 HIA Global Control Level0 Internal interface
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

#ifndef EIP202_GLOBAL_LEVEL0_H_
#define EIP202_GLOBAL_LEVEL0_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip97_global.h"

// EIP-292 Global Control HW interface
#include "eip202_global_hw_interface.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // BIT definitions, bool, uint32_t

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t
#include "device_rw.h"          // Read32, Write32
#include "device_swap.h"        // Device_SwapEndian32


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP202_Set_Slave_Byte_Order_Swap_Word32
 *
 * Helper function that can be used to swap words in the body of the
 * EIP202_Set_Slave_Byte_Order() functions as well as in the other
 * functions where words require byte swap before this can be done
 * by the Packet Engine Slave interface
  */
static inline void
EIP202_Set_Slave_Byte_Order_Swap_Word32(uint32_t* const Word32)
{
#ifndef EIP97_GLOBAL_DISABLE_HOST_SWAP_INIT

    uint32_t tmp = Device_SwapEndian32(*Word32);

    *Word32 = tmp;

#else
    IDENTIFIER_NOT_USED(Word32);
#endif // !EIP97_GLOBAL_DISABLE_HOST_SWAP_INIT
}


/*----------------------------------------------------------------------------
 * EIP202 Global Functions
 *
 */

/*----------------------------------------------------------------------------
 * EIP202_Read32
 *
 * This routine writes to a Register location in the EIP-202.
 */
static inline uint32_t
EIP202_Read32(
        Device_Handle_t Device,
        const unsigned int Offset)
{
    return Device_Read32(Device, Offset);
}


/*----------------------------------------------------------------------------
 * EIP202_Write32
 *
 * This routine writes to a Register location in the EIP-202.
 */
static inline void
EIP202_Write32(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t Value)
{
    Device_Write32(Device, Offset, Value);
}


static inline bool
EIP202_REV_SIGNATURE_MATCH(
        const uint32_t Rev)
{
    return (((uint16_t)Rev) == EIP202_SIGNATURE);
}


static inline void
EIP202_EIP_REV_RD(
        Device_Handle_t Device,
        uint8_t * const EipNumber,
        uint8_t * const ComplmtEipNumber,
        uint8_t * const HWPatchLevel,
        uint8_t * const MinHWRevision,
        uint8_t * const MajHWRevision)
{
    uint32_t RevRegVal;

    RevRegVal = EIP202_Read32(Device, EIP202_G_REG_VERSION);

    *MajHWRevision     = (uint8_t)((RevRegVal >> 24) & MASK_4_BITS);
    *MinHWRevision     = (uint8_t)((RevRegVal >> 20) & MASK_4_BITS);
    *HWPatchLevel      = (uint8_t)((RevRegVal >> 16) & MASK_4_BITS);
    *ComplmtEipNumber  = (uint8_t)((RevRegVal >> 8)  & MASK_8_BITS);
    *EipNumber         = (uint8_t)((RevRegVal)       & MASK_8_BITS);
}


static inline void
EIP202_OPTIONS_RD(
        Device_Handle_t Device,
        uint8_t * const NofRings,
        uint8_t * const NofPes,
        bool * const fExpPlf,
        uint8_t * const CF_Size,
        uint8_t * const RF_Size,
        uint8_t * const HostIfc,
        uint8_t * const DMA_Len,
        uint8_t * const HDW,
        uint8_t * const TgtAlign,
        bool * const fAddr64)
{
    uint32_t RevRegVal;

    RevRegVal = EIP202_Read32(Device, EIP202_G_REG_OPTIONS);

    *fAddr64   = ((RevRegVal & BIT_31) != 0);
    *TgtAlign  = (uint8_t)((RevRegVal >> 28) & MASK_3_BITS);
    *HDW       = (uint8_t)((RevRegVal >> 25) & MASK_3_BITS);
    *DMA_Len   = (uint8_t)((RevRegVal >> 20) & MASK_5_BITS);
    *HostIfc   = (uint8_t)((RevRegVal >> 16) & MASK_4_BITS);
    *fExpPlf   = ((RevRegVal & BIT_15) != 0);
    // Make RD FIFO size = 2^RF_Size in HDW words
    *RF_Size   = (uint8_t)((RevRegVal >> 12) & MASK_3_BITS) + 4;
    // Make CD FIFO size = 2^CF_Size in HDW words
    *CF_Size   = (uint8_t)((RevRegVal >> 9)  & MASK_3_BITS) + 4;
    *NofPes    = (uint8_t)((RevRegVal >> 4)  & MASK_5_BITS);
    *NofRings  = (uint8_t)((RevRegVal)       & MASK_4_BITS);
}


static inline void
EIP202_OPTIONS2_RD(
        Device_Handle_t Device,
        uint8_t * const NofLA_Ifs,
        uint8_t * const NofIN_Ifs,
        uint8_t * const NofAXI_WrChs,
        uint8_t * const NofAXI_RdClusters,
        uint8_t * const NofAXI_RdCPC)
{
    uint32_t RevRegVal;

    RevRegVal = EIP202_Read32(Device, EIP202_G_REG_OPTIONS2);

    *NofAXI_RdCPC      = (uint8_t)((RevRegVal >> 28) & MASK_4_BITS);
    *NofAXI_RdClusters = (uint8_t)((RevRegVal >> 20) & MASK_8_BITS);
    *NofAXI_WrChs      = (uint8_t)((RevRegVal >> 16) & MASK_4_BITS);
    *NofIN_Ifs         = (uint8_t)((RevRegVal >> 4)  & MASK_4_BITS);
    *NofLA_Ifs         = (uint8_t)((RevRegVal)       & MASK_4_BITS);
}


static inline void
EIP202_MST_CTRL_BUS_BURST_SIZE_GET(
        Device_Handle_t Device,
        uint8_t * const BusBurstSize)
{
    uint32_t RegVal;

    RegVal = EIP202_Read32(Device, EIP202_G_REG_MST_CTRL);

    *BusBurstSize  = (uint8_t)((RegVal >> 4  ) & MASK_4_BITS);
}


static inline void
EIP202_MST_CTRL_BUS_BURST_SIZE_UPDATE(
        Device_Handle_t Device,
        const uint8_t BusBurstSize,
        const uint8_t RxBusBurstSize)
{
    uint32_t RegVal;

    // Preserve other settings
    RegVal = EIP202_Read32(Device, EIP202_G_REG_MST_CTRL);

    RegVal &= ~MASK_8_BITS;

    RegVal |= (uint32_t)((((uint32_t)BusBurstSize)   & MASK_4_BITS) << 4);
    RegVal |= (uint32_t)((((uint32_t)RxBusBurstSize) & MASK_4_BITS));

    EIP202_Write32(Device, EIP202_G_REG_MST_CTRL, RegVal);
}


static inline void
EIP202_MST_CTRL_BUS_TIMEOUT_UPDATE(
        Device_Handle_t Device,
        const uint8_t Timeout)
{
    uint32_t RegVal;

    // Preserve other settings
    RegVal = EIP202_Read32(Device, EIP202_G_REG_MST_CTRL);

    RegVal &= ~(MASK_6_BITS << 26);

    RegVal |= (uint32_t)((((uint32_t)Timeout) & MASK_6_BITS) << 26);

    EIP202_Write32(Device, EIP202_G_REG_MST_CTRL, RegVal);
}


static inline void
EIP202_MST_CTRL_BYTE_SWAP_UPDATE(
        const Device_Handle_t Device,
        const bool fByteSwap)
{
    uint32_t SlaveCfg;

    SlaveCfg = EIP202_Read32(Device, EIP202_G_REG_MST_CTRL);

    // Swap the bytes for the Host endianness format (must be Big Endian)
    EIP202_Set_Slave_Byte_Order_Swap_Word32(&SlaveCfg);

    // Enable the Slave Endian Byte Swap in HW
    SlaveCfg &= ~(BIT_25 | BIT_24);
    SlaveCfg |= (fByteSwap ? BIT_24 : BIT_25);

    // Swap the bytes back for the Device endianness format (Little Endian)
    EIP202_Set_Slave_Byte_Order_Swap_Word32(&SlaveCfg);

    // Set byte order in the EIP202_G_REG_MST_CTRL register
    EIP202_Write32(Device, EIP202_G_REG_MST_CTRL, SlaveCfg);
}


/*----------------------------------------------------------------------------
 * EIP202 Ring Arbiter Functions
 *
 */

static inline void
EIP202_RA_PRIO_0_VALUE32_WR(
        Device_Handle_t Device,
        const uint32_t Value)
{
    EIP202_Write32(Device, EIP202_RA_REG_PRIO_0, Value);
}


static inline uint32_t
EIP202_RA_PRIO_0_VALUE32_RD(
        Device_Handle_t Device)
{
    return EIP202_Read32(Device, EIP202_RA_REG_PRIO_0);
}


static inline void
EIP202_RA_PRIO_0_WR(
        Device_Handle_t Device,
        const bool fCDR0High,
        const uint8_t CDR0_Slots,
        const bool fCDR1High,
        const uint8_t CDR1_Slots,
        const bool fCDR2High,
        const uint8_t CDR2_Slots,
        const bool fCDR3High,
        const uint8_t CDR3_Slots)
{
    uint32_t RegVal = EIP202_RA_REG_PRIO_0_DEFAULT;

    if(fCDR0High)
        RegVal |= BIT_7;

    if(fCDR1High)
        RegVal |= BIT_15;

    if(fCDR2High)
        RegVal |= BIT_23;

    if(fCDR3High)
        RegVal |= BIT_31;

    RegVal |= (uint32_t)((((uint32_t)CDR3_Slots) & MASK_4_BITS) << 24);
    RegVal |= (uint32_t)((((uint32_t)CDR2_Slots) & MASK_4_BITS) << 16);
    RegVal |= (uint32_t)((((uint32_t)CDR1_Slots) & MASK_4_BITS) << 8);
    RegVal |= (uint32_t)((((uint32_t)CDR0_Slots) & MASK_4_BITS));

    EIP202_Write32(Device, EIP202_RA_REG_PRIO_0, RegVal);
}


static inline void
EIP202_RA_PRIO_1_WR(
        Device_Handle_t Device,
        const bool fCDR4High,
        const uint8_t CDR4_Slots,
        const bool fCDR5High,
        const uint8_t CDR5_Slots,
        const bool fCDR6High,
        const uint8_t CDR6_Slots,
        const bool fCDR7High,
        const uint8_t CDR7_Slots)
{
    uint32_t RegVal = EIP202_RA_REG_PRIO_1_DEFAULT;

    if(fCDR4High)
        RegVal |= BIT_7;

    if(fCDR5High)
        RegVal |= BIT_15;

    if(fCDR6High)
        RegVal |= BIT_23;

    if(fCDR7High)
        RegVal |= BIT_31;

    RegVal |= (uint32_t)((((uint32_t)CDR7_Slots) & MASK_4_BITS) << 24);
    RegVal |= (uint32_t)((((uint32_t)CDR6_Slots) & MASK_4_BITS) << 16);
    RegVal |= (uint32_t)((((uint32_t)CDR5_Slots) & MASK_4_BITS) << 8);
    RegVal |= (uint32_t)((((uint32_t)CDR4_Slots) & MASK_4_BITS));

    EIP202_Write32(Device, EIP202_RA_REG_PRIO_1, RegVal);
}


static inline void
EIP202_RA_PRIO_2_WR(
        Device_Handle_t Device,
        const bool fCDR8High,
        const uint8_t CDR8_Slots,
        const bool fCDR9High,
        const uint8_t CDR9_Slots,
        const bool fCDR10High,
        const uint8_t CDR10_Slots,
        const bool fCDR11High,
        const uint8_t CDR11_Slots)
{
    uint32_t RegVal = EIP202_RA_REG_PRIO_2_DEFAULT;

    if(fCDR8High)
        RegVal |= BIT_7;

    if(fCDR9High)
        RegVal |= BIT_15;

    if(fCDR10High)
        RegVal |= BIT_23;

    if(fCDR11High)
        RegVal |= BIT_31;

    RegVal |= (uint32_t)((((uint32_t)CDR11_Slots) & MASK_4_BITS) << 24);
    RegVal |= (uint32_t)((((uint32_t)CDR10_Slots) & MASK_4_BITS) << 16);
    RegVal |= (uint32_t)((((uint32_t)CDR9_Slots)  & MASK_4_BITS) << 8);
    RegVal |= (uint32_t)((((uint32_t)CDR8_Slots)  & MASK_4_BITS));

    EIP202_Write32(Device, EIP202_RA_REG_PRIO_2, RegVal);
}


static inline void
EIP202_RA_PRIO_3_WR(
        Device_Handle_t Device,
        const bool fCDR12High,
        const uint8_t CDR12_Slots,
        const bool fCDR13High,
        const uint8_t CDR13_Slots,
        const bool fCDR14High,
        const uint8_t CDR14_Slots)
{
    uint32_t RegVal = EIP202_RA_REG_PRIO_3_DEFAULT;

    if(fCDR12High)
        RegVal |= BIT_7;

    if(fCDR13High)
        RegVal |= BIT_15;

    if(fCDR14High)
        RegVal |= BIT_23;

    RegVal |= (uint32_t)((((uint32_t)CDR14_Slots) & MASK_4_BITS) << 16);
    RegVal |= (uint32_t)((((uint32_t)CDR13_Slots) & MASK_4_BITS) << 8);
    RegVal |= (uint32_t)((((uint32_t)CDR12_Slots) & MASK_4_BITS));

    EIP202_Write32(Device, EIP202_RA_REG_PRIO_3, RegVal);
}


static inline void
EIP202_RA_PRIO_0_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_Write32(Device,
                   EIP202_RA_REG_PRIO_0,
                   EIP202_RA_REG_PRIO_0_DEFAULT);
}


static inline void
EIP202_RA_PRIO_1_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_Write32(Device,
                   EIP202_RA_REG_PRIO_1,
                   EIP202_RA_REG_PRIO_1_DEFAULT);
}


static inline void
EIP202_RA_PRIO_2_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_Write32(Device,
                   EIP202_RA_REG_PRIO_2,
                   EIP202_RA_REG_PRIO_2_DEFAULT);
}


static inline void
EIP202_RA_PRIO_3_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_Write32(Device,
                   EIP202_RA_REG_PRIO_3,
                   EIP202_RA_REG_PRIO_3_DEFAULT);
}

static inline void
EIP202_RA_PE_CTRL_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int PEnr)
{
    EIP202_Write32(Device,
                   EIP202_RA_PE_REG_CTRL(PEnr),
                   EIP202_RA_PE_REG_CTRL_DEFAULT);
}


static inline void
EIP202_RA_PE_CTRL_WR(
        Device_Handle_t Device,
        const unsigned int PEnr,
        const uint16_t RingPEmap,
        const bool fEnable,
        const bool fReset)
{
    uint32_t RegVal = EIP202_RA_PE_REG_CTRL_DEFAULT;

    if(RingPEmap > 0)
        RegVal |= (((uint32_t)RingPEmap) & MASK_15_BITS);

    if(fEnable)
        RegVal |= BIT_30;

    if(fReset)
        RegVal |= BIT_31;

    EIP202_Write32(Device, EIP202_RA_PE_REG_CTRL(PEnr), RegVal);
}
/*----------------------------------------------------------------------------
 * EIP202 DFE Thread Functions
 *
 */

static inline void
EIP202_DFE_CFG_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int PEnr)
{
    EIP202_Write32(Device,
                   EIP202_DFE_REG_CFG(PEnr) + Offset,
                   EIP202_DFE_REG_CFG_DEFAULT);
}


static inline void
EIP202_DFE_CFG_WR(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int PEnr,
        const uint8_t MinDataSize,
        const uint8_t DataCacheCtrl,
        const uint8_t MaxDataSize,
        const uint8_t MinCtrlSize,
        const uint8_t CtrlCacheCtrl,
        const uint8_t MaxCtrlSize,
        const bool fAdvThreshMode,
        const bool fAggressive)
{
    uint32_t RegVal = EIP202_DFE_REG_CFG_DEFAULT;

    if(fAggressive)
        RegVal |= BIT_31;

    if(fAdvThreshMode)
        RegVal |= BIT_29;

    RegVal |= (uint32_t)((((uint32_t)MaxCtrlSize)   & MASK_4_BITS) << 24);
    RegVal |= (uint32_t)((((uint32_t)CtrlCacheCtrl) & MASK_3_BITS) << 20);
    RegVal |= (uint32_t)((((uint32_t)MinCtrlSize)   & MASK_4_BITS) << 16);
    RegVal |= (uint32_t)((((uint32_t)MaxDataSize)   & MASK_4_BITS) << 8);
    RegVal |= (uint32_t)((((uint32_t)DataCacheCtrl) & MASK_3_BITS) << 4);
    RegVal |= (uint32_t)((((uint32_t)MinDataSize)   & MASK_4_BITS));

    EIP202_Write32(Device, EIP202_DFE_REG_CFG(PEnr) + Offset, RegVal);
}


static inline void
EIP202_DFE_TRD_CTRL_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int PEnr)
{
    EIP202_Write32(Device,
                   EIP202_DFE_TRD_REG_CTRL(PEnr) + Offset,
                   EIP202_DFE_TRD_REG_CTRL_DEFAULT);
}


static inline void
EIP202_DFE_TRD_CTRL_WR(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int PEnr,
        const uint16_t RingPEmap,
        const bool fEnable,
        const bool fReset)
{
    uint32_t RegVal = EIP202_DFE_TRD_REG_CTRL_DEFAULT;

    IDENTIFIER_NOT_USED(RingPEmap);
    IDENTIFIER_NOT_USED(fEnable);
    if(fReset)
        RegVal |= BIT_31;

    EIP202_Write32(Device, EIP202_DFE_TRD_REG_CTRL(PEnr) + Offset, RegVal);
}


static inline void
EIP202_DFE_TRD_CTRL_UPDATE(
        const Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int PEnr,
        const uint16_t RingPEmap,
        const bool fEnable,
        const bool fReset)
{
    uint32_t RegVal;

    // Preserve other settings
    RegVal = EIP202_Read32(Device, EIP202_DFE_TRD_REG_CTRL(PEnr) + Offset);

    RegVal &= (~MASK_15_BITS);
    RegVal |= (((uint32_t)RingPEmap) & MASK_15_BITS);

    if(fEnable)
        RegVal |= BIT_30;
    else
        RegVal &= (~BIT_30);

    if(fReset)
        RegVal |= BIT_31;
    else
        RegVal &= (~BIT_31);

    EIP202_Write32(Device, EIP202_DFE_TRD_REG_CTRL(PEnr) + Offset, RegVal);
}


static inline void
EIP202_DFE_TRD_STAT_RD(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int PEnr,
        uint16_t * const CDFIFOWordCount,
        uint8_t * const CDRId,
        uint16_t * const DMAByteCount,
        bool * const fTokenDMABusy,
        bool * const fDataDMABusy,
        bool * const fDMAError)
{
    uint32_t RegVal;

    RegVal = EIP202_Read32(Device, EIP202_DFE_TRD_REG_STAT(PEnr) + Offset);

    *fDMAError         = ((RegVal & BIT_31) != 0);
    *fDataDMABusy      = ((RegVal & BIT_29) != 0);
    *fTokenDMABusy     = ((RegVal & BIT_28) != 0);
    *DMAByteCount      = (uint16_t)((RegVal >> 16) & MASK_12_BITS);
    *CDRId             =  (uint8_t)((RegVal >> 12) & MASK_4_BITS);
    *CDFIFOWordCount   = (uint16_t)((RegVal)       & MASK_12_BITS);
}


static inline void
EIP202_DFE_TRD_STAT_RINGID_RD(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int PEnr,
        uint8_t * const CDRId)
{
    uint32_t RegVal;

    RegVal = EIP202_Read32(Device, EIP202_DFE_TRD_REG_STAT(PEnr) + Offset);

    *CDRId  = (uint8_t)((RegVal >> 12) & MASK_4_BITS);
}


/*----------------------------------------------------------------------------
 * EIP202 DSE Thread Functions
 *
 */

static inline void
EIP202_DSE_CFG_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int PEnr)
{
    EIP202_Write32(Device,
                   EIP202_DSE_REG_CFG(PEnr) + Offset,
                   EIP202_DSE_REG_CFG_DEFAULT);
}


static inline void
EIP202_DSE_CFG_WR(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int PEnr,
        const uint8_t MinDataSize,
        const uint8_t DataCacheCtrl,
        const uint8_t MaxDataSize,
        const uint8_t BufferCtrl,
        const bool fEnableSingleWr,
        const bool fAggressive)
{
    uint32_t RegVal = EIP202_DSE_REG_CFG_DEFAULT;

    RegVal &= (~BIT_31);
    if(fAggressive)
        RegVal |= BIT_31;

    if(fEnableSingleWr)
        RegVal |= BIT_29;

    RegVal &= (uint32_t)(~(MASK_2_BITS << 14)); // Clear BufferCtrl field
    RegVal |= (uint32_t)((((uint32_t)BufferCtrl)    & MASK_2_BITS) << 14);
    RegVal |= (uint32_t)((((uint32_t)MaxDataSize)   & MASK_4_BITS) << 8);
    RegVal |= (uint32_t)((((uint32_t)DataCacheCtrl) & MASK_3_BITS) << 4);
    RegVal |= (uint32_t)((((uint32_t)MinDataSize)   & MASK_4_BITS));

    EIP202_Write32(Device, EIP202_DSE_REG_CFG(PEnr) + Offset, RegVal);
}


static inline void
EIP202_DSE_TRD_CTRL_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int PEnr)
{
    EIP202_Write32(Device,
                   EIP202_DSE_TRD_REG_CTRL(PEnr) + Offset,
                   EIP202_DSE_TRD_REG_CTRL_DEFAULT);
}


static inline void
EIP202_DSE_TRD_CTRL_WR(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int PEnr,
        const uint16_t RingPEmap,
        const bool fEnable,
        const bool fReset)
{
    uint32_t RegVal = EIP202_DSE_TRD_REG_CTRL_DEFAULT;
    IDENTIFIER_NOT_USED(RingPEmap);

    if(fEnable)
        RegVal |= BIT_30;

    if(fReset)
        RegVal |= BIT_31;

    EIP202_Write32(Device, EIP202_DSE_TRD_REG_CTRL(PEnr) + Offset, RegVal);
}


static inline void
EIP202_DSE_TRD_CTRL_UPDATE(
        const Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int PEnr,
        const uint16_t RingPEmap,
        const bool fEnable,
        const bool fReset)
{
    uint32_t RegVal;

    // Preserve other settings
    RegVal = EIP202_Read32(Device, EIP202_DSE_TRD_REG_CTRL(PEnr) + Offset);

    RegVal &= (~MASK_15_BITS);
    RegVal |= (((uint32_t)RingPEmap) & MASK_15_BITS);

    if(fEnable)
        RegVal |= BIT_30;
    else
        RegVal &= (~BIT_30);

    if(fReset)
        RegVal |= BIT_31;
    else
        RegVal &= (~BIT_31);

    EIP202_Write32(Device, EIP202_DSE_TRD_REG_CTRL(PEnr) + Offset, RegVal);
}


static inline void
EIP202_DSE_TRD_STAT_RD(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int PEnr,
        uint16_t * const RDFIFOWordCount,
        uint8_t * const RDRId,
        uint16_t * const DMAByteCount,
        bool * const fDataFlushBusy,
        bool * const fDataDMABusy,
        bool * const fDMAError)
{
    uint32_t RegVal;

    RegVal = EIP202_Read32(Device, EIP202_DSE_TRD_REG_STAT(PEnr) + Offset);

    *fDMAError         = ((RegVal & BIT_31) != 0);
    *fDataDMABusy      = ((RegVal & BIT_29) != 0);
    *fDataFlushBusy    = ((RegVal & BIT_28) != 0);
    *DMAByteCount      = (uint16_t)((RegVal >> 16) & MASK_12_BITS);
    *RDRId             =  (uint8_t)((RegVal >> 12) & MASK_4_BITS);
    *RDFIFOWordCount   = (uint16_t)((RegVal)       & MASK_12_BITS);
}


static inline void
EIP202_DSE_TRD_STAT_RINGID_RD(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int PEnr,
        uint8_t * const CDRId)
{
    uint32_t RegVal;

    RegVal = EIP202_Read32(Device, EIP202_DSE_TRD_REG_STAT(PEnr) + Offset);

    *CDRId = (uint8_t)((RegVal >> 12) & MASK_4_BITS);
}


static inline void
EIP202_LASIDE_BASE_ADDR_LO_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_Write32(Device,
                   EIP202_REG_LASIDE_BASE_ADDR_LO,
                   EIP202_REG_LASIDE_BASE_ADDR_LO_DEFAULT);
}


static inline void
EIP202_LASIDE_BASE_ADDR_LO_WR(
        Device_Handle_t Device,
        const uint8_t DscrDataSwap)
{
    uint32_t RegVal = EIP202_REG_LASIDE_BASE_ADDR_LO_DEFAULT;

    RegVal |= (uint32_t)((((uint32_t)DscrDataSwap)   & MASK_4_BITS));

    EIP202_Write32(Device, EIP202_REG_LASIDE_BASE_ADDR_LO, RegVal);
}


static inline void
EIP202_LASIDE_BASE_ADDR_HI_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_Write32(Device,
                   EIP202_REG_LASIDE_BASE_ADDR_HI,
                   EIP202_REG_LASIDE_BASE_ADDR_HI_DEFAULT);
}


static inline void
EIP202_LASIDE_SLAVE_CTRL_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int FIFOnr)
{
    EIP202_Write32(Device,
                   EIP202_REG_LASIDE_SLAVE_CTRL(FIFOnr),
                   EIP202_REG_LASIDE_SLAVE_CTRL_DEFAULT);
}


static inline void
EIP202_LASIDE_SLAVE_CTRL_WR(
        Device_Handle_t Device,
        const unsigned int FIFOnr,
        const uint8_t InPktDataSwap,
        const uint8_t PktProt,
        const uint8_t TokenDataSwap,
        const uint8_t TokenProt,
        const bool fDscrErrorClear)
{
    uint32_t RegVal = EIP202_REG_LASIDE_SLAVE_CTRL_DEFAULT;

    if(fDscrErrorClear)
        RegVal |= BIT_31;

    RegVal |= (uint32_t)((((uint32_t)TokenProt)     & MASK_4_BITS) << 12);
    RegVal |= (uint32_t)((((uint32_t)TokenDataSwap) & MASK_4_BITS) << 8);
    RegVal |= (uint32_t)((((uint32_t)PktProt)       & MASK_4_BITS) << 4);
    RegVal |= (uint32_t)((((uint32_t)InPktDataSwap) & MASK_4_BITS));

    EIP202_Write32(Device, EIP202_REG_LASIDE_SLAVE_CTRL(FIFOnr), RegVal);
}


static inline void
EIP202_LASIDE_MASTER_CTRL_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int FIFOnr)
{
    EIP202_Write32(Device,
                   EIP202_REG_LASIDE_MASTER_CTRL(FIFOnr),
                   EIP202_REG_LASIDE_MASTER_CTRL_DEFAULT);
}


static inline void
EIP202_LASIDE_MASTER_CTRL_WR(
        Device_Handle_t Device,
        const unsigned int FIFOnr,
        const uint8_t OutPktDataSwap,
        const uint8_t PktProt,
        const bool fDscrErrorClear)
{
    uint32_t RegVal = EIP202_REG_LASIDE_MASTER_CTRL_DEFAULT;

    if(fDscrErrorClear)
        RegVal |= BIT_31;

    RegVal |= (uint32_t)((((uint32_t)PktProt)        & MASK_4_BITS) << 4);
    RegVal |= (uint32_t)((((uint32_t)OutPktDataSwap) & MASK_4_BITS));

    EIP202_Write32(Device, EIP202_REG_LASIDE_MASTER_CTRL(FIFOnr), RegVal);
}


static inline void
EIP202_INLINE_CTRL_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int FIFOnr)
{
    EIP202_Write32(Device,
                   EIP202_REG_INLINE_CTRL(FIFOnr),
                   EIP202_REG_INLINE_CTRL_DEFAULT);
}


static inline void
EIP202_INLINE_CTRL_WR(
        Device_Handle_t Device,
        const unsigned int FIFOnr,
        const uint8_t InPktDataSwap,
        const bool fInProtoErrorClear,
        const uint8_t OutPktDataSwap,
        const uint8_t ThreshLo,
        const uint8_t ThreshHi,
        const uint8_t BurstSize,
        const bool fForceInOrder)
{
    uint32_t RegVal = EIP202_REG_INLINE_CTRL_DEFAULT;

    if(fInProtoErrorClear)
        RegVal |= BIT_7;

    if(fForceInOrder)
        RegVal |= BIT_31;

    RegVal |= (uint32_t)((((uint32_t)BurstSize)      & MASK_4_BITS) << 20);
    RegVal |= (uint32_t)((((uint32_t)ThreshHi)       & MASK_4_BITS) << 16);
    RegVal |= (uint32_t)((((uint32_t)ThreshLo)       & MASK_4_BITS) << 12);
    RegVal |= (uint32_t)((((uint32_t)OutPktDataSwap) & MASK_4_BITS) << 8);
    RegVal |= (uint32_t)((((uint32_t)InPktDataSwap)  & MASK_4_BITS));

    EIP202_Write32(Device, EIP202_REG_INLINE_CTRL(FIFOnr), RegVal);
}

#endif /* EIP202_GLOBAL_LEVEL0_H_ */


/* end of file eip202_global_level0.h */
