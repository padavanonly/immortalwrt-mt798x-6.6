/* umdevxs_smbuf.c
 *
 * Shared Memory Buffer - Obtainer & Provider role.
 *
 * The provider allocates shared memory and hands it out to another host.
 * The obtainer can use memory allocated by another host.
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

#include "c_umdevxs.h"          // config options

#ifndef UMDEVXS_REMOVE_SMBUF

#include "umdevxs_smbuf.h"

#include "clib.h"               // ZEROINIT()
#include "umdevxs_internal.h"
#include "log.h"

// Linux kernel API
#include <linux/mm.h>           // remap_pfn_range & find_vma
#include <linux/sched.h>        // task_struct
#include <linux/types.h>        // uintptr_t, phys_addr_t
#include <asm/io.h>             // virt_to_phys
#include <asm/current.h>        // current
#include <asm/cacheflush.h>     // flush_cache_range
#include <linux/dma-mapping.h>  // dma_sync_single_for_cpu, dma_alloc_coherent,
                                // dma_free_coherent, dma_get_cache_alignment


#ifdef UMDEVXS_64BIT_HOST

#ifdef UMDEVXS_64BIT_DEVICE
#define UMDEVXS_DMA_FLAGS 0 // No special requirements for address.
#else
#define UMDEVXS_DMA_FLAGS GFP_DMA // 64-bit host, 32-bit memory addresses
#endif // UMDEVXS_64BIT_DEVICE

#else
#define UMDEVXS_DMA_FLAGS 0 // No special requirements for address.
#endif // UMDEVXS_64BIT_HOST

#ifdef UMDEVXS_SMBUF_CACHE_COHERENT
#define UMDEVXS_SMBUF_SET_MASK        dma_set_coherent_mask
#else
#define UMDEVXS_SMBUF_SET_MASK        dma_set_mask
#endif


#ifdef UMDEVXS_SMBUF_DIRECT_DCACHE_CONTROL

// dma_get_cache_alignment()
#define UMDEVXS_SMBUF_CACHE_LINE_BYTE_COUNT  L1_CACHE_BYTES

#ifdef UMDEVXS_SMBUF_DSB_ENABLE
/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_StoreBufferFlush
 *
 */
static void
HWPAL_DMAResource_StoreBufferFlush(void)
{
#ifndef UMDEVXS_SMBUF_CACHE_COHERENT
    // Complete all outstanding explicit memory operations
    // (Data Synchronization Barrier)
    asm("mcr p15, 0, r0, c7, c10, 4\n\t"
         "dsb\n\t"
         "bx lr");
#endif // UMDEVXS_SMBUF_CACHE_COHERENT
}
#endif // UMDEVXS_SMBUF_DSB_ENABLE


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_DCache_Clean
 *
 * Host must do cache cleaning before a device can do a dma transfer
 * from the buffer
 */
static void
HWPAL_DMAResource_DCache_Clean(
        const void* Buffer_p,
        const unsigned int ByteCount)
{
#if !defined(UMDEVXS_SMBUF_CACHE_COHERENT) && \
    !defined(UMDEVXS_SMBUF_DCACHE_WRITE_TROUGH)
    uint32_t start = (uint32_t)Buffer_p;
    uint32_t p;

    // Align buffer start address by cache line size
    p = start & (~(UMDEVXS_SMBUF_CACHE_LINE_BYTE_COUNT-1));

    // Clean all cache lines where the buffer resides
    while(p < start + ByteCount)
    {
        /* cache line is UMDEVXS_SMBUF_CACHE_LINE_BYTE_COUNT bytes,
           mask out lower address bits,
           clean cache line using virtual address,
           MCR p15, 0, <Rd>, c7, c10, 1 */
        asm(" mcr p15, 0, %0, c7, c10, 1"
            : /* no output */
            : "r"(p) /* input */);

        p += UMDEVXS_SMBUF_CACHE_LINE_BYTE_COUNT;
    } // while
#else
    IDENTIFIER_NOT_USED(Buffer_p);
    IDENTIFIER_NOT_USED(ByteCount);
#endif
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_DCache_Invalidate
 *
 * Host must do cache invalidation after a device has done a dma transfer to
 * the buffer
 */
static void
HWPAL_DMAResource_DCache_Invalidate(
        const void* Buffer_p,
        const unsigned int ByteCount)
{
#ifndef UMDEVXS_SMBUF_CACHE_COHERENT
    uint32_t start = (uint32_t)Buffer_p;
    uint32_t p;

    // Align buffer start address by cache line size
    p = start & (~(UMDEVXS_SMBUF_CACHE_LINE_BYTE_COUNT-1));

    // Invalidate all cache lines where the buffer resides
    while(p < start + ByteCount)
    {
        /* cache line is UMDEVXS_SMBUF_CACHE_LINE_BYTE_COUNT bytes,
           mask out lower address bits,
           invalidate cache line using virtual address,
           MCR p15, 0, <Rd>, c7, c6,  1 */
        asm(" mcr p15, 0, %0, c7, c6, 1"
            : /* no output */
            : "r"(p) /* input */);

        p += UMDEVXS_SMBUF_CACHE_LINE_BYTE_COUNT;
    } // while
#else
    IDENTIFIER_NOT_USED(Buffer_p);
    IDENTIFIER_NOT_USED(ByteCount);
#endif // !UMDEVXS_SMBUF_CACHE_COHERENT
}
#endif // UMDEVXS_SMBUF_DIRECT_DCACHE_CONTROL


/*----------------------------------------------------------------------------
 * DMABuf_Alloc
 *
 * This implementation supports ignores the Bank field in RequestedProperties
 * and always uses __get_dma_pages to allocate memory.
 * compatible with "IPC Shared Memory". This function always allocates on a
 * 4kB page boundary, so the size must be a multiple of 4kB.
 */
static DMABuf_Status_t
DMABuf_Alloc(
        const DMABuf_Properties_t RequestedProperties,
        DMABuf_DevAddress_t * const DevAddr_p,
        BufAdmin_Handle_t * const Handle_p)
{
    BufAdmin_Handle_t Handle;
    BufAdmin_Record_t * Rec_p;
    int order;
    phys_addr_t PhysAddr;
#ifndef UMDEVXS_REMOVE_PCI
    UMDevXS_PCIDev_Data_t DevData;
#endif

#ifdef UMDEVXS_SMBUF_CACHE_COHERENT
    dma_addr_t DMAAddr = 0;
#endif

    if (Handle_p == NULL ||
        DevAddr_p == NULL)
    {
        return DMABUF_ERROR_BAD_ARGUMENT;
    }

    PhysAddr = (phys_addr_t)(uintptr_t)DevAddr_p->p;

    LOG_INFO(UMDEVXS_LOG_PREFIX
             "%s: phys addr 0x%p, cached %d, size %d\n",
             __func__,
             (void*)PhysAddr,
             RequestedProperties.fCached,
             RequestedProperties.Size);

    // initialize the output parameters
    *Handle_p = BUFADMIN_HANDLE_NULL;

    DevAddr_p->p = NULL;

    // validate the properties
    if (RequestedProperties.Size == 0)
        return DMABUF_ERROR_BAD_ARGUMENT;

    if ((RequestedProperties.Size & (PAGE_SIZE - 1)) != 0)
        return DMABUF_ERROR_BAD_ARGUMENT;

    // we support up to 32 megabyte buffers
    if (RequestedProperties.Size >= 32*1024*1024)
        return DMABUF_ERROR_BAD_ARGUMENT;

    // let order=n, where n is smallest integer for which:
    // PAGE_SIZE * (2^n) >= RequestedProperties.Size
    order = get_order(RequestedProperties.Size);

    // create a record
    Handle = BufAdmin_Record_Create();
    if (Handle == BUFADMIN_HANDLE_NULL)
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX
                "%s: failed to create a handle\n",
                __func__);

        return DMABUF_ERROR_OUT_OF_MEMORY;
    }

    Rec_p = BufAdmin_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        // panic...
        BufAdmin_Record_Destroy(Handle);
        return DMABUF_ERROR_OUT_OF_MEMORY;
    }

    // allocate the memory
    {
        void * p = NULL;

        if (RequestedProperties.Bank == UMDEVXS_SHMEM_BANK_STATIC_FIXED_ADDR)
        {
            struct resource * Resource_p;

#ifndef UMDEVXS_REMOVE_PCI
            ZEROINIT(DevData);

            UMDevXS_PCIDev_GetReference(&DevData);

            PhysAddr += (phys_addr_t)(uintptr_t)DevData.PhysAddr;
#endif // !UMDEVXS_REMOVE_PCI

            // Any dummy non-NULL address will do,
            // p won't be used for addressing
            if (PhysAddr)
                p = (void *)(uintptr_t)PhysAddr;
            else
                p = (void *)0xDEADBEEF;

            Resource_p = request_mem_region(PhysAddr,
                                            RequestedProperties.Size,
                                            "DMA-bank");
            if (!Resource_p)
            {
                LOG_CRIT(UMDEVXS_LOG_PREFIX
                         "%s: request_mem_region() failed, "
                         "phys addr 0x%p, size %d\n",
                         __func__,
                         (void *)(uintptr_t)PhysAddr,
                         RequestedProperties.Size);

                BufAdmin_Record_Destroy(Handle);
                return DMABUF_ERROR_OUT_OF_MEMORY;
            }

            LOG_INFO(UMDEVXS_LOG_PREFIX
                    "%s: request_mem_region() phys addr 0x%p, size %d\n",
                     __func__,
                     (void*)PhysAddr,
                     RequestedProperties.Size);
        }
        else
        {
            // Use __get_dma_pages() to ensure we get page aligned,
            // DMA-capable memory (i.e. from start of system RAM).
            // Assume this function is  called from sleepable context,
            // so no need to pass the GPF_ATOMIC flag.

#ifdef UMDEVXS_SMBUF_CACHE_COHERENT
            struct device * Device_p = NULL;

            LOG_INFO(UMDEVXS_LOG_PREFIX
                     "%s: non-cached allocation\n", __func__);

#ifndef UMDEVXS_REMOVE_DEVICE_OF
            Device_p = UMDevXS_OFDev_GetReference();
#endif // !UMDEVXS_REMOVE_DEVICE_OF

#ifndef UMDEVXS_REMOVE_PCI
            Device_p = UMDevXS_PCIDev_GetReference(NULL);
#endif // !UMDEVXS_REMOVE_PCI

            p = dma_alloc_coherent(Device_p,
                                   RequestedProperties.Size,
                                   &DMAAddr,
                                   GFP_KERNEL | UMDEVXS_DMA_FLAGS);
#else
            p = (void *)__get_free_pages(GFP_KERNEL |
                                        UMDEVXS_DMA_FLAGS, order);
#endif // UMDEVXS_SMBUF_CACHE_COHERENT

            if (p == NULL)
            {
                LOG_CRIT(UMDEVXS_LOG_PREFIX
                         "%s: __get_dma_pages(%u) failed\n",
                         __func__,
                         order);

                goto DESTROY_HANDLE;
            }
        }

        // sanity-check the alignment
        // perhaps a bit too paranoid, but it does not hurt
        {
            unsigned int n = (unsigned int)(uintptr_t)p;

            if ((n & (PAGE_SIZE - 1)) != 0)
            {
                if (RequestedProperties.Bank !=
                            UMDEVXS_SHMEM_BANK_STATIC_FIXED_ADDR)
                {
#ifdef UMDEVXS_SMBUF_CACHE_COHERENT
                    struct device * Device_p = NULL;

#ifndef UMDEVXS_REMOVE_DEVICE_OF
                    Device_p = UMDevXS_OFDev_GetReference();
#endif // !UMDEVXS_REMOVE_DEVICE_OF

#ifndef UMDEVXS_REMOVE_PCI
                    Device_p = UMDevXS_PCIDev_GetReference(NULL);
#endif // !UMDEVXS_REMOVE_PCI

                    dma_free_coherent(Device_p,
                                      RequestedProperties.Size,
                                      p,
                                      DMAAddr);
#else
                    free_pages((unsigned long)p, order);
#endif // UMDEVXS_SMBUF_CACHE_COHERENT
                }

                LOG_CRIT(UMDEVXS_LOG_PREFIX
                        "%s: failed, %p not aligned to 0x%x\n",
                        __func__,
                        p,
                        (unsigned int)(PAGE_SIZE - 1));

                goto DESTROY_HANDLE;
            }
        }

        // fill in the record fields
        Rec_p->Magic                 = UMDEVXS_DMARESOURCE_MAGIC;

        Rec_p->alloc.AllocatedAddr_p = p;
        Rec_p->alloc.AllocatedSize   = PAGE_SIZE << order;
        Rec_p->alloc.MemoryBank      = RequestedProperties.Bank;

        Rec_p->host.Alignment        = PAGE_SIZE;
        Rec_p->host.HostAddr_p       = Rec_p->alloc.AllocatedAddr_p;
        // note: not the allocated size
        Rec_p->host.BufferSize       = RequestedProperties.Size;
    }

    // set the output parameters
    *Handle_p = Handle;

    if (RequestedProperties.Bank == UMDEVXS_SHMEM_BANK_STATIC_FIXED_ADDR)
    {
        phys_addr_t TempAddr1 = 0, TempAddr2 = 0;

#ifndef UMDEVXS_REMOVE_PCI
        TempAddr1 = (phys_addr_t)(uintptr_t)DevData.PhysAddr;
#endif // !UMDEVXS_REMOVE_PCI

        TempAddr2 = (phys_addr_t)UMDEVXS_SHMEM_BANK_STATIC_OFFSET;

        DevAddr_p->p = (void *)(uintptr_t)(PhysAddr - TempAddr1 - TempAddr2);
    }
    else
    {
        phys_addr_t TempAddr1;

#ifdef UMDEVXS_SMBUF_CACHE_COHERENT
        DevAddr_p->p = (void *)DMAAddr;
#else
        DevAddr_p->p = (void *)(uintptr_t)virt_to_phys(Rec_p->alloc.AllocatedAddr_p);
#endif // UMDEVXS_SMBUF_CACHE_COHERENT

        if ((phys_addr_t)(uintptr_t)DevAddr_p->p > UMDEVXS_SHMEM_BANK_STATIC_OFFSET)
        {
            TempAddr1 = (phys_addr_t)(uintptr_t)DevAddr_p->p -
                            UMDEVXS_SHMEM_BANK_STATIC_OFFSET;
        }
        else
        {
            LOG_CRIT(UMDEVXS_LOG_PREFIX
                     "%s: failed, 0x%p is smaller than 0x%016Lx\n",
                     __func__,
                     DevAddr_p->p,
                     (long long int)UMDEVXS_SHMEM_BANK_STATIC_OFFSET);

            goto DESTROY_HANDLE;
        }

        DevAddr_p->p = (void *)(uintptr_t)TempAddr1;
    }

    Rec_p->alloc.PhysAddr_p = DevAddr_p->p;

    LOG_INFO(UMDEVXS_LOG_PREFIX
             "%s: handle %d, "
             "alloc addr 0x%p, bus addr 0x%p, request size %d\n",
             __func__,
             Handle,
             Rec_p->alloc.AllocatedAddr_p,
             Rec_p->alloc.PhysAddr_p,
             Rec_p->host.BufferSize);

    return DMABUF_STATUS_OK;

DESTROY_HANDLE:

    LOG_CRIT(UMDEVXS_LOG_PREFIX
             "%s: failed to allocate DMA resource\n",
             __func__);

    if (RequestedProperties.Bank == UMDEVXS_SHMEM_BANK_STATIC_FIXED_ADDR)
        release_mem_region(PhysAddr, RequestedProperties.Size);

    BufAdmin_Record_Destroy(Handle);

    return DMABUF_ERROR_OUT_OF_MEMORY;
}


/*----------------------------------------------------------------------------
 * DMABuf_Register
 *
 * This function must be used to register an "alien" buffer that was allocated
 * somewhere else. The caller guarantees that this buffer can be used for DMA.
 *
 * ActualProperties (input)
 *     Properties that describe the buffer that is being registered.
 *
 * Buffer_p (input)
 *     Pointer to the buffer. This pointer must be valid to use on the host
 *     in the domain of the driver.
 *
 * Alternative_p (input)
 *     Some allocators return two addresses. This parameter can be used to
 *     pass this second address to the driver. The type is pointer to ensure
 *     it is always large enough to hold a system address, also in LP64
 *     architecture. Set to NULL if not used.
 *
 * AllocatorRef (input)
 *     Number to describe the source of this buffer. The exact numbers
 *     supported is implementation specific. This provides some flexibility
 *     for a specific implementation to support a number of "alien" buffers
 *     from different allocator and properly interpret and use the
 *     Alternative_p parameter when translating the address to the device
 *     memory map. Set to zero if not used.
 *
 * Handle_p (output)
 *     Pointer to the memory location when the handle will be returned.
 *
 * Return Values
 *     DMABUF_STATUS_OK: Success, Handle_p was written.
 *     DMABUF_ERROR_BAD_ARGUMENT
 */
static DMABuf_Status_t
DMABuf_Register(
        const DMABuf_Properties_t ActualProperties,
        void * Buffer_p,
        void * Alternative_p,
        const char AllocatorRef,
        BufAdmin_Handle_t * const Handle_p)
{
    BufAdmin_Handle_t Handle;
    BufAdmin_Record_t * Rec_p;

    if (Handle_p == NULL)
        return DMABUF_ERROR_BAD_ARGUMENT;

    // initialize the output parameter
    *Handle_p = BUFADMIN_HANDLE_NULL;

    // validate the properties
    if (ActualProperties.Size == 0)
        return DMABUF_ERROR_BAD_ARGUMENT;

    // we support up to 32 megabyte buffers
    if (ActualProperties.Size >= 32*1024*1024)
        return DMABUF_ERROR_BAD_ARGUMENT;

    // create a record
    Handle = BufAdmin_Record_Create();
    if (Handle == BUFADMIN_HANDLE_NULL)
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX
                 "%s: failed to create a handle\n",
                 __func__);

        return DMABUF_ERROR_OUT_OF_MEMORY;
    }

    Rec_p = BufAdmin_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        // panic...
        goto DESTROY_HANDLE;
    }

    // register the memory
    {
        // fill in the record fields
        Rec_p->Magic                 = UMDEVXS_DMARESOURCE_MAGIC;

        Rec_p->alloc.AllocatedAddr_p = Buffer_p;
        Rec_p->alloc.AllocatedSize   = ActualProperties.Size;
        Rec_p->alloc.Alternative_p   = Alternative_p;

        IDENTIFIER_NOT_USED(AllocatorRef);

        //Rec_p->host.fCached = ActualProperties.fCached;

        Rec_p->host.Alignment        = ActualProperties.Alignment;
        Rec_p->host.HostAddr_p       = Rec_p->alloc.AllocatedAddr_p;
        Rec_p->host.BufferSize       = Rec_p->alloc.AllocatedSize;
    }

    // set the output parameters
    *Handle_p = Handle;

    LOG_INFO(UMDEVXS_LOG_PREFIX
             "%s: handle %d, "
             "register addr 0x%p, bus addr 0x%p, request size %d\n",
             __func__,
             Handle,
             Rec_p->alloc.AllocatedAddr_p,
             Rec_p->alloc.PhysAddr_p,
             Rec_p->host.BufferSize);

    return DMABUF_STATUS_OK;

DESTROY_HANDLE:

    LOG_CRIT(UMDEVXS_LOG_PREFIX
             "%s: failed to register DMA resource\n",
             __func__);

    BufAdmin_Record_Destroy(Handle);
    return DMABUF_ERROR_OUT_OF_MEMORY;
}


/*----------------------------------------------------------------------------
 * DMABuf_Release
 *
 * Free the DMA resource (unless not allocated locally) and the record used
 * to describe it.
 *
 */
static DMABuf_Status_t
DMABuf_Release(
        BufAdmin_Handle_t Handle)
{
    if (BufAdmin_IsValidHandle(Handle))
    {
        BufAdmin_Record_t * Rec_p;

        Rec_p = BufAdmin_Handle2RecordPtr(Handle);
        if (Rec_p == NULL)
            return DMABUF_ERROR_INVALID_HANDLE;

        if (Rec_p->alloc.AllocatedAddr_p != NULL)
        {
#ifdef UMDEVXS_SMBUF_CACHE_COHERENT
            LOG_INFO(UMDEVXS_LOG_PREFIX "%s: non-cached release\n", __func__);
#endif

            if (Rec_p->alloc.MemoryBank ==
                    UMDEVXS_SHMEM_BANK_STATIC_FIXED_ADDR)
            {
                phys_addr_t TempAddr = 0;

#ifndef UMDEVXS_REMOVE_PCI
                UMDevXS_PCIDev_Data_t DevData;

                ZEROINIT(DevData);

                UMDevXS_PCIDev_GetReference(&DevData);

                TempAddr = (phys_addr_t)(uintptr_t)DevData.PhysAddr;
#endif // !UMDEVXS_REMOVE_PCI

                TempAddr += UMDEVXS_SHMEM_BANK_STATIC_OFFSET;

                release_mem_region((phys_addr_t)(uintptr_t)
                                   Rec_p->alloc.PhysAddr_p + TempAddr,
                                   Rec_p->host.BufferSize);
            }
            else
            {
#ifdef UMDEVXS_SMBUF_CACHE_COHERENT
                struct device * Device_p = NULL;

#ifndef UMDEVXS_REMOVE_DEVICE_OF
                Device_p = UMDevXS_OFDev_GetReference();
#endif // !UMDEVXS_REMOVE_DEVICE_OF

#ifndef UMDEVXS_REMOVE_PCI
                Device_p = UMDevXS_PCIDev_GetReference(NULL);
#endif // !UMDEVXS_REMOVE_PCI

                dma_free_coherent(Device_p,
                                  Rec_p->host.BufferSize,
                                  Rec_p->alloc.AllocatedAddr_p,
                                  (dma_addr_t)Rec_p->alloc.PhysAddr_p +
                                          UMDEVXS_SHMEM_BANK_STATIC_OFFSET);
#else
                free_pages((unsigned long)Rec_p->alloc.AllocatedAddr_p,
                           get_order(Rec_p->alloc.AllocatedSize));
#endif // UMDEVXS_SMBUF_CACHE_COHERENT
            }

            LOG_INFO(UMDEVXS_LOG_PREFIX
                     "%s: handle %d, alloc addr 0x%p, bus addr 0x%p\n",
                     __func__,
                     Handle,
                     Rec_p->alloc.AllocatedAddr_p,
                     Rec_p->alloc.PhysAddr_p);

            Rec_p->alloc.AllocatedAddr_p = NULL;
        }

        Rec_p->Magic = 0;

        BufAdmin_Record_Destroy(Handle);

        return DMABUF_STATUS_OK;
    }

    LOG_CRIT(UMDEVXS_LOG_PREFIX
             "%s: failed, invalid handle %d\n",
             __func__,
             Handle);

    return DMABUF_ERROR_INVALID_HANDLE;
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_PreDMA
 */
static void
HWPAL_DMAResource_PreDMA(
        BufAdmin_Handle_t Handle,
        const unsigned int ByteOffset,
        const unsigned int ByteCount)
{
#ifdef UMDEVXS_DCACHE_CTRL_USERMODE
    IDENTIFIER_NOT_USED(Handle);
    IDENTIFIER_NOT_USED(ByteOffset);
    IDENTIFIER_NOT_USED(ByteCount);

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: dummy invoked, do nothing\n", __func__);
#else
    BufAdmin_Record_t * Rec_p;
    char * cpu_addr;
    size_t size;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: invoked\n", __func__);

    Rec_p = BufAdmin_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        LOG_WARN(
            "HWPAL_DMAResource_PreDMA: "
            "Invalid handle %d\n",
            Handle);

        return;
    }

    // flush_cache_range needs virtual address
    cpu_addr = ((char *)Rec_p->user.Addr) + ByteOffset;
    if (ByteCount == 0)
    {
        size = Rec_p->host.BufferSize;

        LOG_INFO(UMDEVXS_LOG_PREFIX
                 "HWPAL_DMAResource_PreDMA: "
                 "Handle=%d, "
                 "Range=ALL (%u-%u)\n",
                 Handle,
                 ByteOffset,
                 (unsigned int)(ByteOffset + size - 1));
    }
    else
    {
        size = ByteCount;

        LOG_INFO(UMDEVXS_LOG_PREFIX
                "HWPAL_DMAResource_PreDMA: "
                "Handle=%d, "
                "Range=%u-%u\n",
                Handle,
                ByteOffset,
                (unsigned int)(ByteOffset + size - 1));
    }
    LOG_INFO(UMDEVXS_LOG_PREFIX
             "HWPAL_DMAResource_PreDMA: addr=%p, size=0x%08x\n",
             cpu_addr,
             (unsigned int)size);

#ifdef UMDEVXS_SMBUF_DIRECT_DCACHE_CONTROL
    HWPAL_DMAResource_DCache_Clean(cpu_addr, size);
#else
    {
#ifdef UMDEVXS_DCACHE_CTRL_UADDR
        struct vm_area_struct * vma;

        vma = find_vma(current->mm, (unsigned long)cpu_addr);
        if (vma)
        {
             flush_cache_range(
                     vma,
                     (unsigned long)cpu_addr,
                     ((unsigned long)cpu_addr)+size);
        }
        else
        {
             LOG_WARN(
                 "HWPAL_DMAResource_PreDMA: "
                 "find_vma failed for VM address %p", cpu_addr);
        }
#else
        {
            unsigned long start, end;

            start = (unsigned long)Rec_p->user.Addr + ByteOffset;
            end = start + size;

            flush_dcache_range(start, end);

            LOG_INFO(UMDEVXS_LOG_PREFIX
                     "HWPAL_DMAResource_PostDMA: "
                     "addr=0x%016lx, end=0x%016lx\n",
                     start,
                     end);
        }
#endif // UMDEVXS_DCACHE_CTRL_UADDR
    }
#endif // UMDEVXS_SMBUF_DIRECT_DCACHE_CONTROL
    IDENTIFIER_NOT_USED(size);
#endif // UMDEVXS_DCACHE_CTRL_USERMODE
}


/*----------------------------------------------------------------------------
 * UMDevXS_DMABuf_SetAppID
 */
static void
UMDevXS_DMABuf_SetAppID(
        BufAdmin_Handle_t Handle,
        void * AppID)
{
    BufAdmin_Record_t * Rec_p;

    Rec_p = BufAdmin_Handle2RecordPtr(Handle);
    if (Rec_p)
        Rec_p->AppID = AppID;
}


/*----------------------------------------------------------------------------
 * UMDevXS_DMABufCleanUp_EnumFunc
 *
 * This function is called for each registered DMA buffer. We check the
 * AppID and release the buffer (and free the record) if no longer required.
 */
static void
UMDevXS_DMABufCleanUp_EnumFunc(
        BufAdmin_Handle_t Handle,
        BufAdmin_Record_t * const Rec_p,
        void * AppID)
{
    if (Rec_p->AppID == AppID)
    {
        LOG_WARN(
            "Cleaning up Handle=%d (0x%x)\n",
                Handle,
                UMDevXS_Handle_Make(UMDEVXS_HANDLECLASS_SMBUF,  (int)Handle));

        DMABuf_Release(Handle);
    }
}


/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_CleanUp
 *
 * This function looks up all SMBuf handles with the given AppID and frees
 * these by calling DMABuf_Release for each handle.
 */
void
UMDevXS_SMBuf_CleanUp(
        void * AppID)
{
    LOG_INFO(UMDEVXS_LOG_PREFIX
             "UMDevXS_SMBuf_CleanUp() invoked\n");

    BufAdmin_Enumerate(UMDevXS_DMABufCleanUp_EnumFunc, AppID);
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_PostDMA
 */
static void
HWPAL_DMAResource_PostDMA(
        BufAdmin_Handle_t Handle,
        const unsigned int ByteOffset,
        const unsigned int ByteCount)
{
#ifdef UMDEVXS_DCACHE_CTRL_USERMODE
    IDENTIFIER_NOT_USED(Handle);
    IDENTIFIER_NOT_USED(ByteOffset);
    IDENTIFIER_NOT_USED(ByteCount);

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: dummy invoked, do nothing\n", __func__);
#else
    BufAdmin_Record_t * Rec_p;
    char * cpu_addr;
    size_t size;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: invoked\n", __func__);

    Rec_p = BufAdmin_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        LOG_WARN(
            "HWPAL_DMAResource_PostDMA: "
            "Invalid handle %d\n",
            Handle);

        return;
    }

    // dma_sync_single_for_cpu wants the bus address
    cpu_addr = ((char *)Rec_p->user.Addr) + ByteOffset;
    if (ByteCount == 0)
    {
        size = Rec_p->user.Size;

        LOG_INFO(UMDEVXS_LOG_PREFIX
                "HWPAL_DMAResource_PostDMA: "
                "Handle=%d, "
                "Range=ALL (%u-%u)\n",
                Handle,
                ByteOffset,
                (unsigned int)(ByteOffset + size - 1));
    }
    else
    {
         size = ByteCount;

         LOG_INFO(UMDEVXS_LOG_PREFIX
                  "HWPAL_DMAResource_PostDMA: "
                  "Handle=%d, "
                  "Range=%u-%u\n",
                  Handle,
                  ByteOffset,
                  (unsigned int)(ByteOffset + size - 1));
    }
    LOG_INFO(UMDEVXS_LOG_PREFIX
             "HWPAL_DMAResource_PostDMA: "
             "addr=%p, size=0x%08x\n",
             cpu_addr,
             (unsigned int)size);

#ifdef UMDEVXS_SMBUF_DIRECT_DCACHE_CONTROL
    HWPAL_DMAResource_DCache_Invalidate(cpu_addr, size);
#else
    {
#ifdef UMDEVXS_DCACHE_CTRL_UADDR
        struct vm_area_struct * vma;

        vma = find_vma(current->mm, (unsigned long)cpu_addr);
        if (vma)
        {
             flush_cache_range(
                     vma,
                     (unsigned long)cpu_addr,
                     ((unsigned long)cpu_addr)+size);
        }
        else
        {
             LOG_WARN(
                 "HWPAL_DMAResource_PostDMA: "
                 "find_vma failed for VM address %p", cpu_addr);
        }
#else
        {
            unsigned long start, end;

            start = (unsigned long)Rec_p->user.Addr + ByteOffset;
            end = start + size;

            flush_dcache_range(start, end);

            LOG_INFO(UMDEVXS_LOG_PREFIX
                     "HWPAL_DMAResource_PostDMA: "
                     "addr=0x%016lx, end=0x%016lx\n",
                     start,
                     end);
        }
#endif // UMDEVXS_DCACHE_CTRL_UADDR
    }
#endif // UMDEVXS_SMBUF_DIRECT_DCACHE_CONTROL
    IDENTIFIER_NOT_USED(size);
#endif // UMDEVXS_DCACHE_CTRL_USERMODE
}


/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_Map
 */
int
UMDevXS_SMBuf_Map(
        int HandleIndex,
        unsigned int Length,
        struct vm_area_struct * vma_p)
{
    BufAdmin_Handle_t Handle = HandleIndex;
    BufAdmin_Record_t * Rec_p;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s entered\n", __func__);

    LOG_INFO(UMDEVXS_LOG_PREFIX
             "%s: Map DMA buffer handle=%u, size=%d (0x%08x)\n",
             __func__,
             HandleIndex,
             Length,
             Length);

    Rec_p = BufAdmin_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
        return -1;

    // reject oversize mapping request
    if (Length > Rec_p->alloc.AllocatedSize)
        return -2;

    // now map the region into the application memory space
    {
        phys_addr_t StartOfs = 0;
        int ret;

        // if the memory was allocated locally, derive the physical address
        // from Rec_p->alloc.AllocatedAddr_p, the kernel virtual address
        // returned by __get_dma_pages.
        // if the memory was foreign allocated, use the device address
        // that was passed via the SMBUF_ATTACH service.
        if (Rec_p->alloc.AllocatedAddr_p != NULL)
        {
            if (Rec_p->alloc.MemoryBank ==
                    UMDEVXS_SHMEM_BANK_STATIC_FIXED_ADDR)
            {
#ifndef UMDEVXS_REMOVE_PCI
                UMDevXS_PCIDev_Data_t DevData;

                ZEROINIT(DevData);

                UMDevXS_PCIDev_GetReference(&DevData);

                StartOfs = (phys_addr_t)(uintptr_t)DevData.PhysAddr;
#endif // !UMDEVXS_REMOVE_PCI

                StartOfs += UMDEVXS_SHMEM_BANK_STATIC_OFFSET;

                StartOfs += (phys_addr_t)(uintptr_t)Rec_p->alloc.PhysAddr_p;
            }
            else
            {
#ifdef UMDEVXS_SMBUF_CACHE_COHERENT
                StartOfs = (phys_addr_t)Rec_p->alloc.PhysAddr_p;
#else
                StartOfs = (phys_addr_t)virt_to_phys(
                                    Rec_p->alloc.AllocatedAddr_p);
#endif // UMDEVXS_SMBUF_CACHE_COHERENT
            }
        }
        else
        {
             StartOfs = (phys_addr_t)(uintptr_t)Rec_p->alloc.Alternative_p;
        }

        LOG_INFO(UMDEVXS_LOG_PREFIX
                 "UMDevXS_SMBuf_Map: handle %d, "
                 "Start=0x%p, Size=0x%08x, Addr=0x%lx\n",
                 Handle,
                 (void*)StartOfs,
                 Length,
                 vma_p->vm_start);

        if ((StartOfs & (PAGE_SIZE - 1)) != 0)
            return -4;

#if defined(UMDEVXS_SMBUF_UNCACHED_MAPPING) && !defined(UMDEVXS_ARCH_COHERENT)
        LOG_INFO(UMDEVXS_LOG_PREFIX
                 "%s: request non-cached mapping\n", __func__);

        // On some host HW non-cached memory mapping appears to be unavailable.
        // but hardware cache coherency works, so the code runs fine
        // with cached memory.
        vma_p->vm_flags |= VM_IO;
        vma_p->vm_page_prot = pgprot_noncached(vma_p->vm_page_prot);
#endif // UMDEVXS_SMBUF_UNCACHED_MAPPING && !ARCH_X86

        // map the whole physically contiguous area in one piece
        ret = remap_pfn_range(vma_p,
                              vma_p->vm_start,
                              StartOfs >> PAGE_SHIFT,
                              Length,
                              vma_p->vm_page_prot);
        if (ret < 0)
        {
            LOG_CRIT(UMDEVXS_LOG_PREFIX
                     "UMDevXS_SMBuf_Map: io_remap_pfn_range() failed, "
                     "error %d\n",
                     ret);

            return -5;
        }
    }

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: left\n", __func__);

    return 0;       // 0 = success
}


/*----------------------------------------------------------------------------
 * UMDevXSLib_SMBuf_HandleCmd_Alloc
 *
 * This function handles the ALLOC command.
 */
static void
UMDevXSLib_SMBuf_HandleCmd_Alloc(
        void * AppID,
        UMDevXS_CmdRsp_t * const CmdRsp_p)
{
    BufAdmin_Handle_t Handle;
    DMABuf_DevAddress_t DevAddr;
    BufAdmin_Record_t * Rec_p;
    unsigned int Size;
    unsigned int Alignment;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: entered\n", __func__);

    if (CmdRsp_p == NULL)
        return;

    // get the size and round it up to a multiple of PAGE_SIZE
    Size = CmdRsp_p->uint1;
    Size = (Size + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1);

    // this implementation ignores the Bank argument

    // get alignment and reject it if > PAGE_SIZE or not a power of 2
    Alignment = CmdRsp_p->uint3;
    if ((Alignment < 1) ||
        (Alignment > PAGE_SIZE) ||
        (Alignment != (Alignment & (-Alignment))))
    {
        LOG_WARN(
            UMDEVXS_LOG_PREFIX
            "UMDevXSLib_SMBuf_HandleCmd_Alloc: "
            "Invalid alignment (%d)\n",
            Alignment);

        CmdRsp_p->Error = 1;
        return;
    }

    {
        DMABuf_Properties_t Props;
        DMABuf_Status_t dmares;

        ZEROINIT(Props);

        Props.Size      = Size;
        Props.Alignment = Alignment;
        Props.Bank      = CmdRsp_p->uint2;

        DevAddr.p = CmdRsp_p->ptr1;

        dmares = DMABuf_Alloc(Props, &DevAddr, &Handle);
        if (dmares != DMABUF_STATUS_OK)
        {
            LOG_WARN(
                UMDEVXS_LOG_PREFIX
                "UMDevXSLib_SMBuf_HandleCmd_Alloc: "
                "DMABuf_Alloc returned %d\n",
                dmares);

            CmdRsp_p->Error = 1;
            return;
        }
    }

    // link it to this application
    UMDevXS_DMABuf_SetAppID(Handle, AppID);

    // populate the output parameters
    CmdRsp_p->Handle = UMDevXS_Handle_Make(UMDEVXS_HANDLECLASS_SMBUF,
                                           (int)Handle);

    // get actually allocated size
    Rec_p = BufAdmin_Handle2RecordPtr(Handle);
    if (Rec_p)
    {
        CmdRsp_p->uint1 = Rec_p->host.BufferSize;
        // alternatively, the possibly bigger Rec_p->alloc.ActualSize
        // could be used here; in that case, the Length check at the start
        // of UMDevXS_SMBuf_Map must be changed accordingly.
    }
    // In the extremely unlikely event that Rec_p is NULL here, the
    // requested size is returned as actual size

    CmdRsp_p->ptr1 = DevAddr.p;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: left\n", __func__);
}


/*----------------------------------------------------------------------------
 * UMDevXSLib_SMBuf_HandleCmd_Register
 *
 * This function handles the REGISTER command.
 */
static void
UMDevXSLib_SMBuf_HandleCmd_Register(
        void * AppID,
        UMDevXS_CmdRsp_t * const CmdRsp_p)
{
    BufAdmin_Handle_t Handle;
    BufAdmin_Handle_t RetHandle;
    BufAdmin_Record_t * Rec_p = NULL;
    unsigned int Size;
    void * BufPtr;
    unsigned long ParentBuf_HostAddr,
                  RegisteredBuf_HostAddr,
                  ParentBuf_PhysAddr;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: entered\n", __func__);

    if (CmdRsp_p == NULL)
        return;

    Handle = UMDevXS_Handle_GetIndex(CmdRsp_p->Handle);
    Size = CmdRsp_p->uint1;
    BufPtr = CmdRsp_p->ptr1;

    if (BufAdmin_IsValidHandle(Handle))
    {
        Rec_p = BufAdmin_Handle2RecordPtr(Handle);
    }

    if (Rec_p == NULL)
    {
        LOG_WARN(
            UMDEVXS_LOG_PREFIX
            "UMDevXSLib_SMBuf_HandleCmd_Register: "
            "Invalid handle %d\n",
            Handle);

        CmdRsp_p->Error = 1;
        return;
    }

    // check if Handle refers to a buffer that is already allocated or
    // attached by this application:
    if ((Rec_p->AppID != AppID) ||
        ((Rec_p->alloc.AllocatedAddr_p == NULL) &&
         (Rec_p->alloc.Alternative_p == NULL)) ||
        (Rec_p->user.Addr == NULL))
    {
        LOG_WARN(
            UMDEVXS_LOG_PREFIX
            "UMDevXSLib_SMBuf_HandleCmd_Register: "
            "Register denied, bad parent buffer %p\n",
            Rec_p);

        CmdRsp_p->Error = 1;
        return;
    }

    if ((Size > Rec_p->user.Size) ||
        (BufPtr < Rec_p->user.Addr) ||
        (BufPtr + Size > Rec_p->user.Addr + Rec_p->user.Size))
    {
        LOG_WARN(
            UMDEVXS_LOG_PREFIX
            "UMDevXSLib_SMBuf_HandleCmd_Register: "
            "Out-of-range address (%p) or size (%u)\n",
            BufPtr,
            Size);

        CmdRsp_p->Error = 1;
        return;
    }

    ParentBuf_PhysAddr = (unsigned long)Rec_p->alloc.PhysAddr_p;
    ParentBuf_HostAddr = (unsigned long)Rec_p->user.Addr;
    RegisteredBuf_HostAddr = (unsigned long)BufPtr;

    // register buffer and get the handle
    {
        DMABuf_Properties_t Props;
        DMABuf_Status_t dmares;

        ZEROINIT(Props);

        Props.Size = Size;
        Props.Bank = Rec_p->alloc.MemoryBank;

        // Pass NULL addresses to indicate this record is for a registered
        // buffer
        dmares = DMABuf_Register(
                            Props,
                            NULL,
                            /*Alternative_p:*/NULL,
                            /*AllocatorRef:*/0,
                            &RetHandle);

        if (dmares != DMABUF_STATUS_OK)
        {
            LOG_WARN(
                UMDEVXS_LOG_PREFIX
                "UMDevXSLib_SMBuf_HandleCmd_Register: "
                "DMABuf_Register returned %d\n",
                dmares);

            CmdRsp_p->Error = 9;
            return;
        }
    }

    // populate the record's .user fields
    Rec_p = BufAdmin_Handle2RecordPtr(RetHandle);
    if (Rec_p == NULL)
    {
        // highly unlikely...
        CmdRsp_p->Error = 2;
        return;
    }

    // Keep these fields zero so that UMDevXSProxy_SHMem_Free
    // can decide not to unmap when freeing registered memory!
    // This implies that registered memory cannot be sub-registered,
    // but this is also already forbidden in DMAResource_CheckAndRegister,
    // see DMAResourceLib_Find_Matching_DMAResource.
    // Rec_p->user.Addr = BufPtr;
    // Rec_p->user.Size = Size;
    Rec_p->user.Addr = BufPtr;
    Rec_p->user.Size = Size;
    Rec_p->alloc.PhysAddr_p = (void*)(ParentBuf_PhysAddr +
                                          RegisteredBuf_HostAddr -
                                                     ParentBuf_HostAddr);

    // link it to this application
    UMDevXS_DMABuf_SetAppID(RetHandle, AppID);

    // populate the output parameters
    CmdRsp_p->Handle = UMDevXS_Handle_Make(
                                UMDEVXS_HANDLECLASS_SMBUF,
                                (int)RetHandle);

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: left\n", __func__);
}

/*----------------------------------------------------------------------------
 * UMDevXSLib_SMBuf_HandleCmd_Free
 *
 * This function handles the FREE command.
 */
static void
UMDevXSLib_SMBuf_HandleCmd_Free(
        void * AppID,
        UMDevXS_CmdRsp_t * const CmdRsp_p)
{
    BufAdmin_Handle_t Handle;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: entered\n", __func__);

    if (CmdRsp_p == NULL)
        return;

    Handle = UMDevXS_Handle_GetIndex(CmdRsp_p->Handle);

#ifdef UMDEVXS_ENABLE_BUFFER_APPCHECK
    if (AppID != UMDevXS_SMBuf_GetAppID(Handle))
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX
                 "UMDevXSLib_SMBuf_HandleCmd_Free: "
                 "Not owner of handle\n");
        return;
    }
#else
    IDENTIFIER_NOT_USED(AppID);
#endif

    DMABuf_Release(Handle);

    CmdRsp_p->Handle = 0;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: left\n", __func__);
}


/*----------------------------------------------------------------------------
 * UMDevXSLib_SMBuf_HandleCmd_Attach
 *
 * This function handles the ATTACH command.
 * It creates a new record to store the passed info and returns a handle
 * for that record.
 * Passed info is: (phys) address (CmdRsp_p->ptr1)
 *                 size           (CmdRsp_p->uint1)
 *                 bank           (CmdRsp_p->uint2)
 * The info is typically used to implement a subsequent mmap request that
 * receives the handle through the `MapOffset' argument of the mmap call...
 */
static void
UMDevXSLib_SMBuf_HandleCmd_Attach(
        void * AppID,
        UMDevXS_CmdRsp_t * const CmdRsp_p)
{
    BufAdmin_Handle_t Handle;
    void * BufAddr;
    unsigned int BufSize;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: invoked\n", __func__);

    if (CmdRsp_p == NULL)
        return;

    BufAddr = CmdRsp_p->ptr1;
    BufSize = CmdRsp_p->uint1;

    // register the buffer to get a handle
    {
        DMABuf_Properties_t Props;
        DMABuf_Status_t dmares;

        ZEROINIT(Props);

        Props.Size = BufSize;
        Props.Bank = CmdRsp_p->uint2;

        // Pass BufAddr as Alternative_p so created resource record
        // is recognized as referring to foreign-allocated memory.
        dmares = DMABuf_Register(
                            Props,
                            NULL,
                            /*Alternative_p:*/BufAddr,
                            /*AllocatorRef:*/0,
                            &Handle);

        if (dmares != DMABUF_STATUS_OK)
        {
            LOG_WARN(
                UMDEVXS_LOG_PREFIX
                "UMDevXSLib_SMBuf_HandleCmd_Attach: "
                "DMABuf_Register returned %d\n",
                dmares);

            CmdRsp_p->Error = 9;
            return;
        }
    }

    // link this resource to this application
    UMDevXS_DMABuf_SetAppID(Handle, AppID);

    // populate the output parameters
    CmdRsp_p->Handle = UMDevXS_Handle_Make(
                                UMDEVXS_HANDLECLASS_SMBUF,
                                (int)Handle);

    CmdRsp_p->uint1 = BufSize;
}


/*----------------------------------------------------------------------------
 * UMDevXSLib_SMBuf_HandleCmd_Detach
 *
 * This function handles the DETACH command.
 */
static void
UMDevXSLib_SMBuf_HandleCmd_Detach(
        void * AppID,
        UMDevXS_CmdRsp_t * const CmdRsp_p)
{
    BufAdmin_Handle_t Handle;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: invoked\n", __func__);

    if (CmdRsp_p == NULL)
        return;

    Handle = UMDevXS_Handle_GetIndex(CmdRsp_p->Handle);

#ifdef UMDEVXS_ENABLE_BUFFER_APPCHECK
    if (AppID != UMDevXS_SMBuf_GetAppID(Handle))
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX
                 "UMDevXSLib_SMBuf_HandleCmd_Free: "
                 "Not owner of handle\n");
        return;
    }
#else
    IDENTIFIER_NOT_USED(AppID);
#endif

    DMABuf_Release(Handle);

    CmdRsp_p->Handle = 0;
}


/*----------------------------------------------------------------------------
 * UMDevXSLib_SMBuf_HandleCmd_SetBufInfo
 *
 * This function handles the SETBUFINFO command.
 */
static void
UMDevXSLib_SMBuf_HandleCmd_SetBufInfo(
        UMDevXS_CmdRsp_t * const CmdRsp_p)
{
    BufAdmin_Handle_t Handle;
    BufAdmin_Record_t * Rec_p;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: invoked\n", __func__);

    if (CmdRsp_p == NULL)
        return;

    Handle = UMDevXS_Handle_GetIndex(CmdRsp_p->Handle);

    Rec_p = BufAdmin_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        CmdRsp_p->Error = 2;
        return;
    }

    Rec_p->user.Addr = CmdRsp_p->ptr1;
    Rec_p->user.Size = CmdRsp_p->uint1;
}


/*----------------------------------------------------------------------------
 * UMDevXSLib_SMBuf_HandleCmd_GetBufInfo
 *
 * This function handles the GETBUFINFO command.
 */
static void
UMDevXSLib_SMBuf_HandleCmd_GetBufInfo(
        UMDevXS_CmdRsp_t * const CmdRsp_p)
{
    BufAdmin_Handle_t Handle;
    BufAdmin_Record_t * Rec_p;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: invoked\n", __func__);

    if (CmdRsp_p == NULL)
        return;

    Handle = UMDevXS_Handle_GetIndex(CmdRsp_p->Handle);

    Rec_p = BufAdmin_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        CmdRsp_p->Error = 2;
        return;
    }

    if (Rec_p->alloc.AllocatedAddr_p == NULL)
        CmdRsp_p->ptr1 = NULL;
    else
        CmdRsp_p->ptr1 = Rec_p->user.Addr;
    CmdRsp_p->uint1 = Rec_p->user.Size;
}



/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_HandleCmd_CommitRefresh
 *
 * This function handles COMMIT and REFRESH.
 */
static void
UMDevXSLib_SMBuf_HandleCmd_CommitRefresh(
        UMDevXS_CmdRsp_t * const CmdRsp_p)
{
    BufAdmin_Handle_t Handle;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: entered\n", __func__);

    if (CmdRsp_p == NULL)
        return;

    Handle = UMDevXS_Handle_GetIndex(CmdRsp_p->Handle);

    if (BufAdmin_IsValidHandle(Handle))
    {
        if (CmdRsp_p->Opcode == UMDEVXS_OPCODE_SMBUF_COMMIT)
        {
            HWPAL_DMAResource_PreDMA(Handle, CmdRsp_p->uint1,
                                        CmdRsp_p->uint2);
            return;
        }

        if (CmdRsp_p->Opcode == UMDEVXS_OPCODE_SMBUF_REFRESH)
        {
            HWPAL_DMAResource_PostDMA(Handle, CmdRsp_p->uint1,
                                        CmdRsp_p->uint2);
            return;
        }
    }

    CmdRsp_p->Error = 3;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: left\n", __func__);
}


/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_HandleCmd
 *
 * This function handles the ALLOC, REGISTER, FREE, ATTACH, DETACH,
 * GET/SETBUFINFO, COMMIT and REFRESH commands.
 */
void
UMDevXS_SMBuf_HandleCmd(
        void * AppID,
        UMDevXS_CmdRsp_t * const CmdRsp_p)
{
    if (CmdRsp_p == NULL)
        return;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: invoked\n", __func__);

    switch(CmdRsp_p->Opcode)
    {
        case UMDEVXS_OPCODE_SMBUF_ALLOC:
            // allocate an appropriate buffer using DMABuf
            UMDevXSLib_SMBuf_HandleCmd_Alloc(AppID, CmdRsp_p);
            break;

        case UMDEVXS_OPCODE_SMBUF_REGISTER:
            // register an already allocated/attached DMA buffer
            UMDevXSLib_SMBuf_HandleCmd_Register(AppID, CmdRsp_p);
            break;

        case UMDEVXS_OPCODE_SMBUF_FREE:
            // free an allocated or registered DMA buffer
            UMDevXSLib_SMBuf_HandleCmd_Free(AppID, CmdRsp_p);
            break;

        case UMDEVXS_OPCODE_SMBUF_ATTACH:
            // attached to a shared memory buffer
            UMDevXSLib_SMBuf_HandleCmd_Attach(AppID, CmdRsp_p);
            break;

        case UMDEVXS_OPCODE_SMBUF_DETACH:
            UMDevXSLib_SMBuf_HandleCmd_Detach(AppID, CmdRsp_p);
            break;

        case UMDEVXS_OPCODE_SMBUF_SETBUFINFO:
            UMDevXSLib_SMBuf_HandleCmd_SetBufInfo(CmdRsp_p);
            break;

        case UMDEVXS_OPCODE_SMBUF_GETBUFINFO:
            UMDevXSLib_SMBuf_HandleCmd_GetBufInfo(CmdRsp_p);
            break;

        case UMDEVXS_OPCODE_SMBUF_COMMIT:
        case UMDEVXS_OPCODE_SMBUF_REFRESH:
            UMDevXSLib_SMBuf_HandleCmd_CommitRefresh(CmdRsp_p);
            break;

        default:
            // unsupported command
            LOG_INFO(
                UMDEVXS_LOG_PREFIX
                "UMDevXS_SMBuf_HandleCmd: "
                "Unsupported opcode: %u\n",
                CmdRsp_p->Opcode);

            CmdRsp_p->Error = 123;
            break;
    } // switch
}


/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_Init
 */
int
UMDevXS_SMBuf_Init(void)
{
    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: invoked\n", __func__);

    {
        struct device * DMADevice_p = NULL;
        int res;

        LOG_INFO(UMDEVXS_LOG_PREFIX
                 "%s: non-cached allocation\n", __func__);

#ifndef UMDEVXS_REMOVE_DEVICE_OF
        DMADevice_p = UMDevXS_OFDev_GetReference();
#endif // !UMDEVXS_REMOVE_DEVICE_OF

#ifndef UMDEVXS_REMOVE_PCI
        DMADevice_p = UMDevXS_PCIDev_GetReference(NULL);
#endif // !UMDEVXS_REMOVE_PCI

        LOG_INFO("%s: Device DMA address mask 0x%016Lx\n",
                 __func__,
                 (long long unsigned int)UMDEVXS_SMBUF_ADDR_MASK);

        // Set DMA mask wider, so the DMA API will not try to bounce
        res = UMDEVXS_SMBUF_SET_MASK(DMADevice_p,
                                     UMDEVXS_SMBUF_ADDR_MASK);
        if ( res != 0)
        {
            LOG_CRIT("%s: failed, "
                     "host does not support DMA address mask 0x%016Lx\n",
                     __func__,
                     (long long unsigned int)UMDEVXS_SMBUF_ADDR_MASK);
            return false;
        }
    }

    // no initialization required, currently
    return 0;
}


/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_UnInit
 */
void
UMDevXS_SMBuf_UnInit(void)
{
    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: invoked\n", __func__);
}


/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_SetAppID
 */
void
UMDevXS_SMBuf_SetAppID(
        int Handle,
        void * AppID)
{
    BufAdmin_Handle_t BufAdminHandle= UMDevXS_Handle_GetIndex(Handle);

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: invoked\n", __func__);

    UMDevXS_DMABuf_SetAppID(BufAdminHandle, AppID);
}

#ifdef UMDEVXS_ENABLE_BUFFER_APPCHECK
/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_GetAppID
 */
void *
UMDevXS_SMBuf_GetAppID(int HandleIndex)
{
    BufAdmin_Handle_t Handle= HandleIndex;
    BufAdmin_Record_t * Rec_p;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: invoked\n", __func__);

    Rec_p = BufAdmin_Handle2RecordPtr(Handle);
    if (Rec_p)
    {
        return Rec_p->AppID;
    }
    else
    {
        return NULL;
    }
}
#endif


/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_Register
 *
 * This function must be used to register an "alien" buffer that was allocated
 * somewhere else. The caller guarantees that this buffer can be used for DMA.
 *
 * Size (input)
 *     Size of the buffer to be registered (in bytes).
 *
 * Buffer_p (input)
 *     Pointer to the buffer. This pointer must be valid to use on the host
 *     in the domain of the driver.
 *
 * Alternative_p (input)
 *     Some allocators return two addresses. This parameter can be used to
 *     pass this second address to the driver. The type is pointer to ensure
 *     it is always large enough to hold a system address, also in LP64
 *     architecture. Set to NULL if not used.
 *
 * Handle_p (output)
 *     Pointer to the memory location when the handle will be returned.
 *
 * Return Values
 *     DMABUF_STATUS_OK: Success, Handle_p was written.
 *     DMABUF_ERROR_BAD_ARGUMENT
 */
int
UMDevXS_SMBuf_Register(
        const int Size,
        void * Buffer_p,
        void * Alternative_p,
        int * const Handle_p)
{
    DMABuf_Properties_t Props;
    BufAdmin_Handle_t RetHandle;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: entered\n", __func__);

    ZEROINIT(Props);

    Props.Size = Size;
    Props.Bank = 0;

    if (DMABuf_Register(Props,
                        Buffer_p,
                        Alternative_p,
                        0, // AllocatorRef
                        &RetHandle) != 0)
    {
        return -1;
    }
    else
    {
        *Handle_p = UMDevXS_Handle_Make(
            UMDEVXS_HANDLECLASS_SMBUF,
            (int)RetHandle);
        return 0;
    }

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: left\n", __func__);
}


/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_Release
 *
 * Free the DMA resource (unless not allocated locally) and the record used
 * to describe it.
 *
 */
int
UMDevXS_SMBuf_Release(
    int Handle)
{
    BufAdmin_Handle_t BufAdminHandle= UMDevXS_Handle_GetIndex(Handle);

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: invoked\n", __func__);

    if (DMABuf_Release(BufAdminHandle) != 0)
        return -1;
    else
        return 0;
}



#endif /* UMDEVXS_REMOVE_SMBUF */


/* end of file umdevxs_smbuf.c */
