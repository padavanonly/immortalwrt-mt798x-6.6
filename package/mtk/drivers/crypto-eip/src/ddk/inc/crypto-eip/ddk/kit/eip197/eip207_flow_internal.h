/* eip207_flow_internal.h
 *
 * EIP-207 Flow Control Internal interface
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

#ifndef EIP207_FLOW_INTERNAL_H_
#define EIP207_FLOW_INTERNAL_H_


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// EIP-207 Flow Control Driver Library Generic interface
#include "eip207_flow_generic.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip207_flow.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // BIT definitions, bool, uint32_t

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t
#include "device_rw.h"          // Read32, Write32

// Driver Framework DMA Resource API
#include "dmares_types.h"       // DMAResource_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define EIP207_FLOW_DSCR_DUMMY_ADDRESS_BYTE             0

#define EIP207_FLOW_VALUE_64BIT_MAX_NOF_READ_ATTEMPTS   2

#define IOAREA(_p) ((volatile EIP207_Flow_True_IOArea_t *)_p)

#ifdef EIP207_FLOW_STRICT_ARGS
#define EIP207_FLOW_CHECK_POINTER(_p) \
    if (NULL == (_p)) \
        return EIP207_FLOW_ARGUMENT_ERROR;
#define EIP207_FLOW_CHECK_INT_INRANGE(_i, _min, _max) \
    if ((_i) < (_min) || (_i) > (_max)) \
        return EIP207_FLOW_ARGUMENT_ERROR;
#define EIP207_FLOW_CHECK_INT_ATLEAST(_i, _min) \
    if ((_i) < (_min)) \
        return EIP207_FLOW_ARGUMENT_ERROR;
#define EIP207_FLOW_CHECK_INT_ATMOST(_i, _max) \
    if ((_i) > (_max)) \
        return EIP207_FLOW_ARGUMENT_ERROR;
#else
/* EIP207_FLOW_STRICT_ARGS undefined */
#define EIP207_FLOW_CHECK_POINTER(_p)
#define EIP207_FLOW_CHECK_INT_INRANGE(_i, _min, _max)
#define EIP207_FLOW_CHECK_INT_ATLEAST(_i, _min)
#define EIP207_FLOW_CHECK_INT_ATMOST(_i, _max)
#endif /*end of EIP207_FLOW_STRICT_ARGS */

#ifdef EIP207_FLOW_DEBUG_FSM
// EIP-207 Flow Control API States
typedef enum
{
    EIP207_FLOW_STATE_INITIALIZED  = 1,
    EIP207_FLOW_STATE_ENABLED,
    EIP207_FLOW_STATE_INSTALLED,
    EIP207_FLOW_STATE_FATAL_ERROR
} EIP207_Flow_State_t;
#endif // EIP207_FLOW_DEBUG_FSM

// Physical (bus) address that can be used for DMA by the device
typedef struct
{
    // 32-bit physical bus address
    uint32_t Addr;

    // upper 32-bit part of a 64-bit physical address
    // Note: this value has to be provided only for 64-bit addresses,
    // in this case Addr field provides the lower 32-bit part
    // of the 64-bit address, for 32-bit addresses this field is ignored,
    // and should be set to 0.
    uint32_t UpperAddr;

} EIP207_Flow_Internal_Address_t;

// Hash Table parameters
typedef struct
{
    // DMA Resource handle for the Hash Table (HT)
    DMAResource_Handle_t HT_DMA_Handle;

    // Pointer to the Flow Descriptor Table (DT)
    void * DT_p;

    // DMA (physical) base address for records for lookup
    EIP207_Flow_Internal_Address_t BaseAddr;

    // HT table size (in EIP207_Flow_HashTable_Entry_Count_t units)
    unsigned int HT_TableSize;

    // Pointer to List head of free HTE descriptors for overflow hash buckets
    void * FreeList_Head_p;

    // True when FLUEC is enabled
    bool fLookupCached;

} EIP207_Flow_HT_Params_t;

// I/O Area, used internally
typedef struct
{
    // Classification engine state
    uint32_t State;

    // Device handle representing the Classification engine
    Device_Handle_t Device;

#ifdef EIP207_FLOW_DEBUG_FSM
    unsigned int Rec_InstalledCounter;
#endif

    // Flow Hash Table parameters
    EIP207_Flow_HT_Params_t  HT_Params [EIP207_FLOW_MAX_NOF_FLOW_HASH_TABLES_TO_USE];

} EIP207_Flow_True_IOArea_t;


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP207_Flow_Internal_Read64
 */
EIP207_Flow_Error_t
EIP207_Flow_Internal_Read64(
        const DMAResource_Handle_t Handle,
        const unsigned int Value64_WordOffsetLo,
        const unsigned int Value64_WordOffsetHi,
        uint32_t * const Value64_Lo,
        uint32_t * const Value64_Hi);


#ifdef EIP207_FLOW_DEBUG_FSM
/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_State_Set
 *
 */
EIP207_Flow_Error_t
EIP207_Flow_State_Set(
        EIP207_Flow_State_t * const CurrentState,
        const EIP207_Flow_State_t NewState);
#endif // EIP207_FLOW_DEBUG_FSM


#endif /* EIP207_FLOW_INTERNAL_H_ */


/* end of file eip207_flow_internal.h */
