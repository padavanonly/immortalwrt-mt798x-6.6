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
	hw_ctrl.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __HW_CTRL_H__
#define __HW_CTRL_H__
#include "rtmp_type.h"
#include "rtmp_os.h"
#include "security/wpa_cmm.h"
#include "protocol/protection.h"


struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _EDCA_PARM;
struct _WMM_CFG;
struct wifi_dev;
struct _BSS_INFO_ARGUMENT_T;
struct _MAC_TABLE_ENTRY;
struct WIFI_SYS_CTRL;
struct _STA_ADMIN_CONFIG;
struct TWT_AGRT_PARA_T;
struct TWT_AGRT_MGMT_T;
struct physical_device;


typedef NTSTATUS(*HwCmdCb)(struct _RTMP_ADAPTER *pAd, VOID * Args);


#define HWCTRL_CMD_TIMEOUT 100
#define HWCTRL_CMD_WAITTIME 2000
#define ETSI_RXBLOCKER4R 4
#define ETSI_RXBLOCKER1R 1
#define HWCTRL_MCAST_ADDR_LEN 16 /* align with IPV6_ADDR_LEN */

#ifdef SW_CONNECT_SUPPORT
#define MAX_LEN_OF_HWCTRL_QUEUE(_pAd)            (WTBL_MAX_NUM(_pAd)<<2)
#else /* SW_CONNECT_SUPPORT */
#define MAX_LEN_OF_HWCTRL_QUEUE(_pAd)            (WTBL_MAX_NUM(_pAd)<<1)
#endif /* !SW_CONNECT_SUPPORT */

#if (NR_CPUS > 1)
#define HWCTRL_QUE_SCH				32
#else
#define HWCTRL_QUE_SCH				16
#endif

/*for command classify*/
enum {
	HWCMD_TYPE_FIRST = 0,
	HWCMD_TYPE_RADIO = HWCMD_TYPE_FIRST, /*Need Radio Resource Mgmt Related*/
	HWCMD_TYPE_SECURITY,					 /*Security related*/
	HWCMD_TYPE_PERIPHERAL,				/*Peripheral related*/
	HWCMD_TYPE_HT_CAP,						/*HT related*/
	HWCMD_TYPE_PS,							/*Power Saving related*/
	HWCMD_TYPE_WIFISYS,
	HWCMD_TYPE_WMM,
	HWCMD_TYPE_PROTECT,
	HWCMD_TYPE_WA,
	HWCMD_TYPE_WA_MCAST,
	HWCMD_TYPE_END
};


/*for command ID*/
enum {
	HWCMD_ID_FIRST = 0,
	/*Peripheral*/
	HWCMD_ID_GPIO_CHECK			= HWCMD_ID_FIRST,
	/*WSC & LED related*/
	HWCMD_ID_SET_LED_STATUS			= 3,
	HWCMD_ID_LED_WPS_MODE10			= 4,
	HWCMD_ID_LED_GPIO_MAP			= 5,
	/*Security related*/
	HWCMD_ID_DEL_ASIC_WCID			= 7,
#ifdef HTC_DECRYPT_IOT
	HWCMD_ID_SET_ASIC_AAD_OM		= 8,
#endif /* HTC_DECRYPT_IOT */
	HWCMD_ID_ADDREMOVE_ASIC_KEY		= 9,
	/*MT_MAC */
	HWCMD_ID_PS_CLEAR				= 11,
	HWCMD_ID_PS_RETRIEVE_START		= 12,
	HWCMD_ID_UPDATE_DAW_COUNTER		= 14,
	HWCMD_ID_UPDATE_BEACON			= 15,
	HWCMD_ID_GET_TEMPERATURE		= 16,
	HWCMD_ID_SET_SLOTTIME			= 17,
	HWCMD_ID_SET_TX_BURST			= 18,
#ifdef ERR_RECOVERY
	HWCMD_ID_MAC_ERROR_DETECT		= 23,
#endif /* ERR_RECOVERY */
	/*AP realted*/
	HWCMD_ID_AP_ADJUST_EXP_ACK_TIME	= 24,
	HWCMD_ID_AP_RECOVER_EXP_ACK_TIME = 25,
	HWCMD_ID_UPDATE_BSSINFO			= 26,
	HWCMD_ID_SET_BA_REC				= 27,
	/*STA related*/
	HWCMD_ID_PWR_MGT_BIT_WIFI		= 28,
	HWCMD_ID_FORCE_WAKE_UP			= 29,
	HWCMD_ID_FORCE_SLEEP_AUTO_WAKEUP = 30,
	HWCMD_ID_MAKE_FW_OWN			= 31,
	HWCMD_ID_ENTER_PS_NULL			= 32,
	HWCMD_ID_SET_STA_DWRR			= 33,
	HWCMD_ID_UPDATE_RSSI			= 34,
	HWCMD_ID_SET_STA_DWRR_QUANTUM	= 35,
#ifdef CONFIG_STA_SUPPORT
	HWCMD_ID_PERODIC_CR_ACCESS_MLME_DYNAMIC_TX_RATE_SWITCHING	= 36,
#endif /* CONFIG_STA_SUPPORT */
	HWCMD_ID_PERODIC_CR_ACCESS_NIC_UPDATE_RAW_COUNTERS			= 37,
	HWCMD_ID_SET_BCN_OFFLOAD		= 38,
	HWCMD_ID_WIFISYS_LINKDOWN		= 41,
	HWCMD_ID_WIFISYS_LINKUP			= 42,
	HWCMD_ID_WIFISYS_OPEN			= 43,
	HWCMD_ID_WIFISYS_CLOSE			= 44,
	HWCMD_ID_WIFISYS_PEER_LINKUP	= 45,
	HWCMD_ID_WIFISYS_PEER_LINKDOWN	= 46,
	HWCMD_ID_WIFISYS_PEER_UPDATE	= 47,
	HWCMD_ID_THERMAL_PROTECTION_RADIOOFF = 48,
	HWCMD_ID_GET_TX_STATISTIC		= 49,
	HWCMD_ID_RADIO_ON_OFF			= 50,
	HWCMD_ID_PBC_CTRL				= 51,
#ifdef GREENAP_SUPPORT
	HWCMD_ID_GREENAP_ON_OFF			= 52,
#endif /* GREENAP_SUPPORT */
#ifdef LINK_TEST_SUPPORT
	HWCMD_ID_AUTO_LINK_TEST         = 53,
#endif /* LINK_TEST_SUPPORT */
	HWCMD_ID_HT_PROTECT				= 54,
	HWCMD_ID_RTS_THLD				= 55,
#ifdef HOST_RESUME_DONE_ACK_SUPPORT
	HWCMD_ID_HOST_RESUME_DONE_ACK	= 56,
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */
#ifdef ETSI_RX_BLOCKER_SUPPORT
	HWCMD_RX_CHECK_RSSI        = 57,
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	HWCMD_ID_PCIE_ASPM_DYM_CTRL		= 58,
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
#ifdef MBO_SUPPORT
	HWCMD_ID_BSS_TERMINATION        = 59,
#endif /* MBO_SUPPORT */
	HWCMD_ID_PART_SET_WMM			= 60,
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	HWCMD_ID_TWT_AGRT_UPDATE		= 61,
	HWCMD_ID_TWT_AGRT_MGMT		= 62,
	HWCMD_ID_MGMT_FRAME_OFFLOD	= 63,
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
	HWCMD_ID_SET_FRAME_OFFLOAD	= 64,
#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
	HWCMD_ID_UPDATE_4ADDR_HDR_TRANS,
#endif
	HWCMD_ID_UPDATE_MIB_COUNTER		= 66,
#ifdef NF_SUPPORT_V2
	HWCMD_ID_GET_NF_BY_FW			= 67,
#endif
#ifdef WIFI_MD_COEX_SUPPORT
	HWCMD_ID_WIFI_COEX_APCCCI2FW	= 68,
	HWCMD_ID_QUERY_LTE_SAFE_CHANNEL			= 69,
#endif
#ifdef VOW_SUPPORT
	HWCMD_ID_SET_VOW_SCHEDULE_CTRL			= 70,
#endif
#ifdef CFG_SUPPORT_CSI
	HWCMD_ID_GET_CSI_RAW_DATA		= 71,
#endif
#ifdef DABS_QOS
	HWCMD_ID_SET_DEL_QOS				= 72,
#endif
	HWCMD_ID_SET_QOS_CHARACTERISTICS_IE = 73,
	HWCMD_ID_UPDATE_TX_PER					= 76,
	HWCMD_ID_GET_TSF			= 77,
	HWCMD_ID_SMART_CARRIER_SENSE = 78,
	HWCMD_ID_FWLog2Host = 80,
#ifdef DOT11_EHT_BE
	HWCMD_ID_DSCB_UPDATE = 81,
#endif
	HWCMD_ID_BCN_PROC = 82,
	HWCMD_ID_WA_QUERY = 83,
	HWCMD_ID_WA_MULTIQUERY = 84,
	HWCMD_ID_FREE_RRO_SETBL = 85,
	HWCMD_ID_ALL_STA_STATS = 86,
#ifdef PRE_CFG_SUPPORT
	HWCMD_ID_SET_PRECFG_CMD = 87,
#endif
#ifdef IGMP_SNOOP_SUPPORT
	HWCMD_ID_WA_MCAST_ADD = 88,
	HWCMD_ID_WA_MCAST_DEL = 89,
#endif
	HWCMD_ID_GET_TSF_DIFF = 90,
#ifdef DOT11_EHT_BE
	HWCMD_ID_AT2LM_RES = 91,
	HWCMD_ID_NT2LM = 92,
	HWCMD_ID_RECONFIG_TMR = 93,
#endif
#ifdef IGMP_SNOOP_SUPPORT
#ifdef IGMP_SNOOPING_DENY_LIST
	HWCMD_ID_WA_MCAST_DENY = 94,
#endif
#endif
#ifdef DOT11_EHT_BE
	HWCMD_ID_RECONFIG_RM_LINK = 95,
#endif
	HWCMD_ID_SET_RX_FILTER_CNTL	= 96,
	HWCMD_ID_GET_STA_SNR	= 97,
#ifdef ZERO_PKT_LOSS_SUPPORT
	HWCMD_ID_HANDLE_NULL_ACK_EVENT          = 98,
	HWCMD_ID_HANDLE_STA_NULL_ACK_TIMEOUT = 99,
	HWCMD_ID_UPDATE_CHANNEL_STATS = 100,
#endif /*ZERO_LOSS_CSA_SUPPORT*/
	HWCMD_ID_UPDATE_CHANNEL_INFO = 101,
#ifdef FTM_SUPPORT
	HWCMD_ID_FTM_SET_CHANNEL = 102,
#endif /* FTM_SUPPORT */
	HWCMD_ID_SET_RRO_RECOVER = 103,
#ifdef FTM_SUPPORT
	HWCMD_ID_FTM_SET_MC_BURST = 106,
#endif /* FTM_SUPPORT */
#ifdef VLAN_SUPPORT
	HWCMD_ID_UPDATE_STARECVLAN = 107,
#endif
	HWCMD_ID_END,
};


/*for flag ID, is bit mask, 1/2/4/8*/
enum {
	HWFLAG_ID_FIRST = 0,
	HWFLAG_ID_UPDATE_PROTECT = 1 << 0,
	HWFLAG_ID_END,
};

/*HwCtrl CMD structure*/
typedef struct _HwCmdQElmt {
	UINT32 type;
	UINT32 command;
	VOID *buffer;
	UINT32 bufferlength;
	BOOLEAN NeedWait;
	RTMP_OS_COMPLETION ack_done;
	VOID *RspBuffer;
	UINT32 RspBufferLen;
	HwCmdCb CallbackFun;
	VOID *CallbackArgs;
	NDIS_SPIN_LOCK lock;
	os_kref refcnt;
#ifdef DBG_STARVATION
	struct starv_dbg starv;
#endif /*DBG_STARVATION*/
	struct _HwCmdQElmt *next;
	VOID *pAd;
} HwCmdQElmt, *PHwCmdQElmt;


typedef struct _HwCmdQ {
	UINT32 size;
	HwCmdQElmt *head;
	HwCmdQElmt *tail;
	UINT32 CmdQState;
} HwCmdQ, *PHwCmdQ;

typedef struct _HW_CTRL_TXD {
	UINT32			CmdType;
	UINT32			CmdId;
	BOOLEAN			NeedWait;
	UINT32			wait_time;
	VOID			*pInformationBuffer;
	UINT32			InformationBufferLength;
	VOID			*pRespBuffer;
	UINT32			RespBufferLength;
	HwCmdCb		CallbackFun;
	VOID			*CallbackArgs;
} HW_CTRL_TXD;


/* dump timer log if ser execute time > threshold  */
#define SER_TIME_THRESHOLD		1000000		/* 1s, unit: us */

enum {
	SER_TIME_ID_T0 = 0,
	SER_TIME_ID_T1,
	SER_TIME_ID_T2,
	SER_TIME_ID_T3,
	SER_TIME_ID_T4,
	SER_TIME_ID_T5,
	SER_TIME_ID_T6,
	SER_TIME_ID_T7,
	SER_TIME_ID_END,
};

typedef struct _HWCTRL_OP {
	NTSTATUS (*wifi_sys_open)(struct WIFI_SYS_CTRL *wsys);
	NTSTATUS (*wifi_sys_close)(struct WIFI_SYS_CTRL *wsys);
	NTSTATUS (*wifi_sys_link_up)(struct WIFI_SYS_CTRL *wsys);
	NTSTATUS (*wifi_sys_link_down)(struct WIFI_SYS_CTRL *wsys);
	NTSTATUS (*wifi_sys_connt_act)(struct WIFI_SYS_CTRL *wsys);
	NTSTATUS (*wifi_sys_disconnt_act)(struct WIFI_SYS_CTRL *wsys);
	NTSTATUS (*wifi_sys_peer_update)(struct WIFI_SYS_CTRL *wsys);
} HWCTRL_OP;

typedef struct _HW_CTRL_T {
	HwCmdQ HwCtrlQ;
	NDIS_SPIN_LOCK HwCtrlQLock;	/* CmdQLock spinlock */
	RTMP_OS_TASK HwCtrlTask;
	UINT32 TotalCnt;
	HWCTRL_OP hwctrl_ops;
#ifdef ERR_RECOVERY
	RTMP_OS_TASK ser_task;
	INT ser_func_state;
	UINT32 ser_status;
	NDIS_SPIN_LOCK ser_lock;
	UINT32 ser_times[SER_TIME_ID_END];
#endif /* ERR_RECOVERY */
#ifdef DBG_STARVATION
	struct starv_dbg_block block;
#endif /*DBG_STARVATION*/
} HW_CTRL_T;


/*CMD structure */
typedef struct _RT_SET_ASIC_WCID {
	ULONG WCID;		/* mechanism for rekeying: 0:disable, 1: time-based, 2: packet-based */
	ULONG SetTid;		/* time-based: seconds, packet-based: kilo-packets */
	ULONG DeleteTid;	/* time-based: seconds, packet-based: kilo-packets */
	UCHAR Addr[MAC_ADDR_LEN];	/* avoid in interrupt when write key */
	UCHAR Tid;
	UINT16 SN;
	UCHAR Basize;
	INT   Ses_type;
	BOOLEAN IsAdd;
	BOOLEAN IsBMC;
	BOOLEAN IsReset;
} RT_SET_ASIC_WCID, *PRT_SET_ASIC_WCID;

#ifdef HTC_DECRYPT_IOT
typedef struct _RT_SET_ASIC_AAD_OM {
	ULONG WCID;
	UCHAR Value; /* 0 ==> off, 1 ==> on */
} RT_SET_ASIC_AAD_OM, *PRT_SET_ASIC_AAD_OM;
#endif /* HTC_DECRYPT_IOT */

#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
typedef struct _RT_ASIC_4ADDR_HDR_TRANS {
	ULONG  Wcid;
	UCHAR  Enable;
} RT_ASIC_4ADDR_HDR_TRANS, *PRT_ASIC_4ADDR_HDR_TRANS;
#endif

/*MT MAC Specific*/
typedef enum _SEC_ASIC_KEY_OPERATION {
	SEC_ASIC_ADD_PAIRWISE_KEY,
	SEC_ASIC_REMOVE_PAIRWISE_KEY,
	SEC_ASIC_ADD_GROUP_KEY,
	SEC_ASIC_REMOVE_GROUP_KEY,
} SEC_ASIC_KEY_OPERATION;

/*MT MAC Specific*/
typedef enum _SEC_ASIC_KEY_DIRECTION {
	SEC_ASIC_KEY_TX,
	SEC_ASIC_KEY_RX,
	SEC_ASIC_KEY_BOTH,
} SEC_ASIC_KEY_DIRECTION;

typedef struct _SEC_KEY_INFO {
	UCHAR Key[32]; /* TK(32) */
	UCHAR TxMic[8];
	UCHAR RxMic[8];
	UCHAR TxTsc[16]; /* TSC value. Change it from 48bit to 128bit */
	UCHAR RxTsc[16]; /* TSC value. Change it from 48bit to 128bit */
	UCHAR KeyLen; /* Key length for each key, 0: entry is invalid */
} SEC_KEY_INFO, *PSEC_KEY_INFO;

typedef struct _ASIC_SEC_INFO {
	SEC_ASIC_KEY_OPERATION Operation;
	SEC_ASIC_KEY_DIRECTION Direction;
	UINT32 Cipher;
	UINT16 Wcid;
	UINT16 Wcid2;
	UCHAR BssIndex;
	UCHAR KeyIdx;
	SEC_KEY_INFO Key;
	UCHAR IGTK[32];
	UCHAR IGTKKeyLen;
	UCHAR igtk_key_idx;
	UCHAR bigtk[32];
	UCHAR bigtk_key_len;
	UCHAR bigtk_key_idx;
	UCHAR tx_tsc[20];
	UCHAR PeerAddr[MAC_ADDR_LEN];
} ASIC_SEC_INFO, *PASIC_SEC_INFO;

#define IS_ADDKEY_OPERATION(_pSecInfo)    ((_pSecInfo->Operation == SEC_ASIC_ADD_PAIRWISE_KEY) || (_pSecInfo->Operation == SEC_ASIC_ADD_GROUP_KEY))
#define IS_REMOVEKEY_OPERATION(_pSecInfo)    ((_pSecInfo->Operation == SEC_ASIC_REMOVE_PAIRWISE_KEY) || (_pSecInfo->Operation == SEC_ASIC_REMOVE_GROUP_KEY))
#define IS_PAIRWISEKEY_OPERATION(_pSecInfo)    ((_pSecInfo->Operation == SEC_ASIC_ADD_PAIRWISE_KEY) || (_pSecInfo->Operation == SEC_ASIC_REMOVE_PAIRWISE_KEY))
#define IS_GROUPKEY_OPERATION(_pSecInfo)    ((_pSecInfo->Operation == SEC_ASIC_ADD_GROUP_KEY) || (_pSecInfo->Operation == SEC_ASIC_REMOVE_GROUP_KEY))


typedef struct _MT_SET_BSSINFO {
	UCHAR OwnMacIdx;
	UINT8 ucBssIndex;
	UINT8 Bssid[MAC_ADDR_LEN];
	UINT8 BcMcWlanIdx;
	UINT32 NetworkType;
	UINT32 u4ConnectionType;
	UINT8 Active;
	UINT32 u4EnableFeature;
} MT_SET_BSSINFO, *PMT_SET_BSSINFO;

typedef struct _MT_SET_BCN_OFFLOAD {
	UINT8 WdevIdx;
	ULONG WholeLength;
	BOOLEAN Enable;
	UCHAR OffloadPktType;
	ULONG TimIePos;
	ULONG CsaIePos;
} MT_SET_BCN_OFFLOAD, *PMT_SET_BCN_OFFLOAD;

typedef struct _MT_SET_FRAME_OFFLOAD {
	UINT8 WdevIdx;
	UINT8 ucEnable;
	UINT8 ucTxType;
	UINT8 ucTxMode;			/* SU/MU/SU+MU */
	UINT8 ucTxInterval;		/* in unit of TU */
	UINT16 u2PktLength;
	UINT16 u2Wcid;

	UINT8 acPktContent[2048]; /* Eagle project max in-band cmd size */
} MT_SET_FRAME_OFFLOAD, *PMT_SET_FRAME_OFFLOAD;

typedef struct _MT_UPDATE_BEACON {
	struct wifi_dev *wdev;
	UCHAR UpdateReason;
} MT_UPDATE_BEACON, *PMT_UPDATE_BEACON;

typedef struct _MT_SET_STA_REC {
	UINT8 BssIndex;
	UINT8 WlanIdx;
	UINT32 ConnectionType;
	UINT8 ConnectionState;
	UINT32 EnableFeature;
} MT_SET_STA_REC, *PMT_SET_STA_REC;

typedef struct _RT_SET_TR_ENTRY {
	ULONG WCID;
	VOID *pEntry;
} RT_SET_TR_ENTRY, *PRT_SET_TR_ENTRY;

typedef struct _MT_VOW_STA_GROUP {
	UINT16 StaIdx;
	UINT8 GroupIdx;
	UINT8 WmmIdx;
} MT_VOW_STA_GROUP, *PMT_VOW_STA_GROUP;

#ifdef VOW_SUPPORT
typedef struct _MT_VOW_STA_QUANTUM {
	BOOLEAN restore;
	UINT8 quantum;
} MT_VOW_STA_QUANTUM, *PMT_VOW_STA_QUANTUM;

#endif /* VOW_SUPPORT */

typedef struct _SLOT_CFG {
	BOOLEAN bUseShortSlotTime;
	UCHAR Channel;
	struct wifi_dev *wdev;
} SLOT_CFG;

typedef struct _REMOVE_REPT_ENTRY_STRUC {
	UCHAR CliIdx;
} REMOVE_REPT_ENTRY_STRUC, *PREMOVE_REPT_ENTRY_STRUC;

typedef struct _ADD_REPT_ENTRY_STRUC {
	struct wifi_dev *wdev;
	UCHAR arAddr[MAC_ADDR_LEN];
} ADD_REPT_ENTRY_STRUC, *PADD_REPT_ENTRY_STRUC;

typedef struct _TX_STAT_STRUC {
	UINT32 Field;	/* Tx Statistic update method from N9 (GET_TX_STAT_XXX) */
	UINT16 Wcid;
	UINT8 Band;
} TX_STAT_STRUC, *PTX_STAT_STRUC;

struct _CR4_QUERY_STRUC {
	UINT32 arg0;
	UINT32 arg1;
	UINT32 arg2;
	UINT16 number; /* Tx Statistic update method from CR4 */
	UINT8 reserve[2];
	UINT16 list[0];
};

struct _CR4_MCAST_ENTRY {
	PNET_DEV pdev;
	UINT16 wcid;
	UINT8 bss_idx;
	UINT8 filter_type;
	UCHAR group_addr[HWCTRL_MCAST_ADDR_LEN];
	UCHAR member_addr[MAC_ADDR_LEN];
};

#ifdef IGMP_SNOOPING_DENY_LIST
#define HWCTRL_IGMP_DENY_TABLE_SIZE_MAX 32
struct _CR4_MCAST_DENY_LIST {
	UINT8 bss_idx;
	UINT8 entry_cnt;
	UINT8 add_to_list;
	UINT8 deny_list[HWCTRL_IGMP_DENY_TABLE_SIZE_MAX * HWCTRL_MCAST_ADDR_LEN];
	UINT8 Prefix_list[HWCTRL_IGMP_DENY_TABLE_SIZE_MAX];
};
#endif



#ifdef TXBF_SUPPORT
typedef struct _MT_STA_BF_ADJ {
	struct wifi_dev *wdev;
	UCHAR ConnectionState;
} MT_STA_BF_ADJ, *PMT_STA_BF_ADJ;
#endif /* TXBF_SUPPORT */

typedef struct _MT_SET_LED_STS {
	UCHAR Status;
	UCHAR BandIdx;
} MT_SET_LED_STS, *PMT_SET_LED_STS;
typedef struct _MT_LED_GPIO_MAP {
	UINT8 led_index;
	UINT16 map_index;
	BOOLEAN ctr_type;
} MT_LED_GPIO_MAP, *PMT_LED_GPIO_MAP;
#ifdef VLAN_SUPPORT
struct VLAN_STA_REC {
	UINT8 ucBssIndex;
	UINT16 u2WlanIdx;
};
#endif
/* For wifi and md coex in colgin project*/
#ifdef WIFI_MD_COEX_SUPPORT
typedef struct _MT_WIFI_COEX_APCCCI2FW {
	UCHAR data[1640]; /* FW cmd length limit 1.6k bytes*/
	UINT16 len;
} MT_WIFI_COEX_APCCCI2FW, *PMT_WIFI_COEX_APCCCI2FW;
#endif /*WIFI_MD_COEX_SUPPORT*/

struct at2lm_res_req_t {
	u8 mld_addr[MAC_ADDR_LEN];
	u8 fw_mld_idx;		/* WM viewpoint*/
	u8 flag;
	u8 at2lm_id;
	u8 bss_info_idx;
	u8 en_mst_timer;
	u8 en_ed_timer;
	u16 mst_timer_adv_time;	/*unit: ms*/
	u16 e_timer_adv_time;	/*unit: ms*/
	u32 mst_dur;		/*unit: us*/
	u32 e_dur;		/*unit: us*/
	u16 dis_link_id_bitmap; /*disable link_id in bitmap*/
	u8 dis_link_id_bss_info_idx[16]; /*bss info index of disable link_id*/
};

struct at2lm_res_rsp_t {
	u8 sts;
	u8 mld_addr[MAC_ADDR_LEN];
	u8 fw_mld_idx;
	u8 at2lm_id;
	u8 bss_info_idx;
	UINT64 mst_tsf;
	UINT64 ed_tsf;
};

struct nt2lm_req_t {
	u8 self_mld_addr[MAC_ADDR_LEN];	/* self mld addr */
	u8 peer_mld_add[MAC_ADDR_LEN];	/* peer mld addr */
	u8 mld_idx;
	u8 flag;
	u16 link_id_bitmap;		/* which link_id tidmap is valid */
	u16 link_id_to_wcid[16];
	u8 tid_map_dl[16];
	u8 tid_map_ul[16];
};

#ifdef DOT11_EHT_BE
struct reconfig_set_tmr_t {
	UINT8 mld_addr[MAC_ADDR_LEN];
	UINT8 fw_mld_idx;
	UINT8 mld_flag;
	UINT16 tmr_link_id_bmap; /* indicate which links would countdown */
	UINT8 fw_bss_idx[16]; /* fw_bss_idx of each link */
	UINT16 num_seconds; /* number of seconds of timer */
};

struct reconfig_args {
	UINT8 reconfig_mode; /* enum mld_reconfig_mode */
};

struct reconfig_tmr_to_t {
	UINT8 mld_addr[MAC_ADDR_LEN];
	UINT8 fw_mld_idx;
	UINT16 to_link_id_bmap; /* indicate which links countdown timeout */
	UINT8 fw_bss_idx[16]; /* fw_bss_idx of each link */
	struct reconfig_args args;
};

struct reconfig_rm_link_req_t {
	u8 mld_addr[MAC_ADDR_LEN];	/* AP MLD addr */
	u8 fw_mld_idx;			/* WM viewpoint*/
	u8 flag;
	u16 rm_link_id_bitmap;		/* which link_id bitmap is valid */
	u16 link_id_bss_info_idx[16];
};
#endif

/*Export API function*/
UINT32 HwCtrlInit(struct _RTMP_ADAPTER *pAd);
VOID HwCtrlExit(struct _RTMP_ADAPTER *pAd);
UINT32 HWCtrlOpsReg(struct _RTMP_ADAPTER *pAd);
UINT32 hwctrl_queue_len(struct _RTMP_ADAPTER *pAd);
BOOLEAN hwctrl_cmd_q_empty(struct _RTMP_ADAPTER *pAd);

UINT32 HwCmdThreadInit(VOID *physical_dev);
VOID HwCmdThreadExit(VOID *physical_dev);

#ifdef WF_RESET_SUPPORT
UINT32 wf_reset_init(struct physical_device *device);
UINT32 wf_reset_exit(struct physical_device *device);
#endif

NDIS_STATUS HwCtrlEnqueueCmd(
	struct _RTMP_ADAPTER *pAd,
	HW_CTRL_TXD HwCtrlTxd);

INT Show_HwCtrlStatistic_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);


/*Security*/
VOID HW_ADDREMOVE_KEYTABLE(struct _RTMP_ADAPTER *pAd, struct _ASIC_SEC_INFO *pInfo);

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)

#define RTMP_MLME_PRE_SANITY_CHECK(_pAd)

#define RTMP_AP_ADJUST_EXP_ACK_TIME(_pAd) \
	RTMP_IO_WRITE32(_pAd->hdev_ctrl,  EXP_ACK_TIME, 0x005400ca)

#define RTMP_AP_RECOVER_EXP_ACK_TIME(_pAd) \
	RTMP_IO_WRITE32(_pAd->hdev_ctrl,  EXP_ACK_TIME, 0x002400ca)

#define RTMP_SET_LED_STATUS(_pAd, _Status, _BandIdx) \
	RTMPSetLEDStatus(_pAd, _Status, _BandIdx)

#define RTMP_SET_LED(_pAd, _Mode, _BandIdx) \
	RTMPSetLED(_pAd, _Mode, _BandIdx)

VOID RTMP_PWR_MGT_BIT_WIFI(struct _RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT8 ucPwrMgtBit);
VOID RTMP_FORCE_WAKEUP(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg);
VOID RTMP_SLEEP_FORCE_AUTO_WAKEUP(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg);

#else
VOID RTMP_MLME_PRE_SANITY_CHECK(struct _RTMP_ADAPTER *pAd);

/*Security*/
VOID RTMP_AP_ADJUST_EXP_ACK_TIME(struct _RTMP_ADAPTER *pAd);
VOID RTMP_AP_RECOVER_EXP_ACK_TIME(struct _RTMP_ADAPTER *pAd);
VOID RTMP_SET_LED_STATUS(struct _RTMP_ADAPTER *pAd, UCHAR Status, CHAR BandIdx);
VOID RTMP_SET_LED(struct _RTMP_ADAPTER *pAd, UINT32 WPSLedMode10, CHAR BandIdx);

/*STA*/
#ifdef CONFIG_STA_SUPPORT
VOID RTMP_PWR_MGT_BIT_WIFI(struct _RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT8 ucPwrMgtBit);
VOID RTMP_FORCE_WAKEUP(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg);
VOID RTMP_SLEEP_FORCE_AUTO_WAKEUP(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg);
#endif /* CONFIG_STA_SUPPORT */

#endif /*defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)*/
VOID RTMP_LED_GPIO_MAP(struct _RTMP_ADAPTER *pAd, UINT8 led_index, UINT16 map_index, BOOLEAN ctr_type);

/*Common*/
VOID RTMP_UPDATE_RAW_COUNTER(struct _RTMP_ADAPTER *pAd);
VOID RTMP_UPDATE_MIB_COUNTER(struct _RTMP_ADAPTER *pAd);
VOID RTMP_SMART_CARRIER_SENSE(struct _RTMP_ADAPTER *pAd);
VOID RTMP_FWLog2Host(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 FWLog2HostCtrl);
VOID RTMP_UPDATE_CHANNEL_INFO(struct _RTMP_ADAPTER *pAd, UCHAR ch_index);
#ifdef ZERO_PKT_LOSS_SUPPORT
VOID RTMP_UPDATE_CHANNEL_STATS(struct _RTMP_ADAPTER *pAd);
VOID HANDLE_NULL_ACK_EVENT(struct _RTMP_ADAPTER *pAd, UINT16 data);
VOID HANDLE_STA_NULL_ACK_TIMEOUT(struct _RTMP_ADAPTER *pAd, UINT32 len,
		void *ctrl);
#endif /*ZERO_PKT_LOSS_SUPPORT*/
VOID RTMP_SET_RRO_RECOVER(struct _RTMP_ADAPTER *pAd);
VOID RTMP_PS_RETRIVE_START(struct _RTMP_ADAPTER *pAd, UINT16 Wcid);
VOID RTMP_PS_RETRIVE_CLEAR(struct _RTMP_ADAPTER *pAd, UINT16 Wcid);
VOID RTMP_HANDLE_PRETBTT_INT_EVENT(struct _RTMP_ADAPTER *pAd);
VOID RTMP_GET_ALL_STA_STATS(struct _RTMP_ADAPTER *pAd, UINT32 eventTypeBitmap);

VOID HW_SET_TX_BURST(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 ac_type,
					 UINT8 prio, UINT16 level, UINT8 enable);

#ifdef FTM_SUPPORT
VOID HW_SET_FTM_CHANNEL(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
						UCHAR channel);
VOID HW_SET_FTM_MC_BURST(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#endif /* FTM_SUPPORT */

VOID HW_SET_PART_WMM_PARAM(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					 UCHAR wmm_idx, UINT32 AcNum, UINT32 EdcaType, UINT32 EdcaValue);

#ifdef VOW_SUPPORT
VOID HW_SET_VOW_SCHEDULE_CTRL(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN apply_sch_ctrl,
	UINT8 sch_type,
	UINT8 sch_policy);
#endif

#ifdef DABS_QOS
NDIS_STATUS HW_UPDATE_QOS_PARAM(struct _RTMP_ADAPTER *pAd, UINT32 idx, BOOLEAN set_del);
#endif

VOID HW_UPDATE_BSSINFO(struct _RTMP_ADAPTER *pAd, struct _BSS_INFO_ARGUMENT_T *BssInfoArgs);

#ifdef DOT11_EHT_BE
VOID HW_UPDATE_DSCBINFO(
	struct _RTMP_ADAPTER *pAd,
	UINT8 BandIdx,
	UINT8 BssIdx,
	UINT8 DscbPresent,
	UINT16 dscb_bitmap,
	UINT8 Ctrl,
	UINT8 Ccfs0,
	UINT8 Ccfs1);

int HW_SET_RECONFIG_TMR(
	struct wifi_dev *wdev,
	struct reconfig_set_tmr_t *reconfig_info
);
int HW_REQ_RECONFIG_RM_LINK(
	struct wifi_dev *wdev,
	struct reconfig_rm_link_req_t *req
);
#endif

VOID RTMP_SET_BA_REC(struct _RTMP_ADAPTER *pAd, VOID *Buffer, UINT32 Len);
VOID HW_SET_BA_REC(
	struct _RTMP_ADAPTER *pAd,
	UINT16 wcid,
	UCHAR tid,
	UINT16 sn,
	UINT16 basize,
	BOOLEAN isAdd,
	INT ses_type,
	UCHAR amsdu);
VOID HW_SET_DEL_ASIC_WCID(struct _RTMP_ADAPTER *pAd, ULONG Wcid);

#ifdef HTC_DECRYPT_IOT
VOID HW_SET_ASIC_WCID_AAD_OM(struct _RTMP_ADAPTER *pAd, ULONG Wcid, UCHAR value);
#endif /* HTC_DECRYPT_IOT */

#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
VOID HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(struct _RTMP_ADAPTER *pAd, ULONG Wcid, UCHAR IsEnable);
#endif


VOID HW_SET_FRAME_OFFLOAD(struct _RTMP_ADAPTER *pAd,
				UINT8 WdevIdx,
				ULONG WholeLength,
				BOOLEAN Enable,
				UINT16 wcid,
				UINT8 TxType,
				UINT8 TxMode,
				UINT8 TxInterval,
				UCHAR *Buf);

VOID RTMP_GET_TEMPERATURE(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT32 *pTemperature);
VOID RTMP_RADIO_ON_OFF_CTRL(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 ucRadio);
VOID RTMP_FREE_RRO_SETBL(struct _RTMP_ADAPTER *pAd, UINT16 seid);

#ifdef PRE_CFG_SUPPORT
VOID RTMP_SET_PRECFG_CMD(void *pPreCfgTimerCmdCtrl_v);
#endif


int HW_GET_TSF(struct wifi_dev *wdev, UINT32 *current_tsf);
int HW_GET_TSF_DIFF(
	struct wifi_dev *wdev,
	UINT8 BssIdx0,
	UINT8 BssIdx1,
	UINT32 *tsf_diff);
int HW_GET_AT2LM_RES(
	struct wifi_dev *wdev,
	struct at2lm_res_req_t *at2lm_res_req,
	struct at2lm_res_rsp_t *at2lm_res_rsp
);
int HW_SET_NT2LM(
	struct wifi_dev *wdev,
	struct nt2lm_req_t *req
);

#ifdef MBO_SUPPORT
VOID RTMP_BSS_TERMINATION(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#endif /* MBO_SUPPORT */

#ifdef GREENAP_SUPPORT
VOID RTMP_GREENAP_ON_OFF_CTRL(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN ucGreenAP);
#endif /* GREENAP_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
VOID rtmp_pcie_aspm_dym_ctrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN fgL1Enable, BOOLEAN fgL0sEnable);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
VOID rtmp_twt_agrt_update(struct _RTMP_ADAPTER *ad, struct TWT_AGRT_PARA_T *agrt_para);
VOID rtmp_twt_agrt_update_by_cmd_task(struct _RTMP_ADAPTER *ad, struct TWT_AGRT_PARA_T *agrt_para);
VOID rtmp_twt_agrt_mgmt(struct _RTMP_ADAPTER *ad, struct TWT_AGRT_PARA_T *agrt_para, struct TWT_AGRT_MGMT_T *agrt_mgmt);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
VOID rtmp_mgmt_frame_offload(struct _RTMP_ADAPTER *ad, UINT8 bssinfo_idx, UINT16 wcid, UINT8 token,
	UINT8 mlo_en, UINT16 mld_sta_idx, UINT8 action_linkid, UINT16 all_linkid_bitmap, UINT8 *buf, UINT32 len);
VOID HW_SET_SLOTTIME(struct _RTMP_ADAPTER *pAd, BOOLEAN bUseShortSlotTime, UCHAR Channel, struct wifi_dev *wdev);
VOID HW_ENTER_PS_NULL(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg);
VOID HW_BEACON_UPDATE(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR UpdateBeacon);
#ifdef PKT_BUDGET_CTRL_SUPPORT
VOID HW_SET_PBC_CTRL(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UCHAR type);
#endif /*PKT_BUDGET_CTRL_SUPPORT*/

VOID HW_SET_RTS_THLD(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR pkt_num, UINT32 length);

VOID HW_SET_PROTECT(struct _RTMP_ADAPTER *pAd,
		    struct wifi_dev *wdev,
		    enum prot_service_type type,
		    UINT32 cookie1,
		    UINT32 cookie2);

/* Insert the BA bitmap to ASIC for the Wcid entry */
#define RTMP_ADD_BA_SESSION_TO_ASIC(_pAd, _wcid, _TID, _SN, _basize, _type, _amsdu)	\
	HW_SET_BA_REC(_pAd, _wcid, _TID, _SN, _basize, 1, _type, _amsdu);


/* Remove the BA bitmap from ASIC for the Wcid entry */
/*		bitmap field starts at 0x10000 in ASIC WCID table */
#define RTMP_DEL_BA_SESSION_FROM_ASIC(_pAd, _wcid, _TID, _type, _amsdu) \
	HW_SET_BA_REC(_pAd, _wcid, _TID, 0, 0, 0, _type, _amsdu);

VOID RTMP_SET_STA_DWRR(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#ifdef VOW_SUPPORT
VOID RTMP_SET_STA_DWRR_QUANTUM(struct _RTMP_ADAPTER *pAd, BOOLEAN restore, UCHAR quantum);
#endif /* VOW_SUPPORT */
VOID RTMP_SET_UPDATE_PER(struct _RTMP_ADAPTER *pAd, UINT16 Wcid);
VOID RTMP_SET_UPDATE_RSSI(struct _RTMP_ADAPTER *pAd);

#ifdef	ETSI_RX_BLOCKER_SUPPORT /* RX Blocker Solution */
VOID RTMP_CHECK_RSSI(struct _RTMP_ADAPTER *pAd);
#endif /* end ETSI_RX_BLOCKER_SUPPORT */

VOID RTMP_SET_THERMAL_RADIO_OFF(struct _RTMP_ADAPTER *pAd);

VOID NICUpdateRawCountersNew(
	struct _RTMP_ADAPTER *pAd);
#ifdef CONFIG_STA_SUPPORT
VOID MlmeDynamicTxRateSwitchingNew(
	struct _RTMP_ADAPTER *pAd);
#endif /* CONFIG_STA_SUPPORT */

#ifdef ERR_RECOVERY
#ifdef MTK_FE_RESET_RECOVER
unsigned int mtk_fe_reset_notifier_init(struct physical_device *device);
void mtk_fe_reset_notifier_exit(struct physical_device *device);

#define MTK_FE_START_RESET 0x2000
#define MTK_FE_RESET_DONE 0x2001
#define MTK_WIFI_RESET_DONE 0x2002
#define MTK_WIFI_CHIP_ONLINE 0x2003
#define MTK_WIFI_CHIP_OFFLINE 0x2004
#define MTK_FE_STOP_TRAFFIC	0x2005
#define MTK_FE_STOP_TRAFFIC_DONE 0x2006
#define MTK_FE_START_TRAFFIC	0x2007
#define MTK_FE_STOP_TRAFFIC_DONE_FAIL 0x2008
#define MTK_FE_START_RESET_INIT 0x2009

struct mtk_notifier_block {
	struct notifier_block nb;
	void *priv;

};
#endif

typedef enum _ERR_RECOVERY_STAGE {
	ERR_RECOV_STAGE_STOP_IDLE = 0,
	ERR_RECOV_STAGE_STOP_PDMA0,
	ERR_RECOV_STAGE_RESET_PDMA0,
	ERR_RECOV_STAGE_STOP_IDLE_DONE,
	ERR_RECOV_STAGE_WAIT_N9_NORMAL,
	ERR_RECOV_STAGE_EVENT_REENTRY,
#ifdef MTK_FE_RESET_RECOVER
	ERR_RECOV_STAGE_STOP_PDMA0_FE_PRE,
#endif
	ERR_RECOV_STAGE_STATE_NUM
} ERR_RECOVERY_STAGE, *P_ERR_RECOVERY_STAGE;

typedef struct _ERR_RECOVERY_CTRL_T {
	ERR_RECOVERY_STAGE errRecovStage;
	UINT32 status;
	struct _RTMP_ADAPTER *ad;
#ifdef MTK_FE_RESET_RECOVER
	atomic_t notify_fe;
	struct mtk_notifier_block mtk_nb;
#endif
} ERR_RECOVERY_CTRL_T, *P_ERR_RECOVERY_CTRL_T;

VOID RTMP_MAC_RECOVERY(struct _RTMP_ADAPTER *pAd, UINT32 Status);
INT IsStopingPdma(struct _ERR_RECOVERY_CTRL_T *pErrRecoveryCtl);
ERR_RECOVERY_STAGE ErrRecoveryCurStage(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl);
VOID ser_sys_reset(RTMP_STRING *arg);
#ifdef HWIFI_SUPPORT
#define HwRecoveryFromError(_ad)
#else /* HWIFI_SUPPORT */
NTSTATUS HwRecoveryFromError(struct _RTMP_ADAPTER *pAd);
#endif /* !HWIFI_SUPPORT */
void SerTimeLogDump(struct _RTMP_ADAPTER *pAd);
#endif /* ERR_RECOVERY */

#ifdef WF_RESET_SUPPORT
NTSTATUS wf_reset_func(struct physical_device *device);
#endif

UINT32 HW_WIFISYS_OPEN(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);
UINT32 HW_WIFISYS_CLOSE(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);
UINT32 HW_WIFISYS_LINKUP(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);
UINT32 HW_WIFISYS_LINKDOWN(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);
UINT32 HW_WIFISYS_PEER_LINKUP(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);
UINT32 HW_WIFISYS_PEER_LINKDOWN(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);
UINT32 HW_WIFISYS_PEER_UPDATE(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);
VOID HW_WIFISYS_RA_UPDATE(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);

VOID HW_GET_TX_STATISTIC(struct _RTMP_ADAPTER *pAd, TX_STAT_STRUC *P_buf, UCHAR num);

#ifdef WIFI_MD_COEX_SUPPORT
VOID HW_QUERY_LTE_SAFE_CHANNEL(struct _RTMP_ADAPTER *pAd);
#endif


enum {
	HWCTRL_OP_TYPE_V1,
	HWCTRL_OP_TYPE_V2,
};

enum protect_mode_update_method {
	HWCTRL_PROT_UPDATE_METHOD_V1, /*update HW CR by command only*/
	HWCTRL_PROT_UPDATE_METHOD_V2, /*update BSSINFO and HW CR by bssinfo command tag*/
};

void hw_set_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					 UINT8 ac_type, UINT8 prio, UINT16 level, UINT8 enable);

#ifdef WIFI_SYS_FW_V1
VOID hw_ctrl_ops_v1_register(struct _HWCTRL_OP *hwctrl_ops);
#endif /*WIFI_SYS_FW_V1*/

#ifdef WIFI_SYS_FW_V2
VOID hw_ctrl_ops_v2_register(struct _HWCTRL_OP *hwctrl_ops);
#endif /*WIFI_SYS_FW_V2*/

#ifdef LINK_TEST_SUPPORT
VOID RTMP_AUTO_LINK_TEST(struct _RTMP_ADAPTER *pAd);
#endif /* LINK_TEST_SUPPORT */

#ifdef HOST_RESUME_DONE_ACK_SUPPORT
VOID rtmp_host_resume_done_ack(struct _RTMP_ADAPTER *pAd);
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */

#ifdef NF_SUPPORT_V2
VOID HW_NF_UPDATE(struct _RTMP_ADAPTER *pAd, UCHAR flag);
#endif

/* For wifi and md coex in colgin project*/
#ifdef WIFI_MD_COEX_SUPPORT
VOID HW_WIFI_COEX_APCCCI2FW(struct _RTMP_ADAPTER *pAd, VOID *apccci2fw_msg);
#endif /* WIFI_MD_COEX_SUPPORT */

#ifdef CFG_SUPPORT_CSI
VOID HW_CSI_CTRL(struct _RTMP_ADAPTER *pAd, void *prCSICtrl);
#endif

VOID HW_WA_QUERY(struct _RTMP_ADAPTER *pAd, UINT32 arg0, UINT32 arg1, UINT32 arg2);
VOID HW_WA_MULTIQUERY(struct _RTMP_ADAPTER *pAd, UINT32 arg0, UINT32 arg1, UINT32 arg2, struct _CR4_QUERY_STRUC *cr4_query_list);

#ifdef IGMP_SNOOP_SUPPORT
BOOLEAN HW_WA_MCAST_ENTRY_ADD(struct _RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, UINT8 Type, PUCHAR MemberAddr, PNET_DEV dev, UINT16 wcid);
BOOLEAN HW_WA_MCAST_ENTRY_DEL(struct _RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, PUCHAR MemberAddr, PNET_DEV dev, UINT16 wcid);
#ifdef IGMP_SNOOPING_DENY_LIST
BOOLEAN HW_WA_MCAST_Deny_LIST(struct _RTMP_ADAPTER *pAd, UINT8 BssIdx, UINT8 entry_cnt, UINT8 add_to_list, UINT8 *pAddr, UINT8 *Prefix_list);
#endif

#endif

#ifdef QOS_R3
INT32 HW_UPDATE_QOS_CHARA_IE(struct _RTMP_ADAPTER *pAd, VOID *qosData);
#endif
VOID HW_RX_FILTER_CTRL(struct _RTMP_ADAPTER *pAd, void *rxfilter);
VOID HW_GET_STA_SNR(struct _RTMP_ADAPTER *pAd, UINT8 wcid, UINT8 *pSnr);
#ifdef VLAN_SUPPORT
VOID HW_VLAN_BMC_VLANTAG_SET(struct _RTMP_ADAPTER *pAd, UINT8 BssIndex, UINT16 WlanIdx);
#endif
#endif
