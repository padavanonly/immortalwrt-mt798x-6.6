/* cs_intdispatch_umdevxs.h
 *
 * Configuration Settings for the Interrupt Dispatcher.
 */

/*****************************************************************************
* Copyright (c) 2010-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_CS_INTDISPATCH_UMDEVXS_H
#define INCLUDE_GUARD_CS_INTDISPATCH_UMDEVXS_H

#include "cs_driver.h"
#include "cs_adapter.h"
#include "cs_intdispatch_umdevxs_ext.h"

// logging level for Interrupt Dispatcher
// Choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT
#undef LOG_SEVERITY_MAX
#ifdef DRIVER_PERFORMANCE
#define INTDISPATCH_LOG_SEVERITY  LOG_SEVERITY_CRITICAL
#else
#define INTDISPATCH_LOG_SEVERITY  LOG_SEVERITY_WARN
#endif


// Definition of interrupt resources
// the interrupt sources is a bitmask of interrupt sources
// Refer to the data sheet of device for the correct values
// Config: Choose from RISING_EDGE, FALLING_EDGE, ACTIVE_HIGH, ACTIVE_LOW
//                            Name                  Inputs,   Config
#define INTDISPATCH_RESOURCES \
    INTDISPATCH_RESOURCE_ADD(ADAPTER_EIP202_DFE0_INT_NAME, \
                             1<<ADAPTER_EIP202_PHY_DFE0_IRQ, \
                             ACTIVE_LOW), \
    INTDISPATCH_RESOURCE_ADD(ADAPTER_EIP202_DSE0_INT_NAME, \
                             1<<ADAPTER_EIP202_PHY_DSE0_IRQ, \
                             ACTIVE_LOW), \
    INTDISPATCH_RESOURCE_ADD(ADAPTER_EIP202_RING0_INT_NAME, \
                             1<<ADAPTER_EIP202_PHY_RING0_IRQ, \
                             ACTIVE_LOW), \
    INTDISPATCH_RESOURCE_ADD(ADAPTER_EIP202_PE0_INT_NAME, \
                             1<<ADAPTER_EIP202_PHY_PE0_IRQ, \
                             ACTIVE_LOW), \
    INTDISPATCH_RESOURCE_ADD(ADAPTER_EIP202_CDR0_INT_NAME, \
                             1<<(ADAPTER_EIP202_PHY_CDR0_IRQ+30), \
                             ACTIVE_LOW), \
    INTDISPATCH_RESOURCE_ADD(ADAPTER_EIP202_RDR0_INT_NAME, \
                             1<<(ADAPTER_EIP202_PHY_RDR0_IRQ+30),  \
                             ACTIVE_LOW)

// select which interrupts to trace
// comment-out or set to zero to disable tracing
#define INTDISPATCHER_TRACE_FILTER 0x0000FFFF


#endif /* Include Guard */


/* end of file cs_intdispatch_umdevxs.h */
