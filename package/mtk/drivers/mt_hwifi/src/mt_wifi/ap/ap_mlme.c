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
    mlme.c

    Abstract:
    Major MLME state machiones here

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "rt_config.h"
#include <stdarg.h>

#ifdef IXIA_C50_MODE
#endif

#define MCAST_WCID_TO_REMOVE 0 /* Pat: TODO */

#ifdef DOT11_N_SUPPORT

int DetectOverlappingPeriodicRound;


#ifdef DOT11N_DRAFT3
VOID Bss2040CoexistTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	int apidx;
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
		"Recovery to original setting!\n");
	/* Recovery to original setting when next DTIM Interval. */
	pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_TIMER_FIRED);
	NdisZeroMemory(&pAd->CommonCfg.LastBSSCoexist2040, sizeof(BSS_2040_COEXIST_IE));
	pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;

	if (pAd->CommonCfg.bBssCoexEnable == FALSE) {
		/* TODO: Find a better way to handle this when the timer is fired and we disable the bBssCoexEable support!! */
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_INFO,
			"bBssCoexEnable is FALSE, return directly!\n");
		return;
	}

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
		SendBSS2040CoexistMgmtAction(pAd, MCAST_WCID_TO_REMOVE, apidx, 0);
}
#endif /* DOT11N_DRAFT3 */

#endif /* DOT11_N_SUPPORT */


VOID APDetectOverlappingExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
#ifdef DOT11_N_SUPPORT
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;
	struct freq_oper oper;
	BOOLEAN bSupport2G = HcIsRfSupport(pAd, RFIC_24GHZ);
	int i;
	struct wifi_dev *wdev;
	UCHAR cfg_ht_bw;
	UCHAR cfg_ext_cha;

	if (DetectOverlappingPeriodicRound == 0) {
		/* switch back 20/40 */
		if (bSupport2G) {
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				wdev = &pAd->ApCfg.MBSSID[i].wdev;
				cfg_ht_bw = wlan_config_get_ht_bw(wdev);
				if (wmode_2_rfic(wdev->PhyMode) == RFIC_24GHZ && (cfg_ht_bw == HT_BW_40)) {
					cfg_ext_cha = wlan_config_get_ext_cha(wdev);
					wlan_operate_set_ht_bw(wdev, HT_BW_40, cfg_ext_cha);
				}
			}
		}
	} else {
		if ((DetectOverlappingPeriodicRound == 25) || (DetectOverlappingPeriodicRound == 1)) {
			if (hc_radio_query(pAd, &oper) != HC_STATUS_OK) {
				return;
			}
			if (oper.ht_bw == HT_BW_40) {
				SendBeaconRequest(pAd, 1);
				SendBeaconRequest(pAd, 2);
				SendBeaconRequest(pAd, 3);
			}
		}

		DetectOverlappingPeriodicRound--;
	}

#endif /* DOT11_N_SUPPORT */
}

#ifdef WIFI_IAP_BCN_STAT_FEATURE
INT  calculate_beacon_lose(RTMP_ADAPTER *pAd)
{
	PSTA_ADMIN_CONFIG pstacfg = NULL;
	UINT8 idx = 0;
	ULONG now = 0;
	ULONG difftime = 0;
	UINT32 add_loss = 0;
	USHORT beaconperiod = 0;
	UINT8 apclivalid = 0;
	static UINT32 last_add_loss[MAX_MULTI_STA] = {0};
	UINT8 th_bcn_loss = 2;
	struct wifi_dev *wdev = NULL;

	if (!pAd) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"ERROR, pAd = %p;\n", pAd);
		return FALSE;
	}

	for (idx = 0; idx < MAX_MULTI_STA; idx++) {
		pstacfg = &pAd->StaCfg[idx];
		wdev = &pstacfg->wdev;
		apclivalid =  pstacfg->ApcliInfStat.Valid;

		NdisGetSystemUpTime(&now);
		difftime = (UINT32)(now - (pstacfg->LastBeaconRxTime));
		difftime = (UINT32)(difftime * 1000)/OS_HZ;	/*ms*/
		beaconperiod = pstacfg->BeaconPeriod;

		if (wdev && wdev->DevInfo.WdevActive &&
			apclivalid && (difftime > (th_bcn_loss * beaconperiod))) {
			add_loss = (UINT32)(difftime/beaconperiod);
			pstacfg->beacon_loss_count += (UINT32)(add_loss - last_add_loss[idx]);
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			"(%d): beacon_loss = %u; rx_beacon: %lu;\n",
			__LINE__, pstacfg->beacon_loss_count, pstacfg->rx_beacon);

			last_add_loss[idx] = add_loss;
		} else {
			if (!apclivalid) {
				pstacfg->beacon_loss_count = 0;
				pstacfg->rx_beacon = 0;
			}
			last_add_loss[idx] = 0;
		}
	}
	return TRUE;
}
#endif/*WIFI_IAP_BCN_STAT_FEATURE*/


/*
    ==========================================================================
    Description:
	This routine is executed every second -
	1. Decide the overall channel quality
	2. Check if need to upgrade the TX rate to any client
	3. perform MAC table maintenance, including ageout no-traffic clients,
	   and release packet buffer in PSQ is fail to TX in time.
    ==========================================================================
 */
VOID APMlmePeriodicExec(
	PRTMP_ADAPTER pAd)
{
#ifdef A4_CONN
	UCHAR mbss_idx;
#endif
	/*
		Reqeust by David 2005/05/12
		It make sense to disable Adjust Tx Power on AP mode, since we can't
		take care all of the client's situation
		ToDo: need to verify compatibility issue with WiFi product.
	*/
#ifdef VOW_SUPPORT
	vow_display_info_periodic(pAd);
#endif /* VOW_SUPPORT */

	RTMP_CHIP_HIGH_POWER_TUNING(pAd, &pAd->ApCfg.RssiSample);
	RTMP_CHIP_ASIC_TEMPERATURE_COMPENSATION(pAd);

	/* walk through MAC table, see if switching TX rate is required */
	/* MAC table maintenance */
	if (pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE == 0) {
		/* one second timer */
		MacTableMaintenance(pAd);
		/* reset the per second cnt for drop probe rsp usage */
		pAd->probe_rsp_cnt_per_s = 0;
#ifdef WDS_SUPPORT
		WdsTableMaintenance(pAd);
#endif /* WDS_SUPPORT */
#ifdef CCN67_BS_SUPPORT
		Bs_TableMaintain(pAd);
#endif
#ifdef CLIENT_WDS
		CliWds_ProxyTabMaintain(pAd);
#endif /* CLIENT_WDS */
#ifdef A4_CONN
		for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++)
			a4_proxy_maintain(pAd, mbss_idx);
		pAd->a4_need_refresh = FALSE;
#ifdef CONFIG_MAP_3ADDR_SUPPORT
		eth_update_list(pAd);
#endif
#endif /* A4_CONN */
#ifdef WH_EVENT_NOTIFIER
		WHCMlmePeriodicExec(pAd);
#endif /* WH_EVENT_NOTIFIER */

#ifdef WIFI_DIAG
		diag_ap_mlme_one_sec_proc(pAd);
#endif

	}

#ifdef AP_SCAN_SUPPORT
	AutoChannelSelCheck(pAd);
#endif /* AP_SCAN_SUPPORT */
#ifdef APCLI_SUPPORT
#ifdef WIFI_IAP_BCN_STAT_FEATURE
	calculate_beacon_lose(pAd);
#endif/*WIFI_IAP_BCN_STAT_FEATURE*/

	if (pAd->Mlme.OneSecPeriodicRound % 2 == 0)
		ApCliIfMonitor(pAd);

#ifndef APCLI_CFG80211_SUPPORT
	if (pAd->Mlme.OneSecPeriodicRound % 2 == 1)
#if defined(APCLI_AUTO_CONNECT_SUPPORT) || defined(CONFIG_MAP_SUPPORT)
		if (
#ifdef APCLI_AUTO_CONNECT_SUPPORT
				(pAd->ApCfg.ApCliAutoConnectChannelSwitching == FALSE)
#ifdef CONFIG_MAP_SUPPORT
				|| (IS_MAP_TURNKEY_ENABLE(pAd))
#endif /* CONFIG_MAP_SUPPORT */
#else
#ifdef CONFIG_MAP_SUPPORT
				(IS_MAP_TURNKEY_ENABLE(pAd))
#endif /* CONFIG_MAP_SUPPORT */
#endif
		   )
#endif
			ApCliIfUp(pAd);
#endif /* APCLI_CFG80211_SUPPORT */

	{
		INT loop;
		ULONG Now32;
		MAC_TABLE_ENTRY *pEntry;
#ifdef MAC_REPEATER_SUPPORT

		if (pAd->ApCfg.bMACRepeaterEn)
			RTMPRepeaterReconnectionCheck(pAd);

#endif /* MAC_REPEATER_SUPPORT */

		NdisGetSystemUpTime(&Now32);

		for (loop = 0; loop < MAX_APCLI_NUM; loop++) {
			PSTA_ADMIN_CONFIG pApCliEntry = &pAd->StaCfg[loop];


			if ((pApCliEntry->bBlockAssoc == TRUE) &&
				RTMP_TIME_AFTER(Now32, pApCliEntry->LastMicErrorTime + (60*OS_HZ)))
				pApCliEntry->bBlockAssoc = FALSE;


			if ((pApCliEntry->ApcliInfStat.Valid == TRUE)
				&& (VALID_UCAST_ENTRY_WCID(pAd, pApCliEntry->MacTabWCID))) {
				pEntry = entry_get(pAd, pApCliEntry->MacTabWCID);
				/* update channel quality for Roaming and UI LinkQuality display */
				if (pEntry)
					MlmeCalculateChannelQuality(pAd, pEntry, Now32);
			}
		}
	}
#endif /* APCLI_SUPPORT */
#ifdef DOT11_N_SUPPORT
		{
			INT IdBss = 0;
			UCHAR ht_protect_en;
			BSS_STRUCT *pMbss = NULL;

			for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
				pMbss = &pAd->ApCfg.MBSSID[IdBss];
				if (pMbss->wdev.DevInfo.WdevActive) {
					ht_protect_en = wlan_config_get_ht_protect_en(&pMbss->wdev);
					if (ht_protect_en) {
						ApUpdateCapabilityAndErpIe(pAd, pMbss);
						APUpdateOperationMode(pAd, &pMbss->wdev);
					}
				}
			}
		}
#endif /* DOT11_N_SUPPORT */


#ifdef A_BAND_SUPPORT
#ifdef DFS_SLAVE_SUPPORT
	/* When scan for connect going on no need to update */
	/* RDCount as AP is not on current radar channel */
	if ((pAd->CommonCfg.bIEEE80211H == 1) &&
		!(SLAVE_MODE_EN(pAd) && CFG80211DRV_OpsScanRunning(pAd)))
#else
	if (pAd->CommonCfg.bIEEE80211H == 1)
#endif /* DFS_SLAVE_SUPPORT */
	{
		INT IdBss = 0;
#ifdef MT_DFS_SUPPORT
		BOOLEAN BandInCac = FALSE;
#endif /* MT_DFS_SUPPORT */
		BSS_STRUCT *pMbss = NULL;
		struct DOT11_H *pDot11hTest = NULL;
		struct wifi_dev *wdev;
#ifdef MT_DFS_SUPPORT
#ifdef CONFIG_MAP_SUPPORT
		UCHAR bandId = 255;
#endif
#endif

		for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
			pMbss = &pAd->ApCfg.MBSSID[IdBss];
			wdev = &pMbss->wdev;
			if ((pMbss == NULL) || (wdev == NULL) || (wdev->pHObj == NULL))
				continue;

			pDot11hTest = &pAd->Dot11_H;
			if (pDot11hTest == NULL)
				continue;
#ifdef MT_DFS_SUPPORT
#ifdef CONFIG_MAP_SUPPORT
			if (IS_MAP_TURNKEY_ENABLE(pAd) || IS_MAP_BS_ENABLE(pAd)) {
				if (wdev->map_indicate_channel_change && bandId != HcGetBandByWdev(wdev)) {
					if (wdev->map_radar_detect == 1) {
						wdev->map_indicate_channel_change = 0;
						wdev->map_radar_detect = 0;
						wapp_send_ch_change_rsp(pAd, wdev, wdev->channel);
						bandId = HcGetBandByWdev(wdev);
					} else if (wdev->map_radar_detect == 0) {
						wdev->map_indicate_channel_change = 0;
						wapp_send_ch_change_rsp(pAd, wdev, wdev->channel);
						bandId = HcGetBandByWdev(wdev);
					} else if (wdev->map_radar_detect == 2) {
						wdev->map_radar_detect--;
					}
				}
			}
#endif
			if (pDot11hTest->ChannelMode == CHAN_SILENCE_MODE) {
				if (BandInCac == TRUE)
					continue;
				else
					BandInCac = TRUE;

				if (pDot11hTest->RDCount++ > pDot11hTest->cac_time) {
					pDot11hTest->RDCount = 0;
#ifdef CONFIG_MAP_SUPPORT
					if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {

						if (wdev->map_radar_detect == 0)
							wapp_send_ch_change_rsp(pAd, wdev, wdev->channel);

						wapp_send_cac_stop(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), wdev->channel, TRUE);
					}
#endif
#ifdef DFS_ADJ_BW_ZERO_WAIT
					if (pAd->CommonCfg.DfsParameter.BW160ZeroWaitState == DFS_BW160_TX0RX0)
						pAd->CommonCfg.DfsParameter.BW160ZeroWaitState = DFS_BW160_TX160RX160;
					else if (pAd->CommonCfg.DfsParameter.BW160ZeroWaitState == DFS_BW80_TX0RX0)
						pAd->CommonCfg.DfsParameter.BW160ZeroWaitState = DFS_BW80_TX80RX80;
#endif

					pAd->CommonCfg.DfsParameter.cac_channel = wdev->channel;
					MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CAC_END, 0, NULL, HcGetBandByWdev(wdev));
					pDot11hTest->ChannelMode = CHAN_NORMAL_MODE;
					pDot11hTest->InServiceMonitorCount++;
				}
#ifdef DFS_ADJ_BW_ZERO_WAIT
				else if (pDot11hTest->cac_time == CAC_WEATHER_BAND
				&& pDot11hTest->RDCount > CAC_NON_WEATHER_BAND
				&& IS_ADJ_BW_ZERO_WAIT_TX0RX0(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState)
				&& IsChABand(wdev->PhyMode, wdev->channel)) {
					MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CAC_END, 0, NULL, HcGetBandByWdev(wdev));
					pDot11hTest->ChannelMode = CHAN_NORMAL_MODE;
					pDot11hTest->InServiceMonitorCount++;
				}
#endif
			} else
#endif
			{
				pDot11hTest->InServiceMonitorCount++;
#ifdef DFS_ADJ_BW_ZERO_WAIT
				if (BandInCac == TRUE)
					continue;
				else
					BandInCac = TRUE;
				if (IS_ADJ_BW_ZERO_WAIT_TX80RX160(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState))
				{
					if ((pDot11hTest->RDCount++ > pDot11hTest->cac_time) &&
						(IsChABand(wdev->PhyMode, wdev->channel))) {
						pDot11hTest->RDCount = 0;
						MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CAC_END, 0, NULL, HcGetBandByWdev(wdev));
					}
				}
#endif
			}
		}
#ifdef DFS_SLAVE_SUPPORT
		/* In slave only mode as all 5G inf down need to perform CAC on STA inf */
		if (SLAVE_MODE_EN(pAd) && BandInCac == FALSE) {
			UCHAR sta_idx = 0;

			for (sta_idx = 0; sta_idx < MAX_APCLI_NUM; sta_idx++) {
				wdev = &pAd->StaCfg[sta_idx].wdev;
				if ((wdev == NULL) || (wdev->pHObj == NULL))
					continue;

				pDot11hTest = &pAd->Dot11_H;
				if (pDot11hTest == NULL)
					continue;

				if ((pDot11hTest->ChannelMode == CHAN_SILENCE_MODE) &&
					(pDot11hTest->RDCount++ > pDot11hTest->cac_time)) {
					pDot11hTest->RDCount = 0;
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
						"[DFS-SLAVE] STA CAC-END");
					MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CAC_END, 0, NULL, HcGetBandByWdev(wdev));
					pDot11hTest->ChannelMode = CHAN_NORMAL_MODE;
					pDot11hTest->InServiceMonitorCount++;
				}
			}
		}
#endif /* DFS_SLAVE_SUPPORT */
	}
#endif /* A_BAND_SUPPORT */

#ifdef MT_DFS_SUPPORT
	DfsNonOccupancyCountDown(pAd);
	DfsOutBandCacCountUpdate(pAd);
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	DfsV10W56APDownTimeCountDown(pAd);
#endif
#endif
#ifdef MBO_SUPPORT
	MboCheckBssTermination(pAd);
#endif /* MBO_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	FT_R1KHInfoMaintenance(pAd);
#endif /* DOT11R_FT_SUPPORT */
#ifdef BAND_STEERING
	BndStrgHeartBeatMonitor(pAd);
#endif

#ifdef IXIA_C50_MODE
	/*do ixia check if the ixia mode is running*/
	periodic_detect_tx_pkts(pAd);

	/*per chkTmr do ixia check if the traffic is stopped*/
	if ((pAd->Mlme.OneSecPeriodicRound % pAd->ixia_ctl.chkTmr == 0) &&
		pAd->ixia_ctl.iMode == VERIWAVE_MODE) {
		if ((pAd->tx_cnt.txpktdetect < pAd->ixia_ctl.pktthld) &&
			(pAd->rx_cnt.rxpktdetect < pAd->ixia_ctl.pktthld))
			wifi_txrx_parmtrs_dump(pAd);
		else {
			pAd->tx_cnt.txpktdetect = 0;
			pAd->rx_cnt.rxpktdetect = 0;
		}
	}
#endif
}


/*! \brief   To substitute the message type if the message is coming from external
 *  \param  *Fr            The frame received
 *  \param  *Machine       The state machine
 *  \param  *MsgType       the message type for the state machine
 *  \return TRUE if the substitution is successful, FALSE otherwise
 *  \pre
 *  \post
 */
BOOLEAN APMsgTypeSubst(
	IN PRTMP_ADAPTER pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType)
{
	USHORT Seq;
#ifdef DOT11_SAE_SUPPORT
	USHORT Alg;
#endif /* DOT11_SAE_SUPPORT */
#if defined(WSC_AP_SUPPORT) || !defined(RT_CFG80211_SUPPORT)
	UCHAR  EAPType;
#endif
	BOOLEAN     Return = FALSE;
#ifdef WSC_AP_SUPPORT
	UCHAR EAPCode;
	PMAC_TABLE_ENTRY pEntry;
#endif /* WSC_AP_SUPPORT */
	unsigned char hdr_len = LENGTH_802_11;

	if ((pFrame->Hdr.FC.FrDs == 1) && (pFrame->Hdr.FC.ToDs == 1))
		hdr_len = LENGTH_802_11_WITH_ADDR4;
	if (pFrame->Hdr.FC.Order == 1)
		hdr_len += LENGTH_802_11_HTC;
	if (pFrame->Hdr.FC.SubType == SUBTYPE_QDATA)
		hdr_len += LENGTH_802_11_QOS_FIELD;

	/*
		TODO:
		only PROBE_REQ can be broadcast, all others must be unicast-to-me && is_mybssid;
		otherwise, ignore this frame
	*/

	/* wpa EAPOL PACKET */
	if (pFrame->Hdr.FC.Type == FC_TYPE_DATA) {
#ifdef WSC_AP_SUPPORT
		WSC_CTRL *wsc_ctrl;
		struct wifi_dev *wdev = NULL;

		/*WSC EAPOL PACKET */
		pEntry = MacTableLookup(pAd, pFrame->Hdr.Addr2);

		if (pEntry && (pEntry->func_tb_idx < MAX_BEACON_NUM)) {
			wdev = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev;

			if (wdev && (pEntry->bWscCapable
				|| IS_AKM_OPEN(wdev->SecConfig.AKMMap)
				|| IS_AKM_SHARED(wdev->SecConfig.AKMMap)
				|| IS_AKM_AUTOSWITCH(wdev->SecConfig.AKMMap))) {
				/*
					WSC AP only can service one WSC STA in one WPS session.
					Forward this EAP packet to WSC SM if this EAP packets is from
					WSC STA that WSC AP services or WSC AP doesn't service any
					WSC STA now.
				*/
				wsc_ctrl = &wdev->WscControl;

				if ((MAC_ADDR_EQUAL(wsc_ctrl->EntryAddr, pEntry->Addr) ||
					 MAC_ADDR_EQUAL(wsc_ctrl->EntryAddr, ZERO_MAC_ADDR)) &&
					IS_ENTRY_CLIENT(pEntry) &&
					(wsc_ctrl->WscConfMode != WSC_DISABLE)) {
					*Machine = WSC_STATE_MACHINE;
					EAPType = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 1);
					EAPCode = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 4);
					Return = WscMsgTypeSubst(EAPType, EAPCode, MsgType);
				}
			}
		}

#endif /* WSC_AP_SUPPORT */

#ifndef RT_CFG80211_SUPPORT
		if (!Return) {
			*Machine = WPA_STATE_MACHINE;
			EAPType = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 1);
			Return = WpaMsgTypeSubst(EAPType, (INT *) MsgType);
		}
#endif /* RT_CFG80211_SUPPORT */

		return Return;
	}

	if (pFrame->Hdr.FC.Type != FC_TYPE_MGMT)
		return FALSE;

	switch (pFrame->Hdr.FC.SubType) {
	case SUBTYPE_ASSOC_REQ:
		*Machine = ASSOC_FSM;
		*MsgType = ASSOC_FSM_PEER_ASSOC_REQ;
		break;

	/*
	case SUBTYPE_ASSOC_RSP:
		*Machine = ASSOC_FSM;
		*MsgType = APMT2_PEER_ASSOC_RSP;
		break;
	*/
	case SUBTYPE_REASSOC_REQ:
		*Machine = ASSOC_FSM;
		*MsgType = ASSOC_FSM_PEER_REASSOC_REQ;
		break;

	/*
	case SUBTYPE_REASSOC_RSP:
		*Machine = ASSOC_FSM;
		*MsgType = APMT2_PEER_REASSOC_RSP;
		break;
	*/

	case SUBTYPE_BEACON:
		*Machine = SYNC_FSM;
		*MsgType = SYNC_FSM_PEER_BEACON;
		break;

	case SUBTYPE_PROBE_RSP:
		*Machine = SYNC_FSM;
		*MsgType = SYNC_FSM_PEER_PROBE_RSP;
		break;

	case SUBTYPE_PROBE_REQ:
		*Machine = SYNC_FSM;
		*MsgType = SYNC_FSM_PEER_PROBE_REQ;
		break;

	/*
	case SUBTYPE_ATIM:
		*Machine = AP_SYNC_STATE_MACHINE;
		*MsgType = APMT2_PEER_ATIM;
		break;
	*/
	case SUBTYPE_DISASSOC:
		*Machine = ASSOC_FSM;
		*MsgType = ASSOC_FSM_PEER_DISASSOC_REQ;
		break;

	case SUBTYPE_AUTH:
		/* get the sequence number from payload 24 Mac Header + 2 bytes algorithm */
#ifdef DOT11_SAE_SUPPORT
		NdisMoveMemory(&Alg, &pFrame->Octet[0], sizeof(USHORT));
#endif /* DOT11_SAE_SUPPORT */
		NdisMoveMemory(&Seq, &pFrame->Octet[2], sizeof(USHORT));
		*Machine = AUTH_FSM;

		if (Seq == 1
#ifdef DOT11_SAE_SUPPORT
			|| (Alg == AUTH_MODE_SAE && Seq == 2)
#endif /* DOT11_SAE_SUPPORT */
			)
			*MsgType = AUTH_FSM_PEER_AUTH_REQ;
		else if (Seq == 3)
			*MsgType = AUTH_FSM_PEER_AUTH_CONF;
		else {
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
				"wrong AUTH seq=%d Octet=%02x %02x %02x %02x %02x %02x %02x %02x\n", Seq,
				pFrame->Octet[0], pFrame->Octet[1], pFrame->Octet[2], pFrame->Octet[3],
				pFrame->Octet[4], pFrame->Octet[5], pFrame->Octet[6], pFrame->Octet[7]);
			return FALSE;
		}

		break;

	case SUBTYPE_DEAUTH:
		*Machine = AUTH_FSM; /*AP_AUTH_RSP_STATE_MACHINE;*/
		*MsgType = AUTH_FSM_PEER_DEAUTH;
		break;

	case SUBTYPE_ACTION:
	case SUBTYPE_ACTION_NO_ACK:
		*Machine = ACTION_STATE_MACHINE;
		/*  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support */
			if ((pFrame->Octet[0]&0x7F) == CATEGORY_VSP ||
				(pFrame->Octet[0]&0x7F) == CATEGORY_VENDOR_SPECIFIC_WFD)
				*MsgType = MT2_CATEGORY_VSP; /* subtype.*/
			else if ((pFrame->Octet[0]&0x7F) > MAX_PEER_CATE_MSG)
				*MsgType = MT2_ACT_INVALID;
			else {
				if (pFrame->Hdr.FC.Order) {
					/* Bypass HTC 4 bytes to get correct CategoryCode */
					if ((pFrame->Octet[4]&0x7F) > MAX_PEER_CATE_MSG)
						*MsgType = MT2_ACT_INVALID;
					else
						*MsgType = (pFrame->Octet[4]&0x7F);
				} else {
					*MsgType = (pFrame->Octet[0]&0x7F);
				}
			}

		break;

	default:
		return FALSE;
	}

	return TRUE;
}


/*
    ========================================================================
    Routine Description:
	Periodic evaluate antenna link status

    Arguments:
	pAd         - Adapter pointer

    Return Value:
	None

    ========================================================================
*/
VOID APAsicEvaluateRxAnt(
	IN PRTMP_ADAPTER	pAd)
{
	ULONG	TxTotalCnt;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */
	bbp_set_rxpath(pAd, pAd->Antenna.field.RxPath);
	TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount +
				 pAd->RalinkCounters.OneSecTxRetryOkCount +
				 pAd->RalinkCounters.OneSecTxFailCount;

	if (TxTotalCnt > 50) {
		RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 20);
		pAd->Mlme.bLowThroughput = FALSE;
	} else {
		RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 300);
		pAd->Mlme.bLowThroughput = TRUE;
	}
}

/*
    ========================================================================
    Routine Description:
	After evaluation, check antenna link status

    Arguments:
	pAd         - Adapter pointer

    Return Value:
	None

    ========================================================================
*/
VOID APAsicRxAntEvalTimeout(RTMP_ADAPTER *pAd)
{
	CHAR rssi[3], *target_rssi;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */

	/* if the traffic is low, use average rssi as the criteria */
	if (pAd->Mlme.bLowThroughput == TRUE)
		target_rssi = &pAd->ApCfg.RssiSample.LastRssi[0];
	else
		target_rssi = &pAd->ApCfg.RssiSample.AvgRssi[0];

	NdisMoveMemory(&rssi[0], target_rssi, 3);
	/* Disable the below to fix 1T/2R issue. It's suggested by Rory at 2007/7/11. */
	bbp_set_rxpath(pAd, pAd->Mlme.RealRxPath);
}


/*
    ========================================================================
    Routine Description:
	After evaluation, check antenna link status

    Arguments:
	pAd         - Adapter pointer

    Return Value:
	None

    ========================================================================
*/
VOID	APAsicAntennaAvg(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	              AntSelect,
	IN	SHORT * RssiAvg)
{
	SHORT	realavgrssi;
	LONG         realavgrssi1 = 0;
	ULONG	recvPktNum = pAd->RxAnt.RcvPktNum[AntSelect];

	if (AntSelect < ARRAY_SIZE(pAd->RxAnt.Pair1AvgRssiGroup1))
		realavgrssi1 = pAd->RxAnt.Pair1AvgRssiGroup1[AntSelect];
	else
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"AntSelect is out of valid range\n");

	if (realavgrssi1 == 0) {
		*RssiAvg = 0;
		return;
	}

	realavgrssi = (SHORT) (realavgrssi1 / recvPktNum);
	pAd->RxAnt.Pair1AvgRssiGroup1[0] = 0;
	pAd->RxAnt.Pair1AvgRssiGroup1[1] = 0;
	pAd->RxAnt.Pair1AvgRssiGroup2[0] = 0;
	pAd->RxAnt.Pair1AvgRssiGroup2[1] = 0;
	pAd->RxAnt.RcvPktNum[0] = 0;
	pAd->RxAnt.RcvPktNum[1] = 0;
	*RssiAvg = realavgrssi - 256;
}
#ifdef IXIA_C50_MODE
BOOLEAN is_expected_stations(RTMP_ADAPTER *pAd, UINT16 onlinestacnt)
{
	if (pAd->ixia_ctl.iforceIxia) /*Force IXIA mode*/
		return TRUE;

	/*normal ixia test: match the number of stations*/
	if( (onlinestacnt == 5) ||
		(onlinestacnt == 10) ||
		(onlinestacnt == 20) ||
		(onlinestacnt == 40) ||
		(onlinestacnt == 60))
		return TRUE;

	return FALSE;
}
VOID periodic_detect_tx_pkts(RTMP_ADAPTER *pAd)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	INT i;
	CHAR MaxRssi  = -127, MinRssi  = -127, myAvgRssi = -127, deltaRSSI = 0;
	INT maclowbyteMin = 0, maclowbyteMax = 0;
	UCHAR tempAddr[MAC_ADDR_LEN], pollcnt = 0;
	INT maclowbyteSum = 0, tempsum = 0, tempMax = 0;
	UINT32 mac_val = 0;
	UINT16 onlinestacnt = pAd->MacTab->Size;

	/*use the number of stations to simply match the ixia mode*/
	if ((!is_expected_stations(pAd, onlinestacnt)) &&
		(pAd->ixia_ctl.iMode == IXIA_NORMAL_MODE)) {
		return;
	}

	if (!pAd->ixia_ctl.iforceIxia) {
		/*use two specific condition to match  the ixia mode or c50 mode: mac addr and rssi */
		pAd->ixia_ctl.iMacflag = FALSE;
		pAd->ixia_ctl.iRssiflag = FALSE;

		NdisZeroMemory(tempAddr, MAC_ADDR_LEN);
		for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pEntry = entry_get(pAd, i);

			if (!(IS_ENTRY_CLIENT(pEntry) &&
				(pEntry->Sst == SST_ASSOC)))
				continue;
			/*at first, we check the stations' mac addr */
			/*select 1st valid station mac addr as the base addr*/
			if ((maclowbyteMax == 0) && (maclowbyteMin == 0)) {
				COPY_MAC_ADDR(tempAddr, pEntry->Addr);
				maclowbyteMin = (INT)pEntry->Addr[5];
				maclowbyteMax = (INT)pEntry->Addr[5];
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_DEBUG,
					"1st MAC %x:%x:%x:%x:%x:%x.\n", PRINT_MAC(pEntry->Addr));
			}

			/*for the 6th byte or 4th byte of the mac,  find  the minium and maximum*/
			if (NdisEqualMemory(tempAddr, pEntry->Addr, (MAC_ADDR_LEN - 1))) {
				if (maclowbyteMin > (INT)pEntry->Addr[5])
					maclowbyteMin = (INT)pEntry->Addr[5];
				if (maclowbyteMax < (INT)pEntry->Addr[5])
					maclowbyteMax = (INT)pEntry->Addr[5];
				maclowbyteSum += (INT)pEntry->Addr[5];
			} else if (NdisEqualMemory(tempAddr, pEntry->Addr, (MAC_ADDR_LEN - 3)) &&
				NdisEqualMemory(&tempAddr[4], &pEntry->Addr[4], 2)) {
					/* 	00:41:dd:01:00:00
						00:41:dd:02:00:00
						00:41:dd:03:00:00
						00:41:dd:04:00:00
						...
						00:41:dd:0f:00:00
						00:41:dd:10:00:00
						00:41:dd:11:00:00
					*/
				if (maclowbyteMin > (INT)pEntry->Addr[3])
					maclowbyteMin = (INT)pEntry->Addr[3];
				if (maclowbyteMax < (INT)pEntry->Addr[3])
					maclowbyteMax = (INT)pEntry->Addr[3];
				maclowbyteSum += (INT)pEntry->Addr[3];
			} else {
				maclowbyteMin = 0;
				maclowbyteMax = 0;
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_DEBUG,
					"DiffMACDetect %x:%x:%x:%x:%x:%x.\n", PRINT_MAC(pEntry->Addr));
				break;
			}

			/*at second,  we check the stations' rssi */
			/*select 1st valid station's rssi as the base rssi*/
			myAvgRssi = RTMPAvgRssi(pAd, &pEntry->RssiSample); /*get my rssi average*/
			if ((MaxRssi == -127) && (MinRssi == -127)) {
				MaxRssi = myAvgRssi;
				MinRssi = myAvgRssi;
			} else {
				MaxRssi = RTMPMaxRssi(pAd, MaxRssi, myAvgRssi, 0); /* find the max rssi in mactable size.*/
				MinRssi = RTMPMinRssi(pAd, MinRssi, myAvgRssi, 0, 0); /*find the min rssi in mactable size.*/
			}

			pollcnt += 1;
		}

		if (pollcnt != onlinestacnt)	/*to prevent other station is connected*/
			onlinestacnt = pollcnt;

		/*check if the mac info can match the ixia mode*/
		/*Arithmetic Sequence Property:
			Sn = n*(a1 + an)/2, an = a1 + (n -1)*d.
		*/
		tempsum = ((INT)onlinestacnt) * (maclowbyteMax + maclowbyteMin) / 2;
		tempMax = ((INT)onlinestacnt - 1) + maclowbyteMin;	/*Veriwave MAC Address increase by 1, so d=1.*/
		if ((tempsum != 0) &&
			(maclowbyteSum == tempsum) &&
			(maclowbyteMax == tempMax))	/*Arithmetic Sequence and diff is 1.*/
			pAd->ixia_ctl.iMacflag = TRUE;

		/*check if the rssi info can match the ixia mode*/
		deltaRSSI = MaxRssi - MinRssi;

		if ((deltaRSSI < pAd->ixia_ctl.DeltaRssiTh) && (MinRssi >= pAd->ixia_ctl.MinRssiTh))
			pAd->ixia_ctl.iRssiflag = TRUE;
	}

	/*FORCE IXIA MODE or auto detect, default auto detect*/
	if ((pAd->ixia_ctl.iforceIxia) ||
		(pAd->ixia_ctl.iMacflag && pAd->ixia_ctl.iRssiflag)) {
		if (pAd->ixia_ctl.iMode == IXIA_NORMAL_MODE) {
			pAd->ixia_ctl.iMode = VERIWAVE_MODE;

			pAd->ixia_ctl.BA_timeout = (1500 * OS_HZ)/1000; /*flash one time out*/
			pAd->ixia_ctl.max_BA_timeout = (9000 * OS_HZ)/1000; /*flash all time out*/

			if (DebugLevel != 2)
				Set_Debug_Proc(pAd, "2");	/*sometimes, too many logs may decrease the TP*/

			mac_val = 0x04001000;	/* for c50 test: set rts retry times -> unlimited */
			RTMP_IO_WRITE32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_MRCR_ADDR, mac_val);
			RTMP_IO_WRITE32(pAd->hdev_ctrl, BN1_WF_AGG_TOP_MRCR_ADDR, mac_val);

			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_WARN,
				"Enter ixia mode(sta_cnt:%d),iMacflag(%d),iRssiflag(%d).\n",
				onlinestacnt, pAd->ixia_ctl.iMacflag, pAd->ixia_ctl.iRssiflag);

		}
	} else {
		if (pAd->ixia_ctl.iMode == VERIWAVE_MODE) {
			if (onlinestacnt != 0)
				return;
			pAd->ixia_ctl.iMode = IXIA_NORMAL_MODE;

			pAd->ixia_ctl.BA_timeout = (100 * OS_HZ)/1000; /*flash one time out*/
			pAd->ixia_ctl.max_BA_timeout = (1500 * OS_HZ)/1000; /*flash all time out*/

			mac_val = 0x040017c0;	/* for c50 test: set rts retry times -> normal */
			RTMP_IO_WRITE32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_MRCR_ADDR, mac_val);
			RTMP_IO_WRITE32(pAd->hdev_ctrl, BN1_WF_AGG_TOP_MRCR_ADDR, mac_val);

			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_CNTL, DBG_LVL_WARN,
				"leave the ixia mode,sta_cnt(%d),iMacflag(%d),iRssiflag(%d).\n",
				onlinestacnt, pAd->ixia_ctl.iMacflag, pAd->ixia_ctl.iRssiflag);
		}
	}
}
#endif
