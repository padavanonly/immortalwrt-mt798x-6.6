/* cs_lkm.h
 *
 * Top-level LKM configuration.
 */

/*****************************************************************************
* Copyright (c) 2016-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef CS_LKM_H
#define CS_LKM_H

// Driver Framework top-level configuration
#include "cs_hwpal.h"


/*-----------------------------------------------------------------------------
 * Configuration parameters that may not be modified!
 */


/*-----------------------------------------------------------------------------
 * Configuration parameters that may be modified at top level configuration
 */

// Enables strict argument checking for input parameters
#define LKM_STRICT_ARGS_CHECK

// Choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT
#define LKM_LOG_SEVERITY LOG_SEVERITY_CRIT

// Device name in the Device Tree Structure
#ifdef HWPAL_PLATFORM_DEVICE_NAME
#define LKM_PLATFORM_DEVICE_NAME    HWPAL_PLATFORM_DEVICE_NAME
#endif

// Platform-specific number of the IRQ's that will be used by the device
#ifdef HWPAL_PLATFORM_IRQ_COUNT
#define LKM_PLATFORM_IRQ_COUNT      HWPAL_PLATFORM_IRQ_COUNT
#endif

// Platform-specific index of the IRQ that will be used
#ifdef HWPAL_PLATFORM_IRQ_IDX
#define LKM_PLATFORM_IRQ_IDX        HWPAL_PLATFORM_IRQ_IDX
#endif


#endif // CS_LKM_H


/* end of file cs_lkm.h */
