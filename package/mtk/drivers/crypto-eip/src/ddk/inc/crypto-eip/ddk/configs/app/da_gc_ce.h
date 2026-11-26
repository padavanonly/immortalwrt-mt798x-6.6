/* da_gc_ce.h
 *
 * Demo Application Classification Engine Module API
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

#ifndef DA_GC_CE_H_
#define DA_GC_CE_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // bool


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Global_Cs_StatusReport()
 *
 * Obtain all available global status information from the Global Classification
 * hardware and report it.
 */
void
Global_Cs_StatusReport(void);


/*----------------------------------------------------------------------------
 * Global_Cs_Init()
 *
 */
bool
Global_Cs_Init(void);


/*----------------------------------------------------------------------------
 * Global_Cs_UnInit()
 *
 */
void
Global_Cs_UnInit(void);


#endif // DA_GC_CE_H_


/* end of file da_gc_ce.h */
