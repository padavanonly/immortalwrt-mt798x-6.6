/* c_iotoken.h
 *
 * Default configuration file
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

#ifndef C_IOTOKEN_H_
#define C_IOTOKEN_H_

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level configuration
#include "cs_iotoken.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Enable extended errors
//#define IOTOKEN_EXTENDED_ERRORS_ENABLE

// Enable strict argument checking (input parameters)

//#define IOTOKEN_STRICT_ARGS

// Number of 32-bit words passed from Input to Output Token
// without modifications
#ifndef IOTOKEN_BYPASS_WORD_COUNT
#define IOTOKEN_BYPASS_WORD_COUNT        0
#endif

#ifndef LOG_SEVERITY_MAX
#define LOG_SEVERITY_MAX LOG_SEVERITY_CRIT
#endif


// Enable PKt ID increment
//#IOTOKEN_INCREMENT_PKTID

#ifndef IOTOKEN_PADDING_DEFAULT_ON
#define IOTOKEN_PADDING_DEFAULT_ON 0
#endif

// Header Align for inbound DTLS
#ifndef IOTOKEN_DTLS_HDR_ALIGN
#define IOTOKEN_DTLS_HDR_ALIGN 0
#endif


/* end of file c_iotoken.h */


#endif /* C_IOTOKEN_H_ */
