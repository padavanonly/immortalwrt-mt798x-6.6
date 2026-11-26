/* adapter_sleep.c
 *
 * Linux kernel specific Adapter module
 * responsible for adapter-wide time management.
 */

/*****************************************************************************
* Copyright (c) 2008-2020 by Rambus, Inc. and/or its subsidiaries.
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

// Adapter Sleep API
#include "adapter_sleep.h"

// Driver Framework Basic Defs API
#include "basic_defs.h"

// Linux Kernel API
#include <linux/delay.h>        // msleep, no-busy-waiting implementation


/*----------------------------------------------------------------------------
 * Adapter_SleepMS
 */
void
Adapter_SleepMS(
        const unsigned int Duration_ms)
{
    msleep(Duration_ms);
}


/* end of file adapter_sleep.c */
