/* api_global_driver197_init.h
 *
 * Security-IP-197 Global Control Driver Initialization Interface
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

#ifndef DRIVER197_GLOBAL_INIT_H_
#define DRIVER197_GLOBAL_INIT_H_


/*----------------------------------------------------------------------------
 * Driver197_Global_Init
 *
 * Initialize the global control driver. This function must be called before
 * any other driver API function can be called.
 *
 * Returns 0 for success and -1 for failure.
 */
int
Driver197_Global_Init(void);


/*----------------------------------------------------------------------------
 * Driver197_Global_Exit
 *
 * Uninitialize the global control driver. After this function is called
 * no other driver API function can be called except Driver197_Global_Init().
 */
void
Driver197_Global_Exit(void);


#endif /* DRIVER197_GLOBAL_INIT_H_ */


/* end of file api_driver197_init.h */
