/* eip207_ice.c
 *
 * EIP-207s Flow Look-Up Engine (FLUE) interface implementation
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// EIP-207s Flow Look-Up Engine (FLUE) interface
#include "eip207_flue.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip207_global.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // uint32_t, bool

// EIP-207 Global Control Driver Library Internal interfaces
#include "eip207_level0.h"              // EIP-207 Level 0 macros

// EIP-207 HW interface
#include "eip207_hw_interface.h"

// Driver Framework Device API
#include "device_types.h"               // Device_Handle_t

// EIP-207 Firmware Classification API
#include "firmware_eip207_api_cs.h"     // Classification API: General


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define EIP207_FLUE_3ENTRY_LOOKUP_MODE          1


/*----------------------------------------------------------------------------
 * EIP207_FLUE_Init
 */
void
EIP207_FLUE_Init(
        const Device_Handle_t Device,
        const unsigned int HashTableId,
        const EIP207_Global_FLUEConfig_t * const FLUEConf_p,
        const bool fARC4Present,
        const bool fLookupCachePresent)
{
    // Configure hash tables
    EIP207_FLUE_CONFIG_WR(Device,
                          HashTableId, // Hash Table ID
                          HashTableId, // Function,
                                       // set it equal to hash table id
                          0,           // Generation
                          true,        // Table enable
                          true);       // Access enable

    // Use Hash table 0 parameters, they must be the same for all the tables!
    // Initialize FLUE with EIP-207 Firmware Classification API parameters
    EIP207_FLUE_OFFSET_WR(Device,
                          FLUEConf_p->HashTable[HashTableId].fPrefetchXform,
                          fLookupCachePresent ?
                              FLUEConf_p->HashTable[HashTableId].fLookupCached :
                              false,
                          FIRMWARE_EIP207_CS_XFORM_RECORD_WORD_OFFSET);

    // Use Hash table 0 parameters, they must be the same for all the tables!
    // Check if ARC4 Record Cache is available
    if ( fARC4Present )
    {
        EIP207_FLUE_ARC4_OFFSET_WR(
                          Device,
                          FLUEConf_p->HashTable[HashTableId].fPrefetchARC4State,
                          EIP207_GLOBAL_FLUE_LOOKUP_MODE,
                          FIRMWARE_EIP207_CS_ARC4_RECORD_WORD_OFFSET);
    }
#if EIP207_GLOBAL_FLUE_LOOKUP_MODE == EIP207_FLUE_3ENTRY_LOOKUP_MODE
    else
    {
        EIP207_FLUE_ARC4_OFFSET_WR(Device,
                                   HashTableId,
                                   EIP207_GLOBAL_FLUE_LOOKUP_MODE,
                                   0);
    }
#endif // EIP207_GLOBAL_FLUE_LOOKUP_MODE == EIP207_FLUE_1ENTRY_LOOKUP_MODE

#ifdef EIP207_FLUE_HAVE_VIRTUALIZATION
    // Virtualisation support present, initialize the lookup table.
    // All interfaces refer to Table # 0.
    {
        unsigned int i;
        unsigned int c;
        unsigned int idx0,idx1,idx2,idx3;

        c = FLUEConf_p->InterfacesCount;
        if (c > EIP207_FLUE_MAX_NOF_INTERFACES_TO_USE)
            c = EIP207_FLUE_MAX_NOF_INTERFACES_TO_USE;

        for (i = 0; i < c; i += 4)
        {
            idx0 = FLUEConf_p->InterfaceIndex[i];

            if (i + 1 < c)
                idx1 = FLUEConf_p->InterfaceIndex[i + 1];
            else
                idx1 = 0;

            if (i + 2 < c)
                idx2 = FLUEConf_p->InterfaceIndex[i + 2];
            else
                idx2 = 0;

            if (i + 3 < c)
                idx3 = FLUEConf_p->InterfaceIndex[i + 3];
            else
                idx3 = 0;

            EIP207_FLUE_IFC_LUT_WR(Device,
                                   i / 4,
                                   idx0, idx1, idx2, idx3);
        }
    }
#endif

    return;
}


/*----------------------------------------------------------------------------
 * EIP207_FLUE_Status_Get
 */
void
EIP207_FLUE_Status_Get(
        const Device_Handle_t Device,
        EIP207_Global_FLUE_Status_t * const FLUE_Status_p)
{
    IDENTIFIER_NOT_USED(Device);

    FLUE_Status_p->Error1 = 0;
    FLUE_Status_p->Error2 = 0;

    return;
}


/* end of file eip207_flue.c */
