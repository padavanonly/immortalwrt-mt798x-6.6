/* cs_ringhelper.h
 *
 * Ring Helper Configuration File
 */

/*****************************************************************************
* Copyright (c) 2010-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_CS_RINGHELPER_H
#define INCLUDE_GUARD_CS_RINGHELPER_H

#include "cs_driver.h"

// when enabled, all function call parameters are sanity-checked
// comment-out to disable this code
#ifndef DRIVER_PERFORMANCE
#define RINGHELPER_STRICT_ARGS
#endif

// the following switch removes support for the Status Callback Function
//#define RINGHELPER_REMOVE_STATUSFUNC

// the following switch removes support for separate rings
// use when only overlapping rings are used
//#define RINGHELPER_REMOVE_SEPARATE_RING_SUPPORT


#endif /* Include Guard */


/* end of file cs_ringhelper.h */
