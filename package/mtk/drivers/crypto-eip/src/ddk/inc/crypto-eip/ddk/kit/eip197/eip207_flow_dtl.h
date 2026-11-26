/* eip207_flow_dtl.h
 *
 * EIP-207 Flow Control API, FLUE DTL extensions:
 *      Large Transform record support,
 *      Transform records addition,
 *      Flow and Transform records removal,
 *      Direct Transform Record Lookup
 *
 * This API should be used for the In-line IPsec and Fast-Path IPsec
 * use cases only!
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

#ifndef EIP207_FLOW_DTL_H_
#define EIP207_FLOW_DTL_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"           // uint8_t, uint32_t, bool

// Driver Framework Device API
#include "device_types.h"         // Device_Handle_t

// Driver Framework DMA Resource API
#include "dmares_types.h"         // DMAResource_Handle_t

// EIP-207 Driver Library Flow Control Generic API
#include "eip207_flow_generic.h"  // EIP207_FLOW_*


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * EIP207_Flow_DTL_FR_Remove
 *
 * This function removes the flow record from
 * the HT and the classification engine so that the engine can not look it up.
 *
 * Note: only one flow record can be removed at a time for one instance
 *       of the classification engine.
 *
 * IOArea_p (input)
 *     Pointer to the IO Area of the required Classification device.
 *
 * HashTableId (input)
 *     Index of the flow hash table where the flow record must be removed from
 *
 * FR_Dscr_p (input)
 *     Pointer to the flow descriptor data structure that describes
 *     the flow record to be removed. The flow descriptor buffer can
 *     be discarded after the flow record is removed.
 *     The flow descriptor pointer cannot be 0.
 *
 * Return value
 *     EIP207_FLOW_NO_ERROR
 *     EIP207_FLOW_RECORD_REMOVE_BUSY
 *     EIP207_FLOW_UNSUPPORTED_FEATURE_ERROR
 *     EIP207_FLOW_ARGUMENT_ERROR
 *     EIP207_FLOW_ILLEGAL_IN_STATE
 */
EIP207_Flow_Error_t
EIP207_Flow_DTL_FR_Remove(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        EIP207_Flow_FR_Dscr_t * const FR_Dscr_p);


/*----------------------------------------------------------------------------
 * EIP207_Flow_DTL_TR_Large_WordCount_Get
 *
 * This function returns the required size of one Large Transform Record
 * (in 32-bit words).
 */
unsigned int
EIP207_Flow_DTL_TR_Large_WordCount_Get(void);


/*----------------------------------------------------------------------------
 * EIP207_Flow_DTL_TR_Add
 *
 * This function adds the provided transform record to the Hash Table
 * so that the classification device can look it up (Direct Transform Lookup).
 *
 * This function returns the EIP207_FLOW_UNSUPPORTED_FEATURE_ERROR error code
 * when it detects a mismatch in the Flow Control Driver Library configuration
 * and the used EIP-207 HW capabilities.
 *
 * Note: this function must be called only for those transform records
 *       which can be directly looked up by the Classification device.
 *
 * IOArea_p (input)
 *     Pointer to the IO Area of the required Classification device.
 *
 * HashTableId (input)
 *     Index of the flow hash table where the flow record must be added.
 *
 * TR_Dscr_p (input)
 *     Pointer to the data structure that describes the transform record.
 *     This descriptor must exist along with the record it describes until
 *     that record is removed by the EIP207_Flow_DTL_TR_Remove() function.
 *     The transform descriptor pointer cannot be 0.
 *
 * XformInData_p (input)
 *     Pointer to the data structure that contains the input data that will
 *     be used for adding the transform record. The buffer holding this data
 *     structure can be freed after this function returns.
 *
 * Return value
 *     EIP207_FLOW_NO_ERROR
 *     EIP207_FLOW_UNSUPPORTED_FEATURE_ERROR
 *     EIP207_FLOW_ARGUMENT_ERROR
 *     EIP207_FLOW_ILLEGAL_IN_STATE
 */
EIP207_Flow_Error_t
EIP207_Flow_DTL_TR_Add(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        EIP207_Flow_TR_Dscr_t * const TR_Dscr_p,
        const EIP207_Flow_TR_InputData_t * const XformInData_p);


/*----------------------------------------------------------------------------
 * EIP207_Flow_DTL_TR_Large_Read
 *
 * This function reads output data from the provided large transform record.
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
EIP207_Flow_DTL_TR_Large_Read(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        const EIP207_Flow_TR_Dscr_t * const TR_Dscr_p,
        EIP207_Flow_TR_OutputData_t * const XformData_p);


/*----------------------------------------------------------------------------
 * EIP207_Flow_DTL_TR_Remove
 *
 * This function removes the requested transform record from the classification
 * engine.
 *
 * Note: this function must be called only for those transform records
 *       which can be directly looked up by the Classification device.
 *
 * IOArea_p (input)
 *     Pointer to the IO Area of the required Classification device.
 *
 * HashTableId (input)
 *     Index of the flow hash table where the flow record must be removed from.
 *
 * TR_Dscr_p (input)
 *     Pointer to the transform record descriptor.
 *
 * Return value
 *     EIP207_FLOW_NO_ERROR
 *     EIP207_FLOW_UNSUPPORTED_FEATURE_ERROR
 *     EIP207_FLOW_ARGUMENT_ERROR
 *     EIP207_FLOW_ILLEGAL_IN_STATE
 */
EIP207_Flow_Error_t
EIP207_Flow_DTL_TR_Remove(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        EIP207_Flow_TR_Dscr_t * const TR_Dscr_p);


#endif /* EIP207_FLOW_DTL_H_ */


/* end of file eip207_flow_dtl.h */
