/* shdevxs_prng.c
 *
 * Shared Device Access PRNG reseed
 * Linux kernel space
 */

/*****************************************************************************
* Copyright (c) 2012-2022 by Rambus, Inc. and/or its subsidiaries.
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "shdevxs_prng.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "c_shdevxs.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // IDENTIFIER_NOT_USED

// Linux kernel API
#include <linux/module.h>

#include "shdevxs_kernel_internal.h"

#include "eip96_level0.h"

#include "eip97_global_level0.h"

// Runtime Power Management Device Macros API
#include "rpm_device_macros.h"  // RPM_*

#include "log.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */
EXPORT_SYMBOL(SHDevXS_PRNG_Reseed);
EXPORT_SYMBOL(SHDevXS_SupportedFuncs_Get);

/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * SHDevXS_PRNG_Ressed
 */
int
SHDevXS_PRNG_Reseed(
         SHDevXS_PRNG_Reseed_t * const Reseed_p)
{
    unsigned int i;

    {
        uint8_t dummy1, dummy2, dummy3, dummy4, dummy5;
        bool dummy6, dummy7, central_prng;

        EIP97_OPTIONS_RD(SHDevXS_Device,
                         &dummy1,
                         &dummy2,
                         &dummy3,
                         &dummy4,
                         &dummy5,
                         &central_prng,
                         &dummy6,
                         &dummy7);
        if (central_prng)
            return -1;
    }

    // No RPM callbacks used
    if (RPM_DEVICE_INIT_START_MACRO(SHDEVXS_PRNG_RPM_DEVICE_ID, 0, 0) !=
                                                              RPM_SUCCESS)
        return -1; // error

    for (i = 0; i < SHDEVXS_NOF_PE_TO_USE; i++)
    {
        EIP96_PRNG_CTRL_WR(SHDevXS_Device,
                           i, // EIP-96 PE number
                           false,     // Disable PRNG
                           false);    // Set PRNG Manual mode

        // Write new seed data
        EIP96_PRNG_SEED_L_WR(SHDevXS_Device, i,
                             Reseed_p->SeedLo);
        EIP96_PRNG_SEED_H_WR(SHDevXS_Device, i,
                             Reseed_p->SeedHi);

        // Write new key data
        EIP96_PRNG_KEY_0_L_WR(SHDevXS_Device, i,
                              Reseed_p->Key0Lo);
        EIP96_PRNG_KEY_0_H_WR(SHDevXS_Device, i,
                              Reseed_p->Key0Hi);
        EIP96_PRNG_KEY_1_L_WR(SHDevXS_Device, i,
                              Reseed_p->Key1Lo);
        EIP96_PRNG_KEY_1_H_WR(SHDevXS_Device, i,
                              Reseed_p->Key1Hi);

        // Write new LFSR data
        EIP96_PRNG_LFSR_L_WR(SHDevXS_Device, i,
                             Reseed_p->LFSRLo);
        EIP96_PRNG_LFSR_H_WR(SHDevXS_Device, i,
                             Reseed_p->LFSRHi);

        EIP96_PRNG_CTRL_WR(SHDevXS_Device,
                           i, // EIP-96 PE number
                           true,      // Enable PRNG
                           true);     // Set PRNG Auto mode
    } // for

    (void)RPM_DEVICE_INIT_STOP_MACRO(SHDEVXS_PRNG_RPM_DEVICE_ID);

    (void)RPM_DEVICE_UNINIT_START_MACRO(SHDEVXS_PRNG_RPM_DEVICE_ID, false);
    (void)RPM_DEVICE_UNINIT_STOP_MACRO(SHDEVXS_PRNG_RPM_DEVICE_ID);

    return 0;
}



/*----------------------------------------------------------------------------
 * SHDevXS_SupportedFuncs_Get
 */
unsigned int
SHDevXS_SupportedFuncs_Get(void)
{
    uint32_t v = Device_Read32(SHDevXS_Device, EIP96_REG_OPTIONS(0));
    uint32_t v2 = Device_Read32(SHDevXS_Device, EIP97_REG_OPTIONS);
    if ((v2 & BIT_30) != 0)
    {
        v |= BIT_3;
    }
    if ((v2 & BIT_25) != 0)
    {
        v |= BIT_2;
    }
    if ((v2 & BIT_24) != 0)
    {
        v |= BIT_1;
    }
    return v;
}


/* end of file shdevxs_prng.c */
