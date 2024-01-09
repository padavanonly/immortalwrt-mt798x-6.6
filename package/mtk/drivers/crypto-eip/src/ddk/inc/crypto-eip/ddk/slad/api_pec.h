/* api_pec.h
 *
 * Packet Engine Control API (PEC)
 *
 * This API can be used to perform transforms on security protocol packets
 * for a set of security network protocols like IPSec, MACSec, sRTP, SSL,
 * DTLS, etc.
 *
 * This API can supports both Look-Aside (LA) and Hybrid (HB) use cases.
 *
 * Please note that this is a generic API that can be used for many transform
 * engines. A separate document will detail the exact fields and parameters
 * required for the SA and Descriptors.
 */

/*****************************************************************************
* Copyright (c) 2007-2022 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_API_PEC_H
#define INCLUDE_GUARD_API_PEC_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"

// This API requires the use of the DMA Buffer Allocation API. All buffers
// are passed through this API as handles.
#include "api_dmabuf.h"         // DMABuf_Handle_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Error/status codes
typedef enum
{
    PEC_STATUS_OK = 0,
    PEC_STATUS_BUSY,            // Device busy, try again later
    PEC_ERROR_BAD_PARAMETER,
    PEC_ERROR_BAD_HANDLE,
    PEC_ERROR_BAD_USE_ORDER,
    PEC_ERROR_INTERNAL,
    PEC_ERROR_NOT_IMPLEMENTED

} PEC_Status_t;


/*----------------------------------------------------------------------------
 * PEC_InitBlock_t
 *
 * This structure contains service initialization parameters that are passed
 * to the PEC_Init call.
 *
 * For forward-compatibility with future extensions, please zero-init the
 * data structure before setting the fields and providing it to PEC_Init.
 * Example: PEC_InitBlock_t InitBlock = {0};
 */
typedef struct
{
    bool fUseDynamicSA;             // true = use variable-size SA format

    // the following fields are related to Scatter/Gather
    // not all engines support this
    unsigned int FixedScatterFragSizeInBytes;

    bool fContinuousScatter; // Enable Continous Scatter Mode.
} PEC_InitBlock_t;


/*----------------------------------------------------------------------------
 * PEC_CommandDescriptor_t
 *
 * This data structure describes a transform request that will be queued up
 * for processing by the transform engine. It refers to the input and output
 * data buffers, the SA buffer(s) and contains parameters that describe the
 * transform, like the length of the data.
 *
 * For operations that do not require output buffers such as hash operations
 * the SrcPkt_Handle and DstPkt_Handle parameters in the
 * PEC_CommandDescriptor_t descriptor must be set equal by applications.
 *
 */
typedef struct
{
    // the pointer that will be returned in the related result descriptor
    void * User_p;

    // Optional EIP-96 Instruction Token buffer,
    // some packet engines do not use tokens,
    // DMABuf_NULLHandle can be assigned to this variable when not used
    DMABuf_Handle_t Token_Handle;

    // EIP-96 Instruction Token size (in 32-bit words)
    unsigned int Token_WordCount;

    // data buffers
    DMABuf_Handle_t SrcPkt_Handle;
    DMABuf_Handle_t DstPkt_Handle;
    unsigned int SrcPkt_ByteCount;
    unsigned int Bypass_WordCount;

    // SA reference
    uint32_t SA_WordCount;
    DMABuf_Handle_t SA_Handle1;
    DMABuf_Handle_t SA_Handle2;

    // Engine specific control fields
    // with Transform specific values
    uint32_t Control1;
    uint32_t Control2;

    // Input Token buffer with fixed size (determined by use case)
    // This buffer must be allocated before the descriptor can be passed
    // to PEC_Packet_Put()
    uint32_t * InputToken_p;

} PEC_CommandDescriptor_t;

/*----------------------------------------------------------------------------
 * PEC_ResultDescriptor_t
 *
 * This data structure contains the result of the transform request. The user
 * can match the result to a command using the User_p field.
 *
 * A number of parameters from the command descriptor are also returned, as a
 * courtesy service.
 */
typedef struct
{
    // the pointer that from the related command descriptor
    void * User_p;

    // data buffers
    DMABuf_Handle_t SrcPkt_Handle;
    DMABuf_Handle_t DstPkt_Handle;
    uint32_t DstPkt_ByteCount;
    void * DstPkt_p;
    // Number of scatter particles used (continous scatter).
    uint32_t NumParticles;

    unsigned int Bypass_WordCount;

    // Engine specific status fields
    // with Transform specific values
    uint32_t Status1;
    uint32_t Status2;

    // Output Token buffer with fixed size (determined by use case)
    // This buffer must be allocated before the descriptor can be passed
    // to PEC_Packet_Get()
    uint32_t * OutputToken_p;

} PEC_ResultDescriptor_t;


// Per-packet flags
#define PEC_PKT_FLAG_INIT_ARC4     BIT_0
#define PEC_PKT_FLAG_HASH_FINAL    BIT_1


/*----------------------------------------------------------------------------
 * PEC_PacketParams_t
 *
 * This data structure contains the per-packet parameters in a
 * device-independent way.
 */
typedef struct
{
    // Bitwise or of zero or more PEC_PKT_FLAG_* values, 0 if none apply.
    uint8_t flags;

    // Next header byte (IPsec padding) or pad byte value.
    uint8_t PadByte;

    // Blocksize to which the packet must be padded (must be power of 2).
    uint16_t PadBoundary;

    // Offset of ESP header
    uint8_t Offset;

    // Token Header Word.
    uint32_t TokenHeaderWord;

    // Hardware operation.
    uint8_t HW_Services;

} PEC_PacketParams_t;

// Error status bits.
#define PEC_PKT_ERROR_AUTH         BIT_0 // Authentication failure
#define PEC_PKT_ERROR_PAD          BIT_1 // Pad verify fail
#define PEC_PKT_ERROR_SEQNUM       BIT_2 // Sequence number check failure
#define PEC_PKT_ERROR_BADCMD       BIT_3 // Invalid command.
#define PEC_PKT_ERROR_BADALGO      BIT_4 // Invalid algorithm
#define PEC_PKT_ERROR_PROHIBITED   BIT_5 // Prohibited algorithm
#define PEC_PKT_ERROR_ZEROLENGTH   BIT_6 // Zero packet length
#define PEC_PKT_ERROR_BADIP        BIT_7 // Invalid IP header
#define PEC_PKT_ERROR_SPI          BIT_8 // SPI mismatch.
#define PEC_PKT_ERROR_CRYPTBLKSIZE BIT_9 // Block size error
#define PEC_PKT_ERROR_BADCOMBO     BIT_10 // Invalid combination of algorithms
#define PEC_PKT_ERROR_LENGTH       BIT_11 // Length error
#define PEC_PKT_ERROR_PROC         BIT_12 // Processing error
#define PEC_PKT_ERROR_INBOUNDLEN   BIT_13 // Bad Inbound PE length
#define PEC_PKT_ERROR_SYSBUS       BIT_14 // System bus error
#define PEC_PKT_ERROR_DESCRIPTOR   BIT_15 // Command descriptor error
#define PEC_PKT_ERROR_HASHBLKSIZE  BIT_16 // Hash block size error
#define PEC_PKT_ERROR_TOKEN        BIT_17 // Error in token
#define PEC_PKT_ERROR_BYPASS       BIT_18 // Too much bypass data
#define PEC_PKT_ERROR_HASHOVF      BIT_19 // Hash input overflow
#define PEC_PKT_ERROR_TTLHOP       BIT_20 // TTL/Hop limit underflow
#define PEC_PKT_ERROR_CHKSUM       BIT_21 // Checksum error
#define PEC_PKT_ERROR_TIMEOUT      BIT_22 // Packet engine timeout
#define PEC_PKT_ERROR_DSCR_OVF     BIT_25 // Result descriptor overflow
#define PEC_PKT_ERROR_BUF_OVF      BIT_26 // Result buffer overflow


/*----------------------------------------------------------------------------
 * PEC_ResultStatus_t
 *
 * This data structure contains the per-packet status/result in a
 * device-independent way.
 */
typedef struct
{
    // Bitwise or of PEC_PKT_ERROR_* values. Zero for successful operation.
    uint32_t errors;

    // Number of pad bytes detected by packet operation.
    uint8_t PadByteCount;

    // Next header byte detected by inbound IPsec operation.
    uint8_t NextHeader;

    // Hybrid use case only: Type of Service (IPv4) / Traffic class (IPv6)
    uint8_t TOS_TC;

    // IPv4 Hybrid use case only:
    // True if fragmentation is prohibited for the packet
    bool fDF;

    // IPv6 Hybrid use case only: Next Header field offset within packet header
    uint16_t NextHeaderOffset;

    // Hybrid use case only: Header Processing Context reference
    uint32_t HdrProcCtxRef;

} PEC_ResultStatus_t;

/*----------------------------------------------------------------------------
 * PEC_NotifyFunction_t
 *
 * This type specifies the callback function prototype for the function
 * PEC_CommandNotify_Request and PEC_ResultNotify_Request.
 * The notification will occur only once.
 *
 * NOTE: The exact context in which the callback function is invoked and the
 *       allowed actions in that callback are implementation specific. The
 *       intention is that all API functions can be used, except PEC_UnInit.
 */
typedef void (* PEC_NotifyFunction_t)(void);

// Supported cryptographic algorthms and functions.
#define PEC_SUPPORTED_OCE      BIT_1  // Output classification engine.
#define PEC_SUPPORTED_ICE      BIT_2  // Input classification engine.
#define PEC_SUPPORTED_FRC      BIT_3  // Flow record cache.
#define PEC_SUPPORTED_MASK1024 BIT_4  // Anti-replay mask size 1024
#define PEC_SUPPORTED_BC0      BIT_5  // External block cipher
#define PEC_SUPPORTED_SM4      BIT_6
#define PEC_SUPPORTED_SM3      BIT_7
#define PEC_SUPPORTED_CHACHA20 BIT_8
#define PEC_SUPPORTED_POLY1305 BIT_9
#define PEC_SUPPORTED_MASK256  BIT_10  // Anti-replay mask size 256
#define PEC_SUPPORTED_MASK384  BIT_11  // Anti-replay mask size 384
#define PEC_SUPPORTED_AES      BIT_12  // AES modes ECB, CBC, CTR
#define PEC_SUPPORTED_AES_FB   BIT_13  // CFB and OFB modes for AES
#define PEC_SUPPORTED_DES      BIT_15  // DES/3DES modes ECB, CBC
#define PEC_SUPPORTED_DES_FB   BIT_16  // CFB and OFB modes for DES/3DES
#define PEC_SUPPORTED_ARC4     (BIT_19|BIT_18)
#define PEC_SUPPORTED_AES_XTS  BIT_20
#define PEC_SUPPORTED_WIRELESS BIT_21  // Kasumi, SNOW and ZUC algorithms
#define PEC_SUPPORTED_MD5      BIT_22
#define PEC_SUPPORTED_SHA1     BIT_23
#define PEC_SUPPORTED_SHA2_256 BIT_25  // SHA2-224 and SHA2-256
#define PEC_SUPPORTED_SHA2_512 BIT_26  // SHA2-384 and SHA2-512
#define PEC_SUPPORTED_CBCMAC   BIT_27  // (X)CBC-MAC, 128-bit keys
#define PEC_SUPPORTED_CBCMAC256 BIT_29 // CBC-MAC, 192 and 256 bit keys
#define PEC_SUPPORTED_GHASH    BIT_30  // AES-GCM/GMAC
#define PEC_SUPPORTED_SHA3     BIT_31

/*----------------------------------------------------------------------------
 * PEC_Capabilities_t
 *
 * szTextDescription[]
 *     Zero-terminated descriptive text of the available services.
 */
#define PEC_MAXLEN_TEXT  128

typedef struct
{
    char szTextDescription[PEC_MAXLEN_TEXT];
    unsigned int SupportedFuncs;
} PEC_Capabilities_t;


/*----------------------------------------------------------------------------
 * PEC_Capabilities_Get
 *
 * This routine returns a structure that describes the capabilities of the
 * implementation. See description of PEC_Capabilities_t for details.
 *
 * Capabilities_p
 *     Pointer to the capabilities structure to fill in.
 *
 * This function is re-entrant.
 */
PEC_Status_t
PEC_Capabilities_Get(
        PEC_Capabilities_t * const Capabilities_p);


/*----------------------------------------------------------------------------
 * PEC_Init
 *
 * This function must be used to initialize the service. No API function may
 * be used before this function has returned.
 *
 * InterfaceId (input)
 *     Packet I/O interface (such as ring, for example) to be initialized.
 *
 * InitBlock_p (input)
 *     Pointer to the initialization block data
 *
 */
PEC_Status_t
PEC_Init(
        const unsigned int InterfaceId,
        const PEC_InitBlock_t * const InitBlock_p);


/*----------------------------------------------------------------------------
 * PEC_UnInit
 *
 * This call un-initializes the service. Use only when there are no pending
 * transforms. The caller must make sure that no API function is used while or
 * after this function executes.
 *
 * InterfaceId (input)
 *     Packet I/O interface (such as ring, for example) to be un-initialized.
 *
 */
PEC_Status_t
PEC_UnInit(
        const unsigned int InterfaceId);


/*----------------------------------------------------------------------------
 * PEC_SA_Register
 *
 * This function must be used to register an SA so it can be used for
 * transforms. The caller is responsible for filling in the SA fields
 * according to the specification of the engine, with the exception of any
 * fields that are designated to hold addresses of the other SA memory blocks.
 * The buffers are considered arrays of 32bit words in host-native byte order.
 *
 * When this call returns it is no longer allowed to access these SA memory
 * blocks directly.
 *
 * InterfaceId (input)
 *     Not used, reserved.
 *
 * SA_Handle1 (input)
 *     Handle for the main SA memory block. This memory block contains all the
 *     static material and is typically read completely for every transform
 *     but only selective parts are written back after the transform.
 *
 * SA_Handle2 (input)
 * SA_Handle3 (input)
 *     Handles for the optional second and third memory blocks. These are
 *     typically used to remember state information and are therefore
 *     required only for certain types of SA's. These blocks are typically
 *     read and written for every transform. Putting them in a separate memory
 *     blocks allows these to be put in more high performance memory.
 *     Provide zero for unused handles.
 *
 * The exact number of handles required depends on the transform engine and on
 * the SA parameters and is specified in a separate document. The exact size
 * of each memory block is known to the service through the handle.
 */
PEC_Status_t
PEC_SA_Register(
        const unsigned int InterfaceId,
        DMABuf_Handle_t SA_Handle1,
        DMABuf_Handle_t SA_Handle2,
        DMABuf_Handle_t SA_Handle3);


/*----------------------------------------------------------------------------
 * PEC_SA_UnRegister
 *
 * This function must be used to remove an SA from the system, which means it
 * can no longer be used to perform transforms with the engine. When this
 * function returns the caller is allowed to access the SA memory blocks again.
 *
 * InterfaceId (input)
 *     Not used, reserved.
 *
 * SA_Handle1 (input)
 * SA_Handle2 (input)
 * SA_Handle3 (input)
 *     The same handles as we provided in the PEC_SA_Register call.
 *     Provide zero for unused handles.
 */
PEC_Status_t
PEC_SA_UnRegister(
        const unsigned int InterfaceId,
        DMABuf_Handle_t SA_Handle1,
        DMABuf_Handle_t SA_Handle2,
        DMABuf_Handle_t SA_Handle3);


/*----------------------------------------------------------------------------
 * PEC_Packet_Put
 *
 * This function must be put transform requests into a queue. It is possible
 * to queue up just one request, or an array of requests (these do not have to
 * be related). In case the queue is full, none or only a few of the requests
 * might be queued up.
 *
 * InterfaceId (input)
 *     Packet I/O interface (such as ring, for example) where the packet(s)
 *     must be submitted to. See the SLAD PEC API Implementation Notes
 *     for the details on how this parameter is used.
 *
 * Commands_p (input)
 *     Pointer to one (or an array of command descriptors, each describe one
 *     transform request.
 *
 * CommandsCount (input)
 *     The number of command descriptors pointed to by Commands_p.
 *
 * PutCount_p (output)
 *     This parameter is used to return the actual number of descriptors that
 *     was queued up for processing (0..CommandsCount).
 */
PEC_Status_t
PEC_Packet_Put(
        const unsigned int InterfaceId,
        const PEC_CommandDescriptor_t * Commands_p,
        const unsigned int CommandsCount,
        unsigned int * const PutCount_p);


/*----------------------------------------------------------------------------
 * PEC_Packet_Get
 *
 * This function must be used to retrieve the results for the requested
 * transform operations. Every request generates one result, whether it is
 * successful or failed. The caller is able to retrieve one result or an
 * array of results.
 *
 * InterfaceId (input)
 *     Packet I/O interface (such as ring, for example) where the packet(s)
 *     must be retrieved from. See the SLAD PEC API Implementation Notes
 *     for the details on how this parameter is used.
 *
 * Results_p (input)
 *     Pointer to the result descriptor, or array of result descriptors, that
 *     will be populated by the service based on completed transform requests.
 *
 * ResultsLimit (input)
 *     The number of result descriptors available from Results_p and onwards.
 *
 * GetCount_p (output)
 *     The actual number of result descriptors that were populated is returned
 *     in this parameter.
 */
PEC_Status_t
PEC_Packet_Get(
        const unsigned int InterfaceId,
        PEC_ResultDescriptor_t * Results_p,
        const unsigned int ResultsLimit,
        unsigned int * const GetCount_p);


/*----------------------------------------------------------------------------
 * PEC_CommandNotify_Request
 *
 * This routine can be used to request a one-time notification of space
 * available for new commands. It is typically used after PEC_PacketPut
 * returned fewer packets added that requested (could be zero) and the caller
 * does not want to poll for new results.
 *
 * Once the requested number of spaces are available, the implementation will
 * invoke the callback one time to notify the user of the available results.
 * The notification is then immediately disabled.
 *
 * InterfaceId (input)
 *     Packet I/O interface (such as ring, for example) for which notification
 *     must be generated.
 *
 * CBFunc_p
 *     Address of the callback function.
 *
 * CommandsCount
 *     The number of commands that must be possible to add with
 *     PEC_PacketPut before the notification function will be invoked.
 */
PEC_Status_t
PEC_CommandNotify_Request(
        const unsigned int InterfaceId,
        PEC_NotifyFunction_t CBFunc_p,
        const unsigned int CommandsCount);


/*----------------------------------------------------------------------------
 * PEC_ResultNotify_Request
 *
 * This routine can be used to request a one-time notification of available
 * results. It is typically used after PEC_PacketGet returned zero results
 * and the caller does not want to poll for new results.
 *
 * Once the requested transforms have completed, the implementation will
 * invoke the callback one time to notify this fact.
 * The notification is then immediately disabled. It is possible that the
 * notification callback is invoked when fewer than the expected number of
 * transforms have completed (or even zero). In this case the application
 * must invoke PEC_ResultNotify_Request again to be notified of the completion
 * of the remaining expected transforms.
 *
 * Once the notification callback is invoked, the application must use
 * PEC_Packet_Get to read all available results and then call
 * PEC_ResultNotifyRequest again, else the notification
 * callback may not be called again.
 *
 * It is permissible (and even recommended) that the callback function
 * calls PEC_Packet_Get to retrieve all available results and then
 * PEC_ResultNotifyRequest to schedule the next invocation of the same
 * callback. In this case the main application must call
 * PEC_ResultNotify_Request exactly once after PEC_Init.
 *
 *
 * InterfaceId (input)
 *     Packet I/O interface (such as ring, for example) for which notification
 *     must be generated.
 *
 * CBFunc_p
 *     Address of the callback function.
 *
 * ResultsCount
 *     The requested number of results that should be available the next
 *     time PEC_Packet_Get is called after the callback function has occured.
 *     This parameter should have a minimum value of 1 and its maximum value is
 *     implementation dependend.
 */
PEC_Status_t
PEC_ResultNotify_Request(
        const unsigned int InterfaceId,
        PEC_NotifyFunction_t CBFunc_p,
        const unsigned int ResultsCount);


/*----------------------------------------------------------------------------
 * PEC_CD_Control_Write
 *
 * Write the Control1 and Control2 engine-specific fields in the
 * Command Descriptor The other fields (such as SrcPkt_ByteCount and
 * Bypass_WordCount must have been filled in already.
 *
 * Command_p (input, output)
 *     Command descriptor whose Control1 and Control2 fields must be filled in.
 *
 * PacketParams_p (input)
 *     Per-packet parameters.
 *
 * This function is not implemented for all engine types.
 */
PEC_Status_t
PEC_CD_Control_Write(
        PEC_CommandDescriptor_t * Command_p,
        const PEC_PacketParams_t * const PacketParams_p);


/*----------------------------------------------------------------------------
 * PEC_RD_Status_Read
 *
 * Read the engine-specific Status1 and Status2 fields from a Result Descriptor
 * and convert them to an engine-independent format.
 *
 * Result_p (input)
 *     Result descriptor.
 *
 * ResultStatus_p (output)
 *     Engine-independent status information.
 *
 * Not all error conditions can occur on all engine types.
 */
PEC_Status_t
PEC_RD_Status_Read(
        const PEC_ResultDescriptor_t * const Result_p,
        PEC_ResultStatus_t * const ResultStatus_p);


/*----------------------------------------------------------------------------
 * PEC_Scatter_Preload
 *
 * This function must be used to pre-load the service with scatter buffers
 * that will be consumed by transforms configured to use scatter buffers
 * instead of a provided destination buffer(s). This is required for certain
 * engines only - check the driver documentation.
 *
 * InterfaceId (input)
 *    Packet I/O interface (such as ring, for example) for which scatter buffers
 *    must be pre-loaded.
 *
 * Handles_p (input)
 *     Pointer to an array of handles, each handle representing one scatter
 *     buffer.
 *
 * HandlesCount (input)
 *     The number of handles in the array pointed to by Handles_p.
 *
 * AcceptedCount_p (output)
 *     Pointer to the output parameter that reports how many of the handles,
 *     from the start of the array, could accepted by the service due to
 *     storage capacity limit. Value will be between 0 and HandlesCount.
 */
PEC_Status_t
PEC_Scatter_Preload(
        const unsigned int InterfaceId,
        DMABuf_Handle_t * Handles_p,
        const unsigned int HandlesCount,
        unsigned int * const AcceptedCount_p);


/*----------------------------------------------------------------------------
 * PEC_Scatter_PreloadNotify_Request
 *
 * This routine can be used to request a one-time notification of space
 * available in the scatter preload list. This is typically used when
 * PEC_Scatter_Preload does not accept further buffers and the caller
 * does not want to keep making those calls.
 *
 * Once the requested number of scatter buffers have been consumed, the
 * implementation will invoke the callback one time to notify this fact.
 * The notification is then immediately disabled.
 *
 * InterfaceId (input)
 *     Packet I/O interface (such as ring, for example) for which notification
 *     must be generated.
 *
 * CBFunc_p
 *     Address of the callback function.
 *
 * ConsumedCount
 *     The number of scatter buffers consumed before invoking the callback.
 */
PEC_Status_t
PEC_Scatter_PreloadNotify_Request(
        const unsigned int InterfaceId,
        PEC_NotifyFunction_t CBFunc_p,
        const unsigned int ConsumedCount);


/*----------------------------------------------------------------------------
 * PEC_Put_Dump
 *
 * Dump packet put operation administration and cache data.
 *
 * InterfaceId (input)
 *     Packet I/O interface (such as ring, for example) for which notification
 *     must be generated.
 *
 * FirstSlotId (input)
 *     First slot in the input ring buffer to start dumping.
 *
 * LastSlotId (input)
 *     Last slot in the input ring buffer to stop dumping.
 *
 * fDumpPutAdmin (input)
 *     true if the input ring administration data must be dumped.
 *
 * fDumpPutCache (input)
 *     true if the input ring cache data must be dumped.
 *
 * Return value
 *     None
 */
void
PEC_Put_Dump(
        const unsigned int InterfaceId,
        const unsigned int FirstSlotId,
        const unsigned int LastSlotId,
        const bool fDumpPutAdmin,
        const bool fDumpPutCache);


/*----------------------------------------------------------------------------
 * PEC_Get_Dump
 *
 * Dump packet get operation administration and cache data.
 *
 * InterfaceId (input)
 *     Packet I/O interface (such as ring, for example) for which notification
 *     must be generated.
 *
 * FirstSlotId (input)
 *     First slot in the output ring buffer to start dumping.
 *
 * LastSlotId (input)
 *     Last slot in the output ring buffer to stop dumping.
 *
 * fDumpGetAdmin (input)
 *     true if the output ring administration data must be dumped.
 *
 * fDumpGetCache (input)
 *     true if the output ring cache data must be dumped.
 *
 * Return value
 *     None
 */
void
PEC_Get_Dump(
        const unsigned int InterfaceId,
        const unsigned int FirstSlotId,
        const unsigned int LastSlotId,
        const bool fDumpGetAdmin,
        const bool fDumpGetCache);


#endif /* Include Guard */


/* end of file api_pec.h */
