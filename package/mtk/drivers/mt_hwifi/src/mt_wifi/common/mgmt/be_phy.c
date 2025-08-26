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
#include "hdev/hdev.h"

/*
* define  constructor & deconstructor & method
*/
VOID phy_oper_init(struct wifi_dev *wdev, struct phy_op *obj)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	UINT8 ucTxPath;
	UINT8 ucRxPath;
	UINT8 max_nss;

	max_nss = MCS_NSS_CAP(ad)->max_nss;
	ucTxPath = max_nss < MCS_NSS_CAP(ad)->max_path[MAX_PATH_TX]
				? max_nss : MCS_NSS_CAP(ad)->max_path[MAX_PATH_TX];
	ucRxPath = max_nss < MCS_NSS_CAP(ad)->max_path[MAX_PATH_RX]
				? max_nss : MCS_NSS_CAP(ad)->max_path[MAX_PATH_RX];


#ifdef ANTENNA_CONTROL_SUPPORT
	{
		if (ad->bAntennaSetAPEnable) {
			ucTxPath = ad->TxStream;
			ucRxPath = ad->RxStream;
		}
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

	obj->wdev_bw = BW_20;
	obj->tx_stream = wlan_config_get_tx_stream(wdev);
	obj->rx_stream = wlan_config_get_rx_stream(wdev);

	MTWF_DBG(ad, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"obj->tx_stream = %d, obj->rx_stream = %d\n",
		obj->tx_stream, obj->rx_stream);

	if ((obj->tx_stream == 0) || (obj->tx_stream > ucTxPath))
		obj->tx_stream = ucTxPath;

	if ((obj->rx_stream == 0) || (obj->rx_stream > ucRxPath))
		obj->rx_stream = ucRxPath;

#ifdef CONFIG_ATE
	if (!ATE_ON(ad)) {
		/* Set to new T/RX */
		wlan_config_set_tx_stream(wdev, obj->tx_stream);
		wlan_config_set_rx_stream(wdev, obj->rx_stream);

		MTWF_DBG(ad, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"normal mode - set to new T/RX\n");
	}
#endif /* CONFIG_ATE */
	MTWF_DBG(ad, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"Operate TxStream = %d, RxStream = %d\n",
		obj->tx_stream, obj->rx_stream);
}

VOID phy_oper_exit(struct phy_op *obj)
{
	os_zero_mem(obj, sizeof(*obj));
}

/*
* phy_freq internal related function
*/
static UCHAR phy_bw_adjust(UCHAR ht_bw, UCHAR vht_bw)
{
	UCHAR wdev_bw;

	if (ht_bw == HT_BW_40)
		ht_bw = HT_BW_40;
	else
		ht_bw = HT_BW_20;

	if (ht_bw == HT_BW_20)
		wdev_bw = BW_20;
	else {
#ifdef DOT11_VHT_AC
		if (vht_bw == VHT_BW_80)
			wdev_bw = BW_80;
		else if (vht_bw == VHT_BW_160)
			wdev_bw = BW_160;
		else if (vht_bw == VHT_BW_8080)
			wdev_bw = BW_8080;
		else
#endif /* DOT11_VHT_AC */
			wdev_bw = BW_40;
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"ht_bw=%d, vht_bw=%d, bw=%d\n", ht_bw, vht_bw, wdev_bw);
	return wdev_bw;
}

#ifdef DOT11_EHT_BE
/* When peer_coordinate is true, then we compare peer's bw
 * capability with our wdev's bw capabaility to negotiate
 * a suitable bandwidth for each other.
 */
UCHAR phy_bw_adjust_eht(UCHAR bw, UCHAR eht_bw, BOOLEAN peer_coordinate)
{
	UCHAR wdev_eht_bw = BW_20;

	if (eht_bw == EHT_BW_320)
		wdev_eht_bw = BW_320;
	else if (eht_bw == EHT_BW_160)
		wdev_eht_bw = BW_160;
	else if (eht_bw == EHT_BW_80)
		wdev_eht_bw = BW_80;
	else if (eht_bw == EHT_BW_2040)
		wdev_eht_bw = BW_40;

	if ((wdev_eht_bw >= BW_160) && (bw == BW_160) && (peer_coordinate == FALSE))
		return wdev_eht_bw;

	wdev_eht_bw = min(bw, wdev_eht_bw);

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"bw=%d, eht_bw=%d, wdev_eht_bw=%d\n", bw, eht_bw, wdev_eht_bw);
	return wdev_eht_bw;
}
#endif	/* #ifdef DOT11_EHT_BE */

static VOID phy_ht_vht_bw_adjust(UCHAR bw, UCHAR *ht_bw, UCHAR *vht_bw)
{
	if (bw < BW_40)
		*ht_bw = HT_BW_20;
	else
		*ht_bw = HT_BW_40;

#ifdef DOT11_VHT_AC
	*vht_bw = rf_bw_2_vht_bw(bw);
#endif /*DOT11_VHT_AC*/
	return;
}

static BOOLEAN cal_cent_ch_ate(UCHAR prim_ch, CHAR ext_cha, UCHAR *cen_ch)
{
	*cen_ch = prim_ch - ext_cha;

	return TRUE;
}

BOOLEAN phy_freq_adjust(struct wifi_dev *wdev, struct freq_cfg *cfg, struct freq_oper *op)
{
	UCHAR reg_cap_bw;
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	/*initial to legacy setting*/
	if (cfg->prim_ch == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"no prim_ch value for adjust!\n");
		return FALSE;
	}

	op->ht_bw = HT_BW_20;
	op->ext_cha = EXTCHA_NONE;
#ifdef DOT11_VHT_AC
	op->vht_bw = VHT_BW_2040;
#endif /*DOT11_VHT_AC*/
	op->ch_band = cfg->ch_band;
	op->prim_ch = cfg->prim_ch;
	op->cen_ch_2 = 0;
	op->cen_ch_1 = op->prim_ch;
#ifdef DOT11_HE_AX
	op->ap_bw = cfg->ap_bw;
	op->ap_cen_ch = cfg->ap_cen_ch;
#endif	/* DOT11_HE_AX */
	op->rx_stream = cfg->rx_stream;
#ifdef DOT11_EHT_BE
	op->eht_cen_ch = 0;
#endif /* #ifdef DOT11_EHT_BE */
#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode)) {
		op->ht_bw = cfg->ht_bw;
		op->ext_cha = cfg->ext_cha;
		if (!is_testmode_wdev(wdev->wdev_type))
			ht_ext_cha_adjust(pAd, op->prim_ch, &op->ht_bw, &op->ext_cha, wdev);
	}

#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode))
		op->vht_bw = (op->ht_bw >= HT_BW_40) ? cfg->vht_bw : VHT_BW_2040;
#endif /*DOT11_VHT_AC*/

	op->bw = phy_bw_adjust(op->ht_bw, op->vht_bw);
	/* In phy_bw_adjust, we compare the minimum of op->vht_bw and op->eht_bw,
	 * if wdev is not support EHT, set op->eht_bw to EHT_BW_160 as non EHT bw max
	 * to get op->vht_bw since the max value of op->vht_bw is VHT_BW_160
	 */
#ifdef DOT11_EHT_BE
	if (WMODE_CAP_BE(wdev->PhyMode)) {
		op->eht_bw = cfg->eht_bw;
		op->bw = phy_bw_adjust_eht(op->bw, op->eht_bw, false);
	}
#endif

	op->vht_bw = rf_bw_2_vht_bw(op->bw);

	/*check region capability*/
	if (!is_testmode_wdev(wdev->wdev_type)) {
		reg_cap_bw = get_channel_bw_cap(wdev, op->prim_ch);

		if (op->bw > reg_cap_bw) {
			if (!(op->bw == BW_8080 && (reg_cap_bw == BW_80 || reg_cap_bw == BW_160))) {
				/* if bw capability of primary channel is lower than .dat bw config, bw should follow reg_cap_bw*/
				op->bw = reg_cap_bw;
				phy_ht_vht_bw_adjust(op->bw, &op->ht_bw, &op->vht_bw);
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
					"bw is reconfigured to be %d\n", reg_cap_bw);
			}
		}

		if (((pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport &&
		hc_get_hw_band_idx(pAd) == pAd->CommonCfg.DfsParameter.ZeroWaitBandidx) ||
		pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport == TRUE) &&
		pAd->CommonCfg.DfsParameter.ZwAdjBwFlag) {
			if (pAd->CommonCfg.DfsParameter.ZwAdjBw < op->bw) {
				op->bw = pAd->CommonCfg.DfsParameter.ZwAdjBw;
				phy_ht_vht_bw_adjust(op->bw, &op->ht_bw, &op->vht_bw);
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
				"zw bw is reconfigured to be %d\n", pAd->CommonCfg.DfsParameter.ZwAdjBw);
			}
		}
	}

	/*check if need to adjust bw for 40Mhz,
	as 40 MHz is forbiden*/
	if (op->bw == BW_40 && Is40MHzForbid(pChCtrl))
		op->bw = BW_20;

#ifdef DOT11_EHT_BE
	op->eht_bw = rf_bw_2_eht_bw(op->bw);
#endif /* #ifdef DOT11_EHT_BE */

	/* test mode */
	if (is_testmode_wdev(wdev->wdev_type)) {
		cal_cent_ch_ate(op->prim_ch, op->ext_cha, &op->cen_ch_1);

		if (op->bw == BW_8080)
			op->cen_ch_2 = cfg->cen_ch_2;

		return TRUE;
	}

	/*central ch*/
	if (op->bw == BW_40) {
		if (is_testmode_wdev(wdev->wdev_type)) {
			cal_cent_ch_ate(op->prim_ch, op->ext_cha, &op->cen_ch_1);
		} else {
			if (cal_ht_cent_ch(op->prim_ch, op->bw, op->ext_cha, &op->cen_ch_1) != TRUE) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
						 "buggy here.\n");
				return FALSE;
			}
		}
	}
#ifdef DOT11_VHT_AC
	else if ((op->bw > BW_40) && (op->bw < BW_320)) {
		op->cen_ch_1 = vht_cent_ch_freq(op->prim_ch, op->vht_bw, op->ch_band);

		if (op->bw == BW_8080)
			op->cen_ch_2 = cfg->cen_ch_2;
	}

#endif /*DOT11_VHT_AC*/
#endif /*DOT11_N_SUPPORT*/
#ifdef DOT11_EHT_BE
	else if (op->bw == BW_320) {
		struct ch_info ch_info;

		/*TODO: We need to fill cent_ch in HE OP IE, need to refine later.*/
		op->cen_ch_1 = vht_cent_ch_freq(op->prim_ch, op->vht_bw, op->ch_band);

		/* Do chip extra action to limit in 320-1 for Eagle E1 */
		ch_info.prim_ch = op->prim_ch;
		ch_info.bw = op->bw;
		ch_info.ext_cha = cfg->ext_cha;
		ch_info.cen_ch = 0;
		chip_do_extra_action(pAd, wdev, NULL, CHIP_EXTRA_ACTION_EXTCHA_ADJUST,
			(UCHAR *)&ch_info, NULL);

		op->ext_cha = ch_info.ext_cha;
		op->bw = ch_info.bw;
		if (ch_info.cen_ch != 0)
			op->eht_cen_ch = ch_info.cen_ch;
		else
			op->eht_cen_ch = eht_cent_ch_freq(wdev, op->prim_ch, op->eht_bw, op->ch_band);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"cfg->ext_cha=%d, op->ext_cha=%d, op->eht_cen_ch=%d.\n",
			cfg->ext_cha, op->ext_cha, op->eht_cen_ch);
	}
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"op: ht_bw(%d), vht_bw(%d), op->eht_bw(%d), bw(%d)\n",
			op->ht_bw, op->vht_bw, op->eht_bw, op->bw);
#endif /* #ifdef DOT11_EHT_BE */

	return TRUE;
}


static VOID phy_freq_update(struct wifi_dev *wdev, struct freq_oper *oper)
{
	struct wlan_operate *op = (struct wlan_operate *)wdev->wpf_op;
	BOOLEAN vht_phy_change = FALSE;
	struct _RTMP_ADAPTER *ad = NULL;
#ifdef DOT11_EHT_BE
	BOOLEAN eht_phy_change = FALSE;
#endif /* DOT11_EHT_BE */

	if (!op)
		return;

	op->phy_oper.ch_band = oper->ch_band;
	op->phy_oper.prim_ch = oper->prim_ch;
	operate_loader_prim_ch(op);
	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_TURNKEY_ENABLE(ad) || IS_MAP_BS_ENABLE(ad)) {
		if (oper->bw != op->phy_oper.wdev_bw ||
		(op->eht_oper.eht_cen_ch != oper->eht_cen_ch) || (op->eht_oper.bw != oper->eht_bw) ||
			oper->ht_bw != op->ht_oper.ht_bw) {
			wdev->map_indicate_channel_change = 1;
		}
	}
#endif
	op->phy_oper.wdev_bw = oper->bw;
	op->ht_oper.ht_bw = oper->ht_bw;
	operate_loader_ht_bw(op);
	op->ht_oper.ext_cha = oper->ext_cha;
	operate_loader_ext_cha(op);
#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(wdev->PhyMode) && (wdev->wdev_type == WDEV_TYPE_AP)) {
		UCHAR ccfs_0_new = (oper->prim_ch > oper->cen_ch_1) ? (oper->cen_ch_1 + 8):(oper->cen_ch_1 - 8);
		UCHAR ccfs_0 = (wdev->channel > op->phy_oper.cen_ch_1) ?
			(op->phy_oper.cen_ch_1 + 8):(op->phy_oper.cen_ch_1 - 8);

		if ((op->vht_oper.vht_bw != oper->vht_bw) || (op->phy_oper.cen_ch_1 != oper->cen_ch_1))
			vht_phy_change = TRUE;
		else if ((op->vht_oper.vht_bw == VHT_BW_160) && (ccfs_0_new != ccfs_0))
			vht_phy_change = TRUE;
	}
	op->vht_oper.vht_bw = oper->vht_bw;
	wdev->DesiredHtPhyInfo.vht_bw = oper->vht_bw;
#endif /*DOT11_VHT_AC*/
	op->phy_oper.cen_ch_1 = oper->cen_ch_1;
	op->phy_oper.cen_ch_2 = oper->cen_ch_2;
#ifdef DOT11_EHT_BE
#ifdef CONFIG_MAP_SUPPORT
	if (op->eht_oper.eht_cen_ch != oper->eht_cen_ch)
		wdev->eht_ch_change = 1;
#endif
	if ((wdev->wdev_type == WDEV_TYPE_AP) &&
		((op->eht_oper.eht_cen_ch != oper->eht_cen_ch)
		|| (op->eht_oper.bw != oper->eht_bw)))
		eht_phy_change = TRUE;
	op->eht_oper.eht_cen_ch = oper->eht_cen_ch;
	op->eht_oper.bw = oper->eht_bw;
#endif /* #ifdef DOT11_EHT_BE */

	if (vht_phy_change == TRUE) {
		if (bcn_bpcc_op_lock(ad, wdev, TRUE, BCN_BPCC_VHTOP) == FALSE) {
			if (in_interrupt()) {
				OS_SEM_LOCK(&wdev->bpcc_vhtop_info.vht_opinfo_bpcc_lock);
				wdev->bpcc_vhtop_info.status = VHTOP_INFO_UPDATE_FAIL;
				OS_SEM_UNLOCK(&wdev->bpcc_vhtop_info.vht_opinfo_bpcc_lock);
				bcn_bpcc_ct_switch(ad, wdev, BCN_BPCC_VHTOP, BCN_REASON(BCN_UPDATE_IE_CHG));
				return;
			}
			MTWF_DBG(ad, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
				"%s: VHTOP bcn_bpcc_op_lock fail!\n", __func__);
		}
		UpdateBeaconHandler_BPCC(wdev->sys_handle, wdev, BCN_REASON(BCN_UPDATE_IE_CHG),
				BCN_BPCC_VHTOP, TRUE);
	}
#ifdef DOT11_EHT_BE
	if (eht_phy_change) {
		if (bcn_bpcc_op_lock(ad, wdev, TRUE, BCN_BPCC_EHTOP) == FALSE) {
			if (in_interrupt()) {
				bcn_bpcc_ct_switch(ad, wdev, BCN_BPCC_EHTOP, BCN_REASON(BCN_UPDATE_IE_CHG));
				return;
			}
			MTWF_DBG(ad, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
				"%s: EHTOP bcn_bpcc_op_lock fail!\n", __func__);
		}
		UpdateBeaconHandler_BPCC(wdev->sys_handle, wdev, BCN_REASON(BCN_UPDATE_IE_CHG),
				BCN_BPCC_EHTOP, TRUE);
	}
#endif /* DOT11_EHT_BE */
}

/*
* Utility
*/
VOID phy_freq_get_cfg(struct wifi_dev *wdev, struct freq_cfg *fcfg)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (!cfg) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	fcfg->prim_ch = wdev->channel;
	fcfg->ch_band = cfg->phy_conf.ch_band;
	fcfg->cen_ch_2 = cfg->phy_conf.cen_ch_2;

	if (
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		IS_APCLI_SYNC_PEER_DEAUTH_ENBL(pAd) ||
#endif
		(pAd->CommonCfg.bBssCoexEnable &&
		pAd->CommonCfg.BssCoexScanLastResult.bNeedFallBack)) {
		fcfg->ht_bw = wlan_operate_get_ht_bw(wdev);
		fcfg->ext_cha = wlan_operate_get_ext_cha(wdev);
		fcfg->vht_bw = wlan_operate_get_vht_bw(wdev);
	} else {
		fcfg->ht_bw = cfg->ht_conf.ht_bw;
		fcfg->ext_cha = cfg->ht_conf.ext_cha;
		fcfg->vht_bw = cfg->vht_conf.vht_bw;
	}

#ifdef DOT11_HE_AX
	fcfg->ap_bw = cfg->phy_conf.ap_bw;
	fcfg->ap_cen_ch = cfg->phy_conf.ap_cen_ch;
#endif	/* DOT11_HE_AX */
	fcfg->rx_stream = cfg->phy_conf.rx_stream;
#ifdef DOT11_EHT_BE
	fcfg->eht_bw = cfg->eht_conf.bw;
#endif /* DOT11_EHT_BE */
}

/*
* Configure loader
*/

/*
* Operater loader
*/
VOID operate_loader_prim_ch(struct wlan_operate *op)
{
	UCHAR prim_ch = op->phy_oper.prim_ch;

	op->ht_status.addht.ControlChan = prim_ch;
}

/*
*
*/
VOID operate_loader_phy(struct wifi_dev *wdev, struct freq_cfg *cfg)
{
	struct freq_oper oper_dev;
	struct freq_oper oper_tmp_dev;
	struct radio_res res;
	UCHAR i = 0;
	struct wifi_dev *tdev = NULL;
	struct _RTMP_ADAPTER *ad = NULL;
	UCHAR old_bw, new_bw;

	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				 "wdev NULL!");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "(caller:%pS), PhyMode=%d.\n",
			 OS_TRACE, wdev->PhyMode);
#ifdef DOT11_EHT_BE
	MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"oper_cfg: prim_ch(%d), ht_bw(%d), extcha(%d), vht_bw(%d), eht_bw(%d), cen_ch_2(%d).\n",
		cfg->prim_ch,
		cfg->ht_bw,
		cfg->ext_cha,
		cfg->vht_bw,
		cfg->eht_bw,
		cfg->cen_ch_2);
#endif

	if (ad->ch_chg_info.old_bw == 0xFF)
		old_bw = wlan_operate_get_bw(wdev);
	else
		old_bw = ad->ch_chg_info.old_bw;

	os_zero_mem(&oper_dev, sizeof(oper_dev));
	if (!phy_freq_adjust(wdev, cfg, &oper_dev)) {
		MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "phy_freq_adjust failed!");
		return;
	}

#ifdef DOT11_EHT_BE
	MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"bw(%d), prim_ch(%d), cen_ch_1(%d), cen_ch_2(%d), eht_cen_ch(%d), ext_cha(%d)!\n",
		oper_dev.bw,
		oper_dev.prim_ch,
		oper_dev.cen_ch_1,
		oper_dev.cen_ch_2,
		oper_dev.eht_cen_ch,
		oper_dev.ext_cha);
#endif

	phy_freq_update(wdev, &oper_dev);
	MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"oper_dev after update: bw(%d), prim_ch(%d), cen_ch_1(%d), cen_ch_2(%d),ext_cha(%d)!\n",
		oper_dev.bw,
		oper_dev.prim_ch,
		oper_dev.cen_ch_1,
		oper_dev.cen_ch_2,
		oper_dev.ext_cha);

	/*get last radio result for hdev check and update*/
	/*acquire radio resouce*/
	res.reason = REASON_NORMAL_SW;
	res.oper = &oper_dev;

#ifdef CONFIG_AP_SUPPORT
#ifdef MT_DFS_SUPPORT
	/* Perform CAC only for DFS Channel */
	if (DfsRadarChannelCheck(ad, wdev, oper_dev.cen_ch_2, oper_dev.bw)) {
		DfsCacNormalStart(ad, wdev, CHAN_SILENCE_MODE);
		DfsCacNormalStart(ad, wdev, CHAN_NOP_MODE); /* NOP status of channel */
	}
#endif
#endif

	if (hc_radio_res_request(wdev, &res) != TRUE) {
		/*can't get radio resource, update operating to radio setting*/
		MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
				 "oper_dev request radio fail! bw(%d), prim_ch(%d), cen_ch_1(%d), cen_ch_2(%d)!\n",
				  oper_dev.bw,
				  oper_dev.prim_ch,
				  oper_dev.cen_ch_1,
				  oper_dev.cen_ch_2);
		/*Even if the interface is down, the relevant configuration
			should be updated and take effect when it is up*/
		phy_freq_update(wdev, &oper_dev);
		return;
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef MT_DFS_SUPPORT
	DfsCacNormalStart(ad, wdev, CHAN_NORMAL_MODE);

	/* Perform CAC & Radar Detect only for DFS Channel */
	if (DfsRadarChannelCheck(ad, wdev, oper_dev.cen_ch_2, oper_dev.bw))
		WrapDfsRadarDetectStart(ad, wdev);
#endif
#endif

	wdev_sync_prim_ch(ad, wdev);

	new_bw = oper_dev.bw;
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = ad->wdev_list[i];
		if (!tdev)
			continue;

		if (tdev != wdev)
			phy_freq_update(tdev, &oper_dev);

		if (WDEV_BSS_STATE(wdev) >= BSS_ACTIVE) {
			/* Refresh bss and sta entries for ch/bw change */
			if (hc_radio_query_by_wdev(tdev, &oper_tmp_dev) == HC_STATUS_OK)
				ap_update_rf_ch_for_mbss(ad, tdev, &oper_tmp_dev);

			if (new_bw !=  old_bw) {
				MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
					"bw change from %d to:%d\n", old_bw, new_bw);
				UpdatedRaBfInfoBwByWdev(ad, tdev, new_bw);
			}
		}
	}

	if (ad->ch_chg_info.old_channel != wdev->channel || old_bw != new_bw) {
		ad->ch_chg_info.old_channel = wdev->channel;
		ad->ch_chg_info.old_bw = new_bw;
		ad->ch_chg_info.ch_chg_cnt++;
		MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"Channel updating done. prim_ch(%d), bw(%d), ch_chg_cnt(%d)!\n",
			ad->ch_chg_info.old_channel,
			ad->ch_chg_info.old_bw,
			ad->ch_chg_info.ch_chg_cnt);

		for (i = 0; i < WDEV_NUM_MAX; i++) {
			tdev = ad->wdev_list[i];
			if (!tdev)
				continue;
			if (WDEV_BSS_STATE(tdev) >= BSS_ACTIVE) {
				/* update ch/bw info for bss_mngr related IE */
				bss_mngr_dev_update(tdev);
#ifdef DOT11_EHT_BE
				bss_mngr_mld_sync_ml_probe_rsp(tdev);
#endif
			}
		}
	}

#ifdef RT_CFG80211_SUPPORT
	if (!ad->CommonCfg.wifi_cert) {
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
		struct cfg80211_chan_def *chandef = wdev_chandef(ad->pCfg80211_CB->pCfg80211_Wdev, 0);
#else
		struct cfg80211_chan_def *chandef = &ad->pCfg80211_CB->pCfg80211_Wdev->chandef;
#endif /* CFG_CFG80211_VERSION */
		struct net_device *net_dev = ad->pCfg80211_CB->pCfg80211_Wdev->netdev;
		struct ieee80211_channel *p_chan = NULL;
		enum nl80211_band band_idx;
		UCHAR new_bw = oper_dev.bw;

		if (!chandef) {
			MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
				"chandef is NULL\n");
			return;
		}

		p_chan = (struct ieee80211_channel *)CFG80211_FindChan(ad, oper_dev.prim_ch);
		if (!p_chan) {
			MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "p_chan NULL!\n");
			return;
		}

		band_idx = CFG80211_Get_BandId(ad);
		chandef->chan = p_chan;
		if (new_bw < BW_320)
			chandef->center_freq1 = ieee80211_channel_to_frequency(oper_dev.cen_ch_1, band_idx);
		else
			chandef->center_freq1 = ieee80211_channel_to_frequency(oper_dev.eht_cen_ch, band_idx);

		chandef->center_freq2 = ieee80211_channel_to_frequency(oper_dev.cen_ch_2, band_idx);
		chandef->width = phy_bw_2_nl_bw(new_bw);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0) || defined(BACKPORT_NOSTDINC)
		MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"Notify upstream,center_freq=%d,center_freq1=%d,center_freq2=%d,width=%d.\n",
		chandef->chan->center_freq, chandef->center_freq1,
		chandef->center_freq2, chandef->width);
		MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"Notify upstream, freq_offset=%d\n",
		chandef->chan->freq_offset);
#endif /*KERNEL_VERSION(5, 8, 0) > LINUX_VERSION_CODE || defined(BACKPORT_NOSTDINC)*/
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
		cfg80211_ch_switch_notify(net_dev, chandef, 0, 0);
#else
		cfg80211_ch_switch_notify(net_dev, chandef);
#endif /* CFG_CFG80211_VERSION */
	}
#endif /*RT_CFG80211_SUPPORT*/

}

UCHAR wlan_config_get_bw(struct wifi_dev *wdev)
{
	UCHAR config_bw = BW_20;
	struct wlan_config *cfg = NULL;

	cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (cfg == NULL)
		return config_bw;

	if (cfg->ht_conf.ht_bw == HT_BW_20)
		config_bw = BW_20;
	else if (cfg->ht_conf.ht_bw == HT_BW_40) {
		if (cfg->vht_conf.vht_bw == VHT_BW_2040)
			config_bw = BW_40;
		else if (cfg->vht_conf.vht_bw == VHT_BW_80)
			config_bw = BW_80;
		else if (cfg->vht_conf.vht_bw == VHT_BW_160)
			config_bw = BW_160;
#ifdef DOT11_EHT_BE
		else if (cfg->eht_conf.bw == EHT_BW_320)
			config_bw = BW_320;
#endif
		else
			;
	}

	return config_bw;
}

/*
* export function
*/

/*
* operation function
*/
UCHAR wlan_operate_get_bw(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "op NULL\n");
		return 0;
	}

	return op->phy_oper.wdev_bw;
}

UCHAR wlan_operate_get_prim_ch(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "op NULL\n");
		return 0;
	}

	return op->phy_oper.prim_ch;
}

UCHAR wlan_operate_get_ch_band(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "op NULL\n");
		return 0;
	}

	return op->phy_oper.ch_band;
}

INT32 wlan_operate_set_ch_band(struct wifi_dev *wdev, UCHAR ch_band)
{
	struct freq_cfg cfg;

	os_zero_mem(&cfg, sizeof(cfg));
	phy_freq_get_cfg(wdev, &cfg);
	cfg.ch_band = ch_band;
	operate_loader_phy(wdev, &cfg);
	return WLAN_OPER_OK;
}

INT32 wlan_operate_set_prim_ch(struct wifi_dev *wdev, UCHAR prim_ch)
{
	struct freq_cfg cfg;
#ifdef MGMT_TXPWR_CTRL
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#endif
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "(caller:%pS), wdev_idx:%d.\n",
			 OS_TRACE, wdev->wdev_idx);

	os_zero_mem(&cfg, sizeof(cfg));
	phy_freq_get_cfg(wdev, &cfg);
	cfg.prim_ch = prim_ch;
	operate_loader_phy(wdev, &cfg);
#ifdef MGMT_TXPWR_CTRL
	/* Update tx power for each channel change */
	wdev->bPwrCtrlEn = FALSE;
	wdev->TxPwrDelta = 0;
	wdev->mgmt_txd_txpwr_offset = 0;
	/* Get EPA info by Tx Power info cmd*/
	pAd->ApCfg.MgmtTxPwr = 0;
	MtCmdTxPwrShowInfo(pAd, TXPOWER_ALL_RATE_POWER_INFO, HcGetBandByWdev(wdev));
	if (wdev->MgmtTxPwr) {
	/* wait until TX Pwr event rx*/
		RtmpusecDelay(50);
		update_mgmt_frame_power(pAd, wdev);
	}
#endif
	return WLAN_OPER_OK;
}

INT32 wlan_operate_set_phy(struct wifi_dev *wdev, struct freq_cfg *cfg)
{
	operate_loader_phy(wdev, cfg);
	return WLAN_OPER_OK;
}


INT32 wlan_operate_set_tx_stream(struct wifi_dev *wdev, UINT8 tx_stream)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "op NULL\n");
		return 0;
	}

	op->phy_oper.tx_stream = tx_stream;
	return WLAN_OPER_OK;
}


INT32 wlan_operate_set_rx_stream(struct wifi_dev *wdev, UINT8 rx_stream)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "op NULL\n");
		return 0;
	}

	op->phy_oper.rx_stream = rx_stream;
	return WLAN_OPER_OK;
}


UCHAR wlan_operate_get_cen_ch_2(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "op NULL\n");
		return 0;
	}

	return op->phy_oper.cen_ch_2;
}

INT32 wlan_operate_set_cen_ch_2(struct wifi_dev *wdev, UCHAR cen_ch_2)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;
	struct freq_cfg cfg;

	os_zero_mem(&cfg, sizeof(cfg));
	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "op NULL\n");
		return 0;
	}

	if (op->phy_oper.cen_ch_2 == cen_ch_2)
		return WLAN_OPER_OK;

	phy_freq_get_cfg(wdev, &cfg);
	cfg.cen_ch_2 = cen_ch_2;
	operate_loader_phy(wdev, &cfg);
	return WLAN_OPER_OK;
}

UCHAR wlan_operate_get_cen_ch_1(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "op NULL\n");
		return 0;
	}

	return op->phy_oper.cen_ch_1;
}

UCHAR wlan_operate_get_eht_cen_ch(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "op NULL\n");
		return 0;
	}

	return op->eht_oper.eht_cen_ch;
}

UINT8 wlan_operate_get_tx_stream(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "op NULL\n");
		return 0;
	}

	return op->phy_oper.tx_stream;
}


UINT8 wlan_operate_get_rx_stream(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "op NULL\n");
		return 0;
	}

	return op->phy_oper.rx_stream;
}

BOOLEAN wlan_operate_full_bw_scan(struct wifi_dev *wdev, UCHAR prim_ch)
{
	struct radio_res radio, *res = &radio;
	struct freq_oper oper;
	BOOLEAN ret;
	UCHAR cfg_ht_bw = 0;
	UCHAR cfg_vht_bw = 0;
	UCHAR scan_bw = 0;

	os_zero_mem(&oper, sizeof(oper));
	res->oper = &oper;

	/* do not change sequence due to 6GHz might include AC/GN then confused */
	if (WMODE_CAP_6G(wdev->PhyMode))
		oper.ch_band = CMD_CH_BAND_6G;
	else if (WMODE_CAP_5G(wdev->PhyMode))
		oper.ch_band = CMD_CH_BAND_5G;
	else
		oper.ch_band = CMD_CH_BAND_24G;

	cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	cfg_vht_bw = wlan_config_get_vht_bw(wdev);

	/*when cfg bw >= 80M, we use BW_80 do scan*/
	if (cfg_ht_bw != HT_BW_40)
		scan_bw = BW_20;
	else if (cfg_vht_bw == VHT_BW_2040)
		scan_bw = BW_40;
	else
		scan_bw = BW_80;

	switch (scan_bw) {
	case BW_20:
		oper.bw = BW_20;
		oper.ht_bw = HT_BW_20;
		oper.vht_bw = VHT_BW_2040;
		oper.cen_ch_1 = prim_ch;
		break;
	case BW_40:
		oper.bw = BW_40;
		oper.ht_bw = HT_BW_40;
		oper.vht_bw = VHT_BW_2040;
		oper.cen_ch_1 = vht_cent_ch_freq_40mhz(prim_ch, VHT_BW_2040, oper.ch_band);
		break;
	case BW_80:
		oper.bw = BW_80;
		oper.ht_bw = HT_BW_40;
		oper.vht_bw = VHT_BW_80;
		oper.cen_ch_1 = vht_cent_ch_freq(prim_ch, VHT_BW_80, oper.ch_band);
		break;
	default:
		return FALSE;
	}

	oper.ext_cha = EXTCHA_NONE;
	oper.cen_ch_2 = 0;
	oper.prim_ch = prim_ch;
	res->reason = REASON_NORMAL_SCAN;
	ret = hc_radio_res_request(wdev, res);
	return ret;
}
BOOLEAN wlan_operate_scan(struct wifi_dev *wdev, UCHAR prim_ch)
{
	struct radio_res radio, *res = &radio;
	struct freq_oper oper;
	BOOLEAN ret;

	os_zero_mem(&oper, sizeof(oper));
	res->oper = &oper;

	/* do not change sequence due to 6GHz might include AC/GN then confused */
	if (WMODE_CAP_6G(wdev->PhyMode))
		oper.ch_band = CMD_CH_BAND_6G;
	else if (WMODE_CAP_5G(wdev->PhyMode))
		oper.ch_band = CMD_CH_BAND_5G;
	else
		oper.ch_band = CMD_CH_BAND_24G;

	oper.bw = BW_20;
	oper.cen_ch_1 = prim_ch;
	oper.ext_cha = EXTCHA_NONE;
	oper.ht_bw = HT_BW_20;
#ifdef DOT11_VHT_AC
	oper.cen_ch_2 = 0;
	oper.vht_bw = VHT_BW_2040;
#endif /*DOT11_VHT_AC*/
	oper.prim_ch = prim_ch;
	res->reason = REASON_NORMAL_SCAN;
	ret = hc_radio_res_request(wdev, res);
	return ret;
}

