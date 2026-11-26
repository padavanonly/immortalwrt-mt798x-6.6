/* cs_list.h
 *
 * Top-level configuration for the List module
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

#ifndef CS_LIST_H_
#define CS_LIST_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level configuration
#include "cs_driver.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Maximum supported number of the list instances
#ifdef DRIVER_LIST_MAX_NOF_INSTANCES
#define LIST_MAX_NOF_INSTANCES      DRIVER_LIST_MAX_NOF_INSTANCES
#endif

// Strict argument checking
#ifndef DRIVER_PERFORMANCE
#define LIST_STRICT_ARGS
#endif


#endif /* CS_LIST_H_ */


/* end of file cs_list.h */
