/* eip202_cdr_dscr.h
 *
 * EIP-202 Ring Control Driver Library API Command Descriptor Internal interface
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

#ifndef EIP202_CDR_DSCR_H_
#define EIP202_CDR_DSCR_H_


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */



/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP202_CDR_WriteCB
 * A write callback for the Ring Helper
 */
int
EIP202_CDR_WriteCB(
        void * const CallbackParam1_p,
        const int CallbackParam2,
        const unsigned int WriteIndex,
        const unsigned int WriteCount,
        const unsigned int TotalWriteLimit,
        const void * Descriptors_p,
        const int DescriptorCount,
        const unsigned int DescriptorSkipCount);


/*----------------------------------------------------------------------------
 * EIP202_CDR_ReadCB
 * A read callback for the Ring Helper
 */
int
EIP202_CDR_ReadCB(
        void * const CallbackParam1_p,
        const int CallbackParam2,
        const unsigned int ReadIndex,
        const unsigned int ReadLimit,
        void * Descriptors_p,
        const unsigned int DescriptorSkipCount);


/*----------------------------------------------------------------------------
 * EIP202_CDR_StatusCB
 * A status callback for the Ring Helper
 */

int
EIP202_CDR_StatusCB(
        void * const CallbackParam1_p,
        const int CallbackParam2,
        int * const DeviceReadPos_p);


#endif /* EIP202_CDR_DSCR_H_ */


/* end of file eip202_cdr_dscr.h */
