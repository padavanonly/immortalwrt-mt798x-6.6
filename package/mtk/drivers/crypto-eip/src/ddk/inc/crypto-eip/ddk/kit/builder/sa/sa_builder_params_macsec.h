/* sa_builder_params_macsec.h
 *
 * MACsec specific extension to the SABuilder_Params_t type.
 */

/*****************************************************************************
* Copyright (c) 2013-2020 by Rambus, Inc. and/or its subsidiaries.
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


#ifndef SA_BUILDER_PARAMS_MACSEC_H_
#define SA_BUILDER_PARAMS_MACSEC_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "sa_builder_params.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Flag bits for the MACsecFlags field. Combine any values using a
   bitwise or.
 */
#define SAB_MACSEC_ES              BIT_0 /* End Station */
#define SAB_MACSEC_SC              BIT_1 /* Include SCI in frames */
#define SAB_MACSEC_SCB             BIT_2 /* Enable EPON Single Channel Broadcase */


/* Extension record for SAParams_t. Protocol_Extension_p must point
   to this structure when the MACsec  protocol is used.

   SABuilder_Iinit_MACsec() will fill all fields in this structure  with
   sensible defaults.
 */
typedef struct
{
    uint32_t MACsecFlags;  /* See SAB_MACSEC_* flag bits above*/
    const uint8_t *SCI_p;        /* Pointer to 8-byte SCI */
    uint8_t AN;            /* Association Number */

    uint32_t SeqNum;       /* Sequence number.*/

    uint32_t ReplayWindow; /* Size of the anti-replay window */

    uint32_t ConfOffset; /* Confidentiality Offset. Specify a number of
                                unencrypted bytes at the start of each packet.*/
    uint32_t ContextRef; /* Reference to application context */
} SABuilder_Params_MACsec_t;


#endif /* SA_BUILDER_PARAMS_MACSEC_H_ */


/* end of file sa_builder_params_macsec.h */
