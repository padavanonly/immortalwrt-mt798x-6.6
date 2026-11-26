/* shdevxs_irq.c
 *
 * Shared Device for IRQ access.
 * Linux user space
 */

/*****************************************************************************
* Copyright (c) 2012-2022 by Rambus, Inc. and/or its subsidiaries.
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

#include "shdevxs_irq.h"
#include "shdevxs_kernel_internal.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default SHDevXS configuration
#include "c_shdevxs.h"          // SHDEVXS_*

// Default UMDevXS configuration
#include "c_umdevxs.h"          // UMDEVXS_*

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // IDENTIFIER_NOT_USED

#include "log.h"

// Linux kernel API
#include <linux/module.h>

#include "adapter_interrupts.h"

#ifndef UMDEVXS_REMOVE_PCI
// Linux Kernel API
#include <linux/pci.h>              // pci_*
#include "umdevxs_pcidev.h"
#endif

#ifndef UMDEVXS_REMOVE_DEVICE_OF
// Linux Kernel API
#include <linux/of_platform.h>      // of_*,
#include "umdevxs_ofdev.h"
#endif

#include <linux/semaphore.h>
#include <linux/jiffies.h>

// Runtime Power Management Device Macros API
#include "rpm_device_macros.h"  // RPM_*

#include <linux/platform_device.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */
EXPORT_SYMBOL(SHDevXS_IRQ_Init);
EXPORT_SYMBOL(SHDevXS_IRQ_UnInit);
EXPORT_SYMBOL(SHDevXS_IRQ_Enable);
EXPORT_SYMBOL(SHDevXS_IRQ_Disable);
EXPORT_SYMBOL(SHDevXS_IRQ_Clear);
EXPORT_SYMBOL(SHDevXS_IRQ_ClearAndEnable);
EXPORT_SYMBOL(SHDevXS_IRQ_SetHandler);

#define SHDEVXS_KERNEL_APPID ((void*)-1)


/*----------------------------------------------------------------------------
 * Local variables
 */
// signalling int handler -> app thread
static struct semaphore SHDevXS_IRQ_sem[SHDEVXS_IRQ_COUNT];

/* Each interrupt has its own semaphore to wake up a thread in user space. */
/* Each IRQ has its owner */
static void * SHDevXS_IRQ_Owner[SHDEVXS_IRQ_COUNT];

static bool IRQ_Initialized = false;

static void *IRQ_Lock;
static unsigned long Lock_Flags;

// Cached values for enabled IRQ's for RPM device resume operation
static bool SHDevXS_IRQ_Enabled[SHDEVXS_IRQ_COUNT];


#ifdef SHDEVXS_IRQ_RPM_DEVICE_ID
/*----------------------------------------------------------------------------
 * SHDevXS_IRQ_Resume
 *
 */
static int
SHDevXS_IRQ_Resume(void * p)
{
    int rc;
    unsigned int i;

    IDENTIFIER_NOT_USED(p);

    // Restore interrupts
    rc = Adapter_Interrupts_Resume();
    if (rc != 0)
        return rc; // error

    // ... and re-enable
    for (i = 0; i < SHDEVXS_IRQ_COUNT; i++)
        if (SHDevXS_IRQ_Enabled[i])
            Adapter_Interrupt_Enable(i, 0);

    return 0; //success
}
#endif


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_IRQ_Initialize
 */
int
SHDevXS_Internal_IRQ_Initialize(void)
{
    int nIRQ = -1;

    if (IRQ_Initialized)
    {
        LOG_CRIT("%s: already initialized\n", __func__);
        return -1;
    }

#ifndef UMDEVXS_REMOVE_PCI
    {
        struct pci_dev * Device_p;
        void __iomem * p;

        UMDevXS_PCIDev_Get(0, &Device_p, &p);
        if (Device_p == NULL || p == NULL)
        {
            LOG_CRIT("%s: Failed to obtain PCI device\n", __func__);
            return -1;
        }

        nIRQ = Device_p->irq;
    }
#endif // !UMDEVXS_REMOVE_PCI

#ifndef UMDEVXS_REMOVE_DEVICE_OF
    {
        struct platform_device * Device_p;
        void __iomem * p;

        UMDevXS_OFDev_GetDevice(0, &Device_p, &p);
        if (Device_p == NULL || p == NULL)
        {
            LOG_CRIT("%s: Failed to obtain OF device\n", __func__);
            return -1;
        }

        // Exported under GPL
        nIRQ = platform_get_irq(Device_p, 0);
    }
#endif // !UMDEVXS_REMOVE_DEVICE_OF

    IRQ_Lock = SHDevXS_Internal_Lock_Alloc();
    if (!IRQ_Lock)
    {
        LOG_CRIT("%s: lock alloc failed, IRQ = %d\n", __func__, nIRQ);
        return -1;
    }

    {
        unsigned int i;
        int res;

        for (i=0; i<SHDEVXS_IRQ_COUNT; i++)
        {
            SHDevXS_IRQ_Enabled[i] = false;
        }

        if (RPM_DEVICE_INIT_START_MACRO(SHDEVXS_IRQ_RPM_DEVICE_ID,
                                        0, // Suspend callback not used
                                        SHDevXS_IRQ_Resume) != RPM_SUCCESS)
            goto error_exit;

        res = Adapter_Interrupts_Init(nIRQ);

        (void)RPM_DEVICE_INIT_STOP_MACRO(SHDEVXS_IRQ_RPM_DEVICE_ID);

        if (res < 0)
            goto error_exit;
    }

    LOG_CRIT("%s: IRQ init successful, IRQ = %d\n", __func__, nIRQ);

    IRQ_Initialized = true;
    return 0; // Success

error_exit:
    LOG_CRIT("%s: IRQ init failed, IRQ = %d\n", __func__, nIRQ);

    SHDevXS_Internal_Lock_Free(IRQ_Lock);
    IRQ_Lock = NULL;

    return -1; // Error
}


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_IRQ_UnInitialize
 */
void
SHDevXS_Internal_IRQ_UnInitialize(void)
{
    int nIRQ = -1;
    int i;

    if (!IRQ_Initialized)
    {
        LOG_CRIT("SHDevXS_Internal_IRQ_UnInitialize: not initialized\n");
        return;
    }

    for (i=0; i<SHDEVXS_IRQ_COUNT; i++)
    {
        if (SHDevXS_IRQ_Owner[i] != NULL)
            SHDevXS_Internal_IRQ_SetHandler(SHDevXS_IRQ_Owner[i], i, NULL);
    }

    IRQ_Initialized = false;

    SHDevXS_Internal_Lock_Free(IRQ_Lock);
    IRQ_Lock = NULL;

#ifndef UMDEVXS_REMOVE_PCI
    {
        struct pci_dev * Device_p;
        void __iomem * p;

        UMDevXS_PCIDev_Get(0, &Device_p, &p);
        if (Device_p == NULL || p == NULL)
        {
            LOG_CRIT("SHDevXS_Internal_IRQ_UnInitialize: "
                     "Failed to obtain PCI device\n");
            return;
        }

        nIRQ = Device_p->irq;
    }
#endif

#ifndef UMDEVXS_REMOVE_DEVICE_OF
    {
        struct platform_device * Device_p;
        void __iomem * p;

        UMDevXS_OFDev_GetDevice(0, &Device_p, &p);
        if (Device_p == NULL || p == NULL)
        {
            LOG_CRIT("SHDevXS_Internal_IRQ_UnInitialize: "
                     "Failed to obtain OF device\n");
            return;
        }

        // Exported under GPL
        nIRQ = platform_get_irq(Device_p, 0);
    }
#endif // !UMDEVXS_REMOVE_DEVICE_OF

    if (RPM_DEVICE_UNINIT_START_MACRO(SHDEVXS_IRQ_RPM_DEVICE_ID,
                                      true) == RPM_SUCCESS)
    {
        Adapter_Interrupts_UnInit(nIRQ);
        (void)RPM_DEVICE_UNINIT_STOP_MACRO(SHDEVXS_IRQ_RPM_DEVICE_ID);
    }
}


/*----------------------------------------------------------------------------
 * SHDevXS_IRQ_Init
 */
int
SHDevXS_IRQ_Init(void)
{
    if (!IRQ_Initialized)
    {
        LOG_CRIT("SHDevXS_IRQ_Init: not initialized\n");
        return -1;
    }
    return 0;
}


/*----------------------------------------------------------------------------
 * SHDevXS_IRQ_UnInit
 */
int
SHDevXS_IRQ_UnInit(void)
{
    if (!IRQ_Initialized)
    {
        LOG_CRIT("SHDevXS_IRQ_UnInit: not initialized\n");
        return -1;
    }
    return 0;
}


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_IRQ_Enable
 */
int
SHDevXS_Internal_IRQ_Enable(
        void *AppID,
        const int nIRQ,
        const unsigned int Flags)
{
    if (!IRQ_Initialized)
    {
        LOG_CRIT("SHDevXS_IRQ_Enable: not initialized\n");
        return -1;
    }

    if (nIRQ < 0 || nIRQ >= SHDEVXS_IRQ_COUNT ||
        SHDevXS_IRQ_Owner[nIRQ] != AppID)
    {
        LOG_CRIT("SHDevXS_IRQ_Enable: nIRQ not owned: %d\n", nIRQ);
        return -1;
    }

    if (RPM_DEVICE_IO_START_MACRO(SHDEVXS_IRQ_RPM_DEVICE_ID, RPM_FLAG_SYNC) !=
                                                                   RPM_SUCCESS)
        return -1;

    LOG_INFO("SHDevXS_IRQ Enable IRQ %d %x\n",nIRQ,Flags);
    Adapter_Interrupt_Enable(nIRQ, Flags);

    SHDevXS_IRQ_Enabled[nIRQ] = true;

    (void)RPM_DEVICE_IO_STOP_MACRO(SHDEVXS_IRQ_RPM_DEVICE_ID, RPM_FLAG_ASYNC);

    return 0;
}


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_IRQ_Disable
 */
int
SHDevXS_Internal_IRQ_Disable(
        void *AppID,
        const int nIRQ,
        const unsigned int Flags)
{
    if (!IRQ_Initialized)
    {
        LOG_CRIT("SHDevXS_IRQ_Disable: not initialized\n");
        return -1;
    }

    if (nIRQ < 0 || nIRQ >= SHDEVXS_IRQ_COUNT ||
        SHDevXS_IRQ_Owner[nIRQ] != AppID)
    {
        LOG_CRIT("SHDevXS_IRQ_Disable: nIRQ not owned: %d\n", nIRQ);
        return -1;
    }

    if (RPM_DEVICE_IO_START_MACRO(SHDEVXS_IRQ_RPM_DEVICE_ID, RPM_FLAG_SYNC) !=
                                                                   RPM_SUCCESS)
        return -1;

    LOG_INFO("SHDevXS_IRQ Disable IRQ %d %x\n", nIRQ, Flags);
    Adapter_Interrupt_Disable(nIRQ, Flags);

    SHDevXS_IRQ_Enabled[nIRQ] = false;

    (void)RPM_DEVICE_IO_STOP_MACRO(SHDEVXS_IRQ_RPM_DEVICE_ID, RPM_FLAG_ASYNC);

    return 0;
}


#ifdef SHDEVXS_ENABLE_IRQ_CLEAR
/*----------------------------------------------------------------------------
 * SHDevXS_Internal_IRQ_Clear
 */
int
SHDevXS_Internal_IRQ_Clear(
        void *AppID,
        const int nIRQ,
        const unsigned int Flags)
{
    if (!IRQ_Initialized)
    {
        LOG_CRIT("SHDevXS_IRQ_Clear: not initialized\n");
        return -1;
    }
    if (nIRQ < 0 || nIRQ >= SHDEVXS_IRQ_COUNT ||
        SHDevXS_IRQ_Owner[nIRQ] != AppID)
    {
        LOG_CRIT("SHDevXS_IRQ_Clear: "
                 "nIRQ not owned: %d\n",nIRQ);
        return -1;
    }
    LOG_INFO("SHDevXS_IRQ Clear IRQ %d %x\n",nIRQ,Flags);
    Adapter_Interrupt_Clear(nIRQ, Flags);
    return 0;
}


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_IRQ_ClearAndEnable
 */
int
SHDevXS_Internal_IRQ_ClearAndEnable(
        void *AppID,
        const int nIRQ,
        const unsigned int Flags)
{
    if (!IRQ_Initialized)
    {
        LOG_CRIT("SHDevXS_IRQ_ClearAndEnable: not initialized\n");
        return -1;
    }
    if (nIRQ < 0 || nIRQ >= SHDEVXS_IRQ_COUNT ||
        SHDevXS_IRQ_Owner[nIRQ] != AppID)
    {
        LOG_CRIT("SHDevXS_IRQ_ClearAndEnable: "
                 "nIRQ not owned: %d\n",nIRQ);
        return -1;
    }
    LOG_INFO("SHDevXS_IRQ ClearAndEnable IRQ %d %x\n",nIRQ,Flags);
    Adapter_Interrupt_ClearAndEnable(nIRQ, Flags);
    return 0;
}
#endif /* SHDEVXS_ENABLE_IRQ_CLEAR */

/*----------------------------------------------------------------------------
 * SHDevXS_Internal_IRQ_SetHandler
 *
 */
int
SHDevXS_Internal_IRQ_SetHandler(
        void * AppID,
        const int nIRQ,
        SHDevXS_InterruptHandler_t HandlerFunction)
{
    if (!IRQ_Initialized)
    {
        LOG_CRIT("SHDevXS_IRQ_SetHandler: not initialized\n");
        return -1;
    }

    SHDevXS_Internal_Lock_Acquire(IRQ_Lock, &Lock_Flags);

    if (nIRQ < 0 || nIRQ >= SHDEVXS_IRQ_COUNT ||
        (SHDevXS_IRQ_Owner[nIRQ] != AppID && SHDevXS_IRQ_Owner[nIRQ] != NULL))
    {
        LOG_CRIT("SHDevXS_IRQ_SetHandle: "
                 "nIRQ not owned: %d\n",nIRQ);
        SHDevXS_Internal_Lock_Release(IRQ_Lock, &Lock_Flags);
        return -1;
    }

    if (HandlerFunction == NULL)
        SHDevXS_IRQ_Owner[nIRQ] = NULL;
    else
        SHDevXS_IRQ_Owner[nIRQ] = AppID;

    if (AppID != SHDEVXS_KERNEL_APPID)
    {
        sema_init(SHDevXS_IRQ_sem + nIRQ, 0);
    }

    LOG_INFO("SHDevXS_IRQ Set Handle for IRQ %d\n",nIRQ);

    Adapter_Interrupt_SetHandler(nIRQ, HandlerFunction);

    SHDevXS_Internal_Lock_Release (IRQ_Lock, &Lock_Flags);

    return 0;
}


/*----------------------------------------------------------------------------
 * SHDevXS_IRQ_Cleanup
 *
 * Un-initialize all IRQ handling specific to an application.
 */
int
SHDevXS_Internal_IRQ_Cleanup(
        void * AppID)
{
    unsigned i;

    if (!IRQ_Initialized)
    {
        LOG_CRIT("SHDevXS_IRQ_Cleanup: not initialized\n");
        return -1;
    }
    LOG_INFO ("SHDevXS_IRQ_Cleanup\n");
    for (i=0; i<SHDEVXS_IRQ_COUNT; i++)
    {
        if (SHDevXS_IRQ_Owner[i] == AppID)
            SHDevXS_Internal_IRQ_SetHandler(AppID, i, NULL);
    }
    return 0;
}


/*----------------------------------------------------------------------------
 * SHDevXS_IRQ_Enable
 */
int
SHDevXS_IRQ_Enable(
        const int nIRQ,
        const unsigned int Flags)
{
    return SHDevXS_Internal_IRQ_Enable(SHDEVXS_KERNEL_APPID, nIRQ, Flags);
}


/*----------------------------------------------------------------------------
 * SHDevXS_IRQ_Disable
 */
int
SHDevXS_IRQ_Disable(
        const int nIRQ,
        const unsigned int Flags)
{
    return SHDevXS_Internal_IRQ_Disable(SHDEVXS_KERNEL_APPID, nIRQ, Flags);
}


/*----------------------------------------------------------------------------
 * SHDevXS_IRQ_Clear
 */
int
SHDevXS_IRQ_Clear(
        const int nIRQ,
        const unsigned int Flags)
{
#ifdef SHDEVXS_ENABLE_IRQ_CLEAR
    return SHDevXS_Internal_IRQ_Clear(SHDEVXS_KERNEL_APPID, nIRQ, Flags);
#else
    return -1;
#endif
}


/*----------------------------------------------------------------------------
 * SHDevXS_IRQ_Disable
 */
int
SHDevXS_IRQ_ClearAndEnable(
        const int nIRQ,
        const unsigned int Flags)
{
#ifdef SHDEVXS_ENABLE_IRQ_CLEAR
    return SHDevXS_Internal_IRQ_ClearAndEnable(SHDEVXS_KERNEL_APPID, nIRQ, Flags);
#else
    return -1;
#endif
}


/*----------------------------------------------------------------------------
 * SHDevXS_IRQ_SetHandler
 */
int
SHDevXS_IRQ_SetHandler(
        const int nIRQ,
        SHDevXS_InterruptHandler_t HandlerFunction)
{
    return SHDevXS_Internal_IRQ_SetHandler(SHDEVXS_KERNEL_APPID,
                                           nIRQ,
                                           HandlerFunction);
}


/*----------------------------------------------------------------------------
 * SHDevXSLib_Internal_IRQHandler
 *
 * Interrupt handler (in kernel) that is called on behalf of user processes.
 *
 * Increment the semaphore belonging to that interrupt to wake up any threads
 * waiting for it.
 */
void
SHDevXS_Internal_IRQHandler(
        const int nIRQ,
        const unsigned int flags)
{
    IDENTIFIER_NOT_USED(flags);
    if (!IRQ_Initialized)
    {
        LOG_CRIT("SHDevXS_IRQHandler: not initialized\n");
        return;
    }

    if (nIRQ < 0 || nIRQ >= SHDEVXS_IRQ_COUNT)
    {
        LOG_CRIT("SHDevXSLib_Internal_IRQHandler nIRQ out of range: %d\n",
                 nIRQ);
    }
    else
    {
        // increase the semaphore
        up(SHDevXS_IRQ_sem + nIRQ);
    }
}


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_WaitWithTimeout
 *
 */
int
SHDevXS_Internal_WaitWithTimeout(
        void * AppID,
        int nIRQ,
        unsigned int timeout)
{
    int res;
    unsigned long l_jiffies = msecs_to_jiffies(timeout);

    if (!IRQ_Initialized)
    {
        LOG_CRIT("SHDevXS_Internal_WaitWithTImeout: not initialized\n");
        return -1;
    }

    LOG_INFO("SHDevXSLib_Internal_WaitWithTimeout\n");

    if (nIRQ < 0 || nIRQ >= SHDEVXS_IRQ_COUNT ||
        AppID != SHDevXS_IRQ_Owner[nIRQ])
    {
        LOG_CRIT("SHDevXS_Internal_WaitWithTimeout: nIRQ not owned: %d\n",
                 nIRQ);
    }

    res = SHDevXS_IRQ_sem[nIRQ].count;
    if (res > 1)
    {
        LOG_WARN(
            "SHDevXS_Internal_WaitWithTimeout: IRQ=%d, sem.count = %d\n",
            res,
            nIRQ);
    }

    // wait on the semaphore, with timeout
    res = down_timeout(SHDevXS_IRQ_sem + nIRQ, (long)l_jiffies);

    return res;
}


/* end of file shdevxs_irq.c */
