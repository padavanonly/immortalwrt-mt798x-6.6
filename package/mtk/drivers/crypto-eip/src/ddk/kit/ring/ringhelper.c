/* ringhelper.c
 *
 * Ring Helper Library implementation.
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

#include "c_ringhelper.h"  // configuration options

#include "basic_defs.h"     // bool, MIN
#include "ringhelper.h"     // API to implement
#include "clib.h"           // memset


/*----------------------------------------------------------------------------
 * RingHelper_Init
 *
 * This routine must be called once to initialize the administration block
 * related to a ring.
 *
 * See header file for function specification.
 */
int
RingHelper_Init(
        volatile RingHelper_t * const Ring_p,
        volatile const RingHelper_CallbackInterface_t * const CallbackIF_p,
        const bool fSeparateRings,
        const unsigned int CommandRing_MaxDescriptors,
        const unsigned int ResultRing_MaxDescriptors)
{
#ifdef RINGHELPER_STRICT_ARGS
    if (Ring_p == NULL ||
        CallbackIF_p == NULL ||
        CommandRing_MaxDescriptors < 1)
    {
        // invalid argument
        return -1;
    }

    if (fSeparateRings)
    {
#ifdef RINGHELPER_REMOVE_SEPARATE_RING_SUPPORT
        // not supported
        return -1;
#else
        if (ResultRing_MaxDescriptors < 1)
        {
            // invalid arguments
            return -1;
        }
#endif
    }

    if (CallbackIF_p->ReadFunc_p == NULL ||
        CallbackIF_p->WriteFunc_p == NULL)
    {
        return -1;
    }

#ifndef RINGHELPER_REMOVE_STATUSFUNC
    if (CallbackIF_p->StatusFunc_p == NULL)
    {
        return -1;
    }
#endif

#endif /* RINGHELPER_STRICT_ARGS */

    Ring_p->CB = *CallbackIF_p;
    Ring_p->fSupportsDeviceReadPos = true;      // initial assumption
    Ring_p->IN_Size = CommandRing_MaxDescriptors;
    Ring_p->IN_Tail = 0;
    Ring_p->OUT_Head = 0;

#ifndef RINGHELPER_REMOVE_SEPARATE_RING_SUPPORT
    if (fSeparateRings)
    {
        // separate rings
        Ring_p->fSeparate = true;
        Ring_p->OUT_Size = ResultRing_MaxDescriptors;
    }
    else
#endif /* !RINGHELPER_REMOVE_SEPARATE_RING_SUPPORT */
    {
        // combined rings
        Ring_p->fSeparate = false;
        Ring_p->OUT_Size = CommandRing_MaxDescriptors;
    }

    return 0;   // success
}


/*----------------------------------------------------------------------------
 * RingHelper_Put
 *
 * This function tries to add a number of descriptors to the command ring
 * specified.
 *
 * See header file for function specification.
 */
int
RingHelper_Put(
        volatile RingHelper_t * const Ring_p,
        const void * Descriptors_p,
        const int DescriptorCount)
{
    int A, N, W1, W2;

#ifdef RINGHELPER_STRICT_ARGS
    if (Ring_p == NULL ||
        Descriptors_p == NULL ||
        DescriptorCount < 0)
    {
        return -1;
    }
#endif /* RINGHELPER_STRICT_ARGS */

    if (DescriptorCount == 0)
        return 0;

    W2 = 0;

#ifndef RINGHELPER_REMOVE_SEPARATE_RING_SUPPORT
    // out of the descriptors provided, calculate the maximum number of
    // descriptors that can be written sequentially before the ring is full.
    if (Ring_p->fSeparate)
    {
        // separate rings

        // ask how far the device has processed the ring
        // we do this on every call and do not cache the result
        int DeviceReadHead = -1;    // not supported

#ifndef RINGHELPER_REMOVE_STATUSFUNC
        if (Ring_p->fSupportsDeviceReadPos)
        {
            int res;

            res = Ring_p->CB.StatusFunc_p(
                                 Ring_p->CB.CallbackParam1_p,
                                 Ring_p->CB.CallbackParam2,
                                 &DeviceReadHead);

            if (res < 0)
                return res;     // ## RETURN ##

            // suppress these calls if the device does not support it
            if (DeviceReadHead < 0)
                Ring_p->fSupportsDeviceReadPos = false;
        }
#endif /* !RINGHELPER_REMOVE_STATUSFUNC */

        if (DeviceReadHead < 0)
        {
            // device does not expose its read position
            // this means we cannot calculate how much space is available
            // the WriteFunc will have to check, descriptor by descriptor
            A = Ring_p->IN_Size;

            // note: under this condition we rely on the implementation of
            // the callback interface to handle ring-full condition and not
            // overwrite existing descriptors. Because of this, we can
            // fill the ring to the limit and do not have to keep 1 free
            // position as done below.
        }
        else
        {
            unsigned int Device_IN_Head = (unsigned int)DeviceReadHead;

            // based on the device read position we can calculate
            // how many positions in the ring are free
            if (Ring_p->IN_Tail < Device_IN_Head)
            {
                // we have wrapped around
                // available space is between the two
                A = Device_IN_Head - Ring_p->IN_Tail;
            }
            else
            {
                // used positions are between the two pointers
                // rest is free
                A = Ring_p->IN_Size - (Ring_p->IN_Tail - Device_IN_Head);
            }

            // avoid filling the entire ring
            // so we can differentiate full from empty
            if (A != 0)
                A--;
        }
    }
    else
#endif /* !RINGHELPER_REMOVE_SEPARATE_RING_SUPPORT */
    {
        // combined rings

        // Critical: we have to be careful not to read the OUT_Head more
        //           than one, since it might change in between!
        unsigned int OUT_Head_copy = Ring_p->OUT_Head;

        // we can write descriptors up to the point where we expect the
        // result descriptors
        if (Ring_p->IN_Tail < OUT_Head_copy)
        {
            // used positions are around the wrap point
            // free positions are between the pointers
            A = OUT_Head_copy - Ring_p->IN_Tail;
        }
        else
        {
            // used positions are between the two pointers
            // rest is free
            A = Ring_p->IN_Size - (Ring_p->IN_Tail - OUT_Head_copy);
        }

        // avoid filling the entire ring
        // so we can differentiate full from empty
        // (when it contains all commands or all results)
        if (A != 0)
            A--;
    }

    // limit based on provided descriptors
    A = MIN(A, DescriptorCount);

    // limit for sequential writing
    N = MIN(A, (int)(Ring_p->IN_Size - Ring_p->IN_Tail));

    // bail out early if there is no space
    if (N == 0)
    {
        return 0;       // ## RETURN ##
    }

    W1 = Ring_p->CB.WriteFunc_p(
                        Ring_p->CB.CallbackParam1_p,
                        Ring_p->CB.CallbackParam2,
                        /*WriteIndex:*/Ring_p->IN_Tail,
                        /*WriteCount:*/N,
                        /*AvailableSpace*/A,
                        Descriptors_p,
                        DescriptorCount,
                        /*SkipCount:*/0);

    if (W1 <= 0)
    {
        //  0: no descriptors could be added
        // <0: failure
        return W1;      // ## RETURN ##
    }

    if (W1 == N &&
        W1 < DescriptorCount &&
        A > N)
    {
        // we have written all possible positions up to the end of the ring
        // now write the rest
        N = A - N;

        W2 = Ring_p->CB.WriteFunc_p(
                            Ring_p->CB.CallbackParam1_p,
                            Ring_p->CB.CallbackParam2,
                            /*WriteIndex:*/0,
                            /*WriteCount:*/N,
                            /*AvailableSpace*/N,
                            Descriptors_p,
                            DescriptorCount,
                            /*SkipCount:*/W1);

        if (W2 < 0)
        {
            // failure
            return W2;      // ## RETURN ##
        }
    }

    // now update the position for the next write
    {
        unsigned int i = Ring_p->IN_Tail + W1 + W2;

        // do not use % operator to avoid costly divisions
        if (i >= Ring_p->IN_Size)
            i -= Ring_p->IN_Size;

        Ring_p->IN_Tail = i;
    }

    // return how many descriptors were added
    return W1 + W2;
}


/*----------------------------------------------------------------------------
 * RingHelper_Get
 *
 * This routine retrieves a number of descriptors from the result ring
 * specified.
 *
 * See header file for function specification.
 */
int
RingHelper_Get(
        volatile RingHelper_t * const Ring_p,
        const int ReadyCount,
        void * Descriptors_p,
        const int DescriptorsLimit)
{
    int A, N;
    int R1, R2;

    R2 = 0;

#ifdef RINGHELPER_STRICT_ARGS
    if (Ring_p == NULL ||
        Descriptors_p == NULL ||
        DescriptorsLimit < 0)
    {
        return -1;
    }
#endif /* RINGHELPER_STRICT_ARGS */

    if (DescriptorsLimit == 0 ||
        ReadyCount == 0)
    {
        // no space in output buffer
        // or no descriptors ready
        return 0;
    }

    // calculate the maximum number of descriptors that can be retrieved
    // sequentially from this read position, taking into account the
    // DescriptorsLimit and the ReadyCount (if available)

    // A = entries in result ring from read position till end
    A = Ring_p->OUT_Size - Ring_p->OUT_Head;

    N = MIN(A, DescriptorsLimit);

    if (ReadyCount > 0)
        N = MIN(N, ReadyCount);

    // now retrieve this number of descriptors
    R1 = Ring_p->CB.ReadFunc_p(
                        Ring_p->CB.CallbackParam1_p,
                        Ring_p->CB.CallbackParam2,
                        /*ReadIndex:*/Ring_p->OUT_Head,
                        /*ReadLimit:*/N,
                        Descriptors_p,
                        /*SkipCount:*/0);

    if (R1 <= 0)
    {
        //  0: if we got nothing on the first call, we can stop here
        // <0: error while reading
        //     this means we cannot maintain read synchronization

        return R1;      // ## RETURN ##
    }

    // if we got the maximum, we can try to read more
    // after wrapping to the start of the buffer
    if (R1 == N &&
        R1 < DescriptorsLimit &&
        R1 != ReadyCount)
    {
        // A = number of entries in ring up to previous read-start position
        A = Ring_p->OUT_Head;

        N = MIN(A, DescriptorsLimit - R1);

        if (ReadyCount > 0)
            N = MIN(N, ReadyCount - R1);

        R2 = Ring_p->CB.ReadFunc_p(
                            Ring_p->CB.CallbackParam1_p,
                            Ring_p->CB.CallbackParam2,
                            /*ReadIndex:*/0,        // start of buffer
                            /*ReadLimit:*/N,
                            Descriptors_p,
                            /*SkipCount:*/R1);

        if (R2 < 0)
        {
            // failure
            return R2;      // ## RETURN ##
        }
    }

    // now update the position for the next read
    {
        unsigned int i = Ring_p->OUT_Head + R1 + R2;

        // do not use % operator to avoid costly divisions
        if (i >= Ring_p->OUT_Size)
            i -= Ring_p->OUT_Size;

        Ring_p->OUT_Head = i;
    }

    // return the number of descriptors read
    return R1 + R2;
}


/*----------------------------------------------------------------------------
 * RingHelper_FillLevel_Get
 *
 * This function return a number of filled (used) descriptors in the ring
 * specified.
 *
 * See header file for function specification.
 */
int
RingHelper_FillLevel_Get(
        volatile RingHelper_t * const Ring_p)
{
#ifdef RINGHELPER_STRICT_ARGS
    if (Ring_p == NULL)
    {
        return -1;
    }
#endif /* RINGHELPER_STRICT_ARGS */

#ifndef RINGHELPER_REMOVE_SEPARATE_RING_SUPPORT
    // out of the descriptors provided, calculate the maximum number of
    // descriptors that can be written sequentially before the ring is full.
    if (Ring_p->fSeparate)
    {
        // separate rings

        return -1; // not implemented
    }
    else
#endif  /* !RINGHELPER_REMOVE_SEPARATE_RING_SUPPORT */
    {
        // combined rings

        unsigned int FillLevel;
        unsigned int OUT_Head_copy;

        // Critical: we have to be careful not to read the OUT_Head more
        //           than one, since it might change in between!
        OUT_Head_copy = Ring_p->OUT_Head;

        if (Ring_p->IN_Tail < OUT_Head_copy)
        {
            // used positions are around the wrap point
            // free positions are between the pointers
            FillLevel = Ring_p->IN_Size - (OUT_Head_copy - Ring_p->IN_Tail);
        }
        else
        {
            // used positions are between the two pointers
            // rest is free
            FillLevel = Ring_p->IN_Tail - OUT_Head_copy;
        }

        // avoid filling the entire ring
        // so we can differentiate full from empty
        // (when it contains all commands or all results)
        if (FillLevel < Ring_p->IN_Size)
            FillLevel++;

        return (int)FillLevel;
    }
}


/* end of file ringhelper.c */
