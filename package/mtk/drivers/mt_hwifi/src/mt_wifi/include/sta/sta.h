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
    sta.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/

#ifndef __STA_H__
#define __STA_H__


#define STA_NO_SECURITY_ON(_p)          (IS_CIPHER_NONE(_p->StaCfg[0].wdev.SecConfig.PairwiseCipher))
#define STA_WEP_ON(_p)                  (IS_CIPHER_WEP(_p->StaCfg[0].wdev.SecConfig.PairwiseCipher))
#define STA_TKIP_ON(_p)                 (IS_CIPHER_TKIP(_p->StaCfg[0].wdev.SecConfig.PairwiseCipher))
#define STA_AES_ON(_p)                  (IS_CIPHER_CCMP128(_p->StaCfg[0].wdev.SecConfig.PairwiseCipher))

#define STA_TGN_WIFI_ON(_p)             (_p->StaCfg[0].bTGnWifiTest == TRUE)

#define CKIP_KP_ON(_p)				((((_p)->StaCfg[0].CkipFlag) & 0x10) && ((_p)->StaCfg[0].bCkipCmicOn == TRUE))
#define CKIP_CMIC_ON(_p)			((((_p)->StaCfg[0].CkipFlag) & 0x08) && ((_p)->StaCfg[0].bCkipCmicOn == TRUE))

#define STA_EXTRA_SETTING(_pAd)

#define STA_PORT_SECURED_BY_WDEV(_pAd, __wdev) \
	do { \
		BOOLEAN	Cancelled; \
		PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(_pAd, __wdev); \
		MAC_TABLE_ENTRY *pMEntry = NULL;    \
		struct _STA_TR_ENTRY *_tr_entry; \
		pMEntry = GetAssociatedAPByWdev(_pAd, __wdev);   \
		if (!pMEntry) \
			break; \
		__wdev->PortSecured = WPA_802_1X_PORT_SECURED; \
		NdisAcquireSpinLock(((_pAd)->MacTabLock)); \
		_tr_entry = tr_entry_get(_pAd, pMEntry->wcid); \
		_tr_entry->PortSecured = __wdev->PortSecured; \
		pMEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;\
		NdisReleaseSpinLock((_pAd)->MacTabLock); \
		RTMPCancelTimer(&(pStaCfg->LinkDownTimer), &Cancelled);\
		STA_EXTRA_SETTING(_pAd); \
	} while (0);


BOOLEAN RTMPCheckChannel(RTMP_ADAPTER *pAd, UCHAR CentralCh, UCHAR Ch, struct wifi_dev *wdev);

VOID InitChannelRelatedValue(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

VOID AdjustChannelRelatedValue(
	IN PRTMP_ADAPTER pAd,
	OUT UCHAR *pBwFallBack,
	IN USHORT ifIndex,
	IN UCHAR PriCh,
	IN UCHAR ExtraCh,
	IN struct wifi_dev *wdev);

VOID RTMPReportMicError(
	IN  PRTMP_ADAPTER   pAd,
	IN  PSTA_ADMIN_CONFIG pStaCfg,
	IN  PCIPHER_KEY     pWpaKey);

VOID WpaMicFailureReportFrame(
	IN  PRTMP_ADAPTER    pAd,
	IN  MLME_QUEUE_ELEM * Elem);

VOID WpaDisassocApAndBlockAssoc(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

VOID WpaStaPairwiseKeySetting(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID WpaStaGroupKeySetting(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID WpaSendEapolStart(RTMP_ADAPTER *pAd, UCHAR *pBssid, struct wifi_dev *wdev);


INT sta_tx_pkt_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk);
INT sta_amsdu_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk);
INT sta_legacy_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk);
INT sta_frag_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk);
INT sta_mlme_mgmtq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
INT sta_mlme_dataq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
VOID sta_ieee_802_11_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk);
VOID sta_ieee_802_3_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk);
INT sta_ieee_802_3_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry);
INT sta_ieee_802_11_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry);
INT sta_tx_pkt_allowed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);
VOID sta_find_cipher_algorithm(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk);
BOOLEAN sta_dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry);
INT adhoc_link_up(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT sta_inf_open(struct wifi_dev *wdev);
INT sta_inf_close(struct wifi_dev *wdev);
UINT32 bssinfo_sta_feature_decision(struct wifi_dev *wdev, UINT16 wcid, UINT64 *feature);

INT sta_rx_fwd_hnd(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);

INT STAInitialize(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

VOID rtmp_sta_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

VOID MSTA_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps);
VOID MSTA_Remove(RTMP_ADAPTER *pAd);
VOID MSTAStop(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID sta_deauth_act(struct wifi_dev *wdev);
BOOLEAN sta_media_state_connected(struct wifi_dev *wdev);
VOID sta_handle_mic_error_event(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *entry, struct _RX_BLK *pRxBlk);

/* for STA/APCLI - main thread to wait for mlme completes LinkDown */
VOID sta_os_completion_initialize(STA_ADMIN_CONFIG *pStaCfg);
VOID sta_link_down_complete(STA_ADMIN_CONFIG *pStaCfg);
VOID sta_wait_link_down(STA_ADMIN_CONFIG *pStaCfg);
VOID sta_ifdown_fsm_reset(STA_ADMIN_CONFIG *pStaCfg);
VOID sta_fsm_ops_hook(struct wifi_dev *wdev);


VOID ApCliPeerCsaAction(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BCN_IE_LIST *ie_list);
NTSTATUS ApcliMloCsaSwitchChannelHandler(RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
INT apcli_tx_pkt_allowed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);

BOOLEAN ApCliMsgTypeSubst(PRTMP_ADAPTER  pAd, PFRAME_802_11 pFrame, INT *Machine, INT *MsgType);
BOOLEAN preCheckMsgTypeSubset(PRTMP_ADAPTER  pAd, PFRAME_802_11 pFrame, INT *Machine, INT *MsgType);
BOOLEAN  ApCliHandleRxBroadcastFrame(PRTMP_ADAPTER pAd, struct _RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry);
BOOLEAN apcli_fill_non_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk);
BOOLEAN apcli_fill_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk);
INT ApCliIfLookUp(RTMP_ADAPTER *pAd, UCHAR *pAddr);
VOID ApCliIfUp(PRTMP_ADAPTER pAd);
VOID upSpecificApCliIf(RTMP_ADAPTER *pAd, UCHAR staidx);
VOID ApCliIfDown(PRTMP_ADAPTER pAd);
VOID ApCliIfMonitor(PRTMP_ADAPTER pAd);
VOID ApCliIfMonitor(RTMP_ADAPTER *pAd);
#ifdef APCLI_SUPPORT
INT Set_apcli_ocv_support_proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);

#ifdef APCLI_RANDOM_MAC_SUPPORT
BOOLEAN apcli_set_random_mac_addr(
	struct wifi_dev *wdev,
	UCHAR isJoin);
#endif
#endif
#if defined(DOT11_SAE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
INT set_apcli_sae_group_proc(
    IN PRTMP_ADAPTER pAd,
    IN RTMP_STRING *arg);

INT Set_apcli_sae_pk_proc(
    IN PRTMP_ADAPTER pAd,
    IN RTMP_STRING * arg);
INT Set_apcli_sae_pk_only_proc(
    IN PRTMP_ADAPTER pAd,
    IN RTMP_STRING * arg);
#endif

#ifdef CONFIG_OWE_SUPPORT
INT set_apcli_owe_group_proc(
    IN PRTMP_ADAPTER pAd,
    IN RTMP_STRING *arg);
#endif

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
INT set_apcli_del_pmkid_list(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);
#endif

/* STA/APCLI/RPT Cache Utility functions */
INT sta_add_pmkid_cache(
	IN PRTMP_ADAPTER  pAd,
	IN UCHAR *paddr,
	IN UCHAR *pmkid,
	IN UCHAR *pmk,
	IN UINT8 pmk_len,
	IN UINT8 if_index,
	IN struct wifi_dev *wdev,
	IN UINT32 akm,
	IN UCHAR *ssid,
	IN UCHAR ssid_len
	);

INT sta_search_pmkid_cache(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR *paddr,
	IN UCHAR if_index,
	IN struct wifi_dev *wdev,
	IN UINT32 akm,
	IN UCHAR *ssid,
	IN UCHAR ssid_len);

VOID sta_delete_pmkid_cache(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR *paddr,
	IN UCHAR if_index,
	IN struct wifi_dev *wdev,
	IN UINT32 akm,
	IN UCHAR *ssid,
	IN UCHAR ssid_len);

VOID sta_delete_pmkid_cache_all(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR if_index);

VOID sta_delete_pmkid_cache_by_akm(
	IN  PRTMP_ADAPTER   pAd,
	IN UCHAR if_index,
	IN UINT32 akm);

VOID sta_delete_psk_pmkid_cache_all(
	IN  PRTMP_ADAPTER   pAd,
	IN UCHAR if_index);


#ifdef CONFIG_OWE_SUPPORT
VOID sta_reset_owe_parameters(
	IN PRTMP_ADAPTER   pAd,
	IN UCHAR if_index);

#ifndef RT_CFG80211_SUPPORT
BOOLEAN sta_handle_owe_trans(
	IN PRTMP_ADAPTER   pAd,
	struct wifi_dev *wdev,
	BSS_ENTRY *pInBss);
#endif /* RT_CFG80211_SUPPORT */
#endif


#ifdef APCLI_AUTO_CONNECT_SUPPORT
BOOLEAN ApCliAutoConnectExec(
	IN  PRTMP_ADAPTER   pAd,
	IN struct wifi_dev *wdev);

VOID ApCliSwitchCandidateAP(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev);

#ifdef APCLI_AUTO_BW_TMP /* should be removed after apcli auto-bw is applied */
BOOLEAN ApCliAutoConnectBWAdjust(
	IN RTMP_ADAPTER	*pAd,
	IN struct wifi_dev	*wdev,
	IN BSS_ENTRY *bss_entry);
#endif /* APCLI_AUTO_BW_TMP */
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
BOOLEAN isValidApCliIf(SHORT ifIndex);
VOID ApCliMgtMacHeaderInit(
    IN	PRTMP_ADAPTER	pAd,
    IN OUT PHEADER_802_11 pHdr80211,
    IN UCHAR SubType,
    IN UCHAR ToDs,
    IN PUCHAR pDA,
    IN PUCHAR pBssid,
    IN USHORT ifIndex);


#define STA_OS_WAIT_TIMEOUT RTMPMsecsToJiffies(500)
extern struct wifi_dev_ops sta_wdev_ops;
extern struct wifi_dev_ops apcli_wdev_ops;


#endif /* __STA_H__ */

