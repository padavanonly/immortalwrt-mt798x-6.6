/* adapter_global drbg_init.c
 *
 * Initialize Global DRBG Control functionality.
 */

/*****************************************************************************
* Copyright (c) 2017-2020 by Rambus, Inc. and/or its subsidiaries.
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

#include "adapter_global_drbg_init.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_adapter_global.h"

// Global Control DRBG API
#include "api_global_eip74.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint8_t, uint32_t, bool

// Driver Framework C Library API
#include "clib.h"               // memcpy, ZEROINIT

// Log API
#include "log.h"

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */

static bool fDRBGPresent;

#ifdef MODULE
#include <linux/random.h>
#define Global_DRBG_Entropy_Get(p) get_random_bytes(p, 48);
#else
#include <stdio.h>
/*----------------------------------------------------------------------------
 * Global_DRBG_Entropy_Get
 *
 * Get 48 bytes of entropy to initialize/reseed DRBG.
 */
static void
Global_DRBG_Entropy_Get(
    uint8_t * Key_p)
{
    FILE *rng = fopen("/dev/urandom","rb");
    if (rng==NULL)
    {
        LOG_CRIT("/dev/urandom not available\n");
        return;
    }
    if (fread(Key_p, 1, 48, rng) < 48)
    {
        LOG_CRIT("random data not read\n");
        return;
    }
    Log_HexDump("Entropy",0,Key_p,48);

    fclose(rng);
}

#endif

/*----------------------------------------------------------------------------
 * BoolToString()
 *
 * Convert boolean value to string.
 */
static const char *
BoolToString(
        const bool b)
{
    if (b)
        return "true";
    else
        return "false";
}


/*----------------------------------------------------------------------------
 * Adapter_Global_DRBG_StatusReport()
 *
 * Obtain all available global status information from the Global DRBG
 * hardware and report it.
 */
void
Adapter_Global_DRBG_StatusReport(void)
{
    GlobalControl74_Error_t Rc;
    GlobalControl74_Status_t Status;

    LOG_INFO("DA_GC: Global_DRBG_StatusReport \n");

    LOG_CRIT("DA_GC: Global DRBG Status\n");
    Rc = GlobalControl74_Status_Get(&Status);
    if (Rc != GLOBAL_CONTROL_EIP74_NO_ERROR)
    {
        LOG_CRIT("EIP74 status get error\n");
        return;
    }
    Log_FormattedMessage(
        "EIP 74 status: GenBlockCount=%u StuckOut=%s\n"
        "\t\tNotInitialized=%s ReseedErr=%s ReseedWarn=%s\n"
        "\t\tInstantiated=%s AvailableCount=%u\n",
        Status.GenerateBlockCount,
        BoolToString(Status.fStuckOut),
        BoolToString(Status.fNotInitialized),
        BoolToString(Status.fReseedError),
        BoolToString(Status.fReseedWarning),
        BoolToString(Status.fInstantiated),
        Status.AvailableCount);
}


/*----------------------------------------------------------------------------
 * Adapter_Global_DRBG_Init()
 *
 */
bool
Adapter_Global_DRBG_Init(void)
{
    GlobalControl74_Error_t rc;
    GlobalControl74_Capabilities_t Capabilities;
    GlobalControl74_Configuration_t Configuration;
    uint8_t Entropy[48];

    LOG_INFO("DA_GC: Global_DRBG_Init \n");

    ZEROINIT(Configuration);
    Configuration.fStuckOut = true;

    Global_DRBG_Entropy_Get(Entropy);

    rc = GlobalControl74_Init(&Configuration, Entropy);
    if (rc == GLOBAL_CONTROL_EIP74_ERROR_NOT_IMPLEMENTED)
    {
        LOG_CRIT("EIP74 not present\n");
        return true;
    }
    if (rc == GLOBAL_CONTROL_EIP74_NO_ERROR)
    {
        fDRBGPresent = true;
        Log_FormattedMessage("EIP74 initialized OK\n");
    }
    else
    {
        LOG_CRIT("EIP74 initialization error\n");
    }

    Capabilities.szTextDescription[0] = 0;

    GlobalControl74_Capabilities_Get(&Capabilities);

    LOG_CRIT("DA_GC: Global DRBG capabilities: %s\n",
             Capabilities.szTextDescription);

    Adapter_Global_DRBG_StatusReport();

    return true;
}


/*----------------------------------------------------------------------------
 * Adapter_Global_DRBG_UnInit()
 *
 */
void
Adapter_Global_DRBG_UnInit(void)
{
    LOG_INFO("\n\t\t Adapter_Global_DRBG_UnInit \n");

    if (fDRBGPresent)
    {
        Adapter_Global_DRBG_StatusReport();

        GlobalControl74_UnInit();
    }
}


/* end of file adapter_global_drbg_init.c */
