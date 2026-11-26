/* eip207_rc_internal.c
 *
 * EIP-207 Record Cache (RC) interface implementation
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

// Default configuration
#include "c_eip207_global.h"

// EIP-207 Record Cache (RC) internal interface
#include "eip207_rc_internal.h"

// EIP97_Interfaces_Get()
#include "eip97_global_internal.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // uint32_t, bool

// EIP-207 Global Control Driver Library Internal interfaces
#include "eip207_level0.h"              // EIP-207 Level 0 macros

// EIP-206 Global Control Driver Library Internal interfaces
#include "eip206_level0.h"              // EIP-206 Level 0 macros

// Driver Framework Device API
#include "device_types.h"               // Device_Handle_t

// EIP-207 Firmware Classification API
#include "firmware_eip207_api_cs.h"     // Classification API: General


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#if FIRMWARE_EIP207_CS_ARC4RC_RECORD_WORD_COUNT <= \
                FIRMWARE_EIP207_CS_TRC_RECORD_WORD_COUNT
#define EIP207_RC_ARC4_SIZE                             0
#elif FIRMWARE_EIP207_CS_ARC4RC_RECORD_WORD_COUNT <= \
                FIRMWARE_EIP207_CS_TRC_RECORD_WORD_COUNT_LARGE
#define EIP207_RC_ARC4_SIZE                             1
#else
#error "Error: ARC4 State Record size too big"
#endif

// Minimum number of entries in the Record Cache
#define EIP207_RC_MIN_ENTRY_COUNT                       32

// Maximum number of entries in the Record Cache
#define EIP207_RC_MAX_ENTRY_COUNT                       4096

// Maximum number of records in cachs
#define EIP207_RC_MAX_RECORD_COUNT                      1023

// Number of header words (32-bits) in a cache record
#define EIP207_RC_HEADER_WORD_COUNT                     4

// Number of 32-bit words in one administration memory word
#define EIP207_RC_ADMIN_MEMWORD_WORD_COUNT              4

// Number of hash table entries in one administration memory word
#define EIP207_RC_ADMIN_MEMWORD_ENTRY_COUNT             8

// Null value used in Record Caches
#define EIP207_RC_NULL_VALUE                            0x3FF

// Checks for the required configuration parameters
#ifndef EIP207_FRC_ADMIN_RAM_WORD_COUNT
#error "EIP207_FRC_ADMIN_RAM_WORD_COUNT not defined"
#endif // EIP207_FRC_ADMIN_RAM_WORD_COUNT

#ifndef EIP207_TRC_ADMIN_RAM_WORD_COUNT
#error "EIP207_TRC_ADMIN_RAM_WORD_COUNT not defined"
#endif // EIP207_TRC_ADMIN_RAM_WORD_COUNT

#ifndef EIP207_ARC4RC_ADMIN_RAM_WORD_COUNT
#error "EIP207_ARC4RC_ADMIN_RAM_WORD_COUNT not defined"
#endif // EIP207_ARC4RC_ADMIN_RAM_WORD_COUNT

// Minimum required Record Cache Admin RAM size
#define EIP207_RC_MIN_ADMIN_RAM_WORD_COUNT \
      (((EIP207_RC_MIN_ENTRY_COUNT / EIP207_RC_ADMIN_MEMWORD_ENTRY_COUNT) * \
          EIP207_RC_ADMIN_MEMWORD_WORD_COUNT) + \
             EIP207_RC_HEADER_WORD_COUNT * EIP207_RC_MIN_ENTRY_COUNT)

// Check if the configured FRC Admin RAM size is large enough to contain
// the minimum required number of record headers with their hash table
#if EIP207_FRC_ADMIN_RAM_WORD_COUNT > 0 && \
    EIP207_FRC_ADMIN_RAM_WORD_COUNT < EIP207_RC_MIN_ADMIN_RAM_WORD_COUNT
#error "Configured FRC Admin RAM size is too small"
#endif

// Check if the configured TRC Admin RAM size is large enough to contain
// the minimum required number of record headers with their hash table
#if EIP207_TRC_ADMIN_RAM_WORD_COUNT > 0 && \
    EIP207_TRC_ADMIN_RAM_WORD_COUNT < EIP207_RC_MIN_ADMIN_RAM_WORD_COUNT
#error "Configured TRC Admin RAM size is too small"
#endif

// Check if the (optional) configured ARC4RC Admin RAM size is large enough
// to contain the minimum required number of record headers
// with their hash table
#if EIP207_ARC4RC_ADMIN_RAM_WORD_COUNT > 0 && \
    EIP207_ARC4RC_ADMIN_RAM_WORD_COUNT < EIP207_RC_MIN_ADMIN_RAM_WORD_COUNT
#error "Configured ARC4RC Admin RAM size is too small"
#endif

// Hash Table Size calculation for *RC_p_PARAMS registers
// Hash Table entry count =
//      2 ^ (Hash Table Size + EIP207_RC_HASH_TABLE_SIZE_POWER_FACTOR)
#define EIP207_RC_HASH_TABLE_SIZE_POWER_FACTOR      5


/*----------------------------------------------------------------------------
 * EIP207Lib_RC_Internal_LowerPowerOfTwo
 *
 * Rounds down a value to the lower or equal power of two value.
 */
static unsigned int
EIP207Lib_RC_Internal_LowerPowerOfTwo(const unsigned int Value,
                                      unsigned int * const Power)
{
    unsigned int v = Value;
    unsigned int i = 0;

    if (v == 0)
        return v;

    while (v)
    {
        v = v >> 1;
        i++;
    }

    v = 1 << (i - 1);

    *Power = i - 1;

    return v;
}


#define EIP207_RC_DATA_WORDCOUNT_MAX 131072
#define EIP207_RC_DATA_WORDCOUNT_MIN 256
#define EIP207_RC_ADMIN_WORDCOUNT_MAX 16384
#define EIP207_RC_ADMIN_WORDCOUNT_MIN 64

/*----------------------------------------------------------------------------
 * EIP207Lib_Bank_Set
 *
 * Set the bank for the CS RAM to the given address.
 *
 * Device (input)
 *     Device to use.
 *
 * BankNr (input)
 *     Bank number from 0 to 7.
 *
 */
static void
EIP207Lib_Bank_Set(
    const Device_Handle_t Device,
    uint32_t BankNr)
{
    uint32_t OldValue = Device_Read32(Device, EIP207_CS_REG_RAM_CTRL);

    uint32_t NewValue = (OldValue & 0xffff8fff) | ((BankNr&0x7) << 12);

    Device_Write32(Device, EIP207_CS_REG_RAM_CTRL, NewValue);
}


static void
EIP207Lib_RAM_Write32(
        const Device_Handle_t Device,
        uint32_t Address,
        uint32_t Value)
{
    Device_Write32(Device,
                   EIP207_CS_RAM_XS_SPACE_BASE + Address,
                   Value);
}

static uint32_t
EIP207Lib_RAM_Read32(
        const Device_Handle_t Device,
        uint32_t Address)
{
    return Device_Read32(Device,
                         EIP207_CS_RAM_XS_SPACE_BASE + Address);
}



/*----------------------------------------------------------------------------
 * EIP207Lib_RAMSize_Probe
 *
 * Probe the size of the accessible RAM, do not access more memory than
 * indicated by MaxSize.
 *
 * Device (input)
 *     Device to use.
 *
 * MaxSize (input)
 *     Maximum size of RAM
 */
static unsigned int
EIP207Lib_RAMSize_Probe(
    const Device_Handle_t Device,
    const unsigned int MaxSize)
{
    unsigned int MaxBank, MaxPage, MaxOffs, i, RAMSize;

    if (MaxSize <= 16384)
    {
        // All RAM is in a single bank.
        MaxBank = 0;
    }
    else
    {
        // Probe the maximum bank number that has (distinct) RAM.
        for (i=0; i<8; i++)
        {
            EIP207Lib_Bank_Set(Device, 7 - i);
            EIP207Lib_RAM_Write32(Device, 0, 7 - i);
            EIP207Lib_RAM_Write32(Device, 4, 0);
            EIP207Lib_RAM_Write32(Device, 8, 0);
            EIP207Lib_RAM_Write32(Device, 12, 0);
        }
        MaxBank=0;
        for (i=0; i<7; i++)
        {
            EIP207Lib_Bank_Set(Device, i);
            if (EIP207Lib_RAM_Read32(Device, 0) != i)
            {
                break;
            }
            MaxBank = i;
        }
    }

    EIP207Lib_Bank_Set(Device, MaxBank);

    for (i=0; i<0x10000; i+=0x100)
    {
        EIP207Lib_RAM_Write32(Device, 0xff00-i, 0xff00-i);
        EIP207Lib_RAM_Write32(Device, 0xff00-i+4, 0);
        EIP207Lib_RAM_Write32(Device, 0xff00-i+8, 0);
        EIP207Lib_RAM_Write32(Device, 0xff00-i+12, 0);
    }

    MaxPage = 0;
    for (i=0; i<0x10000; i+=0x100)
    {
        if (EIP207Lib_RAM_Read32(Device, i) != i)
        {
            break;
        }
        MaxPage = i;
    }

    for (i=0; i<0x100; i+= 4)
    {
        EIP207Lib_RAM_Write32(Device, MaxPage + 0xfc - i, MaxPage + 0xfc - i);
    }

    MaxOffs = 0;

    for (i=0; i<0x100; i+=4)
    {
        if (EIP207Lib_RAM_Read32(Device, MaxPage + i) != MaxPage + i)
        {
            break;
        }
        MaxOffs = i;
    }

    EIP207Lib_Bank_Set(Device, 0);
    RAMSize = ((MaxBank<<16) + MaxPage + MaxOffs + 4) >> 2;

    if (RAMSize > MaxSize)
        RAMSize = MaxSize;

    return RAMSize;
}

/*----------------------------------------------------------------------------
 * EIP207_RC_Internal_Init
 */
EIP207_Global_Error_t
EIP207_RC_Internal_Init(
        const Device_Handle_t Device,
        const EIP207_RC_Internal_Combination_Type_t CombinationType,
        const uint32_t CacheBase,
        EIP207_Global_CacheParams_t * RC_Params_p,
        const unsigned int RecordWordCount)
{
    unsigned int i;
    uint16_t RC_Record2_WordCount = 0;
    uint8_t ClocksPerTick;
    bool fFrc = false, fTrc = false, fArc4 = false;
    EIP207_Global_CacheParams_t * RC_p = RC_Params_p;
    unsigned int NullVal = EIP207_RC_NULL_VALUE;

    switch (CacheBase)
    {
        case EIP207_FRC_REG_BASE:
            fFrc = true;
            RC_Record2_WordCount = 0;
            break;

        case EIP207_TRC_REG_BASE:
            fTrc = true;
            RC_Record2_WordCount =
                    FIRMWARE_EIP207_CS_TRC_RECORD_WORD_COUNT_LARGE;
            break;

        case EIP207_ARC4RC_REG_BASE:
            fArc4 = true;
            RC_Record2_WordCount =
                    FIRMWARE_EIP207_CS_ARC4RC_RECORD_WORD_COUNT_LARGE;
            break;

        default:
            return EIP207_GLOBAL_ARGUMENT_ERROR;
    }

    if (CombinationType != EIP207_RC_INTERNAL_NOT_COMBINED)
    {
        if( fFrc ) // FRC cannot be combined
            return EIP207_GLOBAL_ARGUMENT_ERROR;

        // Initialize all the configured for use Record Cache sets
        for (i = 0; i < EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE; i++)
            EIP207_RC_PARAMS_WR(Device,
                                CacheBase,
                                i,     // Cache Set number
                                false, // Disable cache RAM access
                                false, // Block Next Command is reserved
                                false, // Enable access cache administration RAM
                                0,
                                0,     // Block Time Base is reserved
                                false, // Not used for this HW
                                RC_Record2_WordCount); // Large record size

        return EIP207_GLOBAL_NO_ERROR;
    }
    // Indicate if ARC4 state records are considered 'large' transform records.
    // Only relevent inin case the TRC is used to store ARC4 state records, but
    // without a combined cache.
    if (fTrc)
    {
        unsigned int NofCEs;
        EIP97_Interfaces_Get(&NofCEs,NULL,NULL,NULL);
        for (i = 0; i < NofCEs; i++)
            EIP206_ARC4_SIZE_WR(Device, i, EIP207_RC_ARC4_SIZE);
    }

    // Initialize all the configured for use Record Cache sets
    for (i = 0; i < EIP207_GLOBAL_MAX_NOF_CACHE_SETS_TO_USE; i++)
    {
        unsigned int RC_RAM_WordCount = RC_p->DataWordCount;
        uint8_t RC_HashTableSize;
        unsigned int j, Power,
                     RC_RecordCount,
                     RC_Record_WordCount,
                     RC_HashTable_EntryCount,
                     RC_HashTable_WordCount,
                     RC_AdminRAM_WordCount,
                     RC_AdminRAM_EntryCount,
                     RC_HashTable_ByteOffset;

        if (RC_p->fEnable == false)
            continue; // Cache is not enabled

        // Enable Record Cache RAM access
        EIP207_CS_RAM_CTRL_WR(
            Device,
            fFrc && i==0,
            fFrc && i==1,
            fFrc && i==2,
            fTrc && i==0,
            fTrc && i==1,
            fTrc && i==2,
            fArc4 && i==0,
            fArc4 && i==1,
            fArc4 && i==2,
            false);               // No FLUEC cache RAM access

        // Take Record Cache into reset
        // Make cache data RAM accessible
        EIP207_RC_PARAMS_WR(Device,
                            CacheBase,
                            i,     // Cache Set number
                            true,  // Enable cache RAM access
                            false,
                            true, // Enable access cache data RAM
                            0,
                            0,
                            false, // Not used here for this HW
                            0);

        if (RC_RAM_WordCount == 0 ||
            RC_RAM_WordCount > EIP207_RC_DATA_WORDCOUNT_MAX)
            RC_RAM_WordCount = EIP207_RC_DATA_WORDCOUNT_MAX;
        // Check the size of the data RAM.
        RC_RAM_WordCount = EIP207Lib_RAMSize_Probe(
            Device,
            RC_RAM_WordCount);

        // Data RAM may be inaccessible on some hardware configurations,
        // so RAM size probing may not work. Assume that provided word count
        // input actually reflects RAM size.
        if (RC_RAM_WordCount < EIP207_RC_DATA_WORDCOUNT_MIN)
            RC_RAM_WordCount = MIN(RC_p->DataWordCount, EIP207_RC_DATA_WORDCOUNT_MAX);
        if (RC_RAM_WordCount < EIP207_RC_DATA_WORDCOUNT_MIN)
            return EIP207_GLOBAL_INTERNAL_ERROR;

        RC_p->DataWordCount = RC_RAM_WordCount;

        // Take Record Cache into reset
        // Make cache administration RAM accessible
        EIP207_RC_PARAMS_WR(Device,
                            CacheBase,
                            i,     // Cache Set number
                            true,  // Enable cache RAM access
                            false,
                            false, // Enable access cache administration RAM
                            0,
                            0,
                            false, // Not used here for this HW
                            0);

        // Get the configured RC Admin RAM size
        RC_AdminRAM_WordCount = RC_p->AdminWordCount;
        if (RC_AdminRAM_WordCount == 0 ||
            RC_AdminRAM_WordCount > EIP207_RC_ADMIN_WORDCOUNT_MAX)
            RC_AdminRAM_WordCount = EIP207_RC_ADMIN_WORDCOUNT_MAX;

        RC_AdminRAM_WordCount = EIP207Lib_RAMSize_Probe(
            Device,
            RC_AdminRAM_WordCount);

        RC_p->AdminWordCount = RC_AdminRAM_WordCount;

        if (RC_AdminRAM_WordCount < EIP207_RC_ADMIN_WORDCOUNT_MIN)
            return EIP207_GLOBAL_INTERNAL_ERROR;

       // Check the size of the Admin RAM

        // Determine the RC record size to use
        if (RC_Record2_WordCount > RecordWordCount)
            RC_Record_WordCount = RC_Record2_WordCount;
        else
            RC_Record_WordCount = RecordWordCount;

        // Calculate the maximum possible record count that
        // the Record Cache Data RAM can contain
        RC_RecordCount = RC_RAM_WordCount / RC_Record_WordCount;
        if (RC_RecordCount > EIP207_RC_MAX_RECORD_COUNT)
            RC_RecordCount = EIP207_RC_MAX_RECORD_COUNT;

        // RC_RecordCount is calculated using the configured RC Data RAM size.

        // RC_AdminRAM_EntryCount is calculated using
        // the configured RC Admin RAM size.

        // Calculate the maximum possible record count that
        // the RC Hash Table (in Record Cache Administration RAM) can contain
        RC_AdminRAM_EntryCount = EIP207_RC_ADMIN_MEMWORD_ENTRY_COUNT *
                                    RC_AdminRAM_WordCount /
                                (EIP207_RC_ADMIN_MEMWORD_WORD_COUNT +
                                        EIP207_RC_ADMIN_MEMWORD_ENTRY_COUNT *
                                           EIP207_RC_HEADER_WORD_COUNT);

        // Try to extend the Hash Table in the RC Admin RAM
        if (RC_RecordCount < RC_AdminRAM_EntryCount)
        {
            unsigned int HTSpace_WordCount;

            // Calculate the size of space available for the Hash Table
            HTSpace_WordCount = RC_AdminRAM_WordCount -
                                  RC_RecordCount * EIP207_RC_HEADER_WORD_COUNT;

            // Calculate maximum possible Hash Table entry count
            RC_HashTable_EntryCount = (HTSpace_WordCount /
                                        EIP207_RC_ADMIN_MEMWORD_WORD_COUNT) *
                                           EIP207_RC_ADMIN_MEMWORD_ENTRY_COUNT;
        }
        else // Extension impossible
            RC_HashTable_EntryCount = RC_AdminRAM_EntryCount;

        // Check minimum number of entries in the record cache
        RC_HashTable_EntryCount = MAX(EIP207_RC_MIN_ENTRY_COUNT,
                                      RC_HashTable_EntryCount);

        // Check maximum number of entries in the record cache
        RC_HashTable_EntryCount = MIN(EIP207_RC_MAX_ENTRY_COUNT,
                                      RC_HashTable_EntryCount);

        // Round down to power of two
        Power = 0;
        RC_HashTable_EntryCount =
             EIP207Lib_RC_Internal_LowerPowerOfTwo(RC_HashTable_EntryCount,
                                                   &Power);

        // Hash Table Mask that determines the hash table size
        if (Power >= EIP207_RC_HASH_TABLE_SIZE_POWER_FACTOR)
            RC_HashTableSize =
                    (uint8_t)(Power - EIP207_RC_HASH_TABLE_SIZE_POWER_FACTOR);
        else
            // Insufficient memory for Hash Table in the RC Admin RAM
            return EIP207_GLOBAL_INTERNAL_ERROR;

        // Calculate the Hash Table size in 32-bit words
        RC_HashTable_WordCount = RC_HashTable_EntryCount /
                                  EIP207_RC_ADMIN_MEMWORD_ENTRY_COUNT *
                                       EIP207_RC_ADMIN_MEMWORD_WORD_COUNT;

        // Recalculate the record count that fits the RC Admin RAM space
        // without the Hash Table, restricting for the maximum records
        // which fit the RC Data RAM
        {
            // Adjusted record count which fits the RC Admin RAM
            unsigned int RC_AdminRAM_AdjustedEntryCount =
                             (RC_AdminRAM_WordCount -
                                RC_HashTable_WordCount) /
                                   EIP207_RC_HEADER_WORD_COUNT;

            // Maximum record count which fits the RC Data RAM - RC_RecordCount
            // use the minimum of the two
            RC_RecordCount = MIN(RC_RecordCount,
                                 RC_AdminRAM_AdjustedEntryCount);
        }

        // Clear all ECC errors
        EIP207_RC_ECCCTRL_WR(Device, CacheBase, i, false, false, false);

        // Clear all record administration words
        // in Record Cache administration RAM
        for (j = 0; j < RC_RecordCount; j++)
        {
            // Calculate byte offset for the current record
            unsigned int ByteOffset = EIP207_CS_RAM_XS_SPACE_BASE +
                                      j *
                                        EIP207_RC_HEADER_WORD_COUNT *
                                          sizeof(uint32_t);

            // Write word 0
            Device_Write32(Device,
                           ByteOffset,
                           (NullVal << 20) | // Hash_Collision_Prev
                           (NullVal << 10)); // Hash_Collision_Next

            // Write word 1
            ByteOffset += sizeof(uint32_t);

            if (j == RC_RecordCount - 1)
            {
                // Last record
                Device_Write32(Device,
                               ByteOffset,
                               ((j - 1) << 10) |   // Free_List_Prev
                               NullVal);           // Free_List_Next
            }
            else if (j == 0)
            {
                // First record
                Device_Write32(Device,
                               ByteOffset,
                               (NullVal << 10) | // Free_List_Prev
                               (j + 1));         // Free_List_Next
            }
            else
            {
                // All other records
                Device_Write32(Device,
                               ByteOffset,
                               ((j - 1) << 10) | // Free_List_Prev
                               (j + 1));         // Free_List_Next
            }

            // Write word 2
            ByteOffset += sizeof(uint32_t);

            Device_Write32(Device,
                           ByteOffset,
                           0); // Address_Key, low bits

            // Write word 3
            ByteOffset += sizeof(uint32_t);

            Device_Write32(Device,
                           ByteOffset,
                           0); // Address_Key, high bits
        } // for (records)

        // Calculate byte offset for the Hash Table
        RC_HashTable_ByteOffset = EIP207_CS_RAM_XS_SPACE_BASE +
                            RC_RecordCount *
                                EIP207_RC_HEADER_WORD_COUNT *
                                    sizeof(uint32_t);

        // Clear all hash table words
        for (j = 0; j < RC_HashTable_WordCount; j++)
            Device_Write32(Device,
                           RC_HashTable_ByteOffset + j * sizeof(uint32_t),
                           0x3FFFFFFF);

        // Disable Record Cache RAM access
        EIP207_CS_RAM_CTRL_DEFAULT_WR(Device);

        // Write head and tail pointers to the RC Free Chain
        EIP207_RC_FREECHAIN_WR(
                  Device,
                  CacheBase,
                  i,                               // Cache Set number
                  0,                               // head pointer
                  (uint16_t)(RC_RecordCount - 1)); // tail pointer

        // Set Hash Table start
        // This is an offset from EIP207_CS_RAM_XS_SPACE_BASE
        // in record administration memory words
        EIP207_RC_PARAMS2_WR(Device,
                             CacheBase,
                             i,
                             (uint16_t)RC_RecordCount,
    /* Small Record size */  fArc4 ? 0 : (uint16_t)RecordWordCount,
                             FIRMWARE_EIP207_RC_DMA_WR_COMB_DLY);

        // Select the highest clock count as specified by
        // the Host and the Firmware for the FRC
#ifdef FIRMWARE_EIP207_CS_BLOCK_NEXT_COMMAND_LOGIC_DISABLE
        ClocksPerTick = RC_p->BlockClockCount;
#else
        {
            uint8_t tmp;

            tmp = (uint8_t)FIRMWARE_EIP207_CS_FRC_BLOCK_TIMEBASE;
            ClocksPerTick = RC_p->BlockClockCount >= tmp ?
                                       CacheConf_p->FRC.BlockClockCount : tmp;
        }
#endif

        // Take Record Cache out of reset
        EIP207_RC_PARAMS_WR(Device,
                            CacheBase,
                            i,     // Cache Set number
                            false, // Disable cache RAM access
                            RC_p->fNonBlock,
                            false, // Disable access cache administration RAM
                            RC_HashTableSize,
                            ClocksPerTick,
                            false, // Not used here for this HW
                            RC_Record2_WordCount); // Large record size

        RC_p++;
    } // for, i cache sets

    return EIP207_GLOBAL_NO_ERROR;
}

static void
EIP207_RC_Internal_DebugStatistics_Single_Get(
        const Device_Handle_t Device,
        const unsigned int CacheSetId,
        uint32_t RegBase,
        EIP207_Global_CacheDebugStatistics_t * const Stats_p)
{
    EIP207_RC_LONGCTR_RD(Device, EIP207_RC_REG_PREFEXEC(RegBase, CacheSetId),
                         &Stats_p->PrefetchExec);
    EIP207_RC_LONGCTR_RD(Device, EIP207_RC_REG_PREFBLCK(RegBase, CacheSetId),
                         &Stats_p->PrefetchBlock);
    EIP207_RC_LONGCTR_RD(Device, EIP207_RC_REG_PREFDMA(RegBase, CacheSetId),
                         &Stats_p->PrefetchDMA);
    EIP207_RC_LONGCTR_RD(Device, EIP207_RC_REG_SELOPS(RegBase, CacheSetId),
                         &Stats_p->SelectOps);
    EIP207_RC_LONGCTR_RD(Device, EIP207_RC_REG_SELDMA(RegBase, CacheSetId),
                         &Stats_p->SelectDMA);
    EIP207_RC_LONGCTR_RD(Device, EIP207_RC_REG_IDMAWR(RegBase, CacheSetId),
                         &Stats_p->IntDMAWrite);
    EIP207_RC_LONGCTR_RD(Device, EIP207_RC_REG_XDMAWR(RegBase, CacheSetId),
                         &Stats_p->ExtDMAWrite);
    EIP207_RC_LONGCTR_RD(Device, EIP207_RC_REG_INVCMD(RegBase, CacheSetId),
                         &Stats_p->InvalidateOps);
    EIP207_RC_RDMAERRFLGS_RD(Device, RegBase, CacheSetId,
                             &Stats_p->ReadDMAErrFlags);
    EIP207_RC_SHORTCTR_RD(Device, EIP207_RC_REG_RDMAERR(RegBase, CacheSetId),
                          &Stats_p->ReadDMAErrors);
    EIP207_RC_SHORTCTR_RD(Device, EIP207_RC_REG_WDMAERR(RegBase, CacheSetId),
                          &Stats_p->WriteDMAErrors);
    EIP207_RC_SHORTCTR_RD(Device, EIP207_RC_REG_INVECC(RegBase, CacheSetId),
                          &Stats_p->InvalidateECC);
    EIP207_RC_SHORTCTR_RD(Device, EIP207_RC_REG_DATECC_CORR(RegBase, CacheSetId),
                          &Stats_p->DataECCCorr);
    EIP207_RC_SHORTCTR_RD(Device, EIP207_RC_REG_ADMECC_CORR(RegBase, CacheSetId),
                          &Stats_p->AdminECCCorr);

}


void
EIP207_RC_Internal_DebugStatistics_Get(
        const Device_Handle_t Device,
        const unsigned int CacheSetId,
        EIP207_Global_CacheDebugStatistics_t * const FRC_Stats_p,
        EIP207_Global_CacheDebugStatistics_t * const TRC_Stats_p)
{
    EIP207_RC_Internal_DebugStatistics_Single_Get(
        Device,
        CacheSetId,
        EIP207_FRC_REG_BASE,
        FRC_Stats_p);
    EIP207_RC_Internal_DebugStatistics_Single_Get(
        Device,
        CacheSetId,
        EIP207_TRC_REG_BASE,
        TRC_Stats_p);
}

/*----------------------------------------------------------------------------
 * EIP207_RC_Internal_Status_Get
 */
void
EIP207_RC_Internal_Status_Get(
        const Device_Handle_t Device,
        const unsigned int CacheSetId,
        EIP207_Global_CacheStatus_t * const FRC_Status_p,
        EIP207_Global_CacheStatus_t * const TRC_Status_p,
        EIP207_Global_CacheStatus_t * const ARC4RC_Status_p)
{
    // Read FRC status
    EIP207_RC_PARAMS_RD(Device,
                        EIP207_FRC_REG_BASE,
                        CacheSetId,
                        &FRC_Status_p->fDMAReadError,
                        &FRC_Status_p->fDMAWriteError);

    EIP207_RC_ECCCTRL_RD_CLEAR(Device,
                               EIP207_FRC_REG_BASE,
                               CacheSetId,
                               &FRC_Status_p->fDataEccOflo,
                               &FRC_Status_p->fDataEccErr,
                               &FRC_Status_p->fAdminEccErr);

    // Read TRC status
    EIP207_RC_PARAMS_RD(Device,
                        EIP207_TRC_REG_BASE,
                        CacheSetId,
                        &TRC_Status_p->fDMAReadError,
                        &TRC_Status_p->fDMAWriteError);

    EIP207_RC_ECCCTRL_RD_CLEAR(Device,
                               EIP207_TRC_REG_BASE,
                               CacheSetId,
                               &TRC_Status_p->fDataEccOflo,
                               &TRC_Status_p->fDataEccErr,
                               &TRC_Status_p->fAdminEccErr);

    // Read ARC4RC status
    EIP207_RC_PARAMS_RD(Device,
                        EIP207_ARC4RC_REG_BASE,
                        CacheSetId,
                        &ARC4RC_Status_p->fDMAReadError,
                        &ARC4RC_Status_p->fDMAWriteError);

    EIP207_RC_ECCCTRL_RD_CLEAR(Device,
                               EIP207_ARC4RC_REG_BASE,
                               CacheSetId,
                               &ARC4RC_Status_p->fDataEccOflo,
                               &ARC4RC_Status_p->fDataEccErr,
                               &ARC4RC_Status_p->fAdminEccErr);
}


/* end of file eip207_rc_internal.c */
