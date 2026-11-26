/* eip207_flow_hte_dscr_dtl.h
 *
 * Helper functions for operations with the HTE Descriptor DTL,
 * interface
 *
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

#ifndef EIP207_FLOW_HTE_DSCR_DTL_H_
#define EIP207_FLOW_HTE_DSCR_DTL_H_

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // uint32_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Record offset mask values
#define EIP207_FLOW_HTE_REC_OFFSET_1        0x01
#define EIP207_FLOW_HTE_REC_OFFSET_2        0x02
#define EIP207_FLOW_HTE_REC_OFFSET_3        0x04
#define EIP207_FLOW_HTE_REC_OFFSET_ALL      MASK_3_BITS

// Type of records for lookup by FLUE
typedef enum
{
    EIP207_FLOW_REC_INVALID = 0,
    EIP207_FLOW_REC_FLOW,
    EIP207_FLOW_REC_TRANSFORM_SMALL,
    EIP207_FLOW_REC_TRANSFORM_LARGE
} EIP207_Flow_Record_Type_t;

// Record states
typedef enum
{
    EIP207_FLOW_REC_STATE_INSTALLED = 1,
    EIP207_FLOW_REC_STATE_REMOVED
} EIP207_Flow_Rec_State_t;

// HTE descriptor list
typedef struct
{
    // Pointer to the next descriptor in the chain
    void * NextDscr_p;

    // Pointer to the previous descriptor in the chain
    void * PrevDscr_p;

} EIP207_Flow_HTE_Dscr_List_t;

// Hash Table Entry (HTE) descriptor
typedef struct
{
    // Hash Bucket byte offset from the Hash Table base address
    uint32_t Bucket_ByteOffset;

    // Record reference count,
    // 0 when Hash Bucket refers to no records for lookup
    unsigned int RecordCount;

    // True when descriptor is for an overflow Hash Bucket
    bool fOverflowBucket;

    // Mask for used record offsets fields in the Hash Bucket,
    // see EIP207_FLOW_HTE_REC_OFFSET_*
    uint32_t UsedRecOffsMask;

    // List of HTE descriptors, either the free list of unused descriptors or
    // the hash bucket overflow chain
    EIP207_Flow_HTE_Dscr_List_t List;

} EIP207_Flow_HTE_Dscr_t;

// Record data
typedef struct
{
    // Record type
    EIP207_Flow_Record_Type_t Type;

    // Host address (pointer) for the HTE descriptor associated with the record
    EIP207_Flow_HTE_Dscr_t * HTE_Dscr_p;

    // Slot number in the Hash Bucket descriptor used by this record
    unsigned int Slot;

} EIP207_Flow_HTE_Dscr_RecData_t;


/*----------------------------------------------------------------------------
 * EIP207_Flow_HTE_Dscr_List_Prev_Get
 *
 * Gets the previous HTE descriptor for the provided one
 * in the descriptor list
 */
static inline EIP207_Flow_HTE_Dscr_t *
EIP207_Flow_HTE_Dscr_List_Prev_Get(
        EIP207_Flow_HTE_Dscr_t * const HTE_Dscr_p)
{
    if (HTE_Dscr_p != NULL)
    {
        EIP207_Flow_HTE_Dscr_List_t * List_p;

        List_p = &HTE_Dscr_p->List;

        return List_p->PrevDscr_p;
    }
    else
        return NULL;
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_HTE_Dscr_List_Prev_Set
 *
 * Sets the previous HTE descriptor for the provided one
 * in the descriptor list
 */
static inline void
EIP207_Flow_HTE_Dscr_List_Prev_Set(
        EIP207_Flow_HTE_Dscr_t * const HTE_Dscr_p,
        EIP207_Flow_HTE_Dscr_t * const Prev_HTE_Dscr_p)
{
    if (HTE_Dscr_p != NULL)
    {
        EIP207_Flow_HTE_Dscr_List_t * List_p;

        List_p = &HTE_Dscr_p->List;

        List_p->PrevDscr_p = Prev_HTE_Dscr_p;
    }
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_HTE_Dscr_List_Next_Get
 *
 * Gets the next HTE descriptor for the provided one
 * in the descriptor list
 */
static inline EIP207_Flow_HTE_Dscr_t *
EIP207_Flow_HTE_Dscr_List_Next_Get(
        EIP207_Flow_HTE_Dscr_t * const HTE_Dscr_p)
{
    if (HTE_Dscr_p != NULL)
    {
        EIP207_Flow_HTE_Dscr_List_t * List_p;

        List_p = &HTE_Dscr_p->List;

        return List_p->NextDscr_p;
    }
    else
        return NULL;
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_HTE_Dscr_List_Next_Set
 *
 * Sets the next HTE descriptor for the provided one
 * in the descriptor list
 */
static inline void
EIP207_Flow_HTE_Dscr_List_Next_Set(
        EIP207_Flow_HTE_Dscr_t * const HTE_Dscr_p,
        EIP207_Flow_HTE_Dscr_t * const Next_HTE_Dscr_p)
{
    if (HTE_Dscr_p)
    {
        EIP207_Flow_HTE_Dscr_List_t * List_p;

        List_p = &HTE_Dscr_p->List;

        List_p->NextDscr_p = Next_HTE_Dscr_p;
    }
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_HTE_Dscr_List_Remove
 *
 * Remove HTE descriptor from the list it's on
 */
static inline void
EIP207_Flow_HTE_Dscr_List_Remove(
        EIP207_Flow_HTE_Dscr_t * const HTE_Dscr_p)
{
    EIP207_Flow_HTE_Dscr_t * HTE_Dscr_Prev_p;
    EIP207_Flow_HTE_Dscr_t * HTE_Dscr_Next_p;

    // Update the list
    HTE_Dscr_Prev_p =
               EIP207_Flow_HTE_Dscr_List_Prev_Get(HTE_Dscr_p);

    HTE_Dscr_Next_p =
               EIP207_Flow_HTE_Dscr_List_Next_Get(HTE_Dscr_p);

    EIP207_Flow_HTE_Dscr_List_Next_Set(HTE_Dscr_Prev_p,
                                       HTE_Dscr_Next_p);

    EIP207_Flow_HTE_Dscr_List_Prev_Set(HTE_Dscr_Next_p,
                                       HTE_Dscr_Prev_p);

    // Update the HTE descriptor
    EIP207_Flow_HTE_Dscr_List_Next_Set(HTE_Dscr_p, NULL);
    EIP207_Flow_HTE_Dscr_List_Prev_Set(HTE_Dscr_p, NULL);
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_HTE_Dscr_List_Insert
 *
 * Insert HTE descriptor to the list using HTE_Dscr_Prev_p as the previous
 * descriptor
 */
static inline void
EIP207_Flow_HTE_Dscr_List_Insert(
        EIP207_Flow_HTE_Dscr_t * const HTE_Dscr_Prev_p,
        EIP207_Flow_HTE_Dscr_t * const HTE_Dscr_p)
{
    EIP207_Flow_HTE_Dscr_t * HTE_Dscr_Next_p;

    // Update the list, get the next HTE descriptor for HTE_Dscr_Prev_p
    HTE_Dscr_Next_p =
               EIP207_Flow_HTE_Dscr_List_Next_Get(HTE_Dscr_Prev_p);

    // Update the HTE_Dscr_Prev_p descriptor
    EIP207_Flow_HTE_Dscr_List_Next_Set(HTE_Dscr_Prev_p,
                                       HTE_Dscr_p);

    // Update the HTE_Dscr_Next_p descriptor
    EIP207_Flow_HTE_Dscr_List_Prev_Set(HTE_Dscr_Next_p,
                                       HTE_Dscr_p);

    // Update the HTE descriptor
    EIP207_Flow_HTE_Dscr_List_Next_Set(HTE_Dscr_p, HTE_Dscr_Next_p);
    EIP207_Flow_HTE_Dscr_List_Prev_Set(HTE_Dscr_p, HTE_Dscr_Prev_p);
}


#endif /* EIP207_FLOW_HTE_DSCR_DTL_H_ */


/* end of file eip207_flow_hte_dscr_dtl.h */
