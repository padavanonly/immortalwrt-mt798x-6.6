/* eip207_flow_generic.h
 *
 * EIP-207 Flow Control Generic API:
 *      Flow hash table initialization,
 *      Flow and Transform records creation and removal
 *
 * This API is not re-entrant.
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : ddk197                                                   */
/*   Version       : 5.6.1                                                    */
/*   Configuration : DDK-197-GPL                                              */
/*                                                                            */
/*   Date          : 2022-Dec-16                                              */
/*                                                                            */
/* Copyright (c) 2008-2022 by Rambus, Inc. and/or its subsidiaries.           */
/*                                                                            */
/* This program is free software: you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation, either version 2 of the License, or          */
/* any later version.                                                         */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               */
/* GNU General Public License for more details.                               */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program. If not, see <http://www.gnu.org/licenses/>.       */
/* -------------------------------------------------------------------------- */

#ifndef EIP207_FLOW_GENERIC_H_
#define EIP207_FLOW_GENERIC_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint8_t, uint32_t, bool

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t

// Driver Framework DMA Resource API
#include "dmares_types.h"       // DMAResource_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define EIP207_FLOW_SELECT_IPV4                BIT_0
#define EIP207_FLOW_SELECT_IPV6                BIT_1

// Hash ID size in 32-bit words (128 bits)
#define EIP207_FLOW_HASH_ID_WORD_COUNT           4

// Flag bitmask for the Flags field in the flow record,
// see EIP207_Flow_OutputData_t
#define EIP207_FLOW_FLAG_INBOUND    BIT_1

// Place holder for device specific internal I/O data
typedef struct
{
    // Pointer to the internal data storage to be allocated by the API user,
    // EIP207_Flow_IOArea_ByteCount_Get() provides the size for the allocation
    void * Data_p;

} EIP207_Flow_IOArea_t;

/*----------------------------------------------------------------------------
 * EIP207_Flow_Error_t
 *
 * Status (error) code type returned by these API functions
 * See each function "Return value" for details.
 *
 * EIP207_FLOW_NO_ERROR : successful completion of the call.
 * EIP207_FLOW_RECORD_REMOVE_BUSY: flow record removal is ongoing
 * EIP207_FLOW_UNSUPPORTED_FEATURE_ERROR : not supported.
 * EIP207_FLOW_ARGUMENT_ERROR :  invalid argument for a function parameter.
 * EIP207_FLOW_BUSY_RETRY_LATER : device is busy.
 * EIP207_FLOW_OUT_OF_MEMORY_ERROR : no more free entries in Hash Table to add
 *                                   new record (or out of overflow buckets).
 * EIP207_FLOW_ILLEGAL_IN_STATE : illegal state transition
 */
typedef enum
{
    EIP207_FLOW_NO_ERROR = 0,
    EIP207_FLOW_RECORD_REMOVE_BUSY,
    EIP207_FLOW_UNSUPPORTED_FEATURE_ERROR,
    EIP207_FLOW_ARGUMENT_ERROR,
    EIP207_FLOW_INTERNAL_ERROR,
    EIP207_FLOW_OUT_OF_MEMORY_ERROR,
    EIP207_FLOW_ILLEGAL_IN_STATE
} EIP207_Flow_Error_t;

// Table/group size encoding in entries
typedef enum
{
    EIP207_FLOW_HASH_TABLE_ENTRIES_32 = 0,
    EIP207_FLOW_HASH_TABLE_ENTRIES_64,
    EIP207_FLOW_HASH_TABLE_ENTRIES_128,
    EIP207_FLOW_HASH_TABLE_ENTRIES_256,
    EIP207_FLOW_HASH_TABLE_ENTRIES_512,
    EIP207_FLOW_HASH_TABLE_ENTRIES_1024,
    EIP207_FLOW_HASH_TABLE_ENTRIES_2048,
    EIP207_FLOW_HASH_TABLE_ENTRIES_4096,
    EIP207_FLOW_HASH_TABLE_ENTRIES_8192,
    EIP207_FLOW_HASH_TABLE_ENTRIES_16384,
    EIP207_FLOW_HASH_TABLE_ENTRIES_32768,
    EIP207_FLOW_HASH_TABLE_ENTRIES_65536,
    EIP207_FLOW_HASH_TABLE_ENTRIES_131072,
    EIP207_FLOW_HASH_TABLE_ENTRIES_262144,
    EIP207_FLOW_HASH_TABLE_ENTRIES_524288,
    EIP207_FLOW_HASH_TABLE_ENTRIES_1048576,
    EIP207_FLOW_HASH_TABLE_ENTRIES_MAX = EIP207_FLOW_HASH_TABLE_ENTRIES_1048576
} EIP207_Flow_HashTable_Entry_Count_t;

// Flow hash ID
typedef struct
{
    // 128-bit flow hash ID value
    uint32_t Word32 [EIP207_FLOW_HASH_ID_WORD_COUNT];

} EIP207_Flow_ID_t;

// Physical (bus) address that can be used for DMA by the device
typedef struct
{
    // 32-bit physical bus address
    uint32_t Addr;

    // upper 32-bit part of a 64-bit physical address
    // Note: this value has to be provided only for 64-bit addresses,
    // in this case Addr field provides the lower 32-bit part
    // of the 64-bit address, for 32bit addresses this field is ignored,
    // and should be set to 0.
    uint32_t UpperAddr;

} EIP207_Flow_Address_t;

// This data structure represents the packet parameters (such as IP addresses
// and ports) that select a particular flow.
typedef struct
{
    // Bitwise or of zero or more EIP207_FLOW_SELECT_* flags
    uint32_t  Flags;

    // IPv4 (4 bytes) source address
    uint8_t * SrcIp_p;

    // IPv4 (4 bytes) destination address
    uint8_t * DstIp_p;

    // IP protocol (UDP, ESP) code as per RFC
    uint8_t   IpProto;

    // Source port for UDP
    uint16_t  SrcPort;

    // Destination port for UDP
    uint16_t  DstPort;

    // SPI in IPsec
    uint32_t  SPI;

    // Epoch in DTLS
    uint16_t  Epoch;
} EIP207_Flow_SelectorParams_t;

// Flow record input data required for the flow record creation
typedef struct
{
    // Bitwise of zero or more EIP207_FLOW_FLAG_* flags, see above
    uint32_t Flags;

    // Flow record hash ID
    EIP207_Flow_ID_t HashID;

    // Physical (DMA/bus) address of the transform record
    EIP207_Flow_Address_t Xform_DMA_Addr;

    // Software flow record reference
    uint32_t SW_FR_Reference;

    // Transform record type, true - large, false - small
    bool fLarge;

} EIP207_Flow_FR_InputData_t;

// Transform record input data required for the Transform record creation
typedef struct
{
    // Transform record hash ID
    EIP207_Flow_ID_t HashID;

    // Transform record type, true - large, false - small
    bool fLarge;

} EIP207_Flow_TR_InputData_t;

// Data that can be read from flow record (flow output data)
// This data is written in the flow record by the firmware running
// inside the classification engine
typedef struct
{
    // The time stamp of the last record access, in engine clock cycles
    uint32_t LastTimeLo;   // Low 32-bits
    uint32_t LastTimeHi;   // High 32-bits

    // Number of packets processed for this flow record, 32-bit integer
    uint32_t PacketsCounter;

    // Number of octets processed for this flow record, 64-bit integer
    uint32_t OctetsCounterLo;  // Low 32-bits
    uint32_t OctetsCounterHi;  // High 32-bits

} EIP207_Flow_FR_OutputData_t;

// Data that can be read from transform record (output data)
// This data is written in the transform record by the firmware running
// inside the classification engine
typedef struct
{
    // The time stamp of the last record access, in engine clock cycles
    uint32_t LastTimeLo;   // Low 32-bits
    uint32_t LastTimeHi;   // High 32-bits

    // 32-bit sequence number of outbound transform
    uint32_t SequenceNumber;

    // Number of packets processed for this transform record, 32-bit integer
    uint32_t PacketsCounter;

    // Number of octets processed for this transform record, 64-bit integer
    uint32_t OctetsCounterLo;   // Low 32-bits
    uint32_t OctetsCounterHi;   // High 32-bits

} EIP207_Flow_TR_OutputData_t;

// Hash table (HT) initialization structure
typedef struct
{
    // Handle for the DMA resource describing the DMA-safe buffer for the Hash
    // Table. One HT entry is one hash bucket.
    DMAResource_Handle_t HT_DMA_Handle;

    // HT DMA (physical) address
    EIP207_Flow_Address_t * HT_DMA_Address_p;

    // HT maximum table size N in HW-specific encoding
    EIP207_Flow_HashTable_Entry_Count_t HT_TableSize;

    // Descriptor Table (DT) host address,
    // this buffer does not have to be DMA-safe
    void * DT_p;

    // DT maximum entry count (size), one entry is one descriptor
    unsigned int DT_EntryCount;

} EIP207_Flow_HT_t;

// General record descriptor
typedef struct
{
    // Record DMA Resource handle
    DMAResource_Handle_t DMA_Handle;

    // Record DMA (bus/physical) address
    EIP207_Flow_Address_t DMA_Addr;

    // Pointer to internal data structure for internal use,
    // the API user must not write or read this field
    void * InternalData_p;

} EIP207_Flow_Dscr_t;

// Flow record data structure,
// Note: while allocating this place holder for this data structure its size
//       must be obtained via the EIP207_Flow_FR_Dscr_ByteCount_Get() function
typedef EIP207_Flow_Dscr_t EIP207_Flow_FR_Dscr_t;

// Transform record data structure
// Note: while allocating this place holder for this data structure its size
//       must be obtained via the EIP207_Flow_TR_Dscr_ByteCount_Get() function
typedef EIP207_Flow_Dscr_t EIP207_Flow_TR_Dscr_t;


/*----------------------------------------------------------------------------
 * EIP207_Flow_IOArea_ByteCount_Get
 *
 * This function returns the required size of the I/O Area (in bytes).
 */
unsigned int
EIP207_Flow_IOArea_ByteCount_Get(void);


/*----------------------------------------------------------------------------
 * EIP207_Flow_HT_Entry_WordCount_Get
 *
 * This function returns the required size of one Hash Table entry
 * (in 32-bit words).
 *
 * This function can be used for the memory allocation for the HT.
 */
unsigned int
EIP207_Flow_HT_Entry_WordCount_Get(void);


/*----------------------------------------------------------------------------
 * EIP207_Flow_HTE_Dscr_ByteCount_Get
 *
 * This function returns the required size of one HT Entry Descriptor
 * (in bytes).
 *
 * This function can be used for the memory allocation for the DT.
 */
unsigned int
EIP207_Flow_HTE_Dscr_ByteCount_Get(void);


/*----------------------------------------------------------------------------
 * EIP207_Flow_FR_Dscr_ByteCount_Get
 *
 * This function returns the required size of one Flow Record Descriptor
 * (in bytes).
 *
 * This function can be used for the memory allocation for the Flow Record
 * Descriptor.
 */
unsigned int
EIP207_Flow_FR_Dscr_ByteCount_Get(void);


/*----------------------------------------------------------------------------
 * EIP207_Flow_TR_Dscr_ByteCount_Get
 *
 * This function returns the required size of one Transform Record Descriptor
 * (in bytes).
 *
 * This function can be used for the memory allocation for the Transform Record
 * Descriptor.
 */
unsigned int
EIP207_Flow_TR_Dscr_ByteCount_Get(void);


/*----------------------------------------------------------------------------
 * EIP207_Flow_Record_Dummy_Addr_Get
 *
 * This function returns the dummy record address.
 */
unsigned int
EIP207_Flow_Record_Dummy_Addr_Get(void);


/*----------------------------------------------------------------------------
 * EIP207_Flow_Init
 *
 * This function performs the initialization of the EIP-207 Flow Control SW
 * interface and transits the API to the "Flow Control Initialized" state for
 * the requested classification device.
 *
 * This function returns the EIP207_FLOW_UNSUPPORTED_FEATURE_ERROR error code
 * when it detects a mismatch in the Flow Control Driver Library configuration
 * and the used EIP-207 HW capabilities.
 *
 * Note: This function should be called either after the HW Reset or
 *       after the Global SW Reset.
 *       This function should be called for the Classification device
 *       identified by the Device parameter before any other functions of
 *       this API (except for EIP207_Flow_ID_Compute()) can be called
 *       for the IO Area returned for this Device.
 *
 * IOArea_p (output)
 *     Pointer to the place holder in memory for the IO Area
 *     for the Classification device identified by the Device parameter.
 *     The memory must be allocated by the caller before invoking
 *     this function.
 *
 * Device (input)
 *     Handle for the EIP-207 Flow Control device instance returned
 *     by Device_Find().
 *
 * Return value
 *     EIP207_FLOW_NO_ERROR
 *     EIP207_FLOW_ARGUMENT_ERROR
 *     EIP207_FLOW_ILLEGAL_IN_STATE
 */
EIP207_Flow_Error_t
EIP207_Flow_Init(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const Device_Handle_t Device);


/*----------------------------------------------------------------------------
 * EIP207_Flow_HashTable_Install
 *
 * This function installs the Hash Table as well as the Descriptor
 * Table and transits the API to the "Flow Control Enabled" state for
 * the classification device identified by the IO Area.
 *
 * Note: This function should be called after the EIP207_Flow_Init()
 *       function is called for the required Classification device.
 *
 * IOArea_p (input)
 *     Pointer to the IO Area of the required Classification device.
 *
 * HashTableId (input)
 *     Index of the flow hash table that must be set up.
 *
 * HT_p (Input)
 *     Pointer for the Hash Table (HT) identified by the HashTableId.
 *     One HT entry is one hash bucket.
 *
 * fLookupCached (input)
 *     Set to true to enable the FLUE cache
 *     (improves performance)
 *
 * fReset (input)
 *     Set to true to re-initialize the HT.
 *
 * Return value
 *     EIP207_FLOW_NO_ERROR
 *     EIP207_FLOW_UNSUPPORTED_FEATURE_ERROR
 *     EIP207_FLOW_ARGUMENT_ERROR
 *     EIP207_FLOW_ILLEGAL_IN_STATE
 */
EIP207_Flow_Error_t
EIP207_Flow_HashTable_Install(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        const EIP207_Flow_HT_t * HT_p,
        bool fLookupCached,
        bool fReset);


/*----------------------------------------------------------------------------
 * EIP207_Flow_RC_BaseAddr_Set
 *
 * Set the FLUE Record Cache base address for the records.
 *
 * Note: This function should be called after the EIP207_Flow_Init()
 *       function is called for the required Classification device.
 *
 * IOArea_p (input)
 *     Pointer to the IO Area of the required Classification device.
 *
 * HashTableId (input)
 *     Index of the Hash Table for which the Record Cache base address
 *     must be set up.
 *
 * FlowBaseAddr (input)
 *     Base address of the Flow Record Cache. All the flow records must be
 *     allocated within 4GB address region from this FlowBaseAddr base address.
 *
 * TransformBaseAddr (input)
 *     Base address of the Transform Record Cache. All the transform records
 *     must be allocated within 4GB address region from this TransformBaseAddr
 *     base address.
 *
 * Return value
 *     EIP207_FLOW_NO_ERROR
 *     EIP207_FLOW_ARGUMENT_ERROR
 *     EIP207_FLOW_ILLEGAL_IN_STATE
 */
EIP207_Flow_Error_t
EIP207_Flow_RC_BaseAddr_Set(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        const EIP207_Flow_Address_t * const FlowBaseAddr,
        const EIP207_Flow_Address_t * const TransformBaseAddr);


/*----------------------------------------------------------------------------
 * EIP207_Flow_ID_Compute
 *
 * This function performs the flow hash ID computation.
 *
 * IOArea_p (input)
 *     Pointer to the IO Area of the required Classification device.
 *
 * HashTableId (input)
 *     Index of the flow hash table.
 *
 * SelectorParams_p (input)
 *     Pointer to the data structure that contain parameters that should be
 *     used for the flow ID computation
 *
 * FlowID (output)
 *     Pointer to the data structure where the calculated flow hash ID will
 *     be stored
 *
 * Return value
 *     EIP207_FLOW_NO_ERROR
 *     EIP207_FLOW_UNSUPPORTED_FEATURE_ERROR
 *     EIP207_FLOW_ARGUMENT_ERROR
 */
EIP207_Flow_Error_t
EIP207_Flow_ID_Compute(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        const EIP207_Flow_SelectorParams_t * const SelectorParams_p,
        EIP207_Flow_ID_t * const FlowID);


/*----------------------------------------------------------------------------
 * EIP207_Flow_FR_WordCount_Get
 *
 * This function returns the required size of one Flow Record
 * (in 32-bit words).
 */
unsigned int
EIP207_Flow_FR_WordCount_Get(void);


/*----------------------------------------------------------------------------
 * EIP207_Flow_FR_Add
 *
 * This function adds the provided flow record to the Flow Hash Table
 * so that the classification device can look it up.
 *
 * This function returns the EIP207_FLOW_UNSUPPORTED_FEATURE_ERROR error code
 * when it detects a mismatch in the Flow Control Driver Library configuration
 * and the used EIP-207 HW capabilities.
 *
 * IOArea_p (input)
 *     Pointer to the IO Area of the required Classification device.
 *
 * HashTableId (input)
 *     Index of the flow hash table where the flow record must be added.
 *
 * FR_Dscr_p (input)
 *     Pointer to the data structure that describes the flow record to be added.
 *     This descriptor must exist along with the flow record it describes until
 *     that flow record is removed by the EIP207_Flow_Record_Remove() function.
 *     The flow descriptor pointer cannot be 0.
 *
 * FlowInData_p (input)
 *     Pointer to the data structure that contains the input data that will
 *     be used for filling in the flow record. The buffer holding this data
 *     structure can be freed after this function returns.
 *
 * Return value
 *     EIP207_FLOW_NO_ERROR
 *     EIP207_FLOW_UNSUPPORTED_FEATURE_ERROR
 *     EIP207_FLOW_ARGUMENT_ERROR
 *     EIP207_FLOW_ILLEGAL_IN_STATE
 */
EIP207_Flow_Error_t
EIP207_Flow_FR_Add(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        EIP207_Flow_FR_Dscr_t * const FR_Dscr_p,
        const EIP207_Flow_FR_InputData_t * const FlowInData_p);


/*----------------------------------------------------------------------------
 * EIP207_Flow_FR_Read
 *
 * This function reads data from the provided flow record.
 *
 * IOArea_p (input)
 *     Pointer to the IO Area of the required Classification device.
 *
 * HashTableId (input)
 *     Index of the flow hash table where the flow record must be removed from.
 *
 * FR_Dscr_p (output)
 *     Pointer to the data structure that describes the flow record
 *     to be read. Cannot be 0.
 *
 * FlowData_p (output)
 *     Pointer to the data structure where the read flow data will be stored.
 *
 * Return value
 *     EIP207_FLOW_NO_ERROR
 *     EIP207_FLOW_UNSUPPORTED_FEATURE_ERROR
 *     EIP207_FLOW_ARGUMENT_ERROR
 *     EIP207_FLOW_ILLEGAL_IN_STATE
 */
EIP207_Flow_Error_t
EIP207_Flow_FR_Read(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        EIP207_Flow_FR_Dscr_t * const FR_Dscr_p,
        EIP207_Flow_FR_OutputData_t * const FlowData_p);


/*----------------------------------------------------------------------------
 * EIP207_Flow_TR_WordCount_Get
 *
 * This function returns the required size of one Transform Record
 * (in 32-bit words).
 */
unsigned int
EIP207_Flow_TR_WordCount_Get(void);


/*----------------------------------------------------------------------------
 * EIP207_Flow_TR_Read
 *
 * This function reads output data from the provided transform record.
 *
 * IOArea_p (input)
 *     Pointer to the IO Area of the required Classification device.
 *
 * HashTableId (input)
 *     Index of the flow hash table where the flow record must be removed from.
 *
 * TR_Dscr_p (input)
 *     Pointer to the transform record descriptor. The transform record
 *     descriptor is only used during this function execution.
 *     It can be discarded after this function returns.
 *
 * XformData_p (output)
 *     Pointer to the data structure where the read transform data will
 *     be stored.
 *
 * Return value
 *     EIP207_FLOW_NO_ERROR
 *     EIP207_FLOW_UNSUPPORTED_FEATURE_ERROR
 *     EIP207_FLOW_ARGUMENT_ERROR
 *     EIP207_FLOW_ILLEGAL_IN_STATE
 */
EIP207_Flow_Error_t
EIP207_Flow_TR_Read(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        const EIP207_Flow_TR_Dscr_t * const TR_Dscr_p,
        EIP207_Flow_TR_OutputData_t * const XformData_p);


#endif /* EIP207_FLOW_GENERIC_H_ */


/* end of file eip207_flow_generic.h */
