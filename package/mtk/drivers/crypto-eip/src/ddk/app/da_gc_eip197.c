/* da_gc_eip197.c
 *
 * Demo Application for the Driver197 Global Control API
 * Linux kernel-space and user-space.
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

// Default configuration
#include "c_da_gc_ce.h"         // configuration switches
#include "c_da_gc_pe.h"         // configuration switches

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint8_t, uint32_t, bool

// Demo Application DRBG Module API
#include "da_gc_drbg.h"

// Demo Application Classification Engine Module API
#include "da_gc_ce.h"

// Demo Application Packet Engine Module API
#include "da_gc_pe.h"

// Driver Initialization API
#include "api_global_driver197_init.h"

// Driver Framework C Library API
#include "clib.h"               // memcpy, ZEROINIT

#include "device_types.h"       // Device_Handle_t
#include "device_mgmt.h"        // Device_find
#include "log.h"                // Log API

#include "moddep.h"

#ifndef DA_GC_USERMODE
#define EXPORT_SYMTAB

#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE(DEMOAPP_GLOBAL_LICENSE);

#endif // !DA_GC_USERMODE


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */
int app_dep;
EXPORT_SYMBOL(app_dep);

static int
da_gc_module_init(void)
{
    LOG_INFO("DA_GC: da_gc_module_init, %s module exit\n",
             DEMOAPP_GLOBAL_NAME);

    // Initialize Packet I/O Global Control
    Global_Init();

    // Initialize Packet Classification Global Control
    Global_Cs_Init();

    // Initialize DRBG Global Control
    Global_DRBG_Init();

    LOG_INFO("DA_GC: da_gc_kmodule_init done \n");

    Log_FormattedMessage("DA_GC: da_gc_k loaded\n");

    return 0;   // success
}


static void
da_gc_module_exit(void)
{
    LOG_INFO("DA_GC: da_gc_module_exit, %s module exit\n",
             DEMOAPP_GLOBAL_NAME);

    // Uninitialize DRBG Global Control
    Global_DRBG_UnInit();

    // Uninitialize Packet Classification Global Control
    Global_Cs_UnInit();

    // Uninitialize Packet I/O Global Control
    Global_UnInit();

    LOG_INFO("DA_GC: da_gc_module_exit done \n");

    Log_FormattedMessage("DA_GC: da_gc_k unloaded\n");
}


#ifdef DA_GC_USERMODE

static void
da_gc_monitor()
{
    unsigned int InParam = 1;

    Log_FormattedMessage("DA_GC: status monitor started\n");

    // Status monitor
    while (InParam != 0)
    {
        Log_FormattedMessage("DA_GC: get status classification (1), "
                             "packet I/O (2), DRBG (3), Exit (0): ");
        scanf("%u", &InParam);

        switch(InParam)
        {
            case 0:
                // Exit
                break;

            case 1:
                // Packet Classification Global Control status report
                Global_Cs_StatusReport();
                continue;

            case 2:
                // Packet packet I/O Global Control status report
                Global_StatusReport();
                continue;

            case 3:
                // DRBG Global Control status report
                Global_DRBG_StatusReport();
                continue;

            default:
                Log_FormattedMessage("DA_GC: invalid input, try again\n");
                break;
        }
    }

    Log_FormattedMessage("DA_GC: status monitor exit\n");

    return;
}


int
main()
{
    int rc;

    // Initialize the EIP-197 Global Control Driver
    rc = Driver197_Global_Init();
    if (rc != 0)
    {
        Log_FormattedMessage(
                "DA_GC: Global Control Driver initialization failed\n");
        return -1;
    }

    da_gc_module_init();

    // Start the global status monitor
    da_gc_monitor();

    da_gc_module_exit();

    // Uninitialize the EIP-197 Global Control Driver
    Driver197_Global_Exit();

    return 0;
}

#else

// Linux kernel module entry point
module_init(da_gc_module_init);


// Linux kernel module exit point
module_exit(da_gc_module_exit);

#endif


/* end of file da_gc_eip197.c */
