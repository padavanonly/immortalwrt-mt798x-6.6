/* rpm_device_macros.h
 *
 * Runtime Power Management (RPM) Device Macros API
 *
 */

/*****************************************************************************
* Copyright (c) 2015-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_RPM_DEVICE_MACROS_H
#define INCLUDE_GUARD_RPM_DEVICE_MACROS_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"     // false, IDENTIFIER_NOT_USED


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define RPM_SUCCESS     0   // No error

#define RPM_DEVICE_CAPABILITIES_STR_MACRO     "RPM stubbed"


/*----------------------------------------------------------------------------
 * RPM_DEVICE_INIT_START_MACRO
 *
 * Expands into a single line expression
 */
#define RPM_DEVICE_INIT_START_MACRO(DevId, SuspendFunc, ResumeFunc) RPM_SUCCESS

/*----------------------------------------------------------------------------
 * RPM_DEVICE_INIT_STOP_MACRO
 *
 * Expands into a single line expression
 */
#define RPM_DEVICE_INIT_STOP_MACRO(DevId) RPM_SUCCESS


/*----------------------------------------------------------------------------
 * RPM_DEVICE_UNINIT_START_MACRO
 *
 * Expands into a single line expression
 */
#define RPM_DEVICE_UNINIT_START_MACRO(DevId, fResume) RPM_SUCCESS


/*----------------------------------------------------------------------------
 * RPM_DEVICE_UNINIT_STOP_MACRO
 *
 * Expands into a single line expression
 */
#define RPM_DEVICE_UNINIT_STOP_MACRO(DevId) RPM_SUCCESS


/*----------------------------------------------------------------------------
 * RPM_DEVICE_IO_START_MACRO
 *
 * Expands into a single line expression
 */
#define RPM_DEVICE_IO_START_MACRO(DevId, flag) RPM_SUCCESS


/*----------------------------------------------------------------------------
 * RPM_DEVICE_IO_STOP_MACRO
 *
 * Expands into a single line expression
 */
#define RPM_DEVICE_IO_STOP_MACRO(DevId, flag) RPM_SUCCESS


#endif /* INCLUDE_GUARD_RPM_DEVICE_MACROS_H */


/* rpm_device_macros.h */
