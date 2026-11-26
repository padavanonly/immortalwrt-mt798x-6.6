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

// Propagate the AIC and IRQ definitions from cs_adapter_ext.h to Int dispatcher
#define ADAPTER_EIP202_ADD_AIC(_name,_idx,_isirq)           \
    INTDISPATCH_AIC_RESOURCE_ADD(_name, _idx, _isirq)
#define INTDISPATCH_AIC_RESOURCES ADAPTER_EIP202_AICS
#define ADAPTER_EIP202_ADD_IRQ(_name,_phy,_aic,_flag,_pol)                \
    INTDISPATCH_RESOURCE_ADD(_name,(1<<(_phy)),_aic,_flag,_pol)
#define INTDISPATCH_RESOURCES ADAPTER_EIP202_IRQS

// select which interrupts to trace
// comment-out or set to zero to disable tracing
#define INTDISPATCHER_TRACE_FILTER 0x00000000


#endif /* Include Guard */


/* end of file cs_intdispatch_umdevxs.h */
