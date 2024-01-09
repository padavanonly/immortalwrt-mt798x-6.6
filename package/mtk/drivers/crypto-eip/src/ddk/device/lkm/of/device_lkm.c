/* device_lkm.c
 *
 * This is the Linux Kernel-mode Driver Framework v4 Device API
 * implementation for open Firmware. The implementation is device-agnostic and
 * receives configuration details from the c_device_lkm.h file.
 *
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "device_mgmt.h"            // API to implement
#include "device_rw.h"              // API to implement

// Driver Framework Device Internal interface
#include "device_internal.h"        // Device_Internal_*


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_device_lkm.h"

// Driver Framework Device API
#include "device_swap.h"            // Device_SwapEndian32

// Driver Framework Device Platform interface
#include "device_platform.h"        // Device_Platform_*

#ifndef HWPAL_USE_UMDEVXS_DEVICE
#ifdef HWPAL_DEVICE_USE_RPM
// Runtime Power Management Kernel Macros API
#include "rpm_kernel_macros.h"      // RPM_*
#endif
#endif

// Logging API
#include "log.h"                    // LOG_*

// Driver Framework C Run-Time Library API
#include "clib.h"                   // memcmp

// Driver Framework Basic Definitions API
#include "basic_defs.h"             // uint32_t, NULL, inline, bool,
                                    // IDENTIFIER_NOT_USED

#ifdef HWPAL_USE_UMDEVXS_DEVICE
#include "umdevxs_ofdev.h"
#endif

#ifndef HWPAL_USE_UMDEVXS_DEVICE
// Linux Kernel Module interface
#include "lkm.h"
#endif

// Linux Kernel API
#include <linux/platform_device.h>  // platform_*,
#include <asm/io.h>                 // ioread32, iowrite32
#include <linux/compiler.h>         // __iomem
#include <linux/slab.h>             // kmalloc, kfree
#include <linux/hardirq.h>          // in_atomic


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define HWPAL_FLAG_READ     BIT_0   // 1
#define HWPAL_FLAG_WRITE    BIT_1   // 2
#define HWPAL_FLAG_SWAP     BIT_2   // 4
#define HWPAL_FLAG_HA       BIT_5   // 32

// c_device_lkm.h file defines a HWPAL_REMAP_ADDRESSES that
// depends on HWPAL_REMAP_ONE
#define HWPAL_REMAP_ONE(_old, _new) \
    case _old: \
        DeviceByteOffset = _new; \
        break;


/*----------------------------------------------------------------------------
 * Local variables
 */

// statically configured devices
static const Device_Admin_Static_t HWPALLib_Devices_Static[] =
{
    HWPAL_DEVICES
};

// number of statically configured devices
#define HWPAL_DEVICE_STATIC_COUNT \
                (sizeof(HWPALLib_Devices_Static) \
                            / sizeof(Device_Admin_Static_t))

// All supported devices
static Device_Admin_t * HWPALLib_Devices_p [HWPAL_DEVICE_COUNT];

// Global administration data
static Device_Global_Admin_t HWPALLib_Device_Global;


/*----------------------------------------------------------------------------
 * HWPAL_Hexdump
 *
 * This function hex-dumps an array of uint32_t.
 */
#if ((defined(HWPAL_TRACE_DEVICE_READ)) || (defined(HWPAL_TRACE_DEVICE_WRITE)))
static void
HWPAL_Hexdump(
        const char * ArrayName_p,
        const char * DeviceName_p,
        const unsigned int ByteOffset,
        const uint32_t * WordArray_p,
        const unsigned int WordCount,
        bool fSwapEndianness)
{
    unsigned int i;

    Log_FormattedMessage(
        "%s: "
        "byte offsets 0x%x - 0x%x"
        " (%s)\n"
        "  ",
        ArrayName_p,
        ByteOffset,
        ByteOffset + WordCount*4 -1,
        DeviceName_p);

    for (i = 1; i <= WordCount; i++)
    {
        uint32_t Value = WordArray_p[i - 1];

        if (fSwapEndianness)
            Value = Device_SwapEndian32(Value);

        Log_FormattedMessage(" 0x%08x", Value);

        if ((i & 7) == 0)
            Log_Message("\n  ");
    }

    if ((WordCount & 7) != 0)
        Log_Message("\n");
}
#endif


/*----------------------------------------------------------------------------
 * Device_RemapDeviceAddress
 *
 * This function remaps certain device addresses (relative within the whole
 * device address map) to other addresses. This is needed when the integration
 * has remapped some EIP device registers to other addresses. The EIP Driver
 * Libraries assume the devices always have the same internal layout.
 */
static inline unsigned int
Device_RemapDeviceAddress(
        unsigned int DeviceByteOffset)
{
#ifdef HWPAL_REMAP_ADDRESSES
    switch(DeviceByteOffset)
    {
        // include the remap statements
        HWPAL_REMAP_ADDRESSES

        default:
            break;
    }
#endif

    return DeviceByteOffset;
}


/*----------------------------------------------------------------------------
 * HWPALLib_Device2RecPtr
 *
 * This function converts an Device_Handle_t received via one of the
 * Device API functions into a HWPALLib_Devices_p record pointer, if it is
 * valid.
 *
 * Return Value
 *     NULL    Provided Device Handle was not valid
 *     other   Pointer to a Device_Admin_t record
 */
static inline Device_Admin_t *
HWPALLib_Device2RecordPtr(
        Device_Handle_t Device)
{
    Device_Admin_t * p = (void *)Device;

    if (p == NULL)
        return NULL;

#ifdef HWPAL_DEVICE_MAGIC
    if (p->Magic != HWPAL_DEVICE_MAGIC)
        return NULL;
#endif

    return p;
}


/*----------------------------------------------------------------------------
 * HWPALLib_IsValid
 *
 * This function checks that the parameters are valid to make the access.
 *
 * Device_p is valid
 * ByteOffset is 32-bit aligned
 * ByteOffset is inside device memory range
 */
static inline bool
HWPALLib_IsValid(
        const Device_Admin_t * const Device_p,
        const unsigned int ByteOffset)
{
    if (Device_p == NULL)
        return false;

    if (ByteOffset & 3)
        return false;

    if (Device_p->FirstOfs + ByteOffset > Device_p->LastOfs)
        return false;

    return true;
}


/*-----------------------------------------------------------------------------
 * device_internal interface
 *
 */

/*----------------------------------------------------------------------------
 * Device_Internal_Static_Count_Get
 */
unsigned int
Device_Internal_Static_Count_Get(void)
{
    return HWPAL_DEVICE_STATIC_COUNT;
}


/*----------------------------------------------------------------------------
 * Device_Internal_Count_Get
 */
unsigned int
Device_Internal_Count_Get(void)
{
    return HWPAL_DEVICE_COUNT;
}


/*----------------------------------------------------------------------------
 * Device_Internal_Admin_Static_Get
 */
const Device_Admin_Static_t *
Device_Internal_Admin_Static_Get(void)
{
    return HWPALLib_Devices_Static;
}


/*----------------------------------------------------------------------------
 * Device_Internal_Admin_Get
 *
 * Returns pointer to the memory location where the device list is stored.
 *
 */
Device_Admin_t **
Device_Internal_Admin_Get(void)
{
    return HWPALLib_Devices_p;
}


/*----------------------------------------------------------------------------
 * Device_Internal_Admin_Global_Get
 */
Device_Global_Admin_t *
Device_Internal_Admin_Global_Get(void)
{
    return &HWPALLib_Device_Global;
}


/*----------------------------------------------------------------------------
 * Device_Internal_Alloc
 */
void *
Device_Internal_Alloc(
        unsigned int ByteCount)
{
    return kmalloc(ByteCount, in_atomic() ? GFP_ATOMIC : GFP_KERNEL);
}


/*----------------------------------------------------------------------------
 * Device_Internal_Free
 */
void
Device_Internal_Free(
        void * Ptr)
{
    kfree(Ptr);
}


/*-----------------------------------------------------------------------------
 * Device_Internal_Initialize
 */
int
Device_Internal_Initialize(
        void * CustomInitData_p)
{
#ifndef HWPAL_USE_UMDEVXS_DEVICE
    unsigned int i;
    LKM_Init_t  LKMInit;

    ZEROINIT(LKMInit);

    LKMInit.DriverName_p        = HWPAL_DRIVER_NAME;
    LKMInit.ResId               = HWPAL_DEVICE_RESOURCE_ID;
    LKMInit.ResByteCount        = HWPAL_DEVICE_RESOURCE_BYTE_COUNT;
    LKMInit.fRetainMap          = true;

#ifdef HWPAL_DEVICE_USE_RPM
    LKMInit.PM_p                = RPM_OPS_PM;
#endif

    if (LKM_Init(&LKMInit) < 0)
    {
        LOG_CRIT("%s: Failed to register the platform device\n", __func__);
        return -1;
    }

    // Output default IRQ line number in custom data
    {
        int * p = (int *)CustomInitData_p;

        int * IRQ_p = (int *)LKMInit.CustomInitData_p;

        *p = IRQ_p[0];
    }

#ifdef HWPAL_DEVICE_USE_RPM
    if (RPM_INIT_MACRO(LKM_DeviceGeneric_Get()) != RPM_SUCCESS)
    {
        LOG_CRIT("%s: RPM_Init() failed\n", __func__);
        LKM_Uninit();
        return -3; // error
    }
#endif

    HWPALLib_Device_Global.Platform.MappedBaseAddr_p
                                        = LKM_MappedBaseAddr_Get();
    HWPALLib_Device_Global.Platform.Platform_Device_p
                                        = LKM_DeviceSpecific_Get();
    HWPALLib_Device_Global.Platform.PhysBaseAddr
                                        = LKM_PhysBaseAddr_Get();

    for(i = 0; i < HWPAL_DEVICE_COUNT; i++)
        if (HWPALLib_Devices_p[i])
        {
            LOG_INFO("%s: mapped device '%s', "
                     "virt base addr 0x%p, "
                     "start byte offset 0x%x, "
                     "last byte offset 0x%x\n",
                     __func__,
                     HWPALLib_Devices_p[i]->DevName,
                     HWPALLib_Device_Global.Platform.MappedBaseAddr_p,
                     HWPALLib_Devices_p[i]->FirstOfs,
                     HWPALLib_Devices_p[i]->LastOfs);
        }
#else
    UMDevXS_OFDev_GetDevice(
           0,
           &HWPALLib_Device_Global.Platform.Platform_Device_p,
           (void __iomem **)&HWPALLib_Device_Global.Platform.MappedBaseAddr_p);

    if (HWPALLib_Device_Global.Platform.Platform_Device_p == NULL ||
            HWPALLib_Device_Global.Platform.MappedBaseAddr_p == NULL)
    {
        LOG_CRIT("%s: Failed, UMDevXS device %p, mapped address %p\n",
                 __func__,
                 HWPALLib_Device_Global.Platform.Platform_Device_p,
                 HWPALLib_Device_Global.Platform.MappedBaseAddr_p);
        return -1;
    }

    {
        int * p = (int *)CustomInitData_p;

        // Exported under GPL
        *p = platform_get_irq(HWPALLib_Device_Global.Platform.Platform_Device_p,
                              HWPAL_PLATFORM_IRQ_IDX);
    }
#endif // HWPAL_USE_UMDEVXS_DEVICE

    return 0;
}


/*-----------------------------------------------------------------------------
 * Device_Internal_UnInitialize
 */
void
Device_Internal_UnInitialize(void)
{
#ifndef HWPAL_USE_UMDEVXS_DEVICE

#ifdef HWPAL_DEVICE_USE_RPM
    // Check if a race condition is possible here with auto-suspend timer
    (void)RPM_UNINIT_MACRO();
#endif

    LKM_Uninit();
#endif

    // Reset global administration
    ZEROINIT(HWPALLib_Device_Global);
}


/*-----------------------------------------------------------------------------
 * Device_Internal_Find
 */
Device_Handle_t
Device_Internal_Find(
        const char * DeviceName_p,
        const unsigned int Index)
{
    IDENTIFIER_NOT_USED(DeviceName_p);

    // Return the device handle
    return (Device_Handle_t)HWPALLib_Devices_p[Index];
}


/*-----------------------------------------------------------------------------
 * Device_Internal_GetIndex
 */
int
Device_Internal_GetIndex(
        const Device_Handle_t Device)
{
    Device_Admin_t * Device_p;

#ifdef HWPAL_STRICT_ARGS_CHECK
    Device_p = HWPALLib_Device2RecordPtr(Device);
#else
    Device_p = Device;
#endif

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (!HWPALLib_IsValid(Device_p, 0))
    {
        LOG_CRIT("%s: invalid device (%p) or ByteOffset (%u)\n",
                 __func__,
                 Device,
                 0);
        return -1;
    }
#endif

    return Device_p->DeviceId;
}


/*-----------------------------------------------------------------------------
 * device_mgmt API
 *
 * These functions support finding a device given its name.
 * A handle is returned that is needed in the device_rw API
 * to read or write the device
 */


/*-----------------------------------------------------------------------------
 * Device_GetReference
 */
Device_Reference_t
Device_GetReference(
        const Device_Handle_t Device,
        Device_Data_t * const Data_p)
{
    Device_Reference_t DevReference;

    // There exists only one reference for this implementation
    IDENTIFIER_NOT_USED(Device);

    if (!HWPALLib_Device_Global.fInitialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return NULL;
    }

    // Return the platform device reference
    // (pointer to the Linux device structure)
    DevReference = &HWPALLib_Device_Global.Platform.Platform_Device_p->dev;

    if (Data_p)
        Data_p->PhysAddr = NULL;

    return DevReference;
}


/*-----------------------------------------------------------------------------
 * device_rw API
 *
 * These functions can be used to transfer a single 32bit word or an array of
 * 32bit words to or from a device.
 * Endianness swapping is performed on the fly based on the configuration for
 * this device.
 *
 */

/*-----------------------------------------------------------------------------
 * Device_Read32
 */
uint32_t
Device_Read32(
        const Device_Handle_t Device,
        const unsigned int ByteOffset)
{
    uint32_t Value = 0;
    Device_Read32Check(Device, ByteOffset, &Value);
    return Value;
}


/*-----------------------------------------------------------------------------
 * Device_Read32Check
 */
int
Device_Read32Check(
        const Device_Handle_t Device,
        const unsigned int ByteOffset,
        uint32_t * const Value_p)
{
    Device_Admin_t * Device_p;
    uint32_t Value = 0;

    if (!HWPALLib_Device_Global.fInitialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return DEVICE_RW_PARAM_ERROR;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    Device_p = HWPALLib_Device2RecordPtr(Device);
#else
    Device_p = Device;
#endif

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (!HWPALLib_IsValid(Device_p, ByteOffset))
    {
        LOG_CRIT("%s: invalid device (%p) or ByteOffset (%u)\n",
                 __func__,
                 Device,
                 ByteOffset);
        return DEVICE_RW_PARAM_ERROR;
    }
#endif

#ifdef HWPAL_ENABLE_HA_SIMULATION
    if (Device_p->Flags & HWPAL_FLAG_HA)
    {
        // HA simulation mode
        // disable access to PKA_MASTER_SEQ_CTRL
        if (ByteOffset == 0x3FC8)
        {
            Value = 0;
            goto HA_SKIP;
        }
    }
#endif

    {
        unsigned int DeviceByteOffset = Device_p->FirstOfs + ByteOffset;

        DeviceByteOffset = Device_RemapDeviceAddress(DeviceByteOffset);

#ifdef HWPAL_DEVICE_DIRECT_MEMIO
        Value = *(uint32_t *)(uintptr_t)(HWPALLib_Device_Global.Platform.MappedBaseAddr_p +
                                                (DeviceByteOffset / 4));
#else
        Value = ioread32(HWPALLib_Device_Global.Platform.MappedBaseAddr_p +
                                                (DeviceByteOffset / 4));
#endif

#ifdef HWPAL_DEVICE_ENABLE_SWAP
        if (Device_p->Flags & HWPAL_FLAG_SWAP)
            Value = Device_SwapEndian32(Value);
#endif

        smp_rmb();
    }

#ifdef HWPAL_ENABLE_HA_SIMULATION
HA_SKIP:
#endif

#ifdef HWPAL_TRACE_DEVICE_READ
    if (Device_p->Flags & HWPAL_FLAG_READ)
    {
        unsigned int DeviceByteOffset = Device_p->FirstOfs + ByteOffset;
        unsigned int DeviceByteOffset2 =
                Device_RemapDeviceAddress(DeviceByteOffset);

        if (DeviceByteOffset2 != DeviceByteOffset)
        {
            DeviceByteOffset2 -= Device_p->FirstOfs;
            Log_FormattedMessage("%s: 0x%x(was 0x%x) = 0x%08x (%s)\n",
                                 __func__,
                                 DeviceByteOffset2,
                                 ByteOffset,
                                 (unsigned int)Value,
                                 Device_p->DevName);
        }
        else
            Log_FormattedMessage("%s: 0x%x = 0x%08x (%s)\n",
                                 __func__,
                                 ByteOffset,
                                 (unsigned int)Value,
                                 Device_p->DevName);
    }
#endif /* HWPAL_TRACE_DEVICE_READ */

    *Value_p = Value;

    return 0;
}


/*-----------------------------------------------------------------------------
 * Device_Write32
 */
int
Device_Write32(
        const Device_Handle_t Device,
        const unsigned int ByteOffset,
        const uint32_t ValueIn)
{
    Device_Admin_t * Device_p;
    uint32_t Value = ValueIn;

    if (!HWPALLib_Device_Global.fInitialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return DEVICE_RW_PARAM_ERROR;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    Device_p = HWPALLib_Device2RecordPtr(Device);
#else
    Device_p = Device;
#endif

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (!HWPALLib_IsValid(Device_p, ByteOffset))
    {
        LOG_CRIT("%s: Invalid Device (%p) or ByteOffset (%u)\n",
                 __func__,
                 Device,
                 ByteOffset);
        return DEVICE_RW_PARAM_ERROR;
    }
#endif

#ifdef HWPAL_TRACE_DEVICE_WRITE
    if (Device_p->Flags & HWPAL_FLAG_WRITE)
        Log_FormattedMessage("%s: 0x%x = 0x%08x (%s)\n",
                             __func__,
                             ByteOffset,
                             (unsigned int)Value,
                             Device_p->DevName);
#endif /* HWPAL_TRACE_DEVICE_WRITE*/

#ifdef HWPAL_ENABLE_HA_SIMULATION
    if (Device_p->Flags & HWPAL_FLAG_HA)
    {
        // HA simulation mode
        // disable access to PKA_MASTER_SEQ_CTRL
        if (ByteOffset == 0x3FC8)
        {
            LOG_CRIT("%s: Unexpected write to PKA_MASTER_SEQ_CTRL\n",
                        __func__);
            return 0;
        }
    }
#endif

    {
        uint32_t DeviceByteOffset = Device_p->FirstOfs + ByteOffset;

        DeviceByteOffset = Device_RemapDeviceAddress(DeviceByteOffset);

#ifdef HWPAL_DEVICE_ENABLE_SWAP
        if (Device_p->Flags & HWPAL_FLAG_SWAP)
            Value = Device_SwapEndian32(Value);
#endif

#ifdef HWPAL_DEVICE_DIRECT_MEMIO
        *(uint32_t *)(uintptr_t)(HWPALLib_Device_Global.Platform.MappedBaseAddr_p +
                                        (DeviceByteOffset / 4)) = Value;
#else
        iowrite32(Value,
                  HWPALLib_Device_Global.Platform.MappedBaseAddr_p +
                                                  (DeviceByteOffset / 4));
#endif

        smp_wmb();
    }

    return 0;
}


/*-----------------------------------------------------------------------------
 * Device_Read32Array
 */
int
Device_Read32Array(
        const Device_Handle_t Device,
        const unsigned int StartByteOffset, // read starts here, +4 increments
        uint32_t * MemoryDst_p,             // writing starts here
        const int Count)                    // number of uint32's to transfer
{
    Device_Admin_t * Device_p;
    unsigned int DeviceByteOffset;

    if (!HWPALLib_Device_Global.fInitialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return DEVICE_RW_PARAM_ERROR;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    Device_p = HWPALLib_Device2RecordPtr(Device);
#else
    Device_p = Device;
#endif

#ifdef HWPAL_STRICT_ARGS_CHECK
    if ((Count <= 0) ||
        MemoryDst_p == NULL ||
        !HWPALLib_IsValid(Device_p, StartByteOffset) ||
        !HWPALLib_IsValid(Device_p, StartByteOffset + (Count - 1) * 4))
    {
        LOG_CRIT("%s: Invalid Device (%p) or read area (%u-%u)\n",
                 __func__,
                 Device,
                 StartByteOffset,
                 (unsigned int)(StartByteOffset +
                          (Count - 1) * sizeof(uint32_t)));
         return DEVICE_RW_PARAM_ERROR;
    }
#endif

#ifdef HWPAL_ENABLE_HA_SIMULATION
    if (Device_p->Flags & HWPAL_FLAG_HA)
    {
        // HA simulation mode
        // disable access to PKA_MASTER_SEQ_CTRL
        return 0;
    }
#endif

    DeviceByteOffset = Device_p->FirstOfs + StartByteOffset;

    {
        unsigned int RemappedOffset;
        uint32_t Value;
        int i;

#ifdef HWPAL_DEVICE_ENABLE_SWAP
        bool fSwap = false;

        if (Device_p->Flags & HWPAL_FLAG_SWAP)
            fSwap = true;
#endif
        for (i = 0; i < Count; i++)
        {
            RemappedOffset = Device_RemapDeviceAddress(DeviceByteOffset);

#ifdef HWPAL_DEVICE_DIRECT_MEMIO
            Value =
               *(uint32_t*)(uintptr_t)(HWPALLib_Device_Global.Platform.MappedBaseAddr_p +
                                                (RemappedOffset / 4));
#else
            Value = ioread32(HWPALLib_Device_Global.Platform.MappedBaseAddr_p +
                                                (RemappedOffset / 4));
#endif

            smp_rmb();

#ifdef HWPAL_DEVICE_ENABLE_SWAP
            // swap endianness if required
            if (fSwap)
                Value = Device_SwapEndian32(Value);
#endif

            MemoryDst_p[i] = Value;
            DeviceByteOffset +=  4;
        } // for
    }

#ifdef HWPAL_TRACE_DEVICE_READ
    if (Device_p->Flags & HWPAL_FLAG_READ)
    {
        HWPAL_Hexdump(
                      __func__,
                      Device_p->DevName,
                      Device_p->FirstOfs + StartByteOffset,
                      MemoryDst_p,
                      Count,
                      false);     // already swapped during read above
    }
#endif /* HWPAL_TRACE_DEVICE_READ */
    return 0;
}


/*----------------------------------------------------------------------------
 * Device_Write32Array
 */
int
Device_Write32Array(
        const Device_Handle_t Device,
        const unsigned int StartByteOffset, // write starts here, +4 increments
        const uint32_t * MemorySrc_p,       // reading starts here
        const int Count)                    // number of uint32's to transfer
{
    Device_Admin_t * Device_p;
    unsigned int DeviceByteOffset;

    if (MemorySrc_p == NULL || Count <= 0)
        return DEVICE_RW_PARAM_ERROR;

    if (!HWPALLib_Device_Global.fInitialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return DEVICE_RW_PARAM_ERROR;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    Device_p = HWPALLib_Device2RecordPtr(Device);
#else
    Device_p = Device;
#endif

#ifdef HWPAL_STRICT_ARGS_CHECK
    if ((Count <= 0) ||
        MemorySrc_p == NULL ||
        !HWPALLib_IsValid(Device_p, StartByteOffset) ||
        !HWPALLib_IsValid(Device_p, StartByteOffset + (Count - 1) * 4))
    {
        LOG_CRIT("%s: Invalid Device (%p) or write area (%u-%u)\n",
                 __func__,
                 Device,
                 StartByteOffset,
                 (unsigned int)(StartByteOffset + (Count - 1) *
                                                 sizeof(uint32_t)));
        return DEVICE_RW_PARAM_ERROR;
    }
#endif

    DeviceByteOffset = Device_p->FirstOfs + StartByteOffset;

#ifdef HWPAL_ENABLE_HA_SIMULATION
    if (Device_p->Flags & HWPAL_FLAG_HA)
    {
        // HA simulation mode
        // disable access to PKA_MASTER_SEQ_CTRL
        return 0;
    }
#endif

#ifdef HWPAL_TRACE_DEVICE_WRITE
    if (Device_p->Flags & HWPAL_FLAG_WRITE)
    {
        bool fSwap = false;

#ifdef HWPAL_DEVICE_ENABLE_SWAP
        if (Device_p->Flags & HWPAL_FLAG_SWAP)
            fSwap = true;
#endif

        HWPAL_Hexdump(
                      __func__,
                      Device_p->DevName,
                      DeviceByteOffset,
                      MemorySrc_p,
                      Count,
                      fSwap);
    }
#endif /* HWPAL_TRACE_DEVICE_WRITE */

    {
        unsigned int RemappedOffset;
        uint32_t Value;
        int i;

#ifdef HWPAL_DEVICE_ENABLE_SWAP
        bool fSwap = false;

        if (Device_p->Flags & HWPAL_FLAG_SWAP)
            fSwap = true;
#endif

        for (i = 0; i < Count; i++)
        {
            RemappedOffset = Device_RemapDeviceAddress(DeviceByteOffset);
            Value = MemorySrc_p[i];

#ifdef HWPAL_DEVICE_ENABLE_SWAP
            if (fSwap)
                Value = Device_SwapEndian32(Value);
#endif

#ifdef HWPAL_DEVICE_DIRECT_MEMIO
            *(uint32_t*)(uintptr_t)(HWPALLib_Device_Global.Platform.MappedBaseAddr_p +
                                            (RemappedOffset / 4)) = Value;
#else
            iowrite32(Value, HWPALLib_Device_Global.Platform.MappedBaseAddr_p +
                                                (RemappedOffset / 4));
#endif

            smp_wmb();

            DeviceByteOffset += 4;
        } // for
    }

    return 0;
}


/* end of file device_lkm.c */
