/* umdevxs_cmd.h
 *
 * Driver Command/Response data structure.
 * Used on character device interface between Kernel Driver and Proxy.
 */

/*****************************************************************************
* Copyright (c) 2009-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_UMDEVXS_CMD_H
#define INCLUDE_GUARD_UMDEVXS_CMD_H

// most operations are done through

enum
{
    UMDEVXS_OPCODE_INIT = 1,
    UMDEVXS_OPCODE_SHUTDOWN,

    UMDEVXS_OPCODE_SMBUF_ALLOC,
    // Alloc:
    //  In: Size(uint1), Bank(uint2), Alignment(uint3)
    // Out: Handle(Handle), ActualSize(uint1), DevAddr(ptr1)
    // currently only supports PAGE_SIZE-aligned, DMA-capable buffers.

    UMDEVXS_OPCODE_SMBUF_SETBUFINFO,
    // GetBufInfo:
    //  In: Handle, Size(uint1), Addr(ptr1)

    UMDEVXS_OPCODE_SMBUF_GETBUFINFO,
    // GetBufInfo:
    //  In: Handle
    // Out: Size(uint1), Addr(ptr1)

    UMDEVXS_OPCODE_SMBUF_REGISTER,
    // Register:
    //  In: Size(uint1), BufPtr(ptr1), Handle(Handle)
    // Out: NewHandle(Handle)
    // input handle must refer to an already allocated/attached buffer that
    // includes the registered buffer.

    UMDEVXS_OPCODE_SMBUF_FREE,
    // Free:
    //  In: Handle

    UMDEVXS_OPCODE_SMBUF_ATTACH,
    // Attach:
    //  In: DevAddr(ptr1), Size(uint1), Bank(uint2)
    // Out: Handle, Size(uint1)

    UMDEVXS_OPCODE_SMBUF_DETACH,
    // Detach:
    //  In: Handle

    UMDEVXS_OPCODE_SMBUF_COMMIT,
    // Commit:
    //  In: Handle, SubsetStart(uint1), SubsetLength(uint2)

    UMDEVXS_OPCODE_SMBUF_REFRESH,
    // Refresh:
    //  In: Handle, SubsetStart(uint1), SubsetLength(uint2)

    UMDEVXS_OPCODE_DEVICE_FIND,
    // Find:
    //   In: szName
    //  Out: Handle, Size(uint1)

    UMDEVXS_OPCODE_DEVICE_FINDRANGE,
    // Find device specified by range:
    //   In: BAR (uint1), Offset (unit2), Size (uint3)
    //  Out: Handle

    UMDEVXS_OPCODE_DEVICE_SETPRIV,
    // SetPrivileged.

    UMDEVXS_OPCODE_DEVICE_ENUM,
    // Enum:
    //   In: DeviceNr(uint1)
    //  Out: szName

    UMDEVXS_OPCODE_DEVICE_PCICFG_READ32,
    // PCI Cfg Read32:
    //   In: ByteOffset (uint1)
    //  Out: Int32 (uint2)

    UMDEVXS_OPCODE_DEVICE_PCICFG_WRITE32,
    // PCI Cfg Write32:
    //   In: ByteOffset (uint1), Int32 (uint2)


    UMDEVXS_OPCODE_LAST,
};


#define UMDEVXS_CMDRSP_MAXLEN_NAME 64

typedef struct
{
    int Magic;                  // in
    int Opcode;                 // in
    int Error;                  // out, 0 = no error

    int Handle;                 // in

    unsigned int uint1;
    unsigned int uint2;
    unsigned int uint3;
    void * ptr1;
    char szName[UMDEVXS_CMDRSP_MAXLEN_NAME + 1];  // zero-terminated

} UMDevXS_CmdRsp_t;


#define UMDEVXS_CMDRSP_MAGIC 876327949

#define UMDEVXS_INTERRUPT_TIMEOUT_MASK      0x00FFFFFFU
#define UMDEVXS_INTERRUPT_INT_ID_MASK       0x000000FFU
#define UMDEVXS_INTERRUPT_INT_ID_OFFSET     24


#endif /* INCLUDE_GUARD_UMDEVXS_CMD_H */

/* umdevxs_cmd.h */
