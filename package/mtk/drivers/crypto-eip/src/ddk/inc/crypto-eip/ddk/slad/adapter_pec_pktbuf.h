/* adapter_pec_pktbuf.h
 *
 * Helper functions to access packet data via DMABuf handles, possibly
 * in scatter-gather lists.
 */

/*****************************************************************************
* Copyright (c) 2020-2021 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef ADAPTER_PEC_PKTBUF_H_
#define ADAPTER_PEC_PKTBUF_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"            // bool

// DMABuf API
#include "api_dmabuf.h"            // DMABuf_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Adapter_PEC_PktData_Get
 *
 * Obtain packet data from a DMA Buf handle in a contiguous buffer.
 * If the data already exists in a contiguous buffer, return a pointer to it.
 * If the desired data is spread across scatter particles, copy it into supplied
 * copy buffer.
 *
 * PacketHandle (input)
 *     DMABuf handle representing the packet (either single buffer or scatter
 *     list).
 *
 * CopyBuffer_p (input)
 *     Buffer to which the packet data must be copied if not contiguous.
 *     Can be NULL if data is assumed to be contiguous.
 *
 * StartOffs (input)
 *     Byte offset from start of the packet of first byte of packet to access.
 *
 * ByteCount (input)
 *     Length of packet data to access.
 *
 * Return:
 *    Non-null pointer; access to packet data.
 *    NULL pointer: scatter list does not contain desired offset or supplied
 *                  copy buffer pointer was NULL and data was spread.
 */
uint8_t *
Adapter_PEC_PktData_Get(
        DMABuf_Handle_t PacketHandle,
        uint8_t *CopyBuffer_p,
        unsigned int StartOffs,
        unsigned int ByteCount);


/*----------------------------------------------------------------------------
 * Adapter_PEC_PktByte_Put
 *
 * Write single byte at specific offset in packet data represented by
 * a DMABuf handle. This can be either a single buffer or a scatter list.
 *
 * PacketHandle (input)
 *     DMABuf handle representing the packet to update.
 *
 * Offset (input)
 *     Byte offset within the packet to update.
 *
 * Byte (input)
 *     Byte value to write.
 */
void
Adapter_PEC_PktByte_Put(
        DMABuf_Handle_t PacketHandle,
        unsigned int Offset,
        unsigned int Byte);


#endif /* ADAPTER_PEC_PKTBUF_H_ */


/* end of file adapter_pec_pktbuf.h */
