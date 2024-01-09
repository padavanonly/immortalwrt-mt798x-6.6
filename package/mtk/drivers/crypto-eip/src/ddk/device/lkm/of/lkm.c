/* lkm.c
 *
 * Linux Kernel Module implementation for Platform device drivers
 *
 */

/*****************************************************************************
* Copyright (c) 2016-2020 by Rambus, Inc. and/or its subsidiaries.
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

// Linux Kernel Module interface
#include "lkm.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_lkm.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"             // uint32_t, NULL, inline, bool,
                                    // IDENTIFIER_NOT_USED

// Driver Framework C Run-Time Library API
#include "clib.h"                   // memcmp

// Logging API
#undef LOG_SEVERITY_MAX
#define LOG_SEVERITY_MAX  LKM_LOG_SEVERITY
#include "log.h"                    // LOG_*

// Linux Kernel API
#include <linux/types.h>            // phys_addr_t, resource_size_t
#include <linux/version.h>          // LINUX_VERSION_CODE, KERNEL_VERSION
#include <linux/device.h>           // struct device
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>  // platform_*,
#include <linux/of_platform.h>      // of_*,
#include <asm/io.h>                 // ioremap, iounmap
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/moduleparam.h>
#include <linux/clk.h>              // clk_*


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


// LKM PCI implementation administration data
typedef struct
{
    // declarations native to Linux kernel
    struct platform_device * Platform_Device_p;

    // virtual address returned by ioremap()
    uint32_t __iomem * MappedBaseAddr_p;

    // physical address passed to ioremap()
    phys_addr_t PhysBaseAddr;

    // Platform device driver data
    struct platform_driver Platform_Driver;

    // Device resource (IO space) identifier
    int ResId;

    // Device resource size in bytes
    resource_size_t ResByteCount;

    // Device virtual IRQ number
    int VirtIrqNr [LKM_PLATFORM_IRQ_COUNT];

    bool fRetainMap;

    // Initialization flag, true - initialized, otherwise - false
    bool fInitialized;

} LKM_Admin_t;


/*----------------------------------------------------------------------------
 * Local variables
 */
static struct clk *gateclk;

static const struct of_device_id LKM_DT_IDTable[] =
{
    { .compatible = LKM_PLATFORM_DEVICE_NAME, },
    { },
};

// LKM administration data
static LKM_Admin_t LKM_Admin;


/*----------------------------------------------------------------------------
 * LKM_Probe
 */
static int
LKM_Probe(
        struct platform_device * Platform_Device_p)
{
    int i;
    LKM_Admin_t * p = &LKM_Admin;
    struct resource * rsc_p = NULL;

    LOG_INFO(LKM_LOG_PREFIX "%s: entered\n", __func__);

    if (Platform_Device_p == NULL)
    {
        LOG_CRIT(LKM_LOG_PREFIX
                 "%s: failed, missing platform device\n",
                 __func__);
        return -ENODEV;
    }

    if (of_find_compatible_node(NULL, NULL, LKM_PLATFORM_DEVICE_NAME))
    {
        LOG_INFO(LKM_LOG_PREFIX
                 "%s: found requested device %s\n",
                 __func__,
                 LKM_PLATFORM_DEVICE_NAME);
    }
    else
    {
        LOG_CRIT(LKM_LOG_PREFIX
                 "%s: device %s not supported\n",
                 __func__,
                 LKM_PLATFORM_DEVICE_NAME);
        return -ENODEV;
    }

    // remember the device reference
    p->Platform_Device_p = Platform_Device_p;

    if (p->fRetainMap)
    {
        // get platform device physical address
        // Exported under GPL
        rsc_p = platform_get_resource(Platform_Device_p,
                                      IORESOURCE_MEM,
                                      p->ResId);

        // device tree specific for this OF device
        // only 32-bit physical addresses are supported
        if(rsc_p)
        {
            LOG_INFO(LKM_LOG_PREFIX
                     "%s: mem start=%08x, end=%08x, flags=%08x\n",
                     __func__,
                     (unsigned int)rsc_p->start,
                     (unsigned int)rsc_p->end,
                     (unsigned int)rsc_p->flags);
        }
        else
        {
            LOG_CRIT(LKM_LOG_PREFIX
                     "%s: memory resource id %d not found\n",
                     __func__,
                     p->ResId);
            return -ENODEV;
        }

        p->PhysBaseAddr      = rsc_p->start;
        p->ResByteCount      = rsc_p->end - rsc_p->start + 1;

        // now map the chip into kernel memory
        // so we can access the EIP static resources

        // note: ioremap is uncached by default
        p->MappedBaseAddr_p = ioremap(p->PhysBaseAddr, p->ResByteCount);
        if (p->MappedBaseAddr_p == NULL)
        {
            LOG_CRIT(LKM_LOG_PREFIX
                     "%s: failed to ioremap platform driver %s, "
                     "resource id %d, phys addr 0x%p, size %ul\n",
                     __func__,
                     p->Platform_Driver.driver.name,
                     p->ResId,
                     (void*)p->PhysBaseAddr,
                     (unsigned int)p->ResByteCount);
            return -ENODEV;
        }

        LOG_INFO(LKM_LOG_PREFIX
                 "%s: Mapped platform driver %s addr %p, phys addr 0x%p, "
                 "sizeof(resource_size_t)=%d\n resource id=%d, size=%ul\n",
                 __func__,
                 p->Platform_Driver.driver.name,
                 p->MappedBaseAddr_p,
                 (void*)p->PhysBaseAddr,
                 (int)sizeof(resource_size_t),
                 p->ResId,
                 (unsigned int)p->ResByteCount);
    }

    // Optional device clock control functionality,
    // only required when the device clock needs to be enabled via the kernel
    gateclk = clk_get(&Platform_Device_p->dev, NULL);
    if (!IS_ERR(gateclk))
    {
        // Exported under GPL
        clk_prepare_enable(gateclk);
        LOG_INFO("%s: clk_prepare_enable() successful\n", __func__);
    }
    else
    {
        // Not all devices support it
        LOG_INFO("%s: clk_get() could not obtain clock\n", __func__);
    }

    for (i = 0; i < LKM_PLATFORM_IRQ_COUNT; i++)
    {
        int IrqNr, IrqIndex;

        IrqIndex = LKM_PLATFORM_IRQ_COUNT > 1 ? i : LKM_PLATFORM_IRQ_IDX;

        // Exported under GPL
        IrqNr = platform_get_irq(Platform_Device_p, IrqIndex);
        if (IrqNr < 0)
        {
            LOG_INFO(LKM_LOG_PREFIX "%s: failed to get IRQ for index %d\n",
                     __func__,
                     IrqIndex);
            return -ENODEV;
        }
        else
            p->VirtIrqNr[i] = IrqNr;
    }

    LOG_INFO(LKM_LOG_PREFIX "%s: left\n", __func__);

    // return 0 to indicate "we decided to take ownership"
    return 0;
}


/*----------------------------------------------------------------------------
 * LKM_Remove
 */
static int
LKM_Remove(
        struct platform_device * Platform_Device_p)
{
    LKM_Admin_t * p = &LKM_Admin;

    LOG_INFO(LKM_LOG_PREFIX "%s: entered\n", __func__);

    if (p->Platform_Device_p != Platform_Device_p)
    {
        LOG_CRIT(LKM_LOG_PREFIX
                 "%s: failed, missing or wrong platform device\n",
                 __func__);
        return -ENODEV;
    }

    LOG_INFO(LKM_LOG_PREFIX
             "%s: mapped base addr=%p\n", __func__, p->MappedBaseAddr_p);

    // Optional device clock control functionality
    if (!IS_ERR(gateclk))
    {
        // Exported under GPL
        clk_disable_unprepare(gateclk);
        clk_put(gateclk);
    }

    if (p->MappedBaseAddr_p && p->fRetainMap)
    {
        iounmap(p->MappedBaseAddr_p);
        p->MappedBaseAddr_p = NULL;
    }

    LOG_INFO(LKM_LOG_PREFIX "%s: left\n", __func__);

    return 0;
}


/*-----------------------------------------------------------------------------
 * LKM_Init
 */
int
LKM_Init(
        LKM_Init_t * const InitData_p)
{
    int Status;
    LKM_Admin_t * p = &LKM_Admin;

    LOG_INFO(LKM_LOG_PREFIX "%s: entered\n", __func__);

    // Check input parameters
    if(InitData_p == NULL)
    {
        LOG_CRIT(LKM_LOG_PREFIX "%s: failed, missing init data\n", __func__);
        return -1;
    }

    if (p->fInitialized)
    {
        LOG_CRIT(LKM_LOG_PREFIX "%s: failed, already initialized\n", __func__);
        return -2;
    }

    // Fill in PCI device driver data
    p->fRetainMap                    = InitData_p->fRetainMap;
    p->ResId                         = InitData_p->ResId; // not used

    p->Platform_Driver.probe         = LKM_Probe;
    p->Platform_Driver.remove        = LKM_Remove;

    p->Platform_Driver.driver.name            = InitData_p->DriverName_p;
    p->Platform_Driver.driver.owner           = THIS_MODULE;
    p->Platform_Driver.driver.pm              = InitData_p->PM_p;
    p->Platform_Driver.driver.of_match_table  = LKM_DT_IDTable;

    // Exported under GPL
    Status = platform_driver_register(&p->Platform_Driver);
    if (Status < 0)
    {
        LOG_CRIT(LKM_LOG_PREFIX
                 "%s: failed to register platform device driver\n",
                 __func__);
        return -3;
    }

    if (p->Platform_Device_p == NULL)
    {
        LOG_CRIT(LKM_LOG_PREFIX "%s: failed, no device detected\n", __func__);
        platform_driver_unregister(&p->Platform_Driver);
        return -4;
    }

    // if provided, CustomInitData_p points to an "int"
    // we return a pointer to the arrays of irq numbers
    InitData_p->CustomInitData_p = p->VirtIrqNr;

    p->fInitialized = true;

    LOG_INFO(LKM_LOG_PREFIX "%s: left\n", __func__);

    return 0; // success
}


/*-----------------------------------------------------------------------------
 * LKM_Uninit
 */
void
LKM_Uninit(void)
{
    LKM_Admin_t * p = &LKM_Admin;

    LOG_INFO(LKM_LOG_PREFIX "%s: entered\n", __func__);

    if (!p->fInitialized)
    {
        LOG_CRIT(LKM_LOG_PREFIX "%s: failed, not initialized yet\n", __func__);
        return;
    }

    LOG_INFO(LKM_LOG_PREFIX
             "%s: calling platform_driver_unregister\n",
             __func__);

    platform_driver_unregister(&p->Platform_Driver);

    ZEROINIT(LKM_Admin); //p->fInitialized = false;

    LOG_INFO(LKM_LOG_PREFIX "%s: left\n", __func__);
}


/*-----------------------------------------------------------------------------
 * LKM_DeviceGeneric_Get
 */
void *
LKM_DeviceGeneric_Get(void)
{
    LKM_Admin_t * p = &LKM_Admin;

    return &p->Platform_Device_p->dev;
}


/*-----------------------------------------------------------------------------
 * LKM_DeviceSpecific_Get
 */
void *
LKM_DeviceSpecific_Get(void)
{
    LKM_Admin_t * p = &LKM_Admin;

    return p->Platform_Device_p;
}


/*-----------------------------------------------------------------------------
 * LKM_PhysBaseAddr_Get
 */
void *
LKM_PhysBaseAddr_Get(void)
{
    LKM_Admin_t * p = &LKM_Admin;

    return (void*)p->PhysBaseAddr;
}


/*-----------------------------------------------------------------------------
 * LKM_MappedBaseAddr_Get
 */
void __iomem *
LKM_MappedBaseAddr_Get(void)
{
    LKM_Admin_t * p = &LKM_Admin;

    return p->MappedBaseAddr_p;
}


/* end of file lkm.c */
