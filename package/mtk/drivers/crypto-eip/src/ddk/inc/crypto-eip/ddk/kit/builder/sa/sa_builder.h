/* sa_builder.h
 *
 * Main API for the SA Builder.
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


#ifndef SA_BUILDER_H_
#define SA_BUILDER_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "sa_builder_params.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */
typedef enum
{
    SAB_STATUS_OK,
    SAB_INVALID_PARAMETER,
    SAB_UNSUPPORTED_FEATURE,
    SAB_ERROR
} SABuilder_Status_t;


/*----------------------------------------------------------------------------
 * SABuilder_GetSizes();
 *
 * Compute the required sizes in 32-bit words of any of up to three memory
 * areas used by the SA.
 *
 * SAParams_p (input)
 *   Pointer to the SA parameters structure.
 * SAWord32Count_p (output)
 *   The size of the normal SA buffer,
 * SAStateWord32Count_p (output)
 *   The size of any SA state record.
 * ARC4StateWord32Count_p (output) T
 *   The size of any ARCFOUR state buffer (output).
 *
 * When the SA state record or ARCFOUR state buffer are not required by
 * the packet engine for this transform, the corresponding size outputs
 * are returned as zero.
 *
 * If any of the output parameters is a null pointer,
 * the corresponding size will not be returned.
 *
 * The SAParams_p structure must be fully filled in: it must have the
 * same contents as it would have when SABuilder_BuildSA is called.
 *
 * Return:
 * SAB_STATUS_OK on success
 * SAB_INVALID_PARAMETER when the record referenced by SAParams_p is invalid,
 * SAB_UNSUPPORTED_FEATURE when SAParams_p describes an operations that
 *    is not supported on the hardware for which this SA builder
 *    is configured.
 */
SABuilder_Status_t
SABuilder_GetSizes(
    SABuilder_Params_t *const SAParams_p,
    unsigned int *const SAWord32Count_p,
    unsigned int *const SAStateWord32Count_p,
    unsigned int *const ARC4StateWord32Count_p);


/*----------------------------------------------------------------------------
 * SABuilder_BuildSA();
 *
 * Construct the SA record for the operation described in SAParams_p in
 * up to three memory buffers. Update the field offset values in the
 * SA parameter structure.
 *
 * SAParams_p (input, output)
 *    Pointer to the SA parameters structure. Field offset values will
 *    be updated.
 * SABuffer_p (output)
 *    Pointer to the the normal SA buffer,
 * SAStateBuffer_p (output)
 *    Pointer to the SA state record buffer,
 * ARC4StateBuffer_p (output)
 *    Pointer to the ARCFOUR state buffer.
 *
 * Each of the Buffer arguments must point to a word-aligned
 * memory buffer whose size in words is at least equal to the
 * corresponding size parameter returned by SABuilder_GetSizes().
 *
 * If any of the three buffers is not required for the SA (the
 * corresponding size in SABuilder_GetSizes() is 0), the corresponding
 * Buffer arguments to this function may be a null pointer.
 *
 * Return:
 * SAB_STATUS_OK on success
 * SAB_INVALID_PARAMETER when SAParams_p is invalid, or if any of
 *    the buffer arguments  is a null pointer while the corresponding buffer
 *    would be required for the operation.
 * SAB_UNSUPPORTED_FEATURE when SAParams_p describes an operations that
 *    is not supported on the hardware for which this SA builder
 *    is configured.
 */
SABuilder_Status_t
SABuilder_BuildSA(
    SABuilder_Params_t * const SAParams_p,
    uint32_t *const SABuffer_p,
    uint32_t *const SAStateBuffer_p,
    uint32_t *const ARC4StateBuffer_p);


/*----------------------------------------------------------------------------
 * SABuilder_SetLargeTransformOffset();
 *
 * Set the (global) offset between the fields in Large transform records versus
 * small transform records. For some hardware configurations (especially those
 * with 1024-bit anti-replay masks) large transform records must contain more
 * space than usual. This function is intended to be used when the hardware
 * configuration is only known at runtime.
 *
 * Offset (inout)
 *     Offset value in words.
 * Return:
 * SAB_STATUS_OK on success
 * SAB_INVALID_PARAMETER when the offset is out of range.
 * SAB_UNSUPPORTED_FEATURE when the SA Builder is configured without support
 * for this feature.
 */
SABuilder_Status_t
SABuilder_SetLargeTransformOffset(
    unsigned int Offset);


#endif /* SA_BUILDER_H_ */


/* end of file sa_builder.h */
