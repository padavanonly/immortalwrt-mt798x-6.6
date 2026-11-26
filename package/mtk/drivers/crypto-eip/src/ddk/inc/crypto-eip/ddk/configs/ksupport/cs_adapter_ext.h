/* cs_adapter_ext.h
 *
 * Configuration Settings for the SLAD Adapter Combined module,
 * extensions.
 *
 */

/*****************************************************************************
* Copyright (c) 2012-2020 by Rambus, Inc. and/or its subsidiaries.
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
 * Adapter EIP-202 configuration parameters
 */

#ifdef DRIVER_AIC_SEPARATE_IRQ
#define ADAPTER_EIP202_AIC_SEPARATE_IRQ
#endif
#ifdef DRIVER_RING_AIC_DISABLE
#define ADAPTER_EIP202_RING_AIC_DISABLE
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
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR11, 23, "EIP201_RING3", 1, ACTIVE_LOW)

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
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR2, 4, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR2, 5, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR3, 6, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR3, 7, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR4, 8, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR4, 9, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR5, 10, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR5, 11, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR6, 12, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR6, 13, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR7, 14, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR7, 15, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR8, 16, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR8, 17, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR9, 18, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR9, 19, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR10, 20, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR10, 21, "EIP201_RING0", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_CDR11, 22, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RDR11, 23, "EIP201_RING1", 1, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RING0, 28, "EIP201_GLOBAL", 0, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RING1, 29, "EIP201_GLOBAL", 0, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RING2, 30, "EIP201_GLOBAL", 0, ACTIVE_LOW), \
    ADAPTER_EIP202_ADD_IRQ(IRQ_RING3, 31, "EIP201_GLOBAL", 0, ACTIVE_LOW)

#endif
#endif

// Enables the EIP-207 Record Cache interface for the record invalidation
#define ADAPTER_EIP202_RC_SUPPORT

#ifdef DRIVER_DMARESOURCE_BANKS_ENABLE
#define ADAPTER_EIP202_DMARESOURCE_BANKS_ENABLE

#define ADAPTER_EIP202_TRANSFORM_RECORD_COUNT      \
                                        DRIVER_TRANSFORM_RECORD_COUNT
#define ADAPTER_EIP202_TRANSFORM_RECORD_BYTE_COUNT \
                                        DRIVER_TRANSFORM_RECORD_BYTE_COUNT
#endif // DRIVER_DMARESOURCE_BANKS_ENABLE


#endif // CS_ADAPTER_EXT_H_


/* end of file cs_adapter_ext.h */
