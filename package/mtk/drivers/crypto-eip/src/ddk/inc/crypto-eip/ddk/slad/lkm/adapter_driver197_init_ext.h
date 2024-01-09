/* adapter_driver197_init_ext.h
 *
 * Linux kernel specific Adapter extensions
 */

/*****************************************************************************
* Copyright (c) 2010-2022 by Rambus, Inc. and/or its subsidiaries.
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

// Global Classification Control Init API
#include "api_global_eip207.h"

// Global Classification Control Status API
#include "api_global_status_eip207.h"

// Global Packet I/O Control Init API
#include "api_global_eip97.h"

// Global Packet I/O Control Status API
#include "api_global_status_eip97.h"

// Global DRBG Control API
#include "api_global_eip74.h"

#include "api_dmabuf.h"     // DMABuf API

#include "device_mgmt.h" // Device_Find
#include "device_rw.h" // Device_Read32/Device_Write32

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level Adapter configuration
#include "cs_adapter.h"

// Linux Kernel API
#include <linux/init.h>     // module_init, module_exit
#include <linux/module.h>   // EXPORT_SYMBOL


MODULE_LICENSE(ADAPTER_LICENSE);

module_init(Driver197_Init);
module_exit(Driver197_Exit);

// Global Classification Control Init API
EXPORT_SYMBOL(GlobalControl207_Capabilities_Get);
EXPORT_SYMBOL(GlobalControl207_Init);
EXPORT_SYMBOL(GlobalControl207_UnInit);

// Global Packet I/O Control Status API
EXPORT_SYMBOL(GlobalControl97_Capabilities_Get);
EXPORT_SYMBOL(GlobalControl97_Init);
EXPORT_SYMBOL(GlobalControl97_UnInit);
EXPORT_SYMBOL(GlobalControl97_Configure);

// Global Classification Control Status API
EXPORT_SYMBOL(GlobalControl207_Status_Get);
EXPORT_SYMBOL(GlobalControl207_GlobalStats_Get);
EXPORT_SYMBOL(GlobalControl207_ClockCount_Get);
EXPORT_SYMBOL(GlobalControl207_Firmware_Configure);

// Global Packet I/O Control Status API
EXPORT_SYMBOL(GlobalControl97_Debug_Statistics_Get);
EXPORT_SYMBOL(GlobalControl97_DFE_Status_Get);
EXPORT_SYMBOL(GlobalControl97_DSE_Status_Get);
EXPORT_SYMBOL(GlobalControl97_Token_Status_Get);
EXPORT_SYMBOL(GlobalControl97_Context_Status_Get);
EXPORT_SYMBOL(GlobalControl97_Interrupt_Status_Get);
EXPORT_SYMBOL(GlobalControl97_OutXfer_Status_Get);
EXPORT_SYMBOL(GlobalControl97_PRNG_Status_Get);
EXPORT_SYMBOL(GlobalControl97_PRNG_Reseed);
EXPORT_SYMBOL(GlobalControl97_Interfaces_Get);

// Global DRBG Control API
EXPORT_SYMBOL(GlobalControl74_Init);
EXPORT_SYMBOL(GlobalControl74_UnInit);
EXPORT_SYMBOL(GlobalControl74_Capabilities_Get);
EXPORT_SYMBOL(GlobalControl74_Status_Get);
EXPORT_SYMBOL(GlobalControl74_Reseed);
EXPORT_SYMBOL(GlobalControl74_Clear);
#ifdef ADAPTER_EIP74_INTERRUPTS_ENABLE
EXPORT_SYMBOL(GlobalControl74_Notify_Request);
#endif

EXPORT_SYMBOL(DMABuf_NULLHandle);
EXPORT_SYMBOL(DMABuf_Handle_IsSame);
EXPORT_SYMBOL(DMABuf_Alloc);
EXPORT_SYMBOL(DMABuf_Register);
EXPORT_SYMBOL(DMABuf_Release);


EXPORT_SYMBOL(Device_Find);
EXPORT_SYMBOL(Device_Read32);
EXPORT_SYMBOL(Device_Write32);

// PEC API LKM implementation extensions
#include "adapter_pec_lkm_ext.h"

#ifdef ADAPTER_PCL_ENABLE
// PCL API LKM implementation extensions
#include "adapter_pcl_lkm_ext.h"
#endif /* ADAPTER_PCL_ENABLE */


/* end of file adapter_driver197_init_ext.h */
