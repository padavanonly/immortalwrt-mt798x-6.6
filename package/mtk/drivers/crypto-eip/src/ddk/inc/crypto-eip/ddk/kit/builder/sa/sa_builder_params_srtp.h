/* sa_builder_params_srtp.h
 *
 * SRTP specific extension to the SABuilder_Params_t type.
 */

/*****************************************************************************
* Copyright (c) 2011-2020 by Rambus, Inc. and/or its subsidiaries.
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


#ifndef SA_BUILDER_PARAMS_SRTP_H_
#define SA_BUILDER_PARAMS_SRTP_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "sa_builder_params.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"



/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Flag values for te SRTPFlags field */
#define SAB_SRTP_FLAG_SRTCP          BIT_0
#define SAB_SRTP_FLAG_INCLUDE_MKI    BIT_1


/* Extension record for SAParams_t. Protocol_Extension_p must point
   to this structure when the SRTP protocol is used.

   SABuilder_Iinit_SRTP() will fill all fields in this structure  with
   sensible defaults.
 */
typedef struct
{
    uint32_t SRTPFlags;
    uint32_t MKI;
    uint32_t ICVByteCount; /* Length of ICV in bytes. */
} SABuilder_Params_SRTP_t;


#endif /* SA_BUILDER_PARAMS_SRTP_H_ */


/* end of file sa_builder_params_srtp.h */
