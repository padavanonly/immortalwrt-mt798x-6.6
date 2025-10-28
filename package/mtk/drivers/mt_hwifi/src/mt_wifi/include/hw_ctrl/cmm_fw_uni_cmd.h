/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc
 */



#ifndef _CMM_FW_UNI_CMD_H
#define _CMM_FW_UNI_CMD_H

#ifdef WIFI_UNIFIED_COMMAND
#include "sr_cmd.h"
#ifdef CFG_SUPPORT_FALCON_PP
#include "pp_cmd.h"
#endif /* CFG_SUPPORT_FALCON_PP */
#ifdef FTM_SUPPORT
#include "feature/ftm/loc.h"
#endif


#define UNI_CMD_OPT_BIT_0_ACK        		BIT(0)
#define UNI_CMD_OPT_BIT_1_UNI_CMD    		BIT(1) /* 1: unified command, 0:original cmd */
#define UNI_CMD_OPT_BIT_2_SET_QUERY  		BIT(2) /* 1: set, 0:query */
#define UNI_CMD_OPT_BIT_1_UNI_EVENT  		BIT(1)
#define UNI_CMD_OPT_BIT_2_UNSOLICIT_EVENT   BIT(2)


#define UNI_CMD_MSG_HEADER_LEN (sizeof(UNI_CMD_HEADER) + sizeof(TMAC_TXD_L))
#define UNI_CMD_MAX_INBAND_CMD_LEN (1600 - UNI_CMD_MSG_HEADER_LEN)

#define UNI_CMD_HERA_METRIC_USERS                 4
#define UNI_CMD_BUFFER_MODE_CONTENT_MAX           1572

#define UNICMD_VOW_AIRTIME_QUANTUM_IDX_TOTAL_NUM  8
#define UNICMD_VOW_BWC_GROUP_NUMBER               16
#define UNICMD_VOW_BW_GROUP_QUANTUM_LEVEL_NUM     16

#define RED_INUSE_BITSHIFT					5
#define RED_INUSE_BITMASK					(0x1f)

#define UNI_CMD_PQS_MANUAL_SCS_PARA_CNT 8
#define RESET_SN 0
#define RANDOM_SN 50

#ifdef CFG_BIG_ENDIAN
typedef	union _UNI_CMD_HEADER_0 {
	struct {
		UINT32 cid:16;
		UINT32 length:16;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_0;
#else
typedef union _UNI_CMD_HEADER_0 {
	struct {
		UINT32 length:16;
		UINT32 cid:16;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_0;
#endif

#ifdef CFG_BIG_ENDIAN
typedef	union _UNI_CMD_HEADER_1 {
	struct {
		UINT32 seq_num:8;
		UINT32 frag_num:8;
		UINT32 pkt_type_id:8;
		UINT32 reserved:8;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_1;
#else
typedef union _UNI_CMD_HEADER_1 {
	struct {
		UINT32 reserved:8;
		UINT32 pkt_type_id:8;
		UINT32 frag_num:8;
		UINT32 seq_num:8;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_1;
#endif

#ifdef CFG_BIG_ENDIAN
typedef	union _UNI_CMD_HEADER_2 {
	struct {
		UINT32 option:8;
		UINT32 s2d_index:8;
		UINT32 checksum:16;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_2;
#else
typedef union _UNI_CMD_HEADER_2 {
	struct {
		UINT32 checksum:16;
		UINT32 s2d_index:8;
		UINT32 option:8;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_2;
#endif

#ifdef CFG_BIG_ENDIAN
typedef	union _UNI_CMD_HEADER_3 {
	struct {
		UINT32 reserved:32;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_3;
#else
typedef union _UNI_CMD_HEADER_3 {
	struct {
		UINT32 reserved:32;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_3;
#endif

typedef struct GNU_PACKED _UNI_CMD_HEADER_ {
	UNI_CMD_HEADER_0 header_0;
	UNI_CMD_HEADER_1 header_1;
	UNI_CMD_HEADER_2 header_2;
	UNI_CMD_HEADER_3 header_3;
} UNI_CMD_HEADER;

enum UNI_CMD_TYPE {
	UNI_CMD_ID_DEVINFO            = 0x01, /* Update DEVINFO */
	UNI_CMD_ID_BSSINFO            = 0x02, /* Update BSSINFO */
	UNI_CMD_ID_STAREC_INFO        = 0x03, /* Update STAREC */
	UNI_CMD_ID_EDCA_SET           = 0x04, /* Update EDCA Set */
	UNI_CMD_ID_BAND_CONFIG        = 0x08, /* Band Config */
	UNI_CMD_ID_REPT_MUAR          = 0x09, /* Repeater Mode Muar Config */
	UNI_CMD_ID_NORM_MUAR          = 0x0A, /* Normal Mode Muar Config */
	UNI_CMD_ID_WSYS_CONFIG        = 0x0B, /* WSYS Configuration */
	UNI_CMD_ID_ACCESS_REG         = 0x0D, /* Access Register */
	UNI_CMD_ID_CHIP_CONFIG        = 0x0E, /* Custom Chip Configuration*/
	UNI_CMD_ID_POWER_CTRL         = 0x0F, /* NIC Power control */
	UNI_CMD_ID_CFG_SMESH          = 0x10, /* Smesh Config */
	UNI_CMD_ID_RX_HDR_TRAN        = 0x12, /* Rx header translation */
	UNI_CMD_ID_SER                = 0x13, /* SER */
	UNI_CMD_ID_TWT                = 0x14, /* 80211AX TWT*/
	UNI_CMD_ID_IDC                = 0x17, /* IDC */
	UNI_CMD_ID_ECC_OPER           = 0x18, /* ECC Operation */
	UNI_CMD_ID_RDD_ON_OFF_CTRL    = 0x19, /* RDD On/Off Control */
	UNI_CMD_ID_GET_MAC_INFO       = 0x1A, /* Get MAC info */
	UNI_CMD_ID_TXCMD_CTRL         = 0x1D, /* Txcmd ctrl */
	UNI_CMD_ID_MIB                = 0x22, /* Get MIB counter */
	UNI_CMD_ID_STATISTICS         = 0x23, /* Get Statistics */
	UNI_CMD_ID_SNIFFER_MODE       = 0x24, /* Sniffer Mode */
	UNI_CMD_ID_SR                 = 0x25, /* SR */
	UNI_CMD_ID_SCS                = 0x26, /* SCS */
	UNI_CMD_ID_DVT                = 0x29, /* DVT */
	UNI_CMD_ID_GPIO               = 0x2A, /* GPIO setting*/
	UNI_CMD_ID_TXPOWER            = 0x2B, /* RLM Tx Power */
	UNI_CMD_ID_EFUSE_CONTROL      = 0x2D, /* EFUSE Control */
	UNI_CMD_ID_RA                 = 0x2F, /* RA */
	UNI_CMD_ID_SPECTRUM           = 0x30, /* Spectrum */
	UNI_CMD_ID_MURU               = 0x31, /* MURU*/
	UNI_CMD_ID_TESTMODE_RX_STAT   = 0x32, /* testmode Rx statistic */
	UNI_CMD_ID_BF                 = 0x33, /* BF */
	UNI_CMD_ID_CHAN_SWITCH        = 0x34, /* Channel Switch */
	UNI_CMD_ID_THERMAL            = 0x35, /* Thermal control */
	UNI_CMD_ID_NOISE_FLOOR        = 0x36, /* Noise Floor */
	UNI_CMD_ID_VOW                = 0x37, /* VOW */
	UNI_CMD_ID_PP                 = 0x38, /* PP */
	UNI_CMD_ID_TPC                = 0x39, /* TPC */
	UNI_CMD_ID_MEC                = 0x3A, /* MEC */
	UNI_CMD_ID_FR_TABLE           = 0x40, /* Set Fixed Rate TBL */
	UNI_CMD_ID_TEST_TR_PARAM      = 0x42, /* TR */
	UNI_CMD_ID_TESTMODE_CTRL      = 0x46, /* Test Ctrl */
	UNI_CMD_ID_HANDLE_PRECAL_RESULT = 0x47, /* Pre-Calibration */
	UNI_CMD_ID_CSI                = 0x4A, /* TM CSI control*/
	UNI_CMD_ID_VLAN_CFG           = 0x4B, /* VLAN */
	UNI_CMD_ID_CAL                = 0x4C, /* Cal */
	UNI_CMD_ID_HWCFG_CTRL         = 0x4E, /* HWCFG */
	UNI_CMD_ID_SWACI_CTRL         = 0x4F, /* SWACI */
	UNI_CMD_ID_DYN_WMM_CTRL       = 0x50, /* DYN_WMM */
	UNI_CMD_ID_EAP_CTRL           = 0x51, /* EAP */
	UNI_CMD_ID_LED                = 0x53, /* LED */
	UNI_CMD_ID_FAST_PATH          = 0x54, /* Fast Path */
	UNI_CMD_ID_RRO                = 0x57, /* RRO */
	UNI_CMD_ID_OFF_CH_SCAN_CTRL   = 0x58, /* Off Channel Scan */
	UNI_CMD_ID_MLO                = 0x59,
#ifdef FTM_SUPPORT
	UNI_CMD_ID_LOCATION           = 0x5D, /* FTM (Fine Time Measurement) */
#endif /* FTM_SUPPORT */
	UNI_CMD_ID_STATE              = 0x6C, /* state */
	UNI_CMD_ID_PER_STA_INFO       = 0x6D, /* per STA info */
	UNI_CMD_ID_ALL_STA_INFO       = 0x6E,  /* all STA info */
	UNI_CMD_ID_TRIG_ASSERT        = 0x6F, /* trigger FW coredump */
	UNI_CMD_ID_SANDBOX_CTRL       = 0x73,  /* SANDBOX */
	UNI_CMD_ID_EEPROM_CONTROL     = 0x74, /* EEPROM Control */
	UNI_CMD_ID_3WIRE_EXT          = 0x78,  /* 3WIRE */
	UNI_CMD_ID_EPCS               = 0x79,   /* Set EPCS parameters */
	UNI_CMD_ID_SCSA               = 0x7A,  /* SCSA */
	UNI_CMD_ID_MLD                = 0x82, /* MLD */
	UNI_CMD_ID_PEER_MLD           = 0x83, /* Peer MLD */
	UNI_CMD_ID_SDO                = 0x88, /* SDO */
};

/* ============== Common Structure Part ============== */
typedef struct GNU_PACKED _UNI_CMD_TAG_HANDLE_T {
	UINT64  u8CmdFeature;
	UINT32  u4StructSize;
	UINT32  u4DyncBufSize;
	UINT16  u2TLVNumber;
	VOID 	*pfHandler;
} UNI_CMD_TAG_HANDLE_T, *P_UNI_CMD_TAG_HANDLE_T;

/* ============== UNI_CMD_ID_DEVINFO Begin ============== */
typedef struct GNU_PACKED _UNI_CMD_DEVINFO_T {
	/* fixed field */
	UINT8 ucOwnMacIdx;
	UINT8 ucDbdcIdx;
	UINT8 aucPadding[2];

	/* tlv */
	UINT8 aucTlvBuffer[0];
} UNI_CMD_DEVINFO_T, *P_UNI_CMD_DEVINFO_T;

/* DevInfo information (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_DEVINFO_ACTIVE_T {
    UINT16 u2Tag;                   /* Tag = 0x00 */
    UINT16 u2Length;
    UINT8 ucActive;
    UINT8 aucPadding[1];
    UINT8 aucOwnMacAddr[6];
} UNI_CMD_DEVINFO_ACTIVE_T, *P_UNI_CMD_DEVINFO_ACTIVE_T;

typedef INT32 (*PFN_DEVINFO_ACTIVE_HANDLE)(struct _RTMP_ADAPTER *pAd,
											  UINT8 Active,
											  UINT8 *OwnMacAddr,
											  VOID *pHandle);

/* DevInfo command Tag */
typedef enum _UNI_CMD_DEVINFO_TAG_T {
    UNI_CMD_DEVINFO_ACTIVE = 0,
    UNI_CMD_DEVINFO_MAX_NUM
} UNI_CMD_DEVINFO_TAG_T;

/* ============== UNI_CMD_ID_DEVINFO End ============== */
/* ============== UNI_CMD_ID_TEST_TR_PARAM Begin ============== */
struct GNU_PACKED UNI_CMD_TEST_TR_PARAM_T {
	/*fix field*/
	UINT_8 ucParamNum;
	UINT_8 au1Reserved[3];

	/*tlv */
	UINT_8 aucTlvBuffer[0];/**< the TLVs included in this field:
	*
	*   TAG                          | ID  | structure
	*   -----------------------------|-----|--------------
	*   UNI_CMD_TEST_TR_PARAM_CTRL   | 0x0 | UNI_CMD_TEST_TR_PARAM_CTRL_T
	*/
};

/** testmode tx/rx parameter command TLV List */
enum UNI_CMD_TEST_TR_PARAM_TAG_T {
	UNI_CMD_TEST_TR_PARAM_CTRL = 0,
};

enum ENUM_TR_PARAM_ACTION_T {
	ENUM_TR_PARAM_SET = 0,
	ENUM_TR_PARAM_GET = 1,
};

/* Enum for ucCapConfigBitMask in UniCmdMloConfigOperation */
enum _MLO_CONFIG_CAP {
	MLO_CONFIG_STRBITMAP = BIT(0),
	MLO_CONFIG_AADLINKADDR = BIT(1),
};


#ifdef CONFIG_HW_HAL_OFFLOAD
enum UNI_CMD_ATE_TRX_IDX {
	UNI_ATE_SET_RESERV = 0x0,
	/*MAC*/
	UNI_ATE_TRX,
	UNI_ATE_RX_FILTER,
	UNI_ATE_RX_FILTER_PKT_LEN,
	UNI_ATE_SLOT_TIME,
	UNI_ATE_CLEAN_PERSTA_TXQUEUE,
	UNI_ATE_AMPDU_WTBL,
	UNI_ATE_MU_RX_AID,
	UNI_ATE_PHY_MANUAL_TX,
	/*PHY*/
	UNI_ATE_RX_PATH,
	UNI_ATE_TX_STREAM,
	UNI_ATE_TSSI_STATUS,
	UNI_ATE_DPD_STATUS,
	UNI_ATE_RATE_POWER_OFFSET_ON_OFF,
	UNI_ATE_THERMO_COMP_STATUS,
	UNI_ATE_FREQ_OFFSET,
	UNI_ATE_FAGC_RSSI_PATH,
	UNI_ATE_PHY_STATUS_COUNT,
	UNI_ATE_RXV_INDEX,

	UNI_ATE_ANTENNA_PORT,
	UNI_ATE_THERMAL_ONOFF,
	UNI_ATE_TX_POWER_CONTROL_ALL_RF,
	UNI_ATE_RATE_POWER_OFFSET,
	UNI_ATE_SLT_CMD_TEST,
	UNI_ATE_SKU,
	UNI_ATE_POWER_PERCENTAGE_ON_OFF,
	UNI_ATE_BF_BACKOFF_ON_OFF,
	UNI_ATE_POWER_PERCENTAGE_LEVEL,
	UNI_ATE_FRTBL_CFG,
	UNI_ATE_PREAMBLE_PUNC_ON_OFF,

	UNI_ATE_MAX_NUM,
};

#define UNI_ATE_TRX_FEATURE (1<<UNI_ATE_TRX)
#define UNI_ATE_RX_FILTER_FEATURE (1<<UNI_ATE_RX_FILTER)
#define UNI_ATE_RX_FILTER_PKT_LEN_FEATURE (1<<UNI_ATE_RX_FILTER_PKT_LEN)
#define UNI_ATE_SLOT_TIME_FEATURE (1<<UNI_ATE_SLOT_TIME)
#define UNI_ATE_CLEAN_PERSTA_TXQUEUE_FEATURE (1<<UNI_ATE_CLEAN_PERSTA_TXQUEUE)
#define UNI_ATE_AMPDU_WTBL_FEATURE (1<<UNI_ATE_AMPDU_WTBL)
#define UNI_ATE_MU_RX_AID_FEATURE (1<<UNI_ATE_MU_RX_AID)
#define UNI_ATE_PHY_MANUAL_TX_FEATURE (1<<UNI_ATE_PHY_MANUAL_TX)
#define UNI_ATE_RX_PATH_FEATURE (1<<UNI_ATE_RX_PATH)
#define UNI_ATE_TX_STREAM_FEATURE (1<<UNI_ATE_TX_STREAM)
#define UNI_ATE_TSSI_STATUS_FEATURE (1<<UNI_ATE_TSSI_STATUS)
#define UNI_ATE_DPD_STATUS_FEATURE (1<<UNI_ATE_DPD_STATUS)
#define UNI_ATE_RATE_POWER_OFFSET_ON_OFF_FEATURE (1<<UNI_ATE_RATE_POWER_OFFSET_ON_OFF)
#define UNI_ATE_THERMO_COMP_STATUS_FEATURE (1<<UNI_ATE_THERMO_COMP_STATUS)
#define UNI_ATE_FREQ_OFFSET_FEATURE (1<<UNI_ATE_FREQ_OFFSET)
#define UNI_ATE_FAGC_RSSI_PATH_FEATURE (1<<UNI_ATE_FAGC_RSSI_PATH)
#define UNI_ATE_PHY_STATUS_COUNT_FEATURE (1<<UNI_ATE_PHY_STATUS_COUNT)
#define UNI_ATE_RXV_INDEX_FEATURE (1<<UNI_ATE_RXV_INDEX)
#define UNI_ATE_ANTENNA_PORT_FEATURE (1<<UNI_ATE_ANTENNA_PORT)
#define UNI_ATE_THERMAL_ONOFF_FEATURE (1<<UNI_ATE_THERMAL_ONOFF)
#define UNI_ATE_TX_POWER_CONTROL_ALL_RF_FEATURE (1<<UNI_ATE_TX_POWER_CONTROL_ALL_RF)
#define UNI_ATE_RATE_POWER_OFFSET_FEATURE (1<<UNI_ATE_RATE_POWER_OFFSET)
#define UNI_ATE_SLT_CMD_TEST_FEATURE (1<<UNI_ATE_SLT_CMD_TEST)
#define UNI_ATE_SKU_FEATURE (1<<UNI_ATE_SKU)
#define UNI_ATE_POWER_PERCENTAGE_ON_OFF_FEATURE (1<<UNI_ATE_POWER_PERCENTAGE_ON_OFF)
#define UNI_ATE_BF_BACKOFF_ON_OFF_FEATURE (1<<UNI_ATE_BF_BACKOFF_ON_OFF)
#define UNI_ATE_POWER_PERCENTAGE_LEVEL_FEATURE (1<<UNI_ATE_POWER_PERCENTAGE_LEVEL)
#define UNI_ATE_FRTBL_CFG_FEATURE (1<<UNI_ATE_FRTBL_CFG)
#define UNI_ATE_PREAMBLE_PUNC_FEATURE (1<<UNI_ATE_PREAMBLE_PUNC_ON_OFF)


struct _CMD_SLT_CMD_TEST_T {
	UINT_8 ucBand;
	UINT_8 ucNumOfChannel;
	UINT_8 aucReserved[2];
	UINT_8 aucChannel[0];
};


struct _TEST_TR_PARAM_INFO_T {
	UINT_32 u4Data;

	struct _ATE_TEST_SET_TRX_T rAteSetTrx;
	struct _ATE_TEST_SET_RX_PATH_T rAteSetRxPath;
	struct _ATE_TEST_SET_RX_FILTER_T rAteSetRxFilter;
	struct _ATE_TEST_SET_TX_STREAM_T rAteSetTxStream;
	struct _RF_TEST_ON_OFF_SETTING_T rCfgOnOff;
	struct _CFG_RX_FILTER_PKT_LEN_T rRxFilterPktLen;
	struct _CFG_PHY_SETTING_RXV_IDX_T rSetRxvIdx;
	struct _CFG_PHY_SETTING_RSSI_PATH_T rSetFagcRssiPath;
	struct _RF_TEST_ON_OFF_SETTING_T rPhyStatusCnt;
	struct _CFG_RF_ANT_PORT_SETTING_T rCfgRfAntPortSetting;
	struct _CMD_SLOT_TIME_SET_T rSlotTimeSet;
	struct _CMD_POWER_PWERCENTAGE_LEVEL_SET_T rPowerLevelSet;
	struct _CMD_SLT_CMD_TEST_T rSLTCmdTest;
	struct _ATE_TEST_SET_CLEAN_PERSTA_TXQUEUE_T rAteSetCleanPerStaTxQueue;
	struct _ATE_TEST_SET_AMPDU_WTBL_T rAteSetAmpduWtbl;
	struct _ATE_TEST_FREQOFFSET_T rFreqOffset;
	struct _ATE_TEST_SET_MURX_AID_T set_mu_rx_aid;
	struct _ATE_TEST_SET_PHY_MANUAL_TX set_phy_manual_tx;
	struct _UNI_CMD_TEST_FRTBL_CFG_T rFrTblCfg;
	struct _UIN_CMD_TEST_PREAMBLE_PUNC_CFG_T rPreamblePuncCfg;
};

struct _UNI_CMD_TEST_TR_PARAM_CTRL_T
{
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_16 u2ParamIdx;
	UINT_8 ucDbdcIdx;
	UINT_8 ucAteTestModeEn;
	UINT_8 ucAction;
	UINT_8 aucReserved[3];
	struct _TEST_TR_PARAM_INFO_T Data;
};

typedef INT32 (*PFN_TEST_TR_PARAM_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_TEST_TR_PARAM_CTRL_T *tr_param,
	VOID *pHandle,
	uint64_t u8TestTrParamFeature);

#endif /* CONFIG_HW_HAL_OFFLOAD */
/* ============== UNI_CMD_ID_TEST_TR_PARAM End ============== */

/* ============== UNI_CMD_ID_BSSINFO Begin ============== */
/* BssInfo command Tag */
typedef enum _UNI_CMD_BSSINFO_TAG_T {
	UNI_CMD_BSSINFO_BASIC = 0,
	UNI_CMD_BSSINFO_RA = 1,
	UNI_CMD_BSSINFO_RLM = 2,
	UNI_CMD_BSSINFO_PROTECT = 3,
	UNI_CMD_BSSINFO_BSS_COLOR = 4,
	UNI_CMD_BSSINFO_HE = 5,
	UNI_CMD_BSSINFO_11V_MBSSID = 6,
	UNI_CMD_BSSINFO_BCN_CONTENT = 7,
	UNI_CMD_BSSINFO_BCN_CSA = 8,
	UNI_CMD_BSSINFO_BCN_BCC = 9,
	UNI_CMD_BSSINFO_BCN_MBSSID = 0xA,
	UNI_CMD_BSSINFO_RATE = 0xB,
	UNI_CMD_BSSINFO_WAPI = 0xC,
	UNI_CMD_BSSINFO_SAP = 0xD,
	UNI_CMD_BSSINFO_P2P = 0xE,
	UNI_CMD_BSSINFO_QBSS = 0xF,
	UNI_CMD_BSSINFO_SEC = 0x10,
	UNI_CMD_BSS_INFO_BCN_PROT = 0x11,
	UNI_CMD_BSSINFO_TXCMD = 0x12,
	UNI_CMD_BSSINFO_UAPSD = 0x13,
	UNI_CMD_BSSINFO_WMM_PS_TEST = 0x14,
	UNI_CMD_BSSINFO_POWER_SAVE = 0x15,
	UNI_CMD_BSSINFO_STA_CONNECTED = 0x16,
	UNI_CMD_BSSINFO_IFS_TIME = 0x17,
	UNI_CMD_BSSINFO_STA_IOT = 0x18,
	UNI_CMD_BSSINFO_OFFLOAD_PKT = 0x19,
	UNI_CMD_BSSINFO_MLD = 0x1A,
	UNI_CMD_BSSINFO_STA_PM_DISABLE = 0x1B,
	UNI_CMD_BSSINFO_BCN_BTWT = 0x1C,
	UNI_CMD_BSSINFO_MAX_IDLE_PERIOD = 0x1D,
	UNI_CMD_BSSINFO_BCN_MLT = 0x1F,
	UNI_CMD_BSSINFO_BCN_CRIT_UPD = 0x20,
	UNI_CMD_BSSINFO_MLD_LINK_OP = 0x24,
	UNI_CMD_BSSINFO_BCN_MLO_CSA = 0x25,
	UNI_CMD_BSSINFO_BCN_RECONFIG_ML = 0x26,
	UNI_CMD_BSSINFO_BCN_AT2LM = 0x27,
	UNI_CMD_BSSINFO_MAX_NUM
} UNI_CMD_BSSINFO_TAG_T;

typedef struct GNU_PACKED _UNI_CMD_BSSINFO_T {
    /* fixed field */
    UINT8 ucBssInfoIdx;
    UINT8 aucPadding[3];

	/* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_BSSINFO_T, *P_UNI_CMD_BSSINFO_T;

/* BssInfo basic information (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_BASIC_T {
    UINT16 u2Tag;          /* Tag = 0x00 */
    UINT16 u2Length;
    UINT8  ucActive;
    UINT8  ucOwnMacIdx;
    UINT8  ucHwBSSIndex;
    UINT8  ucDbdcIdx;
    UINT32 u4ConnectionType;
    UINT8  ucConnectionState;
    UINT8  ucWmmIdx;
    UINT8  aucBSSID[6];
    UINT16 u2BcMcWlanidx;  /* indicate which wlan-idx used for MC/BC transmission. */
    UINT16 u2BcnInterval;
    UINT8  ucDtimPeriod;
    UINT8  ucPhyMode;
    UINT16 u2StaRecIdxOfAP;
    UINT16 u2NonHTBasicPhyType;
    UINT8  ucPhyModeExt;  /* WMODE_AX_6G : BIT(0) */
    UINT8  aucPadding[1];
} UNI_CMD_BSSINFO_BASIC_T, *P_UNI_CMD_BSSINFO_BASIC_T;

typedef enum _ENUM_PARAM_MEDIA_STATE_T {
    MEDIA_STATE_CONNECTED = 0,
    MEDIA_STATE_DISCONNECTED,
    MEDIA_STATE_ROAMING_DISC_PREV,  /* transient disconnected state for normal/fast roamming purpose */
    MEDIA_STATE_TO_BE_INDICATED,
    MEDIA_STATE_NUM
} ENUM_PARAM_MEDIA_STATE, *P_ENUM_PARAM_MEDIA_STATE;

typedef INT32 (*PFN_BSSINFO_BASIC_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											UINT32 *offset,
											VOID *pHandle);
/* BssInfo RA information (Tag1) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_RA_T {
	UINT16  u2Tag;                 /* Tag = 0x01 */
	UINT16  u2Length;
	BOOLEAN  fgShortPreamble;
	BOOLEAN  fgTestbedForceShortGI;
	BOOLEAN  fgTestbedForceGreenField;
	UINT8   ucHtMode;
	BOOLEAN  fgSeOff;
	UINT8   ucAntennaIndex;
	UINT16  u2MaxPhyRate;
	UINT8   ucForceTxStream;
	UINT8   ucMaxMcs; /* only EHT use now.@20220825 */
	UINT8   aucPadding[2];
} UNI_CMD_BSSINFO_RA_T, *P_UNI_CMD_BSSINFO_RA_T;

typedef INT32 (*PFN_BSSINFO_RA_HANDLE)(struct _RTMP_ADAPTER *pAd,
										struct _BSS_INFO_ARGUMENT_T *bss_info,
										VOID *pHandle);
/* BssInfo RLM information (Tag2) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_RLM_T {
	UINT16 u2Tag;  /* Tag = 0x02 */
	UINT16 u2Length;
	UINT8  ucPrimaryChannel;
	UINT8  ucCenterChannelSeg0;
	UINT8  ucCenterChannelSeg1;
	UINT8  ucBandwidth;
	UINT8  ucTxStream;
	UINT8  ucRxStream;
	UINT8  ucHtOpInfo1; /* for mobile segment */
	UINT8  ucSCO;    /* for mobile segment */
	UINT8  ucRfBand;
	UINT8  ucHetbRU26Disable;
	UINT8  ucPaddings[2];
} UNI_CMD_BSSINFO_RLM_T, *P_UNI_CMD_BSSINFO_RLM_T;

typedef INT32 (*PFN_BSSINFO_RLM_HANDLE)(struct _RTMP_ADAPTER *pAd,
										struct _BSS_INFO_ARGUMENT_T *bss_info,
										VOID *pHandle);

/* BssInfo protection information (Tag3) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_PROT_T {
    UINT_16 u2Tag;  /* Tag = 0x03 */
    UINT_16 u2Length;
    UINT_32 u4ProtectMode;
} UNI_CMD_BSSINFO_PROT_T, *P_UNI_CMD_BSSINFO_PROT_T;

typedef INT32 (*PFN_BSSINFO_PROT_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo bss color information (Tag4) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_BSS_COLOR_T {
    UINT16 u2Tag;  /* Tag = 0x4 */
    UINT16 u2Length;
    BOOLEAN fgEnable;
    UINT8  ucBssColor;
    UINT8  aucPadding[2];
} UNI_CMD_BSSINFO_BSS_COLOR_T, *P_UNI_CMD_BSSINFO_BSS_COLOR_T;

typedef INT32 (*PFN_BSSINFO_BSS_COLOR_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												VOID *pHandle);

/* BssInfo HE information (Tag5) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_HE_T {
    UINT16 u2Tag;  /* Tag = 0x05 */
    UINT16 u2Length;
    UINT16 u2TxopDurationRtsThreshold;
    UINT8  ucDefaultPEDuration;
    BOOLEAN fgErSuDisable; /* for mobile segment */
    UINT16 au2MaxNssMcs[3];
    UINT8  aucPadding[2];
} UNI_CMD_BSSINFO_HE_T, *P_UNI_CMD_BSSINFO_HE_T;

typedef INT32 (*PFN_BSSINFO_HE_HANDLE)(struct _RTMP_ADAPTER *pAd,
										struct _BSS_INFO_ARGUMENT_T *bss_info,
										VOID *pHandle);

/* BssInfo 11v MBSSID information (Tag6) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_11V_MBSSID_T {
	UINT16 u2Tag;  /* Tag = 0x06 */
	UINT16 u2Length;
	UINT8  ucMaxBSSIDIndicator;
	UINT8  ucMBSSIDIndex;
	UINT8  ucTxBssOwnMacIdx; /* The omac index of the transmission bss. */
	UINT8  aucPadding[1];
} UNI_CMD_BSSINFO_11V_MBSSID_T, *P_UNI_CMD_BSSINFO_11V_MBSSID_T;

typedef INT32 (*PFN_BSSINFO_11V_MBSSID_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												VOID *pHandle);

/* BssInfo BCN/PROB RSP information (Tag7) */
typedef INT32 (*PFN_BSSINFO_BCN_OFFLOAD_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												UINT32 *offset,
												VOID *pHandle);
/* u2Length need to 4-byte alignment with WM's sanity */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_BCN_CONTENT_T {
    UINT16 u2Tag;       /* Tag = 0x07 */
    UINT16 u2Length;
    UINT16 u2TimIeOffset;
    UINT16 u2CsaIeOffset;
    UINT16 u2BccIeOffset;
    UINT8  ucAction;
    UINT8  aucPktContentType;
    UINT16 u2PktLength;
    UINT8  aucPktContent[0];
} UNI_CMD_BSSINFO_BCN_CONTENT_T, *P_UNI_CMD_BSSINFO_BCN_CONTENT_T;

typedef enum _BCN_CONTENT_ACTION_T {
    BCN_ACTION_DISABLE = 0,
    BCN_ACTION_ENABLE = 1,
    UPDATE_PROBE_RSP = 2,
} BCN_CONTENT_ACTION_T, *P_BCN_CONTENT_ACTION_T;

typedef INT32 (*PFN_BSSINFO_BCN_CONTENT_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct _BSS_INFO_ARGUMENT_T *bss_info,
													VOID *pHandle);

/* BssInfo BCN CSA information (Tag8) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_BCN_CSA_T {
	UINT16 u2Tag; /* Tag = 0x08 */
	UINT16 u2Length;
	UINT8 ucCsaCount;
	BOOLEAN fgStaticPunctureCsa;
	UINT8  aucPadding[2];
} UNI_CMD_BSSINFO_BCN_CSA_T, *P_UNI_CMD_BSSINFO_BCN_CSA_T;

/* BssInfo BCN CSA information on MLO (Tag37) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_BCN_MLO_CSA_T {
	UINT16 u2Tag;	/* Tag = 0x25 */
	UINT16 u2Length;
	UINT16 u2MloCsaIeOffset;
	UINT_8 ucMloCsaBssIdx;
	UINT_8 aucPktContent[9];
} UNI_CMD_BSSINFO_BCN_MLO_CSA_T, *P_UNI_CMD_BSSINFO_BCN_MLO_CSA_T;

typedef INT32 (*PFN_BSSINFO_BCN_CSA_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												VOID *pHandle);


/* BssInfo BCN BCC information (Tag9) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_BCN_BCC_T {
    UINT16 u2Tag;       /* Tag = 0x9 */
    UINT16 u2Length;
    UINT8  ucBccCount;
    UINT8  aucPadding[3];
} UNI_CMD_BSSINFO_BCN_BCC_T, *P_UNI_CMD_BSSINFO_BCN_BCC_T;

typedef INT32 (*PFN_BSSINFO_BCN_BCC_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												VOID *pHandle);

/* BssInfo BCN Mbssid-index ie information (Tag10) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_BCN_MBSSID_T {
	UINT16 u2Tag;       /* Tag = 0xA */
	UINT16 u2Length;
	UINT32 u4Dot11vMbssidBitmap;
	UINT16 u2MbssidIeOffset[32]; /* WM is Fix Len 32 */
} UNI_CMD_BSSINFO_BCN_MBSSID_T, *P_UNI_CMD_BSSINFO_BCN_MBSSID_T;

typedef INT32 (*PFN_BSSINFO_BCN_MBSSID_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												VOID *pHandle);

/* BssInfo RATE information (Tag11) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_RATE_T {
	UINT16 u2Tag;  /* Tag = 0x0B */
	UINT16 u2Length;
	UINT16 u2OperationalRateSet; /* for mobile segment */
	UINT16 u2BSSBasicRateSet;    /* for mobile segment */
	UINT16 u2BcRate; /* for WA */
	UINT16 u2McRate; /* for WA */
	UINT8  ucPreambleMode; /* for WA */
	UINT8  ucBcFixedRateIdx;
	UINT8  ucMcFixedRateIdx;
	UINT8  ucMloCPFixedRateIdx; /* for mlo critical packet */
	UINT8  ucHighPriFalg;
	UINT8  ucHighPriFRIdx[HIGHPRI_MAX_TYPE];
	UINT8  aucPadding[3];
} UNI_CMD_BSSINFO_RATE_T, *P_UNI_CMD_BSSINFO_RATE_T;

typedef INT32 (*PFN_BSSINFO_RATE_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo WAPI information (Tag12) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_WAPI_T {
    UINT16 u2Tag;  /* Tag = 0x0C */
    UINT16 u2Length;
    BOOLEAN fgWapiMode;
    UINT8  aucPadding[3];
} UNI_CMD_BSSINFO_WAPI_T, *P_UNI_CMD_BSSINFO_WAPI_T;

typedef INT32 (*PFN_BSSINFO_WAPI_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo SAP information (Tag13) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_SAP_T {
    UINT16 u2Tag;  /* Tag = 0x0D */
    UINT16 u2Length;
    BOOLEAN fgIsHiddenSSID;
    UINT8  aucPadding[2];
    UINT8  ucSSIDLen;
    UINT8  aucSSID[32];
} UNI_CMD_BSSINFO_SAP_T, *P_UNI_CMD_BSSINFO_SAP_T;

typedef INT32 (*PFN_BSSINFO_SAP_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo P2P information (Tag14) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_P2P_T {
    UINT16 u2Tag;  /* Tag = 0x0E */
    UINT16 u2Length;
    UINT32  u4PrivateData;
} UNI_CMD_BSSINFO_P2P_T, *P_UNI_CMD_BSSINFO_P2P_T;

typedef INT32 (*PFN_BSSINFO_P2P_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo QBSS information (Tag15) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_QBSS_T {
    UINT16 u2Tag;  /* Tag = 0x0F */
    UINT16 u2Length;
    UINT8  ucIsQBSS;
    UINT8  aucPadding[3];
} UNI_CMD_BSSINFO_QBSS_T, *P_UNI_CMD_BSSINFO_QBSS_T;

typedef INT32 (*PFN_BSSINFO_QBSS_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo Security information (Tag16) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_SEC_T {
    UINT16 u2Tag;  /* Tag = 0x10 */
    UINT16 u2Length;
    UINT8  ucAuthMode;/**<
						*     Auth Mode             | Value | Note          |
						*     --------------------  | ------|-------------- |
						*     AUTH_MODE_OPEN        | 0     | -             |
						*     AUTH_MODE_SHARED      | 1     | Shared key    |
						*     AUTH_MODE_AUTO_SWITCH | 2     | Either open system or shared key |
						*     AUTH_MODE_WPA         | 3     | -             |
						*     AUTH_MODE_WPA_PSK     | 4     | -             |
						*     AUTH_MODE_WPA_NONE    | 5     | For Ad hoc    |
						*     AUTH_MODE_WPA2        | 6     | -             |
						*     AUTH_MODE_WPA2_PSK    | 7     | -             |
						*     AUTH_MODE_WPA2_FT     | 8     | 802.11r       |
						*     AUTH_MODE_WPA2_FT_PSK | 9     | 802.11r       |
						*     AUTH_MODE_WPA_OSEN    | 10    | -             |
						*     AUTH_MODE_WPA3_SAE    | 11    | -             |
						*/
    UINT8  ucEncStatus;
    UINT8  ucCipherSuit;
    UINT8  aucPadding[1];
} UNI_CMD_BSSINFO_SEC_T, *P_UNI_CMD_BSSINFO_SEC_T;

typedef INT32 (*PFN_BSSINFO_SEC_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo BCN Prot. information (Tag 0x11) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_BCN_PROT_T {
    UINT16 u2Tag; /* Tag = 0x11 */
    UINT16 u2Length;
    UINT8 aucBcnProtPN[LEN_WPA_TSC];
    UINT8 ucBcnProtEnabled;  /* 0: off, 1: SW mode, 2:HW mode */
    UINT8 ucBcnProtCipherId;
    UINT8 aucBcnProtKey[LEN_MAX_BIGTK];
    UINT8 ucBcnProtKeyId;
    UINT8 aucReserved[3];
} UNI_CMD_BSSINFO_BCN_PROT_T, *P_UNI_CMD_BSSINFO_BCN_PROT_T;

typedef INT32 (*PFN_BSSINFO_BCN_PROT_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												VOID *pHandle);

/* TxCMD Mode information (Tag 0x12) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_TXCMD_T {
    UINT_16 u2Tag;  /* Tag = 0x12 */
    UINT_16 u2Length;
    BOOLEAN fgUseTxCMD;
    UINT_8  aucPadding[3];
} UNI_CMD_BSSINFO_TXCMD_T, *P_UNI_CMD_BSSINFO_TXCMD_T;

typedef INT32 (*PFN_BSSINFO_TXCMD_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo UAPSD information (Tag 0x13) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_UAPSD_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8  ucBmpDeliveryAC;
    UINT8  ucBmpTriggerAC;
    UINT8  aucPadding[2];
} UNI_CMD_BSSINFO_UAPSD_T, *P_UNI_CMD_BSSINFO_UAPSD_T;

typedef INT32 (*PFN_BSSINFO_UAPSD_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo WMM PS test information (Tag 0x14) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_WMM_PS_TEST_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8  ucIsEnterPsAtOnce;
    UINT8  ucIsDisableUcTrigger;
    UINT8  aucPadding[2];
} UNI_CMD_BSSINFO_WMM_PS_TEST_T, *P_UNI_CMD_BSSINFO_WMM_PS_TEST_T;

typedef INT32 (*PFN_BSSINFO_WMM_PS_TEST_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct _BSS_INFO_ARGUMENT_T *bss_info,
													VOID *pHandle);

/* BssInfo Power Save information (Tag 0x15) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_POWER_SAVE_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8  ucPsProfile;/**<
						*     Power Save Mode                 | Value | Note                     |
						*     --------------------            | ------|--------------            |
						*     ENUM_PSP_CONTINUOUS_ACTIVE      | 0     | Leave power save mode    |
						*     ENUM_PSP_CONTINUOUS_POWER_SAVE  | 1     | Enter power save mode    |
						*     ENUM_PSP_FAST_SWITCH            | 2     | Fast switch mode         |
						*     ENUM_PSP_TWT                    | 3     | twt                      |
						*     ENUM_PSP_TWT_SP                 | 4     | twt sp                   |
						*/
    UINT8  aucPadding[3];
} UNI_CMD_BSSINFO_POWER_SAVE_T, *P_UNI_CMD_BSSINFO_POWER_SAVE_T;

typedef INT32 (*PFN_BSSINFO_POWER_SAVE_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct _BSS_INFO_ARGUMENT_T *bss_info,
													VOID *pHandle);

typedef enum _ENUM_POWER_SAVE_PROFILE_T {
    ENUM_PSP_CONTINUOUS_ACTIVE = 0,
    ENUM_PSP_CONTINUOUS_POWER_SAVE,
    ENUM_PSP_FAST_SWITCH,
    ENUM_PSP_TWT,
    ENUM_PSP_TWT_SP,
    ENUM_PSP_NUM
} ENUM_POWER_SAVE_PROFILE_T, *PENUM_POWER_SAVE_PROFILE_T;

/* BssInfo STA connection information (Tag 0x16) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_STA_CONNECTED_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT16 u2BcnInterval;
    UINT8  ucDtimPeriod;
    UINT8  aucPadding[1];
} UNI_CMD_BSSINFO_STA_CONNECTED_T, *P_UNI_CMD_BSSINFO_STA_CONNECTED_T;

typedef INT32 (*PFN_BSSINFO_STA_CONNECTED_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct _BSS_INFO_ARGUMENT_T *bss_info,
													VOID *pHandle);


/* BssInfo IFS time information (Tag 0x17) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_IFS_TIME_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    BOOLEAN fgSlotValid;
    BOOLEAN fgSifsValid;
    BOOLEAN fgRifsValid;
    BOOLEAN fgEifsValid;
    UINT16 u2SlotTime;
    UINT16 u2SifsTime;
    UINT16 u2RifsTime;
    UINT16 u2EifsTime;
} UNI_CMD_BSSINFO_IFS_TIME_T, *P_UNI_CMD_BSSINFO_IFS_TIME_T;

typedef INT32 (*PFN_BSSINFO_IFS_TIME_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												VOID *pHandle);

/* BssInfo Mobile need information (Tag 0x18) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_IOT_T {
    UINT16 u2Tag; /* Tag = 0x18 */
    UINT16 u2Length;
    UINT8 ucIotApBmp;
    UINT8 aucReserved[3];
} UNI_CMD_BSSINFO_IOT_T, *P_UNI_CMD_BSSINFO_IOT_T;

typedef INT32 (*PFN_BSSINFO_IOT_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo frame offload (UPR/FILS) (Tag 0x19) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_OFFLOAD_PKT {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8  ucTxType;
    UINT8  ucTxMode;
    UINT8  ucTxInterval;
    UINT8  fgEnable;
    UINT16 u2Wcid;
    UINT16 u2OffloadPktLength;
    UINT8  aucPktContent[0];
} UNI_CMD_BSSINFO_OFFLOAD_PKT_T, *P_UNI_CMD_BSSINFO_OFFLOAD_PKT_T;

typedef INT32 (*PFN_BSSINFO_OFFLOAD_PKT_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct _BSS_INFO_ARGUMENT_T *bss_info,
													VOID *pHandle);
typedef enum bssinfo_unsolicit_txtype {
    BSSINFO_UNSOLICIT_TX_PROBE_RSP = 0,
    BSSINFO_UNSOLICIT_TX_FILS_DISC = 1,
    BSSINFO_UNSOLICIT_TX_QOS_NULL  = 2 /* packet injector */
} bssinfo_unsolicit_txtype_t;

/* BssInfo MLD information (Tag 0x1A) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_MLD_T {
	UINT16 u2Tag;   /* Tag = 0x1A */
	UINT16 u2Length;
	UINT8  ucGroupMldId;
	UINT8  ucOwnMldId; /* for UMAC, 0~ 63 */
	UINT8  aucOwnMldAddr[MAC_ADDR_LEN];
	/* for AGG: 0~15, 0xFF means this is a legacy BSS, no need to do remapping */
	UINT8  ucOmRemapIdx;
	UINT_8 ucLinkId;
	UINT_8 aucReserved[2];
} UNI_CMD_BSSINFO_MLD_T, *P_UNI_CMD_BSSINFO_MLD_T;

/* BssInfo BCN BTWT ie information (Tag 0x1C) */
struct GNU_PACKED UNI_CMD_BSSINFO_BCN_BTWT_T {
	UINT16 u2Tag; /* Tag = 0x9 */
	UINT16 u2Length;
	UINT16 u2BTWTIeOffset;
	UINT8  aucPadding[2];
};

/* BssInfo BCN Mbssid-index ie information (Tag 0x1E) */
struct GNU_PACKED UNI_CMD_BSSINFO_BCN_MLT_T {
	UINT16 u2Tag;       /* Tag = 0x1C */
	UINT16 u2Length;
	UINT16 u2MltIeOffset;
	UINT16 u2Reserved;
	UINT32 u4Dot11vMbssidBitmap;
	UINT16 u2MbssidMltIeOffset[32]; /* WM is Fix Len 32 */
};

typedef INT32 (*PFN_BSSINFO_MLD_HANDLE)(struct _RTMP_ADAPTER *pAd,
										struct _BSS_INFO_ARGUMENT_T *bss_info,
										VOID *pHandle);

/* BssInfo BCN BSS Parameter Critical Update (Tag 0x20) */
struct GNU_PACKED UNI_CMD_BSSINFO_BCN_CRIT_UPD_T {
	UINT16 u2Tag;       /* Tag = 0x20 */
	UINT16 u2Length;
	UINT32 u4UpdatedBSSBitmap;
	UINT32 u4BypassSeqHandleBmap;
	UINT16 u2TimIeOffset[32];
	UINT16 u2CapInfoOffset[32];
};

typedef INT32 (*PFN_BSSINFO_BCN_CRIT_UPD_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												VOID *pHandle);

#define EHT_MLD_OP_DELETE_LINK 0
#define EHT_MLD_OP_ADD_LINK 1
/* BssInfo MLD Link Operation (Tag 0x24) */
struct GNU_PACKED UNI_CMD_BSSINFO_MLD_LINK_OP_T {
	UINT16 u2Tag;   /* Tag = 0x24 */
	UINT16 u2Length;
	UINT8  ucGroupMldId;
	UINT8  ucOwnMldId; /* for UMAC, 0~ 63 */
	UINT8  aucOwnMldAddr[MAC_ADDR_LEN];
	/* for AGG: 0~15, 0xFF means this is a legacy BSS, no need to do remapping */
	UINT8  ucOmRemapIdx;
	/* 0: Delete link, 1: Add link*/
	UINT8  ucLinkOperation;
	UINT8  aucReserved[2];
};

/* BssInfo BCN Reconfiguration Multi-link IE - AP Removal Timer (Tag 0x26) */
struct GNU_PACKED UNI_CMD_BSSINFO_BCN_RECONFIG_ML_T {
	UINT16 u2Tag;       /* Tag = 0x26 */
	UINT16 u2Length;
	UINT8  ucOffsetNum;
	UINT8  aucReserved[3];
	UINT8  aucOffsetInfo[0]; /* struct UNI_CMD_BSSINFO_RECONF_ML_OFFSET_INFO_T */
};

struct GNU_PACKED UNI_CMD_BSSINFO_RECONF_ML_OFFSET_INFO_T {
	UINT16 u2ApRmTmrOffset;	/* AP Removal Timer in Per-STA Profile */
	UINT8  ucBssIdx;	/* reference BSS to set Timer TBTT */
	UINT8  aucReserved[1];
};

/* BssInfo BCN AT2LM Expected duration (Tag 0x27) */
struct GNU_PACKED UNI_CMD_BSSINFO_BCN_AT2LM_T {
	UINT16 u2Tag;       /* Tag = 0x27 */
	UINT16 u2Length;
	UINT8 ucVldIdBitmap;
	UINT8 Rsvd;
	UINT16 u2T2lmIeOffset[BSS_MNGR_MAX_AT2LM_SET_NUM];
};

/* ============== UNI_CMD_ID_BSSINFO End ============== */

/* ============== UNI_CMD_ID_MLD Begin ============== */
enum UNI_CMD_MLD_TAG_T {
	UNI_CMD_MLD_AT2LM_RES_REQ = 0x02,
	UNI_CMD_MLD_RECONFIG_TMR = 0x03,
	UNI_CMD_MLD_MLR_RM_LINK = 0x04,
};

struct GNU_PACKED UNI_CMD_MLD_T {
	/* fixed field */
	UINT8 ucCmdVer;
	UINT8 aucMldAddr[MAC_ADDR_LEN];
	UINT8 ucFwMldIdx;
	UINT8 ucFlag;
	UINT8 ucRsvd[3];
	/* tlv */
	UINT8 aucTlvBuffer[0];
};

/* MLD advertised t2lm resource request (Tag=0x02) */
struct GNU_PACKED UNI_CMD_MLD_AT2LM_T {
	UINT16 u2Tag;			/* 0x02 */
	UINT16 u2Length;
	UINT8 ucAt2lmId;
	UINT8 ucBssInfoIdx;
	UINT8 ucEnMstTimer;
	UINT8 ucEnETimer;
	UINT16 u2MstTimerAdvTime;	/* ms */
	UINT16 u2ETimerAdvTime;		/* ms */
	UINT32 u4MSTDur;		/* us */
	UINT32 u4EDur;			/* us */
	UINT16 u2DisLinkidBitmap; /*disable link_id in bitmap*/
	UINT8 uDisLinkidBssInfoIdx[MLD_LINK_MAX]; /*bss info index of disable link_id*/
};

/* MLD Reconfiguration Timer Register (Tag=0x03) */
struct GNU_PACKED UNI_CMD_MLD_RECONFIG_TMR_T {
	UINT16 u2Tag;	/* 0x03 */
	UINT16 u2Length;
	UINT16 u2VldLinkIdBmap;
	/* Number of seconds before reconfiguration of each link*/
	UINT16 au2NumSeconds[CFG_WIFI_RAM_BAND_NUM];
	UINT8  aucLinkBssIdx[CFG_WIFI_RAM_BAND_NUM];
	UINT8  aucReserved[1];
};

/* MLD Reconfiguration: provide removal of bss link of
 * ap mld to handle removal of sta link of peer mld (Tag=0x04) */
struct GNU_PACKED UNI_CMD_MLD_RECONFIG_RM_LINK_T {
	UINT16 u2Tag;			/* 0x04 */
	UINT16 u2Length;
	UINT16 u2RmLinkIdBitmap; /* protocol: link id bitmap */
	UINT8 Rsvd[2];
	UINT8 aucLinkIdBssInfoIdx[MLD_LINK_MAX]; /* link id's BssInfoIdx: 16 sets*/
};

/* ============== UNI_CMD_ID_MLD End ============== */

/* ============== UNI_CMD_ID_PEER_MLD Begin ============== */
enum UNI_CMD_PEER_MLD_TAG_T {
	UNI_CMD_PEER_MLD_NT2LM_REQ = 0x00,
};

struct GNU_PACKED UNI_CMD_PEER_MLD_T {
	/* fixed field */
	UINT8 ucCmdVer;
	UINT8 aucMldAddr[MAC_ADDR_LEN];	/* peer mld addr */
	UINT8 ucMldIdx;
	UINT8 ucRsvd[4];
	/* tlv */
	UINT8 aucTlvBuffer[0];
};

/* Peer MLD Nego. t2lm request (Tag=0x00) */
struct GNU_PACKED UNI_CMD_PEER_MLD_NT2LM_T {
	UINT16 u2Tag;			/* 0x00 */
	UINT16 u2Length;
	UINT8 aucMldAddr[MAC_ADDR_LEN]; /* self mld addr */
	UINT16 u2LinkidBitmap;		/* which link id is valid */
	UINT16 au2Wcid[MLD_LINK_MAX];	/* link id to wcid mapping */
	UINT8 aucTidmapDl[MLD_LINK_MAX];
	UINT8 aucTidmapUl[MLD_LINK_MAX];
};
/* ============== UNI_CMD_ID_PEER_MLD End ============== */


/* ============== UNI_CMD_ID_STAREC_INFO Begin ======== */
INT32 UniCmdWtblUpdate(RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT8 ucOperation,
							VOID *pBuffer, UINT32 u4BufferLen);

/* Common part of CMD_STAREC */
struct GNU_PACKED UNI_CMD_STAREC_INFO_T {
    /* Fixed field*/
    UINT8 ucBssInfoIdx;
    UINT8 ucWlanIdxL;
    UINT16 u2TotalElementNum;
    UINT8 ucAppendCmdTLV;
    UINT8 ucMuarIdx;
    UINT8 ucWlanIdxHnVer;
    UINT8 aucPadding[1];

    /* TLV */
    UINT8 aucTlvBuffer[0];
};

struct _UNI_CMD_PER_STA_TID_SN_SET_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2StaTidSnPairNum;
	UINT8 ucUpdateType;   // 0: set SN to current SN + delta. 1 : Set SN by driver
	UINT8 ucReserved;
	UINT8 aucPairContent[0];
};

struct GNU_PACKED UNI_CMD_STAREC_PSM_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	BOOLEAN fgIsPsm;
	UINT8 aucReserved[3];
};

typedef struct GNU_PACKED _UNI_CMD_STAREC_WTBL_T {
	/* WTBL with STAREC update (Tag 0x0b) */
	/* STAREC format, content is WTBL format. */
	UINT16	u2Tag;
	UINT16	u2Length;
	UINT8	aucBuffer[MAX_BUF_SIZE_OF_WTBL_INFO];
} UNI_CMD_CMD_STAREC_WTBL_T, *P_UNI_CMD_CMD_STAREC_WTBL_T;

typedef struct GNU_PACKED _UNI_CMD_STAREC_HE_INFO_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT32 u4HeCap;
    UINT8  ucTrigerFrameMacPadDuration;
    UINT8  ucMaxAmpduLenExponentExtension;
    UINT8  ucChBwSet;
    UINT8  ucDeviceClass;
    UINT8  ucDcmTxMode;
    UINT8  ucDcmTxMaxNss;
    UINT8  ucDcmRxMode;
    UINT8  ucDcmRxMaxNss;
    UINT8  ucDcmMaxRu;
    UINT8  ucPuncPreamRx;
    UINT8  ucPktExt;
    UINT8  ucMaxAmpduLenExponent;
    /*0: BW80, 1: BW160, 2: BW8080*/
    UINT16 au2MaxNssMcs[3];
    UINT8  aucReserve1[2];
} UNI_CMD_STAREC_HE_INFO_T, *P_UNI_CMD_STAREC_HE_INFO_T;

typedef struct GNU_PACKED _UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T {
	UINT16 u2WlanIndex;      /* MLO */
	UINT8  ucMgmtProtection;
	UINT8  ucCipherId;
	UINT8  ucSubLength;      /* Length = total cipher subcmd structure size */
	UINT8  ucKeyIdx;         /* keyid 4,5 for IGTK; 6,7 for BIGTK */
	UINT8  ucKeyLength;
	UINT8  fgNeedRsp;
	UINT8  aucKeyMaterial[32];
	UINT8  aucBipn[LEN_WPA_TSC];
	UINT8  ucBcnProtMode;    /* 0: HW mode, 1: SW mode*/
	UINT8  ucReserve[1];
} UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T, *P_UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T;

typedef INT32 (*PFN_STAREC_HANDLE)(struct _RTMP_ADAPTER *pAd,
									STA_REC_CFG_T *StaRecCfg,
									UNI_CMD_TAG_HANDLE_T *pTagHandle,
									VOID *pHandle);

/*  STA record TLV tag */
typedef enum _UNI_CMD_STAREC_TAG_T {
	UNI_CMD_STAREC_BASIC               	= 0x00,
	UNI_CMD_STAREC_RA                  	= 0x01,
	/* UNI_CMD_STAREC_RA_COMMON_INFO      = 0x02, */
	UNI_CMD_STAREC_RA_UPDATE           	= 0x03,
	UNI_CMD_STAREC_BF                  	= 0x04,
	UNI_CMD_STAREC_MAUNAL_ASSOC        	= 0x05,
	UNI_CMD_STAREC_BA                  	= 0x06,
	UNI_CMD_STAREC_STATE_CHANGED       	= 0x07,
	UNI_CMD_STAREC_TX_PROC                   = 0x08,
	UNI_CMD_STAREC_HT_BASIC            	= 0x09,
	UNI_CMD_STAREC_VHT_BASIC           	= 0x0a,
	UNI_CMD_STAREC_AP_PS               	= 0x0b,
	UNI_CMD_STAREC_INSTALL_KEY         	= 0x0c,
	UNI_CMD_STAREC_WTBL                	= 0x0d, /* deprecated */
	UNI_CMD_STAREC_HE_BASIC_OLD            	= 0x0e, /* deprecated */
	UNI_CMD_STAREC_HW_AMSDU                 = 0x0f,
	UNI_CMD_STAREC_AAD_OM                   = 0x10,
	UNI_CMD_STAREC_INSTALL_KEY_V2           = 0x11,
	UNI_CMD_STAREC_MURU                     = 0x12,
	UNI_CMD_STAREC_BFEE                	= 0x14,
	UNI_CMD_STAREC_PHY_INFO                 = 0x15,
	UNI_CMD_STAREC_BA_OFFLOAD               = 0x16,
	UNI_CMD_STAREC_HE_6G_CAP       	        = 0x17,
	UNI_CMD_STAREC_INSTALL_DEFAULT_KEY      = 0x18,
	UNI_CMD_STAREC_HE_BASIC                 = 0x19,
	UNI_CMD_STAREC_MLD_SETUP                = 0x20,
	UNI_CMD_STAREC_EHT_MLD                  = 0x21,
	UNI_CMD_STAREC_EHT_BASIC                = 0x22,
	UNI_CMD_STAREC_MLD_TEARDOWN             = 0x23,
	UNI_CMD_STAREC_SEC_PN_INFO              = 0x26,
	UNI_CMD_STAREC_HDRT                     = 0x28,
	UNI_CMD_STAREC_EML_OP                   = 0x29,
	UNI_CMD_STAREC_PSM						= 0x2a,
	UNI_CMD_STAREC_HDR_TRANS                = 0x2b,
	UNI_CMD_STA_REC_MLR_INFO				= 0x2d,
	UNI_CMD_STA_REC_TX_CAP					= 0x2f,
	UNI_CMD_STA_REC_TX_PWR                  = 0x3a,
	UNI_CMD_STA_REC_DUAL_CTS                = 0x3b,
	UNI_CMD_STA_REC_RCA1			= 0x41,
	UNI_CMD_STAREC_MAX_NUM
} UNI_CMD_STAREC_TAG_T;

/* Update Aadom Info (0x10)*/
struct GNU_PACKED UNI_CMD_STAREC_AADOM_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucAadOm;
	UINT8 aucReserve[3];
};

typedef struct GNU_PACKED _UNI_CMD_STAREC_PHY_INFO_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2BSSBasicRateSet;
	UINT8 ucDesiredPhyTypeSet;
	UINT8 ucAmpduParam;
	UINT8 ucRtsPolicy;
	UINT8 ucRCPI;
	UINT8 ucVhtMaxAmpduLen;
	UINT8 aucReserve[1];
} UNI_CMD_STAREC_PHY_INFO_T, *P_UNI_CMD_STAREC_PHY_INFO_T;

/* Update HE 6g Info */
typedef struct GNU_PACKED _CMD_STAREC_HE_6G_CAP_T {
    UINT16      u2Tag;
    UINT16      u2Length;
    UINT16      u2He6gBandCapInfo;
    UINT8       aucReserve[2];
} CMD_STAREC_HE_6G_CAP_T, *P_CMD_STAREC_HE_6G_CAP_T;

/** @addtogroup UNI_CMD_ID_STAREC_INFO
 * @{
 */
/**
 * This structure is used for UNI_CMD_STA_REC_HE_BASIC(0x19) of UNI_CMD_ID_STAREC_INFO command (0x03)
 * to set recieved HE capability information.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag                 should be 0x19
 * @param[in] u2Length              the length of this TLV, should be sizeof(UNI_CMD_STAREC_HE_BASIC_T)
 * @param[in] aucHeMacCapInfo       The HE MAC capability information field of received HE CAP IE
 * @param[in] aucHePhyCapInfo       The HE PHY capability information field of received HE CAP IE
 * @param[in] ucPktExt              The packet extension
 * @param[in] au2RxMaxNssMcs          The max nss mcs
 *                                  - au2RxMaxNssMcs[0] for BW80
 *                                  - au2RxMaxNssMcs[1] for BW160
 *                                  - au2RxMaxNssMcs[2] for BW8080
 */
/* Update HE Cap Info (0x19)*/
struct CMD_STAREC_HE_BASIC_T {
    UINT16     u2Tag;
    UINT16     u2Length;

    UINT8      aucHeMacCapInfo[6];
    UINT8      aucHePhyCapInfo[11];
    UINT8      ucPktExt;
    /*0: BW80, 1: BW160, 2: BW8080*/
    UINT16     au2RxMaxNssMcs[3];
};
/** @} */

#define STAREC_MLD_SETUP_EXTRAINFO_MTK_MLO	BIT(0)
#define STAREC_MLD_SETUP_EXTRAINFO_DRAFT1DOT2	BIT(1)

/** @addtogroup UNI_CMD_ID_STAREC_INFO
 * @{
 */
/**
 * This structure is used for STA_REC_MLD_SETUP(0x20) of UNI_CMD_ID_STAREC_INFO
 * command (0x03)
 * to setup mld starec.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag                   should be 0x20
 * @param[in] u2Length                the length of this TLV, should be
 *				      sizeof(CMD_STAREC_MLD_SETUP_T)
 * @param[in] aucPeerMldAddr          the peer mld address
 * @param[in] u2PrimaryMldId          primary mld id
 * @param[in] u2SecondMldId           secondary mld id
 * @param[in] u2SetupWlanId           the wlan id of setup link
 * @param[in] ucLinkNumber            total link number of this MLD STAREC
 * @param[in] aucLinkInfo             the CMD_STAREC_LINK_INFO_T array for
 *                                    wlanidx and bssIdx for each link
 */
/* mld starec setup (Tag 0x20) */
struct CMD_STAREC_MLD_SETUP_T {
	UINT16  u2Tag;                 /* Tag = 0x20 */
	UINT16  u2Length;
	UINT8   aucPeerMldAddr[MAC_ADDR_LEN];
	UINT16  u2PrimaryMldId;
	UINT16  u2SecondMldId;
	UINT16  u2SetupWlanId;
	UINT8   ucLinkNumber;
	UINT8	ucExtraInfo;
	UINT8   audPaddings[2];
	UINT8   aucLinkInfo[0];
};
/** @} */

struct CMD_STAREC_LINK_INFO_T {
	UINT16  u2WlanIdx;
	UINT8   ucBssIdx;
	UINT8   aucPadding[1];
};

/** @addtogroup UNI_CMD_ID_STAREC_INFO
 * @{
 */
/**
 * This structure is used for STA_REC_MLD_BASIC(0x21) of
 * UNI_CMD_ID_STAREC_INFO command (0x03) to set MLD level information.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag             should be 0x21
 * @param[in] u2Length          the length of this TLV, should be
 *				sizeof(CMD_STAREC_MLD_BASIC_T)
 * @param[in] fgNSEP            support NSEP or not
 * @param[in] afgStrCapBitmap   StrCap bitmap
 *                              - pucStrBitmap[0] bit[0]: don't care
 *                              - pucStrBitmap[0] bit[1]: band0 and band1 str capability
 *                              - pucStrBitmap[0] bit[2]: band0 and band2 str capability
 *                              - pucStrBitmap[1] bit[0]: band1 and band0 str capability
 *                              - pucStrBitmap[1] bit[1]: don't care
 *                              - pucStrBitmap[1] bit[2]: band1 and band2 str capability
 *                              - pucStrBitmap[2] bit[0]: band2 and band0 str capability
 *                              - pucStrBitmap[2] bit[1]: band2 and band1 str capability
 *                              - pucStrBitmap[2] bit[2]: don't care
 * @param[in] aucEmlCap         Eml Capabilities subfield
 */
/* starec MLD level information (Tag 0x21) */
struct CMD_STAREC_EHT_MLD_T {
	UINT16 u2Tag;		/* Tag = 0x21 */
	UINT16 u2Length;
	UINT8 fgNSEP;
	UINT8 aucReserved[2];
	UINT8 afgStrCapBitmap[CFG_WIFI_RAM_BAND_NUM];
	UINT16 aucEmlCap;
	UINT8 aucPadding[4];
};
/** @} */


/** @addtogroup UNI_CMD_ID_STAREC_INFO
 * @{
 */
/**
 * This structure is used for STA_REC_EHT_BASIC(0x22) of
 * UNI_CMD_ID_STAREC_INFO command (0x03) to set per link EHT information.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag       should be 0x22
 * @param[in] u2Length	  the length of this TLV, should be
 *			  sizeof(struct CMD_STAREC_EHT_BASIC_T)
 * @param[in] ucTidBitmap the tid-to-link bitmap,  BIT0 for TID0,
 *			  BIT1 for TID1...
 *                        1: supoort transmission for the TID in this link
 *                        0: NOT support transmission for the TID in this link
 * @param[in] afgStrCap   STR Capability
 *                        afgStrCap[0]: TRUE, BAND0 and this band has STR
 *                        afgStrCap[0]: FALSE, BAND0 and this band is NON-STR
 */
/* starec link level EHT information (Tag 0x22) */
struct CMD_STAREC_EHT_BASIC_T {
	UINT_16  u2Tag;		/* Tag = 0x22 */
	UINT_16  u2Length;
	UINT_8   ucTidBitmap;
	UINT_8   aucPadding[1];
	UINT_16  u2EhtMacCap;
	UINT_64  u8EhtPhyCap;
	UINT_64  u8EhtPhyCapExt;
	UINT_8	 aucMscMap20MHzSta[4];
	UINT_8	 aucMscMap80MHz[3];
	UINT_8	 aucMscMap160MHz[3];
	UINT_8	 aucMscMap320MHz[3];
	UINT_8   aucPaddings[3];
};
/** @} */

/** @addtogroup UNI_CMD_ID_STAREC_INFO
 * @{
 */
/**
 * This structure is used for STA_REC_MLD_TEARDOWN(0x23) of
 * UNI_CMD_ID_STAREC_INFO command (0x03) to teardown MLD STAREC.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag                   should be 0x23
 * @param[in] u2Length                the length of this TLV, should be
				      sizeof(CMD_STAREC_MLD_TEARDOWN_T)
 */
/* MLD STAREC teardown (Tag 0x23) */
struct CMD_STAREC_MLD_TEARDOWN_T {
	UINT16  u2Tag;                 /* Tag = 0x23 */
	UINT16  u2Length;
};
/** @} */

struct GNU_PACKED UNI_CMD_STAREC_TX_PROC_T {
	UINT16	u2Tag;		/* Tag = 0x08 */
	UINT16	u2Length;
	UINT32	u4TxProcFlag;
};

#define UNI_RVLAN BIT(0)
#define UNI_IPCSO BIT(1)
#define UNI_TCPUDPCSO BIT(2)
#define UNI_TX_PROC_ACM_CFG_EN BIT(3)
#define UNI_TX_PROC_ACM_CFG_BK BIT(4)
#define UNI_TX_PROC_ACM_CFG_BE BIT(5)
#define UNI_TX_PROC_ACM_CFG_VI BIT(6)
#define UNI_TX_PROC_ACM_CFG_VO BIT(7)


/* Update - PN */
typedef struct GNU_PACKED _UNI_CMD_STAREC_SEC_PN_INFO_T {
	UINT16 u2Tag;               /* Tag = 0x0F */
	UINT16 u2Length;
	UINT8 aucPn[6];
	UINT8 ucTscType;            /* 0:GTK_PN, 1:IPN, 2:BIPN */
	UINT8 aucReserved;
} UNI_CMD_STAREC_SEC_PN_INFO_T, *P_UNI_CMD_STAREC_SEC_PN_INFO_T;

/* STAREC get PN (Tag 0x28) */
struct GNU_PACKED UNI_CMD_STAREC_HDRT_T {
	UINT16 u2Tag;  /* Tag = 0x28 */
	UINT16 u2Length;
	UINT8 ucHdrtMode; /* 0:VLAN packet with 2 LLC headers, 1:VLAN packet with 1 LLC headers */
	UINT8 aucReserved[3];
};

/** @addtogroup UNI_CMD_ID_STAREC_INFO
 * @{
 */
/**
 * This structure is used for UNI_CMD_STAREC_EML_OP(0x29) of UNI_CMD_ID_STAREC_INFO command (0x03)
 * to enable/disable EML Operation.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag               should be 0x29
 * @param[in] u2Length            the length of this TLV, should be sizeof(UNI_CMD_STAREC_EML_OP_T)
 * @param[in] ucEmlsrBitmap       EMLSR bitmap
 *                                - bit0: band0 link enable emlsr
 *                                - bit1: band1 link enable emlsr
 *                                - bit2: band2 link enable emlsr
 *                                - If the ucEmlsrBitmap field is 0, means EMLSR mode is Off
 * @param[in] ucLinkAntNum[3]     Antenna number for apcli/sta mode used
 *                                ucLinkAntNum[0]: band0 link ant number
 *                                ucLinkAntNum[1]: band1 link ant number
 *                                ucLinkAntNum[2]: band2 link ant number
 */
/* STAREC enable/disable EML Operation (Tag 0x29) */
struct UNI_CMD_STAREC_EML_OP_T {
	UINT16      u2Tag;             /* Tag = 0x29 */
	UINT16      u2Length;
	UINT8       ucEmlsrBitmap;
	UINT8       ucLinkAntNum[3];
};

struct _UNI_CMD_ONE_STA_SN_SET_INFO_T {
	UINT16 u2WlanIdx;
	UINT8 ucTid;
	UINT8 aucReserved1;
	UINT16 u2Sn;  /** when ucupdatetype =0 :u2Sn is delta value.
			* when ucupdatetype=1:u2Sn is SN value driver assigned
			*/
	UINT8 aucReserved2[2];
};

/*UPDATE HDR TRANS*/
struct UNI_CMD_STAREC_HDR_TRANS_T {
	UINT16 u2Tag;  /* Tag = 0x2b */
	UINT16 u2Length;
	UINT8 ucFromDs;
	UINT8 ucToDs;
	UINT8 ucDisRxHdrTran;
	UINT8 aucReserved;
};

/* STAREC to update MLR_INFO*/
struct UNI_CMD_STAREC_MLR_INFO_T {
	UINT16 u2Tag;  /* Tag = 0x2d */
	UINT16 u2Length;
	UINT8 ucMlrMode;
	UINT8 ucMlrState;
	UINT8 ucMlrStateForceType;
	UINT8 aucReserved[1];
};

/*UPDATE A4_INFO*/
struct UNI_CMD_STAREC_A4_INFO_T {
	UINT16 u2Tag;  /* Tag = 0x31 */
	UINT16 u2Length;
	UINT8 ucA4Cap;
	UINT8 aucReserved[3];
};

/** @addtogroup UNI_CMD_ID_STAREC_INFO
 * @{
 */
/**
 * This structure is used for UNI_CMD_STA_REC_RCA1(0x2D)
 * of UNI_CMD_ID_STAREC_INFO command (0x03)
 * to set PSM information.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag                   should be 0x2D
 * @param[in] u2Length                the length of this TLV,
 *				          should be sizeof(UNI_CMD_STAREC_RCA1_T)
 * @param[in] fgEnable                0: RCA1 disable, WTBL search doesn't need to check MUAR, 1:
 *                                       RCA1 enable, WTBL search need to check MUAR
 */
/* Update State Info */
struct UNI_CMD_STAREC_RCA1_T {
	UINT_16 u2Tag;       // Tag = 0x2D
	UINT_16 u2Length;
	BOOLEAN fgEnable;
	UINT_8 aucReserved[3];
};
/** @} */

/** @addtogroup UNI_CMD_ID_STAREC_INFO
 * @{
 */
/**
 * This structure is used for UNI_CMD_STA_REC_TX_CAP(0x2F) of UNI_CMD_ID_STAREC_INFO command (0x03)
 * to adjust Tx ability for this peer.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag				should be 0x2F
 * @param[in] u2Length			the length of this TLV, should be
								 sizeof(UNI_CMD_STAREC_TX_CAP_T)
 * @param[in] fgTxLimit			TRUE: adjust tx aggregation
 * @param[in] aucPadding[3];
 */
struct UNI_CMD_STAREC_TX_CAP_T {
	UINT16 u2Tag;       /* Tag = 0x2F */
	UINT16 u2Length;
	BOOLEAN fgTxLimit;
	UINT8 aucPadding[3];
};
/** @} */

/** @addtogroup UNI_CMD_ID_STAREC_INFO
 * @{
 */
/**
 * This structure is used for UNI_CMD_STA_REC_TX_PWR(0x3a) of
UNI_CMD_ID_STAREC_INFO command (0x03)
 * to adjust Tx ability for this peer.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag				should be 0x3a
 * @param[in] u2Length			the length of this TLV, should be
								sizeof(UNI_CMD_STAREC_TX_PWR_T)
 * @param[in] ucTxPowerOffset	control TX power offset
 * @param[in] aucPadding[3];
 */
struct UNI_CMD_STAREC_TX_PWR_T {
	UINT16 u2Tag;       /* Tag = 0x3a */
	UINT16 u2Length;
	UINT8  ucTxPowerOffset;
	UINT8  aucPadding[3];
};
/** @} */

/** @addtogroup UNI_CMD_ID_STAREC_INFO
 * @{
 */
/**
 * This structure is used for UNI_CMD_STA_REC_DUAL_CTS(0x3b) of
UNI_CMD_ID_STAREC_INFO command (0x03)
 * to adjust Tx ability for this peer.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag				should be 0x3b
 * @param[in] u2Length			the length of this TLV, should be
								sizeof(UNI_CMD_STAREC_DUAL_CTS_T)
 * @param[in] fgDualCts         Dual CTS setting
 * @param[in] aucPadding[3];
 */
struct UNI_CMD_STAREC_DUAL_CTS_T {
	UINT16 u2Tag;       /* Tag = 0x3b */
	UINT16 u2Length;
	BOOLEAN fgDualCts;
	UINT8  aucPadding[3];
};
/** @} */

/* ============== UNI_CMD_ID_STAREC_INFO End ========== */

/* ============== UNI_CMD_ID_EDCA_SET Begin ============== */
/* EDCA set command (0x04) */
typedef struct GNU_PACKED _UNI_CMD_EDCA_T {
    /* fixed field */
    UINT8 ucBssInfoIdx;
    UINT8 aucPadding[3];

    /* tlv */
    UINT_8 aucTlvBuffer[0];
} UNI_CMD_EDCA_T, *P_UNI_CMD_EDCA_T;

/* EDCA set command Tag */
typedef enum _UNI_CMD_EDCA_TAG_T {
    UNI_CMD_EDCA_AC_PARM = 0,
    UNI_CMD_EDCA_MAX_NUM
} UNI_CMD_EDCA_TAG_T;

/* EDCA AC Parameters (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_EDCA_AC_PARM_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucAcIndex;
    UINT8 ucValidBitmap;/**<
						*      Define            | BIT | Note              |
						*      ------------------|-----|------------------ |
						*      MASK_AIFS_SET     | 0   | 0x01, AIFSN       |
						*      MASK_WINMIN_SET   | 1   | 0x02, CW min      |
						*      MASK_WINMAX_SET   | 2   | 0x04, CW max      |
						*      MASK_TXOP_SET     | 3   | 0x08, TXOP Limit  |
						*/
    UINT8 ucCWmin;
    UINT8 ucCWmax;
    UINT16 u2TxopLimit;
    UINT8 ucAifsn;
    UINT8 aucPadding[1];
} UNI_CMD_EDCA_AC_PARM_T, *P_UNI_CMD_EDCA_AC_PARM_T;

#define MASK_AIFS_SET   BIT(0)
#define MASK_WINMIN_SET BIT(1)
#define MASK_WINMAX_SET BIT(2)
#define MASK_TXOP_SET   BIT(3)

typedef INT32 (*PFN_EDCA_AC_HANDLE)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
										MT_EDCA_CTRL_T *pEdcaParam, VOID *pHandle);

/* ============== UNI_CMD_ID_EDCA_SET End ============== */


/* ============== UNI_CMD_ID_WSYS_CONFIG Begin ============== */
/* WSYS Config set command (0x0B) */
struct GNU_PACKED UNI_CMD_WSYS_CONFIG_T {
    /* fixed field */
    UINT8 ucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
*TAG					|	ID | structure
*---------------------------------------|----------|---------------------------------------
*UNI_CMD_WSYS_CONFIG_FW_LOG_CTRL	|	0x0|	UNI_CMD_FW_LOG_CTRL_BASIC_T
*UNI_CMD_WSYS_CONFIG_FW_DBG_CTRL	|	0x1|	UNI_CMD_FW_DBG_CTRL_T
*UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL	|	0x2|	UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T
*UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG	|	0x3|	UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T
*UNI_CMD_HOSTREPORT_TX_LATENCY_CONFIG	|	0x4|	UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T
*UNI_CMD_WSYS_CONFIG_FW_LOG_BUFFER_CTRL	|	0x5|	UNI_CMD_WSYS_CONFIG_FW_LOG_BUFFER_CTRL_T
*UNI_CMD_CERT_CFG			|	0x6|	UNI_CMD_WSYS_CONFIG_CERT_CFG_CTRL
*UNI_CMD_FW_TIME_SYNC			|	0x7|	UNI_CMD_WSYS_CONFIG_FW_TIME_SYNC
*UNI_CMD_WSYS_CONFIG_FW_PLUSH_PS_PLE	|	0x8|	UNI_CMD_WSYS_CONFIG_FW_PLUSH_PS_PLE_T
*UNI_CMD_WSYS_CONFIG_FW_DUP_MBSS_BCN	|	0x9|	UNI_CMD_FW_DUP_MBSS_BCN_T
*/
};

/* WSYS Config set command TLV List */
enum UNI_CMD_WSYS_CONFIG_TAG_T {
	UNI_CMD_WSYS_CONFIG_FW_LOG_CTRL = 0,
	UNI_CMD_WSYS_CONFIG_FW_DBG_CTRL = 1,
	UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL = 2,
	UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG = 3,
	UNI_CMD_HOSTREPORT_TX_LATENCY_CONFIG = 4,
	UNI_CMD_WSYS_CONFIG_FW_LOG_BUFFER_CTRL = 5,
	UNI_CMD_CERT_CFG = 6,
	UNI_CMD_FW_TIME_SYNC = 7,
	UNI_CMD_WSYS_CONFIG_FW_PLUSH_PS_PLE = 8, /* TODO  */
	UNI_CMD_WSYS_CONFIG_FW_DUP_MBSS_BCN = 9,
	UNI_CMD_WSYS_CONFIG_MAX_NUM
};

/* FW Log Basic Setting (Tag0) */
struct GNU_PACKED UNI_CMD_FW_LOG_CTRL_BASIC_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucFwLog2HostCtrl;
    UINT8 ucFwLog2HostInterval;/**<
								*      Time takes effect only if these conditions are true:
								*      1. FW log destinations include host
								*      2. ucFwLog2HostInterval > 0 (Unit: second)
								*/
    UINT8 aucPadding[2];
};

enum ENUM_CMD_FW_LOG_2_HOST_CTRL_T {
    ENUM_CMD_FW_LOG_2_HOST_CTRL_OFF = 0,
    ENUM_CMD_FW_LOG_2_HOST_CTRL_2_UART = 1,
    ENUM_CMD_FW_LOG_2_HOST_CTRL_2_HOST = 2,
    ENUM_CMD_FW_LOG_2_HOST_CTRL_2_EMI = 4,
    ENUM_CMD_FW_LOG_2_HOST_CTRL_2_HOST_STORAGE = 8,
    ENUM_CMD_FW_LOG_2_HOST_CTRL_2_HOST_ETHNET = 16,
    ENUM_CMD_FW_LOG_2_HOST_CTRL_2_BUF = BIT(7),
};

typedef INT32 (*PFN_WSYS_FW_LOG_CTRL_BASIC_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct UNI_CMD_FW_LOG_CTRL_BASIC_T *pParam,
													VOID *pHandle);

/* FW Debug Level Setting (Tag1) */
struct GNU_PACKED UNI_CMD_FW_DBG_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT32 u4DbgModuleIdx;
    UINT8 ucDbgClass;
    UINT8 aucPadding[3];
};

typedef INT32 (*PFN_WSYS_FW_DBG_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct UNI_CMD_FW_DBG_CTRL_T *pParam,
												VOID *pHandle);

/* FW Log UI Setting (Tag2) */
struct GNU_PACKED UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT32 ucVersion; /* default is 1 */
    UINT32 ucLogLevel;/* 0: Default, 1: More, 2: Extreme */
    UINT8  aucReserved[4];
};

typedef INT32 (*PFN_WSYS_FW_LOG_UI_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T *pParam,
													VOID *pHandle);

/* FW Debug Level Setting (Tag3) */
struct GNU_PACKED UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT16 u2RxChecksum;   /* bit0: IP, bit1: UDP, bit2: TCP */
    UINT16 u2TxChecksum;   /* bit0: IP, bit1: UDP, bit2: TCP */
    UINT8 ucCtrlFlagAssertPath;
    UINT8 aucPadding[3];
};

typedef INT32 (*PFN_WSYS_FW_BASIC_CONFIG_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T *pParam,
													VOID *pHandle);

/* FW Debug Level Setting (Tag4) */
struct GNU_PACKED UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucActive;
    UINT8 aucReserved[3];
};

typedef INT32 (*PFN_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_HANDLE)(struct _RTMP_ADAPTER *pAd,
																struct UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T *pParam,
																VOID *pHandle);

/* Log Buffer Ctrl (Tag5) */
struct GNU_PACKED UNI_CMD_WSYS_CONFIG_FW_LOG_BUFFER_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT32 u4Address_mcu;
	UINT32 u4Address_wifi;
	UINT32 u4Address_bt;
	UINT32 u4Address_gps;
	UINT8 ucType;
	UINT8 aucReserved[3];
};

typedef INT32 (*PFN_WSYS_ONFIG_FW_LOG_BUFFER_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct UNI_CMD_WSYS_CONFIG_FW_LOG_BUFFER_CTRL_T *pParam,
													VOID *pHandle);

/* FW Debug Level Setting (Tag6) */
struct GNU_PACKED UNI_CMD_WSYS_CONFIG_CERT_CFG_CTRL {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucActive;
	UINT8 aucReserved[3];
};

typedef INT32 (*PFN_WSYS_CONFIG_CERT_CFG_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct UNI_CMD_WSYS_CONFIG_CERT_CFG_CTRL *pParam,
													VOID *pHandle);

/* FW Log Time Sync Setting (Tag7) */
struct GNU_PACKED UNI_CMD_WSYS_CONFIG_FW_TIME_SYNC {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT32 u4Second;
	UINT32 u4Usecond;
};

typedef INT32 (*PFN_WSYS_CONFIG_FW_TIME_SYNC_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct UNI_CMD_WSYS_CONFIG_FW_TIME_SYNC *pParam,
													VOID *pHandle);

/* UNI_CMD_WSYS_CONFIG_FW_PLUSH_PS_PLE_T (Tag8) TODO */

/* 11v MBSS Bcn duplicate on/off in WM (Tag9) */
struct GNU_PACKED  UNI_CMD_FW_DUP_MBSS_BCN_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	BOOLEAN fgEnable;
	UINT8  aucReserved[3];
};

typedef INT32 (*PFN_WSYS_CONFIG_FW_DUP_MBSS_BCN_T)(struct _RTMP_ADAPTER *pAd,
													struct UNI_CMD_FW_DUP_MBSS_BCN_T *pParam,
													VOID *pHandle);

struct GNU_PACKED UNI_CMD_WSYS_CFG_PARAM_T {
	UINT8 McuDest;
	BOOLEAN WsysCfgTagValid[UNI_CMD_WSYS_CONFIG_MAX_NUM];

	struct UNI_CMD_FW_LOG_CTRL_BASIC_T WsysFwLogCtrlBasic;
	struct UNI_CMD_FW_DBG_CTRL_T WsysFwDbgCtrl;
	struct UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T WsysCfgFwLogUICtrl;
	struct UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T WsysCfgFwFwBasicConfig;
	struct UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T WsysCfgHostReportTxLatency;
	struct UNI_CMD_WSYS_CONFIG_FW_LOG_BUFFER_CTRL_T WsysCfgFwLogBufferCtrl;
	struct UNI_CMD_WSYS_CONFIG_CERT_CFG_CTRL WsysCfgCertCtrl;
	struct UNI_CMD_WSYS_CONFIG_FW_TIME_SYNC WsysCfgFwTimeSync;
	struct UNI_CMD_FW_DUP_MBSS_BCN_T WsysCfgFw11vBcnCrtl;
};

/* ============== UNI_CMD_ID_WSYS_CONFIG End ============== */

/* ============== UNI_CMD_ID_ACCESS_REG Begin ============== */
/* register access command (0x0D) */
typedef struct GNU_PACKED _UNI_CMD_ACCESS_REG_T {
    /* fixed field */
    UINT8 ucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
						*
						*   TAG                              | ID  | structure
						*   ---------------------------------|-----|--------------
						*   UNI_CMD_ACCESS_REG_BASIC         | 0x0 | UNI_CMD_ACCESS_REG_BASIC_T
						*   UNI_CMD_ACCESS_RF_REG_BASIC      | 0x1 | UNI_CMD_ACCESS_RF_REG_BASIC_T
						*/
} UNI_CMD_ACCESS_REG_T, *P_UNI_CMD_ACCESS_REG_T;

/* Register access command TLV List */
typedef enum _UNI_CMD_ACCESS_REG_TAG_T {
    UNI_CMD_ACCESS_REG_BASIC = 0,
    UNI_CMD_ACCESS_RF_REG_BASIC,
    UNI_CMD_ACCESS_REG_MAX_NUM
} UNI_CMD_ACCESS_REG_TAG_T;

typedef struct GNU_PACKED _UNI_CMD_ACCESS_REG_BASIC_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT32 u4Addr;
    UINT32 u4Value;
} UNI_CMD_ACCESS_REG_BASIC_T, *P_UNI_CMD_ACCESS_REG_BASIC_T;

typedef INT32 (*PFN_ACCESS_REG_BASIC_HANDLE)(struct _RTMP_ADAPTER *pAd,
												RTMP_REG_PAIR *RegPair,
												VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_ACCESS_RF_REG_BASIC_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT16 u2WifiStream;
    UINT16 u2Reserved;
    UINT32 u4Addr;
    UINT32 u4Value;
} UNI_CMD_ACCESS_RF_REG_BASIC_T, *P_UNI_CMD_ACCESS_RF_REG_BASIC_T;

typedef INT32 (*PFN_ACCESS_RF_REG_BASIC_HANDLE)(struct _RTMP_ADAPTER *pAd,
													MT_RF_REG_PAIR *RfRegPair,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_ACCESS_REG_PARAM_T {
	BOOLEAN bQueryMode;
	UINT32 RegNum[UNI_CMD_ACCESS_REG_MAX_NUM];
	BOOLEAN AccessRegTagValid[UNI_CMD_ACCESS_REG_MAX_NUM];

	struct _RTMP_REG_PAIR *RegPair;
	struct _MT_RF_REG_PAIR *RfRegPair;
} UNI_CMD_ACCESS_REG_PARAM_T, *P_UNI_CMD_ACCESS_REG_PARAM_T;

/* ============== UNI_CMD_ID_ACCESS_REG End ============== */

/**************** UNI_CMD_ID_CHIP_CONFIG End ****************/
/* Chip Config set command TLV List */
/**
 * This structure is used for UNI_CMD_ID_CHIP_CONFIG command (0x0E) for Chip configuration
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] ucReserved       Reserved
 * @param[in] aucTlvBuffer     TLVs
 *
 */
/* Chip Config set command (0x0E) */
struct UNI_CMD_ID_CHIP_CONFIG_T {
	/* fixed field */
	UINT8 ucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
	*
	*   TAG                                    | ID  | structure
	*   ---------------------------------------|-----|--------------
	*   UNI_CMD_CHIP_CONFIG_SW_DBG_CTRL        | 0x0 | UNI_CMD_CHIP_CONFIG_SW_DBG_CTRL_T
	*   UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG       | 0x1 | UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG_T
	*   UNI_CMD_CHIP_CONFIG_CHIP_CFG           | 0x2 | UNI_CMD_CHIP_CONFIG_CHIP_CFG_T
	*   UNI_CMD_CHIP_CONFIG_NIC_CAPABILITY     | 0x3 | UNI_CMD_CHIP_CONFIG_NIC_CAPABILITY_T
	*   UNI_CMD_CHIP_CONFIG_DUP_WTBL_RANGE     | 0x4 | UNI_CMD_CHIP_CONFIG_DUP_WTBL_RANGE_T
	*/
};

enum UNI_CMD_ID_CHIP_CONFIG_TAG_T {
	UNI_CMD_CHIP_CONFIG_SW_DBG_CTRL = 0,
	UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG = 1,
	UNI_CMD_CHIP_CONFIG_CHIP_CFG = 2,
	UNI_CMD_CHIP_CONFIG_NIC_CAPABILITY = 3,
	UNI_CMD_CHIP_CONFIG_DUP_WTBL_RANGE = 4,
	UNI_CMD_CHIP_CONFIG_MAX_NUM
};

/**
 * This structure is used for UNI_CMD_CHIP_CONFIG_NIC_CAPABILITY (0x03)
 * of UNI_CMD_ID_CHIP_CONFIG command (0x0E) to get Connsys capability.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag
 *            should be 0x03
 * @param[in] u2Length
 *            the length of this TLV, should be sizeof(UNI_CMD_CHIP_CONFIG_NIC_CAPABILITY_T)
 */
/* Get Connsys Cpability (Tag3) */
struct UNI_CMD_CHIP_CONFIG_NIC_CAPABILITY_T {
	UINT_16  u2Tag;
	UINT_16  u2Length;
};

/**
 * This structure is used for UNI_CMD_CHIP_CONFIG_DUP_WTBL_RANGE (0x04)
 * of UNI_CMD_ID_CHIP_CONFIG command (0x0E)
 *
 * @param[in] u2Tag                   should be 0x4
 * @param[in] u2Length                the length of this TLV
 *                                    should be sizeof(UNI_CMD_CHIP_CONFIG_DUP_WTBL_RANGE_T)
 * @param[in] u2WtblDupBase           Start of duplicate wtbl
 * @param[in] u2WtblDupNumber         Available number of duplicate wtbl
 * @param[in] aucPadding              Reserved for future
 */
/* set duplicate wtbl range (Tag4) */
struct GNU_PACKED UNI_CMD_CHIP_CONFIG_DUP_WTBL_RANGE_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_16 u2WtblDupBase;
	UINT_16 u2WtblDupNumber;
	UINT_8 aucPadding[4];
};


/**************** UNI_CMD_ID_CHIP_CONFIG End *****************/


/* ============== UNI_CMD_ID_EEPROM_CONTROL Begin ============== */

struct GNU_PACKED UNI_CMD_EEPROM_CONTROL {
	/* fixed field */
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

enum  UNI_CMD_ID_EEPROM_CTRL_TAG {
	UNI_CMD_ID_EEPROM_ACCESS = 0x01,
	UNI_CMD_ID_EEPROM_MAX_NUM
};

struct GNU_PACKED UNI_CMD_ACCESS_EEPROM {
	UINT16  u2Tag;
	UINT16  u2Length;
	UINT32  u4Address; /* for access address */
	UINT32  u4Valid;
	UINT32  u4DataLen;
	UINT8   aucData[1024];
};
/* ============== UNI_CMD_ID_EEPROM_CONTROL End ============== */


/* ============== UNI_CMD_ID_EFUSE_CONTROL Begin ============== */

struct GNU_PACKED UNI_CMD_EFUSE_CONTROL {
	/* fixed field */
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

enum  UNI_CMD_ID_EFUSE_CTRL_TAG {
	UNI_CMD_ID_EFUSE_ACCESS = 0x01,
	UNI_CMD_ID_EFUSE_BUFFER_MODE,
	UNI_CMD_ID_EFUSE_FREE_BLOCK,
	UNI_CMD_ID_EFUSE_BUFFER_RD,
	UNI_CMD_ID_EFUSE_MAX_NUM
};

struct GNU_PACKED UNI_CMD_ACCESS_EFUSE {
	UINT16  u2Tag;
	UINT16  u2Length;
	UINT32  u4Address; /* for access address */
	UINT32  u4Valid;
	UINT8   aucData[16];
};

struct GNU_PACKED UNI_CMD_EFUSE_BUFFER_MODE {
	UINT16  u2Tag;
	UINT16  u2Length;
	UINT8   ucSourceMode;    /* 0: eFuse mode; 1: Buffer mode */
	UINT8   ucContentFormat; /* 0: Bin Content; 1: Whole Content; 2: Multiple Sections */
	UINT16  u2Count;         /* Total number of aBinContent elements */
	UINT8   BinContent[];
};

struct GNU_PACKED UNI_CMD_EFUSE_FREE_BLOCK {
	UINT16  u2Tag;
	UINT16  u2Length;
	UINT8   ucGetFreeBlock; /* the get free block number */
	UINT8   ucVersion; /* 0: original format ; 1: modified format */
	UINT8   ucDieIndex; /* for 7663, 0: D die ; 1: A die */
	UINT8   ucReserved;
};

struct GNU_PACKED UNI_CMD_EFUSE_BUFFER_MODE_READ {
	UINT16  u2Tag;
	UINT16  u2Length;
	UINT8   u1SourceMode;       /* 0: eFuse mode; 1: Buffer mode */
	UINT8   u1ContentFormat;    /* 0: Bin Content; 1: Whole Content; 2: Multiple Sections */
	UINT16  u2Offset;           /* Read Offset */
	UINT16  u2Count;            /* Read Total Counts */
	UINT16  u2Reserved;         /* Reserved */
};
/* ============== UNI_CMD_ID_EFUSE_CONTROL End ============== */

/* ============== UNI_CMD_ID_POWER_CTRL Begin ============== */
typedef struct GNU_PACKED _UNI_CMD_ID_POWER_CTRL_T {
    /* fixed field */
    UINT8 ucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
						*
						*   TAG                      | ID  | structure
						*   -------------------------|-----|--------------
						*   UNI_CMD_POWER_OFF        | 0x0 | UNI_CMD_POWER_OFF_T
						*/
} UNI_CMD_ID_POWER_CTRL_T, *P_UNI_CMD_ID_POWER_CTRL_T;

/* Get power ctrl command TLV List */
typedef enum _UNI_CMD_POWER_CTRL_TAG_T {
	UNI_CMD_POWER_OFF = 0,
	UNI_CMD_POWER_CLOCK_SWITCH_DISABLE = 1,
	UNI_CMD_POWER_POLL_TXRX_IDLE = 2,
	UNI_CMD_POWER_GREEN_AP_CTRL = 3,
	UNI_CMD_POWER_CTRL_MAX_NUM
} UNI_CMD_POWER_CTRL_TAG_T;

/* Get tsf time (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_POWER_OFF_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucPowerMode;
	UINT8 aucReserved[3];
} UNI_CMD_POWER_OFF_T, *P_UNI_CMD_POWER_OFF_T;

struct GNU_PACKED UNI_CMD_POWER_CLOCK_SWITCH_DISABLE_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 fgDisable;
	UINT8 aucReserved[3];
};

struct GNU_PACKED UNI_CMD_POWER_POLL_TXRX_IDLE_T {
	UINT16 u2Tag;
	UINT16 u2Length;
};

struct GNU_PACKED UNI_CMD_POWER_GREEN_AP_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  ucBandIdx;
	UINT8  fgGreenApOn;
	UINT8  aucReserved[2];
};
/* ============== UNI_CMD_ID_POWER_CTRL End ============== */

/* ============== UNI_CMD_ID_CFG_SMESH Begin ============== */
#ifdef AIR_MONITOR
/* SMESH command (0x10) */
typedef struct GNU_PACKED _UNI_CMD_SMESH_T {
    /* fixed field */
    UINT8 ucBand;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_SMESH_T, *P_UNI_CMD_SMESH_T;

/* Smesh command Tag */
typedef enum _UNI_CMD_SMESH_TAG_T {
    UNI_CMD_SMESH_PARAM = 0,
    UNI_CMD_SMESH_MAX_NUM
} UNI_CMD_SMESH_TAG_T;

/* SMESH Config Parameters (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_SMESH_PARAM_T {
    UINT16 u2Tag;    /* Tag = 0x00 */
    UINT16 u2Length;
    UINT8 ucEntryEnable;
    BOOLEAN fgSmeshRxA2;
    BOOLEAN fgSmeshRxA1;
    BOOLEAN fgSmeshRxData;
    BOOLEAN fgSmeshRxMgnt;
    BOOLEAN fgSmeshRxCtrl;
    UINT8 aucPadding[2];
} UNI_CMD_SMESH_PARAM_T, *P_UNI_CMD_SMESH_PARAM_T;
#endif /* AIR_MONITOR */
/* ============== UNI_CMD_ID_CFG_SMESH End ============== */

/* ============== UNI_CMD_ID_RX_HDR_TRAN Begin ============== */
/* Rx header translation command (0x12) */
typedef struct GNU_PACKED _UNI_CMD_RX_HDR_TRAN_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0]; /**<the TLVs includer in this field:
							*
							*  TAG                                   | ID   | structure
							*  -------------                         | -----| -------------
							*  UNI_CMD_RX_HDR_TRAN_ENABLE            | 0x0  | UNI_CMD_RX_HDR_TRAN_ENABLE_T
							*  UNI_CMD_RX_HDR_TRAN_VLAN_CONFIG       | 0x1  | UNI_CMD_RX_HDR_TRAN_VLAN_T
							*  UNI_CMD_RX_HDR_TRAN_BLACKLIST_CONFIG  | 0x2  | UNI_CMD_RX_HDR_TRAN_BLACKLIST_T
							*/
} UNI_CMD_RX_HDR_TRAN_T, *P_UNI_CMD_RX_HDR_TRAN_T;

/* RX HDR TRAN command TLV List */
typedef enum _UNI_CMD_RX_HDR_TRAN_TAG_T {
    UNI_CMD_RX_HDR_TRAN_ENABLE = 0,
    UNI_CMD_RX_HDR_TRAN_VLAN_CONFIG = 1,
    UNI_CMD_RX_HDR_TRAN_BLACKLIST_CONFIG = 2,
    UNI_CMD_RX_HDR_TRAN_MAX_NUM
} UNI_CMD_RX_HDR_TRAN_TAG_T;

/* Rx header translation enable/disable (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_RX_HDR_TRAN_ENABLE_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    BOOLEAN fgEnable;
    BOOLEAN fgCheckBssid;
    UINT8 ucTranslationMode;
    UINT8 aucPadding[1];
} UNI_CMD_RX_HDR_TRAN_ENABLE_T, *P_UNI_CMD_RX_HDR_TRAN_ENABLE_T;

typedef INT32 (*PFN_RX_HDR_TRAN_ENABLE_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_RX_HDR_TRAN_ENABLE_T pParam,
													VOID *pHandle);

/* Rx header translation vlan config (Tag1) */
typedef struct GNU_PACKED _UNI_CMD_RX_HDR_TRAN_VLAN_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    BOOLEAN fgInsertVlan;
    BOOLEAN fgRemoveVlan;
    BOOLEAN fgUseQosTid;
    UINT8 aucPadding[1];
} UNI_CMD_RX_HDR_TRAN_VLAN_T, *P_UNI_CMD_RX_HDR_TRAN_VLAN_T;

typedef INT32 (*PFN_RX_HDR_TRAN_VLAN_HANDLE)(struct _RTMP_ADAPTER *pAd,
												P_UNI_CMD_RX_HDR_TRAN_VLAN_T pParam,
												VOID *pHandle);

/* Rx header translation black list config (Tag2) */
typedef struct GNU_PACKED _UNI_CMD_RX_HDR_TRAN_BLACKLIST_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucBlackListIdx;
    BOOLEAN fgEnable;
    UINT16 u2EtherType;
} UNI_CMD_RX_HDR_TRAN_BLACKLIST_T, *P_UNI_CMD_RX_HDR_TRAN_BLACKLIST_T;

typedef INT32 (*PFN_RX_HDR_TRAN_BLACKLIST_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_RX_HDR_TRAN_BLACKLIST_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_RX_HDR_TRAN_PARAM_T {
	BOOLEAN RxHdrTranValid[UNI_CMD_RX_HDR_TRAN_MAX_NUM];
	UNI_CMD_RX_HDR_TRAN_ENABLE_T RxHdrTranEnable;
	UNI_CMD_RX_HDR_TRAN_VLAN_T RxHdrTranVlan;
	UNI_CMD_RX_HDR_TRAN_BLACKLIST_T RxHdrTranBlackList;
} UNI_CMD_RX_HDR_TRAN_PARAM_T, *P_UNI_CMD_RX_HDR_TRAN_PARAM_T;
/* ============== UNI_CMD_ID_RX_HDR_TRAN End ============== */

/* ============== UNI_CMD_ID_BAND_CONFIG Begin ============ */
/**
 * Common Part of UNI_CMD_ID_BAND_CONFIG (0x08)
 */
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_T {
	/*fixed field*/
	UINT8 ucDbdcIdx;
	UINT8 aucPadding[3];

	/* tlv */
	UINT8 aucTlvBuffer[0];
} UNI_CMD_BAND_CONFIG_T, *P_UNI_CMD_BAND_CONFIG_T;

/* Band Config command Tag */
typedef enum _UNI_CMD_BAND_CONFIG_TAG_T {
	UNI_CMD_BAND_CONFIG_RADIO_ONOFF = 0,
	UNI_CMD_BAND_CONFIG_RXV_CTRL = 1,
	UNI_CMD_BAND_CONFIG_SET_RX_FILTER = 2,
	UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME = 3,
	UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT = 4,
	UNI_CMD_BAND_CONFIG_EDCCA_ENABLE = 5,
	UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD = 6,
	UNI_CMD_BAND_CONFIG_ACK_CTS_TIMEOUT = 7,
	UNI_CMD_BAND_CONFIG_RTS_THRESHOLD = 8,
	UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN = 9,
	UNI_CMD_BAND_CONFIG_SCH_DET_DIS = 0xA,
	UNI_CMD_BAND_CONFIG_RTS0_PKT_THRESHOLD_CFG = 0xB,
	UNI_CMD_BAND_CONFIG_MAC_ENABLE_CTRL = 0xC,
	UNI_CMD_BAND_CONFIG_LPI_CTRL = 0xD,
	UNI_CMD_BAND_CONFIG_RX_ACK_TO_HOST = 0xE,
	UNI_CMD_BAND_CONFIG_BAR_RETRY_LIMIT = 0xF,
	UNI_CMD_BAND_CONFIG_DUAL_CTS_CFG = 0x10,
	UNI_CMD_BAND_CONFIG_DUMP_STA_PAUSE = 0x11,
	UNI_CMD_BAND_CONFIG_MAX_NUM
} UNI_CMD_BAND_CONFIG_TAG_T;

/** UNI_CMD_BAND_CONFIG_RADIO_ONOFF tag(0x0)
 * To turn on/off radio
 */
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T {
    UINT16 u2Tag;      /*should be 0x0*/
    UINT16 u2Length;   /*the length of this TLV, should be sizeof(UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T)*/
    BOOLEAN fgRadioOn;  /*TRUE: turn on radio, FALSE: turn off radio*/
    UINT8  aucPadding[3];
} UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T, *P_UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T;

typedef INT32 (*PFN_BAND_CFG_RADIO_ONOFF_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T pParam,
													VOID *pHandle);

/** UNI_CMD_BAND_CONFIG_RXV_CTRL tag(0x1)
 * To enable/disable RXV control
 */
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_RXV_CTRL_T {
    UINT16 u2Tag;              /*should be 0x1*/
    UINT16 u2Length;           /*the length of this TLV, should be sizeof(UNI_CMD_BAND_CONFIG_RXV_CTRL_T)*/
    UINT8  ucRxvOfRxEnable;    /*1: enable Rx's RXV control, 0: disable Rx's RXV control*/
    UINT8  ucRxvOfTxEnable;    /*1: enable Tx's RXV control, 0: disable Tx's RXV control*/
    UINT8  aucPadding[2];
} UNI_CMD_BAND_CONFIG_RXV_CTRL_T, *P_UNI_CMD_BAND_CONFIG_RXV_CTRL_T;

typedef INT32 (*PFN_BAND_CFG_RXV_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
												P_UNI_CMD_BAND_CONFIG_RXV_CTRL_T pParam,
												VOID *pHandle);

/** UNI_CMD_BAND_CONFIG_SET_RX_FILTER tag(0x2)
*/
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT32 u4RxPacketFilter;
} UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T, *P_UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T;

typedef INT32 (*PFN_BAND_CFG_SET_RX_FILTER_HANDLE)(struct _RTMP_ADAPTER *pAd,
														P_UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T pParam,
														VOID *pHandle);

/** UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME tag(0x3)
*/
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucDropRts;
    UINT8 ucDropCts;
    UINT8 ucDropUnwantedCtrl;
    UINT8 aucReserved[1];
} UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T, *P_UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T;

typedef INT32 (*PFN_BAND_CFG_DROP_CTRL_FRAME_HANDLE)(struct _RTMP_ADAPTER *pAd,
												  P_UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T pParam,
												  VOID *pHandle);

/** UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT tag(0x4)
*/
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucWmmIdx;
	UINT8 ucAc;
	UINT16 ucAggLimit;
} UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T, *P_UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T;

typedef INT32 (*PFN_BAND_CFG_AGG_AC_LIMIT_HANDLE)(struct _RTMP_ADAPTER *pAd,
												  P_UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T pParam,
												  VOID *pHandle);

/* EDCCA OnOff Control (Tag5) */
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T {
    UINT16 u2Tag;    /* Tag = 0x05 */
    UINT16 u2Length;
    UINT8 fgEDCCAEnable;
UINT8 fgEDCCAStd;
UINT8 aucPadding[2];
} UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T, *P_UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T;

typedef INT32 (*PFN_BAND_CFG_EDCCA_ENABLE_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
												  P_UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T pParam,
												  VOID *pHandle);

/* EDCCA Threshold Control (Tag6) */
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T {
	UINT16 u2Tag;    /* Tag = 0x06 */
	UINT16 u2Length;
	UINT8 u1EDCCAThreshold[4];
	UINT8 fginit;
	UINT8 aucReserved[3];
} UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T, *P_UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T;

typedef INT32 (*PFN_BAND_CFG_EDCCA_THRESHOLD_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
												  P_UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T pParam,
												  VOID *pHandle);

struct GNU_PACKED UNI_CMD_BAND_CONFIG_ACK_CTS_TIMEOUT_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 u1Type;
	UINT8 aucReserved[3];
	UINT32 u4TimeoutValue;
};

typedef INT32 (*PFN_BAND_CFG_ACK_CTS_TIMEOUT_HANDLE)(struct _RTMP_ADAPTER *pAd,
												  struct UNI_CMD_BAND_CONFIG_ACK_CTS_TIMEOUT_T *pParam,
												  VOID *pHandle);

struct GNU_PACKED UNI_CMD_BAND_CONFIG_RTS_THRESHOLD_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT32 u4RtsPktLenThreshold;
	UINT32 u4RtsPktNumThreshold;
};

typedef INT32 (*PFN_BAND_CFG_RTS_THRESHOLD_HANDLE)(struct _RTMP_ADAPTER *pAd,
												  struct UNI_CMD_BAND_CONFIG_RTS_THRESHOLD_T *pParam,
												  VOID *pHandle);

struct GNU_PACKED UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN_T {
	UINT16  u2Tag;
	UINT16  u2Length;
	BOOLEAN Enable;
	UINT8   aucReserve[3];
};

typedef INT32 (*PFN_BAND_CFG_RTS_SIGTA_EN_HANDLE)(struct _RTMP_ADAPTER *pAd,
									struct UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN_T *pParam,
									VOID *pHandle);

struct GNU_PACKED UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T {
	UINT16  u2Tag;
	UINT16  u2Length;
	BOOLEAN Disable;
	UINT8   aucReserve[3];
};

typedef INT32 (*PFN_BAND_CFG_SCH_DET_DIS_HANDLE)(struct _RTMP_ADAPTER *pAd,
									struct UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T *pParam,
									VOID *pHandle);

struct GNU_PACKED UNI_CMD_BAND_CONFIG_RTS0_PKT_THRESHOLD_CFG_T {
	UINT16  u2Tag;
	UINT16  u2Length;
	UINT32  u4Value;
	BOOLEAN Enable;
	UINT8   ucType;
	UINT8   aucReserve[2];
};

typedef INT32 (*PFN_BAND_CFG_RTS0_PKT_THRESHOLD_HANDLE)(struct _RTMP_ADAPTER *pAd,
									struct UNI_CMD_BAND_CONFIG_RTS0_PKT_THRESHOLD_CFG_T *pParam,
									VOID *pHandle);


struct GNU_PACKED UNI_CMD_BAND_CONFIG_MAC_ENABLE_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucMacEnable;
	UINT8 aucReserve[3];
};

typedef INT32 (*PFN_BAND_CFG_MAC_ENABLE_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
									struct UNI_CMD_BAND_CONFIG_MAC_ENABLE_CTRL_T *pParam,
									VOID *pHandle);

struct GNU_PACKED UNI_CMD_BAND_CONFIG_LPI_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucLpiEnable;
	UINT8 ucPSDLimit;
	UINT8 aucReserve[2];
};

typedef INT32 (*PFN_BAND_CFG_LPI_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
									struct UNI_CMD_BAND_CONFIG_LPI_CTRL_T *pParam,
									VOID *pHandle);

struct GNU_PACKED UNI_CMD_BAND_CONFIG_RX_ACK_TO_HOST_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	BOOLEAN fgEnable;
	UINT8 aucReserve[3];
};

typedef INT32 (*PFN_BAND_CFG_RX_ACK_TO_HOST_HANDLE)(struct _RTMP_ADAPTER *pAd,
									struct UNI_CMD_BAND_CONFIG_RX_ACK_TO_HOST_T *pParam,
									VOID *pHandle);

struct GNU_PACKED UNI_CMD_BAND_CONFIG_BAR_RETRY_LIMIT_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucBarRetryLimit;
	UINT8 aucReserve[3];
};

typedef INT32 (*PFN_BAND_CFG_BAR_RETRY_HANDLE)(struct _RTMP_ADAPTER *pAd,
									struct UNI_CMD_BAND_CONFIG_BAR_RETRY_LIMIT_T *pParam,
									VOID *pHandle);

/* UNI_CMD_BAND_CONFIG_DUAL_CTS_CFG tag(0x10) */
/* TBD */

/** UNI_CMD_BAND_CONFIG_DUMP_STA_PAUSE tag(0x11)
 */
struct GNU_PACKED UNI_CMD_BAND_CONFIG_DUMP_STA_PAUSE_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 aucReserve[4];
};

typedef INT32 (*PFN_BAND_CFG_DUMP_STA_PAUSE_HANDLE)(struct _RTMP_ADAPTER *pAd,
									struct UNI_CMD_BAND_CONFIG_DUMP_STA_PAUSE_T *pParam,
									VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_BAND_CFG_PARAM_T {
	UINT8 ucDbdcIdx;
	BOOLEAN bQuery;
	BOOLEAN BandCfgTagValid[UNI_CMD_BAND_CONFIG_MAX_NUM];

	UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T BandCfgRadioOnOff;
	UNI_CMD_BAND_CONFIG_RXV_CTRL_T BandCfgRXVCtrl;
	UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T BandCfgSetRxFilter;
	UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T BandCfgDropCtrlFrame;
	UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T BandCfgAGGAcLimit;
	UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T BandCfgEDCCAEnable;
	UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T BandCfgEDCCAThreshold;
	struct UNI_CMD_BAND_CONFIG_ACK_CTS_TIMEOUT_T BandCfgACKCTSTimeout;
	struct UNI_CMD_BAND_CONFIG_RTS_THRESHOLD_T BandCfgRTSThreshold;
	struct UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN_T BandCfgRtsSigtaen;
	struct UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T BandCfgSCHDetDis;
	struct UNI_CMD_BAND_CONFIG_RTS0_PKT_THRESHOLD_CFG_T BandCfgRTS0PktThreshold;
	struct UNI_CMD_BAND_CONFIG_MAC_ENABLE_CTRL_T BandCfgMacEnableCtrl;
	struct UNI_CMD_BAND_CONFIG_LPI_CTRL_T BandCfgLpiCtrl;
	struct UNI_CMD_BAND_CONFIG_RX_ACK_TO_HOST_T BandCfgRxAckToHost;
	struct UNI_CMD_BAND_CONFIG_BAR_RETRY_LIMIT_T BandCfgBarRetryLimit;
	/* UNI_CMD_BAND_CONFIG_DUAL_CTS_CFG : TBD */
	struct UNI_CMD_BAND_CONFIG_DUMP_STA_PAUSE_T BandCfgStaPauseDump;
} UNI_CMD_BAND_CFG_PARAM_T, *P_UNI_CMD_BAND_CFG_PARAM_T;
/* ============== UNI_CMD_ID_BAND_CONFIG End ============== */


/* ============== UNI_CMD_ID_SER Begin ============== */
typedef struct GNU_PACKED _UNI_CMD_SER_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0]; /**<the TLVs includer in this field:
							*
							*  TAG                    | ID   | structure
							*  -------------          | -----| -------------
							*  UNI_CMD_SER_QUERY      | 0x0  | UNI_CMD_SER_QUERY_T
							*  UNI_CMD_SER_ENABLE     | 0x1  | UNI_CMD_SER_ENABLE_T
							*  UNI_CMD_SER_SET        | 0x2  | UNI_CMD_SER_SET_T
							*  UNI_CMD_SER_TRIGGER    | 0x3  | UNI_CMD_SER_TRIGGER_T
							*/
} UNI_CMD_SER_T, *P_UNI_CMD_SER_T;

/* SER command TLV List */
typedef enum _UNI_CMD_SER_TAG_T {
    UNI_CMD_SER_QUERY = 0,
    UNI_CMD_SER_ENABLE = 1,
    UNI_CMD_SER_SET = 2,
    UNI_CMD_SER_TRIGGER = 3,
    UNI_CMD_SER_L0P5_CTRL = 4,
    UNI_CMD_SER_MAX_NUM
} UNI_CMD_SER_TAG_T;

/* Show ser (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_SER_QUERY_T {
    UINT16 u2Tag;
    UINT16 u2Length;
} UNI_CMD_SER_QUERY_T, *P_UNI_CMD_SER_QUERY_T;

typedef INT32 (*PFN_SER_QUERY_HANDLE)(struct _RTMP_ADAPTER *pAd,
										VOID *pHandle);

/* Enable/disable ser (Tag1) */
typedef struct GNU_PACKED _UNI_CMD_SER_ENABLE_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    BOOLEAN fgEnable;
    UINT8 aucPadding[3];
} UNI_CMD_SER_ENABLE_T, *P_UNI_CMD_SER_ENABLE_T;

typedef INT32 (*PFN_SER_ENABLE_HANDLE)(struct _RTMP_ADAPTER *pAd,
										BOOLEAN fgEnable,
										VOID *pHandle);

/* config ser (Tag2) */
typedef struct GNU_PACKED _UNI_CMD_SER_SET_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT32 u4EnableMask;
} UNI_CMD_SER_SET_T, *P_UNI_CMD_SER_SET_T;

typedef INT32 (*PFN_SER_ENABLE_MASK_HANDLE)(struct _RTMP_ADAPTER *pAd,
												UINT32 u4EnableMask,
												VOID *pHandle);

/* trigger ser recovery (Tag3) */
typedef struct GNU_PACKED _UNI_CMD_SER_TRIGGER_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucTriggerMethod;
    UINT8 ucDbdcIdx;
    UINT8 aucPadding[2];
} UNI_CMD_SER_TRIGGER_T, *P_UNI_CMD_SER_TRIGGER_T;

typedef INT32 (*PFN_SER_TRIGGER_HANDLE)(struct _RTMP_ADAPTER *pAd,
											UINT8 ucTriggerMethod,
											UINT8 ucDbdcIdx,
											VOID *pHandle);

/* do some controls in L0.5 reset (Tag4) */
typedef struct GNU_PACKED _UNI_CMD_SER_L0P5_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucCtrlAction;
    UINT8 aucPadding[3];
} UNI_CMD_SER_L0P5_CTRL_T, *P_UNI_CMD_SER_L0P5_CTRL_T;

/* ============== UNI_CMD_ID_SER End ============== */


/* ============== UNI_CMD_ID_TWT Begin ============== */
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
typedef struct GNU_PACKED _UNI_CMD_TWT_T {
	/*fixed field*/
	UINT8 ucBssInfoIdx;
	UINT8 aucPadding[3];
	/* tlv */
	UINT8 aucTlvBuffer[0];
	/**< the TLVs included in this field:
	 *
	 *  TAG                            | ID   | structure
	 *  -------------                  | -----| -------------
	 *  UNI_CMD_TWT_AGRT_UPDATE        | 0x0  | UNI_CMD_TWT_ARGT_UPDATE_T
	 *  UNI_CMD_TWT_MGMT_FRAME_OFFLOAD | 0x1  | UNI_CMD_TWT_MGMT_FRAME_OFFLOAD_T
	 */
} UNI_CMD_TWT_T, *P_UNI_CMD_TWT_T;

typedef enum _UNI_CMD_TWT_TAG_T {
	UNI_CMD_TWT_AGRT_UPDATE = 0,
	UNI_CMD_TWT_MGMT_FRAME_OFFLOAD = 1,
	UNI_CMD_TWT_MAX_NUM
} UNI_CMD_TWT_TAG_T;

/* Basic Scan down notify Parameters (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_TWT_ARGT_UPDATE_T {
	UINT16    u2Tag;    /* Tag = 0x00 */
	UINT16    u2Length;
	UINT8     ucAgrtTblIdx;
	UINT8     ucAgrtCtrlFlag;
	UINT8     ucOwnMacId;
	UINT8     ucFlowId;
	UINT16    u2PeerIdGrpId;
	UINT8     ucAgrtSpDuration;
	UINT8     ucBssIndex;
	UINT32    u4AgrtSpStartTsf_low;
	UINT32    u4AgrtSpStartTsf_high;
	UINT16    u2AgrtSpWakeIntvlMantissa;
	UINT8     ucAgrtSpWakeIntvlExponent;
	UINT8     fgIsRoleAp;
	UINT8     ucAgrtParaBitmap;
	UINT8     ucPersistence;
	UINT16    u2NtbttBeforeReject;
	UINT8     ucGrpMemberCnt;
	UINT8     ucAgrtTblIdxH;
#ifdef DOT11_EHT_BE
	/* Restricted TWT Traffic Info */
	UINT8     ucTidDlBitmap;
	UINT8     ucTidUlBitmap;
#else
	UINT16    u2Reserved_d;
#endif /* DOT11_EHT_BE */
	UINT16    au2StaList[TWT_HW_BTWT_MAX_MEMBER_CNT];
} UNI_CMD_TWT_ARGT_UPDATE_T, *P_UNI_CMD_TWT_ARGT_UPDATE_T;

struct GNU_PACKED UNI_CMD_TWT_MGMT_FRAME_OFFLOAD_T {
	UINT16    u2Tag;    /* Tag = 1 */
	UINT16    u2Length;
	UINT16    u2Wcid;
	UINT8     ucToken;
	UINT8     ucRvd;
	UINT8     ucMloEn;
	UINT8     ucActionLinkid;
	UINT16    u2AllLinkidBitmap;
	UINT16    u2MldStaIndx;
	UINT8     ucRvd1[2];
	UINT8     aucVal[0];
};
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
/* ============== UNI_CMD_ID_TWT End ============== */

/* ======== UNI_CMD_ID_REPT_MUAR & UNI_CMD_ID_NORM_MUAR Begin ======== */
/* NORM/REPT_MUAR command Tag */
typedef enum _UNI_CMD_MUAR_TAG_T {
    UNI_CMD_MUAR_CLEAN = 0, /* No used by Rebb */
    UNI_CMD_MUAR_MC_FILTER = 1, /* No used by Rebb */
    UNI_CMD_MUAR_ENTRY = 2,
    UNI_CMD_MUAR_MAX_NUM
} UNI_CMD_MUAR_TAG_T;

typedef struct GNU_PACKED _UNI_CMD_MUAR_T {
    /* fixed field */
    UINT8 ucBand;       /* operation band */
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_MUAR_T, *P_UNI_CMD_MUAR_T;

/* MC Config Parameters (Tag2) */
typedef struct GNU_PACKED _UNI_CMD_MUAR_ENTRY_T {
    UINT16 u2Tag;             /* should be 0x02 */
    UINT16 u2Length;          /* the length of this TLV, should be 8 */
    BOOLEAN fgSmesh;           /* future used. TRUE: config smart mesh operation, FALSE: config multicast operation */
    UINT8 ucHwBssIndex;       /* hw bss index */
    UINT8 ucMuarIdx;          /* config MUAR table index, set 0xFF means fw auto search index and record */
    UINT8 ucEntryAdd;         /* set TRUE means add one, FALSE means to remove one */
    UINT8 aucMacAddr[6];      /* config mac address in MUAR table */
    UINT8 aucPadding[2];
} UNI_CMD_MUAR_ENTRY_T, *P_UNI_CMD_MUAR_ENTRY_T;
/* ======== UNI_CMD_ID_REPT_MUAR & UNI_CMD_ID_NORM_MUAR End ======== */

/* ======== UNI_CMD_ID_GET_MAC_INFO Begin ======== */
/* Get mac info command (0x1A) */
typedef struct GNU_PACKED _UNI_CMD_GET_MAC_INFO_T {
	/* fixed field */
	UINT8 ucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
							*
							*   TAG                      | ID  | structure
							*   -------------------------|-----|--------------
							*   UNI_CMD_MAC_INFO_TSF     | 0x0 | UNI_CMD_MAC_INFO_TSF_T
							*/
} UNI_CMD_GET_MAC_INFO_T, *P_UNI_CMD_GET_MAC_INFO_T;

/* Get mac info command TLV List */
typedef enum _UNI_CMD_MAC_INFO_TAG_T {
	UNI_CMD_MAC_INFO_TSF = 0,
	UNI_CMD_MAC_INFO_TSF_DIFF = 2,
	UNI_CMD_MAC_INFO_MAX_NUM
} UNI_CMD_MAC_INFO_TAG_T;

/* Get tsf time (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_MAC_INFO_TSF_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucDbdcIdx;
    UINT8 ucHwBssidIndex;
    UINT8 aucPadding[2];
} UNI_CMD_MAC_INFO_TSF_T, *P_UNI_CMD_MAC_INFO_TSF_T;

/* Get tsf time (Tag2) */
struct GNU_PACKED UNI_CMD_MAC_INFO_TSF_DIFF_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucBssIdx0;
	UINT8 ucBssIdx1;
	UINT8 aucPadding[2];
};

/* ======== UNI_CMD_ID_GET_MAC_INFO End ======== */

/* ======== UNI_CMD_ID_TXCMD_CTRL Begin ======== */
/* TXCMD Ctrl command (0x1D) */
typedef struct GNU_PACKED _UNI_CMD_TXCMD_CTRL_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_TXCMD_CTRL_T, *P_UNI_CMD_TXCMD_CTRL_T;

/* TXCMD ctrl cmd tags */
typedef enum _UNI_CMD_TXCMD_CTRL_TAG_T {
    UNI_CMD_SET_TXCMD_DBG_CTRL = 0x00,
    UNI_CMD_SET_TXCMD_DBG_CLEAR = 0x01,
    UNI_CMD_SET_TXCMD_DBG_SXN_GLOBAL = 0x02,
    UNI_CMD_SET_TXCMD_DBG_SXN_PROTECT = 0x03,
    UNI_CMD_SET_TXCMD_DBG_SXN_PROTECT_RUINFO = 0x04,
    UNI_CMD_SET_TXCMD_DBG_SXN_TXDATA = 0x05,
    UNI_CMD_SET_TXCMD_DBG_SXN_TXDATA_USER_INFO = 0x06,
    UNI_CMD_SET_TXCMD_DBG_SXN_TRIGDATA = 0x07,
    UNI_CMD_SET_TXCMD_DBG_SXN_TRIGDATA_USER_ACK_INFO = 0x08,
    UNI_CMD_SET_TXCMD_DBG_TF_TXD = 0x09,
    UNI_CMD_SET_TXCMD_DBG_TF_BASIC = 0x0a,
    UNI_CMD_SET_TXCMD_DBG_TF_BASIC_USER = 0x0b,
    UNI_CMD_SET_TXCMD_DBG_SXN_SW_FID = 0x0c,
    UNI_CMD_SET_TXCMD_DBG_SXN_SW_FID_INFO = 0x0d,
    UNI_CMD_SET_TXCMD_DBG_SW_FID_TXD = 0x0e,
    UNI_CMD_GET_TXCMD_DBG_STATUS = 0x0f,
    UNI_CMD_GET_TXCMD_DBG_SXN_GLOBAL = 0x10,
    UNI_CMD_GET_TXCMD_DBG_SXN_PROTECT = 0x11,
    UNI_CMD_GET_TXCMD_DBG_SXN_TXDATA = 0x12,
    UNI_CMD_GET_TXCMD_DBG_SXN_TRIGDATA = 0x13,
    UNI_CMD_GET_TXCMD_DBG_TF_TXD = 0x14,
    UNI_CMD_GET_TXCMD_DBG_TF_BASIC = 0x15,
    UNI_CMD_GET_TXCMD_DBG_SXN_SW_FID = 0x16,
    UNI_CMD_GET_TXCMD_DBG_SW_FID_TXD = 0x17,
    UNI_CMD_SET_TXCMD_DBG_SOP = 0x18,
    UNI_CMD_TXCMD_CTRL_MAX_NUM
} UNI_CMD_TXCMD_CTRL_TAG_T;

/* Txcmd ctrl Parameters (Tag) */
typedef struct GNU_PACKED _UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T {
    UINT16 u2Tag;    /* Tag = 0x00 */
    UINT16 u2Length;

    UINT8  ucUserIndex;
    UINT8  ucDlUlidx;
    UINT8  ucRsv[2];

    UINT8  aucBuffer[0];
} UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T, *P_UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T;

typedef struct GNU_PACKED _UNI_CMD_SET_TXCMD_DBG_CTRL_ENTRY_T {
	UINT16 u2Tag;
    UINT8 ucUserIndex;
    UINT8 ucDlUlidx;

	UINT32 u4DataLen;
    UINT8 *pData;
} UNI_CMD_SET_TXCMD_DBG_CTRL_ENTRY_T, *P_UNI_CMD_SET_TXCMD_DBG_CTRL_ENTRY_T;

typedef INT32 (*PFN_SET_TXCMD_DBG_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
												P_UNI_CMD_SET_TXCMD_DBG_CTRL_ENTRY_T pParam,
												VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TXCMD_DBG_CTRL_PARAM_T {
	BOOLEAN bQuery; /* for Query */
	UNI_CMD_SET_TXCMD_DBG_CTRL_ENTRY_T SetTxCmdDbgEntry[UNI_CMD_TXCMD_CTRL_MAX_NUM];
	BOOLEAN TxCmdDbgCtrlValid[UNI_CMD_TXCMD_CTRL_MAX_NUM];
} UNI_CMD_TXCMD_DBG_CTRL_PARAM_T, *P_UNI_CMD_TXCMD_DBG_CTRL_PARAM_T;

/* ======== UNI_CMD_ID_TXCMD_CTRL End ======== */

/* =================== UNI_CMD_ID_ECC_OPER Begin =================== */
/* ECC operation cmd tags */
typedef enum _UNI_CMD_ECC_OP_TAG_T {
    UNI_CMD_ECC_OP_CAL_GROUP_POINT   = 0x00,
    UNI_CMD_ECC_OP_MAX_NUM
} UNI_CMD_ECC_OP_TAG_T;

/* This structure is used for UNI_CMD_ID_ECC_OPER command (0x18) to calculate ECC key */
typedef struct GNU_PACKED _UNI_CMD_ECC_OP_T {
    /* fixed field */
    UINT8 aucReserved[4];  /*reserved fixed field*/

    /* tlv */
    UINT8 aucTlvBuffer[0];  /*TLVs*/
} UNI_CMD_ECC_OP_T, *P_UNI_CMD_ECC_OP_T;

/* ECC Operation Parameters (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_ECC_OP_CAL_T {
    UINT16 u2Tag;          /* Tag = 0x00 */
    UINT16 u2Length;       /* the length of this TLV */

    UINT8 ucGroupID;       /* group idx */
    UINT8 ucDataLength;    /* it means the length of total */
    UINT8 ucDataType;      /* 0: only scalar(DG mode in HW), 1: scalar and point(x and y)(DQ mode in HW) */
    UINT8 ucEccCmdId;      /* Ecc cmd queue idx */
    UINT8 aucBuffer[0];    /* key data */
} UNI_CMD_ECC_OP_CAL_T, *P_UNI_CMD_ECC_OP_CAL_T;
/* ==================== UNI_CMD_ID_ECC_OPER End ==================== */

#ifdef MT_DFS_SUPPORT
/* =================== UNI_CMD_ID_RDD_ON_OFF_CTRL Begin =================== */
/* RDD set command (0x19) */
typedef struct GNU_PACKED _UNI_CMD_RDD_T {
    /*fixed field*/
    UINT8 aucPadding[4];
    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_RDD_T, *P_UNI_CMD_RDD_T;

/* RDD on off command (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_RDD_ON_OFF_CTRL_PARM_T {
UINT16 u2Tag;
UINT16 u2Length;
UINT8 u1DfsCtrl;
UINT8 u1RddIdx;
UINT8 u1RddRxSel;
UINT8 u1SetVal;
UINT8 u1DisTm;
UINT8 aucReserve[3];
} UNI_CMD_RDD_ON_OFF_CTRL_PARM_T, *P_UNI_CMD_RDD_ON_OFF_CTRL_PARM_T;

struct _UNI_CMD_RDD_IPI_HIST_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ipi_hist_idx;
	UINT8 band_idx;
	UINT8 set_val;
	UINT8 reserved;
	INT32 idle_pwr_thres;
	UINT32 idle_pwr_max_cnt;
	UINT32 idle_pwr_duration;
	UINT32 idle_pwr_cmd_type;
};
#ifdef IPI_MAINRADIO_SCAN_SUPPORT
struct _UNI_CMD_RDD_IPI_SCAN_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT_8 u1pdSetting;
	UINT_8 u1Band;
	UINT_8 aucReserve[2];
};
#endif
struct UNI_RDM_FCC5_LPN_UPDATE_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT32 tag;		/*Tag = 0x01*/
	UINT16 fcc_lpn_min;
	UINT8  reserved[2];
};

struct GNU_PACKED RDM_RDR_THRES_UPDATE_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT32 tag;				/* Tag = 0x02 */
	UINT16 radar_type_idx; /* Valid Range 0~15*/
	UINT8  rt_en;          /* Radar type is enabled */
	UINT8  rt_stgr;        /* Radar type is staggred radar */
	UINT8  rt_crpn_min;    /* minimum constant PRF radar pulse number */
	UINT8  rt_crpn_max;    /* maximum constant PRF radar pulse number */
	UINT8  rt_crpr_min;    /* minimum constant PRF radar pulse ratio */
	UINT8  rt_pw_min;
	UINT32 rt_pri_min;
	UINT32 rt_pri_max;
	UINT8  rt_pw_max;
	UINT8  rt_crbn_min;  /* Constant PRF Radar: Burst Number */
	UINT8  rt_crbn_max;
	UINT8  rt_stg_pn_min;
	UINT8  rt_stg_pn_max;
	UINT8  rt_stg_pr_min;
	UINT8  reserved[2];
	UINT32 rt_stg_pri_diff_min; /*min PRI Difference between 1st and 2nd */
};

struct GNU_PACKED UNI_RDM_PLS_THRES_UPDATE_T {
	UINT32 tag;                 /* Tag = 0x03 */
	UINT32 prd_pls_width_max;   /* unit us */
	INT32 pls_pwr_max;          /* unit dbm */
	INT32 pls_pwr_min;          /* unit dbm */
	UINT32 pri_min_stgr;		/* unit us */
	UINT32 pri_max_stgr;		/* unit us */
	UINT32 pri_min_cr;			/* unit us */
	UINT32 pri_max_cr;			/* unit us */
	UINT16 u2Tag;
	UINT16 u2Length;
};

struct UNI_RDM_RDD_LOG_CONFIG_UPDATE_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 tag;                 /* Tag = 0x04 */
	UINT8 hw_rdd_log_en;        /* 0: no dump, 1: dump log */
	UINT8 sw_rdd_log_en;        /* 0: no dump, 1: dump log */
	UINT8 sw_rdd_log_cond;
};

struct GNU_PACKED UNI_RDM_RDD_PERIODIC_PULSE_BUFFER_T {
	UINT32 prd_strt_time;   /* Periodic pulse start time */
	UINT16 prd_pls_wdth;    /* Periodic pulse width */
	INT16 prd_pls_pwr;       /* Periodic pulse power */
	UINT8 prd_mdrdy_flg;   /* MDRDY flag, bit1: mdray_early, bit0: mdrdy_late */
	UINT8 reserved[3];        /* Reserved bytes */
};

struct GNU_PACKED UNI_RDM_RDD_TEST_RADAR_PATTERN_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 pls_num;
	UINT8 reserved[3];
	struct UNI_RDM_RDD_PERIODIC_PULSE_BUFFER_T prd_pls_buff[PPB_SIZE];
};

/* RDD set command Tag */
typedef enum _UNI_CMD_RDD_TAG_T {
    UNI_CMD_RDD_ON_OFF_CTRL_PARM = 0,
	UNI_CMD_RDD_IPI_HIST_SET_PARM = 1,
	UNI_CMD_RDD_IPI_HIST_GET_PARM = 2,
	UNI_CMD_SET_RDM_RADAR_THRES = 3,
	UNI_CMD_SET_RDM_TEST_PATTERN = 4,
#ifdef IPI_MAINRADIO_SCAN_SUPPORT
	UNI_CMD_RDD_IPI_SCAN_SET_PARM = 5,
	UNI_CMD_RDD_IPI_SCAN_GET_PARM = 6,
#endif

	UNI_CMD_RDD_MAX_NUM
} UNI_CMD_RDD_TAG_T;

/* ==================== UNI_CMD_ID_RDD_ON_OFF_CTRL End ==================== */
#endif /* MT_DFS_SUPPORT */

/* ==================== UNI_CMD_ID_RRO Begin ============================== */
/* RRO command (0x57) */
struct GNU_PACKED UNI_CMD_RRO_T
{
    /*fixed field*/
	UINT_8 aucPadding[4];
    /* tlv */
	UINT_8 aucTlvBuffer[0];
};

/* UNI_CMD_RRO_SET_FUNCTION_MODE (0x00) */
struct GNU_PACKED UNI_CMD_RRO_SET_FUNCTION_MODE_T
{
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8  ucFunctionMode;
	UINT_8  aucPadding[3];
};

/* UNI_CMD_RRO_DELETE_ENTRY (0x01) */
struct GNU_PACKED UNI_CMD_RRO_DELETE_ENTRY_T
{
	UINT_16   u2Tag;
	UINT_16   u2Length;
	UINT_16   u2WlanIdx;
	UINT_8    ucTid;
	UINT_8    aucPadding[1];
};

/**
 * This structure is used for UNI_CMD_RRO_SET_PLATFORM_TYPE (0x2) of UNI_CMD_ID_RRO command (0x57)
 * to set RRO function mode
 *
 * @param[in] u2Tag      should be 0x2
 * @param[in] u2Length   the length of this TLV, should be sizeof(UNI_CMD_RRO_SET_PLATFORM_TYPE_T)
 * @param[in] ucType     the ring sku type
 */
struct GNU_PACKED UNI_CMD_RRO_SET_PLATFORM_TYPE_T
{
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 ucType;
	UINT_8 aucPadding[3];
};

/**
 * This structure is used for UNI_CMD_RRO_SET_BYPASS_MODE (0x4) of UNI_CMD_ID_RRO command (0x57)
 * to set RRO function mode
 *
 * @param[in] u2Tag            should be 0x4
 * @param[in] u2Length         the length of this TLV,
 *                             should be sizeof(UNI_CMD_RRO_SET_BYPASS_MODE_T)
 * @param[in] u1RroBypassType  set rro bypass mode
 * @param[in] u1MldDestType    set mld dest
 * @param[in] aucPadding       reserved
 */
/* set rro bypass mode (Tag4) */
struct GNU_PACKED UNI_CMD_RRO_SET_BYPASS_MODE_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 u1RroBypassType;
	UINT_8 u1MldDestType;
	UINT_8 aucPadding[2];
};

struct  GNU_PACKED UNI_CMD_RRO_DEL_SESSION_T {
	UINT_16   u2Tag;
	UINT_16   u2Length;
	UINT_16   u2Seid;
	UINT_8    aucPadding[2];
};


/**
 * This structure is used for UNI_CMD_RRO_SET_TX_FREEDONE_PATH(0x5) of UNI_CMD_ID_RRO command (0x57)
 * to set RRO function mode
 *
 * @param[in] u2Tag      should be 0x5
 * @param[in] u2Length   the length of this TLV, should be sizeof(UNI_CMD_RRO_SET_TX_FREEDONE_PATH)
 * @param[in] ucPath     the tx free done event path
 */
 /* rro platform setting (Tag5) */
/**
 * Path definition:
 * Type 0: default: via HIF
 * Type 1: via WA
 */
struct GNU_PACKED UNI_CMD_RRO_SET_TX_FREEDONE_PATH_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 ucPath;
	UINT_8 aucPadding[3];
};

/**
 * This structure is used for UNI_CMD_RRO_SET_FLUSH_TIMEOUT (0x7) of UNI_CMD_ID_RRO command (0x57)
 * to delete ba session.
 *
 * @param[in] u2Tag                   should be 0x7
 * @param[in] u2Length                the length of this TLV
 *                                    should be sizeof(UNI_CMD_RRO_SET_FLUSH_TIMEOUT_T)
 * @param[in] ucStepOneTimeout        StepOneTimeout(ms)
 * @param[in] ucFlushAllTimeout       FlushAllTimeout(ms)
 * @param[in] aucPadding              Reserved for future(e.g. timerid, sessionid)
 */
/* rro set flush timeout (Tag7) */
struct GNU_PACKED UNI_CMD_RRO_SET_FLUSH_TIMEOUT_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_16 u2StepOneTimeout;
	UINT_16 u2FlushAllTimeout;
	UINT_8 aucPadding[4];
};

/**
 * This structure is used for UNI_CMD_RRO_SET_RSS_MAP(0x8) of UNI_CMD_ID_RRO command (0x57)
 * to set RRO function mode
 *
 * @param[in] u2Tag      should be 0x8
 * @param[in] u2Length   the length of this TLV, should be sizeof(UNI_CMD_RRO_SET_RSS_MAP_T)
 * @param[in] rss_map0   the value of RSS MAP 0
 * @param[in] rss_map1   the value of RSS MAP 1
 */
 /* rro RSS map setting (Tag8) */
struct GNU_PACKED UNI_CMD_RRO_SET_RSS_MAP_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_32 rss_map0;
	UINT_32 rss_map1;
};

/* RRO command Tag */
enum UNI_CMD_ID_RRO_TAG_T {
	UNI_CMD_RRO_SET_FUNCTION_MODE = 0,
	UNI_CMD_RRO_DELETE_ENTRY = 1,
	UNI_CMD_RRO_SET_PLATFORM_TYPE = 2,
	UNI_CMD_RRO_GET_BA_SESSION_TABLE  = 3,
	UNI_CMD_RRO_SET_BYPASS_MODE = 4,
	UNI_CMD_RRO_SET_TX_FREEDONE_PATH = 5,
	UNI_CMD_RRO_DEL_SESSION = 6,
	UNI_CMD_RRO_SET_FLUSH_TIMEOUT = 7,
	UNI_CMD_RRO_SET_RSS_MAP = 8,
};

enum {
	RRO_FUNCTION_MODE_PRE_LINK = 0x0,
	RRO_FUNCTION_MODE_BUF_PG = 0x1,
};

/* ==================== UNI_CMD_ID_RRO End ================================ */

/* =================== UNI_CMD_ID_OFF_CH_SCAN_CTRL Begin =================== */
/* Off channel scan set command (0x58) */
typedef struct GNU_PACKED _UNI_CMD_OFF_CH_SCAN_T
{
    /*fixed field*/
	UINT_8 aucPadding[4];
    /* tlv */
	UINT_8 aucTlvBuffer[0];
} UNI_CMD_OFF_CH_SCAN_T, *P_UNI_CMD_OFF_CH_SCAN_T;

/* Off channel scan command (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_OFF_CH_SCAN_CTRL_PARM_T
{
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 u1WorkPrimCh;
	UINT_8 u1WorkCentralCh;
	UINT_8 u1WorkBW;
	UINT_8 u1WorkTxStreamPath;

	UINT_8 u1WorkRxStreamPath;
	UINT_8 u1MonitorPrimCh;
	UINT_8 u1MonitorCentralCh;
	UINT_8 u1MonitorBW;

	UINT_8 u1MonitorTxStreamPath;
	UINT_8 u1MonitorRxStreamPath;
	UINT_8 u1ScanMode;
	UINT_8 u1DbdcIdx;

	UINT_8 u1OffChScanType;
	UINT_8 u1Band;
	UINT_8 aucReserve[2];
} UNI_CMD_OFF_CH_SCAN_CTRL_PARM_T, *P_UNI_CMD_OFF_CH_SCAN_CTRL_PARM_T;

/* Off ch scan set command Tag */
typedef enum _UNI_CMD_ID_OFF_CH_SCAN_TAG_T {
	UNI_CMD_OFF_CH_SCAN_CTRL = 0x0,
	UNI_CMD_OFF_CH_SCAN_MAX_NUM
} UNI_CMD_ID_OFF_CH_SCAN_TAG_T;

/* =================== UNI_CMD_ID_OFF_CH_SCAN_CTRL End =================== */

#ifdef CFG_SUPPORT_FALCON_SR
/* SR Command Tag ID */
enum _UNI_CMD_SR_TAG_T {
	UNI_CMD_SR_RSV = 0x0,
	UNI_CMD_SR_CFG_SR_ENABLE = 0x1,
	UNI_CMD_SR_CFG_SR_SD_ENABLE = 0x2,
	UNI_CMD_SR_CFG_SR_MODE = 0x3,
	UNI_CMD_SR_CFG_DISRT_ENABLE = 0x4,
	UNI_CMD_SR_CFG_DISRT_MIN_RSSI = 0x5,
	UNI_CMD_SR_CFG_SR_BF = 0x6,
	UNI_CMD_SR_CFG_SR_ATF = 0x7,
	UNI_CMD_SR_CFG_TXC_QUEUE = 0x8,
	UNI_CMD_SR_CFG_TXC_QID = 0x9,
	UNI_CMD_SR_CFG_TXC_PATH = 0xA,
	UNI_CMD_SR_CFG_AC_METHOD = 0xB,
	UNI_CMD_SR_CFG_SR_PERIOD_THR = 0xC,
	UNI_CMD_SR_CFG_QUERY_TXD_METHOD = 0xD,
	UNI_CMD_SR_CFG_SR_SD_CG_RATIO = 0xE,
	UNI_CMD_SR_CFG_SR_SD_OBSS_RATIO = 0xF,
	UNI_CMD_SR_CFG_PROFILE = 0x10,
	UNI_CMD_SR_CFG_FNQ_ENABLE = 0x11,
	UNI_CMD_SR_CFG_DPD_ENABLE = 0x12,
	UNI_CMD_SR_CFG_SR_TX_ENABLE = 0x13,
	UNI_CMD_SR_CFG_SR_SD_OM_ENABLE = 0x14,
	UNI_CMD_SR_CFG_SR_TX_ALIGN_ENABLE = 0x15,
	UNI_CMD_SR_CFG_SR_TX_ALIGN_RSSI_THR = 0x16,
	UNI_CMD_SR_CFG_SR_DABS_MODE = 0x17,
	UNI_CMD_CSR_ENABLE = 0x18,
	UNI_CMD_CSR_FIX_PWR = 0x19,
	UNI_CMD_CSR_FIX_RATE = 0x1A,
	UNI_CMD_SR_CFG_SR_SR_RATE_OFFSET = 0x1B,
	UNI_CMD_SR_CFG_SR_SR_APCLI_MODE = 0x1C,
	UNI_CMD_SR_CFG_SR_SUPPORT_MODE = 0x1D,
    /*UNI_CMD_SR_CFG_RSV= 0x1D,*/
    /*UNI_CMD_SR_CFG_RSV= 0x1E,*/
    /*UNI_CMD_SR_CFG_RSV= 0x1F,*/
    /*UNI_CMD_SR_CFG_RSV= 0x20,*/
    /*UNI_CMD_SR_CFG_RSV= 0x21,*/
    /*UNI_CMD_SR_CFG_RSV= 0x22,*/
    /*UNI_CMD_SR_CFG_RSV= 0x23,*/
    /*UNI_CMD_SR_CFG_RSV= 0x24,*/
    /*UNI_CMD_SR_CFG_RSV= 0x25,*/
    /*UNI_CMD_SR_CFG_RSV= 0x26,*/
    /*UNI_CMD_SR_CFG_RSV= 0x27,*/
    /*UNI_CMD_SR_CFG_RSV= 0x28,*/
    /*UNI_CMD_SR_CFG_RSV= 0x29,*/
    /*UNI_CMD_SR_CFG_RSV= 0x2A,*/
    /*UNI_CMD_SR_CFG_RSV= 0x2B,*/
    /*UNI_CMD_SR_CFG_RSV= 0x2C,*/
    /*UNI_CMD_SR_CFG_RSV= 0x2D,*/
    /*UNI_CMD_SR_CFG_RSV= 0x2E,*/
    /*UNI_CMD_SR_CFG_RSV= 0x2F,*/
    /*UNI_CMD_SR_CFG_RSV= 0x30,*/
    /*UNI_CMD_SR_CFG_RSV= 0x31,*/
    /*UNI_CMD_SR_CFG_RSV= 0x32,*/
    /*UNI_CMD_SR_CFG_RSV= 0x33,*/
    /*UNI_CMD_SR_CFG_RSV= 0x34,*/
    /*UNI_CMD_SR_CFG_RSV= 0x35,*/
    /*UNI_CMD_SR_CFG_RSV= 0x36,*/
    /*UNI_CMD_SR_CFG_RSV= 0x37,*/
    /*UNI_CMD_SR_CFG_RSV= 0x38,*/
    /*UNI_CMD_SR_CFG_RSV= 0x39,*/
    /*UNI_CMD_SR_CFG_RSV= 0x3A,*/
    /*UNI_CMD_SR_CFG_RSV= 0x3B,*/
    /*UNI_CMD_SR_CFG_RSV= 0x3C,*/
    /*UNI_CMD_SR_CFG_RSV= 0x3D,*/
    /*UNI_CMD_SR_CFG_RSV= 0x3E,*/
    /*UNI_CMD_SR_CFG_RSV= 0x3F,*/
    /*UNI_CMD_SR_CFG_RSV= 0x40,*/
    /*UNI_CMD_SR_CFG_RSV= 0x41,*/
    /*UNI_CMD_SR_CFG_RSV= 0x42,*/
    /*UNI_CMD_SR_CFG_RSV= 0x43,*/
    /*UNI_CMD_SR_CFG_RSV= 0x44,*/
    /*UNI_CMD_SR_CFG_RSV= 0x45,*/
    /*UNI_CMD_SR_CFG_RSV= 0x46,*/
    /*UNI_CMD_SR_CFG_RSV= 0x47,*/
    /*UNI_CMD_SR_CFG_RSV= 0x48,*/
    /*UNI_CMD_SR_CFG_RSV= 0x49,*/
    /*UNI_CMD_SR_CFG_RSV= 0x4A,*/
    /*UNI_CMD_SR_CFG_RSV= 0x4B,*/
    /*UNI_CMD_SR_CFG_RSV= 0x4C,*/
    /*UNI_CMD_SR_CFG_RSV= 0x4D,*/
    /*UNI_CMD_SR_CFG_RSV= 0x4E,*/
    /*UNI_CMD_SR_CFG_RSV= 0x4F,*/
    /*UNI_CMD_SR_CFG_RSV= 0x50,*/
    /*UNI_CMD_SR_CFG_RSV= 0x51,*/
    /*UNI_CMD_SR_CFG_RSV= 0x52,*/
    /*UNI_CMD_SR_CFG_RSV= 0x53,*/
    /*UNI_CMD_SR_CFG_RSV= 0x54,*/
    /*UNI_CMD_SR_CFG_RSV= 0x55,*/
    /*UNI_CMD_SR_CFG_RSV= 0x56,*/
    /*UNI_CMD_SR_CFG_RSV= 0x57,*/
    /*UNI_CMD_SR_CFG_RSV= 0x58,*/
    /*UNI_CMD_SR_CFG_RSV= 0x59,*/
    /*UNI_CMD_SR_CFG_RSV= 0x5A,*/
    /*UNI_CMD_SR_CFG_RSV= 0x5B,*/
    /*UNI_CMD_SR_CFG_RSV= 0x5C,*/
    /*UNI_CMD_SR_CFG_RSV= 0x5D,*/
    /*UNI_CMD_SR_CFG_RSV= 0x5E,*/
    /*UNI_CMD_SR_CFG_RSV= 0x5F,*/
    /*UNI_CMD_SR_CFG_RSV= 0x60,*/
    /*UNI_CMD_SR_CFG_RSV= 0x61,*/
    /*UNI_CMD_SR_CFG_RSV= 0x62,*/
    /*UNI_CMD_SR_CFG_RSV= 0x63,*/
    /*UNI_CMD_SR_CFG_RSV= 0x64,*/
    /*UNI_CMD_SR_CFG_RSV= 0x65,*/
    /*UNI_CMD_SR_CFG_RSV= 0x66,*/
    /*UNI_CMD_SR_CFG_RSV= 0x67,*/
    /*UNI_CMD_SR_CFG_RSV= 0x68,*/
    /*UNI_CMD_SR_CFG_RSV= 0x69,*/
    /*UNI_CMD_SR_CFG_RSV= 0x6A,*/
    /*UNI_CMD_SR_CFG_RSV= 0x6B,*/
    /*UNI_CMD_SR_CFG_RSV= 0x6C,*/
    /*UNI_CMD_SR_CFG_RSV= 0x6D,*/
    /*UNI_CMD_SR_CFG_RSV= 0x6E,*/
    /*UNI_CMD_SR_CFG_RSV= 0x6F,*/
    /*UNI_CMD_SR_CFG_RSV= 0x70,*/
    /*UNI_CMD_SR_CFG_RSV= 0x71,*/
    /*UNI_CMD_SR_CFG_RSV= 0x72,*/
    /*UNI_CMD_SR_CFG_RSV= 0x73,*/
    /*UNI_CMD_SR_CFG_RSV= 0x74,*/
    /*UNI_CMD_SR_CFG_RSV= 0x75,*/
    /*UNI_CMD_SR_CFG_RSV= 0x76,*/
    /*UNI_CMD_SR_CFG_RSV= 0x77,*/
    /*UNI_CMD_SR_CFG_RSV= 0x78,*/
    /*UNI_CMD_SR_CFG_RSV= 0x79,*/
    /*UNI_CMD_SR_CFG_RSV= 0x7A,*/
    /*UNI_CMD_SR_CFG_RSV= 0x7B,*/
    /*UNI_CMD_SR_CFG_RSV= 0x7C,*/
    /*UNI_CMD_SR_CFG_RSV= 0x7D,*/
    /*UNI_CMD_SR_CFG_RSV= 0x7E,*/
    /*UNI_CMD_SR_CFG_RSV= 0x7F,*/
	UNI_CMD_SR_SW_SRG_BITMAP = 0x80,
	UNI_CMD_SR_SW_MESH_SRG_BITMAP = 0x81,
	UNI_CMD_SR_SW_SRG_BITMAP_REFRESH = 0x82,
	UNI_CMD_SR_SW_CNT = 0x83,
	UNI_CMD_SR_SW_SD = 0x84,
	UNI_CMD_SR_SW_GLOVAR_DROPTA_INFO = 0x85,
	UNI_CMD_SR_SW_GLOVAR_STA_INFO = 0x86,
	UNI_CMD_SR_GLOVAR_SINGLE_DROP_TA_INFO = 0x87,
	UNI_CMD_SR_UPDATE_SR_PARAMS = 0x88,
	UNI_CMD_CSR_COORDAP_INFO = 0x89,
	UNI_CMD_CSR_STA_INFO = 0x8B,
	UNI_CMD_CSR_COORDAP_RESET = 0x8C,
	UNI_CMD_CSR_EVENT_CNT = 0x8D,
	UNI_CMD_CSR_INFO = 0x8E,
	UNI_CMD_SR_DOWNLINK_STA_THRESHOLD = 0x8F,
	UNI_CMD_SR_BH_DOWNLINK_MESH_SR_THRESHOLD = 0x90,
	UNI_CMD_SR_FH_DOWNLINK_MESH_SR_THRESHOLD = 0x91,
	UNI_CMD_SR_SET_FORBIT_MESH_SR = 0x92,
	UNI_CMD_SR_BH_MESH_SR_BITMAP = 0x93,
	UNI_CMD_SR_FH_MESH_SR_BITMAP = 0x94,
	UNI_CMD_SR_FORHIB_MESH_SR_RESET = 0x95,
	UNI_CMD_SR_SW_SR_TX_COUNT = 0x96,
    /*UNI_CMD_SR_SW_RSV= 0x97,*/
    /*UNI_CMD_SR_SW_RSV= 0x98,*/
    /*UNI_CMD_SR_SW_RSV= 0x99,*/
    /*UNI_CMD_SR_SW_RSV= 0x9A,*/
    /*UNI_CMD_SR_SW_RSV= 0x9B,*/
    /*UNI_CMD_SR_SW_RSV= 0x9C,*/
    /*UNI_CMD_SR_SW_RSV= 0x9D,*/
    /*UNI_CMD_SR_SW_RSV= 0x9E,*/
    /*UNI_CMD_SR_SW_RSV= 0x9F,*/
    /*UNI_CMD_SR_SW_RSV= 0xA0,*/
    /*UNI_CMD_SR_SW_RSV= 0xA1,*/
    /*UNI_CMD_SR_SW_RSV= 0xA2,*/
    /*UNI_CMD_SR_SW_RSV= 0xA3,*/
    /*UNI_CMD_SR_SW_RSV= 0xA4,*/
    /*UNI_CMD_SR_SW_RSV= 0xA5,*/
    /*UNI_CMD_SR_SW_RSV= 0xA6,*/
    /*UNI_CMD_SR_SW_RSV= 0xA7,*/
    /*UNI_CMD_SR_SW_RSV= 0xA8,*/
    /*UNI_CMD_SR_SW_RSV= 0xA9,*/
    /*UNI_CMD_SR_SW_RSV= 0xAA,*/
    /*UNI_CMD_SR_SW_RSV= 0xAB,*/
    /*UNI_CMD_SR_SW_RSV= 0xAC,*/
    /*UNI_CMD_SR_SW_RSV= 0xAD,*/
    /*UNI_CMD_SR_SW_RSV= 0xAE,*/
    /*UNI_CMD_SR_SW_RSV= 0xAF,*/
    /*UNI_CMD_SR_SW_RSV= 0xB0,*/
    /*UNI_CMD_SR_SW_RSV= 0xB1,*/
    /*UNI_CMD_SR_SW_RSV= 0xB2,*/
    /*UNI_CMD_SR_SW_RSV= 0xB3,*/
    /*UNI_CMD_SR_SW_RSV= 0xB4,*/
    /*UNI_CMD_SR_SW_RSV= 0xB5,*/
    /*UNI_CMD_SR_SW_RSV= 0xB6,*/
    /*UNI_CMD_SR_SW_RSV= 0xB7,*/
    /*UNI_CMD_SR_SW_RSV= 0xB8,*/
    /*UNI_CMD_SR_SW_RSV= 0xB9,*/
    /*UNI_CMD_SR_SW_RSV= 0xBA,*/
    /*UNI_CMD_SR_SW_RSV= 0xBB,*/
    /*UNI_CMD_SR_SW_RSV= 0xBC,*/
    /*UNI_CMD_SR_SW_RSV= 0xBD,*/
    /*UNI_CMD_SR_SW_RSV= 0xBE,*/
    /*UNI_CMD_SR_SW_RSV= 0xBF,*/
	UNI_CMD_SR_HW_CAP = 0xC0,
	UNI_CMD_SR_HW_PARA = 0xC1,
	UNI_CMD_SR_HW_COND = 0xC2,
	UNI_CMD_SR_HW_RCPI_TBL = 0xC3,
	UNI_CMD_SR_HW_RCPI_TBL_OFST = 0xC4,
	UNI_CMD_SR_HW_Q_CTRL = 0xC5,
	UNI_CMD_SR_HW_IBPD = 0xC6,
	UNI_CMD_SR_HW_NRT = 0xC7,
	UNI_CMD_SR_HW_NRT_CTRL = 0xC8,
	UNI_CMD_SR_HW_NRT_RESET = 0xC9,
	UNI_CMD_SR_HW_CAP_SREN = 0xCA,
	UNI_CMD_SR_HW_IND = 0xCB,
	UNI_CMD_SR_HW_FNQ = 0xCC,
	UNI_CMD_SR_HW_FRMFILT = 0xCD,
	UNI_CMD_SR_HW_INTERPS_CTRL = 0xCE,
	UNI_CMD_SR_HW_INTERPS_DBG = 0xCF,
	UNI_CMD_SR_HW_SIGA_FLAG = 0xD0,
	UNI_CMD_SR_HW_SIGA_AUTO_FLAG = 0xD1,
	UNI_CMD_SR_HW_REMOTE_FH_RSSI = 0xD2,
	UNI_CMD_SR_HW_REMOTE_BH_INFO = 0xD3,
	UNI_CMD_SR_HW_MAP_TOPO = 0xD4,
	UNI_CMD_SR_HW_MAP_TRAFFIC_STATUS = 0xD5,
	UNI_CMD_SR_HW_MAP_BALANCE = 0xD6,
	UNI_CMD_SR_HW_MESH_PHASE = 0xD7,
	UNI_CMD_SR_HW_REMOTE_STA_MODE = 0xD8,
	UNI_CMD_SR_HW_MESH_UL_MODE = 0xD9,
	UNI_CMD_SR_HW_ENHANCE_SR_ENABLE = 0xDA,
	UNI_CMD_SR_HW_NONSRG_SR_ENABLE = 0xDB,
	UNI_CMD_SR_HW_OBSS_PD_THRESHOLD = 0xDC,
    /*UNI_CMD_SR_HW_RSV= 0xDD,*/
    /*UNI_CMD_SR_HW_RSV= 0xDE,*/
    /*UNI_CMD_SR_HW_RSV= 0xDF,*/
    /*UNI_CMD_SR_HW_RSV= 0xE0,*/
    /*UNI_CMD_SR_HW_RSV= 0xE1,*/
    /*UNI_CMD_SR_HW_RSV= 0xE2,*/
    /*UNI_CMD_SR_HW_RSV= 0xE3,*/
    /*UNI_CMD_SR_HW_RSV= 0xE4,*/
    /*UNI_CMD_SR_HW_RSV= 0xE5,*/
    /*UNI_CMD_SR_HW_RSV= 0xE6,*/
    /*UNI_CMD_SR_HW_RSV= 0xE7,*/
    /*UNI_CMD_SR_HW_RSV= 0xE8,*/
    /*UNI_CMD_SR_HW_RSV= 0xE9,*/
    /*UNI_CMD_SR_HW_RSV= 0xEA,*/
    /*UNI_CMD_SR_HW_RSV= 0xEB,*/
    /*UNI_CMD_SR_HW_RSV= 0xEC,*/
    /*UNI_CMD_SR_HW_RSV= 0xED,*/
    /*UNI_CMD_SR_HW_RSV= 0xEE,*/
    /*UNI_CMD_SR_HW_RSV= 0xEF,*/
    /*UNI_CMD_SR_HW_RSV= 0xF0,*/
    /*UNI_CMD_SR_HW_RSV= 0xF1,*/
    /*UNI_CMD_SR_HW_RSV= 0xF2,*/
    /*UNI_CMD_SR_HW_RSV= 0xF3,*/
    /*UNI_CMD_SR_HW_RSV= 0xF4,*/
    /*UNI_CMD_SR_HW_RSV= 0xF5,*/
    /*UNI_CMD_SR_HW_RSV= 0xF6,*/
    /*UNI_CMD_SR_HW_RSV= 0xF7,*/
    /*UNI_CMD_SR_HW_RSV= 0xF8,*/
    /*UNI_CMD_SR_HW_RSV= 0xF9,*/
    /*UNI_CMD_SR_HW_RSV= 0xFA,*/
    /*UNI_CMD_SR_HW_RSV= 0xFB,*/
    /*UNI_CMD_SR_HW_RSV= 0xFC,*/
    /*UNI_CMD_SR_HW_RSV= 0xFD,*/
    /*UNI_CMD_SR_HW_RSV= 0xFE,*/
    /*UNI_CMD_SR_HW_RSV= 0xFF,*/
};

struct UNI_CMD_SR_CMD_T {
    /* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
    /* DW_1 */
	UINT_32 u4Value;
};

struct UNI_CMD_SR_T {
    /*Fixed Fields*/
	UINT_8 u1BandIdx;
	UINT_8 au1Padding[3];
    /*TLV*/
	UINT_8 au1TlvBuffer[0];
};

struct UNI_CMD_SR_SW_SRG_BITMAP_T {
    /* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
    /* DW_1 - DW4*/
	struct SR_SRG_BITMAP_T rSrSrgBitmap[CFG_WIFI_RAM_BAND_NUM];
};

struct UNI_CMD_SR_SW_GLOVAR_DROPTA_INFO_T {
    /* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
    /*DROPTA_INFO*/
    /* DW_1 */
	UINT_32 u4Address2;
    /* DW_2 */
	UINT_8  u1DropTaIdx;
	UINT_8  RSV[3];
};

struct UNI_CMD_SR_SW_GLOVAR_STA_INFO_T {
    /* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
    /*STA_INFO*/
    /* DW_1 */
	BOOLEAN fgIsInit;
	UINT_8  u1DropTaIdx;
	UINT_8  u1StaIdx;
	UINT_8  u1SrRateOffset;
    /* DW_2 */
	UINT_16 u2WlanId;
	UINT_8  RSV[2];
};

struct UNI_CMD_SR_SR_GLOBAL_VAR_SINGLE_DROP_TA_T {
    /* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
    /*DROPTA_INFO*/
    /* DW_1 */
	UINT_8 u1StaIdx;
	UINT_8  u1DropTaIdx;
	UINT_8  RSV[2];
};

/* SR Capability */
/*UNI_CMD_SR_HW_CAP(Tag=0xC0)*/
struct UNI_CMD_SR_HW_CAP_T {
	/* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
	/* RMAC */
	/* DW_1 - DW_5 */
	struct WH_SR_CAP_T_SR_V2 rSrCap;
};

struct UNI_SR_CMD_SR_PARA_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_PARA_T rSrPara;
};

struct UNI_CMD_SR_COND_T_SR_V1 {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct SR_COND_T_SR_V1 rSrCond;
};

struct UNI_CMD_SR_COND_T_SR_V2 {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_COND_T_SR_V2 rSrCond;
};

struct UNI_CMD_SR_RCPITBL_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_RCPITBL_T rSrRcpiTbl;
};

struct UNI_CMD_SR_RCPITBL_OFST_T_SR_V2 {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_RCPITBL_OFST_T_SR_V2 rSrRcpiTblOfst;
};

struct UNI_CMD_SR_Q_CTRL_T_SR_V2 {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_QUEUE_CTRL_T_SR_V2 rSrQCtrl;
};


struct UNI_CMD_SR_IBPD_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_IBPD_T rSrIBPD;
};

struct UNI_CMD_SR_NRT_T_SR_V2 {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_NRT_T_SR_V2 rSrNRT;
};

struct UNI_CMD_SR_NRT_CTRL_T_SR_V2 {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_NRT_CTRL_T_SR_V2 rSrNRTCtrl;
};

struct UNI_CMD_SR_FNQ_CTRL_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_FNQ_CTRL_T rSrFNQCtrl;
};

struct UNI_CMD_SR_INTERPS_CTRL_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct WH_SR_INTERPS_CTRL_T rSrInterPsCtrl;
};

struct UNI_CMD_SR_SIGA_FLAG_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct SR_SIGA_FLAG_T rSrSigaFlag;
};

struct UNI_CMD_SR_SIGA_AUTO_FLAG_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct _SR_SIGA_AUTO_FLAG_T rSrSigaAutoFlag;
};

struct UNI_CMD_MESH_TOPOLOGY_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	union _SR_MESH_TOPOLOGY_T rSrCmdMeshTopo;
};

struct UNI_CMD_SR_UL_TRAFFIC_STATUS_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct _SR_UL_STATUS_T rSrUlStatus;
};

struct UNI_CMD_SET_MAP_BALANCE_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	struct _SR_CMD_MAP_BALANCE_T rSrMapBalance;
};

struct UNI_CMD_SR_MESH_DL_STA_MESH_THRESHOLD_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	struct SR_MESH_SR_DL_STA_THRESHOLD rSrDLStaThrehsold;
};

struct UNI_CMD_CSR_RESET_AP_STA_INFO_T {
	/* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
	/* DW_1 */
	UINT_8 u1ResetTarget;
	UINT_8 u1ResetApIndex;
	UINT_8 u1ResetStaIndex;
	UINT_8 ucPadding;
};

struct UNI_CMD_CSR_APINFO {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 apId;
	UINT_8 CoorAPStatus;
	UINT_8 aCoorAPBSSID[MAC_ADDR_LEN];
	UINT_8 u1Rssi;
	UINT_8 Rsv[3];
};

struct UNI_CMD_CSR_STAINFO {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_16 u1Wcid;
	UINT_8 u1Candstaid;
	UINT_8 u1MLDid;
	UINT_8 u1Pldiff[3];
	UINT_8 u180211ksupport;
	UINT_8 u1status;
	UINT_8 Rsv[3];
};

struct UNI_CMD_CSR_INFO_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	UINT_8 u1STAid;
	UINT_8 u1APid;
	UINT_8 Recv[2];
};

struct UNI_CMD_CSR_FIXED_TXPOWER_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	UINT_8 u1STAid;
	UINT_8 u1APid;
	UINT_8 u1Mode;
	UINT_8 u1FixedTXpower;
};

struct UNI_CMD_CSR_FIXED_RATE_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	UINT_8 u1STAid;
	UINT_8 u1APid;
	UINT_8 u1Mode;
	UINT_8 u1MCS;
};

struct UNI_CMD_SR_SW_GLOVAR_STA_REC_INFO_T {
	/* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
	/*STA_INFO*/
	/* DW_1 */
	UINT_16 u2WlanId;
	UINT_8  RSV[2];
};

#endif/* CFG_SUPPORT_FALCON_SR*/

/* ====================== UNI_CMD_ID_MLO Begin ======================*/
#define MLO_CMD_SET_MLOAGCTX_ARG_NUM 7
#define MLO_CMD_SET_MLOAGCTRIG_ARG_NUM 6
#define MLO_CMD_SET_MLOOPTIONCTRL_ARG_NUM 2
#define MAX_MLO_MGMT_SUPPORT_MLD_NUM 32
#define MAX_MLO_MGMT_SUPPORT_AC_NUM 4

/* MLO Command Tag ID */
enum UNI_CMD_MLO_TAG_T {
	UNI_CMD_MLO_RSV = 0x0,
	UNI_CMD_MLO_MLO_MGMT = 0x1,
	UNI_CMD_MLO_MLD_REC = 0x2,
	UNI_CMD_MLO_MLD_REC_LINK = 0x3,
	UNI_CMD_MLO_MLD_REC_LINK_AGC_TX = 0x4,
	UNI_CMD_MLO_MLD_REC_LINK_AGC_TRIG = 0x5,
	UNI_CMD_MLO_CONFIG_OPERATION = 0x6,
	UNI_CMD_MLO_OPTION_CTRL = 0x7,
};

struct UNI_CMD_MLO_T {
	/*Fixed Fields*/
	UINT_8 u1BandIdx;
	UINT_8 au1Padding[3];
	/*TLV*/
	UINT_8 au1TlvBuffer[0];
};

struct UNI_CMD_MLO_CMD_T {
	/* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
	/* DW_1 */
	UINT_32 u4Value;
};

struct UNI_CMD_MLO_MLD_REC_LINK_AGC_TX_T {
	/* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
	/* DW_1*/
	UINT_8 u1MldRecIdx;
	UINT_8 u1MldRecLinkIdx;
	UINT_8 u1AcIdx;
	UINT_8 u1DispPolTx;
	/* DW_2*/
	UINT_8 u1DispRatioTx;
	UINT_8 u1DispOrderTx;
	UINT_16 u2DispMgfTx;
};

struct UNI_CMD_MLO_MLD_REC_LINK_AGC_TRIG_T {
	/* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
	/* DW_1*/
	UINT_8 u1MldRecIdx;
	UINT_8 u1MldRecLinkIdx;
	UINT_8 u1AcIdx;
	UINT_8 u1DispPolTrig;
	/* DW_2*/
	UINT_8 u1DispRatioTrig;
	UINT_8 u1Rsv;
	UINT_16 u2DispMgfTrig;
};

/**
 * This structure is used for UNI_CMD_MLO_CONFIG_OPERATION(0x06) tag of UNI_CMD_ID_MLO command
 * to configure MLD operation.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag        should be 0x06
 * @param[in] u2Length     the length of this TLV, should be sizeof(UNI_CMD_MLO_CONFIG_OPERATION_T)
 * @param[in] ucCapConfigBitMask    mlo capability config bitmask. Each bit is
 *                                  mapping to different mlo parameters.
 *                         -bit[0] = 1: config ucStrBitmap and ucSyncTxEnable parameters
 *                         -bit[1] = 1: config ucAadNonceLinkAddr parameters
 * @param[in] ucStrBitmap  -bit[0]: set 1 if mlo operation band0 and band1 str link
 *                                : set o if mlo operation band0 and band1 nstr link
 *                         -bit[1]: set 1 if mlo operation band0 and band2 str link
 *                                : set o if mlo operation band0 and band2 nstr link
 *                         -bit[2]: set 1 if mlo operation band1 and band2 str link
 *                                : set o if mlo operation band1 and band2 nstr link
 *                         -other bits: reserved
 * @param[in] ucSyncTxEnable    Sync Tx enable
 * @param[in] ucAadNonceLinkAddr    construct AAD/Nonce for unicast
 *                          - set 1 to use link address
 *                          - set 0 to use mld address
 */
/*UNI_CMD_MLO_CONFIG_OPERATION(Tag=0x06)*/
struct UNI_CMD_MLO_CONFIG_OPERATION_T {
	/* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
	/* DW_1-3*/
	UINT_8 ucCapConfigBitMask;
	UINT_8 ucStrBitmap;
	UINT_8 ucSyncTxEnable;
	UINT_8 ucAadNonceLinkAddr;
	UINT_8 aucReserved[8];
};

struct UNI_CMD_MLO_OPTION_CTRL_T {
/* DW_0 */
UINT_16 u2Tag;
UINT_16 u2Length;
/* DW_1*/
UINT_8 u1OptionType;
UINT_8 u1Value;
UINT_8 u1Rsv[2];
};

/* ====================== UNI_CMD_ID_MLO END ======================*/


/* ================= UNI_CMD_ID_LOCATION Begin ================= */
#ifdef FTM_SUPPORT
/** @addtogroup UNI_CMD_ID_LOCATION
 * @{
 *  @page page_loc
 *     <br/>
 *     RTT Command <br/>
 */
/**
 * This structure is used for UNI_CMD_ID_LOCATION command (0x5d) for debug.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] ucReserved
 * @param[in] au1TlvBuffer     TLVs
 *
 */
typedef struct _UNI_CMD_ID_LOCATION_T {
	/* fixed field */
	UINT_8 ucReserved[4];

	/* tlv */
	UINT_8 aucTlvBuffer[0];/**< the TLVs included in this field:
	*
	*   TAG                                |  ID  | structure
	*   -----------------------------------|------|--------------------------------------
	*   UNI_CMD_LOC_TAG_GET_CAPA           | 0x00 | struct _UNI_CMD_RTT_GET_CAPA_T
	*   UNI_CMD_LOC_TAG_RANGE_REQ_MC       | 0x01 | struct _UNI_CMD_RTT_RANGE_REQ_MC_T
	*   UNI_CMD_LOC_TAG_RANGE_REQ_RSP      | 0x02 | struct _UNI_CMD_RTT_RANGE_REQ_RSP_T
	*   UNI_CMD_LOC_TAG_RSP_ENABLE         | 0x05 | struct _UNI_CMD_LOC_TAG_RSP_ENABLE_T
	*   UNI_CMD_LOC_TAG_DEBUG              | 0x06 | struct _UNI_CMD_LOC_TAG_DEBUG_T
	*   UNI_CMD_LOC_TAG_RANGE_REQ_MC_BURST | 0x07 | struct _UNI_CMD_LOC_TAG_RANGE_REQ_MC_BURST_T
	*   UNI_CMD_LOC_TAG_TOAE_CAL_CFG       | 0x08 | struct _UNI_CMD_LOC_TAG_TOAE_CAL_CFG_T
	*/
} UNI_CMD_ID_LOCATION_T, *P_UNI_CMD_ID_LOCATION_T;
/** @} */


/* LOC Command Tag ID */
typedef enum _UNI_CMD_LOC_TAG_T {
	UNI_CMD_LOC_TAG_GET_CAPA = 0,
	UNI_CMD_LOC_TAG_RANGE_REQ_MC = 1,
	UNI_CMD_LOC_TAG_RANGE_REQ_RSP = 2,
	UNI_CMD_LOC_TAG_RSP_ENABLE = 5,
	UNI_CMD_LOC_TAG_DEBUG = 6,
	UNI_CMD_LOC_TAG_RANGE_REQ_MC_BURST = 7,
	UNI_CMD_LOC_TAG_TOAE_CAL_CFG = 8,
	UNI_CMD_LOC_TAG_NUM
} UNI_CMD_LOC_TAG_T;

/** @addtogroup UNI_CMD_ID_LOCATION
 * @{
 */
/**
 * This structure is used for UNI_CMD_ID_RTT command (0x5d) to get capabilities.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag
 * @param[in] u2Length     the length of this TLV, should be 8
 */
/*UNI_CMD_LOC_TAG_GET_CAPA(Tag=0x00)*/
typedef struct _UNI_CMD_LOC_TAG_GET_CAPA_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
} UNI_CMD_LOC_TAG_GET_CAPA_T, *P_UNI_CMD_LOC_TAG_GET_CAPA_T;

typedef INT32 (*PFN_LOC_TAG_GET_CAPA_HANDLE)(struct _RTMP_ADAPTER *pAd,
										VOID *pHandle);
/** @} */

/** @addtogroup UNI_CMD_ID_LOCATION
 * @{
 */
/**
 * This structure is used for UNI_CMD_LOC_TAG_RANGE_REQ_MC tag of UNI_CMD_ID_LOCATION command (0x5d)
 * to set range request.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag        should be 0x01
 * @param[in] u2Length     the length of this TLV, should be
 * @param[in]
 */
/*UNI_CMD_LOC_TAG_RANGE_REQ_MC(Tag=0x02)*/


struct _UNI_CMD_LOC_TAG_RANGE_REQ_MC_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8  ucSeqNum;
	UINT_8  fgEnable;              /* request or cancel */
	UINT_8  ucConfigNum;
	UINT_8  ucPadding;
	LOC_CONFIG_T arLocConfigs[CFG_LOC_MAX_CANDIDATES];
};

typedef INT32 (*PFN_LOC_TAG_RANGE_REQ_MC_HANDLE)(struct _RTMP_ADAPTER *pAd,
										UINT_8  ucSeqNum,
										UINT_8  fgEnable,
										UINT_8  ucConfigNum,
										VOID *pHandle);
/** @} */

/** @addtogroup UNI_CMD_ID_LOCATION
 * @{
 */
/**
 * This structure is used for UNI_CMD_LOC_TAG_RANGE_REQ_RSP tag of
 * UNI_CMD_ID_LOCATION command (0x5d) to set range request.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag        should be 0x02
 * @param[in] u2Length     the length of this TLV, should be
 * @param[in]
 */
/*UNI_CMD_LOC_TAG_RANGE_REQ_RSP(Tag=0x02)*/
struct _UNI_CMD_LOC_TAG_RANGE_REQ_RSP_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8  ucSeqNum;
	/* fgEnable = 0 (trigger=0)
	   fgEnable = 1 (trigger=1, 1st FTMR (with FTM params))
	   fgEnable = 2 (trigger=1, 2nd or later FTMR (without FTM params)) */
	UINT_8  fgEnable;
	UINT_8  ucConfigNum;
	UINT_8  ucPadding;
	LOC_RSTA_CONFIG_T arLocRspConfigs[CFG_LOC_RSTA_MAX_CANDIDATES];
};

typedef INT32 (*PFN_LOC_TAG_RANGE_REQ_RSP_HANDLE)(struct _RTMP_ADAPTER *pAd,
										struct wifi_dev *wdev,
										UINT_8  fgEnable,
										UINT_8  ucConfigNum,
										VOID *pHandle,
										PFTM_PEER_ENTRY pEntry);

/** @} */

/**
 * This structure is used for UNI_CMD_LOC_TAG_RSP_ENABLE tag of
 * UNI_CMD_ID_LOCATION command (0x5d) to set range request.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag        should be 0x05
 * @param[in] u2Length     the length of this TLV, should be
 */
/*UNI_CMD_LOC_TAG_RSP_ENABLE(Tag=0x05)*/
struct _UNI_CMD_LOC_TAG_RSP_ENABLE_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 fgEnable;
	UINT_8 ucBandIdx;
	UINT_8 aucpadding[2];
};

typedef INT32 (*PFN_LOC_TAG_RSP_ENABLE_HANDLE)(struct _RTMP_ADAPTER *pAd,
												UINT_8  fgEnable,
												VOID *pHandle);

struct _UNI_CMD_LOC_TAG_DEBUG_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_32 u4Debug[CFG_LOC_DBG_NUM];
};

typedef INT32 (*PFN_LOC_TAG_DEDUG_HANDLE)(struct _RTMP_ADAPTER *pAd,
										struct wifi_dev *wdev,
										VOID *pHandle);

/**
 * This structure is used for UNI_CMD_LOC_TAG_RANGE_REQ_MC_BURST tag of
 * UNI_CMD_ID_LOCATION command (0x5d) to set range request.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag        should be 0x07
 * @param[in] u2Length     the length of this TLV, should be
 */
/*UNI_CMD_LOC_TAG_RANGE_REQ_MC_BURST(Tag=0x07)*/
struct _UNI_CMD_LOC_TAG_RANGE_REQ_MC_BURST_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 aucAddr[MAC_ADDR_LEN];
	UINT_8 eType;
	UINT_8 ucBssIndex;
	WIFI_CHANNEL_INFO_T rChannel;
};

typedef INT32 (*PFN_LOC_TAG_RANGE_REQ_MC_BURST_HANDLE)(struct _RTMP_ADAPTER *pAd,
														struct wifi_dev *wdev,
														VOID *pHandle);


struct _UNI_CMD_LOC_TAG_TOAE_CAL_CFG_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	/* TRUE: use the configuration of this command
	 * FALSE: use default configuration, ignore the values of this command */
	BOOLEAN fgBiasTuning;
	UINT_8  u1AntIdx;       /* mac antenna index */
	UINT_8  u1SpeIdxFtm;    /* 0: wf0 / 1: wf1 */
	UINT_8  u1SpeIdxFtmAck; /* 0: wf0 / 1: wf1 */
	UINT_16 u2ChainSel;     /* 0x0000 = T0R0 / 0x1111 = T0R1 */
	UINT_8  aucpadding[2];
};

typedef INT32 (*PFN_LOC_TAG_TOAE_CAL_CFG_HANDLE)(struct _RTMP_ADAPTER *pAd,
										VOID *pHandle);

#endif /* FTM_SUPPORT */
/* ================== UNI_CMD_ID_LOCATION End ================== */


/* ====================== UNI_CMD_ID_MIB Begin ======================*/
/* MIB command Tag */
typedef enum _UNI_CMD_MIB_TAG_T {
    UNI_CMD_MIB_DATA = 0,
    UNI_CMD_MIB_MAX_NUM
} UNI_CMD_MIB_TAG_T;

/* MIB command (0x22) */
typedef struct GNU_PACKED _UNI_CMD_MIB_T {
    /* fixed field */
    UINT8 ucBand;          /* operation band */
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_MIB_T, *P_UNI_CMD_MIB_T;

/* MIB Config Parameters (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_MIB_DATA_T {
    UINT16 u2Tag;      /* should be 0x00 */
    UINT16 u2Length;   /* the length of this TLV */
    UINT32 u4Counter;  /* MIB ID of demanded MIB counter */
} UNI_CMD_MIB_DATA_T, *P_UNI_CMD_MIB_DATA_T;

#define UNI_CMD_MIB_MAX_PAIR 15
enum ENUM_UNI_CMD_MIB_COUNTER_T {
	UNI_CMD_MIB_CNT_RX_FCS_ERR = 0,
	UNI_CMD_MIB_CNT_RX_FIFO_OVERFLOW = 1,
	UNI_CMD_MIB_CNT_RX_MPDU = 2,
	UNI_CMD_MIB_CNT_AMDPU_RX_COUNT = 3,
	UNI_CMD_MIB_CNT_RX_TOTAL_BYTE = 4,
	UNI_CMD_MIB_CNT_RX_VALID_AMPDU_SF = 5,
	UNI_CMD_MIB_CNT_RX_VALID_BYTE = 6,
	UNI_CMD_MIB_CNT_CHANNEL_IDLE = 7,
	UNI_CMD_MIB_CNT_VEC_DROP = 8,
	UNI_CMD_MIB_CNT_DELIMITER_FAIL = 9,
	UNI_CMD_MIB_CNT_VEC_MISMATCH = 10,
	UNI_CMD_MIB_CNT_MDRDY = 11,
	UNI_CMD_MIB_CNT_RX_CCK_MDRDY_TIME = 12,
	UNI_CMD_MIB_CNT_RX_OFDM_LG_MIXED_MDRDY_TIME = 13,
	UNI_CMD_MIB_CNT_RX_OFDM_GREEN_MDRDY_TIME = 14,
	UNI_CMD_MIB_CNT_PF_DROP = 15,
	UNI_CMD_MIB_CNT_LEN_MISMATCH = 16,
	UNI_CMD_MIB_CNT_P_CCA_TIME = 17,
	UNI_CMD_MIB_CNT_S_CCA_TIME = 18,
	UNI_CMD_MIB_CNT_CCA_NAV_TX_TIME = 19,
	UNI_CMD_MIB_CNT_P_ED_TIME = 20,
	UNI_CMD_MIB_CNT_BCN_TX = 21,
	UNI_CMD_MIB_CNT_TX_BW_20MHZ = 22,
	UNI_CMD_MIB_CNT_TX_BW_40MHZ = 23,
	UNI_CMD_MIB_CNT_TX_BW_80MHZ = 24,
	UNI_CMD_MIB_CNT_TX_BW_160MHZ = 25,
	UNI_CMD_RMAC_CNT_OBSS_AIRTIME = 26,
	UNI_CMD_RMAC_CNT_NONWIFI_AIRTIME = 27,
	UNI_CMD_MIB_CNT_TX_DUR_CNT = 28,
	UNI_CMD_MIB_CNT_RX_DUR_CNT = 29,
	UNI_CMD_MIB_CNT_BA_CNT = 30,
	UNI_CMD_MIB_CNT_MAC2PHY_TX_TIME = 31,
	UNI_CMD_MIB_CNT_RX_OUT_OF_RANGE_COUNT = 32,
	UNI_CMD_MIB_CNT_IBF_TX = 33,
	UNI_CMD_MIB_CNT_EBF_TX = 34,
	UNI_CMD_MIB_CNT_MUBF_TX = 35,
	UNI_CMD_MIB_CNT_RX_BF_VHTFBK = 36,
	UNI_CMD_MIB_CNT_RX_BF_HTFBK = 37,
	UNI_CMD_MIB_CNT_RX_BF_HEFBK = 38,
	UNI_CMD_MIB_CNT_BFEE_TXFBK_BFPOLL_TRI = 39,
	UNI_CMD_MIB_CNT_BFEE_TXFBK_TRI = 40,
	UNI_CMD_MIB_CNT_BFEE_RX_FBKCQI = 41,
	UNI_CMD_MIB_CNT_BFEE_RX_NDP = 42,
	UNI_CMD_MIB_CNT_RX_BF_IBF_UPT = 43,
	UNI_CMD_MIB_CNT_RX_BF_EBF_UPT = 44,
	UNI_CMD_MIB_CNT_BFEE_SP_ABORT = 45,
	UNI_CMD_MIB_CNT_BFEE_TB_LEN_ERR = 46,
	UNI_CMD_MIB_CNT_BFEE_TXFBK_MUTE = 47,
	UNI_CMD_MIB_CNT_BFEE_TMAC_ABORT = 48,
	UNI_CMD_MIB_CNT_BFEE_TXFBK_CPL = 49,
	UNI_CMD_MIB_CNT_BFEE_COANT_BLKTX = 50,
	UNI_CMD_MIB_CNT_BFEE_FBK_SEG = 51,
	UNI_CMD_MIB_CNT_NAV_TIME = 52,
	UNI_CMD_MIB_CNT_TX_ABORT_CNT1 = 53,
	UNI_CMD_MIB_CNT_TX_ABORT_CNT0 = 54,
	UNI_CMD_MIB_CNT_TX_ABORT_CNT3 = 55,
	UNI_CMD_MIB_CNT_TX_ABORT_CNT2 = 56,
	UNI_CMD_MIB_CNT_BSS0_RTS_TX_CNT = 57,
	UNI_CMD_MIB_CNT_BSS1_RTS_TX_CNT = 58,
	UNI_CMD_MIB_CNT_BSS2_RTS_TX_CNT = 59,
	UNI_CMD_MIB_CNT_BSS3_RTS_TX_CNT = 60,
	UNI_CMD_MIB_CNT_BSS0_RTS_RETRY = 61,
	UNI_CMD_MIB_CNT_BSS1_RTS_RETRY = 62,
	UNI_CMD_MIB_CNT_BSS2_RTS_RETRY = 63,
	UNI_CMD_MIB_CNT_BSS3_RTS_RETRY = 64,
	UNI_CMD_MIB_CNT_BSS0_BA_MISS = 65,
	UNI_CMD_MIB_CNT_BSS1_BA_MISS = 66,
	UNI_CMD_MIB_CNT_BSS2_BA_MISS = 67,
	UNI_CMD_MIB_CNT_BSS3_BA_MISS = 68,
	UNI_CMD_MIB_CNT_BSS0_ACK_FAIL = 69,
	UNI_CMD_MIB_CNT_BSS1_ACK_FAIL = 70,
	UNI_CMD_MIB_CNT_BSS2_ACK_FAIL = 71,
	UNI_CMD_MIB_CNT_BSS3_ACK_FAIL = 72,
	UNI_CMD_MIB_CNT_BSS0_FRAME_RETRY = 73,
	UNI_CMD_MIB_CNT_BSS1_FRAME_RETRY = 74,
	UNI_CMD_MIB_CNT_BSS2_FRAME_RETRY = 75,
	UNI_CMD_MIB_CNT_BSS3_FRAME_RETRY = 76,
	UNI_CMD_MIB_CNT_BSS0_FRAME_RETRY_2 = 77,
	UNI_CMD_MIB_CNT_BSS1_FRAME_RETRY_2 = 78,
	UNI_CMD_MIB_CNT_BSS2_FRAME_RETRY_2 = 79,
	UNI_CMD_MIB_CNT_BSS3_FRAME_RETRY_2 = 80,
	UNI_CMD_MIB_CNT_RX_A1_SEARCH = 81,
	UNI_CMD_MIB_CNT_RX_DROP_MPDU = 82,
	UNI_CMD_MIB_CNT_RX_UNWANTED = 83,
	UNI_CMD_MIB_CNT_RX_FCS_OK = 84,
	UNI_CMD_MIB_CNT_SU_TX_OK = 85,
	UNI_CMD_MIB_CNT_TX_FULL_BW = 86,
	UNI_CMD_MIB_CNT_TX_AUTO_BW = 87,
	UNI_CMD_MIB_CNT_CCA_SAMPLE_IDLE = 88,
	UNI_CMD_MIB_CNT_CCA_SAMPLE_ACTIVE = 89,
	UNI_CMD_MIB_CNT_CCA_TIME = 90,
	UNI_CMD_MIB_CNT_S_20BW_CCA_TIME = 91,
	UNI_CMD_MIB_CNT_S_40BW_CCA_TIME = 92,
	UNI_CMD_MIB_CNT_S_80BW_CCA_TIME = 93,
	UNI_CMD_MIB_CNT_S_160BW_CCA_TIME = 94,
	UNI_CMD_MIB_CNT_S_P20BW_0_ED_TIME = 95,
	UNI_CMD_MIB_CNT_S_P20BW_1_ED_TIME = 96,
	UNI_CMD_MIB_CNT_S_P20BW_2_ED_TIME = 97,
	UNI_CMD_MIB_CNT_S_P20BW_3_ED_TIME = 98,
	UNI_CMD_MIB_CNT_S_P20BW_4_ED_TIME = 99,
	UNI_CMD_MIB_CNT_S_P20BW_5_ED_TIME = 100,
	UNI_CMD_MIB_CNT_S_P20BW_6_ED_TIME = 101,
	UNI_CMD_MIB_CNT_S_P20BW_7_ED_TIME = 102,
	UNI_CMD_MIB_CNT_S_P20BW_8_ED_TIME = 103,
	UNI_CMD_MIB_CNT_S_P20BW_9_ED_TIME = 104,
	UNI_CMD_MIB_CNT_S_P20BW_10_ED_TIME = 105,
	UNI_CMD_MIB_CNT_S_P20BW_11_ED_TIME = 106,
	UNI_CMD_MIB_CNT_S_P20BW_12_ED_TIME = 107,
	UNI_CMD_MIB_CNT_S_P20BW_13_ED_TIME = 108,
	UNI_CMD_MIB_CNT_S_P20BW_14_ED_TIME = 109,
	UNI_CMD_MIB_CNT_S_P20BW_15_ED_TIME = 110,
	UNI_CMD_MIB_CNT_BSS0_MGMT_RETRY = 111,
	UNI_CMD_MIB_CNT_BSS1_MGMT_RETRY = 112,
	UNI_CMD_MIB_CNT_BSS2_MGMT_RETRY = 113,
	UNI_CMD_MIB_CNT_BSS3_MGMT_RETRY = 114,
	UNI_CMD_MIB_CNT_MBSS0_MGMT_RETRY = 115,
	UNI_CMD_MIB_CNT_MBSS1_MGMT_RETRY = 116,
	UNI_CMD_MIB_CNT_MBSS2_MGMT_RETRY = 117,
	UNI_CMD_MIB_CNT_MBSS3_MGMT_RETRY = 118,
	UNI_CMD_MIB_CNT_MBSS4_MGMT_RETRY = 119,
	UNI_CMD_MIB_CNT_MBSS5_MGMT_RETRY = 120,
	UNI_CMD_MIB_CNT_MBSS6_MGMT_RETRY = 121,
	UNI_CMD_MIB_CNT_MBSS7_MGMT_RETRY = 122,
	UNI_CMD_MIB_CNT_MBSS8_MGMT_RETRY = 123,
	UNI_CMD_MIB_CNT_MBSS9_MGMT_RETRY = 124,
	UNI_CMD_MIB_CNT_MBSS10_MGMT_RETRY = 125,
	UNI_CMD_MIB_CNT_MBSS11_MGMT_RETRY = 126,
	UNI_CMD_MIB_CNT_MBSS12_MGMT_RETRY = 127,
	UNI_CMD_MIB_CNT_MBSS13_MGMT_RETRY = 128,
	UNI_CMD_MIB_CNT_MBSS14_MGMT_RETRY = 129,
	UNI_CMD_MIB_CNT_MBSS15_MGMT_RETRY = 130,
	UNI_CMD_MIB_CNT_BSS0_CTRL_FRAME_CNT = 131,
	UNI_CMD_MIB_CNT_BSS1_CTRL_FRAME_CNT = 132,
	UNI_CMD_MIB_CNT_BSS2_CTRL_FRAME_CNT = 133,
	UNI_CMD_MIB_CNT_BSS3_CTRL_FRAME_CNT = 134,
	UNI_CMD_MIB_CNT_MBSS0_CTRL_FRAME_CNT = 135,
	UNI_CMD_MIB_CNT_MBSS1_CTRL_FRAME_CNT = 136,
	UNI_CMD_MIB_CNT_MBSS2_CTRL_FRAME_CNT = 137,
	UNI_CMD_MIB_CNT_MBSS3_CTRL_FRAME_CNT = 138,
	UNI_CMD_MIB_CNT_MBSS4_CTRL_FRAME_CNT = 139,
	UNI_CMD_MIB_CNT_MBSS5_CTRL_FRAME_CNT = 140,
	UNI_CMD_MIB_CNT_MBSS6_CTRL_FRAME_CNT = 141,
	UNI_CMD_MIB_CNT_MBSS7_CTRL_FRAME_CNT = 142,
	UNI_CMD_MIB_CNT_MBSS8_CTRL_FRAME_CNT = 143,
	UNI_CMD_MIB_CNT_MBSS9_CTRL_FRAME_CNT = 144,
	UNI_CMD_MIB_CNT_MBSS10_CTRL_FRAME_CNT = 145,
	UNI_CMD_MIB_CNT_MBSS11_CTRL_FRAME_CNT = 146,
	UNI_CMD_MIB_CNT_MBSS12_CTRL_FRAME_CNT = 147,
	UNI_CMD_MIB_CNT_MBSS13_CTRL_FRAME_CNT = 148,
	UNI_CMD_MIB_CNT_MBSS14_CTRL_FRAME_CNT = 149,
	UNI_CMD_MIB_CNT_MBSS15_CTRL_FRAME_CNT = 150,
	UNI_CMD_MIB_CNT_RX_DUP_DROP_BSSID0 = 151,
	UNI_CMD_MIB_CNT_RX_DUP_DROP_BSSID1 = 152,
	UNI_CMD_MIB_CNT_RX_DUP_DROP_BSSID2 = 153,
	UNI_CMD_MIB_CNT_RX_DUP_DROP_BSSID3 = 154,
	UNI_CMD_MIB_CNT_AMPDU = 155,
	UNI_CMD_MIB_CNT_MPDU_RETRY_DROP = 156,
	UNI_CMD_MIB_CNT_RTS_DROP = 157,
	UNI_CMD_MIB_CNT_BCN_RX = 158,
	UNI_CMD_MIB_CNT_MAX_NUM
};

/* ======================= UNI_CMD_ID_MIB End =======================*/

/* ============== UNI_CMD_ID_STATISTICS Begin ============== */

/**
 * This structure is used for UNI_CMD_ID_STATISTICS command (0x23) to fetch phy state info
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] au1Reserved        Reserved
 * @param[in] au1TlvBuffer        TLVs
 *
 */
struct _UNI_CMD_ID_STATISTICS_T {
	/*fixed field*/
	UINT8 au1Padding[4];

	/* tlv */
	UINT8 au1TlvBuffer[0];
};

/* Get Statistics Tag */
enum _UNI_CMD_ID_STATISTICS_TAG_T {
	UNI_CMD_ID_STATISTICS_BASIC = 0,
	UNI_CMD_ID_STATISTICS_LINK_QUALITY = 1,
	UNI_CMD_ID_STATISTICS_STA = 2,
	UNI_CMD_ID_STATISTICS_BUG_REPORT = 3,
	UNI_CMD_ID_STATISTICS_MAX_NUM
};

struct _UNI_CMD_ID_STATISTICS_TLV_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  au1Buffer[0];
};

/* ============== UNI_CMD_ID_STATISTICS End ============== */

/* ================= UNI_CMD_ID_SNIFFER_MODE Begin ================= */
/* Sniffer mode command TLV List */
typedef enum _UNI_CMD_SNIFFER_MODE_TAG_T {
    UNI_CMD_SNIFFER_MODE_ENABLE = 0,
    UNI_CMD_SNIFFER_MODE_CONFIG = 1,    /* rebb don't need*/
    UNI_CMD_SNIFFER_MODE_MAX_NUM
} UNI_CMD_SNIFFER_MODE_TAG_T;

/* Sniffer mode command (0x24) */
typedef struct GNU_PACKED _UNI_CMD_SNIFFER_MODE_T {
    /* fixed field */
    UINT8 ucBandIdx;       /* the band index */
    UINT8 ucReserved[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
							*   TAG                        | ID  | structure
							*   ---------------------------|-----|--------------
							*   UNI_CMD_SNIFFER_MODE_ENABLE | 0x00 | UNI_CMD_SNIFFER_MODE_ENABLE_T
							*   UNI_CMD_SNIFFER_MODE_CONFIG | 0x01 | UNI_CMD_SNIFFER_MODE_CONFIG_T
							*/
} UNI_CMD_SNIFFER_MODE_T, *P_UNI_CMD_SNIFFER_MODE_T;


/* Set sniffer mode parameters (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_SNIFFER_MODE_ENABLE_T {
    UINT16 u2Tag;      /* should be 0x00 */
    UINT16 u2Length;   /* should be sizeof(UNI_CMD_SNIFFER_MODE_ENABLE_T) */
    UINT8 ucSNEnable;  /* 0: disable, 1: enable */
    UINT8 aucPadding[3];
} UNI_CMD_SNIFFER_MODE_ENABLE_T, *P_UNI_CMD_SNIFFER_MODE_ENABLE_T;

/* Set sniffer mode parameters (Tag1) */
typedef struct GNU_PACKED _UNI_CMD_SNIFFER_MODE_CONFIG_T {
    UINT16 u2Tag;      /* should be 0x01 */
    UINT16 u2Length;   /* should be sizeof(UNI_CMD_SNIFFER_MODE_CONFIG_T) */
    UINT16 u2Aid;      /* the association ID */
    UINT8 ucBand;      /* the starting freq of the band (the unit is kHz) */
    UINT8 ucChannelWidth;  /* the channel bandwidth */
    UINT8 ucPriChannel;    /* primary channel (central channel on BW20) */
    UINT8 ucSco;           /* the channel offset used in BW40 */
    UINT8 ucChannelS1;     /* central channel 1 (used for BW80/BW160: SX center freq, BW80+80: SX lower center freq) */
    UINT8 ucChannelS2;     /* central channel 2 (used for BW80+80: SX higher center freq) */
    BOOLEAN fgDropFcsErrorFrame; /* TRUE: drop FCS error frame, FALSE: receive FCS error frame */
    UINT8 aucPadding[3];
} UNI_CMD_SNIFFER_MODE_CONFIG_T, *P_UNI_CMD_SNIFFER_MODE_CONFIG_T;

/* ================== UNI_CMD_ID_SNIFFER_MODE End ================== */

#ifdef ZERO_PKT_LOSS_SUPPORT
struct GNU_PACKED _UNI_CMD_SCSA_T
{
	/*fixed field*/
	UINT_8 aucPadding[4];
	/* tlv */
	UINT_8 aucTlvBuffer[0]; /**< the TLVs included in this field:
		*
		*  TAG                          | ID   | structure
		*  -------------                | -----| -------------
		*  UNI_CMD_ID_RDD_ON_OFF_CTRL   | 0x0  | UNI_CMD_RDD_ON_OFF_CTRL_PARM_T
		*  UNI_CMD_RDD_IPI_HIST_SET     | 0x1  | UNI_CMD_RDD_IPI_HIST_T
		*/
};

enum _UNI_CMD_SCSA_TAG_T {
	UNI_CMD_SET_ZERO_PKT_LOSS_VARIABLE = 0x00,
	UNI_CMD_SET_MAC_TX_ENABLE = 0x01,
	UNI_CMD_SET_WTBL_SKIP_TX_ENABLE = 0x02,
	UNI_CMD_SCSA_MAX_NUM
};
#endif

/* ================= UNI_CMD_ID_SCS Begin ================= */
typedef struct GNU_PACKED _UNI_CMD_SCS_T {
    /* fixed field */
    UINT8 ubandid;
    UINT8 ucReserved[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_SCS_T, *P_UNI_CMD_SCS_T;

/* SCS Tag value */
typedef enum _UNI_CMD_SCS_CTRL_TAG_T {
    UNI_CMD_SCS_EVENT_SEND_DATA = 0,
    UNI_CMD_SCS_EVENT_GET_GLO_ADDR,
    UNI_CMD_SCS_EVENT_SET_PD_THR_RANGE,
    UNI_CMD_SCS_EVENT_SCS_ENABLE,
    UNI_CMD_SCS_MAX_EVENT
} UNI_CMD_SCS_CTRL_TAG_T;

/* UNI_CMD_SCS_EVENT_SEND_DATA (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2 {
    /* DW0 */
    UINT16 u2Tag;
    UINT16 u2Length;
    /* DW1 - DW2 */
    UINT16 u2ActiveSTA;
    UINT16 u2eTput;
    UINT8 fgRxOnly;
    UINT8 fgPDreset;
    INT8  i1SCSMinRSSI;
    UINT8 ucReserved;
} UNI_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2, *P_UNI_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2;

typedef INT32 (*PFN_SCS_SEND_DATA_HANDLE)(struct _RTMP_ADAPTER *pAd,
											P_UNI_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2 pParam,
											VOID *pHandle);

/* UNI_CMD_SCS_EVENT_GET_GLO_ADDR (Tag1) */
typedef struct GNU_PACKED _UNI_CMD_SCS_GET_GLO_ADDR_CTRL {
    /* DW0 */
    UINT16 u2Tag;
    UINT16 u2Length;
} UNI_CMD_SCS_GET_GLO_ADDR_CTRL, *P_UNI_CMD_SCS_GET_GLO_ADDR_CTRL;

typedef INT32 (*PFN_SCS_GET_GLO_ADDR_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
											P_UNI_CMD_SCS_GET_GLO_ADDR_CTRL pParam,
											VOID *pHandle);

/* UNI_CMD_SCS_EVENT_SET_PD_THR_RANGE (Tag2) */
typedef struct GNU_PACKED _UNI_CMD_SET_SCS_PD_THR_RANGE {
    /* DW0 */
    UINT16 u2Tag;
    UINT16 u2Length;
    /* DW1 - DW2 */
    UINT16 u2CckPdThrMax;
    UINT16 u2OfdmPdThrMax;
    UINT16 u2CckPdThrMin;
    UINT16 u2OfdmPdThrMin;
} UNI_CMD_SET_SCS_PD_THR_RANGE, *P_UNI_CMD_SET_SCS_PD_THR_RANGE;

typedef INT32 (*PFN_SCS_PD_THR_RANGE_HANDLE)(struct _RTMP_ADAPTER *pAd,
											P_UNI_CMD_SET_SCS_PD_THR_RANGE pParam,
											VOID *pHandle);

/* UNI_CMD_SCS_EVENT_SET_MANUAL_PD_TH (Tag3) */
typedef struct GNU_PACKED _UNI_CMD_SMART_CARRIER_ENABLE {
    /* DW0 */
    UINT16 u2Tag;
    UINT16 u2Length;
    /* DW1 */
    UINT8 u1SCSEnable;
    UINT8 ucRreserved[3];
} UNI_CMD_SMART_CARRIER_ENABLE, *P_UNI_CMD_SMART_CARRIER_ENABLE;

typedef INT32 (*PFN_SCS_SMART_CARRIER_ENABLE_HANDLE)(struct _RTMP_ADAPTER *pAd,
											P_UNI_CMD_SMART_CARRIER_ENABLE pParam,
											VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_SCS_PARAM_T {
	UINT8 ucDbdcIdx;
	BOOLEAN bQuery;
	BOOLEAN SCSTagValid[UNI_CMD_SCS_MAX_EVENT];

	UNI_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2 SCSSendData;
	UNI_CMD_SCS_GET_GLO_ADDR_CTRL SCSGetGloAddrCtrl;
	UNI_CMD_SET_SCS_PD_THR_RANGE SCSPDThrRange;
	UNI_CMD_SMART_CARRIER_ENABLE SCSEnable;
} UNI_CMD_SCS_PARAM_T, *P_UNI_CMD_SCS_PARAM_T;

/* ================= UNI_CMD_ID_SCS End ================= */

/* =================== UNI_CMD_ID_DVT Begin =================== */
typedef struct GNU_PACKED _UNI_CMD_DVT_CONFIG_T {
	/*fixed field*/
	UINT8 ucTestType;
	UINT8 aucPadding[3];

	/* tlv */
	UINT8 aucTlvBuffer[0]; /**< the TLVs included in this field:
							*
							*   TAG                              | ID  | structure
							*   -------------                    | ----| -------------
							*   UNI_CMD_MDVT_SET_PARA            | 0x0 | UNI_CMD_MDVT_SET_PARA_T
							*/
} UNI_CMD_DVT_CONFIG_T, *P_UNI_CMD_DVT_CONFIG_T;

/* dvt config Tag */
typedef enum _UNI_CMD_DVT_TYPE_T {
    UNI_CMD_MODULE_DVT = 0,
    UNI_CMD_DVT_MAX_NUM
} UNI_CMD_DVT_TYPE_T;

/* module dvt config Tag */
typedef enum _UNI_CMD_DVT_TAG_T {
    UNI_CMD_MDVT_SET_PARA = 0,
    UNI_CMD_DVT_TAG_MAX_NUM
} UNI_CMD_DVT_TAG_T;

/* MDVT Parameter Setting (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_MDVT_SET_PARA_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT16 u2ModuleId;
    UINT16 u2CaseId;
    UINT8 u1CapId;
    UINT8 u1Reseved[3];
} UNI_CMD_MDVT_SET_PARA_T, *P_UNI_CMD_MDVT_SET_PARA_T;

/* =================== UNI_CMD_ID_DVT End =================== */

/* =================== UNI_CMD_ID_GPIO Begin =================== */
typedef struct GNU_PACKED _UNI_CMD_GPIO_CONFIG_T {
	/*fixed field*/
	UINT8 ucGpioIdx;
	UINT8 aucPadding[3];

	/* tlv */
	UINT8 aucTlvBuffer[0]; /**< the TLVs included in this field:
							*
							*   TAG                     | ID  | structure
							*   -------------           | ----| -------------
							*   UNI_CMD_GPIO_ENABLE     | 0x0 | UNI_CMD_GPIO_ENABLE_T
							*   UNI_CMD_GPIO_SET_VALUE  | 0x1 | UNI_CMD_GPIO_SET_VALUE_T
							*/
} UNI_CMD_GPIO_CONFIG_T, *P_UNI_CMD_GPIO_CONFIG_T;

/* Gpio config Tag */
typedef enum _UNI_CMD_GPIO_TAG_T {
    UNI_CMD_GPIO_ENABLE = 0,
    UNI_CMD_GPIO_SET_VALUE = 1,
    UNI_CMD_GPIO_TAG_MAX_NUM
} UNI_CMD_GPIO_TAG_T;

/* MDVT Parameter Setting (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_GPIO_ENABLE_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    BOOLEAN fgEnable;
    UINT8 aucReserved[3];
} UNI_CMD_GPIO_ENABLE_T, *P_UNI_CMD_GPIO_ENABLE_T;

typedef INT32 (*PFN_GPIO_ENABLE_HANDLE)(struct _RTMP_ADAPTER *pAd,
											P_UNI_CMD_GPIO_ENABLE_T pParam,
											VOID *pHandle);

/* MDVT Parameter Setting (Tag1) */
typedef struct GNU_PACKED _UNI_CMD_GPIO_SET_VALUE_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucGpioValue;
    UINT8 aucReserved[3];
} UNI_CMD_GPIO_SET_VALUE_T, *P_UNI_CMD_GPIO_SET_VALUE_T;

typedef INT32 (*PFN_GPIO_SET_VALUE_HANDLE)(struct _RTMP_ADAPTER *pAd,
											P_UNI_CMD_GPIO_SET_VALUE_T pParam,
											VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_GPIO_CFG_PARAM_T {
	UINT8 ucGpioIdx;
	BOOLEAN GpioCfgValid[UNI_CMD_GPIO_TAG_MAX_NUM];

	UNI_CMD_GPIO_ENABLE_T GpioEnable;
	UNI_CMD_GPIO_SET_VALUE_T GpioSetValue;
} UNI_CMD_GPIO_CFG_PARAM_T, *P_UNI_CMD_GPIO_CFG_PARAM_T;

/* =================== UNI_CMD_ID_GPIO End =================== */

/* ============== UNI_CMD_ID_TXPOWER Begin ============== */
struct GNU_PACKED UNI_CMD_TXPOWER_CONFIG {
	/*fixed field*/
	UINT8 aucPadding[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

struct GNU_PACKED UNI_CMD_TXPOWER_TLV {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  aucBuffer[0];
};

enum UNI_CMD_TXPOWER_TAG {
	UNI_CMD_TXPOWER_SKU_POWER_LIMIT_CTRL = 0x00,
	UNI_CMD_TXPOWER_PERCENTAGE_CTRL = 0x01,
	UNI_CMD_TXPOWER_PERCENTAGE_DROP_CTRL = 0x02,
	UNI_CMD_TXPOWER_BACKOFF_POWER_LIMIT_CTRL = 0x03,
	UNI_CMD_TXPOWER_POWER_LIMIT_TABLE_CTRL = 0x04,
	UNI_CMD_TXPOWER_RF_TXANT_CTRL = 0x05,
	UNI_CMD_TXPOWER_ATEMODE_CTRL = 0x06,
	UNI_CMD_TXPOWER_TX_POWER_SHOW_INFO = 0x7,
	UNI_CMD_TXPOWER_TPC_FEATURE_CTRL = 0x8,
	UNI_CMD_TXPOWER_MU_TX_POWER_CTRL = 0x09,
	UNI_CMD_TXPOWER_BF_NDPA_TXD_CTRL = 0x0a,
	UNI_CMD_TXPOWER_TSSI_WORKAROUND = 0x0b,
	UNI_CMD_TXPOWER_THERMAL_COMPENSATION_CTRL = 0x0c,
	UNI_CMD_TXPOWER_TX_RATE_POWER_CTRL = 0x0d,
	UNI_CMD_TXPOWER_UP_TABLE_CTRL = 0x0e,
	UNI_CMD_TXPOWER_SET_TARGET_POWER = 0x0f,
	UNI_CMD_TXPOWER_GET_TARGET_POWER = 0x10,
	UNI_CMD_TX_POWER_SET_PER_PKT_POWER = 0x11,
	UNI_CMD_TX_POWER_SET_PER_PKT_MIN_POWER = 0x12,
	UNI_CMD_TXPOWER_CMD_MAX_NUM
};
/* ============== UNI_CMD_ID_TXPOWER End ============== */



/* ============== UNI_CMD_ID_HWCFG_CTRL Begin ============== */
struct GNU_PACKED UNI_CMD_HWCFG_T {
	/* fixed field */
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

enum  UNI_CMD_HWCFG_CTRL_TAG {
	UNI_CMD_HWCFG_READ = 0x0,
	UNI_CMD_HWCFG_MAX_NUM
};

struct GNU_PACKED UNI_CMD_HWCFG_READ_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2Offset;   /* Read Offset */
	UINT16 u2Count;    /* Read Total Counts */
};
/* ============== UNI_CMD_ID_HWCFG_CTRL End ============== */

/* ============== UNI_CMD_ID_SWACI_CTRL Begin ============== */
#ifdef SWACI_MECHANISM
struct GNU_PACKED UNI_CMD_SWACI_CTRL_T {
	/* fixed field */
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

enum UNI_CMD_SWACI_CTRL_TAG_T {
	UNI_CMD_SWACI_GAIN_ADJUST = 0x0,
	UNI_CMD_SWACI_CONDITION_PARAM,
	UNI_CMD_SWACI_LNA_TABLE_PARAM,
	UNI_CMD_SWACI_TIMER_CONTROL,
	UNI_CMD_SWACI_SET_RCPI,
	UNI_CMD_SWACI_TESTMODE,
	UNI_CMD_SWACI_ENABLE,
	UNI_CMD_SWACI_SET_DENSE_PARAMS,
	UNI_CMD_SWACI_SET_THRESH_PARAMS,
	UNI_CMD_SWACI_TAG_MAX_NUM
};

struct GNU_PACKED UNI_CMD_SWACI_SET_TESTMODE {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT32 u4TestMode;
};

struct GNU_PACKED UNI_CMD_SWACI_DENSE_LNA_PARAM {
	UINT16 u2Tag;
	UINT16 u2Length;
	INT8  i1MinSubRssi;
	INT8  i1MaxRcpi;
	UINT8 u1ReadCount;
	UINT8 u1MinDenseCount;
	UINT8 u1MaxDenseCount;
	UINT8 u1DenseCRValue;
	UINT8 au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_SWCR_PARAMS {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1SWCR0;
	UINT8  u1SWCR1;
	UINT8  u1SWCR2;
	INT8   i1SWCR4;
};

struct GNU_PACKED UNI_CMD_SWACI_TIMER_ENABLE {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 u1DbdcIdx;
	UINT8 fgEnable;
	UINT8 au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_SWACI_ENABLE_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 fgEnable;
	UINT8 au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_SWACI_SET_THRESH {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT32 u4Threshold;
};

struct GNU_PACKED UNI_CMD_SWACI_SET_RCPI_TEST {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 au2Rcpi[2][4];
};

struct GNU_PACKED UNI_CMD_SWACI_GAIN_ADJUST_PARAMS {
	UINT16 u2Tag;
	UINT16 u2Length;
	INT8   i1AdcSetPointAci;
	INT8   i1AdcSetPointAAciBw20;
	INT8   i1AdcSetPointAAciBw40;
	INT8   i1AdcSetPointAAciBw80;
	INT8   i1AdcSetPointWanted;
	INT8   i1MaxRfGain;
	INT8   i1RfdgcSetPointAci;
	INT8   i1RfdgcSetPointWanted;
	INT8   i1MaxTotalGain;
	INT8   i1FarGainBoundLong;
	INT8   i1FarGainBoundShort;
	INT8   i1AdcSetPoint;
};

struct GNU_PACKED UNI_CMD_SWACI_TABLE_PARAMS {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1PhyBw;
	INT8   i1LnaParam[5];
	UINT8 au1Reserved[2];
};
#endif
/* ============== UNI_CMD_ID_SWACI_CTRL End ============== */

/* ============== UNI_CMD_ID_SPECTRUM Begin ============== */
struct GNU_PACKED UNI_CMD_SPECTRUM_CONFIG {
	/* fixed field */
	UINT8 au1Reserved[4];

	/* tlv */
	UINT8 au1TlvBuffer[0];
};

enum UNI_CMD_SPECTRUM_TAG {
	UNI_CMD_SET_SPECTRUM_PARAMTER = 0,
	UNI_CMD_GET_SPECTRUM_STATUS = 1,
	UNI_CMD_GET_SPECTRUM_DATA = 2,
	UNI_CMD_SET_PHY_ICS_START = 3,
	UNI_CMD_SET_PHY_ICS_EVENT_ENABLE = 4,
	UNI_CMD_SPECTRUM_TAG_MAX_NUM
};

struct GNU_PACKED UNI_CMD_SET_SPECTRUM_PARAMTER_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT32 fgTrigger;
	UINT32 fgRingCapEn;
	UINT32 u4TriggerEvent;
	UINT32 u4CaptureNode;
	UINT32 u4CaptureLen;    /* Unit : IQ Sample */
	UINT32 u4CapStopCycle;  /* Unit : IQ Sample */
	UINT32 u4MACTriggerEvent;
	UINT32 u4SourceAddressLSB;
	UINT32 u4SourceAddressMSB;
	UINT32 u4BandIdx;
	UINT32 u4BW;
	UINT32 u4EnBitWidth;/* 0:32bit, 1:96bit, 2:128bit */
	UINT32 u4Architech;/* 0:on-chip, 1:on-the-fly */
	UINT32 u4PhyIdx;
	UINT32 u4EmiStartAddress;
	UINT32 u4EmiEndAddress;
	UINT32 u4EmiMsbAddress;
	UINT32 u4CapSource;
	UINT32 u4Reserved[2];
};

struct GNU_PACKED UNI_CMD_GET_SPECTRUM_STATUS_T {
	UINT16 u2Tag;
	UINT16 u2Length;
};

struct GNU_PACKED UNI_CMD_GET_SPECTRUM_DATA_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT32 u4Address;
	UINT32 u4AddrOffset;
	UINT32 u4Bank;
	UINT32 u4BankSize;
	UINT32 u4WFNum;
	UINT32 u4IQType;
	UINT32 u4Reserved[6];
};

struct GNU_PACKED UNI_CMD_SET_PHY_ICS_START_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT32 fgTrigger;
	UINT32 fgRingCapEn;
	UINT32 u4BandIdx;
	UINT32 u4BW;
	UINT32 u4EnBitWidth;
	UINT32 u4Architech;
	UINT32 u4PhyIdx;
	UINT32 u4EmiStartAddress;
	UINT32 u4EmiEndAddress;
	UINT32 u4CapSource;
	UINT32 u4PhyIcsType;
	UINT32 u4PhyIcsEventGroup;
	UINT32 u4PhyIcsEventID[2];
	UINT32 u4PhyIcsTimer;
	UINT32 u4Reserved[2];
};

/* ============== UNI_CMD_ID_SPECTRUM End ============== */

/* ============== UNI_CMD_ID_RA Begin ============== */
/* Config RA command (0x2F) */
struct UNI_CMD_RA {
	/* fixed field */
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0]; /**< the TLVs included in this field:
	*
	*   TAG                                        | ID   |
	*   -------------------------  | --   |
	*   UNI_CMD_RA_GET_RU_RA_INFO    | 0x0 |
	*   UNI_CMD_RA_GET_HERA_RELATED_INFO    | 0x1 |
	*   UNI_CMD_RA_GET_MU_RA_INFO           | 0x2 |
	*   UNI_CMD_RA_HERA_STBC_PRIORITY       | 0x3 |
	*   UNI_CMD_RA_OPTION_CTRL              | 0x4 |
	*   UNI_CMD_RA_SET_VHT_1024_QAM     | 0x5 |
	*   UNI_CMD_RA_CFG_PTEC_PER_PPDU        | 0x6 |
	*   UNI_CMD_RA_CFG_MU_INIT_RATE_INTV    | 0x7 |
	*   UNI_CMD_RA__CFG_MU_DIS_SWITCH_SU    | 0x8 |
	*   UNI_CMD_RA_SET_VHT_RATE_FOR_2G      | 0x9 |
	*   UNI_CMD_RA_FIX_RATE_WO_STA_UPDATE   | 0xA |
	*   UNI_CMD_RA_SUPPORT_MCS_CAP_CTRL | 0xB |
	*   UNI_CMD_RA_DBG_CTRL             | 0xC |
	*   UNI_CMD_RA_GET_TX_RATE              | 0xD |
	*   UNI_CMD_RA_SET_MAX_PHY_RATE     | 0xE |
	*   UNI_CMD_RA_SET_FIXED_RATE          | 0xF |
	*   UNI_CMD_RA_SET_FIXED_RATE_UL_TRIG    | 0x10|
	*   UNI_CMD_RA_SET_AUTO_RATE     | 0x11|
	*/
};
/** @} */

/* RA Tag */
enum GNU_PACKED UNI_CMD_ID_RA_TAG {
	UNI_CMD_RA_GET_RU_RA_INFO = 0x0,
	UNI_CMD_RA_GET_HERA_RELATED_INFO = 0x01,
	UNI_CMD_RA_GET_MU_RA_INFO = 0x02,
	UNI_CMD_RA_HERA_STBC_PRIORITY = 0x03,
	UNI_CMD_RA_OPTION_CTRL = 0x04,
	UNI_CMD_RA_SET_VHT_1024_QAM = 0x05,
	UNI_CMD_RA_CFG_PTEC_PER_PPDU = 0x06,
	UNI_CMD_RA_CFG_MU_INIT_RATE_INTV = 0x07,
	UNI_CMD_RA_CFG_MU_DIS_SWITCH_SU = 0x08,
	UNI_CMD_RA_SET_VHT_RATE_FOR_2G = 0x09,
	UNI_CMD_RA_FIX_RATE_WO_STA_UPDATE = 0x0A,
	UNI_CMD_RA_SUPPORT_MCS_CAP_CTRL = 0x0B,
	UNI_CMD_RA_DBG_CTRL = 0x0C,
	UNI_CMD_RA_GET_TX_RATE = 0x0D,
	UNI_CMD_RA_SET_MAX_PHY_RATE = 0x0E,
	UNI_CMD_RA_SET_FIXED_RATE = 0x0F,
	UNI_CMD_RA_SET_FIXED_RATE_UL_TRIG = 0x10,
	UNI_CMD_RA_SET_AUTO_RATE = 0x11,
	UNI_CMD_RA_MAX_NUM
};

enum GNU_PACKED UNI_CMD_RA_FIXED_RATE_VER {
	UNI_CMD_RA_FIXED_RATE_VER1 = 0x0,
	UNI_CMD_RA_FIXED_RATE_VER_MAX_NUM
};

struct GNU_PACKED UNI_CMD_RA_GET_RU_RA_INFO_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT16 u2WlanIdx;
	UINT16 u2RuIdx;
	UINT16 u2Direction;
	UINT16 u2DumpGroup;
};

struct GNU_PACKED UNI_CMD_RA_GET_MU_RA_INFO_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT16 u2MuGroupIdx;
	UINT16 u2UserIdx;
	UINT16 u2Direction;
	UINT16 u2DumpGroup;
};

struct GNU_PACKED UNI_CMD_RA_GET_HERA_RELATED_INFO_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT16 u2Para1;
	UINT16 u2Para2;
	UINT16 u2Para3;
	UINT16 u2Para4;
};

struct GNU_PACKED UNI_CMD_RA_HERA_STBC_PRIORITY_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT8 u1BandIdx;
	UINT8 u1Operation;
	UINT8 u1StbcPriority;
	UINT8 u1Reserved[5];
};

struct GNU_PACKED UNI_CMD_RA_RA_OPTION_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT8 u1OptionType;
	UINT8 u1Value;
	UINT8 u1Reserved[2];
};

struct  GNU_PACKED UNI_CMD_RA_SET_VHT_1024_QAM_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT8 fgVht1024QamSupport;
	UINT8  au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_RA_CFG_PTEC_PER_PPDU_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT8 fgPtecPerPpduDis;
	UINT8  u1BandIdx;
	UINT8  au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_RA_CFG_MU_INIT_RATE_INTV_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT8 u1IntvInUnit50Ms;
	UINT8 au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_RA_CFG_MU_DIS_SWITCH_SU_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT8 fgDisSwitchSU;
	UINT8  au1Reserved[3];
};

struct  GNU_PACKED UNI_CMD_RA_SET_VHT_RATE_FOR_2G_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT8   fgUseVhtRateFor2G;
	UINT8  aucReserve[3];
};

struct GNU_PACKED UNI_CMD_RA_DBG_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT8 u1RateCtrlFormatId;
	UINT8 u1ParamNum;
	UINT8 u1Reserved[2];
	UINT32 u4Param[20];
};

struct  GNU_PACKED UNI_CMD_RA_GET_TX_RATE_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT8 u1EventType; /*Information type send to driver*/
	UINT8 u1Reserved[3];
};

struct GNU_PACKED UNI_CMD_RA_SUPPORT_MCS_CAP_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT8 u1RateCtrlFormatId;
	UINT8 u1TxMode;
	UINT8 u1TxNss;
	UINT8 u1TxBw;
	UINT16 u2McsCap;
	UINT8 u1Set;
	UINT8 u1Reserved;
};

struct GNU_PACKED UNI_CMD_RA_FIX_RATE_WO_STA_UPDATE_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT8  MODE;
	UINT8  Flags;
	UINT8  STBC;
	UINT8  ShortGI;
	UINT8  BW;
	UINT8  ldpc;
	UINT8  MCS;
	UINT8  VhtNss;
	UINT8  HeLtf;
	UINT8  HeErDCM;
	UINT8  HeEr106t;
	UINT8  u1WlanIdxL;
	UINT8  u1SpeEn;
	UINT8  u1ShortPreamble;
	UINT8  u1WlanIdxHnVer;
	UINT8  u1Reserved;
};

struct GNU_PACKED UNI_CMD_RA_SET_MAX_PHY_RATE_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
	UINT16 u2MaxPhyRate;
	UINT8  aucReserve[2];
};

/*RA fixed rate Parameters (Tag 0x0F) */
struct GNU_PACKED _UNI_CMD_RA_SET_FIXED_RATE_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */

	UINT16 u2Version;
	UINT8   aucBuffer[0];
};

struct GNU_PACKED _UNI_CMD_RA_SET_FIXED_RATE_V1 {
	UINT16  u2WlanIdx;
	UINT8   u1PhyMode;
	UINT8   u1Stbc;
	UINT16  u2ShortGi;
	UINT8   u1Bw;
	UINT8   u1Ecc;
	UINT8   u1Mcs;
	UINT8   u1Nss;
	UINT16  u2HeLtf;
	UINT8   u1Spe;
	UINT8   u1ShortPreamble;
	UINT16  u2Reserve;
};

struct GNU_PACKED _UNI_CMD_RA_SET_FIXED_RATE_UL_TRIG_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */

	UINT16 u2Version;
	UINT8  aucBuffer[0];
};

struct GNU_PACKED _UNI_CMD_RA_SET_FIXED_RATE_UL_TRIG_V1 {
	UINT16 u2WlanIdx;
	UINT8  u1Stbc;
	UINT8  u1ShortGi;
	UINT8  u1Ecc;
	UINT8  u1Mcs;
	UINT8  u1Nss;
	UINT8  u1HeLtf;
	UINT8  u1Spe;
	UINT8  u1ShortPreamble;
};

struct GNU_PACKED _UNI_CMD_RA_SET_AUTO_RATE_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */

	UINT16 u2WlanIdx;
	UINT8  u1AutoRateEn;
	UINT8  u1Mode;
	UINT8  aucReserve[2];
};

typedef INT32 (*PFN_RA_HANDLE)(struct _RTMP_ADAPTER *pAd, IN RTMP_STRING * arg, VOID *pHandle);

/* ============== UNI_CMD_ID_RA End ============== */

#ifdef WIFI_MD_COEX_SUPPORT
/* ============== UNI_CMD_ID_IDC Begin ============== */
struct _UNI_CMD_IDC_COMMON_TLV_T {
	UINT8 ucReserved[4];
	UINT8 aucTlvBuffer[0];
};

struct _UNI_CMD_IDC_SET_STATE_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	BOOLEAN isIDCen;
};

struct _UNI_CMD_IDC_GET_CHAN_T {
	UINT16 u2Tag;
	UINT16 u2Length;
};

struct _UNI_CMD_IDC_GET_INFO_T {
	UINT16 u2Tag;
	UINT16 u2Length;
};

struct _UNI_CMD_IDC_SET_PWR_BACKOFF_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2MaxTxPwrLimit;
	UINT8 ucBandIdx;
	UINT8 ucReserved;
};

INT32 UniCmdQuerySafeChannel(struct _RTMP_ADAPTER *pAd);
INT32 UniCmdQueryIdcInfo(struct _RTMP_ADAPTER *pAd);
INT32 UniCmdUpdateIdcState(struct _RTMP_ADAPTER *pAd);
INT32 UniCmdSetIdcPwrBackoff(struct _RTMP_ADAPTER *pAd, UINT16 maxTxPwrLimit);

/* ============== UNI_CMD_ID_IDC End ============== */
#endif

/* =================== UNI_CMD_ID_MURU Begin =================== */

typedef INT32 (*PFN_MURU_HANDLE)(struct _RTMP_ADAPTER *pAd, IN RTMP_STRING *arg, VOID *pHandle);

enum UNI_CMD_MURU_TAG_T {
	UNI_CMD_MURU_BSRP_CTRL = 0x01,
	UNI_CMD_MURU_SET_ARB_OP_MODE = 0xe,
	UNI_CMD_MURU_SET_ALGO_DBG_CTRL = 0xf,
	UNI_CMD_MURU_SUTX_CTRL = 0x10,
	UNI_CMD_MURU_FIXED_RATE_CTRL = 0x11,
	UNI_CMD_MURU_FIXED_GROUP_RATE_CTRL = 0x12,
	UNI_CMD_MURU_SET_IGNORE_NAV = 0x16,
	UNI_CMD_MURU_CERT_SEND_FRAME_CTRL = 0x17,
	UNI_CMD_MURU_DBG_INFO = 0x18,
	UNI_CMD_MURU_CTS_RETRY_CTRL = 0x19,
	UNI_CMD_MURU_MDVT_INFO = 0x1A,

	UNI_CMD_MURU_GET_GROUP_TBL_ENTRY = 0x32,
	UNI_CMD_MURU_SET_FORCE_MU = 0x33,
	UNI_CMD_MURU_GET_UL_RU_STATUS = 0x34,

	/* Configure commands */
	UNI_CMD_MURU_CFG_DLUL_LIMIT = 0x50,
	UNI_CMD_MURU_SET_DLUL_EN = 0x51,

	/* HQA AP commands offset 100 */
	UNI_CMD_MURU_MANUAL_CONFIG = 0x64,
	UNI_CMD_MURU_SET_ULTX_TRIGGER = 0x65,
	UNI_CMD_MURU_SET_ULTX_CNT_RESET = 0x66,
	UNI_CMD_MURU_GET_ULTX_CNT = 0x67,
	UNI_CMD_MURU_SET_AGGPOLICY = 0x68,
	UNI_CMD_MURU_SET_MU_TX_PKT_CNT = 0x69,
	UNI_CMD_MURU_SET_MU_TX_PKT_EN = 0x6A,
	UNI_CMD_TEST_MODE_TBRX_CONFIG = 0x6B,

	UNI_CMD_MURU_SET_BSRP_PER_TXOP = 0x6C,
	UNI_CMD_MURU_SET_BF_IN_AC01 = 0x6D,

	/* PQS feature */
	UNI_CMD_PQS_SET_MANUAL_SCS_CMD = 0x70,
	//UNI_CMD_PQS_SHOW_SCS_STA_INFO = 0x71,

	UNI_CMD_MURU_SET_TXC_TX_STATS_EN = 0x96,
	UNI_CMD_MURU_GET_TXC_TX_STATS = 0x97,

	UNI_CMD_MURU_SET_MUDL_ACK_POLICY = 0xC8,
	UNI_CMD_MURU_SET_TRIG_TYPE = 0xC9,
	UNI_CMD_MURU_SET_20M_DYN_ALGO = 0xCA,
	UNI_CMD_MURU_PROT_FRAME_THR = 0xCC,
	UNI_CMD_MURU_SET_CERT_MU_EDCA_OVERRIDE = 0xCD,
	UNI_CMD_MURU_GET_FW_BLACKLIST_CTRL = 0xCE,
	UNI_CMD_MURU_SET_DRV_BLACKLIST_CTRL = 0xCF,
	UNI_CMD_MURU_SET_TXOP_ONOFF = 0xD2,
	UNI_CMD_MURU_UL_ONOFF = 0xD3,
	UNI_CMD_MURU_SET_STATISTIC_CONFIG = 0xD4,
	UNI_CMD_MURU_SET_TRIG_VARIANT = 0xD5,
	UNI_CMD_MURU_SET_QOS_CFG = 0xFE,
	UNI_CMD_MURU_MAX
};

enum UNI_CMD_MURU_VER_T {
	/* Command Version */
	UNI_CMD_MURU_VER_LEG = 0,
	UNI_CMD_MURU_VER_HE,
	UNI_CMD_MURU_VER_EHT,
	UNI_CMD_MURU_VER_MAX
};

enum UNI_HW_CHIP_SUPP_T {
	/* Command Version */
	UNI_HW_CHIP_SUPP_LEG = 0,
	UNI_HW_CHIP_SUPP_HE,
	UNI_HW_CHIP_SUPP_EHT,
	UNI_HW_CHIP_SUPP_MAX
};

#define UNI_MAX_MCS_SUPPORT_HE      11
#define UNI_MAX_MCS_SUPPORT_EHT     13

struct GNU_PACKED UNI_CMD_MURU_BSRP_CTRL_T {
	/* DW_0 */
	UINT16     u2Tag;
	UINT16     u2Length;
	/* DW_1*/
	UINT16     u2BsrpInterval;
	UINT16     u2BsrpRuAlloc;
	/* DW_2*/
	UINT32     u4TriggerType; /*@us*/
	/* DW_3*/
	/*0: normal, 1: kick-and-stop, 2: stop, 3: CheckULQueueLen*/
	UINT8      u1TriggerFlow;
	/* TRUE: timer control by ext cmd */
	UINT8      fgExtCmdBsrp;
	UINT8      u1BandBitMap;  /* Bit 0: band0, Bit 1: band1, Bit 2: band2 */
	UINT8      u1Reserved[1];
};

#define UNI_MURU_MAX_NUM_TXCMD_TX_USER       (16)

struct GNU_PACKED UNI_WLAN_TF_BASIC_USER_INFO {
#ifdef CFG_BIG_ENDIAN
	UINT_32 u4Aid12         : 12;
	UINT_32 u4RuAllocBn     : 1;
	UINT_32 u4RuAlloc       : 7;
	UINT_32 u4CodingType    : 1;
	UINT_32 u4Mcs           : 4;
	UINT_32 u4Dcm           : 1;
	UINT_32 u4SsAlloc       : 6;
#else
	UINT_32 u4SsAlloc       : 6;
	UINT_32 u4Dcm           : 1;
	UINT_32 u4Mcs           : 4;
	UINT_32 u4CodingType    : 1;
	UINT_32 u4RuAlloc       : 7;
	UINT_32 u4RuAllocBn     : 1;
	UINT_32 u4Aid12         : 12;
#endif

#ifdef CFG_BIG_ENDIAN
	UINT_8  ucTargetRssi    : 7;
	UINT_8  ucPS160         : 1;
#else
	UINT_8  ucPS160         : 1;
	UINT_8  ucTargetRssi    : 7;
#endif

#ifdef CFG_BIG_ENDIAN
	UINT_8  ucMpduMuSpacing : 2;
	UINT_8  ucTidAggrLimit  : 3;
	UINT_8  ucRsv2          : 1;
	UINT_8  ucPreferredAc   : 2;
#else
	UINT_8  ucPreferredAc   : 2;
	UINT_8  ucRsv2          : 1;
	UINT_8  ucTidAggrLimit  : 3;
	UINT_8  ucMpduMuSpacing : 2;
#endif
};

union UNI_EHTTB_RX_BASIC_USER_INFO {
	struct UNI_WLAN_TF_BASIC_USER_INFO field;
	UINT_64 data;
};

struct GNU_PACKED UNI_WLAN_TF_SPECIAL_USER_INFO
{
#ifdef CFG_BIG_ENDIAN
	UINT_32 u4Aid12         : 12;
	UINT_32 u4PhyVersion    : 3;
	UINT_32 u4UlBwExt       : 2;
	UINT_32 u4SR1           : 4;
	UINT_32 u4SR2           : 4;
	UINT_32 u4USigDisregardAndValidateL : 7;
#else
	UINT_32 u4USigDisregardAndValidateL : 7;
	UINT_32 u4SR2           : 4;
	UINT_32 u4SR1           : 4;
	UINT_32 u4UlBwExt       : 2;
	UINT_32 u4PhyVersion    : 3;
	UINT_32 u4Aid12         : 12;
#endif

#ifdef CFG_BIG_ENDIAN
	UINT_8  u4USigDisregardAndValidateU : 5;
	UINT_8  ucReserved      : 3;
#else
	UINT_8  ucReserved      : 3;
	UINT_8  u4USigDisregardAndValidateU : 5;
#endif
};

union UNI_EHTTB_RX_SPECIAL_USER_INFO {
	struct UNI_WLAN_TF_SPECIAL_USER_INFO field;
	UINT_64 data;
};

struct GNU_PACKED UNI_WLAN_TRIG_CMM_INFO
{
#ifdef CFG_BIG_ENDIAN
	UINT_32 u4TriggerType   : 4;
	UINT_32 u4Length        : 12;
	UINT_32 u4Cascade       : 1;
	UINT_32 u4CsRequired    : 1;
	UINT_32 u4Bw            : 2;
	UINT_32 u4GiLtfType     : 2;
	UINT_32 u4MuMimoLtfMode : 1;
	UINT_32 u4HeLtfSymbols  : 3;
	UINT_32 u4Stbc          : 1;
	UINT_32 u4LdpcExtra     : 1;
	UINT_32 u4apTxPowerLow  : 4;
#else
	UINT_32 u4apTxPowerLow  : 4;
	UINT_32 u4LdpcExtra     : 1;
	UINT_32 u4Stbc          : 1;
	UINT_32 u4HeLtfSymbols  : 3;
	UINT_32 u4MuMimoLtfMode : 1;
	UINT_32 u4GiLtfType     : 2;
	UINT_32 u4Bw            : 2;
	UINT_32 u4CsRequired    : 1;
	UINT_32 u4Cascade       : 1;
	UINT_32 u4Length        : 12;
	UINT_32 u4TriggerType   : 4;
#endif

#ifdef CFG_BIG_ENDIAN
	UINT_32 u4apTxPowerHigh : 2;
	UINT_32 u4pktExt        : 3;
	UINT_32 u4SpatialReuse  : 16;
	UINT_32 u4Doppler       : 1;
	UINT_32 u4HeEhtP160     : 1;
	UINT_32 u4SpecialUserPresent : 1;
	UINT_32 u4RsvEht        : 7;
	UINT_32 u4Reserved      : 1;
#else
	UINT_32 u4Reserved      : 1;
	UINT_32 u4RsvEht        : 7;
	UINT_32 u4SpecialUserPresent : 1;
	UINT_32 u4HeEhtP160     : 1;
	UINT_32 u4Doppler       : 1;
	UINT_32 u4SpatialReuse  : 16;
	UINT_32 u4pktExt        : 3;
	UINT_32 u4apTxPowerHigh : 2;
#endif
};

union UNI_EHTTB_RX_CMM_INFO {
	struct UNI_WLAN_TRIG_CMM_INFO field;
	UINT_64 data;
};

struct GNU_PACKED UNI_CSD_CFG
{
	UINT_32 u4RxHeTbCfg1;
	UINT_32 u4RxHeTbCfg2;
};

union UNI_EHTTB_RX_CSD_CONFIG {
	struct UNI_CSD_CFG field;
	UINT_64 data;
};

struct GNU_PACKED UNI_CMD_TBRX_CFG_INTERFACER
{
	/* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;

	/* DW_1 */
	UINT_8 u1UserCnt;
	UINT_8 au1Reserved[3];

	UINT_64 u8CsdCfg;
	UINT_64 u8CommonInfo;
	UINT_64 u8SpecialUserInfo;
	UINT_64 au8BasicUserInfo[UNI_MURU_MAX_NUM_TXCMD_TX_USER];
};


struct GNU_PACKED UNI_CMD_MURU_SET_BSRP_PER_TXOP_T
{
	/* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;

	/* DW_1*/
	UINT_8 u1Enable;
	UINT_8 au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_TXC_TX_STATS_EN
{
	/* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;

	/* DW_1 */
	UINT_8 u1TxcTxEnable;
	UINT_8 au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_TXC_TX_STATS {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8   u1BandIdx;
	UINT8   au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_FW_BLACKLIST_CTRL {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT16   u2WlanIdx;
	UINT8    au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_MURU_DRV_BLACKLIST_CTRL {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT_16 u2WlanId;
	BOOLEAN fgDrvBlackListDlOfdmaDisable;
	BOOLEAN fgDrvBlackListUlOfdmaDisable;
};

struct GNU_PACKED UNI_MURU_QOS_SETTING {
	UINT_16		u2WlanIdx;
	UINT_8		u1AC;
	UINT_8		u1ForceAC;
	UINT_16		u2DelayBound;
	UINT_16		u2DelayReq;
	UINT_32		u4DataRate;
	UINT_16		u2BWReq;
	UINT_8		u1Dir;
	UINT_8		u1DelayWeight;
	UINT_16		u2DropThres;
	UINT_8		u1Idx;
	UINT_8		u1ScsId;
};

struct UNI_CMD_MURU_QOS_CFG {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT_32		u4OpFlag;
	struct UNI_MURU_QOS_SETTING	QoSSetting;
	UINT_8 u8QosCharacteristicsIE[44];
};

struct GNU_PACKED UNI_CMD_MURU_IGNORE_NAV {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8   u1Ignore;
	UINT8   au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_STAT_RECORD_CTRL {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;

	UINT_8 u1Mode;
	UINT_8 ucReserved;
	UINT_16 u2StartWcid;
	UINT_16 u2EndWcid;
	UINT_16 u2TimerInterval;
};

struct GNU_PACKED UNI_CMD_MURU_CERT_MU_EDCA_OVERRIDE {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8   u1OverRide;
	UINT8   au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_CERT_FRAME_CTRL {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;

	UINT32     u4PpduDur; /*@us*/
	UINT16     u2TargetWcid;
	UINT8      u1Interval; /*@ms*/
	UINT_8		u1Reserved[1];
};

struct GNU_PACKED UNI_MURU_DL_USER_INFO {
	UINT16     u2WlanIdx;
	UINT8      u1RuAllocBn;
	UINT8      u1RuAllocIdx;

	UINT8      u1Ldpc;
	UINT8      u1Nss;
	UINT8      u1Mcs;
	UINT8      u1MuGroupIdx;

	UINT8      u1VhtGid;
	UINT8      u1VhtUp;
	UINT8      u1HeStartStream;
	UINT8      u1HeMuMimoSpatial;

	UINT16     u2TxPwrAlpha;
	UINT8      u1AckPolicy;
	UINT8      u1RuAllocPs160;
};

struct GNU_PACKED UNI_MURU_UL_USER_INFO {
	UINT16     u2WlanIdx;
	UINT8      u1RuAllocBn;
	UINT8      u1RuAllocIdx;

	UINT8      u1Ldpc;
	UINT8      u1Nss;
	UINT8      u1Mcs;
	UINT8      u1TargetRssi;

	UINT32     u4TrigPktSize;
	UINT8      u1RuAllocPs160;
	UINT8      au1Reserved[3];
};

struct GNU_PACKED UNI_MURU_CMM_MANUAL_CONFIG {
	UINT8      u1PdaPol;
	UINT8      u1Band;
	UINT8      u1SpeIdx;
	UINT8      u1ProcType;

	UINT16     u2MloCtrl;
	UINT8      u1SchType;
	UINT8      u1PpduFmt;
	UINT8      u1Ac;
	UINT8      au1Reserved[3];
};

struct GNU_PACKED UNI_MURU_DL_MANUAL_CONFIG {
	UINT8      u1UserCnt;
	UINT8      u1TxMode;
	UINT8      u1Bw;
	UINT8      u1GI;

	UINT8      u1Ltf;
	UINT8      u1SigMcs;
	UINT8      u1SigDcm;
	UINT8      u1SigCmprs;

	UINT16     au2RU[16];

	UINT8      au1C26[2];
	UINT8      u1AckPly;
	UINT8      u1TxPwr;

	UINT16     u2MuPpduDur;
	UINT8      u1AgcDispOrder;
	UINT8      u1Reserved;

	UINT8      u1AgcDispPol;
	UINT8      u1AgcDispRatio;
	UINT16     u2AgcDispLinkMGF;

	UINT16     u2PrmblPuncBmp;
	UINT8      u1Reserved2[2];

	struct UNI_MURU_DL_USER_INFO   arUserInfoDl[UNI_MURU_MAX_NUM_TXCMD_TX_USER];
};

struct GNU_PACKED UNI_MURU_UL_MANUAL_CONFIG {
	UINT8      u1UserCnt;
	UINT8      u1TxMode;

	/* DLTX */
	UINT8      u1BaType;
	UINT8      u1Reserved;

	/* ULTX */
	UINT8      u1UlBw;
	UINT8      u1UlGiLtf;
	UINT16     u2UlLength;

	UINT16     u2TrigCnt;
	UINT8      u1TfPad;
	UINT8      u1TrigType;

	UINT16     u2TrigIntv;
	UINT8      u1TrigTa[MAC_ADDR_LEN];
	UINT16     au2UlRU[16];

	UINT8      au1UlC26[2];
	UINT16     u2AgcDispLinkMGF;

	UINT8      u1AgcDispMuLen;
	UINT8      u1AgcDispPol;
	UINT8      u1AgcDispRatio;
	UINT8      u1AgcDispPuIdx;

	struct UNI_MURU_UL_USER_INFO   arUserInfoUl[UNI_MURU_MAX_NUM_TXCMD_TX_USER];
};

struct GNU_PACKED UNI_MURU_DBG_MANUAL_CONFIG {
	/* HE TB RX Debug */
	UINT_32     u4RxHetbNonsfEnBitmap; //Maximum user:16
	UINT_32     au4RxHetbCfg[2];
};

struct GNU_PACKED UNI_CMD_MURU_T {
	/*Fixed Fields*/
	UINT8 au1Padding[4];
	/*TLV*/
	UINT8 au1TlvBuffer[0];
};

struct GNU_PACKED UNI_MURU_MANUAL_CONFIG_T {
	UINT32     u4ManCfgBmpCmm;
	UINT32     u4ManCfgBmpDl;
	UINT32     u4ManCfgBmpUl;
	UINT32     u4ManCfgBmpDbg;

	struct UNI_MURU_CMM_MANUAL_CONFIG  rCfgCmm;
	struct UNI_MURU_DL_MANUAL_CONFIG   rCfgDl;
	struct UNI_MURU_UL_MANUAL_CONFIG   rCfgUl;
	struct UNI_MURU_DBG_MANUAL_CONFIG  rCfgDbg;
};

struct GNU_PACKED UNI_CMD_MURU_MANUAL_CONFIG_T {
	/* DW_0 */
	UINT16 u2Tag;
	UINT16 u2Length;
	/* DW_1*/
	UINT8   u1CmdVersion;
	UINT8   u1CmdRevision;
	UINT16  u2Reserved;

	struct UNI_MURU_MANUAL_CONFIG_T rMuruManCfg;
};

struct GNU_PACKED UNI_PQS_MANUAL_SCS_CONFIG_T {
	/* DW0 */
	UINT8  u1Mode;
	UINT16 u2WlanIdx;
	UINT8  u1ScsId;
	/* DW1 */
	UINT8  u1Dir;
	UINT8  u1Tid;
	UINT16 u2DelayBound;
	/* DW2 */
	UINT32 u4DataRate;
	/* DW3 */
	UINT32 u4StartTime;
};

struct GNU_PACKED UNI_CMD_PQS_MANUAL_SCS_CONFIG_T
{
	/* DW_0 */
	UINT16 u2Tag;
	UINT16 u2Length;
	/* DW_1*/
	UINT8   u1CmdVersion;
	UINT8   u1CmdRevision;
	UINT16  u2Reserved;

	struct UNI_PQS_MANUAL_SCS_CONFIG_T rPqsScsCfg;
};

struct GNU_PACKED UNI_CMD_MURU_SET_BF_IN_AC01_T
{
	/* DW_0 */
	UINT_16 u2Tag;
	UINT_16 u2Length;
	/* DW_1*/
	UINT_8 u1Enable;
	UINT_8 u1Rsv[3];
};

struct GNU_PACKED UNI_CMD_MURU_SET_UL_TX_TRIG_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1UlTxTrigger;
	UINT8    au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_SET_UL_TX_CNT_RESET_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    fgReset;
	UINT8    au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_GET_UL_TX_CNT_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT32   u4Index;
};

struct GNU_PACKED UNI_CMD_MURU_SET_AGG_POLICY_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8  u1AggPolicy;
	UINT8  u1DurComp;
	UINT8  au1Reserved[2];
};


struct GNU_PACKED UNI_CMD_MURU_SET_MU_TXPKT_CNT {
	/* DW_0 */
	UINT16 u2Tag;
	UINT16 u2Length;
	/* DW_1*/
	UINT8 u1BandIdx;
	UINT8 u1MuTxEn;
	UINT8 u1Rsv[2];
	/* DW_2*/
	UINT32 u4MuTxPktCnt; /* 0: Continueous Tx, else: Limited Tx */
};

struct GNU_PACKED UNI_MURU_MUM_SET_GROUP_TBL_ENTRY {
	UINT16      u2WlidUser0; /* WLANID0 */
	UINT16      u2WlidUser1; /* WLANID1 */
	UINT16      u2WlidUser2; /* WLANID2 */
	UINT16      u2WlidUser3; /* WLANID3 */

	UINT8       u1DlMcsUser0: 4;
	UINT8       u1DlMcsUser1: 4;
	UINT8       u1DlMcsUser2: 4;
	UINT8       u1DlMcsUser3: 4;
	UINT8       u1UlMcsUser0: 4;
	UINT8       u1UlMcsUser1: 4;
	UINT8       u1UlMcsUser2: 4;
	UINT8       u1UlMcsUser3: 4;

	UINT8       u1NumUser: 2;
	UINT8       u1Res: 6;
	UINT8       u1Nss0: 2;
	UINT8       u1Nss1: 2;
	UINT8       u1Nss2: 2;
	UINT8       u1Nss3: 2;
	UINT8       u1RuAlloc;
	UINT8       u1RuAllocExt;

	UINT8       u1Capability;
	UINT8       u1GI;
	UINT8       u1Dl_Ul;
	UINT8       u1Reserved2;
};

struct GNU_PACKED UNI_CMD_MURU_FIXED_GRP_RATE_CTRL_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1CmdVersion;
	UINT8    u1CmdRevision;
	UINT16   u2Reserved;

	struct UNI_MURU_MUM_SET_GROUP_TBL_ENTRY rMuruSetGrpTblEntry;
};

struct GNU_PACKED UNI_CMD_MURU_FIXED_RATE_CTRL_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT16   u2Value;
	UINT16   u2Reserved;
};

struct GNU_PACKED UNI_CMD_MURU_SET_ARB_OP_MODE_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1OpMode;
	UINT8    au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_ALGO_DBG_CTRL_T {
	/* DW_0 */
	UINT16     u2Tag;
	UINT16     u2Length;
	/* DW_1*/
	UINT16     u2Period;
	UINT8      u1OpMode;
	UINT8      u1Enable;
};

struct GNU_PACKED UNI_CMD_MURU_SET_SUTX_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1ForceSuTx;
	UINT8    au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_SET_DBG_INFO {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1 */
	UINT16   u2Item;
	UINT8    au1Reserved[2];
	UINT32   u4Value;
};

struct GNU_PACKED UNI_CMD_MURU_SET_FORCE_MU_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8  u1ForceMu;
	UINT8  au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_GET_GROUP_TBL_ENTRY_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT16   u2Index;
	UINT8    au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_MURU_SET_DLUL_LIMITS_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8   u1BandIdx;
	UINT8   u1Dis160RuMu;
	UINT8   u1MaxRuOfdma;
	UINT8   u1MaxDLMuMimo;
	UINT8   u1MaxULMuMimo;
	UINT8   au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_SET_DLUL_VAL_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8  u1BandBssSelect;
	UINT8  u1Index;
	UINT8  u1DlUlUpdList;
	UINT8  u1DlUlVal;
};

struct GNU_PACKED UNI_CMD_MURU_GET_ULRU_STATUS_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
};

struct GNU_PACKED UNI_CMD_MURU_SET_POLICY_TYPE_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1AckPolicy;
	UINT8    au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_SET_TRIG_TYPE_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1TrigValue;
	UINT8    au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_SET_TRIG_VARIANT_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1TrigVariant;
	UINT8    au1Reserved[3];
};


struct GNU_PACKED UNI_CMD_MURU_SET_20M_DYN_ALGO_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1DynAlgoEnable;
	UINT8    au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_SET_PROT_FRAME_THR_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT32   u4ProtFrameThr;
};

struct GNU_PACKED UNI_CMD_MURU_SET_TXOP_ONOFF_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1TxopOnOff;
	UINT8    au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_CTS_RETRY_CTRL_T {
	/* DW_0 */
	UINT16  u2Tag;
	UINT16  u2Length;
	/* DW_1*/
	UINT16  u2Wcid;
	UINT8   u1Tid;
	UINT8   u1RtsFailThenCtsRetryCnt;
};

struct GNU_PACKED UNI_CMD_MURU_UL_ONOFF_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT16   u2UlBsrpOnOff;
	UINT16   u2UlDataOnOff;
};

#define UNI_CMD_MURU_SET_MDVT_VAL_SIZE 4

struct GNU_PACKED UNI_CMD_MURU_SET_MDVT_INFO_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT16   u2Item;
	UINT_8	 au1Reserved[2];
	UINT_32	 u4Value[UNI_CMD_MURU_SET_MDVT_VAL_SIZE];
};

/* =================== UNI_CMD_ID_MURU End =================== */

/* ============== UNI_CMD_ID_CHAN_SWITCH Begin ============== */

struct GNU_PACKED UNI_CMD_CHAN_SWITCH_T
{
	/*fixed field*/
	uint8_t ucReserved[4];
	/* tlv */
	uint8_t aucTlvBuffer[0];
	/**< the TLVs included in this field:
	*
	*   TAG                              | ID  | structure
	*   ---------------------------------|-----| -------------
	*   UNI_CMD_CHAN_SWITCH              | 0x0 | _UNI_CMD_CHAN_SWITCH_T
	*   UNI_CMD_CHAN_SWITCH_STREAM_PATH  | 0x1 | _UNI_CMD_CHAN_SWITCH_T
	*/
};

struct GNU_PACKED UNI_CMD_CHAN_SWITCH_INFO_T {
	uint8_t	ucPrimCh;
	uint8_t	ucCentralCh;
	uint8_t	ucBW;
	uint8_t	ucTxStreamNum;

	uint8_t	ucRxStreamNum;
	uint8_t	ucSwitchReason;
	uint8_t	ucDbdcIdx;
	uint8_t	ucCentralCh2;

	uint16_t	u2CacCase;
	uint8_t	ucBand;
	uint8_t	aucReserve0[1];

	UINT32  u4OutBandFreq;

	int8_t	cTxPowerDrop;
	uint8_t	ucAPBW;
	uint8_t	ucAPCentralCh;
	uint8_t	aucReserve1[1];

	int8_t	acTxPowerSKU[SKU_TOTAL_SIZE];
	uint8_t	aucReserve2[3];
};

struct GNU_PACKED UNI_CMD_CHAN_SWITCH_TLV_T
{
	uint16_t  u2Tag;                 /* Tag = 0x01 */
	uint16_t  u2Length;
	struct UNI_CMD_CHAN_SWITCH_INFO_T rChanSwitchInfo;
};

/* Channel Switch command Tag */
enum UNI_CMD_CHAN_SWITCH_TAG_T {
	UNI_CMD_CHAN_SWITCH = 0,
	UNI_CMD_CHAN_SWITCH_STREAM_PATH = 1,
	UNI_CMD_CHAN_SWITCH_MAX_NUM
};

/* ============== UNI_CMD_ID_CHAN_SWITCH End ============== */

/* ============== UNI_CMD_ID_EAP_CTRL Begin ============== */
struct GNU_PACKED UNI_CMD_EAP_CTRL_T {
	/*fixed field*/
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

enum UNI_CMD_ID_EAP_TAG_T {
	UNI_CMD_EAP_INIT_IPI_CTRL = 0x0,
	UNI_CMD_EAP_GET_IPI_VALUE,
	UNI_CMD_EAP_SET_DATA_TXPWR_OFFSET,
	UNI_CMD_EAP_SET_RA_TABLE_DATA,
	UNI_CMD_EAP_GET_RATE_INFO,
	UNI_CMD_EAP_MAX_NUM
};

struct GNU_PACKED UNI_CMD_EAP_INIT_IPI_CTRL_T {
	/* DW_0 */
	UINT16 u2Tag;
	UINT16 u2Length;
	/* DW_1*/
	UINT32 u4EapCtrlCmdId;
	UINT8  u1BandIdx;
	UINT8  au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_EAP_SET_DATA_TXPWR_OFFSET_T {
	/* DW_0 */
	UINT16 u2Tag;
	UINT16 u2Length;
	/* DW_1*/
	UINT32 u4EapCtrlCmdId;
	UINT8  u1WlanIdx;
	INT8   i1TxPwrOffset;
	UINT8  u1BandIdx;
	UINT8  u1Reserved;
};

struct GNU_PACKED UNI_CMD_EAP_SET_RA_TABLE_T {
	/* DW_0 */
	UINT16 u2Tag;
	UINT16 u2Length;
	/* DW_1*/
	UINT32 u4EapCtrlCmdId;
	UINT8  u1RaTblTypeIdx;
	UINT8  u1RaTblIdx;
	UINT8  u1BandIdx;
	UINT8  u1Reserved1;
	UINT16 u2RaTblLength;
	UINT16 u2Reserved2;
	UINT8  ucBuf[512];
};

struct GNU_PACKED UNI_CMD_EAP_SHOW_RATE_TABLE_T {
	/* DW_0 */
	UINT16 u2Tag;
	UINT16 u2Length;
	/* DW_1*/
	UINT32 u4EapCtrlCmdId;
	UINT8  u1RaTblTypeIdx;
	UINT8  u1RaTblIdx;
	UINT8  u1BandIdx;
	UINT8  u1RW;
};
/* ============== UNI_CMD_ID_EAP_CTRL End ============== */

/* ============== UNI_CMD_ID_DYN_WMM_CTRL Begin ============== */
struct GNU_PACKED UNI_CMD_DYN_WMM_T {
	/*fixed field*/
	UINT16 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

enum UNI_CMD_ID_DYN_WMM_TAG_T {
	UNI_CMD_DYN_WMM_ENABLE = 0x0,
	UNI_CMD_DYN_WMM_SET_DATA,
	UNI_CMD_DYN_WMM_MAX_NUM
};

struct GNU_PACKED UNI_CMD_DYNAMIC_WMM_ENABLE {
	/* DW_0 */
	UINT16 u2Tag;
	UINT16 u2Length;
	/* DW_1*/
	UINT8 u1BandIdx;
	UINT8 u1DynWmmEnable;
	UINT8 au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_DYMWMM_CTRL_DATA_T {
	/* DW_0 */
	UINT16 u2Tag;
	UINT16 u2Length;
	/* DW_1*/
	UINT8  u1BandIdx;
	UINT8  ActiveSTA;
	UINT8  fgRxOnly;
	UINT8  u1Reserved;
};

/* ============== UNI_CMD_ID_DYN_WMM_CTRL End ============== */

/* =================== UNI_CMD_ID_PP Begin =================== */
typedef struct GNU_PACKED _UNI_CMD_PP_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_PP_T, *P_UNI_CMD_PP_T;

typedef enum _UNI_CMD_ID_PP_TAG_T {
	UNI_CMD_PP_EN_CTRL = 0x0,
	UNI_CMD_PP_ALG_CTRL = 0x1,
	UNI_CMD_PP_DSCB_CTRL = 0x2,
	UNI_CMD_PP_MU_OFDMA_CTRL = 0x3,
	UNI_CMD_PP_CHANGE_CAP_CTRL = 0x4,
	UNI_CMD_PP_NUM
} UNI_CMD_ID_PP_TAG_T;

typedef struct GNU_PACKED _UNI_CMD_PP_EN_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
	UINT_8   u1PpMgmtMode;
	UINT_8   u1DbdcIdx;
	UINT_8   u1PpCtrl;
	UINT_8   u1PpMgmtEn;
	UINT_16  u1PpBitMap;
	BOOLEAN  fgPpCsaEn;
	UINT_8   u1Reserved[1];
} UNI_CMD_PP_EN_CTRL_T, *P_UNI_CMD_PP_EN_CTRL_T;

enum UNI_CMD_PP_ALG_CMD_ACTION {
	UNI_CMD_PP_ALG_SET_TIMER = 0,
	UNI_CMD_PP_ALG_SET_THR = 1,
	UNI_CMD_PP_ALG_GET_STATISTICS = 2,
	UNI_CMD_PP_ALG_MAX_NUM
};

struct GNU_PACKED UNI_CMD_PP_ALG_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	/* tag specific part */
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
	UINT8 u1DbdcIdx;
	UINT8 u1PpAction;
	UINT8 u1Reset;
	UINT8 u1Reserved[1];
};

#ifdef CFG_SUPPORT_FALCON_PP
typedef INT32 (*PFN_PP_EN_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
										P_PP_CMD_T pp_cmd_cap, VOID *pHandle);
typedef INT32 (*PFN_PP_DSCB_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
										struct CMD_STATIC_PP_DSCB_T *pp_dscb, VOID *pHandle);
typedef INT32 (*PFN_PP_MU_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
										struct CMD_PP_MU_CTRL_T *pp_mu_ctrl, VOID *pHandle);
typedef INT32 (*PFN_PP_CHANGE_CAP_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
										struct CMD_CHANGE_PP_CAP_CTRL_T *pp_cap, VOID *pHandle);

#endif /* CFG_SUPPORT_FALCON_PP */

/* =================== UNI_CMD_ID_PP End =================== */
/* ============== UNI_CMD_ID_TESTMODE_RX_STAT  Begin ============== */

struct GNU_PACKED _UNI_CMD_TESTMODE_RX_STAT_T {
    /*fix field*/
	UINT_8 u1Band;
	UINT_8 au1Reserved[3];

    /*tlv */
	UINT_8 aucTlvBuffer[0];/**< the TLVs included in this field:
	*
	*   TAG                                    | ID  | structure
	*   ---------------------------------------|-----|--------------
	*   UNI_CMD_TESTMODE_GET_RX_STAT           | 0x0 | UNI_CMD_TESTMODE_GET_RX_STAT_T
	*   UNI_CMD_TESTMODE_SET_RXV_CTRL          | 0x1 | UNI_CMD_TESTMODE_SET_RXV_CTRL_T
	*   UNI_CMD_TESTMODE_SET_RXV_RU_CTRL       | 0x2 | UNI_CMD_TESTMODE_SET_RXV_RU_CTRL_T
	*   UNI_CMD_TESTMODE_GET_RX_STAT_BAND      | 0x3 | UNI_CMD_TESTMODE_GET_RX_STAT_BAND_T
	*   UNI_CMD_TESTMODE_GET_RX_STAT_PATH      | 0x4 | UNI_CMD_TESTMODE_GET_RX_STAT_PATH_T
	*   UNI_CMD_TESTMODE_GET_RX_STAT_USER      | 0x5 | UNI_CMD_TESTMODE_GET_RX_STAT_USER_T
	*   UNI_CMD_TESTMODE_GET_RX_STAT_COMM      | 0x6 | UNI_CMD_TESTMODE_GET_RX_STAT_COMM_T
	*   UNI_CMD_TESTMODE_SET_RX_STAT_USER_CTRL | 0x7 | UNI_CMD_TESTMODE_SET_RX_STAT_USER_CTRL_T
	*/
};

enum _UNI_CMD_TESTMODE_RX_STAT_TAG_T {
	UNI_CMD_TESTMODE_GET_RX_STAT = 0,
	UNI_CMD_TESTMODE_SET_RXV_CTRL,
	UNI_CMD_TESTMODE_SET_RXV_RU_CTRL,
	UNI_CMD_TESTMODE_GET_RX_STAT_BAND,
	UNI_CMD_TESTMODE_GET_RX_STAT_PATH,
	UNI_CMD_TESTMODE_GET_RX_STAT_USER,
	UNI_CMD_TESTMODE_GET_RX_STAT_COMM,
	UNI_CMD_TESTMODE_SET_RX_STAT_USER_CTRL,
	UNI_CMD_TESTMODE_GET_RX_STAT_ALL = 8,
	UNI_CMD_TESTMODE_GET_RX_STAT_ALL_V2 = 9,

	UNI_CMD_TESTMODE_MAX_NUM,
};

#define UNI_CMD_TESTMODE_GET_RX_STAT_FEATURE (1<<UNI_CMD_TESTMODE_GET_RX_STAT)
#define UNI_CMD_TESTMODE_SET_RXV_CTRL_FEATURE (1<<UNI_CMD_TESTMODE_SET_RXV_CTRL)
#define UNI_CMD_TESTMODE_SET_RXV_RU_CTRL_FEATURE (1<<UNI_CMD_TESTMODE_SET_RXV_RU_CTRL)
#define UNI_CMD_TESTMODE_GET_RX_STAT_BAND_FEATURE (1<<UNI_CMD_TESTMODE_GET_RX_STAT_BAND)
#define UNI_CMD_TESTMODE_GET_RX_STAT_PATH_FEATURE (1<<UNI_CMD_TESTMODE_GET_RX_STAT_PATH)
#define UNI_CMD_TESTMODE_GET_RX_STAT_USER_FEATURE (1<<UNI_CMD_TESTMODE_GET_RX_STAT_USER)
#define UNI_CMD_TESTMODE_GET_RX_STAT_COMM_FEATURE (1<<UNI_CMD_TESTMODE_GET_RX_STAT_COMM)
#define UNI_CMD_TESTMODE_SET_RX_STAT_USER_CTRL_FEATURE (1<<UNI_CMD_TESTMODE_SET_RX_STAT_USER_CTRL)
#define UNI_CMD_TESTMODE_GET_RX_STAT_ALL_FEATURE (1<<UNI_CMD_TESTMODE_GET_RX_STAT_ALL_V2)


struct GNU_PACKED _UNI_CMD_TESTMODE_GET_RX_STAT_INFO_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 u1BandIdx;
	UINT_8 au1Reserved[3];
};

struct GNU_PACKED _UNI_CMD_TESTMODE_SET_RXV_CTRL_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 fgRxvEnable;
	UINT_8 au1Reserved[3];
};

struct GNU_PACKED _UNI_CMD_TESTMODE_SET_RXV_RU_CTRL_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 u1RxvRuIdx;
	UINT_8 au1Reserved[3];
};

struct GNU_PACKED _UNI_CMD_TESTMODE_GET_RX_STAT_INFO_BAND_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 u1BandIdx;
	UINT_8 u1reserved[3];
};

struct GNU_PACKED _UNI_CMD_TESTMODE_GET_RX_STAT_INFO_PATH_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 u1PathIdx;
	UINT_8 u1reserved[3];
};

struct GNU_PACKED _UNI_CMD_TESTMODE_GET_RX_STAT_INFO_USER_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 u1UserIdx;
	UINT_8 u1reserved[3];
};

struct GNU_PACKED _UNI_CMD_TESTMODE_GET_RX_STAT_INFO_COMM_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
};

struct GNU_PACKED _UNI_CMD_TESTMODE_SET_RX_STAT_USER_CTRL_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 u1BandIdx;
	UINT_8 aucReserved;
	UINT_16 u2UserIdx;
};

/* Get rx info all cmd struct (Tag 0x08) */
struct GNU_PACKED _UNI_CMD_TESTMODE_GET_RX_STAT_ALL_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 u1BandIdx;
	UINT_8 aucReserved[3];
};

struct GNU_PACKED _UNI_CMD_TESTMODE_RX_T {
	struct _UNI_CMD_TESTMODE_GET_RX_STAT_INFO_T rRxStatCmd;
	struct _UNI_CMD_TESTMODE_SET_RXV_CTRL_T rRxvCtrlCmd;
	struct _UNI_CMD_TESTMODE_SET_RXV_RU_CTRL_T rRxvRuCtrlCmd;
	struct _UNI_CMD_TESTMODE_GET_RX_STAT_INFO_BAND_T rRxStatBandCmd;
	struct _UNI_CMD_TESTMODE_GET_RX_STAT_INFO_PATH_T rRxStatPathCmd;
	struct _UNI_CMD_TESTMODE_GET_RX_STAT_INFO_USER_T rRxStatUserCmd;
	struct _UNI_CMD_TESTMODE_GET_RX_STAT_INFO_COMM_T rRxStatCommCmd;
	struct _UNI_CMD_TESTMODE_SET_RX_STAT_USER_CTRL_T rRxStatUserCtrlCmd;
	struct _UNI_CMD_TESTMODE_GET_RX_STAT_ALL_T rRxStatAllCmd;
};
typedef INT32 (*PFN_TESTMODE_RX_STAT_HANDLE)(struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_TESTMODE_GET_RX_STAT_INFO_T *p_RxStatInfo,
	VOID *pHandle);

typedef INT32 (*PFN_TESTMODE_RXV_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_TESTMODE_SET_RXV_CTRL_T *p_RxvCtrl,
	VOID *pHandle);

typedef INT32 (*PFN_TESTMODE_RXV_RU_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_TESTMODE_SET_RXV_RU_CTRL_T *p_RxvRuCtrl,
	VOID *pHandle);

typedef INT32 (*PFN_TESTMODE_RX_STAT_INFO_BAND_HANDLE)(struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_TESTMODE_GET_RX_STAT_INFO_BAND_T *p_RxStatInfoBand,
	VOID *pHandle);

typedef INT32 (*PFN_TESTMODE_RX_STAT_INFO_PATH_HANDLE)(struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_TESTMODE_GET_RX_STAT_INFO_PATH_T *p_RxStatInfoPath,
	VOID *pHandle);

typedef INT32 (*PFN_TESTMODE_RX_STAT_INFO_USER_HANDLE)(struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_TESTMODE_GET_RX_STAT_INFO_USER_T *p_RxStatInfoUser,
	VOID *pHandle);

typedef INT32 (*PFN_TESTMODE_RX_STAT_INFO_COMM_HANDLE)(struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_TESTMODE_GET_RX_STAT_INFO_COMM_T *p_RxStatInfoComm,
	VOID *pHandle);

typedef INT32 (*PFN_TESTMODE_RX_STAT_USER_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_TESTMODE_SET_RX_STAT_USER_CTRL_T *p_RxStatUserCtrl,
	VOID *pHandle);

typedef INT32 (*PFN_TESTMODE_RX_STAT_ALL_HANDLE)(struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_TESTMODE_GET_RX_STAT_ALL_T *p_RxStatAll,
	VOID *pHandle);

/* ============== UNI_CMD_ID_TESTMODE_RX_STAT End ============== */

/* ============== UNI_CMD_ID_BF  Begin ============== */
struct GNU_PACKED UNI_CMD_BF {
	/* fixed field */
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0]; /**<the TLVs includer in this field:
	    *
	    *  TAG                                 | ID   | structure
	    *  ------------------------------------| -----| -------------
	    *  UNI_CMD_BF_SOUNDING_OFF       | 0x0  | N/A
	    *  UNI_CMD_BF_SOUNDING_ON              | 0x1  | UNI_CMD_BF_SND
	    *  UNI_CMD_BF_DATA_PACKET_APPLY    | 0x2  | UNI_CMD_BF_APPLY_CTRL
	    *  UNI_CMD_BF_PFMU_MEM_ALLOCATE | 0x3  | UNI_CMD_BF_MEM_ALLOC
	    *  UNI_CMD_BF_PFMU_MEM_RELEASE   | 0x4  | UNI_CMD_BF_MEM_ALLOC
	    *  UNI_CMD_BF_PFMU_TAG_READ     | 0x5  | UNI_CMD_BF_PFMU_TAG_RW
	    *  UNI_CMD_BF_PFMU_TAG_WRITE   | 0x6  | UNI_CMD_BF_PFMU_TAG_RW
	    *  UNI_CMD_BF_PROFILE_READ    | 0x7  | UNI_CMD_BF_PFMU_DATA_R
	    *  UNI_CMD_BF_PROFILE_WRITE   | 0x8  | UNI_CMD_BF_PFMU_DATA_W
	    *  UNI_CMD_BF_PFMU_MEM_ALLOC_MAP_READ  | 0x9  | N/A
	    *  UNI_CMD_BF_AID_SET              | 0xA  | UNI_CMD_PEER_AID
	    *  UNI_CMD_BF_STA_REC_READ   | 0xB  | UNI_CMD_BF_STAREC_READ
	    *  UNI_CMD_BF_PHASE_CALIBRATION  | 0xC  | UNI_CMD_IBF_PHASE_CAL
	    *  UNI_CMD_BF_IBF_PHASE_COMP    | 0xD  | UNI_CMD_IBF_PHASE_COMP
	    *  UNI_CMD_BF_LNA_GAIN_CONFIG      | 0xE  | UNI_CMD_IBF_LNA_GAIN
	    *  UNI_CMD_BF_PROFILE_WRITE_20M_ALL  | 0xF  | UNI_CMD_BF_PFMU_ALL
	    *  UNI_CMD_BF_APCLIENT_CLUSTER       | 0x10 | UNI_CMD_APCLIENT_BF
	    *  UNI_CMD_BF_HW_ENABLE_STATUS | 0x11 | UNI_CMD_BF_HW_ENABLE
	    *  UNI_CMD_BF_BFEE_HW_CTRL         | 0x12 | UNI_CMD_BF_BFEE_CTRL
	    *  UNI_CMD_BF_PFMU_SW_WRITE   | 0x13 | UNI_CMD_ETXBF_PFMU_SW
	    *  UNI_CMD_BF_MOD_EN_CTRL        | 0x14 | UNI_CMD_BF_MOD_EN_CTRL
	    *  UNI_CMD_BF_CONFIG             | 0x15 | UNI_CMD_BF_CONFIG
	    *  UNI_CMD_BF_PFMU_DATA_WRITE      | 0x16 | UNI_CMD_ETXBf_PFMU
	    *  UNI_CMD_BF_FBRPT_DBG_INF0   | 0x17 | UNI_CMD_TXBF_FBRPT_INFO
	    *  UNI_CMD_BF_CMD_TXSND_INFO       | 0x18 | UNI_CMD_BF_SND_CMD
	    *  UNI_CMD_BF_CMD_PLY_INFO       | 0x19 | UNI_CMD_BF_PLY_CMD
	    *  UNI_CMD_BF_CMD_MU_METRIC    | 0x1A | UNI_CMD_HERA_MU_METRIC
	    *  UNI_CMD_BF_CMD_TXCMD         | 0x1B | UNI_CMD_BF_TXCMD_CMD
	    *  UNI_CMD_BF_CMD_CFG_PHY       | 0x1C | UNI_CMD_BF_CFG_BF_PHY
	    *  UNI_CMD_BF_CMD_SND_CNT   | 0x1D | UNI_CMD_BF_SND_CNT_CMD
	    *  UNI_CMD_BF_PROFILE_WRITE_20M_ALL_5X5 | 0x1E | UNI_CMD_BF_PROFILE_WRITE_20M_ALL_5X5
	    */
};

enum UNI_CMD_BF_TAG {
	UNI_CMD_BF_SOUNDING_OFF = 0x00,
	UNI_CMD_BF_SOUNDING_ON = 0x01,
	UNI_CMD_BF_DATA_PACKET_APPLY = 0x02,
	UNI_CMD_BF_PFMU_MEM_ALLOCATE = 0x03,
	UNI_CMD_BF_PFMU_MEM_RELEASE = 0x04,
	UNI_CMD_BF_PFMU_TAG_READ = 0x05,
	UNI_CMD_BF_PFMU_TAG_WRITE = 0x06,
	UNI_CMD_BF_PROFILE_READ = 0x7,
	UNI_CMD_BF_PROFILE_WRITE = 0x8,
	UNI_CMD_BF_PFMU_MEM_ALLOC_MAP_READ = 0x09,
	UNI_CMD_BF_AID_SET = 0x0a,
	UNI_CMD_BF_STA_REC_READ = 0x0b,
	UNI_CMD_BF_PHASE_CALIBRATION = 0x0c,
	UNI_CMD_BF_IBF_PHASE_COMP = 0x0d,
	UNI_CMD_BF_LNA_GAIN_CONFIG = 0x0e,
	UNI_CMD_BF_PROFILE_WRITE_20M_ALL = 0x0f,
	UNI_CMD_BF_APCLIENT_CLUSTER = 0x10,
	UNI_CMD_BF_HW_ENABLE_STATUS_UPDATE = 0x11,
	UNI_CMD_BF_BFEE_HW_CTRL = 0x12,
	UNI_CMD_BF_PFMU_SW_TAG_WRITE = 0x13,
	UNI_CMD_BF_MOD_EN_CTRL = 0x14,
	UNI_CMD_BF_CONFIG = 0x15,
	UNI_CMD_BF_PFMU_DATA_WRITE = 0x16,
	UNI_CMD_BF_FBRPT_DBG_INFO_READ = 0x17,
	UNI_CMD_BF_CMD_TXSND_INFO = 0x18,
	UNI_CMD_BF_CMD_PLY_INFO = 0x19,
	UNI_CMD_BF_CMD_MU_METRIC = 0x1a,
	UNI_CMD_BF_CMD_TXCMD = 0x1b,
	UNI_CMD_BF_CMD_CFG_PHY = 0x1c,
	UNI_CMD_BF_CMD_SND_CNT = 0x1d,
	UNI_CMD_BF_PROFILE_WRITE_20M_ALL_5X5 = 0x1e,
	UNI_CMD_BF_CMD_MAX_NUM
};

struct GNU_PACKED UNI_CMD_BF_SND {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1SuMuSndMode;
	UINT8  u1StaNum;
	UINT8  au1Reserved[2];
	UINT16 u2WlanId[4];
	UINT32 u4SndIntv;
};

struct GNU_PACKED UNI_CMD_BF_APPLY_CTRL {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2WlanIdx;
	UINT8  fgETxBf;
	UINT8  fgITxBf;
	UINT8  fgMuTxBf;
	UINT8  fgPhaseCali;
	UINT8  au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_BF_MEM_ALLOC_CTRL {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2WlanIdx;
	UINT8  u1SuMu;
	UINT8  au1Reserved[5];
};

struct GNU_PACKED UNI_CMD_BF_PFMU_TAG_RW {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1PfmuId;
	UINT8  fgBFer;
	UINT8  u1TxBf;
	UINT8  au1Reserved[5];
	UINT32 au4BfPfmuTag1RawData[7];
	UINT32 au4BfPfmuTag2RawData[7];
};

struct GNU_PACKED UNI_CMD_BF_PFMU_DATA_R {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1PfmuId;
	UINT8  fgBFer;
	UINT16 u2SubCarIdx;
	UINT8  u1TxBf;
	UINT8  au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_BF_PFMU_DATA_W {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2SubCarIdx;
	UINT8  u1PfmuId;
	UINT8  au1Reserved[5];
	UINT32 au4BfPfmuRawData[33];
};

struct GNU_PACKED UNI_CMD_PEER_AID {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2WlanIdx;
	UINT16 u2Aid;
	UINT8  u1OwnMacIdx;
	UINT8  u1BandIdx;
	UINT8  au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_IBF_PHASE_CAL_CTRL {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1Group_L_M_H;
	UINT8  u1Group;
	UINT8  u1DbdcBandIdx;
	UINT8  u1PhaseCalType;
	UINT8  u1PhaseVerifyLnaGainLevel;
	UINT8  u1BandIdx;
	UINT8  u1PhaseOutRptVersion;
	UINT8  au1Reserved[1];
};

struct GNU_PACKED UNI_CMD_IBF_PHASE_COMP {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1BW;
	UINT8  fgJPCh;
	UINT8  u1DbdcBandIdx;
	UINT8  fgRdFromE2p;
	UINT8  fgDisComp;
	UINT8  u1GroupId;
	UINT8  au1Reserved[2];
	UINT8  au1Buf[40];
};

struct GNU_PACKED UNI_CMD_IBF_LNA_GAIN {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1LnaGain;
	UINT8  fgSx2;
	UINT8  u1TxStreamNum;
	UINT8  u1BandIdx;
};

struct GNU_PACKED UNI_CMD_BF_PFMU_DATA_ALL_W {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1PfmuId;
	UINT8  u1TxBf;
	UINT8  au1Reserved[2];
	UINT8  au1Buf[512];
};

struct GNU_PACKED UNI_CMD_BF_PFMU_DATA_ALL_5X5_W {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1PfmuId;
	UINT8  u1TxBf;
	UINT8  au1Reserved[2];
	UINT8  au1Buf[640];
};

struct GNU_PACKED UNI_CMD_APCLIENT_BF {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2WlanIdx;
	UINT16 u2CmmWlanIdx;
	UINT8  au1Reserved[4];
};

struct GNU_PACKED UNI_CMD_BF_STAREC_READ {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2WlanIdx;
	UINT8  au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_BF_HW_EN_STATUS_UPDATE {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1EBfHwEnStatus;
	UINT8  u1IBfHwEnStatus;
	UINT8  au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_BF_BFEE_CTRL {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  fgSndReqChkEn;
	UINT8  au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_BF_MOD_ENABLE_CTRL {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1BfNum;
	UINT8  u1BfBitmap;
	UINT8  au1BFSel[8];
	UINT8  au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_BF_CONFIG_CTRL {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT32 u4ConfigType;
	UINT8  u1ConfigPara[6];
	UINT8  au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_ETXBf_PFMU_FULL_DIM_DATA_W {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2SubCarr;
	UINT8  u1PfmuIdx;
	UINT8  u1BandIdx;
	UINT8  fgBfer;
	UINT8  au1Reserved[3];
	UINT8  au1Buf[268];
};

struct GNU_PACKED UNI_CMD_BF_PLY_CMD {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2WlanIdx;
	UINT8  u1Action;
	UINT8  u1GloOpt;
	UINT8  u1GrpIBfOpt;
	UINT8  u1GrpEBfOpt;
	UINT8  u1Nss;
	UINT8  u1SSPly;
	UINT8  au1Reserved[4];
};

struct GNU_PACKED UNI_CMD_HERA_MU_METRIC {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1Action;
	UINT8  u1ReadClr;
	UINT8  u1Band;
	UINT8  u1NUser;  /* 0: Single user, 1~3: two ~ four users */
	UINT8  u1DBW;     /* 0: BW20, 1: BW40, 2: BW80, 3: BW160 */
	UINT8  u1NTxer;   /* 1~3: two ~ four antennas */
	UINT8  u1PFD;     /* 0x4: VHT, 0xB: HE MU */
	UINT8  u1RuSize;
	UINT8  u1RuIdx;   /* RU allocation, as defined in standard. */
	UINT8  u1SpeIdx;  /* Always 24 */
	UINT8  u1SpeedUp; /* Always 0. */
	UINT8  u1LDPC;    /* 0 - BCC, 1 - LDPC */
	UINT8  u1PollingTime;
	BOOLEAN fgEnable; /* Enable MU Metric */
	UINT8  u1MuInitRatePly; /* MU Initial Rate Policy */
	UINT8  au1Reserved;
	UINT8  u1NStsUser[UNI_CMD_HERA_METRIC_USERS];
	UINT16 u2PfidUser[UNI_CMD_HERA_METRIC_USERS]; /* profile ID in PFMU.*/
};

struct GNU_PACKED UNI_CMD_BF_TXCMD {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1Action;
	UINT8  fgTxCmdBfManual;
	UINT8  u1TxCmdBfBit;
	UINT8  u1Reserved[5];
};

struct GNU_PACKED UNI_CMD_BF_CFG_BF_PHY {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1Action;
	UINT8  u1BandIdx;
	UINT8  u1SmthIntlBypass;
	UINT8  u1Reserved[5];
};

struct GNU_PACKED UNI_CMD_BF_SND_CNT_CMD {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2Action;
	UINT16 u2SndCntLmtMan;
	UINT8  u1Reserved[4];
};

struct GNU_PACKED UNI_CMD_BF_SND_CMD {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1Action;
	UINT8  u1ReadClr;
	UINT8  u1VhtOpt;
	UINT8  u1HeOpt;
	UINT16 u2WlanIdx;
	UINT8  u1GloOpt;
	UINT8  u1SndIntv;
	UINT8  u1SndStop;
	UINT8  u1MaxSndStas;
	UINT8  u1TxTime;
	UINT8  u1Mcs;
	UINT8  u1LDPC;
	UINT8  u1Inf;
	UINT8 fgMan;
	UINT8 ucAcQ;
	UINT8 fgSxnProtect;
	UINT8 fgDirectFBK;
	INT8	i1Thr1;
	INT8	i1Thr2;
	BOOLEAN fgSwSolOn;
	UINT8 au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_TXBF_FBRPT_DBG_INFO {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2WlanIdx;
	UINT8  u1Action;
	UINT8  u1BandIdx;
	UINT8  u1PollPFMUIntrStatTimeOut;
	UINT8  u1FbRptDeQInterval;
	UINT8  u1PFMUUpdateEn;
	UINT8  au1Reserved[1];
};

struct GNU_PACKED UNI_CMD_ETXBF_PFMU_SW_TAG {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8  u1Lm;
	UINT8  u1Nr;
	UINT8  u1Nc;
	UINT8  u1Bw;
	UINT8  u1Codebook;
	UINT8  u1group;
	UINT8  u1TxBf;
	UINT8  au1Reserved[5];
};

/* ============== UNI_CMD_ID_BF  End ============== */

/* =================== UNI_CMD_ID_TPC Begin =================== */
typedef struct GNU_PACKED _UNI_CMD_TPC_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_TPC_T, *P_UNI_CMD_TPC_T;

typedef enum _UNI_CMD_ID_TPC_TAG_T {
    UNI_CMD_TPC_ACT_MANUAL_MODE = 0x0,
    UNI_CMD_TPC_ACT_UL_TX_POWER_CONFIG = 0x1,
    UNI_CMD_TPC_ACT_UL_TARGET_RSSI_CONFIG = 0x2,
    UNI_CMD_TPC_ACT_UL_UPH_MIN_PWR_FG_CONFIG = 0x3,
    UNI_CMD_TPC_ACT_DL_TX_POWER_CMD_CTRL_CONFIG = 0x4,
    UNI_CMD_TPC_ACT_DL_TX_POWER_CONFIG = 0x5,
    UNI_CMD_TPC_ACT_DL_TX_POWER_ALPHA_CONFIG = 0x6,
    UNI_CMD_TPC_ACT_MAN_TBL_INFO = 0x7,
    UNI_CMD_TPC_ACT_WLANID_CTRL = 0x8,
    UNI_CMD_TPC_ACT_UL_UNIT_TEST_CONFIG = 0x9,
    UNI_CMD_TPC_ACT_UL_UNIT_TEST_GO = 0xA,
    UNI_CMD_TPC_ACT_ENABLE_CONFIG = 0xB,
    UNI_CMD_TPC_ALGO_ACTION_NUM
} UNI_CMD_ID_TPC_TAG_T;

/** enum for tpc parameter mode */
typedef enum _UNI_ENUM_TPC_PARAM_MODE {
    UNI_TPC_PARAM_AUTO_MODE = 0,
    UNI_TPC_PARAM_MAN_MODE,
    UNI_TPC_PARAM_MODE_NUM
} UNI_ENUM_TPC_PARAM_MODE, *P_UNI_ENUM_TPC_PARAM_MODE;

/** enum for down-link tx type */
typedef enum _UNI_ENUM_TPC_DL_TX_TYPE {
    UNI_TPC_DL_TX_TYPE_MU_MIMO = 0,
    UNI_TPC_DL_TX_TYPE_MU_OFDMA,
    UNI_TPC_DL_TX_TYPE_NUM
} UNI_ENUM_TPC_DL_TX_TYPE, *P_UNI_ENUM_TPC_DL_TX_TYPE;

typedef struct GNU_PACKED _UNI_CMD_TPC_MAN_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCtrlFormatId;
    BOOLEAN fgTpcEnable;
    UINT8 u1Reserved[2];
    UNI_ENUM_TPC_PARAM_MODE eTpcParamMode;
} UNI_CMD_TPC_MAN_CTRL_T, *P_UNI_CMD_TPC_MAN_CTRL_T;

typedef INT32 (*PFN_TPC_MAN_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_TPC_MAN_CTRL_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TPC_UL_ALGO_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCtrlFormatId;
    UINT8 u1ApTxPwr;
    UINT8 u1EntryIdx;
    UINT8 u1TargetRssi;
    UINT8 u1UPH;
    BOOLEAN fgMinPwrFlag;
    UINT8 u1Reserved[2];
} UNI_CMD_TPC_UL_ALGO_CTRL_T, *P_UNI_CMD_TPC_UL_ALGO_CTRL_T;

typedef INT32 (*PFN_TPC_UL_ALGO_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_TPC_UL_ALGO_CTRL_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TPC_DL_ALGO_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCtrlFormatId;
    INT8 i1DlTxPwr;
    BOOLEAN fgDlTxPwrCmdCtrl;
    UINT8 u1EntryIdx;
    UNI_ENUM_TPC_DL_TX_TYPE eTpcDlTxType;
	INT16 i2DlTxPwrAlpha;
	UINT8 au1Reserved[2];
} UNI_CMD_TPC_DL_ALGO_CTRL_T, *P_UNI_CMD_TPC_DL_ALGO_CTRL_T;

typedef INT32 (*PFN_TPC_DL_ALGO_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_TPC_DL_ALGO_CTRL_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TPC_MAN_TBL_INFO_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCtrlFormatId;
    BOOLEAN fgUplink;
    UINT8 u1Reserved[2];
} UNI_CMD_TPC_MAN_TBL_INFO_T, *P_UNI_CMD_TPC_MAN_TBL_INFO_T;

typedef INT32 (*PFN_TPC_MAN_TBL_INFO_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_TPC_MAN_TBL_INFO_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TPC_MAN_WLAN_ID_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCtrlFormatId;
    UINT8 u1EntryIdx;
    UINT16 u2WlanId;
    UNI_ENUM_TPC_DL_TX_TYPE eTpcDlTxType;
	BOOLEAN fgUplink;
	UINT8 u1Reserved[3];
} UNI_CMD_TPC_MAN_WLAN_ID_CTRL_T, *P_UNI_CMD_TPC_MAN_WLAN_ID_CTRL_T;

typedef INT32 (*PFN_TPC_MAN_WLAN_ID_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_TPC_MAN_WLAN_ID_CTRL_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TPC_UL_UT_VAR_CFG_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCtrlFormatId;
    UINT8 u1EntryIdx;
    INT16 i2Value;
    UINT8 u1VarType;
    UINT8 u1Reserved;
} UNI_CMD_TPC_UL_UT_VAR_CFG_T, *P_UNI_CMD_TPC_UL_UT_VAR_CFG_T;

typedef INT32 (*PFN_TPC_UL_UT_VAR_CFG_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_TPC_UL_UT_VAR_CFG_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TPC_UL_UT_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCtrlFormatId;
    BOOLEAN fgTpcUtGo;
    UINT8 u1Reserved[2];
} UNI_CMD_TPC_UL_UT_CTRL_T, *P_UNI_CMD_TPC_UL_UT_CTRL_T;

typedef INT32 (*PFN_TPC_UL_UT_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_TPC_UL_UT_CTRL_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TPC_PARAM_T {
	BOOLEAN bQuery;
	BOOLEAN TPCTagValid[UNI_CMD_TPC_ALGO_ACTION_NUM];

	UNI_CMD_TPC_MAN_CTRL_T TPCManCtrl;
	UNI_CMD_TPC_UL_ALGO_CTRL_T TPCUlAlgoCtrl;
	UNI_CMD_TPC_DL_ALGO_CTRL_T TPCDlAlgoCtrl;
	UNI_CMD_TPC_MAN_TBL_INFO_T TPCManTblInfo;
	UNI_CMD_TPC_MAN_WLAN_ID_CTRL_T TPCManWlanIDCtrl;
	UNI_CMD_TPC_UL_UT_VAR_CFG_T TPCUlUTVarCfg;
	UNI_CMD_TPC_UL_UT_CTRL_T TPCUlUTCtrl;
} UNI_CMD_TPC_PARAM_T, *P_UNI_CMD_TPC_PARAM_T;

/* =================== UNI_CMD_ID_TPC End =================== */

/* =================== UNI_CMD_ID_MEC Begin =================== */

struct GNU_PACKED UNI_CMD_MEC_T {
	/* fixed field */
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

enum UNI_CMD_ID_MEC_TAG_T {
	UNI_CMD_MEC_READ_INFO = 0,
	UNI_CMD_MEC_AMSDU_ALGO_EN_STA = 1,
	UNI_CMD_MEC_AMSDU_PARA_STA = 2,
	UNI_CMD_MEC_AMSDU_ALGO_THRESHOLD = 3,
	UNI_CMD_MEC_IFAC_SPEED = 4,
	UNI_CMD_MEC_CTRL_ACTION_AMSDU_MAX_LEN = 5,
	UNI_CMD_MEC_MAX_NUM
};

struct GNU_PACKED UNI_CMD_MEC_READ_INFO_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2ReadType;
	UINT8 u1Reserved[2];
};

struct GNU_PACKED UNI_CMD_MEC_ALGO_EN_STA_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2WlanIdx;
	UINT8 u1AmsduAlgoEn;
	UINT8 u1Reserved[1];
};

struct GNU_PACKED UNI_CMD_MEC_AMSDU_PARA_STA_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2WlanIdx;
	UINT8  u1AmsduEn;
	UINT8  u1AmsduNum;
	UINT16 u2AmsduLen;
	UINT8  au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_MEC_AMSDU_ALGO_THR_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 u1BaNum;
	UINT8 u1AmsduNum;
	UINT16 u2AmsduRateThr;
	UINT8 u1Reserved[2];
};

struct GNU_PACKED UNI_CMD_MEC_IFAC_SPEED_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT32 u4IfacSpeed;
};

struct GNU_PACKED UNI_CMD_MEC_SET_AMSDU_MAX_SIZE_STA_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2WlanIdx;
	UINT8 u1Reserved[2];
};

struct GNU_PACKED UNI_CMD_MEC_CTRL_PARAM_T {
	BOOLEAN MecTagValid[UNI_CMD_MEC_MAX_NUM];

	struct UNI_CMD_MEC_READ_INFO_T mec_read_info_t;
	struct UNI_CMD_MEC_ALGO_EN_STA_T mec_algo_en_sta_t;
	struct UNI_CMD_MEC_AMSDU_PARA_STA_T mec_amsdu_para_sta_t;
	struct UNI_CMD_MEC_AMSDU_ALGO_THR_T mec_amsdu_algo_thr;
	struct UNI_CMD_MEC_IFAC_SPEED_T mec_ifac_speed;
	struct UNI_CMD_MEC_SET_AMSDU_MAX_SIZE_STA_T mec_set_amsdu_max_size_t;
};

/* =================== UNI_CMD_ID_MEC End =================== */

/* ---------------- UNI_CMD_ID_FR_TABLE Start --------------- */
/** @addtogroup UNI_CMD_ID_FR_TABLE
 * @{
 *  @page page_FR
 *     <br/>
 *     This command is used to set FR Table by index.
 */
/**
 * This structure is used for UNI_CMD_ID_FR_TABLE command (0x40) to set FR Table by index
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] aucTlvBuffer     TLVs
 *
 */
struct GNU_PACKED UNI_CMD_FR_TABLE_T {
	/*fixed field*/
	UINT_8 aucReserved[4];
	/* tlv */
	UINT_8 aucTlvBuffer[0]; /**< the TLVs included in this field:
	*
	*   TAG                   | ID   | structure
	*   -------------------   | ---- | -------------
	*   UNI_CMD_FR_TBL_SET    | 0x0  | UNI_CMD_FR_TBL_SET_T
	*/
};
/** @} */
enum UNI_CMD_ID_FR_TABLE_TAG_T {
	UNI_CMD_FR_TABLE_SET = 0x0,
	UNI_CMD_FR_TABLE_MAX_NUM
};

#define MAX_FR_TABLE_COUNT 20
#define FR_TABLE_START_IDX 25

/* For Fixed Rate CCK/OFDM, Please Refer TXD_V2_FR_TBL */
enum UNI_CMD_ID_FR_TABLE_TAG_IDX {
	UNI_CMD_FR_TABLE_BAND0_IDX = 25, /* For band0 beacon dup tx */
	UNI_CMD_FR_TABLE_BAND1_IDX = 27, /* For band1 beacon dup tx */
	UNI_CMD_FR_TABLE_BAND2_IDX = 29, /* For band2 beacon dup tx */

	UNI_CMD_FR_TABLE_CCK_11M_IDX = 31,

	/* For CCK/OFDM SPE_ON FR Tab */
	/* FR_CCK_SPE0x18_1M = 7 */
	UNI_CMD_FR_TABLE_CCK_SPE_2M_IDX = 33,
	UNI_CMD_FR_TABLE_CCK_SPE_5_5M_IDX = 35,
	UNI_CMD_FR_TABLE_CCK_SPE_11M_IDX = 37,
	UNI_CMD_FR_TABLE_CCKS_SPE_2M_IDX = 39,
	UNI_CMD_FR_TABLE_CCKS_SPE_5_5M_IDX = 41,
	UNI_CMD_FR_TABLE_CCKS_SPE_11M_IDX = 43,
	/* FR_OFDM_SPE0x18_6M = 8 */
	UNI_CMD_FR_TABLE_OFDM_SPE_9M_IDX = 45,
	UNI_CMD_FR_TABLE_OFDM_SPE_12M_IDX = 47,
	UNI_CMD_FR_TABLE_OFDM_SPE_18M_IDX = 49,
	UNI_CMD_FR_TABLE_OFDM_SPE_24M_IDX = 51,
	UNI_CMD_FR_TABLE_OFDM_SPE_36M_IDX = 53,
	UNI_CMD_FR_TABLE_OFDM_SPE_48_5M_IDX = 55,
	UNI_CMD_FR_TABLE_OFDM_SPE_54M_IDX = 57,

	/* FR_VHT 1SS MCS7 */
	UNI_CMD_FR_TABLE_VHT_1SS_MCS7_IDX = 59,
	UNI_CMD_FR_TABLE_HE_SPE_MCS0_IDX = 61,
	UNI_CMD_FR_TABLE_MAX_IDX,/* Total 0-63 FR Tab */
};

/** @addtogroup UNI_CMD_ID_FR_TABLE
 *  @{
 */
/** This structure is used for UNI_CMD_FR_TABLE_SET tag(0x0) of UNI_CMD_ID_FR_TABLE command (0x40)
 * to set FR Table by index
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag        should be 0x0
 * @param[in] u2Length     the length of this TLV, should be sizeof(UNI_CMD_FR_TABLE_SET_T)
 * @param[in] ucTabIdx
 * @param[in] ucFrAntIdx
 * @param[in] u2FixedRate
 * @param[in] ucFrSpeIdxSel
 * @param[in] ucFrSpeIdx
 * @param[in] ucFrGI
 * @param[in] ucFrHeLtf
 * @param[in] ucFrBw
 * @param[in] fgFrLDPC
 * @param[in] fgFrTBF
 * @param[in] fgFrDynBw
 * @param[in] u1Reserved
 */
struct GNU_PACKED UNI_CMD_FR_TABLE_SET_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	/* tag specific part */

	UINT_8                 ucTabIdx;
	UINT_8                 ucFrAntIdx;
	UINT_16                u2FixedRate;

	UINT_8                 ucFrSpeIdxSel;
	UINT_8                 ucFrSpeIdx;
	UINT_8                 ucFrGI;
	UINT_8                 ucFrHeLtf;

	BOOLEAN                fgFrLDPC;
	BOOLEAN                fgFrTBF;
	BOOLEAN                fgFrDynBw;
	UINT_8                 au1Reserved[1];
};
/* ------------------- UNI_CMD_ID_FR_TABLE End ------------------- */

/* ============== UNI_CMD_ID_TESTMODE_CTRL Start ============== */
struct _UNI_CMD_TESTMODE_CTRL_T {

    /*fix field*/
	UINT_8 au1Reserved[4];

	/*tlv */
	UINT_8 aucTlvBuffer[0];/**< the TLVs included in this field:
				*
				*   TAG                            | ID  | structure
				*   -------------------------------|-----|--------------
				*   UNI_CMD_RF_TEST_CTRL           | 0x0 | UNI_CMD_RF_TEST_CTRL_T
				*/
};

enum _UNI_CMD_TESTMODE_CTRL_TAG_T {
	UNI_CMD_RF_TEST_CTRL = 0,
	UNI_CMD_RF_TEST_NUM
};

#define UNI_CMD_RF_TEST_CTRL_FEATURE (1 << UNI_CMD_RF_TEST_CTRL)

struct _UNI_RSP_T {
	uint16_t u2RspSize;
	u_char *pcRspData;
};

struct GNU_PACKED _UNI_CMD_TEST_CTRL_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;

	uint8_t ucAction;
	uint8_t ucIcapLen;
	uint8_t ucIcapAttr;
	uint8_t aucReserved[1];
	union {
		UINT32 u4OpMode;
		UINT32 u4ChannelFreq;
		struct _PARAM_MTK_WIFI_TEST_STRUC_T rRfATInfo;
	} u;
};

typedef INT32 (*PFN_TESTMODE_RF_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_TEST_CTRL_T *p_TestRxCtrl,
	VOID *pHandle);

void UniEventTestEngineHandleEvent(struct cmd_msg *msg, char *payload, uint16_t payload_len);

/* ============== UNI_CMD_ID_TESTMODE_CTRL End ============== */

/* ============== UNI_CMD_ID_HANDLE_PRECAL_RESULT Start ============== */
struct _UNI_CMD_ID_PRE_CAL_RESULT_CONFIG_T {
	/* fixed field */
	UINT_8 ucAction;    // 0:Load , 1:get
	UINT_8 ucDest;      // ucDataToFromFlash
	UINT_8 ucAttribute; // bit0 : valid
	UINT_8 ucTagNum;

	/* tlv */
	UINT_8 aucTlvBuffer[0];
};

struct GNU_PACKED _UNI_CALRES_CMD_HANDLE_TAG_T {
	UINT_16 u2Tag;           //Tag
	UINT_16 u2Length;        //Length
	UINT_32 u4CalId;         //Value, New Add
	INT_8   i8CalTemp;       // cPreCalTemp
	UINT_8  u1ChBand;        //u1Band
	UINT_8  u1Reserved[2];   //For 4 bytes alignment
	UINT_32 ucCalResOffset;  // ucIndex
	UINT_32 u4CalResLen;     // u4Length
	UINT_32 au4CalRes[0];    //au4Data
};

enum _CMD_PRECAL_CMD_CATEGORY {
	PRECAL_CMD_PRE_CAL_RESULT = 0x0,
	PRECAL_CMD_NUM
};

#define CAL_ID_TX_DPD_FLATNESS_PRE_CAL  0x10000000
#define CAL_ID_GROUP_PRE_CAL            0x20000000

/* ============== UNI_CMD_ID_HANDLE_PRECAL_RESULT End ============== */

/* =================== UNI_CMD_ID_CSI Begin =================== */
/* CSI command (0x48) */
struct GNU_PACKED UNI_CMD_CSI_T {
	UINT8 ucBandIdx;
	UINT8 ucReserved[3];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

/* CSI config Tag */
enum UNI_CMD_CSI_TAG_T {
	UNI_CMD_CSI_STOP = 0,
	UNI_CMD_CSI_START = 1,
	UNI_CMD_CSI_SET_FRAME_TYPE = 2,
	UNI_CMD_CSI_SET_CHAIN_NUMBER = 3,
	UNI_CMD_CSI_SET_FILTER_MODE = 4,
	UNI_CMD_CSI_SET_ACTIVE_MODE = 5,
};

/* CSI Setting (Tag0) */
struct GNU_PACKED UNI_CMD_CSI_STOP_T {
	UINT16 u2Tag;
	UINT16 u2Length;
};

struct GNU_PACKED UNI_CMD_CSI_START_T {
	UINT16 u2Tag;
	UINT16 u2Length;
};

struct GNU_PACKED UNI_CMD_CSI_SET_FRAME_TYPE_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucFrameTypeIndex;
	UINT8 ucFrameType;
	UINT8 aucPadding[2];
};

struct GNU_PACKED UNI_CMD_CSI_SET_CHAIN_NUMBER_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 function;
	UINT8 chain_value;
	UINT8 aucPadding[2];
};

struct GNU_PACKED UNI_CMD_CSI_SET_FILTER_MODE_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucOperation;
	UINT8 aucPadding[1];
	UINT8 aucMACAddr[6];
};

struct GNU_PACKED UNI_CMD_CSI_SET_ACTIVE_MODE_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 interval; /*uint: ms*/
	UINT8 frame_type_idx;
	UINT8 subframe_type_idx;
	UINT32 bitmap; /*sta wcid bitmap*/
	UINT8 aucPadding[4];
};

/* =================== UNI_CMD_ID_CSI End =================== */

/* =================== UNI_CMD_ID_VLAN_CFG Begin =================== */
struct GNU_PACKED UNI_CMD_VLAN_CFG_T {
	/*fixed field*/
	UINT8 aucResrved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

/* VLAN_CFG command Tag */
enum UNI_CMD_VLAN_CFG_TAG_T {
	UNI_CMD_VLAN_TCI_INFO = 0,
	UNI_CMD_VLAN_MAX_NUM
};

/* VLAN TCI operation code */
enum UNI_CMD_VLAN_TCI_OPCODE_T {
	UNI_CMD_VLAN_OP_VID = BIT(0),    /* TCI [11:0] */
	UNI_CMD_VLAN_OP_CFI = BIT(1),    /* TCI [12] */
	UNI_CMD_VLAN_OP_PCP = BIT(2),    /* TCI [15:13] */
	UNI_CMD_VLAN_OP_MAX_NUM
};

/* VLAN TCI Info (Tag0) */
struct GNU_PACKED UNI_CMD_VLAN_TCI_INFO_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucOmacId;
	UINT8 ucBandId;
	UINT8 ucOpCode;
	UINT8 ucTCI_CFI;
	UINT16 u2TCI_VID;
	UINT8 ucTCI_PCP;
	UINT8 ucReserved;
};
/* =================== UNI_CMD_ID_VLAN_CFG End =================== */


/* ============== UNI_CMD_ID_THERMAL Begin ============== */

/**
 * This structure is used for UNI_CMD_ID_THERMAL command (0x35) for thermal related.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] au1Reserved        Reserved
 * @param[in] au1TlvBuffer        TLVs
 *
 */
struct GNU_PACKED UNI_CMD_THERMAL_T {
	/*fixed field*/
	UINT8 au1Reserved[4];
	/* tlv */
	UINT8 au1TlvBuffer[0];
};

/** thermal command TLV List */
enum UNI_CMD_THERMAL_TAG_T {
	UNI_CMD_THERMAL_FEATURE_TEMPERATURE_QUERY = 0x0,
	UNI_CMD_THERMAL_FEATURE_MANUAL_CTRL = 0x1,
	UNI_CMD_THERMAL_FEATURE_BASIC_INFO_QUERY = 0x2,
	UNI_CMD_THERMAL_FEATURE_TASK_MANUAL_CTRL = 0x3,
	UNI_CMD_THERMAL_PROTECT_PARAMETER_CTRL = 0x4,
	UNI_CMD_THERMAL_PROTECT_BASIC_INFO = 0x5,
	UNI_CMD_THERMAL_PROTECT_ENABLE = 0x6,
	UNI_CMD_THERMAL_PROTECT_DISABLE = 0x7,
	UNI_CMD_THERMAL_PROTECT_DUTY_CONFIG = 0x8,
	UNI_CMD_THERMAL_PROTECT_MECH_INFO = 0x9,
	UNI_CMD_THERMAL_PROTECT_DUTY_INFO = 0xA,
	UNI_CMD_THERMAL_PROTECT_STATE_ACT = 0xB,
	UNI_CMD_THERMAL_FEATURE_TEMP_ADC_QUERY = 0xD,
	UNI_CMD_THERMAL_NUM
};

struct GNU_PACKED UNI_CMD_THERMAL_SENSOR_INFO_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8     u1ThermalCtrlFormatId;
	UINT8     u1ActionIdx;    /* 0: get temperature, 1: get thermal sensor ADC */
	UINT8     u1BandIdx;
	UINT8     u1Reserved;
};

struct GNU_PACKED UNI_CMD_THERMAL_TEMP_ADC_INFO_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8	 u1ThermalCtrlFormatId;
	UINT8     u1SensorType;
	UINT8     u1BandIndex;
	UINT8     u1Reserved[1];
};

struct GNU_PACKED UNI_CMD_THERMAL_SET_MAN_CTRL_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8     u1ThermalCtrlFormatId;
	UINT8     u1BandIdx;
	UINT8     fgManualMode;    /* 1/0  enable/disable thermal temperature manual ctrl */
	UINT8     u1ThermalAdc;    /* thermal adc value */
};

struct GNU_PACKED UNI_CMD_THERMAL_BASIC_INFO_T {
	/* DW_0 */
	UINT16 u2Tag;
	UINT16 u2Length;
	/* DW_1*/
	UINT8  u1ThermalCtrlFormatId;
	UINT8  u1BandIdx;
	UINT8  u1Reserved[2];
};

struct GNU_PACKED UNI_CMD_THERMAL_SENSOR_TASK_T {
	/* DW_0 */
	UINT16 u2Tag;
	UINT16 u2Length;
	/* DW_1*/
	UINT8  u1ThermalCtrlFormatId;
	UINT8  u1BandIdx;
	UINT8  u1Thres;
	UINT8  fgTrigEn;
	UINT32 u4FuncPtr;
};

struct GNU_PACKED UNI_CMD_THERMAL_PROTECT_T {
	/* DW_0 */
	UINT16 u2Tag;
	UINT16 u2Length;
	/* DW_1*/
	UINT8  u1ThermalCtrlFormatId;
	UINT8  u1BandIdx;
	UINT8  u1HighEnable;           /* 0: Disable High temperature event to driver 1: Enable */
	INT8   i1HighTempThreshold;    /* if (current temperature >= cHighTempThreshold)  */
	UINT8  u1LowEnable;            /* 0: Disable Low temperature event to driver 1: Enable */
	INT8   i1LowTempThreshold;     /* if (current temperature <= cLowTempThreshold)  */
	UINT32 u4RecheckTimer;          /* Set recheck timer in Sec */
	UINT8  u1RFOffEnable;          /* 0: Disable RFOff event to driver 1: Enable */
	CHAR  i1RFOffThreshold;         /* if (current temperature >= cRFOffThreshold); */
	UINT8  u1Type;                 /* 0: Limit Tx Stream  1: admission control */
	UINT8  u1ExtraTag;
	UINT8  u1Lv0Duty;
	UINT8  u1Lv1Duty;
	UINT8  u1Lv2Duty;
	UINT8  u1Lv3Duty;
};

struct GNU_PACKED UNI_CMD_THERMAL_PROTECT_ENABLE_T {
	/* DW_0 */
	UINT16  u2Tag;
	UINT16  u2Length;
	/* DW_1*/
	UINT8   u1SubCmdId;
	UINT8   u1BandIdx;
	UINT8   u1ProtectionType;
	UINT8   u1TriggerType;
	INT32   i4TriggerTemp;
	INT32   i4RestoreTemp;
	UINT16  u2RecheckTime;
	UINT8   u1Reserved[2];
};

struct GNU_PACKED UNI_CMD_THERMAL_PROTECT_DISABLE_T {
	/* DW_0 */
	UINT16  u2Tag;
	UINT16  u2Length;
	/* DW_1*/
	UINT8   u1SubCmdId;
	UINT8   u1BandIdx;
	UINT8   u1ProtectionType;
	UINT8   u1TriggerType;
};

struct GNU_PACKED UNI_CMD_THERMAL_PROTECT_DUTY_CFG_T {
	/* DW_0 */
	UINT16  u2Tag;
	UINT16  u2Length;
	/* DW_1*/
	UINT8   u1SubCmdId;
	UINT8   u1BandIdx;
	UINT8   u1LevelIndex;
	UINT8   u1Duty;
};

struct GNU_PACKED UNI_CMD_THERMAL_PROTECT_INFO_T {
	/* DW_0 */
	UINT16  u2Tag;
	UINT16  u2Length;
	/* DW_1*/
	UINT8  u1SubCmdId;
	UINT8  u1BandIdx;
	UINT8  au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_THERMAL_PROTECT_DUTY_INFO_T {
	/* DW_0 */
	UINT16  u2Tag;
	UINT16  u2Length;
	/* DW_1*/
	UINT8   u1SubCmdId;
	UINT8   u1BandIdx;
	UINT8   au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_THERMAL_PROTECT_STATE_ACT_T {
	/* DW_0 */
	UINT16 u2Tag;
	UINT16 u2Length;
	/* DW_1*/
	UINT8  u1SubCmdId;
	UINT8  u1BandIdx;
	UINT8  u1ProtectType;
	UINT8  u1TrigType;
	UINT8  u1State;
	UINT8  au1Reserved[3];
};

/* ============== UNI_CMD_ID_THERMAL End ============== */

/* ============== UNI_CMD_ID_NOISE_FLOOR Begin ============== */

/**
 * This structure is used for UNI_CMD_ID_NOISE_FLOOR command (0x36) for Noise Floor related.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] au1Reserved        Reserved
 * @param[in] au1TlvBuffer        TLVs
 *
 */
struct GNU_PACKED UNI_CMD_NOISE_FLOOR_T {
	/*fixed field*/
	UINT8  au1Reserved[4];
	/* tlv */
	UINT8  au1TlvBuffer[0];
};

struct GNU_PACKED UNI_CMD_NOISE_FLOOR_MODE_CTRL_T {
	/* DW_0 */
	UINT16  u2Tag;
	UINT16  u2Length;
	/* DW_1*/
	UINT8   u1Mode;
	UINT8   u1Action;
	UINT8   au1Reserved[2];
};

struct GNU_PACKED UNI_CMD_NF_INFO_T {
	UINT16   u2Tag;
	UINT16   u2Length;
	BOOLEAN   fgEnable;
	UINT8    u1TimeOut;
	UINT8    u1Count;
	UINT8    u1EventCount;
	UINT8    aucPadding[2];
};

enum UNI_CMD_NOISE_FLOOR_MODE_TAG_T {
	UNI_CMD_NOISE_FLOOR_MODE_RECAL = 0x1,
	UNI_CMD_NOISE_FLOOR_MODE_DYNAMIC_G0 = 0x2,
	UNI_CMD_NOISE_FLOOR_MODE_APPLY_HIGH_DPD = 0x3,
	UNI_CMD_NOISE_FLOOR_MODE_NTLT = 0x4,
	UNI_CMD_NOISE_FLOOR_MODE_NUM
};

enum UNI_CMD_NOISE_FLOOR_ACTION_TAG_T {
	UNI_CMD_NOISE_FLOOR_ACTION_DISABLE = 0x0,
	UNI_CMD_NOISE_FLOOR_ACTION_ENABLE = 0x1,
	UNI_CMD_NOISE_FLOOR_ACTION_TRIGGER = 0x2,
	UNI_CMD_NOISE_FLOOR_ACTION_NT2LT_TRIGGER = 0x2,
	UNI_CMD_NOISE_FLOOR_ACTION_LT2NT_TRIGGER = 0x3,
	UNI_CMD_NOISE_FLOOR_ACTION_NUM
};

/* ============== UNI_CMD_ID_NOISE_FLOOR End ============== */

/* ============== UNI_CMD_ID_CAL Begin ============== */

/**
 * This structure is used for UNI_CMD_ID_CAL command (0x4C) for CAL related.
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] au1Reserved        Reserved
 * @param[in] au1TlvBuffer        TLVs
 *
 */
struct GNU_PACKED UNI_CMD_CAL_T {
	/*fixed field*/
	UINT8  au1Reserved[4];
	/* tlv */
	UINT8  au1TlvBuffer[0];
};

struct GNU_PACKED UNI_CMD_CAL_MODE_CTRL_T {
	/* DW_0 */
	UINT16  u2Tag;
	UINT16  u2Length;
	/* DW_1*/
	UINT8   u1Mode;
	UINT8   u1Action;
	UINT8   au1Reserved[2];
};

enum UNI_CMD_CAL_MODE_TAG_T {
	UNI_CMD_CAL_MODE_RECAL = 0x1,
	UNI_CMD_CAL_MODE_DYNAMIC_G0 = 0x2,
	UNI_CMD_CAL_MODE_APPLY_HIGH_DPD = 0x3,
	UNI_CMD_CAL_MODE_NTLT = 0x4,
	UNI_CMD_CAL_MODE_NUM
};

/* ============== UNI_CMD_ID_CAL End ============== */

/* =================== UNI_CMD_ID_VOW Begin ================== */
/* VOW command (0x37) */
typedef struct _UNI_CMD_VOW_T {
	/* fixed field */
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0]; /**<the TLVs includer in this field:
		*
		*  TAG                                       | ID   | structure
		*  ------------------------------------      | -----| -------------
		*  UNI_CMD_VOW_DRR_CTRL                      |  0x0  | UNI_CMD_VOW_DRR_CTRL_T
		*  UNI_CMD_VOW_FEATURE_CTRL                  |  0x1  | UNI_CMD_VOW_FEATURE_CTRL_T
		*  UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP         |  0x2  | UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T
		*  UNI_CMD_VOW_BSSGROUP_TOKEN_CFG            |  0x3  | UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T
		*  UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP       |  0x4  | UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T
		*  UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM     |  0x5  | UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T
		*  UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL |  0x6  | UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T
		*  UNI_CMD_VOW_AT_PROC_EST_FEATURE           |  0x7  | UNI_CMD_VOW_AT_PROC_EST_FEATURE_T
		*  UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD    |  0x8  | UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T
		*  UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO       |  0x9  | UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T
		*  UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING  | 0xA  | UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T
		*  UNI_CMD_VOW_RX_AT_AIRTIME_EN              |  0xB  | UNI_CMD_VOW_RX_AT_AIRTIME_EN_T
		*  UNI_CMD_VOW_RX_AT_MIBTIME_EN              |  0xC  | UNI_CMD_VOW_RX_AT_MIBTIME_EN_T
		*  UNI_CMD_VOW_RX_AT_EARLYEND_EN             |  0xD  | UNI_CMD_VOW_RX_AT_EARLYEND_EN_T
		*  UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN          |  0xE  | UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T
		*  UNI_CMD_VOW_RX_AT_STA_WMM_CTRL            |  0xF  | UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T
		*  UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL           | 0x10  | UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T
		*  UNI_CMD_VOW_RX_AT_ED_OFFSET               | 0x11  | UNI_CMD_VOW_RX_AT_ED_OFFSET_T
		*  UNI_CMD_VOW_RX_AT_SW_TIMER                | 0x12  | UNI_CMD_VOW_RX_AT_SW_TIMER_T
		*  UNI_CMD_VOW_RX_AT_BACKOFF_TIMER           | 0x13  | UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T
		*  UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME  | 0x14  | CMD_TLV_GENERAL_T
		*  UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME     | 0x15  | CMD_TLV_GENERAL_T
		*  UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME    | 0x16  | CMD_TLV_GENERAL_T
		*  UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME  | 0x17  | CMD_TLV_GENERAL_T
		*  UNI_CMD_VOW_RED_ENABLE                    | 0x18  | UNI_CMD_VOW_RED_ENABLE_T
		*  UNI_CMD_VOW_RED_TX_RPT                    | 0x19  | UNI_CMD_VOW_RED_TX_RPT_T
		*/
} UNI_CMD_VOW_T, *P_UNI_CMD_VOW_T;

typedef enum _UNI_CMD_VOW_TAG_T {
	UNI_CMD_VOW_DRR_CTRL = 0x00,
	UNI_CMD_VOW_FEATURE_CTRL = 0x01,
	UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP = 0x02,
	UNI_CMD_VOW_BSSGROUP_TOKEN_CFG = 0x03,
	UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP = 0x04,
	UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM = 0x05,
	UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL = 0x06,
	UNI_CMD_VOW_AT_PROC_EST_FEATURE = 0x07,
	UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD = 0x08,
	UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO = 0x09,
	UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING = 0x0A,
	UNI_CMD_VOW_RX_AT_AIRTIME_EN = 0x0B,
	UNI_CMD_VOW_RX_AT_MIBTIME_EN = 0x0C,
	UNI_CMD_VOW_RX_AT_EARLYEND_EN = 0x0D,
	UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN = 0x0E,
	UNI_CMD_VOW_RX_AT_STA_WMM_CTRL = 0x0F,
	UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL = 0x10,
	UNI_CMD_VOW_RX_AT_ED_OFFSET = 0x11,
	UNI_CMD_VOW_RX_AT_SW_TIMER = 0x12,
	UNI_CMD_VOW_RX_AT_BACKOFF_TIMER = 0x13,
	UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME = 0x14,
	UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME = 0x15,
	UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME = 0x16,
	UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME = 0x17,
	UNI_CMD_VOW_RED_ENABLE = 0x18,
	UNI_CMD_VOW_RED_TX_RPT = 0x19,
	UNI_CMD_VOW_MAX_NUM
} UNI_CMD_VOW_TAG_T;

/* DRR ctrl (Tag 0x00) */
typedef struct _UNI_AIRTIME_QUA_ALL_FIELD_T {
	UINT8     aucAirTimeQuantum[UNICMD_VOW_AIRTIME_QUANTUM_IDX_TOTAL_NUM];
} UNI_AIRTIME_QUA_ALL_FIELD_T, *P_UNI_AIRTIME_QUA_ALL_FIELD_T;

typedef union _UNI_VOW_DRR_CTRL_VALUE_T {
	UINT32                     u4ComValue;
	UNI_AIRTIME_QUA_ALL_FIELD_T rAirTimeQuantumAllField;
} UNI_VOW_DRR_CTRL_VALUE_T, *P_UNI_VOW_DRR_CTRL_VALUE_T;

typedef struct _UNI_CMD_VOW_DRR_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2StaID;

	UINT_8 u1BandIdx;
	UINT_8 u1WmmSet;

	UINT32 u4CtrlFieldID;

	UNI_VOW_DRR_CTRL_VALUE_T rAirTimeCtrlValue;

	UINT_8 u1Reserved[3];
	UINT_8 u1OMACIdx;

} UNI_CMD_VOW_DRR_CTRL_T, *P_UNI_CMD_VOW_DRR_CTRL_T;

/* VOW feature ctrl (Tag 0x01) */
typedef struct _UNI_CMD_VOW_FEATURE_CTRL_T {
	UINT16         u2Tag;
	UINT16         u2Length;

	/* DW#0 */
	UINT16 u2IfApplyBss_0_to_16_CtrlFlag;          /* BITWISE */

#ifdef CFG_BIG_ENDIAN
	UINT16 u2IfApplyEnbwCtrlFlag: 1;
	UINT16 u2IfApplyEnbwrefillFlag: 1;
	UINT16 u2IfApplyAirTimeFairnessFlag: 1;
	UINT16 u2IfApplyEnTxopNoChangeBssFlag: 1;
	UINT16 u2Reserve_b26_to_b27Flag: 2;
	UINT16 u2IfApplyWeightedAirTimeFairnessFlag: 1;
	UINT16 u2Reserve_b22_to_b24Flag: 3;
	UINT16 u2IfApplyDbdc0SearchRuleFlag: 1;
	UINT16 u2IfApplyDbdc1SearchRuleFlag: 1;
	UINT16 u2Reserve_b19Flag: 1;
	UINT16 u2Reserve_b17_to_b18Flag: 2;
	UINT16 u2IfApplyRefillPerildFlag: 1;
#else /* CFG_BIG_ENDIAN */
	UINT16 u2IfApplyRefillPerildFlag: 1;
	UINT16 u2Reserve_b17_to_b18Flag: 2;
	UINT16 u2Reserve_b19Flag: 1;
	UINT16 u2IfApplyDbdc1SearchRuleFlag: 1;
	UINT16 u2IfApplyDbdc0SearchRuleFlag: 1;
	UINT16 u2Reserve_b22_to_b24Flag: 3;
	UINT16 u2IfApplyWeightedAirTimeFairnessFlag: 1;
	UINT16 u2Reserve_b26_to_b27Flag: 2;
	UINT16 u2IfApplyEnTxopNoChangeBssFlag: 1;
	UINT16 u2IfApplyAirTimeFairnessFlag: 1;
	UINT16 u2IfApplyEnbwrefillFlag: 1;
	UINT16 u2IfApplyEnbwCtrlFlag: 1;
#endif /* CFG_BIG_ENDIAN */

	/* DW#1 */
	UINT16 u2IfApplyBssCheckTimeToken_0_to_16_CtrlFlag;    /* BITWISE */
	UINT16 u2Resreve1Flag;

	/* DW#2 */
	UINT16 u2IfApplyBssCheckLengthToken_0_to_16_CtrlFlag;  /* BITWISE */
	UINT16 u2Resreve2Flag;

	/* DW#3, 4 */
	UINT8  u1BandIdx;
	UINT8  u1ReservBackpFlag[3];
	UINT32 u4ReservBackpFlag[1];

	/***********************************************************************/

	/* DW#5 */
	UINT16 u2Bss_0_to_16_CtrlValue;          /* BITWISE */

#ifdef CFG_BIG_ENDIAN
	UINT16 u2EnbwCtrlValue: 1;
	UINT16 u2EnbwrefillValue: 1;
	UINT16 u2AirTimeFairnessValue: 1;
	UINT16 u2EnTxopNoChangeBssValue: 1;
	UINT16 u2Reserve_b26_to_b27Value: 2;
	UINT16 u2WeightedAirTimeFairnessValue: 1;
	UINT16 u2Reserve_b22_to_b24Value: 3;
	UINT16 u2Dbdc0SearchRuleValue: 1;
	UINT16 u2Dbdc1SearchRuleValue: 1;
	UINT16 u2Reserve_b19Value: 1;
	UINT16 u2RefillPerildValue: 3;
#else /* CFG_BIG_ENDIAN */
	UINT16 u2RefillPerildValue: 3;
	UINT16 u2Reserve_b19Value: 1;
	UINT16 u2Dbdc1SearchRuleValue: 1;
	UINT16 u2Dbdc0SearchRuleValue: 1;
	UINT16 u2Reserve_b22_to_b24Value: 3;
	UINT16 u2WeightedAirTimeFairnessValue: 1;
	UINT16 u2Reserve_b26_to_b27Value: 2;
	UINT16 u2EnTxopNoChangeBssValue: 1;
	UINT16 u2AirTimeFairnessValue: 1;
	UINT16 u2EnbwrefillValue: 1;
	UINT16 u2EnbwCtrlValue: 1;
#endif /* CFG_BIG_ENDIAN */

	/* DW#6 */
	UINT16 u2BssCheckTimeToken_0_to_16_CtrlValue;    /* BITWISE */
	UINT16 u2Resreve1Value;

	/* DW#7 */
	UINT16 u2BssCheckLengthToken_0_to_16_CtrlValue;  /* BITWISE */
	UINT16 u2Resreve2Value;

	/* DW#8 */
#ifdef CFG_BIG_ENDIAN
	UINT32 u4Resreve1Value: 1;
	UINT32 u4VowKeepSettingBit: 5;
	UINT32 u4VowKeepSettingValue: 1;
	UINT32 u4IfApplyKeepVoWSettingForSerFlag: 1;
	UINT32 u4RxRifsModeforCckCtsValue: 1;
	UINT32 u4IfApplyRxRifsModeforCckCtsFlag: 1;
	UINT32 u4ApplyRxEifsToZeroValue: 1;
	UINT32 u4IfApplyRxEifsToZeroFlag: 1;
	UINT32 u4RtsFailedChargeDisValue: 1;
	UINT32 u4IfApplyRtsFailedChargeDisFlag: 1; /* don't charge airtime when RTS failed */
	UINT32 u4TxBackOffBoundValue: 5; /* ms */
	UINT32 u4TxBackOffBoundEnable: 1;
	UINT32 u4IfApplyTxBackOffBoundFlag: 1;
	UINT32 u4TxMeasurementModeValue: 1;
	UINT32 u4IfApplyTxMeasurementModeFlag: 1;
	UINT32 u4TxCountValue: 4;
	UINT32 u4IfApplyTxCountModeFlag: 1;
	UINT32 u4KeepQuantumValue: 1;
	UINT32 u4IfApplyKeepQuantumFlag: 1;
	UINT32 u4RtsStaLockValue: 1;
	UINT32 u4IfApplyStaLockForRtsFlag: 1;
#else /* CFG_BIG_ENDIAN */
	UINT32 u4IfApplyStaLockForRtsFlag: 1;
	UINT32 u4RtsStaLockValue: 1;
	UINT32 u4IfApplyKeepQuantumFlag: 1;
	UINT32 u4KeepQuantumValue: 1;
	UINT32 u4IfApplyTxCountModeFlag: 1;
	UINT32 u4TxCountValue: 4;
	UINT32 u4IfApplyTxMeasurementModeFlag: 1;
	UINT32 u4TxMeasurementModeValue: 1;
	UINT32 u4IfApplyTxBackOffBoundFlag: 1;
	UINT32 u4TxBackOffBoundEnable: 1;
	UINT32 u4TxBackOffBoundValue: 5; /* ms */
	UINT32 u4IfApplyRtsFailedChargeDisFlag: 1; /* don't charge airtime when RTS failed */
	UINT32 u4RtsFailedChargeDisValue: 1;
	UINT32 u4IfApplyRxEifsToZeroFlag: 1;
	UINT32 u4ApplyRxEifsToZeroValue: 1;
	UINT32 u4IfApplyRxRifsModeforCckCtsFlag: 1;
	UINT32 u4RxRifsModeforCckCtsValue: 1;
	UINT32 u4IfApplyKeepVoWSettingForSerFlag: 1;
	UINT32 u4VowKeepSettingValue: 1;
	UINT32 u4VowKeepSettingBit: 5;
	UINT32 u4Resreve1Value: 1;
#endif /* CFG_BIG_ENDIAN */

	/* DW#9 */
#ifdef CFG_BIG_ENDIAN
	UINT32 u4ResreveBackupValue: 21;
	UINT32 u4VowSchedulePolicy: 2;
	UINT32 u4VowScheduleType: 2;
	UINT32 u4IfApplyVowSchCtrl: 1;
	UINT32 u4DbgPrnLvl: 2;
	UINT32 u4SplStaNumValue: 3;
	UINT32 u4IfApplySplFlag: 1;
#else
	UINT32 u4IfApplySplFlag: 1;
	UINT32 u4SplStaNumValue: 3;
	UINT32 u4DbgPrnLvl: 2;
	UINT32 u4IfApplyVowSchCtrl: 1;
	UINT32 u4VowScheduleType: 2;
	UINT32 u4VowSchedulePolicy: 2;
	UINT32 u4ResreveBackupValue: 21;
#endif
} UNI_CMD_VOW_FEATURE_CTRL_T, *P_UNI_CMD_VOW_FEATURE_CTRL_T;

typedef struct _UNI_CMD_BW_BSS_TOKEN_SETTING_T {
	/* DW#0 */
	UINT16 u2MinRateToken;				   /* unit: 1 bit */
	UINT16 u2MaxRateToken;				   /* unit: 1 bit */

	/* DW#1 */
#ifdef CFG_BIG_ENDIAN
	UINT32 u4MinTokenBucketLengSize: 12;   /* unit: 1024 bit */
	UINT32 u4D1B19Rev: 1;				   /* reserve */
	UINT32 u4MinAirTimeToken: 11;		   /* unit: 1/8 us */
	UINT32 u4MinTokenBucketTimeSize: 8;    /* unit: 1024 us */
#else /* CFG_BIG_ENDIAN */
	UINT32 u4MinTokenBucketTimeSize: 8;    /* unit: 1024 us */
	UINT32 u4MinAirTimeToken: 11;		   /* unit: 1/8 us */
	UINT32 u4D1B19Rev: 1;				   /* reserve */
	UINT32 u4MinTokenBucketLengSize: 12;   /* unit: 1024 bit */
#endif /* CFG_BIG_ENDIAN */

	/* DW#2 */
#ifdef CFG_BIG_ENDIAN
	UINT32 u4MaxTokenBucketLengSize: 12;   /* unit: 1024 bit */
	UINT32 u4D2B19Rev: 1;				   /* reserve */
	UINT32 u4MaxAirTimeToken: 11;		   /* unit: 1/8 us */
	UINT32 u4MaxTokenBucketTimeSize: 8;    /* unit: 1024 us */
#else /* CFG_BIG_ENDIAN */
	UINT32 u4MaxTokenBucketTimeSize: 8;    /* unit: 1024 us */
	UINT32 u4MaxAirTimeToken: 11;		   /* unit: 1/8 us */
	UINT32 u4D2B19Rev: 1;				   /* reserve */
	UINT32 u4MaxTokenBucketLengSize: 12;   /* unit: 1024 bit */
#endif /* CFG_BIG_ENDIAN */

	/* DW#3 */
#ifdef CFG_BIG_ENDIAN
	UINT32 u4D3B28toB31Rev: 4;			   /* reserve */
	UINT32 u4MaxBacklogSize: 12;		   /* unit: 1024 bit */
	UINT32 u4D3B8toB15Rev: 8;			   /* reserve */
	UINT32 u4MaxWaitTime: 8;			   /* unit: 1024 us */
#else /* CFG_BIG_ENDIAN */
	UINT32 u4MaxWaitTime: 8;			   /* unit: 1024 us */
	UINT32 u4D3B8toB15Rev: 8;			   /* reserve */
	UINT32 u4MaxBacklogSize: 12;		   /* unit: 1024 bit */
	UINT32 u4D3B28toB31Rev: 4;			   /* reserve */
#endif /* CFG_BIG_ENDIAN */
} UNI_CMD_BW_BSS_TOKEN_SETTING_T, *P_UNI_CMD_BW_BSS_TOKEN_SETTING_T;

/* bssgroup ctrl (Tag 0x02) */
typedef struct _UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucBssGroupID;
	UINT8 u1BandIdx;
	UINT8 aucPadding[2];
	UNI_CMD_BW_BSS_TOKEN_SETTING_T  rAllBssGroupMultiField;
} UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T, *P_UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T;

/* bss token cfg (Tag 0x03) */
typedef struct _UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucBssGroupID;
	UINT8 u1BandIdx;
	UINT8 aucPadding[2];
	UINT32 u4SingleFieldIDValue;
	UINT32 u4CfgItemId;
} UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T, *P_UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T;

/* bssgroup ctrl all group (Tag 0x04) */
typedef struct _UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8  u1BandIdx;
	UINT8  aucPadding[3];
	UNI_CMD_BW_BSS_TOKEN_SETTING_T arAllBssGroupMultiField[UNICMD_VOW_BWC_GROUP_NUMBER];
} UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T, *P_UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T;

/* bw group quantum ctrl (Tag 0x05) */
typedef struct _UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucBssGroupQuantumID;
	UINT8 ucBssGroupQuantumTime;
	UINT8 aucPadding[2];
} UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T, *P_UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T;

/* all bw group quantum ctrl (Tag 0x06) */
typedef struct _UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 aucBssGroupQuantumTime[UNICMD_VOW_BW_GROUP_QUANTUM_LEVEL_NUM];
} UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T, *P_UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T;

/* airtime process module estimate ctrl (Tag 0x07) */
typedef struct _UNI_CMD_VOW_AT_PROC_EST_FEATURE_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	BOOLEAN fgAtEstimateOnOff;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_AT_PROC_EST_FEATURE_T, *P_UNI_CMD_VOW_AT_PROC_EST_FEATURE_T;

/* airtime process module estimate monitor period (Tag 0x08) */
typedef struct _UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2AtEstMonitorPeriod;
	UINT8 aucPadding[2];
} UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T, *P_UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T;

/* airtime process module estimate group ratio (Tag 0x09) */
typedef struct _UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT32 u4GroupRatioBitMask;
	UINT16 u2GroupMaxRatioValue[UNICMD_VOW_BWC_GROUP_NUMBER];
	UINT16 u2GroupMinRatioValue[UNICMD_VOW_BWC_GROUP_NUMBER];

	UINT8  u1BandIdx;
	UINT8  aucPadding[3];
} UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T, *P_UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T;

/* airtime process module estimate group to band mapping (Tag 0x0A) */
typedef struct _UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucGrouptoSelectBand;
	UINT8 ucBandSelectedfromGroup;
	UINT8 aucPadding[2];
} UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T, *P_UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T;

/* rx airtime enable (Tag 0x0B) */
typedef struct _UNI_CMD_VOW_RX_AT_AIRTIME_EN_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 fgRxAirTimeEn;
	UINT8 u1BandIdx;
	UINT8 aucPadding[2];
} UNI_CMD_VOW_RX_AT_AIRTIME_EN_T, *P_UNI_CMD_VOW_RX_AT_AIRTIME_EN_T;

/* rx mibtime enable (Tag 0x0C) */
typedef struct _UNI_CMD_VOW_RX_AT_MIBTIME_EN_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 fgRxMibTimeEn;
	UINT8 u1BandIdx;
	UINT8 aucPadding[2];
} UNI_CMD_VOW_RX_AT_MIBTIME_EN_T, *P_UNI_CMD_VOW_RX_AT_MIBTIME_EN_T;

/* rx airtime early end enable (Tag 0x0D) */
typedef struct _UNI_CMD_VOW_RX_AT_EARLYEND_EN_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 fgRxEarlyEndEn;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_RX_AT_EARLYEND_EN_T, *P_UNI_CMD_VOW_RX_AT_EARLYEND_EN_T;

/* rx airtime airtime clr enable (Tag 0x0E) */
typedef struct _UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 fgRxAirTimeClrEn;
	UINT8 u1BandIdx;
	UINT8 aucPadding[2];
} UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T, *P_UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T;

/* rx airtime sta wmm ctrl (Tag 0x0F) */
typedef struct _UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucOwnMacID;
	UINT8 fgtoApplyWm00to03MibCfg;
	UINT8 u1BandIdx;
	UINT8 aucPadding[1];
} UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T, *P_UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T;

/* rx airtime airtime clr enable (Tag 0x10) */
typedef struct _UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucMbssGroup;
	UINT8 ucWmmGroup;
	UINT8 u1BandIdx;
	UINT8 aucPadding[1];
} UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T, *P_UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T;

/* rx airtime ed offset value (Tag 0x11) */
typedef struct _UNI_CMD_VOW_RX_AT_ED_OFFSET_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucEdOffsetValue;
	UINT8 u1BandIdx;
	UINT8 aucPadding[2];
} UNI_CMD_VOW_RX_AT_ED_OFFSET_T, *P_UNI_CMD_VOW_RX_AT_ED_OFFSET_T;

/* rx airtime sw timer value (Tag 0x12) */
typedef struct _UNI_CMD_VOW_RX_AT_SW_TIMER_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucCompensateMode;
	UINT8 ucRxBand;
	UINT8 ucSwCompensateTimeValue;
	UINT8 aucPadding[1];
} UNI_CMD_VOW_RX_AT_SW_TIMER_T, *P_UNI_CMD_VOW_RX_AT_SW_TIMER_T;

/* rx airtime sw timer value (Tag 0x13) */
typedef struct _UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T {
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
} UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T, *P_UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T;

/* rx airtime report non wifi time (Tag 0x14) */
typedef struct _UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucRxNonWiFiBandIdx;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T, *P_UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T;

/* rx airtime report rx obss time (Tag 0x15) */
typedef struct _UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucRxObssBandIdx;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME_T, *P_UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME_T;

/* rx airtime report rx mib time (Tag 0x16) */
typedef struct _UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucRxMibObssBandIdx;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T, *P_UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T;

/* rx airtime report rx mib time (Tag 0x17) */
typedef struct _UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2StaId;
	UINT8 aucPadding[2];
} UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T, *P_UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T;

/* red enable (Tag 0x18) */
typedef struct _UNI_CMD_VOW_RED_ENABLE_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucRedEnable;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_RED_ENABLE_T, *P_UNI_CMD_VOW_RED_ENABLE_T;

typedef struct _UNI_CMD_RED_TX_RPT_T {
	UINT32 u4TCPCnt;
	UINT32 u4TCPAckCnt;
} UNI_CMD_RED_TX_RPT_T, *P_UNI_CMD_RED_TX_RPT_T;

typedef struct GNU_PACKED _UNI_PARSE_EXT_EVENT_RED_TX_RPT_T {
	UINT8 ucfgValid;
	UINT8 wordlen;
	UINT8 Reserve[2];
	UINT32 staInUseBitmap[32];
} UNI_PARSE_EXT_EVENT_RED_TX_RPT_T, *P_UNI_PARSE_EXT_EVENT_RED_TX_RPT_T;

typedef struct GNU_PACKED _UNI_PARSE_RED_TX_RPT_T {
	UINT32 u4TCPCnt;
	UINT32 u4TCPAckCnt;
	UINT16 u2MsduInQueShortTimes;
	UINT16 u2MsduInQueLongTimes;
	UINT8 u1TCPMask;
	UINT8 u1Reserved[3];
} UNI_PARSE_RED_TX_RPT_T, *P_UNI_PARSE_RED_TX_RPT_T;

/* red enable (Tag 0x19) */
typedef struct _UNI_CMD_VOW_RED_TX_RPT_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucWordlen;
	UINT8 aucPadding[3];
	UINT32 u4StaInUseBitmap[32];

	UNI_CMD_RED_TX_RPT_T arTxRpt[0];
} UNI_CMD_VOW_RED_TX_RPT_T, *P_UNI_CMD_VOW_RED_TX_RPT_T;

typedef struct GNU_PACKED _UNI_CMD_VOW_PARAM_T {
	BOOLEAN VOWTagValid[UNI_CMD_VOW_MAX_NUM];

	UNI_CMD_VOW_DRR_CTRL_T VowDrrCtrl; /* TAG 0x00 */
	UNI_CMD_VOW_FEATURE_CTRL_T VowFeatureCtrl; /* TAG 0x01 */
	UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T VowBssgroupCtrl1Group; /* TAG 0x02 */
	UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T VowBssgroupTokenCfg; /* TAG 0x03 */
	UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T VowBssgroupCtrlAllGroup; /* TAG 0x04 */
	UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T VowBssgroupBWGroupQuantum; /* TAG 0x05 */
	UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T VowBssgroupBWGroupQuantumAll; /* TAG 0x06 */
	UNI_CMD_VOW_AT_PROC_EST_FEATURE_T VowATProcEstFeature; /* TAG 0x07 */
	UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T VowATProcEstMonitorPeriod; /* TAG 0x08 */
	UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T VowATProcEstGroupRatio; /* TAG 0x09 */
	UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T VowATProcEstGroupToBandMapping; /* TAG 0x0A */

	UNI_CMD_VOW_RX_AT_AIRTIME_EN_T VowRxAtAirtimeEn; /* TAG 0x0B */
	UNI_CMD_VOW_RX_AT_MIBTIME_EN_T VowRxAtMibtimeEn; /* TAG 0x0C */
	UNI_CMD_VOW_RX_AT_EARLYEND_EN_T VowRxAtEarlyendEn; /* TAG 0x0D */
	UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T VowRxAtAirtimeClrEn; /* TAG 0x0E */
	UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T VowRxAtStaWmmCtrl; /* TAG 0x0F */
	UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T VowRxAtMbssWmmCtrl; /* TAG 0x10 */
	UNI_CMD_VOW_RX_AT_ED_OFFSET_T VowRxAtEdOffset; /* TAG 0x11 */
	UNI_CMD_VOW_RX_AT_SW_TIMER_T VowRxAtSwTimer; /* TAG 0x12 */
	UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T VowRxAtBackoffTimer; /* TAG 0x13 */
	UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T VowRxAtReportRxNonwifiTime; /* TAG 0x14 */
	UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME_T VowRxAtReportRxObssTime; /* TAG 0x15 */
	UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T VowRxAtReportMibObssTime; /* TAG 0x16 */
	UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T VowRxAtReportPerStaRxTime; /* TAG 0x17 */

	UNI_CMD_VOW_RED_ENABLE_T VowRedEnable; /* TAG 0x18 */
	UNI_CMD_VOW_RED_TX_RPT_T VowRedTxRpt; /* TAG 0x19 */
} UNI_CMD_VOW_PARAM_T, *P_UNI_CMD_VOW_PARAM_T;

typedef INT32 (*PFN_VOW_HANDLE)(struct _RTMP_ADAPTER *pAd,
									P_UNI_CMD_VOW_PARAM_T pVowParam,
									VOID *pHandle,
									UINT32 *u4RespStructSize);
/* =================== UNI_CMD_ID_VOW End ==================== */

/* ============== UNI_CMD_ID_LED Begin ============== */
struct GNU_PACKED UNI_CMD_LED_T {
	/* fixed field */
	UINT8 ucLedVer;
	UINT8 ucLedIdx;
	UINT8 ucReversePol;
	UINT8 ucBandSelect;
	UINT8 aucRsvd[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];
};

enum UNI_CMD_ID_LED_TAG_T {
	UNI_CMD_LED_SET_SOLID_ON = 0,
	UNI_CMD_LED_SET_SOLID_OFF = 1,
	UNI_CMD_LED_SET_TX_BLINK = 2,
	UNI_CMD_LED_SET_PWM = 3,
	UNI_CMD_LED_SET_MIX_PWM_BLINK = 4,
	UNI_CMD_LED_SET_GPIO = 5,
	UNI_CMD_ID_LED_MAX_NUM
};

struct GNU_PACKED UNI_CMD_LED_SET_SOLID_ON_T {
	UINT16 u2Tag;
	UINT16 u2Length;
};

struct GNU_PACKED UNI_CMD_LED_SET_SOLID_OFF_T {
	UINT16 u2Tag;
	UINT16 u2Length;
};

struct GNU_PACKED UNI_CMD_LED_SET_TX_BLINK_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucLedCombine;
	UINT8 ucBlinkMode;
	UINT8 ucRsvd1;
	UINT8 ucRsvd2;
	UINT16 u2TxBlinkOnTime;
	UINT16 u2TxBlinkOffTime;
	UINT8 aucRsvd[4];
};

struct GNU_PACKED UNI_CMD_LED_SET_PWM_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucReplayMode;
	UINT8 aucRsvd1[3]; /* reserved */
	UINT32 u4S0TotalTime; /* in ms */
	UINT16 u2S0OnTime; /* in ms */
	UINT16 u2S0OffTime; /* in ms */
	UINT32 u4S1TotalTime; /* in ms */
	UINT16 u2S1OnTime; /* in ms */
	UINT16 u2S1OffTime; /* in ms */
	UINT8 aucRsvd2[4]; /* reserved */
};

struct GNU_PACKED UNI_CMD_LED_SET_MIX_PWM_BLINK_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucLedCombine;
	UINT8 ucBlinkMode;
	UINT8 ucReplayMode;
	UINT8 ucRsvd1;
	UINT32 u4S0TotalTime; /* in ms */
	UINT16 u2S0OnTime; /* in ms */
	UINT16 u2S0OffTime; /* in ms */
	UINT32 u4S1TotalTime; /* in ms */
	UINT16 u2S1OnTime; /* in ms */
	UINT16 u2S1OffTime; /* in ms */
	UINT16 u2TxBlinkOnTime; /* in ms */
	UINT16 u2TxBlinkOffTime; /* in ms */
	UINT8 aucRsvd1[4]; /* reserved */
};

union UNI_LED_SET_CTRL {
	struct UNI_CMD_LED_SET_SOLID_ON_T led_solid_on;
	struct UNI_CMD_LED_SET_SOLID_OFF_T led_solid_off;
	struct UNI_CMD_LED_SET_TX_BLINK_T led_tx_blink;
	struct UNI_CMD_LED_SET_PWM_T led_set_pwm;
	struct UNI_CMD_LED_SET_MIX_PWM_BLINK_T led_set_mix_pwm_blink;
};

struct GNU_PACKED UNI_CMD_LED_SET_GPIO_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 ucGpioIdx;
	UINT8 ucGpioValue;
	UINT8 aucRsvd[2];
};

enum UNI_CMD_LED_GPIO_MODE_TAG_T {
	UNI_CMD_LED_MODE_LED = 0,
	UNI_CMD_LED_MODE_GPIO = 1,
	UNI_CMD_LED_MODE_MAX_NUM
};

/* ============== UNI_CMD_ID_LED End ============== */

/* FAST PATH set command (0x54) */
struct GNU_PACKED UNI_CMD_FAST_PATH_T {
	/* fixed field */
	UINT_8 ucReserved[4];

	/* tlv */
	UINT_8 aucTlvBuffer[0];/**< the TLVs included in this field:
	*
	*   TAG                              | ID  | structure
	*   ---------------------------------|-----|--------------
	*   UNI_CMD_FAST_PATH_PROCESS        | 0x0 | UNI_CMD_FAST_PATH_PROCESS_T
	*/
};
/* Get FAST PATH command TLV List */

enum UNI_CMD_ID_FAST_PATH_TAG_T {
	UNI_CMD_FAST_PATH_PROCESS = 0,
	UNI_CMD_FAST_PATH_MAX_NUM
};

/**
 * This structure is used for UNI_CMD_FAST_PATH_PROCESS(0x00) of
 * UNI_CMD_FAST_PATH_PROCESS command (0x54)
 * to calculate MIC
 * @version Supported from ver:1.0.0.0
 *
 * @param[in] u2Tag                   should be 0x00
 * @param[in] u2Length                the length of this TLV, should be 8
 * @param[in] aucOwnMac               Mac address bring by Driver
 * @param[in] u2RandomNum             Random number genetate by Driver
 * @param[in] u4Keybitmap             Keybitmap send from Driver
 */
/* FAST PATH (Tag0) */
struct GNU_PACKED UNI_CMD_FAST_PATH_PROCESS_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8  aucOwnMac[6];
	UINT_16 u2RandomNum;
	UINT_32 u4Keybitmap[4];
	UINT_8  ucOpMode;
	UINT_8  ucCmdVer;
	UINT_16 u2WlanId;
	UINT_16 u2Mic;
};

/* ============== UNI_CMD_ID_STATE Begin ============== */

struct GNU_PACKED UNI_CMD_STATE_T {
	/* fixed field */
	UINT_8 u1BandIdx;
	UINT_8 au1Reserved[3];

	/* tlv */
	UINT_8 au1TlvBuffer[0];
};

/* STATE command Tag */
enum UNI_CMD_STATE_TAG_T {
	UNI_CMD_STATE_TX_RATE = 0,
	UNI_CMD_STATE_RX_RATE = 1,
	UNI_CMD_STATE_OFDMLQ_CNINFO = 2,
	UNI_CMD_STATE_TX_TD_CCK = 3,
	UNI_CMD_STATE_TOTAL_TX_CNT = 4,
	UNI_CMD_STATE_LAST_TX_RATE = 5,
	UNI_CMD_STATE_RX_STAT = 6,
#ifdef TXRX_STAT_SUPPORT
	UNI_CMD_STATE_ALL_RATE_CNT_REPORT = 7,
	UNI_CMD_STATE_STBC_CNT_REPORT = 8,
	UNI_CMD_STATE_GI_CNT_REPORT = 9,
#endif /* TXRX_STAT_SUPPORT */
	UNI_CMD_STATE_MAX_NUM
};

struct GNU_PACKED UNI_CMD_STATE_TLV_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8  au1Buffer[0];
};

struct GNU_PACKED UNI_CMD_STATE_TX_TD_CCK_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8  u1Enable;
	UINT_8  au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_STATE_RX_STAT_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8  u1Enable;
	UINT_8  au1Reserved[3];
};

#ifdef TXRX_STAT_SUPPORT
struct UNI_CMD_STATE_ALL_RATE_CNT_REPORT_T {
	UINT_16 u2Tag;    // Tag = 0x07
	UINT_16 u2Length;
	UINT_8  u1Direction;
	UINT_8  aucReserved[3];
};
#endif /* TXRX_STAT_SUPPORT */
/* ============== UNI_CMD_ID_STATE End ============== */

/* ============== UNI_CMD_ID_PER_STA_INFO Begin ============== */
#define MAX_STA_LIST_NUM 90

struct GNU_PACKED UNI_CMD_PER_STA_INFO_T {
	/* fixed field */
	UINT_8 u1Reserved;
	UINT_8 u1Unsolicit;
	UINT_8 au1Reserved[2];

	/* tlv */
	UINT_8 au1TlvBuffer[0];
};

/* PER_STA_INFO command Tag */
enum UNI_CMD_PER_STA_INFO_TAG_T {
	UNI_CMD_PER_STA_RSSI = 0,
	UNI_CMD_PER_STA_CONTENTION_RX_PHYRATE = 1,
	UNI_CMD_PER_STA_PER = 2,
	UNI_CMD_PER_STA_SNR = 3,
	UNI_CMD_PER_STA_TX_RATE = 4,
	UNI_CMD_PER_STA_TX_CNT = 5,
	UNI_CMD_PER_STA_TID_SN = 6,
	UNI_CMD_PER_STA_TID_SN_SET = 7,
	UNI_CMD_PER_STA_PKT_CNT = 8,
	UNI_CMD_PER_STA_MAX_NUM
};

struct GNU_PACKED UNI_CMD_PER_STA_INFO_COMMON_T {
	UINT_16 u2Tag;    // Tag = 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
	UINT_16 u2Length;
	UINT_16 u2StaNum;
	UINT_8  au1Reserved[2];
	UINT_16 au2StaList[MAX_STA_LIST_NUM];
};


/* ============== UNI_CMD_ID_PER_STA_INFO End ============== */

/* ============== UNI_CMD_ID_ALL_STA_INFO Begin ============== */

struct GNU_PACKED UNI_CMD_ALL_STA_INFO_T {
	/* fixed field */
	UINT_8 au1Reserved[4];

	/* tlv */
	UINT_8 au1TlvBuffer[0];
};

struct GNU_PACKED UNI_CMD_ALL_STA_INFO_TLV_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8  au1Buffer[0];
};

/* ALL_STA_INFO command Tag */
enum _UNI_CMD_ALL_STA_INFO_TAG_T {
	UNI_CMD_ALL_STA_TX_RATE = 0,
	UNI_CMD_ALL_STA_TX_STAT = 1,
	UNI_CMD_ALL_STA_TXRX_ADM_STAT = 2,
	UNI_CMD_ALL_STA_TXRX_AIR_TIME = 3,
	UNI_CMD_ALL_STA_DATA_TX_RETRY_COUNT = 4,
	UNI_CMD_ALL_STA_GI_MODE = 5,
	UNI_CMD_ALL_STA_TRX_MSDU_COUNT = 6,
	UNI_CMD_ALL_STA_MAX_NUM
};
/* ============== UNI_CMD_ID_ALL_STA_INFO End ============== */

/* UNI_CMD_ID_TRIG_ASSERT Begin */
struct GNU_PACKED UNI_CMD_TRIG_ASSERT_T {
	/* fixed field */
	UINT_8 au1Reserved[4];

	/* tlv */
	UINT_8 au1TlvBuffer[0];
};

/* TRIG_ASSERT command Tag */
enum _UNI_CMD_TRIG_ASSERT_TAG_T {
	UNI_CMD_TRIG_ASSERT_COREDUMP = 0,
	UNI_CMD_TRIG_ASSERT_MAX_NUM
};

struct GNU_PACKED UNI_CMD_TRIG_ASSERT_COREDUMP_T {
	UINT_16 u2Tag;    // Tag = 0x00
	UINT_16 u2Length;
	UINT_8  u1TrigEn;
	UINT_8  au1Reserved[3];
};
/* UNI_CMD_ID_TRIG_ASSERT End */


#ifdef WIFI_MD_COEX_SUPPORT
/* UNI_CMD_ID_IDC Begin */
enum _UNI_CMD_IDC_TAG_T {
	UNI_CMD_IDC_GET_CHN = 0,
	UNI_CMD_IDC_SET_ALWAYS_SCAN_PARAM = 1,
	UNI_CMD_IDC_CCCI_MSG = 2,
	UNI_CMD_IDC_UPDATE_3WIRE_GROUP = 3,
	UNI_CMD_IDC_UPDATE_STATE = 4,
	UNI_CMD_IDC_GET_INFO = 5,
	UNI_CMD_IDC_SET_PWR_BACKOFF = 6,
	UNI_CMD_IDC_MAX_NUM
};
/* UNI_CMD_ID_IDC End */
#endif

#define UNI_EVENT_IPI_HIST_TYPE_NUM 12

#ifdef IPI_MAINRADIO_SCAN_SUPPORT
enum _UNI_EVENT_ID_RDD_IPI_SCAN_CTRL_TAG_T {
	UNI_EVENT_RDD_IPI_SCAN_CTRL = 0x0,
	UNI_EVENT_RDD_IPI_SCAN_CTRL_MAX_NUM
};

struct _UNI_EVENT_RDD_IPI_SCAN_CTRL_T {
	/* fixed field */
	UINT_8 aucReserved[4];

	/* tlv */
	UINT_8 aucTlvBuffer[0];/**< the TLVs included in this field:
	*   TAG                                                      | ID  | structure
	*   --------------------------------   | --- | -------------
	*   UNI_EVENT_RDD_IPI_SCAN_CTRL           | 0x0 | UNI_EVENT_RDD_IPI_SCAN_CTRL_T
	*/
};
/** @} */

#define RDM_NF_MAX_WF_IDX 8
#define POWER_INDICATE_HIST_NUMBER_MAX 11

struct _UNI_EVENT_RDD_IPI_SCAN_T {
	UINT_16   u2Tag;    // Tag = 0x83
	UINT_16   u2Length;
	UINT_32 au4IpiHistVal[RDM_NF_MAX_WF_IDX][POWER_INDICATE_HIST_NUMBER_MAX] /* ant * ipi */;
	UINT_8 ucBandIdx;
	UINT_8 aucReserve[3];
	UINT_32 u4TxAssertTime;
};
#endif

struct GNU_PACKED UNI_EVENT_RDD_IPI_HIST {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT8 u1IpiHistIdx;
	UINT8 u1BandIdx;
	UINT8 aucReserve[2];
	UINT32 u4IpiHistVal[UNI_EVENT_IPI_HIST_TYPE_NUM];
	UINT32 u4TxAssertTime;
};

/* ============== UNI_CMD_ID_SANDBOX_CTRL  Begin ============== */

struct GNU_PACKED UNI_CMD_SANDBOX {
	/* fixed field */
	UINT_8 aucReserved[4];

	/* tlv */
	UINT_8 aucTlvBuffer[0]; /**<the TLVs includer in this field:
		*
		*  TAG                                 | ID   | structure
		*  ------------------------------------| -----| -------------
		*  UNI_CMD_RA_SANDBOX_CTRL             | 0x0  | UNI_CMD_SANDBOX_TAG_T

		*/
};

enum UNI_CMD_SANDBOX_TAG {
	UNI_CMD_RA_SANDBOX_CTRL = 0x00,
	UNI_CMD_SANDBOX_CMD_MAX_NUM
};

struct GNU_PACKED UNI_CMD_RA_SANDBOX {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8  u1UsrIdx;
	UINT_8  u1WlanId;
	BOOLEAN fgSdboxEn;
	UINT_8  u1NcDim;
	UINT_8  u1TblIdx;
	UINT_8  u1UpDnRateInd;
	INT_8   i1Cn1;
	INT_8   i1Cn2;
	UINT_8  u1Reserved;
};

typedef INT32 (*PFN_SANDBOX_HANDLE)(struct _RTMP_ADAPTER *pAd, IN RTMP_STRING * arg, VOID *pHandle);

/* ============== UNI_CMD_ID_SANDBOX_CTRL End ============== */

/* ============== UNI_CMD_ID_EPCS Begin ============== */

struct GNU_PACKED UNI_CMD_EPCS_T {
	/* fixed field */
	UINT_8 aucReserved[4];

	/* tlv */
	UINT_8 aucTlvBuffer[0]; /**<the TLVs includer in this field:
		*
		*  TAG                                 | ID   | structure
		*  ------------------------------------| -----| -------------
		*  UNI_CMD_EPCS_CTRL		       | 0x1  | struct UNI_CMD_EPCS_T

		*/
};

enum UNI_CMD_EPCS_TAG {
	UNI_CMD_EPCS_CTRL = 0x01,
	UNI_CMD_EPCS_MAX_NUM
};

#define UNI_CMD_EPCS_CMD_PARAM_NUM      64

struct GNU_PACKED UNI_CMD_EPCS_CTRL_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_32 u4Cmd;
	UINT_32 au4CmdParam[UNI_CMD_EPCS_CMD_PARAM_NUM];
};

//typedef INT32 (*PFN_EPCS_HANDLE)(struct _RTMP_ADAPTER *pAd, IN RTMP_STRING * arg, VOID *pHandle);

/* ============== UNI_CMD_ID_EPCS End ============== */

/* ============== UNI_CMD_ID_3WIRE_EXT Begin ============== */
#ifdef CONFIG_3_WIRE_SUPPORT
struct GNU_PACKED UNI_CMD_3WIRE_EXT_T {
	UINT_8 ucReserved[4];

	/* tlv */
	UINT_8 aucTlvBuffer[0]; /**<the TLVs includer in this field:
		*
		*  TAG                                 | ID   | structure
		*  ------------------------------------| -----| -------------
		*  UNI_CMD_3WIRE_EXT_EN                | 0x0  | UNI_CMD_3WIRE_EXT_EN_T

		*/
};

/* 3WIRE_EXT config Tag */
enum UNI_CMD_3WIRE_EXT_TAG_T {
	UNI_CMD_3WIRE_EXT_EN = 0,
	UNI_CMD_3WIRE_EXT_MAX_NUM
};

struct GNU_PACKED UNI_CMD_3WIRE_EXT_EN_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8  uc3WireExtEn;  // bit1: 3Wire_EXT1 , bit0:3Wire_EXT0, 1:enable;0:disable
};
#endif
/* ============== UNI_CMD_ID_3WIRE_EXT End ============== */

/* ============== UNI_CMD_ID_SDO Begin ============== */
/*  UNI_CMD_ID_SDO (0x88) */
struct GNU_PACKED _UNI_CMD_SDO_T
{
	/* fix field */
	UINT_8 au1Reserved[4];

	/* tlv */
	UINT_8 aucTlvBuffer[0];/**< the TLVs included in this field:
	*
	*   TAG                             | ID  | structure
	*   --------------------------------|-----|--------------
	*   UNI_CMD_SDO_SET_OP        | 0x1 | UNI_CMD_SDO_SET_OP_T
	*   UNI_CMD_SDO_QUERY_OP      | 0x2 | UNI_CMD_SDO_QUERY_OP_T
	*   UNI_CMD_SDO_AUTO_BA       | 0x3 | UNI_CMD_SDO_AUTO_BA_T
	*/
};

/** @} */

/* thermal command TLV List */
enum _UNI_CMD_SDO_TAG_T {
	UNI_CMD_SDO_SET_OP = 1,
	UNI_CMD_SDO_QUERY_OP = 2,
	UNI_CMD_SDO_AUTO_BA = 3,
	UNI_CMD_SDO_NUM
};

/** @addtogroup UNI_CMD_ID_SDO
 * @{
 */
/**
 * This structure is used for UNI_CMD_SDO_SET_OP (0x1) of UNI_CMD_ID_SDO command (0x88)
 * to set SDO function mode
 *
 * @param[in] u2Tag                   should be 0x1
 * @param[in] u2Length                the length of this TLV, should be sizeof(UNI_CMD_SDO_SET_OP_T)
 * @param[in] u4SdoSetArg0            Set parameter 0
 * @param[in] u4SdoSetArg1            Set parameter 1
 * @param[in] u4SdoSetArg2            Set parameter 2
 */
/* sdo set operation (Tag1) */
struct GNU_PACKED _UNI_CMD_SDO_SET_OP_T
{
	UINT_16   u2Tag;
	UINT_16   u2Length;
	UINT_32   u4SdoSetArg0;
	UINT_32   u4SdoSetArg1;
	UINT_32   u4SdoSetArg2;
};

/** @addtogroup UNI_CMD_ID_SDO
 * @{
 */
/**
 * This structure is used for UNI_CMD_SDO_QUERY_OP (0x2) of UNI_CMD_ID_SDO command (0x88)
 * to set SDO function mode
 *
 * @param[in] u2Tag                   should be 0x2
 * @param[in] u2Length                the length of this TLV
 * @param[in] u4SdoQueryOptionArg0    Query parameter 0
 * @param[in] u4SdoQueryOptionArg1    Query parameter 1
 * @param[in] u4SdoQueryOptionArg2    Query parameter 2
 * @param[in] ucTid                   the Tid
 */
/* sdo set operation (Tag2) */
struct GNU_PACKED _UNI_CMD_SDO_QUERY_OP_T
{
	UINT_16   u2Tag;
	UINT_16   u2Length;
	UINT_32   u4SdoQueryOptionArg0;
	UINT_32   u4SdoQueryOptionArg1;
	UINT_32   u4SdoQueryOptionArg2;
	UINT_32   u4SdoQueryList[0];
};

/** @addtogroup UNI_CMD_ID_SDO
 * @{
 */
/**
 * This structure is used for UNI_CMD_SDO_AUTO_BA (0x3) of UNI_CMD_ID_SDO command (0x88)
 * to set SDO function mode
 *
 * @param[in] u2Tag                   should be 0x3
 * @param[in] u2Length                the length of this TLV, should be sizeof(UNI_CMD_SDO_AUTO_BA_T)
 * @param[in] ucAutoBaEnable          enable AutoBa
 * @param[in] ucTarget                Auto Ba target
 * @param[in] u4Timeout               timeout value(unit ms)
 */
/* sdo auto ba (Tag3) */
struct GNU_PACKED _UNI_CMD_SDO_AUTO_BA_T
{
    UINT_16   u2Tag;
    UINT_16   u2Length;
    UINT_8    ucAutoBaEnable;   // 0: No reload, 1: do reload
    UINT_8    ucTarget;
    UINT_8    aucReserve[2];
    UINT_32   u4Timeout;    /* timeout value, unit ms. */
};
/* ============== UNI_CMD_ID_SDO End ============== */

BOOLEAN UniCmdCheckInitReady(struct _RTMP_ADAPTER *pAd);

struct cmd_msg *AndesAllocUniCmdMsg(struct _RTMP_ADAPTER *pAd, unsigned int length);

INT32 UniCmdDevInfoUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT64 u8EnableFeature);

INT32 UniCmdBssInfoUpdate(struct _RTMP_ADAPTER *pAd, struct _BSS_INFO_ARGUMENT_T *bss_info_argument);

INT32 MtUniCmdPmStateCtrl(struct _RTMP_ADAPTER *pAd, MT_PMSTAT_CTRL_T PmStatCtrl);

INT32 MtUniCmdSlotTimeSet(
	struct _RTMP_ADAPTER *pAd,
	UINT16 SlotTime,
	UINT16 SifsTime,
	UINT16 RifsTime,
	UINT16 EifsTime,
	struct wifi_dev *wdev);

INT32 MtUniCmdFrameOffloadSet(struct _RTMP_ADAPTER *pAd, P_CMD_FRAME_OFFLOAD_T fdFrame_offload);

#ifdef DOT11_EHT_BE
INT32 MtUniCmdMldLinkOp(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
INT32 UniCmdMldAt2lmReq(
	struct _RTMP_ADAPTER *pAd,
	struct AT2LM_RES_REQ_CTRL_T *req,
	struct AT2LM_RES_RSP_CTRL_T *rsp);
INT32 UniCmdPeerMldNt2lmReq(
	struct _RTMP_ADAPTER *pAd,
	struct NT2LM_REQ_CTRL_T *req);
INT32 UniCmdMldReconfigTmr(struct _RTMP_ADAPTER *pAd, struct RECONFIG_SET_TMR_CTRL_T *reconfig_ctrl);
INT32 UniCmdMldReconfigRmLinkReq(
	struct _RTMP_ADAPTER *pAd,
	struct RECONFIG_RM_LINK_REQ_CTRL_T *req);
#endif

INT32 UniCmdStaRecUpdateMuruInfo(struct _RTMP_ADAPTER *pAd, STA_REC_CFG_T *pStaRecCfg,
	UNI_CMD_TAG_HANDLE_T *pTagHandle, VOID *pHandle);

INT32 UniCmdStaRecMldSetup(struct _RTMP_ADAPTER *pAd, STA_REC_CFG_T *pStaRecCfg,
	UNI_CMD_TAG_HANDLE_T *pTagHandle, VOID *pHandle);

INT32 UniCmdStaRecEhtMld(struct _RTMP_ADAPTER *pAd, STA_REC_CFG_T *pStaRecCfg,
	UNI_CMD_TAG_HANDLE_T *pTagHandle, VOID *pHandle);

INT32 UniCmdStaRecUpdate(struct _RTMP_ADAPTER *pAd, STA_REC_CFG_T *pStaRecCfg);

INT32 UniCmdStaRecBaUpdate(struct _RTMP_ADAPTER *pAd, STA_REC_BA_CFG_T StaRecBaCfg);

#ifdef HTC_DECRYPT_IOT
INT32 UniCmdStaRecAADOmUpdate(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, UINT8 AadOm);
#endif /* HTC_DECRYPT_IOT */

#ifdef MLR_SUPPORT
INT32 UniCmdStaRecForceMlrState(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, UINT8 ForceMLRState);
#endif /* MLR_SUPPORT */

INT32 UniCmdStaRecPsmUpdate(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, UINT8 Psm);

INT32 UniCmdSetRecSecPnInfo(RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT8 ucOperation,
					  VOID *pBuffer, UINT32 u4BufferLen, UINT32 tsc_cnt);
#ifdef DOT11_EHT_BE
INT32 UniCmdStaRecEmlOp(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg);

INT32 UniCmdStaRecTxCapUpdate(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg,
	BOOLEAN EnableAGGLimit);
#endif /* DOT11_EHT_BE */
INT32 UniCmdStaSNSet(
	struct _RTMP_ADAPTER *pAd,
	INT wcid,
	UCHAR SnUpdate_type);
INT32 MtUniCmdEdcaParameterSet(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, P_CMD_EDCA_CTRL_T pEdcaParam);

INT32 UniCmdBandConfig(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_BAND_CFG_PARAM_T pParamCtrl);

INT32 UniCmdRadioOnOff(struct _RTMP_ADAPTER *pAd, MT_PMSTAT_CTRL_T PmStatCtrl);

INT32 UniCmdRxvCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 TxRx, UINT8 ucEnable);

INT32 UniCmdSetEDCCAThreshold(struct _RTMP_ADAPTER *pAd, UINT8 u1edcca_threshold[], UINT8 u1BandIdx, BOOLEAN bInit);

INT32 UniCmdGetEDCCAThreshold(struct _RTMP_ADAPTER *pAd, UINT8 u1BandIdx, BOOLEAN bInit);

INT32 UniCmdSetEDCCAEnable(struct _RTMP_ADAPTER *pAd, UINT8 u1EDCCACtrl, UINT8 u1BandIdx, UINT8 u1EDCCAStd);

INT32 UniCmdGetEDCCAEnable(struct _RTMP_ADAPTER *pAd, UINT8 u1BandIdx);

#ifdef ACK_CTS_TIMEOUT_SUPPORT
INT32 UniCmdACKCTSTimeoutRead(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 Type, UINT32 *Value);
#endif /* ACK_CTS_TIMEOUT_SUPPORT */

INT32 UniCmdUpdateRTSThreshold(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR pkt_num, UINT32 length);

INT32 UniCmdConfigSetRtsSignalEn(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN enable);

INT32 UniCmdConfigSetSchDetDis(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN enable);

INT32 UniCmdSetRTS0PktThresholdCfg(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 value);

INT32 UniCmdSetMacTxRx(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx, UINT8 ucMacEnable);

INT32 UniCmdSetRxAckToHost(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN enable);

INT32 UniCmdSetBarRetry(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);

INT32 UniCmdGetStaPause(struct _RTMP_ADAPTER *pAd, UINT8 u1BandIdx);

INT32 UniCmdWsysConfig(struct _RTMP_ADAPTER *pAd, struct UNI_CMD_WSYS_CFG_PARAM_T *pParamCtrl);

INT32 UniCmdAccessReg(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_ACCESS_REG_PARAM_T pParamCtrl);

INT32 UniCmdMultipleMacRegAccessRead(struct _RTMP_ADAPTER *pAd, struct _RTMP_REG_PAIR *RegPair, UINT32 Num);

INT32 UniCmdMultipleMacRegAccessWrite(struct _RTMP_ADAPTER *pAd, struct _RTMP_REG_PAIR *RegPair, UINT32 Num);

INT32 UniCmdMultipleRfRegAccessRead(struct _RTMP_ADAPTER *pAd, struct _MT_RF_REG_PAIR *RegPair, UINT32 Num);

INT32 UniCmdMultipleRfRegAccessWrite(struct _RTMP_ADAPTER *pAd, struct _MT_RF_REG_PAIR *RegPair, UINT32 Num);

INT32 UniCmdRFRegAccessRead(struct _RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 *Value);

INT32 UniCmdRFRegAccessWrite(struct _RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 Value);

INT32 UniCmdRestartDLReqNoRsp(struct _RTMP_ADAPTER *pAd);

INT32 UniCmdClockSwitchDisable(struct _RTMP_ADAPTER *pAd, UINT8 isDisable);

INT32 UniCmdWifiHifCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucDbdcIdx,
	UINT8 ucHifCtrlId,
	VOID *pRsult);

#ifdef GREENAP_SUPPORT
INT32 UniCmdGreenAPOnOffCtrl(
	struct _RTMP_ADAPTER *pAd,
	MT_GREENAP_CTRL_T GreenAPCtrl);
#endif /* GREENAP_SUPPORT */

INT32 UniCmdCfgInfoUpdate(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	ENUM_CFG_FEATURE eFeature,
	VOID *param);

INT32 MtUniCmdFwLog2Host(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 FWLog2HostCtrl);

INT32 MtUniCmdFwDbgCtrl(struct _RTMP_ADAPTER *pAd, UINT32 DbgClass, UINT32 ModuleIdx);

INT32 MtUniCmdFwBasicConfig(struct _RTMP_ADAPTER *pAd);

INT32 MtUniCmdFwTimeSync(struct _RTMP_ADAPTER *pAd, UINT32 second, UINT32 usecond);

INT32 MtUniCmdFwMbssBcnDup(struct _RTMP_ADAPTER *pAd, BOOLEAN bWMDup);

#ifdef AIR_MONITOR
INT32 MtUniCmdSmeshConfigSet(struct _RTMP_ADAPTER *pAd, UCHAR *pdata, P_UNI_CMD_SMESH_PARAM_T prSmeshResult);
#endif /* AIR_MONITOR */

INT32 UniCmdRxHdrTrans(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_RX_HDR_TRAN_PARAM_T pParamCtrl);

INT32 UniCmdSER(struct _RTMP_ADAPTER *pAd, UINT32 u4Action, UINT32 u4SetValue, UINT8 ucDbdcIdx);
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
INT32 UniCmdTwtAgrtUpdate(struct _RTMP_ADAPTER *pAd, struct MT_TWT_AGRT_PARA_T *AgrtPara);
INT32 UniCmdTwtAgrtMgmt(struct _RTMP_ADAPTER *pAd, struct MT_TWT_AGRT_PARA_T *AgrtPara, struct MT_TWT_AGRT_MGMT_T *AgrtMgmt);
INT32 UniCmdTwtMgmtFrameOffload(struct _RTMP_ADAPTER *pAd, struct MT_MGMT_FRAME_OFFLOAD_T *OffloadPara);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

INT32 UniCmdMuarConfigSet(
	struct _RTMP_ADAPTER *pAd,
	UCHAR *pdata,
	BOOLEAN bSmesh,
	BOOLEAN bEntryAdd);

INT32 UniCmdCalculateECC(
	struct _RTMP_ADAPTER *pAd,
	UINT32 oper, UINT32 group,
	UINT8 *scalar,
	UINT8 *point_x,
	UINT8 *point_y);

#ifdef MT_DFS_SUPPORT
INT32 UniCmdRddCtrl(
	struct _RTMP_ADAPTER *pAd,
	UCHAR ucDfsCtrl,
	UCHAR ucRddIdex,
	UCHAR ucRddRxSel,
	UCHAR ucSetVal);

INT32 UniCmdSetRddIpiHist(
	struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_RDD_IPI_HIST_T *pData);

INT32 UniCmdGetRddIpiHist(
	struct _RTMP_ADAPTER *pAd,
	UINT8 rdd_ipi_hist_idx,
	struct UNI_EVENT_RDD_IPI_HIST *p_rdd_ipi_hist_rlt);

#ifdef IPI_MAINRADIO_SCAN_SUPPORT
INT32 UniCmdSetRddIpiScan(
	struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_RDD_IPI_SCAN_T *p_cmd_rdd_ipi_scan);

INT32 UniCmdGetRddIpiScan(
	struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_RDD_IPI_SCAN_T *p_cmd_rdd_ipi_scan,
	struct _UNI_EVENT_RDD_IPI_SCAN_T *p_event_rdd_ipi_scan);

#endif
INT32 UniCmdSetFcc5MinIpn(
	struct _RTMP_ADAPTER *pAd,
	UINT16 min_lpn_update);


INT32 UniCmdSetRadarThresParam(
	struct _RTMP_ADAPTER *pAd,
	P_CMD_RDM_RADAR_THRESHOLD_UPDATE_T p_radar_threshold);

INT32 UniCmdSetPlsThresParam(
	RTMP_ADAPTER *pAd,
	P_CMD_RDM_PULSE_THRESHOLD_UPDATE_T p_pls_threshold);

INT32 UniCmdSetRddLogConfig(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 hw_rdd_log_en,
	IN UINT8 sw_rdd_log_en,
	IN UINT8 sw_rdd_log_cond);

INT32 UniCmdTestRadarPattern(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _EXT_CMD_RDM_TEST_RADAR_PATTERN_T *pls_pattern);
#endif /* MT_DFS_SUPPORT */

#if OFF_CH_SCAN_SUPPORT
INT32 UniCmdOffChScan(
	struct _RTMP_ADAPTER *pAd,
	struct _EXT_CMD_OFF_CH_SCAN_CTRL_T *ext_cmd_param);
#endif

#ifdef ZERO_PKT_LOSS_SUPPORT
INT32 UniCmdZeroLossEnable(
	struct _RTMP_ADAPTER *pAd,
	INT enable);

INT32 UniCmdMacTxEnable(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN enable, UCHAR BandIdx);

INT32 UniCmdWcidSkipTx(
	IN struct _RTMP_ADAPTER *pAd,
	UINT_16 WlanIdx, UINT_8	Operation, BOOLEAN WtblSkipTXEnable, UINT32 *prSkipTxResult);
#endif

INT32 UniCmdGetTsfTime(
	struct _RTMP_ADAPTER *pAd,
	UCHAR HwBssidIdx,
	TSF_RESULT_T *pTsfResult);

INT32 UniCmdGetTsfDiffTime(
	struct _RTMP_ADAPTER *pAd,
	UINT8 BssIdx0,
	UINT8 BssIdx1,
	struct TSF_DIFF_RESULT_T *pTsfDiffResult);

#ifdef CFG_SUPPORT_FALCON_TXCMD_DBG
INT32 UniCmdTxCmdDbgCtrl(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_TXCMD_DBG_CTRL_PARAM_T pParamCtrl);
#endif /* CFG_SUPPORT_FALCON_TXCMD_DBG */

INT32 UniCmdMib(struct _RTMP_ADAPTER *pAd, UCHAR ChIdx, RTMP_MIB_PAIR *RegPair, UINT32 Num);

INT32 UniCmdAllStaInfoGetInfo(
	struct _RTMP_ADAPTER *pAd,
	UINT16 subevent_type
);

#ifdef CONFIG_HW_HAL_OFFLOAD
INT32 UniCmdSetSnifferMode(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_SNIFFER_MODE_T param);
#endif /* CONFIG_HW_HAL_OFFLOAD */

#ifdef SMART_CARRIER_SENSE_SUPPORT
INT32 UniCmdSCS(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_SCS_PARAM_T pParamCtrl);
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#ifdef WIFI_MODULE_DVT
INT32 UniCmdMDVT(struct _RTMP_ADAPTER *pAd, UINT16 u2ModuleId, UINT16 u2CaseId, UINT8 u1CapId);
#endif /* WIFI_MODULE_DVT */

#ifdef WIFI_GPIO_CTRL
INT32 UniCmdGPIO(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_GPIO_CFG_PARAM_T pParamCtrl);
#endif /* WIFI_GPIO_CTRL */

INT32 UniCmdEepromAccessRead(
	struct _RTMP_ADAPTER *pAd,
	UINT32 offset,
	UINT8  *pData,
	PUINT pIsValid,
	UINT32 DataLen
);

INT32 UniCmdEepromAccessWrite(
	struct _RTMP_ADAPTER *pAd,
	UINT32  offset,
	UINT8   *pData,
	UINT32 DataLen
);

INT32 UniCmdEfuseAccessRead(
	struct _RTMP_ADAPTER *pAd,
	UINT16 offset,
	UINT8  *pData,
	PUINT  pIsValid
);

INT32 UniCmdEfuseAccessWrite(
	struct _RTMP_ADAPTER *pAd,
	UINT16  offset,
	UINT8   *pData
);

INT32 UniCmdEfuseFreeBlockCount(
	struct _RTMP_ADAPTER *pAd,
	PVOID pGetFreeBlock,
	PVOID pResult
);

INT32 UniCmdEfuseBufferModeSet(
	struct _RTMP_ADAPTER *pAd,
	UINT8  EepromType
);

INT32 UniCmdEfusBufferModeGet(
	struct _RTMP_ADAPTER *pAd,
	UINT8   EepromType,
	UINT16  dump_offset,
	UINT16  dump_size,
	UINT8   *epprom_content
);

#ifdef WIFI_EAP_FEATURE
INT32 UniCmdEapInitIPICtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx
);

INT32 UniCmdEapGetIPIValue(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx
);

INT32 UniCmdEapSetDataTxPwrOffset(
	struct _RTMP_ADAPTER *pAd,
	UINT16 u2WlanIdx,
	INT8  i1TxPowerOffset,
	UINT8 u1BandIdx
);

INT32 UniCmdEapSetRaTable(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT8 u1TblType,
	UINT8 u1TblIndex,
	UINT16 u2TblLength,
	UINT8 *Buffer,
	UINT16 u2Len
);

INT32 UniCmdEapGetRaTblInfo(
	struct _RTMP_ADAPTER *pAd,
	UINT8 BandIdx,
	UINT8 TblType,
	UINT8 TblIndex,
	UINT8 ReadnWrite
);
#endif

INT32 UniCmdRAParameterSet(
	struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	UINT32 u4EnableFeature);

INT32 UniCmdHeraSupportRateTable(
	IN PRTMP_ADAPTER pAd,
	UINT8 tx_mode,
	UINT8 tx_nss,
	UINT8 tx_bw,
	UINT16 mcs_cap,
	BOOLEAN set);

INT32 UniCmdRAFixRateUpdateWoSta(
	IN PRTMP_ADAPTER pAd,
	UINT16 u2Wcid,
	RA_PHY_CFG_T *pFixedRateCfg,
	UINT8 u1SpeEn,
	UINT8 u1ShortPreamble);

INT32 UniCmdHeraDbgCtrl(IN PRTMP_ADAPTER pAd, UINT8 param_num, UINT32 *param);

INT32 UniCmdGetMuRaInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);

INT32 UniCmdGetRuRaInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);

INT32 UniCmdRAFixedRateProc(
	struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg);

INT32 UniCmdRAFixedRateUlProc(
	struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg);

INT32 UniCmdRAAutoRateProc(
	struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg);

INT32 UniCmdChannelSwitch(
	struct _RTMP_ADAPTER *pAd,
	struct _MT_SWITCH_CHANNEL_CFG SwChCfg);

INT UniCmdSetTxRxPath(
	struct _RTMP_ADAPTER *pAd,
	struct _MT_SWITCH_CHANNEL_CFG SwChCfg);

INT32 UniCmdMuruParameterSet(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	UINT64 u8EnableFeature);

INT32 UniCmdMuruParameterGet(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	UINT64 u8EnableFeature);

INT32 uni_hqa_muru_set_dl_tx_muru_config(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

INT32 uni_hqa_muru_set_ul_tx_muru_config(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

INT32 uni_hqa_muru_set_mu_tx_pkt_en(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg
);

INT32 uni_hqa_muru_set_mu_tx_pkt_cnt(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg
);

INT32 uni_muru_set_drv_black_list_ctrl(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg
);

INT32 uni_set_muru_manual_config(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

INT32 uni_set_pqs_manual_scs(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

VOID uni_muru_update_he_cfg(
	struct _RTMP_ADAPTER *pAd);

#ifdef CFG_SUPPORT_FALCON_PP
INT32 UniCmdPPCapCtrl
	(struct _RTMP_ADAPTER *pAd,
	P_PP_CMD_T pp_cmd_cap,
	struct CMD_STATIC_PP_DSCB_T *dscb,
	struct CMD_PP_MU_CTRL_T *mu_ctrl,
	struct CMD_CHANGE_PP_CAP_CTRL_T *pp_cap);
INT32 UniCmdPPAlgCtrl(struct _RTMP_ADAPTER *pAd, struct PP_ALG_CTRL_T *pp_cmd_alg_ctrl);
#endif /* CFG_SUPPORT_FALCON_PP */

#ifdef TXBF_SUPPORT
INT32 UniCmdETxBfSoundingPeriodicTriggerCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1SndgEn,
	UINT32 u4SNDPeriod,
	UINT8  u1Su_Mu,
	UINT8  u1MuNum,
	UINT8  *pwlanidx);

INT32 UniCmdTxBfTxApplyCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT16 u2WlanId,
	UINT8  fgETxBf,
	UINT8  fgITxBf,
	UINT8  fgMuTxBf,
	UINT8  fgPhaseCali);

INT32 UniCmdPfmuMemAlloc(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1Su_Mu,
	UINT16 u2WlanId);

INT32 UniCmdPfmuMemRelease(
	struct _RTMP_ADAPTER *pAd,
	UINT16 u2WlanId);

INT32 UniCmdETxBfPfmuProfileTagRead(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1PfmuIdx,
	UINT8  fgBFer,
	UINT8  u1BandIdx);

INT32 UniCmdETxBfPfmuProfileTagWrite(
	struct _RTMP_ADAPTER *pAd,
	UINT8  *prPfmuTag1,
	UINT8  *prPfmuTag2,
	UINT8   u1tag1_len,
	UINT8   u1tag2_len,
	UINT8   u1PfmuIdx,
	UINT8   u1BandIdx);

INT32 UniCmdETxBfPfmuProfileDataRead(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1PfmuIdx,
	UINT8  fgBFer,
	UINT16 u2SubCarrIdx,
	UINT8  u1BandIdx);

INT32 UniCmdETxBfPfmuProfileDataWrite(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1PfmuIdx,
	UINT16 u2SubCarrIdx,
	UINT8  *pProfileData,
	UINT8  u1BandIdx);

INT32 UniCmdPfmuMemAllocMapRead(
	struct _RTMP_ADAPTER *pAd);

INT32 UniCmdETxBfAidSetting(
	struct _RTMP_ADAPTER *pAd,
	UINT16 u2Aid,
	UINT8  u1BandIdx,
	UINT8  u1OwnMacIdx);

INT32 UniCmdITxBfPhaseCal(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1Group,
	UINT8  u1Group_L_M_H,
	UINT8  fgSX2,
	UINT8  u1PhaseCalType,
	UINT8  u1PhaseVerifyLnaGainLevel,
	UINT8  u1BandIdx,
	UINT8  u1PhaseOutRptVersion);

INT32 UniCmdITxBfPhaseComp(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1BW,
	BOOLEAN fgJPCh,
	UINT8  u1DbdcBandIdx,
	UINT8  u1Group,
	UINT8  fgRdFromE2p,
	UINT8  fgDisComp);

INT32 UniCmdTxBfLnaGain(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1LnaGain,
	UINT8  u1BandIdx);

INT32 UniCmdETxBfPfmuProfileDataWrite20MAll(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1PfmuIdx,
	UINT8  u1BandIdx,
	UINT8  *pu1ProfileData);

INT32 UniCmdETxBfPfmuProfileDataWrite20MAll_5x5(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1PfmuIdx,
	UINT8  u1BandIdx,
	UINT8  *pu1ProfileData);

INT32 UniCmdTxBfApClientCluster(
	struct _RTMP_ADAPTER *pAd,
	UINT16 u2WlanIdx,
	UINT16 u2CmmWlanId);

INT32 UniCmdTxBfHwEnableStatusUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT8  fgEBf,
	UINT8  fgIBf);

INT32 UniCmdTxBfeeHwCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8  fgBfeeHwEn);

INT32 UniCmd_txbf_config(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1ConfigType,
	UINT8  config_para[]);

INT32 UniCmdETxBfPfmuFullDimDataWrite(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1PfmuIdx,
	UINT16 u2SubCarrIdx,
	UINT8  u1Bfer,
	UINT8  *pu1ProfileData,
	UINT8  u1DataLength,
	UINT8  u1BandIdx);

INT32 UniCmdETxBfPlyInfo(
	struct _RTMP_ADAPTER *pAd,
	UINT8  *pu1Data);

INT32 UniCmdHeRaMuMetricInfo(
	struct _RTMP_ADAPTER *pAd,
	UINT8  *pu1Data);

INT32 UniCmdETxBfTxCmd(
	struct _RTMP_ADAPTER *pAd,
	UINT8  *pu1Data);

INT32 UniCmdETxBfCfgBfPhy(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1Action,
	UINT8  u1BandIdx,
	UINT8  u1SmthIntlBypass);

INT32 UniCmdETxBfSndCnt(
	struct _RTMP_ADAPTER *pAd,
	UINT8  *pu1Data);

INT32 UniCmdTxBfModuleEnCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1BfNum,
	UINT8  u1BfBitmap,
	UINT8  u1BfSelBand[]);

INT32 UniCmdETxBfTxSndInfo(
	struct _RTMP_ADAPTER *pAd,
	UINT8  *pucData);

INT32 UniCmdETxBfFbRptDbgInfo(
	struct _RTMP_ADAPTER *pAd,
	UINT8  *pucData);

INT32 UniCmdETxBfPseudoTagWrite(
	struct _RTMP_ADAPTER *pAd,
	IN UINT8  lm,
	IN UINT8  nr,
	IN UINT8  nc,
	IN UINT8  bw,
	IN UINT8  codebook,
	IN UINT8  group);

INT32 UniCmdETxBfStaRecRead(
	struct _RTMP_ADAPTER *pAd,
	UINT16 u2WlanId);
#endif

INT32 UniCmdTxPowerSKUCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 tx_pwr_sku_en,
	UINT8 u1BandIdx
);

INT32 UniCmdTxPowerPercentCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 fgTxPowerPercentEn,
	UINT8 u1BandIdx
);

INT32 UniCmdTxPowerDropCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 cPowerDropLevel,
	UINT8 u1BandIdx
);

INT32 UniCmdTxBfBackoffCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 fgTxBFBackoffEn,
	UINT8 u1BandIdx
);

INT32 UniCmdPwrLimitTblUpdate(
	struct _RTMP_ADAPTER *pAd,
	P_CMD_POWER_LIMIT_TABLE_CTRL_T prBuffer
);

INT32 UniCmdATEModeCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1ATEMode
);

INT32 UniCmdTxPwrShowInfo(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucTxPowerInfoCatg,
	UINT8 u1BandIdx
);

#ifdef TPC_SUPPORT
INT32 UniCmdTpcFeatureCtrl(
	struct _RTMP_ADAPTER *pAd,
	INT8  TpcPowerValue,
	UINT8 u1BandIdx,
	UINT8 CentralChannel
);
#endif

INT32 UniCmdHwcfgGet(
	struct _RTMP_ADAPTER *pAd,
	UINT16 dump_offset,
	UINT16 dump_size,
	UINT8 *epprom_content
);

INT32 UniCmdATETest(
	struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_TEST_TR_PARAM_CTRL_T *tr_param,
	uint64_t u8TestTrParamFeature,
	uint8_t ucParamNum,
	uint32_t *pu4Status
);

INT32 UniCmdTestmodeRxStat(
	struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_TESTMODE_RX_T *prTestmodeRx,
	uint64_t u8TestmodeRxFeature,
	uint8_t *pucTestmodeRxWb,
	uint8_t band_idx);

#ifdef SWACI_MECHANISM
INT32 UniCmdSetSwlnaTestMode(
	struct _RTMP_ADAPTER *pAd,
	UINT32 u4TestMode
);

INT32 UniCmdSwlnaTimerCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT8 u1Enable
);

INT32 UniCmdSwlnaEnable(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1Enable
);

INT32 UniCmdSetSwlnaThresh(
	struct _RTMP_ADAPTER *pAd,
	UINT32 u4Threshold
);

INT32 UniCmdSetSwlnaCondition(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1Swcr0,
	UINT8 u1Swcr1,
	UINT8 u1Swcr2,
	INT8 i1Swcr4
);

INT32 UniCmdSetSwlnaDenseParam(
	struct _RTMP_ADAPTER *pAd,
	INT8 i1MaxRcpi,
	INT8 i1MinSubRssi,
	UINT8 u1ReadCount,
	UINT8 u1MinDenseCount,
	UINT8 u1MaxDenseCount,
	UINT8 u1DenseCRValue
);

INT32 UniCmdSetSwlnaTable(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1PhyBw,
	INT8 i4LnaIdx0,
	INT8 i4LnaIdx1,
	INT8 i4LnaIdx2,
	INT8 i4LnaIdx3,
	INT8 i4LnaIdx4
);

INT32 UniCmdSetSwaciGain(
	struct _RTMP_ADAPTER *pAd,
	UINT8 *pData
);

INT32 UniCmdSetTestModeRcpi(
	struct _RTMP_ADAPTER *pAd,
	UINT8 *pData
);
#endif

INT32 UniCmdMuPwrCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 fgMuTxPwrManEn,
	INT8  cMuTxPwr,
	UINT8 u1BandIdx
);

INT32 UniCmdTxPwrDataPktCtrl(
	RTMP_ADAPTER *pAd,
	IN UINT16 u2Wcid,
	IN INT8 i1MaxBasePwr,
	IN INT8 i1PowerOffset,
	IN UINT8 u1BandIdx
);

INT32 UniCmdTxPwrMinDataPktCtrl(
	RTMP_ADAPTER *pAd,
	IN INT8 i1MinBasePwr,
	IN UINT8 u1BandIdx
);


INT32 UniCmdTxPwrUpCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	INT8  cPwrUpCat,
	CHAR  cPwrUpValue[POWER_UP_CATEGORY_RATE_NUM_V2]
);

INT32 UniCmdSetTxPowerCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1DbdcIdx,
	INT8  i1TargetPower,
	UINT8 u1AntIdx,
	UINT8 u1CenterChannel
);

INT32 UniCmdGetTxPower(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1DbdcIdx,
	UINT8 u1CenterChannel,
	UINT8 u1AntIdx,
	P_EXT_EVENT_ID_GET_TX_POWER_T prTxPwrResult
);

INT32 UniCmdSetForceTxPowerCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	INT8  i1TxPower,
	UINT8 u1PhyMode,
	UINT8 u1TxRate,
	UINT8 u1BW
);

INT32 UniCmdWifiSpectrumStart(struct _RTMP_ADAPTER *pAd, UINT8 *pData);
INT32 UniCmdWifiSpectrumCapStatus(struct _RTMP_ADAPTER *pAd, INT8 *rsp_payload, UINT16 rsp_wait_time);
INT32 UniCmdSpectrumGetRawData(struct _RTMP_ADAPTER *pAd);

INT32 UniCmdPhyIcsStart(struct _RTMP_ADAPTER *pAd, UINT8 *pData);

INT32 UniCmdPhyIcsEventEnable(struct _RTMP_ADAPTER *pAd, UINT8  *pData);

INT32 UniCmdTPC(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_TPC_PARAM_T pParamCtrl);

INT32 UniCmdTPCManCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgTpcManual);

INT32 UniCmdTPCUlAlgoCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8	u1TpcCmd,
	UINT8	u1ApTxPwr,
	UINT8	u1EntryIdx,
	UINT8	u1TargetRssi,
	UINT8	u1UPH,
	BOOLEAN	fgMinPwrFlag
);

INT32 UniCmdTPCDlAlgoCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8	u1TpcCmd,
	BOOLEAN	fgCmdCtrl,
	UINT8	u1DlTxType,
	CHAR	DlTxPwr,
	UINT8	u1EntryIdx,
	INT16	DlTxpwrAlpha
);

INT32 UniCmdTPCManTblInfo(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgUplink
);

INT32 UniCmdTPCWlanIdCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgUplink,
	UINT8   u1EntryIdx,
	UINT16  u2WlanId,
	UINT8 u1DlTxType
);

INT32 UniCmdTPCUlUtVarCfg(
	struct _RTMP_ADAPTER *pAd,
	UINT8	u1EntryIdx,
	UINT8	u1VarType,
	INT16	i2Value);

INT32 UniCmdTPCUlUtGo(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgTpcUtGo
);

INT32 UniCmdTPCEnableCfg(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgTpcEnable
);

INT32 UniCmdMecCtrl(
	struct _RTMP_ADAPTER *pAd,
	struct UNI_CMD_MEC_CTRL_PARAM_T *pParamCtrl
);

#ifdef CFG_SUPPORT_CSI
INT32 UniCmdCSICtrl(
	struct _RTMP_ADAPTER *pAd,
	struct CMD_CSI_CONTROL_T *prCSICtrl);
#endif /* CFG_SUPPORT_CSI */

#ifdef VLAN_SUPPORT
INT32 UniCmdVLANUpdate(
	struct _RTMP_ADAPTER *pAd,
	UCHAR band_idx,
	UINT8 omac_idx,
	UINT8 op_code,
	UINT16 value);
#endif /* VLAN_SUPPORT */

INT32 UniCmdThermalGetTemp(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1ActionIdx,
	UINT8  u1BandIdx,
	UINT32 *pu4SensorResult
);

INT32 UniCmdThermalGetAdcTemp(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1SensorType,
	UINT8  u1BandIdx,
	UINT8 *pu4SensorResult
);


INT32 UniCmdThermalManCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1BandIdx,
	UINT8  fgManualMode,
	UINT8  u1ThermalAdc
);

INT32 UniCmdThermalBasicInfo(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1BandIdx
);

INT32 UniCmdThermalTaskCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1BandIdx,
	UINT8  fgTrigEn,
	UINT8  u1Thres,
	UINT32 u4FuncPtr);

INT32 UniCmdThermalProtect(
	struct _RTMP_ADAPTER *pAd,
	UINT8  u1Band,
	UINT8  u1HighEn,
	CHAR cHighTempTh,
	UINT8  u1LowEn,
	CHAR cLowTempTh,
	UINT32 u4RechkTimer,
	UINT8  u1RFOffEn,
	CHAR cRFOffTh,
	UINT8  u1Type
);

INT32 UniCmdThermalProtectAdmitDutyInfo(
	struct _RTMP_ADAPTER *pAd
);

INT32 UniCmdThermalProtectEnable(
	struct _RTMP_ADAPTER *pAd,
	UINT8  band_idx,
	UINT8  protection_type,
	UINT8  trigger_type,
	INT32  trigger_temp,
	INT32  restore_temp,
	UINT16 recheck_time
);

INT32 UniCmdThermalProtectDisable(
	struct _RTMP_ADAPTER *pAd,
	UINT8  band_idx,
	UINT8  protection_type,
	UINT8  trigger_type
);

INT32 UniCmdThermalProtectDutyCfg(
	struct _RTMP_ADAPTER *pAd,
	UINT8  band_idx,
	UINT8  level_idx,
	UINT8  duty
);

INT32 UniCmdThermalProtectInfo(
	struct _RTMP_ADAPTER *pAd,
	UINT8  band_idx
);

INT32 UniCmdThermalProtectDutyInfo(
	struct _RTMP_ADAPTER *pAd,
	UINT8  band_idx
);

INT32
UniCmdThermalProtectStateAct(
	struct _RTMP_ADAPTER *pAd,
	UINT8  band_idx,
	UINT8  protect_type,
	UINT8  trig_type,
	UINT8  state
);

INT32
UniCmdSetNoiseFloorMode(
	struct _RTMP_ADAPTER *pAd,
	UINT8  Mode,
	UINT8  Action
);

INT32
UniCmdSetNoiseFloorControl(
	struct _RTMP_ADAPTER *pAd,
	UINT16  Tag,
	BOOLEAN  bEnable,
	UINT8  Timeout,
	UINT8  Count,
	UINT8  EventCount
);

enum _UNI_CMD_NF_TAG_T {
	UNI_CMD_NF_INFO = 0,
	UNI_CMD_NF_INFO_NUM
};

INT32 UniCmdSetCalMode(
	struct _RTMP_ADAPTER *pAd,
	UINT8  Mode,
	UINT8  Action
);

INT32 UniCmdVOWUpdate(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_VOW_PARAM_T pVOWParam,	BOOLEAN isSet, UINT8 McuDest,	VOID *pResult);

INT32 uni_cmd_vow_set_sta(PRTMP_ADAPTER pad, UINT16 sta_id, UINT32 subcmd);

INT uni_cmd_vow_set_drr_dbg(PRTMP_ADAPTER pad, UINT_32 dbg_mode);

INT uni_cmd_vow_set_sta_DWRR_max_time(PRTMP_ADAPTER pad);

INT uni_cmd_vow_set_group(PRTMP_ADAPTER pad, UINT8 group_id, UINT32 subcmd);

INT uni_cmd_vow_set_group_DWRR_max_time(PRTMP_ADAPTER pad);

INT uni_cmd_vow_set_feature_all(PRTMP_ADAPTER pad);

INT uni_cmd_vow_set_at_estimator(PRTMP_ADAPTER pad, UINT32 subcmd);

INT uni_cmd_vow_set_at_estimator_group(PRTMP_ADAPTER pad, UINT32 subcmd, UINT8 group_id);

INT uni_cmd_vow_set_rx_airtime(PRTMP_ADAPTER pad, UINT8 cmd, UINT32 subcmd);

INT uni_cmd_vow_set_wmm_selection(PRTMP_ADAPTER pad, UINT8 om);

INT uni_cmd_vow_set_mbss2wmm_map(PRTMP_ADAPTER pad, UINT8 bss_idx);

INT uni_cmd_vow_set_backoff_time(PRTMP_ADAPTER pad, UINT8 target);

INT uni_cmd_vow_get_rx_time_counter(PRTMP_ADAPTER pad, UINT8 target, UINT8 band_idx);

INT32 UniCmdSetRedEnable(RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 en);

VOID UniCmdExtEventRedTxReportHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);

INT32 UniCmdStateGetTxRxPhyRate(
	RTMP_ADAPTER *pAd,
	UINT16 u2StateCmdTag,
	UINT8 ucBandIdx
);

INT32 UniCmdPerStaGetRxRate(
	RTMP_ADAPTER *pAd,
	UINT16 u2PerStaCmdTag,
	UINT8 ucBandIdx,
	UINT16 u2Wcid,
	UINT32 *prRxRateInfo
);

INT32 UniCmdMultiStaGetRxRate(
	RTMP_ADAPTER *pAd,
	UINT16 u2PerStaCmdTag,
	UINT_16 *pu2Wcid,
	UINT32 *RxRatePair,
	UINT32 Num
);

INT32 UniCmdPerStaGetMultiRssi(
	RTMP_ADAPTER * pAd,
	UINT_8 *RssiPair,
	UINT32 Num
);

INT32 UniCmdPerStaGetRssi(
	RTMP_ADAPTER *pAd,
	UINT16 u2WlanIdx,
	UINT32 *rssi_rpt
);

INT32 UniCmdPerStaGetPER(
	RTMP_ADAPTER * pAd,
	UINT16 u2WlanIdx,
	UINT8 *u1PER
);

INT32 UniCmdStateGetCnInfo(
	RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT16 *u2cninfo
);

#ifdef SPECIAL_11B_OBW_FEATURE
INT32 UniCmdStateSetTxTdCck(
	RTMP_ADAPTER *pAd,
	UINT8 u1Enable
);
#endif

#ifdef TXRX_STAT_SUPPORT
INT32 UniCmdStateAllRateCntGet(
	RTMP_ADAPTER *pAd,
	UINT8 ucBandIdx,
	UINT8 ucDirection
);

INT32 UniCmdStateStbcCntGet(
	RTMP_ADAPTER *pAd,
	UINT8 ucBandIdx
);

INT32 UniCmdStateGiCntGet(
	RTMP_ADAPTER *pAd,
	UINT8 ucBandIdx
);
#endif /* TXRX_STAT_SUPPORT */

INT32 UniCmdStateGetTotalTxCnt(
	RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT32 *prStateResult
);

INT32 UniCmdStateGetLastTxRate(
	RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT32 *prStateResult
);

INT32 UniCmdStateGetRxStat(
	RTMP_ADAPTER *pAd
);

INT32 UniCmdPerStaGetSNR(
	RTMP_ADAPTER *pAd,
	UINT16 u2WlanIdx,
	UINT8 *u1PER
);

INT32 UniCmdPerStaGetTxRate(
	RTMP_ADAPTER *pAd,
	UINT_16 *pu2Wcid,
	UINT32 Num,
	BOOLEAN IsMultiRsp,
	UINT32 *prStateResult
);

INT32 UniCmdPerStaGetTxCnt(
	RTMP_ADAPTER *pAd,
	UINT_16 *pu2Wcid,
	UINT32 Num,
	BOOLEAN IsMultiRsp,
	UINT32 *prStateResult
);

INT32 UniCmdPerStaGetPktCnt(
	RTMP_ADAPTER *pAd,
	UINT_16 *pu2Wcid,
	UINT32 Num,
	BOOLEAN IsMultiRsp,
	UINT32 *prStateResult
);

INT32 UniCmdPerStaGetSN(
	RTMP_ADAPTER * pAd,
	UINT16 u2WlanIdx,
	UINT_16 *tid_sn_rpt
);

#ifdef LED_CONTROL_SUPPORT
INT32 UniCmdLedCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 led_index,
	UINT8 tx_over_blink,
	UINT8 reverse_polarity,
	UINT8 band,
	UINT8 blink_mode,
	UINT8 off_time,
	UINT8 on_time,
	UINT8 led_control_mode);

INT32 UniCmdLedGpio(
	struct _RTMP_ADAPTER *pAd,
	UINT8 led_index,
	UINT16 gpio_index,
	UINT8 gpio_ctrl_type);
#endif /* LED_CONTROL_SUPPORT */

INT32 UniCmdTestmodeCtrl(struct _RTMP_ADAPTER *pAd,
	struct _UNI_CMD_TEST_CTRL_T *prTestmodeCtrl,
	uint64_t u8TestCtrlFeature,
	struct _UNI_RSP_T *pucTestmodeCtrlRsp);

#if defined(PRE_CAL_MT7915_SUPPORT) || \
	defined(PRE_CAL_MT7986_SUPPORT) || defined(PRE_CAL_MT7916_SUPPORT) || \
	defined(PRE_CAL_MT7981_SUPPORT) || defined(PRE_CAL_BELLWETHER_SUPPORT) || \
	defined(PRE_CAL_MT7990_SUPPORT) || defined(PRE_CAL_MT7992_SUPPORT) || \
	defined(PRE_CAL_MT7993_SUPPORT)
INT32 UniCmdSetPreCalSetResult(
	RTMP_ADAPTER *pAd,
	UINT16 idx,
	UINT32 length,
	UINT32 u4CalId,
	UINT32 eeprom_ofst
);
#endif
/* defined(PRE_CAL_MT7915_SUPPORT) ||
*  defined(PRE_CAL_MT7986_SUPPORT) || defined(PRE_CAL_MT7916_SUPPORT) ||
*  defined(PRE_CAL_MT7981_SUPPORT) || defined(PRE_CAL_BELLWETHER_SUPPORT) ||
*  defined(PRE_CAL_MT7990_SUPPORT) || defined(PRE_CAL_MT7992_SUPPORT) ||
*  defined(PRE_CAL_MT7993_SUPPORT)
*/

#ifdef WIFI_UNIFIED_COMMAND
INT32 UniCmdTrigAssert(
	RTMP_ADAPTER *pAd,
	UINT8 u1Enable
);
#endif

#ifdef CONFIG_3_WIRE_SUPPORT
INT32 UniCmd3wireFunctionSet(
	struct _RTMP_ADAPTER *pAd,
	UINT8 threeWire_function_enable
);
#endif

INT32 uni_cmd_rro_rx_path(
	struct _RTMP_ADAPTER *ad,
	UINT8 rx_path_type);

INT32 uni_cmd_release_rro_setbl(
	struct _RTMP_ADAPTER *ad,
	UINT16 seid);

INT32 uni_cmd_rro_bypass_mode(
	struct _RTMP_ADAPTER *ad,
	UINT8 rro_bypass_type);
INT32 uni_cmd_rro_tx_free_done_path(
	struct _RTMP_ADAPTER *ad,
	UINT8 tx_free_done_path);
INT32 uni_cmd_rro_flush_timeout(
	struct _RTMP_ADAPTER *ad,
	UINT16 step_one_timeout,
	UINT16 flush_all_timeout);
INT32 uni_cmd_rro_rss_map(
	struct _RTMP_ADAPTER *ad,
	UINT32 rss_map0,
	UINT32 rss_map1);
INT32 uni_cmd_set_nic_cap_query(
	struct _RTMP_ADAPTER *ad);
INT32 uni_cmd_chip_config_dup_wtbl_range(
	struct _RTMP_ADAPTER *ad,
	UINT16 u2WtblDupBase,
	UINT16 u2WtblDupNumber);
INT32 uni_cmd_fr_table_add_for_duptx(
	struct _RTMP_ADAPTER *ad);
INT32 uni_cmd_fr_table_add(
	struct _RTMP_ADAPTER *ad,
	UINT8 rate_idx);
#ifdef CFG_SUPPORT_FALCON_SR
NDIS_STATUS UniSrCmd(IN PRTMP_ADAPTER pAd, UINT_8 u1CmdSubId, UINT_8 u1DbdcIdx, UINT_32 u4Value, UINT_8 isSet);
NDIS_STATUS UniSrCmdShow(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg, IN UINT_8 u1CmdSubId, IN UINT_8 u1ArgNum, UINT_8 isSet);
NDIS_STATUS UniCmdSRUpdateSrgBitmap(IN PRTMP_ADAPTER pAd, IN struct UNI_CMD_SR_SW_SRG_BITMAP_T prSrCmdSrSrgBitmap, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdSRUpdateGloVarSingleDropTa(IN PRTMP_ADAPTER pAd, IN struct UNI_CMD_SR_SW_GLOVAR_DROPTA_INFO_T *prSrCmdSrGlobalVarSingleDropTa, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdSRUpdateGloVarStaInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg, IN UINT_8 u1ArgNum, IN BOOLEAN isStaInit);
NDIS_STATUS UniCmdSRStaInfo(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_SW_GLOVAR_STA_INFO_T *prSrCmdSrGlobalVarStaInfo,  IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdShowSwSrTxCount(IN PRTMP_ADAPTER pAd, IN struct UNI_CMD_SR_SW_GLOVAR_STA_REC_INFO_T srCmdSrGlobalVarShowSrTxCount, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdShowGloVarSingleDropTa(IN PRTMP_ADAPTER pAd, IN struct UNI_CMD_SR_SR_GLOBAL_VAR_SINGLE_DROP_TA_T srCmdSrGlobalVarSingleDropTa, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdSRUpdateCap(IN PRTMP_ADAPTER pAd, IN struct UNI_CMD_SR_HW_CAP_T *prSrCmdSrCap, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdSRUpdatePara(IN PRTMP_ADAPTER pAd, struct UNI_SR_CMD_SR_PARA_T *prSrCmdSrPara, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdSRUpdateCondV2(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_COND_T_SR_V2 *prSrCmdSrCond, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdSRUpdateRcpiTbl(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_RCPITBL_T *prSrCmdSrRcpiTbl, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdSRUpdateRcpiTblOfstV2(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_RCPITBL_OFST_T_SR_V2 *prSrCmdSrRcpiTblOfst, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdSRUpdateQCtrl(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_Q_CTRL_T_SR_V2 *prSrCmdSrQCtrl, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdSRUpdateIBPD(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_IBPD_T *prSrCmdSrIBPD, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdSRUpdateNRT(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_NRT_T_SR_V2 *prSrCmdSrNRT, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdSRUpdateNRTCtrl(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_NRT_CTRL_T_SR_V2 *prSrCmdSrNRTCtrl, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdSRUpdateFNQCtrl(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_FNQ_CTRL_T *prSrCmdSrFNQCtrl, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdSRUpdateInterPsCtrl(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_INTERPS_CTRL_T *prSrCmdSrInterPsCtrl, IN UINT_8 u1DbdcIdx);

NDIS_STATUS UniCmdSRUpdateSiga(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_SIGA_FLAG_T *prSrCmdSrSigaFlag, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdSRUpdateSigaAuto(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_SIGA_AUTO_FLAG_T *prSrCmdSrSigaAutoFlag, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdMeshTopologyUpd(IN PRTMP_ADAPTER pAd, struct UNI_CMD_MESH_TOPOLOGY_T *prSrCmdMeshTopology, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdMeshUplinkStatusSet(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_UL_TRAFFIC_STATUS_T *prSrCmdMeshTopology, IN UINT_8 u1DbdcIdx);
NDIS_STATUS UniCmdMeshMapBalance(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SET_MAP_BALANCE_T *prSrCmdMapBalance, IN UINT_8 u1DbdcIdx);
#endif/* CFG_SUPPORT_FALCON_SR */
#ifdef RT_CFG80211_SUPPORT
#endif
NDIS_STATUS UniCmdMldRec(RTMP_ADAPTER *pAd, UINT32 mldRecIdx);
NDIS_STATUS UniCmdMloAgcTx(PRTMP_ADAPTER pAd, struct UNI_CMD_MLO_MLD_REC_LINK_AGC_TX_T *prMloAgcTx);
NDIS_STATUS UniCmdMloAgcTrig(PRTMP_ADAPTER pAd, struct UNI_CMD_MLO_MLD_REC_LINK_AGC_TRIG_T *prMloAgcTrig);
#ifdef PRE_CFG_SUPPORT
INT32 UniCmdPreCfgSetCmdByRaw(
	struct _RTMP_ADAPTER *pAd,
	UINT8 DestType,
	UINT32 UniCmdId,
	UCHAR *pCmdData,
	UINT32 CmdSize
);
#endif /* PRE_CFG_SUPPORT */

#ifdef DOT11_EHT_BE
NDIS_STATUS UniCmdMloConfigOperation(struct _RTMP_ADAPTER *pAd, UINT8 ucStrBitmap);
#endif /* DOT11_EHT_BE */
NDIS_STATUS UniCmdMloOptionCtrl(PRTMP_ADAPTER pAd, struct UNI_CMD_MLO_OPTION_CTRL_T *prMloOptionCtrl);
INT32 UniCmdRASandBoxCtrlProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING * arg);
INT32 UniCmdSandBoxCtrl(
	struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	UINT32 u4EnableFeature);
#ifdef FTM_SUPPORT
INT32 UniCmdFTM(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 u4Action, UINT32 u4SetValue, UINT8 ucDbdcIdx, PFTM_PEER_ENTRY pEntry);
#endif /* FTM_SUPPORT */
INT32 UniCmdGetSdo(IN struct _RTMP_ADAPTER *pAd, IN UINT32 arg0, IN UINT32 arg1, IN UINT32 arg2, struct _CR4_QUERY_STRUC *cr4_query_list);
INT32 UniCmdSetSdo(IN struct _RTMP_ADAPTER *pAd, IN UINT32 arg0, IN UINT32 arg1, IN UINT32 arg2, IN UINT32 u4ExtSize, IN PUINT8 pExtData);
INT32 UniCmdSetSdoAutoBA(
	IN struct _RTMP_ADAPTER *pAd,
	IN BOOLEAN Enable,
	IN UINT32 Timeout);
#endif /* WIFI_UNIFIED_COMMAND */
INT32 UniCmdSetLpi(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx, UINT8 ucLpiEnable, UINT8 ucPSDLimit);
INT32 UniCmdStaRecWTBLHeaderTrans(struct _RTMP_ADAPTER *pAd, UINT32 wcid, UINT8 DIS_RHTR);
#endif /* _CMM_FW_UNI_CMD_H */
