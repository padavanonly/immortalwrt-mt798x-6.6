/* umdevxs_bufadmin.c
 *
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

#include "umdevxs_internal.h"   // also LOG_

#include "basic_defs.h"
#include "clib.h"
#include "umdevxs_bufadmin.h"

#include <linux/slab.h>         // kmalloc, kfree

#ifdef HWPAL_LOCK_SLEEPABLE
#include <linux/mutex.h>        // mutex_*
#else
#include <linux/spinlock.h>     // spinlock_*
#endif

/*

 Requirements on the records:
  - pre-allocated array of records
  - valid between Create and Destroy
  - re-use on a least-recently-used basis to make sure accidental continued
    use after destroy does not cause crashes, allowing us to detect the
    situation instead of crashing quickly.

 Requirements on the handles:
  - one handle per record
  - valid between Create and Destroy
  - quickly find the ptr-to-record belonging to the handle
  - detect continued use of a handle after Destroy
  - caller-hidden admin/status, thus not inside the record
  - report leaking handles upon exit

 Solution:
  - handle cannot be a record number (no post-destroy use detection possible)
  - recnr/destroyed in separate memory location for each handle: Handles_p
  - Array of records: Records_p
  - free locations in Array1: Freelist1 (FreeHandles)
  - free record numbers list: Freelist2 (FreeRecords)
 */

typedef struct
{
    int ReadIndex;
    int WriteIndex;
    int * Nrs_p;
} HWPAL_FreeList_t;

static int HandlesCount = 0; // remainder are valid only when this is != 0
static int * Handles_p;
static BufAdmin_Record_t * Records_p;
static HWPAL_FreeList_t FreeHandles;
static HWPAL_FreeList_t FreeRecords;


#ifdef HWPAL_LOCK_SLEEPABLE
static struct mutex HWPAL_Lock;
#else
static spinlock_t HWPAL_SpinLock;
#endif

#define HWPAL_RECNR_DESTROYED  -1

// to avoid NULL for HandleNr=0 we add 1
#define HWPAL_HANDLE_NR2HANDLE(_n) (BufAdmin_Handle_t)((_n) + 1)
#define HWPAL_HANDLE_HANDLE2NR(_p) (((int)(_p)) - 1)


/*----------------------------------------------------------------------------
 * HWPAL_FreeList_Get
 *
 * Gets the next entry from the freelist. Returns -1 when the list is empty.
 */
static inline int
HWPAL_FreeList_Get(
        HWPAL_FreeList_t * const List_p)
{
    int Nr = -1;
    int ReadIndex_Updated = List_p->ReadIndex + 1;

    if (ReadIndex_Updated >= HandlesCount)
        ReadIndex_Updated = 0;

    // if post-increment ReadIndex == WriteIndex, the list is empty
    if (ReadIndex_Updated != List_p->WriteIndex)
    {
        // grab the next number
        Nr = List_p->Nrs_p[List_p->ReadIndex];
        List_p->ReadIndex = ReadIndex_Updated;
    }

    return Nr;
}


/*----------------------------------------------------------------------------
 * HWPAL_FreeList_Add
 *
 * Adds an entry to the freelist.
 */
static inline void
HWPAL_FreeList_Add(
        HWPAL_FreeList_t * const List_p,
        int Nr)
{
    if (List_p->WriteIndex == List_p->ReadIndex)
    {
        LOG_WARN(
            "HWPAL_FreeList_Add: "
            "Attempt to add value %d to full list\n",
            Nr);
        return;
    }

    if (Nr < 0 || Nr >= HandlesCount)
    {
        LOG_WARN(
            "HWPAL_FreeList_Add: "
            "Attempt to put invalid value: %d\n",
            Nr);
        return;
    }

    {
        int WriteIndex_Updated = List_p->WriteIndex + 1;
        if (WriteIndex_Updated >= HandlesCount)
            WriteIndex_Updated = 0;

        // store the number
        List_p->Nrs_p[List_p->WriteIndex] = Nr;
        List_p->WriteIndex = WriteIndex_Updated;
    }
}


/*----------------------------------------------------------------------------
 * HWPAL_Hexdump
 *
 * This function hex-dumps an array of uint32_t.
 */
#ifdef HWPAL_TRACE_DMARESOURCE_READWRITE

static inline void
HWPAL_DMAResource_Hexdump(
        const char * ArrayName_p,
        const uint16_t * Handle_p,
        const unsigned int Offset,
        const uint32_t * WordArray_p,
        const unsigned int WordCount,
        bool fSwapEndianness)
{
    unsigned int i;

    Log_FormattedMessage(
        "%s: "
        "Handle = %p: "
        "byte offsets %u - %u "
        "(swap=%d)\n"
        ArrayName_p,
        Handle_p,
        Offset,
        Offset + WordCount*4 - 1,
        fSwapEndianness);

    for (i = 1; i <= WordCount; i++)
    {
        uint32_t Value = WordArray_p[i - 1];

        if (fSwapEndianness)
            Value = HWPAL_SwapEndian32(Value);

        Log_FormattedMessage(" 0x%08x", Value);

        if ((i & 7) == 0)
            Log_Message("\n  ");
    }

    if ((WordCount & 7) != 0)
        Log_Message("\n");
}
#endif


/*----------------------------------------------------------------------------
 * BufAdmin_Init
 *
 * This function must be used to initialize the buffer record administration.
 * It must be called before any of the other BufAdmin_* functions may
 * be called. It may be called anew only after BufAdmin_UnInit has
 * been called.
 *
 * MaxHandles (input)
 *     Maximum number of handles that must be possible to administrate. The
 *     implementation will allocate enough memory to fulfill this requirement.
 *     Set to 0 to force the implementation to use the memory provided via
 *     AdminMemory_p and AdminMemorySize.
 *
 * AdminMemory_p (input)
 *     This parameter is used only when MaxHandles is 0.
 *     Pointer to the block of memory this implementation may use for the
 *     records.
 *
 * AdminMemorySize (input)
 *     This parameter is used only when MaxHandles is 0.
 *     Size of the memory block pointed to by AdminMemory_p.
 *     The implementation will calculate the maximum number of handles it can
 *     administrate.
 *
 * Return Value
 *     true   Initialization successfully, rest of the API may now be used.
 *     false  Initialization failed.
 */
bool
BufAdmin_Init(
        const unsigned int MaxHandles,
        void * AdminMemory_p,
        const unsigned int AdminMemorySize)
{
    IDENTIFIER_NOT_USED(AdminMemory_p);
    IDENTIFIER_NOT_USED(AdminMemorySize);

#ifdef HWPAL_LOCK_SLEEPABLE
    LOG_INFO(
        "HWPAL_DMAResource_Init: "
        "Lock = mutex\n");
    mutex_init(&HWPAL_Lock);
#else
    LOG_INFO(
        "HWPAL_DMAResource_Init: "
        "Lock = spinlock\n");
    spin_lock_init(&HWPAL_SpinLock);
#endif

    // already initialized?
    if (HandlesCount != 0)
        return false;

    // this implementation only supports MaxHandles != 0
    if (MaxHandles == 0)
        return false;

    Records_p = kmalloc(MaxHandles * sizeof(BufAdmin_Record_t), GFP_KERNEL);
    Handles_p = kmalloc(MaxHandles * sizeof(int), GFP_KERNEL);
    FreeHandles.Nrs_p = kmalloc(MaxHandles * sizeof(int), GFP_KERNEL);
    FreeRecords.Nrs_p = kmalloc(MaxHandles * sizeof(int), GFP_KERNEL);

    // if any allocation failed, free the whole lot
    if (Records_p == NULL ||
        Handles_p == NULL ||
        FreeHandles.Nrs_p == NULL ||
        FreeRecords.Nrs_p == NULL)
    {
        if (Records_p)
            kfree(Records_p);

        if (Handles_p)
            kfree(Handles_p);

        if (FreeHandles.Nrs_p)
            kfree(FreeHandles.Nrs_p);

        if (FreeRecords.Nrs_p)
            kfree(FreeRecords.Nrs_p);

        Records_p = NULL;
        Handles_p = NULL;
        FreeHandles.Nrs_p = NULL;
        FreeRecords.Nrs_p = NULL;

        return false;
    }

    // initialize the record numbers freelist
    // initialize the handle numbers freelist
    // initialize the handles array
    {
        unsigned int i;

        for (i = 0; i < MaxHandles; i++)
        {
            Handles_p[i] = HWPAL_RECNR_DESTROYED;
            FreeHandles.Nrs_p[i] = MaxHandles - 1 - i;
            FreeRecords.Nrs_p[i] = i;
        }

        FreeHandles.ReadIndex = 0;
        FreeHandles.WriteIndex = 0;

        FreeRecords.ReadIndex = 0;
        FreeRecords.WriteIndex = 0;
    }

    HandlesCount = MaxHandles;

    return true;
}


/*----------------------------------------------------------------------------
 * BufAdmin_UnInit
 */
void
BufAdmin_UnInit(void)
{
    // exit if not initialized
    if (HandlesCount == 0)
        return;

    // find resource leaks
#ifdef HWPAL_TRACE_DMARESOURCE_LEAKS
    {
        int i;
        bool fFirstPrint = true;

        for (i = 0; i < HandlesCount; i++)
        {
            int RecNr = Handles_p[i];

            if (RecNr >= 0)
            {
                if (fFirstPrint)
                {
                    fFirstPrint = false;
                    Log_FormattedMessage(
                        "HWPAL_DMAResource_UnInit found leaking handles:\n");
                }

                Log_FormattedMessage(
                    "Handle %p => "
                    "Record %d\n",
                    Handles_p + i,
                    RecNr);

                {
                    BufAdmin_Record_t * Rec_p = Records_p + RecNr;

                    Log_FormattedMessage(
                        " magic: 0x%x;"
                        " appid: %p;"
                        " alloc: %d,%p;"
                        " host: %d,%p;"
                        " user: %d,%p\n",
                        Rec_p->Magic,
                        Rec_p->AppID,
                        Rec_p->alloc.AllocatedSize,
                        Rec_p->alloc.AllocatedAddr_p,
                        Rec_p->host.BufferSize,
                        Rec_p->host.HostAddr_p,
                        Rec_p->user.Size,
                        Rec_p->user.Addr);
                }
            } // if
        } // for

        if (fFirstPrint)
        {
            LOG_INFO("HWPAL_DMAResource_UnInit: no leaks found\n");
        }
    }
#endif /* HWPAL_TRACE_DMARESOURCE_LEAKS */

    HandlesCount = 0;

    kfree(FreeHandles.Nrs_p);
    kfree(FreeRecords.Nrs_p);
    kfree(Handles_p);
    kfree(Records_p);

    FreeHandles.Nrs_p = NULL;
    FreeRecords.Nrs_p = NULL;
    Handles_p = NULL;
    Records_p = NULL;
}


/*----------------------------------------------------------------------------
 * BufAdmin_Record_Create
 */
BufAdmin_Handle_t
BufAdmin_Record_Create(void)
{
#ifndef HWPAL_LOCK_SLEEPABLE
    unsigned long flags;
#endif
    int HandleNr;
    int RecNr = 0;

    // return NULL when not initialized
    if (HandlesCount == 0)
        return BUFADMIN_HANDLE_NULL;

#ifdef HWPAL_LOCK_SLEEPABLE
    mutex_lock(&HWPAL_Lock);
#else
    spin_lock_irqsave(&HWPAL_SpinLock, flags);
#endif

    HandleNr = HWPAL_FreeList_Get(&FreeHandles);
    if (HandleNr != -1)
    {
        RecNr = HWPAL_FreeList_Get(&FreeRecords);
        if (RecNr == -1)
        {
            HWPAL_FreeList_Add(&FreeHandles, HandleNr);
            HandleNr = -1;
        }
    }

#ifdef HWPAL_LOCK_SLEEPABLE
    mutex_unlock(&HWPAL_Lock);
#else
    spin_unlock_irqrestore(&HWPAL_SpinLock, flags);
#endif

    // return NULL when reservation failed
    if (HandleNr == -1)
        return BUFADMIN_HANDLE_NULL;

    // initialize the record
    {
        BufAdmin_Record_t * Rec_p = Records_p + RecNr;
        memset(Rec_p, 0, sizeof(BufAdmin_Record_t));
    }

    // initialize the handle
    Handles_p[HandleNr] = RecNr;

    // return the handle value
    return HWPAL_HANDLE_NR2HANDLE(HandleNr);
}


/*----------------------------------------------------------------------------
 * BufAdmin_Record_Destroy
 */
void
BufAdmin_Record_Destroy(
        BufAdmin_Handle_t Handle)
{
    if (BufAdmin_IsValidHandle(Handle))
    {
        const int HandleNr = HWPAL_HANDLE_HANDLE2NR(Handle);
        int * const p = Handles_p + HandleNr;
        const int RecNr = *p;

        if (RecNr >= 0 &&
            RecNr < HandlesCount)
        {
#ifndef HWPAL_LOCK_SLEEPABLE
            unsigned long flags;
#endif
            // note handle is no longer value
            *p = HWPAL_RECNR_DESTROYED;

#ifdef HWPAL_LOCK_SLEEPABLE
            mutex_lock(&HWPAL_Lock);
#else
            spin_lock_irqsave(&HWPAL_SpinLock, flags);
#endif

            // add the HandleNr and RecNr to respective LRU lists
            HWPAL_FreeList_Add(&FreeHandles, HandleNr);
            HWPAL_FreeList_Add(&FreeRecords, RecNr);

#ifdef HWPAL_LOCK_SLEEPABLE
            mutex_unlock(&HWPAL_Lock);
#else
            spin_unlock_irqrestore(&HWPAL_SpinLock, flags);
#endif
        }
        else
        {
            LOG_WARN(
                "HWPAL_DMAResource_Destroy: "
                "Handle %d was already destroyed\n",
                Handle);
        }
    }
    else
    {
        LOG_WARN(
            "HWPAL_DMAResource_Destroy: "
            "Invalid handle %d\n",
            Handle);
    }
}


/*----------------------------------------------------------------------------
 * BufAdmin_IsValidHandle
 */
bool
BufAdmin_IsValidHandle(
        BufAdmin_Handle_t Handle)
{
    int h = HWPAL_HANDLE_HANDLE2NR(Handle);
    int * p;

    if (h < 0 || h >= HandlesCount)
        return false;

    p = Handles_p + h;

    // check that the handle has not been destroyed yet
    if (*p < 0 ||
        *p >= HandlesCount)
    {
        return false;
    }

    return true;
}


/*----------------------------------------------------------------------------
 * BufAdmin_Handle2RecordPtr
 */
BufAdmin_Record_t *
BufAdmin_Handle2RecordPtr(
        BufAdmin_Handle_t Handle)
{
    // assume handle is valid
    int h = HWPAL_HANDLE_HANDLE2NR(Handle);
    int * p = Handles_p + h;

    if (h < 0 || h >= HandlesCount)
        return NULL;

    if (p != NULL)
    {
        int RecNr = *p;

        if (RecNr >= 0 &&
            RecNr < HandlesCount)
        {
            return Records_p + RecNr;           // ## RETURN ##
        }
    }

    return NULL;
}


/*----------------------------------------------------------------------------
 * BufAdmin_Enumerate
 *
 * This function iterates through all existing records and invokes the
 * provided function for record.
 */
void
BufAdmin_Enumerate(
        BufAdmin_EnumFnc_t EnumFnc,
        void * Param1)
{
    int HandleNr;

    if (EnumFnc == NULL)
        return;

    if (HandlesCount == 0)
        return;

    for (HandleNr = 0; HandleNr < HandlesCount; HandleNr++)
    {
        BufAdmin_Handle_t Handle;

        Handle = HWPAL_HANDLE_NR2HANDLE(HandleNr);

        if (Handles_p[HandleNr] != HWPAL_RECNR_DESTROYED)
        {
            BufAdmin_Record_t * Rec_p;

            Rec_p = BufAdmin_Handle2RecordPtr(Handle);

            EnumFnc(Handle, Rec_p, Param1);
        }
    } // for
}


/* end of file umdevxs_bufadmin.c */
