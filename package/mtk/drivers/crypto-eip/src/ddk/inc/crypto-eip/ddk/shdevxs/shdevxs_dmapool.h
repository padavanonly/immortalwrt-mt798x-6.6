/* shdevxs_dmapool.h
 *
 * Shared Device Access DMA pool API.
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

#ifndef SHDEVXS_DMAPOOL_H_
#define SHDEVXS_DMAPOOL_H_

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "shdevxs_dmapool.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Types Definitions API
#include "basic_defs.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

typedef struct
{
    void * p;
} SHDevXS_Addr_t;

// DMA Pool data structure
typedef struct
{
    // Host (virtual) address of the DMA pool memory bank
    SHDevXS_Addr_t HostAddr;

    // DMA (physical) address of the DMA pool memory bank
    SHDevXS_Addr_t DMA_Addr;

    // Size if bytes of the DMA pool memory bank
    unsigned int ByteCount;

    // Handle to be used for subset allocation.
    int Handle;

    // True when DMA buffer is cached, otherwise false
    bool fCached;

    // DMA pool identifier
    unsigned int PoolId;

} SHDevXS_DMAPool_t;


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * SHDevXS_DMAPool_Init
 *
 * This function initializes the DMA Pool for the calling application.
 *
 * API use order:
 *     This function must be called once before any of the other functions.
 *
 * DMAPool_p (output)
 *     Pointer to memory location where the DMA Pool control data will
 *     be stored. Cannot be NULL.
 *
 * Return Value
 *     0  Success
 *    0>  Failure, code is implementation-specific
 */
int
SHDevXS_DMAPool_Init(
        SHDevXS_DMAPool_t * const DMAPool_p);


/*----------------------------------------------------------------------------
 * SHDevXS_DMAPool_GetBase
 *
 * This function obtains the base address of the entire DMA pool area.
 *
 * BaseAddr_p (output)
 *     Pointer to memory location where the DMA pool base address will
 *     be stored.
 *
 * Return Value
 *     0  Success
 *    0>  Failure, code is implementation-specific
 */
int
SHDevXS_DMAPool_GetBase(
        void ** BaseAddr_p);


/*----------------------------------------------------------------------------
 * SHDevXS_DMAPool_Uninit
 *
 * This function uninitializes and frees all the resources used by the calling
 * application for the DMA pool.
 *
 * API use order:
 *     This function must be called last, as clean-up step before stopping
 *     the application.
 *
 * Return Value
 *     None
 */
void
SHDevXS_DMAPool_Uninit(void);


#endif /* SHDEVXS_DMAPOOL_H_ */


/* end of file shdevxs_dmapool.h */
