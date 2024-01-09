/* log.c
 *
 * Log implementation for specific environment
 */

/*****************************************************************************
* Copyright (c) 2008-2020 by Rambus, Inc. and/or its subsidiaries.
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

#define LOG_SEVERITY_MAX  LOG_SEVERITY_NO_OUTPUT

// Logging API
#include "log.h"            // the API to implement


/*----------------------------------------------------------------------------
 * Log_HexDump
 *
 * This function logs Hex Dump of a Buffer
 *
 * szPrefix
 *     Prefix to be printed on every row.
 *
 * PrintOffset
 *     Offset value that is printed at the start of every row. Can be used
 *     when the byte printed are located at some offset in another buffer.
 *
 * Buffer_p
 *     Pointer to the start of the array of bytes to hex dump.
 *
 * ByteCount
 *     Number of bytes to include in the hex dump from Buffer_p.
 *
 * Return Value
 *     None.
 */
void
Log_HexDump(
        const char * szPrefix_p,
        const unsigned int PrintOffset,
        const uint8_t * Buffer_p,
        const unsigned int ByteCount)
{
    unsigned int i;

    for(i = 0; i < ByteCount; i += 16)
    {
        unsigned int j, Limit;

        // if we do not have enough data for a full line
        if (i + 16 > ByteCount)
            Limit = ByteCount - i;
        else
            Limit = 16;

        Log_FormattedMessage("%s %08d:", szPrefix_p, PrintOffset + i);

        for (j = 0; j < Limit; j++)
            Log_FormattedMessage(" %02X", Buffer_p[i+j]);

        Log_FormattedMessage("\n");
    } // for
}


/*----------------------------------------------------------------------------
 * Log_HexDump32
 *
 * This function logs Hex Dump of an array of 32-bit words
 *
 * szPrefix
 *     Prefix to be printed on every row.
 *
 * PrintOffset
 *     Offset value that is printed at the start of every row. Can be used
 *     when the byte printed are located at some offset in another buffer.
 *
 * Buffer_p
 *     Pointer to the start of the array of 32-bit words to hex dump.
 *
 * Word32Count
 *     Number of 32-bit words to include in the hex dump from Buffer_p.
 *
 * Return Value
 *     None.
 */
void
Log_HexDump32(
        const char * szPrefix_p,
        const unsigned int PrintOffset,
        const uint32_t * Buffer_p,
        const unsigned int Word32Count)
{
    unsigned int i;

    for(i = 0; i < Word32Count; i += 4)
    {
        unsigned int j, Limit;

        // if we do not have enough data for a full line
        if (i + 4 > Word32Count)
            Limit = Word32Count - i;
        else
            Limit = 4;

        Log_FormattedMessage("%s %08d:", szPrefix_p, PrintOffset + i*4);

        for (j = 0; j < Limit; j++)
            Log_FormattedMessage(" %08X", Buffer_p[i+j]);

        Log_FormattedMessage("\n");
    } // for
}

/* end of file log.c */
