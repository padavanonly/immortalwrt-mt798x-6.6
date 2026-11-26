/* umdevxs_device.c
 *
 * Device API for the Linux UMDevXS driver.
 *
 * It allows searching for a named device resource, which returns a handle.
 * The handle can be used to map the device memory into the application
 * memory map (in user mode).
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

// UMDevXS Device API
#include "umdevxs_device.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "c_umdevxs.h"              // config options

#ifndef UMDEVXS_REMOVE_DEVICE

#include "umdevxs_internal.h"

#include <linux/string.h>           // strcmp
#include <linux/mm.h>               // remap_pfn_range
#include <linux/version.h>

#ifdef UMDEVXS_ENABLE_DEVICE_LOCK
#ifdef HWPAL_LOCK_SLEEPABLE
#include <linux/mutex.h>        // mutex_*
#else
#include <linux/spinlock.h>     // spinlock_*
#endif
#endif

#include "umdevxs_pcidev.h"

#include "basic_defs.h"             // IDENTIFIER_NOT_USED


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Device administration structure
typedef struct
{
    // name string used in Device_Find
    const char * DeviceName_p;

    // device offset range memory map
    unsigned int StartByteOffset;
    unsigned int LastByteOffset;

    int Bar;        // only for PCI devices
    int Flags;      // 0 = Normal, 1 = PCI, 2 = Simulation, 3 = Open Firmware
} UMDevXS_DeviceDef_t;

// macro used in cs_umdevxs.h
#define UMDEVXS_DEVICE_ADD(_name, _start, _last) { _name, _start, _last, 0, 0 }

#ifndef UMDEVXS_REMOVE_PCI
#define UMDEVXS_DEVICE_ADD_PCI(_name, _bar, _start, _size) \
            { _name, _start, (_start + _size -1), _bar, 1/*=PCI*/ }
#else
/* empty due to comma! */
#define UMDEVXS_DEVICE_ADD_PCI(_name, _bar, _start, _size) { 0 }
#endif /* UMDEVXS_REMOVE_PCI */

#ifndef UMDEVXS_REMOVE_SIMULATION
#define UMDEVXS_DEVICE_ADD_SIM(_name, _size) \
                                { _name, 0, (_size-1), 0, 2/*=Simulation*/ }
#else
/* empty due to comma! */
#define UMDEVXS_DEVICE_ADD_SIM(_name, _size) { 0 }
#endif /* UMDEVXS_REMOVE_SIMULATION */

#ifndef UMDEVXS_REMOVE_DEVICE_OF
#define UMDEVXS_DEVICE_ADD_OF(_name, _start, _last) \
                                { _name, _start, _last, 0, 3 }
#else
#define UMDEVXS_DEVICE_ADD_OF(_name, _start, _last) { 0 }
#endif


/*----------------------------------------------------------------------------
 * Local variables
 */

static const UMDevXS_DeviceDef_t UMDevXS_Devices[] =
{
#ifndef UMDEVXS_REMOVE_PCI
    // support for up to 4 memory windows
    UMDEVXS_DEVICE_ADD_PCI(
        "PCI.0", 0,
        UMDEVXS_PCI_BAR0_SUBSET_START,
        UMDEVXS_PCI_BAR0_SUBSET_SIZE),
    UMDEVXS_DEVICE_ADD_PCI(
        "PCI.1", 1,
        UMDEVXS_PCI_BAR1_SUBSET_START,
        UMDEVXS_PCI_BAR1_SUBSET_SIZE),
    UMDEVXS_DEVICE_ADD_PCI(
        "PCI.2", 2,
        UMDEVXS_PCI_BAR2_SUBSET_START,
        UMDEVXS_PCI_BAR2_SUBSET_SIZE),
    UMDEVXS_DEVICE_ADD_PCI(
        "PCI.3", 3,
        UMDEVXS_PCI_BAR3_SUBSET_START,
        UMDEVXS_PCI_BAR3_SUBSET_SIZE),
#endif /* UMDEVXS_REMOVE_PCI */
    UMDEVXS_DEVICES
};

// number of devices supported calculated on HWPAL_DEVICES defined
// in cs_linux_pci_x86.h
#define UMDEVXS_DEVICE_COUNT (sizeof(UMDevXS_Devices) \
                                     / sizeof(UMDevXS_DeviceDef_t))

#ifdef UMDEVXS_ENABLE_DEVICE_LOCK

#ifdef HWPAL_LOCK_SLEEPABLE
static struct mutex HWPAL_Lock;
#else
static spinlock_t HWPAL_SpinLock;
#endif

/* Application currently owning the specified page of the device register
   set.  */
static void *
UMDevXS_Device_Owner[UMDEVXS_DEVICE_COUNT][UMDEVXS_DEVICE_LOCK_MAX_SIZE/PAGE_SIZE];

/* Application (if any) currently being the privileged application. */
static void *UMDevXS_PrivilegedAppID;


/*----------------------------------------------------------------------------
 * UMDevXS_DeviceLock_Init
 */
void
UMDevXS_DeviceLock_Init(void)
{
#ifdef HWPAL_LOCK_SLEEPABLE
        LOG_INFO(
            "UMDevXS_Device_Lock: "
            "Lock = mutex\n");
        mutex_init(&HWPAL_Lock);
#else
        LOG_INFO(
            "UMDevXS_Device_Lock: "
            "Lock = spinlock\n");
        spin_lock_init(&HWPAL_SpinLock);
#endif
}


/*----------------------------------------------------------------------------
 * UMDevXS_Device_SetPrivileged
 */
int
UMDevXS_Device_SetPrivileged(
        void *AppID)
{
#ifndef HWPAL_LOCK_SLEEPABLE
    unsigned long flags;
#endif
#ifdef HWPAL_LOCK_SLEEPABLE
    mutex_lock(&HWPAL_Lock);
#else
    spin_lock_irqsave(&HWPAL_SpinLock, flags);
#endif

    if (UMDevXS_PrivilegedAppID != NULL)
    {
#ifdef HWPAL_LOCK_SLEEPABLE
        mutex_unlock(&HWPAL_Lock);
#else
        spin_unlock_irqrestore(&HWPAL_SpinLock, flags);
#endif
        LOG_CRIT("UMDevXS_Device_SetPrivileged failed\n");
        return -1;
    }
    UMDevXS_PrivilegedAppID = AppID;
#ifdef HWPAL_LOCK_SLEEPABLE
    mutex_unlock(&HWPAL_Lock);
#else
    spin_unlock_irqrestore(&HWPAL_SpinLock, flags);
#endif
    LOG_CRIT("UMDevXS_Device_SetPrivileged succeeded\n");
    return 0;
}


/*----------------------------------------------------------------------------
 * UMDevXS_Device_LockRange
 */
int
UMDevXS_Device_LockRange(
        unsigned int DeviceNr,
        unsigned int offset,
        unsigned int size,
        void * AppID)
{
    unsigned int MinPage, MaxPage, i;

#ifndef HWPAL_LOCK_SLEEPABLE
    unsigned long flags;
#endif

    if (DeviceNr >= UMDEVXS_DEVICE_COUNT ||
        offset >= UMDEVXS_DEVICE_LOCK_MAX_SIZE ||
        offset + size > UMDEVXS_DEVICE_LOCK_MAX_SIZE ||
        (offset & (PAGE_SIZE-1)) != 0 ||
        (size & (PAGE_SIZE-1)) != 0)
    {
        LOG_CRIT("UMDevXS_Device_LockRange: Invalid device or range, "
                 "dev=%u, offset=0x%08x, size=0x%08x, "
                 "devcount=%u, maxoffs=%u\n",
                 DeviceNr,
                 offset,
                 size,
                 (unsigned int)UMDEVXS_DEVICE_COUNT,
                 (unsigned int)UMDEVXS_DEVICE_LOCK_MAX_SIZE);
        return -1;
    }

    MinPage = offset / PAGE_SIZE;
    MaxPage = (offset + size - 1) / PAGE_SIZE;

#ifdef HWPAL_LOCK_SLEEPABLE
    mutex_lock(&HWPAL_Lock);
#else
    spin_lock_irqsave(&HWPAL_SpinLock, flags);
#endif

    if (UMDevXS_PrivilegedAppID != AppID)
    {

        for (i = MinPage; i < MaxPage+1; i++)
        {
            if (UMDevXS_Device_Owner[DeviceNr][i] != NULL &&
                UMDevXS_Device_Owner[DeviceNr][i] != AppID)
            {
#ifdef HWPAL_LOCK_SLEEPABLE
                mutex_unlock(&HWPAL_Lock);
#else
                spin_unlock_irqrestore(&HWPAL_SpinLock, flags);
#endif
                LOG_CRIT("UMDevXS_Device_LockRange: "
                "Device %u already in use\n",
                         DeviceNr);
                return -1;
            }
        }

        for (i = MinPage; i < MaxPage+1; i++)
        {
            UMDevXS_Device_Owner[DeviceNr][i] = AppID;
        }
    }
#ifdef HWPAL_LOCK_SLEEPABLE
    mutex_unlock(&HWPAL_Lock);
#else
    spin_unlock_irqrestore(&HWPAL_SpinLock, flags);
#endif

    return 0;
}


/*----------------------------------------------------------------------------
 * UMDevXS_Device_LockRangeIndex
 *
 * Lock a range in the device specified by Index and mark it as owned by
 * the application specified by AppID.
 *
 * Return 0 if successful, -1 if already locked or invalid device.
 */
int
UMDevXS_Device_LockRangeIndex(
        unsigned int Index,
        unsigned int Size,
        void * AppID)
{
    unsigned int DeviceNr = Index & MASK_5_BITS;
    unsigned int Offset = (Index >> 5) * PAGE_SIZE;

    return UMDevXS_Device_LockRange(DeviceNr,Offset,Size,AppID);
}


/*----------------------------------------------------------------------------
 * UMDevXS_Device_Unlock
 *
 * Unlock all device pages owned by the application specified by AppID.
 */
void
UMDevXS_Device_Unlock(
       void * AppID)
{
    unsigned int i, j;

#ifndef HWPAL_LOCK_SLEEPABLE
    unsigned long flags;
#endif

#ifdef HWPAL_LOCK_SLEEPABLE
    mutex_lock(&HWPAL_Lock);
#else
    spin_lock_irqsave(&HWPAL_SpinLock, flags);
#endif

    for (i=0; i<UMDEVXS_DEVICE_COUNT; i++)
        for (j=0; j<UMDEVXS_DEVICE_LOCK_MAX_SIZE / PAGE_SIZE; j++)
            if (UMDevXS_Device_Owner[i][j] == AppID)
                UMDevXS_Device_Owner[i][j] = NULL;

    if (UMDevXS_PrivilegedAppID == AppID)
        UMDevXS_PrivilegedAppID = NULL;

#ifdef HWPAL_LOCK_SLEEPABLE
    mutex_unlock(&HWPAL_Lock);
#else
    spin_unlock_irqrestore(&HWPAL_SpinLock, flags);
#endif
}
#endif


/*----------------------------------------------------------------------------
 * UMDevXSLib_Device_IsValidNameField
 *
 * Check that the Name field is zero-terminated within the required limit and
 * only contains ascii characters.
 *
 * Returns true when the name is valid.
 */
static bool
UMDevXSLib_Device_IsValidNameField(
        const char * Name_p)
{
    int i;

    for (i = 0; i <= UMDEVXS_CMDRSP_MAXLEN_NAME; i++)
    {
        const uint8_t c = (uint8_t)Name_p[i];

        // check for zero-termination
        if (c == 0)
            return true;        // ## RETURN ##

        // check for invalid characters
        if (c < 32 || c > 127)
            return false;
    } // for

    // did not find zero-terminator
    return false;
}


/*----------------------------------------------------------------------------
 * UMDevXS_Device_HandleCmd_Find
 */
void
UMDevXS_Device_HandleCmd_Find(
        UMDevXS_CmdRsp_t * const CmdRsp_p)
{
    int i;

    // sanity-check the name
    // it must be zero-terminated
    if (!UMDevXSLib_Device_IsValidNameField(CmdRsp_p->szName))
    {
        CmdRsp_p->Error = 1;
        return;
    }

    LOG_INFO(
        UMDEVXS_LOG_PREFIX
        "UMDevXS_Device_HandleCmd_Find: "
        "Name=%s\n",
        CmdRsp_p->szName);

    for (i = 0; i < UMDEVXS_DEVICE_COUNT; i++)
    {
        const UMDevXS_DeviceDef_t * const p = UMDevXS_Devices + i;

        // protect again potential empty records
        // caused by incomplete initializers
        if (p->DeviceName_p == NULL)
            continue;

        if (strcmp(CmdRsp_p->szName, p->DeviceName_p) == 0)
        {
            // found it!

            LOG_INFO(
                UMDEVXS_LOG_PREFIX
                "UMDevXS_Device_HandleCmd_Find: "
                "Match on device %d [flag=%d]\n",
                i, p->Flags);

            // fill in the result fields
            CmdRsp_p->Handle = UMDevXS_Handle_Make(
                                        UMDEVXS_HANDLECLASS_DEVICE,
                                        i);

            CmdRsp_p->uint1 = p->LastByteOffset - p->StartByteOffset + 1;

            // forward PCI devices requests
            if (p->Flags == 1)
            {
#ifndef UMDEVXS_REMOVE_PCI
                UMDevXS_PCIDev_HandleCmd_Find(
                            CmdRsp_p,
                            p->Bar,
                            p->StartByteOffset);
#else
                CmdRsp_p->Handle = 0;
                CmdRsp_p->uint1 = 0;
                CmdRsp_p->Error = 6;
#endif /* UMDEVXS_REMOVE_PCI */
            }

            if (p->Flags == 2)
            {
#ifndef UMDEVXS_REMOVE_SIMULATION
                UMDevXS_SimDev_HandleCmd_Find(
                            CmdRsp_p,
                            (void *)p,
                            p->LastByteOffset);
#else
                CmdRsp_p->Handle = 0;
                CmdRsp_p->uint1 = 0;
                CmdRsp_p->Error = 6;
#endif /* UMDEVXS_REMOVE_SIMULATION */
            }

            return; // ## RETURN ##
        }
    } // for

    // not found
    LOG_WARN(
        UMDEVXS_LOG_PREFIX
        "Failed to locate device with name '%s'\n",
        CmdRsp_p->szName);

    CmdRsp_p->Error = 1;
}


/*----------------------------------------------------------------------------
 * UMDevXS_Device_HandleCmd_FindRange
 */
void
UMDevXS_Device_HandleCmd_FindRange(
        UMDevXS_CmdRsp_t * const CmdRsp_p)
{
    unsigned int DeviceNr, Offset;

    DeviceNr = CmdRsp_p->uint1;
    Offset   = CmdRsp_p->uint2;

    CmdRsp_p->Handle = UMDevXS_Handle_Make(UMDEVXS_HANDLECLASS_DEVICE,
                                           DeviceNr + ((Offset/PAGE_SIZE)<<5));
}


/*----------------------------------------------------------------------------
 * UMDevXS_Device_HandleCmd_SetPrivileged
 */
void
UMDevXS_Device_HandleCmd_SetPrivileged(
        void * AppID,
        UMDevXS_CmdRsp_t * const CmdRsp_p)
{
#ifdef UMDEVXS_ENABLE_DEVICE_LOCK
    int res;
    res = UMDevXS_Device_SetPrivileged(AppID);
    if (res < 0)
    {
        CmdRsp_p->Error = 1;
    }
#endif
}


/*----------------------------------------------------------------------------
 * UMDevXS_Device_HandleCmd_Enum
 */
void
UMDevXS_Device_HandleCmd_Enum(
        UMDevXS_CmdRsp_t * const CmdRsp_p)
{
    unsigned int DeviceNr = CmdRsp_p->uint1;

    if (DeviceNr >= UMDEVXS_DEVICE_COUNT)
    {
        // unsupported device number
        CmdRsp_p->Error = 1;
        return;
    }

    {
        const UMDevXS_DeviceDef_t * const p = UMDevXS_Devices + DeviceNr;

        if (p->DeviceName_p == NULL)
        {
            // accidentally empty entry
            // return empty name, but no error
            CmdRsp_p->szName[0] = 0;
        }
        else
        {
            strncpy(
                CmdRsp_p->szName,
                p->DeviceName_p,
                UMDEVXS_CMDRSP_MAXLEN_NAME+1);
        }
    }
}


/*----------------------------------------------------------------------------
 * UMDevXS_Device_Map
 */
int
UMDevXS_Device_Map(
        unsigned int Index,
        unsigned int Length,
        struct vm_area_struct * vma_p)
{
    const UMDevXS_DeviceDef_t * p;
    unsigned int DeviceNr = Index & MASK_5_BITS;
    unsigned int Offset = (Index >> 5) * PAGE_SIZE;
    unsigned long address = 0;

    if (DeviceNr >= UMDEVXS_DEVICE_COUNT)
        return -1;

    p = UMDevXS_Devices + DeviceNr;

    // for mapping requests for PCI resources
    if (p->Flags == 1)
    {
#ifndef UMDEVXS_REMOVE_PCI
        int res;

        res = UMDevXS_PCIDev_Map(
                    p->Bar,
                    p->StartByteOffset + Offset,
                    p->LastByteOffset - p->StartByteOffset + 1 - Offset,
                    Length,
                    vma_p);

        if (res != 0)
        {
            LOG_INFO(
                UMDEVXS_LOG_PREFIX
                "UMDevXS_Device_Map: "
                "UMDevXS_PCIDev_Map returned %d\n",
                res);
        }

        return res;
#else
        return -2;
#endif /* UMDEVXS_REMOVE_PCI */
    }

    if (p->Flags == 2)
    {
#ifndef UMDEVXS_REMOVE_SIMULATION
        int res;

        res = UMDevXS_SimDev_Map((void *)p, &address);
        if (res != 0)
        {
            LOG_INFO(
                UMDEVXS_LOG_PREFIX
                "UMDevXS_Device_Map: "
                "UMDevXS_SimDev_Map returned %d\n",
                res);

            return res;
        }

        // address is filled in; mapping is done below
#else
        return -2;
#endif /* UMDEVXS_REMOVE_SIMULATION */
    }

    if (p->Flags == 3)
    {
#ifndef UMDEVXS_REMOVE_DEVICE_OF
        int res;

        res = UMDevXS_OFDev_Map(
                        p->StartByteOffset + Offset,
                        p->LastByteOffset - p->StartByteOffset + 1 - Offset,
                        Length,
                        vma_p);

        if (res != 0)
        {
            LOG_INFO(
                UMDEVXS_LOG_PREFIX
                "UMDevXS_Device_Map: "
                "Failed UMDevXS_OFDev_Map, error %d\n",
                res);
        }

        return res;
#else
        return -2;
#endif /* UMDEVXS_REMOVE_DEVICE_OF */
    }
    else
    {
        // normal case (not PCI, Simulation device or OF)
        address = p->StartByteOffset;

        // honor (or reject) length request
        if (Length > p->LastByteOffset - p->StartByteOffset + 1)
        {
            LOG_CRIT(
                UMDEVXS_LOG_PREFIX
                "UMDevXS_Device_Map: "
                "failed, length rejected (%u)\n",
                Length);

            return -1;
        }
    }

    {
        int res;

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
            LOG_INFO(
                UMDEVXS_LOG_PREFIX
                "UMDevXS_Device_Map: "
                "remap_pfn_range failed (%d)\n",
                res);

            return res;
        }
    }

    // return success
    return 0;
}


#endif /* UMDEVXS_REMOVE_DEVICE */


/* end of file umdevxs_device.c */
