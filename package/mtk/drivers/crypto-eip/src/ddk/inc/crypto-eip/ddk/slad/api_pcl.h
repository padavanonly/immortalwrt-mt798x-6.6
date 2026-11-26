/* api_pcl.h
 *
 * Packet Classification API.
 *
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
*****************************************************************************/


#ifndef API_PCL_H_
#define API_PCL_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"

// Dependency on DMA Buffer Allocation API (api_dmabuf.h)
#include "api_dmabuf.h"         // DMABuf_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */
typedef enum
{
    PCL_STATUS_OK,              // Operation is successful
    PCL_STATUS_BUSY,            // Device busy, try again later
    PCL_INVALID_PARAMETER,      // Invalid input parameter
    PCL_UNSUPPORTED_FEATURE,    // Feature is not implemented
    PCL_OUT_OF_MEMORY_ERROR,    // Out of memory
    PCL_ERROR                   // Operation has failed
} PCL_Status_t;

/* Flags for selector parameters */
#define PCL_SELECT_IPV4     BIT_0   // Packet is IPv4.
#define PCL_SELECT_IPV6     BIT_1   // Packet is IPv6.


/*----------------------------------------------------------------------------
 * PCL_SelectorParams_t
 *
 * This data structure represents the packet parameters (such as IP addresses
 * and ports) that select a particular flow.
 */
typedef struct
{
    uint32_t flags;    // Bitwise or of zero or more PCL_SELECT_* flags.
    uint8_t *SrcIp;    // IPv4 (4 bytes)source address.
    uint8_t *DstIp;    // IPv4 (4 bytes)destination address.
    uint8_t IpProto;   // IP protocol (UDP, ESP).
    uint16_t SrcPort;  // Source port for UDP.
    uint16_t DstPort;  // Destination port for UDP.
    uint32_t spi;      // SPI in IPsec
    uint16_t epoch;    // Epoch in DTLS.
} PCL_SelectorParams_t;

/* Flags for flow parameters */
#define PCL_FLOW_INBOUND  BIT_1  // Flow is for inbound IPsec packets.
#define PCL_FLOW_SLOWPATH BIT_2  // Packets should not be processed in hardware

/*----------------------------------------------------------------------------
 * PCL_FlowParams_t
 *
 * This data structure represents the flow data structure visible to the
 * application. The actual flow record may contain additional information,
 * such as the physical addresses of the transform record and links for the
 * Flow Hash Table.
 */
typedef struct
{
    uint32_t FlowID[4];       // The 128-bit flow ID computed by the flow hash
                              // function from the selector parameters.

    uint32_t FlowIndex;       // Reference to flow table entry in application.
                              // An Ipsec implementation may maintain a flow
                              // table in software that contains more
                              // information than is accessible to the
                              // Classification Engine. FlowIndex may be
                              // used by software to refer bo this software
                              // flow record.

    uint32_t flags;           // Bitwise of zero or more PCL_FLOW_* flags.
    uint32_t SourceInterface; // Representation of the network interface

    // Transform that is to be applied.
    DMABuf_Handle_t transform;

    // Time last accessed, in engine clock cycles.
    uint32_t LastTimeLo;
    uint32_t LastTimeHi;

    // Number of packets processed by this flow.
    uint32_t PacketsCounterLo;
    uint32_t PacketsCounterHi;

    // Number of octets processed by this flow.
    uint32_t OctetsCounterLo;
    uint32_t OctetsCounterHi;

} PCL_FlowParams_t;

/*----------------------------------------------------------------------------
 * PCL_TransformParams_t
 *
 * This data structure represents parameters from the transform record
 * that the application must be able to access after the transform
 * record is created.
 */
typedef struct
{
    // 32-bit sequence number of outbound transform.
    uint32_t SequenceNumber;

    // Time last of transform record access, in engine clock cycles.
    uint32_t LastTimeLo;
    uint32_t LastTimeHi;

    // Number of packets processed for this transform.
    uint32_t PacketsCounterLo;
    uint32_t PacketsCounterHi;

    // Number of octets processed for this transform.
    uint32_t OctetsCounterLo;
    uint32_t OctetsCounterHi;

} PCL_TransformParams_t;

// Flow Record handle
typedef void * PCL_FlowHandle_t;


/*----------------------------------------------------------------------------
 * PCL_Init
 *
 * Initialize the classification functionality. After the initialization no
 * transform records are registered and no flow records exist. This function
 * must be called before any other function of this API is called.
 *
 * InterfaceId (input)
 *     Identifier of the Classification Engine interface
 *
 * NofFlowHashTables (input)
 *     Number of the Flow Hash Tables to set up
 *
 * Return:
 *     PCL_STATUS_OK
 *     PCL_INVALID_PARAMETER
 *     PCL_UNSUPPORTED_FEATURE
 *     PCL_ERROR
 */
PCL_Status_t
PCL_Init(
        const unsigned int InterfaceId,
        const unsigned int NofFlowHashTables);


/*----------------------------------------------------------------------------
 * PCL_UnInit
 *
 * Uninitialize the classification functionality. Before this
 * function may be called, all the transform records must be unregistered and
 * all the flow records must be destroyed.
 *
 * InterfaceId (input)
 *     Identifier of the Classification Engine interface
 *
 * Return:
 *     PCL_STATUS_OK
 *     PCL_INVALID_PARAMETER
 *     PCL_ERROR
 */
PCL_Status_t
PCL_UnInit(
        const unsigned int InterfaceId);


/*----------------------------------------------------------------------------
 * PCL_Flow_DMABuf_Handle_Get
 *
 * Obtain DMABuf_Handle_t type handle from the provided PCL_FlowHandle_t type
 * handle.
 *
 * FlowHandle (input)
 *     Handle of the flow record.
 *
 * DMAHandle_p (output)
 *     Pointer to the memory location where the handle representing
 *     the allocated flow record as a DMA resource (see DMABuf API)
 *     will be stored.
 *
 * Return:
 *     PCL_STATUS_OK
 *     PCL_INVALID_PARAMETER
 */
PCL_Status_t
PCL_Flow_DMABuf_Handle_Get(
        const PCL_FlowHandle_t FlowHandle,
        DMABuf_Handle_t * const DMAHandle_p);


/*----------------------------------------------------------------------------
 * PCL_Flow_Hash
 *
 * Compute a 128-bit hash value from the provided selector parameters.
 * This value can be assumed to be unique for every different
 * combination of system parameters. The hash computation is the exact
 * same computation that the classification hardware would do for a packet with
 * the same parameters. Which fields are hashed, depends on the specified
 * protocol (UDP will hash source and destination ports, ESP will hash SPI).
 * This value is subsequently used to identify looked-up records.
 *
 * SelectorParams (input)
 *     Set of packet parameters to select a particular flow.
 *
 * FlowID_Word32ArrayOf4 (output)
 *     128-bit hash value that will be used to uniquely identify a looked-up
 *     record. This must point to a buffer that can hold 4 32-bit words.
 *
 * Return:
 *     PCL_STATUS_OK
 *     PCL_INVALID_PARAMETER
 *     PCL_UNSUPPORTED_FEATURE
 *     PCL_ERROR
 */
PCL_Status_t
PCL_Flow_Hash(
        const PCL_SelectorParams_t * const SelectorParams,
        uint32_t * FlowID_Word32ArrayOf4);


/*-----------------------------------------------------------------------------
 * PCL_Flow_Lookup
 *
 * Find a flow in the Flow Hash Table with a matching flow ID.
 * If no such flow is found, return a null handle.
 *
 * InterfaceId (input)
 *     Identifier of the Classification Engine interface
 *
 * FlowHashTableId (input)
 *     Flow Hash Table identifier that should be used for the flow record
 *     lookup
 *
 * FlowID_Word32ArrayOf4 (input)
 *     128-bit flow ID to find.
 *
 * FlowHandle_p (output)
 *     Handle to access the flow record found, e.g. via PCL_Flow_Get_ReadOnly()
 *     (or null handle of no flow found).
 *
 * Return:
 *     PCL_STATUS_OK:           flow record is found
 *     PCL_INVALID_PARAMETER
 *     PCL_ERROR:               no flow record is found
 */
PCL_Status_t
PCL_Flow_Lookup(
        const unsigned int InterfaceId,
        const unsigned int FlowHashTableId,
        const uint32_t * FlowID_Word32ArrayOf4,
        PCL_FlowHandle_t * const FlowHandle_p);


/*-----------------------------------------------------------------------------
 * PCL_Flow_Alloc
 *
 * Allocate all dynamic storage resources for a flow record, but do not
 * add a flow record to the table.
 *
 * Note:  This function will allocate a buffer to store the actual flow record
 *        and all administration used by the driver (represented by FlowHandle).
 *
 * InterfaceId (input)
 *     Identifier of the Classification Engine interface
 *
 * FlowHashTableId (input)
 *     Flow Hash Table identifier where the flow record should be allocated
 *
 * FlowHandle_p (output)
 *     Handle to represent the allocated flow record. This will be used
 *     as an input parameter to other flow related functions in this API.
 *
 * Return:
 *     PCL_STATUS_OK
 *     PCL_INVALID_PARAMETER
 *     PCL_ERROR
 */
PCL_Status_t
PCL_Flow_Alloc(
        const unsigned int InterfaceId,
        const unsigned int FlowHashTableId,
        PCL_FlowHandle_t * const FlowHandle_p);


/*-----------------------------------------------------------------------------
 * PCL_Flow_Add
 *
 * Create a new flow record from the supplied parameters and add it to the
 * Flow Hash Table. Its dynamic storage resources must already be allocated.
 *
 * Note:  This function will add a flow record to the flow table.
 *        The FlowHandle must represent an allocated, but unused
 *        flow record buffer.
 *        No flow record may already exist with the same flow ID.
 *        The application is responsible for this.
 *
 * InterfaceId (input)
 *     Identifier of the Classification Engine interface
 *
 * FlowHashTableId (input)
 *     Flow Hash Table identifier where the flow record should be added
 *
 * FlowParams (input)
 *     Contents of the flow that is to be added.
 *
 * FlowHandle (input)
 *     Handle representing the storage space for the flow record
 *     (as returned by PCL_Flow_Alloc). If this function returns with
 *     PCL_STATUS_OK, the flow record will be in use and the handle can be
 *     used with PCL_Flow_Remove() or PCL_Flow_Get_ReadOnly().
 *
 * Return:
 *     PCL_STATUS_OK
 *     PCL_INVALID_PARAMETER
 *     PCL_ERROR
 */
PCL_Status_t
PCL_Flow_Add(
        const unsigned int InterfaceId,
        const unsigned int FlowHashTableId,
        const PCL_FlowParams_t * const FlowParams,
        const PCL_FlowHandle_t FlowHandle);


/*-----------------------------------------------------------------------------
 * PCL_Flow_Remove
 *
 * Remove the indicated flow record from the Flow Hash Table. If the
 * flow record references a transform record then that transform
 * record must be unregistered after the flow record is removed.
 * The flow handle must represent a flow record that is currently in the
 * Flow Hash Table (by PCL_Flow_Add).
 *
 * When this function returns, the dynamic storage resources of the flow
 * record are still allocated and they can be reused by another call to
 * PCL_Flow_Add() or the resources can be released by PCL_Flow_Release().
 *
 * InterfaceId (input)
 *     Identifier of the Classification Engine interface
 *
 * FlowHashTableId (input)
 *     Flow Hash Table identifier where the flow record should be removed
 *
 * FlowHandle (input)
 *     Handle of the flow record to be removed.
 *
 * Return:
 *     PCL_STATUS_OK
 *     PCL_INVALID_PARAMETER
 *     PCL_ERROR
 */
PCL_Status_t
PCL_Flow_Remove(
        const unsigned int InterfaceId,
        const unsigned int FlowHashTableId,
        const PCL_FlowHandle_t FlowHandle);


/*-----------------------------------------------------------------------------
 * PCL_Flow_Release
 *
 * Release all dynamic storage resources of a flow record. The
 * FlowHandle must represent an allocated, but unused flow record
 * buffer.
 *
 * InterfaceId (input)
 *     Identifier of the Classification Engine interface
 *
 * FlowHashTableId (input)
 *     Flow Hash Table identifier where the flow record was originally
 *     allocated.
 *
 * FlowHandle (input)
 *     Handle of the flow record to be released. When this function returns,
 *     this handle us no longer a valid input to any of the flow related
 *     API functions.
 *
 * Return:
 *     PCL_STATUS_OK
 *     PCL_INVALID_PARAMETER
 *     PCL_ERROR
 */
PCL_Status_t
PCL_Flow_Release(
        const unsigned int InterfaceId,
        const unsigned int FlowHashTableId,
        const PCL_FlowHandle_t FlowHandle);


/*-----------------------------------------------------------------------------
 * PCL_Flow_Get_ReadOnly
 *
 * Read the contents of a flow record with no intent to update it.
 *
 * FlowHandle (input)
 *     Handle of the flow record to be read.
 *
 * FlowParams_p (output)
 *     Contents read from the flow record (storage space provided by
 *     application).
 *
 * Return:
 *     PCL_STATUS_OK
 *     PCL_INVALID_PARAMETER
 *     PCL_ERROR
 */
PCL_Status_t
PCL_Flow_Get_ReadOnly(
        const PCL_FlowHandle_t FlowHandle,
        PCL_FlowParams_t * const FlowParams_p);


/*-----------------------------------------------------------------------------
 * PCL_Transform_Register
 *
 * Register the transform record. Flow records may only reference transform
 * records that have been made registered with this function.
 *
 * Before this function is called, the application has already
 * allocated a buffer through the DMABuf API and has already filled it
 * with transform data, for example via using the SA Builder API.
 *
 * TransformDMAHandle (input)
 *     DMABuf Handle of the transform to be added.
 *
 * Return:
 *     PCL_STATUS_OK when operation was successful.
 *     PCL_INVALID_PARAMETER
 *     PCL_ERROR
 */
PCL_Status_t
PCL_Transform_Register(
        const DMABuf_Handle_t XformDMAHandle);


/*-----------------------------------------------------------------------------
 * PCL_Transform_UnRegister
 *
 * Unregister the transform. The buffer represented
 * by XformDMAHandle remains allocated and the application is responsible
 * for releasing it. The flow record must be removed before its transform
 * record can be unregistered.
 *
 * Note: When this function is called, no flow records may reference
 *       the unregistered transform record.
 *
 * XformDMAHandle (input)
 *     DMABuf Handle of the transform to be removed.
 *
 * Return:
 *     PCL_STATUS_OK when operation was successful.
 *     PCL_INVALID_PARAMETER
 *     PCL_ERROR
 */
PCL_Status_t
PCL_Transform_UnRegister(
        const DMABuf_Handle_t XformDMAHandle);


/*-----------------------------------------------------------------------------
 * PCL_Transform_Get_ReadOnly
 *
 * Read a subset of the contents of the transform record into a
 * parameters structure.
 *
 * XformDMAHandle (input)
 *     DMABuf Handle of the transform to be accessed.
 *
 * TransformParams_p (output)
 *     Transform parameters read from the transform record.
 *
 * Return:
 *     PCL_STATUS_OK
 *     PCL_INVALID_PARAMETER
 *     PCL_ERROR
 */
PCL_Status_t
PCL_Transform_Get_ReadOnly(
        const DMABuf_Handle_t XformDMAHandle,
        PCL_TransformParams_t * const TransformParams_p);


#endif /* API_PCL_H_ */


/* end of file api_pcl.h */
