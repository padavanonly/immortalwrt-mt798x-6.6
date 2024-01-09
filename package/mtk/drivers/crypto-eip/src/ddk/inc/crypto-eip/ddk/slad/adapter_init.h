/* adapter_init.h
 *
 * Data types and Interfaces
 */

/*****************************************************************************
* Copyright (c) 2008-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_ADAPTER_INIT_H
#define INCLUDE_GUARD_ADAPTER_INIT_H

// Driver Framework Basic Defs API
#include "basic_defs.h"

/*----------------------------------------------------------------------------
 *                           Adapter initialization
 *----------------------------------------------------------------------------
 */

bool
Adapter_Init(void);

void
Adapter_UnInit(void);

void
Adapter_Report_Build_Params(void);

#endif /* Include Guard */

/* end of file adapter_init.h */
