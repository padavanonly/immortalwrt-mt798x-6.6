/* adapter_alloc.h
 *
 * Linux kernel-space implementation of the Adapter buffer allocation
 * functionality.
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

#ifndef ADAPTER_ALLOC_H_
#define ADAPTER_ALLOC_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Linux Kernel API
#include <linux/kernel.h>
#include <linux/slab.h>         // kmalloc, kfree
#include <linux/hardirq.h>      // in_atomic


/*----------------------------------------------------------------------------
  Adapter_Alloc
*/
static inline void *
Adapter_Alloc(unsigned int size)
{
    if (size > 0) // Many callers do not check if requested size is non-zero
        // kmalloc() may return non-NULL for size=0
        return kmalloc(size, in_atomic() ? GFP_ATOMIC : GFP_KERNEL);
    else
        return NULL; // ... but only extreme optimists do not check the result!
}


/*----------------------------------------------------------------------------
  Adapter_Free
*/
static inline void
Adapter_Free(void *p)
{
    if (p != NULL)
    {
        kfree(p); // kfree() may take a NULL pointer but why bother
        p = NULL;
    }
}


#endif /* ADAPTER_ALLOC_H_ */


/* end of file adapter_alloc.h */
