/* shdevxs_rc.h
 *
 * Shared Device Access Record Cache API
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

#ifndef SHDEVXS_RC_H_
#define SHDEVXS_RC_H_

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "shdevxs_rc.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Types Definitions API
#include "basic_defs.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * SHDevXS_TRC_Record_Invalidate
 *
 * This function invalidates the transform record in the TRC.
 *
 * ByteOffset (input)
 *     Record byte offset in the Transform Record Cache.
 *
 * Return Value
 *     0  Success
 *    0>  Failure, code is implementation-specific
 */
int
SHDevXS_TRC_Record_Invalidate(
        const uint32_t ByteOffset);


/*----------------------------------------------------------------------------
 * SHDevXS_ARC4RC_Record_Invalidate
 *
 * This function invalidates the ARC4 state record in the ARC4RC.
 *
 * ByteOffset (input)
 *     Record byte offset in the ARC4 Record Cache.
 *
 * Return Value
 *     0  Success
 *    0>  Failure, code is implementation-specific
 */
int
SHDevXS_ARC4RC_Record_Invalidate(
        const uint32_t ByteOffset);


/*----------------------------------------------------------------------------
 * SHDevXS_RC_Lock
 *
 * This function locks the TRC for mutually exclusive use.
 *
 * Return Value
 *     0  Success
 *    0>  Failure, code is implementation-specific
 */
int
SHDevXS_RC_Lock(void);


/*----------------------------------------------------------------------------
 * SHDevXS_RC_Free
 *
 * This function frees the TRC from the mutually exclusive use.
 *
 * Return Value
 *     None
 */
void
SHDevXS_RC_Free(void);


/*----------------------------------------------------------------------------
 * SHDevXS_RC_Record_Dummy_Addr_Get
 *
 * This function returns the dummy record address.
 *
 * Return Value
 *     Record Cache record dummy (NULL) address.
 */
unsigned int
SHDevXS_RC_Record_Dummy_Addr_Get(void);


#endif /* SHDEVXS_RC_H_ */


/* end of file shdevxs_rc.h */
