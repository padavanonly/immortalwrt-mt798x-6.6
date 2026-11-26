#ifdef MTK_LICENSE
/****************************************************************************
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
 ***************************************************************************/
#endif /* MTK_LICENSE */
/****************************************************************************

	Abstract:

	All related CFG80211 FTM Vendor Command Function Body.

	History:

***************************************************************************/

#ifdef RT_CFG80211_SUPPORT

#ifdef FTM_SUPPORT

#include "rt_config.h"


static VOID copy_peer_info(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT8 newInd)
{
	UINT8 status = wdev->FtmCtrl.iSTA_pinfo[newInd].status;

	NdisCopyMemory(&wdev->FtmCtrl.iSTA_pinfo[newInd],
					&wdev->FtmCtrl.iSTA_pinfo[0], sizeof(struct _FTM_PEER_INFO));
	wdev->FtmCtrl.iSTA_pinfo[newInd].status = status;
}

static UINT8 find_peer_num(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev)
{
	UINT8 ind = 0, num = 0, newInd = 0;
	void *pPeerInfoArray;
	struct _FTM_PEER_INFO *pPeerInfo, *pPeerInfoNew;

	pPeerInfoArray = &wdev->FtmCtrl.iSTA_pinfo;
	pPeerInfoNew = (struct _FTM_PEER_INFO *)pPeerInfoArray;

	for (ind = 1; ind < MAX_FTM_TBL_SIZE; ind++) {
		pPeerInfo = (struct _FTM_PEER_INFO *)pPeerInfoArray + ind;
		if (NdisCmpMemory(pPeerInfoNew->Responder, pPeerInfo->Responder, MAC_ADDR_LEN) == 0) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
				"ind=%hhu\n", ind);
			return ind;
		}
	}

	num = wdev->FtmCtrl.LastAssignedISTA + 1;
	for (ind = num; ind < num+MAX_FTM_TBL_SIZE; ind++) {
		newInd = ind % MAX_FTM_TBL_SIZE;
		if (newInd == 0)
			continue;

		pPeerInfo = (struct _FTM_PEER_INFO *)pPeerInfoArray + newInd;
		if (pPeerInfo->status == FTMISTA_UNUSED) {
			wdev->FtmCtrl.LastAssignedISTA = newInd;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
				"newInd=%hhu\n", newInd);
			return newInd;
		}
	}
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"No enough iSTA peerInfo to use.\n");
	wdev->FtmCtrl.LastAssignedISTA = 0;
	return 0;
}


int mtk_cfg80211_vndr_set_ftm_handler(struct wiphy *wiphy,
	struct wireless_dev *wl_dev,
	const void *data, int len)
{
	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_FTM_MAX + 1];

	unsigned char range_req = 0;
	unsigned char ftm_enable = 0, role = 0;
	unsigned char num_burst_exponent = 0, burst_duration = 0, min_delta_ftm = 0;
	unsigned char ptsf_no_perfer = 0, asap = 0;
	unsigned char ftms_per_burst = 0, fmt_and_bw = 0;
	UINT16 ptsf = 0, burst_period = 0;
	UCHAR *target_mac = NULL;
	UCHAR *debug = NULL, *toae_cgf = NULL;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"pPriv is NULL\n");
		return -EFAULT;
	}

	if (!wl_dev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return -EFAULT;
	}

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	net_dev = wl_dev->netdev;
	if (!net_dev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"net_dev is NULL\n");
		return -EFAULT;
	}

	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);

	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"pObj is NULL\n");
		return -EFAULT;
	}

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_FTM_MAX, (struct nlattr *)data,
		len, SUBCMD_FTM_POLICY, NULL);

	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"nla_parse error\n");
		return -EINVAL;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_TOAE_CFG]) {
		toae_cgf = nla_data(tb[MTK_NL80211_VENDOR_ATTR_FTM_TOAE_CFG]);
		mtk_cfg80211_set_ftm_toae_cfg(pAd, wdev, toae_cgf);
		return 0;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_TESTMODE]) {
		ftm_enable = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_FTM_TESTMODE]);
		mtk_cfg80211_set_ftm_test_mode(pAd, wdev, ftm_enable);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_ENABLE]) {
		ftm_enable = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_FTM_ENABLE]);
		mtk_cfg80211_set_ftm_enable(pAd, wdev, ftm_enable);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_ROLE]) {
		role = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_FTM_ROLE]);
		mtk_cfg80211_set_ftm_role(pAd, wdev, role);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_BURST_EXP]) {
		num_burst_exponent = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_FTM_BURST_EXP]);
		mtk_cfg80211_set_ftm_burst_exp(pAd, wdev, num_burst_exponent);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_BURST_DUR]) {
		burst_duration = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_FTM_BURST_DUR]);
		mtk_cfg80211_set_ftm_burst_dur(pAd, wdev, burst_duration);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_MIN_DELTA]) {
		min_delta_ftm = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_FTM_MIN_DELTA]);
		mtk_cfg80211_set_ftm_min_delta(pAd, wdev, min_delta_ftm);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_PARTIAL_TSF]) {
		ptsf = nla_get_u16(tb[MTK_NL80211_VENDOR_ATTR_FTM_PARTIAL_TSF]);
		mtk_cfg80211_set_ftm_ptsf(pAd, wdev, ptsf);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_PTSF_NO_PREFERENCE]) {
		ptsf_no_perfer = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_FTM_PTSF_NO_PREFERENCE]);
		mtk_cfg80211_set_ftm_ptsf_no_perfer(pAd, wdev, ptsf_no_perfer);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_ASAP]) {
		asap = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_FTM_ASAP]);
		mtk_cfg80211_set_ftm_asap(pAd, wdev, asap);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_NUM]) {
		ftms_per_burst = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_FTM_NUM]);
		mtk_cfg80211_set_ftm_num(pAd, wdev, ftms_per_burst);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_FMT_AND_BW]) {
		fmt_and_bw = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_FTM_FMT_AND_BW]);
		if (mtk_cfg80211_set_fmt_and_bw(pAd, wdev, fmt_and_bw) == -EINVAL) {
			mtk_cfg80211_dump_ftm_parm(pAd, wdev);
			return 0;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_BURST_PERIOD]) {
		burst_period = nla_get_u16(tb[MTK_NL80211_VENDOR_ATTR_FTM_BURST_PERIOD]);
		mtk_cfg80211_set_ftm_burst_period(pAd, wdev, burst_period);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_TARGET_MAC]) {
		target_mac = nla_data(tb[MTK_NL80211_VENDOR_ATTR_FTM_TARGET_MAC]);
		mtk_cfg80211_set_ftm_target_mac(pAd, wdev, target_mac);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_DEBUG]) {
		debug = nla_data(tb[MTK_NL80211_VENDOR_ATTR_FTM_DEBUG]);
		mtk_cfg80211_set_ftm_debug(pAd, wdev, debug);
		return 0;
	}

	mtk_cfg80211_dump_ftm_parm(pAd, wdev);

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_RANG_REQ]) {
		if (wdev->FtmCtrl.FTMInit == false) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
				"haven't enable FTM initiator: mwctl dev <devname> set ftm enable=2\n");
			return 0;
		}
		if (wdev->FtmCtrl.iSTA_pinfo[0].preamble == LOC_PREAMBLE_INVALID ||
			wdev->FtmCtrl.iSTA_pinfo[0].bandwidth == LOC_MEAS_BW_INVALID) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
				"bandwidth or preamble is invalid\n");
			return 0;
		}

		range_req = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_FTM_RANG_REQ]);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_NOTICE,
			"range_req=%d\n", range_req);

		if (range_req == 0 || range_req == 1) {
			struct _FTM_PEER_INFO PeerInfo;
			UINT8 *pAddr;
			UINT8 peer_num = find_peer_num(pAd, wdev);

			wdev->FtmCtrl.LastAssignedISTA = peer_num;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
					"peer_num=%hhu\n", peer_num);

			copy_peer_info(pAd, wdev, peer_num);
			PeerInfo = wdev->FtmCtrl.iSTA_pinfo[peer_num];

			if (wdev->wdev_type == WDEV_TYPE_STA) {
				PSTA_ADMIN_CONFIG pStaCfg = NULL;

				pStaCfg = GetStaCfgByWdev(pAd, wdev);

				if (!INFRA_ON(pStaCfg) || RTMPCompareMemory(pStaCfg->Bssid, PeerInfo.Responder, MAC_ADDR_LEN) != 0) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
						"APCLI only support with connected peer\n");
					return 0;
				}
			}

			if (PeerInfo.status == FTMISTA_UNUSED) {
				if (wdev->FtmCtrl.ista_work_cand == CFG_LOC_ISTA_MAX_CANDIDATES) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
						"working iSTA is MAX.\n");
					return 0;
				}
				wdev->FtmCtrl.ista_work_cand += 1;
			}
			wdev->FtmCtrl.iSTA_pinfo[peer_num].status = FTMISTA_FIRST_BURST;

			pAddr = PeerInfo.Responder;
			ftm_create_mac_entry(pAd, wdev, pAddr, &wdev->FtmCtrl.iSTA_pinfo[peer_num]);

			mtk_cfg80211_range_req_mc(pAd, wdev, range_req, peer_num);
		}
	}

	return 0;
}

VOID mtk_cfg80211_set_ftm_toae_cfg(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR *toae_cfg)
{
	int matches;
	struct ftm_toae_cfg *p_toae_cfg;
	UINT8 band_idx;

	p_toae_cfg = &wdev->FtmCtrl.toae_cfg;
	matches = sscanf(toae_cfg, "%hhu-%hhu-%hhu-%hhu-%hu",
		&p_toae_cfg->fgBiasTuning, &p_toae_cfg->u1AntIdx, &p_toae_cfg->u1SpeIdxFtm,
		&p_toae_cfg->u1SpeIdxFtmAck, &p_toae_cfg->u2ChainSel);

	if (matches != 5) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
				"wrong format of toae_cfg\n");
		return;
	}

	band_idx = hc_get_hw_band_idx(pAd);
	UniCmdFTM(pAd, wdev, UNI_CMD_LOC_TAG_TOAE_CAL_CFG, 0, band_idx, NULL);
}

VOID mtk_cfg80211_range_req_mc_after_scan(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev)
{
	UINT8 band_idx;
	BSS_TABLE *scan_tab;
	ULONG bss_idx;
	UINT8 peer_num = wdev->FtmCtrl.LastAssignedISTA;
	struct _FTM_PEER_INFO *pPeerInfo;

	pPeerInfo = &wdev->FtmCtrl.iSTA_pinfo[peer_num];

	/* get target channel */
	scan_tab = get_scan_tab_by_wdev(pAd, wdev);
	if (scan_tab == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
					"scan_tab is null(%p).\n", scan_tab);
		return;
	}

	bss_idx = BssSsidTableSearchByBSSID(scan_tab, pPeerInfo->Responder);
	if (bss_idx < MAX_LEN_OF_BSS_TABLE) {
		pPeerInfo->target_channel = scan_tab->BssEntry[bss_idx].Channel;
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_NOTICE,
				"ori_channel=%hhu, target_channel=%hhu\n",
				pPeerInfo->ori_channel, pPeerInfo->target_channel);
		FtmSwitchChannel(pAd, wdev, pPeerInfo->target_channel, ACTION_GO_TO);
		return;
	}

	pPeerInfo->target_channel = pPeerInfo->ori_channel;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_NOTICE,
				"ori_channel=%hhu, target_channel=%hhu\n",
				pPeerInfo->ori_channel, pPeerInfo->target_channel);

	band_idx = hc_get_hw_band_idx(pAd);
	UniCmdFTM(pAd, wdev, UNI_CMD_LOC_TAG_RANGE_REQ_MC, pPeerInfo->trigger_mc_req, band_idx, NULL);
}

VOID mtk_cfg80211_range_req_mc(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	INT enable,
	UINT8 peer_num)
{
	UINT8 band_idx;
	struct _FTM_PEER_INFO *pPeerInfo;
	BSS_TABLE *scan_tab;
	ULONG bss_idx;

	pPeerInfo = &wdev->FtmCtrl.iSTA_pinfo[peer_num];

	/* reset distance */
	pPeerInfo->distanceMM = 0;

	/* restore origin channel */
	pPeerInfo->ori_channel = wdev->channel;

	/* get target channel */
	scan_tab = get_scan_tab_by_wdev(pAd, wdev);
	if (scan_tab == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
					"scan_tab is null(%p).\n", scan_tab);
		return;
	}

	bss_idx = BssSsidTableSearchByBSSID(scan_tab, pPeerInfo->Responder);
	if (bss_idx < MAX_LEN_OF_BSS_TABLE) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
					"scan_tab bss_idx=%ld < MAX_LEN_OF_BSS_TABLE\n", bss_idx);
		pPeerInfo->target_channel = scan_tab->BssEntry[bss_idx].Channel;
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_NOTICE,
				"ori_channel=%hhu, target_channel=%hhu\n",
				pPeerInfo->ori_channel, pPeerInfo->target_channel);
		FtmSwitchChannel(pAd, wdev, pPeerInfo->target_channel, ACTION_GO_TO);
		return;
	} else if (pPeerInfo->create_STAREC != 1) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
					"exist STAREC, but not in scan table, might be STA.\n");
		pPeerInfo->target_channel = wdev->channel;
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
					"bss_idx=%ld >= MAX_LEN_OF_BSS_TABLE\n", bss_idx);
		wdev->FtmCtrl.trigger_scan = 1;
		pPeerInfo->trigger_mc_req = enable;
		Set_PartialScan_Proc(pAd, "1");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_NOTICE,
				"ori_channel=%hhu, target_channel=%hhu\n",
				pPeerInfo->ori_channel, pPeerInfo->target_channel);

	band_idx = hc_get_hw_band_idx(pAd);
	UniCmdFTM(pAd, wdev, UNI_CMD_LOC_TAG_RANGE_REQ_MC, enable, band_idx, NULL);
}

VOID mtk_cfg80211_dump_ftm_parm(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev)
{
	FTM_CTRL ftmCtrl = wdev->FtmCtrl;
	struct _FTM_PEER_INFO PeerInfo;
	UINT8 *pAddr;
	UINT8 peer_num = 0;

	if (wdev->FtmCtrl.FTMRole == 1)
		PeerInfo = wdev->FtmCtrl.rSTA_pinfo[peer_num];
	else if (wdev->FtmCtrl.FTMRole == 2)
		PeerInfo = wdev->FtmCtrl.iSTA_pinfo[peer_num];
	else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"FTMRole=%hhu is invalid\n", wdev->FtmCtrl.FTMRole);
		return;
	}

	pAddr = PeerInfo.Responder;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"Enable Role (FTM Resp=%d, FTM Init=%d)\n",
			ftmCtrl.FTMResp, ftmCtrl.FTMInit);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"Role Params: %d (1 for Responder, 2 for Initiator)\n", ftmCtrl.FTMRole);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"burst_exp=%d burst_dur=%d min_delta=%d ptsf=%d no_perfer=%d\n",
			PeerInfo.num_burst_exponent, PeerInfo.burst_duration, PeerInfo.min_delta_ftm,
			PeerInfo.partial_tsf, PeerInfo.ptsf_no_perference);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"asap=%d ftm_num=%d burst_period=%d\n",
			PeerInfo.asap, PeerInfo.ftms_per_burst, PeerInfo.burst_period);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"fmt_bw=%d preamble=%hhu bandwidth=%hhu\n",
			PeerInfo.fmt_and_bw, PeerInfo.preamble, PeerInfo.bandwidth);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"target=%02x:%02x:%02x:%02x:%02x:%02x\n",
			pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"status=%hhd, working iSTA=%hhd\n", PeerInfo.status, ftmCtrl.ista_work_cand);
}

VOID mtk_cfg80211_set_ftm_test_mode(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	INT enable)
{
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	UniCmdFTM(pAd, wdev, UNI_CMD_LOC_TAG_RSP_ENABLE, 3, band_idx, NULL);
}


VOID mtk_cfg80211_set_ftm_enable(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	INT ftm_enable)
{
	UINT8 band_idx = hc_get_hw_band_idx(pAd);
	USHORT wmode = wdev->PhyMode;

	/* Disable FTM
	 * if 2G in legacy mode
	 * if 5G in legacy, HT mode */
	if (wmode == WMODE_INVALID) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"wmode is WMODE_INVALID, FTM disable\n");
		ftm_enable = 0;
	}
#ifdef DOT11_EHT_BE
	else if (WMODE_CAP_BE(wmode))
		;
#endif /* DOT11_EHT_BE */
#ifdef DOT11_HE_AX
	else if (WMODE_CAP_AX(wmode))
		;
#endif /* DOT11_HE_AX */
#ifdef DOT11_VHT_AC
	else if (WMODE_CAP_AC(wmode))
		;
#endif /* DOT11_VHT_AC */
#ifdef DOT11_N_SUPPORT
	else if (WMODE_CAP_N(wmode) && band_idx == BAND1) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"not support FTM in 5/6G HT mode, FTM disable\n");
		ftm_enable = 0;
	}
#endif
	else if (wmode & (WMODE_A|WMODE_B|WMODE_G)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"not support FTM in Legacy mode, FTM disable\n");
		ftm_enable = 0;
	}

	/* Get Capability */
	UniCmdFTM(pAd, wdev, UNI_CMD_LOC_TAG_GET_CAPA, 0, band_idx, NULL);

	wdev->FtmCtrl.FTMResp = (ftm_enable >> 0) & 0x1;
	wdev->FtmCtrl.FTMInit = (ftm_enable >> 1) & 0x1;

	if (ftm_enable == 0)
		UniCmdFTM(pAd, wdev, UNI_CMD_LOC_TAG_RSP_ENABLE, 1, band_idx, NULL);
	else {
		if (wdev->FtmCtrl.FTMResp)
			UniCmdFTM(pAd, wdev, UNI_CMD_LOC_TAG_RSP_ENABLE, 1, band_idx, NULL);
		if (wdev->FtmCtrl.FTMInit)
			UniCmdFTM(pAd, wdev, UNI_CMD_LOC_TAG_RSP_ENABLE, 2, band_idx, NULL);
	}
}

VOID mtk_cfg80211_set_ftm_role(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	INT role)
{
	if (role < 1 || role > 2) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"value is invalid\n");
		return;
	}

	wdev->FtmCtrl.FTMRole = role;
}


VOID mtk_cfg80211_set_ftm_burst_exp(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	INT burst_exp)
{
	struct _FTM_PEER_INFO *pPeerInfo;
	UINT8 peer_num = 0;

	if (wdev->FtmCtrl.FTMRole == 1) {
		peer_num = wdev->FtmCtrl.LastAssignedRSTA;
		pPeerInfo = &wdev->FtmCtrl.rSTA_pinfo[peer_num];
	} else if (wdev->FtmCtrl.FTMRole == 2) {
		peer_num = 0;
		pPeerInfo = &wdev->FtmCtrl.iSTA_pinfo[peer_num];
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"FTMRole=%hhu is invalid\n", wdev->FtmCtrl.FTMRole);
		return;
	}

	if (burst_exp < 0 || burst_exp > 15) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"value is invalid\n");
		return;
	}

	pPeerInfo->num_burst_exponent = burst_exp;
}

VOID mtk_cfg80211_set_ftm_burst_dur(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	INT burst_dur)
{
	struct _FTM_PEER_INFO *pPeerInfo;
	UINT8 peer_num = 0;

	if (wdev->FtmCtrl.FTMRole == 1) {
		peer_num = wdev->FtmCtrl.LastAssignedRSTA;
		pPeerInfo = &wdev->FtmCtrl.rSTA_pinfo[peer_num];
	} else if (wdev->FtmCtrl.FTMRole == 2) {
		peer_num = 0;
		pPeerInfo = &wdev->FtmCtrl.iSTA_pinfo[peer_num];
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"FTMRole=%hhu is invalid\n", wdev->FtmCtrl.FTMRole);
		return;
	}

	if (burst_dur < 0 || burst_dur > 15) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"value is invalid\n");
		return;
	} else if ((burst_dur >= 0 && burst_dur <= 1) ||
				(burst_dur >= 12 && burst_dur <= 14)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"burst duration=%d is reversed\n", burst_dur);
		return;
	}

	pPeerInfo->burst_duration = burst_dur;
}

VOID mtk_cfg80211_update_ftm_min_delta(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev)
{
	struct _FTM_PEER_INFO *pPeerInfo;
	UINT8 peer_num = 0;
	UINT8 min_delta = 0;

	// rSTA
	pPeerInfo = &wdev->FtmCtrl.rSTA_pinfo[peer_num];
	min_delta = pPeerInfo->min_delta_ftm;
	if (min_delta < wdev->FtmCtrl.loc_cap.u4MinDeltaTimePerPacket) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
			"FW support min delta time should >= %d\n",
			wdev->FtmCtrl.loc_cap.u4MinDeltaTimePerPacket);
		min_delta = wdev->FtmCtrl.loc_cap.u4MinDeltaTimePerPacket;
		pPeerInfo->min_delta_ftm = min_delta;
	}

	// iSTA
	pPeerInfo = &wdev->FtmCtrl.iSTA_pinfo[peer_num];
	min_delta = pPeerInfo->min_delta_ftm;
	if (min_delta < wdev->FtmCtrl.loc_cap.u4MinDeltaTimePerPacket) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
			"FW support min delta time should >= %d\n",
			wdev->FtmCtrl.loc_cap.u4MinDeltaTimePerPacket);
		min_delta = wdev->FtmCtrl.loc_cap.u4MinDeltaTimePerPacket;
		pPeerInfo->min_delta_ftm = min_delta;
	}
}

VOID mtk_cfg80211_set_ftm_min_delta(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	INT min_delta)
{
	struct _FTM_PEER_INFO *pPeerInfo;
	UINT8 peer_num = 0;

	if (wdev->FtmCtrl.FTMRole == FTMROLE_RESP) {
		peer_num = wdev->FtmCtrl.LastAssignedRSTA;
		pPeerInfo = &wdev->FtmCtrl.rSTA_pinfo[peer_num];
	} else if (wdev->FtmCtrl.FTMRole == FTMROLE_INIT) {
		peer_num = 0;
		pPeerInfo = &wdev->FtmCtrl.iSTA_pinfo[peer_num];
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"FTMRole=%hhu is invalid\n", wdev->FtmCtrl.FTMRole);
		return;
	}

	if (min_delta < 1 || min_delta > 255) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"value is invalid\n");
		return;
	}

	if (min_delta < wdev->FtmCtrl.loc_cap.u4MinDeltaTimePerPacket) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
			"FW support min delta time should >= %d\n",
			wdev->FtmCtrl.loc_cap.u4MinDeltaTimePerPacket);
		min_delta = wdev->FtmCtrl.loc_cap.u4MinDeltaTimePerPacket;
	}

	pPeerInfo->min_delta_ftm = min_delta;
}


VOID mtk_cfg80211_set_ftm_ptsf(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	INT ptsf)
{
	struct _FTM_PEER_INFO *pPeerInfo;
	UINT8 peer_num = 0;

	if (wdev->FtmCtrl.FTMRole == 1) {
		peer_num = wdev->FtmCtrl.LastAssignedRSTA;
		pPeerInfo = &wdev->FtmCtrl.rSTA_pinfo[peer_num];
	} else if (wdev->FtmCtrl.FTMRole == 2) {
		peer_num = 0;
		pPeerInfo = &wdev->FtmCtrl.iSTA_pinfo[peer_num];
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"FTMRole=%hhu is invalid\n", wdev->FtmCtrl.FTMRole);
		return;
	}

	if (ptsf < 0 || ptsf > 65535) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"value is invalid\n");
		return;
	}

	pPeerInfo->partial_tsf = ptsf;
}


VOID mtk_cfg80211_set_ftm_ptsf_no_perfer(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	INT ptsf_no_perfer)
{
	struct _FTM_PEER_INFO *pPeerInfo;
	UINT8 peer_num = 0;

	if (wdev->FtmCtrl.FTMRole == 1) {
		peer_num = wdev->FtmCtrl.LastAssignedRSTA;
		pPeerInfo = &wdev->FtmCtrl.rSTA_pinfo[peer_num];
	} else if (wdev->FtmCtrl.FTMRole == 2) {
		peer_num = 0;
		pPeerInfo = &wdev->FtmCtrl.iSTA_pinfo[peer_num];
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"FTMRole=%hhu is invalid\n", wdev->FtmCtrl.FTMRole);
		return;
	}

	if (ptsf_no_perfer < 0 || ptsf_no_perfer > 1) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"value is invalid\n");
		return;
	}

	pPeerInfo->ptsf_no_perference = ptsf_no_perfer;
}


VOID mtk_cfg80211_set_ftm_asap(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	INT asap)
{
	struct _FTM_PEER_INFO *pPeerInfo;
	UINT8 peer_num = 0;

	if (wdev->FtmCtrl.FTMRole == 1) {
		peer_num = wdev->FtmCtrl.LastAssignedRSTA;
		pPeerInfo = &wdev->FtmCtrl.rSTA_pinfo[peer_num];
	} else if (wdev->FtmCtrl.FTMRole == 2) {
		peer_num = 0;
		pPeerInfo = &wdev->FtmCtrl.iSTA_pinfo[peer_num];
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"FTMRole=%hhu is invalid\n", wdev->FtmCtrl.FTMRole);
		return;
	}

	if (asap < 0 || asap > 1) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"value is invalid\n");
		return;
	}

	pPeerInfo->asap = asap;
}


VOID mtk_cfg80211_set_ftm_num(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	INT ftms_per_burst)
{
	struct _FTM_PEER_INFO *pPeerInfo;
	UINT8 peer_num = 0;

	if (wdev->FtmCtrl.FTMRole == 1) {
		peer_num = wdev->FtmCtrl.LastAssignedRSTA;
		pPeerInfo = &wdev->FtmCtrl.rSTA_pinfo[peer_num];
	} else if (wdev->FtmCtrl.FTMRole == 2) {
		peer_num = 0;
		pPeerInfo = &wdev->FtmCtrl.iSTA_pinfo[peer_num];
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"FTMRole=%hhu is invalid\n", wdev->FtmCtrl.FTMRole);
		return;
	}

	if (ftms_per_burst < 0 || ftms_per_burst == 1 || ftms_per_burst > 31) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"value is invalid\n");
		return;
	}

	pPeerInfo->ftms_per_burst = ftms_per_burst;
}


static struct _FTM_FMT_MAP ftmFmtMap[] = {
	{FTM_BW_NO_PREFERENCE, LOC_PREAMBLE_INVALID, LOC_MEAS_BW_20},
	{FTM_BW_NONHT_BW5, LOC_PREAMBLE_LEGACY, LOC_MEAS_BW_5},
	{FTM_BW_NONHT_BW10, LOC_PREAMBLE_LEGACY, LOC_MEAS_BW_10},
	{FTM_BW_NONHT_BW20, LOC_PREAMBLE_LEGACY, LOC_MEAS_BW_20},
	{FTM_BW_HT_BW20, LOC_PREAMBLE_HT, LOC_MEAS_BW_20},
	{FTM_BW_VHT_BW20, LOC_PREAMBLE_VHT, LOC_MEAS_BW_20},
	{FTM_BW_HT_BW40, LOC_PREAMBLE_HT, LOC_MEAS_BW_40},
	{FTM_BW_VHT_BW40, LOC_PREAMBLE_VHT, LOC_MEAS_BW_40},
	{FTM_BW_VHT_BW80, LOC_PREAMBLE_VHT, LOC_MEAS_BW_80},
	{FTM_BW_VHT_BW80_80, LOC_PREAMBLE_VHT, LOC_MEAS_BW_160},
	{FTM_BW_VHT_BW160_2RFLO, LOC_PREAMBLE_VHT, LOC_MEAS_BW_160},
	{FTM_BW_VHT_BW160_1RFLO, LOC_PREAMBLE_VHT, LOC_MEAS_BW_160},
	{FTM_BW_DMG_BW2160, LOC_PREAMBLE_INVALID, LOC_MEAS_BW_20},
};


static INT LocGetChannelWidth(
	struct wifi_dev *wdev,
	UINT_8 preamble)
{
	USHORT wmode = wdev->PhyMode;
	UCHAR bw = wlan_operate_get_bw(wdev);
	int chWidth;

	// mapping
	switch (bw) {
	case BW_20:
		chWidth = WIFI_CHAN_WIDTH_20;
		break;
	case BW_40:
		chWidth = WIFI_CHAN_WIDTH_40;
		break;
	case BW_80:
		chWidth = WIFI_CHAN_WIDTH_80;
		break;
	case BW_160:
		chWidth = WIFI_CHAN_WIDTH_160;
		break;
	case BW_10:
		return WIFI_CHAN_WIDTH_10;
	case BW_5:
		return WIFI_CHAN_WIDTH_5;
	case BW_8080:
		chWidth = WIFI_CHAN_WIDTH_80P80;
		break;
	default:
		chWidth = WIFI_CHAN_WIDTH_INVALID;
		break;
	}

	if (wmode == WMODE_INVALID)
		return WIFI_CHAN_WIDTH_INVALID;
#ifdef DOT11_EHT_BE
	else if (WMODE_CAP_BE(wmode))
		return chWidth;
#endif /* DOT11_EHT_BE */
#ifdef DOT11_HE_AX
	else if (WMODE_CAP_AX(wmode)) {
		if (preamble & LOC_PREAMBLE_EHT)
			return WIFI_CHAN_WIDTH_INVALID;
		return chWidth;
	}
#endif /* DOT11_HE_AX */
#ifdef DOT11_VHT_AC
	else if (WMODE_CAP_AC(wmode)) {
		if (preamble & (LOC_PREAMBLE_EHT|LOC_PREAMBLE_HE))
			return WIFI_CHAN_WIDTH_INVALID;
		return chWidth;
	}
#endif /* DOT11_VHT_AC */
#ifdef DOT11_N_SUPPORT
	else if (WMODE_CAP_N(wmode)) {
		if (preamble & (LOC_PREAMBLE_EHT|LOC_PREAMBLE_HE|LOC_PREAMBLE_VHT))
			return WIFI_CHAN_WIDTH_INVALID;
		return chWidth;
	}
#endif
	else if (wmode & (WMODE_A|WMODE_B|WMODE_G)) {
		if (preamble & (LOC_PREAMBLE_EHT|LOC_PREAMBLE_HE|LOC_PREAMBLE_VHT|LOC_PREAMBLE_HT))
			return WIFI_CHAN_WIDTH_INVALID;
		return chWidth;
	}

	return WIFI_CHAN_WIDTH_INVALID;
}

static INT ftm_check_format_and_bw(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	struct _FTM_PEER_INFO *pPeerInfo)
{
	USHORT wmode = wdev->PhyMode;
	INT8 channelWidth;
	UCHAR *temp_wmode = NULL;

	temp_wmode = wmode_2_str(wmode);
	if (temp_wmode == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR, "wmode is NULL\n");
		return -EINVAL;
	}

	channelWidth = LocGetChannelWidth(wdev, pPeerInfo->preamble);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
		"channelWidth = %hhd, wmode: %s\n", channelWidth, temp_wmode);
	os_free_mem(temp_wmode);
	temp_wmode = NULL;

	if (channelWidth == WIFI_CHAN_WIDTH_INVALID) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"pPeerInfo->preamble exceed AP capability\n");
		return -EINVAL;
	}

	switch (channelWidth) {
	case WIFI_CHAN_WIDTH_5:
	{
		if (pPeerInfo->bandwidth & LOC_MEAS_BW_5)
			break;

		pPeerInfo->bandwidth = LOC_MEAS_BW_INVALID;
		return -EINVAL;
	}
	case WIFI_CHAN_WIDTH_10:
	{
		if (pPeerInfo->bandwidth & (LOC_MEAS_BW_5|LOC_MEAS_BW_10))
			break;

		pPeerInfo->bandwidth = LOC_MEAS_BW_INVALID;
		return -EINVAL;
	}
	case WIFI_CHAN_WIDTH_20:
	{
		if (pPeerInfo->bandwidth & (LOC_MEAS_BW_5|LOC_MEAS_BW_10|LOC_MEAS_BW_20))
			break;

		pPeerInfo->bandwidth = LOC_MEAS_BW_INVALID;
		return -EINVAL;
	}
	case WIFI_CHAN_WIDTH_40:
	{
		if (pPeerInfo->bandwidth & (LOC_MEAS_BW_5|LOC_MEAS_BW_10|LOC_MEAS_BW_20|LOC_MEAS_BW_40))
			break;

		pPeerInfo->bandwidth = LOC_MEAS_BW_INVALID;
		return -EINVAL;
	}
	case WIFI_CHAN_WIDTH_80:
	{
		if (pPeerInfo->bandwidth & (LOC_MEAS_BW_5|LOC_MEAS_BW_10|LOC_MEAS_BW_20|LOC_MEAS_BW_40|LOC_MEAS_BW_80))
			break;

		pPeerInfo->bandwidth = LOC_MEAS_BW_INVALID;
		return -EINVAL;
	}
	case WIFI_CHAN_WIDTH_160:
	case WIFI_CHAN_WIDTH_80P80:
	default:
		break;
	}

	return 0;
}

INT mtk_cfg80211_set_fmt_and_bw(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	INT fmt_and_bw)
{
	struct _FTM_PEER_INFO *pPeerInfo;
	UINT8 peer_num = 0;
	int x;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	if (wdev->FtmCtrl.FTMRole == 1) {
		peer_num = wdev->FtmCtrl.LastAssignedRSTA;
		pPeerInfo = &wdev->FtmCtrl.rSTA_pinfo[peer_num];
	} else if (wdev->FtmCtrl.FTMRole == 2) {
		peer_num = 0;
		pPeerInfo = &wdev->FtmCtrl.iSTA_pinfo[peer_num];
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"FTMRole=%hhu is invalid\n", wdev->FtmCtrl.FTMRole);
		return -EINVAL;
	}

	if (fmt_and_bw < 0 || fmt_and_bw > 63) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"value is invalid\n");
		return -EINVAL;
	}

	pPeerInfo->fmt_and_bw = fmt_and_bw;

	// mapping
	pPeerInfo->preamble = LOC_PREAMBLE_INVALID;
	pPeerInfo->bandwidth = LOC_MEAS_BW_20;
	for (x = 0; x < ARRAY_SIZE(ftmFmtMap); x++) {
		if (pPeerInfo->fmt_and_bw == ftmFmtMap[x].ftm_fmt_and_bw) {
			pPeerInfo->preamble = ftmFmtMap[x].loc_cfg_preamble;
			pPeerInfo->bandwidth = ftmFmtMap[x].loc_cfg_bandwidth;
			break;
		}
	}

	if (ftm_check_format_and_bw(pAd, wdev, pPeerInfo) == -EINVAL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"ftm_check_format_and_bw is -EINVAL\n");
		return -EINVAL;
	}


	if ((pPeerInfo->preamble == LOC_PREAMBLE_HT) && band_idx == BAND1) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"not support FTM in 5/6G HT mode\n");
		pPeerInfo->preamble = LOC_PREAMBLE_INVALID;
		return -EINVAL;
	} else if (pPeerInfo->preamble == LOC_PREAMBLE_LEGACY) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"not support FTM in Legacy mode\n");
		pPeerInfo->preamble = LOC_PREAMBLE_INVALID;
		return -EINVAL;
	}

	return 0;
}


VOID mtk_cfg80211_set_ftm_burst_period(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	INT burst_period)
{
	struct _FTM_PEER_INFO *pPeerInfo;
	UINT8 peer_num = 0;

	if (wdev->FtmCtrl.FTMRole == 1) {
		peer_num = wdev->FtmCtrl.LastAssignedRSTA;
		pPeerInfo = &wdev->FtmCtrl.rSTA_pinfo[peer_num];
	} else if (wdev->FtmCtrl.FTMRole == 2) {
		peer_num = 0;
		pPeerInfo = &wdev->FtmCtrl.iSTA_pinfo[peer_num];
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"FTMRole=%hhu is invalid\n", wdev->FtmCtrl.FTMRole);
		return;
	}

	if (burst_period < 0 || burst_period > 65535) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"value is invalid\n");
		return;
	}

	pPeerInfo->burst_period = burst_period;
}

VOID mtk_cfg80211_set_ftm_target_mac(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT8 *macAddr)
{
	struct _FTM_PEER_INFO *pPeerInfo;
	UINT8 peer_num = 0;

	if (wdev->FtmCtrl.FTMRole == 1) {
		peer_num = wdev->FtmCtrl.LastAssignedRSTA;
		pPeerInfo = &wdev->FtmCtrl.rSTA_pinfo[peer_num];
	} else if (wdev->FtmCtrl.FTMRole == 2) {
		peer_num = 0;
		pPeerInfo = &wdev->FtmCtrl.iSTA_pinfo[peer_num];
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"FTMRole=%hhu is invalid\n", wdev->FtmCtrl.FTMRole);
		return;
	}

	RTMPMoveMemory(pPeerInfo->Responder, macAddr, MAC_ADDR_LEN);
}


VOID mtk_cfg80211_set_ftm_debug(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR *macAddr)
{
	struct _FTM_PEER_INFO *pPeerInfo;
	UINT8 peer_num = 0;
	int matches;
	UINT8 band_idx;

	if (wdev->FtmCtrl.FTMRole == 1) {
		peer_num = wdev->FtmCtrl.LastAssignedRSTA;
		pPeerInfo = &wdev->FtmCtrl.rSTA_pinfo[peer_num];
	} else if (wdev->FtmCtrl.FTMRole == 2) {
		peer_num = wdev->FtmCtrl.LastAssignedISTA;
		pPeerInfo = &wdev->FtmCtrl.iSTA_pinfo[peer_num];
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"FTMRole=%hhu is invalid\n", wdev->FtmCtrl.FTMRole);
		return;
	}


	matches = sscanf(macAddr, "%u:%u:%u:%u:%u:%u",
		&pPeerInfo->debug[0], &pPeerInfo->debug[1], &pPeerInfo->debug[2],
		&pPeerInfo->debug[3], &pPeerInfo->debug[4], &pPeerInfo->debug[5]);

	if (matches != 6) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"Format Error! Please enter in the following format\n"
			"mwctl dev <inf> set ftm debug=u:u:u:u:u:u\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"debug[0]=%u, debug[1]=%u, debug[2]=%u, debug[3]=%u, debug[4]=%u, debug[5]=%u\n",
			pPeerInfo->debug[0], pPeerInfo->debug[1], pPeerInfo->debug[2],
			pPeerInfo->debug[3], pPeerInfo->debug[4], pPeerInfo->debug[5]);

	band_idx = hc_get_hw_band_idx(pAd);
	UniCmdFTM(pAd, wdev, UNI_CMD_LOC_TAG_DEBUG, 1, band_idx, NULL);
}


int mtk_cfg80211_vndr_get_ftm_stat_handler(struct wiphy *wiphy,
						 struct wireless_dev *wl_dev,
						 const void *data, int len)
{
	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;
	POS_COOKIE pObj;
	ULONG priv_flags;
	RTMP_STRING *msg;
	UINT max_len = 2048;
	INT32 msg_len = 0;
	struct wifi_dev *wdev;
	int ret;
	UCHAR value;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_BSS_STATS_ATTR_MAX + 1];


	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"pPriv is NULL\n");
		return -EFAULT;
	}

	if (!wl_dev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"wl_dev is NULL\n");
		return -EFAULT;
	}

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	if (!wl_dev->netdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"wl_dev->netdev is NULL\n");
		return -EFAULT;
	}

	priv_flags = RT_DEV_PRIV_FLAGS_GET(wl_dev->netdev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, wl_dev->netdev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"pObj is NULL\n");
		return -EFAULT;
	}

	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	if (!wdev)
		return -EINVAL;

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_FTM_STAT_MAX, (struct nlattr *)data,
		len, SUBCMD_FTM_STAT_POLICY, NULL);

	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"nla_parse error\n");
		return -EINVAL;
	}

	os_alloc_mem(pAd, (UCHAR **)&msg, max_len);

	if (msg == NULL)
		return -ENOMEM;

	memset(msg, 0x00, max_len);

	if (tb[MTK_NL80211_VENDOR_ATTR_FTM_STAT_STR]) {
		value = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_FTM_STAT_STR]);
		if (value == 1 || value == 2)
			msg_len = mtk_cfg80211_vndr_get_ftm_stat_iSTA(pAd, pObj, msg, max_len, value);
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"no MTK_NL80211_VENDOR_ATTR_FTM_STAT_STR attr\n");
		if (msg)
			os_free_mem(msg);
		return -EINVAL;
	}

	if (msg_len < 0) {
		ret = -EFAULT;
		goto error;
	} else if (msg_len == 0) {
		ret = snprintf(msg, max_len, "===>Error Statistics string!");
		if (os_snprintf_error(max_len, ret)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
				"msg snprintf error!\n");
			ret = -EINVAL;
			goto error;
		}
		msg_len += ret;
	}

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy, MTK_NL80211_VENDOR_ATTR_FTM_STAT_STR,
						msg, msg_len + 1);
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
				"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");

error:
	os_free_mem(msg);
	return ret;
}

int mtk_cfg80211_vndr_get_ftm_stat_iSTA(
	struct _RTMP_ADAPTER *pAd, POS_COOKIE pObj, RTMP_STRING *msg, int msg_buf_len, int value)
{
	INT msg_len = msg_buf_len;
	INT ret = 0;
	struct wifi_dev *wdev = NULL;

	UINT8 *pAddr;
	UINT8 ind = 0;
	struct _FTM_PEER_INFO *pPeerInfo;
	void *pPeerInfoArray;

	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	if (!wdev)
		return 0;

	memset(msg, 0x00, msg_len);
	ret = snprintf(msg, msg_len, "\n");
	if (os_snprintf_error(msg_len, ret)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"snprintf error!\n");
		return 0;
	}

	if (value == 1) {
		pPeerInfoArray = &wdev->FtmCtrl.rSTA_pinfo;

		ret = snprintf(msg + strlen(msg), msg_len - strlen(msg),
				"======================= as rSTA =======================\n\n"
				"target MAC address\tBandwidth\tDistance\n");
		if (os_snprintf_error(msg_len - strlen(msg), ret)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
				"snprintf error!\n");
			return 0;
		}

		for (ind = 0; ind < wdev->FtmCtrl.LastAssignedRSTA+1; ind++) {
			pPeerInfo = (struct _FTM_PEER_INFO *)pPeerInfoArray + ind;
			pAddr = pPeerInfo->Responder;
			ret = snprintf(msg + strlen(msg), msg_len - strlen(msg),
				"%02x:%02x:%02x:%02x:%02x:%02x\t%d\t\t%d\n",
				pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5],
				pPeerInfo->fmt_and_bw, 0);
			if (os_snprintf_error(msg_len - strlen(msg), ret)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
					"snprintf error!\n");
				return 0;
			}
		}
	} else if (value == 2) {
		pPeerInfoArray = &wdev->FtmCtrl.iSTA_pinfo;

		ret = snprintf(msg + strlen(msg), msg_len - strlen(msg),
				"======================= as iSTA =======================\n\n"
				"target MAC address\tChannel\tBandwidth\tDistance\n");
		if (os_snprintf_error(msg_len - strlen(msg), ret)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
				"snprintf error!\n");
			return 0;
		}

		for (ind = 0; ind < wdev->FtmCtrl.LastAssignedISTA+1; ind++) {
			pPeerInfo = (struct _FTM_PEER_INFO *)pPeerInfoArray + ind;
			pAddr = pPeerInfo->Responder;
			ret = snprintf(msg + strlen(msg), msg_len - strlen(msg),
				"%02x:%02x:%02x:%02x:%02x:%02x\t%d\t%d\t\t%d\n",
				pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5],
				wdev->channel, pPeerInfo->fmt_and_bw, pPeerInfo->distanceMM);
			if (os_snprintf_error(msg_len - strlen(msg), ret)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
					"snprintf error!\n");
				return 0;
			}
		}
	}

	return strlen(msg);
}

#endif /* FTM_SUPPORT */

#endif /* RT_CFG80211_SUPPORT */
