/* cs_driver_ext2.h
 *
 * Top-level Product EIP-197 hardware specific Configuration Settings.
 */

/*****************************************************************************
* Copyright (c) 2012-2022 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_CS_DRIVER_EXT2_H
#define INCLUDE_GUARD_CS_DRIVER_EXT2_H
#include "cs_ddk197.h"

// Disable usage of the EIP-202 Ring Arbiter
//#define DRIVER_EIP202_RA_DISABLE


// Enables DMA banks
#ifdef DRIVER_64BIT_DEVICE
#define DRIVER_DMARESOURCE_BANKS_ENABLE
#endif

// Number of Ring interfaces to use
#define DRIVER_MAX_NOF_RING_TO_USE                     14

#ifdef DDK_EIP197_EIP96_SEQMASK_1024
#define EIP97_EIP96_CTX_SIZE 0x52
#endif
#endif /* Include Guard */


/* end of file cs_driver_ext2.h */
