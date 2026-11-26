/* eip202_cd_format.h
 *
 * EIP-202 Ring Control Driver Library Command Descriptor Internal interface
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : ddk197                                                   */
/*   Version       : 5.6.1                                                    */
/*   Configuration : DDK-197-GPL                                              */
/*                                                                            */
/*   Date          : 2022-Dec-16                                              */
/*                                                                            */
/* Copyright (c) 2008-2022 by Rambus, Inc. and/or its subsidiaries.           */
/*                                                                            */
/* This program is free software: you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation, either version 2 of the License, or          */
/* any later version.                                                         */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               */
/* GNU General Public License for more details.                               */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program. If not, see <http://www.gnu.org/licenses/>.       */
/* -------------------------------------------------------------------------- */

#ifndef EIP202_CD_FORMAT_H_
#define EIP202_CD_FORMAT_H_


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// Descriptor I/O Driver Library API implementation
#include "eip202_cdr.h"                // EIP202_ARM_CommandDescriptor_t


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"                // bool, uint32_t, uint8_t

// Driver Framework DMA Resource API
#include "dmares_types.h"              // DMAResource_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP202_CD_Make_ControlWord
 *
 * This helper function returns the Control Word that can be written to
 * the EIP-202 Command Descriptor.
 *
 * This function is re-entrant.
 *
 */
uint32_t
EIP202_CD_Make_ControlWord(
        const uint8_t TokenWordCount,
        const uint32_t SegmentByteCount,
        const bool fFirstSegment,
        const bool fLastSegment,
        const bool fForceEngine,
        const uint8_t EngineId);


/*----------------------------------------------------------------------------
 * EIP202_CD_Write
 *
 * This helper function writes the EIP-202 Logical Command Descriptor to the CDR
 *
 * This function is not re-entrant.
 *
 */
void
EIP202_CD_Write(
        DMAResource_Handle_t Handle,
        const unsigned int WordOffset,
        const EIP202_ARM_CommandDescriptor_t * const Descr_p,
        const bool fATP);


#endif /* EIP202_CD_FORMAT_H_ */


/* end of file eip202_cd_format.h */
