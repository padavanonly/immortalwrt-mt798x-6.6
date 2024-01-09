/* shdevxs_cmd.h
 *
 * Opcodes (additional to those used by UMDevXS drive) for SHDevXS.
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

#ifndef SHDEVXS_CMD_H_
#define SHDEVXS_CMD_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "umdevxs_cmd.h"


enum {
    UMDEVXS_OPCODE_SHDEVXS_GLOBAL_INIT = UMDEVXS_OPCODE_LAST,
    UMDEVXS_OPCODE_SHDEVXS_GLOBAL_UNINIT,
    UMDEVXS_OPCODE_SHDEVXS_TEST,
    UMDEVXS_OPCODE_SHDEVXS_DMAPOOL_INIT,
    UMDEVXS_OPCODE_SHDEVXS_DMAPOOL_UNINIT,
    UMDEVXS_OPCODE_SHDEVXS_DMAPOOL_GETBASE,
    UMDEVXS_OPCODE_SHDEVXS_RC_TRC_INVALIDATE,
    UMDEVXS_OPCODE_SHDEVXS_RC_ARC4RC_INVALIDATE,
    UMDEVXS_OPCODE_SHDEVXS_RC_LOCK,
    UMDEVXS_OPCODE_SHDEVXS_RC_FREE,
    UMDEVXS_OPCODE_SHDEVXS_IRQ_INIT,
    UMDEVXS_OPCODE_SHDEVXS_IRQ_UNINIT,
    UMDEVXS_OPCODE_SHDEVXS_IRQ_ENABLE,
    UMDEVXS_OPCODE_SHDEVXS_IRQ_DISABLE,
    UMDEVXS_OPCODE_SHDEVXS_IRQ_CLEAR,
    UMDEVXS_OPCODE_SHDEVXS_IRQ_CLEARANDENABLE,
    UMDEVXS_OPCODE_SHDEVXS_IRQ_SETHANDLER,
    UMDEVXS_OPCODE_SHDEVXS_IRQ_WAIT,
    UMDEVXS_OPCODE_SHDEVXS_PRNG_RESEED,
    UMDEVXS_OPCODE_SHDEVXS_SUPPORTEDFUNCS_GET,
};

#endif /* SHDEVXS_CMD_H_ */

/* shdevxs_cmd.h */
