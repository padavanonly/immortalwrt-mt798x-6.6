#ifndef __AP_CFG_H__
#define __AP_CFG_H__

#include "cfg80211/cfg80211.h"

INT RTMPAPPrivIoctlSet(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * pIoctlCmdStr);

INT RTMPAPPrivIoctlShow(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * pIoctlCmdStr);

#ifdef VENDOR_FEATURE6_SUPPORT
VOID RTMPAPGetAssoMacTable(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * pIoctlCmdStr);
#endif /* VENDOR_FEATURE6_SUPPORT */

#if defined(INF_AR9)
#if defined(AR9_MAPI_SUPPORT)
INT RTMPAPPrivIoctlAR9Show(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * pIoctlCmdStr);

VOID RTMPAR9IoctlGetMacTable(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetSTAT2(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetRadioDynInfo(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /*AR9_MAPI_SUPPORT*/
#endif/* INF_AR9 */

INT RTMPAPSetInformation(
	IN	PRTMP_ADAPTER	pAd,
	IN	OUT	RTMP_IOCTL_INPUT_STRUCT * rq,
	IN	INT				cmd);

INT RTMPAPQueryInformation(
	IN	PRTMP_ADAPTER       pAd,
	IN	OUT	RTMP_IOCTL_INPUT_STRUCT * rq,
	IN	INT                 cmd);

VOID RTMPIoctlStatistics(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

INT RTMPIoctlRXStatistics(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetMacTableStaInfo(
        IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPAPIoctlE2PROM(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_IOCTL_INPUT_STRUCT * wrq);

INT RTMPPhyState(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq);

VOID RtmpDrvMaxRateGet(
	IN	VOID					*pReserved,
	IN	UINT8					MODE,
	IN	UINT8					ShortGI,
	IN	UINT8					BW,
	IN	UINT8					MCS,
	IN	UINT8					Antenna,
	OUT	UINT64 *pRate);

#ifdef WSC_AP_SUPPORT
VOID RTMPGetCurrentCred(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);
VOID RTMPIoctlWscProfile(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlWscProfile(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);
/*add by woody */
#if defined(INF_AR9)
#if defined(AR9_MAPI_SUPPORT)
VOID RTMPAR9IoctlWscProfile(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlWscPINCode(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);


VOID RTMPIoctlWscStatus(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetWscDynInfo(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetWscRegsDynInfo(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif/*AR9_MAPI_SUPPORT*/
#endif/* INF_AR9 */
#endif /* WSC_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
VOID RTMPIoctlQueryBaTable(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /* DOT11_N_SUPPORT */

#ifdef DOT1X_SUPPORT
VOID RTMPIoctlAddPMKIDCache(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlSetIdleTimeout(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlQueryStaAid(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlQueryStaRsn(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);


#ifdef RADIUS_ACCOUNTING_SUPPORT
VOID RTMPIoctlQueryStaData(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /* RADIUS_ACCOUNTING_SUPPORT */

#ifdef RADIUS_MAC_ACL_SUPPORT
PRT_802_11_RADIUS_ACL_ENTRY RadiusFindAclEntry(
	PLIST_HEADER            pCacheList,
	IN      PUCHAR          pMacAddr);

VOID RTMPIoctlAddRadiusMacAuthCache(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlDelRadiusMacAuthCache(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlClearRadiusMacAuthCache(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* DOT1X_SUPPORT */

INT Set_AP_Daemon_Status(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 WorkSpaceID,
	IN BOOLEAN Status);

INT Send_ANQP_Rsp(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * PeerMACAddr,
	IN RTMP_STRING * ANQPReq,
	IN UINT32 ANQPReqLen);

INT	ApCfg_Set_PerMbssMaxStaNum_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN INT				apidx,
	IN RTMP_STRING * arg);

#ifdef HOSTAPD_HS_R2_SUPPORT
INT Set_QosMap(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg);
#endif
INT	set_bss_max_idle_period_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_bss_max_idle_period_proc(IN PRTMP_ADAPTER pAd, IN unsigned int idle_period);
#endif /* RT_CFG80211_SUPPORT */

VOID restart_ap(void *wdev_obj);

#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
INT mtk_cancel_roc(PRTMP_ADAPTER pAd, struct wifi_dev *wdev);
INT mtk_start_roc(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, struct roc_req *roc);
INT mtk_send_offchannel_action_frame(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, struct action_frm_data *frm);

#endif /* CHANNEL_SWITCH_MONITOR_CONFIG */

INT Set_Unregister_Mbss(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Free_Mbss(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef CONFIG_6G_SUPPORT
INT Set_Lpi_en(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT Set_Max_ProbeRspCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

struct apcfg_parameters {
	LONG cfg_mode[2]; /*WirelessMode*/
	ULONG tx_power_percentage; /*TxPower*/
	ULONG tx_preamble; /*TxPreamble*/
	UINT32 conf_len_thld; /*RTSThreshold*/
	UINT32 oper_len_thld;
	UINT32 conf_frag_thld; /*FragThreshold*/
	UINT32 oper_frag_thld;
	BOOLEAN bEnableTxBurst;	/*TxBurst*/
	BOOLEAN bUseShortSlotTime;	/*ShortSlot*/
#ifdef DOT11_N_SUPPORT
	UCHAR conf_ht_bw; /*HT_BW*/
	UCHAR oper_ht_bw;
#ifdef DOT11N_DRAFT3
	BOOLEAN bBssCoexEnable; /*HT_BSSCoexistence*/
#endif
	UCHAR ht_tx_streams; /*HT_TxStream*/
	UCHAR ht_rx_streams; /*HT_RxStream*/
	UINT8 ba_decline; /*HT_BADecline*/
	UINT8 ba_en; /*HT_AutoBA*/
	UINT32 AmsduEnable; /*HT_AMSDU*/
	UINT32 ba_rx_wsize; /*HT_BAWinSize*/
	UINT32 ba_tx_wsize; /*HT_BAWinSize*/
	UCHAR ht_gi; /*HT_GI*/
	UCHAR ht_stbc; /*HT_STBC*/
	UCHAR ht_ldpc; /*HT_LDPC*/
	BOOLEAN bRdg; /*HT_RDG*/
#endif

	BOOLEAN HT_DisallowTKIP; /*HT_DisallowTKIP*/

#ifdef DOT11_VHT_AC
	UCHAR conf_vht_bw; /*VHT_BW*/
	UCHAR oper_vht_bw;
	UCHAR vht_sgi; /*VHT_SGI*/
	UCHAR vht_stbc; /*VHT_STBC*/
	UCHAR vht_bw_signal; /*VHT_BW_SIGNAL*/
	UCHAR vht_ldpc; /*VHT_LDPC*/
	BOOLEAN g_band_256_qam; /*G_BAND_256QAM*/
#endif

#ifdef DOT11_HE_AX
	UCHAR conf_he_bw; /*HE_BW*/
	UCHAR oper_he_bw;
#endif
#ifdef DOT11_EHT_BE
	UCHAR conf_eht_bw; /*EHT_BW*/
	UCHAR oper_eht_bw;
#endif

	BOOLEAN bIEEE80211H; /*IEEE80211H*/

#ifdef MT_DFS_SUPPORT
	BOOLEAN bDfsEnable; /*DfsEnable*/
#ifdef MT_BAND4_DFS_SUPPORT /*302502*/
	BOOLEAN band4DfsEnable; /*Band4DfsEnable*/
#endif
#endif

#ifdef BACKGROUND_SCAN_SUPPORT
	BOOLEAN DfsZeroWaitSupport; /*DfsZeroWait*/
#endif

	UCHAR MuOfdmaDlEnable;
	UCHAR MuOfdmaUlEnable;
	UCHAR MuMimoDlEnable;
	UCHAR MuMimoUlEnable;

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	ULONG ETxBfEnCond; /*ETxBfEnCond*/
#endif
#endif

	UINT32 ITxBfEn; /*ITxBfEn*/

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	ULONG MUTxRxEnable; /*MUTxRxEnable*/
#endif
#endif
	UCHAR channel;
	UCHAR CentralChannel;
	UCHAR ext_channel;

#ifdef CFG_SUPPORT_FALCON_MURU
	ULONG TamArbOpMode;
	ULONG HE_PpduFmt;
	ULONG HE_OfdmaUserNum;
#endif
	BOOLEAN vht_1024_qam;
	UCHAR HeOmiUlMuDataDisableRx;
	UCHAR HeErSuRxDisable;
	UINT8 wifi_cert;
};
INT Set_Quick_Channel_Switch_En_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef CONFIG_AP_SUPPORT
VOID ap_update_rf_ch_for_mbss(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct freq_oper *OperCh);
void ap_phy_rrm_init_byRf(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#endif
VOID MacTableResetNonMapWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#ifdef CONFIG_MAP_SUPPORT
INT Set_Bh_Bss_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
INT Set_Fh_Bss_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
INT Set_Map_Channel_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
INT Set_Map_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
#ifdef WSC_INCLUDED
int Fill_OID_WSC_PROFILE(
	IN PRTMP_ADAPTER pAd,
	UCHAR ifIndex,
	p_wsc_apcli_config_msg *wsc_config);
#endif /* WSC_INCLUDED */

void reset_mtk_map_vendor_ie(PRTMP_ADAPTER pAd, struct wifi_dev *wdev);
#ifdef MAP_R2
INT Set_Map_Bh_Primary_Vid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Map_Bh_Primary_Pcp_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Map_Bh_Vid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Map_Fh_Vid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Map_Transparent_Vid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MapR2_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif
#ifdef MAP_R3
INT Set_MapR3_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif
#ifdef MAP_R4
INT Set_MapR4_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif

#ifdef MAP_TS_TRAFFIC_SUPPORT
INT Set_MapTS_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif
#ifdef MAP_BL_SUPPORT
INT Set_BlackList_Add(
	PRTMP_ADAPTER pAd,
	char *arg);
INT Set_BlackList_Del(
	PRTMP_ADAPTER pAd,
	char *arg);
INT Set_BlackList_Show(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif /*  MAP_BL_SUPPORT */
#endif /* CONFIG_MAP_SUPPORT */
#ifdef DPP_SUPPORT
INT Set_Enable_Dpp_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif /* DPP_SUPPORT */

INT set_qiscdump_proc(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
INT set_ch_switch_monitor_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT cancel_ch_switch_monitor_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ch_switch_monitor_cfg(IN PRTMP_ADAPTER pAd, struct ch_switch_cfg *ch_sw_cfg);
VOID ch_switch_monitor_state_machine_init(struct _RTMP_ADAPTER *pAd);
VOID ch_switch_monitor_exit(struct _RTMP_ADAPTER *pAd);
VOID ch_switch_monitor_timeout(IN PVOID system_specific1, IN PVOID function_context,
			IN PVOID system_specific2, IN PVOID system_specific3);
extern INT ch_switch_monitor_scan_ch_restore(RTMP_ADAPTER *pAd, UCHAR OpMode, struct wifi_dev *pwdev);
INT ch_switch_monitor_cancel(IN PRTMP_ADAPTER pAd, struct wifi_dev *pwdev);
#endif


INT show_bss_mngr_info(PRTMP_ADAPTER pAd, char *arg);
INT show_bss_tidmap_info(PRTMP_ADAPTER pAd, char *arg);
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_EHT_BE
INT set_trigger_crit_upd_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#endif


#if defined(DOT11_HE_AX) && defined(FIXED_HE_GI_SUPPORT)
INT set_fgi_and_ltf_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_fgi_and_ltf_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_fgi_and_ltf_profile(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 value);
#endif

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
INT set_mgmt_rate_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
INT show_mgmtrate(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
INT set_high_pri_rate_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
INT show_high_pri_rate(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
INT set_suprateset_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
INT set_htsuprateset_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
INT set_vhtsuprateset_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
#ifdef DOT11_HE_AX
INT set_hesuprateset_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
#endif
#ifdef DOT11_EHT_BE
INT set_ehtsuprateset_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
#endif
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

#ifdef OCE_FILS_SUPPORT
VOID RTMPIoctlStaMlmeEvent(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlRsneSyncEvent(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlKeyEvent(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlPmkCacheEvent(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT *wrq);

#endif /* OCE_FILS_SUPPORT */
#ifdef CFG_SUPPORT_CSI
INT Set_CSI_Ctrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#ifdef DPP_SUPPORT
INT wext_send_dpp_cached_frame(struct wifi_dev *wdev, UINT32 frm_id,
				RTMP_IOCTL_INPUT_STRUCT *wrq);
INT mtk_set_pmk(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, struct pmk_req *pmk_data);
#endif /*DPP_SUPPORT*/

struct radio_dev;
#ifdef ZERO_PKT_LOSS_SUPPORT
INT32 HcSuspendMSDUTx(struct _RTMP_ADAPTER *pAd);
INT32 HcUpdateMSDUTxAllow(struct _RTMP_ADAPTER *pAd);
#endif
struct PRIV_SET_PROC {
	RTMP_STRING *name;
	INT (*set_proc)(PRTMP_ADAPTER pAdapter, RTMP_STRING *arg);
};

struct PRIVATE_SHOW_PROC {
	RTMP_STRING *name;
	INT (*set_proc)(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
};

extern struct PRIV_SET_PROC *PRTMP_PRIVATE_AP_SET_PROC;
extern struct PRIVATE_SHOW_PROC *PRTMP_PRIVATE_AP_SHOW_PROC;
extern struct PRIV_SET_PROC RTMP_PRIVATE_AP_SUPPORT_PROC[];
extern struct PRIVATE_SHOW_PROC RTMP_PRIVATE_AP_SHOW_SUPPORT_PROC[];


#ifdef RT_CFG80211_SUPPORT
#ifdef MBO_SUPPORT
INT mtk_cfg80211_Set_mbo_npc_Proc(PRTMP_ADAPTER pAd, UCHAR *npc_value);
VOID Send_WNM_Notify_Req_toAP(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev);
#endif
#endif

#ifdef AIR_MONITOR
#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_monitor_enable(RTMP_ADAPTER * pAd, UCHAR monitor_enable);
INT mtk_cfg80211_set_monitor_rule(RTMP_ADAPTER *pAd, UCHAR *rx_rule);
INT mtk_cfg80211_set_monitor_target(RTMP_ADAPTER *pAd, UCHAR *mac_addr);
INT mtk_cfg80211_set_monitor_index(RTMP_ADAPTER	*pAd, UCHAR mnt_idx);
INT mtk_cfg80211_set_monitor_clear(RTMP_ADAPTER	*pAd);
INT mtk_cfg80211_set_monitor_sta_index(RTMP_ADAPTER *pAd,
	struct mnt_sta *req_sta);
INT mtk_cfg80211_set_monitor_max_pkt_cnt(IN RTMP_ADAPTER *pAd, UINT32 max_pkt);
INT mtk_cfg80211_get_air_mnt_result(
		IN RTMP_ADAPTER *pAd,
		IN struct wiphy *wiphy,
		IN struct wireless_dev *wl_dev,
		IN const void *cmd,
		IN int cmd_len);
#endif /*RT_CFG80211_SUPPORT*/
#endif /* AIR_MONITOR */

#ifdef VLAN_SUPPORT
#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_Set_VLANEn_Proc(RTMP_ADAPTER *pAd, UCHAR vlan_en);
INT mtk_cfg80211_Set_VLANID_Proc(RTMP_ADAPTER *pAd, UINT16 vlan_id);
INT mtk_cfg80211_Set_VLANPriority_Proc(RTMP_ADAPTER *pAd, UINT8 vlan_priority);
INT mtk_cfg80211_Set_VLAN_TAG_Proc(RTMP_ADAPTER *pAd, UCHAR vlan_tag);
INT mtk_cfg80211_Set_VLAN_Policy_Proc(RTMP_ADAPTER *pAd, UINT32 vlan_dir, UINT32 vlan_policy);
#endif /*RT_CFG80211_SUPPORT*/
#endif /*VLAN_SUPPORT*/

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_Set_AP_ASSOC_REQ_RSSI_THRESHOLD(RTMP_ADAPTER *pAd, CHAR rssi);
#endif /*RT_CFG80211_SUPPORT*/
INT set_Monitor_max_pkt_cnt_app(IN RTMP_ADAPTER * pAd, IN RTMP_STRING * arg);
INT set_testmdoe_en_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_tx_null_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef CONFIG_FAST_NAT_SUPPORT
INT set_force_swpath(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_whnat_en_proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
INT set_rro_bypass_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

UINT8 CheckPSDLimitType(RTMP_ADAPTER *pAd);
void TxPowerLimitTypeInit(RTMP_ADAPTER *pAd, UINT8 lpi_en);

VOID Get_Pkt_Cnt_Per_Sta(RTMP_ADAPTER *pAd, UINT16 wcid, ULONG *PerStaTxRetryCount, ULONG *PerStaTxDropCount);

INT set_assoc_record_proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_StationKeepAlive(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_StationKeepAlive(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef TXRX_STAT_SUPPORT
INT Get_Rfdisable_cnts(RTMP_ADAPTER *pAd);
#endif /* TXRX_STAT_SUPPORT*/

#endif /* __AP_CFG_H__ */
