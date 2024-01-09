/* adapter_interrupts.c
 *
 * Adapter EIP-202 module responsible for interrupts.
 *
 */

/*****************************************************************************
* Copyright (c) 2008-2022 by Rambus, Inc. and/or its subsidiaries.
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

#include "adapter_interrupts.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default Adapter configuration
#include "c_adapter_eip202.h"      // ADAPTER_*INTERRUPT*

// Driver Framework Basic Definitions API
#include "basic_defs.h"            // bool, IDENTIFIER_NOT_USED

// Driver Framework C-RunTime Library API
#include "clib.h"                  // ZEROINIT

// EIP-201 Advanced interrupt Controller (AIC)
#include "eip201.h"

// Logging API
#include "log.h"

// Driver Framework Device API
#include "device_types.h"          // Device_Handle_t
#include "device_mgmt.h"           // Device_Find, Device_GetReference

// Linux Kernel API
#include <linux/interrupt.h>       // request_irq, free_irq,
                                   // DECLARE_TASKLET, tasklet_schedule,
                                   // IRQ_DISABLED
#include <linux/irq.h>             // IRQ_TYPE_LEVEL_HIGH
#include <linux/irqreturn.h>       // irqreturn_t

#ifdef ADAPTER_EIP202_USE_UMDEVXS_IRQ
#include "umdevxs_interrupt.h"
#endif

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define MAX_OS_IRQS                     256

#define ARRAY_ELEMCOUNT(_a)             (sizeof(_a) / sizeof(_a[0]))

#define ADAPTERINT_REQUEST_IRQ_FLAGS    (IRQF_SHARED)

// Data structure for each Advanced interrupt controller
typedef struct
{
    Device_Handle_t Device;
    char *name;    // Device name (can be found with Device_Find().
    int irq_idx;   // if isIRQ, then index of device interrupt.
        // If !isIRQ. then driver IRQ number to which AIC is connected.
    int irq;       // System IRQ number
    int BitIRQs[32]; // Mapping from source bits to internal driver IRQ numbers.
    bool isIRQ; // true if AIC has dedicated system IRQ line, false is it
                // is cascaded from another AIC.
} Adapter_AIC_t;

// Data structure per interrupt
typedef struct
{
    uint32_t SourceBitMask;
    char * name;
    Adapter_AIC_t *AIC;
    char *AICName;
    struct tasklet_struct tasklet;
    Adapter_InterruptHandler_t Handler;
    uint32_t Counter;
    void *extra;
    bool fHaveTasklet;
    EIP201_Config_t Config;
} Adapter_Interrupt_t;


/*----------------------------------------------------------------------------
 * Local variables
 */
static bool Adapter_IRQ_Initialized = false;

#define ADAPTER_EIP202_ADD_AIC(_name,_idx, isirq)  \
    {NULL, _name, _idx, -1, {0, }, isirq}


static Adapter_AIC_t Adapter_AICTable[] = { ADAPTER_EIP202_AICS };

#define ADAPTER_EIP202_ADD_IRQ(_name,_phy,_aicname,_tasklet,_pol)    \
    {(1<<(_phy)),#_name,NULL,_aicname,{},NULL,0,NULL,_tasklet, \
                                              EIP201_CONFIG_##_pol}

static Adapter_Interrupt_t Adapter_IRQTable[] = { ADAPTER_EIP202_IRQS };

// Define maximum number of supported interrupts
#define ADAPTER_MAX_INTERRUPTS  ARRAY_ELEMCOUNT(Adapter_IRQTable)

static Adapter_AIC_t * IRQ_AIC_Mapping[MAX_OS_IRQS];


/*----------------------------------------------------------------------------
 * AdapterINT_GetActiveIntNr
 *
 * Returns 0..31 depending on the lowest '1' bit.
 * Returns 32 when all bits are zero
 *
 * Using binary break-down algorithm.
 */
static inline int
AdapterINT_GetActiveIntNr(
        uint32_t Sources)
{
    unsigned int IntNr = 0;
    unsigned int R16, R8, R4, R2;

    if (Sources == 0)
        return 32;

    // if the lower 16 bits are empty, look at the upper 16 bits
    R16 = Sources & 0xFFFF;
    if (R16 == 0)
    {
        IntNr += 16;
        R16 = Sources >> 16;
    }

    // if the lower 8 bits are empty, look at the high 8 bits
    R8 = R16 & 0xFF;
    if (R8 == 0)
    {
        IntNr += 8;
        R8 = R16 >> 8;
    }
    R4 = R8 & 0xF;
    if (R4 == 0)
    {
        IntNr += 4;
        R4 = R8 >> 4;
    }

    R2 = R4 & 3;
    if (R2 == 0)
    {
        IntNr += 2;
        R2 = R4 >> 2;
    }

    // last two bits are trivial
    // 00 => cannot happen
    // 01 => +0
    // 10 => +1
    // 11 => +0
    if (R2 == 2)
        IntNr++;

    return IntNr;
}


/*----------------------------------------------------------------------------
 * AdapterINT_Report_InterruptCounters
 */
static void
AdapterINT_Report_InterruptCounters(void)
{
    int i;
    for (i=0; i<ARRAY_ELEMCOUNT(Adapter_IRQTable); i++)
    {
        if ( (1<<i) & ADAPTER_EIP202_INTERRUPTS_TRACEFILTER)
        {
            LOG_CRIT("AIC %s interrupt source %s mask %08x counter %d\n",
                     Adapter_IRQTable[i].AICName,
                     Adapter_IRQTable[i].name,
                     Adapter_IRQTable[i].SourceBitMask,
                     Adapter_IRQTable[i].Counter);
        }
    }
}


/*----------------------------------------------------------------------------
 * Adapter_Interrupt_Enable
 */
int
Adapter_Interrupt_Enable(
        const int nIRQ,
        const unsigned int Flags)
{
    int rc = -1;
    IDENTIFIER_NOT_USED(Flags);

    LOG_INFO("\n\t\t %s \n", __func__);

    if (nIRQ < 0 || nIRQ >= ADAPTER_MAX_INTERRUPTS)
    {
            LOG_CRIT(
                    "Adapter_Interrupt_Enable: "
                    "Failed, IRQ %d not supported\n",
                    nIRQ);
    }
    else
    {
        rc=EIP201_SourceMask_EnableSource(Adapter_IRQTable[nIRQ].AIC->Device,
                                       Adapter_IRQTable[nIRQ].SourceBitMask);
        LOG_INFO("\n\t\t\tAdapter_Interrupt_Enable "
                 "IRQ %d %s %s mask=%08x\n",
                 nIRQ,
                 Adapter_IRQTable[nIRQ].AICName,
                 Adapter_IRQTable[nIRQ].name,
                 Adapter_IRQTable[nIRQ].SourceBitMask);
   }

    LOG_INFO("\n\t\t %s done \n", __func__);
    return rc;
}


/*----------------------------------------------------------------------------
 * Adapter_Interrupt_Disable
 */
int
Adapter_Interrupt_Disable(
        const int nIRQ,
        const unsigned int Flags)
{
    int rc = -1;
    IDENTIFIER_NOT_USED(Flags);

    LOG_INFO("\n\t\t %s \n", __func__);

    if (nIRQ < 0 || nIRQ >= ADAPTER_MAX_INTERRUPTS)
    {
            LOG_CRIT(
                    "Adapter_Interrupt_Disable: "
                    "Failed, IRQ %d not supported\n",
                    nIRQ);
    }
    else
    {
        rc = EIP201_SourceMask_DisableSource(Adapter_IRQTable[nIRQ].AIC->Device,
                                       Adapter_IRQTable[nIRQ].SourceBitMask);
        LOG_INFO("\n\t\t\tAdapter_Interrupt_Disable "
                 "IRQ %d %s %s mask=%08x\n",
                 nIRQ,
                 Adapter_IRQTable[nIRQ].AICName,
                 Adapter_IRQTable[nIRQ].name,
                 Adapter_IRQTable[nIRQ].SourceBitMask);
    }

    LOG_INFO("\n\t\t %s done \n", __func__);
    return rc;
}


/*----------------------------------------------------------------------------
 * Adapter_Interrupt_SetHandler
 */
int
Adapter_Interrupt_SetHandler(
        const int nIRQ,
        Adapter_InterruptHandler_t HandlerFunction)
{
    if (nIRQ < 0 || nIRQ >= ADAPTER_MAX_INTERRUPTS)
        return -1;

    LOG_INFO(
            "Adapter_Interrupt_SetHandler: "
            "HandlerFnc=%p for IRQ %d\n",
            HandlerFunction,
            nIRQ);

    Adapter_IRQTable[nIRQ].Handler = HandlerFunction;
    return 0;
}


/*----------------------------------------------------------------------------
 * AdapterINT_CommonTasklet
 *
 * This handler is scheduled in the top-halve interrupt handler when it
 * decodes one of the CDR or RDR interrupt sources.
 * The data parameter is the IRQ value (from adapter_interrupts.h) for that
 * specific interrupt source.
 */
static void
AdapterINT_CommonTasklet(
        unsigned long data)
{
    const unsigned int IntNr = (unsigned int)data;
    Adapter_InterruptHandler_t H;

    LOG_INFO("\n\t\t%s \n", __func__);

    LOG_INFO("Tasklet invoked intnr=%d\n",IntNr);

    // verify we have a handler
    H = Adapter_IRQTable[IntNr].Handler;

    if (H)
    {
        // invoke the handler
        H(IntNr, 0);
    }
    else
    {
        LOG_CRIT(
            "AdapterINT_CommonTasklet: "
            "Error, disabling IRQ %d with missing handler\n",
            IntNr);

        Adapter_Interrupt_Disable(IntNr, 0);
    }

    LOG_INFO("\n\t\t%s done\n", __func__);
}


/*----------------------------------------------------------------------------
 * AdapterINT_AICHandler
 *
 * Handle all interrupts connected to the specified AIC.
 *
 * If this AIC is connected directly to a system IRQ line, this is
 * called directly from the Top Half Handler.
 *
 * If this AIC is connected via an IRQ line of another AIC, this is
 * called from the handler function of that interrupt.
 *
 * Return: 0 for success, -1 for failure.
 */
static int
AdapterINT_AICHandler(Adapter_AIC_t *AIC)
{
    EIP201_SourceBitmap_t Sources;
    int IntNr, irq, rc = 0;

    LOG_INFO("\n\t\t%s \n", __func__);

    if (AIC == NULL)
        return -1;

    if (AIC->Device == NULL)
    {
        LOG_INFO("%s: skipping spurious interrupt for AIC %s, IRQ %d\n",
                 __func__,
                 AIC->name,
                 AIC->irq);
        goto exit; // no error
    }

    Sources = EIP201_SourceStatus_ReadAllEnabled(AIC->Device);
    if (Sources == 0)
    {
        rc = -1;
        goto exit; // error
    }

    EIP201_Acknowledge(AIC->Device, Sources);

    LOG_INFO("%s: AIC %s, IRQ %d, sources=%x\n",
             __func__,
             AIC->name,
             AIC->irq,
             Sources);

    while (Sources)
    {
        IntNr = AdapterINT_GetActiveIntNr(Sources);

        /* Get number of first bit set */
        Sources &= ~(1<<IntNr);

        /* Clear this in sources */
        irq = AIC->BitIRQs[IntNr];

        LOG_INFO("%s: Handle IRQ %d for AIC %s\n", __func__, irq, AIC->name);

        if (irq < 0 || irq >= ADAPTER_MAX_INTERRUPTS)
        {
            LOG_CRIT("%s: %s IRQ not defined for bit %d, disabling source\n",
                     __func__,
                     AIC->name,
                     IntNr);
            EIP201_SourceMask_DisableSource(AIC->Device, (1<<IntNr));
        }

        Adapter_IRQTable[irq].Counter++;

        if ( (1<<irq) & ADAPTER_EIP202_INTERRUPTS_TRACEFILTER)
            LOG_CRIT("%s: encountered interrupt %d, bit %d for AIC %s\n",
                     __func__,
                     irq,
                     IntNr,
                     AIC->name);

        if(Adapter_IRQTable[irq].fHaveTasklet)
        {
            LOG_INFO("%s: Start tasklet\n", __func__);
            /* IRQ is handled via tasklet */
            tasklet_schedule(&Adapter_IRQTable[irq].tasklet);
            Adapter_Interrupt_Disable(irq, 0);
        }
        else
        {
            Adapter_InterruptHandler_t H = Adapter_IRQTable[irq].Handler;
            LOG_INFO("%s: Run normal handler\n", __func__);
            /* Handler is called directly */
            if (H)
            {
                H(irq, 0);
            }
            else
            {
                LOG_CRIT(
                    "%s : Error, disabling IRQ %d with missing handler\n",
                    __func__,
                    irq);

                Adapter_Interrupt_Disable(irq, 0);
            }
        }
    } // while

exit:
    LOG_INFO("\n\t\t%s done\n", __func__);
    return rc;
}


/*----------------------------------------------------------------------------
 * AdapterINT_TopHalfHandler
 *
 * This is the interrupt handler function call by the kernel when our hooked
 * interrupt is active.
 *
 * Call the handler for the associated AIC.
 */
static irqreturn_t
AdapterINT_TopHalfHandler(
        int irq,
        void * dev_id)
{
    irqreturn_t Int_Rc = IRQ_NONE;
    LOG_INFO("\n\t\t%s \n", __func__);

    if (irq < 0 || irq >= MAX_OS_IRQS || IRQ_AIC_Mapping[irq]==NULL)
    {
        LOG_CRIT("%s: No AIC defined for IRQ %d\n",__func__,irq);
        goto error;
    }

    if ( AdapterINT_AICHandler(IRQ_AIC_Mapping[irq]) < 0)
    {
        goto error;
    }

    Int_Rc = IRQ_HANDLED;

error:
    LOG_INFO("\n\t\t%s done\n", __func__);

    IDENTIFIER_NOT_USED(dev_id);
    return Int_Rc;
}


/*----------------------------------------------------------------------------
 * AdapterINT_ChainedAIC
 *
 * Handler function for IRQ that services an entire AIC.
 */
static void
AdapterINT_ChainedAIC(const int irq, const unsigned int flags)
{
    IDENTIFIER_NOT_USED(flags);
    AdapterINT_AICHandler(Adapter_IRQTable[irq].extra);
}


/*----------------------------------------------------------------------------
 * AdapterINT_SetInternalLinkage
 *
 * Create AIC References in Adapter_IRQTable.
 * Fill in BitIRQs references in Adapter_AICTable.
 * Perform some consistency checks.
 *
 * Return 0 on success, -1 on failure.
 */
static int
AdapterINT_SetInternalLinkage(void)
{
    int i,j;
    int IntNr;

    for (i=0; i<ARRAY_ELEMCOUNT(Adapter_IRQTable); i++)
    {
        Adapter_IRQTable[i].AIC = NULL;
    }

    for (i=0; i<ARRAY_ELEMCOUNT(Adapter_AICTable); i++)
    {
        for (j=0; j<32; j++)
        {
            Adapter_AICTable[i].BitIRQs[j] = -1;
        }
        for (j=0; j<ARRAY_ELEMCOUNT(Adapter_IRQTable); j++)
        {
            if (strcmp(Adapter_AICTable[i].name, Adapter_IRQTable[j].AICName)
                == 0)
            {
                if (Adapter_IRQTable[j].AIC)
                {
                    LOG_CRIT("%s: AIC link set more than once\n",__func__);
                }
                Adapter_IRQTable[j].AIC = Adapter_AICTable + i;
                IntNr = AdapterINT_GetActiveIntNr(
                    Adapter_IRQTable[j].SourceBitMask);
                if (IntNr < 0 || IntNr >= 32)
                {
                    LOG_CRIT("%s: IRQ %d source bit %d out of range\n",
                             __func__,j,IntNr);
                    return -1;
                }
                else if (Adapter_AICTable[i].BitIRQs[IntNr] >= 0)
                {
                    LOG_CRIT(
                        "%s: AIC %s IRQ %d source bit %d already defined\n",
                        __func__,
                        Adapter_AICTable[i].name,
                        j,
                        IntNr);
                    return -1;
                }
                else
                {
                    Adapter_AICTable[i].BitIRQs[IntNr] = j;
                }
            }
        }
    }
    for (i=0; i<ARRAY_ELEMCOUNT(Adapter_IRQTable); i++)
    {
        if (Adapter_IRQTable[i].AIC == NULL)
        {
            LOG_CRIT("%s: AIC pointer of IRQ %d is null\n",__func__,i);
            return -1;
        }
    }

    return 0;
}


/*----------------------------------------------------------------------------
 * AdapterINT_AIC_Init
 *
 */
static bool
AdapterINT_AIC_Init(void)
{
    EIP201_Status_t res;
    unsigned int i;

    // Initialize all configured EIP-201 AIC devices
    for (i = 0; i < ARRAY_ELEMCOUNT(Adapter_AICTable); i++)
    {
        LOG_INFO("%s: Initialize AIC %s\n",__func__,Adapter_AICTable[i].name);

        Adapter_AICTable[i].Device = Device_Find(Adapter_AICTable[i].name);
        if (Adapter_AICTable[i].Device == NULL)
        {
            LOG_CRIT("%s: Device_Find() failed for %s\n",
                     __func__,
                     Adapter_AICTable[i].name);
            return false; // error
        }

        res = EIP201_Initialize(Adapter_AICTable[i].Device, NULL, 0);
        if (res != EIP201_STATUS_SUCCESS)
        {
            LOG_CRIT("%s: EIP201_Initialize() failed, error %d\n", __func__, res);
            return false; // error
        }
    }

    return true; // success
}


/*----------------------------------------------------------------------------
 * AdapterINT_AIC_Enable
 *
 */
static void
AdapterINT_AIC_Enable(void)
{
    unsigned int i;

    for (i = 0; i < ARRAY_ELEMCOUNT(Adapter_AICTable); i++)
        if (!Adapter_AICTable[i].isIRQ)
            Adapter_Interrupt_Enable(Adapter_AICTable[i].irq_idx, 0);
}


/*----------------------------------------------------------------------------
 * Adapter_Interrupts_Init
 *
 */
int
Adapter_Interrupts_Init(
        const int nIRQ)
{
    int i;
    int IntNr = nIRQ;

    LOG_INFO("\n\t\t %s \n", __func__);

    if (AdapterINT_SetInternalLinkage() < 0)
    {
        LOG_CRIT("Interrupt AIC and IRQ tables are inconsistent\n");
        return -1;
    }

    // Initialize the AIC devices
    if (!AdapterINT_AIC_Init())
        return -1;

    // Initialize the Adapter_IRQTable and tasklets.
    for (i=0; i<ARRAY_ELEMCOUNT(Adapter_IRQTable); i++)
    {
        Adapter_IRQTable[i].Handler = NULL;
        Adapter_IRQTable[i].extra = NULL;
        Adapter_IRQTable[i].Counter = 0;
        if (Adapter_IRQTable[i].fHaveTasklet)
            tasklet_init(&Adapter_IRQTable[i].tasklet,
                         AdapterINT_CommonTasklet,
                         (long)i);
        EIP201_Config_Change(Adapter_IRQTable[i].AIC->Device,
                             Adapter_IRQTable[i].SourceBitMask,
                             Adapter_IRQTable[i].Config);
        // Clear any pending egde-sensitive interrupts.
        EIP201_Acknowledge(Adapter_IRQTable[i].AIC->Device,
                           Adapter_IRQTable[i].SourceBitMask);
    }

    // Request the IRQs for each AIC or register to IRQ of other AIC.
    for (i=0; i<ARRAY_ELEMCOUNT(Adapter_AICTable); i++)
    {
        if (Adapter_AICTable[i].isIRQ)
        {
            int res;

            LOG_INFO("\n\t\t %s: Request IRQ for AIC %s\n",
                     __func__,Adapter_AICTable[i].name);

#ifdef ADAPTER_EIP202_USE_UMDEVXS_IRQ
            res = UMDevXS_Interrupt_Request(AdapterINT_TopHalfHandler,
                                            Adapter_AICTable[i].irq_idx);
            IntNr = res;
#else
            {
                struct device * Device_p;
                // Get device reference for this resource
                Device_p = Device_GetReference(NULL, NULL);
                res = request_irq(IntNr,
                                  AdapterINT_TopHalfHandler,
                                  ADAPTERINT_REQUEST_IRQ_FLAGS,
                                  ADAPTER_EIP202_DRIVER_NAME,
                                  Device_p);
            }
#endif
            if (res < 0)
            {
                LOG_CRIT("%s: Request IRQ error %d\n", __func__, res);
                return res;
            }
            else
            {
                Adapter_AICTable[i].irq = IntNr;
                IRQ_AIC_Mapping[IntNr] = Adapter_AICTable + i;
                LOG_INFO("%s: Successfully hooked IRQ %d\n", __func__, IntNr);
            }
        }
        else
        {
            IntNr = Adapter_AICTable[i].irq_idx;
            LOG_INFO("%s: Hook up AIC %s to chained IRQ %d\n",
                     __func__,
                     Adapter_AICTable[i].name,
                     IntNr);
            if (IntNr < 0 || IntNr >= ADAPTER_MAX_INTERRUPTS)
            {
                LOG_CRIT("%s: IRQ %d out of range\n", __func__,IntNr);
            }
            Adapter_IRQTable[IntNr].extra =  Adapter_AICTable + i;
            Adapter_Interrupt_SetHandler(IntNr, AdapterINT_ChainedAIC);
        }
    }

    // Enable AIC
    AdapterINT_AIC_Enable();

    LOG_INFO("\n\t\t %s done\n", __func__);
    Adapter_IRQ_Initialized = true;

    return 0;
}


/*----------------------------------------------------------------------------
 * Adapter_Interrupts_UnInit
 */
int
Adapter_Interrupts_UnInit(const int nIRQ)
{
    unsigned int i;
    IDENTIFIER_NOT_USED(nIRQ);

    LOG_INFO("\n\t\t %s \n", __func__);
    if (!Adapter_IRQ_Initialized)
        return -1;

    // disable all interrupts
    for (i = 0; i < ARRAY_ELEMCOUNT(Adapter_AICTable); i++)
    {
        if (Adapter_AICTable[i].Device)
        {
            EIP201_SourceMask_DisableSource(Adapter_AICTable[i].Device,
                                            EIP201_SOURCE_ALL);
            Adapter_AICTable[i].Device = NULL;
        }

        if(Adapter_AICTable[i].isIRQ && Adapter_AICTable[i].irq > 0)
        {
#ifdef ADAPTER_EIP202_USE_UMDEVXS_IRQ
            UMDevXS_Interrupt_Request(NULL,Adapter_AICTable[i].irq_idx);
#else
            // Get device reference for this resource
            struct device * Device_p = Device_GetReference(NULL, NULL);

            LOG_INFO("%s: Free IRQ %d for AIC %s\n",
                     __func__,
                     Adapter_AICTable[i].irq,
                     Adapter_AICTable[i].name);

            // unhook the interrupt
            free_irq(Adapter_AICTable[i].irq, Device_p);

            LOG_INFO("%s: Successfully freed IRQ %d for AIC %s\n",
                     __func__,
                     Adapter_AICTable[i].irq,
                     Adapter_AICTable[i].name);
#endif
        }
    }

    // Kill all tasklets
    for (i = 0; i < ARRAY_ELEMCOUNT(Adapter_IRQTable); i++)
        if (Adapter_IRQTable[i].fHaveTasklet)
            tasklet_kill(&Adapter_IRQTable[i].tasklet);

    AdapterINT_Report_InterruptCounters();

    ZEROINIT(IRQ_AIC_Mapping);

    Adapter_IRQ_Initialized = false;

    LOG_INFO("\n\t\t %s done\n", __func__);

    return 0;
}


#ifdef ADAPTER_PEC_RPM_EIP202_DEVICE0_ID
/*----------------------------------------------------------------------------
 * Adapter_Interrupts_Resume
 */
int
Adapter_Interrupts_Resume(void)
{
    LOG_INFO("\n\t\t %s \n", __func__);

    if (!Adapter_IRQ_Initialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return -1;
    }

    // Resume AIC devices
    if (!AdapterINT_AIC_Init())
        return -2; // error

    // Re-enable AIC interrupts
    AdapterINT_AIC_Enable();

    LOG_INFO("\n\t\t %s done\n", __func__);

    return 0; // success
}
#endif


/* end of file adapter_interrupts.c */
