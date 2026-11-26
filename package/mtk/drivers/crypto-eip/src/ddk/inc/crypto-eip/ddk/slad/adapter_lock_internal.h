/* adapter_lock_internal.h
 *
 * Adapter concurrency (locking) management
 * Internal interface
 *
 */

/*****************************************************************************
* Copyright (c) 2013-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_ADAPTER_LOCK_INTERNAL_H
#define INCLUDE_GUARD_ADAPTER_LOCK_INTERNAL_H


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // bool


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Internal critical section data structure
typedef struct
{
    // Generic lock pointer
    void * Lock_p;

    // True if the lock is taken
    volatile bool fLocked;

} Adapter_Lock_CS_Internal_t;


#endif // INCLUDE_GUARD_ADAPTER_LOCK_INTERNAL_H


/* end of file adapter_lock_internal.h */
