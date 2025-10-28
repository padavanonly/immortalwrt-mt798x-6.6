/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc
 */



#ifndef _CMM_FW_UNI_EVENT_H
#define _CMM_FW_UNI_EVENT_H

#ifdef WIFI_UNIFIED_COMMAND
#ifdef SMART_CARRIER_SENSE_SUPPORT
#include "scs.h"
#endif /*SMART_CARRIER_SENSE_SUPPORT */
#ifdef FTM_SUPPORT
#include "feature/ftm/loc.h"
#endif /* FTM_SUPPORT */

#define RAM_BAND_NUM                    2

/*
 * TLV element structure should start with a 2-byte Tag field and a 2-byte
 * length field and pad to 4-byte alignment. The u2Length field indicate
 * the length of the whole TLV including tag and length field.
 */
struct GNU_PACKED TAG_HDR {
	UINT16 u2Tag;
	UINT16 u2Length;
};

typedef void(*PROCESS_RX_UNSOLICIT_EVENT_FUNCTION) (
	VOID *physical_dev, UINT8 *, UINT32, EVENT_RXD *);

#ifndef ALIGN_4
	#define ALIGN_4(_value)             (((_value) + 3) & ~3u)
#endif /* ALIGN_4 */

#define TAG_ID(fp)	(((struct TAG_HDR *) fp)->u2Tag)
#define TAG_LEN(fp)	(((struct TAG_HDR *) fp)->u2Length)
#define TAG_HDR_LEN sizeof(struct TAG_HDR)

#define UNI_EVENT_HERA_METRIC_USERS         4
#define UNI_HWCFG_INF0_MAX_DUMP_SIZE 1200
#define UNI_EVENT_IPI_HIST_TYPE_NUM 12

#define UNI_CMD_RDD_EVENT_MAX_HW_PB_SIZE 32
#define UNI_CMD_RDD_EVENT_PPB_SIZE 32
#define UNI_CMD_RDD_EVENT_LPB_SIZE 32

#define TAG_FOR_EACH(_pucTlvBuf, _u4TlvBufLen, _u2Offset) \
for ((_u2Offset) = 0;	\
	(((_u2Offset) <= (USHRT_MAX - (UINT16)4)) && \
	(((_u2Offset) + (UINT16)4) <= (_u4TlvBufLen)) && \
	((TAG_LEN(_pucTlvBuf)) > 0) && \
	(((_u2Offset) + TAG_LEN(_pucTlvBuf)) <= (_u4TlvBufLen))); \
	(_u2Offset) += TAG_LEN(_pucTlvBuf), (_pucTlvBuf) += TAG_LEN(_pucTlvBuf))

struct GNU_PACKED UNI_EVENT_CMD_RESULT_T {
	UINT16 u2CID;
	UINT8 aucReserved[2];
	UINT32 u4Status;
};

typedef enum _ENUM_UNI_EVENT_ID_T {
	UNI_EVENT_ID_CMD_RESULT      = 0x01,  /* Generic event for return cmd status */
	UNI_EVENT_ID_BMC_RPY_DT      = 0x02,
	UNI_EVENT_ID_HIF_CTRL        = 0x03,
	UNI_EVENT_ID_FW_LOG_2_HOST   = 0x04,
	UNI_EVENT_ID_ACCESS_REG      = 0x06,
	UNI_EVENT_ID_CHIP_CONFIG     = 0x07,
	UNI_EVENT_ID_SMESH_INFO      = 0x08,
	UNI_EVENT_ID_IE_COUNTDOWN    = 0x09,
	UNI_EVENT_ID_ASSERT_DUMP     = 0x0A,
	UNI_EVENT_ID_SLEEP_NOTIFY    = 0x0B,
	UNI_EVENT_ID_BEACON_TIMEOUT  = 0x0C,
	UNI_EVENT_ID_PS_SYNC         = 0x0D,
	UNI_EVENT_ID_SCAN_DONE       = 0x0E,
	UNI_EVENT_ID_STAREC          = 0x0F,
	UNI_EVENT_ID_ECC_CAL         = 0x10,
	UNI_EVENT_ID_RDD             = 0x11,
	UNI_EVENT_ID_ADD_KEY_DONE    = 0x12,
	UNI_EVENT_ID_OBSS_UPDATE     = 0x13,
	UNI_EVENT_ID_SER             = 0x14,
	UNI_EVENT_ID_IDC             = 0x17,
	UNI_EVENT_ID_RDD_IPI_HIST_CTRL = 0x19,
	UNI_EVENT_ID_MAC_INFO        = 0x1A,
	UNI_EVENT_ID_TDLS            = 0x1B,
	UNI_EVENT_ID_SAP             = 0x1C,
	UNI_EVENT_ID_TXCMD_CTRL      = 0x1D,
	UNI_EVENT_ID_BAND_CONFIG     = 0x21,
	UNI_EVENT_ID_MIB             = 0x22,
	UNI_EVENT_ID_STATISTICS      = 0x23,
	UNI_EVENT_ID_SR              = 0x25,
	UNI_EVENT_ID_SCS             = 0x26,
	UNI_EVENT_ID_CNM             = 0x27,
	UNI_EVENT_ID_MBMC            = 0x28,
	UNI_EVENT_ID_BSS_IS_ABSENCE  = 0x29,
	UNI_EVENT_ID_TXPOWER         = 0x2A,
	UNI_EVENT_ID_BA_OFFLOAD      = 0x2C,
	UNI_EVENT_ID_RA				 = 0x2F,
	UNI_EVENT_ID_SPECTRUM        = 0x30,
	UNI_EVENT_ID_MURU            = 0x31,
	UNI_EVENT_ID_TESTMODE_RX_STAT_INFO  = 0x32,
	UNI_EVENT_ID_BF              = 0X33,
	UNI_EVENT_ID_THERMAL         = 0x35,
	UNI_EVENT_ID_NOISE_FLOOR     = 0x36,
	UNI_EVENT_ID_TPC             = 0x38,
	UNI_EVENT_ID_MEC             = 0x3A,
	UNI_EVENT_ID_CHIP_CAPABILITY = 0x43,
	UNI_EVENT_ID_TESTMODE_CTRL   = 0x46,
	UNI_EVENT_ID_TWT_SYNC        = 0x47,
	UNI_EVENT_ID_EFUSE_CONTROL   = 0x48,
	UNI_EVENT_ID_CSI             = 0x4A,
	UNI_EVENT_ID_CAL             = 0x4C,
	UNI_EVENT_ID_RXFE_CTRL       = 0x4D,
	UNI_EVENT_ID_HWCFG_CTRL      = 0x4E,
	UNI_EVENT_ID_EAP_CTRL        = 0x51,
	UNI_EVENT_ID_BSS_ER          = 0x53,
	UNI_EVENT_ID_RRO             = 0x57,
	UNI_EVENT_ID_MLO_ALGO        = 0x59,
	UNI_EVENT_ID_PP              = 0x5A,
#ifdef FTM_SUPPORT
	UNI_EVENT_ID_LOCATION        = 0x5D,
#endif /* FTM_SUPPORT */
	UNI_EVENT_ID_STATE           = 0x6C,
	UNI_EVENT_ID_PER_STA_INFO    = 0x6D,
	UNI_EVENT_ID_ALL_STA_INFO    = 0x6E,
	UNI_EVENT_ID_EPCS	         = 0x6F,
	UNI_EVENT_ID_BSS_MATCH	     = 0x71,
	UNI_EVENT_ID_SCSA            = 0x7A,
	UNI_EVENT_ID_MLD             = 0x81,
	UNI_EVENT_ID_MAX_NUM
} ENUM_UNI_EVENT_ID_T, *P_ENUM_UNI_EVENT_ID_T;

#ifdef ZERO_PKT_LOSS_SUPPORT
struct _UNI_EVENT_SCSA_T {
	/* fixed field */
	UINT_8 aucReserved[4];

	/* tlv */
	UINT_8 aucTlvBuffer[0];/**< the TLVs included in this field:
		*
		*   TAG                              | ID  | structure
		*   -----------------                | ----| -------------
		*   UNI_EVENT_SCSA_BCN_TXOK_STATUS     | 0x0 | UNI_EVENT_SCSA_BCN_TXOK_STATUS_T
		*/
};

enum _UNI_EVENT_SCSA_TAG_T {
	UNI_EVENT_SCSA_BCN_TXOK_STATUS = 0x00,
	UNI_EVENT_SCSA_WTBL_SKIP_TX_STATUS = 0x01,
	UNI_EVENT_SCSA_MAX_NUM
};

struct _UNI_EVENT_BCN_TXOK_STATUS_T {
	UINT_16 u2Tag;    // Tag = 0x0
	UINT_16 u2Length;
	UINT_8 u1BandIdx;
	UINT_8 u1Csa_count;
	UINT_8 u1OwnMacIdx;
	UINT_8 aucReserve;
};

struct _UNI_EVENT_WTBL_SKIP_TX_STATUS_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_16 u2WlanIdx;
	BOOLEAN fgWtblSkipTXEnable;
	UINT_8  aucReserve;
};

struct _UNI_EVENT_WTBL_SKIP_TX_STATUS_RESULT_T {
	UINT_16 u2WlanIdx;
	BOOLEAN fgWtblSkipTXEnable;
};
#endif

typedef struct GNU_PACKED _UNI_EVENT_FW_LOG2HOST_T
{
    /* fixed field */
    UINT8 ucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
						*
						*   TAG                              | ID  | structure
						*   ---------------------------------|-----|--------------
						*   UNI_EVENT_FW_LOG_FORMAT          | 0x0 | UNI_EVENT_FW_LOG_FORMAT (Should always be the last TLV element)
						*/
} UNI_EVENT_FW_LOG2HOST_T, *P_UNI_EVENT_FW_LOG2HOST_T;

/* FW Log 2 Host event Tag */
typedef enum _UNI_EVENT_FWLOG2HOST_TAG_T {
	UNI_EVENT_FW_LOG_FORMAT = 0,
	UNI_EVENT_FW_LOG_MEMORY = 1,
	UNI_EVENT_FW_LOG_MAX_NUM
} UNI_EVENT_FWLOG2HOST_TAG_T;

/* mobile */
/* Rebb */
enum {
    DEBUG_MSG_TYPE_MEM8 = 0x00,
    DEBUG_MSG_TYPE_MEM32 = 0x01,
    DEBUG_MSG_TYPE_ASCII = 0x02,
    DEBUG_MSG_TYPE_BINARY = 0x03,
    DEBUG_MSG_TYPE_END
};

/* FW Log with Format (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_FW_LOG_FORMAT_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucMsgFmt;
    UINT8 ucReserved[3];
    UINT8 acMsg[0];
} UNI_EVENT_FW_LOG_FORMAT_T, *P_UNI_EVENT_FW_LOG_FORMAT_T;

typedef struct GNU_PACKED _UNI_EVENT_ACCESS_REG_T {
    /* fixed field */
    UINT8 ucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
							*
							*   TAG                              | ID  | structure
							*   ---------------------------------|-----|---------------------------------
							*   UNI_EVENT_ACCESS_REG_BASIC       | 0x0 | UNI_EVENT_ACCESS_REG_BASIC_T
							*   UNI_EVENT_ACCESS_RF_REG_BASIC    | 0x1 | UNI_EVENT_ACCESS_RF_REG_BASIC_T
							*/
} UNI_EVENT_ACCESS_REG_T, *P_UNI_EVENT_ACCESS_REG_T;

/* Register access event Tag */
typedef enum _UNI_EVENT_ACCESS_REG_TAG_T {
    UNI_EVENT_ACCESS_REG_BASIC = 0,
    UNI_EVENT_ACCESS_RF_REG_BASIC,
    UNI_EVENT_ACCESS_REG_MAX_NUM
} UNI_EVENT_ACCESS_REG_TAG_T;

/* Access Register (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_ACCESS_REG_BASIC_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT32 u4Addr;
    UINT32 u4Value;
} UNI_EVENT_ACCESS_REG_BASIC_T, *P_UNI_EVENT_ACCESS_REG_BASIC_T;

/* Access RF address (Tag1) */
typedef struct GNU_PACKED _UNI_EVENT_ACCESS_RF_REG_BASIC_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT16 u2WifiStream;
    UINT16 u2Reserved;
    UINT32 u4Addr;
    UINT32 u4Value;
} UNI_EVENT_ACCESS_RF_REG_BASIC_T, *P_UNI_EVENT_ACCESS_RF_REG_BASIC_T;

typedef struct GNU_PACKED _UNI_EVENT_SMESH_INFO_T {
    /* fixed field */
    UINT8 ucBand;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_SMESH_INFO_T, *P_UNI_EVENT_SMESH_INFO_T;

typedef struct GNU_PACKED _UNI_EVENT_IE_COUNTDOWNT_T {
    /* fixed field */
    UINT8 ucBand;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_IE_COUNTDOWN_T, *P_UNI_EVENT_IE_COUNTDOWN_T;

/* IE countdown event Tag */
typedef enum _UNI_EVENT_IE_COUNTDOWN_TAG_T {
	UNI_EVENT_IE_COUNTDOWN_CSA = 0,
	UNI_EVENT_IE_COUNTDOWN_BCC = 1,
	UNI_EVENT_IE_COUNTDOWN_STATIC_PUNCTURE_CSA = 2,
	UNI_EVENT_IE_COUNTDOWN_MAX_NUM
} UNI_EVENT_IE_COUNTDOWN_TAG_T;

/* CSA notify Parameters (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_CSA_NOTIFY_T {
    UINT16 u2Tag;    /* Tag = 0x00 */
    UINT16 u2Length;
    UINT8 ucOwnMacIdx;
    UINT8 ucChannelSwitchCount;
    UINT8 aucPadding[2];
} UNI_EVENT_CSA_NOTIFY_T, *P_UNI_EVENT_CSA_NOTIFY_T;

/* BCC notify Parameters (Tag1) */
typedef struct GNU_PACKED _UNI_EVENT_BCC_NOTIFY_T {
    UINT16 u2Tag;    /* Tag = 0x01 */
    UINT16 u2Length;
    UINT8 ucOwnMacIdx;
    UINT8 ucColorSwitchCount;
    UINT8 aucPadding[2];
} UNI_EVENT_BCC_NOTIFY_T, *P_UNI_EVENT_BCC_NOTIFY_T;

typedef struct GNU_PACKED _UNI_EVENT_ASSERT_DUMP_T {
    /*fixed field*/
    UINT8 ucBssIndex;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0]; /**< the TLVs included in this field:
							*
							*   TAG                             | ID  | structure
							*   -------------                   | ----| -------------
							*   UNI_EVENT_ASSERT_CONTENT_DUMP   | 0x0 | UNI_EVENT_ASSERT_CONTENT_T
							*/
} UNI_EVENT_ASSERT_DUMP_T, *P_UNI_EVENT_ASSERT_DUMP_T;

/* Assert Dump event Tag */
typedef enum _UNI_EVENT_ASSERT_DUMP_TAG_T {
    UNI_EVENT_ASSERT_CONTENT_DUMP = 0,
} UNI_EVENT_ASSERT_DUMP_TAG_T;


/* Sleep Notify (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_ASSERT_CONTENT_T {
    UINT16 u2Tag;                   /* Tag = 0x00 */
    UINT16 u2Length;

    UINT8 aucBuffer[0];
} UNI_EVENT_ASSERT_CONTENT_T, *P_UNI_EVENT_ASSERT_CONTENT_T;

/* ============== UNI_EVENT_ID_SLEEP_NOTIFY Start ============ */
struct GNU_PACKED UNI_EVENT_SLEEP_NOTIFY_T {
	/*fixed field*/
	UINT8 ucBssIndex;
	UINT8 aucPadding[3];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

enum UNI_EVENT_SLEEP_NOTYFY_TAG_T {
	UNI_EVENT_SLEEP_INFO = 0,
};

struct GNU_PACKED UNI_EVENT_SLEEP_INFO_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucSleepyState;
	UINT8 aucPadding[3];
};
/* ============== UNI_EVENT_ID_SLEEP_NOTIFY End ============ */

typedef struct GNU_PACKED _UNI_EVENT_BEACON_TIMEOUT_T {
    /* fixed field */
    UINT8 ucBssIndex;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_BEACON_TIMEOUT_T, *P_UNI_EVENT_BEACON_TIMEOUT_T;

typedef enum _UNI_EVENT_BEACON_TIMEOUT_TAG_T {
    UNI_EVENT_BEACON_TIMEOUT_INFO = 0,
    UNI_EVENT_BEACON_TIMEOUT_MAX_NUM
} UNI_EVENT_BEACON_TIMEOUT_TAG_T;

/* Beacon timeout reason (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_BEACON_TIMEOUT_INFO_T {
    UINT16 u2Tag;    /* Tag = 0x00 */
    UINT16 u2Length;
    UINT8 ucReasonCode;
    UINT8 aucPadding[3];
} UNI_EVENT_BEACON_TIMEOUT_INFO_T, *P_UNI_EVENT_BEACON_TIMEOUT_INFO_T;

typedef enum _UNI_ENUM_BCN_TIMEOUT_REASON_T {
    UNI_ENUM_BCN_LOSS_STA = 0x00,
    UNI_ENUM_BCN_LOSS_ADHOC = 0x01,
    UNI_ENUM_BCN_NULL_FRAME_THRESHOLD = 0x02,
    UNI_ENUM_BCN_PERIOD_NOT_ILLIGAL = 0x03,
    UNI_ENUM_BCN_CONNECTION_FAIL = 0x04,
    UNI_ENUM_BCN_ALLOCAT_NULL_PKT_FAIL_THRESHOLD = 0x05,
    UNI_ENUM_BCN_UNSPECIF_REASON = 0x06,
    UNI_ENUM_BCN_NULL_FRAME_LIFE_TIMEOUT = 0x07,
    UNI_ENUM_BCN_LOSS_AP_DISABLE = 0x08,
    UNI_ENUM_BCN_LOSS_AP_ERROR = 0x09,
    UNI_ENUM_BCN_TIMEOUT_REASON_MAX_NUM
} UNI_ENUM_BCN_TIMEOUT_REASON_T;


typedef struct GNU_PACKED _UNI_EVENT_PS_SYNC_T {
    /*fixed field*/
    UINT8 ucBssIndex;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0]; /**< the TLVs included in this field:
							*
							*   TAG                             | ID  | structure
							*   -------------                   | ----| -------------
							*   UNI_EVENT_CLIENT_PS_INFO          | 0x0 | UNI_EVENT_CLIENT_PS_INFO_T
							*/
} UNI_EVENT_PS_SYNC_T, *P_UNI_EVENT_PS_SYNC_T;

/* PS Sync event Tag */
typedef enum _UNI_EVENT_PS_SYNC_TAG_T {
	UNI_EVENT_CLIENT_PS_INFO = 0,
	UNI_EVENT_MULTI_CLIENT_PS_INFO,
} UNI_EVENT_PS_SYNC_TAG_T;

/* PS SYNC (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_CLIENT_PS_INFO_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    UINT8 ucPsBit;
    UINT8 aucPadding[1];
    UINT16 ucWtblIndex;
    UINT8 ucBufferSize;
    UINT8 aucReserved[3];
} UNI_EVENT_CLIENT_PS_INFO_T, *P_UNI_EVENT_CLIENT_PS_INFO_T;

#define MULTI_PS_PSBIT              BIT(15)
#define MULTI_PS_PSBIT_OFFSET       15
#define MULTI_PS_BUF_SIZE           BITS(12, 14)
#define MULTI_PS_BUF_SIZE_OFFSET    12
#define MULTI_PS_WTBL_INDEX         BITS(0, 10)
#define MULTI_PS_WTBL_INDEX_OFFSET  0

#define GET_MULTI_PS_PSBIT(p) \
	(((p) & MULTI_PS_PSBIT) >> MULTI_PS_PSBIT_OFFSET)

#define GET_MULTI_PS_BUF_SIZE(p) \
	(((p) & MULTI_PS_BUF_SIZE) >> MULTI_PS_BUF_SIZE_OFFSET)

#define GET_MULTI_PS_WTBL_INDEX(p) \
	(((p) & MULTI_PS_WTBL_INDEX) >> MULTI_PS_WTBL_INDEX_OFFSET)

struct GNU_PACKED UNI_EVENT_MULTI_PS_EVENT_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2StaCnt;
	UINT16 u2StaPsInfo[256];
	UINT8 aucReserved[2];
};

/* ============== UNI_EVENT_ID_CSI Begin ============== */
struct GNU_PACKED UNI_EVENT_CSI_T {
	/* fixed field */
	UINT8 band_idx;
	UINT8 aucPadding[3];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

enum UNI_EVENT_CSI_TAG_T {
	UNI_EVENT_CSI_DATA = 0,
	UNI_EVENT_CSI_MAX_NUM
};

struct GNU_PACKED UNI_EVENT_CSI_DATA_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  aucBuffer[0];
};
/* ============== UNI_EVENT_ID_CSI End ============== */

/* ECC event (0x10) */
typedef struct GNU_PACKED _UNI_EVENT_ECC_CAL_T {
    /* fixed field */
    UINT8 aucPadding[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_ECC_CAL_T, *P_UNI_EVENT_ECC_CAL_T;

/* ECC operation event tags */
typedef enum _UNI_EVENT_ECC_CAL_TAG_T {
    UNI_EVENT_ECC_CAL_GROUP_POINT_RESULT   = 0x00,
    UNI_EVENT_ECC_CAL_MAX_NUM
} UNI_EVENT_ECC_AL_TAG_T;

/* ECC Operation Parameters (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_ECC_CAL_RES_T {
    UINT16 u2Tag;    /* Tag = 0x00 */
    UINT16 u2Length;

    UINT8 ucDqxDataLength;
    UINT8 ucDqyDataLength;
    UINT8 ucEccCmdId;
    UINT8 ucIsResFail;
    UINT8 aucDqxBuffer[48];
    UINT8 aucDqyBuffer[48];
} UNI_EVENT_ECC_CAL_RES_T, *P_UNI_EVENT_ECC_CAL_RES_T;

typedef struct GNU_PACKED _UNI_EVENT_SER_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
						*
						*   TAG                          | ID  | structure
						*   -------------                | ----| -------------
						*   UNI_EVENT_SER_QUERY_CMM      | 0x0 | UNI_EVENT_SER_QUERY_CMM_T
						*   UNI_EVENT_SER_QUERY_BAND     | 0x1 | UNI_EVENT_SER_QUERY_BAND_T
						*   UNI_EVENT_SER_QUERY_WFDMA    | 0x2 | UNI_EVENT_SER_QUERY_WFDMA_T
						*/
} UNI_EVENT_SER_T, *P_UNI_EVENT_SER_T;

/* SER event Tag */
typedef enum _UNI_EVENT_SER_TAG_T {
    UNI_EVENT_SER_QUERY_CMM = 0,
    UNI_EVENT_SER_QUERY_BAND = 1,
    UNI_EVENT_SER_QUERY_WFDMA = 2,
    UNI_EVENT_SER_MAX_NUM
} UNI_EVENT_SER_TAG_T;

/* Beacon timeout reason (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_SER_QUERY_CMM_T {
    UINT16   u2Tag;    /* Tag = 0x00 */
    UINT16   u2Length;
    UINT8    ucEnableSER;
    UINT8    ucSerL1RecoverCnt;
    UINT8    ucSerL2RecoverCnt;
    UINT8    aucPaddings[1];
    UINT16   u2PSEErrorCnt[32];
    UINT16   u2PSEError1Cnt[32];
    UINT16   u2PLEErrorCnt[32];
    UINT16   u2PLEError1Cnt[32];
    UINT16   u2PLEErrorAmsduCnt[32];
} UNI_EVENT_SER_QUERY_CMM_T, *P_UNI_EVENT_SER_QUERY_CMM_T;

/* Per band SER counter (Tag1) */
typedef struct GNU_PACKED _UNI_EVENT_SER_QUERY_BAND_T {
    UINT16   u2Tag;    /* Tag = 0x01 */
    UINT16   u2Length;
    UINT8    ucBandIdx;
    UINT8    ucL3RxAbortCnt;
    UINT8    ucL3TxAbortCnt;
    UINT8    ucL3TxDisableCnt;
    UINT8    ucL4RecoverCnt;
    UINT8    aucPaddings[3];
    UINT16   au2LMACError6Cnt[32];
    UINT16   au2LMACError7Cnt[32];
} UNI_EVENT_SER_QUERY_BAND_T, *P_UNI_EVENT_SER_QUERY_BAND_T;

/* wfdma counter (Tag2) */
typedef struct GNU_PACKED _UNI_EVENT_SER_QUERY_WFDMA_T {
    UINT16   u2Tag;    /* Tag = 0x02 */
    UINT16   u2Length;
    UINT16   au2WfdmaTxBusyCnt[4];
    UINT16   au2WfdmaRxBusyCnt[4];
} UNI_EVENT_SER_QUERY_WFDMA_T, *P_UNI_EVENT_SER_QUERY_WFDMA_T;

#ifdef WIFI_MD_COEX_SUPPORT
struct _UNI_EVENT_IDC_T {
	UINT8 aucReserved[4];
	UINT8 aucTlvBuffer[0];
};

/* IDC event Tag */
enum _UNI_EVENT_IDC_TAG_T {
	UNI_EVENT_IDC_MD_SAFE_CHN	= 0,
	UNI_EVENT_IDC_CCCI_MSG	= 1,
	UNI_EVENT_IDC_INFO	= 2,
	UNI_EVENT_IDC_MAX_NUM
};
#endif

typedef struct GNU_PACKED _UNI_EVENT_MAC_IFNO_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_MAC_IFNO_T, *P_UNI_EVENT_MAC_IFNO_T;

/* Mac info event Tag */
typedef enum _UNI_EVENT_MAC_INFO_TAG_T {
	UNI_EVENT_MAC_INFO_TSF  = 0,
	UNI_EVENT_MAC_INFO_TSF_DIFF  = 2,
	UNI_EVENT_MAC_INFO_MAX_NUM
} UNI_EVENT_MAC_INFO_TAG_T;

/* Beacon timeout reason (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_MAC_INFO_TSF_T {
    UINT16   u2Tag;    /* Tag = 0x00 */
    UINT16   u2Length;
    UINT8    ucDbdcIdx;
    UINT8    ucHwBssidIndex;
    UINT8    aucPadding[2];
    UINT32   u4TsfBit0_31;
    UINT32   u4TsfBit63_32;
} UNI_EVENT_MAC_INFO_TSF_T, *P_UNI_EVENT_MAC_INFO_TSF_T;

/* tsf diff (Tag2) */
struct GNU_PACKED UNI_EVENT_MAC_INFO_TSF_DIFF_T {
	UINT16   u2Tag;    /* Tag = 0x02 */
	UINT16   u2Length;
	UINT32   u4Tsf0Bit0_31;
	UINT32   u4Tsf0Bit63_32;
	UINT32   u4Tsf1Bit0_31;
	UINT32   u4Tsf1Bit63_32;
};

/* TXCMD Ctrl command (0x1D) */
typedef struct GNU_PACKED _UNI_EVENT_TXCMD_CTRL_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_TXCMD_CTRL_T, *P_UNI_EVENT_TXCMD_CTRL_T;

/* TXCMD ctrl cmd tags */
typedef enum _UNI_EVENT_TXCMD_CTRL_TAG_T {
    UNI_EVENT_GET_TXCMD_DBG_STATUS = 0x0f,
    UNI_EVENT_GET_TXCMD_DBG_SXN_GLOBAL = 0x10,
    UNI_EVENT_GET_TXCMD_DBG_SXN_PROTECT = 0x11,
    UNI_EVENT_GET_TXCMD_DBG_SXN_TXDATA = 0x12,
    UNI_EVENT_GET_TXCMD_DBG_SXN_TRIGDATA = 0x13,
    UNI_EVENT_GET_TXCMD_DBG_TF_TXD = 0x14,
    UNI_EVENT_GET_TXCMD_DBG_TF_BASIC = 0x15,
    UNI_EVENT_GET_TXCMD_DBG_SXN_SW_FID = 0x16,
    UNI_EVENT_GET_TXCMD_DBG_SW_FID_TXD = 0x17,
    UNI_EVENT_TXCMD_CTRL_MAX_NUM
} UNI_EVENT_TXCMD_CTRL_TAG_T;

/* Txcmd ctrl Parameters (Tag) */
typedef struct GNU_PACKED _UNI_EVENT_GET_TXCMD_DBG_CMD_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    UINT8  ucUserIndex;
    UINT8  ucDlUlidx;

    UINT8  aucBuffer[0];
} UNI_EVENT_GET_TXCMD_DBG_CMD_CTRL_T, *P_UNI_EVENT_GET_TXCMD_DBG_CMD_CTRL_T;

struct GNU_PACKED UNI_EVENT_BAND_CONFIG_T {
	/* fixed field */
	UINT8 ucBand;
	UINT8 aucPadding[3];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

/* EDCCA CTRL event Tag */
enum UNI_EVENT_BAND_CONFIG_TAG_T {
	UNI_EVENT_BAND_CONFIG_EDCCA_ENABLE = 0,
	UNI_EVENT_BAND_CONFIG_EDCCA_THRESHOLD = 1,
	UNI_EVENT_BAND_CONFIG_ACK_CTS_TIMEOUT = 7,
	UNI_EVENT_BAND_CONFIG_DUMP_STA_PAUSE = 8,
	UNI_EVENT_BAND_CONFIG_NUM
};

/* EDCCA OnOff Control (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_EDCCA_ENABLE_T {
    UINT16 u2Tag;    /* Tag = 0x00 */
    UINT16 u2Length;
    UINT8 fgEDCCAEnable;
    UINT8 aucPadding[3];
} UNI_EVENT_EDCCA_ENABLE_T, *P_UNI_EVENT_EDCCA_ENABLE_T;

/* EDCCA Threshold Control (Tag1) */
typedef struct GNU_PACKED _UNI_EVENT_EDCCA_THRESHOLD_T {
	UINT16 u2Tag;    /* Tag = 0x01 */
	UINT16 u2Length;
	UINT8 u1EDCCAThreshold[4];
	UINT8 fginit;
} UNI_EVENT_EDCCA_THRESHOLD_T, *P_UNI_EVENT_EDCCA_THRESHOLD_T;

struct GNU_PACKED UNI_EVENT_ACK_CTS_TIMEOUT_T {
	UINT16 u2Tag;    // Tag = 0x07
	UINT16 u2Length;
	UINT8 u1Type;
	UINT8 aucReserved[3];
	UINT32 u4TimeoutValue;
};

struct GNU_PACKED _UNI_EVENT_DUMP_STA_PAUSE_T {
	UINT16 u2Tag;    // Tag = 0x08
	UINT16 u2Length;
	UINT16 u2StaPauseTotBytes;
	UINT16 u2StaAcBytes;
	UINT16 u2StaTwtMapBytes;
	UINT8 aucReserved[2];
	UINT8 aucStaPauseContent[0];
};

/* Get MIB event Tag */
typedef enum _UNI_EVENT_MIB_TAG_T {
    UNI_EVENT_MIB_DATA = 0,
    UNI_EVENT_MIB_MAX_NUM
} UNI_EVENT_MIB_TAG_T;
typedef struct GNU_PACKED _UNI_EVENT_MIB_T {
    /* fixed field */
    UINT8 ucBand;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_MIB_T, *P_UNI_EVENT_MIB_T;

typedef struct GNU_PACKED _UNI_EVENT_MIB_DATA_T {
    UINT16 u2Tag;       /* should be 0x00 */
    UINT16 u2Length;    /* the length of this TLV */
    UINT32 u4Counter;   /* MIB ID of demanded MIB counter */
    UINT64 u8Data;      /* cumulated MIB counter for corresponded MIB ID */
} UNI_EVENT_MIB_DATA_T, *P_UNI_EVENT_MIB_DATA_T;

/* ============== UNI_EVENT_ID_STATISTICS Start ============ */
struct _UNI_EVENT_ID_STATISTICS_T {
	/* fixed field */
	UINT8 au1Reserved[4];
	/* tlv */
	UINT8 au1TlvBuffer[0];
};

struct _UNI_EVENT_STATISTICS_TLV_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT8  au1TagBuffer[0];
};

enum _UNI_EVENT_ID_STATISTICS_TAG_T {
	UNI_EVENT_ID_STATISTICS_BASIC = 0,
	UNI_EVENT_ID_STATISTICS_LINK_QUALITY = 1,
	UNI_EVENT_ID_STATISTICS_STA = 2,
	UNI_EVENT_ID_STATISTICS_BUG_REPORT = 3,
	UNI_EVENT_STATISTICS_MAX_NUM
};

/* ============== UNI_EVENT_ID_STATISTICS End ============ */

#ifdef SMART_CARRIER_SENSE_SUPPORT
typedef struct GNU_PACKED _UNI_EVENT_SCS {
    /* fixed field */
    UINT8 u1Bandidx;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_SCS_T, *P_UNI_EVENT_SCS_T;

/** SCS EVENT TLV List */
typedef enum _UNI_EVENT_SCS_TAG_T {
    UNI_EVENT_SCS_GET_GLO_ADDR = 0,
    UNI_EVENT_SCS_MAX_NUM
} UNI_EVENT_SCS_TAG_T;

typedef struct GNU_PACKED _UNI_EVENT_GET_SCS_GLO_ADDR {
    UINT16 u2Tag;
    UINT16 u2Length;

    UINT32 u4Index;
    EVENT_SCS_GLO rGloInfo;
} UNI_EVENT_GET_SCS_GLO_ADDR, *P_UNI_EVENT_GET_SCS_GLO_ADDR;
#endif /* SMART_CARRIER_SENSE_SUPPORT */

/* TPC event Tag */
typedef enum _UNI_EVENT_ID_TPC_TAG_T {
    UNI_EVENT_TPC_DOWNLINK_TABLE = 0,
    UNI_EVENT_TPC_UPLINK_TABLE = 1,
    UNI_EVENT_TPC_MAX_NUM
} UNI_EVENT_ID_TPC_TAG_T;

typedef struct GNU_PACKED _UNI_EVENT_TPC_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_TPC_T, *P_UNI_EVENT_TPC_T;

struct _UNI_EVENT_TPC_TLV_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	UINT_8  aucBuffer[0];
};

typedef struct GNU_PACKED _UNI_TPC_DL_MAN_MODE_PARAM_ELEMENT {
    UINT16 u2WlanId;
    UINT8 u1Reserved[2];
    INT16 i2DlTxPwrAlpha[UNI_TPC_DL_TX_TYPE_NUM];
} UNI_TPC_DL_MAN_MODE_PARAM_ELEMENT, *P_UNI_TPC_DL_MAN_MODE_PARAM_ELEMENT;

#define UNI_TPC_SUPPORT_STA_NUM         16
#define UNI_EVENT_RBIST_DUMP_DATA_SIZE  256

/* TPC DL info (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_TPC_INFO_DOWNLINK_TABLE_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCategory;
	UINT8 au1Reserved[3];
    /*AP Info*/
    BOOLEAN fgCmdPwrCtrl[UNI_TPC_DL_TX_TYPE_NUM];
    INT8 i1DlTxPwr[UNI_TPC_DL_TX_TYPE_NUM];
    UNI_TPC_DL_MAN_MODE_PARAM_ELEMENT rTpcDlManModeParamElem[UNI_TPC_SUPPORT_STA_NUM];
} UNI_EVENT_TPC_INFO_DOWNLINK_TABLE_T, *P_UNI_EVENT_TPC_INFO_DOWNLINK_TABLE_T;

typedef struct GNU_PACKED _UNI_TPC_UL_STA_COMM_INFO {
    UINT8 u1TargetRssi;
    UINT8 u1PwrHeadRoom;
    BOOLEAN fgMinPwr;
	UINT_8  u1Reserved;
} UNI_TPC_UL_STA_COMM_INFO, *P_UNI_TPC_UL_STA_COMM_INFO;

typedef struct GNU_PACKED _UNI_TPC_UL_MAN_MODE_PARAM_ELEMENT {
    UINT16 u2WlanId;
	UINT_8  au1Reserved[2];
    UNI_TPC_UL_STA_COMM_INFO rTpcUlStaCmmInfo;
} UNI_TPC_UL_MAN_MODE_PARAM_ELEMENT, *P_UNI_TPC_UL_MAN_MODE_PARAM_ELEMENT;

/* TPC UL info (Tag1) */
typedef struct GNU_PACKED _UNI_EVENT_TPC_INFO_UPLINK_TABLE_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8  u1TpcCategory;

    /*AP Info*/
    UINT8  u1ApTxPwr;
    UINT8  au1Reserved[2];

    UNI_TPC_UL_MAN_MODE_PARAM_ELEMENT rTpcUlManModeParamElem[UNI_TPC_SUPPORT_STA_NUM];
} UNI_EVENT_TPC_INFO_UPLINK_TABLE_T, *P_UNI_EVENT_TPC_INFO_UPLINK_TABLE_T;

/* ============== UNI_EVENT_ID_TWT_SYNC Begin ============== */
struct GNU_PACKED UNI_EVENT_TWT_SYNC_T {
	/*fixed field*/
	UINT8 aucPadding[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

/* TWT Sync event Tag */
enum UNI_EVENT_TWT_SYNC_TAG_T {
	UNI_EVENT_CLIENT_TWT_INFO = 0,
	UNI_EVENT_TWT_ACTION_TXS_INFO = 1,
};

/* TWT SYNC (Tag0) */
struct GNU_PACKED UNI_EVENT_CLIENT_TWT_INFO_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2WlanIdx;
	UINT8 ucBssIndex;
	UINT8 ucFlowId;
	UINT8 ucIdle;
	UINT8 aucReserved[3];
};

/* TWT Action TXS (Tag1) */
struct GNU_PACKED UNI_EVENT_TWT_ACTION_TXS_INFO_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2WlanIdx;
	UINT8 ucBssIndex;
	UINT8 ucToken;
	UINT8 ucMloEn;
	UINT8 ucActionLinkid;
	UINT16 u2AllLinkidBitmap;
	UINT16 u2MldStaIndx;
	UINT8 ucStatus;
	UINT8 Reserved;
};
/* ============== UNI_EVENT_ID_TWT_SYNC End ============== */

/* ============== UNI_EVENT_ID_RRO Begin ================= */
/* RRO event (0x57) */
struct GNU_PACKED UNI_EVENT_RRO_T {
	/*fixed field*/
	UINT8 aucPadding[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

/* UNI_EVENT_RRO_BA_SESSION_STATUS (0x00) */
struct GNU_PACKED UNI_EVENT_RRO_BA_SESSION_STATUS_T
{
	UINT16 u2Tag; /* Tag = 0x00 */
	UINT16 u2Length;
	UINT16 u2WlanIdx;
	UINT8 ucTid;
	UINT8 ucReserved;
	UINT32 u4Status;
	UINT16 u2SessionId;
	UINT8 aucPadding[2];
};

/* UNI_EVENT_RRO_BA_SESSION_DEL_CHECK_DONE (0x02)*/
struct GNU_PACKED UNI_EVENT_RRO_BA_SESSION_DEL_CHECK_DONE_T
{
	UINT16 u2Tag; /* Tag = 0x0x2 */
	UINT16 u2Length;
	UINT16 u2Seid;
	UINT16 u2MldId;
	UINT8  ucTid;
	UINT8  aucReserved[3];
};

enum UNI_EVENT_RRO_BA_SESSION_STATUS_TAG {
	UNI_EVENT_RRO_BA_SESSION_STATUS = 0,
	UNI_EVENT_RRO_BA_SESSION_TBL    = 1,
	UNI_EVENT_RRO_BA_SESSION_DEL_CHK_DONE = 2,
	UNI_EVENT_RRO_BA_SESSION_MAX_NUM
};

enum ENUM_RRO_STATUS_T /* RRO Status*/
{
	RRO_STATUS_SUCCESS = 0,
	RRO_STATUS_FAILURE = 1, /* wtbl.bypass_rro = 0 */
	RRO_STATUS_BY_PASS = 2, /* wtbl.bypass_rro = 1 */
	RRO_STATUS_SKIP_ADD = 3
};

/* ============== UNI_EVENT_ID_RRO End =================== */

/* ============== UNI_EVENT_ID_BSS_ER Start =================== */
struct GNU_PACKED UNI_EVENT_ID_BSS_ER_T
{
	/* fixed field */
	UINT8 aucPadding[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

/* BSS ER event tags */
enum UNI_EVENT_ID_BSS_ER_TAG_T {
	UNI_EVENT_BSS_ER_TX_MODE  = 0,
	UNI_EVENT_MLR_FSM_UPDATE = 1, /* MLR update */
	UNI_EVENT_ID_BSS_ER_MAX_NUM
};

/**
 * This structure is used for UNI_EVENT_MLR_FSM_UPDATE tag(0x01) of UNI_EVENT_ID_BSS_ER event (0x53)
 * The event is for operating MLR request from firmware
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag                Should be 0x01
 * @param[in] u2Length             The length of this TLV
 * @param[in] u2WlanIdx            wlan index
 * @param[in] ucMlrMode            MLR support mode (bitmap)
 * @param[in] ucMlrState           MLR state :MLR_STATE_IDLE (0x0), MLR_STATE_START (0x1)
 */
struct GNU_PACKED UNI_EVENT_MLR_FSM_UPDATE_T
{
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2WlanIdx;
	UINT8 ucMlrMode;
	UINT8 ucMlrState;
	UINT8 ucMlrTxdFrIdx;
	UINT8 aucReserved[3];
};
/* ============== UNI_EVENT_ID_BSS_ER End =================== */

/* ============== UNI_EVENT_ID_EEPROM_CONTROL Begin ============== */

struct GNU_PACKED UNI_EVENT_EEPROM_CONTROL {
	/* fixed field */
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

enum UNI_EVENT_ID_EEPROM_TAG {
	UNI_EVENT_EEPROM_ACCESS = 0,
	UNI_EVENT_EEPROM_MAX_NUM
};

struct GNU_PACKED UNI_EVENT_EEPROM_ACCESS_T {
	UINT16  u2Tag;
	UINT16  u2Length;
	UINT32  u4EventVer;
	UINT32  u4Address;
	UINT32  u4Valid;
	UINT32  u4Size;
	UINT32  u4MagicNum;
	UINT32  u4Type;        /* Reserved */
	UINT32  u4Reserved[4]; /* Reserved */
	UINT32 u4DataLen;
	UINT8  aucData[1024];
};

/* ============== UNI_EVENT_ID_EEPROM_CONTROL End ============== */

/* ============== UNI_EVENT_ID_EFUSE_CONTROL Begin ============== */

struct GNU_PACKED UNI_EVENT_EFUSE_CONTROL {
	/* fixed field */
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

enum UNI_EVENT_ID_EFUSE_TAG {
	UNI_EVENT_EFUSE_BUFFER_MODE_READ = 0,
	UNI_EVENT_EFUSE_FREE_BLOCK = 1,
	UNI_EVENT_EFUSE_ACCESS = 2,
	UNI_EVENT_EFUSE_MAX_NUM
};

struct GNU_PACKED UNI_EVENT_EFUSE_BUFFER_MODE_READ_T {
	UINT16  u2Tag;
	UINT16  u2Length;
	UINT8   u1SourceMode;       /* 0: eFuse mode; 1: Buffer mode */
	UINT8   u1ContentFormat;    /* 0: Bin Content; 1: Whole Content; 2: Multiple Sections */
	UINT16  u2Offset;           /* Read Offset */
	UINT16  u2Count;            /* Read Total Counts */
	UINT8   au1Reserved[2];     /* Read Total Counts */
	UINT8   BinContent[];       /* The content of read */
};

struct GNU_PACKED UNI_EVENT_EFUSE_FREE_BLOCK_T {
	UINT16  u2Tag;
	UINT16  u2Length;
	UINT8   ucGetFreeBlock;
	UINT8   ucVersion;        /* 0: original format ; 1: modified format */
	UINT8   ucTotalBlockNum;  /* Total Block */
	UINT8   uceFuseNum;
};

struct GNU_PACKED UNI_EVENT_EFUSE_ACCESS_T {
	UINT16  u2Tag;
	UINT16  u2Length;
	UINT32  u4EventVer;
	UINT32  u4Address;
	UINT32  u4Valid;
	UINT32  u4Size;
	UINT32  u4MagicNum;
	UINT32  u4Type;        /* Reserved */
	UINT32  u4Reserved[4]; /* Reserved */
	UINT8  aucData[32];
};

/* ============== UNI_EVENT_ID_EFUSE_CONTROL End ============== */

/* ============== UNI_EVENT_TXPOWER Begin ============== */
struct GNU_PACKED UNI_EVENT_TXPOWER {
	/* fixed field */
	UINT8 aucPadding[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

enum UNI_EVENT_ID_TXPOWER_TAG {
	UNI_EVENT_TXPOWER_EVENT_SHOW_INFO = 0x0,
	UNI_EVENT_TXPOWER_EVENT_UPDATE_COMPENSATE_TABLE = 0x1,
	UNI_EVENT_TXPOWER_EVENT_UPDATE_EPA_STATUS = 0x2,
	UNI_EVENT_TXPOWER_EVENT_POWER_BACKUP_TABLE_SHOW_INFO = 0x3,
	UNI_EVENT_TXPOWER_EVENT_TARGET_POWER_INFO_GET = 0x4,
	UNI_EVENT_TXPOWER_EVENT_SHOW_ALL_RATE_TXPOWER_INFO = 0x5,
	UNI_EVENT_TXPOWER_EVENT_THERMAL_COMPENSATE_TABLE_SHOW_INFO = 0x6,
	UNI_EVENT_TXPOWER_EVENT_TXV_BBP_POWER_SHOW_INFO = 0x7,
	UNI_EVENT_TXPOWER_EVENT_NUM
};

struct GNU_PACKED _UNI_EVENT_TXPOWER_RSP_T
{
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 aucBuffer[0];
};

/* ============== UNI_EVENT_TXPOWER End ============== */

/* ============== UNI_EVENT_ID_SPECTRUM Begin ============== */
struct GNU_PACKED UNI_EVENT_SPECTRUM {
	/* fixed field */
	UINT8 au1Reserved[4];

	/* tlv */
	UINT8 au1TlvBuffer[0];
};

enum UNI_EVENT_SPECTRUM_TAG {
	UNI_EVENT_ID_SPECTRUM_STATUS = 0,
	UNI_EVENT_ID_SPECTRUM_DATA = 1,
	UNI_EVENT_ID_PHY_ICS_DUMP_DATA = 2,
	UNI_EVENT_ID_SPECTRUM_MAX_NUM
};

struct GNU_PACKED UNI_EVENT_ID_SPECTRUM_STATUS_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT32 u4FuncIndex;
	UINT32 u4CapDone;
	UINT32 u4Reserved[15];
};

struct GNU_PACKED UNI_EVENT_ID_SPECTRUM_DATA_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT32 u4FuncIndex;
	UINT32 u4PktNum;
	UINT32 u4Bank;
	UINT32 u4DataLen;
	UINT32 u4WFCnt;
	UINT32 u4SmplCnt;
	UINT32 u4Reserved[6];
	INT32  u4Data[UNI_EVENT_RBIST_DUMP_DATA_SIZE];
};

struct GNU_PACKED UNI_EVENT_ID_PHY_ICS_DUMP_DATA_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT32 u4FuncIndex; /* 0x15 = 21 */
	UINT32 u4PktNum;
	UINT32 u4WifiSysTimestamp;
	UINT32 u4DataLen;
	UINT32 u4Reserved[5];
	UINT32 u4Data[UNI_EVENT_RBIST_DUMP_DATA_SIZE];
};

/* ============== UNI_EVENT_ID_SPECTRUM End ============== */

/* ============== UNI_EVENT_ID_MURU Start ============ */
struct _UNI_EVENT_ID_MURU_T {
	/* fixed field */
	UINT8 au1Reserved[4];
	/* tlv */
	UINT8 au1TlvBuffer[0];
};

struct _UNI_EVENT_MURU_TLV_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  aucBuffer[0];
};

/* ============== UNI_EVENT_ID_MURU End ============== */

#ifdef MT_DFS_SUPPORT
/* ========== UNI_EVENT_ID_RDD_IPI_HIST_CTRL Begin ============ */
/* RDD event (0x11) */
typedef struct _UNI_EVENT_RDD_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
    *
    *   TAG                          | ID  | structure
    *   -------------                | ----| -------------
    *   UNI_EVENT_RDD_SEND_PULSE     | 0x0 | UNI_EVENT_RDD_SEND_PULSE_T
    *   UNI_EVENT_RDD_REPORT         | 0x1 | UNI_EVENT_RDD_REPORT_T
    */
} UNI_EVENT_RDD_T, *P_UNI_EVENT_RDD_T;

typedef enum _UNI_EVENT_RDD_TAG_T {
    UNI_EVENT_RDD_SEND_PULSE = 0x0,
    UNI_EVENT_RDD_TAG_REPORT = 0x1,
    UNI_EVENT_RDD_MAX_NUM
} UNI_EVENT_RDD_TAG_T;

typedef struct GNU_PACKED _UNI_LONG_PULSE_BUFFER_T {
    UINT32 lng_strt_time;       /* Long pulse start time */
    UINT16 lng_pls_wdth;        /* Long pulse width */
    INT16 lng_pls_pwr;          /* Long pulse power */
    UINT8 lng_mdrdy_flg;        /* Long pulse MDRDY flag, bit1: mdray_early_flag, bit0: mdrdy_late_flag */
    UINT8 reserved[3];          /* Reserved bytes */
} UNI_LONG_PULSE_BUFFER_T, *P_UNI_LONG_PULSE_BUFFER_T;

typedef struct GNU_PACKED _UNI_PERIODIC_PULSE_BUFFER_T {
    UINT32 prd_strt_time;       /* Periodic pulse start time */
    UINT16 prd_pls_wdth;        /* Periodic pulse width */
    INT16 prd_pls_pwr;          /* Periodic pulse power */
    UINT8 prd_mdrdy_flg;        /* Periodic pulse MDRDY flag, bit1: mdray_early_flag, bit0: mdrdy_late_flag */
    UINT8 reserved[3];          /* Reserved bytes */
} UNI_PERIODIC_PULSE_BUFFER_T, *P_UNI_PERIODIC_PULSE_BUFFER_T;

typedef struct GNU_PACKED _UNI_WH_RDD_PULSE_CONTENT_T {
    UINT32 hw_start_time;
    UINT16 hw_pls_width;
    INT16 hw_pls_pwr;
    BOOLEAN hw_sc_pass;
    BOOLEAN hw_sw_reset;
    UINT8 hw_mdrdy_flag; /* bit1: mdray_early_flag, bit0: mdrdy_late_flag */
    UINT8 hw_tx_active;  /* bit1: tx_early_flag, bit0: tx_late_flag */
} UNI_WH_RDD_PULSE_CONTENT_T, *P_UNI_WH_RDD_PULSE_CONTENT_T;
/* Beacon timeout reason (Tag0) */
typedef struct _UNI_EVENT_RDD_SEND_PULSE_T {
    UINT16   u2Tag;    // Tag = 0x00
    UINT16   u2Length;
    UINT8 rdd_idx;
    UINT8 lng_pls_detected;
    UINT8 cr_pls_detected;
    UINT8 stgr_pls_detected;
    UINT8 rt_idx;      /* radar type index */
    UINT8 prd_pls_num; /* period pulse num */
    UINT8 lng_pls_num; /* long pulse num */
    UINT8 hw_pls_num;
    UINT8 out_lpn;     /* Long Pulse Number */
    UINT8 out_spn;     /* Short Pulse Number */
    UINT8 out_crpn;    /* Constant PRF Radar: Pulse Number */
    UINT8 out_crpw;    /* Constant PRF Radar: Pulse width */
    UINT8 out_crbn;   /* Constant PRF Radar: Burst Number */
    UINT8 out_stg_pn;  /* Staggered PRF radar: Staggered pulse number */
    UINT8 out_stg_pw;  /* Staggered PRF radar: maximum pulse width */
    UINT8 reserved;
    UINT32 out_pri_const;
    UINT32 out_pri_stg1;
    UINT32 out_pri_stg2;
    UINT32 out_pri_stg3;
    UINT32 out_pri_stg_dmin; /* Staggered PRF radar: min PRI Difference between 1st and 2nd  */
    UNI_LONG_PULSE_BUFFER_T lng_pls_buff[LPB_SIZE];
    UNI_PERIODIC_PULSE_BUFFER_T prd_pls_buff[PPB_SIZE];
    UNI_WH_RDD_PULSE_CONTENT_T hw_pls_buff[MAX_HW_PB_SIZE];
} UNI_EVENT_RDD_SEND_PULSE_T, *P_UNI_EVENT_RDD_SEND_PULSE_T;

typedef struct GNU_PACKED UNI_EVENT_RDD_TAG_REPORT_T {
    UINT16   u2Tag;    // Tag = 0x00
    UINT16   u2Length;
    UINT32 u4FuncIndex;
    UINT32 u4FuncLength;
    UINT32 u4Prefix;
    UINT32 u4Count;
    UINT8 ucRddIdx;
    UINT8 aucReserve[3];
    UINT8 aucBuffer[0];
} UNI_EVENT_RDD_TAG_REPORT_T, *P_UNI_EVENT_RDD_TAG_REPORT_T;

/* ========== UNI_EVENT_ID_RDD_IPI_HIST_CTRL End ============ */
#endif /* MT_DFS_SUPPORT */

/* =================== UNI_CMD_ID_VOW Begin ================== */
/* VOW event (0x37) */
typedef struct _UNI_EVENT_VOW_T {
	/* fixed field */
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0]; /**<the TLVs includer in this field:
		*
		*  TAG                                         | ID    | structure
		*  ------------------------------------        | ----- | -------------
		*  UNI_EVENT_VOW_DRR_CTRL                      |  0x0  | UNI_EVENT_VOW_DRR_CTRL_T
		*  UNI_EVENT_VOW_FEATURE_CTRL                  |  0x1  | UNI_EVENT_VOW_FEATURE_CTRL_T
		*  UNI_EVENT_VOW_BSSGROUP_CTRL_1_GROUP         |  0x2  | UNI_EVENT_VOW_BSSGROUP_CTRL_1_GROUP_T
		*  UNI_EVENT_VOW_BSSGROUP_TOKEN_CFG            |  0x3  | UNI_EVENT_VOW_BSSGROUP_TOKEN_CFG_T
		*  UNI_EVENT_VOW_BSSGROUP_CTRL_ALL_GROUP       |  0x4  | UNI_EVENT_VOW_BSSGROUP_CTRL_ALL_GROUP_T
		*  UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM     |  0x5  | UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_T
		*  UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL |  0x6  | UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_T
		*  UNI_EVENT_VOW_AT_PROC_EST_FEATURE           |  0x7  | UNI_EVENT_VOW_AT_PROC_EST_FEATURE_T
		*  UNI_EVENT_VOW_AT_PROC_EST_MONITOR_PERIOD    |  0x8  | UNI_EVENT_VOW_AT_PROC_EST_MONITOR_PERIOD_T
		*  UNI_EVENT_VOW_AT_PROC_EST_GROUP_RATIO       |  0x9  | UNI_EVENT_VOW_AT_PROC_EST_GROUP_RATIO_T
		*  UNI_EVENT_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING  | 0xA  | UNI_EVENT_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T
		*  UNI_EVENT_VOW_RX_AT_AIRTIME_EN              |  0xB  | UNI_EVENT_VOW_RX_AT_AIRTIME_EN_T
		*  UNI_EVENT_VOW_RX_AT_STA_WMM_CTRL            |  0xF  | UNI_EVENT_VOW_RX_AT_STA_WMM_CTRL_T
		*  UNI_EVENT_VOW_RX_AT_MBSS_WMM_CTRL           | 0x10  | UNI_EVENT_VOW_RX_AT_MBSS_WMM_CTRL_T
		*  UNI_EVENT_VOW_RX_AT_ED_OFFSET               | 0x11  | UNI_EVENT_VOW_RX_AT_ED_OFFSET_T
		*  UNI_EVENT_VOW_RX_AT_SW_TIMER                | 0x12  | UNI_EVENT_VOW_RX_AT_SW_TIMER_T
		*  UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER           | 0x13  | UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T
		*  UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME  | 0x14  | UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T
		*  UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME     | 0x15  | UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T
		*  UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME    | 0x16  | UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T
		*  UNI_EVENT_VOW_RX_AT_REPORT_PER_STA_RX_TIME  | 0x17  | UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T
		*/
} UNI_EVENT_VOW_T, *P_UNI_EVENT_VOW_T;

typedef enum _UNI_EVENT_VOW_TAG_T {
	UNI_EVENT_VOW_DRR_CTRL = 0x00,
	UNI_EVENT_VOW_FEATURE_CTRL = 0x01,
	UNI_EVENT_VOW_BSSGROUP_CTRL_1_GROUP = 0x02,
	UNI_EVENT_VOW_BSSGROUP_TOKEN_CFG = 0x03,
	UNI_EVENT_VOW_BSSGROUP_CTRL_ALL_GROUP = 0x04,
	UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM = 0x05,
	UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL = 0x06,
	UNI_EVENT_VOW_AT_PROC_EST_FEATURE = 0x07,
	UNI_EVENT_VOW_AT_PROC_EST_MONITOR_PERIOD = 0x08,
	UNI_EVENT_VOW_AT_PROC_EST_GROUP_RATIO = 0x09,
	UNI_EVENT_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING = 0x0A,
	UNI_EVENT_VOW_RX_AT_AIRTIME_EN = 0x0B,
	UNI_EVENT_VOW_RX_AT_STA_WMM_CTRL = 0x0F,
	UNI_EVENT_VOW_RX_AT_MBSS_WMM_CTRL = 0x10,
	UNI_EVENT_VOW_RX_AT_ED_OFFSET = 0x11,
	UNI_EVENT_VOW_RX_AT_SW_TIMER = 0x12,
	UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER = 0x13,
	UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME = 0x14,
	UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME = 0x15,
	UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME = 0x16,
	UNI_EVENT_VOW_RX_AT_REPORT_PER_STA_RX_TIME = 0x17,
} UNI_EVENT_VOW_TAG_T;

/* TAG: UNI_EVENT_VOW_DRR_CTRL (0x00) */
typedef struct _UNI_EVENT_VOW_DRR_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2StaID;

	UINT_8 u1BandIdx;
	UINT_8 u1WmmSet;

	UINT32 u4CtrlFieldID;

	UNI_VOW_DRR_CTRL_VALUE_T rAirTimeCtrlValue;

	UINT_8 u1Reserved[3];
	UINT_8 u1OMACIdx;

} UNI_EVENT_VOW_DRR_CTRL_T, *P_UNI_EVENT_VOW_DRR_CTRL_T;

/* TAG: UNI_EVENT_VOW_FEATURE_CTRL (0x01) */
typedef struct _UNI_CMD_VOW_FEATURE_CTRL_T    UNI_EVENT_VOW_FEATURE_CTRL_T, *P_UNI_EVENT_VOW_FEATURE_CTRL_T;

/* TAG: UNI_EVENT_VOW_BSSGROUP_CTRL_1_GROUP (0x01) */
typedef struct _UNI_EVENT_VOW_BSSGROUP_CTRL_1_GROUP_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucBssGroupID;
	UINT8 u1BandIdx;
	UINT8 aucPadding[2];
	UNI_CMD_BW_BSS_TOKEN_SETTING_T rAllBssGroupMultiField;
} UNI_EVENT_VOW_BSSGROUP_CTRL_1_GROUP_T, *P_UNI_EVENT_VOW_BSSGROUP_CTRL_1_GROUP_T;

/* TAG: UNI_EVENT_VOW_BSSGROUP_TOKEN_CFG (0x03) */
typedef struct _UNI_EVENT_VOW_BSSGROUP_TOKEN_CFG_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucBssGroupID;
	UINT8 u1BandIdx;
	UINT8 aucPadding[2];
	UINT32 u4SingleFieldIDValue;
	UINT32 u4CfgItemId;
} UNI_EVENT_VOW_BSSGROUP_TOKEN_CFG_T, *P_UNI_EVENT_VOW_BSSGROUP_TOKEN_CFG_T;

/* TAG: UNI_EVENT_VOW_BSSGROUP_CTRL_ALL_GROUP (0x04) */
typedef struct _UNI_EVENT_VOW_BSSGROUP_CTRL_ALL_GROUP_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 u1BandIdx;
	UINT8 u1Reserved[3];

	UNI_CMD_BW_BSS_TOKEN_SETTING_T arAllBssGroupMultiField[UNICMD_VOW_BWC_GROUP_NUMBER];
} UNI_EVENT_VOW_BSSGROUP_CTRL_ALL_GROUP_T, *P_UNI_EVENT_VOW_BSSGROUP_CTRL_ALL_GROUP_T;

/* TAG: UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM (0x05) */
typedef struct _UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucBssGroupQuantumID;
	UINT8 ucBssGroupQuantumTime;
	UINT8 aucPadding[2];
} UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_T, *P_UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_T;

/* TAG: UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL (0x06) */
typedef struct _UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 aucBssGroupQuantumTime[UNICMD_VOW_BW_GROUP_QUANTUM_LEVEL_NUM];
} UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T, *P_UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T;

/* TAG: UNI_EVENT_VOW_AT_PROC_EST_FEATURE (0x07) */
typedef struct _UNI_EVENT_VOW_AT_PROC_EST_FEATURE_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	BOOLEAN fgAtEstimateOnOff;
	UINT8 aucPadding[3];
} UNI_EVENT_VOW_AT_PROC_EST_FEATURE_T, *P_UNI_EVENT_VOW_AT_PROC_EST_FEATURE_T;

/* TAG: UNI_EVENT_VOW_AT_PROC_EST_MONITOR_PERIOD (0x08) */
typedef struct _UNI_EVENT_VOW_AT_PROC_EST_MONITOR_PERIOD_T {
	UINT16         u2Tag;
	UINT16         u2Length;

	UINT16         u2AtEstMonitorPeriod;
	UINT8          aucPadding[2];
} UNI_EVENT_VOW_AT_PROC_EST_MONITOR_PERIOD_T, *P_UNI_EVENT_VOW_AT_PROC_EST_MONITOR_PERIOD_T;

/* TAG: UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO (0x09) */
typedef struct _UNI_EVENT_VOW_AT_PROC_EST_GROUP_RATIO_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2GroupMaxRatioValue[UNICMD_VOW_BWC_GROUP_NUMBER];
	UINT16 u2GroupMinRatioValue[UNICMD_VOW_BWC_GROUP_NUMBER];
	UINT8  u1BandIdx;
	UINT8  u1Reserved[3];

} UNI_EVENT_VOW_AT_PROC_EST_GROUP_RATIO_T, *P_UNI_EVENT_VOW_AT_PROC_EST_GROUP_RATIO_T;

/* TAG: UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING (0x0A) */
typedef struct _UNI_EVENT_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucGrouptoSelectBand;
	UINT8 ucBandSelectedfromGroup;
	UINT8 aucPadding[2];
} UNI_EVENT_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T, *P_UNI_EVENT_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T;

/* rx airtime airtime enable (Tag 0x0B) */
typedef struct _UNI_EVENT_VOW_RX_AT_AIRTIME_EN_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 fgRxAirTimeEn;
	UINT8 u1BandIdx;
	UINT8 aucPadding[2];
} UNI_EVENT_VOW_RX_AT_AIRTIME_EN_T, *P_UNI_EVENT_VOW_RX_AT_AIRTIME_EN_T;

/* rx airtime sta wmm ctrl (Tag 0x0F) */
typedef struct _UNI_EVENT_VOW_RX_AT_STA_WMM_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucOwnMacID;
	UINT8 fgtoApplyWm00to03MibCfg;
	UINT8 u1BandIdx;
	UINT8 aucPadding[1];
} UNI_EVENT_VOW_RX_AT_STA_WMM_CTRL_T, *P_UNI_EVENT_VOW_RX_AT_STA_WMM_CTRL_T;

/* rx airtime airtime clr enable (Tag 0x10) */
typedef struct _UNI_EVENT_VOW_RX_AT_MBSS_WMM_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucMbssGroup;
	UINT8 ucWmmGroup;
	UINT8 u1BandIdx;
	UINT8 aucPadding[1];
} UNI_EVENT_VOW_RX_AT_MBSS_WMM_CTRL_T, *P_UNI_EVENT_VOW_RX_AT_MBSS_WMM_CTRL_T;

/* rx airtime ed offset value (Tag 0x11) */
typedef struct _UNI_EVENT_VOW_RX_AT_ED_OFFSET_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucEdOffsetValue;
	UINT8 u1BandIdx;
	UINT8 aucPadding[2];
} UNI_EVENT_VOW_RX_AT_ED_OFFSET_T, *P_UNI_EVENT_VOW_RX_AT_ED_OFFSET_T;

/* rx airtime sw timer value (Tag 0x12) */
typedef struct _UNI_EVENT_VOW_RX_AT_SW_TIMER_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucCompensateMode;
	UINT8 ucRxBand;
	UINT8 ucSwCompensateTimeValue;
	UINT8 aucPadding[1];
} UNI_EVENT_VOW_RX_AT_SW_TIMER_T, *P_UNI_EVENT_VOW_RX_AT_SW_TIMER_T;

/* rx airtime sw timer value (Tag 0x13) */
typedef struct _UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2AC0Backoff;
	UINT16 u2AC1Backoff;
	UINT16 u2AC2Backoff;
	UINT16 u2AC3Backoff;
	UINT8 ucRxATBackoffWmmGroupIdx;
	UINT8 ucRxAtBackoffAcQMask;
	UINT8 u1BandIdx;
	UINT8 aucPadding[1];
} UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T, *P_UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T;

/* rx airtime report non wifi time (Tag 0x14) */
typedef struct _UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucRxNonWiFiBandIdx;
	UINT8 aucPadding[3];
	UINT32 u4RxNonWiFiBandTimer;
} UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T, *P_UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T;

/* rx airtime report rx obss time (Tag 0x15) */
typedef struct _UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucRxObssBandIdx;
	UINT8 aucPadding[3];
	UINT32 u4RxObssBandTimer;
} UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME_T, *P_UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME_T;

/* rx airtime report rx mib time (Tag 0x16) */
typedef struct _UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucRxMibObssBandIdx;
	UINT8 aucPadding[3];
	UINT32 u4RxMibObssBandTimer;
} UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T, *P_UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T;

/* rx airtime report rx mib time (Tag 0x17) */
typedef struct _UNI_EVENT_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    UINT16 u2StaId;
    UINT8 aucPadding[2];
    UINT32 au4StaAcRxTimer[4];
} UNI_EVENT_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T, *P_UNI_EVENT_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T;

typedef struct GNU_PACKED _UNI_EVENT_VOW_PARAM_T {
	/* Resp */
	UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T EventVowRxAtReportRxNonwifiTime; /* TAG 14 */
	UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME_T EventVowRxAtReportRxObssTime; /* TAG 15 */
	UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T EventVowRxAtReportMibObssTime; /* TAG 16 */
} UNI_EVENT_VOW_PARAM_T, *P_UNI_EVENT_VOW_PARAM_T;

/* =================== UNI_CMD_ID_VOW End ==================== */

/* ============== UNI_EVENT_ID_BA_OFFLOAD Begin ============== */
struct UNI_EVENT_ID_BA_OFFLOAD {
	/* fixed field */
	UINT8 aucPadding[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

enum UNI_EVENT_ID_BA_OFFLOAD_TAG {
	UNI_EVENT_ID_BA_OFFLOAD_TAG_RX_ADDBA  = 0,
	UNI_EVENT_ID_BA_OFFLOAD_TAG_RX_DELBA  = 1,
	UNI_EVENT_ID_BA_OFFLOAD_TAG_TX_ADDBA  = 2,
	UNI_EVENT_ID_BA_OFFLOAD_TAG_NUM
};

/**
 * This structure is used for UNI_EVENT_RX_ADDBA tag(0x00) of
 * UNI_EVENT_ID_BA_OFFLOAD event (0x2C)
 * This event is sent to the host when an RX BA agreement is established,
 * i.e., when a received adding BA request (ADDBA_REQ) is accepted.
 * @u2Tag: Tag id
 * @u2Length: The length of this TLV
 * @ucStaRecIdx: The station record index associated with the BA agreement
 * @ucDialogToken: The Dialog Token field in the ADDBA_REQ frame
 * @u2WinSize:
 * @u2BATimeoutValue: BlockAckTimeout in the ADDBA_REQ frame
 * @u2BAStartSeqCtrl: BlockAckStartingSequenceControl in the ADDBA_REQ frame
 * @ucTid: The TID value associated with the deleted BA agreement
 * @aucReserved: reserved
 */
struct UNI_EVENT_RX_ADDBA {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  ucStaRecIdx;
	UINT8  ucDialogToken;
	UINT16 u2WinSize;
	UINT16 u2BATimeoutValue;
	UINT16 u2BAStartSeqCtrl;
	UINT8 ucTid;
	UINT8 aucReserved[3];
};

/**
 * This structure is used for UNI_EVENT_RX_DELBA tag(0x01) of
 * UNI_EVENT_ID_BA_OFFLOAD event (0x2C)
 * This event is sent to the host when an RX BA agreement is deleted.
 *
 * @u2Tag: Tag id
 * @u2Length: The length of this TLV
 * @ucStaRecIdx: The station record index associated with the deleted BA
 * @ucTid: The TID value associated with the deleted BA agreement
 * @aucReserved: reserved
 */
struct UNI_EVENT_RX_DELBA {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  ucStaRecIdx;
	UINT8  ucTid;
	UINT8  aucReserved[2];
};

/**
 * This structure is used for UNI_EVENT_TX_ADDBA tag(0x02) of
 * UNI_EVENT_ID_BA_OFFLOAD event (0x2C)
 * This event will notify host for Tx ADDBA establish state and AMSDU in
 * AMPDU related information.
 *
 * @u2Tag: Tag id
 * @u2Length: The length of this TLV
 * @ucStaRecIdx:
 * @ucTid:
 * @u2WinSize:
 * @ucAmsduEnBitmap: AMSDU in AMPDU is enabled or not (TID bitmap)
 * @u2SSN: AMSDU count/length limits by count *OR* length
 *                 Count: MPDU count in an AMSDU shall not exceed ucMaxMpduCount
 *                 Length: AMSDU length shall not exceed u4MaxMpduLen
 * @ucMaxMpduCount: Max MPDU count in an AMSDU
 *
 * @u4MaxMpduLen: Max AMSDU length.
 *                Note: If length of a packet < u4MinMpduLen then it shall not
 *                be aggregated in an AMSDU
 * @u4MinMpduLen: Min MPDU length to be AMSDU
 * @aucReserved2[16]:
 */
struct UNI_EVENT_TX_ADDBA {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  ucStaRecIdx;
	UINT8  ucTid;
	UINT16 u2WinSize;
	UINT8  ucAmsduEnBitmap;
	UINT16 u2SSN;
	UINT8  ucMaxMpduCount;
	UINT32 u4MaxMpduLen;
	UINT32 u4MinMpduLen;
	UINT8  aucReserved2[16];
};

/* ============== UNI_EVENT_ID_BA_OFFLOAD End ============== */
/* ============== UNI_EVENT_ID_TESTMODE_RX_STAT_INFO Begin ============== */

struct GNU_PACKED _UNI_EVENT_TESTMODE_RX_STAT_T {
	/*fix field*/
	UINT_8 au1Reserved[4];

	/*tlv */
	UINT_8 aucTlvBuffer[0];/**< the TLVs included in this field:
	*
	*   TAG                               | ID  | structure
	*   ----------------------------------|-----|--------------
	*   UNI_EVENT_TESTMODE_RXV_REPORT     | 0x0 | UNI_EVENT_TESTMODE_RXV_REPORT_T
	*   UNI_EVENT_TESTMODE_RX_STAT        | 0x1 | UNI_EVENT_TESTMODE_RX_STAT_T
	*   UNI_EVENT_TESTMODE_RX_STAT_BAND   | 0x2 | UNI_EVENT_TESTMODE_RX_STAT_BAND_T
	*   UNI_EVENT_TESTMODE_RX_STAT_PATH   | 0x3 | UNI_EVENT_TESTMODE_RX_STAT_PATH_T
	*   UNI_EVENT_TESTMODE_RX_STAT_USER   | 0x4 | UNI_EVENT_TESTMODE_RX_STAT_USER_T
	*   UNI_EVENT_TESTMODE_RX_STAT_COMM   | 0x5 | UNI_EVENT_TESTMODE_RX_STAT_COMM_T
	*   UNI_EVENT_TESTMODE_RX_STAT_ALL    | 0x5 | UNI_EVENT_TESTMODE_RX_STAT_ALL_T
	*/
};

enum _UNI_EVENT_TESTMODE_RX_STAT_INFO_TAG_T {
	UNI_EVENT_TESTMODE_RXV_REPORT = 0,
	UNI_EVENT_TESTMODE_RX_STAT,
	UNI_EVENT_TESTMODE_RX_STAT_BAND,
	UNI_EVENT_TESTMODE_RX_STAT_PATH,
	UNI_EVENT_TESTMODE_RX_STAT_USER,
	UNI_EVENT_TESTMODE_RX_STAT_COMM,
	UNI_EVENT_TESTMODE_RX_STAT_ALL = 6, /* unused */
	UNI_EVENT_TESTMODE_RX_STAT_ALL_V2 = 7,

	UNI_EVENT_TESTMODE_RX_MAX_NUM
};

struct GNU_PACKED _UNI_EVENT_TESTMODE_RXV_REPORT_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 u1RxvFormatType;
	UINT_8 u1RxvCbEntryType;
	UINT_8 u1Reserved[2];

	/* rxv entry content */
	UINT_16 u2PostMd;
	UINT_16 u2PostRssiRu;

	UINT_8 u1RxCeLtfSnr;
	UINT_8 u1Reserved1[3];

	UINT_32 u4TftFoe;

	UINT_8 u1PostNoiseFloorRx0;
	UINT_8 u1PostNoiseFloorRx1;
	UINT_8 u1PostNoiseFloorRx2;
	UINT_8 u1PostNoiseFloorRx3;

	UINT_8 u1DecUserNum;
	UINT_8 u1UserRate;
	UINT_8 u1UserStreamNum;
	UINT_8 u1UserRuAlloc;

	UINT_16 u2MuAid;
	bool    fgRxFcsErr;
	UINT_8 u1Reserved2;

	UINT_32 u4OfdmRu26Snr0;
	UINT_32 u4OfdmRu26Snr1;
	UINT_32 u4OfdmRu26Snr2;
	UINT_32 u4OfdmRu26Snr3;
	UINT_32 u4OfdmRu26Snr4;
	UINT_32 u4OfdmRu26Snr5;
	UINT_32 u4OfdmRu26Snr6;
	UINT_32 u4OfdmRu26Snr7;
	UINT_32 u4OfdmRu26Snr8;
	UINT_32 u4OfdmRu26Snr9;
};

struct GNU_PACKED _UNI_TESTMODE_STATISTIC_INFO_BAND_T {
	/* mac part */
	UINT_16 u2MacRxFcsErrCnt;
	UINT_16 u2MacRxLenMisMatch;
	UINT_16 u2MacRxFcsOkCnt;
	UINT_8 u1Reserved1[2];
	UINT_32 u4MacRxMdrdyCnt;

	/* phy part */
	UINT_16 u2PhyRxFcsErrCntCck;
	UINT_16 u2PhyRxFcsErrCntOfdm;
	UINT_16 u2PhyRxPdCck;
	UINT_16 u2PhyRxPdOfdm;
	UINT_16 u2PhyRxSigErrCck;
	UINT_16 u2PhyRxSfdErrCck;
	UINT_16 u2PhyRxSigErrOfdm;
	UINT_16 u2PhyRxTagErrOfdm;
	UINT_16 u2PhyRxMdrdyCntCck;
	UINT_16 u2PhyRxMdrdyCntOfdm;
};

struct _UNI_TESTMODE_STATISTIC_INFO_BAND_EXT1_T {
	/* mac part */
	UINT_32 u4RxMpduU2MCnt;
	/* phy part */
	UINT_8 u1Reserved[4];
};

struct GNU_PACKED _UNI_TEST_RX_STAT_USER_INFO_T {
	INT_32 i4FreqOffsetFromRx;
	INT_32 i4Snr;
	UINT_32 u4FcsErrorCnt;
};

struct GNU_PACKED _UNI_TESTMODE_STATISTIC_INFO_COMM_T {
	UINT_16 u2MacRxFifoFull;
	UINT_8 u1Reserved1[2];

	UINT_32 u4AciHitLow;
	UINT_32 u4AciHitHigh;
};

struct GNU_PACKED _UNI_TESTMODE_STATISTIC_INFO_RXV_T {
	UINT_16 u2Rcpi;
	INT_16 i2Rssi;
	INT_16 i2Snr;
	INT_16 i2AdcRssi;
};

struct GNU_PACKED _UNI_TESTMODE_STATISTIC_INFO_RSSI_T {
	INT_8 i1RssiIb;
	INT_8 i1RssiWb;
	UINT_8 u1Reserved1[2];
};

struct GNU_PACKED _UNI_EVENT_TESTMODE_RX_STAT_ALL_T {
	UINT_32 mac_rx_fcs_err_cnt;
	UINT_32 mac_fcs_ok; //new
	UINT_32 mac_rx_mdrdy_cnt;
	UINT_32 phy_rx_fcs_err_cnt_cck;
	UINT_32 phy_rx_fcs_err_cnt_ofdm;
	UINT_32 phy_rx_pd_cck;
	UINT_32 phy_rx_pd_ofdm;
	UINT_32 phy_rx_sig_err_cck;
	UINT_32 phy_rx_sfd_err_cck;
	UINT_32 phy_rx_sig_err_ofdm;
	UINT_32 phy_rx_tag_err_ofdm;
	UINT_32 wb_rssi0;
	UINT_32 ib_rssi0;
	UINT_32 wb_rssi1;
	UINT_32 ib_rssi1;
	UINT_32 phy_rx_mdrdy_cnt_cck;
	UINT_32 phy_rx_mdrdy_cnt_ofdm;
	UINT_32 driver_rx_count;
	UINT_32 rcpi0;
	UINT_32 rcpi1;
	UINT_32 freq_offset_from_rx;
	UINT_32 rssi0;
	UINT_32 rssi1;
	UINT_32 rx_fifo_full;  /* out_of_resource */
	UINT_32 mac_rx_len_mismatch;
	UINT_32 mac_rx_fcs_err_cnt_band1;
	UINT_32 mac_fcs_ok_band1; //new
	UINT_32 mac_rx_mdrdy_cnt_band1;
	UINT_32 fagc_ib_RSSSI0;
	UINT_32 fagc_ib_RSSSI1;
	UINT_32 fagc_ib_RSSSI2;
	UINT_32 fagc_ib_RSSSI3;
	UINT_32 fagc_wb_RSSSI0;
	UINT_32 fagc_wb_RSSSI1;
	UINT_32 fagc_wb_RSSSI2;
	UINT_32 fagc_wb_RSSSI3;
	UINT_32 inst_ib_RSSSI0;
	UINT_32 inst_ib_RSSSI1;
	UINT_32 inst_ib_RSSSI2;
	UINT_32 inst_ib_RSSSI3;
	UINT_32 inst_wb_RSSSI0;
	UINT_32 inst_wb_RSSSI1;
	UINT_32 inst_wb_RSSSI2;
	UINT_32 inst_wb_RSSSI3;
	UINT_32 aci_hit_low;
	UINT_32 aci_hit_high;
	UINT_32 driver_rx_count1;
	UINT_32 rcpi2;
	UINT_32 rcpi3;
	UINT_32 rssi2;
	UINT_32 rssi3;
	UINT_32 snr0;
	UINT_32 snr1;
	UINT_32 snr2;
	UINT_32 snr3;
	UINT_32 rx_fifo_full_band1;
	UINT_32 mac_rx_len_mismatch_band1;
	UINT_32 phy_rx_pd_cck_band1;
	UINT_32 phy_rx_pd_ofdm_band1;
	UINT_32 phy_rx_sig_err_cck_band1;
	UINT_32 phy_rx_sfd_err_cck_band1;
	UINT_32 phy_rx_sig_err_ofdm_band1;
	UINT_32 phy_rx_tag_err_ofdm_band1;
	UINT_32 phy_rx_mdrdy_cnt_cck_band1;
	UINT_32 phy_rx_mdrdy_cnt_ofdm_band1;
	UINT_32 phy_rx_fcs_err_cnt_cck_band1;
	UINT_32 phy_rx_fcs_err_cnt_ofdm_band1;
	UINT_32 mu_pkt_count;
	UINT_32 sig_mcs;
	UINT_32 sinr;
	UINT_32 rxv_rssi;
	/* UINT_32 reserved[184]; */
	UINT_32 phy_mdrdy;
	UINT_32 noise_floor;
	UINT_32 all_len_mismatch_ch_cnt_band0;
	UINT_32 all_len_mismatch_ch_cnt_band1;
	UINT_32 all_mac_mdrdy0;
	UINT_32 all_mac_mdrdy1;
	UINT_32 all_fcs_err0;
	UINT_32 all_fcs_ok0;//new
	UINT_32 all_fcs_err1;
	UINT_32 all_fcs_ok1;//new
	UINT_32 rx_ok0;
	UINT_32 rx_ok1;
	UINT_32 per0;
	UINT_32 per1;
	/* total 76 param cut */
};

struct GNU_PACKED _UNI_EVENT_TESTMODE_STAT_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	struct _UNI_TESTMODE_STATISTIC_INFO_BAND_T rInfoBand;
	struct _UNI_TESTMODE_STATISTIC_INFO_COMM_T rInfoComm;

	/* rxv part */
	struct _UNI_TESTMODE_STATISTIC_INFO_RXV_T rInfoRXV[MAX_ANTENNA_NUM];

	/* RSSI */
	struct _UNI_TESTMODE_STATISTIC_INFO_RSSI_T rInfoFagc[MAX_ANTENNA_NUM];
	struct _UNI_TESTMODE_STATISTIC_INFO_RSSI_T rInfoInst[MAX_ANTENNA_NUM];
};

struct GNU_PACKED _UNI_EVENT_TESTMODE_STAT_BAND_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	struct _UNI_TESTMODE_STATISTIC_INFO_BAND_T rInfoBand;
};

struct GNU_PACKED _UNI_EVENT_TESTMODE_STAT_PATH_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	struct _UNI_TESTMODE_STATISTIC_INFO_RSSI_T rInfoInst;
};

struct GNU_PACKED _UNI_EVENT_TESTMODE_STAT_USER_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	INT_32 i4FreqOffsetFromRx;
	INT_32 i4Snr;
	UINT_32 u4FcsErrorCnt;
};

struct GNU_PACKED _UNI_EVENT_TESTMODE_STAT_COMM_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	struct _UNI_TESTMODE_STATISTIC_INFO_COMM_T rInfoComm;
};

struct GNU_PACKED _UNI_EVENT_TESTMODE_STAT_ALL_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	struct _UNI_EVENT_TESTMODE_RX_STAT_ALL_T rStatAll;
};

struct _UNI_TESTMODE_STATISTIC_INFO_USER_T {
	INT_32 i4FreqOffsetFromRx;
	INT_32 i4Snr;
	UINT_32 u4FcsErrorCnt;
};

struct _UNI_TESTMODE_STATISTIC_INFO_COMM_EXT1_T {
	UINT_32 u4DrvRxCnt;
	UINT_32 u4Sinr;
	UINT_32 u4MuRxCnt;
	/* mac part */
	UINT_8 u1Reserved0[4];
	/* phy part */
	UINT_8 u1EhtSigMcs;
	UINT_8 u1Reserved1[3];
};

struct _UNI_TESTMODE_STATISTIC_INFO_USER_EXT1_T {
	INT_8 u1NeVarDbAllUser;
	UINT_8 u1Reserved1[3];
};

/* this struct has to match with fw struct size */
struct GNU_PACKED _UNI_EVENT_TESTMODE_STAT_ALL_V2_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	struct _UNI_TESTMODE_STATISTIC_INFO_BAND_T rInfoBand;
	struct _UNI_TESTMODE_STATISTIC_INFO_BAND_EXT1_T rInfoBandExt1;
	struct _UNI_TESTMODE_STATISTIC_INFO_COMM_T rInfoComm;
	struct _UNI_TESTMODE_STATISTIC_INFO_COMM_EXT1_T rInfoCommExt1;

	/* rxv part */
	struct _UNI_TESTMODE_STATISTIC_INFO_RXV_T rInfoRXV[8];

	/* RSSI */
	struct _UNI_TESTMODE_STATISTIC_INFO_RSSI_T rInfoFagc[8];
	struct _UNI_TESTMODE_STATISTIC_INFO_RSSI_T rInfoInst[8];

	/* User */
	struct _UNI_TESTMODE_STATISTIC_INFO_USER_T rInfoUser[16];
	struct _UNI_TESTMODE_STATISTIC_INFO_USER_EXT1_T rInfoUserExt1[16];
};

struct GNU_PACKED _UNI_EVENT_TESTMODE_STAT_WB_T {
	struct _UNI_EVENT_TESTMODE_STAT_T rEvtTestmodeStat;
	struct _UNI_EVENT_TESTMODE_STAT_BAND_T rEvtTestmodeStatBand;
	struct _UNI_EVENT_TESTMODE_STAT_PATH_T rEvtTestmodeStatPath;
	struct _UNI_EVENT_TESTMODE_STAT_USER_T rEvtTestmodeStatUser;
	struct _UNI_EVENT_TESTMODE_STAT_COMM_T rEvtTestmodeStatComm;
	struct _UNI_EVENT_TESTMODE_STAT_ALL_V2_T rEvtTestmodeStatAll;
};

/* ============== UNI_EVENT_ID_TESTMODE_RX_STAT_INFO End ============== */
/* ============== UNI_EVENT_RA Begin ============== */
/* RA event (0x2F) */
struct GNU_PACKED UNI_EVENT_RA {
	/* fixed field */
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

/* RA event tags */
enum UNI_EVENT_RA_TAG {
	UNI_EVENT_RA_G_BAND_256QAM_PROBE_RESULT = 0,
	UNI_EVENT_RA_RU_RA_INFO = 1,
	UNI_EVENT_RA_MU_RA_INFO = 2,
	UNI_EVENT_RA_All_TX_FAIL_WCID_INFORM = 3,
	UNI_EVENT_RA_MAX_NUM
};

struct GNU_PACKED UNI_EVENT_RA_G_BAND_256QAM_PROBE_RESULT_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT8 u1WlanIdxL;
	UINT8 ucResult;
	UINT8 u1WlanIdxHnVer;
	UINT8 ucReserved;
};

#define EXE_IN_INET 0
/*42 bytes */
struct  GNU_PACKED _UNI_TxCMD_RATE_INFO_T {
	UINT16 u2SuggestTxModeRate;
	UINT16 u2UsrMinRate;  /* for dynamic sounding */
	UINT16 u2UsrMaxRate;  /* for dynamic sounding */
	UINT16 u2InitRateDownMCS;
	UINT16 u2CurrRate;  /* Get form TxCMD report. 4bytes for Nss, 4bytes for MCS */
	UINT16 initRateDownTotalCnt;
	UINT16 initRateDownOkCnt;
	UINT16 u2StSucceCnt;
	UINT16 u2StTotalTxCnt;  /* ST_TotalTxCnt */
	UINT16 u2RuPrevRate;
	UINT16 u2StartProbeUpMCS;
	UINT8  u1NoRateUpCnt; /* the PPDU conter to trigger quick fall back */
	UINT8  u1StTotalPpduCnt; /* for low traffic RA */
	UINT8  u1Gi;
	UINT8  u1RuTryupFailCnt;
	UINT8  u1RuTryupCnt;
	UINT8  u1SuggestWF;
	UINT8  fgRuTryupCheck;
	UINT8  fgIsProbeUpPeriod;
	UINT8  fgProbeDownPending;
	UINT8  u1UserIdx;      /* only for debug that show in fw parser */
#if (EXE_IN_INET == 1)
	UINT32 u4LtMcs; /* Upper bound */
	UINT16 u2LtMcsTxOkCnt;  /* Distinguish low traffic */
	UINT16 u2LtTotalTxCnt; /* Distinguish low traffic */
#endif
};

struct GNU_PACKED UNI_EVENT_RA_RU_RA_INFO_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT32 u4EventId;
	UINT16 u2WlanIdx;
	UINT16 u2RuIdx;
	UINT16 u2Direction;
	UINT16 u2DumpGroup;
	struct _UNI_TxCMD_RATE_INFO_T rRuIdxRateInfo;
};

struct GNU_PACKED UNI_EVENT_RA_MU_RA_INFO_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT32 u4EventId;
	UINT16 u2MuGroupIdx;
	UINT16 u2UserIdx;
	UINT16 u2Direction;
	UINT16 u2DumpGroup;
	struct _UNI_TxCMD_RATE_INFO_T rRuIdxRateInfo;
};

struct  GNU_PACKED UNI_EVENT_All_TX_FAIL_WCID_INFORM_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT16 u2WlanIdx;
	UINT8   au1Reserved[2];
};

/* ============== UNI_EVENT_RA End ============== */
/* ============== UNI_EVENT_BF Begin ============== */

/* BF event (0x33) */
struct GNU_PACKED UNI_EVENT_BF {
	/* fixed field */
	UINT8 au1Reserved[4];

	/* tlv */
	UINT8 au1TlvBuffer[0];
};

/* BF event tags */
enum UNI_EVENT_BF_TAG {
	UNI_EVENT_BF_PFMU_TAG = 0x5,
	UNI_EVENT_BF_PFMU_DATA = 0x7,
	UNI_EVENT_BF_STAREC = 0xB,
	UNI_EVENT_BF_CAL_PHASE = 0xC,
	UNI_EVENT_BF_FBK_INFO = 0x17,
	UNI_EVENT_BF_TXSND_INFO = 0x18,
	UNI_EVENT_BF_PLY_INFO = 0x19,
	UNI_EVENT_BF_METRIC_INFO = 0x1A,
	UNI_EVENT_BF_TXCMD_CFG_INFO = 0x1B,
	UNI_EVENT_BF_SND_CNT_INFO = 0x1D,
	UNI_EVENT_BF_MAX_NUM
};


enum UNI_EVENT_BF_SND_CNT_CONDITION {
	UNI_SND_CNT_CONDI_DEFAULT = 0,
	UNI_SND_CNT_CONDI_8RU = BIT(0),
	UNI_SND_CNT_CONDI_MANUAL = BIT(7)
};

struct GNU_PACKED UNI_EVENT_BF_PFMU_TAG_READ {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  fgBFer;
	UINT8  au1Reserved[3];
	UINT32 au4Tag1RawData[7];
	UINT32 au4Tag2RawData[7];
};

struct GNU_PACKED UNI_EVENT_BF_PFMU_DATA_READ {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2SubCarrIdx;
	UINT8  au1Reserved[2];
	/* Todo: Please check here if new chip model is added */
#if defined(BELLWETHER) || defined(MT7990) || defined(MT7992) || defined(MT7993)
	UINT32 au4RawData[67];
#else
	UINT32 au4RawData[33];
#endif
};

struct GNU_PACKED UNI_EVENT_BF_STA_REC {
	UINT16 u2Tag;
	UINT16 u2Length;

	TXBF_PFMU_STA_INFO rTxBfPfmuInfo;
};


struct GNU_PACKED UNI_EVENT_IBF_PHASE_INFO {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1EventCategoryID;
	UINT8  u1Group_L_M_H;
	UINT8  u1Group;
	UINT8  u1DbdcBandIdx;
	UINT8  u1Status;
	UINT8  u1PhaseCalType;
	UINT8  au1Reserved[2];
	UINT8  au1IbfPhaseOut[20];
	UINT8  au1IbfPhaseGx[44];
};

struct GNU_PACKED UNI_EVENT_IBF_PHASE_INFO_V3 {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1EventCategoryID;
	UINT8  u1Group_L_M_H;
	UINT8  u1Group;
	UINT8  u1DbdcBandIdx;
	UINT8  u1Status;
	UINT8  u1PhaseCalType;
	UINT8  u1Nsts;
	UINT8  u1PhaseOutRptVersion;
	UINT8  au1IbfPhaseOut[25];
	UINT8  au1IbfPhaseGx[34];
	UINT8  au1Reserved1[5];
};

struct GNU_PACKED UNI_EVENT_BF_PLY_NSS {
	UINT8 u1SSGrp: 4;
	UINT8 u1SSPly: 4;
};

struct GNU_PACKED UNI_EVENT_BF_PLY_INFORM {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1GloOpt;
	UINT8  u1GrpIBfOpt;
	UINT8  u1GrpEBfOpt;
	UINT8  u1Reserved;
	struct UNI_EVENT_BF_PLY_NSS arStaSS[MAX_LEN_OF_MAC_TABLE][4];
	UINT8  u1StaRlt[MAX_LEN_OF_MAC_TABLE];
};

struct GNU_PACKED UNI_EVENT_BF_MU_METRIC_RPT {
	UINT8 u1BPSK;
	UINT8 u1QPSK;
	UINT8 u116QAM;
	UINT8 u164QAM;
	UINT8 u1256QAM;
	UINT8 u11024QAM;
	UINT8 u1Capacity;
	UINT8 u1Reserved;
};

struct GNU_PACKED UNI_EVENT_BF_METRIC {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1CurState;
	UINT8  u1RunningFailCnt;
	UINT8  u1ErrRptCnt;
	UINT8  u1FreeReqCnt;
	UINT8  u1PendingReqCnt;
	UINT8  u1PollingTime;
	UINT8  u1NUser;
	UINT8  fgIsLQErr;
	UINT16 u2LQErr;
	UINT8  u1Reserved[2];
	struct UNI_EVENT_BF_MU_METRIC_RPT arMetricRpt[UNI_EVENT_HERA_METRIC_USERS];
	UINT8 au1InitMCSUser[UNI_EVENT_HERA_METRIC_USERS];
};

struct GNU_PACKED UNI_EVENT_BF_TXCMD_CFG_INFORM {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  fgTxCmdBfManual;
	UINT8  u1TxCmdBfBit;
	UINT8  u1Reserved[2];
};

struct GNU_PACKED UNI_EVENT_BF_SND_CNT_INFORM_BASIC {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2SndCntLmt;
	UINT16 u2SndCntLmtMan;
	UINT8  u1SndCndCondi;
	UINT8  u1Reserved[3];
};

/*
 * struct GNU_PACKED UNI_EVENT_BF_SND_CNT_INFORM {
 *	UINT16 u2Tag;
 *	UINT16 u2Length;
 *	UINT16 u2SndCntLmt;
 *	UINT16 u2SndCntLmtMan;
 *	UINT8  u1SndCndCondi;
 *	UINT8  u1Reserved[3];
 *	UINT16 au2SndCnt[ALIGN_4(CFG_WIFI_RAM_BAND_NUM)];
 *};
 */

struct GNU_PACKED UNI_EVENT_BF_TXSND_STA_INFO {
	UINT8 u1SndIntv;      /* Sounding interval upper bound, unit:15ms */
	UINT8 u1SndIntvCnt;   /* Sounding interval counter */
	UINT8 u1SndTxCnt;     /* Tx sounding count for debug */
	UINT8 u1SndStopReason;  /* Bitwise reason to put in Stop Queue */
};

struct GNU_PACKED UNI_EVENT_BF_SND_INFO_BASIC {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8  u1VhtOpt;
	UINT8  u1HeOpt;
	UINT8  u1GloOpt;
	UINT8  u1Reserved;
};

/*
 * struct GNU_PACKED UNI_EVENT_BF_SND_INFO {
 *	UINT16 u2Tag;
 *	UINT16 u2Length;
 *
 *	UINT8  u1VhtOpt;
 *	UINT8  u1HeOpt;
 *	UINT8  u1GloOpt;
 *	UINT8  u1Reserved;
 *	UINT32 au4SndRecSuSta[BF_SND_CTRL_STA_DWORD_CNT];
 *	UINT32 au4SndRecVhtMuSta[BF_SND_CTRL_STA_DWORD_CNT];
 *	UINT32 au4SndRecHeTBSta[BF_SND_CTRL_STA_DWORD_CNT];
 *	UINT32 au4SndRecEhtTBSta[BF_SND_CTRL_STA_DWORD_CNT];
 *	UINT16 u2WlanIdxForMcSnd[ALIGN_4(CFG_WIFI_RAM_BAND_NUM)];
 *	UINT16 u2WlanIdxForHeTbSnd[ALIGN_4(CFG_WIFI_RAM_BAND_NUM)];
 *	UINT16 u2WlanIdxForEhtTbSnd[ALIGN_4(CFG_WIFI_RAM_BAND_NUM)];
 *	UINT16 u2ULLength;
 *	UINT8  u1Mcs;
 *	UINT8  u1LDPC;
 *	struct UNI_EVENT_BF_TXSND_STA_INFO arSndStaInfo[CFG_BF_STA_REC_NUM];
 * };
 */

struct GNU_PACKED UNI_EVENT_TXBF_FBK_INFO_BASIC {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT32 u4DeQInterval;     /* By ms */
	UINT32 u4PollPFMUIntrStatTimeOut; /* micro-sec */
	UINT32 u4RptPktTimeOutListNum;
	UINT32 u4RptPktListNum;
	UINT32 u4PFMUWRTimeOutCnt;
	UINT32 u4PFMUWRFailCnt;
	UINT32 u4PFMUWRDoneCnt;
	UINT32 u4PFMUWRTimeoutFreeCnt;
	UINT32 u4FbRptPktDropCnt;
};

/*
 * struct GNU_PACKED UNI_EVENT_TXBF_FBK_INFO {
 *	UINT16 u2Tag;
 *	UINT16 u2Length;
 *
 *	UINT32 u4DeQInterval;
 *	UINT32 u4PollPFMUIntrStatTimeOut;
 *	UINT32 u4RptPktTimeOutListNum;
 *	UINT32 u4RptPktListNum;
 *	UINT32 u4PFMUWRTimeOutCnt;
 *	UINT32 u4PFMUWRFailCnt;
 *	UINT32 u4PFMUWRDoneCnt;
 *	UINT32 u4PFMUWRTimeoutFreeCnt;
 *	UINT32 u4FbRptPktDropCnt;
 *	UINT32 au4RxPerStaFbRptCnt[CFG_BF_STA_REC_NUM];
 *};
 */

/* ============== UNI_EVENT_BF End ============== */

/* ============== UNI_EVENT_ID_RXFE_CTRL Begin ============== */
struct GNU_PACKED UNI_EVENT_RXFE_T {
	/* fixed field */
	UINT8 au1Padding[4];

	/* tlv */
	UINT8 au1TlvBuffer[0];
};

enum UNI_EVENT_ID_RXFE_TAG_T {
	UNI_EVENT_RXFE_LOSS_COMP = 0,
	UNI_EVENT_RXFE_MAX_NUM
};

struct GNU_PACKED UNI_EVENT_PHY_RXFELOSS_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 u1BandIdx;
	UINT8 au1Reserved[3];
	INT8  i1FeLossComp[MAX_ANTENNA_NUM];
};
/* ============== UNI_EVENT_ID_RXFE_CTRL End ============== */

/* ============== UNI_EVENT_ID_HWCFG_CTRL Begin ============== */

struct GNU_PACKED UNI_EVENT_HWCFG_T {
	/* fixed field */
	UINT8 au1Padding[4];

	/* tlv */
	UINT8 au1TlvBuffer[0];
};

enum _UNI_EVENT_ID_HWCFG_TAG_T {
	UNI_EVENT_HWCFG_READ = 0,
	UNI_EVENT_HWCFG_MAX_NUM
};

struct GNU_PACKED UNI_EVENT_HWCFG_READ_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2Offset;  /* Read Offset */
	UINT16 u2Count;   /* Read Total Counts */
	UINT8  u1BinContent[UNI_HWCFG_INF0_MAX_DUMP_SIZE];
};
/* ============== UNI_EVENT_ID_HWCFG_CTRL End ============== */

/* ============== UNI_EVENT_STA_REC Begin ======= */
/* BF event tags */
enum UNI_EVENT_STAREC_TAG {
	UNI_EVENT_STAREC_UPDATE_MAX_AMSDU_LEN = 0,
	UNI_EVENT_STAREC_SEC_PN_INFO = 0x26,
	UNI_EVENT_STAREC_MAX_NUM
};

typedef struct _UNI_EVENT_STAREC_T
{
	/* fixed field */
	UINT16 u2WlanIdx;
	UINT8 aucPadding[2];

	/* tlv */
	UINT8 aucTlvBuffer[0]; /**< the TLVs included in this field:
								*
								*   TAG                                   |  ID  | structure
								*   -----------------------------------   | -----| -----------------------------------
								*   UNI_EVENT_STAREC_UPDATE_MAX_AMSDU_LEN | 0x00 | UNI_EVENT_STAREC_UPDATE_MAX_AMSDU_LEN_T
								*   UNI_EVENT_STAREC_PN_INFO              | 0x26 | UNI_EVENT_STAREC_PN_INFO_T
								*/
} UNI_EVENT_STAREC_T, *P_UNI_EVENT_STAREC_T;

typedef struct GNU_PACKED _UNI_EVENT_STAREC_UPDATE_MAX_AMSDU_LEN_T
{
	UINT16 u2Tag; /* Tag = 0x00 */
	UINT16 u2Length;
	UINT16 u2AmsduLen;
	UINT8 aucPadding[2];
} UNI_EVENT_STAREC_UPDATE_MAX_AMSDU_LEN_T, *P_UNI_EVENT_STAREC_UPDATE_MAX_AMSDU_LEN_T;
/* ============== UNI_EVENT_STA_REC Begin ======= */

/* ============== UNI_EVENT_ID_THERMAL Begin ============== */

struct UNI_EVENT_THERMAL_T {
	/* fixed field */
	UINT8 au1Padding[4];

	/* tlv */
	UINT8 au1TlvBuffer[0];
};

struct _UNI_EVENT_THERMAL_RSP_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	UINT_8 aucBuffer[0];
};

enum UNI_THERMAL_EVENT_CATEGORY {
	UNI_THERMAL_EVENT_TEMPERATURE_INFO = 0x0,
	UNI_THERMAL_EVENT_THERMAL_SENSOR_BASIC_INFO = 0x1,
	UNI_THERMAL_EVENT_THERMAL_SENSOR_TASK_RESPONSE = 0x2,
	UNI_THERMAL_EVENT_THERMAL_PROTECT_MECH_INFO = 0x3,
	UNI_THERMAL_EVENT_THERMAL_PROTECT_DUTY_INFO = 0x4,
	UNI_THERMAL_EVENT_ADC_TEMP_INFO = 0x5,
	UNI_THERMAL_EVENT_THERMAL_PROTECT_DUTY_UPDATE = 0x6,
	UNI_THERMAL_EVENT_THERMAL_PROTECT_RADIO_UPDATE = 0x7,
	UNI_THERMAL_EVENT_NUM
};

struct UNI_EVENT_RDD_IPI_HIST_CTRL_T {
UINT8 aucReserved[4];
UINT8 aucTlvBuffer[0];
};

enum UNI_EVENT_ID_RDD_IPI_HIST_CTRL_TAG_T {
UNI_EVENT_RDD_IPI_HIST_CTRL = 0x0,
UNI_EVENT_RDD_IPI_HIST_CTRL_MAX_NUM
};

/* ============== UNI_EVENT_ID_THERMAL End ============== */

/* ============== UNI_EVENT_ID_EAP_CTRL Begin ============== */

struct GNU_PACKED UNI_EVENT_EAP_CTRL_T {
	/* fixed field */
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
	    *   TAG                                                      | ID  | structure
	    *   --------------------------------   | --- | -------------
	    *   UNI_EVENT_EAP_GET_IPI_VALUE           | 0x0 | UNI_EVENT_EAP_GET_IPI_VALUE_T
	    *   UNI_EVENT_EAP_SHOW_RATE_TABLE     | 0x1 | UNI_EVENT_EAP_SHOW_RATE_TABLE_T
	    */
};

enum UNI_EVENT_ID_EAP_TAG_T {
	UNI_EVENT_EAP_GET_IPI_VALUE = 0x0,
	UNI_EVENT_EAP_SHOW_RATE_TABLE,
	UNI_EVENT_EAP_MAX_NUM
};

struct  GNU_PACKED UNI_EVENT_EAP_TLV {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 aucBuffer[0];
};

struct  GNU_PACKED UNI_EVENT_EAP_GET_IPI_VALUE_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT32 u4EapCtrlEventId;
	UINT32 au4IPIValue[11];
};

struct GNU_PACKED UNI_EVENT_EAP_SHOW_RATE_TABLE_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT32 u4EapCtrlEventId;
	UINT16 u2RaTblLength;
	UINT8  u1RaTblTypeIdx;
	UINT8  u1RaTblIdx;
	UINT8  u1RW;
	UINT8  u1Reserved[3];
	UINT8  ucBuf[512];
};
/* ============== UNI_EVENT_ID_EAP_CTRL End ============== */

/* ============== UNI_EVENT_ID_NOISE_FLOOR Begin ============== */

struct UNI_EVENT_NOISE_FLOOR_T {
	/* fixed field */
	UINT8 au1Padding[4];

	/* tlv */
	UINT8 au1TlvBuffer[0];
};

struct UNI_EVENT_NF_ENABLE_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 u1mode; /*0: idle power, 1: IPI */
	UINT_8 au1reserved[3];
	UINT_32 au4avgpwr[4];
	UINT_32 au4avgPIHist[12][11] /* ant * ipi */;
	UINT_8  aucReserved[2];
};

enum _UNI_EVENT_NOISE_FLOOR_TAG_T {
	UNI_EVENT_NF_ENABLE = 0,
	UNI_EVENT_NF_MAX_NUM
};

#if defined(MT7990)
#define MAX_BAND_NUM 3
#else
#define MAX_BAND_NUM 2
#endif

/* ============== UNI_EVENT_ID_NOISE_FLOOR End ============== */

/* ============== UNI_EVENT_TWT_AGRT_INFO Start ============ */
struct UNI_EVENT_TWT_AGRT_INFO_T {
	/* fixed field */
	UINT8 au1Reserved[4];
	/* tlv */
	UINT8 au1TlvBuffer[0];
};

/* TWT agrt event Tag */
enum UNI_EVENT_TWT_AGRT_EVENT_TAG_T {
	UNI_EVENT_TWT_AGRT_EVENT_TAG_ALLOC = 0x0,
	UNI_EVENT_TWT_AGRT_EVENT_TAG_NUM
};

/* get TWT hw resource (Tag 0) */

struct UNI_EVENT_TWT_AGRT_ALLOC_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT32 u4Sts;
	UINT16 u2Wcid;
	UINT16 u2ResId;
};
/* ============== UNI_EVENT_TWT_AGRT_INFO End ============ */

/* ============== UNI_EVENT_ID_MEC Start ============ */

struct _UNI_EVENT_MEC_T {
	/* fixed field */
	UINT8 au1Reserved[4];

	/* tlv */
	UINT8 au1TlvBuffer[0];
};

enum _UNI_CMD_MEC_CTRL_BA_TYPE_NUM {
	UNI_CMD_MEC_CTRL_BA_NUM_64 = 0,
	UNI_CMD_MEC_CTRL_BA_NUM_256,
	UNI_CMD_MEC_CTRL_BA_NUM_MAX
};

enum _UNI_CMD_MEC_CTRL_AMSDU_THR_LEN {
	UNI_CMD_MEC_CTRL_AMSDU_THR_IDX_LEN_2 = 0,
	UNI_CMD_MEC_CTRL_AMSDU_THR_IDX_LEN_3,
	UNI_CMD_MEC_CTRL_AMSDU_THR_IDX_LEN_4,
	UNI_CMD_MEC_CTRL_AMSDU_THR_IDX_LEN_5,
	UNI_CMD_MEC_CTRL_AMSDU_THR_IDX_LEN_6,
	UNI_CMD_MEC_CTRL_AMSDU_THR_IDX_LEN_7,
	UNI_CMD_MEC_CTRL_AMSDU_THR_MAX
};

enum _UNI_EVENT_ID_MEC_TAG_T {
	UNI_EVENT_MEC_INFO = 0,
	UNI_EVENT_MEC_MAX_NUM
};

struct _UNI_EVENT_MEC_INFO_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2ReadType;
	UINT8  aucReserved[2];
	UINT32 au4MecAlgoEnSta[(MAX_LEN_OF_MAC_TABLE >> 5)];
	UINT32 au4MecAmsduTable[UNI_CMD_MEC_CTRL_BA_NUM_MAX][UNI_CMD_MEC_CTRL_AMSDU_THR_MAX];
};

/* ============== UNI_EVENT_ID_MEC End ============ */

/**************** UNI_EVENT_ID_CHIP_CAPABILITY Start ****************/

struct EVENT_NIC_CAPABILITY_V2 {
	UINT16 u2TotalElementNum;
	UINT8 aucReserved[2];
	UINT8 aucBuffer[0];
};

typedef UINT32 (*NIC_CAP_V2_ELEMENT_HDLR)(
	struct _RTMP_ADAPTER *ad, UINT8 *buff);

struct NIC_CAPABILITY_V2_REF_TABLE {
	UINT32 tag_type; /* UNI_EVENT_ID_CHIP_CAPABILITY_TAG_T */
	NIC_CAP_V2_ELEMENT_HDLR hdlr;
};

struct NIC_CAPABILITY_V2_HW_VERSION {
	UINT16 chip_id;
	UINT16 hw_ver;
	UINT32 mac_ver;
	UINT32 bbp_id;
	UINT32 top_ver;
	UINT32 config_id;
};

struct NIC_CAPABILITY_V2_EFUSE_VERSION {
	UINT32 part_number;
	UINT32 reserve;
};

/* CHIP CONFIG Tag */
enum UNI_EVENT_ID_CHIP_CAPABILITY_TAG_T {
	UNI_EVENT_CHIP_CONFIG_CAP_TX_RESOURCE = 0x0,
	UNI_EVENT_CHIP_CONFIG_CAP_TX_EFUSEADDRESS = 0x1,
	UNI_EVENT_CHIP_CONFIG_CAP_COEX_FEATURE = 0x2,
	UNI_EVENT_CHIP_CONFIG_CAP_SINGLE_SKU = 0x3,
	UNI_EVENT_CHIP_CONFIG_CAP_CSUM_OFFLOAD = 0x4,
	UNI_EVENT_CHIP_CONFIG_CAP_HW_VERSION = 0x5,
	UNI_EVENT_CHIP_CONFIG_CAP_SW_VERSION = 0x6,
	UNI_EVENT_CHIP_CONFIG_CAP_MAC_ADDR = 0x7,
	UNI_EVENT_CHIP_CONFIG_CAP_PHY_CAP = 0x8,
	UNI_EVENT_CHIP_CONFIG_CAP_MAC_CAP = 0x9,
	UNI_EVENT_CHIP_CONFIG_CAP_FRAME_BUF_CAP = 0xa,
	UNI_EVENT_CHIP_CONFIG_CAP_BEAMFORM_CAP = 0xb,
	UNI_EVENT_CHIP_CONFIG_CAP_LOCATION_CAP = 0xc,
	UNI_EVENT_CHIP_CONFIG_CAP_MUMIMO_CAP = 0xd,
	UNI_EVENT_CHIP_CONFIG_CAP_BUFFER_MODE_INFO = 0xe,
	UNI_EVENT_CHIP_CONFIG_CAP_R_Mode_CAP = 0xf,
	UNI_EVENT_CHIP_CONFIG_CAP_CMD_ID_SUPPORT_LIST = 0x10,
	UNI_EVENT_CHIP_CONFIG_CAP_CMD_EXTID_SUPPORT_LIST = 0x11,
	UNI_EVENT_CHIP_CONFIG_CAP_CMD_TAGID_SUPPORT_LIST_24 = 0x12,
	UNI_EVENT_CHIP_CONFIG_CAP_CMD_TAGID_SUPPORT_LIST_56 = 0x13,
	UNI_EVENT_CHIP_CONFIG_CAP_HW_ADIE_VERSION = 0x14,
	UNI_EVENT_CHIP_CONFIG_CAP_ATCMD_FEATURE_SUPPORT_LIST = 0x15,
	UNI_EVENT_CHIP_CONFIG_CAP_ANT_SWAP = 0x16,
	UNI_EVENT_CHIP_CONFIG_CAP_WFDMA_REALLOC = 0x17,
	UNI_EVENT_CHIP_CONFIG_CAP_6G_CAP = 0x18,
	UNI_EVENT_CHIP_CONFIG_CAP_HOST_STATUS_ADDRESS = 0x19,
	UNI_EVENT_CHIP_CONFIG_CAP_FAST_PATH = 0x1A,
	UNI_EVENT_CHIP_CONFIG_CAP_PSE_RX_QUOTA = 0x1B,
	UNI_EVENT_CHIP_CONFIG_CAP_LLS_DATA_EMI_OFFSET = 0x1C,
	UNI_EVENT_CHIP_CONFIG_CAP_CASAN_LOAD_TYPE = 0x1D,
	UNI_EVENT_CHIP_CONFIG_CAP_RDL_INFO = 0x1E,
	UNI_EVENT_CHIP_CONFIG_CAP_HOST_SUSPEND_INFO = 0x1F,
	UNI_EVENT_CHIP_CONFIG_CAP_UNIFIED_CMD_VERSION = 0x20,
	UNI_EVENT_CHIP_CONFIG_CAP_EFUSE_VERSION = 0x21,
	UNI_EVENT_CHIP_CONFIG_CAP_TOTAL
};

/**************** UNI_EVENT_ID_CHIP_CAPABILITY End ****************/

/* ============== UNI_EVENT_ID_TESTMODE_CTRL Start ============ */

struct _UNI_EVENT_TESTMODE_CTRL_T {
	/*fix field*/
	UINT_8 au1Reserved[4];

	/*tlv */
	UINT_8 aucTlvBuffer[0];/**< the TLVs included in this field:
	*
	*   TAG                               | ID  | structure
	*   ----------------------------------|-----|--------------
	*   UNI_EVENT_RF_TEST_RESULT     | 0x0 | UNI_EVENT_RF_TEST_RESULT_T
	*/
};
/** @} */

/* testmode RF test event tag */
enum _UNI_EVENT_TESTMODE_CTRL_TAG_T {
	UNI_EVENT_RF_TEST_RESULT,
	UNI_EVENT_RF_TEST_MAX_NUM
};

struct _UNI_EVENT_RF_TEST_TLV_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	UINT_8  aucBuffer[0];
};

struct _UNI_EVENT_RF_TEST_RESULT_T {
	UINT32 u4FuncIndex;
	UINT32 u4PayloadLength;
	uint8_t  aucEvent[0];
};

struct GNU_PACKED _UNI_EVENT_RBIST_DUMP_DATA_T {
	UINT32 u4FuncIndex;
	UINT32 u4PktNum;
	UINT32 u4Bank;
	UINT32 u4DataLen;
	UINT32 u4WFCnt;
	UINT32 u4SmplCnt;
	UINT32 u4Reserved[6];
	UINT32 u4Data[UNI_EVENT_RBIST_DUMP_DATA_SIZE];
};

struct GNU_PACKED _UNI_EVENT_RBIST_CAP_STATUS_T {
	UINT32 u4FuncIndex;
	UINT32 u4CapDone;
	UINT32 u4Reserved[15];
};

/* ============== UNI_EVENT_ID_TESTMODE_CTRL End ============== */

/* ============== UNI_EVENT_SR Start ============ */
/* SR Event Tag ID */
#ifdef CFG_SUPPORT_FALCON_SR

enum _UNI_EVENT_SR_TAG_T {
	UNI_EVENT_SR_RSV = 0x0,
	UNI_EVENT_SR_CFG_SR_ENABLE = 0x1,
	UNI_EVENT_SR_CFG_SR_SD_ENABLE = 0x2,
	UNI_EVENT_SR_CFG_SR_MODE = 0x3,
	UNI_EVENT_SR_CFG_DISRT_ENABLE = 0x4,
	UNI_EVENT_SR_CFG_DISRT_MIN_RSSI = 0x5,
	UNI_EVENT_SR_CFG_SR_BF = 0x6,
	UNI_EVENT_SR_CFG_SR_ATF = 0x7,
	UNI_EVENT_SR_CFG_TXC_QUEUE = 0x8,
	UNI_EVENT_SR_CFG_TXC_QID = 0x9,
	UNI_EVENT_SR_CFG_TXC_PATH = 0xA,
	UNI_EVENT_SR_CFG_AC_METHOD = 0xB,
	UNI_EVENT_SR_CFG_SR_PERIOD_THR = 0xC,
	UNI_EVENT_SR_CFG_QUERY_TXD_METHOD = 0xD,
	UNI_EVENT_SR_CFG_SR_SD_CG_RATIO = 0xE,
	UNI_EVENT_SR_CFG_SR_SD_OBSS_RATIO = 0xF,
	UNI_EVENT_SR_CFG_PROFILE = 0x10,
	UNI_EVENT_SR_CFG_FNQ_ENABLE = 0x11,
	UNI_EVENT_SR_CFG_DPD_ENABLE = 0x12,
	UNI_EVENT_SR_CFG_SR_TX_ENABLE = 0x13,
	UNI_EVENT_SR_CFG_SR_SD_OM_ENABLE = 0x14,
	UNI_EVENT_SR_CFG_SR_TX_ALIGN_ENABLE = 0x15,
	UNI_EVENT_SR_CFG_SR_TX_ALIGN_RSSI_THR = 0x16,
	UNI_EVENT_SR_CFG_SR_DABS_MODE = 0x17,
	UNI_EVENT_CSR_CFG_ENABLE = 0x18,
	UNI_EVENT_CSR_FIX_PWR = 0x19,
	UNI_EVENT_CSR_FIX_RATE = 0x1A,
	UNI_EVENT_SR_SW_COSR_SHARED_AP_STATUS = 0x1B,
	UNI_EVENT_SR_SW_SR_RATE_OFFSET = 0x1C,
	UNI_EVENT_SR_SW_SR_APCLI_MODE = 0x1D,
	UNI_EVENT_SR_SW_SR_SUPOORT_MODE = 0x1E,
	/*UNI_EVENT_SR_CFG_RSV= 0x1E,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x1F,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x20,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x21,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x22,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x23,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x24,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x25,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x26,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x27,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x28,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x29,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x2A,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x2B,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x2C,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x2D,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x2E,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x2F,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x30,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x31,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x32,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x33,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x34,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x35,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x36,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x37,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x38,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x39,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x3A,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x3B,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x3C,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x3D,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x3E,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x3F,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x40,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x41,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x42,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x43,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x44,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x45,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x46,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x47,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x48,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x49,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x4A,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x4B,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x4C,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x4D,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x4E,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x4F,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x50,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x51,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x52,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x53,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x54,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x55,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x56,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x57,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x58,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x59,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x5A,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x5B,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x5C,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x5D,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x5E,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x5F,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x60,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x61,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x62,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x63,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x64,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x65,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x66,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x67,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x68,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x69,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x6A,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x6B,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x6C,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x6D,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x6E,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x6F,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x70,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x71,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x72,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x73,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x74,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x75,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x76,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x77,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x78,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x79,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x7A,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x7B,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x7C,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x7D,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x7E,*/
	/*UNI_EVENT_SR_CFG_RSV= 0x7F,*/
	UNI_EVENT_SR_SW_SRG_BITMAP = 0x80,
	UNI_EVENT_SR_SW_MESH_SRG_BITMAP = 0x81,
	UNI_EVENT_SR_SW_CNT = 0x82,
	UNI_EVENT_SR_SW_SD = 0x83,
	UNI_EVENT_SR_SW_GLOVAR_DROPTA_INFO = 0x84,
	UNI_EVENT_SR_SW_GLOVAR_STA_INFO = 0x85,
	UNI_EVENT_SR_GLOVAR_SINGLE_DROP_TA_INFO = 0x86,
	UNI_EVENT_CSR_COORDAP_INFO = 0x89,
	UNI_EVENT_CSR_STA_INFO = 0x8B,
	UNI_EVENT_CSR_EVENT_CNT = 0x8D,
	UNI_EVENT_CSR_INFO = 0x8E,
	UNI_EVENT_SR_SW_MESH_STA_RSSI_TH = 0x8F,
	UNI_EVENT_SR_SW_BH_MESH_SRG_BITMAP = 0x90,
	UNI_EVENT_SR_SW_FH_MESH_SRG_BITMAP = 0x91,
	UNI_EVENT_SR_SW_BH_DL_MESH_SR_RSSI_TH = 0x92,
	UNI_EVENT_SR_SW_FH_DL_MESH_SR_RSSI_TH = 0x93,
	UNI_EVENT_SR_SW_BH_FORBID_BITMAP = 0x94,
	UNI_EVENT_SR_SW_SR_TX_COUNTER = 0x95,
	/*UNI_EVENT_SR_SW_RSV= 0x96,*/
	/*UNI_EVENT_SR_SW_RSV= 0x97,*/
	/*UNI_EVENT_SR_SW_RSV= 0x98,*/
	/*UNI_EVENT_SR_SW_RSV= 0x99,*/
	/*UNI_EVENT_SR_SW_RSV= 0x9A,*/
	/*UNI_EVENT_SR_SW_RSV= 0x9B,*/
	/*UNI_EVENT_SR_SW_RSV= 0x9C,*/
	/*UNI_EVENT_SR_SW_RSV= 0x9D,*/
	/*UNI_EVENT_SR_SW_RSV= 0x9E,*/
	/*UNI_EVENT_SR_SW_RSV= 0x9F,*/
	/*UNI_EVENT_SR_SW_RSV= 0xA0,*/
	/*UNI_EVENT_SR_SW_RSV= 0xA1,*/
	/*UNI_EVENT_SR_SW_RSV= 0xA2,*/
	/*UNI_EVENT_SR_SW_RSV= 0xA3,*/
	/*UNI_EVENT_SR_SW_RSV= 0xA4,*/
	/*UNI_EVENT_SR_SW_RSV= 0xA5,*/
	/*UNI_EVENT_SR_SW_RSV= 0xA6,*/
	/*UNI_EVENT_SR_SW_RSV= 0xA7,*/
	/*UNI_EVENT_SR_SW_RSV= 0xA8,*/
	/*UNI_EVENT_SR_SW_RSV= 0xA9,*/
	/*UNI_EVENT_SR_SW_RSV= 0xAA,*/
	/*UNI_EVENT_SR_SW_RSV= 0xAB,*/
	/*UNI_EVENT_SR_SW_RSV= 0xAC,*/
	/*UNI_EVENT_SR_SW_RSV= 0xAD,*/
	/*UNI_EVENT_SR_SW_RSV= 0xAE,*/
	/*UNI_EVENT_SR_SW_RSV= 0xAF,*/
	/*UNI_EVENT_SR_SW_RSV= 0xB0,*/
	/*UNI_EVENT_SR_SW_RSV= 0xB1,*/
	/*UNI_EVENT_SR_SW_RSV= 0xB2,*/
	/*UNI_EVENT_SR_SW_RSV= 0xB3,*/
	/*UNI_EVENT_SR_SW_RSV= 0xB4,*/
	/*UNI_EVENT_SR_SW_RSV= 0xB5,*/
	/*UNI_EVENT_SR_SW_RSV= 0xB6,*/
	/*UNI_EVENT_SR_SW_RSV= 0xB7,*/
	/*UNI_EVENT_SR_SW_RSV= 0xB8,*/
	/*UNI_EVENT_SR_SW_RSV= 0xB9,*/
	/*UNI_EVENT_SR_SW_RSV= 0xBA,*/
	/*UNI_EVENT_SR_SW_RSV= 0xBB,*/
	/*UNI_EVENT_SR_SW_RSV= 0xBC,*/
	/*UNI_EVENT_SR_SW_RSV= 0xBD,*/
	/*UNI_EVENT_SR_SW_RSV= 0xBE,*/
	/*UNI_EVENT_SR_SW_RSV= 0xBF,*/
	UNI_EVENT_SR_HW_CAP = 0xC0,
	UNI_EVENT_SR_HW_PARA = 0xC1,
	UNI_EVENT_SR_HW_COND = 0xC2,
	UNI_EVENT_SR_HW_RCPI_TBL = 0xC3,
	UNI_EVENT_SR_HW_RCPI_TBL_OFST = 0xC4,
	UNI_EVENT_SR_HW_Q_CTRL = 0xC5,
	UNI_EVENT_SR_HW_IBPD = 0xC6,
	UNI_EVENT_SR_HW_NRT = 0xC7,
	UNI_EVENT_SR_HW_NRT_CTRL = 0xC8,
	UNI_EVENT_SR_HW_IND = 0xC9,
	UNI_EVENT_SR_HW_FNQ = 0xCA,
	UNI_EVENT_SR_HW_FRMFILT = 0xCB,
	UNI_EVENT_SR_HW_INTERPS_CTRL = 0xCC,
	UNI_EVENT_SR_HW_INTERPS_DBG = 0xCD,
	UNI_EVENT_SR_HW_SIGA_FLAG = 0xCE,
	UNI_EVENT_SR_HW_SIGA_AUTO_FLAG = 0xCF,
	UNI_EVENT_SR_HW_REMOTE_FH_RSSI = 0xD0,
	UNI_EVENT_SR_HW_REMOTE_BH_INFO = 0xD1,
	UNI_EVENT_SR_HW_MAP_TOPO = 0xD2,
	UNI_EVENT_SR_HW_UPLINK_STATUS = 0xD3,
	UNI_EVENT_SEND_MESH_UPLINK_TRAFFIC = 0xD4,
	UNI_EVENT_SR_HW_MAP_PHASE = 0xD5,
	UNI_EVENT_SR_HW_REMOTE_STA_MODE = 0xD6,
	UNI_EVENT_SR_HW_NONSRG_SR_ENABLE = 0xD7,
	UNI_EVENT_SR_HW_ESR_ENABLE = 0xD8,
	/*UNI_EVENT_SR_HW_RSV= 0xD9,*/
	/*UNI_EVENT_SR_HW_RSV= 0xDA,*/
	/*UNI_EVENT_SR_HW_RSV= 0xDB,*/
	/*UNI_EVENT_SR_HW_RSV= 0xDC,*/
	/*UNI_EVENT_SR_HW_RSV= 0xDD,*/
	/*UNI_EVENT_SR_HW_RSV= 0xDE,*/
	/*UNI_EVENT_SR_HW_RSV= 0xDF,*/
	/*UNI_EVENT_SR_HW_RSV= 0xE0,*/
	/*UNI_EVENT_SR_HW_RSV= 0xE1,*/
	/*UNI_EVENT_SR_HW_RSV= 0xE2,*/
	/*UNI_EVENT_SR_HW_RSV= 0xE3,*/
	/*UNI_EVENT_SR_HW_RSV= 0xE4,*/
	/*UNI_EVENT_SR_HW_RSV= 0xE5,*/
	/*UNI_EVENT_SR_HW_RSV= 0xE6,*/
	/*UNI_EVENT_SR_HW_RSV= 0xE7,*/
	/*UNI_EVENT_SR_HW_RSV= 0xE8,*/
	/*UNI_EVENT_SR_HW_RSV= 0xE9,*/
	/*UNI_EVENT_SR_HW_RSV= 0xEA,*/
	/*UNI_EVENT_SR_HW_RSV= 0xEB,*/
	/*UNI_EVENT_SR_HW_RSV= 0xEC,*/
	/*UNI_EVENT_SR_HW_RSV= 0xED,*/
	/*UNI_EVENT_SR_HW_RSV= 0xEE,*/
	/*UNI_EVENT_SR_HW_RSV= 0xEF,*/
	/*UNI_EVENT_SR_HW_RSV= 0xF0,*/
	/*UNI_EVENT_SR_HW_RSV= 0xF1,*/
	/*UNI_EVENT_SR_HW_RSV= 0xF2,*/
	/*UNI_EVENT_SR_HW_RSV= 0xF3,*/
	/*UNI_EVENT_SR_HW_RSV= 0xF4,*/
	/*UNI_EVENT_SR_HW_RSV= 0xF5,*/
	/*UNI_EVENT_SR_HW_RSV= 0xF6,*/
	/*UNI_EVENT_SR_HW_RSV= 0xF7,*/
	/*UNI_EVENT_SR_HW_RSV= 0xF8,*/
	/*UNI_EVENT_SR_HW_RSV= 0xF9,*/
	/*UNI_EVENT_SR_HW_RSV= 0xFA,*/
	/*UNI_EVENT_SR_HW_RSV= 0xFB,*/
	/*UNI_EVENT_SR_HW_RSV= 0xFC,*/
	/*UNI_EVENT_SR_HW_RSV= 0xFD,*/
	/*UNI_EVENT_SR_HW_RSV= 0xFE,*/
	/*UNI_EVENT_SR_HW_RSV= 0xFF,*/
};

struct UNI_EVENT_SR_EVENT_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	UINT_32 u4Value;
};

struct UNI_EVENT_SR_T {
	UINT_8 u1DbdcIdx;
	UINT_8 au1Padding[3];
	/*TLV*/
	UINT_8 au1TlvBuffer[0];
};

struct UNI_EVENT_SR_SW_SRG_BITMAP_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct SR_SRG_BITMAP_T rSrSrgBitmap[RAM_BAND_NUM];
};

struct UNI_EVENT_SR_CNT_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_32 u4Value;

	struct SR_CNT_T rSrCnt[RAM_BAND_NUM];
};

struct UNI_EVENT_SR_SD_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct SR_SD_T rSrSd[RAM_BAND_NUM];
};

struct UNI_EVENT_SR_GLOBAL_VAR_SINGLE_DROP_TA_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	UINT_16 u2SrTtlTxCntThr;
	UINT_8 u1StaIdx;
	UINT_8 u1DropTaIdx;
	UINT_8 u1CurSrDropTaIdx;
	UINT_8 RSV[3];
	struct SR_DROP_TA_INFO_T rSrDropTaInfo;
};

struct UNI_EVENT_SR_HW_CAP_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_CAP_T_SR_V2 rSrCap;
};

struct UNI_EVENT_SR_PARA_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_PARA_T rSrPara;
};

struct UNI_EVENT_SR_COND_T_SR_V2 {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_COND_T_SR_V2 rSrCond;
};

struct UNI_EVENT_SR_RCPITBL_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_RCPITBL_T rSrRcpiTbl;
};

struct UNI_EVENT_SR_RCPITBL_OFST_T_SR_V2 {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_RCPITBL_OFST_T_SR_V2 rSrRcpiTblOfst;
};

struct UNI_EVENT_SR_Q_CTRL_T_SR_V2 {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_QUEUE_CTRL_T_SR_V2 rSrQCtrl;
};

struct UNI_EVENT_SR_NRT_T_SR_V2 {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_NRT_T_SR_V2 rSrNRT[SR_NRT_ROW_NUM];
};

struct UNI_EVENT_SR_IBPD_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_IBPD_T rSrIBPD;
};

struct UNI_EVENT_SR_NRT_CTRL_T_SR_V2 {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_NRT_CTRL_T_SR_V2 rSrNRTCtrl;
};

struct UNI_EVENT_SR_IND_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_IND_T rSrInd;
};

struct UNI_EVENT_SR_FNQ_CTRL_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_FNQ_CTRL_T rSrFNQCtrl;
};

struct UNI_EVENT_SR_FRM_FILT_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	UINT_32 u4SrFrmFilt;
};

struct UNI_EVENT_SR_INTERPS_CTRL_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_INTERPS_CTRL_T rSrInterPsCtrl;
};

struct UNI_EVENT_SR_INTERPS_DBG_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_INTERPS_DBG_T rSrInterPsDbg;
};

struct UNI_EVENT_SR_SIGA_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct SR_SIGA_FLAG_T rSrSigaFlag;
};

struct UNI_EVENT_SR_MESH_DL_STA_THRESHOLD_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	struct SR_MESH_SR_DL_STA_THRESHOLD rSrMeshSrDLStaThreshold;
};

struct UNI_EVENT_MESH_TOPOLOGY_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	union _SR_MESH_TOPOLOGY_T rSrCmdMeshTopo;
};

struct UNI_EVENT_SR_UL_TRAFFIC_STATUS_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct _SR_UL_STATUS_T rSrUlStatus;
};

#define COSR_COOR_AP_NUM 3

struct UNI_EVENT_CSR_COORDAP_INFO_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	UINT_8 apId[3];
	UINT_8 CoorAPStatus[3];
	UINT_8 aCoorAPBSSID[3][MAC_ADDR_LEN];
	UINT_8 u1Rssi[3];
	UINT_8 Resv;
};

struct UNI_EVENT_CSR_STA_INFO_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	UINT_16 wcid[8];
	UINT_8 mldId[8];
	UINT_8 plDiff[8][3];
	UINT_8 support_11k[8];
	UINT_8 u1Status[8];
};

struct UNI_EVENT_CSR_INFO_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	struct SHARED_STA_RA_T rSharedStaRAInfo;
	struct SHARING_STA_RA_T rSharingStaRAInfo;
};

struct UNI_EVENT_COSR_EVENT_REC_CNT_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	UINT_16 SharingEventCnt[COSR_COOR_AP_NUM];
	UINT_16 SharingAPTxSuccCnt[COSR_COOR_AP_NUM];
	UINT_16 SharedEventCnt[COSR_COOR_AP_NUM];
	UINT_16 SharedAPInsufTimeCnt[COSR_COOR_AP_NUM];
	UINT_16 SharedAPTxSTASrchFaiCnt[COSR_COOR_AP_NUM];
	UINT_16 SharedAPTxCEnqCnt[COSR_COOR_AP_NUM];
	UINT_16 SharedAPTxQEmptyCnt[COSR_COOR_AP_NUM];
	UINT_16 SharedAPTxCEnqAbortCnt[COSR_COOR_AP_NUM];
	UINT_16 SharedAPTxSuccCnt[COSR_COOR_AP_NUM];
	UINT_16 SharedAPTxAcked[COSR_COOR_AP_NUM];
	UINT_16 u2SharingBoostEventCnt;
	UINT_16 u2SharingBoostTxSuccCnt;
};

struct UNI_EVENT_CSR_FIXED_TXPOWER {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 u1Mode;
	UINT_8 u1FixedTXpower;
	UINT_8 Resv[2];
};

struct UNI_EVENT_CSR_FIXED_RATE {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 u1Mode;
	UINT_8 u1FixedRate;
	UINT_8 Resv[2];
};

struct UNI_EVENT_SR_SW_SR_TX_COUNTER_T {
	/* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_32 u4SrTotalTxCount;
	UINT_32 u4SrTotalSucTxCount;
};

#endif/*CFG_SUPPORT_FALCON_SR */
/* ============== UNI_EVENT_SR END ============ */

/* ============== UNI_EVENT_ID_MLO_ALGO START ============ */

#define CONFIG_WIFI_RAM_MAX_MLO_MGMT_SUPPORT_MLD_NUM 32
#define CONFIG_WIFI_RAM_MAX_MLO_MGMT_SUPPORT_TID_NUM 1
#define CONFIG_WIFI_RAM_MAX_MLO_MGMT_SUPPORT_AC_NUM 4

enum UNI_EVENT_MLO_TAG_T {
	UNI_EVENT_MLO_RSV = 0x0,
	UNI_EVENT_MLO_MLO_MGMT = 0x1,
	UNI_EVENT_MLO_MLD_REC = 0x2,
	UNI_EVENT_MLO_MLD_REC_LINK = 0x3,
	UNI_EVENT_MLO_MLD_REC_TID_AGC_TX = 0x4,
	UNI_EVENT_MLO_MLD_REC_TID_AGC_TRIG = 0x5,
	UNI_EVENT_MLO_CONFIG_OPERATION = 0x6,
	UNI_EVENT_MLO_OPTION_CTRL = 0x7,
};

/* MLD Record - The record of MLD that connect with me.*/
struct MLO_AGC_DISP_PARAM_TX_T {
	UINT_8 u1Rsv[3];
	UINT_8 u1AgcStateTx;/* enum MLO_STATE */
	UINT_8 u1DispPolTx[CONFIG_WIFI_RAM_MAX_MLO_MGMT_SUPPORT_AC_NUM];
	UINT_8 u1DispRatioTx;
	UINT_8 u1DispOrderTx;
	UINT_16 u2DispMgfTx;
};

struct MLO_AGC_DISP_PARAM_TRIG_T {
	UINT_8 u1Rsv[3];
	UINT_8 u1AgcStateTrig;/* enum MLO_STATE */
	UINT_8 u1DispPolTrig[CONFIG_WIFI_RAM_MAX_MLO_MGMT_SUPPORT_AC_NUM];
	UINT_8 u1DispRatioTrig;
	UINT_8 u1DispMuLenTrig;
	UINT_16 u2DispMgfTrig;
};

struct MLO_OVL_RPT_T {
	UINT_16 u2Corr1;
	UINT_16 u2Corr0;
	UINT_16 u2InCorr1;
	UINT_16 u2InCorr0;
};

struct MLD_RECORD_LINK_T {
BOOLEAN fgActive;
BOOLEAN fgSuspend;
BOOLEAN fgSuspendEmlsr;
BOOLEAN fgTxLimit;
BOOLEAN fgShareDispRatio;
UINT_8  u1ParentMldRecIdx;
UINT_8  u1Band;
UINT_8  u1CmdRptTxFailCnt;
UINT_8	u1Rsv;
UINT_16 u2WlanIdx;
UINT_32 u4SuspendTs;
struct MLO_AGC_DISP_PARAM_TX_T rAgcDispParamTx;
struct MLO_AGC_DISP_PARAM_TRIG_T rAgcDispParamTrig;
struct MLO_OVL_RPT_T arOvlpRptCntTx[CONFIG_WIFI_RAM_MAX_MLO_MGMT_SUPPORT_AC_NUM];
struct MLO_OVL_RPT_T arOvlpRptCntTrig[CONFIG_WIFI_RAM_MAX_MLO_MGMT_SUPPORT_AC_NUM];
};

struct MLD_RECORD_T {
UINT_8  u1MldRecState;
UINT_8  u1MldRecIdx;
UINT_16 u2StaRecMldIdx;
UINT_16 u2PrimaryMldId;
UINT_16 u2SecondMldId;
BOOLEAN fgIsStr;
UINT_8  u1StrBmp;
UINT_8  u1EmlsrBmp;
UINT_8  u1ActiveLinkNum;
UINT_8  u1ActiveLinkBmp;
UINT_8  u1SuspendLinkNum;
BOOLEAN fgAgcAggressiveMode[2];
UINT_8  u1DesireSuspendBmp;
UINT_8  u1PauseAllLinkCnt;
UINT_8  u1Rsv[2];
struct MLD_RECORD_LINK_T arMldRecLink[CFG_WIFI_RAM_BAND_NUM];
};

struct UNI_EVENT_MLO_MLD_REC_T {
	/* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
	/* DW_1 */
	struct MLD_RECORD_T rMldRec;
};

/* ============== UNI_EVENT_ID_MLO_ALGO END ============ */
#ifdef DOT11_EHT_BE
/* ============== UNI_EVENT_ID_PP Start ============== */
struct UNI_EVENT_PP_ALG_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT32 u4PpTimerIntv;
	UINT32 u4ThrX2_Value;
	UINT32 u4ThrX2_Shift;
	UINT32 u4ThrX3_Value;
	UINT32 u4ThrX3_Shift;
	UINT32 u4ThrX4_Value;
	UINT32 u4ThrX4_Shift;
	UINT32 u4ThrX5_Value;
	UINT32 u4ThrX5_Shift;
	UINT32 u4ThrX6_Value;
	UINT32 u4ThrX6_Shift;
	UINT32 u4ThrX7_Value;
	UINT32 u4ThrX7_Shift;
	UINT32 u4ThrX8_Value;
	UINT32 u4ThrX8_Shift;
	UINT32 u4SwPpTime;
	UINT32 u4HwPpTime;
	UINT32 u4NoPpTime;
	UINT32 u4AutoBwTime;
	UINT8  u1DbdcIdx;
	UINT8  u1Reserved[1];
	UINT16 u2SwPpBitmap;
	UINT64 u8TxFBwCnt;
	UINT64 u8TxAutoBwCnt;
	UINT64 u8TxPpCnt;
	UINT16 u2OpPpBitmap;
	UINT16 u2ApStaticPpBitmap;
};

struct  UNI_EVENT_STATIC_PP_DSCB_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1BandIdx;
	UINT8  u1OmacIdx;
	UINT8  fgIsDscbEnable;
	UINT8  ucReserved;
	UINT16 u2DscbBitmap;
	UINT8  u1Reserved[2];
};

/* Static PP event Tag */
enum ENUM_UNI_EVENT_STATIC_PP_TAG {
	UNI_EVENT_PP_TAG_ALG_CTRL = 0x1,
	UNI_EVENT_STATIC_PP_TAG_DSCB_IE = 2,
	UNI_EVENT_STATIC_PP_TAG_CSA_DSCB_IE = 3,
	UNI_EVENT_PP_SHOW_INFO = 4,
	UNI_EVENT_STATIC_PP_TAG_NUM
};

struct GNU_PACKED UNI_EVENT_DSCB_T {
    /* fixed field */
    UINT8 aucReserved[4];
    /* tlv */
    UINT8 aucTlvBuffer[0];
};
VOID UniEventStaticPpHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);
/* ============== UNI_EVENT_ID_PP End ============== */


/* ============== UNI_EVENT_ID_LOCATION Start ============== */
#ifdef FTM_SUPPORT
/** @addtogroup UNI_EVENT_ID_LOCATION
 *  @{
 */
/** This structure is used for UNI_EVENT_ID_LOCATION event (0x5d)
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] ucReserved       Reserved
 * @param[in] aucTlvBuffer     TLVs
 */
struct _UNI_EVENT_ID_LOCATION_T {
	/*fixed field*/
	UINT8 ucBandIndex;
	UINT16 ucWcid;
	UINT8 ucReserved[1];

	/* tlv */
	UINT_8 aucTlvBuffer[0]; /**< the TLVs included in this field:
	*
	*                TAG                     | ID   | structure
	*   -------------------------------------| -----| -------------
	*   UNI_EVENT_LOC_TAG_LOC_CAPA           | 0x00 | _UNI_EVENT_LOC_CAPA_T
	*   UNI_EVENT_LOC_TAG_LOC_RESULT         | 0x01 | _UNI_EVENT_LOC_RESULT_T
	*   UNI_EVENT_LOC_TAG_LOC_DONE           | 0x02 | _UNI_EVENT_LOC_DONE_T
	*   UNI_EVENT_LOC_TAG_LOC_TRIGGER        | 0x03 | _UNI_EVENT_LOC_TRIGGER_T
	*   UNI_EVENT_LOC_TAG_RANGE_REQ_MC_BURST | 0x07 | _UNI_EVENT_LOC_TAG_RANGE_REQ_MC_BURST_T
	*/
};
/** @} */

/* LOC Event Tag ID */
enum _UNI_EVENT_ID_LOCATION_TAG_T {
	UNI_EVENT_LOC_TAG_LOC_CAPA = 0,
	UNI_EVENT_LOC_TAG_LOC_RESULT = 1,
	UNI_EVENT_LOC_TAG_LOC_DONE = 2,
	UNI_EVENT_LOC_TAG_LOC_TRIGGER = 3,
	UNI_EVENT_LOC_TAG_RANGE_REQ_MC_BURST = 7,
	UNI_EVENT_LOC_TAG_NUM
};

/*UNI_EVENT_LOC_CAPA_T(Tag=0x0)*/
struct _UNI_EVENT_LOC_CAPA_T_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	CMD_GET_LOC_CAP_T rCapabilities;
};

/*UNI_EVENT_LOC_RESULT_T(Tag=0x1)*/
struct _UNI_EVENT_LOC_RESULT_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 aucMacAddr[MAC_ADDR_LEN];
	UINT_8 ucNumPerBurstPeer;
	UINT_8 ucRetryAfterDuration;
	UINT_32 u4BurstNum;
	UINT_32 u4MeasurementNumber;
	UINT_32 u4SuccessNumber;
	ENUM_LOC_STATUS_T eStatus;
	ENUM_LOC_TYPE_T eType;
	INT_32 i4Rssi;
	INT_32 i4RssiSpread;
	WIFI_RATE_T rTxRate;
	WIFI_RATE_T rRxRate;
	INT_64 i8Rtt;
	INT_64 i8RttSd;
	INT_64 i8RttSpread;
	INT_32 i4DistanceMM;
	INT_32 i4DistanceSdMM;
	INT_32 i4DistanceSpreadMM;
	INT_64 i8Ts;
	INT_32 i4BurstDuration;
	INT_32 i4NegotiateBurstNum;
	UINT_16 u2IELen;
	UINT_8 aureserved[2];
	UINT_8 aucIE[0];
};

/*UNI_EVENT_LOC_DONE_T(Tag=0x2)*/
struct _UNI_EVENT_LOC_DONE_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8  ucSeqNum;
};

/*UNI_EVENT_LOC_TRIGGER_T(Tag=0x3)*/
struct _UNI_EVENT_LOC_TRIGGER_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 aucMacAddr[MAC_ADDR_LEN];
	enum _ENUM_LOC_TYPE_T eLocType;
};

/*UNI_EVENT_LOC_TAG_RANGE_REQ_MC_BURST(Tag=0x07)*/
struct _UNI_EVENT_LOC_TAG_RANGE_REQ_MC_BURST_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 aucAddr[MAC_ADDR_LEN];
	UINT_8 eType;
	UINT_8 ucBssIndex;
	WIFI_CHANNEL_INFO_T rChannel;
};

VOID UniEventLocUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);


#endif /* FTM_SUPPORT */
/* ============== UNI_EVENT_ID_LOCATION End ============== */


/* ============== UNI_EVENT_ID_MLD Start ============== */
enum UNI_EVENT_MLD_TAG_T {
	UNI_EVENT_MLD_AT2LM_RES_RSP = 0x02,	/*solicited*/
	UNI_EVENT_MLD_AT2LM_TO = 0x03,		/*unsolicited*/
	UNI_EVENT_MLD_RECONFIG_TO = 0x04,	/*unsolicited*/
};

struct GNU_PACKED UNI_EVENT_MLD_T {
	/* fixed field */
	UINT8 ucEventVer;
	UINT8 aucMldAddr[MAC_ADDR_LEN];
	UINT8 ucFwMldIdx;
	UINT8 ucRsvd[4];
	/* tlv */
	UINT8 aucTlvBuffer[0];
};

struct GNU_PACKED UNI_EVENT_MLD_AT2LM_RSP_T {
	UINT16 u2Tag;		/* 0x02 */
	UINT16 u2Length;
	UINT8 ucSts;
	UINT8 ucAt2lmIdx;
	UINT8 ucBssInfIdx;
	UINT8 Rsvd;
	UINT32 u4MstTsf[2];
	UINT32 u4DisEndTsf[2];
};

struct GNU_PACKED UNI_EVENT_MLD_AT2LM_TO_T {
	UINT16 u2Tag;		/* 0x03 */
	UINT16 u2Length;
	UINT8 ucAt2lmId;
	UINT8 ucType;
	UINT8 Rsvd[2];
};

struct GNU_PACKED UNI_EVENT_MLD_RECONFIG_TO_T {
	UINT16 u2Tag;		/* 0x04 */
	UINT16 u2Length;
	UINT16 ucToLinkIdBmap;
	UINT8  aucFwBssIdxLink[CFG_WIFI_RAM_BAND_NUM];
	UINT8  aucReserved[3];
};
/* ============== UNI_EVENT_ID_MLD End ============== */
#endif

/* ============== UNI_EVENT_ID_STATE Start ============ */

struct GNU_PACKED UNI_EVENT_STATE_T {
	/* fixed field */
	UINT_8 u1BandIdx;
	UINT_8 au1Reserved[3];

	/* tlv */
	UINT_8 au1TlvBuffer[0];
};


struct GNU_PACKED UNI_EVENT_STATE_COMMON_T {
	UINT_16 u2Tag;    // Tag = 0x00, 0x01, 0x02, 0x03, 0x04, 0x05
	UINT_16 u2Length;
	UINT_8 au1StateContent[0];
};

enum UNI_EVENT_STATE_TAG_T {
	UNI_EVENT_STATE_TX_RATE = 0,
	UNI_EVENT_STATE_RX_RATE = 1,
	UNI_EVENT_STATE_OFDMLQ_CNINFO = 2,
	UNI_EVENT_STATE_TX_TD_CCK = 3,
	UNI_EVENT_STATE_TOTAL_TX_CNT = 4,
	UNI_EVENT_STATE_LAST_TX_RATE = 5,
	UNI_EVENT_STATE_RX_STAT = 6,
#ifdef TXRX_STAT_SUPPORT
	UNI_EVENT_STATE_ALL_RATE_CNT_REPORT = 7,
	UNI_EVENT_STATE_STBC_CNT_REPORT = 8,
	UNI_EVENT_STATE_GI_CNT_REPORT = 9,
#endif /* TXRX_STAT_SUPPORT */
	UNI_EVENT_STATE_MAX_NUM
};

struct GNU_PACKED UNI_EVENT_STATE_TX_RATE_T {
	UINT_16 u2Tag;    // Tag = 0x00
	UINT_16 u2Length;
	UINT_8  u1TxRate;
	UINT_8  u1TxMode;
	UINT_8  u1TxNsts;
	UINT_8  u1Reserved;
};

struct GNU_PACKED UNI_EVENT_STATE_RX_RATE_T {
	UINT_16 u2Tag;    // Tag = 0x01
	UINT_16 u2Length;
	UINT_8  u1RxRate;
	UINT_8  u1RxMode;
	UINT_8  u1RxNsts;
	UINT_8  u1Reserved;
};

struct GNU_PACKED UNI_EVENT_STATE_OFDMLQ_CNINFO_T {
	UINT_16 u2Tag;    // Tag = 0x02
	UINT_16 u2Length;
	UINT_16 u2OfdmLqCn;
	UINT_8  au1Reserved[2];
};

struct GNU_PACKED UNI_EVENT_STATE_TX_TD_CCK_T {
	UINT_16 u2Tag;    // Tag = 0x03
	UINT_16 u2Length;
	UINT_8  au1Reserved[4];
};

struct GNU_PACKED UNI_EVENT_STATE_TOTAL_TX_CNT_T {
	UINT_16 u2Tag;    // Tag = 0x04
	UINT_16 u2Length;
	UINT_8  u1BandIdx;
	UINT_8  au1Reserved[3];
	UINT_32 u4TotalTxCount;
	UINT_32 u4TotalTxFailCount;
	UINT_32 u4CurrBwTxCnt;
	UINT_32 u4CurrBwTxFailCnt;
	UINT_32 u4OtherBwTxCnt;
	UINT_32 u4OtherBwTxFailCnt;
};

struct GNU_PACKED UNI_EVENT_STATE_LAST_TX_RATE_PER_BAND_T {
	UINT_8  u1Mode;
	UINT_8  u1Flags;
	UINT_8  u1Stbc;
	UINT_8  u1ShortGI;
	UINT_8  u1Bw;
	UINT_8  u1Ldpc;
	UINT_8  u1Mcs;
	UINT_8  u1VhtNss;
	UINT_8  u1HeLtf;
	UINT_8  au1Reserved[3];
};

struct GNU_PACKED UNI_EVENT_STATE_LAST_TX_RATE_T {
	UINT_16 u2Tag;    // Tag = 0x05
	UINT_16 u2Length;
	UINT_8  u1BandIdx;
	UINT_8  au1Reserved[3];
	struct UNI_EVENT_STATE_LAST_TX_RATE_PER_BAND_T rLastTxRate;
};

struct GNU_PACKED UNI_EVENT_STATE_RX_STAT_PER_BAND_T {
	UINT_16 u2PhyRxPdCck;
	UINT_16 u2PhyRxPdOfdm;
	UINT_16 u2PhyRxMdrdyCntCck;
	UINT_16 u2PhyRxMdrdyCntOfdm;
};

struct GNU_PACKED UNI_EVENT_STATE_RX_STAT_T {
	UINT_16 u2Tag;    // Tag = 0x06
	UINT_16 u2Length;
	struct UNI_EVENT_STATE_RX_STAT_PER_BAND_T arRxStatResult[RAM_BAND_NUM];
};

#ifdef TXRX_STAT_SUPPORT
#ifdef MT7992_SKU_BE3600SDB
struct UNI_EVENT_RATE_CNT_T {
	UINT_32 FailCnt;
	UINT_32 SuccessCnt;
};
struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_2G_T {
	struct UNI_EVENT_RATE_CNT_T rCckCnt[CCK_RATE_MAX];
	struct UNI_EVENT_RATE_CNT_T rOfdmCnt[OFDM_RATE_MAX];
	struct UNI_EVENT_RATE_CNT_T rHtCnt[RA_NSS_MAX][HT_RATE_MAX];
	struct UNI_EVENT_RATE_CNT_T rHeCnt[RA_NSS_MAX][HE_RATE_MAX];
	struct UNI_EVENT_RATE_CNT_T rEhtCnt[RA_NSS_MAX][EHT_RATE_MAX];
};

struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_5G_T {
	struct UNI_EVENT_RATE_CNT_T rOfdmCnt[OFDM_RATE_MAX];
	struct UNI_EVENT_RATE_CNT_T rHtCnt[RA_NSS_MAX][HT_RATE_MAX];
	struct UNI_EVENT_RATE_CNT_T rVhtCnt[RA_NSS_MAX][VHT_RATE_MAX];
	struct UNI_EVENT_RATE_CNT_T rHeCnt[RA_NSS_MAX][HE_RATE_MAX];
	struct UNI_EVENT_RATE_CNT_T rEhtCnt[RA_NSS_MAX][EHT_RATE_MAX];
};

struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_6G_T {
	struct UNI_EVENT_RATE_CNT_T rHeCnt[RA_NSS_MAX][HE_RATE_MAX];
	struct UNI_EVENT_RATE_CNT_T rEhtCnt[RA_NSS_MAX][EHT_RATE_MAX];
};

#else
union UNI_EVENT_UNION_RATE_CNT_T {
	UINT_32 raw;
	struct {
		UINT_32 FailCnt:12;
		UINT_32 SuccessCnt : 20;
	} rRateCnt;
};

struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_2G_T {
	union UNI_EVENT_UNION_RATE_CNT_T rCckCnt[CCK_RATE_MAX];
	union UNI_EVENT_UNION_RATE_CNT_T rOfdmCnt[OFDM_RATE_MAX];
	union UNI_EVENT_UNION_RATE_CNT_T rHtCnt[RA_NSS_MAX][HT_RATE_MAX];
	union UNI_EVENT_UNION_RATE_CNT_T rHeCnt[RA_NSS_MAX][HE_RATE_MAX];
	union UNI_EVENT_UNION_RATE_CNT_T rEhtCnt[RA_NSS_MAX][EHT_RATE_MAX];
};

struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_5G_T {
	union UNI_EVENT_UNION_RATE_CNT_T rOfdmCnt[OFDM_RATE_MAX];
	union UNI_EVENT_UNION_RATE_CNT_T rHtCnt[RA_NSS_MAX][HT_RATE_MAX];
	union UNI_EVENT_UNION_RATE_CNT_T rVhtCnt[RA_NSS_MAX][VHT_RATE_MAX];
	union UNI_EVENT_UNION_RATE_CNT_T rHeCnt[RA_NSS_MAX][HE_RATE_MAX];
	union UNI_EVENT_UNION_RATE_CNT_T rEhtCnt[RA_NSS_MAX][EHT_RATE_MAX];
};

struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_6G_T {
	union UNI_EVENT_UNION_RATE_CNT_T rHeCnt[RA_NSS_MAX][HE_RATE_MAX];
	union UNI_EVENT_UNION_RATE_CNT_T rEhtCnt[RA_NSS_MAX][EHT_RATE_MAX];
};
#endif /* MT7992_SKU_BE3600SDB */

struct UNI_EVENT_STATE_ALL_TXRX_RATE_CNT_REPORT_2G_T {
	UINT_16 u2Tag;    // Tag = 0x07
	UINT_16 u2Length;
	UINT_8 u1BandIdx;
	UINT_8 u1Direction;
	UINT_8 aucReserved[2];
	struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_2G_T rAllRateCnt2G;
};

struct UNI_EVENT_STATE_ALL_TXRX_RATE_CNT_REPORT_5G_T {
	UINT_16 u2Tag;    // Tag = 0x07
	UINT_16 u2Length;
	UINT_8 u1BandIdx;
	UINT_8 u1Direction;
	UINT_8 aucReserved[2];
	struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_5G_T rAllRateCnt5G;
};

struct UNI_EVENT_STATE_ALL_TXRX_RATE_CNT_REPORT_6G_T {
	UINT_16 u2Tag;    // Tag = 0x07
	UINT_16 u2Length;
	UINT_8 u1BandIdx;
	UINT_8 u1Direction;
	UINT_8 aucReserved[2];
	struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_6G_T rAllRateCnt6G;
};

struct UNI_EVENT_STATE_STBC_CNT_REPORT_T {
	UINT_16 u2Tag;    // Tag = 0x08
	UINT_16 u2Length;
	UINT_8 u1BandIdx;
	UINT_8 aucReserved[3];
	UINT32 STBCCnt[ENUM_DIR_MAX];
};

struct UNI_EVENT_STATE_GI_CNT_REPORT_T {
	UINT_16 u2Tag;    // Tag = 0x09
	UINT_16 u2Length;
	UINT_8 u1BandIdx;
	UINT_8 aucReserved[3];
	UINT32 HeEhtGICnt[ENUM_DIR_MAX][ENUM_GI_MAX];
	UINT32 HtVhtGICnt[ENUM_DIR_MAX][ENUM_GI_MAX];
};

#endif /* TXRX_STAT_SUPPORT */
/* ============== UNI_EVENT_ID_STATE End ============ */

/* ============== UNI_EVENT_ID_PER_STA_INFO Start ============ */

struct GNU_PACKED UNI_EVENT_PER_STA_INFO_T {
	/* fixed field */
	UINT_8 au1Reserved[4];

	/* tlv */
	UINT_8 au1TlvBuffer[0];
};

enum UNI_EVENT_PER_STA_INFO_TAG_T {
	UNI_EVENT_PER_STA_RSSI = 0,
	UNI_EVENT_PER_STA_CONTENTION_RX_PHYRATE = 1,
	UNI_EVENT_PER_STA_PER = 2,
	UNI_EVENT_PER_STA_SNR = 3,
	UNI_EVENT_PER_STA_TX_RATE = 4,
	UNI_EVENT_PER_STA_TX_CNT = 5,
	UNI_EVENT_PER_STA_TID_SN = 6,
	UNI_EVENT_PER_STA_PKT_CNT = 8,
	UNI_EVENT_PER_STA_MAX_NUM
};

struct GNU_PACKED UNI_EVENT_PER_STA_INFO_COMMON_T {
	UINT_16 u2Tag;    // Tag = 0x00, 0x01, 0x02, 0x03, 0x04, 0x05
	UINT_16 u2Length;
	UINT_8  au1PerStaContent[0];
};

struct GNU_PACKED UNI_EVENT_PER_STA_RSSI_T {
	UINT_16 u2WlanIdx;
	UINT_8  au1Rcpi4;
	UINT_8  au1Reserved;
	UINT_8  au1Rcpi[4];
};

struct GNU_PACKED UNI_EVENT_PER_STA_CONTENTION_RX_PHYRATE_T {
	UINT_16 u2WlanIdx;
	UINT_8  au1Reserved[2];
	UINT_8  u1RxRate;
	UINT_8  u1RxMode;
	UINT_8  u1RxNsts;
	UINT_8  u1Gi;
	UINT_8  u1Coding;
	UINT_8  u1Stbc;
	UINT_8  u1Bw;
	UINT_8  u1Reserved;
};

struct GNU_PACKED UNI_EVENT_PER_STA_PER_T {
	UINT_16 u2WlanIdx;
	UINT_8  au1Reserved1[2];
	UINT_8  u1Per;
	UINT_8  au1Reserved2[3];
};

struct GNU_PACKED UNI_EVENT_PER_STA_SNR_T {
	UINT_16 u2WlanIdx;
	UINT_8  au1Reserved[2];
	UINT_8  au1Snr[4];
};

struct GNU_PACKED UNI_EVENT_PER_STA_TX_RATE_T {
	UINT_16 u2WlanIdx;
	UINT_8  au1Reserved1[2];
	struct _RA_PHY_CFG_T rEntryTxRate;
	UINT_8  au1Reserved2[3];
};

struct GNU_PACKED UNI_EVENT_PER_STA_TX_CNT_T {
	UINT_16 u2WlanIdx;
	UINT_8  au1Reserved[2];
	UINT_32 u4EntryTxCount;
	UINT_32 u4EntryTxFailCount;
};

struct GNU_PACKED UNI_EVENT_PER_STA_PKT_CNT_T {
	UINT_16 u2WlanIdx;
	UINT_8  au1Reserved[2];
	UINT_32 u4EntryTxDropCount;
	UINT_32 u4EntryTxRetryCount;
};

struct GNU_PACKED UNI_EVENT_PER_STA_TID_SN_T {
	UINT_16 u2WlanIdx;
	UINT_8  au1Reserved[2];
	UINT_16 u2TidSn[8];
};


/* ============== UNI_EVENT_ID_PER_STA_INFO End ============ */

/* ============== UNI_EVENT_ID_ALL_STA_INFO Start ============ */

struct GNU_PACKED UNI_EVENT_ALL_STA_INFO_T {
	/* fixed field */
	UINT_8 au1Reserved[4];

	/* tlv */
	UINT_8 au1TlvBuffer[0];
};

#define UNI_EVENT_ALL_STA_TX_RATE_PER_EVENT                 70
#define UNI_EVENT_ALL_STA_TX_STAT_PER_EVENT                 120
#define UNI_EVENT_ALL_STA_TXRX_ADM_STAT_PER_EVENT           40
#define UNI_EVENT_ALL_STA_TXRX_AIR_TIME_PER_EVENT           40
#define UNI_EVENT_ALL_STA_DATA_TX_RETRY_COUNT_PER_EVENT     180
#define UNI_EVENT_ALL_STA_GI_MODE_PER_EVENT                 180
#define UNI_EVENT_ALL_STA_TRX_MSDU_COUNT_PER_EVENT           180
#define MAX_ACCESS_CAT_NUM                                  4

enum UNI_EVENT_ALL_STA_INFO_TAG_T {
	UNI_EVENT_ALL_STA_TX_RATE = 0,
	UNI_EVENT_ALL_STA_TX_STAT = 1,
	UNI_EVENT_ALL_STA_TXRX_ADM_STAT = 2,
	UNI_EVENT_ALL_STA_TXRX_AIR_TIME = 3,
	UNI_EVENT_ALL_STA_DATA_TX_RETRY_COUNT = 4,
	UNI_EVENT_ALL_STA_GI_MODE = 5,
	UNI_EVENT_ALL_STA_TRX_MSDU_COUNT = 6,
	UNI_EVENT_ALL_STA_MAX_NUM
};

struct GNU_PACKED UNI_EVENT_ALL_STA_INFO_COMMON_T {
	UINT_16 u2Tag;    // Tag = 0x00, 0x01, 0x02, 0x03, 0x04, 0x05
	UINT_16 u2Length;
	UINT_8  u1FlagMoreEvent;
	UINT_8  u1Reserved;
	UINT_16 u2StaNum;
	UINT_8  au1Reserved[4];
	UINT_8  au1AllStaContent[0];
};

struct GNU_PACKED UNI_EVENT_ONE_STA_TX_RATE_T {
	UINT_16 u2WlanIdx;
	UINT_8  au1Reserved1[2];
	UINT_8  u1TxMode;
	UINT_8  u1Flags;
	UINT_8  u1Stbc;
	UINT_8  u1ShortGI;
	UINT_8  u1Bw;
	UINT_8  u1Ldpc;
	UINT_8  u1Mcs;
	UINT_8  u1VhtNss;
	UINT_8  u1RxRate;
	UINT_8  u1RxMode;
	UINT_8  u1RxNsts;
	UINT_8  u1RxGi;
	UINT_8  u1RxCoding;
	UINT_8  u1RxStbc;
	UINT_8  u1RxBW;
	UINT_8  au1Reserved2;
};

struct GNU_PACKED UNI_EVENT_ALL_STA_TX_RATE_T {
	struct UNI_EVENT_ONE_STA_TX_RATE_T arTxRateResult[UNI_EVENT_ALL_STA_TX_RATE_PER_EVENT];
};

struct GNU_PACKED UNI_EVENT_ONE_STA_TX_STAT_T {
	UINT_16 u2WlanIdx;
	UINT_8  au1Reserved1[2];
	UINT_32 u4TotalTxCount;
	UINT_32 u4TotalTxFailCount;
};

struct GNU_PACKED UNI_EVENT_ALL_STA_TX_STAT_T {
	struct UNI_EVENT_ONE_STA_TX_STAT_T arTxStatResult[UNI_EVENT_ALL_STA_TX_STAT_PER_EVENT];
};

struct GNU_PACKED UNI_EVENT_ONE_STA_TXRX_ADM_STAT_T {
	UINT_16 u2WlanIdx;
	UINT_8  au1Reserved[2];
	UINT_32 au4TxBytesAdm[MAX_ACCESS_CAT_NUM];
	UINT_32 au4RxBytesAdm[MAX_ACCESS_CAT_NUM];
};

struct GNU_PACKED UNI_EVENT_ALL_STA_TXRX_ADM_STAT_T {
	struct UNI_EVENT_ONE_STA_TXRX_ADM_STAT_T arTxRxAdmStatResult[UNI_EVENT_ALL_STA_TXRX_ADM_STAT_PER_EVENT];
};

struct GNU_PACKED UNI_EVENT_ONE_STA_TXRX_AIR_TIME_T {
	UINT_16 u2WlanIdx;
	UINT_8  au1Reserved[2];
	UINT_32 au4WtblTxTime[MAX_ACCESS_CAT_NUM];
	UINT_32 au4WtblRxTime[MAX_ACCESS_CAT_NUM];
};

struct GNU_PACKED UNI_EVENT_ALL_STA_TXRX_AIR_TIME_T {
	struct UNI_EVENT_ONE_STA_TXRX_AIR_TIME_T arTxRxAirTimeResult[UNI_EVENT_ALL_STA_TXRX_AIR_TIME_PER_EVENT];
};

struct GNU_PACKED UNI_EVENT_ONE_STA_DATA_TX_RETRY_COUNT_T {
	UINT_16 u2WlanIdx;
	UINT_8  au1Reserved1[2];
	UINT_16 u2TxRetryCnt;
	UINT_8  au1Reserved2[2];
};

struct GNU_PACKED UNI_EVENT_ALL_STA_DATA_TX_RETRY_COUNT_T {
	struct UNI_EVENT_ONE_STA_DATA_TX_RETRY_COUNT_T arDataTxRetryCountResult[UNI_EVENT_ALL_STA_DATA_TX_RETRY_COUNT_PER_EVENT];
};

struct GNU_PACKED UNI_EVENT_ONE_STA_GI_MODE_T {
	UINT_16 u2WlanIdx;
	UINT_8  au1Reserved1[2];
	UINT_8  u1GiMode;
	UINT_8  au1Reserved2[3];
};

struct GNU_PACKED UNI_EVENT_ALL_STA_GI_MODE_T {
	struct UNI_EVENT_ONE_STA_GI_MODE_T arGiModeResult[UNI_EVENT_ALL_STA_GI_MODE_PER_EVENT];
};

struct GNU_PACKED UNI_EVENT_ONE_STA_TRX_MSDU_COUNT_T {
	UINT_16 u2WlanIdx;
	UINT_8  au1Reserved[2];
	UINT_32 u4TxMsduCnt;
	UINT_32 u4RxMsduCnt;
};

struct GNU_PACKED UNI_EVENT_ALL_STA_TRX_MSDU_COUNT_T {
	struct UNI_EVENT_ONE_STA_TRX_MSDU_COUNT_T arTRxMsduCountResult[UNI_EVENT_ALL_STA_TRX_MSDU_COUNT_PER_EVENT];
};

/* ============== UNI_EVENT_ID_ALL_STA_INFO Start ============ */

/* ============== UNI_EVENT_EPCS START ============ */
struct GNU_PACKED UNI_EVENT_EPCS_T {
	/* fixed field */
	UINT_8 aucPadding[4];

	/* tlv */
	UINT_8 aucTlvBuffer[0];
};

enum UNI_EVENT_EPCS_TAG_T {
	UNI_EVENT_EPCS_RESULT  = 1,
	UNI_EVENT_ID_EPCS_MAX_NUM
};

#define UNI_EVENT_EPCS_RESULT_NUM 64

struct UNI_EVENT_EPCS_RESULT_T {
	/* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
	/* DW_1 */
	UINT_32 u4Cmd;
	UINT_32 au4Result[UNI_EVENT_EPCS_RESULT_NUM];
};

/* ============== UNI_EVENT_EPCS END ============ */

/* ============== UNI_EVENT_BSS_MATCH Start ============ */
struct GNU_PACKED UNI_EVENT_BSS_MATCH_T {
	/* fixed field */
	UINT_8 ucBssIndex;
	UINT_8 aucPadding[3];

	/* tlv */
	UINT_8 aucTlvBuffer[0];
};

enum UNI_EVENT_ID_BSS_MATCH_TAG_T {
	UNI_EVENT_BSS_MATCH_DATA = 0,
	UNI_EVENT_BSS_MATCH_MAX_NUM
};

struct GNU_PACKED UNI_EVENT_BSS_MATCH_DATA_T {
	UINT_16 u2Tag;               /* Tag = 0x00 */
	UINT_16 u2Length;
	UINT_8  ucFail;
	UINT_8  aucPadding[3];
};

/* ============== UNI_EVENT_BSS_MATCH End ============ */

VOID UniCmdResultToRsp(struct cmd_msg *msg);

VOID UniCmdResultRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len);

VOID UniCmdStaRecRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len);

VOID UniEventAccessRegHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

#ifdef AIR_MONITOR
VOID UniEventSmeshInfoRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len);
#endif /* AIR_MONITOR */

#ifdef WIFI_MD_COEX_SUPPORT
VOID UniEventIDCRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len);
#endif

void UniEventEepromHandleEvent(
	struct cmd_msg *msg,
	char  *payload,
	UINT16 payload_len);

void UniEventEepromAccessWrite(
	struct cmd_msg *msg,
	char   *payload,
	UINT16 payload_len);

void UniEventEffuseHandleEvent(
	struct cmd_msg *msg,
	char   *payload,
	UINT16 payload_len);

void UniEventEfuseHandleEventV2(
	struct cmd_msg *msg,
	char   *payload,
	UINT16 payload_len);

void UniEventEfuseAccessWrite(
	struct cmd_msg *msg,
	char   *payload,
	UINT16 payload_len);

#ifdef WIFI_EAP_FEATURE
void UniEventEapHandleEvent(
	struct cmd_msg *msg,
	char *payload,
	UINT16 payload_len);
#endif

VOID UniEventTxPowerUnsolicitHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);

void UniEventTxPowerHandleEvent(
	struct cmd_msg *msg,
	char   *payload,
	UINT16 payload_len);

void UniEventRDDGetIpiHist(
	struct cmd_msg *msg,
	char   *payload,
	UINT16 payload_len);

#ifdef IPI_MAINRADIO_SCAN_SUPPORT
void UniEventRDDGetIpiScan(
	struct cmd_msg *msg,
	char   *payload,
	UINT16 payload_len);
#endif

VOID UniEventSpectrumGetData(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);

void UniEventSpectrumHandler(
	struct cmd_msg *msg,
	char   *payload,
	UINT16 payload_len);

#ifdef DOT11_HE_AX
VOID muruUniEventDispatcher(struct cmd_msg *msg, char *Data, UINT16 Len);
VOID UniEventMuruUnsolicitHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);
#endif /* DOT11_HE_AX */

VOID UniEventSERHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

VOID UniEventMACInfoHandler(struct cmd_msg *msg, char *payload, uint16_t payload_len);

VOID UniEventBandConfigHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

VOID UniEventACKCTSTimeoutHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

VOID UniEventMldHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

#ifdef CFG_SUPPORT_FALCON_TXCMD_DBG
VOID UniEventTxCmdDbgHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);
#endif /* CFG_SUPPORT_FALCON_TXCMD_DBG */

VOID UniEventMibHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

#ifdef SMART_CARRIER_SENSE_SUPPORT
VOID UniEventSCSHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);
#endif /* SMART_CARRIER_SENSE_SUPPORT */

VOID UniEventTPCHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

#ifdef TXBF_SUPPORT
VOID UniEventBfStaRecHandle(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf);

VOID UniEventBFCtrlTxSndHandle(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf);

VOID UniEventBfPlyInfo(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf);

VOID UniEventBFMuMetricInfo(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf);

VOID UniEventBfTxCmdCfgInfo(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf);

VOID UniEventTxBfSndCntInfo(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf);

VOID UniEventBFCtrlFbRptDbgInfoHandle(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf);

VOID UniEventBfPfmuTagReadHandle(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf);

VOID UniEventBfPfmuDataReadHandle(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf);

VOID UniEventBfCalPhase(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf);

void UniEventBFHandleEvent(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);

#endif

VOID UniEventRxFeCompHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);

#ifdef ZERO_PKT_LOSS_SUPPORT
VOID UniEventSCSAHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);
#endif

void UniEventHwcfgRDRsp(
	struct cmd_msg *msg,
	char *payload,
	UINT16 payload_len);

VOID UniEventThemalSensorRsp(
	struct cmd_msg *msg,
	char   *payload,
	UINT16 payload_len
);
VOID UniEventThemalTempAdcRsp(
	struct cmd_msg *msg,
	char *payload,
	UINT16 payload_len
);

VOID UniEventThermalUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);

VOID UniEventNFAvgPwr(
	RTMP_ADAPTER *pAd,
	struct UNI_EVENT_NF_ENABLE_T *Data,
	UINT16 Length);

VOID UniEventNoiseFloorHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);

VOID UniEventTpcUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);

VOID UniEventMecUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);

VOID UniEventTestModeUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);

void UniEventRAHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);

#ifdef CFG_SUPPORT_FALCON_SR
void UniEventSRHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);
#endif/*CFG_SUPPORT_FALCON_SR*/

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
VOID UniEventTWTSyncHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

#ifdef CFG_SUPPORT_CSI
VOID UniEventCSIHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);
#endif /* CFG_SUPPORT_CSI */

VOID uni_event_state_cn_info_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len);

VOID uni_event_state_total_tx_cnt_handler(struct cmd_msg *msg, char *rsp_payload,
						UINT16 rsp_payload_len);

VOID uni_event_state_last_tx_rate_handler(struct cmd_msg *msg, char *rsp_payload,
						UINT16 rsp_payload_len);

#ifdef TXRX_STAT_SUPPORT
VOID uni_event_state_all_rate_cnt_report(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len);
VOID uni_event_state_stbc_cnt_report(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len);
VOID uni_event_state_gi_cnt_report(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len);
#endif /* TXRX_STAT_SUPPORT */

VOID UniEventStateUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);

VOID uni_per_sta_info_multi_rssi_handler(struct cmd_msg *msg,
						char *Data, UINT16 Len);

VOID uni_event_per_sta_multi_tx_rate_handler(struct cmd_msg *msg,
						char *payload, uint16_t payload_len);

VOID uni_event_per_sta_multi_tx_cnt_handler(struct cmd_msg *msg,
						char *payload, uint16_t payload_len);

VOID uni_event_per_sta_multi_pkt_cnt_handler(struct cmd_msg *msg,
						char *payload, uint16_t payload_len);

VOID uni_event_per_sta_rssi_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len);

VOID uni_event_per_sta_contention_rxrate_handler(struct cmd_msg *msg, char *rsp_payload,
							UINT16 rsp_payload_len);

VOID uni_event_multi_sta_contention_rxrate_handler(struct cmd_msg *msg, char *rsp_payload,
							UINT16 rsp_payload_len);

VOID uni_event_per_sta_per_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len);

VOID uni_event_per_sta_snr_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len);

VOID uni_event_per_sta_txrate_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len);

VOID uni_event_per_sta_txcnt_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len);

VOID uni_event_per_sta_pkt_cnt_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len);

VOID uni_event_per_sta_tid_sn_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len);

#ifdef FTM_SUPPORT
VOID UniEventLocHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);
#endif /* FTM_SUPPORT */

VOID UniEventPerStaUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);

VOID UniEventAllStaUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);

VOID UniEventMloMldAlgoHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd);

VOID UniEventUnsolicitMainHandler(VOID *physical_dev, PNDIS_PACKET net_pkt);

/* Here is related to Unsolicit Event */
VOID UniEventFwLog2HostHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

VOID UniEventStaRecUnsolicitHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

VOID UniEventIECountDownHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

VOID UniEventAssertDumpHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

VOID UniEventBeaconTimeoutHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

VOID UniEventPSSyncHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

VOID UniEventECCCalHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

VOID UniEventRDDReportHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

#ifdef WIFI_MD_COEX_SUPPORT
VOID UniEventIDCHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);
#endif

INT32 MT_ATEInsertRddLog(struct _RTMP_ADAPTER *pAd, UCHAR *log, UINT32 log_type, UINT32 len);

VOID UniEventVowHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

void UniEventStaRecEvent(struct cmd_msg *msg,	char *payload, UINT16 payload_len);

VOID UniEventCmdPollTxRxIdleRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len);

VOID UniEventTwtAgrtMgmtRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len);

void UniEventTrParamHandleEvent(struct cmd_msg *msg, char *payload, uint16_t payload_len);

void UniEventTestmodeRxStat(struct cmd_msg *msg, char *payload, uint16_t payload_len);

void UniEventTestEngineHandleEvent(struct cmd_msg *msg, char *payload, uint16_t payload_len);

void UniEventTestCtrlHandleEvent(struct cmd_msg *msg, char *payload, uint16_t payload_len);
VOID UniEventCmdFastPathCalMICRsp(struct cmd_msg *msg, char *Data, UINT16 Len);
void UniEventUnsolicitTestCtrlHandle(struct _RTMP_ADAPTER *pAd,
	uint8_t *payload, uint32_t payload_len, EVENT_RXD *event_rxd);

VOID UniEventNicCapHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

VOID UniEventEPCSHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

VOID UniEventBssMatchHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

#ifdef ZERO_PKT_LOSS_SUPPORT
VOID uni_event_wcid_skip_tx_status_handler(struct cmd_msg *msg, char *rsp_payload,
						UINT16 rsp_payload_len);
#endif

#endif /* WIFI_UNIFIED_COMMAND */
#endif /* _CMM_FW_UNI_EVENT_H */
