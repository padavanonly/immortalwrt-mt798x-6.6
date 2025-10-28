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
	cmm_asic.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/


#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Cmm_asic.tmh"
#endif
#elif defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#include "mcu/mt_cmd.h"
#endif
#include "hdev/hdev.h"
#define MCAST_WCID_TO_REMOVE 0 /* Pat: TODO */


static char *hif_2_str[] = {"HIF_RTMP", "HIF_RLT", "HIF_MT", "Unknown"};
VOID AsicNotSupportFunc(RTMP_ADAPTER *pAd, const RTMP_STRING *caller)
{
	RTMP_STRING *str;
	UINT32 hif_type = GET_HIF_TYPE(pAd);

	if (hif_type <= HIF_MAX)
		str = hif_2_str[hif_type];
	else
		str = hif_2_str[HIF_MAX];

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
		"%s(): NotSupportedFunc for this arch(%s)!\n",
		caller, str);
}

#ifndef	COMPOS_TESTMODE_WIN
UINT32 AsicGetCrcErrCnt(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archGetCrcErrCnt)
		return arch_ops->archGetCrcErrCnt(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}


UINT32 AsicGetCCACnt(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
#ifdef MT_MAC
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archGetCCACnt)
		return arch_ops->archGetCCACnt(pAd, BandIdx);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}


UINT32 AsicGetChBusyCnt(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
#ifdef MT_MAC
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	if (arch_ops->archGetChBusyCnt)
		return arch_ops->archGetChBusyCnt(pAd, BandIdx);
#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

INT AsicSetAutoFallBack(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
#ifdef MT_MAC
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetAutoFallBack)
		return arch_ops->archSetAutoFallBack(pAd, enable);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicAutoFallbackInit(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archAutoFallbackInit)
		return arch_ops->archAutoFallbackInit(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


VOID AsicUpdateRtsThld(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT32 pkt_num,
	UINT32 length)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
#ifdef CONFIG_ATE

		if (ATE_ON(pAd))
			return;

#endif /* CONFIG_ATE */

		if (arch_ops->archUpdateRtsThld)
			return arch_ops->archUpdateRtsThld(pAd, wdev, pkt_num, length);
	}

	AsicNotSupportFunc(pAd, __func__);
}


/*
 * ========================================================================
 *
 * Routine Description:
 * Set MAC register value according operation mode.
 * OperationMode AND bNonGFExist are for MM and GF Proteciton.
 * If MM or GF mask is not set, those passing argument doesn't not take effect.
 *
 * Operation mode meaning:
 * = 0 : Pure HT, no preotection.
 * = 0x01; there may be non-HT devices in both the control and extension channel, protection is optional in BSS.
 * = 0x10: No Transmission in 40M is protected.
 * = 0x11: Transmission in both 40M and 20M shall be protected
 * if (bNonGFExist)
 * we should choose not to use GF. But still set correct ASIC registers.
 * ========================================================================
 */
static BOOLEAN
protect_mode_para_preparation(struct _RTMP_ADAPTER *ad,
			      MT_PROTECT_CTRL_T *prot,
			      MT_PROTECT_CTRL_T  *prot_5g)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);
	UINT32 mode = 0, wdev_idx = 0;
	struct wifi_dev *wdev = NULL;

	do {
		wdev = ad->wdev_list[wdev_idx];

		if (wdev == NULL)
			break;

		mode = wdev->protection;
		if (mode & SET_PROTECT(ERP))
			prot->erp_mask = ERP_OMAC_ALL;

		if (mode & SET_PROTECT(NON_MEMBER_PROTECT)) {
			prot->mix_mode = 1;
			prot->gf = 1;
			prot->bw40 = 1;
		}

		if (mode & SET_PROTECT(HT20_PROTECT))
			prot->bw40 = 1;

		if (mode & SET_PROTECT(NON_HT_MIXMODE_PROTECT)) {
			prot->mix_mode = 1;
			prot->gf = 1;
			prot->bw40 = 1;
		}

		if (mode & SET_PROTECT(GREEN_FIELD_PROTECT))
			prot->gf = 1;

		/* if (mode & SET_PROTECT(RDG)) { */
		if (RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_RDG_ACTIVE))
			prot->long_nav = 1;

		if (mode & SET_PROTECT(LONG_NAV_PROTECT))
			prot->long_nav = 1;

		if (mode & SET_PROTECT(RIFS_PROTECT)) {
			prot->long_nav = 1;
			prot->rifs = 1;
		}

		if (mode & SET_PROTECT(FORCE_RTS_PROTECT)) {
			arch_ops->archUpdateRtsThld(ad, wdev, 0, 1);
			goto end;
		}

		if (mode & SET_PROTECT(_NOT_DEFINE_HT_PROTECT)) {
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_ERROR,
					"NOT Defined HT Protection!\n");
		}

		wdev_idx++;
	} while (wdev_idx < WDEV_NUM_MAX);


	return TRUE;
end:
	return FALSE;

}

VOID AsicUpdateProtect(struct _RTMP_ADAPTER *pAd, struct prot_info *prot)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MT_PROTECT_CTRL_T  protect;
	MT_PROTECT_CTRL_T  protect_5g;

	os_zero_mem(&protect, sizeof(MT_PROTECT_CTRL_T));
	os_zero_mem(&protect_5g, sizeof(MT_PROTECT_CTRL_T));

	if (arch_ops->archUpdateProtect == NULL) {
		AsicNotSupportFunc(pAd, __func__);
		return;
	}

	switch (cap->hw_protect_update_ver) {
	case HWCTRL_PROT_UPDATE_METHOD_V2:
		arch_ops->archUpdateProtect(pAd, (VOID *)prot);
		break;
	case HWCTRL_PROT_UPDATE_METHOD_V1:
	default:
		if (protect_mode_para_preparation(pAd,
						  &protect,
						  &protect_5g) == FALSE)
			goto end;

		arch_ops->archUpdateProtect(pAd, (VOID *)&protect);
		break;
	}
end:
	return;
}

/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicSwitchChannel(RTMP_ADAPTER *pAd, UCHAR band_idx, struct freq_oper *oper, BOOLEAN bScan)
{
#ifdef MT_MAC
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_SWITCH_CHANNEL_CFG SwChCfg;
#ifdef CONFIG_6G_SUPPORT
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
		UINT8 PsdLimit = CheckPSDLimitType(pAd);

		if (wdev && WMODE_CAP_6G(wdev->PhyMode)) {
			if (!pAd->CommonCfg.LpiEn
#ifdef CONFIG_6G_AFC_SUPPORT
				|| is_afc_in_run_state(pAd)
#endif /*CONFIG_6G_AFC_SUPPORT*/
				)
				PsdLimit = 0;
			MtAsicSetLpi(pAd, 0, PsdLimit, band_idx);
		}
#ifdef CONFIG_6G_AFC_SUPPORT
		afc_save_switch_channel_params(pAd, band_idx, oper, bScan);
#endif /*CONFIG_6G_AFC_SUPPORT*/
#endif /*CONFIG_6G_SUPPORT */

		os_zero_mem(&SwChCfg, sizeof(MT_SWITCH_CHANNEL_CFG));
		SwChCfg.bScan = bScan;
		SwChCfg.CentralChannel = oper->cen_ch_1;
#ifdef DOT11_HE_AX
		SwChCfg.ap_central_channel = oper->ap_cen_ch;
#endif	/* DOT11_HE_AX */
		SwChCfg.BandIdx = band_idx;

		SwChCfg.RxStream = pAd->Antenna.field.RxPath;
		SwChCfg.TxStream = pAd->Antenna.field.TxPath;

		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"\x1b[41mRxStream[%d], RxPath[0x%x]\x1b[m\n",
			SwChCfg.RxStream,
			pAd->Antenna.field.RxPath);

#ifdef ANTENNA_CONTROL_SUPPORT
		if (pAd->bAntennaSetAPEnable) {
			SwChCfg.TxStream = pAd->TxStream;
			SwChCfg.RxStream = pAd->RxStream;
		}
#endif /* ANTENNA_CONTROL_SUPPORT */

		oper->rx_stream = SwChCfg.RxStream;
		SwChCfg.Bw = oper->bw;
		SwChCfg.Channel_Band = oper->ch_band;
		/* channel_band 5G as 1 (Not support 802.11j)*/
		if (oper->cen_ch_1 >= 36 && oper->cen_ch_1 <= 165) {
			if (SwChCfg.Channel_Band == 0) {
				SwChCfg.Channel_Band = 1;
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"\x1b[41m5G Channel:%d, then must be Channel_Band:%d !!\x1b[m\n",
				oper->cen_ch_1, SwChCfg.Channel_Band);
			}
		}
#ifdef DOT11_HE_AX
		SwChCfg.ap_bw = oper->ap_bw;
#endif	/* DOT11_HE_AX */
		SwChCfg.ControlChannel = oper->prim_ch;
		SwChCfg.OutBandFreq = 0;
#ifdef DOT11_VHT_AC
		SwChCfg.ControlChannel2 = oper->cen_ch_2;
#endif /* DOT11_VHT_AC */
#ifdef MT_DFS_SUPPORT
		SwChCfg.bDfsCheck = DfsSwitchCheck(pAd, SwChCfg.ControlChannel, band_idx);
#endif
#ifdef DOT11_EHT_BE
		SwChCfg.eht_cen_ch = oper->eht_cen_ch;
#endif /* DOT11_EHT_BE */

		if (arch_ops->archSwitchChannel)
			arch_ops->archSwitchChannel(pAd, SwChCfg);
	}

#endif
}

#ifdef CONFIG_STA_SUPPORT
/*
 * ==========================================================================
 * Description:
 * put PHY to sleep here, and set next wakeup timer. PHY doesn't not wakeup
 * automatically. Instead, MCU will issue a TwakeUpInterrupt to host after
 * the wakeup timer timeout. Driver has to issue a separate command to wake
 * PHY up.
 *
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicSleepAutoWakeup(PRTMP_ADAPTER pAd, PSTA_ADMIN_CONFIG pStaCfg)
{
	USHORT	TbttNumToNextWakeUp = 0;
	USHORT	NextDtim = pStaCfg->DtimPeriod;
	ULONG	Now = 0;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG, "pStaCfg(0x%p)\n",	pStaCfg);
	NdisGetSystemUpTime(&Now);
	NextDtim -= (USHORT)(Now - pStaCfg->LastBeaconRxTime)
		/ pAd->CommonCfg.BeaconPeriod;
	pStaCfg->ThisTbttNumToNextWakeUp = pStaCfg->DefaultListenCount;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM) && (TbttNumToNextWakeUp > NextDtim))
		pStaCfg->ThisTbttNumToNextWakeUp = NextDtim;

	/* if WMM-APSD is failed, try to disable following line*/
	ASIC_STA_SLEEP_AUTO_WAKEUP(pAd, pStaCfg);
}

/*
 * ==========================================================================
 * Description:
 * AsicWakeup() is used whenever Twakeup timer (set via AsicSleepAutoWakeup)
 * expired.
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 * ==========================================================================
 */
VOID AsicWakeup(RTMP_ADAPTER *pAd, BOOLEAN bFromTx, PSTA_ADMIN_CONFIG pStaCfg)
{
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG, "pStaCfg(0x%p)\n", pStaCfg);
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG, "--> AsicWakeup\n");
	ASIC_STA_WAKEUP(pAd, bFromTx, pStaCfg);
}
#endif /* CONFIG_STA_SUPPORT */
#endif/*COMPOS_TESTMODE_WIN*/

INT asic_rts_on_off_detail(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT32 rts_num, UINT32 rts_len, BOOLEAN rts_en)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->asic_rts_on_off)
		return arch_ops->asic_rts_on_off(ad, band_idx, rts_num, rts_len, rts_en);

	AsicNotSupportFunc(ad, __func__);
	return FALSE;
}

BOOLEAN AsicUpdateBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev, BOOLEAN BcnSntReq, UCHAR UpdateReason)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archUpdateBeacon)
		return arch_ops->archUpdateBeacon(pAd, wdev, BcnSntReq, UpdateReason);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

#ifdef DOT11_N_SUPPORT
INT AsicSetRDG(RTMP_ADAPTER *pAd,
			   UINT16 wlan_idx,
			   UCHAR band_idx,
			   UCHAR init,
			   UCHAR resp)
{
	INT ret = FALSE;
	INT bSupport = FALSE;
	BOOLEAN is_en;
	MT_RDG_CTRL_T rdg;
	RTMP_ARCH_OP *arch_op = hc_get_arch_ops(pAd->hdev_ctrl);

	is_en = (init && resp) ? TRUE : FALSE;

	if (arch_op->archSetRDG) {
		bSupport = TRUE;
		rdg.WlanIdx = wlan_idx;
		rdg.BandIdx = band_idx;
		rdg.Init = init;
		rdg.Resp = resp;
		rdg.Txop = (is_en) ? (0x80) : (0x60);
		rdg.LongNav = (is_en) ? (1) : (0);
		ret = arch_op->archSetRDG(pAd, &rdg);
	}

	if (ret == TRUE) {
		if (is_en)
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
		else
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
	}

	if (!bSupport)
		AsicNotSupportFunc(pAd, __func__);

	return ret;
}
#endif /* DOT11_N_SUPPORT */

INT AsicSetPreTbtt(RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR HwBssidIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetPreTbtt) {
		arch_ops->archSetPreTbtt(pAd, enable, HwBssidIdx);
		return TRUE;
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicSetGPTimer(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout)
{
#ifdef MT_MAC
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetGPTimer)
		return arch_ops->archSetGPTimer(pAd, enable, timeout);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicSetChBusyStat(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
#ifdef MT_MAC
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetChBusyStat)
		return arch_ops->archSetChBusyStat(pAd, enable);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicGetTsfTime(
	RTMP_ADAPTER *pAd,
	UINT32 *high_part,
	UINT32 *low_part,
	UCHAR HwBssidIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archGetTsfTime)
		return arch_ops->archGetTsfTime(pAd, high_part, low_part, HwBssidIdx);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicGetTsfDiffTime(
	RTMP_ADAPTER * pAd,
	UINT8 BssIdx0,
	UINT8 BssIdx1,
	UINT32 *Tsf0Bit0_31,
	UINT32 *Tsf0Bit63_32,
	UINT32 *Tsf1Bit0_31,
	UINT32 *Tsf1Bit63_32)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archGetTsfDiffTime)
		return arch_ops->archGetTsfDiffTime(
			pAd, BssIdx0, BssIdx1, Tsf0Bit0_31, Tsf0Bit63_32, Tsf1Bit0_31, Tsf1Bit63_32);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

#ifdef WIFI_UNIFIED_COMMAND
INT AsicSetWmmParam(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR idx, UINT32 ac, UINT32 type, UINT32 val)
#else
INT AsicSetWmmParam(RTMP_ADAPTER *pAd, UCHAR idx, UINT32 ac, UINT32 type, UINT32 val)
#endif /* WIFI_UNIFIED_COMMAND */
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		if (arch_ops->archUniCmdSetWmmParam) {
			return arch_ops->archUniCmdSetWmmParam(pAd, wdev, idx, ac, type, val);
		}
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (arch_ops->archSetWmmParam) {
			return arch_ops->archSetWmmParam(pAd, idx, ac, type, val);
		}
	}

	AsicNotSupportFunc(pAd, __func__);
	return NDIS_STATUS_FAILURE;
}


/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicSetEdcaParm(RTMP_ADAPTER *pAd, struct wmm_entry *entry, struct wifi_dev *wdev)
{
	UINT16 i;
	UCHAR EdcaIdx = wdev->EdcaIdx;
	EDCA_PARM *pEdca = NULL;
	PEDCA_PARM pEdcaParm = &entry->edca;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	struct _MAC_TABLE_ENTRY *peer;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap;
#endif /* WIFI_UNIFIED_COMMAND */
#ifdef CONFIG_STA_SUPPORT
	UCHAR EdcaUpdateCountBak = 0;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif
#ifdef WIFI_UNIFIED_COMMAND
	pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (EdcaIdx >= WMM_NUM_OF_AC) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR, "EdcaIdx >= 4\n");
		return;
	}


	switch (wdev->wdev_type) {
#ifdef CONFIG_AP_SUPPORT
	case WDEV_TYPE_AP:
#ifdef WDS_SUPPORT
	case WDEV_TYPE_WDS:
#endif /*WDS_SUPPORT*/
		pEdca = &pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx];
		break;
#endif /*CONFIG_AP_SUPPORT*/

#ifdef CONFIG_STA_SUPPORT
	case WDEV_TYPE_STA:
		pEdca = &pStaCfg->MlmeAux.APEdcaParm;
		break;
#endif /*CONFIG_STA_SUPPORT*/
	default:
		pEdca = &pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx];
		break;
	}


	if ((pEdcaParm == NULL) || (pEdcaParm->bValid == FALSE)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "NoEDCAParam\n");
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WMM_INUSED);
		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			peer = entry_get(pAd, i);
			if (IS_ENTRY_CLIENT(peer) ||
				IS_ENTRY_PEER_AP(peer) ||
				IS_ENTRY_REPEATER(peer))
				/*check clear for this bss only*/
				if (peer->wdev == wdev)
					CLIENT_STATUS_CLEAR_FLAG(peer, fCLIENT_STATUS_WMM_CAPABLE);
		}

		os_zero_mem(pEdca, sizeof(EDCA_PARM));
	} else {
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_WMM_INUSED);
#ifdef CONFIG_STA_SUPPORT
		/*
			EdcaUpdateCount must keep when linkup, in case that beacon will
			set wmm again for EdcaUpdateCount is not the same
			CR: WCNCR00260092
		*/
		if (wdev->wdev_type == WDEV_TYPE_STA) {
			EdcaUpdateCountBak = pStaCfg->MlmeAux.APEdcaParm.EdcaUpdateCount;
			os_move_mem(pEdca, pEdcaParm, sizeof(EDCA_PARM));
			pEdca->EdcaUpdateCount = EdcaUpdateCountBak;
		} else
#endif /* CONFIG_STA_SUPPORT */
		{
			os_move_mem(pEdca, pEdcaParm, sizeof(EDCA_PARM));
		}

		if (!ADHOC_ON(pAd)) {
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"EDCA [#%d]: AIFSN CWmin CWmax  TXOP(us)  ACM, WMM Set: %d, BandIdx: %d\n",
			pEdcaParm->EdcaUpdateCount,
			entry->wmm_set,
			entry->dbdc_idx);
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"     AC_BE      %2d     %2d     %2d      %4d     %d\n",
			pEdcaParm->Aifsn[0],
			pEdcaParm->Cwmin[0],
			pEdcaParm->Cwmax[0],
			pEdcaParm->Txop[0] << 5,
			pEdcaParm->bACM[0]);
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"     AC_BK      %2d     %2d     %2d      %4d     %d\n",
			pEdcaParm->Aifsn[1],
			pEdcaParm->Cwmin[1],
			pEdcaParm->Cwmax[1],
			pEdcaParm->Txop[1] << 5,
			pEdcaParm->bACM[1]);
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"     AC_VI      %2d     %2d     %2d      %4d     %d\n",
			pEdcaParm->Aifsn[2],
			pEdcaParm->Cwmin[2],
			pEdcaParm->Cwmax[2],
			pEdcaParm->Txop[2] << 5,
			pEdcaParm->bACM[2]);
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"     AC_VO      %2d     %2d     %2d      %4d     %d\n",
			pEdcaParm->Aifsn[3],
			pEdcaParm->Cwmin[3],
			pEdcaParm->Cwmax[3],
			pEdcaParm->Txop[3] << 5,
			pEdcaParm->bACM[3]);
		}
#ifdef APCLI_SUPPORT
		/* This is added for TGn 5.2.30 */
		if (pAd->bApCliCertTest && pStaCfg && wdev->wdev_type == WDEV_TYPE_STA) {
			/* SSID for TGn TC 5.2.30 */
			UCHAR Ssid[] = "Bg(*^J78";
			UCHAR SsidEqual = 0;
			/* SSID for TGn TC 5.2.43 */
			UCHAR Ssid2[] = "AP1-5.2.43";
			UCHAR Ssid2Equal = 0;
			/* SSID for TGac TC 5.2.61 */
			UCHAR Ssid3[] = "VHT-5.2.61-AP1";
			UCHAR Ssid3Equal = 0;
			/* SSID for TGac TC 5.2.28 */
			UCHAR Ssid4[] = "VHT-5.2.28";
			UCHAR Ssid4Equal = 0;
			/* SSID for TGn TC 5.2.33 */
			UCHAR Ssid5[] = "5.2.33";
			UCHAR Ssid5Equal = 0;

			SsidEqual = SSID_EQUAL(pStaCfg->CfgSsid, pStaCfg->CfgSsidLen, Ssid, strlen(Ssid));
			Ssid2Equal = SSID_EQUAL(pStaCfg->CfgSsid, pStaCfg->CfgSsidLen, Ssid2, strlen(Ssid2));
			Ssid3Equal = SSID_EQUAL(pStaCfg->CfgSsid, pStaCfg->CfgSsidLen, Ssid3, strlen(Ssid3));
			Ssid4Equal = SSID_EQUAL(pStaCfg->CfgSsid, pStaCfg->CfgSsidLen, Ssid4, strlen(Ssid4));
			Ssid5Equal = SSID_EQUAL(pStaCfg->CfgSsid, pStaCfg->CfgSsidLen, Ssid5, strlen(Ssid5));

			/* To tame down the BE aggresiveness increasing the Cwmin */
			if ((SsidEqual || Ssid2Equal) && (pEdcaParm->Cwmin[0] == 4)) {
				if (SsidEqual)
					pAd->CommonCfg.bEnableTxBurst = TRUE;
				pEdcaParm->Cwmin[0]++;
			}

			if (Ssid3Equal) {
				pEdcaParm->Aifsn[0] = 7;
				pEdcaParm->Cwmin[0] += 2;
			}

			if ((Ssid4Equal || Ssid5Equal) && (pEdcaParm->Cwmin[2] == 3)) {
				pEdcaParm->Cwmin[2]++;
				pEdcaParm->Cwmax[2]++;
			}
		}
#endif
	}

#ifdef VOW_SUPPORT
	vow_update_om_wmm(pAd, wdev, entry->wmm_set, pEdcaParm);
#endif /* VOW_SUPPORT */
#ifdef WIFI_UNIFIED_COMMAND
	if (pChipCap->uni_cmd_support) {
		if (arch_ops->archUniCmdSetEdcaParm) {
			arch_ops->archUniCmdSetEdcaParm(pAd, wdev, entry->wmm_set, entry->tx_mode, pEdcaParm);
			return;
		}
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (arch_ops->archSetEdcaParm) {
			arch_ops->archSetEdcaParm(pAd, entry->wmm_set, entry->tx_mode, pEdcaParm);
			return;
		}
	}
	AsicNotSupportFunc(pAd, __func__);
}

VOID AsicFeLossGet(RTMP_ADAPTER *pAd, UCHAR channel, CHAR *RssiOffset)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archFeLossGet)
		arch_ops->archFeLossGet(pAd, channel, RssiOffset);
}

VOID AsicRcpiReset(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	/* Need to add fw cmd to reset rcpi of wtbl if need. */
	AsicNotSupportFunc(pAd, __func__);
}

INT AsicSetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit)
{
#ifdef MT_MAC
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetRetryLimit)
		return arch_ops->archSetRetryLimit(pAd, type, limit);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


UINT32 AsicGetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type)
{
#ifdef MT_MAC
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archGetRetryLimit)
		return arch_ops->archGetRetryLimit(pAd, type);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicSetSlotTime(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bUseShortSlotTime,
	IN UCHAR channel,
	IN struct wifi_dev *wdev)
{
	UINT32 SlotTime = 0;
	UINT32 SifsTime = SIFS_TIME_24G;
	UCHAR BandIdx;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif

	if (bUseShortSlotTime && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED))
		return;
#ifdef CONFIG_STA_SUPPORT
	else if (pStaCfg && (!bUseShortSlotTime) && (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED)))
		return;
#endif

	if (bUseShortSlotTime)
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
	else
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);

	SlotTime = (bUseShortSlotTime) ? 9 : pAd->CommonCfg.SlotTime;
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		UINT8 ba_en;

		if (WMODE_CAP_5G(wdev->PhyMode)) {
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
			bUseShortSlotTime = TRUE;
		}

		SlotTime = (bUseShortSlotTime) ? 9 : 20;

		/* force using short SLOT time for FAE to demo performance when TxBurst is ON*/
		ba_en = wlan_config_get_ba_enable(wdev);
		if (((pStaCfg->StaActive.SupportedPhyInfo.bHtEnable == FALSE) && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED)))
#ifdef DOT11_N_SUPPORT
			|| ((pStaCfg->StaActive.SupportedPhyInfo.bHtEnable == TRUE) && !ba_en)
#endif /* DOT11_N_SUPPORT */
		   ) {
			/* In this case, we will think it is doing Wi-Fi test*/
			/* And we will not set to short slot when bEnableTxBurst is TRUE.*/
		} else if (pAd->CommonCfg.bEnableTxBurst) {
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
			SlotTime = 9;
		}

		/* For some reasons, always set it to short slot time.*/
		/* ToDo: Should consider capability with 11B*/
		if (pStaCfg->BssType == BSS_ADHOC) {
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
			SlotTime = 20;
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	BandIdx = HcGetBandByChannel(pAd, channel);
#ifdef MT_MAC
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		if (arch_ops->archUniCmdSetSlotTime) {
			arch_ops->archUniCmdSetSlotTime(pAd, SlotTime, SifsTime, wdev);
			return;
		}
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (arch_ops->archSetSlotTime) {
			arch_ops->archSetSlotTime(pAd, SlotTime, SifsTime, BandIdx);
			return;
		}
	}
}
#endif
	AsicNotSupportFunc(pAd, __func__);
}


INT AsicSetMacMaxLen(RTMP_ADAPTER *pAd)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetMacMaxLen) {
		INT ret = 0;

		ret = arch_ops->archSetMacMaxLen(pAd);
		return ret;
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


VOID AsicGetTxTsc(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 pn_type_mask, UCHAR *pTxTsc)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archGetTxTsc) {
		arch_ops->archGetTxTsc(pAd, wdev, pn_type_mask, pTxTsc);
		return;
	}
	AsicNotSupportFunc(pAd, __func__);
}

VOID AsicSetSMPS(RTMP_ADAPTER *pAd, UINT16 Wcid, UCHAR smps)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetSMPS)
		arch_ops->archSetSMPS(pAd, Wcid, smps);
	else
		AsicNotSupportFunc(pAd, __func__);
}
static UCHAR *phy_bw_str_txrx_stainfo[] = {"20M", "40M", "80M", "160M", "320M"};
char *get_bw_str_stainfo(int bandwidth, UCHAR bwInfoSrc)
{
	switch (bwInfoSrc) {
	case BW_FROM_TXRX_INFO:
		if (bandwidth >= 0 && bandwidth < ARRAY_SIZE(phy_bw_str_txrx_stainfo))
			return phy_bw_str_txrx_stainfo[bandwidth];
		else if (bandwidth == 7)
			return phy_bw_str_txrx_stainfo[4];
		else
			return "N/A";
	default:
		return "N/A";
	}
}

static UCHAR *phy_bw_str_oid[] = {"20M", "40M", "80M", "160M", "10M", "5M", "80+80", "320M"};
static UCHAR *phy_bw_str_txrx_info[] = {"20M", "40M", "80M", "160M", "320M/BW80_PUNC4", "BW80_PUNC5", "BW160_PUNC6", "BW160_PUNC7"};
char *get_bw_str(int bandwidth, UCHAR bwInfoSrc)
{
	switch (bwInfoSrc) {
	case BW_FROM_TXRX_INFO:
		if (bandwidth >= 0 && bandwidth < ARRAY_SIZE(phy_bw_str_txrx_info))
			return phy_bw_str_txrx_info[bandwidth];
		else
			return "N/A";
	case BW_FROM_OID:
		if (bandwidth >= BW_20 && bandwidth <= BW_320)
			return phy_bw_str_oid[bandwidth];
		else
			return "N/A";
	default:
		return "N/A";
	}
}

VOID AsicTxCapAndRateTableUpdate(RTMP_ADAPTER *pAd, UINT16 u2Wcid, RA_PHY_CFG_T *prTxPhyCfg, UINT32 *Rate, BOOL fgSpeEn)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archTxCapAndRateTableUpdate)
		arch_ops->archTxCapAndRateTableUpdate(pAd, u2Wcid, prTxPhyCfg, Rate, fgSpeEn);
	else
		AsicNotSupportFunc(pAd, __func__);
}


/*
 * ========================================================================
 * Description:
 * Add Shared key information into ASIC.
 * Update shared key, TxMic and RxMic to Asic Shared key table
 * Update its cipherAlg to Asic Shared key Mode.
 *
 * Return:
 * ========================================================================
 */
VOID AsicAddSharedKeyEntry(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			BssIndex,
	IN UCHAR			KeyIdx,
	IN PCIPHER_KEY		pCipherKey)
{
#ifdef MT_MAC
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archAddSharedKeyEntry) {
		arch_ops->archAddSharedKeyEntry(pAd, BssIndex, KeyIdx, pCipherKey);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}


/*	IRQL = DISPATCH_LEVEL*/
VOID AsicRemoveSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 BssIndex,
	IN UCHAR		 KeyIdx)
{
#ifdef MT_MAC
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		arch_ops->archRemoveSharedKeyEntry(pAd, BssIndex, KeyIdx);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}

VOID AsicUpdateBASession(RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR tid, UINT16 sn, UINT16 basize, BOOLEAN isAdd, INT ses_type, UCHAR amsdu)
{
	MAC_TABLE_ENTRY *mac_entry;
	MT_BA_CTRL_T BaCtrl;
	STA_REC_BA_CFG_T StaRecBaCfg;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	os_zero_mem(&BaCtrl, sizeof(MT_BA_CTRL_T));
	mac_entry = entry_get(pAd, wcid);
	BaCtrl.BaSessionType = ses_type;
	BaCtrl.BaWinSize = basize;
	BaCtrl.isAdd = isAdd;
	BaCtrl.Sn = sn;
	BaCtrl.Wcid = wcid;
	BaCtrl.Tid = tid;
	BaCtrl.amsdu = amsdu;

	if (mac_entry && mac_entry->wdev) {
		BaCtrl.band_idx = HcGetBandByWdev(mac_entry->wdev);
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"mac_entry=%p!mac_entry->wdev=%p, Set BaCtrl.band_idx=%d\n",
			mac_entry, mac_entry->wdev, BaCtrl.band_idx);
	} else {
		BaCtrl.band_idx = 0;
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"mac_entry=%p!Set BaCtrl.band_idx=%d\n",
			mac_entry, BaCtrl.band_idx);
	}

	if (ses_type == BA_SESSION_RECP) {
		/* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
		if (isAdd)
			os_move_mem(&BaCtrl.PeerAddr[0], &mac_entry->Addr[0], MAC_ADDR_LEN);
	}

	if (arch_ops->archUpdateBASession) {
		arch_ops->archUpdateBASession(pAd, BaCtrl);

		if (arch_ops->archUpdateStaRecBa) {
			if (!mac_entry  || !mac_entry->wdev)
				return;

			StaRecBaCfg.baDirection = ses_type;
			StaRecBaCfg.sn = sn;
			StaRecBaCfg.ba_wsize = basize;
			StaRecBaCfg.BssIdx = mac_entry->wdev->bss_info_argument.ucBssIndex;
			StaRecBaCfg.MuarIdx = mac_entry->wdev->OmacIdx;
			StaRecBaCfg.tid = tid;
			StaRecBaCfg.BaEnable = (isAdd << tid);
			StaRecBaCfg.WlanIdx = wcid;
			StaRecBaCfg.amsdu = amsdu;
			arch_ops->archUpdateStaRecBa(pAd, StaRecBaCfg);
		}

		return;
	}

	AsicNotSupportFunc(pAd, __func__);
	return;
}

/*
 * ==========================================================================
 * Description:
 *
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicDelWcidTab(RTMP_ADAPTER *pAd, UINT16 wcid_idx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archDelWcidTab)
		return arch_ops->archDelWcidTab(pAd, wcid_idx);

	AsicNotSupportFunc(pAd, __func__);
	return;
}

#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
VOID AsicSetWcid4Addr_HdrTrans(RTMP_ADAPTER *pAd, UINT16 wcid_idx, UCHAR IsEnable)
{
	MAC_TABLE_ENTRY *pEntry;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	UCHAR IsApcliEntry = 0;

	pEntry = entry_get(pAd, wcid_idx);
	if (IS_ENTRY_PEER_AP(pEntry))
		IsApcliEntry = 1;

#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (arch_ops->archSetWcid4Addr_HdrTrans) {

			return arch_ops->archSetWcid4Addr_HdrTrans(pAd, wcid_idx, IsEnable, IsApcliEntry);
		} else {
			AsicNotSupportFunc(pAd, __FUNCTION__);
			return;
		}
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
	return;
}
#endif


#ifdef HTC_DECRYPT_IOT
VOID AsicSetWcidAAD_OM(RTMP_ADAPTER *pAd, UINT16 wcid_idx, CHAR value)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetWcidAAD_OM)
		arch_ops->archSetWcidAAD_OM(pAd, wcid_idx, value);

	AsicNotSupportFunc(pAd, __func__);
	return;
}
#endif /* HTC_DECRYPT_IOT */

VOID AsicSetWcidPsm(RTMP_ADAPTER *pAd, UINT16 wcid_idx, UCHAR value)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetWcidPsm)
		arch_ops->archSetWcidPsm(pAd, wcid_idx, value);
	else
		AsicNotSupportFunc(pAd, __func__);
}

VOID AsicAddRemoveKeyTab(
	IN PRTMP_ADAPTER pAd,
	IN ASIC_SEC_INFO *pInfo)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archAddRemoveKeyTab)
		return arch_ops->archAddRemoveKeyTab(pAd, pInfo);

	AsicNotSupportFunc(pAd, __func__);
}


INT AsicSendCommandToMcu(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


BOOLEAN AsicSendCmdToMcuAndWait(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

#ifdef DOT11_N_SUPPORT
INT AsicReadAggCnt(RTMP_ADAPTER *pAd, ULONG *aggCnt, int cnt_len)
{
	NdisZeroMemory(aggCnt, cnt_len * sizeof(ULONG));
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}
#endif /* DOT11_N_SUPPORT */

INT AsicSetMacTxRx(RTMP_ADAPTER *pAd, INT txrx, BOOLEAN enable)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	INT ret = 0;
	u8 band_idx = hc_get_hw_band_idx(pAd);

	if (arch_ops->archSetMacTxRx) {
		if (band_idx >= CFG_WIFI_RAM_BAND_NUM) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"invalid band index(=%d)\n", band_idx);
			return FALSE;
		}
		ret = arch_ops->archSetMacTxRx(pAd, txrx, enable, band_idx);

		if (ret != 0)
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"SetMacTxRx failed!\n");
		return ret;
	}
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetRxvFilter(RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR ucBandIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	INT ret = 0;

	if (arch_ops->archSetRxvFilter) {
		ret = arch_ops->archSetRxvFilter(pAd, enable, ucBandIdx);

		if (ret != 0) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"SetRxvTxRx failed!\n");
			return ret;
		}

		return ret;
	}
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicSetMacWD(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetMacWD)
		return arch_ops->archSetMacWD(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicSetTxStream(RTMP_ADAPTER *pAd, UINT32 StreamNum, UCHAR opmode, BOOLEAN up, UCHAR BandIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetTxStream) {
		INT Ret;

		Ret = arch_ops->archSetTxStream(pAd, StreamNum, BandIdx);
		return Ret;
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetRxStream(RTMP_ADAPTER *pAd, UINT32 rx_path, UCHAR BandIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetRxStream) {
		INT Ret;

		Ret = arch_ops->archSetRxStream(pAd, rx_path, BandIdx);
		return Ret;
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicSetRxPath(RTMP_ADAPTER *pAd, UINT32 RxPathSel, UCHAR BandIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetRxPath)
		return arch_ops->archSetRxPath(pAd, RxPathSel, BandIdx);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;

}

UINT32 AsicGetRxStat(RTMP_ADAPTER *pAd, UINT type)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archGetRxStat)
		return arch_ops->archGetRxStat(pAd, type);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetCtrlCh(RTMP_ADAPTER *pAd, UINT8 extch)
{
#ifdef MT_MAC
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetCtrlCh)
		return arch_ops->archSetCtrlCh(pAd, extch);

#endif /* MT_MAC */
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

VOID AsicSetTmrCR(RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetTmrCR)
		arch_ops->archSetTmrCR(pAd, enable, BandIdx);
	else
		AsicNotSupportFunc(pAd, __func__);
}

UINT16 asic_tx_rate_to_tmi_rate(struct _RTMP_ADAPTER *pAd, UINT8 mode, UINT8 mcs, UINT8 nss, BOOLEAN stbc, UINT8 preamble)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->tx_rate_to_tmi_rate)
		return arch_ops->tx_rate_to_tmi_rate(mode, mcs, nss, stbc, preamble);
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

UCHAR asic_get_nsts_by_mcs(struct _RTMP_ADAPTER *pAd, UCHAR phy_mode, UCHAR mcs, BOOLEAN stbc, UCHAR vht_nss)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->get_nsts_by_mcs)
		return arch_ops->get_nsts_by_mcs(phy_mode, mcs, stbc, vht_nss);

	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

VOID asic_update_raw_counters(struct _RTMP_ADAPTER *pAd)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->update_raw_counters)
		arch_ops->update_raw_counters(pAd);
	else
		AsicNotSupportFunc(pAd, __func__);
}

VOID asic_update_mib_bucket(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->update_mib_bucket)
		ops->update_mib_bucket(pAd);
	else
		AsicNotSupportFunc(pAd, __func__);
}

#ifdef ZERO_PKT_LOSS_SUPPORT
VOID asic_read_channel_stat_registers(RTMP_ADAPTER *pAd, UINT8 BandIdx, void *ChStat)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->read_channel_stat_registers)
		arch_ops->read_channel_stat_registers(pAd, BandIdx, ChStat);
	else
		AsicNotSupportFunc(pAd, __func__);
}

UINT8 AsicReadSkipTx(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->read_skip_tx)
		return arch_ops->read_skip_tx(pAd, wcid);

	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

VOID AsicUpdateSkipTx(RTMP_ADAPTER *pAd, UINT16 wcid, UINT8 set)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
					"Wcid(%d), Set(%d)\n", wcid, set);

	if (arch_ops->update_skip_tx)
		return arch_ops->update_skip_tx(pAd, wcid, set);

	AsicNotSupportFunc(pAd, __func__);
}
#endif

#ifdef CONFIG_AP_SUPPORT
/* set Wdev Mac Address, some chip arch need to set CR .*/
VOID AsicSetWdevIfAddr(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT opmode)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetWdevIfAddr)
		arch_ops->archSetWdevIfAddr(pAd, wdev, opmode);
	else
		AsicNotSupportFunc(pAd, __func__);
}

/* set Wdev Mac Address, some chip arch need to set CR .*/
VOID AsicSetMbssHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetMbssHwCRSetting)
		arch_ops->archSetMbssHwCRSetting(pAd, mbss_idx, enable);
	else
		AsicNotSupportFunc(pAd, __func__);
}

/* set Wdev Mac Address, some chip arch need to set CR .*/
VOID AsicSetExtMbssEnableCR(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetExtMbssEnableCR)
		arch_ops->archSetExtMbssEnableCR(pAd, mbss_idx, enable);
	else
		AsicNotSupportFunc(pAd, __func__);
}

VOID AsicSetExtTTTTHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetExtTTTTHwCRSetting)
		arch_ops->archSetExtTTTTHwCRSetting(pAd, mbss_idx, enable);
	else
		AsicNotSupportFunc(pAd, __func__);
}
#endif /* CONFIG_AP_SUPPORT */


VOID AsicDMASchedulerInit(RTMP_ADAPTER *pAd, INT mode)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_DMASCH_CTRL_T DmaSchCtrl;

		DmaSchCtrl.bBeaconSpecificGroup = TRUE;
		DmaSchCtrl.mode = mode;
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}

INT32 AsicDevInfoUpdate(
	RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT64 EnableFeature)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
		"Set OwnMac="MACSTR"\n", MAC2STR(OwnMacAddr));

	if (arch_ops->archSetDevMac)
		return arch_ops->archSetDevMac(pAd, OwnMacIdx, OwnMacAddr, BandIdx, Active, EnableFeature);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT32 AsicBssInfoUpdate(
	RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info_argument)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
		"Set Bssid="MACSTR", BssIndex(%d)\n",
		MAC2STR(bss_info_argument->Bssid),
		bss_info_argument->ucBssIndex);

	if (arch_ops->archSetBssid)
		return arch_ops->archSetBssid(pAd, bss_info_argument);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

#ifdef DOT11_EHT_BE
INT32 AsicDscbInfoUpdate(RTMP_ADAPTER *pAd, struct CMD_STATIC_PP_DSCB_T *dscb)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archUpdateDscbInfo)
		return arch_ops->archUpdateDscbInfo(pAd, dscb);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicGetAt2lmRes(
	RTMP_ADAPTER *pAd,
	struct AT2LM_RES_REQ_CTRL_T *req,
	struct AT2LM_RES_RSP_CTRL_T *rsp)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archReqAt2lmRes)
		return arch_ops->archReqAt2lmRes(pAd, req, rsp);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicSetNt2lm(
	RTMP_ADAPTER *pAd,
	struct NT2LM_REQ_CTRL_T *req)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archReqNt2lm)
		return arch_ops->archReqNt2lm(pAd, req);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicReqReconfigRmLink(
	RTMP_ADAPTER *pAd,
	struct RECONFIG_RM_LINK_REQ_CTRL_T *req)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archReqReconfigRmLink)
		return arch_ops->archReqReconfigRmLink(pAd, req);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT32 AsicSetReconfigTmr(
	RTMP_ADAPTER *pAd,
	struct RECONFIG_SET_TMR_CTRL_T *reconfig_ctrl)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetReconfigTmr)
		return arch_ops->archSetReconfigTmr(pAd, reconfig_ctrl);

	AsicNotSupportFunc(pAd, __func__);
	return NDIS_STATUS_FAILURE;
}

INT asic_mgmt_emlsr_update(struct _RTMP_ADAPTER *pAd, MAC_TX_INFO *info, struct _TX_BLK *tx_blk)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archUpdateEmlsrMgmt)
		return arch_ops->archUpdateEmlsrMgmt(pAd, info, tx_blk);

	AsicNotSupportFunc(pAd, __func__);

	return NDIS_STATUS_SUCCESS;
}
#endif

VOID AsicSetTmrCal(struct _RTMP_ADAPTER *pAd, UCHAR TmrType, UCHAR Channel, UCHAR Bw)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetTmrCal)
		return arch_ops->archSetTmrCal(pAd, TmrType, Channel, Bw);

	AsicNotSupportFunc(pAd, __func__);
}

UINT32 asic_get_hwq_from_ac(struct _RTMP_ADAPTER *ad, UCHAR wmm_idx, UCHAR ac)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->archGetHwQFromAc)
		return arch_ops->archGetHwQFromAc(wmm_idx, ac);

	AsicNotSupportFunc(ad, __func__);
	return 0;
}


VOID AsicTOPInit(struct _RTMP_ADAPTER *pAd)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archTOPInit)
		arch_ops->archTOPInit(pAd);
	else
		AsicNotSupportFunc(pAd, __func__);
}


INT32 AsicExtPwrMgtBitWifi(RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT8 ucPwrMgtBit)
{
	MT_PWR_MGT_BIT_WIFI_T rPwtMgtBitWiFi = {0};

	rPwtMgtBitWiFi.u2WlanIdx = u2WlanIdx;
	rPwtMgtBitWiFi.ucPwrMgtBit = ucPwrMgtBit;
	return MtCmdExtPwrMgtBitWifi(pAd, rPwtMgtBitWiFi);
}

INT32 AsicStaRecUpdate(
	RTMP_ADAPTER *pAd,
	STA_REC_CTRL_T *sta_rec_ctrl)
{
	UINT16 WlanIdx = sta_rec_ctrl->WlanIdx;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
#if defined(DOT11_HE_AX) && defined(FIXED_HE_GI_SUPPORT)
	UCHAR gi_idx = 0;
	UCHAR phy_mode = 0;
#endif

	if (arch_ops->archSetStaRec) {
		STA_REC_CFG_T StaCfg;
		PMAC_TABLE_ENTRY pEntry = NULL;
		INT32 ret = 0;
		struct _STA_TR_ENTRY *tr_entry;

		os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));

		tr_entry = tr_entry_get(pAd, WlanIdx);
		/* Need to provide H/W BC/MC WLAN index to CR4 */
		if (tr_entry->EntryType == ENTRY_CAT_MCAST)
			pEntry = NULL;
		else {
			pEntry	= entry_get(pAd, WlanIdx);
#ifdef SW_CONNECT_SUPPORT
			if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
				STA_TR_ENTRY *sw_tr_entry;

				sw_tr_entry = tr_entry_get(pAd, pEntry->tr_tb_idx);
				/*
				*  Fix Crash on BF use WlanIdx as S/W
				*  actually S/W Entry can't handle BF, so skip BF cmds in S/W Entry
				*/
				if (IS_SW_MAIN_STA(sw_tr_entry) || IS_SW_STA(sw_tr_entry))
					sta_rec_ctrl->EnableFeature &= ~(STA_REC_BF_FEATURE | STA_REC_BFEE_FEATURE);

				/* unicast replace the real H/W WCID */
				if (IS_SW_STA(sw_tr_entry)) {
					WlanIdx = sw_tr_entry->HwWcid;
					pEntry = entry_get(pAd, WlanIdx);
				}
			}
#endif /* SW_CONNECT_SUPPORT */
		}

#ifdef SW_CONNECT_SUPPORT
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
			"Wcid(%d), HwWcid(%d), u8EnableFeature(0x%llx)\n",
			sta_rec_ctrl->WlanIdx, WlanIdx, sta_rec_ctrl->EnableFeature);
#else
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
			"Wcid(%d), u8EnableFeature(0x%llx)\n",
			sta_rec_ctrl->WlanIdx, sta_rec_ctrl->EnableFeature);
#endif /* SW_CONNECT_SUPPORT */

		if (pEntry && !IS_ENTRY_NONE(pEntry)) {
			if (!pEntry->wdev) {
				ASSERT(pEntry->wdev);
				return -1;
			}

			StaCfg.MuarIdx = pEntry->wdev->OmacIdx;
		} else {
			StaCfg.MuarIdx = 0xe;/* TODO: Carter, check this on TX_HDR_TRANS */
		}

#ifdef TXBF_SUPPORT
		if (pEntry && !IS_ENTRY_NONE(pEntry)
			&& (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))) {
			UINT8 ucTxPath = pAd->Antenna.field.TxPath;

#ifdef ANTENNA_CONTROL_SUPPORT
			{
				if (pAd->bAntennaSetAPEnable)
					ucTxPath = pAd->TxStream;
			}
#endif /* ANTENNA_CONTROL_SUPPORT */

			if (bf_is_support(pEntry->wdev) == TRUE) {
				if (sta_rec_ctrl->EnableFeature & STA_REC_BF_FEATURE) {
					if (ucTxPath > 1)
						AsicBfStaRecUpdate(pAd, pEntry->wdev->PhyMode, sta_rec_ctrl->BssIndex, WlanIdx);
				}

				if (sta_rec_ctrl->EnableFeature & STA_REC_BFEE_FEATURE) {
					AsicBfeeStaRecUpdate(pAd, pEntry->wdev->PhyMode, sta_rec_ctrl->BssIndex, WlanIdx);
				}
			}
		}

#endif /* TXBF_SUPPORT */

#ifdef DOT11_HE_AX
		if (sta_rec_ctrl->EnableFeature
			& (STA_REC_BASIC_HE_INFO_FEATURE | STA_REC_HE_BASIC_FEATURE))
			os_move_mem(&StaCfg.he_sta, &sta_rec_ctrl->he_sta, sizeof(sta_rec_ctrl->he_sta));
#endif
		StaCfg.ConnectionState = sta_rec_ctrl->ConnectionState;
		StaCfg.ConnectionType = sta_rec_ctrl->ConnectionType;
		StaCfg.u8EnableFeature = sta_rec_ctrl->EnableFeature;
		StaCfg.ucBssIndex = sta_rec_ctrl->BssIndex;
		StaCfg.u2WlanIdx = WlanIdx;
		if (sta_rec_ctrl->EnableFeature
			& (STA_REC_BASIC_STA_RECORD_FEATURE & ~STA_REC_BASIC_STA_RECORD_BMC_FEATURE))
			pAd->u2BMCIdx = StaCfg.u2WlanIdx;

		StaCfg.pEntry = pEntry;
#ifdef CONFIG_6G_SUPPORT
		if ((sta_rec_ctrl->update_ra == TRUE)
			&& (tr_entry->EntryType == ENTRY_CAT_MCAST))
			StaCfg.pEntry = entry_get(pAd, WlanIdx);
#endif /* CONFIG_6G_SUPPORT */
		StaCfg.IsNewSTARec = sta_rec_ctrl->IsNewSTARec;
		os_move_mem(&StaCfg.asic_sec_info, &sta_rec_ctrl->asic_sec_info, sizeof(ASIC_SEC_INFO));
		ret = arch_ops->archSetStaRec(pAd, &StaCfg);
#if defined(DOT11_HE_AX) && defined(FIXED_HE_GI_SUPPORT)
		if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->wdev)) {
			gi_idx = wlan_config_get_he_gi(pEntry->wdev);
			phy_mode = pEntry->MaxHTPhyMode.field.MODE;
			if ((gi_idx != GI_AUTO) && ((phy_mode >= MODE_HE) && (phy_mode != MODE_UNKNOWN)))
				ap_set_he_fixed_gi_ltf_by_wcid_or_bss(pAd, gi_idx-1, GI_BY_WCID, pEntry->wcid, pEntry->wdev);
		}
#endif
		return ret;
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


#ifdef MT_MAC
INT32 AsicRaParamStaRecUpdate(
	RTMP_ADAPTER *pAd,
	UINT16 WlanIdx,
	P_CMD_STAREC_AUTO_RATE_UPDATE_T prParam,
	UINT64 EnableFeature)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archSetStaRec) {
		STA_REC_CFG_T StaCfg;
		PMAC_TABLE_ENTRY pEntry = NULL;

		os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));

		/* Need to provide H/W BC/MC WLAN index to CR4 */
		if (!VALID_UCAST_ENTRY_WCID(pAd, WlanIdx))
			pEntry = NULL;
		else
			pEntry	= entry_get(pAd, WlanIdx);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
			"Wcid(%d), u8EnableFeature(0x%llx)\n",
			WlanIdx, EnableFeature);

		if (pEntry && !IS_ENTRY_NONE(pEntry)) {
			if (!pEntry->wdev) {
				ASSERT(pEntry->wdev);
				return -1;
			}

			StaCfg.MuarIdx = pEntry->wdev->OmacIdx;

			StaCfg.ucBssIndex = pEntry->wdev->bss_info_argument.ucBssIndex;
		} else {
			StaCfg.MuarIdx = 0xe;/* TODO: Carter, check this on TX_HDR_TRANS */
		}

		StaCfg.ConnectionState = STATE_CONNECTED;
		StaCfg.u8EnableFeature = EnableFeature;
		StaCfg.u2WlanIdx = WlanIdx;
		StaCfg.pEntry = pEntry;
		StaCfg.pRaParam = prParam;
		/*tracking the starec input history*/
		return arch_ops->archSetStaRec(pAd, &StaCfg);
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}
#endif /* MT_MAC */

VOID AsicTurnOffRFClk(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	AsicNotSupportFunc(pAd, __func__);
}

INT32 AsicRadioOnOffCtrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 ucRadio)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_PMSTAT_CTRL_T PmStatCtrl = {0};
#ifdef WIFI_UNIFIED_COMMAND
		RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

		PmStatCtrl.PmNumber = PM5;
		PmStatCtrl.DbdcIdx = ucDbdcIdx;

		if (ucRadio == WIFI_RADIO_ON) {
			PmStatCtrl.PmState = EXIT_PM_STATE;
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_WARN,
					"DbdcIdx=%d RadioOn\n",
					 ucDbdcIdx);
		} else {
			PmStatCtrl.PmState = ENTER_PM_STATE;
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_WARN,
					"DbdcIdx=%d RadioOff\n",
					 ucDbdcIdx);
		}
#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support)
			return UniCmdRadioOnOff(pAd, PmStatCtrl);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			return MtCmdExtPmStateCtrl(pAd, PmStatCtrl);
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

#ifdef GREENAP_SUPPORT
INT32 AsicGreenAPOnOffCtrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN ucGreenAPOn)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_GREENAP_CTRL_T GreenAPCtrl = {0};
#ifdef WIFI_UNIFIED_COMMAND
		RTMP_CHIP_CAP * pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

		GreenAPCtrl.ucDbdcIdx = ucDbdcIdx;
		GreenAPCtrl.ucGreenAPOn = ucGreenAPOn;
#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support)
			return UniCmdGreenAPOnOffCtrl(pAd, GreenAPCtrl);
#endif /* WIFI_UNIFIED_COMMAND */
		return MtCmdExtGreenAPOnOffCtrl(pAd, GreenAPCtrl);
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}
#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
INT32 asic_pcie_aspm_dym_ctrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN fgL1Enable, BOOLEAN fgL0sEnable)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_PCIE_ASPM_DYM_CTRL_T mt_pcie_aspm_dym_ctrl = {0};

		mt_pcie_aspm_dym_ctrl.ucDbdcIdx = ucDbdcIdx;
		mt_pcie_aspm_dym_ctrl.fgL1Enable = fgL1Enable;
		mt_pcie_aspm_dym_ctrl.fgL0sEnable = fgL0sEnable;
		return  mt_cmd_ext_pcie_aspm_dym_ctrl(pAd, mt_pcie_aspm_dym_ctrl);
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
/* hw ctrl cmd --> middle --> mt cmd */
INT32 asic_twt_agrt_update(struct _RTMP_ADAPTER *ad, struct TWT_AGRT_PARA_T *agrt_para)
{
#ifdef MT_MAC
	if (IS_HIF_TYPE(ad, HIF_MT)) {
		struct MT_TWT_AGRT_PARA_T _agrt_para = {0};
#ifdef WIFI_UNIFIED_COMMAND
		RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

		os_move_mem((UINT8 *)&_agrt_para,
			(UINT8 *)agrt_para,
			sizeof(struct TWT_AGRT_PARA_T));
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			return UniCmdTwtAgrtUpdate(ad, &_agrt_para);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			return mt_cmd_ext_twt_agrt_update(ad, &_agrt_para);
	}

#endif
	AsicNotSupportFunc(ad, __func__);
	return 0;
}

INT32 asic_twt_agrt_mgmt(struct _RTMP_ADAPTER *ad, struct TWT_AGRT_PARA_T *agrt_para, struct TWT_AGRT_MGMT_T *agrt_mgmt)
{
	INT32 ret = NDIS_STATUS_FAILURE;
#ifdef MT_MAC
	if (IS_HIF_TYPE(ad, HIF_MT)) {
#ifdef WIFI_UNIFIED_COMMAND
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
		struct MT_TWT_AGRT_PARA_T _agrt_para = {0};
		struct MT_TWT_AGRT_MGMT_T _agrt_mgmt = {0};

		if (!agrt_para)
			return ret;

		os_move_mem((UINT8 *)&_agrt_para,
			(UINT8 *)agrt_para,
			sizeof(struct TWT_AGRT_PARA_T));

		if (cap->uni_cmd_support) {
			ret = UniCmdTwtAgrtMgmt(ad, &_agrt_para, &_agrt_mgmt);
			if (ret == NDIS_STATUS_SUCCESS) {
				if (agrt_mgmt)
					os_move_mem((UINT8 *)agrt_mgmt,
						(UINT8 *)&_agrt_mgmt,
						sizeof(struct MT_TWT_AGRT_MGMT_T));
			}
			return ret;
		}
#endif /* WIFI_UNIFIED_COMMAND */
	}

#endif
	AsicNotSupportFunc(ad, __func__);
	return ret;
}

INT32 asic_twt_mgmt_frame_offload(struct _RTMP_ADAPTER *ad, struct MGMT_FRAME_OFFLOAD_T *offload_para)
{
	INT ret = 0;
#ifdef MT_MAC
	if (IS_HIF_TYPE(ad, HIF_MT)) {
#ifdef WIFI_UNIFIED_COMMAND
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
		struct MT_MGMT_FRAME_OFFLOAD_T _offload_para = {0};

		os_move_mem((UINT8 *)&_offload_para,
			(UINT8 *)offload_para,
			sizeof(struct MGMT_FRAME_OFFLOAD_T));

		if (cap->uni_cmd_support) {
			ret = UniCmdTwtMgmtFrameOffload(ad, &_offload_para);

			return ret;
		}
#endif /* WIFI_UNIFIED_COMMAND */
	}

#endif /* MT_MAC */

	AsicNotSupportFunc(ad, __func__);
	return ret;
}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

#ifdef CONFIG_STA_SUPPORT
INT32 AsicExtPmStateCtrl(
	RTMP_ADAPTER *pAd,
	PSTA_ADMIN_CONFIG pStaCfg,
	UINT8 ucPmNumber,
	UINT8 ucPmState)
{
	struct wifi_dev *wdev = NULL;

	wdev = &pStaCfg->wdev;
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PS, CATPS_LP, DBG_LVL_ERROR,
			"wdev(NULL) for PM=%d, State=%d\n", ucPmNumber, ucPmState);
		return 0;
	}

	if (wdev->wdev_type != WDEV_TYPE_STA) {
		MTWF_DBG(pAd, DBG_CAT_PS, CATPS_LP, DBG_LVL_ERROR,
			"wdev_type(0x%x)!=STA,return!\n", wdev->wdev_type);
		return 0;
	}

#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_PMSTAT_CTRL_T PmStatCtrl = {0};
#ifdef WIFI_UNIFIED_COMMAND
		RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

		PmStatCtrl.PmNumber = ucPmNumber;
		PmStatCtrl.PmState = ucPmState;

		if (ucPmNumber == PM4) {
			PmStatCtrl.DbdcIdx = HcGetBandByWdev(wdev);
			PmStatCtrl.WlanIdx = pStaCfg->MacTabWCID;
			PmStatCtrl.Aid = pStaCfg->StaActive.Aid;
			PmStatCtrl.BcnInterval = pStaCfg->BeaconPeriod;
			PmStatCtrl.DtimPeriod = pStaCfg->DtimPeriod;
			PmStatCtrl.BcnLossCount = BEACON_OFFLOAD_LOST_TIME;
			NdisCopyMemory(PmStatCtrl.Bssid, pStaCfg->Bssid, MAC_ADDR_LEN);
			PmStatCtrl.OwnMacIdx = wdev->OmacIdx;
			PmStatCtrl.WmmIdx = HcGetWmmIdx(pAd, wdev);
		}
#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support)
			return MtUniCmdPmStateCtrl(pAd, PmStatCtrl);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			return MtCmdExtPmStateCtrl(pAd, PmStatCtrl);
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}
#endif /* CONFIG_STA_SUPPORT */

INT32 AsicExtWifiHifCtrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 PmStatCtrl, VOID *pReslt)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
#ifdef WIFI_UNIFIED_COMMAND
		RTMP_CHIP_CAP * pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

		if (pChipCap->uni_cmd_support)
			return UniCmdWifiHifCtrl(pAd, ucDbdcIdx, PmStatCtrl, pReslt);
#endif /* WIFI_UNIFIED_COMMAND */
		return MtCmdWifiHifCtrl(pAd, ucDbdcIdx, PmStatCtrl, pReslt);
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}


INT32
AsicThermalProtect(
	RTMP_ADAPTER *pAd,
	UINT8 ucBand,
	UINT8 HighEn,
	CHAR HighTempTh,
	UINT8 LowEn,
	CHAR LowTempTh,
	UINT32 RechkTimer,
	UINT8 RFOffEn,
	CHAR RFOffTh,
	UINT8 ucType)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		INT32 ret = 0;

		ret = MtCmdThermalProtect(pAd, ucBand, HighEn, HighTempTh, LowEn, LowTempTh, RechkTimer, RFOffEn, RFOffTh, ucType);

		return ret;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}


INT32
AsicThermalProtectAdmitDuty(
	RTMP_ADAPTER *pAd,
	UINT8 ucBand,
	UINT32 u4Lv0Duty,
	UINT32 u4Lv1Duty,
	UINT32 u4Lv2Duty,
	UINT32 u4Lv3Duty
)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		INT32 ret = 0;

		ret = MtCmdThermalProtectAdmitDuty(pAd, ucBand, u4Lv0Duty, u4Lv1Duty, u4Lv2Duty, u4Lv3Duty);
		return ret;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

INT AsicThermalProtectAdmitDutyInfo(
	IN PRTMP_ADAPTER	pAd
)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		BOOLEAN  fgStatus = FALSE;

		fgStatus = MtCmdThermalProtectAdmitDutyInfo(pAd);

		return fgStatus;
    }

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;

}

INT32 AsicGetAntMode(RTMP_ADAPTER *pAd, UCHAR *AntMode)
{
	INT32 ret;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archGetAntMode) {
		ret = arch_ops->archGetAntMode(pAd, AntMode);
		return ret;
	}
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

INT32 AsicRxHeaderTransCtl(RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan,
						   BOOLEAN SwPcP)
{
	INT32 ret = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archRxHeaderTransCtl)
		ret = arch_ops->archRxHeaderTransCtl(pAd, En, ChkBssid, InSVlan, RmVlan, SwPcP);

	return ret;
}

INT32 AsicRxHeaderTaranBLCtl(RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType)
{
	INT32 ret = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archRxHeaderTaranBLCtl)
		ret = arch_ops->archRxHeaderTaranBLCtl(pAd, Index, En, EthType);

	return ret;
}

#ifdef VLAN_SUPPORT
INT32 asic_update_vlan_id(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT8 omac_idx, UINT16 vid)
{
	INT32 ret = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->update_vlan_id)
		ret = arch_ops->update_vlan_id(ad, band_idx, omac_idx, vid);

	return ret;
}

INT32 asic_update_vlan_priority(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT8 omac_idx, UINT8 priority)
{
	INT32 ret = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->update_vlan_priority)
		ret = arch_ops->update_vlan_priority(ad, band_idx, omac_idx, priority);

	return ret;
}
#endif

#ifdef IGMP_SNOOP_SUPPORT
BOOLEAN AsicMcastEntryInsert(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, UINT8 Type, PUCHAR MemberAddr,
							 PNET_DEV dev, UINT16 wcid)
{
	INT32 Ret = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archMcastEntryInsert)
		Ret = arch_ops->archMcastEntryInsert(pAd, GrpAddr, BssIdx, Type, MemberAddr, dev, wcid);

	return Ret;
}


BOOLEAN AsicMcastEntryDelete(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, PUCHAR MemberAddr, PNET_DEV dev,
							 UINT16 wcid)
{
	INT32 Ret = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archMcastEntryDelete)
		Ret = arch_ops->archMcastEntryDelete(pAd, GrpAddr, BssIdx, MemberAddr, dev, wcid);

	return Ret;
}

#ifdef IGMP_SNOOPING_DENY_LIST
BOOLEAN AsicMcastEntryDenyList(struct _RTMP_ADAPTER *pAd, UINT8 BssIdx,
							UINT8 entry_cnt, UINT8 add_to_list, UINT8 *pAddr, UINT8 *Prefix_list)
{
	INT32 Ret = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archMcastEntryDenyList)
		Ret = arch_ops->archMcastEntryDenyList(pAd, BssIdx, entry_cnt, add_to_list, pAddr, Prefix_list);

	return Ret;
}
#endif

#ifdef IGMP_TVM_SUPPORT
BOOLEAN AsicMcastConfigAgeOut(RTMP_ADAPTER *pAd, UINT8 AgeOutTime, UINT8 omac_idx)
{
	INT32 Ret = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archMcastConfigAgeout)
		Ret = arch_ops->archMcastConfigAgeout(pAd, AgeOutTime, omac_idx);

	return Ret;
}

BOOLEAN AsicMcastGetMcastTable(RTMP_ADAPTER *pAd, UINT8 ucOwnMacIdx, struct wifi_dev *wdev)
{
	INT32 Ret = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	if (arch_ops->archMcastGetMcastTable)
		Ret = arch_ops->archMcastGetMcastTable(pAd, ucOwnMacIdx, wdev);

	return Ret;
}

#endif /* IGMP_TVM_SUPPORT*/

#endif

#ifdef DOT11_VHT_AC
INT AsicSetRtsSignalTA(RTMP_ADAPTER *pAd, UCHAR bw_sig)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	u8 band_idx = hc_get_hw_band_idx(pAd);

	if (arch_ops->archSetRtsSignalTA) {
		if (band_idx >= CFG_WIFI_RAM_BAND_NUM) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"invalid band index(=%d)\n", band_idx);
			return FALSE;
		}
		arch_ops->archSetRtsSignalTA(pAd, band_idx, bw_sig);
	}
	return TRUE;
}
#endif /*DOT11_VHT_AC*/

VOID RssiUpdate(RTMP_ADAPTER *pAd)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT16 i = 0, j = 0, k = 0, entry_client_num = 0, valid_entry_num = 0;
	INT16 total_rssi[MAX_RSSI_LEN];
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
#endif /* CONFIG_STA_SUPPORT */
	P_RSSI_PAIR pRssiPair;
	UINT16 max_sta_num = GET_MAX_UCAST_NUM(pAd);
#ifdef SW_CONNECT_SUPPORT
	STA_TR_ENTRY *tr_entry = NULL;
#endif /* SW_CONNECT_SUPPORT */

	if (pAd->MacTab->Size == 0)
		return;
	/* init total rssi variable */
	NdisZeroMemory(total_rssi, sizeof(INT16) * MAX_RSSI_LEN);
	os_alloc_mem(pAd, (UCHAR **)&pRssiPair, sizeof(RSSI_PAIR) * MAX_INBAND_WTBL_NUM);
	if (pRssiPair == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Allocate memory failed!\n");
		return;
	}
	NdisZeroMemory(pRssiPair, sizeof(RSSI_PAIR) * MAX_INBAND_WTBL_NUM);

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = entry_get(pAd, i);

		if (IS_VALID_ENTRY(pEntry)) {
#ifdef SW_CONNECT_SUPPORT
			/* TODO : SW STA RSSI GET */
			tr_entry = tr_entry_get(pAd, i);
			if (IS_SW_STA(tr_entry))
				continue;
#endif /* SW_CONNECT_SUPPORT */
			pRssiPair[valid_entry_num++].u2WlanIdx = pEntry->wcid;
		}
		if (((valid_entry_num == MAX_INBAND_WTBL_NUM) || (i == (max_sta_num - 1))) && (valid_entry_num > 0)) {
			MtCmdMultiRssi(pAd, pRssiPair, valid_entry_num);
			for (k = 0; k < valid_entry_num; k++) {
				pEntry = entry_get(pAd, pRssiPair[k].u2WlanIdx);

				/* AP */
				if (IS_ENTRY_CLIENT(pEntry)) {
					entry_client_num++;
					for (j = 0; j < RX_STREAM_PATH_SINGLE_MODE; j++) {
						pEntry->RssiSample.AvgRssi[j] = pRssiPair[k].rssi[j];
						pEntry->RssiSample.AckRssi[j] = pRssiPair[k].rssi[j];
						pEntry->RssiSample.LastRssi[j] = pRssiPair[k].rssi[j];
						total_rssi[j] += pRssiPair[k].rssi[j];
					}
				}

				if (!pEntry->wdev)
					continue;
#ifdef WDS_SUPPORT
				/* WDS */
				if (IS_ENTRY_WDS(pEntry)) {
					for (j = 0; j < RX_STREAM_PATH_SINGLE_MODE; j++) {
						pEntry->RssiSample.AvgRssi[j] = pRssiPair[k].rssi[j];
						pEntry->RssiSample.AckRssi[j] = pRssiPair[k].rssi[j];
						pEntry->RssiSample.LastRssi[j] = pRssiPair[k].rssi[j];
					}
				}
#endif	/* WDS_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
				/* STA or APCLI */
				if (pEntry->wdev->wdev_type == WDEV_TYPE_STA) {
					for (j = 0; j < RX_STREAM_PATH_SINGLE_MODE; j++) {
						pEntry->RssiSample.AvgRssi[j] = pRssiPair[k].rssi[j];
						pEntry->RssiSample.AckRssi[j] = pRssiPair[k].rssi[j];
						pEntry->RssiSample.LastRssi[j] = pRssiPair[k].rssi[j];
						pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);
						if (pStaCfg) {
							pStaCfg->RssiSample.AvgRssi[j] = pRssiPair[k].rssi[j];
							pStaCfg->RssiSample.LastRssi[j] = pRssiPair[k].rssi[j];
						}
					}
				}
#endif /* CONFIG_STA_SUPPORT */
				/* STA or Repeater */
				if (pEntry->wdev->wdev_type == WDEV_TYPE_REPEATER) {
					for (j = 0; j < RX_STREAM_PATH_SINGLE_MODE; j++) {
						pEntry->RssiSample.AvgRssi[j] = pRssiPair[k].rssi[j];
						pEntry->RssiSample.AckRssi[j] = pRssiPair[k].rssi[j];
						pEntry->RssiSample.LastRssi[j] = pRssiPair[k].rssi[j];
					}
				}
				pEntry->RssiSample.Rssi_Updated = TRUE;
			}
			valid_entry_num = 0;
		}
	}
	os_free_mem(pRssiPair);

#ifdef CONFIG_AP_SUPPORT
	/* AP */
	for (i = 0; i < RX_STREAM_PATH_SINGLE_MODE; i++) {
		if (entry_client_num == 0)
			break;

		pAd->ApCfg.RssiSample.LastRssi[i] = total_rssi[i] / entry_client_num;
		pAd->ApCfg.RssiSample.AvgRssi[i] = pAd->ApCfg.RssiSample.LastRssi[i];
	}
#endif /* CONFIG_AP_SUPPORT */
}

UINT32 rtmp_get_rssi(RTMP_ADAPTER *pAd, UINT16 Wcid, CHAR *rssi, UINT8 rssi_len)
{
	UINT16 i = 0;
	MAC_TABLE_ENTRY *pEntry = NULL, *pEntryTmp = NULL;
	BOOLEAN entry_found = FALSE;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
#endif /* CONFIG_STA_SUPPORT */

	/* search mac entry list */
	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntryTmp = entry_get(pAd, i);
		if (IS_VALID_ENTRY(pEntryTmp)) {
			if (pEntryTmp->wcid == Wcid) {
				pEntry = pEntryTmp;
				entry_found = TRUE;
				break;
			}
		}
	}

	/* check entry valid or not */
	if (!entry_found)
		return 1;

#ifdef CONFIG_STA_SUPPORT
	/* STA or APCLI or Repeater */
	if (pEntry->wdev) {
		if ((pEntry->wdev->wdev_type == WDEV_TYPE_STA) || (pEntry->wdev->wdev_type == WDEV_TYPE_REPEATER)) {
			/* check traffic status */
			if ((pEntry->AvgRxBytes != 0) || (pEntry->AvgTxBytes != 0))
				NdisMoveMemory(rssi, pEntry->RssiSample.AckRssi, sizeof(CHAR) * rssi_len);
			else {
				pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);
				if (pStaCfg)
					NdisMoveMemory(rssi, pStaCfg->BcnRssiAvg, sizeof(CHAR) * rssi_len);
				else {
					for (i = 0; i < rssi_len; i++)
						rssi[i] = -127;
				}
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	/* AP */
	if (IS_ENTRY_CLIENT(pEntry))
		NdisMoveMemory(rssi, pEntry->RssiSample.AckRssi, sizeof(CHAR) * rssi_len);

	return 0;
}

/* end Trace for every 100 ms */
#ifdef ETSI_RX_BLOCKER_SUPPORT
VOID CheckRssi(RTMP_ADAPTER *pAd)
{
	UINT8   u1MaxWRssiIdx;
	UINT8   u1WFBitMap	   = BITMAP_WF_ALL;
	CHAR	c1MaxWbRssi	= MINIMUM_POWER_VALUE;
	UINT32	u4WbRssi	   = 0;
	UINT8	u1CheckIdx;
	UINT32  u4DcrfCr = 0;
	UCHAR   u1BandIdx = 0;

	switch (pAd->u1RxBlockerState) {
	case ETSI_RXBLOCKER4R:

		/* Enable DCRF tracking */
		PHY_IO_READ32(pAd->hdev_ctrl, DCRF_TRACK, &u4DcrfCr);
		u4DcrfCr &= ~(BITS(28, 29));
		u4DcrfCr |= ((0x3 << 28) & BITS(28, 29)); /*Enable DCRF*/
		PHY_IO_WRITE32(pAd->hdev_ctrl, DCRF_TRACK, u4DcrfCr);


		/* confidence count check for 1R transition */
		for (u1CheckIdx = 0; u1CheckIdx < pAd->u1To1RCheckCnt; u1CheckIdx++) {
			/* update Max WBRSSI index */
			u1MaxWRssiIdx = ETSIWbRssiCheck(pAd);

			/* log check Max Rssi Index or not found */
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"----------------------------------------------------------------------\n");
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			" i1MaxWRssiIdxPrev: %x\n", pAd->i1MaxWRssiIdxPrev);
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			" u1MaxWRssiIdx: %x\n", u1MaxWRssiIdx);
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"----------------------------------------------------------------------\n");
			/* ---------------- */

			/* not found Max WBRSSI Index */
			if (u1MaxWRssiIdx == 0xFF) {
				pAd->u1ValidCnt = 0;
				pAd->i1MaxWRssiIdxPrev = 0xFF;
			}
			/* confidence count increment to 1R state */
			else if (pAd->i1MaxWRssiIdxPrev == u1MaxWRssiIdx) {
				pAd->u1ValidCnt++;
				MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"--------------------------------------------------------------\n");
				MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				" Same index: u1ValidCnt: %d\n", pAd->u1ValidCnt);
				MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"--------------------------------------------------------------\n");
			}
			/* Max WBRSSI index changed */
			else {
				pAd->u1ValidCnt = 1;
				pAd->i1MaxWRssiIdxPrev = u1MaxWRssiIdx;
				MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"--------------------------------------------------------------\n");
				MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				" Different index: u1ValidCnt: %d\n", pAd->u1ValidCnt);
				MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"--------------------------------------------------------------\n");
			}

			/* confidence count check */
			if (pAd->u1ValidCnt >= pAd->u2To1RvaildCntTH) {
				/* config Rx index according to bitmap */
				switch (u1MaxWRssiIdx) {
				case 0:
					u1WFBitMap = BITMAP_WF0;
					break;
				case 1:
					u1WFBitMap = BITMAP_WF1;
					break;
				case 2:
					u1WFBitMap = BITMAP_WF2;
					break;
				case 3:
					u1WFBitMap = BITMAP_WF3;
					break;
				default:
					break;
				}

				/* config Rx */
				MtCmdLinkTestRxCtrl(pAd, u1WFBitMap, u1BandIdx);

				/* reset confidence count */
				pAd->u1ValidCnt = 0;
				/* update state */
				pAd->u1RxBlockerState = ETSI_RXBLOCKER1R;
				/* break out for loop */
				break;
			}
		}
		break;

	case ETSI_RXBLOCKER1R:

	/* Disable DCRF tracking */
	PHY_IO_READ32(pAd->hdev_ctrl, DCRF_TRACK, &u4DcrfCr);
	u4DcrfCr &= ~(BITS(28, 29));
	u4DcrfCr |= ((0x0 << 28) & BITS(28, 29));/*Disable DCRF*/
	PHY_IO_WRITE32(pAd->hdev_ctrl, DCRF_TRACK, u4DcrfCr);

	/* log for check Rssi Read (WBRSSI/IBRSSI) */
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
	"----------------------------------(1R State)-----------------------------------\n");
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
	" c1MaxWbRssi: %x\n", c1MaxWbRssi&0xFF);
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
	"-----------------------------------------------------------------------------\n");
	/* ---------------- */

		/* CR risk - no expected 0x80 value on WF0/WF1 ; WF2/WF3  */
		if ((c1MaxWbRssi&0xFF) == 0x80) {

			pAd->u1RxBlockerState = ETSI_RXBLOCKER1R;

		}
		/* No CR risk */
		else {

			/* check whether back to 4R mode */
			if (c1MaxWbRssi < pAd->c1WBRssiTh4R) {
				/* CR risk - Protect unexpected value */
				if (pAd->u14RValidCnt >= pAd->u2To4RvaildCntTH) {

					MtCmdLinkTestRxCtrl(pAd, BITMAP_WF_ALL, u1BandIdx);
					/* update state */
					pAd->u1RxBlockerState = ETSI_RXBLOCKER4R;
					pAd->u14RValidCnt = 1;

					/* log for check Rssi Read (WBRSSI/IBRSSI) */
					MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
					"----------------------------------(TO 4R State)",
					"-------------------------------\n");
					MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
					" u14RValidCnt: %d\n", pAd->u14RValidCnt);
					MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
					"-----------------------------------------------",
					"------------------------------\n");
					/* ---------------- */
				} else {
					pAd->u1RxBlockerState = ETSI_RXBLOCKER1R;
					pAd->u14RValidCnt++;
					/* log for check Rssi Read (WBRSSI/IBRSSI) */
					MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
					"----------------------------------(Keep 1R State)",
					"-------------------------------\n");
					MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
					" c1MaxWbRssi: %d, c1WBRssiTh4R: %d\n",
					c1MaxWbRssi, pAd->c1WBRssiTh4R);
					MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
					" CR risk!! u14RValidCnt: %d\n", pAd->u14RValidCnt);
					MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
					"-------------------------------------------------",
					"----------------------------\n");
					/* ---------------- */
				}

			} else
				pAd->u1RxBlockerState = ETSI_RXBLOCKER1R;
		}

		break;
	default:
		break;
	}
}
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */

#define RTS_NUM_DIS_VALUE 0xff
#define RTS_LEN_DIS_VALUE 0xffffff
INT asic_rts_on_off(struct wifi_dev *wdev, BOOLEAN rts_en)
{
	struct _RTMP_ADAPTER *ad;
	UCHAR band_idx;
	UINT32 rts_num;
	UINT32 rts_len;
	struct _RTMP_ARCH_OP *arch_ops;

	if (!wdev)
		return 0;

	ad = wdev->sys_handle;
	band_idx = HcGetBandByWdev(wdev);
	arch_ops = hc_get_arch_ops(ad->hdev_ctrl);


	if (arch_ops->asic_rts_on_off) {
		if (rts_en) {
			rts_num = wlan_operate_get_rts_pkt_thld(wdev);
			rts_len = wlan_operate_get_rts_len_thld(wdev);
		} else {
			rts_num = RTS_NUM_DIS_VALUE;
			rts_len = RTS_LEN_DIS_VALUE;
		}
		return arch_ops->asic_rts_on_off(ad, band_idx, rts_num, rts_len, rts_en);
	}

	AsicNotSupportFunc(ad, __func__);
	return 0;
}

INT asic_set_agglimit(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UCHAR ac, struct wifi_dev *wdev, UINT32 agg_limit)
{
	struct _RTMP_ARCH_OP *arch_ops;

	arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->asic_set_agglimit)
		return arch_ops->asic_set_agglimit(ad, band_idx, ac, wdev, agg_limit);

	AsicNotSupportFunc(ad, __func__);
	return 0;
}

INT asic_set_rts_retrylimit(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT32 limit)
{
	struct _RTMP_ARCH_OP *arch_ops;

	arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->asic_set_rts_retrylimit)
		return arch_ops->asic_set_rts_retrylimit(ad, band_idx, limit);

	AsicNotSupportFunc(ad, __func__);
	return 0;
}

INT AsicAmpduEfficiencyAdjust(struct wifi_dev *wdev, UCHAR	aifs_adjust)
{
	struct _RTMP_ADAPTER *ad;
	UINT32	wmm_idx;
	struct _RTMP_ARCH_OP *arch_ops;

	if (!wdev)
		return 0;

	ad = wdev->sys_handle;
	wmm_idx = HcGetWmmIdx(ad, wdev);
	arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->asic_ampdu_efficiency_on_off)
		return arch_ops->asic_ampdu_efficiency_on_off(ad, wmm_idx, aifs_adjust);

	AsicNotSupportFunc(ad, __func__);
	return 0;
}

VOID AsicUpdateRxWCIDTableDetail(RTMP_ADAPTER *pAd, MT_WCID_TABLE_INFO_T WtblInfo)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->archUpdateRxWCIDTable)
		arch_ops->archUpdateRxWCIDTable(pAd, WtblInfo);
	else
		AsicNotSupportFunc(pAd, __func__);
}

VOID asic_show_mac_info(struct _RTMP_ADAPTER *pAd)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->show_mac_info)
		arch_ops->show_mac_info(pAd);
	else
		AsicNotSupportFunc(pAd, __func__);
}

INT asic_init_wtbl(struct _RTMP_ADAPTER *pAd, BOOLEAN bHardReset)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->init_wtbl)
		return arch_ops->init_wtbl(pAd, bHardReset);

	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

INT asic_get_wtbl_entry234(struct _RTMP_ADAPTER *pAd, UINT16 widx, struct wtbl_entry *ent)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->get_wtbl_entry234)
		return arch_ops->get_wtbl_entry234(pAd, widx, ent);

	AsicNotSupportFunc(pAd, __func__);
	return TRUE;
}



#ifdef LINK_TEST_SUPPORT
VOID LinkTestRcpiSet(RTMP_ADAPTER *pAd, UINT16 wcid, UINT8 u1AntIdx, CHAR i1Rcpi)
{
	struct wtbl_entry tb_entry;
	union WTBL_DW28 wtbl_wd28;

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));
	if (!asic_get_wtbl_entry234(pAd, wcid, &tb_entry)) {
		MTWF_DBG(pAd, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_ERROR,
			"Cannot found WTBL2/3/4 for WCID(%d)\n", wcid);
		return;
	}

	/* Read RCPI from WTBL DW28 */
	HW_IO_READ32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 112, &wtbl_wd28.word);

	switch (u1AntIdx) {
	case BITMAP_WF0:
		wtbl_wd28.field.resp_rcpi_0 = i1Rcpi;
		break;
	case BITMAP_WF1:
		wtbl_wd28.field.resp_rcpi_1 = i1Rcpi;
		break;
	case BITMAP_WF2:
		wtbl_wd28.field.resp_rcpi_2 = i1Rcpi;
		break;
	case BITMAP_WF3:
		wtbl_wd28.field.resp_rcpi_3 = i1Rcpi;
		break;
	}

	/* Write Back RCPI from WTBL DW28 */
	HW_IO_WRITE32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 112, wtbl_wd28.word);

	return;
}

VOID LinkTestPeriodHandler(RTMP_ADAPTER *pAd)
{
	UINT8 u1BandIdx;

	if (pAd->CommonCfg.LinkTestSupport) {
		if (!pAd->fgCmwLinkDone) {
			/* CSD config for state transition */
			for (u1BandIdx = BAND0; u1BandIdx <= pAd->CommonCfg.dbdc_mode; u1BandIdx++)
				LinkTestTxCsdCtrl(pAd, FALSE, u1BandIdx);

			MTWF_DBG(pAd, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_DEBUG,
				"NOT Link up!!!\n");
		} else {
			/* Auto Link Test Control Handler executes with period 100 ms */
			RTMP_AUTO_LINK_TEST(pAd);

			MTWF_DBG(pAd, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_DEBUG,
				"Link up!!!\n");
		}
	}
}

VOID LinkTestTimeSlotLinkHandler(RTMP_ADAPTER *pAd)
{
	BOOLEAN fgCmwLinkStatus = TRUE;
	UINT16 wcid;
	UINT8 u1BandIdx = BAND0;
	struct _MAC_TABLE_ENTRY *pEntry;

	/* get pointer to Entry */
	pEntry = entry_get(pAd, 0);

	/* Test scenario check (only one STA Connect) */
	if (pAd->MacTab->Size != 1)
		fgCmwLinkStatus = FALSE;

	/* Search pEntry Address */
	for (wcid = 0; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		pEntry = entry_get(pAd, wcid);

		/* APclient and Repeater not apply Link Test mechanism */
		if ((IS_ENTRY_REPEATER(pEntry)) || (IS_ENTRY_PEER_AP(pEntry))) {
			fgCmwLinkStatus = FALSE;
			break;
		}

		if (IS_ENTRY_CLIENT(pEntry)) {
			/* Check Test Instrument Condition */
			fgCmwLinkStatus = LinkTestInstrumentCheck(pAd, pEntry);
			/* Update Band Index */
			u1BandIdx = HcGetBandByWdev(pEntry->wdev);
			break;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_INFO,
		"fgCmwLinkStatus: %d\n", fgCmwLinkStatus);

	/* Tx Specifi Spatial Extension config */
	LinkTestSpeIdxCtrl(pAd, fgCmwLinkStatus, u1BandIdx);

	/* Tx Bw Handler */
	LinkTestTxBwCtrl(pAd, fgCmwLinkStatus, pEntry);

	/* Tx Csd Handler */
	LinkTestTxCsdCtrl(pAd, fgCmwLinkStatus, u1BandIdx);

	/* Rcpi Computation Method Handler */
	LinkTestRcpiCtrl(pAd, fgCmwLinkStatus, u1BandIdx);

	/* Tx Power Up Handler */
	LinkTestTxPowerCtrl(pAd, fgCmwLinkStatus, u1BandIdx);

	/* Rx Filter Mode control Handler */
	LinkTestAcrCtrl(pAd, fgCmwLinkStatus, wcid, u1BandIdx);

	/* Rx Stream Handler */
	LinkTestRxStreamCtrl(pAd, fgCmwLinkStatus, wcid);
}

VOID LinkTestStaLinkUpHandler(RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry)
{
	BOOLEAN fgCmwLinkStatus = FALSE;

	if (pAd->CommonCfg.LinkTestSupport) {
		/* Check Test Instrument Condition */
		if ((pEntry->MaxHTPhyMode.field.BW == BW_20) && (pAd->MacTab->Size == 1))
			fgCmwLinkStatus = LinkTestInstrumentCheck(pAd, pEntry);
		/* Tx Spur workaround */
		LinkTestTxBwCtrl(pAd, fgCmwLinkStatus, pEntry);
		/* Update Link Up status (Enable) */
		pAd->fgCmwLinkDone = TRUE;
		MTWF_DBG(pAd, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_INFO,
			"Build Association: Link Up!!\n");
	}
}

VOID LinkTestApClientLinkUpHandler(RTMP_ADAPTER *pAd)
{
	if (pAd->CommonCfg.LinkTestSupport)
		pAd->fgApclientLinkUp = TRUE;
}

BOOLEAN LinkTestInstrumentCheck(RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry)
{
	BOOLEAN fgCmwLinkStatus = TRUE;

	/* Check Condition: Support VHT mode Support */
	if (pEntry->SupportRateMode & SUPPORT_VHT_MODE)
		fgCmwLinkStatus = FALSE;

	/* Check condition: 1 Tx Spatial Stream */
	if ((pEntry->SupportHTMCS > 0xFF) && ((MODE_HTMIX == pEntry->MaxHTPhyMode.field.MODE) || (pEntry->MaxHTPhyMode.field.MODE == MODE_HTGREENFIELD)))
		fgCmwLinkStatus = FALSE;

	/* Check condition: only support BW20 */
	if (pEntry->MaxHTPhyMode.field.BW != BW_20)
		fgCmwLinkStatus = FALSE;

	return fgCmwLinkStatus;
}

VOID LinkTestChannelBandUpdate(RTMP_ADAPTER *pAd, UINT8 u1BandIdx, UINT8 u1ControlChannel)
{
	if (pAd->CommonCfg.LinkTestSupport) {
		/* update channel band info */
		if (u1ControlChannel <= 14) {
			pAd->ucCmwChannelBand = CHANNEL_BAND_2G;
			MTWF_DBG(pAd, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_INFO,
				" 2G Channel Band !!\n");
		} else {
			pAd->ucCmwChannelBand = CHANNEL_BAND_5G;
			MTWF_DBG(pAd, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_INFO,
				" 5G Channel Band !!\n");
		}

		/* update channe band info flag */
		pAd->fgChannelBandInfoUpdate = TRUE;
	}
}

VOID LinkTestChannelSwitchHandler(RTMP_ADAPTER *pAd, UINT8 u1BandIdx)
{
	if (pAd->CommonCfg.LinkTestSupport) {
		/* Update Link Up status (Disable) */
		pAd->fgCmwLinkDone = FALSE;
		/* Update Apclient Link up Flag */
		pAd->fgApclientLinkUp = FALSE;
		/* clear Timeout Count */
		pAd->ucRxTestTimeoutCount = 0;
		/* Restore to 4R Config */
		MtCmdLinkTestRxCtrl(pAd, BITMAP_WF_ALL, u1BandIdx);
		/* Update specific nR config Status */
		pAd->ucRxStreamState = RX_DEFAULT_RXSTREAM_STATE;
		/* Update specific nR previous config Status */
		pAd->ucRxStreamStatePrev = RX_DEFAULT_RXSTREAM_STATE;
	}
}

VOID LinkTestTxBwSwitch(RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry)
{
	UINT8 u1BandIdx;

	/* Update Band Index */
	u1BandIdx = HcGetBandByWdev(pEntry->wdev);

	/* Backup Primary channel Info */
	pAd->ucPrimChannel = wlan_operate_get_prim_ch(pEntry->wdev);

	/* Backup Central channel Info */
	pAd->ucCentralChannel = wlan_operate_get_cen_ch_1(pEntry->wdev);

	/* Backup Central channel2 Info */
	pAd->ucCentralChannel2 = wlan_operate_get_cen_ch_2(pEntry->wdev);

	/* Backup Extend channel Info */
	pAd->ucExtendChannel = wlan_operate_get_ext_cha(pEntry->wdev);

	/* Backup HT Bw Info */
	pAd->ucHtBw = wlan_operate_get_ht_bw(pEntry->wdev);

	/* Backup VHT Bw Info */
	pAd->ucVhtBw = wlan_operate_get_vht_bw(pEntry->wdev);

	MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_DEBUG,
		"[BAND%d] PrimChannel: %d, CentralChannel: %d, CentralChannel2: %d",
		u1BandIdx,
		pAd->ucPrimChannel,
		pAd->ucCentralChannel,
		pAd->ucCentralChannel2,
		", ExtChannel: %d, Ht_Bw: %d, Vht_Bw: %d\n",
		pAd->ucExtendChannel,
		pAd->ucHtBw,
		pAd->ucVhtBw);

	/* Config HT BW20 */
	wlan_operate_set_ht_bw(pEntry->wdev, HT_BW_20, EXTCHA_NONE);

	/* Config VHT BW20 */
	wlan_operate_set_vht_bw(pEntry->wdev, VHT_BW_2040);
}

VOID LinkTestTxBwRestore(RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry)
{
	UINT8 u1BandIdx;

	MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_DEBUG,
		"Restore Bw config !!!\n");

	/* Update Band Index */
	u1BandIdx = HcGetBandByWdev(pEntry->wdev);

	/* Restore Ht Bw config */
	wlan_operate_set_ht_bw(pEntry->wdev, pAd->ucHtBw, pAd->ucExtendChannel);

	/* Restore Vht Bw config */
	wlan_operate_set_vht_bw(pEntry->wdev, pAd->ucVhtBw);
}

VOID LinkTestSpeIdxCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, UINT8 u1BandIdx)
{
	/* check if Spatial Extension workaround enabled */
	if (pAd->fgTxSpeEn) {
		/* state transition for enable/disable Tx Spatial Extension workaround */
		switch (pAd->ucLinkSpeState) {
		case TX_UNDEFINED_SPEIDX_STATE:
		case TX_DEFAULT_SPEIDX_STATE:
			if (fgCmwLinkStatus) {
				/* Enable specific Spatial Extension config for Link test */
				MtCmdLinkTestSeIdxCtrl(pAd, TX_SWITCHING_SPEIDX_STATE);

				/* update Tx Spatial Extension State */
				pAd->ucLinkSpeState = TX_SWITCHING_SPEIDX_STATE;

				/* update Tx Spatial Extension previous State */
				pAd->ucLinkSpeStatePrev = TX_SWITCHING_SPEIDX_STATE;
			}
			break;

		case TX_SWITCHING_SPEIDX_STATE:
			if (!fgCmwLinkStatus) {
				/* Disable specific Spatial Extension config for Link test */
				MtCmdLinkTestSeIdxCtrl(pAd, TX_DEFAULT_SPEIDX_STATE);

				/* update Tx Spatial Extension State */
				pAd->ucLinkSpeState = TX_DEFAULT_SPEIDX_STATE;

				/* update Tx Spatial Extension previous State */
				pAd->ucLinkSpeStatePrev = TX_DEFAULT_SPEIDX_STATE;
			}
			break;

		default:
			break;
		}
	}
}

VOID LinkTestTxBwCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, struct _MAC_TABLE_ENTRY *pEntry)
{
	UINT8 u1BandIdx;

	/* Update Band Index */
	u1BandIdx = HcGetBandByWdev(pEntry->wdev);

	/* check if Tx Spur workaround enabled */
	if (pAd->fgTxSpurEn) {
		/* state transition for enable/disable Tx Spur workaround */
		switch (pAd->ucLinkBwState) {
		case TX_UNDEFINED_BW_STATE:
		case TX_DEFAULT_BW_STATE:

			if (fgCmwLinkStatus) {
				/* Bw switching */
				LinkTestTxBwSwitch(pAd, pEntry);

				/* update Bw and channel Info status flag */
				pAd->fgBwInfoUpdate = TX_SWITCHING_BW_STATE;

				/* update Bw State */
				pAd->ucLinkBwState = TX_SWITCHING_BW_STATE;

				/* update Bw previous State */
				pAd->ucLinkBwStatePrev = TX_SWITCHING_BW_STATE;
			}
			break;

		case TX_SWITCHING_BW_STATE:

			if ((!fgCmwLinkStatus) && (pAd->fgBwInfoUpdate)) {
				/* Bw Restore */
				LinkTestTxBwRestore(pAd, pEntry);

				/* reset Bw and channel Info status flag */
				pAd->fgBwInfoUpdate = TX_DEFAULT_BW_STATE;

				/* update Bw State */
				pAd->ucLinkBwState = TX_DEFAULT_BW_STATE;

				/* update Bw previous State */
				pAd->ucLinkBwStatePrev = TX_DEFAULT_BW_STATE;
			}
			break;

		default:
			break;
		}
	}
}

VOID LinkTestTxCsdCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, UINT8 u1BandIdx)
{
	BOOLEAN fgZeroCsd = FALSE;

	/* channel band info updated sanity check */
	if (pAd->fgChannelBandInfoUpdate)
		return;

	/* check MAC Table size to determine enabled/disabled status of Tx CSD config */
	if ((pAd->MacTab.Size == 0) || ((pAd->MacTab.Size == 1) && (fgCmwLinkStatus)))
		fgZeroCsd = TRUE;

	MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_DEBUG,
		"fgCmwLinkStatus: %d, fgZeroCsd: %d\n",
		fgCmwLinkStatus, fgZeroCsd);

	/* CSD config for state transition */
	switch (pAd->ucTxCsdState) {
	case TX_UNDEFINED_CSD_STATE:
	case TX_DEFAULT_CSD_STATE:

		if (fgZeroCsd) {
			/* Zero CSD config for Link test */
			MtCmdLinkTestTxCsdCtrl(pAd, TX_ZERO_CSD_STATE, u1BandIdx, pAd->ucCmwChannelBand);

			/* update Tx CSD State */
			pAd->ucTxCsdState = TX_ZERO_CSD_STATE;

			/* update Tx CSD previos State */
			pAd->ucTxCsdStatePrev = TX_ZERO_CSD_STATE;
		}
		break;

	case TX_ZERO_CSD_STATE:

		if (!fgZeroCsd) {
			/* Default CSD config for Link test */
			MtCmdLinkTestTxCsdCtrl(pAd, TX_DEFAULT_CSD_STATE, u1BandIdx, pAd->ucCmwChannelBand);

			/* update Tx CSD State */
			pAd->ucTxCsdState = TX_DEFAULT_CSD_STATE;
		}
		break;

	default:
		break;
	}
}

VOID LinkTestRcpiCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, UINT8 u1BandIdx)
{
	/* check if Rcpi workaround enabled */
	if (pAd->fgRxRcpiEn) {
		/* state transition for enable/disable Rcpi workaround */
		switch (pAd->ucLinkRcpiState) {
		case RX_UNDEFINED_RCPI_STATE:
		case RX_DEFAULT_RCPI_STATE:
			if (fgCmwLinkStatus) {
				/* Enable specific Rcpi config for Link test (Rcpi computation refer to both Response Frame and Data Frame) */
				MtCmdLinkTestRcpiCtrl(pAd, RX_SPECIFIC_RCPI_STATE);

				/* update Rcpi State */
				pAd->ucLinkRcpiState = RX_SPECIFIC_RCPI_STATE;
			}
			break;

		case RX_SPECIFIC_RCPI_STATE:
			if (!fgCmwLinkStatus) {
				/* Disable specific Rcpi config for Link test */
				MtCmdLinkTestRcpiCtrl(pAd, RX_DEFAULT_RCPI_STATE);

				/* update Rcpi State */
				pAd->ucLinkRcpiState = RX_DEFAULT_RCPI_STATE;
			}
			break;

		default:
			break;
		}
	}
}

VOID LinkTestTxPowerCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, UINT8 u1BandIdx)
{
	BOOLEAN fgPowerBoost = FALSE;

	/* channel band info updated sanity check */
	if (pAd->fgChannelBandInfoUpdate)
		return;

	/* check MAC Table size to determine enabled/disabled status of Tx Power up config */
	if ((pAd->MacTab.Size == 1) && (fgCmwLinkStatus))
		fgPowerBoost = TRUE;

	MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_DEBUG,
		"fgCmwLinkStatus: %d, fgPowerBoost: %d\n", fgCmwLinkStatus, fgPowerBoost);

	/* Tx Power config for state transition */
	switch (pAd->ucTxPwrBoostState) {
	case TX_UNDEFINED_POWER_STATE:
	case TX_DEFAULT_POWER_STATE:

		if (fgPowerBoost) {
			/* Boost Tx Power config for Link test */
			MtCmdLinkTestTxPwrCtrl(pAd, TX_BOOST_POWER_STATE, u1BandIdx, pAd->ucCmwChannelBand);

			/* update Tx Power State */
			pAd->ucTxPwrBoostState = TX_BOOST_POWER_STATE;
		}
		break;

	case TX_BOOST_POWER_STATE:

		if (!fgPowerBoost) {
			/* Default Tx Power config for Link test */
			MtCmdLinkTestTxPwrCtrl(pAd, TX_DEFAULT_POWER_STATE, u1BandIdx, pAd->ucCmwChannelBand);

			/* update Tx Power State */
			pAd->ucTxPwrBoostState = TX_DEFAULT_POWER_STATE;
		}
		break;

	default:
		break;
	}
}

VOID LinkTestRxCntCheck(RTMP_ADAPTER *pAd)
{
	INT64 c8RxCount;

	/* Read Rx Count Info */
	c8RxCount = pAd->WlanCounters.ReceivedFragmentCount.QuadPart;
	MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_INFO,
		"c8RxCount: %lld, c8TempRxCount: %lld\n",
		c8RxCount, pAd->c8TempRxCount);

	/* check Rx test timeout count */
	if (c8RxCount - pAd->c8TempRxCount <= pAd->c8RxCountTh)
		pAd->ucRxTestTimeoutCount++;
	else
		pAd->ucRxTestTimeoutCount = 0;

	MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_INFO,
		"nR mode Rx Count: %lld, RxTestTimeoutCount: %d\n",
		c8RxCount - pAd->c8TempRxCount, pAd->ucRxTestTimeoutCount);

	/* update Rx Count to temp buffer */
	pAd->c8TempRxCount = pAd->WlanCounters.ReceivedFragmentCount.QuadPart;
}

VOID LinkTestRxStreamCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, UINT16 wcid)
{
	struct _MAC_TABLE_ENTRY *pEntry;
	UINT8 u1SpeRssiIdx = 0;
	UINT8 u1RssiReason;
	UINT8 u1BandIdx;
	UINT8 u1AndIdx;
	CHAR i1MaxRssi = MINIMUM_POWER_VALUE;
	CHAR i1Rssi[WF_NUM] = {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};

	/* get pointer to Entry for specific WlanId */
	pEntry = entry_get(pAd, wcid);

	/* Update Band Index */
	u1BandIdx = HcGetBandByWdev(pEntry->wdev);

	/* get Rssi value in WTBL */
	LinkTestRssiGet(pAd, RSSI_CHECK_WTBL_RSSI, wcid, i1Rssi);

	/* get Max Rssi value in WTBL */
	for (u1AndIdx = WF0; u1AndIdx < WF_NUM; u1AndIdx++) {
		/* Rssi sanity protection */
		if (i1Rssi[u1AndIdx] >= 0)
			i1Rssi[u1AndIdx] = MINIMUM_POWER_VALUE;
		/* update Max Rssi value */
		if (i1Rssi[u1AndIdx] > i1MaxRssi)
			i1MaxRssi = i1Rssi[u1AndIdx];
	}

	/* Rssi value sanity check */
	if (i1MaxRssi == MINIMUM_POWER_VALUE)
		return;

	/* Config RSSI Moving Average Ratio 1/2 */
	MtCmdLinkTestRcpiMACtrl(pAd, CMW_RCPI_MA_1_2);

	/* check if Rx sensitivity workaround enabled */
	if (pAd->fgRxSensitEn) {
		MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_DEBUG,
			"Rssi: (%d : %d : %d : %d)\n",
			i1Rssi[WF0], i1Rssi[WF1], i1Rssi[WF2], i1Rssi[WF3]);

		/* only allow potentail state transition for Max Rssi < -40dB for change channel scenario */
		switch (pAd->ucRxStreamState) {
		case RX_UNDEFINED_RXSTREAM_STATE:
		case RX_DEFAULT_RXSTREAM_STATE:

			if ((i1MaxRssi < pAd->cNrRssiTh) && (fgCmwLinkStatus)) {

				/* Rssi significance check */
				LinkTestRssiCheck(pAd, i1Rssi, u1BandIdx, &u1SpeRssiIdx, &u1RssiReason, wcid);
				MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_DEBUG,
					"Band: %d, Specific Rssi Rx Path BitMap Index: %d\n",
					u1BandIdx, u1SpeRssiIdx);

				/* Significant Rssi value Antenna Index check */
				if (u1SpeRssiIdx == 0x0) {
					/* Reset Rx Stream switching confidence count */
					pAd->ucRxSenCount = 0;
					MTWF_DBG(pAd, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_INFO,
					"Increment Rx Stream switching confidence count: %d\n",
					pAd->ucRxSenCount);
				} else if (u1SpeRssiIdx != pAd->u1SpeRssiIdxPrev) {
					/* config Rx Stream switching confidence count to unity */
					pAd->ucRxSenCount = 1;
				} else {
					/* Increment Rx Stream switching confidence count */
					pAd->ucRxSenCount++;
					MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_INFO,
						"Reset Rx Stream switching confidence count\n");
				}

				/* update previous significant Rssi value Antenna Index */
				pAd->u1SpeRssiIdxPrev = u1SpeRssiIdx;

				/* Rx Stream switching confidence count check */
				if (pAd->ucRxSenCount >= pAd->ucRxSenCountTh) {
					/* Rx Stream switching */
					LinkTestRxStreamTrans(pAd, u1BandIdx, u1SpeRssiIdx);
					/* clear Rssi value in WTBL */
					AsicRcpiReset(pAd, wcid);
					/* update Rx Stream switching reason */
					pAd->u1RxStreamSwitchReason = u1RssiReason;
				}
			}
			break;

		case RX_RXSTREAM_WF0_STATE:
		case RX_RXSTREAM_WF1_STATE:
		case RX_RXSTREAM_WF2_STATE:
		case RX_RXSTREAM_WF3_STATE:
		case RX_RXSTREAM_WF01_STATE:
		case RX_RXSTREAM_WF02_STATE:
		case RX_RXSTREAM_WF03_STATE:
		case RX_RXSTREAM_WF12_STATE:
		case RX_RXSTREAM_WF13_STATE:
		case RX_RXSTREAM_WF23_STATE:
		case RX_RXSTREAM_WF012_STATE:
		case RX_RXSTREAM_WF013_STATE:
		case RX_RXSTREAM_WF023_STATE:
		case RX_RXSTREAM_WF123_STATE:

			if (pAd->u1RxStreamSwitchReason == RSSI_REASON_SENSITIVITY) {
				/* check Rx count Status */
				LinkTestRxCntCheck(pAd);
				/* Restore to default Rx Stream config for Timeout condition or not Link Test scenario or change path scenario */
				if ((pAd->ucRxTestTimeoutCount > pAd->ucTimeOutTh) || (!fgCmwLinkStatus)) {
					/* clear Rssi value in WTBL */
					AsicRcpiReset(pAd, wcid);
					/* Restore to 4R Config */
					LinkTestRxStreamTrans(pAd, u1BandIdx, BITMAP_WF_ALL);
					/* Reset Timeout Count */
					pAd->ucRxTestTimeoutCount = 0;
				}
			} else if (pAd->u1RxStreamSwitchReason == RSSI_REASON_RX_BLOCKING) {
				/* check WB Rssi value */
				u1SpeRssiIdx = LinkTestRssiCheckItem(pAd, i1Rssi, u1BandIdx, RSSI_CHECK_BBP_WBRSSI, wcid);
				/* Restore to default Rx Stream config for Timeout condition or not Link Test scenario or change path scenario */
				if ((u1SpeRssiIdx == 0x0) || (!fgCmwLinkStatus)) {
					/* clear Rssi value in WTBL */
					AsicRcpiReset(pAd, wcid);
					/* Restore to 4R Config */
					LinkTestRxStreamTrans(pAd, u1BandIdx, BITMAP_WF_ALL);
				}
			}
			break;

		default:
			break;
		}
	}
}

VOID LinkTestRxStreamTrans(RTMP_ADAPTER *pAd, UINT8 u1BandIdx, UINT8 u1SpeRssiIdx)
{
	MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_INFO,
		"nR mode after link up!!! Rx Path Bitmap Index: %d\n", u1SpeRssiIdx);

	/* Reset Rx Stream switching confidence count */
	pAd->ucRxSenCount = 0;

	/* Enter specific nR mode */
	MtCmdLinkTestRxCtrl(pAd, u1SpeRssiIdx, u1BandIdx);

	/* Update specific nR config Status */
	pAd->ucRxStreamState = u1SpeRssiIdx;
}

VOID LinkTestAcrCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, UINT16 wcid, UINT8 u1BandIdx)
{
	UINT8 u1AndIdx;
	CHAR i1MaxRssi = MINIMUM_POWER_VALUE;
	CHAR i1Rssi[WF_NUM] = {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};

	/* get Rssi value in WTBL */
	LinkTestRssiGet(pAd, RSSI_CHECK_WTBL_RSSI, wcid, i1Rssi);

	/* get Max Rssi value */
	for (u1AndIdx = WF0; u1AndIdx < WF_NUM; u1AndIdx++) {
		/* Rssi sanity protection */
		if (i1Rssi[u1AndIdx] >= 0)
			i1Rssi[u1AndIdx] = MINIMUM_POWER_VALUE;
		/* update Max Rssi value */
		if (i1Rssi[u1AndIdx] > i1MaxRssi)
			i1MaxRssi = i1Rssi[u1AndIdx];
	}

	MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_DEBUG,
		"%s(): Rssi: (%d : %d : %d : %d)\n",
		i1Rssi[WF0], i1Rssi[WF1], i1Rssi[WF2], i1Rssi[WF3]);

	/* Instrument check */
	if (!fgCmwLinkStatus)
		return;

	/* check ACR patch enable/disable status */
	if (pAd->fgACREn) {

		switch (pAd->ucRxFilterstate) {
		case TX_UNDEFINED_RXFILTER_STATE:
		case TX_DEFAULT_MAXIN_STATE:

			if (i1MaxRssi <= pAd->cMaxInRssiTh) {
				if (pAd->ucRxFilterConfidenceCnt >= pAd->ucACRConfidenceCntTh) {
					/* Fw command to apply ACR patch */
					MtCmdLinkTestACRCtrl(pAd, TX_SPECIFIC_ACR_STATE, u1BandIdx);  /* ACR patch */

					/* Clear ACR confidence count */
					pAd->ucRxFilterConfidenceCnt = 0;

					/* update Rx Filter State */
					pAd->ucRxFilterstate = TX_SPECIFIC_ACR_STATE;

					MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_DEBUG,
						"ACR patch!!!\n");
				} else {
					/* ACI confidence count increment */
					pAd->ucRxFilterConfidenceCnt++;
				}
			}
			break;

		case TX_SPECIFIC_ACR_STATE:

			if (i1MaxRssi > pAd->cMaxInRssiTh) {
				if (pAd->ucRxFilterConfidenceCnt >= pAd->ucMaxInConfidenceCntTh) {
					/* Fw command to apply MaxIn patch */
					MtCmdLinkTestACRCtrl(pAd, TX_DEFAULT_MAXIN_STATE, u1BandIdx); /* Max Input patch */

					/* Clear MaxIn confidence count */
					pAd->ucRxFilterConfidenceCnt = 0;

					/* update Rx Filter State */
					pAd->ucRxFilterstate = TX_DEFAULT_MAXIN_STATE;

					MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_DEBUG,
						"MaxIn patch!!!\n");
				} else {
					/* MaxIn confidence count increment */
					pAd->ucRxFilterConfidenceCnt++;
				}
			}
			break;

		default:
			break;
		}
	}
}

VOID LinkTestRssiGet(RTMP_ADAPTER *pAd, ENUM_RSSI_CHECK_SOURCE eRssiSrc, UINT16 wcid, PCHAR pi1Rssi)
{
	UINT32 u4Buffer1, u4Buffer2;

	switch (eRssiSrc) {
	case RSSI_CHECK_BBP_WBRSSI:
		/* read Rssi value*/
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_2, &u4Buffer1);
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_AGC_DEBUG_2, &u4Buffer2);
		break;

	case RSSI_CHECK_BBP_IBRSSI:
		/* read Rssi value*/
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_2, &u4Buffer1);
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_AGC_DEBUG_2, &u4Buffer2);
		break;

	case RSSI_CHECK_WTBL_RSSI:
		chip_get_rssi(pAd, wcid, pi1Rssi);
		break;
	default:
		break;
	}
}

VOID LinkTestRssiCheck(RTMP_ADAPTER *pAd, PCHAR pi1Rssi, UINT8 u1BandIdx, PUINT8 pu1SpeRssiIdx, PUINT8 pu1RssiReason, UINT16 wcid)
{
	UINT8 u1SpeRssiIdx = 0, u1SpeRssiIdx2 = 0;

	/* check Rssi value in WTBL */
	u1SpeRssiIdx = LinkTestRssiCheckItem(pAd, pi1Rssi, u1BandIdx, RSSI_CHECK_WTBL_RSSI, wcid);

	/* update Rssi reason */
	*pu1RssiReason = RSSI_REASON_SENSITIVITY;

	if (u1SpeRssiIdx == 0x0) {
		/* check WB Rssi value */
		u1SpeRssiIdx = LinkTestRssiCheckItem(pAd, pi1Rssi, u1BandIdx, RSSI_CHECK_BBP_WBRSSI, wcid);

		/* check IB Rssi value */
		u1SpeRssiIdx2 = LinkTestRssiCheckItem(pAd, pi1Rssi, u1BandIdx, RSSI_CHECK_BBP_IBRSSI, wcid);

		/* reset specific Rx path for not satisfy condition IB Rssi not small enough for non-significance path for Rx Block scenario */
		if ((u1SpeRssiIdx & u1SpeRssiIdx2) != u1SpeRssiIdx2)
			u1SpeRssiIdx = 0;

		/* update Rssi reason */
		*pu1RssiReason = RSSI_REASON_RX_BLOCKING;
	}

	/* update Specific Rx path bitmap */
	*pu1SpeRssiIdx = u1SpeRssiIdx;
}

/*
 *  Function: check RSSI Significance path
 *
 *  Parameter:
 *
 *	  @ pAd
 *
 *	  @ pcRSSI: pointer of array of RSSI values
 *
 *	  @ ucRSSIThManual: RSSI Significance Threshold. If this value is 0xFF, program will use dynamic threshold.
 *
 *	  @ ucBandIdx: DBDC Band Index
 *
 *  Return:
 *
 *	  @ ucRxIdx: RSSI Significant path index bitmap. 0x5 means WF0 and WF2. 0x0 mean no RSSI significant path.
 */

UINT8 LinkTestRssiCheckItem(RTMP_ADAPTER *pAd, PCHAR pi1Rssi, UINT8 u1BandIdx, ENUM_RSSI_CHECK_SOURCE eRssiCheckSrc, UINT16 wcid)
{
	UINT8 ucRxIdx = 0;
	UINT8 u1RssiNum = 0;
	CHAR i1RssiBuffer[WF_NUM] = {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};
	CHAR cRssiBackup[WF_NUM] = {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};

	/* Rssi source handler */
	LinkTestRssiGet(pAd, eRssiCheckSrc, wcid, i1RssiBuffer);

	MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_INFO,
		"original Rssi: (%d : %d : %d : %d)\n",
		pi1Rssi[WF0], pi1Rssi[WF1], pi1Rssi[WF2], pi1Rssi[WF3]);

	if (pAd->CommonCfg.dbdc_mode) {
		/* update Rssi Num */
		u1RssiNum = RX_STREAM_PATH_DBDC_MODE;
		/* Rssi pre-prossing for comparison */
		switch (u1BandIdx) {
		case BAND0:
			os_move_mem(cRssiBackup, i1RssiBuffer, u1RssiNum);
			break;
		case BAND1:
			os_move_mem(cRssiBackup, i1RssiBuffer + 2, u1RssiNum);
			break;
		default:
			break;
		}
	} else {
		/* update Rssi Num */
		u1RssiNum = RX_STREAM_PATH_SINGLE_MODE;
		/* Rssi pre-prossing for comparison */
		os_move_mem(cRssiBackup, i1RssiBuffer, u1RssiNum);
	}

	/* Rssi Significance check */
	ucRxIdx = LinkTestRssiComp(pAd, i1RssiBuffer, u1RssiNum, eRssiCheckSrc);

	return ucRxIdx;
}

UINT8 LinkTestRssiComp(RTMP_ADAPTER *pAd, PCHAR pi1Rssi, UINT8 u1RssiNum, ENUM_RSSI_CHECK_SOURCE eRssiCheckSrc)
{
	CHAR cRssiBackup[MAX_RSSI_LEN];
	UINT8 u1RxPathRssiOrder[MAX_RSSI_LEN];
	UINT8 u1SpecificRxPathBitMap = 0;
	UINT8 u1AntIdx, u1AntIdx2;

	for (u1AntIdx = 0; u1AntIdx < MAX_RSSI_LEN; u1AntIdx++) {
		cRssiBackup[u1AntIdx] = MINIMUM_POWER_VALUE;
		u1RxPathRssiOrder[u1AntIdx] = u1AntIdx;
	}

	/* sanity check */
	if (u1RssiNum < 2) {
		MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_INFO,
			"Rssi Num not enough!! ( RssiNum < 2)\n");
		return 0;
	}

	/* Backup Rssi info to buffer */
	os_move_mem(cRssiBackup, pi1Rssi, u1RssiNum);

	/* Bubble sorting for Rssi (from small to large) */
	for (u1AntIdx = 0; u1AntIdx < u1RssiNum - 1; u1AntIdx++) {
		for (u1AntIdx2 = 0; u1AntIdx2 < u1RssiNum - u1AntIdx - 1; u1AntIdx2++) {
			if (cRssiBackup[u1AntIdx2] > cRssiBackup[u1AntIdx2 + 1]) {
				/* Swap Rssi value */
				LinkTestSwap(cRssiBackup + u1AntIdx2, cRssiBackup + u1AntIdx2 + 1);
				/* Swap Rx Path order */
				LinkTestSwap(u1RxPathRssiOrder + u1AntIdx2, u1RxPathRssiOrder + u1AntIdx2 + 1);
			}
		}
	}

	MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_INFO,
		"Reordered Rssi: (%d : %d : %d : %d)\n",
		cRssiBackup[0], cRssiBackup[1], cRssiBackup[2], cRssiBackup[3]);
	MTWF_DBG(NULL, DBG_CAT_CMW, CATCMW_LINK, DBG_LVL_INFO,
		"Reordered Rx Path: (%d : %d : %d : %d)\n",
		u1RxPathRssiOrder[0], u1RxPathRssiOrder[1],
		u1RxPathRssiOrder[2], u1RxPathRssiOrder[3]);

	/* update significant Rx path Num */
	u1SpecificRxPathBitMap = LinkTestRssiSpecificRxPath(pAd, cRssiBackup, u1RxPathRssiOrder, u1RssiNum, eRssiCheckSrc);

	/* return final Significant Rx Path BitMap */
	return u1SpecificRxPathBitMap;
}

UINT8 LinkTestRssiSpecificRxPath(RTMP_ADAPTER *pAd, PCHAR pi1Rssi, PUINT8 pu1RxPathRssiOrder, UINT8 u1RssiNum, ENUM_RSSI_CHECK_SOURCE eRssiCheckSrc)
{
	UINT8 u1RssiTh;
	UINT8 u1AntIdx;
	UINT8 ucSigRxPathNum = 0;
	UINT8 ucRxPathNumCnt = 0;
	UINT8 u1SpecificRxPathBitMap = 0;

	switch (eRssiCheckSrc) {
	case RSSI_CHECK_WTBL_RSSI:
		/* update Rssi Threshold for check */
		u1RssiTh = pAd->ucRssiSigniTh;

		/* check specific Rx path Num */
		for (u1AntIdx = 0; u1AntIdx < u1RssiNum - 1; u1AntIdx++) {
			if ((pi1Rssi[u1AntIdx + 1] - pi1Rssi[u1AntIdx]) > u1RssiTh) {
				ucSigRxPathNum = u1RssiNum - u1AntIdx - 1;
				break;
			}
		}

		/* config specific Rx Path BitMap */
		if (ucSigRxPathNum != 0) {
			for (u1AntIdx = u1RssiNum - 1, ucRxPathNumCnt = 0;
				ucRxPathNumCnt < ucSigRxPathNum; u1AntIdx--, ucRxPathNumCnt++) {
				/* Enable specific Rx Path Index */
				u1SpecificRxPathBitMap |= (1 << pu1RxPathRssiOrder[u1AntIdx]);
			}
		}
		break;

	case RSSI_CHECK_BBP_WBRSSI:
		/* update Rssi Threshold for check */
		u1RssiTh = pAd->cWBRssiTh;

		/* config specific Rx Path BitMap */
		for (u1AntIdx = 0; u1AntIdx < u1RssiNum; u1AntIdx++) {
			/* Enable specific Rx Path Index (WBRssi is larger than Threshold) */
			if (pi1Rssi[u1AntIdx] > u1RssiTh)
				u1SpecificRxPathBitMap |= (1 << pu1RxPathRssiOrder[u1AntIdx]);
		}
		break;

	case RSSI_CHECK_BBP_IBRSSI:
		/* update Rssi Threshold for check */
		u1RssiTh = pAd->cIBRssiTh;

		/* config specific Rx Path BitMap */
		for (u1AntIdx = 0; u1AntIdx < u1RssiNum; u1AntIdx++) {
			/* Enable specific Rx Path Index (IBRssi is larger than Threshold) */
			if (pi1Rssi[u1AntIdx] > u1RssiTh)
				u1SpecificRxPathBitMap |= (1 << pu1RxPathRssiOrder[u1AntIdx]);
		}
		break;

	default:
		break;
	}

	return ucSigRxPathNum;
}

VOID LinkTestSwap(PCHAR pi1Value1, PCHAR pi1Value2)
{
	CHAR i1Temp;

	/* backup value */
	i1Temp = *pi1Value1;

	/* assign value1 to be value2 */
	*pi1Value1 = *pi1Value2;

	/* assign value2 to be backup value */
	*pi1Value2 = i1Temp;
}
#endif /* LINK_TEST_SUPPORT */

/*----------------------------------------------------------------------------*/
/*! Key word: "RXBLOCKER", "WBRSSI", "IBRSSI"
* \Concept:  Switch 4RX to 1RX by detect WBRSSI
*1
* \Input:	 None
*
* \return:   WBRSSI[MAX] or -1
*/
/*----------------------------------------------------------------------------*/
#ifdef	ETSI_RX_BLOCKER_SUPPORT
UINT8 ETSIWbRssiCheck(
	RTMP_ADAPTER *pAd
)
{
	BOOLEAN	fg1RVaild		   = TRUE;
	UINT8	u1WfIdx;
	UINT8	u1MaxWbRssiIdx	  = 0;
	CHAR	c1MaxWbRssi		 = MINIMUM_POWER_VALUE;
	CHAR	c1WbRssi[WF_NUM]	= {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};
	CHAR	c1IbRssi[WF_NUM]	= {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};

	/* buffer to read CR */
	UINT32	u4WbRssi			= 0;

	/* Read CR (manual command) */
	if (pAd->fgFixWbIBRssiEn) {
		/* WBRSSI */
		c1WbRssi[WF0] = pAd->c1WbRssiWF0;
		c1WbRssi[WF1] = pAd->c1WbRssiWF1;
		c1WbRssi[WF2] = pAd->c1WbRssiWF2;
		c1WbRssi[WF3] = pAd->c1WbRssiWF3;
		/* IBRSSI */
		c1IbRssi[WF0] = pAd->c1IbRssiWF0;
		c1IbRssi[WF1] = pAd->c1IbRssiWF1;
		c1IbRssi[WF2] = pAd->c1IbRssiWF2;
		c1IbRssi[WF3] = pAd->c1IbRssiWF3;
	}

	/* log for check Rssi Read (WBRSSI/IBRSSI) */
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"-----------------------------------------------------------------------------\n");
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		" c1WbRssi: WF0: %x, WF1: %x, WF2: %x, WF3: %x\n",
		c1WbRssi[WF0]&0xFF, c1WbRssi[WF1]&0xFF, c1WbRssi[WF2]&0xFF, c1WbRssi[WF3]&0xFF);
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		" c1IbRssi: WF0: %x, WF1: %x, WF2: %x, WF3: %x\n",
		c1IbRssi[WF0]&0xFF, c1IbRssi[WF1]&0xFF, c1IbRssi[WF2]&0xFF, c1IbRssi[WF3]&0xFF);
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"-----------------------------------------------------------------------------\n");
	/* ---------------- */


	/* CR risk - no expected 0x80 value on WF0/WF1 ; WF2/WF3  */
	if (((c1WbRssi[WF0]&0xFF) == 0x80) || ((c1WbRssi[WF2]&0xFF) == 0x80)) {

		fg1RVaild = TRUE;
		/* log for check Rssi Read (WBRSSI/IBRSSI) */
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"-----------------------------------------------------------------------------\n");
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		" CR risk !!\n");
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"-----------------------------------------------------------------------------\n");
		/* ---------------- */
	}
	/* No CR risk */
	else {

	/* Find Max Rssi */
	for (u1WfIdx = WF0; u1WfIdx < WF_NUM; u1WfIdx++) {
		if (c1WbRssi[u1WfIdx] > c1MaxWbRssi) {
			/* update Max WBRSSI value */
			c1MaxWbRssi = c1WbRssi[u1WfIdx];
			/* update Max WBRSSI index */
			u1MaxWbRssiIdx = u1WfIdx;
		}
	}


	/* log Max Rssi Value and Max Rssi Index */
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
	"-----------------------------------(4R State)-------------------------------------\n");
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
	" u1WfIdx: %x\n", u1WfIdx);
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
	" c1MaxWbRssi: %x\n", c1MaxWbRssi);
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
	"-----------------------------------------------------------------------------\n");
	/* ---------------- */


	/* check state transition status (4R->1R) */
	if (c1MaxWbRssi >= pAd->c1RWbRssiHTh) {
		for (u1WfIdx = WF0; u1WfIdx < WF_NUM; u1WfIdx++) {
			if ((u1WfIdx != u1MaxWbRssiIdx) && \
				((c1WbRssi[u1WfIdx] > pAd->c1RWbRssiLTh) || (c1IbRssi[u1WfIdx] > pAd->c1RIbRssiLTh))) {
				fg1RVaild = FALSE;
			} else
				fg1RVaild = TRUE;
		}
	} else
		fg1RVaild = FALSE;

	}

	/* log check flag to 1R */
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
	"-----------------------------------------------------------------------------\n");
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
	" fg1RVaild: %x\n", fg1RVaild);
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
	"-----------------------------------------------------------------------------\n");
	/* ---------------- */

	/* check 1R transition flag */
	if (fg1RVaild)
		return u1MaxWbRssiIdx;
	else
		return 0xFF;
}
#endif /* end ETSI_RX_BLOCKER_SUPPORT */

VOID asic_write_tmac_info_fixed_rate(struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR *tmac_info, MAC_TX_INFO *info, union _HTTRANSMIT_SETTING *pTransmit)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->write_tmac_info_fixed_rate)
		arch_ops->write_tmac_info_fixed_rate(pAd, wdev, tmac_info, info, pTransmit);
	else
		AsicNotSupportFunc(pAd, __func__);

	if (arch_ops->txd_post_process)
		arch_ops->txd_post_process(pAd, tmac_info, info, pTransmit);

	return;
}

VOID asic_dump_tmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->dump_tmac_info)
		return arch_ops->dump_tmac_info(pAd, tmac_info);
}

inline VOID asic_set_resource_state(struct _RTMP_ADAPTER *pAd, UCHAR resource_idx, BOOLEAN state)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->set_resource_state)
		arch_ops->set_resource_state(pAd, resource_idx, state);
	else
		AsicNotSupportFunc(pAd, __func__);
}

#ifdef SNIFFER_RADIOTAP_SUPPORT
UINT32 asic_trans_rxd_into_radiotap(RTMP_ADAPTER *pAd, VOID *rx_packet, struct _RX_BLK *rx_blk)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	UINT32 status = NDIS_STATUS_FAILURE;

	if (arch_ops->trans_rxd_into_radiotap)
		status = arch_ops->trans_rxd_into_radiotap(pAd, rx_packet, rx_blk);
	else
		AsicNotSupportFunc(pAd, __func__);

	return status;
}
#endif


INT32 asic_trans_rxd_into_rxblk(RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk, VOID *rx_pkt)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->trans_rxd_into_rxblk)
		return arch_ops->trans_rxd_into_rxblk(pAd, rx_blk, rx_pkt);
	else
		AsicNotSupportFunc(pAd, __func__);

	/* Return RMAC Info Length */
	return 0;
}

inline UINT32 asic_txdone_handle(RTMP_ADAPTER *pAd, VOID *ptr, UINT8 resource_idx)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->txdone_handle)
		return arch_ops->txdone_handle(pAd, ptr, resource_idx);
	else {
#ifdef HWIFI_SUPPORT
		MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR,
			 "This packet shall be handled by hwifi, CHECK!!\n");
#endif /* HWIFI_SUPPORT */
		return NDIS_STATUS_FAILURE;
	}
}

UINT32 asic_rxv_handler(RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk, VOID *rx_packet)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->rxv_handler)
		return arch_ops->rxv_handler(pAd, rx_blk, rx_packet);
	else
		return NDIS_STATUS_FAILURE;
}

VOID asic_dump_rmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->dump_rmac_info)
		return arch_ops->dump_rmac_info(pAd, rmac_info);
}

VOID asic_dump_rxinfo(struct _RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->dump_rx_info)
		return arch_ops->dump_rx_info(pAd, rmac_info);
}

VOID asic_dump_txs(struct _RTMP_ADAPTER *pAd, UINT8 format, CHAR *data)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->dump_txs)
		return arch_ops->dump_txs(pAd, format, data);
}

VOID asic_dump_rmac_info_for_ICVERR(struct _RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->dump_rmac_info_for_icverr)
		return arch_ops->dump_rmac_info_for_icverr(pAd, rmac_info);
}

INT asic_mlme_hw_tx(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info, MAC_TX_INFO *info, union _HTTRANSMIT_SETTING *pTransmit, struct _TX_BLK *tx_blk)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->mlme_hw_tx)
		return arch_ops->mlme_hw_tx(pAd, tmac_info, info, pTransmit, tx_blk);
	else
		AsicNotSupportFunc(pAd, __func__);
	return NDIS_STATUS_SUCCESS;
}

INT asic_hw_tx(struct _RTMP_ADAPTER *ad, struct _TX_BLK *tx_blk)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->hw_tx)
		return arch_ops->hw_tx(ad, tx_blk);
	else
		AsicNotSupportFunc(ad, __func__);
	return NDIS_STATUS_SUCCESS;
}

VOID asic_rx_event_handler(struct _RTMP_ADAPTER *ad, VOID *rx_packet)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->rx_event_handler)
		arch_ops->rx_event_handler(ad, rx_packet);
}

VOID asic_dump_wtbl_base_info(struct _RTMP_ADAPTER *pAd)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->dump_wtbl_base_info)
		return arch_ops->dump_wtbl_base_info(pAd);
}

VOID asic_dump_wtbl_info(struct _RTMP_ADAPTER *pAd, UINT16 wtbl_idx)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->dump_wtbl_info)
		return arch_ops->dump_wtbl_info(pAd, wtbl_idx);
}

VOID asic_wa_update(struct _RTMP_ADAPTER *ad)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->asic_wa_update)
		return arch_ops->asic_wa_update(ad);
}

#ifdef ERR_RECOVERY
INT asic_ser_handler(struct _RTMP_ADAPTER *ad, u32 action, u32 status)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->ser_handler)
		return arch_ops->ser_handler(ad, action, status);

	AsicNotSupportFunc(ad, __func__);
	return NDIS_STATUS_FAILURE;
}
#endif /* ERR_RECOVERY */

#if defined(NF_SUPPORT)

BOOLEAN asic_calculate_nf(struct _RTMP_ADAPTER *ad, UCHAR band_idx)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->arch_calculate_nf)
		arch_ops->arch_calculate_nf(ad, band_idx);
	else
		AsicNotSupportFunc(ad, __func__);
	return TRUE;
}

BOOLEAN asic_reset_enable_nf_registers(struct _RTMP_ADAPTER *ad, UCHAR band_idx)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->arch_reset_enable_nf_registers)
		arch_ops->arch_reset_enable_nf_registers(ad, band_idx);
	else
		AsicNotSupportFunc(ad, __func__);
	return TRUE;
}

BOOLEAN asic_enable_nf_support(struct _RTMP_ADAPTER *ad)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->arch_enable_nf_support)
		arch_ops->arch_enable_nf_support(ad);
	else
		AsicNotSupportFunc(ad, __func__);
	return TRUE;
}
#endif
VOID asic_calculate_ecc(struct _RTMP_ADAPTER *ad, UINT32 oper, UINT32 group, UINT8 *scalar, UINT8 *point_x, UINT8 *point_y)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->arch_calculate_ecc)
		arch_ops->arch_calculate_ecc(ad, oper, group, scalar, point_x, point_y);

	return;
}

inline UINT32 asic_get_bcn_tx_cnt(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->arch_get_bcn_tx_cnt)
		return arch_ops->arch_get_bcn_tx_cnt(pAd, BandIdx);
	else
		AsicNotSupportFunc(pAd, __func__);

	return NDIS_STATUS_FAILURE;
}

#ifdef AIR_MONITOR
INT asic_set_air_mon_enable(RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR band_idx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->arch_set_air_mon_enable)
		return arch_ops->arch_set_air_mon_enable(pAd, enable, band_idx);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT asic_set_air_mon_rule(RTMP_ADAPTER *pAd, UCHAR *rule, UCHAR band_idx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->arch_set_air_mon_rule)
		return arch_ops->arch_set_air_mon_rule(pAd, rule, band_idx);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT asic_set_air_mon_idx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR mnt_idx, UCHAR band_idx)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->arch_set_air_mon_idx)
		return arch_ops->arch_set_air_mon_idx(pAd, wdev, mnt_idx, band_idx);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}



#endif

#ifdef ACK_CTS_TIMEOUT_SUPPORT
INT asic_set_ack_timeout_mode_byband_by_fw(
	struct _RTMP_ADAPTER *pAd,
	UINT32 timeout,
	UINT32 bandidx,
	UINT8 ackmode)
{
	switch (ackmode) {
		case CCK_TIME_OUT: {
			set_ack_timeout_cr(pAd, WH_TX_ACK_CTS_TYPE_CCK_DCF_TIMEOUT, timeout);
			break;
		}
		case OFDM_TIME_OUT: {
			set_ack_timeout_cr(pAd, WH_TX_ACK_CTS_TYPE_OFDM_DCF_TIMEOUT, timeout);
			break;
		}
		case OFDMA_TIME_OUT: {
			set_ack_timeout_cr(pAd, WH_TX_ACK_CTS_TYPE_OFDMA_MU_DCF_TIMEOUT, timeout);
			break;
		}
		case ACK_ALL_TIME_OUT: {
			set_ack_timeout_cr(pAd, WH_TX_ACK_CTS_TYPE_CCK_DCF_TIMEOUT, timeout);
			set_ack_timeout_cr(pAd, WH_TX_ACK_CTS_TYPE_OFDM_DCF_TIMEOUT, timeout);
			set_ack_timeout_cr(pAd, WH_TX_ACK_CTS_TYPE_OFDMA_MU_DCF_TIMEOUT, timeout);
			break;
		}
		default: {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
				"invalid ackmode ,  ackmode=%d!!\n", ackmode);
			return FALSE;
		}
	}
	return TRUE;
}

INT32 asic_get_ack_timeout_mode_byband_by_fw(
	struct _RTMP_ADAPTER *pAd,
	UINT32 *ptimeout,
	UINT32 bandidx,
	UINT8 ackmode)
{
	UINT32 tmp_val = 0;

	switch (ackmode) {
		case CCK_TIME_OUT: {
			get_ack_timeout_bycr(pAd, WH_TX_ACK_CTS_TYPE_CCK_DCF_TIMEOUT, ptimeout);
			break;
		}
		case OFDM_TIME_OUT: {
			get_ack_timeout_bycr(pAd, WH_TX_ACK_CTS_TYPE_OFDM_DCF_TIMEOUT, ptimeout);
			break;
			}
		case OFDMA_TIME_OUT: {
			get_ack_timeout_bycr(pAd, WH_TX_ACK_CTS_TYPE_OFDMA_MU_DCF_TIMEOUT, ptimeout);
			break;
			}
		case ACK_ALL_TIME_OUT: {
			get_ack_timeout_bycr(pAd, WH_TX_ACK_CTS_TYPE_CCK_DCF_TIMEOUT, &tmp_val);
			*ptimeout = (tmp_val & MAX_ACK_TIMEOUT);
			get_ack_timeout_bycr(pAd, WH_TX_ACK_CTS_TYPE_OFDM_DCF_TIMEOUT, &tmp_val);
			*ptimeout = max(*ptimeout, (tmp_val & MAX_ACK_TIMEOUT));
			get_ack_timeout_bycr(pAd, WH_TX_ACK_CTS_TYPE_OFDMA_MU_DCF_TIMEOUT, &tmp_val);
			*ptimeout = max(*ptimeout, (tmp_val & MAX_ACK_TIMEOUT));
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
				"*ptimeout = %d us!!\n", (*ptimeout) & MAX_ACK_TIMEOUT);
				break;
			}
		default: {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"invalid ackmode ,  ackmode=%d!!\n", ackmode);
			return FALSE;
		}
	}
	return TRUE;
}
#endif /* ACK_CTS_TIMEOUT_SUPPORT */

#ifdef WF_RESET_SUPPORT
VOID asic_wf_subsys_reset(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	if (arch_ops->wf_subsys_reset)
		return arch_ops->wf_subsys_reset(ad);

	AsicNotSupportFunc(ad, __func__);

	return;
}
#endif

#ifdef TXRX_STAT_SUPPORT
UINT32 asic_get_mib_txrx_cnts(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->arch_get_mib_txrx_cnts)
		return arch_ops->arch_get_mib_txrx_cnts(pAd, BandIdx);

	AsicNotSupportFunc(pAd, __func__);

	return NDIS_STATUS_FAILURE;
}

UINT32 asic_get_all_rate_cnts(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx, UINT8 direction)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->arch_get_all_rate_cnts)
		return arch_ops->arch_get_all_rate_cnts(pAd, BandIdx, direction);

	AsicNotSupportFunc(pAd, __func__);

	return NDIS_STATUS_FAILURE;
}

UINT32 asic_get_stbc_cnts(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->arch_get_stbc_cnts)
		return arch_ops->arch_get_stbc_cnts(pAd, BandIdx);

	AsicNotSupportFunc(pAd, __func__);

	return NDIS_STATUS_FAILURE;
}

UINT32 asic_get_gi_cnts(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops->arch_get_gi_cnts)
		return arch_ops->arch_get_gi_cnts(pAd, BandIdx);

	AsicNotSupportFunc(pAd, __func__);

	return NDIS_STATUS_FAILURE;
}

#endif /* TXRX_STAT_SUPPORT */
