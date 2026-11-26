/* shdevxs_kernel_init.c
 *
 * Initiializ/uninitialize the kernel support driver.
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
#include "shdevxs_kernel.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_shdevxs.h"

// Logging API
#include "log.h"            // LOG_*

// Driver Framework Device API
#include "device_mgmt.h"    // Device_Initialize, Device_UnInitialize
#include "device_rw.h"      // Device_Read32, Device_Write32

// Driver Framework Basic Definitions API
#include "basic_defs.h"     // bool, true, false

// KernelSupport_RC_Initialize/UnInitialize
#include "shdevxs_kernel_internal.h"


/*----------------------------------------------------------------------------
 * Local variables
 */

static bool SHDevXS_IsInitialized = false;

#ifdef SHDEVXS_ENABLE_FUNCTIONS
Device_Handle_t SHDevXS_Device;
#endif


/*----------------------------------------------------------------------------
 * SHDevXS_Init
 */
int
SHDevXS_Init(void)
{
    int nIRQ = -1;

    if (SHDevXS_IsInitialized != false)
    {
        LOG_WARN("SHDevXS_Init: Already initialized\n");
        return 0;
    }

    // trigger first-time initialization.
    if (Device_Initialize(&nIRQ) < 0)
        return -1;

#ifdef SHDEVXS_ENABLE_FUNCTIONS
    SHDevXS_Device = Device_Find(SHDEVXS_DEVICE_NAME);
    if (SHDevXS_Device == NULL)
    {
        LOG_CRIT("SHDevXS_Init: Failed to locate %s\n", SHDEVXS_DEVICE_NAME);
        return -1;
    }
#endif // SHDEVXS_ENABLE_FUNCTIONS

#ifdef SHDEVXS_ENABLE_DMAPOOL_FUNCTIONS
    if (SHDevXS_Internal_DMAPool_Initialize() != 0)
    {
        Device_UnInitialize();
        return -1;
    }
#endif

    LOG_CRIT("SHDevXS_Init Finished\n");
    SHDevXS_IsInitialized = true;
    return 0;
}

/*----------------------------------------------------------------------------
 * SHDevXS_UnInit
 */
int
SHDevXS_UnInit(void)
{
#ifdef SHDEVXS_ENABLE_RC_FUNCTIONS
    SHDevXS_Internal_RC_UnInitialize();
#endif
    Device_UnInitialize();
    LOG_CRIT("SHDevXS_UnInit Finished\n");
    SHDevXS_IsInitialized = false;
    return 0;
}

/*----------------------------------------------------------------------------
 * SHDevXS_Cleanup
 */
void
SHDevXS_Cleanup(void *AppID)
{
#ifdef SHDEVXS_ENABLE_DMAPOOL_FUNCTIONS
    SHDevXS_Internal_DMAPool_Uninit(AppID);
#endif
    SHDevXS_Internal_IRQ_Cleanup(AppID);
}


/* shdevxs_kernel_init.c*/
