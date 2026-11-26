/* c_da_gc_pe.h
 *
 * Default Demo Application Configuration,
 * Packet Engine Module
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

#include "cs_da_gc.h"

// this file is included by globalcontrol.c to get configuration switches

// log level for the entire adapter (for now)
// choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT
#ifndef LOG_SEVERITY_MAX
#define LOG_SEVERITY_MAX  LOG_SEVERITY_INFO
#endif

#ifndef DEMOAPP_GLOBAL_LICENSE
#define DEMOAPP_GLOBAL_LICENSE "GPL"
#endif

// Packet get retries
#ifndef DEMOAPP_GLOBAL_RETRY_COUNT
#define DEMOAPP_GLOBAL_RETRY_COUNT      10
#endif // DEMOAPP_GLOBAL_RETRY_COUNT

// Packet get wait timeout (in milliseconds)
#ifndef DEMOAPP_GLOBAL_TIMEOUT_MS
#define DEMOAPP_GLOBAL_TIMEOUT_MS      10
#endif // DEMOAPP_GLOBAL_RETRY_COUNT

#ifndef DEMOAPP_GLOBAL_NOF_PES
#define DEMOAPP_GLOBAL_NOF_PES           1
#endif


/* end of file c_da_gc_pe.h */
