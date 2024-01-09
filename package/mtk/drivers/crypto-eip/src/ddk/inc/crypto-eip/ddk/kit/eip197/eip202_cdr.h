/* eip202_cdr.h
 *
 * EIP-202 Driver Library API:
 * Command Descriptor Ring (CDR)
 *
 * All the CDR API functions can be used concurrently with the RDR API functions
 * for any ring interface ID unless the API function description states
 * otherwise.
 *
 * Refer to the EIP-202 Driver Library User Guide for more information about
 * usage of this API.
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

#ifndef EIP202_CDR_H_
#define EIP202_CDR_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip202_ring.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"             // uint32_t, bool

// Driver Framework Device API
#include "device_types.h"           // Device_Handle_t

// EIP-202 Ring Control Driver Library Common Types API
#include "eip202_ring_types.h"       // EIP202_* common types


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Command descriptor Control Data size (in 32-bit words)
// Note: This is valid for ATP mode only!
#ifdef EIP202_64BIT_DEVICE
#define EIP202_CD_CTRL_DATA_MAX_WORD_COUNT  6
#else
#define EIP202_CD_CTRL_DATA_MAX_WORD_COUNT  3
#endif // !EIP202_64BIT_DEVICE

// CDR Status information
typedef struct
{
    // Fatal errors (interrupts), set to true when pending
    // CDR must be re-initialized in case of a fatal error
    bool fDMAError;
    bool fError;
    bool fOUFlowError;

    // Threshold Interrupt, set to true when pending
    bool fTresholdInt;

    // Timeout Interrupt, set to true when pending
    bool fTimeoutInt;

    // Number of 32-bit words that are currently free
    // in the Command Descriptor FIFO
    uint16_t CDFIFOWordCount;

    // Number of valid 32-bit Prepared Command Descriptor words that
    // are prepared in the CDR
    uint32_t CDPrepWordCount;

    // Number of valid 32-bit Processed Command Descriptor words that
    // are processed in the CDR
    uint32_t CDProcWordCount;

    // Number of full packets (i.e. the number of descriptors marked Last)
    // that are fully processed by the DFE and not yet acknowledged by Host.
    // If more than 127 packets are processed this field returns the value 127.
    uint8_t CDProcPktWordCount;
} EIP202_CDR_Status_t;

// CDR settings
typedef struct
{
    // Additional Token Pointer Descriptor Mode
    // When true the token data can be stored in a separate from the descriptor
    // DMA buffer
    bool                        fATP;

    // When true then the tokens consisting out of 1 or 2 32-bit words
    // can be passed to the PE directly via the command descriptor
    bool                        fATPtoToken;

    // Other ARM settings
    EIP202_ARM_Ring_Settings_t  Params;

} EIP202_CDR_Settings_t;

// Control word parameters for the Logical Command Descriptor
typedef struct
{
    // Set to true for the first descriptor in the descriptor chain
    bool        fFirstSegment;

    // Set to true for the last descriptor in the descriptor chain
    bool        fLastSegment;

    // Segment size in bytes, can be 0 for an empty segment
    uint32_t    SegmentByteCount;

    // Token size in 32-bit words,
    // important for the first segment only but can be 0 too,
    // must be always 0 for the non-first segments
    uint8_t     TokenWordCount;

    // Force the command to be processed on a specific engine.
    bool        fForceEngine;

    // Engine ID to process the command on.
    uint8_t     EngineId;

} EIP202_CDR_Control_t;

// Logical Command Descriptor
typedef struct
{
    // control fields for the command descriptor
    // EIP202_CDR_Write_ControlWord() helper function
    // can be used for obtaining this word
    uint32_t ControlWord;

    // Token header word
    uint32_t TokenHeader;

    // This parameter is copied through from command to result descriptor
    // unless EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS configuration parameter
    // is defined in c_eip202_ring.h
    uint32_t ApplicationId;

    // Source packet data length, in bytes
    unsigned int SrcPacketByteCount;

    // Source packet data, has to be provided by the caller:
    // Physical address that can be used by Device DMA
    EIP202_DeviceAddress_t SrcPacketAddr;

    // Context Data DMA buffer, has to be allocated and filled in by the caller
    // Physical address that can be used by Device DMA
    EIP202_DeviceAddress_t TokenDataAddr;

    // Context Data DMA buffer, has to be allocated and filled in by the caller
    // Physical address that can be used by Device DMA
    EIP202_DeviceAddress_t ContextDataAddr;

    // Input Token buffer with fixed size
    uint32_t * Token_p;

} EIP202_ARM_CommandDescriptor_t;


/*----------------------------------------------------------------------------
 * CDR Initialization API functions
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * EIP202_CDR_Init
 *
 * This function performs the initialization of the EIP-202 CDR
 * interface and transits the API to the Initialized state.
 *
 * This function returns the EIP202_RING_UNSUPPORTED_FEATURE_ERROR error code
 * when it detects a mismatch in the Ring Control Driver Library configuration.
 *
 * Note: This function should be called either after the EIP-202 HW Reset or
 *       after the Ring SW Reset, see the EIP202_CDR_Reset() function.
 *       This function as well as optionally EIP202_CDR_Reset() function must be
 *       executed before any other of the EIP202_CDR_*() functions can be called.
 *
 * This function cannot be called concurrently with
 * the EIP202_CDR_Reset() function for the same Device.
 *
 * IOArea_p (output)
 *     Pointer to the place holder in memory for the IO Area
 *     for the CDR instance identified by the Device parameter.
 *
 * Device (input)
 *     Handle for the Ring Control device instance returned by Device_Find
 *     for this CDR instance.
 *
 * CDRSettings_p (input)
 *     Pointer to the data structure that contains CDR configuration parameters
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * Return value
 *     EIP202_RING_NO_ERROR : operation is completed
 *     EIP202_RING_UNSUPPORTED_FEATURE_ERROR : not supported by the device.
 *     EIP202_RING_ARGUMENT_ERROR : Passed wrong argument
 *     EIP202_RING_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP202_Ring_Error_t
EIP202_CDR_Init(
        EIP202_Ring_IOArea_t * IOArea_p,
        const Device_Handle_t Device,
        const EIP202_CDR_Settings_t * const CDRSettings_p);


/*----------------------------------------------------------------------------
 * EIP202_CDR_Reset
 *
 * This function performs the CDR SW Reset and transits the API
 * to the Uninitialized state. This function must be called before using
 * the other CDR API functions if the state of the ring is Unknown.
 *
 * This function can be used to recover the CDR from a fatal error.
 *
 * Note: This function must be called before calling the EIP202_CDR_Init()
 *       function only if the EIP-202 HW Reset was not done. Otherwise it still
 *       is can be called but it is not necessary.
 *
 * This function cannot be called concurrently with
 * the EIP202_CDR_Init() function for the same Device.
 *
 * IOArea_p (output)
 *     Pointer to the place holder in memory for the IO Area
 *     for the CDR instance identified by the Device parameter.
 *
 * Device (input)
 *     Handle for the Ring Control device instance returned by Device_Find
 *     for this CDR instance.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * Return value
 *     EIP202_RING_NO_ERROR : operation is completed
 *     EIP202_RING_ARGUMENT_ERROR : Passed wrong argument
 *     EIP202_RING_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP202_Ring_Error_t
EIP202_CDR_Reset(
        EIP202_Ring_IOArea_t * const IOArea_p,
        const Device_Handle_t Device);

/*----------------------------------------------------------------------------
 * EIP202_CDR_Options_Get
 *
 * This function reads the local options register of the CDR device and
 * returns the information in a data structure of type EIP202_Ring_Options_t.
 *
 * Note: the options register (and therefore this function) is not available
 *       in all versions of the EIP202.
 *
 * This function can be called in any state, even when the device is not
 * yet initialized.
 *
 * Device (input)
 *     Handle for the Ring Control device instance returned by Device_Find
 *     for this CDR instance.
 *
 * Options_p (output)
 *     Pointer to the options data structure to be filled in by this
 *     function.
 *
 * This function is re-entrant for any Device.
 *
 * Return value
 *     EIP202_RING_NO_ERROR : operation is completed
 *     EIP202_RING_UNSUPPORTED_FEATURE_ERROR : not supported by the device.
 *     EIP202_RING_ARGUMENT_ERROR : Passed wrong argument
 */
EIP202_Ring_Error_t
EIP202_CDR_Options_Get(
        const Device_Handle_t Device,
        EIP202_Ring_Options_t * const Options_p);


/*----------------------------------------------------------------------------
 * CDR Descriptor I/O API functions
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * EIP202_CDR_FillLevel_Get
 *
 * This function outputs the fill level of the ring requested by means of
 * the Device parameter in the I/O Area. The fill level is obtained as a number
 * of command descriptors that have been submitted by the Host to the CDR but
 * not processed by the device yet.
 *
 * When the CDR is in the "Ring Full" state this function outputs the fill
 * level equal to the CDR size. When the CDR is in the "Ring Empty" state this
 * functions outputs the zero fill level. the CDR is in the "Ring Free" state
 * when this function returns the fill level that is greater than zero and less
 * than the CDR size (in command descriptors).
 *
 * This function cannot be called concurrently with
 * the EIP202_CDR_Descriptor_Put() function for the same Device.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area
 *     for the CDR instance which contains the Device handle.
 *
 * FillLevelDscrCount_p (output)
 *     Pointer to the memory location where the number of command
 *     descriptors pending in the CDR will be stored
 *     CDR Empty: FillLevel = 0
 *     CDR Free:  0 < FillLevel < RingSize (in descriptors)
 *     CDR Full:  FillLevel = RingSize (in descriptors)
 *
 * This function is re-entrant for any Device.
 *
 * Return value
 *     EIP202_RING_NO_ERROR : operation is completed
 *     EIP202_RING_ARGUMENT_ERROR : Passed wrong argument
 *     EIP202_RING_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP202_Ring_Error_t
EIP202_CDR_FillLevel_Get(
        EIP202_Ring_IOArea_t * const IOArea_p,
        unsigned int * FillLevelDscrCount_p);


/*----------------------------------------------------------------------------
 * EIP202_CDR_Write_ControlWord
 *
 * This helper function returns the control word that can be written to
 * the logical command descriptor.
 *
 * This function is re-entrant.
 *
 */
uint32_t
EIP202_CDR_Write_ControlWord(
        const EIP202_CDR_Control_t * const  CommandCtrl_p);


/*----------------------------------------------------------------------------
 * EIP202_CDR_Descriptor_Put
 *
 * This function puts a requested number of command descriptors to the CDR.
 * The function returns EIP202_RING_NO_ERROR and zero descriptors done count
 * (*DscrDoneCount_p set to 0) when no descriptors can be added to the CDR,
 * e.g. "Ring Full" state.
 *
 * This function can be called after the EIP202_CDR_FillLevel_Get()
 * function when the latter checks how many command descriptors can be added
 * to the CDR.
 *
 * The execution context calling this API function must
 *
 * 1) Provide input data via the CDR using the EIP202_CDR_Descriptor_Put()
 * function before it or another context can obtain output data from the RDR.
 * This requirement is relevant for the look-aside use case only.
 *
 * 2) Ensure that the Token Data, Packet Data and Context Data DMA buffers are
 * not re-used or freed by it or another context until the processed result
 * descriptor(s) referring to the packet associated with these buffers is(are)
 * fully processed by the Device. This is required not only when in-place
 * packet transform is done using the same Packet Data DMA buffer as input and
 * output buffer but also when different DMA buffers are used for the packet
 * processing input and output data.
 *
 * 3) Keep the packet descriptor chain state consistent. All descriptors that
 * belong to the same packet must be submitted atomically into the CDR without
 * being intermixed with descriptors that belong to other packets.
 *
 * 4) Submit the descriptors that belong to the same packet descriptor chain in
 * the right order, e.g. first descriptor for the first segment followed by
 * the middle descriptors followed by the last descriptor.
 *
 * 5) Ensure it does not call this function concurrently with another context
 * for the same Device.
 *
 * 6) Ensure it does not call this function concurrently with another context
 * calling the EIP202_CDR_FillLevel_Get() function for the same Device.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area
 *     for the CDR instance which contains the Device handle.
 *
 * CommandDscr_p (input)
 *     Pointer to 1st in the the array of command descriptors.
 *
 * DscrRequestedCount (input)
 *     Number of descriptors stored back-to-back in the array
 *     pointed to by CommandDscr_p.
 *
 * DscrDoneCount_p (output)
 *     Pointer to the memory location where the number of descriptors
 *     actually added to the CDR will be stored.
 *
 * FillLevelDscrCount_p (output)
 *     Pointer to the memory location where the number of command
 *     descriptors pending in the CDR will be stored
 *     CDR Empty: FillLevel = 0
 *     CDR Free:  0 < FillLevel < RingSize (in descriptors)
 *     CDR Full:  FillLevel = RingSize (in descriptors)
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * Return value
 *     EIP202_RING_NO_ERROR : operation is completed
 *     EIP202_RING_UNSUPPORTED_FEATURE_ERROR : feature is not supported
 *     EIP202_RING_ARGUMENT_ERROR : passed wrong argument
 *     EIP202_RING_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP202_Ring_Error_t
EIP202_CDR_Descriptor_Put(
        EIP202_Ring_IOArea_t * const IOArea_p,
        const EIP202_ARM_CommandDescriptor_t * CommandDscr_p,
        const unsigned int DscrRequestedCount,
        unsigned int * const DscrDoneCount_p,
        unsigned int * FillLevelDscrCount_p);


/*----------------------------------------------------------------------------
 * CDR Event Management API functions
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * EIP202_CDR_Status_Get
 *
 * This function retrieves the CDR status information. It can be called
 * periodically to monitor the CDR status and occurrence of fatal errors.
 *
 * In case of a fatal error the EIP202_CDR_Reset() function can be called to
 * recover the CDR and bring it to the sane and safe state.
 * The RDR SW Reset by means of the EIP202_RDR_Reset() function as well
 * as the Global SW Reset by means of the EIP202_Global_Reset() function
 * or the HW Reset must be performed too.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area
 *     for this CDR instance which contains the Device handle.
 *
 * Status_p (output)
 *     Pointer to the memory location where the CDR status information
 *     will be stored
 *
 * This function is re-entrant for any Device.
 *
 * Return value
 *     EIP202_RING_NO_ERROR : operation is completed
 *     EIP202_RING_ARGUMENT_ERROR : Passed wrong argument
 *     EIP202_RING_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP202_Ring_Error_t
EIP202_CDR_Status_Get(
        EIP202_Ring_IOArea_t * const IOArea_p,
        EIP202_CDR_Status_t * const Status_p);


/*----------------------------------------------------------------------------
 * EIP202_CDR_FillLevel_Low_INT_Enable
 *
 * This function enables the Command Descriptor Threshold and Timeout
 * interrupts. This function does not change the current API state. It must
 * be called every time after the CDR Threshold or Timeout interrupt occurs
 * in order to re-enable these one-shot interrupts.
 *
 * The CDR Manager interrupts are routed to the EIP-201 Advanced Interrupt
 * Controller (AIC) which in its turn can be connected to a System Interrupt
 * Controller (SIC).
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area
 *     for the CDR instance which contains the Device handle.
 *
 * ThresholdDscrCount (input)
 *     When the CDR descriptor fill level reaches the value below the number
 *     specified by this parameter the CDR Threshold interrupt (cdr_thresh_irq)
 *     will be generated.
 *
 * Timeout (input)
 *     When the CDR descriptor fill level is non-zero and not decremented
 *     for the amount of time specified by this parameter the CDR Timeout
 *     interrupt (cdr_timeout_irq) will be generated.
 *     The timeout value must be specified in 256 clock cycles
 *     of the EIP-202 HIA clock.
 *
 * This function is re-entrant for any Device.
 *
 * Return value
 *     EIP202_RING_NO_ERROR : operation is completed
 *     EIP202_RING_ARGUMENT_ERROR : Passed wrong argument
 *     EIP202_RING_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP202_Ring_Error_t
EIP202_CDR_FillLevel_Low_INT_Enable(
        EIP202_Ring_IOArea_t * const IOArea_p,
        const unsigned int ThresholdDscrCount,
        const unsigned int Timeout);


/*----------------------------------------------------------------------------
 * EIP202_CDR_FillLevel_Low_INT_ClearAndDisable
 *
 * This function clears and disables the Command Descriptor
 * Threshold (cdr_thresh_irq) and Timeout (cdr_timeout_irq) interrupts.
 * This function does not change the current API state.
 *
 * This function must be called as soon as the Command Descriptor Threshold or
 * Timeout interrupt occurs. The occurrence of these interrupts can be detected
 * by means of the EIP202_CDR_Status_Get() function, for example from an
 * Interrupt Service Routine.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area
 *     for the CDR instance which contains the Device handle.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * Return value
 *     EIP202_RING_NO_ERROR : operation is completed
 *     EIP202_RING_ARGUMENT_ERROR : Passed wrong argument
 *     EIP202_RING_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP202_Ring_Error_t
EIP202_CDR_FillLevel_Low_INT_ClearAndDisable(
        EIP202_Ring_IOArea_t * const IOArea_p);


/*----------------------------------------------------------------------------
 * EIP202_CDR_Dump
 *
 */
void
EIP202_CDR_Dump(
        EIP202_Ring_IOArea_t * const IOArea_p,
        EIP202_RingAdmin_t * const RingAdmin_p);


/* end of file eip202_cdr.h */


#endif /* EIP202_CDR_H_ */
