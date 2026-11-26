/* umdevxs_lkm.c
 *
 * Loadable Kernel Module (LKM) support the Linux UMDevXS driver.
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


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_umdevxs.h"

// Linux Kernel API
#include <linux/errno.h>
#include <linux/module.h>

#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
#include "shdevxs_kernel.h"

#ifndef UMDEVXS_REMOVE_PCI
#include "umdevxs_pcidev.h"
#endif

#ifndef UMDEVXS_REMOVE_DEVICE_OF
#include "umdevxs_ofdev.h"
#endif

#include "umdevxs_device.h"
#include "umdevxs_interrupt.h"
#endif // UMDEVXS_ENABLE_KERNEL_SUPPORT

#include "umdevxs_chrdev.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

MODULE_LICENSE(UMDEVXS_LICENSE);

#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT

#ifndef UMDEVXS_REMOVE_PCI
EXPORT_SYMBOL(UMDevXS_PCIDev_Get);
EXPORT_SYMBOL(UMDevXS_PCIDev_HandleCmd_Write32);
EXPORT_SYMBOL(UMDevXS_PCIDev_HandleCmd_Read32);
EXPORT_SYMBOL(UMDevXS_PCIDev_Init);
EXPORT_SYMBOL(UMDevXS_PCIDev_UnInit);
#endif

#ifndef UMDEVXS_REMOVE_DEVICE_OF
#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
EXPORT_SYMBOL(UMDevXS_OFDev_GetDevice);
#endif
#endif

EXPORT_SYMBOL(UMDevXS_Interrupt_Request);
EXPORT_SYMBOL(UMDevXS_Device_LockRange);
EXPORT_SYMBOL(UMDevXS_Device_Unlock);
EXPORT_SYMBOL(UMDevXS_Device_SetPrivileged);

#endif // UMDEVXS_ENABLE_KERNEL_SUPPORT


/*----------------------------------------------------------------------------
 * Local variables
 */

#ifndef UMDEVXS_REMOVE_INTERRUPT
static int UMDevXS_LKM_nIRQ;
#endif


/*----------------------------------------------------------------------------
 * UMDevXS_module_exit
 */
static void
UMDevXS_module_exit(void);


/*----------------------------------------------------------------------------
 * UMDevXS_module_init
 */
static int
UMDevXS_module_init(void)
{
    int Status;

#if (!defined(UMDEVXS_REMOVE_INTERRUPT) && \
     defined(UMDEVXS_REMOVE_DEVICE_PCI) && \
     defined(UMDEVXS_REMOVE_DEVICE_OF))
    Status = UMDEVXS_INTERRUPT_STATIC_IRQ;
#endif

    LOG_INFO(UMDEVXS_LOG_PREFIX
             "loading driver\n");

#ifndef UMDEVXS_REMOVE_PCI
    Status = UMDevXS_PCIDev_Init();
    if (Status < 0)
        goto error_exit;
#endif

#ifndef UMDEVXS_REMOVE_DEVICE_OF
    Status = UMDevXS_OFDev_Init();
    if (Status < 0)
        goto error_exit;
#endif

#ifndef UMDEVXS_REMOVE_INTERRUPT
    {
        const unsigned int DeviceCount = UMDEVXS_INTERRUPT_IC_DEVICE_COUNT;
        unsigned int i;

        UMDevXS_LKM_nIRQ = Status;

        if (DeviceCount == 1)
            UMDevXS_Interrupt_Init(UMDevXS_LKM_nIRQ);
        else
            for (i = 0; i < DeviceCount; i++)
                UMDevXS_Interrupt_Init(i);
    }
#endif

    Status = UMDevXS_ChrDev_Init();
    if (Status < 0)
        goto error_exit;

#ifndef UMDEVXS_REMOVE_DMABUF
    if (!BufAdmin_Init(UMDEVXS_DMARESOURCE_HANDLES_MAX, NULL, 0))
    {
        Status = -ENOMEM;
        goto error_exit;
    }
#endif

#ifndef UMDEVXS_REMOVE_SMBUF
    Status = UMDevXS_SMBuf_Init();
    if (Status < 0)
        return Status;
#endif

#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
    Status = SHDevXS_Init();
    if (Status < 0)
        goto error_exit;
#endif
    return 0;

error_exit:
    UMDevXS_module_exit();
    return Status;
}


/*----------------------------------------------------------------------------
 * UMDevXS_module_exit
 */
static void
UMDevXS_module_exit(void)
{
    LOG_INFO(UMDEVXS_LOG_PREFIX
             "unloading driver\n");

#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
    SHDevXS_UnInit();
#endif

#ifndef UMDEVXS_REMOVE_SMBUF
    UMDevXS_SMBuf_UnInit();
#endif

#ifndef UMDEVXS_REMOVE_INTERRUPT
    {
        const unsigned int DeviceCount = UMDEVXS_INTERRUPT_IC_DEVICE_COUNT;
        unsigned int i;

        if (DeviceCount == 1)
            UMDevXS_Interrupt_UnInit(UMDevXS_LKM_nIRQ);
        else
            for (i = 0; i < DeviceCount; i++)
                UMDevXS_Interrupt_UnInit(i);
    }
#endif

#ifndef UMDEVXS_REMOVE_PCI
    UMDevXS_PCIDev_UnInit();
#endif

#ifndef UMDEVXS_REMOVE_DEVICE_OF
    UMDevXS_OFDev_UnInit();
#endif

    UMDevXS_ChrDev_UnInit();

#ifndef UMDEVXS_REMOVE_DMABUF
    BufAdmin_UnInit();
#endif
}

module_init(UMDevXS_module_init);
module_exit(UMDevXS_module_exit);


/* end of file umdevxs_lkm.c */
