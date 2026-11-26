/* cs_shdevxs.h
 *
 * Configuration for Kernel Support Driver.
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

#ifndef CS_SHDEVXS_H_
#define CS_SHDEVXS_H_

#define LOG_SEVERITY_MAX LOG_SEVERITY_WARN

#define SHDEVXS_DEVICE_NAME  "EIP197_GLOBAL"

#define SHDEVXS_ENABLE_DMAPOOL_FUNCTIONS
//#define SHDEVXS_ENABLE_RC_FUNCTIONS
#define SHDEVXS_ENABLE_PRNG_FUNCTIONS

#define SHDEVXS_TR_BANK_SIZE (512*1024)  // Transform record memory bank size per application.
#define SHDEVXS_MAX_APPS      4          // Maximum number of applications

//#define SHDEVXS_LOCK_SLEEPABLE
//#define SHDEVXS_ENABLE_IRQ_CLEAR

#ifdef ARCH_ARM64
// Enable cache-coherent DMA buffer allocation
#define SHDEVXS_DMARESOURCE_ALLOC_CACHE_COHERENT
#endif


#define SHDEVXS_IRQ_COUNT 31 /* Maximum number of IRQs  */

#define SHDEVXS_NOF_PE_TO_USE          12


#endif /* CS_KSUPPORT_H_ */


/* end of file cs_ksupport.h */
