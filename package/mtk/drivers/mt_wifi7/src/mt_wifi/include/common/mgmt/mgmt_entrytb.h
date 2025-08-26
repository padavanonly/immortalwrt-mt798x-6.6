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

	Module Name:
	mgmt_entrytb.h

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/

#ifndef _MGMT_ENTRYTB_H_
#define _MGMT_ENTRYTB_H_

#include "rtmp_type.h"
#include "rtmp_def.h"
#include "oid.h"
#include "rtmp_dot11.h"
#include "rtmp_timer.h"
#ifdef WIFI_TWT_SUPPORT
#include "twt/twt_def.h"
#endif /* WIFI_TWT_SUPPORT */
#include "mlme.h"
#include "hw_ctrl/rate_ctrl/ra_ctrl.h"
#ifdef DOT11W_PMF_SUPPORT
#include "security/pmf_cmm.h"
#endif /* DOT11W_PMF_SUPPORT */
#include "tr.h"
#ifdef RT_CFG80211_SUPPORT
#include "mtk_vendor_nl80211.h"
#endif

#define INVALID_AID	2008 /*According SPEC, aid is in the range of 1 to 2007.*/
#define MAX_MCS_SET 16		/* From MCS 0 ~ MCS 15 */
#ifdef DOT11_EHT_BE
#define MLD_STA_MAX_NUM	(PEER_AP_MLD_IDX + 1)
#endif /*DOT11_EHT_BE*/

struct _RTMP_CHIP_CAP;
struct _RTMP_ADAPTER;
struct _RTMP_CHIP_DBG;

struct _aid_info {
	UINT32 *aid_bitmap;
	UINT16 aid_allocate_from_idx;
	UINT16 max_aid;
};

/* sub catalog of MAC_TABLE ENTRY */
struct ampdu_caps {
	/* ht */
	UINT8 max_ht_ampdu_len_exp;
	UINT8 min_mpdu_start_spacing;
	/* vht */
	UINT8 max_mpdu_len;
	UINT8 max_vht_ampdu_len_exp;
	/* he */
	UINT8 max_he_ampdu_len_exp;
	UINT8 multi_tid_agg;
	/* he_6g */
	UINT8 he6g_min_mpdu_start_spacing;
	UINT8 he6g_max_mpdu_len;
	UINT8 he6g_max_ampdu_len_exp;
};

struct stbc_caps {
	/* ht */
	UINT8 ht_rx_stbc;
	/* vht */
	UINT8 vht_rx_stbc;
	/* he */
	enum he_stbc_caps he_stbc;
};

struct support_ch_bw {
	/* ht */
	UINT8 ht_support_ch_width_set;
	/* vht */
	UINT8 vht_support_ch_width_set;
	UINT8 ext_nss_bw;
	/* he */
	UINT8 he_ch_width;
	UINT8 he_bw20_242tone;
	UINT8 he6g_ch_width;
	UINT8 he6g_prim_ch;
	UINT8 he6g_ccfs_0;
	UINT8 he6g_ccfs_1;
	/* ch info */
	UINT8 prim_ch;
	UINT8 ccfs_0;
	UINT8 ccfs_1;
	UINT8 ccfs_2;
};

struct he_bss_color {
	UINT8 bss_color;
	UINT8 partial_bss_color;
	UINT8 bss_color_dis;
};
/* sub catalog of MAC_TABLE ENTRY end */

enum support_mode {
	HT_MIX_SUPPORT = 1,
	HT_GF_SUPPORT = (1 << 1),
	VHT_SUPPORT = (1 << 2),
	HE_24G_SUPPORT = (1 << 3),
	HE_5G_SUPPORT = (1 << 4),
	HE_6G_SUPPORT = (1 << 5),
	EHT_24G_SUPPORT = (1 << 6),
	EHT_5G_SUPPORT = (1 << 7),
	EHT_6G_SUPPORT = (1 << 8),
};

struct caps_info {
	struct ampdu_caps ampdu;
	struct stbc_caps stbc;
	struct support_ch_bw ch_bw;
	struct rate_caps rate;
	enum support_mode modes;
	/* ht */
	enum ht_caps ht_cap;
	UINT8 max_amsdu_len;
	UINT8 pco_tx_time;
	UINT8 mfb;
	/* vht */
	enum vht_caps vht_cap;
	UINT8 bfee_sts_cap;
	UINT8 sound_dim;
	UINT8 vht_link_adapt;
	/* he */
	enum he_mac_caps he_mac_cap;
	enum he_phy_caps he_phy_cap;
	enum he_gi_caps he_gi;
	struct he_bss_color bss_color_info;
	struct he_bf_info he_bf;
	UINT8 default_pe_dur;
	UINT16 txop_dur_rts_thld;
	UINT8 he_frag_level;
	UINT8 max_frag_msdu_num;
	UINT8 min_frag_size;
	UINT8 tf_mac_pad_duration;
	UINT8 he_link_adapt;
	UINT8 punc_preamble_rx;
	UINT8 midamble_rx_max_nsts;
	UINT8 dcm_max_constellation_tx;
	UINT8 dcm_max_constellation_rx;
	UINT8 dcm_max_ru;
	/* he 6g */
	struct he_6g_cap_ie he_6g_cap;
	UINT8 he6g_op_present;
	UINT8 he6g_dup_bcn;
	UINT8 min_rate;
	enum sm_power_save he6g_smps;
	UINT8 rd_resp;
	UINT8 rx_ant_pattern_consist;
	UINT8 tx_and_pattern_consist;
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	struct itwt_ie_t twt_ie;
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
#ifdef DOT11_EHT_BE
	enum eht_phy_caps eht_phy_cap;
	enum eht_gi_caps eht_gi;
	struct eht_bf_info eht_bf;
	UINT8 eht_common_nominal_padd;
	UINT8 eht_mcs15_mru_set;
#endif /* DOT11_EHT_BE */
};

#ifdef OCE_FILS_SUPPORT
#define MAX_FILS_PTK_LEN 88 /* KCK + KEK + TK */

struct fils_info {
	UINT16 last_pending_id;
	UCHAR *pending_ie;
	UINT pending_ie_len;
	UCHAR *extra_ie;
	UINT extra_ie_len;
	struct raw_rssi_info rssi_info;

	UCHAR auth_algo;

	UCHAR PTK[MAX_FILS_PTK_LEN];
	UCHAR PTK_len;

	INT16 status;
	BOOLEAN is_pending_auth;
	BOOLEAN is_pending_assoc;
	BOOLEAN is_pending_decrypt;
	BOOLEAN is_pending_encrypt;
	BOOLEAN is_post_assoc;

	VOID (*pending_action)(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*pending_decrypt)(PRTMP_ADAPTER pAd, struct _MAC_TABLE_ENTRY *pEntry,
		struct _SECURITY_CONFIG *pSecConfig, MLME_QUEUE_ELEM *Elem);
};
#endif /* OCE_FILS_SUPPORT */

#ifdef MWDS
struct MWDS_STRUCT {
	UINT32 Addr4PktNum;
	UINT32 Addr3PktNum;
	UINT32 NullPktNum;
	UINT32 bcPktNum;
};
#endif /* MWDS */

#ifdef EAP_STATS_SUPPORT
struct tx_stats_monitor {
	UINT32 fail_cnt;
	UINT32 total_cnt;
	ULONG contd_fail_cnt;
	ULONG last_calc_time;
	UINT32 per_err_times;
};
#endif /* EAP_STATS_SUPPORT */


typedef struct _MAC_TABLE_ENTRY {
	STA_TR_ENTRY tr_entry; /* Keep in first */
	UINT32 EntryType;
	UINT32 EntryState;
	UINT32 EntryDelState;
	struct wifi_dev *wdev;
	PVOID pAd;
	struct _MAC_TABLE_ENTRY *pNext;

	struct caps_info cap;
	ULONG ClientStatusFlags;
	ULONG cli_cap_flags;

	union _HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode;
	union _HTTRANSMIT_SETTING MinHTPhyMode;
	struct phy_params phy_param;
	UCHAR sta_vht_ch_bw;
	UCHAR sta_he_ch_bw;
	BOOLEAN vht_ie_flag;
	BOOLEAN	ht_ie_flag;
	BOOLEAN he_ie_flag;
#ifdef DATA_TXPWR_CTRL
	BOOLEAN DataTxPwrEn;	//Indicates that data power ctrl is in effect
	INT8 PowerOffset;	//codesize insufficient just support per sta per txpwr offset
#endif
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN IsRekeyGTK;	/* Determine whether this is rekey GTK */
	BOOLEAN IsRekeyIGTK;	/* Determine whether this is rekey IGTK */
	BOOLEAN IsRekeyBIGTK;	/* Determine whether this is rekey BIGTK */
#endif /* CONFIG_STA_SUPPORT */
#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
	INT16 CurRssi; /* Current Average RSSI */
#endif
	/*
		wcid:

		tr_tb_idx:

		func_tb_idx used to indicate following index:
			in StaCfg
			in pAd->MeshTab
			in WdsTab.MacTab

		apidx: should remove this
	*/
#ifdef OCE_SUPPORT
#ifdef OCE_FILS_SUPPORT
	struct fils_info filsInfo;
#endif /* OCE_FILS_SUPPORT */
	struct oce_info oceInfo;
#endif /* OCE_SUPPORT */

	UINT16 wcid;
	UINT16 tr_tb_idx;
	UCHAR func_tb_idx;
	UCHAR apidx;		/* MBSS number */

	BOOLEAN isRalink;
	/* Flag if this is Atheros chip that has IOT problem.
	   We need to turn on RTS/CTS protection. */
	BOOLEAN bIAmBadAtheros;

#ifdef MBO_SUPPORT
	BOOLEAN bIndicateNPC;
	BOOLEAN bIndicateCDC;
	BOOLEAN bindicate_NPC_event;
	BOOLEAN bindicate_CDC_event;
	MBO_STA_CH_PREF_CDC_INFO MboStaInfoNPC;
	MBO_STA_CH_PREF_CDC_INFO MboStaInfoCDC;
	BOOLEAN is_mbo_bndstr_sta;
#endif /* MBO_SUPPORT */
#ifdef A4_CONN
	UCHAR	a4_entry;		/* Determine if this entry act which A4 role */
	BOOLEAN roaming_entry;
#endif /* A4_CONN */

#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
	BOOLEAN bEnable4Addr;
#endif

#ifdef DYNAMIC_VLAN_SUPPORT
	UINT32 vlan_id;
#endif

#ifdef RADIUS_MAC_AUTH_SUPPORT
	BOOLEAN bAllowTraffic;
#endif

#ifdef MWDS
	UCHAR	MWDSEntry;		/* Determine if this entry act which MWDS role */
	BOOLEAN bSupportMWDS;	/* Determine If own MWDS capability */
	BOOLEAN bEnableMWDS;	/* Determine If do 3-address to 4-address */
	struct MWDS_STRUCT MWDSInfo;
#endif /* MWDS */

	UCHAR Addr[MAC_ADDR_LEN];
#ifdef CONFIG_AP_SUPPORT
	void *pMbss;
#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
	BOOLEAN bRoamingRefreshDone;
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/*
		STATE MACHINE Status
	*/
	USHORT Aid;	/* in range 1~2007, with bit 14~15 = b'11, e.g., 0xc001~0xc7d7 */
	SST Sst;
	AUTH_STATE AuthState;	/* for SHARED KEY authentication state machine used only */

	/* Rx status related parameters */
	RSSI_SAMPLE RssiSample;
#ifdef RT_CFG80211_SUPPORT
#endif
	UINT32 LastTxRate;
	UINT32 LastRxRate;
#ifdef CONFIG_MAP_SUPPORT
	UINT32 map_LastTxRate;
	UINT32 map_LastRxRate;
#endif
	SHORT freqOffset;		/* Last RXWI FOFFSET */
	SHORT freqOffsetValid;	/* Set when freqOffset field has been updated */

#ifdef AIR_MONITOR
	UCHAR mnt_idx;
	UCHAR mnt_band;
#endif /* AIR_MONITOR */


	/* WPA/WPA2 4-way database */
	UCHAR EnqueueEapolStartTimerRunning;	/* Enqueue EAPoL-Start for triggering EAP SM */
	struct _SECURITY_CONFIG SecConfig;
	UCHAR RSNIE_Len;
	UCHAR RSN_IE[MAX_LEN_OF_RSNIE];
	UCHAR CMTimerRunning;
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;	/* PrivacyFilter enum for 802.1X */

	UCHAR bssid[MAC_ADDR_LEN];
	BOOLEAN IsReassocSta;	/* Indicate whether this is a reassociation procedure */
	ULONG NoDataIdleCount;
#if defined(DFS_VENDOR10_CUSTOM_FEATURE) || defined(CCN34_SPLIT_MAC_SUPPORT)
	ULONG LastRxTimeCount;
#endif
#ifdef CCN67_BS_SUPPORT
	BOOLEAN ProbeWithhold;
#endif
	ULONG sleep_from;
	ULONG AssocDeadLine;
	UINT16 StationKeepAliveCount;	/* unit: second */
	USHORT CapabilityInfo;
	UCHAR PsMode;
	UCHAR FlgPsModeIsWakeForAWhile; /* wake up for a while until a condition */
	UCHAR VirtualTimeout; /* peer power save virtual timeout */

#ifdef WDS_SUPPORT
	BOOLEAN LockEntryTx;	/* TRUE = block to WDS Entry traffic, FALSE = not. */
#endif /* WDS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	struct _REPEATER_CLIENT_ENTRY *pReptCli;
	VOID *ProxySta;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	UINT32 StaConnectTime;	/* the live time of this station since associated with AP */
	UINT32 StaIdleTimeout;	/* idle timeout per entry */
	BOOLEAN sta_force_keep;
	UINT64	assoc_at;  /* the boot time of associated with AP */

#ifdef UAPSD_SUPPORT
	/* these UAPSD states are used on the fly */
	/* 0:AC_BK, 1:AC_BE, 2:AC_VI, 3:AC_VO */
	BOOLEAN bAPSDCapablePerAC[4];	/* for trigger-enabled */
	BOOLEAN bAPSDDeliverEnabledPerAC[4];	/* for delivery-enabled */


	UCHAR MaxSPLength;

	BOOLEAN bAPSDAllAC;	/* 1: all AC are delivery-enabled U-APSD */

	struct _QUEUE_HEADER UAPSDQueue[WMM_NUM_OF_AC];	/* queue for each U-APSD */
	USHORT UAPSDQIdleCount;	/* U-APSD queue timeout */

	struct _QUEUE_ENTRY *pUAPSDEOSPFrame;	/* the last U-APSD frame */
	USHORT UAPSDTxNum;	/* total U-APSD frame number */
	BOOLEAN bAPSDFlagEOSPOK;	/* 1: EOSP frame is tx by ASIC */
	BOOLEAN bAPSDFlagSPStart;	/* 1: SP is started */

	/* need to use unsigned long, because time parameters in OS is defined as
	   unsigned long */
	unsigned long UAPSDTimeStampLast;	/* unit: 1000000/OS_HZ */
	BOOLEAN bAPSDFlagSpRoughUse;	/* 1: use rough SP (default: accurate) */

	/* we will set the flag when PS-poll frame is received and
	   clear it when statistics handle.
	   if the flag is set when PS-poll frame is received then calling
	   statistics handler to clear it. */
	BOOLEAN bAPSDFlagLegacySent;	/* 1: Legacy PS sent but yet statistics handle */

#endif /* UAPSD_SUPPORT */

	UINT FIFOCount;
	UINT DebugFIFOCount;
	UINT DebugTxCount;

	/* ==================================================== */
	enum RATE_ADAPT_ALG rateAlg;
	UCHAR RateLen;
	UCHAR MaxSupportedRate;

	BOOLEAN bAutoTxRateSwitch;
	UCHAR CurrTxRate;
	UCHAR CurrTxRateIndex;
	UCHAR lastRateIdx;
	UCHAR *pTable;	/* Pointer to this entry's Tx Rate Table */
#ifdef ANDLINK_FEATURE_SUPPORT
	UINT	ipaddr; /* In network order */
	UCHAR ipv6addr[16];/*IPV6 support*/
	RTMP_STRING hostname[32];/*device host name*/
#endif

	/* to record the each TX rate's quality. 0 is best, the bigger the worse. */
	USHORT TxQuality[MAX_TX_RATE_INDEX + 1];
	BOOLEAN fLastSecAccordingRSSI;
	UCHAR LastSecTxRateChangeAction; /* 0: no change, 1:rate UP, 2:rate down */
	CHAR LastTimeTxRateChangeAction; /* Keep last time value of LastSecTxRateChangeAction */
	ULONG LastTxOkCount; /* TxSuccess count in last Rate Adaptation interval */
	UCHAR LastTxPER;	/* Tx PER in last Rate Adaptation interval */
	UCHAR PER[MAX_TX_RATE_INDEX + 1];
	UINT32 CurrTxRateStableTime;	/* # of second in current TX rate */
	UCHAR TxRateUpPenalty;	/* extra # of second penalty due to last unstable condition */

	BOOLEAN fgGband256QAMSupport;
	UCHAR SupportRateMode; /* 1: CCK 2:OFDM 4: HT, 8:VHT */
	UINT8 SupportCCKMCS;
	UINT8 SupportOFDMMCS;
#ifdef DOT11_N_SUPPORT
	UINT32 SupportHTMCS;
#ifdef DOT11_VHT_AC
	UINT16 SupportVHTMCS1SS;
	UINT16 SupportVHTMCS2SS;
	UINT16 SupportVHTMCS3SS;
	UINT16 SupportVHTMCS4SS;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

#ifdef MFB_SUPPORT
	/* last legal mfb which is used to set rate */
	UCHAR lastLegalMfb;
	/* purpose: true when mfb has changed but the new mfb is not adopted for Tx */
	BOOLEAN isMfbChanged;
	struct _RTMP_RA_LEGACY_TB *LegalMfbRS;
	BOOLEAN fLastChangeAccordingMfb;
	NDIS_SPIN_LOCK fLastChangeAccordingMfbLock;
	/* Tx MRQ */
	BOOLEAN toTxMrq;
	UCHAR msiToTx, mrqCnt;	/*mrqCnt is used to count down the inverted-BF mrq to be sent */
	/* Rx mfb */
	UCHAR pendingMfsi;
	/* Tx MFB */
	BOOLEAN toTxMfb;
	UCHAR	mfbToTx;
	UCHAR	mfb0, mfb1;
#endif	/* MFB_SUPPORT */
#ifdef TXBF_SUPPORT
	UCHAR		eTxBfEnCond;
	UCHAR		iTxBfEn;
	COUNTER_TXBF TxBFCounters;		/* TxBF Statistics */
#ifdef MT_MAC
	VENDOR_BF_SETTING rStaBfRecVendorUpdate;
	TXBF_PFMU_STA_INFO rStaRecBf;
	BFEE_STA_REC rStaRecBfee;
	UCHAR	has_oui; /* Indication of STA's OUI when association */
#endif
#endif /* TXBF_SUPPORT */

#ifdef VHT_TXBF_SUPPORT
	UINT8 snd_dialog_token;
#ifdef SOFT_SOUNDING
	BOOLEAN snd_reqired;
	union _HTTRANSMIT_SETTING snd_rate;
#endif /* SOFT_SOUNDING */
#endif /* VHT_TXBF_SUPPORT */

	UINT32 OneSecTxNoRetryOkCount;
	UINT32 OneSecTxRetryOkCount;
	UINT32 OneSecTxFailCount;
	UINT32 OneSecRxLGICount;		/* unicast-to-me Long GI count */
	UINT32 OneSecRxSGICount;		/* unicast-to-me Short GI count */
	UINT32 ContinueTxFailCnt;
	ULONG TimeStamp_toTxRing;

	/*==================================================== */
	EXT_CAP_INFO_ELEMENT ext_cap;
	struct _vendor_ie_cap vendor_ie;
#ifdef DOT11_N_SUPPORT
	HT_CAPABILITY_IE HTCapability;
	struct BA_INFO ba_info;
	UCHAR MpduDensity;
	UCHAR MaxRAmpduFactor;
	UCHAR AMsduSize;
	UINT32 amsdu_limit_len;
	UINT32 amsdu_limit_len_adjust;
	UCHAR MmpsMode;		/* MIMO power save mode. */
	BOOLEAN agg_err_flag;
	UINT32 tx_per;
	UINT32 winsize_limit;

#ifdef DOT11N_DRAFT3
	UCHAR BSS2040CoexistenceMgmtSupport;
	BOOLEAN bForty_Mhz_Intolerant;
#endif /* DOT11N_DRAFT3 */

#ifdef DOT11_VHT_AC
	VHT_CAP_IE vht_cap_ie;

	/* only take effect if ext_cap.operating_mode_notification = 1 */
	BOOLEAN force_op_mode;
	OPERATING_MODE operating_mode;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_DOT11V_WNM
	UCHAR BssTransitionManmtSupport;
#endif /* CONFIG_DOT11V_WNM */

	BOOLEAN bWscCapable;
	UCHAR Receive_EapolStart_EapRspId;

	UINT32 TXMCSExpected[MAX_MCS_SET];
	UINT32 TXMCSSuccessful[MAX_MCS_SET];
	UINT32 TXMCSFailed[MAX_MCS_SET];
	UINT32 TXMCSAutoFallBack[MAX_MCS_SET][MAX_MCS_SET];

#ifdef CONFIG_STA_SUPPORT
	ULONG LastBeaconRxTime;
#endif /* CONFIG_STA_SUPPORT */


#ifdef DOT11R_FT_SUPPORT
	FT_MDIE_INFO MdIeInfo;
	FT_FTIE_INFO FtIeInfo;

	UINT8 InitialMDIE[5];
	UINT8 InitialFTIE[256];
	UINT InitialFTIE_Len;

	UCHAR FT_PMK_R0[32];
	UCHAR FT_PMK_R0_NAME[16];
	UCHAR FT_PMK_R1[32];
	UCHAR FT_PMK_R1_NAME[16];
	UCHAR PTK_NAME[16];

	UCHAR FT_UCipher[4];
	UCHAR FT_Akm[4];
	UCHAR FT_PTK[LEN_MAX_PTK]; /* 512 bits max, KCK(16)+KEK(16)+TK(32) */
	UCHAR FT_Status;
	UCHAR FT_R1kh_CacheMiss_Times;

#ifdef R1KH_HARD_RETRY
	UCHAR FT_R1kh_CacheMiss_Hard;
	RTMP_OS_COMPLETION ack_r1kh;
#endif /* R1KH_HARD_RETRY */

#ifdef HOSTAPD_11R_SUPPORT
	FT_INFO Auth_FtInfo;
#endif

#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
	RRM_EN_CAP_IE RrmEnCap;
#endif /* DOT11K_RRM_SUPPORT */

#ifdef CONFIG_MAP_SUPPORT
	UCHAR assoc_req_frame[ASSOC_REQ_LEN];
	USHORT assoc_req_len;
	UCHAR DevPeerRole;
	UCHAR cur_rssi_status;
	UCHAR pre_rssi_status;
	BOOLEAN isTriggerSteering;
	UCHAR pre_traffic_mode;
#ifdef MAP_R2
	UCHAR profile;
#endif
	UINT32 TxBytesMAP;
	UINT32 RxBytesMAP;
	UINT16 DisconnectReason;
#endif
	UINT32 TxRxTime[4][2];
	UINT32 wrapArTxRxTime[4][2];
	LARGE_INTEGER TxPackets;
	LARGE_INTEGER RxPackets;
#ifdef CONFIG_AP_SUPPORT
#ifdef TXRX_STAT_SUPPORT
	LARGE_INTEGER TxDataPacketCount;
	LARGE_INTEGER TxDataPacketByte;
	LARGE_INTEGER TxUnicastPktCount;
	LARGE_INTEGER TxDataPacketCount1SecValue;
	LARGE_INTEGER TxDataPacketByte1SecValue;
	LARGE_INTEGER LastTxDataPacketCountValue;
	LARGE_INTEGER LastTxDataPacketByteValue;
	LARGE_INTEGER TxDataPacketCountPerAC[4];	/*per access category*/
	LARGE_INTEGER TxMgmtPacketCount;
	LARGE_INTEGER RxDataPacketCount;
	LARGE_INTEGER RxDataPacketByte;
	LARGE_INTEGER RxUnicastPktCount;
	LARGE_INTEGER RxUnicastByteCount;
	LARGE_INTEGER RxDataPacketCount1SecValue;
	LARGE_INTEGER RxDataPacketByte1SecValue;
	LARGE_INTEGER LastRxDataPacketCountValue;
	LARGE_INTEGER LastRxDataPacketByteValue;
	LARGE_INTEGER RxDataPacketCountPerAC[4];/*per access category*/
	LARGE_INTEGER RxMgmtPacketCount;
	LARGE_INTEGER RxCtrlPacketCount;
	LARGE_INTEGER RxDecryptionSuccessCount;
	LARGE_INTEGER RxDecryptionErrorCount;
	LARGE_INTEGER RxMICErrorCount;
	LARGE_INTEGER TxPacketsRetryCnt;
	ULONG RxLastMgmtPktRate;
	CHAR LastDataPktRssi[MAX_RSSI_LEN];
	CHAR LastMgmtPktRssi[MAX_RSSI_LEN];
	UINT32 LastOneSecTxTotalCountByWtbl;
	UINT32 LastOneSecTxFailCountByWtbl;
	UINT32 LastOneSecPER;
	UINT32 TxSuccessByWtbl;/*data/unicast same variable, updated per sec*/
#ifdef WIFI_IAP_STA_DUMP_FEATURE
	UINT32 TxFailCountByWtbl;/*tx fail total count, updated per sec*/
#endif/*WIFI_IAP_STA_DUMP_FEATURE*/
#endif
#endif /* CONFIG_AP_SUPPORT */
	UINT64 TxBytes;
	UINT64 RxBytes;
	UINT64 TxFailCount;

	ULONG OneSecTxBytes;
	ULONG OneSecRxBytes;
#ifdef ANTENNA_DIVERSITY_SUPPORT
	ULONG ant_div_rx_bytes;
	ULONG ant_div_tx_bytes;
#endif
	ULONG AvgTxBytes;
	ULONG AvgRxBytes;
	ULONG one_sec_tx_pkts;
	ULONG avg_tx_pkts;
	ULONG avg_tx_mpdu_pkts;
	ULONG avg_tx_pkt_len;
	ULONG avg_rx_pkts;
	ULONG avg_rx_pkt_len;
	ULONG one_sec_tx_succ_pkts;
	ULONG one_sec_tx_mpdu_pkts;
	ULONG one_sec_tx_mpdu_succ_pkts;
	ULONG txMsduDropCnt;
	ULONG txMsduRetryCnt;
#if (defined(ANDLINK_FEATURE_SUPPORT) && defined(ANDLINK_V4_0))
	ULONGLONG andlink_tx_rate_rt[ANDLINK_IF_MAX];
	ULONGLONG andlink_rx_rate_rt[ANDLINK_IF_MAX];
	ULONGLONG andlink_avg_tx_rate[ANDLINK_IF_MAX];
	ULONGLONG andlink_avg_rx_rate[ANDLINK_IF_MAX];
	ULONGLONG andlink_max_tx_rate[ANDLINK_IF_MAX];
	ULONGLONG andlink_max_rx_rate[ANDLINK_IF_MAX];
	ULONGLONG andlink_sample_tx_bytes[ANDLINK_IF_MAX];
	ULONGLONG andlink_sample_rx_bytes[ANDLINK_IF_MAX];
	ULONGLONG andlink_period_tx_bytes[ANDLINK_IF_MAX];
	ULONGLONG andlink_period_rx_bytes[ANDLINK_IF_MAX];
#endif/*ANDLINK_FEATURE_SUPPORT & ANDLINK_V4_0*/

#ifdef RX_COUNT_DETECT
	ULONG one_sec_rx_pkts;
#endif /* RX_COUNT_DETECT */
#ifdef VOW_SUPPORT
	UINT mcliTcpCnt;
	UINT mcliTcpAckCnt;
#endif /* VOW_SUPPORT */
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_WPA3_SUPPORT)
	IE_LISTS *ie_list;
#endif
#ifdef RT_CFG80211_SUPPORT
	MLME_QUEUE_ELEM Elem_for_hostapd_owe;
#endif
#ifdef DABS_QOS
	unsigned short qos_tbl_idx[8];
#endif
#ifdef CFG_RED_SUPPORT
	LARGE_INTEGER msdu_sdo_drop;
	LARGE_INTEGER mpdu_mac_drop;
	LARGE_INTEGER msdu_sdo_drop_per_ac[4];
	LARGE_INTEGER mpdu_mac_drop_per_ac[4];
	LARGE_INTEGER msdu_tot_txcnt;
	LARGE_INTEGER msdu_tot_txcnt_per_ac[4];
#endif
	LARGE_INTEGER mpdu_attempts;
	LARGE_INTEGER mpdu_retries;
	LARGE_INTEGER mpdu_xretries;
#ifdef EAP_STATS_SUPPORT
	UINT32	tx_latency_min;
	UINT32	tx_latency_max;
	UINT32	tx_latency_avg;
#ifdef EAP_ENHANCE_STATS_SUPPORT
	UINT32	air_latency_min;
	UINT32	air_latency_max;
	UINT32	air_latency_avg;
	UINT8	tx_cnt_min;
	UINT8	tx_cnt_max;
	UINT8	tx_cnt_avg;
#endif /* EAP_ENHANCE_STATS_SUPPORT */
	struct tx_stats_monitor txm;
#endif
	ULONG ChannelQuality;	/* 0..100, Channel Quality Indication for Roaming */
#if defined(CONFIG_HOTSPOT_R2) || defined(QOS_R1)
	UCHAR				QosMapSupport;
	UCHAR				DscpExceptionCount;
	USHORT				DscpRange[8];
	USHORT				DscpException[21];
	UCHAR				MSCSSupport;
#endif
#ifdef QOS_R2
	UCHAR DSCPPolicyEnable;
#endif
#ifdef CONFIG_HOTSPOT_R2
	UCHAR				IsWNMReqValid;
	struct wnm_req_data	*ReqData;
	struct _sta_hs_info hs_info;
	UCHAR OSEN_IE_Len;
	UCHAR OSEN_IE[MAX_LEN_OF_RSNIE];
#endif /* CONFIG_HOTSPOT_R2 */
#ifdef CONFIG_HOTSPOT_R3
	STA_HS_CONSORTIUM_OI hs_consortium_oi;
#endif /* CONFIG_HOTSPOT_R3 */
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM)
	UCHAR				IsBTMReqValid;
	UCHAR				IsKeep;
	UINT16				BTMDisassocCount;
	BOOLEAN				bBSSMantSTASupport;
	struct btm_req_data	*ReqbtmData;
#endif

	BOOLEAN bACMBit[WMM_NUM_OF_AC];

	RA_ENTRY_INFO_T RaEntry;
	RA_INTERNAL_INFO_T	RaInternal;
	UINT32	ConnectionType;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	BOOLEAN bTxPktChk;
	UINT8 TxStatRspCnt;
	UINT32 TotalTxSuccessCnt;	/* Accumulated Tx success count from N9 (WTBL) */
#endif

#ifdef HTC_DECRYPT_IOT
	/* keep the ICV Error cnt of HTC Rx Cnt */
	UINT32 HTC_ICVErrCnt;
	/* when reach the threshold, force set the WTBL.DW2.AAD_OM to 1 */
	UCHAR HTC_AAD_OM_Force;
	/* settling time (1 count 1 second) for start count HTC_ICVErrCnt */
	UINT32 HTC_AAD_OM_CountDown;
	/* Treat the entry's AAD_OM setting is correct now */
	UCHAR HTC_AAD_OM_Freeze;
	UINT32 HTC_AAD_OM_False_Trigger;
	UINT32 HTC_AAD_OM_Valid_Trigger;
#endif /* HTC_DECRYPT_IOT */

#ifdef WH_EVENT_NOTIFIER
	UCHAR custom_ie_len;			   /* Length of Vendor Information Element */
	UCHAR custom_ie[CUSTOM_IE_TOTAL_LEN];  /* Vendor Information Element  */
	StaActivityItem tx_state;			  /* Station's tx state record */
	StaActivityItem rx_state;			  /* Station's rx state record */
#endif /* WH_EVENT_NOTIFIER */
	UINT16 DISABLE_PN_CHK;
	ULONG PNChkFailCnt;
#ifdef GN_MIXMODE_SUPPORT
	BOOLEAN FlgIs11bSta;
#endif /*GN_MIXMODE_SUPPORT*/
#ifdef PN_UC_REPLAY_DETECTION_SUPPORT
	UINT64 CCMP_UC_PN[NUM_OF_TID];
#endif /* PN_UC_REPLAY_DETECTION_SUPPORT */
	UINT64 CCMP_BC_PN[4];
	BOOLEAN Init_CCMP_BC_PN_Passed[4];
	BOOLEAN AllowUpdateRSC;
#ifdef REDUCE_TCP_ACK_SUPPORT
	BOOLEAN RACKEnalbedSta;
#endif

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	struct TWT_PEER_CTRL_T twt_ctrl;
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
	BOOLEAN WtblSetFlag; /* This will reflect whetherWtbl entry is created or not */
	RTMP_OS_COMPLETION WtblSetDone; /* To intimate status when Wtbl entry is created */

#ifdef DOT11_HE_AX
	UCHAR ucMUEdcaUpdateCnt;

	/*
	 *  Store MU EDCA params for each ACs in BSS info
	 *  Use the same format as the update cmd for memory copy
	 */
	struct he_mu_edca_params arMUEdcaParams[ACI_AC_NUM];

	/* Spatial Reuse Parameter Set for the BSS */
	UCHAR SRControl;
	UCHAR NonSRGObssPdMaxOffset;
	UCHAR SRGObssPdMinOffset;
	UCHAR SRGObssPdMaxOffset;
	UINT64 SRGBSSColorBitmap;
	UINT64 SRGPartialBSSIDBitmap;
	UINT8 aucHeMacCapInfo[6];
	UINT8 aucHePhyCapInfo[11];
	BOOLEAN update_he_maxra; /* Used to check whether to update HE or EHT max rate
								in UpdteMaxRA() function */
#endif

#ifdef RATE_PRIOR_SUPPORT
	ULONG McsTotalRxCount;
	ULONG McsLowRateRxCount;
#endif/*RATE_PRIOR_SUPPORT*/
#ifdef IGMP_TVM_SUPPORT
	UCHAR TVMode;
#endif /* IGMP_TVM_SUPPORT */

#ifdef HOSTAPD_PMKID_IN_DRIVER_SUPPORT
	UCHAR PmkidByHostapd[LEN_PMKID];
#endif /*HOSTAPD_PMKID_IN_DRIVER_SUPPORT*/
#ifdef RACTRL_LIMIT_MAX_PHY_RATE
	BOOLEAN fgRaLimitPhyRate;
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */
	BOOLEAN sta_rec_valid;

#ifdef MSCS_PROPRIETARY
	bool dabs_cfg;
	UINT16 APRandNum;
	UINT16 STARandNum;
	UINT8 dabs_trans_id;
	RALINK_TIMER_STRUCT DABSRetryTimer;
	ULONG DABSTimerFlag;
#endif
	ULONG	TxRetriedPktCount;
#ifdef DOT11_EHT_BE
	UINT8 mlo_join; /* only for auth use */
	struct eht_mlo_t mlo;
	struct eht_cfg eht_cfg;
	UINT16 eht_mac_cap;
	struct mld_starec *mld_sta;
	UINT64 eht_phy_cap;
	UINT64 eht_phy_cap_ext;
	struct eht_support_mcs_nss eht_support_mcs_nss;
	UINT8   EmlsrBitmap;
	BOOLEAN update_eht_maxra; /* Used to check whether to update HE or EHT max rate
								in UpdteMaxRA() function */
	struct mld_entry_t *mld_entry;
#endif /* DOT11_EHT_BE */
#ifdef PRE_CFG_SUPPORT
	BOOLEAN fgPreCfgRunning;
#endif /* PRE_CFG_SUPPORT */
	UINT8  rcpi0;
	UINT8  rcpi1;
	UINT8  rcpi2;
	UINT8  rcpi3;
#ifdef CUSTOMISED_MGMT_TRANS_SUPPORT
	UINT16 del_reason;
#endif
	UINT8 sw_pn[LEN_PN];
#ifdef IGMP_SNOOP_SUPPORT
	UINT64 M2U_TxPackets;
	UINT64 M2U_TxBytes;
#ifdef IGMP_SNOOPING_OFFLOAD
	UINT64 one_sec_M2U_TxPackets;
	UINT64 one_sec_M2U_TxBytes;
#endif

#endif
#ifdef CONFIG_FAST_NAT_SUPPORT
	UINT8 ForceSwPath;
#endif
#ifdef MLR_SUPPORT
	UINT8 MlrMode;
	UINT8 MlrCurState;
#endif
#if defined(CONFIG_MAP_SUPPORT) || defined(WAPP_SUPPORT)
	UINT8  is_cli_join_sent;
	UINT8 MAP_Enqueue_single;
#ifdef MAP_R6
	ULONG current_reassc_time;
	UCHAR current_reassoc_mac[MAC_ADDR_LEN];
#endif /* MAP_R6 */
#endif
} MAC_TABLE_ENTRY, *PMAC_TABLE_ENTRY;

struct mld_mac_table_entry_t {
};

struct mld_entry_ext_t {
	struct mld_mac_table_entry_t mac_entry;
	struct mld_tr_entry_t tr_entry;
};

typedef enum _MAC_ENT_STATUS_ {
	/* fAnyStationInPsm */
	MAC_TB_ANY_PSM = 0x1,
	/*
		fAnyStationBadAtheros
		Check if any Station is atheros 802.11n Chip.
		We need to use RTS/CTS with Atheros 802,.11n chip.
	*/
	MAC_TB_ANY_ATH = 0x2,
	/*
		fAnyTxOPForceDisable
		Check if it is necessary to disable BE TxOP
	*/
	MAC_TB_FORCE_TxOP = 0x4,
	/*
		fAllStationAsRalink
		Check if all stations are ralink-chipset
	*/
	MAC_TB_ALL_RALINK = 0x8,
	/*
		fAnyStationIsLegacy
		Check if I use legacy rate to transmit to my BSS Station
	*/
	MAC_TB_ANY_LEGACY = 0x10,
	/*
		fAnyStationNonGF
		Check if any Station can't support GF
	*/
	MAC_TB_ANY_NON_GF = 0x20,
	/* fAnyStation20Only */
	MAC_TB_ANY_HT20 = 0x40,
	/*
		fAnyStationMIMOPSDynamic
		Check if any Station is MIMO Dynamic
	*/
	MAC_TB_ANY_MIMO_DYNAMIC = 0x80,
	/*
		fAnyBASession
		Check if there is BA session.  Force turn on RTS/CTS
	*/
	MAC_TB_ANY_BA = 0x100,
	/* fAnyStaFortyIntolerant */
	MAC_TB_ANY_40_INTOlERANT = 0x200,
	/*
		fAllStationGainGoodMCS
		Check if all stations more than MCS threshold
	*/
	MAC_TB_ALL_GOOD_MCS = 0x400,
	/*
		fAnyStationIsHT
		Check if still has any station set the Intolerant bit on!
	*/
	MAC_TB_ANY_HT = 0x800,
	/* fAnyWapiStation */
	MAC_TB_ANY_WAPI = 0x1000,
} MAC_ENT_STATUS;


typedef struct _MAC_TABLE {
	MAC_TABLE_ENTRY * Hash[HASH_TABLE_SIZE];
	MAC_TABLE_ENTRY *Content;/*[MAX_LEN_OF_MAC_TABLE];*/
	struct mld_entry_ext_t *mld_entry_ext; /*[MLD_STA_MAX_NUM]*/

	/*
		Be care in mgmt_entrytb.c  MacTableReset() will NdisZeroMemory(&pAd->MacTab.Size,
		sizeof(MAC_TABLE)-offsetof(MAC_TABLE, Size));
		above need to be backup, klock's warnnig @118489 should be mark as not an issue.
	*/
	UINT16 Size;
#ifdef CCN67_BS_SUPPORT
	UINT16 ClientCount;
#endif
	struct _QUEUE_HEADER McastPsQueue;
	ULONG PsQIdleCount;
	MAC_ENT_STATUS sta_status;

	BOOLEAN fAnyStationInPsm;
	/*
	   Check if any Station is atheros 802.11n Chip.
	   We need to use RTS/CTS with Atheros 802,.11n chip.
	*/
	BOOLEAN fAnyStationBadAtheros;
	BOOLEAN fAnyTxOPForceDisable;	/* Check if it is necessary to disable BE TxOP */
	BOOLEAN fCurrentStaBw40;		/* Check if only one STA w/ BW40 */
#ifdef DOT11_N_SUPPORT
	/* Check if I use legacy rate to transmit to my BSS Station */
	BOOLEAN fAnyStationIsLegacy;
	/* Check if any Station can't support GF. */
	BOOLEAN fAnyStationNonGF[CFG_WIFI_RAM_BAND_NUM];
	/* Check if any Station can't support GF. */
	BOOLEAN fAnyStation20Only;
	/* Check if any Station is MIMO Dynamic */
	BOOLEAN fAnyStationMIMOPSDynamic;
	/* Check if there is BA session.  Force turn on RTS/CTS */
	BOOLEAN fAnyBASession;
	/* Check if still has any station set the Intolerant bit on! */
	BOOLEAN fAnyStaFortyIntolerant;
	/* Check if all stations more than MCS threshold */
	BOOLEAN fAllStationGainGoodMCS;
#endif /* DOT11_N_SUPPORT */


	USHORT MsduLifeTime; /* life time for PS packet */
} MAC_TABLE, *PMAC_TABLE;



typedef UINT32 (*entrytb_traversal_func)(struct _MAC_TABLE_ENTRY *entry, void *cookie);

#ifdef DOT11_EHT_BE
enum ENTRY_SYNC_MAIN_CAT {
	ENTRY_SYNC_MAIN_SETUPLINK_CONN_MLD_LEVEL = 0,
	ENTRY_SYNC_MAIN_SETUPLINK_CONN_LINK_LEVEL = 1,
	ENTRY_SYNC_MAIN_SETUPLINK_NOT_CONN_MLD_LEVEL = 2,
	ENTRY_SYNC_MAIN_SETUPLINK_NOT_CONN_LINK_LEVEL = 3,
};

enum ENTRY_SYNC_SUB_CAT {
	ENTRY_SYNC_SUB_APPS		= 0,
	ENTRY_SYNC_SUB_QOS		= 1,
	ENTRY_SYNC_SUB_SST		= 2,
	ENTRY_SYNC_SUB_AF		= 3,
	ENTRY_SYNC_SUB_HT_CAP		= 4,
	ENTRY_SYNC_SUB_HE_CAP		= 5,
	ENTRY_SYNC_SUB_AMSDU		= 6,
	ENTRY_SYNC_SUB_EHT_CAP		= 7,
	ENTRY_SYNC_SUB_VHT_CAP		= 8,
	ENTRY_SYNC_SUB_SR		= 9,
	ENTRY_SYNC_SUB_MU_EDCA   = 10
};
#endif /* DOT11_EHT_BE */

#define ISEntryDelSync(_pentry)	(_pentry->EntryDelState == ENTRY_STATE_SYNC)

INLINE struct _MAC_TABLE_ENTRY *entry_get(
	struct _RTMP_ADAPTER *mac_ad, UINT16 wcid);
INLINE UCHAR *entry_addr_get(
	struct _RTMP_ADAPTER *mac_ad, UINT16 wcid);
INLINE struct wifi_dev *entry_wdev_get(
	struct _RTMP_ADAPTER *mac_ad, UINT16 wcid);
INLINE UCHAR entry_func_tb_idx_get(
	struct _RTMP_ADAPTER *mac_ad, UINT16 wcid);
INLINE struct _STA_TR_ENTRY *tr_entry_get(
	struct _RTMP_ADAPTER *mac_ad, UINT16 wcid);

void entrytb_aid_bitmap_init(struct _aid_info *aid_info);
void entrytb_aid_bitmap_free(struct _aid_info *aid_info);
void entrytb_aid_bitmap_reserve(struct _RTMP_ADAPTER *ad, struct _aid_info *aid_info);
UINT16 _entrytb_aid_aquire(struct _aid_info *aid_info);
uint16_t entrytb_aid_aquire(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 ent_type, uint16_t mld_sta_idx);

INT show_aid_info(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

typedef struct _entrytb_aid_search_t {
	UINT16 aid_search;/*the AID that would like to search in the entrytb*/
	struct _MAC_TABLE_ENTRY *entry;/*the returned entry once the AID matched*/
} entrytb_aid_search_t;

UINT32 traversal_func_find_entry_by_aid(struct _MAC_TABLE_ENTRY *entry, void *cookie);
UINT32 traversal_func_dump_entry_rate_by_aid(struct _MAC_TABLE_ENTRY *entry, void *cookie);
UINT32 traversal_func_dump_entry_psm_by_aid(struct _MAC_TABLE_ENTRY *entry, void *cookie);
typedef struct _entrytb_bss_idx_search_t {
	UINT32 bss_idx;/*the BSS IDX that would like to search in the entrytb*/
	UINT32 need_print_field_name;
} entrytb_bss_idx_search_t;
UINT32 traversal_func_dump_entry_associated_to_bss(struct _MAC_TABLE_ENTRY *entry, void *cookie);

UINT32 entrytb_traversal(struct _RTMP_ADAPTER *ad, entrytb_traversal_func func, void *cookie);
void seq_ctrl_init(struct seq_ctrl_t *seq_ctrl);

#ifdef DOT11_EHT_BE
void entrytb_mlo_sync(struct _MAC_TABLE_ENTRY *entry);
#endif /* DOT11_EHT_BE */
void entry_del_hw_ppe_entry(struct _RTMP_ADAPTER *pad, unsigned char *mac);

int mld_entry_ext_get(struct _MAC_TABLE_ENTRY *entry, struct mld_entry_ext_t **mld_entry_ext);
struct mld_entry_ext_t *mld_entry_ext_get_by_idx(
	struct _RTMP_ADAPTER *ad, UINT16 mld_sta_idx);

void entry_table_init(struct physical_device *device);
void entry_table_deinit(struct physical_device *device);
void pause_bitmap_init(struct physical_device *device);
void pause_bitmap_deinit(struct physical_device *device);
#endif /*_MGMT_ENTRYTB_H_*/


