/* rpm_kernel_macros.h
 *
 * Runtime Power Management (RPM) Kernel Macros API
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

#ifndef INCLUDE_GUARD_RPM_KERNEL_MACROS_H
#define INCLUDE_GUARD_RPM_KERNEL_MACROS_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Forward declarations
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define RPM_SUCCESS     0   // No error

#define RPM_OPS_INIT

#define RPM_OPS_PM      NULL


/*----------------------------------------------------------------------------
 * RPM_INIT_MACRO
 *
 * Expands into a single line expression
 */
#define RPM_INIT_MACRO(data)    RPM_SUCCESS


/*----------------------------------------------------------------------------
 * RPM_Uninit
 *
 * Expands into a single line expression
 */
#define RPM_UNINIT_MACRO()    RPM_SUCCESS


#endif /* INCLUDE_GUARD_RPM_KERNEL_MACROS_H */


/* rpm_kernel_macros.h */
