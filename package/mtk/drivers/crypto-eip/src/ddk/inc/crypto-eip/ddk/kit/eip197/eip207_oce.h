/* eip207_oce.h
 *
 * EIP-207c Output Classification Engine (OCE) interface
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

#ifndef EIP207_OCE_H_
#define EIP207_OCE_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// EIP-207 Global Control Init API
#include "eip207_global_init.h"         // EIP207_*


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // bool, uint32_t

// Driver Framework Device API
#include "device_types.h"               // Device_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * EIP207_OCE_Firmware_Load
 */
EIP207_Global_Error_t
EIP207_OCE_Firmware_Load(
        const Device_Handle_t Device,
        const unsigned int TimerPrescaler,
        EIP207_Firmware_t * const PUE_Firmware_p,
        EIP207_Firmware_t * const FPP_Firmware_p);


/*----------------------------------------------------------------------------
 * EIP207_OCE_GlobalStats_Get
 */
EIP207_Global_Error_t
EIP207_OCE_GlobalStats_Get(
        const Device_Handle_t Device,
        const unsigned int CE_Number,
        EIP207_Global_OCE_GlobalStats_t * const OCE_GlobalStats_p);


/*----------------------------------------------------------------------------
 * EIP207_OCE_ClockCount_Get
 */
EIP207_Global_Error_t
EIP207_OCE_ClockCount_Get(
        const Device_Handle_t Device,
        const unsigned int CE_Number,
        EIP207_Global_Value64_t * const OCE_Clock_p);


/*-----------------------------------------------------------------------------
 * EIP207_OCE_Status_Get
 */
EIP207_Global_Error_t
EIP207_OCE_Status_Get(
        const Device_Handle_t Device,
        const unsigned int CE_Number,
        EIP207_Global_CE_Status_t * const OCE_Status_p);


/* end of file eip207_oce.h */


#endif /* EIP207_OCE_H_ */
