/* c_adapter_memxs.h
 *
 * Default MemXS Adapter Module Configuration
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

#ifndef C_ADAPTER_MEMXS_H_
#define C_ADAPTER_MEMXS_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level Driver Framework configuration
#include "cs_hwpal.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Here is the dependency on the Driver Framework configuration
// via the MemXS configuration
#ifndef HWPAL_DEVICES
#error "Expected HWPAL_DEVICES defined by cs_memxs.h"
#endif


#endif /* C_ADAPTER_MEMXS_H_ */


/* end of file c_adapter_memxs.h */

