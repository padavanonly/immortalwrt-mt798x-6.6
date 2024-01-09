/* adapter_int_shdevxs.c
 *
 * Adapter Shared Device Access module responsible for interrupts.
 * Implementation depends on the Kernel Support Driver.
 *
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

#include "adapter_interrupts.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"            // bool, IDENTIFIER_NOT_USED

// Kernel support driver
#include "shdevxs_irq.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * Adapter_Interrupt_Enable
 */
int
Adapter_Interrupt_Enable(
        const int nIRQ,
        const unsigned int Flags)
{
    return SHDevXS_IRQ_Enable(nIRQ, Flags);
}


/*----------------------------------------------------------------------------
 * Adapter_Interrupt_Disable
 *
 */
int
Adapter_Interrupt_Disable(
        const int nIRQ,
        const unsigned int Flags)
{
    return SHDevXS_IRQ_Disable(nIRQ, Flags);
}


/*----------------------------------------------------------------------------
 * Adapter_Interrupt_SetHandler
 */
int
Adapter_Interrupt_SetHandler(
        const int nIRQ,
        Adapter_InterruptHandler_t HandlerFunction)
{
    return SHDevXS_IRQ_SetHandler(nIRQ, HandlerFunction);
}


/*----------------------------------------------------------------------------
 * Adapter_Interrupts_Init
 */
int
Adapter_Interrupts_Init(
        const int nIRQ)
{
    int res = SHDevXS_IRQ_Init();

    IDENTIFIER_NOT_USED(nIRQ);

    return res;
}


/*----------------------------------------------------------------------------
 * Adapter_Interrupts_UnInit
 */
int
Adapter_Interrupts_UnInit(
        const int nIRQ)
{
    IDENTIFIER_NOT_USED(nIRQ);
    return SHDevXS_IRQ_UnInit();
}


/*----------------------------------------------------------------------------
 * Adapter_Interrupts_Resume
 */
int
Adapter_Interrupts_Resume(void)
{
    // Resume AIC devices is done in SHDevXS IRQ module
    return 0; // success
}


/* end of file adapter_int_shdevxs.c */
