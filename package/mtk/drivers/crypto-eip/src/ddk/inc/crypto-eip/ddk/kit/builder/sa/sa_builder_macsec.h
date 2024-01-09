/* sa_builder_macsec.h
 *
 * MACsec specific functions of the SA Builder.
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


#ifndef  SA_BUILDER_MACSEC_H_
#define SA_BUILDER_MACSEC_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "sa_builder_params.h"
#include "sa_builder.h"
#include "sa_builder_params_macsec.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"


/*----------------------------------------------------------------------------
 * SABuilder_Init_MACsec
 *
 * This function initializes the SABuilder_Params_t data structure and its
 * SABuilder_Params_MACsec_t extension with sensible defaults for MACsec
 * processing.
 *
 * SAParams_p (output)
 *   Pointer to SA parameter structure to be filled in.
 * SAParamsMACsec_p (output)
 *   Pointer to MACsec parameter extension to be filled in
 * SCI_p (input)
 *   Pointer to Secure Channel Identifier, 8 bytes.
 * AN (input)
 *   Association number, a number for 0 to 3.
 * direction (input)
 *   Must be one of SAB_DIRECTION_INBOUND or SAB_DIRECTION_OUTBOUND.
 *
 * Both the crypto and the authentication algorithm are initialized to
 * NULL. The crypto algorithm (which may remain NULL) must be set to
 * one of the algorithms supported by the protocol. The authentication
 * algorithm must also be set to one of the algorithms supported by
 * the protocol..Any required keys have to be specified as well.
 *
 * Both the SAParams_p and SAParamsMACsec_p input parameters must point
 * to valid storage where variables of the appropriate type can be
 * stored. This function initializes the link from SAParams_p to
 * SAParamsMACsec_p.
 *
 * Return:
 * SAB_STATUS_OK on success
 * SAB_INVALID_PARAMETER when one of the pointer parameters is NULL
 *   or the remaining parameters have illegal values.
 */
SABuilder_Status_t
SABuilder_Init_MACsec(
    SABuilder_Params_t * const SAParams_p,
    SABuilder_Params_MACsec_t * const SAParamsMACsec_p,
    const uint8_t *SCI_p,
    const uint8_t AN,
    const SABuilder_Direction_t direction);


#endif /* SA_BUILDER_MACSEC_H_ */


/* end of file sa_builder_macsec.h */
