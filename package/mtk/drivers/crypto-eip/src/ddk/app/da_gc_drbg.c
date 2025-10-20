/* da_gc_drbg.c
 *
 * Demo Application for the DRBG Global Control API
 * Linux kernel-space and user-space.
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

// Demo Application Classification Engine Module API
#include "da_gc_ce.h"
#include "da_gc_drbg.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_da_gc_drbg.h"         // configuration switches

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint8_t, uint32_t, bool

// Global Control DRBG API
#include "api_global_eip74.h" // GlobalControl74_Init/Capabilities_Get/UnInit

// Driver Framework C Library API
#include "clib.h"               // memcpy, ZEROINIT

// Logging API
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


#ifdef DA_GLOBAL_USE_INTERRUPTS
/*----------------------------------------------------------------------------
 * Global_DRBG_Notify_Handler
 */
static void
DRBG_Notify_Handler(void)
{
    GlobalControl74_Error_t Rc;
    GlobalControl74_Status_t Status;
    uint8_t Entropy[48];
    Rc = GlobalControl74_Status_Get(&Status);
    LOG_INFO("DRBG_Notify_Handler\n");
    if (Rc != GLOBAL_CONTROL_EIP74_NO_ERROR)
    {
        LOG_CRIT("%s: EIP74 status get error\n",__func__);

    }
    LOG_INFO(
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
    if (Status.fReseedWarning)
    {
        Global_DRBG_Entropy_Get(Entropy);
        Rc = GlobalControl74_Reseed(Entropy);
        if (Rc == GLOBAL_CONTROL_EIP74_NO_ERROR)
        {
            LOG_INFO("EIP74 reseed OK\n");
        }
        else
        {
            LOG_CRIT("EIP74 reseed error\n");
        }
    }
    Rc = GlobalControl74_Clear();
    if (Rc == GLOBAL_CONTROL_EIP74_NO_ERROR)
    {
        LOG_INFO("EIP74 clear OK\n");
    }
    else
    {
        LOG_CRIT("EIP74 clear error\n");
    }

    GlobalControl74_Notify_Request(DRBG_Notify_Handler);
}
#endif

/*----------------------------------------------------------------------------
 * Global_DRBG_StatusReport()
 *
 * Obtain all available global status information from the Global DRBG
 * hardware and report it.
 */
void
Global_DRBG_StatusReport(void)
{
    GlobalControl74_Error_t Rc;
    GlobalControl74_Status_t Status;

    LOG_INFO("DA_GC: Global_Cs_StatusReport \n");

    if (!fDRBGPresent)
    {
        LOG_CRIT("EIP74 not present\n");
        return;
    }

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
 * Global_DRBG_Init()
 *
 */
bool
Global_DRBG_Init(void)
{
    GlobalControl74_Error_t rc;
    GlobalControl74_Capabilities_t Capabilities;
    GlobalControl74_Configuration_t Configuration;
    uint8_t Entropy[48];

    LOG_INFO("DA_GC: Global_DRBG_Init \n");

    ZEROINIT(Configuration);
    Configuration.GenerateBlockSize = DA_GC_GENERATE_BLOCK_SIZE;
    Configuration.ReseedThr = DA_GC_RESEED_THR;
    Configuration.ReseedThrEarly = DA_GC_RESEED_THR_EARLY;
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

    LOG_CRIT("DA_GC: Global Classification capabilities: %s\n",
             Capabilities.szTextDescription);

    Global_DRBG_StatusReport();
#ifdef DA_GLOBAL_USE_INTERRUPTS
    GlobalControl74_Notify_Request(DRBG_Notify_Handler);
#endif
    return true;
}


/*----------------------------------------------------------------------------
 * Global_DRBG_UnInit()
 *
 */
void
Global_DRBG_UnInit(void)
{
    LOG_INFO("DA_GC: Global_DRBG_UnInit \n");

    if (fDRBGPresent)
    {
        // DRBG Global Control status report
        Global_DRBG_StatusReport();

        // Uninitialize DRBG Global Control
        GlobalControl74_UnInit();
    }
}


/* end of file da_gc_drbg.c */
