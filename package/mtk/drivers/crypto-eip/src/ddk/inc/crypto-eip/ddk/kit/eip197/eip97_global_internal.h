/* eip97_global_internal.h
 *
 * EIP-97 Global Control Driver Library Internal interface
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

#ifndef EIP97_GLOBAL_INTERNAL_H_
#define EIP97_GLOBAL_INTERNAL_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip97_global.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint8_t, uint32_t

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t

// EIP-97 Driver Library Types API
#include "eip97_global_types.h" // EIP97_* types

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define PE_DEFAULT_NR     0  // Default Processing Engine number

#if (PE_DEFAULT_NR >= EIP97_GLOBAL_MAX_NOF_PE_TO_USE)
#error "Error: PE_DEFAULT_NR must be less than EIP97_GLOBAL_MAX_NOF_PE_TO_USE"
#endif

// I/O Area, used internally
typedef struct
{
    Device_Handle_t Device;
    uint32_t State;
} EIP97_True_IOArea_t;

#define IOAREA(_p) ((volatile EIP97_True_IOArea_t *)_p)

#ifdef EIP97_GLOBAL_STRICT_ARGS
#define EIP97_GLOBAL_CHECK_POINTER(_p) \
    if (NULL == (_p)) \
        return EIP97_GLOBAL_ARGUMENT_ERROR;
#define EIP97_GLOBAL_CHECK_INT_INRANGE(_i, _min, _max) \
    if ((_i) < (_min) || (_i) > (_max)) \
        return EIP97_GLOBAL_ARGUMENT_ERROR;
#define EIP97_GLOBAL_CHECK_INT_ATLEAST(_i, _min) \
    if ((_i) < (_min)) \
        return EIP97_GLOBAL_ARGUMENT_ERROR;
#define EIP97_GLOBAL_CHECK_INT_ATMOST(_i, _max) \
    if ((_i) > (_max)) \
        return EIP97_GLOBAL_ARGUMENT_ERROR;
#else
/* EIP97_GLOBAL_STRICT_ARGS undefined */
#define EIP97_GLOBAL_CHECK_POINTER(_p)
#define EIP97_GLOBAL_CHECK_INT_INRANGE(_i, _min, _max)
#define EIP97_GLOBAL_CHECK_INT_ATLEAST(_i, _min)
#define EIP97_GLOBAL_CHECK_INT_ATMOST(_i, _max)
#endif /*end of EIP97_GLOBAL_STRICT_ARGS */

#define TEST_SIZEOF(type, size) \
    extern int size##_must_bigger[1 - 2*((int)(sizeof(type) > size))]

// validate the size of the fake and real IOArea structures
TEST_SIZEOF(EIP97_True_IOArea_t, EIP97_GLOBAL_IOAREA_REQUIRED_SIZE);


/*----------------------------------------------------------------------------
 * EIP97_Interfaces_Get
 */
void
EIP97_Interfaces_Get(
    unsigned int * const NofPEs_p,
    unsigned int * const NofRings_p,
    unsigned int * const NofLA_p,
    unsigned int * const NofIN_p);

/*----------------------------------------------------------------------------
 * EIP97_DFEDSE_Offset_Get
 */
unsigned int
EIP97_DFEDSE_Offset_Get(void);

#endif /* EIP97_GLOBAL_INTERNAL_H_ */


/* end of file eip97_global_internal.h */
