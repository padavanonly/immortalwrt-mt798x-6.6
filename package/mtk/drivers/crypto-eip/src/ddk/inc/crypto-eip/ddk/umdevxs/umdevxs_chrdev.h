/* umdevxs_chrdev.h
 *
 * Linux UMDevXS Driver Character Device Interfaces.
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

#ifndef INCLUDE_GUARD_UMDEVXS_CHRDEV_H
#define INCLUDE_GUARD_UMDEVXS_CHRDEV_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "umdevxs_cmd.h"        // UMDevXS_CmdRsp_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

typedef int (* UMDevXS_HandleCmdFunction_t)(
        void * AppID,
        UMDevXS_CmdRsp_t * const CmdRsp_p);


/*----------------------------------------------------------------------------
 * UMDevXS_ChrDev_Init
 *
 * This routine hooks the character device.
 *
 * Return Value:
 *     0    Success
 *     <0   Error code return by a kernel call
 */
int
UMDevXS_ChrDev_Init(void);


/*----------------------------------------------------------------------------
 * UMDevXS_ChrDev_UnInit
 *
 * This routine unhooks the character device.
 */
void
UMDevXS_ChrDev_UnInit(void);

/*----------------------------------------------------------------------------
 * UMDevXS_ChrDev_HandleCmdFunc_Set
 *
 * This routine installs the external handle function for commands.
 */
void
UMDevXS_ChrDev_HandleCmdFunc_Set
        (UMDevXS_HandleCmdFunction_t Func);


#endif /* INCLUDE_GUARD_UMDEVXS_CHRDEV_H */


/* end of file umdevxs_chrdev.h */
