/* adapter_global_cs_init.c
 *
 * Initialize Global Classification Control functionality.
 */

/*****************************************************************************
* Copyright (c) 2011-2021 by Rambus, Inc. and/or its subsidiaries.
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

#include "adapter_global_cs_init.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_adapter_cs.h"

// Global Control API
#include "api_global_eip97.h"

// Global Control Classification API
#include "api_global_eip207.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint8_t, uint32_t, bool

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

static const uint32_t Global_IV_Data[4] = ADAPTER_CS_IV;


/*----------------------------------------------------------------------------
 * YesNo
 *
 * Convert boolean value to string.
 */
static const char *
YesNo(
        const bool b)
{
    if (b)
        return "YES";
    else
        return "no";
}


/*----------------------------------------------------------------------------
 * Adapter_Global_Cs_StatusReport()
 *
 * Obtain all available global status information from the Global Classification
 * hardware and report it.
 */
static void
Adapter_Global_Cs_StatusReport(void)
{
    GlobalControl207_Error_t rc;
    unsigned int i;
    unsigned int NofCEs;

    LOG_INFO("\n\t\t Adapter_Global_Cs_StatusReport \n");

    LOG_CRIT("Global Classification Control Status\n");
    GlobalControl97_Interfaces_Get(&NofCEs, NULL, NULL, NULL);

    for (i = 0; i < NofCEs; i++)
    {
        GlobalControl207_Status_t CE_Status;
        GlobalControl207_GlobalStats_t CE_GlobalStats;
        GlobalControl207_Clock_t CE_Clock;

        ZEROINIT(CE_Status);
        ZEROINIT(CE_GlobalStats);
        ZEROINIT(CE_Clock);

        LOG_CRIT("Classification Engine %d status\n", i);

        rc = GlobalControl207_Status_Get(i, &CE_Status);
        if (rc != EIP207_GLOBAL_CONTROL_NO_ERROR)
            LOG_CRIT("%s: GlobalControl207_Status_Get() failed\n", __func__);
        else
        {
            if (CE_Status.ICE.fPUE_EccCorr          ||
                CE_Status.ICE.fPUE_EccDerr          ||
                CE_Status.ICE.fFPP_EccCorr          ||
                CE_Status.ICE.fFPP_EccDerr          ||
                CE_Status.ICE.fTimerOverflow        ||
                CE_Status.OCE.fPUE_EccCorr          ||
                CE_Status.OCE.fPUE_EccDerr          ||
                CE_Status.OCE.fFPP_EccCorr          ||
                CE_Status.OCE.fFPP_EccDerr          ||
                CE_Status.OCE.fTimerOverflow        ||
                CE_Status.FLUE.Error1 != 0          ||
                CE_Status.FLUE.Error2 != 0          ||
                CE_Status.FRC[0].fDMAReadError      ||
                CE_Status.FRC[0].fDMAWriteError     ||
                CE_Status.FRC[0].fDataEccOflo       ||
                CE_Status.FRC[0].fDataEccErr        ||
                CE_Status.FRC[0].fAdminEccErr       ||
                CE_Status.TRC[0].fDMAReadError      ||
                CE_Status.TRC[0].fDMAWriteError     ||
                CE_Status.TRC[0].fDataEccOflo       ||
                CE_Status.TRC[0].fDataEccErr        ||
                CE_Status.TRC[0].fAdminEccErr       ||
                CE_Status.ARC4RC[0].fDMAReadError   ||
                CE_Status.ARC4RC[0].fDMAWriteError  ||
                CE_Status.ARC4RC[0].fDataEccOflo    ||
                CE_Status.ARC4RC[0].fDataEccErr     ||
                CE_Status.ARC4RC[0].fAdminEccErr)
            {
                LOG_CRIT("%s: error(s) detected\n", __func__);
                LOG_CRIT(
                 "\tICE Pull-Up ECC Correctable Err:        %s\n"
                 "\tICE Pull-Up ECC Non-Corr Err:           %s\n"
                 "\tICE Post-Processor ECC Correctable Err: %s\n"
                 "\tICE Post-Processor ECC Non-Corr Err:    %s\n"
                 "\tICE Timer ovf detected:                 %s\n"
                 "\tOCE Pull-Up ECC Correctable Err:        %s\n"
                 "\tOCE Pull-Up ECC Non-Corr Err:           %s\n"
                 "\tOCE Post-Processor ECC Correctable Err: %s\n"
                 "\tOCE Post-Processor ECC Non-Corr Err:    %s\n"
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
            }
            else
                LOG_CRIT("%s: all OK\n", __func__);
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
        }

        rc = GlobalControl207_GlobalStats_Get(i, &CE_GlobalStats);
        if (rc != EIP207_GLOBAL_CONTROL_NO_ERROR)
            LOG_CRIT("Adapter_Global_Cs_StatusReport: "
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
            LOG_CRIT("Adapter_Global_Cs_StatusReport: "
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
 * Adapter_Global_Cs_Init()
 *
 */
bool
Adapter_Global_Cs_Init(void)
{
    GlobalControl207_Error_t rc;
    GlobalControl207_Capabilities_t Capabilities;
    GlobalControl207_IV_t Data;

    LOG_INFO("\n\t\t Adapter_Global_Cs_Init \n");

    Data.IV[0] = Global_IV_Data[0];
    Data.IV[1] = Global_IV_Data[1];
    Data.IV[2] = Global_IV_Data[2];
    Data.IV[3] = Global_IV_Data[3];

    // Request the classification firmware download during the initialization
    rc = GlobalControl207_Init(true, &Data);
    if (rc != EIP207_GLOBAL_CONTROL_NO_ERROR)
    {
        LOG_CRIT("Adaptar_Global_Init: Classification initialization failed\n");
        return false;
    }

    Capabilities.szTextDescription[0] = 0;

    GlobalControl207_Capabilities_Get(&Capabilities);

    LOG_CRIT("Global Classification capabilities: %s\n",
             Capabilities.szTextDescription);

    Adapter_Global_Cs_StatusReport();

    return true;
}


/*----------------------------------------------------------------------------
 * Adapter_Global_Cs_UnInit()
 *
 */
void
Adapter_Global_Cs_UnInit(void)
{
    LOG_INFO("\n\t\t Adapter_Global_Cs_UnInit \n");

    Adapter_Global_Cs_StatusReport();

    GlobalControl207_UnInit();
}


/* end of file adapter_global_cs_init.c */
