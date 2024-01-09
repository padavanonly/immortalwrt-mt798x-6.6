/* sa_builder_ipsec.h
 *
 * IPsec specific functions of the SA Builder.
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


#ifndef  SA_BUILDER_IPSEC_H_
#define SA_BUILDER_IPSEC_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "sa_builder_params.h"
#include "sa_builder.h"
#include "sa_builder_params_ipsec.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"


/*----------------------------------------------------------------------------
 * SABuilder_Init_ESP
 *
 * This function initializes the SABuilder_Params_t data structure and its
 * SABuilder_Params_IPsec_t extension with sensible defaults for ESP
 * processing.
 *
 * SAParams_p (output)
 *   Pointer to SA parameter structure to be filled in.
 * SAParamsIPsec_p (output)
 *   Pointer to IPsec parameter extension to be filled in
 * spi (input)
 *   SPI of the newly created parameter structure (must not be zero).
 * TunnelTransport (input)
 *   Must be one of SAB_IPSEC_TUNNEL or SAB_IPSEC_TRANSPORT.
 * IPMode (input)
 *   Must be one of SAB_IPSEC_IPV4 or SAB_IPSEC_IPV6.
 * direction (input)
 *   Must be one of SAB_DIRECTION_INBOUND or SAB_DIRECTION_OUTBOUND.
 *
 * Both the crypto and the authentication algorithm are initialized to
 * NULL, which is illegal according to the IPsec standards, but it is
 * possible to use this setting for debug purposes.
 *
 * Both the SAParams_p and SAParamsIPsec_p input parameters must point
 * to valid storage where variables of the appropriate type can be
 * stored. This function initializes the link from SAParams_p to
 * SAParamsIPsec_p.
 *
 * Return:
 * SAB_STATUS_OK on success
 * SAB_INVALID_PARAMETER when one of the pointer parameters is NULL
 *   or the remaining parameters have illegal values.
 */
SABuilder_Status_t
SABuilder_Init_ESP(
    SABuilder_Params_t * const SAParams_p,
    SABuilder_Params_IPsec_t * const SAParamsIPsec_p,
    const uint32_t spi,
    const uint32_t TunnelTransport,
    const uint32_t IPMode,
    const SABuilder_Direction_t direction);


#endif /* SA_BUILDER_IPSEC_H_ */


/* end of file sa_builder_ipsec.h */
