/* umdevxs_interrupt.c
 *
 * Interrupt support for the Linux UMDevXS driver.
 */

/*****************************************************************************
* Copyright (c) 2009-2020 by Rambus, Inc. and/or its subsidiaries.
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

#include "umdevxs_internal.h"

#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
#include "umdevxs_interrupt.h"
#endif


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_umdevxs.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"             // uint8_t, IDENTIFIER_NOT_USED

// Linux Kernel API
#include <linux/delay.h>            // msleep

#ifndef UMDEVXS_REMOVE_INTERRUPT
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>        // mutex_*
#endif /* UMDEVXS_REMOVE_INTERRUPT */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */

#ifndef UMDEVXS_REMOVE_INTERRUPT

// signaling int handler -> app thread
static struct semaphore
                    UMDevXS_Interrupt_sem [UMDEVXS_INTERRUPT_IC_DEVICE_COUNT];

// Mutex for concurrency protection
static struct mutex
                 UMDevXS_Interrupt_mutex [UMDEVXS_INTERRUPT_IC_DEVICE_COUNT];

static int UMDevXS_Interrupt_InstalledIRQ [UMDEVXS_INTERRUPT_IC_DEVICE_COUNT];

static int UMDevXS_Interrupt_fInstalled [UMDEVXS_INTERRUPT_IC_DEVICE_COUNT];

#endif /* UMDEVXS_REMOVE_INTERRUPT */


/*----------------------------------------------------------------------------
 * UMDevXS_Interrupt_VirtualIrq_Get
 *
 * Converts IC DeviceId to the corresponding system-specific irq line numbers
 */
#ifndef UMDEVXS_REMOVE_INTERRUPT
static int
UMDevXS_Interrupt_VirtualIrq_Get(
        const unsigned int ICDeviceId)
{
#ifndef UMDEVXS_REMOVE_DEVICE_OF
    return UMDevXS_OFDev_GetInterrupt(ICDeviceId);
#else
    // One-to-one mapping from IC device index to OS virtual interrupts (irq)
    // Note: this must be customized for a specific HW platform.
    //       Some OS'es differentiate between physical and virtual
    //       interrupts and some don't.
    return ICDeviceId;
#endif
}
#endif /* UMDEVXS_REMOVE_INTERRUPT */

/*----------------------------------------------------------------------------
 * UMDevXS_Interrupt_ICDeviceId_Get
 *
 * Converts system-specific irq line number to the corresponding IC DeviceId
 */
#ifndef UMDEVXS_REMOVE_INTERRUPT
static unsigned int
UMDevXS_Interrupt_ICDeviceId_Get(
        const int VirtualIrq)
{
    unsigned int i;

    if (VirtualIrq < 0)
        return UMDEVXS_INTERRUPT_IC_DEVICE_COUNT; // invalid IRQ

    for (i = 0; i < UMDEVXS_INTERRUPT_IC_DEVICE_COUNT; i++)
    {
         if (UMDevXS_Interrupt_InstalledIRQ[i] == VirtualIrq)
             return i;
    }

    return UMDEVXS_INTERRUPT_IC_DEVICE_COUNT; // not found
}
#endif /* UMDEVXS_REMOVE_INTERRUPT */

/*----------------------------------------------------------------------------
 * UMDevXS_Interrupt_WaitWithTimeout
 *
 * Returns 0 on interrupt, 1 on timeout, <0 in case of error.
 */
int
UMDevXS_Interrupt_WaitWithTimeout(
        const unsigned int Timeout_ms,
        const int nIRQ)
{
    LOG_INFO(UMDEVXS_LOG_PREFIX
             "%s: invoked for IRQ %d\n",
             __func__,
             nIRQ);

    if (nIRQ < 0)
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX
                 "%s: invalid IRQ %d\n",
                 __func__,
                 nIRQ);
        return -1; // invalid IRQ
    }

#ifdef UMDEVXS_REMOVE_INTERRUPT

    // simulate the timeout
    msleep(Timeout_ms);

    return 1;

#else
    {
        // convert the timeout to jiffies
        unsigned long l_jiffies = msecs_to_jiffies(Timeout_ms);
        int res;
        unsigned int ICDeviceId;
        const unsigned int ICDeviceCount = UMDEVXS_INTERRUPT_IC_DEVICE_COUNT;

        if (ICDeviceCount == 1)
            ICDeviceId = 0;
        else
        {
            if (nIRQ >= UMDEVXS_INTERRUPT_IC_DEVICE_COUNT)
            {
                LOG_CRIT(UMDEVXS_LOG_PREFIX
                         "%s: invalid IRQ %d\n",
                         __func__,
                         nIRQ);
                return -1; // Invalid IRQ
            }

#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
            ICDeviceId = nIRQ;
#else
            ICDeviceId = UMDEVXS_INTERRUPT_IC_DEVICE_IDX;
#endif
        }

        if (!UMDevXS_Interrupt_fInstalled[ICDeviceId])
        {
            LOG_INFO(UMDEVXS_LOG_PREFIX
                     "%s: simulate with timeout %d for IRQ %d\n",
                     __func__,
                     Timeout_ms,
                     nIRQ);
            // simulate the timeout
            msleep(Timeout_ms);
            return 1;
        }

        // warn for high semaphore count
        res = UMDevXS_Interrupt_sem[ICDeviceId].count;
        if (res > 1)
        {
            LOG_WARN(UMDEVXS_LOG_PREFIX
                     "%s: sem.count = %d\n",
                     __func__,
                     res);
        }

        // concurrency protection for the following section
        mutex_lock(&UMDevXS_Interrupt_mutex[ICDeviceId]);

        // when we get here, the interrupt can be enabled or disabled
        // we wait for the semaphore
        // when we get it, we know the interrupt occurred, incremented the
        // semaphore and disabled the interrupt.
        // we can then safely enable the interrupt again

        // wait on the semaphore, with timeout
        res = down_timeout(&UMDevXS_Interrupt_sem[ICDeviceId],
                            (long)l_jiffies);
        if (res == 0)
        {
            // managed to decrement the semaphore

            // allow the semaphore to be incremented by the interrupt handler
            enable_irq(UMDevXS_Interrupt_InstalledIRQ[ICDeviceId]);
        }

        // end of concurrency protection
        mutex_unlock(&UMDevXS_Interrupt_mutex[ICDeviceId]);

        if (res == 0)
        {
            LOG_INFO(UMDEVXS_LOG_PREFIX
                     "%s: down_timeout() returned %d for IRQ %d\n",
                     __func__,
                     res,
                     nIRQ);
            return 0;
        }

        // handle timeout
        // interrupt is still enabled, so do not touch it
        if (res == -ETIME)
        {
            LOG_INFO(UMDEVXS_LOG_PREFIX
                     "%s: down_timeout() returned %d for IRQ %d, still enabled\n",
                     __func__,
                     res,
                     nIRQ);
            return 1;
        }

        LOG_WARN(UMDEVXS_LOG_PREFIX
                 "%s: down_timeout() returned %d for IRQ %d\n",
                 __func__,
                 res,
                 nIRQ);
    }
#endif /* UMDEVXS_REMOVE_INTERRUPT */

    return -2;
}


/*----------------------------------------------------------------------------
 * UMDevXS_Interrupt_TopHalfHandler
 *
 * This is the interrupt handler function call by the kernel when our hooked
 * interrupt is active, which means the interrupt from the device.
 * The interrupt controller is checked for the active interrupt sources.
 * The interrupt handler is invoked directly in the same IRQ top-half context.
 */
#ifndef UMDEVXS_REMOVE_INTERRUPT
static irqreturn_t
UMDevXS_Interrupt_TopHalfHandler(
        int VirtualIrq,
        void * dev_id)
{
    unsigned int ICDeviceId;

    IDENTIFIER_NOT_USED(dev_id);

    LOG_INFO(UMDEVXS_LOG_PREFIX
             "%s: invoked for IRQ %d\n",
             __func__,
             VirtualIrq);

    ICDeviceId = UMDevXS_Interrupt_ICDeviceId_Get(VirtualIrq);
    if (ICDeviceId >= UMDEVXS_INTERRUPT_IC_DEVICE_COUNT)
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX
                 "%s: invalid IC device %d for IRQ %d\n",
                 __func__,
                 ICDeviceId,
                 VirtualIrq);
        return IRQ_NONE;
    }

    if (ICDeviceId & UMDEVXS_INTERRUPT_TRACE_FILTER)
    {
        Log_FormattedMessage(UMDEVXS_LOG_PREFIX
                             "%s: VirtualIrq = %d, for IC device %d\n",
                             __func__,
                             VirtualIrq,
                             ICDeviceId);
    }

    // Disable the interrupt to avoid spinning, it will be enabled when
    // the interrupt event has been propagated to the user-space
    disable_irq_nosync(VirtualIrq);

    // Increment the semaphore
    up(&UMDevXS_Interrupt_sem[ICDeviceId]);

    return IRQ_HANDLED;
}
#endif /* UMDEVXS_REMOVE_INTERRUPT */


/*----------------------------------------------------------------------------
 * UMDevXS_Interrupt_Init
 */
void
UMDevXS_Interrupt_Init(
        const int nIRQ)
{
    if (nIRQ < 0)
        return; // Invalid IRQ

#ifndef UMDEVXS_REMOVE_INTERRUPT
    {
        int res, VirtualIrq;
        void * dev;
        unsigned long flags;
        unsigned int ICDeviceId;
        const unsigned int ICDeviceCount = UMDEVXS_INTERRUPT_IC_DEVICE_COUNT;

        if (ICDeviceCount == 1)
        {
            VirtualIrq = nIRQ;
            ICDeviceId = 0;
        }
        else
        {
            if (nIRQ >= UMDEVXS_INTERRUPT_IC_DEVICE_COUNT)
                return; // Invalid IRQ

            VirtualIrq = UMDevXS_Interrupt_VirtualIrq_Get(nIRQ);
            ICDeviceId = nIRQ;
        }

#ifndef UMDEVXS_REMOVE_DEVICE_OF
        dev = UMDevXS_OFDev_GetReference();
        flags = IRQF_SHARED; // IRQF_DISABLED | IRQ_TYPE_EDGE_RISING;
#else
        dev = (void *)&UMDevXS_Interrupt_InstalledIRQ[0];
        flags = IRQF_SHARED;
#endif

        // semaphore is used in top-half, so make it ready
        sema_init(&UMDevXS_Interrupt_sem[ICDeviceId], /*initial value:*/0);

        mutex_init(&UMDevXS_Interrupt_mutex[ICDeviceId]);

        // must set prior to hooking
        // when interrupt happens immediately, top-half check against this
        UMDevXS_Interrupt_InstalledIRQ[ICDeviceId] = VirtualIrq;

        // install the top-half interrupt handler for the given IRQ
        // any reason not to allow sharing?
        res = request_irq(VirtualIrq,
                          UMDevXS_Interrupt_TopHalfHandler,
                          flags,
                          UMDEVXS_MODULENAME,
                          dev);
        if (res)
        {
            // not hooked after all, so clear global to avoid irq_free
            UMDevXS_Interrupt_InstalledIRQ[ICDeviceId] = -1;

            LOG_CRIT(UMDEVXS_LOG_PREFIX
                     "%s: request_irq() error %d for IRQ %d, IC device %d\n",
                     __func__,
                     res,
                     VirtualIrq,
                     ICDeviceId);
        }
        else
        {
            UMDevXS_Interrupt_fInstalled[ICDeviceId] = true;

            LOG_CRIT(UMDEVXS_LOG_PREFIX
                     "%s: Successfully hooked IRQ %d for IC device %d\n",
                     __func__,
                     VirtualIrq,
                     ICDeviceId);
        }
    }
#endif /* UMDEVXS_REMOVE_INTERRUPT */
}


/*----------------------------------------------------------------------------
 * UMDevXS_Interrupt_UnInit
 */
void
UMDevXS_Interrupt_UnInit(
        const int nIRQ)
{
    if (nIRQ < 0)
        return; // invalid IRQ

#ifndef UMDEVXS_REMOVE_INTERRUPT
    {
        void * dev;
        unsigned int ICDeviceId;
        const unsigned int ICDeviceCount = UMDEVXS_INTERRUPT_IC_DEVICE_COUNT;

        if (ICDeviceCount == 1)
            ICDeviceId = 0;
        else
        {
            if (nIRQ >= UMDEVXS_INTERRUPT_IC_DEVICE_COUNT)
                return; // Invalid IRQ

            ICDeviceId = nIRQ;
        }

        if (!UMDevXS_Interrupt_fInstalled[ICDeviceId])
            return; // Not initialized

#ifndef UMDEVXS_REMOVE_DEVICE_OF
        dev = UMDevXS_OFDev_GetReference();
#else
        dev = (void *)&UMDevXS_Interrupt_InstalledIRQ[0];
#endif

        // Unhook the interrupt
        free_irq(UMDevXS_Interrupt_InstalledIRQ[ICDeviceId], dev);

        UMDevXS_Interrupt_InstalledIRQ[ICDeviceId] = -1;
    }
#endif /* UMDEVXS_REMOVE_INTERRUPT */
}


#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
/*----------------------------------------------------------------------------
 * UMDevXS_Interrupt_Request
 */
int
UMDevXS_Interrupt_Request(
        irq_handler_t Handler_p,
        int ICDeviceId)
{
    int nIRQ = 0;

#ifndef UMDEVXS_REMOVE_INTERRUPT
    int res;
    void * dev;
    unsigned long flags;

    if (!UMDevXS_Interrupt_fInstalled[ICDeviceId])
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX
                 "%s: failed, IC device %d not initialized\n",
                 __func__,
                 ICDeviceId);
        return -1;
    }

#ifndef UMDEVXS_REMOVE_DEVICE_OF
    dev = UMDevXS_OFDev_GetReference();
    nIRQ = UMDevXS_OFDev_GetInterrupt(ICDeviceId);
    flags = IRQF_SHARED;
#else
    dev = (void *)&UMDevXS_Interrupt_InstalledIRQ[0];
    nIRQ = UMDevXS_Interrupt_InstalledIRQ[ICDeviceId];
    flags = IRQF_SHARED;
#endif // !UMDEVXS_REMOVE_DEVICE_OF

    LOG_INFO(UMDEVXS_LOG_PREFIX
             "%s: unhooking handler for IRQ %d\n",
             __func__,
             UMDevXS_Interrupt_InstalledIRQ[ICDeviceId]);

    if (nIRQ == UMDevXS_Interrupt_InstalledIRQ[ICDeviceId] ||
                Handler_p == NULL)
    {
        // unhook original interrupt
        free_irq(nIRQ, dev);
    }

    if (Handler_p == NULL && nIRQ ==
                        UMDevXS_Interrupt_InstalledIRQ[ICDeviceId])
        Handler_p = UMDevXS_Interrupt_TopHalfHandler;

    if (Handler_p != NULL)
    {
        res = request_irq(nIRQ,
                          Handler_p,
                          flags,
                          UMDEVXS_MODULENAME,
                          dev);
        if (res)
        {
            // not hooked after all, so clear global to avoid irq_free
            UMDevXS_Interrupt_InstalledIRQ[ICDeviceId] = -1;

            LOG_CRIT(UMDEVXS_LOG_PREFIX
                     "%s: request_irq failed, error %d\n",
                     __func__,
                     res);
            return -1;
        }

        LOG_INFO(UMDEVXS_LOG_PREFIX
                 "%s: successfully hooked new handler "
                 "for IRQ %d, IC device %d\n",
                 __func__,
                 nIRQ,
                 ICDeviceId);
    }

#endif // !UMDEVXS_REMOVE_INTERRUPT

    return nIRQ;
}
#endif // !UMDEVXS_ENABLE_KERNEL_SUPPORT


/* end of file umdevxs_interrupt.c */
