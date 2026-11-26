/* adapter_global_driver97_init.c
 *
 * Adapter top level module,
 * Security-IP-97 global control driver's entry point.
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

#include "api_global_driver97_init.h"    // Driver Init API


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level Adapter configuration
#include "c_adapter_global.h"             // ADAPTER_GLOBAL_DRIVER_NAME

// Adapter Initialization API - for global only tjc
#include "adapter_global_control_init.h"  // Adapter_* for global only

// Logging API
#include "log.h"                          // LOG_INFO


/*----------------------------------------------------------------------------
 * Driver97_Global_Init
 */
int
Driver97_Global_Init(void)
{
    LOG_INFO("\n\t Driver97_Global_Init \n");

    LOG_INFO("%s driver: initializing\n", ADAPTER_GLOBAL_DRIVER_NAME);

    Adapter_Global_Control_Report_Build_Params();

    if (!Adapter_Global_Control_Init())
    {
        return -1;
    }

    LOG_INFO("\n\t Driver97_Global_Init done \n");

    return 0;   // success
}


/*----------------------------------------------------------------------------
 * Driver97_Global_Exit
 */
void
Driver97_Global_Exit(void)
{
    LOG_INFO("\n\t Driver97_Global_Exit \n");

    LOG_INFO("%s driver: exit\n", ADAPTER_GLOBAL_DRIVER_NAME);

    Adapter_Global_Control_UnInit();

    LOG_INFO("\n\t Driver97_Global_Exit done \n");
}


#include "adapter_driver97_global_init_ext.h"


/* end of file adapter_global_driver97_init.c */
