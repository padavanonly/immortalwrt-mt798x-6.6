/* adapter_global_control_init.c
 *
 * Adapter module responsible for adapter global control initialization tasks.
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
*/

#include "adapter_global_control_init.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level Adapter configuration
#include "c_adapter_global.h"
#include "c_adapter_cs.h"

// Runtime Power Management Device Macros API
#include "rpm_device_macros.h"  // RPM_*

// Logging API
#include "log.h"            // LOG_*

// Driver Framework Device API
#include "device_mgmt.h"    // Device_Initialize, Device_UnInitialize
#include "device_rw.h"      // Device_Read32, Device_Write32

// Driver Framework C Library API
#include "clib.h"               // memcpy, ZEROINIT

// Driver Framework Basic Definitions API
#include "basic_defs.h"     // bool, true, false

#ifdef GLOBALCONTROL_BUILD
#include "shdevxs_init.h"  // SHDevXS_Global_init()
#endif


/*----------------------------------------------------------------------------
 * Local variables
 */

static bool Adapter_IsInitialized = false;

#ifdef ADAPTER_GLOBAL_BOARDCTRL_SUPPORT_ENABLE
static Device_Handle_t Adapter_Device_BOARDCTRL;
#endif


/*----------------------------------------------------------------------------
 * Forward declarations
 */

static int
AdapterLib_EIP197_Resume(void * p);

#ifdef ADAPTER_GLOBAL_RPM_EIP197_DEVICE_ID
static int
AdapterLib_EIP197_Suspend(void * p);
#endif // ADAPTER_GLOBAL_RPM_EIP197_DEVICE_ID


/*----------------------------------------------------------------------------
 * AdapterLib_EIP197_Resume
 *
 */
static int
AdapterLib_EIP197_Resume(void * p)
{
    IDENTIFIER_NOT_USED(p);

    // FPGA board specific functionality
#ifdef ADAPTER_GLOBAL_BOARDCTRL_SUPPORT_ENABLE
    {
#ifdef ADAPTER_GLOBAL_FPGA_HW_RESET_ENABLE
        LOG_INFO("%s: reset FPGA\n", __func__);
        // Perform HW Reset for the EIP-197 FPGA board
        Device_Write32(Adapter_Device_BOARDCTRL, 0x2000, 0);
        Device_Write32(Adapter_Device_BOARDCTRL, 0x2000, 0xFFFFFFFF);
        Device_Write32(Adapter_Device_BOARDCTRL, 0x2000, 0);
#endif // ADAPTER_GLOBAL_FPGA_HW_RESET_ENABLE
    }
#endif // ADAPTER_GLOBAL_BOARDCTRL_SUPPORT_ENABLE

    return 0; // success
}


#ifdef ADAPTER_GLOBAL_RPM_EIP197_DEVICE_ID
/*----------------------------------------------------------------------------
 * AdapterLib_EIP197_Suspend
 *
 */
static int
AdapterLib_EIP197_Suspend(void * p)
{
    IDENTIFIER_NOT_USED(p);

    // FPGA board specific functionality
#ifdef ADAPTER_GLOBAL_BOARDCTRL_SUPPORT_ENABLE
    {
#ifdef ADAPTER_GLOBAL_FPGA_HW_RESET_ENABLE
        LOG_INFO("%s: reset FPGA\n", __func__);
        // Perform HW Reset for the EIP-197 FPGA board
        Device_Write32(Adapter_Device_BOARDCTRL, 0x2000, 0);
        Device_Write32(Adapter_Device_BOARDCTRL, 0x2000, 0xFFFFFFFF);
        Device_Write32(Adapter_Device_BOARDCTRL, 0x2000, 0);
#endif // ADAPTER_GLOBAL_FPGA_HW_RESET_ENABLE
    }
#endif // ADAPTER_GLOBAL_BOARDCTRL_SUPPORT_ENABLE

    return 0; // success
}
#endif // ADAPTER_GLOBAL_RPM_EIP197_DEVICE_ID


/*----------------------------------------------------------------------------
 * Adapter_Global_Control_Init
 *
 * Return Value
 *     true   Success
 *     false  Failure (fatal!)
 */
bool
Adapter_Global_Control_Init(void)
{
    int nIRQ = -1;

    if (Adapter_IsInitialized != false)
    {
        LOG_WARN("Adapter_Global_Control_Init: Already initialized\n");
        return true;
    }

    // trigger first-time initialization of the adapter
    if (Device_Initialize(&nIRQ) < 0)
        return false;

#ifdef GLOBALCONTROL_BUILD
    LOG_INFO("\n SHDevXS_Global_SetPrivileged \n");
    if (SHDevXS_Global_SetPrivileged() != 0)
    {
        LOG_CRIT("%s: SHDevXS_Global_SetPrivileged() failed\n", __func__);
        Device_UnInitialize();
        return false;
    }
#endif

#ifdef ADAPTER_GLOBAL_BOARDCTRL_SUPPORT_ENABLE
    Adapter_Device_BOARDCTRL = Device_Find("BOARD_CTRL");
    if (Adapter_Device_BOARDCTRL == NULL)
    {
        LOG_CRIT("Adapter_Global_Control_Init: "
                 "Failed to locate BOARD_CTRL\n");
        Device_UnInitialize();
        return false;
    }
#endif // ADAPTER_GLOBAL_BOARDCTRL_SUPPORT_ENABLE

    if (RPM_DEVICE_INIT_START_MACRO(ADAPTER_GLOBAL_RPM_EIP197_DEVICE_ID,
                                    AdapterLib_EIP197_Suspend,
                                    AdapterLib_EIP197_Resume)
                                                        != RPM_SUCCESS)
    {
        Device_UnInitialize();
        return false;
    }

    AdapterLib_EIP197_Resume(NULL);

    (void)RPM_DEVICE_INIT_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP197_DEVICE_ID);

    Adapter_IsInitialized = true;

    return true;    // success
}


/*----------------------------------------------------------------------------
 * Adapter_Global_Control_UnInit
 */
void
Adapter_Global_Control_UnInit(void)
{
    if (!Adapter_IsInitialized)
    {
        LOG_WARN("Adapter_Global_Control_UnInit: Adapter is uninitialized\n");
        return;
    }

    Adapter_IsInitialized = false;

    (void)RPM_DEVICE_UNINIT_START_MACRO(ADAPTER_GLOBAL_RPM_EIP197_DEVICE_ID, false);
    (void)RPM_DEVICE_UNINIT_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP197_DEVICE_ID);

    Device_UnInitialize();
}


/*----------------------------------------------------------------------------
 * Adapter_Global_Control_Report_Build_Params
 */
void
Adapter_Global_Control_Report_Build_Params(void)
{
#ifdef LOG_INFO_ENABLED
    int dummy = 0;

    // This function is dependent on config file cs_adapter.h.
    // Please update this when Config file for Adapter is changed.
    Log_FormattedMessage("Adapter Global Control build configuration:\n");

#define REPORT_SET(_X) \
    Log_FormattedMessage("\t" #_X "\n")

#define REPORT_STR(_X) \
    Log_FormattedMessage("\t" #_X ": %s\n", _X)

#define REPORT_INT(_X) \
    dummy = _X; Log_FormattedMessage("\t" #_X ": %d\n", _X)

#define REPORT_HEX32(_X) \
    dummy = _X; Log_FormattedMessage("\t" #_X ": 0x%08X\n", _X)

#define REPORT_EQ(_X, _Y) \
    dummy = (_X + _Y); Log_FormattedMessage("\t" #_X " == " #_Y "\n")

#define REPORT_EXPL(_X, _Y) \
    Log_FormattedMessage("\t" #_X _Y "\n")

#ifdef ADAPTER_64BIT_HOST
    REPORT_EXPL(ADAPTER_64BIT_HOST,
                " is SET => addresses are 64-bit");
#else
    REPORT_EXPL(ADAPTER_64BIT_HOST,
                " is NOT set => addresses are 32-bit");
#endif

    // Global interrupts
#ifdef ADAPTER_GLOBAL_INTERRUPTS_TRACEFILTER
    REPORT_INT(ADAPTER_GLOBAL_INTERRUPTS_TRACEFILTER);
#endif

    // Adapter Global Classification Control
#ifdef ADAPTER_CS_TIMER_PRESCALER
    REPORT_INT(ADAPTER_CS_TIMER_PRESCALER);
#endif

#ifdef ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE
    REPORT_INT(ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE);
#endif

#ifdef ADAPTER_CS_GLOBAL_DEVICE_NAME
    REPORT_STR(ADAPTER_CS_GLOBAL_DEVICE_NAME);
#endif

#ifdef ADAPTER_GLOBAL_BOARDCTRL_SUPPORT_ENABLE
    REPORT_SET(ADAPTER_GLOBAL_BOARDCTRL_SUPPORT_ENABLE);
#endif

    // Log
    Log_FormattedMessage("Logging:\n");

#if (LOG_SEVERITY_MAX == LOG_SEVERITY_INFO)
    REPORT_EQ(LOG_SEVERITY_MAX, LOG_SEVERITY_INFO);
#elif (LOG_SEVERITY_MAX == LOG_SEVERITY_WARNING)
    REPORT_EQ(LOG_SEVERITY_MAX, LOG_SEVERITY_WARNING);
#elif (LOG_SEVERITY_MAX == LOG_SEVERITY_CRITICAL)
    REPORT_EQ(LOG_SEVERITY_MAX, LOG_SEVERITY_CRITICAL);
#else
    REPORT_EXPL(LOG_SEVERITY_MAX, " - Unknown (not info/warn/crit)");
#endif

    IDENTIFIER_NOT_USED(dummy);

    // Adapter other
    Log_FormattedMessage("Other:\n");
    REPORT_STR(ADAPTER_GLOBAL_DRIVER_NAME);
    REPORT_STR(ADAPTER_GLOBAL_LICENSE);
    REPORT_STR(RPM_DEVICE_CAPABILITIES_STR_MACRO);

#endif //LOG_INFO_ENABLED
}


/* end of file adapter_global_control_init.c */
