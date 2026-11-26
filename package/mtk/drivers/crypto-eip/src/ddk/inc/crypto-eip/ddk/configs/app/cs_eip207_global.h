/* cs_eip207_global.h
 *
 * Top-level configuration parameters
 * for the EIP-207 Global Control
 *
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

#ifndef CS_EIP207_GLOBAL_H_
#define CS_EIP207_GLOBAL_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Maximum supported number of flow hash tables
#ifdef DEMOAPP_GLOBAL_MAX_NOF_FLOW_HASH_TABLES_TO_USE
#define EIP207_MAX_NOF_FLOW_HASH_TABLES_TO_USE    \
                           DEMOAPP_GLOBAL_MAX_NOF_FLOW_HASH_TABLES_TO_USE
#else
#define EIP207_MAX_NOF_FLOW_HASH_TABLES_TO_USE     1
#endif

// Maximum supported number of FRC/TRC/ARC4 cache sets
#ifdef DEMOAPP_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE
#define EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE     \
                                DEMOAPP_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE
#else
#define EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE    1
#endif


#endif /* CS_EIP207_GLOBAL_H_ */


/* end of file cs_eip207_global.h */
