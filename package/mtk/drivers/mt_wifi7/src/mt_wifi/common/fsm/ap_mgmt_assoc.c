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
/****************************************************************************
 ****************************************************************************

    Module Name:
    assoc.c

    Abstract:
    Handle association related requests either from WSTA or from local MLME

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "rt_config.h"


extern UCHAR	CISCO_OUI[];
extern UCHAR	WPA_OUI[];
extern UCHAR	RSN_OUI[];
extern UCHAR	WME_INFO_ELEM[];
extern UCHAR	WME_PARM_ELEM[];
extern UCHAR	RALINK_OUI[];
extern UCHAR	BROADCOM_OUI[];
extern UCHAR	WPS_OUI[];

#ifdef IGMP_TVM_SUPPORT
extern UCHAR IGMP_TVM_OUI[];
#endif /* IGMP_TVM_SUPPORT */


struct _assoc_api_ops ap_assoc_api;


#ifdef IAPP_SUPPORT
/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Update Frame to update forwarding table in Layer 2 devices.

 Arguments:
    *mac_p - the STATION MAC address pointer

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN IAPP_L2_Update_Frame_Send(RTMP_ADAPTER *pAd, UINT8 *mac, INT wdev_idx)
{
	NDIS_PACKET	*pNetBuf;
	struct wifi_dev *wdev;

	if (wdev_idx >= WDEV_NUM_MAX || wdev_idx < 0) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_ERROR,
			 "INVALID wdev_idx%d\n", wdev_idx);
		return FALSE;
	}

	wdev = pAd->wdev_list[wdev_idx];
	pNetBuf = RtmpOsPktIappMakeUp(get_netdev_from_bssid(pAd, wdev_idx), mac);

	if (pNetBuf == NULL)
		return FALSE;

	/* UCOS: update the built-in bridge, too (don't use gmac.xmit()) */
	announce_802_3_packet(pAd, pNetBuf, OPMODE_AP);
	IAPP_L2_UpdatePostCtrl(pAd, mac, wdev_idx);
	return TRUE;
} /* End of IAPP_L2_Update_Frame_Send */
#endif /* IAPP_SUPPORT */

static USHORT update_associated_mac_entry(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN IE_LISTS * ie_list,
	IN UCHAR MaxSupportedRate,
	IN BOOLEAN isReassoc)
{
	BSS_STRUCT *mbss;
	struct wifi_dev *wdev;
#ifdef TXBF_SUPPORT
	BOOLEAN	 supportsETxBF = FALSE;
#endif /* TXBF_SUPPORT // */
	STA_TR_ENTRY *tr_entry;
	USHORT PhyMode;
	UCHAR Channel;
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	BOOLEAN need_clr_set_wtbl = FALSE;
	struct common_ies *cmm_ies = &ie_list->cmm_ies;
	UCHAR ht_protect_en = 1;
	BOOLEAN is_cap_ht = FALSE, is_cap_vht = FALSE, is_cap_he = FALSE;
	UCHAR i = 0;
	UCHAR peer_bw_by_opclass = BW_NUM;
	USHORT peer_phymode;

	ASSERT((pEntry->func_tb_idx < pAd->ApCfg.BssidNum));
	mbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
	wdev = &mbss->wdev;
	tr_entry = tr_entry_get(pAd, pEntry->wcid);
	PhyMode = wdev->PhyMode;
	Channel = wdev->channel;
	/* use this to decide reassociation rsp need to clear set wtbl or not */
	if (pEntry->Sst == SST_ASSOC && isReassoc == TRUE)
		need_clr_set_wtbl = TRUE;

	/* Update auth, wep, legacy transmit rate setting . */
	pEntry->Sst = SST_ASSOC;
	/* Use peer mode to get peer working bw by op class IE. */
	peer_phymode = wdev->PhyMode;
#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(peer_phymode) && !(HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists)))
		peer_phymode &= ~WMODE_AC;
#endif
#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(peer_phymode) && !(HAS_HE_CAPS_EXIST(cmm_ies->ie_exists)))
		peer_phymode &= ~(WMODE_AX_24G | WMODE_AX_5G | WMODE_AX_6G);
#endif
#ifdef DOT11_EHT_BE
	if (WMODE_CAP_BE(peer_phymode) && !(HAS_EHT_CAPS_EXIST(cmm_ies->ie_exists)))
		peer_phymode &= ~(WMODE_BE_24G | WMODE_BE_5G | WMODE_BE_6G);
#endif
	if (!pAd->CommonCfg.wifi_cert)
		get_spacing_by_reg_class(pAd, ie_list->current_opclass, peer_phymode, &peer_bw_by_opclass);
	pEntry->MaxSupportedRate = min(wdev->rate.MaxTxRate, MaxSupportedRate);
	MacTableSetEntryPhyCfg(pAd, pEntry);
	pEntry->CapabilityInfo = ie_list->CapabilityInfo;

	if (IS_AKM_PSK_Entry(pEntry)) {
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
		pEntry->SecConfig.Handshake.WpaState = AS_INITPSK;
	}

#ifdef DOT1X_SUPPORT
	else if (IS_AKM_1X_Entry(pEntry) || IS_IEEE8021X_Entry(wdev)) {
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
		pEntry->SecConfig.Handshake.WpaState = AS_AUTHENTICATION;
	}

#endif /* DOT1X_SUPPORT */
	/* Ralink proprietary Piggyback and Aggregation support for legacy RT61 chip */
	MacTableSetEntryRaCap(pAd, pEntry, &ie_list->cmm_ies.vendor_ie);
#ifdef DOT11_VHT_AC
	if (HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists))
		MacTableEntryCheck2GVHT(pAd, pEntry);
#endif /* DOT11_VHT_AC */


	/* delay the  port serured to ap_cmm_peer_assoc_req_action() */
	tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

#if defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT)
	/* There are some situation to need to encryption by software
	   1. The Client support PMF. It shall ony support AES cipher.
	   2. The Client support WAPI.
	   If use RT3883 or later, HW can handle the above.
	   */
#ifdef DOT11W_PMF_SUPPORT

	if ((cap->FlgPMFEncrtptMode == PMF_ENCRYPT_MODE_0)
		&& (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE))
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT);

#endif /* DOT11W_PMF_SUPPORT */

#ifdef SW_CONNECT_SUPPORT
		if ((IS_CIPHER_CCMP128(pEntry->SecConfig.PairwiseCipher) ||
			IS_CIPHER_WEP40(pEntry->SecConfig.PairwiseCipher) ||
			IS_CIPHER_WEP104(pEntry->SecConfig.PairwiseCipher)) &&
			IS_SW_STA(tr_entry)) {
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT);
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
					"wcid(%u), hw_wcid(%u), SW Encrypt!\n",
					pEntry->wcid, tr_entry->HwWcid);
		}
#endif /* SW_CONNECT_SUPPORT */

#endif /* defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT) */

	if (wdev->bWmmCapable && ie_list->bWmmCapable)
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
	else
		CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

#ifdef DOT11_N_SUPPORT

	/*
		WFA recommend to restrict the encryption type in 11n-HT mode.
		So, the WEP and TKIP are not allowed in HT rate.
	*/
	if (pAd->CommonCfg.HT_DisallowTKIP &&
		IS_INVALID_HT_SECURITY(pEntry->SecConfig.PairwiseCipher)) {
		/* Force to None-HT mode due to WiFi 11n policy */
		CLR_HT_CAPS_EXIST(cmm_ies->ie_exists);
#ifdef DOT11_VHT_AC
		CLR_VHT_CAPS_EXIST(cmm_ies->ie_exists);
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
		CLR_HE_CAPS_EXIST(cmm_ies->ie_exists);
#endif /* DOT11_HE_AX */
#ifdef DOT11_EHT_BE
		CLR_EHT_CAPS_EXIST(cmm_ies->ie_exists);
#endif /* DOT11_EHT_BE */
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				 "Force the STA as Non-HT mode\n");
	}

	/* If this Entry supports 802.11n, upgrade to HT rate. */
	if (HAS_HT_CAPS_EXIST(cmm_ies->ie_exists) &&
		(wdev->DesiredHtPhyInfo.bHtEnable) &&
		WMODE_CAP_N(PhyMode)) {
		is_cap_ht = TRUE;
		pEntry->ht_ie_flag = TRUE;
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			"HT IE present in Assoc Req\n");
		ht_mode_adjust(pAd, pEntry, &cmm_ies->ht_cap);

#ifdef DOT11N_DRAFT3

		if (ie_list->ExtCapInfo.BssCoexistMgmtSupport)
			pEntry->BSS2040CoexistenceMgmtSupport = 1;

#endif /* DOT11N_DRAFT3 */
#if defined(CONFIG_HOTSPOT_R2) || defined(QOS_R1)
		if (ie_list->ExtCapInfo.qosmap)
			pEntry->QosMapSupport = 1;
#endif
#ifdef QOS_R1
if (ie_list->ExtCapInfo.dot11MSCSActivated)
	pEntry->MSCSSupport = 1;
#endif
#ifdef CONFIG_DOT11V_WNM
			if (ie_list->ExtCapInfo.BssTransitionManmt)
				pEntry->BssTransitionManmtSupport = 1;
#endif /* CONFIG_DOT11V_WNM */

		/* 40Mhz BSS Width Trigger events */
		if (
#ifdef BW_VENDOR10_CUSTOM_FEATURE
			/* Soft AP to follow BW of Root AP */
			(IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd) == FALSE) &&
#endif
			ie_list->cmm_ies.ht_cap.HtCapInfo.Forty_Mhz_Intolerant &&
			(wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G)) {

#ifdef DOT11N_DRAFT3
			UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
			UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
			UCHAR op_ext_cha = wlan_operate_get_ext_cha(wdev);

			pEntry->bForty_Mhz_Intolerant = TRUE;
			pAd->MacTab->fAnyStaFortyIntolerant = TRUE;

			/*set bForty_Mhz_Intolerant to TRUE to
			ignore the following BSS_2040_COEXIST Management frame
			to intercept action frame attacks*/
			pAd->CommonCfg.bForty_Mhz_Intolerant = TRUE;

			if (((cfg_ht_bw == HT_BW_40) &&
				WMODE_CAP_2G(wdev->PhyMode)) &&
				((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
				 (op_ht_bw != HT_BW_20) &&
				 (op_ext_cha != 0))
			   ) {
				pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 1;
				wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
				pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
					"Update Beacon for mbss_idx:%d\n", pEntry->func_tb_idx);
				UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));
			}

			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				"pEntry set 40MHz Intolerant as 1\n");
#endif /* DOT11N_DRAFT3 */
			Handle_BSS_Width_Trigger_Events(pAd, Channel);
		}

		/* find max fixed rate */
		pEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(&wdev->DesiredHtPhyInfo.MCSSet[0],
										 &cmm_ies->ht_cap.MCSSet[0]);

		if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO) {
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					 "@@@ IF-ra%d DesiredTransmitSetting.field.MCS = %d\n",
					  pEntry->func_tb_idx,
					  wdev->DesiredTransmitSetting.field.MCS);
			set_ht_fixed_mcs(pEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);
		}

		set_sta_ht_cap(pAd, pEntry, &cmm_ies->ht_cap);
		/* Record the received capability from association request */
		NdisMoveMemory(&pEntry->HTCapability, &cmm_ies->ht_cap, sizeof(HT_CAPABILITY_IE));
#ifdef DOT11_VHT_AC
		if (RADIO_IN_2G(wdev) &&
			HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists)) {
			pEntry->vht_ie_flag = TRUE;
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				"VHT IE present in Assoc Req for 2G case\n");
			vht_mode_adjust_sta_max_capab(pAd, pEntry, &cmm_ies->vht_cap,
				HAS_VHT_OP_EXIST(cmm_ies->ie_exists) ? &cmm_ies->vht_op : NULL,
				(cmm_ies->operating_mode_len == 0) ? NULL :  &cmm_ies->operating_mode,
				(peer_bw_by_opclass != BW_NUM) ? &peer_bw_by_opclass:NULL);
		}
		if (WMODE_CAP_AC(PhyMode) &&
			RADIO_IN_ABAND(wdev) &&
			HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists)) {
			is_cap_vht = TRUE;
			pEntry->vht_ie_flag = TRUE;
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				"VHT IE present in Assoc Req\n");
			vht_mode_adjust(pAd, pEntry, &cmm_ies->vht_cap,
					HAS_VHT_OP_EXIST(cmm_ies->ie_exists) ? &cmm_ies->vht_op : NULL,
					(cmm_ies->operating_mode_len == 0) ? NULL :  &cmm_ies->operating_mode,
					(peer_bw_by_opclass != BW_NUM) ? &peer_bw_by_opclass:NULL);
			dot11_vht_mcs_to_internal_mcs(pAd, wdev, &cmm_ies->vht_cap, &pEntry->MaxHTPhyMode);

			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					 "Peer's PhyCap=>Mode:%s, BW:%s, NSS:%d, MCS:%d\n",
					  get_phymode_str(pEntry->MaxHTPhyMode.field.MODE),
					  get_bw_str(pEntry->MaxHTPhyMode.field.BW, BW_FROM_OID),
					  ((pEntry->MaxHTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1,
					  (pEntry->MaxHTPhyMode.field.MCS & 0xf));

			set_vht_cap(pAd, pEntry, &cmm_ies->vht_cap);
			NdisMoveMemory(&pEntry->vht_cap_ie, &cmm_ies->vht_cap, sizeof(VHT_CAP_IE));
		}

		if (ie_list->cmm_ies.operating_mode_len == sizeof(OPERATING_MODE) &&
			ie_list->cmm_ies.operating_mode.rx_nss_type == 0) {
			pEntry->operating_mode = ie_list->cmm_ies.operating_mode;
			pEntry->force_op_mode = TRUE;
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					 "Peer's OperatingMode=>RxNssType: %d, RxNss: %d, ChBW: %d, bw160/8080: %d\n",
					  pEntry->operating_mode.rx_nss_type,
					  pEntry->operating_mode.rx_nss,
					  pEntry->operating_mode.ch_width,
					  pEntry->operating_mode.bw160_bw8080);
		} else
			pEntry->force_op_mode = FALSE;

		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			"%s(): Peer's bw: %d, extChanOffset: %d, RecomWidth: %d\n",
			__func__, pEntry->MaxHTPhyMode.field.BW,
			cmm_ies->ht_op.AddHtInfo.ExtChanOffset,
			cmm_ies->ht_op.AddHtInfo.RecomWidth);
		if ((pEntry->MaxHTPhyMode.field.BW == BW_40) &&
			WMODE_CAP_2G(wdev->PhyMode) &&
			HAS_HT_OP_EXIST(cmm_ies->ie_exists) &&
			(cmm_ies->ht_op.AddHtInfo.ExtChanOffset == EXTCHA_NONE) &&
			(cmm_ies->ht_op.AddHtInfo.RecomWidth == 0)) {
			pEntry->operating_mode.ch_width = 0;
			pEntry->operating_mode.rx_nss_type = 0;
			pEntry->operating_mode.rx_nss = (wlan_operate_get_rx_stream(wdev) - 1);
			pEntry->force_op_mode = TRUE;
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				"Special Peer's OperatingMode=>RxNssType: %d, RxNss: %d, ChBW: %d\n",
				pEntry->operating_mode.rx_nss_type,
				pEntry->operating_mode.rx_nss,
				pEntry->operating_mode.ch_width);
		}

#endif /* DOT11_VHT_AC */
	}

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(PhyMode) && HAS_HE_CAPS_EXIST(cmm_ies->ie_exists)) {
		is_cap_he = TRUE;
		pEntry->he_ie_flag = TRUE;
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			"HE IE present in Assoc Req\n");
		update_peer_he_caps(pEntry, cmm_ies);
		update_peer_he_operation(pEntry, cmm_ies);
		he_mode_adjust(wdev, pEntry, (peer_bw_by_opclass != BW_NUM) ? &peer_bw_by_opclass:NULL,
			(is_cap_ht || is_cap_vht));

		for (i = 0; i < DOT11AX_MAX_STREAM; i++) {
			if(pEntry->cap.rate.he80_rx_nss_mcs[i] == 3)
				break ;
		}
		if(i != 0) {
			i = i <= wlan_operate_get_tx_stream(wdev) ? i : wlan_operate_get_tx_stream(wdev);
			pEntry->MaxHTPhyMode.field.MCS = ((i-1) << 4);
		}
		else {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
			"STA antenna information provided is incorrect");
		}

		if(pEntry->cap.rate.he80_rx_nss_mcs[0] == 2)
			pEntry->MaxHTPhyMode.field.MCS += HE_MCS_11 ;
		else if(pEntry->cap.rate.he80_rx_nss_mcs[0] == 1)
			pEntry->MaxHTPhyMode.field.MCS += HE_MCS_9 ;
		else if(pEntry->cap.rate.he80_rx_nss_mcs[0] == 0)
			pEntry->MaxHTPhyMode.field.MCS += HE_MCS_7;
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
			"STA MCS information provided is incorrect");
	}
#endif /*DOT11_HE_AX*/
#ifdef TXBF_SUPPORT
#ifdef DOT11_HE_AX
		if (WMODE_CAP_AX(PhyMode) && HAS_HE_CAPS_EXIST(cmm_ies->ie_exists)) {
			supportsETxBF = (pEntry->cap.he_bf.bf_cap & HE_SU_BFER) || (pEntry->cap.he_bf.bf_cap & HE_SU_BFEE);
		} else
#endif
#ifdef DOT11_VHT_AC
			if (WMODE_CAP_AC(PhyMode) && (WMODE_CAP_5G(wdev->PhyMode)) &&
				HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists))
				supportsETxBF = mt_WrapClientSupportsVhtETxBF(pAd, &ie_list->cmm_ies.vht_cap.vht_cap);
			else
#endif /* DOT11_VHT_AC */
				supportsETxBF = mt_WrapClientSupportsETxBF(pAd, &cmm_ies->ht_cap.TxBFCap);
#endif /* TXBF_SUPPORT */

#ifdef DOT11_EHT_BE
	if (WMODE_CAP_BE(PhyMode) && HAS_EHT_CAPS_EXIST(cmm_ies->ie_exists)) {
		eht_update_peer_caps(pEntry, cmm_ies);
		eht_mode_adjust(wdev, pEntry, NULL);
	}
#endif /* DOT11_EHT_BE */


	/* avoid Legacy station IOT issue.
	   some STA only 8K rx cap, but annouce 11k cap.  ex: ac7260
	   adjust AMSDU size to 8K. */
	if (pEntry->MaxHTPhyMode.field.MODE == MODE_VHT &&
		pEntry->MaxHTPhyMode.field.BW <= BW_80 &&
	    pEntry->AMsduSize > MPDU_7991_OCTETS) {
		pEntry->AMsduSize = MPDU_7991_OCTETS;
	}


#ifdef QOS_R1
		if (ie_list->ExtCapInfo.qosmap)
			pEntry->QosMapSupport = 1;
#endif

	if (!is_cap_ht && !is_cap_vht && !is_cap_he) {
#ifdef CONFIG_HOTSPOT_R2
		if (ie_list->ExtCapInfo.qosmap)
			pEntry->QosMapSupport = 1;
#endif
		pAd->MacTab->fAnyStationIsLegacy = TRUE;
		NdisZeroMemory(&pEntry->HTCapability, sizeof(HT_CAPABILITY_IE));
		pEntry->SupportHTMCS = 0;
		pEntry->SupportRateMode &= (~SUPPORT_HT_MODE);
#ifdef DOT11_VHT_AC
		/* TODO: shiang-usw, it's ugly and need to revise it */
		NdisZeroMemory(&pEntry->vht_cap_ie, sizeof(VHT_CAP_IE));
		pEntry->SupportVHTMCS1SS = 0;
		pEntry->SupportVHTMCS2SS = 0;
		pEntry->SupportVHTMCS3SS = 0;
		pEntry->SupportVHTMCS4SS = 0;
		pEntry->SupportRateMode &= (~SUPPORT_VHT_MODE);
#endif /* DOT11_VHT_AC */
	}

#endif /* DOT11_N_SUPPORT */
	pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;
		pEntry->CurrTxRate = pEntry->MaxSupportedRate;

#ifdef MFB_SUPPORT
	pEntry->lastLegalMfb = 0;
	pEntry->isMfbChanged = FALSE;
	pEntry->fLastChangeAccordingMfb = FALSE;
	pEntry->toTxMrq = TRUE;
	pEntry->msiToTx = 0;/*has to increment whenever a mrq is sent */
	pEntry->mrqCnt = 0;
	pEntry->pendingMfsi = 0;
	pEntry->toTxMfb = FALSE;
	pEntry->mfbToTx = 0;
	pEntry->mfb0 = 0;
	pEntry->mfb1 = 0;
#endif	/* MFB_SUPPORT */
	pEntry->freqOffsetValid = FALSE;
#ifdef TXBF_SUPPORT

	if (cap->FlgHwTxBfCap)
		chip_tx_bf_init(pAd, pEntry, ie_list, supportsETxBF);

#endif /* TXBF_SUPPORT // */
#ifdef MT_MAC

	if (cap->hif_type == HIF_MT) {
		if (wdev->bAutoTxRateSwitch == TRUE)
			pEntry->bAutoTxRateSwitch = TRUE;
		else {
			pEntry->HTPhyMode.field.MCS = wdev->HTPhyMode.field.MCS;
			pEntry->bAutoTxRateSwitch = FALSE;

			if (pEntry->HTPhyMode.field.MODE >= MODE_VHT) {
				pEntry->HTPhyMode.field.MCS = wdev->DesiredTransmitSetting.field.MCS +
											  ((wlan_operate_get_tx_stream(wdev) - 1) << 4);
			}

			/* If the legacy mode is set, overwrite the transmit setting of this entry. */
			RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, pEntry);
		}
	}

#endif /* MT_MAC */

	if (IS_NO_SECURITY(&pEntry->SecConfig) || IS_CIPHER_WEP(pEntry->SecConfig.PairwiseCipher))
		ApLogEvent(pAd, pEntry->Addr, EVENT_ASSOCIATED);


	if ((pEntry->MaxHTPhyMode.field.MODE == MODE_OFDM) ||
		(pEntry->MaxHTPhyMode.field.MODE == MODE_CCK))
		pAd->MacTab->fAnyStationIsLegacy = TRUE;

	NdisAcquireSpinLock(pAd->MacTabLock);
	nonerp_sta_num(pEntry, PEER_JOIN);
	NdisReleaseSpinLock(pAd->MacTabLock);

	ApUpdateCapabilityAndErpIe(pAd, mbss);
#ifdef DOT11_N_SUPPORT
	ht_protect_en = wlan_config_get_ht_protect_en(wdev);
	if (ht_protect_en)
		APUpdateOperationMode(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
	return MLME_SUCCESS;
}


/*
    ==========================================================================
    Description:
       assign a new AID to the newly associated/re-associated STA and
       decide its MaxSupportedRate and CurrTxRate. Both rates should not
       exceed AP's capapbility
    Return:
       MLME_SUCCESS - association successfully built
       others - association failed due to resource issue
    ==========================================================================
 */
#if defined(HOSTAPD_WPA3_SUPPORT) || defined(HOSTAPD_11R_SUPPORT)
USHORT APBuildAssociation
#else
static USHORT APBuildAssociation
#endif
(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN IE_LISTS * ie_list,
	IN UCHAR MaxSupportedRateIn500Kbps,
	OUT USHORT *pAid,
	IN BOOLEAN isReassoc)
{
	USHORT StatusCode = MLME_SUCCESS;
	UCHAR MaxSupportedRate;
	struct wifi_dev *wdev;
#ifdef WSC_AP_SUPPORT
	WSC_CTRL *wsc_ctrl;
#endif /* WSC_AP_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg = NULL;
#endif /* DOT11R_FT_SUPPORT */
	STA_TR_ENTRY *tr_entry;
#ifndef RT_CFG80211_SUPPORT
#ifdef CONFIG_OWE_SUPPORT
	PUINT8 pPmkid = NULL;
	UINT8 pmkid_count = 0;
#endif /*CONFIG_OWE_SUPPORT*/
#endif /*RT_CFG80211_SUPPORT*/

#ifdef RATE_PRIOR_SUPPORT
	PBLACK_STA pBlackSta = NULL;
#endif/*RATE_PRIOR_SUPPORT*/

	if (!pEntry)
		return MLME_UNSPECIFY_FAIL;

	wdev = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev;
	MaxSupportedRate = dot11_2_ra_rate(MaxSupportedRateIn500Kbps);

	if ((WMODE_EQUAL(wdev->PhyMode, WMODE_G)
#ifdef DOT11_N_SUPPORT
		 || WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
#endif /* DOT11_N_SUPPORT */
		)
		&& (MaxSupportedRate < RATE_FIRST_OFDM_RATE)
	   )
		return MLME_ASSOC_REJ_DATA_RATE;

#ifdef GN_MIXMODE_SUPPORT
	if (pAd->CommonCfg.GNMixMode
		&& (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
			|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)
			|| WMODE_EQUAL(wdev->PhyMode, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G)))
		&& pEntry->FlgIs11bSta)
		return MLME_ASSOC_REJ_DATA_RATE;
#endif /* GN_MIXMODE_SUPPORT */

#ifdef DOT11_N_SUPPORT

	/* 11n only */
	if (WMODE_HT_ONLY(wdev->PhyMode) && !HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists))
		return MLME_ASSOC_REJ_DATA_RATE;

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode) &&
		WMODE_CAP_5G(wdev->PhyMode) &&
		!HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists) &&
		(pAd->CommonCfg.bNonVhtDisallow))
		return MLME_ASSOC_REJ_DATA_RATE;

#endif /* DOT11_VHT_AC */


	if ((wdev->IsNoAGMode) && !WMODE_CAP_6G(wdev->PhyMode) &&
		!HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
			"Not Support only AG Mode MLME_ASSOC_REJ_DATA_RATE\n");

		return MLME_ASSOC_REJ_DATA_RATE;
	}

	if ((wdev->IsNoNMode) && !WMODE_CAP_6G(wdev->PhyMode) &&
		!HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists)) {

		if (pAd->CommonCfg.bNonVhtDisallow) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"1. Not Support only N Mode MLME_ASSOC_REJ_DATA_RATE\n");

			return MLME_ASSOC_REJ_DATA_RATE;
		}
		if (!(pAd->CommonCfg.bNonVhtDisallow) &&
			!HAS_HE_CAPS_EXIST(ie_list->cmm_ies.ie_exists)) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"2. Not Support only N Mode MLME_ASSOC_REJ_DATA_RATE\n");

			return MLME_ASSOC_REJ_DATA_RATE;
		}
	}
#ifdef DOT11_HE_AX
	/* 11ax only */
	if ((wdev->IsAXOnly) &&
		!HAS_HE_CAPS_EXIST(ie_list->cmm_ies.ie_exists)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
			"11AX only MLME_ASSOC_REJ_DATA_RATE\n");

		return MLME_ASSOC_REJ_DATA_RATE;
	}
#endif /*DOT11_HE_AX*/

#ifdef DOT11_EHT_BE
	/* 11be only */
	if ((wdev->IsBEOnly) &&
		!HAS_EHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
			"11BE only MLME_ASSOC_REJ_DATA_RATE\n");

		return MLME_ASSOC_REJ_DATA_RATE;
	}
#endif /* DOT11_EHT_BE */

#ifdef RATE_PRIOR_SUPPORT
			if (pAd->LowRateCtrl.RatePrior) {
				if (WMODE_CAP_2G(wdev->PhyMode) && !HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists))
					return MLME_UNSPECIFY_FAIL;
#ifdef DOT11_VHT_AC
				if (WMODE_CAP_5G(wdev->PhyMode) && !HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists))
					return MLME_UNSPECIFY_FAIL;
#endif /* DOT11_VHT_AC */
				OS_SEM_LOCK(&pAd->LowRateCtrl.BlackListLock);
				DlListForEach(pBlackSta, &pAd->LowRateCtrl.BlackList, BLACK_STA, List) {
					if (NdisCmpMemory(pBlackSta->Addr, pEntry->Addr, MAC_ADDR_LEN) == 0) {
						MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
						"Reject blk sta: "MACSTR"\n", MAC2STR(pBlackSta->Addr));
						OS_SEM_UNLOCK(&pAd->LowRateCtrl.BlackListLock);
						return MLME_UNSPECIFY_FAIL;
					}
				}
				OS_SEM_UNLOCK(&pAd->LowRateCtrl.BlackListLock);
			}
#endif /*RATE_PRIOR_SUPPORT*/

#ifdef DOT11R_FT_SUPPORT
	pFtCfg = &(wdev->FtCfg);
	/* Set Sst = SST_ASSOC for ft_over_ds case, as FT_AUth is skip as per spec */
	if (StatusCode == MLME_SUCCESS && isReassoc == TRUE &&
			pFtCfg->FtCapFlag.FtOverDs && pEntry->Sst != SST_ASSOC)
		pEntry->Sst = SST_ASSOC;
#endif /* DOT11R_FT_SUPPORT */

	if ((pEntry->Sst == SST_AUTH) || (pEntry->Sst == SST_ASSOC)) {
		/* TODO:
			should qualify other parameters, for example -
			capablity, supported rates, listen interval, etc., to
			decide the Status Code
		*/
		tr_entry = tr_entry_get(pAd, pEntry->wcid);
		*pAid = pEntry->Aid;
		pEntry->NoDataIdleCount = 0;
		tr_entry->NoDataIdleCount = 0;
		pEntry->StaConnectTime = 0;
		pEntry->sleep_from = 0;
		sec_update_entry_rsn_ie(pEntry, ie_list);
#ifdef CONFIG_HOTSPOT_R2

		if (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE))
#endif
		{
			if (pEntry->bWscCapable == FALSE)
			{
				/* check the validity of the received RSNIE */
				StatusCode = WPAValidateRSNIE(wdev,
							      &pEntry->SecConfig,
							      &pEntry->RSN_IE[0],
							      pEntry->RSNIE_Len);
#ifdef DOT11R_FT_SUPPORT
				if (!IS_FT_STA(pEntry)) /* IS_FT_RSN_STA should be use at 4-way only due to rnsie is assigned at assoc state */
#endif
				{
#ifdef DOT11_SAE_SUPPORT
#ifndef HOSTAPD_WPA3_SUPPORT
					struct __SAE_CFG *sae_cfg =
						PD_GET_SAE_CFG_PTR(pAd->physical_dev);
					INT cacheidx;
					UCHAR *peer_mac = pEntry->Addr;
					UCHAR is_mlo_connect = FALSE;
					UCHAR *own_mac = pEntry->wdev->bssid;

#ifdef DOT11_EHT_BE
					if (pEntry->mlo.mlo_en) {
						is_mlo_connect = TRUE;
						own_mac = pEntry->wdev->bss_info_argument.mld_info.mld_addr;
						peer_mac = pEntry->mlo.mld_addr;
						if (MAC_ADDR_EQUAL(peer_mac, ZERO_MAC_ADDR)) {
							MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
								"peer_mac invalid\n");
							StatusCode = MLME_UNSPECIFY_FAIL;
						}
					}
#endif

					if ((StatusCode == MLME_SUCCESS)
						&& IS_AKM_WPA3PSK(pEntry->SecConfig.AKMMap)
						&& (is_rsne_pmkid_cache_match(ie_list->RSN_IE,
							   ie_list->RSNIE_Len,
							   PD_GET_PMKID_PTR(pAd->physical_dev),
							   own_mac,
							   peer_mac,
							   is_mlo_connect,
							   &cacheidx))
						&& (cacheidx == INVALID_PMKID_IDX))
						StatusCode = MLME_INVALID_PMKID;
					else if ((StatusCode == MLME_SUCCESS)
						&& IS_AKM_WPA3PSK(pEntry->SecConfig.AKMMap)
						&& !sae_get_pmk_cache(sae_cfg, own_mac,
							peer_mac, is_mlo_connect, NULL, NULL))
						StatusCode = MLME_UNSPECIFY_FAIL;
#endif

#endif /* DOT11_SAE_SUPPORT */

#ifndef RT_CFG80211_SUPPORT
#ifdef CONFIG_OWE_SUPPORT
					if ((StatusCode == MLME_SUCCESS)
						&& IS_AKM_OWE(pEntry->SecConfig.AKMMap))
						StatusCode = owe_pmkid_ecdh_process(pAd,
										    pEntry,
										    ie_list->RSN_IE,
										    ie_list->RSNIE_Len,
										    &ie_list->ecdh_ie,
										    ie_list->ecdh_ie.length,
										    pPmkid,
										    &pmkid_count,
										    SUBTYPE_ASSOC_REQ);
#endif /*CONFIG_OWE_SUPPORT*/
#endif /*RT_CFG80211_SUPPORT*/
				}

#ifdef MTK_MLO_MAP_SUPPORT
				if (IS_MAP_ENABLE(pAd) && (StatusCode == MLME_SUCCESS)) {
#ifdef DOT11R_FT_SUPPORT
					/* IS_FT_RSN_STA should be use at
					 * 4-way only due to rnsie is assigned
					 * at assoc state
					 */
					if (!IS_FT_STA(pEntry))
#endif
						StatusCode = mtk_map_parse_assoc_rsnie_pmkid_match(pAd,
							pEntry, ie_list);
				}
#endif /* MTK_MLO_MAP_SUPPORT */
#ifdef MAP_R3
				if ((IS_MAP_ENABLE(pAd) && !IS_MAP_CERT_ENABLE(pAd))
					|| !IS_MAP_ENABLE(pAd)) {
#endif
					if (StatusCode == MLME_SUCCESS)
						StatusCode = parse_rsnxe_ie(&pEntry->SecConfig,
							&ie_list->rsnxe_ie[0], ie_list->rsnxe_ie_len, TRUE);
#ifdef MAP_R3
				}
#endif
#ifdef MLR_SUPPORT
				pEntry->MlrMode = mlr_determine_version(pAd,
					&(ie_list->cmm_ies.vendor_ie), &(pEntry->MlrCurState));
#endif
				if (StatusCode != MLME_SUCCESS) {
					/* send wireless event - for RSN IE sanity check fail */
					RTMPSendWirelessEvent(pAd, IW_RSNIE_SANITY_FAIL_EVENT_FLAG, pEntry->Addr, 0, 0);
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
						"invalid status code(%d) !!!\n", StatusCode);
					return StatusCode;
				}

				if (pEntry->SecConfig.AKMMap == 0x0) {
					SET_AKM_OPEN(pEntry->SecConfig.AKMMap);
					SET_CIPHER_NONE(pEntry->SecConfig.PairwiseCipher);
					SET_CIPHER_NONE(pEntry->SecConfig.GroupCipher);
				}

				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					"(AID#%d AKM=0x%x, PairwiseCipher=0x%x)\n",
					pEntry->Aid, pEntry->SecConfig.AKMMap,
					pEntry->SecConfig.PairwiseCipher);
			}
		}


		if (*pAid == 0 || *pAid == INVALID_AID)
			/* Here should be MAC table full */
			StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;
		else if ((pEntry->RSNIE_Len == 0) &&
				 (IS_AKM_WPA_CAPABILITY_Entry(pEntry))
#ifdef RT_CFG80211_SUPPORT
				 && (wdev->Hostapd == Hostapd_CFG)
#endif /*RT_CFG80211_SUPPORT*/
				) {
#ifdef WSC_AP_SUPPORT
			wsc_ctrl = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl;

			if (((wsc_ctrl->WscConfMode != WSC_DISABLE) &&
				 pEntry->bWscCapable
#ifdef WSC_V2_SUPPORT
				 && (wsc_ctrl->WscV2Info.bWpsEnable ||
					 (wsc_ctrl->WscV2Info.bEnableWpsV2 == FALSE))
#endif /* WSC_V2_SUPPORT */
				)
#ifdef RT_CFG80211_SUPPORT
				|| wdev->Hostapd == Hostapd_CFG
#endif /*RT_CFG80211_SUPPORT*/
			   ) {
				pEntry->Sst = SST_ASSOC;
				StatusCode = MLME_SUCCESS;

				/* In WPA or 802.1x mode, the port is not secured. */
				if (IS_AKM_WPA_CAPABILITY(pEntry->SecConfig.AKMMap)
#ifdef DOT1X_SUPPORT
					|| IS_IEEE8021X_Entry(wdev)
#endif /* DOT1X_SUPPORT */
				   )
					tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
				else
					tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

				if (IS_AKM_PSK(pEntry->SecConfig.AKMMap)) {
					pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
					pEntry->SecConfig.Handshake.WpaState = AS_INITPSK;
				}

#ifdef DOT1X_SUPPORT
				else if (IS_AKM_1X(pEntry->SecConfig.AKMMap) || IS_IEEE8021X_Entry(wdev)) {
					pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
					pEntry->SecConfig.Handshake.WpaState = AS_AUTHENTICATION;
				}

#endif /* DOT1X_SUPPORT */
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					"ASSOC - WPS SM is OFF.<WscConfMode = %d, apidx =%d>\n",
					wsc_ctrl->WscConfMode, pEntry->func_tb_idx);
				StatusCode = MLME_ASSOC_DENY_OUT_SCOPE;
			}

#else  /* WSC_AP_SUPPORT */
			StatusCode = MLME_ASSOC_DENY_OUT_SCOPE;
#endif /* WSC_AP_SUPPORT */
		} else
		StatusCode = update_associated_mac_entry(pAd, pEntry, ie_list, MaxSupportedRate, isReassoc);
	}

	if (StatusCode == MLME_SUCCESS) {
#ifdef LINK_TEST_SUPPORT
		/* STA Link up Handler */
		LinkTestStaLinkUpHandler(pAd, pEntry);
#endif /* LINK_TEST_SUPPORT */
	}

	return StatusCode;
}

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
static BOOLEAN PeerDisassocReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID *Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pDA,
	OUT PUCHAR pAddr2,
	OUT	UINT16 *SeqNum,
	OUT USHORT *Reason)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)Msg;

	COPY_MAC_ADDR(pDA, &Fr->Hdr.Addr1);
	COPY_MAC_ADDR(pAddr2, &Fr->Hdr.Addr2);
	*SeqNum = Fr->Hdr.Sequence;
	NdisMoveMemory(Reason, &Fr->Octet[0], 2);
	return TRUE;
}

BOOLEAN PeerAssocReqCmmSanity
(
	RTMP_ADAPTER *pAd,
	BOOLEAN isReassoc,
	VOID *Msg,
	INT MsgLen,
	IE_LISTS *ie_lists)
{
	CHAR *Ptr;
	PFRAME_802_11	Fr = (PFRAME_802_11)Msg;
	PEID_STRUCT eid_ptr;
	UCHAR Sanity = 0;
	UCHAR WPA1_OUI[4] = { 0x00, 0x50, 0xF2, 0x01 };
#ifdef CONFIG_HOTSPOT_R2
	UCHAR HS2_OSEN_OUI[4] = { 0x50, 0x6f, 0x9a, 0x12 };
	UCHAR HS2OUIBYTE[4] = {0x50, 0x6f, 0x9a, 0x10};
#endif
#ifdef CONFIG_HOTSPOT_R3
	UCHAR HS2_ROAMING_CONSORTIUM_SELECTION_OUI[4] = {0x50, 0x6f, 0x9a, 0x1D};
#endif /* CONFIG_HOTSPOT_R3*/
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
#endif /* DOT11R_FT_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
	unsigned char map_cap = 0;
#ifdef MAP_R2
	UCHAR map_profile;
	UINT16 map_vid;
#endif
#endif
	MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)NULL;
	struct frag_ie_info frag_info = {0};
#ifdef DOT11R_FT_SUPPORT
	FT_INFO *pFtInfo = &ie_lists->FtInfo;
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
	RRM_EN_CAP_IE *pRrmEnCap = &ie_lists->RrmEnCap;
#endif /* DOT11K_RRM_SUPPORT */
	HT_CAPABILITY_IE *pHtCapability = &ie_lists->cmm_ies.ht_cap;
	UCHAR i;
	struct legacy_rate *rate = &ie_lists->rate;
#ifdef CFG_BIG_ENDIAN
	UINT32 tmp_1;
	UINT64 tmp_2;
	UINT16 tmp;
	UCHAR *pextCapInfo = NULL;
#endif
	INT remain_ie_len = 0;

	pEntry = MacTableLookup(pAd, &Fr->Hdr.Addr2[0]);

#ifdef DOT11R_FT_SUPPORT
	pFtCfg = &(pAd->ApCfg.MBSSID[apidx].wdev.FtCfg);

	if (pFtCfg && pFtCfg->FtCapFlag.FtOverDs && !pEntry) {
		pEntry = MacTableLookup(pAd, &pFtCfg->sta_mld_addr[0]);
		if (pEntry) {
			COPY_MAC_ADDR(pEntry->Addr, &Fr->Hdr.Addr2[0]);
			pFtCfg->update_sta_mac = TRUE;
			pEntry->Sst = SST_ASSOC;
		}
	}
#endif /* DOT11R_FT_SUPPORT */

	if (pEntry == NULL)
		return FALSE;

	/* Fixed item */
	if (isReassoc)
		i = LENGTH_802_11 + LENGTH_802_11_CAP_INFO + LENGTH_802_11_LISTEN_INTERVAL + MAC_ADDR_LEN;
	else
		i = LENGTH_802_11 + LENGTH_802_11_CAP_INFO + LENGTH_802_11_LISTEN_INTERVAL;

	if (MsgLen < i + sizeof(struct _EID_STRUCT)) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
			"Ivalid assoc request\n");
		return FALSE;
	}
	remain_ie_len = MsgLen - i;

	/* record end of frame for IE defragmentation */
	frag_info.frm_end = (UINT8 *)Msg + MsgLen;

#ifdef TXBF_SUPPORT
	/* Reset OUI when recieving a new Assoc Request */
	txbf_clear_oui();
#endif

	COPY_MAC_ADDR(&ie_lists->Addr1[0], &Fr->Hdr.Addr1[0]);
#ifdef DOT11R_FT_SUPPORT
	if (pFtCfg && pFtCfg->update_sta_mac) {
#ifdef RT_CFG80211_SUPPORT
		nl80211_send_event_sta_link_mac(pAd->pCfg80211_CB->pCfg80211_Wdev->wiphy,
				pAd->pCfg80211_CB->pCfg80211_Wdev,
				&Fr->Hdr.Addr2[0],
				MAC_ADDR_LEN);
#endif /* RT_CFG80211_SUPPORT */
		COPY_MAC_ADDR(&pFtCfg->sta_link_addr[0], &Fr->Hdr.Addr2[0]);
		COPY_MAC_ADDR(&ie_lists->Addr2[0], &Fr->Hdr.Addr2[0]);
		COPY_MAC_ADDR(&Fr->Hdr.Addr2[0], &pFtCfg->sta_mld_addr[0]);
	} else
#endif /* DOT11R_FT_SUPPORT */
		COPY_MAC_ADDR(&ie_lists->Addr2[0], &Fr->Hdr.Addr2[0]);
	Ptr = (PCHAR)Fr->Octet;
	NdisMoveMemory(&ie_lists->CapabilityInfo, &Fr->Octet[0], 2);
	NdisMoveMemory(&ie_lists->ListenInterval, &Fr->Octet[2], 2);

	if (isReassoc) {
		NdisMoveMemory(&ie_lists->ApAddr[0], &Fr->Octet[4], 6);
		eid_ptr = (PEID_STRUCT) &Fr->Octet[10];
	} else
		eid_ptr = (PEID_STRUCT) &Fr->Octet[4];

	/* get variable fields from payload and advance the pointer */
	while ((remain_ie_len >= 2) && ((eid_ptr->Len+2) <= remain_ie_len)) {
		switch (eid_ptr->Eid) {
		case IE_SSID:
			if (((Sanity & 0x1) == 1))
				break;

			if (parse_ssid_ie(eid_ptr)) {
				Sanity |= 0x01;
				NdisMoveMemory(&ie_lists->Ssid[0], eid_ptr->Octet, eid_ptr->Len);
				ie_lists->SsidLen = eid_ptr->Len;
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
					"PeerAssocReqSanity - SsidLen = %d\n", ie_lists->SsidLen);
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					"PeerAssocReqSanity - wrong IE_SSID\n");
				return FALSE;
			}

			break;

		case IE_SUPP_RATES:
			if (parse_support_rate_ie(rate, eid_ptr))
				Sanity |= 0x2;
			else {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
					"wrong IE_SUPP_RATES (len=%d)\n", eid_ptr->Len);
				return FALSE;
			}
			break;

		case IE_EXT_SUPP_RATES:
			if (parse_support_ext_rate_ie(rate, eid_ptr) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
					"wrong IE_EXT_SUPP_RATES\n");
				return FALSE;
			}
			break;

		case IE_HT_CAP:
			if (parse_ht_cap_ie(eid_ptr->Len)) {
				NdisMoveMemory(pHtCapability, eid_ptr->Octet, SIZE_HT_CAP_IE);
				*(USHORT *)(&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
				{
					EXT_HT_CAP_INFO extHtCapInfo;

					NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&pHtCapability->ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
					*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
					NdisMoveMemory((PUCHAR)(&pHtCapability->ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
				}
#else
				*(USHORT *)(&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */
#ifdef CFG_BIG_ENDIAN
				*(USHORT *)(&pHtCapability->TxBFCap) = le2cpu32(*(USHORT *)(&pHtCapability->TxBFCap));
#endif
				SET_HT_CAPS_EXIST(ie_lists->cmm_ies.ie_exists);
				Sanity |= 0x10;
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
					"IE_HT_CAP\n");
			} else {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					"wrong IE_HT_CAP.eid_ptr->Len = %d\n",
					eid_ptr->Len);
				return FALSE;
			}
			break;

		case IE_EXT_CAPABILITY:
			if (eid_ptr->Len) {
				INT ext_len = eid_ptr->Len;
				ext_len = ext_len > sizeof(EXT_CAP_INFO_ELEMENT) ? sizeof(EXT_CAP_INFO_ELEMENT) : ext_len;
				NdisMoveMemory(&ie_lists->ExtCapInfo, eid_ptr->Octet, ext_len);
#ifdef CFG_BIG_ENDIAN
				pextCapInfo = (UCHAR *)&ie_lists->ExtCapInfo;
				*((UINT32 *)pextCapInfo) = cpu2le32(*((UINT32 *)pextCapInfo));
				*((UINT32 *)(pextCapInfo + 4)) = cpu2le32(*((UINT32 *)(pextCapInfo + 4)));
#endif
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
					"IE_EXT_CAPABILITY!\n");
			}

			break;

		case IE_WPA:    /* same as IE_VENDOR_SPECIFIC */

#ifdef QOS_R2
			if (NdisEqualMemory(eid_ptr->Octet, wfa_oui, 3) &&
				(eid_ptr->Octet[3] == WFA_CAPA_IE_OUI_TYPE) && (eid_ptr->Len >= 6)) {
				ie_lists->DSCPPolicyEnable = eid_ptr->Octet[5] & 0x01;
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"DSCPPolicyEnable = %d\n", ie_lists->DSCPPolicyEnable);
			}
#endif

#ifdef MBO_SUPPORT
			if (IS_MBO_ENABLE(&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev) &&
					(eid_ptr->Octet[0] == MBO_OCE_OUI_0) &&
					(eid_ptr->Octet[1] == MBO_OCE_OUI_1) &&
					(eid_ptr->Octet[2] == MBO_OCE_OUI_2) &&
					(eid_ptr->Octet[3] == MBO_OCE_OUI_TYPE) &&
					(eid_ptr->Len >= 5)
			   ) {
				MboParseStaMboIE(pAd, &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev, pEntry, eid_ptr->Octet, eid_ptr->Len, MBO_FRAME_TYPE_ASSOC_REQ);
				break;
			}
#ifdef OCE_SUPPORT
			if (IS_OCE_ENABLE(&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev) && (eid_ptr->Octet[0] == MBO_OCE_OUI_0) &&
					(eid_ptr->Octet[1] == MBO_OCE_OUI_1) &&
					(eid_ptr->Octet[2] == MBO_OCE_OUI_2) &&
					(eid_ptr->Octet[3] == MBO_OCE_OUI_TYPE) &&
					(eid_ptr->Len >= 5)
			   ) {
				OceParseStaAssoc(pAd, &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev, pEntry, eid_ptr->Octet, eid_ptr->Len, OCE_FRAME_TYPE_ASSOC_REQ);
				break;
			}
#endif /* OCE_SUPPORT */
#endif /* MBO_SUPPORT */

#ifdef IGMP_TVM_SUPPORT
			if (IS_IGMP_TVM_MODE_EN(pEntry->wdev->IsTVModeEnable)) {
				if (NdisEqualMemory(eid_ptr->Octet, IGMP_TVM_OUI, 4) &&
					(eid_ptr->Len == IGMP_TVM_IE_LENGTH)) {
					RTMPMoveMemory(&ie_lists->tvm_ie, &eid_ptr->Eid, IGMP_TVM_IE_LENGTH+2);
					break;
				}
			}
#endif /* IGMP_TVM_SUPPORT */


#ifdef CONFIG_MAP_SUPPORT
			if (map_check_cap_ie(eid_ptr, &map_cap
#ifdef MAP_R2
				, &map_profile, &map_vid
#endif
				) == TRUE) {
				ie_lists->MAP_AttriValue = map_cap;
#ifdef MAP_R2
				ie_lists->MAP_ProfileValue = map_profile;
#endif
			}
#endif /* CONFIG_MAP_SUPPORT */
#ifdef MWDS

			if (NdisEqualMemory(MTK_OUI, eid_ptr->Octet, 3)) {
				if (MWDS_SUPPORT(eid_ptr->Octet[3])) {
					pEntry->bSupportMWDS = TRUE;
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
						"Peer supports MWDS\n");
				} else
					pEntry->bSupportMWDS = FALSE;
			}

#endif /* MWDS */
#ifdef WH_EVENT_NOTIFIER

			if (pAd->ApCfg.EventNotifyCfg.CustomOUILen &&
				(eid_ptr->Len >= pAd->ApCfg.EventNotifyCfg.CustomOUILen) &&
				NdisEqualMemory(eid_ptr->Octet, pAd->ApCfg.EventNotifyCfg.CustomOUI, pAd->ApCfg.EventNotifyCfg.CustomOUILen)) {
				pEntry->custom_ie_len = eid_ptr->Len;
				NdisMoveMemory(pEntry->custom_ie, eid_ptr->Octet, eid_ptr->Len);
				break;
			}

#endif /* WH_EVENT_NOTIFIER */
#ifdef TXBF_SUPPORT
			/*
			 * For DWA-192 BFee disable workaround. Use "no OUI" to recognize DWA-192.
			 * Most vendors have its own OUI other than 00:50:f2 (Microsoft): BCM, QCM, MTK.
			 * Have no OUI but Microsoft 00:50:f2: RTK, Intel.
			 */
			if (!NdisEqualMemory(WPA_OUI, eid_ptr->Octet, 3)) { /* Find OUI, which is not 00:50:f2 */
				pEntry->has_oui++;
			}
			MTWF_DBG(NULL, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
				"[%d] has_oui: %d\n", pEntry->wcid, pEntry->has_oui);

			/* Record Broadcom OUI in Assoc Request for BF IOT */
			MTWF_DBG(NULL, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
				"eid_ptr->Octet[0-2] = %x, %x, %x\n",
				eid_ptr->Octet[0], eid_ptr->Octet[1], eid_ptr->Octet[2]);
			if (NdisEqualMemory(BROADCOM_OUI, eid_ptr->Octet, 3)) {
				txbf_set_oui(ENUM_BF_OUI_BROADCOM);
			}
#endif /* TXBF_SUPPORT */
			fallthrough;
		case IE_WPA2:
#ifdef DOT11R_FT_SUPPORT
#endif /* DOT11R_FT_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2

			if (NdisEqualMemory(eid_ptr->Octet, HS2OUIBYTE, sizeof(HS2OUIBYTE)) && (eid_ptr->Len >= 5)) {
				/* UCHAR tmp2 = 0x12; */
				UCHAR *hs2_config = (UCHAR *)&eid_ptr->Octet[4];
				UCHAR ppomo_exist = ((*hs2_config) >> 1) & 0x01;
				UCHAR hs2_version = ((*hs2_config) >> 4) & 0x0f;
				/* UCHAR *tmp3 = (UCHAR *)&pEntry->hs_info.ppsmo_id; */
				/* UCHAR tmp[2] = {0x12,0x34}; */
				pEntry->hs_info.version = hs2_version;
				pEntry->hs_info.ppsmo_exist = ppomo_exist;

				if (pEntry->hs_info.ppsmo_exist) {
					NdisMoveMemory(&pEntry->hs_info.ppsmo_id, &eid_ptr->Octet[5], 2);
					/* NdisMoveMemory(tmp3, tmp, 2); */
				}

				break;
			}

#endif /* CONFIG_HOTSPOT_R2 */
#ifdef CONFIG_HOTSPOT_R3
			if (NdisEqualMemory(eid_ptr->Octet, HS2_ROAMING_CONSORTIUM_SELECTION_OUI, sizeof(HS2_ROAMING_CONSORTIUM_SELECTION_OUI))) {
				NdisZeroMemory(&pEntry->hs_consortium_oi, sizeof(STA_HS_CONSORTIUM_OI));
				pEntry->hs_consortium_oi.sta_wcid = pEntry->wcid;
				pEntry->hs_consortium_oi.oi_len = eid_ptr->Len - sizeof(HS2_ROAMING_CONSORTIUM_SELECTION_OUI);

				if (pEntry->hs_consortium_oi.oi_len == 3 || pEntry->hs_consortium_oi.oi_len == 5) {
					NdisMoveMemory(&pEntry->hs_consortium_oi.selected_roaming_consortium_oi[0], &eid_ptr->Octet[4], pEntry->hs_consortium_oi.oi_len);
				}

				break;
			}
#endif /* CONFIG_HOTSPOT_R3 */
			if (NdisEqualMemory(eid_ptr->Octet, WPS_OUI, 4)) {
#ifdef WSC_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

				if ((pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.WscV2Info.bWpsEnable) ||
					(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.WscV2Info.bEnableWpsV2 == FALSE))
#endif /* WSC_V2_SUPPORT */
					ie_lists->bWscCapable = TRUE;
#endif /* WSC_AP_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
				ie_lists->bWscCapable = TRUE;
#endif

				break;
			}

			/* Handle Atheros and Broadcom draft 11n STAs */
			if (NdisEqualMemory(eid_ptr->Octet, BROADCOM_OUI, 3)) {
				switch (eid_ptr->Octet[3]) {
				case 0x33:
					if ((eid_ptr->Len - 4) == sizeof(HT_CAPABILITY_IE)) {
						NdisMoveMemory(pHtCapability, &eid_ptr->Octet[4], SIZE_HT_CAP_IE);
						*(USHORT *)(&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
						{
							EXT_HT_CAP_INFO extHtCapInfo;

							NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&pHtCapability->ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
							*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
							NdisMoveMemory((PUCHAR)(&pHtCapability->ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
						}
#else
						*(USHORT *)(&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */
						SET_HT_CAPS_EXIST(ie_lists->cmm_ies.ie_exists);
					}

					break;

				default:
					/* ignore other cases */
					break;
				}
			}

			check_vendor_ie(pAd, (UCHAR *)eid_ptr, &(ie_lists->cmm_ies.vendor_ie));

#ifdef VHT_TXBF_2G_EPIGRAM_IE
			if (ie_lists->vendor_ie.is_brcm_etxbf_2G
					&& HAS_HT_CAPS_EXIST(ie_lists->cmm_ies.ie_exists)) {
				if (WMODE_CAP_2G(wdev->PhyMode))
					pEntry->rStaBfRecVendorUpdate.fgIsBrcm2GeTxBFIe = TRUE;
				pEntry->rStaBfRecVendorUpdate.Nrow =
					get_max_nss_by_htcap_ie_mcs(
						&(ie_lists->cmm_ies.ht_cap.MCSSet[0]));
			}
#endif /* VHT_TXBF_2G_EPIGRAM_IE */

			/* WMM_IE */
			if (NdisEqualMemory(eid_ptr->Octet, WME_INFO_ELEM, 6) && (eid_ptr->Len == 7)) {
				ie_lists->bWmmCapable = TRUE;
#ifdef UAPSD_SUPPORT

				if (pEntry) {
					UAPSD_AssocParse(pAd,
									 pEntry, (UINT8 *)&eid_ptr->Octet[6],
									 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.UapsdInfo.bAPSDCapable);
				}

#endif /* UAPSD_SUPPORT */
				break;
			}

			if (IS_NO_SECURITY(&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig)
				|| IS_CIPHER_WEP(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig.PairwiseCipher))
				break;

			/*	If this IE did not begins with 00:0x50:0xf2:0x01,
				it would be proprietary. So we ignore it. */
			if (!NdisEqualMemory(eid_ptr->Octet, WPA1_OUI, sizeof(WPA1_OUI))
				&& !(eid_ptr->Eid == IE_WPA2)) {
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					"Not RSN IE, maybe WMM IE!!!\n");
#ifdef CONFIG_HOTSPOT_R2

				if (!NdisEqualMemory(eid_ptr->Octet, HS2_OSEN_OUI, sizeof(HS2_OSEN_OUI))) {
					unsigned char *tmp = (unsigned char *)eid_ptr->Octet;

					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
						"!!!!!!not found OSEN IE,%x:%x:%x:%x\n",
						*tmp, *(tmp + 1), *(tmp + 2), *(tmp + 3));
					pEntry->OSEN_IE_Len = 0;
					CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE);
					break;
				}

				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE);
				NdisMoveMemory(pEntry->OSEN_IE, eid_ptr, eid_ptr->Len + 2);
				pEntry->OSEN_IE_Len = eid_ptr->Len + 2;
				SET_AKM_WPA2(pEntry->SecConfig.AKMMap);
				SET_CIPHER_CCMP128(pEntry->SecConfig.PairwiseCipher);
				SET_CIPHER_CCMP128(pEntry->SecConfig.GroupCipher);
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					"!!!!!!found OSEN IE Entry AuthMode %s, EncryptType %s\n",
					GetAuthModeStr(pEntry->SecConfig.AKMMap),
					GetEncryModeStr(pEntry->SecConfig.PairwiseCipher));
#else
				break;
#endif
			}

			/* Copy whole RSNIE context */
			NdisMoveMemory(&ie_lists->RSN_IE[0], eid_ptr, eid_ptr->Len + 2);
			ie_lists->RSNIE_Len = eid_ptr->Len + 2;
#ifdef DOT11R_FT_SUPPORT
			NdisMoveMemory(pFtInfo->RSN_IE, eid_ptr, eid_ptr->Len + 2);
			pFtInfo->RSNIE_Len = eid_ptr->Len + 2;
#endif /* DOT11R_FT_SUPPORT */

			break;
#ifdef DOT11R_FT_SUPPORT

		case IE_FT_MDIE:
			if (FT_FillMdIeInfo(eid_ptr, &pFtInfo->MdIeInfo) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_NOTICE,
						"wrong IE_FT_MDIE\n");
				return FALSE;
			}
			break;

		case IE_FT_FTIE:
			if (FT_FillFtIeInfo(eid_ptr, &pFtInfo->FtIeInfo) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_NOTICE,
						"wrong IE_FT_FTIE\n");
				return FALSE;
			}
			break;

		case IE_FT_RIC_DATA:

			/* record the pointer of first RDIE. */
			if (pFtInfo->RicInfo.pRicInfo == NULL) {
				pFtInfo->RicInfo.pRicInfo = &eid_ptr->Eid;
				pFtInfo->RicInfo.Len = ((UCHAR *)Fr + MsgLen) - (UCHAR *)eid_ptr + 1;
			}
			break;

		case IE_FT_RIC_DESCRIPTOR:
			if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
				NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
							   &eid_ptr->Eid, eid_ptr->Len + 2);
				pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
			} else {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_NOTICE,
						"wrong IE_FT_RIC_DESCRIPTOR\n");
				return FALSE;
			}
			break;
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT

		case IE_RRM_EN_CAP: {
			UINT64 value;

			if (parse_rm_enable_cap_ie(eid_ptr)) {
				NdisMoveMemory(&value, eid_ptr->Octet, eid_ptr->Len);
				pRrmEnCap->word = le2cpu64(value);
			} else {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_NOTICE,
						"wrong IE_RRM_EN_CAP\n");
				return FALSE;
			}
		}
		break;
#endif /* DOT11K_RRM_SUPPORT */
#ifdef DOT11_VHT_AC

		case IE_VHT_CAP:
			if (eid_ptr->Len == sizeof(VHT_CAP_IE)) {
				NdisMoveMemory(&ie_lists->cmm_ies.vht_cap, eid_ptr->Octet, sizeof(VHT_CAP_IE));
				SET_VHT_CAPS_EXIST(ie_lists->cmm_ies.ie_exists);
#ifdef CFG_BIG_ENDIAN
				NdisCopyMemory(&tmp_1, &ie_lists->cmm_ies.vht_cap.vht_cap, 4);
				tmp_1 = le2cpu32(tmp_1);
				NdisCopyMemory(&ie_lists->cmm_ies.vht_cap.vht_cap, &tmp_1, 4);

				NdisCopyMemory(&tmp_2, &(ie_lists->cmm_ies.vht_cap.mcs_set), 8);
				tmp_2 = le2cpu64(tmp_2);
				NdisCopyMemory(&(ie_lists->cmm_ies.vht_cap.mcs_set), &tmp_2, 8);
#endif
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
					"IE_VHT_CAP\n");
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
					"wrong IE_VHT_CAP, eid->Len = %d\n",
					eid_ptr->Len);
			}
			break;

		case IE_VHT_OP:
			if (eid_ptr->Len == sizeof(VHT_OP_IE)) {
				NdisMoveMemory(&ie_lists->cmm_ies.vht_op, eid_ptr->Octet, sizeof(VHT_OP_IE));
				SET_VHT_OP_EXIST(ie_lists->cmm_ies.ie_exists);
#ifdef CFG_BIG_ENDIAN
				NdisCopyMemory(&tmp, &ie_lists->cmm_ies.vht_op.basic_mcs_set, sizeof(VHT_MCS_MAP));
				tmp = le2cpu16(tmp);
				NdisCopyMemory(&ie_lists->cmm_ies.vht_op.basic_mcs_set, &tmp, sizeof(VHT_MCS_MAP));
#endif
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					"IE_VHT_OP\n");
			}
			break;

		case IE_OPERATING_MODE_NOTIFY:
			if (eid_ptr->Len == sizeof(OPERATING_MODE)) {
				ie_lists->cmm_ies.operating_mode_len = sizeof(OPERATING_MODE);
				NdisMoveMemory(&ie_lists->cmm_ies.operating_mode,
						&eid_ptr->Octet[0], sizeof(OPERATING_MODE));
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					"IE_OPERATING_MODE_NOTIFY!\n");
			}

			break;
#endif /* DOT11_VHT_AC */

		case IE_SUPP_CHANNELS:
			if (eid_ptr->Len > MAX_LEN_OF_SUPPORTED_CHL || (eid_ptr->Len % 2)) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					"wrong IE_SUPP_CHANNELS, eid->Len = %d\n",
					 eid_ptr->Len);
			} else if (eid_ptr->Len + ie_lists->SupportedChlLen <= MAX_LEN_OF_SUPPORTED_CHL) {
				UINT32 _ChlIdx = ie_lists->SupportedChlLen %
								 MAX_LEN_OF_SUPPORTED_CHL;
				NdisMoveMemory(&ie_lists->SupportedChl[_ChlIdx], eid_ptr->Octet,
							   eid_ptr->Len);
				ie_lists->SupportedChlLen += eid_ptr->Len;
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
					"IE_SUPP_CHANNELS, eid->Len = %d\n",
					eid_ptr->Len);
			} else  if (ie_lists->SupportedChlLen <  MAX_LEN_OF_SUPPORTED_CHL)  {
				NdisMoveMemory(&ie_lists->SupportedChl[ie_lists->SupportedChlLen], eid_ptr->Octet,
							   MAX_LEN_OF_SUPPORTED_CHL - (ie_lists->SupportedChlLen));
				ie_lists->SupportedChlLen = MAX_LEN_OF_SUPPORTED_CHL;
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
					"IE_SUPP_CHANNELS, eid->Len = %d (Exceeded)\n",
					eid_ptr->Len);
			}

			if (ie_lists->SupportedChlLen > MAX_LEN_OF_SUPPORTED_CHL)
				ie_lists->SupportedChlLen = MAX_LEN_OF_SUPPORTED_CHL;

			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
				"  Supported Channels: [ FirstCh : NumOfCh ]\n");

			for (i = 0; i < ie_lists->SupportedChlLen; i += 2)
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
				"  [ %4d : %4d ]\n", ie_lists->SupportedChl[i],
				ie_lists->SupportedChl[i + 1]);

			break;
#ifdef DOT11_HE_AX
		case IE_RSNXE:
			/* Copy whole RSNXEIE context */
			NdisMoveMemory(&ie_lists->rsnxe_ie[0], eid_ptr, eid_ptr->Len + 2);
			ie_lists->rsnxe_ie_len = eid_ptr->Len + 2;
			break;
#endif /* DOT11_HE_AX */
		case IE_SUPP_REG_CLASS:
			ie_lists->current_opclass = eid_ptr->Octet[0];
			break;
		case IE_WLAN_EXTENSION:
		{
			/*parse EXTENSION EID*/
			UCHAR *extension_id = (UCHAR *)eid_ptr + 2;

#ifdef DOT11_EHT_BE
			if (eht_get_ies((UINT8 *)eid_ptr, &ie_lists->cmm_ies, &frag_info) < 0)
				return FALSE;
#endif /* DOT11_EHT_BE */

			switch (*extension_id) {
			case IE_EXTENSION_ID_ECDH:
#if defined(CONFIG_OWE_SUPPORT) || defined(HOSTAPD_OWE_SUPPORT)
			{
				if (eid_ptr->Len > (sizeof(ie_lists->ecdh_ie) - 2)) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
						"IE_WLAN_EXTENSION: parsing HE related ies(%d) unexpected error\n", *extension_id);
					break;
				}
				os_zero_mem(&ie_lists->ecdh_ie, sizeof(ie_lists->ecdh_ie));
				ie_lists->ecdh_ie.ext_ie_id = IE_WLAN_EXTENSION;
				ie_lists->ecdh_ie.length = eid_ptr->Len;
				NdisMoveMemory(&ie_lists->ecdh_ie.ext_id_ecdh, eid_ptr->Octet, eid_ptr->Len);
			}
#endif /*CONFIG_OWE_SUPPORT*/
				break;

#ifdef QOS_R1
			case IE_EXTENSION_ID_MSCS_DESC:
				ie_lists->has_mscs_req = QoS_parse_mscs_descriptor_ies(pAd, pEntry->wdev,
								&ie_lists->Addr2[0], eid_ptr->Octet, eid_ptr->Len);
				break;
#endif
#ifdef DOT11_HE_AX
			case IE_EXTENSION_ID_HE_CAP:
			fallthrough;
			case IE_EXTENSION_ID_HE_6G_CAP:
				if (parse_he_assoc_req_ies((UINT8 *)eid_ptr, ie_lists) <= 0) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
						"IE_WLAN_EXTENSION: parsing HE related ies(%d) unexpected error\n", *extension_id);
				}
				break;
#endif
			default:
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"IE_WLAN_EXTENSION: no handler for extension_id:%d\n", *extension_id);
				break;
			}
		}
			break;

		case IE_ADD_HT:
				if (parse_ht_info_ie(eid_ptr)) {
					NdisMoveMemory(&ie_lists->cmm_ies.ht_op, eid_ptr->Octet, eid_ptr->Len);
					SET_HT_OP_EXIST(ie_lists->cmm_ies.ie_exists);
				} else {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
							"%s() - wrong IE_ADD_HT\n", __func__);
					return FALSE;
				}
			break;

		default:
			break;
		}

		if (frag_info.is_frag) {
			remain_ie_len -= frag_info.ie_len_all;
			eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + frag_info.ie_len_all);
			frag_info.is_frag = FALSE;
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"[frag] Jump to next IE: %d, (defrag=%d)\n",
				frag_info.ie_len_all, frag_info.ie_len_defrag);
		} else {
			remain_ie_len -= (2 + eid_ptr->Len);
			eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
		}
	}

	if ((Sanity & 0x3) != 0x03) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
			"missing mandatory field\n");
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG, "success\n");
	return TRUE;

}

static BOOLEAN rate_is_11b_only(struct legacy_rate *rate)
{
	BOOLEAN only_11b = TRUE;
	UCHAR idx;

	for (idx = 0; idx < rate->sup_rate_len; idx++) {
		if (((rate->sup_rate[idx] & 0x7F) != 2) &&
			((rate->sup_rate[idx] & 0x7F) != 4) &&
			((rate->sup_rate[idx] & 0x7F) != 11) &&
			((rate->sup_rate[idx] & 0x7F) != 22)) {
			only_11b = FALSE;
			break;
		}
	}

	return only_11b;
}

#ifdef CONFIG_MAP_SUPPORT
static BOOLEAN is_controller_found(struct wifi_dev *wdev)
{
	struct map_vendor_ie *ie = (struct map_vendor_ie *)wdev->MAPCfg.vendor_ie_buf;

	if (ie->connectivity_to_controller)
		return TRUE;

	return FALSE;
}
#endif
#ifdef A4_CONN
void del_ppe_entry_by_roaming_detected(struct wifi_dev *wdev, unsigned char *addr)
{
	UCHAR i, band_idx;
	UINT16 tmp_wcid;
	PRTMP_ADAPTER tmp_pad = NULL;
	struct wifi_dev *tmp_wdev = NULL;
	BOOLEAN del_en = FALSE;

	if (!wdev || !addr)
		return;

	for (band_idx = 0; band_idx < MAX_BAND_NUM; band_idx++) {
		tmp_pad = bss_mngr_mld_get_pad_by_band(wdev, band_idx);
		if (!tmp_pad)
			continue;
		for (i = 0; i < tmp_pad->ApCfg.BssidNum; i++) {
			tmp_wdev = &tmp_pad->ApCfg.MBSSID[i].wdev;
			if (RoutingTabLookup(tmp_pad, tmp_wdev->func_idx, addr, FALSE, &tmp_wcid)) {
				entry_del_hw_ppe_entry(tmp_pad, addr);
				del_en = TRUE;
				break;
			}
		}
		if (del_en)
			break;
	}
}
#endif
static VOID ap_cmm_peer_assoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN BOOLEAN isReassoc)
{
	struct wifi_dev *wdev = NULL;
	struct legacy_rate *rate;
	BSS_STRUCT *pMbss;
	BOOLEAN bAssocSkip = FALSE;
	CHAR rssi;
	IE_LISTS *ie_list = NULL;
	HEADER_802_11 AssocRspHdr;
	USHORT CapabilityInfoForAssocResp;
	USHORT StatusCode = MLME_SUCCESS;
	USHORT Aid = 0, PhyMode;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	UCHAR MaxSupportedRate = 0;
	UCHAR SupRateLen;
	UCHAR sum_rate[MAX_LEN_OF_SUPPORTED_RATES], sum_rate_len, *p_sum_rate;
	BOOLEAN FlgIs11bSta;
	UCHAR check_rsnxe_install = TRUE;
#ifdef MBO_SUPPORT
#ifdef RT_CFG80211_SUPPORT
	P_MBO_CTRL pMboCtrl = NULL;
#endif /* RT_CFG80211_SUPPORT */
#endif /* MBO_SUPPORT */
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	struct BA_INFO *ba_info;
#ifdef DBG
	UCHAR *sAssoc = isReassoc ? (PUCHAR)"ReASSOC" : (PUCHAR)"ASSOC";
#endif /* DBG */
	UCHAR SubType;
	BOOLEAN bACLReject = FALSE;
#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_BL_SUPPORT)
	BOOLEAN bBlReject = FALSE;
#endif
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg = NULL;
	PFT_INFO FtInfoBuf = NULL;
#endif /* DOT11R_FT_SUPPORT */
#ifdef WSC_AP_SUPPORT
	WSC_CTRL *wsc_ctrl;
#endif /* WSC_AP_SUPPORT */
	ADD_HT_INFO_IE *addht;
	struct _build_ie_info ie_info = {0};
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	struct customer_vendor_ie *ap_vendor_ie;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
#ifdef WAPP_SUPPORT
	UINT8 wapp_cnnct_stage = WAPP_ASSOC;
	UINT16 wapp_assoc_fail = NOT_FAILURE;
#endif /* WAPP_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	struct _ASIC_SEC_INFO *info = NULL;
	PUCHAR rsnxe_ie = NULL;
#endif /* DOT11R_FT_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2
		PHOTSPOT_CTRL pHSCtrl = NULL;
#endif
#ifdef MBO_SUPPORT
	UCHAR APIndex;
#endif /* MBO_SUPPORT */
#ifdef WARP_512_SUPPORT
	MAC_TABLE_ENTRY *pA4Entry = NULL;
	BOOLEAN mwds_enable = FALSE;
#endif /* WARP_512_SUPPORT */
	BOOLEAN is_mlo_setup_link = (Elem && Elem->Others) ? FALSE : TRUE;
#ifdef DOT11_EHT_BE
	SST setup_link_Sst;
	ULONG offset_ml_ie = 0;
	UINT8 *ml_ie_buf = NULL;
	BOOLEAN setup_link_success = TRUE;
	struct eht_assoc_req_priv *assoc_info = NULL;
#endif /* DOT11_EHT_BE */
	PUINT8 assoc_resp_frame = NULL;
	UINT16 assoc_resp_frame_len = 0;
#ifdef MAP_R6
	ULONG TNow;
#endif /* MAP_R6 */

#ifdef DFS_SLAVE_SUPPORT
	if (SLAVE_BEACON_STOPPED(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"[DFS-SLAVE] beaconing off, ignore assoc\n");
		goto assoc_check;
	} /* DFS_SLAVE_SUPPORT */
#endif

#ifdef DOT11R_FT_SUPPORT
	os_alloc_mem_suspend(NULL, (UCHAR **)&FtInfoBuf, sizeof(FT_INFO));

	if (FtInfoBuf == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_ERROR,
				 "mem alloc failed\n");
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_NO_RESOURCE;
#endif /* WAPP_SUPPORT */
		goto assoc_check;
	}

	NdisZeroMemory(FtInfoBuf, sizeof(FT_INFO));


/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&rsnxe_ie, MAX_LEN_OF_RSNXEIE);

	if (rsnxe_ie == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_ERROR,
				 "mem alloc failed\n");
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_NO_RESOURCE;
#endif /* WAPP_SUPPORT */
		goto assoc_check;
	}
#endif /* DOT11R_FT_SUPPORT */

	NdisZeroMemory(rsnxe_ie, MAX_LEN_OF_RSNXEIE);

	/* disallow new association */
	if (pAd->ApCfg.BANClass3Data == TRUE) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				 "Disallow new Association\n");
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = DISALLOW_NEW_ASSOCI;
#endif /* WAPP_SUPPORT */
		goto assoc_check;
	}

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(IE_LISTS));

	if (ie_list == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "mem alloc failed\n");
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_NO_RESOURCE;
#endif /* WAPP_SUPPORT */
		goto assoc_check;
	}

	NdisZeroMemory(ie_list, sizeof(IE_LISTS));
	if (!PeerAssocReqCmmSanity(pAd, isReassoc, Elem->Msg, Elem->MsgLen, ie_list))
		goto LabelOK;

#ifdef DOT11_EHT_BE
	if (!PeerAssocReqMloCapSanity(pAd, isReassoc, Elem->Msg,
			Elem->MsgLen, ie_list, is_mlo_setup_link))
		goto LabelOK;
#endif /* DOT11_EHT_BE */

	if (isReassoc) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
			"received re-assoc req from "MACSTR" %s ...\n",
			MAC2STR(ie_list->Addr2), is_mlo_setup_link ? "" : "(NS)");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
			"received assoc req from "MACSTR" %s ...\n",
			MAC2STR(ie_list->Addr2), is_mlo_setup_link ? "" : "(NS)");
	}

#if defined(CONFIG_MAP_SUPPORT)
#ifdef MAP_R6
	pEntry = MacTableLookup(pAd, ie_list->Addr2);
	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "NoAuth MAC in reassoc case - "MACSTR"\n",
				  MAC2STR(ie_list->Addr2));
		goto LabelOK;
	}
	if (IS_MAP_ENABLE(pAd) && !IS_MAP_CERT_ENABLE(pAd)
		&& isReassoc && is_mlo_setup_link) {
		if (MAC_ADDR_EQUAL(pEntry->current_reassoc_mac, ZERO_MAC_ADDR)) {
			NdisGetSystemUpTime(&pEntry->current_reassc_time);
			memcpy(pEntry->current_reassoc_mac, ie_list->Addr2, MAC_ADDR_LEN);
			MTWF_PRINT("[%s - %d] current_reassc_time = %lu\n", __func__, __LINE__, pEntry->current_reassc_time);
			MTWF_PRINT("[%s - %d] current_reassoc_mac = " MACSTR "\n", __func__, __LINE__, MAC2STR(pEntry->current_reassoc_mac));
		} else if (MAC_ADDR_EQUAL(pEntry->current_reassoc_mac, ie_list->Addr2)) {
			MTWF_PRINT("[%s - %d] this is same mac as before need to check time diff\n", __func__, __LINE__);
			NdisGetSystemUpTime(&TNow);
			if (TNow < (pEntry->current_reassc_time + (15 * OS_HZ))) {
				MTWF_PRINT("[%s - %d] reassoc from same mac within 15 sec need to ignore\n", __func__, __LINE__);
				memcpy(pEntry->current_reassoc_mac, ZERO_MAC_ADDR, MAC_ADDR_LEN);
				pEntry->current_reassc_time = 0;
				goto LabelOK;
			} else {
				MTWF_PRINT("[%s - %d] reassoc from same mac more than 15 sec need to update timer\n", __func__, __LINE__);
				NdisGetSystemUpTime(&pEntry->current_reassc_time);
			}
		} else {
			memcpy(pEntry->current_reassoc_mac, ie_list->Addr2, MAC_ADDR_LEN);
			NdisGetSystemUpTime(&pEntry->current_reassc_time);
			MTWF_PRINT("[%s - %d] current_reassc_time = %lu\n", __func__, __LINE__, pEntry->current_reassc_time);
			MTWF_PRINT("[%s - %d] current_reassoc_mac = " MACSTR "\n", __func__, __LINE__, MAC2STR(pEntry->current_reassoc_mac));
		}
	} else {
		MTWF_PRINT("[%s - %d] assoc case set to zero\n", __func__, __LINE__);
		memcpy(pEntry->current_reassoc_mac, ZERO_MAC_ADDR, MAC_ADDR_LEN);
		pEntry->current_reassc_time = 0;
	}
#endif /* MAP_R6 */
#endif /* CONFIG_MAP_SUPPORT */

#ifdef DOT11_EHT_BE
	/* build multi-link connect request */
	if (HAS_EHT_MLD_EXIST(ie_list->cmm_ies.ie_exists)) {
		wdev = wdev_search_by_address(pAd, ie_list->Addr1);

		if (wdev) {
			uint16_t mld_sta_idx;

			mld_sta_idx = eht_build_multi_link_conn_req(wdev, SUBTYPE_ASSOC_REQ,
				ie_list->cmm_ies.ml_ie, &ie_list->cmm_ies.ml_frag_info,
				ie_list->cmm_ies.t2l_ie, ie_list->Addr2, NULL, 0);

			if (mld_sta_idx == MLD_STA_NONE) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"Fail: mld conn request (bss:%s, setup addr:%pM)\n",
					RtmpOsGetNetDevName(wdev->if_dev), ie_list->Addr2);
				goto LabelOK;
			}
		}
	}
#endif /* DOT11_EHT_BE */

	pEntry = MacTableLookup(pAd, ie_list->Addr2);

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "NoAuth MAC - "MACSTR"\n",
				  MAC2STR(ie_list->Addr2));
		goto LabelOK;
	}

	if (pEntry->func_tb_idx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "pEntry bounding invalid wdev(apidx=%d)\n",
				  pEntry->func_tb_idx);
		goto LabelOK;
	}
#ifdef DOT11_EHT_BE
#ifdef MWDS
	if (pEntry->wdev) {
		if (pEntry->bSupportMWDS && pEntry->wdev->bSupportMWDS) {
			mlo_mwds_ap_sync(pAd, pEntry);
		}
	}
#endif
#endif
#ifdef WARP_512_SUPPORT
#if defined(MWDS) || defined(CONFIG_MAP_SUPPORT)
	if (pAd->Warp512Support &&
		((pEntry->wcid < A4_APCLI_FIRST_WCID) ||
		(pEntry->wcid >= A4_APCLI_FIRST_WCID + MAX_RESERVE_ENTRY))) {
#ifdef MWDS
		if (pEntry->bSupportMWDS && pEntry->wdev && pEntry->wdev->bSupportMWDS)
			mwds_enable = TRUE;
#endif
#ifdef CONFIG_MAP_SUPPORT
			pEntry->DevPeerRole = ie_list->MAP_AttriValue;
			if (IS_MAP_ENABLE(pAd) &&
			(pEntry->wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_BACKHAUL_BSS)) &&
			(pEntry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA))) {
				pA4Entry = ReassignWcidForA4Entry(pAd, pEntry, ie_list);
				if (!pA4Entry)
					goto assoc_check;

				pEntry = pA4Entry;
				mwds_enable = FALSE;
			}
#endif /* CONFIG_MAP_SUPPORT */
#ifdef MWDS
		if (mwds_enable) {
			pA4Entry = ReassignWcidForA4Entry(pAd, pEntry, ie_list);
			if (!pA4Entry)
				goto assoc_check;

			pEntry = pA4Entry;
		}
#endif
	}
#endif /* defined(MWDS) || defined(CONFIG_MAP_SUPPORT) */
#endif /* WARP_512_SUPPORT */

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
			 "%s():pEntry->func_tb_idx=%d\n",
			  __func__, pEntry->func_tb_idx);
	wdev = wdev_search_by_address(pAd, ie_list->Addr1);

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "Wrong Addr1 - "MACSTR"\n",
				  MAC2STR(ie_list->Addr1));
		goto LabelOK;
	}

	/* Correct the pEntry member, when STA is ReAsso to AP  */
	if (wdev->func_idx != pEntry->func_tb_idx) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "@@ ERROR 1! - MAC="MACSTR", wdev->func_idx=%d, pEntry->func_tb_idx=%d, pEntry->wcid=%d\n",
				  MAC2STR(pEntry->Addr), wdev->func_idx, pEntry->func_tb_idx, pEntry->wcid);
		pEntry->func_tb_idx = wdev->func_idx;
#ifdef MBO_SUPPORT
		if (0 == pEntry->is_mbo_bndstr_sta)
			pEntry->is_mbo_bndstr_sta = 1;
#endif /* MBO_SUPPORT */


	}

	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	tr_entry = tr_entry_get(pAd, pEntry->wcid);
#ifdef WSC_AP_SUPPORT
	wsc_ctrl = &wdev->WscControl;
#endif /* WSC_AP_SUPPORT */
	PhyMode = wdev->PhyMode;
	rate = &wdev->rate.legacy_rate;
#ifdef DOT11_EHT_BE
	/* Sst of setup link for sync non-setup link association */
	setup_link_Sst = pEntry->Sst;
#endif
	/* prevent setup basic rate with self capability that exceed peer's capability */
	if ((ie_list->rate.sup_rate_len + ie_list->rate.ext_rate_len) < (rate->sup_rate_len + rate->ext_rate_len)) {
		rate = &ie_list->rate;
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			 "%s(): Support rate follow STA's settings\n", __func__);
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			 "%s(): Support rate follow AP's settings\n", __func__);
	}
	addht = wlan_operate_get_addht(wdev);

	if (!OPSTATUS_TEST_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "AP is not ready, disallow new Association\n");
		goto LabelOK;
	}

	if (pAd->FragFrame.wcid == pEntry->wcid) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO, "\nClear Wcid = %d FragBuffer !!!!!\n", pEntry->wcid);
		RESET_FRAGFRAME(pAd->FragFrame);
	}

#ifdef OCE_FILS_SUPPORT
	if ((pEntry->filsInfo.is_post_assoc == TRUE) &&
		(pEntry->filsInfo.auth_algo == AUTH_MODE_FILS) &&
		IS_AKM_FILS(wdev->SecConfig.AKMMap) &&
		IS_AKM_FILS(pEntry->SecConfig.AKMMap)) {

		StatusCode = pEntry->filsInfo.status;
		pEntry->filsInfo.is_post_assoc = FALSE;
		goto assoc_post;
	}
#endif /* OCE_FILS_SUPPORT */

	ie_info.frame_subtype = SUBTYPE_ASSOC_RSP;
	ie_info.channel = wdev->channel;
	ie_info.phy_mode = PhyMode;
	ie_info.wdev = wdev;

#ifdef HTC_DECRYPT_IOT

	if ((pEntry->HTC_ICVErrCnt)
		|| (pEntry->HTC_AAD_OM_Force)
		|| (pEntry->HTC_AAD_OM_CountDown)
		|| (pEntry->HTC_AAD_OM_Freeze)
	   ) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "@@@ (wcid=%u), HTC_ICVErrCnt(%u), HTC_AAD_OM_Freeze(%u), HTC_AAD_OM_CountDown(%u),  HTC_AAD_OM_Freeze(%u) is in Asso. stage!\n",
				  pEntry->wcid, pEntry->HTC_ICVErrCnt, pEntry->HTC_AAD_OM_Force, pEntry->HTC_AAD_OM_CountDown,
				  pEntry->HTC_AAD_OM_Freeze);
		/* Force clean. */
		pEntry->HTC_ICVErrCnt = 0;
		pEntry->HTC_AAD_OM_Force = 0;
		pEntry->HTC_AAD_OM_CountDown = 0;
		pEntry->HTC_AAD_OM_Freeze = 0;
	}

#endif /* HTC_DECRYPT_IOT */
	FlgIs11bSta = rate_is_11b_only(rate);

#ifdef CONFIG_MAP_SUPPORT
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
			"%s():IS_MAP_ENABLE(pAd)=%d IS_MAP_BS_ENABLE(pAd)=%d\n", __func__, IS_MAP_ENABLE(pAd), IS_MAP_BS_ENABLE(pAd));
	if ((IS_MAP_ENABLE(pAd)) || (IS_MAP_BS_ENABLE(pAd))) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
				"%s():Assoc Req len=%ld, ASSOC_REQ_LEN = %d\n",
				 __func__, Elem->MsgLen, ASSOC_REQ_LEN);
		if (Elem->MsgLen > ASSOC_REQ_LEN)
			pEntry->assoc_req_len = ASSOC_REQ_LEN;
		else
			pEntry->assoc_req_len = Elem->MsgLen;
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
		if (pEntry->mlo.mlo_en && !(pEntry->mlo.is_setup_link_entry)) {
			struct _MAC_TABLE_ENTRY *entry_ptr;
			struct mld_entry_t *mld_entry = NULL;
			int i = 0;

			mt_rcu_read_lock();
			mld_entry = rcu_dereference(pEntry->mld_entry);
			if (mld_entry) {
				for (i = 0; i < MLD_LINK_MAX; i++) {
					entry_ptr = mld_entry->link_entry[i];

					if (!entry_ptr)
						continue;

					if (entry_ptr->mlo.is_setup_link_entry) {
						NdisMoveMemory(pEntry->assoc_req_frame, entry_ptr->assoc_req_frame, entry_ptr->assoc_req_len);
						pEntry->assoc_req_len = entry_ptr->assoc_req_len;
						break;
					}
				}
			} else
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "ERROR, mld_entry=NULL\n");

			mt_rcu_read_unlock();
		} else {
#endif  /* MTK_MLO_MAP_SUPPORT  && DOT11_EHT_BE */
			NdisMoveMemory(pEntry->assoc_req_frame, Elem->Msg, pEntry->assoc_req_len);
#ifdef MTK_MLO_MAP_SUPPORT
		}
#endif /* MTK_MLO_MAP_SUPPORT && DOT11_EHT_BE */
	}
#endif

#ifdef GN_MIXMODE_SUPPORT
	pEntry->FlgIs11bSta = FlgIs11bSta;
#endif /*GN_MIXMODE_SUPPORT*/

#ifdef MBO_SUPPORT
	if (IS_MBO_ENABLE(wdev)) {
		/* Set the Assoc disallow reason to 2 if Max sta num reached */
		if (pAd->ApCfg.EntryClientCount > GET_MAX_UCAST_NUM(pAd)) {
			wdev->MboCtrl.AssocDisallowReason = MBO_AP_DISALLOW_MAX_STA_NUM_REACHED;
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
				"MBO Max Sta num %d reached\n",
				pAd->ApCfg.EntryClientCount);

			if (isReassoc) {
				for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++) {
					if (MAC_ADDR_EQUAL(ie_list->ApAddr, pAd->ApCfg.MBSSID[APIndex].wdev.bssid)) {
						pEntry->is_mbo_bndstr_sta = 1;
						break;
					}
				}
			}
		}

		/* Disallow assoc with RC = 17 if client entry is greater than MaxStaNum*/
		if (!MBO_AP_ALLOW_ASSOC(wdev) && 1 != pEntry->is_mbo_bndstr_sta) {
			StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;
#ifdef WAPP_SUPPORT
			wapp_assoc_fail = MLME_UNABLE_HANDLE_STA;
#endif /* WAPP_SUPPORT */
			goto SendAssocResponse;
		}
	}
#endif /* MBO_SUPPORT */

	/* YF@20120419: Refuse the weak signal of AssocReq */
	rssi = RTMPMaxRssi(pAd,
					   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
					   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
					   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
			 "ra[%d] ASSOC_REQ Threshold = %d, PktMaxRssi=%d\n",
			  pEntry->func_tb_idx, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].AssocReqRssiThreshold,
			  rssi);

	if ((pAd->ApCfg.MBSSID[pEntry->func_tb_idx].AssocReqRssiThreshold != 0) &&
		(rssi < pAd->ApCfg.MBSSID[pEntry->func_tb_idx].AssocReqRssiThreshold)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "Reject this ASSOC_REQ due to Weak Signal.\n");
#ifdef OCE_SUPPORT
		if (IS_OCE_ENABLE(wdev)) {
			struct oce_info *oceInfo = &pEntry->oceInfo;
			CHAR rssiThres = pAd->ApCfg.MBSSID[pEntry->func_tb_idx].AssocReqRssiThreshold;

			oceInfo->DeltaAssocRSSI = (rssiThres > rssi) ?
				(rssiThres - rssi) : 0;
			StatusCode = MLME_DISASSOC_LOW_ACK;
#ifdef WAPP_SUPPORT
			wapp_assoc_fail = MLME_UNABLE_HANDLE_STA;
#endif /* WAPP_SUPPORT */
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"Reject the AssocReq and DeltaRssi :%d\n",
				oceInfo->DeltaAssocRSSI);

			goto SendAssocResponse;
		} else
#endif /* OCE_SUPPORT */
			bAssocSkip = TRUE;
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			"Accept RSSI: ===> %d, %d\n",
			pAd->ApCfg.MBSSID[pEntry->func_tb_idx].AssocReqRssiThreshold, rssi);
	}

#if defined(RT_CFG80211_SUPPORT) && defined(HOSTAPD_PMKID_IN_DRIVER_SUPPORT)
	{
		/*handle PMKID bug of iphone, sending PMKID even on phone reboot/forget AP*/
		UINT8 pmkid_count = 0;
		UCHAR zeroPmkid[LEN_PMKID] = {0};
		PUINT8 pPMKID = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE, ie_list->RSNIE_Len, PMKID_LIST, &pmkid_count);

		if ((pPMKID != NULL) && (!IS_AKM_OWE(pEntry->SecConfig.AKMMap))) {
			hex_dump_with_lvl("PMKID from AssocReq", pPMKID, LEN_PMKID, DBG_LVL_INFO);
			if (NdisCmpMemory(pEntry->PmkidByHostapd, zeroPmkid, LEN_PMKID) &&
				NdisCmpMemory(pPMKID, pEntry->PmkidByHostapd, LEN_PMKID)) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"PMKID not match (pEntry->PmkidByHostapd is not equal to AssocReq->PMKID)!!\n");
				hex_dump_with_cat_and_lvl("pEntry->PMKID", pEntry->PmkidByHostapd, LEN_PMKID, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO);
				hex_dump_with_cat_and_lvl("AssocReq->PMKID", pPMKID, LEN_PMKID, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO);
				StatusCode = MLME_INVALID_PMKID;
#ifdef WAPP_SUPPORT
				wapp_assoc_fail = MLME_INVALID_PMKID;
#endif /* WAPP_SUPPORT */
				goto SendAssocResponse;
			}  else {
				INT cacheidx;
				UCHAR *peer_mac = pEntry->Addr;
				UCHAR is_mlo_connect = FALSE;
				UCHAR *own_mac = wdev->bssid;
				BOOLEAN is_owe;
#ifdef DOT11_EHT_BE
				if (pEntry->mlo.mlo_en) {
					is_mlo_connect = TRUE;
					own_mac = wdev->bss_info_argument.mld_info.mld_addr;
					peer_mac = pEntry->mlo.mld_addr;
					if (MAC_ADDR_EQUAL(peer_mac, ZERO_MAC_ADDR)) {
						MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
							"peer_mac invalid\n");
						goto assoc_check;
					}
				}
#endif
				is_owe = IS_AKM_OWE(wdev->SecConfig.AKMMap);
#ifdef CFG_RSNO_SUPPORT
				if (!is_owe)
					is_owe = IS_AKM_OWE(wdev->SecConfig_ext.AKMMap);
#endif /* CFG_RSNO_SUPPORT */
				if (IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap)
#ifdef MTK_MLO_MAP_SUPPORT
					&& (!IS_MAP_ENABLE(pAd))
#endif /* MTK_MLO_MAP_SUPPORT */
					&& (!is_owe)
					&& (!IS_AKM_FT_SAE(wdev->SecConfig.AKMMap))
					&& is_rsne_pmkid_cache_match(ie_list->RSN_IE,
					ie_list->RSNIE_Len,
					PD_GET_PMKID_PTR(pAd->physical_dev),
					own_mac,
					peer_mac,
					is_mlo_connect,
					&cacheidx)
					&& (cacheidx == INVALID_PMKID_IDX)) {
						StatusCode = MLME_INVALID_PMKID;
						MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
							"PMKSA cache not match!!\n");
						goto SendAssocResponse;
				} else
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
						"PMKID match\n");
			}
		} else
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO, "PMKID NULL\n");
	}
#endif /*RT_CFG80211_SUPPORT && HOSTAPD_PMKID_IN_DRIVER_SUPPORT*/

#ifdef DOT11W_PMF_SUPPORT

	if ((tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
		&& (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)
#ifdef DOT11R_FT_SUPPORT
		&& (!IS_FT_RSN_STA(pEntry))
#endif /* DOT11R_FT_SUPPORT */
#ifdef OCE_FILS_SUPPORT
				&& (pEntry->filsInfo.auth_algo != AUTH_MODE_FILS)
#endif /* OCE_FILS_SUPPORT */
		) {
		StatusCode = MLME_ASSOC_REJ_TEMPORARILY;
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_ASSOC_REJ_TEMP;
#endif /* WAPP_SUPPORT */
		goto SendAssocResponse;
	}

#endif /* DOT11W_PMF_SUPPORT */

	/* clear the previous Pairwise key table */
	if ((pEntry->Aid != 0)
#ifdef DOT11R_FT_SUPPORT
	&& (!IS_FT_STA(pEntry))
#endif /* DOT11R_FT_SUPPORT */
#ifdef OCE_FILS_SUPPORT
		&& (pEntry->filsInfo.auth_algo != AUTH_MODE_FILS)
#endif /* OCE_FILS_SUPPORT */
		&& ((!IS_AKM_OPEN(pEntry->SecConfig.AKMMap)) || (!IS_AKM_SHARED(pEntry->SecConfig.AKMMap))
#ifdef DOT1X_SUPPORT
			|| IS_IEEE8021X(&pEntry->SecConfig)
#endif /* DOT1X_SUPPORT */
		   )) {
		ASIC_SEC_INFO *Info;
		/* clear GTK state */
		pEntry->SecConfig.Handshake.GTKState = REKEY_NEGOTIATING;
		NdisZeroMemory(&pEntry->SecConfig.PTK, LEN_MAX_PTK);
		/* Set key material to Asic */

		os_alloc_mem(pAd, (UCHAR **)&Info, sizeof(ASIC_SEC_INFO));
		if (Info == NULL) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"allocate memory for Info failed!\n");
			goto assoc_check;
		}
		os_zero_mem(Info, sizeof(ASIC_SEC_INFO));

		Info->Operation = SEC_ASIC_REMOVE_PAIRWISE_KEY;
		Info->Wcid = pEntry->wcid;
#ifdef SW_CONNECT_SUPPORT
		if (IS_SW_STA(tr_entry))
			Info->Wcid = tr_entry->HwWcid;

		if (IS_NORMAL_STA(tr_entry))
#endif /* SW_CONNECT_SUPPORT */
		{
			/* Set key material to Asic */
			HW_ADDREMOVE_KEYTABLE(pAd, Info);
		}
#if defined(DOT1X_SUPPORT) && !defined(RADIUS_ACCOUNTING_SUPPORT)

		/* Notify 802.1x daemon to clear this sta info */
		if (IS_AKM_1X(pEntry->SecConfig.AKMMap)
			|| IS_IEEE8021X(&pEntry->SecConfig))
			DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

#endif /* DOT1X_SUPPORT */
		os_free_mem(Info);
	}

	pEntry->bWscCapable = ie_list->bWscCapable;
#ifdef WSC_AP_SUPPORT
	/* since sta has been left, ap should receive EapolStart and EapRspId again. */
	pEntry->Receive_EapolStart_EapRspId = 0;
#ifdef WSC_V2_SUPPORT

	if ((wsc_ctrl->WscV2Info.bEnableWpsV2) &&
		(wsc_ctrl->WscV2Info.bWpsEnable == FALSE))
		;
	else
#endif /* WSC_V2_SUPPORT */
	{
		if (pEntry->func_tb_idx < pAd->ApCfg.BssidNum) {
			if (MAC_ADDR_EQUAL(pEntry->Addr, wsc_ctrl->EntryAddr)) {
				BOOLEAN Cancelled;

				RTMPZeroMemory(wsc_ctrl->EntryAddr, MAC_ADDR_LEN);
				RTMPCancelTimer(&wsc_ctrl->EapolTimer, &Cancelled);
				wsc_ctrl->EapolTimerRunning = FALSE;
			}
		}

		if ((ie_list->RSNIE_Len == 0
#ifdef CFG_RSNO_SUPPORT
				&& (ie_list->rsneo_ie_len == 0)
				&& (ie_list->rsnxeo_ie_len == 0)
#endif /* CFG_RSNO_SUPPORT */
				)
			&& (IS_AKM_WPA_CAPABILITY_Entry(wdev))
			&& (wsc_ctrl->WscConfMode != WSC_DISABLE))
			pEntry->bWscCapable = TRUE;
	}
#endif /* WSC_AP_SUPPORT */

		/* for hidden SSID sake, SSID in AssociateRequest should be fully verified */
		if ((ie_list->SsidLen != pMbss->SsidLen) ||
			(NdisEqualMemory(ie_list->Ssid, pMbss->Ssid, ie_list->SsidLen) == 0)) {
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"SSID mismatch, MBSS(len=%d, %s), AssocReq(len=%d, %s)\n",
				pMbss->SsidLen, pMbss->Ssid, ie_list->SsidLen, ie_list->Ssid);
			goto LabelOK;
		}

#ifdef WSC_V2_SUPPORT
#if defined(RT_CFG80211_SUPPORT) && defined(WSC_INCLUDED)
	/* Do not check ACL when WPS V2 is enabled and ACL policy is positive. */
	if ((pMbss->wdev.wps_triggered) && (pMbss->AccessControlList.Policy == 1) && ie_list->bWscCapable)
		;
#else
	if ((pMbss->wdev.WscControl.WscConfMode != WSC_DISABLE) &&
			(pMbss->wdev.WscControl.bWscTrigger) &&
			(pMbss->wdev.WscControl.WscV2Info.bWpsEnable) &&
			(pMbss->AccessControlList.Policy == 1))
		;
#endif
	else
#endif /* WSC_V2_SUPPORT */
#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_BL_SUPPORT)
		/* set a flag for sending Assoc-Fail response to unwanted STA later. */
		if (map_is_entry_bl(pAd, ie_list->Addr2, pEntry->func_tb_idx) == TRUE) {
			bBlReject = TRUE;
		} else
#endif /*  MAP_BL_SUPPORT */

#ifdef DOT11_EHT_BE
			/*check mld addr for mlo acl*/
			if (pEntry->mlo.mlo_en) {
				/*check if mld addr do acl fail*/
				if (!mld_check_acl(pAd, pEntry->mlo.mld_addr, pMbss->mld_grp_idx)) {
					bACLReject = TRUE;
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
					"ASSOC - BSS_%d(%pM): mld addr(%pM) check ACL fail.\n",
					wdev->func_idx, wdev->if_addr, pEntry->mlo.mld_addr);
				}
#ifdef MAP_R6
				/*for map r6:if we check pass for mld addr for black list,
				 *we need also check every link addr, if need deny the mld sta*/
				if (bACLReject == FALSE &&
					pAd->ApCfg.MBSSID[pMbss->mbss_idx].AccessControlList.mlo_policy == 2) {
					u8 link_id = 0;
					MAC_TABLE_ENTRY *entry_ptr = NULL;
					struct mld_entry_t *mld_entry = NULL;

					mt_rcu_read_lock();
					mld_entry = rcu_dereference(pEntry->mld_entry);
					if (mld_entry) {
						for (link_id = 0; link_id < MLD_LINK_MAX; link_id++) {
							entry_ptr = mld_entry->link_entry[link_id];

							if (!entry_ptr)
								continue;

							if (!ApCheckAccessControlList(entry_ptr->pAd, entry_ptr->Addr, entry_ptr->func_tb_idx)) {
								bACLReject = TRUE;
								MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
								"%s - link(%d)-MBSS(%d): MLD link addr(%pM) check ACL fail.\n",
								sAssoc, link_id, entry_ptr->func_tb_idx, entry_ptr->Addr);
								break;
							}
						}
					} else
						MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
						 "ERROR, mld_entry=NULL\n");

					mt_rcu_read_unlock();
				}
#endif
			} else
#endif
				/* set a flag for sending Assoc-Fail response
					to unwanted STA later.
				*/
				if (!ApCheckAccessControlList(pAd, ie_list->Addr2,
						pEntry->func_tb_idx))
					bACLReject = TRUE;

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_BL_SUPPORT)
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			 "%s - MBSS(%d), receive %s request from "MACSTR" Rej BL/ACL %d/%d\n",
			  sAssoc, pEntry->func_tb_idx, sAssoc, MAC2STR(ie_list->Addr2), bBlReject, bACLReject);
#else
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			 "%s - MBSS(%d), receive %s request from "MACSTR"\n",
			  sAssoc, pEntry->func_tb_idx, sAssoc, MAC2STR(ie_list->Addr2));
#endif

	p_sum_rate = (PUCHAR)sum_rate;
	SupportRate(rate, &p_sum_rate, &sum_rate_len, &MaxSupportedRate);

	/*
		Assign RateLen here or we will select wrong rate table in
		APBuildAssociation() when 11N compile option is disabled.
	*/
	pEntry->RateLen = sum_rate_len;
	RTMPSetSupportMCS(pAd,
					  OPMODE_AP,
					  pEntry,
					  rate,
#ifdef DOT11_VHT_AC
					  HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists),
					  &ie_list->cmm_ies.vht_cap,
#endif /* DOT11_VHT_AC */
					  &ie_list->cmm_ies.ht_cap,
					  HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists));
#ifdef GN_MIXMODE_SUPPORT
	if (pAd->CommonCfg.GNMixMode
		&& (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
			|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)
			|| WMODE_EQUAL(wdev->PhyMode, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G)))) {
		pEntry->SupportRateMode &= ~SUPPORT_CCK_MODE;
		pEntry->SupportCCKMCS &= ~(1 << MCS_0 | 1 << MCS_1 | 1 << MCS_2 | 1 << MCS_3);
	}
#endif /* GN_MIXMODE_SUPPORT */

	/* 2. qualify this STA's auth_asoc status in the MAC table, decide StatusCode */
	StatusCode = APBuildAssociation(pAd, pEntry, ie_list, MaxSupportedRate, &Aid, isReassoc);

	if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC)) {
		update_sta_conn_state(wdev, pEntry);
	}

#ifdef WAPP_SUPPORT
	if (StatusCode != MLME_SUCCESS)
		wapp_assoc_fail = MLME_UNABLE_HANDLE_STA;
#endif /* WAPP_SUPPORT */

#ifdef DOT11R_FT_SUPPORT

	if (pEntry->func_tb_idx < pAd->ApCfg.BssidNum
#ifdef DOT11_EHT_BE
		&& (!pEntry->mlo.mlo_en || !isReassoc)
#endif
	) {
		pFtCfg = &(wdev->FtCfg);

		if ((pFtCfg->FtCapFlag.Dot11rFtEnable)
			&& (StatusCode == MLME_SUCCESS))
			StatusCode = FT_AssocReqHandler(pAd, isReassoc, pFtCfg, pEntry,
											&ie_list->FtInfo,
											ie_list->rsnxe_ie,
											ie_list->rsnxe_ie_len,
											FtInfoBuf);

#ifdef WAPP_SUPPORT
		if (StatusCode != MLME_SUCCESS)
			wapp_assoc_fail = FT_ERROR;
#endif /* WAPP_SUPPORT */

		/* just silencely discard this frame */
		if (StatusCode == 0xFFFF)
			goto LabelOK;
	}

#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT

	if ((pEntry->func_tb_idx < pAd->ApCfg.BssidNum)
		&& IS_RRM_ENABLE(wdev))
		pEntry->RrmEnCap.word = ie_list->RrmEnCap.word;

#endif /* DOT11K_RRM_SUPPORT */
#ifdef DOT11_VHT_AC

	if (HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists)) {
		/* +++Add by shiang for debug */
		if (WMODE_CAP_AC(wdev->PhyMode)) {
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
					 "%s():Peer is VHT capable device!\n", __func__);
			NdisMoveMemory(&pEntry->ext_cap, &ie_list->ExtCapInfo, sizeof(ie_list->ExtCapInfo));
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
					 "\tOperatingModeNotification=%d\n",
					  pEntry->ext_cap.operating_mode_notification);
			/* dump_vht_cap(pAd, &ie_list->vht_cap); */
		}

		/* ---Add by shiang for debug */
	}

#endif /* DOT11_VHT_AC */

#ifdef IGMP_TVM_SUPPORT
		if (IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
			/* Check whether the Peer has TV IE or not, because this needs to be set to */
			/* FW to enable/disabled Mcast Packet cloning and conversion */
			if (ie_list->tvm_ie.len == IGMP_TVM_IE_LENGTH) {
				if (ie_list->tvm_ie.data.field.TVMode == IGMP_TVM_IE_MODE_ENABLE)
					pEntry->TVMode = IGMP_TVM_IE_MODE_ENABLE;
				else
					pEntry->TVMode = IGMP_TVM_IE_MODE_AUTO;
			} else {
				pEntry->TVMode = IGMP_TVM_IE_MODE_DISABLE;
			}
		} else {
			pEntry->TVMode = IGMP_TVM_IE_MODE_DISABLE;
		}
#endif /* IGMP_TVM_SUPPORT */


	if (StatusCode == MLME_ASSOC_REJ_DATA_RATE)
		RTMPSendWirelessEvent(pAd, IW_STA_MODE_EVENT_FLAG, pEntry->Addr, wdev->wdev_idx, 0);

SendAssocResponse:

	/* 3. send Association Response */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		goto LabelOK;

#ifdef MAP_R3
	if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd) && (ie_list->ecdh_ie.ext_ie_id == IE_WLAN_EXTENSION)
#ifdef HOSTAPD_OWE_SUPPORT
	 && (wdev) && (!(IS_AKM_OWE(wdev->SecConfig.AKMMap)))
#endif
	) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "ASSOC rejected due to Diffie-Hellman IE element!\n");
		StatusCode = MLME_ASSOC_DENY_OUT_SCOPE;
	}
#endif /* MAP_R3 */

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			 "%s - Send %s response (Status=%d)...\n",
			  sAssoc, sAssoc, StatusCode);
	Aid |= 0xc000; /* 2 most significant bits should be ON */
	SubType = isReassoc ? SUBTYPE_REASSOC_RSP : SUBTYPE_ASSOC_RSP;
	CapabilityInfoForAssocResp = pMbss->CapabilityInfo; /*use AP's cability */
#ifdef WSC_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

	if ((wsc_ctrl->WscV2Info.bEnableWpsV2) &&
		(wsc_ctrl->WscV2Info.bWpsEnable == FALSE))
		;
	else
#endif /* WSC_V2_SUPPORT */
	{
		if ((wsc_ctrl->WscConfMode != WSC_DISABLE) &&
			(ie_list->CapabilityInfo & 0x0010))
			CapabilityInfoForAssocResp |= 0x0010;
	}

#endif /* WSC_AP_SUPPORT */
	/* fail in ACL checking => send an Assoc-Fail resp. */
	SupRateLen = rate->sup_rate_len;

	/* TODO: need to check rate in support rate element, not number */
	if (FlgIs11bSta == TRUE)
		SupRateLen = 4;

	if (bACLReject == TRUE || bAssocSkip
#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_BL_SUPPORT)
		|| bBlReject == TRUE
#endif
	) {
		MgtMacHeaderInit(pAd, &AssocRspHdr, SubType, 0, ie_list->Addr2,
						 wdev->if_addr, wdev->bssid);
		StatusCode = MLME_UNSPECIFY_FAIL;
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_UNSPECIFY_FAILURE;
#endif /* WAPP_SUPPORT */
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(HEADER_802_11), &AssocRspHdr,
						  2,                        &CapabilityInfoForAssocResp,
						  2,                        &StatusCode,
						  2,                        &Aid,
						  END_OF_ARGS);
		FrameLen += build_support_rate_ie(wdev, rate->sup_rate, SupRateLen, pOutBuffer + FrameLen);
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
		MlmeFreeMemory((PVOID) pOutBuffer);
		pOutBuffer = NULL;
		RTMPSendWirelessEvent(pAd, IW_MAC_FILTER_LIST_EVENT_FLAG, ie_list->Addr2, wdev->wdev_idx, 0);
#ifdef WSC_V2_SUPPORT

		/* If this STA exists, delete it. */
		if (pEntry) {
#ifdef RT_CFG80211_SUPPORT
			CFG80211_ApStaDelSendEvent(pAd, pEntry->Addr, wdev->if_dev);
#endif /* RT_CFG80211_SUPPORT */

			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			pEntry = NULL;
		}

#endif /* WSC_V2_SUPPORT */

		if (bAssocSkip == TRUE) {
			pEntry = MacTableLookup(pAd, ie_list->Addr2);

			if (pEntry) {
#ifdef RT_CFG80211_SUPPORT
				CFG80211_ApStaDelSendEvent(pAd, pEntry->Addr, wdev->if_dev);
#endif /* RT_CFG80211_SUPPORT */

				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
				pEntry = NULL;
			}
		}

		goto LabelOK;
	}

	MgtMacHeaderInit(pAd, &AssocRspHdr, SubType, 0, ie_list->Addr2,
					 wdev->if_addr, wdev->bssid);
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(HEADER_802_11), &AssocRspHdr,
					  2,                        &CapabilityInfoForAssocResp,
					  2,                        &StatusCode,
					  2,                        &Aid,
					  END_OF_ARGS);
	FrameLen += build_support_rate_ie(wdev, rate->sup_rate, SupRateLen, pOutBuffer + FrameLen);

	if (FlgIs11bSta == FALSE)
		FrameLen += build_support_ext_rate_ie(wdev, SupRateLen, rate->ext_rate,
						rate->ext_rate_len, pOutBuffer + FrameLen);

#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd)) {
		pEntry->DevPeerRole = ie_list->MAP_AttriValue;
#ifdef MAP_R2
		pEntry->profile = ie_list->MAP_ProfileValue;
		if (((pEntry->profile == 0) && pAd->ApCfg.MBSSID[pEntry->func_tb_idx].bh_disallow_info.profile1_bh_assoc_disallow) ||
				((pEntry->profile == 2) && pAd->ApCfg.MBSSID[pEntry->func_tb_idx].bh_disallow_info.profile2_bh_assoc_disallow)) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"Reject this ASSOC_REQ due bh_assoc disallow pEntry->profile %d P1 %d, P2 %d.\n",
				pEntry->profile, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].bh_disallow_info.profile1_bh_assoc_disallow,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].bh_disallow_info.profile2_bh_assoc_disallow);
			StatusCode = MLME_ASSOC_DENY_OUT_SCOPE;
		}
#endif /* MAP_R2 */
		if ((IS_MAP_TURNKEY_ENABLE(pAd)) &&
			(((pEntry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)) &&
			(wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_BACKHAUL_BSS)) &&
			!(is_controller_found(wdev))) ||
			((!ie_list->MAP_AttriValue) &&
			(!(wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_FRONTHAUL_BSS)))))) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"disallowing connection, DevOwnRole=%u,DevPeerRole=%u,controller=%d\n",
					 wdev->MAPCfg.DevOwnRole, pEntry->DevPeerRole, is_controller_found(wdev));
			MlmeDeAuthAction(pAd, pEntry, REASON_DECLINED, FALSE);
			goto LabelOK;
		} else
			MAP_InsertMapCapIE(pAd, wdev, pOutBuffer+FrameLen, &FrameLen);
	}
#endif /* CONFIG_MAP_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT

	if (IS_RRM_ENABLE(wdev))
		RRM_InsertRRMEnCapIE(pAd, wdev, pOutBuffer + FrameLen, &FrameLen, pEntry->func_tb_idx);

#endif /* DOT11K_RRM_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT

	if (StatusCode == MLME_ASSOC_REJ_TEMPORARILY) {
		ULONG TmpLen;
		UCHAR IEType = IE_TIMEOUT_INTERVAL; /* IE:0x15 */
		UCHAR IELen = 5;
		UCHAR TIType = 3;
		UINT32 units = 1 << 10; /* 1 seconds, should be 0x3E8 */

		MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
						  1, &IEType,
						  1, &IELen,
						  1, &TIType,
						  4, &units,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}

#endif /* DOT11W_PMF_SUPPORT */
#ifdef CONFIG_OWE_SUPPORT
	if (IS_AKM_OWE_Entry(pEntry)) {
		CHAR rsne_idx;
		ULONG	TmpLen;
		struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;
		/*struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;*/

		WPAMakeRSNIE(wdev->wdev_type, pSecConfig, pEntry);

		for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
			if (pSecConfig->RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
				continue;

			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
								1, &pSecConfig->RSNE_EID[rsne_idx][0],
								1, &pSecConfig->RSNE_Len[rsne_idx],
								pSecConfig->RSNE_Len[rsne_idx],
								&pSecConfig->RSNE_Content[rsne_idx][0],
								END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}
#endif
#ifdef DOT11R_FT_SUPPORT
	if ((pFtCfg != NULL) && (pFtCfg->FtCapFlag.Dot11rFtEnable)) {
		PUINT8	mdie_ptr;
		UINT8	mdie_len;
		PUINT8	ftie_ptr = NULL;
		UINT8	ftie_len = 0;
		PUINT8  ricie_ptr = NULL;
		UINT8   ricie_len = 0;
		UCHAR rsnxe_ie_len = 0;

		rsnxe_ie_len = build_rsnxe_ie(wdev, &wdev->SecConfig, rsnxe_ie);

		if ((rsnxe_ie_len != 0 || wpa3_test_ctrl == 8) && FtInfoBuf->FtIeInfo.MICCtr.field.IECnt)
			FtInfoBuf->FtIeInfo.MICCtr.field.rsnxe_used = 1;

		/* for ft-sae rsnxe interop issue, we should not carry rsnxe if peer not support */
		if (pEntry->SecConfig.rsnxe_len == 0)
			rsnxe_ie_len = 0;

		if (FtInfoBuf->FtIeInfo.MICCtr.field.IECnt) {
			if (rsnxe_ie_len != 0)
				FtInfoBuf->FtIeInfo.MICCtr.field.IECnt++;
			else
				check_rsnxe_install = FALSE;

			if (ie_list->FtInfo.RicInfo.Len)
				FtInfoBuf->FtIeInfo.MICCtr.field.IECnt++;
		}

		/* Insert RSNIE if necessary */
		if (FtInfoBuf->RSNIE_Len != 0) {
			ULONG TmpLen;

			MakeOutgoingFrame(pOutBuffer + FrameLen,      &TmpLen,
							  FtInfoBuf->RSNIE_Len,		FtInfoBuf->RSN_IE,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}

		/* Insert MDIE. */
		mdie_ptr = pOutBuffer + FrameLen;
		mdie_len = 5;

		/* Insert MdId only if the Peer has sent one */
		if (FtInfoBuf->MdIeInfo.Len != 0) {
			FT_InsertMdIE(pOutBuffer + FrameLen,
					  &FrameLen,
					  FtInfoBuf->MdIeInfo.MdId,
					  FtInfoBuf->MdIeInfo.FtCapPlc);
		}

		/* Insert FTIE. */
		if (FtInfoBuf->FtIeInfo.Len != 0) {
			ftie_ptr = pOutBuffer + FrameLen;
			ftie_len = (2 + FtInfoBuf->FtIeInfo.Len);
			FT_InsertFTIE(pOutBuffer + FrameLen, &FrameLen,
						  FtInfoBuf->FtIeInfo.Len,
						  FtInfoBuf->FtIeInfo.MICCtr,
						  FtInfoBuf->FtIeInfo.MIC,
						  FtInfoBuf->FtIeInfo.ANonce,
						  FtInfoBuf->FtIeInfo.SNonce);
		}

		/* Insert R1KH IE into FTIE. */
		if (FtInfoBuf->FtIeInfo.R1khIdLen != 0)
			FT_FTIE_InsertKhIdSubIE(pOutBuffer + FrameLen,
									&FrameLen,
									FT_R1KH_ID,
									FtInfoBuf->FtIeInfo.R1khId,
									FtInfoBuf->FtIeInfo.R1khIdLen);

		/* Insert GTK Key info into FTIE. */
		if (FtInfoBuf->FtIeInfo.GtkLen != 0)
			FT_FTIE_InsertSubIE(pOutBuffer + FrameLen,
								   &FrameLen,
								   FT_GTK,
								   FtInfoBuf->FtIeInfo.GtkSubIE,
								   FtInfoBuf->FtIeInfo.GtkLen);

		/* Insert R0KH IE into FTIE. */
		if (FtInfoBuf->FtIeInfo.R0khIdLen != 0)
			FT_FTIE_InsertKhIdSubIE(pOutBuffer + FrameLen,
									&FrameLen,
									FT_R0KH_ID,
									FtInfoBuf->FtIeInfo.R0khId,
									FtInfoBuf->FtIeInfo.R0khIdLen);

		/* Insert RIC. */
		if (ie_list->FtInfo.RicInfo.Len) {
			ULONG TempLen;

			FT_RIC_ResourceRequestHandle(pAd, pEntry,
										 (PUCHAR)ie_list->FtInfo.RicInfo.pRicInfo,
										 ie_list->FtInfo.RicInfo.Len,
										 (PUCHAR)pOutBuffer + FrameLen,
										 (PUINT32)&TempLen);
			ricie_ptr = (PUCHAR)(pOutBuffer + FrameLen);
			ricie_len = TempLen;
			FrameLen += TempLen;
		}
		/* Insert IGTK Key info into FTIE. */

		if (FtInfoBuf->FtIeInfo.IGtkLen != 0) {
			FT_FTIE_InsertSubIE(pOutBuffer+FrameLen,
					&FrameLen,
					FT_IGTK_ID,
					FtInfoBuf->FtIeInfo.IGtkSubIE,
					FtInfoBuf->FtIeInfo.IGtkLen);
		}

		/* Insert OCI into FTIE. */

		if (FtInfoBuf->FtIeInfo.OCILen != 0) {
			FT_FTIE_InsertSubIE(pOutBuffer+FrameLen,
					&FrameLen,
					FT_OCI_ID,
					FtInfoBuf->FtIeInfo.OCISubIE,
					FtInfoBuf->FtIeInfo.OCILen);
		}

		/* Insert BIGTK Key info into FTIE. */

		if (FtInfoBuf->FtIeInfo.BIGtkLen != 0) {
			FT_FTIE_InsertSubIE(pOutBuffer+FrameLen,
					&FrameLen,
					FT_BIGTK_ID,
					FtInfoBuf->FtIeInfo.BIGtkSubIE,
					FtInfoBuf->FtIeInfo.BIGtkLen);
		}

		/* Calculate the FT MIC for FT procedure */
		if (FtInfoBuf->FtIeInfo.MICCtr.field.IECnt) {
			UINT8	ft_mic[FT_MIC_LEN];
			PFT_FTIE	pFtIe;

			FT_CalculateMIC(pEntry->Addr,
							wdev->bssid,
							pEntry->FT_PTK,
							6,
							FtInfoBuf->RSN_IE,
							FtInfoBuf->RSNIE_Len,
							mdie_ptr,
							mdie_len,
							ftie_ptr,
							ftie_len,
							ricie_ptr,
							ricie_len,
							rsnxe_ie,
							rsnxe_ie_len,
							ft_mic);
			/* Update the MIC field of FTIE */
			pFtIe = (PFT_FTIE)(ftie_ptr + 2);
			NdisMoveMemory(pFtIe->MIC, ft_mic, FT_MIC_LEN);
			/* Install pairwise key */
		}

		/*	Record the MDIE & FTIE of (re)association response of
			Initial Mobility Domain Association. It's used in
			FT 4-Way handshaking */
		if ((IS_AKM_WPA2_Entry(pEntry) || IS_AKM_WPA2PSK_Entry(pEntry)
			|| IS_AKM_WPA3PSK_Entry(pEntry) || IS_AKM_WPA3_192BIT_Entry(pEntry))
			&& ie_list->FtInfo.FtIeInfo.Len == 0) {
			NdisMoveMemory(&pEntry->InitialMDIE,
						   mdie_ptr, mdie_len);
			pEntry->InitialFTIE_Len = ftie_len;
			NdisMoveMemory(pEntry->InitialFTIE, ftie_ptr, ftie_len);
		}
	}
#ifdef DOT11_EHT_BE
	if (pEntry->func_tb_idx < pAd->ApCfg.BssidNum
		&& pEntry->mlo.mlo_en && isReassoc) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			"%s: insert RSNE to non-setup link for MLO FT Reassociation response\n",
			__func__);
		FrameLen += build_rsn_ie(pAd, wdev, (UCHAR *)(pOutBuffer + FrameLen));
	}
#endif /* DOT11_EHT_BE */
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11_N_SUPPORT
	{
		BOOLEAN HtEnable = (HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists) &&
			WMODE_CAP_N(wdev->PhyMode) && wdev->DesiredHtPhyInfo.bHtEnable) ? TRUE : FALSE;

		/* HT capability in AssocRsp frame. */
		if (HtEnable) {
			ie_info.is_draft_n_type = FALSE;
			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
			FrameLen += build_ht_ies(pAd, &ie_info);

			if ((ie_list->cmm_ies.vendor_ie.ra_cap) == 0 || (pAd->bBroadComHT == TRUE)) {
				ie_info.is_draft_n_type = TRUE;
				ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
				FrameLen += build_ht_ies(pAd, &ie_info);

			}
		}

		/* BSS Max Idle Period element */
		if (pMbss->max_idle_ie_en) {
			FrameLen += build_bss_max_idle_ie(wdev, (UCHAR *)(pOutBuffer + FrameLen),
							pMbss->max_idle_period,
							pMbss->max_idle_option);
		}

		if (HtEnable) {
			struct _build_ie_info vht_ie_info;

#ifdef DOT11_VHT_AC
			vht_ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
			vht_ie_info.frame_subtype = SUBTYPE_ASSOC_RSP;
			vht_ie_info.channel = wdev->channel;
			vht_ie_info.phy_mode = wdev->PhyMode;
			vht_ie_info.wdev = wdev;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
			ucETxBfCap = wlan_config_get_etxbf(wdev);

			if (bf_is_support(wdev) == FALSE)
				wlan_config_set_etxbf(wdev, SUBF_OFF);

			txbf_bfee_cap_set(TRUE,
							  ie_list->cmm_ies.vht_cap.vht_cap.bfer_cap_su,
							  ie_list->cmm_ies.vht_cap.vht_cap.num_snd_dimension);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
		if (HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists)
			|| HAS_VHT_OP_EXIST(ie_list->cmm_ies.ie_exists)
			|| !WMODE_CAP_2G(wdev->PhyMode))
			FrameLen += build_vht_ies(pAd, &vht_ie_info);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
			wlan_config_set_etxbf(wdev, ucETxBfCap);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#endif /* DOT11_VHT_AC */
		}

#ifdef DOT11_HE_AX
		if (HAS_HE_CAPS_EXIST(ie_list->cmm_ies.ie_exists)
			&& IS_HE_STA(pEntry->cap.modes) && WMODE_CAP_AX(wdev->PhyMode)
				&& wdev->DesiredHtPhyInfo.bHtEnable) {
			UINT32 offset = 0;

			offset = add_assoc_rsp_he_ies(wdev, (UINT8 *)pOutBuffer, FrameLen);
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					"add he assoc_rsp, len=%d\n", offset);
			FrameLen += offset;
		}
#endif /*DOT11_HE_AX*/
	}
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_HOTSPOT_R2
	/* qosmap IE */
	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO, "entry wcid %d QosMapSupport=%d\n",
				pEntry->wcid, pEntry->QosMapSupport);
	if (pEntry->QosMapSupport) {
		ULONG	TmpLen;
		UCHAR	QosMapIE, ielen = 0, explen = 0;

		pHSCtrl = &pAd->ApCfg.MBSSID[pEntry->apidx].HotSpotCtrl;

		if (pHSCtrl->QosMapEnable && pHSCtrl->HotSpotEnable) {
			QosMapIE = IE_QOS_MAP_SET;

			/* Fixed field Dscp range:16, len:1 IE_ID:1*/
			if (pHSCtrl->QosMapSetIELen > 18)
				explen = pHSCtrl->QosMapSetIELen - 18;

			pEntry->DscpExceptionCount = explen;
			memcpy((UCHAR *)pEntry->DscpRange, (UCHAR *)pHSCtrl->DscpRange, 16);
			memcpy((UCHAR *)pEntry->DscpException, (UCHAR *)pHSCtrl->DscpException, 42);
			ielen = explen + 16;
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  1,			&QosMapIE,
							  1,			&ielen,
							  explen,		pEntry->DscpException,
							  16,			pEntry->DscpRange,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}

#endif /* CONFIG_HOTSPOT_R2 */
#ifdef QOS_R1
	if (IS_QOS_ENABLE(pAd)) {
		ULONG	TmpLen = 0;
		UCHAR	QosMapIE, tmpbuf[50] = {0}, ielen = 0, explen = 0;
		BSS_STRUCT *pmbss = &pAd->ApCfg.MBSSID[wdev->func_idx];

		if (/*pEntry->QosMapSupport &&*/
			pmbss->QosMapSupport && pmbss->QoSMapIsUP) {
			QosMapIE = IE_QOS_MAP_SET;
			explen = pmbss->DscpExceptionCount;
			pEntry->DscpExceptionCount = explen;
			memcpy((UCHAR *)pEntry->DscpRange, (UCHAR *)pmbss->DscpRange, 16);
			memcpy((UCHAR *)pEntry->DscpException, (UCHAR *)pmbss->DscpException, 42);
			ielen = explen + 16;
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  1,			&QosMapIE,
							  1,			&ielen,
							  explen,		pEntry->DscpException,
							  16,			pEntry->DscpRange,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
		if (ie_list->has_mscs_req == 1) {
			QoS_get_default_mscs_descriptor(tmpbuf, &ielen);
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen, ielen, tmpbuf, END_OF_ARGS);
			FrameLen += TmpLen;
		}
#ifdef QOS_R2
		if (pEntry->DSCPPolicyEnable != ie_list->DSCPPolicyEnable) {
			pEntry->DSCPPolicyEnable = ie_list->DSCPPolicyEnable;
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				"%pM, DSCPPolicyEnable=%d\n", pEntry->Addr, pEntry->DSCPPolicyEnable);
		}

		if (IS_QOS_ENABLE(pAd)) {
			QoS_Build_WFACapaIE(tmpbuf, &ielen, pmbss->bDSCPPolicyEnable);
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen, ielen, tmpbuf, END_OF_ARGS);
			FrameLen += TmpLen;
		}
#endif
	}
#endif

	ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
	FrameLen += build_extended_cap_ie(pAd, &ie_info);
#ifdef CONFIG_DOT11V_WNM
		/* #ifdef CONFIG_HOTSPOT_R2 Remove for WNM independance */
		if (ie_list->ExtCapInfo.BssTransitionManmt == 1)
			pEntry->bBSSMantSTASupport = TRUE;
#endif /* CONFIG_DOT11V_WNM */

#ifdef DOT11V_MBSSID_SUPPORT
		if (IS_MBSSID_IE_NEEDED(&pMbss->mbss_11v))
			ie_list->ExtCapInfo.mbssid = 1;
		else
			ie_list->ExtCapInfo.mbssid = 0;
#endif

#ifdef MBO_SUPPORT
	if (IS_MBO_ENABLE(wdev)) {
#ifdef RT_CFG80211_SUPPORT
		pMboCtrl = &wdev->MboCtrl;
		if (pMboCtrl->MboIELen > 0 && pMboCtrl->MboIELen <= MBO_IE_MAX_LEN) {
			ULONG TmpLen;
			/* MBO element */
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
					pMboCtrl->MboIELen,
					pMboCtrl->MBOIE, END_OF_ARGS);
			FrameLen += TmpLen;
		} else
#endif /* RT_CFG80211_SUPPORT */
			MakeMboOceIE(pAd, wdev, pEntry, pOutBuffer+FrameLen, &FrameLen, MBO_FRAME_TYPE_ASSOC_RSP);
	}
#endif /* MBO_SUPPORT */

#ifndef RT_CFG80211_SUPPORT
#ifdef WSC_AP_SUPPORT

	if (pEntry->bWscCapable) {
		UCHAR *pWscBuf = NULL, WscIeLen = 0;
		ULONG WscTmpLen = 0;

		os_alloc_mem(NULL, (UCHAR **)&pWscBuf, 512);

		if (pWscBuf) {
			NdisZeroMemory(pWscBuf, 512);
			WscBuildAssocRespIE(pAd, pEntry->func_tb_idx, 0, 512, pWscBuf, &WscIeLen);
			MakeOutgoingFrame(pOutBuffer + FrameLen, &WscTmpLen,
							  WscIeLen, pWscBuf,
							  END_OF_ARGS);
			FrameLen += WscTmpLen;
			os_free_mem(pWscBuf);
		}
	}

#endif /* WSC_AP_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */

#ifdef OCE_FILS_SUPPORT
	ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
	FrameLen += oce_build_ies(pAd, &ie_info, TRUE);
#endif /*OCE_FILS_SUPPORT */

	if (check_rsnxe_install) {
		if (wpa3_test_ctrl == 9) {
			INT len = build_rsnxe_ie(wdev, &wdev->SecConfig, (UCHAR *)pOutBuffer + FrameLen);
			if (len != 0) {
				UCHAR cap;
				UCHAR *buf = (UCHAR *)pOutBuffer + FrameLen;

				NdisMoveMemory(&cap, buf + 2, sizeof(cap));
				cap |= (1 << IE_RSNXE_CAPAB_PROTECTED_TWT);
				NdisMoveMemory(buf + 2, &cap, sizeof(cap));
				hex_dump_with_cat_and_lvl("rsnxe content", buf, len,
					DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO);

				FrameLen += len;
			}
		} else
			FrameLen +=  build_rsnxe_ie(wdev, &wdev->SecConfig, (UCHAR *)pOutBuffer + FrameLen);
	}
#ifndef RT_CFG80211_SUPPORT
#ifdef CONFIG_OWE_SUPPORT
	if (IS_AKM_OWE_Entry(pEntry) && (StatusCode == MLME_SUCCESS)) {
		BOOLEAN need_ecdh_ie = FALSE;
		INT CacheIdx;/* Key cache */
		UINT8 *pmkid = NULL;
		UINT8 pmkid_count = 0;
		UCHAR *peer_mac = pEntry->Addr;
		UCHAR is_mlo_connect = FALSE;
		UCHAR *own_mac = wdev->bssid;

#ifdef DOT11_EHT_BE
		if (pEntry->mlo.mlo_en) {
			is_mlo_connect = TRUE;
			own_mac = wdev->bss_info_argument.mld_info.mld_addr;
			peer_mac = pEntry->mlo.mld_addr;
			if (MAC_ADDR_EQUAL(peer_mac, ZERO_MAC_ADDR)) {
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
					"peer_mac invalid\n");
				goto assoc_check;
			}
		}
#endif
		pmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE, ie_list->RSNIE_Len, PMKID_LIST, &pmkid_count);
		if (pmkid != NULL) {
			CacheIdx = RTMPSearchPMKIDCache(PD_GET_PMKID_PTR(pAd->physical_dev),
				own_mac, peer_mac, FALSE, is_mlo_connect);
			if ((CacheIdx == -1) ||
			    ((RTMPEqualMemory(pmkid,
				PD_GET_PMKID_PTR(pAd->physical_dev)->BSSIDInfo[CacheIdx].PMKID,
				LEN_PMKID)) == 0)) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
						"AKM_OWE_Entry PMKID not found, do normal ECDH procedure\n");
				need_ecdh_ie = TRUE;
			}
		} else
			need_ecdh_ie = TRUE;

		if (need_ecdh_ie == TRUE) {
			FrameLen +=  build_owe_dh_ie(pAd,
						     pEntry,
						     (UCHAR *)pOutBuffer + FrameLen,
						     pEntry->SecConfig.owe.last_try_group);
		}
	}
#endif /*CONFIG_OWE_SUPPORT*/
#endif /* RT_CFG80211_SUPPORT */


#ifdef IGMP_TVM_SUPPORT
	/* ADD TV IE to this packet */
	MakeTVMIE(pAd, wdev, pOutBuffer, &FrameLen);
#endif /* IGMP_TVM_SUPPORT*/


#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	ap_vendor_ie = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].ap_vendor_ie;
	RTMP_SPIN_LOCK(&ap_vendor_ie->vendor_ie_lock);
	if (ap_vendor_ie->pointer != NULL) {
		ULONG TmpLen;

		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			"SYNC - Send Association response to "MACSTR"...and add vendor ie\n",
			MAC2STR(ie_list->Addr2));
		MakeOutgoingFrame(pOutBuffer + FrameLen,
				  &TmpLen,
				  ap_vendor_ie->length,
				  ap_vendor_ie->pointer,
				  END_OF_ARGS);
		FrameLen += TmpLen;
	}
	RTMP_SPIN_UNLOCK(&ap_vendor_ie->vendor_ie_lock);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

#ifdef OCE_FILS_SUPPORT

	if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC)) {
		if ((pEntry->filsInfo.auth_algo == AUTH_MODE_FILS) &&
			IS_AKM_FILS(wdev->SecConfig.AKMMap) &&
			IS_AKM_FILS(pEntry->SecConfig.AKMMap)) {
			struct fils_info *filsInfo = &pEntry->filsInfo;
			PFRAME_802_11 Fr = (PFRAME_802_11)Elem->Msg;

			if (!filsInfo->is_pending_assoc) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
						 "STA - "MACSTR" do FILS assoc with Pending\n",
						  MAC2STR(ie_list->Addr2));

				if (filsInfo->pending_ie) {
					os_free_mem(filsInfo->pending_ie);
					filsInfo->pending_ie_len = 0;
					filsInfo->pending_ie = NULL;
				}

				if (filsInfo->extra_ie) {
					os_free_mem(filsInfo->extra_ie);
					filsInfo->extra_ie_len = 0;
					filsInfo->extra_ie = NULL;
				}

				filsInfo->pending_ie_len = Elem->MsgLen;
				os_alloc_mem(NULL, (UCHAR **)&filsInfo->pending_ie, filsInfo->pending_ie_len);
				if (!filsInfo->pending_ie) {
					goto LabelOK;
				}
				NdisMoveMemory(filsInfo->pending_ie, Elem->Msg, filsInfo->pending_ie_len);

				filsInfo->extra_ie_len = FrameLen;
				os_alloc_mem(NULL, (UCHAR **)&filsInfo->extra_ie, filsInfo->extra_ie_len);
				if (!filsInfo->extra_ie) {
					goto LabelOK;
				}
				NdisMoveMemory(filsInfo->extra_ie, pOutBuffer, filsInfo->extra_ie_len);

				NdisMoveMemory(&filsInfo->rssi_info, &Elem->rssi_info, sizeof(struct raw_rssi_info));
				if (isReassoc)
					filsInfo->pending_action = ap_assoc_api.peer_reassoc_req_action;
				else
					filsInfo->pending_action = ap_assoc_api.peer_assoc_req_action;

				filsInfo->is_pending_assoc = TRUE;
				filsInfo->last_pending_id = Fr->Hdr.Sequence;

				DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_MLME_EVENT);
				goto free_check;
			} else {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
						 "STA - "MACSTR" skip FILS assoc in Pending\n",
						  MAC2STR(ie_list->Addr2));
				goto free_check;
			}
		}
	}
#endif /* OCE_FILS_SUPPORT */

#ifdef DOT11_EHT_BE
	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			"(be)TODO: cap.mode=0x%x, bHtEn=%d\n",
			pEntry->cap.modes, wdev->DesiredHtPhyInfo.bHtEnable);

	if (IS_EHT_STA(pEntry->cap.modes)
		&& WMODE_CAP_BE(wdev->PhyMode)
		&& wdev->DesiredHtPhyInfo.bHtEnable
		&& chip_determine_mlo_allow(pAd, &ie_list->cmm_ies.vendor_ie)) {
		/* this offset should be set to the position of EHT Cap IE */
		/* and be used later for ML IE reorder */
		offset_ml_ie = FrameLen;
		FrameLen += eht_add_assoc_rsp_ies(
				wdev, pOutBuffer, FrameLen, &ie_list->cmm_ies);
	}

	/* EHT MLO non-setup link association */
	if (Elem->Others) {
		assoc_info = (struct eht_assoc_req_priv *)Elem->Others;
		assoc_info->buf = pOutBuffer;
		assoc_info->buf_len = FrameLen;
		assoc_info->non_setup_link_status = StatusCode;
		setup_link_success = assoc_info->setup_link_success;
		/* In this case, pOutBuffer will be freed */
		/* in eht_ap_mlo_peer_non_setup_links_assoc_req() */
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
			"non-setup link assoc rsp: %p, len: %ld\n",
			assoc_info->buf, assoc_info->buf_len);
	} else
#endif /* DOT11_EHT_BE */
	{
		/* legacy & EHT MLO setup link association */
#ifdef DOT11_EHT_BE
		/*
		 * In 802.11be, we shall deduce the inheritance of IEs in STA Profile
		 * in the Basic Multi-link IE in management frames. First, we shall form
		 * the "complete set" of IEs (except ML IE) in the association response
		 * frame of the setup link, and therefore we can do non-setup link association
		 * and apply the inheritance rule to association response frames belonging
		 * to setup link and non-setup links. Consequently, we must put the query
		 * of ML IE at the last of the setup link association, and perform reorder
		 * of ML IE to form the correct association response frame of the setup link.
		 */
		if (IS_EHT_STA(pEntry->cap.modes)
			&& WMODE_CAP_BE(wdev->PhyMode)
			&& wdev->DesiredHtPhyInfo.bHtEnable
			&& chip_determine_mlo_allow(pAd, &ie_list->cmm_ies.vendor_ie)) {
			ULONG ml_ie_len = 0;
			UINT8 *ml_ie_end_pos = NULL;
			struct mtk_mac_bss *mac_bss;

			/* MLO: non-setup links association */
			if (HAS_EHT_MLD_EXIST(ie_list->cmm_ies.ie_exists) && pEntry->mlo.mlo_en) {
				struct buffer_wrapper setup_link_rsp;
				struct mld_entry_t *mld_entry;
				uint16_t mld_sta_idx;

				/* Basic variant Multi-Link element */
				mac_bss = (struct mtk_mac_bss *)wdev->pHObj;
				mt_rcu_read_lock();
				mld_entry = rcu_dereference(pEntry->mld_entry);
				if (!mld_entry) {
					mt_rcu_read_unlock();
					MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
						"mld_entry is NULL\n");
					goto assoc_check;
				}
				mld_sta_idx = mld_entry->mld_sta_idx;
				mt_rcu_read_unlock();

				if ((mac_bss->if_cfg.mld_group_idx != 0) &&
					(mac_bss->if_cfg.mld_group_idx != MLD_GROUP_NONE)) {

					setup_link_rsp.buf = pOutBuffer;
					setup_link_rsp.buf_len = FrameLen;

					eht_ap_mlo_peer_non_setup_links_assoc_req(Elem, pEntry,
						setup_link_Sst, StatusCode, &setup_link_rsp, &ie_list->cmm_ies, isReassoc);

					os_alloc_mem(pAd, (UCHAR **)&ml_ie_buf, MAX_LEN_OF_MLIE);
					if (!ml_ie_buf) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
							"Error: can not allocate memory for ml ie\n");
						goto LabelOK;
					}

					ml_ie_end_pos = eht_build_multi_link_ie(
										wdev, ml_ie_buf,
										HAS_EHT_MLD_EXIST(ie_list->cmm_ies.ie_exists) ?
											ie_list->cmm_ies.ml_ie : NULL,
										&ie_list->cmm_ies.ml_frag_info,
										BMGR_QUERY_ML_IE_ASSOC_RSP,
										mld_sta_idx);
					ml_ie_len = ml_ie_end_pos - ml_ie_buf;

					/* reorder Basic Variance Multi-Link element */
					if (ml_ie_len != 0) {
						NdisMoveMemory(pOutBuffer + offset_ml_ie + ml_ie_len,
							pOutBuffer + offset_ml_ie, FrameLen - offset_ml_ie);

						NdisMoveMemory(pOutBuffer + offset_ml_ie, ml_ie_buf, ml_ie_len);
						FrameLen += ml_ie_len;
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_INFO,
							"After Reorder ML IE, buf:%p, buf_len:%ld\n",
							pOutBuffer, FrameLen);
					}

					if (ml_ie_buf)
						os_free_mem(ml_ie_buf);
					ml_ie_buf = NULL;
				}
			}
		}
#endif /* DOT11_EHT_BE */

		/* add MTK MLO V1 IE */
		FrameLen += build_mtk_tlv1_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen),
			VIE_ASSOC_RESP, BIT(MEDIATEK_TLV1_TYPE4));
#ifdef MLR_SUPPORT
		if (pAd->CommonCfg.bMLREnable)
			FrameLen += build_mtk_tlv1_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen),
				VIE_ASSOC_RESP, BIT(MEDIATEK_TLV1_TYPE1));
#endif
		/* add Ralink-specific IE here -
		 * Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back */
		FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen), VIE_ASSOC_RESP);

		/* add WMM IE here */
		if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) {
			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
			FrameLen += build_wmm_cap_ie(pAd, &ie_info);
		}

		if (isReassoc)
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
				"send re-assoc resp to "MACSTR" (status=%d)...\n",
				MAC2STR(ie_list->Addr2), StatusCode);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
				"send assoc resp to "MACSTR" (status=%d)...\n",
				MAC2STR(ie_list->Addr2), StatusCode);

		os_alloc_mem(NULL, (UCHAR **)&assoc_resp_frame, FrameLen);
		if (!assoc_resp_frame)
			goto LabelOK;
		NdisZeroMemory(assoc_resp_frame, FrameLen);
		NdisMoveMemory(assoc_resp_frame, pOutBuffer, FrameLen);
		assoc_resp_frame_len = FrameLen;

		/* transmit the association response frame */
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
		MlmeFreeMemory((PVOID) pOutBuffer);
	}
	pOutBuffer = NULL;
#ifdef A4_CONN
	if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC))
		del_ppe_entry_by_roaming_detected(wdev, pEntry->Addr);
#endif
#ifdef OCE_FILS_SUPPORT
assoc_post:
#endif /* OCE_FILS_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT

	if (StatusCode == MLME_ASSOC_REJ_TEMPORARILY)
		PMF_MlmeSAQueryReq(pAd, pEntry);

#endif /* DOT11W_PMF_SUPPORT */

	/*is status is success ,update STARec*/
	if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC)
#ifdef DOT11_EHT_BE
		/* In MLO, the condition of successful association of non-setup link
		 * is also based on result of setup link association */
		&& (!pEntry->mlo.mlo_en || setup_link_success)
#endif /* DOT11_EHT_BE */
	) {
		/* legacy link or mlo setup link */
		if (wdev_do_conn_act(wdev, pEntry) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"connect action fail!!\n");
			goto assoc_check;
		}

#ifdef DOT11R_FT_SUPPORT
		if ((pFtCfg != NULL) && (pFtCfg->FtCapFlag.Dot11rFtEnable)) {
			struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;
			struct _SEC_KEY_INFO *pKey = NULL;

			if (FtInfoBuf->FtIeInfo.MICCtr.field.IECnt) {

#ifdef MBO_SUPPORT
					/* YF_FT */
				if (IS_MBO_ENABLE(wdev))
					/* update STA bssid & security info to daemon */
					MboIndicateStaBssidInfo(pAd, wdev, pEntry->Addr);
#endif /* MBO_SUPPORT */
#ifdef OCE_SUPPORT
		if (IS_OCE_ENABLE(wdev))
			/* update OCE STA info to daemon */
			OceIndicateStaInfo(pAd, wdev, pEntry->Addr);
#endif /* OCE_SUPPORT */

				pSecConfig->Handshake.GTKState = REKEY_ESTABLISHED;
				pSecConfig->Handshake.WpaState = AS_PTKINITDONE;
				pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

				os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
				if (info) {

					os_zero_mem(info, sizeof(ASIC_SEC_INFO));
					NdisCopyMemory(pSecConfig->PTK, pEntry->FT_PTK, LEN_MAX_PTK);
					info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
					info->Direction = SEC_ASIC_KEY_BOTH;
					info->Wcid = pEntry->wcid;
					info->BssIndex = pEntry->func_tb_idx;
					info->Cipher = pSecConfig->PairwiseCipher;
					info->KeyIdx = pSecConfig->PairwiseKeyId;
					os_move_mem(&info->PeerAddr[0],
									pEntry->Addr, MAC_ADDR_LEN);
					os_move_mem(info->Key.Key,
									&pEntry->FT_PTK[LEN_PTK_KCK + LEN_PTK_KEK],
									(LEN_TK + LEN_TK2));
					pKey = &info->Key;

					if (IS_CIPHER_TKIP(info->Cipher))
							pKey->KeyLen = LEN_TKIP_TK;
					else if (IS_CIPHER_CCMP128(info->Cipher))
							pKey->KeyLen = LEN_CCMP128_TK;
					else if (IS_CIPHER_CCMP256(info->Cipher))
							pKey->KeyLen = LEN_CCMP256_TK;
					else if (IS_CIPHER_GCMP128(info->Cipher))
							pKey->KeyLen = LEN_GCMP128_TK;
					else if (IS_CIPHER_GCMP256(info->Cipher))
							pKey->KeyLen = LEN_GCMP256_TK;

					if (IS_CIPHER_TKIP(info->Cipher)) {
							os_move_mem(pKey->TxMic, &pKey->Key[LEN_TK], LEN_TKIP_MIC);
							os_move_mem(pKey->RxMic, &pKey->Key[LEN_TK + 8], LEN_TKIP_MIC);
					}
					WifiSysUpdatePortSecur(pAd, pEntry, info);
				} else {
					WifiSysUpdatePortSecur(pAd, pEntry, NULL);
				}
			}
		}
#endif /* DOT11R_FT_SUPPORT */

#ifdef REDUCE_TCP_ACK_SUPPORT
		/* RACK is enabled only for 1ss sta*/
		if (IS_ENTRY_CLIENT(pEntry) &&
			pEntry->SupportVHTMCS1SS &&
			(pEntry->SupportVHTMCS2SS == 0) &&
			(pEntry->SupportVHTMCS3SS == 0) &&
			(pEntry->SupportVHTMCS4SS == 0) &&
			(pAd->MacTab->Size == 1))
			pEntry->RACKEnalbedSta = TRUE;
		else
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					 "Not RACK Enalbed Sta !!\n");
#endif
	}

	/* set up BA session */
	if (StatusCode == MLME_SUCCESS
#ifdef DOT11_EHT_BE
		/* In MLO, the condition of successful association of non-setup link
		 * is also based on result of setup link association */
		&& (!pEntry->mlo.mlo_en || setup_link_success)
#endif /* DOT11_EHT_BE */
	) {

#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT) && defined(CFG_SUPPORT_FALCON_SR)
		if (IS_MAP_ENABLE(pAd) && (pAd->CommonCfg.SRMode == 1))
			SrMeshSrUpdateSTAMode(pAd, TRUE, IS_HE_STA(pEntry->cap.modes));
#endif /* defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT) && defined(CFG_SUPPORT_FALCON_SR) */

	/* In WPA or 802.1x mode, the port is not secured, otherwise is secued. */
	if (!(IS_AKM_WPA_CAPABILITY_Entry(pEntry)
#ifdef DOT1X_SUPPORT
		|| IS_IEEE8021X_Entry(wdev)
#endif /* DOT1X_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
		|| wdev->IsCFG1xWdev
#endif /* RT_CFG80211_SUPPORT */
#ifdef WSC_INCLUDED
		|| (pEntry->bWscCapable && IS_AKM_WPA_CAPABILITY_Entry(wdev))
#endif /* WSC_INCLUDED */
	   ))
		tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

#ifdef WH_EVENT_NOTIFIER

	if (pEntry && tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
		EventHdlr pEventHdlrHook = NULL;

		pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_JOIN);

		if (pEventHdlrHook && pEntry->wdev)
			pEventHdlrHook(pAd, pEntry, Elem);
	}

#endif /* WH_EVENT_NOTIFIER */

		pEntry->PsMode = PWR_ACTIVE;
		tr_entry->PsMode = PWR_ACTIVE;
		MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CONNECTION_NOT_READY);
#ifdef IAPP_SUPPORT
		/*PFRAME_802_11 Fr = (PFRAME_802_11)Elem->Msg; */
		/*		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie; */
		{
			/* send association ok message to IAPPD */
			IAPP_L2_Update_Frame_Send(pAd, pEntry->Addr, wdev->wdev_idx);
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					 "####### Send L2 Frame Mac="MACSTR"\n",
					  MAC2STR(pEntry->Addr));
		}

		/*		SendSingalToDaemon(SIGUSR2, pObj->IappPid); */
#ifdef DOT11R_FT_SUPPORT
		{
			/*
				Do not do any check here.
				We need to send MOVE-Req frame to AP1 even open mode.
			*/
			/*		if (IS_FT_RSN_STA(pEntry) && (FtInfo.FtIeInfo.Len != 0)) */
			if (IS_FT_STA(pEntry)) {
				if (isReassoc == 1) {
					/* only for reassociation frame */
					FT_KDP_EVT_REASSOC EvtReAssoc;

					EvtReAssoc.SeqNum = 0;
					NdisMoveMemory(EvtReAssoc.MacAddr, pEntry->Addr, MAC_ADDR_LEN);
					NdisMoveMemory(EvtReAssoc.OldApMacAddr, ie_list->ApAddr, MAC_ADDR_LEN);
					FT_KDP_EVENT_INFORM(pAd, pEntry->func_tb_idx, FT_KDP_SIG_FT_REASSOCIATION,
										&EvtReAssoc, sizeof(EvtReAssoc), NULL);
				}
			}
		}

#endif /* DOT11R_FT_SUPPORT */
#endif /* IAPP_SUPPORT */
		/* ap_assoc_info_debugshow(pAd, isReassoc, pEntry, ie_list); */
		/* send wireless event - for association */
		RTMPSendWirelessEvent(pAd, IW_ASSOC_EVENT_FLAG, pEntry->Addr, 0, 0);
		/* This is a reassociation procedure */
		pEntry->IsReassocSta = isReassoc;
		ba_info = &pEntry->ba_info;

#ifdef DOT11_EHT_BE
		mt_rcu_read_lock();

		if (IS_ENTRY_MLO(pEntry) && pEntry->mlo.is_setup_link_entry) {
			struct mld_entry_t *mld_entry;

			mld_entry = rcu_dereference(pEntry->mld_entry);
			if (mld_entry)
				ba_info = &mld_entry->ba_info;
		}
#endif /* DOT11_EHT_BE */

		/* clear txBA bitmap */
		ba_info->TxBitmap = 0;

#ifdef DOT11_EHT_BE
		mt_rcu_read_unlock();
#endif

		if (pEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX) {
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

			if (WMODE_CAP_2G(wdev->PhyMode) &&
				addht->AddHtInfo.ExtChanOffset &&
				(ie_list->cmm_ies.ht_cap.HtCapInfo.ChannelWidth == BW_40))
				SendBeaconRequest(pAd, pEntry->wcid);

#ifdef DOT11_EHT_BE
			if (!IS_ENTRY_MLO(pEntry) || pEntry->mlo.is_setup_link_entry)
#endif
				ba_ori_session_start(pAd, tr_entry->wcid, 5);
		}

#ifdef DOT11R_FT_SUPPORT

		/*	If the length of FTIE field of the (re)association-request frame
			is larger than zero, it shall indicate the Fast-BSS transition is in progress. */
		if (ie_list->FtInfo.FtIeInfo.Len > 0)
			;
		else
#endif /* DOT11R_FT_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
#ifndef CFG80211_BYPASS
			if (TRUE) { /*CFG_TODO*/
				/* need to update pEntry to  inform later flow to keep ConnectionState in connected */
				pEntry->bWscCapable = ie_list->bWscCapable;
				{
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					"SINGLE CFG: NOITFY ASSOCIATED, pEntry->bWscCapable:%d\n",
					pEntry->bWscCapable);
#ifdef RT_CFG80211_SUPPORT
#ifdef DOT11_EHT_BE
				if (pEntry->mlo.mlo_en) {
					/* Non-setup link req ie has no MLE ie */
					if (pEntry->mlo.is_setup_link_entry)
						CFG80211OS_NewSta(wdev->if_dev,
							  ie_list->Addr2,
							  (PUCHAR)Elem->Msg,
							  Elem->MsgLen, isReassoc,
							  assoc_resp_frame,
							  assoc_resp_frame_len,
							  TRUE,
							  pEntry->mlo.link_info.link_id,
							  pEntry->mlo.mld_addr);
					} else
#endif
						CFG80211OS_NewSta(wdev->if_dev,
							  ie_list->Addr2,
							  (PUCHAR)Elem->Msg,
							  Elem->MsgLen, isReassoc,
							  assoc_resp_frame,
							  assoc_resp_frame_len,
							  FALSE,
							  0,
							  NULL);
#endif
					if (IS_CIPHER_WEP(pEntry->SecConfig.PairwiseCipher)) {
						ASIC_SEC_INFO *Info;

						os_alloc_mem(pAd, (UCHAR **)&Info, sizeof(ASIC_SEC_INFO));
					if (Info == NULL) {
						MTWF_DBG(pAd,
							DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
							"allocate memory for Info failed!\n");
						goto assoc_check;
					}
						os_zero_mem(Info, sizeof(ASIC_SEC_INFO));

						/* Set key material to Asic */
						Info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
						Info->Direction = SEC_ASIC_KEY_BOTH;
						Info->Wcid = pEntry->wcid;
						Info->BssIndex = pEntry->func_tb_idx;
						Info->KeyIdx = pEntry->SecConfig.PairwiseKeyId;
						Info->Cipher = pEntry->SecConfig.PairwiseCipher;
						Info->KeyIdx = pEntry->SecConfig.PairwiseKeyId;
						os_move_mem(&Info->Key,
							    &pEntry->SecConfig.WepKey[pEntry->SecConfig.PairwiseKeyId],
							    sizeof(SEC_KEY_INFO));
						os_move_mem(&Info->PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
						HW_ADDREMOVE_KEYTABLE(pAd, Info);

						os_free_mem(Info);
					}
				}

				hex_dump("ASSOC_REQ", Elem->Msg, Elem->MsgLen);
			} else
#endif /* CFG80211_BYPASS */
#endif

			/* enqueue a EAPOL_START message to trigger EAP state machine doing the authentication */
			if (IS_AKM_PSK_Entry(pEntry)) {
				INT cacheidx;
				UCHAR *peer_mac = pEntry->Addr;
				UCHAR is_mlo_connect = FALSE;
				UCHAR *own_mac = wdev->bssid;

#ifdef DOT11_EHT_BE
				if (pEntry->mlo.mlo_en) {
					is_mlo_connect = TRUE;
					own_mac = wdev->bss_info_argument.mld_info.mld_addr;
					peer_mac = pEntry->mlo.mld_addr;
					if (MAC_ADDR_EQUAL(peer_mac, ZERO_MAC_ADDR)) {
						MTWF_DBG(pAd,
							DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
							"peer_mac invalid\n");
						goto assoc_check;
					}
				}
#endif


				if (is_rsne_pmkid_cache_match(ie_list->RSN_IE,
							      ie_list->RSNIE_Len,
							      PD_GET_PMKID_PTR(pAd->physical_dev),
							      own_mac,
							      peer_mac,
							      is_mlo_connect,
							      &cacheidx)) {
					store_pmkid_cache_in_sec_config(pAd, pEntry, cacheidx);
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
						 "ASSOC - CacheIdx = %d\n",
						  cacheidx);
				}

#ifdef WSC_AP_SUPPORT

				/*
				 * In WPA-PSK mode,
				 * If Association Request of station has RSN/SSN,
				 * WPS AP Must Not send EAP-Request/Identity to station
				 * no matter WPS AP does receive EAPoL-Start from STA or not.
				 * Marvell WPS test bed(v2.1.1.5) will send AssocReq with WPS IE and RSN/SSN IE.
				 */
				if (pEntry->bWscCapable || (ie_list->RSNIE_Len == 0)) {
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
							 "ASSOC - IF(ra%d) This is a WPS Client.\n\n",
							  pEntry->func_tb_idx);
					goto LabelOK;
				} else {
					pEntry->bWscCapable = FALSE;
					pEntry->Receive_EapolStart_EapRspId = (WSC_ENTRY_GET_EAPOL_START |
									       WSC_ENTRY_GET_EAP_RSP_ID);
					/* This STA is not a WPS STA */
					NdisZeroMemory(wsc_ctrl->EntryAddr, 6);
				}

#endif /* WSC_AP_SUPPORT */

				/* Enqueue a EAPOL-start message with the pEntry for WPAPSK State Machine */
				if (1
#ifdef WSC_AP_SUPPORT
				    && !pEntry->bWscCapable
#endif /* WSC_AP_SUPPORT */
				   ) {
					/* Enqueue a EAPOL-start message with the pEntry */
#ifdef DOT11_EHT_BE
					if (pEntry->mlo.mlo_en) {
						os_move_mem(&pEntry->SecConfig.Handshake.AAddr,
							wdev->bss_info_argument.mld_info.mld_addr, MAC_ADDR_LEN);
						if (!MAC_ADDR_EQUAL(pEntry->mlo.mld_addr, ZERO_MAC_ADDR))
							os_move_mem(&pEntry->SecConfig.Handshake.SAddr,
								pEntry->mlo.mld_addr, MAC_ADDR_LEN);
						else {
							MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
								"mld_addr invalid\n");
							goto assoc_check;
						}
					} else
#endif
					{
						os_move_mem(&pEntry->SecConfig.Handshake.AAddr, wdev->bssid, MAC_ADDR_LEN);
						os_move_mem(&pEntry->SecConfig.Handshake.SAddr, pEntry->Addr, MAC_ADDR_LEN);
					}

					if (!IS_AKM_WPA3PSK(pEntry->SecConfig.AKMMap) &&
#ifdef DPP_SUPPORT
						!(IS_AKM_DPP(pEntry->SecConfig.AKMMap)) &&
#endif /* DPP_SUPPORT */
					    !(IS_AKM_OWE(pEntry->SecConfig.AKMMap)))
						os_move_mem(&pEntry->SecConfig.PMK, &wdev->SecConfig.PMK, LEN_PMK);

#ifndef RT_CFG80211_SUPPORT
					RTMPSetTimer(&pEntry->SecConfig.StartFor4WayTimer, ENQUEUE_EAPOL_START_TIMER);
#endif /*RT_CFG80211_SUPPORT*/
				}
			}

#ifdef DOT1X_SUPPORT
			else if (IS_AKM_WPA2_Entry(pEntry) ||
				 IS_AKM_WPA3_192BIT_Entry(pEntry)) {
				INT	cacheidx;
				UCHAR *peer_mac = pEntry->Addr;
				UCHAR is_mlo_connect = FALSE;
				UCHAR *own_mac = wdev->bssid;

#ifdef DOT11_EHT_BE
				if (pEntry->mlo.mlo_en) {
					is_mlo_connect = TRUE;
					own_mac = wdev->bss_info_argument.mld_info.mld_addr;
					peer_mac = pEntry->mlo.mld_addr;
					if (MAC_ADDR_EQUAL(peer_mac, ZERO_MAC_ADDR)) {
						MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
							"peer_mac invalid\n");
						goto assoc_check;
					}
				}
#endif

				if (is_rsne_pmkid_cache_match(ie_list->RSN_IE,
							      ie_list->RSNIE_Len,
							      PD_GET_PMKID_PTR(pAd->physical_dev),
							      own_mac,
							      peer_mac,
							      is_mlo_connect,
							      &cacheidx))
					process_pmkid(pAd, wdev, pEntry, cacheidx);
			} else if (IS_AKM_1X_Entry(pEntry) ||
				  (IS_IEEE8021X(&pEntry->SecConfig)
#ifdef WSC_AP_SUPPORT
				   && (!pEntry->bWscCapable)
#endif /* WSC_AP_SUPPORT */
				   )) {
				/* Enqueue a EAPOL-start message to trigger EAP SM */
				if (pEntry->EnqueueEapolStartTimerRunning == EAPOL_START_DISABLE
				   ) {
					pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_1X;
#ifndef RT_CFG80211_SUPPORT
					RTMPSetTimer(&pEntry->SecConfig.StartFor4WayTimer, ENQUEUE_EAPOL_START_TIMER);
#endif /*RT_CFG80211_SUPPORT*/
				}
			}

#endif /* DOT1X_SUPPORT */
#if defined(MWDS) || defined(CONFIG_BS_SUPPORT) || defined(CONFIG_MAP_SUPPORT) || defined(WAPP_SUPPORT) || defined(QOS_R1)
#ifdef WAPP_SUPPORT
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
		if (!pEntry->mlo.mlo_en)
#endif
			wapp_send_cli_join_event(pAd, pEntry);
#endif
		if (tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
#ifdef MWDS
			MWDSAPPeerEnable(pAd, pEntry);
#endif
#ifdef CONFIG_MAP_SUPPORT
			if (IS_MAP_ENABLE(pAd)) {
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
				if (pEntry->mlo.mlo_en)
					map_a4_mlo_peer_enable(pAd, NULL, pEntry, TRUE);
				else
#endif
					map_a4_peer_enable(pAd, pEntry, TRUE);
			}
#endif /* CONFIG_MAP_SUPPORT */
#ifdef QOS_R1
#ifdef MSCS_PROPRIETARY
#ifdef DOT11_EHT_BE
			if ((pEntry->mlo.mlo_en && (pEntry->mlo.is_setup_link_entry)) ||
				(!pEntry->mlo.mlo_en))
#endif
			{
				if (IS_AKM_OPEN(wdev->SecConfig.AKMMap) && IS_QOS_ENABLE(pAd) &&
						pAd->SupportFastPath && pEntry->MSCSSupport) {
					pEntry->dabs_trans_id = 1;
					Send_DABS_Announce(pAd, pEntry->wcid);
					RTMPSetTimer(&pEntry->DABSRetryTimer, DABS_WAIT_TIME);
					OS_SET_BIT(DABS_TIMER_RUNNING, &pEntry->DABSTimerFlag);
				}
			}
#endif
#endif
		}
#endif

#ifdef GREENAP_SUPPORT
		if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC))
			greenap_check_peer_connection_at_link_up_down(pAd, wdev);
#endif /* GREENAP_SUPPORT */

#ifdef CONFIG_HOTSPOT_R2
		pHSCtrl = &pAd->ApCfg.MBSSID[pEntry->apidx].HotSpotCtrl;

		/* add to cr4 pool */
		if (pEntry->QosMapSupport && pHSCtrl->HotSpotEnable) {
			if (pHSCtrl->QosMapEnable) {
				if (!pHSCtrl->QosMapAddToPool) {
					pHSCtrl->QosMapAddToPool = TRUE;
					pHSCtrl->QosMapPoolID = hotspot_qosmap_add_pool(pAd, pEntry);
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
						"add current MBSS qosmap to CR4\n");
				}

				hotspot_qosmap_update_sta_mapping_to_cr4(pAd, pEntry, pHSCtrl->QosMapPoolID);
			}
		}

#endif /* CONFIG_HOTSPOT_R2 */

#ifdef MBSS_AS_WDS_AP_SUPPORT
	if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) {
		if (IS_ENTRY_CLIENT(pEntry)) {
			pEntry->bEnable4Addr = TRUE;
			if (wdev->wds_enable) {
#ifdef WIFI_UNIFIED_COMMAND
				if (wifi_sys_header_trans_update(pAd, pEntry) != NDIS_STATUS_SUCCESS)
					MTWF_PRINT("WDS AP Address Mode failed to set for wcid %d\n", pEntry->wcid);
#else
				HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pEntry->wcid, TRUE);
#endif
			}

			else if (MAC_ADDR_EQUAL(pAd->ApCfg.wds_mac, pEntry->Addr)) {
#ifdef WIFI_UNIFIED_COMMAND
				if (wifi_sys_header_trans_update(pAd, pEntry) != NDIS_STATUS_SUCCESS)
					MTWF_PRINT("WDS AP Address Mode failed to set for wcid %d\n", pEntry->wcid);
#else


				HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pEntry->wcid, TRUE);
#endif
				}

			}
		}
#endif

	}

#ifdef BAND_STEERING
		if ((pAd->ApCfg.BandSteering))
			BndStrg_UpdateEntry(pAd, pEntry, ie_list, TRUE);
#endif


LabelOK:
#ifdef RT_CFG80211_SUPPORT

	if (pEntry && (StatusCode != MLME_SUCCESS) && (StatusCode != MLME_ASSOC_REJ_TEMPORARILY) && (StatusCode != MLME_INVALID_PMKID))
		CFG80211_ApStaDelSendEvent(pAd, pEntry->Addr, wdev->if_dev);

#endif /* RT_CFG80211_SUPPORT */

assoc_check:
#ifdef WAPP_SUPPORT
#ifdef EM_PLUS_SUPPORT
	StatusCode = REASON_QOS_LACK_BANDWIDTH;
#endif /* EM_PLUS_SUPPORT */
	if (StatusCode != MLME_SUCCESS && wapp_assoc_fail != NOT_FAILURE)
		wapp_send_sta_connect_rejected(pAd, wdev, ie_list->Addr2,
			ie_list->Addr1, wapp_cnnct_stage, wapp_assoc_fail, StatusCode, 0);
#endif /* WAPP_SUPPORT */

#ifdef OCE_FILS_SUPPORT
free_check:
#endif /* OCE_FILS_SUPPORT */

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	/* fix memory leak when trigger scan continuously*/
	if (ie_list && ie_list->CustomerVendorIE.pointer)
		os_free_mem(ie_list->CustomerVendorIE.pointer);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

	if (ie_list != NULL)
		os_free_mem(ie_list);
#ifdef DOT11R_FT_SUPPORT
	if (rsnxe_ie != NULL)
		os_free_mem(rsnxe_ie);

	if (FtInfoBuf != NULL)
		os_free_mem(FtInfoBuf);
#endif
#ifdef DOT11_EHT_BE
	if (ml_ie_buf != NULL)
		os_free_mem(ml_ie_buf);
#endif /* DOT11_EHT_BE */
	if (pOutBuffer != NULL)
		MlmeFreeMemory((PVOID) pOutBuffer);

	if (assoc_resp_frame)
		os_free_mem(assoc_resp_frame);

	return;
}



/*
    ==========================================================================
    Description:
	peer assoc req handling procedure
    Parameters:
	Adapter - Adapter pointer
	Elem - MLME Queue Element
    Pre:
	the station has been authenticated and the following information is stored
    Post  :
	-# An association response frame is generated and sent to the air
    ==========================================================================
 */
static VOID ap_peer_assoc_req_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	ap_cmm_peer_assoc_req_action(pAd, Elem, 0);
}

/*
    ==========================================================================
    Description:
	mlme reassoc req handling procedure
    Parameters:
	Elem -
    Pre:
	-# SSID  (Adapter->ApCfg.ssid[])
	-# BSSID (AP address, Adapter->ApCfg.bssid)
	-# Supported rates (Adapter->ApCfg.supported_rates[])
	-# Supported rates length (Adapter->ApCfg.supported_rates_len)
	-# Tx power (Adapter->ApCfg.tx_power)
    ==========================================================================
 */
static VOID ap_peer_reassoc_req_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	ap_cmm_peer_assoc_req_action(pAd, Elem, 1);
}

/*
    ==========================================================================
    Description:
	left part of IEEE 802.11/1999 p.374
    Parameters:
	Elem - MLME message containing the received frame
    ==========================================================================
 */
static VOID ap_peer_disassoc_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
	USHORT Reason;
	UINT16 SeqNum;
	MAC_TABLE_ENTRY *pEntry;
	struct wifi_dev *wdev;

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
		"ASSOC - 1 receive DIS-ASSOC request\n");

	if (!PeerDisassocReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr1, Addr2, &SeqNum, &Reason))
		return;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
			 "ASSOC - receive DIS-ASSOC(seq-%d) request from "MACSTR", reason=%d\n",
			  SeqNum, MAC2STR(Addr2), Reason);
	pEntry = MacTableLookup(pAd, Addr2);

	if (pEntry == NULL)
		return;

#ifdef DOT11W_PMF_SUPPORT
	{
		PMF_CFG *pPmfCfg = NULL;
		FRAME_802_11 *Fr = (PFRAME_802_11)Elem->Msg;

		pPmfCfg = &pEntry->SecConfig.PmfCfg;

		if (pPmfCfg->UsePMFConnect == TRUE && Fr->Hdr.FC.Wep == 0) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO,
				"drop due to unprotect disassoc frame\n");
			return;
		}
	}
#endif

	if (VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid)) {
		wdev = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev;

		/*
			iPhone sometimes sends disassoc frame which DA is old AP and BSSID is new AP.
			@2016/1/26
		*/
		if (!MAC_ADDR_EQUAL(wdev->if_addr, Addr1)) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					 "ASSOC - The DA of this DIS-ASSOC request is "MACSTR", ignore.\n",
					  MAC2STR(Addr1));
			return;
		}

#if defined(DOT1X_SUPPORT) && !defined(RADIUS_ACCOUNTING_SUPPORT)

		/* Notify 802.1x daemon to clear this sta info */
		if (IS_AKM_1X_Entry(pEntry) || IS_IEEE8021X_Entry(wdev))
			DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

#endif /* DOT1X_SUPPORT */
		/* send wireless event - for disassociation */
		RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, Addr2, 0, 0);
		ApLogEvent(pAd, Addr2, EVENT_DISASSOCIATED);
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_DEAUTH, REASON_DEAUTH_STA_LEAVING);
#endif
#ifdef CONN_FAIL_EVENT
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				Reason);
#endif

		if (IS_ENTRY_CLIENT(pEntry)) {
#ifdef MAP_R2
			if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
				pEntry->DisconnectReason = Reason;
		/*	// TODO: if the port secured is not true, then send failed assoc.*/
#endif
		}
#ifdef CUSTOMISED_MGMT_TRANS_SUPPORT
		pEntry->del_reason = MTK_NL80211_VENDOR_DISC_STA_LEAVE_DISASSOC;
#endif
		MacTableDeleteEntry(pAd, Elem->Wcid, Addr2);

#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT) && defined(CFG_SUPPORT_FALCON_SR)
		if (IS_MAP_ENABLE(pAd) && (pAd->CommonCfg.SRMode == 1))
			SrMeshSrUpdateSTAMode(pAd, FALSE, FALSE);
#endif /* defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT) && defined(CFG_SUPPORT_FALCON_SR) */

#ifdef WH_EVENT_NOTIFIER
		{
			EventHdlr pEventHdlrHook = NULL;

			pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_LEAVE);

			if (pEventHdlrHook && wdev)
				pEventHdlrHook(pAd, wdev, Addr2, Elem->Channel);
		}
#endif /* WH_EVENT_NOTIFIER */
	}
}

VOID ap_mlme_broadcast_disassoc_req_action(
		IN PRTMP_ADAPTER pAd,
		IN MLME_QUEUE_ELEM * Elem,
		struct wifi_dev *wdev)
{
	MLME_DISCONNECT_STRUCT	*pInfo;
	HEADER_802_11			Hdr;
	PUCHAR					pOutBuffer = NULL;
	NDIS_STATUS				NStatus;
	ULONG					FrameLen = 0;
	MAC_TABLE_ENTRY			*pEntry;
	UCHAR					apidx = 0;
	UINT16 wcid;

	pInfo = (PMLME_DISCONNECT_STRUCT)Elem->Msg;
	if (!MAC_ADDR_EQUAL(pInfo->addr, BROADCAST_ADDR))
		return;
	apidx = wdev->func_idx;
	for (wcid = 1; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		pEntry = entry_get(pAd, wcid);
		if (pEntry->wdev != wdev)
			continue;
		RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG,
			pEntry->Addr, 0, 0);
		ApLogEvent(pAd, pInfo->addr, EVENT_DISASSOCIATED);
#ifdef MAP_R2
		if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
			wapp_handle_sta_disassoc(pAd, wcid, pInfo->reason);
#endif
		MacTableDeleteEntry(pAd, wcid, pEntry->Addr);
	}
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS)
		return;
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
			"send broadcast dis-assoc req, reason=%d\n", pInfo->reason);
	MgtMacHeaderInit(pAd, &Hdr, SUBTYPE_DISASSOC, 0, pInfo->addr,
					pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
					pAd->ApCfg.MBSSID[apidx].wdev.bssid);
	MakeOutgoingFrame(pOutBuffer,				&FrameLen,
					  sizeof(HEADER_802_11), &Hdr,
					  2, &pInfo->reason,
					  END_OF_ARGS);
#ifdef DOT11W_PMF_SUPPORT
	/* Add BIP MIC if PMF is enabled */
	if (!PMF_AddBIPMIC(pAd, wdev, (pOutBuffer+FrameLen), &FrameLen)) {
		MlmeFreeMemory(pOutBuffer);
		return;
	}
#endif /* DOT11W_PMF_SUPPORT */
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
	MlmeFreeMemory(pOutBuffer);
}

/*
    ==========================================================================
    Description:
	Upper layer orders to disassoc s STA
    Parameters:
	Elem -
    ==========================================================================
 */
static VOID ap_mlme_disassoc_req_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	MLME_DISCONNECT_STRUCT *DisassocReq = (MLME_DISCONNECT_STRUCT *)(Elem->Msg); /* snowpin for cntl mgmt */
	struct wifi_dev *wdev = Elem->wdev;
	MAC_TABLE_ENTRY *pEntry;

	pEntry = MacTableLookup(pAd, DisassocReq->addr);

	if (IS_VALID_ENTRY(pEntry) && IS_ENTRY_CLIENT(pEntry)) {
		APMlmeKickOutSta(pAd, DisassocReq->addr, pEntry->wcid, DisassocReq->reason);
	} else if (MAC_ADDR_EQUAL(BROADCAST_ADDR, DisassocReq->addr)) {
		ap_mlme_broadcast_disassoc_req_action(pAd, Elem, wdev);
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
			"%s, MAC addr invalid!\n", __func__);
	}
}


/*
    ==========================================================================
    Description:
	association state machine init, including state transition and timer init
    Parameters:
	S - pointer to the association state machine
    Note:
	The state machine looks like the following

				    AP_ASSOC_IDLE
	APMT2_MLME_DISASSOC_REQ    mlme_disassoc_req_action
	APMT2_PEER_DISASSOC_REQ    peer_disassoc_action
	APMT2_PEER_ASSOC_REQ       drop
	APMT2_PEER_REASSOC_REQ     drop
	APMT2_CLS3ERR              cls3err_action
    ==========================================================================
 */
VOID ap_assoc_init(struct wifi_dev *wdev)
{
	ap_assoc_api.peer_assoc_req_action = ap_peer_assoc_req_action;
	ap_assoc_api.peer_reassoc_req_action = ap_peer_reassoc_req_action;
	ap_assoc_api.mlme_disassoc_req_action = ap_mlme_disassoc_req_action;
	ap_assoc_api.peer_disassoc_action =     ap_peer_disassoc_action;
	wdev->assoc_api = &ap_assoc_api;
}

#ifdef BW_VENDOR10_CUSTOM_FEATURE
BOOLEAN IsClientConnected(RTMP_ADAPTER *pAd)
{
	INT i;
	PMAC_TABLE_ENTRY pEntry;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = entry_get(pAd, i);

		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			return TRUE;
	}

	return FALSE;
}
#endif

/*
    ==========================================================================
    Description:
	delete it from STA and disassoc s STA
    Parameters:
	Elem -
    ==========================================================================
 */
VOID MbssKickOutStas(RTMP_ADAPTER *pAd, INT apidx, USHORT Reason)
{
	INT i;
	PMAC_TABLE_ENTRY pEntry;
	struct wifi_dev *wdev = NULL;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#ifdef MBO_SUPPORT
	if (IS_MBO_ENABLE(wdev))
		MboIndicateStaDisassocToDaemon(pAd, NULL, MBO_MSG_AP_TERMINATION);
#endif /* MBO_SUPPORT */

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = entry_get(pAd, i);

		if (pEntry && IS_ENTRY_CLIENT(pEntry) && pEntry->wdev == wdev)
			APMlmeKickOutSta(pAd, pEntry->Addr, pEntry->wcid, Reason);
	}

	/* mbo wait for max 500ms and check all IsKeep is zero */
#ifdef MBO_SUPPORT
	if (IS_MBO_ENABLE(wdev))
		MboWaitAllStaGone(pAd, apidx);
#endif /* MBO_SUPPORT */

}


/*
    ==========================================================================
    Description:
	delete it from STA and disassoc s STA
    Parameters:
	Elem -
    ==========================================================================
 */
VOID APMlmeKickOutSta(RTMP_ADAPTER *pAd, UCHAR *pStaAddr, UINT16 Wcid, USHORT Reason)
{
	HEADER_802_11 DisassocHdr;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	NDIS_STATUS NStatus;
	MAC_TABLE_ENTRY *pEntry;
	UINT16 Aid;
	UCHAR ApIdx;

	pEntry = MacTableLookup(pAd, pStaAddr);

	if (pEntry == NULL)
		return;

	Aid = pEntry->Aid;
	ApIdx = pEntry->func_tb_idx;

	if (ApIdx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR, "Invalid Apidx=%d\n",
				 ApIdx);
		return;
	}

	if (VALID_UCAST_ENTRY_WCID(pAd, Wcid)) {
		/* send wireless event - for disassocation */
		RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, pStaAddr, 0, 0);
		ApLogEvent(pAd, pStaAddr, EVENT_DISASSOCIATED);
		/* 2. send out a DISASSOC request frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

#ifdef MAP_R2
		if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
			wapp_handle_sta_disassoc(pAd, Wcid, Reason);
#endif

		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
			"ASSOC - MLME disassociates "MACSTR", reason=%d in dis-assoc req\n",
			MAC2STR(pStaAddr), Reason);
		MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pStaAddr,
						 pAd->ApCfg.MBSSID[ApIdx].wdev.if_addr,
						 pAd->ApCfg.MBSSID[ApIdx].wdev.bssid);
		MakeOutgoingFrame(pOutBuffer,            &FrameLen,
						  sizeof(HEADER_802_11), &DisassocHdr,
						  2,                     &Reason,
						  END_OF_ARGS);
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
		MlmeFreeMemory(pOutBuffer);
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr, DIAG_CONN_DEAUTH, Reason);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				Reason);
#endif
		MacTableDeleteEntry(pAd, Wcid, pStaAddr);

#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT) && defined(CFG_SUPPORT_FALCON_SR)
		if (IS_MAP_ENABLE(pAd) && (pAd->CommonCfg.SRMode == 1))
			SrMeshSrUpdateSTAMode(pAd, FALSE, FALSE);
#endif /* defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT) && defined(CFG_SUPPORT_FALCON_SR) */
	}
}


#ifdef DOT11W_PMF_SUPPORT
VOID APMlmeKickOutAllSta(RTMP_ADAPTER *pAd, UCHAR apidx, USHORT Reason)
{
	HEADER_802_11 DisassocHdr;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	NDIS_STATUS     NStatus;
	UCHAR           BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	struct wifi_dev *wdev = NULL;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if ((apidx < pAd->ApCfg.BssidNum) && wdev) {
		/* Send out a Deauthentication request frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO, "Send DISASSOC Broadcast frame(%d) with ra%d\n", Reason, apidx);
		/* 802.11 Header */
		NdisZeroMemory(&DisassocHdr, sizeof(HEADER_802_11));
		DisassocHdr.FC.Type = FC_TYPE_MGMT;
		DisassocHdr.FC.SubType = SUBTYPE_DISASSOC;
		DisassocHdr.FC.ToDs = 0;
		DisassocHdr.FC.Wep = 0;
		COPY_MAC_ADDR(DisassocHdr.Addr1, BROADCAST_ADDR);
		COPY_MAC_ADDR(DisassocHdr.Addr2, pAd->ApCfg.MBSSID[apidx].wdev.bssid);
		COPY_MAC_ADDR(DisassocHdr.Addr3, pAd->ApCfg.MBSSID[apidx].wdev.bssid);
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(HEADER_802_11), &DisassocHdr,
						  2, &Reason,
						  END_OF_ARGS);

		/* Add BIP MIC if PMF is enabled */
		if (!PMF_AddBIPMIC(pAd, wdev, (pOutBuffer+FrameLen), &FrameLen)) {
			MlmeFreeMemory(pOutBuffer);
			return;
		}

		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
		MlmeFreeMemory(pOutBuffer);
	}
}
#endif /* DOT11W_PMF_PLUGFEST */

#ifdef DOT11_EHT_BE
/*
	Description:
		convert association request frame of setup link into
		temporary association request frame for non-setup link
*/
static NDIS_STATUS eht_ap_mlo_preprocess_non_setup_link_assoc_req(
	IN OUT struct _MLME_QUEUE_ELEM *Elem,
	IN struct ml_ie_link *ml_link,
	IN MAC_TABLE_ENTRY *pEntry,
	IN BOOLEAN isReassoc)
{
	UINT8 *source_buf = NULL, *target_buf = NULL;
	ULONG source_len = 0, target_len = 0, shift = 0;
	USHORT peer_listen_inverval, peer_cap_info;

	UINT8 reassoc_addr[MAC_ADDR_LEN], source_ssid_ie[35];
	UINT8 non_setup_link_id, source_ssid_len;
	UINT8 ie_list[100] = {0}, ie_ext_list[100] = {0};
	UINT8 ie_list_len = 0, ie_ext_list_len = 0;

	UINT8 *sta_profile = ml_link->sta_profile;
	UINT16 sta_profile_len = ml_link->sta_profile_len;
	UINT8 *sta_profile_ie_head = NULL;
	UINT16 sta_profile_ie_len = 0, final_sta_profile_ie_len = 0;

	struct wifi_dev *wdev = pEntry->wdev;
	PRTMP_ADAPTER pAd = wdev->sys_handle;
	PFRAME_802_11 frame = NULL;
	HEADER_802_11 assoc_req_hdr;
	struct _EID_STRUCT *frame_eid = NULL;
	struct _EID_STRUCT *sta_pf_eid = NULL;
	NDIS_STATUS status;

	struct buffer_wrapper source, target, id_buf, id_ext_buf;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
		"Start pre-processing non setup link assoc req\n");

	non_setup_link_id = pEntry->mlo.link_info.link_id;

	source_buf = Elem->Msg;
	source_len = Elem->MsgLen;
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
		"Source Buffer (setup assoc req):%p, len=%ld\n", source_buf, source_len);

	/* offset for fixed field of (re)assoc request frames */
	shift = sizeof(HEADER_802_11) + sizeof(peer_listen_inverval) + sizeof(peer_cap_info);
	if (isReassoc)
		shift += MAC_ADDR_LEN;

	if ((source_len < shift) ||
		((source_buf + Elem->MsgLen) < (source_buf + shift))) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
			"Error: input frame is invalid\n");
		return NDIS_STATUS_FAILURE;
	}

	frame = (PFRAME_802_11)Elem->Msg; /* only use for frame header/field parsing */

	target_len = 0;
	status = MlmeAllocateMemory(pAd, &target_buf);

	if (status != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	/* Step 1. fetch some fixed fields and SSID */

	/* copy Listen Interval and SSID IE from setup link assoc request frame */
	NdisMoveMemory(&peer_listen_inverval, &frame->Octet[2], 2);

	if (isReassoc) {
		COPY_MAC_ADDR(reassoc_addr, &frame->Octet[4]);
		frame_eid = (PEID_STRUCT)&frame->Octet[10];
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_NOTICE,
			"Notice: Reassociation request\n");
	} else
		frame_eid = (PEID_STRUCT)&frame->Octet[4];

	source_ssid_len = frame_eid->Len + 2;
	if (source_ssid_len <= ARRAY_SIZE(source_ssid_ie))
		NdisMoveMemory(source_ssid_ie, frame_eid, source_ssid_len);

	if (sta_profile_len == 0) {
		/* TODO: added for handle Per-STA Profile w/o STA Profile field,
		 * use the cap info of setup link */
		NdisMoveMemory(&peer_cap_info, &frame->Octet[0], 2);
		sta_profile_ie_head = sta_profile;
		sta_profile_ie_len = final_sta_profile_ie_len = 0;
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
			"Notice: No STA Profile in Per-STA Profile\n");
	} else {
		/* copy Capability Information from STA Profile */
		NdisMoveMemory(&peer_cap_info, sta_profile, sizeof(peer_cap_info));
		sta_profile_ie_head = sta_profile + sizeof(peer_cap_info);
		sta_profile_ie_len = final_sta_profile_ie_len = sta_profile_len - sizeof(peer_cap_info);
	}

	/* Step 2. prepare non-setup link assoc/reassoc req frame header/fixed fields */
	if (isReassoc) {
		MgtMacHeaderInitExt(pAd, &assoc_req_hdr, SUBTYPE_REASSOC_REQ, 0,
			wdev->if_addr, pEntry->Addr, wdev->bssid);

		MakeOutgoingFrame(target_buf, &target_len,
						  sizeof(HEADER_802_11),	&assoc_req_hdr,
						  2,                        &peer_cap_info,
						  2,						&peer_listen_inverval,
						  MAC_ADDR_LEN,				reassoc_addr,
						  source_ssid_len,			source_ssid_ie,
						  END_OF_ARGS);
	} else {
		MgtMacHeaderInitExt(pAd, &assoc_req_hdr, SUBTYPE_ASSOC_REQ, 0,
			wdev->if_addr, pEntry->Addr, wdev->bssid);

		MakeOutgoingFrame(target_buf, &target_len,
						  sizeof(HEADER_802_11),	&assoc_req_hdr,
						  2,                        &peer_cap_info,
						  2,						&peer_listen_inverval,
						  source_ssid_len,			source_ssid_ie,
						  END_OF_ARGS);
	}

	// hex_dump_always("Setup link header w/ SSID:", (UCHAR *)source_buf,
	//	sizeof(HEADER_802_11) + frame_eid->Len + 6);
	// hex_dump_always("Non-setup link header w/ SSID:", (UCHAR *)target_buf,
	//	sizeof(HEADER_802_11) + 4 + source_ssid_len);


	/* Step 3. Removed IE that also appeared in STA Profile in assoc req frame */
	/* the IDs listed in ie_list will be removed from setup link assoc req */
	ie_list[0] = IE_SSID;			/* SSID IE */
	ie_list[1] = IE_TIM;			/* TIM IE */
	ie_list[2] = IE_BSS_MAX_IDLE;	/* BSS Max Idle Period IE */
	ie_list[3] = IE_OPERATING_MODE_NOTIFY;	/* VHT OMI IE */
	ie_list_len += 4;

	ie_ext_list[0] = EID_EXT_EHT_MULTI_LINK;	/* MLD IE */
	ie_ext_list[1] = EID_EXT_EHT_TID2LNK_MAP;	/* TID-To-Link Mapping IE */
	ie_ext_list_len += 2;

	sta_pf_eid = (struct _EID_STRUCT *)sta_profile_ie_head;
	while ((UINT8 *)sta_pf_eid < (sta_profile_ie_head + sta_profile_ie_len)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
				 "non-setup link STA Profile, eid(%d), len(%d)\n",
				 sta_pf_eid->Eid, sta_pf_eid->Len);

		switch (sta_pf_eid->Eid) {
		case IE_VENDOR_SPECIFIC:
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
				"[non-setup link id=%d] found Vendor Specific in STA Profile!\n",
				non_setup_link_id);
			break;

		case IE_WLAN_EXTENSION:
		{
			/* parse EXTENSION EID */
			UINT8 *extension_id = (UINT8 *)sta_pf_eid + 2;

			switch (*extension_id) {
			case IE_EXTENSION_ID_NON_INHERITANCE:
			{
				UINT8 i, non_inherit_id_len, non_inherit_id_ext_len;
				UINT8 *non_inherit_eid = (UINT8 *)sta_pf_eid + 3;

				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
					"[non-setup link id=%d] parse non-inheritance element\n",
					non_setup_link_id);

				/* IE */
				non_inherit_id_len = *non_inherit_eid;
				if (non_inherit_id_len > (sizeof(ie_list) - ie_list_len)) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
						"Error: IE of input frame is invalid\n");
					if (target_buf)
						os_free_mem(target_buf);
					return NDIS_STATUS_FAILURE;
				}
				for (i = 0; i < non_inherit_id_len; i++) {
					non_inherit_eid++; /* jump to next id */
					ie_list[ie_list_len] = *non_inherit_eid;
					ie_list_len++;
				}
				/* IE_Extension */
				non_inherit_eid++;
				non_inherit_id_ext_len = *non_inherit_eid;
				if (non_inherit_id_ext_len > (sizeof(ie_ext_list) - ie_ext_list_len)) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
						"Error: IE Extension of input frame is invalid\n");
					if (target_buf)
						os_free_mem(target_buf);
					return NDIS_STATUS_FAILURE;
				}
				for (i = 0; i < non_inherit_id_ext_len; i++) {
					non_inherit_eid++; /* jump to next id */
					ie_ext_list[ie_ext_list_len] = *non_inherit_eid;
					ie_ext_list_len++;
				}

				/* remove the non-inheritance IE in STA Profile */
				final_sta_profile_ie_len -= (5 + non_inherit_id_len + non_inherit_id_ext_len);
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
					"[non-setup link id=%d] In non-inheritance element: #ID=%d, #IDExt=%d\n",
					non_setup_link_id, non_inherit_id_len, non_inherit_id_ext_len);
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
					"[non-setup link id=%d] STA Profile len=%d, Final STA Profile len=%d\n",
					non_setup_link_id, sta_profile_ie_len, final_sta_profile_ie_len);
				break;
			}
			default:
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
					"[non-setup link id=%d] found ID_EXT[%d] in STA Profile\n",
					non_setup_link_id, *extension_id);
				ie_ext_list[ie_ext_list_len] = *extension_id;
				ie_ext_list_len++;
				break;
			}
		}
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
				"[non-setup link id=%d] found ID[%d] in STA Profile\n",
				non_setup_link_id, sta_pf_eid->Eid);
			ie_list[ie_list_len] = sta_pf_eid->Eid;
			ie_list_len++;
			break;
		}

		sta_pf_eid = (PEID_STRUCT)((u8 *)sta_pf_eid + 2 + sta_pf_eid->Len);
	}


	/* remove the IEs that also appear in STA Profile in association request frame
	 *
	 * skip header & fixed fields: assoc req frame - header - CapInfo(2) - ListenInterval(2)
	 *  reassoc req frame - header - CapInfo(2) - ListenInterval(2) - Current AP address(6)
	 */
	WRAP_BUFFER(source, source_buf + shift, source_len - shift);
	WRAP_BUFFER(target, target_buf, target_len);
	WRAP_BUFFER(id_buf, ie_list, ie_list_len);
	WRAP_BUFFER(id_ext_buf, ie_ext_list, ie_ext_list_len);
	eht_remove_ie_by_id_list(&source, &target, &id_buf, &id_ext_buf);

	target_len = target.buf_len;

	/* Step 4. append IEs from STA Profile to non-setup link association req frame */
	NdisMoveMemory(target_buf + target_len, sta_profile_ie_head, final_sta_profile_ie_len);
	target_len += final_sta_profile_ie_len;
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_INFO,
			"[PreProcess done] Move STA Profile (len=%d) to Req Buf, final len=%ld\n",
			final_sta_profile_ie_len, target_len);

	/* Step 5. move the resulting assoc req frame to input buffer (Elem->Msg) */
	NdisMoveMemory(Elem->Msg, target_buf, target_len);
	Elem->MsgLen = target_len;

	if (target_buf)
		os_free_mem(target_buf);

	return NDIS_STATUS_SUCCESS;
}

/*
	Description:
		convert association response frame of non-setup link into
		STA Profile in Per-STA Profile sub-element in Basic multi-link IE
*/
static NDIS_STATUS eht_ap_mlo_postprocess_non_setup_link_assoc_rsp(
	IN OUT struct eht_assoc_req_priv *assoc_info,		/* non-setup link assoc rsp */
	IN struct buffer_wrapper *setup_link_rsp,			/* setup link assoc rsp */
	IN UINT8 non_setup_link_id)
{
// #define POST_REMOVE_IE
#undef POST_REMOVE_IE

	UINT8 *setup_source = setup_link_rsp->buf;
	ULONG setup_source_len = setup_link_rsp->buf_len;

	UINT8 *non_setup_source = assoc_info->buf;
	ULONG non_setup_source_len = assoc_info->buf_len;

	UINT8 *target_buf = NULL;
	ULONG target_len = 0;

	UINT16 cap_info, status_code;
#ifdef POST_REMOVE_IE
	UINT8 ie_list[100] = {0}, ie_ext_list[100] = {0};
	UINT8 ie_list_len = 0, ie_ext_list_len = 0;
	struct buffer_wrapper id_buf, id_ext_buf;
#endif

	struct buffer_wrapper setup_rsp, non_setup_rsp, target;

	ULONG shift;
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
		"Start post-processing non setup link assoc rsp\n");
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
		"setup rsp: len=%ld, addr=%p\n", setup_source_len, setup_source);
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
		"non-setup rsp: len=%ld, addr=%p\n", non_setup_source_len, non_setup_source);

	status = MlmeAllocateMemory(NULL, &target_buf);
	if (status != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	/* Step 1. process setup link first */
	/* skip cap info: 2, status code: 2, AID: 2 */
	shift = sizeof(HEADER_802_11) + 6;
	setup_source += shift;
	setup_source_len -= shift;

	/* Step 2. then process non-setup link */
	/* skip 802.11 header */
	shift = sizeof(HEADER_802_11);
	non_setup_source += shift;
	non_setup_source_len -= shift;

	NdisMoveMemory(&cap_info, non_setup_source, sizeof(cap_info));
	non_setup_source += sizeof(cap_info);
	NdisMoveMemory(&status_code, non_setup_source, sizeof(status_code));
	non_setup_source += sizeof(status_code);

	/* cap info: 2, status code: 2, AID: 2 */
	/* skip AID */
	non_setup_source += 2;
	non_setup_source_len -= 6;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_DEBUG,
		"[after remove header] setup rsp: len=%ld, addr=%p\n",
		setup_source_len, setup_source);
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_DEBUG,
		"[after remove header] non-setup rsp: len=%ld, addr=%p\n",
		non_setup_source_len, non_setup_source);

	/* Step 3. inheritance rule */
	WRAP_BUFFER(setup_rsp, setup_source, setup_source_len);
	WRAP_BUFFER(non_setup_rsp, non_setup_source, non_setup_source_len);
	WRAP_BUFFER(target, target_buf, target_len);
	status = eht_mlo_sta_profile_inheritance_rule(&setup_rsp, &non_setup_rsp, &target);
	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
			"inheritance rule fail, stop post-processing\n");
		goto non_setup_assoc_post_fail;
	}

	target_len = target.buf_len;
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
		"length after inheritance rule: %ld\n", target_len);

	/* Step 4. Start to form the output (STA Profile for non-setup link) */
	assoc_info->buf_len = 0;
	NdisMoveMemory(assoc_info->buf, &cap_info, sizeof(cap_info));
	assoc_info->buf_len += sizeof(cap_info);

	if (status_code != MLME_SUCCESS)
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
			"Error: non-setup link assoc fail, status code is 0x%x\n",
			status_code);
	/*
	 * If the setup link fails, the status code of the non-setup link needs to be
	 * replaced with MLME_MLO_ASSOC_SETUP_LINK_FAIL, if the non-setup link is successful.
	 */
	if (!assoc_info->setup_link_success && (status_code == MLME_SUCCESS)) {
		status_code = MLME_MLO_ASSOC_SETUP_LINK_FAIL;
		assoc_info->non_setup_link_status = MLME_MLO_ASSOC_SETUP_LINK_FAIL;
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
			"Notice: setup link assoc fail, patch the status code of non-setup link to %d\n",
			status_code);
	}
	NdisMoveMemory(assoc_info->buf + assoc_info->buf_len, &status_code, sizeof(status_code));
	assoc_info->buf_len += sizeof(status_code);

	/* any intentional IE removal or simply append the result of inheritance rule */
#ifdef POST_REMOVE_IE /* remove IEs */
	/* the IDs listed in ie_list will be removed */
	ie_list[0] = IE_BSS_MAX_IDLE;	/* BSS Max Idle Period IE */
	ie_list_len += 1;

	// ie_ext_list[0] = EID_EXT_EHT_MULTI_LINK;		/* MLD IE */
	// ie_ext_list[1] = EID_EXT_EHT_TID2LNK_MAP;	/* TID-To-Link Mapping IE */
	// ie_ext_list_len += 2;

	WRAP_BUFFER(non_setup_rsp, target_buf, target_len);
	WRAP_BUFFER(target, assoc_info->buf, assoc_info->buf_len);
	WRAP_BUFFER(id_buf, ie_list, ie_list_len);
	WRAP_BUFFER(id_ext_buf, ie_ext_list, ie_ext_list_len);
	eht_remove_ie_by_id_list(&non_setup_rsp, &target, &id_buf, &id_ext_buf);

	assoc_info->buf_len = target.buf_len;
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
		"length after removal: %d\n", assoc_info->buf_len);
#else
	NdisMoveMemory(assoc_info->buf + assoc_info->buf_len, target_buf, target_len);
	assoc_info->buf_len += target_len;
#endif

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_INFO,
		"(PostProcess done) final length=%ld\n",
		assoc_info->buf_len);

non_setup_assoc_post_fail:
	if (target_buf)
		os_free_mem(target_buf);

	return status;
}

/*
	Description:
		handle MLO non-setup links association, includes:
		1. (preprocess) convert setup link assoc req to non-setup link assoc req
		2. apply the non-setup link association
		3. (postprocess) convert non-setup link assoc rsp to STA Profile
		4. delivery STA Profile to BSS manager and will be added into ML IE
*/
VOID eht_ap_mlo_peer_non_setup_links_assoc_req(
	IN struct _MLME_QUEUE_ELEM *Elem,			/* setup link req */
	IN struct _MAC_TABLE_ENTRY *pEntry,			/* setup link pEntry */
	IN UINT8 setup_link_Sst,					/* setup link Sst */
	IN USHORT setup_link_status_code,			/* setup link Status Code */
	IN struct buffer_wrapper *setup_link_rsp,	/* setup link rsp */
	IN struct common_ies *cmm_ies,				/* non-setup link STA Profile */
	IN BOOLEAN isReassoc)
{
	NDIS_STATUS status;
	UINT8 i, non_setup_link_id, setup_link_id;
	UINT8 *error_result_buf = NULL;
	BOOLEAN link_assoc_done[MLD_LINK_MAX] = {0};
	struct ml_ie_info *ml_info = NULL;
	struct ml_ie_link ml_link = {0};
	struct eht_assoc_req_priv assoc_info = {0};
	PRTMP_ADAPTER pAd = NULL;
	MAC_TABLE_ENTRY *non_setup_link_pEntry = NULL;
	MLME_QUEUE_ELEM *temp_elem = NULL;
	struct mld_entry_t *mld_entry = NULL;
	struct _MAC_TABLE_ENTRY *mld_link_entry[MLD_LINK_MAX];
	uint16_t mld_sta_idx;

	mt_rcu_read_lock();
	if (pEntry->mlo.mlo_en)
		mld_entry = rcu_dereference(pEntry->mld_entry);
	if (!mld_entry) {
		mt_rcu_read_unlock();
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
			"mld_entry is NULL\n");
		return;
	}

	for (i = 0; i < MLD_LINK_MAX; i++)
		mld_link_entry[i] = mld_entry->link_entry[i];

	mld_sta_idx = mld_entry->mld_sta_idx;
	mt_rcu_read_unlock();

	setup_link_id = pEntry->mlo.link_info.link_id;
	if (!HAS_EHT_MLD_EXIST(cmm_ies->ie_exists)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
			"[setup link id=%d] Error: MLD IE doesn't exist\n", setup_link_id);
		return;
	}

	link_assoc_done[setup_link_id] = TRUE;
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_INFO,
		"[setup link=%d]\n\t\t----- Start the non-setup link association -----\n", setup_link_id);

	/* 1. collect non-setup link IEs information (address & length) */
	os_alloc_mem(NULL, (UCHAR **)&ml_info, sizeof(struct ml_ie_info));
	if (!ml_info) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
			"failed to allocate memory for parsing ML IE\n");
		goto non_setup_assoc_fail;
	}
	NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
	eht_parse_multi_link_ie(pEntry->wdev, cmm_ies->ml_ie, &cmm_ies->ml_frag_info, ml_info);

	os_alloc_mem(NULL, (UCHAR **)&temp_elem, sizeof(MLME_QUEUE_ELEM));
	if (!temp_elem) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
			"[setup link id=%d] Fatal error: fail to allocate memory\n", setup_link_id);
		goto non_setup_assoc_fail;
	}

	/* 2. MLO, do non-setup links association */
	for (i = 0; i < MLD_LINK_MAX; i++) {

		if (i == setup_link_id)
			continue;

		non_setup_link_pEntry = mld_link_entry[i];
		if (!non_setup_link_pEntry ||
			!non_setup_link_pEntry->mlo.mlo_en) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
				"[link id=%d] STA do not request this non-setup link\n", i);
			continue;
		}
		ml_link = ml_info->link[non_setup_link_pEntry->mlo.link_info.link_id];

		if (ml_link.active) {
			/* sync Sst from setup link to non-setup link */
			non_setup_link_pEntry->Sst = setup_link_Sst;

			/* pre-process the temp assoc buffer */
			NdisZeroMemory(temp_elem, sizeof(MLME_QUEUE_ELEM));
			NdisMoveMemory(temp_elem, Elem, sizeof(MLME_QUEUE_ELEM));

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
				"[link id=%d] Length in ML IE: sta_info=%d, sta_profile=%d\n",
				i, ml_link.sta_info_len, ml_link.sta_profile_len);

			non_setup_link_id = non_setup_link_pEntry->mlo.link_info.link_id;

			/* form a temp association request for non-setup link based on
			 * setup link association frame & Per-STA Profile in ML IE
			 */
			status = eht_ap_mlo_preprocess_non_setup_link_assoc_req(
						temp_elem,	/* used for both input/output */
						&ml_link,	/* per-link info */
						non_setup_link_pEntry,
						isReassoc);

			if (status != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
					"[setup link id=%d] Error: fail to preprocess frame for non-setup link[%d]\n",
					setup_link_id, non_setup_link_id);
				goto non_setup_assoc_fail;
			}

			/* sync temp_elem for non-setup link association */
			temp_elem->Wcid = non_setup_link_pEntry->wcid;
			temp_elem->wdev = non_setup_link_pEntry->wdev;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
				"[link id=%d] Temp Assoc Req, Wcid=%d, wdev=%s\n",
				i, temp_elem->Wcid, temp_elem->wdev->if_dev->name);

			/* prepare assoc_info struct to get non-setup link association response */
			assoc_info.buf = NULL;
			assoc_info.buf_len = 0;
			assoc_info.non_setup_link_status = MLME_UNSPECIFY_FAIL;
			assoc_info.setup_link_success = (setup_link_status_code == MLME_SUCCESS) ? TRUE : FALSE;
			temp_elem->Others = &assoc_info;

			/* non-setup link build association */
			pAd = non_setup_link_pEntry->wdev->sys_handle;
			ap_cmm_peer_assoc_req_action(pAd, temp_elem, isReassoc);

			if (assoc_info.buf_len == 0) {
				/* error handling: can not acquire association response frame
				 * for non-setup links. Insert capability information & Status Code
				 */
				BSS_STRUCT *pMbss = non_setup_link_pEntry->wdev->func_dev;
				USHORT error_code = MLME_UNSPECIFY_FAIL;

				os_alloc_mem(NULL, (UCHAR **)&error_result_buf,
					sizeof(error_code) + sizeof(pMbss->CapabilityInfo));
				if (!error_result_buf) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
						"[setup link id=%d] Fatal error: fail to allocate memory\n", setup_link_id);
					goto non_setup_assoc_fail;
				}

				NdisMoveMemory(error_result_buf, &error_code, sizeof(error_code));
				NdisMoveMemory(error_result_buf + sizeof(error_code),
					&pMbss->CapabilityInfo, sizeof(pMbss->CapabilityInfo));
				assoc_info.buf = error_result_buf;
				assoc_info.buf_len = sizeof(error_code) + sizeof(pMbss->CapabilityInfo);
				assoc_info.non_setup_link_status = error_code;
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
					"[non-setup link id=%d] Error: ASSOC unknown error\n", i);
			} else {
				/* post-process: convert non-setup link association response
				 * to STA Profile for Per-STA Profile in ML IE in setup link
				 * association response frame */
				status = eht_ap_mlo_postprocess_non_setup_link_assoc_rsp(
							&assoc_info,  /* for both input/output */
							setup_link_rsp,
							non_setup_link_id);

				if (status != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
						"[non-setup link id=%d] Fatal error: fail to postprocess frame\n",
						non_setup_link_id);
					/* free mem allocated by ap_cmm_peer_assoc_req_action() */
					if (assoc_info.buf)
						MlmeFreeMemory((PVOID) assoc_info.buf);
					goto non_setup_assoc_fail;
				}
			}

			/* deliver STA Profile to BSS Manager */
			/* deliver to setup link */
			if (assoc_info.buf_len < EHT_MAX_STA_PROFILE_FIELD_LEN)
				bss_mngr_mld_add_sta_profile(
					non_setup_link_pEntry->wdev, mld_sta_idx,
					non_setup_link_id, BMGR_MLD_ASSOC_RSP_COMPLETE_PROFILE,
					assoc_info.buf, assoc_info.buf_len);
			else
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
					"[link id=%d] Error!!! After postprocess, len of STA Profile > %d, drop it\n",
					i, EHT_MAX_STA_PROFILE_FIELD_LEN);

			link_assoc_done[i] = TRUE;

			/* free memory allocated by ap_cmm_peer_assoc_req_action() */
			if (assoc_info.buf)
				MlmeFreeMemory((PVOID) assoc_info.buf);

			if (assoc_info.non_setup_link_status != MLME_SUCCESS) {
				/* Inform bssmngr to delete unsuccessful assoc non-setup link */
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_NOTICE,
					"[link id=%d] Delete entry of unsuccess non-setup link(%pM)\n",
					i, non_setup_link_pEntry->Addr);
				/* corresponding to-be-deleted link in MLD STA = link_id of wdev */
				bss_mngr_mld_disconn_op(
					non_setup_link_pEntry->wdev,
					mld_sta_idx,
					MLD_DISC_OP_DEL_STA_LINK);
			}
		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
				"[link id=%d] this non-setup link do not setup MLO\n", i);
		}
	}

non_setup_assoc_fail:
	if (ml_info)
		os_free_mem(ml_info);

	if (temp_elem)
		os_free_mem(temp_elem);

	/* report the association results */
	for (i = 0; i < MLD_LINK_MAX; i++) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
			"%s link[%d] assoc: %s\n", i == setup_link_id ? "setup" : "non-setup",
			i, link_assoc_done[i] ? "done" : "NG or w/o requested");
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_INFO,
		"[setup link=%d]\n\t\t----- End of the non-setup link association -----\n",
		setup_link_id);
}

BOOLEAN PeerAssocReqMloCapSanity
(
	RTMP_ADAPTER *pAd,
	BOOLEAN isReassoc,
	VOID *Msg,
	INT MsgLen,
	IE_LISTS *ie_lists,
	BOOLEAN is_mlo_setup_link)
{
	struct _MAC_TABLE_ENTRY *pEntry;
	PFRAME_802_11 Fr = (PFRAME_802_11)Msg;

	pEntry = MacTableLookup(pAd, &Fr->Hdr.Addr2[0]);

	if (pEntry == NULL)
		return FALSE;

	/* existing peer entry's mlo-capability not sync to previous */
	if (is_mlo_setup_link) {
		if ((!IS_ENTRY_MLO(pEntry) && HAS_EHT_MLD_EXIST(ie_lists->cmm_ies.ie_exists)) ||
			(IS_ENTRY_MLO(pEntry) && !HAS_EHT_MLD_EXIST(ie_lists->cmm_ies.ie_exists))) {
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"Fail: setup sta(%pM) wcid(%d) mlo-capability(%d) changed (invalid)\n",
				pEntry->Addr, pEntry->wcid, IS_ENTRY_MLO(pEntry));
			return FALSE;
		}
	} else {
		if (!IS_ENTRY_MLO(pEntry)) {
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"Fail: non-setup sta(%pM) wcid(%d) mlo-capability(%d) changed (invalid)\n",
				pEntry->Addr, pEntry->wcid, IS_ENTRY_MLO(pEntry));
			return FALSE;
		}
	}

	return TRUE;
}

#endif /* DOT11_EHT_BE */
