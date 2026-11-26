/* umdevxs_ofdev.c
 *
 * Platform device support for the Linux UMDevXS driver.
 */

/*****************************************************************************
* Copyright (c) 2010-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef UMDEVXS_REMOVE_DEVICE_OF

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "umdevxs_internal.h"

#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
#include "umdevxs_ofdev.h"          // UMDevXS_OFDev_GetDevice
#endif


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_umdevxs.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"             // uint8_t, MASK_xx_BITS, etc.

// Linux Kernel Module interface
#include "lkm.h"                    // LKM_*

// Linux Kernel API
#include <linux/platform_device.h>  // platform_*,
#include <linux/mm.h>               // remap_pfn_range
#include <linux/version.h>

#ifdef UMDEVXS_USE_RPM
// Runtime Power Management Kernel Macros API
#include "rpm_kernel_macros.h"      // RPM_*
#endif


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */

// Pointer to the array of IRQ devices
#ifndef UMDEVXS_REMOVE_INTERRUPT
static int * UMDEVXS_OFDev_Virtual_IRQ_p;
#endif


/*----------------------------------------------------------------------------
 * UMDevXS_OFDev_Init
 *
 * Returns <0 on error.
 * Returns >=0 on success. The number is the interrupt number associated
 * with this OF device.
 */
int
UMDevXS_OFDev_Init(void)
{
    LKM_Init_t LKMInit;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: entered\n", __func__);

    ZEROINIT(LKMInit);

    LKMInit.DriverName_p        = UMDEVXS_MODULENAME;

#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
    LKMInit.fRetainMap          = true;
#endif

#ifdef UMDEVXS_USE_RPM
    LKMInit.PM_p                = RPM_OPS_PM;
#endif

    if (LKM_Init(&LKMInit) < 0)
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX
                 "%s: Failed to register the OF device\n",
                 __func__);
        return -1;
    }

#ifdef UMDEVXS_USE_RPM
    if (RPM_INIT_MACRO(LKM_DeviceGeneric_Get()) != RPM_SUCCESS)
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX "%s: RPM_Init() failed\n", __func__);
        LKM_Uninit();
        return -2; // error
    }
#endif

    LOG_INFO(UMDEVXS_LOG_PREFIX
             "%s: left, initialization successful\n",
             __func__);

#ifndef UMDEVXS_REMOVE_INTERRUPT
    UMDEVXS_OFDev_Virtual_IRQ_p = (int *)LKMInit.CustomInitData_p;

    return UMDEVXS_OFDev_Virtual_IRQ_p[0];
#else
    return 0; // success
#endif // UMDEVXS_REMOVE_INTERRUPT
}


/*----------------------------------------------------------------------------
 * UMDevXS_OFDev_UnInit
 */
void
UMDevXS_OFDev_UnInit(void)
{
    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: entered\n", __func__);

#ifdef UMDEVXS_USE_RPM
    // Check if a race condition is possible here with auto-suspend timer
    (void)RPM_UNINIT_MACRO();
#endif

    LKM_Uninit();

#ifndef UMDEVXS_REMOVE_INTERRUPT
    UMDEVXS_OFDev_Virtual_IRQ_p = NULL;
#endif

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: left\n", __func__);
}


/*----------------------------------------------------------------------------
 * UMDevXS_OFDev_Map
 */
int
UMDevXS_OFDev_Map(
        unsigned int SubsetStart,       // defined
        unsigned int SubsetSize,        // defined
        unsigned int Length,            // requested
        struct vm_area_struct * vma_p)
{
    int res;
    uint32_t address = 0;

    LOG_INFO(UMDEVXS_LOG_PREFIX
             "%s: Subset start=%u and size=%u, Length=%u\n",
             __func__,
             SubsetStart,
             SubsetSize,
             Length);

    // honor requested length if less than defined
    if (SubsetSize > Length)
        SubsetSize = Length;

    // reject requested length request if greater than defined
    if (Length > SubsetSize)
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX
                 "%s: length rejected (%u)\n",
                 __func__,
                 Length);

        return -1;
    }

    // was the OF device enabled by the OS
    // this only happens when a compatible OF device is found
    if (LKM_DeviceSpecific_Get() == NULL)
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX
                 "%s: platform device not found\n",
                 __func__);

        return -2;
    }

    // get OF device physical address
    {
        struct resource * rsc_p = NULL;

        // Exported under GPL
        rsc_p = platform_get_resource(LKM_DeviceSpecific_Get(),
                                      IORESOURCE_MEM,
                                      0); // resource ID

        // device tree specific for this OF device
        // only 32-bit physical addresses are supported
        if(rsc_p)
        {
            LOG_INFO(UMDEVXS_LOG_PREFIX
                     "UMDevXS_OFDev_Map: "
                     "mem start=%08x, end=%08x, flags=%08x\n",
                     (unsigned int)rsc_p->start,
                     (unsigned int)rsc_p->end,
                     (unsigned int)rsc_p->flags);

            if(Length > rsc_p->end - rsc_p->start + 1)
            {
                LOG_CRIT(UMDEVXS_LOG_PREFIX
                         "UMDevXS_OFDev_Map: length rejected (%u)\n",
                         Length);

                return -3;
            }

            address = (uint32_t)rsc_p->start + (uint32_t)SubsetStart;
        }
        else
        {
            LOG_CRIT(UMDEVXS_LOG_PREFIX
                     "UMDevXS_OFDev_Map: memory resource not found\n");

            return -4;
        }
    }

    // avoid caching and buffering
    vma_p->vm_page_prot = pgprot_noncached(vma_p->vm_page_prot);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0))
        vm_flags_set(vma_p, VM_IO);
#else
        vma_p->vm_flags |= VM_IO;
#endif

    // map the range into application space
    res = remap_pfn_range(
                    vma_p,
                    vma_p->vm_start,
                    address >> PAGE_SHIFT,
                    Length,
                    vma_p->vm_page_prot);

    if (res < 0)
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX
                 "UMDevXS_OFDev_Map: "
                 "remap_pfn_range failed (%d), addr = %08x, len = %x\n",
                 res,
                 (unsigned int)address,
                 Length);

        return -5;
    }

    LOG_INFO(UMDEVXS_LOG_PREFIX
             "%s: successfully mapped platform device\n",
             __func__);

    // return success
    return 0;
}


/*----------------------------------------------------------------------------
 * UMDevXS_OFDev_GetReference
 */
void*
UMDevXS_OFDev_GetReference(void)
{
    return LKM_DeviceGeneric_Get();
}


#ifndef UMDEVXS_REMOVE_INTERRUPT
/*----------------------------------------------------------------------------
 * UMDevXS_OFDev_GetInterrupts
 */
unsigned int
UMDevXS_OFDev_GetInterrupt(unsigned int index)
{
    if (index >= UMDEVXS_INTERRUPT_IC_DEVICE_COUNT)
        return 0;
    else
        return UMDEVXS_OFDev_Virtual_IRQ_p[index];
}
#endif


#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
/*----------------------------------------------------------------------------
 * UMDevXS_OFDev_GetDevice
 */
int
UMDevXS_OFDev_GetDevice(
              unsigned int DeviceID,
              struct platform_device ** OF_Device_pp,
              void __iomem ** MappedBaseAddr_pp)
{
    *OF_Device_pp      = LKM_DeviceSpecific_Get();
    *MappedBaseAddr_pp = LKM_MappedBaseAddr_Get();

    IDENTIFIER_NOT_USED(DeviceID);

    return 0;
}
#endif // UMDEVXS_ENABLE_KERNEL_SUPPORT


#endif // UMDEVXS_REMOVE_DEVICE_OF


/* end of file umdevxs_ofdev.c */
