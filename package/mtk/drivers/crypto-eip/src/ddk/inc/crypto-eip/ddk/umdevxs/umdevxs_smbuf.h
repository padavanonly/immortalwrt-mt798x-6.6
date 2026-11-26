/* umdevxs_smbuf.h
 *
 * Exported SMBuf API of the UMDexXS/UMPCI driver.
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

#ifndef INCLUDE_GUARD_UMDEVXS_SMBUF_H
#define INCLUDE_GUARD_UMDEVXS_SMBUF_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Static DMA bank with fixed address type for UMDevXS
#define UMDEVXS_SHMEM_BANK_STATIC_FIXED_ADDR          99


/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_SetAppID
 */
void
UMDevXS_SMBuf_SetAppID(
        int Handle,
        void * AppID);

/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_Register
 *
 * This function must be used to register an "alien" buffer that was allocated
 * somewhere else. The caller guarantees that this buffer can be used for DMA.
 *
 * Size (input)
 *     Size of the buffer to be registered (in bytes).
 *
 * Buffer_p (input)
 *     Pointer to the buffer. This pointer must be valid to use on the host
 *     in the domain of the driver.
 *
 * Alternative_p (input)
 *     Some allocators return two addresses. This parameter can be used to
 *     pass this second address to the driver. The type is pointer to ensure
 *     it is always large enough to hold a system address, also in LP64
 *     architecture. Set to NULL if not used.
 *
 * Handle_p (output)
 *     Pointer to the memory location when the handle will be returned.
 *
 * Return Values
 *     DMABUF_STATUS_OK: Success, Handle_p was written.
 *     DMABUF_ERROR_BAD_ARGUMENT
 */
int
UMDevXS_SMBuf_Register(
        const int Size,
        void * Buffer_p,
        void * Alternative_p,
        int * const Handle_p);


/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_Release
 *
 * Free the DMA resource (unless not allocated locally) and the record used
 * to describe it.
 *
 */
int
UMDevXS_SMBuf_Release(
    int Handle);




#endif /* INCLUDE_GUARD_UMDEVXS_SMBUF_H */

/* umdevxs_smbuf.h */
