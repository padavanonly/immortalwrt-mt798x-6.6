/* adapter_getpid.c
 *
 * Linux kernel specific Adapter module
 * responsible for adapter-wide pid management.
 */

/*****************************************************************************
* Copyright (c) 2015-2020 by Rambus, Inc. and/or its subsidiaries.
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

// Adapter GetPid API
#include "adapter_getpid.h"

// Linux Kernel API
#include <asm/current.h>       // process information
#include <linux/sched.h>       // for "struct task_struct"


/*----------------------------------------------------------------------------
 * Adapter_GetPid
 */
int
Adapter_GetPid(void)
{
    return (int)current->pid;
}


/* end of file adapter_getpid.c */
