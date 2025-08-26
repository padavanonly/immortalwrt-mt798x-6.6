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
	config_red.c
*/

#include "rt_config.h"

#ifdef CFG_RED_SUPPORT
/*
static u32 au4RedFullThdDBDC2G5G[HWRED_MAX_THLD_NUM] = {15, 30, 45, 60, 75, 95, 100, 107};
static u32 au4RedDropThdDBDC2G5G[HWRED_MAX_BN_NUM][HWRED_MAX_THLD_NUM] = {
	{50, 100, 100, 100, 100, 100, 100, 100},
	{0, 0, 0, 0, 0, 25, 100, 100},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0}
};

u32 au4RedFullThdDBDC2G6G[HWRED_MAX_THLD_NUM] = {15, 30, 45, 60, 75, 95, 100, 107};
static u32 au4RedDropThdDBDC2G6G[HWRED_MAX_BN_NUM][HWRED_MAX_THLD_NUM] = {
	{0, 100, 100, 100, 100, 100, 100, 100},
	{0, 0, 0, 0, 0, 0, 100, 100},
	{0, 0, 0, 0, 0, 60, 90, 100},
	{0, 0, 0, 0, 0, 0, 100, 100}
};
*/
static u32 au4RedFullThdTBTC[HWRED_MAX_THLD_NUM] = {15, 30, 45, 60, 75, 95, 100, 107};
static u32 au4RedDropThdTBTC[HWRED_MAX_BN_NUM][HWRED_MAX_THLD_NUM] = {
	{0, 100, 100, 100, 100, 100, 100, 100},
	{0, 0, 0, 100, 100, 100, 100, 100},
	{0, 0, 0, 0, 100, 100, 100, 100},
	{0, 0, 0, 0, 0, 0, 100, 100}
};

static u16 auACTailDropMinThldSingleBand[HWRED_MAX_BN_NUM][AC_NUM] = {
	{900, 1500, 1900, 1900},
	{900, 1500, 1900, 1900},
	{900, 1500, 1900, 1900},
	{900, 1500, 1900, 1900}
};
/*
static u16 auACTailDropMinThldTBTC[HWRED_MAX_BN_NUM][AC_NUM] = {
	{90, 150, 190, 190},
	{450, 750, 950, 950},
	{900, 1500, 1900, 1900},
	{900, 1500, 1900, 1900}
};
*/
static u16 auACTailDropMaxThldSingleBand[HWRED_MAX_BN_NUM][AC_NUM] = {
	{900, 1500, 1900, 1900},
	{900, 1500, 1900, 1900},
	{900, 1500, 1900, 1900},
	{900, 1500, 1900, 1900}
};
/*
static u16 auACTailDropMaxThldTBTC[HWRED_MAX_BN_NUM][AC_NUM] = {
	{HWRED_MAX_PAGE_NUM - 1, HWRED_MAX_PAGE_NUM - 1,
	HWRED_MAX_PAGE_NUM - 1, HWRED_MAX_PAGE_NUM - 1},
	{HWRED_MAX_PAGE_NUM - 1, HWRED_MAX_PAGE_NUM - 1,
	HWRED_MAX_PAGE_NUM - 1, HWRED_MAX_PAGE_NUM - 1},
	{HWRED_MAX_PAGE_NUM - 1, HWRED_MAX_PAGE_NUM - 1,
	HWRED_MAX_PAGE_NUM - 1, HWRED_MAX_PAGE_NUM - 1},
	{HWRED_MAX_PAGE_NUM - 1, HWRED_MAX_PAGE_NUM - 1,
	HWRED_MAX_PAGE_NUM - 1, HWRED_MAX_PAGE_NUM - 1}
};
*/

static INT32 red_cmd_cr4_set(
	struct _RTMP_ADAPTER *pAd, UINT32 arg0, UINT32 u4ExtSize, PUINT8 pExtData);

static BOOLEAN acm_send_cmd(
	struct _RTMP_ADAPTER *pAd, u8 ucOpMode, u8 *pr_red_param, u16 sta_num);

static int acm_config_sta_cmd(struct _RTMP_ADAPTER *pAd, struct red_cmd_admctrl_config *param_config);
static int acm_reset_sta_ctrl(struct _RTMP_ADAPTER *pAd);
static int acm_config_sta(struct _RTMP_ADAPTER *pAd, char *arg_sta);
static int acm_dump_drop(struct _RTMP_ADAPTER *pAd, UINT_16 mld_id1, UINT_16 mld_idn, BOOLEAN fgReset);
static int acm_cmd_help(struct _RTMP_ADAPTER *pAd);
static int acm_dump_setting(struct _RTMP_ADAPTER *pAd);
static int acm_init(struct _RTMP_ADAPTER *pAd);

#if (CFG_RED_HOST_RED == 1)
static void red_bad_node(
	u16 u2WlanIdx, u8 ATC_WATF_Enable, struct _RTMP_ADAPTER *pAd)
{
	struct red_sta_info *prRedSta = PD_GET_RED_STA_BY_IDX(pAd->physical_dev, u2WlanIdx);
	struct red_ac_element *prAcElm = &prRedSta->arRedElm[0];
	PMAC_TABLE_ENTRY pEntry = entry_get(pAd, u2WlanIdx);
	struct BA_INFO *ba_info = &pEntry->ba_info;
	UINT8 i;
	UINT8 ucGoodNodeCnt = 0;
	UINT8 ucBadNodeCnt = 0;
	UINT8 ucIsBadNode = FALSE;
	UINT8 ucIsSetDefault = FALSE;
	UINT16 pkt_buf_cnt;

	for (i = WMM_AC_BK; i < WMM_NUM_OF_AC; i++) {
		ucBadNodeCnt = (prAcElm->ucGBCnt & RED_BAD_NODE_CNT_MASK);
		ucGoodNodeCnt =
			(prAcElm->ucGBCnt & RED_GOOD_NODE_CNT_MASK) >> RED_GOOD_NODE_CNT_SHIFT_BIT;
		ucIsBadNode =
			(prAcElm->ucGBCnt & RED_IS_BAD_NODE_MASK) >> RED_IS_BAD_NODE_SHIFT_BIT;

		/* Check Is Bad Node or not */
		pkt_buf_cnt = prAcElm->u2EnqueueCnt - prAcElm->u2DequeueCnt;
		if (pkt_buf_cnt >= (prRedSta->u2Dropth >> prAcElm->ucShiftBit)) {
			/*
			 * Drop packet immediately if ATC for WATF enable
			 * to prevent slow client occupy much PLE buffer.
			 *
			 */
			if (ATC_WATF_Enable)
				ucBadNodeCnt = RED_MAX_BAD_NODE_CNT;
			else
				ucBadNodeCnt++;

			ucGoodNodeCnt = 0;
		} else {
			/* Good Node */
			ucGoodNodeCnt++;
			ucBadNodeCnt = 0;
		}

		if ((prAcElm->ucShiftBit > 0) &&
			(prRedSta->u2Dropth >> prAcElm->ucShiftBit)
				<= (RED_BAD_NODE_DROP_THRESHOLD * QLEN_SCALED)) {
			if (ba_info->TxBitmap != 0)
				/*HT and VHT */
				prRedSta->u2Dropth = (RED_BAD_NODE_DROP_THRESHOLD * QLEN_SCALED)
					<< prAcElm->ucShiftBit;
			else
				/*Legacy */
				prRedSta->u2Dropth =
					(RED_BAD_NODE_LEGACY_DEFAULT_THRESHOLD * QLEN_SCALED)
					<< prAcElm->ucShiftBit;

			ucIsSetDefault = TRUE;
		}

		if (ucBadNodeCnt >= RED_MAX_BAD_NODE_CNT) {
			ucBadNodeCnt = 0;
			ucIsBadNode = TRUE;
			if (ATC_WATF_Enable && !ucIsSetDefault)
				prAcElm->ucShiftBit++;
		}

		if (ucGoodNodeCnt >= RED_MAX_GOOD_NODE_CNT) {
			ucGoodNodeCnt = 0;

			if (prAcElm->ucShiftBit > 0)
				prAcElm->ucShiftBit--;
			else	/*ucShiftBit == 0 */
				ucIsBadNode = FALSE;
		}

/*
ucGBCnt
 ----------------------------------------------------
Bits |       7          |       6|      5|      4       |       3|      2|      1|      0       |
      |IsBadNode   |GoodNodeCnt                |       BadNodeCnt                    |
----------------------------------------------------
*/
		prAcElm->ucGBCnt = ((ucIsBadNode << RED_IS_BAD_NODE_SHIFT_BIT) |
						(ucGoodNodeCnt << RED_GOOD_NODE_CNT_SHIFT_BIT) |
						(ucBadNodeCnt));
		prAcElm++;
	}
}

static void red_update_target_delay(
	u8 ATC_WATF_Enable, struct _RTMP_ADAPTER *pAd)
{
	if (ATC_WATF_Enable)
		PD_SET_RED_TARGET_DELAY(
			pAd->physical_dev, PD_GET_RED_ATM_ON_TARGET_DELAY(pAd->physical_dev));
	else
		PD_SET_RED_TARGET_DELAY(
			pAd->physical_dev, PD_GET_RED_ATM_OFF_TARGET_DELAY(pAd->physical_dev));
}

/* Timer */
DECLARE_TIMER_FUNCTION(red_badnode_timeout);
VOID red_badnode_timeout(PVOID SystemSpecific1, PVOID FunctionContext,
			PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	PMAC_TABLE_ENTRY pEntry;
	STA_TR_ENTRY *tr_entry;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	UINT8 fgATCEnable = 0;
	UINT8 fgATFEnable = 0;
	UINT8 fgWATFEnable = 0;
	UINT8 fgATCorWATFEnable = 0;
	UINT16 u2WlanIdx;
#ifdef VOW_SUPPORT
	fgATCEnable = pAd->vow_cfg.en_bw_ctrl;
	fgATFEnable = pAd->vow_cfg.en_airtime_fairness;
	fgWATFEnable = pAd->vow_watf_en;
#endif /* VOW__SUPPORT */
	fgATCorWATFEnable = fgATCEnable || (fgATFEnable && fgWATFEnable);

	for (u2WlanIdx = 0 ; u2WlanIdx < RED_STA_REC_NUM; u2WlanIdx++) {
		pEntry = entry_get(pAd, u2WlanIdx);
		if (IS_ENTRY_NONE(pEntry))
			continue;
		if (!VALID_UCAST_ENTRY_WCID(pAd, pEntry->wcid))
			continue;

		tr_entry = tr_entry_get(pAd, pEntry->tr_tb_idx);
		if (tr_entry->StaRec.ConnectionState == STATE_PORT_SECURE)
			red_bad_node(u2WlanIdx, fgATCorWATFEnable, pAd);
	}

	red_update_target_delay(fgATCorWATFEnable, pAd);
}
BUILD_TIMER_FUNCTION(red_badnode_timeout);
#endif
#if (CFG_RED_RAAC_TAIL_DROP == 1)
static BOOLEAN red_send_cmd(
	struct _RTMP_ADAPTER *pAd, u8 ucOpMode, u8 *pr_red_param, u16 sta_num)
{
	BOOLEAN ret = FALSE;
	UINT_8 u1BandIdx;
	PUINT_8 param = NULL;
	UINT_32 *param_in;
	struct red_cmd_header *param_header;
	struct red_cmd_watermark *param_global;
	struct red_cmd_raac_thres *param_raac;
	struct red_cmd_release_raac_thres *param_rel_raac;
	struct red_cmd_set_ctrl *param_ctrl;
	UINT_32 u4ExtSize;
	UINT_16 u2Idx;
	struct red_ctrl *prRedCtrl;

	if (!PD_GET_RED_MCU_OFFLOAD(pAd->physical_dev))
		goto error;

	u4ExtSize = sizeof(struct red_cmd_header);
	if ((ucOpMode == RED_SET_GLOBAL_WATERMARK)
		|| (ucOpMode == RED_SET_GLOBAL_TOKEN_WATERMARK)
		|| (ucOpMode == RED_SET_GLOBAL_PAGE_WATERMARK)
		|| (ucOpMode == RED_SET_GLOBAL_BAND_WATERMARK)) {
		prRedCtrl = (struct red_ctrl *)pr_red_param;

		u4ExtSize += sizeof(struct red_cmd_watermark);

		os_alloc_mem(pAd, (PUCHAR *)&param, u4ExtSize);

		if (param == NULL)
			goto error;

		os_zero_mem(param, u4ExtSize);

		param_global = (struct red_cmd_watermark *)(param + sizeof(struct red_cmd_header));
		param_global->u2AllTokenHighMark = prRedCtrl->u2AllTokenHighMark;
		param_global->u2AllTokenLowMark = prRedCtrl->u2AllTokenLowMark;
		param_global->u2PageHighMark = prRedCtrl->u2PageHighMark;
		param_global->u2PageLowMark = prRedCtrl->u2PageLowMark;
		for (u1BandIdx = 0; u1BandIdx < CFG_WIFI_RAM_BAND_NUM; u1BandIdx++) {
			param_global->u2TokenHighMark[u1BandIdx] = prRedCtrl->u2TokenHighMark[u1BandIdx];
			param_global->u2TokenLowMark[u1BandIdx] = prRedCtrl->u2TokenLowMark[u1BandIdx];
		}
	}

	if (ucOpMode == RED_SET_STA_THRES_BY_HOST) {
		param_in = (UINT_32 *)pr_red_param;
		os_alloc_mem(pAd,
			(PUCHAR *)&param, u4ExtSize + sta_num*sizeof(struct red_cmd_raac_thres));

		if (param == NULL)
			goto error;

		os_zero_mem(param, u4ExtSize + sta_num*sizeof(struct red_cmd_raac_thres));

		param_raac = (struct red_cmd_raac_thres *)(param + sizeof(struct red_cmd_header));

		for (u2Idx = 0; u2Idx < sta_num; u2Idx++) {
			param_raac->u2Idx = (UINT_16)param_in[0];
			param_raac->ucAc = (UINT_8)param_in[1];
			param_raac->u2TokenHighMark = (UINT_16)param_in[2];
			param_raac->u2TokenLowMark = (UINT_16)param_in[3];
			param_raac->u2PageHighMark = (UINT_16)param_in[4];
			param_raac->u2PageLowMark = (UINT_16)param_in[5];
			param_raac->u2TokenComp = (UINT_16)param_in[6];
			param_raac->u2PageComp = (UINT_16)param_in[7];
			param_raac++;
			param_in += 8;
		}
		u4ExtSize += sizeof(struct red_cmd_raac_thres)*sta_num;
	}

	if (ucOpMode == RED_RELEASE_STA_THRES_FROM_HOST) {
		param_in = (UINT_32 *)pr_red_param;
		os_alloc_mem(pAd,
			(PUCHAR *)&param,
			u4ExtSize + sta_num*sizeof(struct red_cmd_release_raac_thres));

		if (param == NULL)
			goto error;

		os_zero_mem(param, u4ExtSize + sta_num*sizeof(struct red_cmd_release_raac_thres));

		param_rel_raac =
			(struct red_cmd_release_raac_thres *)(param + sizeof(struct red_cmd_header));

		for (u2Idx = 0; u2Idx < sta_num; u2Idx++) {
			param_rel_raac->u2Idx = (u16)param_in[0];
			param_rel_raac->ucAc = (u8)param_in[1];
			param_rel_raac++;
			param_in += 8;
		}
		u4ExtSize += sizeof(struct red_cmd_release_raac_thres)*sta_num;
	}


	if (ucOpMode == RED_SET_CTRL) {
		prRedCtrl = (struct red_ctrl *)pr_red_param;

		os_alloc_mem(pAd, (PUCHAR *)&param, u4ExtSize + sizeof(struct red_cmd_set_ctrl));

		if (param == NULL)
			goto error;

		os_zero_mem(param, u4ExtSize + sizeof(struct red_cmd_set_ctrl));

		param_ctrl = (struct red_cmd_set_ctrl *)(param + sizeof(struct red_cmd_header));
		param_ctrl->fgEnable = prRedCtrl->fgEnable;
		param_ctrl->fgDbgShow = prRedCtrl->fgDbgShow;
		param_ctrl->fgDbgNoDrop = prRedCtrl->fgDbgNoDrop;
		param_ctrl->tx_bh_period = prRedCtrl->tx_bh_period;
		param_ctrl->rx_bh_period = prRedCtrl->rx_bh_period;
		param_ctrl->u1PfmEvent1 = prRedCtrl->u1PfmEvent1;
		param_ctrl->u1PfmEvent2 = prRedCtrl->u1PfmEvent2;

		u4ExtSize += sizeof(struct red_cmd_set_ctrl);
	}

	if ((ucOpMode == RED_DUMP_CTRL) || (param == NULL)) {
		os_alloc_mem(pAd, (PUCHAR *)&param, u4ExtSize);

		if (param == NULL)
			goto error;

		os_zero_mem(param, u4ExtSize);
	}

	param_header = (struct red_cmd_header *)param;
	param_header->ucOpMode = ucOpMode;
	param_header->ucCmdVer = 0;
	param_header->u2CmdLen = u4ExtSize;

	red_cmd_cr4_set(pAd, WA_SET_OPTION_RED_SETTING_CMD, u4ExtSize, (PUINT_8)param);
	os_free_mem(param);
	ret = TRUE;
error:
	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			 "(ret = %d)\n", ret);

	return ret;
}
#endif
#if (CFG_RED_RAAC_TAIL_DROP == 1)
static void red_tail_drop_init(struct _RTMP_ADAPTER *pAd)
{
	UINT_16 u2TokenCnt;
	RTMP_CHIP_CAP *chip_cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	bool whnat_en = false;
	struct red_ctrl *red_ctrl = PD_GET_RED_CTRL_PTR(pAd->physical_dev);

	os_zero_mem(red_ctrl, sizeof(struct red_ctrl));

	u2TokenCnt = chip_cap->tkn_info.token_tx_cnt;

#ifdef WHNAT_SUPPORT
	if (PD_GET_WHNAT_ENABLE(pAd->physical_dev)) {
		u2TokenCnt = chip_cap->tkn_info.hw_tx_token_cnt;
		whnat_en = true;
	}
#endif /* WHNAT_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_WARN,
		"available token:%u, whnat_en:%u\n", u2TokenCnt, whnat_en);

	red_ctrl->u2AllTokenHighMark = u2TokenCnt - 256;
	red_ctrl->u2AllTokenLowMark = red_ctrl->u2AllTokenHighMark - 1536;

	red_send_cmd(pAd, RED_SET_GLOBAL_TOKEN_WATERMARK, (PUINT_8)red_ctrl, 0);
}
#endif

void red_setting_init(struct physical_device *ph_dev)
{
	PD_SET_RED_ENABLE(ph_dev, TRUE);
	PD_SET_RED_TARGET_DELAY(ph_dev, 20000);
	PD_SET_RED_ATM_ON_TARGET_DELAY(ph_dev, 15000);
	PD_SET_RED_ATM_OFF_TARGET_DELAY(ph_dev, 20000);
	PD_SET_RED_STA_NUM(ph_dev, 0);
}

void red_init(struct _RTMP_ADAPTER *ad)
{
	RTMP_CHIP_CAP *pChipCap = PD_GET_CHIP_CAP_PTR(ad->physical_dev);
	UINT32 red_en_type;

	/* Only init once. */
	if (hwifi_get_inf_num(ad) != 0)
		return;
	if (pChipCap->asic_caps & fASIC_CAP_MCU_OFFLOAD)
		PD_SET_RED_MCU_OFFLOAD(ad->physical_dev, TRUE);
	else
		PD_SET_RED_MCU_OFFLOAD(ad->physical_dev, FALSE);

	/* Send cmd to enable N9 MPDU timer */
	if (PD_GET_RED_MCU_OFFLOAD(ad->physical_dev))
		red_en_type = RED_BY_WA_ENABLE;
	else
		red_en_type = RED_BY_HOST_ENABLE;


#ifdef WIFI_UNIFIED_COMMAND
	UniCmdSetRedEnable(ad, HOST2N9,
		((PD_GET_RED_ENABLE(ad->physical_dev) > 0) ? red_en_type : RED_DISABLE));
#endif /* WIFI_UNIFIED_COMMAND */

	/* For 7615 (CR4 offload)*/
	if (PD_GET_RED_MCU_OFFLOAD(ad->physical_dev)) {
		MtCmdCr4Set(ad, CR4_SET_ID_RED_ENABLE, PD_GET_RED_ENABLE(ad->physical_dev), 0);
		MTWF_DBG(ad, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			"set CR4/N9 RED Enable to %d.\n", PD_GET_RED_ENABLE(ad->physical_dev));
	} else {
#if (CFG_RED_HOST_RED == 1)
		if (PD_GET_RED_ENABLE(ad->physical_dev)) {
			struct red_ac_element *prAcElm = NULL;
			struct red_sta_info *prRedSta = PD_GET_RED_STA_BY_IDX(ad->physical_dev, 0);
			int i, j;

			/*For 7622 (No CR4), need to initial the parameter on driver */
			for (i = 0; i < RED_STA_REC_NUM; i++) {
				prRedSta->i4MpduTime = RED_MPDU_TIME_INIT;	/*us */
				prRedSta->u2Dropth = RED_HT_BW40_DEFAULT_THRESHOLD * QLEN_SCALED;
				prRedSta->ucMultiplyNum = RED_MULTIPLE_NUM_DEFAULT;
				prRedSta->u2DriverFRCnt = 0;
				prRedSta->tx_msdu_avg_cnt = 0;
				prRedSta->tx_msdu_cnt = 0;
				prAcElm = &prRedSta->arRedElm[0];

				for (j = WMM_AC_BK; j < WMM_NUM_OF_AC; j++) {
					prAcElm->u2TotalDropCnt = 0;
					prAcElm->u2DropCnt = 0;
					prAcElm->u2EnqueueCnt = 0;
					prAcElm->u2DequeueCnt = 0;
					prAcElm->u2qEmptyCnt = 0;
					prAcElm->ucShiftBit = 0;
					prAcElm->ucGBCnt = 0;
					prAcElm++;
				}

				prRedSta++;
			}
			RTMPInitTimer(ad, PD_GET_RED_BAD_NODE_TIMER_PTR(ad->physical_dev),
				GET_TIMER_FUNCTION(red_badnode_timeout), ad, TRUE);
			RTMPSetTimer(PD_GET_RED_BAD_NODE_TIMER_PTR(ad->physical_dev),
				BADNODE_TIMER_PERIOD);
		}
#endif
		MTWF_DBG(ad, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
				"set Driver/N9 RED Enable to %d.\n",
				PD_GET_RED_ENABLE(ad->physical_dev));
	}

#if (CFG_RED_RAAC_TAIL_DROP == 1)
	if (PD_GET_RED_MCU_OFFLOAD(ad->physical_dev))
		red_tail_drop_init(ad);
#endif
#if (CFG_RED_HW_ACM == 1)
	if (PD_GET_RED_MCU_OFFLOAD(ad->physical_dev)) {
		acm_init(ad);
		HW_SET_PBC_CTRL(ad, NULL, NULL, PBC_TYPE_NORMAL);
	}
#endif
	MTWF_DBG(ad, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR, "RED Initiailize Done.\n");
}

/* Command */
INT set_red_enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 en, rv;
	UINT32 red_en_type;

	if (arg) {
		rv = sscanf(arg, "%d", &en);

		if ((rv > 0) && (en <= 1)) {
			PD_SET_RED_ENABLE(pAd->physical_dev, en);
			/* to CR4 & N9 */
			if (PD_GET_RED_MCU_OFFLOAD(pAd->physical_dev))
				red_en_type = RED_BY_WA_ENABLE;
			else
				red_en_type = RED_BY_HOST_ENABLE;
#ifdef WIFI_UNIFIED_COMMAND
				UniCmdSetRedEnable(pAd, HOST2N9,
					((en > 0) ? red_en_type : RED_DISABLE));
#endif /* WIFI_UNIFIED_COMMAND */
#if (CFG_RED_HOST_RED == 1)
			if (!PD_GET_RED_MCU_OFFLOAD(pAd->physical_dev)) {
				BOOLEAN Cancelled;

			if (en == 0) {
				if (PD_IS_RED_BAD_NODE_TIMER_VALID(pAd->physical_dev))
					RTMPReleaseTimer(
						PD_GET_RED_BAD_NODE_TIMER_PTR(pAd->physical_dev),
						&Cancelled);
			} else {
				if (!PD_IS_RED_BAD_NODE_TIMER_VALID(pAd->physical_dev)) {
					RTMPInitTimer(pAd,
						PD_GET_RED_BAD_NODE_TIMER_PTR(pAd->physical_dev),
						GET_TIMER_FUNCTION(red_badnode_timeout),
						pAd, TRUE);
					RTMPSetTimer(
						PD_GET_RED_BAD_NODE_TIMER_PTR(pAd->physical_dev),
						BADNODE_TIMER_PERIOD);
				}
			}
			}
#endif
			if (PD_GET_RED_MCU_OFFLOAD(pAd->physical_dev)) {
				MtCmdCr4Set(pAd, CR4_SET_ID_RED_ENABLE,
					PD_GET_RED_ENABLE(pAd->physical_dev), 0);
				MTWF_PRINT("%s: set CR4/N9 RED Enable to %d.\n",
					__func__, PD_GET_RED_ENABLE(pAd->physical_dev));
			} else {
				MTWF_PRINT("%s: set Driver/N9 RED Enable to %d.\n",
					__func__, PD_GET_RED_ENABLE(pAd->physical_dev));
			}
		} else if (en == 2) {
			;
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_red_show_sta(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 sta, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &sta);

		if ((rv > 0) && (IS_WCID_VALID(pAd, sta))) {
			if (PD_GET_RED_MCU_OFFLOAD(pAd->physical_dev)) {
				MtCmdCr4Set(pAd, CR4_SET_ID_RED_SHOW_STA, sta, 0);
				MTWF_PRINT("%s: set CR4 RED show sta to %d.\n", __func__, sta);
			} else {
				PD_SET_RED_STA_NUM(pAd->physical_dev, sta);
				MTWF_PRINT("%s: set Driver RED show sta to %d.\n", __func__, sta);
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_red_config(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT_32 cmd = 3, rv, ret = 1;
	UINT_8 ucOpMode = 0;
	UINT32 param[10] = {0};
	BOOLEAN fgApply = FALSE;
	UINT_8 idx = 0, idx2 = 0;
	struct red_ctrl *red_ctrl = PD_GET_RED_CTRL_PTR(pAd->physical_dev);

	if (arg) {
		rv = sscanf(arg, "%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u", &cmd, &param[0], &param[1], &param[2], &param[3],
								&param[4], &param[5], &param[6], &param[7], &param[8], &param[9]);

		if (rv == 0)
			cmd = 100;

		switch (cmd) {
		case 0:
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
				"set priority/tcp offset\n");

			red_ctrl->u2PriorityOffset = (UINT_16)param[0];
			red_ctrl->u2TCPOffset = (UINT_16)param[1];

			ucOpMode = ACM_OFFSET_CONFIG;

			if (param[2] == 1)
				fgApply = TRUE;
			break;
		case 1:
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
				"set total token count\n");

			for (idx = 0; idx < HWRED_MAX_SRC_NUM ; idx++)
				red_ctrl->u2TokenPerSrc[idx] = (UINT_16)param[idx];

			ucOpMode = ACM_TOKEN_CONFIG;

			if (param[4] == 1)
				fgApply = TRUE;
			break;
		case 2:
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
				"set total token thld\n");

			for (idx = 0; idx < HWRED_MAX_SRC_NUM ; idx++)
				red_ctrl->u2TokenThldPerSrc[idx] = (UINT_16)param[idx];

			ucOpMode = ACM_TKID_TAIL_DROP_CONFIG;

			if (param[4] == 1)
				fgApply = TRUE;
			break;

		case 3:
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
				"set RED full thld for TBL%u\n", param[0]);

			idx = param[0];

			for (idx2 = 0; idx2 < HWRED_MAX_THLD_NUM ; idx2++)
				red_ctrl->u2FullTbl[idx][idx2] = (UINT_16)param[idx2 + 1];

			ucOpMode = ACM_PER_BAND_RED_CONFIG;

			if (param[9] == 1)
				fgApply = TRUE;
			break;

		case 4:
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
				"set RED drop thld for TBL%u\n", param[0]);

			idx = param[0];

			for (idx2 = 0; idx2 < HWRED_MAX_THLD_NUM ; idx2++)
				red_ctrl->u2DropTbl[idx][idx2] = (UINT_16)param[idx2 + 1];

			ucOpMode = ACM_PER_BAND_RED_CONFIG;

			if (param[9] == 1)
				fgApply = TRUE;
			break;

		case 5:
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
				"set ac tail drop band%u min thld\n", param[0]);

			idx = param[0];

			for (idx2 = 0; idx2 < AC_NUM ; idx2++)
				red_ctrl->u2ACTailDropMin[idx][idx2] = (UINT_16)param[idx2 + 1];

			ucOpMode = ACM_AC_TAIL_DROP_CONFIG;

			if (param[5] == 1)
				fgApply = TRUE;
			break;

		case 6:
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
				"set ac tail drop band%u max thld\n", param[0]);

			idx = param[0];

			for (idx2 = 0; idx2 < AC_NUM ; idx2++)
				red_ctrl->u2ACTailDropMax[idx][idx2] = (UINT_16)param[idx2 + 1];

			ucOpMode = ACM_AC_TAIL_DROP_CONFIG;

			if (param[5] == 1)
				fgApply = TRUE;
			break;
		case 7:
			acm_dump_setting(pAd);

			ucOpMode = ACM_DUMP_CONFIG;
			fgApply = TRUE;
			break;
		case 8:
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
				"set sta red_dis and rate_shift\n");

			if (param[0] > 0)
				acm_config_sta(pAd, arg + sizeof(char)*2);
			if ((param[0] == 0) && (param[1] == 0))
				acm_reset_sta_ctrl(pAd);
			if ((param[0] == 0) && (param[1] == 1) && (red_ctrl->sta_config_cnt > 0))
				fgApply = TRUE;

			ucOpMode = ACM_STA_CONFIG;
			break;
		case 9:
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
				"set which sta to dump drop counter\n");

			acm_dump_drop(pAd, param[0], param[1], param[2]);

			ucOpMode = 0;
			break;
		case 10:
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
				"release HWRED control\n");

			ucOpMode = ACM_RELEASE_CONFIG;

			fgApply = TRUE;
			break;
		default:
			acm_cmd_help(pAd);
			break;
		}
	} else
		acm_cmd_help(pAd);

	if (fgApply) {
		if ((ucOpMode > 0) && (ucOpMode < ACM_CMD_MAX))
			acm_send_cmd(pAd, ucOpMode, (PUINT_8)red_ctrl, 0);
	}

	return ret;
}

static INT32 red_cmd_cr4_set(
	struct _RTMP_ADAPTER *pAd, UINT32 arg0, UINT32 u4ExtSize, PUINT8 pExtData)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	struct _EXT_CMD_CR4_SET_T  CmdCr4SetSet;
	struct _CMD_ATTRIBUTE attr = {0};

#ifdef WIFI_UNIFIED_COMMAND
	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_SDO_ONLY))
		return UniCmdSetSdo(pAd, arg0, 0, 0, u4ExtSize, pExtData);
#endif /* WIFI_UNIFIED_COMMAND */

	msg = AndesAllocCmdMsg(pAd, sizeof(CmdCr4SetSet) + u4ExtSize);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		return Ret;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, INIT_CMD_ID_CR4);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CR4_SET);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdCr4SetSet, sizeof(CmdCr4SetSet));
	CmdCr4SetSet.u4Cr4SetArg0 = cpu2le32(arg0);
	CmdCr4SetSet.u4Cr4SetArg1 = 0;
	CmdCr4SetSet.u4Cr4SetArg2 = 0;
	AndesAppendCmdMsg(msg, (char *)&CmdCr4SetSet, sizeof(CmdCr4SetSet));
	AndesAppendCmdMsg(msg, (char *)pExtData, u4ExtSize);

	Ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
		"(ret = %d)\n", Ret);
	return Ret;
}

static BOOLEAN acm_send_cmd(
	struct _RTMP_ADAPTER *pAd, u8 ucOpMode, u8 *pr_red_param, u16 sta_num)
{
	BOOLEAN ret = FALSE;
	UINT_8 idx, idx2;
	PUINT_8 param = NULL;
	struct red_cmd_header *param_header;
	struct red_cmd_admctrl_config *param_config;
	UINT_32 u4ExtSize;
	struct red_ctrl *prRedCtrl;

	if (!PD_GET_RED_MCU_OFFLOAD(pAd->physical_dev))
		goto error_acm;

	u4ExtSize = sizeof(struct red_cmd_header);
	u4ExtSize += sizeof(struct red_cmd_admctrl_config);

	os_alloc_mem(pAd, (PUCHAR *)&param, u4ExtSize);

	if (param == NULL)
		goto error_acm;

	os_zero_mem(param, u4ExtSize);

	prRedCtrl = (struct red_ctrl *)pr_red_param;

	param_config = (struct red_cmd_admctrl_config *)(param + sizeof(struct red_cmd_header));

	if (ucOpMode == ACM_STA_CONFIG)
		acm_config_sta_cmd(pAd, param_config);

	param_config->global.u2TCPOffset = prRedCtrl->u2TCPOffset;
	param_config->global.u2PriorityOffset = prRedCtrl->u2PriorityOffset;

	for (idx = 0; idx < HWRED_MAX_SRC_NUM; idx++) {
		param_config->global.u2TokenPerSrc[idx] = prRedCtrl->u2TokenPerSrc[idx];
		param_config->global.u2TokenThldPerSrc[idx] = prRedCtrl->u2TokenThldPerSrc[idx];
	}

	for (idx = 0; idx < HWRED_MAX_BN_NUM; idx++) {
		for (idx2 = 0; idx2 < HWRED_MAX_THLD_NUM; idx2++) {
			param_config->global.u2FullTbl[idx][idx2] = prRedCtrl->u2FullTbl[idx][idx2];
			param_config->global.u2DropTbl[idx][idx2] = prRedCtrl->u2DropTbl[idx][idx2];
		}
	}

	for (idx = 0; idx < HWRED_MAX_BN_NUM; idx++) {
		for (idx2 = 0; idx2 < HWRED_MAX_AC_QUE_NUM; idx2++) {
			param_config->global.u2ACTailDropMin[idx][idx2] = prRedCtrl->u2ACTailDropMin[idx][idx2];
			param_config->global.u2ACTailDropMax[idx][idx2] = prRedCtrl->u2ACTailDropMax[idx][idx2];
		}
	}

	param_header = (struct red_cmd_header *)param;
	param_header->ucOpMode = ucOpMode;
	param_header->ucCmdVer = 0;
	param_header->u2CmdLen = u4ExtSize;

	red_cmd_cr4_set(pAd, WA_SET_OPTION_RED_SETTING_CMD, u4ExtSize, (PUINT_8)param);
	os_free_mem(param);
	ret = TRUE;

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			 "(ret = %d, size = %d)\n", ret, u4ExtSize);
error_acm:
	return ret;
}

static int acm_cmd_help(struct _RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			"red_config=0-[red priority offset]-[red tcp offset]-[1:apply,0:not yet]\n");

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			"red_config=1-[tkn cnt src0]-[src1]-[src2]-[src3]-[1:apply,0:not yet]\n");

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			"red_config=2-[tkn thld src0]-[src1]-[src2]-[src3]-[1:apply,0:not yet]\n");

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			"red_config=3-[band]-[red full thld0]-[1]-[2]-[3]-[4]-[5]-[6]-[7]-[1:apply,0:not yet]\n");

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			"red_config=4-[band]-[red drop thld0]-[1]-[2]-[3]-[4]-[5]-[6]-[7]-[1:apply,0:not yet]\n");

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			"red_config=5-[band]-[ac tail drop min thld ac0]-[ac1]-[ac2]-[ac3]-[1:apply,0:not yet]\n");

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			"red_config=6-[band]-[ac tail drop max thld ac0]-[ac1]-[ac2]-[ac3]-[1:apply,0:not yet]\n");

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			"red_config=7 [dump setting]\n");

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			"red_config=8-[mld_id]-[red disable=1,enable=0]-[red rate shift]-[red rate shift apply=1,not=0]\n");

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			"red_config=9-[mld_id1=0:dump all]-[mld_idN]-[1:reset,0:keep] {dump drop count}\n");

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			"red_config=10 { release host config control}\n");

	return 0;
}

static int acm_dump_setting(struct _RTMP_ADAPTER *pAd)
{
	int idx = 0;
	struct red_ctrl *red_ctrl = PD_GET_RED_CTRL_PTR(pAd->physical_dev);

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			 "priority/tcp offset: %u / %u\n", red_ctrl->u2PriorityOffset, red_ctrl->u2TCPOffset);

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			 "Token Cnt : %u %u %u %u\n", red_ctrl->u2TokenPerSrc[0], red_ctrl->u2TokenPerSrc[1],
			 red_ctrl->u2TokenPerSrc[2], red_ctrl->u2TokenPerSrc[3]);

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			 "Tkid tail drop Thld : %u %u %u %u\n", red_ctrl->u2TokenThldPerSrc[0], red_ctrl->u2TokenThldPerSrc[1],
			 red_ctrl->u2TokenThldPerSrc[2], red_ctrl->u2TokenThldPerSrc[3]);


	for (idx = 0 ; idx < HWRED_MAX_BN_NUM; idx++) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			 "RED Full Thld TBL%u: %u %u %u %u %u %u %u %u\n", idx, red_ctrl->u2FullTbl[idx][0], red_ctrl->u2FullTbl[idx][1],
			 red_ctrl->u2FullTbl[idx][2], red_ctrl->u2FullTbl[idx][3], red_ctrl->u2FullTbl[idx][4],
			 red_ctrl->u2FullTbl[idx][5], red_ctrl->u2FullTbl[idx][6], red_ctrl->u2FullTbl[idx][7]);
	}

	for (idx = 0 ; idx < HWRED_MAX_BN_NUM ; idx++) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			 "RED Drop Thld TBL%u: %u %u %u %u %u %u %u %u\n", idx, red_ctrl->u2DropTbl[idx][0], red_ctrl->u2DropTbl[idx][1],
			 red_ctrl->u2DropTbl[idx][2], red_ctrl->u2DropTbl[idx][3], red_ctrl->u2DropTbl[idx][4],
			 red_ctrl->u2DropTbl[idx][5], red_ctrl->u2DropTbl[idx][6], red_ctrl->u2DropTbl[idx][7]);
	}

	for (idx = 0 ; idx < CFG_WIFI_RAM_BAND_NUM; idx++) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			 "AC Tail Drop Min Thld band%u: %u %u %u %u\n", idx, red_ctrl->u2ACTailDropMin[idx][0],
			 red_ctrl->u2ACTailDropMin[idx][1], red_ctrl->u2ACTailDropMin[idx][2], red_ctrl->u2ACTailDropMin[idx][3]);
	}

	for (idx = 0 ; idx < CFG_WIFI_RAM_BAND_NUM; idx++) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
			 "AC Tail Drop Max Thld band%u: %u %u %u %u\n", idx, red_ctrl->u2ACTailDropMax[idx][0],
			 red_ctrl->u2ACTailDropMax[idx][1], red_ctrl->u2ACTailDropMax[idx][2], red_ctrl->u2ACTailDropMax[idx][3]);
	}

	return 0;
}

static int acm_dump_drop(struct _RTMP_ADAPTER *pAd, UINT_16 mld_id1, UINT_16 mld_idn, BOOLEAN fgReset)
{
	char buf[512] = {0};
	int pos = 0, left_buf_size, ret_val;
	int idx = 0, idx2 = 0;
	UCHAR band = 0;
	UINT_16 mld_s, mld_e;
	struct red_ctrl *red_ctrl = PD_GET_RED_CTRL_PTR(pAd->physical_dev);
	PMAC_TABLE_ENTRY pEntry = NULL;
	BOOLEAN fgAllDropDump = FALSE;

	for (idx = 0; idx < CFG_WIFI_RAM_BAND_NUM; idx++) {
		red_ctrl->drop_cnt_per_band[idx].QuadPart = 0;
		red_ctrl->tot_txcnt_per_band[idx].QuadPart = 0;
	}

	for (idx2 = 0; idx2 < AC_NUM; idx2++) {
		red_ctrl->drop_cnt_per_ac[idx2].QuadPart = 0;
		red_ctrl->tot_txcnt_per_ac[idx2].QuadPart = 0;
	}

	red_ctrl->drop_cnt.QuadPart = 0;
	red_ctrl->tot_txcnt.QuadPart = 0;


	mld_s = mld_id1;
	mld_e = mld_idn;

	if ((mld_id1 == 0) || (mld_id1 >= mld_idn)) {
		mld_s = 1;
		mld_e = HWRED_MAX_WTBL_NUM - 1;
		fgAllDropDump = TRUE;
	}

	for (idx = 0; idx < RED_STA_REC_NUM; idx++) {
		if (!VALID_UCAST_ENTRY_WCID(pAd, idx))
			continue;

		pEntry = entry_get(pAd, idx);

		if (IS_ENTRY_NONE(pEntry))
			continue;

		if ((fgAllDropDump) && (pEntry->msdu_tot_txcnt.QuadPart == 0))
			continue;

		if (!pEntry->wdev)
			continue;

		band = HcGetBandByWdev(pEntry->wdev);

		if ((idx >= mld_s) && (idx <= mld_e)) {
			left_buf_size = sizeof(buf) - pos;
			ret_val = snprintf(buf + pos, left_buf_size,
					"[STA%04u:band%u,%08llu/%08llu/%08llu,{%08llu,%08llu,%08llu,%08llu]",
					idx, band, pEntry->mpdu_mac_drop.QuadPart,
					pEntry->msdu_sdo_drop.QuadPart,
					pEntry->msdu_tot_txcnt.QuadPart,
					pEntry->msdu_tot_txcnt_per_ac[0].QuadPart,
					pEntry->msdu_tot_txcnt_per_ac[1].QuadPart,
					pEntry->msdu_tot_txcnt_per_ac[2].QuadPart,
					pEntry->msdu_tot_txcnt_per_ac[3].QuadPart);

			if (os_snprintf_error(left_buf_size, ret_val)) {
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR,
					"snprintf check error!\n");
				break;
			}

			pos += ret_val;

			if ((idx % 1) == 0) {
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR, "%s\n", buf);
				memset(buf, 0, 512);
				pos = 0;
			}
		}

		red_ctrl->drop_cnt.QuadPart += pEntry->msdu_sdo_drop.QuadPart;

		red_ctrl->tot_txcnt.QuadPart += pEntry->msdu_tot_txcnt.QuadPart;

		for (idx2 = 0; idx2 < AC_NUM; idx2++) {
			red_ctrl->drop_cnt_per_ac[idx2].QuadPart = red_ctrl->drop_cnt_per_ac[idx2].QuadPart +
				pEntry->msdu_sdo_drop_per_ac[idx2].QuadPart;

			red_ctrl->tot_txcnt_per_ac[idx2].QuadPart = red_ctrl->tot_txcnt_per_ac[idx2].QuadPart +
				pEntry->msdu_tot_txcnt_per_ac[idx2].QuadPart;
		}

		if (fgReset) {
			pEntry->mpdu_mac_drop.QuadPart = 0;
			pEntry->msdu_sdo_drop.QuadPart = 0;
			pEntry->msdu_tot_txcnt.QuadPart = 0;
			for (idx2 = 0; idx2 < AC_NUM; idx2++) {
				pEntry->msdu_sdo_drop_per_ac[idx2].QuadPart = 0;
				pEntry->msdu_tot_txcnt_per_ac[idx2].QuadPart = 0;
			}
		}

		if (band < CFG_WIFI_RAM_BAND_NUM) {
			red_ctrl->drop_cnt_per_band[band].QuadPart =
				red_ctrl->drop_cnt_per_band[band].QuadPart + pEntry->msdu_sdo_drop.QuadPart;
			red_ctrl->tot_txcnt_per_band[band].QuadPart =
				red_ctrl->tot_txcnt_per_band[band].QuadPart + pEntry->msdu_tot_txcnt.QuadPart;
		}
	}

	if (pos > 0)
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR, "%s\n", buf);

	for (idx = 0; idx < CFG_WIFI_RAM_BAND_NUM; idx++)
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR, "band%u:%llu/%llu\n", idx,
			red_ctrl->drop_cnt_per_band[idx].QuadPart,
			red_ctrl->tot_txcnt_per_band[idx].QuadPart);

	for (idx2 = 0; idx2 < AC_NUM; idx2++) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR, "ac%u:%llu/%llu\n", idx2,
			red_ctrl->drop_cnt_per_ac[idx2].QuadPart,
			red_ctrl->tot_txcnt_per_ac[idx2].QuadPart);
	}

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_ERROR, "total:%llu/%llu, WMM det:%u\n",
		red_ctrl->drop_cnt.QuadPart, red_ctrl->tot_txcnt.QuadPart, red_ctrl->fgWMMDetect);

	if (fgReset) {
		red_ctrl->drop_cnt.QuadPart = 0;
		red_ctrl->tot_txcnt.QuadPart = 0;

		for (idx = 0; idx < CFG_WIFI_RAM_BAND_NUM; idx++) {
			red_ctrl->drop_cnt_per_band[idx].QuadPart = 0;
			red_ctrl->tot_txcnt_per_band[idx].QuadPart = 0;
		}
	}

	return 0;
}

void acm_tx_cnt_update(
	struct _RTMP_ADAPTER *pAd,
	u8 qid,
	u16 wcid,
	u32 txcnt,
	u32 stat)
{
	PMAC_TABLE_ENTRY pEntry = NULL;

	pEntry = entry_get(pAd, wcid);

	if (!pEntry)
		return;

	if (stat == 2) {
		pEntry->msdu_sdo_drop.QuadPart += txcnt;
		pEntry->msdu_sdo_drop_per_ac[qid % AC_NUM].QuadPart += txcnt;
	}

	if (stat == 1) {//hw drop
		pEntry->mpdu_mac_drop.QuadPart++;
		pEntry->mpdu_mac_drop_per_ac[qid % AC_NUM].QuadPart += txcnt;
	}

	pEntry->msdu_tot_txcnt.QuadPart += txcnt;
	pEntry->msdu_tot_txcnt_per_ac[qid % AC_NUM].QuadPart += txcnt;
}

static int acm_config_sta_cmd(struct _RTMP_ADAPTER *pAd, struct red_cmd_admctrl_config *param_config)
{
	struct red_ctrl *red_ctrl = PD_GET_RED_CTRL_PTR(pAd->physical_dev);
	unsigned char idx = 0;

	for (idx = 0; idx < MAX_BITMAP_WORDLEN; idx++) {
		if (red_ctrl->sta_config_bitmap[idx]) {
			param_config->bitmap[idx] = red_ctrl->sta_config_bitmap[idx];
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_WARN,
				"bitmap[%u]=%08X\n", idx, param_config->bitmap[idx]);
		}
	}

	param_config->sta_config_cnt = red_ctrl->sta_config_cnt;

	for (idx = 0; idx < red_ctrl->sta_config_cnt; idx++) {
		param_config->sta_ctrl[idx].fgDisable = red_ctrl->sta_ctrl[idx].fgDisable;
		param_config->sta_ctrl[idx].ucRateShift = red_ctrl->sta_ctrl[idx].ucRateShift;
		param_config->sta_ctrl[idx].fgRateShiftDeRef = red_ctrl->sta_ctrl[idx].fgRateShiftDeRef;
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_WARN,
			"idx:%u,red_dis:%u,rate_shift=0x%x,rate_deref:%u,sta_config_cnt=%u\n",
			idx, param_config->sta_ctrl[idx].fgDisable,
			param_config->sta_ctrl[idx].ucRateShift,
			param_config->sta_ctrl[idx].fgRateShiftDeRef, param_config->sta_config_cnt);
	}

	acm_reset_sta_ctrl(pAd);

	return 0;
}

static int acm_reset_sta_ctrl(struct _RTMP_ADAPTER *pAd)
{
	struct red_ctrl *red_ctrl = PD_GET_RED_CTRL_PTR(pAd->physical_dev);
	unsigned char idx = 0;

	for (idx = 0; idx < MAX_BITMAP_WORDLEN; idx++)
		red_ctrl->sta_config_bitmap[idx] = 0;

	red_ctrl->sta_config_cnt = 0;

	memset(&red_ctrl->sta_ctrl[0], 0, sizeof(struct admctrl_sta) * HWRED_MAX_CONFIG_STA_NUM);

	return 0;
}

static int acm_config_sta(struct _RTMP_ADAPTER *pAd, char *arg_sta)
{
	unsigned int wcid = 0, red_dis, rate_shift, rate_shift_apply;
	struct red_ctrl *red_ctrl = PD_GET_RED_CTRL_PTR(pAd->physical_dev);
	unsigned char idx = 0;
	int rv;

	if (red_ctrl->sta_config_cnt >= HWRED_MAX_CONFIG_STA_NUM)
		goto error;

	if (red_ctrl->sta_config_cnt == 0) {
		for (idx = 0; idx < MAX_BITMAP_WORDLEN; idx++)
			red_ctrl->sta_config_bitmap[idx] = 0;
	}

	wcid = 0;
	red_dis = rate_shift = rate_shift_apply = 0;

	rv = sscanf(arg_sta, "%u-%u-%u-%u", &wcid, &red_dis, &rate_shift, &rate_shift_apply);
	if (rv < 0)
		goto error;

	idx = red_ctrl->sta_config_cnt;

	red_ctrl->sta_ctrl[idx].fgDisable = red_dis ? 1 : 0;
	red_ctrl->sta_ctrl[idx].ucRateShift = rate_shift & 0x0F;
	red_ctrl->sta_ctrl[idx].fgRateShiftDeRef = rate_shift_apply ? 0 : 1;

	if (!(red_ctrl->sta_config_bitmap[wcid >> 5] & BIT(wcid & 0x1F))) {
		red_ctrl->sta_config_cnt++;
		red_ctrl->sta_config_bitmap[wcid >> 5] |= BIT(wcid & 0x1F);
	}

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_WARN,
		"OK,sta%u,idx:%u,red_dis:%u,rate_shift=0x%x,rate_deref:%u,sta_config_cnt=%u\n",
		wcid, idx, red_ctrl->sta_ctrl[idx].fgDisable, red_ctrl->sta_ctrl[idx].ucRateShift,
		red_ctrl->sta_ctrl[idx].fgRateShiftDeRef, red_ctrl->sta_config_cnt);

	return 0;
error:
	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_WARN,
		"fail!,sta%u,sta_config_cnt=%u\n", wcid, red_ctrl->sta_config_cnt);

	return 1;
}

static int acm_config_init(struct _RTMP_ADAPTER *pAd, UINT_16 hwred_max_tkid_num[])
{
	int idx = 0, idx2 = 0;
	struct red_ctrl *red_ctrl = PD_GET_RED_CTRL_PTR(pAd->physical_dev);

	red_ctrl->u2PriorityOffset = 255;
	red_ctrl->u2TCPOffset = 200;

	for (idx = 0; idx < HWRED_MAX_SRC_NUM; idx++)
		red_ctrl->u2TokenPerSrc[idx] = (UINT_16)hwred_max_tkid_num[idx];

	for (idx = 0; idx < HWRED_MAX_SRC_NUM; idx++)
		red_ctrl->u2TokenThldPerSrc[idx] = (UINT_16)hwred_max_tkid_num[idx];


	for (idx = 0 ; idx < HWRED_MAX_BN_NUM; idx++) {
		for (idx2 = 0; idx2 < HWRED_MAX_THLD_NUM; idx2++)
			red_ctrl->u2FullTbl[idx][idx2] = (UINT_16)au4RedFullThdTBTC[idx2];
	}

	for (idx = 0 ; idx < HWRED_MAX_BN_NUM; idx++) {
		for (idx2 = 0; idx2 < HWRED_MAX_THLD_NUM; idx2++)
			red_ctrl->u2DropTbl[idx][idx2] = (UINT_16)au4RedDropThdTBTC[idx][idx2];
	}

	for (idx = 0 ; idx < CFG_WIFI_RAM_BAND_NUM; idx++) {
		for (idx2 = 0; idx2 < AC_NUM; idx2++)
			red_ctrl->u2ACTailDropMin[idx][idx2] = (UINT_16)auACTailDropMinThldSingleBand[idx][idx2];
	}

	for (idx = 0 ; idx < CFG_WIFI_RAM_BAND_NUM; idx++) {
		for (idx2 = 0; idx2 < AC_NUM; idx2++)
			red_ctrl->u2ACTailDropMax[idx][idx2] = (UINT_16)auACTailDropMaxThldSingleBand[idx][idx2];
	}

	return 0;
}

static int acm_init(struct _RTMP_ADAPTER *pAd)
{
	UINT_16 sw_tkn_cnt;
	UINT_8 idx;
	RTMP_CHIP_CAP *chip_cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	bool whnat_en = false;
	struct red_ctrl *red_ctrl = PD_GET_RED_CTRL_PTR(pAd->physical_dev);
	UINT_16 hwred_max_tkid_num[HWRED_MAX_SRC_NUM] = {'\0'};

	hwifi_get_tx_token_num(pAd, hwred_max_tkid_num, HWRED_MAX_SRC_NUM);
	for (idx = 0; idx < HWRED_MAX_SRC_NUM; idx++) {
		if (hwred_max_tkid_num[idx] == 0 || hwred_max_tkid_num[idx] > HWRED_MAX_TKID_NUM)
			hwred_max_tkid_num[idx] = HWRED_MAX_TKID_NUM;
	}

	os_zero_mem(red_ctrl, sizeof(struct red_ctrl));

	acm_config_init(pAd, hwred_max_tkid_num);

	sw_tkn_cnt = chip_cap->tkn_info.token_tx_cnt;

#ifdef WHNAT_SUPPORT
	if (PD_GET_WHNAT_ENABLE(pAd->physical_dev)) {
		sw_tkn_cnt = sw_tkn_cnt - chip_cap->tkn_info.hw_tx_token_cnt;
		whnat_en = true;
	}
#endif /* WHNAT_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_RED, DBG_LVL_WARN,
		"available sw token:%u, whnat_en:%u\n", sw_tkn_cnt, whnat_en);

	for (idx = 0; idx < HWRED_MAX_SRC_NUM; idx++)
		red_ctrl->u2TokenPerSrc[idx] = (UINT_16)hwred_max_tkid_num[idx];

	red_ctrl->u2TokenPerSrc[HWRED_MAX_SRC_NUM - 1] = sw_tkn_cnt;

	acm_send_cmd(pAd, ACM_TOKEN_CONFIG, (PUINT_8)red_ctrl, 0);

	return 0;
}

#endif /* CFG_RED_SUPPORT */
