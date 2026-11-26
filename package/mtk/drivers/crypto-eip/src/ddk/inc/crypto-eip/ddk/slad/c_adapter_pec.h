/* c_adapter_pec.h
 *
 * Default Adapter PEC configuration
 */

/*****************************************************************************
* Copyright (c) 2012-2021 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_C_ADAPTER_PEC_H
#define INCLUDE_GUARD_C_ADAPTER_PEC_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level Adapter configuration
#include "cs_adapter.h"


/****************************************************************************
 * Adapter general configuration parameters
 */

#ifndef ADAPTER_PEC_BANK_PACKET
#define ADAPTER_PEC_BANK_PACKET         0
#endif

#ifndef ADAPTER_PEC_BANK_TOKEN
#define ADAPTER_PEC_BANK_TOKEN          0
#endif

#ifndef ADAPTER_PEC_BANK_SA
#define ADAPTER_PEC_BANK_SA             0
#endif

//#define ADAPTER_PEC_STRICT_ARGS

#ifndef ADAPTER_PEC_DEVICE_COUNT
#define ADAPTER_PEC_DEVICE_COUNT        1
#endif

#ifndef ADAPTER_PEC_MAX_PACKETS
#define ADAPTER_PEC_MAX_PACKETS         32
#endif

#ifndef ADAPTER_PEC_MAX_LOGICDESCR
#define ADAPTER_PEC_MAX_LOGICDESCR      32
#endif

//#define ADAPTER_PEC_SEPARATE_RINGS
//#define ADAPTER_PEC_ENABLE_SCATTERGATHER
//#define ADAPTER_PEC_INTERRUPTS_ENABLE
//#define ADAPTER_PEC_ARMRING_ENABLE_SWAP

// Remove bounce buffers support
//#define ADAPTER_PEC_REMOVE_BOUNCEBUFFERS

// EIP-202 Ring manager device ID, keep undefined if RPM for EIP-202 not used
//#define ADAPTER_PEC_RPM_EIP202_DEVICE0_ID  0


#endif /* Include Guard */


/* end of file c_adapter_pec.h */
