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
	hw_ctrl.c
*/
#include "rt_config.h"
#include  "hw_ctrl.h"
#include "hw_ctrl_basic.h"
#include "hw_ctrl/cmm_chip.h"
#include "mcu/mt_cmd.h"
#ifdef LINUX
#include <linux/netdevice.h>
#endif
#ifdef ZERO_PKT_LOSS_SUPPORT
#include <linux/math64.h>
#endif

static NTSTATUS HwCtrlUpdateRtsThreshold(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct rts_thld *rts = (struct rts_thld *)CMDQelmt->buffer;

	AsicUpdateRtsThld(pAd, rts->wdev, rts->pkt_thld, rts->len_thld);
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS HwCtrlUpdateProtect(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct prot_info *prot = (struct prot_info *)CMDQelmt->buffer;
	AsicUpdateProtect(pAd, prot);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlDelAsicWcid(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	RT_SET_ASIC_WCID SetAsicWcid;

	SetAsicWcid = *((PRT_SET_ASIC_WCID)(CMDQelmt->buffer));

	if (!IS_WCID_VALID(pAd, SetAsicWcid.WCID) && (SetAsicWcid.WCID != WCID_ALL))
		return NDIS_STATUS_FAILURE;

	AsicDelWcidTab(pAd, SetAsicWcid.WCID);
	return NDIS_STATUS_SUCCESS;
}


#ifdef HTC_DECRYPT_IOT
static NTSTATUS HwCtrlSetAsicWcidAAD_OM(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	RT_SET_ASIC_AAD_OM SetAsicAAD_OM;

	SetAsicAAD_OM = *((PRT_SET_ASIC_AAD_OM)(CMDQelmt->buffer));
	AsicSetWcidAAD_OM(pAd, SetAsicAAD_OM.WCID, SetAsicAAD_OM.Value);
	return NDIS_STATUS_SUCCESS;
}
#endif /* HTC_DECRYPT_IOT */

#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
static NTSTATUS HwCtrlUpdate4Addr_HdrTrans(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	RT_ASIC_4ADDR_HDR_TRANS Update_4Addr_Hdr_Trans;
	Update_4Addr_Hdr_Trans = *((PRT_ASIC_4ADDR_HDR_TRANS)(CMDQelmt->buffer));

	AsicSetWcid4Addr_HdrTrans(pAd, Update_4Addr_Hdr_Trans.Wcid, Update_4Addr_Hdr_Trans.Enable);

	return NDIS_STATUS_SUCCESS;
}
#endif

static void update_txop_level(UINT16 *dst, UINT16 *src,
							  UINT32 bitmap, UINT32 len)
{
	UINT32 prio;

	for (prio = 0; prio < len; prio++) {
		if (bitmap & (1 << prio)) {
			if (*(dst + prio) < *(src + prio))
				*(dst + prio) = *(src + prio);
		}
	}
}

static void tx_burst_arbiter(struct _RTMP_ADAPTER *pAd,
							 struct wifi_dev *curr_wdev,
							 UCHAR bss_idx)
{
	struct wifi_dev **wdev = pAd->wdev_list;
	static UCHAR last_wmm_idx[CFG_WIFI_RAM_BAND_NUM];
	static UINT16 last_txop_level[CFG_WIFI_RAM_BAND_NUM];
	static struct wifi_dev *last_wdev[CFG_WIFI_RAM_BAND_NUM];
	UINT32 idx = 0;
	UINT32 _prio_bitmap = 0;
	UINT16 txop_level;
	UINT16 _txop_level[MAX_PRIO_NUM] = {0};
	UINT8 prio;
	UINT8 curr_prio = PRIO_DEFAULT;
	EDCA_PARM *edca_param = NULL;
	UCHAR wmm_idx = 0;
	UINT8 band_idx = 0;
#ifdef WIFI_UNIFIED_COMMAND
	UCHAR ac_queue[WMM_NUM_OF_AC] = {
		TxQ_IDX_AC1, /* ACI:0 AC_BE */
		TxQ_IDX_AC0, /* ACI:1 AC_BK */
		TxQ_IDX_AC2, /* ACI:2 AC_VI */
		TxQ_IDX_AC3, /* ACI:3 AC_VO */
	};
#endif

	edca_param = hwifi_get_edca(pAd, curr_wdev);

	if (edca_param == NULL)
		return;

	wmm_idx = HcGetWmmIdx(pAd, curr_wdev);

	band_idx = HcGetBandByWdev(curr_wdev);
	if (band_idx >= CFG_WIFI_RAM_BAND_NUM)
		return;

	/* judge the final prio bitmap for specific BSS */
	do {
		if (wdev[idx] == NULL)
			break;

		if (wdev[idx]->bss_info_argument.ucBssIndex == bss_idx) {
			_prio_bitmap |= wdev[idx]->prio_bitmap;
			update_txop_level(_txop_level, wdev[idx]->txop_level,
							  _prio_bitmap, MAX_PRIO_NUM);
		}

		idx++;
	} while (idx < WDEV_NUM_MAX);

	/* update specific BSS's prio bitmap & txop_level array */
	curr_wdev->bss_info_argument.prio_bitmap = _prio_bitmap;
	memcpy(curr_wdev->bss_info_argument.txop_level, _txop_level,
		   (sizeof(UINT16) * MAX_PRIO_NUM));

	/* find the highest prio module */
	for (prio = 0; prio < MAX_PRIO_NUM; prio++) {
		if (_prio_bitmap & (1 << prio))
			curr_prio = prio;
	}

	txop_level = curr_wdev->bss_info_argument.txop_level[curr_prio];

	if (pAd->CommonCfg.bEnableTxBurst) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_TXOP, DBG_LVL_INFO,
			 "band_idx=%d, wmm_idx=%d, last_wmm_idx=%d, txop_level=%x last_txop_level=%x\n",
			  band_idx, wmm_idx, last_wmm_idx[band_idx], txop_level, last_txop_level[band_idx]);
		if (wmm_idx == last_wmm_idx[band_idx] &&
				txop_level == last_txop_level[band_idx] &&
				curr_wdev == last_wdev[band_idx]) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_TXOP, DBG_LVL_INFO,
			 "wmm_idx & txop_level are the same! Don't apply to FW\n");
		} else {
#ifdef WIFI_UNIFIED_COMMAND
			AsicSetWmmParam(pAd, curr_wdev, wmm_idx, ac_queue[WMM_AC_BE], WMM_PARAM_TXOP, txop_level);
#else
			AsicSetWmmParam(pAd, wmm_idx, WMM_AC_BE, WMM_PARAM_TXOP, txop_level);
#endif /* WIFI_UNIFIED_COMMAND */
		}

		last_wmm_idx[band_idx] = wmm_idx;
		last_txop_level[band_idx] = txop_level;
		last_wdev[band_idx] = curr_wdev;
	}
}

static void set_tx_burst(struct _RTMP_ADAPTER *pAd, struct _tx_burst_cfg *txop_cfg)
{
	struct _BSS_INFO_ARGUMENT_T *bss_info = NULL;
	UCHAR bss_idx = 0;

#ifdef SW_CONNECT_SUPPORT
	if ((atomic_read(&pAd->dummy_obj.connect_cnt) > 0) &&
		(txop_cfg->prio == PRIO_MULTI_CLIENT))
		return;
#endif /* SW_CONNECT_SUPPORT */


	if (txop_cfg->enable) {
		txop_cfg->wdev->prio_bitmap |= (1 << txop_cfg->prio);
		txop_cfg->wdev->txop_level[txop_cfg->prio] = txop_cfg->txop_level;
	} else
		txop_cfg->wdev->prio_bitmap &= ~(1 << txop_cfg->prio);

	bss_info = &txop_cfg->wdev->bss_info_argument;
	bss_idx = bss_info->ucBssIndex;
	tx_burst_arbiter(pAd, txop_cfg->wdev, bss_idx);
}

void hw_set_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					 UINT8 ac_type, UINT8 prio, UINT16 level, UINT8 enable)
{
	struct _tx_burst_cfg txop_cfg;

	if (wdev == NULL)
		return;

	txop_cfg.wdev = wdev;
	txop_cfg.prio = prio;
	txop_cfg.ac_type = ac_type;
	txop_cfg.txop_level = level;
	txop_cfg.enable = enable;
	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_TXOP, DBG_LVL_INFO,
			 "<caller: %pS>\n -%s: prio=%x, level=%x, enable=%x\n",
			  __builtin_return_address(0), __func__,
			  prio, level, enable);
	set_tx_burst(pAd, &txop_cfg);
}


static NTSTATUS HwCtrlSetTxBurst(struct _RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _tx_burst_cfg *txop_cfg = (struct _tx_burst_cfg *)CMDQelmt->buffer;

	if (txop_cfg == NULL)
		return NDIS_STATUS_FAILURE;

	set_tx_burst(pAd, txop_cfg);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlSetPartWmmParam(struct _RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _part_wmm_cfg *part_wmm_cfg = (struct _part_wmm_cfg *)CMDQelmt->buffer;
#ifdef WIFI_UNIFIED_COMMAND
	UCHAR ac_queue[WMM_NUM_OF_AC] = {
		TxQ_IDX_AC1, /* ACI:0 AC_BE */
		TxQ_IDX_AC0, /* ACI:1 AC_BK */
		TxQ_IDX_AC2, /* ACI:2 AC_VI */
		TxQ_IDX_AC3, /* ACI:3 AC_VO */
	};
#endif

	if (part_wmm_cfg == NULL)
		return NDIS_STATUS_FAILURE;
#ifdef WIFI_UNIFIED_COMMAND
	AsicSetWmmParam(pAd, part_wmm_cfg->wdev, part_wmm_cfg->wmm_idx, ac_queue[part_wmm_cfg->ac_num], part_wmm_cfg->edca_type, part_wmm_cfg->edca_value);
#else
	AsicSetWmmParam(pAd, part_wmm_cfg->wmm_idx, part_wmm_cfg->ac_num, part_wmm_cfg->edca_type, part_wmm_cfg->edca_value);
#endif /* WIFI_UNIFIED_COMMAND */
	return NDIS_STATUS_SUCCESS;
}
#ifdef DABS_QOS
static NTSTATUS HwCtrlSetQosParam(struct _RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct qos_param_set_del *pqos_param_set_del = (struct qos_param_set_del *)CMDQelmt->buffer;
	UINT32 idx = pqos_param_set_del->idx;
	BOOLEAN sel_del = pqos_param_set_del->sel_del;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (idx >= MAX_QOS_PARAM_TBL) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
			 "idx[%d] >= MAX_QOS_PARAM_TBL!!! sel_del[%d]\n", idx, sel_del);
		return NDIS_STATUS_FAILURE;
	}

	if (sel_del == TRUE) {
		enable_qos_param_tbl_by_idx(idx);
		if (set_qos_param_to_fw(pAd, pqos_param_set_del, TRUE) == FALSE) {
			disable_qos_param_tbl_by_idx(idx);
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				 "Set fw QoS TBL fail!!! idx[%d], sel_del[%d]!!!\n", idx, sel_del);
			return NDIS_STATUS_FAILURE;
		}
	} else {
		if (set_qos_param_to_fw(pAd, pqos_param_set_del, FALSE) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				 "Del fw QoS TBL fail!!! idx[%d], sel_del[%d]!!!\n", idx, sel_del);
			return NDIS_STATUS_FAILURE;
		} else {
			OS_SPIN_LOCK_BH(&qos_param_table_lock);
			pEntry = qos_param_table[idx].pEntry;
			if (pEntry)
				memset(pEntry->qos_tbl_idx, 0, sizeof(unsigned short)*8);
			memset(&qos_param_table[idx], 0, sizeof(struct qos_param_rec));
			OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
		}
	}

	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef QOS_R3
static NTSTATUS HwCtrlSetQosCharacteristicsIE(struct _RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
#ifdef WIFI_UNIFIED_COMMAND
	INT32 ret = FALSE;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct UNI_CMD_MURU_QOS_CFG *pQosCfg = (struct UNI_CMD_MURU_QOS_CFG *)CMDQelmt->buffer;

	if (cap->uni_cmd_support)
		ret = UniCmdMuruParameterSet(pAd, (RTMP_STRING *)pQosCfg, UNI_CMD_MURU_SET_QOS_CFG);

	if (ret == TRUE)
		return NDIS_STATUS_SUCCESS;
	else
		return NDIS_STATUS_FAILURE;

#else
	return NDIS_STATUS_FAILURE;
#endif
}
#endif

#ifdef CONFIG_AP_SUPPORT
static NTSTATUS HwCtrlAPAdjustEXPAckTime(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"CmdThread::CMDTHREAD_AP_ADJUST_EXP_ACK_TIME\n");
		RTMP_IO_WRITE32(pAd->hdev_ctrl, EXP_ACK_TIME, 0x005400ca);
	}
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS HwCtrlAPRecoverEXPAckTime(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"CmdThread::CMDTHREAD_AP_RECOVER_EXP_ACK_TIME\n");
		RTMP_IO_WRITE32(pAd->hdev_ctrl, EXP_ACK_TIME, 0x002400ca);
	}
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT */


static NTSTATUS HwCtrlUpdateRawCounters(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_DEBUG, "--->\n");
	asic_update_raw_counters(pAd);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlUpdateMibCounters(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_DEBUG, "--->\n");
	asic_update_mib_bucket(pAd);
	return NDIS_STATUS_SUCCESS;
}

#ifdef ZERO_PKT_LOSS_SUPPORT
static void update_channel_stats(RTMP_ADAPTER *pAd, UINT8 BandIdx)
{
	UINT32	OBSSAirtime, MyTxAirtime, MyRxAirtime, PCCA_Time, EDCCA_Time, MyTx2Airtime, BACount;
	UINT64	BATime;
	//OBSS Air time
	OBSSAirtime = diffu32(pAd->ChannelStats.CurrentPeriodicChStat.ObssAirtimeAcc[BandIdx],
					pAd->ChannelStats.PrevPeriodicStatStore.ObssAirtimeAcc[BandIdx]);
	pAd->Ch_Stats.Obss_Time = OBSSAirtime;
	pAd->Ch_Stats.Obss_Time_5_sec = (pAd->Ch_Stats.Obss_Time_5sec_Avg != 0) ?
											(OBSSAirtime + 49 * pAd->Ch_Stats.Obss_Time_5sec_Avg)
											: OBSSAirtime * 50;
	pAd->Ch_Stats.Obss_Time_5sec_Avg = div_u64(pAd->Ch_Stats.Obss_Time_5_sec, 50);

	//My Tx Air time
	MyTxAirtime = diffu32(pAd->ChannelStats.CurrentPeriodicChStat.MyTxAirtimeAcc[BandIdx],
					pAd->ChannelStats.PrevPeriodicStatStore.MyTxAirtimeAcc[BandIdx]);
	pAd->Ch_Stats.Tx_Time = MyTxAirtime;

	//My Rx Air time
	MyRxAirtime = diffu32(pAd->ChannelStats.CurrentPeriodicChStat.MyRxAirtimeAcc[BandIdx],
					pAd->ChannelStats.PrevPeriodicStatStore.MyRxAirtimeAcc[BandIdx]);
	pAd->Ch_Stats.Rx_Time = MyRxAirtime;

	pAd->Ch_Stats.Rx_Time_5_sec = (pAd->Ch_Stats.Rx_Time_5sec_Avg != 0) ?
										(MyRxAirtime + 49 * pAd->Ch_Stats.Rx_Time_5sec_Avg)
										: MyRxAirtime * 50;
	pAd->Ch_Stats.Rx_Time_5sec_Avg = div_u64(pAd->Ch_Stats.Rx_Time_5_sec, 50);

	//PCCA time
	PCCA_Time = diffu32(pAd->ChannelStats.CurrentPeriodicChStat.PCcaTimeAcc[BandIdx],
					pAd->ChannelStats.PrevPeriodicStatStore.PCcaTimeAcc[BandIdx]);
	pAd->Ch_Stats.PCCA_Time = PCCA_Time;

	pAd->Ch_Stats.PCCA_Time_5_sec = (pAd->Ch_Stats.PCCA_Time_5sec_Avg != 0) ?
										(PCCA_Time + 49 * pAd->Ch_Stats.PCCA_Time_5sec_Avg)
										: PCCA_Time * 50;
	pAd->Ch_Stats.PCCA_Time_5sec_Avg  = div_u64(pAd->Ch_Stats.PCCA_Time_5_sec, 50);

	//EDCCA Time
	EDCCA_Time = diffu32(pAd->ChannelStats.CurrentPeriodicChStat.EdccaAirtimeAcc[BandIdx],
					pAd->ChannelStats.PrevPeriodicStatStore.EdccaAirtimeAcc[BandIdx]);
	pAd->Ch_Stats.SCCA_Time = EDCCA_Time;

	pAd->Ch_Stats.SCCA_Time_5_sec = (pAd->Ch_Stats.SCCA_Time_5sec_Avg != 0) ?
										(EDCCA_Time + 49 * pAd->Ch_Stats.SCCA_Time_5sec_Avg)
										: EDCCA_Time * 50;
	pAd->Ch_Stats.SCCA_Time_5sec_Avg  = div_u64(pAd->Ch_Stats.SCCA_Time_5_sec, 50);

	//Mac2PhyTxTime
	MyTx2Airtime = diffu32(pAd->ChannelStats.CurrentPeriodicChStat.MyMac2PhyTxTimeAcc[BandIdx],
					pAd->ChannelStats.PrevPeriodicStatStore.MyMac2PhyTxTimeAcc[BandIdx]);
	pAd->Ch_Stats.Tx2_Time = MyTx2Airtime;

	/*BA time = BA_Count * 32 (32 = Tx time of 1 BA @ 24Mbps)
	*  32 = TxtimeBA=Tpreamble + Tsig + (Tsym*Nsym) = 16 + 4 +(4*3)
	*/
	BACount = diffu32(pAd->ChannelStats.CurrentPeriodicChStat.BACountAcc[BandIdx],
					pAd->ChannelStats.PrevPeriodicStatStore.BACountAcc[BandIdx]);
	BATime = BACount * 32;
	pAd->Ch_Stats.BA_Time = BATime;
	pAd->Ch_Stats.BA_Time_5_sec =
		(pAd->Ch_Stats.BA_Time_5sec_Avg != 0) ?
		(BATime + 49 * pAd->Ch_Stats.BA_Time_5sec_Avg) :
		BATime * 50;
	pAd->Ch_Stats.BA_Time_5sec_Avg = div_u64(pAd->Ch_Stats.BA_Time_5_sec, 50);

	//Ch Busy time band 0, Min(MyTx,MyTx2)+BA+MyRx+OBSS+IPI
	if ((MyTxAirtime + MyRxAirtime + OBSSAirtime) <= 100000)
		pAd->Ch_BusyTime = MyTxAirtime + MyRxAirtime + OBSSAirtime;
	else
		pAd->Ch_BusyTime = 99999;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"Band[%d] Airtime ==> OBSS:%u, MyTx:%u, MyRx:%u, PCCA:%u, MyTx2:%u, BACount: %u\n",
		BandIdx, OBSSAirtime, MyTxAirtime, MyRxAirtime, PCCA_Time, MyTx2Airtime, BACount);
}

static void update_previous_periodic_stat_store(RTMP_ADAPTER *pAd, UINT8 BandIdx)
{
	/*update previous periodic onchannel stat */
	pAd->ChannelStats.PrevPeriodicStatStore.ObssAirtimeAcc[BandIdx] =
		pAd->ChannelStats.CurrentPeriodicChStat.ObssAirtimeAcc[BandIdx];

	pAd->ChannelStats.PrevPeriodicStatStore.MyTxAirtimeAcc[BandIdx] =
		pAd->ChannelStats.CurrentPeriodicChStat.MyTxAirtimeAcc[BandIdx];

	pAd->ChannelStats.PrevPeriodicStatStore.MyRxAirtimeAcc[BandIdx] =
		pAd->ChannelStats.CurrentPeriodicChStat.MyRxAirtimeAcc[BandIdx];

	pAd->ChannelStats.PrevPeriodicStatStore.EdccaAirtimeAcc[BandIdx] =
		pAd->ChannelStats.CurrentPeriodicChStat.EdccaAirtimeAcc[BandIdx];

	pAd->ChannelStats.PrevPeriodicStatStore.CcaNavTxTimeAcc[BandIdx] =
		pAd->ChannelStats.CurrentPeriodicChStat.CcaNavTxTimeAcc[BandIdx];

	pAd->ChannelStats.PrevPeriodicStatStore.PCcaTimeAcc[BandIdx] =
		pAd->ChannelStats.CurrentPeriodicChStat.PCcaTimeAcc[BandIdx];

	pAd->ChannelStats.PrevPeriodicStatStore.MyMac2PhyTxTimeAcc[BandIdx] =
		pAd->ChannelStats.CurrentPeriodicChStat.MyMac2PhyTxTimeAcc[BandIdx];

	pAd->ChannelStats.PrevPeriodicStatStore.BACountAcc[BandIdx] =
		pAd->ChannelStats.CurrentPeriodicChStat.BACountAcc[BandIdx];
}
static void UpdateChannelStatsPerBand(RTMP_ADAPTER *pAd, UINT8 BandIdx)
{
	/*update channel stats*/
	if (pAd->ScanCtrl.OffChScan != TRUE) {
		/*regular update on channel stats*/
		/*read channel stats*/
		asic_read_channel_stat_registers(pAd, BandIdx, (void *)&(pAd->ChannelStats.CurrentPeriodicChStat));
		/*update CH stats*/
		update_channel_stats(pAd, BandIdx);
		/*Update Previous register val*/
		update_previous_periodic_stat_store(pAd, BandIdx);

	} else {
		if (pAd->ScanCtrl.OffChScan_Ongoing != TRUE) {
			pAd->ScanCtrl.OffChScan = FALSE;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
					"%s: Scan state set to IDLE and Register reset for  Band\n", __func__);
			/*regular update on channel stats*/
			/*read channel stats*/
			asic_read_channel_stat_registers(pAd, BandIdx, (void *)&(pAd->ChannelStats.CurrentPeriodicChStat));
			/*update CH stats: skip as scan happened in last periodic*/

			/*Update Previous register val*/
			update_previous_periodic_stat_store(pAd, BandIdx);

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
					"%s: Register reset for  Band\n", __func__);

		}
	}
}

static NTSTATUS HwCtrlUpdateChannelStats(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UINT8 uBandIdx = hc_get_hw_band_idx(pAd);

	UpdateChannelStatsPerBand(pAd, uBandIdx);

	return NDIS_STATUS_SUCCESS;
}

static VOID StaNullAckTimeoutHandler(
		RTMP_ADAPTER *pAd)
{
	int i;

	for (i = 0; i < 3; i++) {
		if (pAd->ZeroLossSta[i].ChnlSwitchSkipTx && pAd->ZeroLossSta[i].wcid &&
			(pAd->Dot11_H.ChnlSwitchState >= ASIC_CHANNEL_SWITCH_COMMAND_ISSUED)) {
			pAd->ZeroLossSta[i].SendNullFrame = 0;
			pAd->ZeroLossSta[i].ChnlSwitchSkipTx = 0;
			AsicUpdateSkipTx(pAd, pAd->ZeroLossSta[i].wcid, 0); //reset skip tx
			pAd->ZeroLossSta[i].resume_time = jiffies_to_msecs(jiffies);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
						"Tx Enabled for wcid %d\n", pAd->ZeroLossSta[i].wcid);
		} else
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
						"Tx Enable skipped for wcid %d\n", pAd->ZeroLossSta[i].wcid);
	}

	pAd->Dot11_H.ChnlSwitchState = SET_CHANNEL_IDLE;
}

static VOID WcidNullAckEventHandler(
		RTMP_ADAPTER *pAd, UINT16 wcid)
{
	UINT16 i = 0, ZeroLossStaIndex = 0;
	INT8 index = -1;
	struct wifi_dev *wdev = wdev_search_by_wcid(pAd, wcid);
	struct DOT11_H *pDot11h = (struct DOT11_H *)wdev->pDot11_H;

	for (i = 0; i < 3; i++) {
		if (pAd->ZeroLossSta[i].wcid == wcid) {
			index = i;
			break;
		}
	}

	if (index < 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
						"Invalid wcid %d\n", wcid);
		return;
	}

	ZeroLossStaIndex = index;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
				"null ack event for wcid:%d  ChnlSwitchSkipTx:%d  ChnlSwitchState:%d\n",
				wcid, pAd->ZeroLossSta[ZeroLossStaIndex].ChnlSwitchSkipTx, pDot11h->ChnlSwitchState);

	if ((pAd->ZeroLossSta[ZeroLossStaIndex].ChnlSwitchSkipTx)
		&& (pDot11h->ChnlSwitchState >= ASIC_CHANNEL_SWITCH_COMMAND_ISSUED)) {

		pAd->ZeroLossSta[ZeroLossStaIndex].SendNullFrame = 0;
		pAd->ZeroLossSta[ZeroLossStaIndex].ChnlSwitchSkipTx = 0;
		pAd->chan_switch_time[15] = jiffies_to_msecs(jiffies);
		AsicUpdateSkipTx(pAd, wcid, 0);	//reset skip hw tx for wtbl entry
		pAd->ZeroLossSta[ZeroLossStaIndex].resume_time = jiffies_to_msecs(jiffies);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
						"Tx Enabled for wcid %d after %lu msec\n", wcid,
						(pAd->ZeroLossSta[ZeroLossStaIndex].resume_time
							- pAd->ZeroLossSta[ZeroLossStaIndex].suspend_time));
	}

	if (pDot11h->ChnlSwitchState >= ASIC_CHANNEL_SWITCH_COMMAND_ISSUED) {
		BOOLEAN Cancelled;

		for (i = 0; i < 3; i++) {
			if ((pAd->ZeroLossSta[i].wcid)
				&& (HcGetBandByWdev(wdev) == pAd->ZeroLossSta[i].band)) {
				if (pAd->ZeroLossSta[i].ChnlSwitchSkipTx == 1)
					return;		/*still pending null/ack for more stations*/
			}
		}

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
					"rcv null/ack for all sta\n");
		/*cancel and release last bcn timer*/
		RTMPCancelTimer(&pDot11h->ChnlSwitchStaNullAckWaitTimer, &Cancelled);
		pDot11h->ChnlSwitchState = SET_CHANNEL_IDLE;
	}
}

static NTSTATUS HwCtrlHandleNullAckEvent(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UINT16 *wcid = (UINT16 *)CMDQelmt->buffer;

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_NOTICE, "wcid:%d\n", *wcid);
	WcidNullAckEventHandler(pAd, *wcid);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlHandleStaNullAckTimeout(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_NOTICE, "NULL ACK Timeout\n");
	StaNullAckTimeoutHandler(pAd);
	return NDIS_STATUS_SUCCESS;
}
#endif /*ZERO_PKT_LOSS_SUPPORT*/

#ifdef FTM_SUPPORT
static NTSTATUS HwCtrlFTMSetChannel(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _ftm_set_channel_cfg *ftm_set_channel_cfg = (struct _ftm_set_channel_cfg *)CMDQelmt->buffer;

	if (ftm_set_channel_cfg == NULL)
		return NDIS_STATUS_FAILURE;

	ftm_switch_channel(ftm_set_channel_cfg->pAd,
						ftm_set_channel_cfg->wdev,
						ftm_set_channel_cfg->channel);

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlFTMSetMCBurst(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _ftm_set_mc_burst *p_ftm_set_mc_burst = (struct _ftm_set_mc_burst *)CMDQelmt->buffer;

	if (p_ftm_set_mc_burst == NULL)
		return NDIS_STATUS_FAILURE;

	ftm_mc_burst(p_ftm_set_mc_burst->pAd,
					p_ftm_set_mc_burst->wdev,
					p_ftm_set_mc_burst->band_idx);

	return NDIS_STATUS_SUCCESS;
}
#endif /* FTM_SUPPORT */

static NTSTATUS HwCtrlAddRemoveKeyTab(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	ASIC_SEC_INFO *pInfo;

	pInfo = (PASIC_SEC_INFO) CMDQelmt->buffer;
	AsicAddRemoveKeyTab(pAd, pInfo);
	return NDIS_STATUS_SUCCESS;
}

#ifdef MT_MAC
static NTSTATUS HwCtrlSetFrameOffload(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_SET_FRAME_OFFLOAD pSetFrameOffload = (PMT_SET_FRAME_OFFLOAD)CMDQelmt->buffer;
	P_CMD_FRAME_OFFLOAD_T pFrame_offload = NULL;
	struct wifi_dev *wdev = NULL;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */


	if (pSetFrameOffload->WdevIdx >= WDEV_NUM_MAX) {
		MTWF_PRINT("!!ERROR!!, WdevIdx =%u\n", pSetFrameOffload->WdevIdx);
		return NDIS_STATUS_FAILURE;
	}

	wdev = pAd->wdev_list[pSetFrameOffload->WdevIdx];

	if (wdev == NULL) {
		MTWF_PRINT("!!ERROR!!, wdev is NULL , WdevIdx=%u\n", pSetFrameOffload->WdevIdx);
		return NDIS_STATUS_FAILURE;
	}

	os_alloc_mem(NULL, (PUCHAR *)&pFrame_offload, sizeof(CMD_FRAME_OFFLOAD_T));

	if (!pFrame_offload) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATHW_CMD_CTRL, DBG_LVL_ERROR,
			 "can not allocate fd_frame_offload\n");
			return NDIS_STATUS_FAILURE;
	}

	os_zero_mem(pFrame_offload, sizeof(CMD_FRAME_OFFLOAD_T));

	pFrame_offload->ucEnable = pSetFrameOffload->ucEnable;
	pFrame_offload->ucWlanIdx = 0; /* only for non-uni cmd */
	pFrame_offload->ucOwnMacIdx = wdev->OmacIdx;
	pFrame_offload->ucBandIdx = HcGetBandByWdev(wdev);
	pFrame_offload->ucTxType = pSetFrameOffload->ucTxType;
	pFrame_offload->ucTxMode = pSetFrameOffload->ucTxMode;
	pFrame_offload->ucTxInterval = pSetFrameOffload->ucTxInterval;
#ifdef WIFI_UNIFIED_COMMAND
	pFrame_offload->u2Wcid = pSetFrameOffload->u2Wcid;
#endif

	pFrame_offload->u2PktLength = pSetFrameOffload->u2PktLength;
	if (pFrame_offload->ucEnable) {
		os_move_mem(pFrame_offload->acPktContent, pSetFrameOffload->acPktContent,
			pFrame_offload->u2PktLength);
	}

	hex_dump_with_lvl("HwCtrlSetFrameOffload", pFrame_offload->acPktContent,
		pFrame_offload->u2PktLength, DBG_LVL_INFO);

#ifdef WIFI_UNIFIED_COMMAND
	if (pChipCap->uni_cmd_support)
		MtUniCmdFrameOffloadSet(pAd, pFrame_offload);
	else
#endif /*WIFI_UNIFIED_COMMAND*/
		MtCmdFdFrameOffloadSet(pAd, pFrame_offload);

	if (pFrame_offload)
		os_free_mem(pFrame_offload);

	return NDIS_STATUS_SUCCESS;
}

#ifndef DOT11V_MBSSID_SUPPORT
static NTSTATUS HwCtrlSetBcnOffload(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_SET_BCN_OFFLOAD pSetBcnOffload = (PMT_SET_BCN_OFFLOAD)CMDQelmt->buffer;
	struct wifi_dev *wdev = pAd->wdev_list[pSetBcnOffload->WdevIdx];
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	CMD_BCN_OFFLOAD_T_V2 *bcn_offload_v2 = NULL;
#endif
	CMD_BCN_OFFLOAD_T bcn_offload;
	BCN_BUF_STRUCT *bcn_buf = NULL;
	UCHAR *buf;
	PNDIS_PACKET *pkt = NULL;

#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (pSetBcnOffload->OffloadPktType == PKT_V2_BCN) {
		os_alloc_mem(NULL, (PUCHAR *)&bcn_offload_v2, sizeof(*bcn_offload_v2));
		if (!bcn_offload_v2) {
			MTWF_DBG(pAd, DBG_CAT_FW, CATHW_CMD_CTRL, DBG_LVL_ERROR,
				 "can not allocate bcn_offload\n");
				return NDIS_STATUS_FAILURE;
		}
		os_zero_mem(bcn_offload_v2, sizeof(*bcn_offload_v2));
	} else
	NdisZeroMemory(&bcn_offload, sizeof(CMD_BCN_OFFLOAD_T));
#else
	NdisZeroMemory(&bcn_offload, sizeof(CMD_BCN_OFFLOAD_T));
#endif

	if ((pSetBcnOffload->OffloadPktType == PKT_V1_BCN)
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	|| (pSetBcnOffload->OffloadPktType == PKT_V2_BCN)
#endif
	) {
		bcn_buf = &wdev->bcn_buf;

		if (!bcn_buf) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_ERROR,
					 "bcn_buf is NULL!\n");
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (pSetBcnOffload->OffloadPktType == PKT_V2_BCN)
			os_free_mem(bcn_offload_v2);
#endif
			return NDIS_STATUS_FAILURE;
		}

		pkt = bcn_buf->BeaconPkt;
	}

#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (pSetBcnOffload->OffloadPktType == PKT_V2_BCN) {
			bcn_offload_v2->ucEnable = pSetBcnOffload->Enable;
			bcn_offload_v2->ucWlanIdx = 0;/* hardcode at present */
			bcn_offload_v2->ucOwnMacIdx = wdev->OmacIdx;
			bcn_offload_v2->ucBandIdx = HcGetBandByWdev(wdev);
			bcn_offload_v2->u2PktLength = pSetBcnOffload->WholeLength;
			bcn_offload_v2->ucPktType = pSetBcnOffload->OffloadPktType;
#ifdef CONFIG_AP_SUPPORT
			bcn_offload_v2->u2TimIePos = pSetBcnOffload->TimIePos;
			bcn_offload_v2->u2CsaIePos = pSetBcnOffload->CsaIePos;
			bcn_offload_v2->ucCsaCount = wdev->csa_count;
#endif
			buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
			NdisCopyMemory(bcn_offload_v2->acPktContent, buf, pSetBcnOffload->WholeLength);
			MtCmdBcnV2OffloadSet(pAd, bcn_offload_v2);
	} else {
	bcn_offload.ucEnable = pSetBcnOffload->Enable;
	bcn_offload.ucWlanIdx = 0;/* hardcode at present */
	bcn_offload.ucOwnMacIdx = wdev->OmacIdx;
	bcn_offload.ucBandIdx = HcGetBandByWdev(wdev);
	bcn_offload.u2PktLength = pSetBcnOffload->WholeLength;
	bcn_offload.ucPktType = pSetBcnOffload->OffloadPktType;
#ifdef CONFIG_AP_SUPPORT
	bcn_offload.u2TimIePos = pSetBcnOffload->TimIePos;
	bcn_offload.u2CsaIePos = pSetBcnOffload->CsaIePos;
	bcn_offload.ucCsaCount = wdev->csa_count;
#endif
	buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
	NdisCopyMemory(bcn_offload.acPktContent, buf, pSetBcnOffload->WholeLength);
	MtCmdBcnOffloadSet(pAd, &bcn_offload);
}
#else
{
	bcn_offload.ucEnable = pSetBcnOffload->Enable;
	bcn_offload.ucWlanIdx = 0;/* hardcode at present */
	bcn_offload.ucOwnMacIdx = wdev->OmacIdx;
	bcn_offload.ucBandIdx = HcGetBandByWdev(wdev);
	bcn_offload.u2PktLength = pSetBcnOffload->WholeLength;
	bcn_offload.ucPktType = pSetBcnOffload->OffloadPktType;
#ifdef CONFIG_AP_SUPPORT
	bcn_offload.u2TimIePos = pSetBcnOffload->TimIePos;
	bcn_offload.u2CsaIePos = pSetBcnOffload->CsaIePos;
	bcn_offload.ucCsaCount = wdev->csa_count;
#endif
	buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
	NdisCopyMemory(bcn_offload.acPktContent, buf, pSetBcnOffload->WholeLength);
	MtCmdBcnOffloadSet(pAd, &bcn_offload);
}
#endif
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (pSetBcnOffload->OffloadPktType == PKT_V2_BCN)
		os_free_mem(bcn_offload_v2);
#endif

	return NDIS_STATUS_SUCCESS;
}
#endif /* DOT11V_MBSSID_SUPPORT */

static NTSTATUS HwCtrlUpdateBssInfo(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	BSS_INFO_ARGUMENT_T *pBssInfoArgs = (BSS_INFO_ARGUMENT_T *)CMDQelmt->buffer;
	UINT32 ret;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_INFO, "CmdThread\n");
	ret = AsicBssInfoUpdate(pAd, pBssInfoArgs);
	return ret;
}

#ifdef DOT11_EHT_BE
static NTSTATUS HwCtrlUpdateDscbInfo(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct CMD_STATIC_PP_DSCB_T *dscb = (struct CMD_STATIC_PP_DSCB_T *)CMDQelmt->buffer;
	UINT32 ret;

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_INFO, "CmdThread\n");
	ret = AsicDscbInfoUpdate(pAd, dscb);
	return ret;
}

static NTSTATUS HwCtrlSetReconfigTmr(RTMP_ADAPTER *ad, HwCmdQElmt *CMDQelmt)
{
	INT32 ret;
	UINT8 link;
	struct reconfig_set_tmr_t *reconfig_info = (struct reconfig_set_tmr_t *)CMDQelmt->buffer;
	struct RECONFIG_SET_TMR_CTRL_T reconfig_ctrl;

	COPY_MAC_ADDR(reconfig_ctrl.aucMldAddr, reconfig_info->mld_addr);
	reconfig_ctrl.ucFwMldIdx = reconfig_info->fw_mld_idx;
	reconfig_ctrl.ucFlag = reconfig_info->mld_flag;
	reconfig_ctrl.u2VldLinkIdBmap = reconfig_info->tmr_link_id_bmap;
	reconfig_ctrl.u2NumSeconds = reconfig_info->num_seconds;
	for (link = 0; link < CFG_WIFI_RAM_BAND_NUM; link++) {
		if (reconfig_info->tmr_link_id_bmap & BIT(link))
			reconfig_ctrl.aucFwBssIdxLink[link] = reconfig_info->fw_bss_idx[link];
	}

	ret = AsicSetReconfigTmr(ad, &reconfig_ctrl);

	return ret;
}
#endif

static NTSTATUS HwCtrlShowBcnProc(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct CMD_BCN_PROC *pInfo = (struct CMD_BCN_PROC *)CMDQelmt->buffer;
	RTMP_STRING *BandIdx = pInfo->BandIdx;
	UINT32 ret;

	ret = ShowBcnProc(pAd, BandIdx);
	return ret;
}

static NTSTATUS HwCtrlFreeRroSetblProc(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
#ifdef WIFI_UNIFIED_COMMAND
	UINT16 *seid = (UINT16 *)CMDQelmt->buffer;

	uni_cmd_release_rro_setbl(pAd, *seid);
#endif
	return NDIS_STATUS_SUCCESS;
}

#ifdef PRE_CFG_SUPPORT
static NTSTATUS HwCtrlSetPrecfgCmdProc(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct PRECFG_TIMER_CMD_CTRL *pPreCfgTimerCmdCtrl = (struct PRECFG_TIMER_CMD_CTRL *)CMDQelmt->buffer;
	struct UNI_CMD_STAREC_INFO_T *pCmdStaRecUpdate = (struct UNI_CMD_STAREC_INFO_T *)(pPreCfgTimerCmdCtrl->pCmdBuf);
	struct _RTMP_ADAPTER *ad = pPreCfgTimerCmdCtrl->pAd;
	UINT32 CmdSize = pPreCfgTimerCmdCtrl->u4RealUsedBufSize;

	pCmdStaRecUpdate->u2TotalElementNum = cpu2le16(pPreCfgTimerCmdCtrl->u2TLVTotalNumber);
	pCmdStaRecUpdate->ucAppendCmdTLV = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	UniCmdPreCfgSetCmdByRaw(ad, HOST2CR4N9, UNI_CMD_ID_STAREC_INFO, (UCHAR *)pCmdStaRecUpdate, CmdSize);
#endif
	return NDIS_STATUS_SUCCESS;
}
#endif

static NTSTATUS HwCtrlAllStaStats(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
#ifdef WIFI_UNIFIED_COMMAND
	struct CMD_STAT_EVENT_TYPE *pInfo = (struct CMD_STAT_EVENT_TYPE *)CMDQelmt->buffer;
	RTMP_CHIP_CAP *pChipCap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	UINT8 type = 0;

	if (pChipCap->uni_cmd_support && pInfo != NULL) {
		for (type = 0; type < EVENT_PHY_MAX; type++) {
			if (pInfo->eventTypeBitmap & BIT(type))
				MtCmdGetAllStaStats(pAd, type);
		}
	}
#endif /* WIFI_UNIFIED_COMMAND */
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlSetBaRec(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MT_BA_CTRL_T *pSetBaRec = (MT_BA_CTRL_T *)CMDQelmt->buffer;
	struct _MAC_TABLE_ENTRY *entry = entry_get(pAd, pSetBaRec->Wcid);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_INFO, "CmdThread\n");
	AsicUpdateBASession(pAd, pSetBaRec->Wcid, pSetBaRec->Tid, pSetBaRec->Sn, pSetBaRec->BaWinSize, pSetBaRec->isAdd,
						pSetBaRec->BaSessionType, pSetBaRec->amsdu);

	if (pSetBaRec->BaSessionType == BA_SESSION_ORI)
		chip_do_extra_action(pAd, NULL, entry->Addr,
							 CHIP_EXTRA_ACTION_TX_ENABLE_AGG_CAP,
							 (UINT8 *)&pSetBaRec->BaWinSize, NULL);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlHandleUpdateBeacon(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MT_UPDATE_BEACON *prMtUpdateBeacon = (MT_UPDATE_BEACON *)CMDQelmt->buffer;
	struct wifi_dev *wdev = prMtUpdateBeacon->wdev;
	UCHAR UpdateReason = prMtUpdateBeacon->UpdateReason;
	UCHAR i;
	BOOLEAN UpdateAfterTim = FALSE;
	BCN_BUF_STRUCT *pbcn_buf = NULL;
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	UINT8 max_v2_bcn_num = cap->max_v2_bcn_num;
#endif

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
					 "Update reason: %d\n", UpdateReason);

	switch (BCN_REASON(UpdateReason)) {
	case BCN_UPDATE_INIT:
	case BCN_UPDATE_IF_STATE_CHG:
	case BCN_UPDATE_IE_CHG:
	case BCN_UPDATE_CSA:
	case BCN_UPDATE_BMGR:
		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			if (wdev != NULL) {
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
				if (wdev->func_idx < max_v2_bcn_num)
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_V2_BCN, TRUE, UpdateReason);
				else
#endif
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_V1_BCN, TRUE, UpdateReason);
			} else
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
						 "wdev = NULL, reason(%d)\n", UpdateReason);
		}
	break;

	case BCN_UPDATE_ALL_AP_RENEW: {
		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			/* Update/Renew all wdev */
			for (i = 0; i < WDEV_NUM_MAX; i++) {
				if (pAd->wdev_list[i] != NULL) {
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
					if (pAd->wdev_list[i]->func_idx < max_v2_bcn_num)
						UpdateBeaconProc(pAd, pAd->wdev_list[i], UpdateAfterTim, PKT_V2_BCN, TRUE, UpdateReason);
					else
#endif
						UpdateBeaconProc(pAd, pAd->wdev_list[i], UpdateAfterTim, PKT_V1_BCN, TRUE, UpdateReason);
				}
			}
		}
	}
	break;

	case BCN_UPDATE_ENABLE_TX: {
		if (wdev != NULL) {
			pbcn_buf = &wdev->bcn_buf;
			if (WDEV_WITH_BCN_ABILITY(wdev) && wdev->bAllowBeaconing) {
				wdev->bcn_buf.bBcnSntReq = TRUE;
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
				if (wdev->func_idx < max_v2_bcn_num)
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim,
						PKT_V2_BCN, TRUE, UpdateReason);
				else
#endif
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim,
						PKT_V1_BCN, TRUE, UpdateReason);
			}
		}
	}
	break;

	case BCN_UPDATE_DISABLE_TX: {
		if (wdev != NULL) {
			pbcn_buf = &wdev->bcn_buf;

			if (WDEV_WITH_BCN_ABILITY(wdev)) {
				wdev->bcn_buf.bBcnSntReq = FALSE;
				/* No need to make beacon */
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
				if (wdev->func_idx < max_v2_bcn_num)
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim,
						PKT_V2_BCN, FALSE, UpdateReason);
				else
#endif
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim,
						PKT_V1_BCN, FALSE, UpdateReason);
			}
		}
	}
	break;

	case BCN_UPDATE_PRETBTT: {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef AP_QLOAD_SUPPORT
			ULONG UpTime;
			/* update channel utilization */
			NdisGetSystemUpTime(&UpTime);
			QBSS_LoadUpdate(pAd, UpTime);
#endif /* AP_QLOAD_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
			RRM_QuietUpdata(pAd);
#endif /* DOT11K_RRM_SUPPORT */
			UpdateAfterTim = TRUE;
			updateBeaconRoutineCase(pAd, UpdateAfterTim);
		}

#endif /* CONFIG_AP_SUPPORT */
	}
	break;

	default:
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				 "Wrong Update reason: %d\n",
				  UpdateReason);
		break;
	}

	/* release bpcc lock after bcn update */
	if ((wdev != NULL) && (wdev->BcnBPCC_Init) &&
		((IS_BCN_CRIT_UPD(UpdateReason)) || (BCN_REASON_EQUAL(UpdateReason, BCN_UPDATE_IE_CHG))))
		bcn_bpcc_op_unlock_by_hwctrl(pAd, wdev, FALSE, UpdateReason);

	return NDIS_STATUS_SUCCESS;
}

#ifdef ERR_RECOVERY
static UINT32 ErrRecoveryTimeDiff(UINT32 time1, UINT32 time2)
{
	UINT32 timeDiff = 0;

	if (time1 > time2)
		timeDiff = (0xFFFFFFFF - time1 + 1) + time2;
	else
		timeDiff = time2 - time1;

	return timeDiff;
}

void SerTimeLogDump(RTMP_ADAPTER *pAd)
{
	UINT32 idx = 0;
	UINT32 *pSerTimes = NULL;

	if (pAd == NULL)
		return;

	pSerTimes = &pAd->HwCtrl.ser_times[0];

	for (idx = SER_TIME_ID_T0; idx < SER_TIME_ID_END; idx++) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_DEBUG,
				  "::E  R  , Time[%d](us)=%u\n", idx,
				  pSerTimes[idx]);
	}

	for (idx = SER_TIME_ID_T0; idx < (SER_TIME_ID_END - 1); idx++) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_DEBUG,
				  "::E  R  , T%d - T%d(us)=%u\n",
				  idx + 1, idx, ErrRecoveryTimeDiff(pSerTimes[idx],
						  pSerTimes[idx + 1]));
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
			  "::E  R  , Total Time(us)=%u\n",
			  ErrRecoveryTimeDiff(pSerTimes[SER_TIME_ID_T0],
						  pSerTimes[SER_TIME_ID_T7]));
}


VOID ser_sys_reset(RTMP_STRING *arg)
{
#ifdef SDK_TIMER_WDG
	/*kernel_restart(NULL);*/
	panic(arg); /* trigger SDK WATCHDOG TIMER */
#endif /* SDK_TIMER_WDG */
}
#endif /* ERR_RECOVERY */

#ifdef WF_RESET_SUPPORT
#ifdef CONFIG_CONNINFRA_SUPPORT
enum consys_drv_type {
	CONNDRV_TYPE_BT = 0,
	CONNDRV_TYPE_FM = 1,
	CONNDRV_TYPE_GPS = 2,
	CONNDRV_TYPE_WIFI = 3,
	CONNDRV_TYPE_CONNINFRA = 4,
	CONNDRV_TYPE_MAX
};
int conninfra_pwr_on(enum consys_drv_type drv_type);
int conninfra_pwr_off(enum consys_drv_type drv_type);

#endif /* CONFIG_CONNINFRA_SUPPORT */

NTSTATUS wf_reset_func(struct physical_device *device)
{
	int wdev_idx, band_idx;
	struct wifi_dev *wdev;
	PNET_DEV net_dev;
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(device);
	UCHAR band_num = PD_GET_BAND_NUM(device);
	int wdev_all = 0;

	PD_SET_WF_RESET_IN_PROGRESS(pAd->physical_dev, TRUE);
	for (band_idx = 0; band_idx < band_num; band_idx++) {
		pAd = physical_device_get_mac_adapter_by_band(device, band_idx);
		if (pAd == NULL)
			continue;

		for (wdev_all = 0; wdev_all < WDEV_NUM_MAX; wdev_all++) {

			if (RT_DEV_PRIV_FLAGS_GET(pAd->wdev_list[0]->if_dev) == INT_MAIN)
				wdev_idx = WDEV_NUM_MAX - wdev_all - 1;
			else
				wdev_idx = wdev_all;

			wdev = pAd->wdev_list[wdev_idx];

			if (wdev != NULL) {
				net_dev = wdev->if_dev;
				/* backup setting*/
				pAd->wdev_list_backup[wdev_idx].if_up_down_state = wdev->if_up_down_state;
				if (RTMP_OS_NETDEV_STATE_RUNNING(net_dev)) {
					switch (RT_DEV_PRIV_FLAGS_GET(net_dev)) {
					case INT_MAIN:
						main_virtual_if_close(net_dev);
						rtnl_lock();
						call_netdevice_notifiers(NETDEV_GOING_DOWN, net_dev);
						rtnl_unlock();
						rtnl_lock();
						dev_change_flags(net_dev, (net_dev->flags &= ~IFF_UP), NULL);
						rtnl_unlock();
						rtnl_lock();
						call_netdevice_notifiers(NETDEV_CHANGE, net_dev);
						rtnl_unlock();
						break;

					case INT_MBSSID:
						mbss_virtual_if_close(net_dev);
						rtnl_lock();
						call_netdevice_notifiers(NETDEV_GOING_DOWN, net_dev);
						rtnl_unlock();
						rtnl_lock();
						dev_change_flags(net_dev, (net_dev->flags &= ~IFF_UP), NULL);
						rtnl_unlock();
						rtnl_lock();
						call_netdevice_notifiers(NETDEV_CHANGE, net_dev);
						rtnl_unlock();
						break;

					case INT_WDS:
						wds_virtual_if_close(net_dev);
						rtnl_lock();
						call_netdevice_notifiers(NETDEV_GOING_DOWN, net_dev);
						rtnl_unlock();
						rtnl_lock();
						dev_change_flags(net_dev, (net_dev->flags &= ~IFF_UP), NULL);
						rtnl_unlock();
						rtnl_lock();
						call_netdevice_notifiers(NETDEV_CHANGE, net_dev);
						rtnl_unlock();
						break;
#ifdef CONFIG_STA_SUPPORT
					case INT_APCLI:
						msta_virtual_if_close(net_dev);
						rtnl_lock();
						call_netdevice_notifiers(NETDEV_GOING_DOWN, net_dev);
						rtnl_unlock();
						rtnl_lock();
						dev_change_flags(net_dev, (net_dev->flags &= ~IFF_UP), NULL);
						rtnl_unlock();
						rtnl_lock();
						call_netdevice_notifiers(NETDEV_CHANGE, net_dev);
						rtnl_unlock();
						break;
#endif /* CONFIG_STA_SUPPORT */
					default:
						break;
					}
				}

			}
		}
	}
#ifdef CONFIG_CONNINFRA_SUPPORT
		conninfra_pwr_off(CONNDRV_TYPE_WIFI);
		mdelay(15);
		conninfra_pwr_on(CONNDRV_TYPE_WIFI);
		mdelay(15);
#endif /* CONFIG_CONNINFRA_SUPPORT */
	/* DO l0.5 RESET for recovery wifi */
	for (band_idx = 0; band_idx < band_num; band_idx++) {
		pAd = physical_device_get_mac_adapter_by_band(device, band_idx);
		if (pAd != NULL)
			break;
	}

	asic_wf_subsys_reset(pAd);
	mdelay(5);
	PD_SET_WF_RESET_IN_PROGRESS(pAd->physical_dev, FALSE);

	for (band_idx = 0; band_idx < band_num; band_idx++) {
		pAd = physical_device_get_mac_adapter_by_band(device, band_idx);
		if (pAd == NULL)
			continue;

		for (wdev_all = 0; wdev_all < WDEV_NUM_MAX; wdev_all++) {

			if (RT_DEV_PRIV_FLAGS_GET(pAd->wdev_list[0]->if_dev) == INT_MAIN)
				wdev_idx = WDEV_NUM_MAX - wdev_all - 1;
			else
				wdev_idx = wdev_all;

			wdev = pAd->wdev_list[wdev_idx];
			if ((wdev != NULL) && (pAd->wdev_list_backup[wdev_idx].if_up_down_state == TRUE)) {
				net_dev = wdev->if_dev;

				switch (RT_DEV_PRIV_FLAGS_GET(net_dev)) {
				case INT_MAIN:
					main_virtual_if_open(net_dev);
					rtnl_lock();
					dev_change_flags(net_dev, (net_dev->flags |= IFF_UP), NULL);
					rtnl_unlock();
					rtnl_lock();
					call_netdevice_notifiers(NETDEV_CHANGE, net_dev);
					rtnl_unlock();
					break;

				case INT_MBSSID:
					mbss_virtual_if_open(net_dev);
					rtnl_lock();
					dev_change_flags(net_dev, (net_dev->flags |= IFF_UP), NULL);
					rtnl_unlock();
					rtnl_lock();
					call_netdevice_notifiers(NETDEV_CHANGE, net_dev);
					rtnl_unlock();
					break;

				case INT_WDS:
					wds_virtual_if_open(net_dev);
					rtnl_lock();
					dev_change_flags(net_dev, (net_dev->flags |= IFF_UP), NULL);
					rtnl_unlock();
					rtnl_lock();
					call_netdevice_notifiers(NETDEV_CHANGE, net_dev);
					rtnl_unlock();
					break;

				case INT_APCLI:
					msta_virtual_if_open(net_dev);
					rtnl_lock();
					dev_change_flags(net_dev, (net_dev->flags |= IFF_UP), NULL);
					rtnl_unlock();
					rtnl_lock();
					call_netdevice_notifiers(NETDEV_CHANGE, net_dev);
					rtnl_unlock();
					break;

				default:
					break;
				}
			}
		}
	}
	return NDIS_STATUS_SUCCESS;
}
#endif
#endif

/*STA part*/
#ifdef CONFIG_STA_SUPPORT

static NTSTATUS HwCtrlPwrMgtBitWifi(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_PWR_MGT_BIT_WIFI_T	rpMtPwrMgtBitWifi = (PMT_PWR_MGT_BIT_WIFI_T)(CMDQelmt->buffer);

	AsicExtPwrMgtBitWifi(pAd, rpMtPwrMgtBitWifi->u2WlanIdx, rpMtPwrMgtBitWifi->ucPwrMgtBit);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwEnterPsNull(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_STA_CFG_PTR_T	prStaCfgPtr = (PMT_STA_CFG_PTR_T)(CMDQelmt->buffer);
	PSTA_ADMIN_CONFIG pStaCfg = prStaCfgPtr->pStaCfg;
	PMAC_TABLE_ENTRY pMacEntry = GetAssociatedAPByWdev(pAd, &pStaCfg->wdev);

	RTMPSendNullFrame(pAd, pMacEntry, pAd->CommonCfg.TxRate, TRUE, PWR_SAVE);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlWakeUp(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_STA_CFG_PTR_T	prStaCfgPtr = (PMT_STA_CFG_PTR_T)(CMDQelmt->buffer);

	AsicWakeup(pAd, TRUE, prStaCfgPtr->pStaCfg);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlSleepAutoWakeup(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_STA_CFG_PTR_T	prStaCfgPtr = (PMT_STA_CFG_PTR_T)(CMDQelmt->buffer);

	AsicSleepAutoWakeup(pAd, prStaCfgPtr->pStaCfg);
	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef HOST_RESUME_DONE_ACK_SUPPORT
static NTSTATUS hw_ctrl_host_resume_done_ack(struct _RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->HostResumeDoneAck != NULL)
		ops->HostResumeDoneAck(pAd);

	return NDIS_STATUS_SUCCESS;
}
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
static NTSTATUS HwCtrlMlmeDynamicTxRateSwitching(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MlmeDynamicTxRateSwitching(pAd);
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_STA_SUPPORT */

static NTSTATUS HwCtrlNICUpdateRawCounters(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	asic_update_raw_counters(pAd);
	return NDIS_STATUS_SUCCESS;
}

/*Pheripheral Handler*/
static NTSTATUS HwCtrlCheckGPIO(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	return NDIS_STATUS_SUCCESS;
}

#ifdef LED_CONTROL_SUPPORT
static NTSTATUS HwCtrlSetLEDStatus(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_SET_LED_STS	prLedStatus = (PMT_SET_LED_STS)(CMDQelmt->buffer);

	RTMPSetLEDStatus(pAd, prLedStatus->Status, prLedStatus->BandIdx);
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_INFO,
		"CMDTHREAD_SET_LED_STATUS (LEDStatus = %d, BandIdx = %d)\n",
		prLedStatus->Status, prLedStatus->BandIdx);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlLedGpioMap(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_LED_GPIO_MAP prLedMapping = (PMT_LED_GPIO_MAP)(CMDQelmt->buffer);
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

#ifdef WIFI_UNIFIED_COMMAND
	if (pChipCap->uni_cmd_support)
		UniCmdLedGpio(pAd, prLedMapping->led_index, prLedMapping->map_index, prLedMapping->ctr_type);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		AndesLedGpioMap(pAd, prLedMapping->led_index, prLedMapping->map_index, prLedMapping->ctr_type);
	return NDIS_STATUS_SUCCESS;
}

#endif /* LED_CONTROL_SUPPORT */

#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
/*WPS LED MODE 10*/
static NTSTATUS HwCtrlLEDWPSMode10(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_SET_LED_STS	prLedStatus = (PMT_SET_LED_STS)(CMDQelmt->buffer);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_DEBUG,
		"WPS LED mode 10::ON or Flash or OFF : %x\n", prLedStatus->Status);

	switch (prLedStatus->Status) {
	case LINK_STATUS_WPS_MODE10_TURN_ON:
		RTMPSetLEDStatus(pAd, LED_WPS_MODE10_TURN_ON, prLedStatus->BandIdx);
		break;

	case LINK_STATUS_WPS_MODE10_FLASH:
		RTMPSetLEDStatus(pAd, LED_WPS_MODE10_FLASH, prLedStatus->BandIdx);
		break;

	case LINK_STATUS_WPS_MODE10_TURN_OFF:
		RTMPSetLEDStatus(pAd, LED_WPS_MODE10_TURN_OFF, prLedStatus->BandIdx);
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_DEBUG,
			"WPS LED mode 10:: No this status %d!!!\n", prLedStatus->Status);
		break;
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef CONFIG_AP_SUPPORT
static NTSTATUS HwCtrlSetStaDWRR(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MT_VOW_STA_GROUP *pVoW  =  (MT_VOW_STA_GROUP *)(CMDQelmt->buffer);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_DEBUG,
		"group %d, staid %d\n", pVoW->GroupIdx, pVoW->StaIdx);
	vow_set_client(pAd, pVoW->GroupIdx, pVoW->StaIdx, pVoW->WmmIdx);
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT */
#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
static NTSTATUS HwCtrlSetStaDWRRQuantum(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	INT32 ret;
	MT_VOW_STA_QUANTUM *pVoW  =  (MT_VOW_STA_QUANTUM *)(CMDQelmt->buffer);
#ifdef WIFI_UNIFIED_COMMAND
		RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_INFO,
		"\x1b[31mrestore %d, quantum %d\x1b[m\n", pVoW->restore, pVoW->quantum);

	if (pVoW->restore) {
		if (vow_watf_is_enabled(pAd)) {
			pAd->vow_cfg.vow_sta_dwrr_quantum[0] = pAd->vow_watf_q_lv0;
			pAd->vow_cfg.vow_sta_dwrr_quantum[1] = pAd->vow_watf_q_lv1;
			pAd->vow_cfg.vow_sta_dwrr_quantum[2] = pAd->vow_watf_q_lv2;
			pAd->vow_cfg.vow_sta_dwrr_quantum[3] = pAd->vow_watf_q_lv3;
		} else {
			pAd->vow_cfg.vow_sta_dwrr_quantum[0] = VOW_STA_DWRR_QUANTUM0;
			pAd->vow_cfg.vow_sta_dwrr_quantum[1] = VOW_STA_DWRR_QUANTUM1;
			pAd->vow_cfg.vow_sta_dwrr_quantum[2] = VOW_STA_DWRR_QUANTUM2;
			pAd->vow_cfg.vow_sta_dwrr_quantum[3] = VOW_STA_DWRR_QUANTUM3;
		}
	} else {
		UINT8 ac;
		/* 4 ac with the same quantum */
		for (ac = 0; ac < WMM_NUM_OF_AC; ac++)
			pAd->vow_cfg.vow_sta_dwrr_quantum[ac] = pVoW->quantum;
	}

#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support)
			ret = uni_cmd_vow_set_sta(pAd, 0, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_ALL);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			ret = vow_set_sta(pAd, 0, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_ALL);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_INFO,
		"\x1b[31mret %d\x1b[m\n", ret);

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlSetVOWScheduleCtrl(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _VOW_SCH_CFG_T *vow_sch_cfg = (struct _VOW_SCH_CFG_T *)CMDQelmt->buffer;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	pAd->vow_sch_cfg.apply_sch_ctrl = vow_sch_cfg->apply_sch_ctrl;
	pAd->vow_sch_cfg.sch_type = vow_sch_cfg->sch_type;
	pAd->vow_sch_cfg.sch_policy = vow_sch_cfg->sch_policy;

#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support)
			return uni_cmd_vow_set_feature_all(pAd);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			return vow_set_feature_all(pAd);
}

#endif /* CONFIG_AP_SUPPORT */
#endif /* VOW_SUPPORT */

static NTSTATUS HwCtrlThermalProtRadioOff(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_INFO, "\n");
	/* Set Radio off Process*/
	Set_RadioOn_Proc(pAd, "0");
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlUpdateRssi(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
	if (pAd->V10UpdateRssi == 1) {
		Vendor10RssiUpdate(pAd, NULL, FALSE, 0);
		pAd->V10UpdateRssi = 0;
	} else
#endif
		RssiUpdate(pAd);

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlUpdateTxPer(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UINT8 per = 0;
	UINT16 wlanid;
	PMAC_TABLE_ENTRY pEntry;

	wlanid = *(UINT16 *)CMDQelmt->buffer;
	pEntry = entry_get(pAd, wlanid);

	chip_get_sta_per(pAd, wlanid, &per);
	//pEntry->tx_per = per;

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_INFO,
		"STA(%d) update tx per(%d)!\n", pEntry->wcid, per);

	return NDIS_STATUS_SUCCESS;
}

#ifdef ETSI_RX_BLOCKER_SUPPORT
static NTSTATUS HwCtrlCheckRssi(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	/* Check RSSI on evey 100 ms */
	CheckRssi(pAd);
	return NDIS_STATUS_SUCCESS;
}
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */

static NTSTATUS HwCtrlGetTemperature(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UINT32 temperature = 0;
	UINT8 uBandIdx = *(UINT8*)CMDQelmt->buffer;

	/*ActionIdx 0 means get temperature*/
	MtCmdGetThermalSensorResult(pAd, 0, uBandIdx ,&temperature);
	os_move_mem(CMDQelmt->RspBuffer, &temperature, CMDQelmt->RspBufferLen);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlGetTsf(RTMP_ADAPTER *ad, HwCmdQElmt *CMDQelmt)
{
	INT32 ret = 0;
	EXTRA_ARG_TSF_T *tsf_arg = (EXTRA_ARG_TSF_T *)CMDQelmt->buffer;
	TSF_RESULT_T TsfResult = {0};

	ret = AsicGetTsfTime(ad,
		&TsfResult.u4TsfBit63_32,
		&TsfResult.u4TsfBit0_31,
		tsf_arg->ucHwBssidIndex);

	if (ret == TRUE)
		os_move_mem(CMDQelmt->RspBuffer, &TsfResult, CMDQelmt->RspBufferLen);

	return ret;
}

static NTSTATUS HwCtrlGetTsfDiff(RTMP_ADAPTER *ad, HwCmdQElmt *CMDQelmt)
{
	INT32 ret = 0;
	struct EXTRA_ARG_TSF_DIFF_T *TsfDiffArg = (struct EXTRA_ARG_TSF_DIFF_T *)CMDQelmt->buffer;
	struct TSF_DIFF_RESULT_T TsfDiffResult = {0};

	ret = AsicGetTsfDiffTime(ad,
		TsfDiffArg->ucBssIdx0,
		TsfDiffArg->ucBssIdx1,
		&TsfDiffResult.u4Tsf0Bit0_31,
		&TsfDiffResult.u4Tsf0Bit63_32,
		&TsfDiffResult.u4Tsf1Bit0_31,
		&TsfDiffResult.u4Tsf1Bit63_32);
	os_move_mem(CMDQelmt->RspBuffer, &TsfDiffResult, CMDQelmt->RspBufferLen);

	return ret;
}

static NTSTATUS HwCtrlGetAt2lmRes(RTMP_ADAPTER *ad, HwCmdQElmt *CMDQelmt)
{
	INT32 ret = 0;
	struct AT2LM_RES_REQ_CTRL_T *req = (struct AT2LM_RES_REQ_CTRL_T *)CMDQelmt->buffer;
	struct AT2LM_RES_RSP_CTRL_T rsp = {0};

	ret = AsicGetAt2lmRes(ad, req, &rsp);
	os_move_mem(CMDQelmt->RspBuffer, &rsp, CMDQelmt->RspBufferLen);

	return ret;
}

static NTSTATUS HwCtrlSetNt2lm(RTMP_ADAPTER *ad, HwCmdQElmt *CMDQelmt)
{
	INT32 ret = 0;
	struct NT2LM_REQ_CTRL_T *req = (struct NT2LM_REQ_CTRL_T *)CMDQelmt->buffer;

	ret = AsicSetNt2lm(ad, req);

	return ret;
}

static NTSTATUS HwCtrlReqReconfigRmLink(RTMP_ADAPTER *ad, HwCmdQElmt *CMDQelmt)
{
	INT32 ret = 0;
	struct RECONFIG_RM_LINK_REQ_CTRL_T *req = (struct RECONFIG_RM_LINK_REQ_CTRL_T *)CMDQelmt->buffer;

	ret = AsicReqReconfigRmLink(ad, req);

	return ret;
}
#ifdef VLAN_SUPPORT
static NTSTATUS HwCtrlsStaRecUpBmcVLANTag(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct VLAN_STA_REC *vstarec = (struct VLAN_STA_REC *)CMDQelmt->buffer;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	UINT16 WlanIdx = 0;
	UINT8 bssIdx = 0;
	STA_REC_CFG_T StaCfg;
	PMAC_TABLE_ENTRY pEntry = NULL;
	INT32 ret = 0;

	if (!vstarec)
		return -1;
	WlanIdx =  vstarec->u2WlanIdx;
	bssIdx = vstarec->ucBssIndex;
	if (arch_ops && arch_ops->archSetStaRec) {
		os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));

		/* Need to provide H/W BC/MC WLAN index to CR4 */
		if (!VALID_UCAST_ENTRY_WCID(pAd, WlanIdx))
			pEntry = NULL;
		else
			pEntry	= entry_get(pAd, WlanIdx);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_INFO,
			"Wcid(%d)\n", WlanIdx);
		if (pEntry && !IS_ENTRY_NONE(pEntry)) {
			if (!pEntry->wdev) {
				ASSERT(pEntry->wdev);
				return -1;
			}
			StaCfg.MuarIdx = pEntry->wdev->OmacIdx;
		} else
			StaCfg.MuarIdx = 0xe;/* TODO: Carter, check this on TX_HDR_TRANS */
		StaCfg.ConnectionState = STATE_PORT_SECURE;
		StaCfg.ConnectionType = CONNECTION_INFRA_BC;
		StaCfg.u8EnableFeature = STA_REC_TX_PROC_FEATURE;
		StaCfg.ucBssIndex = bssIdx;
		StaCfg.u2WlanIdx = WlanIdx;
		StaCfg.pEntry = pEntry;
		ret = arch_ops->archSetStaRec(pAd, &StaCfg);
		return ret;
	}
	AsicNotSupportFunc(pAd, __func__);
	return NDIS_STATUS_SUCCESS;
}
#endif
static NTSTATUS HwCtrlGetTxStatistic(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	TX_STAT_STRUC *pTxStat = (PTX_STAT_STRUC)CMDQelmt->buffer;
	struct _MAC_TABLE_ENTRY *pEntry;
	struct _STA_TR_ENTRY *tr_entry;
	struct wifi_dev *wdev;
	UCHAR dbdc_idx = 0;
	UINT32 len = CMDQelmt->bufferlength;
	UINT16 wcid = 0, i = 0;
	UCHAR num = len/sizeof(TX_STAT_STRUC);
	UCHAR valid_num = 0;
	TX_STAT_STRUC *p_temp = NULL, *pTxStat_cmd = NULL;

	os_alloc_mem_suspend(pAd, (UCHAR **)&pTxStat_cmd, num * sizeof(TX_STAT_STRUC));

	if (!pTxStat_cmd) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_ERROR,
			"alloc mem fail!!!\n");
		return NDIS_STATUS_FAILURE;
	}

	os_zero_mem(pTxStat_cmd, num * sizeof(TX_STAT_STRUC));

	/*error handling: check if the sta is connected*/
	for (i = 0; i < num; i++) {
		p_temp = pTxStat + i;

		wcid = p_temp->Wcid;

		if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
			continue;

		pEntry = entry_get(pAd, wcid);

		if (IS_ENTRY_NONE(pEntry))
			continue;

		wdev = pEntry->wdev;

		if (!wdev)
			continue;

		dbdc_idx = HcGetBandByWdev(wdev);

		tr_entry = tr_entry_get(pAd, pEntry->tr_tb_idx);

		if (tr_entry->StaRec.ConnectionState != STATE_PORT_SECURE)
			continue;

		os_move_mem(pTxStat_cmd + valid_num, p_temp, sizeof(TX_STAT_STRUC));
		valid_num++;

	}
	mt_cmd_get_sta_tx_statistic(pAd, pTxStat_cmd, valid_num);

	if (pTxStat_cmd)
		os_free_mem(pTxStat_cmd);
#endif

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlRadioOnOff(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PRADIO_ON_OFF_T pRadioOnOff = (PRADIO_ON_OFF_T)CMDQelmt->buffer;

	AsicRadioOnOffCtrl(pAd, pRadioOnOff->ucDbdcIdx, pRadioOnOff->ucRadio);
	return NDIS_STATUS_SUCCESS;
}

#ifdef LINK_TEST_SUPPORT
static NTSTATUS HwCtrlAutoLinkTest(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	/* Link Test Time Slot Handler */
	LinkTestTimeSlotLinkHandler(pAd);

	return NDIS_STATUS_SUCCESS;
}
#endif /* LINK_TEST_SUPPORT */

#ifdef GREENAP_SUPPORT
static NTSTATUS HwCtrlGreenAPOnOff(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PGREENAP_ON_OFF_T pGreenAP = (PGREENAP_ON_OFF_T)CMDQelmt->buffer;

	AsicGreenAPOnOffCtrl(pAd, pGreenAP->ucDbdcIdx, pGreenAP->ucGreenAPOn);
	return NDIS_STATUS_SUCCESS;
}
#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
static NTSTATUS hw_ctrl_pcie_aspm_dym_ctrl(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	P_PCIE_ASPM_DYM_CTRL_T ppcie_aspm_dym_ctrl = (P_PCIE_ASPM_DYM_CTRL_T)CMDQelmt->buffer;

	asic_pcie_aspm_dym_ctrl(
		pAd,
		ppcie_aspm_dym_ctrl->ucDbdcIdx,
		ppcie_aspm_dym_ctrl->fgL1Enable,
		ppcie_aspm_dym_ctrl->fgL0sEnable);

	return NDIS_STATUS_SUCCESS;
}
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
static NTSTATUS hw_ctrl_twt_agrt_update(struct _RTMP_ADAPTER *ad, HwCmdQElmt *CMDQelmt)
{
	struct TWT_AGRT_PARA_T *agrt_para = (struct TWT_AGRT_PARA_T *)CMDQelmt->buffer;

	return asic_twt_agrt_update(ad, agrt_para);
}

static NTSTATUS hw_ctrl_twt_agrt_mgmt(struct _RTMP_ADAPTER *ad, HwCmdQElmt *CMDQelmt)
{
	struct TWT_AGRT_PARA_T *agrt_para = (struct TWT_AGRT_PARA_T *)CMDQelmt->buffer;
	struct TWT_AGRT_MGMT_T *agrt_mgmt = (struct TWT_AGRT_MGMT_T *)CMDQelmt->RspBuffer;

	return asic_twt_agrt_mgmt(ad, agrt_para, agrt_mgmt);
}

static NTSTATUS hw_ctrl_mgmt_frame_offload(struct _RTMP_ADAPTER *ad, HwCmdQElmt *CMDQelmt)
{
	struct MGMT_FRAME_OFFLOAD_T *offload_para = (struct MGMT_FRAME_OFFLOAD_T *)CMDQelmt->buffer;

	return asic_twt_mgmt_frame_offload(ad, offload_para);
}

#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

static NTSTATUS HwCtrlSetSlotTime(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	SLOT_CFG *pSlotCfg = (SLOT_CFG *)CMDQelmt->buffer;

	AsicSetSlotTime(pAd, pSlotCfg->bUseShortSlotTime, pSlotCfg->Channel, pSlotCfg->wdev);
	return NDIS_STATUS_SUCCESS;
}

#ifdef PKT_BUDGET_CTRL_SUPPORT
/*
*
*/
static NTSTATUS HwCtrlSetPbc(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct pbc_ctrl *pbc = (struct pbc_ctrl *)CMDQelmt->buffer;
	INT32 ret = 0;
	UINT8 bssid = (pbc->wdev) ?  (pbc->wdev->bss_info_argument.ucBssIndex) : PBC_BSS_IDX_FOR_ALL;
	UINT16 wcid = (pbc->entry) ? (pbc->entry->wcid) : PBC_WLAN_IDX_FOR_ALL;

	ret = MtCmdPktBudgetCtrl(pAd, bssid, wcid, pbc->type);
	return ret;
}
#endif /*PKT_BUDGET_CTRL_SUPPORT*/

/*
*
*/
static NTSTATUS HwCtrlWifiSysOpen(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_open)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_open(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysClose(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_close)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_close(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysLinkUp(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_link_up)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_link_up(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysLinkDown(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_link_down)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_link_down(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysPeerLinkDown(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_disconnt_act)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_disconnt_act(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysPeerLinkUp(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_connt_act)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_connt_act(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysPeerUpdate(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_peer_update)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_peer_update(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}

#ifdef MBO_SUPPORT
static NTSTATUS HwCtrlBssTermination(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct wifi_dev *wdev = (struct wifi_dev *)CMDQelmt->buffer;
	UCHAR RfIC = 0;

	RfIC = wmode_2_rfic(wdev->PhyMode);

		if (!wdev->if_up_down_state) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MBO, DBG_LVL_INFO,
				"==>(%s) but IF is done, ignore!!! (wdev_idx %d)\n",
				"OFF", wdev->wdev_idx);
			return TRUE;
		}

		if (IsHcRadioCurStatOffByWdev(wdev)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MBO, DBG_LVL_INFO,
				"==>(%s) equal to current state, ignore!!! (wdev_idx %d)\n",
				"OFF", wdev->wdev_idx);
			return TRUE;
		}

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			MSTAStop(pAd, wdev);
		}
#endif
		MlmeRadioOff(pAd, wdev);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MBO, DBG_LVL_INFO,
			"==>(OFF)\n");

	return NDIS_STATUS_SUCCESS;
}
#endif

BOOLEAN hwctrl_cmd_q_empty(RTMP_ADAPTER *pAd)
{
	struct MCU_CTRL *ctl = PD_GET_MCU_CTRL_PTR(pAd->physical_dev);
	UINT16 time_out_cnt = 100;
	UINT16 current_cnt = 0;
	UINT32 hw_ctrl_q_len = 0;
	UINT32 txcmd_q_len = 0;

	while(current_cnt < time_out_cnt) {
		hw_ctrl_q_len = hwctrl_queue_len(pAd);
		txcmd_q_len = AndesQueueLen(ctl, &ctl->txq);
		if ((hw_ctrl_q_len != 0) || (txcmd_q_len != 0))
			msleep(100);
		else
			break;
		current_cnt++;
	}

	if (current_cnt == time_out_cnt)
		return FALSE;
	else
		return TRUE;
}

#ifdef NF_SUPPORT_V2
static NTSTATUS HwCtrlUpdateNF(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UCHAR en = *((UCHAR *)CMDQelmt->buffer);
#ifdef WIFI_UNIFIED_COMMAND
	UniCmdSetNoiseFloorControl(pAd, UNI_CMD_NF_INFO, en, 100, 5, 0);
#else
	EnableNF(pAd, en, 100, 5, 0); /*read CR per 100ms, 5 counts, about 500ms*/
#endif
	return NDIS_STATUS_SUCCESS;
}
#endif

/* For wifi and md coex in colgin project */
#ifdef WIFI_MD_COEX_SUPPORT
static NTSTATUS HwCtrlWifiCoexApccci2fw(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _MT_WIFI_COEX_APCCCI2FW *apccci2fw_msg =
			(struct _MT_WIFI_COEX_APCCCI2FW *)CMDQelmt->buffer;
	SendApccci2fwMsg(pAd, apccci2fw_msg);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlQueryLteSafeChannel(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_INFO, "\n");
	QueryLteSafeChannel(pAd);
	return NDIS_STATUS_SUCCESS;
}

#endif

#ifdef CFG_SUPPORT_CSI
static NTSTATUS HwCtrlGetCSIData(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct CMD_CSI_CONTROL_T *prCSICtrl =
			(struct CMD_CSI_CONTROL_T *)CMDQelmt->buffer;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support)
		return UniCmdCSICtrl(pAd, prCSICtrl);
#endif /* WIFI_UNIFIED_COMMAND */

	return AndesCSICtrl(pAd, prCSICtrl);
}
#endif

static NTSTATUS HwCtrlSmartCarrierSense(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_DEBUG, "--->\n");
	Smart_Carrier_Sense(pAd);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlFWLog2Host(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
#endif
	struct CMD_FwLog2Host *pInfo = NULL;

	pInfo = (struct CMD_FwLog2Host *)CMDQelmt->buffer;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_DEBUG, "--->\n");
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		MtUniCmdFwLog2Host(pAd, pInfo->McuDest, pInfo->FWLog2HostCtrl);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdFwLog2Host(pAd, pInfo->McuDest, pInfo->FWLog2HostCtrl);

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlSetRRORecover(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
#endif

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_DEBUG, "RRO recover--->\n");
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdSER(pAd, UNI_SER_ACTION_SET_TRIGGER, SER_SET_L4_RRO_LEAK_RECOVER, 0);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L4_RRO_LEAK_RECOVER, 0);

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlWAQuery(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct CMD_WAQuery *pQuery = NULL;

	pQuery = (struct CMD_WAQuery *)CMDQelmt->buffer;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_DEBUG, "--->\n");

	MtCmdCr4Query(pAd, pQuery->arg0, pQuery->arg1, pQuery->arg2);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlWAMultiQuery(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _CR4_QUERY_STRUC *cr4_query_list = NULL;

	cr4_query_list = (struct _CR4_QUERY_STRUC *)CMDQelmt->buffer;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_DEBUG, "--->\n");

	MtCmdCr4MultiQuery(pAd, cr4_query_list->arg0, cr4_query_list->arg1, cr4_query_list->arg2, cr4_query_list);
	return NDIS_STATUS_SUCCESS;
}

#ifdef IGMP_SNOOP_SUPPORT
static NTSTATUS HwCtrlWAMcastEntryAdd(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _CR4_MCAST_ENTRY *mcast_info;

	mcast_info = (struct _CR4_MCAST_ENTRY *)CMDQelmt->buffer;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_DEBUG, "--->\n");

	CmdMcastEntryInsert(pAd, &mcast_info->group_addr[0], mcast_info->bss_idx,
		mcast_info->filter_type, &mcast_info->member_addr[0],
		mcast_info->pdev, mcast_info->wcid);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlWAMcastEntryDel(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _CR4_MCAST_ENTRY *mcast_info;

	mcast_info = (struct _CR4_MCAST_ENTRY *)CMDQelmt->buffer;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_DEBUG, "--->\n");

	CmdMcastEntryDelete(pAd, &mcast_info->group_addr[0], mcast_info->bss_idx,
		&mcast_info->member_addr[0], mcast_info->pdev, mcast_info->wcid);
	return NDIS_STATUS_SUCCESS;
}

#ifdef IGMP_SNOOPING_DENY_LIST
static NTSTATUS HwCtrlWAMcastEntryDeny(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _CR4_MCAST_DENY_LIST *deny_list_info;

	deny_list_info = (struct _CR4_MCAST_DENY_LIST *)CMDQelmt->buffer;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_DEBUG, "--->\n");

	CmdMcastEntryDenyList(pAd, deny_list_info->bss_idx,
		deny_list_info->entry_cnt, deny_list_info->add_to_list, &deny_list_info->deny_list[0], &deny_list_info->Prefix_list[0]);
	return NDIS_STATUS_SUCCESS;
}
#endif

#endif
static NTSTATUS HwCtrlSetRxFilterCntl(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UINT8 *ucAction = (UINT8 *)CMDQelmt->buffer;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * cap = NULL;
#endif
	struct wifi_dev *wdev = NULL;

	wdev = &pAd->ApCfg.MBSSID[0].wdev;
#ifdef WIFI_UNIFIED_COMMAND
	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	if (cap->uni_cmd_support)
		UniCmdCfgInfoUpdate(pAd, wdev, CFGINFO_RX_FILTER_DROP_CTRL_FRAME_FEATURE, ucAction);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_RX_FILTER_DROP_CTRL_FRAME_FEATURE, ucAction);
	return NDIS_STATUS_SUCCESS;
}
static NTSTATUS HwCtrlGetStaSnr(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UINT8 uwcid = *((UINT8 *)CMDQelmt->buffer);
	UINT8 snr = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * cap = NULL;
#endif

#ifdef WIFI_UNIFIED_COMMAND
	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	if (cap->uni_cmd_support)
		UniCmdPerStaGetSNR(pAd, uwcid, &snr);
#endif /* WIFI_UNIFIED_COMMAND */
	os_move_mem(CMDQelmt->RspBuffer, &snr, CMDQelmt->RspBufferLen);
	return NDIS_STATUS_SUCCESS;
}
static NTSTATUS HwCtrlUpdateChannelInfo(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UCHAR ch_index =  *((UCHAR *)CMDQelmt->buffer);

	phy_update_channel_info(pAd, ch_index);
	return NDIS_STATUS_SUCCESS;
}
/*HWCMD_TYPE_RADIO*/
static HW_CMD_TABLE_T HwCmdRadioTable[] = {
	{HWCMD_ID_UPDATE_DAW_COUNTER, HwCtrlUpdateRawCounters, 0},
#ifdef MT_MAC
	{HWCMD_ID_SET_BA_REC, HwCtrlSetBaRec, 0},
	{HWCMD_ID_UPDATE_BSSINFO, HwCtrlUpdateBssInfo, 0},
	{HWCMD_ID_UPDATE_BEACON, HwCtrlHandleUpdateBeacon, 0},
	{HWCMD_ID_SET_TX_BURST, HwCtrlSetTxBurst, 0},
#endif /*MT_MAC*/
#ifdef CONFIG_AP_SUPPORT
	{HWCMD_ID_AP_ADJUST_EXP_ACK_TIME, HwCtrlAPAdjustEXPAckTime, 0},
	{HWCMD_ID_AP_RECOVER_EXP_ACK_TIME,	HwCtrlAPRecoverEXPAckTime, 0},
#endif
#ifdef CONFIG_AP_SUPPORT
	{HWCMD_ID_SET_STA_DWRR, HwCtrlSetStaDWRR, 0},
#ifdef VOW_SUPPORT
	{HWCMD_ID_SET_STA_DWRR_QUANTUM, HwCtrlSetStaDWRRQuantum, 0},
	{HWCMD_ID_SET_VOW_SCHEDULE_CTRL, HwCtrlSetVOWScheduleCtrl, 0},
#endif /* VOW_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	{HWCMD_ID_UPDATE_RSSI, HwCtrlUpdateRssi, 0},
	{HWCMD_ID_GET_TEMPERATURE, HwCtrlGetTemperature, 0},
	{HWCMD_ID_SET_SLOTTIME, HwCtrlSetSlotTime, 0},
#ifdef ETSI_RX_BLOCKER_SUPPORT
	{HWCMD_RX_CHECK_RSSI, HwCtrlCheckRssi, 0},
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */
#ifndef DOT11V_MBSSID_SUPPORT
	{HWCMD_ID_SET_BCN_OFFLOAD, HwCtrlSetBcnOffload, 0},
#endif /* DOT11V_MBSSID_SUPPORT */
	{HWCMD_ID_SET_FRAME_OFFLOAD, HwCtrlSetFrameOffload, 0},
	{HWCMD_ID_THERMAL_PROTECTION_RADIOOFF, HwCtrlThermalProtRadioOff, 0},
	{HWCMD_ID_RADIO_ON_OFF, HwCtrlRadioOnOff, 0},
#ifdef LINK_TEST_SUPPORT
	{HWCMD_ID_AUTO_LINK_TEST, HwCtrlAutoLinkTest, 0},
#endif /* LINK_TEST_SUPPORT */
#ifdef GREENAP_SUPPORT
	{HWCMD_ID_GREENAP_ON_OFF, HwCtrlGreenAPOnOff, 0},
#endif /* GREENAP_SUPPORT */
#ifdef MBO_SUPPORT
	{HWCMD_ID_BSS_TERMINATION, HwCtrlBssTermination, 0},
#endif /* MBO_SUPPORT */
#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
	{HWCMD_ID_UPDATE_4ADDR_HDR_TRANS, HwCtrlUpdate4Addr_HdrTrans, 0},
#endif
	{HWCMD_ID_UPDATE_MIB_COUNTER, HwCtrlUpdateMibCounters, 0},
#ifdef NF_SUPPORT_V2
	{HWCMD_ID_GET_NF_BY_FW, HwCtrlUpdateNF, 0},
#endif
#ifdef WIFI_MD_COEX_SUPPORT
	{HWCMD_ID_WIFI_COEX_APCCCI2FW, HwCtrlWifiCoexApccci2fw, 0},
	{HWCMD_ID_QUERY_LTE_SAFE_CHANNEL, HwCtrlQueryLteSafeChannel, 0},
#endif /* WIFI_MD_COEX_SUPPORT */
#ifdef CFG_SUPPORT_CSI
	{HWCMD_ID_GET_CSI_RAW_DATA, HwCtrlGetCSIData, 0},
#endif
	{HWCMD_ID_UPDATE_TX_PER, HwCtrlUpdateTxPer, 0},
	{HWCMD_ID_GET_TSF, HwCtrlGetTsf, 0},
	{HWCMD_ID_SMART_CARRIER_SENSE, HwCtrlSmartCarrierSense, 0},
	{HWCMD_ID_FWLog2Host, HwCtrlFWLog2Host, 0},
#ifdef DOT11_EHT_BE
	{HWCMD_ID_DSCB_UPDATE, HwCtrlUpdateDscbInfo, 0},
	{HWCMD_ID_RECONFIG_TMR, HwCtrlSetReconfigTmr, 0},
#endif
	{HWCMD_ID_BCN_PROC, HwCtrlShowBcnProc, 0},
	{HWCMD_ID_FREE_RRO_SETBL, HwCtrlFreeRroSetblProc, 0},
	{HWCMD_ID_ALL_STA_STATS, HwCtrlAllStaStats, 0},
#ifdef PRE_CFG_SUPPORT
	{HWCMD_ID_SET_PRECFG_CMD, HwCtrlSetPrecfgCmdProc, 0},
#endif
	{HWCMD_ID_GET_TSF_DIFF, HwCtrlGetTsfDiff, 0},
#ifdef DOT11_EHT_BE
	{HWCMD_ID_AT2LM_RES, HwCtrlGetAt2lmRes, 0},
	{HWCMD_ID_NT2LM, HwCtrlSetNt2lm, 0},
	{HWCMD_ID_RECONFIG_RM_LINK, HwCtrlReqReconfigRmLink, 0},
#endif
	{HWCMD_ID_SET_RX_FILTER_CNTL, HwCtrlSetRxFilterCntl, 0},
	{HWCMD_ID_GET_STA_SNR, HwCtrlGetStaSnr, 0},
#ifdef ZERO_PKT_LOSS_SUPPORT
	{HWCMD_ID_HANDLE_NULL_ACK_EVENT, HwCtrlHandleNullAckEvent, 0},
	{HWCMD_ID_HANDLE_STA_NULL_ACK_TIMEOUT, HwCtrlHandleStaNullAckTimeout, 0},
	{HWCMD_ID_UPDATE_CHANNEL_STATS, HwCtrlUpdateChannelStats, 0},
#endif /*ZERO_PKT_LOSS_SUPPORT*/
	{HWCMD_ID_SET_RRO_RECOVER, HwCtrlSetRRORecover, 0},
#ifdef FTM_SUPPORT
	{HWCMD_ID_FTM_SET_CHANNEL, HwCtrlFTMSetChannel, 0},
	{HWCMD_ID_FTM_SET_MC_BURST, HwCtrlFTMSetMCBurst, 0},
#endif /* FTM_SUPPORT */
	{HWCMD_ID_UPDATE_CHANNEL_INFO, HwCtrlUpdateChannelInfo, 0},
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_SECURITY*/
static HW_CMD_TABLE_T HwCmdSecurityTable[] = {
	{HWCMD_ID_ADDREMOVE_ASIC_KEY, HwCtrlAddRemoveKeyTab, 0},
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_PERIPHERAL*/
static HW_CMD_TABLE_T HwCmdPeripheralTable[] = {
	{HWCMD_ID_GPIO_CHECK, HwCtrlCheckGPIO, 0},
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
	{HWCMD_ID_LED_WPS_MODE10, HwCtrlLEDWPSMode10, 0},
#endif
#endif
#ifdef LED_CONTROL_SUPPORT
	{HWCMD_ID_SET_LED_STATUS, HwCtrlSetLEDStatus, 0},
	{HWCMD_ID_LED_GPIO_MAP, HwCtrlLedGpioMap, 0},
#endif
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_HT_CAP*/
static HW_CMD_TABLE_T HwCmdHtCapTable[] = {
	{HWCMD_ID_DEL_ASIC_WCID, HwCtrlDelAsicWcid, 0},
#ifdef HTC_DECRYPT_IOT
	{HWCMD_ID_SET_ASIC_AAD_OM, HwCtrlSetAsicWcidAAD_OM, 0},
#endif /* HTC_DECRYPT_IOT */

	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_PS*/
static HW_CMD_TABLE_T HwCmdPsTable[] = {
#ifdef MT_MAC
#endif
#ifdef CONFIG_STA_SUPPORT
	{HWCMD_ID_PWR_MGT_BIT_WIFI, HwCtrlPwrMgtBitWifi, 0},
	{HWCMD_ID_FORCE_WAKE_UP, HwCtrlWakeUp, 0},
	{HWCMD_ID_FORCE_SLEEP_AUTO_WAKEUP, HwCtrlSleepAutoWakeup, 0},
	{HWCMD_ID_ENTER_PS_NULL, HwEnterPsNull, 0},
#endif
#ifdef CONFIG_STA_SUPPORT
	{HWCMD_ID_PERODIC_CR_ACCESS_MLME_DYNAMIC_TX_RATE_SWITCHING, HwCtrlMlmeDynamicTxRateSwitching, 0},
#endif /* CONFIG_STA_SUPPORT */
	{HWCMD_ID_PERODIC_CR_ACCESS_NIC_UPDATE_RAW_COUNTERS, HwCtrlNICUpdateRawCounters, 0},
#ifdef HOST_RESUME_DONE_ACK_SUPPORT
	{HWCMD_ID_HOST_RESUME_DONE_ACK, hw_ctrl_host_resume_done_ack, 0},
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	{HWCMD_ID_PCIE_ASPM_DYM_CTRL, hw_ctrl_pcie_aspm_dym_ctrl, 0},
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	{HWCMD_ID_TWT_AGRT_UPDATE, hw_ctrl_twt_agrt_update, 0},
	{HWCMD_ID_TWT_AGRT_MGMT, hw_ctrl_twt_agrt_mgmt, 0},
	{HWCMD_ID_MGMT_FRAME_OFFLOD, hw_ctrl_mgmt_frame_offload, 0},
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
	{HWCMD_ID_END, NULL, 0}
};


/*HWCMD_TYPE_WIFISYS*/
static HW_CMD_TABLE_T HwCmdWifiSysTable[] = {
	{HWCMD_ID_WIFISYS_LINKDOWN, HwCtrlWifiSysLinkDown, 0},
	{HWCMD_ID_WIFISYS_LINKUP, HwCtrlWifiSysLinkUp, 0},
	{HWCMD_ID_WIFISYS_OPEN, HwCtrlWifiSysOpen, 0},
	{HWCMD_ID_WIFISYS_CLOSE, HwCtrlWifiSysClose, 0},
	{HWCMD_ID_WIFISYS_PEER_LINKDOWN, HwCtrlWifiSysPeerLinkDown, 0},
	{HWCMD_ID_WIFISYS_PEER_LINKUP, HwCtrlWifiSysPeerLinkUp, 0},
	{HWCMD_ID_WIFISYS_PEER_UPDATE, HwCtrlWifiSysPeerUpdate, 0},
	{HWCMD_ID_GET_TX_STATISTIC, HwCtrlGetTxStatistic, 0},
#ifdef VLAN_SUPPORT
	{HWCMD_ID_UPDATE_STARECVLAN, HwCtrlsStaRecUpBmcVLANTag},
#endif
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_WMM*/
static HW_CMD_TABLE_T HwCmdWmmTable[] = {
	{HWCMD_ID_PART_SET_WMM, HwCtrlSetPartWmmParam, 0},
#ifdef PKT_BUDGET_CTRL_SUPPORT
	{HWCMD_ID_PBC_CTRL, HwCtrlSetPbc, 0},
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
#ifdef DABS_QOS
	{HWCMD_ID_SET_DEL_QOS, HwCtrlSetQosParam, 0},
#endif
#ifdef QOS_R3
	{HWCMD_ID_SET_QOS_CHARACTERISTICS_IE, HwCtrlSetQosCharacteristicsIE, 0},
#endif
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_PROTECT*/
static HW_CMD_TABLE_T HwCmdProtectTable[] = {
	{HWCMD_ID_RTS_THLD, HwCtrlUpdateRtsThreshold, 0},
	{HWCMD_ID_HT_PROTECT, HwCtrlUpdateProtect, 0},
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_WA*/
static HW_CMD_TABLE_T HwCmdWAQueryTable[] = {
	{HWCMD_ID_WA_QUERY, HwCtrlWAQuery, 0},
	{HWCMD_ID_WA_MULTIQUERY, HwCtrlWAMultiQuery, 0},
	{HWCMD_ID_END, NULL, 0}
};

/* HWCMD_TYPE_WA_MCAST_ADD */
static HW_CMD_TABLE_T HwCmdWAMcastTable[] = {
#ifdef IGMP_SNOOP_SUPPORT
	{HWCMD_ID_WA_MCAST_ADD, HwCtrlWAMcastEntryAdd, 0},
	{HWCMD_ID_WA_MCAST_DEL, HwCtrlWAMcastEntryDel, 0},
#ifdef IGMP_SNOOPING_DENY_LIST
	{HWCMD_ID_WA_MCAST_DENY, HwCtrlWAMcastEntryDeny, 0},
#endif
#endif
	{HWCMD_ID_END, NULL, 0}
};

/*Order can't be changed, follow HW_CMD_TYPE order definition*/
HW_CMD_TABLE_T *HwCmdTable[] = {
	HwCmdRadioTable,
	HwCmdSecurityTable,
	HwCmdPeripheralTable,
	HwCmdHtCapTable,
	HwCmdPsTable,
	HwCmdWifiSysTable,
	HwCmdWmmTable,
	HwCmdProtectTable,
	HwCmdWAQueryTable,
	HwCmdWAMcastTable,
	NULL
};


