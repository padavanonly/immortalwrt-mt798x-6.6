/* shdevxs_init.c
 *
 * Shared Device Access for Global initialization and test.
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

#include "shdevxs_init.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_shdevxs.h"

// SHDevXS_Internal_IRQ_Initialize
#include "shdevxs_kernel_internal.h"

// UMDevXS Device API
#include "umdevxs_device.h"     // UMDevXS_Device_*

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // IDENTIFIER_NOT_USED

#include "log.h"

// Linux kernel API
#include <linux/module.h>



/*----------------------------------------------------------------------------
 * Definitions and macros
 */
EXPORT_SYMBOL(SHDevXS_Global_SetPrivileged);
EXPORT_SYMBOL(SHDevXS_Global_Init);
EXPORT_SYMBOL(SHDevXS_Global_UnInit);
EXPORT_SYMBOL(SHDevXS_Test);


/*----------------------------------------------------------------------------
 * Local variables
 */

/*----------------------------------------------------------------------------
 * SHDevXS_Global_SetPrivileged
 */
int
SHDevXS_Global_SetPrivileged(
        void)
{
    LOG_INFO("%s called\n", __func__);

    if (UMDevXS_Device_SetPrivileged(UMDEVXS_KERNEL_APPID) < 0)
    {
        LOG_CRIT("%s failed\n", __func__);
        return -1; // Error
    }

    return 0;
}


/*----------------------------------------------------------------------------
 * SHDevXS_Global_Init
 */
int
SHDevXS_Global_Init(
        void)
{
    LOG_INFO("%s called\n", __func__);

#ifdef SHDEVXS_ENABLE_RC_FUNCTIONS
    if (SHDevXS_Internal_RC_Initialize() < 0)
    {
        LOG_CRIT("%s failed\n", __func__);
        return -1;
    }
#endif

#ifndef SHDEVXS_DISABLE_INTERRUPTS
    return SHDevXS_Internal_IRQ_Initialize();
#else
    return 0;
#endif
}


/*----------------------------------------------------------------------------
 * SHDevXS_Global_UnInit
 *
 */
int
SHDevXS_Global_UnInit(
        void)
{
    LOG_INFO("%s called\n", __func__);

#ifndef SHDEVXS_DISABLE_INTERRUPTS
    SHDevXS_Internal_IRQ_UnInitialize();
#endif

#ifdef SHDEVXS_ENABLE_RC_FUNCTIONS
    SHDevXS_Internal_RC_UnInitialize();
#endif
    return 0;
}

/*----------------------------------------------------------------------------
 * SHDevXS_Test
 */
int
SHDevXS_Test(void)
{
    LOG_INFO("%s called\n", __func__);

    return 0;
}



/* end of file shdevxs_init.c */


