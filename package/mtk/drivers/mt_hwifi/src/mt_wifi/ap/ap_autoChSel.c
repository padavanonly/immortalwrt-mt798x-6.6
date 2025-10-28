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

    Abstract:
*/


#include "rt_config.h"
#include "ap_autoChSel.h"
#include "rtmp_type.h"
#ifdef TR181_SUPPORT
#include "hdev/hdev_basic.h"
#endif /*TR181_SUPPORT*/

extern UCHAR ZeroSsid[32];

extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_2GHZ[];
extern UINT16 const Country_Region_GroupNum_2GHZ;
extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_5GHZ[];
extern UINT16 const Country_Region_GroupNum_5GHZ;
#ifdef AP_SCAN_SUPPORT
extern INT scan_ch_restore(RTMP_ADAPTER *pAd, UCHAR OpMode, struct wifi_dev *pwdev);
#endif/*AP_SCAN_SUPPORT*/

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
extern VOID DfsV10AddWeighingFactor(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *pwdev);
#endif

/*final bw for ACS*/
static UCHAR ACS_final_bw = BW_20;

static inline INT GetABandChOffset(
	IN INT Channel)
{
#ifdef A_BAND_SUPPORT

	if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
			(Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157) ||
			(Channel == 165) || (Channel == 173))
		return 1;
	else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) ||
			 (Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161) ||
			 (Channel == 169) || (Channel == 177))
		return -1;


#endif /* A_BAND_SUPPORT */
	return 0;
}

static inline INT Get56GBandChOffset(
	IN INT Channel,
	IN UCHAR ucChBand)
{
	if (ucChBand == WIFI_CH_BAND_5G) {
#ifdef A_BAND_SUPPORT

		if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
			(Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157))
			return 1;
		else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) ||
				 (Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161))
			return -1;

#endif /* A_BAND_SUPPORT */
	}
	else if (ucChBand == WIFI_CH_BAND_6G) {
		if ((Channel >= 1) && (Channel <= 229)) {
			if (Channel % 8 == 1)
				return 1;
			else if (Channel % 8 == 5)
				return -1;
		}
	}

	return 0;
}

ULONG AutoChBssSearchWithSSID(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR Bssid,
	IN PUCHAR pSsid,
	IN UCHAR SsidLen,
	IN UCHAR Channel,
	IN struct wifi_dev *pwdev)
{
	UINT i = 0;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	PBSSINFO pBssInfoTab = pAutoChCtrl->pBssInfoTab;

	if (pBssInfoTab == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "pAd->pBssInfoTab equal NULL.\n");
		return (ULONG)BSS_NOT_FOUND;
	}

	for (i = 0; i < pBssInfoTab->BssNr; i++) {
		if ((((pBssInfoTab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((pBssInfoTab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(&(pBssInfoTab->BssEntry[i].Bssid), Bssid) &&
			(SSID_EQUAL(pSsid, SsidLen, pBssInfoTab->BssEntry[i].Ssid, pBssInfoTab->BssEntry[i].SsidLen) ||
			 (NdisEqualMemory(pSsid, ZeroSsid, SsidLen)) ||
			 (NdisEqualMemory(pBssInfoTab->BssEntry[i].Ssid, ZeroSsid, pBssInfoTab->BssEntry[i].SsidLen))))
			return i;
	}

	return (ULONG)BSS_NOT_FOUND;
}

static inline VOID AutoChBssEntrySet(
	OUT BSSENTRY * pBss,
	IN PUCHAR pBssid,
	IN CHAR Ssid[],
	IN UCHAR SsidLen,
	IN UCHAR Channel,
	IN UCHAR ExtChOffset,
	IN CHAR Rssi)
{
	COPY_MAC_ADDR(pBss->Bssid, pBssid);

	if (SsidLen > 0 && SsidLen <= MAX_LEN_OF_SSID) {
		/*
			For hidden SSID AP, it might send beacon with SSID len equal to 0,
			Or send beacon /probe response with SSID len matching real SSID length,
			but SSID is all zero. such as "00-00-00-00" with length 4.
			We have to prevent this case overwrite correct table
		*/
		if (NdisEqualMemory(Ssid, ZeroSsid, SsidLen) == 0) {
			NdisMoveMemory(pBss->Ssid, Ssid, SsidLen);
			pBss->SsidLen = SsidLen;
		}
	}

	pBss->Channel = Channel;
	pBss->ExtChOffset = ExtChOffset;
	pBss->Rssi = Rssi;
}
VOID UpdatePreACSInfo(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *pwdev)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_MIB_PAIR Reg[UNI_CMD_MIB_MAX_PAIR];
#else /* WIFI_UNIFIED_COMMAND */
	RTMP_MIB_PAIR Reg[7];
#endif /* !WIFI_UNIFIED_COMMAND */
	UCHAR BandIdx = HcGetBandByWdev(pwdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	NdisZeroMemory(Reg, sizeof(Reg));
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support) {
		Reg[0].Counter = UNI_CMD_RMAC_CNT_OBSS_AIRTIME;
		Reg[1].Counter = UNI_CMD_MIB_CNT_TX_DUR_CNT;
		Reg[2].Counter = UNI_CMD_MIB_CNT_RX_DUR_CNT;
		Reg[3].Counter = UNI_CMD_RMAC_CNT_NONWIFI_AIRTIME;
		Reg[4].Counter = UNI_CMD_MIB_CNT_CCA_NAV_TX_TIME;
		Reg[5].Counter = UNI_CMD_MIB_CNT_P_CCA_TIME;
		Reg[6].Counter = UNI_CMD_MIB_CNT_CCA_TIME;
		UniCmdMib(pAd, BandIdx, Reg, 7);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		Reg[0].Counter = RMAC_CNT_OBSS_AIRTIME;/* RMAC.AIRTIME14 OBSS Air time */
		Reg[1].Counter = MIB_CNT_TX_DUR_CNT;/* M0SDR36 TX Air time */
		Reg[2].Counter = MIB_CNT_RX_DUR_CNT;/* M0SDR37 RX Air time */
		Reg[3].Counter = RMAC_CNT_NONWIFI_AIRTIME;/*AIRTIME13 Non Wifi Air time */
		Reg[4].Counter = MIB_CNT_CCA_NAV_TX_TIME;/* M0SDR9 Channel Busy Time */
		Reg[5].Counter = MIB_CNT_P_CCA_TIME;/* M0SDR16 Primary Channel Busy Time */
		MtCmdMultipleMibRegAccessRead(pAd, BandIdx, Reg, 6);
	}
	pAutoChCtrl->AutoChSelCtrl.pre_obss_time = Reg[0].Value;
	pAutoChCtrl->AutoChSelCtrl.pre_tx_air_time = Reg[1].Value;
	pAutoChCtrl->AutoChSelCtrl.pre_rx_air_time = Reg[2].Value;
	pAutoChCtrl->AutoChSelCtrl.pre_non_wifi_time = Reg[3].Value;
	pAutoChCtrl->AutoChSelCtrl.pre_cca_nav_tx_time = Reg[4].Value;
	pAutoChCtrl->AutoChSelCtrl.pre_pcca_time = Reg[5].Value;
	pAutoChCtrl->AutoChSelCtrl.pre_full_pcca_time = Reg[6].Value;
}
INT AutoChSelSendPerChannelInfo(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	int ch_index)
{
	struct EVENTACSINFO event;
	AUTO_CH_CTRL *pAutoChCtrl;

	if (!wdev->if_dev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"Network device is not available.\n");
		return -ENODEV;
	}
	if (ch_index >= MAX_NUM_OF_CHANNELS + 1 || ch_index < 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"Invalid ch_index!\n");
		return -EINVAL;
	}
	NdisZeroMemory(&event, sizeof(event));
	pAutoChCtrl = HcGetAutoChCtrl(pAd);

	event.channel = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ch_index].Channel;
	event.ApCnt = pAutoChCtrl->pChannelInfo->ApCnt[ch_index];
	event.chanbusytime = pAutoChCtrl->pChannelInfo->chanbusytime[ch_index];
	event.pcca_time = pAutoChCtrl->pChannelInfo->pcca_time[ch_index];
	event.full_cca_time = pAutoChCtrl->pChannelInfo->full_cca_time[ch_index];
	event.obss_time = pAutoChCtrl->pChannelInfo->obss_time[ch_index];
	event.tx_air_time = pAutoChCtrl->pChannelInfo->tx_air_time[ch_index];
	event.rx_air_time = pAutoChCtrl->pChannelInfo->rx_air_time[ch_index];
	event.non_wifi_time = pAutoChCtrl->pChannelInfo->non_wifi_time[ch_index];
	event.cca_nav_tx_time = pAutoChCtrl->pChannelInfo->cca_nav_tx_time[ch_index];
#ifdef RT_CFG80211_SUPPORT
	if (nl80211_send_event_ACS_per_channel_info(pAd->pCfg80211_CB->pCfg80211_Wdev->wiphy,
			(PUCHAR)&event, sizeof(struct EVENTACSINFO)) != 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"Error: Failed to send event.\n");
		return -EFAULT;
	}
#endif

	return 0;
}

VOID UpdateChannelInfo(
	IN PRTMP_ADAPTER pAd,
	IN int ch_index,
	IN ChannelSel_Alg Alg,
	IN struct wifi_dev *pwdev)
{
	UCHAR BandIdx = HcGetBandByWdev(pwdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */
#ifdef ACS_CTCC_SUPPORT
	CHANNEL_CTRL *ch_ctrl = NULL;
	INT score = 0;
#endif

	if (ch_index >= MAX_NUM_OF_CHANNELS + 1 || ch_index < 0)
		return;

#ifdef ACS_CTCC_SUPPORT
	ch_ctrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
#endif
	if (pAutoChCtrl->pChannelInfo != NULL) {
		UINT32 BusyTime = 0;
#ifdef WIFI_UNIFIED_COMMAND
		RTMP_MIB_PAIR Reg[UNI_CMD_MIB_MAX_PAIR];
#else /* WIFI_UNIFIED_COMMAND */
		RTMP_MIB_PAIR Reg[7];
#endif /* !WIFI_UNIFIED_COMMAND */
		UINT32 cca_cnt;
		UINT32 current_cca_cnt;

		NdisZeroMemory(Reg, sizeof(Reg));

		current_cca_cnt = AsicGetCCACnt(pAd, BandIdx);
		cca_cnt = (UINT32)(current_cca_cnt - pAutoChCtrl->AutoChSelCtrl.pre_cca_nav_tx_time);
		pAd->RalinkCounters.OneSecFalseCCACnt += cca_cnt;
		pAutoChCtrl->pChannelInfo->cca_nav_tx_time[ch_index] = cca_cnt;
#ifdef OFFCHANNEL_SCAN_FEATURE
		pAd->ChannelInfo.cca_nav_tx_time[ch_index] = cca_cnt;
#endif

		/*
			do busy time statistics for primary channel
			scan time 200ms, beacon interval 100 ms
		*/
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support) {
			Reg[0].Counter = UNI_CMD_RMAC_CNT_OBSS_AIRTIME;
			Reg[1].Counter = UNI_CMD_MIB_CNT_TX_DUR_CNT;
			Reg[2].Counter = UNI_CMD_MIB_CNT_RX_DUR_CNT;
			Reg[3].Counter = UNI_CMD_RMAC_CNT_NONWIFI_AIRTIME;
			Reg[4].Counter = UNI_CMD_MIB_CNT_CCA_NAV_TX_TIME;
			Reg[5].Counter = UNI_CMD_MIB_CNT_P_CCA_TIME;
			Reg[6].Counter = UNI_CMD_MIB_CNT_CCA_TIME;
			UniCmdMib(pAd, BandIdx, Reg, 7);
		} else
#endif /* WIFI_UNIFIED_COMMAND */
		{
			Reg[0].Counter = RMAC_CNT_OBSS_AIRTIME;/* RMAC.AIRTIME14 OBSS Air time */
			Reg[1].Counter = MIB_CNT_TX_DUR_CNT;/* M0SDR36 TX Air time */
			Reg[2].Counter = MIB_CNT_RX_DUR_CNT;/* M0SDR37 RX Air time */
			Reg[3].Counter = RMAC_CNT_NONWIFI_AIRTIME;/*AIRTIME13 Non Wifi Air time */
			Reg[4].Counter = MIB_CNT_CCA_NAV_TX_TIME;/* M0SDR9 Channel Busy Time */
			Reg[5].Counter = MIB_CNT_P_CCA_TIME;/* M0SDR16 Primary Channel Busy Time */
			MtCmdMultipleMibRegAccessRead(pAd, BandIdx, Reg, 6);
		}

		pAutoChCtrl->pChannelInfo->obss_time[ch_index] = (UINT32)(Reg[0].Value - pAutoChCtrl->AutoChSelCtrl.pre_obss_time);
		pAutoChCtrl->pChannelInfo->tx_air_time[ch_index] = (UINT32)(Reg[1].Value - pAutoChCtrl->AutoChSelCtrl.pre_tx_air_time);
		pAutoChCtrl->pChannelInfo->rx_air_time[ch_index] = (UINT32)(Reg[2].Value - pAutoChCtrl->AutoChSelCtrl.pre_rx_air_time);
		pAutoChCtrl->pChannelInfo->non_wifi_time[ch_index] = (UINT32)(Reg[3].Value - pAutoChCtrl->AutoChSelCtrl.pre_non_wifi_time);
		pAutoChCtrl->pChannelInfo->full_cca_time[ch_index] = (UINT32)(Reg[6].Value - pAutoChCtrl->AutoChSelCtrl.pre_full_pcca_time);
		BusyTime = (UINT32)(Reg[5].Value - pAutoChCtrl->AutoChSelCtrl.pre_pcca_time);
		pAutoChCtrl->pChannelInfo->pcca_time[ch_index] = BusyTime;
#ifdef OFFCHANNEL_SCAN_FEATURE
		if (pAd->ScanCtrl.CurrentGivenChan_Index >= MAX_AWAY_CHANNEL) {
			return;
		} else if ((pAd->ScanCtrl.ScanTime[pAd->ScanCtrl.CurrentGivenChan_Index]) != 0) {
			pAd->ScanCtrl.ScanTimeActualEnd = mt_ktime_get();
		}
#endif

#ifdef AP_QLOAD_SUPPORT
		/* QLOAD ALARM, ever alarm from QLOAD module */
	if (QLOAD_DOES_ALARM_OCCUR(pAd))
		pAutoChCtrl->pChannelInfo->chanbusytime[ch_index] = (BusyTime * 100) / AUTO_CHANNEL_SEL_TIMEOUT;
	else
		pAutoChCtrl->pChannelInfo->chanbusytime[ch_index] = (BusyTime * 100) / 200;
#else
		pAutoChCtrl->pChannelInfo->chanbusytime[ch_index] = (BusyTime * 100) / 200;
#endif/* AP_QLOAD_SUPPORT */

#ifdef OFFCHANNEL_SCAN_FEATURE
		pAd->ChannelInfo.chanbusytime[ch_index] = BusyTime;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"channel busy time[%d] = %d\n", ch_index, BusyTime);
		if ((pAd->ScanCtrl.ScanTime[pAd->ScanCtrl.CurrentGivenChan_Index]) != 0) {
			/* Calculate the channel busy value precision by using actual scan time */
			pAd->ScanCtrl.ScanTimeActualDiff = ktime_to_ms(ktime_sub(pAd->ScanCtrl.ScanTimeActualEnd,
			pAd->ScanCtrl.ScanTimeActualStart)) + 1;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "time_diff: %d Busytime: %d\n",
				pAd->ScanCtrl.ScanTimeActualDiff, pAd->ChannelInfo.chanbusytime[ch_index]);
		}
#endif

#ifdef ACS_CTCC_SUPPORT
		score = 100 - BusyTime/(AUTO_CHANNEL_SEL_TIMEOUT * 10);
		if (score < 0)
			score = 0;
		pAutoChCtrl->pChannelInfo->supp_ch_list[ch_index].busy_time = (BusyTime * 100) / AUTO_CHANNEL_SEL_TIMEOUT;
		pAutoChCtrl->pChannelInfo->channel_score[ch_index].score = score;
		pAutoChCtrl->pChannelInfo->channel_score[ch_index].channel = ch_ctrl->ChList[ch_index].Channel;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN, "channel %d busytime %d\n",
			pAutoChCtrl->pChannelInfo->supp_ch_list[ch_index].channel, pAutoChCtrl->pChannelInfo->chanbusytime[ch_index]);
#endif /* ACS_CTCC_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
		if (GetCurrentChannelOpOwner(pAd, pwdev) == CH_OP_OWNER_ACS)
			AutoChSelSendPerChannelInfo(pAd, pwdev, ch_index);
#endif
	} else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "pAutoChCtrl->pChannelInfo equal NULL.\n");
}

static inline INT GetChIdx(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel)
{
	INT Idx;

	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (Idx = 0; Idx < pChCtrl->ChListNum; Idx++) {
		if (Channel == pChCtrl->ChList[Idx].Channel)
			break;
	}

	return Idx;
}

static inline VOID AutoChannelSkipListSetDirty(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR i;
	CHANNEL_CTRL *pChCtrl = NULL;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;
	UCHAR channel_idx = 0;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	pAutoChCtrl = HcGetAutoChCtrl(pAd);

	for (i = 0; i < pAd->ApCfg.AutoChannelSkipListNum; i++) {
		channel_idx = GetChIdx(pAd, pAd->ApCfg.AutoChannelSkipList[i]);
		if (channel_idx >= MAX_NUM_OF_CHANNELS + 1)
			break;

		if (channel_idx != pChCtrl->ChListNum)
			pAutoChCtrl->pChannelInfo->SkipList[channel_idx] = TRUE;
	}
}

static inline BOOLEAN AutoChannelSkipListCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	UCHAR i;
	BOOLEAN result = FALSE;

	for (i = 0; i < pAd->ApCfg.AutoChannelSkipListNum; i++) {
		if (Ch == pAd->ApCfg.AutoChannelSkipList[i]) {
			result = TRUE;
			break;
		}
	}

	return result;
}

static inline BOOLEAN BW40_ChannelCheck(
	IN UCHAR ch)
{
	INT i;
	BOOLEAN result = TRUE;
	UCHAR NorBW40_CH[] = {140, 165};
	UCHAR NorBW40ChNum = sizeof(NorBW40_CH) / sizeof(UCHAR);

	for (i = 0; i < NorBW40ChNum; i++) {
		if (ch == NorBW40_CH[i]) {
			result = FALSE;
			break;
		}
	}

	return result;
}

static inline UCHAR SelectClearChannelRandom(RTMP_ADAPTER *pAd)
{
	UCHAR cnt, ch = 0, i, RadomIdx = 0;
	/*BOOLEAN bFindIt = FALSE;*/
	UINT8 TempChList[MAX_NUM_OF_CHANNELS] = {0};
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	UCHAR random_ChListNum = RandomByte(pAd)%pChCtrl->ChListNum;

	if (random_ChListNum < MAX_NUM_OF_CHANNELS) {
		ch = pChCtrl->ChList[random_ChListNum].Channel;
		if (ch == 0)
			ch = FirstChannel(pAd, wdev);
	} else
		ch = FirstChannel(pAd, wdev);

	if (pAd->CommonCfg.bIEEE80211H) {
		cnt = 0;

		/* Filter out an available channel list */
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			/* Check DFS channel NonOccupancy */
			if (pChCtrl->ChList[i].NonOccupancy)
				continue;
			/* Check skip channel list */
			if (AutoChannelSkipListCheck(pAd, pChCtrl->ChList[i].Channel) == TRUE)
				continue;
#ifdef DOT11_N_SUPPORT
			/* Check N-group of BW40 */
			if (cfg_ht_bw == BW_40 && !(pChCtrl->ChList[i].Flags & CHANNEL_40M_CAP))
				continue;
#endif /* DOT11_N_SUPPORT */
			/* Store available channel to temp list */
			TempChList[cnt++] = pChCtrl->ChList[i].Channel;
		}

		ch = get_channel_by_reference(pAd, 1, &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);

		/* Randomly select a channel from temp list */
		if (cnt) {
			RadomIdx = RandomByte(pAd)%cnt;
			if (RadomIdx < MAX_NUM_OF_CHANNELS)
				ch = TempChList[RadomIdx];
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "Select Channel %d\n", ch);
	return ch;
}

/*
	==========================================================================
	Description:
	This routine calaulates the dirtyness of all channels by the
	CCA value  and Rssi. Store dirtyness to pChannelInfo strcut.
		This routine is called at iwpriv cmmand or initialization. It chooses and returns
		a good channel whith less interference.
	Return:
		ch -  channel number that
	NOTE:
	==========================================================================
 */

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
VOID AutoChannelSkipChannels(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			size,
	IN UINT16			grpStart)
{
	UCHAR i = 0;

	for (i = 0; i < size; i++)
		AutoChannelSkipListAppend(pAd, (grpStart + (i*4)));
}

VOID AutoChannelSkipListClear(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR ChIdx = 0;

	os_zero_mem(pAd->ApCfg.AutoChannelSkipList, 20);
	pAd->ApCfg.AutoChannelSkipListNum = 0;

	for (ChIdx = 0; ChIdx < 20; ChIdx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"Ch = %3d\n", pAd->ApCfg.AutoChannelSkipList[ChIdx]);
	}
}

VOID AutoChannelSkipListAppend(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	if (pAd->ApCfg.AutoChannelSkipListNum < MAX_NUM_OF_CHANNELS + 1
		&& pAd->ApCfg.AutoChannelSkipListNum >= 0) {
		pAd->ApCfg.AutoChannelSkipList[pAd->ApCfg.AutoChannelSkipListNum] = Ch;
		pAd->ApCfg.AutoChannelSkipListNum++;
	}
}

BOOLEAN DfsV10ACSMarkChnlConsumed(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR i = 0;
	BOOLEAN status = FALSE;
	UCHAR channelcount = 0;

	if (IS_DFS_V10_ACS_VALID(pAd) == FALSE)
		return FALSE;

	if (pAd->CommonCfg.bCh144Enabled)
		channelcount = V10_TOTAL_CHANNEL_COUNT;
	else
		channelcount = V10_TOTAL_CHANNEL_COUNT - 1;

	for (i = 0; i < channelcount; i++) {
		if (channel == pDfsParam->DfsV10SortedACSList[i].Channel) {
			pDfsParam->DfsV10SortedACSList[i].isConsumed = TRUE;
			status = TRUE;
			goto done;
		} else
			continue;
	}

	if (status == FALSE)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
		"Channel %d not found\n", channel);
done:
	return status;
}

BOOLEAN DfsV10ACSListSortFunction(
	IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UINT_32 temp_busy = 0;
	UCHAR i = 0, j = 0, temp_chnl = 0;

	if ((!pAd->ApCfg.bAutoChannelAtBootup) && IS_DFS_V10_ACS_VALID(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "False Entry\n");
		return FALSE;
	}

	for (i = 0; i < V10_TOTAL_CHANNEL_COUNT; i++) {
		for (j = i+1; j < V10_TOTAL_CHANNEL_COUNT; j++) {
			if (pDfsParam->DfsV10SortedACSList[i].BusyTime > pDfsParam->DfsV10SortedACSList[j].BusyTime) {
				temp_busy = pDfsParam->DfsV10SortedACSList[i].BusyTime;
				temp_chnl = pDfsParam->DfsV10SortedACSList[i].Channel;

				pDfsParam->DfsV10SortedACSList[i].BusyTime = pDfsParam->DfsV10SortedACSList[j].BusyTime;
				pDfsParam->DfsV10SortedACSList[i].Channel  = pDfsParam->DfsV10SortedACSList[j].Channel;

				pDfsParam->DfsV10SortedACSList[j].BusyTime = temp_busy;
				pDfsParam->DfsV10SortedACSList[j].Channel  = temp_chnl;
			}
		}
	}

	for (i = 0; i < V10_TOTAL_CHANNEL_COUNT; i++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"Channel %d\t", pDfsParam->DfsV10SortedACSList[i].Channel);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"Busy Time %d\t", pDfsParam->DfsV10SortedACSList[i].BusyTime);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"Used %d\n", pDfsParam->DfsV10SortedACSList[i].isConsumed);
	}

	pDfsParam->bV10ChannelListValid = TRUE;

	return TRUE;
}
#endif


/*
	==========================================================================
	Description:
	This routine calaulates the dirtyness of all channels by the dirtiness value and
	number of AP in each channel and stores in pChannelInfo strcut.
		This routine is called at iwpriv cmmand or initialization. It chooses and returns
		a good channel whith less interference.
	Return:
		ch -  channel number that
	NOTE:
	==========================================================================
 */
static inline UCHAR SelectClearChannelApCnt(RTMP_ADAPTER *pAd, struct wifi_dev *pwdev)
{
	/*PBSSINFO pBssInfoTab = pAutoChCtrl->pBssInfoTab; */
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	PCHANNELINFO pChannelInfo = pAutoChCtrl->pChannelInfo;
	/*BSSENTRY *pBss; */
	UCHAR channel_index = 0, dirty, base = 0;
	UCHAR final_channel = 0;
	UCHAR op_ht_bw = wlan_operate_get_ht_bw(pwdev);
	UCHAR ext_cha = wlan_operate_get_ext_cha(pwdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	if (pChannelInfo == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "pAd->pChannelInfo equal NULL.\n");
		return FirstChannel(pAd, pwdev);
	}

	/* Calculate Dirtiness */

	for (channel_index = 0; channel_index < pChCtrl->ChListNum; channel_index++) {
		if (pChannelInfo->ApCnt[channel_index] > 0) {
			INT ll;

			pChannelInfo->dirtyness[channel_index] += 30;

			/*5G */
			if (pChannelInfo->IsABand) {
				if (pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channel_index].CentralChannel == pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channel_index + 1].CentralChannel)
					pChannelInfo->dirtyness[channel_index + 1] += 1;
			}

			/*2.4G */
			if (!pChannelInfo->IsABand) {
				int ChanOffset = 0;

				if ((op_ht_bw == BW_40) &&
						(ext_cha == EXTCHA_BELOW)) {
					/*
						BW is 40Mhz
						the distance between two channel to prevent interference
						is 4 channel width plus 4 channel width (secondary channel)
					*/
					ChanOffset = 8;
				} else {
					/*
						BW is 20Mhz
						The channel width of 2.4G band is 5Mhz.
						The distance between two channel to prevent interference is 4 channel width
					*/
					ChanOffset = 4;
				}

				for (ll = channel_index + 1; ll < (channel_index + ChanOffset + 1); ll++) {
					if (ll < MAX_NUM_OF_CHANNELS)
						pChannelInfo->dirtyness[ll]++;
				}

				if ((op_ht_bw == BW_40) &&
						(ext_cha == EXTCHA_ABOVE)) {
					/* BW is 40Mhz */
					ChanOffset = 8;
				} else {
					/* BW is 20Mhz */
					ChanOffset = 4;
				}

				for (ll = channel_index - 1; ll > (channel_index - ChanOffset - 1); ll--) {
					if (ll >= 0 && ll < MAX_NUM_OF_CHANNELS+1)
						pChannelInfo->dirtyness[ll]++;
				}
			}
		}
	} /* Calculate Dirtiness */

	/* AutoChannelSkipListSetDirty(pAd); */
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE, "=====================================================\n");

	for (channel_index = 0; channel_index < pAutoChCtrl->AutoChSelCtrl.ChListNum; channel_index++)
		/* debug messages */
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE, "Channel %d : Dirty = %ld, ApCnt=%ld, Busy Time = %d, Skip Channel = %s\n",
				 pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channel_index].Channel,
				 pChannelInfo->dirtyness[channel_index],
				 pChannelInfo->ApCnt[channel_index],
#ifdef AP_QLOAD_SUPPORT
				 pChannelInfo->chanbusytime[channel_index],
#else
				 0,
#endif /* AP_QLOAD_SUPPORT */
				 (pChannelInfo->SkipList[channel_index] == TRUE) ? "TRUE" : "FALSE");

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE, "=====================================================\n");
	pAd->ApCfg.AutoChannel_Channel = 0;

	/* RULE 1. pick up a good channel that no one used */

	for (channel_index = 0; channel_index < pAutoChCtrl->AutoChSelCtrl.ChListNum; channel_index++) {
		if (AutoChannelSkipListCheck(pAd, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channel_index].Channel))
			continue;
		if ((pAd->ApCfg.bAvoidDfsChannel == TRUE) && (pChannelInfo->IsABand == TRUE)
			&& RadarChannelCheck(pAd, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channel_index].Channel))
			continue;
		if (!CheckNonOccupancyChannel(pAd, pwdev, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channel_index].Channel))
			continue;

#ifdef AP_QLOAD_SUPPORT

		/* QLOAD ALARM */
		if (QBSS_LoadIsBusyTimeAccepted(pAd,
										pChannelInfo->chanbusytime[channel_index]) == FALSE)
			continue;

#endif /* AP_QLOAD_SUPPORT */

		if (pChannelInfo->dirtyness[channel_index] == 0)
			break;
	}

	if (channel_index < pAutoChCtrl->AutoChSelCtrl.ChListNum) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "Rule 1 APCnt : dirtiness == 0 (no one used and no interference) ==> Select Channel %d\n", pChCtrl->ChList[channel_index].Channel);
		return pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channel_index].Channel;
	}

	/* RULE 2. if not available, then co-use a channel that's no interference (dirtyness=30) */
	/* RULE 3. if not available, then co-use a channel that has minimum interference (dirtyness=31,32) */
	for (dirty = 30; dirty <= 32; dirty++) {
		BOOLEAN candidate[MAX_NUM_OF_CHANNELS+1], candidate_num = 0;
		UCHAR min_ApCnt = 255;

		final_channel = 0;
		NdisZeroMemory(candidate, MAX_NUM_OF_CHANNELS+1);

		for (channel_index = 0; channel_index < pAutoChCtrl->AutoChSelCtrl.ChListNum; channel_index++) {
			if (pChannelInfo->SkipList[channel_index] == TRUE)
				continue;

			if (pChannelInfo->dirtyness[channel_index] == dirty) {
				candidate[channel_index] = TRUE;
				candidate_num++;
			}
		}

		/* if there's more than 1 candidate, pick up the channel with minimum RSSI */
		if (candidate_num) {
			for (channel_index = 0; channel_index < pAutoChCtrl->AutoChSelCtrl.ChListNum; channel_index++) {
#ifdef AP_QLOAD_SUPPORT

				/* QLOAD ALARM */
				/* when busy time of a channel > threshold, skip it */
				/* TODO: Use weight for different references to do channel selection */
				if (QBSS_LoadIsBusyTimeAccepted(pAd,
												pChannelInfo->chanbusytime[channel_index]) == FALSE) {
					/* check next one */
					continue;
				}

#endif /* AP_QLOAD_SUPPORT */

				if (candidate[channel_index] && (pChannelInfo->ApCnt[channel_index] < min_ApCnt)) {
					if ((pAd->ApCfg.bAvoidDfsChannel == TRUE) && (pChannelInfo->IsABand == TRUE)
						&& RadarChannelCheck(pAd, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channel_index].Channel))
						continue;

					final_channel = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channel_index].Channel;
					min_ApCnt = pChannelInfo->ApCnt[channel_index];
				}
			}

			if (final_channel != 0) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
						"Rule 2 APCnt : minimum APCnt with  minimum interference(dirtiness: 30~32) ==> Select Channel %d\n", final_channel);
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE, " Dirtiness = %d ,  Min ApCnt = %d\n", dirty, min_ApCnt);
				return final_channel;
			}
		}
	}

	/* RULE 3. still not available, pick up the random channel */
	base = RandomByte(pAd);

	for (channel_index = 0; channel_index < pAutoChCtrl->AutoChSelCtrl.ChListNum; channel_index++) {
		final_channel = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[(base + channel_index) % pAutoChCtrl->AutoChSelCtrl.ChListNum].Channel;

		if (AutoChannelSkipListCheck(pAd, final_channel))
			continue;

		if ((pAd->ApCfg.bAvoidDfsChannel == TRUE) && (pChannelInfo->IsABand == TRUE)
			&& RadarChannelCheck(pAd, final_channel))
			continue;
		if (!CheckNonOccupancyChannel(pAd, pwdev, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channel_index].Channel))
			continue;

		break;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "Rule 3 APCnt : Randomly Select  ==> Select Channel %d\n", final_channel);
	return final_channel;
}

ULONG AutoChBssInsertEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pBssid,
	IN CHAR Ssid[],
	IN UCHAR SsidLen,
	IN UCHAR ChannelNo,
	IN UCHAR ExtChOffset,
	IN CHAR Rssi,
	IN struct wifi_dev *pwdev)
{
	ULONG	Idx;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	PBSSINFO pBssInfoTab = pAutoChCtrl->pBssInfoTab;

	if (pBssInfoTab == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "pAd->pBssInfoTab equal NULL.\n");
		return BSS_NOT_FOUND;
	}

	Idx = AutoChBssSearchWithSSID(pAd, pBssid, (PUCHAR)Ssid, SsidLen, ChannelNo, pwdev);

	if (Idx == BSS_NOT_FOUND) {
		Idx = pBssInfoTab->BssNr;
		if (Idx >= MAX_LEN_OF_BSS_TABLE)
			return BSS_NOT_FOUND;

		AutoChBssEntrySet(&pBssInfoTab->BssEntry[Idx % MAX_LEN_OF_BSS_TABLE], pBssid, Ssid, SsidLen,
						  ChannelNo, ExtChOffset, Rssi);
		pBssInfoTab->BssNr++;
	} else {
		AutoChBssEntrySet(&pBssInfoTab->BssEntry[Idx % MAX_LEN_OF_BSS_TABLE], pBssid, Ssid, SsidLen,
						  ChannelNo, ExtChOffset, Rssi);
	}

	return Idx;
}

void AutoChBssTableDestroy(RTMP_ADAPTER *pAd)
{
	AUTO_CH_CTRL *pAutoChCtrl;

	pAutoChCtrl = HcGetAutoChCtrl(pAd);

	if (!pAutoChCtrl) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
				 "pAutoChCtrl is NULL.\n");
		return;
	}

	if (pAutoChCtrl->pBssInfoTab) {
		os_free_mem(pAutoChCtrl->pBssInfoTab);
		pAutoChCtrl->pBssInfoTab = NULL;
	}
}


static VOID AutoChBssTableReset(RTMP_ADAPTER *pAd)
{
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	PBSSINFO pBssInfoTab = pAutoChCtrl->pBssInfoTab;

	if (pBssInfoTab)
		NdisZeroMemory(pBssInfoTab, sizeof(BSSINFO));
	else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
				 "pAutoChCtrl->pBssInfoTab equal NULL.\n");
	}
}


void AutoChBssTableInit(
	IN PRTMP_ADAPTER pAd)
{
	BSSINFO *pBssInfoTab = NULL;
	AUTO_CH_CTRL *pAutoChCtrl;

	pAutoChCtrl = HcGetAutoChCtrl(pAd);
	os_alloc_mem(pAd, (UCHAR **)&pBssInfoTab, sizeof(BSSINFO));

	if (pBssInfoTab) {
		NdisZeroMemory(pBssInfoTab, sizeof(BSSINFO));
		pAutoChCtrl->pBssInfoTab = pBssInfoTab;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
					 "Fail to alloc memory for pAutoChCtrl->pBssInfoTab");
	}
}


void ChannelInfoDestroy(
	IN PRTMP_ADAPTER pAd)
{
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);

	if (pAutoChCtrl->pChannelInfo) {
		os_free_mem(pAutoChCtrl->pChannelInfo);
		pAutoChCtrl->pChannelInfo = NULL;
	}
}


static VOID ChannelInfoReset(RTMP_ADAPTER *pAd)
{
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	CHANNELINFO *ch_info = pAutoChCtrl->pChannelInfo;

	if (ch_info)
		NdisZeroMemory(ch_info, sizeof(CHANNELINFO));
	else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
				 "pChannelInfo equal NULL\n");
}


void ChannelInfoInit(
	IN PRTMP_ADAPTER pAd)
{
	CHANNELINFO *ch_info = NULL;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);

	ch_info = NULL;
	os_alloc_mem(pAd, (UCHAR **)&ch_info, sizeof(CHANNELINFO));

	if (ch_info) {
		os_zero_mem(ch_info, sizeof(CHANNELINFO));
		pAutoChCtrl->pChannelInfo = ch_info;
	} else {
		pAutoChCtrl->pChannelInfo = NULL;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
					"Fail to alloc memory for pAd->pChannelInfo");
	}
}

#ifdef ACS_CTCC_SUPPORT
VOID build_acs_scan_ch_list(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev)
{
	UCHAR channel_idx = 0;
	UCHAR ch_list_num = 0;
	UCHAR ch = 0;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	UCHAR op_ext_cha = wlan_config_get_ext_cha(wdev);
	CHANNEL_CTRL *ch_ctrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	AUTO_CH_CTRL *auto_ch_ctrl = NULL;
	UCHAR ch_band = wlan_config_get_ch_band(wdev);
	auto_ch_ctrl = HcGetAutoChCtrl(pAd);

	if (ch_band == CMD_CH_BAND_6G)
		return;

	AutoChannelSkipListSetDirty(pAd);
	if (ch_band == CMD_CH_BAND_5G) {
		for (channel_idx = 0; channel_idx < ch_ctrl->ChListNum; channel_idx++) {
			ch = ch_ctrl->ChList[channel_idx].Channel;
			auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].ap_cnt = auto_ch_ctrl->pChannelInfo->ApCnt[ch_list_num];
			if ((cfg_ht_bw == BW_20) || (wlan_config_get_vht_bw(wdev) == VHT_BW_160)) {
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].channel = ch_ctrl->ChList[channel_idx].Channel;
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel = ch_ctrl->ChList[channel_idx].Channel;
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].dfs_req = ch_ctrl->ChList[channel_idx].DfsReq;
				if (auto_ch_ctrl->pChannelInfo->SkipList[channel_idx] == TRUE)
				    auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].skip_channel = TRUE;
				ch_list_num++;
			}
#ifdef DOT11_N_SUPPORT
			else if ((cfg_ht_bw == BW_40)
#ifdef DOT11_VHT_AC
				&& (wlan_config_get_vht_bw(wdev) == VHT_BW_2040)
#endif /* DOT11_VHT_AC */
				) {
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].channel = ch_ctrl->ChList[channel_idx].Channel;
				if (N_ChannelGroupCheck(pAd, ch, wdev)) {
					if (GetABandChOffset(ch) == 1)
						auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel =
						ch_ctrl->ChList[channel_idx].Channel + 2;
					else
						auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel =
						ch_ctrl->ChList[channel_idx].Channel - 2;
				} else
					auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel =
						ch_ctrl->ChList[channel_idx].Channel;
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].dfs_req = ch_ctrl->ChList[channel_idx].DfsReq;
				if (auto_ch_ctrl->pChannelInfo->SkipList[channel_idx] == TRUE)
				    auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].skip_channel = TRUE;
				ch_list_num++;
			}
#ifdef DOT11_VHT_AC
			else if (wlan_config_get_vht_bw(wdev) == VHT_BW_80) {
				UCHAR ch_band = wlan_config_get_ch_band(wdev);
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].channel = ch_ctrl->ChList[channel_idx].Channel;
				if (vht80_channel_group(pAd, ch, wdev))
					auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel =
					vht_cent_ch_freq(ch, VHT_BW_80, ch_band);
				else
					auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel =
					ch_ctrl->ChList[channel_idx].Channel;
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].dfs_req = ch_ctrl->ChList[channel_idx].DfsReq;
				if (auto_ch_ctrl->pChannelInfo->SkipList[channel_idx] == TRUE)
					auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].skip_channel = TRUE;
				ch_list_num++;
			}
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
		}
	} else {
		for (channel_idx = 0; channel_idx < ch_ctrl->ChListNum; channel_idx++) {
			if (cfg_ht_bw == BW_40) {
				if (op_ext_cha == EXTCHA_ABOVE)
					auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel = ch_ctrl->ChList[channel_idx].Channel + 2;
				else {
					if (auto_ch_ctrl->pChannelInfo->supp_ch_list[channel_idx].channel == 14)
						auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel = ch_ctrl->ChList[channel_idx].Channel - 1;
					else
						auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel = ch_ctrl->ChList[channel_idx].Channel - 2;
				}
			} else
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel = ch_ctrl->ChList[channel_idx].Channel;
			auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].channel = ch_ctrl->ChList[channel_idx].Channel;
			if (auto_ch_ctrl->pChannelInfo->SkipList[channel_idx] == TRUE)
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].skip_channel = TRUE;
			ch_list_num++;
		}
	}
	auto_ch_ctrl->pChannelInfo->channel_list_num = ch_list_num;
	for (channel_idx = 0; channel_idx < auto_ch_ctrl->pChannelInfo->channel_list_num; channel_idx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "Support channel: PrimCh=%d, CentCh=%d, DFS=%d, skip %d\n",
			auto_ch_ctrl->pChannelInfo->supp_ch_list[channel_idx].channel, auto_ch_ctrl->pChannelInfo->supp_ch_list[channel_idx].cen_channel,
			auto_ch_ctrl->pChannelInfo->supp_ch_list[channel_idx].dfs_req, auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].skip_channel);
	}
}

UINT8 acs_group_ch_list_search(
	AUTO_CH_CTRL *auto_ch_ctrl,
	UCHAR cen_channel)
{
	UCHAR i;
	struct acs_scan_ch_group_list *group_ch_list = auto_ch_ctrl->pChannelInfo->group_ch_list;

	for (i = 0; i < auto_ch_ctrl->pChannelInfo->group_ch_list_num; i++)	{
		if (group_ch_list->cen_channel == cen_channel)
			return i;
		group_ch_list++;
	}

	return 0xff;
}

VOID acs_group_ch_list_insert(
	RTMP_ADAPTER * pAd,
	AUTO_CH_CTRL *auto_ch_ctrl,
	struct acs_scan_supp_ch_list *source,
	IN struct wifi_dev *wdev)
{
	UCHAR i = auto_ch_ctrl->pChannelInfo->group_ch_list_num;
	UCHAR j = 0;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	UCHAR cfg_vht_bw = wlan_config_get_vht_bw(wdev);
	struct acs_scan_ch_group_list *group_ch_list = &auto_ch_ctrl->pChannelInfo->group_ch_list[i];

	group_ch_list->best_ctrl_channel = source->channel;
	group_ch_list->cen_channel = source->cen_channel;
	group_ch_list->max_busy_time = source->busy_time;
	group_ch_list->min_busy_time = source->busy_time;
	group_ch_list->skip_group = source->skip_channel;
	if (cfg_vht_bw == VHT_BW_80) {
		if (group_ch_list->bw80_grp_ch_member_idx >= 4 || group_ch_list->bw80_grp_ch_member_idx < 0)
			return;
		group_ch_list->bw80_grp_ch_member[group_ch_list->bw80_grp_ch_member_idx].channel = source->channel;
		group_ch_list->bw80_grp_ch_member[group_ch_list->bw80_grp_ch_member_idx].busy_time = source->busy_time;
		group_ch_list->bw80_grp_ch_member_idx++;
	} else if ((cfg_ht_bw == BW_40) && (cfg_vht_bw == VHT_BW_2040)) {
		if (group_ch_list->bw40_grp_ch_member_idx >= 2 || group_ch_list->bw40_grp_ch_member_idx < 0)
			return;
		group_ch_list->bw40_grp_ch_member[group_ch_list->bw40_grp_ch_member_idx].channel = source->channel;
		group_ch_list->bw40_grp_ch_member[group_ch_list->bw40_grp_ch_member_idx].busy_time = source->busy_time;
		group_ch_list->bw40_grp_ch_member_idx++;
	} else {
	}

	for (j = 0; j < auto_ch_ctrl->pChannelInfo->channel_list_num; j++) {
		if (source->channel == auto_ch_ctrl->pChannelInfo->channel_score[j].channel) {
			group_ch_list->grp_score = auto_ch_ctrl->pChannelInfo->channel_score[j].score;
		}
	}

	auto_ch_ctrl->pChannelInfo->group_ch_list_num = i + 1;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "Insert new group channel list Number=%d cen_channel=%d"
		" best_ctrl_channel=%d BUSY_TIEM=%d, skip_group=%d, grp_score=%d\n",
		auto_ch_ctrl->pChannelInfo->group_ch_list_num, group_ch_list->cen_channel,
		group_ch_list->best_ctrl_channel, group_ch_list->max_busy_time, group_ch_list->skip_group, group_ch_list->grp_score);
}

VOID acs_group_ch_list_update(
	RTMP_ADAPTER * pAd,
	AUTO_CH_CTRL *auto_ch_ctrl,
	UCHAR index,
	struct acs_scan_supp_ch_list *source,
	IN struct wifi_dev *wdev)
{
	UCHAR i = 0;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	UCHAR cfg_vht_bw = wlan_config_get_vht_bw(wdev);
	struct acs_scan_ch_group_list *group_ch_list = NULL;

	if (index < 0 && index >= MAX_NUM_OF_CHANNELS + 1)
		return;
	group_ch_list = &auto_ch_ctrl->pChannelInfo->group_ch_list[index];

	if (source->busy_time > group_ch_list->max_busy_time) {
		group_ch_list->max_busy_time = source->busy_time;
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (source->channel == auto_ch_ctrl->pChannelInfo->channel_score[i].channel) {
				group_ch_list->grp_score = auto_ch_ctrl->pChannelInfo->channel_score[i].score;
			}
		}
	}

	if (source->busy_time < group_ch_list->min_busy_time) {
		group_ch_list->min_busy_time = source->busy_time;
		group_ch_list->best_ctrl_channel = source->channel;
	}

	if (group_ch_list->skip_group == 0 && source->skip_channel == 1)
		group_ch_list->skip_group = source->skip_channel;

	if (cfg_vht_bw == VHT_BW_80) {
		if (group_ch_list->bw80_grp_ch_member_idx >= 4 || group_ch_list->bw80_grp_ch_member_idx < 0)
			return;
		group_ch_list->bw80_grp_ch_member[group_ch_list->bw80_grp_ch_member_idx].channel = source->channel;
		group_ch_list->bw80_grp_ch_member[group_ch_list->bw80_grp_ch_member_idx].busy_time = source->busy_time;
		group_ch_list->bw80_grp_ch_member_idx++;
	} else if ((cfg_ht_bw == BW_40) && (cfg_vht_bw == VHT_BW_2040)) {
		if (group_ch_list->bw40_grp_ch_member_idx >= 2 || group_ch_list->bw40_grp_ch_member_idx < 0)
			return;
		group_ch_list->bw40_grp_ch_member[group_ch_list->bw40_grp_ch_member_idx].channel = source->channel;
		group_ch_list->bw40_grp_ch_member[group_ch_list->bw40_grp_ch_member_idx].busy_time = source->busy_time;
		group_ch_list->bw40_grp_ch_member_idx++;
	} else {
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "Update group channel list index=%d cen_channel=%d "
		"best_ctrl_channel=%d BUSY_TIEM=%d skip_group=%d, grp_score=%d\n",
		auto_ch_ctrl->pChannelInfo->group_ch_list_num, group_ch_list->cen_channel,
		group_ch_list->best_ctrl_channel, group_ch_list->max_busy_time, group_ch_list->skip_group, group_ch_list->grp_score);
}

VOID acs_generate_group_channel_list(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev)
{
	UCHAR i = 0;
	UCHAR list_index = 0;
	AUTO_CH_CTRL *auto_ch_ctrl = NULL;
	struct acs_scan_supp_ch_list *supp_ch_list = NULL;

	auto_ch_ctrl = HcGetAutoChCtrl(pAd);
	supp_ch_list = &auto_ch_ctrl->pChannelInfo->supp_ch_list[0];
	memset(auto_ch_ctrl->pChannelInfo->group_ch_list, 0, (MAX_NUM_OF_CHANNELS+1) * sizeof(struct acs_scan_ch_group_list));
	auto_ch_ctrl->pChannelInfo->group_ch_list_num = 0;

	for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
		list_index = acs_group_ch_list_search(auto_ch_ctrl, supp_ch_list->cen_channel);
		if (list_index == 0xff) {
			acs_group_ch_list_insert(pAd, auto_ch_ctrl, supp_ch_list, wdev);
		} else {
			acs_group_ch_list_update(pAd, auto_ch_ctrl, list_index, supp_ch_list, wdev);
		}

		supp_ch_list++;
	}
}

UCHAR find_best_channel_of_all_grp(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev)
{
	UINT32 i = 0;
	UINT32 j = 0;
	UINT32 k = 0;
	INT l = 0;
	UINT32 min_busy = 0xffffffff;
	UINT32 max_busy = 0x0;
	UINT32 min_score = 0x0;
	UINT32 busy = 0;
	UINT32 best_ch_score = 0;
	UCHAR best_channel = 0;
	AUTO_CH_CTRL *auto_ch_ctrl = HcGetAutoChCtrl(pAd);
	CHANNEL_CTRL *ch_ctrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	struct auto_ch_sel_grp_member tmp;
	struct acs_scan_ch_group_list *acs_grp_ch_list = NULL;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	UCHAR cfg_vht_bw = wlan_config_get_vht_bw(wdev);

	for (i = 0; i < auto_ch_ctrl->pChannelInfo->group_ch_list_num; i++) {
		acs_grp_ch_list = &auto_ch_ctrl->pChannelInfo->group_ch_list[i];

		if (cfg_vht_bw == VHT_BW_80) {
			for (k = 0; k < 3; k++) {
				for (j = 0; j < 3 - k; j++) {
					if (acs_grp_ch_list->bw80_grp_ch_member[j].busy_time > acs_grp_ch_list->bw80_grp_ch_member[j+1].busy_time) {
						tmp.busy_time = acs_grp_ch_list->bw80_grp_ch_member[j+1].busy_time;
						tmp.channel =  acs_grp_ch_list->bw80_grp_ch_member[j+1].channel;
						acs_grp_ch_list->bw80_grp_ch_member[j+1].busy_time = acs_grp_ch_list->bw80_grp_ch_member[j].busy_time;
						acs_grp_ch_list->bw80_grp_ch_member[j+1].channel = acs_grp_ch_list->bw80_grp_ch_member[j].channel;
						acs_grp_ch_list->bw80_grp_ch_member[j].busy_time = tmp.busy_time;
						acs_grp_ch_list->bw80_grp_ch_member[j].channel = tmp.channel;
					}
				}
			}

			if (acs_grp_ch_list->bw80_grp_ch_member[0].channel == 0) {
				acs_grp_ch_list->bw80_not_allowed = TRUE;
			}
		} else if ((cfg_ht_bw == BW_40) && (cfg_vht_bw == VHT_BW_2040)) {
			if (acs_grp_ch_list->bw40_grp_ch_member[0].busy_time > acs_grp_ch_list->bw40_grp_ch_member[1].busy_time) {
				tmp.busy_time = acs_grp_ch_list->bw40_grp_ch_member[1].busy_time;
				tmp.channel =  acs_grp_ch_list->bw40_grp_ch_member[1].channel;
				acs_grp_ch_list->bw40_grp_ch_member[1].busy_time = acs_grp_ch_list->bw40_grp_ch_member[0].busy_time;
				acs_grp_ch_list->bw40_grp_ch_member[1].channel = acs_grp_ch_list->bw40_grp_ch_member[0].channel;
				acs_grp_ch_list->bw40_grp_ch_member[0].busy_time = tmp.busy_time;
				acs_grp_ch_list->bw40_grp_ch_member[0].channel = tmp.channel;
			}

			if (acs_grp_ch_list->bw40_grp_ch_member[0].channel == 0) {
				acs_grp_ch_list->bw40_not_allowed = TRUE;
			}
		} else {
		}

		if (auto_ch_ctrl->pChannelInfo->group_ch_list[i].skip_group == FALSE) {
			busy = auto_ch_ctrl->pChannelInfo->group_ch_list[i].max_busy_time;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "ChIdx=%d control-Channle=%d cen-channel=%d Max_BUSY_TIME=%d\n",
				i, auto_ch_ctrl->pChannelInfo->group_ch_list[i].best_ctrl_channel, auto_ch_ctrl->pChannelInfo->group_ch_list[i].cen_channel,
				auto_ch_ctrl->pChannelInfo->group_ch_list[i].max_busy_time);

			if ((busy <= min_busy) &&
				(((cfg_vht_bw == VHT_BW_80) && (acs_grp_ch_list->bw80_not_allowed == FALSE) && (auto_ch_ctrl->pChannelInfo->group_ch_list[i].bw80_grp_ch_member_idx == 4))
				|| ((cfg_ht_bw == BW_40) && (cfg_vht_bw == VHT_BW_2040) && (acs_grp_ch_list->bw40_not_allowed == FALSE)
					&& (auto_ch_ctrl->pChannelInfo->group_ch_list[i].bw40_grp_ch_member_idx == 2)))) {
				min_busy = busy;
				best_channel = auto_ch_ctrl->pChannelInfo->group_ch_list[i].best_ctrl_channel;
			}

			if (busy > max_busy) {
				max_busy = busy;
				min_score = 100 - max_busy/1000;
				if (min_score < 0)
					min_score = 0;
			}
	    }
	}

	for (i = 0; i < auto_ch_ctrl->pChannelInfo->group_ch_list_num; i++) {
		for (j = 0; j < auto_ch_ctrl->pChannelInfo->channel_list_num; j++) {
			if (auto_ch_ctrl->pChannelInfo->group_ch_list[i].best_ctrl_channel == auto_ch_ctrl->pChannelInfo->supp_ch_list[j].channel) {
				if (cfg_vht_bw == VHT_BW_80) {
					for (k = 0; k < 4; k++) {
						for (l = -3; l < 4; l++) {
							if ((j+l < 0) || (j+l >= auto_ch_ctrl->pChannelInfo->channel_list_num))
								continue;
							if (auto_ch_ctrl->pChannelInfo->channel_score[j+l].channel == auto_ch_ctrl->pChannelInfo->group_ch_list[i].bw80_grp_ch_member[k].channel) {
								auto_ch_ctrl->pChannelInfo->channel_score[j+l].score = auto_ch_ctrl->pChannelInfo->group_ch_list[i].grp_score + 3 - k;
								if ((auto_ch_ctrl->pChannelInfo->group_ch_list[i].bw80_not_allowed == TRUE) ||
									(vht80_channel_group(pAd, auto_ch_ctrl->pChannelInfo->channel_score[j+l].channel, wdev) == FALSE) ||
									(auto_ch_ctrl->pChannelInfo->group_ch_list[i].bw80_grp_ch_member_idx != 4)) {
									auto_ch_ctrl->pChannelInfo->channel_score[j+l].score = min_score;
									MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "Channel %d don't support BW80, force to assign min score(%d)!!!!!!\n",
										auto_ch_ctrl->pChannelInfo->channel_score[j+l].channel, auto_ch_ctrl->pChannelInfo->channel_score[j+l].score);
								}
								if (auto_ch_ctrl->pChannelInfo->channel_score[j+l].score > 100)
									auto_ch_ctrl->pChannelInfo->channel_score[j+l].score = 100;
								break;
							}
						}
					}
				} else if ((cfg_ht_bw == BW_40) && (cfg_vht_bw == VHT_BW_2040)) {
					for (k = 0; k < 2; k++) {
						for (l = -1; l < 2; l++) {
							if ((j+l < 0) || (j+l >= auto_ch_ctrl->pChannelInfo->channel_list_num))
								continue;
							if (auto_ch_ctrl->pChannelInfo->channel_score[j+l].channel == auto_ch_ctrl->pChannelInfo->group_ch_list[i].bw40_grp_ch_member[k].channel) {
								auto_ch_ctrl->pChannelInfo->channel_score[j+l].score = auto_ch_ctrl->pChannelInfo->group_ch_list[i].grp_score + 1 - k;
								if ((auto_ch_ctrl->pChannelInfo->group_ch_list[i].bw40_not_allowed == TRUE) ||
									(vht40_channel_group(pAd, auto_ch_ctrl->pChannelInfo->channel_score[j+l].channel, wdev) == FALSE)) {
									auto_ch_ctrl->pChannelInfo->channel_score[j+l].score = min_score;
									MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "Channel %d don't support BW40, force to assign min score(%d)!!!!!!\n",
										auto_ch_ctrl->pChannelInfo->channel_score[j+l].channel, auto_ch_ctrl->pChannelInfo->channel_score[j+l].score);
								}
								if (auto_ch_ctrl->pChannelInfo->channel_score[j+l].score > 100)
									auto_ch_ctrl->pChannelInfo->channel_score[j+l].score = 100;
								break;
							}
						}
					}
				} else {
				}
			}
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "=================ACS score board===================\n");
	for (j = 0; j < ch_ctrl->ChListNum; j++) {
		if (auto_ch_ctrl->pChannelInfo->channel_score[j].channel == 0)
			continue;
		if (auto_ch_ctrl->pChannelInfo->channel_score[j].channel == best_channel)
			best_ch_score = auto_ch_ctrl->pChannelInfo->channel_score[j].score;
	}

	for (j = 0; j < ch_ctrl->ChListNum; j++) {
		if ((auto_ch_ctrl->pChannelInfo->channel_score[j].score == best_ch_score) && (best_ch_score != 0)
			&& (auto_ch_ctrl->pChannelInfo->channel_score[j].channel != best_channel)) {
			auto_ch_ctrl->pChannelInfo->channel_score[j].score -= 1;
		}
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "CH%d final score is %d\n", auto_ch_ctrl->pChannelInfo->channel_score[j].channel,
			 auto_ch_ctrl->pChannelInfo->channel_score[j].score);
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "Min Busy Time=%d,select best channel %d\n", min_busy, best_channel);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "====================================================\n");

	/*if no best channel can be selected, use default ch*/
	if (best_channel == 0) {
		best_channel = FirstChannel(pAd, wdev);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
			"no best ch can be selected, use default ch: %d\n", best_channel);
	}
	return best_channel;
}

static inline UCHAR select_clear_channel_bw_160M(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev)
{
	UINT32 i = 0;
	UINT32 best_ch_score = 0;
	UINT32 min_busy = 0xffffffff;
	UINT32 min_busy_actual = 0xffffffff;
	UCHAR best_channel = 0;
	UCHAR best_channel_actual = 0;
	AUTO_CH_CTRL *auto_ch_ctrl = HcGetAutoChCtrl(pAd);

	/*first, we get the best_channel from all channel list*/
	/*the best_channel is just the best primary 20M channel*/
	/*the best_channel has the min_busy*/
	for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
		if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time < min_busy) {
			min_busy = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
			best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
		}
	}

	/*second, we get the best_ch_score from all channel list*/
	for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
		if (auto_ch_ctrl->pChannelInfo->channel_score[i].channel == best_channel) {
			best_ch_score = auto_ch_ctrl->pChannelInfo->channel_score[i].score;
			break;
		}
	}

	/*third, we get the best_channel_actual from bw 160M channel list 36-64*/
	/*the best_channel_actual is just the best primary 20M channel,can't be a skipped one*/
	for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
		if ((auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel > 64) ||
			auto_ch_ctrl->pChannelInfo->supp_ch_list[i].skip_channel == TRUE)
			continue;
		if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time < min_busy_actual) {
			min_busy_actual = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
			best_channel_actual = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
		}
	}

	/*last, there will be the only one best_ch_score*/
	for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
		if ((auto_ch_ctrl->pChannelInfo->channel_score[i].score == best_ch_score) && (best_ch_score != 0)
			&& (auto_ch_ctrl->pChannelInfo->channel_score[i].channel != best_channel)) {
			auto_ch_ctrl->pChannelInfo->channel_score[i].score -= 1;
		}
	}

	for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				"Channel %d : Busy Time = %u, Score %d,Skip Channel = %s\n",
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel,
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time,
				auto_ch_ctrl->pChannelInfo->channel_score[i].score,
				(auto_ch_ctrl->pChannelInfo->supp_ch_list[i].skip_channel == TRUE) ? "TRUE" : "FALSE");
	}

	/*if no best channel can be selected, use default ch*/
	if (best_channel_actual == 0) {
		best_channel_actual = FirstChannel(pAd, wdev);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
			"no best ch can be selected, use default ch: %d\n", best_channel_actual);
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "for BW 160M, select the best primary channel %d\n", best_channel_actual);
	return best_channel_actual;
}

static inline UCHAR select_clear_channel_busy_time(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev)
{
	UINT32 i = 0;
	INT32 score = 0;
	UINT32 best_ch_score = 0;
	UINT32 ch1_busy_time = 0xffffffff;
	UINT32 ch6_busy_time = 0xffffffff;
	UINT32 ch11_busy_time = 0xffffffff;
	UINT32 min_busy = 0xffffffff;
	UINT32 max_busy = 0;
	UCHAR best_channel = 0;
	UINT8 bit_map = 0;/*ch1:bit0;ch6:bit1;ch11:bit2*/
	AUTO_CH_CTRL *auto_ch_ctrl = HcGetAutoChCtrl(pAd);

	for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
		if (((auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 1) ||
			(auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 6) ||
			(auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 11))) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time < min_busy) {
				min_busy = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
				best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
			}
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time > max_busy)
				max_busy = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
		}

		if ((auto_ch_ctrl->pChannelInfo->ApCnt[i] != 0)) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 1) {
				bit_map |= (1<<0);
			} else if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 6) {
				bit_map |= (1<<1);
			} else if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 11) {
				bit_map |= (1<<2);
			}
		}
	}

	/*AP @ Ch 1,6,11*/
	switch (bit_map) {
	case 7:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (((auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 1) ||
				(auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 6) ||
				(auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 11))) {
				if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time < min_busy) {
					min_busy = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
					best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
				}
			}
		}

		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (((auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 1) &&
				(auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 6) &&
				(auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 11))) {
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
			}
		}
	break;
	case 6:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 1) {
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
			} else
				best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
		}
	break;
	case 5:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 6) {
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
			} else
				best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
		}
	break;
	case 4:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 1)
				ch1_busy_time = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
			else if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 6)
				ch6_busy_time = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
		}

		if (ch1_busy_time <= ch6_busy_time) {
			for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
				if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 1) {
					auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
				} else
					best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
			}
		} else {
			for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
				if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 6) {
					auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
				} else
					best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
			}
		}
	break;
	case 3:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 11) {
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
			} else
				best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
		}
	break;
	case 2:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 1)
				ch1_busy_time = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
			else if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 11)
				ch11_busy_time = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
		}

		if (ch1_busy_time <= ch11_busy_time) {
			for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
				if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 1) {
					auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
				} else
					best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
			}
		} else {
			for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
				if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 11) {
					auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
				} else
					best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
			}
		}
	break;
	case 1:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 6)
				ch6_busy_time = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
			else if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 11)
				ch11_busy_time = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
		}

		if (ch6_busy_time <= ch11_busy_time) {
			for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
				if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 6) {
					auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
				} else
					best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
			}
		} else {
			for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
				if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 11) {
					auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
				} else
					best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
			}
		}
	break;
	case 0:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != best_channel)
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
		}
	break;

	default:
	break;
	}

	for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
		score = 100 - (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time / 1000);
		if (score < 0)
			score = 0;
		if (auto_ch_ctrl->pChannelInfo->channel_score[i].channel == best_channel)
			best_ch_score = score;
		auto_ch_ctrl->pChannelInfo->channel_score[i].score = score;
		auto_ch_ctrl->pChannelInfo->channel_score[i].channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
	}

	for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
		if ((auto_ch_ctrl->pChannelInfo->channel_score[i].score == best_ch_score) && (best_ch_score != 0)
			&& (auto_ch_ctrl->pChannelInfo->channel_score[i].channel != best_channel)) {
			auto_ch_ctrl->pChannelInfo->channel_score[i].score -= 1;
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "=====================================================\n");
	for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "Channel %d : Busy Time = %u, Score %d,Skip Channel = %s\n",
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel,
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time,
				auto_ch_ctrl->pChannelInfo->channel_score[i].score,
				(auto_ch_ctrl->pChannelInfo->supp_ch_list[i].skip_channel == TRUE) ? "TRUE" : "FALSE");
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "=====================================================\n");
	}

	if (WMODE_CAP_5G(wdev->PhyMode)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "=============== select best ch for 5G ===============\n");
		acs_generate_group_channel_list(pAd, wdev);
		best_channel = find_best_channel_of_all_grp(pAd, wdev);
	} else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "=============== select best ch for 2.4G =============\n");

	return best_channel;
}
#endif

#ifdef DOT11_EHT_BE
static inline UCHAR SelectClearChannelForBW320(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev)
{
	UCHAR i = 0, GroupIdx = 0;
	UCHAR ch_band = wlan_config_get_ch_band(wdev);
	UCHAR best_ch = FirstChannel(pAd, wdev);
	UCHAR ext_ch = wlan_config_get_ext_cha(wdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	PCHANNELINFO pChannelInfo = pAutoChCtrl->pChannelInfo;
	INT total_group_num = 0;
	struct eht_ch_layout *eht_ch_320M = get_eht_ch_array(BW_320, ch_band, &total_group_num);
	UINT32 best_group_busytime = 0xFFFFFFFF, best_ch_busytime = 0;
	UCHAR best_ch_prio = 0;
	UCHAR CandidateGroupNum = 0;
	UCHAR ch_offset = 0;
	AUTOCH_SEL_CTRL *pAutoChSelCtrl = &pAutoChCtrl->AutoChSelCtrl;

	if (eht_ch_320M == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			 "eht_ch_320M is null\n");
		return best_ch;
	}

	if (ch_band != WIFI_CH_BAND_6G) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			 "ch_band=%d, not 6G BAND, return\n", ch_band);
		return best_ch;
	}

	if (pChannelInfo == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
				"pChannelInfo equal NULL\n");
		return best_ch;
	}

	if (pAutoChSelCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
				"pAutoChSelCtrl equal NULL\n");
		return best_ch;
	}

	if (eht_ch_320M == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
				"eht_ch_320M equal NULL\n");
		return best_ch;
	}

	ch_offset = (ext_ch == EXTCHA_ABOVE) ? 0 : 8;

	for (i = 0; i < pAutoChSelCtrl->ChListNum; i++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"Channel %d : full cca time = %u\n",
				pAutoChSelCtrl->AutoChSelChList[i].Channel,
				pChannelInfo->chanbusytime[i]);
	}

	for (GroupIdx = 0; GroupIdx < total_group_num; GroupIdx++) {
		UCHAR low_ch, up_ch;
		UCHAR low_ch_idx, high_ch_idx;
		UCHAR group_max_priority, lower_subgrp_max_priority, upper_subgrp_max_priority;
		UCHAR lower_subgrp_best_ch_idx, upper_subgrp_best_ch_idx;
		UCHAR group_best_ch_idx;
		UINT32 group_max_busytime;

		low_ch = eht_ch_320M[GroupIdx].ch_low_bnd;
		up_ch = eht_ch_320M[GroupIdx].ch_up_bnd;

		for (low_ch_idx = 0; low_ch_idx < pAutoChSelCtrl->ChListNum; low_ch_idx++)
			if (pAutoChSelCtrl->AutoChSelChList[low_ch_idx].Channel == low_ch)
				break;

		if (low_ch_idx == pAutoChSelCtrl->ChListNum) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				"low_ch= %d is not found in ACS ch list\n", low_ch);
			continue;
		}

		for (high_ch_idx = 0; high_ch_idx < pAutoChSelCtrl->ChListNum; high_ch_idx++)
			if (pAutoChSelCtrl->AutoChSelChList[high_ch_idx].Channel == up_ch)
				break;

		if (high_ch_idx == pAutoChSelCtrl->ChListNum) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				"up_ch= %d is not found in ACS ch list\n", up_ch);
			continue;
		}
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"low_ch= %d, low_ch_idx= %d\n", low_ch, low_ch_idx);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"up_ch= %d, high_ch_idx= %d\n", up_ch, high_ch_idx);

		if (high_ch_idx != low_ch_idx + 15) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
			"group:%d not supports 320M\n", GroupIdx);
			continue;
		}

		if (low_ch_idx > MAX_NUM_OF_CHANNELS) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
				"Invalid low_ch_idx:%d\n", low_ch_idx);
			continue;
		}

		group_max_busytime = pAutoChCtrl->pChannelInfo->chanbusytime[low_ch_idx];
		lower_subgrp_max_priority = pAutoChSelCtrl->AutoChSelChList[low_ch_idx].Priority;
		lower_subgrp_best_ch_idx = low_ch_idx;
		upper_subgrp_max_priority = pAutoChSelCtrl->AutoChSelChList[low_ch_idx + 8].Priority;
		upper_subgrp_best_ch_idx = low_ch_idx + 8;

		/*find max busytime with max priority in the group*/
		for (i = low_ch_idx; i <= high_ch_idx; i++) {

			if (pAutoChCtrl->pChannelInfo->chanbusytime[i] > group_max_busytime)
				group_max_busytime = pAutoChCtrl->pChannelInfo->chanbusytime[i];

			/*Record the maximum priority and best channel index
			of upper and lower subgroups*/
			/*lower subgroup*/
			if (i < low_ch_idx + 8) {
				if (lower_subgrp_max_priority < pAutoChSelCtrl->AutoChSelChList[i].Priority) {

					lower_subgrp_max_priority = pAutoChSelCtrl->AutoChSelChList[i].Priority;
					lower_subgrp_best_ch_idx = i;

				} else if (lower_subgrp_max_priority == pAutoChSelCtrl->AutoChSelChList[i].Priority
							&& pAutoChCtrl->pChannelInfo->chanbusytime[i] < pAutoChCtrl->pChannelInfo->chanbusytime[lower_subgrp_best_ch_idx]) {

					lower_subgrp_best_ch_idx = i;
				}
				/*upper subgroup*/
			} else {
				if (upper_subgrp_max_priority < pAutoChSelCtrl->AutoChSelChList[i].Priority) {

					upper_subgrp_max_priority = pAutoChSelCtrl->AutoChSelChList[i].Priority;
					upper_subgrp_best_ch_idx = i;

				} else if (upper_subgrp_max_priority == pAutoChSelCtrl->AutoChSelChList[i].Priority
						&& pAutoChCtrl->pChannelInfo->chanbusytime[i] < pAutoChCtrl->pChannelInfo->chanbusytime[upper_subgrp_best_ch_idx]) {

					upper_subgrp_best_ch_idx = i;
				}
			}
		}

		CandidateGroupNum++;

		/*check if this group is the first or last group*/
		if (GroupIdx == 0  || GroupIdx == total_group_num - 1) {
			if (upper_subgrp_max_priority > lower_subgrp_max_priority) {

				group_max_priority = upper_subgrp_max_priority;
				group_best_ch_idx = upper_subgrp_best_ch_idx;

			} else if (upper_subgrp_max_priority == lower_subgrp_max_priority) {

				group_max_priority = upper_subgrp_max_priority;
				group_best_ch_idx = pAutoChCtrl->pChannelInfo->chanbusytime[upper_subgrp_best_ch_idx]
									< pAutoChCtrl->pChannelInfo->chanbusytime[lower_subgrp_best_ch_idx] ?
									upper_subgrp_best_ch_idx : lower_subgrp_best_ch_idx;

			} else {
				group_max_priority = lower_subgrp_max_priority;
				group_best_ch_idx = lower_subgrp_best_ch_idx;
			}
		} else {
			if (ext_ch == EXTCHA_ABOVE) {
				group_max_priority = lower_subgrp_max_priority;
				group_best_ch_idx = lower_subgrp_best_ch_idx;

			} else {
				group_max_priority = upper_subgrp_max_priority;
				group_best_ch_idx = upper_subgrp_best_ch_idx;
			}
		}

		/*update the best channel*/
		if (group_max_busytime < best_group_busytime
			&& group_max_priority == pAutoChSelCtrl->MaxChPriority) {

			best_group_busytime = group_max_busytime;
			best_ch = pAutoChSelCtrl->AutoChSelChList[group_best_ch_idx].Channel;
			best_ch_busytime = pChannelInfo->chanbusytime[group_best_ch_idx];
			best_ch_prio = pAutoChSelCtrl->AutoChSelChList[group_best_ch_idx].Priority;

		} else if (group_max_busytime == best_group_busytime &&
					group_max_priority == pAutoChSelCtrl->MaxChPriority) {

			if (best_ch_busytime > pChannelInfo->chanbusytime[group_best_ch_idx]) {
				best_ch = pAutoChSelCtrl->AutoChSelChList[group_best_ch_idx].Channel;
				best_ch_busytime = pChannelInfo->chanbusytime[group_best_ch_idx];
			}
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
			"Rule 3 Channel Busy time value : Select Primary Channel %d\n", best_ch);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
			"Rule 3 Channel Busy time value : Min Channel Busy = %d, priority = %d\n",
			best_ch_busytime, best_ch_prio);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
			"Rule 3 Channel Busy time value : ext_ch = %s\n",
			(ext_ch == EXTCHA_ABOVE) ? "EXTCHA_ABOVE" : "EXTCHA_BELOW");
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
			"Rule 3 Channel Busy time value : BW = 320\n");

	return best_ch;

}
#endif



static inline UCHAR SelectClearChannelBusyTime(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev)
{
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	PCHANNELINFO pChannelInfo = pAutoChCtrl->pChannelInfo;
	UINT32 SubGroupMaxBusyTime, SubGroupMaxBusyTimeChIdx, MinBusyTime;
	UINT32 SubGroupMinBusyTime, SubGroupMinBusyTimeChIdx, ChannelIdx, StartChannelIdx, Temp1, Temp2;
	INT	i, j, GroupNum, CandidateCh1 = 0, CandidateChIdx1, base;
	UCHAR maxChPrio = 0, MaxChPrio, CurChPrio;
#ifdef DOT11_VHT_AC
	UINT32 MinorMinBusyTime;
	INT CandidateCh2, CandidateChIdx2;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	UCHAR vht_bw = wlan_config_get_vht_bw(wdev);
	UCHAR cen_ch_2;
	UCHAR ch_band = wlan_config_get_ch_band(wdev);
#endif/* DOT11_VHT_AC */
#ifndef DOT11_VHT_AC
#endif/* DOT11_VHT_AC */
	PUINT32 pSubGroupMaxBusyTimeTable = NULL;
	PUINT32 pSubGroupMaxBusyTimeChIdxTable = NULL;
	PUINT32 pSubGroupMinBusyTimeTable = NULL;
	PUINT32 pSubGroupMinBusyTimeChIdxTable = NULL;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	AUTOCH_SEL_CTRL *pAutoChSelCtrl = &pAutoChCtrl->AutoChSelCtrl;


	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "[SelectClearChannelBusyTime] - cfg_ht_bw = %d vht_bw = %d\n", cfg_ht_bw, vht_bw);

	if (pChannelInfo == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "pAd->pChannelInfo equal NULL.\n");
		return FirstChannel(pAd, wdev);
	}

	if (pAutoChSelCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "pAutoChSelCtrl is NULL.\n");
		return FirstChannel(pAd, wdev);
	}

	if (pAutoChSelCtrl->ChListNum == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "ChListNum is 0.\n");
		return FirstChannel(pAd, wdev);
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "====================================================================\n");

	for (ChannelIdx = 0; ChannelIdx < pAutoChSelCtrl->ChListNum; ChannelIdx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				"Channel %3d : Busy Time = %6u, Skip Channel = %s, BwCap = %s, Priority = %d\n",
				 pAutoChSelCtrl->AutoChSelChList[ChannelIdx].Channel, pChannelInfo->chanbusytime[ChannelIdx],
				 (pAutoChSelCtrl->AutoChSelChList[ChannelIdx].SkipChannel == TRUE) ? "TRUE" : "FALSE",
				 (pAutoChSelCtrl->AutoChSelChList[ChannelIdx].BwCap == TRUE)?"TRUE" : "FALSE",
				 pAutoChSelCtrl->AutoChSelChList[ChannelIdx].Priority);
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "====================================================================\n");

#ifdef DOT11_EHT_BE
	/* bw320 */
	if (pAutoChSelCtrl->AutoChSelChList[0].Bw == BW_320)
		return SelectClearChannelForBW320(pAd, wdev);
#endif /* DOT11_EHT_BE */
	/*Initialization*/
	SubGroupMaxBusyTimeChIdx = 0;
	SubGroupMaxBusyTime = pChannelInfo->chanbusytime[SubGroupMaxBusyTimeChIdx];
	SubGroupMinBusyTimeChIdx = 0;
	SubGroupMinBusyTime = pChannelInfo->chanbusytime[SubGroupMinBusyTimeChIdx];
	StartChannelIdx = SubGroupMaxBusyTimeChIdx + 1;
	GroupNum = 0;
	maxChPrio = pAutoChSelCtrl->AutoChSelChList[SubGroupMaxBusyTimeChIdx].Priority;

	os_alloc_mem(pAd, (UCHAR **)&pSubGroupMaxBusyTimeTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	if (!pSubGroupMaxBusyTimeTable) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"alloc buf for pSubGroupMaxBusyTimeTable failed!\n");
		goto ReturnCh;
	}
	os_alloc_mem(pAd, (UCHAR **)&pSubGroupMaxBusyTimeChIdxTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	if (!pSubGroupMaxBusyTimeChIdxTable) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"alloc buf for pSubGroupMaxBusyTimeChIdxTable failed!\n");
		goto ReturnCh;
	}
	os_alloc_mem(pAd, (UCHAR **)&pSubGroupMinBusyTimeTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	if (!pSubGroupMinBusyTimeTable) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"alloc buf for pSubGroupMinBusyTimeTable failed!\n");
		goto ReturnCh;
	}
	os_alloc_mem(pAd, (UCHAR **)&pSubGroupMinBusyTimeChIdxTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	if (!pSubGroupMinBusyTimeChIdxTable) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"alloc buf for pSubGroupMinBusyTimeChIdxTable failed!\n");
		goto ReturnCh;
	}
	NdisZeroMemory(pSubGroupMaxBusyTimeTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	NdisZeroMemory(pSubGroupMaxBusyTimeChIdxTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	NdisZeroMemory(pSubGroupMinBusyTimeTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	NdisZeroMemory(pSubGroupMinBusyTimeChIdxTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	if ((pAd->ApCfg.bAutoChannelAtBootup) && IS_SUPPORT_V10_DFS(pAd) && (WMODE_CAP_5G(wdev->PhyMode))
		&& (IS_DFS_V10_ACS_VALID(pAd) == FALSE) && (wlan_config_get_vht_bw(wdev) == VHT_BW_2040)) {
		UCHAR listSize = 0;

		ChannelIdx = 0;
		listSize = V10_TOTAL_CHANNEL_COUNT;

		NdisZeroMemory(pDfsParam->DfsV10SortedACSList, (V10_TOTAL_CHANNEL_COUNT)*sizeof(V10_CHANNEL_LIST));
		pDfsParam->DfsV10SortedACSList[ChannelIdx].BusyTime = pChannelInfo->chanbusytime[ChannelIdx];
		pDfsParam->DfsV10SortedACSList[ChannelIdx].Channel =
			pAutoChSelCtrl->AutoChSelChList[ChannelIdx].Channel;
	}
#endif

	for (ChannelIdx = StartChannelIdx; ChannelIdx < pAutoChSelCtrl->ChListNum; ChannelIdx++) {
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		if ((pAd->ApCfg.bAutoChannelAtBootup) && IS_SUPPORT_V10_DFS(pAd) &&
			(WMODE_CAP_5G(wdev->PhyMode)) && (IS_DFS_V10_ACS_VALID(pAd) == FALSE) &&
			(wlan_config_get_vht_bw(wdev) == VHT_BW_2040)) {
			pDfsParam->DfsV10SortedACSList[ChannelIdx].BusyTime = pChannelInfo->chanbusytime[ChannelIdx];
			pDfsParam->DfsV10SortedACSList[ChannelIdx].Channel =
					pAutoChSelCtrl->AutoChSelChList[ChannelIdx].Channel;
		}
#endif

		/*Compare the busytime with each other in the same group*/
		if (pAutoChSelCtrl->AutoChSelChList[ChannelIdx].CentralChannel == pAutoChSelCtrl->AutoChSelChList[ChannelIdx-1].CentralChannel) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
					 "pChannelInfo->chanbusytime[%d] = %d, SubGroupMaxBusyTime = %d, SubGroupMinBusyTime = %d\n",
					  ChannelIdx, pChannelInfo->chanbusytime[ChannelIdx], SubGroupMaxBusyTime, SubGroupMinBusyTime);

			if (pChannelInfo->chanbusytime[ChannelIdx] > SubGroupMaxBusyTime) {
				SubGroupMaxBusyTime = pChannelInfo->chanbusytime[ChannelIdx];
				SubGroupMaxBusyTimeChIdx = ChannelIdx;
			}

			if (pAutoChSelCtrl->AutoChSelChList[ChannelIdx].Priority > maxChPrio) {

				SubGroupMinBusyTime = pChannelInfo->chanbusytime[ChannelIdx];
				SubGroupMinBusyTimeChIdx = ChannelIdx;
				maxChPrio = pAutoChSelCtrl->AutoChSelChList[ChannelIdx].Priority;

			} else if (pAutoChSelCtrl->AutoChSelChList[ChannelIdx].Priority == maxChPrio
				&& pChannelInfo->chanbusytime[ChannelIdx] < SubGroupMinBusyTime) {

				SubGroupMinBusyTime = pChannelInfo->chanbusytime[ChannelIdx];
				SubGroupMinBusyTimeChIdx = ChannelIdx;

			}

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
					 "SubGroupMaxBusyTime = %d, SubGroupMaxBusyTimeChIdx = %d,SubGroupMinBusyTime = %d SubGroupMinBusyTimeChIdx = %d\n",
					  SubGroupMaxBusyTime, SubGroupMaxBusyTimeChIdx, SubGroupMinBusyTime, SubGroupMinBusyTimeChIdx);

			/*Fill in the group table in order for the last group*/
			if (ChannelIdx == (pAutoChSelCtrl->ChListNum - 1)) {
				pSubGroupMaxBusyTimeTable[GroupNum] = SubGroupMaxBusyTime;
				pSubGroupMaxBusyTimeChIdxTable[GroupNum] = SubGroupMaxBusyTimeChIdx;
				pSubGroupMinBusyTimeTable[GroupNum] = SubGroupMinBusyTime;
				pSubGroupMinBusyTimeChIdxTable[GroupNum] = SubGroupMinBusyTimeChIdx;

				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
						 "SubGroupMaxBusyTimeTable[%d] = %d, SubGroupMaxBusyTimeChIdxTable[%d] = %d, SubGroupMinBusyTimeTable[%d] = %d, SubGroupMinBusyTimeChIdxTable[%d] = %d\n",
						  GroupNum, pSubGroupMaxBusyTimeTable[GroupNum], GroupNum, pSubGroupMaxBusyTimeChIdxTable[GroupNum],
						  GroupNum, pSubGroupMinBusyTimeTable[GroupNum], GroupNum, pSubGroupMinBusyTimeChIdxTable[GroupNum]);
				GroupNum++;
			}

		} else {
			/*Fill in the group table*/
			pSubGroupMaxBusyTimeTable[GroupNum] = SubGroupMaxBusyTime;
			pSubGroupMaxBusyTimeChIdxTable[GroupNum] = SubGroupMaxBusyTimeChIdx;
			pSubGroupMinBusyTimeTable[GroupNum] = SubGroupMinBusyTime;
			pSubGroupMinBusyTimeChIdxTable[GroupNum] = SubGroupMinBusyTimeChIdx;

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
					 "SubGroupMaxBusyTimeTable[%d] = %d, SubGroupMaxBusyTimeChIdxTable[%d] = %d, SubGroupMinBusyTimeTable[%d] = %d, SubGroupMinBusyTimeChIdxTable[%d] = %d\n",
					  GroupNum, pSubGroupMaxBusyTimeTable[GroupNum], GroupNum, pSubGroupMaxBusyTimeChIdxTable[GroupNum],
					  GroupNum, pSubGroupMinBusyTimeTable[GroupNum], GroupNum, pSubGroupMinBusyTimeChIdxTable[GroupNum]);

			GroupNum++;

			/*Fill in the group table in order for the last group in case of BW20*/
			if ((ChannelIdx == (pAutoChSelCtrl->ChListNum - 1))
				&& (pAutoChSelCtrl->AutoChSelChList[ChannelIdx].Bw == BW_20)) {

				pSubGroupMaxBusyTimeTable[GroupNum] = pChannelInfo->chanbusytime[ChannelIdx];
				pSubGroupMaxBusyTimeChIdxTable[GroupNum] = ChannelIdx;
				pSubGroupMinBusyTimeTable[GroupNum] = pChannelInfo->chanbusytime[ChannelIdx];
				pSubGroupMinBusyTimeChIdxTable[GroupNum] = ChannelIdx;

				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
						 "SubGroupMaxBusyTimeTable[%d] = %d, SubGroupMaxBusyTimeChIdxTable[%d] = %d, SubGroupMinBusyTimeTable[%d] = %d, SubGroupMinBusyTimeChIdxTable[%d] = %d\n",
						  GroupNum, pSubGroupMaxBusyTimeTable[GroupNum], GroupNum, pSubGroupMaxBusyTimeChIdxTable[GroupNum],
						  GroupNum, pSubGroupMinBusyTimeTable[GroupNum], GroupNum, pSubGroupMinBusyTimeChIdxTable[GroupNum]);

				GroupNum++;
			} else {
				/*Reset indices in order to start checking next group*/
				SubGroupMaxBusyTime = pChannelInfo->chanbusytime[ChannelIdx];
				SubGroupMaxBusyTimeChIdx = ChannelIdx;
				SubGroupMinBusyTime = pChannelInfo->chanbusytime[ChannelIdx];
				SubGroupMinBusyTimeChIdx = ChannelIdx;
				maxChPrio = pAutoChSelCtrl->AutoChSelChList[ChannelIdx].Priority;
			}
		}
	}

	for (i = 0; i < GroupNum; i++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				 "SubGroupMaxBusyTimeTable[%d] = %d, pSubGroupMaxBusyTimeChIdxTable[%d] = %d,\nSubGroupMinBusyTimeTable[%d] = %d, pSubGroupMinBusyTimeChIdxTable[%d] = %d\n",
				  i, pSubGroupMaxBusyTimeTable[i], i, pSubGroupMaxBusyTimeChIdxTable[i],
				  i, pSubGroupMinBusyTimeTable[i], i, pSubGroupMinBusyTimeChIdxTable[i]);
	}

	/*Sort max_busy_time group table from the smallest to the biggest one */
	for (i = 0; i < GroupNum; i++) {
		MaxChPrio = pAutoChSelCtrl->AutoChSelChList[pSubGroupMinBusyTimeChIdxTable[i]].Priority;

		for (j = 1; j < (GroupNum-i); j++) {
			if ((i+j) >= MAX_NUM_OF_CHANNELS + 1)
				break;
			else {
				CurChPrio = pAutoChSelCtrl->AutoChSelChList[pSubGroupMinBusyTimeChIdxTable[i+j]].Priority;

				/*swap in 3 scenarios:
				1. Higher priority
				2. Same priority, smaller maximum busy time
				3. Same priority, same maximum busy time,
					smaller minimum busy time*/
				if (MaxChPrio < CurChPrio
				|| (MaxChPrio == CurChPrio && pSubGroupMaxBusyTimeTable[i] > pSubGroupMaxBusyTimeTable[i+j])
				|| (MaxChPrio == CurChPrio && pSubGroupMaxBusyTimeTable[i] == pSubGroupMaxBusyTimeTable[i+j]
					&& pSubGroupMinBusyTimeTable[i] > pSubGroupMinBusyTimeTable[i+j])) {

					/*update MaxChPrio*/
					MaxChPrio = CurChPrio;
					/*Swap pSubGroupMaxBusyTimeTable[i]
					for pSubGroupMaxBusyTimeTable[i+j]*/
					Temp1 = pSubGroupMaxBusyTimeTable[i+j];
					pSubGroupMaxBusyTimeTable[i+j] = pSubGroupMaxBusyTimeTable[i];
					pSubGroupMaxBusyTimeTable[i] = Temp1;
					/*Swap pSubGroupMaxBusyTimeChIdxTable[i]
					for pSubGroupMaxBusyTimeChIdxTable[i+j]*/
					Temp2 = pSubGroupMaxBusyTimeChIdxTable[i+j];
					pSubGroupMaxBusyTimeChIdxTable[i+j] = pSubGroupMaxBusyTimeChIdxTable[i];
					pSubGroupMaxBusyTimeChIdxTable[i] = Temp2;
					/*Swap pSubGroupMinBusyTimeTable[i]
					for pSubGroupMinBusyTimeTable[i+j]*/
					Temp1 = pSubGroupMinBusyTimeTable[i+j];
					pSubGroupMinBusyTimeTable[i+j] = pSubGroupMinBusyTimeTable[i];
					pSubGroupMinBusyTimeTable[i] = Temp1;
					/*Swap pSubGroupMinBusyTimeChIdxTable[i]
					for pSubGroupMinBusyTimeChIdxTable[i+j]*/
					Temp2 = pSubGroupMinBusyTimeChIdxTable[i+j];
					pSubGroupMinBusyTimeChIdxTable[i+j] = pSubGroupMinBusyTimeChIdxTable[i];
					pSubGroupMinBusyTimeChIdxTable[i] = Temp2;
				}
			}
		}
	}

	for (i = 0; i < GroupNum; i++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				 "SubGroupMaxBusyTimeTable[%d] = %d, pSubGroupMaxBusyTimeChIdxTable[%d] = %d,\nSubGroupMinBusyTimeTable[%d] = %d, pSubGroupMinBusyTimeChIdxTable[%d] = %d, channelpriority = %d\n",
				  i, pSubGroupMaxBusyTimeTable[i], i, pSubGroupMaxBusyTimeChIdxTable[i],
				  i, pSubGroupMinBusyTimeTable[i], i, pSubGroupMinBusyTimeChIdxTable[i],
				  pAutoChSelCtrl->AutoChSelChList[pSubGroupMinBusyTimeChIdxTable[i]].Priority);
	}

#ifdef DOT11_VHT_AC

	/*Return channel in case of VHT BW80+80*/
	if ((vht_bw == VHT_BW_8080)
		&& (cfg_ht_bw == BW_40)
		&& (GroupNum > 2)
		&& (WMODE_CAP_AC(wdev->PhyMode) == TRUE)) {
		MinBusyTime = pSubGroupMinBusyTimeTable[0];
		MinorMinBusyTime = pSubGroupMinBusyTimeTable[1];
		/*Select primary channel, whose busy time is minimum in the group*/
		CandidateChIdx1 = pSubGroupMinBusyTimeChIdxTable[0];
		CandidateCh1 = pAutoChSelCtrl->AutoChSelChList[CandidateChIdx1].Channel;
		/*Select secondary VHT80 central channel*/
		CandidateChIdx2 = pSubGroupMaxBusyTimeChIdxTable[1];
		CandidateCh2 = pAutoChSelCtrl->AutoChSelChList[CandidateChIdx2].Channel;
		cen_ch_2 = vht_cent_ch_freq((UCHAR)CandidateCh2, VHT_BW_80, ch_band);
		/*Since primary channel is not updated yet ,cannot update sec Ch here*/
		wdev->auto_channel_cen_ch_2 = cen_ch_2;
		wlan_config_set_cen_ch_2(wdev, cen_ch_2);
		wlan_operate_set_cen_ch_2(wdev, cen_ch_2);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				 "Rule 3 Channel Busy time value : Select Primary Channel %d\n", CandidateCh1);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				 "Rule 3 Channel Busy time value : Select Secondary Central Channel %d\n", cen_ch_2);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				 "Rule 3 Channel Busy time value : Min Channel Busy = %u\n", MinBusyTime);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				 "Rule 3 Channel Busy time value : MinorMin Channel Busy = %u\n", MinorMinBusyTime);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				 "Rule 3 Channel Busy time value : BW = %s\n", "80+80");
		goto ReturnCh;
	}

#endif/*DOT11_VHT_AC*/

	if (GroupNum > 0) {
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	/* V10 VHT80 ACS Enable */
		if ((pAd->ApCfg.bAutoChannelAtBootup) && IS_SUPPORT_V10_DFS(pAd) && (WMODE_CAP_5G(wdev->PhyMode))
			&& (IS_DFS_V10_ACS_VALID(pAd) == FALSE) && (wlan_config_get_vht_bw(wdev) == VHT_BW_80)) {
			/* Record Best Channels from each group */
			os_zero_mem(pDfsParam->DfsV10SortedACSList, (GroupNum)*sizeof(V10_CHANNEL_LIST));
			for (ChannelIdx = 0; ChannelIdx < GroupNum; ChannelIdx++) {
				CandidateCh1 =
				pAutoChSelCtrl->AutoChSelChList[pSubGroupMinBusyTimeChIdxTable[ChannelIdx]].Channel;
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
					"Channel %d BusyTime %d Idx %d\n",
					CandidateCh1, pSubGroupMinBusyTimeTable[ChannelIdx],
					pSubGroupMinBusyTimeChIdxTable[ChannelIdx]);
				pDfsParam->DfsV10SortedACSList[ChannelIdx].Channel = CandidateCh1;
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
					"Channel %d\n", pDfsParam->DfsV10SortedACSList[ChannelIdx].Channel);
			}
			/* Enable V10 VHT80 ACS List */
			pDfsParam->bV10ChannelListValid = TRUE;
			pDfsParam->GroupCount = GroupNum;
		}

		if ((pAd->ApCfg.bAutoChannelAtBootup) && (wlan_config_get_vht_bw(wdev) == VHT_BW_2040)
			&& DfsV10ACSListSortFunction(pAd, wdev) == FALSE)
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"Invalid V10 ACS List BW %d\n", wlan_config_get_vht_bw(wdev));
#endif

		MinBusyTime = pSubGroupMinBusyTimeTable[0];
		/*Select primary channel, whose busy time is minimum in the group*/
		CandidateChIdx1 = pSubGroupMinBusyTimeChIdxTable[0];
		CandidateCh1 = pAutoChSelCtrl->AutoChSelChList[CandidateChIdx1].Channel;
		pDfsParam->band_bw = pAutoChSelCtrl->AutoChSelChList[CandidateChIdx1].Bw;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				 "Rule 3 Channel Busy time value : Select Primary Channel %d, priority %d\n",
				 CandidateCh1, pAutoChSelCtrl->AutoChSelChList[CandidateChIdx1].Priority);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				 "Rule 3 Channel Busy time value : Min Channel Busy = %u\n", MinBusyTime);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				 "Rule 3 Channel Busy time value : BW = %s\n",
				 (pAutoChSelCtrl->AutoChSelChList[CandidateChIdx1].Bw == BW_320)
				 ? "320"
				 : (pAutoChSelCtrl->AutoChSelChList[CandidateChIdx1].Bw == BW_160)
				 ? "160"
				 : (pAutoChSelCtrl->AutoChSelChList[CandidateChIdx1].Bw == BW_80)
				 ? "80"
				 : (pAutoChSelCtrl->AutoChSelChList[CandidateChIdx1].Bw == BW_40)
				 ? "40":"20");

		goto ReturnCh;
	}

	base = RandomByte(pAd);

	for (ChannelIdx = 0; ChannelIdx < pAutoChSelCtrl->ChListNum; ChannelIdx++) {
		CandidateCh1 = pAutoChSelCtrl->AutoChSelChList[(base + ChannelIdx) % pAutoChSelCtrl->ChListNum].Channel;

		if (AutoChannelSkipListCheck(pAd, CandidateCh1))
			continue;

		if ((pAd->ApCfg.bAvoidDfsChannel == TRUE)
			&& (pAutoChSelCtrl->IsABand == TRUE)
			&& RadarChannelCheck(pAd, CandidateCh1))
			continue;

		break;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
			 "Randomly Select : Select Channel %d\n", CandidateCh1);
ReturnCh:
	if (pSubGroupMaxBusyTimeTable)
		os_free_mem(pSubGroupMaxBusyTimeTable);

	if (pSubGroupMaxBusyTimeChIdxTable)
		os_free_mem(pSubGroupMaxBusyTimeChIdxTable);

	if (pSubGroupMinBusyTimeTable)
		os_free_mem(pSubGroupMinBusyTimeTable);

	if (pSubGroupMinBusyTimeChIdxTable)
		os_free_mem(pSubGroupMinBusyTimeChIdxTable);
	return CandidateCh1;
}

#ifdef CONFIG_6G_SUPPORT
static struct  eht_ch_layout acs_psc_ch_160M_6G[] = {
	{5, 21, 15},
	{37, 53, 47},
	{69, 85, 79},
	{101, 117, 111},
	{133, 149, 143},
	{165, 181, 175},
	{197, 213, 207},
	{0, 0, 0},
};

static struct  eht_ch_layout acs_psc_ch_320M_6G[] = {
	{5, 53, 31},
	{37, 85, 63},
	{69, 117, 95},
	{101, 149, 127},
	{133, 181, 159},
	{165, 213, 191},
	{0, 0, 0},
};

static inline UCHAR FindBestPSC6GByGroupSorting(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev)
{
	AUTO_CH_CTRL *pAutoChCtrl;
	PCHANNELINFO pChannelInfo;
	UCHAR i, GroupIdx;
	UCHAR CandidateCh = FirstChannel(pAd, wdev);
	UCHAR CandidateChPrio = 0;
	UCHAR CandidateGroupNum = 0;
	UCHAR CandidateGroupIdx = 0xff;
	UCHAR ext_ch = wlan_config_get_ext_cha(wdev);
	UINT32 best_group_busytime = 0xFFFFFFFF, best_ch_busytime = 0xFFFFFFFF;
	struct eht_ch_layout *acs_psc_ch_groups = acs_psc_ch_160M_6G;
	UCHAR total_group_num = ARRAY_SIZE(acs_psc_ch_160M_6G) - 1;
	BOOLEAN acs_320M = FALSE;
	AUTOCH_SEL_CTRL *pAutoChSelCtrl;

	pAutoChCtrl = HcGetAutoChCtrl(pAd);
	if (pAutoChCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "pAutoChCtrl is NULL.\n");
		return CandidateCh;
	}

	pChannelInfo = pAutoChCtrl->pChannelInfo;
	if (pChannelInfo == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "pChannelInfo is NULL.\n");
		return CandidateCh;
	}

	pAutoChSelCtrl = &pAutoChCtrl->AutoChSelCtrl;
	if (pAutoChSelCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "pAutoChSelCtrl is NULL.\n");
		return CandidateCh;
	}

	if (pAutoChSelCtrl->ChListNum > MAX_NUM_OF_CHANNELS) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "ChListNum is overrange.\n");
		return CandidateCh;
	}

	if (pAutoChSelCtrl->ChListNum == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"ACS channel list is null\n");
		return CandidateCh;
	}

#ifdef DOT11_EHT_BE
	if (pAutoChSelCtrl->AutoChSelChList[0].Bw == BW_320) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE, "acs for psc 320M\n");
		acs_psc_ch_groups = acs_psc_ch_320M_6G;
		total_group_num = ARRAY_SIZE(acs_psc_ch_320M_6G) - 1;
		acs_320M = TRUE;
	}
#endif

	if (total_group_num == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "total_group_num is 0.\n");
		return CandidateCh;
	}

	for (i = 0; i < pAutoChSelCtrl->ChListNum; i++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
				"Channel= %d,  chIdx= %d, full cca time = %u, priority = %u\n",
				pAutoChSelCtrl->AutoChSelChList[i].Channel,
				i,
				pChannelInfo->full_cca_time[i],
				pAutoChSelCtrl->AutoChSelChList[i].Priority);
	}

	for (GroupIdx = 0; GroupIdx < total_group_num; GroupIdx++) {
		UINT32 group_max_busytime;
		UCHAR group_max_chprio = 0, group_best_ch_idx = 0;
		UCHAR low_ch, up_ch;
		UCHAR low_ch_idx;
		UCHAR high_ch_idx;
		UCHAR lower_subgrp_max_ch_prio, upper_subgrp_max_ch_prio;
		UCHAR lower_subgrp_best_ch_idx, upper_subgrp_best_ch_idx;

		low_ch = acs_psc_ch_groups[GroupIdx].ch_low_bnd;
		up_ch = acs_psc_ch_groups[GroupIdx].ch_up_bnd;

		for (low_ch_idx = 0; low_ch_idx < pAutoChSelCtrl->ChListNum; low_ch_idx++)
			if (pAutoChSelCtrl->AutoChSelChList[low_ch_idx].Channel == low_ch)
				break;

		if (low_ch_idx == pAutoChSelCtrl->ChListNum) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				"low_ch= %d is not found in ACS ch list\n", low_ch);
			continue;
		}

		for (high_ch_idx = 0; high_ch_idx < pAutoChSelCtrl->ChListNum; high_ch_idx++)
			if (pAutoChSelCtrl->AutoChSelChList[high_ch_idx].Channel == up_ch)
				break;

		if (high_ch_idx == pAutoChSelCtrl->ChListNum) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				"up_ch= %d is not found in ACS ch list\n", up_ch);
			continue;
		}

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"low_ch= %d, low_ch_idx= %d\n", low_ch, low_ch_idx);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"up_ch= %d, high_ch_idx= %d\n", up_ch, high_ch_idx);
		if (acs_320M) {
			if (high_ch_idx != low_ch_idx + 3) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				"group:%d not supports 320M\n", GroupIdx);
				continue;
			}
		} else if (high_ch_idx != low_ch_idx + 1) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
			"group:%d not supports 160M\n", GroupIdx);
			continue;
		}

		if (high_ch_idx >= MAX_NUM_OF_CHANNELS || low_ch_idx >= MAX_NUM_OF_CHANNELS) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
			"group:%d 's idx[%d, %d] is over range\n", GroupIdx, low_ch_idx, high_ch_idx);
			continue;
		}

		group_max_busytime = pChannelInfo->full_cca_time[low_ch_idx];
		lower_subgrp_max_ch_prio = pAutoChSelCtrl->AutoChSelChList[low_ch_idx].Priority;
		lower_subgrp_best_ch_idx = low_ch_idx;
		upper_subgrp_max_ch_prio = pAutoChSelCtrl->AutoChSelChList[high_ch_idx].Priority;
		upper_subgrp_best_ch_idx = high_ch_idx;
		/*1.find the group_max_busytime in a PSC group
		2.Record the maximum priority and best channel index
		of upper and lower subgroups*/
		/*for 320MHz*/
		if (acs_320M) {

			for (i = low_ch_idx; i <= high_ch_idx; i++) {
				if (pChannelInfo->full_cca_time[i] > group_max_busytime)
					group_max_busytime = pChannelInfo->full_cca_time[i];

				/*lower subgroup*/
				if (i < low_ch_idx + 2) {
					if (lower_subgrp_max_ch_prio < pAutoChSelCtrl->AutoChSelChList[i].Priority) {

						lower_subgrp_max_ch_prio = pAutoChSelCtrl->AutoChSelChList[i].Priority;
						lower_subgrp_best_ch_idx = i;

					} else if (lower_subgrp_max_ch_prio == pAutoChSelCtrl->AutoChSelChList[i].Priority
								&& pChannelInfo->full_cca_time[i] < pChannelInfo->full_cca_time[lower_subgrp_best_ch_idx]) {

						lower_subgrp_best_ch_idx = i;
					}
					/*upper subgroup*/
				} else {
					if (upper_subgrp_max_ch_prio < pAutoChSelCtrl->AutoChSelChList[i].Priority) {

						upper_subgrp_max_ch_prio = pAutoChSelCtrl->AutoChSelChList[i].Priority;
						upper_subgrp_best_ch_idx = i;

					} else if (upper_subgrp_max_ch_prio == pAutoChSelCtrl->AutoChSelChList[i].Priority
								&& pChannelInfo->full_cca_time[i] <= pChannelInfo->full_cca_time[upper_subgrp_best_ch_idx]) {

						upper_subgrp_best_ch_idx = i;
					}
				}
			}
		/*for 160MHz*/
		} else {
			for (i = low_ch_idx; i <= high_ch_idx; i++) {
				if (pChannelInfo->full_cca_time[i] > group_max_busytime)
					group_max_busytime = pChannelInfo->full_cca_time[i];
			}
		}

		/*get max channel priority and best channel's index*/
		/*ext_ch will not be considered in the following cases:
		1.1st group and last group
		2.BW = 160MHz*/
		if (GroupIdx == 0 || GroupIdx == total_group_num - 1 || !acs_320M) {
			/*find the best channel in the group*/
			if (lower_subgrp_max_ch_prio > upper_subgrp_max_ch_prio) {
				group_max_chprio = lower_subgrp_max_ch_prio;
				group_best_ch_idx = lower_subgrp_best_ch_idx;

			} else if (lower_subgrp_max_ch_prio == upper_subgrp_max_ch_prio) {

				group_max_chprio = lower_subgrp_max_ch_prio;
				group_best_ch_idx = pChannelInfo->full_cca_time[upper_subgrp_best_ch_idx]
									< pChannelInfo->full_cca_time[lower_subgrp_best_ch_idx] ?
									upper_subgrp_best_ch_idx : lower_subgrp_best_ch_idx;

			} else {
				group_max_chprio = upper_subgrp_max_ch_prio;
				group_best_ch_idx = upper_subgrp_best_ch_idx;
			}

			/*for other cases, consider ext_ch*/
		} else {
			if (ext_ch == EXTCHA_ABOVE) {
				group_max_chprio = lower_subgrp_max_ch_prio;
				group_best_ch_idx = lower_subgrp_best_ch_idx;

			} else {
				group_max_chprio = upper_subgrp_max_ch_prio;
				group_best_ch_idx = upper_subgrp_best_ch_idx;
			}
		}

		if (group_best_ch_idx >= pAutoChSelCtrl->ChListNum) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"group_best_ch_idx:%d is out of range\n", group_best_ch_idx);
			continue;
		}

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"max busytime in group:%d is %d and channel priority is %u\n",
				GroupIdx, group_max_busytime, group_max_chprio);

		CandidateGroupNum++;

		/*find min group_max_busytime with highest priority in all groups*/
		if (group_max_busytime < best_group_busytime
			&& group_max_chprio == pAutoChSelCtrl->MaxChPriority) {

			best_group_busytime = group_max_busytime;
			CandidateGroupIdx = GroupIdx;
			CandidateCh = pAutoChSelCtrl->AutoChSelChList[group_best_ch_idx].Channel;
			best_ch_busytime = pChannelInfo->full_cca_time[group_best_ch_idx];
			CandidateChPrio = pAutoChSelCtrl->AutoChSelChList[group_best_ch_idx].Priority;

		} else if (group_max_busytime == best_group_busytime
					&& group_max_chprio == pAutoChSelCtrl->MaxChPriority) {

			if (pChannelInfo->full_cca_time[group_best_ch_idx] < best_ch_busytime) {
				CandidateGroupIdx = GroupIdx;
				CandidateCh = pAutoChSelCtrl->AutoChSelChList[group_best_ch_idx].Channel;
				best_ch_busytime = pChannelInfo->full_cca_time[group_best_ch_idx];
			}
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
			"CandidateGroupIdx is:%d, and CandidateCh is: %d, priority is %u\n",
			CandidateGroupIdx, CandidateCh, CandidateChPrio);

	return CandidateCh;
}

static inline UCHAR FindBestPSC6G(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev)
{
	UINT32 i = 0;
	UINT32 min_busy = 0xffffffff;
	UCHAR best_channel = FirstChannel(pAd, wdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	AUTOCH_SEL_CTRL *pAutoChSelCtrl = &pAutoChCtrl->AutoChSelCtrl;

	if (pAutoChSelCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "pAutoChSelCtrl is NULL.\n");
		return best_channel;
	}


	for (i = 0; i < pAutoChSelCtrl->ChListNum; i++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
				"Channel %d : full cca time = %u, Skip Channel = %s, Priority = %u\n",
				pAutoChSelCtrl->AutoChSelChList[i].Channel,
				pAutoChCtrl->pChannelInfo->full_cca_time[i],
				(pAutoChSelCtrl->AutoChSelChList[i].SkipChannel == TRUE)
				? "TRUE" : "FALSE",
				pAutoChSelCtrl->AutoChSelChList[i].Priority);
	}

	/*we get the best_channel from all PSC channel list*/
	/*the best_channel has the min full cca busytime*/
	for (i = 0; i < pAutoChSelCtrl->ChListNum; i++) {
		if (pAutoChSelCtrl->AutoChSelChList[i].SkipChannel == TRUE)
			continue;

		if ((pAutoChSelCtrl->AutoChSelChList[i].Priority == pAutoChSelCtrl->MaxChPriority)
			&& pAutoChCtrl->pChannelInfo->full_cca_time[i] < min_busy) {

			min_busy = pAutoChCtrl->pChannelInfo->full_cca_time[i];
			best_channel = pAutoChSelCtrl->AutoChSelChList[i].Channel;
		}
	}

	return best_channel;
}

static inline UCHAR SelectClearChannelForPSC6G(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev)
{
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	UCHAR bw = BW_20;
	UCHAR ch_band = wlan_config_get_ch_band(wdev);
	UCHAR best_ch = FirstChannel(pAd, wdev);
	AUTOCH_SEL_CTRL *pAutoChSelCtrl = &pAutoChCtrl->AutoChSelCtrl;

	if (pAutoChSelCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "pAutoChSelCtrl is NULL.\n");
		return best_ch;
	}

	if (ch_band != WIFI_CH_BAND_6G) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			 "ch_band=%d, not 6G BAND, return\n", ch_band);
		return best_ch;
	}


	if (pAutoChSelCtrl->ChListNum == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"ACS channel list is null\n");
		return best_ch;
	}

	bw = pAutoChSelCtrl->AutoChSelChList[0].Bw;

	/*when bw <= 80M, we choose the PSC ch with min full_cca busytime*/
	/*when bw > 80M, we need choose the best via PSC ch groups sorting*/
	if (bw <= BW_80)
		best_ch = FindBestPSC6G(pAd, wdev);
	else
		best_ch = FindBestPSC6GByGroupSorting(pAd, wdev);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
			"choose best 6G PSC ch: %d\n", best_ch);
	return best_ch;
}
#endif
/*
	==========================================================================
	Description:
		This routine sets the current PhyMode for calculating
		the dirtyness.
	Return:
		none
	NOTE:
	==========================================================================
 */
void CheckPhyModeIsABand(RTMP_ADAPTER *pAd)
{
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	USHORT PhyMode = HcGetRadioPhyModeByBandIdx(pAd);

	pAutoChCtrl->pChannelInfo->IsABand = (WMODE_CAP_5G(PhyMode)) ? TRUE : FALSE;
}


UCHAR SelectBestChannel(RTMP_ADAPTER *pAd, ChannelSel_Alg Alg, struct wifi_dev *pwdev)
{
	UCHAR ch = 0;
	/* init RadioCtrl.pChannelInfo->IsABand */
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
#ifdef ACS_CTCC_SUPPORT
	UCHAR cfg_ht_bw = 0;
	UCHAR cfg_vht_bw = 0;
	cfg_ht_bw = wlan_config_get_ht_bw(pwdev);
#ifdef DOT11_VHT_AC
	cfg_vht_bw = wlan_config_get_vht_bw(pwdev);
#endif
#endif
	CheckPhyModeIsABand(pAd);
#ifdef MICROWAVE_OVEN_SUPPORT

	if (Alg == ChannelAlgCCA)
		pAd->CommonCfg.MO_Cfg.bEnable = TRUE;

#endif /* MICROWAVE_OVEN_SUPPORT */

	switch (Alg) {
	case ChannelAlgRandom:
	case ChannelAlgApCnt:
		ch = SelectClearChannelApCnt(pAd, pwdev);
		break;

	case ChannelAlgCCA:
	/* select busytime alg*/
	case ChannelAlgBusyTime:
#ifdef ACS_CTCC_SUPPORT
		if ((WMODE_CAP_2G(pwdev->PhyMode) && (cfg_ht_bw == BW_20)) ||
			(WMODE_CAP_5G(pwdev->PhyMode) && (cfg_vht_bw == VHT_BW_80)) ||
			(WMODE_CAP_5G(pwdev->PhyMode) && (cfg_ht_bw == BW_40) && (cfg_vht_bw == VHT_BW_2040)))
			ch = select_clear_channel_busy_time(pAd, pwdev);
		else if (WMODE_CAP_5G(pwdev->PhyMode) && (cfg_vht_bw == VHT_BW_160))
			ch = select_clear_channel_bw_160M(pAd, pwdev);
		else
			ch = SelectClearChannelBusyTime(pAd, pwdev);
#else
#ifdef CONFIG_6G_SUPPORT
		if (pAutoChCtrl->AutoChSelCtrl.PSC_ACS)
			ch = SelectClearChannelForPSC6G(pAd, pwdev);
		else
#endif
			ch = SelectClearChannelBusyTime(pAd, pwdev);
#endif
		break;

	default:
#ifdef ACS_CTCC_SUPPORT
		if ((WMODE_CAP_2G(pwdev->PhyMode) && (cfg_ht_bw == BW_20)) ||
			(WMODE_CAP_5G(pwdev->PhyMode) && (cfg_vht_bw == VHT_BW_80)) ||
			(WMODE_CAP_5G(pwdev->PhyMode) && (cfg_ht_bw == BW_40) && (cfg_vht_bw == VHT_BW_2040)))
			ch = select_clear_channel_busy_time(pAd, pwdev);
		else if (WMODE_CAP_5G(pwdev->PhyMode) && (cfg_vht_bw == VHT_BW_160))
			ch = select_clear_channel_bw_160M(pAd, pwdev);
		else
			ch = SelectClearChannelBusyTime(pAd, pwdev);
#else
#ifdef CONFIG_6G_SUPPORT
		if (pAutoChCtrl->AutoChSelCtrl.PSC_ACS)
			ch = SelectClearChannelForPSC6G(pAd, pwdev);
		else
#endif
			ch = SelectClearChannelBusyTime(pAd, pwdev);
#endif
		break;
	}

	RTMPSendWirelessEvent(pAd, IW_CHANNEL_CHANGE_EVENT_FLAG, 0, 0, ch);
	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_SELECTED;
	pAutoChCtrl->AutoChSelCtrl.SelCh = ch;
	return ch;
}

VOID APAutoChannelInit(RTMP_ADAPTER *pAd, struct wifi_dev *pwdev)
{
	/* reset bss table */
	AutoChBssTableReset(pAd);
	/* clear Channel Info */
	ChannelInfoReset(pAd);
	/* init RadioCtrl.pChannelInfo->IsABand */
	CheckPhyModeIsABand(pAd);
#ifdef ACS_CTCC_SUPPORT
    build_acs_scan_ch_list(pAd, pwdev);
#endif
	pAd->ApCfg.current_channel_index = 0;
}

/*
	==========================================================================
	Description:
		This routine is called at initialization. It returns a channel number
		that complies to regulation domain and less interference with current
		enviornment.
	Return:
		ch -  channel number that
	NOTE:
		The retruned channel number is guaranteed to comply to current regulation
		domain that recorded in pAd->CommonCfg.CountryRegion
	==========================================================================
 */
UCHAR APAutoSelectChannel(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev,
	IN ChannelSel_Alg Alg,
	IN BOOLEAN IsABand)
{
	UCHAR ch = 0;

	if (pAd->phy_op && pAd->phy_op->AutoCh)
		ch = pAd->phy_op->AutoCh(pAd, pwdev, Alg, IsABand);

	return ch;
}

UCHAR MTAPAutoSelectChannel(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev,
	IN ChannelSel_Alg Alg,
	IN BOOLEAN IsABand)
{
	UCHAR ch = 0, i = 0, bss_idx = 0;
	struct wifi_dev *bss_wdev;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);

	if (pAutoChCtrl->AutoChSelCtrl.ACSChStat == ACS_CH_STATE_SELECTED) {
		ch = pAutoChCtrl->AutoChSelCtrl.SelCh;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "[MTAPAutoSelectChannel] ACS channel is selected, selected ch = %d\n", ch);
		return ch;
	}

	/* Init some structures before doing AutoChannelSelect() */
	APAutoChannelInit(pAd, pwdev);

#ifdef MICROWAVE_OVEN_SUPPORT
	pAd->CommonCfg.MO_Cfg.bEnable = FALSE;
	AsicMeasureFalseCCA(pAd);
#endif /* MICROWAVE_OVEN_SUPPORT */

	/* Re-arrange channel list and fill in channel properties for auto-channel selection*/
	AutoChSelBuildChannelList(pAd, pwdev);

	/* Disable beacon tx for all BSS during channel scan */
	for (bss_idx = 0; bss_idx < pAd->ApCfg.BssidNum; bss_idx++) {
		bss_wdev = &pAd->ApCfg.MBSSID[bss_idx].wdev;

		if (bss_wdev == NULL)
			continue;

		if (bss_wdev->bAllowBeaconing)
			UpdateBeaconHandler(pAd, bss_wdev, BCN_REASON(BCN_UPDATE_DISABLE_TX));
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
			"IsABand = %d, ChannelListNum = %d\n", IsABand, pAutoChCtrl->AutoChSelCtrl.ChListNum);
#ifdef ACS_CTCC_SUPPORT
	for (i = 0; i < pAutoChCtrl->pChannelInfo->channel_list_num; i++)
#else
	for (i = 0; i < pAutoChCtrl->AutoChSelCtrl.ChListNum; i++)
#endif
	{
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		ULONG wait_time = GET_V10_OFF_CHNL_TIME(pAd);
#else
		ULONG wait_time = 200; /* Wait for 200 ms at each channel. */
#endif

#ifdef ACS_CTCC_SUPPORT
#ifdef CONFIG_6G_SUPPORT
		if (pAutoChCtrl->AutoChSelCtrl.PSC_ACS)
			wlan_operate_full_bw_scan(pwdev, pAutoChCtrl->pChannelInfo->supp_ch_list[i].channel);
		else
#endif
			wlan_operate_scan(pwdev, pAutoChCtrl->pChannelInfo->supp_ch_list[i].channel);
#else
#ifdef CONFIG_6G_SUPPORT
		if (pAutoChCtrl->AutoChSelCtrl.PSC_ACS)
			wlan_operate_full_bw_scan(pwdev, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[i].Channel);
		else
#endif
			wlan_operate_scan(pwdev, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[i].Channel);
#endif
		pAd->ApCfg.current_channel_index = i;
		pAd->ApCfg.AutoChannel_Channel = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[i].Channel;

		UpdatePreACSInfo(pAd, pwdev);

#ifdef AP_QLOAD_SUPPORT
		/* QLOAD ALARM, ever alarm from QLOAD module */
		if (QLOAD_DOES_ALARM_OCCUR(pAd))
			wait_time = 400;
#endif /* AP_QLOAD_SUPPORT */
		OS_WAIT(wait_time);
		UpdateChannelInfo(pAd, i, Alg, pwdev);
	}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	if (IS_SUPPORT_V10_DFS(pAd) && pAutoChCtrl->pChannelInfo)
		/* Weighting Factor for ACS Enable Case */
		DfsV10AddWeighingFactor(pAd, pwdev);
#endif

	/* Enable beacon tx for all BSS */
	for (bss_idx = 0; bss_idx < pAd->ApCfg.BssidNum; bss_idx++) {
		bss_wdev = &pAd->ApCfg.MBSSID[bss_idx].wdev;

		if (bss_wdev == NULL)
			continue;

		if (bss_wdev->bAllowBeaconing)
			UpdateBeaconHandler(pAd, bss_wdev, BCN_REASON(BCN_UPDATE_ENABLE_TX));
	}

	ch = SelectBestChannel(pAd, Alg, pwdev);
	return ch;
}

/*
   ==========================================================================
   Description:
       Update channel to wdev which is supported for A-band or G-band.

    Return:
	None.
   ==========================================================================
 */
VOID AutoChSelUpdateChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel,
	IN BOOLEAN IsABand,
	IN struct wifi_dev *pwdev)
{
	UINT8 ExtChaDir;
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	UCHAR i = 0;
	struct wifi_dev *wdev = NULL;
	UCHAR band_idx, BandIdx;

	if (pwdev)
		band_idx = HcGetBandByWdev(pwdev);
#endif

	if (IsABand) {
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
#ifdef CONFIG_AP_SUPPORT
		if (IS_SUPPORT_V10_DFS(pAd)) {
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {

				for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					wdev = &pAd->ApCfg.MBSSID[i].wdev;
					BandIdx = HcGetBandByWdev(wdev);
					if (band_idx == BandIdx)
						wdev->channel = Channel;
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
						"BSS%d Channel=%d\n", i, wdev->channel);
				}
			}
		} else
#endif /* CONFIG_AP_SUPPORT */
#endif

		/*5G Channel*/
		pwdev->channel = Channel;
	} else {
		/*2G Channel*/
		/* Update primary channel in wdev */
		pwdev->channel = Channel;

		/* Query ext_cha in wdev */
		ExtChaDir = wlan_config_get_ext_cha(pwdev);

		/* Check current extension channel */
		if (!ExtChCheck(pAd, Channel, ExtChaDir, pwdev)) {
			if (ExtChaDir == EXTCHA_BELOW)
				ExtChaDir = EXTCHA_ABOVE;
			else
				ExtChaDir = EXTCHA_BELOW;

			/* Update ext_cha in wdev */
			wlan_config_set_ext_cha(pwdev, ExtChaDir);
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
	"Update channel for wdev for this band PhyMode = %d, Channel = %d\n",
	pwdev->PhyMode, pwdev->channel);
}

/*
   ==========================================================================
   Description:
       Build channel list for auto-channel selection.

    Return:
	None.
   ==========================================================================
 */
VOID AutoChSelBuildChannelList(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev)
{
	USHORT PhyMode = pwdev->PhyMode;
	UCHAR ucChBand = WIFI_CH_BAND_NUM;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);

	/* Initialize channel list*/
	os_zero_mem(pAutoChCtrl->AutoChSelCtrl.AutoChSelChList, (MAX_NUM_OF_CHANNELS+1) * sizeof(AUTOCH_SEL_CH_LIST));

	if (WMODE_CAP_6G(PhyMode))
		ucChBand = WIFI_CH_BAND_6G;
	else if (WMODE_CAP_2G(PhyMode))
		ucChBand = WIFI_CH_BAND_2G;
	else if (WMODE_CAP_5G(PhyMode))
		ucChBand = WIFI_CH_BAND_5G;

	if ((ucChBand == WIFI_CH_BAND_5G) || ucChBand == WIFI_CH_BAND_6G) {
		pAutoChCtrl->AutoChSelCtrl.IsABand = TRUE;

		/* Build 5G or 6G channel list used by ACS */
		AutoChSelBuildChannelListFor56G(pAd, pwdev, ucChBand);
	} else if (ucChBand == WIFI_CH_BAND_2G) {
		pAutoChCtrl->AutoChSelCtrl.IsABand = FALSE;

		/* Build 2G channel list used by ACS */
		AutoChSelBuildChannelListFor2G(pAd, pwdev);
	}
}
/*
   ==========================================================================
   Description:
       Build channel list for 2.4G according to 1) Country Region 2) RF IC type for auto-channel selection.

    Return:
	None.
   ==========================================================================
 */
VOID AutoChSelBuildChannelListFor2G(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev)
{
	INT ChIdx, ChListNum = 0;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(pwdev);
	AUTOCH_SEL_CH_LIST *pACSChList;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	pAutoChCtrl->AutoChSelCtrl.ChListNum = pChCtrl->ChListNum;

	/* Initialize local ACS channel list*/
	os_alloc_mem(NULL, (UCHAR **)&pACSChList, (MAX_NUM_OF_CHANNELS+1) * sizeof(AUTOCH_SEL_CH_LIST));
	if (pACSChList == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
				"Fail to allocate memory!\n");
		return;
	}
	os_zero_mem(pACSChList, (MAX_NUM_OF_CHANNELS+1) * sizeof(AUTOCH_SEL_CH_LIST));

	for (ChIdx = 0; ChIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChIdx++) {
		pACSChList[ChIdx].Channel = pChCtrl->ChList[ChIdx].Channel;
		pACSChList[ChIdx].Priority = pChCtrl->ChList[ChIdx].Priority;
	}


	for (ChIdx = 0; ChIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChIdx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				 "Ch = %3d\n", pACSChList[ChIdx].Channel);
	}

	/* Check for skip-channel list */
	for (ChIdx = 0; ChIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChIdx++)
		pACSChList[ChIdx].SkipChannel = AutoChannelSkipListCheck(pAd, pACSChList[ChIdx].Channel);

	for (ChIdx = 0; ChIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChIdx++) {
		/* 2.4G only support for BW20 auto-channel selection */
		pACSChList[ChIdx].Bw = BW_20;
		pACSChList[ChIdx].CentralChannel = pACSChList[ChIdx].Channel;

		if (cfg_ht_bw == BW_20)
			pACSChList[ChIdx].BwCap = TRUE;
#ifdef DOT11_N_SUPPORT
		else if ((cfg_ht_bw == BW_40)
			&& N_ChannelGroupCheck(pAd, pACSChList[ChIdx].Channel, pwdev))
			pACSChList[ChIdx].BwCap = TRUE;
#endif /* DOT11_N_SUPPORT */
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "[AutoChSelBuildChannelListFor2G] - ChIdx = %d,  ChListNum = %d\n", ChIdx, pAutoChCtrl->AutoChSelCtrl.ChListNum);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				 "PrimChannel =  %3d, CenChannel = %3d, BW= %d, BwCap= %d, SkipChannel= %d, ChannelPriority= %d\n",
				  pACSChList[ChIdx].Channel, pACSChList[ChIdx].CentralChannel,
				  pACSChList[ChIdx].Bw, pACSChList[ChIdx].BwCap,
				  pACSChList[ChIdx].SkipChannel, pACSChList[ChIdx].Priority);
	}

	for (ChIdx = 0; ChIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChIdx++) {
		if ((pACSChList[ChIdx].SkipChannel == TRUE) || (pACSChList[ChIdx].BwCap == FALSE || pACSChList[ChIdx].Priority == 0))
			continue;
		else {
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Channel = pACSChList[ChIdx].Channel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Bw = pACSChList[ChIdx].Bw;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].BwCap = pACSChList[ChIdx].BwCap;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].CentralChannel = pACSChList[ChIdx].CentralChannel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].SkipChannel = pACSChList[ChIdx].SkipChannel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Flags = pACSChList[ChIdx].Flags;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Priority = pACSChList[ChIdx].Priority;
			ChListNum++;
		}
	}
#ifdef OCE_SUPPORT

		if (IS_OCE_ENABLE(pwdev)) {
			ChListNum = 0;
			for (ChIdx = 0; ChIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChIdx++) {
				if (pACSChList[ChIdx].Channel != 1 && pACSChList[ChIdx].Channel != 6 &&
					pACSChList[ChIdx].Channel != 11)
					continue;
				else {
					pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Channel =
						pACSChList[ChIdx].Channel;
					pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Bw =
						pACSChList[ChIdx].Bw;
					pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].BwCap = TRUE;
					pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].CentralChannel =
						pACSChList[ChIdx].CentralChannel;
					pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].SkipChannel =
						pACSChList[ChIdx].SkipChannel;
					pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Flags =
						pACSChList[ChIdx].Flags;
					pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Priority =
						pACSChList[ChIdx].Priority;
					ChListNum++;
				}
			}
			if (ChListNum == 0) {
				for (ChIdx = 1; ChIdx <= 11; ChIdx += 5) {
					pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Channel =
						pACSChList[ChIdx].Channel;
					pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Bw = pACSChList[ChIdx].Bw;
					pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].BwCap = pACSChList[ChIdx].BwCap;
					pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].CentralChannel =
						pACSChList[ChIdx].CentralChannel;
					pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].SkipChannel =
						pACSChList[ChIdx].SkipChannel;
					pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Flags = pACSChList[ChIdx].Flags;
					pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Priority =
						pACSChList[ChIdx].Priority;
					ChListNum++;
				}
			}
		}
#endif /* OCE_SUPPORT */

	pAutoChCtrl->AutoChSelCtrl.ChListNum = ChListNum;
	os_free_mem(pACSChList);
}

static VOID update_channel_bw_by_group(AUTOCH_SEL_CH_LIST *pACSChList, UCHAR ChIdx, UCHAR ChNum, UCHAR Bw, UCHAR CentralChannel)
{
	UCHAR idx;

	for (idx = 0; idx < ChNum; idx++) {
		pACSChList[ChIdx + idx].BwCap = TRUE;
		pACSChList[ChIdx + idx].Bw = Bw;
		pACSChList[ChIdx + idx].CentralChannel = CentralChannel;
		pACSChList[ChIdx + idx].BuildDone = TRUE;
	}
}

VOID build_acs_scan_ch_list_by_bw(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev,
	IN UCHAR ucChBand,
	IN struct _AUTOCH_SEL_CH_LIST *pACSChList,
	IN UCHAR bw)
{
#define EXT_ABOVE     1
#define EXT_BELOW    -1

	UINT ChIdx = 0, ChListNum56G = 0, ChListNum = 0;
	UCHAR CentralChannel, CurMaxChPrio = 0;

	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
#ifdef DOT11_VHT_AC
	INT idx = 0, ch_layout_size[3] = {0};
	struct vht_ch_layout *vht_ch_80M = get_ch_array(BW_80, ucChBand, &(ch_layout_size[0]));
	struct vht_ch_layout *vht_ch_160M = get_ch_array(BW_160, ucChBand, &(ch_layout_size[1]));
#ifdef DOT11_EHT_BE
	struct eht_ch_layout *eht_ch_320M = get_eht_ch_array(BW_320, ucChBand, &(ch_layout_size[2]));
#endif
#endif/* DOT11_VHT_AC */

	for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
		/*Skip invalid channel*/
		if (!IsValidChannel(pAd, pChCtrl->ChList[ChIdx].Channel, pwdev))
			continue;

		if (AutoChannelSkipListCheck(pAd, pChCtrl->ChList[ChIdx].Channel))
			continue;

#ifdef BACKGROUND_SCAN_SUPPORT
		if (pAd->BgndScanCtrl.SkipDfsChannel && RadarChannelCheck(pAd, pChCtrl->ChList[ChIdx].Channel))
			continue;
#endif /* BACKGROUND_SCAN_SUPPORT */

#ifdef WIFI_MD_COEX_SUPPORT
		if (!IsChannelSafe(pAd, pChCtrl->ChList[ChIdx].Channel) && !IsPwrChannelSafe(pAd, pChCtrl->ChList[ChIdx].Channel))
			continue;
#endif

		pACSChList[ChListNum56G].Channel = pChCtrl->ChList[ChIdx].Channel;
		pACSChList[ChListNum56G].CentralChannel = pChCtrl->ChList[ChIdx].Channel;
		pACSChList[ChListNum56G].BwCap = FALSE;
		pACSChList[ChListNum56G].BuildDone = FALSE;
		pACSChList[ChListNum56G].SkipChannel = FALSE;
		pACSChList[ChListNum56G].Priority = pChCtrl->ChList[ChIdx].Priority;

#ifdef CONFIG_6G_SUPPORT
		pACSChList[ChListNum56G].PSC_Ch = pChCtrl->ChList[ChIdx].PSC_Ch;
#endif
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "Ch(%d), Prio(%d)\n",
			pACSChList[ChIdx].Channel, pACSChList[ChListNum56G].Priority);
		ChListNum56G++;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "ChListNum56G = %d\n", ChListNum56G);

	/* Build ACS channel list */
	for (ChIdx = 0; ChIdx < ChListNum56G; ChIdx++) {
		if (bw == BW_20
#ifdef CONFIG_6G_AFC_SUPPORT
			&& afc_check_valid_channel_by_bw(pAd, pACSChList[ChIdx].Channel, BW_20)
#endif
		) {
			pACSChList[ChIdx].Bw = BW_20;
			pACSChList[ChIdx].BwCap = TRUE;
		} else if (bw == BW_40
#ifdef CONFIG_6G_AFC_SUPPORT
			  && afc_check_valid_channel_by_bw(pAd, pACSChList[ChIdx].Channel, BW_40)
#endif
		) {
			if ((Get56GBandChOffset(pACSChList[ChIdx].Channel, ucChBand) == EXT_ABOVE)
				&& ((ChIdx + 1) < ChListNum56G)
				&& (pACSChList[ChIdx + 1].Channel == (pACSChList[ChIdx].Channel + 4))) {
				CentralChannel = pACSChList[ChIdx].Channel + 2;
				update_channel_bw_by_group(pACSChList, ChIdx, 2, BW_40, CentralChannel);
				continue;
			}
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
			if ((IS_SUPPORT_V10_DFS(pAd) && pACSChList[ChIdx].Channel == 140 &&
				pAd->CommonCfg.bCh144Enabled == FALSE) || (IS_SUPPORT_V10_DFS(pAd) &&
				pACSChList[ChIdx].Channel == 144 && pAd->CommonCfg.bCh144Enabled == TRUE))
				pACSChList[ChIdx].BwCap = TRUE;
#endif

			pACSChList[ChIdx].Bw = BW_40;
		} else if (bw == BW_80
#ifdef CONFIG_6G_AFC_SUPPORT
			  && afc_check_valid_channel_by_bw(pAd, pACSChList[ChIdx].Channel, BW_80)
#endif
		) {

			for (idx = 0; idx < ch_layout_size[0]; idx++) {
				if ((ChIdx + 3) < ChListNum56G
					&& pACSChList[ChIdx].Channel == vht_ch_80M[idx].ch_low_bnd
					&& pACSChList[ChIdx+3].Channel == vht_ch_80M[idx].ch_up_bnd) {
					CentralChannel = vht_cent_ch_freq(pACSChList[ChIdx].Channel, VHT_BW_80, ucChBand);
					update_channel_bw_by_group(pACSChList, ChIdx, 4, BW_80, CentralChannel);
					break;
				}
			}
#ifdef ACS_CTCC_SUPPORT
			if (idx < ch_layout_size[0])
				pACSChList[ChIdx].BwCap = TRUE;
#endif
			pACSChList[ChIdx].Bw = BW_80;
		} else if (bw == BW_160
#ifdef CONFIG_6G_AFC_SUPPORT
			  && afc_check_valid_channel_by_bw(pAd, pACSChList[ChIdx].Channel, BW_160)
#endif
		) {
			for (idx = 0; idx < ch_layout_size[1]; idx++) {
				if ((ChIdx + 7) < ChListNum56G
					&& pACSChList[ChIdx].Channel == vht_ch_160M[idx].ch_low_bnd
					&& pACSChList[ChIdx+7].Channel == vht_ch_160M[idx].ch_up_bnd) {
					CentralChannel = vht_cent_ch_freq(pACSChList[ChIdx].Channel, VHT_BW_160, ucChBand);
					update_channel_bw_by_group(pACSChList, ChIdx, 8, BW_160, CentralChannel);
					break;
				}
			}
			pACSChList[ChIdx].Bw = BW_160;
		}
#ifdef DOT11_EHT_BE
		else if (bw == BW_320
#ifdef CONFIG_6G_AFC_SUPPORT
			&& afc_check_valid_channel_by_bw(pAd, pACSChList[ChIdx].Channel, BW_320)
#endif
		) {
			for (idx = 0; idx < ch_layout_size[2]; idx++) {
				if ((ChIdx + 15) < ChListNum56G
					&& pACSChList[ChIdx].Channel == eht_ch_320M[idx].ch_low_bnd
					&& pACSChList[ChIdx+15].Channel == eht_ch_320M[idx].ch_up_bnd) {
					CentralChannel = eht_cent_ch_freq(pwdev, pACSChList[ChIdx].Channel, EHT_BW_320, ucChBand);
					update_channel_bw_by_group(pACSChList, ChIdx, 16, BW_320, CentralChannel);
					break;
				}
			}
			pACSChList[ChIdx].Bw = BW_320;
		}
#endif
	}

	/*Show ACS channel list*/
	for (ChIdx = 0; ChIdx < ChListNum56G; ChIdx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"PrimChannel=%3d, CenChannel=%3d, BW=%d, BwCap=%d, SkipChannel=%d, BuildDone=%d, ChannelPriority=%d\n",
				pACSChList[ChIdx].Channel, pACSChList[ChIdx].CentralChannel,
				pACSChList[ChIdx].Bw, pACSChList[ChIdx].BwCap,
				pACSChList[ChIdx].SkipChannel,
				pACSChList[ChIdx].BuildDone,
				pACSChList[ChIdx].Priority);
	}

	/*Set channel list of auto channel selection*/
	for (ChIdx = 0; ChIdx < ChListNum56G; ChIdx++) {
		if (pACSChList[ChIdx].BwCap == FALSE)
			continue;
#ifdef CONFIG_6G_SUPPORT
		else if ((ucChBand == WIFI_CH_BAND_6G) &&
			(pAutoChCtrl->AutoChSelCtrl.PSC_ACS == TRUE) &&
			!pACSChList[ChIdx].PSC_Ch)
			/*Skip no-PSC channel if PSC_ACS enable */
			continue;
#endif
		else {

			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Bw = pACSChList[ChIdx].Bw;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Channel = pACSChList[ChIdx].Channel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].BwCap = pACSChList[ChIdx].BwCap;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].CentralChannel = pACSChList[ChIdx].CentralChannel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].SkipChannel = pACSChList[ChIdx].SkipChannel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Flags = pACSChList[ChIdx].Flags;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Priority = pACSChList[ChIdx].Priority;
			CurMaxChPrio = CurMaxChPrio < pACSChList[ChIdx].Priority ? pACSChList[ChIdx].Priority : CurMaxChPrio;

			ChListNum++;
		}
	}

	/*if the ACS max channel priority is lower than
	the basic channel list's max priority*/
	if (CurMaxChPrio < pAutoChCtrl->AutoChSelCtrl.MaxChPriority)
		ChListNum = 0;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE, "ChListNum(%d).\n", ChListNum);

/*when the acs channellist is null, the ACS channel list is re-build with reduced BandWidth*/
	if (ChListNum == 0) {
		if (bw == BW_320)
			build_acs_scan_ch_list_by_bw(pAd, pwdev, ucChBand, pACSChList, BW_160);
		else if (bw == BW_160)
			build_acs_scan_ch_list_by_bw(pAd, pwdev, ucChBand, pACSChList, BW_80);
		/*consider whether 40MHz is allowed or not*/
		else if (bw == BW_80) {
			if (Is40MHzForbid(pChCtrl))
				build_acs_scan_ch_list_by_bw(pAd, pwdev, ucChBand, pACSChList, BW_20);
			else
				build_acs_scan_ch_list_by_bw(pAd, pwdev, ucChBand, pACSChList, BW_40);
		}
		else if (bw == BW_40)
			build_acs_scan_ch_list_by_bw(pAd, pwdev, ucChBand, pACSChList, BW_20);

		/*if can't build list in 20MHz, check whether is caused by chprio*/
		else if (CurMaxChPrio < pAutoChCtrl->AutoChSelCtrl.MaxChPriority) {

			/*lower the chprio and then rebuild it*/
			pAutoChCtrl->AutoChSelCtrl.MaxChPriority--;
			build_acs_scan_ch_list_by_bw(pAd, pwdev, ucChBand, pACSChList, ACS_final_bw);

			/*if it is not caused by chprio, notify user*/
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE, "The BW(%d) is wrong!!\n", bw);
		}
	} else {
		pAutoChCtrl->AutoChSelCtrl.ChListNum = ChListNum;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE, "ACSChListNum = %d, ACSChBW = %d\n", ChListNum, bw);
	}
}


/*
   ==========================================================================
   Description:
       Build channel list for 5G, 6G according to 1) Country Region 2) RF IC type for
       auto-channel selection.

    Return:
	None.
   ==========================================================================
 */
VOID AutoChSelBuildChannelListFor56G(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev,
	IN UCHAR ucChBand)
{

	AUTOCH_SEL_CH_LIST *pACSChList;
	UCHAR bw = BW_20;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(pwdev);
	UCHAR cfg_vht_bw = wlan_config_get_vht_bw(pwdev);
	UCHAR cfg_eht_bw = EHT_BW_MAX;
	CHANNEL_CTRL * pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);

#ifdef DOT11_EHT_BE
		cfg_eht_bw = wlan_config_get_eht_bw(pwdev);
#endif

	if (!pChCtrl) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "can't find pChCtrl !\n");
		return;
	}

#ifdef CONFIG_6G_AFC_SUPPORT
	if (WMODE_CAP_6G(pwdev->PhyMode))
		afc_get_available_bw(pAd, &cfg_vht_bw, &cfg_ht_bw, &cfg_eht_bw);
#endif

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "cfg_ht_bw = %d, cfg_vht_bw = %d, cfg_eht_bw = %d\n", cfg_ht_bw, cfg_vht_bw, cfg_eht_bw);

	/* Initialize local ACS channel list*/
	os_alloc_mem(NULL, (UCHAR **)&pACSChList, (MAX_NUM_OF_CHANNELS+1) * sizeof(AUTOCH_SEL_CH_LIST));
	if (pACSChList == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
				"Fail to allocate memory!\n");
		return;
	}
	os_zero_mem(pACSChList, (MAX_NUM_OF_CHANNELS+1) * sizeof(AUTOCH_SEL_CH_LIST));


	if (cfg_ht_bw != BW_40)
		bw = BW_20;
	else if (cfg_vht_bw == VHT_BW_2040)
		bw = BW_40;
	else if ((cfg_vht_bw == VHT_BW_80) || (cfg_vht_bw == VHT_BW_8080))
		bw = BW_80;
	else if (cfg_vht_bw == VHT_BW_160
#ifdef DOT11_EHT_BE
			&& (cfg_eht_bw != EHT_BW_320)
#endif
	)
		bw = BW_160;
#ifdef DOT11_EHT_BE
	else if (cfg_eht_bw == EHT_BW_320)
		bw = BW_320;
#endif /* DOT11_EHT_BE */
	else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"cfg bw is wrong!\n");

	if (Is40MHzForbid(pChCtrl) && bw == BW_40)
		bw = BW_20;

	/*update the max channel priority that ACS should select
	and record the current bw*/
	pAutoChCtrl->AutoChSelCtrl.MaxChPriority = GetMaxChannelPriority(pAd);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
		"acs_max_prio(%d), ch_max_prio(%d)\n",
		pAutoChCtrl->AutoChSelCtrl.MaxChPriority, GetMaxChannelPriority(pAd));
	ACS_final_bw = bw;
	build_acs_scan_ch_list_by_bw(pAd, pwdev, ucChBand, pACSChList, bw);
	os_free_mem(pACSChList);
}



/*
	==========================================================================
	Description:

	Return:
		ScanChIdx - Channel index which is mapping to related channel to be scanned.
	Note:
		return -1 if no more next channel
	==========================================================================
 */
CHAR AutoChSelFindScanChIdx(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev,
	IN CHAR LastScanChIdx)
{
	CHAR ScanChIdx;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);

	if (LastScanChIdx == -1)
		ScanChIdx = 0;
	else {
		ScanChIdx = LastScanChIdx + 1;
#ifdef ACS_CTCC_SUPPORT
		if (ScanChIdx >= pAutoChCtrl->pChannelInfo->channel_list_num)
#else
		if (ScanChIdx >= pAutoChCtrl->AutoChSelCtrl.ChListNum)
#endif
			ScanChIdx = -1;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
			 "LastScanChIdx = %d, ScanChIdx = %d, ChannelListNum = %d\n",
			 LastScanChIdx, ScanChIdx, pAutoChCtrl->AutoChSelCtrl.ChListNum);
	return ScanChIdx;
}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
UINT8 SelectBestV10Chnl_From_List(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR band_idx)
{
	UCHAR Best_Channel = 0;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR i = 0;
	UCHAR idx;
	UCHAR BandIdx = BAND0;
	struct wifi_dev *wdev;

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		BandIdx = HcGetBandByWdev(wdev);
		if (band_idx == BandIdx)
			break;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
			"IS_DFS_V10_ACS_VALID(pAd) = %d\n", IS_DFS_V10_ACS_VALID(pAd));

	if (IS_DFS_V10_ACS_VALID(pAd)) {
		/* Fetch Channel from ACS Channel List */
		for (i = 0; i < ((wlan_config_get_vht_bw(wdev) == VHT_BW_2040)
			? (V10_TOTAL_CHANNEL_COUNT) : (pDfsParam->GroupCount)); i++) {
			if (pDfsParam->DfsV10SortedACSList[i].isConsumed) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "Channel %d Consumed\n",
					pDfsParam->DfsV10SortedACSList[i].Channel);
				continue;
			}

			if (!CheckNonOccupancyChannel(pAd, wdev, pDfsParam->DfsV10SortedACSList[i].Channel)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO, "Channel %d NOP\n",
					pDfsParam->DfsV10SortedACSList[i].Channel);
				continue;
			}

			Best_Channel = pDfsParam->DfsV10SortedACSList[i].Channel;
			pDfsParam->DfsV10SortedACSList[i].isConsumed = TRUE;
			break;
		}
	}
	return Best_Channel;
}
#endif

#ifdef OFFCHANNEL_SCAN_FEATURE
VOID ChannelInfoResetNew(
	IN PRTMP_ADAPTER	pAd)
{
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	CHANNELINFO *ch_info = pAutoChCtrl->pChannelInfo;

	if (ch_info) {
		NdisZeroMemory(ch_info, sizeof(CHANNELINFO));
		pAd->ApCfg.current_channel_index = 0;
	}
}
#endif

/*
	==========================================================================
	Description:
		Scan next channel
	==========================================================================
 */
VOID AutoChSelScanNextChannel(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev)
{
	RALINK_TIMER_STRUCT *ScanTimer;
	CHAR Idx;
	ULONG wait_time = 200; /* Wait for 200 ms at each channel. */
	UCHAR NewCh = 0;
	UCHAR bss_idx = 0;
	struct wifi_dev *bss_wdev = NULL;
	INT ret;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
#ifdef TR181_SUPPORT
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
#endif /*TR181_SUPPORT*/
#ifdef ACS_CTCC_SUPPORT
	UCHAR cfg_ht_bw = 0;
	UCHAR cfg_vht_bw = 0;
	cfg_ht_bw = wlan_config_get_ht_bw(pwdev);
#ifdef DOT11_VHT_AC
	cfg_vht_bw = wlan_config_get_vht_bw(pwdev);
#endif
#endif
	ScanTimer = &pAutoChCtrl->AutoChSelCtrl.AutoChScanTimer;
	Idx = pAutoChCtrl->AutoChSelCtrl.ScanChIdx;

#ifdef AP_QLOAD_SUPPORT

	/* QLOAD ALARM, ever alarm from QLOAD module */
	if (QLOAD_DOES_ALARM_OCCUR(pAd))
		wait_time = 400;

#endif /* AP_QLOAD_SUPPORT */

	if (pAutoChCtrl->AutoChSelCtrl.ScanChIdx == -1) {
#ifdef ACS_CTCC_SUPPORT
		if ((WMODE_CAP_2G(pwdev->PhyMode) && (cfg_ht_bw == BW_20)) ||
			(WMODE_CAP_5G(pwdev->PhyMode) && (cfg_vht_bw == VHT_BW_80)) ||
			(WMODE_CAP_5G(pwdev->PhyMode) && (cfg_ht_bw == BW_40) && (cfg_vht_bw == VHT_BW_2040)))
			NewCh = select_clear_channel_busy_time(pAd, pwdev);
		else if (WMODE_CAP_5G(pwdev->PhyMode) && (cfg_vht_bw == VHT_BW_160))
			NewCh = select_clear_channel_bw_160M(pAd, pwdev);
		else
			NewCh = SelectClearChannelBusyTime(pAd, pwdev);

#else
#ifdef CONFIG_6G_SUPPORT
		if (pAutoChCtrl->AutoChSelCtrl.PSC_ACS)
			NewCh = SelectClearChannelForPSC6G(pAd, pwdev);
		else
#endif
			if (pAd->ApCfg.AutoChannelAlg == ChannelAlgApCnt)
				NewCh = SelectClearChannelApCnt(pAd, pwdev);
			else
				NewCh = SelectClearChannelBusyTime(pAd, pwdev);
#endif

		if (!pAd->ApCfg.auto_ch_score_flag)
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
					"Current channel = %d , selected new channel = %d\n", pwdev->channel, NewCh);

#ifdef AP_SCAN_SUPPORT
		scan_ch_restore(pAd, OPMODE_AP, pwdev); /* Restore original channel */
#endif /* AP_SCAN_SUPPORT */
		/* Enable beacon tx for all BSS */
		for (bss_idx = 0; bss_idx < pAd->ApCfg.BssidNum; bss_idx++) {
			bss_wdev = &pAd->ApCfg.MBSSID[bss_idx].wdev;

			if (bss_wdev == NULL)
				continue;

			if (bss_wdev->bAllowBeaconing)
				UpdateBeaconHandler(pAd, bss_wdev, BCN_REASON(BCN_UPDATE_ENABLE_TX));
		}
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "Enable all beacon!\n");
		if (!pAd->ApCfg.auto_ch_score_flag)
		{
			if (NewCh != pwdev->channel) {
				ret = rtmp_set_channel(pAd, pwdev, NewCh);
			    if (!ret) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
							"Fail to set channel !!\n");
			    }
#ifdef TR181_SUPPORT
				else if (ctrl) {
					if (ctrl->rdev.pRadioCtrl->ACSTriggerFlag == 2) {
						/*ACS triggered by manual command*/
						ctrl->rdev.pRadioCtrl->ForceACSChannelChangeCount++;
						ctrl->rdev.pRadioCtrl->TotalChannelChangeCount++;
						ctrl->rdev.pRadioCtrl->ACSTriggerFlag = 0;
					} else if (ctrl->rdev.pRadioCtrl->ACSTriggerFlag == 1) {
						/*ACS triggered by periodic refresh*/
						ctrl->rdev.pRadioCtrl->RefreshACSChannelChangeCount++;
						ctrl->rdev.pRadioCtrl->TotalChannelChangeCount++;
						ctrl->rdev.pRadioCtrl->ACSTriggerFlag = 0;
					}
				}
#endif /*TR181_SUPPORT*/
			}
		}
		RTMPSendWirelessEvent(pAd, IW_CHANNEL_CHANGE_EVENT_FLAG, 0, 0, 0);
		pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_SELECTED;
		/* Update current state from listen state to idle. */
		if (pAd->ApCfg.set_ch_async_flag == FALSE) {
			pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState = AUTO_CH_SEL_SCAN_IDLE;
			if (pAd->ApCfg.iwpriv_event_flag)
				RTMP_OS_COMPLETE(&pAd->ApCfg.set_ch_aync_done);
		}
		pAd->ApCfg.auto_ch_score_flag = FALSE;
		/* Enable MibBucket after ACS done */
		pAd->MsMibBucket.Enabled = TRUE;
		pAd->OneSecMibBucket.Enabled = TRUE;
	} else if (pAutoChCtrl->AutoChSelCtrl.PartialScanACS && pAutoChCtrl->AutoChSelCtrl.RestoreFlag) {
#ifdef AP_SCAN_SUPPORT
		ACS_Restore_During_Partial_Scan(pAd, pwdev); /* Restore original channel */
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN, "Restore channel!\n");
#endif /* AP_SCAN_SUPPORT */
	} else {
		/* Update current state from idle state to listen. */
		pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState = AUTO_CH_SEL_SCAN_LISTEN;
		if (Idx < 0 || Idx > MAX_NUM_OF_CHANNELS) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
					"Invalid Scan Channel Index(%d)\n", Idx);
			return;
		}
#ifdef ACS_CTCC_SUPPORT
#ifdef CONFIG_6G_SUPPORT
		if (pAutoChCtrl->AutoChSelCtrl.PSC_ACS)
			wlan_operate_full_bw_scan(pwdev, pAutoChCtrl->pChannelInfo->supp_ch_list[Idx].channel);
		else
#endif
			wlan_operate_scan(pwdev, pAutoChCtrl->pChannelInfo->supp_ch_list[Idx].channel);
#else
		if (pAd->ApCfg.current_channel_index < pAutoChCtrl->AutoChSelCtrl.ChListNum)
			pAd->ApCfg.AutoChannel_Channel = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[Idx].Channel;
#ifdef CONFIG_6G_SUPPORT
		if (pAutoChCtrl->AutoChSelCtrl.PSC_ACS)
			wlan_operate_full_bw_scan(pwdev, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[Idx].Channel);
		else
#endif
			wlan_operate_scan(pwdev, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[Idx].Channel);
#endif
		/*check ScanningDwell time is configured*/
		if (pAutoChCtrl->AutoChSelCtrl.ScanningDwell != 0)
			wait_time = pAutoChCtrl->AutoChSelCtrl.ScanningDwell;

		/* Setting the RestoreFlag through the count of scanning channel continouosly. */
		if (pAutoChCtrl->AutoChSelCtrl.PartialScanACS) {
			if (pAutoChCtrl->AutoChSelCtrl.CountofCh < (pAutoChCtrl->AutoChSelCtrl.ScanningChNum - 1)) {
				pAutoChCtrl->AutoChSelCtrl.CountofCh++;
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
					"AutoChSelCtrl.CountofCh = %d\n", pAutoChCtrl->AutoChSelCtrl.CountofCh);
			}

			else {
				pAutoChCtrl->AutoChSelCtrl.CountofCh = 0;
				pAutoChCtrl->AutoChSelCtrl.RestoreFlag = TRUE;
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
					"Rest restore flag and CountofCh!\n");
			}

		}
		UpdatePreACSInfo(pAd, pwdev);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
			"pre_obss_time = %u, pre_tx_air_time = %u, pre_rx_air_time = %u\n",
			pAutoChCtrl->AutoChSelCtrl.pre_obss_time,
			pAutoChCtrl->AutoChSelCtrl.pre_tx_air_time,
			pAutoChCtrl->AutoChSelCtrl.pre_rx_air_time);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
			"pre_non_wifi_time = %u, pre_cca_nav_tx_time = %u, pre_pcca_time = %u\n",
			pAutoChCtrl->AutoChSelCtrl.pre_non_wifi_time,
			pAutoChCtrl->AutoChSelCtrl.pre_cca_nav_tx_time,
			pAutoChCtrl->AutoChSelCtrl.pre_pcca_time);

		RTMPSetTimer(ScanTimer, wait_time);
    }
}

/*
    ==========================================================================
    Description:
	Auto-channel selection SCAN req state machine procedure
    ==========================================================================
 */
VOID AutoChSelScanReqAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM * pElem)
{
	BOOLEAN	Cancelled;
	AUTO_CH_CTRL *pAutoChCtrl;
	struct wifi_dev *pwdev = (struct wifi_dev *)pElem->wdev;

	if (!pwdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "AutoChSelScanReqAction - pwdev == NULL \n");
		return;
	}
	pAutoChCtrl = HcGetAutoChCtrl(pAd);

	RTMPCancelTimer(&pAutoChCtrl->AutoChSelCtrl.AutoChScanTimer, &Cancelled);
#ifdef ACS_CTCC_SUPPORT
	APAutoChannelInit(pAd, pwdev);
#endif
	AutoChSelScanNextChannel(pAd, pwdev);
}

/*
    ==========================================================================
    Description:
	Auto-channel selection SCAN timeout state machine procedure
    ==========================================================================
 */
VOID AutoChSelRestoreTimeoutAction(
	IN RTMP_ADAPTER * pAd,
	IN MLME_QUEUE_ELEM * pElem)
{
	CHAR Idx;
	AUTO_CH_CTRL *pAutoChCtrl;
	struct wifi_dev *bss_wdev;
	struct wifi_dev *pwdev = pElem->wdev;
	UCHAR bss_idx = 0;

	if (!pwdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"AutoChSelScanTimeoutAction - pwdev == NULL\n");
		return;
	}

	pAutoChCtrl = HcGetAutoChCtrl(pAd);
	Idx = pAutoChCtrl->AutoChSelCtrl.ScanChIdx;
	pAd->MsMibBucket.Enabled = FALSE;
	pAd->OneSecMibBucket.Enabled = FALSE;

	for (bss_idx = 0; bss_idx < pAd->ApCfg.BssidNum; bss_idx++) {
		bss_wdev = &pAd->ApCfg.MBSSID[bss_idx].wdev;

		if (bss_wdev == NULL)
			continue;
		if (bss_wdev->bAllowBeaconing)
			UpdateBeaconHandler(pAd, bss_wdev, BCN_REASON(BCN_UPDATE_DISABLE_TX));
	}

	pAutoChCtrl->AutoChSelCtrl.RestoreFlag = FALSE;
	AutoChSelScanNextChannel(pAd, pwdev);
}

/*
    ==========================================================================
    Description:
	Auto-channel selection SCAN timeout state machine procedure
    ==========================================================================
 */
VOID AutoChSelScanTimeoutAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM * pElem)
{
	CHAR Idx;
	AUTO_CH_CTRL *pAutoChCtrl;
	struct wifi_dev *pwdev = pElem->wdev;
	if (!pwdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "AutoChSelScanTimeoutAction - pwdev == NULL \n");
		return;
	}

	pAutoChCtrl = HcGetAutoChCtrl(pAd);
	Idx = pAutoChCtrl->AutoChSelCtrl.ScanChIdx;

	UpdateChannelInfo(pAd, Idx, ChannelAlgBusyTime, pwdev);
	pAutoChCtrl->AutoChSelCtrl.ScanChIdx = AutoChSelFindScanChIdx(pAd, pwdev, Idx);
	AutoChSelScanNextChannel(pAd, pwdev);
}


VOID AutoChSelScanStartSkipBuildChList(
	IN struct _RTMP_ADAPTER	*pAd,
	IN struct wifi_dev *pwdev)
{
	UCHAR bss_idx = 0;
	struct wifi_dev *bss_wdev;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	/* Reset ACS ChCtrl*/
	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_NONE;

	/* Disable MibBucket during doing ACS */
	pAd->MsMibBucket.Enabled = FALSE;
	pAd->OneSecMibBucket.Enabled = FALSE;
	/* Init false for Partial Scan ACS*/
	pAutoChCtrl->AutoChSelCtrl.RestoreFlag = FALSE;
	/*Init count of scanning contiouosly for Partial Scan ACS*/
	pAutoChCtrl->AutoChSelCtrl.CountofCh = 0;
	pAutoChCtrl->AutoChSelCtrl.ScanChIdx = 0; /* Start from first channel */
	pAutoChCtrl->AutoChSelCtrl.pScanReqwdev = pwdev;

#ifdef SCAN_SUPPORT
	if (scan_in_run_state(pAd, pwdev)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
			"Failed!!!Scan is running, please try again after scan done!\n");
		return;
	}
#endif

	/* Disable beacon tx for all BSS during channel scan */
	for (bss_idx = 0; bss_idx < pAd->ApCfg.BssidNum; bss_idx++) {
		bss_wdev = &pAd->ApCfg.MBSSID[bss_idx].wdev;

		if (bss_wdev == NULL)
			continue;
		if (bss_wdev->bAllowBeaconing)
			UpdateBeaconHandler(pAd, bss_wdev, BCN_REASON(BCN_UPDATE_DISABLE_TX));
	}

	MlmeEnqueueWithWdev(pAd, AUTO_CH_SEL_STATE_MACHINE,  AUTO_CH_SEL_SCAN_REQ,
		0, NULL, pwdev->func_idx, pwdev, FALSE, NULL);
	RTMP_MLME_HANDLER(pAd);
}

/*
    ==========================================================================
    Description:
	Scan start handler, executed in timer thread
    ==========================================================================
 */
VOID AutoChSelScanStart(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev)
{
	UCHAR bss_idx = 0, ChIdx;
	struct wifi_dev *bss_wdev;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	UCHAR ch, prior;

	/* Reset ACS ChCtrl*/
	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_NONE;
	/* Disable MibBucket during doing ACS */
	pAd->MsMibBucket.Enabled = FALSE;
	pAd->OneSecMibBucket.Enabled = FALSE;
	/* Init false for Partial Scan ACS*/
	pAutoChCtrl->AutoChSelCtrl.RestoreFlag = FALSE;
	/*Init count of scanning contiouosly for Partial Scan ACS*/
	pAutoChCtrl->AutoChSelCtrl.CountofCh = 0;
	pAutoChCtrl->AutoChSelCtrl.ScanChIdx = 0; /* Start from first channel */
	pAutoChCtrl->AutoChSelCtrl.pScanReqwdev = pwdev;
	AutoChSelBuildChannelList(pAd, pwdev);

	/* FOR DEBUG USE ONLY */
	for (ChIdx = 0; ChIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChIdx++) {
		ch = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChIdx].Channel;
		prior = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChIdx].Priority;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"ch(%d):prio(%d) ", ch, prior);
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
			"IsABand = %d, ChannelListNum = %d\n", pAutoChCtrl->AutoChSelCtrl.IsABand, pAutoChCtrl->AutoChSelCtrl.ChListNum);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
			"PartialScanACS = %d", pAutoChCtrl->AutoChSelCtrl.PartialScanACS);
#ifdef SCAN_SUPPORT
	if (scan_in_run_state(pAd, pwdev)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
			"Failed!!!Scan is running, please try again after scan done!\n");
		return;
	}
#endif

	/* Disable beacon tx for all BSS during channel scan */
	for (bss_idx = 0; bss_idx < pAd->ApCfg.BssidNum; bss_idx++) {
		bss_wdev = &pAd->ApCfg.MBSSID[bss_idx].wdev;

		if (bss_wdev == NULL)
			continue;
		if (bss_wdev->bAllowBeaconing)
			UpdateBeaconHandler(pAd, bss_wdev, BCN_REASON(BCN_UPDATE_DISABLE_TX));
	}

	MlmeEnqueueWithWdev(pAd, AUTO_CH_SEL_STATE_MACHINE,  AUTO_CH_SEL_SCAN_REQ,
		0, NULL, pwdev->func_idx, pwdev, FALSE, NULL);
	RTMP_MLME_HANDLER(pAd);
}

/*
    ==========================================================================
    Description:
	Scan timeout handler, executed in timer thread
    ==========================================================================
 */
VOID AutoChSelScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PTIMER_FUNC_CONTEXT pContext = (PTIMER_FUNC_CONTEXT)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pContext->pAd;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	struct wifi_dev *pwdev = pAutoChCtrl->AutoChSelCtrl.pScanReqwdev;

	MlmeEnqueueWithWdev(pAd, AUTO_CH_SEL_STATE_MACHINE,  AUTO_CH_SEL_SCAN_TIMEOUT,
		0, NULL, 0, pwdev, FALSE, NULL);
	RTMP_MLME_HANDLER(pAd);
}

/*
    ==========================================================================
    Description:
	Restore timeout handler, executed in timer thread
    ==========================================================================
 */
VOID AutoChSelRestoreTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PTIMER_FUNC_CONTEXT pContext = (PTIMER_FUNC_CONTEXT)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pContext->pAd;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	struct wifi_dev *pwdev = pAutoChCtrl->AutoChSelCtrl.pScanReqwdev;

	MlmeEnqueueWithWdev(pAd, AUTO_CH_SEL_STATE_MACHINE,  AUTO_CH_SEL_SCAN_RESTORE,
		0, NULL, 0, pwdev, FALSE, NULL);
	RTMP_MLME_HANDLER(pAd);
}

/*
   ==========================================================================
   Description:
	Auto-channel selection state machine.
   Parameters:
		Sm - pointer to the state machine
   NOTE:
	The state machine is classified as follows:
	a. AUTO_CH_SEL_SCAN_IDLE
	b. AUTO_CH_SEL_SCAN_LISTEN
   ==========================================================================
 */
VOID AutoChSelStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR BandIdx,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{

	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	pAutoChCtrl->AutoChSelCtrl.ACSTimerFuncContex.pAd = pAd;
	pAutoChCtrl->AutoChSelCtrl.ACSTimerFuncContex.BandIdx = BandIdx;

	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, AUTO_CH_SEL_SCAN_MAX_STATE, AUTO_CH_SEL_SCAN_MAX_MSG,
					 (STATE_MACHINE_FUNC)Drop, AUTO_CH_SEL_SCAN_IDLE, AUTO_CH_SEL_MACHINE_BASE);
	/* Scan idle state */
	StateMachineSetAction(Sm, AUTO_CH_SEL_SCAN_IDLE, AUTO_CH_SEL_SCAN_REQ, (STATE_MACHINE_FUNC)AutoChSelScanReqAction);
	/* Scan listen state */
	StateMachineSetAction(Sm, AUTO_CH_SEL_SCAN_LISTEN, AUTO_CH_SEL_SCAN_TIMEOUT, (STATE_MACHINE_FUNC)AutoChSelScanTimeoutAction);
	StateMachineSetAction(Sm, AUTO_CH_SEL_SCAN_LISTEN, AUTO_CH_SEL_SCAN_RESTORE, (STATE_MACHINE_FUNC)AutoChSelRestoreTimeoutAction);
	RTMPInitTimer(pAd, &pAutoChCtrl->AutoChSelCtrl.AutoChScanTimer, GET_TIMER_FUNCTION(AutoChSelScanTimeout), &pAutoChCtrl->AutoChSelCtrl.ACSTimerFuncContex, FALSE);
	RTMPInitTimer(pAd, &pAutoChCtrl->AutoChSelCtrl.AutoChResoreTimer, GET_TIMER_FUNCTION(AutoChSelRestoreTimeout), &pAutoChCtrl->AutoChSelCtrl.ACSTimerFuncContex, FALSE);
}

/*
   ==========================================================================
   Description:
	Init for auto-channel selection scan-timer.
   NOTE:
   ==========================================================================
 */
VOID AutoChSelInit(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR BandIdx = 0;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);

	AutoChSelStateMachineInit(pAd, BandIdx, &pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine, pAutoChCtrl->AutoChSelCtrl.AutoChScanFunc);
}

/*
   ==========================================================================
   Description:
	Release auto-channel selection scan-timer.
   NOTE:
   ==========================================================================
 */

VOID AutoChSelRelease(
	IN PRTMP_ADAPTER pAd)
{
	BOOLEAN Cancelled;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;

	pAutoChCtrl = HcGetAutoChCtrl(pAd);
	if (pAutoChCtrl)
		RTMPReleaseTimer(&pAutoChCtrl->AutoChSelCtrl.AutoChScanTimer, &Cancelled);
	else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "pAutoChCtrl is NULL!\n");
}
/*
   ==========================================================================
   Description:
	Restore	channel in partial scan ACS.
   NOTE:
   ==========================================================================
 */

INT ACS_Restore_During_Partial_Scan(RTMP_ADAPTER *pAd, struct wifi_dev *pwdev)
{
	UCHAR ch, apidx;
	INT BssIdx;
	INT MaxNumBss = pAd->ApCfg.BssidNum;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *apWdev = NULL;
	RALINK_TIMER_STRUCT *RestoreTimer;
	struct DOT11_H *pDot11h = NULL;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	UINT wait_time = pAutoChCtrl->AutoChSelCtrl.RestoreDwell;

	if (pwdev) {
		apidx = pwdev->func_idx;
		pMbss = &pAd->ApCfg.MBSSID[apidx];
	} else
		return FALSE;
	RestoreTimer = &pAutoChCtrl->AutoChSelCtrl.AutoChResoreTimer;
	ch = wlan_operate_get_prim_ch(pwdev);
	/*restore to original channel*/
	if (ch != 0)
		wlan_operate_set_prim_ch(pwdev, ch);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
		"Partial Scan ACS restore to BW(%d) channel %d\n", wlan_operate_get_bw(pwdev), ch);

	pDot11h = pwdev->pDot11_H;

	if (((wlan_config_get_ch_band(pwdev) == CMD_CH_BAND_5G) &&
		 (pAd->CommonCfg.bIEEE80211H == TRUE) &&
		 RadarChannelCheck(pAd, pwdev->channel)) &&
		(pDot11h && pDot11h->ChannelMode != CHAN_SWITCHING_MODE)) {
		if (pDot11h->InServiceMonitorCount) {
			pDot11h->ChannelMode = CHAN_NORMAL_MODE;
			/* Enable beacon tx for all BSS */
			for (BssIdx = 0; BssIdx < MaxNumBss; BssIdx++) {
				apWdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
				if (!apWdev || apWdev->channel != pwdev->channel)
					continue;

				if (apWdev->bAllowBeaconing)
					UpdateBeaconHandler(pAd, apWdev, BCN_REASON(BCN_UPDATE_ENABLE_TX));
			}
		} else
			pDot11h->ChannelMode = CHAN_SILENCE_MODE;
	} else {
		/* Enable beacon tx for all BSS */
		for (BssIdx = 0; BssIdx < MaxNumBss; BssIdx++) {
			apWdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
			if (!apWdev || apWdev->channel != pwdev->channel)
				continue;

			if (apWdev->bAllowBeaconing)
				UpdateBeaconHandler(pAd, apWdev, BCN_REASON(BCN_UPDATE_ENABLE_TX));
		}
	}
	RTMPSetTimer(RestoreTimer, wait_time);

	return TRUE;
}

/*
   ==========================================================================
   Description:
       Set auto channel select parameters by reading profile settings.

    Return:
	None.
   ==========================================================================
*/
VOID auto_ch_select_set_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *buffer)
{
	ChannelSel_Alg sel_alg;
	BOOLEAN acs_bootup = FALSE;
	long value = 0;

	if (!buffer) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");
		return;
	}
	if (buffer && kstrtol(buffer, 10, &value)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input(=%s)\n",
			buffer);
		return;
	}
	sel_alg = (ChannelSel_Alg)value;
	if ((ChannelAlgRandom < sel_alg) && (sel_alg <= ChannelAlgBusyTime))
		acs_bootup = TRUE;
	else if (ChannelAlgRandom == sel_alg)
		acs_bootup = FALSE;
	else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Invalid argument(=%d)!\n",
			sel_alg);
		return;
	}

	pAd->ApCfg.AutoChannelAlg = sel_alg;
	pAd->ApCfg.bAutoChannelAtBootup = acs_bootup;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
		"AutoChannelAtBootup=%d, AutoChannelAlg = %d\n",
		pAd->ApCfg.bAutoChannelAtBootup, pAd->ApCfg.AutoChannelAlg);
}

VOID acs_set_scan_mode(RTMP_ADAPTER *pAd, RTMP_STRING *buffer)
{
	AUTO_CH_CTRL *pAutoChCtrl = NULL;
	int status = 0;
	long value = 0;

	if (!buffer) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");
		return;
	}

	pAutoChCtrl = HcGetAutoChCtrl(pAd);

	status = kstrtol(buffer, 10, &value);
	if (status)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");
	else {
		if (value > 0)
			pAutoChCtrl->AutoChSelCtrl.PartialScanACS = TRUE;
		else
			pAutoChCtrl->AutoChSelCtrl.PartialScanACS = FALSE;

		MTWF_PRINT("\x1b[42m: PartialScanACS=%d \x1b[m\n",
			pAutoChCtrl->AutoChSelCtrl.PartialScanACS);
	}
}

VOID acs_set_scan_dwell(RTMP_ADAPTER *pAd, RTMP_STRING *buffer)
{
	AUTO_CH_CTRL *pAutoChCtrl = NULL;
	int status = 0;
	long value = 0;

	if (!buffer) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");
		return;
	}

	pAutoChCtrl = HcGetAutoChCtrl(pAd);

	status = kstrtol(buffer, 10, &value);
	if (status)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");
	else {
		if (value > 0)
			pAutoChCtrl->AutoChSelCtrl.ScanningDwell = (USHORT)value;
		else
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");

		MTWF_PRINT("\x1b[42m: ScanningDwell=%d \x1b[m\n",
			pAutoChCtrl->AutoChSelCtrl.ScanningDwell);
	}
}

VOID acs_set_restore_dwell(RTMP_ADAPTER *pAd, RTMP_STRING *buffer)
{
	AUTO_CH_CTRL *pAutoChCtrl = NULL;
	int status = 0;
	long value = 0;

	if (!buffer) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");
		return;
	}

	pAutoChCtrl = HcGetAutoChCtrl(pAd);

	status = kstrtol(buffer, 10, &value);
	if (status)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");
	else {
		if (value > 0)
			pAutoChCtrl->AutoChSelCtrl.RestoreDwell = (USHORT)value;
		else
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");

		MTWF_PRINT("\x1b[42m: RestoreDwell=%d \x1b[m\n",
			pAutoChCtrl->AutoChSelCtrl.RestoreDwell);
	}
}

VOID acs_set_partial_scan_ch_num(RTMP_ADAPTER *pAd, RTMP_STRING *buffer)
{
	AUTO_CH_CTRL *pAutoChCtrl = NULL;
	int status = 0;
	long value = 0;

	if (!buffer) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");
		return;
	}

	pAutoChCtrl = HcGetAutoChCtrl(pAd);

	status = kstrtol(buffer, 10, &value);
	if (status)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");
	else {
		if (value > 0)
			pAutoChCtrl->AutoChSelCtrl.ScanningChNum = (UCHAR)value;
		else
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");

		MTWF_PRINT("\x1b[42m: ScanningChNum=%d \x1b[m\n", pAutoChCtrl->AutoChSelCtrl.ScanningChNum);
	}
}

VOID acs_set_ch_util_thr(RTMP_ADAPTER *pAd, RTMP_STRING *buffer)
{
	int status = 0;
	long value = 0;

	if (!buffer) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");
		return;
	}

	status = kstrtol(buffer, 10, &value);
	if (status)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");
	else {
		if (value <= 255)
			pAd->ApCfg.AcsCfg.ChUtilThr = (UCHAR)value;
		else
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");

		MTWF_PRINT("\x1b[42m: ChUtilThr=%d \x1b[m\n", pAd->ApCfg.AcsCfg.ChUtilThr);
	}
}

VOID acs_set_sta_num_thr(RTMP_ADAPTER *pAd, RTMP_STRING *buffer)
{
	int status = 0;
	long value = 0;

	if (!buffer) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");
		return;
	}

	status = kstrtol(buffer, 10, &value);
	if (status)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");
	else {
		pAd->ApCfg.AcsCfg.StaNumThr = (USHORT)value;
		MTWF_PRINT("\x1b[42m: StaNumThr=%d \x1b[m\n", pAd->ApCfg.AcsCfg.StaNumThr);
	}
}

VOID acs_config_init(RTMP_ADAPTER *pAd)
{
	AUTO_CH_CTRL *pAutoChCtrl = NULL;

	pAd->ApCfg.AcsCfg.ACSCheckTime = PERIODIC_ACS_CH_CHK_TIME;
	pAd->ApCfg.AcsCfg.ChUtilThr = PERIODIC_ACS_CH_UTIL_RATIO_THR;
	pAd->ApCfg.AcsCfg.StaNumThr = PERIODIC_ACS_CH_STA_NUM_THR;

	pAutoChCtrl = HcGetAutoChCtrl(pAd);
	pAutoChCtrl->AutoChSelCtrl.ACSCheckCount = 0;
	pAutoChCtrl->AutoChSelCtrl.AutoTriggerCount = 0;
}

#ifdef CONFIG_6G_SUPPORT
/*
   ==========================================================================
   Description:
       Set 6G PSC auto channel select parameters by reading profile settings.

    Return:
	None.
   ==========================================================================
*/
VOID auto_ch_select_PSC_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *buffer)
{
	UINT8 PSC_ACS = 0;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;

	if (!buffer) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Error input\n");
		return;
	}
	pAutoChCtrl = HcGetAutoChCtrl(pAd);
	PSC_ACS = simple_strtol(buffer, 0, 10);
	if (PSC_ACS > 0)
		pAutoChCtrl->AutoChSelCtrl.PSC_ACS = TRUE;
	else
		pAutoChCtrl->AutoChSelCtrl.PSC_ACS = FALSE;
	MTWF_PRINT("\x1b[42m: PSC_ACS=%d \x1b[m\n", pAutoChCtrl->AutoChSelCtrl.PSC_ACS);
}
#endif

/* return: TRUE->over the threshold, FALSE->under the threshold	*/
static BOOLEAN ch_util_over_thr_chk(RTMP_ADAPTER *pAd)
{
	UINT32 ChTimeByOther;
	UCHAR ChUtilRatio;

	ChTimeByOther = pAd->OneSecMibBucket.OBSSAirtime + pAd->OneSecMibBucket.EDCCAtime;
	ChUtilRatio = ChTimeByOther / 10000;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
		"Current channel util_ratio: %d.\n", ChUtilRatio);

	if (ChUtilRatio >= pAd->ApCfg.AcsCfg.ChUtilThr)
		return TRUE;

	return FALSE;
}

/* return: TRUE->over the threshold, FALSE->under the threshold	*/
static BOOLEAN sta_num_over_thr_chk(RTMP_ADAPTER *pAd)
{
	UINT16 i, StaNum = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (pAd->ApCfg.AcsCfg.StaNumThr > 0) {
		if (pAd->MacTab->Size != 0) {
			for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
				pEntry = entry_get(pAd, i);
				if ((pEntry->wdev != NULL) && (IS_ENTRY_CLIENT(pEntry))) {
					StaNum++;

					if (StaNum >= pAd->ApCfg.AcsCfg.StaNumThr) {
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
							"Ignore ACS checking, more than %d clients connected.\n",
							StaNum);

						return TRUE;
					}
				}
			}
		}
	} else
		return FALSE;

	return FALSE;
}

#ifdef AP_SCAN_SUPPORT
/*
   ==========================================================================
   Description:
       trigger Auto Channel Selection every period of ACSCheckTime.

   NOTE:
		This function is called in a 1-sec mlme periodic check.
		Do ACS only on one HW band at a time.
		Do ACS only when no clients is associated.
   ==========================================================================
 */
VOID AutoChannelSelCheck(RTMP_ADAPTER *pAd)
{
	UINT16 i;
	struct wifi_dev *pwdev = NULL;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;

	/* Do nothing if ACSCheckTime is not configured */
	if (pAd->ApCfg.AcsCfg.ACSCheckTime == 0)
		return;

	pAutoChCtrl = HcGetAutoChCtrl(pAd);
	pAutoChCtrl->AutoChSelCtrl.ACSCheckCount++;
	if (pAutoChCtrl->AutoChSelCtrl.ACSCheckCount > pAd->ApCfg.AcsCfg.ACSCheckTime
		&& pAd->ApCfg.AutoChannelAlg == ChannelAlgBusyTime) {
		/* Find an active wdev */
		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			pwdev = &pAd->ApCfg.MBSSID[i].wdev;
			if (WDEV_BSS_STATE(pwdev) >= BSS_ACTIVE)
				break;
		}

		if (!pwdev) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"No any active bss, ignore ACS scan.\n");
			return;
		}

		/* Do nothing if AP is doing channel scanning */
		if (scan_in_run_state(pAd, pwdev)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"Scan is running, ignore ACS scan.\n");
			return;
		}

		/* Need to enable the mibBucket to obtain the busytime */
		if (!pAd->OneSecMibBucket.Enabled) {
			pAd->OneSecMibBucket.Enabled = TRUE;
			return;
		}

		/* Reset Counter */
		pAutoChCtrl->AutoChSelCtrl.ACSCheckCount = 0;

		/* Do Auto Channel Selection only when the threshold is over  */
		if (sta_num_over_thr_chk(pAd) || !ch_util_over_thr_chk(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
				"Under the threshold, ignore ACS scan.\n");
			return;
		}

		/* Start for ACS checking */
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
				"Scanning channels for channel selection.\n");

		pAutoChCtrl->AutoChSelCtrl.AutoTriggerCount++;

		if (pAd->ApCfg.AutoChannelAlg == ChannelAlgBusyTime) {
#ifdef TR181_SUPPORT
			{
				struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

				/*set ACS trigger flag to periodic refresh trigger*/
				if (ctrl)
					ctrl->rdev.pRadioCtrl->ACSTriggerFlag = 1;
			}
#endif /*TR181_SUPPORT*/
			AutoChSelScanStart(pAd, pwdev);
		} else
			ApSiteSurvey_by_wdev(pAd, NULL, SCAN_PASSIVE, TRUE, pwdev);

		return;
	}
}

INT show_acs_debug_proc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *arg)
{
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);

	MTWF_PRINT("ACS config:\n");
	MTWF_PRINT("-----------\n");

	MTWF_PRINT("AutoChannelAlg=%d\n", pAd->ApCfg.AutoChannelAlg);
	MTWF_PRINT("auto_ch_score_flag=%d\n", pAd->ApCfg.auto_ch_score_flag);
	MTWF_PRINT("ACSCheckTime=%d\n", pAd->ApCfg.AcsCfg.ACSCheckTime);
	MTWF_PRINT("ChUtilThr=%d\n", pAd->ApCfg.AcsCfg.ChUtilThr);
	MTWF_PRINT("StaNumThr=%d\n", pAd->ApCfg.AcsCfg.StaNumThr);
	MTWF_PRINT("PartialScanACS=%d\n", pAutoChCtrl->AutoChSelCtrl.PartialScanACS);
	MTWF_PRINT("ScanningDwell=%d\n", pAutoChCtrl->AutoChSelCtrl.ScanningDwell);
	MTWF_PRINT("RestoreDwell=%d\n", pAutoChCtrl->AutoChSelCtrl.RestoreDwell);
	MTWF_PRINT("ScanningChNum=%d\n", pAutoChCtrl->AutoChSelCtrl.ScanningChNum);
#ifdef CONFIG_6G_SUPPORT
	MTWF_PRINT("PSC_ACS=%d\n", pAutoChCtrl->AutoChSelCtrl.PSC_ACS);
#endif

	MTWF_PRINT("\nDaynamic info:\n");
	MTWF_PRINT("-----------\n");
	MTWF_PRINT("AutoTriggerCount=%d\n", pAutoChCtrl->AutoChSelCtrl.AutoTriggerCount);
	MTWF_PRINT("ACSCheckCount=%d\n", pAutoChCtrl->AutoChSelCtrl.ACSCheckCount);
	MTWF_PRINT("ChListNum=%d\n", pAutoChCtrl->AutoChSelCtrl.ChListNum);
	MTWF_PRINT("CountofCh=%d\n", pAutoChCtrl->AutoChSelCtrl.CountofCh);
	MTWF_PRINT("RestoreFlag=%d\n", pAutoChCtrl->AutoChSelCtrl.RestoreFlag);
	MTWF_PRINT("MaxChPriority=%d\n", pAutoChCtrl->AutoChSelCtrl.MaxChPriority);

	{
		UINT32 ChTimeByOther;
		UCHAR ChUtilRatio;

		ChTimeByOther = pAd->OneSecMibBucket.OBSSAirtime + pAd->OneSecMibBucket.EDCCAtime;
		ChUtilRatio = ChTimeByOther / 10000;
		MTWF_PRINT("Time: OBSS=%d, NonWifi=%d, Busy=%d, MyTx=%d, MRx=%d, ChUtilRatio=%d\n",
			pAd->OneSecMibBucket.OBSSAirtime,
			pAd->OneSecMibBucket.EDCCAtime,
			pAd->OneSecMibBucket.ChannelBusyTime,
			pAd->OneSecMibBucket.MyTxAirtime,
			pAd->OneSecMibBucket.MyRxAirtime,
			ChUtilRatio);
	}

	return TRUE;
}
#endif /* AP_SCAN_SUPPORT */

VOID auto_ch_select_reset_sm(RTMP_ADAPTER *pAd, struct wifi_dev *pwdev)
{
	AUTO_CH_CTRL *pAutoChCtrl = NULL;

	if (pwdev) {
		pAutoChCtrl = HcGetAutoChCtrl(pAd);
		if (pAutoChCtrl)
			pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState = AUTO_CH_SEL_SCAN_IDLE;
	}
}

NDIS_STATUS set_idle_pwr_test(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	INT32 recv = 0;
	INT32 pwr_thres = 0;
	UINT32 time = 0, cnt = 0, cmd_type = 0;
	EXT_CMD_RDD_IPI_HIST_T cmd_rdd_ipi_hist;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (arg) {
		recv = sscanf(arg, "%d:%d:%d:%d", &(pwr_thres), &(time), &(cnt), &(cmd_type));

		if (recv != 4) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
				"Format Error! Please enter in the following format\n"
					"pwr_thres:time(ms):cnt:offset_level\n");
			return TRUE;
		}

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			 "pwr_thres %d, time %d, cnt = %d, cmd_type = %d, band_idx = %d\n",
			 pwr_thres, time, cnt, cmd_type, u1DbdcIdx);

		os_zero_mem(&cmd_rdd_ipi_hist, sizeof(EXT_CMD_RDD_IPI_HIST_T));
		cmd_rdd_ipi_hist.ipi_hist_idx = RDD_SET_IDLE_PWR;
		cmd_rdd_ipi_hist.idle_pwr_thres = pwr_thres;
		cmd_rdd_ipi_hist.idle_pwr_max_cnt = cnt;
		cmd_rdd_ipi_hist.idle_pwr_duration = time;
		cmd_rdd_ipi_hist.idle_pwr_cmd_type = cmd_type;
		cmd_rdd_ipi_hist.band_idx = u1DbdcIdx;

		mt_cmd_set_rdd_ipi_hist(pAd, &cmd_rdd_ipi_hist);
	} else {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "Arg is Null\n");
		status = NDIS_STATUS_FAILURE;

	}

	return status;
}

