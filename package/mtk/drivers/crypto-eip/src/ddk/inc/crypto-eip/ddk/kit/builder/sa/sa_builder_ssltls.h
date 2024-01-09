/* sa_builder_ssltls.h
 *
 * SSL/TLS/DTLS specific functions of the SA Builder.
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


#ifndef  SA_BUILDER_SSLTLS_H_
#define SA_BUILDER_SSLTLS_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "sa_builder_params.h"
#include "sa_builder.h"
#include "sa_builder_params_ssltls.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"


/*----------------------------------------------------------------------------
 * SABuilder_Init_SSLTLS
 *
 * This function initializes the SABuilder_Params_t data structure and its
 * SABuilder_Params_SSLTLS_t extension with sensible defaults for SSL, TLS
 * and DTLS processing.
 *
 * SAParams_p (output)
 *   Pointer to SA parameter structure to be filled in.
 * SAParamsSSLTLS_p (output)
 *   Pointer to SSLTLS parameter extension to be filled in
 * version (input)
 *   Version code for the desired protcol (choose one of the SAB_*_VERSION_*
 *   constants from sa_builder_params_ssltls.h).
 * direction (input)
 *   Must be one of SAB_DIRECTION_INBOUND or SAB_DIRECTION_OUTBOUND.
 *
 * Both the crypto and the authentication algorithm are initialized to
 * NULL. The crypto algorithm (which may remain NULL) must be set to
 * one of the algorithms supported by the protocol. The authentication
 * algorithm must also be set to one of the algorithms supported by
 * the protocol..Any required keys have to be specified as well.
 *
 * Both the SAParams_p and SAParamsSSLTLS_p input parameters must point
 * to valid storage where variables of the appropriate type can be
 * stored. This function initializes the link from SAParams_p to
 * SAParamsSSSLTLS_p.
 *
 * Return:
 * SAB_STATUS_OK on success
 * SAB_INVALID_PARAMETER when one of the pointer parameters is NULL
 *   or the remaining parameters have illegal values.
 */
SABuilder_Status_t
SABuilder_Init_SSLTLS(
    SABuilder_Params_t * const SAParams_p,
    SABuilder_Params_SSLTLS_t * const SAParamsSSLTLS_p,
    const uint16_t version,
    const SABuilder_Direction_t direction);


#endif /* SA_BUILDER_SSLTLS_H_ */


/* end of file sa_builder_ssltls.h */
