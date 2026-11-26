/* adapter_driver97_init.c
 *
 * Adapter top level module, Security-IP-97 driver's entry point.
 */

/*****************************************************************************
* Copyright (c) 2011-2020 by Rambus, Inc. and/or its subsidiaries.
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

#include "api_driver97_init.h"    // Driver Init API


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default Adapter configuration
#include "cs_adapter.h"      // ADAPTER_DRIVER_NAME

// Adapter Initialization API
#include "adapter_init.h"   // Adapter_*
#include "adapter_global_init.h"

// Logging API
#include "log.h"            // LOG_INFO


/*----------------------------------------------------------------------------
 * Driver97_Init
 */
int
Driver97_Init(void)
{
    LOG_INFO("\n\t Driver97_Init \n");

    LOG_INFO("%s driver: initializing\n", ADAPTER_DRIVER_NAME);

    Adapter_Report_Build_Params();

    if (!Adapter_Init())
    {
        return -1;
    }

    if (!Adapter_Global_Init())
    {
        Adapter_UnInit();
        return -1;
    }

    LOG_INFO("\n\t Driver97_Init done \n");

    return 0;   // success
}


/*----------------------------------------------------------------------------
 * Driver97_Exit
 */
void
Driver97_Exit(void)
{
    LOG_INFO("\n\t Driver97_Exit \n");

    LOG_INFO("%s driver: exit\n", ADAPTER_DRIVER_NAME);

    Adapter_Global_UnInit();
    Adapter_UnInit();

    LOG_INFO("\n\t Driver97_Exit done \n");
}

#include "adapter_driver97_init_ext.h"

/* end of file adapter_driver97_init.c */
