/* c_adapter_global.h
 *
 * Default Adapter Global configuration
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

#ifndef INCLUDE_GUARD_C_ADAPTER_GLOBAL_H
#define INCLUDE_GUARD_C_ADAPTER_GLOBAL_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level Adapter configuration
#include "cs_adapter.h"

#ifndef ADAPTER_GLOBAL_DRIVER_NAME
#define ADAPTER_GLOBAL_DRIVER_NAME     "Security"
#endif

#ifndef ADAPTER_GLOBAL_LICENSE
#define ADAPTER_GLOBAL_LICENSE         "GPL"
#endif

#ifndef ADAPTER_GLOBAL_PRNG_SEED
// 8 words to seed the PRNG to provide IVs, input to
#define ADAPTER_GLOBAL_PRNG_SEED {0x48c24cfd, 0x6c07f742, \
                                  0xaee75681, 0x0f27c239, \
                                  0x79947198, 0xe2991275, \
                                  0x21ac3c7c, 0xd008c4b4}
#endif

#ifndef ADAPTER_GLOBAL_DEVICE_NAME
#define ADAPTER_GLOBAL_DEVICE_NAME           "EIP197_GLOBAL"
#endif // ADAPTER_GLOBAL_DEVICE_NAME

#ifndef ADAPTER_GLOBAL_RESET_MAX_RETRIES
#define ADAPTER_GLOBAL_RESET_MAX_RETRIES      1000
#endif // ADAPTER_GLOBAL_RESET_MAX_RETRIES

#ifndef ADAPTER_GLOBAL_EIP97_NOF_PES
#define ADAPTER_GLOBAL_EIP97_NOF_PES          1
#endif

#ifndef ADAPTER_GLOBAL_EIP97_RINGMASK
#define ADAPTER_GLOBAL_EIP97_RINGMASK         0x0001
#endif

#ifndef ADAPTER_GLOBAL_EIP97_PRIOMASK
#define ADAPTER_GLOBAL_EIP97_PRIOMASK         0
#endif

// Enables PCI device support
//#define ADAPTER_GLOBAL_PCI_SUPPORT_ENABLE

// Enables board control device support
//#define ADAPTER_GLOBAL_BOARDCTRL_SUPPORT_ENABLE

// Enables board reset via the board control device
//#define ADAPTER_GLOBAL_FPGA_HW_RESET_ENABLE

// Runtime Power Management EIP-97 device identifier
#ifndef ADAPTER_GLOBAL_RPM_EIP97_DEVICE_ID
#define ADAPTER_GLOBAL_RPM_EIP97_DEVICE_ID  0
#endif

#ifndef ADAPTER_GLOBAL_RPM_EIP207_DEVICE_ID
#define ADAPTER_GLOBAL_RPM_EIP207_DEVICE_ID 0
#endif

// Specify a redirect ring.
//#define ADAPTER_CS_GLOBAL_REDIR_RING 7


#endif /* Include Guard */


/* end of file c_adapter_global.h */
