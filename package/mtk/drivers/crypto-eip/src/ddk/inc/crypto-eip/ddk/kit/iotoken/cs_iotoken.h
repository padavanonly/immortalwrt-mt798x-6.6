/* cs_iotoken.h
 *
 * Top-level configuration file
 */

/*****************************************************************************
* Copyright (c) 2016-2022 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef CS_IOTOKEN_H_
#define CS_IOTOKEN_H_

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */
#include "cs_ddk197.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
// Enable extended errors
#ifdef DDK_EIP197_EXTENDED_ERRORS_ENABLE
#define IOTOKEN_EXTENDED_ERRORS_ENABLE
#endif


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Enable strict argument checking (input parameters)
#define IOTOKEN_STRICT_ARGS

#ifdef DDK_EIP197_FW_IOTOKEN_METADATA_ENABLE
// Number of 32-bit words passed from Input to Output Token
// without modifications
#define IOTOKEN_BYPASS_WORD_COUNT        4
#else
#define IOTOKEN_BYPASS_WORD_COUNT        0
#endif

/* end of file cs_iotoken.h */


#endif /* CS_IOTOKEN_H_ */
