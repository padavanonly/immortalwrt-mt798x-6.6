/* shdevxs_kernel_lock.c
 *
 * Locking implementation.
 * Linux kernel space
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

#include "shdevxs_kernel_internal.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_shdevxs.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // IDENTIFIER_NOT_USED

// Linux Kernel API
#include <linux/slab.h>         // kmalloc, kfree
#ifdef SHDEVXS_LOCK_SLEEPABLE
#include <linux/mutex.h>        // mutex_*
#else
#include <linux/spinlock.h>     // spinlock_*
#include <linux/hardirq.h>      // in_atomic
#endif

#include "log.h"


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_Lock_Alloc
 */
void *
SHDevXS_Internal_Lock_Alloc(void)
{
#ifdef SHDEVXS_LOCK_SLEEPABLE
    struct mutex * SHDevXS_Lock = kmalloc(sizeof(struct mutex), GFP_KERNEL);
    if (SHDevXS_Lock == NULL)
        return NULL;

    LOG_INFO("SHDevXS_Internal_Lock_Alloc: Lock = mutex\n");
    mutex_init(SHDevXS_Lock);

    return SHDevXS_Lock;
#else
    spinlock_t * SHDevXS_SpinLock;
    gfp_t flags = 0;

    size_t LockSize = sizeof(spinlock_t);
    if (LockSize == 0)
        LockSize = 4;

    if (in_atomic())
        flags |= GFP_ATOMIC;    // non-sleepable
    else
        flags |= GFP_KERNEL;    // sleepable

    SHDevXS_SpinLock = kmalloc(LockSize, flags);
    if (SHDevXS_SpinLock == NULL)
        return NULL;

    LOG_INFO("SHDevXS_Internal_Lock_Alloc: Lock = spinlock\n");
    spin_lock_init(SHDevXS_SpinLock);

    return SHDevXS_SpinLock;
#endif
}


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_Lock_Free
 */
void
SHDevXS_Internal_Lock_Free(void * Lock_p)
{
    kfree(Lock_p);
}


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_Lock_Acquire
 */
void
SHDevXS_Internal_Lock_Acquire(
        void * Lock_p,
        unsigned long * Flags)
{
#ifdef SHDEVXS_LOCK_SLEEPABLE
    IDENTIFIER_NOT_USED(Flags);
    mutex_lock((struct mutex*)Lock_p);
#else
    spin_lock_irqsave((spinlock_t *)Lock_p, *Flags);
#endif
}


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_Lock_Release
 */
void
SHDevXS_Internal_Lock_Release(
        void * Lock_p,
        unsigned long * Flags)
{
#ifdef SHDEVXS_LOCK_SLEEPABLE
    IDENTIFIER_NOT_USED(Flags);
    mutex_unlock((struct mutex*)Lock_p);
#else
    spin_unlock_irqrestore((spinlock_t *)Lock_p, *Flags);
#endif
}

/* end of file shdevxs_kernel_lock.c */


