/* c_device_generic.h
 *
 * This is the default configuration file for the generic Driver Framework
 * implementation.
 */

/*****************************************************************************
* Copyright (c) 2017-2020 by Rambus, Inc. and/or its subsidiaries.
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

// Top-level Hardware platform configuration
#include "cs_hwpal.h"

// Enables strict argument checking for input parameters
//#define HWPAL_STRICT_ARGS_CHECK

// choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT
#ifndef LOG_SEVERITY_MAX
#define LOG_SEVERITY_MAX LOG_SEVERITY_INFO
#endif

#ifndef HWPAL_MAX_DEVICE_NAME_LENGTH
#error "HWPAL_MAX_DEVICE_NAME_LENGTH undefined"
#endif

// Some magic number for device data validation
//#define HWPAL_DEVICE_MAGIC   0xBABADEDAUL


/* end of file c_device_generic.h */
