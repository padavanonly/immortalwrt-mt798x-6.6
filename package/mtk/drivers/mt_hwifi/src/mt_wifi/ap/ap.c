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
****************************************************************************

   Module Name:
   soft_ap.c

   Abstract:
   Access Point specific routines and MAC table maintenance routines

   Revision History:
   Who         When          What
   --------    ----------    ----------------------------------------------
   John Chang  08-04-2003    created for 11g soft-AP

*/

#include "rt_config.h"
#define VLAN_HDR_LEN	4
#define MCAST_WCID_TO_REMOVE 0 /* Pat: TODO */
#ifdef TR181_SUPPORT
#include "hdev/hdev_basic.h"
#endif /*TR181_SUPPORT*/

#if defined(VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT)
extern VOID vow_avg_pkt_len_reset(struct _RTMP_ADAPTER *ad);
extern VOID vow_avg_pkt_len_calculate(struct _MAC_TABLE_ENTRY *entry);
#endif

#ifdef IAPP_SUPPORT
BOOLEAN IAPP_L2_Update_Frame_Send(RTMP_ADAPTER *pAd, UINT8 *mac, INT wdev_idx);
#endif /*IAPP_SUPPORT*/

static VOID dynamic_ampdu_efficiency_adjust_all(struct _RTMP_ADAPTER *ad);

UCHAR IXIA_PROBE_ADDR[MAC_ADDR_LEN] = {0x00, 0x00, 0x02, 0x00, 0x00, 0x02};
char const *pEventText[EVENT_MAX_EVENT_TYPE] = {
	"restart access point",
	"successfully associated",
	"has disassociated",
	"has been aged-out and disassociated",
	"active countermeasures",
	"has disassociated with invalid PSK password"
};

#ifdef EAP_STATS_SUPPORT
struct tx_stats_monitor_threshold txm_thres = {
	.per_err_total = CONTD_PER_ERR_CNT,
	.tx_contd_fail_total = CONTD_TX_FAIL_CNT
};
#endif

UCHAR get_apidx_by_addr(RTMP_ADAPTER *pAd, UCHAR *addr)
{
	UCHAR apidx;

	for (apidx = 0; ((apidx < pAd->ApCfg.BssidNum) && (apidx < MAX_MBSSID_NUM(pAd))); apidx++) {
		if (RTMPEqualMemory(addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN))
			break;
	}

	return apidx;
}

static INT ap_mlme_set_capability(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	struct wifi_dev *wdev = &pMbss->wdev;
	BOOLEAN SpectrumMgmt = FALSE;
#ifdef A_BAND_SUPPORT

	/* Decide the Capability information field */
	/* In IEEE Std 802.1h-2003, the spectrum management bit is enabled in the 5 GHz band */
	if (((wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G) ||
		(wlan_config_get_ch_band(wdev) == CMD_CH_BAND_6G))
		&& pAd->CommonCfg.bIEEE80211H == TRUE)
		SpectrumMgmt = TRUE;

#endif /* A_BAND_SUPPORT */
	pMbss->CapabilityInfo = CAP_GENERATE(1,
								 0,
								 IS_SECURITY_Entry(wdev),
								 (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1),
								 pAd->CommonCfg.bUseShortSlotTime,
								 SpectrumMgmt);
#ifdef DOT11K_RRM_SUPPORT

	if (IS_RRM_ENABLE(wdev))
		pMbss->CapabilityInfo |= RRM_CAP_BIT;

#endif /* DOT11K_RRM_SUPPORT */

	if (pMbss->wdev.bWmmCapable == TRUE) {
		/*
		    In WMM spec v1.1, A WMM-only AP or STA does not set the "QoS"
		    bit in the capability field of association, beacon and probe
		    management frames.
		*/
		/*          pMbss->CapabilityInfo |= 0x0200; */
	}
#ifdef UAPSD_SUPPORT
	if (pMbss->wdev.UapsdInfo.bAPSDCapable == TRUE) {
		/*
			QAPs set the APSD subfield to 1 within the Capability
			Information field when the MIB attribute
			dot11APSDOptionImplemented is true and set it to 0 otherwise.
			STAs always set this subfield to 0.
		*/
		pMbss->CapabilityInfo |= 0x0800;
	}
#endif /* UAPSD_SUPPORT */

	return TRUE;
}

static VOID do_sta_keep_action(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry, BOOLEAN is_no_rx)
{
#ifdef IAPP_SUPPORT
	if (entry->wdev &&
		is_no_rx &&
		(ad->Mlme.PeriodicRound % STA_KEEP_ALIVE_NOTIFY_L2) == 0) {

#ifdef DOT11_EHT_BE
		if (IS_ENTRY_MLO(entry)) {
			/* In MLO case, XID packet only update on setup link interface */
			if (entry->mlo.is_setup_link_entry) {
				MTWF_DBG(ad, DBG_CAT_AP, CATAP_KPLIVE, DBG_LVL_DEBUG,
						 "XID entry->mlo.mld_addr="MACSTR
						 " entry->Addr="MACSTR"\n"
						 , MAC2STR(entry->mlo.mld_addr),
						 MAC2STR(entry->Addr));
				IAPP_L2_Update_Frame_Send(ad, entry->mlo.mld_addr, entry->wdev->wdev_idx);
			} else
				MTWF_DBG(ad, DBG_CAT_AP, CATAP_KPLIVE, DBG_LVL_DEBUG,
						 "No update XID entry->mlo.mld_addr="MACSTR
						 " entry->Addr="MACSTR"\n",
						 MAC2STR(entry->mlo.mld_addr),
						 MAC2STR(entry->Addr));
		} else
#endif
			IAPP_L2_Update_Frame_Send(ad, entry->Addr, entry->wdev->wdev_idx);
	}
#endif /*IAPP_SUPPORT*/
}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
BOOLEAN ApAutoChannelSkipListBuild(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR chGrp = 0;
	BOOLEAN status = FALSE;
	UCHAR size = 0;

	SET_V10_OFF_CHNL_TIME(pAd, V10_NORMAL_SCAN_TIME);

	if (pAd->ApCfg.bAutoChannelAtBootup) {
		/* ACS Enable */
		if (wdev->channel != 0) {
			/* Non-Zero Channel in ACS */
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"Non-Zero Chnl in ACS\n");
			goto done;
		} else {
			AutoChannelSkipListClear(pAd);
			if (wlan_config_get_vht_bw(wdev) == VHT_BW_80) {
				if (pAd->CommonCfg.bCh144Enabled == FALSE) {
					AutoChannelSkipChannels(pAd, V10_W56_VHT20_SIZE, GROUP5_LOWER);
					AutoChannelSkipChannels(pAd, 1, 144);
				}
				AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
				status = TRUE;
			} else if (wlan_config_get_vht_bw(wdev) == VHT_BW_2040) {
				if (pAd->CommonCfg.bCh144Enabled == FALSE)
					AutoChannelSkipChannels(pAd, 1, 144);
				AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
				status = TRUE;
			} else
				return FALSE;
		}
	} else {
		/* ACS Disable */
		if (wdev->channel == 0) {
			/* No Channel in Non-ACS */
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"Zero Channel in Non ACS %d\n", wdev->channel);
			goto done;
		} else {
			/* Background ACS Algorithm = 3 */
			pAd->ApCfg.AutoChannelAlg = ChannelAlgBusyTime;

			if (IS_V10_W56_GRP_VALID(pAd))
				chGrp = W56_UA;
			else if (IS_V10_W56_VHT80_SWITCHED(pAd) && (pAd->CommonCfg.bCh144Enabled == FALSE))
				chGrp = W56_UC;
			else
				chGrp = DfsV10CheckChnlGrp(pAd, wdev->channel);

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"CHgrp %d Channel %d\n", chGrp, wdev->channel);

			/* Clean Skip List */
			AutoChannelSkipListClear(pAd);

			if (chGrp == NA_GRP) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
						"Illegal Group Number\n");
				return status;
			} else if (chGrp == W52) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "W52\n");
				AutoChannelSkipChannels(pAd, V10_W53_SIZE, GROUP2_LOWER);
				AutoChannelSkipChannels(pAd, V10_W56_SIZE, GROUP3_LOWER);
				AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
				status = TRUE;
			} else if (chGrp == W53) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "W53\n");

				if (DfsV10CheckGrpChnlLeft(pAd, W53, V10_W53_SIZE, BandIdx))
					AutoChannelSkipChannels(pAd, V10_W52_SIZE, GROUP1_LOWER);
				else {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
							"W53 Channel Finish\n");
					AutoChannelSkipChannels(pAd, V10_W53_SIZE, GROUP2_LOWER);
				}

				AutoChannelSkipChannels(pAd, V10_W56_SIZE, GROUP3_LOWER);
				AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);

				status = TRUE;
			} else if (chGrp == W56_UA || chGrp == W56_UB || chGrp == W56_UC) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "W56\n");
				SET_V10_W56_GRP_VALID(pAd, FALSE);

				if (wlan_config_get_vht_bw(wdev) == VHT_BW_2040) {
					if (pAd->CommonCfg.bCh144Enabled)
						size = V10_W56_VHT80_SIZE;
					else
						size = V10_W56_VHT80_SIZE - V10_W56_VHT80_C_SIZE;

					if (IS_V10_W56_VHT80_SWITCHED(pAd) && chGrp == W56_UC)
						AutoChannelSkipChannels(pAd, size, GROUP3_LOWER);

					AutoChannelSkipChannels(pAd, V10_W52_SIZE, GROUP1_LOWER);
					AutoChannelSkipChannels(pAd, V10_W53_SIZE, GROUP2_LOWER);
					if (pAd->CommonCfg.bCh144Enabled == FALSE)
						AutoChannelSkipChannels(pAd, 1, 144);
					AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
					status = TRUE;
				} else if (wlan_config_get_vht_bw(wdev) == VHT_BW_80) {
					if (chGrp == W56_UC && (pAd->CommonCfg.bCh144Enabled == FALSE)) {
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
							"Incorrect Channel W56 C\n");
						return status;
					}

					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
						"[%s] W56_80\n");
					AutoChannelSkipChannels(pAd, V10_W52_SIZE, GROUP1_LOWER);
					AutoChannelSkipChannels(pAd, V10_W53_SIZE, GROUP2_LOWER);
					if (pAd->CommonCfg.bCh144Enabled == FALSE) {
						AutoChannelSkipChannels(pAd, V10_W56_VHT20_SIZE, GROUP5_LOWER);
						AutoChannelSkipChannels(pAd, 1, 144);
					}
					AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
					status = TRUE;
				}
			}
		}
	}
done:
	return status;
}
#endif

static VOID ap_enable_rxtx(RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_TRCTRL, DBG_LVL_INFO, "==> ap_enable_rxtx\n");

	if (pAd->CommonCfg.bTXRX_RXV_ON)
		AsicSetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, TRUE);
	else
		AsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);

	MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_TRCTRL, DBG_LVL_INFO, "<== ap_enable_rxtx\n");
}

UCHAR ApAutoChannelAtBootUp(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
#define SINGLE_BAND 0
#define DUAL_BAND   1

	UCHAR NewChannel;
	UCHAR OriChannel;
	BOOLEAN IsAband;
	AUTO_CH_CTRL *pAutoChCtrl;
	UCHAR vht_bw;
#ifdef TR181_SUPPORT
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
#endif /*TR181_SUPPORT*/
#if ((DFS_ZEROWAIT_DEFAULT_FLOW == 1) && defined(BACKGROUND_SCAN_SUPPORT))
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
#endif /*(DFS_ZEROWAIT_DEFAULT_FLOW == 1) && defined(BACKGROUND_SCAN_SUPPORT)*/
	PCHANNEL_CTRL pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "----------------->\n");

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
				 "\x1b[41m wdev is NULL !!\x1b[m\n");
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "<-----------------\n");
		return FALSE;
	}
	OriChannel = wdev->channel;
	vht_bw = wlan_config_get_vht_bw(wdev);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
		"%d\n", pAd->ApCfg.bAutoChannelAtBootup);

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		if (IS_SUPPORT_V10_DFS(pAd) && (IS_V10_BOOTACS_INVALID(pAd) == FALSE)
				&& (IS_V10_APINTF_DOWN(pAd) == FALSE)) {
			if (ApAutoChannelSkipListBuild(pAd, wdev) == FALSE)
				return FALSE;
		} else if (!pAd->ApCfg.bAutoChannelAtBootup) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
				"<-----------------\n");
			return FALSE;
		}
#endif

	pAutoChCtrl = HcGetAutoChCtrl(pAd);

	/* Now Enable RxTx*/
	ap_enable_rxtx(pAd);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
			 "PhyMode: %d\n", wdev->PhyMode);

	if (WMODE_CAP_5G(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode))
		IsAband = TRUE;
	else
		IsAband = FALSE;

	if ((wdev->channel == 0)
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		|| (IS_SUPPORT_V10_DFS(pAd) && (IS_V10_BOOTACS_INVALID(pAd) == FALSE))
#endif
		)

	{

		if (!pAd->ApCfg.bAutoChannelAtBootup) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
				"<-----------------\n");
			return FALSE;
		}

		/* Now we can receive the beacon and do ACS */
		if (pAutoChCtrl->AutoChSelCtrl.ACSChStat != ACS_CH_STATE_SELECTED) {
			/* Disable MibBucket during doing ACS */
			pAd->MsMibBucket.Enabled = FALSE;
			pAd->OneSecMibBucket.Enabled = FALSE;

			NewChannel = APAutoSelectChannel(pAd, wdev, pAd->ApCfg.AutoChannelAlg, IsAband);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				 "Auto channel selection: Selected channel = %d, IsAband = %d\n", NewChannel, IsAband);

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		if (IS_SUPPORT_V10_DFS(pAd) && NewChannel) {
			SET_V10_BOOTACS_INVALID(pAd, TRUE);
			DfsV10ACSMarkChnlConsumed(pAd, NewChannel);
		}
#endif

#ifdef MT_DFS_SUPPORT
		/* Record the boot up ACS channel */
		pChCtrl->ch_cfg.boot_chan = NewChannel;
#endif /* MT_DFS_SUPPORT */

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
		if (RadarChannelCheck(pAd, NewChannel)
			&& pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport == FALSE) {

			if ((pAd->CommonCfg.DfsParameter.CACMemoEn) &&
				(dfs_cac_op(pAd, wdev, CAC_DONE_CHECK, NewChannel))) {
				if (pAd->CommonCfg.DfsParameter.bPreCacEn)
					dfs_pre_cac_start_detect(pAd, wdev);
				else
					pAd->CommonCfg.DfsParameter.inband_ch_stat = DFS_INB_CH_SWITCH_CH;
			} else {
				/* If DFS ch X is selected by ACS,
				CAC of DFS ch X will be checked by dedicated RX */
				zero_wait_dfs_update_ch(pAd, wdev, OriChannel, &NewChannel);
			}
		} else
			dfs_pre_cac_start_detect(pAd, wdev);

#endif
#endif
		} else {
			NewChannel = pAutoChCtrl->AutoChSelCtrl.SelCh;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE, "ACS is already DONE, Channel = %d\n", NewChannel);
		}

		/* Update channel of wdev as new channel */
		AutoChSelUpdateChannel(pAd, NewChannel, IsAband, wdev);

		/* Enable MibBucket after ACS done */
		pAd->MsMibBucket.Enabled = TRUE;
		pAd->OneSecMibBucket.Enabled = TRUE;

		if (!pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport) {
			/* Check new channel is DFS channel or not */
			if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G)
				RadarStateCheck(pAd, wdev);
		}
	}
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
		"Chnl Restore Enbl %d\n", IS_V10_OLD_CHNL_VALID(wdev));
	if (!IS_V10_OLD_CHNL_VALID(wdev))
		/* Update channel of wdev as new channel */
		AutoChSelUpdateChannel(pAd, NewChannel, IsAband, wdev);
#endif

#endif /* MT_DFS_SUPPORT && BACKGROUND_SCAN_SUPPORT */


#ifdef RT_CFG80211_SUPPORT
	/* send acs complete event*/
	nl80211_send_acs_complete_event(pAd, wdev);
#endif

	/* sync to other device */
	wdev_sync_prim_ch(pAd, wdev);

	/* Update primay channel */
	wlan_operate_set_prim_ch(wdev, wdev->channel);
#ifdef TR181_SUPPORT
	pAd->ApBootACSChannelChangePerBandCount++;
	if (ctrl)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "Boot:%d Total:%d\n",
			pAd->ApBootACSChannelChangePerBandCount,
			(ctrl->rdev.pRadioCtrl->TotalChannelChangeCount +
			pAd->ApBootACSChannelChangePerBandCount));
#endif /*TR181_SUPPORT*/

#ifdef MT_DFS_SUPPORT
	DfsBuildChannelList(pAd, wdev);
#if ((DFS_ZEROWAIT_DEFAULT_FLOW == 1) && defined(BACKGROUND_SCAN_SUPPORT))
	if (pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport == FALSE) {
		if (*ch_stat == DFS_INB_CH_INIT)
			zero_wait_dfs_switch_ch(pAd, wdev, RDD_DEDICATED_IDX);
		else
			pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_RDD_DETEC;
	}
#endif
#endif /* DFS_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "<-----------------\n");
	return TRUE;
}


/*
	==========================================================================
	Description:
	Check to see the exist of long preamble STA in associated list
    ==========================================================================
 */
static BOOLEAN ApCheckLongPreambleSTA(RTMP_ADAPTER *pAd)
{
	UINT16   i;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = entry_get(pAd, i);

		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
			continue;

		if (!CAP_IS_SHORT_PREAMBLE_ON(pEntry->CapabilityInfo)) {
			return TRUE;
		}
	}

	return FALSE;
}

/*
	==========================================================================
	Description:
		Initialize AP specific data especially the NDIS packet pool that's
		used for wireless client bridging.
	==========================================================================
 */
static VOID ap_run_at_boot(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	if (ApAutoChannelAtBootUp(pAd, wdev) != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
				"\x1b[41m ACS is disable !!\x1b[m\n");
	}

}

NDIS_STATUS APOneShotSettingInitialize(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "--->\n");
	RTMPInitTimer(pAd, &pAd->ApCfg.CounterMeasureTimer, GET_TIMER_FUNCTION(CMTimerExec), pAd, FALSE);
#ifdef IDS_SUPPORT
	/* Init intrusion detection timer */
	RTMPInitTimer(pAd, &pAd->ApCfg.IDSTimer, GET_TIMER_FUNCTION(RTMPIdsPeriodicExec), pAd, FALSE);
	pAd->ApCfg.IDSTimerRunning = FALSE;
#endif /* IDS_SUPPORT */
#ifdef IGMP_SNOOP_SUPPORT
#ifndef IGMP_SNOOPING_NON_OFFLOAD
	if (!IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
#endif
		MulticastFilterTableInit(pAd, &pAd->pMulticastFilterTable);
	MulticastWLTableInit(pAd, &pAd->pMcastWLTable);
#ifdef IGMP_SNOOPING_DENY_LIST
	MulticastDLTableInit(pAd, &pAd->pMcastDLTable);
#endif
#endif /* IGMP_SNOOP_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
	RRM_CfgInit(pAd);
#endif /* DOT11K_RRM_SUPPORT */
	/* Init BssTab & ChannelInfo tabbles for auto channel select.*/
	AutoChBssTableInit(pAd);
	ChannelInfoInit(pAd);
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	RTMP_11N_D3_TimerInit(pAd);
#endif /* DOT11N_DRAFT3 */
#endif /*DOT11_N_SUPPORT*/

#ifdef AP_QLOAD_SUPPORT
	QBSS_LoadInit(pAd);
#endif /* AP_QLOAD_SUPPORT */
	/*
	    Some modules init must be called before APStartUp().
	    Or APStartUp() will make up beacon content and call
	    other modules API to get some information to fill.
	*/

#ifdef CLIENT_WDS
	CliWds_ProxyTabInit(pAd);
#endif /* CLIENT_WDS */

	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "<---\n");
	return Status;
}


/*
	==========================================================================
	Description:
		Shutdown AP and free AP specific resources
	==========================================================================
 */
VOID APShutdown(RTMP_ADAPTER *pAd)
{
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];

	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_INFO, "--->\n");
#ifdef MT_MAC
	/*	Disable RX */
	APStop(pAd, pMbss, AP_BSS_OPER_ALL);
	/* MlmeRadioOff(pAd); */
#else
	MlmeRadioOff(pAd);
	APStop(pAd, pMbss, AP_BSS_OPER_ALL);
#endif
	/*remove sw related timer and table*/
	rtmp_ap_exit(pAd);

#ifdef WDS_SUPPORT
	NdisFreeSpinLock(&pAd->WdsTab.WdsTabLock);
#endif /* WDS_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_INFO, "<---\n");
}



/*
	==========================================================================
	Description:
		Update ERP IE and CapabilityInfo based on STA association status.
		The result will be auto updated into the next outgoing BEACON in next
		TBTT interrupt service routine
	==========================================================================
 */

VOID ApUpdateCapabilityAndErpIe(RTMP_ADAPTER *pAd, struct _BSS_STRUCT *mbss)
{
	UCHAR  ErpIeContent = 0;
	UINT16 i;
	BOOLEAN bUseBGProtection;
	BOOLEAN LegacyBssExist;
	BOOLEAN bNeedBcnUpdate = FALSE;
	BOOLEAN b2GBand = TRUE;
	MAC_TABLE_ENTRY *pEntry = NULL;
	USHORT *pCapInfo = NULL;
	struct wifi_dev *wdev = &mbss->wdev;
	UCHAR Channel = wdev->channel;
	USHORT PhyMode = wdev->PhyMode;
	BOOLEAN ShortSlotCapable = pAd->CommonCfg.bUseShortSlotTime;


	if (WMODE_EQUAL(PhyMode, WMODE_B))
		return;

	if (WMODE_CAP_5G(wdev->PhyMode))
		b2GBand = FALSE;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = entry_get(pAd, i);

		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
			continue;

		if (wdev != pEntry->wdev)
			continue;


		/* at least one 11b client associated, turn on ERP.NonERPPresent bit */
		/* almost all 11b client won't support "Short Slot" time, turn off for maximum compatibility */
		if (b2GBand) {
				if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE) {
				ShortSlotCapable = FALSE;
				ErpIeContent |= 0x01;
			}
		}

		/* at least one client can't support short slot */
		if ((pEntry->CapabilityInfo & 0x0400) == 0)
			ShortSlotCapable = FALSE;
	}

	/* legacy BSS exist within 5 sec */
	if ((pAd->ApCfg.LastOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32)
		LegacyBssExist = TRUE;
	else
		LegacyBssExist = FALSE;

	/* decide ErpIR.UseProtection bit, depending on pAd->CommonCfg.UseBGProtection
	   AUTO (0): UseProtection = 1 if any 11b STA associated
	   ON (1): always USE protection
	   OFF (2): always NOT USE protection
	   */
	if (b2GBand) {
		if (pAd->CommonCfg.UseBGProtection == 0) {
			ErpIeContent = (ErpIeContent) ? 0x03 : 0x00;

			/*if ((pAd->ApCfg.LastOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32) // legacy BSS exist within 5 sec */
			if (LegacyBssExist) {
				ErpIeContent |= 0x02;                                     /* set Use_Protection bit */
			}
		} else if (pAd->CommonCfg.UseBGProtection == 1)
			ErpIeContent |= 0x02;

		bUseBGProtection = (pAd->CommonCfg.UseBGProtection == 1) ||    /* always use */
						   ((pAd->CommonCfg.UseBGProtection == 0) && ERP_IS_USE_PROTECTION(ErpIeContent));
	} else {
		/* always no BG protection in A-band. falsely happened when switching A/G band to a dual-band AP */
		bUseBGProtection = FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UPDATE, DBG_LVL_DEBUG,
			 "-- bUseBGProtection: %s, BG_PROTECT_INUSED: %s, ERP IE Content: 0x%x\n",
			  (bUseBGProtection) ? "Yes" : "No",
			  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED)) ? "Yes" : "No",
			  ErpIeContent);

	if (b2GBand) {
		if (bUseBGProtection != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED)) {

			if (bUseBGProtection)
				OPSTATUS_SET_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
			else
				OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);

		}
	}

	/* Decide Barker Preamble bit of ERP IE */
	if ((pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong) || (ApCheckLongPreambleSTA(pAd) == TRUE))
		ErpIeContent |= 0x04;

	if (b2GBand) {
		if (ErpIeContent != pAd->ApCfg.ErpIeContent)
			bNeedBcnUpdate = TRUE;

		pAd->ApCfg.ErpIeContent = ErpIeContent;
	}

#ifdef A_BAND_SUPPORT

	/* Force to use ShortSlotTime at A-band */
	if (WMODE_CAP_5G(wdev->PhyMode))
		ShortSlotCapable = TRUE;

#endif /* A_BAND_SUPPORT */
	pCapInfo = &(mbss->CapabilityInfo);

	/* In A-band, the ShortSlotTime bit should be ignored. */
	if (ShortSlotCapable
#ifdef A_BAND_SUPPORT
		&& WMODE_CAP_2G(wdev->PhyMode)
#endif /* A_BAND_SUPPORT */
	   )
		(*pCapInfo) |= 0x0400;
	else
		(*pCapInfo) &= 0xfbff;

	if (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong)
		(*pCapInfo) &= (~0x020);
	else
		(*pCapInfo) |= 0x020;


	/*update slot time only when value is difference*/
	if (pAd->CommonCfg.bUseShortSlotTime != ShortSlotCapable) {
		HW_SET_SLOTTIME(pAd, ShortSlotCapable, Channel, wdev);
		pAd->CommonCfg.bUseShortSlotTime = ShortSlotCapable;
	}

	if (bNeedBcnUpdate)
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));
}



static INT ap_hw_tb_init(RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "Reset WCID Table\n");
	HW_SET_DEL_ASIC_WCID(pAd, WCID_ALL);
	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "Reset Sec Table\n");
	return TRUE;
}

#ifdef CONFIG_6G_SUPPORT
static INT ap_6g_cfg_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	pap_6g_cfg pap_cfg = &wdev->ap6g_cfg;
	pdiscov_iob dsc_iob = &pap_cfg->dsc_iob;

	NdisZeroMemory(pap_cfg, sizeof(ap_6g_cfg));

#ifdef DOT11V_MBSSID_SUPPORT
	/* disable discovery frame of non-transmitted bssid */
	if (VALID_MBSS(pAd, wdev->func_idx)
		&& (IS_BSSID_11V_NON_TRANS(&pAd->ApCfg.MBSSID[wdev->func_idx].mbss_11v)))
		in_band_discovery_update_oper(wdev, UNSOLICIT_TX_DISABLE, 0, UNSOLICIT_TXMODE_NON_HT);
#endif

	/* 6g: in-of-band buf */
	if (!dsc_iob->pkt_buf) {
		NdisAllocateSpinLock(pAd, &dsc_iob->pkt_lock);
		Status = MlmeAllocateMemory(pAd, (PVOID)&dsc_iob->pkt_buf);
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
				 "alloc discovery pkt_buf 0x%p\n", dsc_iob->pkt_buf);
		if (Status == NDIS_STATUS_FAILURE)
			return FALSE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR,
		"discovery pkt_buf allocated!\n");
	}

	return TRUE;
}

static INT ap_6g_cfg_deinit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	pap_6g_cfg pap_cfg = &wdev->ap6g_cfg;
	pdiscov_iob dsc_iob = &pap_cfg->dsc_iob;

	/* de-initial for 6g in-of-band buf */
	if (dsc_iob->pkt_buf) {
		OS_SEM_LOCK(&dsc_iob->pkt_lock);
		MlmeFreeMemory(dsc_iob->pkt_buf);
		dsc_iob->pkt_buf = NULL;
		OS_SEM_UNLOCK(&dsc_iob->pkt_lock);

		NdisFreeSpinLock(&dsc_iob->pkt_lock);
	}

	return TRUE;
}

static UCHAR frame_offload_iob_type_map[] = {
	BSSINFO_UNSOLICIT_TX_FILS_DISC, /*UNSOLICIT_TX_DISABLE, default FILS in 6G cert.*/
	BSSINFO_UNSOLICIT_TX_PROBE_RSP, /*UNSOLICIT_TX_PROBE_RSP*/
	BSSINFO_UNSOLICIT_TX_FILS_DISC, /*UNSOLICIT_TX_FILS_DISC*/
};

INT ap_6g_set_frame_offload(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	pdiscov_iob dsc_iob = &wdev->ap6g_cfg.dsc_iob;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 u2Length = 0;
	UINT16 bmc_wcid = wdev->bss_info_argument.bmc_wlan_idx;
	BOOLEAN enable;
	UINT8 iob_type = wlan_operate_get_unsolicit_tx_type(wdev);
	UINT8 ucTxInterval = wlan_operate_get_unsolicit_tx_tu(wdev);
	UINT8 ucTxType, ucTxMode;

	/* fw only support SU */
	ucTxMode = PROBE_RSP_TX_MODE_SU;

	if ((iob_type != UNSOLICIT_TX_DISABLE) && (dsc_iob->pkt_buf)
		&& (iob_type < ARRAY_SIZE(frame_offload_iob_type_map))) {
		ucTxType = frame_offload_iob_type_map[iob_type];
		u2Length = cap->tx_hw_hdr_len + dsc_iob->pkt_len;
		enable = true;
	} else {
		ucTxType	= BSSINFO_UNSOLICIT_TX_FILS_DISC;
		u2Length = 0;
		enable = false;
	}

	HW_SET_FRAME_OFFLOAD(pAd, wdev->wdev_idx, u2Length,
		enable, bmc_wcid, ucTxType, ucTxMode, ucTxInterval, dsc_iob->pkt_buf);

	return TRUE;
}

ULONG ap_6g_build_unsol_bc_probe_rsp(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT8 *f_buf)
{
	HEADER_802_11 hdr;
	ULONG frame_len = 0;
	LARGE_INTEGER FakeTimestamp;
	BSS_STRUCT *mbss = wdev->func_dev;
	struct legacy_rate *rate = &wdev->rate.legacy_rate;
	UCHAR SsidLen = 0;

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_DISC, DBG_LVL_INFO,
		"wdev(%d) f_buf = 0x%p\n", wdev->wdev_idx, f_buf);

	MgtMacHeaderInit(pAd, &hdr, SUBTYPE_PROBE_RSP, 0, BROADCAST_ADDR,
						 wdev->if_addr, wdev->bssid);
	if (!mbss->bHideSsid)
		SsidLen = mbss->SsidLen;
	MakeOutgoingFrame(f_buf,					  &frame_len,
					sizeof(HEADER_802_11),	  &hdr,
					TIMESTAMP_LEN,			  &FakeTimestamp,
					2,						  &pAd->CommonCfg.BeaconPeriod,
					2,						  &mbss->CapabilityInfo,
					1,						  &SsidIe,
					1,						  &SsidLen,
					SsidLen,				  mbss->Ssid,
					END_OF_ARGS);

	frame_len += build_support_rate_ie(wdev, rate->sup_rate, rate->sup_rate_len, f_buf + frame_len);

#ifdef CONFIG_HOTSPOT_R2
	if ((mbss->HotSpotCtrl.HotSpotEnable == 0) && (mbss->HotSpotCtrl.bASANEnable == 1) &&
		(IS_AKM_WPA2_Entry(wdev))) {
		/* replace RSN IE with OSEN IE if it's OSEN wdev */
		ULONG temp_len = 0;
		UCHAR RSNIe = IE_WPA;
		extern UCHAR			OSEN_IE[];
		extern UCHAR			OSEN_IELEN;

		MakeOutgoingFrame(f_buf + frame_len,			&temp_len,
						  1,							&RSNIe,
						  1,							&OSEN_IELEN,
						  OSEN_IELEN,					OSEN_IE,
						  END_OF_ARGS);
		frame_len += temp_len;
	} else
#endif /* CONFIG_HOTSPOT_R2 */
		ComposeBcnPktTail(pAd, wdev, &frame_len, f_buf, FALSE);

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_DISC, DBG_LVL_INFO,
		"Build BC_PROBE_RSP, Len = %ld\n", frame_len);

	return frame_len;
}

ULONG ap_6g_build_fils_discovery(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT8 *f_buf)
{
	ULONG frame_len = 0;

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_DISC, DBG_LVL_INFO,
		"wdev(%d) f_buf = 0x%p\n", wdev->wdev_idx, f_buf);

	frame_len = build_fils_discovery_action(wdev, f_buf);

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_DISC, DBG_LVL_INFO,
		"Build FILS_DISCOVERY, Len = %ld\n", frame_len);

	return frame_len;
}

ULONG ap_6g_build_qos_null_injector(struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev, UINT8 *f_buf)
{
	ULONG frame_len = 0;

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_DISC, DBG_LVL_INFO,
		"wdev(%d) f_buf = 0x%p\n", wdev->wdev_idx, f_buf);

	frame_len = build_qos_null_injector(wdev, f_buf);

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_DISC, DBG_LVL_INFO,
		"Build QOS_NULL, Len = %ld\n", frame_len);

	return frame_len;
}

NDIS_STATUS ap_6g_build_discovery_frame(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev)
{
	pdiscov_iob dsc_iob = &wdev->ap6g_cfg.dsc_iob;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	PUCHAR pkt = NULL;
	ULONG frame_len = 0;
	UCHAR type, sub_type;
	union _HTTRANSMIT_SETTING TransmitSet = {.word = 0};   /* MGMT frame PHY rate setting when operatin at HT rate. */
	UCHAR iob_type = wlan_operate_get_unsolicit_tx_type(wdev);
	UCHAR iob_mode = wlan_operate_get_unsolicit_tx_mode(wdev);

	if (!dsc_iob->pkt_buf) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_DISC, DBG_LVL_ERROR,
				 "pkt_buf = NULL!\n");
		return NDIS_STATUS_FAILURE;
	}

	pkt = (UCHAR *)(dsc_iob->pkt_buf + tx_hw_hdr_len);

	OS_SEM_LOCK(&dsc_iob->pkt_lock);

	switch (iob_type) {
	case UNSOLICIT_TX_PROBE_RSP:
		type =		FC_TYPE_MGMT;
		sub_type =	SUBTYPE_PROBE_RSP;
		frame_len =	ap_6g_build_unsol_bc_probe_rsp(pAd, wdev, pkt);
		break;
	case UNSOLICIT_TX_FILS_DISC:
		type =		FC_TYPE_MGMT;
		sub_type =	SUBTYPE_ACTION;
		frame_len =	ap_6g_build_fils_discovery(pAd, wdev, pkt);
		break;
	default:
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_DISC, DBG_LVL_INFO, "discovery frame disabled (%d)\n", iob_type);
		break;
	}

	if (frame_len) {
		/* fixed-rate option */
		if (iob_mode == UNSOLICIT_TXMODE_NON_HT_DUP) {
			if (pAd->CommonCfg.wifi_cert)
				TransmitSet.field.BW = BW_80;
			else {
#ifdef CONFIG_6G_AFC_SUPPORT
				if (is_afc_in_run_state(pAd)
					&& pAd->CommonCfg.AfcSpBwDup == FALSE)
					TransmitSet.field.BW = BW_20; /* STD Power*/
				else
#endif /*CONFIG_6G_AFC_SUPPORT*/
					TransmitSet.field.BW = BW_80; /* LPI Power*/
			}
			TransmitSet.field.MODE	= MODE_OFDM;
			TransmitSet.field.MCS	= MCS_RATE_6;
		} else {
			TransmitSet.field.BW	= BW_20;
			TransmitSet.field.MODE	= MODE_OFDM;
			TransmitSet.field.MCS	= MCS_RATE_6;
		}

		write_tmac_info_offload_pkt(pAd, wdev,
									type, sub_type,
									dsc_iob->pkt_buf,
									&TransmitSet,
									frame_len);
	}
	dsc_iob->pkt_len = frame_len;

	OS_SEM_UNLOCK(&dsc_iob->pkt_lock);

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_DISC, DBG_LVL_INFO,
		"%s, Type(%d), Len(%ld), mode:%d, mcs:%d\n",
		__func__, iob_type, frame_len, TransmitSet.field.MODE, TransmitSet.field.MCS);

	return NDIS_STATUS_SUCCESS;
}
#endif

/*
	==========================================================================
	Description:
		Start AP service. If any vital AP parameter is changed, a STOP-START
		sequence is required to disassociate all STAs.

	IRQL = DISPATCH_LEVEL.(from SetInformationHandler)
	IRQL = PASSIVE_LEVEL. (from InitializeHandler)

	Note:
		Can't call NdisMIndicateStatus on this routine.

		RT61 is a serialized driver on Win2KXP and is a deserialized on Win9X
		Serialized callers of NdisMIndicateStatus must run at IRQL = DISPATCH_LEVEL.

	==========================================================================
 */

VOID APStartUpForMbss(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	struct wifi_dev *wdev = &pMbss->wdev;
	BOOLEAN bWmmCapable = FALSE;
	EDCA_PARM *pEdca = NULL, *pBssEdca = NULL;
	USHORT phy_mode = pAd->CommonCfg.cfg_wmode;
	UCHAR ucBandIdx = 0;
	UINT8 wifi_cert_program = 0;
#if defined(TX_POWER_CONTROL_SUPPORT) || defined(TX_POWER_CONTROL_SUPPORT_V2)
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
#endif

	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_WARN, "===>(caller:%pS), mbss_idx:%d, CfgMode:%d\n", OS_TRACE, pMbss->mbss_idx, phy_mode);

	ucBandIdx = hc_get_hw_band_idx(pAd);
#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_LINK_UP, ucBandIdx);
#endif /* LED_CONTROL_SUPPORT */

#ifdef ANTENNA_CONTROL_SUPPORT
	Antenna_Control_Init(pAd, wdev);
#endif /* ANTENNA_CONTROL_SUPPORT */

	/*Ssid length sanity check.*/
	if ((pMbss->SsidLen <= 0) || (pMbss->SsidLen > MAX_LEN_OF_SSID)) {
		NdisMoveMemory(pMbss->Ssid, "HT_AP", 5);
		pMbss->Ssid[5] = '0' + pMbss->mbss_idx;
		pMbss->SsidLen = 6;
	}

	if (wdev->func_idx == 0)
		MgmtTableSetMcastEntry(pAd, MCAST_WCID_TO_REMOVE, wdev);

	APSecInit(pAd, wdev);

#ifdef MWDS
	if (wdev->bDefaultMwdsStatus == TRUE)
		MWDSEnable(pAd, pMbss->mbss_idx, TRUE, TRUE);
#endif /* MWDS */

#if defined(CONFIG_MAP_SUPPORT) && defined(A4_CONN)
	if (IS_MAP_ENABLE(pAd))
		map_a4_init(pAd, pMbss->mbss_idx, TRUE);
#endif
#if defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)
	/* Deduce IPv6 Link local address for this MBSS & IPv6 format checksum for use in MLD query message*/
	calc_mldv2_gen_query_chksum(pAd, pMbss);
#endif

#if defined(WAPP_SUPPORT)
	wapp_init(pAd, pMbss);
#endif

	ap_mlme_set_capability(pAd, pMbss);
#ifdef WSC_V2_SUPPORT

	if (wdev->WscControl.WscV2Info.bEnableWpsV2) {
		/*
		    WPS V2 doesn't support Chiper WEP and TKIP.
		*/
		struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

		if (IS_CIPHER_WEP_TKIP_ONLY(pSecConfig->PairwiseCipher)
			|| (pMbss->bHideSsid))
			WscOnOff(pAd, wdev->func_idx, TRUE);
		else
			WscOnOff(pAd, wdev->func_idx, FALSE);
	}

#endif /* WSC_V2_SUPPORT */

	/* If any BSS is WMM Capable, we need to config HW CRs */
	if (wdev->bWmmCapable)
		bWmmCapable = TRUE;

	if ((WMODE_CAP_N(wdev->PhyMode) || bWmmCapable)
		&& (wdev->EdcaIdx < WMM_NUM)) {
		pEdca = &pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx];

		/* EDCA parameters used for AP's own transmission */
		if (pEdca->bValid == FALSE)
			set_default_ap_edca_param(pEdca);

		pBssEdca = wlan_config_get_ht_edca(wdev);

		if (pBssEdca) {
			/* EDCA parameters to be annouced in outgoing BEACON, used by WMM STA */
			if (pBssEdca->bValid == FALSE)
				set_default_sta_edca_param(pBssEdca);
		}
	}

#ifdef DOT11_N_SUPPORT
#ifdef EXT_BUILD_CHANNEL_LIST
	if (WMODE_CAP_6G(wdev->PhyMode))
		BuildChannelList(pAd, wdev);
	else
		BuildChannelListEx(pAd, wdev);
#else
	BuildChannelList(pAd, wdev);
#endif

	/*update rate info for wdev*/
#ifdef GN_MIXMODE_SUPPORT
	if (pAd->CommonCfg.GNMixMode
		&& (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
			|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)
			|| WMODE_EQUAL(wdev->PhyMode, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G))))
		RTMPUpdateGNRateInfo(wdev->PhyMode, &wdev->rate);
	else
#endif /*GN_MIXMODE_SUPPORT*/
		RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
		rtmpeapupdaterateinfo(wdev->PhyMode, &wdev->rate, &wdev->eap);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

	RTMPSetPhyMode(pAd, wdev, wdev->PhyMode);

	if (!(WMODE_CAP_N(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode)))
		wlan_config_set_ht_bw(wdev, HT_BW_20);

#ifdef DOT11N_DRAFT3
	/*
	    We only do this Overlapping BSS Scan when system up, for the
	    other situation of channel changing, we depends on station's
	    report to adjust ourself.
	*/

	if (pAd->CommonCfg.bForty_Mhz_Intolerant == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
				 "Disable 20/40 BSSCoex Channel Scan(BssCoex=%d, 40MHzIntolerant=%d)\n",
				  pAd->CommonCfg.bBssCoexEnable,
				  pAd->CommonCfg.bForty_Mhz_Intolerant);
	} else if (pAd->CommonCfg.bBssCoexEnable == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
			"Enable 20/40 BSSCoex Channel Scan(BssCoex=%d)\n", pAd->CommonCfg.bBssCoexEnable);
		APOverlappingBSSScan(pAd, wdev);
	}

	if (pAd->CommonCfg.wifi_cert) {
		struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);

		wifi_cert_program = pAd->CommonCfg.wifi_cert; /* Please follow the program enum in capi.h */
#ifdef WIFI_UNIFIED_COMMAND
		if (chip_cap->uni_cmd_support)
			UniCmdCfgInfoUpdate(pAd, wdev, CFGINFO_CERT_CFG_FEATURE, &wifi_cert_program);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_CERT_CFG_FEATURE, &wifi_cert_program);

		/* respond with three Probe response per request */
		chip_cap->ProbeRspTimes = 3;
	}

#ifdef WIFI_MD_COEX_SUPPORT
	if (pAd->CommonCfg.powerBackoff != 0)
		MtCmdIdcSetTxPwrLimit(pAd, pAd->CommonCfg.powerBackoff);
#endif
#endif /* DOT11N_DRAFT3 */
#endif /*DOT11_N_SUPPORT*/
	MlmeUpdateTxRates(pAd, FALSE, wdev->func_idx);
#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode))
		MlmeUpdateHtTxRates(pAd, wdev);

#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_EHT_BE
	if (WMODE_CAP_BE(wdev->PhyMode)) {
		UINT8 mld_type;
		UINT8 *mld_addr;
		UCHAR multilink_mld_addr[MAC_ADDR_LEN];
		struct eht_mld_param mld_param = {0};

		/* MLD MAC addr of single-link MLD = single-link link MAC addr */
		mld_addr = (BMGR_IS_ML_MLD_GRP_IDX(pMbss->mld_grp_idx) && pMbss->mld_addr_by_cfg) ?
			pMbss->pf_mld_addr : wdev->bssid;

		/* generate unique mld mac address for mbss */
		memcpy(pMbss->multilink_mld_addr, wdev->if_addr, MAC_ADDR_LEN);
		pMbss->multilink_mld_addr[0] |= 0x2; /*local bit enable*/
		if (pMbss->multilink_mld_addr[0]&0x20)
			pMbss->multilink_mld_addr[0] &= ~0x20;
		else
			pMbss->multilink_mld_addr[0] |= 0x20;

		/* get available mld mac from MBSS list */
		if (BMGR_IS_ML_MLD_GRP_IDX(pMbss->mld_grp_idx) &&
			!pMbss->mld_addr_by_cfg &&
			pMbss->unique_mld_addr_enable) {
			if (eht_ap_mld_available_address_search(wdev,
				pMbss->mld_grp_idx, multilink_mld_addr, BMGR_MLD_TYPE_MULTI)){
				mld_addr = multilink_mld_addr;
			}
		}

		eht_ap_mld_fill_mld_param_from_selected_link(wdev, &mld_param);

		if (eht_ap_mld_create(wdev, &pMbss->mld_grp_idx,
				mld_addr, &mld_param, &mld_type) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR,
				"EHT AP MLD Create Failed! (grp:%d, addr:%pM)\n",
				pMbss->mld_grp_idx, mld_addr);
		}
	}
#endif /* DOT11_EHT_BE*/

	if (WDEV_WITH_BCN_ABILITY(wdev) && wdev->bAllowBeaconing) {
		if (wdev_do_linkup(wdev, NULL) != TRUE)
			MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR, "link up fail!!\n");
	}

#ifdef SINGLE_SKU_V2
	/* enable/disable SKU via profile */
	TxPowerSKUCtrl(pAd, pAd->CommonCfg.SKUenable, ucBandIdx);

	/* enable/disable BF Backoff via profile */
	TxPowerBfBackoffCtrl(pAd, pAd->CommonCfg.BFBACKOFFenable, ucBandIdx);
#endif /* SINGLE_SKU_V2*/
	/* enable/disable Power Percentage via profile */
	TxPowerPercentCtrl(pAd, pAd->CommonCfg.PERCENTAGEenable, ucBandIdx);

	/* Tx Power Percentage value via profile */
	TxPowerDropCtrl(pAd, pAd->CommonCfg.ucTxPowerPercentage, ucBandIdx);

	/* Config Tx CCK Stream */
	TxCCKStreamCtrl(pAd, pAd->CommonCfg.CCKTxStream, ucBandIdx);

#if defined(TX_POWER_CONTROL_SUPPORT) || defined(TX_POWER_CONTROL_SUPPORT_V2)
	if (arch_ops && arch_ops->arch_txpower_boost)
		arch_ops->arch_txpower_boost(pAd, ucBandIdx);
#endif /* TX_POWER_CONTROL_SUPPORT */

	/*init tx/rx stream */
	hc_set_rrm_init(wdev);

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	/* Update bInitMbssZeroWait for MBSS Zero Wait */
	UPDATE_MT_INIT_ZEROWAIT_MBSS(pAd, FALSE);
#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */
#ifdef VOW_SUPPORT
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
		if (VALID_MBSS(pAd, wdev->func_idx) &&
			(pAd->ApCfg.MBSSID[wdev->func_idx].APStartPseduState == AP_STATE_ALWAYS_START_AP_DEFAULT))
			vow_mbss_init(pAd, wdev);
#else
	vow_mbss_init(pAd, wdev);
#endif
#endif /* VOW_SUPPORT */

#ifdef GREENAP_SUPPORT
	greenap_check_when_ap_bss_change(pAd);
#endif /* GREENAP_SUPPORT */

#ifdef BAND_STEERING
#ifdef CONFIG_AP_SUPPORT
	if (pAd->ApCfg.BandSteering)
		BndStrg_SetInfFlags(pAd, &pMbss->wdev, &pAd->ApCfg.BndStrgTable, TRUE);
#ifdef SPECIAL_11B_OBW_FEATURE
	if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G)
		MtCmdSetTxTdCck(pAd, TRUE);
#endif /* SPECIAL_11B_OBW_FEATURE */
#endif /* CONFIG_AP_SUPPORT */
#endif /* BAND_STEERING */

#ifdef CONFIG_DOT11U_INTERWORKING
	pMbss->GASCtrl.b11U_enable = 1;
#endif /* CONFIG_DOT11U_INTERWORKING */

	pMbss->ShortSSID = Crcbitbybitfast(pMbss->Ssid, pMbss->SsidLen);

#ifdef OCE_SUPPORT
	if (IS_OCE_ENABLE(wdev) && VALID_MBSS(pAd, wdev->func_idx)) {
		BOOLEAN Cancelled;
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

		if (IS_OCE_FD_FRAME_ENABLE(wdev) &&
			IS_FD_FRAME_FW_MODE(cap) &&
			(wdev->func_idx >= 0) && (wdev->func_idx < MAX_BEACON_NUM)) {
			OceSendFilsDiscoveryAction(pAd, &pAd->ApCfg.MBSSID[wdev->func_idx].wdev);
		}

		if (IS_OCE_RNR_ENABLE(wdev))
			RTMPSetTimer(&pAd->ApCfg.APAutoScanNeighborTimer, OCE_RNR_SCAN_PERIOD);
		else
			RTMPCancelTimer(&pAd->ApCfg.APAutoScanNeighborTimer, &Cancelled);
	}
#endif /* OCE_SUPPORT */

#ifdef CONFIG_MAP_SUPPORT
#if defined(WAPP_SUPPORT)
	wapp_send_bss_state_change(pAd, wdev, WAPP_BSS_START);
	wapp_send_wdev_info_report(pAd, wdev);
#endif /*WAPP_SUPPORT*/
#endif /* CONFIG_MAP_SUPPORT */

#ifdef CFG_SUPPORT_FALCON_MURU
	/* muru_tam_arb_op_mode(pAd); */
	muru_update_he_cfg(pAd);
#endif
#ifdef CFG_SUPPORT_FALCON_SR
	/* Spatial Reuse initialize via profile */
	SrMbssInit(pAd, wdev);
#endif /* CFG_SUPPORT_FALCON_SR */

#ifdef CFG_SUPPORT_FALCON_PP
	/* Preamble puncture initialize via profile */
	pp_mbss_init(pAd, wdev);
#endif /* CFG_SUPPORT_FALCON_SR */

	Set_CpuUtilEn_Proc(pAd, "1");
#ifdef DSCP_PRI_SUPPORT
	/*write CR4 for DSCP user prio and flag*/
	if (pMbss->dscp_pri_map_enable)
		MtCmdSetDscpPri(pAd, pMbss->mbss_idx, INT_MBSSID);
#endif /*DSCP_PRI_SUPPORT*/

	/* EDCCA mode init */
	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "===>edcca initial\n");
	EDCCAInit(pAd, ucBandIdx);

#ifdef PRE_CFG_SUPPORT
	pre_cfg_interface_init(pAd, pMbss->mbss_idx, TRUE);
#endif /* PRE_CFG_SUPPORT */

	TxPowerLimitTypeInit(pAd, pAd->CommonCfg.LpiEn);
	MtCmdTxPwrShowInfo(pAd, TXPOWER_ALL_RATE_POWER_INFO, ucBandIdx);
#ifdef MGMT_TXPWR_CTRL
	/* read base pwr for CCK/OFDM rate */
	wdev->bPwrCtrlEn = FALSE;
	wdev->TxPwrDelta = 0;
	wdev->mgmt_txd_txpwr_offset = 0;
	pAd->ApCfg.MgmtTxPwr = 0;

	/* Update beacon/probe TxPwr wrt profile param */
	if (wdev->MgmtTxPwr) {
		/* wait until TX Pwr event rx*/
		if (!(pAd->ApCfg.MgmtTxPwr))
			RtmpusecDelay(50);

		update_mgmt_frame_power(pAd, wdev);
	}
#endif

#ifdef SW_CONNECT_SUPPORT
	if (WMODE_CAP_2G(wdev->PhyMode)) {
		pAd->dummy_obj.bFixedRateSet = TRUE;
		pAd->dummy_obj.fr_tbl_idx = FR_CCK_11M;
		pAd->dummy_obj.fr_bw = BW_20;
	} else if (WMODE_CAP_5G(wdev->PhyMode)) {
		pAd->dummy_obj.bFixedRateSet = TRUE;
		pAd->dummy_obj.fr_tbl_idx = FR_OFDM_6M;
		pAd->dummy_obj.fr_bw = BW_20;
	} else if (WMODE_CAP_6G(wdev->PhyMode)) {
		pAd->dummy_obj.bFixedRateSet = TRUE;
		pAd->dummy_obj.fr_tbl_idx = FR_OFDM_6M;
		pAd->dummy_obj.fr_bw = BW_20;
	}

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
		"wdev_idx=%d, fr_tbl_idx=%d, fr_bw=%d\n",
		wdev->wdev_idx, pAd->dummy_obj.fr_tbl_idx, pAd->dummy_obj.fr_bw);
#endif /* SW_CONNECT_SUPPORT */
#ifdef CCN67_BS_SUPPORT
	pMbss->iv_bb_rssi_thresh = 20;
#endif

	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "<===mbss_idx %d, pNetDev = %p\n",
			 pMbss->mbss_idx, wdev->if_dev);
}

VOID APStartUpByBss(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	struct wifi_dev *wdev;

	if (pMbss == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR, "Invalid Mbss\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "===>mbss_idx %d\n", pMbss->mbss_idx);
	wdev = &pMbss->wdev;

	/* setup tx preamble */
	MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble);
	/* Clear BG-Protection flag */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);

	/* Update devinfo for any phymode change */
	if (wdev_do_open(wdev) != TRUE)
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR, "open fail!!!\n");

	ApLogEvent(pAd, pAd->CurrentAddress, EVENT_RESET_ACCESS_POINT);
	pAd->Mlme.PeriodicRound = 0;
	pAd->Mlme.OneSecPeriodicRound = 0;
	pAd->MacTab->MsduLifeTime = 5; /* default 5 seconds */
	pAd->BcnCheckInfo.BcnInitedRnd = pAd->Mlme.PeriodicRound;
	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

	/* start sending BEACON out */
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
		if (pMbss && (pMbss->APStartPseduState == AP_STATE_ALWAYS_START_AP_DEFAULT))
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */
			UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IF_STATE_CHG));
	/*
		Set group re-key timer if necessary.
		It must be processed after clear flag "fRTMP_ADAPTER_HALT_IN_PROGRESS"
	*/
	APStartRekeyTimer(pAd, wdev);

	/* Pre-tbtt interrupt setting. */
	AsicSetPreTbtt(pAd, TRUE, HW_BSSID_0);
#ifdef IDS_SUPPORT

	/* Start IDS timer */
	if (pAd->ApCfg.IdsEnable) {
#ifdef SYSTEM_LOG_SUPPORT

		if (pAd->CommonCfg.bWirelessEvent == FALSE)
			MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
					 "!!! WARNING !!! The WirelessEvent parameter doesn't be enabled\n");

#endif /* SYSTEM_LOG_SUPPORT */
		RTMPIdsStart(pAd);
	}

#endif /* IDS_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_LINK_UP, HcGetBandByWdev(wdev));
#endif /* LED_CONTROL_SUPPORT */

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);
	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "Main bssid = "MACSTR"\n",
			 MAC2STR(pAd->ApCfg.MBSSID[BSS0].wdev.bssid));
	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "<===\n");
}

VOID ap_over_lapping_scan(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	struct wifi_dev *wdev;
	if (pMbss == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "Invalid Mbss\n");
		return;
	}
	wdev = &pMbss->wdev;
	if (pAd->CommonCfg.bForty_Mhz_Intolerant == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"Disable 20/40 BSSCoex Channel Scan(BssCoex=%d, 40MHzIntolerant=%d)\n",
				 pAd->CommonCfg.bBssCoexEnable,
				 pAd->CommonCfg.bForty_Mhz_Intolerant);
	} else if (pAd->CommonCfg.bBssCoexEnable == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"Enable 20/40 BSSCoex Channel Scan(BssCoex=%d)\n",
			pAd->CommonCfg.bBssCoexEnable);
		APOverlappingBSSScan(pAd, wdev);
	}
}
VOID APStartUp(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, ENUM_AP_BSS_OPER oper)
{
	UINT32 idx;
	BSS_STRUCT *pCurMbss = NULL;
	UCHAR ch = 0;
#ifdef CONFIG_MAP_SUPPORT
	UCHAR i;
	struct DOT11_H *pDot11h = NULL;
	struct wifi_dev *wdev_temp = NULL;
#ifdef CONFIG_STA_SUPPORT
	UCHAR j;
	struct wifi_dev *sta_wdev = NULL;
#endif /* CONFIG_STA_SUPPORT */
#endif

	/* Don't care pMbss if operation is for all */
	if ((pMbss == NULL) && (oper != AP_BSS_OPER_ALL)) {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR, "Invalid oper(%d)\n", oper);
		return;
	}

	if (pMbss)
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "(caller:%pS), oper(%d) bssid(%d)="MACSTR"\n",
				 OS_TRACE, oper, pMbss->mbss_idx, MAC2STR(pMbss->wdev.bssid));
	else
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR, "(caller:%pS), oper(%d)\n",
				 OS_TRACE, oper);

	switch (oper) {
	case AP_BSS_OPER_ALL:
		for (idx = 0; ((idx < pAd->ApCfg.BssidNum) && (idx < MAX_MBSSID_NUM(pAd))); idx++) {
			pCurMbss = &pAd->ApCfg.MBSSID[idx];
			pCurMbss->mbss_idx = idx;

			/* check MBSS status is up */
			if (!pCurMbss->wdev.if_up_down_state)
				continue;

			APStartUpByBss(pAd, pCurMbss);
		}
		break;
	case AP_BSS_OPER_BY_RF:
		ch = pMbss->wdev.channel;
		for (idx = 0; ((idx < pAd->ApCfg.BssidNum) && (idx < MAX_MBSSID_NUM(pAd))); idx++) {
			pCurMbss = &pAd->ApCfg.MBSSID[idx];
			pCurMbss->mbss_idx = idx;

			/* check MBSS status is up */
			if (!pCurMbss->wdev.if_up_down_state)
				continue;

			/* check MBSS work on the same RF(channel) */
			if (pCurMbss->wdev.channel != ch)
				continue;
#ifdef CONFIG_MAP_SUPPORT
			if (pCurMbss->is_bss_stop_by_map)
				continue;
#endif
			APStartUpByBss(pAd, pCurMbss);
		}
		break;

	case AP_BSS_OPER_SINGLE:
	default:
		APStartUpByBss(pAd, pMbss);
		if (pMbss->wdev.if_up_down_state && pMbss->wdev.channel < 14)
			ap_over_lapping_scan(pAd, pMbss);
		break;
	}
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);
#ifdef CONFIG_MAP_SUPPORT
/*If this AP stop start is due to radar detect then send the radar notify event to MAP*/
	if (IS_MAP_TURNKEY_ENABLE(pAd) && pMbss) {
		ch = pMbss->wdev.channel;
		for (i = 0; ((i < pAd->ApCfg.BssidNum) && (i < MAX_MBSSID_NUM(pAd))); i++) {
			wdev_temp = &pAd->ApCfg.MBSSID[i].wdev;
			if ((wdev_temp == NULL) || (wdev_temp->pDot11_H == NULL)) {
				continue;
			}

			pDot11h = wdev_temp->pDot11_H;
			/* check MBSS status is up */
			if (!wdev_temp->if_up_down_state)
				continue;
			/* check MBSS work on the same RF(channel) */
			if (wdev_temp->channel != ch)
				continue;
			if (pDot11h->ChannelMode == CHAN_SILENCE_MODE) {
				if (wdev_temp->cac_not_required == TRUE)
					wapp_send_cac_period_event(pAd, RtmpOsGetNetIfIndex(wdev_temp->if_dev), wdev_temp->channel, 2, 0);
				else
					wapp_send_cac_period_event(pAd, RtmpOsGetNetIfIndex(wdev_temp->if_dev), wdev_temp->channel, 1, pDot11h->cac_time);
#ifdef CONFIG_STA_SUPPORT
				for (j = 0; j < MAX_APCLI_NUM; j++) {
					sta_wdev = &pAd->StaCfg[j].wdev;
					if (sta_wdev->channel == wdev_temp->channel) {
						pAd->StaCfg[j].ApcliInfStat.Enable = FALSE;
					}
				}
#endif /* CONFIG_STA_SUPPORT */
			}
			if (wdev_temp->map_radar_detect) {
				wapp_send_radar_detect_notif(pAd, wdev_temp, wdev_temp->map_radar_channel, wdev_temp->map_radar_bw, 0);
				wdev_temp->map_radar_detect = 0;
				wdev_temp->map_radar_channel = 0;
				wdev_temp->map_radar_bw = 0;
			}
#ifdef WIFI_MD_COEX_SUPPORT
			if (wdev_temp->map_lte_unsafe_ch_detect) {
				wapp_send_lte_safe_chn_event(pAd, pAd->LteSafeChCtrl.SafeChnBitmask);
				wdev_temp->map_lte_unsafe_ch_detect = 0;
			}
#endif
		}
	}
#endif
	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "<===\n");
}

/*Only first time will run it*/
VOID APInitForMain(RTMP_ADAPTER *pAd)
{
	BSS_STRUCT *pMbss = NULL;
	UINT32 apidx = 0;

	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "===>\n");
	/*reset hw wtbl*/
	ap_hw_tb_init(pAd);
	pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	pMbss->mbss_idx = MAIN_MBSSID;
	/*update main runtime attribute*/

	/* Workaround end */
	/* Clear BG-Protection flag */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
	MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble);

#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_LINK_UP, HcGetBandByWdev(&pMbss->wdev));
#endif /* LED_CONTROL_SUPPORT */
	ApLogEvent(pAd, pAd->CurrentAddress, EVENT_RESET_ACCESS_POINT);
	pAd->Mlme.PeriodicRound = 0;
	pAd->Mlme.OneSecPeriodicRound = 0;
	pAd->MacTab->MsduLifeTime = 5; /* default 5 seconds */
	pAd->BcnCheckInfo.BcnInitedRnd = pAd->Mlme.PeriodicRound;
	pAd->mcli_ctl.tx_cnt_from_red = TRUE;
	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	/*
		NOTE!!!:
			All timer setting shall be set after following flag be cleared
				fRTMP_ADAPTER_HALT_IN_PROGRESS
	*/
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
	/*
		Set group re-key timer if necessary.
		It must be processed after clear flag "fRTMP_ADAPTER_HALT_IN_PROGRESS"
	*/
	APStartRekeyTimer(pAd, &pMbss->wdev);
	/* RadarStateCheck(pAd); */

	/* Pre-tbtt interrupt setting. */
	AsicSetPreTbtt(pAd, TRUE, HW_BSSID_0);

	for (apidx = 0; ((apidx < pAd->ApCfg.BssidNum) && (apidx < MAX_MBSSID_NUM(pAd))); apidx++) {
		pMbss = &pAd->ApCfg.MBSSID[apidx];
		OPSTATUS_SET_FLAG_WDEV(&pMbss->wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	}

#ifdef IDS_SUPPORT

	/* Start IDS timer */
	if (pAd->ApCfg.IdsEnable) {
#ifdef SYSTEM_LOG_SUPPORT

		if (pAd->CommonCfg.bWirelessEvent == FALSE)
			MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
					 "!!! WARNING !!! The WirelessEvent parameter doesn't be enabled\n");

#endif /* SYSTEM_LOG_SUPPORT */
		RTMPIdsStart(pAd);
	}

#endif /* IDS_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	FT_Init(pAd);
#endif /* DOT11R_FT_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "Main bssid = "MACSTR"\n",
			 MAC2STR(pAd->ApCfg.MBSSID[BSS0].wdev.bssid));
}

VOID APStopByBss(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	BOOLEAN Cancelled;
#if defined(APCLI_SUPPORT)
	INT idx = 0;
	struct wifi_dev *wdev = NULL;
#ifdef WSC_INCLUDED
	PWSC_CTRL pWscControl;
#endif /* WSC_INCLUDED */
#endif	/* APCLI_SUPPORT */
	struct wifi_dev *wdev_bss = NULL;

	if (pMbss == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR, "Invalid Mbss\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_INFO, "===>mbss_idx %d\n", pMbss->mbss_idx);
	wdev_bss = &pMbss->wdev;

	if (wdev_bss->if_dev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR, "Invalid wdev(%d)\n", wdev_bss->wdev_idx);
		return;
	}

	/* */
	/* Cancel the Timer, to make sure the timer was not queued. */
	/* */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
#ifdef APCLI_SUPPORT
#ifdef WAPP_SUPPORT
	if (wdev_bss->avoid_apcli_linkdown != TRUE) {
#endif
	{
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		if (IS_APCLI_SYNC_PEER_DEAUTH_ENBL(pAd) == FALSE) {
#endif
			for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
				wdev = &pAd->StaCfg[idx].wdev;
#ifdef WSC_INCLUDED
				/* WPS cli will disconnect and connect again */
				pWscControl = &pAd->StaCfg[idx].wdev.WscControl;
				if (pWscControl->bWscTrigger == TRUE)
					continue;
#endif /* WSC_INCLUDED */
				if (wdev->channel == wdev_bss->channel) {
					UINT8 enable = pAd->StaCfg[idx].ApcliInfStat.Enable;

					if (enable) {
						pAd->StaCfg[idx].ApcliInfStat.Enable = FALSE;
						ApCliIfDown(pAd);
						pAd->StaCfg[idx].ApcliInfStat.Enable = enable;
					}
				}
			}
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		}
#endif
	}
#ifdef WAPP_SUPPORT
	}
#endif
#endif /* APCLI_SUPPORT */

#ifdef WIFI_TWT_SUPPORT
	twt_release_btwt_resource(wdev_bss);
#endif /* WIFI_TWT_SUPPORT */

	/*AP mode*/
	/*For Tgac 4.2.16h test sniffer do check, after CSA AP should*/
	/*not send further beacon with/without CSAIE even if CSA IE*/
	/*is updated in beacon template after firmware send the CSA*/
	/*done event in next preTBTT interupt beacon queue in firmware*/
	/*get enabled so first diabling the queue then update the IE*/
	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_INFO, "===>Disabling BCN\n");
	UpdateBeaconHandler(pAd, wdev_bss, BCN_REASON(BCN_UPDATE_DISABLE_TX));

	MacTableResetWdev(pAd, wdev_bss);
	OPSTATUS_CLEAR_FLAG_WDEV(wdev_bss, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

#ifdef GREENAP_SUPPORT
	greenap_check_when_ap_bss_change(pAd);
#endif /* GREENAP_SUPPORT */

	CMDHandler(pAd);
	/* Disable pre-tbtt interrupt */
	AsicSetPreTbtt(pAd, FALSE, HW_BSSID_0);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
#ifdef LED_CONTROL_SUPPORT
		/* Set LED */
		RTMPSetLED(pAd, LED_LINK_DOWN, HcGetBandByWdev(wdev_bss));
#endif /* LED_CONTROL_SUPPORT */
	}

#ifdef MWDS
	MWDSDisable(pAd, pMbss->mbss_idx, TRUE, TRUE);
#endif /* MWDS */

#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd))
		map_a4_deinit(pAd, pMbss->mbss_idx, TRUE);
#endif

	/* clear protection to default */
	wdev_bss->protection = 0;

	/* clear csa cnt */
	wdev_bss->csa_count = 0;

#ifdef CONFIG_RCSA_SUPPORT
	/* When RCSA is send, ALTX is en and BF suspended, restore state*/
	rcsa_recovery(pAd, wdev_bss);
#endif

#ifdef PRE_CFG_SUPPORT
	pre_cfg_interface_deinit(pAd, pMbss->mbss_idx, TRUE);
#endif /* PRE_CFG_SUPPORT */

	if (wdev_do_linkdown(wdev_bss) != TRUE)
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR, "linkdown fail!!!\n");

	if (WMODE_CAP_5G(wdev_bss->PhyMode)) {
#ifdef MT_DFS_SUPPORT /* Jelly20150217 */
		WrapDfsRadarDetectStop(pAd);
		/* Zero wait hand off recovery for CAC period + interface down case */
		DfsZeroHandOffRecovery(pAd, wdev_bss);
#endif
	}

	/* Update devinfo for any phymode change */
	if (wdev_do_close(wdev_bss) != TRUE)
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR, "close fail!!!\n");

	APReleaseRekeyTimer(pAd, wdev_bss);
#ifdef BAND_STEERING
	if (pAd->ApCfg.BandSteering)
		BndStrg_SetInfFlags(pAd, &pMbss->wdev, &pAd->ApCfg.BndStrgTable, FALSE);
#endif /* BAND_STEERING */

	if (pAd->ApCfg.CMTimerRunning == TRUE) {
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = FALSE;
		pAd->ApCfg.BANClass3Data = FALSE;
	}

#ifdef IDS_SUPPORT
	/* if necessary, cancel IDS timer */
	RTMPIdsStop(pAd);
#endif /* IDS_SUPPORT */
#ifdef VOW_SUPPORT
	vow_reset(pAd);
#endif /* VOW_SUPPORT */
#ifdef OCE_SUPPORT
	OceDeInit(pAd, wdev_bss);
#endif /* OCE_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
#if defined(WAPP_SUPPORT)
	wapp_send_bss_state_change(pAd, wdev_bss, WAPP_BSS_STOP);
#endif /*WAPP_SUPPORT*/
#endif /*CONFIG_MAP_SUPPORT WAPP_SUPPORT*/

	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_INFO, "<===mbss_idx %d\n", pMbss->mbss_idx);

}

VOID APStop(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, ENUM_AP_BSS_OPER oper)
{
	UINT32 idx;
	BSS_STRUCT *pCurMbss = NULL;
	UCHAR ch = 0;

	/* Don't care pMbss if operation is for all */
	if ((pMbss == NULL) && (oper != AP_BSS_OPER_ALL)) {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR, "Invalid oper(%d)\n", oper);
		return;
	}

	if (pMbss)
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_INFO, "(caller:%pS), oper(%d) bssid(%d)="MACSTR"\n",
				 OS_TRACE, oper, pMbss->mbss_idx, MAC2STR(pMbss->wdev.bssid));
	else
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR, "(caller:%pS), oper(%d)\n",
				 OS_TRACE, oper);

	switch (oper) {
	case AP_BSS_OPER_ALL:
		for (idx = 0; ((idx < pAd->ApCfg.BssidNum) && (idx < MAX_MBSSID_NUM(pAd))); idx++) {
			pCurMbss = &pAd->ApCfg.MBSSID[idx];
			pCurMbss->mbss_idx = idx;

			APStopByBss(pAd, pCurMbss);
#ifdef CONFIG_6G_AFC_SUPPORT
			if ((pCurMbss != NULL) && WMODE_CAP_6G(pCurMbss->wdev.PhyMode))
				afc_free_spectrum();
#endif
		}
		break;

	case AP_BSS_OPER_BY_RF:
		ch = pMbss->wdev.channel;
		for (idx = 0; ((idx < pAd->ApCfg.BssidNum) && (idx < MAX_MBSSID_NUM(pAd))); idx++) {
			pCurMbss = &pAd->ApCfg.MBSSID[idx];
			pCurMbss->mbss_idx = idx;

			/* check MBSS status is up */
			if (!pCurMbss->wdev.if_up_down_state && !pCurMbss->wdev.bcn_buf.bBcnSntReq)
				continue;

			/* check MBSS work on the same RF(channel) */
			if (pCurMbss->wdev.channel != ch)
				continue;
			APStopByBss(pAd, pCurMbss);
		}
		break;

	case AP_BSS_OPER_SINGLE:
	default:
		APStopByBss(pAd, pMbss);
		break;
	}

#ifdef CONFIG_6G_AFC_SUPPORT
	if ((pMbss != NULL) && WMODE_CAP_6G(pMbss->wdev.PhyMode))
		afc_free_spectrum();
#endif
	/* MacTableReset(pAd); */
	CMDHandler(pAd);
	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_INFO, "<===\n");
}

/*
	==========================================================================
	Description:
		This routine is used to clean up a specified power-saving queue. It's
		used whenever a wireless client is deleted.
	==========================================================================
 */
VOID APCleanupPsQueue(RTMP_ADAPTER *pAd, struct _QUEUE_HEADER *pQueue)
{
	struct _QUEUE_ENTRY *pEntry;
	PNDIS_PACKET pPacket;

	MTWF_DBG(pAd, DBG_CAT_PS, CATPS_CFG, DBG_LVL_INFO, "(0x%08lx)...\n", (ULONG)pQueue);

	while (pQueue->Head) {
		MTWF_DBG(pAd, DBG_CAT_PS, CATPS_CFG, DBG_LVL_INFO, "%u...\n", pQueue->Number);
		pEntry = RemoveHeadQueue(pQueue);
		/*pPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReservedEx); */
		pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}
}

VOID APCleanupQueue(RTMP_ADAPTER *pAd, struct _QUEUE_HEADER *pQueue, NDIS_SPIN_LOCK *queue_lock)
{
	struct _QUEUE_ENTRY *pEntry;
	PNDIS_PACKET pPacket;
	ULONG irqflags;

	if (queue_lock)
		RTMP_IRQ_LOCK(queue_lock, irqflags);
	while (pQueue->Head) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO, "%u...\n", pQueue->Number);
		pEntry = RemoveHeadQueue(pQueue);
		/*pPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReservedEx); */
		pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}
	if (queue_lock)
		RTMP_IRQ_UNLOCK(queue_lock, irqflags);
}

/*
*
*/
static VOID avg_pkt_len_reset(struct _RTMP_ADAPTER *ad)
{
#if defined(VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	vow_avg_pkt_len_reset(ad);
#endif
	ad->mcli_ctl.pkt_avg_len = 0;
	ad->mcli_ctl.sta_nums = 0;
	ad->mcli_ctl.pkt_rx_avg_len = 0;
	ad->mcli_ctl.rx_sta_nums = 0;
}

/*
*
*/
static VOID avg_pkt_len_calculate(struct _MAC_TABLE_ENTRY *entry, UINT8 band_idx)
{
	struct _RTMP_ADAPTER *ad = entry->wdev->sys_handle;
	UINT32 avg_pkt_len = 0;
	struct multi_cli_ctl *mctrl = NULL;
#ifdef RX_COUNT_DETECT
	UINT32 avg_rx_pkt_len = 0;
#endif

	mctrl = &ad->mcli_ctl;
#if defined(VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	vow_avg_pkt_len_calculate(entry);
#endif
	if (entry->avg_tx_pkts > 0)
		avg_pkt_len = (UINT32)(entry->AvgTxBytes / entry->avg_tx_pkts);
#ifdef RX_COUNT_DETECT
	if (entry->avg_rx_pkts > 0)
		avg_rx_pkt_len = (UINT32)(entry->AvgRxBytes / entry->avg_rx_pkts);
#endif
	/*moving average for pkt avg length*/
	if ((avg_pkt_len <= VERIWAVE_INVALID_PKT_LEN_HIGH) &&
			(avg_pkt_len >= VERIWAVE_INVALID_PKT_LEN_LOW)) {
		mctrl->pkt_avg_len =
			((mctrl->pkt_avg_len * mctrl->sta_nums) + avg_pkt_len) / (UINT32)(mctrl->sta_nums + 1);
		mctrl->sta_nums++;
		mctrl->tot_tx_pkts += entry->avg_tx_pkts;
	}
	entry->avg_tx_pkt_len = avg_pkt_len;

#ifdef RX_COUNT_DETECT
	if (avg_rx_pkt_len > 0) {
	mctrl->pkt_rx_avg_len =
		((mctrl->pkt_rx_avg_len * mctrl->rx_sta_nums) + avg_rx_pkt_len) / (mctrl->rx_sta_nums + 1);
	mctrl->rx_sta_nums++;
		mctrl->tot_rx_pkts += entry->avg_rx_pkts;
	}
	entry->avg_rx_pkt_len = avg_rx_pkt_len;
#endif
}

#define MAX_VERIWAVE_DELTA_RSSI  (10)
#define FAR_CLIENT_RSSI	(-70)
#define FAR_CLIENT_DELTA_RSSI	(10)
static VOID CalFarClientNum(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry, UINT_8 band_idx)
{
	UINT8 idx;
	CHAR avg_rssi[MAX_RSSI_LEN];
	BOOLEAN far_client_found = FALSE, large_rssi_gap_found = FALSE;

	for (idx = 0; idx < MAX_RSSI_LEN; idx++)
		avg_rssi[idx] = ad->ApCfg.RssiSample.LastRssi[idx] - ad->BbpRssiToDbmDelta;

	for (idx = 0; idx < MAX_RSSI_LEN; idx++) {
		if (entry->RssiSample.AvgRssi[idx] != -127) {
			if (((avg_rssi[idx] - entry->RssiSample.AvgRssi[idx]) >= MAX_VERIWAVE_DELTA_RSSI) &&
				(large_rssi_gap_found == FALSE)) {
				ad->mcli_ctl.large_rssi_gap_num++;
				large_rssi_gap_found = TRUE;
			}

			if ((entry->RssiSample.AvgRssi[idx] < FAR_CLIENT_RSSI) && (far_client_found == FALSE))
				if ((avg_rssi[idx] - entry->RssiSample.AvgRssi[idx]) >= FAR_CLIENT_DELTA_RSSI) {
					ad->nearfar_far_client_num++;
					far_client_found = TRUE;
				}
		}
	}
}

static VOID dynamic_txop_adjust(struct _RTMP_ADAPTER *pAd)
{
	BOOLEAN	eap_mcli_check = FALSE;
	UINT16	mcliTx = 0, mcliRx = 0;

#ifdef VOW_SUPPORT
	/* always turn on TXOP if SPL is enabled */
	if ((pAd->vow_gen.VOW_GEN == VOW_GEN_TALOS) &&
	    (pAd->vow_misc_cfg.spl_sta_count > 0)) {
		if (pAd->txop_ctl.multi_cli_txop_running) {
			pAd->txop_ctl.multi_cli_txop_running = 0x00;
			disable_tx_burst(pAd, pAd->txop_ctl.cur_wdev,
				AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
		}
		return;
	}
#endif
	eap_mcli_check = FALSE;

	mcliTx = pAd->txop_ctl.multi_client_nums;
#ifdef RX_COUNT_DETECT
	mcliRx = pAd->txop_ctl.multi_rx_client_nums;
#endif
	if (mcliTx + mcliRx >= pAd->multi_cli_nums_eap_th)
		eap_mcli_check = TRUE;

	if (pAd->mcli_ctl.debug_on & MCLI_DEBUG_PER_BAND)
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_TXOP, DBG_LVL_INFO,
			"McliCheck[%d-%d/%d])\n", eap_mcli_check,
			pAd->txop_ctl.multi_client_nums, pAd->multi_cli_nums_eap_th);

	if (eap_mcli_check) {
	/*	For Real sta test, We sense different eap_mutil_client case:
	*	if eap mutilclient >= pAd->multi_cli_nums_eap_th(32)
	*		for UL: enable PRIO_MULTI_CLIENT(TXOP_0)
	*		for DL: disable PRIO_MULTI_CLIENT(TXOP_0)
	*	else
	*		disable PRIO_MULTI_CLIENT(TXOP_0)
	*no matter TCP/UDP test, adjust pAd->multi_cli_nums_eap_th(32) according scenarios
	*/
		if (pAd->peak_tp_ctl.main_traffc_mode == TRAFFIC_UL_MODE) {
			if (!(pAd->txop_ctl.multi_cli_txop_running & (1 << TRAFFIC_UL_MODE))) {
				pAd->txop_ctl.multi_cli_txop_running &= ~(1 << TRAFFIC_UL_MODE);
				pAd->txop_ctl.multi_cli_txop_running |= (1 << TRAFFIC_UL_MODE);
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_TXOP, DBG_LVL_INFO,
					"UL enable_tx_burst TXOP_0\n");
				enable_tx_burst(pAd, pAd->txop_ctl.cur_wdev,
						AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
			}
		} else if (pAd->peak_tp_ctl.main_traffc_mode == TRAFFIC_DL_MODE) {
			if (!(pAd->txop_ctl.multi_cli_txop_running & (1 << TRAFFIC_DL_MODE))) {
				pAd->txop_ctl.multi_cli_txop_running &= ~(1 << TRAFFIC_DL_MODE);
				pAd->txop_ctl.multi_cli_txop_running |= (1 << TRAFFIC_DL_MODE);
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_TXOP, DBG_LVL_INFO,
					"DL disable_tx_burst TXOP_0\n");
				disable_tx_burst(pAd, pAd->txop_ctl.cur_wdev,
						AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
			}
		}
	} else {
	/* Non eap_mutil_client case, disable PRIO_MULTI_CLIENT(TXOP_0), enable other TXOP */
		if (pAd->txop_ctl.multi_cli_txop_running) {
			pAd->txop_ctl.multi_cli_txop_running = 0x00;
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_TXOP, DBG_LVL_INFO,
					"[%d]Non-eap_mcli disable_tx_burst TXOP_0\n", __LINE__);
			disable_tx_burst(pAd, pAd->txop_ctl.cur_wdev,
					 AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
		}
	}
}


#if defined(VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT)
static VOID dynamic_near_far_adjust(struct _RTMP_ADAPTER *pAd)
{
	UINT32 wcid;
	MAC_TABLE_ENTRY *pEntry;
	union _HTTRANSMIT_SETTING last_tx_rate_set;
	ULONG last_tx_rate;
	NEAR_FAR_CTRL_T *near_far_ctrl = &pAd->vow_misc_cfg.near_far_ctrl;
	BOOLEAN fast_sta_flg = FALSE, slow_sta_flg = FALSE;
	struct txop_ctl *txop_ctl_p = NULL;
	struct wifi_dev *wdev = NULL;

	txop_ctl_p = &pAd->txop_ctl;
	if (txop_ctl_p->cur_wdev == NULL)
		return;

	wdev = txop_ctl_p->cur_wdev;
	if (wlan_config_get_ch_band(wdev) != CMD_CH_BAND_5G)
		return;

	if (!near_far_ctrl->adjust_en) {
		if (txop_ctl_p->near_far_txop_running == TRUE) {
			disable_tx_burst(pAd, wdev, AC_BE, PRIO_NEAR_FAR, TXOP_0);
			txop_ctl_p->near_far_txop_running = FALSE;
		}
		return;
	}

	for (wcid = 0; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		pEntry = entry_get(pAd, wcid);

		if (IS_ENTRY_NONE(pEntry))
			continue;

		if (pEntry->wdev != wdev)
			continue;

#ifdef SW_CONNECT_SUPPORT
		if (hc_is_sw_wcid(pAd, wcid))
			continue;
#endif /* SW_CONNECT_SUPPORT */

		if (pEntry->avg_tx_pkts > VERIWAVE_5G_PKT_CNT_TH) {
			last_tx_rate_set.word = (USHORT)pEntry->LastTxRate;
			getRate(last_tx_rate_set, &last_tx_rate);
			if (last_tx_rate >= near_far_ctrl->fast_phy_th)
				fast_sta_flg = TRUE;
			else if (last_tx_rate <= near_far_ctrl->slow_phy_th)
				slow_sta_flg = TRUE;
		}
	}

	if (fast_sta_flg & slow_sta_flg) {
		if (txop_ctl_p->near_far_txop_running == FALSE) {
			enable_tx_burst(pAd, wdev, AC_BE, PRIO_NEAR_FAR, TXOP_0);
			txop_ctl_p->near_far_txop_running = TRUE;
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
				"PRIO_NEAR_FAR: start\n");
		}
	} else {
		if (txop_ctl_p->near_far_txop_running == TRUE) {
			disable_tx_burst(pAd, wdev, AC_BE, PRIO_NEAR_FAR, TXOP_0);
			txop_ctl_p->near_far_txop_running = FALSE;
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
				"PRIO_NEAR_FAR: end\n");
		}
	}
}

static VOID dynamic_airtime_fairness_adjust(struct _RTMP_ADAPTER *ad)
{
	BOOLEAN shrink_flag = FALSE;
	BOOLEAN mc_flg = FALSE, fband_2g = FALSE;
	UCHAR band_idx = 0;
	UINT16 multi_client_num_th = 0, veriwave_tp_amsdu_dis_th = 0;
	struct wifi_dev *wdev = NULL;
	struct _RTMP_ADAPTER *mac_ad;
	UCHAR ch_band = 0;

	if (!ad->vow_cfg.en_airtime_fairness)
		return;

	if (ad->txop_ctl.multi_client_nums == 0)
		return;
	else {
		/*concurrent case, not adjust*/
		for (band_idx = 0; band_idx < CFG_WIFI_RAM_BAND_NUM; band_idx++) {
			mac_ad = physical_device_get_mac_adapter_by_band(
				ad->physical_dev, band_idx);
			if (mac_ad == ad)
				continue;
			if (mac_ad == NULL)
				continue;
			if (mac_ad->txop_ctl.multi_client_nums > 0)
				return;
		}
	}

	wdev = ad->txop_ctl.cur_wdev;
	if (!wdev)
		return;

	ch_band = wlan_config_get_ch_band(wdev);

	if (ch_band == CMD_CH_BAND_24G) {
		multi_client_num_th = MULTI_CLIENT_2G_NUMS_TH;
		veriwave_tp_amsdu_dis_th = VERIWAVE_2G_TP_AMSDU_DIS_TH;
		fband_2g = TRUE;
	} else if (ch_band == CMD_CH_BAND_5G || ch_band == CMD_CH_BAND_6G) {
		multi_client_num_th = MULTI_CLIENT_NUMS_TH;
		veriwave_tp_amsdu_dis_th = VERIWAVE_TP_AMSDU_DIS_TH;
	} else {
		MTWF_DBG(ad, DBG_CAT_TX, CATTX_VOW, DBG_LVL_ERROR, "[%d]\n", __LINE__);
		return;
	}

	/*adjust amsdu*/
	if (ad->txop_ctl.multi_client_nums >= multi_client_num_th) {
		if (ad->vow_mcli_ctl.pkt_avg_len > VERIWAVE_PKT_LEN_LOW)
			shrink_flag = TRUE;

	} else {
		if (ad->txop_ctl.multi_client_nums > 1) {
			if (ad->vow_mcli_ctl.pkt_avg_len > veriwave_tp_amsdu_dis_th)
				shrink_flag = TRUE;
			else if (ad->vow_mcli_ctl.pkt_avg_len > VERIWAVE_PKT_LEN_LOW)
				shrink_flag = TRUE;
		}
	}

	if ((ad->vow_gen.VOW_GEN >= VOW_GEN_FALCON) || (ad->mcli_ctl.large_rssi_gap_num > 0))
		shrink_flag = FALSE;

	if (ad->txop_ctl.multi_client_nums >= multi_client_num_th)
		mc_flg = TRUE;

	if (ad->vow_sta_frr_flag != shrink_flag) {
		/* adj airtime quantum only when WATF is not enabled */
		ad->vow_sta_frr_flag = shrink_flag;
		if (ad->vow_cfg.en_airtime_fairness && ad->vow_sta_frr_quantum && !vow_watf_is_enabled(ad)) {
			if ((shrink_flag == TRUE) && (ad->nearfar_far_client_num <= 1) && mc_flg)
				RTMP_SET_STA_DWRR_QUANTUM(ad, FALSE, ad->vow_sta_frr_quantum);/* fast round robin */
			else
				RTMP_SET_STA_DWRR_QUANTUM(ad, TRUE, 0);/* restore */
		}

	}
	return;
}

/* Used for mcli balance/near-far case test, note that it may reduce the total T-PUT */
static VOID dynamic_mcli_param_adjust(struct _RTMP_ADAPTER *pAd)
{
	UINT16	mcliTx = 0, mcliRx = 0;
	BOOLEAN	mcli_tcp_check = FALSE;
	UINT8	wmm_idx, default_vow_sch_type, default_vow_sch_policy;
	UINT32	default_cwmax, default_cwmin, mcli_cwmax, mcli_cwmin;
	struct wifi_dev *wdev = NULL;

	mcliTx = pAd->txop_ctl.multi_client_nums;
#ifdef RX_COUNT_DETECT
	mcliRx = pAd->txop_ctl.multi_rx_client_nums;
#endif

	if ((mcliTx + mcliRx >= pAd->multi_cli_nums_eap_th) &&
		pAd->vow_cfg.mcli_sch_cfg.mcli_tcp_num >= pAd->multi_cli_nums_eap_th)
		mcli_tcp_check = TRUE;

	if (pAd->mcli_ctl.debug_on & MCLI_DEBUG_PER_BAND)
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
			"mcli_tcp_check/mcli_tcp_num: [%d/%d]\n",
			mcli_tcp_check, pAd->vow_cfg.mcli_sch_cfg.mcli_tcp_num);
	/* reset mcli_tcp_num for next period */
	pAd->vow_cfg.mcli_sch_cfg.mcli_tcp_num = 0;

	if (pAd->txop_ctl.cur_wdev != NULL) {
		wdev = pAd->txop_ctl.cur_wdev;
		wmm_idx = HcGetWmmIdx(pAd, wdev);
		/* save default BE cwmax/cwmin value,
		* set multi user BE cwmax/cwmin value.
		*/
		default_cwmax = pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[0];
		default_cwmin = pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[0];
		/* save default vow schedule type/policy value,
		 * set multi-client vow schedule type/policy value.
		 */
		default_vow_sch_type = pAd->vow_cfg.mcli_sch_cfg.sch_type;
		default_vow_sch_policy = pAd->vow_cfg.mcli_sch_cfg.sch_policy;
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
			"wmm_idx = %d, cwmax = %d, cwmin = %d !!\n",
			wmm_idx, default_cwmax, default_cwmin);
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
			"cur_wdev is null!!\n");
		return;
	}

	if (mcli_tcp_check) {
		if (pAd->peak_tp_ctl.main_traffc_mode == TRAFFIC_UL_MODE) {
			if (!(pAd->vow_cfg.mcli_sch_cfg.schedule_cond_running & (1 << VOW_MCLI_UL_MODE))) {
				pAd->vow_cfg.mcli_sch_cfg.schedule_cond_running &= ~(1 << VOW_MCLI_UL_MODE);
				pAd->vow_cfg.mcli_sch_cfg.schedule_cond_running |= (1 << VOW_MCLI_UL_MODE);
				mcli_cwmax = pAd->vow_cfg.mcli_sch_cfg.cwmax[VOW_MCLI_UL_MODE];
				mcli_cwmin = pAd->vow_cfg.mcli_sch_cfg.cwmin[VOW_MCLI_UL_MODE];
				HW_SET_PART_WMM_PARAM(pAd, wdev, wmm_idx,
					WMM_AC_BE, WMM_PARAM_CWMAX, mcli_cwmax);
				HW_SET_PART_WMM_PARAM(pAd, wdev, wmm_idx,
					WMM_AC_BE, WMM_PARAM_CWMIN, mcli_cwmin);
				if (pAd->vow_cfg.mcli_sch_cfg.dl_wrr_en)
					HW_SET_VOW_SCHEDULE_CTRL(pAd, TRUE,
						VOW_SCH_FOLLOW_HW, VOW_SCH_POL_WRR);
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
					"mcli scene, UL mode, Enter schedule !!\n");
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
					"mcli scene, UL mode, wrren:cwmin:cwmax=%d:%d:%d !!\n",
					pAd->vow_cfg.mcli_sch_cfg.dl_wrr_en,
					mcli_cwmin, mcli_cwmax);
			}
		} else if (pAd->peak_tp_ctl.main_traffc_mode == TRAFFIC_DL_MODE) {
			if (!(pAd->vow_cfg.mcli_sch_cfg.schedule_cond_running & (1 << VOW_MCLI_DL_MODE))) {
				pAd->vow_cfg.mcli_sch_cfg.schedule_cond_running &= ~(1 << VOW_MCLI_DL_MODE);
				pAd->vow_cfg.mcli_sch_cfg.schedule_cond_running |= (1 << VOW_MCLI_DL_MODE);
				mcli_cwmax = pAd->vow_cfg.mcli_sch_cfg.cwmax[VOW_MCLI_DL_MODE];
				mcli_cwmin = pAd->vow_cfg.mcli_sch_cfg.cwmin[VOW_MCLI_DL_MODE];
				HW_SET_PART_WMM_PARAM(pAd, wdev, wmm_idx,
					WMM_AC_BE, WMM_PARAM_CWMAX, mcli_cwmax);
				HW_SET_PART_WMM_PARAM(pAd, wdev, wmm_idx,
					WMM_AC_BE, WMM_PARAM_CWMIN, mcli_cwmin);
				if (pAd->vow_cfg.mcli_sch_cfg.dl_wrr_en)
					HW_SET_VOW_SCHEDULE_CTRL(pAd, TRUE,
						VOW_SCH_FOLLOW_HW, VOW_SCH_POL_WRR);
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
					"mcli scene, DL mode, Enter schedule !!\n");
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
					"mcli scene, DL mode, wrren:cwmin:cwmax=%d:%d:%d\n",
					pAd->vow_cfg.mcli_sch_cfg.dl_wrr_en,
					mcli_cwmin, mcli_cwmax);
			}
		}
	} else {
		if (pAd->vow_cfg.mcli_sch_cfg.schedule_cond_running) {
			pAd->vow_cfg.mcli_sch_cfg.schedule_cond_running = 0x00;
			HW_SET_PART_WMM_PARAM(pAd, wdev, wmm_idx, WMM_AC_BE,
				WMM_PARAM_CWMAX, default_cwmax);
			HW_SET_PART_WMM_PARAM(pAd, wdev, wmm_idx, WMM_AC_BE,
				WMM_PARAM_CWMIN, default_cwmin);
			if (pAd->vow_cfg.mcli_sch_cfg.dl_wrr_en)
				HW_SET_VOW_SCHEDULE_CTRL(pAd, TRUE, default_vow_sch_type,
					default_vow_sch_policy);
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
				"no mcli scene, Enter default condition !!\n");
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
				"no mcli scene, mode:cwmin:cwmax=%d:%d:%d !!\n",
				pAd->peak_tp_ctl.main_traffc_mode, default_cwmin,
				default_cwmax);
		}
	}
}
#endif /* defined (VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT) */

#ifdef DYNAMIC_ADJUST_BAWINSZ
VOID dynamic_adjust_ba_winsize(struct _RTMP_ADAPTER *ad, MAC_TABLE_ENTRY *entry)
{
	UINT32 ori_idx, per, winsize, WinSizeThr;
	struct BA_ORI_ENTRY *pBAEntry = NULL;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(ad->physical_dev);
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(ad->hdev_ctrl);

	ori_idx = entry->ba_info.OriWcidArray[0];//TID0

#ifdef DOT11_EHT_BE
	if (IS_ENTRY_MLO(entry)) {
		struct mld_entry_t *mld_entry;

		mt_rcu_read_lock();
		mld_entry = rcu_dereference(entry->mld_entry);
		if (!mld_entry) {
			mt_rcu_read_unlock();
			return;
		}
		ori_idx = mld_entry->ba_info.OriWcidArray[0];
		mt_rcu_read_unlock();
	}
#endif /* DOT11_EHT_BE */

	pBAEntry = &ba_ctl->BAOriEntry[ori_idx];

	if (ori_idx == 0 || !pBAEntry) {
		MTWF_DBG(ad, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
			"STA(%d) doesn't establish BA session, return!\n", entry->wcid);
		return;
	}

	if (chip_dbg->set_ba_winsize) {
		if (pBAEntry->BAWinSize <= BA_WIN_SZ_64)
			WinSizeThr = 0x7;
		else if (pBAEntry->BAWinSize <= BA_WIN_SZ_256)
			WinSizeThr = 0xB;
		else if (pBAEntry->BAWinSize <= BA_WIN_SZ_1024)
			WinSizeThr = 0xE;

		if (entry->winsize_limit == 0xF)
			entry->winsize_limit = WinSizeThr;

		winsize = entry->winsize_limit;//init value
		per = entry->tx_per;

		if (per > ad->per_dn_th) {//you can adjust dn/up-kp for real test.
			if (entry->winsize_limit > 0x7)
				winsize = entry->winsize_limit >> 1;
			else if (entry->winsize_limit > ad->winsize_kp_idx)
				winsize = entry->winsize_limit - 1;
		} else if (per < ad->per_up_th) {
			if (entry->winsize_limit < WinSizeThr)
				winsize = entry->winsize_limit + 1;
		}

		if (winsize == entry->winsize_limit)
			entry->agg_err_flag = FALSE;

		MTWF_DBG(ad, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
			"STA(%d), per/dnth/upth(%d/%d/%d), winsize/limit/kpth/Thr(%d/%d/%d/%d)\n",
			entry->wcid, per, ad->per_dn_th, ad->per_up_th,
			winsize, entry->winsize_limit, ad->winsize_kp_idx, WinSizeThr);

		if (winsize < 0xF && entry->winsize_limit != winsize) {
			/* Only set BA_WIN_SIZE(TID0), should consider other tid? */
			chip_dbg->set_ba_winsize(ad, entry->wcid, 0, winsize);
			entry->winsize_limit = winsize;
		}
	}

}

VOID dynamic_agg_per_sta_adjust(struct _RTMP_ADAPTER *ad)
{
	UINT16  wcid;
	BOOLEAN mcli_flag = FALSE;

	if (ad->txop_ctl.multi_client_nums >= ad->multi_cli_nums_eap_th
#ifdef RX_COUNT_DETECT
		|| ad->txop_ctl.multi_rx_client_nums >= ad->multi_cli_nums_eap_th
#endif
		)
		mcli_flag = TRUE;

	for (wcid = 0; VALID_UCAST_ENTRY_WCID(ad, wcid); wcid++) {
		MAC_TABLE_ENTRY *pEntry = entry_get(ad, wcid);
		/* Only HW STA have AGG */
#ifdef SW_CONNECT_SUPPORT
		STA_TR_ENTRY * tr_entry = tr_entry_get(ad, wcid);
			if (IS_SW_STA(tr_entry))
				continue;
#endif /* SW_CONNECT_SUPPORT */
		if (pEntry && IS_ENTRY_CLIENT(pEntry)
			&& pEntry->Sst == SST_ASSOC) {

			/* Process sta belongs to this pAd */
			if (pEntry->pAd != ad)
				continue;
			/* Only HW STA have AGG */
#ifdef SW_CONNECT_SUPPORT
			if (hc_is_sw_wcid(ad, wcid))
				continue;
#endif /* SW_CONNECT_SUPPORT */
			/* firstly we should check aggManualEn;
			* then check agg_err_flag and multiclient condition.
			*/
			if (ad->aggManualEn ||
				(mcli_flag && pEntry->agg_err_flag)) {
				MTWF_DBG(ad, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
					"STA(%d) hit agg err, need adjust agg num!\n", pEntry->wcid);
				/*
				* we have cal tx_per from event_get_tx_statistic_handle;
				* according sta tx_per set wtbl(BA_WIN_SIZE).
				*/
				dynamic_adjust_ba_winsize(ad, pEntry);
			}
		}
	}

}
#endif /* defined DYNAMIC_ADJUST_BAWINSZ */

static inline BOOLEAN is_tcp_pkt(NDIS_PACKET *pkt)
{
	PUCHAR pSrcBuf;
	USHORT TypeLen;

	if (pkt) {
		pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
		TypeLen = OS_NTOHS(*((UINT16 *)(pSrcBuf + 12)));
	} else {
		return FALSE;
	}

	if ((TypeLen == 0x0800) && /* Type: IP (0x0800) */
	    (pSrcBuf[23] == 0x06)) /* Protocol: TCP (0x06) */
		return TRUE;

	return FALSE;
}

static inline void check_sta_tcp_flow(RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry,
				      UINT band_idx)
{
	int q_idx;
	struct _QUEUE_ENTRY *q_entry;
	NDIS_PACKET *ptk = NULL;
	BOOLEAN	is_tcp = FALSE;

	if (pAd->txop_ctl.multi_tcp_nums >= MULTI_TCP_NUMS_TH)
		return;

	for (q_idx = 0 ; q_idx < WMM_QUE_NUM; q_idx++) {
		if (tr_entry->tx_queue[q_idx].Number == 0)
			continue;

		RTMP_SPIN_LOCK(&tr_entry->txq_lock[q_idx]);
		if (tr_entry->tx_queue[q_idx].Head) {
			q_entry = tr_entry->tx_queue[q_idx].Head;
			ptk = QUEUE_ENTRY_TO_PACKET(q_entry);
			is_tcp = is_tcp_pkt(ptk);
		}
		RTMP_SPIN_UNLOCK(&tr_entry->txq_lock[q_idx]);

		if (is_tcp) {
			pAd->txop_ctl.multi_tcp_nums++;
			break;
		}
	}
}

#if (defined(IGMP_SNOOP_SUPPORT) && defined(IGMP_SNOOPING_OFFLOAD))
static VOID tx_offload_m2u_counter_update(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry)
{
	struct _BSS_STRUCT *mbss;

	if (IS_VALID_ENTRY(entry) && IS_ENTRY_CLIENT(entry) && entry->pMbss) {
		mbss = MBSS_GET(entry->pMbss);
		mbss->M2U_TxBytes += entry->one_sec_M2U_TxBytes;
		mbss->M2U_TxPackets += entry->one_sec_M2U_TxPackets;
		entry->M2U_TxBytes += entry->one_sec_M2U_TxBytes;
		entry->M2U_TxPackets += entry->one_sec_M2U_TxPackets;
	}
}
#endif

#ifdef WHNAT_SUPPORT
static VOID tx_offload_counter_update(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry)
{
#ifdef CONFIG_AP_SUPPORT
		struct _BSS_STRUCT *mbss;

		if (IS_ENTRY_CLIENT(entry) && entry->pMbss) {
			mbss = MBSS_GET(entry->pMbss);
			mbss->TxCount += entry->one_sec_tx_pkts;
			mbss->TransmittedByteCount += entry->OneSecTxBytes;

#ifdef MAP_R2
			if (IS_MAP_ENABLE(ad) && IS_MAP_R2_ENABLE(ad))
				mbss->ucBytesTx += entry->OneSecTxBytes;
			if (IS_MAP_ENABLE(ad))
				entry->TxBytesMAP += entry->OneSecTxBytes;
#endif

			/* per STA */
			entry->TxPackets.QuadPart += entry->one_sec_tx_pkts;
			entry->TxBytes += entry->OneSecTxBytes;
			/* per Band */
			ad->WlanCounters.TxTotByteCount.QuadPart += entry->OneSecTxBytes;
			MTWF_DBG(ad, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
				 "WCID=%d TxPackets=%ld TxBytes=%ld, TxTotByteCount=%ld,one_sec_tx_pkts=%ld OneSecTxBytes=%ld\n",
				 entry->wcid, (ULONG)entry->TxPackets.QuadPart,
				 (ULONG)ad->WlanCounters.TxTotByteCount.QuadPart,
				 (ULONG)entry->TxBytes,
				 entry->one_sec_tx_pkts, entry->OneSecTxBytes);
		}
#endif /*CONFIG_AP_SUPPORT*/

#ifdef APCLI_SUPPORT
		if ((IS_ENTRY_PEER_AP(entry) || IS_ENTRY_REPEATER(entry)) && entry->wdev) {
			struct _STA_ADMIN_CONFIG *apcli = GetStaCfgByWdev(ad, entry->wdev);

			apcli->StaStatistic.TxCount += entry->one_sec_tx_pkts;
			apcli->StaStatistic.TransmittedByteCount += entry->OneSecTxBytes;
		}
#endif /*APCLI_SUPPORT*/

#ifdef WDS_SUPPORT

	if (IS_ENTRY_WDS(entry)) {
		ad->WdsTab.WdsEntry[entry->func_tb_idx].WdsCounter.TransmittedFragmentCount.QuadPart +=  entry->one_sec_tx_pkts;
		ad->WdsTab.WdsEntry[entry->func_tb_idx].WdsCounter.TransmittedByteCount += entry->OneSecTxBytes;
	}
#endif /* WDS_SUPPORT */
}
#endif /*WHNAT_SUPPORT*/


/*
	==========================================================================
	Description:
		This routine is called by APMlmePeriodicExec() every second to check if
		1. any associated client in PSM. If yes, then TX MCAST/BCAST should be
		   out in DTIM only
		2. any client being idle for too long and should be aged-out from MAC table
		3. garbage collect PSQ
	==========================================================================
*/

VOID MacTableMaintenance(RTMP_ADAPTER *pAd)
{
	int wcid, startWcid, i;
#ifdef RTMP_MAC_PCI
	unsigned long	IrqFlags;
#endif /* RTMP_MAC_PCI */
	MAC_TABLE *pMacTable;
	CHAR avgRssi;
	BSS_STRUCT *pMbss;
	BOOLEAN skip_kick = FALSE;
#ifdef MT_MAC
	BOOLEAN bPreAnyStationInPsm = FALSE;
#endif /* MT_MAC */
	UINT     BandIdx = hc_get_hw_band_idx(pAd);
#ifdef SMART_CARRIER_SENSE_SUPPORT
	CHAR    tmpRssi = 0;
#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef APCLI_SUPPORT
	ULONG	apcli_avg_tx = 0;
	ULONG	apcli_avg_rx = 0;
	struct wifi_dev *apcli_wdev = NULL;
	PSTA_ADMIN_CONFIG sta_cfg = NULL;
#endif /* APCLI_SUPPORT */
	struct wifi_dev *sta_wdev = NULL;
	struct wifi_dev *txop_wdev = NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR    sta_hit_2g_infra_case_number = 0;
#if defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)
	BOOLEAN IgmpQuerySendTickChanged = FALSE;
	BOOLEAN MldQuerySendTickChanged = FALSE;
#endif
	struct _RTMP_CHIP_CAP *cap;
	struct peak_tp_ctl *peak_tp_ctl = NULL;
	BOOLEAN is_no_rx_cnt = FALSE;
	BOOLEAN is_tx_traffic = FALSE, is_rx_traffic = FALSE;
	UINT32 tx_ratio = 0, rx_ratio = 0;
#ifdef RACTRL_LIMIT_MAX_PHY_RATE
	UINT16 u2TxTP = 0;
#endif
	UINT32 sta_idle_timeout = 0;
	TX_STAT_STRUC *p_temp = NULL;
	TX_STAT_STRUC *p_tx_statics_queue = NULL;
	UCHAR tx_statics_qcnt = 0;
	UCHAR ch_band;
#ifdef PEAK_ENHANCE
	struct peak_enhance_ctrl *ctrl = &pAd->CommonCfg.peek_enhance;
#endif
	RTMP_ADAPTER *pAd_per_band = NULL;
	UCHAR band_idx;
#if (defined(IGMP_SNOOP_SUPPORT) && defined(IGMP_SNOOPING_OFFLOAD))
	struct _CR4_QUERY_STRUC *cr4_m2u_query_list;
	UINT16 cr4_m2u_query_len;
#endif

	/*alloc mem to agg tx statics cmd for cr4*/
#ifdef WHNAT_SUPPORT
	struct _CR4_QUERY_STRUC *cr4_query_list;
	UINT16 cr4_query_len = sizeof(*cr4_query_list) + sizeof(*cr4_query_list->list) * MAX_CR4_QUERY_NUM;

	os_alloc_mem(pAd, (UCHAR **)&cr4_query_list, cr4_query_len);
	if (cr4_query_list == NULL)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "alloc mem fail!!!\n");
	else
		os_zero_mem(cr4_query_list, cr4_query_len);
#endif

#if (defined(IGMP_SNOOP_SUPPORT) && defined(IGMP_SNOOPING_OFFLOAD))
	cr4_m2u_query_len = sizeof(*cr4_m2u_query_list) + sizeof(*cr4_m2u_query_list->list) * MAX_CR4_QUERY_NUM;
	os_alloc_mem(pAd, (UCHAR **)&cr4_m2u_query_list, cr4_m2u_query_len);
	if (cr4_m2u_query_list == NULL)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "alloc mem fail!!!\n");
	else
		os_zero_mem(cr4_m2u_query_list, cr4_m2u_query_len);
#endif

	/*alloc mem to agg tx statics cmd*/
	os_alloc_mem(pAd, (UCHAR **)&p_tx_statics_queue, sizeof(TX_STAT_STRUC) * MAX_FW_AGG_MSG);

	if (!p_tx_statics_queue) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "alloc mem fail!!!\n");
	} else
		os_zero_mem(p_tx_statics_queue, sizeof(TX_STAT_STRUC) * MAX_FW_AGG_MSG);

	cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	pMacTable = pAd->MacTab;
#ifdef MT_MAC
	bPreAnyStationInPsm = pMacTable->fAnyStationInPsm;
#endif /* MT_MAC */
	pMacTable->fAnyStationInPsm = FALSE;
	pMacTable->fAnyStationBadAtheros = FALSE;
	pMacTable->fAnyTxOPForceDisable = FALSE;
	pMacTable->fCurrentStaBw40 = FALSE;
#ifdef DOT11_N_SUPPORT
	pMacTable->fAnyStation20Only = FALSE;
	pMacTable->fAnyStationIsLegacy = FALSE;
	pMacTable->fAnyStationMIMOPSDynamic = FALSE;
#ifdef DOT11N_DRAFT3
	pMacTable->fAnyStaFortyIntolerant = FALSE;
#endif /* DOT11N_DRAFT3 */
	pMacTable->fAllStationGainGoodMCS = TRUE;
#endif /* DOT11_N_SUPPORT */
	startWcid = 0;
	/* fix "hobj is not ready" log continuous printf */
	pAd->txop_ctl.cur_wdev = NULL;

#ifdef SMART_CARRIER_SENSE_SUPPORT
	pAd->SCSCtrl.SCSMinRssi =  0; /* (Reset)The minimum RSSI of STA */
	pAd->SCSCtrl.OneSecRxByteCount = 0;
	pAd->SCSCtrl.OneSecTxByteCount = 0;
#endif /* SMART_CARRIER_SENSE_SUPPORT */

	/* step1. scan all wdevs, clean all wdev non_gf_sta counter */
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];
		if (wdev != NULL) {
			wlan_operate_set_non_gf_sta(wdev, 0);
		}
	}

	if (BandIdx < ARRAY_SIZE(pMacTable->fAnyStationNonGF))
		pMacTable->fAnyStationNonGF[BandIdx] = FALSE;

	/* for get tx/rx pkt cnt and tx/rx byte cnt, tx retry/error count,
	   only send 1 times can get all sta info in 3 bands */
	for (band_idx = 0; band_idx < MAX_BAND_NUM; band_idx++) {
		pAd_per_band = physical_device_get_mac_adapter_by_band(pAd->physical_dev, band_idx);
		if (pAd_per_band != NULL && VIRTUAL_IF_NUM(pAd_per_band) != 0)
			break;//Find first Alive pAd/Band;
	}
	if (pAd == pAd_per_band) {
		UINT32 eventTypeBitmap = 0;

		eventTypeBitmap = BIT(EVENT_PHY_TX_STAT_PER_WCID) |
				BIT(EVENT_PHY_TXRX_ADM_STAT) |
				BIT(EVENT_PHY_TRX_MSDU_COUNT);
#ifdef TR181_SUPPORT
		eventTypeBitmap |= BIT(EVENT_PHY_TX_DATA_RETRY_COUNT);
#endif

		for (band_idx = 0; band_idx < MAX_BAND_NUM; band_idx++) {
			pAd_per_band = physical_device_get_mac_adapter_by_band(pAd->physical_dev, band_idx);
			if (pAd_per_band == NULL)
				continue;
#ifdef CONFIG_MAP_SUPPORT
			if ((IS_MAP_TURNKEY_ENABLE(pAd_per_band) || IS_MAP_BS_ENABLE(pAd_per_band))) {
				if (pAd_per_band->ApCfg.EntryClientCount)
					eventTypeBitmap |= BIT(EVENT_PHY_ALL_TX_RX_RATE) | BIT(EVENT_PHY_TXRX_AIR_TIME);
			}
#endif
#ifdef TXRX_STAT_SUPPORT
			if (pAd_per_band->OneSecMibBucket.Enabled == TRUE)
				eventTypeBitmap |= BIT(EVENT_PHY_RX_STAT);
#endif /* TXRX_STAT_SUPPORT */
		}

		RTMP_GET_ALL_STA_STATS(pAd, eventTypeBitmap);

		chip_do_extra_action(pAd, NULL, NULL,
			CHIP_EXTRA_ACTION_PN_CHK, NULL, NULL);
	}

	avg_pkt_len_reset(pAd);
#ifdef PEAK_ENHANCE
	reset_txop_all_txrx_counter(pAd);
#endif

#ifdef SW_CONNECT_SUPPORT
	skip_kick = hc_is_sw_sta_enable(pAd) ? TRUE : FALSE;
#endif /* SW_CONNECT_SUPPORT */

	for (wcid = startWcid; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		MAC_TABLE_ENTRY *pEntry = entry_get(pAd, wcid);
		STA_TR_ENTRY *tr_entry = tr_entry_get(pAd, wcid);
		struct BA_INFO *ba_info = &pEntry->ba_info;
		BOOLEAN bDisconnectSta = FALSE;
#ifdef RACTRL_LIMIT_MAX_PHY_RATE
#ifdef DOT11_HE_AX
		RA_ENTRY_INFO_T *raentry;
		struct _STA_REC_CTRL_T *strec = &tr_entry->StaRec;
		struct he_mcs_info *mcs;

		raentry = &pEntry->RaEntry;
#endif
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */

		/* Process sta belongs to this pAd */
		if (pEntry->pAd != pAd)
			continue;

		if (IS_ENTRY_NONE(pEntry) || IS_ENTRY_MCAST(pEntry))
			continue;

#ifdef SW_CONNECT_SUPPORT
		if (hc_is_sw_wcid(pAd, wcid))
			continue;
#endif /* SW_CONNECT_SUPPORT */

#ifdef HTC_DECRYPT_IOT

		if (pEntry && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))) {
			if (pEntry->HTC_AAD_OM_CountDown > 0) /* count down to start all new pEntry->HTC_ICVErrCnt */
				pEntry->HTC_AAD_OM_CountDown--;
		}

#endif /* HTC_DECRYPT_IOT */

#ifdef RACTRL_LIMIT_MAX_PHY_RATE
#ifdef DOT11_VHT_AC

		if (pEntry->fgRaLimitPhyRate == FALSE) {
			BOOLEAN fgPhyModeCheck = FALSE;

			u2TxTP = pEntry->OneSecTxBytes >> BYTES_PER_SEC_TO_MBPS;

			if (pEntry->SupportRateMode & SUPPORT_VHT_MODE) {
				if (pEntry->MaxHTPhyMode.field.BW == BW_160)
					fgPhyModeCheck = TRUE;

				if (RACTRL_LIMIT_MAX_PHY_RATE >= MAX_PHY_RATE_3SS) {
					if (pEntry->SupportVHTMCS4SS)
						fgPhyModeCheck = TRUE;
				} else if (RACTRL_LIMIT_MAX_PHY_RATE >= MAX_PHY_RATE_2SS) {
					if (pEntry->SupportVHTMCS3SS)
						fgPhyModeCheck = TRUE;
				} else
					fgPhyModeCheck = TRUE;
			}

			if ((u2TxTP > LIMIT_MAX_PHY_RATE_THRESHOLD) && fgPhyModeCheck) {
				MtCmdSetMaxPhyRate(pAd, RACTRL_LIMIT_MAX_PHY_RATE);
				pEntry->fgRaLimitPhyRate = TRUE;
			}
#ifdef DOT11_HE_AX
			if (pEntry->HTPhyMode.field.MODE == MODE_HE) {
				for (i = 2; i < DOT11AX_MAX_STREAM; i++) {
					if ((pEntry->cap.rate.he80_rx_nss_mcs[i] == 1) ||
					    (pEntry->cap.rate.he80_rx_nss_mcs[i] == 2) ||
						(pEntry->cap.rate.he160_rx_nss_mcs[i] == 1) ||
						(pEntry->cap.rate.he160_rx_nss_mcs[i] == 2) ||
						(pEntry->cap.rate.he8080_rx_nss_mcs[i] == 1) ||
						(pEntry->cap.rate.he8080_rx_nss_mcs[i] == 2))
						fgPhyModeCheck = TRUE;
				}
			}
			if ((u2TxTP > LIMIT_MAX_PHY_RATE_THRESHOLD) && fgPhyModeCheck) {
				pEntry->fgRaLimitPhyRate = TRUE;
				raentry->u2SupportVHTMCS1SS = (pEntry->SupportVHTMCS1SS & 255);
				raentry->u2SupportVHTMCS2SS = (pEntry->SupportVHTMCS2SS & 255);
				raentry->u2SupportVHTMCS3SS = (pEntry->SupportVHTMCS3SS & 255);
				raentry->u2SupportVHTMCS4SS = (pEntry->SupportVHTMCS4SS & 255);
				mcs = &strec->he_sta.max_nss_mcs;

				for (i = 0 ; i < DOT11AX_MAX_STREAM; i++) {
					if ((mcs->bw80_mcs[i] == 1) || (mcs->bw80_mcs[i] == 2))
						mcs->bw80_mcs[i] = 0;

					if ((mcs->bw160_mcs[i] == 1) || (mcs->bw160_mcs[i] == 2))
						mcs->bw160_mcs[i] = 0;

					if ((mcs->bw8080_mcs[i] == 1) || (mcs->bw8080_mcs[i] == 2))
						mcs->bw8080_mcs[i] = 0;
				}
				pEntry->fgRaLimitPhyRate = TRUE;
				pEntry->update_he_maxra = true;
				RTEnqueueInternalCmd(pAd, CMDTHREAD_UPDATE_MAXRA, pEntry, sizeof(MAC_TABLE_ENTRY));
			}
		} else {
			u2TxTP = pEntry->OneSecTxBytes >> BYTES_PER_SEC_TO_MBPS;
			if (u2TxTP < LIMIT_MAX_PHY_RATE_THRESHOLD) {
				/* restore to default rate */
				raentry->u2SupportVHTMCS1SS = pEntry->SupportVHTMCS1SS;
				raentry->u2SupportVHTMCS2SS = pEntry->SupportVHTMCS2SS;
				raentry->u2SupportVHTMCS3SS = pEntry->SupportVHTMCS3SS;
				raentry->u2SupportVHTMCS4SS = pEntry->SupportVHTMCS4SS;
				mcs = &strec->he_sta.max_nss_mcs;
				os_move_mem(mcs->bw80_mcs, pEntry->cap.rate.he80_rx_nss_mcs,
					sizeof(mcs->bw80_mcs));
				os_move_mem(mcs->bw160_mcs, pEntry->cap.rate.he160_rx_nss_mcs,
					sizeof(mcs->bw160_mcs));
				os_move_mem(mcs->bw8080_mcs, pEntry->cap.rate.he8080_rx_nss_mcs,
					sizeof(mcs->bw8080_mcs));
				pEntry->update_he_maxra = true;
				RTEnqueueInternalCmd(pAd, CMDTHREAD_UPDATE_MAXRA, pEntry, sizeof(MAC_TABLE_ENTRY));
				pEntry->fgRaLimitPhyRate = FALSE;
			}
#endif
		}

#endif /* DOT11_VHT_AC */
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */
#ifdef APCLI_SUPPORT

		if (IS_ENTRY_PEER_AP(pEntry) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
			&& (pEntry->func_tb_idx < MAX_MULTI_STA)) {
			PSTA_ADMIN_CONFIG pApCliEntry = &pAd->StaCfg[pEntry->func_tb_idx];

			pApCliEntry->StaStatistic.OneSecTxBytes = pEntry->OneSecTxBytes;
			pApCliEntry->StaStatistic.OneSecRxBytes = pEntry->OneSecRxBytes;
#ifdef WIFI_IAP_STA_DUMP_FEATURE
			if (pEntry->OneSecTxBytes || pEntry->OneSecRxBytes) {
				pEntry->NoDataIdleCount++;
			} else {
				pEntry->NoDataIdleCount = 0;
			}
#endif/*WIFI_IAP_STA_DUMP_FEATURE*/

		}

#endif /* APCLI_SUPPORT */

#if (defined(IGMP_SNOOP_SUPPORT) && defined(IGMP_SNOOPING_OFFLOAD))
		if (IS_MT7992(pAd)) {
			cr4_m2u_query_list->list[cr4_m2u_query_list->number++] = wcid;
			if (cr4_m2u_query_list->number == MAX_CR4_QUERY_NUM) {
				HW_WA_MULTIQUERY(pAd, CR4_QUERY_OPTION_GET_TX_STATISTICS, cr4_m2u_query_list->number, 0, cr4_m2u_query_list);
				cr4_m2u_query_list->number = 0;
			}

			tx_offload_m2u_counter_update(pAd, pEntry);
		}
#endif


#ifdef WHNAT_SUPPORT
		if (IS_MT7986(pAd) && PD_GET_WHNAT_ENABLE(pAd->physical_dev) && (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))) {
			cr4_query_list->list[cr4_query_list->number++] = wcid;
			if (cr4_query_list->number == MAX_CR4_QUERY_NUM) {
				HW_WA_MULTIQUERY(pAd, CR4_QUERY_OPTION_GET_TX_STATISTICS, cr4_query_list->number, 0, cr4_query_list);
				cr4_query_list->number = 0;
			}

			tx_offload_counter_update(pAd, pEntry);
#ifdef DELAY_TCP_ACK_V2 /*for panther*/
			if (IS_MT7986(pAd)) {
				UINT8 tid_num = 0;
				for (tid_num = 0; tid_num < NUM_OF_TID; tid_num++) {
					pEntry->OneSecRxBytes += (pAd->wo_rxcnt[tid_num][pEntry->wcid].rx_byte_cnt - pAd->wo_last_rxcnt[tid_num][pEntry->wcid].rx_byte_cnt);
					pAd->wo_last_rxcnt[tid_num][pEntry->wcid].rx_byte_cnt = pAd->wo_rxcnt[tid_num][pEntry->wcid].rx_byte_cnt;
#ifdef RX_COUNT_DETECT
					pEntry->one_sec_rx_pkts += (pAd->wo_rxcnt[tid_num][pEntry->wcid].rx_pkt_cnt - pAd->wo_last_rxcnt[tid_num][pEntry->wcid].rx_pkt_cnt);
					pAd->wo_last_rxcnt[tid_num][pEntry->wcid].rx_pkt_cnt = pAd->wo_rxcnt[tid_num][pEntry->wcid].rx_pkt_cnt;
#endif /* RX_COUNT_DETECT */
				}
				/* report rx pkts from wo-900ms/wa-1000ms, need transfer */
				pEntry->OneSecRxBytes = (pEntry->OneSecRxBytes * 100) / (PEAK_TP_WO_REPORT_TIME * 15);
				pEntry->one_sec_rx_pkts = (pEntry->one_sec_rx_pkts * 100) / (PEAK_TP_WO_REPORT_TIME * 15);
			}
#endif /* DELAY_TCP_ACK_V2 */
		} else
#endif
		{
			pEntry->AvgTxBytes = (pEntry->AvgTxBytes == 0) ?
								pEntry->OneSecTxBytes :
								((pEntry->AvgTxBytes + pEntry->OneSecTxBytes) >> 1);
			pEntry->avg_tx_pkts = (pEntry->avg_tx_pkts == 0) ? \
								pEntry->one_sec_tx_pkts : \
								((pEntry->avg_tx_pkts + pEntry->one_sec_tx_pkts) >> 1);
			pEntry->avg_tx_mpdu_pkts = (pEntry->avg_tx_mpdu_pkts == 0) ?
								pEntry->one_sec_tx_mpdu_pkts :
								((pEntry->avg_tx_mpdu_pkts + pEntry->one_sec_tx_mpdu_pkts) >> 1);
		}

		pEntry->AvgRxBytes = (pEntry->AvgRxBytes == 0) ?
							pEntry->OneSecRxBytes :
							((pEntry->AvgRxBytes + pEntry->OneSecRxBytes) >> 1);

#ifdef SMART_CARRIER_SENSE_SUPPORT

		if (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)) {
			pAd->SCSCtrl.OneSecRxByteCount += pEntry->OneSecRxBytes;
			pAd->SCSCtrl.OneSecTxByteCount += pEntry->OneSecTxBytes;

			if (pAd->SCSCtrl.SCSEnable == SCS_ENABLE) {
				tmpRssi = RTMPMinRssi(pAd, pEntry->RssiSample.AvgRssi[0], pEntry->RssiSample.AvgRssi[1],
									  pEntry->RssiSample.AvgRssi[2], pEntry->RssiSample.AvgRssi[3]);

				if (tmpRssi < pAd->SCSCtrl.SCSMinRssi)
					pAd->SCSCtrl.SCSMinRssi = tmpRssi;
			}
		}

#endif /* SMART_CARRIER_SENSE_SUPPORT */

#ifdef RX_COUNT_DETECT
		pEntry->avg_rx_pkts = (pEntry->avg_rx_pkts == 0) ? \
							  pEntry->one_sec_rx_pkts : \
							  ((pEntry->avg_rx_pkts + pEntry->one_sec_rx_pkts) >> 1);
#endif /* RX_COUNT_DETECT */

#ifdef PEAK_ENHANCE
		sum_txop_all_txrx_counter(pAd, pEntry);
#endif


#if (defined(ANDLINK_FEATURE_SUPPORT) && defined(ANDLINK_V4_0))
	if (pAd->CommonCfg.andlink_enable) {
		update_andlink_statistics(pAd, pEntry);
	}
#endif/*ANDLINK_SUPPORT*/

#ifdef APCLI_SUPPORT
		if (IS_ENTRY_PEER_AP(pEntry)) {
			PSTA_ADMIN_CONFIG pApCliEntry = &pAd->StaCfg[pEntry->func_tb_idx];

			if (pApCliEntry->ApcliInfStat.Valid == TRUE) {
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
				{
					if (cap->fgRateAdaptFWOffload == TRUE) {
						if (!pEntry->bTxPktChk &&
							RTMP_TIME_AFTER(pAd->Mlme.Now32, pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime + (1 * OS_HZ))) {
							pEntry->bTxPktChk = TRUE;
							pEntry->TotalTxSuccessCnt = 0;
							pEntry->TxStatRspCnt = 0;
						} else if (pEntry->bTxPktChk) {
							 if ((pEntry->TxStatRspCnt >= 1) && (pEntry->TotalTxSuccessCnt)) {
									pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime = pAd->Mlme.Now32;
#ifdef DOT11_EHT_BE
									sta_mld_update_rx_bcn_time(&pApCliEntry->wdev, ML_APC_BCN_RX_TIME);
#endif/*DOT11_EHT_BE*/
									pEntry->bTxPktChk = FALSE;
							 }
						}
					}
				}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
			}
		}
#endif /* APCLI_SUPPORT */
#ifdef ERR_RECOVERY
        if (!IsStopingPdma(&pAd->ErrRecoveryCtl))
#endif
		{
			if (p_tx_statics_queue) {
				if (tx_statics_qcnt < MAX_FW_AGG_MSG) {
					p_temp = p_tx_statics_queue + tx_statics_qcnt;
					p_temp->Field = GET_TX_STAT_ENTRY_TX_CNT;
					p_temp->Band = BandIdx;
					p_temp->Wcid = wcid;
					tx_statics_qcnt++;
				}

				if (!IS_MT7990(pAd) && !IS_MT7992(pAd) && !IS_MT7993(pAd) && tx_statics_qcnt == MAX_FW_AGG_MSG) {
					HW_GET_TX_STATISTIC(pAd, p_tx_statics_queue, MAX_FW_AGG_MSG);
					tx_statics_qcnt = 0;
					os_zero_mem(p_tx_statics_queue, sizeof(TX_STAT_STRUC) * MAX_FW_AGG_MSG);
				}
			}
		}
	/*notify tput detect*/
	call_traffic_notifieriers(TRAFFIC_NOTIFY_TPUT_DETECT, pAd, pEntry);

#ifdef APCLI_SUPPORT

		if ((IS_ENTRY_PEER_AP(pEntry)/* || IS_ENTRY_REPEATER(pEntry)*/)
			&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
		   ) {
			sta_cfg = GetStaCfgByWdev(pAd, pEntry->wdev);
#ifdef MAC_REPEATER_SUPPORT

			if (IS_REPT_LINK_UP(pEntry->pReptCli)) {
				pEntry->pReptCli->ReptCliIdleCount++;

				if (IS_REPT_CLI_TYPE(pEntry->pReptCli, REPT_ETH_CLI)
					&& (pEntry->pReptCli->ReptCliIdleCount >= MAC_TABLE_AGEOUT_TIME)
					&& (!IS_REPT_CLI_TYPE(pEntry->pReptCli, REPT_BRIDGE_CLI))) { /* Do NOT ageout br0 link. @2016/1/27 */
					RepeaterDisconnectRootAP(pAd, pEntry->pReptCli, APCLI_DISCONNECT_SUB_REASON_MTM_REMOVE_STA);
					continue;
				}
			}

#endif /* MAC_REPEATER_SUPPORT */

			if (IS_ENTRY_PEER_AP(pEntry))
				apcli_wdev = pEntry->wdev;

			apcli_avg_tx += pEntry->AvgTxBytes;
			apcli_avg_rx += pEntry->AvgRxBytes;
			if (pEntry->wdev) {
				UINT32 tx_tp = (pEntry->AvgTxBytes >> BYTES_PER_SEC_TO_MBPS);
				UINT32 rx_tp = (pEntry->AvgRxBytes >> BYTES_PER_SEC_TO_MBPS);
				UINT8 traffc_mode = 0;

				if ((tx_tp + rx_tp) == 0)
					traffc_mode = TRAFFIC_0;
				else if (((tx_tp * 100) / (tx_tp + rx_tp)) > TX_MODE_RATIO_THRESHOLD)
					traffc_mode = TRAFFIC_DL_MODE;
				else if (((rx_tp * 100) / (tx_tp + rx_tp)) > RX_MODE_RATIO_THRESHOLD)
					traffc_mode = TRAFFIC_UL_MODE;

				/* Detect main STA(Maximum TP of TX+RX) traffic mode */
				peak_tp_ctl = &pAd->peak_tp_ctl;

				pAd->txop_ctl.cur_wdev = pEntry->wdev;
#ifdef PEAK_ENHANCE
#ifdef DOT11_EHT_BE
				if (pEntry->mlo.mlo_en) {
					pAd->txop_ctl.mlo_entry = TRUE;
					peak_tp_ctl->main_entry = pEntry;
				}
#endif /* DOT11_EHT_BE */
#endif /* PEAK_ENHANCE */
				if (peak_tp_ctl->main_wdev == NULL)
					peak_tp_ctl->main_wdev = pEntry->wdev;
				peak_tp_ctl->client_nums++;
				if ((tx_tp + rx_tp) > (peak_tp_ctl->main_tx_tp + peak_tp_ctl->main_rx_tp)) {
					peak_tp_ctl->main_tx_tp = tx_tp;
					peak_tp_ctl->main_rx_tp = rx_tp;
					peak_tp_ctl->main_traffc_mode = traffc_mode;
					peak_tp_ctl->main_entry = pEntry;
					peak_tp_ctl->main_wdev = pEntry->wdev;
				}
			}

			if ((pAd->Mlme.OneSecPeriodicRound % 10) == 8) {
				/* use Null or QoS Null to detect the ACTIVE station*/
				BOOLEAN ApclibQosNull = FALSE;

				if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) {
					ApclibQosNull = TRUE;
				}

				if (sta_cfg) {
					if (!sta_cfg->PwrMgmt.bDoze) {
						USHORT PwrMgmt = PWR_ACTIVE;
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
						if (twtPlannerIsRunning(pAd, sta_cfg))
							PwrMgmt = PWR_SAVE;
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
						ApCliRTMPSendNullFrame(pAd, pEntry->CurrTxRate,
							ApclibQosNull, pEntry, PwrMgmt);
					}
				}
				continue;
			}
		}

#endif /* APCLI_SUPPORT */

		if (!IS_ENTRY_CLIENT(pEntry)) {
			pEntry->one_sec_tx_succ_pkts = 0;
			continue;
		}

#ifdef DOT11_EHT_BE
		mt_rcu_read_lock();

		if (IS_ENTRY_MLO(pEntry)) {
			struct mld_entry_t *mld_entry;

			mld_entry = rcu_dereference(pEntry->mld_entry);
			if (!mld_entry) {
				mt_rcu_read_unlock();
				continue;
			}
			ba_info = &mld_entry->ba_info;
		}
#endif /* DOT11_EHT_BE */

		if (pEntry->fgGband256QAMSupport && (ba_info->RxBitmap != 0) && (ba_info->TxBitmap != 0) &&
			sta_hit_2g_infra_case_number <= STA_NUMBER_FOR_TRIGGER
#ifdef PEAK_ENHANCE
			&& !(ctrl->enable_adjust & ENABLE_MLO_ENTRY_ADJUST_ONLY)
#endif
			) {
			sta_wdev = pEntry->wdev;
			ch_band = 0xff;
			if (sta_wdev)
				ch_band = wlan_config_get_ch_band(sta_wdev);
			if (ch_band == CMD_CH_BAND_24G) {
				UINT tx_tp = (pEntry->AvgTxBytes >> BYTES_PER_SEC_TO_MBPS);
				UINT rx_tp = (pEntry->AvgRxBytes >> BYTES_PER_SEC_TO_MBPS);

				if (tx_tp > INFRA_TP_PEEK_BOUND_THRESHOLD && ((tx_tp + rx_tp) > 0) &&
					(tx_tp * 100) / (tx_tp + rx_tp) > TX_MODE_RATIO_THRESHOLD) {
					if (sta_hit_2g_infra_case_number < STA_NUMBER_FOR_TRIGGER) {
						txop_wdev = sta_wdev;
						sta_hit_2g_infra_case_number++;
					} else
						sta_hit_2g_infra_case_number++;
				}
			}
		}

#ifdef DOT11_EHT_BE
		mt_rcu_read_unlock();
#endif

		if (pEntry->wdev) {
			UINT32 tx_tp = (pEntry->AvgTxBytes >> BYTES_PER_SEC_TO_MBPS);
			UINT32 rx_tp = (pEntry->AvgRxBytes >> BYTES_PER_SEC_TO_MBPS);
			ULONG avg_tx_b = pEntry->AvgTxBytes;
			ULONG avg_rx_b = pEntry->AvgRxBytes;
			UINT8 traffc_mode = TRAFFIC_0;
#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
			ULONG data_rate = 0;
			UINT32 tp_ratio = 0;
			UINT8 bidir_traffc_mode = 0;
#endif

			if ((avg_tx_b + avg_rx_b) > 0) {
				tx_ratio = TX_MODE_RATIO_THRESHOLD;
				if (pEntry->avg_rx_pkt_len < 256)
					rx_ratio = 50;
				else
					rx_ratio = RX_MODE_RATIO_THRESHOLD;
				is_tx_traffic = (avg_tx_b > (((avg_tx_b + avg_rx_b) / 10) * (tx_ratio / 10))) ? TRUE : FALSE;
				is_rx_traffic = (avg_rx_b > (((avg_tx_b + avg_rx_b) / 10) * (rx_ratio / 10))) ? TRUE : FALSE;
			} else {
				is_tx_traffic = FALSE;
				is_rx_traffic = FALSE;
				traffc_mode = TRAFFIC_0;
			}

			if (is_tx_traffic)
				traffc_mode = TRAFFIC_DL_MODE;
			else if (is_rx_traffic)
				traffc_mode = TRAFFIC_UL_MODE;

			/* Detect main STA(Maximum TP of TX+RX) traffic mode */
			peak_tp_ctl = &pAd->peak_tp_ctl;
			if (peak_tp_ctl->main_wdev == NULL)
				peak_tp_ctl->main_wdev = pEntry->wdev;
			peak_tp_ctl->client_nums++;
			if ((avg_tx_b + avg_rx_b) > peak_tp_ctl->main_txrx_bytes) {
				peak_tp_ctl->main_tx_tp = tx_tp;
				peak_tp_ctl->main_rx_tp = rx_tp;
#ifdef DELAY_TCP_ACK_V2
				peak_tp_ctl->main_tx_tp_record = tx_tp;
				peak_tp_ctl->main_rx_tp_record = rx_tp;
#endif /* DELAY_TCP_ACK_V2 */

				peak_tp_ctl->main_txrx_bytes = avg_tx_b + avg_rx_b;
				peak_tp_ctl->main_traffc_mode = traffc_mode;
				peak_tp_ctl->main_entry = pEntry;
				peak_tp_ctl->main_wdev = pEntry->wdev;
			}

#ifdef CONFIG_TX_DELAY
			if (IS_TX_DELAY_SW_MODE(cap) && !pAd->CommonCfg.dbdc_mode) {
				struct tx_delay_control *tx_delay_ctl = NULL;
				struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);

				if (qm_ops->get_qm_delay_ctl) {
					tx_delay_ctl = qm_ops->get_qm_delay_ctl(pAd, BandIdx);

					if ((rx_tp >= tx_delay_ctl->min_tx_delay_en_tp) &&
					    (rx_tp <= tx_delay_ctl->max_tx_delay_en_tp)) {
						tx_delay_ctl->que_agg_en = TRUE;
					} else {
						tx_delay_ctl->que_agg_en = FALSE;
					}
				}
			}
#endif /* CONFIG_TX_DELAY */

#ifndef HWIFI_SUPPORT
#ifdef RTMP_MAC_PCI
			if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
				if (IS_ASIC_CAP(pAd, fASIC_CAP_DLY_INT_LUMPED)
					|| IS_ASIC_CAP(pAd, fASIC_CAP_DLY_INT_PER_RING)) {
					pci_dynamic_dly_int_adjust(pAd->hdev_ctrl, tx_tp, rx_tp);
				}
			}
#endif
#endif /* !HWIFI_SUPPORT */

			if (pAd->mcli_ctl.debug_on & MCLI_DEBUG_PER_STA) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
					"STA%d:avg_tx_b:%lu,avg_rx_b:%lu,avg_tx_pkts:%lu,avg_rx_pkts:%lu,rx_ratio:%u\n",
					pEntry->wcid, avg_tx_b, avg_rx_b, pEntry->avg_tx_pkts, pEntry->avg_rx_pkts, rx_ratio);
			}
#ifdef PEAK_ENHANCE
#ifdef DOT11_EHT_BE
			if (pEntry->mlo.mlo_en) {
				sta_hit_2g_infra_case_number = 0xFF;
				pAd->txop_ctl.mlo_entry = TRUE;
			}
#endif /* DOT11_EHT_BE */
#endif /* PEAK_ENHANCE */
				pAd->txop_ctl.cur_wdev = pEntry->wdev;
			ch_band = wlan_config_get_ch_band(pEntry->wdev);
			if (ch_band == CMD_CH_BAND_5G || ch_band == CMD_CH_BAND_6G) {
				if ((pEntry->avg_tx_pkts > VERIWAVE_5G_PKT_CNT_TH || avg_tx_b > MCLI_TRAFFIC_TP_TH * 5) && is_tx_traffic)
					pAd->txop_ctl.multi_client_nums++;
			}
#ifdef RX_COUNT_DETECT
			if (ch_band == CMD_CH_BAND_5G || ch_band == CMD_CH_BAND_6G) {
				if ((pEntry->avg_rx_pkts > VERIWAVE_5G_PKT_CNT_TH || avg_rx_b > MCLI_TRAFFIC_TP_TH * 5) && is_rx_traffic)
					pAd->txop_ctl.multi_rx_client_nums++;
			}
#endif

			if (ch_band == CMD_CH_BAND_24G) {
				if ((pEntry->avg_tx_pkts > VERIWAVE_2G_PKT_CNT_TH || avg_tx_b > MCLI_TRAFFIC_TP_TH) && is_tx_traffic)
					pAd->txop_ctl.multi_client_nums++;
			}
#ifdef RX_COUNT_DETECT
			if (ch_band == CMD_CH_BAND_24G) {
				if ((pEntry->avg_rx_pkts > VERIWAVE_2G_PKT_CNT_TH || avg_rx_b > MCLI_TRAFFIC_TP_TH) && is_rx_traffic)
					pAd->txop_ctl.multi_rx_client_nums++;
			}
#endif
#ifdef VOW_SUPPORT
			/* check if having TCP flow per STA while SPL is disabled */
			if ((pAd->vow_gen.VOW_GEN == VOW_GEN_TALOS) &&
			    (pAd->vow_misc_cfg.spl_sta_count == 0))
				check_sta_tcp_flow(pAd, tr_entry, BandIdx);

			if (pAd->vow_cfg.mcli_schedule_en && pAd->Mlme.OneSecPeriodicRound % 3 == 0) {
				if ((is_tx_traffic && pEntry->mcliTcpCnt >= pAd->vow_cfg.mcli_sch_cfg.tcp_cnt_th)
					|| (is_rx_traffic && pEntry->mcliTcpAckCnt >= pAd->vow_cfg.mcli_sch_cfg.tcp_cnt_th))
					pAd->vow_cfg.mcli_sch_cfg.mcli_tcp_num++;

				if (pAd->mcli_ctl.debug_on & MCLI_DEBUG_PER_STA)
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
						"STA%d: TxRxtraffic(%u/%u), Tcp/Ack(%d/%d)\n",
						pEntry->wcid, is_tx_traffic, is_rx_traffic,
						pEntry->mcliTcpCnt, pEntry->mcliTcpAckCnt);

				pEntry->mcliTcpCnt = 0;
				pEntry->mcliTcpAckCnt = 0;
			}
#endif

			avg_pkt_len_calculate(pEntry, BandIdx);
			CalFarClientNum(pAd, pEntry, BandIdx);

			if (pEntry->one_sec_tx_succ_pkts > INFRA_KEEP_STA_PKT_TH && pEntry->NoDataIdleCount != 0) {
				pEntry->NoDataIdleCount = 0;
				pEntry->one_sec_tx_succ_pkts = 0;
				is_no_rx_cnt = TRUE;
			}

#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
			if (IS_MAP_ENABLE(pAd)) {
				getRate(pEntry->HTPhyMode, &data_rate);
				tp_ratio = ((tx_tp + rx_tp) * 100) / data_rate;

				if (tp_ratio > STA_TP_IDLE_THRESHOLD)
					bidir_traffc_mode = TRAFFIC_BIDIR_ACTIVE_MODE;
				else
					bidir_traffc_mode = TRAFFIC_BIDIR_IDLE_MODE;

				if ((pEntry->pre_traffic_mode == TRAFFIC_BIDIR_ACTIVE_MODE) &&
					(tp_ratio <= STA_TP_IDLE_THRESHOLD))
					wapp_send_cli_active_change(pAd, pEntry, INACTIVE);
				else if ((pEntry->pre_traffic_mode == TRAFFIC_BIDIR_IDLE_MODE) &&
					(tp_ratio > STA_TP_IDLE_THRESHOLD))
					wapp_send_cli_active_change(pAd, pEntry, ACTIVE);

				pEntry->pre_traffic_mode = bidir_traffc_mode;
			}
#endif /*WAPP_SUPPORT CONFIG_MAP_SUPPORT*/
		}

		if (pEntry->NoDataIdleCount == 0) {
			pEntry->StationKeepAliveCount = 0;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
			pEntry->bTxPktChk = FALSE;
			pEntry->ContinueTxFailCnt = 0;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
		}

#if defined(DFS_VENDOR10_CUSTOM_FEATURE) || defined(CCN34_SPLIT_MAC_SUPPORT)
		pEntry->LastRxTimeCount++;
#endif

		pEntry->NoDataIdleCount++;
		tr_entry->NoDataIdleCount++;
		pEntry->StaConnectTime++;
		pMbss = (pEntry->func_tb_idx < MAX_MBSSID_NUM(pAd)) ? &pAd->ApCfg.MBSSID[pEntry->func_tb_idx]:NULL;

		/* 0. STA failed to complete association should be removed to save MAC table space. */
		if ((pEntry->Sst != SST_ASSOC) && (pEntry->NoDataIdleCount >= pEntry->AssocDeadLine) && !pEntry->sta_force_keep) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					 MACSTR" fail to complete ASSOC in %lu sec\n",
					  MAC2STR(pEntry->Addr), pEntry->AssocDeadLine);
#ifdef CUSTOMISED_MGMT_TRANS_SUPPORT
			pEntry->del_reason = MTK_NL80211_VENDOR_DISC_NO_DATA_ASSOC_TIMEOUT;
#endif

#ifdef WSC_AP_SUPPORT

			if ((pMbss != NULL) && NdisEqualMemory(pEntry->Addr, pMbss->wdev.WscControl.EntryAddr, MAC_ADDR_LEN))
				NdisZeroMemory(pMbss->wdev.WscControl.EntryAddr, MAC_ADDR_LEN);

#endif /* WSC_AP_SUPPORT */
			mac_entry_delete(pAd, pEntry, TRUE);
			continue;
		} else {
			do_sta_keep_action(pAd, pEntry, is_no_rx_cnt);
		}
		/*
			1. check if there's any associated STA in power-save mode. this affects outgoing
				MCAST/BCAST frames should be stored in PSQ till DtimCount=0
		*/
		if (pEntry->PsMode == PWR_SAVE) {
			pMacTable->fAnyStationInPsm = TRUE;

			if (pEntry->wdev &&
				(pEntry->wdev->wdev_type == WDEV_TYPE_AP || pEntry->wdev->wdev_type == WDEV_TYPE_GO)
				&& (pEntry->wdev->tr_tb_idx < WTBL_MAX_NUM(pAd))) {
				/* TODO: it looks like pEntry->wdev->tr_tb_idx is not assigned? */
				struct _STA_TR_ENTRY *bmc_tr_entry;

				bmc_tr_entry = tr_entry_get(pAd, pEntry->wdev->tr_tb_idx);
				bmc_tr_entry->PsMode = PWR_SAVE;

				if (tr_entry->PsDeQWaitCnt) {
					tr_entry->PsDeQWaitCnt++;

					if (tr_entry->PsDeQWaitCnt > 2)
						tr_entry->PsDeQWaitCnt = 0;
				}
			}
		}

#ifdef DOT11_N_SUPPORT

		if (pEntry->MmpsMode == MMPS_DYNAMIC)
			pMacTable->fAnyStationMIMOPSDynamic = TRUE;

		if (pEntry->MaxHTPhyMode.field.BW == BW_20)
			pMacTable->fAnyStation20Only = TRUE;

		if (pEntry->MaxHTPhyMode.field.MODE != MODE_HTGREENFIELD) {
			if (pEntry->wdev) {
				UINT16 non_gf_sta = wlan_operate_get_non_gf_sta(wdev);

				non_gf_sta++;
				wlan_operate_set_non_gf_sta(wdev, non_gf_sta);
				if (BandIdx < ARRAY_SIZE(pMacTable->fAnyStationNonGF))
					pMacTable->fAnyStationNonGF[BandIdx] = TRUE;
			}
		}

		if ((pEntry->MaxHTPhyMode.field.MODE == MODE_OFDM) || (pEntry->MaxHTPhyMode.field.MODE == MODE_CCK))
			pMacTable->fAnyStationIsLegacy = TRUE;

#ifdef DOT11N_DRAFT3

		if (pEntry->bForty_Mhz_Intolerant)
			pMacTable->fAnyStaFortyIntolerant = TRUE;

#endif /* DOT11N_DRAFT3 */

#endif /* DOT11_N_SUPPORT */
		/* detect the station alive status */

		/* detect the station alive status */
		if ((pMbss != NULL) && (pMbss->StationKeepAliveTime > 0) &&
			(pEntry->NoDataIdleCount >= pMbss->StationKeepAliveTime)) {
			/*
				If no any data success between ap and the station for
				StationKeepAliveTime, try to detect whether the station is
				still alive.

				Note: Just only keepalive station function, no disassociation
				function if too many no response.
			*/

			/*
				For example as below:

				1. Station in ACTIVE mode,

			    ......
			    sam> tx ok!
			    sam> count = 1!	 ==> 1 second after the Null Frame is acked
			    sam> count = 2!	 ==> 2 second after the Null Frame is acked
			    sam> count = 3!
			    sam> count = 4!
			    sam> count = 5!
			    sam> count = 6!
			    sam> count = 7!
			    sam> count = 8!
			    sam> count = 9!
			    sam> count = 10!
			    sam> count = 11!
			    sam> count = 12!
			    sam> count = 13!
			    sam> count = 14!
			    sam> count = 15! ==> 15 second after the Null Frame is acked
			    sam> tx ok!      ==> (KeepAlive Mechanism) send a Null Frame to
										detect the STA life status
			    sam> count = 1!  ==> 1 second after the Null Frame is acked
			    sam> count = 2!
			    sam> count = 3!
			    sam> count = 4!
			    ......

				If the station acknowledges the QoS Null Frame,
				the NoDataIdleCount will be reset to 0.


				2. Station in legacy PS mode,

				We will set TIM bit after 15 seconds, the station will send a
				PS-Poll frame and we will send a QoS Null frame to it.
				If the station acknowledges the QoS Null Frame, the
				NoDataIdleCount will be reset to 0.


				3. Station in legacy UAPSD mode,

				Currently we do not support the keep alive mechanism.
				So if your station is in UAPSD mode, the station will be
				kicked out after 300 seconds.

				Note: the rate of QoS Null frame can not be 1M of 2.4GHz or
				6M of 5GHz, or no any statistics count will occur.
			*/
			if (pEntry->StationKeepAliveCount++ == 0) {
				/* use Null or QoS Null to detect the ACTIVE station */
				BOOLEAN bQosNull = FALSE;
				BOOLEAN bTx_null = TRUE;

#ifdef DOT11_EHT_BE
				if (!IS_ENTRY_MLO(pEntry))
					bTx_null = TRUE;
				else if (IS_ENTRY_MLO(pEntry) && !pEntry->mlo.is_setup_link_entry)
					bTx_null = FALSE;
#endif

				if (bTx_null == TRUE) {
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
					if (!pEntry->bTxPktChk) {
						pEntry->TotalTxSuccessCnt = 0;
						pEntry->TxStatRspCnt = 0;
						pEntry->bTxPktChk = TRUE;
					}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

					if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
						bQosNull = TRUE;
#ifdef CONFIG_AP_SUPPORT
					MTWF_DBG(pAd, DBG_CAT_AP, CATAP_KPLIVE, DBG_LVL_DEBUG, "AP send null frame to wcid=%d(%d,%d,%d,%d)dev:"MACSTR" A1:"MACSTR" A2:"MACSTR" A3:"MACSTR"\n\r",
						pEntry->wcid,
#ifdef DOT11_EHT_BE
						IS_ENTRY_MLO(pEntry), pEntry->mlo.is_setup_link_entry,
#else
						0, 0,
#endif
						pEntry->func_tb_idx,
						pEntry->apidx, MAC2STR(pEntry->wdev->if_addr), MAC2STR(pEntry->Addr),
						MAC2STR(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid),
						MAC2STR(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.if_addr));
#endif
#ifdef FORCE_NULL_TX
					RtmpEnqueueForceNullFrame(pAd, pEntry->Addr, pEntry->CurrTxRate,
							pEntry->wcid, pEntry->func_tb_idx, bQosNull, TRUE, 0);
#else
					RtmpEnqueueNullFrame(pAd, pEntry->Addr, pEntry->CurrTxRate,
							pEntry->wcid, pEntry->func_tb_idx, bQosNull, TRUE, 0);
#endif
					chip_do_extra_action(
						pAd, NULL, pEntry->Addr,
						CHIP_EXTRA_ACTION_IDLE_DETECT, NULL, NULL);
				}
			} else {
				if (pEntry->StationKeepAliveCount >= pMbss->StationKeepAliveTime) {
					pEntry->StationKeepAliveCount = 0;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
					pEntry->bTxPktChk = FALSE;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
				}
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
				else if (pEntry->bTxPktChk) {
					if ((pEntry->TxStatRspCnt >= 1) && (pEntry->TotalTxSuccessCnt > 0)) {
						pEntry->NoDataIdleCount = 0;
						pEntry->StationKeepAliveCount = 0;
						pEntry->ContinueTxFailCnt = 0;
						pEntry->bTxPktChk = FALSE;
					}
				}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
			}
		}

		/* 2. delete those MAC entry that has been idle for a long time */

#if defined(DOT11_HE_AX) && defined(WIFI_TWT_SUPPORT)
		/* If TWT agreement is present for this STA, add maximum TWT wake up interval in sta idle timeout. */
		if (GET_PEER_ITWT_FID_BITMAP(pEntry)) {
			sta_idle_timeout = pEntry->StaIdleTimeout + pEntry->twt_ctrl.twt_interval_max;
		} else
#endif
		{
			sta_idle_timeout = pEntry->StaIdleTimeout;
		}

		if (pEntry->NoDataIdleCount >= sta_idle_timeout) {
#ifdef VERIFICATION_MODE
			if (pAd->veri_ctrl.skip_ageout == TRUE) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
				"skip ageout: "MACSTR" aleady slient for %d-sece\n",
					MAC2STR(pEntry->Addr),
					(UINT32)pEntry->NoDataIdleCount);
			} else
#endif /* VERIFICATION_MODE */
#ifdef DOT11_EHT_BE
			if (is_mld_link_entry_on_traffic(pEntry)) {
				/* ignore age-out mlo link if any link is on traffic*/
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
					"skip ageout: "MACSTR
					" when related MLO entry has traffic\n",
					MAC2STR(pEntry->Addr));
			} else
#endif
			{
				bDisconnectSta = TRUE;
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_NOTICE,
					"ageout "MACSTR" after %d-sec silence\n",
						 MAC2STR(pEntry->Addr), sta_idle_timeout);
#ifdef CUSTOMISED_MGMT_TRANS_SUPPORT
				pEntry->del_reason = MTK_NL80211_VENDOR_DISC_NO_DATA_TIMEOUT;
#endif
				ApLogEvent(pAd, pEntry->Addr, EVENT_AGED_OUT);
#ifdef WH_EVENT_NOTIFIER
				if (pEntry) {
					EventHdlr pEventHdlrHook = NULL;

					pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_TIMEOUT);

					if (pEventHdlrHook && pEntry->wdev)
						pEventHdlrHook(pAd, pEntry);
				}
#endif /* WH_EVENT_NOTIFIER */
#ifdef WIFI_DIAG
				if (pEntry && IS_ENTRY_CLIENT(pEntry))
					diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
						DIAG_CONN_FRAME_LOST, REASON_AGING_TIME_OUT);
#endif
#ifdef CONN_FAIL_EVENT
				if (IS_ENTRY_CLIENT(pEntry))
					ApSendConnFailMsg(pAd,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
						pEntry->Addr,
						REASON_DEAUTH_STA_LEAVING);
#endif
			}
		}

#ifdef EAP_STATS_SUPPORT
		if (pEntry->txm.per_err_times && pEntry->txm.contd_fail_cnt) {
			if (pEntry->txm.per_err_times >= txm_thres.per_err_total &&
				pEntry->txm.contd_fail_cnt >= txm_thres.tx_contd_fail_total)
				bDisconnectSta = TRUE;
#ifdef DOT11_EHT_BE
			if (check_mld_link_entry_per_times(pEntry))
				bDisconnectSta = TRUE;
#endif
			if (bDisconnectSta) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_NOTICE,
						"###notice, will delete mac entry of "MACSTR"\n",
						 MAC2STR(pEntry->Addr));
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_NOTICE,
						"wcid %d, pEntry->txm.per_err_times %d, pEntry->txm.contd_fail_cnt %ld\n",
						pEntry->wcid, pEntry->txm.per_err_times, pEntry->txm.contd_fail_cnt);
#ifdef CUSTOMISED_MGMT_TRANS_SUPPORT
				pEntry->del_reason = MTK_NL80211_VENDOR_DISC_NO_DATA_TIMEOUT;
#endif
				ApLogEvent(pAd, pEntry->Addr, EVENT_AGED_OUT);
#ifdef WH_EVENT_NOTIFIER
				if (pEntry) {
					EventHdlr pEventHdlrHook = NULL;

					pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_TIMEOUT);

					if (pEventHdlrHook && pEntry->wdev)
						pEventHdlrHook(pAd, pEntry);
				}
#endif /* WH_EVENT_NOTIFIER */
#ifdef WIFI_DIAG
				if (pEntry && IS_ENTRY_CLIENT(pEntry))
					diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
						DIAG_CONN_FRAME_LOST, REASON_AGING_TIME_OUT);
#endif
#ifdef CONN_FAIL_EVENT
				if (IS_ENTRY_CLIENT(pEntry))
					ApSendConnFailMsg(pAd,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
						pEntry->Addr,
						REASON_DEAUTH_STA_LEAVING);
#endif
			}
		}
#endif
#ifdef RT_CFG80211_SUPPORT
#endif
		avgRssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);

		if ((skip_kick == FALSE) &&
			(pMbss != NULL) && pEntry->RssiSample.Rssi_Updated && pMbss->RssiLowForStaKickOut &&
			(avgRssi < pMbss->RssiLowForStaKickOut)) {
			bDisconnectSta = TRUE;
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_NOTICE,
					 "Disassoc STA "MACSTR", RSSI Kickout Thres[%d]-[%d]\n",
					  MAC2STR(pEntry->Addr), pMbss->RssiLowForStaKickOut,	avgRssi);
#ifdef CUSTOMISED_MGMT_TRANS_SUPPORT
			pEntry->del_reason = MTK_NL80211_VENDOR_DISC_RSSI_LOW;
#endif
#ifdef WIFI_DIAG
			if (pEntry && IS_ENTRY_CLIENT(pEntry))
				diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
					DIAG_CONN_DEAUTH, REASON_RSSI_TOO_LOW);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					REASON_DEAUTH_STA_LEAVING);
#endif
		}

		if (bDisconnectSta && (pMbss != NULL)) {
			/* send wireless event - for ageout */
			RTMPSendWirelessEvent(pAd, IW_AGEOUT_EVENT_FLAG, pEntry->Addr, 0, 0);

			if (pEntry->Sst == SST_ASSOC) {
				PUCHAR pOutBuffer = NULL;
				NDIS_STATUS NStatus;
				ULONG FrameLen = 0;
				HEADER_802_11 DeAuthHdr;
				USHORT Reason;
				/*  send out a DISASSOC request frame */
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

				if (NStatus != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
						"MlmeAllocateMemory fail  ..\n");
					/*NdisReleaseSpinLock(&pAd->MacTabLock); */
					continue;
				}

				Reason = REASON_DEAUTH_STA_LEAVING;
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
					"Send DEAUTH - Reason = %d frame TO %x %x %x %x %x %x\n",
					Reason, PRINT_MAC(pEntry->Addr));
				MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->Addr,
								 pMbss->wdev.if_addr,
								 pMbss->wdev.bssid);
				MakeOutgoingFrame(pOutBuffer, &FrameLen,
								  sizeof(HEADER_802_11), &DeAuthHdr,
								  2, &Reason,
								  END_OF_ARGS);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
				MlmeFreeMemory(pOutBuffer);
			}

			mac_entry_delete(pAd, pEntry, FALSE);
			continue;
		}

#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM)
	/* SW STA : when S/W STA on , SKIP the BTM method from daemon to Kick STA */
	if ((skip_kick == FALSE) && (pEntry->BTMDisassocCount == 1) && (pMbss != NULL)) {
		PUCHAR      pOutBuffer = NULL;
		NDIS_STATUS NStatus;
		ULONG       FrameLen = 0;
		HEADER_802_11 DisassocHdr;
		USHORT      Reason;
		/*  send out a DISASSOC request frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"MlmeAllocateMemory fail  ..\n");
			/*NdisReleaseSpinLock(&pAd->MacTabLock); */
			continue;
		}

		Reason = REASON_DISASSOC_INACTIVE;
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
				 "BTM ASSOC - Send DISASSOC  Reason = %d frame  TO %x %x %x %x %x %x\n", Reason, pEntry->Addr[0], pEntry->Addr[1],
				  pEntry->Addr[2], pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]);
		MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pEntry->Addr, pMbss->wdev.if_addr, pMbss->wdev.bssid);
		MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
		MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen, NULL);
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

		/* JERRY */
		if (!pEntry->IsKeep)
			mac_entry_delete(pAd, pEntry, TRUE);

		continue;
	}

	if (pEntry->BTMDisassocCount != 0)
		pEntry->BTMDisassocCount--;
#endif /* CONFIG_HOTSPOT_R2 */

		/* 3. garbage collect the ps_queue if the STA has being idle for a while */
		if ((pEntry->PsMode == PWR_SAVE) && (tr_entry->ps_state == APPS_RETRIEVE_DONE ||
											 tr_entry->ps_state == APPS_RETRIEVE_IDLE)) {
			if (tr_entry->enqCount > 0) {
				tr_entry->PsQIdleCount++;

				if (tr_entry->PsQIdleCount > 5) {
					struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);

					if (qm_ops->sta_clean_queue)
						qm_ops->sta_clean_queue(pAd, pEntry->wcid);

					tr_entry->PsQIdleCount = 0;
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "Clear WCID[%d] packets\n", pEntry->wcid);
				}
			}
		} else
			tr_entry->PsQIdleCount = 0;

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

		if ((pEntry->BSS2040CoexistenceMgmtSupport)
			&& (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
			&& (pAd->CommonCfg.bBssCoexEnable == TRUE)
		   )
			SendNotifyBWActionFrame(pAd, pEntry->wcid, pEntry->func_tb_idx);

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

		/* only apply burst when run in MCS0,1,8,9,16,17, not care about phymode */
		if ((pEntry->HTPhyMode.field.MCS != 32) &&
			((pEntry->HTPhyMode.field.MCS % 8 == 0) || (pEntry->HTPhyMode.field.MCS % 8 == 1)))
			pMacTable->fAllStationGainGoodMCS = FALSE;

		/* Check Current STA's Operation Mode is BW20 or BW40 */
		pMacTable->fCurrentStaBw40 = (pEntry->HTPhyMode.field.BW == BW_40) ? TRUE : FALSE;
#ifdef WH_EVENT_NOTIFIER

		if (pAd->ApCfg.EventNotifyCfg.bStaRssiDetect) {
			avgRssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);

			if (avgRssi < pAd->ApCfg.EventNotifyCfg.StaRssiDetectThreshold) {
				if (pEntry && IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
					&& !IS_ENTRY_A4(pEntry)
#endif /* A4_CONN */
					&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
				   ) {
					EventHdlr pEventHdlrHook = NULL;

					pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_RSSI_TOO_LOW);

					if (pEventHdlrHook && pEntry->wdev)
						pEventHdlrHook(pAd, pEntry->wdev, pEntry->Addr, avgRssi);
				}
			}
		}
#endif
#if (defined(WH_EVENT_NOTIFIER) || (defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)))
		if (pEntry && IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
			&& !IS_ENTRY_A4(pEntry)
#endif /* A4_CONN */
			&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
		   ) {
#ifdef WH_EVENT_NOTIFIER
			EventHdlr pEventHdlrHook = NULL;
			struct EventNotifierCfg *pEventNotifierCfg = &pAd->ApCfg.EventNotifyCfg;

			if (pEventNotifierCfg->bStaStateTxDetect && (pEventNotifierCfg->StaTxPktDetectPeriod > 0)) {
				pEventNotifierCfg->StaTxPktDetectRound++;

				if (((pEventNotifierCfg->StaTxPktDetectRound % pEventNotifierCfg->StaTxPktDetectPeriod) == 0)) {
					if ((pEntry->tx_state.CurrentState == WHC_STA_STATE_ACTIVE) &&
						(pEntry->tx_state.PacketCount < pEventNotifierCfg->StaStateTxThreshold)) {
						pEntry->tx_state.CurrentState = WHC_STA_STATE_IDLE;
						pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);

						if (pEventHdlrHook && pEntry->wdev)
							pEventHdlrHook(pAd, pEntry, TRUE);
					} else if ((pEntry->tx_state.CurrentState == WHC_STA_STATE_IDLE) &&
							   (pEntry->tx_state.PacketCount >= pEventNotifierCfg->StaStateTxThreshold)) {
						pEntry->tx_state.CurrentState = WHC_STA_STATE_ACTIVE;
						pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);

						if (pEventHdlrHook && pEntry->wdev)
							pEventHdlrHook(pAd, pEntry, TRUE);
					}

					pEventNotifierCfg->StaTxPktDetectRound = 0;
					pEntry->tx_state.PacketCount = 0;
				}
			}

			if (pEventNotifierCfg->bStaStateRxDetect && (pEventNotifierCfg->StaRxPktDetectPeriod > 0)) {
				pEventNotifierCfg->StaRxPktDetectRound++;

				if (((pEventNotifierCfg->StaRxPktDetectRound % pEventNotifierCfg->StaRxPktDetectPeriod) == 0)) {
					if ((pEntry->rx_state.CurrentState == WHC_STA_STATE_ACTIVE) &&
						(pEntry->rx_state.PacketCount < pEventNotifierCfg->StaStateRxThreshold)) {
						pEntry->rx_state.CurrentState = WHC_STA_STATE_IDLE;
						pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);

						if (pEventHdlrHook && pEntry->wdev)
							pEventHdlrHook(pAd, pEntry, FALSE);
					} else if ((pEntry->rx_state.CurrentState == WHC_STA_STATE_IDLE) &&
							   (pEntry->rx_state.PacketCount >= pEventNotifierCfg->StaStateRxThreshold)) {
						pEntry->rx_state.CurrentState = WHC_STA_STATE_ACTIVE;
						pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);

						if (pEventHdlrHook && pEntry->wdev)
							pEventHdlrHook(pAd, pEntry, FALSE);
					}

					pEventNotifierCfg->StaRxPktDetectRound = 0;
					pEntry->rx_state.PacketCount = 0;
				}
			}
#endif /* WH_EVENT_NOTIFIER */
		}

#if defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)
		if ((pMbss != NULL) && pMbss->a4_init && pMbss->wdev.IgmpSnoopEnable) {
									/* If Snooping & MWDS enabled*/
			if ((pAd->Mlme.OneSecPeriodicRound % 10) == 0)	{
										/* Per 10 sec check */
				/* Logic implemented to do Periodic Multicast membership queries to NON-MWDS STAs
				 for update of entries in IGMP table, to avoid aging of interested members.
				 Also, in case another Membership querier detected in network, logic implemented to
				 hold internal query transmission to avoid flooding on network.
				 Decrement time tick counters for Query Sent & Query hold as applicable
				 Send the query once on each MBSS, for which both these counters are 0.
				 (Ensured that even with multiple STA on a MBSS, only one multicast query transmitted)*/

				if (pAd->bIGMPperiodicQuery == TRUE) {
					/* If Periodic IGMP query feature enabled */

					if ((pMbss->IgmpQueryHoldTick > 0) && (pMbss->IgmpQueryHoldTickChanged == FALSE)) {
						/*Decrement IgmpQueryHoldTick, only once for each MBSS*/
						pMbss->IgmpQueryHoldTick--;
						pMbss->IgmpQueryHoldTickChanged = TRUE;
					}

					if ((pAd->IgmpQuerySendTick > 0) && (IgmpQuerySendTickChanged == FALSE)) {
						/*Decrement IgmpQuerySendTick, only once for each MBSS*/
						pAd->IgmpQuerySendTick--;
						IgmpQuerySendTickChanged = TRUE;
					}

					if ((pMbss->IGMPPeriodicQuerySent == FALSE)
					   && ((pMbss->IgmpQueryHoldTick == 0) && (pAd->IgmpQuerySendTick == 0))) {
						/*transmit IGMP query on this MBSS, only once*/
						send_igmpv3_gen_query_pkt(pAd, pEntry);

						pMbss->IGMPPeriodicQuerySent = TRUE;
					}
				}

				if (pAd->bMLDperiodicQuery == TRUE) { /*If Periodic MLD query feature enabled*/

					if ((pMbss->MldQueryHoldTick > 0) && (pMbss->MldQueryHoldTickChanged == FALSE)) {
						/*Decrement MldQueryHoldTick, only once for each MBSS*/
						pMbss->MldQueryHoldTick--;
						pMbss->MldQueryHoldTickChanged = TRUE;
					}

					if ((pAd->MldQuerySendTick > 0) && (MldQuerySendTickChanged == FALSE)) {
						/*Decrement MldQuerySendTick, only once for each MBSS*/
						pAd->MldQuerySendTick--;
						MldQuerySendTickChanged = TRUE;
					}
					if ((pMbss->MLDPeriodicQuerySent == FALSE)
					   && ((pMbss->MldQueryHoldTick == 0) && (pAd->MldQuerySendTick == 0))) {
						/*transmit MLD query on this MBSS, only once*/
						send_mldv2_gen_query_pkt(pAd, pEntry);

						pMbss->MLDPeriodicQuerySent = TRUE;
					}

				}

			} else if ((pAd->Mlme.OneSecPeriodicRound % 10) == 1) { /* Per 11 sec check */

				if (pAd->IgmpQuerySendTick == 0) /* Set the period for IGMP query again */
					pAd->IgmpQuerySendTick = QUERY_SEND_PERIOD;

				if (pAd->MldQuerySendTick == 0) /* Set the period for MLD query again */
					pAd->MldQuerySendTick = QUERY_SEND_PERIOD;

				if (pMbss->IGMPPeriodicQuerySent == TRUE)
					pMbss->IGMPPeriodicQuerySent = FALSE; /* Reset flag for next period query */

				if (pMbss->MLDPeriodicQuerySent == TRUE)
					pMbss->MLDPeriodicQuerySent = FALSE; /* Reset flag for next period query */

				pMbss->IgmpQueryHoldTickChanged = FALSE; /* Reset flag for next 10th second counter edit */
				pMbss->MldQueryHoldTickChanged = FALSE; /* Reset flag for next 10th second counter edit */

			}
		}
#endif
#endif
	}

	/*if the number of station is less than MAX_FW_AGG_MSG*/
	if (!IS_MT7990(pAd) && !IS_MT7992(pAd) && !IS_MT7993(pAd) && tx_statics_qcnt && p_tx_statics_queue) {
		HW_GET_TX_STATISTIC(pAd, p_tx_statics_queue, tx_statics_qcnt);
		tx_statics_qcnt = 0;
		os_zero_mem(p_tx_statics_queue, sizeof(TX_STAT_STRUC) * MAX_FW_AGG_MSG);
	}

	if (p_tx_statics_queue)
		os_free_mem(p_tx_statics_queue);

#if (defined(IGMP_SNOOP_SUPPORT) && defined(IGMP_SNOOPING_OFFLOAD))
	if (cr4_m2u_query_list) {
		if (IS_MT7992(pAd) && (cr4_m2u_query_list->number))
			HW_WA_MULTIQUERY(pAd, CR4_QUERY_OPTION_GET_TX_STATISTICS, cr4_m2u_query_list->number, 0, cr4_m2u_query_list);
		os_free_mem(cr4_m2u_query_list);
	}
#endif

#ifdef WHNAT_SUPPORT
	/*if the number of station is less than MAX_CR4_QUERY_NUM*/
	if (cr4_query_list) {
		if (cr4_query_list->number)
			HW_WA_MULTIQUERY(pAd, CR4_QUERY_OPTION_GET_TX_STATISTICS, cr4_query_list->number, 0, cr4_query_list);
		os_free_mem(cr4_query_list);
	}
#endif

#ifdef CONFIG_TX_DELAY
	if (IS_TX_DELAY_HW_MODE(cap)) {
		struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
		UINT8 band_idx = HcGetBandByWdev(peak_tp_ctl->main_wdev);
		struct tx_delay_control *tx_delay_ctl = qm_ops->get_qm_delay_ctl(pAd, band_idx);
		UINT32 reg_val = 0;
		UINT32 ap_rx_peak_th = 0;
		/* No take DBDC case into consideration because only one PLE Delay CR */
		if (!pAd->CommonCfg.dbdc_mode) {
			peak_tp_ctl = &pAd->peak_tp_ctl;
			if (peak_tp_ctl->main_wdev == NULL)
				ap_rx_peak_th = 0;
			else if (wlan_config_get_ch_band(peak_tp_ctl->main_wdev) == CMD_CH_BAND_5G)
				ap_rx_peak_th = cap->Ap5GPeakTpTH;
			else if (wlan_config_get_ch_band(peak_tp_ctl->main_wdev) == CMD_CH_BAND_24G)
				ap_rx_peak_th = cap->Ap2GPeakTpTH;

			/* only apply on non-wmm case */
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE) &&
				ap_rx_peak_th != 0 &&
				peak_tp_ctl->main_rx_tp > ap_rx_peak_th &&
				peak_tp_ctl->main_traffc_mode == TRAFFIC_UL_MODE) {
				if (!tx_delay_ctl->hw_enabled) {
					tx_delay_ctl->hw_enabled = TRUE;
					reg_val |= SET_PLE_TX_DELAY_PAGE_THRES(tx_delay_ctl->tx_process_batch_cnt);
					reg_val |= SET_PLE_TX_DELAY_TIME_THRES(tx_delay_ctl->que_agg_timeout_value);
					HW_IO_WRITE32(pAd->hdev_ctrl, PLE_TX_DELAY_CTRL, reg_val);
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
						"Enable PLE Delay with %x\n", reg_val);

				}
			} else {
				if (tx_delay_ctl->hw_enabled) {
					tx_delay_ctl->hw_enabled = FALSE;
					HW_IO_WRITE32(pAd->hdev_ctrl, PLE_TX_DELAY_CTRL, 0);
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
						"Disabled PLE Delay\n");
				}
			}
		}
	}
#endif /* CONFIG_TX_DELAY */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
		pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_INFO_NOTIFY);

#endif /* DOT11N_DRAFT3 */

#ifdef APCLI_SUPPORT


#endif /* APCLI_SUPPORT */
	if (sta_hit_2g_infra_case_number == STA_NUMBER_FOR_TRIGGER) {
		if (pAd->G_MODE_INFRA_TXOP_RUNNING == FALSE) {
			pAd->g_mode_txop_wdev = txop_wdev;
			pAd->G_MODE_INFRA_TXOP_RUNNING = TRUE;
			enable_tx_burst(pAd, txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_FE);
		} else if (pAd->g_mode_txop_wdev != txop_wdev) {
			disable_tx_burst(pAd, pAd->g_mode_txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_0);
			enable_tx_burst(pAd, txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_FE);
			pAd->g_mode_txop_wdev = txop_wdev;
		}
	} else {
		if (pAd->G_MODE_INFRA_TXOP_RUNNING == TRUE) {
			disable_tx_burst(pAd, pAd->g_mode_txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_0);
			pAd->G_MODE_INFRA_TXOP_RUNNING = FALSE;
			pAd->g_mode_txop_wdev = NULL;
		}
	}
	dynamic_txop_adjust(pAd);
#ifdef PEAK_ENHANCE
	peak_adjust_txop(pAd);
#endif /* PEAK_ENHANCE */
#if defined(VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	dynamic_near_far_adjust(pAd);
	dynamic_airtime_fairness_adjust(pAd);
	if (pAd->vow_cfg.mcli_schedule_en
		&& pAd->Mlme.OneSecPeriodicRound % 3 == 0)
		dynamic_mcli_param_adjust(pAd);
#endif

#ifdef DYNAMIC_ADJUST_BAWINSZ
	dynamic_agg_per_sta_adjust(pAd);
#endif

	dynamic_ampdu_efficiency_adjust_all(pAd);

#endif /* DOT11_N_SUPPORT */
#ifdef RTMP_MAC_PCI
	RTMP_IRQ_LOCK(&PD_GET_DEVICE_IRQ(pAd->physical_dev), IrqFlags);
#endif /* RTMP_MAC_PCI */

	/*
	   4.
	   garbage collect pAd->MacTab.McastPsQueue if backlogged MCAST/BCAST frames
	   stale in queue. Since MCAST/BCAST frames always been sent out whenever
		DtimCount==0, the only case to let them stale is surprise removal of the NIC,
		so that ASIC-based Tbcn interrupt stops and DtimCount dead.
	*/
	/* TODO: shiang-usw. revise this becasue now we have per-BSS McastPsQueue! */
#ifdef RT_CFG80211_SUPPORT
#else
	if (pMacTable->McastPsQueue.Head) {
		pMacTable->PsQIdleCount++;

		if (pMacTable->PsQIdleCount > 1) {
			APCleanupPsQueue(pAd, &pMacTable->McastPsQueue);
			pMacTable->PsQIdleCount = 0;

			if (pAd->ApCfg.BssidNum > MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);
		}
	} else
		pMacTable->PsQIdleCount = 0;
#endif

#ifdef RTMP_MAC_PCI
	RTMP_IRQ_UNLOCK(&PD_GET_DEVICE_IRQ(pAd->physical_dev), IrqFlags);
#endif /* RTMP_MAC_PCI */
	pAd->txop_ctl.last_client_num = pAd->txop_ctl.multi_client_nums;
	pAd->txop_ctl.last_rx_client_num = pAd->txop_ctl.multi_rx_client_nums;
	pAd->mcli_ctl.last_large_rssi_gap_num = pAd->mcli_ctl.large_rssi_gap_num;
	pAd->txop_ctl.last_tcp_nums =  pAd->txop_ctl.multi_tcp_nums;
	pAd->txop_ctl.multi_client_nums = 0;
	pAd->txop_ctl.multi_rx_client_nums = 0;
	pAd->txop_ctl.multi_tcp_nums = 0;
	pAd->mcli_ctl.tot_tx_cnt = 0;
	pAd->mcli_ctl.tot_tx_fail_cnt = 0;
	pAd->mcli_ctl.tot_tx_pkts = 0;
	pAd->mcli_ctl.tot_rx_pkts = 0;
	pAd->mcli_ctl.large_rssi_gap_num = 0;
	pAd->nearfar_far_client_num = 0;
}

UINT32 MacTableAssocStaNumGet(RTMP_ADAPTER *pAd)
{
	UINT32 num = 0;
	UINT32 i;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		MAC_TABLE_ENTRY *pEntry = entry_get(pAd, i);

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		if (pEntry->Sst == SST_ASSOC)
			num++;
	}

	return num;
}

/*
	==========================================================================
	Description:
		Look up a STA MAC table. Return its Sst to decide if an incoming
		frame from this STA or an outgoing frame to this STA is permitted.
	Return:
	==========================================================================
*/
MAC_TABLE_ENTRY *APSsPsInquiry(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR *pAddr,
	OUT SST * Sst,
	OUT USHORT *Aid,
	OUT UCHAR *PsMode,
	OUT UCHAR *Rate)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (MAC_ADDR_IS_GROUP(pAddr)) { /* mcast & broadcast address */
		*Sst = SST_ASSOC;
		*Aid = MCAST_WCID_TO_REMOVE;	/* Softap supports 1 BSSID and use WCID=0 as multicast Wcid index */
		*PsMode = PWR_ACTIVE;
		*Rate = pAd->CommonCfg.MlmeRate;
	} else { /* unicast address */
		pEntry = MacTableLookupForTx(pAd, pAddr, wdev);

		if (pEntry) {
			*Sst = pEntry->Sst;
			*Aid = pEntry->Aid;
			*PsMode = pEntry->PsMode;

			if (IS_AKM_WPA_CAPABILITY_Entry(pEntry)
				&& (pEntry->SecConfig.Handshake.GTKState != REKEY_ESTABLISHED))
				*Rate = pAd->CommonCfg.MlmeRate;
			else
				*Rate = pEntry->CurrTxRate;
		} else {
			*Sst = SST_NOT_AUTH;
			*Aid = MCAST_WCID_TO_REMOVE;
			*PsMode = PWR_ACTIVE;
			*Rate = pAd->CommonCfg.MlmeRate;
		}
	}

	return pEntry;
}


#ifdef SYSTEM_LOG_SUPPORT
/*
	==========================================================================
	Description:
		This routine is called to log a specific event into the event table.
		The table is a QUERY-n-CLEAR array that stop at full.
	==========================================================================
 */
VOID ApLogEvent(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT Event)
{
	if (pAd->EventTab.Num < MAX_NUM_OF_EVENT) {
		RT_802_11_EVENT_LOG *pLog = &pAd->EventTab.Log[pAd->EventTab.Num];

		RTMP_GetCurrentSystemTime(&pLog->SystemTime);
		COPY_MAC_ADDR(pLog->Addr, pAddr);
		pLog->Event = Event;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "LOG#%ld "MACSTR" %s\n",
				 pAd->EventTab.Num, MAC2STR(pAddr), pEventText[Event]);
		pAd->EventTab.Num += 1;
	}
}
#endif /* SYSTEM_LOG_SUPPORT */


#ifdef DOT11_N_SUPPORT
/*
	==========================================================================
	Description:
		Operationg mode is as defined at 802.11n for how proteciton in this BSS operates.
		Ap broadcast the operation mode at Additional HT Infroamtion Element Operating Mode fields.
		802.11n D1.0 might has bugs so this operating mode use  EWC MAC 1.24 definition first.

		Called when receiving my bssid beacon or beaconAtJoin to update protection mode.
		40MHz or 20MHz protection mode in HT 40/20 capabale BSS.
		As STA, this obeys the operation mode in ADDHT IE.
		As AP, update protection when setting ADDHT IE and after new STA joined.
	==========================================================================
*/
VOID APUpdateOperationMode(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	BOOLEAN bNonGFExist = FALSE;
	ADD_HT_INFO_IE *addht = NULL;
	UCHAR band = 0;
	UINT32 new_protection = 0;
	UCHAR op_mode = NON_PROTECT;

	if (wdev == NULL)
		return;

	if (pAd != wdev->sys_handle)
		return;

	addht = wlan_operate_get_addht(wdev);
	band = hc_get_hw_band_idx(pAd);

	if (!addht)
		return;

	/* non HT BSS exist within 5 sec */
	if ((pAd->ApCfg.LastNoneHTOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32 &&
		pAd->Mlme.Now32 != 0) {
		op_mode = NONMEMBER_PROTECT;
		bNonGFExist = TRUE; /* non-HT means nonGF support */
		new_protection = SET_PROTECT(NON_MEMBER_PROTECT);
	}

	/* If I am 40MHz BSS, and there exist HT-20MHz station. */
	/* Update to 2 when it's zero.  Because OperaionMode = 1 or 3 has more protection. */
	if ((op_mode == NON_PROTECT) &&
		(pAd->MacTab->fAnyStation20Only) &&
		(wlan_config_get_ht_bw(wdev) == HT_BW_40)) {
		op_mode = BW20_PROTECT;
		new_protection = SET_PROTECT(HT20_PROTECT);
	}

	if (pAd->MacTab->fAnyStationIsLegacy || pAd->MacTab->Size >= pAd->multi_cli_nums_eap_th) {
		op_mode = NONHT_MM_PROTECT;
		new_protection = SET_PROTECT(NON_HT_MIXMODE_PROTECT);
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_DEBUG,
			 "OperationMode: %d, bNonGFExist: %d\n",
			  addht->AddHtInfo2.OperaionMode, bNonGFExist);

	if ((op_mode != addht->AddHtInfo2.OperaionMode)
			|| (pAd->MacTab->fAnyStationNonGF[band] != addht->AddHtInfo2.NonGfPresent)) {
		addht->AddHtInfo2.OperaionMode = op_mode;
		addht->AddHtInfo2.NonGfPresent = pAd->MacTab->fAnyStationNonGF[band];

		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));
	}

	if (bNonGFExist == FALSE)
		bNonGFExist = pAd->MacTab->fAnyStationNonGF[band];

	if (bNonGFExist)
		new_protection |= SET_PROTECT(GREEN_FIELD_PROTECT);

	if (VALID_MBSS(pAd, wdev->func_idx) &&
		(nonerp_protection(&pAd->ApCfg.MBSSID[wdev->func_idx])))
		new_protection |= SET_PROTECT(ERP);

#if defined(MT_MAC)
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if ((new_protection & wdev->protection) != new_protection) {
			wdev->protection = new_protection;

			HW_SET_PROTECT(pAd, wdev, PROT_PROTOCOL, 0, 0);
		}

	}
#endif
}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_EHT_BE
BOOLEAN mld_check_acl(RTMP_ADAPTER *pAd, UCHAR *mld_addr, UCHAR group_id)
{
	struct wifi_dev *pwdev;
	UCHAR link_id, i;
	PRT_802_11_ACL p_link_acl[BSS_MNGR_MAX_BAND_NUM] = {0};
	BOOLEAN result = FALSE;
	RTMP_ADAPTER *temp_pAd;
	UCHAR mld_acl_policy = 0;

	if (!group_id || !mld_addr) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"wrong group id or null mld addr, ignore mlo acl!\n");
		return TRUE;
	}

	/*step 1: get all link's acl in the same group*/
	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {

		pwdev = bss_mngr_query_group_wdev_by_link(group_id, link_id);

		if (pwdev == NULL)
			continue;

		temp_pAd = pwdev->sys_handle;

		p_link_acl[link_id] = &temp_pAd->ApCfg.MBSSID[pwdev->func_idx].AccessControlList;

		/*step 2: all link's mlo policy has sync, so use any link mlo policy*/
		if (!mld_acl_policy)
			mld_acl_policy = p_link_acl[link_id]->mlo_policy;
	}

	/*step 3: check mld acl rule
		*policy==0  -> return true, no acl operation.
		*policy==1  -> white list mode, check all link,
			if find, return true, else true false.
		*policy==2  -> black list mode, check all link,
			if find, return false, else true true.
	*/
	if (mld_acl_policy == 0)			/* ACL is disabled */
		result = TRUE;
	else if (mld_acl_policy == 1)		/* ACL is a white list */
		result = FALSE;
	else if (mld_acl_policy == 2)		/* ACL is a black list */
		result = TRUE;

	if (mld_acl_policy) {
		for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {

			if (p_link_acl[link_id] == NULL)
				continue;

			for (i = 0; i < p_link_acl[link_id]->Num; i++) {
				if (MAC_ADDR_EQUAL(mld_addr, p_link_acl[link_id]->Entry[i].Addr)) {
					result = !result;
					return result;
				}
			}
		}
	}

	return result;
}
#endif

/*
	==========================================================================
	Description:
		Check if the specified STA pass the Access Control List checking.
		If fails to pass the checking, then no authentication nor association
		is allowed
	Return:
		MLME_SUCCESS - this STA passes ACL checking

	==========================================================================
*/
BOOLEAN ApCheckAccessControlList(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR Apidx)
{
	BOOLEAN Result = TRUE;
#ifdef ACL_BLK_COUNT_SUPPORT
	ULONG idx;
#endif
	if (Apidx >= MAX_MBSSID_NUM(pAd))
		return FALSE;

	if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 0)       /* ACL is disabled */
		Result = TRUE;
	else {
		ULONG i;

		if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 1)   /* ACL is a positive list */
			Result = FALSE;
		else                                              /* ACL is a negative list */
			Result = TRUE;

		for (i = 0; i < pAd->ApCfg.MBSSID[Apidx].AccessControlList.Num; i++) {
			if (MAC_ADDR_EQUAL(pAddr, pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[i].Addr)) {
				Result = !Result;
				break;
			}
		}
	}
#ifdef ACL_BLK_COUNT_SUPPORT
		if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy != 2) {
			for (idx = 0; idx < pAd->ApCfg.MBSSID[Apidx].AccessControlList.Num; idx++)
				(pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[idx].Reject_Count) = 0;
		}
#endif

	if (Result == FALSE) {
#ifdef ACL_BLK_COUNT_SUPPORT
		if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 2) {
			for (idx = 0; idx < pAd->ApCfg.MBSSID[Apidx].AccessControlList.Num; idx++) {
				if (MAC_ADDR_EQUAL(pAddr, pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[idx].Addr)) {
						(pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[idx].Reject_Count) += 1;
					break;
				}
			}
		}
#endif/*ACL_BLK_COUNT_SUPPORT*/
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			MACSTR" failed in ACL checking\n", MAC2STR(pAddr));
	}

	return Result;
}


/*
	==========================================================================
	Description:
		This routine update the current MAC table based on the current ACL.
		If ACL change causing an associated STA become un-authorized. This STA
		will be kicked out immediately.
	==========================================================================
*/
VOID ApUpdateAccessControlList(RTMP_ADAPTER *pAd, UCHAR Apidx)
{
	USHORT   AclIdx, MacIdx;
	BOOLEAN  Matched;
	PUCHAR      pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG       FrameLen = 0;
#ifndef EM_PLUS_SUPPORT
	HEADER_802_11 DisassocHdr;
#else
	HEADER_802_11 DeAuthHdr;
#endif
	USHORT      Reason = REASON_DECLINED;
	MAC_TABLE_ENTRY *pEntry;
	BSS_STRUCT *pMbss;
	BOOLEAN drop;
#ifdef DOT11_EHT_BE
	UCHAR group_id;
	UCHAR link_id;
	struct wifi_dev *pwdev = NULL;
	RTMP_ADAPTER *temp_pAd;
	PRT_802_11_ACL p_link_acl[BSS_MNGR_MAX_BAND_NUM] = {0};
	UCHAR i;
#endif

	ASSERT(Apidx < MAX_MBSSID_NUM(pAd));

	if (Apidx >= MAX_MBSSID_NUM(pAd))
		return;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "Apidx = %d\n", Apidx);
	/* ACL is disabled. Do nothing about the MAC table. */
	pMbss = &pAd->ApCfg.MBSSID[Apidx];

#ifdef DOT11_EHT_BE
	/*if mlo is create, sync policy to all link*/
	if (pMbss->mld_grp_idx) {
		group_id = pMbss->mld_grp_idx;
		for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
			pwdev = bss_mngr_query_group_wdev_by_link(group_id, link_id);

			if (pwdev == NULL)
				continue;

			temp_pAd = pwdev->sys_handle;
			p_link_acl[link_id] = &temp_pAd->ApCfg.MBSSID[pwdev->func_idx].AccessControlList;

			p_link_acl[link_id]->mlo_policy = pMbss->AccessControlList.Policy;
		}
	}
#endif

	if (pMbss->AccessControlList.Policy == 0)
		return;

	for (MacIdx = 0; VALID_UCAST_ENTRY_WCID(pAd, MacIdx); MacIdx++) {
		pEntry = entry_get(pAd, MacIdx);

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		/* We only need to update associations related to ACL of MBSSID[Apidx]. */
		if (!ether_addr_equal(pEntry->wdev->bssid, pMbss->wdev.bssid))
			continue;

		drop = FALSE;
		Matched = FALSE;

		for (AclIdx = 0; AclIdx < pMbss->AccessControlList.Num; AclIdx++) {
			if (MAC_ADDR_EQUAL(&pEntry->Addr[0], pMbss->AccessControlList.Entry[AclIdx].Addr)) {
				Matched = TRUE;
				break;
			}
		}

#ifdef DOT11_EHT_BE
		if (pEntry->mlo.mlo_en) {
			for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {

				if (Matched == TRUE)
					break;

				if (p_link_acl[link_id] == NULL)
					continue;

				for (i = 0; i < p_link_acl[link_id]->Num; i++) {
					if (MAC_ADDR_EQUAL(pEntry->mlo.mld_addr, p_link_acl[link_id]->Entry[i].Addr)) {
						Matched = TRUE;
						break;
					}
				}
			}
		}
#endif

		if ((Matched == FALSE) && (pMbss->AccessControlList.Policy == 1)) {
			drop = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"STA not on positive ACL. remove it...\n");
		} else if ((Matched == TRUE) && (pMbss->AccessControlList.Policy == 2)) {
			drop = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"STA on negative ACL. remove it...\n");
		}

		if (drop == TRUE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "Apidx = %d\n", Apidx);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"pAd->ApCfg.MBSSID[%d].AccessControlList.Policy = %ld\n", Apidx,
				pMbss->AccessControlList.Policy);

			/* Before delete the entry from MacTable, send disassociation packet to client. */
			if (pEntry->Sst == SST_ASSOC) {
				/* send out a DISASSOC frame */
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

				if (NStatus != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
						"MlmeAllocateMemory fail  ..\n");
					return;
				}

#ifdef EM_PLUS_SUPPORT
				Reason = REASON_NO_LONGER_VALID;
#else
#ifdef MAP_R5
				if (pMbss->AccessControlList.user_reason_code == 1)
					Reason = REASON_AP_INITIATED;
				else
#endif
					Reason = REASON_DECLINED;
#endif /* EM_PLUS_SUPPORT */

#ifndef EM_PLUS_SUPPORT
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"ASSOC - Send DISASSOC  Reason = %d frame  TO %x %x %x %x %x %x\n",
						 Reason, PRINT_MAC(pEntry->Addr));
				MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0,
								 pEntry->Addr,
								 pMbss->wdev.if_addr,
								 pMbss->wdev.bssid);
				MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
				MlmeFreeMemory(pOutBuffer);
#else
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
						"Send DEAUTH - Reason = %d frame TO %x %x %x %x %x %x\n",
						Reason, PRINT_MAC(pEntry->Addr));
				MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->Addr,
						pMbss->wdev.if_addr,
						pMbss->wdev.bssid);
				MakeOutgoingFrame(pOutBuffer, &FrameLen,
						sizeof(HEADER_802_11), &DeAuthHdr,
						2, &Reason,
						END_OF_ARGS);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
				MlmeFreeMemory(pOutBuffer);
#endif
				RtmpusecDelay(5000);
			}
#ifdef WIFI_DIAG
			if (pEntry && IS_ENTRY_CLIENT(pEntry))
				diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr, DIAG_CONN_ACL_BLK, 0);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					REASON_DECLINED);
#endif
#ifdef MAP_R2
			if (IS_ENTRY_CLIENT(pEntry) && IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
				wapp_handle_sta_disassoc(pAd, pEntry->wcid, Reason);
#endif

			mac_entry_delete(pAd, pEntry, TRUE);
		}
	}
}


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
/*
	Depends on the 802.11n Draft 4.0, Before the HT AP start a BSS, it should scan some specific channels to
collect information of existing BSSs, then depens on the collected channel information, adjust the primary channel
and secondary channel setting.

	For 5GHz,
		Rule 1: If the AP chooses to start a 20/40 MHz BSS in 5GHz and that occupies the same two channels
				as any existing 20/40 MHz BSSs, then the AP shall ensure that the primary channel of the
				new BSS is identical to the primary channel of the existing 20/40 MHz BSSs and that the
				secondary channel of the new 20/40 MHz BSS is identical to the secondary channel of the
				existing 20/40 MHz BSSs, unless the AP discoverr that on those two channels are existing
				20/40 MHz BSSs with different primary and secondary channels.
		Rule 2: If the AP chooses to start a 20/40MHz BSS in 5GHz, the selected secondary channel should
				correspond to a channel on which no beacons are detected during the overlapping BSS
				scan time performed by the AP, unless there are beacons detected on both the selected
				primary and secondary channels.
		Rule 3: An HT AP should not start a 20 MHz BSS in 5GHz on a channel that is the secondary channel
				of a 20/40 MHz BSS.
	For 2.4GHz,
		Rule 1: The AP shall not start a 20/40 MHz BSS in 2.4GHz if the value of the local variable "20/40
				Operation Permitted" is FALSE.

		20/40OperationPermitted =  (P == OPi for all values of i) AND
								(P == OTi for all values of i) AND
								(S == OSi for all values if i)
		where
			P	is the operating or intended primary channel of the 20/40 MHz BSS
			S	is the operating or intended secondary channel of the 20/40 MHz BSS
			OPi  is member i of the set of channels that are members of the channel set C and that are the
				primary operating channel of at least one 20/40 MHz BSS that is detected within the AP's
				BSA during the previous X seconds
			OSi  is member i of the set of channels that are members of the channel set C and that are the
				secondary operating channel of at least one 20/40 MHz BSS that is detected within AP's
				BSA during the previous X seconds
			OTi  is member i of the set of channels that comparises all channels that are members of the
				channel set C that were listed once in the Channel List fields of 20/40 BSS Intolerant Channel
				Report elements receved during the previous X seconds and all channels that are members
				of the channel set C and that are the primary operating channel of at least one 20/40 MHz
				BSS that were detected within the AP's BSA during the previous X seconds.
			C	is the set of all channels that are allowed operating channels within the current operational
				regulatory domain and whose center frequency falls within the 40 MHz affected channel
				range given by following equation:
											 Fp + Fs                  Fp + Fs
					40MHz affected channel range = [ ------  - 25MHz,  ------- + 25MHz ]
											      2                          2
					Where
						Fp = the center frequency of channel P
						Fs = the center frequency of channel S

			"==" means that the values on either side of the "==" are to be tested for equaliy with a resulting
				 Boolean value.
				=>When the value of OPi is the empty set, then the expression (P == OPi for all values of i)
					is defined to be TRUE
				=>When the value of OTi is the empty set, then the expression (P == OTi for all values of i)
					is defined to be TRUE
				=>When the value of OSi is the empty set, then the expression (S == OSi for all values of i)
					is defined to be TRUE
*/
INT GetBssCoexEffectedChRange(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN BSS_COEX_CH_RANGE * pCoexChRange,
	IN UCHAR Channel)
{
	INT index, cntrCh = 0;
	UCHAR op_ext_cha = wlan_operate_get_ext_cha(wdev);

	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	memset(pCoexChRange, 0, sizeof(BSS_COEX_CH_RANGE));
	/* Build the effected channel list, if something wrong, return directly. */
#ifdef A_BAND_SUPPORT

	if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G) {
		UINT ChListSize = ARRAY_SIZE(pChCtrl->ChList);
		/* For 5GHz band */
		for (index = 0; index < pChCtrl->ChListNum; index++) {
			if (pChCtrl->ChList[index].Channel == Channel)
				break;
		}

		if (index < pChCtrl->ChListNum) {
			/* First get the primary channel */
			pCoexChRange->primaryCh = pChCtrl->ChList[index].Channel;

			/* Now check about the secondary and central channel */
			if (op_ext_cha == EXTCHA_ABOVE) {
				pCoexChRange->effectChStart = pCoexChRange->primaryCh;
				pCoexChRange->effectChEnd = pCoexChRange->primaryCh + 4;
				pCoexChRange->secondaryCh = pCoexChRange->effectChEnd;
			} else {
				pCoexChRange->effectChStart = pCoexChRange->primaryCh - 4;
				pCoexChRange->effectChEnd = pCoexChRange->primaryCh;
				pCoexChRange->secondaryCh = pCoexChRange->effectChStart;
			}
			if ((ChListSize > pCoexChRange->primaryCh) && (ChListSize > pCoexChRange->secondaryCh) &&
				(ChListSize > pCoexChRange->effectChStart) && (ChListSize > pCoexChRange->effectChEnd)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
						 "5.0GHz: Found CtrlCh idx(%d) from the ChList, ExtCh=%s, PriCh=[Idx:%d, CH:%d], SecCh=[Idx:%d, CH:%d], effected Ch=[CH:%d~CH:%d]!\n",
						  index,
						  ((op_ext_cha == EXTCHA_ABOVE) ? "ABOVE" : "BELOW"),
						  pCoexChRange->primaryCh, pChCtrl->ChList[pCoexChRange->primaryCh].Channel,
						  pCoexChRange->secondaryCh, pChCtrl->ChList[pCoexChRange->secondaryCh].Channel,
						  pChCtrl->ChList[pCoexChRange->effectChStart].Channel,
						  pChCtrl->ChList[pCoexChRange->effectChEnd].Channel);
			} else
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					"Invalid params: PriCh idx %d, SecCh idx %d, effected Ch idx %d~%d\n",
					pCoexChRange->primaryCh, pCoexChRange->secondaryCh, pCoexChRange->effectChStart, pCoexChRange->effectChEnd);
			return TRUE;
		}

		/* It should not happened! */
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "5GHz: Cannot found the CtrlCh(%d) in ChList, something wrong?\n", Channel);
	} else if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G)
#endif /* A_BAND_SUPPORT */
	{	/* For 2.4GHz band */
		for (index = 0; index < pChCtrl->ChListNum; index++) {
			if (pChCtrl->ChList[index].Channel == Channel)
				break;
		}

		if (index < pChCtrl->ChListNum) {
			/* First get the primary channel */
			pCoexChRange->primaryCh = index;

			/* Now check about the secondary and central channel */
			if (op_ext_cha == EXTCHA_ABOVE) {
				if ((index + 4) < pChCtrl->ChListNum) {
					cntrCh = index + 2;
					pCoexChRange->secondaryCh = index + 4;
				}
			} else {
				if (index >= 4) {
					cntrCh = index - 2;
					pCoexChRange->secondaryCh = index - 4;
				}
			}

			if (cntrCh) {
				pCoexChRange->effectChStart = (cntrCh > 5) ? (cntrCh - 5) : 0;
				pCoexChRange->effectChEnd = (cntrCh <= (255 - 5)) ? (cntrCh + 5) : 255;

				if (pCoexChRange->effectChStart < MAX_NUM_OF_CHANNELS && pCoexChRange->effectChEnd < MAX_NUM_OF_CHANNELS)
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
						 "2.4GHz: Found CtrlCh idx(%d) from the ChList, ExtCh=%s, PriCh=[Idx:%d, CH:%d], SecCh=[Idx:%d, CH:%d], effected Ch=[CH:%d~CH:%d]!\n",
						  index,
						  ((op_ext_cha == EXTCHA_ABOVE) ? "ABOVE" : "BELOW"),
						  pCoexChRange->primaryCh, pChCtrl->ChList[pCoexChRange->primaryCh].Channel,
						  pCoexChRange->secondaryCh, pChCtrl->ChList[pCoexChRange->secondaryCh].Channel,
						  pChCtrl->ChList[pCoexChRange->effectChStart].Channel,
						  pChCtrl->ChList[pCoexChRange->effectChEnd].Channel);
				else
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
						"pChCtrl->ChList index out of bound.");
			}

			return TRUE;
		}

		/* It should not happened! */
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				 "2.4GHz: Didn't found valid channel range, Ch index=%d, ChListNum=%hhu, CtrlCh=%hhu\n",
				  index, pChCtrl->ChListNum, Channel);
	}

	return FALSE;
}

#define BSS_2040_COEX_SCAN_RESULT_VALID_TIME	(60 * OS_HZ)	/* 60 seconds */

VOID APOverlappingBSSScan(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	BOOLEAN needFallBack = FALSE;
	UINT chStartIdx, chEndIdx, index, curPriChIdx, curSecChIdx;
	BSS_COEX_CH_RANGE  coexChRange;
	USHORT PhyMode = wdev->PhyMode;
	UCHAR Channel = wdev->channel;
	UCHAR ht_bw = wlan_config_get_ht_bw(wdev);
	UCHAR ext_cha = wlan_config_get_ext_cha(wdev);
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	P_BSS_COEX_SCAN_LAST_RESULT pScanResult = &pAd->CommonCfg.BssCoexScanLastResult;
	ULONG Now32;

	if (wlan_config_get_ch_band(wdev) != CMD_CH_BAND_24G) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
			"not support 2G PhyMode, return!\n");
		return;
	}

	/* We just care BSS who operating in 40MHz N Mode. */
	if ((!WMODE_CAP_N(PhyMode)) ||
		(ht_bw == BW_20)
	   ) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
			"The wdev->PhyMode=%d, BW=%d, didn't need channel adjustment!\n", PhyMode, ht_bw);
		return;
	}

	NdisGetSystemUpTime(&Now32);
	if (RTMP_TIME_BEFORE(Now32, pScanResult->LastScanTime + BSS_2040_COEX_SCAN_RESULT_VALID_TIME) &&
		(pScanResult->LastScanTime != 0)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
				"leverage result of wdev(%d@BN%d), FallBack=%d (remaining %ld ms)\n",
				pScanResult->WdevIdx, BandIdx, pScanResult->bNeedFallBack,
				(BSS_2040_COEX_SCAN_RESULT_VALID_TIME - (Now32 - pScanResult->LastScanTime)) * 1000 / OS_HZ);

		needFallBack = pScanResult->bNeedFallBack;
		goto coex_scan_result_apply;
	}

	/* Build the effected channel list, if something wrong, return directly. */
	/* For 2.4GHz band */
	for (index = 0; index < pChCtrl->ChListNum; index++) {
		if (pChCtrl->ChList[index].Channel == Channel)
			break;
	}

	/* Check ext_cha invalid */
	ht_ext_cha_adjust(pAd, Channel, &ht_bw, &ext_cha, wdev);

	if (index < pChCtrl->ChListNum) {
		if (ext_cha == EXTCHA_ABOVE) {
			curPriChIdx = index;
			curSecChIdx = ((index + 4) < pChCtrl->ChListNum) ? (index + 4) : (pChCtrl->ChListNum - 1);
			chStartIdx = (curPriChIdx >= 4) ? (curPriChIdx - 4) : 0;
			chEndIdx = ((curSecChIdx + 4) < pChCtrl->ChListNum) ? (curSecChIdx + 4) :
					   (pChCtrl->ChListNum - 1);
		} else {
			curPriChIdx = index;
			curSecChIdx = (index >= 4) ? (index - 4) : 0;
			chStartIdx = (curSecChIdx >= 4) ? (curSecChIdx - 4) : 0;
			chEndIdx =  ((curPriChIdx + 4) < pChCtrl->ChListNum) ? (curPriChIdx + 4) :
						(pChCtrl->ChListNum - 1);
		}
	} else {
		/* It should not happened! */
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				 "2.4GHz: Cannot found the Control Channel(%d) in ChannelList, something wrong?\n", Channel);
		return;
	}

#ifdef GREENAP_SUPPORT
	greenap_suspend(pAd, GREENAP_REASON_AP_OVERLAPPING_SCAN);
#endif /* GREENAP_SUPPORT */

	GetBssCoexEffectedChRange(pAd, wdev, &coexChRange, Channel);

	/* Before we do the scanning, clear the bEffectedChannel as zero for latter use. */
	for (index = 0; index < pChCtrl->ChListNum; index++)
		pChCtrl->ChList[index].bEffectedChannel = 0;

	pAd->CommonCfg.BssCoexApCnt = 0;
	memset(pAd->CommonCfg.BssCoexApMac, 0, sizeof(pAd->CommonCfg.BssCoexApMac));

	/* If we are not ready for Tx/Rx Pakcet, enable it now for receiving Beacons. */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP) == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"Card still not enable Tx/Rx, enable it now!\n");
		/* rtmp_rx_done_handle() API will check this flag to decide accept incoming packet or not. */
		/* Set the flag be ready to receive Beacon frame for autochannel select. */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	}

	ap_enable_rxtx(pAd);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"Ready to do passive scanning for Channel[%d] to Channel[%d]!\n",
		pChCtrl->ChList[chStartIdx].Channel, pChCtrl->ChList[chEndIdx].Channel);
	/* Now start to do the passive scanning. */
	pAd->CommonCfg.bOverlapScanning = TRUE;

	for (index = chStartIdx; index <= chEndIdx; index++) {
		AsicSetMacTxRx(pAd, ASIC_MAC_TXRX, FALSE);
		Channel = pChCtrl->ChList[index].Channel;
		wlan_operate_scan(wdev, Channel);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"AP OBSS SYNC - BBP R4 to 20MHz.l\n");
		AsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);
		OS_WAIT(120); /* wait for 120 ms at each channel. */
	}

	pAd->CommonCfg.bOverlapScanning = FALSE;

	/* After scan all relate channels, now check the scan result to find out if we need fallback to 20MHz. */
	for (index = chStartIdx; index <= chEndIdx; index++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"Channel[Idx=%d, Ch=%d].bEffectedChannel=0x%x!\n",
			index, pChCtrl->ChList[index].Channel, pChCtrl->ChList[index].bEffectedChannel);

		if ((pChCtrl->ChList[index].bEffectedChannel & (EFFECTED_CH_PRIMARY | EFFECTED_CH_LEGACY))  &&
			(index != curPriChIdx)) {
			needFallBack = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"needFallBack=TRUE due to OP/OT!\n");
		}

		if ((pChCtrl->ChList[index].bEffectedChannel & EFFECTED_CH_SECONDARY)  && (index != curSecChIdx)) {
			needFallBack = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"needFallBack=TRUE due to OS!\n");
		}
	}

	/* check threshold of coex AP counts */
	needFallBack = (pAd->CommonCfg.BssCoexApCnt > pAd->CommonCfg.BssCoexApCntThr) ? needFallBack : FALSE;

	/* update last scan result */
	pScanResult->WdevIdx = wdev->wdev_idx;
	pScanResult->LastScanTime = Now32;
	pScanResult->bNeedFallBack = needFallBack;

coex_scan_result_apply:
	/* If need fallback, now do it. */
	if (needFallBack == TRUE) {
		wlan_operate_set_prim_ch(wdev, wdev->channel);
		wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
		pAd->CommonCfg.need_fallback = 1;
		pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 1;
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
	} else {
		/*restore to original channel*/
		pAd->CommonCfg.need_fallback = 0;
		wlan_operate_set_prim_ch(wdev, wdev->channel);
		wlan_operate_set_ht_bw(wdev, ht_bw, ext_cha);
	}
#ifdef GREENAP_SUPPORT
	greenap_resume(pAd, GREENAP_REASON_AP_OVERLAPPING_SCAN);
#endif /* GREENAP_SUPPORT */
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

#define TP_100M_LOWER_BOUND	60
#define TP_100M_UPPER_BOUND	120
#define DBDC_PEAK_TP_PER_THRESHOLD 5

static VOID dynamic_ampdu_efficiency_adjust_all(struct _RTMP_ADAPTER *ad)
{
	EDCA_PARM *edcaparam;
	ULONG per = 0;
	ULONG tx_cnt;
	COUNTER_802_11 *wlan_ct = NULL;
	struct wifi_dev *wdev = NULL;
	UINT16 multi_client_num_th = 0, ap_peak_tp_th = 0;
	UCHAR band_idx = hc_get_hw_band_idx(ad);
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(ad->physical_dev);
	struct peak_tp_ctl *peak_tp_ctl = NULL;
	UINT16 level = cap->peak_txop;
#ifdef PEAK_ENHANCE
	struct peak_enhance_ctrl *ctrl = &ad->CommonCfg.peek_enhance;
#endif
#ifdef RX_COUNT_DETECT
	BOOLEAN limit_ampdu_flag = FALSE, fband_2g = FALSE;
#endif /* RX_COUNT_DETECT */

	peak_tp_ctl = &ad->peak_tp_ctl;

#ifdef PEAK_ENHANCE
	if (ctrl->enable_adjust & ENABLE_MLO_ENTRY_ADJUST_ONLY)
		goto Reset;
#endif

	MTWF_DBG(ad, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_DEBUG, "BandIdx:%d\n", band_idx);
	if (peak_tp_ctl->client_nums == 0)
		goto ignore_ampdu_efficiency_check;

	wlan_ct = &ad->WlanCounters;
	wdev = peak_tp_ctl->main_wdev;
	tx_cnt = wlan_ct->AmpduSuccessCount.u.LowPart;

	if (wdev && WMODE_CAP_2G(wdev->PhyMode)) {
#ifdef RX_COUNT_DETECT
		fband_2g = TRUE;
#endif /* RX_COUNT_DETECT */
		multi_client_num_th = MULTI_CLIENT_2G_NUMS_TH;
		if (ad->CommonCfg.dbdc_mode)
			ap_peak_tp_th = cap->ApDBDC2GPeakTpTH;
		else
			ap_peak_tp_th = cap->Ap2GPeakTpTH;
	} else if (wdev && (WMODE_CAP_5G(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode))) {
		multi_client_num_th = MULTI_CLIENT_NUMS_TH;
		if (ad->CommonCfg.dbdc_mode)
			ap_peak_tp_th = cap->ApDBDC5GPeakTpTH;
		else
			ap_peak_tp_th = cap->Ap5GPeakTpTH;
	} else {
		MTWF_DBG(ad, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_ERROR, "[%d]\n", __LINE__);
		goto Reset;
	}

#ifdef RX_COUNT_DETECT
	MTWF_DBG(ad, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
		"[%d] multi_client_nums = %d\n",
		__LINE__, ad->txop_ctl.multi_client_nums);
	if ((ad->MacTab->Size >= ad->multi_cli_nums_eap_th) &&
		(ad->mcli_ctl.large_rssi_gap_num > 0)) {
		struct peak_tp_ctl *peak_tp_ctl = NULL;

		peak_tp_ctl = &ad->peak_tp_ctl;
		if (peak_tp_ctl->main_traffc_mode == TRAFFIC_UL_MODE)
			limit_ampdu_flag = TRUE;
	}
	/* limit ampdu only for old WRR scheduler */
	if (ad->vow_gen.VOW_GEN >= VOW_GEN_FALCON)
		limit_ampdu_flag = FALSE;
#endif /* RX_COUNT_DETECT*/

	if (ad->txop_ctl.multi_client_nums >= multi_client_num_th) {
		/* do no apply patch, it is in veriwave multi-client case */
		goto ignore_ampdu_efficiency_check;
	}

	if (tx_cnt > 0)
		per = 100 * (wlan_ct->AmpduFailCount.u.LowPart) / (wlan_ct->AmpduFailCount.u.LowPart + tx_cnt);

	if (per >= DBDC_PEAK_TP_PER_THRESHOLD) {
		/* do no apply patch, it is in noise environment */
		MTWF_DBG(ad, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO, "[%d]per=%lu\n", __LINE__, per);
		goto ignore_ampdu_efficiency_check;
	}
	/* scenario detection, peak TP TH is pre setting on each CHIP's cap */
	if (ap_peak_tp_th != 0 &&
		(peak_tp_ctl->main_tx_tp > ap_peak_tp_th) &&
		peak_tp_ctl->main_traffc_mode == TRAFFIC_DL_MODE) {
		peak_tp_ctl->cli_peak_tp_running = 1;
		level = TXOP_FE;

		if (level != peak_tp_ctl->cli_peak_tp_txop_level)
			peak_tp_ctl->cli_peak_tp_txop_enable = FALSE;
	} else if (peak_tp_ctl->main_traffc_mode == TRAFFIC_DL_MODE &&
		(peak_tp_ctl->main_tx_tp > TP_100M_LOWER_BOUND) &&
		(peak_tp_ctl->main_tx_tp < TP_100M_UPPER_BOUND)) {
		peak_tp_ctl->cli_peak_tp_running = 1;
		level = TXOP_60;

		if (level != peak_tp_ctl->cli_peak_tp_txop_level)
			peak_tp_ctl->cli_peak_tp_txop_enable = FALSE;
	} else
		peak_tp_ctl->cli_peak_tp_running = 0;

	MTWF_DBG(ad, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
		"[%d]per=%lu, tx=%d M, (%d, %d, %d, %d)\n\r",
		__LINE__, per, ad->peak_tp_ctl.main_tx_tp,
		peak_tp_ctl->cli_peak_tp_running,
		peak_tp_ctl->cli_ampdu_efficiency_running,
		peak_tp_ctl->cli_peak_tp_txop_enable,
		peak_tp_ctl->main_traffc_mode);
	/* increase ampdu efficiency if running peak T.P */
	if (peak_tp_ctl->cli_peak_tp_running) {
		if (!peak_tp_ctl->cli_ampdu_efficiency_running) {
			if (peak_tp_ctl->main_entry->vendor_ie.is_mtk == FALSE && query_tx_burst_prio(ad, wdev) <= PRIO_PEAK_TP) {
				AsicAmpduEfficiencyAdjust(wdev, 0xf);
				peak_tp_ctl->cli_ampdu_efficiency_running = TRUE;
			}
		}

		if (!peak_tp_ctl->cli_peak_tp_txop_enable) {
			enable_tx_burst(ad, wdev, AC_BE, PRIO_PEAK_TP, level);
			peak_tp_ctl->cli_peak_tp_txop_level = level;
			peak_tp_ctl->cli_peak_tp_txop_enable = TRUE;
		}
	} else {
		/* restore to original */
		if (peak_tp_ctl->cli_ampdu_efficiency_running) {
			edcaparam = hwifi_get_edca(ad, wdev);

			if (edcaparam)
				AsicAmpduEfficiencyAdjust(wdev, edcaparam->Aifsn[0]);

			peak_tp_ctl->cli_ampdu_efficiency_running = FALSE;
		}

		if (peak_tp_ctl->cli_peak_tp_txop_enable) {
			disable_tx_burst(ad, wdev, AC_BE, PRIO_PEAK_TP, level);
			peak_tp_ctl->cli_peak_tp_txop_enable = FALSE;
		}
	}

ignore_ampdu_efficiency_check:

	/* restore aifs adjust since dynamic txop owner is not peak throughput */
	if (peak_tp_ctl->cli_ampdu_efficiency_running) {
		if (query_tx_burst_prio(ad, wdev) > PRIO_PEAK_TP) {
			edcaparam = hwifi_get_edca(ad, wdev);

			if (edcaparam)
				AsicAmpduEfficiencyAdjust(wdev, edcaparam->Aifsn[0]);

			peak_tp_ctl->cli_ampdu_efficiency_running = FALSE;
		}
	}

#ifdef RX_COUNT_DETECT
	if (ad->txop_ctl.limit_ampdu_size_running != limit_ampdu_flag) {
		/* TODO : use in-band cmd for per wtbl or per AC */
		if (ad->txop_ctl.limit_ampdu_size_running == FALSE) {
			/* limit ampdu count to 0xa */
			ad->txop_ctl.limit_ampdu_size_running = TRUE;
			if (fband_2g)
				MAC_IO_WRITE32(ad->hdev_ctrl, AGG_AALCR0, 0x0a00);
			else
				MAC_IO_WRITE32(ad->hdev_ctrl, AGG_AALCR1, 0x0a00);
		} else {
			/* restore to unlimit*/
			ad->txop_ctl.limit_ampdu_size_running = FALSE;

			if (fband_2g)
				MAC_IO_WRITE32(ad->hdev_ctrl, AGG_AALCR0, 0x0);
			else
				MAC_IO_WRITE32(ad->hdev_ctrl, AGG_AALCR1, 0x0);
		}
	}
#endif /* RX_COUNT_DETECT */

Reset:
	/* clear some record */
	peak_tp_ctl->client_nums = 0;
	peak_tp_ctl->main_tx_tp = 0;
	peak_tp_ctl->main_rx_tp = 0;
	peak_tp_ctl->main_txrx_bytes = 0;
}

#ifdef ANTENNA_DIVERSITY_SUPPORT
VOID ant_diversity_periodic_exec(RTMP_ADAPTER *pAd)
{
	ant_diversity_update_indicator(pAd);
	if (ant_diversity_allowed(pAd, hc_get_hw_band_idx(pAd)) == TRUE)
		ant_diversity_process(pAd, hc_get_hw_band_idx(pAd));
}

VOID ant_diversity_gpio_control(struct _RTMP_ADAPTER *ad, UINT8 band_idx, UINT8 ant)
{
/*
			|		Band0			|		band1
			|main ant	|aux ant	|main ant	|aux ant
	GPIO36	|x			|x			|0			|1
	GPIO37	|x			|x			|1			|0
	GPIO38	|0			|1			|x			|x
	GPIO39	|1			|0			|x			|x
*/
	UINT32 Value = 0;

	if (band_idx == DBDC_BAND0) {
		/*GPIO38, set pinmux to GPIO*/
		RTMP_IO_READ32(ad->hdev_ctrl, 0x70005068, &Value);
		Value &= ~(BIT31 | BIT30 | BIT29 | BIT28);
		Value |= 0x9 << 28;
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x70005068, Value);

		RTMP_IO_READ32(ad->hdev_ctrl, 0x700040C0, &Value);
		Value |= (BIT6);
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040C0, Value);

		/*GPIO39, set pinmux to GPIO*/
		RTMP_IO_READ32(ad->hdev_ctrl, 0x7000506C, &Value);
		Value &= ~(BIT3 | BIT2 | BIT1 | BIT0);
		Value |= 0x9;
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x7000506C, Value);

		RTMP_IO_READ32(ad->hdev_ctrl, 0x700040C0, &Value);
		Value |= (BIT7);
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040C0, Value);

		if (ant == ANT_AUX) {
			/*GPIO38 set to 0, GPIO39 set to 1*/
			RTMP_IO_READ32(ad->hdev_ctrl, 0x700040B0, &Value);
			Value &= ~(BIT6 | BIT7);
			Value |= 0x2 << 6;
			RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040B0, Value);
		} else {
			/*GPIO39 set to 0, GPIO38 set to 1*/
			RTMP_IO_READ32(ad->hdev_ctrl, 0x700040B0, &Value);
			Value &= ~(BIT6 | BIT7);
			Value |= 0x1 << 6;
			RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040B0, Value);
		}
	}

	if (band_idx == DBDC_BAND1) {
		/*GPIO36, set pinmux to GPIO*/
		RTMP_IO_READ32(ad->hdev_ctrl, 0x70005068, &Value);
		Value &= ~(BIT23 | BIT22 | BIT21 | BIT20);
		Value |= 0x9 << 20;
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x70005068, Value);

		RTMP_IO_READ32(ad->hdev_ctrl, 0x700040C0, &Value);
		Value |= (BIT4);
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040C0, Value);

		/*GPIO37, set pinmux to GPIO*/
		RTMP_IO_READ32(ad->hdev_ctrl, 0x70005068, &Value);
		Value &= ~(BIT27 | BIT26 | BIT25 | BIT24);
		Value |= 0x9 << 24;
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x70005068, Value);

		RTMP_IO_READ32(ad->hdev_ctrl, 0x700040C0, &Value);
		Value |= (BIT5);
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040C0, Value);

		if (ant == ANT_AUX) {
			/*GPIO36 set to 0, GPIO37 set to 1*/
			RTMP_IO_READ32(ad->hdev_ctrl, 0x700040B0, &Value);
			Value &= ~(BIT4 | BIT5);
			Value |= 0x2 << 4;
			RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040B0, Value);
		} else {
			/*GPIO37 set to 0, GPIO36 set to 1*/
			RTMP_IO_READ32(ad->hdev_ctrl, 0x700040B0, &Value);
			Value &= ~(BIT4 | BIT5);
			Value |= 0x1 << 4;
			RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040B0, Value);
		}
	}
}

VOID ant_diversity_restore_rxv_cr(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT32 value = 0;

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO, "---Enter---\n");

	if (band_idx == DBDC_BAND0) {
		value = pAd->diversity_ctl.rxv_cr_value;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820e3070, value);
	}
	else {
		value = pAd->diversity_ctl.rxv_cr_value;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820f3070, value);
	}
}

VOID ant_diversity_backup_and_enable_rxv_cr(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT32 value = 0;

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_RXPHY, DBG_LVL_INFO, "---Enter---\n");

	if (band_idx == DBDC_BAND0) {
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x820e3070, &value);
		pAd->diversity_ctl.rxv_cr_value = value;/* backup CR*/
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820e3070, 0x91);/* enable CR*/
	}
	else {
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x820f3070, &value);
		pAd->diversity_ctl.rxv_cr_value = value;/* backup CR*/
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820f3070, 0x91);/* enable CR*/
	}
}

VOID diversity_switch_antenna(RTMP_ADAPTER *pAd, UINT8 band_idx, UINT8 ant)
{
	UINT8 target_ant = ant;

	if (target_ant == ANT_INIT)
		target_ant = ANT_MAIN;

	if (target_ant > ANT_AUX) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "invalid ant number(%d)\n", target_ant);
		return;
	}

	if (pAd->diversity_ctl.cur_ant != target_ant) {
		pAd->diversity_ctl.cur_ant = target_ant;
		ant_diversity_gpio_control(pAd, band_idx, target_ant);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "switch to ant(%d), band_idx(%d)\n", target_ant, band_idx);
	}
}

VOID ant_diversity_ctrl_reset(RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "---Enter---\n");
	pAd->diversity_ctl.diversity_force_disable = 0;
	pAd->diversity_ctl.dbg_cn_deg_cnt_th = 0;
	pAd->diversity_ctl.dbg_rx_rate_deg_cnt_th = 0;
	pAd->diversity_ctl.dbg_tp_deg_th = 0;
	pAd->diversity_ctl.dbg_cn_deg_th_max = 0;
	pAd->diversity_ctl.dbg_cn_deg_th_min = 0;
	pAd->diversity_ctl.dbg_rx_rate_deg_th = 0;
	pAd->diversity_ctl.dbg_countdown = 0;
	pAd->diversity_ctl.dbg_ul_tp_th = 0;
	pAd->diversity_ctl.dbg_ul_rate_delta_th = 0;
	pAd->diversity_ctl.dbg_flag_lvl1 = 0;
	pAd->diversity_ctl.dbg_flag_lvl2 = 0;
	pAd->diversity_ctl.rxv_cr_value = 0x1;
	pAd->cn_rate_read_interval = 100;
}

VOID ant_diversity_ctrl_init(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT32 ul_mode_th = 0;
	UINT8 countdown = 0;

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "---Enter---\n");

	if (band_idx == DBDC_BAND0) {
		ul_mode_th = UL_MODE_TH_2G;
		countdown = ANT_COUNTDOWN_2G;
	} else {
		ul_mode_th = UL_MODE_TH_5G;
		countdown = ANT_COUNTDOWN_5G;
	}

	if (pAd->diversity_ctl.dbg_countdown != 0)
		countdown = pAd->diversity_ctl.dbg_countdown;
	if (pAd->diversity_ctl.dbg_ul_tp_th != 0)
		ul_mode_th = pAd->diversity_ctl.dbg_ul_tp_th;

	pAd->diversity_ctl.ul_mode_th = ul_mode_th;
	pAd->diversity_ctl.cur_ant = ANT_INIT;
	pAd->diversity_ctl.cur_traffic_mode = TRAFFIC_0;
	pAd->diversity_ctl.last_traffic_mode = TRAFFIC_0;
	pAd->diversity_ctl.cur_rx_tput[ANT_MAIN] = 0;
	pAd->diversity_ctl.cur_rx_tput[ANT_AUX] = 0;
	pAd->diversity_ctl.cur_rx_cn[ANT_MAIN] = 0;
	pAd->diversity_ctl.cur_rx_cn[ANT_AUX] = 0;
	pAd->diversity_ctl.cur_tx_tput[ANT_MAIN] = 0;
	pAd->diversity_ctl.cur_tx_tput[ANT_AUX] = 0;
	pAd->diversity_ctl.last_rx_tput[ANT_MAIN] = 0;
	pAd->diversity_ctl.last_rx_tput[ANT_AUX] = 0;
	pAd->diversity_ctl.last_tx_tput[ANT_MAIN] = 0;
	pAd->diversity_ctl.last_tx_tput[ANT_AUX] = 0;
	pAd->diversity_ctl.last_rx_cn[ANT_MAIN] = 0;
	pAd->diversity_ctl.last_rx_cn[ANT_AUX] = 0;
	pAd->diversity_ctl.last_rx_rate[ANT_MAIN] = 0;
	pAd->diversity_ctl.last_rx_rate[ANT_AUX] = 0;
	pAd->diversity_ctl.cur_rx_rate[ANT_MAIN] = 0;
	pAd->diversity_ctl.cur_rx_rate[ANT_AUX] = 0;
	pAd->diversity_ctl.cn_deg_cnt = 0;
	pAd->diversity_ctl.rx_delta_cn = 0;
	pAd->diversity_ctl.ap_nss = 2;
	pAd->diversity_ctl.sta_nss = 2;
	pAd->diversity_ctl.sta_rssi = -127;
	pAd->diversity_ctl.client_num = 0;
	pAd->diversity_ctl.is_he_sta = FALSE;
	pAd->diversity_ctl.ant_switch_countdown = countdown;
	pAd->diversity_ctl.diversity_status = ANT_DIVERSITY_STATUS_IDLE;
	pAd->rec_start_cn_flag = FALSE;
	pAd->rec_start_rx_rate_flag = FALSE;
	pAd->cn_average = ANT_DIVERSITY_CN_INIT_VAL;
	pAd->phy_rate_average = ANT_DIVERSITY_RX_RATE_INIT_VAL;
	pAd->rate_rec_sum = 0;
	pAd->rate_rec_num = 0;
	pAd->cn_rec_sum = 0;
	pAd->cn_rec_num = 0;

	diversity_switch_antenna(pAd, band_idx, ANT_INIT);
}

UINT8 ant_diversity_get_sta_nss(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	UINT8 sta_nss = 1;

	if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
			(pEntry->HTCapability.MCSSet[0] != 0x00) &&
			(pEntry->HTCapability.MCSSet[1] != 0x00))
			sta_nss = 2;

	return sta_nss;
}

VOID show_ant_diversity_debug_log(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
	"---mode(%d,%d),Ant(%d),CnAvg(%d),CurRxTp(%d,%d),LastRxTp(%d,%d),CurCN(%d,%d),LastCN(%d,%d),DeltaCN(%d),CurRate(%d,%d),LastRate(%d,%d),Status(%d),(%d,%d,%d,%d,%d)---\n",
	band_idx,
	pAd->diversity_ctl.cur_traffic_mode,
	pAd->diversity_ctl.cur_ant,
	pAd->cn_average,
	pAd->diversity_ctl.cur_rx_tput[ANT_MAIN],
	pAd->diversity_ctl.cur_rx_tput[ANT_AUX],
	pAd->diversity_ctl.last_rx_tput[ANT_MAIN],
	pAd->diversity_ctl.last_rx_tput[ANT_AUX],
	pAd->diversity_ctl.cur_rx_cn[ANT_MAIN],
	pAd->diversity_ctl.cur_rx_cn[ANT_AUX],
	pAd->diversity_ctl.last_rx_cn[ANT_MAIN],
	pAd->diversity_ctl.last_rx_cn[ANT_AUX],
	pAd->diversity_ctl.rx_delta_cn,
	pAd->diversity_ctl.cur_rx_rate[ANT_MAIN],
	pAd->diversity_ctl.cur_rx_rate[ANT_AUX],
	pAd->diversity_ctl.last_rx_rate[ANT_MAIN],
	pAd->diversity_ctl.last_rx_rate[ANT_AUX],
	pAd->diversity_ctl.diversity_status,
	pAd->diversity_ctl.ap_nss,
	pAd->diversity_ctl.sta_nss,
	pAd->diversity_ctl.client_num,
	pAd->diversity_ctl.is_he_sta,
	pAd->diversity_ctl.sta_rssi);
}

UINT8 ant_diversity_update_indicator(RTMP_ADAPTER *pAd)
{
	CHAR avg_rssi = -127;
	UINT8 cur_ant = ANT_MAIN;
	UINT8 traffc_mode = 0;
	UINT16 client_num = 0;
	UINT16 wcid = 0;
	UINT32 tx_tp = 0;
	UINT32 rx_tp = 0;
	ULONG rx_bytes = 0;
	ULONG tx_bytes = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;

	for (wcid = 0; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		pEntry = entry_get(pAd, wcid);
		if ((!pEntry) || (!(IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))))
			continue;

		if (pEntry->wdev) {
			rx_bytes = pEntry->ant_div_rx_bytes;
			rx_tp = (rx_bytes >> 16);/*per 500ms */
			tx_bytes = pEntry->ant_div_tx_bytes;
			tx_tp = (tx_bytes >> 16);/*per 500ms */

			if ((tx_tp + rx_tp) == 0)
				traffc_mode = TRAFFIC_0;
			else if (((tx_tp * 100) / (tx_tp + rx_tp)) > TX_MODE_RATIO_THRESHOLD)
				traffc_mode = TRAFFIC_DL_MODE;
			else if (((rx_tp * 100) / (tx_tp + rx_tp)) > RX_MODE_RATIO_THRESHOLD)
				traffc_mode = TRAFFIC_UL_MODE;
			else
				traffc_mode = TRAFFIC_0;

			cur_ant = pAd->diversity_ctl.cur_ant;
			pAd->diversity_ctl.cur_traffic_mode = traffc_mode;
			pAd->diversity_ctl.cur_rx_tput[cur_ant] = rx_tp;
			pAd->diversity_ctl.cur_tx_tput[cur_ant] = tx_tp;
			pAd->diversity_ctl.ap_nss = wlan_config_get_tx_stream(pEntry->wdev);
			pAd->diversity_ctl.sta_nss = ant_diversity_get_sta_nss(pAd, pEntry);
			if ((pEntry->MaxHTPhyMode.field.MODE >= MODE_HE) && (pEntry->MaxHTPhyMode.field.MODE != MODE_UNKNOWN))
				pAd->diversity_ctl.is_he_sta = TRUE;
			else
				pAd->diversity_ctl.is_he_sta = FALSE;

			client_num++;

			avg_rssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);
			pAd->diversity_ctl.sta_rssi = avg_rssi;
			if (pAd->diversity_ctl.dbg_flag_lvl2 != 0) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"mode(%d), tx_tp(%d), rx_tp(%d), client(%d), avgRssi(%d), ap_nss(%d), sta_nss(%d), he_sta(%d)\n",
				traffc_mode, tx_tp, rx_tp, client_num, avg_rssi, pAd->diversity_ctl.ap_nss,
				pAd->diversity_ctl.sta_nss, pAd->diversity_ctl.is_he_sta);
			}
			pEntry->ant_div_rx_bytes = 0;
			pEntry->ant_div_tx_bytes = 0;
		}
	}
	pAd->diversity_ctl.client_num = client_num;
	return 0;
}

UINT8 ant_diversity_allowed(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	BOOLEAN allowed = FALSE;
	BOOLEAN is_cn_ready = FALSE;
	BOOLEAN is_rx_rate_ready = FALSE;
	UINT8 cur_ant = pAd->diversity_ctl.cur_ant;
	UINT8 countdown = 0;
	UINT32 tp_th_ul = 0;
	UINT32 tp_th_dl = 0;
	UINT32 cur_rx_tp = pAd->diversity_ctl.cur_rx_tput[cur_ant];
	CHAR rssi_th = -127;
	UINT8 cur_mode = pAd->diversity_ctl.cur_traffic_mode;
	UINT8 last_mode = pAd->diversity_ctl.last_traffic_mode;
	UINT32 cur_cn = pAd->cn_average;
	UINT32 cur_rx_rate = pAd->phy_rate_average;
	UINT8 cur_status = pAd->diversity_ctl.diversity_status;

	if (pAd->diversity_ctl.diversity_force_disable != 0)
		return FALSE;

	if ((cur_mode == TRAFFIC_UL_MODE) && (cur_status != ANT_DIVERSITY_STATUS_IDLE)) {
		if (cur_cn != ANT_DIVERSITY_CN_INIT_VAL)
			is_cn_ready = TRUE;
		if (cur_rx_rate != ANT_DIVERSITY_RX_RATE_INIT_VAL)
			is_rx_rate_ready = TRUE;

		if ((is_cn_ready == TRUE) && (is_rx_rate_ready == TRUE)) {
			if (pAd->diversity_ctl.dbg_flag_lvl1 != 0) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_WARN,
						"band(%d) AvgCN(%d,%d,%d), AvgRxRate(%d,%d,%d)\n", band_idx,
						cur_cn, pAd->cn_rec_sum, pAd->cn_rec_num,
						cur_rx_rate, pAd->rate_rec_sum, pAd->rate_rec_num);
			}
			pAd->diversity_ctl.cur_rx_cn[cur_ant] = cur_cn;
			pAd->cn_rec_sum = 0;
			pAd->cn_rec_num = 0;
			pAd->cn_average = ANT_DIVERSITY_CN_INIT_VAL;
			pAd->diversity_ctl.cur_rx_rate[cur_ant] = cur_rx_rate;
			pAd->rate_rec_sum = 0;
			pAd->rate_rec_num = 0;
			pAd->phy_rate_average = ANT_DIVERSITY_RX_RATE_INIT_VAL;
			pAd->rec_start_cn_flag = TRUE;
			pAd->rec_start_rx_rate_flag = TRUE;
		} else {
			if (pAd->diversity_ctl.dbg_flag_lvl1 != 0) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_WARN,
					"band(%d) AvgCN is %s ready(%d,%d), AvgRxRate is %s ready(%d,%d)!!!\n", band_idx,
					(is_cn_ready == FALSE)?"not":"", pAd->cn_rec_sum, pAd->cn_rec_num,
					(is_rx_rate_ready == FALSE)?"not":"", pAd->rate_rec_sum, pAd->rate_rec_num);
			}
			return FALSE;
		}
	}

	if (band_idx == DBDC_BAND0) {
		tp_th_ul = UL_MODE_TH_2G;
		tp_th_dl = DL_MODE_TH_2G;
		countdown = ANT_COUNTDOWN_2G;
		rssi_th = ANT_DIVERSITY_RSSI_TH_2G;
	} else if (band_idx == DBDC_BAND1) {
		tp_th_ul= UL_MODE_TH_5G;
		tp_th_dl = DL_MODE_TH_5G;
		countdown = ANT_COUNTDOWN_5G;
		rssi_th = ANT_DIVERSITY_RSSI_TH_5G;
	}

	if (pAd->diversity_ctl.dbg_countdown != 0)
		countdown = pAd->diversity_ctl.dbg_countdown;
	if (pAd->diversity_ctl.dbg_ul_tp_th != 0)
		tp_th_ul = pAd->diversity_ctl.dbg_ul_tp_th;
	if (pAd->diversity_ctl.dbg_flag_lvl1 != 0)
		show_ant_diversity_debug_log(pAd, band_idx);

	if ((cur_mode != last_mode) && (last_mode!= TRAFFIC_0)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"band(%d) condition from %d to %d!\n",
			band_idx, last_mode, cur_mode);
		ant_diversity_ctrl_init(pAd, band_idx);
		ant_diversity_restore_rxv_cr(pAd, band_idx);
		pAd->diversity_ctl.last_traffic_mode = cur_mode;
		return FALSE;
	}

	if ((pAd->diversity_ctl.client_num == 1)
		&& (pAd->diversity_ctl.sta_rssi > rssi_th)
		&& (pAd->diversity_ctl.ap_nss > 1)
		&& (pAd->diversity_ctl.sta_nss > 1)
		&& (pAd->diversity_ctl.is_he_sta)
		&& (((cur_mode == TRAFFIC_UL_MODE) && (cur_rx_tp > tp_th_ul))
			|| ((cur_mode == TRAFFIC_DL_MODE) && (pAd->diversity_ctl.cur_tx_tput[cur_ant] > tp_th_dl)))) {
		if (cur_status == ANT_DIVERSITY_STATUS_IDLE) {
			pAd->diversity_ctl.diversity_status = ANT_DIVERSITY_STATUS_STREAM_START;
			pAd->diversity_ctl.ant_switch_countdown = countdown;
			if (cur_mode == TRAFFIC_UL_MODE) {
				ant_diversity_backup_and_enable_rxv_cr(pAd, band_idx);
				pAd->rec_start_cn_flag = TRUE;
				pAd->rec_start_rx_rate_flag = TRUE;
			}
		}
		return TRUE;
	} else {
		/* reset indicators when conditions are not met */
		if (cur_status != ANT_DIVERSITY_STATUS_IDLE) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "band(%d) condition mismatch, not allowed!\n", band_idx);
			ant_diversity_ctrl_init(pAd, band_idx);
			ant_diversity_restore_rxv_cr(pAd, band_idx);
			allowed = FALSE;
		}
	}

	return allowed;
}


VOID diversity_sel_ant_by_higher_tp(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT32 tp_main = pAd->diversity_ctl.cur_tx_tput[ANT_MAIN];
	UINT32 tp_aux = pAd->diversity_ctl.cur_tx_tput[ANT_AUX];
	UINT8 cur_ant = pAd->diversity_ctl.cur_ant;
	UINT8 sel_ant = 0;
	UINT8 cur_mode = pAd->diversity_ctl.cur_traffic_mode;

	if (cur_mode != TRAFFIC_DL_MODE) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Wrong traffic mode(%d)! Keep cur_ant(%d)\n",
			cur_mode, cur_ant);
		return;
	}

	if (tp_main > tp_aux)
		sel_ant = ANT_MAIN;
	else if (tp_main < tp_aux)
		sel_ant = ANT_AUX;
	else
		sel_ant = cur_ant;

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"tp_main(%dM), tp_aux(%dM), cur_ant(%d), select ant(%d)\n",
		tp_main, tp_aux, cur_ant, sel_ant);

	if (sel_ant != cur_ant)
		diversity_switch_antenna(pAd, band_idx, sel_ant);
}

VOID diversity_sel_ant_by_better_cn(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT32 cn_main = pAd->diversity_ctl.cur_rx_cn[ANT_MAIN];
	UINT32 cn_aux = pAd->diversity_ctl.cur_rx_cn[ANT_AUX];
	UINT8 cur_ant = pAd->diversity_ctl.cur_ant;
	UINT8 sel_ant = 0;
	UINT32 delta_cn = 0;

	if (cn_aux == cn_main) {
		pAd->diversity_ctl.rx_delta_cn = 0;
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
		"CN main & aux are the same value(%d, %d), keep current ant(%d)\n",
		cn_main, cn_aux, cur_ant);
		return;
	}

	if (cn_aux > cn_main) {
		delta_cn = cn_aux - cn_main;
		sel_ant = ANT_MAIN;
	}
	else {
		delta_cn = cn_main - cn_aux;
		sel_ant = ANT_AUX;
	}

	pAd->diversity_ctl.rx_delta_cn = delta_cn;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"cn_main(%d), cn_aux(%d), delta_cn(%d), cur_ant(%d), select ant(%d)\n",
		cn_main, cn_aux, delta_cn, cur_ant, sel_ant);

	if (sel_ant != cur_ant)
		diversity_switch_antenna(pAd, band_idx, sel_ant);
}

BOOLEAN diversity_sel_ant_by_better_rx_rate(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT32 rate_main = pAd->diversity_ctl.cur_rx_rate[ANT_MAIN];
	UINT32 rate_aux = pAd->diversity_ctl.cur_rx_rate[ANT_AUX];
	UINT8 cur_ant = pAd->diversity_ctl.cur_ant;
	UINT8 sel_ant = 0;
	UINT32 delta_rate_th = 0;

	if (band_idx == DBDC_BAND0)
		delta_rate_th = RX_RATE_DELTA_TH_2G;
	else
		delta_rate_th = RX_RATE_DELTA_TH_5G;

	if (pAd->diversity_ctl.dbg_ul_rate_delta_th != 0) //dbg
		delta_rate_th = pAd->diversity_ctl.dbg_ul_rate_delta_th;

	if ((rate_main > rate_aux) && ((rate_main - rate_aux) > delta_rate_th))
		sel_ant = ANT_MAIN;
	else if ((rate_aux > rate_main) && ((rate_aux - rate_main) > delta_rate_th))
		sel_ant = ANT_AUX;
	else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
		"band(%d), Rx rate is similar:main(%d,%d),delta_th(%d),select by CN alg!\n",
		band_idx, rate_main, rate_aux, delta_rate_th);
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"band(%d), rate_main(%d), rate_aux(%d), delta_th(%d), select ant(%d)\n",
		band_idx, rate_main, rate_aux, delta_rate_th, sel_ant);

	if (sel_ant != cur_ant)
		diversity_switch_antenna(pAd, band_idx, sel_ant);
	return TRUE;
}

BOOLEAN ant_diversity_tp_degrade(RTMP_ADAPTER *pAd, UINT8 band_idx, UINT8 cur_ant)
{
	BOOLEAN tp_degrade = FALSE;
	UINT32 last_tp;
	UINT8 cur_mode = pAd->diversity_ctl.cur_traffic_mode;
	UINT32 cur_tp = 0;
	UINT8 degrade_th = 0;

	if (cur_mode != TRAFFIC_DL_MODE) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Band(%d), Error!!! Cur_mode is incorrect!!!(%d)\n",
			band_idx, cur_mode);
		return tp_degrade;

		cur_tp = pAd->diversity_ctl.cur_tx_tput[cur_ant];
		last_tp = pAd->diversity_ctl.last_tx_tput[cur_ant];
		if (band_idx == DBDC_BAND0)
			degrade_th = DL_DEGRADE_TH_2G;
		else
			degrade_th = DL_DEGRADE_TH_5G;
	}

	if (pAd->diversity_ctl.dbg_tp_deg_th != 0)
		degrade_th = pAd->diversity_ctl.dbg_tp_deg_th;

	if ((cur_tp * 100) < (last_tp * degrade_th)) {
		tp_degrade = TRUE;
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"cur_ant(%d), band_idx(%d), last_tp(%dM), cur_tp(%dM), tp_degrade(%d)\n",
			cur_ant, band_idx, last_tp, cur_tp, tp_degrade);
	}
	return tp_degrade;
}

UINT8 ant_diversity_cn_degrade(RTMP_ADAPTER *pAd, UINT8 band_idx, UINT8 cur_ant)
{
	UINT8 cn_degrade = ANT_DIVERSITY_CN_DEGRADE_FALSE;
	UINT32 last_cn = 0;
	UINT32 cur_cn = 0;
	UINT32 delta_cn = 0;
	UINT8 delta_cn_max_th = 0;
	UINT8 delta_cn_min_th = 0;
	UINT32 peer_ant_cn = 0;
	UINT8 cn_deg_cnt = pAd->diversity_ctl.cn_deg_cnt;
	UINT8 cn_deg_cnt_th = 0;

	cur_cn = pAd->diversity_ctl.cur_rx_cn[cur_ant];
	last_cn = pAd->diversity_ctl.last_rx_cn[cur_ant];

	if (band_idx == DBDC_BAND0) {
		delta_cn_max_th = UL_CN_DEGRADE_TH_MAX_2G;
		delta_cn_min_th = UL_CN_DEGRADE_TH_MIN_2G;
		cn_deg_cnt_th = ANT_DIVERSITY_CN_DEGRADE_CNT_TH_2G;
	}
	else {
		delta_cn_max_th = UL_CN_DEGRADE_TH_MAX_5G;
		delta_cn_min_th = UL_CN_DEGRADE_TH_MIN_5G;
		cn_deg_cnt_th = ANT_DIVERSITY_CN_DEGRADE_CNT_TH_5G;
	}

	if (pAd->diversity_ctl.dbg_cn_deg_th_max != 0) //dbg
		delta_cn_max_th = pAd->diversity_ctl.dbg_cn_deg_th_max;
	if (pAd->diversity_ctl.dbg_cn_deg_th_min != 0)//dbg
		delta_cn_min_th = pAd->diversity_ctl.dbg_cn_deg_th_min;
	if (pAd->diversity_ctl.dbg_cn_deg_cnt_th != 0)//dbg
		cn_deg_cnt_th = pAd->diversity_ctl.dbg_cn_deg_cnt_th;

	if (cur_cn > last_cn) {
		delta_cn = pAd->diversity_ctl.rx_delta_cn;
		if (cur_ant == ANT_MAIN)
			peer_ant_cn = pAd->diversity_ctl.last_rx_cn[ANT_AUX];
		else
			peer_ant_cn = pAd->diversity_ctl.last_rx_cn[ANT_MAIN];

		if ((cur_cn - last_cn) >= delta_cn_max_th) {
			cn_deg_cnt++;
			cn_degrade = ANT_DIVERSITY_CN_DEGRADE_TRUE;
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"[Case 1]--> cur_ant(%d), band_idx(%d), last_cn(%d), cur_cn(%d), cn_degrade(%d)\n",
			cur_ant, band_idx, last_cn, cur_cn, cn_degrade);
		} else if ((delta_cn > delta_cn_min_th) && (cur_cn > peer_ant_cn)) {
			cn_deg_cnt++;
			cn_degrade = ANT_DIVERSITY_CN_DEGRADE_TRUE;
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"[Case 2]--> cur_ant(%d), band_idx(%d), last_cn(%d), cur_cn(%d), delta_cn(%d), peer_ant_cn(%d), cn_degrade(%d)\n",
			cur_ant, band_idx, last_cn, cur_cn, delta_cn, peer_ant_cn, cn_degrade);
		} else if ((delta_cn <= delta_cn_min_th) && (cur_cn >= (peer_ant_cn + delta_cn_min_th))) {
			cn_deg_cnt++;
			cn_degrade = ANT_DIVERSITY_CN_DEGRADE_TRUE;
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"[Case 3]--> cur_ant(%d), band_idx(%d), last_cn(%d), cur_cn(%d), delta_cn(%d), peer_ant_cn(%d), cn_degrade(%d)\n",
			cur_ant, band_idx, last_cn, cur_cn, delta_cn, peer_ant_cn, cn_degrade);
		}
	}

	if (cn_degrade == ANT_DIVERSITY_CN_DEGRADE_TRUE) {
		if (cn_deg_cnt < cn_deg_cnt_th) {
			cn_degrade = ANT_DIVERSITY_CN_DEGRADE_WAITING;
			pAd->diversity_ctl.cn_deg_cnt++;
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"band_idx(%d), cn_deg_cnt(%d) < cn_deg_cnt_th(%d), keep current ant(%d)!\n",
			band_idx, cn_deg_cnt, cn_deg_cnt_th, cur_ant);
		} else {
			pAd->diversity_ctl.cn_deg_cnt = 0;
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"band_idx(%d), CN degrade continuously for %d times at ant(%d)!\n",
			band_idx, cn_deg_cnt, cur_ant);
		}
	} else
		pAd->diversity_ctl.cn_deg_cnt = 0;

	return cn_degrade;
}

UINT8 ant_diversity_rx_rate_degrade(RTMP_ADAPTER *pAd, UINT8 band_idx, UINT8 cur_ant)
{
	UINT32 rx_rate_degrade = ANT_DIVERSITY_RX_RATE_DEGRADE_FALSE;
	UINT32 last_rx_rate = 0;
	UINT32 cur_rx_rate = 0;
	UINT32 delta_rate = 0;
	UINT32 delta_rate_th = 0;
	UINT32 peer_ant_rx_rate = 0;
	UINT8 rx_rate_deg_cnt = pAd->diversity_ctl.rx_rate_deg_cnt;
	UINT8 rx_rate_deg_cnt_th = 0;

	cur_rx_rate = pAd->diversity_ctl.cur_rx_rate[cur_ant];
	last_rx_rate = pAd->diversity_ctl.last_rx_rate[cur_ant];

	if (band_idx == DBDC_BAND0) {
		delta_rate_th = UL_RX_RATE_DEGRADE_TH_2G;
		rx_rate_deg_cnt_th = ANT_DIVERSITY_RX_RATE_DEGRADE_CNT_TH_2G;
	}
	else {
		delta_rate_th = UL_RX_RATE_DEGRADE_TH_5G;
		rx_rate_deg_cnt_th = ANT_DIVERSITY_RX_RATE_DEGRADE_CNT_TH_5G;
	}

	if (pAd->diversity_ctl.dbg_rx_rate_deg_th != 0) //dbg
		delta_rate_th = pAd->diversity_ctl.dbg_rx_rate_deg_th;
	if (pAd->diversity_ctl.dbg_rx_rate_deg_cnt_th != 0)//dbg
		rx_rate_deg_cnt_th = pAd->diversity_ctl.dbg_rx_rate_deg_cnt_th;

	if (cur_rx_rate < last_rx_rate) {
		delta_rate = last_rx_rate - cur_rx_rate;
		if (cur_ant == ANT_MAIN)
			peer_ant_rx_rate = pAd->diversity_ctl.last_rx_rate[ANT_AUX];
		else
			peer_ant_rx_rate = pAd->diversity_ctl.last_rx_rate[ANT_MAIN];

		if ((delta_rate >= delta_rate_th) && (cur_rx_rate < peer_ant_rx_rate)) {
			rx_rate_deg_cnt++;
			rx_rate_degrade = ANT_DIVERSITY_RX_RATE_DEGRADE_TRUE;
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"[Case 1]--> cur_ant(%d), band_idx(%d), last_rx_rate(%d), cur_rx_rate(%d), peer_ant_rx_rate(%d)\n",
			cur_ant, band_idx, last_rx_rate, cur_rx_rate, peer_ant_rx_rate);
		}
	}

	if (rx_rate_degrade == ANT_DIVERSITY_RX_RATE_DEGRADE_TRUE) {
		if (rx_rate_deg_cnt < rx_rate_deg_cnt_th) {
			rx_rate_degrade = ANT_DIVERSITY_RX_RATE_DEGRADE_WAITING;
			pAd->diversity_ctl.rx_rate_deg_cnt++;
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"band_idx(%d), rx_rate_deg_cnt(%d) < rx_rate_deg_cnt_th(%d), keep current ant(%d)!\n",
			band_idx, rx_rate_deg_cnt, rx_rate_deg_cnt_th, cur_ant);
		} else {
			pAd->diversity_ctl.rx_rate_deg_cnt = 0;
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"band_idx(%d), CN degrade continuously for %d times at ant(%d)!Need switch to another ant!\n",
			band_idx, rx_rate_deg_cnt, cur_ant);
		}
	} else
		pAd->diversity_ctl.rx_rate_deg_cnt = 0;

	return rx_rate_degrade;
}

VOID ant_diversity_process(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT8 cur_ant = pAd->diversity_ctl.cur_ant;
	UINT32 cur_tp_ul = pAd->diversity_ctl.cur_rx_tput[cur_ant];
	UINT32 cur_tp_dl = pAd->diversity_ctl.cur_tx_tput[cur_ant];
	UINT8 countdown = 0;
	UINT8 cur_mode = pAd->diversity_ctl.cur_traffic_mode;
	UINT32 cur_cn = pAd->diversity_ctl.cur_rx_cn[cur_ant];
	UINT32 cur_status = pAd->diversity_ctl.diversity_status;
	UINT32 cur_rx_rate = pAd->diversity_ctl.cur_rx_rate[cur_ant];
	UINT8 cn_degrade_status = ANT_DIVERSITY_CN_DEGRADE_FALSE;
	UINT8 rx_rate_degrade_status = ANT_DIVERSITY_RX_RATE_DEGRADE_FALSE;

	if (cur_status == ANT_DIVERSITY_STATUS_IDLE) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"should not happen!!! band_idx(%d)\n", band_idx);
		return;
	}

	if (band_idx == DBDC_BAND0)
		countdown = ANT_COUNTDOWN_2G;
	else
		countdown = ANT_COUNTDOWN_5G;
	if (pAd->diversity_ctl.dbg_countdown != 0)
		countdown = pAd->diversity_ctl.dbg_countdown;

	/* if T-put or CN degrade more than TH, need to switch to another ant, then compare*/
	if (cur_status == ANT_DIVERSITY_STATUS_TP_RUNNING) {
		if (cur_mode == TRAFFIC_UL_MODE) {
			if (pAd->diversity_ctl.ant_switch_countdown == 0) {
				if (ant_diversity_rx_rate_degrade(pAd, band_idx, cur_ant) == ANT_DIVERSITY_RX_RATE_DEGRADE_TRUE) {
					if (cur_ant == ANT_MAIN)
						diversity_switch_antenna(pAd, band_idx, ANT_AUX);
					else
						diversity_switch_antenna(pAd, band_idx, ANT_MAIN);
					pAd->diversity_ctl.diversity_status = ANT_DIVERSITY_STATUS_TP_COMPARE;
					pAd->diversity_ctl.ant_switch_countdown = countdown;
				}
			} else
				pAd->diversity_ctl.ant_switch_countdown--;
		} else {
			if (ant_diversity_tp_degrade(pAd, band_idx, cur_ant) == TRUE) {
				if (cur_ant == ANT_MAIN)
					diversity_switch_antenna(pAd, band_idx, ANT_AUX);
				else
					diversity_switch_antenna(pAd, band_idx, ANT_MAIN);
				pAd->diversity_ctl.diversity_status = ANT_DIVERSITY_STATUS_TP_COMPARE;
			}
		}
	}
	/* for DL: compare T-put; for UL: compare Rx rate & CN base on main & aux, then select the better one*/
	if (cur_status == ANT_DIVERSITY_STATUS_TP_COMPARE) {
		if (cur_mode == TRAFFIC_UL_MODE) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"Band(%d) compare stage! countdown(%d).\n",
				band_idx, pAd->diversity_ctl.ant_switch_countdown);
			if (pAd->diversity_ctl.ant_switch_countdown == 0) {
				if (diversity_sel_ant_by_better_rx_rate(pAd, band_idx) == FALSE) {
					diversity_sel_ant_by_better_cn(pAd, band_idx);
				}
				pAd->diversity_ctl.ant_switch_countdown = countdown;
				pAd->diversity_ctl.diversity_status = ANT_DIVERSITY_STATUS_TP_RUNNING;
			} else
				pAd->diversity_ctl.ant_switch_countdown--;
		}
		else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"Band(%d) T-put compare\n", band_idx);
			diversity_sel_ant_by_higher_tp(pAd, band_idx);
			pAd->diversity_ctl.diversity_status = ANT_DIVERSITY_STATUS_TP_RUNNING;
		}
	}
	/* at the beginning of T-put streaming, forcedly switch to ANT_AUX to calculate the T-put/CN/Rx rate, then compare with ANT_MAIN*/
	if (cur_status == ANT_DIVERSITY_STATUS_STREAM_START) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"Band(%d) streaming start, countdown(%d), cur_traf_mode(%d)\n",
			band_idx, pAd->diversity_ctl.ant_switch_countdown, pAd->diversity_ctl.cur_traffic_mode);
		if (pAd->diversity_ctl.ant_switch_countdown == 0) {
			diversity_switch_antenna(pAd, band_idx, ANT_AUX);
			pAd->diversity_ctl.diversity_status = ANT_DIVERSITY_STATUS_TP_COMPARE;
			pAd->diversity_ctl.ant_switch_countdown = countdown;
		} else {
			pAd->diversity_ctl.ant_switch_countdown--;
		}
	}

	pAd->diversity_ctl.last_rx_tput[cur_ant] = cur_tp_ul;
	pAd->diversity_ctl.last_tx_tput[cur_ant] = cur_tp_dl;
	if (cn_degrade_status != ANT_DIVERSITY_CN_DEGRADE_WAITING)
		pAd->diversity_ctl.last_rx_cn[cur_ant] = cur_cn;
	if (rx_rate_degrade_status != ANT_DIVERSITY_RX_RATE_DEGRADE_WAITING)
		pAd->diversity_ctl.last_rx_rate[cur_ant] = cur_rx_rate;
	return;
}

INT set_ant_diversity_debug_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 log_enable_1st = 0;
	UINT32 log_enable_2nd = 0;

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "Enter!\n");

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d-%d", &(band_idx), &(log_enable_1st), &(log_enable_2nd));
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band_idx = %d(deprecated), log_enable_1st = %d, log_enable_2nd = %d\n",
				band_idx, log_enable_1st, log_enable_2nd);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1))
			|| ((log_enable_1st != 0) && (log_enable_1st != 1)) || ((log_enable_2nd != 0) && (log_enable_2nd != 1)) || (i4Recv != 3)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"set band(%d)(deprecated) log_enable_1st %s, log_enable_2nd %s success!\n",
				band_idx, ((log_enable_1st == 1)?"enable":"disable"),
				((log_enable_2nd == 1)?"enable":"disable"));
			pAd->diversity_ctl.dbg_flag_lvl1 = log_enable_1st;
			pAd->diversity_ctl.dbg_flag_lvl2 = log_enable_2nd;

			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_disable_forcedly(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 disable = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(disable));
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band_idx = %d(deprecated), disable = %d\n", band_idx, disable);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1))
			|| ((disable != 0) && (disable != 1)) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band(%d)(deprecated), set diversity %s success!\n",
				band_idx, ((disable == 0)?"enable":"disable"));
			pAd->diversity_ctl.diversity_force_disable = disable;
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_dbg_tp_deg_th_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 degrade_th = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(degrade_th));
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band_idx = %d(deprecated), degrade_th = %d\n", band_idx, degrade_th);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1)) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band(%d)(deprecated), set T-put degrade TH(%d) success!\n",
				band_idx, degrade_th);
			pAd->diversity_ctl.dbg_tp_deg_th = degrade_th;
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_dbg_cn_deg_th_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 max_cn_th = 0;
	UINT32 min_cn_th = 0;

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "Enter!\n");

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d-%d", &(band_idx), &(max_cn_th), &(min_cn_th));
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band_idx = %d(deprecated), max_cn_th = %d, min_cn_th = %d\n",
				band_idx, max_cn_th, min_cn_th);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1))
			|| (max_cn_th < 0) || (min_cn_th < 0) || (max_cn_th >255) || (min_cn_th >255)
			|| (i4Recv != 3)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band(%d)(deprecated), set CN degrade threshold(%d:%d) success!\n",
				band_idx, max_cn_th, min_cn_th);
			pAd->diversity_ctl.dbg_cn_deg_th_min = min_cn_th;
			pAd->diversity_ctl.dbg_cn_deg_th_max = max_cn_th;
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_dbg_rx_rate_deg_th_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 rx_rate_deg_th = 0;

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "Enter!\n");

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(rx_rate_deg_th));
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band_idx = %d(deprecated), rx_rate_deg_th = %d\n",
				band_idx, rx_rate_deg_th);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1)) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band(%d)(deprecated), set Rx rate degrade threshold(%d) success!\n",
				band_idx, rx_rate_deg_th);
			pAd->diversity_ctl.dbg_rx_rate_deg_th = rx_rate_deg_th;
			return TRUE;
		}
	}
	return -1;
}

INT set_read_interval_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value = 0;

	value = (UINT16)simple_strtol(arg, 0, 10);
	pAd->cn_rate_read_interval = value;
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		"cn_rate_read_interval = %d\n", value);
	return TRUE;
}

INT set_ant_diversity_dbg_countdown_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 count_down = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(count_down));
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band_idx = %d(deprecated), count_down = %d\n", band_idx, count_down);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1)) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band(%d)(deprecated), set countdown TH(%d) success!\n",
				band_idx, count_down);
			pAd->diversity_ctl.dbg_countdown = count_down;
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_dbg_ul_th_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 tp_th = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(tp_th));
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band_idx = %d(deprecated), ul_tp_th = %d\n", band_idx, tp_th);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1)) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band(%d)(deprecated), set uplink T-put TH(%d) success!\n",
				band_idx, tp_th);
			pAd->diversity_ctl.dbg_ul_tp_th = tp_th;
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_select_antenna(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 ant_idx = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(ant_idx));
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band_idx = %d, ant_idx = %d\n", band_idx, ant_idx);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1))
			|| ((ant_idx != ANT_MAIN) && (ant_idx != ANT_AUX))
			|| (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band(%d), switch to ant(%d) success!\n", band_idx, ant_idx);
			ant_diversity_gpio_control(pAd, band_idx, ant_idx);
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_rx_rate_delta_th(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 delta = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(delta));
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band_idx = %d(deprecated), delta_th = %d\n", band_idx, delta);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1)) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band(%d)(deprecated), set rx rate delta TH(%d) success!\n",
				band_idx, delta);
			pAd->diversity_ctl.dbg_ul_rate_delta_th = delta;
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_cn_deg_continuous_th(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 threshold = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(threshold));
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band_idx = %d(deprecated), threshold = %d\n", band_idx, threshold);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1)) || (threshold == 0) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band(%d)(deprecated), set CN continuous degrade threshold(%d) success!\n",
				band_idx, threshold);
			pAd->diversity_ctl.dbg_cn_deg_cnt_th = threshold;
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_rx_rate_deg_continuous_th(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 threshold = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(threshold));
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band_idx = %d(deprecated), threshold = %d\n", band_idx, threshold);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1)) || (threshold == 0) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"band(%d)(deprecated), set Rx rate continuous degrade threshold(%d) success!\n",
				band_idx, threshold);
			pAd->diversity_ctl.dbg_rx_rate_deg_cnt_th = threshold;
			return TRUE;
		}
	}
	return -1;
}
#endif

#ifdef DOT1X_SUPPORT
/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Frame to notify 802.1x daemon. This is a internal command

 Arguments:

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN DOT1X_InternalCmdAction(
	IN  PRTMP_ADAPTER	pAd,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN UINT8 cmd)
{
	INT 			apidx = MAIN_MBSSID;
	UCHAR			RalinkIe[9] = {221, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
	UCHAR			s_addr[MAC_ADDR_LEN];
	UCHAR			EAPOL_IE[] = {0x88, 0x8e};
#ifdef OCE_FILS_SUPPORT
#define OCE_FILS_CMD_PAYLOAD_LEN 1500
#else
#define OCE_FILS_CMD_PAYLOAD_LEN 0
#endif /* OCE_FILS_SUPPORT */
#ifdef RADIUS_ACCOUNTING_SUPPORT
	DOT1X_QUERY_STA_DATA	data;
#define FRAME_BUF_LEN	(LENGTH_802_3 + sizeof(RalinkIe) + VLAN_HDR_LEN + sizeof(DOT1X_QUERY_STA_DATA) + OCE_FILS_CMD_PAYLOAD_LEN)
#else
#define FRAME_BUF_LEN	(LENGTH_802_3 + sizeof(RalinkIe) + VLAN_HDR_LEN + OCE_FILS_CMD_PAYLOAD_LEN)
#endif /*RADIUS_ACCOUNTING_SUPPORT*/

	UINT			frame_len = 0;
	PCHAR			PFrameBuf;
	UINT8			offset = 0;
	USHORT			bss_Vlan_Priority = 0;
	USHORT			bss_Vlan;
	USHORT			TCI;
	/* Init the frame buffer */
	os_alloc_mem(pAd, (UCHAR **)&PFrameBuf, FRAME_BUF_LEN);
	if (PFrameBuf == NULL)
		return FALSE;
	else
		os_zero_mem(PFrameBuf, FRAME_BUF_LEN);

	if (pEntry && (pEntry->func_tb_idx < MAX_MBSSID_NUM(pAd))) {
#ifdef RADIUS_ACCOUNTING_SUPPORT
		NdisMoveMemory(data.StaAddr, pEntry->Addr, MAC_ADDR_LEN);
		data.rx_bytes = pEntry->RxBytes;
		data.tx_bytes = pEntry->TxBytes;
		data.rx_packets = pEntry->RxPackets.u.LowPart;
		data.tx_packets = pEntry->TxPackets.u.LowPart;
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
			"rx_byte:%lu  tx_byte:%lu rx_pkt:%lu tx_pkt:%lu\n",
			data.rx_bytes, data.tx_bytes, data.rx_packets, data.rx_packets);
#endif /* RADIUS_ACCOUNTING_SUPPORT */
		apidx = pEntry->func_tb_idx;
		NdisMoveMemory(s_addr, pEntry->Addr, MAC_ADDR_LEN);
	} else {
		/* Fake a Source Address for transmission */
		NdisMoveMemory(s_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
		s_addr[0] |= 0x80;
	}

	/* Assign internal command for Ralink dot1x daemon */
	RalinkIe[5] = cmd;

	if (apidx < 0 || apidx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_ERROR,
				"apidx(%d) out of range\n", apidx);
		os_free_mem(PFrameBuf);
		return FALSE;
	}

	if (pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID) {
		bss_Vlan = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID;
		bss_Vlan_Priority = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_Priority;
		frame_len = (LENGTH_802_3 + sizeof(RalinkIe) + VLAN_HDR_LEN);
		MAKE_802_3_HEADER(PFrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, s_addr, EAPOL_IE);
		offset += LENGTH_802_3 - 2;
		NdisMoveMemory((PFrameBuf + offset), TPID, 2);
		offset += 2;
		TCI = (bss_Vlan & 0x0FFF) | ((bss_Vlan_Priority & 0x7) << 13);
#ifndef CFG_BIG_ENDIAN
		TCI = SWAP16(TCI);
#endif
		*(USHORT *)(PFrameBuf + offset) = TCI;
		offset += 2;
		NdisMoveMemory((PFrameBuf + offset), EAPOL_IE, 2);
		offset += 2;
	} else {
		frame_len = (LENGTH_802_3 + sizeof(RalinkIe));
		/* Prepare the 802.3 header */
		MAKE_802_3_HEADER(PFrameBuf,
						  pAd->ApCfg.MBSSID[apidx].wdev.bssid,
						  s_addr,
						  EAPOL_IE);
		offset += LENGTH_802_3;
	}

	/* Prepare the specific header of internal command */
	NdisMoveMemory((PFrameBuf + offset), RalinkIe, sizeof(RalinkIe));
#ifdef RADIUS_ACCOUNTING_SUPPORT
	offset += sizeof(RalinkIe);
	/*add accounting info*/
	NdisMoveMemory((PFrameBuf + offset), (unsigned char *)&data, sizeof(DOT1X_QUERY_STA_DATA));
#endif /*RADIUS_ACCOUNTING_SUPPORT*/

#ifdef OCE_FILS_SUPPORT
	if ((cmd == DOT1X_MLME_EVENT) ||
		(cmd == DOT1X_AEAD_ENCR_EVENT)) {
		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_ERROR,
				"pEntry is NULL\n");
			os_free_mem(PFrameBuf);
			return FALSE;
		}

#ifdef RADIUS_ACCOUNTING_SUPPORT
		offset += sizeof(DOT1X_QUERY_STA_DATA);
#else
		offset += sizeof(RalinkIe);
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
		frame_len += pEntry->filsInfo.pending_ie_len;
		if (frame_len > FRAME_BUF_LEN) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_ERROR,
				"frame_len(%d) > FRAME_BUF_LEN(%d) (cmd=%d)\n",
				frame_len, (UINT32)FRAME_BUF_LEN, cmd);
			os_free_mem(PFrameBuf);
			return FALSE;
		}

		NdisMoveMemory((PFrameBuf + offset), pEntry->filsInfo.pending_ie, pEntry->filsInfo.pending_ie_len);
	} else if (cmd == DOT1X_AEAD_DECR_EVENT) {
		PHEADER_802_11 pHeader = NULL;
		UCHAR hdr_len = LENGTH_802_11;

		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_ERROR,
				"pEntry is NULL\n");
			os_free_mem(PFrameBuf);
			return FALSE;
		}

#ifdef RADIUS_ACCOUNTING_SUPPORT
		offset += sizeof(DOT1X_QUERY_STA_DATA);
#else
		offset += sizeof(RalinkIe);
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
		/* PTK Info */
		frame_len += pEntry->filsInfo.PTK_len;
		NdisMoveMemory((PFrameBuf + offset), pEntry->filsInfo.PTK, pEntry->filsInfo.PTK_len);
		offset += pEntry->filsInfo.PTK_len;

		/* EAPOL Packet */
		pHeader = (PHEADER_802_11)pEntry->filsInfo.pending_ie;
#ifdef A4_CONN
		if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
			hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif /* A4_CONN */

		frame_len += (pEntry->filsInfo.pending_ie_len - hdr_len - LENGTH_802_1_H);
		if (frame_len > FRAME_BUF_LEN) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_ERROR,
				"frame_len(%d) > FRAME_BUF_LEN(%d) (cmd=%d)\n",
				frame_len, (UINT32)FRAME_BUF_LEN, cmd);
			os_free_mem(PFrameBuf);
			return FALSE;
		}
		NdisMoveMemory((PFrameBuf + offset), &pEntry->filsInfo.pending_ie[hdr_len + LENGTH_802_1_H],
			(pEntry->filsInfo.pending_ie_len - hdr_len - LENGTH_802_1_H));
	}
#endif /* OCE_FILS_SUPPORT */

	/* Report to upper layer */
	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
		"==================================================\n");
	hex_dump_with_lvl("PFrameBuf", (char *)PFrameBuf, frame_len, DBG_LVL_INFO);
	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
		"==================================================\n");
	if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, (char *)PFrameBuf, frame_len) == FALSE) {
		os_free_mem(PFrameBuf);
		return FALSE;
	}
	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO, "done. (cmd=%d)\n", cmd);
	os_free_mem(PFrameBuf);
	return TRUE;
}

/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Frame to trigger 802.1x EAP state machine.

 Arguments:

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN DOT1X_EapTriggerAction(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	/* TODO: shiang-usw, fix me for pEntry->apidx to func_tb_idx */
	INT				apidx = MAIN_MBSSID;
	UCHAR			eapol_start_1x_hdr[4] = {0x01, 0x01, 0x00, 0x00};
	UINT8			frame_len = LENGTH_802_3 + sizeof(eapol_start_1x_hdr);
	UCHAR			*FrameBuf = NULL;
	UINT8			offset = 0;
	USHORT			bss_Vlan_Priority = 0;
	USHORT			bss_Vlan;
	USHORT			TCI;

	if (!pEntry)
		return FALSE;
	if (IS_AKM_1X_Entry(pEntry) || IS_IEEE8021X_Entry(&pAd->ApCfg.MBSSID[apidx].wdev)) {
		/* Init the frame buffer */
		os_alloc_mem(pAd, (UCHAR **)&FrameBuf, (frame_len+32));
		if (FrameBuf == NULL) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_ERROR,
					"Fail to allocate memory!\n");
			return FALSE;
		}
		NdisZeroMemory(FrameBuf, frame_len);
		/* Assign apidx */
		apidx = pEntry->func_tb_idx;

		if (apidx < 0 || apidx >= MAX_BEACON_NUM) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_ERROR,
					"apidx(%d) out of range\n", apidx);
			os_free_mem(FrameBuf);
			return FALSE;
		}

		if (pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID) {
			/*Prepare 802.3 header including VLAN tag*/
			bss_Vlan = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID;
			bss_Vlan_Priority = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_Priority;
			frame_len += VLAN_HDR_LEN;
			MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, pEntry->Addr, EAPOL)
			offset += LENGTH_802_3 - 2;
			NdisMoveMemory((FrameBuf + offset), TPID, 2);
			offset += 2;
			TCI = (bss_Vlan & 0x0fff) | ((bss_Vlan_Priority & 0x7) << 13);
#ifndef CFG_BIG_ENDIAN
			TCI = SWAP16(TCI);
#endif
			*(USHORT *)(FrameBuf + offset) = TCI;
			offset += 2;
			NdisMoveMemory((FrameBuf + offset), EAPOL, 2);
			offset += 2;
		} else {
			/* Prepare the 802.3 header */
			MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, pEntry->Addr, EAPOL);
			offset += LENGTH_802_3;
		}

		/* Prepare a fake eapol-start body */
		NdisMoveMemory(&FrameBuf[offset], eapol_start_1x_hdr, sizeof(eapol_start_1x_hdr));
#ifdef CONFIG_HOTSPOT_R2

		if (pEntry) {
			BSS_STRUCT *pMbss = MBSS_GET(pEntry->pMbss);

			if ((pMbss->HotSpotCtrl.HotSpotEnable == 1) && (IS_AKM_WPA2_Entry(&pMbss->wdev)) &&
				(pEntry->hs_info.ppsmo_exist == 1)) {
				UCHAR HS2_Header[4] = {0x50, 0x6f, 0x9a, 0x12};

				memcpy(&FrameBuf[offset + sizeof(eapol_start_1x_hdr)], HS2_Header, 4);
				memcpy(&FrameBuf[offset + sizeof(eapol_start_1x_hdr) + 4], &pEntry->hs_info, sizeof(struct _sta_hs_info));
				frame_len += 4 + sizeof(struct _sta_hs_info);
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
						 "event eapol start, %x:%x:%x:%x\n",
						 FrameBuf[offset + sizeof(eapol_start_1x_hdr) + 4], FrameBuf[offset + sizeof(eapol_start_1x_hdr) + 5],
						 FrameBuf[offset + sizeof(eapol_start_1x_hdr) + 6], FrameBuf[offset + sizeof(eapol_start_1x_hdr) + 7]);
			}
		}

#endif

		/* Report to upper layer */
		if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, FrameBuf, frame_len) == FALSE)  {
			os_free_mem(FrameBuf);
			return FALSE;
		}
		os_free_mem(FrameBuf);

		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
			"Notify 802.1x daemon to trigger EAP-SM for this sta("MACSTR")\n",
			MAC2STR(pEntry->Addr));
	}

	return TRUE;
}

#endif /* DOT1X_SUPPORT */


INT rtmp_ap_init(RTMP_ADAPTER *pAd)
{
#ifdef WSC_AP_SUPPORT
	UCHAR j;
	BSS_STRUCT *mbss = NULL;
	struct wifi_dev *wdev = NULL;
	PWSC_CTRL pWscControl;

	for (j = BSS0; j < pAd->ApCfg.BssidNum; j++) {
		mbss = &pAd->ApCfg.MBSSID[j];
		wdev = &pAd->ApCfg.MBSSID[j].wdev;
		{
			pWscControl = &wdev->WscControl;
			pWscControl->WscRxBufLen = 0;
			pWscControl->pWscRxBuf = NULL;
			os_alloc_mem(pAd, &pWscControl->pWscRxBuf, MAX_MGMT_PKT_LEN);

			if (pWscControl->pWscRxBuf)
				NdisZeroMemory(pWscControl->pWscRxBuf, MAX_MGMT_PKT_LEN);

			pWscControl->WscTxBufLen = 0;
			pWscControl->pWscTxBuf = NULL;
			os_alloc_mem(pAd, &pWscControl->pWscTxBuf, MAX_MGMT_PKT_LEN);

			if (pWscControl->pWscTxBuf)
				NdisZeroMemory(pWscControl->pWscTxBuf, MAX_MGMT_PKT_LEN);
		}
	}

#endif /* WSC_AP_SUPPORT */
	APOneShotSettingInitialize(pAd);
	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
		"apstart up "MACSTR"\n", MAC2STR(pAd->CurrentAddress));
#ifdef BAND_STEERING
	if (pAd->ApCfg.BandSteering)
		BndStrg_Init(pAd);
#endif /* BAND_STEERING */


	APInitForMain(pAd);

	/* Set up the Mac address*/
	RtmpOSNetDevAddrSet(pAd->OpMode, pAd->net_dev, &pAd->CurrentAddress[0], NULL);

#ifdef CFG_SUPPORT_FALCON_MURU
	if (IS_MT7915(pAd)) {
		SetMuruPlatformTypeProc(pAd);
	}
#endif
#ifdef MT_FDB
	fdb_enable(pAd);
#endif /* MT_FDB */
#ifdef CFG_SUPPORT_FALCON_MURU
	if (IS_MT7915(pAd)) {
		/* Send In-Band Command to N9 in MT7915 */
		muru_cfg_dlul_limits(pAd, hc_get_hw_band_idx(pAd));
	}
#endif /* CFG_SUPPORT_FALCON_MURU */

	return NDIS_STATUS_SUCCESS;
}


VOID rtmp_ap_exit(RTMP_ADAPTER *pAd)
{
	BOOLEAN Cancelled;
#ifdef DOT11K_RRM_SUPPORT
	INT loop;
	PRRM_CONFIG pRrmCfg;
#endif /* DOT11K_RRM_SUPPORT */
#if defined(HOSTAPD_HS_R2_SUPPORT) || defined(CONFIG_DOT11U_INTERWORKING)
	INT idx;
#endif

	RTMPReleaseTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
#ifdef IDS_SUPPORT
	/* Init intrusion detection timer */
	RTMPReleaseTimer(&pAd->ApCfg.IDSTimer, &Cancelled);
#endif /* IDS_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++) {
		pRrmCfg = &pAd->ApCfg.MBSSID[loop].wdev.RrmCfg;

		RTMPCancelTimer(&pRrmCfg->QuietCB.QuietOffsetTimer, &Cancelled);
		RTMPReleaseTimer(&pRrmCfg->QuietCB.QuietOffsetTimer, &Cancelled);
		RTMPCancelTimer(&pRrmCfg->QuietCB.QuietTimer, &Cancelled);
		RTMPReleaseTimer(&pRrmCfg->QuietCB.QuietTimer, &Cancelled);
	}
#endif /*CONFIG_AP_SUPPORT*/
#ifdef CONFIG_STA_SUPPORT
	for (loop = 0; loop < MAX_MULTI_STA; loop++) {
		pRrmCfg = &pAd->StaCfg[loop].wdev.RrmCfg;

		RTMPCancelTimer(&pRrmCfg->QuietCB.QuietOffsetTimer, &Cancelled);
		RTMPReleaseTimer(&pRrmCfg->QuietCB.QuietOffsetTimer, &Cancelled);
		RTMPCancelTimer(&pRrmCfg->QuietCB.QuietTimer, &Cancelled);
		RTMPReleaseTimer(&pRrmCfg->QuietCB.QuietTimer, &Cancelled);
	}
#endif /*CONFIG_STA_SUPPORT*/
#endif /* DOT11K_RRM_SUPPORT */

#ifdef HOSTAPD_HS_R2_SUPPORT
	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[idx];
		PHOTSPOT_CTRL pHSCtrl = &mbss->HotSpotCtrl;

		OS_SEM_LOCK(&pHSCtrl->IeLock);
		if (pHSCtrl->HSIndicationIE) {
			os_free_mem(pHSCtrl->HSIndicationIE);
			pHSCtrl->HSIndicationIE = NULL;
		}
		if (pHSCtrl->P2PIE) {
			os_free_mem(pHSCtrl->P2PIE);
			pHSCtrl->P2PIE = NULL;
		}
		if (pHSCtrl->QosMapSetIE) {
			os_free_mem(pHSCtrl->QosMapSetIE);
			pHSCtrl->QosMapSetIE = NULL;
		}
		OS_SEM_UNLOCK(&pHSCtrl->IeLock);
	}
#endif /* HOSTAPD_HS_R2_SUPPORT */
#ifdef CONFIG_DOT11U_INTERWORKING
	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[idx];
		PGAS_CTRL pGasCtrl = &mbss->GASCtrl;

		OS_SEM_LOCK(&pGasCtrl->IeLock);
		if (pGasCtrl->InterWorkingIE) {
			os_free_mem(pGasCtrl->InterWorkingIE);
			pGasCtrl->InterWorkingIE = NULL;
		}
		if (pGasCtrl->AdvertisementProtoIE) {
			os_free_mem(pGasCtrl->AdvertisementProtoIE);
			pGasCtrl->AdvertisementProtoIE = NULL;
		}
		if (pGasCtrl->RoamingConsortiumIE) {
			os_free_mem(pGasCtrl->RoamingConsortiumIE);
			pGasCtrl->RoamingConsortiumIE = NULL;
		}
		OS_SEM_UNLOCK(&pGasCtrl->IeLock);
	}
#endif /* CONFIG_DOT11U_INTERWORKING */

#ifdef DOT11R_FT_SUPPORT
	FT_Release(pAd);
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	RTMP_11N_D3_TimerRelease(pAd);
#endif /*DOT11N_DRAFT3*/
#endif /*DOT11_N_SUPPORT*/
	/* Free BssTab & ChannelInfo tabbles.*/
	AutoChBssTableDestroy(pAd);
	ChannelInfoDestroy(pAd);
#ifdef IGMP_SNOOP_SUPPORT
	MultiCastFilterTableReset(pAd, &pAd->pMulticastFilterTable);
	MultiCastWLTableReset(pAd, &pAd->pMcastWLTable);
#ifdef IGMP_SNOOPING_DENY_LIST
	MulticastDLTableReset(pAd, &pAd->pMcastDLTable);
#endif
#endif /* IGMP_SNOOP_SUPPORT */
}

/*
* system security decision for ap mode
*/
UINT32 starec_ap_feature_decision(
	struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT64 *feature)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) wdev->sys_handle;
	UINT64 features = 0;
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(ad->physical_dev);

	if (cap->APPSMode == APPS_MODE2)
		features |= STA_REC_AP_PS_FEATURE;

	/*temport used for security will integrate to CMD*/
	if (IS_CIPHER_WEP(entry->SecConfig.PairwiseCipher))
		features |= STA_REC_INSTALL_KEY_FEATURE;

	/*return value, must use or operation*/
	*feature |= features;
	return TRUE;
}

#ifdef DOT11_EHT_BE
/*
 * translate BMGR MLD Type to MAC MLD Type (wlan driver/hwifi driver)
 * input: enum bmgr_mld_type
 * output: enum mtk_mac_mld_type
 */
UINT8 ap_mld_type_trans(enum bmgr_mld_type mld_type)
{
	switch (mld_type) {
	case BMGR_MLD_TYPE_NONE:
		return MAC_MLD_TYPE_NONE;
	case BMGR_MLD_TYPE_SINGLE:
		return MAC_MLD_TYPE_SINGLE;
	case BMGR_MLD_TYPE_MULTI:
		return MAC_MLD_TYPE_MULTI;
	default:
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"Error: unknown mld type %d, return none\n", mld_type);
		return MAC_MLD_TYPE_NONE;
	}
}

INT eht_ap_mld_create(
	IN struct wifi_dev *wdev,
	INOUT u8 *mld_group_idx,
	IN u8 *mld_addr,
	IN struct eht_mld_param *mld_param,
	OUT u8 *mld_type)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	u8 grp, type;

	if (!mld_addr || !mld_group_idx || !mld_type || !mld_param) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"Input error: grp:%p, addr:%p, type:%p, mld_param:%p\n",
			mld_group_idx, mld_addr, mld_type, mld_param);
		return NDIS_STATUS_FAILURE;
	}

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
		"===> Create AP MLD, grp(%d), addr(%pM)\n", *mld_group_idx, mld_addr);

	/* 1. sanity */
	if ((wdev->wdev_type == WDEV_TYPE_AP) && WMODE_CAP_BE(wdev->PhyMode)) {
		if (!NdisCmpMemory(mld_addr, ZERO_MAC_ADDR, MAC_ADDR_LEN)) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"\tError: MLD Addr is zero MAC (idx %d)\n", *mld_group_idx);
			return NDIS_STATUS_FAILURE;
		}

		/* 2. config conversion */

		/* 3. call core */
		grp = *mld_group_idx;
		type = BMGR_IS_ML_MLD_GRP_IDX(grp) ? BMGR_MLD_TYPE_MULTI : BMGR_MLD_TYPE_SINGLE;
		if (bss_mngr_mld_group_create(wdev, &grp,
				mld_addr, mld_param, type) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"\tEHT AP MLD create fail!!!\n");
			return NDIS_STATUS_FAILURE;
		}

		/* 4. collect info from Core */
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
			"\tCreate AP MLD Success, grp(%d)(ML:%d), type(%d), addr(%pM)\n",
			grp, BMGR_IS_ML_MLD_GRP_IDX(grp), type, mld_addr);

		/* 5. notify other modules */

		/* 6. return status and update args */
		*mld_group_idx = grp;
		*mld_type = ap_mld_type_trans(type);
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_WARN,
			"Fail: wdev_type %d, phymode 0x%x\n",
			wdev->wdev_type, wdev->PhyMode);
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

INT eht_ap_mld_destroy(struct wifi_dev *wdev, u8 mld_group_idx)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
		"===> Destroy AP MLD, grp(%d)\n", mld_group_idx);

	/* 1. sanity */
	if ((wdev->wdev_type == WDEV_TYPE_AP) && WMODE_CAP_BE(wdev->PhyMode)) {

		/* 2. config conversion */

		/* 3. call core */
		if (bss_mngr_mld_group_destroy(wdev, mld_group_idx) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"\tAP MLD destroy fail\n");
			return NDIS_STATUS_FAILURE;
		}

		/* 4. collect info from Core */
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
			"\tDestroy AP MLD Success, grp(%d)\n", mld_group_idx);

		/* 5. notify other modules */

		/* 6. return status and update args */
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_WARN,
			"Fail: wdev_type %d, phymode 0x%x\n",
			wdev->wdev_type, wdev->PhyMode);
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

INT eht_ap_mld_destroy_by_mld_addr(struct wifi_dev *wdev, u8 *mld_addr)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct query_mld_basic mld_basic = {0};

	/* 1. MAC not NULL 2. MAC not zero */
	if (!mld_addr || !NdisCmpMemory(mld_addr, ZERO_MAC_ADDR, MAC_ADDR_LEN)) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"Input error, NULL or zero MLD MAC %p\n", mld_addr);
		return NDIS_STATUS_FAILURE;
	}

	/* 3. query mld_grp_idx by mld_addr */
	COPY_MAC_ADDR(mld_basic.addr, mld_addr);
	if (bss_mngr_query_mld_basic(&mld_basic) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"MLD Lookup failed: MLD MAC (%pM)\n", mld_addr);
		return NDIS_STATUS_FAILURE;
	}

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
		"found mld group: MLD MAC(%pM) <-> idx(%d)\n",
		mld_addr, mld_basic.mld_grp_idx);
	/* 5. call eht_ap_mld_destroy(wdev, mld_grp_idx) 6. return status */
	return eht_ap_mld_destroy(wdev, mld_basic.mld_grp_idx);
}

/**
 * @brief Handler function to update attributes of an MLD
 *
 * This function handles the update flow of attribute of
 * the MLD maintained in BSS/MLD Manager.
 *
 * @param wdev BSS used to sanity
 * @param mld_group_idx The MLD index used in BSS/MLD Manager.
 * @param op_ctrl Operation control and parameters.
 */
INT eht_ap_mld_attr_set(struct wifi_dev *wdev, u8 mld_group_idx, struct eht_mld_op_ctrl *op_ctrl)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
		"===> Set attr of AP MLD, Grp(%d)\n", mld_group_idx);

	/* sanity */
	if ((wdev->wdev_type == WDEV_TYPE_AP) && WMODE_CAP_BE(wdev->PhyMode)) {

		/* call core */
		if (bss_mngr_mld_group_attr_set(mld_group_idx, op_ctrl) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"\tSet attr of AP MLD fail\n");
			return NDIS_STATUS_FAILURE;
		}

		/* collect info from Core */
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
			"\tSet attr of AP MLD Success, Grp(%d)\n", mld_group_idx);

		/* notify other modules */

		/* return status and update args */
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_WARN,
			"Fail: wdev_type %d, phymode 0x%x\n",
			wdev->wdev_type, wdev->PhyMode);
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

/**
 * @brief Handler function to update attributes of an MLD identified by MLD MAC addr.
 *
 * This function handles the update flow of attribute of
 * the MLD maintained in BSS/MLD Manager. The MLD is identified
 * by the MLD MAC address.
 *
 * @param wdev BSS used to sanity
 * @param mld_addr The MLD MAC address of the MLD.
 * @param op_ctrl Operation control and parameters.
 */
INT eht_ap_mld_attr_set_by_mld_addr(struct wifi_dev *wdev, u8 *mld_addr, struct eht_mld_op_ctrl *op_ctrl)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct query_mld_basic mld_basic = {0};

	/* 1. MAC not NULL 2. MAC not zero */
	if (!mld_addr || !NdisCmpMemory(mld_addr, ZERO_MAC_ADDR, MAC_ADDR_LEN)) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"Input error, NULL or zero MLD MAC %p\n", mld_addr);
		return NDIS_STATUS_FAILURE;
	}

	/* 3. query mld_grp_idx by mld_addr */
	COPY_MAC_ADDR(mld_basic.addr, mld_addr);
	if (bss_mngr_query_mld_basic(&mld_basic) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"MLD Lookup failed: MLD MAC (%pM)\n", mld_addr);
		return NDIS_STATUS_FAILURE;
	}

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
		"found mld group: MLD MAC(%pM) <-> idx(%d)\n",
		mld_addr, mld_basic.mld_grp_idx);
	/* 5. call eht_ap_mld_attr_set(wdev, mld_grp_idx, op_ctrl) 6. return status */
	return eht_ap_mld_attr_set(wdev, mld_basic.mld_grp_idx, op_ctrl);
}

/**
 * @brief Handler function to update single attribute (eml_mode) of an MLD.
 *
 * This function handles the update flow of eml_mode of
 * the MLD maintained in BSS/MLD Manager. The MLD is identified
 * by the MLD MAC address.
 *
 * @param wdev BSS used to sanity
 * @param mld_addr The MLD MAC address of the MLD.
 * @param eml_mode New eml_mode of MLD.
 */
INT eht_ap_mld_attr_set_eml_mode(struct wifi_dev *wdev, u8 *mld_addr, u8 eml_mode)
{
	struct eht_mld_op_ctrl op_ctrl = {0};

	op_ctrl.cfg_disconn = TRUE;
	op_ctrl.pres_bmap |= MLD_OP_PARAM_PRES_EML_MODE;
	op_ctrl.mld_param.eml_mode = eml_mode;

	return eht_ap_mld_attr_set_by_mld_addr(wdev, mld_addr, &op_ctrl);
}

/**
 * @brief Wrapper function to fill MLD attribute from BSS to struct eht_mld_param
 *
 * This function fills the MLD attibutes sourced from a selected link of MLD
 * to the eht_mld_param structure. The mld_param will be used to create MLD
 * later.
 *
 * @param wdev MLD Selected link of an MLD
 * @param mld_param Save the parameters(attributes) of an MLD.
 */
INT eht_ap_mld_fill_mld_param_from_selected_link(struct wifi_dev *wdev, struct eht_mld_param *mld_param)
{
	if (!wdev || !mld_param) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"Input error: wdev:%p, mld_param:%p\n",
			wdev, mld_param);
		return NDIS_STATUS_FAILURE;
	}

	mld_param->msd_en = wlan_config_get_med_sync_pres(wdev);
	mld_param->msd_dur = wlan_config_get_med_sync_dur(wdev);
	mld_param->msd_ofdm_ed_thr = wlan_config_get_med_sync_ofdm_ed_thr(wdev);
	mld_param->msd_max_txop = wlan_config_get_med_sync_max_txop(wdev);

	mld_param->eml_mode = wlan_config_get_emlsr_mr(wdev);
	mld_param->eml_trans_to = wlan_config_get_trans_to(wdev);
	mld_param->eml_omn = wlan_config_get_eml_omn_en(wdev);

	mld_param->t2lm_nego_supp = wlan_config_get_t2lm_nego_support(wdev);

	return NDIS_STATUS_SUCCESS;
}

INT eht_ap_mld_add_link(struct wifi_dev *wdev, u8 mld_group_idx)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	BSS_STRUCT *pMbss = wdev->func_dev;
	struct bss_mld_info mld_info = {0};

	if (!mld_group_idx) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"Input error: grp:%d\n", mld_group_idx);
		return NDIS_STATUS_FAILURE;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
		"===> %s requests to join mld_grp(%d)\n",
		RtmpOsGetNetDevName(wdev->if_dev), mld_group_idx);

	/* 1. sanity */
	if ((wdev->wdev_type == WDEV_TYPE_AP) &&
		WMODE_CAP_BE(wdev->PhyMode) &&
		WDEV_BSS_STATE(wdev) >= BSS_READY) {

		/* 2. config conversion */

		/* 3. call core */
		if (bss_mngr_mld_group_add_link(wdev, mld_group_idx) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"\tAP MLD Add Link fail!!!\n");
			return NDIS_STATUS_FAILURE;
		}

		/* 4. collect info from Core */
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
			"\tAP(%s) join MLD(%d) Success\n",
			RtmpOsGetNetDevName(wdev->if_dev), mld_group_idx);

		/* 5. notify other modules */
		pMbss->mld_grp_idx = mld_group_idx;
		wifi_sys_bss_query_mld(wdev, &mld_info);

		bss_mngr_ie_update(wdev);

		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
			"enable beacon (%d)\n", wdev->bcn_buf.bBcnSntReq);

		UpdateBeaconHandler_BPCC(pAd, wdev,
			BCN_REASON(BCN_UPDATE_ENABLE_TX), BCN_BPCC_ADD_LINK, TRUE);

		bss_mngr_mld_sync_ml_probe_rsp(wdev);

		/* hostapd part */
#ifdef RT_CFG80211_SUPPORT
		mtk_cfg80211_send_bss_ml_event(wdev, CFG80211_ML_EVENT_ADDLINK);
#endif /* RT_CFG80211_SUPPORT */

		/* WM/WA Part */
		MtUniCmdMldLinkOp(pAd, wdev);

		/* 6. return status and update args */
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"Error: wdev_type %d, phymode 0x%x, state %d\n",
			wdev->wdev_type, wdev->PhyMode, WDEV_BSS_STATE(wdev));
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

INT eht_ap_mld_add_link_by_mld_addr(struct wifi_dev *wdev, u8 *mld_addr)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct query_mld_basic mld_basic = {0};

	/* 1. MAC not NULL 2. MAC not zero */
	if (!mld_addr || !NdisCmpMemory(mld_addr, ZERO_MAC_ADDR, MAC_ADDR_LEN)) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"Input error, NULL or zero MLD MAC %p\n", mld_addr);
		return NDIS_STATUS_FAILURE;
	}

	/* 3. query mld_grp_idx by mld_addr */
	COPY_MAC_ADDR(mld_basic.addr, mld_addr);
	if (bss_mngr_query_mld_basic(&mld_basic) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"MLD Lookup failed: MLD MAC (%pM)\n", mld_addr);
		return NDIS_STATUS_FAILURE;
	}

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
		"found mld group: MLD MAC(%pM) <-> idx(%d)\n",
		mld_addr, mld_basic.mld_grp_idx);
	/* 5. call eht_ap_mld_add_link(wdev, mld_grp_idx) 6. return status */
	return eht_ap_mld_add_link(wdev, mld_basic.mld_grp_idx);
}

INT eht_ap_mld_del_link(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	BSS_STRUCT *pMbss = wdev->func_dev;
	struct bss_mld_info mld_info = {0};

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
		"===> %s requests to leave mld_grp\n",
		RtmpOsGetNetDevName(wdev->if_dev));

	/* 1. sanity */
	if ((wdev->wdev_type == WDEV_TYPE_AP) &&
		WMODE_CAP_BE(wdev->PhyMode) &&
		WDEV_BSS_STATE(wdev) >= BSS_READY) {

		/* 2. config conversion */

		/* 3. call core */
		if (bss_mngr_mld_group_del_link(wdev) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"\tAP MLD Del Link fail!!!\n");
			return NDIS_STATUS_FAILURE;
		}

		/* 4. collect info from Core */
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
			"\tAP(%s) leave MLD Success\n",
			RtmpOsGetNetDevName(wdev->if_dev));

		/* 5. notify other modules */
		/* 0 means non-MLO */
		pMbss->mld_grp_idx = 0;
		wifi_sys_bss_query_mld(wdev, &mld_info);

		bss_mngr_ie_update(wdev);
		bss_mngr_sync_bcn_update(wdev);
		bss_mngr_mld_sync_ml_probe_rsp(wdev);

		/* hostapd part */
#ifdef RT_CFG80211_SUPPORT
		mtk_cfg80211_send_bss_ml_event(wdev, CFG80211_ML_EVENT_DELLINK);
#endif /* RT_CFG80211_SUPPORT */

		/* WM/WA Part */
		MtUniCmdMldLinkOp(pAd, wdev);

		/* 6. return status and update args */
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"Error: wdev_type %d, phymode 0x%x, state %d\n",
			wdev->wdev_type, wdev->PhyMode, WDEV_BSS_STATE(wdev));
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

INT eht_ap_add_to_single_link_mld(struct wifi_dev *wdev)
{
	/* 0 indicate an MLD w/ single-link MLD type */
	u8 mld_type, grp = 0;
	struct eht_mld_param mld_param = {0};
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"wdev(%s), bssid(%pM)\n", RtmpOsGetNetDevName(wdev->if_dev), wdev->bssid);

	eht_ap_mld_fill_default_mld_param(&mld_param);

	if (eht_ap_mld_create(wdev, &grp,
			wdev->bssid, &mld_param, &mld_type) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"AP MLD Create Failed! (grp:%d, addr:%pM)\n",
			grp, wdev->bssid);
		return NDIS_STATUS_FAILURE;
	}

	if (eht_ap_mld_add_link(wdev, grp) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"AP MLD Add Link fail! (grp=%d)\n", grp);
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

INT eht_ap_mld_link_transfer(struct wifi_dev *wdev, u8 mld_group_idx)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	BSS_STRUCT *pMbss = wdev->func_dev;
	u8 original_grp = pMbss->mld_grp_idx;
	struct query_mld_ap_basic bss_mld_info_basic = {0};
	struct reconfig_tmr_to_t reconfig_to_info = {0};

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"===> %s requests, source grp %d, target grp %d\n",
		RtmpOsGetNetDevName(wdev->if_dev), original_grp, mld_group_idx);

	/* sanity */
	if ((wdev->wdev_type == WDEV_TYPE_AP) &&
		WMODE_CAP_BE(wdev->PhyMode) &&
		WDEV_BSS_STATE(wdev) >= BSS_READY) {

		/* self MLD (AP MLD) */
		if (bss_mngr_query_mld_ap_basic(wdev, &bss_mld_info_basic) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
				"Invalid query\n");
			return NDIS_STATUS_FAILURE;
		}

		COPY_MAC_ADDR(reconfig_to_info.mld_addr, bss_mld_info_basic.addr);
		reconfig_to_info.to_link_id_bmap |= BIT(bss_mld_info_basic.link_id);
		reconfig_to_info.args.reconfig_mode = MLD_RECONFIG_MODE_INSTANT;

		if (eht_ap_mld_exec_link_reconfiguration(&reconfig_to_info) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
				"\tAP (%s) Del Link fail\n", RtmpOsGetNetDevName(wdev->if_dev));
			return NDIS_STATUS_FAILURE;
		}

		if (eht_ap_mld_destroy(wdev, original_grp) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_NOTICE,
				"\tAP MLD not Destroy (grp=%d)\n", original_grp);
		}

		if (eht_ap_mld_add_link(wdev, mld_group_idx) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
				"\tAP (%s) Add Link fail (grp=%d), join SL\n",
				RtmpOsGetNetDevName(wdev->if_dev), mld_group_idx);
			eht_ap_add_to_single_link_mld(wdev);
			// return NDIS_STATUS_FAILURE;
		}

		/* collect result */
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_NOTICE,
			"\tAP(%s) MLD op Success\n",
			RtmpOsGetNetDevName(wdev->if_dev));
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
			"Error: wdev_type %d, phymode 0x%x, state %d\n",
			wdev->wdev_type, wdev->PhyMode, WDEV_BSS_STATE(wdev));
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

INT eht_ap_mld_trig_link_reconfiguration(struct wifi_dev *wdev, u16 reconfig_to)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct reconfig_set_tmr_t reconfig_info = {0};
	struct mld_reconfig_ie_build_t reconf_ie_info = {0};
	struct mld_reconfig_ie_query_t reconf_query = {0};
	struct query_mld_ap_basic bss_mld_info_basic = {0};
	u8 temp_buf[MAX_RECONFIG_ML_IE_LEN];
	int ret;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_NOTICE,
		"===> %s trigger MLD reconfiguration (remove link)\n",
		RtmpOsGetNetDevName(wdev->if_dev));

	if ((wdev->wdev_type == WDEV_TYPE_AP) &&
		WMODE_CAP_BE(wdev->PhyMode) &&
		WDEV_BSS_STATE(wdev) >= BSS_READY) {

		/* Part 0. sanity */
		/* TODO, multiple MLD reconfiguration */


		/* Part 1. init the reconfiguration timer */
		/* 1. query which link will be removed */
		if (bss_mngr_query_mld_ap_basic(wdev, &bss_mld_info_basic) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
				"Invalid query\n");
			ret = NDIS_STATUS_FAILURE;
			goto err;
		}

		reconf_query.f_buf = temp_buf;
		if (bss_mngr_mld_reconfig_ie_query(bss_mld_info_basic.mld_grp_idx, &reconf_query) > 0) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
				"Fail: Support one link reconfiguration at the same time\n");
			ret = NDIS_STATUS_FAILURE;
			goto err;
		}

		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_NOTICE,
			"Link id %d to be removed from %pM\n",
			bss_mld_info_basic.link_id, bss_mld_info_basic.addr);

		/* 2. tx UNI_CMD_MLD_RECONFIG_TMR_T to FW */
		COPY_MAC_ADDR(reconfig_info.mld_addr, bss_mld_info_basic.addr);
		reconfig_info.fw_mld_idx = mld_grp_2_fw_mld_id(bss_mld_info_basic.mld_grp_idx);
		reconfig_info.mld_flag = 0;
		reconfig_info.tmr_link_id_bmap |= BIT(bss_mld_info_basic.link_id);
		reconfig_info.fw_bss_idx[bss_mld_info_basic.link_id] = wdev->bss_info_argument.ucBssIndex;
		reconfig_info.num_seconds = reconfig_to;
		ret = HW_SET_RECONFIG_TMR(wdev, &reconfig_info);
		if (ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
				"Error: HW_SET_RECONFIG_TMR failed\n");
			goto err;
		}

		/* Part 2. update beacon and in-band command for offset */
		/* 1. build Reconfiguration ML IE for each "MLD"
		 * (Beacons in each link use the same content) */
		reconf_ie_info.rm_links[bss_mld_info_basic.link_id] = TRUE;
		reconf_ie_info.tmr[bss_mld_info_basic.link_id] = reconfig_to;

		if (bcn_bpcc_op_lock(pAd, wdev, TRUE, BCN_BPCC_RECONFIG) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
				"Error: bcn_bpcc_op_lock fail\n");
			goto err;
		}

		if (bss_mngr_mld_reconfig_ie_build(
				bss_mld_info_basic.mld_grp_idx, &reconf_ie_info) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
				"Error: build Reconfiguration multi-link IE failed\n");
			bcn_bpcc_op_unlock(pAd, wdev, TRUE);
			ret = NDIS_STATUS_FAILURE;
			goto err;
		}

		/* 2. Set Critical Update Flag and do Bcn Update for each link in MLD */
		UpdateBeaconHandler_BPCC(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG), BCN_BPCC_RECONFIG, TRUE);

		/* 3. MBSS reconfiuration stat change (IDLE -> COUNTDOWN)*/
		MBSS_Reconfig_State_transition(pAd, (BSS_STRUCT *)wdev->func_dev, BSS_RECONFIG_COUNTDOWN_STAGE, reconfig_to*1000);

		ret = NDIS_STATUS_SUCCESS;
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
			"Error: wdev_type %d, phymode 0x%x, state %d\n",
			wdev->wdev_type, wdev->PhyMode, WDEV_BSS_STATE(wdev));
		ret = NDIS_STATUS_FAILURE;
	}

err:
	return ret;
}

INT eht_ap_mld_exec_link_reconfiguration(struct reconfig_tmr_to_t *reconfig_to_info)
{
	struct wifi_dev *wdev = NULL;
	struct _RTMP_ADAPTER *pAd;
	struct query_mld_basic mld_basic = {0};
	INT ret = NDIS_STATUS_SUCCESS;
	u8 link_id = 0;
	struct reconfig_rm_link_req_t req = {0};
	BSS_STRUCT *pMbss[BSS_MNGR_MAX_BAND_NUM] = {0};

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_INFO,
		"Addr=%pM, fw_mld_idx=%d, timeout_bmap=0x%x\n",
		reconfig_to_info->mld_addr,
		reconfig_to_info->fw_mld_idx, reconfig_to_info->to_link_id_bmap);

	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		if (reconfig_to_info->to_link_id_bmap & BIT(link_id)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_INFO,
				"Link %d, fw_bss_idx %d\n",
				link_id, reconfig_to_info->fw_bss_idx[link_id]);
		}
	}

	COPY_MAC_ADDR(mld_basic.addr, reconfig_to_info->mld_addr);
	if (bss_mngr_query_mld_basic(&mld_basic)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
			"MLD Lookup failed: MLD MAC (%pM)\n", reconfig_to_info->mld_addr);
		return NDIS_STATUS_FAILURE;
	}

	switch (reconfig_to_info->args.reconfig_mode) {
	case MLD_RECONFIG_MODE_COUNTDOWN:
		/* TODO, handle Per-link clean (common part also) */
		bss_mngr_mld_reconfig_ie_clean(mld_basic.mld_grp_idx);

		/* handle non-AP MLD */
		bss_mngr_mld_reconfig_peer_mld(mld_basic.mld_grp_idx,
			reconfig_to_info->to_link_id_bmap);

		/* pause tx of remove link */
		for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
			if (reconfig_to_info->to_link_id_bmap & BIT(link_id)) {
				wdev = bss_mngr_query_group_wdev_by_link(mld_basic.mld_grp_idx, link_id);
				if (wdev)
					pMbss[link_id] = (BSS_STRUCT *)wdev->func_dev;
				break;
			}
		}

		if (!wdev) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
				"no available wdev\n");
			return NDIS_STATUS_FAILURE;
		}

		COPY_MAC_ADDR(req.mld_addr, reconfig_to_info->mld_addr);
		req.fw_mld_idx = reconfig_to_info->fw_mld_idx;
		req.flag = 0;
		req.rm_link_id_bitmap = reconfig_to_info->to_link_id_bmap;
		for (link_id = 0; link_id < MLD_LINK_MAX; link_id++)
			req.link_id_bss_info_idx[link_id] = reconfig_to_info->fw_bss_idx[link_id];
		HW_REQ_RECONFIG_RM_LINK(wdev, &req);

		break;

	case MLD_RECONFIG_MODE_INSTANT:
		/* Disassociate ML-capable STAs */
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_NOTICE,
			"Instant mode, disassoc non-AP MLDs (link_bmap=0x%x)\n",
			reconfig_to_info->to_link_id_bmap);
		for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
			if (reconfig_to_info->to_link_id_bmap & BIT(link_id)) {
				struct _MAC_TABLE_ENTRY *pEntry;
				INT i;

				wdev = bss_mngr_query_group_wdev_by_link(mld_basic.mld_grp_idx, link_id);

				if (!wdev) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
						"Error: wdev Lookup failed: mld=%d, link=%d\n",
						mld_basic.mld_grp_idx, link_id);
					continue;
				}
				pMbss[link_id] = (BSS_STRUCT *)wdev->func_dev;
				pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
				for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
					pEntry = entry_get(pAd, i);

					if (pEntry && IS_ENTRY_CLIENT(pEntry) &&
						pEntry->wdev == wdev && IS_ENTRY_MLO(pEntry))
						APMlmeKickOutSta(pAd, pEntry->Addr, pEntry->wcid,
							REASON_DISASSOC_INACTIVE);
				}
			}
		}

		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
			"Unknown MLD Reconfig mode: %d\n", reconfig_to_info->args.reconfig_mode);
		return NDIS_STATUS_FAILURE;
	}

	/* handle AP MLD*/
	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		if (reconfig_to_info->to_link_id_bmap & BIT(link_id)) {
			wdev = bss_mngr_query_group_wdev_by_link(mld_basic.mld_grp_idx, link_id);

			if (!wdev) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
					"Error: wdev Lookup failed: mld=%d, link=%d\n",
					mld_basic.mld_grp_idx, link_id);
				continue;
			}

			ret = eht_ap_mld_del_link(wdev);
			if (ret != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
					"EHT AP (%s) Del Link fail\n", RtmpOsGetNetDevName(wdev->if_dev));
			} else {
				pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
					"disable beacon (%d)\n", wdev->bcn_buf.bBcnSntReq);
				UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_DISABLE_TX));
			}
		}
	}

	switch (reconfig_to_info->args.reconfig_mode) {
	case MLD_RECONFIG_MODE_COUNTDOWN:
		for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
			if (pMbss[link_id] == NULL)
				continue;
			pAd = (struct _RTMP_ADAPTER *)pMbss[link_id]->wdev.sys_handle;
			MBSS_Reconfig_State_transition(pAd, pMbss[link_id], BSS_RECONFIG_COUNTDOWN_STAGE_END, 2000);
		}
		break;
	case MLD_RECONFIG_MODE_INSTANT:
		for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
			if (pMbss[link_id] == NULL)
				continue;
			pAd = (struct _RTMP_ADAPTER *)pMbss[link_id]->wdev.sys_handle;
			MBSS_Reconfig_State_transition(pAd, pMbss[link_id], BSS_RECONFIG_COUNTDOWN_STAGE_END, 2000);
		}
		break;
	}

	return ret;
}

static BOOLEAN get_value_by_key(RTMP_STRING *key, RTMP_STRING *dest, UINT32 dest_size, RTMP_STRING *buffer)
{
	RTMP_STRING temp_buf1[100] = {0}, temp_buf2[100] = {0};
	RTMP_STRING *start_ptr, *end_ptr;
	RTMP_STRING *ptr;
	INT len;

	// tmpbuf1 = "key="
	strlcpy(temp_buf1, key, strlen(key) + 1);
	strncat(temp_buf1, "=", strlen("="));

	// search buffer, get start_ptr
	start_ptr = rtstrstr(buffer, temp_buf1);
	if (!start_ptr) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_DEBUG,
			"Error: start_ptr is null\n");
		return FALSE;
	}

	// find next , in buffer and get end_ptr
	end_ptr = rtstrstr(start_ptr, ",");
	if (!end_ptr)
		end_ptr = start_ptr + strlen(start_ptr);

	if (end_ptr < start_ptr) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"Error: end_ptr < start_ptr\n");
		return FALSE;
	}

	if (end_ptr - start_ptr >= 100) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"Error: ptr is too long\n");
		return FALSE;
	}

	// get tmpbuf2
	NdisMoveMemory(temp_buf2, start_ptr, end_ptr - start_ptr);
	temp_buf2[end_ptr - start_ptr] = '\0';

	// find = and get the string after =
	start_ptr = rtstrstr(temp_buf2, "=");
	if (!start_ptr)
		return FALSE;

	// remove =
	ptr = (start_ptr + 1);

	// get result
	len = strlen(start_ptr);
	NdisZeroMemory(dest, dest_size);
	strlcpy(dest, ptr, dest_size);

	return TRUE;
}

static BOOLEAN parse_mld_create_args(RTMP_STRING *arg, struct eht_mld_op_ctrl *op_ctrl, u32 *pres)
{
	RTMP_STRING *attrs = arg;
	RTMP_STRING *tmpbuf = NULL;
	u32 pres_bmap = 0;
	BOOLEAN ret = FALSE;

	if (!arg || !strlen(arg) || !op_ctrl || !pres) {
		MTWF_PRINT("Err: group and addr are required.\n");
		return FALSE;
	}

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (!tmpbuf)
		return FALSE;

	if (get_value_by_key("group", tmpbuf, 100, attrs)) {
		u8 mld_grp;

		if (!kstrtou8(tmpbuf, 10, &mld_grp)) {
			if (mld_grp < BMGR_MAX_MLD_GRP_CNT) {
				op_ctrl->mld_grp = mld_grp;

				pres_bmap |= MLD_OP_PARAM_PRES_GROUP_IDX;

				MTWF_PRINT("mld_grp_idx=%d\n", op_ctrl->mld_grp);
			} else {
				MTWF_PRINT("INVALID mld_grp_idx: %d (larger than max=%d)\n",
					mld_grp, BMGR_MAX_MLD_GRP_CNT);
				goto err;
			}
		} else
			goto err;
	} else {
		MTWF_PRINT("Err: group is required.\n");
		goto err;
	}

	if (get_value_by_key("addr", tmpbuf, 100, attrs)) {
		u16 i, mac_len;
		u8 Address[MAC_ADDR_LEN];

		/* Mac address acceptable format 01:02:03:04:05:06 (len=17) */
		mac_len = strlen(tmpbuf);
		if ((mac_len != 17) || !strcmp(tmpbuf, ZERO_MAC_ADDR)) {
			MTWF_PRINT("invalid addr (len=%d)\n", mac_len);
			goto err;
		} else {
			for (i = 0; i < MAC_ADDR_LEN; i++)
				AtoH((tmpbuf + (i*3)), &Address[i], 1);

			/* MLD Mac address which registers to BSS Manager */
			COPY_MAC_ADDR(op_ctrl->mld_addr, Address);

			pres_bmap |= MLD_OP_PARAM_PRES_ADDR;

			MTWF_PRINT("mld_addr=%pM\n", op_ctrl->mld_addr);
		}
	} else {
		MTWF_PRINT("Err: addr is required.\n");
		goto err;
	}

	if (get_value_by_key("eml_mode", tmpbuf, 100, attrs)) {
		u8 eml_mode;

		if (!kstrtou8(tmpbuf, 10, &eml_mode)) {
			op_ctrl->mld_param.eml_mode = eml_mode;

			pres_bmap |= MLD_OP_PARAM_PRES_EML_MODE;

			MTWF_PRINT("eml_mode=%d\n", op_ctrl->mld_param.eml_mode);
		} else {
			MTWF_PRINT("Err: eml_mode is invalid.\n");
			goto err;
		}
	} else {
		/* default: EMLSR */
		pres_bmap |= MLD_OP_PARAM_PRES_EML_MODE;

		op_ctrl->mld_param.eml_mode = EMLSR;
	}

	if (get_value_by_key("eml_trans_to", tmpbuf, 100, attrs)) {
		u8 eml_trans_to;

		if (!kstrtou8(tmpbuf, 10, &eml_trans_to)) {
			op_ctrl->mld_param.eml_trans_to = eml_trans_to;

			pres_bmap |= MLD_OP_PARAM_PRES_EML_TRANS_TO;

			MTWF_PRINT("eml_trans_to=%d\n", op_ctrl->mld_param.eml_trans_to);
		} else {
			MTWF_PRINT("Err: eml_trans_to is invalid.\n");
			goto err;
		}
	}

	if (get_value_by_key("eml_omn", tmpbuf, 100, attrs)) {
		u8 eml_omn;

		if (!kstrtou8(tmpbuf, 10, &eml_omn)) {
			op_ctrl->mld_param.eml_omn = eml_omn;

			pres_bmap |= MLD_OP_PARAM_PRES_EML_OMN;

			MTWF_PRINT("eml_omn=%d\n", op_ctrl->mld_param.eml_omn);
		} else {
			MTWF_PRINT("Err: eml_omn is invalid.\n");
			goto err;
		}
	}

	if (get_value_by_key("t2lm_nego", tmpbuf, 100, attrs)) {
		u8 t2lm_nego;

		if (!kstrtou8(tmpbuf, 10, &t2lm_nego)) {
			op_ctrl->mld_param.t2lm_nego_supp = t2lm_nego;

			pres_bmap |= MLD_OP_PARAM_PRES_T2LM_NEGO;

			MTWF_PRINT("t2lm_nego=%d\n", op_ctrl->mld_param.t2lm_nego_supp);
		} else {
			MTWF_PRINT("Err: t2lm_nego is invalid.\n");
			goto err;
		}
	} else {
		/* default: 1 */
		pres_bmap |= MLD_OP_PARAM_PRES_T2LM_NEGO;

		op_ctrl->mld_param.t2lm_nego_supp = 1;
	}

	MTWF_PRINT("presence: 0x%x\n", pres_bmap);
	*pres = pres_bmap;
	ret = TRUE;

err:
	if (ret == FALSE)
		NdisZeroMemory(op_ctrl, sizeof(struct eht_mld_op_ctrl));
	if (tmpbuf)
		os_free_mem(tmpbuf);
	return ret;
}

static BOOLEAN parse_mld_destroy_args(RTMP_STRING *arg, struct eht_mld_op_ctrl *op_ctrl, u32 *pres)
{
	RTMP_STRING *attrs = arg;
	RTMP_STRING *tmpbuf = NULL;
	u32 pres_bmap = 0;
	BOOLEAN ret = FALSE;

	if (!arg || !strlen(arg) || !op_ctrl || !pres) {
		MTWF_PRINT("Err: one of group and addr is required.\n");
		return FALSE;
	}

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (!tmpbuf)
		return FALSE;

	if (get_value_by_key("group", tmpbuf, 100, attrs)) {
		u8 mld_grp;

		if (!kstrtou8(tmpbuf, 10, &mld_grp)) {
			if (mld_grp < BMGR_MAX_MLD_GRP_CNT) {
				op_ctrl->mld_grp = mld_grp;

				pres_bmap |= MLD_OP_PARAM_PRES_GROUP_IDX;

				MTWF_PRINT("mld_grp_idx=%d\n", op_ctrl->mld_grp);
			} else {
				MTWF_PRINT("INVALID mld_grp_idx: %d (larger than max=%d)\n",
					mld_grp, BMGR_MAX_MLD_GRP_CNT);
				goto err;
			}
		} else
			goto err;
	}

	if (get_value_by_key("addr", tmpbuf, 100, attrs)) {
		u16 i, mac_len;
		u8 Address[MAC_ADDR_LEN];

		/* Mac address acceptable format 01:02:03:04:05:06 (len=17) */
		mac_len = strlen(tmpbuf);
		if ((mac_len != 17) || !strcmp(tmpbuf, ZERO_MAC_ADDR)) {
			MTWF_PRINT("invalid addr (len=%d)\n", mac_len);
			goto err;
		} else {
			for (i = 0; i < MAC_ADDR_LEN; i++)
				AtoH((tmpbuf + (i*3)), &Address[i], 1);

			/* MLD Mac address which registers to BSS Manager */
			COPY_MAC_ADDR(op_ctrl->mld_addr, Address);

			pres_bmap |= MLD_OP_PARAM_PRES_ADDR;

			MTWF_PRINT("mld_addr=%pM\n", op_ctrl->mld_addr);
		}
	}

	MTWF_PRINT("presence: 0x%x\n", pres_bmap);
	*pres = pres_bmap;
	ret = TRUE;

err:
	if (ret == FALSE)
		NdisZeroMemory(op_ctrl, sizeof(struct eht_mld_op_ctrl));
	if (tmpbuf)
		os_free_mem(tmpbuf);
	return ret;
}

static BOOLEAN parse_mld_add_link_args(RTMP_STRING *arg, struct eht_mld_op_ctrl *op_ctrl, u32 *pres)
{
	RTMP_STRING *attrs = arg;
	RTMP_STRING *tmpbuf = NULL;
	u32 pres_bmap = 0;
	BOOLEAN ret = FALSE;

	if (!arg || !strlen(arg) || !op_ctrl || !pres) {
		MTWF_PRINT("Err: one of group and addr is required.\n");
		return FALSE;
	}

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (!tmpbuf)
		return FALSE;

	if (get_value_by_key("group", tmpbuf, 100, attrs)) {
		u8 mld_grp;

		if (!kstrtou8(tmpbuf, 10, &mld_grp)) {
			if (mld_grp < BMGR_MAX_MLD_GRP_CNT) {
				op_ctrl->mld_grp = mld_grp;

				pres_bmap |= MLD_OP_PARAM_PRES_GROUP_IDX;

				MTWF_PRINT("mld_grp_idx=%d\n", op_ctrl->mld_grp);
			} else {
				MTWF_PRINT("INVALID mld_grp_idx: %d (larger than max=%d)\n",
					mld_grp, BMGR_MAX_MLD_GRP_CNT);
				goto err;
			}
		} else
			goto err;
	}

	if (get_value_by_key("addr", tmpbuf, 100, attrs)) {
		u16 i, mac_len;
		u8 Address[MAC_ADDR_LEN];

		/* Mac address acceptable format 01:02:03:04:05:06 (len=17) */
		mac_len = strlen(tmpbuf);
		if ((mac_len != 17) || !strcmp(tmpbuf, ZERO_MAC_ADDR)) {
			MTWF_PRINT("invalid addr (len=%d)\n", mac_len);
			goto err;
		} else {
			for (i = 0; i < MAC_ADDR_LEN; i++)
				AtoH((tmpbuf + (i*3)), &Address[i], 1);

			/* MLD Mac address which registers to BSS Manager */
			COPY_MAC_ADDR(op_ctrl->mld_addr, Address);

			pres_bmap |= MLD_OP_PARAM_PRES_ADDR;

			MTWF_PRINT("mld_addr=%pM\n", op_ctrl->mld_addr);
		}
	}

	MTWF_PRINT("presence: 0x%x\n", pres_bmap);
	*pres = pres_bmap;
	ret = TRUE;

err:
	if (ret == FALSE)
		NdisZeroMemory(op_ctrl, sizeof(struct eht_mld_op_ctrl));
	if (tmpbuf)
		os_free_mem(tmpbuf);
	return ret;
}

static BOOLEAN parse_mld_del_link_args(RTMP_STRING *arg, struct eht_mld_op_ctrl *op_ctrl, u32 *pres)
{
	RTMP_STRING *attrs = arg;
	RTMP_STRING *tmpbuf = NULL;
	u32 pres_bmap = 0;
	BOOLEAN ret = FALSE;

	if (!op_ctrl || !pres)
		return FALSE;

	/* all arguments are optional */
	if (!arg || !strlen(arg))
		return TRUE;

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (!tmpbuf)
		return FALSE;

	if (get_value_by_key("reconfig_to", tmpbuf, 100, attrs)) {
		u16 reconfig_to;

		if (!kstrtou16(tmpbuf, 10, &reconfig_to)) {
			op_ctrl->reconfig_to = reconfig_to;

			pres_bmap |= MLD_OP_PARAM_PRES_RECONFIG_TO;

			MTWF_PRINT("reconfig_to=%d\n", op_ctrl->reconfig_to);
		} else {
			MTWF_PRINT("Err: reconfig_to is invalid.\n");
			goto err;
		}
	}

	MTWF_PRINT("presence: 0x%x\n", pres_bmap);
	*pres = pres_bmap;
	ret = TRUE;

err:
	if (ret == FALSE)
		NdisZeroMemory(op_ctrl, sizeof(struct eht_mld_op_ctrl));
	if (tmpbuf)
		os_free_mem(tmpbuf);
	return ret;
}

static BOOLEAN parse_mld_link_transfer_args(RTMP_STRING *arg, struct eht_mld_op_ctrl *op_ctrl, u32 *pres)
{
	RTMP_STRING *attrs = arg;
	RTMP_STRING *tmpbuf = NULL;
	u32 pres_bmap = 0;
	BOOLEAN ret = FALSE;

	if (!arg || !strlen(arg) || !op_ctrl || !pres) {
		MTWF_PRINT("Err: one of group and addr is required.\n");
		return FALSE;
	}

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (!tmpbuf)
		return FALSE;

	if (get_value_by_key("group", tmpbuf, 100, attrs)) {
		u8 mld_grp;

		if (!kstrtou8(tmpbuf, 10, &mld_grp)) {
			if (mld_grp < BMGR_MAX_MLD_GRP_CNT) {
				op_ctrl->mld_grp = mld_grp;

				pres_bmap |= MLD_OP_PARAM_PRES_GROUP_IDX;

				MTWF_PRINT("mld_grp_idx=%d\n", op_ctrl->mld_grp);
			} else {
				MTWF_PRINT("INVALID mld_grp_idx: %d (larger than max=%d)\n",
					mld_grp, BMGR_MAX_MLD_GRP_CNT);
				goto err;
			}
		} else
			goto err;
	}

	if (get_value_by_key("addr", tmpbuf, 100, attrs)) {
		u16 i, mac_len;
		u8 Address[MAC_ADDR_LEN];

		/* Mac address acceptable format 01:02:03:04:05:06 (len=17) */
		mac_len = strlen(tmpbuf);
		if ((mac_len != 17) || !strcmp(tmpbuf, ZERO_MAC_ADDR)) {
			MTWF_PRINT("invalid addr (len=%d)\n", mac_len);
			goto err;
		} else {
			for (i = 0; i < MAC_ADDR_LEN; i++)
				AtoH((tmpbuf + (i*3)), &Address[i], 1);

			/* MLD Mac address which registers to BSS Manager */
			COPY_MAC_ADDR(op_ctrl->mld_addr, Address);

			pres_bmap |= MLD_OP_PARAM_PRES_ADDR;

			MTWF_PRINT("mld_addr=%pM\n", op_ctrl->mld_addr);
		}
	}

	MTWF_PRINT("presence: 0x%x\n", pres_bmap);
	*pres = pres_bmap;
	ret = TRUE;

err:
	if (ret == FALSE)
		NdisZeroMemory(op_ctrl, sizeof(struct eht_mld_op_ctrl));
	if (tmpbuf)
		os_free_mem(tmpbuf);
	return ret;
}

static BOOLEAN parse_mld_attr_set_args(RTMP_STRING *arg, struct eht_mld_op_ctrl *op_ctrl, u32 *pres)
{
	RTMP_STRING *attrs = arg;
	RTMP_STRING *tmpbuf = NULL;
	u32 pres_bmap = 0;
	BOOLEAN ret = FALSE;

	if (!arg || !strlen(arg) || !op_ctrl || !pres) {
		MTWF_PRINT("Err: one of group and addr is required.\n");
		return FALSE;
	}

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (!tmpbuf)
		return FALSE;

	if (get_value_by_key("group", tmpbuf, 100, attrs)) {
		u8 mld_grp;

		if (!kstrtou8(tmpbuf, 10, &mld_grp)) {
			if (mld_grp < BMGR_MAX_MLD_GRP_CNT) {
				op_ctrl->mld_grp = mld_grp;

				pres_bmap |= MLD_OP_PARAM_PRES_GROUP_IDX;

				MTWF_PRINT("mld_grp_idx=%d\n", op_ctrl->mld_grp);
			} else {
				MTWF_PRINT("INVALID mld_grp_idx: %d (larger than max=%d)\n",
					mld_grp, BMGR_MAX_MLD_GRP_CNT);
				goto err;
			}
		} else
			goto err;
	}

	if (get_value_by_key("addr", tmpbuf, 100, attrs)) {
		u16 i, mac_len;
		u8 Address[MAC_ADDR_LEN];

		/* Mac address acceptable format 01:02:03:04:05:06 (len=17) */
		mac_len = strlen(tmpbuf);
		if ((mac_len != 17) || !strcmp(tmpbuf, ZERO_MAC_ADDR)) {
			MTWF_PRINT("invalid addr (len=%d)\n", mac_len);
			goto err;
		} else {
			for (i = 0; i < MAC_ADDR_LEN; i++)
				AtoH((tmpbuf + (i*3)), &Address[i], 1);

			/* MLD Mac address which registers to BSS Manager */
			COPY_MAC_ADDR(op_ctrl->mld_addr, Address);

			pres_bmap |= MLD_OP_PARAM_PRES_ADDR;

			MTWF_PRINT("mld_addr=%pM\n", op_ctrl->mld_addr);
		}
	}

	if (get_value_by_key("cfg_disconn", tmpbuf, 100, attrs)) {
		u8 cfg_disconn;

		if (!kstrtou8(tmpbuf, 10, &cfg_disconn)) {
			op_ctrl->cfg_disconn = cfg_disconn;

			MTWF_PRINT("cfg_disconn=%d\n", op_ctrl->cfg_disconn);
		} else {
			MTWF_PRINT("Err: cfg_disconn is invalid.\n");
			goto err;
		}
	} else {
		/* default TRUE */
		op_ctrl->cfg_disconn = TRUE;
	}

	if (get_value_by_key("eml_mode", tmpbuf, 100, attrs)) {
		u8 eml_mode;

		if (!kstrtou8(tmpbuf, 10, &eml_mode)) {
			op_ctrl->mld_param.eml_mode = eml_mode;

			pres_bmap |= MLD_OP_PARAM_PRES_EML_MODE;

			MTWF_PRINT("eml_mode=%d\n", op_ctrl->mld_param.eml_mode);
		} else {
			MTWF_PRINT("Err: eml_mode is invalid.\n");
			goto err;
		}
	}

	if (get_value_by_key("eml_trans_to", tmpbuf, 100, attrs)) {
		u8 eml_trans_to;

		if (!kstrtou8(tmpbuf, 10, &eml_trans_to)) {
			op_ctrl->mld_param.eml_trans_to = eml_trans_to;

			pres_bmap |= MLD_OP_PARAM_PRES_EML_TRANS_TO;

			MTWF_PRINT("eml_trans_to=%d\n", op_ctrl->mld_param.eml_trans_to);
		} else {
			MTWF_PRINT("Err: eml_trans_to is invalid.\n");
			goto err;
		}
	}

	if (get_value_by_key("eml_omn", tmpbuf, 100, attrs)) {
		u8 eml_omn;

		if (!kstrtou8(tmpbuf, 10, &eml_omn)) {
			op_ctrl->mld_param.eml_omn = eml_omn;

			pres_bmap |= MLD_OP_PARAM_PRES_EML_OMN;

			MTWF_PRINT("eml_omn=%d\n", op_ctrl->mld_param.eml_omn);
		} else {
			MTWF_PRINT("Err: eml_omn is invalid.\n");
			goto err;
		}
	}

	if (get_value_by_key("t2lm_nego", tmpbuf, 100, attrs)) {
		u8 t2lm_nego;

		if (!kstrtou8(tmpbuf, 10, &t2lm_nego)) {
			op_ctrl->mld_param.t2lm_nego_supp = t2lm_nego;

			pres_bmap |= MLD_OP_PARAM_PRES_T2LM_NEGO;

			MTWF_PRINT("t2lm_nego=%d\n", op_ctrl->mld_param.t2lm_nego_supp);
		} else {
			MTWF_PRINT("Err: t2lm_nego is invalid.\n");
			goto err;
		}
	}

	MTWF_PRINT("presence: 0x%x\n", pres_bmap);
	*pres = pres_bmap;
	ret = TRUE;

err:
	if (ret == FALSE)
		NdisZeroMemory(op_ctrl, sizeof(struct eht_mld_op_ctrl));
	if (tmpbuf)
		os_free_mem(tmpbuf);
	return ret;
}

INT set_ap_mld_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,
		pObj->ioctl_if, pObj->ioctl_if_type);
	RTMP_STRING *mld_op, *op_attr;
	struct eht_mld_op_ctrl op_ctrl = {0};
	u32 pres_bmap = 0;

	if (arg == NULL || strlen(arg) == 0)
		goto err;

	if (!wdev) {
		MTWF_PRINT("err: wdev not found\n");
		return FALSE;
	}

	mld_op = arg;
	MTWF_PRINT("(%s) command: %s\n", RtmpOsGetNetDevName(wdev->if_dev), mld_op);

	op_attr = rtstrchr(mld_op, ':');
	if (op_attr) {
		*op_attr = 0;
		op_attr++;
	}

	if (strcmp("create", mld_op) == 0) {
		u8 mld_type;

		if (!parse_mld_create_args(op_attr, &op_ctrl, &pres_bmap))
			goto err;

		eht_ap_mld_create(wdev, &op_ctrl.mld_grp,
			op_ctrl.mld_addr, &op_ctrl.mld_param, &mld_type);
	} else if (strcmp("destroy", mld_op) == 0) {
		if (!parse_mld_destroy_args(op_attr, &op_ctrl, &pres_bmap))
			goto err;

		if (pres_bmap & MLD_OP_PARAM_PRES_GROUP_IDX) {
			eht_ap_mld_destroy(wdev, op_ctrl.mld_grp);
		} else if (pres_bmap & MLD_OP_PARAM_PRES_ADDR) {
			eht_ap_mld_destroy_by_mld_addr(wdev, op_ctrl.mld_addr);
		} else {
			MTWF_PRINT("Err: one of group and addr is required.\n");
			goto err;
		}
	} else if (strcmp("addlink", mld_op) == 0) {
		if (!parse_mld_add_link_args(op_attr, &op_ctrl, &pres_bmap))
			goto err;

		if (pres_bmap & MLD_OP_PARAM_PRES_GROUP_IDX) {
			eht_ap_mld_add_link(wdev, op_ctrl.mld_grp);
		} else if (pres_bmap & MLD_OP_PARAM_PRES_ADDR) {
			eht_ap_mld_add_link_by_mld_addr(wdev, op_ctrl.mld_addr);
		} else {
			MTWF_PRINT("Err: one of group and addr is required.\n");
			goto err;
		}
	} else if (strcmp("dellink", mld_op) == 0) {
		if (!parse_mld_del_link_args(op_attr, &op_ctrl, &pres_bmap))
			goto err;

		if ((pres_bmap & MLD_OP_PARAM_PRES_RECONFIG_TO) && op_ctrl.reconfig_to) {
			/* trigger 802.11be MLO Reconfiguration*/
			MTWF_PRINT("trigger MLO reconfiguration (to=%d)\n", op_ctrl.reconfig_to);
			eht_ap_mld_trig_link_reconfiguration(wdev, op_ctrl.reconfig_to);
		} else {
			struct query_mld_ap_basic bss_mld_info_basic = {0};
			struct reconfig_tmr_to_t reconfig_to_info = {0};

			if (bss_mngr_query_mld_ap_basic(wdev, &bss_mld_info_basic) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
					"Invalid query\n");
				goto err;
			}

			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_NOTICE,
				"Link id %d to be removed from %pM\n",
				bss_mld_info_basic.link_id, bss_mld_info_basic.addr);

			COPY_MAC_ADDR(reconfig_to_info.mld_addr, bss_mld_info_basic.addr);
			reconfig_to_info.to_link_id_bmap |= BIT(bss_mld_info_basic.link_id);
			reconfig_to_info.args.reconfig_mode = MLD_RECONFIG_MODE_INSTANT;
			eht_ap_mld_exec_link_reconfiguration(&reconfig_to_info);
		}
	} else if (strcmp("tsfrlink", mld_op) == 0) {
		if (!parse_mld_link_transfer_args(op_attr, &op_ctrl, &pres_bmap))
			goto err;

		if ((pres_bmap & MLD_OP_PARAM_PRES_RECONFIG_TO) && op_ctrl.reconfig_to) {
			/* trigger 802.11be MLO Reconfiguration*/
			MTWF_PRINT("TODO: trigger MLO reconfiguration\n");
		} else {
			if (pres_bmap & MLD_OP_PARAM_PRES_GROUP_IDX) {
				eht_ap_mld_link_transfer(wdev, op_ctrl.mld_grp);
			} else if (pres_bmap & MLD_OP_PARAM_PRES_ADDR) {
				; /* TODO */
			} else {
				MTWF_PRINT("Err: one of group and addr is required.\n");
				goto err;
			}
		}
	} else if (strcmp("attr", mld_op) == 0) {
		if (!parse_mld_attr_set_args(op_attr, &op_ctrl, &pres_bmap))
			goto err;

		op_ctrl.pres_bmap = pres_bmap;

		if (pres_bmap & MLD_OP_PARAM_PRES_GROUP_IDX) {
			eht_ap_mld_attr_set(wdev, op_ctrl.mld_grp, &op_ctrl);
		} else if (pres_bmap & MLD_OP_PARAM_PRES_ADDR) {
			eht_ap_mld_attr_set_by_mld_addr(wdev, op_ctrl.mld_addr, &op_ctrl);
		} else {
			MTWF_PRINT("Err: one of group and addr is required.\n");
			goto err;
		}
	} else {
		MTWF_PRINT("Unknown op\n");
		goto err;
	}

	return TRUE;
err:
	MTWF_PRINT
		("\n\tUsage: iwpriv $(inf_name) set apmld=");
	MTWF_PRINT
		("operation:param1=value1,[param2=value2,]...\n");
	MTWF_PRINT
		("\t\toperation: create|destroy|addlink|dellink|tsfrlink|attr\n");
	MTWF_PRINT
		("\t\tparam for create:  group,addr\n");
	MTWF_PRINT
		("\t\tparam for destroy: group or addr\n");
	MTWF_PRINT
		("\t\tparam for addlink: group or addr\n");
	MTWF_PRINT
		("\t\tparam for dellink: [reconfig_to]\n");
	MTWF_PRINT
		("\t\tparam for tsfrlink: group or addr\n");
	MTWF_PRINT
		("\t\tparam for attr: group or addr,[eml_mode]\n");
	MTWF_PRINT
		("\t\t\teml_mode: 0(disable), 1(emlsr)\n");

	return FALSE;
}

BOOLEAN eht_ap_mld_available_address_search(
	IN struct wifi_dev *wdev,
	IN u8 mld_group_idx,
	OUT u8 *mld_addr,
	IN u8 mld_type)
{
	struct _RTMP_ADAPTER *pAd = NULL;
	struct _BSS_STRUCT *pMy_Mbss = NULL;

	if (!wdev || !mld_addr)
		return FALSE;

	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	if (!pAd)
		return FALSE;

	pMy_Mbss = &pAd->ApCfg.MBSSID[wdev->func_idx];

	if (mld_type == BMGR_MLD_TYPE_SINGLE) {
		memcpy(mld_addr, wdev->if_addr, MAC_ADDR_LEN);
		goto mld_find;
	}
	/* get profile mld mac */
	if ((mld_type == BMGR_MLD_TYPE_MULTI) &&
		pMy_Mbss->mld_addr_by_cfg) {
		memcpy(mld_addr, pMy_Mbss->pf_mld_addr, MAC_ADDR_LEN);
		goto mld_find;
	}
	/* get available unique mld mac from MBSS list */
	if ((mld_type == BMGR_MLD_TYPE_MULTI) &&
		BMGR_IS_ML_MLD_GRP_IDX(mld_group_idx) &&
		pMy_Mbss->unique_mld_addr_enable) {
		UINT32 i, my_band_idx, band_idx;
		struct _BSS_STRUCT *pMbss_search = NULL;
		struct query_mld_basic mld_basic;
		struct _RTMP_ADAPTER *mac_ad = NULL;

		/* search from assigned wdev */
		COPY_MAC_ADDR(mld_basic.addr, pMy_Mbss->multilink_mld_addr);
		if (bss_mngr_query_mld_basic(&mld_basic) != NDIS_STATUS_SUCCESS) {
			/* this mld address is not in use */
			memcpy(mld_addr, pMy_Mbss->multilink_mld_addr, MAC_ADDR_LEN);
			goto mld_find;
		}

		my_band_idx = HcGetBandByWdev(wdev);
		/* search from assigned wdev band */
		mac_ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, my_band_idx);
		/* the band is not existed,next band */
		if (!mac_ad)
			return FALSE;
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_INFO,
			"\x1b[41m (band:%d, bssid_num:%d)\x1b[m\n\r",
			my_band_idx, mac_ad->ApCfg.BssidNum);

		for (i = 0; i < mac_ad->ApCfg.BssidNum; i++) {
			pMbss_search = &mac_ad->ApCfg.MBSSID[i];
			if (pMbss_search == pMy_Mbss) /* already searched */
				continue;
			if (pMbss_search->wdev.if_up_down_state == FALSE)
				continue;

			/* query success,means mld address is already in use, use next one
			*/
			COPY_MAC_ADDR(mld_basic.addr, pMbss_search->multilink_mld_addr);
			if (bss_mngr_query_mld_basic(&mld_basic) != NDIS_STATUS_SUCCESS) {
				/* this mld address is not in use */
				memcpy(mld_addr, pMbss_search->multilink_mld_addr, MAC_ADDR_LEN);
				goto mld_find;
			}
		}

		for (band_idx = 0; band_idx < CFG_WIFI_RAM_BAND_NUM; band_idx++) {
			if (band_idx == my_band_idx)
				continue;/* already searched */
			mac_ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, band_idx);
			/* the band is not existed,next band */
			if (!mac_ad)
				continue;/* BUG?? */
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_INFO,
				"\x1b[41m (band:%d, bssid_num:%d)\x1b[m\n\r",
				band_idx, mac_ad->ApCfg.BssidNum);

			for (i = 0; i < mac_ad->ApCfg.BssidNum; i++) {
				pMbss_search = &mac_ad->ApCfg.MBSSID[i];
				if (pMbss_search->wdev.if_up_down_state == FALSE)
					continue;

				/* query success,means mld address is already in use, use next one
				*/
				COPY_MAC_ADDR(mld_basic.addr, pMbss_search->multilink_mld_addr);
				if (bss_mngr_query_mld_basic(&mld_basic) != NDIS_STATUS_SUCCESS) {
					/* this mld address is not in use */
					memcpy(mld_addr, pMbss_search->multilink_mld_addr, MAC_ADDR_LEN);
					goto mld_find;
				}
			}
		}
	}
	return FALSE;

mld_find:
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_INFO,
		"\x1b[41m (grp:%d, mld addr:%pM, wdev addr:%pM)\x1b[m\n\r",
		mld_group_idx, mld_addr, wdev->if_addr);

	return TRUE;
}

INT eht_ap_mld_register_link_recomm(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev, /* link that tx LR */
	IN u8 *recomm_link_addr, /* recommended link */
	IN u8 enable,
	IN u16 num_dtim,
	IN u16 tx_times)
{
	u8 tx_link_id;
	struct eht_period_link_recomm_t *recom_info;

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
		"%s Link Recomms\n", enable ? "enable" : "disable");

	recom_info = &pAd->ApCfg.link_recom;
	if (enable) {
		if ((wdev->wdev_type == WDEV_TYPE_AP) &&
			WMODE_CAP_BE(wdev->PhyMode) &&
			WDEV_BSS_STATE(wdev) >= BSS_READY) {

			struct query_mld_ap_basic bss_mld_info_basic = {0};
			struct query_mld_basic mld_basic = {0};
			BSS_STRUCT *pMbss = wdev->func_dev;

			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
				"===> Recommended link addr(%pM)\n", recomm_link_addr);

			if (recom_info->cnt > 0) {
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
					"Disable existed Link Recomm First!\n");
				return NDIS_STATUS_FAILURE;
			}

			/* interface -> link_id */
			if (bss_mngr_query_mld_ap_basic(wdev, &bss_mld_info_basic) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
					"Invalid query\n");
				return NDIS_STATUS_FAILURE;
			}
			tx_link_id = bss_mld_info_basic.link_id;

			/* 1. Link Addr -> MLD Addr & link_id */
			NdisCopyMemory(bss_mld_info_basic.bssid, recomm_link_addr, MAC_ADDR_LEN);
			if (bss_mngr_query_mld_by_bssid(&bss_mld_info_basic) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
					"Invalid BSSID (%pM)\n", recomm_link_addr);
				return NDIS_STATUS_FAILURE;
			}

			/* 2. MLD Addr -> mld_group_idx */
			COPY_MAC_ADDR(mld_basic.addr, bss_mld_info_basic.addr);
			if (bss_mngr_query_mld_basic(&mld_basic) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
					"MLD Lookup failed: MLD MAC (%pM)\n", bss_mld_info_basic.addr);
				return NDIS_STATUS_FAILURE;
			}

			/* 3. register Link Recommendation scheduling */
			recom_info->recom_mld_grp = mld_basic.mld_grp_idx;
			recom_info->recom_link_bmap = 0 | BIT(bss_mld_info_basic.link_id);
			recom_info->tx_link_bmap = 0 | BIT(tx_link_id);

			recom_info->num_mlme_period =
				(num_dtim * pMbss->DtimPeriod * pAd->CommonCfg.BeaconPeriod) / MLME_TASK_EXEC_INTV;
			recom_info->tx_times = tx_times;

			recom_info->cnt++;
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
				"grp=%d, link=0x%x, num_mlme_pd=%d, tx_bmap=%d, times=%d, cnt=%d\n",
				recom_info->recom_mld_grp, recom_info->recom_link_bmap,
				recom_info->num_mlme_period, recom_info->tx_link_bmap, recom_info->tx_times,
				recom_info->cnt);
		} else {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
				"Invalid Op, dev %s, cap 0x%x, state %d\n",
				RtmpOsGetNetDevName(wdev->if_dev),
				WMODE_CAP_BE(wdev->PhyMode),
				WDEV_BSS_STATE(wdev));
			return NDIS_STATUS_FAILURE;
		}
	} else {
		if (recom_info->cnt) {
			recom_info->cnt--;
			recom_info->tx_link_bmap = 0;
			recom_info->tx_times = 0;
			recom_info->recom_link_bmap = 0;
			recom_info->recom_mld_grp = 0;
		} else {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_NOTICE,
				"without any LR scheduling\n");
		}
	}

	return NDIS_STATUS_SUCCESS;
}

INT eht_ap_mld_periodic_link_recomm(
	IN struct _RTMP_ADAPTER *pAd)
{
	struct query_mld_conn *mld_conn = NULL;
	struct mld_link_recomm *link_recomm = NULL;
	struct eht_period_link_recomm_t recom_args;
	u32 mem_size, i;
	int ret;

	if (!pAd->ApCfg.link_recom.cnt)
		return NDIS_STATUS_SUCCESS;

	if (((pAd->Mlme.PeriodicRound % pAd->ApCfg.link_recom.num_mlme_period) == 0) &&
		pAd->ApCfg.link_recom.tx_times) {

		struct query_mld_sta *mld_sta_info;
		struct mld_recomm_sta_info *recomm_sta_info;

		pAd->ApCfg.link_recom.tx_times--;

		if (!pAd->ApCfg.link_recom.tx_times)
			pAd->ApCfg.link_recom.cnt--;

		NdisCopyMemory(&recom_args, &pAd->ApCfg.link_recom, sizeof(recom_args));

		/* 3. mld_group_idx -> mld_sta_idx(s) */
		mem_size = sizeof(struct query_mld_conn);
		mem_size += BMGR_MAX_MLD_STA_CNT * sizeof(struct query_mld_sta);

		os_alloc_mem(NULL, (UCHAR **)&mld_conn, mem_size);
		if (!mld_conn)
			return NDIS_STATUS_FAILURE;

		NdisZeroMemory(mld_conn, mem_size);

		mld_conn->mld_group_idx = recom_args.recom_mld_grp;
		ret = bss_mngr_query_mld_conn(mld_conn);
		if (ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"MLD Conn Query failed: mld_group_idx(%d)\n",
				pAd->ApCfg.link_recom.recom_mld_grp);
			goto err;
		}

		/* 4. transmit LR frames */
		mem_size = sizeof(struct mld_link_recomm);
		mem_size += mld_conn->mld_sta_num * sizeof(struct mld_recomm_sta_info);

		os_alloc_mem(NULL, (UCHAR **)&link_recomm, mem_size);
		if (!link_recomm) {
			ret = NDIS_STATUS_FAILURE;
			goto err;
		}
		NdisZeroMemory(link_recomm, mem_size);

		link_recomm->reason_code = REASON_UNSPECIFY;
		link_recomm->mld_sta_num = mld_conn->mld_sta_num;

		recomm_sta_info = (struct mld_recomm_sta_info *)link_recomm->mld_sta_info;
		mld_sta_info = (struct query_mld_sta *)mld_conn->mld_sta_info;
		for (i = 0; i < mld_conn->mld_sta_num; i++) {
			recomm_sta_info->mld_sta_idx = mld_sta_info->mld_sta_idx;
			recomm_sta_info->aid = mld_sta_info->aid;
			/* recommend one link */
			recomm_sta_info->recomm_links = recom_args.recom_link_bmap;
			recomm_sta_info->valid = TRUE;

			recomm_sta_info++;
			mld_sta_info++;
		}

		bss_mngr_mld_tx_link_recomm(recom_args.recom_mld_grp,
			recom_args.tx_link_bmap, link_recomm);
	}

	ret = NDIS_STATUS_SUCCESS;

err:
	if (mld_conn)
		os_free_mem(mld_conn);

	if (link_recomm)
		os_free_mem(link_recomm);

	return ret;
}

INT set_ap_mld_link_recomm_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,
		pObj->ioctl_if, pObj->ioctl_if_type);
	CHAR *param;
	u8 i, enable, mac_len, recomm_link_addr[MAC_ADDR_LEN] = {0};
	u16 tx_times = 60, num_dtim = 5;

	if (arg == NULL || strlen(arg) == 0)
		goto err;

	if (!wdev) {
		MTWF_PRINT("(ERROR) wdev not found\n");
		return FALSE;
	}

	param = rstrtok(arg, "-");
	if (param) {
		if (!kstrtou8(param, 10, &enable)) {
			MTWF_PRINT("enable=%d\n", enable);
		} else {
			MTWF_PRINT("Err: enable is invalid.\n");
			goto err;
		}

		if (enable) {
			param = rstrtok(NULL, "-");
			if (!param)
				goto err;

			/* Mac address acceptable format 01:02:03:04:05:06 (len=17) */
			mac_len = strlen(param);
			if (mac_len != 17) {
				MTWF_PRINT("invalid MAC length (%d)\n", mac_len);
				goto err;
			} else if (strcmp(param, "00:00:00:00:00:00") == 0) {
				MTWF_PRINT("invalid mac setting\n");
				goto err;
			} else {
				for (i = 0; i < MAC_ADDR_LEN; i++)
					AtoH((param + (i*3)), &recomm_link_addr[i], 1);

				MTWF_PRINT("Recommended Link MAC addr: %pM\n", recomm_link_addr);
			}

			param = rstrtok(NULL, "-");
			if (param) {
				if (!kstrtou16(param, 10, &num_dtim)) {
					MTWF_PRINT("num_dtim=%d\n", num_dtim);
				} else {
					MTWF_PRINT("Err: num_dtim is invalid.\n");
					goto err;
				}

				param = rstrtok(NULL, "-");
				if (param) {
					if (!kstrtou16(param, 10, &tx_times)) {
						MTWF_PRINT("tx_times=%d\n", tx_times);
					} else {
						MTWF_PRINT("Err: tx_times is invalid.\n");
						goto err;
					}
				}
			}
		}
	} else
		goto err;

	if (eht_ap_mld_register_link_recomm(pAd, wdev, recomm_link_addr,
			enable, num_dtim, tx_times) != NDIS_STATUS_SUCCESS)
		goto err;

	return TRUE;
err:
	MTWF_PRINT("\n\tRegister Link Recommendation Scheduling\n");
	MTWF_PRINT("\n\tUsage: iwpriv $(inf_name) set link_recomm=");
	MTWF_PRINT("enable-Recommended_BSSID[-num_dtim][-tx_times]\n");
	MTWF_PRINT("\t\tinf_name: AP that tranmits LR frames\n");
	MTWF_PRINT("\t\tenable: enable/disable schedule Link Recommendataion Frames\n");
	MTWF_PRINT("\t\tRecommended_BSSID: Link MAC Address of recommended link\n");
	MTWF_PRINT("\t\tnum_dtim: number of DTIM the tx LR frames event occurs\n");
	MTWF_PRINT("\t\ttx_times: number of LR frame events\n\n");
	MTWF_PRINT("\tExample:\n");
	MTWF_PRINT("\tTrigger 1 time tx LR frames\n");
	MTWF_PRINT("\t\tiwpriv ra0 set link_recomm=1-00:01:23:45:67:89-1-1\n");
	MTWF_PRINT("\tTrigger 60 times tx LR frames every 5 DTIMs\n");
	MTWF_PRINT("\t\tiwpriv ra0 set link_recomm=1-00:01:23:45:67:89-5-60\n");
	MTWF_PRINT("\tDisable LR frame scheduling\n");
	MTWF_PRINT("\t\tiwpriv ra0 set link_recomm=0\n");

	MTWF_PRINT("\nScheduling Info:\n");
	MTWF_PRINT("cnt = %d\n", pAd->ApCfg.link_recom.cnt);
	MTWF_PRINT("num_mlme_period = %d\n", pAd->ApCfg.link_recom.num_mlme_period);
	MTWF_PRINT("tx_link_bmap = %d\n", pAd->ApCfg.link_recom.tx_link_bmap);
	MTWF_PRINT("tx_times = %d\n", pAd->ApCfg.link_recom.tx_times);
	MTWF_PRINT("recom_mld_grp = %d\n", pAd->ApCfg.link_recom.recom_mld_grp);
	MTWF_PRINT("recom_link_bmap = %d\n", pAd->ApCfg.link_recom.recom_link_bmap);

	return FALSE;
}

/**
 * @parse nt2lm request argument initated by one BSS of AP MLD
 *  iwpriv IF set nt2lm=req:addr=peer_link_addr,tid=aa:bb:cc:dd:ee:ff
 *  aa~ff is tid map in hex such as 0x00 or 0xff
 *
 * @param *peer_mld peer mld addr
 * @param *ap_mld ap mld addr
 * @param *tid_map_dl dl tid map for links
 * @param *tid_map_ul dl tid map for links
 */
int nt2lm_req_args_parse(
	RTMP_STRING * arg,
	u8 *peer_mld,
	u8 *dir,
	u8 *tid_map
)
{
	int ret = 0;
	RTMP_STRING *attrs = arg;
	RTMP_STRING *tmpbuf = NULL;

	if (!arg || !strlen(arg)) {
		ret = -EINVAL;
		goto err;
	}

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (!tmpbuf) {
		ret = -ENOMEM;
		goto err;
	}

	if (get_value_by_key("peer_addr", tmpbuf, 100, attrs)) {
		u16 i, mac_len;
		u8 mac_addr[MAC_ADDR_LEN];

		/* mac_addr acceptable format 01:02:03:04:05:06 (len=17) */
		mac_len = strlen(tmpbuf);
		if ((mac_len != 17)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
				"invalid peer_mld_addr (len=%d)\n", mac_len);
			ret = -EINVAL;
			goto err;
		} else {
			for (i = 0; i < MAC_ADDR_LEN; i++)
				AtoH((tmpbuf + (i*3)), &mac_addr[i], 1);
			COPY_MAC_ADDR(peer_mld, mac_addr);
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_NOTICE,
				"peer_addr=%pM\n", peer_mld);
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"Err: addr is required\n");
		ret = -EINVAL;
		goto err;
	}

	if (get_value_by_key("dir", tmpbuf, 100, attrs)) {
		if (!kstrtou8(tmpbuf, 10, dir)) {
			if (*dir <= TID_DL_UL_MAX) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_NOTICE,
					"dir=%d\n", *dir);
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
					"Err: invalid dir=%d\n", *dir);
				ret = -EINVAL;
				goto err;
			}
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"Err: addr is required\n");
		ret = -EINVAL;
		goto err;
	}

	if (get_value_by_key("tid", tmpbuf, 100, attrs)) {
		u16 i, len;

		/* tid map acceptable format aa:bb:cc (len=8) */
		len = strlen(tmpbuf);
		if ((len != 8)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
				"invalid tid map (len=%d)\n", len);
			ret = -EINVAL;
			goto err;
		} else {
			if (tid_map) {
				for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++)
					AtoH((tmpbuf + (i*3)), &tid_map[i], 1);
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_NOTICE,
					"tid_map[]=0x%.2x,0x%.2x,0x%.2x\n",
					tid_map[0], tid_map[1], tid_map[2]);
			}
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"Err: addr is required\n");
		ret = -EINVAL;
		goto err;
	}

err:
	if (tmpbuf)
		os_free_mem(tmpbuf);
	return ret;
}

/**
 * @parse nt2lm teardown argument
 *  iwpriv IF set nt2lm=teardown:addr=mac_addr
 *
 * @param *addr peer link to be sent out nt2lm teardown frame
 */
int nt2lm_teardown_args_parse(
	RTMP_STRING *arg,
	u8 *addr
)
{
	int ret = 0;
	RTMP_STRING *attrs = arg;
	RTMP_STRING *tmpbuf = NULL;

	if (!arg || !strlen(arg)) {
		ret = -EINVAL;
		goto err;
	}

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (!tmpbuf) {
		ret = -ENOMEM;
		goto err;
	}

	if (get_value_by_key("addr", tmpbuf, 100, attrs)) {
		u16 i, mac_len;
		u8 mac_addr[MAC_ADDR_LEN];

		/* mac_addr acceptable format 01:02:03:04:05:06 (len=17) */
		mac_len = strlen(tmpbuf);
		if ((mac_len != 17)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
				"invalid link addr (len=%d)\n", mac_len);
			ret = -EINVAL;
			goto err;
		} else {
			for (i = 0; i < MAC_ADDR_LEN; i++)
				AtoH((tmpbuf + (i*3)), &mac_addr[i], 1);
			COPY_MAC_ADDR(addr, mac_addr);
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_NOTICE,
				"link addr=%pM\n", addr);
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"Err: addr is required\n");
		ret = -EINVAL;
		goto err;
	}

err:
	if (tmpbuf)
		os_free_mem(tmpbuf);
	return ret;
}

/**
 * @set_ap_at2lm_proc handle advertisted t2lm request
 * 0: UCC or normal operation (ex. 0:10000:16000)
 * 1: force to trigger MST timeout
 * 2: force to trigger E timeout
 *
 * @param *ad RTMP_ADAPTER
 * @param *arg RTMP_STRING
 */
INT set_ap_at2lm_proc(RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) ad->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(ad,
		pObj->ioctl_if, pObj->ioctl_if_type);
	int para_num = 0;
	u32 type = 0;

	if (arg == NULL || strlen(arg) == 0)
		goto err;

	if (!wdev) {
		MTWF_PRINT("err: wdev not found\n");
			return FALSE;
	}

	para_num = sscanf(arg, "%d", &type);
	if (para_num != 1) {
		MTWF_PRINT("Invalid format. Please use below format\n");
		MTWF_PRINT("Usage: at2lm=0:MST(ms):ED(ms)\n");
		MTWF_PRINT("Usage: at2lm=1:mld_addr:dis_linkid_bitmap:MST(ms):ED(ms)\n");
		goto err;
	}

	switch (type) {
	case 0: {
		u32 mst_dur = 0;
		u32 e_dur = 0;

		para_num = sscanf(arg, "%d:%d:%d", &type, &mst_dur, &e_dur);

		if (para_num != 3) {
			MTWF_PRINT("Invalid format. Usage: at2lm=0:MST(ms):ED(ms)\n");
			goto err;
		}

		MTWF_PRINT("type=%d, mst_dur=%d(ms), e_dur=%d(ms)\n", type, mst_dur, e_dur);

		at2lm_req_certi(wdev, mst_dur, e_dur);
		break;
	}
	case 1: {
		u8 i = 0;
		u32 _mld_addr[MAC_ADDR_LEN];
		u8 mld_addr[MAC_ADDR_LEN];
		u32 dis_linkid_bitmap = 0;
		u32 mst_dur = 0;
		u32 e_dur = 0;

		para_num = sscanf(arg, "%d:%02x:%02x:%02x:%02x:%02x:%02x:%04x:%d:%d",
			&type, &_mld_addr[0], &_mld_addr[1], &_mld_addr[2], &_mld_addr[3], &_mld_addr[4],
			&_mld_addr[5], &dis_linkid_bitmap, &mst_dur, &e_dur);

		for (i = 0; i < MAC_ADDR_LEN; i++)
			mld_addr[i] = _mld_addr[i];

		if (para_num != 10) {
			MTWF_PRINT("Invalid format. Usage: at2lm=1:mld_addr:dis_linkid_bitmap:");
			MTWF_PRINT("MST(ms):ED(ms)\n");
			goto err;
		}

		MTWF_PRINT("type=%d, mld_addr=%pM, dis_linkid_bitmap=0x%04x, ",
			type, mld_addr, dis_linkid_bitmap);
		MTWF_PRINT("MST=%d(ms), ED=%d(ms)\n",
			mst_dur, e_dur);

		at2lm_req(mld_addr, dis_linkid_bitmap, mst_dur, e_dur);
		break;
	}
	default:
		MTWF_PRINT("err: type=%d not support\n", type);
		break;
	}

	return TRUE;
err:
	return FALSE;
}

INT set_ap_at2lm_setting(RTMP_ADAPTER *ad, void *data)
{
	int ret = 0;
	u8 link_id = 0, mld_bss_idx = 0;
	struct bmgr_mlo_dev *mld = NULL;
	u16 max_dis_linkid_bitmap = BITS(0, BSS_MNGR_MAX_BAND_NUM - 1);
	struct nl_80211_at2lm_map *at2lm_map = (struct nl_80211_at2lm_map *)data;

	if (!ad || !data) {
		MTWF_PRINT("invalid pointer, ad:%p, data:%p\n", ad, data);
		return -1;
	}

	MTWF_DBG_NP(DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"mld_addr:%pM, dis_link_map: 0x%x, mst_dur:%d, exp_dur:%d\n",
		at2lm_map->mld_addr, at2lm_map->dis_link_map,
		at2lm_map->mst_dur, at2lm_map->exp_dur);

	ret = find_ap_mld_by_mld_addr(at2lm_map->mld_addr, &mld);
	if (ret) {
		MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"can't find ap by mld_addr:%pM\n", at2lm_map->mld_addr);
		return -1;
	}
	/*limit disable link map*/
	at2lm_map->dis_link_map &= max_dis_linkid_bitmap;

	/*refine disable link map*/
	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		if (at2lm_map->dis_link_map & (1 << link_id)) {
			mld_bss_idx = mld->bss_idx_mld[link_id];
			if (!BMGR_VALID_BSS_IDX(mld_bss_idx))
				at2lm_map->dis_link_map &= ~(1 << link_id);
		}
	}
	MTWF_DBG_NP(DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"refine dis_link_map: 0x%x\n", at2lm_map->dis_link_map);

	if (!at2lm_map->dis_link_map) {
		MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"invalid disable link map(0)\n");
		return -1;
	}

	at2lm_req(at2lm_map->mld_addr, at2lm_map->dis_link_map,
		at2lm_map->mst_dur, at2lm_map->exp_dur);

	return 0;
}

/**
 * @set_ap_nt2lm_proc handle nego. t2lm request
 * req: peer mld sta request nego t2lm
 *  ex. n2tlm=req:peer_addr=addr,dir=[0/1/2],
 *                 tid=link_id_0_tid_map:link_id_1_tid_map:link_id_2_tid_map
 * teardown: apl mld teardown nego. t2lm to peer link=addr3
 *  ex. nt2lm=teardown:addr=addr_3
 *
 * @param *ad RTMP_ADAPTER
 * @param *arg RTMP_STRING
 */
INT set_ap_nt2lm_proc(RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) ad->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(ad,
		pObj->ioctl_if, pObj->ioctl_if_type);
	RTMP_STRING *n2tlm_op, *op_attr;
	u8 link_id, tid_idx;

	if (arg == NULL || strlen(arg) == 0)
		goto err;

	if (!wdev) {
		MTWF_PRINT("err: wdev not found\n");
		return FALSE;
	}

	n2tlm_op = arg;
	MTWF_PRINT("(%s) command: %s\n", RtmpOsGetNetDevName(wdev->if_dev), n2tlm_op);

	op_attr = rtstrchr(n2tlm_op, ':');
	if (op_attr) {
		*op_attr = 0;
		op_attr++;
	}

	if (strcmp("req", n2tlm_op) == 0) {
		u8 peer_addr[MAC_ADDR_LEN];
		u8 dir;
		u8 tid_map[MLD_LINK_MAX] = {0};
		MAC_TABLE_ENTRY *entry = NULL;
		struct nt2lm_contract_t *nt2lm_contract = NULL;
		u8 tid_bit_map = 0;
		u16 link_map[TID_MAX] = {0};

		if (nt2lm_req_args_parse(op_attr, peer_addr, &dir, tid_map))
			goto err;
		entry = MacTableLookup(ad, peer_addr);
		if (!entry) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
					"INVALID mac entry\n");
			goto err;
		}
		if (entry->mlo.mlo_en) {
			if (wdev->wdev_type == WDEV_TYPE_AP) {
				struct bmgr_mld_sta *mld_sta = NULL;

				find_mld_sta_by_wcid(ad, entry->wcid, &mld_sta);
				if (!mld_sta || !mld_sta->valid) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
					"INVALID MLD_STA\n");
					goto err;
				}
				nt2lm_contract = &mld_sta->nt2lm_contract;
			} else if (wdev->wdev_type == WDEV_TYPE_STA) {
				struct mld_dev *mld = wdev->mld_dev;

				if (!mld)
					goto err;
				if (!mld->peer_mld.valid)
					goto err;

				nt2lm_contract = &mld->peer_mld.nt2lm_contract;
			} else
				goto err;

			nt2lm_contract->dir = dir;
			for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
				nt2lm_contract->tid_map[link_id] = tid_map[link_id];
				for (tid_idx = 0; tid_idx < TID_MAX; tid_idx++)
					tid_bit_map |= (tid_map[link_id] & (1 << tid_idx));
			}
			nt2lm_contract->tid_bit_map = tid_bit_map;
			nt2lm_contract->request_type = REQ_FROM_IWPRIV;

			for (tid_idx = 0; tid_idx < TID_MAX; tid_idx++) {
				for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++)
					link_map[tid_idx] |= (((tid_map[link_id] >> tid_idx) & 1) << link_id);
				nt2lm_contract->link_map[tid_idx] = link_map[tid_idx];
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
					"link_map[%d]=0x%.4x\n", tid_idx, nt2lm_contract->link_map[tid_idx]);
			}

			nt2lm_t2lm_request(ad, wdev, entry->wcid);
		}
	} else if (strcmp("teardown", n2tlm_op) == 0) {
		u8 addr[MAC_ADDR_LEN];
		MAC_TABLE_ENTRY *entry = NULL;

		if (nt2lm_teardown_args_parse(op_attr, addr))
			goto err;

		entry = MacTableLookup(ad, addr);
		if (!(IS_VALID_ENTRY(entry) && (IS_ENTRY_CLIENT(entry) ||
			IS_ENTRY_PEER_AP(entry)))) {
			MTWF_PRINT("err: entry(%pM) is not valid\n", addr);
			return FALSE;
		}
		nt2lm_t2lm_teardown(ad, entry->wdev, entry->wcid);
	} else if (strcmp("show", n2tlm_op) == 0) {
		mld_sta_tid_map_show(wdev);
	} else {
		MTWF_PRINT("Unknown op\n");
		goto err;
	}

	return TRUE;
err:
	MTWF_PRINT
		("\n\tUsage: iwpriv $(inf_name) set nt2lm=");
	MTWF_PRINT
		("op:param1=value1,[param2=value2,]...\n");
	MTWF_PRINT
		("\t\top: req|teardown\n");
	MTWF_PRINT
		("\t\tparam for req: addr/tid\n");
	MTWF_PRINT
		("\t\tparam for teardown: addr\n");

	return FALSE;
}

INT set_ap_reconfig_rm_link_proc(RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) ad->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(ad,
		pObj->ioctl_if, pObj->ioctl_if_type);
	int para_num = 0;
	u32 cmd;


	if (arg == NULL || strlen(arg) == 0)
		goto err;

	if (!wdev) {
		MTWF_PRINT("err: wdev not found\n");
			return FALSE;
	}

	para_num = sscanf(arg, "%d", &cmd);

	if (para_num > 1) {
		MTWF_PRINT("Invalid format.\n");
		goto err;
	}

	switch (cmd) {
	case 0: {
		u8 i = 0;
		struct reconfig_rm_link_req_t req = {0};
		u8 link_id_bss_info_idx[MLD_LINK_MAX] = {0};
		u8 mld_addr[MAC_ADDR_LEN] = {0x00, 0x0c, 0x43, 0x26, 0x60, 0x11};

		MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_NOTICE,
			"wdev=%s, bss_mngr_mld_reconfig_peer_mld() req\n", wdev->if_dev->name);

		bss_mngr_mld_reconfig_peer_mld(1, 0x01);

		COPY_MAC_ADDR(req.mld_addr, mld_addr);
		req.fw_mld_idx = 0xff;
		req.flag = 0;
		req.rm_link_id_bitmap = 0x01;
		for (i = 0; i < MLD_LINK_MAX; i++)
			req.link_id_bss_info_idx[i] = link_id_bss_info_idx[i];
		HW_REQ_RECONFIG_RM_LINK(wdev, &req);
		break;
	}
	default:
		break;
	}

	return TRUE;
err:
	MTWF_PRINT("\n\tUsage: iwpriv $(inf_name) set mlr=0");
	return FALSE;
}

#endif /* DOT11_EHT_BE */

/*system for ap mode*/
/*
*
*/
INT ap_link_up(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{

	MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_WARN,
		"(caller:%pS), wdev(%d)\n", OS_TRACE, wdev->wdev_idx);

	if (WDEV_BSS_STATE(wdev) == BSS_INIT) {
		wifi_sys_linkup(wdev, NULL);
		APStartRekeyTimer(wdev->sys_handle, wdev);
	}

	OPSTATUS_SET_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	return TRUE;
}

/*
*
*/
INT ap_link_down(struct wifi_dev *wdev)
{

	MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_INFO,
		"(caller:%pS), wdev(%d)\n", OS_TRACE, wdev->wdev_idx);

	APStopRekeyTimer(wdev->sys_handle, wdev);
	ap_sec_deinit(wdev);

	if (WDEV_BSS_STATE(wdev) >= BSS_ACTIVE) {
		/* bss not ready for mlme */
		WDEV_BSS_STATE(wdev) = BSS_ACTIVE;
		/* kick out all stas behind the Bss */
		MbssKickOutStas(wdev->sys_handle, wdev->func_idx, REASON_DISASSOC_INACTIVE);
		UpdateBeaconHandler(
			wdev->sys_handle,
			wdev,
			BCN_REASON(BCN_UPDATE_DISABLE_TX));

		/*linkdown bssinfo*/
		if (wifi_sys_linkdown(wdev) != TRUE) {
			MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR,
					 "linkdown fail!\n");
			return FALSE;
		}
	}

	return TRUE;
}

/*
*
*/
INT ap_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#ifdef SW_CONNECT_SUPPORT
	struct _STA_TR_ENTRY *tr_entry = NULL;
#endif /* SW_CONNECT_SUPPORT */

	/*generic connection action*/
	if (wifi_sys_conn_act(wdev, entry) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR,
				 "connect action fail!\n");
	}

	/* Indicating Complete status for wait completion at WpaSend */

	if (entry->EntryState == ENTRY_STATE_SYNC) {
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
			"**calling RTMP_OS_COMPLETE(WtblSetDone)\n");
		entry->WtblSetFlag = TRUE;
		RTMP_OS_COMPLETE(&entry->WtblSetDone);
	}

#ifdef SW_CONNECT_SUPPORT
	tr_entry = tr_entry_get(ad, entry->tr_tb_idx);
	if (tr_entry && (IS_NORMAL_STA(tr_entry) || IS_SW_MAIN_STA(tr_entry)))
#endif /* SW_CONNECT_SUPPORT */
	{
		chip_ra_init(ad, entry);

#if defined(CONFIG_RA_PHY_RATE_SUPPORT)
#if defined(DOT11_HE_AX)
		if (wdev->eap.eap_hesuprate_en  &&  IS_HE_STA(entry->cap.modes)) {
			entry->update_he_maxra = true;
			eaprawrapperentryset(ad, entry, &entry->RaEntry);
			RTEnqueueInternalCmd(ad, CMDTHREAD_UPDATE_MAXRA, entry,
					     sizeof(MAC_TABLE_ENTRY));
		}
#endif
#if defined(DOT11_EHT_BE)
		if (wdev->eap.eap_ehtsuprate_en && IS_EHT_STA(entry->cap.modes)) {
			entry->update_eht_maxra = true;
			eaprawrapperentryset(ad, entry, &entry->RaEntry);
			RTEnqueueInternalCmd(ad, CMDTHREAD_UPDATE_MAXRA, entry,
					     sizeof(MAC_TABLE_ENTRY));
		}
#endif
#endif

		/* WiFi Certification config per peer */
		ap_set_wireless_sta_configs(ad, entry);
		/* WiFi Certification config per bss */
		ap_set_wireless_bss_configs(ad, entry->wdev);
	}
	return TRUE;
}

/*
*
*/
INT ap_inf_open(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#ifdef WIFI_UNIFIED_COMMAND
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */


	if (wdev->state)
		return TRUE;

	if (!VALID_MBSS(pAd, wdev->func_idx))
		return FALSE;

	wdev->state = 1;
	/* mbss_idx required for Mac address assignment */
	pAd->ApCfg.MBSSID[wdev->func_idx].mbss_idx = wdev->func_idx;
#ifdef DBDC_ONE_BAND_SUPPORT
	/*dont open second band wdev, if dbdc is set to one band only */
	if (pAd->CommonCfg.DbdcBandSupport) {
		if ((pAd->CommonCfg.DbdcBandSupport == 1) && (WMODE_CAP_5G(wdev->PhyMode))) {
			MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR,
						"skip wdev open for 5G band wdev\n");
			return TRUE;
		} else if ((pAd->CommonCfg.DbdcBandSupport == 2) && (WMODE_CAP_2G(wdev->PhyMode))) {
			MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR,
						"skip wdev open for 5G band wdev\n");
			return TRUE;
		}
	}
#endif /*DBDC_ONE_BAND_SUPPORT*/

#ifdef CONFIG_6G_AFC_SUPPORT
	if (WMODE_CAP_6G(wdev->PhyMode)) {
		afc_init(pAd, wdev);
		afc_check_pre_cond(pAd, wdev);
	}
#endif /*CONFIG_6G_AFC_SUPPORT*/

	if (wifi_sys_open(wdev) != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR, "open fail!!!\n");
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
		"AP inf up for ra_%x(func_idx) OmacIdx=%d\n", wdev->func_idx, wdev->OmacIdx);

	MlmeRadioOn(pAd, wdev);

	/* action for ap interface up */
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
	if (pAd->ApCfg.MBSSID[wdev->func_idx].APStartPseduState != AP_STATE_ALWAYS_START_AP_DEFAULT) {
		wdev->bAllowBeaconing = FALSE;
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "Disable Beaconing\n");
	} else {
		wdev->bAllowBeaconing = TRUE;
	}
#else
	wdev->bAllowBeaconing = TRUE;
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */
	wdev->bRejectAuth = FALSE;
	auto_ch_select_reset_sm(pAd, wdev);
/*bndstrg init need after auto channel selection*/

#ifdef BACKGROUND_SCAN_SUPPORT
	BackgroundScanInit(pAd, wdev);
#endif /* BACKGROUND_SCAN_SUPPORT */
	ap_run_at_boot(pAd, wdev);

	APStartUpForMbss(pAd, &pAd->ApCfg.MBSSID[wdev->func_idx]);
#ifdef CFG_SUPPORT_FALCON_PP
#ifdef DOT11_EHT_BE
	pp_black_list_init(pAd);
	pp_mu_ctrl_init(pAd);
#endif
#endif

	/* Logic to perform OBSS scan for 2.4G only and
	 * one time for all  MBSS configured to same channel.
	 */
	{
	BSS_STRUCT *pMbss = NULL;

	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	/*
	* GBandChanBitMap is used to store the 2.4Ghz channel for which
	* BSS overlap scan is done.
	* Purpose: In case of MBSS, to avoid repeated
	* overlapped scan for the same channel.
	* No need for obss scan if RF up is on 5Ghz
	*/
	if (pMbss && (wlan_config_get_ch_band(&pMbss->wdev) == CMD_CH_BAND_24G)) {
		if (!(pAd->ApCfg.ObssGBandChanBitMap & (1 << pMbss->wdev.channel))) {
			pAd->ApCfg.ObssGBandChanBitMap |= (1 << pMbss->wdev.channel);

			MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
			"wdev->func_idx:%d channel:%d ApCfg.BssidNum:%d \
			 wdev.if_up_down_state:%d GBandChanBitMap:0x%x\n",
			wdev->func_idx, wdev->channel,
			pAd->ApCfg.BssidNum, pMbss->wdev.if_up_down_state,
			pAd->ApCfg.ObssGBandChanBitMap);

			if (pMbss->wdev.if_up_down_state) {
				MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
					"calling ap_over_lapping_scan\n");
				ap_over_lapping_scan(pAd, pMbss);
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
				"2.4G Band CHAN already Set:: GBandChanBitMap:0x%x pMbss->wdev.channel:%d\n",
				pAd->ApCfg.ObssGBandChanBitMap, pMbss->wdev.channel);
			if (pAd->CommonCfg.need_fallback == 1) {
				MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
						"Need fallback to 20 MHz: pMbss->wdev.channel:%d\n",
						 pMbss->wdev.channel);
				wlan_operate_set_ht_bw(&pMbss->wdev, HT_BW_20, EXTCHA_NONE);
			}
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
		"CHAN is not 2G Band: pMbss->wdev.channel:%d\n",
			pMbss->wdev.channel);
	}
	}
#ifdef DOT11_VHT_AC
	if (wlan_config_get_vht_bw_sig(wdev) > BW_SIGNALING_DISABLE) {
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support) {
			UniCmdConfigSetRtsSignalEn(pAd, wdev, TRUE);
			UniCmdConfigSetSchDetDis(pAd, wdev, FALSE);
		} else
#endif
		{
			struct _EXT_CMD_CFG_SET_RTS_SIGTA_EN_T ExtCmdRtsSigTaCfg = {0};
			struct _EXT_CMD_CFG_SET_SCH_DET_DIS_T ExtCmdSchDetDisCfg = {0};

			ExtCmdRtsSigTaCfg.Enable = TRUE;
			CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_RTS_SIGTA_EN_FEATURE, &ExtCmdRtsSigTaCfg);

			ExtCmdSchDetDisCfg.Disable = FALSE;
			CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_SCH_DET_DIS_FEATURE, &ExtCmdSchDetDisCfg);
		}
	}

#endif

#ifdef CONFIG_6G_SUPPORT
	ap_6g_cfg_init(pAd, wdev);
#endif

#ifdef WSC_INCLUDED
	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
			"apidx %d for WscUUIDInit\n", wdev->func_idx);
	WscUUIDInit(pAd, wdev->func_idx, FALSE);
#endif /* WSC_INCLUDED */

#ifdef DPP_SUPPORT
	DlListInit(&wdev->dpp_frame_event_list);
#endif /* DPP_SUPPORT */


	return TRUE;
}

/*
*
*/
INT ap_inf_close(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (!wdev->state)
		return TRUE;
	wdev->state = 0;

	DoActionBeforeDownIntf(pAd, wdev);

	/* Move orig RTMPInfClose here */
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev->bAllowBeaconing = FALSE;
		wdev->bRejectAuth = TRUE;

#ifdef CONFIG_6G_SUPPORT
		ap_6g_cfg_deinit(pAd, wdev);
#ifdef CONFIG_6G_AFC_SUPPORT
		afc_send_daemon_stop_event(pAd, wdev);
#endif
#endif /*CONFIG_6G_AFC_SUPPORT*/

		if (wdev_do_linkdown(wdev) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR,
				"linkdown fail!!!\n");
		}

		if (wifi_sys_close(wdev) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR, "close fail!!!\n");

			DoActionAfterDownIntf(pAd, wdev);
			return FALSE;
		}

#ifdef BAND_STEERING
		if (pAd->ApCfg.BandSteering)
			/* Inform daemon interface down */
			BndStrg_SetInfFlags(pAd, wdev, &pAd->ApCfg.BndStrgTable, FALSE);
#endif /* BAND_STEERING */
		auto_ch_select_reset_sm(pAd, wdev);

	}

	DoActionAfterDownIntf(pAd, wdev);

	return TRUE;
}

BOOLEAN media_state_connected(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad;

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	return (OPSTATUS_TEST_FLAG(ad, fOP_STATUS_MEDIA_STATE_CONNECTED)
			&& OPSTATUS_TEST_FLAG(ad, fOP_AP_STATUS_MEDIA_STATE_CONNECTED));
}

VOID ap_fsm_ops_hook(struct wifi_dev *wdev)
{
	ap_cntl_init(wdev);
	ap_auth_init(wdev);
	ap_assoc_init(wdev);
}

#ifdef CONN_FAIL_EVENT
void ApSendConnFailMsg(
	PRTMP_ADAPTER pAd,
	CHAR *Ssid,
	UCHAR SsidLen,
	UCHAR *StaAddr,
	USHORT ReasonCode)
{
	struct CONN_FAIL_MSG msg;

	if (!pAd)
		return;

	memset(&msg, 0, sizeof(msg));

	if (SsidLen > 32)
		msg.SsidLen = 32;
	else
		msg.SsidLen = SsidLen;

	if (Ssid && (msg.SsidLen > 0))
		memcpy(msg.Ssid, Ssid, msg.SsidLen);

	if (StaAddr)
		memcpy(msg.StaAddr, StaAddr, 6);

	msg.ReasonCode = ReasonCode;

	RtmpOSWrielessEventSend(
		pAd->net_dev,
		RT_WLAN_EVENT_CUSTOM,
		OID_802_11_CONN_FAIL_MSG,
		NULL,
		(UCHAR *)&msg,
		sizeof(msg));

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		"SsidLen=%d, Ssid=%s, StaAddr="MACSTR", ReasonCode=%d\n",
		msg.SsidLen, msg.Ssid, MAC2STR(msg.StaAddr), msg.ReasonCode);
}
#endif

#ifdef DFS_SLAVE_SUPPORT
void slave_bcn_ctrl(PRTMP_ADAPTER pAd, UCHAR en)
{
	struct wifi_dev *wdev = NULL;
	UCHAR bss_idx = 0;
	struct DFS_SLAVE_CTRL *pslave_ctrl = &pAd->slave_ctrl;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
		"[DFS-SLAVE][%s] en:%d\n", __func__, en);

	if ((pslave_ctrl->disable_beacon && en == 0) ||
		(!pslave_ctrl->disable_beacon && en > 0)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"[DFS-SLAVE][%s] Same State en:%d fStopBeacon:%d\n",
			__func__, en, pslave_ctrl->disable_beacon);
		return;
	}

	pslave_ctrl->disable_beacon = (en > 0) ? 0 : 1;

	for (bss_idx = 0; bss_idx < pAd->ApCfg.BssidNum; bss_idx++) {
		wdev = &pAd->ApCfg.MBSSID[bss_idx].wdev;
		if (!wdev->if_up_down_state)
			continue;
		if (wdev->bAllowBeaconing == TRUE) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				"[DFS-SLAVE][%s] wdev_idx:%d en:%d\n", __func__, wdev->wdev_idx, en);
			if (en == 0) {
				/* disconnect all STA */
				if (pslave_ctrl->disconnect_sta)
					MbssKickOutStas(pAd, wdev->func_idx, REASON_DISASSOC_STA_LEAVING);
				/* BH link down stop AP beaconing */
				UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_DISABLE_TX));
			} else {
				/* BH link up, start beaconing */
				if (!wdev->bcn_buf.BeaconPkt)
					UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_INIT));
				else
					UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_ENABLE_TX));
			}
		}
	}
}

void slave_bh_event(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR bh_status)
{
	struct BH_STATUS_MSG *pbhstatus = NULL;
	int ret;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
		"[DFS-SLAVE]send event for band_idx:%d bh_status:%d\n", HcGetBandByWdev(wdev), bh_status);

	os_alloc_mem(pAd, (UCHAR **)&pbhstatus, sizeof(struct BH_STATUS_MSG));

	if (!pbhstatus) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
		"[DFS-SLAVE] !!!!mem alloc failed !!!return!!!!\n");
		return;
	}

	NdisZeroMemory(pbhstatus, sizeof(struct BH_STATUS_MSG));
	pbhstatus->fBhStatus = bh_status;
	NdisCopyMemory(pbhstatus->inf_name, RtmpOsGetNetDevName(wdev->if_dev), IFNAMSIZ);

#ifdef RT_CFG80211_SUPPORT
	ret = mtk_nl80211_bhstatus_event(pAd->pCfg80211_CB->pCfg80211_Wdev->wiphy,
						 pAd->pCfg80211_CB->pCfg80211_Wdev, (void *)pbhstatus, sizeof(struct BH_STATUS_MSG));
	if (ret != 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"Err: event send fail!\n");
	}
#else
	RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM,
					OID_BH_STATUS_MSG, NULL, (PUCHAR)pbhstatus, sizeof(struct BH_STATUS_MSG));
#endif
	os_free_mem(pbhstatus);
}
#endif /* DFS_SLAVE_SUPPORT */
