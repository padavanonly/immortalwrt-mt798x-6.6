/* c_eip201.h
 *
 * Configuration options for the EIP201 Driver Library module.
 * The project-specific cs_eip201.h file is included,
 * whereafter defaults are provided for missing parameters.
 */

/*****************************************************************************
* Copyright (c) 2007-2020 by Rambus, Inc. and/or its subsidiaries.
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

/*----------------------------------------------------------------
 * Defines that can be used in the cs_xxx.h file
 */

/* EIP201_STRICT_ARGS
 *
 * Set this option to enable checking of all arguments to all EIP201 DL
 * functions. Disable it to reduce code size and reduce overhead.
 */

/* EIP201_STRICT_ARGS_MAX_NUM_OF_INTERRUPTS <NOI>
 *
 * This configures the maximum Number Of Interrupt (NOI) sources
 * actually available in the EIP201 AIC.
 * This can be used for strict argument checking.
 */

/*----------------------------------------------------------------
 * inclusion of cs_eip201.h
 */

// EIP-201 Driver Library API
#include "cs_eip201.h"


/*----------------------------------------------------------------
 * provide backup values for all missing configuration parameters
 */
#if !defined(EIP201_STRICT_ARGS_MAX_NUM_OF_INTERRUPTS) \
             || (EIP201_STRICT_ARGS_MAX_NUM_OF_INTERRUPTS > 32)
#undef  EIP201_STRICT_ARGS_MAX_NUM_OF_INTERRUPTS
#define EIP201_STRICT_ARGS_MAX_NUM_OF_INTERRUPTS 32
#endif


/*----------------------------------------------------------------
 * other configuration parameters that cannot be set in cs_xxx.h
 * but are considered product-configurable anyway
 */


/*----------------------------------------------------------------
 * correct implementation-specific collisions
 */

#ifndef EIP201_REMOVE_INITIALIZE
// EIP201_Initialize internally depends on EIP201_Config_Change
#ifdef EIP201_REMOVE_CONFIG_CHANGE
#undef EIP201_REMOVE_CONFIG_CHANGE
#endif
#endif

#ifndef EIP201_LO_REG_BASE
#define EIP201_LO_REG_BASE 0x00
#endif
#ifndef EIP201_HI_REG_BASE
#define EIP201_HI_REG_BASE 0x00
#endif



/* end of file c_eip201.h */
