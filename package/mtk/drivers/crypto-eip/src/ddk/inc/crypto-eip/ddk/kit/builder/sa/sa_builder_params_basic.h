/* sa_builder_params_basic.h
 *
 * Basic crypto and hash specific extension to the SABuilder_Params_t type.
 */

/*****************************************************************************
* Copyright (c) 2011-2022 by Rambus, Inc. and/or its subsidiaries.
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


#ifndef SA_BUILDER_PARAMS_BASIC_H_
#define SA_BUILDER_PARAMS_BASIC_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "sa_builder_params.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/* Flag bits for the BasicFlags field. Combine any values using a
   bitwise or.
 */
#define SAB_BASIC_FLAG_EXTRACT_ICV   BIT_0 /* Extract and verify ICV from packet*/
#define SAB_BASIC_FLAG_ENCRYPT_AFTER_HASH BIT_1
/* Encrypt the hashed data (default is hashing the encrypted data). */
#define SAB_BASIC_FLAG_HMAC_PRECOMPUTE BIT_2
/* Special operation to precompute HMAC keys */
#define SAB_BASIC_FLAG_XFRM_API BIT_3
/* Specify bypass operation for XFRM API */


/* Extension record for SAParams_t. Protocol_Extension_p must point
   to this structure when the Basic crypto/hash protocol is used.

   SABuilder_Iinit_Basic() will fill all fields in this structure  with
   sensible defaults.
 */

typedef struct
{
    uint32_t BasicFlags;
    uint32_t DigestBlockCount;
    uint32_t ICVByteCount; /* Length of ICV in bytes. */
    uint32_t  fresh;      /* 32-bit 'fresh' value for wireless authentication
                             algorithms. */
    uint8_t bearer;       /* 5-bit 'bearer' value for wireless algorithms. */
    uint8_t direction;   /* 1-bit 'direction' value for wireless algorithms. */
    uint32_t ContextRef; /* Reference to application context */
} SABuilder_Params_Basic_t;


#endif /* SA_BUILDER_PARAMS_BASIC_H_ */


/* end of file sa_builder_params_basic.h */
