/* eip74.c
 *
 * Implementation of the EIP-74 Driver Library.
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

// EIP76 initialization API
#include "eip74.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip74.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint32_t

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t

// EIP-76 Driver Library Internal interfaces
#include "eip74_level0.h"       // Level 0 macros

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// I/O Area, used internally
typedef struct
{
    Device_Handle_t Device;
} EIP74_True_IOArea_t;

#define TEST_SIZEOF(type, size) \
    extern int size##_must_bigger[1 - 2*((int)(sizeof(type) > size))]


#ifdef EIP74_STRICT_ARGS
#define EIP74_CHECK_POINTER(_p) \
    if (NULL == (_p)) \
        return EIP74_ARGUMENT_ERROR;
#define EIP74_CHECK_INT_INRANGE(_i, _min, _max) \
    if ((_i) < (_min) || (_i) > (_max)) \
        return EIP74_ARGUMENT_ERROR;
#define EIP74_CHECK_INT_ATLEAST(_i, _min) \
    if ((_i) < (_min)) \
        return EIP74_ARGUMENT_ERROR;
#define EIP74_CHECK_INT_ATMOST(_i, _max) \
    if ((_i) > (_max)) \
        return EIP74_ARGUMENT_ERROR;
#else
/* EIP74_STRICT_ARGS undefined */
#define EIP74_CHECK_POINTER(_p)
#define EIP74_CHECK_INT_INRANGE(_i, _min, _max)
#define EIP74_CHECK_INT_ATLEAST(_i, _min)
#define EIP74_CHECK_INT_ATMOST(_i, _max)
#endif /*end of EIP74_STRICT_ARGS */


// validate the size of the fake and real IOArea structures
TEST_SIZEOF(EIP74_True_IOArea_t, EIP74_IOAREA_REQUIRED_SIZE);


/*----------------------------------------------------------------------------
 * EIP74Lib_Detect
 *
 * Checks the presence of EIP-74 device. Returns true when found.
 */
static bool
EIP74Lib_Detect(
        const Device_Handle_t Device)
{
    uint32_t Value;

    Value = EIP74_Read32(Device, EIP74_REG_VERSION);
    if (!EIP74_REV_SIGNATURE_MATCH( Value ))
        return false;

    return true;
}


/*----------------------------------------------------------------------------
 * EIP74Lib_Reset_IsDone
 */
static bool
EIP74Lib_Reset_IsDone(
        const Device_Handle_t Device)
{
    bool fReady, fPSAIWriteOK, fStuckOut, fEarlyReseed;
    bool fTestReady,fGenPktError, fInstantiated, fTestStuckOut, fNeedClock;
    uint8_t BlocksAvailable;

    EIP74_STATUS_RD(Device,
                    &fReady,
                    &fPSAIWriteOK,
                    &fStuckOut,
                    &fEarlyReseed,
                    &fTestReady,
                    &fGenPktError,
                    &fInstantiated,
                    &fTestStuckOut,
                    &BlocksAvailable,
                    &fNeedClock);

    return fPSAIWriteOK;
}


/*----------------------------------------------------------------------------
 * EIP74_Init
 */
EIP74_Error_t
EIP74_Init(
        EIP74_IOArea_t * const IOArea_p,
        const Device_Handle_t Device)
{
    volatile EIP74_True_IOArea_t * const TrueIOArea_p =
        (EIP74_True_IOArea_t *)IOArea_p;
    EIP74_CHECK_POINTER(IOArea_p);

    TrueIOArea_p->Device = Device;

    if (!EIP74Lib_Detect(Device))
    {
        return EIP74_UNSUPPORTED_FEATURE_ERROR;
    }

    return EIP74_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP74_Reset
 */
EIP74_Error_t
EIP74_Reset(
        EIP74_IOArea_t * const IOArea_p)
{
    Device_Handle_t Device;
    volatile EIP74_True_IOArea_t * const TrueIOArea_p =
        (EIP74_True_IOArea_t *)IOArea_p;
    EIP74_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    EIP74_CONTROL_WR(Device,
                     false, /* fReadyMask */
                     false, /* fStuckOut */
                     false, /* fTestMode */
                     false, /* fHostMode */
                     false, /* fEnableDRBG */
                     false, /* fForceStuckOut */
                     false, /* fRequestdata */
                     0); /* DataBlocks */

    if (EIP74Lib_Reset_IsDone(Device))
    {
        return EIP74_NO_ERROR;
    }
    else
    {
        return EIP74_BUSY_RETRY_LATER;
    }
}


/*----------------------------------------------------------------------------
 * EIP74_Reset_IsDone
 */
EIP74_Error_t
EIP74_Reset_IsDone(
        EIP74_IOArea_t * const IOArea_p)
{
    Device_Handle_t Device;
    volatile EIP74_True_IOArea_t * const TrueIOArea_p =
        (EIP74_True_IOArea_t *)IOArea_p;
    EIP74_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    if (EIP74Lib_Reset_IsDone(Device))
    {
        return EIP74_NO_ERROR;
    }
    else
    {
        return EIP74_BUSY_RETRY_LATER;
    }
}


/*----------------------------------------------------------------------------
 * EIP74_HWRevision_Get
  */
EIP74_Error_t
EIP74_HWRevision_Get(
        const Device_Handle_t Device,
        EIP74_Capabilities_t * const Capabilities_p)
{
    EIP74_CHECK_POINTER(Capabilities_p);

    EIP74_VERSION_RD(Device,
                     &Capabilities_p->HW_Revision.EipNumber,
                     &Capabilities_p->HW_Revision.ComplmtEipNumber,
                     &Capabilities_p->HW_Revision.HWPatchLevel,
                     &Capabilities_p->HW_Revision.MinHWRevision,
                     &Capabilities_p->HW_Revision.MajHWRevision);
    EIP74_OPTIONS_RD(Device,
                     &Capabilities_p->HW_Options.ClientCount,
                     &Capabilities_p->HW_Options.AESCoreCount,
                     &Capabilities_p->HW_Options.AESSpeed,
                     &Capabilities_p->HW_Options.FIFODepth);

    return EIP74_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP74_Configure
 */
EIP74_Error_t
EIP74_Configure(
        EIP74_IOArea_t * const IOArea_p,
        const EIP74_Configuration_t * const Configuration_p)
{
    Device_Handle_t Device;
    volatile EIP74_True_IOArea_t * const TrueIOArea_p =
        (EIP74_True_IOArea_t *)IOArea_p;
    EIP74_CHECK_POINTER(IOArea_p);
    EIP74_CHECK_POINTER(Configuration_p);
    EIP74_CHECK_INT_INRANGE(Configuration_p->GenerateBlockSize, 1, 4095);

    Device = TrueIOArea_p->Device;

    EIP74_GEN_BLK_SIZE_WR(Device, Configuration_p->GenerateBlockSize);
    EIP74_RESEED_THR_WR(Device, Configuration_p->ReseedThr);
    EIP74_RESEED_THR_EARLY_WR(Device, Configuration_p->ReseedThrEarly);

    return EIP74_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP74_Instantiate
 */
EIP74_Error_t
EIP74_Instantiate(
        EIP74_IOArea_t * const IOArea_p,
        const uint32_t * const Entropy_p,
        bool fStuckOut)
{
    Device_Handle_t Device;
    volatile EIP74_True_IOArea_t * const TrueIOArea_p =
        (EIP74_True_IOArea_t *)IOArea_p;
    EIP74_CHECK_POINTER(IOArea_p);
    EIP74_CHECK_POINTER(Entropy_p);

    Device = TrueIOArea_p->Device;

    EIP74_PS_AI_WR(Device, Entropy_p, 12);

    EIP74_CONTROL_WR(Device,
                     false, /* fReadyMask */
                     fStuckOut,
                     false, /* fTestMode */
                     false, /* fHostMode */
                     true, /* fEnableDRBG */
                     false, /* fForceStuckOut */
                     false, /* fRequestdata */
                     0); /* DataBlocks */

    return EIP74_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP74_Reseed
 */
EIP74_Error_t
EIP74_Reseed(
        EIP74_IOArea_t * const IOArea_p,
        const uint32_t * const Entropy_p)
{
    Device_Handle_t Device;
    volatile EIP74_True_IOArea_t * const TrueIOArea_p =
        (EIP74_True_IOArea_t *)IOArea_p;
    EIP74_CHECK_POINTER(IOArea_p);
    EIP74_CHECK_POINTER(Entropy_p);

    Device = TrueIOArea_p->Device;

    EIP74_PS_AI_WR(Device, Entropy_p, 12);

    return EIP74_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP74_Status_Get
 */
EIP74_Error_t
EIP74_Status_Get(
        EIP74_IOArea_t * const IOArea_p,
        EIP74_Status_t * const Status_p)
{
    Device_Handle_t Device;
    volatile EIP74_True_IOArea_t * const TrueIOArea_p =
        (EIP74_True_IOArea_t *)IOArea_p;
    bool fReady, fPSAIWriteOK, fStuckOut, fEarlyReseed;
    bool fTestReady,fGenPktError, fInstantiated, fTestStuckOut, fNeedClock;
    uint8_t BlocksAvailable;
    uint32_t GenBlockCount, ReseedThr;
    EIP74_CHECK_POINTER(IOArea_p);
    EIP74_CHECK_POINTER(Status_p);

    Device = TrueIOArea_p->Device;

    EIP74_STATUS_RD(Device,
                    &fReady,
                    &fPSAIWriteOK,
                    &fStuckOut,
                    &fEarlyReseed,
                    &fTestReady,
                    &fGenPktError,
                    &fInstantiated,
                    &fTestStuckOut,
                    &BlocksAvailable,
                    &fNeedClock);

    EIP74_GENERATE_CNT_RD(Device, &GenBlockCount);
    EIP74_RESEED_THR_RD(Device, &ReseedThr);

    Status_p->GenerateBlockCount = GenBlockCount;
    Status_p->fStuckOut = fStuckOut;
    Status_p->fNotInitialized = fGenPktError;
    Status_p->fReseedError = GenBlockCount == ReseedThr;
    Status_p->fReseedWarning = fEarlyReseed;
    Status_p->fInstantiated = fInstantiated;
    Status_p->AvailableCount = BlocksAvailable;

    return EIP74_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP74_Clear
 */
EIP74_Error_t
EIP74_Clear(
        EIP74_IOArea_t * const IOArea_p)
{
    Device_Handle_t Device;
    volatile EIP74_True_IOArea_t * const TrueIOArea_p =
        (EIP74_True_IOArea_t *)IOArea_p;
    EIP74_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    EIP74_INTACK_WR(Device, false, true, false);

    return EIP74_NO_ERROR;
}


/* end of file eip74.c */
