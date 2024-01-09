/* eip207_oce.c
 *
 * EIP-207c Output Classification Engine (OCE) interface implementation
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

// EIP-207c Output Classification Engine (OCE) interface
#include "eip207_oce.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip207_global.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // uint32_t, bool

// Driver Framework C Run-Time Library Abstraction API
#include "clib.h"                 // ZEROINIT

// EIP-207 Global Control Driver Library Internal interfaces
#include "eip207_level0.h"              // EIP-207 Level 0 macros

// Driver Framework Device API
#include "device_types.h"               // Device_Handle_t
#include "device_rw.h"                  // Read32, Write32

// EIP-207 Firmware Classification API
#include "firmware_eip207_api_cs.h"     // Classification API: General

// EIP-207 Firmware Download API
#include "firmware_eip207_api_dwld.h"   // Classification API: FW download

// EIP97_Interfaces_Get()
#include "eip97_global_internal.h"

// EIP97 Global init API
#include "eip97_global_init.h"

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Allow legacy firmware to be used, which does not have these constants.
#ifndef FIRMWARE_EIP207_DWLD_ADMIN_RAM_INPUT_LAST_BYTE_COUNT
#define FIRMWARE_EIP207_DWLD_ADMIN_RAM_INPUT_LAST_BYTE_COUNT FIRMWARE_EIP207_DWLD_ADMIN_RAM_LAST_BYTE_COUNT
#endif
#ifndef FIRMWARE_EIP207_CS_ADMIN_RAM_INPUT_LAST_BYTE_COUNT
#define FIRMWARE_EIP207_CS_ADMIN_RAM_INPUT_LAST_BYTE_COUNT FIRMWARE_EIP207_DWLD_ADMIN_RAM_INPUT_LAST_BYTE_COUNT
#endif

// Number fo words to check when reading firmware back.
#define EIP207_FW_CHECK_WORD_COUNT 16


/*----------------------------------------------------------------------------
 * EIP207_OCE_Firmware_Load
 */
EIP207_Global_Error_t
EIP207_OCE_Firmware_Load(
        const Device_Handle_t Device,
        const unsigned int TimerPrescaler,
        EIP207_Firmware_t * const PUE_Firmware_p,
        EIP207_Firmware_t * const FPP_Firmware_p)
{
    unsigned int NofCEs;

    if ((EIP97_SupportedFuncs_Get() & BIT_1) != 0)
    {
        EIP97_Interfaces_Get(&NofCEs,NULL,NULL,NULL);
        // Check if the Adapter provides the OPUE firmware image with correct size.
        if (!PUE_Firmware_p->Image_p ||
            PUE_Firmware_p->ImageWordCount > EIP207_OPUE_PROG_RAM_WORD_COUNT)
            return EIP207_GLOBAL_ARGUMENT_ERROR;

        // Check if the Adapter provides the OFPP firmware image with correct size.
        if (!FPP_Firmware_p->Image_p ||
            FPP_Firmware_p->ImageWordCount > EIP207_OPUE_PROG_RAM_WORD_COUNT)
            return EIP207_GLOBAL_ARGUMENT_ERROR;


        // Clear EIP-207c OCE Scratchpad RAM where the the firmware
        // administration data will be located
        {
            unsigned int i, CECount, BlockCount, RequiredRAMByteCount;

            RequiredRAMByteCount =
                MAX(FIRMWARE_EIP207_CS_ADMIN_RAM_INPUT_LAST_BYTE_COUNT,
                    FIRMWARE_EIP207_DWLD_ADMIN_RAM_INPUT_LAST_BYTE_COUNT);

            // Check if the administration RAM required by the EIP-207 firmware
            // fits into physically available scratchpad RAM
            if ((EIP207_OCE_SCRATCH_RAM_128B_BLOCK_COUNT * 128) <
                RequiredRAMByteCount)
                return EIP207_GLOBAL_ARGUMENT_ERROR;

            // Calculate how many 128-byte blocks are required for the firmware
            // administration data
            BlockCount = (RequiredRAMByteCount + 127) / 128;

            for (CECount = 0; CECount < NofCEs; CECount++)
            {
                // Make OCE Scratchpad RAM accessible and set the timer
                EIP207_OCE_SCRATCH_CTRL_WR(
                    Device,
                    CECount,
                    true, // Change timer
                    true, // Enable timer
                    (uint16_t)TimerPrescaler,
                    EIP207_OCE_SCRATCH_TIMER_OFLO_BIT, // Timer overflow bit
                    true, // Change access
                    (uint8_t)BlockCount);

#ifdef DEBUG
                // Check if the timer runs
                {
                    uint32_t Value32;
                    unsigned int i;

                    for (i = 0; i < 10; i++)
                    {
                        Value32 = Device_Read32(Device,
                                                EIP207_OCE_REG_TIMER_LO(CECount));

                        if (Value32 == 0)
                            return EIP207_GLOBAL_INTERNAL_ERROR;
                    }
                }
#endif

                // Write the OCE Scratchpad RAM with 0
                for(i = 0; i < (BlockCount * 32); i++)
                {
                    Device_Write32(Device,
                                   EIP207_OCE_REG_SCRATCH_RAM(CECount) +
                                   i * sizeof(uint32_t),
                                   0);
#ifdef DEBUG
                    // Perform read-back check
                    {
                        uint32_t Value32 =
                            Device_Read32(
                                Device,
                                EIP207_OCE_REG_SCRATCH_RAM(CECount) +
                                i * sizeof(uint32_t));

                        if (Value32 != 0)
                            return EIP207_GLOBAL_INTERNAL_ERROR;
                    }
#endif
                }
            }

            // Leave the scratchpad RAM accessible for the Host
        }

        // Download the firmware
        {
            unsigned int CECount;
#ifdef DEBUG
            unsigned int i;
#endif
            for (CECount = 0; CECount < NofCEs; CECount++)
            {
                // Reset the Input Flow Post-Processor micro-engine (OFPP) to make its
                // Program RAM accessible
                EIP207_OCE_FPP_CTRL_WR(Device,
                                       CECount,
                                       0,     // No start address for debug mode
                                       1,       // Clear ECC correctable error
                                       1,       // Clear ECC non-correctable error
                                       false, // Debug mode OFF
                                       true); // SW Reset ON

                // Enable access to OFPP Program RAM
                EIP207_OCE_RAM_CTRL_WR(Device, CECount, false, true);
            }

#ifdef DEBUG
            // Write the Input Flow post-Processor micro-Engine firmware
            for(i = 0; i < FPP_Firmware_p->ImageWordCount; i++)
            {
                Device_Write32(Device,
                               EIP207_CS_RAM_XS_SPACE_BASE + i * sizeof(uint32_t),
                               FPP_Firmware_p->Image_p[i]);
                // Perform read-back check
                {
                    uint32_t Value32 =
                        Device_Read32(
                            Device,
                           EIP207_CS_RAM_XS_SPACE_BASE + i * sizeof(uint32_t));

                    if (Value32 != FPP_Firmware_p->Image_p[i])
                        return EIP207_GLOBAL_INTERNAL_ERROR;
                }
            }
#else
            Device_Write32Array(Device,
                                EIP207_CS_RAM_XS_SPACE_BASE,
                                FPP_Firmware_p->Image_p,
                                FPP_Firmware_p->ImageWordCount);
            // Perform read-back check of small subset
            {
                static uint32_t ReadBuf[EIP207_FW_CHECK_WORD_COUNT];
                Device_Read32Array(Device,
                                   EIP207_CS_RAM_XS_SPACE_BASE,
                                   ReadBuf,
                                   EIP207_FW_CHECK_WORD_COUNT);
                if (memcmp(ReadBuf,
                           FPP_Firmware_p->Image_p,
                           EIP207_FW_CHECK_WORD_COUNT * sizeof(uint32_t)) != 0)
                    return EIP207_GLOBAL_INTERNAL_ERROR;

            }
#endif

            for (CECount = 0; CECount < NofCEs; CECount++)
            {
                // Disable access to OFPP Program RAM
                // Enable access to OPUE Program RAM
                EIP207_OCE_RAM_CTRL_WR(Device, CECount, true, false);

                // Reset the Input Pull-Up micro-Engine (OPUE) to make its
                // Program RAM accessible
                EIP207_OCE_PUE_CTRL_WR(Device,
                                       CECount,
                                       0,     // No start address for debug mode
                                       1,       // Clear ECC correctable error
                                       1,       // Clear ECC non-correctable error
                                   false, // Debug mode OFF
                                       true); // SW Reset ON
            }

#ifdef DEBUG
            // Write the Input Pull-Up micro-Engine firmware
            for(i = 0; i < PUE_Firmware_p->ImageWordCount; i++)
            {
                Device_Write32(Device,
                               EIP207_CS_RAM_XS_SPACE_BASE + i * sizeof(uint32_t),
                               PUE_Firmware_p->Image_p[i]);
                // Perform read-back check
                {
                    uint32_t Value32 =
                        Device_Read32(
                            Device,
                            EIP207_CS_RAM_XS_SPACE_BASE + i * sizeof(uint32_t));

                    if (Value32 != PUE_Firmware_p->Image_p[i])
                        return EIP207_GLOBAL_INTERNAL_ERROR;
                }
            }
#else
            Device_Write32Array(Device,
                                EIP207_CS_RAM_XS_SPACE_BASE,
                                PUE_Firmware_p->Image_p,
                                PUE_Firmware_p->ImageWordCount);
            // Perform read-back check of small subset
            {
                static uint32_t ReadBuf[EIP207_FW_CHECK_WORD_COUNT];
                Device_Read32Array(Device,
                                   EIP207_CS_RAM_XS_SPACE_BASE,
                                   ReadBuf,
                               EIP207_FW_CHECK_WORD_COUNT);
                if (memcmp(ReadBuf,
                           PUE_Firmware_p->Image_p,
                           EIP207_FW_CHECK_WORD_COUNT * sizeof(uint32_t)) != 0)
                    return EIP207_GLOBAL_INTERNAL_ERROR;

            }
#endif

            // Disable access to OPUE Program RAM
            for (CECount = 0; CECount < EIP207_GLOBAL_MAX_NOF_CE_TO_USE; CECount++)
                EIP207_OCE_RAM_CTRL_WR(Device, CECount, false, false);

#ifdef EIP207_GLOBAL_FIRMWARE_DOWNLOAD_VERSION_CHECK
            // Check the firmware version and start all the engines
            for (CECount = 0; CECount < NofCEs; CECount++)
            {
                uint32_t Value32;
                unsigned int Ma, Mi, Pl, i;
                bool fUpdated;

                // Start the OFPP in Debug mode for the firmware version check
                EIP207_OCE_FPP_CTRL_WR(
                    Device,
                    CECount,
                    FIRMWARE_EIP207_DWLD_OFPP_VERSION_CHECK_DBG_PROG_CNTR,
                    1,       // Clear ECC correctable error
                    1,       // Clear ECC non-correctable error
                    true,    // Debug mode ON
                    false);  // SW Reset OFF

                // Wait for the OFPP version update
                for (i = 0; i < EIP207_FW_VER_CHECK_MAX_NOF_READ_ATTEMPTS; i++)
                {
                    Value32 = Device_Read32(
                        Device,
                        EIP207_OCE_REG_SCRATCH_RAM(CECount) +
                        FIRMWARE_EIP207_DWLD_ADMIN_RAM_OFPP_CTRL_BYTE_OFFSET);

                    FIRMWARE_EIP207_DWLD_OFPP_VersionUpdated_Read(Value32,
                                                                  &fUpdated);

                    if (fUpdated)
                        break;
                }

                if (!fUpdated)
                    return EIP207_GLOBAL_INTERNAL_ERROR;

                Value32 = Device_Read32(
                    Device,
                    EIP207_OCE_REG_SCRATCH_RAM(CECount) +
                    FIRMWARE_EIP207_DWLD_ADMIN_RAM_OFPP_VERSION_BYTE_OFFSET);

                FIRMWARE_EIP207_DWLD_Version_Read(Value32, &Ma, &Mi, &Pl);

                if (FPP_Firmware_p->Major == 0 &&
                    FPP_Firmware_p->Minor == 0 &&
                    FPP_Firmware_p->PatchLevel == 0)
                {  // Adapter did not provide expected version, return it.
                    FPP_Firmware_p->Major = Ma;
                    FPP_Firmware_p->Minor = Mi;
                    FPP_Firmware_p->PatchLevel = Pl;
                }
                else
                {
                    // Adapter provided expected version, check it.
                    if (FPP_Firmware_p->Major != Ma ||
                        FPP_Firmware_p->Minor != Mi ||
                        FPP_Firmware_p->PatchLevel != Pl)
                        return EIP207_GLOBAL_INTERNAL_ERROR;
                }

                // Start the OPUE in Debug mode for the firmware version check
                EIP207_OCE_PUE_CTRL_WR(
                    Device,
                    CECount,
                    FIRMWARE_EIP207_DWLD_OPUE_VERSION_CHECK_DBG_PROG_CNTR,
                    1,       // Clear ECC correctable error
                    1,       // Clear ECC non-correctable error
                    true,    // Debug mode ON
                    false);  // SW Reset OFF

                // Wait for the OPUE version update
                for (i = 0; i < EIP207_FW_VER_CHECK_MAX_NOF_READ_ATTEMPTS; i++)
                {
                    Value32 = Device_Read32(
                        Device,
                        EIP207_OCE_REG_SCRATCH_RAM(CECount) +
                        FIRMWARE_EIP207_DWLD_ADMIN_RAM_OPUE_CTRL_BYTE_OFFSET);

                    FIRMWARE_EIP207_DWLD_OPUE_VersionUpdated_Read(Value32,
                                                                  &fUpdated);

                    if (fUpdated)
                        break;
                }

                if (!fUpdated)
                    return EIP207_GLOBAL_INTERNAL_ERROR;

                Value32 = Device_Read32(
                    Device,
                     EIP207_OCE_REG_SCRATCH_RAM(CECount) +
                    FIRMWARE_EIP207_DWLD_ADMIN_RAM_OPUE_VERSION_BYTE_OFFSET);

                FIRMWARE_EIP207_DWLD_Version_Read(Value32, &Ma, &Mi, &Pl);

                if (PUE_Firmware_p->Major == 0 &&
                    PUE_Firmware_p->Minor == 0 &&
                PUE_Firmware_p->PatchLevel == 0)
                {  // Adapter did not provide expected version, return it.
                    PUE_Firmware_p->Major = Ma;
                    PUE_Firmware_p->Minor = Mi;
                    PUE_Firmware_p->PatchLevel = Pl;
                }
                else
                {
                    // Adapter provided expected version, check it.
                    if (PUE_Firmware_p->Major != Ma ||
                    PUE_Firmware_p->Minor != Mi ||
                        PUE_Firmware_p->PatchLevel != Pl)
                        return EIP207_GLOBAL_INTERNAL_ERROR;
                }
            } // for
#else
            for (CECount = 0; CECount < EIP207_GLOBAL_MAX_NOF_CE_TO_USE; CECount++)
            {
            // Start the OFPP in Debug mode
                EIP207_OCE_FPP_CTRL_WR(
                    Device,
                    CECount,
                    FIRMWARE_EIP207_DWLD_OFPP_VERSION_CHECK_DBG_PROG_CNTR,
                    1,       // Clear ECC correctable error
                    1,       // Clear ECC non-correctable error
                    true,    // Debug mode ON
                    false);  // SW Reset OFF

                // Start the OPUE in Debug mode
                EIP207_OCE_PUE_CTRL_WR(
                    Device,
                    CECount,
                    FIRMWARE_EIP207_DWLD_OPUE_VERSION_CHECK_DBG_PROG_CNTR,
                    1,       // Clear ECC correctable error
                    1,       // Clear ECC non-correctable error
                            true,    // Debug mode ON
                    false);  // SW Reset OFF
            } // for
#endif // EIP207_GLOBAL_FIRMWARE_DOWNLOAD_VERSION_CHECK
        }
    }
    return EIP207_GLOBAL_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP207_OCE_GlobalStats_Get
 */
EIP207_Global_Error_t
EIP207_OCE_GlobalStats_Get(
        const Device_Handle_t Device,
        const unsigned int CE_Number,
        EIP207_Global_OCE_GlobalStats_t * const OCE_GlobalStats_p)
{
    IDENTIFIER_NOT_USED(Device);
    IDENTIFIER_NOT_USED(CE_Number);

    // Not used / implemented yet
    ZEROINIT(*OCE_GlobalStats_p);

    return EIP207_GLOBAL_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP207_OCE_ClockCount_Get
 */
EIP207_Global_Error_t
EIP207_OCE_ClockCount_Get(
        const Device_Handle_t Device,
        const unsigned int CE_Number,
        EIP207_Global_Value64_t * const OCE_Clock_p)
{
    IDENTIFIER_NOT_USED(Device);
    IDENTIFIER_NOT_USED(CE_Number);

    // Not used / implemented yet
    ZEROINIT(*OCE_Clock_p);

    return EIP207_GLOBAL_NO_ERROR;
}


/*-----------------------------------------------------------------------------
 * EIP207_OCE_Status_Get
 */
EIP207_Global_Error_t
EIP207_OCE_Status_Get(
        const Device_Handle_t Device,
        const unsigned int CE_Number,
        EIP207_Global_CE_Status_t * const OCE_Status_p)
{
    if ((EIP97_SupportedFuncs_Get() & BIT_1) != 0)
    {
        EIP207_OCE_PUE_CTRL_RD_CLEAR(Device,
                                     CE_Number,
                                     &OCE_Status_p->fPUE_EccCorr,
                                     &OCE_Status_p->fPUE_EccDerr);

        EIP207_OCE_FPP_CTRL_RD_CLEAR(Device,
                                     CE_Number,
                                     &OCE_Status_p->fFPP_EccCorr,
                                     &OCE_Status_p->fFPP_EccDerr);

        EIP207_OCE_SCRATCH_CTRL_RD(Device,
                                   CE_Number,
                                   &OCE_Status_p->fTimerOverflow);
    }
    return EIP207_GLOBAL_NO_ERROR;
}


/* end of file eip207_oce.c */
