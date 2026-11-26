/* shdevxs_kernel.h
 *
 * Initiializ/uninitialize the kernel support driver.
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

#ifndef INCLUDE_GUARD_SHDEVXS_KERNEL_H
#define INCLUDE_GUARD_SHDEVXS_KERNEL_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "umdevxs_cmd.h"

/*----------------------------------------------------------------------------
 * UMDevXS_Init
 */
int
SHDevXS_Init(void);

/*----------------------------------------------------------------------------
 * SHDevXS_UnInit
 */
int
SHDevXS_UnInit(void);

/*----------------------------------------------------------------------------
 * SHDevXS_Cleanup
 *
 * Un-allocate all resources owned by the spefified application.
 */
void
SHDevXS_Cleanup(void *AppID);

/*----------------------------------------------------------------------------
 * SHDevXS_HandleCmd
 */
void
SHDevXS_HandleCmd(
    void *file_p,
    UMDevXS_CmdRsp_t * const CmdRsp);


#endif /* INCLUDE_GUARD_SHDEVXS_KERNEL_H */

/* shdevxs_kernel.h */
