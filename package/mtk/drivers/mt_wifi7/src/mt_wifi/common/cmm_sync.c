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
	cmm_sync.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	John Chang	2004-09-01      modified for rt2561/2661
*/
#include "rt_config.h"


/*BaSizeArray follows the 802.11n definition as MaxRxFactor.  2^(13+factor) bytes. When factor =0, it's about Ba buffer size =8.*/
UCHAR BaSizeArray[4] = {8, 16, 32, 64};

extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_2GHZ[];
extern UINT16 const Country_Region_GroupNum_2GHZ;
extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_5GHZ[];
extern UINT16 const Country_Region_GroupNum_5GHZ;
extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_6GHZ[];
extern UINT16 const Country_Region_GroupNum_6GHZ;

/*
	==========================================================================
	Description:
		Update StaCfg[0]->ChannelList[] according to 1) Country Region 2) RF IC type,
		and 3) PHY-mode user selected.
		The outcome is used by driver when doing site survey.

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static UCHAR BuildChannelListFor2G(RTMP_ADAPTER *pAd, CHANNEL_CTRL *pChCtrl, USHORT PhyMode)
{
	UCHAR ChIdx, ChIdx_TxPwr, num = 0;
	PCH_DESC pChDesc = NULL;
	BOOLEAN bRegionFound = FALSE;
	PUCHAR pChannelList;
	PUCHAR pChannelListFlag;
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "--->\n");

	for (ChIdx = 0; ChIdx < Country_Region_GroupNum_2GHZ; ChIdx++) {
		if ((pAd->CommonCfg.CountryRegion & 0x7f) ==
			Country_Region_ChDesc_2GHZ[ChIdx].RegionIndex) {
			pChDesc = Country_Region_ChDesc_2GHZ[ChIdx].pChDesc;
			num = TotalChNum(pChDesc);
			pAd->CommonCfg.pChDesc2G = (PUCHAR)pChDesc;
			bRegionFound = TRUE;
			break;
		}
	}

	if (!bRegionFound) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"CountryRegion=%d not support", pAd->CommonCfg.CountryRegion);
		goto done;
	}

	if (num > 0) {
		UCHAR ch_valid_idx = 0;
		os_alloc_mem(NULL, (UCHAR **)&pChannelList, num * sizeof(UCHAR));

		if (!pChannelList) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"Allocate memory for ChannelList failed\n");
			goto done;
		}

		os_alloc_mem(NULL, (UCHAR **)&pChannelListFlag, num * sizeof(UCHAR));

		if (!pChannelListFlag) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"Allocate memory for ChannelListFlag failed\n");
			os_free_mem(pChannelList);
			goto done;
		}

		/*do FreqList check*/
		for (ChIdx = 0; ChIdx < num; ChIdx++) {
			if (ChannelValidForFreqList(pAd, GetChannel_2GHZ(pChDesc, ChIdx))) {
				pChannelList[ch_valid_idx] = GetChannel_2GHZ(pChDesc, ChIdx);
				pChannelListFlag[ch_valid_idx] = GetChannelFlag(pChDesc, ChIdx);
				ch_valid_idx++;
			}
		}

		num = ch_valid_idx;

		for (ChIdx = 0; ChIdx < num; ChIdx++) {
			for (ChIdx_TxPwr = 0; ChIdx_TxPwr < MAX_NUM_OF_CHANNELS; ChIdx_TxPwr++) {
				if (pChannelList[ChIdx] == pAd->TxPower[ChIdx_TxPwr].Channel)
					hc_set_ChCtrl(pChCtrl, pAd, ChIdx, ChIdx_TxPwr);
			}
#ifdef CONFIG_AP_SUPPORT
			if (pChannelList[ChIdx] == 14) {
				if (!strncmp((RTMP_STRING *) pAd->CommonCfg.CountryCode, "JP", 2)) {
					/* for JP, ch14 can only be used when PhyMode is "B only" */
					if (!WMODE_EQUAL(PhyMode, WMODE_B)) {
						num--;
						break;
					}
				} else {
					/* Ch14 can only be used in Japan */
					num--;
					break;
				}
			}
#endif
			pChCtrl->ChList[ChIdx].Channel = pChannelList[ChIdx];
			if (!strncmp((RTMP_STRING *) pAd->CommonCfg.CountryCode, "CN", 2))
				pChCtrl->ChList[ChIdx].MaxTxPwr = pAd->MaxTxPwr;/*for CN CountryCode*/
			else
				pChCtrl->ChList[ChIdx].MaxTxPwr = 20;
			pChCtrl->ChList[ChIdx].Flags = pChannelListFlag[ChIdx];

#ifdef RT_CFG80211_SUPPORT
			CFG80211OS_ChanInfoInit(
				pAd->pCfg80211_CB,
				ChIdx,
				pChCtrl->ChList[ChIdx].Channel,
				pChCtrl->ChList[ChIdx].MaxTxPwr,
				TRUE,
				TRUE);
#endif /* RT_CFG80211_SUPPORT */
		}

		pChCtrl->ChListNum = num;

		os_free_mem(pChannelList);
		os_free_mem(pChannelListFlag);
	}

#ifdef RT_CFG80211_SUPPORT

	if (CFG80211OS_UpdateRegRuleByRegionIdx(pAd->pCfg80211_CB, pChDesc, NULL, NULL) != 0)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "Update RegRule failed!\n");

#endif /* RT_CFG80211_SUPPORT */
done:
	return num;
}


static UCHAR BuildChannelListFor5G(RTMP_ADAPTER *pAd, CHANNEL_CTRL *pChCtrl, USHORT PhyMode)
{
	UCHAR ChIdx, ChIdx2, num = 0, Radarnum = 0;
	PCH_DESC pChDesc = NULL;
	BOOLEAN bRegionFound = FALSE;
	PUCHAR pChannelList;
	PUCHAR pChannelListFlag;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "--->\n");

	for (ChIdx = 0; ChIdx < Country_Region_GroupNum_5GHZ; ChIdx++) {
		if ((pAd->CommonCfg.CountryRegionForABand & 0x7f) ==
			Country_Region_ChDesc_5GHZ[ChIdx].RegionIndex) {
			pChDesc = Country_Region_ChDesc_5GHZ[ChIdx].pChDesc;
			num = TotalChNum(pChDesc);
			pAd->CommonCfg.pChDesc5G = (PUCHAR)pChDesc;
			bRegionFound = TRUE;
			break;
		}
	}

	if (!bRegionFound) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"CountryRegionABand=%d not support",
			pAd->CommonCfg.CountryRegionForABand);
		goto done;
	}

	if (num > 0) {
		UCHAR ch_valid_idx = 0;
		UCHAR RadarCh[16] = {52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144};
#ifdef CONFIG_AP_SUPPORT
		UCHAR q = 0;
#endif
#ifdef MT_BAND4_DFS_SUPPORT /*302502*/
		UCHAR B4RadarCh[21] = {52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 149, 153, 157, 161, 165};

		if (pAd->CommonCfg.DfsParameter.band4DfsEnable)
			Radarnum = 21;
		else
#endif
			Radarnum = 16;
		os_alloc_mem(NULL, (UCHAR **)&pChannelList, num * sizeof(UCHAR));

		if (!pChannelList) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"Allocate memory for ChannelList failed\n");
			goto done;
		}

		os_alloc_mem(NULL, (UCHAR **)&pChannelListFlag, num * sizeof(UCHAR));

		if (!pChannelListFlag) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"Allocate memory for ChannelListFlag failed\n");
			os_free_mem(pChannelList);
			goto done;
		}

		for (ChIdx = 0; ChIdx < num; ChIdx++) {
			pChannelList[ChIdx] = GetChannel_5GHZ(pChDesc, ChIdx);
			pChannelListFlag[ChIdx] = GetChannelFlag(pChDesc, ChIdx);
		}
#ifdef CONFIG_AP_SUPPORT

		for (ChIdx = 0; ChIdx < num; ChIdx++) {
			if ((pAd->CommonCfg.bIEEE80211H == 0) || ((pAd->CommonCfg.bIEEE80211H == 1) && (pAd->CommonCfg.RDDurRegion != FCC))) {
				/* Profile parameter - ChannelGrp is enabled */
				if (MTChGrpValid(pChCtrl)) {
					/* Get channels according to ChannelGrp */
					if (MTChGrpChannelChk(pChCtrl, GetChannel_5GHZ(pChDesc, ChIdx))) {
						pChannelList[q] = GetChannel_5GHZ(pChDesc, ChIdx);
						pChannelListFlag[q] = GetChannelFlag(pChDesc, ChIdx);
						q++;
					}
				} else {
					pChannelList[q] = GetChannel_5GHZ(pChDesc, ChIdx);
					pChannelListFlag[q] = GetChannelFlag(pChDesc, ChIdx);
					q++;
				}
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
					"RDDurRegion != FCC - q=%d!\n",
					q);
			}
			/*Based on the requiremnt of FCC, some channles could not be used anymore when test DFS function.*/
			else if ((pAd->CommonCfg.bIEEE80211H == 1) &&
					 (pAd->CommonCfg.RDDurRegion == FCC) &&
					 (pAd->Dot11_H.bDFSIndoor == 1)) {
				if (MTChGrpValid(pChCtrl)) {
					if (MTChGrpChannelChk(pChCtrl, GetChannel_5GHZ(pChDesc, ChIdx))) {
						pChannelList[q] = GetChannel_5GHZ(pChDesc, ChIdx);
						pChannelListFlag[q] = GetChannelFlag(pChDesc, ChIdx);
						q++;
					}
				} else {
					pChannelList[q] = GetChannel_5GHZ(pChDesc, ChIdx);
					pChannelListFlag[q] = GetChannelFlag(pChDesc, ChIdx);
					q++;
				}
			} else if ((pAd->CommonCfg.bIEEE80211H == 1) &&
					   (pAd->CommonCfg.RDDurRegion == FCC) &&
					   (pAd->Dot11_H.bDFSIndoor == 0)) {
				if ((GetChannel_5GHZ(pChDesc, ChIdx) < 100) || (GetChannel_5GHZ(pChDesc, ChIdx) > 140)) {
					if (MTChGrpValid(pChCtrl)) {
						if (MTChGrpChannelChk(pChCtrl, GetChannel_5GHZ(pChDesc, ChIdx))) {
							pChannelList[q] = GetChannel_5GHZ(pChDesc, ChIdx);
							pChannelListFlag[q] = GetChannelFlag(pChDesc, ChIdx);
							q++;
						}
					} else {
						pChannelList[q] = GetChannel_5GHZ(pChDesc, ChIdx);
						pChannelListFlag[q] = GetChannelFlag(pChDesc, ChIdx);
						q++;
					}
				}
			} else if (MTChGrpValid(pChCtrl) && MTChGrpChannelChk(pChCtrl, GetChannel_5GHZ(pChDesc, ChIdx))) {
				pChannelList[q] = GetChannel_5GHZ(pChDesc, ChIdx);
				pChannelListFlag[q] = GetChannelFlag(pChDesc, ChIdx);
				q++;
			}
		}

		num = q;

		/* If Channel group is exclusive of supported channels of CountryRegionForABand */
		/* Build channel list based on channel group to avoid NULL channel list */
		if (num == 0 && MTChGrpValid(pChCtrl)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"Build channel list based on channel group to avoid NULL channel list!\n");
			for (ChIdx = 0; ChIdx < pChCtrl->ChGrpABandChNum; ChIdx++) {
				pChannelList[ChIdx] = pChCtrl->ChGrpABandChList[ChIdx];
				pChannelListFlag[ChIdx] = CHANNEL_DEFAULT_PROP;
			}

			num = pChCtrl->ChGrpABandChNum;
		}

#endif /* CONFIG_AP_SUPPORT */
		/*do FreqList check*/
		for (ChIdx = 0; ChIdx < num; ChIdx++) {
			if (ChannelValidForFreqList(pAd, pChannelList[ChIdx])) {
				pChannelList[ch_valid_idx] = pChannelList[ChIdx];
				pChannelListFlag[ch_valid_idx] = pChannelListFlag[ChIdx];
				ch_valid_idx++;
			}
		}

		num = ch_valid_idx;

		for (ChIdx = 0; ChIdx < num; ChIdx++) {
			for (ChIdx2 = 0; ChIdx2 < MAX_NUM_OF_CHANNELS; ChIdx2++) {
				if (pChannelList[ChIdx] == pAd->TxPower[ChIdx2].Channel)
					hc_set_ChCtrl(pChCtrl, pAd, ChIdx, ChIdx2);

				pChCtrl->ChList[ChIdx].Channel = pChannelList[ChIdx];
				pChCtrl->ChList[ChIdx].Flags = pChannelListFlag[ChIdx];
			}

			for (ChIdx2 = 0; ChIdx2 < Radarnum; ChIdx2++) {
#ifdef MT_BAND4_DFS_SUPPORT /*302502*/
				if (pAd->CommonCfg.DfsParameter.band4DfsEnable) {
					if (pChannelList[ChIdx] == B4RadarCh[ChIdx2])
						pChCtrl->ChList[ChIdx].DfsReq = TRUE;
				} else
#endif
					if (pChannelList[ChIdx] == RadarCh[ChIdx2])
						pChCtrl->ChList[ChIdx].DfsReq = TRUE;
			}
			if (!strncmp((RTMP_STRING *) pAd->CommonCfg.CountryCode, "CN", 2))
				pChCtrl->ChList[ChIdx].MaxTxPwr = pAd->MaxTxPwr;/*for CN CountryCode*/
			else
				pChCtrl->ChList[ChIdx].MaxTxPwr = 20;

#ifdef RT_CFG80211_SUPPORT
			CFG80211OS_ChanInfoInit(
				pAd->pCfg80211_CB,
				ChIdx,
				pChCtrl->ChList[ChIdx].Channel,
				pChCtrl->ChList[ChIdx].MaxTxPwr,
				TRUE,
				TRUE);
#endif /*RT_CFG80211_SUPPORT*/
		}

		pChCtrl->ChListNum = num;
		os_free_mem(pChannelList);
		os_free_mem(pChannelListFlag);
	}

#ifdef RT_CFG80211_SUPPORT

	if (CFG80211OS_UpdateRegRuleByRegionIdx(pAd->pCfg80211_CB, NULL, pChDesc, NULL) != 0)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "Update RegRule failed!\n");

#endif /*RT_CFG80211_SUPPORT*/
done:
	return num;
}

static UCHAR build_ch_list_for_6G(RTMP_ADAPTER *pAd, CHANNEL_CTRL *pChCtrl, UCHAR PhyMode)
{
	UCHAR ch_idx, ch_num = 0;
	PCH_DESC pChDesc = NULL;
	BOOLEAN bRegionFound = FALSE;
	PUCHAR pChannelList;
	PUCHAR pChannelListFlag;
	UCHAR ch_valid_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "--->\n");

	for (ch_idx = 0; ch_idx < Country_Region_GroupNum_6GHZ; ch_idx++) {
		if ((pAd->CommonCfg.CountryRegionForABand & 0x7f) ==
			Country_Region_ChDesc_6GHZ[ch_idx].RegionIndex) {
			pChDesc = Country_Region_ChDesc_6GHZ[ch_idx].pChDesc;
			ch_num = TotalChNum(pChDesc);
			pAd->CommonCfg.pChDesc5G = (PUCHAR)pChDesc;
			bRegionFound = TRUE;
			break;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"ch_num %d\n", ch_num);

	if (!bRegionFound) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"CountryRegionABand=%d not support",
			pAd->CommonCfg.CountryRegionForABand);
		goto done;
	}

	if (ch_num == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"ch_num=%d ch list is empty",
			ch_num);
		goto done;
	}

	os_alloc_mem(NULL, (UCHAR **)&pChannelList, ch_num * sizeof(UCHAR));

	if (!pChannelList) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"Allocate memory for ChannelList failed\n");
		goto done;
	}

	os_alloc_mem(NULL, (UCHAR **)&pChannelListFlag, ch_num * sizeof(UCHAR));

	if (!pChannelListFlag) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"Allocate memory for ChannelListFlag failed\n");
		os_free_mem(pChannelList);
		goto done;
	}

	/*do FreqList check*/
	for (ch_idx = 0; ch_idx < ch_num; ch_idx++) {
		if (ChannelValidForFreqList(pAd, GetChannel_5GHZ(pChDesc, ch_idx))) {
			pChannelList[ch_valid_idx] = GetChannel_5GHZ(pChDesc, ch_idx);
			pChannelListFlag[ch_valid_idx] = GetChannelFlag(pChDesc, ch_idx);
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"%s() ch_idx %d, ch_valid_idx %d!\n",
				__func__, ch_idx, ch_valid_idx);
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"%s() ch %d, ch_flag %d!\n",
				__func__, pChannelList[ch_valid_idx], pChannelListFlag[ch_valid_idx]);

			ch_valid_idx++;
		}
	}

	ch_num = ch_valid_idx;

	for (ch_idx = 0; ch_idx < ch_num; ch_idx++) {
		pChCtrl->ChList[ch_idx].Channel = pChannelList[ch_idx];
		pChCtrl->ChList[ch_idx].Flags = pChannelListFlag[ch_idx];
#ifdef CONFIG_6G_SUPPORT
		if (PSC_Ch_Check(pChCtrl->ChList[ch_idx].Channel))
			pChCtrl->ChList[ch_idx].PSC_Ch = TRUE;
#endif
	}

	pChCtrl->ChListNum = ch_num;
	os_free_mem(pChannelList);
	os_free_mem(pChannelListFlag);

#ifdef RT_CFG80211_SUPPORT
	if (CFG80211OS_UpdateRegRuleByRegionIdx
			(pAd->pCfg80211_CB, NULL, NULL, pChDesc) != 0)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Update RegRule failed!\n");
#endif /*RT_CFG80211_SUPPORT*/

done:
	return ch_num;
}


#if defined(CONFIG_STA_SUPPORT)
static bool Set_Diff_Bw(RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR bw, UCHAR ext_ch)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR op_ht_bw, op_ext_ch;
	struct _RTMP_CHIP_CAP *cap;
	pEntry = entry_get(pAd, wcid);
	wdev = pEntry->wdev;

	if (!wdev)
		return FALSE;

	op_ht_bw = wlan_operate_get_ht_bw(wdev);
	op_ext_ch = wlan_operate_get_ext_cha(wdev);
	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (op_ht_bw != bw) {
		wlan_operate_set_ht_bw(wdev, bw, ext_ch);
		pEntry->HTPhyMode.field.BW = bw;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

		if (cap->fgRateAdaptFWOffload == TRUE) {
			CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;
			NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));

			if (bw == HT_BW_40)
				rRaParam.u4Field = RA_PARAM_HT_2040_BACK;
			else
				rRaParam.u4Field = RA_PARAM_HT_2040_COEX;

			RAParamUpdate(pAd, pEntry, &rRaParam);
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"FallBack APClient BW to %s\n",
				rRaParam.u4Field == (RA_PARAM_HT_2040_BACK) ? "BW_40" : "BW_20");
		}

#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	}

	return TRUE;
}
#endif	/* CONFIG_STA_SUPPORT */

static VOID channel_related_info_backup(
	struct _RTMP_ADAPTER *pAd,
	struct _CH_PRIO_INFO *pChPrio,
	unsigned char *MaxChPrioBackup,
	unsigned char *powner)
{
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	UCHAR ChIdx;

	if (pChCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"ERROR: %s's pChCtrl is NULL !\n", __func__);
		return;
	}

	if (pChCtrl->ChListNum > MAX_NUM_OF_CHANNELS) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"ChListNum = %d, out of [0-%d] range.\n",
			pChCtrl->ChListNum, MAX_NUM_OF_CHANNELS);
		return;
	}

	if (pChCtrl->ChListNum == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
			"ChListNum = 0, no need to backup, update from cfg.\n");
		return;
	}

	/*Storing data*/
	/*get lock,to avoid being interrupted by other processes*/
	NdisAcquireSpinLock(&pChCtrl->ChPrioOpLock);

	pChPrio->ch_num = pChCtrl->ChListNum;
	*powner = pChCtrl->CurChPrioOpOwner;
	*MaxChPrioBackup = pChCtrl->MaxChPrio;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"=================================================\n");
	for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
		pChPrio->ch_list[ChIdx].channel = pChCtrl->ChList[ChIdx].Channel;
		pChPrio->ch_list[ChIdx].priority = pChCtrl->ChList[ChIdx].Priority;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"channel = %d, priority = %d.\n", pChPrio->ch_list[ChIdx].channel,
			pChPrio->ch_list[ChIdx].priority);
	}
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"owner is %d\n", *powner);

	/*unlock channel priority operation lock*/
	NdisReleaseSpinLock(&pChCtrl->ChPrioOpLock);

	#ifdef MT_DFS_SUPPORT
	/*back up NOP list*/
	Nop_List_Backup(pAd);
	#endif

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"=================================================\n");
}

static VOID channel_related_info_restore(
	struct _RTMP_ADAPTER *pAd,
	struct _CH_PRIO_INFO *pChPrio,
	UCHAR MaxChPrioBackup,
	UCHAR owner)
{
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	UCHAR ChIdx, DFSBackupChIdx;
	NOP_LIST *pNopList = (NOP_LIST *)pAd->NopListBk;

	if (pChCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"ERROR: %s's pChCtrl is NULL !\n", __func__);
		return;
	}

	if (pChCtrl->ChListNum > MAX_NUM_OF_CHANNELS) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"ChListNum = %d, out of [0-%d] range.\n",
			pChCtrl->ChListNum, MAX_NUM_OF_CHANNELS);
		return;
	}

	if (pChCtrl->ChListNum == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"ChListNum = 0, no need to restore, update from cfg.\n");
		if (!LoadCfgChprio(pChCtrl))
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"load cfg channel priority failed!\n");
		return;
	}

	if (pChCtrl->ChListNum != pChPrio->ch_num) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"ChList has been changed, no need to restore, update from cfg.\n");
		if (!LoadCfgChprio(pChCtrl))
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"load cfg channel priority failed!\n");
		return;
	}

	/*get lock,to avoid being interrupted by other processes*/
	NdisAcquireSpinLock(&pChCtrl->ChPrioOpLock);

	if (pChCtrl->NeedResetToCfgChPrio) {
		/*unlock channel priority operation lock*/
		NdisReleaseSpinLock(&pChCtrl->ChPrioOpLock);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"flag needresettocfgchprio is set, update from cfg.\n");
		if (!LoadCfgChprio(pChCtrl))
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"load cfg channel priority failed!\n");
		return;
	}

	/* Restore channel related info */
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"-----------------------------\n");
	for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
		if (pChPrio->ch_list[ChIdx].channel != pChCtrl->ChList[ChIdx].Channel) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"current channel no match with backup channel, update from cfg.\n");
			NdisReleaseSpinLock(&pChCtrl->ChPrioOpLock);
			if (!LoadCfgChprio(pChCtrl))
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"load cfg channel priority failed!\n");
			return;
		}

		pChCtrl->ChList[ChIdx].Channel = pChPrio->ch_list[ChIdx].channel;
		pChCtrl->ChList[ChIdx].Priority = pChPrio->ch_list[ChIdx].priority;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"channel %d, priority %d\n", pChCtrl->ChList[ChIdx].Channel, pChCtrl->ChList[ChIdx].Priority);
	}
	pChCtrl->MaxChPrio = MaxChPrioBackup;
	pChCtrl->CurChPrioOpOwner = owner;
	/*unlock channel priority operation lock*/
	NdisReleaseSpinLock(&pChCtrl->ChPrioOpLock);

	/*restore NOP*/
	if (pAd->NopListBk) {
		if (pNopList->ChListNum > DFS_AVAILABLE_LIST_CH_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
						"pNopList->ChListNum invalid.\n");
			os_free_mem(pAd->NopListBk);
			pAd->NopListBk = NULL;
		return;
	}

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Sync NopList count:%d\n", pNopList->ChListNum);
		for (DFSBackupChIdx = 0; DFSBackupChIdx < pNopList->ChListNum; DFSBackupChIdx++) {
			for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
				if (pChCtrl->ChList[ChIdx].Channel == pNopList->DfsChList[DFSBackupChIdx].Channel) {
					pChCtrl->ChList[ChIdx].NonOccupancy = pNopList->DfsChList[DFSBackupChIdx].NonOccupancy;
					pChCtrl->ChList[ChIdx].NOPClrCnt = pNopList->DfsChList[DFSBackupChIdx].NOPClrCnt;
					pChCtrl->ChList[ChIdx].NOPSetByBw = pNopList->DfsChList[DFSBackupChIdx].NOPSetByBw;
					pChCtrl->ChList[ChIdx].NOPSaveForClear = pNopList->DfsChList[DFSBackupChIdx].NOPSaveForClear;
					pChCtrl->ChList[ChIdx].SupportBwBitMap = pNopList->DfsChList[DFSBackupChIdx].SupportBwBitMap;
					pChCtrl->ChList[ChIdx].IsOutBandCacDone = pNopList->DfsChList[DFSBackupChIdx].IsOutBandCacDone;
				}
			}
		}
		/* clear nop list as sync to channel list done*/
		os_free_mem(pAd->NopListBk);
		pAd->NopListBk = NULL;
	}
}

VOID BuildChannelList(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR ChIdx = 0, MaxChPrioBackup = 0, OwnerBackup = 0;
	struct _CH_PRIO_INFO ChPrioInfo = {0};
	/*Get Band index from wdev*/
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	USHORT PhyMode = wdev->PhyMode;
	/* Get channel ctrl address */
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	if (!pChCtrl) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
		"ERROR: %s's pChCtrl is NULL !\n", __func__);
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
		"BandIdx = %d, PhyMode = %d\n", BandIdx, PhyMode);

#ifdef WIFI_MD_COEX_SUPPORT
	if (pAd->LteSafeChCtrl.bEnabled && !pAd->LteSafeChCtrl.bQueryLteDone) {
		HW_QUERY_LTE_SAFE_CHANNEL(pAd);
		pAd->LteSafeChCtrl.bQueryLteDone = TRUE;
	}
#endif

	/* Check state of channel list */
	if (hc_check_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_DONE)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"BandIdx %d, channel list is already DONE\n", BandIdx);
		return;
	}

	/* Backup channel related information */
	channel_related_info_backup(pAd, &ChPrioInfo, &MaxChPrioBackup, &OwnerBackup);

	/* Initialize channel list*/
	os_zero_mem(pChCtrl->ChList, MAX_NUM_OF_CHANNELS * sizeof(CHANNEL_TX_POWER));
	pChCtrl->ChListNum = 0;

	/* Build channel list based on PhyMode */
	/* do not change sequence due to 6GHz might include AC/GN then confused */
	if (WMODE_CAP_6G(PhyMode))
		build_ch_list_for_6G(pAd, pChCtrl, PhyMode);
	else if (WMODE_CAP_2G(PhyMode))
		BuildChannelListFor2G(pAd, pChCtrl, PhyMode);
	else if (WMODE_CAP_5G(PhyMode))
		BuildChannelListFor5G(pAd, pChCtrl, PhyMode);

	/*restore channel related info*/
	channel_related_info_restore(pAd, &ChPrioInfo, MaxChPrioBackup, OwnerBackup);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
		"BandIdx = %d, PhyMode = %d, ChListNum = %d:\n",
		BandIdx, PhyMode, pChCtrl->ChListNum);

	/* Build Channel CAP, should after get ChListNum */
#ifdef DOT11_N_SUPPORT

	for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
		if (N_ChannelGroupCheck(pAd, pChCtrl->ChList[ChIdx].Channel, wdev))
			hc_set_ChCtrlFlags_CAP(pChCtrl, CHANNEL_40M_CAP, ChIdx);

#ifdef DOT11_VHT_AC
		if (vht80_channel_group(pAd, pChCtrl->ChList[ChIdx].Channel, wdev))
			hc_set_ChCtrlFlags_CAP(pChCtrl, CHANNEL_80M_CAP, ChIdx);

		if (vht160_channel_group(pAd, pChCtrl->ChList[ChIdx].Channel, wdev))
			hc_set_ChCtrlFlags_CAP(pChCtrl, CHANNEL_160M_CAP, ChIdx);

#endif /* DOT11_VHT_AC */
#ifdef DOT11_EHT_BE
		if (eht320_channel_group(pAd, pChCtrl->ChList[ChIdx].Channel, wdev))
			hc_set_ChCtrlFlags_CAP(pChCtrl, CHANNEL_320M_CAP, ChIdx);
#endif /* #ifdef DOT11_EHT_BE */
	}
#endif /* DOT11_N_SUPPORT */

	if (WMODE_CAP_6G(PhyMode)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"CountryCode(6G)=%d, RFIC=%d, support %d channels\n",
			pAd->CommonCfg.CountryRegion, pAd->RfIcType, pChCtrl->ChListNum);
	} else if (WMODE_CAP_2G(PhyMode)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"CountryCode(2.4G)=%d, RFIC=%d, support %d channels\n",
			pAd->CommonCfg.CountryRegion, pAd->RfIcType, pChCtrl->ChListNum);
	} else if (WMODE_CAP_5G(PhyMode)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"CountryCode(5G)=%d, RFIC=%d, support %d channels\n",
			pAd->CommonCfg.CountryRegionForABand, pAd->RfIcType, pChCtrl->ChListNum);
	}

#ifdef MT_DFS_SUPPORT
	pAd->CommonCfg.DfsParameter.NeedSetNewChList = DFS_SET_NEWCH_ENABLED;
	DfsBuildChannelList(pAd, wdev);
#endif

#ifdef WIFI_MD_COEX_SUPPORT
	LteSafeBuildChnBitmask(pAd);
#endif
	/* Set state of channel list*/
	hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_DONE);
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"%s() SupportedChannelList (ChCtrlStat = DONE):\n", __func__);

	for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"\tChannel # %d: Pwr0/1 = %d/%d, Flags = %x\n ",
			pChCtrl->ChList[ChIdx].Channel,
			pChCtrl->ChList[ChIdx].Power,
			pChCtrl->ChList[ChIdx].Power2,
			pChCtrl->ChList[ChIdx].Flags);
	}
}

/*
	==========================================================================
	Description:
		This routine return the first channel number according to the country
		code selection and RF IC selection (signal band or dual band). It is called
		whenever driver need to start a site survey of all supported channels.
	Return:
		ch - the first channel number of current country code setting

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
UCHAR FirstChannel(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
#ifdef CONFIG_6G_SUPPORT
	struct _SCAN_CTRL_ *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

	if ((wlan_config_get_ch_band(wdev) == CMD_CH_BAND_6G) &&
		(ScanCtrl->psc_scan_en == TRUE))
		return pChCtrl->ChList_6G_psc_scan[0].Channel;
#endif
	return pChCtrl->ChList[0].Channel;
}

/*
	==========================================================================
	Description:
		This routine returns the first non-DFS channel number. This routine is called
		during driver need to start a site survey of all supported channels.
	Return:
		ch - the first channel number valid in current country code setting.
	Note:
		return 0 if no non-DFS channel
	==========================================================================
 */
UCHAR FirstNonDfsChannel(RTMP_ADAPTER *pAd)
{
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	UCHAR ch = 0;
	UCHAR i;
	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (pChCtrl->ChList[i].DfsReq != TRUE) {
			ch = pChCtrl->ChList[i].Channel;
			break;
		}
	}
	return ch;
}

/*
	==========================================================================
	Description:
		This routine returns the next channel number. This routine is called
		during driver need to start a site survey of all supported channels.
	Return:
		next_channel - the next channel number valid in current country code setting.
	Note:
		return 0 if no more next channel
	==========================================================================
 */
UCHAR NextChannel(
	RTMP_ADAPTER *pAd,
	SCAN_CTRL *ScanCtrl,
	UCHAR channel,
	struct wifi_dev *wdev)
{
	int i;
	UCHAR next_channel = 0;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

#ifdef CONFIG_6G_SUPPORT
	if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_6G) {
		if (ScanCtrl->psc_scan_en == TRUE) {
			for (i = 0; i < (pChCtrl->ChListNum_6G_psc_scan - 1); i++) {
				if (channel == pChCtrl->ChList_6G_psc_scan[i].Channel) {
					next_channel = pChCtrl->ChList_6G_psc_scan[i + 1].Channel;
					break;
				}
			}

		} else {
			for (i = 0; i < (pChCtrl->ChListNum - 1); i++) {
				if (channel == pChCtrl->ChList[i].Channel) {
					next_channel = pChCtrl->ChList[i + 1].Channel;
					break;
				}
			}
		}
		return next_channel;
	}
#endif

	for (i = 0; i < (pChCtrl->ChListNum - 1); i++) {
		if (channel == pChCtrl->ChList[i].Channel) {
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

			/* Only scan effected channel if this is a SCAN_2040_BSS_COEXIST*/
			/* 2009 PF#2: Nee to handle the second channel of AP fall into affected channel range.*/
			if ((ScanCtrl->ScanType == SCAN_2040_BSS_COEXIST) && (pChCtrl->ChList[i + 1].Channel > 14)) {
				channel = pChCtrl->ChList[i + 1].Channel;
				continue;
			} else
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef MT_DFS_SUPPORT
			if (pChCtrl->ChList[i + 1].NonOccupancy > 0 || pChCtrl->ChList[i + 1].NOPSaveForClear > 0) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
					"Skip scanning channel %u due to remaining %u/%u sec NOP\n",
					pChCtrl->ChList[i + 1].Channel,
					pChCtrl->ChList[i + 1].NonOccupancy,
					pChCtrl->ChList[i + 1].NOPSaveForClear);
				channel = pChCtrl->ChList[i + 1].Channel;
				continue;
			} else
#endif /* MT_DFS_SUPPORT */
			{
				/* Record this channel's idx in ChannelList array.*/
				next_channel = pChCtrl->ChList[i + 1].Channel;
				break;
			}
		}
	}

	return next_channel;
}


/*
	==========================================================================
	Description:
		This routine is for Cisco Compatible Extensions 2.X
		Spec31. AP Control of Client Transmit Power
	Return:
		None
	Note:
	   Required by Aironet dBm(mW)
		   0dBm(1mW),   1dBm(5mW), 13dBm(20mW), 15dBm(30mW),
		  17dBm(50mw), 20dBm(100mW)

	   We supported
		   3dBm(Lowest), 6dBm(10%), 9dBm(25%), 12dBm(50%),
		  14dBm(75%),   15dBm(100%)

		The client station's actual transmit power shall be within +/- 5dB of
		the minimum value or next lower value.
	==========================================================================
 */
VOID ChangeToCellPowerLimit(RTMP_ADAPTER *pAd, UCHAR PowerLimit)
{
	/*
		valud 0xFF means that hasn't found power limit information
		from the AP's Beacon/Probe response
	*/
	if (PowerLimit == 0xFF)
		return;

	if (PowerLimit < 6) { /*Used Lowest Power Percentage.*/
		pAd->CommonCfg.ucTxPowerPercentage = 6;
	} else if (PowerLimit < 9) {
		pAd->CommonCfg.ucTxPowerPercentage = 10;
	} else if (PowerLimit < 12) {
		pAd->CommonCfg.ucTxPowerPercentage = 25;
	} else if (PowerLimit < 14) {
		pAd->CommonCfg.ucTxPowerPercentage = 50;
	} else if (PowerLimit < 15) {
		pAd->CommonCfg.ucTxPowerPercentage = 75;
	} else {
		pAd->CommonCfg.ucTxPowerPercentage = 100; /*else used maximum*/
	}

	if (pAd->CommonCfg.ucTxPowerPercentage > pAd->CommonCfg.ucTxPowerDefault)
		pAd->CommonCfg.ucTxPowerPercentage = pAd->CommonCfg.ucTxPowerDefault;
}

CHAR ConvertToRssi(RTMP_ADAPTER *pAd, struct raw_rssi_info *rssi_info, UCHAR rssi_idx)
{
	UCHAR RssiOffset = 0, LNAGain;
	CHAR BaseVal;
	CHAR rssi, ret = 0;
#ifdef CONNAC_EFUSE_FORMAT_SUPPORT
	UCHAR FeOffset[4] = {0};
#endif /* CONNAC_EFUSE_FORMAT_SUPPORT */
	/* Rssi equals to zero or rssi_idx larger than 3 should be an invalid value*/
	if (rssi_idx >= pAd->Antenna.field.RxPath)
		return -99;

	rssi_idx = min(rssi_idx, (UCHAR)(sizeof(rssi_info->raw_rssi) / sizeof(CHAR)));
	rssi = rssi_info->raw_rssi[rssi_idx];

	if (rssi == 0)
		return -99;

	LNAGain = pAd->hw_cfg.lan_gain;
#ifdef CONNAC_EFUSE_FORMAT_SUPPORT
	AsicFeLossGet(pAd, pAd->LatchRfRegs.Channel, FeOffset);
	RssiOffset = FeOffset[rssi_idx];
#else
	if (pAd->LatchRfRegs.Channel > 14)
		RssiOffset = pAd->ARssiOffset[rssi_idx];
	else
		RssiOffset = pAd->BGRssiOffset[rssi_idx];
#endif /* CONNAC_EFUSE_FORMAT_SUPPORT */

	BaseVal = -12;

	ret = (rssi + (CHAR)RssiOffset - (CHAR)LNAGain);
	return ret;
}


CHAR ConvertToSnr(RTMP_ADAPTER *pAd, UCHAR Snr)
{
	CHAR ret = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->SnrFormula == SNR_FORMULA2) {
		ret = (Snr * 3 + 8) >> 4;
		return ret;
	} else if (cap->SnrFormula == SNR_FORMULA3) {
		ret = (Snr * 3 / 16);
		return ret; /* * 0.1881 */
	} else {
		ret = ((0xeb	- Snr) * 3) /	16;
		return ret;
	}
}

static UINT8 ch_offset_abs(UINT8 x, UINT8 y)
{

	if (x > y)
		return x - y;
	else
		return y - x;
}

UCHAR check_vht_op_bw(struct vht_opinfo *vht_op_info,
			VHT_CAP_IE *vht_cap_ie, ADD_HTINFO2 *add_ht_info)
{
	UCHAR bw = 0;
	UINT8 p80ccf = vht_op_info->ccfs_0;
	UINT8 s80160ccf = vht_op_info->ccfs_1;

	switch (vht_op_info->ch_width) {
	case VHT_BW_2040:
		bw = VHT_BW_2040;
		break;
	case VHT_BW_80:
		if (s80160ccf == 0) {
			if ((vht_cap_ie->vht_cap.ex_nss_bw > 0)
				&& (add_ht_info->CentFreq2 > 0))
				bw = VHT_BW_160;
			else
				bw = VHT_BW_80;
		} else if (ch_offset_abs(s80160ccf, p80ccf) == 8) {
			bw = VHT_BW_160;
		} else if (ch_offset_abs(s80160ccf, p80ccf) >= 16) {
			bw = VHT_BW_8080;
		}
		break;
	case VHT_BW_160:
		bw = VHT_BW_160;
		break;
	case VHT_BW_8080:
		bw = VHT_BW_8080;
		break;
	default:
		break;
	}

	return bw;
}

#ifdef BW_VENDOR10_CUSTOM_FEATURE
/* BW Sync when Soft AP is Down */
BOOLEAN CheckSoftAPSyncRequired(RTMP_ADAPTER *pAd, struct wifi_dev *wdev_ap, struct common_ies *cmm_ies)
{
	/* Soft AP must be disabled */
	if (wlan_operate_get_state(wdev_ap) != WLAN_OPER_STATE_INVALID)
		return FALSE;

	if (WMODE_CAP_N(wdev_ap->PhyMode) && wdev_ap->channel < 14) {
		if (wlan_operate_get_ht_bw(wdev_ap) != cmm_ies->ht_op.AddHtInfo.RecomWidth)
			wdev_sync_ht_bw(pAd, wdev_ap, &cmm_ies->ht_op.AddHtInfo);
	}

	if (WMODE_CAP_AC(wdev_ap->PhyMode)) {
		if (wlan_operate_get_vht_bw(wdev_ap) != cmm_ies->vht_op.vht_op_info.ch_width)
			wdev_sync_vht_bw(pAd, wdev_ap, cmm_ies->vht_op.vht_op_info.ch_width, cmm_ies->vht_op.vht_op_info.ccfs_1);
	}

	return TRUE;
}
#endif

#ifdef CONFIG_STA_SUPPORT
BOOLEAN AdjustBwToSyncAp(RTMP_ADAPTER *pAd, BCN_IE_LIST *ie_list, struct wifi_dev *wdev)
{
	BOOLEAN bAdjust = FALSE;
#ifdef DOT11_N_SUPPORT
	PMAC_TABLE_ENTRY pEntry = NULL;
	UCHAR op_ht_bw = 0;
	UCHAR cfg_ht_bw = 0;
	UCHAR ExtCha = 0;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	UCHAR channel_bw_cap;
#ifdef DOT11_VHT_AC
	/*UINT16 VhtOperationBW = wlan_operate_get_vht_bw(wdev);*/
	UCHAR op_vht_bw = 0;
	UCHAR cfg_vht_bw = 0;
	BOOLEAN bSupportVHTMCS1SS = FALSE;
	BOOLEAN bSupportVHTMCS2SS = FALSE;
	BOOLEAN bSupportVHTMCS3SS = FALSE;
	BOOLEAN bSupportVHTMCS4SS = FALSE;
	CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam = {0};
#endif /* DOT11_VHT_AC */
	struct common_ies *cmm_ies = &ie_list->cmm_ies;
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	BOOLEAN bAdjustHTBW = FALSE, bAdjustVHTBW = FALSE;
	UCHAR softap_op_ht_bw = 0;
	UCHAR softap_op_vht_bw = 0;
	UINT_8 i = 0;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *pwdev = NULL;

	if (pAd == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
			"pAd == NULL!\n");
		return FALSE;
	}

	/* Soft AP wdev/mbss */
	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		pMbss = &pAd->ApCfg.MBSSID[i];
		pwdev = &pAd->ApCfg.MBSSID[i].wdev;
		if ((pMbss) && (pwdev) && (pMbss->wdev.wdev_type == WDEV_TYPE_AP) && (pwdev->channel == wdev->channel))
			break;
	}
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
		"\nwdev_channel %d pwdev_channel %d\n", wdev->channel, pwdev->channel);
#endif

	if (!ie_list || !pStaCfg)
		return FALSE;

	pEntry = entry_get(pAd, pStaCfg->MacTabWCID);

	cfg_ht_bw = wlan_config_get_ht_bw(wdev);

	if (WMODE_CAP_N(wdev->PhyMode) && HAS_HT_OP_EXIST(cmm_ies->ie_exists) && wdev->channel < 14) {
		op_ht_bw = wlan_operate_get_ht_bw(wdev);
		ExtCha = wlan_operate_get_ext_cha(wdev);

		/* BW 40 -> 20 */
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		if (pwdev) {
			softap_op_ht_bw = wlan_operate_get_ht_bw(pwdev);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
				"op_ht_bw %d softap_op_bw %d softap_op_ht_bw %d cfg_ht_bw %d\n",
				op_ht_bw, wlan_operate_get_bw(pwdev), wlan_operate_get_ht_bw(pwdev), cfg_ht_bw);
		}

		if (op_ht_bw == HT_BW_40 || (pwdev && softap_op_ht_bw == HT_BW_40))
#else
		if (op_ht_bw == HT_BW_40)
#endif
		{
			/* Check if root-ap change BW to 20 */
			if ((cmm_ies->ht_op.AddHtInfo.ExtChanOffset == EXTCHA_NONE &&
				 cmm_ies->ht_op.AddHtInfo.RecomWidth == 0)
				|| (ie_list->NewExtChannelOffset == 0x0)) {
				bAdjust = TRUE;
				Set_Diff_Bw(pAd, pStaCfg->MacTabWCID, BW_20, EXTCHA_NONE);
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					"FallBack APClient BW to 20MHz\n");
#ifdef BW_VENDOR10_CUSTOM_FEATURE
				/* Sync new BW & Ext Channel for Soft AP */
				if (pwdev && IS_SYNC_BW_POLICY_VALID(pAd, TRUE, HT_4020_DOWN_ENBL)) {
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
							" Enter 4020 HT Sync\n");
					wdev_sync_ht_bw(pAd, pwdev, &cmm_ies->ht_op.AddHtInfo);
					bAdjustHTBW = TRUE;
				}
#endif
			}
		}
			/* BW 20 -> 40 */
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		else if ((op_ht_bw == HT_BW_20 || (pwdev && softap_op_ht_bw == HT_BW_20)) && cfg_ht_bw != HT_BW_20)
#else
		else if (op_ht_bw == HT_BW_20 && cfg_ht_bw != HT_BW_20)
#endif
		{
			/* Check if root-ap change BW to 40 */
			if (cmm_ies->ht_op.AddHtInfo.ExtChanOffset != EXTCHA_NONE &&
				cmm_ies->ht_op.AddHtInfo.RecomWidth == 1 &&
				HAS_HT_CAPS_EXIST(cmm_ies->ie_exists) &&
				cmm_ies->ht_cap.HtCapInfo.ChannelWidth == 1) {
				/* if extension channel is same with root ap else keep 20MHz */
				if (ExtCha == EXTCHA_NONE || cmm_ies->ht_op.AddHtInfo.ExtChanOffset == ExtCha) {
					/* UCHAR RFChannel; */
					bAdjust = TRUE;
					Set_Diff_Bw(pAd, pStaCfg->MacTabWCID, BW_40, cmm_ies->ht_op.AddHtInfo.ExtChanOffset);
					/*update channel for all of wdev belong this band*/
					wlan_operate_set_prim_ch(wdev, wdev->channel);
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "FallBack Client/APClient BW to 40MHz\n");
				}
#ifdef BW_VENDOR10_CUSTOM_FEATURE
				/* Sync new BW & Ext Channel for Soft AP */
				if (pwdev && IS_SYNC_BW_POLICY_VALID(pAd, TRUE, HT_2040_UP_ENBL)) {
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
						" Enter 2040 HT Sync\n");
					wdev_sync_ht_bw(pAd, pwdev, &cmm_ies->ht_op.AddHtInfo);
					bAdjustHTBW = TRUE;
				}
#endif
			}
		}

		if (bAdjust == TRUE) {
#ifdef MAC_REPEATER_SUPPORT
			if (pAd->ApCfg.bMACRepeaterEn) {
				UCHAR CliIdx, update_bw, update_ext_ch;
				REPEATER_CLIENT_ENTRY *rept = NULL;
				RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

				update_bw = wlan_operate_get_ht_bw(wdev);
				update_ext_ch = wlan_operate_get_ext_cha(wdev);

				for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
					rept = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
					if (IS_REPT_LINK_UP(rept) && (rept->main_wdev == &pStaCfg->wdev)) {
						if (rept->pMacEntry) {
							Set_Diff_Bw(pAd, rept->pMacEntry->wcid, update_bw, update_ext_ch);
						}
					}
				}

				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "FallBack APClient BW to (%d)\n", update_bw);
			}
#endif /* MAC_REPEATER_SUPPORT */
		}
	}

#ifdef DOT11_VHT_AC
	/**
	 * This code is for VHT BW switching which follows VHT Operation mode.
	 * AP communicates VHT operation mode in VHT Operation IE
	 */
	if (WMODE_CAP_AC(wdev->PhyMode) && (pStaCfg->StaActive.SupportedPhyInfo.bVhtEnable == TRUE)) {
		if (HAS_VHT_OP_EXIST(cmm_ies->ie_exists) && HAS_HT_OP_EXIST(cmm_ies->ie_exists)) {

			UCHAR current_operating_bw = 0;
			UCHAR prev_operating_bw = 0;
#ifdef BW_VENDOR10_CUSTOM_FEATURE
			UCHAR softap_prev_op_bw = 0;
#endif
			UINT8 p80ccf = cmm_ies->vht_op.vht_op_info.ccfs_0;
			UINT8 s80160ccf = cmm_ies->vht_op.vht_op_info.ccfs_1;
			UCHAR AP_Operting_BW = cmm_ies->vht_op.vht_op_info.ch_width;
			USHORT AP_HT_centr_freq2 = cmm_ies->ht_op.AddHtInfo2.CentFreq2;
			UINT8 AP_vht_ex_nss_bw = cmm_ies->vht_cap.vht_cap.ex_nss_bw;
			/*P_RA_ENTRY_INFO_T pRaEntry = &pEntry->RaEntry;*/
			BOOLEAN force_ra_update = FALSE;
			cfg_vht_bw = wlan_config_get_vht_bw(wdev);
			op_ht_bw = wlan_operate_get_ht_bw(wdev);
			op_vht_bw = wlan_operate_get_vht_bw(wdev);
#ifdef BW_VENDOR10_CUSTOM_FEATURE
			if (pwdev) {
				softap_op_ht_bw = wlan_operate_get_ht_bw(pwdev);
				softap_op_vht_bw = wlan_operate_get_vht_bw(pwdev);
				/* Beacon Sync Scenario */
				if (softap_op_ht_bw == BW_20)
					softap_prev_op_bw = BW_20;
				else if (softap_op_vht_bw == VHT_BW_2040)
					softap_prev_op_bw = 1; /*40Mhz*/
				else if (softap_op_vht_bw == VHT_BW_80)
					softap_prev_op_bw = 2; /*80Mhz*/
				else if ((softap_op_vht_bw == VHT_BW_160) ||
					(softap_op_vht_bw == VHT_BW_8080))
					softap_prev_op_bw = 3; /*80_80,160Mhz*/

				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					" ApCli Wdev %d AP Wdev %d AP BW %d\n",
					wdev->wdev_idx, pwdev->wdev_idx, softap_prev_op_bw);
			}
#endif


			if ((cmm_ies->ht_op.AddHtInfo.ExtChanOffset == EXTCHA_NONE &&
						cmm_ies->ht_op.AddHtInfo.RecomWidth == 0))
				current_operating_bw = 0; /*20Mhz*/
			else{
				if (AP_Operting_BW == VHT_BW_2040) {
					current_operating_bw = 1; /*40Mhz*/
				} else if (AP_Operting_BW == VHT_BW_80) {
					if (cfg_vht_bw <= VHT_BW_80)
						current_operating_bw = 2; /*80Mhz*/
					else if (cfg_vht_bw > VHT_BW_80) {
						if (s80160ccf == 0) {
							if (AP_vht_ex_nss_bw > 0 && AP_HT_centr_freq2 > 0) {
								AP_Operting_BW = VHT_BW_160;
								current_operating_bw = BW_160; /*160Mhz*/
							} else
								current_operating_bw = BW_80; /*80Mhz*/
						} else if (ch_offset_abs(s80160ccf, p80ccf) == 8) {
							AP_Operting_BW = VHT_BW_160;
							current_operating_bw = 3; /*160Mhz*/
						} else if (ch_offset_abs(s80160ccf, p80ccf) >= 16) {
							AP_Operting_BW = VHT_BW_8080;
							current_operating_bw = 3; /*80_80*/
							if (wlan_operate_get_cen_ch_2(wdev) != s80160ccf) {
								wlan_operate_set_cen_ch_2(wdev, s80160ccf);
								force_ra_update = TRUE;
							}
						}
					}
#ifdef BW_VENDOR10_CUSTOM_FEATURE
					/* Current Op BW = 20/40 */
					else if (cfg_vht_bw == VHT_BW_2040)
						current_operating_bw = 2;
#endif
				} else if ((AP_Operting_BW == VHT_BW_160) ||
						(AP_Operting_BW == VHT_BW_8080)) {
					current_operating_bw = 3; /*80_80,160Mhz*/
				}
				pAd->PeerApccfs1 = s80160ccf;
			}

			if (op_ht_bw == HT_BW_20)
				prev_operating_bw = BW_20;
			else if (op_vht_bw == VHT_BW_2040)
				prev_operating_bw = 1; /*40Mhz*/
			else if (op_vht_bw == VHT_BW_80)
				prev_operating_bw = 2; /*80Mhz*/
			else if ((op_vht_bw == VHT_BW_160) ||
					(op_vht_bw == VHT_BW_8080))
				prev_operating_bw = 3; /*80_80,160Mhz*/
#ifdef BW_VENDOR10_CUSTOM_FEATURE
			if ((prev_operating_bw != current_operating_bw) ||
				(pwdev && (softap_prev_op_bw != current_operating_bw)))
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					"BW Diff (apcli previous= %d softap previous= %d current=%d)\n",
					 prev_operating_bw, softap_prev_op_bw, current_operating_bw);
#else
			if (prev_operating_bw != current_operating_bw)
#endif
			{
				/* Get Channel Bandwidth capability */
				channel_bw_cap = get_channel_bw_cap(&pStaCfg->wdev, pStaCfg->wdev.channel);
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					"SYNC - Peer AP Changed VHT BW[old:new] = [%u:%u], HT BW[old:new] = [%u:%u]\n",
					prev_operating_bw,
					current_operating_bw,
					op_ht_bw,
					cmm_ies->ht_op.AddHtInfo.RecomWidth);
				SET_VHT_OP_EXIST(pStaCfg->MlmeAux.ie_exists);
				NdisMoveMemory(&pStaCfg->MlmeAux.vht_op, &cmm_ies->vht_op, SIZE_OF_VHT_OP_IE);

				pStaCfg->MlmeAux.AddHtInfo.AddHtInfo.RecomWidth =
					cmm_ies->ht_op.AddHtInfo.RecomWidth;
				pStaCfg->MlmeAux.AddHtInfo.AddHtInfo.ExtChanOffset =
					cmm_ies->ht_op.AddHtInfo.ExtChanOffset;
				/* Added check to take care when BW in HT capability IE is set
				* to 0(20MHZ) and HT info IE set to 1
				 */
				if ((cfg_ht_bw >= cmm_ies->ht_op.AddHtInfo.RecomWidth) ||
					(cfg_ht_bw >= cmm_ies->ht_cap.HtCapInfo.ChannelWidth)) {
					if (cmm_ies->ht_op.AddHtInfo.RecomWidth > cmm_ies->ht_cap.HtCapInfo.ChannelWidth) {
						wlan_operate_set_ht_bw(wdev, cmm_ies->ht_cap.HtCapInfo.ChannelWidth, EXTCHA_NONE);
						pEntry->HTPhyMode.field.BW = cmm_ies->ht_cap.HtCapInfo.ChannelWidth;
					} else {
						wlan_operate_set_ht_bw(wdev, pStaCfg->MlmeAux.AddHtInfo.AddHtInfo.RecomWidth,
						pStaCfg->MlmeAux.AddHtInfo.AddHtInfo.ExtChanOffset);
						pEntry->HTPhyMode.field.BW = pStaCfg->MlmeAux.AddHtInfo.AddHtInfo.RecomWidth;
					}
				}

			/* If IE VHT BW is 80, then need to check our prim channel has support for 80 in that region */
			/* 80 MHz operation is prevented in CE and JAP for channel 132~144 */
			if (cfg_vht_bw >= AP_Operting_BW)
				wlan_operate_set_vht_bw(&pStaCfg->wdev, AP_Operting_BW);

			/**
			 * It is seen that as a part of change in BW notification some AP
			 * also changes Rx Mcs as well as NSS
			 */
			if ((pEntry) && HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists)) {
				UCHAR TxStreamIdx = 0, tmp = 0;

				/* Currently we only support for 4 Tx Stream, so check only for 4 Rx Nss */
				bSupportVHTMCS1SS = FALSE;
				bSupportVHTMCS2SS = FALSE;
				bSupportVHTMCS3SS = FALSE;
				bSupportVHTMCS4SS = FALSE;

				SET_VHT_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists);
				NdisMoveMemory(&pStaCfg->MlmeAux.vht_cap, &cmm_ies->vht_cap, SIZE_OF_VHT_CAP_IE);

				tmp = wlan_operate_get_tx_stream(wdev);
				if (tmp > 255)
					return 0;
				for (TxStreamIdx = tmp;
					TxStreamIdx > 0;
					TxStreamIdx--) {
					switch (TxStreamIdx) {
					case 1:
						if (cmm_ies->vht_cap.mcs_set.rx_mcs_map.mcs_ss1 < VHT_MCS_CAP_NA)
							bSupportVHTMCS1SS = TRUE;
						break;

					case 2:
						if (cmm_ies->vht_cap.mcs_set.rx_mcs_map.mcs_ss2 < VHT_MCS_CAP_NA)
							bSupportVHTMCS2SS = TRUE;

						break;

					case 3:
						if (cmm_ies->vht_cap.mcs_set.rx_mcs_map.mcs_ss3 < VHT_MCS_CAP_NA)
							bSupportVHTMCS3SS = TRUE;

						break;

					case 4:
						if (cmm_ies->vht_cap.mcs_set.rx_mcs_map.mcs_ss4 < VHT_MCS_CAP_NA)
							bSupportVHTMCS4SS = TRUE;

						break;

					default:
						break;
					}
				}
			}

			/**
			 * Find RX Mcs using following as mentioned in spec
			 * The Max VHT-MCS For n SS subfield (where n = 1, ..., 8) is
			 *  encoded as follows:
			 * 0 indicates support for VHT-MCS 0-7 for n spatial streams
			 * 1 indicates support for VHT-MCS 0-8 for n spatial streams
			 * 2 indicates support for VHT-MCS 0-9 for n spatial streams
			 * 3 indicates that n spatial streams is not supported
			 * Set to 0 for NSS = 1
			 * Set to 1 for NSS = 2
			 * ...
			 * Set to 7 for NSS = 8
			 */

			if (bSupportVHTMCS4SS == TRUE) {
				if (pEntry->operating_mode.rx_nss != 3)
					pEntry->operating_mode.rx_nss = 3;
				else
					goto end;
			} else if (bSupportVHTMCS3SS == TRUE) {
				if (pEntry->operating_mode.rx_nss != 2)
					pEntry->operating_mode.rx_nss = 2;
				else
					goto end;
			} else if (bSupportVHTMCS2SS == TRUE) {
				if (pEntry->operating_mode.rx_nss != 1)
					pEntry->operating_mode.rx_nss = 1;
				else
					goto end;
			} else {
				if (pEntry->operating_mode.rx_nss != 0)
					pEntry->operating_mode.rx_nss = 0;
				else
					goto end;
			}

			pEntry->force_op_mode = TRUE;
			/* Set to 0 to indicate that the Rx NSS subfield carries the maximum number
			 * of spatial streams that the STA can receive.
			 */
			pEntry->operating_mode.rx_nss_type = 0;



#ifdef BW_VENDOR10_CUSTOM_FEATURE
			/* Sync new HT BW & Ext Channel for Soft AP */
			if (pwdev && (current_operating_bw == 0 && (prev_operating_bw >= 1 || softap_prev_op_bw >= 1) && cmm_ies->ht_op.AddHtInfo.RecomWidth == 0)
				&& IS_SYNC_BW_POLICY_VALID(pAd, TRUE, HT_4020_DOWN_ENBL)) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					" Enter 4020 HT Sync\n");
				bAdjustHTBW = TRUE;
			} else if (pwdev && (current_operating_bw >= 1 && (prev_operating_bw == 0 || softap_prev_op_bw == 0) && cmm_ies->ht_op.AddHtInfo.RecomWidth == 1)
				&& IS_SYNC_BW_POLICY_VALID(pAd, TRUE, HT_2040_UP_ENBL)) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					" Enter 2040 HT Sync\n");
				bAdjustHTBW = TRUE;
			}

			/* Sync new VHT BW & Ext Channel for Soft AP */
			if (pwdev && ((prev_operating_bw == 2 || softap_prev_op_bw == 2) && current_operating_bw <= 1)
				&& IS_SYNC_BW_POLICY_VALID(pAd, FALSE, VHT_80_2040_DOWN_CHK)) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					" Enter 80-2040 VHT Sync\n");
				bAdjustVHTBW = TRUE;
			} else if (pwdev && ((prev_operating_bw <= 1 || softap_prev_op_bw <= 1) && current_operating_bw == 2)
				&& IS_SYNC_BW_POLICY_VALID(pAd, FALSE, VHT_2040_80_UP_CHK)) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					" Enter 2040-80 VHT Sync\n");
				bAdjustVHTBW = TRUE;
			}

			if (bAdjustHTBW)
				wdev_sync_ht_bw(pAd, pwdev, &cmm_ies->ht_op.AddHtInfo);
			if (bAdjustVHTBW)
				wdev_sync_vht_bw(pAd, pwdev, AP_Operting_BW, p80ccf);
#endif
			pStaCfg->MlmeAux.force_op_mode = pEntry->force_op_mode;
			NdisMoveMemory(&pStaCfg->MlmeAux.op_mode, &pEntry->operating_mode, 1);
			NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
			rRaParam.u4Field = RA_PARAM_VHT_OPERATING_MODE;
			RAParamUpdate(pAd, pEntry, &rRaParam);
			bAdjust = TRUE;
			}
		}
	}
#endif /* DOT11_VHT_AC */
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	/* Add check to skip sync, if already done */
	if (!(bAdjustHTBW || bAdjustVHTBW) && IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd) && pwdev)
		/* Soft AP Interface Down Handling */
		CheckSoftAPSyncRequired(pAd, pwdev, cmm_ies);

	if ((bAdjustHTBW || bAdjustVHTBW) && (IS_APCLI_SYNC_PEER_DEAUTH_ENBL(pAd) == FALSE)) {
		if (IS_APCLI_SYNC_PEER_DEAUTH_VALID(pAd) && IsClientConnected(pAd)) {
			SET_APCLI_SYNC_PEER_DEAUTH_ENBL(pAd, TRUE);

			if (IS_APCLI_SYNC_BAND_VALID(pAd, DIFF_BAND_SYNC)) {
				/* Both 2G/5G Bands Soft AP Clients Disconnect */
				APStop(pAd, pMbss, AP_BSS_OPER_ALL);
				APStartUp(pAd, pMbss, AP_BSS_OPER_ALL);
			} else if (IS_APCLI_SYNC_BAND_VALID(pAd, SAME_BAND_SYNC)) {
				/* Same Band SoftAP Clients Disconnect */
				APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
				APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
			}

			SET_APCLI_SYNC_PEER_DEAUTH_ENBL(pAd, FALSE);
		} else {
			/* Soft AP Clients Disconnect Disable Case */
			UpdateBeaconHandler(pAd, pwdev, BCN_REASON(BCN_UPDATE_IE_CHG));
			/* Same Band Client BW Update */
		}
	}
#endif

#endif /* DOT11_N_SUPPORT */
end:
	return bAdjust;
}
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_N_SUPPORT
extern int DetectOverlappingPeriodicRound;

VOID Handle_BSS_Width_Trigger_Events(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	ULONG Now32;
	UCHAR i;
#ifdef DOT11N_DRAFT3

	if ((pAd->CommonCfg.bBssCoexEnable == FALSE) ||
		(pAd->CommonCfg.bOverlapScanning == TRUE))
		return;

#endif /* DOT11N_DRAFT3 */

	if ((Channel <= 14)) {
		for (i = 0; i < WDEV_NUM_MAX; i++) {
			struct wifi_dev *wdev;
			UCHAR ht_bw;
			wdev = pAd->wdev_list[i];

			if (!wdev || (wdev->channel != Channel) || (wdev->wdev_type != WDEV_TYPE_AP))
				continue;

			if (wlan_config_get_ch_band(wdev) != CMD_CH_BAND_24G)
				continue;
			ht_bw = wlan_operate_get_ht_bw(wdev);

			if (ht_bw < HT_BW_40)
				continue;

			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
				"Rcv BSS Width Trigger Event: 40Mhz --> 20Mhz\n");
			NdisGetSystemUpTime(&Now32);
			pAd->CommonCfg.LastRcvBSSWidthTriggerEventsTime = Now32;
			pAd->CommonCfg.bRcvBSSWidthTriggerEvents = TRUE;
			wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
		}

		DetectOverlappingPeriodicRound = 31;
	}
}
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID BuildEffectedChannelList(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev
)
{
	UCHAR		EChannel[11];
	UCHAR		k;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

#endif /* CONFIG_STA_SUPPORT */
	RTMPZeroMemory(EChannel, 11);
	/* 802.11n D4 11.14.3.3: If no secondary channel has been selected, all channels in the frequency band shall be scanned. */
	for (k = 0; k < pChCtrl->ChListNum; k++) {
		if (pChCtrl->ChList[k].Channel <= 14)
			pChCtrl->ChList[k].bEffectedChannel = TRUE;
	}

	return;
}


#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

UCHAR get_channel_bw_cap(struct wifi_dev *wdev, UCHAR channel)
{
	BOOLEAN find = FALSE;
	UCHAR i;
	UCHAR cap = BW_20;
	UINT flag;
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	CHANNEL_CTRL *pChCtrl = NULL;

	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"%s's PAd is null\n", __func__);
			return cap;
	}

	pChCtrl = hc_get_channel_ctrl(ad->hdev_ctrl);

	if (!pChCtrl) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"%s can't find pChCtrl\n", __func__);
		return cap;
	}


	for (i = 0; (i < pChCtrl->ChListNum) && (i < MAX_NUM_OF_CHANNELS); i++) {
		if (channel == pChCtrl->ChList[i].Channel && IsValidChannel(ad, channel, wdev)) {
			find = TRUE;
			flag = pChCtrl->ChList[i].Flags;
			break;
		}
	}

	if (find) {
		if (flag & CHANNEL_320M_CAP)
			cap = BW_320;
		else if (flag & CHANNEL_160M_CAP)
			cap = BW_160;
		else if (flag & CHANNEL_80M_CAP)
			cap = BW_80;
		else if ((flag & CHANNEL_40M_CAP))
			cap = BW_40;
		else
			cap = BW_20;
	}

	return cap;
}

BOOLEAN Is40MHzForbid(CHANNEL_CTRL *pChCtrl)
{
	struct Chan_Config *pChCfg = NULL;

	if (!pChCtrl) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "%s can't find pChCtrl !\n", __func__);
		return FALSE;
	}

	pChCfg = &pChCtrl->ch_cfg;

	return pChCfg->bw40_forbid;
}

#ifdef CONFIG_6G_SUPPORT
VOID add_non_psc_channel(IN PRTMP_ADAPTER pAd, UCHAR channel)
{
	UCHAR i;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (i = 0; i < pChCtrl->ChListNum_6G_psc_scan; i++) {
		if (channel == pChCtrl->ChList_6G_psc_scan[i].Channel)
			return;
	}
	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (channel == pChCtrl->ChList[i].Channel) {
			pChCtrl->ChList_6G_psc_scan[pChCtrl->ChListNum_6G_psc_scan].Channel = channel;
			pChCtrl->ChListNum_6G_psc_scan++;
			return;
		}
	}
}

VOID update_channel_list_by_scantable(IN PRTMP_ADAPTER pAd,
	IN PRTMP_ADAPTER pAd_temp, struct wifi_dev *wdev_temp)
{
	UINT i;
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd_temp, wdev_temp);
	BSS_ENTRY *bss;

	for (i = 0; (i < ScanTab->BssNr) && (ScanTab->BssNr <= MAX_LEN_OF_BSS_TABLE); i++) {
		bss = &ScanTab->BssEntry[i];
		if (bss->rnr_channel != 0)
			add_non_psc_channel(pAd, bss->rnr_channel);
	}
}

VOID build_PSC_scan_channel_list(IN PRTMP_ADAPTER pAd)
{
	UCHAR i;
	UCHAR ch_num = 0;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	struct wifi_dev *temp_wdev;
	struct _RTMP_ADAPTER *pAd_temp;

	/*Build 6G PSC channel list first*/
	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (pChCtrl->ChList[i].PSC_Ch) {
			pChCtrl->ChList_6G_psc_scan[ch_num].Channel = pChCtrl->ChList[i].Channel;
			ch_num++;
		}
	}
	pChCtrl->ChListNum_6G_psc_scan = ch_num;

	/*Add rnr channel to PSC scan channel list*/
	temp_wdev = bss_mngr_query_wdev_by_band(BAND_24G);    /*get 2G rnr*/
	if (temp_wdev) {
		pAd_temp = (struct _RTMP_ADAPTER *)temp_wdev->sys_handle;
		update_channel_list_by_scantable(pAd, pAd_temp, temp_wdev);
	} else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
			"can't find 2g BSS\n");

	temp_wdev = bss_mngr_query_wdev_by_band(BAND_5G);   /*get 5G rnr*/
	if (temp_wdev) {
		pAd_temp = (struct _RTMP_ADAPTER *)temp_wdev->sys_handle;
		update_channel_list_by_scantable(pAd, pAd_temp, temp_wdev);
	} else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
			"can't find 5g BSS\n");
}

#endif
