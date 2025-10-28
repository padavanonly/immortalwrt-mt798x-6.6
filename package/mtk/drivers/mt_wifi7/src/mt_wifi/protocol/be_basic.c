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
/***************************************************************************
***************************************************************************

*/

#include "rt_config.h"
#include "mgmt/be_internal.h"


VOID dump_ht_cap(struct wifi_dev *wdev)
{
	HT_CAPABILITY_IE *ht_cap = NULL;

	if (wdev) {
		ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);

		if (!ht_cap) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_ERROR,
				"ht_cap is null, return!\n");
			return;
		}

		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_WARN,
			"Caller: %pS\n", __builtin_return_address(0));
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
			"\nphy_mode=%d, ch=%d, wdev_type=%d\n",
				wdev->PhyMode, wdev->channel, wdev->wdev_type);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
			"ht_cap: ht_cap->HtCapInfo,\n");
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
		"ldpc=%d,ch_width=%d,gf=%d,sgi20=%d,sgi40=%d,tx_stbc=%d,rx_stbc=%d,amsdu_size=%d\n",
		ht_cap->HtCapInfo.ht_rx_ldpc, ht_cap->HtCapInfo.ChannelWidth,
		ht_cap->HtCapInfo.GF, ht_cap->HtCapInfo.ShortGIfor20, ht_cap->HtCapInfo.ShortGIfor40,
		ht_cap->HtCapInfo.TxSTBC, ht_cap->HtCapInfo.RxSTBC, ht_cap->HtCapInfo.AMsduSize);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
			"ht_cap: ht_cap->HtCapParm,\n");
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
			"mdpu_density=%d, ampdu_factor=%d\n",
			ht_cap->HtCapParm.MpduDensity, ht_cap->HtCapParm.MaxRAmpduFactor);
	}
}

/*init radio*/
static VOID radio_operate_init(struct wifi_dev *wdev)
{
	struct freq_cfg fcfg;
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *ad = NULL;
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	UCHAR BandIdx;
#endif
#ifdef MT_DFS_SUPPORT
	struct DOT11_H *pDot11h = NULL;
#endif

	if (wdev == NULL)
		return;
	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			 "coex(%d), bNeedFallBack(%d).\n",
			  ad->CommonCfg.bBssCoexEnable,
			  ad->CommonCfg.BssCoexScanLastResult.bNeedFallBack);

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	BandIdx = HcGetBandByWdev(wdev);
#endif

	if (CheckNonOccupancyChannel(ad, wdev, RDD_CHECK_NOP_BY_WDEV) == FALSE) {
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		UCHAR grpWidth = 0, channel = 0;
		UCHAR chGrp = DfsV10CheckChnlGrp(ad, wdev->channel);

		if (IS_SUPPORT_V10_DFS(ad) && WMODE_CAP_5G(wdev->PhyMode) && (chGrp >= W53 && chGrp <= W56)) {
			if (chGrp >= W56_UA && chGrp <= W56_UC) {
				chGrp = W56;
				if (ad->CommonCfg.bCh144Enabled)
					grpWidth = V10_W56_SIZE;
				else
					grpWidth = V10_W56_SIZE-1;
			} else if (chGrp == W53)
				grpWidth = V10_W53_SIZE;

		channel = DfsV10FindNonNopChannel(ad, wdev, chGrp, grpWidth);
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"[%s] NOP channel %d grp %d\n", __func__, channel, chGrp);
			if (channel && (DfsV10CheckChnlGrp(ad, channel) == W56_UC &&
				(ad->CommonCfg.bCh144Enabled == FALSE))) {
				AutoChSelUpdateChannel(ad, channel, TRUE, wdev);
				wlan_config_set_ht_bw(wdev, HT_BW_20);
				wlan_config_set_vht_bw(wdev, VHT_BW_2040);
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
					"[%s] Set BW to HT20.\n", __func__);
			}

			if (!channel && chGrp == W53)
				wdev->channel = FirstChannel(ad, wdev);
			else if (!channel && chGrp == W56) {
				DfsV10W56APDownStart(ad, HcGetAutoChCtrl(ad),
					DfsV10W56FindMaxNopDuration(ad), BandIdx);
				SET_V10_APINTF_DOWN(ad, TRUE);
			}
		} else
#endif/* DFS_VENDOR10_CUSTOM_FEATURE */
		wdev->channel = FirstChannel(ad, wdev);
	}
#endif
	os_zero_mem(&fcfg, sizeof(fcfg));
	phy_freq_get_cfg(wdev, &fcfg);

	/*Check ChannelMode before setting channel*/
#ifdef MT_DFS_SUPPORT
	pDot11h = wdev->pDot11_H;
	if (!DfsBypassRadarStateCheck(wdev)) {
		if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G)
			RadarStateCheck(ad, wdev);
		else if (pDot11h)
			pDot11h->ChannelMode = CHAN_NORMAL_MODE;
	}
#endif
#ifdef ZERO_PKT_LOSS_SUPPORT
	ad->chan_switch_time[6] = jiffies_to_msecs(jiffies);
#endif /*ZERO_PKT_LOSS_SUPPORT*/
	operate_loader_phy(wdev, &fcfg);
}


/*
* private structure definition to prevent direct access
*/
VOID wlan_operate_init(struct wifi_dev *wdev)
{
	struct wlan_operate *obj = (struct wlan_operate *)wdev->wpf_op;

	if (!obj) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"Can't find wlan operate for wdev.\n");
		return;
	}

	/*reset to default*/
	phy_oper_init(wdev, &obj->phy_oper);
	ht_oper_init(wdev, &obj->ht_oper);
	ht_op_status_init(wdev, &obj->ht_status);
#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(wdev->PhyMode))
		vht_oper_init(wdev, &obj->vht_oper);
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(wdev->PhyMode))
		he_oper_init(wdev, &obj->he_oper);
#endif

	/*adjust radio operate from configure*/
	radio_operate_init(wdev);

	dump_ht_cap(wdev);

#ifdef BACKGROUND_SCAN_SUPPORT
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	dfs_pre_cac_detect_next_channel(pAd, wdev);
}
#endif
}

VOID wlan_operate_exit(struct wifi_dev *wdev)
{
	struct wlan_operate *obj = (struct wlan_operate *)wdev->wpf_op;

	if (!obj) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"Can't find wlan operate for wdev.\n");
		return;
	}

	phy_oper_exit(&obj->phy_oper);
	ht_oper_exit(&obj->ht_oper);
#ifdef DOT11_VHT_AC
	vht_oper_exit(&obj->vht_oper);
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
	he_oper_exit(&obj->he_oper);
#endif
	ht_op_status_exit(&obj->ht_status);
}

UCHAR wlan_operate_get_state(struct wifi_dev *wdev)
{
	struct wlan_operate *obj = (struct wlan_operate *)wdev->wpf_op;

	if (!obj) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Can't find wlan operate for wdev.\n");
		return WLAN_OPER_STATE_INVALID;
	}

	return obj->state;
}

UCHAR wlan_operate_set_state(struct wifi_dev *wdev, UCHAR state)
{
	struct wlan_operate *obj = (struct wlan_operate *)wdev->wpf_op;

	if (!obj) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Can't find wlan operate for wdev.\n");
		return WLAN_OPER_FAIL;
	}

	obj->state = state;
	return WLAN_OPER_OK;
}

