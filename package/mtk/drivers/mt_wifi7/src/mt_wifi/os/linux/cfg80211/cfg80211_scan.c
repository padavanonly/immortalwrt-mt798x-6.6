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
 ***************************************************************************/

/****************************************************************************
 *
 *	Abstract:
 *
 *	All related CFG80211 Scan function body.
 *
 *	History:
 *
 ***************************************************************************/

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"

#ifdef CONFIG_STA_SUPPORT
VOID CFG80211DRV_OpsScanInLinkDownAction(
	VOID						*pAdOrg)
{
#ifndef APCLI_CFG80211_SUPPORT
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	BOOLEAN Cancelled;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	SCAN_ACTION_INFO scan_action_info = {0};

	pStaCfg = GetStaCfgByWdev(pAd, &(pAd->StaCfg[0].wdev));
	RTMPCancelTimer(&pStaCfg->MlmeAux.JoinTimer, &Cancelled);
	pAd->ScanCtrl.Channel = 0;
	CFG80211OS_ScanEnd(pAd, TRUE);
	scan_next_channel(pAd, OPMODE_STA, &(scan_action_info));
#endif /* CONFIG_STA_SUPPORT */
#endif
}

BOOLEAN CFG80211DRV_OpsScanRunning(
	VOID						*pAdOrg)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;

	return (pAd->cfg80211_ctrl.FlgCfg80211Scanning == TRUE);
}
#endif /*CONFIG_STA_SUPPORT*/

/* Refine on 2013/04/30 for two functin into one */
INT CFG80211DRV_OpsScanGetNextChannel(
	VOID						*pAdOrg)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;

	if (cfg80211_ctrl->pCfg80211ChanList != NULL) {
		if (cfg80211_ctrl->Cfg80211CurChanIndex < cfg80211_ctrl->Cfg80211ChanListLen)
			return cfg80211_ctrl->pCfg80211ChanList[cfg80211_ctrl->Cfg80211CurChanIndex++];

		os_free_mem(cfg80211_ctrl->pCfg80211ChanList);
		cfg80211_ctrl->pCfg80211ChanList = NULL;
		cfg80211_ctrl->Cfg80211ChanListLen = 0;
		cfg80211_ctrl->Cfg80211CurChanIndex = 0;
		return 0;
	}

	return 0;
}

BOOLEAN CFG80211DRV_OpsScanSetSpecifyChannel(
	VOID						*pAdOrg,
	VOID						*pData,
	UINT8						 dataLen)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;
	UINT32 *pChanList = (UINT32 *) pData;

	if (pChanList != NULL) {
		if (cfg80211_ctrl->pCfg80211ChanList != NULL)
			os_free_mem(cfg80211_ctrl->pCfg80211ChanList);

		os_alloc_mem(NULL, (UCHAR **)&cfg80211_ctrl->pCfg80211ChanList, sizeof(UINT32) * dataLen);

		if (cfg80211_ctrl->pCfg80211ChanList != NULL) {
			NdisCopyMemory(cfg80211_ctrl->pCfg80211ChanList, pChanList, sizeof(UINT32) * dataLen);
			cfg80211_ctrl->Cfg80211ChanListLen = dataLen;
			cfg80211_ctrl->Cfg80211CurChanIndex = 0; /* Start from index 0 */
			return NDIS_STATUS_SUCCESS;
		} else
			return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_FAILURE;
}

BOOLEAN CFG80211DRV_OpsScanCheckStatus(
	VOID						*pAdOrg,
	UINT8						 IfType)
{
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;

	pStaCfg = GetStaCfgByWdev(pAd, &(pAd->StaCfg[0].wdev));

	/* CFG_TODO */
	if (CFG80211DRV_OpsScanRunning(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"SCAN_FAIL: CFG80211 Internal SCAN Flag On\n");
		return FALSE;
	}

	/* To avoid the scan cmd come-in during driver init */
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"SCAN_FAIL: Scan cmd before Startup finish\n");
		return FALSE;
	}

	if ((STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) &&
		(IS_AKM_WPA_CAPABILITY(pStaCfg->wdev.SecConfig.AKMMap)) &&
		(pAd->StaCfg[0].wdev.PortSecured == WPA_802_1X_PORT_NOT_SECURED)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"SCAN_FAIL: Link UP, Port Not Secured! ignore this scan request\n");
		return FALSE;
	}

	if (pAd->StaCfg[0].wdev.cntl_machine.CurrState != CNTL_IDLE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Cntl sta Machine not idle\n");
		return FALSE;
	}

#ifdef RT_CFG80211_SUPPORT

	if (pAd->cfg80211_ctrl.FlgCfg8021Disable2040Scan == TRUE &&
		(IfType == RT_CMD_80211_IFTYPE_AP)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"Disable 20/40 scan!!\n");
		return FALSE;
	}

#endif /* RT_CFG80211_SUPPORT */
	/* do scan */
	pAd->cfg80211_ctrl.FlgCfg80211Scanning = TRUE;
#endif /*CONFIG_STA_SUPPORT*/
	return TRUE;
}
void CFG80211DRV_ApSiteSurvey(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	RT_CMD_STA_IOCTL_SCAN *pScan = (RT_CMD_STA_IOCTL_SCAN *)pData;
	NDIS_802_11_SSID	Ssid;
	UCHAR ScanType = SCAN_ACTIVE;
	CFG80211_CB *pCfg80211_CB;

	pCfg80211_CB = pAd->pCfg80211_CB;

	if (pScan->ScanType != 0)
		ScanType = pScan->ScanType;
	else
		ScanType = SCAN_ACTIVE;

	Ssid.SsidLength = pScan->SsidLen;
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"req.essid_len-%d, essid-%s\n", pScan->SsidLen, pScan->pSsid);
	NdisZeroMemory(&Ssid.Ssid, NDIS_802_11_LENGTH_SSID);
	if (pScan->SsidLen)
		NdisMoveMemory(Ssid.Ssid, pScan->pSsid, Ssid.SsidLength);

	ApSiteSurvey(pAd, &Ssid, pScan->duration, ScanType, &pAd->ApCfg.MBSSID[pScan->ApIndex].wdev);
}


#ifdef APCLI_CFG80211_SUPPORT
void CFG80211DRV_ApcliSiteSurvey(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	RT_CMD_STA_IOCTL_SCAN *pScan = (RT_CMD_STA_IOCTL_SCAN *)pData;
	NDIS_802_11_SSID	Ssid;
	UCHAR ScanType = SCAN_ACTIVE;
	CFG80211_CB *pCfg80211_CB;

	pCfg80211_CB = pAd->pCfg80211_CB;

	if (pScan->ScanType != 0)
		ScanType = pScan->ScanType;
	else
		ScanType = SCAN_ACTIVE;

	Ssid.SsidLength = pScan->SsidLen;
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"CFG80211DRV_ApcliSiteSurvey:: req.essid_len-%d, essid-%s\n", pScan->SsidLen, pScan->pSsid);
	NdisZeroMemory(&Ssid.Ssid, NDIS_802_11_LENGTH_SSID);
	if (pScan->SsidLen)
		NdisMoveMemory(Ssid.Ssid, pScan->pSsid, Ssid.SsidLength);
	StaSiteSurvey(pAd, &Ssid, ScanType, &pAd->StaCfg[pScan->StaIndex].wdev);
}
#endif /* APCLI_CFG80211_SUPPORT */

BOOLEAN CFG80211DRV_OpsScanExtraIesSet(
	VOID						*pAdOrg)
{
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CFG80211_CB *pCfg80211_CB = pAd->pCfg80211_CB;
	UINT ie_len = 0;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;

	if (pCfg80211_CB->pCfg80211_ScanReq)
		ie_len = pCfg80211_CB->pCfg80211_ScanReq->ie_len;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_DEBUG,
		"80211> ==> %d\n", ie_len);

	if (ie_len == 0)
		return FALSE;

	/* Reset the ExtraIe and Len */
	if (cfg80211_ctrl->pExtraIe) {
		os_free_mem(cfg80211_ctrl->pExtraIe);
		cfg80211_ctrl->pExtraIe = NULL;
	}

	cfg80211_ctrl->ExtraIeLen = 0;
	os_alloc_mem(pAd, (UCHAR **) &(cfg80211_ctrl->pExtraIe), ie_len);

	if (cfg80211_ctrl->pExtraIe) {
		NdisCopyMemory(cfg80211_ctrl->pExtraIe, pCfg80211_CB->pCfg80211_ScanReq->ie, ie_len);
		cfg80211_ctrl->ExtraIeLen = ie_len;
		hex_dump("CFG8021_SCAN_EXTRAIE", cfg80211_ctrl->pExtraIe, cfg80211_ctrl->ExtraIeLen);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"80211> CFG80211DRV_OpsExtraIesSet ==> allocate fail.\n");
		return FALSE;
	}

#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}

#ifdef CFG80211_SCAN_SIGNAL_AVG
static void CFG80211_CalBssAvgRssi(
	IN      BSS_ENTRY * pBssEntry)
{
	BOOLEAN bInitial = FALSE;

	if (!(pBssEntry->AvgRssiX8 | pBssEntry->AvgRssi))
		bInitial = TRUE;

	if (bInitial) {
		pBssEntry->AvgRssiX8 = pBssEntry->Rssi << 3;
		pBssEntry->AvgRssi  = pBssEntry->Rssi;
	} else {
		/* For smooth purpose, oldRssi for 7/8, newRssi for 1/8 */
		pBssEntry->AvgRssiX8 =
			(pBssEntry->AvgRssiX8 - pBssEntry->AvgRssi) + pBssEntry->Rssi;
	}

	pBssEntry->AvgRssi = pBssEntry->AvgRssiX8 >> 3;
}

static void CFG80211_UpdateBssTableRssi(
	IN VOID							*pAdCB)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;
	struct wiphy *pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	struct ieee80211_channel *chan;
	struct cfg80211_bss *bss;
	BSS_ENTRY *pBssEntry;
	UINT index;
	UINT32 CenFreq;
	BSS_TABLE *ScanTab = NULL;
	UCHAR BandIdx = 0;


	ScanTab = pAd->ScanCtrl.ScanTab;
	for (index = 0; index < ScanTab->BssNr; index++) {
		pBssEntry = &ScanTab->BssEntry[index];

		if (ScanTab->BssEntry[index].Channel > 14)
			CenFreq = ieee80211_channel_to_frequency(ScanTab->BssEntry[index].Channel, IEEE80211_BAND_5GHZ);
		else
			CenFreq = ieee80211_channel_to_frequency(ScanTab->BssEntry[index].Channel, IEEE80211_BAND_2GHZ);

		chan = ieee80211_get_channel(pWiphy, CenFreq);
		bss = cfg80211_get_bss(pWiphy, chan, pBssEntry->Bssid, pBssEntry->Ssid, pBssEntry->SsidLen,
					IEEE80211_BSS_TYPE_ESS, IEEE80211_PRIVACY_ANY);

		if (bss == NULL) {
			/* ScanTable Entry not exist in kernel buffer */
		} else {
			/* HIT */
			CFG80211_CalBssAvgRssi(pBssEntry);
			bss->signal = pBssEntry->AvgRssi * 100; /* UNIT: MdBm */
			CFG80211OS_PutBss(pWiphy, bss);
		}
	}
}
#endif /* CFG80211_SCAN_SIGNAL_AVG */

/*
 * ========================================================================
 * Routine Description:
 *	Inform us that a scan is got.
 *
 * Arguments:
 *	pAdCB				- WLAN control block pointer
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	Call RT_CFG80211_SCANNING_INFORM, not CFG80211_Scaning
 * ========================================================================
 */
VOID CFG80211_Scaning(
	IN VOID							*pAdCB,
	IN UINT32						BssIdx,
	IN UINT32						ChanId,
	IN UCHAR						*pFrame,
	IN UINT32						FrameLen,
	IN INT32						RSSI,
	IN UINT16						RawChannel)
{
#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	VOID *pCfg80211_CB = pAd->pCfg80211_CB;
	BOOLEAN FlgIsNMode;
	UINT8 BW;
#ifdef APCLI_CFG80211_SUPPORT
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
#else
	struct wifi_dev *wdev = get_default_wdev(pAd);
#endif
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"80211> Network is down!\n");
		return;
	}

	if (!pCfg80211_CB) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"80211> pCfg80211_CB is invalid!\n");
		return;
	}

	/*
	 *	In connect function, we also need to report BSS information to cfg80211;
	 *	Not only scan function.
	 */
	if ((!CFG80211DRV_OpsScanRunning(pAd)) &&
		(pAd->cfg80211_ctrl.FlgCfg80211Connecting == FALSE)) {
		return; /* no scan is running from wpa_supplicant */
	}

	/* init */
	/* Note: Can not use local variable to do pChan */
	if (WMODE_CAP_N(wdev->PhyMode))
		FlgIsNMode = TRUE;
	else
		FlgIsNMode = FALSE;

	if (cfg_ht_bw == BW_20)
		BW = 0;
	else
		BW = 1;

	CFG80211OS_Scaning(pCfg80211_CB,
					   ChanId,
					   pFrame,
					   FrameLen,
					   RSSI,
					   FlgIsNMode,
					   RawChannel,
					   BW);
#endif /* CONFIG_STA_SUPPORT */
}


/*
 * ========================================================================
 * Routine Description:
 *	Inform us that scan ends.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	FlgIsAborted	- 1: scan is aborted
 *
 * Return Value:
 *	NONE
 * ========================================================================
 */
VOID CFG80211_ScanEnd(
	IN VOID						*pAdCB,
	IN BOOLEAN					FlgIsAborted)
{
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"80211> Network is down!\n");
		return;
	}

	if (!CFG80211DRV_OpsScanRunning(pAd)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"80211> No scan is running!\n");
		return; /* no scan is running */
	}

	if (FlgIsAborted == TRUE)
		FlgIsAborted = 1;
	else {
		FlgIsAborted = 0;
#ifdef CFG80211_SCAN_SIGNAL_AVG
		CFG80211_UpdateBssTableRssi(pAd);
#endif /* CFG80211_SCAN_SIGNAL_AVG */
	}

	CFG80211OS_ScanEnd(pAd, FlgIsAborted);
#endif /* CONFIG_STA_SUPPORT */
}

VOID CFG80211_ScanStatusLockInit(
	IN VOID						*pAdCB,
	IN UINT                      init)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;

	if (init)
		NdisAllocateSpinLock(pAd, &pCfg80211_CB->scan_notify_lock);
	else
		NdisFreeSpinLock(&pCfg80211_CB->scan_notify_lock);
}

#endif /* RT_CFG80211_SUPPORT */

