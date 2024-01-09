/* c_list.h
 *
 * Default configuration for the List module
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

#ifndef C_LIST_H_
#define C_LIST_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level configuration
#include "cs_list.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Maximum supported number of the list instances
#ifndef LIST_MAX_NOF_INSTANCES
#define LIST_MAX_NOF_INSTANCES      2
#endif // LIST_MAX_NOF_INSTANCES

// Strict argument checking
//#define LIST_STRICT_ARGS


#endif /* C_LIST_H_ */


/* end of file c_list.h */
