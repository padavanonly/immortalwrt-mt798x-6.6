/* cs_adapter_global.h
 *
 * Configuration Settings for the Global Control Adapter module.
 */

/*****************************************************************************
* Copyright (c) 2011-2020 by Rambus, Inc. and/or its subsidiaries.
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

// we accept a few settings from the top-level configuration file
#include "cs_driver.h"

// Adapter extensions
#include "cs_adapter_ext.h"


/****************************************************************************
 * Adapter Global configuration parameters
 */

// log level for the entire adapter (for now)
// choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT
#ifndef LOG_SEVERITY_MAX
#ifdef DRIVER_PERFORMANCE
#define LOG_SEVERITY_MAX  LOG_SEVERITY_CRITICAL
#else
#define LOG_SEVERITY_MAX  LOG_SEVERITY_WARN
#endif
#endif

#ifdef DRIVER_NAME
#define ADAPTER_GLOBAL_DRIVER_NAME     DRIVER_NAME
#else
#define ADAPTER_GLOBAL_DRIVER_NAME     "SecurityGlobal"
#endif

#ifdef DRIVER_LICENSE
#define ADAPTER_GLOBAL_LICENSE         DRIVER_LICENSE
#else
#define ADAPTER_GLOBAL_LICENSE         "GPL"
#endif

#define ADAPTER_GLOBAL_DBG_STATISTICS

#define ADAPTER_GLOBAL_EIP97_NOF_PES DRIVER_MAX_NOF_PE_TO_USE


// PCI configuration value: Cache Line Size, in 32bit words
// Advised value: 1
#define ADAPTER_PCICONFIG_CACHELINESIZE             1

// PCI configuration value: Master Latency Timer, in PCI bus clocks
// Advised value: 0xf8
#define ADAPTER_PCICONFIG_MASTERLATENCYTIMER    0xf8

// filter for printing interrupts
//#define ADAPTER_GLOBAL_INTERRUPTS_TRACEFILTER 0x0007FFFF

// Is host platform 64-bit?
#ifdef DRIVER_64BIT_HOST
#define ADAPTER_64BIT_HOST
#endif  // DRIVER_64BIT_HOST


#define ADAPTER_GLOBAL_DEVICE_NAME          "EIP197_GLOBAL"

/****************************************************************************
 * Adapter Classification (Global Control) configuration parameters
 */

#define ADAPTER_CS_GLOBAL_DEVICE_NAME       ADAPTER_GLOBAL_DEVICE_NAME

// Maximum number of EIP-207c Classification Engines that can be used
#ifdef DRIVER_CS_MAX_NOF_CE_TO_USE
#define ADAPTER_CS_GLOBAL_MAX_NOF_CE_TO_USE DRIVER_CS_MAX_NOF_CE_TO_USE
#else
#define ADAPTER_CS_GLOBAL_MAX_NOF_CE_TO_USE            1
#endif

// Maximum supported number of flow hash tables
#ifdef DRIVER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE
#define ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE  \
                        DRIVER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE
#else
#define ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE     1
#endif

#ifdef DRIVER_INTERRUPTS
#define ADAPTER_PEC_INTERRUPTS_ENABLE
#define ADAPTER_EIP74_INTERRUPTS_ENABLE
#define ADAPTER_EIP74_ERR_IRQ IRQ_DRBG_ERR // DRBG error interrupt
#define ADAPTER_EIP74_RES_IRQ IRQ_DRBG_RES // DRBG reseed early interrupt
#endif




#define ADAPTER_EIP202_DEVICES


/* end of file cs_adapter.h */
