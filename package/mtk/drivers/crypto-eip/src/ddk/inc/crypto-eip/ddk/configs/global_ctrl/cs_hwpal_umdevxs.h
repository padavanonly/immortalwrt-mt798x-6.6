/* cs_hwpal_umdevxs.h
 *
 * Configuration for Driver Framework Device API implementation for
 * Linux user-space.
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

#ifndef INCLUDE_GUARD_CS_HWPAL_UMDEVXS_H
#define INCLUDE_GUARD_CS_HWPAL_UMDEVXS_H

// we accept a few settings from the top-level configuration file
#include "cs_hwpal.h"

//Define this to override the overall HWPAL log level.
//#define HWPAL_UMDEVXS_LOG_SEVERITY  LOG_SEVERITY_WARN
#ifdef HWPAL_UMDEVXS_LOG_SEVERITY
#undef LOG_SEVERITY_MAX
#define LOG_SEVERITY_MAX HWPAL_UMDEVXS_LOG_SEVERITY
#endif

#define HWPAL_DEVICE0_UMDEVXS  HWPAL_DEVICE_TO_FIND

// Check if the endianness conversion must be performed
#ifdef DRIVER_SWAPENDIAN
#define HWPAL_DEVICE_ENABLE_SWAP
#endif // DRIVER_SWAPENDIAN

// Remove Attach/Detach functions.
#define HWPAL_DMARESOURCE_REMOVE_ATTACH


#endif // INCLUDE_GUARD_CS_HWPAL_UMDEVXS_H


/* end of file cs_hwpal_umdevxs.h */
