/* eip202_rdr.h
 *
 * EIP-202 Driver Library API:
 * Result Descriptor Ring (RDR)
 *
 * All the RDR API functions can be used concurrently with the CDR API functions
 * for any ring interface ID unless the API function description states
 * otherwise.
 *
 * Refer to the EIP-202 Driver Library User Guide for more information about
 * usage of this API
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

#ifndef EIP202_RDR_H_
#define EIP202_RDR_H_


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

// Processed result descriptor Control Data size (in 32-bit words)

#ifdef EIP202_64BIT_DEVICE
#define EIP202_RD_CTRL_DATA_MAX_WORD_COUNT  4
#else
#define EIP202_RD_CTRL_DATA_MAX_WORD_COUNT  2
#endif // !EIP202_64BIT_DEVICE

// RDR Status information
typedef struct
{
    // Fatal errors (interrupts), set to true when pending
    // RDR must be re-initialized in case of a fatal error
    bool        fDMAError;
    bool        fError;
    bool        fOUFlowError;

    // Result Descriptor Buffer Overflow Interrupt (not fatal),
    // set to true when pending
    bool        fRDBufOverflowInt;

    // Result Descriptor Overflow Interrupt (not fatal),
    // set to true when pending
    bool        fRDOverflowInt;

   /* The Buffer/Descriptor Overflow interrupts are not fatal errors and do
    * not require the reset. These events signal that packet or result
    * descriptor data  is lost. In this situation the packet processing result
    * must be discarded by the Host. These events when occur are also signaled
    * via the result descriptor bits.
    */

    // Threshold Interrupt (not fatal), set to true when pending
    bool        fTresholdInt;

    // Timeout Interrupt (not fatal), set to true when pending
    bool        fTimeoutInt;

    // Number of 32-bit words that are currently free
    // in the Result Descriptor FIFO
    uint16_t    RDFIFOWordCount;

    // Number of 32-bit words that are prepared in the RDR
    uint32_t    RDPrepWordCount;

    // Number of 32-bit words that are processed and stored in the RDR
    uint32_t    RDProcWordCount;

    // Number of full packets (i.e. the amount of
    // descriptors marked Last_Seg written) written to the RDR
    // (i.e. ‘processed’ / ‘updated’) and not yet acknowledged (processed)
    // by the host.
    uint8_t     RDProcPktWordCount;
} EIP202_RDR_Status_t;

// RDR settings
typedef struct
{
    // Other ARM settings
    EIP202_ARM_Ring_Settings_t   Params;

    // Make settings specific for continuous scatter mode.
    bool fContinuousScatter;
} EIP202_RDR_Settings_t;

// Control word parameters for the Logical Prepared Descriptor
typedef struct
{
    // Set to true for the first descriptor in the descriptor chain
    bool        fFirstSegment;

    // Set to true for the last descriptor in the descriptor chain
    bool        fLastSegment;

    // Prepared segment size in bytes
    uint32_t    PrepSegmentByteCount;

    // Expected result token data size in 32-bit words, optional
    uint32_t    ExpectedResultWordCount;

} EIP202_RDR_Prepared_Control_t;

// Logical Prepared Descriptor
typedef struct
{
    // Control word for the prepared descriptor
    // EIP202_RDR_Write_Prepared_ControlWord() helper function
    // can be used for obtaining this word
    uint32_t                PrepControlWord;

    // Destination packet data buffer, has to be provided by the caller:
    // Physical address that can be used by Device DMA
    EIP202_DeviceAddress_t   DstPacketAddr;

} EIP202_ARM_PreparedDescriptor_t;

// Control word parameters for the Logical Result Descriptor
typedef struct
{
    // Set to true for the first descriptor in the descriptor chain
    bool        fFirstSegment;

    // Set to true for the last descriptor in the descriptor chain
    bool        fLastSegment;

    // Set to true when the output data does not fit in the last output segment
    // assigned to this packet
    bool        fBufferOverflow;

    // Set to true when the result data does not fit the result descriptor
    bool        fDscrOverflow;

    // Actual processed segment size in bytes
    uint32_t    ProcSegmentByteCount;

    // Actual processed result token data size in 32-bit words,
    // for the last segment only
    uint32_t    ProcResultWordCount;

} EIP202_RDR_Result_Control_t;

// Result Token Data data structure embedded into processed result descriptors
typedef struct
{
    // PE specific error code
    uint32_t    ErrorCode;

    // Result packet size,
    // sum of all actual segment sizes that belong to this packet
    uint32_t    PacketByteCount;

    uint8_t     NextHeader;
    uint8_t     HashByteCount;
    uint8_t     BypassWordCount;  // in 32-bit words
    uint8_t     PadByteCount;
    bool        fE15;             // true when E15 occurred
    bool        fHash;            // true when hash "HashByteCount" bytes are
                                  // appended at the end of packet data
    uint8_t     BCNL;             // BCNL mask in bits 0 (B) - 3 (L)

    // This parameter is copied through from command to result descriptor
    // unless EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS configuration parameter
    // is defined in c_eip202_ring.h
    uint32_t    ApplicationId;

    // Bypass token words received from the PE
    // Optional, up to 3 32-bit words, actual length is in "BypassWordCount"
    uint32_t*   BypassData_p;

} EIP202_RDR_Result_Token_t;

// Bypass Data for successful packet processing
typedef struct
{
    // Type of Service / Traffic class
    uint8_t TOS_TC;

    // Don't fragment flag
    bool fDF;

    // Next Header field offset within IPv6 packet header header
    uint16_t NextHeaderOffset;

    // Application-specific reference to the Header Processing Context
    uint32_t HdrProcCtxRef;

} EIP202_RDR_BypassData_Pass_t;

// Bypass Data for failed packet processing
typedef struct
{
    // See EIP202_RDR_BYPASS_FLAG_*
    uint8_t ErrorFlags;

} EIP202_RDR_BypassData_Fail_t;

// Bypass data
typedef union
{
    // Use when BypassWordCount = 2 in Result Token
    EIP202_RDR_BypassData_Pass_t Pass;

    // Use when BypassWordCount = 1 in Result Token
    EIP202_RDR_BypassData_Fail_t Fail;

} EIP202_RDR_BypassData_t;

// Logical Result Descriptor
typedef struct
{
    // control fields for the command descriptor
    // EIP202_RDR_Read_Processed_ControlWord() helper function
    // can be used for obtaining this word
    uint32_t ProcControlWord;

    // Destination packet (segment) data, has to be provided by the caller:
    // Physical address that can be used by Device DMA
    EIP202_DeviceAddress_t DstPacketAddr;

    // control fields for the command descriptor
    // EIP202_RDR_Read_Processed_ControlWord() helper function
    // can be used for obtaining this word

    // Output Token buffer with fixed size
    uint32_t * Token_p;

} EIP202_ARM_ResultDescriptor_t;


/*----------------------------------------------------------------------------
 * RDR Initialization API functions
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * EIP202_RDR_Init
 *
 * This function performs the initialization of the EIP-202 RDR
 * interface and transits the API to the Initialized state.
 *
 * This function returns the EIP202_RING_UNSUPPORTED_FEATURE_ERROR error code
 * when it detects a mismatch in the Ring Control Driver Library configuration.
 *
 * Note: This function must be called either after the EIP-202 HW Reset or
 *       after the Ring SW Reset, see the EIP202_RDR_Reset() function.
 *       This function as well as optionally EIP202_RDR_Reset() function must be
 *       executed before any other of the EIP202_RDR_*() functions can be called.
 *
 * This function cannot be called concurrently with
 * the EIP202_RDR_Reset() function for the same Device.
 *
 * IOArea_p (output)
 *     Pointer to the place holder in memory for the IO Area
 *     for the RDR instance.
 *
 * Device (input)
 *     Handle for the Ring Control device instance returned by Device_Find()
 *     for this RDR instance.
 *
 * RingSettings_p (input)
 *     Pointer to the data structure that contains RDR configuration parameters
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
EIP202_RDR_Init(
        EIP202_Ring_IOArea_t * IOArea_p,
        const Device_Handle_t Device,
        const EIP202_RDR_Settings_t * const RingSettings_p);


/*----------------------------------------------------------------------------
 * EIP202_RDR_Reset
 *
 * This function performs the RDR SW Reset and transits the API
 * to the Uninitialized state. This function must be called before using
 * the other RDR API functions if the state of the ring is Unknown.
 *
 * This function can be used to recover the RDR from a fatal error.
 *
 * Note: This function must be called before calling the EIP202_RDR_Init()
 *       function only if the EIP-202 HW Reset was not done. Otherwise it still
 *       is can be called but it is not necessary.
 *
 * This function cannot be called concurrently with
 * the EIP202_RDR_Init() function for the same Device.
 *
 * IOArea_p (output)
 *     Pointer to the place holder in memory for the IO Area
 *     for the RDR instance identified by the Device parameter.
 *
 * Device (input)
 *     Handle for the Ring Control device instance returned by Device_Find
 *     for this RDR instance.
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
EIP202_RDR_Reset(
        EIP202_Ring_IOArea_t * const IOArea_p,
        const Device_Handle_t Device);


/*----------------------------------------------------------------------------
 * RDR Descriptor I/O API functions
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * EIP202_RDR_FillLevel_Get
 *
 * This function outputs the fill level of the RDR for all the descriptors
 * requested by means of the RingID parameter. The fill level is obtained as
 * a number of prepared  (via EIP202_RDR_Descriptor_Prepare) and processed result
 * descriptors that are NOT obtained (via EIP202_RDR_Descriptor_Get)
 * by the Host yet.
 *
 * The RDR states such as "Ring Full", "Ring Empty" and "Ring Free" are not
 * changed by this function, e.g. it does not perform any state transition.
 *
 * This function cannot be called concurrently with
 * the EIP202_RDR_Descriptor_Prepare() function for the same Device.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area
 *     which contains the Device handle for the RDR instance.
 *
 * FillLevelDscrCount_p (output)
 *     Pointer to the memory location where the number of prepared
 *     descriptors pending in the RDR will be stored
 *
 * This function is re-entrant for any Device.
 *
 * Return value
 *     EIP202_RING_NO_ERROR : operation is completed
 *     EIP202_RING_ARGUMENT_ERROR : Passed wrong argument
 *     EIP202_RING_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP202_Ring_Error_t
EIP202_RDR_FillLevel_Get(
        EIP202_Ring_IOArea_t * const IOArea_p,
        unsigned int * FillLevelDscrCount_p);


/*----------------------------------------------------------------------------
 * EIP202_RDR_Prepared_FillLevel_Get
 *
 * This function outputs the fill level of the RDR for the Prepared Descriptors
 * requested by means of the RingID parameter. The fill level is obtained as
 * a number of prepared descriptors that have been submitted to the RDR but
 * not processed by the device yet.
 *
 * The RDR states such as "Ring Full", "Ring Empty" and "Ring Free" are not
 * changed by this function, e.g. it does not perform any state transition.
 *
 * This function cannot be called concurrently with
 * the EIP202_RDR_Descriptor_Prepare() function for the same Device.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area
 *     which contains the Device handle for the RDR instance.
 *
 * FillLevelDscrCount_p (output)
 *     Pointer to the memory location where the number of prepared
 *     descriptors pending in the RDR will be stored
 *
 * This function is re-entrant for any Device.
 *
 * Return value
 *     EIP202_RING_NO_ERROR : operation is completed
 *     EIP202_RING_ARGUMENT_ERROR : Passed wrong argument
 *     EIP202_RING_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP202_Ring_Error_t
EIP202_RDR_Prepared_FillLevel_Get(
        EIP202_Ring_IOArea_t * const IOArea_p,
        unsigned int * FillLevelDscrCount_p);


/*----------------------------------------------------------------------------
 * EIP202_RDR_Write_Prepared_ControlWord
 *
 * This helper function returns the control word that can be written to
 * the logical prepared descriptor.
 *
 * This function is re-entrant.
 *
 */
uint32_t
EIP202_RDR_Write_Prepared_ControlWord(
        const EIP202_RDR_Prepared_Control_t * const  PreparedCtrl_p);


/*----------------------------------------------------------------------------
 * EIP202_RDR_Descriptor_Prepare
 *
 * This function prepares a requested number of descriptors to the RDR.
 * The function returns EIP202_RING_NO_ERROR and zero descriptors done count
 * (*DscrDoneCount_p set to 0) when no descriptors can be added to the RDR.
 * Until prepared descriptors are submitted to the RDR no result descriptors
 * can be retrieved from it. The prepared descriptors must refer to the Packet
 * Data DMA buffers that are large enough to receive the processed by the PE
 * data submitted via the CDR (see the EIP-202 Ring Control CDR API).
 *
 * The RDR states such as "Ring Full", "Ring Empty" and "Ring Free" are not
 * changed by this function, e.g. it does not perform any state transition.
 *
 * This function can be called after
 * the EIP202_RDR_FillLevel_Get() function when the latter checks
 * how many descriptors can be added to the RDR.
 *
 * The execution context calling this API must
 *
 * 1) Provide input data via the RDR using the EIP202_RDR_Descriptor_Prepare()
 * function before it or another context can obtain output data from the RDR.
 * This requirement is relevant for the look-aside and in-line use case only.
 *
 * 2) Ensure that Packet Data DMA buffers referred to by the prepared
 * descriptors are not re-used or freed by it or another context until
 * the processed result descriptor(s) referring to the packet associated with
 * these buffers is(are) fully processed by the Device. This is required not
 * only when in-place packet transform is done using the same Packet Data DMA
 * buffer as input and output buffer but also when different DMA buffers are
 * used for the packet processing input and output data.
 *
 * 3) Keep the packet descriptor chain state consistent. All descriptors that
 * belong to the same packet must be submitted atomically into the RDR without
 * being intermixed with descriptors that belong to other packets.
 *
 * 4) Submit the descriptors that belong to the same packet descriptor chain in
 * the right order, e.g. first descriptor for the first segment followed by
 * the middle descriptors followed by the last descriptor.
 *
 * 5) Ensure it does not call this function concurrently with another context
 * for the same RingID.
 *
 * 6) Ensure it does not call this function concurrently with another context
 * calling the EIP202_RDR_Prepared_FillLevel_Get() and
 * EIP202_RDR_FillLevel_Get() functions for the same Device.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area
 *     which contains the Device handle for the RDR instance.
 *
 * PreparedDscr_p (input)
 *     Pointer to 1st in the the array of prepared descriptors.
 *
 * DscrRequestedCount (input)
 *     Number of prepared descriptors stored back-to-back in the array
 *     pointed to by PreparedDscr_p.
 *
 * DscrPreparedCount_p (output)
 *     Pointer to the memory location where the number of prepared descriptors
 *     actually added to the RDR will be stored.
 *
 * FillLevelDscrCount_p (output)
 *     Pointer to the memory location where the number of prepared
 *     descriptors pending in the RDR will be stored
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
EIP202_RDR_Descriptor_Prepare(
        EIP202_Ring_IOArea_t * const IOArea_p,
        const EIP202_ARM_PreparedDescriptor_t * PreparedDscr_p,
        const unsigned int DscrRequestedCount,
        unsigned int * const DscrPreparedCount_p,
        unsigned int * FillLevelDscrCount_p);


/*----------------------------------------------------------------------------
 * EIP202_RDR_Processed_FillLevel_Get
 *
 * This function outputs the fill level of the ring requested by means of
 * the RingID parameter. The fill level is obtained as a number of result
 * descriptors that have been processed by the Device but not processed
 * by the Host yet.
 *
 * When the RDR is in the "Ring Full" state this function outputs the fill
 * level equal to the RDR size. When the RDR is in the "Ring Empty" state this
 * functions outputs the zero fill level. the RDR is in the "Ring Free" state
 * when this function returns the fill level that is greater than zero and less
 * than the RDR size (in result descriptors).
 *
 * This function cannot be called concurrently with
 * the EIP202_RDR_Descriptor_Get() function for the same Device.
 *
 * Note: This function returns EIP202_RING_UNSUPPORTED_FEATURE_ERROR
 *       when the EIP202_RING_ANTI_DMA_RACE_CONDITION_CDS parameter is
 *       set.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area
 *     which contains the Device handle for the RDR instance.
 *
 * FillLevelDscrCount_p (output)
 *     Pointer to the memory location where the number of processed result
 *     descriptors pending in the RDR will be stored
 *     RDR Empty: FillLevel = 0
 *     RDR Free:  0 < FillLevel < RingSize (in descriptors)
 *     RDR Full:  FillLevel = RingSize (in descriptors)
 *
 * FillLevelPktCount_p (output)
 *     Pointer to the memory location where the number of processed result
 *     packets (whose descriptors are pending in the RDR) will be stored.
 *     In case this parameter outputs 127 then there can be more packets
 *     available.
 *
 * This function is re-entrant for any Device.
 *
 * Return value
 *     EIP202_RING_NO_ERROR : operation is completed
 *     EIP202_RING_UNSUPPORTED_FEATURE_ERROR : feature is not supported
 *     EIP202_RING_ARGUMENT_ERROR : Passed wrong argument
 *     EIP202_RING_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP202_Ring_Error_t
EIP202_RDR_Processed_FillLevel_Get(
        EIP202_Ring_IOArea_t * const IOArea_p,
        unsigned int * FillLevelDscrCount_p,
        unsigned int * FillLevelPktCount_p);


/*----------------------------------------------------------------------------
 * EIP202_RDR_Read_Processed_ControlWord
 *
 * This helper function outputs the control word that can be read from
 * the logical processed result descriptor.
 *
 * ResDscr_p (input)
 *     Processed result descriptor with the control information and
 *     token result data must be read from.
 *
 * RDControl_p (output)
 *     Pointer to the data structure where the result descriptor control
 *     information will be written.
 *
 * ResToken_p (output)
 *     Pointer to the data structure where the result token
 *     information will be written.
 *
 * This function is re-entrant.
 *
 */
void
EIP202_RDR_Read_Processed_ControlWord(
        EIP202_ARM_ResultDescriptor_t * const  ResDscr_p,
        EIP202_RDR_Result_Control_t * const RDControl_p,
        EIP202_RDR_Result_Token_t * const ResToken_p);


/*----------------------------------------------------------------------------
 * EIP202_RDR_Read_Processed_BypassData
 *
 * This helper function outputs the bypass data that can be read from
 * the result token data of the logical processed result descriptor.
 *
 * ResToken_p (input)
 *     Pointer to the data structure where the result token
 *     information will be written.
 *
 * BD_p (output)
 *     Pointer to the data structure where the bypass data
 *     information will be written.
 *
 * This function is re-entrant.
 */
void
EIP202_RDR_Read_Processed_BypassData(
        const EIP202_RDR_Result_Token_t * const  ResToken_p,
        EIP202_RDR_BypassData_t * const BD_p);


/*----------------------------------------------------------------------------
 * EIP202_RDR_Descriptor_Get
 *
 * This function gets a requested number of result descriptors from the RDR.
 * The function returns EIP202_RING_NO_ERROR and zero descriptors done count
 * (*DscrDoneCount_p set to 0) when no descriptors can be retrieved from
 * the RDR, e.g. "Ring Empty" state.
 *
 * This function can be used together with
 * the EIP202_RDR_Processed_FillLevel_Get() function when the latter checks
 * how many command descriptors can be retrieved from the RDR.
 *
 * The execution context calling this API function must
 *
 * 1) Ensure it does not call this function concurrently with another context
 * for the same Device;
 *
 * 2) Ensure it does not call this function concurrently with another context
 * calling the EIP202_RDR_Processed_FillLevel_Get() function for the same Device.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area
 *     which contains the Device handle for the RDR instance.
 *
 * ResultDscr_p (input)
 *     Pointer to 1st in the the array of command descriptors.
 *
 * PktRequestedCount (input)
 *     Maximum number of packet descriptor chains which result descriptors
 *     should be obtained by this function. This function will obtain
 *     the result descriptors for the fully processed packet chains only.
 *     If set to 0 then this function will try to obtain no more result
 *     descriptors than specified by the DscrRequestedCount parameter.
 *
 * DscrRequestedCount (input)
 *     Number of descriptors stored back-to-back in the array
 *     pointed to by ResultDscr_p. Cannot be zero.
 *
 * DscrDoneCount_p (output)
 *     Pointer to the memory location where the number of descriptors
 *     actually added to the RDR will be stored.
 *
 * FillLevelDscrCount_p (output)
 *     Pointer to the memory location where the number of processed result
 *     descriptors pending in the RDR will be stored
 *     RDR Empty: FillLevel = 0
 *     RDR Free:  0 < FillLevel < RingSize (in descriptors)
 *     RDR Full:  FillLevel = RingSize (in descriptors)
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
EIP202_RDR_Descriptor_Get(
        EIP202_Ring_IOArea_t * const IOArea_p,
        EIP202_ARM_ResultDescriptor_t * ResultDscr_p,
        const unsigned int PktRequestedCount,
        const unsigned int DscrRequestedCount,
        unsigned int * const DscrDoneCount_p,
        unsigned int * FillLevelDscrCount_p);


/*----------------------------------------------------------------------------
 * RDR Event Management API functions
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * EIP202_RDR_Status_Get
 *
 * This function retrieves the RDR status information. It can be called
 * periodically to monitor the RDR status and occurrence of fatal errors.
 *
 * In case of a fatal error the EIP202_RDR_Reset() function can be called to
 * recover the RDR and bring it to the sane and safe state.
 * The RDR SW Reset by means of the EIP202_RDR_Reset() function as well
 * as the Global SW Reset by means of the EIP202_Global_Reset() function
 * or the HW Reset must be performed too.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area
 *     which contains the Device handle for this RDR instance.
 *
 * Status_p (output)
 *     Pointer to the memory location where the RDR status information
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
EIP202_RDR_Status_Get(
        EIP202_Ring_IOArea_t * const IOArea_p,
        EIP202_RDR_Status_t * const Status_p);


/*----------------------------------------------------------------------------
 * EIP202_RDR_Processed_FillLevel_High_INT_Enable
 *
 * This function enables the Command Descriptor Threshold and Timeout
 * interrupts. This function does not change the current API state. It must
 * be called every time after the RDR Threshold or Timeout interrupt occurs
 * in order to re-enable these one-shot interrupts.
 *
 * The RDR Manager interrupts are routed to the EIP-201 Advanced Interrupt
 * Controller (AIC) which in its turn can be connected to a System Interrupt
 * Controller (SIC).
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area
 *     which contains the Device handle for the RDR instance.
 *
 * ThresholdDscrCount (input)
 *     When the RDR descriptor fill level reaches the value below the number
 *     specified by this parameter the RDR Threshold interrupt (cdr_thresh_irq)
 *     will be generated.
 *
 * Timeout (input)
 *     When the RDR descriptor fill level is non-zero and not decremented
 *     for the amount of time specified by this parameter the RDR Timeout
 *     interrupt (cdr_timeout_irq) will be generated.
 *     The timeout value must be specified in 256 clock cycles
 *     of the EIP-202 HIA clock.
 *
 * fIntPerPacket (input)
 *     Descriptor-oriented or Packet-oriented Interrupts
 *     (rd_proc_thresh_irq and rd_proc_timeout_irq)
 *     When set to true the interrupts will be generated per packet,
 *     otherwise interrupts are generated per descriptor
 *
 * This function is re-entrant for any Device.
 *
 * Return value
 *     EIP202_RING_NO_ERROR : operation is completed
 *     EIP202_RING_ARGUMENT_ERROR : Passed wrong argument
 *     EIP202_RING_ILLEGAL_IN_STATE : invalid API state transition
 */
EIP202_Ring_Error_t
EIP202_RDR_Processed_FillLevel_High_INT_Enable(
        EIP202_Ring_IOArea_t * const IOArea_p,
        const unsigned int ThresholdDscrCount,
        const unsigned int Timeout,
        const bool fIntPerPacket);


/*----------------------------------------------------------------------------
 * EIP202_RDR_Processed_FillLevel_High_INT_ClearAndDisable
 *
 * This function clears and disables the Result Descriptor
 * Threshold (cdr_thresh_irq) and Timeout (cdr_timeout_irq) interrupts as well
 * as the Descriptor (rd_proc_oflo_irq) and Buffer (rd_buf_oflo_irq) Overflow
 * interrupts. This function does not change the current API state.
 *
 * This function must be called as soon as these interrupts occur.
 * The occurrence of these interrupts can be detected
 * by means of the EIP202_RDR_Status_Get() function, for example from an
 * Interrupt Service Routine.
 *
 * IOArea_p (input)
 *     Pointer to the place holder in memory for the IO Area
 *     which contains the Device handle for the RDR instance.
 *
 * fOvflIntOnly (input)
 *     When true this function will clear the buffer and descriptor overflow
 *     interrupts only without clearing and disabling the already enabled RDR
 *     threshold and timeout interrupts at the same time.
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
EIP202_RDR_Processed_FillLevel_High_INT_ClearAndDisable(
        EIP202_Ring_IOArea_t * const IOArea_p,
        const bool fOvflIntOnly);


/*----------------------------------------------------------------------------
 * EIP202_RDR_Dump
 *
 */
void
EIP202_RDR_Dump(
        EIP202_Ring_IOArea_t * const IOArea_p,
        EIP202_RingAdmin_t * const RingAdmin_p);


/* end of file eip202_rdr.h */


#endif /* EIP202_RDR_H_ */
