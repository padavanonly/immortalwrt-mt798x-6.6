/*
 * Copyright (c) [2020], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/
/*
 ***************************************************************************
 ***************************************************************************

	Module Name:
	band_steering_def.h
*/

#ifndef _BS_DEF_H_
#define __BS_DEF_H__

#ifdef CCN67_BS_SUPPORT

#define BS_MINSTA_THRESH_DEFAULT 20
#define BS_MAX_TABLE_SIZE	64
/* #define SIZE_OF_VHT_CAP_IE		12 */
#define IS_5G_BAND(_p)			(((_p)&BAND_5G) == BAND_5G)

struct bs_cli_entry {
	BOOLEAN bValid;
	BOOLEAN bConnStatus;
	BOOLEAN isBlackListed;
	UINT32 withhold_limit;		/*default is 3 */
	ULONG withhold_age;		/* default is 1 to 30 second how to update this */
	UINT32 withhold_count;
	ULONG withhold_time;
	UINT8 probe_withhold;
	UINT8 auth_withhold;
	UINT8	TableIndex;
	UINT32 Control_Flags;
	ULONG   entryTime;		/* timestamp when insert-entry */
	UINT32  elapsed_time; /* ms */
	UCHAR Addr[MAC_ADDR_LEN];
	CHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR SsidLen;
	UCHAR Rssi;
	UCHAR BS_Sta_State;
	UINT32 Auth_Count;
	UINT32 Probe_Count;
	struct bs_cli_entry *pNext;
};

struct bs_entry_list {
	struct bs_entry_list *pNext;
	UCHAR addr[MAC_ADDR_LEN];
	UCHAR state;
};

struct bs_cli_table {
	BOOLEAN bInitialized;
	BOOLEAN bEnabled;
	UINT32 Size;
	struct bs_cli_entry Entry[BS_MAX_TABLE_SIZE];
	struct bs_cli_entry *Hash[HASH_TABLE_SIZE];
	NDIS_SPIN_LOCK Lock;
	VOID *priv;
	BOOLEAN bInfReady;
	CHAR	ucIfName[32];
	UINT8	uIdx;
	UINT8		Band;
	UINT8		Channel;
};

struct ieee80211req_bba_stats {
	UINT32   btm_request_sent;
	UINT32   btm_response[10];
};

enum BS_RETURN_CODE {
	BS_SUCCESS = 0,
	BS_INVALID_ARG,
	BS_RESOURCE_ALLOC_FAIL,
	BS_TABLE_FULL,
	BS_TABLE_IS_NULL,
	BS_NOT_INITIALIZED,
	BS_2G_INF_NOT_READY,
	BS_5G_INF_NOT_READY,
	BS_STA_IS_CONNECTED,
	BS_UNEXP,
};

enum BS_STA_STATE {
	BS_STA_INIT = 0,
	BS_STA_ASSOC,
	BS_STA_DISASSOC,
};


#endif /* CCN67_BS_SUPPORT */
#endif /* _BS_DEF_H_ */

