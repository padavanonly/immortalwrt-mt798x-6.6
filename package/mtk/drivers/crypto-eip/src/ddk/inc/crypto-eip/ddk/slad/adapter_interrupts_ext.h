/* adapter_interrupts_ext.h
 *
 * Host hardware specific extensions for the Adapter Interrupts interface
 */

/*****************************************************************************
* Copyright (c) 2011-2022 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef ADAPTER_INTERRUPTS_EXT_H_
#define ADAPTER_INTERRUPTS_EXT_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Adapter EIP-202 Default configuration
#include "c_adapter_eip202.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define ADAPTER_EIP202_ADD_IRQ(_name,_phy,_aicname,_tasklet,_pol)  \
    _name

// Adapter logical EIP-(1)97 interrupts
enum
{
    ADAPTER_EIP202_IRQS
};

#undef ADAPTER_EIP202_ADD_IRQ

#endif /* ADAPTER_INTERRUPTS_EXT_H_ */

/* end of file adapter_interrupts_ext.h */
