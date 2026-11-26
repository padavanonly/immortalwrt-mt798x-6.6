/* shdevxs_kernel_internal.h
 *
 * Internal API used by parts of the Kernel Support Driver.
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

#ifndef INCLUDE_GUARD_SHDEVXS_KERNEL_INTERNAL_H
#define INCLUDE_GUARD_SHDEVXS_KERNEL_INTERNAL_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_shdevxs.h"

#ifdef SHDEVXS_ENABLE_DMAPOOL_FUNCTIONS
#include "shdevxs_dmapool.h"
#endif
#ifdef SHDEVXS_ENABLE_RC_FUNCTIONS
#include "shdevxs_rc.h"
#endif
#include "shdevxs_irq.h"
#ifdef SHDEVXS_ENABLE_FUNCTIONS
#include "eip97_global_types.h"
#endif
#include "device_types.h"

#ifdef SHDEVXS_ENABLE_FUNCTIONS
extern Device_Handle_t SHDevXS_Device;
#endif

/*----------------------------------------------------------------------------
 * SHDevXS_Internal_IRQ_Initialize
 *
 * Initialize the interrupt subsystem.
 */
int
SHDevXS_Internal_IRQ_Initialize(void);

/*----------------------------------------------------------------------------
 * SHDevXS_Internal_IRQ_Initialize
 *
 * De-initialize the interrupt subsystem.
 */
void
SHDevXS_Internal_IRQ_UnInitialize(void);

/*----------------------------------------------------------------------------
 * SHDevXS_IRQ_UnInit
 *
 * Un-initialize all IRQ handling specific to an application.
 */
int
SHDevXS_Internal_IRQ_Cleanup(void *AppID);

/*----------------------------------------------------------------------------
 * SHDevXS_IRQ_Enable
 *
 * Enable the specified IRQ on the hardware.
 *
 * Return 0 on success, -1 on failure.
 */
int
SHDevXS_Internal_IRQ_Enable(
        void *AppID,
        const int nIRQ,
        const unsigned int Flags);

/*----------------------------------------------------------------------------
 * SHDevXS_IRQ_Disable
 *
 * Disable the specified IRQ on the hardware.
 *
 * Return 0 on success, -1 on failure.
 */
int
SHDevXS_Internal_IRQ_Disable(
        void *AppID,
        const int nIRQ,
        const unsigned int Flags);

/*----------------------------------------------------------------------------
 * SHDevXS_IRQ_Clear
 *
 * Clear the specified IRQ on the hardware.
 *
 * Return 0 on success, -1 on failure.
 */
int
SHDevXS_Internal_IRQ_Clear(
        void *AppID,
        const int nIRQ,
        const unsigned int Flags);

/*----------------------------------------------------------------------------
 * SHDevXS_IRQ_ClearAndEnable
 *
 * Clear and enable the specified IRQ on the hardware.
 *
 * Return 0 on success, -1 on failure.
 */
int
SHDevXS_Internal_IRQ_ClearAndEnable(
        void *AppID,
        const int nIRQ,
        const unsigned int Flags);


/*----------------------------------------------------------------------------
 * SHDevXS_IRQ_SetHandler
 *
 * Install the specified handler for the specified IRQ.
 * If the handle is NULL, remove the handler from the interrupt.
 *
 * This operation also claims the specified interrupt for a specific
 * application. This operation fails if the interrupt is already owned
 * by a different application.
 *
 * AppID (input)
 *    Reference to the application that claims the interrupt.
 * nIRQ (input)
 *    Number of the interrupt.
 * HandlerFunction (input)
 *    Handler function for the interrupt.
 *
 * Return 0 on success, -1 on failure.
 */
int
SHDevXS_Internal_IRQ_SetHandler(
        void *AppID,
        const int nIRQ,
        SHDevXS_InterruptHandler_t HandlerFunction);


/*----------------------------------------------------------------------------
 * SHDevXSLib_Internal_IRQHandler
 *
 * Interrupt handler (in kernel) that is called on behalf of user processes.
 *
 */
void
SHDevXS_Internal_IRQHandler(const int nIRQ, const unsigned int flags);


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_WaitWithTimeout
 *
 * This function is called on behalf of a user process and waits until the
 * specified interrupt has occurred. Check whether the specified interrupt
 * is owned by the appplication.
 *
 * AppID (input)
 *    reference to the application waiting for the interrupt.
 * nIRQ (input)
 *    the number of the IRQ to wait on.
 * timeout (input)
 *    timeout in milliseconds.
 *
 * Return 0 on success.
 */
int
SHDevXS_Internal_WaitWithTimeout(
        void *AppID,
        int nIRQ,
        unsigned int timeout);


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_Lock_Alloc
 */
void *
SHDevXS_Internal_Lock_Alloc(void);


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_Lock_Free
 */
void
SHDevXS_Internal_Lock_Free(void * Lock_p);


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_Lock_Acquire
 */
void
SHDevXS_Internal_Lock_Acquire(
        void * Lock_p,
        unsigned long * Flags);

/*----------------------------------------------------------------------------
 * SHDevXS_Internal_Lock_Release
 */
void
SHDevXS_Internal_Lock_Release(
        void * Lock_p,
        unsigned long * Flags);


#ifdef SHDEVXS_ENABLE_RC_FUNCTIONS
/*----------------------------------------------------------------------------
 * SHDevXS_Internal_RC_Initialize
 *
 * Initialize the transform record cache functions.
 *
 * Return 0 on success, -1 on failure.
 */
int
SHDevXS_Internal_RC_Initialize(void);

/*----------------------------------------------------------------------------
 * SHDevXS_Internal_RC_Initialize
 *
 * Uninitialize the transform record cache functions.
 */
void
SHDevXS_Internal_RC_UnInitialize(void);

/*----------------------------------------------------------------------------
 * SHDevXS_Internal_RC_SetBase
 *
 * Set the base address of the transform record cache to the indicated address.
 */
void
SHDevXS_Internal_RC_SetBase(
        const void * BasePtr);
#endif


#ifdef SHDEVXS_ENABLE_DMAPOOL_FUNCTIONS
/*----------------------------------------------------------------------------
 * SHDevXS_Internal_DMAPool_Initialize
 *
 * Initialize the DMA pool memory bank.
 *
 * Return 0 on success, -1 on failure.
 */
int
SHDevXS_Internal_DMAPool_Initialize(void);

/*----------------------------------------------------------------------------
 * SHDevXS_Internal_DMAPool_Initialize
 *
 * Uninitialize the DMA pool memory bank.
 */
void
SHDevXS_Internal_DMAPool_UnInitialize(void);


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_DMAPool_Init
 *
 * This function initialize the DMA pool for the calling application.
 *
 * API use order:
 *     This function must be called once before any of the other functions.
 *
 * AppId (input)
 *     Reference to the application (or NULL for the kernel driver).
 *
 * DMAPool_p (output)
 *     Pointer to memory location where the DMA pool control data will
 *     be stored. Cannot be NULL.
 *
 * Return Value
 *     0  Success
 *    0>  Failure, code is implementation-specific
 */
int
SHDevXS_Internal_DMAPool_Init(
        void *AppId,
        SHDevXS_DMAPool_t * const DMAPool_p);


/*----------------------------------------------------------------------------
 * SHDevXS_Internal_DMAPool_Uninit
 *
 * This function uninitializes and frees all the resources used by the calling
 * application for the Record Cache.
 *
 * API use order:
 *     This function must be called last, as clean-up step before stopping
 *     the application.
 *
 * AppId (input)
 *     Reference to the application (or NULL for the kernel driver).
 *
 * Return Value
 *     None
 */
void
SHDevXS_Internal_DMAPool_Uninit(void *AppId);
#endif /* SHDEVXS_ENABLE_DMAPOOL_FUNCTIONS */




#endif /* INCLUDE_GUARD_SHDEVXS_KERNEL_INTERNAL_H */

/* shdevxs_kernel_internal.h */
