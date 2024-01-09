/* eip97_global_event.h
 *
 * EIP-97 Global Control Driver Library API:
 * Event Management use case
 *
 * Refer to the EIP-97 Driver Library User Guide for information about
 * re-entrance and usage from concurrent execution contexts of this API
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

#ifndef EIP97_GLOBAL_EVENT_H_
#define EIP97_GLOBAL_EVENT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint8_t, uint16_t, bool

// EIP-97 Global Control Driver Library Types API
#include "eip97_global_types.h" // EIP97_* types


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// EIP-97 Debug statistics counters.
typedef struct
{
    // Input packets for each interface.
    uint64_t Ifc_Packets_In[16];
    // Output packets for each interface.
    uint64_t Ifc_Packets_Out[16];
    // Total processed packets for each processing pipe.
    uint64_t Pipe_Total_Packets[16];
    // Total amount of data processed in each pipe.
    uint64_t Pipe_Data_Count[16];
    // Current number of packets in each pipe.
    uint8_t Pipe_Current_Packets[16];
    // Maximum number of packets in each pipe.
    uint8_t Pipe_Max_Packets[16];
} EIP97_Global_Debug_Statistics_t;


// EIP-97 Data Fetch Engine (DFE) thread status,
// 1 DFE thread corresponds to 1 Processing Engine (PE)
typedef struct
{
    // Number of 32-bit words currently available from the assigned CD FIFO for
    // this thread
    uint16_t CDFifoWord32Count;

    // This field contains the number of the CDR from which the thread is
    // currently processing Command Descriptors. A value of all ones
    // (reset value) means unassigned, in that case all other thread status
    // information is invalid
    uint8_t CDR_ID;

    // Size of the current DMA operation for this thread, only valid
    // if fAtDMABusy or fDataDMABusy are true
    uint16_t DMASize;

    // When true the thread is currently busy DMA-ing Additional Token data
    // to the Processing Engine
    bool fAtDMABusy;

    // When true the thread is currently busy DMA-ing packet data to
    // the Processing Engine
    bool fDataDMABusy;

    // FATAL ERROR when true requiring Global SW or HW Reset.
    // True when a DMA error was detected, the thread is stopped as
    // a result of this. A DFE thread error interrupt for thread n occurs
    // simultaneously with the assertion of this bit.
    // Further information may be extracted from the other thread status
    // registers.
    // Note: A DMA error is a Host bus Master interface read/write error and is
    // qualified as a fatal error. The result is unknown and in the worst case
    // it can cause a system hang-up. The only correct way to recover is
    // to issue a complete system reset (HW Reset or Global SW Reset).
    bool fDMAError;
} EIP97_Global_DFE_Status_t;

// EIP-97 Data Store Engine (DSE) thread status,
// 1 DSE thread corresponds to 1 Processing Engine (PE)
typedef struct
{
    // Number of 32-bit words currently available from the assigned RD FIFO for
    // this thread
    uint16_t RDFifoWord32Count;

    // This field contains the number of the RDR from which the thread is
    // currently processing Result Descriptors. A value of all ones
    // (reset value) means unassigned, in that case all other thread status
    // information is invalid
    uint8_t RDR_ID;

    // Size of the current DMA operation for this thread, only valid
    // if fDataDMABusy is true
    uint16_t DMASize;

    // When true the thread is currently busy flushing any remaining packet data
    // from the Processing Engine either because it did not fit into
    // the reserved packet buffer or because a destination not allowed
    // interrupt (DSE thread n Irq) was fired
    bool fDataFlushBusy;

    // When true the thread is currently busy DMA-ing packet data from the
    // Processing Engine to Host memory.
    bool fDataDMABusy;

    // FATAL ERROR when true requiring Global SW or HW Reset.
    // True when a DMA error was detected, the thread is stopped as
    // a result of this. A DSE thread error interrupt for thread n occurs
    // simultaneously with the assertion of this bit.
    // Further information may be extracted from the other thread status
    // registers.
    // Note: A DMA error is a Host bus Master interface read/write error and is
    // qualified as a fatal error. The result is unknown and in the worst case
    // it can cause a system hang-up. The only correct way to recover is
    // to issue a complete system reset (HW Reset or Global SW Reset).
    bool fDMAError;
} EIP97_Global_DSE_Status_t;

// EIP-96 Token Status
typedef struct
{
    // Number of tokens located in the EIP-96, result token not included
    // (maximum is two)
    uint8_t ActiveTokenCount;

    // If true then a new token can be read by the EIP-96
    bool fTokenLocationAvailable;

    // If true then a (partial) result token is available in the EIP-96
    bool fResultTokenAvailable;

    // If true then a token is currently read by the EIP-96
    bool fTokenReadActive;

    // If true then the context cache contains a new context
    bool fContextCacheActive;

    // If true then the context cache is currently filled
    bool fContextFetch;

    // If true then the context cache contains result context data that
    // needs to be updated
    bool fResultContext;

    // If true then no (part of) tokens are in the EIP-96 and no context
    // update is required
    bool fProcessingHeld;

    // If true then packet engine is busy (a context is active)
    bool fBusy;
} EIP96_Token_Status_t;

// EIP-96 Context Status
typedef struct
{
    /*
     * Packet processing error bit mask:
        error_0  Packet length error
        error_1  Token error, unknown token command/instruction.
        error_2  Token contains to much bypass data.
        error_3  Cryptographic block size error.
        error_4  Hash block size error (basic hash only).
        error_5  Invalid command/algorithm/mode/combination.
        error_6  Prohibited algorithm.
        error_7  Hash input overflow (basic hash only).
        error_8  TTL / HOP-limit underflow.
        error_9  Authentication failed.
        error_10 Sequence number check failed / roll-over detected.
        error_11 SPI check failed.
        error_12 Checksum incorrect.
        error_13 Pad verification failed.
        error_14 Internal Packet Engine time-out:
                 FATAL ERROR when set requiring Global SW or HW Reset.
        error_15 Reserved error bit, will never be set to 1b.
     */
    uint16_t Error;

    // Number of available tokens is the sum of new, active and result tokens
    // that are available
    uint8_t AvailableTokenCount;

    // True indicates that a context is active
    bool fActiveContext;

    // True indicates that a new context is (currently) loaded
    bool fNextContext;

    // True indicates that a result context data needs to be stored.
    // Result context and next context cannot be both active.
    bool fResultContext;

    // True indicates that an existing error condition has not yet been properly
    // handled to completion. Note that the next packet context and data fetch
    // can be started. In addition, error bits may still be active due
    // to the previous packet.
    bool fErrorRecovery;
} EIP96_Context_Status_t;

// EIP-96 Interrupt Status
typedef struct
{
    // FATAL ERROR when true requiring Global SW or HW Reset.
    // True when the input fetch engine does not properly receive all packet
    // data.
    bool fInputDMAError;

    // True when the output store engine does not properly store all packet data
    bool fOutputDMAError;

    // A logic OR of the error_0 up to and including error_7 of the Error field
    // in the EIP-96 Context Status
    bool fPacketProcessingError;

    // FATAL ERROR when true requiring Global SW or HW Reset.
    // True when the Internal Packet Engine time-out, copy of error_14 from
    // the Error field in the EIP-96 Context Status
    // Fatal Error that requires the engine reset (via HW Reset or
    // Global SW Reset)
    bool fPacketTimeout;

    // FATAL ERROR when true requiring Global SW or HW Reset.
    // True when Fatal internal error within EIP-96 Packet Engine is detected,
    // reset of engine required via HW Reset or Global SW Reset).
    bool fFatalError;

    // If true then there is at least one pending EIP-96 interrupt
    bool fPeInterruptOut;

    // If true then the input_dma_error interrupt is enabled
    bool fInputDMAErrorEnabled;

    // If true then the output_dma_error interrupt is enabled
    bool fOutputDMAErrorEnabled;

    // If true then the packet_processin interrupt is enabled
    bool fPacketProcessingEnabled;

    // If true then the packet_timeout interrupt is enabled
    bool fPacketTimeoutEnabled;

    // If true then the packet_timeout interrupt is enabled
    bool fFatalErrorEnabled;

    // If true then the EIP-96 interrupt output is enabled.
    // If false then the EIP-96 interrupts will never become active.
    bool fPeInterruptOutEnabled;
} EIP96_Interrupt_Status_t;

// EIP-96 Output Transfer Status
typedef struct
{
    // Number of (32-bit) words that are available in the 2K Bytes data
    // output buffer, shows value 255 when more than 255 words are available.
    uint8_t AvailableWord32Count;

    // Minimum number of words that is transferred per beat (fixed)
    uint8_t MinTransferWordCount;

    // Maximum number of words that can be transferred per beat (fixed)
    uint8_t MaxTransferWordCount;

    // Masks the number of available word entries to obtain an optimal
    // transfer size (fixed)
    uint8_t TransferSizeMask;
} EIP96_Output_Transfer_Status_t;

// EIP-96 PRNG Status
typedef struct
{
    // True when the PRNG is busy generating a Pseudo-Random Number
    bool fBusy;

    // True when a valid Pseudo-Random Number is available
    bool fResultReady;
} EIP96_PRNG_Status_t;


/*----------------------------------------------------------------------------
 * EIP97_Global_Debug_Statistics_Get
 *
 * This function returns debug statistics information in
 * the EIP97_Global_Debug_Statistics_t data structure.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * Debug_Statistics_p (output)
 *     Pointer to the data structure where the debug statistics
 *     will be stored.
 *
 * This function is re-entrant.
 *
 * Return value
 *     EIP97_GLOBAL_NO_ERROR : operation is completed
 *     EIP97_GLOBAL_ARGUMENT_ERROR : Passed wrong argument
 *     EIP97_GLOBAL_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP97_Global_Error_t
EIP97_Global_Debug_Statistics_Get(
        EIP97_Global_IOArea_t * const IOArea_p,
        EIP97_Global_Debug_Statistics_t * const Debug_Statistics_p);


/*----------------------------------------------------------------------------
 * EIP97_Global_DFE_Status_Get
 *
 * This function returns hardware status information in
 * the EIP97_Global_DFE_Status_t data structure.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * PE_Number (input)
 *     Number of the PE for which the DFE status must be obtained.
 *
 * DFE_Status_p (output)
 *     Pointer to the data structure where the DFE status
 *     will be stored.
 *
 * This function is re-entrant.
 *
 * Return value
 *     EIP97_GLOBAL_NO_ERROR : operation is completed
 *     EIP97_GLOBAL_ARGUMENT_ERROR : Passed wrong argument
 *     EIP97_GLOBAL_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP97_Global_Error_t
EIP97_Global_DFE_Status_Get(
        EIP97_Global_IOArea_t * const IOArea_p,
        const unsigned int PE_Number,
        EIP97_Global_DFE_Status_t * const DFE_Status_p);


/*----------------------------------------------------------------------------
 * EIP97_Global_DSE_Status_Get
 *
 * This function returns hardware status information in
 * the EIP97_Global_DSE_Status_t data structure.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * PE_Number (input)
 *     Number of the PE for which the DSE status must be obtained.
 *
 * DSE_Status_p (output)
 *     Pointer to the data structure where the DSE status
 *     will be stored.
 *
 * This function is re-entrant.
 *
 * Return value
 *     EIP97_GLOBAL_NO_ERROR : operation is completed
 *     EIP97_GLOBAL_ARGUMENT_ERROR : Passed wrong argument
 *     EIP97_GLOBAL_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP97_Global_Error_t
EIP97_Global_DSE_Status_Get(
        EIP97_Global_IOArea_t * const IOArea_p,
        const unsigned int PE_Number,
        EIP97_Global_DSE_Status_t * const DSE_Status_p);


/*----------------------------------------------------------------------------
 * EIP97_Global_EIP96_Token_Status_Get
 *
 * This function returns hardware status information in
 * the EIP96_Token_Status_t data structure.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * PE_Number (input)
 *     Number of the PE for which the status must be obtained.
 *
 * Token_Status_p (output)
 *     Pointer to the data structure where the EIP-96 Token status
 *     will be stored.
 *
 * This function is re-entrant.
 *
 * Return value
 *     EIP97_GLOBAL_NO_ERROR : operation is completed
 *     EIP97_GLOBAL_ARGUMENT_ERROR : Passed wrong argument
 *     EIP97_GLOBAL_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP97_Global_Error_t
EIP97_Global_EIP96_Token_Status_Get(
        EIP97_Global_IOArea_t * const IOArea_p,
        const unsigned int PE_Number,
        EIP96_Token_Status_t * const Token_Status_p);


/*----------------------------------------------------------------------------
 * EIP97_Global_EIP96_Context_Status_Get
 *
 * This function returns hardware status information in
 * the EIP96_Context_Status_t data structure.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * PE_Number (input)
 *     Number of the PE for which the status must be obtained.
 *
 * Context_Status_p (output)
 *     Pointer to the data structure where the EIP-96 Context status
 *     will be stored.
 *
 * This function is re-entrant.
 *
 * Return value
 *     EIP97_GLOBAL_NO_ERROR : operation is completed
 *     EIP97_GLOBAL_ARGUMENT_ERROR : Passed wrong argument
 *     EIP97_GLOBAL_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP97_Global_Error_t
EIP97_Global_EIP96_Context_Status_Get(
        EIP97_Global_IOArea_t * const IOArea_p,
        const unsigned int PE_Number,
        EIP96_Context_Status_t * const Context_Status_p);


/*----------------------------------------------------------------------------
 * EIP97_Global_EIP96_OutXfer_Status_Get
 *
 * This function returns hardware status information in
 * the EIP96_Output_Transfer_Status_t data structure.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * PE_Number (input)
 *     Number of the PE for which the status must be obtained.
 *
 * OutXfer_Status_p (output)
 *     Pointer to the data structure where the EIP-96 Output Transfer status
 *     will be stored.
 *
 * This function is re-entrant.
 *
 * Return value
 *     EIP97_GLOBAL_NO_ERROR : operation is completed
 *     EIP97_GLOBAL_ARGUMENT_ERROR : Passed wrong argument
 *     EIP97_GLOBAL_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP97_Global_Error_t
EIP97_Global_EIP96_OutXfer_Status_Get(
        EIP97_Global_IOArea_t * const IOArea_p,
        const unsigned int PE_Number,
        EIP96_Output_Transfer_Status_t * const OutXfer_Status_p);


/*----------------------------------------------------------------------------
 * EIP97_Global_EIP96_PRNG_Status_Get
 *
 * This function returns hardware status information in
 * the EIP96_PRNG_Status_t data structure.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area.
 *
 * PE_Number (input)
 *     Number of the PE for which the status must be obtained.
 *
 * OutXfer_Status_p (output)
 *     Pointer to the data structure where the EIP-96 PRNG status
 *     will be stored.
 *
 * This function is re-entrant.
 *
 * Return value
 *     EIP97_GLOBAL_NO_ERROR : operation is completed
 *     EIP97_GLOBAL_ARGUMENT_ERROR : Passed wrong argument
 *     EIP97_GLOBAL_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP97_Global_Error_t
EIP97_Global_EIP96_PRNG_Status_Get(
        EIP97_Global_IOArea_t * const IOArea_p,
        const unsigned int PE_Number,
        EIP96_PRNG_Status_t * const PRNG_Status_p);


#endif /* EIP97_GLOBAL_EVENT_H_ */


/* end of file eip97_global_event.h */
