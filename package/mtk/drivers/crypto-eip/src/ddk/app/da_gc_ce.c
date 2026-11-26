/* da_gc_ce.c
 *
 * Demo Application for the Packet Classification Global Control API
 * Linux kernel-space and user-space.
 */

/*****************************************************************************
* Copyright (c) 2012-2020 by Rambus, Inc. and/or its subsidiaries.
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


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_da_gc_ce.h"         // configuration switches

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint8_t, uint32_t, bool

// Global Control API
#include "api_global_eip97.h" // GlobalControl207_Init/Capabilities_Get/UnInit

// Global Control Classification API
#include "api_global_eip207.h" // GlobalControl207_Init/Capabilities_Get/UnInit

// Global Control Classification Status API
#include "api_global_status_eip207.h"

// Driver Framework C Library API
#include "clib.h"               // memcpy, ZEROINIT

#include "device_types.h"       // Device_Handle_t
#include "device_mgmt.h"        // Device_find
#include "log.h"                // Log API


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */

static const uint32_t Global_IV_Data[4] = DEMOAPP_GLOBAL_CS_IV;


/*----------------------------------------------------------------------------
 * YesNo
 *
 * Convert boolean value to string.
 */
static char *
YesNo(
        const bool b)
{
    if (b)
        return "Yes";
    else
        return "No";
}


/*----------------------------------------------------------------------------
 * Global_Cs_StatusReport()
 *
 * Obtain all available global status information from the Global Classification
 * hardware and report it.
 */
void
Global_Cs_StatusReport(void)
{
    GlobalControl207_Error_t rc;
    unsigned int i;
    unsigned int NofCE;

    LOG_INFO("DA_GC: Global_Cs_StatusReport \n");

    LOG_CRIT("DA_GC: Global Classification Control Status\n");

    GlobalControl97_Interfaces_Get(&NofCE, NULL, NULL, NULL);

    for (i = 0; i < NofCE; i++)
    {
        GlobalControl207_Status_t CE_Status;
        GlobalControl207_GlobalStats_t CE_GlobalStats;
        GlobalControl207_Clock_t CE_Clock;

        ZEROINIT(CE_Status);
        ZEROINIT(CE_GlobalStats);
        ZEROINIT(CE_Clock);

        LOG_CRIT("DA_GC: Classification Engine %d status\n", i);

        rc = GlobalControl207_Status_Get(i, &CE_Status);
        if (rc != EIP207_GLOBAL_CONTROL_NO_ERROR)
            LOG_CRIT("DA_GC: Global_Cs_StatusReport: "
                     "GlobalControl207_Status_Get() failed\n");
        else
            LOG_CRIT(
                 "\tICE Pull-Up Correctable Err:            %s\n"
                 "\tICE Pull-Up Non-correctable Err:        %s\n"
                 "\tICE Post-Processor Correctable Err:     %s\n"
                 "\tICE Post-Processor Non-correctable Err: %s\n"
                 "\tICE Timer ovf detected:                 %s\n"
                 "\tOCE Pull-Up Correctable Err:            %s\n"
                 "\tOCE Pull-Up Non-correctable Err:        %s\n"
                 "\tOCE Post-Processor Correctable Err:     %s\n"
                 "\tOCE Post-Processor Non-correctable Err: %s\n"
                 "\tOCE Timer ovf detected:                 %s\n"
                 "\tFLUE Err 1:                             %s (mask=0x%08x)\n"
                 "\tFLUE Err 2:                             %s (mask=0x%08x)\n"
                 "\tFRC#0 DMA Read Err:                     %s\n"
                 "\tFRC#0 DMA Write Err:                    %s\n"
                 "\tFRC#0 Data RAM ECC Non-Corr Ovf Err:    %s\n"
                 "\tFRC#0 Data RAM ECC Non-Corr Err:        %s\n"
                 "\tFRC#0 Admin RAM ECC Non-Corr Err:       %s\n"
                 "\tTRC#0 DMA Read Err:                     %s\n"
                 "\tTRC#0 DMA Write Err:                    %s\n"
                 "\tTRC#0 Data RAM ECC Non-Corr Ovf Err:    %s\n"
                 "\tTRC#0 Data RAM ECC Non-Corr Err:        %s\n"
                 "\tTRC#0 Admin RAM ECC Non-Corr Err:       %s\n"
                 "\tARC4RC#0 DMA Read Err:                  %s\n"
                 "\tARC4RC#0 DMA Write Err:                 %s\n"
                 "\tARC4RC#0 Data RAM ECC Non-Corr Ovf Err: %s\n"
                 "\tARC4RC#0 Data RAM ECC Non-Corr Err:     %s\n"
                 "\tARC4RC#0 Admin RAM ECC Non-Corr Err:    %s\n\n",
                 YesNo(CE_Status.ICE.fPUE_EccCorr),
                 YesNo(CE_Status.ICE.fPUE_EccDerr),
                 YesNo(CE_Status.ICE.fFPP_EccCorr),
                 YesNo(CE_Status.ICE.fFPP_EccDerr),
                 YesNo(CE_Status.ICE.fTimerOverflow),
                 YesNo(CE_Status.OCE.fPUE_EccCorr),
                 YesNo(CE_Status.OCE.fPUE_EccDerr),
                 YesNo(CE_Status.OCE.fFPP_EccCorr),
                 YesNo(CE_Status.OCE.fFPP_EccDerr),
                 YesNo(CE_Status.OCE.fTimerOverflow),
                 YesNo(CE_Status.FLUE.Error1 != 0),
                 CE_Status.FLUE.Error1,
                 YesNo(CE_Status.FLUE.Error2 != 0),
                 CE_Status.FLUE.Error2,
                 YesNo(CE_Status.FRC[0].fDMAReadError),
                 YesNo(CE_Status.FRC[0].fDMAWriteError),
                 YesNo(CE_Status.FRC[0].fDataEccOflo),
                 YesNo(CE_Status.FRC[0].fDataEccErr),
                 YesNo(CE_Status.FRC[0].fAdminEccErr),
                 YesNo(CE_Status.TRC[0].fDMAReadError),
                 YesNo(CE_Status.TRC[0].fDMAWriteError),
                 YesNo(CE_Status.TRC[0].fDataEccOflo),
                 YesNo(CE_Status.TRC[0].fDataEccErr),
                 YesNo(CE_Status.TRC[0].fAdminEccErr),
                 YesNo(CE_Status.ARC4RC[0].fDMAReadError),
                 YesNo(CE_Status.ARC4RC[0].fDMAWriteError),
                 YesNo(CE_Status.ARC4RC[0].fDataEccOflo),
                 YesNo(CE_Status.ARC4RC[0].fDataEccErr),
                 YesNo(CE_Status.ARC4RC[0].fAdminEccErr));
        if (CE_Status.FRCStats[0].PrefetchExec ||
            CE_Status.FRCStats[0].PrefetchBlock ||
            CE_Status.FRCStats[0].PrefetchDMA ||
            CE_Status.FRCStats[0].SelectOps ||
            CE_Status.FRCStats[0].SelectDMA ||
            CE_Status.FRCStats[0].IntDMAWrite ||
            CE_Status.FRCStats[0].ExtDMAWrite ||
            CE_Status.FRCStats[0].InvalidateOps)
        {
            LOG_CRIT("FRC statistics:\n"
                     "\tPrefetches executed: %u\n"
                     "\tPrefetches blocked:  %u\n"
                     "\tPrefetches with DMA: %u\n"
                     "\tSelect ops:          %u\n"
                     "\tSelect ops with DMA: %u\n"
                     "\tInternal DMA writes: %u\n"
                     "\tExternal DMA writes: %u\n"
                     "\tInvalidate ops:      %u\n"
                     "\tDMA err flags        0x%x\n"
                     "\tRead DMA errs:       %u\n"
                     "\tWrite DMA errs:      %u\n"
                     "\tECC invalidates:     %u\n"
                     "\tECC Data RAM Corr:   %u\n"
                     "\tECC Admin RAM Corr:  %u\n",
                     (uint32_t)CE_Status.FRCStats[0].PrefetchExec,
                     (uint32_t)CE_Status.FRCStats[0].PrefetchBlock,
                     (uint32_t)CE_Status.FRCStats[0].PrefetchDMA,
                     (uint32_t)CE_Status.FRCStats[0].SelectOps,
                     (uint32_t)CE_Status.FRCStats[0].SelectDMA,
                     (uint32_t)CE_Status.FRCStats[0].IntDMAWrite,
                     (uint32_t)CE_Status.FRCStats[0].ExtDMAWrite,
                     (uint32_t)CE_Status.FRCStats[0].InvalidateOps,
                     (uint32_t)CE_Status.FRCStats[0].ReadDMAErrFlags,
                     (uint32_t)CE_Status.FRCStats[0].ReadDMAErrors,
                     (uint32_t)CE_Status.FRCStats[0].WriteDMAErrors,
                     (uint32_t)CE_Status.FRCStats[0].InvalidateECC,
                     (uint32_t)CE_Status.FRCStats[0].DataECCCorr,
                     (uint32_t)CE_Status.FRCStats[0].AdminECCCorr);
        }
        if (CE_Status.TRCStats[0].PrefetchExec ||
            CE_Status.TRCStats[0].PrefetchBlock ||
            CE_Status.TRCStats[0].PrefetchDMA ||
            CE_Status.TRCStats[0].SelectOps ||
            CE_Status.TRCStats[0].SelectDMA ||
            CE_Status.TRCStats[0].IntDMAWrite ||
            CE_Status.TRCStats[0].ExtDMAWrite ||
            CE_Status.TRCStats[0].InvalidateOps)
        {
            LOG_CRIT("TRC statistics:\n"
                     "\tPrefetches executed: %u\n"
                     "\tPrefetches blocked:  %u\n"
                     "\tPrefetches with DMA: %u\n"
                     "\tSelect ops:          %u\n"
                     "\tSelect ops with DMA: %u\n"
                     "\tInternal DMA writes: %u\n"
                     "\tExternal DMA writes: %u\n"
                     "\tInvalidate ops:      %u\n"
                     "\tDMA err flags        0x%x\n"
                     "\tRead DMA errs:       %u\n"
                     "\tWrite DMA errs:      %u\n"
                     "\tECC invalidates:     %u\n"
                     "\tECC Data RAM Corr:   %u\n"
                     "\tECC Admin RAM Corr:  %u\n",
                     (uint32_t)CE_Status.TRCStats[0].PrefetchExec,
                     (uint32_t)CE_Status.TRCStats[0].PrefetchBlock,
                     (uint32_t)CE_Status.TRCStats[0].PrefetchDMA,
                     (uint32_t)CE_Status.TRCStats[0].SelectOps,
                     (uint32_t)CE_Status.TRCStats[0].SelectDMA,
                     (uint32_t)CE_Status.TRCStats[0].IntDMAWrite,
                     (uint32_t)CE_Status.TRCStats[0].ExtDMAWrite,
                     (uint32_t)CE_Status.TRCStats[0].InvalidateOps,
                     (uint32_t)CE_Status.TRCStats[0].ReadDMAErrFlags,
                     (uint32_t)CE_Status.TRCStats[0].ReadDMAErrors,
                     (uint32_t)CE_Status.TRCStats[0].WriteDMAErrors,
                     (uint32_t)CE_Status.TRCStats[0].InvalidateECC,
                     (uint32_t)CE_Status.TRCStats[0].DataECCCorr,
                     (uint32_t)CE_Status.TRCStats[0].AdminECCCorr);
        }

        rc = GlobalControl207_GlobalStats_Get(i, &CE_GlobalStats);
        if (rc != EIP207_GLOBAL_CONTROL_NO_ERROR)
            LOG_CRIT("DA_GC: Global_Cs_StatusReport: "
                     "GlobalControl207_GlobalStats_Get() failed\n");
        else
            LOG_CRIT(
                 "\tICE Dropped Packets Counter (low 32-bits):     0x%08x\n"
                 "\tICE Dropped Packets Counter (high 32-bits):    0x%08x\n"
                 "\tICE Inbound Packets Counter:                   0x%08x\n"
                 "\tICE Outbound Packets Counter:                  0x%08x\n"
                 "\tICE Inbound Octets Counter (low 32-bits):      0x%08x\n"
                 "\tICE Inbound Octets Counter (high 32-bits):     0x%08x\n"
                 "\tICE Outbound Octets Counter (low 32-bits):     0x%08x\n"
                 "\tICE Outbound Octets Counter (high 32-bits):    0x%08x\n"
                 "\tOCE Dropped Packets Counter (low 32-bits):     0x%08x\n"
                 "\tOCE Dropped Packets Counter (high 32-bits):    0x%08x\n\n",
                 CE_GlobalStats.ICE.DroppedPacketsCounter.Value64_Lo,
                 CE_GlobalStats.ICE.DroppedPacketsCounter.Value64_Hi,
                 CE_GlobalStats.ICE.InboundPacketsCounter,
                 CE_GlobalStats.ICE.OutboundPacketCounter,
                 CE_GlobalStats.ICE.InboundOctetsCounter.Value64_Lo,
                 CE_GlobalStats.ICE.InboundOctetsCounter.Value64_Hi,
                 CE_GlobalStats.ICE.OutboundOctetsCounter.Value64_Lo,
                 CE_GlobalStats.ICE.OutboundOctetsCounter.Value64_Hi,
                 CE_GlobalStats.OCE.DroppedPacketsCounter.Value64_Lo,
                 CE_GlobalStats.OCE.DroppedPacketsCounter.Value64_Hi);

        rc = GlobalControl207_ClockCount_Get(i, &CE_Clock);
        if (rc != EIP207_GLOBAL_CONTROL_NO_ERROR)
            LOG_CRIT("DA_GC: Global_Cs_StatusReport: "
                     "GlobalControl207_ClockCount_Get() failed\n");
        else
            LOG_CRIT(
                 "\tICE Clock Count (low 32-bits):   0x%08x\n"
                 "\tICE Clock Count (high 32-bits):  0x%08x\n"
                 "\tOCE Clock Count (low 32-bits):   0x%08x\n"
                 "\tOCE Clock Count (high 32-bits):  0x%08x\n\n",
                 CE_Clock.ICE.Value64_Lo,
                 CE_Clock.ICE.Value64_Hi,
                 CE_Clock.OCE.Value64_Lo,
                 CE_Clock.OCE.Value64_Hi);
    } // for
}


/*----------------------------------------------------------------------------
 * Global_Cs_Init()
 *
 */
bool
Global_Cs_Init(void)
{
    GlobalControl207_Error_t rc;
    GlobalControl207_Capabilities_t Capabilities;
    GlobalControl207_IV_t Data;

    LOG_INFO("DA_GC: Global_Cs_Init \n");

    Data.IV[0] = Global_IV_Data[0];
    Data.IV[1] = Global_IV_Data[1];
    Data.IV[2] = Global_IV_Data[2];
    Data.IV[3] = Global_IV_Data[3];

    // Request the classification firmware download during the initialization
    rc = GlobalControl207_Init(true, &Data);
    if (rc != EIP207_GLOBAL_CONTROL_NO_ERROR)
    {
        LOG_CRIT("DA_GC: Global_Cs_Init, "
                 "classification initialization failed\n");
        return false;
    }

    Capabilities.szTextDescription[0] = 0;

    GlobalControl207_Capabilities_Get(&Capabilities);

    LOG_CRIT("DA_GC: Global Classification capabilities: %s\n",
             Capabilities.szTextDescription);

    Global_Cs_StatusReport();

    return true;
}


/*----------------------------------------------------------------------------
 * Global_Cs_UnInit()
 *
 */
void
Global_Cs_UnInit(void)
{
    LOG_INFO("DA_GC: Global_Cs_UnInit \n");

    // Packet Classification Global Control status report
    Global_Cs_StatusReport();

    // Uninitialize Packet Classification Global Control
    GlobalControl207_UnInit();
}


/* end of file da_gc_ce.c */
