/* shdevxs_dmapool.c
 *
 * Shared Device Access DMA Pool API implementation
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

#include "shdevxs_dmapool.h"

#include "shdevxs_kernel_internal.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_shdevxs.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // IDENTIFIER_NOT_USED

// UMDevXS API: Shared Memory Buffer
#include "umdevxs_smbuf.h"
#include "umdevxs_internal.h"

// Logging API
#include "log.h"

// Runtime Power Management Device Macros API
#include "rpm_device_macros.h"  // RPM_*

// Linux Kernel API
#include <linux/module.h>       // EXPORT_SYMBOL
#include <linux/mm.h>           // _get_free_pages, free_pages, get_order
#include <asm/io.h>             // virt_to_phys
#include <linux/slab.h>         // kmalloc, kfree
#include <linux/dma-mapping.h>  // dma_alloc_coherent, dma_free_coherent

/*----------------------------------------------------------------------------
 * Definitions and macros
 */
EXPORT_SYMBOL(SHDevXS_DMAPool_Init);
EXPORT_SYMBOL(SHDevXS_DMAPool_Uninit);
EXPORT_SYMBOL(SHDevXS_DMAPool_GetBase);

#ifdef UMDEVXS_64BIT_HOST
#ifdef UMDEVXS_64BIT_DEVICE
#define SHDEVXS_DMA_FLAGS 0 // No special requirements for address.
#else
#define SHDEVXS_DMA_FLAGS GFP_DMA // 64-bit host, 32-bit memory addresses
#endif
#else
#define SHDEVXS_DMA_FLAGS 0 // No special requirements for address.
#endif

typedef struct
{
    void *AppId;
    bool IsUsed;
    int Handle; // SM Buf Handle.
} DMAPool_MemArea_Descriptor_t;

/*----------------------------------------------------------------------------
 * Local variables
 */
static void *DMAPool_MemArea_p = NULL;
// Host address of the Record cache memory area

static void *DMAPool_DMAAddr_p = NULL;
// Bus address of the same area.

static DMAPool_MemArea_Descriptor_t
    DMAPool_MemArea_Descriptors[SHDEVXS_MAX_APPS];

static void * Descriptor_Lock;


#ifdef SHDEVXS_ENABLE_RC_FUNCTIONS
#ifdef SHDEVXS_IRQ_RPM_DEVICE_ID
/*----------------------------------------------------------------------------
 * SHDevXS_DMAPool_Resume
 *
 */
static int
SHDevXS_DMAPool_Resume(void * p)
{
    IDENTIFIER_NOT_USED(p);

    // Restore EIP-207 Record Cache base address for shared DMA pool
    SHDevXS_Internal_RC_SetBase(DMAPool_DMAAddr_p);

    return 0; //success
}
#endif
#endif


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_DMAPool_Initialize
 *
 * Initialize the DMA Pool and the associcated memory bank.
 *
 * Return 0 on success, -1 on failure.
 */
int
SHDevXS_Internal_DMAPool_Initialize(void)
{
    unsigned int i;

#ifdef SHDEVXS_DMARESOURCE_ALLOC_CACHE_COHERENT
    dma_addr_t DMAAddr;
    struct device * Device_p = NULL;

#ifndef UMDEVXS_REMOVE_DEVICE_OF
    Device_p = UMDevXS_OFDev_GetReference();
#endif

#ifndef UMDEVXS_REMOVE_PCI
    Device_p = UMDevXS_PCIDev_GetReference(NULL);
#endif

    DMAPool_MemArea_p = dma_alloc_coherent(Device_p,
                                           SHDEVXS_MAX_APPS *
                                               SHDEVXS_TR_BANK_SIZE,
                                           &DMAAddr,
                                           GFP_KERNEL | SHDEVXS_DMA_FLAGS);
    if (DMAPool_MemArea_p == NULL)
    {
        LOG_CRIT("SHDevXS_Internal_DMAPool_Initialize: bank allocation failed\n");
        return -1;
    }
    DMAPool_DMAAddr_p = (void*)DMAAddr;
#else
    int order = get_order(SHDEVXS_MAX_APPS * SHDEVXS_TR_BANK_SIZE);

    // Allocate the memory area.
    DMAPool_MemArea_p = (void *)__get_free_pages(GFP_KERNEL | SHDEVXS_DMA_FLAGS,
                                                 order);
    if (DMAPool_MemArea_p == NULL)
    {
        LOG_CRIT("SHDevXS_Internal_DMAPool_Initialize: bank allocation failed\n");
        return -1;
    }
    DMAPool_DMAAddr_p = (void*)virt_to_phys(DMAPool_MemArea_p);
#endif

    // Initialize the data structures
    for (i = 0; i < SHDEVXS_MAX_APPS; i++)
        DMAPool_MemArea_Descriptors[i].IsUsed = false;

    // Initialize the locks
    Descriptor_Lock = SHDevXS_Internal_Lock_Alloc();

	if (Descriptor_Lock == NULL)
	{
		LOG_CRIT("SHDevXS_Internal_DMAPool_Initialize: lock allocation failed\n");
		SHDevXS_Internal_DMAPool_UnInitialize();
		return -1;
	} else {
		LOG_INFO("SHDevXS_Internal_DMAPool_Initialize: Lock obtained\n");
	}

    return 0;
}


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_DMAPool_UnInitialize
 *
 * Uninitialize the DMA pool and the associated memory bank.
 */
void
SHDevXS_Internal_DMAPool_UnInitialize(void)
{
    unsigned int i;

    // Check whether some application still use their memory area
    for (i = 0; i < SHDEVXS_MAX_APPS; i++)
    {
        if(DMAPool_MemArea_Descriptors[i].IsUsed)
        {
            LOG_CRIT("SHDevXS_Internal_DMAPool_UnInitialize: "
                     "area %u not freed\n",i);
        }
    }

    // Free Memory area
    if (DMAPool_MemArea_p != NULL)
    {
#ifdef SHDEVXS_DMARESOURCE_ALLOC_CACHE_COHERENT
        struct device * Device_p = NULL;

#ifndef UMDEVXS_REMOVE_DEVICE_OF
        Device_p = UMDevXS_OFDev_GetReference();
#endif

#ifndef UMDEVXS_REMOVE_PCI
        Device_p = UMDevXS_PCIDev_GetReference(NULL);
#endif

        dma_free_coherent(Device_p,
                          SHDEVXS_MAX_APPS * SHDEVXS_TR_BANK_SIZE,
                          DMAPool_MemArea_p,
                          (dma_addr_t)DMAPool_DMAAddr_p);

#else
        int order = get_order(SHDEVXS_MAX_APPS * SHDEVXS_TR_BANK_SIZE);
        free_pages((unsigned long)DMAPool_MemArea_p, order);
#endif
    }

    DMAPool_MemArea_p = NULL;
    DMAPool_DMAAddr_p = NULL;

    if(Descriptor_Lock != NULL)
    {
        SHDevXS_Internal_Lock_Free(Descriptor_Lock);
        Descriptor_Lock = NULL;
    }
}



/*----------------------------------------------------------------------------
 * SHDevXS_Internal_DMAPool_Init
 */
int
SHDevXS_Internal_DMAPool_Init(
        void *AppId,
        SHDevXS_DMAPool_t * const DMAPool_p)
{
    unsigned long flags;
    unsigned int i;

    if (DMAPool_p == NULL)
        return -1;

    // Obtain the lock
    SHDevXS_Internal_Lock_Acquire(Descriptor_Lock,&flags);

    // Find an unused application area
    for (i = 0; i < SHDEVXS_MAX_APPS; i++)
    {
        if (DMAPool_MemArea_Descriptors[i].IsUsed == false)
            break;
    }
    if (i == SHDEVXS_MAX_APPS)
    {
        LOG_CRIT("SHDevXS_Internal_DMAPool_Init: Allocation error\n");
        // Release the lock
        SHDevXS_Internal_Lock_Release(Descriptor_Lock,&flags);
        return -1;
    }

    // Claim it.
    LOG_INFO("SHDevXS_Internal_DMAPool_Init: "
             "Allocating DMA Pool area %u address %p\n",
             i,
             DMAPool_DMAAddr_p + i * SHDEVXS_TR_BANK_SIZE);

    DMAPool_MemArea_Descriptors[i].IsUsed = true;
    DMAPool_MemArea_Descriptors[i].AppId = AppId;

    // Release the lock
    SHDevXS_Internal_Lock_Release(Descriptor_Lock,&flags);

    // Fill in the fields in DMAPool_p.
    DMAPool_p->HostAddr.p   = DMAPool_MemArea_p + i * SHDEVXS_TR_BANK_SIZE;
    DMAPool_p->DMA_Addr.p   = DMAPool_DMAAddr_p + i * SHDEVXS_TR_BANK_SIZE;
    DMAPool_p->fCached      = true;
    DMAPool_p->ByteCount    = SHDEVXS_TR_BANK_SIZE;
    DMAPool_p->Handle       = 0;
    DMAPool_p->PoolId       = i;

    // Obtain a handle if not kernel driver
    if (AppId != NULL)
    {
        int res;
        int RetHandle;

        LOG_INFO("SHDevXS_Internal_DAMPool_Init: Allocating handle\n");

        res = UMDevXS_SMBuf_Register(SHDEVXS_TR_BANK_SIZE,
                                     NULL,
                                     DMAPool_p->DMA_Addr.p,
                                     &RetHandle);
        if (res != 0)
        {
            LOG_CRIT("SHDevXS_Internal_DMAPool_Init: "
                     "Failed to obtain handle.\n");
            return -1;
        }

        UMDevXS_SMBuf_SetAppID(RetHandle, AppId);

        DMAPool_MemArea_Descriptors[i].Handle = RetHandle;
        DMAPool_p->Handle = RetHandle;
    }

#ifdef SHDEVXS_ENABLE_RC_FUNCTIONS
    if (RPM_DEVICE_INIT_START_MACRO(SHDEVXS_RC_RPM_DEVICE_ID,
                                    0, // Suspend callback not used
                                    SHDevXS_DMAPool_Resume) != RPM_SUCCESS)
        return -1;

    SHDevXS_Internal_RC_SetBase(DMAPool_DMAAddr_p);

    (void)RPM_DEVICE_INIT_STOP_MACRO(SHDEVXS_RC_RPM_DEVICE_ID);
#endif

    return 0;
}


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_DMAPool_Uninit
 */
void
SHDevXS_Internal_DMAPool_Uninit(
        void *AppId)
{
    unsigned int i;
    unsigned long flags;
    unsigned int HandlesToFree [SHDEVXS_MAX_APPS];

    // Obtain the lock
    SHDevXS_Internal_Lock_Acquire(Descriptor_Lock,&flags);

    // Iterate over all application areas,
    for (i = 0; i < SHDEVXS_MAX_APPS; i++)
    {
        HandlesToFree[i] = -1; // set to invalid value

        if (DMAPool_MemArea_Descriptors[i].IsUsed &&
            DMAPool_MemArea_Descriptors[i].AppId == AppId)
        {
            // Found area belonging to that application.
            LOG_INFO("SHDevXS_Internal_DMAPool_Uninit: "
                     "Freeing DMAPool area %u\n",i);

            if (AppId != NULL)
            {
                // Release buffer handle.
                LOG_INFO("SHDevXS_Internal_DMAPool_Uninit: Releasing handle\n");
                HandlesToFree[i] = DMAPool_MemArea_Descriptors[i].Handle;
            }

            // Mark as unused
            DMAPool_MemArea_Descriptors[i].IsUsed = false;
            DMAPool_MemArea_Descriptors[i].AppId = NULL;
        }
    }

    // Release the lock
    SHDevXS_Internal_Lock_Release(Descriptor_Lock,&flags);

    for (i = 0; i < SHDEVXS_MAX_APPS; i++)
    {
        // Free all valid handles
        if (HandlesToFree[i] != -1)
            UMDevXS_SMBuf_Release(HandlesToFree[i]);
    }

    (void)RPM_DEVICE_UNINIT_START_MACRO(SHDEVXS_RC_RPM_DEVICE_ID, false);
    (void)RPM_DEVICE_UNINIT_STOP_MACRO(SHDEVXS_RC_RPM_DEVICE_ID);

    return;
}


/*----------------------------------------------------------------------------
 * SHDevXS_DMAPool_Init
 */
int
SHDevXS_DMAPool_Init(
        SHDevXS_DMAPool_t * const DMAPool_p)
{
    return SHDevXS_Internal_DMAPool_Init(NULL, DMAPool_p);
}


/*----------------------------------------------------------------------------
 * SHDevXS_DMAPool_Uninit
 */
void
SHDevXS_DMAPool_Uninit(void)
{
    SHDevXS_Internal_DMAPool_Uninit(NULL);
    return;
}


/*----------------------------------------------------------------------------
 * SHDevXS_DMAPool_GetBase
 */
int
SHDevXS_DMAPool_GetBase(
        void ** BaseAddr_p)
{
    *BaseAddr_p = DMAPool_DMAAddr_p;
    return 0;
}

/* end of file shdevxs_dmapool.c */


