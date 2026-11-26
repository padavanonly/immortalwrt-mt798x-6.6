/* cs_adapter_ext.h
 *
 * Configuration Settings for the SLAD Adapter Combined module,
 * extensions.
 *
 */

/*****************************************************************************
* Copyright (c) 2012-2022 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef CS_ADAPTER_EXT_H_
#define CS_ADAPTER_EXT_H_


// Top-level product configuration
#include "cs_ddk197.h"


/****************************************************************************
 * Adapter Global configuration parameters
 */


/****************************************************************************
 * Adapter PEC configuration parameters
 */


/****************************************************************************
 * Adapter Classification (Global Control) configuration parameters
 */


/****************************************************************************
 * Adapter PCL (Flow Control) configuration parameters
 */

#ifdef DRIVER_DMARESOURCE_BANKS_ENABLE

// Element byte count in the DMA bank
#ifdef DRIVER_DMA_BANK_ELEMENT_BYTE_COUNT
#define ADAPTER_TRANSFORM_RECORD_BYTE_COUNT DRIVER_DMA_BANK_ELEMENT_BYTE_COUNT
#define ADAPTER_PCL_FLOW_RECORD_BYTE_COUNT  DRIVER_DMA_BANK_ELEMENT_BYTE_COUNT
#endif

// Element count in the DMA bank
#ifdef DRIVER_DMA_BANK_ELEMENT_COUNT
#define ADAPTER_TRANSFORM_RECORD_COUNT    DRIVER_DMA_BANK_ELEMENT_COUNT
#define ADAPTER_PCL_FLOW_RECORD_COUNT     DRIVER_DMA_BANK_ELEMENT_COUNT
#endif

#endif // DRIVER_DMARESOURCE_BANKS_ENABLE


/****************************************************************************
 * Adapter EIP-202 configuration parameters
 */
#ifdef DRIVER_AIC_SEPARATE_IRQ
#define ADAPTER_EIP202_AIC_SEPARATE_IRQ
#endif
#ifdef DRIVER_RING_AIC_DISABLE
#define ADAPTER_EIP202_RING_AIC_DISABLE
#endif

// Enables the EIP-207 Record Cache interface for DMA banks
//#define ADAPTER_EIP202_RC_DMA_BANK_SUPPORT

// Enables the EIP-207 Record Cache interface for record cache invalidation
//#define ADAPTER_EIP202_RC_DIRECT_INVALIDATE_SUPPORT

// Enable workaround for spurious TTL underflow error reported in
// EIP-202 result descriptor
#define ADAPTER_EIP202_RING_TTL_ERROR_WA


// Enable exactly one of ADAPTER_EIP202_RING_LOCAL_CONFIGURE,
// ADAPTER_EIP202_MANUAL_CONFIGURE or ADAPTER_EIP202_AUTO_THRESH_DISABLE

// Provide CDR and RDR configuration parameters via local CDR options register.
#define ADAPTER_EIP202_RING_LOCAL_CONFIGURE

// Provide manually CDR and RDR configuration parameters
// for the threshold and fetch sizes calculation
//#define ADAPTER_EIP202_RING_MANUAL_CONFIGURE

// Configure EIP-202 HIA CDR and RDR threshold and fetch sizes directly
// instead of calculating them
//#define ADAPTER_EIP202_AUTO_THRESH_DISABLE

#ifdef ADAPTER_EIP202_RING_MANUAL_CONFIGURE
// Host interface data width:
//   0 = 32 bits, 1 = 64 bits, 2 = 128 bits, 3 = 256 bits
#define ADAPTER_EIP202_HOST_DATA_WIDTH      2

// Command Descriptor FIFO size, the actual size is 2^(CF_Size+4) HDW words
// Note: "+4" is already included in this value
#define ADAPTER_EIP202_CF_SIZE              6

// Result Descriptor FIFO size, the actual size is 2^(RF_Size+4) HDW words
// Note: "+4" is already included in this value
#define ADAPTER_EIP202_RF_SIZE              6
#endif // ADAPTER_EIP202_RING_MANUAL_CONFIGURE

#ifdef ADAPTER_EIP202_AUTO_THRESH_DISABLE
#define ADAPTER_EIP202_CDR_DSCR_FETCH_WORD_COUNT    16
#define ADAPTER_EIP202_CDR_DSCR_THRESH_WORD_COUNT   12
#define ADAPTER_EIP202_RDR_DSCR_FETCH_WORD_COUNT    32 // 48 80
#define ADAPTER_EIP202_RDR_DSCR_THRESH_WORD_COUNT   8 // 12 20
#endif

#ifdef ADAPTER_EIP202_RING_AIC_DISABLE

// System does not have a Ring AIC, all ring IRQs connected to global AIC.

// Definition of AIC devices to use in system.
//                  DeviceName IntNR flag
// if flag is 0, IntNr refers to one of the interrupts in ADAPTER_EIP202_IRQS
//   for example IRQ_RING0 and the AIC is connected to an input of another AIC.
// If flag is 1, IntNr is the index of a system interrupt and the AIC
//   has a dedicated system interrupt.
#define ADAPTER_EIP202_AICS \
    ADAPTER_EIP202_ADD_AIC("EIP201_GLOBAL", 0, 1)

// Definitions of logical interrupts in the system.
//                 Name PhysBit AICDevName TaskletFlag, Polarity .
// Name is a symbolic constant like IRQ_CDR0, from which an enum will be made.
// PhysBit is the physical input line of the AIC for this interrupt.
// AICDevName is the device name of the AIC.
// TaskletFLag is 1 if the IRQ is to be handled via a tasklet.
// Polarity is the polarity of the interrupt signal.
#define ADAPTER_EIP202_IRQS \
    ADAPTER_EIP202_ADD_IRQ(IRQ_DFE0, 0, "EIP201_GLOBAL", 0, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_DSE0, 1, "EIP201_GLOBAL", 0, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_PE0, 4, "EIP201_GLOBAL", 0, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR0, 24, "EIP201_GLOBAL", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR0, 25, "EIP201_GLOBAL", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR1, 26, "EIP201_GLOBAL", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR1, 27, "EIP201_GLOBAL", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR2, 28, "EIP201_GLOBAL", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR2, 29, "EIP201_GLOBAL", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR3, 30, "EIP201_GLOBAL", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR3, 31, "EIP201_GLOBAL", 1, ACTIVE_LOW)

#else

#if defined(ADAPTER_EIP202_AIC_SEPARATE_IRQ)

// System has Ring AICs with separate IRQ lines.

// Definition of AIC devices to use in system.
//                  DeviceName IntNR flag
// if flag is 0, IntNr refers to one of the interrupts in ADAPTER_EIP202_IRQS
//   for example IRQ_RING0 and the AIC is connected to an input of another AIC.
// If flag is 1, IntNr is the index of a system interrupt and the AIC
//   has a dedicated system interrupt.
#define ADAPTER_EIP202_AICS \
    ADAPTER_EIP202_ADD_AIC("EIP201_GLOBAL", 0, 1), \
    ADAPTER_EIP202_ADD_AIC("EIP201_RING0", 1, 1), \
    ADAPTER_EIP202_ADD_AIC("EIP201_RING1", 2, 1), \
    ADAPTER_EIP202_ADD_AIC("EIP201_RING2", 3, 1), \
    ADAPTER_EIP202_ADD_AIC("EIP201_RING3", 4, 1),   \
    ADAPTER_EIP202_ADD_AIC("EIP201_CS", IRQ_CS, 0)


// Definitions of logical interrupts in the system.
//                 Name PhysBit AICDevName TaskletFlag.
// Name is a symbolic constant like IRQ_CDR0, from which an enum will be made.
// PhysBit is the physical input line of the AIC for this interrupt.
// AICDevName is the device name of the AIC.
// TaskletFLag is 1 if the IRQ is to be handled via a tasklet.
// Polarity is the polarity of the interrupt signal.
#define ADAPTER_EIP202_IRQS \
    ADAPTER_EIP202_ADD_IRQ(IRQ_MST_ERR, 0, "EIP201_GLOBAL", 0, ACTIVE_LOW),  \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CS,  1, "EIP201_GLOBAL", 0, ACTIVE_LOW),  \
    ADAPTER_EIP202_ADD_IRQ(IRQ_PE0, 4, "EIP201_GLOBAL", 0, ACTIVE_LOW),   \
    ADAPTER_EIP202_ADD_IRQ(IRQ_DRBG_ERR, 8, "EIP201_CS", 1, ACTIVE_HIGH), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_DRBG_RES, 9, "EIP201_CS", 1, RISING_EDGE), \
    /* DRBG reseed must be edge sensitive as reseed operation does not */ \
    /* clear IRQ signal from device immediately */                       \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR0, 0, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR0, 1, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR1, 2, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR1, 3, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR2, 4, "EIP201_RING2", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR2, 5, "EIP201_RING2", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR3, 6, "EIP201_RING3", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR3, 7, "EIP201_RING3", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR4, 8, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR4, 9, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR5, 10, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR5, 11, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR6, 12, "EIP201_RING2", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR6, 13, "EIP201_RING2", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR7, 14, "EIP201_RING3", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR7, 15, "EIP201_RING3", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR8, 16, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR8, 17, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR9, 18, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR9, 19, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR10, 20, "EIP201_RING2", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR10, 21, "EIP201_RING2", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR11, 22, "EIP201_RING3", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR11, 23, "EIP201_RING3", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR12, 24, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR12, 25, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR13, 26, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR13, 27, "EIP201_RING1", 1, ACTIVE_LOW)

#else

// System has Ring AICs, chained to global AIC.

// Definition of AIC devices to use in system.
//                  DeviceName IntNR flag
// if flag is 0, IntNr refers to one of the interrupts in ADAPTER_EIP202_IRQS
//   for example IRQ_RING0 and the AIC is connected to an input of another AIC.
// If flag is 1, IntNr is the index of a system interrupt and the AIC
//   has a dedicated system interrupt.
#define ADAPTER_EIP202_AICS \
    ADAPTER_EIP202_ADD_AIC("EIP201_GLOBAL", 0, 1), \
    ADAPTER_EIP202_ADD_AIC("EIP201_RING0", IRQ_RING0, 0), \
    ADAPTER_EIP202_ADD_AIC("EIP201_RING1", IRQ_RING1, 0), \
    ADAPTER_EIP202_ADD_AIC("EIP201_RING2", IRQ_RING2, 0), \
    ADAPTER_EIP202_ADD_AIC("EIP201_RING3", IRQ_RING3, 0),   \
    ADAPTER_EIP202_ADD_AIC("EIP201_CS", IRQ_CS, 0)

// Definitions of logical interrupts in the system.
//                 Name PhysBit AICDevName TaskletFlag Polarity.
// Name is a symbolic constant like IRQ_CDR0, from which an enum will be made.
// PhysBit is the physical input line of the AIC for this interrupt.
// AICDevName is the device name of the AIC.
// TaskletFLag is 1 if the IRQ is to be handled via a tasklet.
// Polarity is the polarity of the interrupt signal.
#define ADAPTER_EIP202_IRQS \
    ADAPTER_EIP202_ADD_IRQ(IRQ_MST_ERR, 0, "EIP201_GLOBAL", 0, ACTIVE_LOW),  \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CS,  1, "EIP201_GLOBAL", 0, ACTIVE_LOW),  \
    ADAPTER_EIP202_ADD_IRQ(IRQ_PE0, 4, "EIP201_GLOBAL", 0, ACTIVE_LOW),   \
    ADAPTER_EIP202_ADD_IRQ(IRQ_DRBG_ERR, 8, "EIP201_CS", 1, ACTIVE_HIGH), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_DRBG_RES, 9, "EIP201_CS", 1, RISING_EDGE), \
    /* DRBG reseed must be edge sensitive as reseed operation does not */ \
    /* clear IRQ signal from device immediately */                       \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR0, 0, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR0, 1, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR1, 2, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR1, 3, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR2, 4, "EIP201_RING2", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR2, 5, "EIP201_RING2", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR3, 6, "EIP201_RING3", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR3, 7, "EIP201_RING3", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR4, 8, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR4, 9, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR5, 10, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR5, 11, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR6, 12, "EIP201_RING2", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR6, 13, "EIP201_RING2", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR7, 14, "EIP201_RING3", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR7, 15, "EIP201_RING3", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR8, 16, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR8, 17, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR9, 18, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR9, 19, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR10, 20, "EIP201_RING2", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR10, 21, "EIP201_RING2", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR11, 22, "EIP201_RING3", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR11, 23, "EIP201_RING3", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR12, 24, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR12, 25, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR13, 26, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR13, 27, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RING0, 28, "EIP201_GLOBAL", 0, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RING1, 29, "EIP201_GLOBAL", 0, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RING2, 30, "EIP201_GLOBAL", 0, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RING3, 31, "EIP201_GLOBAL", 0, ACTIVE_LOW)

#endif
#endif

// Driver allows for NULL packet pointers for record invalidation commands
#define ADAPTER_EIP202_INVALIDATE_NULL_PKT_POINTER

// Command Descriptor offset
#ifdef DDK_EIP202_CD_OFFSET_BYTE_COUNT
#define ADAPTER_EIP202_CD_OFFSET_BYTE_COUNT     DDK_EIP202_CD_OFFSET_BYTE_COUNT
#endif

// Result Descriptor offset
#ifdef DDK_EIP202_RD_OFFSET_BYTE_COUNT
#define ADAPTER_EIP202_RD_OFFSET_BYTE_COUNT     DDK_EIP202_RD_OFFSET_BYTE_COUNT
#endif


#endif // CS_ADAPTER_EXT_H_


/* end of file cs_adapter_ext.h */
