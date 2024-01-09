/* adapter_pcl_lkm_ext.h
 *
 * PCL DTL API Linux kernel implementation extensions
 *
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

#ifndef ADAPTER_PCL_LKM_EXT_H_
#define ADAPTER_PCL_LKM_EXT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Linux Kernel API
#include <linux/init.h>     // module_init, module_exit
#include <linux/module.h>   // EXPORT_SYMBOL


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "api_pcl.h"        // PCL API

#include "api_pcl_dtl.h"    // PCL DTL API


EXPORT_SYMBOL(PCL_Init);
EXPORT_SYMBOL(PCL_UnInit);
EXPORT_SYMBOL(PCL_Flow_DMABuf_Handle_Get);
EXPORT_SYMBOL(PCL_Flow_Hash);
EXPORT_SYMBOL(PCL_Flow_Alloc);
EXPORT_SYMBOL(PCL_Flow_Add);
EXPORT_SYMBOL(PCL_Flow_Remove);
EXPORT_SYMBOL(PCL_Flow_Release);
EXPORT_SYMBOL(PCL_Flow_Get_ReadOnly);
EXPORT_SYMBOL(PCL_Transform_Register);
EXPORT_SYMBOL(PCL_Transform_UnRegister);
EXPORT_SYMBOL(PCL_Transform_Get_ReadOnly);

EXPORT_SYMBOL(PCL_DTL_Init);
EXPORT_SYMBOL(PCL_DTL_UnInit);
EXPORT_SYMBOL(PCL_DTL_Transform_Add);
EXPORT_SYMBOL(PCL_DTL_Transform_Remove);
EXPORT_SYMBOL(PCL_DTL_Hash_Remove);


#endif // ADAPTER_PCL_LKM_EXT_H_


/* end of file adapter_pcl_lkm_ext.h */
