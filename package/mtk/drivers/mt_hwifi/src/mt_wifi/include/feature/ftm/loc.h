/****************************************************************************
* Mediatek Inc.
* 5F., No.5, Taiyuan 1st St., Zhubei City,
* Hsinchu County 302, Taiwan, R.O.C.
* (c) Copyright 2014, Mediatek, Inc.
*
* All rights reserved. Mediatek's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code
* contains confidential trade secret material of Mediatek. Any attempt
* or participation in deciphering, decoding, reverse engineering or in any
* way altering the source code is stricitly prohibited, unless the prior
* written consent of Mediatek, Inc. is obtained.
****************************************************************************

	Module Name:
	loc.h

	Abstract:
	802.11mc FTM protocol function prototype declaration.
*/

#ifndef __LOC_CMD_H__
#define __LOC_CMD_H__

#include "hw_ctrl/rate_ctrl/ra_ctrl.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define CFG_LOC_MAX_CANDIDATES 10
#define CFG_LOC_ISTA_MAX_CANDIDATES 4
#define CFG_LOC_RSTA_MAX_CANDIDATES 1
#define WPA_LTF_KEYSEED_MAX_LEN 48
#define WPA_TK_MAX_LEN 32
#define CFG_LOC_DBG_NUM 6

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
/* 11mc Table 9-258 Format and bandwidth */
typedef enum _ENUM_LOC_FTM_BW_T {
	LOC_FTM_BW_HT_MIXED_BW20 = 0x9,
	LOC_FTM_BW_VHT_BW20  = 0xa,
	LOC_FTM_BW_HT_MIXED_BW40 = 0xb,
	LOC_FTM_BW_VHT_BW40 = 0xc,
	LOC_FTM_BW_VHT_BW80 = 0xd,
	LOC_FTM_BW_VHT_BW160 = 0x10,
	LOC_FTM_BW_HE_BW80 = 0x13,
	LOC_FTM_BW_HE_BW160 = 0x16,
} ENUM_LOC_FTM_BW_T;

/* Ranging status */
typedef enum _ENUM_LOC_STATUS_T {
	LOC_STATUS_SUCCESS       = 0,
	// general failure status
	LOC_STATUS_FAILURE       = 1,
	// target STA does not respond to request
	LOC_STATUS_FAIL_NO_RSP   = 2,
	// request rejected. Applies to 2-sided LOC only
	LOC_STATUS_FAIL_REJECTED = 3,
	LOC_STATUS_FAIL_NOT_SCHEDULED_YET  = 4,
	// timing measurement times out
	LOC_STATUS_FAIL_TM_TIMEOUT         = 5,
	// Target on different channel, cannot range
	LOC_STATUS_FAIL_AP_ON_DIFF_CHANNEL = 6,
	// ranging not supported
	LOC_STATUS_FAIL_NO_CAPABILITY  = 7,
	// request aborted for unknown reason
	LOC_STATUS_ABORTED             = 8,
	// Invalid T1-T4 timestamp
	LOC_STATUS_FAIL_INVALID_TS     = 9,
	// 11mc protocol failed
	LOC_STATUS_FAIL_PROTOCOL       = 10,
	// request could not be scheduled
	LOC_STATUS_FAIL_SCHEDULE       = 11,
	// responder cannot collaborate at time of request
	LOC_STATUS_FAIL_BUSY_TRY_LATER = 12,
	// bad request args
	LOC_STATUS_INVALID_REQ         = 13,
	// WiFi not enabled
	LOC_STATUS_NO_WIFI             = 14,
	// Responder overrides param info, cannot range with new params
	LOC_STATUS_FAIL_FTM_PARAM_OVERRIDE = 15,
	//Negotiation failure
	LOC_STATUS_NAN_RANGING_PROTOCOL_FAILURE = 16,
	//concurrency not supported (NDP+LOC)
	LOC_STATUS_NAN_RANGING_CONCURRENCY_NOT_SUPPORTED = 17,
	// ftmr tx fail (hw retry time excess), info driver rsta might not be the same channek
	LOC_STATUS_FTMR_TX_FAIL = 18,
} ENUM_LOC_STATUS_T;


/* LOC peer type */
typedef enum _ENUM_LOC_PEER_TYPE_T {
	LOC_PEER_AP = 0x1,
	LOC_PEER_STA  = 0x2,
	LOC_PEER_P2P_GO = 0x3,
	LOC_PEER_P2P_CLIENT = 0x4,
	LOC_PEER_NAN = 0x5
} ENUM_LOC_PEER_TYPE_T;


/* LOC Measurement Bandwidth */
typedef enum _ENUM_LOC_MEAS_BW_T {
	LOC_MEAS_BW_INVALID = 0x00, // driver only
	LOC_MEAS_BW_5   = 0x01,
	LOC_MEAS_BW_10  = 0x02,
	LOC_MEAS_BW_20  = 0x04,
	LOC_MEAS_BW_40  = 0x08,
	LOC_MEAS_BW_80  = 0x10,
	LOC_MEAS_BW_160 = 0x20,
	LOC_MEAS_BW_320 = 0x40
} ENUM_LOC_MEAS_BW_T;

/* LOC Measurement Preamble */
typedef enum _ENUM_LOC_PREAMBLE {
	LOC_PREAMBLE_INVALID = 0x0,
	LOC_PREAMBLE_LEGACY  = 0x1,
	LOC_PREAMBLE_HT      = 0x2,
	LOC_PREAMBLE_VHT     = 0x4,
	LOC_PREAMBLE_HE      = 0x8,
	LOC_PREAMBLE_EHT     = 0x10,
} ENUM_LOC_PREAMBLE;

/* LOC Support Protocol Type */
typedef enum _ENUM_LOC_TYPE_T {
	LOC_TYPE_1_SIDED          = 0x1,
	/* Deprecated. Use LOC_TYPE_2_SIDED_11MC instead. */
	LOC_TYPE_2_SIDED          = 0x2,
	LOC_TYPE_2_SIDED_11MC     = LOC_TYPE_2_SIDED,
	LOC_TYPE_2_SIDED_11AZ_NTB = 0x3,
	LOC_TYPE_2_SIDED_11AZ_TB = 0x4,
	LOC_TYPE_2_SIDED_11AZ_NTB_SLTF = 0x5,
	LOC_TYPE_2_SIDED_11AZ_TB_SLTF = 0x6,
} ENUM_LOC_TYPE_T;

/* Event Type*/
typedef enum _ENUM_LOC_EVET_TYPE_T {
	LOC_EVENT_PER_PACKET = 0x1,
	LOC_EVENT_PER_BURST  = 0x2
} ENUM_LOC_EVENT_TYPE_T;

typedef enum _ENUM_WIFI_CHANNEL_WIDTH_T {
	WIFI_CHAN_WIDTH_20    = 0,
	WIFI_CHAN_WIDTH_40    = 1,
	WIFI_CHAN_WIDTH_80    = 2,
	WIFI_CHAN_WIDTH_160   = 3,
	WIFI_CHAN_WIDTH_80P80 = 4,
	WIFI_CHAN_WIDTH_5     = 5,
	WIFI_CHAN_WIDTH_10    = 6,
	WIFI_CHAN_WIDTH_INVALID = -1
} ENUM_WIFI_CHANNEL_WIDTH_T;

typedef struct _WIFI_CHANNEL_INFO_T {
	INT_32 width;  // channel width (20, 40, 80, 80+80, 160, 320)
	INT_32 i4center_freq;                   // primary 20 MHz channel
	INT_32 center_freq0;                    // center frequency (MHz) first segment
	INT_32 center_freq1;                    // center frequency (MHz) second segment
} WIFI_CHANNEL_INFO_T, *P_WIFI_CHANNEL_INFO_T;


/* API for setting LCI/LCR information to be provided to a requestor */
typedef enum _ENUM_LOC_MOTION_PATTERN {
	WIFI_MOTION_NOT_EXPECTED = 0, // Not expected to change location
	WIFI_MOTION_EXPECTED = 1,     // Expected to change location
	WIFI_MOTION_UNKNOWN  = 2,     // Movement pattern unknown
} ENUM_LOC_MOTION_PATTERN;

typedef struct _LOC_INFORMATION_LCI_T {
	UINT_64 latitude;             // latitude in degrees * 2^25 , 2's complement
	UINT_64 longitude;            // latitude in degrees * 2^25 , 2's complement
	INT_8  altitude;              // Altitude in units of 1/256 m
	UINT_8 latitude_unc;          // As defined in Section 2.3.2 of IETF RFC 6225
	UINT_8 longitude_unc;         // As defined in Section 2.3.2 of IETF RFC 6225
	UINT_8 altitude_unc;          // As defined in Section 2.4.5 from IETF RFC 6225:

	//Following element for configuring the Z subelement
	ENUM_LOC_MOTION_PATTERN motion_pattern;
	INT_8  floor;                 // floor in units of 1/16th of floor. 0x80000000 if unknown.
	INT_8  height_above_floor;    // in units of 1/64 m
	INT_8  height_unc;            // in units of 1/64 m. 0 if unknown
	INT_8  reserved;
} LOC_INFORMATION_LCI_T, *P_LOC_INFORMATION_LCI_T;

typedef struct _LOC_INFORMATION_LCR_T {
	UINT_8 country_code[2];       // country code
	INT_8  length;                // length of the info field
	INT_8  reserved;
	UINT_8 civic_info[256];       // Civic info to be copied in FTM frame
} LOC_INFORMATION_LCR_T, *P_LOC_INFORMATION_LCR_T;

typedef struct _CMD_GET_LOC_CAP_T {
	// if ftm LOC data collection is supported - Initiator
	// B0: 1-side RTT  B1: mc B2: NTB, B3: TB, B4: NTB_Phy B5: TB_phy
	UINT_16 u4LOCInitSupported;
	// if 11mc responder mode is supported - Responder
	// B0: 1-side RTT  B1: mc B2: NTB, B3: TB, B4: NTB_Phy B5: TB_phy
	UINT_16 u4ResponderSupported;
	// if initiator supports LCI request. Applies to 2-sided LOC
	UINT_8 u4LciSupport;
	// if initiator supports LCR request. Applies to 2-sided LOC
	UINT_8 u4LcrSupport;
	// bit mask indicates what preamble
	UINT_16 u4PreambleSupport;
	// bit mask indicates what BW
	UINT_16 u4BwSupport;
	UINT_16 u4AzBwSupport;
	UINT_32 u4MinDeltaTimePerPacket;
	UINT_32 u4Reserved;
} CMD_GET_LOC_CAP_T, *P_CMD_GET_LOC_CAP_T;

typedef struct     _LOC_CONFIG_T {
	UINT_8 aucAddr[MAC_ADDR_LEN];     /* peer device mac address */
	UINT_8 eType;                     /* enum ENUM_LOC_TYPE */
	WIFI_CHANNEL_INFO_T rChannel;
	UINT_16 u2BurstPeriod;            // mc:
	UINT_16 u2BurstExponent;
	// mc: preferred time to start first burst, reserved if no preference
	UINT_16 u2PreferencePartialTsfTimer;
	UINT_8 ucNumFramesPerBurst;
	UINT_8 ucLciRequest;
	UINT_8 ucLcrRequest;
	UINT_8 ucBurstDuration;          // mc:
	UINT_8 ePreamble;                /* enum ENUM_LOC_PREAMBLE */
	UINT_8 eBw;                      /* enum ENUM_LOC_MEAS_BW */
	UINT_8 eBand;
	UINT_8 ucPrimaryChannel;
	UINT_8 ucS1;
	UINT_8 ucS2;
	UINT_8 ucChannelWidth;
	UINT_8 ucBssIndex;
	UINT_8 ucASAP;
	UINT_8 ucFtmMinDeltaTime;        // mc: UNIT:100us
	UINT_8 ucReserved;               // 4 byte align
} LOC_CONFIG_T, *P_LOC_CONFIG_T;

typedef struct _PASN_T {
	UINT_8 ucCipher;
	UINT_8 ucShaType;
	UINT_8 ucTklen;
	UINT_8 ucLtfKeyseedLen;
	UINT_8 aucTk[WPA_TK_MAX_LEN];
	UINT_8 aucLtfKeyseed[WPA_LTF_KEYSEED_MAX_LEN];
} PASN_T, *P_PASN_T;

/* LOC configuration v3 (11az support)*/
typedef struct _LOC_CONFIG_NTB_T {
	LOC_CONFIG_T rLOCConfig;
	// 11az Non-Trigger-based (non-TB) minimum measurement time in units of 100 microseconds
	UINT_64 u8NtbMinMeasTime;
	// 11az Non-Trigger-based (non-TB) maximum measurement time in units of 10 milliseconds
	UINT_64 u8NtbMaxMeasTime;
} LOC_CONFIG_NTB_T, *P_LOC_CONFIG_NTB_T;

typedef struct _LOC_CONFIG_NTB_PHY_SEC_T {
	LOC_CONFIG_T rLOCConfig;
	UINT_64 u8NtbMinMeasTime; // units of 100 microseconds
	UINT_64 u8NtbMaxMeasTime; // units of 10 milliseconds
	UINT_8  u8ImmdiateI2rFeedback;
	PASN_T  rPasnInfo;
} LOC_CONFIG_NTB_PHY_SEC_T, *P_LOC_CONFIG_NTB_PHY_SEC_T;


typedef struct _LOC_RSTA_CONFIG_T {
	LOC_CONFIG_T rLOCConfig;
	LOC_INFORMATION_LCI_T rLocLciInfo;  //To Be Design
	LOC_INFORMATION_LCR_T rLocLcrInfo;
} LOC_RSTA_CONFIG_T, *P_LOC_RSTA_CONFIG_T;


/* wifi rate */
typedef struct _WIFI_RATE_T {
	UINT_32 u4Preamble    : 3;   // 0: OFDM, 1:CCK, 2:HT 3:VHT 4:HE 5:EHT 6..7 reserved
	UINT_32 u4Nss         : 2;   // 0:1x1, 1:2x2, 2:3x3, 3:4x4
	UINT_32 u4Bw          : 3;   // 0:20MHz, 1:40Mhz, 2:80Mhz, 3:160Mhz 4:320Mhz
	// OFDM/CCK rate code would be as per ieee std in the units of 0.5mbps
	UINT_32 u4RateMcsIdx  : 8;
	// HT/VHT/HE it would be mcs index
	UINT_32 u4Reserved    : 16;  // reserved
	UINT_32 u4Bitrate;           // units of 100 Kbps
} WIFI_RATE_T, *P_WIFI_RATE_T;

typedef struct _LOC_RESULT_T {
	UINT_8 aucMacAddr[MAC_ADDR_LEN];
	UINT_32 u4BurstNum;         // burst number in a multi-burst request.  IEEE 802.11mc only.
	UINT_32 u4MeasurementNumber;// Total LOC measurement frames attempted
	UINT_32 u4SuccessNumber;    // Total successful LOC measurement frames
	UINT_8 ucNumPerBurstPeer;   // Maximum number of "FTM frames per burst" supported by
	// the responder STA. Applies to 2-sided LOC only.
	ENUM_LOC_STATUS_T eStatus;  // ranging status
	UINT_8 ucRetryAfterDuration;
	ENUM_LOC_TYPE_T eType;      // LOC type
	INT_32 i4Rssi;              // average rssi in 0.5 dB steps e.g. 143 implies -71.5 dB
	INT_32 i4RssiSpread;        // rssi spread in 0.5 dB steps e.g. 5 implies 2.5 dB
	WIFI_RATE_T rTxRate;        // 2-sided LOC: TX rate of iSTA's Ack in response to FTM frame.
	WIFI_RATE_T rRxRate;        // 2-sided LOC: TX rate of FTM frame coming from responder.
	INT_64 i8LOC;               // round trip time in picoseconds
	INT_64 i8LOCSd;             // No support
	INT_64 i8LOCSpread;         // No support
	INT_32 i4DistanceMM;        // distance in mm (optional)
	INT_32 i4DistanceSdMM;      // No support
	INT_32 i4DistanceSpreadMM;  // No support
	INT_64 i8Ts;                // time of the measurement (in microseconds since boot)
	// in ms, actual time taken by the FW to finish one burst measurement.
	INT_32 i4BurstDuration;
	INT_32 i4NegotiateBurstNum; // Number of bursts allowed by the responder.
	LOC_INFORMATION_LCI_T *LCI; // for 11mc and 11az only
	LOC_INFORMATION_LCR_T *LCR; // for 11mc and 11az only
} LOC_RESULT_T, *P_LOC_RESULT_T;

/* LOC results version 2 */
typedef struct _LOC_RESULT_V2_T {
	// Legacy wifi LOC result structure
	LOC_RESULT_T rLOCResult;
	// primary channel frequency (MHz) used for ranging measurements
	WIFI_CHANNEL_INFO_T rChannel;
	// If frequency is unknown, this will be set to |UNSPECIFIED(-1)|
	// LOC packet bandwidth is an average BW of the BWs of LOC frames.
	// Cap the average close to a specific valid LOCBw.
	ENUM_LOC_MEAS_BW_T ucLOCBw;
} LOC_RESULT_V2_T, *P_LOC_RESULT_V2_T;

/* LOC results v3 (11az support)*/
typedef struct _LOC_RESULT_AZ_T {
	LOC_RESULT_V2_T rLOCResult;
	/* Multiple transmissions of HE-LTF symbols in an HE (I2R) Ranging NDP.
	 * An HE-LTF repetition value of 1 indicates no repetitions. */
	UINT_8 ucI2RLtf;
	/* Multiple transmissions of HE-LTF symbols in an HE (R2I) Ranging NDP.
	 * An HE-LTF repetition value of 1 indicates no repetitions. */
	UINT_8 ucR2ILtf;
	/* Minimum non-trigger based (non-TB) dynamic measurement time
	 * in units of 100 microseconds assigned by the 11az responder. */
	UINT_64 u8NtbMinMeasTime;
	/* Maximum non-trigger based (non-TB) dynamic measurement time
	 * in units of 10 milliseconds assigned by the 11az responder. */
	UINT_64 u8NtbMaxMeasTime;
	// Number of transmit space-time streams used.
	UINT_8 ucTxSts;
	// Number of receive space-time streams used.
	UINT_8 ucRxSts;
} LOC_RESULT_AZ_T, *P_LOC_RESULT_AZ_T;

/* LOC result callbacks */
/* TBD*/

typedef struct _CMD_LOC_REQUEST_T {
	UINT_8 ucSeqNum;
	UINT_8 fgEnable;
	UINT_8 ucConfigNum;
	UINT_8 ucPaddings[5];
	LOC_CONFIG_T arLOCConfigs[CFG_LOC_MAX_CANDIDATES];
} CMD_LOC_REQUEST_T, *P_CMD_LOC_REQUEST_T;

typedef struct _EVENT_LOC_RESULT_T {
	LOC_RESULT_T rResult;
	UINT_16 u2IELen;
	UINT_8 aucIE[0];
} EVENT_LOC_RESULT_T, *P_EVENT_LOC_RESULT_T;

typedef struct _EVENT_LOC_DONE_T {
	UINT_8 ucSeqNum;
} EVENT_LOC_DONE_T, *P_EVENT_LOC_DONE_T;

struct _FTM_FMT_MAP {
	UINT8 ftm_fmt_and_bw;
	UINT8 loc_cfg_preamble;
	UINT8 loc_cfg_bandwidth;
};

#endif /* __LOC_CMD_H__ */
