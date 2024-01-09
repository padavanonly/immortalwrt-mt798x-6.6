/* adapter_driver97_pec_init_ext.h
 *
 * Linux kernel specific Adapter extensions
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


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "api_dmabuf.h"     // DMABuf API


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level Adapter configuration
#include "cs_adapter.h"

// Linux Kernel API
#include <linux/init.h>     // module_init, module_exit
#include <linux/module.h>   // EXPORT_SYMBOL


MODULE_LICENSE(ADAPTER_LICENSE);

module_init(Driver97_PEC_Init);
module_exit(Driver97_PEC_Exit);

EXPORT_SYMBOL(DMABuf_NULLHandle);
EXPORT_SYMBOL(DMABuf_Handle_IsSame);
EXPORT_SYMBOL(DMABuf_Alloc);
EXPORT_SYMBOL(DMABuf_Register);
EXPORT_SYMBOL(DMABuf_Release);


// PEC API LKM implementation extensions
#include "adapter_pec_lkm_ext.h"


/* end of file adapter_driver97_pec_init_ext.h */
