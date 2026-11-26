/* eip207_flow_dtl.c
 *
 * Partial EIP-207 Flow Control Generic API implementation and
 * full EIP-207 Flow Control DTL API implementation
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// EIP-207 Driver Library Flow Control Generic API
#include "eip207_flow_dtl.h"

// EIP-207 Driver Library Flow Control Generic API
#include "eip207_flow_generic.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip207_flow.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // uint32_t

// Driver Framework C Run-time Library API
#include "clib.h"                       // ZEROINIT, memset, memcpy

// Driver Framework Device API
#include "device_types.h"               // Device_Handle_t

// Driver Framework DMA Resource API
#include "dmares_types.h"
#include "dmares_rw.h"

// EIP-207 Flow Control Driver Library Internal interfaces
#include "eip207_flow_level0.h"         // EIP-207 Level 0 macros
#include "eip207_flow_hte_dscr_dtl.h"   // HTE descriptor DTL
#include "eip207_flow_internal.h"

#ifndef EIP207_FLUE_RC_HP
// EIP-207 (Global Control) Record Cache (RC) interface
#include "eip207_rc.h"

// EIP-207s (Global Control) Flow Look-Up Engine Cache (FLUEC) interface
#include "eip207_fluec.h"
#endif // !EIP207_FLUE_RC_HP

// EIP-207 Firmware API
#include "firmware_eip207_api_flow_cs.h" // Classification API: Flow Control
#include "firmware_eip207_api_cs.h"      // Classification API: General


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define EIP207_FLOW_MAX_RECORDS_PER_BUCKET              3


// Flow record input data required for the flow record creation
typedef struct
{
    // Bitwise of zero or more EIP207_FLOW_FLAG_* flags, see above
    uint32_t Flags;

    // Physical (DMA/bus) address of the transform record
    EIP207_Flow_Address_t Xform_DMA_Addr;

    // Software flow record reference
    uint32_t SW_FR_Reference;

    // Transform record type, true - large, false - small
    bool fLarge;
} EIP207_Flow_FR_Data_t;

// Transform record input data required for the transform record creation
typedef struct
{
    // Transform record type, true - large, false - small
    bool fLarge;

} EIP207_Flow_TR_Data_t;

// Transform record input data required for the Transform record creation
typedef struct
{
    // Record hash ID
    const EIP207_Flow_ID_t * HashID_p;

    // Flow record input data, fill with NULL if not used
    const EIP207_Flow_FR_Data_t * FR_Data_p;

    // Transform record input data, fill with NULL if not used
    const EIP207_Flow_TR_Data_t * TR_Data_p;

    // Note: FR_Data_p and TR_Data_p cannot be both NULL!

} EIP207_Flow_Record_InputData_t;

// Full record descriptor
typedef struct
{
    EIP207_Flow_Dscr_t                  RecDscr;

    EIP207_Flow_HTE_Dscr_RecData_t      RecData;

    void *                              Reserved_p;
} EIP207_Flow_Rec_Dscr_t;


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_Entry_Lookup
 */
static void
EIP207Lib_Flow_Entry_Lookup(
        const uint32_t HashID_Word0,
        const uint32_t TableSize,
        unsigned int * const HT_Entry_ByteOffset,
        unsigned int * const DT_Entry_Index)
{
    uint32_t HashID_W0, Mask;

    // From the EIP-197 Programmer Manual:
    // The hash engine completes the hash ID calculation and
    // adds bits [(table_size+10):6] (with table_size coming from
    // the FLUE_SIZE(_VM)_f register) of the hash ID, multiplied by 64,
    // to the table base address given in the FLUE_HASHBASE(_VM)_f_* registers.

    // Calculate the entry byte offset in the HT for this record
    HashID_W0 = (HashID_Word0 & (~MASK_6_BITS));

    Mask = (1 << (TableSize + 10 + 1)) - 1;

    // Calculate the HT entry byte offset
    *HT_Entry_ByteOffset = (unsigned int)(HashID_W0 & Mask);

    // Translate the HT entry byte offset to the index in the HT
    // HT entry size is one hash bucket (16 words = 64 bytes)
    *DT_Entry_Index = (*HT_Entry_ByteOffset) >> 6; // divide by 64
}


/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_TableSize_To_EntryCount
 *
 * Convert EIP207_Flow_HashTable_Entry_Count_t to a number
 */
static inline unsigned int
EIP207Lib_Flow_TableSize_To_EntryCount(
        const unsigned int TableSize)
{
    return (1 << (TableSize + 5));
}


/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_HTE_Dscr_ByteCount_Get
 */
static inline unsigned int
EIP207Lib_Flow_HTE_Dscr_ByteCount_Get(void)
{
    return sizeof(EIP207_Flow_HTE_Dscr_t);
}


/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_HB_Record_Add
 *
 * Updates Hash ID and record offset in the hash bucket,
 * first Hash ID then record byte offset
 */
static inline void
EIP207Lib_Flow_HB_Record_Add(
        const DMAResource_Handle_t HT_DMA_Handle,
        const unsigned int HB_ByteOffset,
        const unsigned int slot,
        const uint32_t Rec_ByteOffset,
        const uint32_t RecType,
        const EIP207_Flow_ID_t * const HashID_p)
{
    unsigned int i;
    unsigned int HB_WordOffset = HB_ByteOffset >> 2;

    // Write record Hash ID words
    for (i = 0; i < EIP207_FLOW_HASH_ID_WORD_COUNT; i++)
        DMAResource_Write32(HT_DMA_Handle,
                            HB_WordOffset +
                             EIP207_FLOW_HB_HASH_ID_1_WORD_OFFSET + i +
                             EIP207_FLOW_HASH_ID_WORD_COUNT * (slot - 1),
                            HashID_p->Word32[i]);

    // Write record offset
    DMAResource_Write32(HT_DMA_Handle,
                        HB_WordOffset +
                        EIP207_FLOW_HB_REC_1_WORD_OFFSET + (slot - 1),
                        Rec_ByteOffset | RecType);

    // Perform pre-DMA for this hash bucket
    DMAResource_PreDMA(HT_DMA_Handle,
                       HB_ByteOffset,
                       EIP207_FLOW_HT_ENTRY_WORD_COUNT *
                         sizeof(uint32_t));
}


/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_HB_Record_Remove
 *
 * Updates Hash ID and record offset in the hash bucket,
 * first record byte offset then Hash ID
 */
static inline void
EIP207Lib_Flow_HB_Record_Remove(
        const DMAResource_Handle_t HT_DMA_Handle,
        const unsigned int HB_ByteOffset,
        const unsigned int slot,
        EIP207_Flow_ID_t * HashID_p)
{
    unsigned int i;
    unsigned int HB_WordOffset = HB_ByteOffset >> 2;
    EIP207_Flow_ID_t HashID;

    // Write record offset
    DMAResource_Write32(HT_DMA_Handle,
                        HB_WordOffset +
                        EIP207_FLOW_HB_REC_1_WORD_OFFSET + (slot - 1),
                        EIP207_FLOW_RECORD_DUMMY_ADDRESS);

    // Read old record Hash ID words
    for (i = 0; i < EIP207_FLOW_HASH_ID_WORD_COUNT; i++)
        HashID.Word32[i] = DMAResource_Read32(
                             HT_DMA_Handle,
                             HB_WordOffset +
                             EIP207_FLOW_HB_HASH_ID_1_WORD_OFFSET + i +
                             EIP207_FLOW_HASH_ID_WORD_COUNT * (slot - 1));

    // Write new record Hash ID words
    for (i = 0; i < EIP207_FLOW_HASH_ID_WORD_COUNT; i++)
    {
        DMAResource_Write32(HT_DMA_Handle,
                            HB_WordOffset +
                             EIP207_FLOW_HB_HASH_ID_1_WORD_OFFSET + i +
                             EIP207_FLOW_HASH_ID_WORD_COUNT * (slot - 1),
                            HashID_p->Word32[i]);

        // Store old record Hash ID words
        HashID_p->Word32[i] = HashID.Word32[i];
    }

    // Perform pre-DMA for this hash bucket
    DMAResource_PreDMA(HT_DMA_Handle,
                       HB_ByteOffset,
                       EIP207_FLOW_HT_ENTRY_WORD_COUNT *
                         sizeof(uint32_t));
}


/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_HB_BuckOffs_Update
 *
 * Update bucket offset in the hash bucket (identified by HB_ByteOffset)
 * by writing the Update_Value in there
 */
static inline void
EIP207Lib_Flow_HB_BuckOffs_Update(
        const DMAResource_Handle_t HT_DMA_Handle,
        const unsigned int HB_ByteOffset,
        const uint32_t Update_Value)
{
    unsigned int HB_WordOffset = HB_ByteOffset >> 2;

    // Write record offset
    DMAResource_Write32(HT_DMA_Handle,
                        HB_WordOffset +
                        EIP207_FLOW_HB_OVFL_BUCKET_WORD_OFFSET,
                        Update_Value);

    // Perform pre-DMA for this hash bucket
    DMAResource_PreDMA(HT_DMA_Handle,
                       HB_ByteOffset,
                       EIP207_FLOW_HT_ENTRY_WORD_COUNT *
                         sizeof(uint32_t));
}


#ifdef EIP207_FLOW_CONSISTENCY_CHECK
/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_HB_HashID_Match
 *
 * Checks if the hash bucket already contains this Hash ID
 */
static bool
EIP207Lib_Flow_HB_HashID_Match(
        const DMAResource_Handle_t HT_DMA_Handle,
        const unsigned int HB_ByteOffset,
        const uint32_t RecType,
        const EIP207_Flow_ID_t * const HashID_p)
{
    uint32_t Word32;
    bool fMatch;
    unsigned int i, j;
    unsigned int HB_WordOffset = HB_ByteOffset >> 2;

    // Read Hash ID's from the bucket
    // (this DMA resource is read-only for the FLUE device)
    for (i = 0; i < EIP207_FLOW_HTE_BKT_NOF_REC_MAX; i++)
    {
        for (j = 0; j < EIP207_FLOW_HASH_ID_WORD_COUNT; j++)
        {
            fMatch = true;

            Word32 = DMAResource_Read32(
                                 HT_DMA_Handle,
                                 HB_WordOffset +
                                  EIP207_FLOW_HB_HASH_ID_1_WORD_OFFSET +
                                    i * EIP207_FLOW_HASH_ID_WORD_COUNT + j);

            if (Word32 != HashID_p->Word32[j])
            {
                fMatch = false;
                break; // No match, skip this hash ID
            }
        } // for

        if (fMatch)
            break; // Matching Hash ID found, stop searching
    } // for

    if (fMatch)
    {
        uint32_t Match_RecType;

        // Check the record type in the record offset for Hash ID with index i
        // to confirm the match
        Word32 = DMAResource_Read32(
                             HT_DMA_Handle,
                             HB_WordOffset +
                              EIP207_FLOW_HB_REC_1_WORD_OFFSET + i);

        // Note: small transform record and large transform record
        //       fall under the same record type for this check!
        Match_RecType = Word32 & RecType;

        if ((Match_RecType == EIP207_FLOW_RECORD_FR_ADDRESS  &&
             (RecType == EIP207_FLOW_RECORD_TR_ADDRESS         ||
              RecType == EIP207_FLOW_RECORD_TR_LARGE_ADDRESS))    ||

            (RecType == EIP207_FLOW_RECORD_FR_ADDRESS        &&
             (Match_RecType == EIP207_FLOW_RECORD_TR_ADDRESS   ||
              Match_RecType == EIP207_FLOW_RECORD_TR_LARGE_ADDRESS)))
        {
            // Match not confirmed, a different record type is used
            fMatch = false;
        }
    }

    return fMatch;
}
#endif // EIP207_FLOW_CONSISTENCY_CHECK


/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_HB_Slot_Get
 *
 * Determine the free offset slot number (1,2 or 3) and claim it
 */
static inline EIP207_Flow_Error_t
EIP207Lib_Flow_HB_Slot_Get(
        uint32_t * const RecOffsMask_p,
        unsigned int * FreeSlot_p)
{
    if ((*RecOffsMask_p & EIP207_FLOW_HTE_REC_OFFSET_1) == 0)
    {
        *FreeSlot_p = 1;
        *RecOffsMask_p |= EIP207_FLOW_HTE_REC_OFFSET_1;
    }
    else if((*RecOffsMask_p & EIP207_FLOW_HTE_REC_OFFSET_2) == 0)
    {
        *FreeSlot_p = 2;
        *RecOffsMask_p |= EIP207_FLOW_HTE_REC_OFFSET_2;
    }
    else if((*RecOffsMask_p & EIP207_FLOW_HTE_REC_OFFSET_3) == 0)
    {
        *FreeSlot_p = 3;
        *RecOffsMask_p |= EIP207_FLOW_HTE_REC_OFFSET_3;
    }
    else
        // Consistency checks for the found HTE descriptor
        return EIP207_FLOW_INTERNAL_ERROR;

    return EIP207_FLOW_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_HB_Slot_Put
 *
 * Determine the used offset slot number (1,2 or 3) and release it
 */
static inline EIP207_Flow_Error_t
EIP207Lib_Flow_HB_Slot_Put(
        uint32_t * const RecOffsMask_p,
        const unsigned int Slot)
{
    switch (Slot)
    {
        case 1:
            *RecOffsMask_p &= ~EIP207_FLOW_HTE_REC_OFFSET_1;
            break;

        case 2:
            *RecOffsMask_p &= ~EIP207_FLOW_HTE_REC_OFFSET_2;
            break;

        case 3:
            *RecOffsMask_p &= ~EIP207_FLOW_HTE_REC_OFFSET_3;
            break;

        default:
            // Consistency checks for the slot to release
            return EIP207_FLOW_INTERNAL_ERROR;
    }

    return EIP207_FLOW_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_Dscr_InternalData_Set
 */
static inline void
EIP207Lib_Flow_Dscr_InternalData_Set(
        EIP207_Flow_Dscr_t * const Dscr_p)
{
    EIP207_Flow_Rec_Dscr_t * p = (EIP207_Flow_Rec_Dscr_t*)Dscr_p;

    Dscr_p->InternalData_p = &p->RecData;
}


/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_FR_Write
 */
static void
EIP207Lib_Flow_FR_Write(
        const DMAResource_Handle_t DMA_Handle,
        const EIP207_Flow_FR_Data_t * const FR_Data_p,
        const uint32_t Xform_ByteOffset,
        const uint32_t Xform_Addr,
        const uint32_t Xform_UpperAddr)
{
    unsigned int i;
    uint32_t Record_Type;

    // Write flow record with 0's
    for (i = 0; i < FIRMWARE_EIP207_CS_FRC_RECORD_WORD_COUNT; i++)
        DMAResource_Write32(DMA_Handle, i, 0);

#ifdef EIP207_FLOW_NO_TYPE_BITS_IN_FLOW_RECORD
    Record_Type = 0;
#else
    Record_Type = FR_Data_p->fLarge ?
        EIP207_FLOW_RECORD_TR_LARGE_ADDRESS :
        EIP207_FLOW_RECORD_TR_ADDRESS;
#endif

    // Write Transform Record 32-bit offset
    DMAResource_Write32(DMA_Handle,
                        FIRMWARE_EIP207_CS_FLOW_FR_XFORM_OFFS_WORD_OFFSET,
                        Xform_ByteOffset + Record_Type);

    // Write Transform Record low half of 64-bit address
    DMAResource_Write32(DMA_Handle,
                        FIRMWARE_EIP207_CS_FLOW_FR_XFORM_ADDR_WORD_OFFSET,
                        Xform_Addr + Record_Type);

    // Write Transform Record high half of 64-bit address
    DMAResource_Write32(DMA_Handle,
                        FIRMWARE_EIP207_CS_FLOW_FR_XFORM_ADDR_HI_WORD_OFFSET,
                        Xform_UpperAddr);

    // Not supported yet
    // Write ARC4 State Record physical address
    //DMAResource_Write32(FR_Dscr_p->Data.DMA_Handle,
    //                    FIRMWARE_EIP207_CS_FLOW_FR_ARC4_ADDR_WORD_OFFSET,
    //                    EIP207_FLOW_RECORD_DUMMY_ADDRESS);

    // Write Software Flow Record Reference
    // NOTE: this is a 32-bit value!
    DMAResource_Write32(DMA_Handle,
                        FIRMWARE_EIP207_CS_FLOW_FR_SW_ADDR_WORD_OFFSET,
                        FR_Data_p->SW_FR_Reference);

    // Write the Flags field
    DMAResource_Write32(DMA_Handle,
                        FIRMWARE_EIP207_CS_FLOW_FR_FLAGS_WORD_OFFSET,
                        FR_Data_p->Flags);

    // Perform pre-DMA for the entire flow record
    DMAResource_PreDMA(DMA_Handle, 0, 0);
}


/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_DTL_Record_Add
 */
static EIP207_Flow_Error_t
EIP207Lib_Flow_DTL_Record_Add(
        volatile EIP207_Flow_HT_Params_t * const HT_Params_p,
        EIP207_Flow_Dscr_t * const Rec_Dscr_p,
        const EIP207_Flow_Record_InputData_t * const RecData_p)
{
    uint32_t Rec_ByteOffset;
    unsigned int slot = 0;
    EIP207_Flow_HTE_Dscr_t * HTE_Dscr_Updated_p = NULL;

#ifdef EIP207_FLOW_CONSISTENCY_CHECK
    // Consistency check for the provided record descriptor
    if (Rec_Dscr_p->DMA_Addr.Addr == EIP207_FLOW_RECORD_DUMMY_ADDRESS)
        return EIP207_FLOW_INTERNAL_ERROR;
#endif // EIP207_FLOW_CONSISTENCY_CHECK

    // Add the record to the Hash Table
    {
        EIP207_Flow_HTE_Dscr_t * HTE_Dscr_p;
        unsigned int Entry_Index, HTE_ByteOffset, HT_TableSize;
        uint32_t Record_Type = EIP207_FLOW_RECORD_DUMMY_ADDRESS;
        DMAResource_Handle_t HT_DMA_Handle = HT_Params_p->HT_DMA_Handle;

        // Transform record
        if (RecData_p->TR_Data_p != NULL)
            Record_Type = RecData_p->TR_Data_p->fLarge ?
                            EIP207_FLOW_RECORD_TR_LARGE_ADDRESS :
                                       EIP207_FLOW_RECORD_TR_ADDRESS;
        // Flow record
        else if (RecData_p->FR_Data_p != NULL)
            Record_Type = EIP207_FLOW_RECORD_FR_ADDRESS;
        else
            return EIP207_FLOW_INTERNAL_ERROR; // Unknown record, error

        HT_TableSize = HT_Params_p->HT_TableSize;

        HTE_ByteOffset  = 0;
        Entry_Index     = 0;

        EIP207Lib_Flow_Entry_Lookup(
                RecData_p->HashID_p->Word32[0], // Word 0 is used for lookup
                HT_TableSize,
                &HTE_ByteOffset,
                &Entry_Index);

#ifdef EIP207_FLOW_CONSISTENCY_CHECK
        {
            unsigned int HT_EntryCount =
                    EIP207Lib_Flow_TableSize_To_EntryCount(HT_TableSize);

            if (Entry_Index >= HT_EntryCount)
                return EIP207_FLOW_INTERNAL_ERROR;
        }
#endif

        // Calculate the record byte offset,
        // HT_Params_p->BaseAddr.Addr is the base
        // address of the DMA bank where the record
        // must have been allocated
        Rec_ByteOffset = Rec_Dscr_p->DMA_Addr.Addr - HT_Params_p->BaseAddr.Addr;

        // Convert the DT entry index to the HTE descriptor pointer
        {
            void * p;
            unsigned char * DT_p = (unsigned char*)HT_Params_p->DT_p;

            // Read the entry value at the calculated offset from the DT
            p = DT_p + Entry_Index * EIP207Lib_Flow_HTE_Dscr_ByteCount_Get();

            HTE_Dscr_p = (EIP207_Flow_HTE_Dscr_t*)p;
        }

#ifdef EIP207_FLOW_CONSISTENCY_CHECK
        // Consistency checks for the found HTE descriptor
        if (HTE_Dscr_p->Bucket_ByteOffset != (uint32_t)HTE_ByteOffset)
            return EIP207_FLOW_INTERNAL_ERROR;

        // Check that this HTE is not an overflow one
        if (HTE_Dscr_p->fOverflowBucket)
            return EIP207_FLOW_INTERNAL_ERROR;
#endif // EIP207_FLOW_CONSISTENCY_CHECK

        do // Walk the bucket chain, look for bucket where record can be added
        {
#ifdef EIP207_FLOW_CONSISTENCY_CHECK
            if (HTE_Dscr_p->RecordCount > EIP207_FLOW_MAX_RECORDS_PER_BUCKET)
                return EIP207_FLOW_INTERNAL_ERROR;

            // Check if the hash bucket does not contain this Hash ID already
            if (HTE_Dscr_Updated_p == NULL && EIP207Lib_Flow_HB_HashID_Match(
                     HT_DMA_Handle,
                     HTE_Dscr_p->Bucket_ByteOffset,
                     Record_Type,
                     RecData_p->HashID_p))
                return EIP207_FLOW_INTERNAL_ERROR;
#endif // EIP207_FLOW_CONSISTENCY_CHECK

            // Check if the found hash bucket can be used to add
            // the record or a next bucket in the chain must be used
            if (HTE_Dscr_p->RecordCount < EIP207_FLOW_MAX_RECORDS_PER_BUCKET)
            {
                uint32_t TempMask = HTE_Dscr_p->UsedRecOffsMask;
                uint32_t TempSlot = slot;

                // Use the found bucket to add the record

                // Determine the offset slot (1,2 or 3) where the record offset
                // and Hash ID can be stored
                {
                    EIP207_Flow_Error_t Flow_Rc =
                            EIP207Lib_Flow_HB_Slot_Get(
                                    &TempMask,
                                    &TempSlot);

                    if(Flow_Rc != EIP207_FLOW_NO_ERROR)
                        return Flow_Rc;
                }

#ifdef EIP207_FLOW_CONSISTENCY_CHECK
                // Check if the record offset in the HTE slot is a dummy pointer
                {
                    // Read record offset
                    // (this DMA resource is read-only for the FLUE device)
                    uint32_t Record_ByteOffset =
                                DMAResource_Read32(
                                         HT_DMA_Handle,
                                         (HTE_Dscr_p->Bucket_ByteOffset >> 2) +
                                           EIP207_FLOW_HB_REC_1_WORD_OFFSET +
                                            (TempSlot - 1));

                    if ((Record_ByteOffset &
                         EIP207_FLOW_RECORD_ADDRESS_TYPE_BITS) !=
                            EIP207_FLOW_RECORD_DUMMY_ADDRESS)
                        return EIP207_FLOW_INTERNAL_ERROR;
                }
#endif // EIP207_FLOW_CONSISTENCY_CHECK

                // Add the record only if it has not been done already
                if (HTE_Dscr_Updated_p == NULL)
                {
                    HTE_Dscr_p->UsedRecOffsMask = TempMask;
                    slot = TempSlot;

                    // If flow record is added then write it
                    if (RecData_p->FR_Data_p != NULL)
                        EIP207Lib_Flow_FR_Write(
                                 Rec_Dscr_p->DMA_Handle,
                                 RecData_p->FR_Data_p,
                                 RecData_p->FR_Data_p->Xform_DMA_Addr.Addr -
                                 HT_Params_p->BaseAddr.Addr,
                                 RecData_p->FR_Data_p->Xform_DMA_Addr.Addr,
                                 RecData_p->FR_Data_p->Xform_DMA_Addr.UpperAddr);

                    // Update the found hash bucket with the
                    // record Hash ID and offset

                    // CDS point: after this operation is done the FLUE
                    //            hardware can find the transform record!
                    EIP207Lib_Flow_HB_Record_Add(HT_DMA_Handle,
                                             HTE_Dscr_p->Bucket_ByteOffset,
                                             slot,
                                             Rec_ByteOffset,
                                             Record_Type,
                                             RecData_p->HashID_p);

                    // Increase the HTE descriptor record count
                    // for the added record
                    HTE_Dscr_p->RecordCount++;
                }

                // The found HTE is updated, record is added
                if (HTE_Dscr_Updated_p == NULL)
                    HTE_Dscr_Updated_p = HTE_Dscr_p;

#ifdef EIP207_FLOW_CONSISTENCY_CHECK
                // Get the next HTE descriptor in the chain
                HTE_Dscr_p = EIP207_Flow_HTE_Dscr_List_Next_Get(HTE_Dscr_p);
                continue; // Validate the entire chain
#else
                break;
#endif
            }
            else // Found bucket record count is max possible
            {
                EIP207_Flow_HTE_Dscr_t * HTE_Dscr_Next_p;

                // Use a next overflow bucket to add the record, find a bucket
                // in the chain with a free slot for the new record

                // Get the next HTE descriptor in the chain
                HTE_Dscr_Next_p =
                           EIP207_Flow_HTE_Dscr_List_Next_Get(HTE_Dscr_p);

                // Check if we have one and
                // that the record has not been added already
                if (HTE_Dscr_Next_p == NULL && HTE_Dscr_Updated_p == NULL)
                {
                    // No chain is present for HTE_Dscr_p.
                    // Link a new HTE descriptor from the free list to
                    // the found HTE descriptor

                    EIP207_Flow_HTE_Dscr_t * FreeList_Head_p;

#ifdef EIP207_FLOW_CONSISTENCY_CHECK
                    // Check if the overflow bucket offset is a dummy pointer
                    {
                        // Read the overflow bucket offset
                        // (this DMA resource is read-only for the FLUE device)
                        uint32_t Bucket_ByteOffset =
                                DMAResource_Read32(
                                    HT_DMA_Handle,
                                    (HTE_Dscr_p->Bucket_ByteOffset >> 2) +
                                      EIP207_FLOW_HB_OVFL_BUCKET_WORD_OFFSET);

                        if (Bucket_ByteOffset !=
                                EIP207_FLOW_RECORD_DUMMY_ADDRESS)
                            return EIP207_FLOW_INTERNAL_ERROR;
                    }
#endif // EIP207_FLOW_CONSISTENCY_CHECK

                    FreeList_Head_p =
                        (EIP207_Flow_HTE_Dscr_t*)HT_Params_p->FreeList_Head_p;

                    // Check if the record can be added
                    if (FreeList_Head_p == NULL)
                        // Out of descriptors for overflow buckets
                        return EIP207_FLOW_OUT_OF_MEMORY_ERROR;

                    slot = 1; // start with slot 1 in the new bucket

                    // Get a free HTE descriptor from the free list head
                    HTE_Dscr_Next_p =
                          EIP207_Flow_HTE_Dscr_List_Next_Get(FreeList_Head_p);

                    // Check if the last descriptor is present in the free list
                    if (HTE_Dscr_Next_p == NULL)
                    {
                        HTE_Dscr_Next_p = FreeList_Head_p;
                        HT_Params_p->FreeList_Head_p = NULL; // List is empty
                    }
                    else
                    {
                        // Remove the got HTE descriptor from the free list
                        EIP207_Flow_HTE_Dscr_List_Remove(HTE_Dscr_Next_p);
                    }

                    // Add the got HTE descriptor to the overflow chain,
                    // we know that HTE_Dscr_p is the last descriptor
                    // in the chain
                    EIP207_Flow_HTE_Dscr_List_Next_Set(HTE_Dscr_p,
                                                       HTE_Dscr_Next_p);
                    EIP207_Flow_HTE_Dscr_List_Prev_Set(HTE_Dscr_Next_p,
                                                       HTE_Dscr_p);

                    // We add first record to the bucket which HTE descriptor
                    // just fresh-taken from the free list
                    HTE_Dscr_Next_p->UsedRecOffsMask =
                                       EIP207_FLOW_HTE_REC_OFFSET_1;

                    // If flow record is added then write it
                    if (RecData_p->FR_Data_p != NULL)
                        EIP207Lib_Flow_FR_Write(
                                 Rec_Dscr_p->DMA_Handle,
                                 RecData_p->FR_Data_p,
                                 RecData_p->FR_Data_p->Xform_DMA_Addr.Addr -
                                 HT_Params_p->BaseAddr.Addr,
                                 RecData_p->FR_Data_p->Xform_DMA_Addr.Addr,
                                 RecData_p->FR_Data_p->Xform_DMA_Addr.UpperAddr);

                    // Update the overflow bucket with hash ID and record offset
                    EIP207Lib_Flow_HB_Record_Add(HT_DMA_Handle,
                                             HTE_Dscr_Next_p->Bucket_ByteOffset,
                                             slot,
                                             Rec_ByteOffset,
                                             Record_Type,
                                             RecData_p->HashID_p);

                    // CDS point: after the next write32() operation is done
                    //            the FLUE HW can find the overflow bucket!

                    // Update bucket offset to the found bucket
                    // Note: bucket offset can use any address (pointer) type
                    //       but not NULL!
                    EIP207Lib_Flow_HB_BuckOffs_Update(
                                           HT_DMA_Handle,
                                           HTE_Dscr_p->Bucket_ByteOffset,
                                           HTE_Dscr_Next_p->Bucket_ByteOffset |
                                             EIP207_FLOW_RECORD_TR_ADDRESS);

                    // Increment record count for the added record
                    HTE_Dscr_Next_p->RecordCount++;

                    // Bucket is updated, record is added
                    if (HTE_Dscr_Updated_p == NULL)
                        HTE_Dscr_Updated_p = HTE_Dscr_Next_p;

#ifdef EIP207_FLOW_CONSISTENCY_CHECK
                    HTE_Dscr_p = HTE_Dscr_Next_p;
                    continue; // Validate the entire chain
#else
                    break;
#endif
                }
                else // Overflow chain is present for the found HTE_Dscr_p
                {
                    // HTE_Dscr_p - last found HTE descriptor in the chain
                    // HTE_Dscr_Next_p - next HTE descriptor for HTE_Dscr_p

#ifdef EIP207_FLOW_CONSISTENCY_CHECK
                    if (HTE_Dscr_Next_p != NULL)
                    {
                        // Consistency checks: next HTE descriptor must be on
                        //                     the overflow chain
                        if (!HTE_Dscr_Next_p->fOverflowBucket)
                            return EIP207_FLOW_INTERNAL_ERROR;

                        // Check if the overflow bucket offset is
                        // a dummy pointer
                        {
                            // Read the overflow bucket offset
                            // from the previous bucket
                            uint32_t Bucket_ByteOffset =
                                DMAResource_Read32(
                                       HT_DMA_Handle,
                                       (HTE_Dscr_p->Bucket_ByteOffset >> 2) +
                                       EIP207_FLOW_HB_OVFL_BUCKET_WORD_OFFSET);

                            if (Bucket_ByteOffset !=
                                (HTE_Dscr_Next_p->Bucket_ByteOffset|
                                EIP207_FLOW_RECORD_TR_ADDRESS))
                                return EIP207_FLOW_INTERNAL_ERROR;
                        }
                    }
#endif // EIP207_FLOW_CONSISTENCY_CHECK

                    HTE_Dscr_p = HTE_Dscr_Next_p;
                    continue;

                } // overflow bucket is found

            } // "bucket full" handling is done

        } while(HTE_Dscr_p != NULL);

    } // Record is added to the Hash Table

    // Fill in record descriptor internal data
    {
        EIP207_Flow_HTE_Dscr_RecData_t * Rec_DscrData_p;

#ifdef EIP207_FLOW_CONSISTENCY_CHECK
        // Check if the free slot number was found and
        // the HTE descriptor to update was found
        if (slot == 0 || HTE_Dscr_Updated_p == NULL)
            return EIP207_FLOW_INTERNAL_ERROR;
#endif

        EIP207Lib_Flow_Dscr_InternalData_Set(Rec_Dscr_p);
        Rec_DscrData_p =
              (EIP207_Flow_HTE_Dscr_RecData_t*)Rec_Dscr_p->InternalData_p;

        Rec_DscrData_p->Slot         = slot;
        Rec_DscrData_p->HTE_Dscr_p   = HTE_Dscr_Updated_p;

        if (RecData_p->TR_Data_p != NULL)
        {
            if (RecData_p->TR_Data_p->fLarge)
                Rec_DscrData_p->Type = EIP207_FLOW_REC_TRANSFORM_LARGE;
            else
                Rec_DscrData_p->Type = EIP207_FLOW_REC_TRANSFORM_SMALL;
        }
        else if (RecData_p->FR_Data_p != NULL)
            Rec_DscrData_p->Type = EIP207_FLOW_REC_FLOW;
        else
            return EIP207_FLOW_INTERNAL_ERROR;
    }

    return EIP207_FLOW_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_DTL_Record_Remove
 */
static EIP207_Flow_Error_t
EIP207Lib_Flow_DTL_Record_Remove(
        const Device_Handle_t Device,
        const unsigned int HashTableId,
        volatile EIP207_Flow_HT_Params_t * const HT_Params_p,
        EIP207_Flow_Dscr_t * const Rec_Dscr_p)
{
    EIP207_Flow_ID_t HashID;
    DMAResource_Handle_t HT_DMA_Handle;
    EIP207_Flow_HTE_Dscr_t * HTE_Dscr_p;
    EIP207_Flow_HTE_Dscr_RecData_t * Rec_DscrData_p =
               (EIP207_Flow_HTE_Dscr_RecData_t*)Rec_Dscr_p->InternalData_p;

#ifdef EIP207_FLOW_CONSISTENCY_CHECK
    // Consistency check for the provided TR descriptor
    if (Rec_Dscr_p->DMA_Addr.Addr == EIP207_FLOW_RECORD_DUMMY_ADDRESS)
        return EIP207_FLOW_INTERNAL_ERROR;

    if (Rec_DscrData_p == NULL)
        return EIP207_FLOW_INTERNAL_ERROR;
#endif // EIP207_FLOW_CONSISTENCY_CHECK

    // get the HTE descriptor for the record to remove
    HTE_Dscr_p = Rec_DscrData_p->HTE_Dscr_p;

    HT_DMA_Handle = HT_Params_p->HT_DMA_Handle;

    ZEROINIT(HashID);

#ifdef EIP207_FLOW_CONSISTENCY_CHECK
    // Check record count is sane
    if (HTE_Dscr_p->RecordCount > EIP207_FLOW_MAX_RECORDS_PER_BUCKET ||
        HTE_Dscr_p->RecordCount == 0)
        return EIP207_FLOW_INTERNAL_ERROR;

    // Check the record descriptor slot number
    if (Rec_DscrData_p->Slot < 1 || Rec_DscrData_p->Slot > 3)
        return EIP207_FLOW_INTERNAL_ERROR; // Incorrect record slot number

    // Check the record offset calculated from the record descriptor
    // matches the one on the bucket
    {
        uint32_t Rec_ByteOffset2;
        uint32_t Rec_ByteOffset1 =
                DMAResource_Read32(HT_DMA_Handle,
                                   (HTE_Dscr_p->Bucket_ByteOffset >> 2) +
                                     EIP207_FLOW_HB_REC_1_WORD_OFFSET +
                                       (Rec_DscrData_p->Slot - 1));

        // Clear the bits that specify the record type
        Rec_ByteOffset1 &= (~EIP207_FLOW_RECORD_ADDRESS_TYPE_BITS);

        // Calculate the record byte offset,
        // HT_Params_p->BaseAddr.Addr is the base
        // address of the DMA bank where the record
        // must have been allocated
        Rec_ByteOffset2 = Rec_Dscr_p->DMA_Addr.Addr -
                                            HT_Params_p->BaseAddr.Addr;

        if (Rec_ByteOffset1 != Rec_ByteOffset2)
            return EIP207_FLOW_INTERNAL_ERROR; // Incorrect record offset
    }
#endif // EIP207_FLOW_CONSISTENCY_CHECK

    if (HTE_Dscr_p->RecordCount > 1)
    {
        // Remove the non-last record from the bucket

        EIP207_Flow_Error_t Flow_Rc;

        // CDS point: record is removed when the next function returns!

        // Update the record offset in the bucket.
        // Update the Hash ID (optional)
        EIP207Lib_Flow_HB_Record_Remove(HT_DMA_Handle,
                                        HTE_Dscr_p->Bucket_ByteOffset,
                                        Rec_DscrData_p->Slot,
                                        &HashID);

        // Mark this record slot as free in the HTE descriptor
        Flow_Rc = EIP207Lib_Flow_HB_Slot_Put(&HTE_Dscr_p->UsedRecOffsMask,
                                             Rec_DscrData_p->Slot);
        if (Flow_Rc != EIP207_FLOW_NO_ERROR)
            return Flow_Rc;

        // Decrease record count in the HTE descriptor
        HTE_Dscr_p->RecordCount--;
    }
    else // record count = 1, so when removing the record also remove the bucket
    {
        // Remove the last record from the removed bucket.
        // Also remove the bucket from the chain first.

        if (HTE_Dscr_p->fOverflowBucket)
        {
            // Remove the last record from the overflow bucket

            EIP207_Flow_HTE_Dscr_t * FreeList_Head_p;

            // Get the neighboring HTE descriptors in the chain
            EIP207_Flow_HTE_Dscr_t * HTE_Dscr_Prev_p =
                           EIP207_Flow_HTE_Dscr_List_Prev_Get(HTE_Dscr_p);
            EIP207_Flow_HTE_Dscr_t * HTE_Dscr_Next_p =
                           EIP207_Flow_HTE_Dscr_List_Next_Get(HTE_Dscr_p);

#ifdef EIP207_FLOW_CONSISTENCY_CHECK
            // Check if the previous HTE descriptor is not NULL,
            // it may not be NULL for the overflow bucket
            if (HTE_Dscr_Prev_p == NULL)
                return EIP207_FLOW_INTERNAL_ERROR;
#endif

            // CDS point: bucket (and record) is removed when
            //            the next function returns and the wait loop is done!

            // Remove HTE_Dscr_p bucket from the chain,
            // make the HTE_Dscr_Prev_p bucket refer to HTE_Dscr_Next_p bucket
            // Note: bucket offset can use any address (pointer) type
            //       but not NULL!
            EIP207Lib_Flow_HB_BuckOffs_Update(
                                     HT_DMA_Handle,
                                     HTE_Dscr_Prev_p->Bucket_ByteOffset,
                                     HTE_Dscr_Next_p ? // the last in chain?
                                         (HTE_Dscr_Next_p->Bucket_ByteOffset |
                                             EIP207_FLOW_RECORD_TR_ADDRESS) :
                                          EIP207_FLOW_RECORD_DUMMY_ADDRESS);

            // Wait loop: this is to wait for the EIP-207 packet classification
            //            engine to complete processing using this bucket
            //            before it can be re-used for another record lookup
            {
                unsigned int LoopCount, Value = 1;

                for (LoopCount = 0;
                     LoopCount < EIP207_FLOW_RECORD_REMOVE_WAIT_COUNT;
                     LoopCount++)
                {
                    // Do some work in the loop
                    if (LoopCount & BIT_0)
                        Value <<= 1;
                    else
                        Value >>= 1;
                }
            } // HB remove wait loop is done!

            // Remove record from the HTE_Dscr_p bucket
            // which is not in the chain anymore
            EIP207Lib_Flow_HB_Record_Remove(HT_DMA_Handle,
                                            HTE_Dscr_p->Bucket_ByteOffset,
                                            Rec_DscrData_p->Slot,
                                            &HashID);

            // Update the bucket offset in the removed from the chain
            // HTE_Dscr_p bucket
            EIP207Lib_Flow_HB_BuckOffs_Update(
                                     HT_DMA_Handle,
                                     HTE_Dscr_p->Bucket_ByteOffset,
                                     EIP207_FLOW_RECORD_DUMMY_ADDRESS);

            // Remove HTE descriptor from the chain
            EIP207_Flow_HTE_Dscr_List_Remove(HTE_Dscr_p);

            // Add the HTE descriptor to the free list
            FreeList_Head_p =
                    (EIP207_Flow_HTE_Dscr_t*)HT_Params_p->FreeList_Head_p;

            // Insert the HTE descriptor at the free list head
            if (FreeList_Head_p != NULL)
                EIP207_Flow_HTE_Dscr_List_Insert(FreeList_Head_p, HTE_Dscr_p);
        }
        else
        {
            // Remove the last record from the HT bucket
            // Note: the chain may still be present for this HTE_Dscr_p bucket

            // CDS point: record is removed when the next function returns!

            // Update the record offset in the bucket.
            // Update the Hash ID (optional)
            EIP207Lib_Flow_HB_Record_Remove(HT_DMA_Handle,
                                            HTE_Dscr_p->Bucket_ByteOffset,
                                            Rec_DscrData_p->Slot,
                                            &HashID);

            // Wait loop: this is to wait for the EIP-207 packet classification
            //            engine to complete processing using this bucket
            //            before it can be re-used for another record lookup
            {
                unsigned int LoopCount, Value = 1;

                for (LoopCount = 0;
                     LoopCount < EIP207_FLOW_RECORD_REMOVE_WAIT_COUNT;
                     LoopCount++)
                {
                    // Do some work in the loop
                    if (LoopCount & BIT_0)
                        Value <<= 1;
                    else
                        Value >>= 1;
                }
            } // HB remove wait loop is done!

            // Mark this record slot as free in the HTE descriptor
            EIP207Lib_Flow_HB_Slot_Put(&HTE_Dscr_p->UsedRecOffsMask,
                                       Rec_DscrData_p->Slot);
        }

        // Update the HTE_Dscr_p record offset mask and record count
        HTE_Dscr_p->UsedRecOffsMask = 0;
        HTE_Dscr_p->RecordCount = 0;
    }

#ifndef EIP207_FLUE_RC_HP
    // Invalidate lookup result in the FLUEC
    EIP207_FLUEC_Invalidate(Device,
                            (uint8_t)HashTableId,
                            HashID.Word32[0],
                            HashID.Word32[1],
                            HashID.Word32[2],
                            HashID.Word32[3]);
#else
    IDENTIFIER_NOT_USED(Device);
    IDENTIFIER_NOT_USED(HashTableId);
#endif

    // Invalidate the internal data of the DTL transform record descriptor
    Rec_DscrData_p->Slot         = 0;
    Rec_DscrData_p->HTE_Dscr_p   = NULL;
    Rec_DscrData_p->Type         = EIP207_FLOW_REC_INVALID;

    return EIP207_FLOW_NO_ERROR;
}

#ifdef EIP207_FLOW_STRICT_ARGS
/*----------------------------------------------------------------------------
 * EIP207Lib_Flow_Is32bitAddressable
 */
static bool
EIP207Lib_Flow_Is32bitAddressable(
        const uint32_t BaseAddrLo,
        const uint32_t BaseAddrHi,
        const uint32_t AddrLo,
        const uint32_t AddrHi)
{
    if(BaseAddrHi == AddrHi)
    {
        if(BaseAddrLo > AddrLo)
            return false;
    }
    else if (BaseAddrHi < AddrHi && (BaseAddrHi == AddrHi + 1))
    {
        if(BaseAddrLo <= AddrLo)
            return false;
    }
    else
        return false;

    return true;
}
#endif // EIP207_FLOW_STRICT_ARGS


/*****************************************************************************
 * Generic API functions implemented in DTL-specific way
 */

/*----------------------------------------------------------------------------
 * EIP207_Flow_HTE_Dscr_ByteCount_Get
 */
unsigned int
EIP207_Flow_HTE_Dscr_ByteCount_Get(void)
{
    return EIP207Lib_Flow_HTE_Dscr_ByteCount_Get();
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_FR_Dscr_ByteCount_Get
 */
unsigned int
EIP207_Flow_FR_Dscr_ByteCount_Get(void)
{
    return sizeof(EIP207_Flow_Rec_Dscr_t);
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_TR_Dscr_ByteCount_Get
 */
unsigned int
EIP207_Flow_TR_Dscr_ByteCount_Get(void)
{
    return sizeof(EIP207_Flow_Rec_Dscr_t);
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_Record_Dummy_Addr_Get
 */
unsigned int
EIP207_Flow_Record_Dummy_Addr_Get(void)
{
    return EIP207_FLOW_RECORD_DUMMY_ADDRESS;
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_HashTable_Install
 */
EIP207_Flow_Error_t
EIP207_Flow_HashTable_Install(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        const EIP207_Flow_HT_t * HT_p,
        bool fLookupCached,
        bool fReset)
{
    unsigned int i, EntryCount;
    Device_Handle_t Device;
    volatile EIP207_Flow_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP207_FLOW_CHECK_POINTER(IOArea_p);
    EIP207_FLOW_CHECK_POINTER(HT_p->HT_DMA_Address_p);
    EIP207_FLOW_CHECK_POINTER(HT_p->DT_p);
    EIP207_FLOW_CHECK_INT_ATMOST(HashTableId + 1,
                                 EIP207_FLOW_MAX_NOF_FLOW_HASH_TABLES_TO_USE);

    EntryCount = (unsigned int)HT_p->HT_TableSize;
    EIP207_FLOW_CHECK_INT_ATMOST(EntryCount,
                                 EIP207_FLOW_HASH_TABLE_ENTRIES_MAX);

    // Convert EIP207_Flow_HashTable_Entry_Count_t to a number
    EntryCount = EIP207Lib_Flow_TableSize_To_EntryCount(EntryCount);
    EIP207_FLOW_CHECK_INT_ATMOST(EntryCount, HT_p->DT_EntryCount);

    // Check Flow Hash Table address alignment
    if (HT_p->HT_DMA_Address_p->Addr == EIP207_FLOW_RECORD_DUMMY_ADDRESS)
        return EIP207_FLOW_ARGUMENT_ERROR;

    // Save the flag that indicates whether the FLUEC must be used
    // Note: FLUEC is enabled and configured via the Global Control interface
    TrueIOArea_p->HT_Params[HashTableId].fLookupCached = fLookupCached;

    // Save the host address of the Flow Descriptor Table and its size
    TrueIOArea_p->HT_Params[HashTableId].DT_p          = HT_p->DT_p;
    TrueIOArea_p->HT_Params[HashTableId].HT_TableSize  =
                                            (unsigned int)HT_p->HT_TableSize;

    // Save the DMA resource handle of the Flow Hash Table
    TrueIOArea_p->HT_Params[HashTableId].HT_DMA_Handle =
                                                    HT_p->HT_DMA_Handle;

    Device = TrueIOArea_p->Device;

    // Initialize the HT and the DT free list with initial values
    if (fReset)
    {
        EIP207_Flow_HTE_Dscr_t * HTE_Dscr_p =
                                        (EIP207_Flow_HTE_Dscr_t*)HT_p->DT_p;

        // Initialize the Descriptor Table with initial values
        memset(HT_p->DT_p,
               0,
               HT_p->DT_EntryCount * sizeof(EIP207_Flow_HTE_Dscr_t));

        // Number of HTE descriptors is equal to the number of hash buckets
        // in the HT plus the number of overflow hash buckets.

        for (i = 0; i < HT_p->DT_EntryCount; i++)
        {
            unsigned int j;

            // Initialize with dummy address all the words (j)
            // in HTE descriptor i
            for (j = 0; j < EIP207_FLOW_HT_ENTRY_WORD_COUNT; j++)
                DMAResource_Write32(HT_p->HT_DMA_Handle,
                                    i * EIP207_FLOW_HT_ENTRY_WORD_COUNT + j,
                                    EIP207_FLOW_RECORD_DUMMY_ADDRESS);

            // Add only the HTE descriptors for the overflow hash buckets
            // to the free list.
            if (i >= EntryCount)
            {
                // Descriptor Table free list initialization,
                // all the HTE descriptors are added to the free list

                // First overflow HTE descriptor in DT?
                if (i == EntryCount)
                {
                    EIP207_Flow_HTE_Dscr_List_Prev_Set(HTE_Dscr_p, NULL);
                    EIP207_Flow_HTE_Dscr_List_Next_Set(HTE_Dscr_p,
                                                       HTE_Dscr_p + 1);

                    // Set the free list head
                    TrueIOArea_p->HT_Params[HashTableId].FreeList_Head_p =
                                                                    HTE_Dscr_p;
                }

                // Last overflow HTE descriptor in DT?
                if (i == HT_p->DT_EntryCount - 1)
                {
                    // First and last overflow HTE descriptor in DT?
                    if (i != EntryCount)
                        EIP207_Flow_HTE_Dscr_List_Prev_Set(HTE_Dscr_p,
                                                           HTE_Dscr_p - 1);
                    EIP207_Flow_HTE_Dscr_List_Next_Set(HTE_Dscr_p, NULL);
                }

                // Non-first and non-last overflow HTE descriptor in DT?
                if (i != EntryCount && i != (HT_p->DT_EntryCount - 1))
                {
                    EIP207_Flow_HTE_Dscr_List_Prev_Set(HTE_Dscr_p,
                                                       HTE_Dscr_p - 1);
                    EIP207_Flow_HTE_Dscr_List_Next_Set(HTE_Dscr_p,
                                                       HTE_Dscr_p + 1);
                }

                // Mark overflow hash buckets
                HTE_Dscr_p->fOverflowBucket = true;
            }
            else
                // Mark non-overflow hash buckets
                HTE_Dscr_p->fOverflowBucket = false;

            // Set hash bucket byte offset
            HTE_Dscr_p->Bucket_ByteOffset = i *
                                        EIP207_FLOW_HT_ENTRY_WORD_COUNT *
                                            sizeof(uint32_t);

            HTE_Dscr_p++; // Next HTE descriptor in the DT
        } // for

        // Perform pre-DMA for the entire HT including the overflow buckets,
        // both are expected to be in one linear contiguous DMA-safe buffer
        DMAResource_PreDMA(HT_p->HT_DMA_Handle, 0, 0);
    }

    // Install the FHT
    EIP207_FLUE_HASHBASE_LO_WR(Device,
                               HashTableId,
                               HT_p->HT_DMA_Address_p->Addr);
    EIP207_FLUE_HASHBASE_HI_WR(Device,
                               HashTableId,
                               HT_p->HT_DMA_Address_p->UpperAddr);
    EIP207_FLUE_SIZE_UPDATE(Device,
                            HashTableId,
                            (uint8_t)HT_p->HT_TableSize);

#ifdef EIP207_FLOW_DEBUG_FSM
    if (fReset)
    {
        EIP207_Flow_Error_t rv;
        uint32_t State = TrueIOArea_p->State;

        // Transit to a new state
        rv = EIP207_Flow_State_Set(
                (EIP207_Flow_State_t* const)&State,
                EIP207_FLOW_STATE_ENABLED);

        TrueIOArea_p->State = State;

        if (rv != EIP207_FLOW_NO_ERROR)
            return EIP207_FLOW_ILLEGAL_IN_STATE;
    }
#endif // EIP207_FLOW_DEBUG_FSM

    return EIP207_FLOW_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_FR_Add

 */
EIP207_Flow_Error_t
EIP207_Flow_FR_Add(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        EIP207_Flow_FR_Dscr_t * const FR_Dscr_p,
        const EIP207_Flow_FR_InputData_t * const FlowInData_p)
{
    volatile EIP207_Flow_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);
    EIP207_Flow_Record_InputData_t Rec_Data;
    EIP207_Flow_FR_Data_t FR_Data;
    EIP207_Flow_Error_t Flow_Rc;

    EIP207_FLOW_CHECK_POINTER(IOArea_p);
    EIP207_FLOW_CHECK_INT_ATMOST(HashTableId + 1,
                                 EIP207_FLOW_MAX_NOF_FLOW_HASH_TABLES_TO_USE);
    EIP207_FLOW_CHECK_POINTER(FR_Dscr_p);
    EIP207_FLOW_CHECK_POINTER(FlowInData_p);

#ifdef EIP207_FLOW_STRICT_ARGS
    // Provided flow record physical DMA address must be addressable by
    // a 32-bit offset to the Base Address installed via
    // the EIP207_Flow_RC_BaseAddr_Set() function
    if(!EIP207Lib_Flow_Is32bitAddressable(
            TrueIOArea_p->HT_Params[HashTableId].BaseAddr.Addr,
            TrueIOArea_p->HT_Params[HashTableId].BaseAddr.UpperAddr,
            FR_Dscr_p->DMA_Addr.Addr,
            FR_Dscr_p->DMA_Addr.UpperAddr))
        return EIP207_FLOW_ARGUMENT_ERROR;

    // Provided transform record physical DMA address must be addressable by
    // a 32-bit offset to the Base Address installed via
    // the EIP207_Flow_RC_BaseAddr_Set() function
    if(FlowInData_p->Xform_DMA_Addr.Addr != EIP207_FLOW_RECORD_DUMMY_ADDRESS &&
       !EIP207Lib_Flow_Is32bitAddressable(
                TrueIOArea_p->HT_Params[HashTableId].BaseAddr.Addr,
                TrueIOArea_p->HT_Params[HashTableId].BaseAddr.UpperAddr,
                FlowInData_p->Xform_DMA_Addr.Addr,
                FlowInData_p->Xform_DMA_Addr.UpperAddr))
        return EIP207_FLOW_ARGUMENT_ERROR;
#endif // EIP207_FLOW_STRICT_ARGS

    ZEROINIT(Rec_Data);
    ZEROINIT(FR_Data);

    FR_Data.Flags           = FlowInData_p->Flags;
    FR_Data.SW_FR_Reference = FlowInData_p->SW_FR_Reference;
    FR_Data.Xform_DMA_Addr  = FlowInData_p->Xform_DMA_Addr;
    FR_Data.fLarge          = FlowInData_p->fLarge;

    Rec_Data.HashID_p       = &FlowInData_p->HashID;
    Rec_Data.FR_Data_p      = &FR_Data;

    Flow_Rc = EIP207Lib_Flow_DTL_Record_Add(
                            &TrueIOArea_p->HT_Params[HashTableId],
                            (EIP207_Flow_Dscr_t*)FR_Dscr_p,
                            &Rec_Data);
    if (Flow_Rc != EIP207_FLOW_NO_ERROR)
        return Flow_Rc;

#ifdef EIP207_FLOW_DEBUG_FSM
    {
        EIP207_Flow_Error_t rv;
        uint32_t State = TrueIOArea_p->State;

        TrueIOArea_p->Rec_InstalledCounter++;

        // Transit to a new state
        rv = EIP207_Flow_State_Set((EIP207_Flow_State_t* const)&State,
                                   EIP207_FLOW_STATE_INSTALLED);

        TrueIOArea_p->State = State;

        if (rv != EIP207_FLOW_NO_ERROR)
            return EIP207_FLOW_ILLEGAL_IN_STATE;
    }
#endif // EIP207_FLOW_DEBUG_FSM

    return EIP207_FLOW_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP207_Flow_RC_BaseAddr_Set
 */
EIP207_Flow_Error_t
EIP207_Flow_RC_BaseAddr_Set(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        const EIP207_Flow_Address_t * const FlowBaseAddr_p,
        const EIP207_Flow_Address_t * const TransformBaseAddr_p)
{
    Device_Handle_t Device;
    volatile EIP207_Flow_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP207_FLOW_CHECK_POINTER(IOArea_p);
    EIP207_FLOW_CHECK_INT_ATMOST(HashTableId + 1,
                                 EIP207_FLOW_MAX_NOF_FLOW_HASH_TABLES_TO_USE);

    IDENTIFIER_NOT_USED(TransformBaseAddr_p);

    Device = TrueIOArea_p->Device;

    // Set the common base address for all the records which can be looked up
    // Note: Standard (legacy) record cache should have
    //       EIP207_RC_SET_NR_DEFAULT set (which is 0, just one Hash Table);
    //       High-performance (HP) record cache should have
    //       HashTableId set
#ifndef EIP207_FLUE_RC_HP
    EIP207_RC_BaseAddr_Set(Device,
                           EIP207_FRC_REG_BASE,
                           HashTableId, // EIP207_RC_SET_NR_DEFAULT
                           FlowBaseAddr_p->Addr,
                           FlowBaseAddr_p->UpperAddr);
#else
    EIP207_FLUE_CACHEBASE_LO_WR(Device, HashTableId, FlowBaseAddr_p->Addr);
    EIP207_FLUE_CACHEBASE_HI_WR(Device, HashTableId, FlowBaseAddr_p->UpperAddr);
#endif

    TrueIOArea_p->HT_Params[HashTableId].BaseAddr.Addr = FlowBaseAddr_p->Addr;
    TrueIOArea_p->HT_Params[HashTableId].BaseAddr.UpperAddr =
                                                    FlowBaseAddr_p->UpperAddr;

    return  EIP207_FLOW_NO_ERROR;
}


/*****************************************************************************
 * DTL-specific API functions
 */

/*----------------------------------------------------------------------------
 * EIP207_Flow_DTL_FR_Remove
 */
EIP207_Flow_Error_t
EIP207_Flow_DTL_FR_Remove(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        EIP207_Flow_FR_Dscr_t * const FR_Dscr_p)
{
    volatile EIP207_Flow_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);
    EIP207_Flow_HTE_Dscr_RecData_t * FR_Data_p;
    EIP207_Flow_Error_t Flow_Rc;

    EIP207_FLOW_CHECK_POINTER(IOArea_p);
    EIP207_FLOW_CHECK_INT_ATMOST(HashTableId + 1,
                                 EIP207_FLOW_MAX_NOF_FLOW_HASH_TABLES_TO_USE);
    EIP207_FLOW_CHECK_POINTER(FR_Dscr_p);

    FR_Data_p = (EIP207_Flow_HTE_Dscr_RecData_t*)FR_Dscr_p->InternalData_p;

    EIP207_FLOW_CHECK_POINTER(FR_Data_p);
    EIP207_FLOW_CHECK_POINTER(FR_Data_p->HTE_Dscr_p);

#ifdef EIP207_FLOW_STRICT_ARGS
    // Provided flow record physical DMA address must be addressable by
    // a 32-bit offset to the Based Address installed via
    // the EIP207_Flow_RC_BaseAddr_Set() function
    if(!EIP207Lib_Flow_Is32bitAddressable(
                TrueIOArea_p->HT_Params[HashTableId].BaseAddr.Addr,
                TrueIOArea_p->HT_Params[HashTableId].BaseAddr.UpperAddr,
                FR_Dscr_p->DMA_Addr.Addr,
                FR_Dscr_p->DMA_Addr.UpperAddr))
        return EIP207_FLOW_ARGUMENT_ERROR;

    if (FR_Data_p->Type != EIP207_FLOW_REC_FLOW)
        return EIP207_FLOW_INTERNAL_ERROR;
#endif // EIP207_FLOW_STRICT_ARGS

    Flow_Rc = EIP207Lib_Flow_DTL_Record_Remove(
                          TrueIOArea_p->Device,
                          HashTableId,
                          &TrueIOArea_p->HT_Params[HashTableId],
                          (EIP207_Flow_Dscr_t*)FR_Dscr_p);
    if (Flow_Rc != EIP207_FLOW_NO_ERROR)
        return Flow_Rc;

    IDENTIFIER_NOT_USED(FR_Data_p);

#ifdef EIP207_FLOW_DEBUG_FSM
    {
        EIP207_Flow_Error_t rv;
        uint32_t State = TrueIOArea_p->State;

        TrueIOArea_p->Rec_InstalledCounter--;

        // Transit to a new state
        if (TrueIOArea_p->Rec_InstalledCounter == 0)
            // Last record is removed
            rv = EIP207_Flow_State_Set(
                    (EIP207_Flow_State_t* const)&State,
                    EIP207_FLOW_STATE_ENABLED);
        else
            // Non-last record is removed
            rv = EIP207_Flow_State_Set(
                    (EIP207_Flow_State_t* const)&State,
                    EIP207_FLOW_STATE_INSTALLED);

        TrueIOArea_p->State = State;

        if (rv != EIP207_FLOW_NO_ERROR)
            return EIP207_FLOW_ILLEGAL_IN_STATE;
    }
#endif // EIP207_FLOW_DEBUG_FSM

    return EIP207_FLOW_NO_ERROR;
}


#ifndef EIP207_FLOW_REMOVE_TR_LARGE_SUPPORT
/*----------------------------------------------------------------------------
 * EIP207_Flow_DTL_TR_Large_WordCount_Get
 */
unsigned int
EIP207_Flow_DTL_TR_Large_WordCount_Get(void)
{
    return FIRMWARE_EIP207_CS_FLOW_TRC_RECORD_WORD_COUNT_LARGE;
}
#endif // !EIP207_FLOW_REMOVE_TR_LARGE_SUPPORT


/*----------------------------------------------------------------------------
 * EIP207_Flow_DTL_TR_Add
 */
EIP207_Flow_Error_t
EIP207_Flow_DTL_TR_Add(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        EIP207_Flow_TR_Dscr_t * const TR_Dscr_p,
        const EIP207_Flow_TR_InputData_t * const XformInData_p)
{
    volatile EIP207_Flow_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);
    EIP207_Flow_Record_InputData_t Rec_Data;
    EIP207_Flow_TR_Data_t TR_Data;
    EIP207_Flow_Error_t Flow_Rc;

    EIP207_FLOW_CHECK_POINTER(IOArea_p);
    EIP207_FLOW_CHECK_INT_ATMOST(HashTableId + 1,
                                 EIP207_FLOW_MAX_NOF_FLOW_HASH_TABLES_TO_USE);
    EIP207_FLOW_CHECK_POINTER(TR_Dscr_p);
    EIP207_FLOW_CHECK_POINTER(XformInData_p);

#ifdef EIP207_FLOW_STRICT_ARGS
    // Provided transform record physical DMA address must be addressable by
    // a 32-bit offset to the Transform Based Address installed via
    // the EIP207_Flow_RC_BaseAddr_Set() function
    if(!EIP207Lib_Flow_Is32bitAddressable(
                    TrueIOArea_p->HT_Params[HashTableId].BaseAddr.Addr,
                    TrueIOArea_p->HT_Params[HashTableId].BaseAddr.UpperAddr,
                    TR_Dscr_p->DMA_Addr.Addr,
                    TR_Dscr_p->DMA_Addr.UpperAddr))
        return EIP207_FLOW_ARGUMENT_ERROR;
#endif // EIP207_FLOW_STRICT_ARGS

    ZEROINIT(Rec_Data);
    ZEROINIT(TR_Data);

    TR_Data.fLarge     = XformInData_p->fLarge;

    Rec_Data.HashID_p  = &XformInData_p->HashID;
    Rec_Data.TR_Data_p = &TR_Data;

    Flow_Rc = EIP207Lib_Flow_DTL_Record_Add(
                            &TrueIOArea_p->HT_Params[HashTableId],
                            (EIP207_Flow_Dscr_t*)TR_Dscr_p,
                            &Rec_Data);
    if (Flow_Rc != EIP207_FLOW_NO_ERROR)
        return Flow_Rc;

#ifdef EIP207_FLOW_DEBUG_FSM
    {
        EIP207_Flow_Error_t rv;
        uint32_t State = TrueIOArea_p->State;

        TrueIOArea_p->Rec_InstalledCounter++;

        // Transit to a new state
        rv = EIP207_Flow_State_Set((EIP207_Flow_State_t* const)&State,
                                   EIP207_FLOW_STATE_INSTALLED);

        TrueIOArea_p->State = State;

        if (rv != EIP207_FLOW_NO_ERROR)
            return EIP207_FLOW_ILLEGAL_IN_STATE;
    }
#endif // EIP207_FLOW_DEBUG_FSM

    return EIP207_FLOW_NO_ERROR;
}


#ifndef EIP207_FLOW_REMOVE_TR_LARGE_SUPPORT
/*----------------------------------------------------------------------------
 * EIP207_Flow_DTL_TR_Large_Read
 */
EIP207_Flow_Error_t
EIP207_Flow_DTL_TR_Large_Read(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        const EIP207_Flow_TR_Dscr_t * const TR_Dscr_p,
        EIP207_Flow_TR_OutputData_t * const XformData_p)
{
    EIP207_Flow_Error_t rv;
    uint32_t Value32, SeqNrWordOffset;

    EIP207_FLOW_CHECK_POINTER(IOArea_p);
    EIP207_FLOW_CHECK_POINTER(TR_Dscr_p);
    EIP207_FLOW_CHECK_POINTER(XformData_p);
    EIP207_FLOW_CHECK_INT_ATMOST(HashTableId + 1,
                                 EIP207_FLOW_MAX_NOF_FLOW_HASH_TABLES_TO_USE);

    IDENTIFIER_NOT_USED(IOArea_p);
    IDENTIFIER_NOT_USED(HashTableId);

    // Prepare the transform record for reading
    DMAResource_PostDMA(TR_Dscr_p->DMA_Handle, 0, 0);

    // Read the transform record data

    // Recent record Packets Counter
    XformData_p->PacketsCounter =
            DMAResource_Read32(
                    TR_Dscr_p->DMA_Handle,
                    FIRMWARE_EIP207_CS_FLOW_TR_STAT_PKT_WORD_OFFSET_LARGE);

    // Read Token Context Instruction word
    Value32 =
        DMAResource_Read32(TR_Dscr_p->DMA_Handle,
                FIRMWARE_EIP207_CS_FLOW_TR_TK_CTX_INST_WORD_OFFSET_LARGE);

    // Extract the Sequence Number word offset from the read value
    FIRMWARE_EIP207_CS_Flow_SeqNum_Offset_Read(Value32, &SeqNrWordOffset);

    // Read the sequence number
    XformData_p->SequenceNumber =
            DMAResource_Read32(TR_Dscr_p->DMA_Handle, SeqNrWordOffset);

    // Recent record time stamp
    rv = EIP207_Flow_Internal_Read64(
                   TR_Dscr_p->DMA_Handle,
                   FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_LO_WORD_OFFSET_LARGE,
                   FIRMWARE_EIP207_CS_FLOW_TR_TIME_STAMP_HI_WORD_OFFSET_LARGE,
                   &XformData_p->LastTimeLo,
                   &XformData_p->LastTimeHi);
    if (rv != EIP207_FLOW_NO_ERROR)
        return rv;

    // Recent record Octets Counter
    rv = EIP207_Flow_Internal_Read64(
                   TR_Dscr_p->DMA_Handle,
                   FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_LO_WORD_OFFSET_LARGE,
                   FIRMWARE_EIP207_CS_FLOW_TR_STAT_OCT_HI_WORD_OFFSET_LARGE,
                   &XformData_p->OctetsCounterLo,
                   &XformData_p->OctetsCounterHi);
    if (rv != EIP207_FLOW_NO_ERROR)
        return rv;

    return EIP207_FLOW_NO_ERROR;
}
#endif // !EIP207_FLOW_REMOVE_TR_LARGE_SUPPORT


/*----------------------------------------------------------------------------
 * EIP207_Flow_DTL_TR_Remove
 */
EIP207_Flow_Error_t
EIP207_Flow_DTL_TR_Remove(
        EIP207_Flow_IOArea_t * const IOArea_p,
        const unsigned int HashTableId,
        EIP207_Flow_TR_Dscr_t * const TR_Dscr_p)
{
    volatile EIP207_Flow_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);
    EIP207_Flow_HTE_Dscr_RecData_t * TR_Data_p;
    EIP207_Flow_Error_t Flow_Rc;

    EIP207_FLOW_CHECK_POINTER(IOArea_p);
    EIP207_FLOW_CHECK_INT_ATMOST(HashTableId + 1,
                                 EIP207_FLOW_MAX_NOF_FLOW_HASH_TABLES_TO_USE);
    EIP207_FLOW_CHECK_POINTER(TR_Dscr_p);

    TR_Data_p = (EIP207_Flow_HTE_Dscr_RecData_t*)TR_Dscr_p->InternalData_p;

    EIP207_FLOW_CHECK_POINTER(TR_Data_p);
    EIP207_FLOW_CHECK_POINTER(TR_Data_p->HTE_Dscr_p);

#ifdef EIP207_FLOW_STRICT_ARGS
    // Provided transform record physical DMA address must be addressable by
    // a 32-bit offset to the Transform Based Address installed via
    // the EIP207_Flow_RC_BaseAddr_Set() function
    if(!EIP207Lib_Flow_Is32bitAddressable(
                    TrueIOArea_p->HT_Params[HashTableId].BaseAddr.Addr,
                    TrueIOArea_p->HT_Params[HashTableId].BaseAddr.UpperAddr,
                    TR_Dscr_p->DMA_Addr.Addr,
                    TR_Dscr_p->DMA_Addr.UpperAddr))
        return EIP207_FLOW_ARGUMENT_ERROR;

    if (TR_Data_p->Type != EIP207_FLOW_REC_TRANSFORM_SMALL &&
        TR_Data_p->Type != EIP207_FLOW_REC_TRANSFORM_LARGE)
        return EIP207_FLOW_INTERNAL_ERROR;
#endif // EIP207_FLOW_STRICT_ARGS

    Flow_Rc = EIP207Lib_Flow_DTL_Record_Remove(
                          TrueIOArea_p->Device,
                          HashTableId,
                          &TrueIOArea_p->HT_Params[HashTableId],
                          (EIP207_Flow_Dscr_t*)TR_Dscr_p);
    if (Flow_Rc != EIP207_FLOW_NO_ERROR)
        return Flow_Rc;

    IDENTIFIER_NOT_USED(TR_Data_p);

#ifdef EIP207_FLOW_DEBUG_FSM
    {
        EIP207_Flow_Error_t rv;
        uint32_t State = TrueIOArea_p->State;

        TrueIOArea_p->Rec_InstalledCounter--;

        // Transit to a new state
        if (TrueIOArea_p->Rec_InstalledCounter == 0)
            // Last record is removed
            rv = EIP207_Flow_State_Set(
                    (EIP207_Flow_State_t* const)&State,
                    EIP207_FLOW_STATE_ENABLED);
        else
            // Non-last record is removed
            rv = EIP207_Flow_State_Set(
                    (EIP207_Flow_State_t* const)&State,
                    EIP207_FLOW_STATE_INSTALLED);

        TrueIOArea_p->State = State;

        if (rv != EIP207_FLOW_NO_ERROR)
            return EIP207_FLOW_ILLEGAL_IN_STATE;
    }
#endif // EIP207_FLOW_DEBUG_FSM

    return EIP207_FLOW_NO_ERROR;
}


/* end of file eip207_flow_dtl.c */
