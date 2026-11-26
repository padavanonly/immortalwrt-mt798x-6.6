/* c_adapter_eip74.h
 *
 * Default Adapter Global configuration for EIP-74
 */

/*****************************************************************************
* Copyright (c) 2017-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_C_ADAPTER_EIP74_H
#define INCLUDE_GUARD_C_ADAPTER_EIP74_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level Adapter configuration
#include "cs_adapter.h"


#ifndef ADAPTER_EIP74_DEVICE_NAME
#define ADAPTER_EIP74_DEVICE_NAME            "EIP74"
#endif // ADAPTER_EIP74_DEVICE_NAME

#ifndef ADAPTER_EIP74_RESET_MAX_RETRIES
#define ADAPTER_EIP74_RESET_MAX_RETRIES      1000
#endif // ADAPTER_EIP74_RESET_MAX_RETRIES


// EIP-74 generate block size
#ifndef ADAPTER_EIP74_GEN_BLK_SIZE
#define ADAPTER_EIP74_GEN_BLK_SIZE           4095
#endif

// EIP-74 reseed threshold.
#ifndef ADAPTER_EIP74_RESEED_THR
#define ADAPTER_EIP74_RESEED_THR             0xffffffff
#endif

// EIP-74 reaseed early wrning threshold.
#ifndef ADAPTER_EIP74_RESEED_THR_EARLY
#define ADAPTER_EIP74_RESEED_THR_EARLY       0xff000000
#endif

// Define if EIP-74 uses interrupts.
//ADAPTER_EIP74_INTERRUPTS_ENABLE

#ifdef ADAPTER_EIP74_INTERRUPTS_ENABLE
#ifndef ADAPTER_EIP74_ERR_IRQ
#define ADAPTER_EIP74_ERR_IRQ           0
#endif
#ifndef ADAPTER_EIP74_RES_IRQ
#define ADAPTER_EIP74_RES_IRQ           0
#endif
#endif

// EIP-74 DRBG device ID, keep undefined if RPM for EIP-74 not used
//#define ADAPTER_PEC_RPM_EIP74_DEVICE0_ID  0


#endif /* Include Guard */


/* end of file c_adapter_eip74.h */
