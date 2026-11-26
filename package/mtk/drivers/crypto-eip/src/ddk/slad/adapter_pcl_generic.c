/* adapter_pcl_generic.c
 *
 * Packet Classification (PCL) Generic API implementation.
 *
 * Notes:
 * - this implementation does not use SHDEVXS
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
******************************************************************************/

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// PCL API
#include "api_pcl.h"            // PCL_DTL_*

// Adapter PCL Internal API
#include "adapter_pcl.h"        // AdapterPCL_*


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default Adapter configuration
#include "c_adapter_pcl.h"

// DMABuf API
#include "api_dmabuf.h"         // DMABuf_*

// Adapter DMABuf internal API
#include "adapter_dmabuf.h"

// Convert address to pair of 32-bit words.
#include "adapter_addrpair.h"

// Buffer allocation (non-DMA) API
#include "adapter_alloc.h"

// sleeping API/
#include "adapter_sleep.h"

// Adapter Locking internal API
#include "adapter_lock.h"       // Adapter_Lock_*

// Runtime Power Management Device Macros API
#include "rpm_device_macros.h"  // RPM_*

// Logging API
#include "log.h"

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t
#include "device_mgmt.h"        // Device_find

// Driver Framework DMAResource API
#include "dmares_types.h"       // DMAResource_Handle_t
#include "dmares_mgmt.h"        // DMAResource management functions
#include "dmares_rw.h"          // DMAResource buffer access.
#include "dmares_addr.h"        // DMAResource addr translation functions.
#include "dmares_buf.h"         // DMAResource buffer allocations

// List API
#include "list.h"               // List_*

// Driver Framework C Run-Time Library API
#include "clib.h"               // memcpy, memset

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // bool, uint32_t

// EIP-207 Driver Library
#include "eip207_flow_generic.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#if ADAPTER_PCL_MAX_DEVICE_DIGITS > 2
#error "ADAPTER_PCL_MAX_DEVICE_DIGITS > 2 unsupported"
#endif

// Maximum number of 32-bit words in a 4Gb bank - representable as uint32_t
#define ADAPTER_PCL_MAX_BANK_WORDS ((uint32_t)((1ULL<<32) / sizeof(uint32_t)))


/*----------------------------------------------------------------------------
 * Local variables
 */

static AdapterPCL_Device_Instance_Data_t Dev_Instance [ADAPTER_PCL_MAX_FLUE_DEVICES];

// DMA buffer allocation alignment, in bytes
static int AdapterPCL_DMA_Alignment_ByteCount =
                                ADAPTER_DMABUF_ALIGNMENT_INVALID;

// Global lock and critical section for PCL API
static ADAPTER_LOCK_DEFINE(AdapterPCL_Lock);
static Adapter_Lock_CS_t AdapterPCL_CS;

// Cached values for RPM device resume operation
static EIP207_Flow_Address_t EIP207_FlowBaseAddr;
static EIP207_Flow_Address_t EIP207_TransformBaseAddr;
static EIP207_Flow_Address_t EIP207_FlowAddr;
static EIP207_Flow_HT_t EIP207_HT_Params;


/*----------------------------------------------------------------------------
 * Function definitions
 */


/*----------------------------------------------------------------------------
  AdapterPCLLib_HashTable_Entries_Num_To_Size

  Convert numerical value of number of hashtable entries, to corresponding enum

  number (input)
    value indicating number of hashtable entries (eg. 32)

  Return:
      EIP207_Flow_HashTable_Entry_Count_t value representing 'number'
      or
      EIP207_FLOW_HASH_TABLE_ENTRIES_MAX+1 indicating error

   Note: implemented using macros to ensure numeric values in step with enums
 */
static EIP207_Flow_HashTable_Entry_Count_t
AdapterPCLLib_HashTable_Entries_Num_To_Size(
        int number)
{
#define HASHTABLE_SIZE_CASE(num) \
    case (num): return EIP207_FLOW_HASH_TABLE_ENTRIES_##num

    switch (number)
    {
        HASHTABLE_SIZE_CASE(32);
        HASHTABLE_SIZE_CASE(64);
        HASHTABLE_SIZE_CASE(128);
        HASHTABLE_SIZE_CASE(256);
        HASHTABLE_SIZE_CASE(512);
        HASHTABLE_SIZE_CASE(1024);
        HASHTABLE_SIZE_CASE(2048);
        HASHTABLE_SIZE_CASE(4096);
        HASHTABLE_SIZE_CASE(8192);
        HASHTABLE_SIZE_CASE(16384);
        HASHTABLE_SIZE_CASE(32768);
        HASHTABLE_SIZE_CASE(65536);
        HASHTABLE_SIZE_CASE(131072);
        HASHTABLE_SIZE_CASE(262144);
        HASHTABLE_SIZE_CASE(524288);
        HASHTABLE_SIZE_CASE(1048576);

        default:
            return EIP207_FLOW_HASH_TABLE_ENTRIES_MAX + 1;  //invalid value
    }
#undef HASHTABLE_SIZE_CASE
}


/*----------------------------------------------------------------------------
 * AdapterPCLLib_Device_Find
 *
 * Returns device handle corresponding to device interface id (index)
 * Appends characters corresponding to id number to the device base name.
 * The base name ADAPTER_PCL_FLUE_DEFAULT_DEVICE_NAME may optionally end in zero '0'.
 * @note Will handle interface index 0-99
 */
static Device_Handle_t
AdapterPCLLib_Device_Find(unsigned int InterfaceId)
{
    char device_name_digit;
    // base name
    char device_name[]= ADAPTER_PCL_FLUE_DEFAULT_DEVICE_NAME "\0\0\0\0";

    // string's last char
    int device_name_digit_index = strlen(device_name) - 1;

    // parameter validation
    if (InterfaceId > 1 ||  // implementation max
            InterfaceId >= ADAPTER_PCL_MAX_FLUE_DEVICES ||    // config max
            InterfaceId > 99   // algorithm max
            )
    {
        return NULL;
    }

    // use final digit (0) if it's present, otherwise skip to end of base name
    device_name_digit = device_name[device_name_digit_index];
    if (device_name_digit != '0')
    {
        ++device_name_digit_index;
    }

    if (InterfaceId >= 10)   // two digit identifier, write tens digit
    {
        device_name[device_name_digit_index] = (InterfaceId / 10) + '0';
        ++device_name_digit_index;
        InterfaceId %= 10;
    }

    // write units digit
    device_name[device_name_digit_index] = InterfaceId + '0';

    // look up actual device
    return Device_Find(device_name);
}


/*-----------------------------------------------------------------------------
 * AdapterPCL_Int_To_Ptr
 */
static inline void *
AdapterPCL_Int_To_Ptr(
        const unsigned int Value)
{
    union Number
    {
        void * p;
        uintptr_t Value;
    } N;

    N.Value = (uintptr_t)Value;

    return N.p;
}


#ifdef ADAPTER_PCL_RPM_EIP207_DEVICE_ID
/*-----------------------------------------------------------------------------
 * AdapterPCL_Resume
 */
static int
AdapterPCL_Resume(void * p)
{
    EIP207_Flow_Error_t res;
    AdapterPCL_Device_Instance_Data_t * Dev_p;
    int InterfaceId = *(int *)p;

    // only one hash table in this implementation
    const unsigned int hashtable_id = 0;

    LOG_INFO("\n\t %s \n", __func__);

    if (InterfaceId < 0 || InterfaceId < ADAPTER_PCL_RPM_EIP207_DEVICE_ID)
        return -1; // error

    InterfaceId -= ADAPTER_PCL_RPM_EIP207_DEVICE_ID;

    // select current device global instance data, on validated InterfaceId
    Dev_p = &Dev_Instance[InterfaceId];

#ifdef ADAPTER_PCL_DMARESOURCE_BANKS_ENABLE
    LOG_INFO("\n\t\t EIP207_Flow_RC_BaseAddr_Set \n");

    // Restore base addresses for flow and transform records
    res = EIP207_Flow_RC_BaseAddr_Set(Dev_p->EIP207_IOArea_p,
                                      hashtable_id,
                                      &EIP207_FlowBaseAddr,
                                      &EIP207_TransformBaseAddr);
    if (res != EIP207_FLOW_NO_ERROR)
    {
        LOG_CRIT("%s: set records base address failed, error %d\n",
                 __func__,
                 res);
        return -3;
    }
#endif // ADAPTER_PCL_DMARESOURCE_BANKS_ENABLE

    LOG_INFO("\n\t\t EIP207_Flow_HashTable_Install \n");

    // Restore FLUE hashtable
    res =  EIP207_Flow_HashTable_Install(Dev_p->EIP207_IOArea_p,
                                         hashtable_id,
                                         &EIP207_HT_Params,
                                         ADAPTER_PCL_ENABLE_FLUE_CACHE,
                                         false);
    if (res != EIP207_FLOW_NO_ERROR)
    {
        LOG_CRIT("%s: EIP207_Flow_HashTable_Install() failed, error %d\n",
                 __func__,
                 res);
        return -4;
    }

    return 0;
}
#endif


/*-----------------------------------------------------------------------------
 * Adapter PCL Internal API functions implementation
 */

/*----------------------------------------------------------------------------
 * AdapterPCL_DMABuf_To_TRDscr
 */
PCL_Status_t
AdapterPCL_DMABuf_To_TRDscr(
        const DMABuf_Handle_t TransformHandle,
        EIP207_Flow_TR_Dscr_t * const TR_Dscr_p,
        DMAResource_Record_t ** Rec_pp)
{
    DMAResource_Handle_t DMAHandle =
        Adapter_DMABuf_Handle2DMAResourceHandle(TransformHandle);

    DMAResource_AddrPair_t PhysAddr;
    DMAResource_Record_t * Rec_p;

    Rec_p = DMAResource_Handle2RecordPtr(DMAHandle);

    if (Rec_p == NULL)
        return PCL_ERROR;

#ifdef ADAPTER_PCL_DMARESOURCE_BANKS_ENABLE
    if (Rec_p->Props.Bank != ADAPTER_PCL_BANK_TRANSFORM)
    {
        LOG_CRIT("PCL Adapter: Invalid bank for Transform\n");
        return PCL_ERROR;
    }
#endif

    if (DMAResource_Translate(DMAHandle, DMARES_DOMAIN_BUS, &PhysAddr) < 0)
    {
        LOG_CRIT("PCL_Transform: Failed to obtain physical address.\n");
        return PCL_ERROR;
    }

    Adapter_AddrToWordPair(PhysAddr.Address_p, 0,
                           &TR_Dscr_p->DMA_Addr.Addr,
                           &TR_Dscr_p->DMA_Addr.UpperAddr);

    TR_Dscr_p->DMA_Handle = DMAHandle;

    *Rec_pp = Rec_p;

    return PCL_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * AdapterPCL_Device_Get
 */
AdapterPCL_Device_Instance_Data_t *
AdapterPCL_Device_Get(
        const unsigned int InterfaceId)
{
    return &Dev_Instance[InterfaceId];
}


/*-----------------------------------------------------------------------------
 * AdapterPCL_ListID_Get
 */
PCL_Status_t
AdapterPCL_ListID_Get(
        const unsigned int InterfaceId,
        unsigned int * const ListID_p)
{
    // validate parameter
    if (ListID_p == NULL)
        return PCL_INVALID_PARAMETER;

    *ListID_p = Dev_Instance[InterfaceId].FreeListID;
    return PCL_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * Adapter PCL API functions implementation
 */

/*----------------------------------------------------------------------------
 * PCL_Init
 */
PCL_Status_t
PCL_Init(
        const unsigned int InterfaceId,
        const unsigned int NofFlowHashTables)
{
    EIP207_Flow_Error_t res;
    Device_Handle_t EIP207_Device;
    AdapterPCL_Device_Instance_Data_t * Dev_p;

    unsigned int ioarea_size_bytes;
    unsigned int descr_total_size_bytes;
    unsigned int hashtable_entry_size_words, hashtable_total_size_words;

    List_Element_t * ElementPool_p = NULL;
    unsigned char * RecDscrPool_p = NULL;
    EIP207_Flow_IOArea_t * ioarea_p = NULL;
    void * descr_area_ptr = NULL;

    // includes overflow records
    const unsigned int total_hasharea_element_count =
                            (ADAPTER_PCL_FLOW_HASH_OVERFLOW_COUNT +
                                    ADAPTER_PCL_FLOW_HASH_ENTRIES_COUNT);

    DMAResource_Handle_t hashtable_dma_handle = NULL;

    // only one hash table in this implementation
    const unsigned int hashtable_id = 0;

    LOG_INFO("\n\t PCL_Init \n");

    // check number of hash tables against implementation & config limits
    if (NofFlowHashTables == 0 ||   // validity
        NofFlowHashTables > 1 || // implementation limit
        NofFlowHashTables > ADAPTER_CS_MAX_NOF_FLOW_HASH_TABLES_TO_USE)
    {
        LOG_CRIT("PCL_Init: Invalid number (%d) of hash tables\n",
                 NofFlowHashTables);
        return PCL_INVALID_PARAMETER;
    }

    Adapter_Lock_CS_Set(&AdapterPCL_CS, &AdapterPCL_Lock);

    if (!Adapter_Lock_CS_Enter(&AdapterPCL_CS))
        return PCL_STATUS_BUSY;

    // select current device global instance data, on validated InterfaceId
    Dev_p = &Dev_Instance[InterfaceId];

    // state consistency check
    if (Dev_p->PCL_IsInitialized)
    {
        LOG_CRIT("PCL_Init: Already initialized\n");
        Adapter_Lock_CS_Leave(&AdapterPCL_CS);
        return PCL_ERROR;
    }

    // Initialize instance variables to 0/NULL defaults
    ZEROINIT(*Dev_p);

    // Allocate device lock
    Dev_p->AdapterPCL_DevLock = Adapter_Lock_Alloc();
    if (Dev_p->AdapterPCL_DevLock == Adapter_Lock_NULL)
    {
        LOG_CRIT("PCL_Init: PutLock allocation failed\n");
        Adapter_Lock_CS_Leave(&AdapterPCL_CS);
        return PCL_OUT_OF_MEMORY_ERROR;
    }
    Adapter_Lock_CS_Set(&Dev_p->AdapterPCL_DevCS,
                         Dev_p->AdapterPCL_DevLock);

    // identify device, check InterfaceId
    EIP207_Device = AdapterPCLLib_Device_Find(InterfaceId);
    if (EIP207_Device == NULL)
    {
        LOG_CRIT("PCL_Init: Cannot find EIP-207 device, id=%d\n", InterfaceId);
        goto error_exit;
    }

    // Get DMA buffer allocation alignment
    AdapterPCL_DMA_Alignment_ByteCount = Adapter_DMAResource_Alignment_Get();
    if (AdapterPCL_DMA_Alignment_ByteCount == ADAPTER_DMABUF_ALIGNMENT_INVALID)
    {
#if ADAPTER_PCL_DMA_ALIGNMENT_BYTE_COUNT == 0
        LOG_CRIT("PCL_Init: Failed to get DMA alignment\n");
        goto error_exit;
#else
        AdapterPCL_DMA_Alignment_ByteCount =
                                ADAPTER_PCL_DMA_ALIGNMENT_BYTE_COUNT;
#endif
    }

    // allocate IOArea non-DMA memory
    ioarea_size_bytes = EIP207_Flow_IOArea_ByteCount_Get();
    ioarea_p = Adapter_Alloc(ioarea_size_bytes);

    if (ioarea_p == NULL)
    {
        LOG_CRIT("PCL_Init: Cannot allocate IOArea\n");
        goto error_exit;
    }

    // store value for global use
    Dev_p->EIP207_IOArea_p = ioarea_p;

    if (RPM_DEVICE_INIT_START_MACRO(ADAPTER_PCL_RPM_EIP207_DEVICE_ID + InterfaceId,
                                    0, // No suspend callback is used
                                    AdapterPCL_Resume) != RPM_SUCCESS)
        goto error_exit;

    LOG_INFO("\n\t\t EIP207_Flow_Init \n");

    // hand IOArea memory to driver
    res = EIP207_Flow_Init(Dev_p->EIP207_IOArea_p,
                           EIP207_Device);
    if (res != EIP207_FLOW_NO_ERROR)
    {
        LOG_CRIT("PCL_Init: EIP207_Flow_Init() failed\n");
        goto fail;  // exit which frees allocated resources
    }

#ifdef ADAPTER_PCL_DMARESOURCE_BANKS_ENABLE
    {
        unsigned int RecordWordCount;

        RecordWordCount = EIP207_Flow_TR_WordCount_Get();

        if (RecordWordCount * sizeof(uint32_t) >
                ADAPTER_TRANSFORM_RECORD_BYTE_COUNT)
        {
            LOG_CRIT("PCL_Init: Bad Record size in transform bank, "
                     "is %d, at least %d required\n",
                     (int)(ADAPTER_TRANSFORM_RECORD_BYTE_COUNT *
                               sizeof(uint32_t)),
                     RecordWordCount);
            goto fail;  // exit which frees allocated resources
        }

        RecordWordCount = EIP207_Flow_FR_WordCount_Get();

        if (RecordWordCount * sizeof(uint32_t) >
                           ADAPTER_PCL_FLOW_RECORD_BYTE_COUNT)
        {
            LOG_CRIT("PCL_Init: Bad Record size in flow bank, "
                     "is %d, at least %d required\n",
                     (int)(ADAPTER_PCL_FLOW_RECORD_BYTE_COUNT *
                               sizeof(uint32_t)),
                     RecordWordCount);
            goto fail;  // exit which frees allocated resources
        }
    }

    { // Set the SA pool base address.
        int dmares;
        DMAResource_Handle_t DMAHandle;
        DMAResource_Properties_t DMAProperties;
        DMAResource_AddrPair_t DMAAddr;

        ZEROINIT(EIP207_FlowBaseAddr);
        ZEROINIT(EIP207_TransformBaseAddr);

        // Perform a full-bank allocation in transform bank to obtain the bank base
        // address.
        {
            DMAProperties.Alignment = AdapterPCL_DMA_Alignment_ByteCount;
            DMAProperties.Bank      = ADAPTER_PCL_BANK_TRANSFORM;
            DMAProperties.fCached   = false;
            DMAProperties.Size      = ADAPTER_TRANSFORM_RECORD_COUNT *
                                          ADAPTER_TRANSFORM_RECORD_BYTE_COUNT;
            dmares = DMAResource_Alloc(DMAProperties,
                                       &DMAAddr,
                                       &DMAHandle);
            if (dmares != 0)
            {
                LOG_CRIT(
                        "PCL_Init: allocate transforms base address failed\n");
                goto fail;  // exit which frees allocated resources
            }

            // Derive the physical address from the DMA resource.
            if (DMAResource_Translate(DMAHandle,
                                      DMARES_DOMAIN_BUS,
                                      &DMAAddr)             < 0)
            {
                DMAResource_Release(DMAHandle);
                LOG_CRIT(
                      "PCL_Init: translate transforms base address failed\n");
                goto fail;  // exit which frees allocated resources
            }

            Adapter_AddrToWordPair(DMAAddr.Address_p,
                                   0,
                                   &EIP207_TransformBaseAddr.Addr,
                                   &EIP207_TransformBaseAddr.UpperAddr);

            // Release the DMA resource
            DMAResource_Release(DMAHandle);
        }

        // Perform a size 0 allocation in flow bank to obtain the bank base
        // address.
        DMAProperties.Alignment = AdapterPCL_DMA_Alignment_ByteCount;
        DMAProperties.Bank      = ADAPTER_PCL_BANK_FLOW;
        DMAProperties.fCached   = false;
        DMAProperties.Size      = ADAPTER_PCL_FLOW_RECORD_COUNT *
                                          ADAPTER_PCL_FLOW_RECORD_BYTE_COUNT;
        dmares = DMAResource_Alloc(DMAProperties,
                                   &DMAAddr,
                                   &DMAHandle);
        if (dmares != 0)
        {
            LOG_CRIT("PCL_Init: allocate flow base address failed\n");
            goto fail;  // exit which frees allocated resources
        }

        // Derive the physical address from the DMA resource.
        if (DMAResource_Translate(DMAHandle, DMARES_DOMAIN_BUS, &DMAAddr) < 0)
        {
            DMAResource_Release(DMAHandle);
            LOG_CRIT("PCL_Init: translate flow base address failed\n");
            goto fail;  // exit which frees allocated resources
        }

        Adapter_AddrToWordPair(DMAAddr.Address_p, 0,
                               &EIP207_FlowBaseAddr.Addr,
                               &EIP207_FlowBaseAddr.UpperAddr);

        // Release the DMA resource - handle to zero-size request.
        DMAResource_Release(DMAHandle);

        LOG_INFO("\n\t\t EIP207_Flow_RC_BaseAddr_Set \n");

        // set base addresses for flow and transform records
        res = EIP207_Flow_RC_BaseAddr_Set(
                ioarea_p,
                hashtable_id,
                &EIP207_FlowBaseAddr,
                &EIP207_TransformBaseAddr);
        if (res != EIP207_FLOW_NO_ERROR)
        {
            LOG_CRIT("PCL_Init: set records base address failed\n");
            goto fail;  // exit which frees allocated resources
        }

    }
#endif // ADAPTER_PCL_DMARESOURCE_BANKS_ENABLE

    // Install Hashtable:
    // - calculate amount of required descriptor memory & allocate it
    // - calculate amount of required DMA-safe hashtable+overflow memory &
    //   allocate it
    // - fill in struct for HT install, call install function

    // descriptor memory
    descr_total_size_bytes = EIP207_Flow_HTE_Dscr_ByteCount_Get() *
                                               total_hasharea_element_count;

    descr_area_ptr = Adapter_Alloc(descr_total_size_bytes);
    if (descr_area_ptr == NULL)
    {
        LOG_CRIT("PCL_Init: Cannot allocate descriptor area\n");
        goto fail;  // exit which frees allocated resources
    }

    // store value for global use
    Dev_p->EIP207_Descriptor_Area_p = descr_area_ptr;

    // get required memory size for hash table
    // = (hash table size + overflow entries)*bucket size
    hashtable_entry_size_words = EIP207_Flow_HT_Entry_WordCount_Get();
    hashtable_total_size_words =
            hashtable_entry_size_words * total_hasharea_element_count;

    // check total size of hashtable region (lookup + overflow)
    // would not exceed a 32-bit addressable bank of 4GB
    if (hashtable_total_size_words >= ADAPTER_PCL_MAX_BANK_WORDS)
    {
        LOG_CRIT("PCL_Init: Too many hashtable lookup elements for bank\n");
        goto fail;  // exit which frees allocated resources
    }

    // get DMA-safe memory for hashtable in appropriate bank
    {
        DMAResource_Properties_t TableProperties;
        DMAResource_AddrPair_t TableHostAddr;
        DMAResource_AddrPair_t PhysAddr;
        int dmares;

        // required DMA buffer properties
        TableProperties.Alignment = AdapterPCL_DMA_Alignment_ByteCount;
        // Hash table DMA bank
        TableProperties.Bank      = ADAPTER_PCL_BANK_FLOWTABLE;
        TableProperties.fCached   = false;
        // Check if this does not exceed 4 GB, do it somewhere above
        // size in bytes
        TableProperties.Size      = hashtable_total_size_words *
                                                        sizeof(uint32_t);

        // Perform a full-bank allocation in flow table bank to obtain
        // the bank base address.
        dmares = DMAResource_Alloc(TableProperties,
                                   &TableHostAddr,
                                   &hashtable_dma_handle);

#ifdef ADAPTER_PCL_ENABLE_SWAP
        DMAResource_SwapEndianness_Set(hashtable_dma_handle, true);
#endif
        if (dmares != 0)
        {
            LOG_CRIT("PCL_Init: Failed to allocate flow hash table\n");
            goto fail;  // exit which frees allocated resources
        }

        // get physical address from handle
        if (DMAResource_Translate(hashtable_dma_handle,
                                  DMARES_DOMAIN_BUS,
                                  &PhysAddr) < 0)
        {
            LOG_CRIT("PCL_Init: Failed to obtain physical address.\n");
            goto fail;  // exit which frees allocated resources
        }

        ZEROINIT(EIP207_FlowAddr);

        // physical address as upper and lower (EIP207_Flow_Address_t)
        Adapter_AddrToWordPair(PhysAddr.Address_p, 0,
                               &EIP207_FlowAddr.Addr,
                               &EIP207_FlowAddr.UpperAddr);

        // fill in FLUE hashtable descriptor fields
        ZEROINIT(EIP207_HT_Params);

        // handle
        EIP207_HT_Params.HT_DMA_Handle    = hashtable_dma_handle;

        // translated addr
        EIP207_HT_Params.HT_DMA_Address_p = &EIP207_FlowAddr;

        // Convert numerical value to enum
        EIP207_HT_Params.HT_TableSize     =
                            AdapterPCLLib_HashTable_Entries_Num_To_Size(
                                         ADAPTER_PCL_FLOW_HASH_ENTRIES_COUNT);
        EIP207_HT_Params.DT_p             = descr_area_ptr;

        // hash table plus overflow
        EIP207_HT_Params.DT_EntryCount    = total_hasharea_element_count;
    }

    LOG_INFO("\n\t\t EIP207_Flow_HashTable_Install \n");

    // install FLUE hashtable
    res =  EIP207_Flow_HashTable_Install(ioarea_p,
                                         hashtable_id,
                                         &EIP207_HT_Params,
                                         ADAPTER_PCL_ENABLE_FLUE_CACHE,
                                         true);
    if (res != EIP207_FLOW_NO_ERROR)
    {
        LOG_CRIT("PCL_Init: EIP207_Flow_HashTable_Install failed\n");
        goto fail;  // exit which frees allocated resources
    }

    // Initialize the free list of record descriptors
    {
        unsigned int i;
        List_Element_t * Element_p;
        unsigned char * RecDscr_p;
        unsigned int RecordByteCount = MAX(EIP207_Flow_FR_Dscr_ByteCount_Get(),
            EIP207_Flow_TR_Dscr_ByteCount_Get());

        // Set the free list ID
        Dev_p->FreeListID = ADAPTER_PCL_LIST_ID_OFFSET + InterfaceId;

        // Allocate a pool of list elements
        ElementPool_p = Adapter_Alloc(sizeof(List_Element_t) *
                                        total_hasharea_element_count);
        if (ElementPool_p == NULL)
        {
            LOG_CRIT("PCL_Init: free list allocation failed\n");
            goto fail;
        }

        if (List_Init(Dev_p->FreeListID, NULL) != LIST_STATUS_OK)
        {
            LOG_CRIT("PCL_Init: free list initialization failed\n");
            goto fail;
        }

        // Allocate a pool of record descriptors
        RecDscrPool_p = Adapter_Alloc( RecordByteCount *
                                            total_hasharea_element_count);
        if (RecDscrPool_p == NULL)
        {
            LOG_CRIT("PCL_Init: record descriptor allocation failed\n");
            goto fail;
        }

        // Populate the free list with the elements (record descriptors)
        Element_p = ElementPool_p;
        Element_p->DataObject_p = RecDscr_p = RecDscrPool_p;
        for(i = 0; i < total_hasharea_element_count; i++)
        {
            if (List_AddToHead(Dev_p->FreeListID,
                               NULL,
                               Element_p) == LIST_STATUS_OK)
            {
                if (i < total_hasharea_element_count - 1)
                {
                    Element_p++;
                    RecDscr_p += RecordByteCount;
                    Element_p->DataObject_p = RecDscr_p;
                }
            }
            else
            {
                LOG_CRIT("PCL_Init: free list population failed\n");
                goto fail;
            }
        }

        Dev_p->RecDscrPool_p = RecDscrPool_p;
        Dev_p->ElementPool_p = ElementPool_p;
    }  // Record descriptors free list initialized

    // set remaining instance variables, on success
    Dev_p->EIP207_Hashtable_DMA_Handle = hashtable_dma_handle;
    Dev_p->PCL_IsInitialized = true;

    (void)RPM_DEVICE_INIT_STOP_MACRO(ADAPTER_PCL_RPM_EIP207_DEVICE_ID + InterfaceId);

    Adapter_Lock_CS_Leave(&AdapterPCL_CS);

    return PCL_STATUS_OK;

fail:
    (void)RPM_DEVICE_INIT_STOP_MACRO(ADAPTER_PCL_RPM_EIP207_DEVICE_ID + InterfaceId);

    // free List data structures
    Adapter_Free(ElementPool_p);
    Adapter_Free(RecDscrPool_p);
    List_Uninit(Dev_p->FreeListID, NULL);

    // free memory areas and DMA-safe memory
    Adapter_Free(Dev_p->EIP207_IOArea_p);
    Adapter_Free(Dev_p->EIP207_Descriptor_Area_p);
    if (hashtable_dma_handle != NULL)
        DMAResource_Release(hashtable_dma_handle);

error_exit:
    Adapter_Lock_CS_Leave(&AdapterPCL_CS);

    return PCL_ERROR;
}


/*----------------------------------------------------------------------------
 * PCL_UnInit
 */
PCL_Status_t
PCL_UnInit(
        const unsigned int InterfaceId)
{
    AdapterPCL_Device_Instance_Data_t * Dev_p;

    LOG_INFO("\n\t PCL_UnInit \n");

    if (InterfaceId >= ADAPTER_PCL_MAX_FLUE_DEVICES)
    {
        return PCL_INVALID_PARAMETER;
    }

    Adapter_Lock_CS_Set(&AdapterPCL_CS, &AdapterPCL_Lock);

    if (!Adapter_Lock_CS_Enter(&AdapterPCL_CS))
        return PCL_STATUS_BUSY;

    // select current device instance data, on validated InterfaceId
    Dev_p = &Dev_Instance[InterfaceId];

    if (!Dev_p->PCL_IsInitialized)
    {
        LOG_CRIT("PCL_UnInit: Not initialized\n");
        Adapter_Lock_CS_Leave(&AdapterPCL_CS);
        return PCL_ERROR;
    }

    if (!Adapter_Lock_CS_Enter(&Dev_p->AdapterPCL_DevCS))
    {
        Adapter_Lock_CS_Leave(&Dev_p->AdapterPCL_DevCS);
        Adapter_Lock_CS_Leave(&AdapterPCL_CS);
        return PCL_STATUS_BUSY;
    }

    if (RPM_DEVICE_UNINIT_START_MACRO(ADAPTER_PCL_RPM_EIP207_DEVICE_ID + InterfaceId,
                                      false) != RPM_SUCCESS)
    {
        Adapter_Lock_CS_Leave(&Dev_p->AdapterPCL_DevCS);
        Adapter_Lock_CS_Leave(&AdapterPCL_CS);
        return PCL_ERROR;
    }

    // free List data structures
    Adapter_Free(Dev_p->ElementPool_p);
    Adapter_Free(Dev_p->RecDscrPool_p);
    List_Uninit(Dev_p->FreeListID, NULL);

    // pool list data structures
    Adapter_Free(Dev_p->ListElementPool_p);
    Adapter_Free(Dev_p->ListPool_p);
    if (Dev_p->List_p)
    {
        List_Uninit(LIST_DUMMY_LIST_ID, Dev_p->List_p);
        Dev_p->List_p = NULL;
    }

    // free memory areas and DMA-safe memory
    Adapter_Free(Dev_p->EIP207_IOArea_p);
    Adapter_Free(Dev_p->EIP207_Descriptor_Area_p);
    DMAResource_Release(Dev_p->EIP207_Hashtable_DMA_Handle);

    // reset globals
    Dev_p->EIP207_IOArea_p              = NULL;
    Dev_p->EIP207_Hashtable_DMA_Handle  = NULL;
    Dev_p->EIP207_Descriptor_Area_p     = NULL;
    Dev_p->RecDscrPool_p                = NULL;
    Dev_p->ElementPool_p                = NULL;

    Dev_p->PCL_IsInitialized = false;

    AdapterPCL_DMA_Alignment_ByteCount = ADAPTER_DMABUF_ALIGNMENT_INVALID;

    Adapter_Lock_CS_Leave(&Dev_p->AdapterPCL_DevCS);

    // Free device lock
    Adapter_Lock_Free(Adapter_Lock_CS_Get(&Dev_p->AdapterPCL_DevCS));
    Adapter_Lock_CS_Set(&Dev_p->AdapterPCL_DevCS, Adapter_Lock_NULL);

    (void)RPM_DEVICE_UNINIT_STOP_MACRO(ADAPTER_PCL_RPM_EIP207_DEVICE_ID + InterfaceId);

    Adapter_Lock_CS_Leave(&AdapterPCL_CS);

    return PCL_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * PCL_Flow_DMABuf_Handle_Get
 */
PCL_Status_t
PCL_Flow_DMABuf_Handle_Get(
        const PCL_FlowHandle_t FlowHandle,
        DMABuf_Handle_t * const DMAHandle_p)
{
    DMAResource_Handle_t DMARes_Handle;
    EIP207_Flow_FR_Dscr_t * FlowDescriptor_p;
    List_Element_t * Element_p = (List_Element_t*)FlowHandle;

    LOG_INFO("\n\t PCL_Flow_DMABuf_Handle_Get \n");

    if (DMAHandle_p == NULL ||
        Element_p == NULL)
    {
        LOG_CRIT("PCL_Flow_DMABuf_Handle_Get: failed, invalid DMABuf handle or Flow Handle\n");
        return PCL_INVALID_PARAMETER;
    }

    FlowDescriptor_p = (EIP207_Flow_FR_Dscr_t *)Element_p->DataObject_p;

    if (FlowDescriptor_p == NULL)
    {
        LOG_CRIT("PCL_Flow_DMABuf_Handle_Get: failed, invalid flow handle\n");
        return PCL_INVALID_PARAMETER;
    }

    DMARes_Handle = FlowDescriptor_p->DMA_Handle;

    *DMAHandle_p = Adapter_DMAResource_Handle2DMABufHandle(DMARes_Handle);

    return PCL_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * PCL_Flow_Hash
 */
PCL_Status_t
PCL_Flow_Hash(
        const PCL_SelectorParams_t * const SelectorParams,
        uint32_t * FlowID_Word32ArrayOf4)
{
    EIP207_Flow_SelectorParams_t EIP207_SelectorParams;
    EIP207_Flow_ID_t FlowID;
    EIP207_Flow_Error_t res;
    const unsigned int hashtable_id = 0;    // implementation limit

    LOG_INFO("\n\t PCL_Flow_Hash \n");

    if (SelectorParams == NULL || FlowID_Word32ArrayOf4 == NULL)
        return PCL_ERROR;

    ZEROINIT(EIP207_SelectorParams);
    ZEROINIT(FlowID);

    EIP207_SelectorParams.Flags   = SelectorParams->flags;
    EIP207_SelectorParams.SrcIp_p = SelectorParams->SrcIp;
    EIP207_SelectorParams.DstIp_p = SelectorParams->DstIp;
    EIP207_SelectorParams.IpProto = SelectorParams->IpProto;
    EIP207_SelectorParams.SrcPort = SelectorParams->SrcPort;
    EIP207_SelectorParams.DstPort = SelectorParams->DstPort;
    EIP207_SelectorParams.SPI     = SelectorParams->spi;
    EIP207_SelectorParams.Epoch  = SelectorParams->epoch;

    if (RPM_DEVICE_IO_START_MACRO(ADAPTER_PCL_RPM_EIP207_DEVICE_ID,
                                  RPM_FLAG_SYNC) != RPM_SUCCESS)
        return PCL_ERROR;

    LOG_INFO("\n\t\t EIP207_Flow_ID_Compute \n");

    res = EIP207_Flow_ID_Compute(
            Dev_Instance[hashtable_id].EIP207_IOArea_p,
            hashtable_id,
            &EIP207_SelectorParams,
            &FlowID);

    // Note: only one EIP-207 hash table is supported!
    (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_PCL_RPM_EIP207_DEVICE_ID,
                                   RPM_FLAG_ASYNC);

    if (res != EIP207_FLOW_NO_ERROR)
    {
        LOG_CRIT("PCL_Flow_Hash: operation failed\n");
        return PCL_ERROR;
    }

    FlowID_Word32ArrayOf4[0] = FlowID.Word32[0];
    FlowID_Word32ArrayOf4[1] = FlowID.Word32[1];
    FlowID_Word32ArrayOf4[2] = FlowID.Word32[2];
    FlowID_Word32ArrayOf4[3] = FlowID.Word32[3];

    return PCL_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * PCL_Flow_Alloc
 */
PCL_Status_t
PCL_Flow_Alloc(
        const unsigned int InterfaceId,
        const unsigned int FlowHashTableId,
        PCL_FlowHandle_t * const FlowHandle_p)
{
    PCL_Status_t PCL_Rc = PCL_ERROR;
    DMAResource_Handle_t DMAHandle;
    DMAResource_Properties_t DMAProperties;
    DMAResource_AddrPair_t HostAddr;
    DMAResource_AddrPair_t PhysAddr;
    int dmares;
    EIP207_Flow_FR_Dscr_t * FlowDescriptor_p;
    unsigned int FR_WordCount;
    List_Element_t * Element_p;
    AdapterPCL_Device_Instance_Data_t * Dev_p;

    LOG_INFO("\n\t PCL_Flow_Alloc \n");

    IDENTIFIER_NOT_USED(FlowHashTableId);

    // validate interface id
    if (InterfaceId >= ADAPTER_PCL_MAX_FLUE_DEVICES)
        return PCL_INVALID_PARAMETER;

    Dev_p = &Dev_Instance[InterfaceId];

    if (Adapter_Lock_CS_Get(&Dev_p->AdapterPCL_DevCS) == Adapter_Lock_NULL)
    {
        LOG_CRIT("PCL_Flow_Alloc: no device lock, not initialized?\n");
        return PCL_ERROR;
    }

    if (!Adapter_Lock_CS_Enter(&Dev_p->AdapterPCL_DevCS))
        return PCL_STATUS_BUSY;

    // select current device instance data, on validated InterfaceId
    if (!Dev_p->PCL_IsInitialized)
    {
        LOG_CRIT("PCL_Flow_Alloc: Not initialized\n");
        goto error_exit;
    }

    *FlowHandle_p = NULL;

    LOG_INFO("\n\t\t EIP207_Flow_FR_WordCount_Get \n");

    // Get the required size of the DMA buffer for the flow record.
    FR_WordCount = EIP207_Flow_FR_WordCount_Get();

    // Allocate a buffer for the flow record (DMA).
    DMAProperties.Alignment = AdapterPCL_DMA_Alignment_ByteCount;
    DMAProperties.Bank      = ADAPTER_PCL_BANK_FLOW;
    DMAProperties.fCached   = false;
    DMAProperties.Size      = 4 * FR_WordCount;

    dmares = DMAResource_Alloc(DMAProperties, &HostAddr, &DMAHandle);
    if (dmares != 0)
    {
        LOG_CRIT("PCL_Flow_Alloc: could not allocate buffer\n");
        goto error_exit;
    }

#ifdef ADAPTER_PCL_ENABLE_SWAP
    DMAResource_SwapEndianness_Set(DMAHandle, true);
#endif

    // Get a record descriptor from the free list
    {
        List_Status_t List_Rc =
                List_RemoveFromTail(Dev_p->FreeListID,
                                    NULL,
                                    &Element_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT("PCL_Flow_Alloc: failed to allocate record\n");
            goto error_exit;
        }

        // Get the flow record descriptor place-holder from the list element
        FlowDescriptor_p = (EIP207_Flow_FR_Dscr_t*)Element_p->DataObject_p;
        if (FlowDescriptor_p == NULL)
        {
            LOG_CRIT("PCL_Flow_Alloc: failed to get record descriptor\n");
            goto error_exit;
        }
    }

    // Fill in the descriptor.
    FlowDescriptor_p->DMA_Handle = DMAHandle;

    if (DMAResource_Translate(DMAHandle, DMARES_DOMAIN_BUS, &PhysAddr) < 0)
    {
        LOG_CRIT("PCL_FlowAlloc: Failed to obtain physical address.\n");
        DMAResource_Release(DMAHandle);
        goto error_exit;
    }

    Adapter_AddrToWordPair(PhysAddr.Address_p, 0,
                           &FlowDescriptor_p->DMA_Addr.Addr,
                           &FlowDescriptor_p->DMA_Addr.UpperAddr);

    *FlowHandle_p = (PCL_FlowHandle_t)Element_p;

    PCL_Rc = PCL_STATUS_OK;

error_exit:
    Adapter_Lock_CS_Leave(&Dev_p->AdapterPCL_DevCS);

    return PCL_Rc;
}


/*-----------------------------------------------------------------------------
 * PCL_Flow_Add
 */
PCL_Status_t
PCL_Flow_Add(
        const unsigned int InterfaceId,
        const unsigned int FlowHashTableId,
        const PCL_FlowParams_t * const FlowParams,
        const PCL_FlowHandle_t FlowHandle)
{
    PCL_Status_t PCL_Rc = PCL_ERROR;
    DMAResource_Handle_t DMAHandle;
    DMAResource_AddrPair_t PhysAddr;
    EIP207_Flow_Error_t res;
    EIP207_Flow_FR_InputData_t FlowData;
    EIP207_Flow_FR_Dscr_t * FlowDescriptor_p;
    List_Element_t * Element_p = (List_Element_t*)FlowHandle;
    AdapterPCL_Device_Instance_Data_t * Dev_p;

    LOG_INFO("\n\t PCL_Flow_Add \n");

    // validate interface id
    if (InterfaceId >= ADAPTER_PCL_MAX_FLUE_DEVICES)
        return PCL_INVALID_PARAMETER;

    // check valid flow handle
    if (Element_p == NULL)
        return PCL_INVALID_PARAMETER;

    FlowDescriptor_p = (EIP207_Flow_FR_Dscr_t *)Element_p->DataObject_p;

    // check valid flow record descriptor
    if (FlowDescriptor_p == NULL)
        return PCL_INVALID_PARAMETER;

    Dev_p = &Dev_Instance[InterfaceId];

    if (Adapter_Lock_CS_Get(&Dev_p->AdapterPCL_DevCS) == Adapter_Lock_NULL)
    {
        LOG_CRIT("PCL_Flow_Add: no device lock, not initialized?\n");
        return PCL_ERROR;
    }

    if (!Adapter_Lock_CS_Enter(&Dev_p->AdapterPCL_DevCS))
        return PCL_STATUS_BUSY;

    // check interface is initialised
    if (!Dev_p->PCL_IsInitialized)
    {
        LOG_CRIT("PCL_Flow_Add: Not initialized\n");
        goto error_exit;
    }

    // Fill in the input data
    DMAHandle = Adapter_DMABuf_Handle2DMAResourceHandle(FlowParams->transform);
    if (DMAHandle)
    {
        if (DMAResource_Translate(DMAHandle, DMARES_DOMAIN_BUS, &PhysAddr) < 0)
        {
            LOG_CRIT("PCL_Flow_Add: Failed to obtain physical address.\n");
            goto error_exit;
        }
    }
    else
    {
        PhysAddr.Domain    = DMARES_DOMAIN_BUS;
        PhysAddr.Address_p = AdapterPCL_Int_To_Ptr(
                                    EIP207_Flow_Record_Dummy_Addr_Get());
    }

    FlowData.Flags               = FlowParams->flags;
    FlowData.HashID.Word32[0]    = FlowParams->FlowID[0];
    FlowData.HashID.Word32[1]    = FlowParams->FlowID[1];
    FlowData.HashID.Word32[2]    = FlowParams->FlowID[2];
    FlowData.HashID.Word32[3]    = FlowParams->FlowID[3];
    FlowData.SW_FR_Reference     = FlowParams->FlowIndex;
    Adapter_AddrToWordPair(PhysAddr.Address_p,0,
                           &FlowData.Xform_DMA_Addr.Addr,
                           &FlowData.Xform_DMA_Addr.UpperAddr);

    FlowData.fLarge = false;

#ifndef ADAPTER_PCL_USE_LARGE_TRANSFORM_DISABLE
    /* Determine whether we have a large transform record. */
    if (DMAHandle)
    {
        DMAResource_Record_t * Rec_p = DMAResource_Handle2RecordPtr(DMAHandle);

        if (Rec_p->fIsLargeTransform)
           FlowData.fLarge = true;
    }
#endif // !ADAPTER_PCL_USE_LARGE_TRANSFORM_DISABLE

    LOG_INFO("\n\t\t EIP207_Flow_FR_Add \n");

    // Add the record
    res = EIP207_Flow_FR_Add(Dev_p->EIP207_IOArea_p,
                             FlowHashTableId,
                             FlowDescriptor_p,
                             &FlowData);
    if (res == EIP207_FLOW_OUT_OF_MEMORY_ERROR)
    {
        LOG_CRIT("PCL_FLow_Add: failed to install flow, out of memory\n");
    }
    else if (res == EIP207_FLOW_NO_ERROR)
    {
       PCL_Rc = PCL_STATUS_OK;
    }
    else
    {
        LOG_CRIT("PCL_FLow_Add: failed to install flow, internal error\n");
    }

error_exit:

    Adapter_Lock_CS_Leave(&Dev_p->AdapterPCL_DevCS);
    return PCL_Rc;
}


/*-----------------------------------------------------------------------------
 * PCL_Flow_Release
 */
PCL_Status_t
PCL_Flow_Release(
        const unsigned int InterfaceId,
        const unsigned int FlowHashTableId,
        const PCL_FlowHandle_t FlowHandle)
{
    PCL_Status_t PCL_Rc = PCL_ERROR;
    EIP207_Flow_FR_Dscr_t * FlowDescriptor_p;
    List_Element_t * Element_p = (List_Element_t*)FlowHandle;
    AdapterPCL_Device_Instance_Data_t * Dev_p;

    LOG_INFO("\n\t PCL_Flow_Release \n");

    IDENTIFIER_NOT_USED(InterfaceId);
    IDENTIFIER_NOT_USED(FlowHashTableId);
    IDENTIFIER_NOT_USED(FlowHandle);

    // check valid flow handle
    if (Element_p == NULL)
        return PCL_INVALID_PARAMETER;

    FlowDescriptor_p = (EIP207_Flow_FR_Dscr_t *)Element_p->DataObject_p;

    // check valid flow record descriptor
    if (FlowDescriptor_p == NULL)
        return PCL_INVALID_PARAMETER;

    Dev_p = &Dev_Instance[InterfaceId];

    if (Adapter_Lock_CS_Get(&Dev_p->AdapterPCL_DevCS) == Adapter_Lock_NULL)
    {
        LOG_CRIT("PCL_Flow_Release: no device lock, not initialized?\n");
        return PCL_ERROR;
    }

    if (!Adapter_Lock_CS_Enter(&Dev_p->AdapterPCL_DevCS))
        return PCL_STATUS_BUSY;

    // check interface is initialised
    if (!Dev_p->PCL_IsInitialized)
    {
        LOG_CRIT("PCL_Flow_Release: Not initialized\n");
        goto error_exit;
    }

    DMAResource_Release(FlowDescriptor_p->DMA_Handle);

    // Put the record descriptor back on the free list
    {
        List_Status_t List_Rc;

        List_Rc = List_AddToHead(Dev_p->FreeListID,
                                 NULL,
                                 Element_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT("PCL_Flow_Release: "
                     "failed to put descriptor on the free list\n");
            goto error_exit;
        }
    }

    PCL_Rc = PCL_STATUS_OK;

error_exit:
    Adapter_Lock_CS_Leave(&Dev_p->AdapterPCL_DevCS);

    return PCL_Rc;
}


/*-----------------------------------------------------------------------------
 * PCL_Flow_Get_ReadOnly
 */
PCL_Status_t
PCL_Flow_Get_ReadOnly(
        const PCL_FlowHandle_t FlowHandle,
        PCL_FlowParams_t * const FlowParams_p)
{
    EIP207_Flow_FR_Dscr_t * FlowDescriptor_p;
    EIP207_Flow_FR_OutputData_t FlowData;
    EIP207_Flow_Error_t res;
    List_Element_t * Element_p = (List_Element_t*)FlowHandle;

    LOG_INFO("\n\t PCL_Flow_Get_ReadOnly \n");

    // check valid flow handle
    if (Element_p == NULL)
        return PCL_INVALID_PARAMETER;

    FlowDescriptor_p = (EIP207_Flow_FR_Dscr_t *)Element_p->DataObject_p;

    // check valid flow record descriptor
    if (FlowDescriptor_p == NULL)
        return PCL_INVALID_PARAMETER;

    LOG_INFO("\n\t\t EIP207_Flow_FR_Read \n");

    res = EIP207_Flow_FR_Read(Dev_Instance[0].EIP207_IOArea_p,
                             0,
                             FlowDescriptor_p,
                             &FlowData);
    if (res == EIP207_FLOW_ARGUMENT_ERROR)
    {
        return PCL_INVALID_PARAMETER;
    }
    else if (res != EIP207_FLOW_NO_ERROR)
    {
        return PCL_ERROR;
    }

    FlowParams_p->LastTimeLo = FlowData.LastTimeLo;
    FlowParams_p->LastTimeHi = FlowData.LastTimeHi;

    FlowParams_p->PacketsCounterLo = FlowData.PacketsCounter;
    FlowParams_p->PacketsCounterHi = 0;

    FlowParams_p->OctetsCounterLo = FlowData.OctetsCounterLo;
    FlowParams_p->OctetsCounterHi = FlowData.OctetsCounterHi;

    return PCL_STATUS_OK;
}

/* end of file adapter_pcl_generic.c */
