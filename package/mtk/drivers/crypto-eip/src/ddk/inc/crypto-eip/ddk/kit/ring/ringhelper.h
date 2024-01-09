/* ringhelper.h
 *
 * Ring Helper Library API specification.
 */

/*****************************************************************************
* Copyright (c) 2008-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_RINGHELPER_H
#define INCLUDE_GUARD_RINGHELPER_H

#include "basic_defs.h"

/*----------------------------------------------------------------------------
 * RingHelperCB_WriteFunc_t
 *
 * This routine must write a limited number of descriptors to the command ring
 * and hand these off to the device for processing, using the mechanism
 * supported by the device.
 *
 * If this is a command-only ring (not shared with resutls) AND the
 * implementation of the callback interface is unable to provide the read
 * position of the device in the command ring (see RingHelperCB_StatusFunc_t)
 * then the Ring Helper module cannot know if the ring is almost full.
 * Therefore, under these conditions, the implementation of this callback
 * is also required to:
 * - check that the descriptor position to be written is indeed free and does
 *   not contain an unprocessed command descriptor (this happens when the ring
 *   becomes full).
 *
 * This function can be called in response to invoking the RingHelper_Put API
 * function and is expected to be fully re-entrant. If this is not possible,
 * the caller must avoid the re-entrance of the RingHelper_Put API function.
 *
 * CallbackParam1_p
 * CallbackParam2
 *     These parameters are anonymous for the Ring Helper and were provided by
 *     the caller during the ring initialization. They have meaning for the
 *     the module that is invoked by this callback.
 *
 * WriteIndex
 *     Zero-based index number in the command ring from where the descriptors
 *     must be written. The caller guarantees that the descriptors can be
 *     written sequentially and no index number wrapping will be required.
 *     The implementation is expected to know the address of the command ring
 *     and size of a descriptor.
 *
 * WriteCount
 *     The number of descriptors to immediately add to the command ring.
 *
 * AvailableSpace
 *     The currently available number of free positions in the command ring
 *     to where the descriptors can be written.
 *     The following always holds:
 *
 *     WriteCount <= AvailableSpace
 *
 *
 * Descriptors_p
 *     Pointer to the memory where the descriptors are currently stored.
 *     The descriptors are available sequentially and back-to-back.
 *
 * DescriptorCount
 *     The total number of descriptors that were requested to be added
 *     to the command ring by the caller of the RingHelper_Put function.
 *     When a write operation is split in two (due to index number wrapping),
 *     this parameter is greater than WriteCount. Also if there is not enough
 *     space available in the ring, this paramater is greater than WriteCount.
 *     So the following holds:
 *
 *     DescriptorCount == WriteCount, if no index number wrapping occured
 *                        in the caller's decision before calling this routine,
 *                        and there is enough space in the ring;
 *     DescriptorCount > WriteCount, if an index number wrapping occured
 *                       in the caller's decision before calling this routine
 *                       or there is not enough space in the ring;
 *
 * DescriptorSkipCount
 *     The number of descriptors from Descriptors_p already copied. These
 *     descriptors must be skipped. This parameter is required because the Ring
 *     Helper does not know the size of a descriptor.
 *     When an operation is split in two (due to index number wrapping),
 *     the first call has this parameter set to zero,
 *     and the second has it set to the number of descriptors written
 *     by the first call.
 *
 * Return Value
 *     <0  Failure code, this can be a Driver Library specific error code.
 *      0  No descriptors could be written, but no explicit error
 *     >0  Actual number of descriptors written
 *
 * It should be noted that returning a failure code will cause synchronization
 * with the device to be lost and probably requires a reset of the device to
 * recover.
 */
typedef int (* RingHelperCB_WriteFunc_t)(
                    void * const CallbackParam1_p,
                    const int CallbackParam2,
                    const unsigned int WriteIndex,
                    const unsigned int WriteCount,
                    const unsigned int AvailableSpace,
                    const void * Descriptors_p,
                    const int DescriptorCount,
                    const unsigned int DescriptorSkipCount);


/*----------------------------------------------------------------------------
 * RingHelperCB_ReadFunc_t
 *
 * This routine is expected to read a limited number of descriptors from the
 * result ring and then mark these positions as free, allowing new result
 * descriptors (separate rings) or command descriptors (shared ring) to be
 * written to these position. The exact method used depends on the device
 * implementation.
 *
 * This function can be called in response to invoking the RingHelper_Get API
 * function and is expected to be fully re-entrant. If this is not possible,
 * the caller must avoid the re-entrance of the RingHelper_Get API function.
 *
 * CallbackParam1_p
 * CallbackParam2
 *     These parameters are anonymous for the Ring Helper and were provided by
 *     the caller during the ring initialization. They have meaning for the
 *     the module that is invoked by this callback.
 *
 * ReadLimit
 *     The maximum number of descriptors that fit in Descriptors_p.
 *     Also the maximum number of descriptors that can be read sequentially
 *     without having to wrap the ReadIndex.
 *     Also the maximum number of descriptors that are ready, if this was
 *     indicated by the ReadyCount parameter in the call to RingHelper_Get.
 *
 * ReadIndex
 *     Zero-based position in the result ring where the first descriptor can be
 *     read. The caller guarantees that up to ReadLimit descriptors can be read
 *     sequentially from this position without having to wrap the ReadIndex.
 *     The implementation is expected to know the address of the command ring
 *     and size of a descriptor.
 *
 * Descriptors_p
 *     Pointer to the block of memory where up to ReadLimit whole descriptors
 *     can be written back-to-back by the implementation.
 *
 * DescriptorSkipCount
 *     The number of descriptors already stored from Descriptors_p. This memory
 *     must be skipped. This parameter is required because the Ring Helper does
 *     not know the size of a descriptor.
 *     When an operation is split in two, the first call has this parameter set
 *     to zero, and the second has it set to the number of descriptors returned
 *     by the first call.
 *
 * Return Value
 *    <0  Failure code, this can be a Driver Library specific error code.
 *     0  No ready descriptors were available in the result ring.
 *    >0  Actual number of descriptors copied from the result ring to the
 *        buffer pointed to by Descriptors_p.
 *
 * It should be noted that returning a failure code will cause synchronization
 * with the device to be lost and probably requires a reset of the device to
 * recover.
 */
typedef int (* RingHelperCB_ReadFunc_t)(
                    void * const CallbackParam1_p,
                    const int CallbackParam2,
                    const unsigned int ReadIndex,
                    const unsigned int ReadLimit,
                    void * Descriptors_p,
                    const unsigned int DescriptorSkipCount);


/*----------------------------------------------------------------------------
 * RingHelperCB_StatusFunc_t
 *
 * This routine reads and return the ring status from the device.
 *
 * This function can be called in response to invoking the RingHelper_Put API
 * function and is expected to be fully re-entrant. If this is not possible,
 * the caller must avoid the re-entrance of the RingHelper_Put API function.
 *
 * CallbackParam1_p
 * CallbackParam2
 *     These parameters are anonymous for the Ring Helper and were provided by
 *     the caller during the ring initialization. They have meaning for the
 *     the module that is invoked by this callback.
 *
 * DeviceReadPos_p
 *     Pointer to the output parameter that will receive the most recent read
 *     position used by the device. The value is the zero-based descriptor
 *     index in the command ring. The caller will assume that the device has
 *     not yet processed this descriptor.
 *     The value -1 must be returned when the device cannot provide this info.
 *     The implementation is expected to consistently return this value and
 *     not to return -1 selectively. When the function has returned -1 once,
 *     it is assumed not to support this functionality and will not be called
 *     anew thereafter.
 *
 * Return Value
 *    <0  Failure code, this can be a Driver Library specific error code.
 *     0  Success  (also to be used when *DeviceReadPos_p was set to -1)
 *
 * It should be noted that returning a failure code will cause synchronization
 * with the device to be lost and probably requires a reset of the device to
 * recover.
 */
typedef int (* RingHelperCB_StatusFunc_t)(
                    void * const CallbackParam1_p,
                    const int CallbackParam2,
                    int * const DeviceReadPos_p);


/*----------------------------------------------------------------------------
 * RingHelper_CallbackInterface_t
 *
 * Data structure containing pointers to callback functions that will be used
 * by the Ring Helper Library to manipulate descriptors in the ring.
 */
typedef struct
{
    // pointers to the functions that form the Callback Interface
    // see type definitions above for details

    // note: all three API functions are mandatory to provide
    RingHelperCB_WriteFunc_t  WriteFunc_p;
    RingHelperCB_ReadFunc_t   ReadFunc_p;
    RingHelperCB_StatusFunc_t StatusFunc_p;

    // the following two parameters will be used as parameters when invoking
    // the above callback functions. The example usage is shown below.
    void * CallbackParam1_p;        // typically I/O Area pointer
    int CallbackParam2;             // typically Ring Number

} RingHelper_CallbackInterface_t;


/*----------------------------------------------------------------------------
 * RingHelper_t
 *
 * Data structure used to hold the ring administration data. The caller must
 * maintain an instance of this data structure for each ring.
 *
 * The caller should not access this data structure directly.
 *
 * For currency analysis, the following details have been specified:
 * - RingHelper_Init writes all fields.
 * - RingHelper_Put and RingHelper_Get do NOT write shared fields and can
 *   therefore be used concurrently.
 * - RingHelper_Get and RingHelper_Notify are multiple exlusive use by design.
 */
typedef struct
{
    unsigned int IN_Size;
    unsigned int IN_Tail;               // written by Put

    unsigned int OUT_Size;
    unsigned int OUT_Head;              // written by Get

    bool fSeparate;
    bool fSupportsDeviceReadPos;        // written by Put

    // callback interface
    RingHelper_CallbackInterface_t CB;

} RingHelper_t;


/*----------------------------------------------------------------------------
 * RingHelper_Init
 *
 * This routine must be called once to initialize a ring.
 *
 * The Ring Helper module will start to use the ring(s) from index zero. The
 * caller must make sure the device is configured accordingly.
 *
 * Ring_p
 *     Pointer to the ring administration data.
 *
 * CallbackIF_p
 *     References to the callback functions and parameters that will be used
 *     by the Ring Helper to request manipulation of the descriptors.
 *     The entire structure pointed to by this parameters will be copied.
 *
 * fSeparateRings
 *     This parameter indicates whether the command and result rings share one
 *     ring (one block of memory) or use separate rings (two memory blocks).
 *
 * MaxDescriptors_CommandRing
 * MaxDescriptors_ResultRing
 *     The maximum number of descriptors that fit in each of the rings.
 *     The MaxDescriptors_ResultRing is ignored when fSeparateRings is false.
 *
 * Return value
 *    <0  Failure code
 *     0  Success
 *
 * None of the other API functions may be called for this Ring_p before this
 * function returns with a success return value.
 */
int
RingHelper_Init(
        volatile RingHelper_t * const Ring_p,
        volatile const RingHelper_CallbackInterface_t * const CallbackIF_p,
        const bool fSeparateRings,
        const unsigned int MaxDescriptors_CommandRing,
        const unsigned int MaxDescriptors_ResultRing);


/*----------------------------------------------------------------------------
 * RingHelper_Put
 *
 * This routine can be used to add one or more descriptors to a specific
 * command ring.
 *
 * Ring_p
 *     Pointer to the ring administration data.
 *
 * Descriptors_p
 *     Pointer to where to read the array of descriptors.
 *
 * DescriptorCount
 *     Number of descriptors stored back-to-back in the array pointed to by
 *     Descriptors_p.
 *
 * Return Value
 *     <0  Failure code, this can be a Driver Library specific error code
 *         returned from the RingHelperCB_WriteFunc_t callback function.
 *     >0  Number of descriptors actually added to the command ring.
 *      0  The command ring is full, no descriptors could be added
 *         The caller should retry when the device has indicated that some
 *         descriptors have been processed.
 *
 * This function is not re-entrant for the same Ring_p. It is allowed to call
 * RingHelper_Get concurrently, even for the same Ring_p.
 */
int
RingHelper_Put(
        volatile RingHelper_t * const Ring_p,
        const void * Descriptors_p,
        const int DescriptorCount);


/*----------------------------------------------------------------------------
 * RingHelper_Get
 *
 * This routine can be used to read one or more descriptors from a specific
 * result ring.
 *
 * Ring_p
 *     Pointer to the ring administration data.
 *
 * ReadyCount
 *     The number of descriptors the caller guarantees are available in the
 *     result ring. This information is available from some devices.
 *     Use -1 if this information is not availalble. In this case the
 *     implementation of the read function (part of the callback interface)
 *     must check this information when retrieving the descriptors.
 *
 * Descriptors_p
 *     Pointer to the buffer where the descriptors will be written.
 *
 * DescriptorsLimit
 *     The size of the buffer pointed to by Descriptors_p, expressed in the
 *     maximum number of whole descriptors that fit into the buffer.
 *     Or, if smaller than the buffer size, the maximum number of whole
 *     descriptors that will be retrieved during this call.
 *
 * Return Value
 *     <0 Failure code, this can be a Driver Library specific error code
 *         returned from the RingHelperCB_ReadFunc_t callback function.
 *      0 No descriptors ready to be retrieved, retry later
 *     >0 Number of descriptors actually written from Descriptors_p
 *
 * This function is not re-entrant for the same Ring_p. It is allowed to call
 * RingHelper_Put concurrently, even for the same Ring_p.
 */
int
RingHelper_Get(
        volatile RingHelper_t * const Ring_p,
        const int ReadyCount,
        void * Descriptors_p,
        const int DescriptorsLimit);


/*----------------------------------------------------------------------------
 * RingHelper_FillLevel_Get
 *
 * This function returns a number of filled (used) descriptors in the ring
 * specified.
 *
 * Ring_p
 *     Pointer to the ring administration data.
 *
 * Return Value
 *     <0 Failure code, this can be a Driver Library specific error code
 *      0 No filled descriptors, empty ring
 *     >0 Number of filled descriptors in the ring
 *
 * This function is not re-entrant for the same Ring_p. It is not allowed
 * to call it concurrently with RingHelper_Put for the same Ring_p.
 */
int
RingHelper_FillLevel_Get(
        volatile RingHelper_t * const Ring_p);


#endif /* Include Guard */


/* end of file ringhelper.h */
