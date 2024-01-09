/* dmares_gen.c
 *
 * This Module implements the Generic DMA Resource API
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

#include "dmares_mgmt.h"
#include "dmares_buf.h"
#include "dmares_addr.h"
#include "dmares_rw.h"

// helper functions, not part of the actual DMAResource API
#include "dmares_gen.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_dmares_gen.h"

#include "device_swap.h"        // Device_SwapEndian32

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint32_t, NULL, inline, bool,
                                // IDENTIFIER_NOT_USED

// Driver Framework C Run-Time Library Abstraction API
#include "clib.h"               // ZEROINIT()

// Logging API
#include "log.h"                // LOG_*

#ifdef HWPAL_DMARESOURCE_BANKS_ENABLE
// List API
#include "list.h"
#endif // HWPAL_DMARESOURCE_BANKS_ENABLE

// HW- and OS-specific abstraction API
#include "dmares_hwpal.h"

#ifdef HWPAL_DMARESOURCE_USE_UMDEVXS_DEVICE
#ifdef HWPAL_DMARESOURCE_BANKS_ENABLE
// Shared Device Access DMA Pool API
#include "shdevxs_dmapool.h"
#endif
#endif // HWPAL_DMARESOURCE_USE_UMDEVXS_DEVICE


/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#define DMARES_MAX_SIZE     (1 * 1024 * 1024) // 1 MB

#ifdef HWPAL_DMARESOURCE_BANKS_ENABLE
/*
 * Static (fixed-size) DMA banks properties implemented here:
 *
 * - One static bank contains one DMA pool;
 *
 * - Static bank types:
 *     1) HWPAL_DMARESOURCE_BANK_STATIC - dynamically allocated
 *     2) HWPAL_DMARESOURCE_BANK_STATIC_FIXED_ADDR - allocated on a fixed
 *        address configured in c_dmares_gen.h
 *
 * - One DMA Pool contains a fixed compile-time configurable number of blocks;
 *
 * - All blocks in one DMA pool have the same fixed compile-time configurable
 *   size;
 *
 * - The DMA pools for all the configured static banks are allocated
 *   in DMAResource_Init() and freed in DMAResource_Uninit();
 *
 * - DMA resources can be allocated in a static bank using DMAResource_Alloc()
 *   and they must be freed using DMAResource_Release();
 *
 * - Only sub-sets of DMA resources allocated in a static bank can be
 *   registered in that bank using DMAResource_CheckAndRegister();
 *   If the DMAResource_CheckAndRegister() function is called for a static bank
 *   then it must use allocator type 82 ('R') and the required memory block
 *   must belong to an already allocated DMA resource in that bank;
 *
 * - The DMAResource_CheckAndRegister() function can be called for a static
 *   bank also using allocator type 78 ('N') to register a DMA-unsafe buffer;
 *   These DMA resources must be subsequently freed using the DMABuf_Release()
 *   function;
 *
 * - An "all-pool" DMA resource of size (nr_of_blocks * block_size) can be
 *   allocated in a static bank using DMAResource_Alloc() where nr_of_blocks
 *   and block_size are compile-time configuration parameters
 *   (see HWPAL_DMARESOURCE_BANKS in c_dmares_gen.h);
 *   The DMAResource_CheckAndRegister() function can be used to register
 *   sub-sets of this DMA resource; Only one such a all-pool DMA resource
 *   can be allocated in one static bank and must be freed using
 *   DMAResource_Release() function;
 *
 * - No other DMA resources can be allocated in a static bank where an all-pool
 *   DMA resource is allocated.
 */
typedef struct
{
    // Bank ID corresponding to DMA pool
    uint8_t Bank;

    // Bank type, see HWPAL_DMARESOURCE_BANK_*
    unsigned int BankType;

    // True if bank must be shared between multiple concurrent applications
    bool fAppsShared;

    // True if bank must be allocated in cached memory.
    // Note: implementation may still chose to allocate it
    //       in the non-cached memory.
    bool fCached;

    // For static HWPAL_DMARESOURCE_BANK_STATIC_FIXED_ADDR bank types only,
    // otherwise ignored.
    // Note: by default physical addresses are used.
    void * Addr; // bank address

    // List ID for elements associated with free blocks (DMA resources)
    // in DMA pool
    unsigned int PoolListID;

    // List ID for used dangling elements which are not associated with
    // free blocks in DMA pool
    unsigned int DanglingListID;

    // Pointer to array of elements which can be present either
    // in the Pool list or in the Dangling list
    // (but not in both at the same time)
    List_Element_t * ListElements;

    // Number of blocks in DMA pool
    unsigned int BlockCount;

    // Block size (in bytes)
    unsigned int BlockByteCount;

    // DMA pool handle
    DMAResource_Handle_t PoolHandle;

    // Pointer to a list lock for concurrent access protection,
    // used for both lists
    void * Lock_p;

    // When true no more DMA resource can be allocated in the bank's DMA Pool
    bool fDMAPoolLocked;

} HWPAL_DMAResource_Bank_t;

#define HWPAL_DMARESOURCE_BANK_ADD(_bank, _type, _shared, _cached, _addr, _blocks, _bsize) \
               {_bank, _type, _shared, _cached, (void*)(_addr), 0, 0, NULL, _blocks, _bsize, NULL, NULL, false}

#endif // HWPAL_DMARESOURCE_BANKS_ENABLE


/*----------------------------------------------------------------------------
 * Local variables
 */

#ifdef HWPAL_DMARESOURCE_BANKS_ENABLE
static HWPAL_DMAResource_Bank_t HWPAL_DMA_Banks[] =
{
    HWPAL_DMARESOURCE_BANKS
};

// Number of DMA pools supported calculated on HWPAL_DMA_Banks defined
// in c_dmares_lkm.h
#define HWPAL_DMARESOURCE_BANK_COUNT \
        (sizeof(HWPAL_DMA_Banks) / sizeof(HWPAL_DMAResource_Bank_t))
#endif // HWPAL_DMARESOURCE_BANKS_ENABLE

// except when x is zero,
// (x & (-x)) returns a value where all bits of `x' have
// been cleared except the right-most '1'
#define IS_POWER_OF_TWO(_x) (((_x) & (0 - (_x))) == (_x))


/*----------------------------------------------------------------------------
 * Forward declarations
 */

static inline void write32_volatile(uint32_t b, volatile void *addr)
{
    *(volatile uint32_t *) addr = b;
}

static inline uint32_t read32_volatile(const volatile void *addr)
{
    return *(const volatile uint32_t *) addr;
}


/*----------------------------------------------------------------------------
 * DMAResourceLib_IsSaneInput
 *
 * Return true if the DMAResource defined by the given address pair
 * and properties appears to be valid.
 */
/* static */ bool
DMAResourceLib_IsSaneInput(
        const DMAResource_AddrPair_t * AddrPair_p,
        const char * const AllocatorRef_p,
        const DMAResource_Properties_t * Props_p)
{
    unsigned int Alignment = (unsigned int)Props_p->Alignment;

    if ((Alignment < 1) ||
        (Alignment > HWPAL_DMAResource_MaxAlignment_Get()) ||
        !IS_POWER_OF_TWO(Alignment))
    {
        LOG_CRIT(
            "DMAResourceLib_IsSaneInput: "
            "Bad alignment value: %d\n",
            Alignment);
        return false;
    }

    // we support up to 1 megabyte buffers
    if(Props_p->Size == 0 ||
       Props_p->Size >= DMARES_MAX_SIZE)
    {
        LOG_CRIT(
            "DMAResourceLib_IsSaneInput: "
            "Bad size value: %d\n",
            Props_p->Size);
        return false;
    }

    if (AddrPair_p != NULL)
    {
        uintptr_t Address = (unsigned int)(uintptr_t)AddrPair_p->Address_p;

        // Reject NULL as address
        if (Address == 0)
        {
            LOG_CRIT(
                "DMAResourceLib_IsSaneInput: "
                "Bad address: %p\n",
                AddrPair_p->Address_p);
            return false;
        }

        // If requested verify if address is consistent with alignment
        // Skip address alignment check for non-DMA safe buffers
        if (AllocatorRef_p != NULL && *AllocatorRef_p != 'N')
        {
            if ((Address & (Alignment-1)) != 0)
            {
                LOG_CRIT(
                    "DMAResourceLib_IsSaneInput: "
                    "Address (%p) alignment (0x%x bytes) check failed\n",
                    AddrPair_p->Address_p,
                    Alignment);
                return false;
            }
        }
    }

    return true;
}


/*----------------------------------------------------------------------------
 * DMAResourceLib_AlignForSize
 */
/* static */ unsigned int
DMAResourceLib_AlignForSize(
        const unsigned int ByteCount,
        const unsigned int AlignTo)
{
    unsigned int AlignedByteCount = ByteCount;

    // Check if alignment and padding for length alignment is required
    if (AlignTo > 1 && ByteCount % AlignTo)
        AlignedByteCount = ByteCount / AlignTo * AlignTo + AlignTo;

    return AlignedByteCount;
}


/*----------------------------------------------------------------------------
 * DMAResourceLib_AlignForAddress
 */
/* static */ unsigned int
DMAResourceLib_AlignForAddress(
        const unsigned int ByteCount,
        const unsigned int AlignTo)
{
    unsigned int AlignedByteCount = ByteCount;

    // Check if alignment is required
    if(AlignTo > 1)
    {
        // Speculative padding for address alignment
        AlignedByteCount += AlignTo;
    }

    return AlignedByteCount;
}


/*----------------------------------------------------------------------------
 * DMAResourceLib_LookupDomain
 *
 * Lookup given domain in Rec_p->AddrPairs array.
 */
/* static */ DMAResource_AddrPair_t *
DMAResourceLib_LookupDomain(
        const DMAResource_Record_t * Rec_p,
        const DMAResource_AddrDomain_t Domain)
{
    const DMAResource_AddrPair_t * res = Rec_p->AddrPairs;

    while (res->Domain != Domain)
    {
        if (res->Domain == 0)
        {
            return NULL;
        }

        if (++res == Rec_p->AddrPairs + DMARES_ADDRPAIRS_CAPACITY)
        {
            return NULL;
        }
    }

    {
        // Return 'res' but drop the 'const' qualifier
        DMAResource_AddrPair_t * rv = (DMAResource_AddrPair_t *)((uintptr_t)res);
        return rv;
    }
}


#ifdef HWPAL_DMARESOURCE_BANKS_ENABLE
/*----------------------------------------------------------------------------
 * DMAResourceLib_DMAPool_Bank_Get
 */
static HWPAL_DMAResource_Bank_t *
DMAResourceLib_DMAPool_Bank_Get(
        const uint8_t Bank)
{
    unsigned int i, BankCount = HWPAL_DMARESOURCE_BANK_COUNT;

    // we only support pre-configured memory banks
    for(i = 0; i < BankCount; i++)
    {
        if (HWPAL_DMA_Banks[i].Bank == Bank)
            return &HWPAL_DMA_Banks[i];
    }

    return NULL;
}


/*----------------------------------------------------------------------------
 * DMAResourceLib_DMAPool_Lock
 *
 * Checks if no memory blocks have already been obtained from
 * the DMA pool of the provided bank.
 *
 * Returns true and locks the bank when no dangling blocks are found.
 * Returns false if the pool is locked or if dangling blocks are found.
 */
static inline bool
DMAResourceLib_DMAPool_Lock(
        HWPAL_DMAResource_Bank_t * const Bank_p)
{
    List_Status_t List_Rc;
    unsigned long Flags = 0;
    unsigned int ElementCount = 1;

    // DMA pool lock flag access protected by the bank lock
    HWPAL_DMAResource_Lock_Acquire(Bank_p->Lock_p, &Flags);

    if (Bank_p->fDMAPoolLocked == false)
    {
        List_Rc = List_GetListElementCount(Bank_p->DanglingListID,
                                           NULL,
                                           &ElementCount);
        if (List_Rc != LIST_STATUS_OK)
        {
            HWPAL_DMAResource_Lock_Release(Bank_p->Lock_p, &Flags);

            LOG_CRIT("DMAResourceLib_DMAPool_IsAllocated: failed for list %d\n",
                     Bank_p->DanglingListID);

            return false;
        }

        if (ElementCount == 0)
            Bank_p->fDMAPoolLocked = true;
    }

    HWPAL_DMAResource_Lock_Release(Bank_p->Lock_p, &Flags);

    if (ElementCount == 0)
        return true;
    else
        return false;
}


/*----------------------------------------------------------------------------
 * DMAResourceLib_DMAPool_Unlock
 *
 * Unlocks the DMA pool of the provided bank so that blocks
 * can be obtained from it using DMAResourceLib_DMAPool_Get().
 */
static bool
DMAResourceLib_DMAPool_Unlock(
        HWPAL_DMAResource_Bank_t * const Bank_p)
{
    bool fSuccess = false;
    unsigned long Flags = 0;

    // DMA pool lock flag access protected by the bank lock
    HWPAL_DMAResource_Lock_Acquire(Bank_p->Lock_p, &Flags);

    if (Bank_p->fDMAPoolLocked)
    {
        Bank_p->fDMAPoolLocked = false;
        fSuccess = true;
    }

    HWPAL_DMAResource_Lock_Release(Bank_p->Lock_p, &Flags);

    return fSuccess;
}


/*----------------------------------------------------------------------------
 * DMAResourceLib_DMAPool_Free
 */
static void
DMAResourceLib_DMAPool_Free(void)
{
    unsigned int i, BankCount = HWPAL_DMARESOURCE_BANK_COUNT;
    bool fFreed = false;

    LOG_INFO("%s: freeing %d DMA banks\n", __func__, BankCount);

    // Free allocated DMA pools
    for(i = 0; i < BankCount; i++)
    {
        if (HWPAL_DMA_Banks[i].BankType == HWPAL_DMARESOURCE_BANK_DYNAMIC)
        {
            LOG_INFO("%s: skipping dynamic bank %d\n",
                     __func__,
                     HWPAL_DMA_Banks[i].Bank);
            continue;
        }

        if (HWPAL_DMA_Banks[i].PoolHandle != NULL)
        {
#ifdef HWPAL_DMARESOURCE_USE_UMDEVXS_DEVICE
            if (HWPAL_DMA_Banks[i].fAppsShared)
            {
                DMAResource_Record_t * Rec_p;

                LOG_INFO("%s: freeing app-shared DMA bank %d\n", __func__, i);

                // free administration resources
                Rec_p = DMAResource_Handle2RecordPtr(HWPAL_DMA_Banks[i].PoolHandle);
                if (Rec_p != NULL)
                    Rec_p->Magic = 0;

                DMAResource_DestroyRecord(HWPAL_DMA_Banks[i].PoolHandle);
            }
            else
#endif // HWPAL_DMARESOURCE_USE_UMDEVXS_DEVICE
            {
                HWPAL_DMAResource_Release(HWPAL_DMA_Banks[i].PoolHandle);
            }

            HWPAL_DMA_Banks[i].PoolHandle = NULL;
            fFreed = true;
        }

        if (HWPAL_DMA_Banks[i].ListElements != NULL)
        {
            HWPAL_DMAResource_MemFree(HWPAL_DMA_Banks[i].ListElements);
            HWPAL_DMA_Banks[i].ListElements = NULL;
            fFreed = true;
        }

        if (HWPAL_DMA_Banks[i].Lock_p != NULL)
        {
            HWPAL_DMAResource_Lock_Free(HWPAL_DMA_Banks[i].Lock_p);
            HWPAL_DMA_Banks[i].Lock_p = NULL;
            fFreed = true;
        }

        if (fFreed)
        {
            LOG_INFO("%s: %d freed static bank %d at offset 0x%p (cached=%d), "
                    "for %d blocks, %d bytes each)\n",
                    __func__,
                    i,
                    HWPAL_DMA_Banks[i].Bank,
                    HWPAL_DMA_Banks[i].Addr,
                    HWPAL_DMA_Banks[i].fCached,
                    HWPAL_DMA_Banks[i].BlockCount,
                    HWPAL_DMA_Banks[i].BlockByteCount);;
        }

        List_Uninit(HWPAL_DMA_Banks[i].PoolListID, NULL);
        List_Uninit(HWPAL_DMA_Banks[i].DanglingListID, NULL);
    } // for

#ifdef HWPAL_DMARESOURCE_USE_UMDEVXS_DEVICE
    SHDevXS_DMAPool_Uninit();
#endif // HWPAL_DMARESOURCE_USE_UMDEVXS_DEVICE
}


/*----------------------------------------------------------------------------
 * DMAResourceLib_DMAPool_Alloc
 *
 * Allocate DMA pools for static banks
 *
 * Return Values
 *     0    Success
 *     <0   Error code (implementation specific)
 */
static int
DMAResourceLib_DMAPool_Alloc(void)
{
    unsigned int i, ListID, BankCount;
    DMAResource_Properties_t PoolProperties;
    HWPAL_DMAResource_Properties_Ext_t PoolPropertiesExt;

    ZEROINIT(PoolProperties);
    ZEROINIT(PoolPropertiesExt);

    ListID = 0;

    PoolProperties.Alignment  = HWPAL_DMARESOURCE_DMA_ALIGNMENT_BYTE_COUNT;
    PoolProperties.fCached    = true;

    BankCount = HWPAL_DMARESOURCE_BANK_COUNT;

    if (BankCount > HWPAL_DMARESOURCE_MAX_NOF_BANKS)
    {
       LOG_CRIT("%s: failed, maximum %d DMA banks supported\n",
                __func__,
                HWPAL_DMARESOURCE_MAX_NOF_BANKS);
       return -1;
    }

    LOG_INFO("%s: allocating %d DMA banks\n", __func__, BankCount);

    for(i = 0; i < BankCount; i++)
    {
        unsigned int j, AlignedBlockByteCount;
        List_Element_t * p;
        uint8_t * Byte_p;

        // Only static banks require DMA pool allocation
        if (HWPAL_DMA_Banks[i].BankType == HWPAL_DMARESOURCE_BANK_DYNAMIC)
        {
            LOG_INFO("%s: skipping dynamic bank %d\n",
                     __func__,
                     HWPAL_DMA_Banks[i].Bank);
            continue;
        }

        // Only non-cached SFA DMA banks are supported at the moment
        if (HWPAL_DMA_Banks[i].BankType ==
                HWPAL_DMARESOURCE_BANK_STATIC_FIXED_ADDR &&
            HWPAL_DMA_Banks[i].fCached)
        {
            LOG_CRIT("%s: failed for bank %d, "
                     "cached SFA DMA banks not supported\n",
                     __func__,
                     HWPAL_DMA_Banks[i].Bank);
            goto fail;
        }

        AlignedBlockByteCount = DMAResourceLib_AlignForSize(
                                HWPAL_DMA_Banks[i].BlockByteCount,
                                (unsigned int)PoolProperties.Alignment);
                               /*HWPAL_DMAResource_DCache_Alignment_Get()*/

        PoolProperties.Bank      = HWPAL_DMA_Banks[i].Bank;
        PoolProperties.Size      = AlignedBlockByteCount *
                                               HWPAL_DMA_Banks[i].BlockCount;
        PoolProperties.fCached   = HWPAL_DMA_Banks[i].fCached;

        // Will be used only for HWPAL_DMA_Banks[i].BankType ==
        // HWPAL_DMARESOURCE_BANK_STATIC_FIXED_ADDR
        PoolPropertiesExt.Addr      = HWPAL_DMA_Banks[i].Addr;
        PoolPropertiesExt.BankType  = HWPAL_DMA_Banks[i].BankType;

        // Allocate DMA pool
#ifdef HWPAL_DMARESOURCE_USE_UMDEVXS_DEVICE
        if (HWPAL_DMA_Banks[i].fAppsShared)
        {
            DMAResource_Record_t * Rec_p;
            DMAResource_AddrPair_t * Pair_p;
            SHDevXS_DMAPool_t DMAPool;

            LOG_INFO("%s: allocating app-shared DMA bank %d\n", __func__, i);

            ZEROINIT(DMAPool);

            if (SHDevXS_DMAPool_Init(&DMAPool) != 0)
                goto fail;

            if (DMAPool.ByteCount < PoolProperties.Size)
                goto fail;

            PoolProperties.Size = DMAPool.ByteCount;
            PoolProperties.fCached = DMAPool.fCached;

            // allocate record -> Handle & Rec_p
            HWPAL_DMA_Banks[i].PoolHandle = DMAResource_CreateRecord();
            if (HWPAL_DMA_Banks[i].PoolHandle == NULL)
                goto fail;

            Rec_p = DMAResource_Handle2RecordPtr(HWPAL_DMA_Banks[i].PoolHandle);
            if (Rec_p == NULL)
                goto fail;

            Rec_p->Magic = DMARES_RECORD_MAGIC;
            Rec_p->Props = PoolProperties;
            Rec_p->AllocatorRef = 'A'; // Internal allocator
            Rec_p->BufferSize = DMAPool.ByteCount;

            HWPAL_DMAResource_Record_Update(DMAPool.Handle, Rec_p);

            Pair_p = Rec_p->AddrPairs;

            Pair_p->Address_p = DMAPool.DMA_Addr.p;
            Pair_p->Domain = DMARES_DOMAIN_BUS;

            ++Pair_p;
            Pair_p->Address_p = DMAPool.HostAddr.p;
            Pair_p->Domain = DMARES_DOMAIN_HOST;

            Byte_p = (uint8_t*)DMAPool.HostAddr.p;
        }
        else
#endif // !HWPAL_DMARESOURCE_USE_UMDEVXS_DEVICE
        {
            DMAResource_AddrPair_t PoolAddrPair;

            ZEROINIT(PoolAddrPair);

            if (HWPAL_DMAResource_Alloc(PoolProperties,
                                        PoolPropertiesExt,
                                        &PoolAddrPair,
                                        &HWPAL_DMA_Banks[i].PoolHandle) != 0)
                goto fail;

            Byte_p = (uint8_t*)PoolAddrPair.Address_p;
        }

        // Allocate an array of list elements
        p = HWPAL_DMAResource_MemAlloc(sizeof(List_Element_t) *
                                           HWPAL_DMA_Banks[i].BlockCount);
        if (p == NULL)
            goto fail;

        HWPAL_DMA_Banks[i].fDMAPoolLocked = false;
        HWPAL_DMA_Banks[i].ListElements   = p;
        HWPAL_DMA_Banks[i].PoolListID     = ListID++;
        HWPAL_DMA_Banks[i].DanglingListID = ListID++;

        if (List_Init(HWPAL_DMA_Banks[i].PoolListID, NULL) != LIST_STATUS_OK ||
            List_Init(HWPAL_DMA_Banks[i].DanglingListID, NULL) != LIST_STATUS_OK)
            goto fail;

        for(j = 0; j < HWPAL_DMA_Banks[i].BlockCount; j++)
        {
            p->DataObject_p = Byte_p + j * AlignedBlockByteCount;
            if (List_AddToHead(HWPAL_DMA_Banks[i].PoolListID, NULL, p++) !=
                    LIST_STATUS_OK)
                goto fail;
        }

        // Allocate a pool lock
        HWPAL_DMA_Banks[i].Lock_p = HWPAL_DMAResource_Lock_Alloc();
        if (HWPAL_DMA_Banks[i].Lock_p == NULL)
            goto fail;

        LOG_INFO("%s: %d allocated static bank %d at offset 0x%p (cached=%d), "
                 "for %d blocks, %d (aligned %d) bytes each), handle 0x%p\n",
                 __func__,
                 i,
                 HWPAL_DMA_Banks[i].Bank,
                 HWPAL_DMA_Banks[i].Addr,
                 HWPAL_DMA_Banks[i].fCached,
                 HWPAL_DMA_Banks[i].BlockCount,
                 HWPAL_DMA_Banks[i].BlockByteCount,
                 AlignedBlockByteCount,
                 HWPAL_DMA_Banks[i].PoolHandle);
    } // for

    return 0;

fail:
    DMAResourceLib_DMAPool_Free();

    return -1;
} // DMA pools allocation for static banks done


/*----------------------------------------------------------------------------
 * DMAResourceLib_DMAPool_Put
 *
 * Put a DMA resource host address to a DMA pool

 * Return Values
 *     0    Success
 *     <0   Error code (implementation specific)
 */
static int
DMAResourceLib_DMAPool_Put(
        HWPAL_DMAResource_Bank_t * Bank_p,
        void * const Addr_p)
{
    List_Status_t List_rc1, List_rc2;
    List_Element_t * LE_p;
    unsigned long Flags = 0;

    // Dangling list access protected by lock
    HWPAL_DMAResource_Lock_Acquire(Bank_p->Lock_p, &Flags);

    List_rc1 = List_RemoveFromTail(Bank_p->DanglingListID, NULL, &LE_p);

    LE_p->DataObject_p = Addr_p;

    List_rc2 = List_AddToHead(Bank_p->PoolListID, NULL, LE_p);

    HWPAL_DMAResource_Lock_Release(Bank_p->Lock_p, &Flags);

    if (List_rc1 != LIST_STATUS_OK || List_rc2 != LIST_STATUS_OK)
    {
        LOG_CRIT("DMAResourceLib_DMAPool_Put: failed\n");
        return -1;
    }

    return 0; // success
}


/*----------------------------------------------------------------------------
 * DMAResourceLib_DMAPool_Get
 *
 * Get a DMA resource host address from a DMA pool
 *
 * Return Values
 *     0    Success
 *     <0   Error code (implementation specific)
 */
static int
DMAResourceLib_DMAPool_Get(
        HWPAL_DMAResource_Bank_t * Bank_p,
        void ** const Addr_pp)
{
    List_Status_t List_rc1, List_rc2;
    List_Element_t * LE_p;
    unsigned long Flags = 0;

    // Pool list access protected by lock
    HWPAL_DMAResource_Lock_Acquire(Bank_p->Lock_p, &Flags);

    if (Bank_p->fDMAPoolLocked)
    {
        HWPAL_DMAResource_Lock_Release(Bank_p->Lock_p, &Flags);

        LOG_CRIT("DMAResourceLib_DMAPool_Get: failed, DMA pool is locked\n");
        return -1;
    }

    List_rc1 = List_RemoveFromTail(Bank_p->PoolListID, NULL, &LE_p);
    if (List_rc1 != LIST_STATUS_OK)
    {
        HWPAL_DMAResource_Lock_Release(Bank_p->Lock_p, &Flags);

        LOG_CRIT("DMAResourceLib_DMAPool_Get: failed, out of list elements\n");
        return -1;
    }

    List_rc2 = List_AddToHead(Bank_p->DanglingListID, NULL, LE_p);

    HWPAL_DMAResource_Lock_Release(Bank_p->Lock_p, &Flags);

    if (List_rc2 != LIST_STATUS_OK)
    {
        LOG_CRIT("DMAResourceLib_DMAPool_Get: "
                 "failed to add to list of elements\n");
        return -1;
    }

    *Addr_pp = LE_p->DataObject_p;

    return 0;
}
#endif // HWPAL_DMARESOURCE_BANKS_ENABLE


/*----------------------------------------------------------------------------
 * DMAResource_Init
 */
bool
DMAResource_Init(void)
{
    if (HWPAL_DMAResource_Init() == false)
        return false;

#ifdef HWPAL_DMARESOURCE_BANKS_ENABLE
    if (DMAResourceLib_DMAPool_Alloc() == 0)
        return true;
    else
    {
        LOG_CRIT("%s: DMA pool allocation failed\n", __func__);
        return false;
    }
#else
    return true;
#endif // HWPAL_DMARESOURCE_BANKS_ENABLE
}


/*----------------------------------------------------------------------------
 * DMAResource_UnInit
 *
 * This function can be used to uninitialize the DMAResource administration.
 * The caller must make sure that handles will not be used after this function
 * returns.
 * If memory was allocated by DMAResource_Init, this function will free it.
 */
void
DMAResource_UnInit(void)
{
#ifdef HWPAL_DMARESOURCE_BANKS_ENABLE
    DMAResourceLib_DMAPool_Free();
#endif // HWPAL_DMARESOURCE_BANKS_ENABLE

    HWPAL_DMAResource_UnInit();
}


/*----------------------------------------------------------------------------
 * DMAResource_Alloc
 */
int
DMAResource_Alloc(
        const DMAResource_Properties_t RequestedProperties,
        DMAResource_AddrPair_t * const AddrPair_p,
        DMAResource_Handle_t * const Handle_p)
{
    HWPAL_DMAResource_Properties_Ext_t PoolPropertiesExt;

#ifdef HWPAL_DMARESOURCE_BANKS_ENABLE
    HWPAL_DMAResource_Bank_t * Bank_p;
#endif

    ZEROINIT(PoolPropertiesExt);

#ifdef HWPAL_DMARESOURCE_BANKS_ENABLE
    // Find the bank
    Bank_p = DMAResourceLib_DMAPool_Bank_Get(RequestedProperties.Bank);
    if (Bank_p == NULL)
    {
        LOG_CRIT("DMAResource_Alloc: failed for unsupported bank %d\n",
                 (int)RequestedProperties.Bank);
        return -1; // Bank not supported
    }

    if (Bank_p->BankType == HWPAL_DMARESOURCE_BANK_DYNAMIC)
        // Handle non-static banks
        return HWPAL_DMAResource_Alloc(RequestedProperties,
                                       PoolPropertiesExt,
                                       AddrPair_p,
                                       Handle_p);
    else
    {   // Handle static banks

        if (RequestedProperties.Size <= Bank_p->BlockByteCount)
        {
            int Rc;
            DMAResource_AddrPair_t AddrPair;
            DMAResource_Properties_t NewProperties;

            if( DMAResourceLib_DMAPool_Get(Bank_p,
                                           &AddrPair.Address_p) != 0)
            {
                LOG_WARN("DMAResource_Alloc: failed for static bank %d\n",
                         (int)RequestedProperties.Bank);
                return -1;
            }

            AddrPair.Domain = DMARES_DOMAIN_HOST;
            // All memory blocks have the same fixed size in one bank
            NewProperties = RequestedProperties;

            if (NewProperties.Size != Bank_p->BlockByteCount)
            {
                LOG_INFO(
                     "%s: changing requested resource size from %d to %d bytes "
                     "for static bank %d\n",
                     __func__,
                     NewProperties.Size,
                     Bank_p->BlockByteCount,
                     (int)RequestedProperties.Bank);

                NewProperties.Size = Bank_p->BlockByteCount;
            }

            if (NewProperties.fCached != Bank_p->fCached)
            {
                LOG_INFO(
                     "%s: changing requested resource caching from %d to %d "
                     "for static bank %d\n",
                     __func__,
                     NewProperties.fCached,
                     Bank_p->fCached,
                     (int)RequestedProperties.Bank);

                NewProperties.fCached = Bank_p->fCached;
            }

            Rc = HWPAL_DMAResource_CheckAndRegister(NewProperties,
                                                    AddrPair,
                                                    'R',
                                                    Handle_p);
            if (Rc != 0)
            {
                // Failed to register the memory block obtained from
                // the DMA pool as a DMA resource, put it back to the pool
                DMAResourceLib_DMAPool_Put(Bank_p, AddrPair.Address_p);
            }
            else
            {
                AddrPair_p->Domain    = AddrPair.Domain;
                AddrPair_p->Address_p = AddrPair.Address_p;
            }

            return Rc;
        }
        else if (RequestedProperties.Size ==
                 Bank_p->BlockByteCount * Bank_p->BlockCount)
        {   // Bank-full DMA resource allocation is requested

            // Check that no DMA resources have been allocated in this bank yet
            // If true then lock the bank so that
            // no new DMA resources can be allocated in it
            if(DMAResourceLib_DMAPool_Lock(Bank_p))
            {
                // No DMA resources have been allocated in this bank yet
                *Handle_p = Bank_p->PoolHandle;

                return 0; // Bank-full DMA resource is locked
            }
            else
            {   // DMA resources have been already allocated in this bank
                LOG_CRIT("DMAResource_Alloc: failed, "
                         "all-pool DMA resource unavailable for bank %d\n",
                         (int)RequestedProperties.Bank);
                return -1;
            }
        }
        else
        {   // Requested size unsupported
            LOG_CRIT("DMAResource_Alloc: failed, "
                     "requested size %d > bank block size %d (bytes)\n",
                     (int)RequestedProperties.Size,
                     Bank_p->BlockByteCount);
            return -1;
        }
    }
#else
    return HWPAL_DMAResource_Alloc(RequestedProperties,
                                   PoolPropertiesExt,
                                   AddrPair_p,
                                   Handle_p);
#endif // HWPAL_DMARESOURCE_BANKS_ENABLE
}


/*----------------------------------------------------------------------------
 * DMAResource_Release
 */
int
DMAResource_Release(
        const DMAResource_Handle_t Handle)
{
#ifdef HWPAL_DMARESOURCE_BANKS_ENABLE
    HWPAL_DMAResource_Bank_t * Bank_p;
    DMAResource_Record_t * Rec_p;

    Rec_p = DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        LOG_CRIT("DMAResource_Release: invalid handle %p\n", Handle);
        return -1;
    }

    // Find the bank
    Bank_p = DMAResourceLib_DMAPool_Bank_Get(Rec_p->Props.Bank);
    if (Bank_p == NULL)
    {
        LOG_CRIT("DMAResource_Release: failed for unsupported bank %d\n",
                 (int)Rec_p->Props.Bank);
        return -1; // Bank not supported
    }

    // Handle non-static banks
    if (Bank_p->BankType == HWPAL_DMARESOURCE_BANK_DYNAMIC)
        return HWPAL_DMAResource_Release(Handle);
    else
    {
        // Check if the all-pool DMA resource release is requested
        if (Handle == Bank_p->PoolHandle)
        {
            // Unlock the DMA pool in the bank so that new DMA resources
            // can be allocated in it
            if (DMAResourceLib_DMAPool_Unlock(Bank_p) == false)
            {
                LOG_CRIT("DMAResource_Release: failed, "
                         "all-pool DMA resource for bank %d "
                         "already released\n",
                         (int)Rec_p->Props.Bank);
                return -1;
            }
            else
                // DMA pool handle will be released by DMAResource_UnInit()
                return 0; // Bank-full DMA resource is unlocked
        }

        if (Rec_p->AllocatorRef == 'R')
        {
            DMAResource_AddrPair_t * AddrPair_p;

            AddrPair_p = DMAResourceLib_LookupDomain(Rec_p, DMARES_DOMAIN_HOST);
            if (AddrPair_p == NULL)
            {
                LOG_CRIT("DMAResource_Release: "
                         "host address lookup failed for handle %p\n",
                         Handle);
                return -1;
            }

            // Check if resource belongs to a DMA pool
            if (Bank_p->BlockByteCount == Rec_p->Props.Size)
            {
                // It does, put it back to pool
                if (DMAResourceLib_DMAPool_Put(Bank_p,
                                               AddrPair_p->Address_p) != 0)
                {
                    LOG_CRIT("DMAResource_Release: "
                             "put to DMA pool failed for handle %p\n",
                             Handle);
                    return -1;
                }
            }
        }

        return HWPAL_DMAResource_Release(Handle);
    }
#else
    return HWPAL_DMAResource_Release(Handle);
#endif // HWPAL_DMARESOURCE_BANKS_ENABLE
}


/*----------------------------------------------------------------------------
 * DMAResource_CheckAndRegister
 */
int
DMAResource_CheckAndRegister(
        const DMAResource_Properties_t RequestedProperties,
        const DMAResource_AddrPair_t AddrPair,
        const char AllocatorRef,
        DMAResource_Handle_t * const Handle_p)
{
#ifdef HWPAL_DMARESOURCE_BANKS_ENABLE
    HWPAL_DMAResource_Bank_t * Bank_p;

    // Find the bank
    Bank_p = DMAResourceLib_DMAPool_Bank_Get(RequestedProperties.Bank);
    if (Bank_p == NULL)
    {
        LOG_CRIT(
            "DMAResource_CheckAndRegister: failed for unsupported bank %d\n",
            (int)RequestedProperties.Bank);

        return -1; // Bank not supported
    }

    // Handle non-static banks
    if (Bank_p->BankType == HWPAL_DMARESOURCE_BANK_DYNAMIC)
        return HWPAL_DMAResource_CheckAndRegister(RequestedProperties,
                                                  AddrPair,
                                                  AllocatorRef,
                                                  Handle_p);
    else // Handle static banks in a different way
    {
        // Only allocators 'R' and 'N' are supported for static banks
        if (AllocatorRef != 'R' && AllocatorRef != 'N')
        {
            LOG_CRIT(
                "DMAResource_CheckAndRegister: failed, "
                "allocator %c unsupported for static bank %d\n",
                AllocatorRef,
                (int)RequestedProperties.Bank);
            return -1;
        }

        // Check if requested size is less than the bank memory block size
        if ((AllocatorRef == 'R' &&
             RequestedProperties.Size < Bank_p->BlockByteCount) ||
             AllocatorRef == 'N')
        {
            return HWPAL_DMAResource_CheckAndRegister(RequestedProperties,
                                                      AddrPair,
                                                      AllocatorRef,
                                                      Handle_p);
        }
        else
        {
            LOG_CRIT(
                "DMAResource_CheckAndRegister: failed, "
                "requested size %d >= block size %d in static bank %d\n",
                RequestedProperties.Size,
                Bank_p->BlockByteCount,
                (int)RequestedProperties.Bank);
            return -1;
        }
    }
#else
    return HWPAL_DMAResource_CheckAndRegister(RequestedProperties,
                                              AddrPair,
                                              AllocatorRef,
                                              Handle_p);
#endif // HWPAL_DMARESOURCE_BANKS_ENABLE
}


/*----------------------------------------------------------------------------
 * DMAResource_Read32
 */
uint32_t
DMAResource_Read32(
        const DMAResource_Handle_t Handle,
        const unsigned int WordOffset)
{
    DMAResource_AddrPair_t * Pair_p;
    DMAResource_Record_t * Rec_p;

    Rec_p = DMAResource_Handle2RecordPtr(Handle);
#ifndef HWPAL_DMARESOURCE_OPT1
    if (Rec_p == NULL)
    {
        LOG_WARN(
            "DMAResource_Read32: "
            "Invalid handle %p\n",
            Handle);

        return 0;
    }
#endif

#ifdef HWPAL_DMARESOURCE_STRICT_ARGS_CHECKS
    if (WordOffset * 4 >= Rec_p->Props.Size)
    {
        LOG_WARN(
            "DMAResource_Read32: "
            "Invalid WordOffset %u for Handle %p\n",
            WordOffset,
            Handle);

        return 0;
    }
#endif

    Pair_p = DMAResourceLib_LookupDomain(Rec_p, DMARES_DOMAIN_HOST);
#ifndef HWPAL_DMARESOURCE_OPT1
    if (Pair_p == NULL)
    {
        LOG_WARN(
            "DMAResource_Read32: "
            "No host address found for Handle %p?\n",
            Handle);

        return 0;
    }
#endif

    {
        uint32_t * Address_p = Pair_p->Address_p;
        uint32_t Value = read32_volatile(Address_p + WordOffset);

#ifndef HWPAL_DMARESOURCE_OPT2
        // swap endianness, if required
        if (Rec_p->fSwapEndianness)
            Value = Device_SwapEndian32(Value);
#endif

#ifdef HWPAL_TRACE_DMARESOURCE_READ
        Log_FormattedMessage(
            "DMAResource_Read32:  "
            "(handle %p) "
            "0x%08x = [%u] "
            "(swap=%d)\n",
            Handle,
            Value,
            WordOffset,
            Rec_p->fSwapEndianness);
#endif

        return Value;
    }
}


/*----------------------------------------------------------------------------
 * DMAResource_Write32
 */
void
DMAResource_Write32(
        const DMAResource_Handle_t Handle,
        const unsigned int WordOffset,
        const uint32_t Value)
{
    DMAResource_AddrPair_t * Pair_p;
    DMAResource_Record_t * Rec_p;

    Rec_p = DMAResource_Handle2RecordPtr(Handle);
#ifndef HWPAL_DMARESOURCE_OPT1
    if (Rec_p == NULL)
    {
        LOG_WARN(
            "DMAResource_Write32: "
            "Invalid handle %p\n",
            Handle);

        return;
    }
#endif

#ifdef HWPAL_DMARESOURCE_STRICT_ARGS_CHECKS
    if (WordOffset * 4 >= Rec_p->Props.Size)
    {
        LOG_WARN(
            "DMAResource_Write32: "
            "Invalid WordOffset %u for Handle %p\n",
            WordOffset,
            Handle);

        return;
    }
#endif

    Pair_p = DMAResourceLib_LookupDomain(Rec_p, DMARES_DOMAIN_HOST);
#ifndef HWPAL_DMARESOURCE_OPT1
    if (Pair_p == NULL)
    {
        LOG_WARN(
            "DMAResource_Write32: "
            "No host address found for Handle %p?\n",
            Handle);

        return;
    }
#endif

#ifdef HWPAL_TRACE_DMARESOURCE_WRITE
    Log_FormattedMessage(
        "DMAResource_Write32: "
        "(handle %p) "
        "[%u] = 0x%08x "
        "(swap=%d)\n",
        Handle,
        WordOffset,
        Value,
        Rec_p->fSwapEndianness);
#endif

    {
        uint32_t * Address_p = Pair_p->Address_p;
        uint32_t WriteValue = Value;

#ifndef HWPAL_DMARESOURCE_OPT2
        // swap endianness, if required
        if (Rec_p->fSwapEndianness)
            WriteValue = Device_SwapEndian32(WriteValue);
#endif

        write32_volatile(WriteValue, Address_p + WordOffset);
    }
}


/*----------------------------------------------------------------------------
 * DMAResource_Read32Array
 */
void
DMAResource_Read32Array(
        const DMAResource_Handle_t Handle,
        const unsigned int StartWordOffset,
        const unsigned int WordCount,
        uint32_t * Values_p)
{
    DMAResource_AddrPair_t * Pair_p;
    DMAResource_Record_t * Rec_p;

    if (WordCount == 0)
        return;

    Rec_p = DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        LOG_WARN(
            "DMAResource_Read32Array: "
            "Invalid handle %p\n",
            Handle);
        return;
    }

#ifdef HWPAL_DMARESOURCE_STRICT_ARGS_CHECKS
    if ((StartWordOffset + WordCount - 1) * 4 >= Rec_p->Props.Size)
    {
        LOG_WARN(
            "DMAResource_Read32Array: "
            "Invalid range: %u - %u\n",
            StartWordOffset,
            StartWordOffset + WordCount - 1);
        return;
    }
#endif

    Pair_p = DMAResourceLib_LookupDomain(Rec_p, DMARES_DOMAIN_HOST);
    if (Pair_p == NULL)
    {
        LOG_WARN(
            "DMAResource_Read32Array: "
            "No host address found for Handle %p?\n",
            Handle);

        return;
    }

    {
        uint32_t * Address_p = Pair_p->Address_p;
        unsigned int i;

        for (i = 0; i < WordCount; i++)
        {
            uint32_t Value = read32_volatile(Address_p + StartWordOffset + i);

            // swap endianness, if required
            if (Rec_p->fSwapEndianness)
                Value = Device_SwapEndian32(Value);

            Values_p[i] = Value;
        } // for
    }

#ifdef HWPAL_TRACE_DMARESOURCE_READ
    {
        uint32_t * Address_p = Pair_p->Address_p;
        if (Values_p == Address_p + StartWordOffset)
        {
            Log_FormattedMessage(
                "DMAResource_Read32Array: "
                "(handle %p) "
                "[%u..%u] IN-PLACE "
                "(swap=%d)\n",
                Handle,
                StartWordOffset,
                StartWordOffset + WordCount - 1,
                Rec_p->fSwapEndianness);
        }
        else
        {
            Log_FormattedMessage(
                "DMAResource_Read32Array: "
                "(handle %p) "
                "[%u..%u] "
                "(swap=%d)\n",
                Handle,
                StartWordOffset,
                StartWordOffset + WordCount - 1,
                Rec_p->fSwapEndianness);
        }
    }
#endif
}


/*----------------------------------------------------------------------------
 * DMAResource_Write32Array
 */
void
DMAResource_Write32Array(
        const DMAResource_Handle_t Handle,
        const unsigned int StartWordOffset,
        const unsigned int WordCount,
        const uint32_t * Values_p)
{
    DMAResource_AddrPair_t * Pair_p;
    DMAResource_Record_t * Rec_p;

    if (WordCount == 0)
        return;

    Rec_p = DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        LOG_WARN(
            "DMAResource_Write32Array: "
            "Invalid handle %p\n",
            Handle);
        return;
    }

#ifdef HWPAL_DMARESOURCE_STRICT_ARGS_CHECKS
    if ((StartWordOffset + WordCount - 1) * 4 >= Rec_p->Props.Size)
    {
        LOG_WARN(
            "DMAResource_Write32Array: "
            "Invalid range: %u - %u\n",
            StartWordOffset,
            StartWordOffset + WordCount - 1);
        return;
    }
#endif

    Pair_p = DMAResourceLib_LookupDomain(Rec_p, DMARES_DOMAIN_HOST);
    if (Pair_p == NULL)
    {
        LOG_WARN(
            "DMAResource_Write32Array: "
            "No host address found for Handle %p?\n",
            Handle);

        return;
    }

    {
        uint32_t * Address_p = Pair_p->Address_p;
        unsigned int i;

        for (i = 0; i < WordCount; i++)
        {
            uint32_t Value = Values_p[i];

            // swap endianness, if required
            if (Rec_p->fSwapEndianness)
                Value = Device_SwapEndian32(Value);

            write32_volatile(Value, Address_p + StartWordOffset + i);
        } // for
    }

#ifdef HWPAL_TRACE_DMARESOURCE_WRITE
    {
        uint32_t * Address_p = Pair_p->Address_p;
        if (Values_p == Address_p + StartWordOffset)
        {
            Log_FormattedMessage(
                "DMAResource_Write32Array: "
                "(handle %p) "
                "[%u..%u] IN-PLACE "
                "(swap=%d)\n",
                Handle,
                StartWordOffset,
                StartWordOffset + WordCount - 1,
                Rec_p->fSwapEndianness);
        }
        else
        {
            Log_FormattedMessage(
                "DMAResource_Write32Array: "
                "(handle %p) "
                "[%u..%u] "
                "(swap=%d)\n",
                Handle,
                StartWordOffset,
                StartWordOffset + WordCount - 1,
                Rec_p->fSwapEndianness);
        }
    }
#endif /* HWPAL_TRACE_DMARESOURCE_WRITE */
}


/*----------------------------------------------------------------------------
 * DMAResource_SwapEndianness_Set
 */
int
DMAResource_SwapEndianness_Set(
        const DMAResource_Handle_t Handle,
        const bool fSwapEndianness)
{
    DMAResource_Record_t * Rec_p;

    Rec_p = DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        LOG_WARN(
            "DMAResource_SwapEndianness_Set: "
            "Invalid handle %p\n",
            Handle);
        return -1;
    }

    Rec_p->fSwapEndianness = fSwapEndianness;
    return 0;
}


/*----------------------------------------------------------------------------
 * DMAResource_SwapEndianness_Get
 */
int
DMAResource_SwapEndianness_Get(
        const DMAResource_Handle_t Handle)
{
    DMAResource_Record_t * Rec_p;

    Rec_p = DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        LOG_WARN(
            "DMAResource_SwapEndianness_Get: "
            "Invalid handle %p\n",
            Handle);
        return -1;
    }

    if (Rec_p->fSwapEndianness)
    {
        return 1;
    }
    return 0;
}


/*----------------------------------------------------------------------------
 * DMAResource_AddPair
 */
int
DMAResource_AddPair(
        const DMAResource_Handle_t Handle,
        const DMAResource_AddrPair_t Pair)
{
    DMAResource_Record_t * Rec_p;
    DMAResource_AddrPair_t * AddrPair_p;

    Rec_p = DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        LOG_WARN(
            "DMAResource_AddPair: "
            "Invalid handle %p\n",
            Handle);
        return -1;
    }

    // check if this pair already exists
    AddrPair_p = DMAResourceLib_LookupDomain(Rec_p, Pair.Domain);
    if (AddrPair_p)
    {
        LOG_INFO(
            "DMAResource_AddPair: "
            "Replacing address for handle %p?\n",
            Handle);
    }
    else
    {
        // find a free slot to store this domain info
        AddrPair_p = DMAResourceLib_LookupDomain(Rec_p, 0);
        if (AddrPair_p == NULL)
        {
            LOG_WARN(
                "DMAResource_AddPair: "
                "Table overflow for handle %p\n",
                Handle);
            return -2;
        }
    }

    if (!DMAResourceLib_IsSaneInput(&Pair, &Rec_p->AllocatorRef, &Rec_p->Props))
    {
        return -3;
    }

    *AddrPair_p = Pair;
    return 0;
}


/*----------------------------------------------------------------------------
 * DMAResource_Translate
 *
 * This function implements a fake address translation which only extracts
 * the required address information from the corresponding resource record
 *
 * All the address information is stored in the resource record in the
 * DMAResource_Alloc() and DMAResource_CheckAndRegister() functions
 *
 */
int
DMAResource_Translate(
        const DMAResource_Handle_t Handle,
        const DMAResource_AddrDomain_t DestDomain,
        DMAResource_AddrPair_t * const PairOut_p)
{
    DMAResource_Record_t * Rec_p;
    DMAResource_AddrPair_t * Pair_p;

    if (NULL == PairOut_p)
    {
        return -1;
    }

    Rec_p = DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        LOG_WARN(
            "DMAResource_Translate: "
            "Invalid handle %p\n",
            Handle);
        return -1;
    }

    switch (DestDomain)
    {
        case DMARES_DOMAIN_HOST_UNALIGNED:
        case DMARES_DOMAIN_HOST:
        case DMARES_DOMAIN_BUS:
            Pair_p = DMAResourceLib_LookupDomain(Rec_p, DestDomain);
            if (Pair_p != NULL)
            {
                *PairOut_p = *Pair_p;
                return 0;
            }
            break;

        default:
            LOG_WARN(
                "DMAResource_Translate: "
                "Unsupported domain %u\n",
                DestDomain);
            PairOut_p->Address_p = NULL;
            PairOut_p->Domain = DMARES_DOMAIN_UNKNOWN;
            return -1;
    } // switch

    LOG_WARN(
        "DMAResource_Translate: "
        "No address for domain %u (Handle=%p)\n",
        DestDomain,
        Handle);

    PairOut_p->Address_p = NULL;
    PairOut_p->Domain = DMARES_DOMAIN_UNKNOWN;
    return -1;
}


/* end of file dmares_gen.c */
