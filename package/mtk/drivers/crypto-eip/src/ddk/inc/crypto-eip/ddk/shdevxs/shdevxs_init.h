/* shdevxs_init.h
 *
 * Kernel suport Driver initialization API.
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

#ifndef SHDEVXS_INIT_H_
#define SHDEVXS_INIT_H_

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "shdevxs_init.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Types Definitions API
#include "basic_defs.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * SHDevXS_Global_SetPrivileged
 *
 * This function is called once by the Global Control Driver to
 * tell the kernel support driver that it has privileged access
 * to the EIP hardware.
 *
 * API use order:
 *     This function must be called first by the Global Control Driver, before
 *     any other SHDevXS functions.
 *
 * Return Value
 *     0  Success
 *    0>  Failure, code is implementation-specific
 */
int
SHDevXS_Global_SetPrivileged(
        void);


/*----------------------------------------------------------------------------
 * SHDevXS_Global_Init
 *
 * This function is called once by the Global Control Driver to
 * tell the kernel support driver that it has done basic initialization
 * of the hardware, so the Kernel Support Driver is now allowed to access
 * hardware.
 *
 * API use order:
 *     This function must be called once by the Global Control Driver, before
 *     any other SHDevXS functions. It can be called again after
 *     SHDevXS_HW_UnInit
 *
 * Return Value
 *     0  Success
 *    0>  Failure, code is implementation-specific
 */
int
SHDevXS_Global_Init(
        void);


/*----------------------------------------------------------------------------
 * SHDevXS_Global_UnInit
 *
 * This function is called once by the Global Control Driver when it
 * exits. At that point, the Kernel Support Driver is no longer allowed to
 * access hardware and no other applications may do so.
 *
 * API use order:
 *     This function must be called once by the Global Control Driver, before
 *     it exits.
 *
 * Return Value
 *     0  Success
 *    0>  Failure, code is implementation-specific
 */
int
SHDevXS_Global_UnInit(
        void);


/*----------------------------------------------------------------------------
 * SHDevXS_Test
 *
 * Function to test whether communication with the kernel support driver
 * is possible.
 *
 * Return Value
 *     0  Success
 *    0>  Failure, code is implementation-specific
 */
int
SHDevXS_Test(void);


#endif /* SHDEVXS_INIT_H_ */


/* end of file shdevxs_init.h */
