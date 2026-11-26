/* eip202_ring_internal.h
 *
 * EIP-202 Ring Control Driver Library Internal interface
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

#ifndef EIP202_RING_INTERNAL_H_
#define EIP202_RING_INTERNAL_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip202_ring.h"       // EIP202_RING_STRICT_ARGS

// EIP-202 Ring Driver Library Types API
#include "eip202_ring_types.h"   // EIP202_* types

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint32_t

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t

// Driver Framework DMA Resource API
#include "dmares_types.h"       // types of the DMA resource API

// Ring Helper API
#include "ringhelper.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// RDR ownership word pattern
#define EIP202_RDR_OWNERSHIP_WORD_PATTERN       0xAAAAAAAAU

// CDR I/O Area, used internally
typedef struct
{
    Device_Handle_t Device;
    unsigned int State;
    RingHelper_t RingHelper;
    RingHelper_CallbackInterface_t RingHelperCallbacks;
    DMAResource_Handle_t RingHandle;
    uint32_t DescOffsWordCount;
    uint32_t RingSizeWordCount;
    uint32_t CountRD;
    bool fValidCountRD;
    bool fATP;
} EIP202_CDR_True_IOArea_t;

// RDR I/O Area, used internally
typedef struct
{
    Device_Handle_t Device;
    unsigned int State;
    RingHelper_t RingHelper;
    RingHelper_CallbackInterface_t RingHelperCallbacks;
    DMAResource_Handle_t RingHandle;
    uint32_t DescOffsWordCount;
    uint32_t RingSizeWordCount;
    uint32_t TokenOffsetWordCount;
    unsigned int AcknowledgedRDCount;
    unsigned int AcknowledgedPktCount;
    unsigned int RDToGetCount;
    unsigned int PktToGetCount;
    bool PacketFound;
} EIP202_RDR_True_IOArea_t;

#define CDRIOAREA(_p) ((volatile EIP202_CDR_True_IOArea_t *)_p)
#define RDRIOAREA(_p) ((volatile EIP202_RDR_True_IOArea_t *)_p)

#ifdef EIP202_RING_STRICT_ARGS
#define EIP202_RING_CHECK_POINTER(_p) \
    if (NULL == (_p)) \
        return EIP202_RING_ARGUMENT_ERROR;
#define EIP202_RING_CHECK_INT_INRANGE(_i, _min, _max) \
    if ((_i) < (_min) || (_i) > (_max)) \
        return EIP202_RING_ARGUMENT_ERROR;
#define EIP202_RING_CHECK_INT_ATLEAST(_i, _min) \
    if ((_i) < (_min)) \
        return EIP202_RING_ARGUMENT_ERROR;
#define EIP202_RING_CHECK_INT_ATMOST(_i, _max) \
    if ((_i) > (_max)) \
        return EIP202_RING_ARGUMENT_ERROR;
#else
/* EIP202_RING_STRICT_ARGS undefined */
#define EIP202_RING_CHECK_POINTER(_p)
#define EIP202_RING_CHECK_INT_INRANGE(_i, _min, _max)
#define EIP202_RING_CHECK_INT_ATLEAST(_i, _min)
#define EIP202_RING_CHECK_INT_ATMOST(_i, _max)
#endif /*end of EIP202_RING_STRICT_ARGS */

#define TEST_SIZEOF(type, size) \
    extern int size##_must_bigger[1 - 2*((int)(sizeof(type) > size))]

// validate the size of the fake and real IOArea structures
TEST_SIZEOF(EIP202_CDR_True_IOArea_t, EIP202_IOAREA_REQUIRED_SIZE);
TEST_SIZEOF(EIP202_RDR_True_IOArea_t, EIP202_IOAREA_REQUIRED_SIZE);


#endif /* EIP202_RING_INTERNAL_H_ */


/* end of file eip202_ring_internal.h */
