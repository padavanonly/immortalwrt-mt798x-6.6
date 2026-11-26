/* cs_eip201.h
 *
 * Configuration Settings for the EIP-201 Driver Library module.
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

#include "cs_driver.h"      // top-level driver configuration

// set this option to enable checking of all arguments to all EIP201 functions
// disable it to reduce code size and reduce overhead
#ifndef DRIVER_PERFORMANCE
#define EIP201_STRICT_ARGS
#endif

// maximum number of interrupts as defined for this instance
#define EIP201_STRICT_ARGS_MAX_NUM_OF_INTERRUPTS  29

/* EIP201_REMOVE_*
 *
 * These switches allow removal of unused API functions
 * to reduce footprint and increase code-coverage figures
 */
#define EIP201_REMOVE_CONFIG_CHANGE
#define EIP201_REMOVE_CONFIG_READ
//#define EIP201_REMOVE_SOURCEMASK_ENABLESOURCE
//#define EIP201_REMOVE_SOURCEMASK_DISABLESOURCE
#define EIP201_REMOVE_SOURCEMASK_SOURCEISENABLED
#define EIP201_REMOVE_SOURCEMASK_READALL
#define EIP201_REMOVE_SOURCESTATUS_ISENABLEDSOURCEPENDING
#define EIP201_REMOVE_SOURCESTATUS_ISRAWSOURCEPENDING
//#define EIP201_REMOVE_SOURCESTATUS_READALLENABLED
#define EIP201_REMOVE_SOURCESTATUS_READALLRAW
//#define EIP201_REMOVE_INITIALIZE
//#define EIP201_REMOVE_ACKNOWLEDGE

#ifdef DRIVER_POLLING
// disable all functions
#define EIP201_REMOVE_CONFIG_CHANGE
#define EIP201_REMOVE_CONFIG_READ
#define EIP201_REMOVE_SOURCEMASK_ENABLESOURCE
#define EIP201_REMOVE_SOURCEMASK_DISABLESOURCE
#define EIP201_REMOVE_SOURCEMASK_SOURCEISENABLED
#define EIP201_REMOVE_SOURCEMASK_READALL
#define EIP201_REMOVE_SOURCESTATUS_ISENABLEDSOURCEPENDING
#define EIP201_REMOVE_SOURCESTATUS_ISRAWSOURCEPENDING
#define EIP201_REMOVE_SOURCESTATUS_READALLENABLED
#define EIP201_REMOVE_SOURCESTATUS_READALLRAW
#define EIP201_REMOVE_INITIALIZE
#define EIP201_REMOVE_ACKNOWLEDGE
#endif /* DRIVER_POLLING */

/* end of file cs_eip201.h */
