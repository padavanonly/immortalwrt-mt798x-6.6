/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: wo interface definitions
	warp_woif.h
*/

#ifndef _WARP_WOIF_H_
#define _WARP_WOIF_H_

#include "warp_utility.h"

#define MAX_WO_PROF_LVL	(10)

#define WOIF_UNDEFINED 0
#define WOIF_DISABLE_DONE 1
#define WOIF_TRIGGER_ENABLE 2
#define WOIF_ENABLE_DONE 3
#define WOIF_TRIGGER_GATING 4
#define WOIF_GATING_DONE 5
#define WOIF_TRIGGER_HALT 6
#define WOIF_HALT_DONE 7

#define WOCPU_TIMEOUT     ((1000 * HZ)/1000)    /* 1s*/

/* cmd which sends to wo */
enum wo_cmd_id {
	WO_CMD_WED_START = 0x0000,
	WO_CMD_WED_CFG  = WO_CMD_WED_START,
	WO_CMD_WED_RX_STAT = 0x0001,
	WO_CMD_RRO_SER = 0x0002,
	WO_CMD_DBG_INFO = 0x0003,
	WO_CMD_DEV_INFO = 0x0004,
	WO_CMD_BSS_INFO = 0x0005,
	WO_CMD_STA_REC = 0x0006,
	WO_CMD_DEV_INFO_DUMP = 0x0007,
	WO_CMD_BSS_INFO_DUMP = 0x0008,
	WO_CMD_STA_REC_DUMP = 0x0009,
	WO_CMD_BA_INFO_DUMP = 0x000A,
	WO_CMD_FBCMD_Q_DUMP = 0x000B,
	WO_CMD_FW_LOG_CTRL = 0x000C,
	WO_CMD_LOG_FLUSH = 0x000D,
	WO_CMD_CHANGE_STATE = 0x000E,
	WO_CMD_CPU_STATS_ENABLE = 0x000F,
	WO_CMD_CPU_STATS_DUMP = 0x0010,
	WO_CMD_EXCEPTION_INIT = 0x0011,
	WO_CMD_PROF_CTRL = 0x0012,
	WO_CMD_STA_BA_DUMP = 0x0013,
	WO_CMD_BA_CTRL_DUMP = 0x0014,
	WO_CMD_RXCNT_CTRL = 0x0015,
	WO_CMD_RXCNT_INFO = 0x0016,
	WO_CMD_SET_CAP = 0x0017,
	WO_CMD_CCIF_RING_DUMP = 0x0018,
	WO_CMD_WED_END
};

enum wo_event_id {
	WO_EVT_LOG_DUMP = 0x0001,
	WO_EVT_PROFILING = 0x0002,
	WO_EVT_RXCNT_INFO = 0x0003
};

enum wo_state {
	WO_STATE_UNDEFINED 	= 0x0,
	WO_STATE_INIT 		= 0x1,
	WO_STATE_ENABLE		= 0x2,
	WO_STATE_DISABLE	= 0x3,
	WO_STATE_HALT		= 0x4,
	WO_STATE_GATING		= 0x5,
	WO_STATE_SER_RESET 	= 0x6,
	WO_STATE_WF_RESET	= 0x7,
	WO_STATE_END
};

/* WO_CMD_WED_CFG */
enum wo_ring_type {
	WO_MIOD_RING = 0,
	WO_FBCMD_RING,
	WO_RING_TYPE_END
};

#ifdef WED_RX_D_SUPPORT
/* WO_CMD_WED_CFG */
struct wo_cmd_ring_para {
#if (WED_HWRRO_MODE == WED_HWRRO_MODE_WOCPU)
	u32 baseaddr;
#else
	unsigned long baseaddr;
#endif
	u32	maxcnt;
	u32	unit_sz;
};

/* WO_CMD_WED_CFG */
struct wo_cmd_wedcfg_para {
	struct wo_cmd_ring_para ring[WO_RING_TYPE_END];
#if (WED_HWRRO_MODE == WED_HWRRO_MODE_WOCPU)
	u32 wed_baseaddr;
#else
	unsigned long wed_baseaddr;
#endif
	u8 mac_ver;
};
#endif

/* WO_CMD_WED_RX_STAT */
struct wo_cmd_rxstat_para {
	u32	rx_err_cnt;
	u32	rx_drop_cnt;
	u32	rx_rev_cnt;
	u32	rx_ack_cnt;
	u32	prof_record[MAX_WO_PROF_LVL*2];
};

enum {
	DEVINFO_ACTIVE = 0,
	DEVINFO_MAX_NUM = 1,
};

struct __attribute__((packed)) _CMD_DEVINFO_UPDATE_T {
	u8  ucOwnMacIdx;
	u8  ucBandIdx;
	u16 u2TotalElementNum;
	u8  ucAppendCmdTLV;
	u8  aucReserve[3];
	u8  aucBuffer[];
};;

struct __attribute__((packed)) _CMD_DEVINFO_ACTIVE_T {
	/* Device information (Tag0) */
	u16 u2Tag;		/* Tag = 0x00 */
	u16 u2Length;
	u8  ucActive;
	u8  ucDbdcIdx;
	u8  aucOwnMAC[6];
};

enum {
	BSS_INFO_OWN_MAC = 0,
	BSS_INFO_BASIC = 1,
	BSS_INFO_RF_CH = 2,
	BSS_INFO_PM = 3,
	BSS_INFO_UAPSD = 4,
	BSS_INFO_ROAM_DETECTION = 5,
	BSS_INFO_LQ_RM = 6,
	BSS_INFO_EXT_BSS = 7,
	BSS_INFO_BROADCAST_INFO = 8,
	BSS_INFO_SYNC_MODE = 9,
	BSS_INFO_RA = 10,
	BSS_INFO_HW_AMSDU = 11,
	BSS_INFO_BSS_COLOR = 12,
	BSS_INFO_HE_BASIC = 13,
	BSS_INFO_PROTECT_INFO = 14,
	BSS_INFO_BCN_OFFLOAD = 15,
	BSS_INFO_11V_MBSSID = 16,
	BSS_INFO_MAX_NUM
};

struct __attribute__((packed)) _CMD_BSSINFO_UPDATE_T {
	u8  ucBssIndex;
	u8  ucReserve;
	u16 u2TotalElementNum;
	u8  ucAppendCmdTLV;
	u8  aucReserve[3];
	u8  aucBuffer[];
};

struct __attribute__((packed)) _BSSINFO_CONNECT_OWN_DEV_T {
	u16 u2Tag;		/* Tag = 0x00 */
	u16 u2Length;
	u8  ucHwBSSIndex;
	u8  ucOwnMacIdx;
	u8  ucBandIdx;
	u8  aucReserve;
	u32 u4ConnectionType;
	u32 u4Reserved;
};

struct __attribute__((packed)) _BSSINFO_BASIC_T {
	u16	u2Tag;		/* Tag = 0x01 */
	u16 u2Length;
	u32	u4NetworkType;
	u8  ucActive;
	u8  ucReserve0;
	u16	u2BcnInterval;
	u8  aucBSSID[6];
	u8  ucWmmIdx;
	u8  ucDtimPeriod;
	u8  ucBmcWlanIdxL;			/* indicate which wlan-idx used for MC/BC transmission. */
	u8  ucCipherSuit;
	u8  ucPhyMode;
	u8  uc11vMaxBssidIndicator;	/* Max BSSID indicator. Range from 1 to 8, 0 means MBSSID disabled */
	u8  uc11vBssidIdx;			/* BSSID index of non-transmitted BSSID, 0 means transmitted BSSID */
	u8  ucBmcWlanIdxHnVer;		/* High Byte and Version */
	u8  acuReserve[2];
};

enum {
	STA_REC_BASIC_STA_RECORD = 0,
	STA_REC_RA = 1,
	STA_REC_RA_COMMON_INFO = 2,
	STA_REC_RA_UPDATE = 3,
	STA_REC_BF = 4,
	STA_REC_AMSDU = 5,
	STA_REC_BA = 6,
	STA_REC_RED = 7,
	STA_REC_TX_PROC = 8,
	STA_REC_BASIC_HT_INFO = 9,
	STA_REC_BASIC_VHT_INFO = 10,
	STA_REC_AP_PS = 11,
	STA_REC_INSTALL_KEY = 12,
	STA_REC_WTBL = 13,
	STA_REC_BASIC_HE_INFO = 14,
	STA_REC_HW_AMSDU = 15,
	STA_REC_WTBL_AADOM = 16,
	STA_REC_INSTALL_KEY_V2 = 17,
	STA_REC_MURU = 18,
	STA_REC_MAX_NUM
};

struct __attribute__((packed)) _CMD_STAREC_UPDATE_T {
	u8  ucBssIndex;
	u8  ucWlanIdxL;		/* #256STA - Low Byte */
	u16 u2TotalElementNum;
	u8  ucAppendCmdTLV;
	u8  ucMuarIdx;
	u8  ucWlanIdxHnVer;		/* #256STA - High Byte and Version */
	u8  aucReserve;
	u8  aucBuffer[];
};

struct __attribute__((packed)) _STAREC_COMMON_T {
	/* Basic STA record (Group0) */
	u16 u2Tag;		/* Tag = 0x00 */
	u16 u2Length;
	u32 u4ConnectionType;
	u8  ucConnectionState;
	u8	ucIsQBSS;
	u16	u2AID;
	u8  aucPeerMacAddr[6];
	/*This is used especially for 7615 to indicate this STAREC is to create new one or simply update
	In some case host may send new STAREC without delete old STAREC in advance. (ex: lost de-auth or get assoc twice)
	We need extra info to know if this is a brand new STAREC or not
	Consider backward compatibility, we check bit 0 in this reserve.
	Only the bit 0 is on, N9 go new way to update STAREC if bit 1 is on too.
	If neither bit match, N9 go orinal way to update STAREC. */
	u16	u2ExtraInfo;
};

enum _BA_SESSION_TYPE {
	BA_SESSION_INV = 0,
	BA_SESSION_ORI = 1,
	BA_SESSION_RECP = 2,
};

struct __attribute__((packed)) _STAREC_BA_T {
	u16 u2Tag;       /* Tag = 0x06 */
	u16 u2Length;
	u8  ucTid;
	u8  ucBaDirection;
	u8  ucAmsduCap;
	u8  ucBaEenable;
	u16 u2BaStartSeq;
	u16 u2BaWinSize;
};

struct __attribute__((packed)) wo_cmd_query {
	u32 query_arg0;
	u32 query_arg1;
};

#endif /*_WARP_WOIF_H_*/
