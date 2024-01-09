/* eip207_global_config.h
 *
 * EIP-207 Global Control Driver Library
 * Configuration Module
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

#ifndef EIP207_GLOBAL_CONFIG_H_
#define EIP207_GLOBAL_CONFIG_H_


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // bool

// Driver Framework Device API
#include "device_types.h"               // Device_Handle_t

// Data type for firmware config
#include "eip207_global_init.h"

// EIP-207 Global Control Driver Library Internal interfaces
#include "eip207_level0.h"              // EIP207_ICE_REG_SCRATCH_RAM

// EIP-207c Firmware Classification API
#include "firmware_eip207_api_cs.h"     // FIRMWARE_EIP207_CS_*


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/*----------------------------------------------------------------------------
 * Local variables
 */


/*--------------------- -------------------------------------------------------
 * EIP207_Global_Firmware_Configure
 */
static inline void
EIP207_Global_Firmware_Configure(
        const Device_Handle_t Device,
        const unsigned int CE_Number,
        const EIP207_Firmware_Config_t * const FWConfig_p)
{
    unsigned int ByteOffset;
#ifdef FIRMWARE_EIP207_CS_ADMIN_RAM_DTLS_HDR_SIZE_BYTE_OFFSET
    uint32_t Value;
#endif
    // Set input and output token format (with or without meta-data)
    ByteOffset = FIRMWARE_EIP207_CS_ADMIN_RAM_TOKEN_FORMAT_BYTE_OFFSET;

    EIP207_Write32(Device,
                   EIP207_ICE_REG_SCRATCH_RAM(CE_Number) + ByteOffset,
                   FWConfig_p->fTokenExtensionsEnable ? 1 : 0);
#ifdef FIRMWARE_EIP207_CS_ADMIN_RAM_DTLS_HDR_SIZE_BYTE_OFFSET
    ByteOffset = FIRMWARE_EIP207_CS_ADMIN_RAM_DTLS_HDR_SIZE_BYTE_OFFSET;
    Value = FWConfig_p->DTLSRecordHeaderAlign;
    if (FWConfig_p->fDTLSDeferCCS)
        Value |= BIT_15;
    if (FWConfig_p->fDTLSDeferAlert)
        Value |= BIT_13;
    if (FWConfig_p->fDTLSDeferHandshake)
        Value |= BIT_13;
    if (FWConfig_p->fDTLSDeferAppData)
        Value |= BIT_12;
    if (FWConfig_p->fDTLSDeferCAPWAP)
        Value |= BIT_11;
    EIP207_Write32(Device,
                   EIP207_ICE_REG_SCRATCH_RAM(CE_Number) + ByteOffset,
                   Value);
#endif
#ifdef EIP207_OCE_REG_SCRATCH_RAM // Test if we have this scratch RAM.
#ifdef FIRMWARE_EIP207_CS_ADMIN_RAM_PKTID_BYTE_OFFSET
        ByteOffset = FIRMWARE_EIP207_CS_ADMIN_RAM_PKTID_BYTE_OFFSET;
        EIP207_Write32(Device,
                       EIP207_OCE_REG_SCRATCH_RAM(CE_Number) + ByteOffset,
                       FWConfig_p->fIncrementPktID ?
                       FWConfig_p->PktID | BIT_16 :
                       0);
#endif
#ifdef FIRMWARE_EIP207_CS_ADMIN_RAM_ECN_CONTROL_BYTE_OFFSET
        ByteOffset = FIRMWARE_EIP207_CS_ADMIN_RAM_ECN_CONTROL_BYTE_OFFSET;
        EIP207_Write32(Device,
                       EIP207_OCE_REG_SCRATCH_RAM(CE_Number) + ByteOffset,
                       FWConfig_p->ECNControl);
#endif
#ifdef FIRMWARE_EIP207_CS_ADMIN_RAM_REDIR_BYTE_OFFSET
        ByteOffset = FIRMWARE_EIP207_CS_ADMIN_RAM_REDIR_BYTE_OFFSET;
        EIP207_Write32(Device,
                       EIP207_OCE_REG_SCRATCH_RAM(CE_Number) + ByteOffset,
                       FWConfig_p->RedirRing |
                       (FWConfig_p->fRedirRingEnable << 16) |
                       (FWConfig_p->fAlwaysRedirect << 17));
#endif
#endif
#ifdef FIRMWARE_EIP207_CS_ADMIN_RAM_REDIR_BYTE_OFFSET
        ByteOffset = FIRMWARE_EIP207_CS_ADMIN_RAM_REDIR_BYTE_OFFSET;
        EIP207_Write32(Device,
                       EIP207_ICE_REG_SCRATCH_RAM(CE_Number) + ByteOffset,
                       FWConfig_p->TransformRedirEnable);
#endif
}


#endif // EIP207_GLOBAL_CONFIG_H_


/* end of file eip207_global_config.h */
