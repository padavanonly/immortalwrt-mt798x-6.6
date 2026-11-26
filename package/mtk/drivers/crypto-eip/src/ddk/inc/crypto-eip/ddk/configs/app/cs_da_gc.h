/* cs_da_gc.h
 *
 * Configuration Settings for the Demo App Global Control.
 */

/*****************************************************************************
* Copyright (c) 2012-2021 by Rambus, Inc. and/or its subsidiaries.
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


/****************************************************************************
 * DemoApp Global Control configuration parameters
 */

// log level for the entire adapter (for now)
// choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT
#define LOG_SEVERITY_MAX  LOG_SEVERITY_WARN

#define DEMOAPP_GLOBAL_NAME            "GlobalControl"

#define DEMOAPP_GLOBAL_LICENSE         "GPL"

//#define DEMOAPP_GLOBAL_MAX_NOF_RING_TO_USE 4


#define DA_GLOBAL_USE_INTERRUPTS
#define DA_GC_GENERATE_BLOCK_SIZE 128
#define DA_GC_RESEED_THR          0xffffffff
#define DA_GC_RESEED_THR_EARLY    128

#define DA_GC_DBG_STATISTICS

/* end of file cs_da_gc.h */
