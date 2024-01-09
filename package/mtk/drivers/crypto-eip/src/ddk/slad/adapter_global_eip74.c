/* api_global_eip74.c
 *
 * Deterministic Random Bit Generator (EIP-74) Global Control Initialization
 * Adapter. The EIP-74 is used to generate pseudo-random IVs for outbound
 * operations in CBC mode.
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
#include "api_global_eip74.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Configuration.
#include "c_adapter_eip74.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // bool, uint8_t

// memcpy
#include "clib.h"

// EIP-73 Driver Library.
#include "eip74.h"

#include "device_types.h"       // Device_Handle_t
#include "device_mgmt.h"        // Device_find

// Logging API
#include "log.h"                // Log_*, LOG_*

// Adapter interrupts API
#include "adapter_interrupts.h" // Adapter_Interrupt_*

// Runtime Power Management Device Macros API
#include "rpm_device_macros.h"  // RPM_*


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */

/* Put all adapter local variables in one structure */
static struct {
    EIP74_IOArea_t IOArea;
#ifdef ADAPTER_EIP74_INTERRUPTS_ENABLE
    GlobalControl74_NotifyFunction_t Notify_CBFunc;
#endif
#ifdef ADAPTER_PEC_RPM_EIP74_DEVICE0_ID
    GlobalControl74_Configuration_t CachedConfig;
    bool fInitialized;
#ifdef ADAPTER_EIP74_INTERRUPTS_ENABLE
    bool fInterruptEnabled;
#endif
#endif
} EIP74State;


static const  GlobalControl74_Capabilities_t Global_CapabilitiesString =
{
  "EIP-74 v_._p_"// szTextDescription
};


/*----------------------------------------------------------------------------
 * GlobalControl74Lib_Init
 */
static GlobalControl74_Error_t
GlobalControl74Lib_Init(
        const GlobalControl74_Configuration_t * const Configuration_p,
        const uint8_t * const Entropy_p);


#ifdef ADAPTER_PEC_RPM_EIP74_DEVICE0_ID
/*----------------------------------------------------------------------------
 * GlobalControl74Lib_Resune
 */
static int
GlobalControl74Lib_Resune(
        void *p)
{
    uint8_t Entropy[48];
    IDENTIFIER_NOT_USED(p);
    if (EIP74State.fInitialized)
    {
        /* Note we should add fresh random data here */
        ZEROINIT(Entropy);
        if (GlobalControl74Lib_Init(&EIP74State.CachedConfig, Entropy) !=
            GLOBAL_CONTROL_EIP74_NO_ERROR)
        {
            return -1;
        }

#ifdef ADAPTER_EIP74_INTERRUPTS_ENABLE
        if (EIP74State.fInitialized && EIP74State.fInterruptEnabled)
        {
            Adapter_Interrupt_Disable(ADAPTER_EIP74_ERR_IRQ, 0);
            Adapter_Interrupt_Disable(ADAPTER_EIP74_RES_IRQ, 0);
        }
#endif
    }
    return 0;
}


/*----------------------------------------------------------------------------
 * GlobalControl74Lib_Suspend
 */
static int
GlobalControl74Lib_Suspend(
        void *p)
{
    IDENTIFIER_NOT_USED(p);
#ifdef ADAPTER_EIP74_INTERRUPTS_ENABLE
    if (EIP74State.fInitialized && EIP74State.fInterruptEnabled)
    {
        Adapter_Interrupt_Disable(ADAPTER_EIP74_ERR_IRQ, 0);
        Adapter_Interrupt_Disable(ADAPTER_EIP74_RES_IRQ, 0);
    }
#endif
    return 0;
}

#endif

/*----------------------------------------------------------------------------
 * GlobalControl74Lib_CopyKeyMat
 *
 * Copy a key represented as a byte array into a word array..
 *
 * Destination_p (input)
 *   Destination (word-aligned) of the word array
 *
 * Source_p (input)
 *   Source (byte aligned) of the data.
 *
 * KeyByteCount (input)
 *   Size of the key in bytes.
 *
 * Destination_p is allowed to be a null pointer, in which case no key
 * will be written.
 */
static void
GlobalControl74Lib_CopyKeyMat(
        uint32_t * const Destination_p,
        const uint8_t * const Source_p,
        const unsigned int KeyByteCount)
{
    uint32_t *dst = Destination_p;
    const uint8_t *src = Source_p;
    unsigned int i,j;
    uint32_t w;
    if (Destination_p == NULL)
        return;
    for(i=0; i < KeyByteCount / sizeof(uint32_t); i++)
    {
        w=0;
        for(j=0; j<sizeof(uint32_t); j++)
            w=(w<<8)|(*src++);
        *dst++ = w;
    }
}

#ifdef ADAPTER_EIP74_INTERRUPTS_ENABLE
/*----------------------------------------------------------------------------
 * GlobalControl74_InterruptHandlerNotify
 */
static void
GlobalControl74_InterruptHandlerNotify(
        const int nIRQ,
        const unsigned int flags)
{
    GlobalControl74_NotifyFunction_t CB_Func = EIP74State.Notify_CBFunc;

    IDENTIFIER_NOT_USED(nIRQ);
    IDENTIFIER_NOT_USED(flags);

    LOG_INFO("GlobalControl74_InterruptHandlerNotify\n");
#ifdef ADAPTER_PEC_RPM_EIP74_DEVICE0_ID
    EIP74State.fInterruptEnabled = true;
#endif

    EIP74State.Notify_CBFunc = NULL;
    if (CB_Func != NULL)
    {
        LOG_INFO("\t Invoking callback\n");
        CB_Func();
    }
}
#endif
/*----------------------------------------------------------------------------
 * GlobalControl74Lib_Init
 *
 * This function performs the initialization of the EIP-74 Deterministic
 * Random Bit Generator.
 *
 * Note: the Device was already found and the IOArea is already initialized.
 *
 * Configuration_p (input)
 *     Configuration parameters of the DRBG.
 *
 * Entropy_p (input)
 *     Pointer to a string of exactly 48 bytes that serves as the entropy.
 *     to initialize the DRBG.
 *
 * Return value
 *     GLOBAL_CONTROL_EIP74_NO_ERROR : initialization performed successfully
 *     GLOBAL_CONTROL_EIP74_ERROR_INTERNAL : initialization failed
 */
static GlobalControl74_Error_t
GlobalControl74Lib_Init(
        const GlobalControl74_Configuration_t * const Configuration_p,
        const uint8_t * const Entropy_p)
{
    EIP74_Error_t Rc;
    EIP74_Configuration_t Conf;
    unsigned LoopCounter = ADAPTER_EIP74_RESET_MAX_RETRIES;
    uint32_t Entropy[12];

    Rc = EIP74_Reset(&EIP74State.IOArea);
    do
    {
        if (Rc == EIP74_BUSY_RETRY_LATER)
        {
            LoopCounter--;
            if (LoopCounter == 0)
            {
                LOG_CRIT("%s EIP74 reset timed out\n",__func__);
                return GLOBAL_CONTROL_EIP74_ERROR_INTERNAL;
            }
            Rc = EIP74_Reset_IsDone(&EIP74State.IOArea);
        }
        else if (Rc != EIP74_NO_ERROR)
        {
            LOG_CRIT("%s EIP74 reset error\n",__func__);
            return GLOBAL_CONTROL_EIP74_ERROR_INTERNAL;
        }
    } while (Rc != EIP74_NO_ERROR);

    if (Configuration_p->GenerateBlockSize == 0)
    {
        Conf.GenerateBlockSize =  ADAPTER_EIP74_GEN_BLK_SIZE;
    }
    else
    {
        Conf.GenerateBlockSize = Configuration_p->GenerateBlockSize;
    }

    if (Configuration_p->ReseedThr == 0)
    {
        Conf.ReseedThr =  ADAPTER_EIP74_RESEED_THR;
    }
    else
    {
        Conf.ReseedThr = Configuration_p->ReseedThr;
    }

    if (Configuration_p->ReseedThrEarly == 0)
    {
        Conf.ReseedThrEarly =  ADAPTER_EIP74_RESEED_THR_EARLY;
    }
    else
    {
        Conf.ReseedThrEarly = Configuration_p->ReseedThrEarly;
    }

    Rc = EIP74_Configure(&EIP74State.IOArea, &Conf);
    if (Rc != EIP74_NO_ERROR)
    {
        LOG_CRIT("%s EIP74 could not be configured\n",__func__);
        return GLOBAL_CONTROL_EIP74_ERROR_INTERNAL;
    }

    GlobalControl74Lib_CopyKeyMat(Entropy, Entropy_p, 48);

    Rc = EIP74_Instantiate(&EIP74State.IOArea, Entropy, Configuration_p->fStuckOut);

    if (Rc != EIP74_NO_ERROR)
    {
        LOG_CRIT("%s EIP74 could not be instantiated\n",__func__);
        return GLOBAL_CONTROL_EIP74_ERROR_INTERNAL;
    }

    return GLOBAL_CONTROL_EIP74_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * GlobalControl74_Capabilities_Get
 */
void
GlobalControl74_Capabilities_Get(
        GlobalControl74_Capabilities_t * const Capabilities_p)
{
    Device_Handle_t Device;
    uint8_t Versions[3];
    LOG_INFO("\n\t\t\t GlobalControl74_Capabilities_Get\n");

    memcpy(Capabilities_p, &Global_CapabilitiesString,
           sizeof(Global_CapabilitiesString));

    Device = Device_Find(ADAPTER_EIP74_DEVICE_NAME);
    if (Device == NULL)
    {
        LOG_CRIT("%s EIP74 Device not found\n",__func__);
        return;
    }

    {
        EIP74_Capabilities_t Capabilities;
        EIP74_Error_t Rc;

        if (RPM_DEVICE_IO_START_MACRO(ADAPTER_GLOBAL_RPM_EIP74_DEVICE_ID,
                                RPM_FLAG_SYNC) != RPM_SUCCESS)
            return;

        Rc = EIP74_HWRevision_Get(Device, &Capabilities);

        (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP74_DEVICE_ID,
                                       RPM_FLAG_ASYNC);

        if (Rc != EIP74_NO_ERROR)
        {
            LOG_CRIT("%s EIP74_Capaboilities_Get() failed\n",__func__);
            return;
        }

        Log_FormattedMessage(
            "EIP74 options: Nof Clients=%u Nof AESCores=%u\n"
            "\t\tAESSpeed=%u FIFODepth=%u\n",
            Capabilities.HW_Options.ClientCount,
            Capabilities.HW_Options.AESCoreCount,
            Capabilities.HW_Options.AESSpeed,
            Capabilities.HW_Options.FIFODepth);


        Versions[0] = Capabilities.HW_Revision.MajHWRevision;
        Versions[1] = Capabilities.HW_Revision.MinHWRevision;
        Versions[2] = Capabilities.HW_Revision.HWPatchLevel;
    }

    {
        char * p = Capabilities_p->szTextDescription;
        int VerIndex = 0;
        int i = 0;

        while(p[i])
        {
            if (p[i] == '_' && VerIndex < 3)
            {
                if (Versions[VerIndex] > 9)
                    p[i] = '?';
                else
                    p[i] = '0' + Versions[VerIndex];

                VerIndex++;
            }

            i++;
        }
    }
}


/*----------------------------------------------------------------------------
 * GlobalControl74_Init
 */
GlobalControl74_Error_t
GlobalControl74_Init(
        const GlobalControl74_Configuration_t * const Configuration_p,
        const uint8_t * const Entropy_p)
{
    EIP74_Error_t Rc;
    Device_Handle_t Device;

    LOG_INFO("\n\t\t\t GlobalControl74_Init\n");

    Device = Device_Find(ADAPTER_EIP74_DEVICE_NAME);
    if (Device == NULL)
    {
        LOG_CRIT("%s EIP74 Device not found\n",__func__);
        return GLOBAL_CONTROL_EIP74_ERROR_NOT_IMPLEMENTED;
    }


    Rc = EIP74_Init(&EIP74State.IOArea, Device);
    if (Rc != EIP74_NO_ERROR)
    {
        LOG_CRIT("%s EIP74 could not be initialized\n",__func__);
        return GLOBAL_CONTROL_EIP74_ERROR_NOT_IMPLEMENTED;
    }

    if (RPM_DEVICE_INIT_START_MACRO(ADAPTER_GLOBAL_RPM_EIP74_DEVICE_ID,
                                    GlobalControl74Lib_Suspend,
                                    GlobalControl74Lib_Resume) != RPM_SUCCESS)
        return GLOBAL_CONTROL_EIP74_ERROR_INTERNAL;

    if (GlobalControl74Lib_Init(Configuration_p,Entropy_p) !=
        GLOBAL_CONTROL_EIP74_NO_ERROR)
    {
        (void)RPM_DEVICE_INIT_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP74_DEVICE_ID);

        return GLOBAL_CONTROL_EIP74_ERROR_INTERNAL;
    }


#ifdef ADAPTER_PEC_RPM_EIP74_DEVICE0_ID
    EIP74State.fInitialized = true;
    EIP74State.CachedConfig = *Configuration_p;
#endif
    (void)RPM_DEVICE_INIT_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP74_DEVICE_ID);


#ifdef ADAPTER_EIP74_INTERRUPTS_ENABLE
    Adapter_Interrupt_SetHandler(ADAPTER_EIP74_ERR_IRQ,
                                 GlobalControl74_InterruptHandlerNotify);
    Adapter_Interrupt_SetHandler(ADAPTER_EIP74_RES_IRQ,
                                 GlobalControl74_InterruptHandlerNotify);
#endif

    return GLOBAL_CONTROL_EIP74_NO_ERROR;
}



/*----------------------------------------------------------------------------
 * GlobalControl74_UnInit
 */
GlobalControl74_Error_t
GlobalControl74_UnInit(void)
{
    EIP74_Error_t Rc;
    unsigned LoopCounter = ADAPTER_EIP74_RESET_MAX_RETRIES;

    LOG_INFO("\n\t\t\t GlobalControl74_UnInit\n");

#ifdef ADAPTER_EIP74_INTERRUPTS_ENABLE
    Adapter_Interrupt_SetHandler(ADAPTER_EIP74_ERR_IRQ, NULL);
    Adapter_Interrupt_SetHandler(ADAPTER_EIP74_RES_IRQ, NULL);
    Adapter_Interrupt_Disable(ADAPTER_EIP74_ERR_IRQ, 0);
    Adapter_Interrupt_Disable(ADAPTER_EIP74_RES_IRQ, 0);
#endif
    (void)RPM_DEVICE_UNINIT_START_MACRO(ADAPTER_GLOBAL_RPM_EIP74_DEVICE_ID, false);

    Rc = EIP74_Reset(&EIP74State.IOArea);
    do
    {
        if (Rc == EIP74_BUSY_RETRY_LATER)
        {
            LoopCounter--;
            if (LoopCounter == 0)
            {
                LOG_CRIT("%s EIP74 reset timed out\n",__func__);
                (void)RPM_DEVICE_UNINIT_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP74_DEVICE_ID);
                return GLOBAL_CONTROL_EIP74_ERROR_INTERNAL;
            }
            Rc = EIP74_Reset_IsDone(&EIP74State.IOArea);
        }
        else if (Rc != EIP74_NO_ERROR)
        {
            LOG_CRIT("%s EIP74 reset error\n",__func__);
            (void)RPM_DEVICE_UNINIT_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP74_DEVICE_ID);

            return GLOBAL_CONTROL_EIP74_ERROR_INTERNAL;
        }
    } while (Rc != EIP74_NO_ERROR);
    (void)RPM_DEVICE_UNINIT_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP74_DEVICE_ID);

#ifdef ADAPTER_PEC_RPM_EIP74_DEVICE0_ID
    EIP74State.fInitialized = false;
#ifdef ADAPTER_EIP74_INTERRUPTS_ENABLE
    EIP74State.fInterruptEnabled = false;
#endif
#endif

    return GLOBAL_CONTROL_EIP74_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * GlobalControl74_Reseed
 */
GlobalControl74_Error_t
GlobalControl74_Reseed(
        const uint8_t * const Entropy_p)
{
    EIP74_Error_t Rc;
    uint32_t Entropy[12];
    LOG_INFO("\n\t\t\t GlobalControl74_Reseed\n");


    GlobalControl74Lib_CopyKeyMat(Entropy, Entropy_p, 48);

    if (RPM_DEVICE_IO_START_MACRO(ADAPTER_GLOBAL_RPM_EIP74_DEVICE_ID,
                                  RPM_FLAG_SYNC) != RPM_SUCCESS)
        return GLOBAL_CONTROL_EIP74_ERROR_INTERNAL;

    Rc = EIP74_Reseed(&EIP74State.IOArea, Entropy);

    (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP74_DEVICE_ID,
                                   RPM_FLAG_ASYNC);

    if (Rc != EIP74_NO_ERROR)
    {
        LOG_CRIT("%s EIP74 could not be reseeded\n",__func__);
        return GLOBAL_CONTROL_EIP74_ERROR_INTERNAL;
    }

    return GLOBAL_CONTROL_EIP74_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * GlobalControl74_Status_Get
 */
GlobalControl74_Error_t
GlobalControl74_Status_Get(
        GlobalControl74_Status_t * const Status_p)
{
    EIP74_Error_t Rc;
    EIP74_Status_t Status;
    LOG_INFO("\n\t\t\t GlobalControl74_Status_Get\n");

    if (RPM_DEVICE_IO_START_MACRO(ADAPTER_GLOBAL_RPM_EIP74_DEVICE_ID,
                                  RPM_FLAG_SYNC) != RPM_SUCCESS)
        return GLOBAL_CONTROL_EIP74_ERROR_INTERNAL;

    Rc = EIP74_Status_Get(&EIP74State.IOArea, &Status);

    (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP74_DEVICE_ID,
                                   RPM_FLAG_ASYNC);

    if (Rc != EIP74_NO_ERROR)
    {
        LOG_CRIT("%s EIP74 status could not be read\n",__func__);
        return GLOBAL_CONTROL_EIP74_ERROR_INTERNAL;
    }

    Status_p->GenerateBlockCount = Status.GenerateBlockCount;
    Status_p->fStuckOut = Status.fStuckOut;
    Status_p->fNotInitialized = Status.fNotInitialized;
    Status_p->fReseedError = Status.fReseedError;
    Status_p->fReseedWarning = Status.fReseedWarning;
    Status_p->fInstantiated = Status.fInstantiated;
    Status_p->AvailableCount = Status.AvailableCount;

    return GLOBAL_CONTROL_EIP74_NO_ERROR;
}

/*----------------------------------------------------------------------------
 * GlobalControl74_Clear
 */
GlobalControl74_Error_t
GlobalControl74_Clear(void)
{
    EIP74_Error_t Rc;
    LOG_INFO("\n\t\t\t GlobalControl74_Clear\n");

    if (RPM_DEVICE_IO_START_MACRO(ADAPTER_GLOBAL_RPM_EIP74_DEVICE_ID,
                                  RPM_FLAG_SYNC) != RPM_SUCCESS)
        return GLOBAL_CONTROL_EIP74_ERROR_INTERNAL;

    Rc = EIP74_Clear(&EIP74State.IOArea);

    (void)RPM_DEVICE_IO_STOP_MACRO(ADAPTER_GLOBAL_RPM_EIP74_DEVICE_ID,
                                   RPM_FLAG_ASYNC);

    if (Rc != EIP74_NO_ERROR)
    {
        LOG_CRIT("%s EIP74 could not be cleared\n",__func__);
        return GLOBAL_CONTROL_EIP74_ERROR_INTERNAL;
    }

    return GLOBAL_CONTROL_EIP74_NO_ERROR;
}


#ifdef ADAPTER_EIP74_INTERRUPTS_ENABLE
/*----------------------------------------------------------------------------
 * GlobalControl74_Notify_Request
 */
GlobalControl74_Error_t
GlobalControl74_Notify_Request(
        GlobalControl74_NotifyFunction_t CBFunc_p)
{
    LOG_INFO("\n\t\t\t GlobalControl74_Notify_Request\n");
    IDENTIFIER_NOT_USED(CBFunc_p);

    EIP74State.Notify_CBFunc = CBFunc_p;
    if (CBFunc_p != NULL)
    {
        Adapter_Interrupt_Enable(ADAPTER_EIP74_ERR_IRQ, 0);
        Adapter_Interrupt_Enable(ADAPTER_EIP74_RES_IRQ, 0);
#ifdef ADAPTER_PEC_RPM_EIP74_DEVICE0_ID
        EIP74State.fInterruptEnabled = true;
#endif
    }

    return GLOBAL_CONTROL_EIP74_NO_ERROR;
}
#endif


/* end of file adapter_global_eip74.c */
