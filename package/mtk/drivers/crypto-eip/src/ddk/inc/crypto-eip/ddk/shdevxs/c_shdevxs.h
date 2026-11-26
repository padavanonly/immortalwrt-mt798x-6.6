/* c_shdevxs.h
 *
 * Configuration for Kernel Support Driver.
 */

/*****************************************************************************
* Copyright (c) 2012-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef C_SHDEVXS_H_
#define C_SHDEVXS_H_

#include "cs_shdevxs.h"

#ifndef LOG_SEVERITY_MAX
#define LOG_SEVERITY_MAX LOG_SEVERITY_WARN
#endif

// Number of Packet Engines to use
#ifndef SHDEVXS_NOF_PE_TO_USE
#define SHDEVXS_NOF_PE_TO_USE       1
#endif

/* Set this to enable DMA pool functions */
//#define SHDEVXS_ENABLE_DMAPOOOL_FUNCTIONS

/* Set this to enable record cache functions */
//#define SHDEVXS_ENABLE_RC_FUNCTIONS

/* Set this to enable EIP-96 PRNG reseed function */
//#define SHDEVXS_ENABLE_PRNG_FUNCTIONS

/* Set this to enable the SHDevXS_IRQ_Clear functions */
//#define SHDEVXS_ENABLE_IRQ_CLEAR

#ifdef SHDEVXS_ENABLE_RC_FUNCTIONS
#define SHDEVXS_ENABLE_DMAPOOL_FUNCTIONS
#endif

#if defined(SHDEVXS_ENABLE_RC_FUNCTIONS) || defined(SHDEVXS_ENABLE_PRNG_FUNCTIONS)
#define SHDEVXS_ENABLE_FUNCTIONS
#ifndef SHDEVXS_DEVICE_NAME
#error "SHDEVXS_DEVICE_NAME is not defined"
#endif
#endif

#ifndef SHDEVXS_TR_BANK_SIZE
#define SHDEVXS_TR_BANK_SIZE (128*1024)  // Transform record memory bank size per application.
#endif

#ifndef SHDEVXS_MAX_APPS
#define SHDEVXS_MAX_APPS      8          // Maximum number of applications
#endif

// Specifes the number of IRQs supported by the driver.
#ifndef SHDEVXS_IRQ_COUNT
#define SHDEVXS_IRQ_COUNT     1
#endif

// Enable cache-coherent DMA buffer allocation
//#define SHDEVXS_DMARESOURCE_ALLOC_CACHE_COHERENT

//#define SHDEVXS_LOCK_SLEEPABLE

// RPM Device identifier to use for EIP-96 PRNG hardware runtime power management
// Keep undefined if RPM for device is not used
//#define SHDEVXS_PRNG_RPM_DEVICE_ID  0

// RPM Device identifier to use for EIP-201 AIC hardware runtime power management
// Keep undefined if RPM for device is not used
//#define SHDEVXS_IRQ_RPM_DEVICE_ID   1

// RPM Device identifier to use for EIP-207 RC hardware runtime power management
// Keep undefined if RPM for device is not used
//#define SHDEVXS_RC_RPM_DEVICE_ID    2


#endif /* C_SHDEVXS_H_ */


/* end of file c_shdevxs.h */
