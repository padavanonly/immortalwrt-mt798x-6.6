/* umdevxs_interrupt.h
 *
 * Exported API for interrupt of the UMDexXS/UMPCI driver.
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

#ifndef INCLUDE_GUARD_UMDEVXS_INTERRUPT_H
#define INCLUDE_GUARD_UMDEVXS_INTERRUPT_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include <linux/interrupt.h>

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * UMDevXS_Interrupt_Request
 *
 * Request an IRQ handler to be registered for the IRQ of the device managed
 * by this driver. It temporarily unregisters the driver's own IRQ handler.
 * Unregister any handler if a null pointer is passed and restore the driver's
 * own handler
 *
 * Handler_p (input)
 *    Pointer to the interrupt (top half) handler to be registered for the
 *    device. If NULL is passed, restore the driver's own handler.
 *
 * Index (input)
 *    Index of the interrupt for which a handler should be registered.
 *    0 for the first interrupt, 1 for the second, etc.
 *
 * Return: OS IRQ number on success (nonnegative).
 *         -1 on failure.
 */
int
UMDevXS_Interrupt_Request(irq_handler_t Handler_p, int Index);


#endif /* INCLUDE_GUARD_UMDEVXS_INTERRUPT_H */

/* umdevxs_interrupt.h */
