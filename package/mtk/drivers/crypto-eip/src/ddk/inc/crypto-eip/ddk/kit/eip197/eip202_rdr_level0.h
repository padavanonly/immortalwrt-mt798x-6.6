/* eip202_rdr_level0.h
 *
 * EIP-202 Internal interface: RDR Manager
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

#ifndef EIP202_RDR_LEVEL0_H_
#define EIP202_RDR_LEVEL0_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip202_ring.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint8_t, uint32_t

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t
#include "device_rw.h"          // Read32, Write32


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Read/Write register constants

/*****************************************************************************
 * Byte offsets of the EIP-202 HIA RDR registers
 *****************************************************************************/

#define EIP202_RDR_OFFS           4

// 2 KB MMIO space for one RDR instance
#define EIP202_RDR_BASE                   0x00000800
#define EIP202_RDR_RING_BASE_ADDR_LO      ((EIP202_RDR_BASE) + \
                                            (0x00 * EIP202_RDR_OFFS))
#define EIP202_RDR_RING_BASE_ADDR_HI      ((EIP202_RDR_BASE) + \
                                            (0x01 * EIP202_RDR_OFFS))
#define EIP202_RDR_DATA_BASE_ADDR_LO      ((EIP202_RDR_BASE) + \
                                            (0x02 * EIP202_RDR_OFFS))
#define EIP202_RDR_DATA_BASE_ADDR_HI      ((EIP202_RDR_BASE) + \
                                            (0x03 * EIP202_RDR_OFFS))
#define EIP202_RDR_RING_SIZE              ((EIP202_RDR_BASE) + \
                                            (0x06 * EIP202_RDR_OFFS))
#define EIP202_RDR_DESC_SIZE              ((EIP202_RDR_BASE) + \
                                            (0x07 * EIP202_RDR_OFFS))
#define EIP202_RDR_CFG                    ((EIP202_RDR_BASE) + \
                                            (0x08 * EIP202_RDR_OFFS))
#define EIP202_RDR_DMA_CFG                ((EIP202_RDR_BASE) + \
                                            (0x09 * EIP202_RDR_OFFS))
#define EIP202_RDR_THRESH                 ((EIP202_RDR_BASE) + \
                                            (0x0A * EIP202_RDR_OFFS))
#define EIP202_RDR_PREP_COUNT             ((EIP202_RDR_BASE) + \
                                            (0x0B * EIP202_RDR_OFFS))
#define EIP202_RDR_PROC_COUNT             ((EIP202_RDR_BASE) + \
                                            (0x0C * EIP202_RDR_OFFS))
#define EIP202_RDR_PREP_PNTR              ((EIP202_RDR_BASE) + \
                                            (0x0D * EIP202_RDR_OFFS))
#define EIP202_RDR_PROC_PNTR              ((EIP202_RDR_BASE) + \
                                            (0x0E * EIP202_RDR_OFFS))
#define EIP202_RDR_STAT                   ((EIP202_RDR_BASE) + \
                                            (0x0F * EIP202_RDR_OFFS))

// Default EIP202_RDR_x register values
#define EIP202_RDR_RING_BASE_ADDR_LO_DEFAULT       0x00000000
#define EIP202_RDR_RING_BASE_ADDR_HI_DEFAULT       0x00000000
#define EIP202_RDR_DATA_BASE_ADDR_LO_DEFAULT       0x00000000
#define EIP202_RDR_DATA_BASE_ADDR_HI_DEFAULT       0x00000000
#define EIP202_RDR_RING_SIZE_DEFAULT               0x00000000
#define EIP202_RDR_DESC_SIZE_DEFAULT               0x00000000
#define EIP202_RDR_CFG_DEFAULT                     0x00000000
#define EIP202_RDR_DMA_CFG_DEFAULT                 0x01800000
#define EIP202_RDR_THRESH_DEFAULT                  0x00000000
#define EIP202_RDR_PREP_COUNT_DEFAULT              0x00000000
#define EIP202_RDR_PROC_COUNT_DEFAULT              0x00000000
#define EIP202_RDR_PREP_PNTR_DEFAULT               0x00000000
#define EIP202_RDR_PROC_PNTR_DEFAULT               0x00000000
// Ignore the RD FIFO size after reset for this register default value for now
#define EIP202_RDR_STAT_DEFAULT                    0x00000000


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP202_RDR_Read32
 *
 * This routine writes to a Register location in the EIP-202 RDR.
 */
static inline uint32_t
EIP202_RDR_Read32(
        Device_Handle_t Device,
        const unsigned int Offset)
{
    return Device_Read32(Device, Offset);
}


/*----------------------------------------------------------------------------
 * EIP202_RDR_Write32
 *
 * This routine writes to a Register location in the EIP-202 RDR.
 */
static inline void
EIP202_RDR_Write32(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t Value)
{
    Device_Write32(Device, Offset, Value);
}


static inline void
EIP202_RDR_RING_BASE_ADDR_LO_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_RDR_Write32(Device,
                       EIP202_RDR_RING_BASE_ADDR_LO,
                       EIP202_RDR_RING_BASE_ADDR_LO_DEFAULT);
}


static inline void
EIP202_RDR_RING_BASE_ADDR_LO_WR(
        Device_Handle_t Device,
        const uint32_t LowAddr)
{
    EIP202_RDR_Write32(Device, EIP202_RDR_RING_BASE_ADDR_LO, LowAddr);
}


static inline void
EIP202_RDR_RING_BASE_ADDR_HI_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_RDR_Write32(Device,
                       EIP202_RDR_RING_BASE_ADDR_HI,
                       EIP202_RDR_RING_BASE_ADDR_HI_DEFAULT);
}


static inline void
EIP202_RDR_RING_BASE_ADDR_HI_WR(
        Device_Handle_t Device,
        const uint32_t HiAddr)
{
    EIP202_RDR_Write32(Device, EIP202_RDR_RING_BASE_ADDR_HI, HiAddr);
}


static inline void
EIP202_RDR_RING_SIZE_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_RDR_Write32(Device,
                       EIP202_RDR_RING_SIZE,
                       EIP202_RDR_RING_SIZE_DEFAULT);
}


static inline void
EIP202_RDR_RING_SIZE_WR(
        Device_Handle_t Device,
        const uint32_t RDRWordCount)
{
    uint32_t RegVal = EIP202_RDR_RING_SIZE_DEFAULT;

    RegVal |= ((((uint32_t)RDRWordCount) & MASK_22_BITS) << 2);

    EIP202_RDR_Write32(Device, EIP202_RDR_RING_SIZE, RegVal);
}


static inline void
EIP202_RDR_DESC_SIZE_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_RDR_Write32(Device,
                       EIP202_RDR_DESC_SIZE,
                       EIP202_RDR_DESC_SIZE_DEFAULT);
}


static inline void
EIP202_RDR_DESC_SIZE_WR(
        Device_Handle_t Device,
        const uint8_t DscrWordCount,
        const uint8_t DscrOffsWordCount,
        const bool f64bit)
{
    uint32_t RegVal = EIP202_RDR_DESC_SIZE_DEFAULT;

    if(f64bit)
        RegVal |= BIT_31;

    RegVal |= ((((uint32_t)DscrOffsWordCount) & MASK_8_BITS) << 16);
    RegVal |= ((((uint32_t)DscrWordCount)     & MASK_8_BITS)      );

    EIP202_RDR_Write32(Device, EIP202_RDR_DESC_SIZE, RegVal);
}


static inline void
EIP202_RDR_CFG_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_RDR_Write32(Device,
                       EIP202_RDR_CFG,
                       EIP202_RDR_CFG_DEFAULT);
}


static inline void
EIP202_RDR_CFG_WR(
        Device_Handle_t Device,
        const uint16_t RDFetchWordCount,
        const uint16_t RDFetchThreshWordCount,
        const bool fOwnEnable)
{
    uint32_t RegVal = EIP202_RDR_CFG_DEFAULT;

    if(fOwnEnable)
        RegVal |= BIT_31;

    RegVal |= ((((uint32_t)RDFetchThreshWordCount) & MASK_12_BITS)  << 16);
    RegVal |= ((((uint32_t)RDFetchWordCount)       & MASK_16_BITS)      );

    EIP202_RDR_Write32(Device, EIP202_RDR_CFG, RegVal);
}


static inline void
EIP202_RDR_DMA_CFG_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_RDR_Write32(Device,
                       EIP202_RDR_DMA_CFG,
                       EIP202_RDR_DMA_CFG_DEFAULT);
}


static inline void
EIP202_RDR_DMA_CFG_WR(
        Device_Handle_t Device,
        const uint8_t RDSwap,
        const uint8_t DataSwap,
        const bool fResBuf,
        const bool fCtrlBuf,
        const bool fOwnBuf,
        const uint8_t WrCache,
        const uint8_t RdCache,
        const uint8_t RdProtection,
        const uint8_t DataProtection,
        const bool fPadToOffset)
{
    uint32_t RegVal = EIP202_RDR_DMA_CFG_DEFAULT;

    if(fPadToOffset)
        RegVal |= BIT_28;

    RegVal &= (~BIT_24);
    if(fOwnBuf)
        RegVal |= BIT_24;

    RegVal &= (~BIT_23);
    if(fCtrlBuf)
        RegVal |= BIT_23;

    if(fResBuf)
        RegVal |= BIT_22;

    RegVal |= ((((uint32_t)RdCache)        & MASK_3_BITS) << 29);
    RegVal |= ((((uint32_t)WrCache)        & MASK_3_BITS) << 25);
    RegVal |= ((((uint32_t)DataProtection) & MASK_4_BITS) << 12);
    RegVal |= ((((uint32_t)DataSwap)       & MASK_4_BITS) << 8);
    RegVal |= ((((uint32_t)RdProtection)   & MASK_4_BITS) << 4);
    RegVal |= ((((uint32_t)RDSwap)         & MASK_4_BITS));

    EIP202_RDR_Write32(Device, EIP202_RDR_DMA_CFG, RegVal);
}


static inline void
EIP202_RDR_THRESH_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_RDR_Write32(Device,
                       EIP202_RDR_THRESH,
                       EIP202_RDR_THRESH_DEFAULT);
}


static inline void
EIP202_RDR_THRESH_WR(
        Device_Handle_t Device,
        const uint32_t RDThreshWordCount,
        const bool fProcPktMode,
        const uint8_t RDTimeout)
{
    uint32_t RegVal = EIP202_RDR_THRESH_DEFAULT;

    if(fProcPktMode)
        RegVal |= BIT_23;

    RegVal |= (RDThreshWordCount      & MASK_22_BITS);
    RegVal |= ((((uint32_t)RDTimeout) & MASK_8_BITS) << 24);

    EIP202_RDR_Write32(Device, EIP202_RDR_THRESH, RegVal);
}


static inline void
EIP202_RDR_PREP_COUNT_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_RDR_Write32(Device,
                       EIP202_RDR_PREP_COUNT,
                       EIP202_RDR_PREP_COUNT_DEFAULT);
}


static inline void
EIP202_RDR_PREP_COUNT_WR(
        Device_Handle_t Device,
        const uint16_t RDCount,
        const bool fClearCount)
{
    uint32_t RegVal = EIP202_RDR_PREP_COUNT_DEFAULT;

    if(fClearCount)
        RegVal |= BIT_31;

    RegVal |= ((((uint32_t)RDCount) & MASK_14_BITS) << 2);

    EIP202_RDR_Write32(Device, EIP202_RDR_PREP_COUNT, RegVal);
}


static inline void
EIP202_RDR_PREP_COUNT_RD(
        Device_Handle_t Device,
        uint32_t * const PrepRDWordCount_p)
{
    uint32_t RegVal;

    RegVal = EIP202_RDR_Read32(Device, EIP202_RDR_PREP_COUNT);

    *PrepRDWordCount_p   = (uint32_t)((RegVal >> 2) & MASK_22_BITS);
}


static inline void
EIP202_RDR_PROC_COUNT_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_RDR_Write32(Device,
                       EIP202_RDR_PROC_COUNT,
                       EIP202_RDR_PROC_COUNT_DEFAULT);
}


static inline void
EIP202_RDR_PROC_COUNT_WR(
        Device_Handle_t Device,
        const uint16_t RDWordCount,
        const uint8_t PktCount,
        const bool fClearCount)
{
    uint32_t RegVal = EIP202_RDR_PROC_COUNT_DEFAULT;

    if(fClearCount)
        RegVal |= BIT_31;

    RegVal |= ((((uint32_t)RDWordCount)  & MASK_14_BITS) << 2);
    RegVal |= ((((uint32_t)PktCount)     & MASK_7_BITS)  << 24);

    EIP202_RDR_Write32(Device, EIP202_RDR_PROC_COUNT, RegVal);
}


static inline void
EIP202_RDR_PROC_COUNT_RD(
        Device_Handle_t Device,
        uint32_t * const ProcRDWordCount_p,
        uint8_t * const ProcPktWordCount_p)
{
    uint32_t RegVal;

    RegVal = EIP202_RDR_Read32(Device, EIP202_RDR_PROC_COUNT);

    *ProcRDWordCount_p    = (uint32_t)((RegVal >> 2)  & MASK_22_BITS);
    *ProcPktWordCount_p   =  (uint8_t)((RegVal >> 24) & MASK_7_BITS);
}


static inline void
EIP202_RDR_PREP_PNTR_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_RDR_Write32(Device,
                       EIP202_RDR_PREP_PNTR,
                       EIP202_RDR_PREP_PNTR_DEFAULT);
}


static inline void
EIP202_RDR_PROC_PNTR_DEFAULT_WR(
        Device_Handle_t Device)
{
    EIP202_RDR_Write32(Device,
                       EIP202_RDR_PROC_PNTR,
                       EIP202_RDR_PROC_PNTR_DEFAULT);
}


static inline void
EIP202_RDR_STAT_RD(
        Device_Handle_t Device,
        bool * const fDMAErrorIrq,
        bool * const fTreshIrq,
        bool * const fErrorIrq,
        bool * const fOuflowIrq,
        bool * const fTimeoutIrq,
        bool * const fBufOfloIrq,
        bool * const fProcOfloIrq,
        uint16_t * const RDFIFOWordCount)
{
    uint32_t RegVal;

    RegVal = EIP202_RDR_Read32(Device, EIP202_RDR_STAT);

    *fDMAErrorIrq      = ((RegVal & BIT_0) != 0);
    *fErrorIrq         = ((RegVal & BIT_2) != 0);
    *fOuflowIrq        = ((RegVal & BIT_3) != 0);
    *fTreshIrq         = ((RegVal & BIT_4) != 0);
    *fTimeoutIrq       = ((RegVal & BIT_5) != 0);
    *fBufOfloIrq       = ((RegVal & BIT_6) != 0);
    *fProcOfloIrq      = ((RegVal & BIT_7) != 0);
    *RDFIFOWordCount   = (uint16_t)((RegVal >> 16) & MASK_12_BITS);
}


static inline void
EIP202_RDR_STAT_FIFO_SIZE_RD(
        Device_Handle_t Device,
        uint16_t * const RDFIFOWordCount)
{
    uint32_t RegVal;

    RegVal = EIP202_RDR_Read32(Device, EIP202_RDR_STAT);

    *RDFIFOWordCount   = (uint16_t)((RegVal >> 16) & MASK_12_BITS);
}


static inline void
EIP202_RDR_STAT_CLEAR_ALL_IRQ_WR(
        Device_Handle_t Device)
{
    EIP202_RDR_Write32(Device,
                       EIP202_RDR_STAT,
                       (uint32_t)(EIP202_RDR_STAT_DEFAULT | MASK_8_BITS));
}


static inline void
EIP202_RDR_STAT_CLEAR_DSCR_BUF_OFLO_IRQ_WR(
        Device_Handle_t Device)
{
    EIP202_RDR_Write32(Device,
                       EIP202_RDR_STAT,
                       (uint32_t)(EIP202_RDR_STAT_DEFAULT | BIT_7 | BIT_6));
}


#endif /* EIP202_RDR_LEVEL0_H_ */


/* end of file eip202_rdr_level0.h */
