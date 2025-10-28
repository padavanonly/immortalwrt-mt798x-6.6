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
#ifndef __PRE_CFG_H__
#define __PRE_CFG_H__
#ifdef PRE_CFG_SUPPORT
#include "rt_config.h"

struct PRECFG_FAKE_NEGO_PARAM;
struct PRECFG_CMD;
struct PRECFG_CTRL;
struct PRECFG_PEER_OP;
struct PRECFG_ENTRY;
struct PRECFG_TIMER_CMD_CTRL;
union  PRECFG_DATA;

/*******************************************************************************
 * Macro/Enum/Define
 *******************************************************************************/
#define PRE_CFG_ON_NONE   	0
#define PRE_CFG_ON_AP		1
#define PRE_CFG_ON_STA		2

#define PRE_CFG_ENTRY_PEER_NONE   	0
#define PRE_CFG_ENTRY_PEER_AP		1
#define PRE_CFG_ENTRY_PEER_STA		2

#define IS_PRE_CFG_ENTRY_VALID(_x)		((_x)->fgValid == TRUE)
#define IS_PRE_CFG_ENTRY(_x)			((_x)->u1EntryPeer != PRE_CFG_ENTRY_PEER_NONE)
#define IS_PRE_CFG_ENTRY_PEER_AP(_x)	((_x)->u1EntryPeer == PRE_CFG_ENTRY_PEER_AP)
#define IS_PRE_CFG_ENTRY_PEER_STA(_x)	((_x)->u1EntryPeer == PRE_CFG_ENTRY_PEER_STA)

#define GET_PRE_CFG_ENTRY_PEER(_x)			((_x)->u1EntryPeer)
#define SET_PRE_CFG_ENTRY_PEER(_x, _type)	((_x)->u1EntryPeer = _type)

enum {
	PRE_CFG_NONE                          = 0,
	PRE_CFG_UNI_CMD_STAREC_BASIC          = ((UINT64)1 << 0),
	PRE_CFG_UNI_CMD_STAREC_AAD_OM         = ((UINT64)1 << 1),
	PRE_CFG_UNI_CMD_STAREC_HDRT           = ((UINT64)1 << 2),
	PRE_CFG_UNI_CMD_STAREC_PHY_INFO       = ((UINT64)1 << 3),
	PRE_CFG_UNI_CMD_STAREC_HW_AMSDU       = ((UINT64)1 << 4),
	PRE_CFG_UNI_CMD_STAREC_HT_BASIC       = ((UINT64)1 << 5),
	PRE_CFG_UNI_CMD_STAREC_VHT_BASIC      = ((UINT64)1 << 6),
	PRE_CFG_UNI_CMD_STAREC_HE_BASIC       = ((UINT64)1 << 7),
	PRE_CFG_UNI_CMD_STAREC_EHT_BASIC      = ((UINT64)1 << 8),
	PRE_CFG_UNI_CMD_STAREC_RA             = ((UINT64)1 << 9),
	PRE_CFG_UNI_CMD_STAREC_MURU_INFO      = ((UINT64)1 << 10),
	PRE_CFG_UNI_CMD_STAREC_MLD_SETUP      = ((UINT64)1 << 11),
	PRE_CFG_UNI_CMD_STAREC_EHT_MLD	      = ((UINT64)1 << 12),
	PRE_CFG_UNI_CMD_STAREC_INSTALL_KEY_V2 = ((UINT64)1 << 13),
	PRE_CFG_UNI_CMD_STAREC_MLD_TEARDOWN   = ((UINT64)1 << 35),
	};

#define BASIC_CONNECT_FLAGS (\
	PRE_CFG_UNI_CMD_STAREC_BASIC     |\
	PRE_CFG_UNI_CMD_STAREC_AAD_OM    |\
	PRE_CFG_UNI_CMD_STAREC_HDRT      |\
	PRE_CFG_UNI_CMD_STAREC_PHY_INFO  |\
	PRE_CFG_UNI_CMD_STAREC_HW_AMSDU  |\
	PRE_CFG_UNI_CMD_STAREC_HT_BASIC  |\
	PRE_CFG_UNI_CMD_STAREC_VHT_BASIC |\
	PRE_CFG_UNI_CMD_STAREC_HE_BASIC  |\
	PRE_CFG_UNI_CMD_STAREC_EHT_BASIC |\
	PRE_CFG_UNI_CMD_STAREC_MURU_INFO)

#define PRE_CFG_CMD_BUF_MAX 2048
#define PRE_CFG_TAG_MAX 16
#define PRE_CFG_TAG_END 0

#define TAG_END \
{\
	.u8CmdFeature = PRE_CFG_TAG_END,\
	.pfPrepareHandler = NULL,\
	.pfUpdateHandler = NULL \
}

/* This section is for PreCfg Config Set definition */
#define PRECFG_CFG_SET_BUF_MAX 32

enum PRECFG_CFGSET_ID {
	PRECFG_CFGSET_BASIC      = 0,
	PRECFG_CFGSET_RA         = 1,
	PRECFG_CFGSET_MLD        = 2, // MLD
	PRECFG_CFGSET_SEC        = 3, // Security add key
	PRECFG_CFGSET_RAND_AMSDU = 4, // Random amsdu configuration
	PRECFG_CFGSET_RAND_RADIO = 5, // Random radio on & off
	PRECFG_CFGSET_MAX
};

struct GNU_PACKED PRE_CFG_TAG_HANDLE {
	UINT64 u8CmdFeature;

	NTSTATUS(*pfPrepareHandler)(struct _RTMP_ADAPTER *pAd,
								struct PRECFG_CTRL *pPreCfgCtrl,
								struct PRECFG_ENTRY *pPreCfgEntry,
								BOOLEAN fgConnect);

	NTSTATUS(*pfUpdateHandler)(struct _RTMP_ADAPTER *pAd,
								struct PRECFG_CTRL *pPreCfgCtrl,
								struct PRECFG_ENTRY *pPreCfgEntry,
								BOOLEAN fgConnect,
								UCHAR * *pCmdBuf,
								UINT16 * u2TLVNumber);
};

struct GNU_PACKED PRECFG_CFG_SET {
	UINT16 u2CfgSetID;
	UINT64 u8CmdFeature;
	UCHAR *pucDescription;

	NTSTATUS(*pfCommHandler)(struct _RTMP_ADAPTER *pAd,
							struct PRECFG_CTRL *pPreCfgCtrl,
							struct PRECFG_ENTRY *pPreCfgEntry,
							BOOLEAN fgConnect,
							UCHAR **pCmdBuf);

	NTSTATUS(*pfSendHandler)(struct _RTMP_ADAPTER *pAd,
							UINT16 u2TLVTotalNumber,
							UCHAR *pCmdData,
							UINT32 CmdSize);

	VOID(*pfTimerHandler)(struct PRECFG_TIMER_CMD_CTRL *pPreCfgTimerCmdCtrl);

	struct PRE_CFG_TAG_HANDLE pfHandler[PRE_CFG_TAG_MAX];
};

struct GNU_PACKED pre_cfg_proc_tbl {
	UINT8 u1PreCfgType;
	UCHAR *target;
	struct PRECFG_CMD *cmd_tbl;
};

/*******************************************************************************
 * Struct Definition
 *******************************************************************************/
struct GNU_PACKED PRECFG_FAKE_NEGO_PARAM
{
	PUCHAR	pParamStr;
	UINT32	u4Value;
	BOOLEAN fgIsCfgDone;
};

/*
 * Table entry of precfg command handler
 */
struct GNU_PACKED PRECFG_CMD {
	UCHAR *action;

	INT(*action_handler)(struct _RTMP_ADAPTER *pAd,
						RTMP_STRING * arg,
						UINT8 u1InfIdx);
};

/* Test data for precfg dvt */
union GNU_PACKED PRECFG_DATA {
	struct {
		NDIS_SPIN_LOCK PreCfgEntryLock;
		struct _DL_LIST EntryList;
		struct _BSS_STRUCT *pMbss;
		struct ml_ie_info ml_info;
		BOOLEAN fgSecurity;
	} ApData;
#ifdef APCLI_SUPPORT
	struct {
		BOOLEAN fgValid;
		BOOLEAN fgSecurity;
		UINT16 u2MldStaIdx;
		UCHAR ApMacAddr[MAC_ADDR_LEN];
		struct _STA_ADMIN_CONFIG *pStaCfg;
		struct PRECFG_ENTRY *pPreCfgEntry;
		struct ml_ie_info ml_info;
	} StaData;
#endif /* APCLI_SUPPORT */
};

/*
 * Function pointers for precfg operations
 */
struct GNU_PACKED PRECFG_PEER_OP {
	NTSTATUS (*peer_sys_init)(IN struct _RTMP_ADAPTER *pAd,
							IN struct PRECFG_CTRL *pPreCfgCtrl,
							IN UCHAR *pMacAddr,
							IN UINT16 u2MldStaIdx,
							IN UINT16 *pu2CfgSetInput,
							IN UINT16 u2CfgSetCount);
	NTSTATUS (*peer_fw_update)(IN struct _RTMP_ADAPTER *pAd,
							IN struct PRECFG_CTRL *pPreCfgCtrl,
							IN struct PRECFG_ENTRY *pPreCfgEntry,
							IN BOOLEAN fgConnect);
	NTSTATUS (*sys_link_up)(IN struct _RTMP_ADAPTER *pAd,
							IN struct PRECFG_CTRL *pPreCfgCtrl,
							IN struct PRECFG_ENTRY *pPreCfgEntry,
							IN struct _MAC_TABLE_ENTRY *pMacEntry);
	NTSTATUS (*sys_link_down)(IN struct _RTMP_ADAPTER *pAd,
							IN struct PRECFG_CTRL *pPreCfgCtrl,
							IN struct PRECFG_ENTRY *pPreCfgEntry);
	NTSTATUS (*peer_connect)(IN struct _RTMP_ADAPTER *pAd,
							IN struct PRECFG_CTRL *pPreCfgCtrl,
							IN struct PRECFG_ENTRY *pPreCfgEntry);
	NTSTATUS (*peer_disconnect)(IN struct _RTMP_ADAPTER *pAd,
							IN struct PRECFG_CTRL *pPreCfgCtrl,
							IN struct PRECFG_ENTRY *pPreCfgEntry);
	NTSTATUS (*peer_sys_conn_act)(IN struct _RTMP_ADAPTER *pAd,
							IN struct PRECFG_CTRL *pPreCfgCtrl,
							IN struct PRECFG_ENTRY *pPreCfgEntry);
	NTSTATUS (*peer_sys_disconn_act)(IN struct _RTMP_ADAPTER *pAd,
							IN struct PRECFG_CTRL *pPreCfgCtrl,
							IN struct PRECFG_ENTRY *pPreCfgEntry,
							IN struct _MAC_TABLE_ENTRY *pMacEntry);
};

struct GNU_PACKED PRECFG_ENTRY {
	BOOLEAN fgValid;
	BOOLEAN fgConnect;
	UINT8   u1EntryPeer;
	UINT16  mld_sta_idx;
	UINT16  u2CfgSetCount;
	UINT16  u2ArrCfgSetInput[PRECFG_CFG_SET_BUF_MAX];
	struct  _DL_LIST List; /* Only for peer is STA */
	struct  _STA_REC_CFG StaRecCfg;
	struct  _MAC_TABLE_ENTRY *pMacEntry;
	struct  PRECFG_CTRL *pPreCtrl;
};

struct GNU_PACKED PRECFG_TIMER_CMD_CTRL {
	struct _DL_LIST List;
	struct _RTMP_ADAPTER *pAd;
	struct PRECFG_ENTRY *pPreCfgEntry;
	UINT16 u2CfgSetIdx;
	UINT16 u2TLVTotalNumber;
	UCHAR *pCmdBuf;
	UINT32 u4RealUsedBufSize;
	RALINK_TIMER_STRUCT PreCfgTimer;
	VOID(*pfTimerHandler)(struct PRECFG_TIMER_CMD_CTRL *pPreCfgTimerCmdCtrl);

};

/*
 * Control info for precfg dvt
 * For AP role		: _BSS_STRUCT.PRECFG_CTRL
 * For Client role	: _STA_ADMIN_CONFIG.PRECFG_CTRL
 */
struct GNU_PACKED PRECFG_CTRL {
	BOOLEAN fgEnable;
	UINT8 u1PreCfgType;
	union PRECFG_DATA PreCfgData;
	struct PRECFG_PEER_OP PeerOp;
	VOID *pPriv;
	struct _DL_LIST TimerList;
};

/*******************************************************************************
 * Functions
 *******************************************************************************/
#ifdef WIFI_UNIFIED_COMMAND
VOID UniCmdPreCfgResultRsp(
	struct cmd_msg *msg,
	char *payload,
	UINT16 payload_len);
#endif /* WIFI_UNIFIED_COMMAND */

BOOLEAN pre_cfg_interface_init(
	IN struct _RTMP_ADAPTER *pAd,
	IN INT8 InfIdx,
	IN BOOLEAN fgApRole);

BOOLEAN pre_cfg_interface_deinit(
	IN struct _RTMP_ADAPTER *pAd,
	IN INT8 InfIdx,
	IN BOOLEAN fgApRole);

VOID pre_cfg_free_peer_all_sta(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _BSS_STRUCT *pMbss);

INT	Set_PreCfg_Peer_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg);

INT Set_PreCfg_Negotiation_Param_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg);

/* Command handlers for ap_set_pre_cfg_tbl (Start) */
INT pre_cfg_add_peer_mld_sta_info(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx);

INT pre_cfg_add_peer_sta(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx);

INT pre_cfg_del_peer_sta(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx);

INT pre_cfg_connect_peer_sta(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx);

INT pre_cfg_disconnect_peer_sta(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx);
/* Command handlers for ap_set_pre_cfg_tbl (End) */

/* Command handlers for sta_set_pre_cfg_tbl (Start) */
#ifdef APCLI_SUPPORT
INT pre_cfg_add_peer_mld_ap_info(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx);

INT pre_cfg_add_peer_ap(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx);

INT pre_cfg_del_peer_ap(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx);

INT pre_cfg_connect_peer_ap(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx);

INT pre_cfg_disconnect_peer_ap(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx);
#endif /* APCLI_SUPPORT */
/* Command handlers for sta_set_pre_cfg_tbl (End) */

/* Operation handlers for PRECFG_PEER_OP */
NTSTATUS pre_cfg_comm_fw_update(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect);

/* Function to load PreCfg Configuration Set */
NTSTATUS PreCfg_Load_Config_Set(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry);

/* Function to do link up */
NTSTATUS PreCfg_Do_LinkUp(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry);

/* Function to do connection */
NTSTATUS PreCfg_Do_Connection(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry);

#endif /* PRE_CFG_SUPPORT */
#endif /* __PRE_CFG_H__ */
