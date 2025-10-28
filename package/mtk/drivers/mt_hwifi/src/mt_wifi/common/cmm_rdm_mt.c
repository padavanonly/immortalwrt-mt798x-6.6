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
 ***************************************************************************

	Module Name:
	cmm_rdm_mt.c//Jelly20140123
*/

#ifdef MT_DFS_SUPPORT
/* Remember add RDM compiler flag - Shihwei20141104 */
#include "rt_config.h"
#include "hdev/hdev.h"
#include "config_internal.h"
#include "chlist.h"

/*******************************************************************************
*							   C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*							  D A T A	T Y P E S
********************************************************************************
*/

/*******************************************************************************
*							 P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*							P R I V A T E	D A T A
********************************************************************************
*/

EXT_EVENT_RDD_REPORT_T g_radar_info[HW_RDD_NUM];
#ifdef WIFI_UNIFIED_COMMAND
UNI_EVENT_RDD_SEND_PULSE_T uni_g_radar_info[HW_RDD_NUM];
#endif

/*******************************************************************************
*					F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

typedef int (*_k_ARC_ZeroWait_DFS_collision_report_callback_fun_type) (UCHAR SyncNum, UCHAR monitored_Ch, UCHAR Bw);
typedef int (*_k_ARC_ZeroWait_DFS_CAC_Time_Meet_report_callback_fun_type)(UCHAR SyncNum, UCHAR Bw, UCHAR monitored_Ch);
typedef int (*_k_ARC_ZeroWait_DFS_NOP_Timeout_report_callback_fun_type) (UCHAR Bw80ChNum, PDFS_REPORT_AVALABLE_CH_LIST pBw80AvailableChList, UCHAR Bw40ChNum, PDFS_REPORT_AVALABLE_CH_LIST pBw40AvailableChList, UCHAR Bw20ChNum, PDFS_REPORT_AVALABLE_CH_LIST pBw20AvailableChList);

_k_ARC_ZeroWait_DFS_collision_report_callback_fun_type radar_detected_callback_func;
_k_ARC_ZeroWait_DFS_CAC_Time_Meet_report_callback_fun_type DfsCacTimeOutCallBack;
_k_ARC_ZeroWait_DFS_NOP_Timeout_report_callback_fun_type DfsNopTimeOutCallBack;

void k_ZeroWait_DFS_Collision_Report_Callback_Function_Registeration(_k_ARC_ZeroWait_DFS_collision_report_callback_fun_type callback_detect_collision_func)
{
	radar_detected_callback_func = callback_detect_collision_func;
}

void k_ZeroWait_DFS_CAC_Time_Meet_Report_Callback_Function_Registeration(_k_ARC_ZeroWait_DFS_CAC_Time_Meet_report_callback_fun_type callback_CAC_time_meet_func)
{
	DfsCacTimeOutCallBack = callback_CAC_time_meet_func;
}

void k_ZeroWait_DFS_NOP_Timeout_Report_Callback_Function_Registeration(_k_ARC_ZeroWait_DFS_NOP_Timeout_report_callback_fun_type callback_NOP_Timeout_func)
{
	DfsNopTimeOutCallBack = callback_NOP_Timeout_func;
}

EXPORT_SYMBOL(k_ZeroWait_DFS_Collision_Report_Callback_Function_Registeration);
EXPORT_SYMBOL(k_ZeroWait_DFS_CAC_Time_Meet_Report_Callback_Function_Registeration);
EXPORT_SYMBOL(k_ZeroWait_DFS_NOP_Timeout_Report_Callback_Function_Registeration);

INLINE void do_cac_op(
	struct _CHANNEL_TX_POWER *ch_info,
	enum _CAC_OP op,
	UCHAR *status)
{
	*status = TRUE;

	if (op == CAC_DONE_UPDATE) {
		ch_info->Flags |= CHANNEL_CAC_DONE;
		NdisGetSystemUpTime(&ch_info->cac_done_timestamp);
	} else if (op == CAC_DONE_CHECK) {
		if (ch_info->DfsReq && !(ch_info->Flags & CHANNEL_CAC_DONE))
			*status = FALSE;
	}
}

BOOLEAN dfs_cac_op(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	enum _CAC_OP op,
	UCHAR CacChannel)
{
	BOOLEAN status = TRUE;
	UCHAR band_idx, ch_idx = 0;
	UCHAR bw_cap, bw = 0, vht_bw, ext_ch;
	struct _DFS_PARAM *pDfsParam = &pAd->CommonCfg.DfsParameter;
	CHANNEL_CTRL *pChCtrl = NULL;
	struct DOT11_H *pDot11h = NULL;

	if (!pDfsParam) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "Null pDfsParam!!!\n");
		return FALSE;
	}
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "Null wdev!!!\n");
		return FALSE;
	}

	pDot11h = wdev->pDot11_H;
	band_idx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	if (!pDot11h) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "Null pDot11h!!!\n");
		return FALSE;
	}
	if (!pChCtrl) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "Null pChCtrl!!!\n");
		return FALSE;
	}

	if (op == CAC_DONE_UPDATE) {
		if (pDot11h && (pDot11h->ChannelMode == CHAN_SWITCHING_MODE)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				"RDD channel switch going on, don't update CAC done\n");
			return FALSE;
		} else if ((CacChannel != pDfsParam->cac_channel) &&
		(pDfsParam->bDedicatedZeroWaitDefault == FALSE)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				"CAC channel[%d] different from current channel[%d]\n",
				pDfsParam->cac_channel, CacChannel);
			pDfsParam->cac_channel = 0;
			return FALSE;
		}
	}

	/* calculate bw */
	if (pDfsParam->bDedicatedZeroWaitDefault) {
		if (op == CAC_DONE_UPDATE)
			bw = pDfsParam->OutBandBw;
		else
			dfs_get_outband_bw(pAd, wdev, &bw);
	} else {
		bw = wlan_config_get_ht_bw(wdev); /* config ht bw */
		/* update bw based on channel op */
		ht_ext_cha_adjust(pAd, CacChannel, &bw, &ext_ch, wdev);
		if (bw > BW_20) {
			vht_bw = wlan_config_get_vht_bw(wdev);
			if (vht_bw == VHT_BW_80)
				bw = BW_80;
			else if (vht_bw == VHT_BW_160)
				bw = BW_160;
			else if (vht_bw == VHT_BW_8080)
				bw = BW_8080;
			else
				bw = BW_40;
			bw_cap = get_channel_bw_cap(wdev, CacChannel);
			if (bw > bw_cap)
				bw = bw_cap;
		}
	}

	switch (bw) {
	case BW_20:
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (CacChannel == pChCtrl->ChList[ch_idx].Channel) {
				do_cac_op(&pChCtrl->ChList[ch_idx], op, &status);
				break;
			}
		}
		break;

	case BW_40:
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if ((CacChannel == pChCtrl->ChList[ch_idx].Channel)
				|| ((CacChannel >> 2 & 1) && (pChCtrl->ChList[ch_idx].Channel - CacChannel == 4))
				|| (!(CacChannel >> 2 & 1) && (CacChannel - pChCtrl->ChList[ch_idx].Channel == 4))) {
				do_cac_op(&pChCtrl->ChList[ch_idx], op, &status);
				if (!status)
					break;
			}
		}
		break;

	case BW_80:
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (vht_cent_ch_freq(pChCtrl->ChList[ch_idx].Channel, VHT_BW_80, CMD_CH_BAND_5G) ==
				vht_cent_ch_freq(CacChannel, VHT_BW_80, CMD_CH_BAND_5G)) {
				do_cac_op(&pChCtrl->ChList[ch_idx], op, &status);
				if (!status)
					break;
			}
		}
		break;

	case BW_160:
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (pChCtrl->ChList[ch_idx].DfsReq &&
				(vht_cent_ch_freq(pChCtrl->ChList[ch_idx].Channel, VHT_BW_160, CMD_CH_BAND_5G) ==
				vht_cent_ch_freq(CacChannel, VHT_BW_160, CMD_CH_BAND_5G))) {
				do_cac_op(&pChCtrl->ChList[ch_idx], op, &status);
				if (!status)
					break;
			}
		}
		break;

	case BW_8080:
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (pChCtrl->ChList[ch_idx].DfsReq &&
			   ((vht_cent_ch_freq(pChCtrl->ChList[ch_idx].Channel, VHT_BW_8080, CMD_CH_BAND_5G) ==
				 vht_cent_ch_freq(pDfsParam->band_ch, VHT_BW_8080, CMD_CH_BAND_5G)) ||
				(vht_cent_ch_freq(pChCtrl->ChList[ch_idx].Channel, VHT_BW_8080, CMD_CH_BAND_5G) ==
				 vht_cent_ch_freq(pDfsParam->band_ch, VHT_BW_8080, CMD_CH_BAND_5G)))) {
				do_cac_op(&pChCtrl->ChList[ch_idx], op, &status);
				if (!status)
					break;
			}
		}
		break;

	default:
		break;
	}

	pDfsParam->cac_channel = 0;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
		"(caller:%pS) CAC op:%d ch=%d bw=%d status:%d\n", OS_TRACE, op, CacChannel, bw, status);

	return status;

}

#ifdef DFS_SLAVE_SUPPORT
inline void ch_rdd_flag_op(
	PCHANNEL_TX_POWER ch_info,
	enum ch_flag_op op,
	BOOLEAN *status)
{
	if (op == flag_reset)
		ch_info->Flags &= ~CHANNEL_RDD_HIT;
	else if (op == flag_set)
		ch_info->Flags |= CHANNEL_RDD_HIT;
	else if ((op == flag_check) && (ch_info->Flags & CHANNEL_RDD_HIT))
		*status = TRUE;
}

/* @func : set, reset and check radar flag for channel */
BOOLEAN slave_rdd_op(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	enum ch_flag_op op)
{
	BOOLEAN status = FALSE;
	UCHAR ch_idx = 0;
	UCHAR bw_cap, bw, vht_bw, ext_ch, channel = 0;
	CHANNEL_CTRL *pChCtrl = NULL;
	struct _DFS_PARAM *pDfsParam = &pAd->CommonCfg.DfsParameter;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "Null wdev!!!\n");
		return FALSE;
	}
	if (!pDfsParam) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "Null pDfsParam!!!\n");
		return FALSE;
	}

	channel = wdev->channel;
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	/* calculate bw */
	if (pDfsParam->bDedicatedZeroWaitDefault) {
		//precacTODO:check op
		if (op == flag_check)
			bw = pDfsParam->OutBandBw;
		else
			dfs_get_outband_bw(pAd, wdev, &bw);
	} else {
		bw = wlan_config_get_ht_bw(wdev); /* config ht bw */
		/* update bw based on channel op */
		ht_ext_cha_adjust(pAd, channel, &bw, &ext_ch, wdev);
		if (bw > BW_20) {
			vht_bw = wlan_config_get_vht_bw(wdev);
			if (vht_bw == VHT_BW_80)
				bw = BW_80;
			else if (vht_bw == VHT_BW_160)
				bw = BW_160;
			else
				bw = BW_40;
			bw_cap = get_channel_bw_cap(wdev, channel);
			if (bw > bw_cap)
				bw = bw_cap;
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		"(caller:%pS)[%s] ch:%d bw:%d op:%d\n",
		OS_TRACE, __func__, channel, bw, op);

	switch (bw) {
	case BW_20:
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (channel == pChCtrl->ChList[ch_idx].Channel)
				ch_rdd_flag_op(&pChCtrl->ChList[ch_idx], op, &status);
		}
		break;

	case BW_40:
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if ((channel == pChCtrl->ChList[ch_idx].Channel)
				|| ((channel >> 2 & 1) && (pChCtrl->ChList[ch_idx].Channel - channel == 4))
				|| (!(channel >> 2 & 1) && (channel - pChCtrl->ChList[ch_idx].Channel == 4)))
				ch_rdd_flag_op(&pChCtrl->ChList[ch_idx], op, &status);
		}
		break;

	case BW_80:
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (vht_cent_ch_freq(pChCtrl->ChList[ch_idx].Channel, VHT_BW_80, CMD_CH_BAND_5G) ==
				vht_cent_ch_freq(channel, VHT_BW_80, CMD_CH_BAND_5G))
				ch_rdd_flag_op(&pChCtrl->ChList[ch_idx], op, &status);
		}
		break;

	case BW_160:
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (pChCtrl->ChList[ch_idx].DfsReq &&
				(vht_cent_ch_freq(pChCtrl->ChList[ch_idx].Channel, VHT_BW_160, CMD_CH_BAND_5G) ==
				vht_cent_ch_freq(channel, VHT_BW_160, CMD_CH_BAND_5G)))
				ch_rdd_flag_op(&pChCtrl->ChList[ch_idx], op, &status);
		}
		break;

	default:
		break;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		"[%s] status:%d\n", __func__, status);

	return status;
}
#endif /* DFS_SLAVE_SUPPORT */

BOOLEAN IsChABand(USHORT PhyMode, UCHAR channel)
{
	if (WMODE_CAP_6G(PhyMode))
		return FALSE;
	else if (channel > 14)
		return TRUE;
	else
		return FALSE;
}

static VOID ZeroWaitDfsEnable(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR bZeroWaitDfsCtrl;

	bZeroWaitDfsCtrl = msg->zerowait_dfs_ctrl_msg.Enable;

#ifdef BACKGROUND_SCAN_SUPPORT
#if (RDD_2_SUPPORTED == 0)
	DfsDedicatedDynamicCtrl(pAd, bZeroWaitDfsCtrl);
#endif /* RDD_2_SUPPORTED */
#endif /* BACKGROUND_SCAN_SUPPORT */
}

static VOID ZeroWaitDfsInitAvalChListUpdate(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR Bw80TotalChNum;
	UCHAR Bw40TotalChNum;
	UCHAR Bw20TotalChNum;
	DFS_REPORT_AVALABLE_CH_LIST Bw80AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw40AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw20AvalChList[DFS_AVAILABLE_LIST_CH_NUM];

	Bw80TotalChNum = msg->aval_channel_list_msg.Bw80TotalChNum;
	Bw40TotalChNum = msg->aval_channel_list_msg.Bw40TotalChNum;
	Bw20TotalChNum = msg->aval_channel_list_msg.Bw20TotalChNum;

	memcpy(Bw80AvalChList,
		msg->aval_channel_list_msg.Bw80AvalChList,
		Bw80TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	memcpy(Bw40AvalChList,
		msg->aval_channel_list_msg.Bw40AvalChList,
		Bw40TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	memcpy(Bw20AvalChList,
		msg->aval_channel_list_msg.Bw20AvalChList,
		Bw20TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Bw20ChNum: %d\n", Bw20TotalChNum);
#ifdef DFS_DBG_LOG_0
	for (i = 0; i < Bw20TotalChNum; i++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Bw20 ChList[%d] Channel:%d\n",
			i, Bw20AvalChList[i].Channel);
	}
#endif
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Bw40ChNum: %d\n", Bw40TotalChNum);

#ifdef DFS_DBG_LOG_0
	for (i = 0; i < Bw40TotalChNum; i++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Bw40 ChList[%d] Channel:%d\n",
			i, Bw40AvalChList[i].Channel);
	}
#endif

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Bw80ChNum: %d\n", Bw80TotalChNum);

#ifdef DFS_DBG_LOG_0
	for (i = 0; i < Bw80TotalChNum; i++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Bw80 ChList[%d] Channel:%d\n",
			i, Bw80AvalChList[i].Channel);
	}
#endif
	ZeroWait_DFS_Initialize_Candidate_List(pAd,
	Bw80TotalChNum, Bw80AvalChList,
	Bw40TotalChNum, Bw40AvalChList,
	Bw20TotalChNum, Bw20AvalChList);
}

static VOID ZeroWaitDfsMonitorChUpdate(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR SynNum;
	UCHAR Channel;
	UCHAR Bw;
	BOOLEAN doCAC;

	SynNum = msg->set_monitored_ch_msg.SyncNum;
	Channel = msg->set_monitored_ch_msg.Channel;
	Bw = msg->set_monitored_ch_msg.Bw;
	doCAC = msg->set_monitored_ch_msg.doCAC;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "\x1b[1;33m[RDM] SynNum: %d, Channel: %d, Bw: %d \x1b[m \n",
		SynNum, Channel, Bw);

#ifdef BACKGROUND_SCAN_SUPPORT

	switch (SynNum) {
	case RDD_INBAND_IDX_1:
#if (RDD_2_SUPPORTED == 1)
	case RDD_INBAND_IDX_2:
#endif /* RDD_2_SUPPORTED */
		DfsDedicatedInBandSetChannel(pAd, Channel, Bw, doCAC, SynNum);
		break;

	case RDD_DEDICATED_IDX:
		DfsDedicatedOutBandSetChannel(pAd, Channel, Bw, SynNum);
		break;

	default:
		break;
	}

#endif

}

static VOID ZeroWaitDfsSetNopToChList(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR Channel = 0, Bw = 0;
	USHORT NOPTime = 0;

	Channel = msg->nop_force_set_msg.Channel;
	Bw = msg->nop_force_set_msg.Bw;
	NOPTime = msg->nop_force_set_msg.NOPTime;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "\x1b[1;33m[RDM] Channel: %d, Bw: %d, NOP: %d \x1b[m \n",
		Channel, Bw, NOPTime);

	ZeroWait_DFS_set_NOP_to_Channel_List(pAd, Channel, Bw, NOPTime);

}

static VOID ZeroWaitDfsPreAssignNextTarget(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR Channel;
	UCHAR Bw;
	USHORT CacValue;

	Channel = msg->assign_next_target.Channel;
	Bw = msg->assign_next_target.Bw;
	CacValue = msg->assign_next_target.CacValue;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "\x1b[1;33m [RDM] Channel: %d, Bw: %d \x1b[m \n",
		Channel, Bw);

	ZeroWait_DFS_Pre_Assign_Next_Target_Channel(pAd, Channel, Bw, CacValue);
}

static VOID ZeroWaitShowTargetInfo(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR mode;

	mode = msg->target_ch_show.mode;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "\x1b[1;33m[RDM] mode: %d \x1b[m \n", mode);

	ZeroWait_DFS_Next_Target_Show(pAd, mode);
}

VOID ZeroWaitDfsMsgHandle(
	PRTMP_ADAPTER pAd,
	UCHAR *msg
)
{
	switch (*msg) {
	case ZERO_WAIT_DFS_ENABLE:
		ZeroWaitDfsEnable(pAd, (union dfs_zero_wait_msg *)msg);
		break;

	case INIT_AVAL_CH_LIST_UPDATE:
		ZeroWaitDfsInitAvalChListUpdate(pAd, (union dfs_zero_wait_msg *)msg);
		break;

	case MONITOR_CH_ASSIGN:
		ZeroWaitDfsMonitorChUpdate(pAd, (union dfs_zero_wait_msg *)msg);
		break;

	case NOP_FORCE_SET:
		ZeroWaitDfsSetNopToChList(pAd, (union dfs_zero_wait_msg *)msg);
		break;

	case PRE_ASSIGN_NEXT_TARGET:
		ZeroWaitDfsPreAssignNextTarget(pAd, (union dfs_zero_wait_msg *)msg);
		break;

	case SHOW_TARGET_INFO:
		ZeroWaitShowTargetInfo(pAd, (union dfs_zero_wait_msg *)msg);
		break;
	default:
		break;
	}
}

INT ZeroWaitDfsCmdHandler(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT * wrq
)
{
	INT status = NDIS_STATUS_SUCCESS;
	union dfs_zero_wait_msg msg;

	if (!wrq)
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&msg, sizeof(msg));

	if (wrq->u.data.length != sizeof(union dfs_zero_wait_msg))
		return NDIS_STATUS_FAILURE;

	if (copy_from_user(&msg, wrq->u.data.pointer, wrq->u.data.length)) {
		status = -EFAULT;
	} else {
		ZeroWaitDfsMsgHandle(pAd, (CHAR *)&msg);
	}

	return status;
}

static VOID ZeroWaitDfsQueryNopOfChList(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR ch_idx = 0;
	DfsProvideNopOfChList(pAd, msg);

	for (ch_idx = 0; ch_idx < msg->nop_of_channel_list_msg.NOPTotalChNum; ch_idx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_DEBUG,
		"NopReportChList[%d].Channel = %d, Bw = %d, NOP = %d\n",
		ch_idx,
		msg->nop_of_channel_list_msg.NopReportChList[ch_idx].Channel,
		msg->nop_of_channel_list_msg.NopReportChList[ch_idx].Bw,
		msg->nop_of_channel_list_msg.NopReportChList[ch_idx].NonOccupancy);
	}
}


VOID ZeroWaitDfsQueryAvalChListNonDbdc(PRTMP_ADAPTER pAd, UCHAR *Bw80ChNum, UCHAR *Bw40ChNum, UCHAR *Bw20ChNum,
			UCHAR *Bw160ChNum,
			DFS_REPORT_AVALABLE_CH_LIST Bw80AvailableChList[DFS_AVAILABLE_LIST_CH_NUM],
			DFS_REPORT_AVALABLE_CH_LIST Bw40AvailableChList[DFS_AVAILABLE_LIST_CH_NUM],
			DFS_REPORT_AVALABLE_CH_LIST Bw20AvailableChList[DFS_AVAILABLE_LIST_CH_NUM],
			DFS_REPORT_AVALABLE_CH_LIST Bw160AvailableChList[DFS_AVAILABLE_LIST_CH_NUM])
{
	UCHAR band_idx = 0;
	UINT_8 bw_idx, ch_idx, idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	PCHANNEL_CTRL pChCtrl = NULL;

	if (pAd->Dot11_H.ChannelMode == CHAN_SWITCHING_MODE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"Channel list query fail during channel switch\n");
		return;
	}

	for (bw_idx = 0; bw_idx < DFS_AVAILABLE_LIST_BW_NUM; bw_idx++) {
		for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
			pDfsParam->dfs_ch_grp.AvailableBwChIdx[bw_idx][ch_idx] = 0xff;
		}
	}

	DfsBwChQueryAllList(pAd, BW_160, pDfsParam, FALSE, band_idx);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
		if (pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_20][ch_idx] != 0xff) {
			idx = pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_20][ch_idx];
			Bw20AvailableChList[ch_idx].Channel = pChCtrl->ChList[idx].Channel;
			Bw20AvailableChList[ch_idx].RadarHitCnt = pChCtrl->ChList[idx].NOPClrCnt;
		} else
			break;
	}
	*Bw20ChNum = ch_idx;

	for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
		if (pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_40][ch_idx] != 0xff) {
			idx = pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_40][ch_idx];
			Bw40AvailableChList[ch_idx].Channel = pChCtrl->ChList[idx].Channel;
			Bw40AvailableChList[ch_idx].RadarHitCnt = pChCtrl->ChList[idx].NOPClrCnt;
		} else
			break;
	}
	*Bw40ChNum = ch_idx;

	for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
		if (pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_80][ch_idx] != 0xff) {
			idx = pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_80][ch_idx];
			Bw80AvailableChList[ch_idx].Channel = pChCtrl->ChList[idx].Channel;
			Bw80AvailableChList[ch_idx].RadarHitCnt = pChCtrl->ChList[idx].NOPClrCnt;
		} else
			break;
	}
	*Bw80ChNum = ch_idx;
	for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
		if (pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_160][ch_idx] != 0xff) {
			idx = pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_160][ch_idx];
			Bw160AvailableChList[ch_idx].Channel = pChCtrl->ChList[idx].Channel;
			Bw160AvailableChList[ch_idx].RadarHitCnt = pChCtrl->ChList[idx].NOPClrCnt;
		} else
			break;
	}
	*Bw160ChNum = ch_idx;

}
static VOID ZeroWaitDfsQueryAvalChList(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UINT_8 bw_idx, ch_idx, idx;
	UCHAR BandIdx = hc_get_hw_band_idx(pAd);

	UCHAR Bw80TotalChNum = 0;
	UCHAR Bw40TotalChNum = 0;
	UCHAR Bw20TotalChNum = 0;
	UCHAR Bw160TotalChNum = 0;
	DFS_REPORT_AVALABLE_CH_LIST Bw80AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw40AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw20AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw160AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	PCHANNEL_CTRL pChCtrl = NULL;

	os_zero_mem(&Bw80AvailableChList, sizeof(DFS_REPORT_AVALABLE_CH_LIST) * DFS_AVAILABLE_LIST_CH_NUM);
	os_zero_mem(&Bw40AvailableChList, sizeof(DFS_REPORT_AVALABLE_CH_LIST) * DFS_AVAILABLE_LIST_CH_NUM);
	os_zero_mem(&Bw20AvailableChList, sizeof(DFS_REPORT_AVALABLE_CH_LIST) * DFS_AVAILABLE_LIST_CH_NUM);
	os_zero_mem(&Bw160AvailableChList, sizeof(DFS_REPORT_AVALABLE_CH_LIST) * DFS_AVAILABLE_LIST_CH_NUM);

	if (pAd->CommonCfg.dbdc_mode == 0) {
		ZeroWaitDfsQueryAvalChListNonDbdc(pAd, &Bw80TotalChNum, &Bw40TotalChNum, &Bw20TotalChNum,
									&Bw160TotalChNum, Bw80AvailableChList,
									Bw40AvailableChList, Bw20AvailableChList,
									Bw160AvailableChList);
	} else {
		if (pAd->Dot11_H.ChannelMode == CHAN_SWITCHING_MODE) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				"Channel list query fail during channel switch\n");
			return;
		}

		for (bw_idx = 0; bw_idx < DFS_AVAILABLE_LIST_BW_NUM; bw_idx++) {
			for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++)
				pDfsParam->dfs_ch_grp.AvailableBwChIdx[bw_idx][ch_idx]
											= 0xff;
		}

		DfsBwChQueryAllList(pAd, BW_160, pDfsParam, FALSE, BandIdx);

		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

		for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
			if (pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_20][ch_idx] != 0xff) {
				idx = pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_20][ch_idx];
				Bw20AvailableChList[ch_idx].Channel = pChCtrl->ChList[idx].Channel;
				Bw20AvailableChList[ch_idx].RadarHitCnt = pChCtrl->ChList[idx].NOPClrCnt;
			} else
				break;
		}
		Bw20TotalChNum = ch_idx;

		for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
			if (pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_40][ch_idx] != 0xff) {
				idx = pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_40][ch_idx];
				Bw40AvailableChList[ch_idx].Channel = pChCtrl->ChList[idx].Channel;
				Bw40AvailableChList[ch_idx].RadarHitCnt = pChCtrl->ChList[idx].NOPClrCnt;
			} else
				break;
		}
		Bw40TotalChNum = ch_idx;

		for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
			if (pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_80][ch_idx] != 0xff) {
				idx = pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_80][ch_idx];
				Bw80AvailableChList[ch_idx].Channel = pChCtrl->ChList[idx].Channel;
				Bw80AvailableChList[ch_idx].RadarHitCnt = pChCtrl->ChList[idx].NOPClrCnt;
			} else
				break;
		}
		Bw80TotalChNum = ch_idx;
		for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
			if (pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_160][ch_idx] != 0xff) {
				idx = pDfsParam->dfs_ch_grp.AvailableBwChIdx[BW_160][ch_idx];
				Bw160AvailableChList[ch_idx].Channel = pChCtrl->ChList[idx].Channel;
				Bw160AvailableChList[ch_idx].RadarHitCnt = pChCtrl->ChList[idx].NOPClrCnt;
			} else
				break;
		}
		Bw160TotalChNum = ch_idx;
	}
	msg->aval_channel_list_msg.Bw80TotalChNum = Bw80TotalChNum;
	msg->aval_channel_list_msg.Bw40TotalChNum = Bw40TotalChNum;
	msg->aval_channel_list_msg.Bw20TotalChNum = Bw20TotalChNum;
	msg->aval_channel_list_msg.Bw160TotalChNum = Bw160TotalChNum;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		"Bw20ChNum: %d\n", msg->aval_channel_list_msg.Bw20TotalChNum);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		"Bw40ChNum: %d\n", msg->aval_channel_list_msg.Bw40TotalChNum);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		"Bw80ChNum: %d\n", msg->aval_channel_list_msg.Bw80TotalChNum);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		"Bw160ChNum: %d\n", msg->aval_channel_list_msg.Bw160TotalChNum);

	memcpy(msg->aval_channel_list_msg.Bw80AvalChList,
		Bw80AvailableChList,
		Bw80TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	memcpy(msg->aval_channel_list_msg.Bw40AvalChList,
		Bw40AvailableChList,
		Bw40TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	memcpy(msg->aval_channel_list_msg.Bw20AvalChList,
		Bw20AvailableChList,
		Bw20TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	memcpy(msg->aval_channel_list_msg.Bw160AvalChList,
		Bw160AvailableChList,
		Bw160TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	for (ch_idx = 0; ch_idx < msg->aval_channel_list_msg.Bw80TotalChNum; ch_idx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		"ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
		ch_idx,
		Bw80AvailableChList[ch_idx].Channel,
		Bw80AvailableChList[ch_idx].RadarHitCnt);
	}
	for (ch_idx = 0; ch_idx < msg->aval_channel_list_msg.Bw40TotalChNum; ch_idx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		"ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
		ch_idx,
		Bw40AvailableChList[ch_idx].Channel,
		Bw40AvailableChList[ch_idx].RadarHitCnt);
	}
	for (ch_idx = 0; ch_idx < msg->aval_channel_list_msg.Bw20TotalChNum; ch_idx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		"ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
		ch_idx,
		Bw20AvailableChList[ch_idx].Channel,
		Bw20AvailableChList[ch_idx].RadarHitCnt);
	}
	for (ch_idx = 0; ch_idx < msg->aval_channel_list_msg.Bw160TotalChNum; ch_idx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		"ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
		ch_idx,
		Bw160AvailableChList[ch_idx].Channel,
		Bw160AvailableChList[ch_idx].RadarHitCnt);
	}

}

VOID ZeroWaitDfsQueryMsgHandle(
	PRTMP_ADAPTER pAd,
	UCHAR *msg
)
{
	switch (*msg) {
	case QUERY_AVAL_CH_LIST:
		ZeroWaitDfsQueryAvalChList(pAd, (union dfs_zero_wait_msg *)msg);
		break;
	case QUERY_NOP_OF_CH_LIST:
		ZeroWaitDfsQueryNopOfChList(pAd, (union dfs_zero_wait_msg *)msg);
		break;
	default:
		break;
	}
}

BOOLEAN DfsCheckHitBandBWDbdcMode(PRTMP_ADAPTER pAd, UCHAR bw)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if ((pDfsParam->DFSChHitBand != DFS_BAND_NONE) && (bw == BW_80) && !(pAd->CommonCfg.dbdc_mode))
		return true;
	else
		return false;
}


INT ZeroWaitDfsQueryCmdHandler(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT * wrq
)
{
	INT status = NDIS_STATUS_SUCCESS;
	union dfs_zero_wait_msg msg;
	os_zero_mem(&msg, sizeof(union dfs_zero_wait_msg));
#ifdef MAP_R2
	if (IS_MAP_TURNKEY_ENABLE(pAd))
		msg.aval_channel_list_msg.Action = QUERY_AVAL_CH_LIST;
#endif
	ZeroWaitDfsQueryMsgHandle(pAd, (CHAR *)&msg);
	wrq->u.data.length = sizeof(union dfs_zero_wait_msg);

	if (copy_to_user(wrq->u.data.pointer, &msg, wrq->u.data.length)) {
		status = -EFAULT;
	}

	return status;
}

PCHANNEL_CTRL DfsGetChCtrl(
	IN PRTMP_ADAPTER pAd)
{
	PCHANNEL_CTRL pChCtrl = NULL;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	return pChCtrl;
}

UCHAR DfsGetNonDfsDefaultCh(/*Get non-DFS default channel*/
	IN PRTMP_ADAPTER pAd,
	IN PDFS_PARAM pDfsParam)
{
	UCHAR channel = 0;
	UCHAR firstDfsChannelInList = 0;
	UCHAR i;
	PCHANNEL_CTRL pChCtrl = NULL;
	BOOLEAN bAvailableNonDfsCh = FALSE;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	if (pChCtrl->ch_cfg.chan_prio_valid) {
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			if (RadarChannelCheck(pAd, pChCtrl->ChList[i].Channel)) {
				if (IS_CH_BETWEEN(pChCtrl->ChList[i].Channel, 36, 48) &&
					pChCtrl->ChList[i].Priority > 0) {
					bAvailableNonDfsCh = TRUE;
					channel = pChCtrl->ChList[i].Channel;
					break;
				}

				if (pChCtrl->ChList[i].Priority > 0 && firstDfsChannelInList == 0)
					firstDfsChannelInList = pChCtrl->ChList[i].Channel;
			} else if (pChCtrl->ChList[i].Priority > 0) {
				bAvailableNonDfsCh = TRUE;
				channel = pChCtrl->ChList[i].Channel;
				break;
			}
		}

		if (bAvailableNonDfsCh == FALSE) {
			if (pDfsParam->OutBandCh != 0) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
					"No available non-DFS Ch, return outband ch(%d)\n", pDfsParam->OutBandCh);
				channel = pDfsParam->OutBandCh;
			} else {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
					"No available non-DFS Ch, return first DFS ch in ch list(%d)\n", firstDfsChannelInList);
				channel = firstDfsChannelInList;
			}
		}
	} else {
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			if (pChCtrl->ChList[i].DfsReq != TRUE) {
				if (pChCtrl->ChList[i].Channel == 36 && pDfsParam->OutBandCh == 36)
					continue;

				channel = pChCtrl->ChList[i].Channel;
				break;
			}
		}
	}

	return channel;
}

#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
INT zero_wait_dfs_update_inband_nondfsch(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	INOUT PUCHAR ch
)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
	CHANNEL_CTRL *pChCtrl;
	UCHAR i;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Inband BW : %d\n", pDfsParam->band_bw);

	if (pDfsParam->bNoAvailableCh &&
		pDfsParam->DfsNopExpireSetChPolicy == DFS_NOP_EXPIRE_SET_CH_POLICY_RESTORE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
		"No Available Channel now, so disable MAC TX.\n");
		pDfsParam->bBootUpInBandUnavailableCh = TRUE;
		return TRUE;
	}

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	if (pChCtrl->ch_cfg.chan_prio_valid) {
		/* Select non-dfs channel by priority or zero */
		pDfsParam->bBootUpInBandUnavailableCh = TRUE;
		*ch = 0;
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			if (RadarChannelCheck(pAd, pChCtrl->ChList[i].Channel)) {
				if (IS_CH_BETWEEN(pChCtrl->ChList[i].Channel, 36, 48) && pChCtrl->ChList[i].Priority > 0) {
					*ch = pChCtrl->ChList[i].Channel;
					pDfsParam->bBootUpInBandUnavailableCh = FALSE;
					if (pDfsParam->band_bw == BW_160 || pDfsParam->band_bw == BW_80)
						DfsAdjustOpBwSetting(pAd, wdev, BW_160, BW_80);

					break;
				}
			} else if (pChCtrl->ChList[i].Priority > 0) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				"Get non-DFS ch\n");
				*ch = pChCtrl->ChList[i].Channel;
				pDfsParam->bBootUpInBandUnavailableCh = FALSE;
				break;
			}
		}

		if (*ch == 0) {
			*ch = pAd->CommonCfg.DfsParameter.OutBandCh;
			pAd->CommonCfg.DfsParameter.bSetInBandCacReStart = TRUE;

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"Not find non-DFS ch, set ch to %d, but disable now(waiting CAC)\n", *ch);

			RadarStateCheck(pAd, wdev);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
					"Get non-DFS ch(%d)\n", *ch);
			if (pDfsParam->band_bw == BW_160 || pDfsParam->band_bw == BW_80)
				DfsAdjustOpBwSetting(pAd, wdev, BW_160, BW_80);
		}
	} else {
		if (pDfsParam->band_bw == BW_160 || pDfsParam->band_bw == BW_80) {
			*ch = FirstNonDfsChannel(pAd);
			if (*ch == 0)
				*ch = 36;
			DfsAdjustOpBwSetting(pAd, wdev, BW_160, BW_80);
		} else
			*ch = 36;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				 "DFS ch %d is selected, use non-DFS ch %d, ch_stat %d\n",
					pAd->CommonCfg.DfsParameter.OutBandCh,
					*ch,
					*ch_stat);
	}

	return TRUE;
}


BOOLEAN mt_validate_dfs_channel_for_zw_dfs(RTMP_ADAPTER *pAdapter, struct wifi_dev *wdev, PUCHAR ch)
{
	BSS_ENTRY *bss;
	UINT i = 0;
	PBSS_TABLE ScanTab = NULL;

	if ((wdev->wdev_type != WDEV_TYPE_STA) &&
	(wdev->wdev_type != WDEV_TYPE_REPEATER))
		return TRUE;

	ScanTab = get_scan_tab_by_wdev(pAdapter, wdev);
	if (ScanTab->BssNr == 0)
		return TRUE;

	for (i = 0; i < ScanTab->BssNr; i++) {
		bss = &ScanTab->BssEntry[i];
		if (bss->Channel == *ch)
			return FALSE;
	}
	return TRUE;
}

/*
RDD_INBAND_IDX_1 = Bandidx 1, Phyidx 1, rddidx 1
RDD_INBAND_IDX_2 = Bandidx 2, Phyidx 2, rddidx 0
RDD_DEDICATED_IDX = Bandidx 1, Phyidx 0, rddidx 2
*/
INT zero_wait_dfs_update_ch(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR OriChannel,
	INOUT PUCHAR ch
)
{
	PUCHAR ch_outband = &pAd->CommonCfg.DfsParameter.OutBandCh;
	PUCHAR phy_bw_outband = &pAd->CommonCfg.DfsParameter.OutBandBw;
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	UCHAR RddIdx = (BandIdx == BAND1) ? RDD_INBAND_IDX_1 : RDD_INBAND_IDX_2;

#ifdef MAP_R2
	int i = 0;
#endif
	BOOLEAN orichannel_is_nondfs;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
		"ch %d, outband ch %d, ch_stat %d, pDfsParam->OutBandBw=%d, pDfsParam->band_bw=%d\n",
		*ch, *ch_outband, *ch_stat, pDfsParam->OutBandBw, pDfsParam->band_bw);

	if (pDfsParam->bDedicatedZeroWaitDefault == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"bDedicatedZeroWaitDefault == 0\n");
		return FALSE;
	}

	if (wlan_config_get_ch_band(wdev) != CMD_CH_BAND_5G)
		return FALSE;

	if ((pDfsParam->bDedicatedZeroWaitSupport == FALSE
	|| hc_get_hw_band_idx(pAd) != pDfsParam->ZeroWaitBandidx))
		return FALSE;

#ifdef DFS_ADJ_BW_ZERO_WAIT
	if (pDfsParam->BW160ZeroWaitSupport == TRUE)
		return FALSE;
#endif

#ifdef MAP_R2
	if (IS_MAP_TURNKEY_ENABLE(pAd) && pDfsParam->bDfsEnable) {

		if (!mt_validate_dfs_channel_for_zw_dfs(pAd, wdev, ch))
			return FALSE;

		for (i = 0; i < MAX_BEACON_NUM; i++) {
			if ((pAd->ApCfg.MBSSID[i].wdev.channel == wdev->channel)
			   && (pAd->ApCfg.MBSSID[i].wdev.cac_not_required == TRUE))
				return FALSE;
		}
	}
#endif
	orichannel_is_nondfs = !RadarChannelCheck(pAd, OriChannel);

	switch (*ch_stat) {
	case DFS_INB_DFS_OUTB_CH_CAC:
		if (pDfsParam->SetOutBandChStat != OUTB_SET_CH_CAC)
			return FALSE;
		fallthrough;
	case DFS_INB_CH_INIT:
	case DFS_OUTB_CH_CAC:
		/* If DFS ch X is selected, CAC of DFS ch X will be checked by dedicated RX */
		/* Update new channel as outband Channel */

		if (pDfsParam->BW80DedicatedZWSupport == TRUE &&
			pDfsParam->band_bw == BW_160 && (IS_CH_BETWEEN(*ch, 36, 64))) {
			pDfsParam->OutBandOriCh = *ch;
			pDfsParam->OutBandCh = 52;
		} else
			pDfsParam->OutBandCh = *ch;

		dfs_get_outband_bw(pAd, wdev, phy_bw_outband);

		if (pDfsParam->BW80DedicatedZWSupport == TRUE && pDfsParam->band_bw == BW_160)
			*phy_bw_outband = BW_80;

		if (*ch_stat == DFS_INB_CH_INIT || *ch_stat == DFS_OUTB_CH_CAC) {
			/* Stop RDD */
			mtRddControl(pAd, RDD_STOP, RddIdx, 0, 0);
		} else if (pDfsParam->SetOutBandChStat == OUTB_SET_CH_CAC) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
					"Do not stop RDD\n");
		}

		/* Need to update non-DFS ch Y as new ch if Original channel is unsafe channel or bootup from channel 0*/
		if (OriChannel == 0
#ifdef WIFI_MD_COEX_SUPPORT
			|| (!IsChannelSafe(pAd, OriChannel) && !IsPwrChannelSafe(pAd, OriChannel))
#endif
			)
			zero_wait_dfs_update_inband_nondfsch(pAd, wdev, ch);

		/* No need to update non-DFS ch Y as new ch if original channel is a non-DFS channel*/
		else if (orichannel_is_nondfs) {
			*ch = OriChannel;
			if (pDfsParam->band_bw != BW_160)
				pAd->CommonCfg.DfsParameter.ZwAdjBw = pDfsParam->band_bw;

			if (pDfsParam->band_bw == BW_160 || pDfsParam->band_bw == BW_80) {
				DfsAdjustOpBwSetting(pAd, wdev, BW_160, BW_80);
				pDfsParam->band_bw = BW_80;
			}

			pAd->CommonCfg.DfsParameter.ZwAdjBwFlag = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
				"DFS ch %d is selected, use non-DFS ch %d, ch_stat %d\n",
				pAd->CommonCfg.DfsParameter.OutBandCh,
				*ch,
				*ch_stat);
		}

		/* No need to update non-DFS ch Y as new ch */
		/* if original channel is a DFS channel & CAC Done*/
		else if (pDfsParam->SetOutBandChStat == OUTB_SET_CH_CAC) {
			*ch = OriChannel;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				"OutBand DFS ch %d is selected, use origin DFS ch %d, ch_stat %d\n",
				pAd->CommonCfg.DfsParameter.OutBandCh,
				*ch,
				*ch_stat);
		}

		/* Need to update non-DFS ch Y as new ch if Original channel is a DFS channel*/
		else
			zero_wait_dfs_update_inband_nondfsch(pAd, wdev, ch);
		/* 5th RX is set */
		if (*ch_outband != 0) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
					"5th RX is set to ch%d\n", *ch_outband);

		}
		break;
	case DFS_INB_DFS_RADAR_OUTB_CAC_DONE:
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
			"Do not switch to DFS ch immediately!\n");
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
			"ch_stat %d\n", *ch_stat);
		return FALSE;

	default:
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"ch_stat %d\n", *ch_stat);
		return FALSE;

	}

	return TRUE;
}

#ifdef BACKGROUND_SCAN_SUPPORT
INT zero_wait_dfs_switch_ch(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR band_idx
)
{
	PUCHAR ch_outband = &pAd->CommonCfg.DfsParameter.OutBandCh;
	PUCHAR phy_bw_outband = &pAd->CommonCfg.DfsParameter.OutBandBw;
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
	PRALINK_TIMER_STRUCT set_ob_ch_timer = &pAd->BgndScanCtrl.DfsZeroWaitTimer;
	ULONG wait_time = 500; /* Wait for 6,000 ms */

	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR BandIdx = 0;
	UCHAR RddIdx = 0;

	(VOID)band_idx;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
		"(caller:%pS), outband ch %d, ch_stat %d\n", OS_TRACE, *ch_outband, *ch_stat);

	if (!WMODE_CAP_5G(wdev->PhyMode))
		return FALSE;

	if (pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
			"bDedicatedZeroWaitDefault == 0\n");
		return FALSE;
	}

	if ((pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport == FALSE
	|| hc_get_hw_band_idx(pAd) != pAd->CommonCfg.DfsParameter.ZeroWaitBandidx)
	|| pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport == TRUE)
		return FALSE;

	switch (*ch_stat) {
	case DFS_INB_CH_INIT:
		*ch_stat = DFS_OUTB_CH_CAC;
		fallthrough;
	case DFS_INB_DFS_RADAR_OUTB_CAC_DONE:
		if (*ch_outband != 0) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
					"OutBandCh %d, OutBandBw %d\n", *ch_outband, *phy_bw_outband);

			if (*ch_stat == DFS_INB_DFS_RADAR_OUTB_CAC_DONE) {
				wait_time = 2500; /* Wait for 2,000 ms */
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
					"Do not switch to DFS ch immediately\n");
			}
			/* Initialize timer for setting a DFS channel later */
			RTMPInitTimer(pAd, &pAd->BgndScanCtrl.DfsZeroWaitTimer, GET_TIMER_FUNCTION(dfs_zero_wait_ch_init_timeout), pAd, FALSE);
			/* Set out-band channel after calibration is done */
			RTMPSetTimer(set_ob_ch_timer, wait_time);
		}
		break;

	case DFS_OUTB_CH_CAC:
	case DFS_INB_CH_SWITCH_CH:
	case DFS_INB_DFS_OUTB_CH_CAC:
	case DFS_INB_DFS_OUTB_CH_CAC_DONE:
	case DFS_INB_DFS_RADAR_OUTB_CAC_DONE_QUICK: /* For no available non-DFS Channel case */
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
			"OUTBAND_SWITCH, ch_stat %d\n", *ch_stat);

		if (*ch_stat == DFS_INB_DFS_RADAR_OUTB_CAC_DONE_QUICK) {
			BandIdx = dfs_get_band_by_ch(pAd, pDfsParam->OutBandCh);
			RddIdx = (BandIdx == BAND1) ? RDD_INBAND_IDX_1 : RDD_INBAND_IDX_2;

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
				"Quickly set outband ch(%d) to inband\n", pDfsParam->OutBandCh);

			pDfsParam->bOutBandAvailable = TRUE;
			pDfsParam->bSetInBandCacReStart = FALSE;

			/* Assign DFS outband Channel to inband Channel */
			*ch_stat = DFS_INB_CH_SWITCH_CH;

			DfsDedicatedInBandSetChannel(pAd, pDfsParam->OutBandCh, pDfsParam->OutBandBw, FALSE, RddIdx);
		}

		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_RDD_DETEC;
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_OUTBAND_SWITCH, 0, NULL, 0);
		RTMP_MLME_HANDLER(pAd);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "ch_stat %d\n", *ch_stat);
		return FALSE;

	}

	return TRUE;
}
#endif

#endif


#ifdef DFS_ADJ_BW_ZERO_WAIT

VOID Adj_ZeroWait_Status_Update(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	INOUT PUCHAR  new_bw,
	INOUT PUCHAR ch
	)
{

	UCHAR BandIdx = 0;
	struct DOT11_H *pDot11hTest = NULL;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	pDot11hTest = &pAd->Dot11_H;


	BandIdx = HcGetBandByWdev(wdev);
	if (BandIdx >= RDD_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			 "Invalid BandIdx(%d)\n", BandIdx);
		return;
	}

	if (IS_CH_BETWEEN(*ch, 36, 64)
	&& *new_bw == BW_160) {
		pDfsParam->band_orich = *ch;
		*ch = 36;
		pDfsParam->BW160ZeroWaitState = DFS_BW160_TX80RX160;
		DfsAdjustOpBwSetting(pAd, wdev, BW_160, BW_80);
		pDfsParam->band_bw = BW_80;
		*new_bw = BW_80;
		pDot11hTest->RDCount = 0;
		pDot11hTest->InServiceMonitorCount = 0;
	} else if (IS_CH_BETWEEN(*ch, 52, 64)
	&& *new_bw == BW_80) {
		pDfsParam->band_orich = *ch;
		*ch = 36;
		pDfsParam->BW160ZeroWaitState = DFS_BW80_TX80RX160;
		pDot11hTest->RDCount = 0;
		pDot11hTest->InServiceMonitorCount = 0;
	} else if (IS_CH_BETWEEN(*ch, 100, 128)
	&& *new_bw == BW_160) {
		pDfsParam->band_orich = *ch;
		pDfsParam->BW160ZeroWaitState = DFS_BW160_TX0RX0;
	} else if (IS_CH_BETWEEN(*ch, 100, 128)
	&& *new_bw == BW_80) {
		pDfsParam->band_orich = *ch;
		pDfsParam->BW160ZeroWaitState = DFS_BW80_TX0RX0;
	} else
		pDfsParam->BW160ZeroWaitState = DFS_NOT_ENABLE;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
		"[RDM] SW based ZW-DFS init, set to Channel:%d, BW:%d, state:%d\n",
		*ch, *new_bw, pDfsParam->BW160ZeroWaitState);

}

#endif /*DFS_ADJ_BW_ZERO_WAIT*/

#ifdef CONFIG_AP_SUPPORT
static inline BOOLEAN AutoChannelSkipListCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	UCHAR i;

	for (i = 0; i < pAd->ApCfg.AutoChannelSkipListNum; i++) {
		if (Ch == pAd->ApCfg.AutoChannelSkipList[i])
			return TRUE;
	}

	return FALSE;
}
#endif

static inline UCHAR CentToPrim(
	UCHAR Channel)
{
	return Channel - 2;
}

static BOOLEAN DfsCheckChAvailableByBw(
	UCHAR Channel, UCHAR Bw, PCHANNEL_CTRL pChCtrl)
{
#define BW40_CHGRP_NUM	15
#define BW80_CHGRP_NUM	8
#define BW160_CHGRP_NUM 4

	UCHAR i = 0, j = 0, k = 0;
	UCHAR *pBwChGroup = NULL;
	UCHAR BW40_CH_GROUP[BW40_CHGRP_NUM][2] = {
	{36, 40}, {44, 48},
	{52, 56}, {60, 64},
	{100, 104}, {108, 112},
	{116, 120}, {124, 128},
	{132, 136}, {140, 144},
	{149, 153}, {157, 161},
	{165, 169}, {173, 177}, {0, 0}
	};

	UCHAR BW80_CH_GROUP[BW80_CHGRP_NUM][4] = {
	{36, 40, 44, 48},
	{52, 56, 60, 64},
	{100, 104, 108, 112},
	{116, 120, 124, 128},
	{132, 136, 140, 144},
	{149, 153, 157, 161},
	{165, 169, 173, 177},
	{0, 0, 0, 0}
	};

	UCHAR BW160_CH_GROUP[BW160_CHGRP_NUM][8] = {
	{36, 40, 44, 48, 52, 56, 60, 64},
	{100, 104, 108, 112, 116, 120, 124, 128},
	{149, 153, 157, 161, 165, 169, 173, 177},
	{0, 0, 0, 0, 0, 0, 0, 0}
	};

	if (Bw == BW_20)
		return TRUE;
	else if (Bw == BW_40) {
		pBwChGroup = &BW40_CH_GROUP[0][0];
		while (*pBwChGroup != 0) {
			if (*pBwChGroup == Channel)
				break;
			i++;
			if (i >= sizeof(BW40_CH_GROUP))
				return FALSE;
			pBwChGroup++;
		}
		i /= 2;
		for (j = 0; j < pChCtrl->ChListNum; j++) {
			if (pChCtrl->ChList[j].Channel == BW40_CH_GROUP[i][0])
				break;
		}

		if (j == pChCtrl->ChListNum)
			return FALSE;
		else if (pChCtrl->ChList[j+1].Channel == BW40_CH_GROUP[i][1])
			return TRUE;
	} else if (Bw == BW_80) {
		pBwChGroup = &BW80_CH_GROUP[0][0];
		while (*pBwChGroup != 0) {
			if (*pBwChGroup == Channel)
				break;
			i++;
			if (i >= sizeof(BW80_CH_GROUP))
				return FALSE;
			pBwChGroup++;
		}
		i /= 4;
		for (j = 0; j < pChCtrl->ChListNum; j++) {
			if (pChCtrl->ChList[j].Channel == BW80_CH_GROUP[i][0])
				break;
		}
		if (j == pChCtrl->ChListNum)
			return FALSE;
		else if ((pChCtrl->ChList[j+1].Channel == BW80_CH_GROUP[i][1])
			&& (pChCtrl->ChList[j+2].Channel == BW80_CH_GROUP[i][2])
			&& (pChCtrl->ChList[j+3].Channel == BW80_CH_GROUP[i][3])
		)
			return TRUE;

	} else if (Bw == BW_160) {
		pBwChGroup = &BW160_CH_GROUP[0][0];
		while (*pBwChGroup != 0) {
			if (*pBwChGroup == Channel)
				break;
			i++;
			if (i >= sizeof(BW160_CH_GROUP))
				return FALSE;
			pBwChGroup++;
		}
		i /= 8;
		for (j = 0; j < pChCtrl->ChListNum; j++) {
			if (pChCtrl->ChList[j].Channel == BW160_CH_GROUP[i][0])
				break;
		}
		if (j == pChCtrl->ChListNum)
			return FALSE;
		else {
			for (k = 1; k < 7 ; k++) {
				if (pChCtrl->ChList[j+k].Channel != BW160_CH_GROUP[i][k])
					return FALSE;
			}
			return TRUE;
		}
	}

	return FALSE;
}

static BOOLEAN ByPassChannelByBw(
	UCHAR Channel, UCHAR Bw, PCHANNEL_CTRL pChCtrl)
{
	UINT_8 i;
	BOOLEAN BwSupport = FALSE;

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (Channel == pChCtrl->ChList[i].Channel)
			BwSupport = (pChCtrl->ChList[i].SupportBwBitMap) & BIT(Bw);
	}

	if (BwSupport)
		return FALSE;
	else
		return TRUE;

}

UCHAR DfsPrimToCent(
	UCHAR Channel, UCHAR Bw)
{
	UINT_8 i = 0;

	UCHAR CH_EXT_ABOVE[] = {
	36, 44, 52, 60,
	100, 108, 116, 124,
	132, 140, 149, 157, 0
	};

	UCHAR CH_EXT_BELOW[] = {
	40, 48, 56, 64,
	104, 112, 120, 128,
	136, 144, 153, 161, 0
	};

	if (Bw == BW_20)
		return Channel;
	else if (Bw == BW_40) {
		while (CH_EXT_ABOVE[i] != 0) {
			if (Channel == CH_EXT_ABOVE[i]) {
				return Channel + 2;
			} else if (Channel == CH_EXT_BELOW[i]) {
				return Channel - 2;
			}
			i++;
		}
	} else if (Bw == BW_80)
		return vht_cent_ch_freq(Channel, VHT_BW_80, CMD_CH_BAND_5G);
	else if (Bw == BW_160)
		return vht_cent_ch_freq(Channel, VHT_BW_160, CMD_CH_BAND_5G);

	return Channel;
}

UCHAR DfsGetBgndParameter(
	IN PRTMP_ADAPTER pAd, UCHAR QueryParam)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	switch (QueryParam) {
#if (RDD_2_SUPPORTED == 1)

	case INBAND_CH_BAND1:
	case INBAND_CH_BAND2:
		return pDfsParam->band_ch;

	case INBAND_BW_BAND1:
	case INBAND_BW_BAND2:
		return pDfsParam->band_bw;

#else
	case INBAND_CH:
		return pDfsParam->band_ch;

	case INBAND_BW:
		return pDfsParam->band_bw;

#endif /* RDD_2_SUPPORTED */

	case OUTBAND_CH:
		return pDfsParam->OutBandCh;

	case OUTBAND_BW:
		return pDfsParam->OutBandBw;

	case ORI_INBAND_CH:
		return pDfsParam->OrigInBandCh;

	case ORI_INBAND_BW:
		return pDfsParam->OrigInBandBw;

	default:
		return pDfsParam->OutBandCh;

	}
}

VOID DfsGetSysParameters(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR vht_cent2,
	UCHAR phy_bw)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR prim_ch;
	UCHAR Ch;
	UCHAR bandIdx;
	CHANNEL_CTRL *pChCtrl;

	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	prim_ch = wdev->channel;
	bandIdx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	if (phy_bw <= BW_160) {
		while (ByPassChannelByBw(wdev->channel, phy_bw, pChCtrl)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
					"Warning:This Channel can not match BW(%d)\n", phy_bw);
			if (phy_bw <= BW_20)
				break;
			phy_bw = phy_bw - 1;
		}
	}

	/* Avoid 'pDfsParam->band_bw' be adjusted to BW_160 when ch52~64 in NOP status */
	if (phy_bw == BW_160 && (IS_CH_BETWEEN(wdev->channel, 36, 48))) {
		for (Ch = 52; Ch <= 64; Ch += 4) {
			if (CheckNonOccupancyChannel(pAd, wdev, Ch) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
						 "Adjust phy_bw(%d) to bw(%d) for channel(%d)\n",
						 phy_bw, BW_80, wdev->channel);
				phy_bw = BW_80;
				break;
			}
		}
	}

	pDfsParam->PrimCh = prim_ch;
	pDfsParam->PrimBand = bandIdx;
	pDfsParam->band_ch = prim_ch;
	pDfsParam->band_bw = phy_bw;
	pDfsParam->Dot11_H.ChannelMode = pDot11h->ChannelMode;
	pDfsParam->bIEEE80211H = pAd->CommonCfg.bIEEE80211H;
	pDfsParam->bDfsEnable = pAd->CommonCfg.DfsParameter.bDfsEnable;
}


VOID DfsParamInit(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR rdd_idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	PCHANNEL_CTRL pChCtrl = NULL;
	PDFS_PULSE_THRESHOLD_PARAM pls_thrshld_param = NULL;
	PDFS_RADAR_THRESHOLD_PARAM radar_thrshld_param = NULL;

	os_zero_mem(pDfsParam, sizeof(DFS_PARAM));

	pDfsParam->PrimBand = RDD_BAND0;
	for (rdd_idx = 0; rdd_idx < HW_RDD_NUM; rdd_idx++) {
		pDfsParam->DfsChBand[rdd_idx] = FALSE;
		pDfsParam->RadarDetected[rdd_idx] = FALSE;
		pDfsParam->RadarDetectState[rdd_idx] = FALSE;
	}
#ifdef ZERO_PKT_LOSS_SUPPORT
	pDfsParam->ZeroLossRadarDetect = FALSE;
#endif
	pDfsParam->bNoSwitchCh = FALSE;
	pDfsParam->bZeroWaitCacSecondHandle = FALSE;
	pDfsParam->bDedicatedZeroWaitSupport = FALSE;
	pDfsParam->ZeroWaitBandidx = 1;
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	pDfsParam->bV10ChannelListValid = FALSE;
	pDfsParam->bV10BootACSValid = FALSE;
	pDfsParam->gV10OffChnlWaitTime = 0;
	pDfsParam->bV10W56APDownEnbl = FALSE;
	pDfsParam->bV10APBcnUpdateEnbl =  FALSE;
	pDfsParam->bV10W56GrpValid = FALSE;
	pDfsParam->bV10APInterfaceDownEnbl = FALSE;
	pDfsParam->bV10W56SwitchVHT80 = FALSE;
#endif
	pDfsParam->DfsNopExpireSetChPolicy = DFS_NOP_EXPIRE_SET_CH_POLICY_RESTORE;
	pDfsParam->bDfsDedicatedRxPreselectCh = DFS_PRE_SELECT_CH_ENABLE;
	pDfsParam->OutBandCh = 0;
	pDfsParam->OutBandBw = 0;
	pDfsParam->bZeroWaitSupport = 0;
	pDfsParam->bOutBandAvailable = FALSE;
	pDfsParam->DedicatedOutBandCacCount = 0;
	pDfsParam->bSetInBandCacReStart = FALSE;
	pDfsParam->bDedicatedZeroWaitDefault = FALSE;
	pDfsParam->bInitOutBandBranch = FALSE;
	pDfsParam->RadarHitReport = FALSE;
	pDfsParam->OutBandAvailableCh = 0;
	pDfsParam->targetCh = 0;
	pDfsParam->targetBw = 0;
	pDfsParam->targetCacValue = 0;
	pDfsParam->DfsChSelPrefer = 0;
	pDfsParam->ByPassCac = FALSE;
	pDfsParam->CacCtrl = CAC_INIT;
	pDfsParam->bBootUpInBandUnavailableCh = FALSE;
	pDfsParam->SetChByCmd = FALSE;
	pDfsParam->SetOutBandChStat = OUTB_SET_CH_DEFAULT;
#ifdef DFS_SDB_SUPPORT
	pDfsParam->bDfsSdbEnable = FALSE;
#endif /* DFS_SDB_SUPPORT */

	/* Threshold parameters*/
	radar_thrshld_param = &pAd->CommonCfg.DfsParameter.radar_thrshld_param;
	pls_thrshld_param = &radar_thrshld_param->pls_thrshld_param;

	pls_thrshld_param->pls_width_max = 110; /* unit: us */
	pls_thrshld_param->pls_pwr_max = -10; /* unit: dBm */
	pls_thrshld_param->pls_pwr_min = -80; /* unit: dBm */

	pls_thrshld_param->pri_min_stgr = 40; /* unit: us */
	pls_thrshld_param->pri_max_stgr = 5200; /* unit: us */
	pls_thrshld_param->pri_min_cr = 128; /* unit: us */
	pls_thrshld_param->pri_max_cr = 5200; /* unit: us */

	pDfsParam->fcc_lpn_min = 8;

	pDfsParam->is_hw_rdd_log_en = FALSE;
	pDfsParam->is_sw_rdd_log_en = FALSE;
	pDfsParam->sw_rdd_log_cond = TRUE;
	pDfsParam->is_radar_emu = FALSE;

	/* FCC-1/JP-1 */
	radar_thrshld_param->sw_radar_type[0].rt_det = 0;
	radar_thrshld_param->sw_radar_type[0].rt_en = 1;
	radar_thrshld_param->sw_radar_type[0].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[0].rt_crpn_min = 8;
	radar_thrshld_param->sw_radar_type[0].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[0].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[0].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[0].rt_pw_max = 13;
	radar_thrshld_param->sw_radar_type[0].rt_pri_min = 508;
	radar_thrshld_param->sw_radar_type[0].rt_pri_max = 3076;
	radar_thrshld_param->sw_radar_type[0].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[0].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[0].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[0].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[0].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[0].rt_stg_pri_diff_min = 0;
	/* FCC-2 */
	radar_thrshld_param->sw_radar_type[1].rt_det = 0;
	radar_thrshld_param->sw_radar_type[1].rt_en = 1;
	radar_thrshld_param->sw_radar_type[1].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[1].rt_crpn_min = 12;
	radar_thrshld_param->sw_radar_type[1].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[1].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[1].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[1].rt_pw_max = 17;
	radar_thrshld_param->sw_radar_type[1].rt_pri_min = 140;
	radar_thrshld_param->sw_radar_type[1].rt_pri_max = 240;
	radar_thrshld_param->sw_radar_type[1].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[1].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[1].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[1].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[1].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[1].rt_stg_pri_diff_min = 0;
	/* FCC-3 */
	radar_thrshld_param->sw_radar_type[2].rt_det = 0;
	radar_thrshld_param->sw_radar_type[2].rt_en = 1;
	radar_thrshld_param->sw_radar_type[2].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[2].rt_crpn_min = 8;
	radar_thrshld_param->sw_radar_type[2].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[2].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[2].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[2].rt_pw_max = 22;
	radar_thrshld_param->sw_radar_type[2].rt_pri_min = 190;
	radar_thrshld_param->sw_radar_type[2].rt_pri_max = 510;
	radar_thrshld_param->sw_radar_type[2].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[2].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[2].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[2].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[2].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[2].rt_stg_pri_diff_min = 0;
	/* FCC-4 */
	radar_thrshld_param->sw_radar_type[3].rt_det = 0;
	radar_thrshld_param->sw_radar_type[3].rt_en = 1;
	radar_thrshld_param->sw_radar_type[3].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[3].rt_crpn_min = 6;
	radar_thrshld_param->sw_radar_type[3].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[3].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[3].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[3].rt_pw_max = 32;
	radar_thrshld_param->sw_radar_type[3].rt_pri_min = 190;
	radar_thrshld_param->sw_radar_type[3].rt_pri_max = 510;
	radar_thrshld_param->sw_radar_type[3].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[3].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[3].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[3].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[3].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[3].rt_stg_pri_diff_min = 0;
	/* FCC-6 */
	radar_thrshld_param->sw_radar_type[4].rt_det = 0;
	radar_thrshld_param->sw_radar_type[4].rt_en = 1;
	radar_thrshld_param->sw_radar_type[4].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[4].rt_crpn_min = 9;
	radar_thrshld_param->sw_radar_type[4].rt_crpn_max = 255;
	radar_thrshld_param->sw_radar_type[4].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[4].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[4].rt_pw_max = 13;
	radar_thrshld_param->sw_radar_type[4].rt_pri_min = 323;
	radar_thrshld_param->sw_radar_type[4].rt_pri_max = 343;
	radar_thrshld_param->sw_radar_type[4].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[4].rt_crbn_max = 32;
	radar_thrshld_param->sw_radar_type[4].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[4].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[4].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[4].rt_stg_pri_diff_min = 0;
	/* ETSI-1 */
	radar_thrshld_param->sw_radar_type[5].rt_det = 0;
	radar_thrshld_param->sw_radar_type[5].rt_en = 1;
	radar_thrshld_param->sw_radar_type[5].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[5].rt_crpn_min = 6;
	radar_thrshld_param->sw_radar_type[5].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[5].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[5].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[5].rt_pw_max = 17;
	radar_thrshld_param->sw_radar_type[5].rt_pri_min = 990;
	radar_thrshld_param->sw_radar_type[5].rt_pri_max = 5010;
	radar_thrshld_param->sw_radar_type[5].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[5].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[5].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[5].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[5].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[5].rt_stg_pri_diff_min = 0;
	/* ETSI-2 */
	radar_thrshld_param->sw_radar_type[6].rt_det = 0;
	radar_thrshld_param->sw_radar_type[6].rt_en = 1;
	radar_thrshld_param->sw_radar_type[6].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[6].rt_crpn_min = 9;
	radar_thrshld_param->sw_radar_type[6].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[6].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[6].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[6].rt_pw_max = 27;
	radar_thrshld_param->sw_radar_type[6].rt_pri_min = 615;
	radar_thrshld_param->sw_radar_type[6].rt_pri_max = 5010;
	radar_thrshld_param->sw_radar_type[6].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[6].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[6].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[6].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[6].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[6].rt_stg_pri_diff_min = 0;
	/* ETSI-3 */
	radar_thrshld_param->sw_radar_type[7].rt_det = 0;
	radar_thrshld_param->sw_radar_type[7].rt_en = 1;
	radar_thrshld_param->sw_radar_type[7].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[7].rt_crpn_min = 8;
	radar_thrshld_param->sw_radar_type[7].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[7].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[7].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[7].rt_pw_max = 27;
	radar_thrshld_param->sw_radar_type[7].rt_pri_min = 245;
	radar_thrshld_param->sw_radar_type[7].rt_pri_max = 445;
	radar_thrshld_param->sw_radar_type[7].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[7].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[7].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[7].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[7].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[7].rt_stg_pri_diff_min = 0;
	/* ETSI-4 */
	radar_thrshld_param->sw_radar_type[8].rt_det = 0;
	radar_thrshld_param->sw_radar_type[8].rt_en = 1;
	radar_thrshld_param->sw_radar_type[8].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[8].rt_crpn_min = 6;
	radar_thrshld_param->sw_radar_type[8].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[8].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[8].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[8].rt_pw_max = 42;
	radar_thrshld_param->sw_radar_type[8].rt_pri_min = 245;
	radar_thrshld_param->sw_radar_type[8].rt_pri_max = 510;
	radar_thrshld_param->sw_radar_type[8].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[8].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[8].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[8].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[8].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[8].rt_stg_pri_diff_min = 0;
	/* ETSI-5, 2PRI */
	radar_thrshld_param->sw_radar_type[9].rt_det = 0;
	radar_thrshld_param->sw_radar_type[9].rt_en = 1;
	radar_thrshld_param->sw_radar_type[9].rt_stgr = 1;
	radar_thrshld_param->sw_radar_type[9].rt_crpn_min = 0;
	radar_thrshld_param->sw_radar_type[9].rt_crpn_max = 0;
	radar_thrshld_param->sw_radar_type[9].rt_crpr_min = 0;
	radar_thrshld_param->sw_radar_type[9].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[9].rt_pw_max = 14;
	radar_thrshld_param->sw_radar_type[9].rt_pri_min = 2490;
	radar_thrshld_param->sw_radar_type[9].rt_pri_max = 3343;
	radar_thrshld_param->sw_radar_type[9].rt_crbn_min = 0;
	radar_thrshld_param->sw_radar_type[9].rt_crbn_max = 0;
	radar_thrshld_param->sw_radar_type[9].rt_stg_pn_min = 12;
	radar_thrshld_param->sw_radar_type[9].rt_stg_pn_max = 32;
	radar_thrshld_param->sw_radar_type[9].rt_stg_pr_min = 28;
	radar_thrshld_param->sw_radar_type[9].rt_stg_pri_diff_min = (131 - 5);
	/* ETSI-5, 3PRI */
	radar_thrshld_param->sw_radar_type[10].rt_det = 0;
	radar_thrshld_param->sw_radar_type[10].rt_en = 1;
	radar_thrshld_param->sw_radar_type[10].rt_stgr = 1;
	radar_thrshld_param->sw_radar_type[10].rt_crpn_min = 0;
	radar_thrshld_param->sw_radar_type[10].rt_crpn_max = 0;
	radar_thrshld_param->sw_radar_type[10].rt_crpr_min = 0;
	radar_thrshld_param->sw_radar_type[10].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[10].rt_pw_max = 14;
	radar_thrshld_param->sw_radar_type[10].rt_pri_min = 2490;
	radar_thrshld_param->sw_radar_type[10].rt_pri_max = 3343;
	radar_thrshld_param->sw_radar_type[10].rt_crbn_min = 0;
	radar_thrshld_param->sw_radar_type[10].rt_crbn_max = 0;
	radar_thrshld_param->sw_radar_type[10].rt_stg_pn_min = 15;
	radar_thrshld_param->sw_radar_type[10].rt_stg_pn_max = 32;
	radar_thrshld_param->sw_radar_type[10].rt_stg_pr_min = 24;
	radar_thrshld_param->sw_radar_type[10].rt_stg_pri_diff_min = (131 - 5);
	/* ETSI-6, 2PRI */
	radar_thrshld_param->sw_radar_type[11].rt_det = 0;
	radar_thrshld_param->sw_radar_type[11].rt_en = 1;
	radar_thrshld_param->sw_radar_type[11].rt_stgr = 1;
	radar_thrshld_param->sw_radar_type[11].rt_crpn_min = 0;
	radar_thrshld_param->sw_radar_type[11].rt_crpn_max = 0;
	radar_thrshld_param->sw_radar_type[11].rt_crpr_min = 0;
	radar_thrshld_param->sw_radar_type[11].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[11].rt_pw_max = 14;
	radar_thrshld_param->sw_radar_type[11].rt_pri_min = 823;
	radar_thrshld_param->sw_radar_type[11].rt_pri_max = 2510;
	radar_thrshld_param->sw_radar_type[11].rt_crbn_min = 0;
	radar_thrshld_param->sw_radar_type[11].rt_crbn_max = 0;
	radar_thrshld_param->sw_radar_type[11].rt_stg_pn_min = 18;
	radar_thrshld_param->sw_radar_type[11].rt_stg_pn_max = 32;
	radar_thrshld_param->sw_radar_type[11].rt_stg_pr_min = 28;
	radar_thrshld_param->sw_radar_type[11].rt_stg_pri_diff_min = (59 - 5);
	/*ETSI-6, 3PRI */
	radar_thrshld_param->sw_radar_type[12].rt_det = 0;
	radar_thrshld_param->sw_radar_type[12].rt_en = 1;
	radar_thrshld_param->sw_radar_type[12].rt_stgr = 1;
	radar_thrshld_param->sw_radar_type[12].rt_crpn_min = 0;
	radar_thrshld_param->sw_radar_type[12].rt_crpn_max = 0;
	radar_thrshld_param->sw_radar_type[12].rt_crpr_min = 0;
	radar_thrshld_param->sw_radar_type[12].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[12].rt_pw_max = 14;
	radar_thrshld_param->sw_radar_type[12].rt_pri_min = 823;
	radar_thrshld_param->sw_radar_type[12].rt_pri_max = 2510;
	radar_thrshld_param->sw_radar_type[12].rt_crbn_min = 0;
	radar_thrshld_param->sw_radar_type[12].rt_crbn_max = 0;
	radar_thrshld_param->sw_radar_type[12].rt_stg_pn_min = 27;
	radar_thrshld_param->sw_radar_type[12].rt_stg_pn_max = 32;
	radar_thrshld_param->sw_radar_type[12].rt_stg_pr_min = 24;
	radar_thrshld_param->sw_radar_type[12].rt_stg_pri_diff_min = (59 - 5);
	/* ETSI-7 Hopping 1 */
	radar_thrshld_param->sw_radar_type[13].rt_det = 0;
	radar_thrshld_param->sw_radar_type[13].rt_en = 1;
	radar_thrshld_param->sw_radar_type[13].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[13].rt_crpn_min = 6;
	radar_thrshld_param->sw_radar_type[13].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[13].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[13].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[13].rt_pw_max = 5;
	radar_thrshld_param->sw_radar_type[13].rt_pri_min = 300;
	radar_thrshld_param->sw_radar_type[13].rt_pri_max = 360;
	radar_thrshld_param->sw_radar_type[13].rt_crbn_min = 0;
	radar_thrshld_param->sw_radar_type[13].rt_crbn_max = 5;
	radar_thrshld_param->sw_radar_type[13].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[13].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[13].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[13].rt_stg_pri_diff_min = 0;
	/* ETSI-8 Hopping 2 */
	radar_thrshld_param->sw_radar_type[14].rt_det = 0;
	radar_thrshld_param->sw_radar_type[14].rt_en = 1;
	radar_thrshld_param->sw_radar_type[14].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[14].rt_crpn_min = 6;
	radar_thrshld_param->sw_radar_type[14].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[14].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[14].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[14].rt_pw_max = 42;
	radar_thrshld_param->sw_radar_type[14].rt_pri_min = 190;
	radar_thrshld_param->sw_radar_type[14].rt_pri_max = 250;
	radar_thrshld_param->sw_radar_type[14].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[14].rt_crbn_max = 3;
	radar_thrshld_param->sw_radar_type[14].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[14].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[14].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[14].rt_stg_pri_diff_min = 0;
	/* JP-2 */
	radar_thrshld_param->sw_radar_type[15].rt_det = 0;
	radar_thrshld_param->sw_radar_type[15].rt_en = 1;
	radar_thrshld_param->sw_radar_type[15].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[15].rt_crpn_min = 7;
	radar_thrshld_param->sw_radar_type[15].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[15].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[15].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[15].rt_pw_max = 14;
	radar_thrshld_param->sw_radar_type[15].rt_pri_min = 3836;
	radar_thrshld_param->sw_radar_type[15].rt_pri_max = 3856;
	radar_thrshld_param->sw_radar_type[15].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[15].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[15].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[15].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[15].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[15].rt_stg_pri_diff_min = 0;
	/* New JP radar, JP_3 */
	radar_thrshld_param->sw_radar_type[16].rt_det = 0;
	radar_thrshld_param->sw_radar_type[16].rt_en = 1;
	radar_thrshld_param->sw_radar_type[16].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[16].rt_crpn_min = 6;
	radar_thrshld_param->sw_radar_type[16].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[16].rt_crpr_min = 22;
	radar_thrshld_param->sw_radar_type[16].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[16].rt_pw_max = 110;
	radar_thrshld_param->sw_radar_type[16].rt_pri_min = 615;
	radar_thrshld_param->sw_radar_type[16].rt_pri_max = 5010;
	radar_thrshld_param->sw_radar_type[16].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[16].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[16].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[16].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[16].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[16].rt_stg_pri_diff_min = 0;
	/* New JP radar, JP_4 */
	radar_thrshld_param->sw_radar_type[17].rt_det = 0;
	radar_thrshld_param->sw_radar_type[17].rt_en = 1;
	radar_thrshld_param->sw_radar_type[17].rt_stgr = 1;
	radar_thrshld_param->sw_radar_type[17].rt_crpn_min = 0;
	radar_thrshld_param->sw_radar_type[17].rt_crpn_max = 0;
	radar_thrshld_param->sw_radar_type[17].rt_crpr_min = 0;
	radar_thrshld_param->sw_radar_type[17].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[17].rt_pw_max = 110;
	radar_thrshld_param->sw_radar_type[17].rt_pri_min = 15;
	radar_thrshld_param->sw_radar_type[17].rt_pri_max = 5010;
	radar_thrshld_param->sw_radar_type[17].rt_crbn_min = 0;
	radar_thrshld_param->sw_radar_type[17].rt_crbn_max = 0;
	radar_thrshld_param->sw_radar_type[17].rt_stg_pn_min = 12;
	radar_thrshld_param->sw_radar_type[17].rt_stg_pn_max = 32;
	radar_thrshld_param->sw_radar_type[17].rt_stg_pr_min = 28;
	radar_thrshld_param->sw_radar_type[17].rt_stg_pri_diff_min = 0;
	/* KR-1 */
	radar_thrshld_param->sw_radar_type[18].rt_det = 0;
	radar_thrshld_param->sw_radar_type[18].rt_en = 1;
	radar_thrshld_param->sw_radar_type[18].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[18].rt_crpn_min = 8;
	radar_thrshld_param->sw_radar_type[18].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[18].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[18].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[18].rt_pw_max = 14;
	radar_thrshld_param->sw_radar_type[18].rt_pri_min = 1419;
	radar_thrshld_param->sw_radar_type[18].rt_pri_max = 1439;
	radar_thrshld_param->sw_radar_type[18].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[18].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[18].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[18].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[18].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[18].rt_stg_pri_diff_min = 0;
	/* KR-2 */
	radar_thrshld_param->sw_radar_type[19].rt_det = 0;
	radar_thrshld_param->sw_radar_type[19].rt_en = 1;
	radar_thrshld_param->sw_radar_type[19].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[19].rt_crpn_min = 4;
	radar_thrshld_param->sw_radar_type[19].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[19].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[19].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[19].rt_pw_max = 14;
	radar_thrshld_param->sw_radar_type[19].rt_pri_min = 546;
	radar_thrshld_param->sw_radar_type[19].rt_pri_max = 566;
	radar_thrshld_param->sw_radar_type[19].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[19].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[19].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[19].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[19].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[19].rt_stg_pri_diff_min = 0;
	/* KR-3 */
	radar_thrshld_param->sw_radar_type[20].rt_det = 0;
	radar_thrshld_param->sw_radar_type[20].rt_en = 1;
	radar_thrshld_param->sw_radar_type[20].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[20].rt_crpn_min = 9;
	radar_thrshld_param->sw_radar_type[20].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[20].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[20].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[20].rt_pw_max = 14;
	radar_thrshld_param->sw_radar_type[20].rt_pri_min = 3020;
	radar_thrshld_param->sw_radar_type[20].rt_pri_max = 3040;
	radar_thrshld_param->sw_radar_type[20].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[20].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[20].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[20].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[20].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[20].rt_stg_pri_diff_min = 0;
	/* BAND4-12345 */
	radar_thrshld_param->sw_radar_type[21].rt_det = 0;
	radar_thrshld_param->sw_radar_type[21].rt_en = 1;
	radar_thrshld_param->sw_radar_type[21].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[21].rt_crpn_min = 6;
	radar_thrshld_param->sw_radar_type[21].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[21].rt_crpr_min = 16;
	radar_thrshld_param->sw_radar_type[21].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[21].rt_pw_max = 27;
	radar_thrshld_param->sw_radar_type[21].rt_pri_min = 190;
	radar_thrshld_param->sw_radar_type[21].rt_pri_max = 5010;
	radar_thrshld_param->sw_radar_type[21].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[21].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[21].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[21].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[21].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[21].rt_stg_pri_diff_min = 0;
	/* BAND4-6 */
	radar_thrshld_param->sw_radar_type[22].rt_det = 0;
	radar_thrshld_param->sw_radar_type[22].rt_en = 1;
	radar_thrshld_param->sw_radar_type[22].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[22].rt_crpn_min = 6;
	radar_thrshld_param->sw_radar_type[22].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[22].rt_crpr_min = 16;
	radar_thrshld_param->sw_radar_type[22].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[22].rt_pw_max = 42;
	radar_thrshld_param->sw_radar_type[22].rt_pri_min = 245;
	radar_thrshld_param->sw_radar_type[22].rt_pri_max = 4000;
	radar_thrshld_param->sw_radar_type[22].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[22].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[22].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[22].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[22].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[22].rt_stg_pri_diff_min = 0;

	pAd->Dot11_H.DfsZeroWaitChMovingTime = 3;
	pDfsParam->bNoAvailableCh = FALSE;
	pDfsParam->bDfsWaitCfgChNop = FALSE;
	pDfsParam->bDfsRestoreCfgChAfterNopEnd = FALSE;
	pDfsParam->band_ch = 0;
	pDfsParam->band_orich = 0;
	pDfsParam->BW160ZeroWaitState = DFS_NOT_ENABLE;
	pDfsParam->BW160ZeroWaitSupport = 0;
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	if ((pDfsParam->NeedSetNewChList == DFS_SET_NEWCH_INIT)
		|| (pChCtrl->ChListNum == 0))
		pDfsParam->NeedSetNewChList = DFS_SET_NEWCH_ENABLED;
	else
		pDfsParam->NeedSetNewChList = DFS_SET_NEWCH_DISABLED;

#ifdef CONFIG_RCSA_SUPPORT
	pDfsParam->fSendRCSA = FALSE;
	pDfsParam->ChSwMode = 1;
#endif

	pDfsParam->TriggerEventIntvl = 0;
	pDfsParam->ucDisTm = 0;

	DfsStateMachineInit(pAd, &pAd->CommonCfg.DfsParameter.DfsStatMachine, pAd->CommonCfg.DfsParameter.DfsStateFunc);
}

VOID DfsStateMachineInit(
	IN RTMP_ADAPTER * pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, DFS_MAX_STATE, DFS_MAX_MSG, (STATE_MACHINE_FUNC)Drop, DFS_BEFORE_SWITCH, DFS_MACHINE_BASE);
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_CAC_END, (STATE_MACHINE_FUNC)DfsCacEndUpdate);
#if ((DFS_ZEROWAIT_DEFAULT_FLOW == 1) && defined(BACKGROUND_SCAN_SUPPORT))
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_OFF_CAC_END, (STATE_MACHINE_FUNC)dfs_off_cac_end_update);
#endif
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_V10_W56_APDOWN_ENBL, (STATE_MACHINE_FUNC)DfsV10W56APDownEnbl);
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_V10_W56_APDOWN_FINISH,
		(STATE_MACHINE_FUNC)DfsV10W56APDownPass);
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_V10_ACS_CSA_UPDATE, (STATE_MACHINE_FUNC)DfsV10APBcnUpdate);
#endif
}

INT Set_RadarDetectMode_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	UCHAR value, ret;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
#ifdef CONFIG_ATE
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
	struct DOT11_H *dot11h = NULL;
#endif /* CONFIG_ATE */

	if (!wdev)
		return FALSE;

#ifdef CONFIG_ATE
	if (!ATE_ON(pAd)) {
		UINT8 rx_stream = 1;
		UINT8 tx_stream = 1;
		MTWF_PRINT("normal mode - set to new T/RX\n");

		/* Set to 1 TRX stream when detection mode is set */
		wlan_config_set_rx_stream(wdev, rx_stream);
		wlan_config_set_tx_stream(wdev, tx_stream);

		wlan_operate_set_rx_stream(wdev, rx_stream);
		wlan_operate_set_tx_stream(wdev, tx_stream);

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			dot11h = wdev->pDot11_H;
			if (!dot11h)
				return FALSE;

			dot11h->ChannelMode = CHAN_SWITCHING_MODE;

			APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
			APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
		}
#endif /* CONFIG_AP_SUPPORT */
	}
#endif /* CONFIG_ATE */

	value = os_str_tol(arg, 0, 10);

	if (value >= RDD_DETMODE_NUM) {
		MTWF_PRINT("In Set_RadarDetectMode_Proc, invalid mode: %d\n", value);
	} else {
		MTWF_PRINT("In Set_RadarDetectMode_Proc, mode: %d\n", value);
		ret = mtRddControl(pAd, RDD_DET_MODE, 0, 0, value);
	}

	switch (value) {
	case RDD_DETMODE_OFF: /* Turn OFF detection mode */
		pDfsParam->bNoSwitchCh = FALSE;
		break;
	case RDD_DETMODE_ON: /* Turn ON detection mode */
	case RDD_DETMODE_DEBUG: /* Turn ON detection/debug mode */
		pDfsParam->bNoSwitchCh = TRUE;
		break;
	default:
		pDfsParam->bNoSwitchCh = FALSE;
		break;
	}

	return TRUE;
}

INT Set_RadarDetectStart_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	ULONG value, ret1, ret2;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	struct freq_oper oper;
	UCHAR phy_bw;
	UCHAR rd_region = 0; /* Region of radar detection */
	value = os_str_tol(arg, 0, 10);
	if (hc_radio_query(pAd, &oper) != HC_STATUS_OK) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "Cannot get radio info!\n");
		return FALSE;
	}
	phy_bw = oper.bw;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "In Set_RadarDetectStart_Proc:\n");
	rd_region = pAd->CommonCfg.RDDurRegion;

	if (value == 0) {
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD0, 0, 0);
		ret1 = mtRddControl(pAd, RDD_START, HW_RDD0, RXSEL_0, rd_region);
		ret1 = mtRddControl(pAd, RDD_DET_MODE, HW_RDD0, 0, RDD_DETMODE_ON);
		pDfsParam->bNoSwitchCh = TRUE;
	} else if (value == 1) {
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
		ret1 = mtRddControl(pAd, RDD_START, HW_RDD1, RXSEL_0, rd_region);
		ret1 = mtRddControl(pAd, RDD_DET_MODE, HW_RDD1, 0, RDD_DETMODE_ON);
		pDfsParam->bNoSwitchCh = TRUE;
	} else if (value == 2) {
#ifdef DOT11_VHT_AC
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD0, 0, 0);
		ret1 = mtRddControl(pAd, RDD_START, HW_RDD0, RXSEL_0, rd_region);
		ret1 = mtRddControl(pAd, RDD_DET_MODE, HW_RDD0, 0, RDD_DETMODE_ON);

		if (phy_bw == BW_160) {
			ret2 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
			ret2 = mtRddControl(pAd, RDD_START, HW_RDD1, RXSEL_0, rd_region);
			ret2 = mtRddControl(pAd, RDD_DET_MODE, HW_RDD1, 0, RDD_DETMODE_ON);
		} else
#endif
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "In Set_RadarDetectStart_Proc: Bandwidth not 80+80 or 160\n");

		pDfsParam->bNoSwitchCh = TRUE;
	} else
		;

	return TRUE;
}


INT Set_RadarDetectStop_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	ULONG value, ret1, ret2;
	struct freq_oper oper;
	UCHAR phy_bw;
	if (hc_radio_query(pAd, &oper) != HC_STATUS_OK)
		return FALSE;

	phy_bw = oper.bw;
	value = os_str_tol(arg, 0, 10);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "In Set_RadarDetectStop_Proc:\n");

	if (value == 0)
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD0, 0, 0);
	else if (value == 1)
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
	else if (value == 2) {
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD0, 0, 0);
#ifdef DOT11_VHT_AC

		if (phy_bw == BW_160)
			ret2 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
		else
#endif
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "In Set_RadarDetectStop_Proc: Bandwidth not 80+80 or 160\n");
	} else
		;

	return TRUE;
}

INT Set_ByPassCac_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR value; /* CAC time */
	value = os_str_tol(arg, 0, 10);

	MTWF_PRINT("%s(): set CAC value to %d\n", __func__, value);
	if ((pAd->Dot11_H.ChannelMode == CHAN_SILENCE_MODE)
		|| (pDfsParam->BW160ZeroWaitSupport == TRUE))
		pAd->Dot11_H.RDCount = pAd->Dot11_H.cac_time;


	pDfsParam->DedicatedOutBandCacCount = pDfsParam->DedicatedOutBandCacTime;
	return TRUE;
}

INT Set_RDDReport_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	UCHAR value;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR if_idx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	value = os_str_tol(arg, 0, 10);
	if (wdev == NULL)
		return FALSE;
	if (value >= HW_RDD_NUM) {
		MTWF_PRINT("Invalid parameter, please input the correct band index!\n");
		return FALSE;
	}

	if (wdev->if_dev == NULL ||
		(wdev->if_dev != NULL && !RTMP_OS_NETDEV_STATE_RUNNING(wdev->if_dev))) {
		/* the interface is down*/
		return false;
	}

	if (!pAd->CommonCfg.DfsParameter.bDfsEnable) {
		MTWF_PRINT("[%s][RDM]: The Radar detection does not Enable!!!\n", __func__);
		return FALSE;
	}
	if (value == RDD_INBAND_IDX_1 || value == RDD_INBAND_IDX_2) {
		if (!RadarChannelCheck(pAd, wdev->channel)) {
			MTWF_PRINT("[%s]wdev is not working in radar channel\n", __func__);
			return FALSE;
		}
	}
#if (RDD_2_SUPPORTED == 1)
	if (value == RDD_DEDICATED_IDX) {
		if (!RadarChannelCheck(pAd, pAd->CommonCfg.DfsParameter.OutBandCh) &&
			!(IS_CH_BETWEEN(pAd->CommonCfg.DfsParameter.OutBandCh, 36, 48) && pAd->CommonCfg.DfsParameter.OutBandBw == BW_160)) {
			MTWF_PRINT("[%s]Out-band is not working in radar channel\n", __func__);
			return FALSE;
		}
	}
#endif  /* RDD_2_SUPPORTED */

	pAd->CommonCfg.DfsParameter.is_radar_emu = TRUE;

	mtRddControl(pAd, RDD_RADAR_EMULATE, value, 0, 0);
	return TRUE;
}

/**
* Trigger_RDD_Event - Trigger RDD related event.
* @pAd: pointer of the RTMP_ADAPTER
* @arg: event type (0: not send event, 1: radar detect; 2: CAC timeout; 3: CSA done.)
*
* This function is for feature debug
*
**/
INT Trigger_RDD_Event(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	UINT32 interval;

	if (arg == NULL || strlen(arg) == 0)
		return FALSE;

	interval = (UINT32) os_str_tol(arg, 0, 10);
	pAd->CommonCfg.DfsParameter.TriggerEventIntvl = interval;
	if (interval == 0) {
		MTWF_PRINT("Shut down RDD event trigger.\n");
	} else {
		MTWF_PRINT("Trigger RDD event per %d msecs.\n", interval);
	}

	return TRUE;
}

/**
* MakeUpRDDEvent - Make up radar detected event.
* @pAd: pointer of the RTMP_ADAPTER
*
* This function is for feature debug
*
**/
VOID MakeUpRDDEvent(RTMP_ADAPTER *pAd)
{
	UINT i;
	struct wifi_dev *wdev;
	BOOLEAN found_dfs_chn = FALSE;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];
		if (wdev != NULL) {
			if (wdev->if_up_down_state == TRUE) {
				if (RadarChannelCheck(pAd, wdev->channel)) {
					found_dfs_chn = TRUE;
					break;
				}
			}
		}
	}

	if (found_dfs_chn) {
		pAd->CommonCfg.DfsParameter.is_radar_emu = TRUE;
		mtRddControl(pAd, RDD_RADAR_EMULATE, HcGetBandByWdev(wdev), 0, 0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "No any wdev is not working in radar channel! \n");
	}
}

INT Set_DfsChannelShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	UCHAR value;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	value = os_str_tol(arg, 0, 10);

	MTWF_PRINT("Current 5G channel, Band0Ch: %d, Band1Ch: %d\n",
				 pDfsParam->OutBandCh, pDfsParam->band_ch);
	return TRUE;
}

INT Set_DfsBwShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	ULONG value;
	UCHAR band_idx = DBDC_BAND0;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "wdev is NULL\n");
		return FALSE;
	}

	band_idx = HcGetBandByWdev(wdev);
	value = os_str_tol(arg, 0, 10);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Current Normal RX DFS Bw is %d\n", pDfsParam->band_bw);
	return TRUE;
}

#ifdef CONFIG_AP_SUPPORT
INT Set_DfsRDModeShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	ULONG value;
	UCHAR BssIdx;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdevEach = NULL;
	struct DOT11_H *pDot11hEach = NULL;

	value = os_str_tol(arg, 0, 10);

	MTWF_PRINT("pAd->ChannelMode=%d\n", pAd->Dot11_H.ChannelMode);

	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		MTWF_PRINT("BssIdx: %d\n", BssIdx);
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdevEach = &pMbss->wdev;
		if (pMbss == NULL || wdevEach == NULL)
			continue;
		MTWF_PRINT("wdevIdx: %d. BandIdx: %d, channel: %d\n", wdevEach->wdev_idx, HcGetBandByWdev(wdevEach), wdevEach->channel);
		pDot11hEach = wdevEach->pDot11_H;
		if (pDot11hEach == NULL)
			continue;
		MTWF_PRINT("ChannelMode: %d\n\n", pDot11hEach->ChannelMode);
	}
	return TRUE;
}
#endif

INT Set_DfsRDDRegionShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	ULONG value;
	value = os_str_tol(arg, 0, 10);
	MTWF_PRINT("RDD Region is %d\n", pAd->CommonCfg.RDDurRegion);
	return TRUE;
}

INT Show_DfsNonOccupancy_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	UINT_8 ch_idx, band_idx;
	PCHANNEL_CTRL pChCtrl = NULL;

	MTWF_PRINT("[%s][RDM]:\n", __func__);

	for (band_idx = 0; band_idx < CFG_WIFI_RAM_BAND_NUM; band_idx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
		MTWF_PRINT("band_idx: %d\n", band_idx);

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			MTWF_PRINT("DfsChannelList[%d].Channel = %d, NonOccupancy = %d, ",
			ch_idx,
			pChCtrl->ChList[ch_idx].Channel,
			pChCtrl->ChList[ch_idx].NonOccupancy);

			MTWF_PRINT("NOPClrCnt = %d, NOPSetByBw = %d, NOPSaveForClear is %d, ",
			pChCtrl->ChList[ch_idx].NOPClrCnt,
			pChCtrl->ChList[ch_idx].NOPSetByBw,
			pChCtrl->ChList[ch_idx].NOPSaveForClear);

			MTWF_PRINT("SupportBwBitMap is %d, IsOutBandCacDone is %d",
			pChCtrl->ChList[ch_idx].SupportBwBitMap,
			pChCtrl->ChList[ch_idx].IsOutBandCacDone);

			MTWF_PRINT(", NOPSaveForRestore is %d, Priority is %d\n",
			pChCtrl->ChList[ch_idx].NOPSaveForRestore, pChCtrl->ChList[ch_idx].Priority);
		}
	}
	return TRUE;
}

INT Nop_List_Backup(
	IN PRTMP_ADAPTER pAd)
{
	UINT_8 ch_idx, band_idx, count = 0;
	PCHANNEL_CTRL pChCtrl = NULL;
	NOP_LIST *pNopList = NULL;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "---->\n");
	os_alloc_mem(pAd, (UCHAR **)&pNopList, sizeof(NOP_LIST));
	if (pNopList) {
		os_zero_mem(pNopList, sizeof(NOP_LIST));
		for (band_idx = 0; band_idx < CFG_WIFI_RAM_BAND_NUM; band_idx++) {
			pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
			for (ch_idx = 0; (ch_idx < pChCtrl->ChListNum) && (count < DFS_AVAILABLE_LIST_CH_NUM); ch_idx++) {
				if (pChCtrl->ChList[ch_idx].NonOccupancy != 0 || pChCtrl->ChList[ch_idx].NOPSaveForClear != 0) {
					pNopList->DfsChList[count].Channel = pChCtrl->ChList[ch_idx].Channel;
					pNopList->DfsChList[count].NonOccupancy = pChCtrl->ChList[ch_idx].NonOccupancy;
					pNopList->DfsChList[count].NOPClrCnt = pChCtrl->ChList[ch_idx].NOPClrCnt;
					pNopList->DfsChList[count].NOPSetByBw = pChCtrl->ChList[ch_idx].NOPSetByBw;
					pNopList->DfsChList[count].NOPSaveForClear = pChCtrl->ChList[ch_idx].NOPSaveForClear;
					pNopList->DfsChList[count].SupportBwBitMap = pChCtrl->ChList[ch_idx].SupportBwBitMap;
					pNopList->DfsChList[count].IsOutBandCacDone = pChCtrl->ChList[ch_idx].IsOutBandCacDone;
					pNopList->DfsChList[count].NOPSaveForRestore = pChCtrl->ChList[ch_idx].NOPSaveForRestore;
					pNopList->DfsChList[count].Priority = pChCtrl->ChList[ch_idx].Priority;
					count++;
				}
			}
		}
	}

	if (pAd->NopListBk) {
		/* free earlier NOP list memory if any*/
		os_free_mem(pAd->NopListBk);
		pAd->NopListBk = NULL;
	}

	if (count == 0) {
		if (pNopList)
			os_free_mem(pNopList);
	} else {
		pNopList->ChListNum = count;
		pAd->NopListBk = (VOID *)pNopList;
	}

	MTWF_PRINT("[%s] NopList Channel count:%d <----\n", __func__, count);

	return 0;
}

INT show_dfs_ch_info_proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
	struct DOT11_H *pDot11h = &pAd->Dot11_H;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	struct Chan_Config *pChCfg = NULL;

	if (!pChCtrl) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "can't find pChCtrl !\n");
		return FALSE;
	}
	pChCfg = &pChCtrl->ch_cfg;

	MTWF_PRINT("[RDM]: DfsNopExpireSetChPolicy=%d, Cfg Channel=%d, Bandwidth=%d\n",
		pDfsParam->DfsNopExpireSetChPolicy, pChCfg->boot_chan, wlan_config_get_eht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev));

	switch (*ch_stat) {
	case DFS_INB_CH_INIT:
		MTWF_PRINT("[RDM]: DFS channel info, ch_stat=DFS_INB_CH_INIT\n");
		break;
	case DFS_OUTB_CH_CAC:
		MTWF_PRINT("[RDM]: DFS channel info, ch_stat=DFS_OUTB_CH_CAC\n");
		break;
	case DFS_INB_CH_SWITCH_CH:
		MTWF_PRINT("[RDM]: DFS channel info, ch_stat=DFS_INB_CH_SWITCH_CH\n");
		break;
	case DFS_INB_DFS_OUTB_CH_CAC:
		MTWF_PRINT("[RDM]: DFS channel info, ch_stat=DFS_INB_DFS_OUTB_CH_CAC\n");
		break;
	case DFS_INB_DFS_OUTB_CH_CAC_DONE:
		MTWF_PRINT("[RDM]: DFS channel info, ch_stat=DFS_INB_DFS_OUTB_CH_CAC_DONE\n");
		break;
	case DFS_INB_DFS_RADAR_OUTB_CAC_DONE:
		MTWF_PRINT("[RDM]: DFS channel info, ch_stat=DFS_INB_DFS_RADAR_OUTB_CAC_DONE\n");
		break;
	default:
		MTWF_PRINT("[RDM]: DFS channel info, ch_stat=%d\n", *ch_stat);
		break;
	}

	MTWF_PRINT("[RDM]: ChannelMode=%d, bDfsWaitCfgChNop=%d\n",
		pDot11h->ChannelMode, pDfsParam->bDfsWaitCfgChNop);

	MTWF_PRINT("[RDM]: DfsChSelPrefer=%d, SWZeroWait=%d\n", pDfsParam->DfsChSelPrefer, pDfsParam->BW160ZeroWaitSupport);
	MTWF_PRINT("=========================================\n ");

		MTWF_PRINT("CH: %d,\tBW: %d,\tCAC cnt: %d,\tCAC: %d\n",
			pDfsParam->band_ch,
			pDfsParam->band_bw,
			pDot11h->RDCount,
			pDot11h->cac_time);
		MTWF_PRINT("-----------------------------------------\n ");
	if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE && hc_get_hw_band_idx(pAd) == pDfsParam->ZeroWaitBandidx
	&& pDfsParam->BW160ZeroWaitSupport == FALSE) || pDfsParam->bOcacEnable != 0) {
		MTWF_PRINT("dedicated RX:\n");
		MTWF_PRINT("CH: %d,\tBW: %d,\tCAC cnt: %d,\tCAC: %d\n",
			pDfsParam->OutBandCh,
			pDfsParam->OutBandBw,
			pDfsParam->DedicatedOutBandCacCount,
			pDfsParam->DedicatedOutBandCacTime);
	}
	MTWF_PRINT("=========================================\n ");

	return TRUE;
}

#ifdef WIFI_UNIFIED_COMMAND
VOID uni_dfs_dump_radar_sw_pls_info(
	PRTMP_ADAPTER pAd,
	P_UNI_EVENT_RDD_SEND_PULSE_T prRadarReport)
{
	UINT8 pls_idx = 0, rt_idx = 0;
	UINT32 pri_value = 0;
	BOOLEAN prd_radar_detected = FALSE;
	BOOLEAN sw_rdd_log_cond = pAd->CommonCfg.DfsParameter.sw_rdd_log_cond;

	if (prRadarReport == NULL)
		return;
	if ((prRadarReport->cr_pls_detected == 1) || (prRadarReport->stgr_pls_detected == 1))
		prd_radar_detected = TRUE;

	if ((prRadarReport->lng_pls_detected == 1) || (prd_radar_detected == TRUE) || (sw_rdd_log_cond == FALSE)) {
		if (prRadarReport->lng_pls_detected == 1) {
			MTWF_PRINT("===> RDD-%d: Long pulse radar is detected\n", prRadarReport->rdd_idx);
		} else {
				MTWF_PRINT("===> RDD-%d: No Long pulse radar is detected\n", prRadarReport->rdd_idx);
		}

		MTWF_PRINT("LPN = %d (FCC5_LPN = %d)\n",
			prRadarReport->out_lpn,
			pAd->CommonCfg.DfsParameter.fcc_lpn_min);

		if (prRadarReport->lng_pls_num <= LPB_SIZE) {
			MTWF_PRINT("\n----------------------Long pulse buffer----------------------\n");
			MTWF_PRINT("Index\t| ST(us)\t | PW(us)\t | Power(dBm)\t | PRI(us)\n");

			for (pls_idx = 0; pls_idx < prRadarReport->lng_pls_num; pls_idx++) {
				MTWF_PRINT("%u\t%u\t\t",
					pls_idx,
					(UINT32)(prRadarReport->lng_pls_buff[pls_idx].lng_strt_time * 4/10));
				MTWF_PRINT("%u\t\t%d\t\t",
					(UINT16)(prRadarReport->lng_pls_buff[pls_idx].lng_pls_wdth * 4/10),
					(INT16)((prRadarReport->lng_pls_buff[pls_idx].lng_pls_pwr - 1024)/4));
				if (pls_idx == 0)
					pri_value = 0;
				else {
					pri_value = (UINT32)(
					((prRadarReport->lng_pls_buff[pls_idx].lng_strt_time -
					prRadarReport->lng_pls_buff[pls_idx - 1].lng_strt_time) + RAMP_TIME) % RAMP_TIME);

					pri_value = (pri_value * 4 / 10);
				}
				MTWF_PRINT("%d\n", pri_value);
			}
			MTWF_PRINT("-----------------------------------------------------------\n");

			MTWF_PRINT("\n----------------------Long pulse raw data----------------------\n");
			MTWF_PRINT("ST-PW-Power;\n");

			for (pls_idx = 0; pls_idx < prRadarReport->lng_pls_num; pls_idx++) {
				MTWF_PRINT("%u-%u-%d;",
					(UINT32)(prRadarReport->lng_pls_buff[pls_idx].lng_strt_time),
					(UINT16)(prRadarReport->lng_pls_buff[pls_idx].lng_pls_wdth),
					(INT16)(prRadarReport->lng_pls_buff[pls_idx].lng_pls_pwr));
			}
			MTWF_PRINT("\n-----------------------------------------------------------\n");
		}

		if (prd_radar_detected == TRUE) {
			PSW_RADAR_TYPE_T sw_radar_type = NULL;

			MTWF_PRINT("===> RDD-%d: Periodic radar (RT-%d, RT_STGR = %d) is detected\n",
				prRadarReport->rdd_idx,
				prRadarReport->rt_idx,
				prRadarReport->stgr_pls_detected);

			rt_idx = prRadarReport->rt_idx;
			if (rt_idx >= RDD_RT_NUM) {
				MTWF_PRINT("Invalid rt_idx: %d\n", rt_idx);
				return;
			}

			sw_radar_type = &pAd->CommonCfg.DfsParameter.radar_thrshld_param.sw_radar_type[rt_idx];

			if (sw_radar_type == NULL)
				return;

			MTWF_PRINT("SPN = %d\n", prRadarReport->out_spn);
			MTWF_PRINT("CRPN = %d \t(RT_CRPN_MIN = %d, RT_CRPN_MAX = %d)\n",
				prRadarReport->out_crpn,
				sw_radar_type->rt_crpn_min,
				sw_radar_type->rt_crpn_max);
			MTWF_PRINT("CRPR = %d/%d \t(RT_CRPR_MIN = %d/%d)\n",
				prRadarReport->out_crpn,
				prRadarReport->prd_pls_num,
				sw_radar_type->rt_crpr_min, PPB_SIZE);
			MTWF_PRINT("CRPW = %d \t(RT_PW_MIN = %d, RT_PW_MAX = %d)\n",
				prRadarReport->out_crpw,
				sw_radar_type->rt_pw_min,
				sw_radar_type->rt_pw_max);
			MTWF_PRINT("PRI_CONST = %d \t(RT_PRI_MIN = %d, RT_PRI_MAX = %d)\n",
				prRadarReport->out_pri_const,
				sw_radar_type->rt_pri_min,
				sw_radar_type->rt_pri_max);
			MTWF_PRINT("CRBN = %d \t(RT_CRBN_MIN = %d, RT_CRBN_MAX = %d)\n",
				prRadarReport->out_crbn,
				sw_radar_type->rt_crbn_min,
				sw_radar_type->rt_crbn_max);
			MTWF_PRINT("PRI_STG1 = %d \t(RT_PRI_MIN = %d, RT_PRI_MAX*3 = %d)\n",
				prRadarReport->out_pri_stg1,
				sw_radar_type->rt_pri_min,
				sw_radar_type->rt_pri_max * 3);
			MTWF_PRINT("PRI_STG2 = %d \t(RT_PRI_MIN = %d, RT_PRI_MAX*3 = %d)\n",
				prRadarReport->out_pri_stg2,
				sw_radar_type->rt_pri_min,
				sw_radar_type->rt_pri_max * 3);
			MTWF_PRINT("PRI_STG3 = %d\n", prRadarReport->out_pri_stg3);
			MTWF_PRINT("PRI_DIFF12 = %d\n", prRadarReport->out_pri_stg_dmin);
			MTWF_PRINT("STGPW = %d \t(RT_PW_MIN = %d, RT_PW_MAX = %d)\n",
				prRadarReport->out_stg_pw,
				sw_radar_type->rt_pw_min,
				sw_radar_type->rt_pw_max);
			MTWF_PRINT("STGPN = %d \t(RT_STGPN_MIN = %d, RT_STGPN_MAX = %d)\n",
				prRadarReport->out_stg_pn,
				sw_radar_type->rt_stg_pn_min,
				sw_radar_type->rt_stg_pn_max);
			MTWF_PRINT("STGPR = %d/%d \t(RT_STGPR_MIN = %d/%d)\n",
				prRadarReport->out_stg_pn,
				prRadarReport->prd_pls_num,
				sw_radar_type->rt_stg_pr_min, PPB_SIZE);
		} else {
			MTWF_PRINT("===> RDD-%d: No periodic radar is detected\n", prRadarReport->rdd_idx);
			MTWF_PRINT("SPN = %d\n", prRadarReport->out_spn);
			MTWF_PRINT("CRPN = %d\n", prRadarReport->out_crpn);
			MTWF_PRINT("CRPR = %d/%d\n", prRadarReport->out_crpn, prRadarReport->prd_pls_num);
			MTWF_PRINT("CRPW = %d\n", prRadarReport->out_crpw);
			MTWF_PRINT("PRI_CONST = %d\n", prRadarReport->out_pri_const);
			MTWF_PRINT("CRBN = %d \n", prRadarReport->out_crbn);
			MTWF_PRINT("PRI_STG1 = %d \n", prRadarReport->out_pri_stg1);
			MTWF_PRINT("PRI_STG2 = %d \n", prRadarReport->out_pri_stg2);
			MTWF_PRINT("PRI_STG3 = %d\n", prRadarReport->out_pri_stg3);
			MTWF_PRINT("STG_PRI12_DIFF = %d\n", prRadarReport->out_pri_stg_dmin);
			MTWF_PRINT("STGPW = %d \n", prRadarReport->out_stg_pw);
			MTWF_PRINT("STGPN = %d \n", prRadarReport->out_stg_pn);
			MTWF_PRINT("STGPR = %d/%d\n", prRadarReport->out_stg_pn, prRadarReport->prd_pls_num);
		}

		if (prRadarReport->prd_pls_num <= PPB_SIZE) {
			MTWF_PRINT("\n----------------------Short pulse buffer----------------------\n");
			MTWF_PRINT("Index\t| ST(us)\t | PW(us)\t | Power(dBm)\t | PRI(us)\n");

			for (pls_idx = 0; pls_idx < prRadarReport->prd_pls_num; pls_idx++) {
				MTWF_PRINT("%u\t%u\t\t",
					pls_idx,
					(UINT32)(prRadarReport->prd_pls_buff[pls_idx].prd_strt_time * 4/10));
				MTWF_PRINT("%u\t\t%d\t\t",
					(UINT16)(prRadarReport->prd_pls_buff[pls_idx].prd_pls_wdth * 4/10),
					(INT16)(prRadarReport->prd_pls_buff[pls_idx].prd_pls_pwr - 1024)/4);
				if (pls_idx == 0)
					pri_value = 0;
				else {
					pri_value = (UINT32)(
					((prRadarReport->prd_pls_buff[pls_idx].prd_strt_time -
					prRadarReport->prd_pls_buff[pls_idx - 1].prd_strt_time + RAMP_TIME) % RAMP_TIME));

					pri_value = (pri_value * 4 / 10);
				}
				MTWF_PRINT("%d\n", pri_value);
			}
			MTWF_PRINT("--------------------------------------------------------------\n");

			MTWF_PRINT("\n----------------------Short pulse raw data----------------------\n");
			MTWF_PRINT("ST-PW-Power;\n");

			for (pls_idx = 0; pls_idx < prRadarReport->prd_pls_num; pls_idx++) {
				MTWF_PRINT("%u-%u-%d;",
					(UINT32)(prRadarReport->prd_pls_buff[pls_idx].prd_strt_time),
					(UINT16)(prRadarReport->prd_pls_buff[pls_idx].prd_pls_wdth),
					(INT16)(prRadarReport->prd_pls_buff[pls_idx].prd_pls_pwr));
			}
			MTWF_PRINT("\n--------------------------------------------------------------\n");
		}
	}

}

VOID uni_dfs_dump_radar_hw_pls_info(
	PRTMP_ADAPTER pAd,
	P_UNI_EVENT_RDD_SEND_PULSE_T prRadarReport)
{
	UINT8 pls_idx = 0;

	if (prRadarReport == NULL)
		return;

	MTWF_PRINT("\n--------------------------------------------------------------\n");

	MTWF_PRINT("===> RDD-%d: Interrupt\n", prRadarReport->rdd_idx);
	MTWF_PRINT("\n------------------------HW pulse buffer-----------------------\n");
	MTWF_PRINT("Index\t | ST(us)\t | PW(us)\t | Power(dBm)\t | \tSC\t | \tReset\t | \tMDRDY | \tTX_active\n");

	if (prRadarReport->hw_pls_num > MAX_HW_PB_SIZE) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"wrong parameter hw_pls_num %d\n", prRadarReport->hw_pls_num);
		return;
	}

	for (pls_idx = 0; pls_idx < prRadarReport->hw_pls_num; pls_idx++) {
		MTWF_PRINT("%d\t%u\t\t", pls_idx,
			(UINT32)(prRadarReport->hw_pls_buff[pls_idx].hw_start_time * 4/10));
		MTWF_PRINT("%u\t\t%d\t\t",
			(UINT16)(prRadarReport->hw_pls_buff[pls_idx].hw_pls_width * 4/10),
			(INT16)(prRadarReport->hw_pls_buff[pls_idx].hw_pls_pwr - 1024)/4);
		MTWF_PRINT("%s\t\t%s\t\t", prRadarReport->hw_pls_buff[pls_idx].hw_sc_pass ? "true":"false",
			prRadarReport->hw_pls_buff[pls_idx].hw_sw_reset ? "true":"false");
		MTWF_PRINT("%d\t\t", prRadarReport->hw_pls_buff[pls_idx].hw_mdrdy_flag);
		MTWF_PRINT("%d\t\t\n", prRadarReport->hw_pls_buff[pls_idx].hw_tx_active);
	}

	MTWF_PRINT("--------------------------------------------------------------\n");

	MTWF_PRINT("\n------------------------HW pulse raw data-----------------------\n");
	MTWF_PRINT("ST-PW-Power;\n");

	for (pls_idx = 0; pls_idx < prRadarReport->hw_pls_num; pls_idx++) {
		MTWF_PRINT("%u-%u-%d;",
			(UINT32)(prRadarReport->hw_pls_buff[pls_idx].hw_start_time),
			(UINT16)(prRadarReport->hw_pls_buff[pls_idx].hw_pls_width),
			(INT16)(prRadarReport->hw_pls_buff[pls_idx].hw_pls_pwr));
	}

	MTWF_PRINT("\n--------------------------------------------------------------\n");

}

VOID uni_dfs_update_radar_info(
	P_UNI_EVENT_RDD_SEND_PULSE_T prRadarReport)
{
	UINT8 rdd_idx = HW_RDD0;

	if (prRadarReport == NULL)
		return;

	rdd_idx = prRadarReport->rdd_idx;

	switch (rdd_idx) {
	case RDD_INBAND_IDX_1:
	case RDD_INBAND_IDX_2:
#if (RDD_2_SUPPORTED == 1)
	case RDD_DEDICATED_IDX:
#endif /* RDD_2_SUPPORTED */
		os_zero_mem(&uni_g_radar_info[rdd_idx], sizeof(UNI_EVENT_RDD_SEND_PULSE_T));
		uni_g_radar_info[rdd_idx] = *prRadarReport;
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"wrong parameter rdd_idx %d\n", rdd_idx);
		break;
	}
}
#endif
VOID dfs_dump_radar_sw_pls_info(
	PRTMP_ADAPTER pAd,
	P_EXT_EVENT_RDD_REPORT_T prRadarReport)
{
	UINT8 pls_idx = 0, rt_idx = 0;
	UINT32 pri_value = 0;
	BOOLEAN prd_radar_detected = FALSE;
	BOOLEAN sw_rdd_log_cond = pAd->CommonCfg.DfsParameter.sw_rdd_log_cond;

	if (prRadarReport == NULL)
		return;
	if ((prRadarReport->cr_pls_detected == 1) || (prRadarReport->stgr_pls_detected == 1))
		prd_radar_detected = TRUE;

	if ((prRadarReport->lng_pls_detected == 1) || (prd_radar_detected == TRUE) || (sw_rdd_log_cond == FALSE)) {
		if (prRadarReport->lng_pls_detected == 1) {
			MTWF_PRINT("===> RDD-%d: Long pulse radar is detected\n", prRadarReport->rdd_idx);
		} else {
				MTWF_PRINT("===> RDD-%d: No Long pulse radar is detected\n", prRadarReport->rdd_idx);
		}

		MTWF_PRINT("LPN = %d (FCC5_LPN = %d)\n",
			prRadarReport->out_lpn,
			pAd->CommonCfg.DfsParameter.fcc_lpn_min);

		if (prRadarReport->lng_pls_num) {
			MTWF_PRINT("\n----------------------Long pulse buffer----------------------\n");
			MTWF_PRINT("Index\t| ST(us)\t | PW(us)\t | Power(dBm)\t | PRI(us)\n");

			for (pls_idx = 0; pls_idx < prRadarReport->lng_pls_num; pls_idx++) {
				MTWF_PRINT("%u\t%u\t\t",
					pls_idx,
					(UINT32)(prRadarReport->lng_pls_buff[pls_idx].lng_strt_time * 4/10));
				MTWF_PRINT("%u\t\t%d\t\t",
					(UINT16)(prRadarReport->lng_pls_buff[pls_idx].lng_pls_wdth * 4/10),
					(INT16)((prRadarReport->lng_pls_buff[pls_idx].lng_pls_pwr - 1024)/4));
				if (pls_idx == 0)
					pri_value = 0;
				else {
					pri_value = (UINT32)(
					((prRadarReport->lng_pls_buff[pls_idx].lng_strt_time -
					prRadarReport->lng_pls_buff[pls_idx - 1].lng_strt_time) + RAMP_TIME) % RAMP_TIME);

					pri_value = (pri_value * 4 / 10);
				}
				MTWF_PRINT("%d\n", pri_value);
			}
			MTWF_PRINT("-----------------------------------------------------------\n");

			MTWF_PRINT("\n----------------------Long pulse raw data----------------------\n");
			MTWF_PRINT("ST-PW-Power;\n");

			for (pls_idx = 0; pls_idx < prRadarReport->lng_pls_num; pls_idx++) {
				MTWF_PRINT("%u-%u-%d;",
					(UINT32)(prRadarReport->lng_pls_buff[pls_idx].lng_strt_time),
					(UINT16)(prRadarReport->lng_pls_buff[pls_idx].lng_pls_wdth),
					(INT16)(prRadarReport->lng_pls_buff[pls_idx].lng_pls_pwr));
			}
			MTWF_PRINT("\n-----------------------------------------------------------\n");
		}

		if (prd_radar_detected == TRUE) {
			PSW_RADAR_TYPE_T sw_radar_type = NULL;

			MTWF_PRINT("===> RDD-%d: Periodic radar (RT-%d, RT_STGR = %d) is detected\n",
				prRadarReport->rdd_idx,
				prRadarReport->rt_idx,
				prRadarReport->stgr_pls_detected);

			rt_idx = prRadarReport->rt_idx;
			sw_radar_type = &pAd->CommonCfg.DfsParameter.radar_thrshld_param.sw_radar_type[rt_idx];

			if (sw_radar_type == NULL)
				return;

			MTWF_PRINT("SPN = %d\n", prRadarReport->out_spn);
			MTWF_PRINT("CRPN = %d \t(RT_CRPN_MIN = %d, RT_CRPN_MAX = %d)\n",
				prRadarReport->out_crpn,
				sw_radar_type->rt_crpn_min,
				sw_radar_type->rt_crpn_max);
			MTWF_PRINT("CRPR = %d/%d \t(RT_CRPR_MIN = %d/%d)\n",
				prRadarReport->out_crpn,
				prRadarReport->prd_pls_num,
				sw_radar_type->rt_crpr_min, PPB_SIZE);
			MTWF_PRINT("CRPW = %d \t(RT_PW_MIN = %d, RT_PW_MAX = %d)\n",
				prRadarReport->out_crpw,
				sw_radar_type->rt_pw_min,
				sw_radar_type->rt_pw_max);
			MTWF_PRINT("PRI_CONST = %d \t(RT_PRI_MIN = %d, RT_PRI_MAX = %d)\n",
				prRadarReport->out_pri_const,
				sw_radar_type->rt_pri_min,
				sw_radar_type->rt_pri_max);
			MTWF_PRINT("CRBN = %d \t(RT_CRBN_MIN = %d, RT_CRBN_MAX = %d)\n",
				prRadarReport->out_crbn,
				sw_radar_type->rt_crbn_min,
				sw_radar_type->rt_crbn_max);
			MTWF_PRINT("PRI_STG1 = %d \t(RT_PRI_MIN = %d, RT_PRI_MAX*3 = %d)\n",
				prRadarReport->out_pri_stg1,
				sw_radar_type->rt_pri_min,
				sw_radar_type->rt_pri_max * 3);
			MTWF_PRINT("PRI_STG2 = %d \t(RT_PRI_MIN = %d, RT_PRI_MAX*3 = %d)\n",
				prRadarReport->out_pri_stg2,
				sw_radar_type->rt_pri_min,
				sw_radar_type->rt_pri_max * 3);
			MTWF_PRINT("PRI_STG3 = %d\n", prRadarReport->out_pri_stg3);
			MTWF_PRINT("PRI_DIFF12 = %d\n", prRadarReport->out_pri_stg_dmin);
			MTWF_PRINT("STGPW = %d \t(RT_PW_MIN = %d, RT_PW_MAX = %d)\n",
				prRadarReport->out_stg_pw,
				sw_radar_type->rt_pw_min,
				sw_radar_type->rt_pw_max);
			MTWF_PRINT("STGPN = %d \t(RT_STGPN_MIN = %d, RT_STGPN_MAX = %d)\n",
				prRadarReport->out_stg_pn,
				sw_radar_type->rt_stg_pn_min,
				sw_radar_type->rt_stg_pn_max);
			MTWF_PRINT("STGPR = %d/%d \t(RT_STGPR_MIN = %d/%d)\n",
				prRadarReport->out_stg_pn,
				prRadarReport->prd_pls_num,
				sw_radar_type->rt_stg_pr_min, PPB_SIZE);
		} else {
			MTWF_PRINT("===> RDD-%d: No periodic radar is detected\n", prRadarReport->rdd_idx);
			MTWF_PRINT("SPN = %d\n", prRadarReport->out_spn);
			MTWF_PRINT("CRPN = %d\n", prRadarReport->out_crpn);
			MTWF_PRINT("CRPR = %d/%d\n", prRadarReport->out_crpn, prRadarReport->prd_pls_num);
			MTWF_PRINT("CRPW = %d\n",prRadarReport->out_crpw);
			MTWF_PRINT("PRI_CONST = %d\n", prRadarReport->out_pri_const);
			MTWF_PRINT("CRBN = %d \n", prRadarReport->out_crbn);
			MTWF_PRINT("PRI_STG1 = %d \n", prRadarReport->out_pri_stg1);
			MTWF_PRINT("PRI_STG2 = %d \n", prRadarReport->out_pri_stg2);
			MTWF_PRINT("PRI_STG3 = %d\n", prRadarReport->out_pri_stg3);
			MTWF_PRINT("STG_PRI12_DIFF = %d\n", prRadarReport->out_pri_stg_dmin);
			MTWF_PRINT("STGPW = %d \n", prRadarReport->out_stg_pw);
			MTWF_PRINT("STGPN = %d \n", prRadarReport->out_stg_pn);
			MTWF_PRINT("STGPR = %d/%d\n", prRadarReport->out_stg_pn, prRadarReport->prd_pls_num);
		}

		if (prRadarReport->prd_pls_num) {
			MTWF_PRINT("\n----------------------Short pulse buffer----------------------\n");
			MTWF_PRINT("Index\t| ST(us)\t | PW(us)\t | Power(dBm)\t | PRI(us)\n");

			for (pls_idx = 0; pls_idx < prRadarReport->prd_pls_num; pls_idx++) {
				MTWF_PRINT("%u\t%u\t\t",
					pls_idx,
					(UINT32)(prRadarReport->prd_pls_buff[pls_idx].prd_strt_time * 4/10));
				MTWF_PRINT("%u\t\t%d\t\t",
					(UINT16)(prRadarReport->prd_pls_buff[pls_idx].prd_pls_wdth * 4/10),
					(INT16)(prRadarReport->prd_pls_buff[pls_idx].prd_pls_pwr - 1024)/4);
				if (pls_idx == 0)
					pri_value = 0;
				else {
					pri_value = (UINT32)(
					((prRadarReport->prd_pls_buff[pls_idx].prd_strt_time -
					prRadarReport->prd_pls_buff[pls_idx - 1].prd_strt_time + RAMP_TIME) % RAMP_TIME));

					pri_value = (pri_value * 4 / 10);
				}
				MTWF_PRINT("%d\n", pri_value);
			}
			MTWF_PRINT("--------------------------------------------------------------\n");

			MTWF_PRINT("\n----------------------Short pulse raw data----------------------\n");
			MTWF_PRINT("ST-PW-Power;\n");

			for (pls_idx = 0; pls_idx < prRadarReport->prd_pls_num; pls_idx++) {
				MTWF_PRINT("%u-%u-%d;",
					(UINT32)(prRadarReport->prd_pls_buff[pls_idx].prd_strt_time),
					(UINT16)(prRadarReport->prd_pls_buff[pls_idx].prd_pls_wdth),
					(INT16)(prRadarReport->prd_pls_buff[pls_idx].prd_pls_pwr));
			}
			MTWF_PRINT("\n--------------------------------------------------------------\n");
		}
	}

}

VOID dfs_dump_radar_hw_pls_info(
	PRTMP_ADAPTER pAd,
	P_EXT_EVENT_RDD_REPORT_T prRadarReport)
{
	UINT8 pls_idx = 0;

	if (prRadarReport == NULL)
		return;

	MTWF_PRINT("\n--------------------------------------------------------------\n");

	MTWF_PRINT("===> RDD-%d: Interrupt\n", prRadarReport->rdd_idx);
	MTWF_PRINT("\n------------------------HW pulse buffer-----------------------\n");
	MTWF_PRINT("Index\t | ST(us)\t | PW(us)\t | Power(dBm)\t | \tSC\t | \tReset\t | \tMDRDY | \tTX_active\n");

	for (pls_idx = 0; pls_idx < prRadarReport->hw_pls_num; pls_idx++) {
		MTWF_PRINT("%d\t%u\t\t", pls_idx,
			(UINT32)(prRadarReport->hw_pls_buff[pls_idx].hw_start_time * 4/10));
		MTWF_PRINT("%u\t\t%d\t\t",
			(UINT16)(prRadarReport->hw_pls_buff[pls_idx].hw_pls_width * 4/10),
			(INT16)(prRadarReport->hw_pls_buff[pls_idx].hw_pls_pwr - 1024)/4);
		MTWF_PRINT("%s\t\t%s\t\t", prRadarReport->hw_pls_buff[pls_idx].hw_sc_pass ? "true":"false",
			prRadarReport->hw_pls_buff[pls_idx].hw_sw_reset ? "true":"false");
		MTWF_PRINT("%d\t\t", prRadarReport->hw_pls_buff[pls_idx].hw_mdrdy_flag);
		MTWF_PRINT("%d\t\t\n", prRadarReport->hw_pls_buff[pls_idx].hw_tx_active);
	}

	MTWF_PRINT("--------------------------------------------------------------\n");

	MTWF_PRINT("\n------------------------HW pulse raw data-----------------------\n");
	MTWF_PRINT("ST-PW-Power;\n");

	for (pls_idx = 0; pls_idx < prRadarReport->hw_pls_num; pls_idx++) {
		MTWF_PRINT("%u-%u-%d;",
			(UINT32)(prRadarReport->hw_pls_buff[pls_idx].hw_start_time),
			(UINT16)(prRadarReport->hw_pls_buff[pls_idx].hw_pls_width),
			(INT16)(prRadarReport->hw_pls_buff[pls_idx].hw_pls_pwr));
	}

	MTWF_PRINT("\n--------------------------------------------------------------\n");

}

VOID dfs_update_radar_info(
	P_EXT_EVENT_RDD_REPORT_T prRadarReport)
{
	UINT8 rdd_idx = HW_RDD0;

	if (prRadarReport == NULL)
		return;

	rdd_idx = prRadarReport->rdd_idx;

	switch (rdd_idx) {
	case HW_RDD0:
	case HW_RDD1:
#if (RDD_2_SUPPORTED == 1)
	case HW_RDD2:
#endif /* RDD_2_SUPPORTED */
		os_zero_mem(&g_radar_info[rdd_idx], sizeof(EXT_EVENT_RDD_REPORT_T));
		g_radar_info[rdd_idx] = *prRadarReport;
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"wrong parameter rdd_idx %d\n", rdd_idx);
		break;
	}
}

INT show_dfs_debug_proc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *arg)
{
	INT16 value = 0;
	UCHAR pls_idx = 0, rdd_idx = 0;

	value = (INT16)simple_strtol(arg, 0, 10);

	if (value == 1) {
		os_zero_mem(&g_radar_info, sizeof(EXT_EVENT_RDD_REPORT_T) * HW_RDD_NUM);
		MTWF_PRINT("Info clear\n");
	} else if (value == 0) {
		MTWF_PRINT("Debug info Start\n");
		for (rdd_idx = HW_RDD0; rdd_idx < HW_RDD_NUM; rdd_idx++) {
			MTWF_PRINT("RDD%d INFO\n", rdd_idx);

			if (!(g_radar_info[rdd_idx].lng_pls_detected ||
				g_radar_info[rdd_idx].cr_pls_detected ||
				g_radar_info[rdd_idx].stgr_pls_detected)) {
				MTWF_PRINT("\tNo data\n");
				continue;
			}

			for (pls_idx = 0; pls_idx < g_radar_info[rdd_idx].lng_pls_num; pls_idx++) {
				MTWF_PRINT("%d\t", pls_idx);
				MTWF_PRINT("%u\t", g_radar_info[rdd_idx].lng_pls_buff[pls_idx].lng_strt_time);
				MTWF_PRINT("%u\t", g_radar_info[rdd_idx].lng_pls_buff[pls_idx].lng_pls_wdth);
				MTWF_PRINT("%d\n", g_radar_info[rdd_idx].lng_pls_buff[pls_idx].lng_pls_pwr);
			}

			for (pls_idx = 0; pls_idx < g_radar_info[rdd_idx].prd_pls_num; pls_idx++) {
				MTWF_PRINT("%d\t", pls_idx);
				MTWF_PRINT("%u\t", g_radar_info[rdd_idx].prd_pls_buff[pls_idx].prd_strt_time);
				MTWF_PRINT("%u\t", g_radar_info[rdd_idx].prd_pls_buff[pls_idx].prd_pls_wdth);
				MTWF_PRINT("%d\n", g_radar_info[rdd_idx].prd_pls_buff[pls_idx].prd_pls_pwr);
			}
		}
		MTWF_PRINT("Debug info End\n");
	}
	return TRUE;
}

#if (((DFS_ZEROWAIT_DEFAULT_FLOW == 1) && defined(BACKGROUND_SCAN_SUPPORT)) || defined(MAP_R2))
static BOOLEAN dfs_ch_in_nop_status(IN RTMP_ADAPTER * pAd, IN UINT_8 target_ch)
{
	UCHAR i;
	BOOLEAN bChannelInNopStatus = FALSE;
	PCHANNEL_CTRL pChCtrl = NULL;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (pChCtrl->ChList[i].Channel == target_ch) {
			if ((pChCtrl->ChList[i].NonOccupancy == 0) &&
				(pChCtrl->ChList[i].NOPSaveForClear == 0))
				bChannelInNopStatus = FALSE;
			else {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
					"Ch(%d) still in NOP status\n", target_ch);
				bChannelInNopStatus = TRUE;
			}
			break;
		}
	}
	return bChannelInNopStatus;
}
#endif /* (((DFS_ZEROWAIT_DEFAULT_FLOW == 1) && def(BACKGROUND_SCAN_SUPPORT)) || def(MAP_R2)) */

static BOOLEAN IsAllChannelUnavailable(
	IN PRTMP_ADAPTER pAd)
{
	INT ch_idx;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
		if (pChCtrl->ChList[ch_idx].Priority != 0 && (pChCtrl->ChList[ch_idx].NonOccupancy == 0)
			&& (pChCtrl->ChList[ch_idx].NOPSaveForClear == 0))
			break;
	}

	if (ch_idx == pChCtrl->ChListNum)
		return TRUE;
	else
		return FALSE;
}

/*
    ==========================================================================
    Description:
	Set Channel NOP (channel, bw, state) from easymesh cmd
	if state equal to TRUE, Set the NOP of channel by bandwidth
	else if state equal to FALSE, Clear the NOP of channel by bandwidth
	bandwidth equal to BW_20, BW_40, BW_80 or BW_160
    Return:
	0 if all parameters are OK, negative value otherwise
    ==========================================================================
*/
#ifdef MAP_R2
INT mtk_nl80211_easymesh_set_nop_state_by_bw(
	IN PRTMP_ADAPTER pAd, UCHAR chan, UCHAR bw, BOOLEAN state)
{
	INT err_code = 0;
	UINT_8 target_ch, target_bw = 0;
	BOOLEAN target_ch_dfsband = FALSE;
	UINT_8 idx, ch_idx;
	ULONG nop_value;
	BOOLEAN find_ch = FALSE;
	UCHAR cap_bw = BW_20;
	UINT flag;
	CHANNEL_CTRL *pChCtrl;
	UCHAR BssIdx;
	UCHAR BandIdx = 0;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	struct DOT11_H *pDot11h = NULL;
	UCHAR RddIdx;
	BOOLEAN AllChannelUnavailable = FALSE;

	/* Set NOP of channel */
	if (state == TRUE)
		nop_value = CHAN_NON_OCCUPANCY;
	/* Clear NOP of channel */
	else
		nop_value = 0;

	if (bw > BW_160) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"Invalid bandwidth(%d) for set NOP!!\n", bw);
		return -EINVAL;
	}

	if (nop_value == CHAN_NON_OCCUPANCY &&
		(dfs_ch_in_nop_status(pAd, chan) == TRUE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"Avoid secondary setting NOP.\n");
		return err_code;
	}

	if (nop_value == 0 &&
		(dfs_ch_in_nop_status(pAd, chan) == FALSE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"Avoid secondary clear NOP.\n");
		return err_code;
	}

	BandIdx = hc_get_hw_band_idx(pAd);
	RddIdx = (BandIdx == BAND1) ? RDD_INBAND_IDX_1 : RDD_INBAND_IDX_2;

	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdev = &pMbss->wdev;

		if (wdev->pHObj == NULL)
			continue;
		/*Need choose RadioOn interface, before enq cmd to set channel*/
		if (IsHcRadioCurStatOffByWdev(wdev))
			continue;

		if (HcGetBandByWdev(wdev) == BandIdx)
			break;
	}

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"wdev is null!\n");
		return err_code;
	}

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"Dot11h is null!\n");
		return err_code;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"bw=%d, state=%d\n", bw, state);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"Set NOP of channel(%d) as %ld.\n", chan, nop_value);

	if (bw == BW_160 && (chan >= 36 && chan <= 64)) {
		target_ch = 58;
		target_bw = BW_80;
	} else {
		target_ch = chan;
		target_bw = bw;

		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (chan == pChCtrl->ChList[ch_idx].Channel) {
				find_ch = TRUE;
				flag = pChCtrl->ChList[ch_idx].Flags;
				break;
			}
		}

		if (find_ch) {
			if (flag & CHANNEL_320M_CAP)
				cap_bw = BW_320;
			else if (flag & CHANNEL_160M_CAP)
				cap_bw = BW_160;
			else if (flag & CHANNEL_80M_CAP)
				cap_bw = BW_80;
			else if (flag & CHANNEL_40M_CAP)
				cap_bw = BW_40;
			else
				cap_bw = BW_20;

			if (target_bw > cap_bw) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
					"Change to the best capability of channel bw(%d).\n", cap_bw);
				target_bw = cap_bw;
			}
		}
	}
	target_ch_dfsband = RadarChannelCheck(pAd, chan);

	for (idx = 0; idx < CFG_WIFI_RAM_BAND_NUM; idx++)
		DfsSetNonOccupancy(pAd, idx, target_ch, target_bw, target_ch_dfsband, nop_value);

	/* Synchronize the channel mode after NOP be synchronized */
	AllChannelUnavailable = IsAllChannelUnavailable(pAd);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
				"Current ChannelMode: %d, AllChannelUnavailable: %d\n",
				pDot11h->ChannelMode, AllChannelUnavailable);
	if ((pDot11h->ChannelMode == CHAN_SILENCE_MODE || pDot11h->ChannelMode == CHAN_NORMAL_MODE) &&
		(AllChannelUnavailable == TRUE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
					"Change the ChannelMode from ch mode(%d) to NOP mode. Disable MAC TX\n", pDot11h->ChannelMode);
		pDot11h->ChannelMode = CHAN_NOP_MODE;
		mtRddControl(pAd, CAC_START, RddIdx, 0, 0);
	} else if ((pDot11h->ChannelMode == CHAN_NOP_MODE) && (AllChannelUnavailable == FALSE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
					"Change the ChannelMode from NOP mode to NORMAL mode. Enable MAC TX\n");
		pDot11h->ChannelMode = CHAN_NORMAL_MODE;
		mtRddControl(pAd, NORMAL_START, RddIdx, 0, 0);
	}

	return err_code;
}
#endif /* MAP_R2 */

INT Set_DfsNOP_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	ULONG value;
	UINT_8 band_idx, ch_idx;
	PCHANNEL_CTRL pChCtrl = NULL;

	value = simple_strtol(arg, 0, 10);
	MTWF_PRINT("Set NOP of all channel as %ld.\n", value);

	for (band_idx = 0; band_idx < CFG_WIFI_RAM_BAND_NUM; band_idx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (RadarChannelCheck(pAd, pChCtrl->ChList[ch_idx].Channel)) {
				pChCtrl->ChList[ch_idx].NonOccupancy = value;
				pChCtrl->ChList[ch_idx].NOPClrCnt = 0;
				pChCtrl->ChList[ch_idx].NOPSetByBw = 0;
				pChCtrl->ChList[ch_idx].NOPSaveForClear = 0;
			}

			if (IS_CH_BETWEEN(pChCtrl->ChList[ch_idx].Channel, 36, 48))
				pChCtrl->ChList[ch_idx].NOPSaveForRestore = value;
		}
	}

	return TRUE;
}

/* DFS Zero Wait */
INT Set_DfsZeroWaitEnable_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	UCHAR Value;
	Value = (UCHAR) os_str_tol(arg, 0, 10);
	if (RDD_2_SUPPORTED == 1) {
		switch (Value) {
			case DFS_DEDICATED_ZERO_WAIT_DISABLED:
				pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
				pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
				break;

			case DFS_DEDICATED_ZERO_WAIT_ENABLED:
				pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
				pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
				break;

			case DFS_DEDICATED_ZERO_WAIT_DEFAULT_FLOW_ENABLED:
				pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
				pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = TRUE;
				break;

			default:
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_ERROR, "Invalid parameter, Please enter 0 or 1 or 2!\n");
				break;
		}
	} else
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_ERROR, "DedicatedZeroWait not support!\n");

	MTWF_PRINT("The DFS paramater: bDedicatedZeroWaitSupport=%d, bDedicatedZeroWaitDefault=%d\n",
		pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport,
		pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault);
	return TRUE;
}

INT Set_DfsChSelPrefer_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	UCHAR Value;

	Value = (UCHAR) os_str_tol(arg, 0, 10);
	if (Value >= RadarDetectSelectNum)
		MTWF_PRINT("Invalid parameter, Please enter 0 or 1 or 2!\n");
	else {
		MTWF_PRINT("DfsChSelPrefer = %d!\n", Value);
		pAd->CommonCfg.DfsParameter.DfsChSelPrefer = Value;
	}
	return TRUE;
}

INT Set_DfsZeroWaitCert_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	ULONG value, ret;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	struct freq_oper oper;
	UCHAR rd_region = 0; /* Region of radar detection */

	value = os_str_tol(arg, 0, 10);
	if (hc_radio_query(pAd, &oper) != HC_STATUS_OK) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "Cannot get radio info!\n");
		return FALSE;
	}
	pDfsParam->ucDisTm = value;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "In Set_RadarDetectStart_Proc:\n");
	rd_region = pAd->CommonCfg.RDDurRegion;
	ret = mtRddControl(pAd, DISABLE_ZW_TM, RDD_DEDICATED_IDX, RXSEL_0, rd_region);

	return TRUE;
}

INT Set_DfsZeroWaitCacTime_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	UCHAR Value;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	Value = (UCHAR) os_str_tol(arg, 0, 10);
	pDfsParam->DfsZeroWaitCacTime = Value;
	MTWF_PRINT("[%s][RDM]CacTime=%d/%d\n", __func__, Value, pDfsParam->DfsZeroWaitCacTime);
	return TRUE;
}

INT Set_DedicatedBwCh_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	CHAR *value = 0;
	UCHAR SynNum = 0, Channel = 0, Bw = 0, doCAC = 1;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	MTWF_PRINT("[%s][RDM]\n", __FUNCTION__);

	for (i = 0, value = rstrtok(arg, ":"); value && (i <= 3); value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0: /* Set Syn Num*/
			SynNum = simple_strtol(value, 0, 10);
			break;
		case 1: /* Set InBand ControlChannel */
			Channel = simple_strtol(value, 0, 10);
			break;
		case 2: /* Set InBand Bw*/
			Bw = simple_strtol(value, 0, 10);
			break;
		case 3: /* Set doCAC*/
			doCAC = simple_strtol(value, 0, 10);
			break;
		default:
			break;
		}
	}

	/* Disable zero-wait default flow */
	pDfsParam->bDedicatedZeroWaitDefault = FALSE;

#ifdef BACKGROUND_SCAN_SUPPORT
	switch (SynNum) {
	case RDD_INBAND_IDX_1:
#if (RDD_2_SUPPORTED == 1)
	case RDD_INBAND_IDX_2:
#endif /* RDD_2_SUPPORTED */
		DfsDedicatedInBandSetChannel(pAd, Channel, Bw, doCAC, SynNum);
		break;

	case RDD_DEDICATED_IDX:
		DfsDedicatedOutBandSetChannel(pAd, Channel, Bw, SynNum);
		break;

	default:
		break;
	}

#endif

	return TRUE;
}

INT Set_DfsZeroWaitDynamicCtrl_Proc(
	RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Value;

	Value = (UCHAR) simple_strtol(arg, 0, 10);

#ifdef BACKGROUND_SCAN_SUPPORT
#if (RDD_2_SUPPORTED == 0)
	DfsDedicatedDynamicCtrl(pAd, Value);
#endif
#endif

	return TRUE;
}

INT Set_DfsZeroWaitNOP_Proc(
		RTMP_ADAPTER * pAd, RTMP_STRING *arg)
{
	INT i;
	CHAR *value = 0;
	UCHAR Channel = 0, Bw = 0;
	USHORT NOPTime = 0;

	MTWF_PRINT("[%s][RDM]\n", __FUNCTION__);

	for (i = 0, value = rstrtok(arg, ":"); value && (i <= 2); value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			Channel = simple_strtol(value, 0, 10);
			break;
		case 1:
			Bw = simple_strtol(value, 0, 10);
			break;
		case 2:
			NOPTime = simple_strtol(value, 0, 10);
			break;
		default:
			break;
		}
	}

	ZeroWait_DFS_set_NOP_to_Channel_List(pAd, Channel, Bw, NOPTime);

	return TRUE;
}

INT Set_DfsTargetCh_Proc(
		RTMP_ADAPTER * pAd, RTMP_STRING *arg)
{
		INT i;
	CHAR *value = 0;
	UCHAR Channel = 0, Bw = 0;
	USHORT CacValue = 0;

	MTWF_PRINT("[%s][RDM]\n", __FUNCTION__);

	for (i = 0, value = rstrtok(arg, ":"); value && (i <= 2); value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			Channel = simple_strtol(value, 0, 10);
			break;
		case 1:
			Bw = simple_strtol(value, 0, 10);
			break;
		case 2:
			CacValue = simple_strtol(value, 0, 10);
			break;
		default:
			break;
		}
	}

	ZeroWait_DFS_Pre_Assign_Next_Target_Channel(pAd, Channel, Bw, CacValue);

	return TRUE;
}


VOID DfsSetCalibration(
	IN PRTMP_ADAPTER pAd, UINT_32 DisableDfsCal)
{
	if (!DisableDfsCal)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Enable DFS calibration in firmware.\n");
	else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Disable DFS calibration in firmware.\n");
		mtRddControl(pAd, DISABLE_DFS_CAL, HW_RDD0, 0, 0);
	}
}

VOID DfsSetZeroWaitCacSecond(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	pDfsParam->bZeroWaitCacSecondHandle = TRUE;
}

BOOLEAN DfsBypassRadarStateCheck(struct wifi_dev *wdev)
{
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return FALSE;

	pDot11h = wdev->pDot11_H;

	if (pDot11h == NULL)
		return FALSE;

	if (pDot11h->ChannelMode == CHAN_NORMAL_MODE)
		return TRUE;

	return FALSE;
}

BOOLEAN DfsZwBypassCac(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR OriChannel, UCHAR NewChannel)
{
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	UCHAR wdev_bw = 0, ch_band = 0, i;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	struct wifi_dev *tdev = NULL;

	wdev_bw = wlan_operate_get_vht_bw(wdev);
	ch_band = wlan_config_get_ch_band(wdev);

	if (pDfsParam->bDedicatedZeroWaitSupport == TRUE &&
		pDfsParam->bDedicatedZeroWaitDefault == TRUE &&
		hc_get_hw_band_idx(pAd) == pDfsParam->ZeroWaitBandidx) {

		if (ch_band == CMD_CH_BAND_5G) {
			if (wdev_bw == VHT_BW_160) {
				if ((vht_cent_ch_freq(OriChannel, wdev_bw, ch_band) == vht_cent_ch_freq(NewChannel, wdev_bw, ch_band))
					&& (IS_CH_BETWEEN(OriChannel, 36, 48) && IS_CH_BETWEEN(NewChannel, 36, 48))
					&& pAd->CommonCfg.DfsParameter.inband_ch_stat != DFS_INB_DFS_RADAR_OUTB_CAC_DONE) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
						"non-dfs ch switch to non-dfs ch, skip CAC\n ");
					return TRUE;
				}
			} else {
				for (i = 0; i < WDEV_NUM_MAX; i++) {
					tdev = pAd->wdev_list[i];

					if (tdev && tdev->wdev_type == WDEV_TYPE_STA)
						break;
				}

				if (tdev == NULL) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
						"tdev is NULL!!!\n ");
					return FALSE;
				}

				if ((tdev->func_idx < 0) || (tdev->func_idx >= MAX_MULTI_STA)) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
						"func_idx is error!!!\n ");
					return FALSE;
				}

				if (pAd->StaCfg[tdev->func_idx].ApcliInfStat.Enable && tdev->if_up_down_state) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
						"Apcli mode is enable, skip CAC\n ");
					return TRUE;
				}
			}
		}
	}
#endif

	return FALSE;
}


BOOLEAN DfsSetZeroWaitInitMbssCh(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR Ch)
{
	INT	i;
	BOOLEAN result = FALSE;
	CHANNEL_CTRL *pChCtrl;

	if (!pAd->CommonCfg.DfsParameter.bDfsEnable)
		return FALSE;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "No any wdev is active!\n");
		return FALSE;
	}

	if (wlan_config_get_ch_band(wdev) != CMD_CH_BAND_5G)
		return FALSE;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (Ch == pChCtrl->ChList[i].Channel) {
			result = pChCtrl->ChList[i].DfsReq;
			/*No need to consider bandwidth when scanning,*/
			if ((GetCurrentChannelOpOwner(pAd, wdev) == CH_OP_OWNER_SCAN) ||
				(GetCurrentChannelOpOwner(pAd, wdev) == CH_OP_OWNER_PARTIAL_SCAN))
				break;

			if (IS_CH_BETWEEN(Ch, 36, 48) &&
				((wlan_operate_get_vht_bw(wdev) == VHT_BW_160) || (wlan_config_get_vht_bw(wdev) == VHT_BW_160)))
				result = TRUE;

			break;
		}
	}

	if (result)
		pAd->CommonCfg.DfsParameter.MbssInitCac = TRUE;

	return result;
}

VOID Set_Zwdfs_Proc(RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
#ifdef MT_DFS_SUPPORT
	UCHAR OriChannel;
	BOOLEAN IsAband;
	struct freq_cfg fcfg;
	struct DOT11_H *pDot11h = NULL;
#if (((DFS_ZEROWAIT_DEFAULT_FLOW == 1) || defined(DFS_SDB_SUPPORT)) && defined(BACKGROUND_SCAN_SUPPORT))
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);
#endif
#if ((DFS_ZEROWAIT_DEFAULT_FLOW == 1) && defined(BACKGROUND_SCAN_SUPPORT))
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &ad->CommonCfg.DfsParameter.inband_ch_stat;
#endif
#endif /* MT_DFS_SUPPORT */

	if (WMODE_CAP_5G(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode))
		IsAband = TRUE;
	else
		IsAband = FALSE;

	os_zero_mem(&fcfg, sizeof(fcfg));
	phy_freq_get_cfg(wdev, &fcfg);

	if (ad->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault)
		ad->CommonCfg.DfsParameter.ZwChBootUp = TRUE;

#ifdef MT_DFS_SUPPORT
	pDot11h = wdev->pDot11_H;
	if (!DfsBypassRadarStateCheck(wdev)) {
		if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G)
			RadarStateCheck(ad, wdev);
		else if (pDot11h)
			pDot11h->ChannelMode = CHAN_NORMAL_MODE;
	} else {
		pDot11h->ChannelMode = CHAN_NORMAL_MODE;
		ad->CommonCfg.DfsParameter.ZwChBootUp = FALSE;
	}
#endif

	OriChannel = wdev->channel;
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
		if (!ad->CommonCfg.dbdc_mode) {
			if (RadarChannelCheck(ad, wdev->channel) && RadarChannelCheck(ad, wdev->vht_sec_80_channel)) {
				ad->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
				ad->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
				MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
					"dual band are all DFS channel, disable zero-wait\n");
			}
		}
#endif
	if (WMODE_CAP_5G(wdev->PhyMode)) {
#ifdef DFS_ADJ_BW_ZERO_WAIT
		if (pDot11h && RadarChannelCheck(ad, wdev->channel)
		&& ad->CommonCfg.DfsParameter.BW160ZeroWaitSupport == TRUE) {
			UCHAR vht_bw = wlan_config_get_vht_bw(wdev);

			ad->CommonCfg.DfsParameter.band_orich = wdev->channel;

			if (IS_CH_BETWEEN(wdev->channel, 36, 64) && vht_bw == VHT_BW_160) {
				pDot11h->ChannelMode = CHAN_NORMAL_MODE;
				wdev->channel = 36;
				ad->CommonCfg.DfsParameter.band_ch = 36;
				ad->CommonCfg.DfsParameter.BW160ZeroWaitState = DFS_BW160_TX80RX160;
				DfsAdjustOpBwSetting(ad, wdev, BW_160, BW_80);
				ad->CommonCfg.DfsParameter.band_bw = BW_80;
				MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
					"\x1b[1;33m SW based ZW-DFS init, CHAN_NORMAL_MODE \x1b[m\n");
			} else if (IS_CH_BETWEEN(wdev->channel, 52, 64) && vht_bw == VHT_BW_80) {
				pDot11h->ChannelMode = CHAN_NORMAL_MODE;
				wdev->channel = 36;
				ad->CommonCfg.DfsParameter.band_ch = 36;
				ad->CommonCfg.DfsParameter.BW160ZeroWaitState = DFS_BW80_TX80RX160;
				MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
					"\x1b[1;33m SW based ZW-DFS init, CHAN_NORMAL_MODE \x1b[m\n");
			} else if (IS_CH_BETWEEN(wdev->channel, 100, 128) && vht_bw == VHT_BW_160) {
				pDot11h->ChannelMode = CHAN_SILENCE_MODE;
				MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				"\x1b[1;33m SW based ZW-DFS init, CHAN_SILENCE_MODE \x1b[m\n");
				if (vht_bw == VHT_BW_160)
					ad->CommonCfg.DfsParameter.BW160ZeroWaitState = DFS_BW160_TX0RX0;
			} else
				ad->CommonCfg.DfsParameter.BW160ZeroWaitState = DFS_NOT_ENABLE;

			MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				"[RDM] OriCh:%d, NewCH:%d, BW:%d, DFSstate:%d\n", ad->CommonCfg.DfsParameter.band_orich, ad->CommonCfg.DfsParameter.band_ch,
				ad->CommonCfg.DfsParameter.band_bw, ad->CommonCfg.DfsParameter.BW160ZeroWaitState);
		}
#endif /* DFS_ADJ_BW_ZERO_WAIT */
		if (ad->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault == 0
			|| ad->CommonCfg.DfsParameter.BW160ZeroWaitSupport == TRUE) {
			MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"zero-wait DFS Dedicated is not enabled\n");
			DfsSyncCh1stBss(ad);
			if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G) {
#ifdef DFS_SDB_SUPPORT
				if (ad->CommonCfg.DfsParameter.bDfsSdbEnable == TRUE) {
					/* Use dedicated RX to monitor inband radar */
					ad->CommonCfg.DfsParameter.OutBandCh = ad->CommonCfg.DfsParameter.band_ch;
					ad->CommonCfg.DfsParameter.OutBandBw = ad->CommonCfg.DfsParameter.band_bw;
					if (ops->set_off_ch_scan) {
						MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
							"Use dedicated RX to monitor inband(ch:%d, bw:%d)\n",
								ad->CommonCfg.DfsParameter.band_ch,
								ad->CommonCfg.DfsParameter.band_bw);
						ops->set_off_ch_scan(ad, CH_SWITCH_BACKGROUND_SCAN_START, ENUM_BGND_DFS_TYPE);
					}
				} else
#endif /* DFS_SDB_SUPPORT */
					ops->set_off_ch_scan(ad, CH_SWITCH_BACKGROUND_SCAN_STOP, ENUM_BGND_DFS_TYPE);
			}
		} else {
			/* If DFS ch X is set by profile,
			CAC of DFS ch X will be checked by dedicated RX */
			if ((RadarChannelCheck(ad, (wdev->channel))
				|| DfsSetZeroWaitInitMbssCh(ad, wdev, (wdev->channel)))
				&& ad->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault != 0
				&& ad->CommonCfg.DfsParameter.BW160ZeroWaitSupport == FALSE) {
				DfsSyncCh1stBss(ad);
				if (*ch_stat == DFS_INB_CH_INIT) {
					if (zero_wait_dfs_update_ch(ad, wdev, OriChannel, &(wdev->channel))) {
						MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
						"DFS ch %d is set, use non-DFS ch %d\n",
						ad->CommonCfg.DfsParameter.OutBandCh, wdev->channel);

						/* Update channel of wdev as new channel */
						AutoChSelUpdateChannel(ad, wdev->channel, IsAband, wdev);

						os_zero_mem(&fcfg, sizeof(fcfg));
						phy_freq_get_cfg(wdev, &fcfg);
					} else {
						zero_wait_dfs_switch_ch(ad, wdev, RDD_DEDICATED_IDX);
						MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "<-------\n");
						return;
					}
				}
			} else if (wdev->channel == 0) {
				DfsAdjustOpBwSetting(ad, wdev, wlan_operate_get_bw(wdev), ad->CommonCfg.DfsParameter.band_bw);
				DfsSyncCh1stBss(ad);
			}
		}
	}

#endif /* DFS_ZEROWAIT_DEFAULT_FLOW */
#endif /*(defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT))*/
}

void DfsSyncCh1stBss(struct _RTMP_ADAPTER *ad)
{
	UCHAR ucCliNum = 0;
	struct wifi_dev *tdev;

	for (ucCliNum = 0; ucCliNum < ad->ApCfg.BssidNum; ucCliNum++) {
		tdev = &ad->ApCfg.MBSSID[ucCliNum].wdev;

		if (tdev && WMODE_CAP_5G(tdev->PhyMode)) {
			/* sync to other device */
			wdev_sync_prim_ch(ad, tdev);
			break;
		}
	}
}

BOOLEAN DfsRadarChannelCheck(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR vht_cent2,
	UCHAR phy_bw)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	BOOLEAN ret = FALSE;

	if (!IsChABand(wdev->PhyMode, wdev->channel) && (wdev->channel != 0))
		return FALSE;

	if (!pDfsParam->bDfsEnable)
		return FALSE;

#ifdef FTM_SUPPORT
	if (pDfsParam->ignore_dfs)
		return FALSE;
#endif /* FTM_SUPPORT */

#ifdef DOT11_VHT_AC
	/* for ch36~ch48 BW160 case, need to check radar detection */
	if ((phy_bw == BW_160) && (wdev->channel >= GROUP1_LOWER && wdev->channel <= GROUP1_UPPER))
		ret = TRUE;
#ifdef DFS_ADJ_BW_ZERO_WAIT
	/* for BW160 case, all the channel will hit radar */
	else if (pDfsParam->BW160ZeroWaitSupport == TRUE
	&& (IS_ADJ_BW_ZERO_WAIT_BW160(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState)
	|| IS_ADJ_BW_ZERO_WAIT_TX80RX160(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState))) {
		ret = TRUE;
	}
#endif
	else
#endif
	{
		ret = RadarChannelCheck(pAd, wdev->channel);
	}

	if (ret == TRUE
	|| (pDfsParam->bDedicatedZeroWaitSupport && hc_get_hw_band_idx(pAd) == pDfsParam->ZeroWaitBandidx))
		DfsGetSysParameters(pAd, wdev, vht_cent2, phy_bw);

	return ret;

}

VOID DfsCacEndUpdate(
	RTMP_ADAPTER * pAd,
	MLME_QUEUE_ELEM *Elem)
{
	UCHAR BandIdx;
	UCHAR RddIdx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UINT_8 BssIdx = 0;
	struct wifi_dev *wdev = NULL;
	UCHAR wdev_band_index = DBDC_BAND0;


	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "[RDM] CAC end. Enable MAC TX.\n");
	BandIdx = (UCHAR)(Elem->Priv);
	RddIdx = (BandIdx == BAND1) ? RDD_INBAND_IDX_1 : RDD_INBAND_IDX_2;
	mtRddControl(pAd, CAC_END, RddIdx, 0, 0);

	if (DfsCacTimeOutCallBack) {
		DfsCacTimeOutCallBack(BandIdx, pDfsParam->band_bw, pDfsParam->band_ch);
	}

#ifdef DFS_ADJ_BW_ZERO_WAIT
	DfsAdjBwCacDone(pAd);
#endif /* DFS_ADJ_BW_ZERO_WAIT */

	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
		wdev_band_index = HcGetBandByWdev(wdev);
		if ((wdev->bAllowBeaconing) && (wdev_band_index == BandIdx) && (!wdev->bcn_buf.bBcnSntReq)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
				"[RDM] Enabling Beaconing.\n");
			UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_ENABLE_TX));
		}
	}

#ifdef DFS_SLAVE_SUPPORT
	if (SLAVE_MODE_EN(pAd)) {
		for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
			wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
			if (wdev->wdev_type == WDEV_TYPE_AP)
				break;
		}
		slave_rdd_op(pAd, wdev, flag_reset);
	}
#endif /* DFS_SLAVE_SUPPORT */

	EDCCAInit(pAd, BandIdx);

	/*update CAC done*/
	if (pAd->CommonCfg.DfsParameter.CACMemoEn) {
		for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
			wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
			if (BandIdx == HcGetBandByWdev(wdev))
				break;
		}
		dfs_cac_op(pAd, wdev, CAC_DONE_UPDATE, wdev->channel);
	}
}


#if ((DFS_ZEROWAIT_DEFAULT_FLOW == 1) && defined(BACKGROUND_SCAN_SUPPORT))
VOID dfs_off_cac_end_update(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
	UCHAR BandIdx = 0;
	UCHAR RddIdx = 0;
	struct wifi_dev *wdev = NULL;
	UINT_8 BssIdx = 0;
	struct wlan_config *cfg = NULL;
	struct DOT11_H *dot11h_param = NULL;

	dot11h_param = &pAd->Dot11_H;

	if ((pDfsParam->bDedicatedZeroWaitSupport == FALSE
	|| hc_get_hw_band_idx(pAd) != pDfsParam->ZeroWaitBandidx) && pDfsParam->bOcacEnable == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "ZeroWaitDefault is not enabled\n");
		goto err;
	}

	if (!pDfsParam->bOutBandAvailable) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, " OutBand is not available\n");
		goto err;
	}

	if (dfs_ch_in_nop_status(pAd, pDfsParam->OutBandCh)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
		"Check the NOP times of outband ch(%d), avoid unavailable ch changed to inband\n",
		pDfsParam->OutBandCh);
		goto err;
	}

	if (pDfsParam->bOcacEnable != 0) {
		DfsSetOutbandCacDone(pAd, BandIdx, pDfsParam->OutBandCh, pDfsParam->OutBandBw, TRUE);
		goto err;
	}
#ifdef DFS_ADJ_BW_ZERO_WAIT
	if (pDfsParam->BW160ZeroWaitSupport == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"SW ZeroWait, OutBand is not available\n");
		goto err;
	}
#endif
#ifdef WIFI_MD_COEX_SUPPORT
	if (!IsChannelSafe(pAd, pDfsParam->OutBandCh)) {
		if (IsPwrChannelSafe(pAd, pDfsParam->OutBandCh))
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				"\x1b[1;33m New Channel %d is pwr backoff channel\x1b[m\n",
				pDfsParam->OutBandCh);

		else
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				"\x1b[1;33m New Channel %d is unsafe channel, stay in current channel\x1b[m\n",
				pDfsParam->OutBandCh);
		goto err;
	}
#endif
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "zero-wait CAC end, ch_stat %d\n", *ch_stat);

	if (*ch_stat == DFS_INB_DFS_OUTB_CH_CAC &&
		pDfsParam->SetOutBandChStat == OUTB_SET_CH_CAC) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
				"The set channel has completed CAC by outband\n");
		*ch_stat = DFS_OUTB_CH_CAC;
		pDfsParam->SetOutBandChStat = OUTB_SET_CH_DEFAULT;
	}

	switch (*ch_stat) {
	case DFS_OUTB_CH_CAC:
		/* Assign DFS outband Channel to inband Channel */
		/* use channel to find band index */
		BandIdx = dfs_get_band_by_ch(pAd, pDfsParam->OutBandCh);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "New inband channel %d bandidx %d\n",
					pDfsParam->OutBandCh, BandIdx);

		/* Assign DFS outband Channel to inband Channel */
		*ch_stat = DFS_INB_CH_SWITCH_CH;

		for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
			wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
			cfg = (struct wlan_config *)wdev->wpf_cfg;
			if (cfg) {
				MTWF_PRINT("Get device bssidx: %d\n", BssIdx);
				break;
			}
		}

		if (!cfg) {
			MTWF_PRINT("Invalid device!\n");
			break;
		}

		if (!DfsDedicatedInBandSetChannel(pAd, pDfsParam->OutBandCh, pDfsParam->OutBandBw, FALSE, RDD_DEDICATED_IDX)) {
			*ch_stat = DFS_OUTB_CH_CAC;
			DfsOutBandCacReset(pAd);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				"DfsDedicatedInBandSetChannel fail!! reset ch_stat to %d\n",
				*ch_stat);
		}
		break;

	case DFS_INB_DFS_OUTB_CH_CAC:
	case DFS_INB_DFS_OUTB_CH_CAC_DONE:
		/* new zero-wait CAC of outband is available */
		BandIdx = dfs_get_band_by_ch(pAd, pDfsParam->OutBandCh);
		RddIdx = (BandIdx == BAND1) ? RDD_INBAND_IDX_1 : RDD_INBAND_IDX_2;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Zero-wait CAC ch %d is available, bandidx %d\n",
					pDfsParam->OutBandCh, BandIdx);
		*ch_stat = DFS_INB_DFS_OUTB_CH_CAC_DONE;

		if (pAd->CommonCfg.DfsParameter.CACMemoEn
			&& pAd->CommonCfg.DfsParameter.bPreCacEn) {
			BandIdx = hc_get_hw_band_idx(pAd);
			for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
				wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
				if (BandIdx == HcGetBandByWdev(wdev))
					break;
			}
			dfs_pre_cac_detect_next_channel(pAd, wdev);
			}

		if (pDfsParam->BW80DedicatedZWSupport &&
			(IS_CH_BETWEEN(DfsPrimToCent(pDfsParam->OutBandCh, pDfsParam->OutBandBw), 100, 128)) &&
			(IS_CH_BETWEEN(DfsPrimToCent(pDfsParam->band_ch, pDfsParam->band_bw), 100, 128)) &&
			pDfsParam->OutBandBw == pDfsParam->band_bw)
			DfsDedicatedInBandSetChannel(pAd, pDfsParam->band_ch, BW_160, FALSE, RddIdx);

		ReleaseChannelOpChargeByBand(pAd, hc_get_hw_band_idx(pAd), CH_OP_OWNER_DFS);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "Invalid input %d\n", *ch_stat);
		ReleaseChannelOpChargeByBand(pAd, hc_get_hw_band_idx(pAd), CH_OP_OWNER_DFS);
		break;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Ch_stat %d\n", *ch_stat);

	return;
err:
	ReleaseChannelOpChargeByBand(pAd, hc_get_hw_band_idx(pAd), CH_OP_OWNER_DFS);
	return;
}


VOID DfsSetOutbandCacDone(/* Set Outband CAC Done */
	IN PRTMP_ADAPTER pAd,
	IN UCHAR band_idx,
	IN UINT_8 target_ch,
	IN UINT_8 target_bw,
	IN BOOLEAN target_ch_dfsband
)
{
	UINT_8 ch_idx;
	PCHANNEL_CTRL pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	switch (target_bw) {
	case BW_20:
		if (target_ch_dfsband == FALSE)
			return;

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (target_ch == pChCtrl->ChList[ch_idx].Channel)
				pChCtrl->ChList[ch_idx].IsOutBandCacDone = 1;
		}
		break;

	case BW_40:
		if (target_ch_dfsband == FALSE)
			return;

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (target_ch == pChCtrl->ChList[ch_idx].Channel)
				pChCtrl->ChList[ch_idx].IsOutBandCacDone = 1;
			else if (((target_ch) >> 2 & 1) && ((pChCtrl->ChList[ch_idx].Channel - target_ch) == 4))
				pChCtrl->ChList[ch_idx].IsOutBandCacDone = 1;
			else if (!((target_ch) >> 2 & 1) && ((target_ch - pChCtrl->ChList[ch_idx].Channel) == 4))
				pChCtrl->ChList[ch_idx].IsOutBandCacDone = 1;
			else
				;
		}
		break;

	case BW_80:
		if (target_ch_dfsband == FALSE)
			return;

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (vht_cent_ch_freq(pChCtrl->ChList[ch_idx].Channel, VHT_BW_80, CMD_CH_BAND_5G) ==
				vht_cent_ch_freq(target_ch, VHT_BW_80, CMD_CH_BAND_5G)) {
				pChCtrl->ChList[ch_idx].IsOutBandCacDone = 1;
			}
		}
		break;


	case BW_160:
		if (target_ch_dfsband == FALSE)
			return;

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (vht_cent_ch_freq(pChCtrl->ChList[ch_idx].Channel, VHT_BW_160, CMD_CH_BAND_5G) ==
				vht_cent_ch_freq(target_ch, VHT_BW_160, CMD_CH_BAND_5G)) {
				pChCtrl->ChList[ch_idx].IsOutBandCacDone = 1;
			}
		}
		break;

	default:
		break;
	}

}

UCHAR dfs_get_band_by_ch(
	RTMP_ADAPTER *pAd,
	UCHAR ch)
{
	return hc_get_hw_band_idx(pAd);
}


#endif

#ifdef CONFIG_AP_SUPPORT
NTSTATUS DfsChannelSwitchTimeoutAction(
	PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	UINT_32 SetChInfo;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	UINT8 bandIdx;
	UINT8 BssIdx;
	UINT8 NextCh;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

#ifdef DFS_SLAVE_SUPPORT
	UINT8 StaIdx = 0;
#endif /* DFS_SLAVE_SUPPORT */

	NdisMoveMemory(&SetChInfo, CMDQelmt->buffer, sizeof(UINT_32));

	bandIdx = (SetChInfo >> 16) & 0xff;
	BssIdx = (SetChInfo >> 8) & 0xff;
	NextCh = SetChInfo & 0xff;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "[RDM] bandIdx: %d, BssIdx: %d, NextCh: %d\n",
		bandIdx, BssIdx, NextCh);
	pMbss = &pAd->ApCfg.MBSSID[BssIdx];
	wdev = &pMbss->wdev;
#ifdef DFS_SLAVE_SUPPORT
	if (SLAVE_MODE_EN(pAd) &&
		(SetChInfo >> 24)) {
		StaIdx = (SetChInfo >> 8) & 0xff;
		wdev = &pAd->StaCfg[StaIdx].wdev;
	}
#endif /* DFS_SLAVE_SUPPORT */
	pDfsParam->RadarHitIdxRecord = bandIdx;

#ifdef BACKGROUND_SCAN_SUPPORT
	DedicatedZeroWaitStop(pAd, FALSE);
#endif

	perform_channel_change(pAd, wdev, NextCh);

	MtCmdSetDfsTxStart(pAd, bandIdx);

	DfsReportCollision(pAd);

	/*if no need CSA, just release ChannelOpCharge here*/
	if (pAd->ApCfg.set_ch_async_flag == FALSE)
		ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_DFS);

	return 0;
}

NTSTATUS DfsSwitchChAfterRadarDetected(
	PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	UINT_32 SetChInfo;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	UINT8 bandIdx;
	UINT8 BssIdx;
	UINT8 NextCh;
#ifdef DFS_SLAVE_SUPPORT
	UINT8 StaIdx = 0;
#endif /* DFS_SLAVE_SUPPORT */

#if ((DFS_ZEROWAIT_DEFAULT_FLOW == 1) && defined(BACKGROUND_SCAN_SUPPORT))
			/* Stop RDD of dedicated RX before doing ch switch */
			DedicatedZeroWaitStop(pAd, FALSE);
#endif

	NdisMoveMemory(&SetChInfo, CMDQelmt->buffer, sizeof(UINT_32));

	bandIdx = (SetChInfo >> 16) & 0xff;
	BssIdx = (SetChInfo >> 8) & 0xff;
	NextCh = SetChInfo & 0xff;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "[RDM] bandIdx: %d, BssIdx: %d, NextCh: %d\n",
		bandIdx, BssIdx, NextCh);

	pMbss = &pAd->ApCfg.MBSSID[BssIdx];
	wdev = &pMbss->wdev;
#ifdef DFS_SLAVE_SUPPORT
	if (SLAVE_MODE_EN(pAd) &&
		(SetChInfo >> 24)) {
		StaIdx = (SetChInfo >> 8) & 0xff;
		wdev = &pAd->StaCfg[StaIdx].wdev;
	}
#endif /* DFS_SLAVE_SUPPORT */
	rtmp_set_channel(pAd, wdev, NextCh);
	/*if no need CSA, just release ChannelOpCharge here*/
	if (pAd->ApCfg.set_ch_async_flag == FALSE)
		ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_DFS);

	return 0;
}

NTSTATUS DfsAPRestart(
	PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	UINT_32 SetChInfo;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	UINT8 bandIdx;
	UINT8 BssIdx;
	UINT8 NextCh;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	NdisMoveMemory(&SetChInfo, CMDQelmt->buffer, sizeof(UINT_32));

	bandIdx = (SetChInfo >> 16) & 0xff;
	BssIdx = (SetChInfo >> 8) & 0xff;
	NextCh = SetChInfo & 0xff;

	pMbss = &pAd->ApCfg.MBSSID[BssIdx];
	wdev = &pMbss->wdev;
	pDfsParam->RadarHitIdxRecord = bandIdx;

	APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);

	MtCmdSetDfsTxStart(pAd, bandIdx);


	return 0;
}
#endif

/*
RDD_INBAND_IDX_1 = Bandidx 1, Phyidx 1, rddidx 1
RDD_INBAND_IDX_2 = Bandidx 2, Phyidx 2, rddidx 0
RDD_DEDICATED_IDX = Bandidx 1, Phyidx 0, rddidx 2
*/
VOID DfsCacNormalStart(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR CompareMode)
{
	struct DOT11_H *pDot11h = NULL;
	UCHAR BandIdx;
	UCHAR RddIdx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if (wdev == NULL)
		return;

	if (wdev->channel <= 14)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;

	BandIdx = HcGetBandByWdev(wdev);
	RddIdx = (BandIdx == BAND1) ? RDD_INBAND_IDX_1 : RDD_INBAND_IDX_2;

	if (BandIdx >= RDD_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			 "Invalid BandIdx(%d)\n", BandIdx);
		return;
	}

	if (WMODE_CAP_6G(wdev->PhyMode)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "[%s]: wdev is 6G\n", __func__);
		return;
	}

#ifdef DFS_ADJ_BW_ZERO_WAIT
	if (IS_ADJ_BW_ZERO_WAIT_TX80RX160(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState)
	&& (IS_CH_BETWEEN(pDfsParam->band_ch, 100, 128)) && pDot11h->cac_time == CAC_WEATHER_BAND)
		return;
#endif

	if ((pAd->CommonCfg.RDDurRegion == CE) &&
		DfsCacRestrictBand(pAd, pDfsParam->band_bw, pDfsParam->band_ch,
				   pDfsParam->OutBandCh)) {
		/* Weather band channel */
		if (pDfsParam->targetCh != 0)
			pDot11h->cac_time = pDfsParam->targetCacValue;
		else
			pDot11h->cac_time = CAC_WEATHER_BAND;
	} else {
		if (pDfsParam->targetCh != 0)
			pDot11h->cac_time = pDfsParam->targetCacValue;
		else
			pDot11h->cac_time = CAC_NON_WEATHER_BAND;
	}

#ifdef DFS_ADJ_BW_ZERO_WAIT
	if (IS_ADJ_BW_ZERO_WAIT_TX80RX160(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState))
	{
		pDot11h->ChannelMode = CHAN_NORMAL_MODE;
		CompareMode = CHAN_NORMAL_MODE;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
			"[RDM] TX80/RX160 mode, CAC %d seconds start.\n",
			pDot11h->cac_time);
	}
#endif

	if ((pDot11h->ChannelMode == CHAN_SILENCE_MODE) && (CompareMode == CHAN_SILENCE_MODE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"[RDM] CAC %d seconds start . Disable MAC TX\n", pDot11h->cac_time);
		mtRddControl(pAd, CAC_START, RddIdx, 0, 0);
	} else if ((pDot11h->ChannelMode == CHAN_NORMAL_MODE) && (CompareMode == CHAN_NORMAL_MODE)) {
		if (pDot11h->RDCount == 0) {
			if (RadarChannelCheck(pAd, wdev->channel))
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
					"[RDM] Normal start. Enable MAC TX\n");

			mtRddControl(pAd, NORMAL_START, RddIdx, 0, 0);
		}
#ifdef DFS_ADJ_BW_ZERO_WAIT
		else if (IS_ADJ_BW_ZERO_WAIT_TX80RX160(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				"[RDM]  TX80/RX160 mode Normal start. Enable MAC TX\n");
			mtRddControl(pAd, NORMAL_START, RddIdx, 0, 0);
		} else if (IS_ADJ_BW_ZERO_WAIT_CAC_DONE(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				"[RDM]  Clear CAC Count\n");
			pDot11h->RDCount = 0;
		}
#endif
		else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
						"[RDM] End CAC. Enable MAC TX\n");
#ifdef CONFIG_MAP_SUPPORT
			if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {

				if (wdev->map_radar_detect == 0)
					wapp_send_ch_change_rsp(pAd, wdev, wdev->channel);

				wapp_send_cac_stop(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), wdev->channel, TRUE);
			}
#endif
			MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CAC_END, 0, NULL, HcGetBandByWdev(wdev));
			pDot11h->RDCount = 0;
		}
	} else if ((pDot11h->ChannelMode == CHAN_NOP_MODE) && (CompareMode == CHAN_NOP_MODE)) {
		/* Disable TX For no available channel */
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
			"Wait for NOP of channel. Disable MAC TX\n");
		mtRddControl(pAd, CAC_START, RddIdx, 0, 0);
	} else
		;
}

#ifdef DFS_ADJ_BW_ZERO_WAIT
VOID DfsAdjBwCacDone(
	RTMP_ADAPTER * pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	struct DOT11_H *pDot11h = NULL;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR BandIdx = hc_get_hw_band_idx(pAd);
	UCHAR new_ch = 0;
	UCHAR new_bw = 0;
	UCHAR new_vhtbw = 0;
	UINT_8 BssIdx = 0;

	pDot11h = &pAd->Dot11_H;
	if (pDot11h == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				"\x1b[41m pAd->Dot11_H is NULL !!\x1b[m\n");
		return;
	}

	if (IS_ADJ_BW_ZERO_WAIT_TX80RX160(pDfsParam->BW160ZeroWaitState)
		|| IS_ADJ_BW_ZERO_WAIT_TX0RX0(pDfsParam->BW160ZeroWaitState)
		|| IS_ADJ_BW_ZERO_WAIT_CAC_DONE(pDfsParam->BW160ZeroWaitState)) {

		if (IS_ADJ_BW_ZERO_WAIT_TX0RX0(pDfsParam->BW160ZeroWaitState))
			new_ch = 100;
		else
			new_ch = pDfsParam->band_orich;

		if (!new_ch)
			new_ch = DfsGetNonDfsDefaultCh(pAd, pDfsParam);

		if (pDfsParam->BW160ZeroWaitState == DFS_BW80_TX80RX160) {
			pDfsParam->BW160ZeroWaitState = DFS_BW80_TX80RX80;
			new_bw = BW_80;
			new_vhtbw = VHT_BW_80;
		} else if (pDfsParam->BW160ZeroWaitState == DFS_BW160_TX80RX160) {
			pDfsParam->BW160ZeroWaitState = DFS_BW160_TX160RX160;
			new_bw = BW_160;
			new_vhtbw = VHT_BW_160;
		} else if (pDfsParam->BW160ZeroWaitState == DFS_BW160_TX0RX0) {
			pDfsParam->BW160ZeroWaitState = DFS_BW160_TX80RX160;
			new_bw = BW_80;
			new_vhtbw = VHT_BW_80;
		} else if (pDfsParam->BW160ZeroWaitState == DFS_BW80_TX0RX0) {
			pDfsParam->BW160ZeroWaitState = DFS_BW80_TX80RX160;
			new_bw = BW_80;
			new_vhtbw = VHT_BW_80;
		} else {
			new_bw = pDfsParam->band_bw;
			new_vhtbw = (pDfsParam->band_bw == BW_20) ? VHT_BW_2040 : pDfsParam->band_bw-1;
		}

		/*To handle Radar event, need TakeChannelOpCharge first*/
		if (pDfsParam->bNoSwitchCh) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				"Turn OFF detection mode!!\n");
			return;
		}
		/*choose RadioOn interface*/
		for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
			pMbss = &pAd->ApCfg.MBSSID[BssIdx];
			wdev = &pMbss->wdev;

			if (wdev->pHObj == NULL)
				continue;

			/*Need choose RadioOn interface, before enq cmd to set channel*/
			if (IsHcRadioCurStatOffByWdev(wdev))
				continue;

			if (HcGetBandByWdev(wdev) == BandIdx)
				break;
		}

		DfsAdjustOpBwSetting(pAd, wdev, pDfsParam->band_bw, new_bw);
		pDfsParam->band_bw = new_bw;
		wlan_config_set_vht_bw(wdev, new_vhtbw);
		pDfsParam->band_ch = new_ch;
		perform_channel_change(pAd, wdev, new_ch);

		// if disable csa, need release lock
		if (pAd->ApCfg.set_ch_async_flag == FALSE)
			ReleaseChannelOpChargeByBand(pAd, BandIdx, CH_OP_OWNER_DFS);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
			"[RDM]\x1b[1;33m CHAN_NORMAL_MODE. Update Uniform Ch=%d, BW=%d \x1b[m\n",
			new_ch, new_bw);
	}
}
#endif

BOOLEAN DfsCacRestrictBand(/* Weather band channel: 5600 MHz - 5650 MHz */
	IN PRTMP_ADAPTER pAd, IN UCHAR Bw, IN UCHAR Ch, IN UCHAR SecCh)
{
	BOOLEAN ret = FALSE;
#ifdef DOT11_VHT_AC
	if ((Bw == BW_160) && (Ch >= GROUP3_LOWER && Ch <= RESTRICTION_BAND_HIGH))
		return TRUE;
	else
#endif
	{
		if (strncmp(pAd->CommonCfg.CountryCode, "KR", 2) == 0)
			ret = RESTRICTION_BAND_KOREA(pAd, Ch, Bw);
		else
			ret = RESTRICTION_BAND_1(pAd, Ch, Bw);
		return ret;
	}
}

BOOLEAN DfsCacRestrictBandForCentralCh(/* Weather band channel: 5600 MHz - 5650 MHz */
	IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev, IN UCHAR Bw, IN UCHAR Ch, IN UCHAR SecCh)
{
	BOOLEAN ret = FALSE, result = FALSE;
	UCHAR i, j;
	CHANNEL_CTRL *pChCtrl;
	u8 WeatherCh[3] = {120, 124, 128};

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	for (i = 0; i < pChCtrl->ChListNum; i++) {
		for (j = 0; j < 3; j++) {
			if ((WeatherCh[j] == pChCtrl->ChList[i].Channel) && pChCtrl->ChList[i].DfsReq) {
				result = TRUE;
				break;
			}
		}
	}
#ifdef DOT11_VHT_AC
	if ((Bw == BW_160) && (Ch >= GROUP3_LOWER && Ch <= RESTRICTION_BAND_HIGH)) {
		return result;
	} else
#endif
	{
		if (strncmp(pAd->CommonCfg.CountryCode, "KR", 2) == 0)
			ret = RESTRICTION_BAND_KOREA(pAd, Ch, Bw);
		else
			ret = RESTRICTION_BAND_1(pAd, Ch, Bw);
		return ret;
	}
}

VOID DfsBuildChannelList(
	IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev)
{
	UINT_8 i, j;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR band_idx = 0;
	CHANNEL_CTRL *pChCtrl = NULL;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "[RDM]: wdev is NULL.\n");
		return;
	}

	if (WMODE_CAP_6G(wdev->PhyMode)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "[%s]: wdev is 6G\n", __func__);
		return;
	}

	if (!WMODE_CAP_5G(wdev->PhyMode)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "[RDM]: wdev is not 5G.\n");
		return;
	}

	band_idx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	if (pChCtrl->ch_cfg.bw40_forbid)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
					"Forbid 40MHz Channel supported.\n");

	if (pDfsParam->NeedSetNewChList == DFS_SET_NEWCH_ENABLED) {
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			pChCtrl->ChList[i].SupportBwBitMap = 0;
			if (DfsCheckChAvailableByBw(pChCtrl->ChList[i].Channel, BW_20, pChCtrl))
				pChCtrl->ChList[i].SupportBwBitMap |= 0x01;
			if (DfsCheckChAvailableByBw(pChCtrl->ChList[i].Channel, BW_40, pChCtrl) &&
				!pChCtrl->ch_cfg.bw40_forbid)
				pChCtrl->ChList[i].SupportBwBitMap |= 0x02;
			if (DfsCheckChAvailableByBw(pChCtrl->ChList[i].Channel, BW_80, pChCtrl))
				pChCtrl->ChList[i].SupportBwBitMap |= 0x04;
			if (DfsCheckChAvailableByBw(pChCtrl->ChList[i].Channel, BW_160, pChCtrl))
				pChCtrl->ChList[i].SupportBwBitMap |= 0x08;
		}
	}

	/*restore NOP*/
	if (pAd->NopListBk) {
		NOP_LIST *pNopList = (NOP_LIST *)pAd->NopListBk;

		if (pNopList->ChListNum > DFS_AVAILABLE_LIST_CH_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
						"pNopList->ChListNum invalid.\n");
			os_free_mem(pAd->NopListBk);
			pAd->NopListBk = NULL;
		return;
	}

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Sync NopList count:%d\n", pNopList->ChListNum);
		for (j = 0; j < pNopList->ChListNum; j++) {
			for (i = 0; i < pChCtrl->ChListNum; i++) {
				if (pChCtrl->ChList[i].Channel == pNopList->DfsChList[j].Channel) {
					pChCtrl->ChList[i].NonOccupancy = pNopList->DfsChList[j].NonOccupancy;
					pChCtrl->ChList[i].NOPClrCnt = pNopList->DfsChList[j].NOPClrCnt;
					pChCtrl->ChList[i].NOPSetByBw = pNopList->DfsChList[j].NOPSetByBw;
					pChCtrl->ChList[i].NOPSaveForClear = pNopList->DfsChList[j].NOPSaveForClear;
					pChCtrl->ChList[i].SupportBwBitMap = pNopList->DfsChList[j].SupportBwBitMap;
					pChCtrl->ChList[i].IsOutBandCacDone = pNopList->DfsChList[j].IsOutBandCacDone;
					pChCtrl->ChList[i].Priority = pNopList->DfsChList[j].Priority;
				}
			}
		}
		/* clear nop list as sync to channel list done*/
		os_free_mem(pAd->NopListBk);
		pAd->NopListBk = NULL;
	}

	DfsBuildChannelGroupByBw(pAd, wdev);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Done\n");
	pDfsParam->NeedSetNewChList = DFS_SET_NEWCH_DISABLED;
}

VOID DfsBuildChannelGroupByBw(
	IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev)
{
	UINT_8 ch_idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	INT_8 BW40GroupIdx = -1;
	INT_8 BW80GroupIdx = -1;
	INT_8 BW160GroupIdx = -1;
	INT_8 BW40GroupMemberCnt = 0;
	INT_8 BW80GroupMemberCnt = 0;
	INT_8 BW160GroupMemberCnt = 0;
	UINT_8 PreviousBW40CentCh = 0xff;
	UINT_8 PreviousBW80CentCh = 0xff;
	UINT_8 PreviousBW160CentCh = 0xff;
	CHANNEL_CTRL *pChCtrl = NULL;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
		if (!IsChABand(wdev->PhyMode, pChCtrl->ChList[ch_idx].Channel))
			continue;
		if (!ByPassChannelByBw(pChCtrl->ChList[ch_idx].Channel, BW_40, pChCtrl)) {
			if (DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, BW_40) != PreviousBW40CentCh) {
				BW40GroupMemberCnt = 0;
				if ((++BW40GroupIdx < DFS_BW40_GROUP_NUM) && (BW40GroupMemberCnt < DFS_BW40_PRIMCH_NUM)
					&& (BW40GroupIdx >= 0))
					pDfsParam->dfs_ch_grp.Bw40GroupIdx[BW40GroupIdx][BW40GroupMemberCnt] = ch_idx;
			} else {
				if ((BW40GroupIdx >= 0) && (BW40GroupIdx < DFS_BW40_GROUP_NUM)
				 && (++BW40GroupMemberCnt < DFS_BW40_PRIMCH_NUM))
					pDfsParam->dfs_ch_grp.Bw40GroupIdx[BW40GroupIdx][BW40GroupMemberCnt] = ch_idx;
			}

			PreviousBW40CentCh = DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, BW_40);
		}

		if (!ByPassChannelByBw(pChCtrl->ChList[ch_idx].Channel, BW_80, pChCtrl)) {
			if (DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, BW_80) != PreviousBW80CentCh) {
				BW80GroupMemberCnt = 0;
				if ((++BW80GroupIdx < DFS_BW80_GROUP_NUM) && (BW80GroupMemberCnt < DFS_BW80_PRIMCH_NUM)
					&& (BW80GroupIdx >= 0))
					pDfsParam->dfs_ch_grp.Bw80GroupIdx[BW80GroupIdx][BW80GroupMemberCnt] = ch_idx;
			} else {
				if ((BW80GroupIdx >= 0) && (BW80GroupIdx < DFS_BW80_GROUP_NUM)
				 && (++BW80GroupMemberCnt < DFS_BW80_PRIMCH_NUM))
					pDfsParam->dfs_ch_grp.Bw80GroupIdx[BW80GroupIdx][BW80GroupMemberCnt] = ch_idx;
			}

			PreviousBW80CentCh = DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, BW_80);
		}
		if (!ByPassChannelByBw(pChCtrl->ChList[ch_idx].Channel, BW_160, pChCtrl)) {
			if (DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, BW_160) != PreviousBW160CentCh) {
				BW160GroupMemberCnt = 0;
				if ((++BW160GroupIdx < DFS_BW160_GROUP_NUM) && (BW160GroupMemberCnt < DFS_BW160_PRIMCH_NUM)
					&& (BW160GroupIdx >= 0))
					pDfsParam->dfs_ch_grp.Bw160GroupIdx[BW160GroupIdx][BW160GroupMemberCnt] = ch_idx;
			} else {
				if ((BW160GroupIdx >= 0) && (BW160GroupIdx < DFS_BW160_GROUP_NUM)
				 && (++BW160GroupMemberCnt < DFS_BW160_PRIMCH_NUM))
					pDfsParam->dfs_ch_grp.Bw160GroupIdx[BW160GroupIdx][BW160GroupMemberCnt] = ch_idx;
			}

			PreviousBW160CentCh = DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, BW_160);
		}

	}
}

BOOLEAN DfsCheckBwGroupAllAvailable(
	UCHAR CheckChIdx, UCHAR Bw, IN PRTMP_ADAPTER pAd, IN UCHAR band_idx)
{
	UCHAR *pBwxxGroupIdx = NULL;
	UCHAR i, j;
	UCHAR GroupNum = 4, BwxxPrimNum = 4;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	PCHANNEL_CTRL pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	if (Bw == BW_20)
		return TRUE;
	else if (Bw == BW_40) {
		pBwxxGroupIdx = &pDfsParam->dfs_ch_grp.Bw40GroupIdx[0][0];
		GroupNum = DFS_BW40_GROUP_NUM;
		BwxxPrimNum = DFS_BW40_PRIMCH_NUM;
	} else if (Bw == BW_80) {
		pBwxxGroupIdx = &pDfsParam->dfs_ch_grp.Bw80GroupIdx[0][0];
		GroupNum = DFS_BW80_GROUP_NUM;
		BwxxPrimNum = DFS_BW80_PRIMCH_NUM;
	} else if (Bw == BW_160) {
		pBwxxGroupIdx = &pDfsParam->dfs_ch_grp.Bw160GroupIdx[0][0];
		GroupNum = DFS_BW160_GROUP_NUM;
		BwxxPrimNum = DFS_BW160_PRIMCH_NUM;
	} else
		return FALSE;

	for (i = 0; i < (GroupNum * BwxxPrimNum); i++) {
		if (*pBwxxGroupIdx == CheckChIdx) {
			break;
		}
		pBwxxGroupIdx++;
	}

	if (i >= (GroupNum * BwxxPrimNum))
		return FALSE;

	j = i%BwxxPrimNum;
	i = i/BwxxPrimNum;

	pBwxxGroupIdx = pBwxxGroupIdx - j;

	for (j = 0; j < BwxxPrimNum; j++) {
		if (pChCtrl->ChList[*pBwxxGroupIdx].NonOccupancy != 0)
			return FALSE;
		if ((pChCtrl->ChList[*pBwxxGroupIdx].NonOccupancy == 0)
		 && (pChCtrl->ChList[*pBwxxGroupIdx].NOPClrCnt != 0)
		 && (pChCtrl->ChList[*pBwxxGroupIdx].NOPSetByBw <= Bw)
		 )
			return FALSE;

		pBwxxGroupIdx++;
	}

	return TRUE;
}

BOOLEAN DfsSwitchCheck(
	IN PRTMP_ADAPTER pAd,
	UCHAR Channel,
	UCHAR bandIdx)
{
	INT IdBss = 0;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR bandIdx_wdev = 0;

	for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
		pMbss = &pAd->ApCfg.MBSSID[IdBss];
		wdev = &pMbss->wdev;

		if ((pMbss == NULL) || (wdev == NULL) || (wdev->pHObj == NULL))
			continue;

		bandIdx_wdev = HcGetBandByWdev(wdev);

		if (bandIdx_wdev != bandIdx)
			continue;

		if (WMODE_CAP_6G(wdev->PhyMode)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "wdev is 6G\n");
			return FALSE;
		}
	}

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "NULL wdev!\n");
		return FALSE;
	}

	if ((pAd->Dot11_H.ChannelMode == CHAN_SILENCE_MODE) && WMODE_CAP_5G(wdev->PhyMode)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "DFS ByPass TX calibration.\n");
		return TRUE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "NON DFS calibration.\n");
		return FALSE;
	}
}

BOOLEAN DfsStopWifiCheck(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	return (pDfsParam->bNoAvailableCh == TRUE);
}


/*
	==========================================================================
	Description:
	Restore Cfg Channel & Bandwidth
	Return:
	VOID
	==========================================================================
*/
static VOID RestoreCfgCh(
	IN PRTMP_ADAPTER pAd, BOOLEAN RestoreOri)
{
	UINT_8 band_idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	BOOLEAN ret = 0;
	UINT_32 SetChInfo = 0;
	UCHAR BandIdx;
	UCHAR RddIdx;

	int BssIdx = 0;
	int Channel = 0;

	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR cfg_bw = wlan_config_get_eht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);

	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	struct Chan_Config *pChCfg = NULL;

#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
#endif

	if (!pChCtrl) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "can't find pChCtrl !\n");
		return;
	}

	pChCfg = &pChCtrl->ch_cfg;

	band_idx = hc_get_hw_band_idx(pAd);

	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdev = &pMbss->wdev;

		if (wdev->pHObj == NULL)
			continue;
		/*Need choose RadioOn interface, before enq cmd to set channel*/
		if (IsHcRadioCurStatOffByWdev(wdev))
			continue;

		if (HcGetBandByWdev(wdev) == band_idx) {
			BandIdx = HcGetBandByWdev(wdev);
			RddIdx = (BandIdx == BAND1) ? RDD_INBAND_IDX_1 : RDD_INBAND_IDX_2;
			if (RestoreOri)
				Channel = pChCfg->boot_chan;
			else if (!RadarChannelCheck(pAd, wdev->channel) && pDfsParam->OutBandCh == 0)
				Channel = WrapDfsRandomSelectChannel(pAd, pDfsParam->band_ch, RddIdx);
			break;
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "[RDM]Update wdev of BssIdx %d\n", BssIdx);

	if (RestoreOri && Channel) {

#if defined(CONFIG_MAP_SUPPORT) || defined(DFS_SLAVE_SUPPORT)
		UINT_8 i = 0;
		struct wifi_dev *sta_wdev = NULL;
#endif
			/*To handle Radar event, need TakeChannelOpCharge first*/
		if (pDfsParam->bNoSwitchCh) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				"Turn OFF detection mode!!\n");
			return;
		}
		if (!TakeChannelOpChargeByBand(pAd, BandIdx, CH_OP_OWNER_DFS, FALSE)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				"TakeChannelOpCharge fail for DFS!!\n");
			return;
		}

		if (BssIdx >= pAd->ApCfg.BssidNum) {
			/* this condition can occur in following two cases */
			/* case 1 : All 5G AP inf are down and apcli inf up */
			/* case 2 : "BandIdx = 2" for RDD on dedicated rx */
			wdev = NULL;
#ifdef DFS_SLAVE_SUPPORT
			if (SLAVE_MODE_EN(pAd)) {
				for (i = 0; i < MAX_APCLI_NUM; i++) {
					sta_wdev = &pAd->StaCfg[i].wdev;
					if ((HcGetBandByWdev(sta_wdev) == band_idx) &&
						sta_wdev->if_up_down_state) {
						wdev = sta_wdev;
						break;
					}
				}
			}
#endif /* DFS_SLAVE_SUPPORT */
		}

		if (wdev) {
			SetChInfo = 0;
#ifdef CONFIG_MAP_SUPPORT
/*On radar detect let AP stop start happen without apcli disconnect at AP stop*/
/*Link down only after sending the radar detect notification*/
			if (IS_MAP_ENABLE(pAd)) {
				wdev->map_radar_detect = 2;
				wdev->map_radar_channel = wdev->channel;
				wdev->map_radar_bw = wlan_operate_get_bw(wdev);
#ifdef CONFIG_STA_SUPPORT
				for (i = 0; i < MAX_APCLI_NUM; i++) {
					sta_wdev = &pAd->StaCfg[i].wdev;
					if (wdev->quick_ch_change != QUICK_CH_SWICH_DISABLE)
						wapp_send_radar_detect_notif(pAd, wdev, wdev->map_radar_channel, wdev->map_radar_bw, 0);
					if (sta_wdev->channel == wdev->channel)
						pAd->StaCfg[i].ApcliInfStat.Enable = FALSE;
				}
#endif /* CONFIG_STA_SUPPORT */
			}
#endif
#ifdef DFS_ADJ_BW_ZERO_WAIT
			if (pDfsParam->BW160ZeroWaitSupport == TRUE)
				pDfsParam->BW160ZeroWaitState = DFS_SET_CHANNEL;
#endif
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
			if (pDfsParam->bDedicatedZeroWaitSupport && pDfsParam->bDedicatedZeroWaitDefault)
				*ch_stat = DFS_INB_CH_INIT;
#endif
			SetChInfo |= Channel;
			SetChInfo |= (BssIdx << 8);
			SetChInfo |= (band_idx << 16);

#ifdef DFS_SLAVE_SUPPORT
			if (SLAVE_MODE_EN(pAd) && (wdev->wdev_type == WDEV_TYPE_STA)) {
				/* clear bssidx info and update staidx */
				SetChInfo &= 0xffff00ff; /* clear bit8-15 */
				SetChInfo |= (wdev->func_idx << 8);
				SetChInfo |= (1 << 24); /* use bit24 to indicate wdev sta */
			}
#endif /* DFS_SLAVE_SUPPORT */

			/* Init restore channel flag */
			pDfsParam->bDfsRestoreCfgChAfterNopEnd = TRUE;

			DfsAdjustBwSetting(pAd, wdev, pDfsParam->band_bw, cfg_bw);
			DfsAdjustOpBwSetting(pAd, wdev, pDfsParam->band_bw, cfg_bw);
			pDfsParam->band_bw = cfg_bw;

			ret = RTEnqueueInternalCmd(pAd, CMDTHRED_DFS_RADAR_DETECTED_SW_CH, &SetChInfo, sizeof(UINT_32));
			if (ret == NDIS_STATUS_SUCCESS)
				RTMP_MLME_HANDLER(pAd);
			else
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "[RDM]Enqueue DfsRestoreCH cmd fail[%d]\n", band_idx);
		}

		/*just release ChannelOpCharge before returning*/
		ReleaseChannelOpChargeByBand(pAd, BandIdx, CH_OP_OWNER_DFS);
	}
}

#ifdef CONFIG_AP_SUPPORT
VOID DfsNonOccupancyCountDown(/*NonOccupancy --*/
	IN PRTMP_ADAPTER pAd)
{
	UINT_8 ch_idx, band_idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	BOOLEAN InBandAvailable = FALSE;
	BOOLEAN Restore = FALSE, RestoreOri = FALSE, NeedUpDateChInfo = FALSE;
	PCHANNEL_CTRL pChCtrl = NULL;
	struct Chan_Config *pChCfg = NULL;
	struct wifi_dev *wdev = NULL;
	BSS_STRUCT *pMbss = NULL;

	band_idx = hc_get_hw_band_idx(pAd);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	if (!pChCtrl) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "can't find pChCtrl !\n");
		return;
	}
	pChCfg = &pChCtrl->ch_cfg;

	for (ch_idx = 0; ch_idx < pAd->ApCfg.BssidNum; ch_idx++) {
		pMbss = &pAd->ApCfg.MBSSID[ch_idx];
		wdev = &pMbss->wdev;

		if (wdev->pHObj == NULL)
			continue;
		/*Need choose RadioOn interface, before enq cmd to set channel*/
		if (IsHcRadioCurStatOffByWdev(wdev))
			continue;

		if (HcGetBandByWdev(wdev) == band_idx)
			break;
	}

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				 "wdev NULL!");
		return;
	}

	for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
		if (pChCtrl->ChList[ch_idx].NonOccupancy > 0) {
			pChCtrl->ChList[ch_idx].NonOccupancy--;

			/* Check the restore channel policy,
			and check the NOP status of the boot channel. */
			if (pChCtrl->ChList[ch_idx].NonOccupancy == 0) {
				NeedUpDateChInfo = TRUE;
				if (pDfsParam->DfsNopExpireSetChPolicy == DFS_NOP_EXPIRE_SET_CH_POLICY_RESTORE) {
					if (pChCfg->boot_chan != 0 && pChCtrl->ChList[ch_idx].Channel == pChCfg->boot_chan) {
						RestoreOri = TRUE;
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
						"Restore Original Channel %d.\n", pChCfg->boot_chan);
					}
					if (!Restore)
						Restore = TRUE;
				}
			}

#ifdef CONFIG_MAP_SUPPORT
			if (IS_MAP_TURNKEY_ENABLE(pAd)) {
				if (pChCtrl->ChList[ch_idx].NonOccupancy == 0) {
					int j = 0;

					for (j = 0; j < WDEV_NUM_MAX; j++) {
						if (pAd->wdev_list[j]) {
							wapp_send_radar_detect_notif(pAd, pAd->wdev_list[j],
									pChCtrl->ChList[ch_idx].Channel, wlan_operate_get_bw(pAd->wdev_list[j]), TRUE);
							break;
						}
					}
				}
			}
#endif
		}

		if (pChCtrl->ChList[ch_idx].NOPSaveForClear > 0) {
			pChCtrl->ChList[ch_idx].NOPSaveForClear--;

			/* Check the restore channel policy,
			and check the NOP status of the boot channel. */
			if (pChCtrl->ChList[ch_idx].NOPSaveForClear == 0) {
				NeedUpDateChInfo = TRUE;
				if (pDfsParam->DfsNopExpireSetChPolicy == DFS_NOP_EXPIRE_SET_CH_POLICY_RESTORE) {
					if (pChCfg->boot_chan != 0 && pChCtrl->ChList[ch_idx].Channel == pChCfg->boot_chan) {
						RestoreOri = TRUE;
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
						"Restore Original Channel %d.\n", pChCfg->boot_chan);
					}
					if (!Restore)
						Restore = TRUE;
				}
			}
#ifdef CONFIG_MAP_SUPPORT
			if (IS_MAP_TURNKEY_ENABLE(pAd)) {
				if (pChCtrl->ChList[ch_idx].NOPSaveForClear == 0) {
					int j = 0;

					for (j = 0; j < WDEV_NUM_MAX; j++) {
						if (pAd->wdev_list[j]) {
							wapp_send_radar_detect_notif(pAd, pAd->wdev_list[j],
									pChCtrl->ChList[ch_idx].Channel, wlan_operate_get_bw(pAd->wdev_list[j]), TRUE);
							break;
						}
					}
				}
			}
#endif

		}

		/* Check the restore channel policy,
		and check the NOP(ch36~48) status of the boot channel. */
		if (pChCtrl->ChList[ch_idx].NOPSaveForRestore > 0) {
			pChCtrl->ChList[ch_idx].NOPSaveForRestore--;
			if (pChCtrl->ChList[ch_idx].NOPSaveForRestore == 0) {
				NeedUpDateChInfo = TRUE;
				if (pDfsParam->DfsNopExpireSetChPolicy == DFS_NOP_EXPIRE_SET_CH_POLICY_RESTORE) {
					if (pChCfg->boot_chan != 0 && pChCtrl->ChList[ch_idx].Channel == pChCfg->boot_chan) {
						RestoreOri = TRUE;
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
						"Restore Original Channel %d.\n", pChCfg->boot_chan);
					}
					if (!Restore)
						Restore = TRUE;
				}
			}
		}

		else if ((pChCtrl->ChList[ch_idx].NOPSaveForClear == 0)
				&& (pChCtrl->ChList[ch_idx].NOPClrCnt != 0))
			pChCtrl->ChList[ch_idx].NOPClrCnt = 0;
	}


	if (pDfsParam->bNoAvailableCh == TRUE) {
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (pChCtrl->ChList[ch_idx].Channel == pDfsParam->PrimCh) {
				if (pChCtrl->ChList[ch_idx].NonOccupancy == 0 &&
					pChCtrl->ChList[ch_idx].NOPSaveForClear == 0)
					InBandAvailable = TRUE;
			}
		}

		if (InBandAvailable == TRUE) {
			pDfsParam->bNoAvailableCh = FALSE;

		}
	}

	/*update channel cap and pre-CAC channel*/
	if (NeedUpDateChInfo) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
				"update channel cap and pre-CAC channel\n");
#ifdef BACKGROUND_SCAN_SUPPORT
		dfs_pre_cac_detect_next_channel(pAd, NULL);
#endif /*BACKGROUND_SCAN_SUPPORT*/
		hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
		BuildChannelList(pAd, wdev);
	}

	if (Restore)
		RestoreCfgCh(pAd, RestoreOri);
}
#endif

/* For ch36~ch64 BW160 case. Set channel non-occupancy retore time */
static VOID DfsSetNOPSaveForRestore(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR band_idx,
	IN UINT_8 target_ch,
	IN UINT_8 target_bw,
	IN BOOLEAN target_ch_dfsband
)
{
	UINT_8 ch_idx;
	PCHANNEL_CTRL pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	switch (target_bw) {
	case BW_160:
		if (target_ch_dfsband == FALSE)
			return;

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (vht_cent_ch_freq(pChCtrl->ChList[ch_idx].Channel, VHT_BW_160, CMD_CH_BAND_5G) ==
				vht_cent_ch_freq(target_ch, VHT_BW_160, CMD_CH_BAND_5G))
				pChCtrl->ChList[ch_idx].NOPSaveForRestore = CHAN_NON_OCCUPANCY;
		}
		break;

	default:
		break;
	}
}

VOID WrapDfsSetNonOccupancy(/* Set Channel non-occupancy time */
	IN PRTMP_ADAPTER pAd,
	IN UCHAR rddidx,
	IN UCHAR BandIdx
)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UINT_8 target_ch = 0, target_bw = 0;
	BOOLEAN target_ch_dfsband = FALSE;
	UCHAR idx;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
		"[RDM]: Rdd index: %d, Band index: %d\n", rddidx, BandIdx);

	switch (rddidx) {
	case RDD_INBAND_IDX_1:
	case RDD_INBAND_IDX_2:
		if (pDfsParam->Dot11_H.ChannelMode == CHAN_SWITCHING_MODE)
			return;
		break;

	default:
		break;
	}

	if (((pDfsParam->bDedicatedZeroWaitSupport == TRUE
	&& hc_get_hw_band_idx(pAd) == pDfsParam->ZeroWaitBandidx)
	|| pDfsParam->bOcacEnable != 0)
	&& pDfsParam->RadarDetected[RDD_DEDICATED_IDX] == TRUE) {
		if (pDfsParam->OutBandBw == BW_160 && (pDfsParam->OutBandCh >= 36 && pDfsParam->OutBandCh <= 64)) {
			target_ch = 58;
			target_bw = BW_80;

			/* For ch36~ch64 BW160 case */
			/* Set NOP restore time for channel if radar is detected */
			for (idx = 0; idx < CFG_WIFI_RAM_BAND_NUM; idx++)
				DfsSetNOPSaveForRestore(pAd, idx, pDfsParam->OutBandCh, pDfsParam->OutBandBw, TRUE);
		} else {
			target_ch = pDfsParam->OutBandCh;
			target_bw = pDfsParam->OutBandBw;
		}
		target_ch_dfsband = pDfsParam->DfsChBand[RDD_DEDICATED_IDX];
	}
#ifdef DFS_ADJ_BW_ZERO_WAIT
	/* if current mode is adjust bw zero wait, althought channel is 36~48, but it is hit 52~64 actually */
	else if (IS_ADJ_BW_ZERO_WAIT_TX80RX160(pDfsParam->BW160ZeroWaitState))
	{
		if (IS_CH_BETWEEN(pDfsParam->band_ch, 36, 64)) {
			target_ch = 58;
			target_bw = BW_80;
			target_ch_dfsband = TRUE;
		} else if (IS_CH_BETWEEN(pDfsParam->band_ch, 100, 128)) {
			target_ch = 114;
			target_bw = BW_160;
			target_ch_dfsband = TRUE;
		}
	}
#endif
	/* For ch36~ch64 BW160 case, only ch52~ch64 set NOP */
	else if (pDfsParam->band_bw == BW_160 && (pDfsParam->band_ch >= 36 && pDfsParam->band_ch <= 64)) {
		target_ch = 58;
		target_bw = BW_80;
		target_ch_dfsband = TRUE;

		/* For ch36~ch64 BW160 case */
		/* Set NOP restore time for channel of band0 and band1 if radar is detected */
		for (idx = 0; idx < CFG_WIFI_RAM_BAND_NUM; idx++)
			DfsSetNOPSaveForRestore(pAd, idx, pDfsParam->band_ch, pDfsParam->band_bw, target_ch_dfsband);
	} else {
		target_ch = pDfsParam->band_ch;
		target_bw = pDfsParam->band_bw;
		target_ch_dfsband = pDfsParam->DfsChBand[rddidx];
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		"[RDM]: rddidx: %d, target_ch %d, target_bw %d, target_ch_dfsband %d\n",
		rddidx, target_ch, target_bw, target_ch_dfsband);

	/* Set NOP for channel of band0 and band1 if radar is detected */
	for (idx = 0; idx < CFG_WIFI_RAM_BAND_NUM; idx++)
		DfsSetNonOccupancy(pAd, idx, target_ch, target_bw, target_ch_dfsband, CHAN_NON_OCCUPANCY);
}

VOID DfsSetNonOccupancy(/* Set channel non-occupancy time */
	IN PRTMP_ADAPTER pAd,
	IN UCHAR band_idx,
	IN UINT_8 target_ch,
	IN UINT_8 target_bw,
	IN BOOLEAN target_ch_dfsband,
	IN ULONG nop_value
)
{
	UINT_8 ch_idx;
	PCHANNEL_CTRL pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR BssIdx;
	UCHAR BandIdx = hc_get_hw_band_idx(pAd);
	UCHAR vht_bw;

	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdev = &pMbss->wdev;

		if (wdev->pHObj == NULL)
			continue;
		/*Need choose RadioOn interface, before enq cmd to set channel*/
		if (IsHcRadioCurStatOffByWdev(wdev))
			continue;

		if (HcGetBandByWdev(wdev) == BandIdx)
			break;
	}

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				 "wdev NULL!");
		return;
	}

	switch (target_bw) {
	case BW_20:
		if (target_ch_dfsband == FALSE)
			return;

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if ((target_ch == pChCtrl->ChList[ch_idx].Channel)) {
				pChCtrl->ChList[ch_idx].NonOccupancy = nop_value;
				if (nop_value == 0) {
					pChCtrl->ChList[ch_idx].NOPSaveForClear = nop_value;
					pChCtrl->ChList[ch_idx].NOPClrCnt = 0;
				}
				pChCtrl->ChList[ch_idx].NOPSetByBw = target_bw;
				pChCtrl->ChList[ch_idx].IsOutBandCacDone = 0;
				pChCtrl->ChList[ch_idx].cac_done_timestamp = 0;
				pChCtrl->ChList[ch_idx].Flags &= ~CHANNEL_CAC_DONE;
#ifdef DFS_SLAVE_SUPPORT
				if (SLAVE_MODE_EN(pAd))
					pChCtrl->ChList[ch_idx].Flags |= CHANNEL_RDD_HIT;
#endif /* DFS_SLAVE_SUPPORT */
			}
		}
		break;

	case BW_40:
		if (target_ch_dfsband == FALSE)
			return;

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if ((target_ch == pChCtrl->ChList[ch_idx].Channel)) {
				pChCtrl->ChList[ch_idx].NonOccupancy = nop_value;
				if (nop_value == 0) {
					pChCtrl->ChList[ch_idx].NOPSaveForClear = nop_value;
					pChCtrl->ChList[ch_idx].NOPClrCnt = 0;
				}
				pChCtrl->ChList[ch_idx].NOPSetByBw = target_bw;
				pChCtrl->ChList[ch_idx].IsOutBandCacDone = 0;
				pChCtrl->ChList[ch_idx].cac_done_timestamp = 0;
				pChCtrl->ChList[ch_idx].Flags &= ~CHANNEL_CAC_DONE;
#ifdef DFS_SLAVE_SUPPORT
			if (SLAVE_MODE_EN(pAd))
				pChCtrl->ChList[ch_idx].Flags |= CHANNEL_RDD_HIT;
#endif /* DFS_SLAVE_SUPPORT */
			} else if (((target_ch) >> 2 & 1) && ((pChCtrl->ChList[ch_idx].Channel - target_ch) == 4)) {
				pChCtrl->ChList[ch_idx].NonOccupancy = nop_value;
				if (nop_value == 0) {
					pChCtrl->ChList[ch_idx].NOPSaveForClear = nop_value;
					pChCtrl->ChList[ch_idx].NOPClrCnt = 0;
				}
				pChCtrl->ChList[ch_idx].NOPSetByBw = target_bw;
				pChCtrl->ChList[ch_idx].IsOutBandCacDone = 0;
				pChCtrl->ChList[ch_idx].cac_done_timestamp = 0;
				pChCtrl->ChList[ch_idx].Flags &= ~CHANNEL_CAC_DONE;
#ifdef DFS_SLAVE_SUPPORT
			if (SLAVE_MODE_EN(pAd))
				pChCtrl->ChList[ch_idx].Flags |= CHANNEL_RDD_HIT;
#endif /* DFS_SLAVE_SUPPORT */
			} else if (!((target_ch) >> 2 & 1) && ((target_ch - pChCtrl->ChList[ch_idx].Channel) == 4)) {
				pChCtrl->ChList[ch_idx].NonOccupancy = nop_value;
				if (nop_value == 0) {
					pChCtrl->ChList[ch_idx].NOPSaveForClear = nop_value;
					pChCtrl->ChList[ch_idx].NOPClrCnt = 0;
				}
				pChCtrl->ChList[ch_idx].NOPSetByBw = target_bw;
				pChCtrl->ChList[ch_idx].IsOutBandCacDone = 0;
				pChCtrl->ChList[ch_idx].cac_done_timestamp = 0;
				pChCtrl->ChList[ch_idx].Flags &= ~CHANNEL_CAC_DONE;
#ifdef DFS_SLAVE_SUPPORT
				if (SLAVE_MODE_EN(pAd))
					pChCtrl->ChList[ch_idx].Flags |= CHANNEL_RDD_HIT;
#endif /* DFS_SLAVE_SUPPORT */
			}
			else
				;
		}
		break;

	case BW_80:
		if (target_ch_dfsband == FALSE)
			return;

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (vht_cent_ch_freq(pChCtrl->ChList[ch_idx].Channel, VHT_BW_80, CMD_CH_BAND_5G) ==
				vht_cent_ch_freq(target_ch, VHT_BW_80, CMD_CH_BAND_5G)) {
				pChCtrl->ChList[ch_idx].NonOccupancy = nop_value;
				if (nop_value == 0) {
					pChCtrl->ChList[ch_idx].NOPSaveForClear = nop_value;
					pChCtrl->ChList[ch_idx].NOPClrCnt = 0;
				}
				pChCtrl->ChList[ch_idx].NOPSetByBw = target_bw;
				pChCtrl->ChList[ch_idx].IsOutBandCacDone = 0;
				pChCtrl->ChList[ch_idx].cac_done_timestamp = 0;
				pChCtrl->ChList[ch_idx].Flags &= ~CHANNEL_CAC_DONE;
#ifdef DFS_SLAVE_SUPPORT
				if (SLAVE_MODE_EN(pAd))
					pChCtrl->ChList[ch_idx].Flags |= CHANNEL_RDD_HIT;
#endif /* DFS_SLAVE_SUPPORT */
			}
		}
		break;


	case BW_160:
		vht_bw = VHT_BW_160;

		if (target_ch_dfsband == FALSE)
				return;

		if (IS_CH_BETWEEN(target_ch, 132, 144)) {
			/* vht not support 132~140 bw160 */
			vht_bw = VHT_BW_80;
		}

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (vht_cent_ch_freq(pChCtrl->ChList[ch_idx].Channel, vht_bw, CMD_CH_BAND_5G) ==
				vht_cent_ch_freq(target_ch, vht_bw, CMD_CH_BAND_5G)) {
				pChCtrl->ChList[ch_idx].NonOccupancy = nop_value;
				if (nop_value == 0) {
					pChCtrl->ChList[ch_idx].NOPSaveForClear = nop_value;
					pChCtrl->ChList[ch_idx].NOPClrCnt = 0;
				}
				pChCtrl->ChList[ch_idx].NOPSetByBw = target_bw;
				pChCtrl->ChList[ch_idx].IsOutBandCacDone = 0;
				pChCtrl->ChList[ch_idx].cac_done_timestamp = 0;
				pChCtrl->ChList[ch_idx].Flags &= ~CHANNEL_CAC_DONE;
#ifdef DFS_SLAVE_SUPPORT
				if (SLAVE_MODE_EN(pAd))
					pChCtrl->ChList[ch_idx].Flags |= CHANNEL_RDD_HIT;
#endif /* DFS_SLAVE_SUPPORT */
			}
		}
		break;

	default:
		break;
	}

	hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
	BuildChannelList(pAd, wdev);
}

/*
RDD_INBAND_IDX_1 = Bandidx 1, Phyidx 1, rddidx 1
RDD_INBAND_IDX_2 = Bandidx 2, Phyidx 2, rddidx 0
RDD_DEDICATED_IDX = Bandidx 1, Phyidx 0, rddidx 2
*/
#ifdef CONFIG_AP_SUPPORT
VOID WrapDfsRddReportHandle(/* handle the event of UNI_EVENT_ID_RDD */
	IN PRTMP_ADAPTER pAd, UCHAR ucRddIdx)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR NextCh = 0;
	UCHAR NextBw = 0;
	UCHAR KeepBw = 0;
	UCHAR BandIdx = 0;
	UCHAR BssIdx, BssIdx1;
	UINT_32 SetChInfo = 0;
	BSS_STRUCT *pMbss = NULL;
#if defined(CONFIG_MAP_SUPPORT) || defined(DFS_SLAVE_SUPPORT)
	UINT_8 i = 0;
	struct wifi_dev *sta_wdev = NULL;
#endif
	struct wifi_dev *wdev = NULL;
	struct DOT11_H *dot11h_param = NULL;
#if defined(DFS_VENDOR10_CUSTOM_FEATURE)
	USHORT BwChannel;
#endif
#if defined(OFFCHANNEL_SCAN_FEATURE) && defined(MAP_R2)
	OFFCHANNEL_SCAN_MSG Rsp;
#endif
#ifdef TR181_SUPPORT
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
#endif /*TR181_SUPPORT*/
	BOOLEAN ret = 0;

#ifdef CONFIG_MAP_SUPPORT
#ifdef MAP_R2
		UCHAR band;
		UCHAR first_wdev = TRUE;
		UCHAR Channel_for_radar = 0;
		UCHAR bw_for_radar = 0;
#endif
#endif

#if (RDD_2_SUPPORTED == 1)
	if (!pDfsParam->bDfsDedicatedRxPreselectCh && pDfsParam->bDedicatedZeroWaitSupport &&
		pDfsParam->bDedicatedZeroWaitDefault && (ucRddIdx == HW_RDD2) &&
		RadarChannelCheck(pAd, pDfsParam->band_ch)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"[RDM]: Dedicated Rx is not in working state\n");
		return;
	}
#endif

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
		"[RDM]:  Radar detected !!!!!!!!!!!!!!!!!\n");

#ifdef DFS_SDB_SUPPORT
	if (pDfsParam->bDfsSdbEnable == TRUE && ucRddIdx == RDD_DEDICATED_IDX) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
			"For 5 + 6G enable, mapping rddidx to %d\n", RDD_INBAND_IDX_2);
		/* TODO: always mapping to RDD_INBAND_IDX_1? */
		ucRddIdx = RDD_INBAND_IDX_2;
		pDfsParam->OutBandCh = 0;
		pDfsParam->OutBandBw = 0;
	}
#endif /* DFS_SDB_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
		"[RDM]:  ucRddIdx: %d\n", ucRddIdx);

	BandIdx = hc_get_hw_band_idx(pAd);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "BandIdx:%d, ucRddIdx: %d\n", BandIdx, ucRddIdx);

	switch (ucRddIdx) {
	case RDD_INBAND_IDX_1:
	case RDD_INBAND_IDX_2:
		dot11h_param = &pAd->Dot11_H;
		pDfsParam->Dot11_H.ChannelMode = dot11h_param->ChannelMode;
		break;

#if (RDD_2_SUPPORTED == 1)
	case RDD_DEDICATED_IDX:
		break;
#endif

	default:
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"Error ucRddIdx = %d\n", ucRddIdx);
		return;
	}

	/* To handle no available channel in inband case */
	if (((ucRddIdx == RDD_INBAND_IDX_1) || (ucRddIdx == RDD_INBAND_IDX_2)) &&
		dot11h_param->ChannelMode == CHAN_NOP_MODE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
		"Ignore inband radar event when no available ch at inband\n");
		RTEnqueueInternalCmd(pAd, CMDTHREAD_DROP_RADAR_EVENT, &BandIdx, sizeof(UCHAR));
		return;
	}

	/*To handle Radar event, need TakeChannelOpCharge first*/
	if (pDfsParam->bNoSwitchCh) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "Turn OFF detection mode!!\n");
#if (RDD_2_SUPPORTED == 1)
		if (ucRddIdx != RDD_DEDICATED_IDX)
			RTEnqueueInternalCmd(pAd, CMDTHREAD_DROP_RADAR_EVENT, &BandIdx, sizeof(UCHAR));
#else
		RTEnqueueInternalCmd(pAd, CMDTHREAD_DROP_RADAR_EVENT, &BandIdx, sizeof(UCHAR));
#endif
		return;
	}
	if (!TakeChannelOpChargeByBand(pAd, BandIdx, CH_OP_OWNER_DFS, FALSE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "TakeChannelOpCharge fail for DFS!!\n");
#if (RDD_2_SUPPORTED == 1)
		if (ucRddIdx != RDD_DEDICATED_IDX)
			RTEnqueueInternalCmd(pAd, CMDTHREAD_DROP_RADAR_EVENT, &BandIdx, sizeof(UCHAR));
#else
		RTEnqueueInternalCmd(pAd, CMDTHREAD_DROP_RADAR_EVENT, &BandIdx, sizeof(UCHAR));
#endif
		return;
	}

#if defined(OFFCHANNEL_SCAN_FEATURE) && defined(MAP_R2)
	if (IS_MAP_ENABLE(pAd)) {
		Rsp.Action = DFS_RADAR_HIT;
		band = (pAd->CommonCfg.dbdc_mode) ? DBDC_BAND1 : DBDC_BAND0;
		memcpy(Rsp.ifrn_name, pAd->ScanCtrl.if_name, IFNAMSIZ);
		pAd->radar_hit = TRUE;
		Rsp.data.operating_ch_info.cfg_ht_bw = wlan_config_get_ht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
		Rsp.data.operating_ch_info.cfg_vht_bw = wlan_config_get_vht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
		Rsp.data.operating_ch_info.RDDurRegion = pAd->CommonCfg.RDDurRegion;
		Rsp.data.operating_ch_info.region = GetCountryRegionFromCountryCode(pAd->CommonCfg.CountryCode);
		/* Can be used as an info from driver by default yes */
		Rsp.data.operating_ch_info.is4x4Mode = 1;

		for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
			pMbss = &pAd->ApCfg.MBSSID[BssIdx];
			wdev = &pMbss->wdev;
			if (wdev->pHObj == NULL)
				continue;
			if (HcGetBandByWdev(wdev) != BandIdx)
				continue;
			Rsp.ifIndex = RtmpOsGetNetIfIndex(wdev->if_dev);
		}
		if (ucRddIdx == RDD_DEDICATED_IDX)
			Rsp.data.operating_ch_info.channel = pDfsParam->OutBandCh;
		else
			Rsp.data.operating_ch_info.channel = wdev->channel;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "Channel: %d %d\n",
			Rsp.data.operating_ch_info.channel, pDfsParam->OutBandCh);
		if (Rsp.data.operating_ch_info.channel > 14) {
			if (wapp_send_event_offchannel_info(pAd, (UCHAR *)&Rsp, sizeof(OFFCHANNEL_SCAN_MSG))) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
					"Err: wapp_send_event_offchannel_info fail\n");
				return;
			}
		}
		Channel_for_radar = Rsp.data.operating_ch_info.channel;
	}
#endif

	if (!DfsRddReportHandle(pAd, pDfsParam, ucRddIdx, BandIdx))
		goto end;

	/* By pass these setting when dedicated DFS zero wait is enabled and radar is detected on out-band */
	if (((pDfsParam->bDedicatedZeroWaitSupport == TRUE
	&& hc_get_hw_band_idx(pAd) == pDfsParam->ZeroWaitBandidx)
	|| pDfsParam->bOcacEnable != 0)
	&& (pDfsParam->RadarDetected[RDD_DEDICATED_IDX] == TRUE))
		;
	else {
		if (!dot11h_param) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "Dereferencing NULL pointer\n");
			goto end;
		}
		if (dot11h_param->ChannelMode == CHAN_SILENCE_MODE)
			dot11h_param->RDCount = 0;
	}

	WrapDfsSetNonOccupancy(pAd, ucRddIdx, BandIdx);

	if (pDfsParam->bOcacEnable == 1) {
		pDfsParam->RadarDetected[RDD_DEDICATED_IDX] = FALSE;
		goto end;
	}
#ifdef BACKGROUND_SCAN_SUPPORT
	/* Choose another channel for out-band */
	if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE
	&& hc_get_hw_band_idx(pAd) == pDfsParam->ZeroWaitBandidx
	&& pDfsParam->BW160ZeroWaitSupport == FALSE)
	|| pDfsParam->bOcacEnable == 2) {
		/* Clean set channel flag to default */
		pDfsParam->SetOutBandChStat = OUTB_SET_CH_DEFAULT;

		if (pDfsParam->RadarDetected[RDD_DEDICATED_IDX] == TRUE) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				"RDD[%d] detect. Please switch to another outBand channel\n", RDD_DEDICATED_IDX);
			ZeroWait_DFS_collision_report(pAd, RDD_DEDICATED_IDX, pDfsParam->OutBandCh, pDfsParam->OutBandBw);

			if (pDfsParam->bDedicatedZeroWaitDefault) {

#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
				P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
				/* Change channel state */
				switch (*ch_stat) {
				case DFS_INB_CH_SWITCH_CH:
					/* radar detected during in-band ch switch */
					*ch_stat = DFS_INB_CH_INIT;
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "ch_stat %d\n", *ch_stat);
					break;

				default:
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "ch_stat %d\n", *ch_stat);
					MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_OUTBAND_RADAR_FOUND, 0, NULL, 0);
					RTMP_MLME_HANDLER(pAd);
					break;
				}
#else
				MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_OUTBAND_RADAR_FOUND, 0, NULL, 0);
				RTMP_MLME_HANDLER(pAd);
#endif
			} else if (pDfsParam->bOcacEnable == 2) {
				MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_OUTBAND_RADAR_FOUND, 0, NULL, 0);
				RTMP_MLME_HANDLER(pAd);
			}

			pDfsParam->RadarDetected[RDD_DEDICATED_IDX] = FALSE;
#ifdef DFS_CAC_R2
			if (first_wdev && IS_MAP_TURNKEY_ENABLE(pAd)) {
				wapp_send_radar_detect_notif(pAd, wdev, Channel_for_radar, bw_for_radar, 0);
				first_wdev = FALSE;
			}
#endif
			goto end;
		} else if ((pDfsParam->RadarDetected[ucRddIdx] == TRUE) && GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				"RDD[%d] detect. OutBand channel come back to InBand\n", ucRddIdx);

			pDfsParam->OrigInBandCh = pDfsParam->band_ch;
			pDfsParam->OrigInBandBw = pDfsParam->band_bw;
			pDfsParam->RadarHitReport = TRUE;
		}
	}
#endif

#ifdef CONFIG_RCSA_SUPPORT
	if (pDfsParam->RadarDetected[ucRddIdx] == TRUE)
		pDfsParam->fSendRCSA = TRUE;
#endif

	/* Keep BW info because the BW may be changed after selecting a new channel */
	KeepBw = pDfsParam->band_bw;
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	if (IS_SUPPORT_V10_DFS(pAd) && (pDfsParam->RadarDetected[ucRddIdx] == TRUE)) {
		BwChannel = DfsV10SelectBestChannel(pAd, HcGetRadioChannel(pAd), band_idx);
		/* AP BCN Update for ACS Case */
		if (IS_V10_AP_BCN_UPDATE_ENBL(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Beacon Update\n");

			MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_V10_ACS_CSA_UPDATE, sizeof(UCHAR), &band_idx, 0);
			goto end;
		}

		/* W56 Channel Exhausted : Ap Down for 30 Minutes */
		if (!BwChannel && IS_V10_W56_AP_DOWN_ENBLE(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "AP Down %ld\n", pDfsParam->gV10W56TrgrApDownTime);
			SET_V10_W56_AP_DOWN(pAd, FALSE);

			pDfsParam->DfsChBand[0] = FALSE;
			pDfsParam->DfsChBand[1] = FALSE;
			pDfsParam->RadarDetected[0] = FALSE;
			pDfsParam->RadarDetected[1] = FALSE;
			goto end;
		}
			pDfsParam->PrimBand = RDD_BAND0;
			pDfsParam->OutBandCh = pDfsParam->PrimCh = BwChannel & 0xFF;
			pDfsParam->OutBandBw = BwChannel >> 8;
	} else {
		WrapDfsSelectChannel(pAd, ucRddIdx);
	}
#else
	WrapDfsSelectChannel(pAd, ucRddIdx);
#endif
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
	"[RDM]PrimCh: %d, OutBandCh:%d, band_ch:%d\n"
	, pDfsParam->PrimCh, pDfsParam->OutBandCh, pDfsParam->band_ch);


	/* Normal DFS uniform Ch */
	NextCh = pDfsParam->PrimCh;
	for (BssIdx1 = 0; BssIdx1 < pAd->ApCfg.BssidNum; BssIdx1++) {
		SetChInfo = 0;
		pMbss = &pAd->ApCfg.MBSSID[BssIdx1];
		wdev = &pMbss->wdev;

		if (wdev->pHObj == NULL)
			continue;
		/*Need choose RadioOn interface, before enq cmd to set channel*/
		if (IsHcRadioCurStatOffByWdev(wdev))
			continue;

		if (HcGetBandByWdev(wdev) != BandIdx)
			continue;

			/* Adjust Bw */
#ifdef BACKGROUND_SCAN_SUPPORT
		if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE
		&& hc_get_hw_band_idx(pAd) == pDfsParam->ZeroWaitBandidx)
		&& GET_BGND_STATE(pAd, BGND_RDD_DETEC)
		&& !(IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd))
		&& pDfsParam->BW160ZeroWaitSupport == FALSE) {
			if (pDfsParam->bPreCacEn)
				pDfsParam->OutBandBw = wlan_operate_get_bw(wdev);

			DfsAdjustOpBwSetting(pAd, wdev, pDfsParam->band_bw, pDfsParam->OutBandBw);
			if (pDfsParam->OutBandBw == BW_160) {
				pDfsParam->band_bw = BW_80;
				pDfsParam->ZwAdjBw = BW_80;
				NextBw = BW_80;
			} else
				NextBw = pDfsParam->OutBandBw;
		} else {
			if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {
				if (pDfsParam->band_bw == BW_160) {
					pDfsParam->MapZwFlag = TRUE;
					pDfsParam->band_bw = BW_80;
				}
			}
			DfsAdjustOpBwSetting(pAd, wdev, KeepBw, pDfsParam->band_bw);
			NextBw = pDfsParam->band_bw;
		}
#endif /* BACKGROUND_SCAN_SUPPORT */

#ifdef DFS_ADJ_BW_ZERO_WAIT
		if (pDfsParam->BW160ZeroWaitSupport == TRUE
		&& IS_ADJ_BW_ZERO_WAIT_RDD_DETECT(pDfsParam->BW160ZeroWaitState)) {
			pDfsParam->BW160ZeroWaitState = DFS_RADAR_DETECTED;
			if ((IS_CH_BETWEEN(wdev->channel, 36, 64))
			|| (IS_CH_BETWEEN(wdev->channel, 100, 128) && pDfsParam->band_bw == BW_160)) {

				if (IS_CH_BETWEEN(pDfsParam->band_ch, 36, 64))
					pDfsParam->band_orich = pDfsParam->band_ch;
				else
					pDfsParam->band_orich = 36;

				NextCh = 36;
				pDfsParam->band_ch = 36;
				dot11h_param->ChannelMode = CHAN_NORMAL_MODE;
				NextBw = BW_80;
				if (IS_CH_BETWEEN(wdev->channel, 100, 128))
					pDfsParam->BW160ZeroWaitState = DFS_BW160_TX80RX160;
				else if (IS_CH_BETWEEN(wdev->channel, 36, 64))
					pDfsParam->BW160ZeroWaitState = DFS_BW80_TX80RX80;
				else if (pDfsParam->band_bw == BW_160)
					pDfsParam->BW160ZeroWaitState = DFS_BW160_TX80RX80;
				else if (pDfsParam->band_bw == BW_80)
					pDfsParam->BW160ZeroWaitState = DFS_BW80_TX80RX80;

				DfsAdjustOpBwSetting(pAd, wdev, pDfsParam->band_bw, NextBw);
				pDfsParam->band_bw = NextBw;
			} else if (pDfsParam->band_bw == BW_160)
				pDfsParam->BW160ZeroWaitState = DFS_BW160_TX0RX0;
			else if (pDfsParam->band_bw == BW_80)
				pDfsParam->BW160ZeroWaitState = DFS_BW80_TX0RX0;

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				"[RDM]\x1b[1;33m CHAN_NORMAL_MODE. Update Uniform Ch=%d, BW=%d, band_orich=%d \x1b[m\n",
				pDfsParam->band_ch, pDfsParam->band_bw, pDfsParam->band_orich);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				"RDD[%d] detect. SW-based ZeroWait DFS\n", ucRddIdx);
		}
#endif /* DFS_ADJ_BW_ZERO_WAIT */
	}

	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdev = &pMbss->wdev;

		if (wdev->pHObj == NULL)
			continue;
		/*Need choose RadioOn interface, before enq cmd to set channel*/
		if (IsHcRadioCurStatOffByWdev(wdev))
			continue;

		if (HcGetBandByWdev(wdev) == BandIdx)
			break;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "[RDM]Update wdev of BssIdx %d\n", BssIdx);

	if (BssIdx >= pAd->ApCfg.BssidNum) {
		/* this condition can occur in following two cases */
		/* case 1 : All 5G AP inf are down and apcli inf up */
		/* case 2 : "BandIdx = 2" for RDD on dedicated rx */
		wdev = NULL;
#ifdef DFS_SLAVE_SUPPORT
		if (SLAVE_MODE_EN(pAd)) {
			for (i = 0; i < MAX_APCLI_NUM; i++) {
				sta_wdev = &pAd->StaCfg[i].wdev;
				if ((HcGetBandByWdev(sta_wdev) == BandIdx) &&
					sta_wdev->if_up_down_state) {
					wdev = sta_wdev;
					break;
				}
			}
		}
#endif /* DFS_SLAVE_SUPPORT */
	}

	if (NextBw < BW_160 && (IS_CH_BETWEEN(NextCh, 36, 48)))
		pAd->CommonCfg.DfsParameter.CacCtrl = NO_NEED_CAC;

	if (wdev) {
		SetChInfo = 0;
#ifdef CONFIG_MAP_SUPPORT
/*On radar detect let AP stop start happen without apcli disconnect at AP stop*/
/*Link down only after sending the radar detect notification*/
		if (IS_MAP_ENABLE(pAd)) {
			wdev->map_radar_detect = 2;
			wdev->map_radar_channel = wdev->channel;
			wdev->map_radar_bw = wlan_operate_get_bw(wdev);
#ifdef CONFIG_STA_SUPPORT
			for (i = 0; i < MAX_APCLI_NUM; i++) {
				sta_wdev = &pAd->StaCfg[i].wdev;
				if(wdev->quick_ch_change != QUICK_CH_SWICH_DISABLE)
					wapp_send_radar_detect_notif(pAd, wdev, wdev->map_radar_channel, wdev->map_radar_bw, 0);
				if (sta_wdev->channel == wdev->channel)
					pAd->StaCfg[i].ApcliInfStat.Enable = FALSE;
			}
#endif /* CONFIG_STA_SUPPORT */
		}
#endif

		if (dot11h_param->ChannelMode == CHAN_NORMAL_MODE) {
			pDfsParam->DfsChBand[ucRddIdx] = FALSE;
			pDfsParam->RadarDetected[ucRddIdx] = FALSE;

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "[RDM]\x1b[1;33m Normal Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
					 NextCh,
					 NextBw);

			SetChInfo |= NextCh;
			SetChInfo |= (BssIdx << 8);
			SetChInfo |= (BandIdx << 16);

#ifdef DFS_SLAVE_SUPPORT
			if (SLAVE_MODE_EN(pAd) && (wdev->wdev_type == WDEV_TYPE_STA)) {
				/* clear bssidx info and update staidx */
				SetChInfo &= 0xffff00ff; /* clear bit8-15 */
				SetChInfo |= (wdev->func_idx << 8);
				SetChInfo |= (1 << 24); /* use bit24 to indicate wdev sta */
			}
#endif /* DFS_SLAVE_SUPPORT */
			ret = RTEnqueueInternalCmd(pAd, CMDTHRED_DFS_RADAR_DETECTED_SW_CH, &SetChInfo, sizeof(UINT_32));
			/* Enqueue success, CHAN_NORMAL_MODE will do CSA, no need release ChOpCharge here */
			if (ret == NDIS_STATUS_SUCCESS) {
				RTMP_MLME_HANDLER(pAd);
#ifdef TR181_SUPPORT
				/*increase radio channel change count due to radar detection*/
				/*todo: find rdev using api, instead of direct access*/
				ctrl->rdev.pRadioCtrl->DFSTriggeredChannelChangeCount++;
				ctrl->rdev.pRadioCtrl->TotalChannelChangeCount++;
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "[RDM] channel changed for Band[%d]\n", BandIdx);
#endif /*TR181_SUPPORT*/
				return;
			} else {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "[RDM] Enqueue DFsSwitchCH cmd fail for Band[%d]\n", BandIdx);
				goto end;
			}
		} else if (dot11h_param->ChannelMode == CHAN_SILENCE_MODE) {
			pDfsParam->DfsChBand[ucRddIdx] = FALSE;
			pDfsParam->RadarDetected[ucRddIdx] = FALSE;

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "\x1b[1;33m [RDM]Silence Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
					 NextCh,
					 NextBw);

			SetChInfo |= NextCh;
			SetChInfo |= (BssIdx << 8);
			SetChInfo |= (BandIdx << 16);

#ifdef DFS_SLAVE_SUPPORT
			if (SLAVE_MODE_EN(pAd) && (wdev->wdev_type == WDEV_TYPE_STA)) {
				/* clear bssidx info and update staidx */
				SetChInfo &= 0xffff00ff; /* clear bit8-15 */
				SetChInfo |= (wdev->func_idx << 8);
				SetChInfo |= (1 << 24); /* use bit24 to indicate wdev sta */
				}
#endif /* DFS_SLAVE_SUPPORT */
			ret = RTEnqueueInternalCmd(pAd, CMDTHRED_DFS_CAC_TIMEOUT, &SetChInfo, sizeof(UINT_32));
			/* Enqueue success, CHAN_SILENCE_MODE will not do CSA, release in TimeOutHandler */
			if (ret == NDIS_STATUS_SUCCESS) {
				RTMP_MLME_HANDLER(pAd);
#ifdef TR181_SUPPORT
				/*increase radio channel change count due to radar detection*/
				/*todo: find rdev using api, instead of direct access*/
				ctrl->rdev.pRadioCtrl->DFSTriggeredChannelChangeCount++;
				ctrl->rdev.pRadioCtrl->TotalChannelChangeCount++;
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "[RDM]channel changed for band[%d]\n", BandIdx);
#endif /*TR181_SUPPORT*/
				return;
			} else {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "[RDM]Enqueue DFS CAC Timeout cmd fail on band[%d]\n", BandIdx);
				goto end;
			}
		}
	}

end:
#if (RDD_2_SUPPORTED == 1)
	if (ucRddIdx != RDD_DEDICATED_IDX)
#endif
		RTEnqueueInternalCmd(pAd, CMDTHREAD_DROP_RADAR_EVENT, &BandIdx, sizeof(UCHAR));
	/*just release ChannelOpCharge before returning*/
	ReleaseChannelOpChargeByBand(pAd, BandIdx, CH_OP_OWNER_DFS);

	return;
}
#endif

BOOLEAN DfsRddReportHandle(/*handle the event of UNI_EVENT_ID_RDD*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam, UCHAR rddidx, UCHAR BandIdx)
{
	BOOLEAN RadarDetected = FALSE;
	UCHAR BssIdx;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	BOOLEAN RadarChannel = FALSE;
	struct wlan_config *cfg = NULL;
	UCHAR phy_bw = 0;

	switch (rddidx) {
	case RDD_INBAND_IDX_1:
	case RDD_INBAND_IDX_2:
		/* Radar is detected by RDD0 or RDD1 */
		if ((pDfsParam->RadarDetected[rddidx] == FALSE) &&
			(pDfsParam->DfsChBand[rddidx]) &&
			(pDfsParam->Dot11_H.ChannelMode != CHAN_SWITCHING_MODE)) {

			pDfsParam->RadarDetected[rddidx] = TRUE;
			RadarDetected = TRUE;
#ifdef ZERO_PKT_LOSS_SUPPORT
			pDfsParam->ZeroLossRadarDetect = TRUE;
#endif
		}

		for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
			pMbss = &pAd->ApCfg.MBSSID[BssIdx];
			wdev = &pMbss->wdev;
			if (wdev->pHObj == NULL)
				continue;

			cfg = (struct wlan_config *)wdev->wpf_cfg;
			if (cfg == NULL)
				continue;

			if (HcGetBandByWdev(wdev) != BandIdx)
				continue;

			if (cfg->ht_conf.ht_bw == HT_BW_20)
				phy_bw = BW_20;
			else if (cfg->ht_conf.ht_bw == HT_BW_40) {
				if (cfg->vht_conf.vht_bw == VHT_BW_2040)
					phy_bw = BW_40;
				else if (cfg->vht_conf.vht_bw == VHT_BW_80)
					phy_bw = BW_80;
				else if (cfg->vht_conf.vht_bw == VHT_BW_160)
					phy_bw = BW_160;
				else
					;
			}

			if (DfsRadarChannelCheck(pAd, wdev, cfg->phy_conf.cen_ch_2, phy_bw)) {
				RadarChannel = TRUE;
				break;
			}
		}

#ifdef DFS_SLAVE_SUPPORT
		if (SLAVE_MODE_EN(pAd) && (RadarChannel == FALSE)) {
			UCHAR sta_idx = 0;

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				"[RDM][DFS-SLAVE] STA checking RDD\n");
			for (sta_idx = 0; sta_idx < MAX_APCLI_NUM; sta_idx++) {
				wdev = &pAd->StaCfg[sta_idx].wdev;

				if (wdev->pHObj == NULL)
					continue;

				cfg = (struct wlan_config *)wdev->wpf_cfg;
				if (cfg == NULL)
					continue;

				if (HcGetBandByWdev(wdev) != BandIdx)
					continue;

				if (cfg->ht_conf.ht_bw == HT_BW_20)
					phy_bw = BW_20;
				else if (cfg->ht_conf.ht_bw == HT_BW_40) {
					if (cfg->vht_conf.vht_bw == VHT_BW_2040)
						phy_bw = BW_40;
					else if (cfg->vht_conf.vht_bw == VHT_BW_80)
						phy_bw = BW_80;
					else if (cfg->vht_conf.vht_bw == VHT_BW_160)
						phy_bw = BW_160;
					else
						;
				}

				if (DfsRadarChannelCheck(pAd, wdev, cfg->phy_conf.cen_ch_2, phy_bw)) {
					RadarChannel = TRUE;
					break;
				}
			}
		}
#endif

		if (RadarChannel == FALSE) {
			RadarDetected = FALSE;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "[RDM]: No wdev work on Radar Channel!\n");
		}

		break;

#if (RDD_2_SUPPORTED == 1)
	case RDD_DEDICATED_IDX:
		/* Radar is detected by dedicated RX */
		if (((
#ifdef DFS_SDB_SUPPORT
		(pDfsParam->bDfsSdbEnable == TRUE || pDfsParam->bDedicatedZeroWaitSupport == TRUE)
#else
		pDfsParam->bDedicatedZeroWaitSupport == TRUE
#endif /* DFS_SDB_SUPPORT */
		&& hc_get_hw_band_idx(pAd) == pDfsParam->ZeroWaitBandidx)
		|| pDfsParam->bOcacEnable != 0)
		&& pDfsParam->RadarDetected[rddidx] == FALSE
		&& pDfsParam->DfsChBand[rddidx]) {
			pDfsParam->RadarDetected[rddidx] = TRUE;
			RadarDetected = TRUE;
		}
		break;
#endif

	default:
		break;
	}
	return RadarDetected;
}

VOID WrapDfsSelectChannel(/*Select new channel*/
	IN PRTMP_ADAPTER pAd, UCHAR ucRddIdx)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	DfsSelectChannel(pAd, pDfsParam, ucRddIdx);

#ifdef DOT11_VHT_AC
	if (ucRddIdx >= RDD_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			 "[RDM]:Invalid ucRddIdx(%d)\n", ucRddIdx);
		return;
	}

#endif
}

VOID DfsSelectChannel(/*Select new channel*/
	IN PRTMP_ADAPTER pAd,
	IN PDFS_PARAM pDfsParam,
	IN UCHAR ucRddIdx)
{
#ifdef BACKGROUND_SCAN_SUPPORT
	UCHAR tempCh = 0;
#endif /* BACKGROUND_SCAN_SUPPORT */
	UCHAR idx;
	UCHAR BandIdx = 0;

	for (idx = 0; idx < RDD_BAND_NUM; idx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				"[RDM]: RadarDetected[%d]=%d, pDfsParam->DfsChBand[%d]=%d\n",
				 idx,
				 pDfsParam->RadarDetected[idx],
				 idx,
				 pDfsParam->DfsChBand[idx]);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				"[RDM]: OutBand BW=%d, InBand BW=%d\n",
				 pDfsParam->OutBandBw,
				 pDfsParam->band_bw);

	}

	if (ucRddIdx >= RDD_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			 "Invalid ucRddIdx(%d)\n", ucRddIdx);
		return;
	}

	if (pDfsParam->RadarDetected[ucRddIdx] && pDfsParam->DfsChBand[ucRddIdx]) {
#ifdef BACKGROUND_SCAN_SUPPORT
		CHANNEL_CTRL *pChCtrl;

		if (((pDfsParam->bDedicatedZeroWaitSupport == TRUE
		&& hc_get_hw_band_idx(pAd) == pDfsParam->ZeroWaitBandidx)
		|| pDfsParam->bOcacEnable == 2) && GET_BGND_STATE(pAd, BGND_RDD_DETEC)
		&& (pDfsParam->BW160ZeroWaitSupport == FALSE)) {
			if (pDfsParam->band_bw == BW_160) {
				tempCh = WrapDfsRandomSelectChannel(pAd, 0, ucRddIdx);
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				"[RDM]: BW160 tempCh selected is %d\n", tempCh);
			} else {
				tempCh = WrapDfsRandomSelectChannel(pAd, 0, ucRddIdx);
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
					"[RDM]: tempCh selected is %d\n", tempCh);
			}

			if (tempCh == 0 && ucRddIdx == RDD_INBAND_IDX_1)
				tempCh = DfsGetNonDfsDefaultCh(pAd, pDfsParam);

			if (RadarChBwCheck(pAd, tempCh, pDfsParam->band_bw)) {
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
				P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;

				BandIdx = hc_get_hw_band_idx(pAd);
				pChCtrl = DfsGetChCtrl(pAd);

				if (!pDfsParam->bDfsDedicatedRxPreselectCh && pDfsParam->bDedicatedZeroWaitSupport
					&& pDfsParam->bDedicatedZeroWaitDefault && (*ch_stat == DFS_INB_DFS_OUTB_CH_CAC_DONE))
					*ch_stat = DFS_INB_DFS_OUTB_CH_CAC;

				switch (*ch_stat) {
				case DFS_INB_DFS_OUTB_CH_CAC_DONE:
					/* zero-wait CAC of out-band is ended */
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
						"[RDM]: Out-band CAC is ended, ch_stat %d\n",
						*ch_stat);

					/* If DFS channel is selected randomly by SynA, */
					/* SynA will use the DFS channel of SynB*/
					pDfsParam->band_ch = DfsGetNonDfsDefaultCh(pAd, pDfsParam);

					if (pDfsParam->band_ch == pDfsParam->OutBandCh) {
						*ch_stat = DFS_INB_DFS_RADAR_OUTB_CAC_DONE_QUICK;
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
							"Directly switch outband channel to inband\n");
					} else {
						*ch_stat = DFS_INB_DFS_RADAR_OUTB_CAC_DONE;
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
							"RDD[%d] detect. OutBand channel %d will be set to InBand\n", ucRddIdx, pDfsParam->OutBandCh);
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
							"[RDM]: ch_stat %d\n", *ch_stat);
					}
					break;

				case DFS_INB_DFS_OUTB_CH_CAC:
						/* radar is detected on in-band ch and out-band CAC is not ended */
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
						"[RDM]: Out-band CAC is not ended, ch_stat %d\n",
						*ch_stat);
					*ch_stat = DFS_INB_CH_INIT;

					if (!pDfsParam->bDfsDedicatedRxPreselectCh && pDfsParam->bDedicatedZeroWaitSupport
						&& pDfsParam->bDedicatedZeroWaitDefault)
						pDfsParam->OutBandCh = tempCh;

					pDfsParam->band_ch = pDfsParam->OutBandCh;
					break;

				default:
					if (pDfsParam->bDedicatedZeroWaitDefault == FALSE) {
						pDfsParam->band_ch = pDfsParam->OutBandCh;
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
							"RDD[%d] detect. zw DFS is not enabled\n", ucRddIdx);
					}
#ifdef CONFIG_MAP_SUPPORT
					if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {
						pDfsParam->band_ch = DfsGetNonDfsDefaultCh(pAd, pDfsParam);
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
						"[RDM]: Mesh enable, Change InBand Channel to: %d, Current BW: %d\n", pDfsParam->band_ch, pDfsParam->band_bw);
					}
#endif
					break;
				}
#else
					/* If DFS channel is selected randomly by SynA, SynA will use the DFS channel of SynB*/
				pDfsParam->band_ch = pDfsParam->OutBandCh;
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
					"RDD[%d] detect. OutBand channel come back to InBand\n", ucRddIdx);
#endif
			} else {
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
				P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
				*ch_stat = DFS_INB_CH_INIT;
				pAd->CommonCfg.DfsParameter.OutBandCh = 0;
#endif /* DFS_ZEROWAIT_DEFAULT_FLOW */

				pDfsParam->OutBandBw = pDfsParam->band_bw;

				pDfsParam->band_ch = tempCh;
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
					"RDD[%d] detect. InBand channel is switched to another non-DFS channel randomly\n",
					ucRddIdx);
			}
		} else
#endif
		{
#ifdef DFS_ADJ_BW_ZERO_WAIT
			if (pDfsParam->BW160ZeroWaitSupport == TRUE)
				pDfsParam->band_ch = WrapDfsRandomSelectChannel(pAd, 0, ucRddIdx);
			else
#endif
			{
				pDfsParam->band_ch = WrapDfsRandomSelectChannel(pAd, pDfsParam->OutBandCh, ucRddIdx);
			}
		}

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"[RDM]: ucRddIdx:%d, BandIdx: %d, InBand selected is %d\n",
			ucRddIdx,
			BandIdx,
			pDfsParam->band_ch);
	}
	pDfsParam->PrimCh = pDfsParam->band_ch;
	pDfsParam->PrimBand = BandIdx;

}

UCHAR WrapDfsRandomSelectChannel(/*Select new channel using random selection*/
	IN PRTMP_ADAPTER pAd, UCHAR avoidedCh, UCHAR ucRddIdx)
{
	PDFS_PARAM	pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR		Channel;

	if (pDfsParam->targetCh != 0) {
		if ((pDfsParam->targetCh != pDfsParam->band_ch)
		|| (pDfsParam->targetBw != pDfsParam->band_bw)) {
			pDfsParam->band_bw = pDfsParam->targetBw;
			Channel = pDfsParam->targetCh;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "Channel:%d, pDfsParam->targetCh:%d\n ", Channel, pDfsParam->targetCh);

#ifdef WIFI_MD_COEX_SUPPORT
			if (!IsChannelSafe(pAd, Channel)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				 "Target Channel %d is in unsafe channel list!\n ", Channel);
				Channel = DfsRandomSelectChannel(pAd, pDfsParam, avoidedCh, ucRddIdx);
			}
#endif

			return Channel;
		} else {
			pDfsParam->targetCh = 0;
			pDfsParam->targetBw = 0;
			pDfsParam->targetCacValue = 0;
		}
	}
	return DfsRandomSelectChannel(pAd, pDfsParam, avoidedCh, ucRddIdx);
}

static BOOLEAN DfsCheckChPriorityByCh(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel)
{
	UINT8 chIdx;
	PCHANNEL_CTRL pChCtrl = NULL;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (chIdx = 0; chIdx < pChCtrl->ChListNum; chIdx++) {
		if (pChCtrl->ChList[chIdx].Channel == Channel &&
			pChCtrl->ChList[chIdx].Priority > 0)
			return TRUE;
	}

	return FALSE;
}

UCHAR DfsRandomSelectChannel(/*Select new channel using random selection*/
	IN PRTMP_ADAPTER pAd,
	IN PDFS_PARAM pDfsParam,
	IN UCHAR avoidedCh,
	IN UCHAR ucRddIdx)
{
	UINT_8 i, cnt, ch;
	UCHAR band_bw;
	UCHAR config_bw;
	UCHAR BandIdx = 0;
#ifdef WIFI_MD_COEX_SUPPORT
	UCHAR safeChnCnt = 0, pwrChnCnt = 0;
	UINT_8 TempSafeChList[MAX_NUM_OF_CHANNELS] = {0};
	UINT_8 TempPwrChList[MAX_NUM_OF_CHANNELS] = {0};
#endif
	UINT_8 TempChList[MAX_NUM_OF_CHANNELS] = {0};
	UINT_8 TempChPrioirtyList[MAX_NUM_OF_CHANNELS] = {0};
	UINT_8 NonDFSChList[MAX_NUM_OF_CHANNELS] = {0};

	PCHANNEL_CTRL pChCtrl = NULL;
	USHORT PhyMode = 0;
	UCHAR DfsChSelPrefer = pDfsParam->DfsChSelPrefer;
	UCHAR BssIdx;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	struct Chan_Config *pChCfg = NULL;

	cnt = 0;
	BandIdx = hc_get_hw_band_idx(pAd);
	pChCtrl = DfsGetChCtrl(pAd);
	PhyMode = HcGetRadioPhyModeByBandIdx(pAd);

	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdev = &pMbss->wdev;

		if (wdev->pHObj == NULL)
			continue;
		/*Need choose RadioOn interface, before enq cmd to set channel*/
		if (IsHcRadioCurStatOffByWdev(wdev))
			continue;

		if (HcGetBandByWdev(wdev) == BandIdx)
			break;
	}

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"wdev is null!\n");
		return FALSE;
	}

	if ((pChCtrl->ChListNum > MAX_NUM_OF_CHANNELS) || (pChCtrl->ChListNum <= 0)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			 "Incorrect ChListNum(%d)\n", pChCtrl->ChListNum);
		return FALSE;
	}

	if (!(pDfsParam->bIEEE80211H) || (wlan_config_get_ch_band(wdev) != CMD_CH_BAND_5G)) {
#ifdef WIFI_MD_COEX_SUPPORT
		for (i = 0; i < pChCtrl->ChListNum; i++) {
#ifdef CONFIG_AP_SUPPORT
			if (AutoChannelSkipListCheck(pAd, pChCtrl->ChList[i].Channel) == TRUE)
				continue;
#endif
			if (IsChannelSafe(pAd, pChCtrl->ChList[i].Channel))
				TempSafeChList[safeChnCnt++] = pChCtrl->ChList[i].Channel;
			else {
				if (IsPwrChannelSafe(pAd, pChCtrl->ChList[i].Channel))
					TempPwrChList[pwrChnCnt++] = pChCtrl->ChList[i].Channel;
				else
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
						 "The channel %d is in unsafe channel list!!\n ", pChCtrl->ChList[i].Channel);
			}
		}

		if (safeChnCnt) {
			ch = TempSafeChList[(UINT_8)(RandomByte(pAd) % safeChnCnt)];
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				 "select free run channel %d\n", ch);
		} else if (pwrChnCnt) {
			/* If the current channel is pwr backoff channel and
			 * there is no safe channel, stay in the current channel
			 */
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				 "No free run channel\n");
			if (!IsPwrChannelSafe(pAd, wdev->channel)) {
				ch = TempPwrChList[(UINT_8)(RandomByte(pAd) % pwrChnCnt)];
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
					 "select power backoff channel %d\n", ch);
			} else
				ch = 0;
		} else {
			ch =  0;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				 "No available channel to use, return 0\n");
		}
		/* Don't care IEEE80211 disable when bSkipDfsCh is FALSE */
		return ch;
#else
		ch = pChCtrl->ChList[(UINT_8)(RandomByte(pAd) % pChCtrl->ChListNum)].Channel;

		if (ch == 0)
			ch = pChCtrl->ChList[0].Channel;

		/* Don't care IEEE80211 disable when bSkipDfsCh is FALSE */
		return ch;
#endif
	}

	config_bw = wlan_config_get_bw(wdev);
	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (!ByPassChannelByBw(pChCtrl->ChList[i].Channel, config_bw, pChCtrl))
			pDfsParam->band_bw = config_bw;
	}

	for (i = 0; i < pChCtrl->ChListNum; i++) {

		if (pChCtrl->ChList[i].NonOccupancy || pChCtrl->ChList[i].NOPSaveForClear)
			continue;

#ifdef CONFIG_AP_SUPPORT
		if (AutoChannelSkipListCheck(pAd, pChCtrl->ChList[i].Channel) == TRUE)
			continue;
#endif

		if (!IsChABand(PhyMode, pChCtrl->ChList[i].Channel))
			continue;

		/* Skip DFS channel for DFS using case */
		if (DfsChSelPrefer == RadarDetectSelectNonDFS) {
			if (RadarChannelCheck(pAd, pChCtrl->ChList[i].Channel))
				continue;
		}
		/* Skip non-DFS channel for DFS using case */
		if (DfsChSelPrefer == RadarDetectSelectDFS
			|| DfsChSelPrefer == RadarDetectSelectWidestBWDFS) {
			if (!RadarChannelCheck(pAd, pChCtrl->ChList[i].Channel))
				continue;
		}

		if (ByPassChannelByBw(pChCtrl->ChList[i].Channel, pDfsParam->band_bw, pChCtrl))
			continue;


		/* BW160 and radar hit at channel 52 ~ 64 */
		if (pDfsParam->band_bw == BW_160 && (pChCtrl->ChList[i].Channel >= 36 && pChCtrl->ChList[i].Channel <= 48)
			&& (pChCtrl->ChList[4].NonOccupancy))
			continue;

		/* 5G + 5G case */
		if ((avoidedCh != 0) &&
			(pAd->CommonCfg.dbdc_mode == TRUE) &&
			DfsPrimToCent(pChCtrl->ChList[i].Channel, pDfsParam->band_bw) == DfsPrimToCent(avoidedCh, pDfsParam->band_bw))
			continue;

		if (!DfsDedicatedCheckChBwValid(pAd, pChCtrl->ChList[i].Channel, pDfsParam->band_bw, BandIdx))
			continue;

		/* Store available channel to temp list */
		TempChList[cnt] = pChCtrl->ChList[i].Channel;
		TempChPrioirtyList[cnt] = pChCtrl->ChList[i].Priority;
		cnt++;
	}

	if (pChCtrl->ch_cfg.chan_prio_valid) {
		/* priority 0 channel won't be selected, if channel priority valid */
		UCHAR max_prio = 1;
		UCHAR new_cnt = 0;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
			"Channel priority is valid\n");

		for (i = 0; i < cnt; i++)
			max_prio = TempChPrioirtyList[i] > max_prio ? TempChPrioirtyList[i] : max_prio;

		for (i = 0; i < cnt; i++) {
			if (TempChPrioirtyList[i] == max_prio) {
				TempChList[new_cnt] = TempChList[i];
				TempChPrioirtyList[new_cnt] = TempChPrioirtyList[i];
				new_cnt++;
			}
		}

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
				"Filter out channel by maximum priority(%d), cnt(%d) to new cnt(%d)\n",
				max_prio, cnt, new_cnt);
		cnt = new_cnt;
	}

	if (DfsChSelPrefer == RadarDetectSelectWidestBWDFS) {
		band_bw = pDfsParam->band_bw;
		/* Find non-DFS Channel with widest BW */
		while (!cnt && band_bw <= BW_160) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				 "Find non-DFS Channel with bw: %d\n", band_bw);
			for (i = 0; i < pChCtrl->ChListNum; i++) {
				if (RadarChannelCheck(pAd, pChCtrl->ChList[i].Channel))
					continue;

				if (ByPassChannelByBw(pChCtrl->ChList[i].Channel, band_bw, pChCtrl))
					continue;

				TempChList[cnt++] = pChCtrl->ChList[i].Channel;
			}
			band_bw -= 1;
		}
	}

#ifdef WIFI_MD_COEX_SUPPORT
	if (cnt) {
		for (i = 0; i < cnt; i++) {
			if (IsChannelSafe(pAd, TempChList[i]))
				TempSafeChList[safeChnCnt++] = TempChList[i];
			else {
				if (IsPwrChannelSafe(pAd, TempChList[i]))
					TempPwrChList[pwrChnCnt++] = TempChList[i];
				else
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
						 "The channel %d is in unsafe channel list!!\n ", pChCtrl->ChList[i].Channel);
			}
		}

		if (safeChnCnt)
			ch = TempSafeChList[(UINT_8)(RandomByte(pAd) % safeChnCnt)];
		else if (pwrChnCnt) {
			ch = TempPwrChList[(UINT_8)(RandomByte(pAd) % pwrChnCnt)];
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				 "select power backoff channel %d\n", ch);
		}
	}
#else
	if (cnt)
		ch = TempChList[(UINT_8)(RandomByte(pAd) % cnt)];
#endif
	else if (DfsChSelPrefer == RadarDetectSelectNonDFS) {
		if (pDfsParam->band_bw == BW_160 && pDfsParam->RadarDetected[ucRddIdx]) {
			UINT_8 nonDFS_cnt = 0;

			pDfsParam->band_bw = BW_80;
			DfsAdjustOpBwSetting(pAd, wdev, BW_160, BW_80);
			for (i = 0; i < pChCtrl->ChListNum; i++) {
				if ((IS_CH_BETWEEN(pChCtrl->ChList[i].Channel, 36, 48))
				|| (IS_CH_BETWEEN(pChCtrl->ChList[i].Channel, 149, 165))) {
					NonDFSChList[nonDFS_cnt] = pChCtrl->ChList[i].Channel;
					nonDFS_cnt++;
				}
			}
			if (nonDFS_cnt)
				ch = NonDFSChList[(UINT_8)(RandomByte(pAd) % nonDFS_cnt)];
			else
				ch = FirstNonDfsChannel(pAd);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				 "Select Non-DFS channel %d, %s\n ", ch, __func__);
		} else if (pDfsParam->RadarDetected[ucRddIdx] && pDfsParam->DfsChBand[ucRddIdx]) {
			ch = FirstNonDfsChannel(pAd);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				 "All Non-DFS channels are not available!!!! , just return first non-DFS channel %d, %s\n ", ch, __func__);
		} else {
			ch = 0;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
					 "All Non-DFS channels are not available!! , return channel 0, %s\n ", __func__);
		}
	} else {
		USHORT MinTime = 0xFFFF;
		UINT_16 BwChannel = 0;
		BOOLEAN ChInNop = FALSE;

		ch = 0;
		pDfsParam->bNoAvailableCh = FALSE;

		if (DfsChSelPrefer == RadarDetectSelectDFS ||
			DfsChSelPrefer == RadarDetectSelectWidestBWDFS)
			BwChannel = DfsBwChQueryByDefault(pAd, BW_160, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, TRUE, BandIdx);
		else
			BwChannel = DfsBwChQueryByDefault(pAd, BW_160, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, FALSE, BandIdx);
		ch = BwChannel & 0xff;

		pDfsParam->band_bw = BwChannel>>8;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
					"Ch(%d) selected by DfsBwChQueryByDefault\n", ch);

		/* Choose non-DFS ch if there are available non-DFS ch
		   when DfsChSelPrefer equal to DFS or WidestBWDFS  */
		if ((DfsChSelPrefer == RadarDetectSelectDFS || DfsChSelPrefer == RadarDetectSelectWidestBWDFS) &&
			ch == 0 &&
			IsAllChannelUnavailable(pAd) == FALSE) {
			UCHAR TargetBw = BW_160;
#ifdef DFS_SDB_SUPPORT
			/* Selectable Dual band 5g + 6g only support 40MHz */
			if (pDfsParam->bDfsSdbEnable == TRUE)
				TargetBw = BW_40;
#endif /* DFS_SDB_SUPPORT */

			BwChannel = DfsBwChQueryByDefault(pAd, TargetBw, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, FALSE, BandIdx);
			ch = BwChannel & 0xff;
			pDfsParam->band_bw = BwChannel>>8;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
					"DfsChSelPrefer=%d, select non-DFS Ch(%d), Bw(%d)\n", DfsChSelPrefer, ch, pDfsParam->band_bw);
		}

		if (ch == 0) {
			pDfsParam->bNoAvailableCh = TRUE;

			if (DfsCheckChPriorityByCh(pAd, pChCtrl->ch_cfg.boot_chan) &&
				((IS_CH_BETWEEN(pChCtrl->ch_cfg.boot_chan, 36, 48)) ||
				 (RadarChannelCheck(pAd, pChCtrl->ch_cfg.boot_chan) &&
				  pDfsParam->DfsNopExpireSetChPolicy == DFS_NOP_EXPIRE_SET_CH_POLICY_RESTORE))) {
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
				P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
				*ch_stat = DFS_INB_CH_INIT;
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
					"Reset ch_stat to  DFS_INB_CH_INIT\n");
#endif
				/* Restore Cfg bw  */
				pDfsParam->band_bw = wlan_config_get_eht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);

				if (IS_CH_BETWEEN(pChCtrl->ch_cfg.boot_chan, 36, 48)) {
					if (pDfsParam->band_bw == BW_160 || pDfsParam->band_bw == BW_80)
						DfsAdjustOpBwSetting(pAd, wdev, BW_160, BW_80);

					/* Use non-DFS ch between 36 and 48 (BW80) */
					pDfsParam->bNoAvailableCh = FALSE;
				}

				pDfsParam->bDfsWaitCfgChNop = TRUE;
				pChCfg = &pChCtrl->ch_cfg;
				ch = pChCfg->boot_chan;
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
					"Set to boot channel %d\n", pChCfg->boot_chan);
			} else {
				for (i = 0; i < pChCtrl->ChListNum; i++) {
					if (pChCtrl->ChList[i].NonOccupancy >= MinTime)
						continue;

					if (pChCtrl->ch_cfg.chan_prio_valid == TRUE &&
						pChCtrl->ChList[i].Priority == 0)
						continue;

					if (!IsChABand(PhyMode, pChCtrl->ChList[i].Channel))
						continue;
					if (ByPassChannelByBw(pChCtrl->ChList[i].Channel, pDfsParam->band_bw, pChCtrl))
						continue;
					if ((avoidedCh != 0)
						&& DfsPrimToCent(pChCtrl->ChList[i].Channel, BW_80) == DfsPrimToCent(avoidedCh, BW_80))
						continue;
					MinTime = pChCtrl->ChList[i].NonOccupancy;
					ch = pChCtrl->ChList[i].Channel;

					if ((pChCtrl->ChList[i].NonOccupancy > 0) ||
						(pChCtrl->ChList[i].NOPClrCnt != 0))
						ChInNop = TRUE;
					else
						ChInNop = FALSE;
				}
				if (ChInNop == TRUE) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
						"Channel(%d) in NOP, set DfsWaitCfgChNop to TRUE\n", ch);
					pDfsParam->bDfsWaitCfgChNop = TRUE;
				}
			}
		} else {
			if (RadarChannelCheck(pAd, ch) &&
				RadarChannelCheck(pAd, pChCtrl->ch_cfg.boot_chan) &&
				pDfsParam->DfsNopExpireSetChPolicy == DFS_NOP_EXPIRE_SET_CH_POLICY_RESTORE &&
				!DfsIsOutBandAvailable(pAd, wdev) &&
				IsAllChannelUnavailable(pAd)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
					"No channels available, bNoAvailableCh, bDfsWaitCfgChNop marked to TRUE\n");

				pDfsParam->bNoAvailableCh = TRUE;
				pDfsParam->bDfsWaitCfgChNop = TRUE;
			}
		}

#ifdef MT_DFS_SUPPORT
#if ((DFS_ZEROWAIT_DEFAULT_FLOW == 1) && defined(BACKGROUND_SCAN_SUPPORT))
		if (pAd->CommonCfg.DfsParameter.bPreCacEn) {
			P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat;

			ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
				"PreCac enabled find next channel to do pre-CAC\n");
			*ch_stat = DFS_INB_DFS_OUTB_CH_CAC_DONE;
			dfs_pre_cac_detect_next_channel(pAd, wdev);
		}
#endif/*MT_DFS_SUPPORT*/
#endif/*(DFS_ZEROWAIT_DEFAULT_FLOW == 1) && defined(BACKGROUND_SCAN_SUPPORT)*/

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"[RDM]:No channels available, Set new OutBandBw: %d\n",
			pDfsParam->OutBandBw);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
		"[RDM]:Currently no immediately available Channel. Choose Ch %d\n", ch);
	}

	return ch;
}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
UINT_8 DFS_V10_W52_LIST[V10_W52_SIZE] = {36, 40, 44, 48};
UINT_8 DFS_V10_W53_LIST[V10_W53_SIZE] = {52, 56, 60, 64};
UINT_8 DFS_V10_W56_VHT80_LIST[V10_W56_VHT80_A_SIZE + V10_W56_VHT80_B_SIZE + V10_W56_VHT80_C_SIZE] = {100,
	104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144};
UINT_8 DFS_V10_W56_LIST[V10_W56_VHT80_A_SIZE + V10_W56_VHT80_B_SIZE + V10_W56_VHT80_C_SIZE] = {100,
	104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144};
UINT_8 DFS_V10_W56_VHT80_LISTA[V10_W56_VHT80_A_SIZE] = {100, 104, 108, 112};
UINT_8 DFS_V10_W56_VHT80_LISTB[V10_W56_VHT80_B_SIZE] = {116, 120, 124, 128};
UINT_8 DFS_V10_W56_VHT80_LISTC[V10_W56_VHT80_C_SIZE] = {132, 136, 140, 144};
UINT_8 DFS_V10_W56_VHT20_LIST[V10_W56_VHT20_SIZE] = {132, 136, 140};

UINT_8 DfsV10FindNonNopChannel(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR		 chGrp,
	IN UCHAR		 grpWidth)
{
	UCHAR ChIdx = 0;
	UINT_8 channel = 0;

	if ((chGrp == W53 || chGrp == W56) && grpWidth && wdev) {
		/*Skip Non occupancy channel*/
		for (ChIdx = 0; ChIdx < grpWidth; ChIdx++) {
			channel = (chGrp == W53) ? (DFS_V10_W53_LIST[ChIdx]) : (DFS_V10_W56_LIST[ChIdx]);
			if (CheckNonOccupancyChannel(pAd, wdev, channel))
				return channel;
		}
	}

	return 0;
}

UINT_8 DfsV10W56FindMaxNopDuration(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR ChIdx = 0;
	USHORT channelNopTime = 0;
	UCHAR upperBoundCh = 0;
	PCHANNEL_CTRL pChCtrl = NULL;

	if (pAd->CommonCfg.bCh144Enabled)
		upperBoundCh = 144;
	else
		upperBoundCh = 140;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
		if (pChCtrl->ChList[ChIdx].Channel >= 100 &&
				pChCtrl->ChList[ChIdx].Channel <= upperBoundCh) {
			if (channelNopTime < pChCtrl->ChList[ChIdx].NonOccupancy)
				channelNopTime = pChCtrl->ChList[ChIdx].NonOccupancy;
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "MAX NOP %d\n", channelNopTime);
	return channelNopTime;
}

BOOLEAN DfsV10CheckGrpChnlLeft(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 chGrp,
	IN UCHAR		 grpWidth,
	IN UCHAR		 band_idx)
{
	UCHAR ChIdx = 0, ChCnt = 0;
	UCHAR idx, BandIdx;
	BOOLEAN status = FALSE;
	struct wifi_dev *wdev;

	if (chGrp == W53 || chGrp == W56 || chGrp == W56_UAB || chGrp == W56_UC) {

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		BandIdx = HcGetBandByWdev(wdev);
		if (band_idx == BandIdx)
			break;
	}

		/*Skip Non occupancy channel*/
		for (ChIdx = 0; ChIdx < grpWidth; ChIdx++) {
			if (CheckNonOccupancyChannel(pAd, wdev,
				((chGrp == W53) ? (DFS_V10_W53_LIST[ChIdx]) :
				((wlan_config_get_vht_bw(wdev) == VHT_BW_80) ?
					(DFS_V10_W56_VHT80_LIST[ChIdx]) : ((chGrp == W56) ?
					(DFS_V10_W56_LIST[ChIdx]) : ((pAd->CommonCfg.bCh144Enabled == FALSE) ?
					DFS_V10_W56_VHT20_LIST[ChIdx] : 0)))))) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "ChCnt++\n");
				ChCnt++;
			}
		}
	}

	if (ChCnt)
		status =  TRUE;
	else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "ChGrp %d VHT20 ChCnt %d Empty\n", chGrp, ChCnt);
		status = FALSE;
	}
	return status;
}

BOOLEAN DfsV10CheckChnlGrpW52(
	IN UCHAR Channel)
{
	UCHAR i = 0;

	while (i < V10_W52_SIZE && Channel != DFS_V10_W52_LIST[i])
		i++;

	if (i < V10_W52_SIZE)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN DfsV10CheckChnlGrpW53(
	IN UCHAR Channel)
{
	UCHAR i = 0;

	while (i < V10_W53_SIZE && Channel != DFS_V10_W53_LIST[i])
		i++;

	if (i < V10_W53_SIZE)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN DfsV10CheckChnlGrpW56UA(
	IN UCHAR Channel)
{
	UCHAR i = 0;

	while (i < V10_W56_VHT80_A_SIZE && Channel != DFS_V10_W56_VHT80_LISTA[i])
		i++;

	if (i < V10_W56_VHT80_A_SIZE)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN DfsV10CheckChnlGrpW56UB(
	IN UCHAR Channel)
{
	UCHAR i = 0;

	while (i < V10_W56_VHT80_B_SIZE && Channel != DFS_V10_W56_VHT80_LISTB[i])
		i++;

	if (i < V10_W56_VHT80_B_SIZE)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN DfsV10CheckChnlGrpW56UC(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel)
{
	UCHAR i = 0;

	if (pAd->CommonCfg.bCh144Enabled == FALSE) {
		while (i < V10_W56_VHT20_SIZE && Channel != DFS_V10_W56_VHT20_LIST[i])
			i++;

		if (i < V10_W56_VHT20_SIZE)
			return TRUE;
		else
			return FALSE;

	} else {
		while (i < V10_W56_VHT80_C_SIZE && Channel != DFS_V10_W56_VHT80_LISTC[i])
			i++;
		if (i < V10_W56_VHT80_C_SIZE)
			return TRUE;
		else
			return FALSE;
	}
}

BOOLEAN DfsV10CheckW56Grp(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel)
{
	BOOLEAN isW56 = FALSE;

	if (DfsV10CheckChnlGrpW56UA(channel))
		isW56 = TRUE;
	else if (DfsV10CheckChnlGrpW56UB(channel))
		isW56 = TRUE;
	else if (DfsV10CheckChnlGrpW56UC(pAd, channel))
		isW56 = TRUE;
	else
		isW56 = FALSE;

	return isW56;
}

UCHAR DfsV10CheckChnlGrp(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel)
{
	if (DfsV10CheckChnlGrpW52(Channel))
		return W52;
	else if (DfsV10CheckChnlGrpW53(Channel))
		return W53;
	else if (DfsV10CheckChnlGrpW56UA(Channel))
		return W56_UA;
	else if (DfsV10CheckChnlGrpW56UB(Channel))
		return W56_UB;
	else if (DfsV10CheckChnlGrpW56UC(pAd, Channel))
		return W56_UC;
	else
		return NA_GRP;
}

BOOLEAN DfsV10W56APDownStart(
	IN PRTMP_ADAPTER pAd,
	IN PAUTO_CH_CTRL pAutoChCtrl,
	IN ULONG		 V10W56TrgrApDownTime,
	IN UCHAR		 band_idx)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR BandIdx = band_idx;

	if (pAutoChCtrl == NULL)
		return FALSE;

	/* Disable AP 30 Minutes */
	pDfsParam->gV10W56TrgrApDownTime = V10W56TrgrApDownTime;
	SET_V10_W56_AP_DOWN(pAd, TRUE);

	/* ReEnable Boot ACS */
	SET_V10_BOOTACS_INVALID(pAd, FALSE);

	SET_V10_W56_GRP_VALID(pAd, TRUE);

	MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_V10_W56_APDOWN_ENBL, sizeof(UCHAR), &BandIdx, 0);

	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_NONE;

	return TRUE;
}

USHORT DfsV10SelectBestChannel(/*Select the Channel from Rank List by ACS*/
	IN PRTMP_ADAPTER pAd,
	IN UCHAR oldChannel,
	IN UCHAR band_idx)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	USHORT BwChannel = 0;
	struct wifi_dev *wdev;
	UCHAR BandIdx = BAND0;
	UCHAR idx;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		BandIdx = HcGetBandByWdev(wdev);
		if (band_idx == BandIdx)
			break;
	}

	pAutoChCtrl = HcGetAutoChCtrl(pAd);

	/* New Channel Identification */
	if (pAd->ApCfg.bAutoChannelAtBootup) {
		/* Pick AutoCh2 Update from List */
		BwChannel = SelectBestV10Chnl_From_List(pAd, BandIdx);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "ACS Disable\n");
		/* Push CSA BCN Update out of interrupt context */
		SET_V10_AP_BCN_UPDATE_ENBL(pAd, TRUE);
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
		"Select channel = %d from V10 list\n", BwChannel);

	BwChannel |= (pDfsParam->band_bw << 8);
	return BwChannel;
}

/* Weighing Factor for W56>W52>W53 Priority */
VOID DfsV10AddWeighingFactor(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *pwdev)
{
	UCHAR channelIdx = 0, chnlGrp = 0;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);

	/* ACS Disable: Weighing Factor Not Required */
	if (!pAd->ApCfg.bAutoChannelAtBootup)
		return;

	for (channelIdx = 0; channelIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; channelIdx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Channel %3d : Busy Time = %6u\n",
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channelIdx].Channel,
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx]);

	chnlGrp = DfsV10CheckChnlGrp(pAd, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channelIdx].Channel);

	if (chnlGrp == W52) {
		if (pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx])
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] *= V10_WEIGH_FACTOR_W52;
		else
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] += (V10_WEIGH_FACTOR_W52 * 10);
	} else if (chnlGrp == W53) {
		if (pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx])
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] *= V10_WEIGH_FACTOR_W53;
		else
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] += (V10_WEIGH_FACTOR_W53 * 10);
	} else if (chnlGrp >= W56_UA && chnlGrp <= W56_UC) {
		if (pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx])
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] *= V10_WEIGH_FACTOR_W56;
		else
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] += (V10_WEIGH_FACTOR_W56 * 10);
	} else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
		"Error Group Ch%d", pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channelIdx].Channel);
	}
}

VOID DfsV10W56APDownTimeCountDown(/*RemainingTimeForUse --*/
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if (IS_SUPPORT_V10_DFS(pAd) && IS_V10_W56_AP_DOWN_ENBLE(pAd)
		&& pDfsParam->gV10W56TrgrApDownTime > 0) {
		pDfsParam->gV10W56TrgrApDownTime--;
		if (!pDfsParam->gV10W56TrgrApDownTime) {
			/* Bring Up AP */
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "AP Down Pass\n");
			MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_V10_W56_APDOWN_FINISH, 0, NULL, 0);
		}
	}
}

VOID DfsV10W56APDownEnbl(
	RTMP_ADAPTER *pAd,
	PMLME_QUEUE_ELEM pElem)
{
	struct DOT11_H *pDot11hTest = NULL;
	struct wifi_dev *wdev;
	UCHAR BandIdx, idx, band_idx;
	BSS_STRUCT *pMbss;

	NdisMoveMemory(&band_idx, pElem->Msg, pElem->MsgLen);

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		BandIdx = HcGetBandByWdev(wdev);
		if (band_idx == BandIdx)
			break;
	}

	pMbss = &pAd->ApCfg.MBSSID[idx];

	pDot11hTest = &pAd->Dot11_H;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
		"W56 Down Time Start %d\n", IS_V10_W56_AP_DOWN_ENBLE(pAd));

	if (IS_SUPPORT_V10_DFS(pAd) && (!IS_V10_W56_AP_DOWN_ENBLE(pAd) || IS_V10_APINTF_DOWN(pAd))) {
		pDot11hTest->RDCount = 0;
		MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CAC_END, 0, NULL, HcGetBandByWdev(wdev));
		pDot11hTest->ChannelMode = CHAN_NORMAL_MODE;

		SET_V10_W56_AP_DOWN(pAd, TRUE);
		APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
	}
}

VOID DfsV10W56APDownPass(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "W56 Down Time Pass\n");

	if (IS_SUPPORT_V10_DFS(pAd) && IS_V10_W56_AP_DOWN_ENBLE(pAd)) {
		SET_V10_W56_AP_DOWN(pAd, FALSE);
		SET_V10_APINTF_DOWN(pAd, FALSE);
		APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
	}
}

VOID DfsV10APBcnUpdate(
	RTMP_ADAPTER *pAd,
	PMLME_QUEUE_ELEM pElem)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	USHORT BwChannel = 0;
	struct wifi_dev *wdev;
	UCHAR BandIdx = BAND0;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;
	UCHAR NextCh = 0, CurCh = 0;
	UCHAR NextBw = 0;
	UCHAR KeepBw = 0;
	UCHAR BssIdx;
	UCHAR idx;
	UINT_32 SetChInfo = 0;
	BSS_STRUCT *pMbss = NULL;
	UCHAR GrpSize;
	UCHAR band_idx;

	NdisMoveMemory(&band_idx, pElem->Msg, pElem->MsgLen);

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		BandIdx = HcGetBandByWdev(wdev);
		if (band_idx == BandIdx)
			break;
	}

	pAutoChCtrl = HcGetAutoChCtrl(pAd);

	/* Backup Original channel as we are doing off Channel scan */
	CurCh = wdev->channel;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "CurCh %d\n", CurCh);

	if (IS_V10_AP_BCN_UPDATE_ENBL(pAd))
		SET_V10_AP_BCN_UPDATE_ENBL(pAd, FALSE);

	ApAutoChannelSkipListBuild(pAd, wdev);
	if (DfsV10CheckW56Grp(pAd, wdev->channel)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "group 56\n");
		if (wlan_config_get_vht_bw(wdev) == VHT_BW_2040) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "VHT_BW_2040\n");
			if (pAd->CommonCfg.bCh144Enabled)
				GrpSize = V10_W56_SIZE;
			else
				GrpSize = V10_W56_SIZE - 1;

			if ((DfsV10CheckGrpChnlLeft(pAd, W56, GrpSize, BandIdx) == FALSE)
				|| (IS_V10_W56_VHT80_SWITCHED(pAd) &&
				DfsV10CheckGrpChnlLeft(pAd, W56_UC, V10_W56_VHT20_SIZE, BandIdx) == FALSE)) {
				if (IS_V10_W56_VHT80_SWITCHED(pAd)) {
					/* VHT 20 -> VHT 80 */
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
						"BW Switched to VHT80\n");
					wlan_config_set_ht_bw(wdev, HT_BW_40);
					wlan_config_set_vht_bw(wdev, VHT_BW_80);
#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
					pAd->CommonCfg.MCastPhyMode.field.BW = HT_BW_40;
					pAd->CommonCfg.MCastPhyMode_5G.field.BW = HT_BW_40;
#else
					pAd->CommonCfg.mcastphymode.field.BW = HT_BW_40;
#endif /* MCAST_VENDOR10_CUSTOM_FEATURE */
#endif /* MCAST_RATE_SPECIFIC */
					SET_V10_W56_VHT80_SWITCH(pAd, FALSE);
				} else
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
						"BW Not Switched to VHT80 %d\n",
						IS_V10_W56_VHT80_SWITCHED(pAd));

				/* No Channel Left in W53/ W56_UC VHT20 Case */
				if (DfsV10W56APDownStart(pAd, pAutoChCtrl, V10_W56_APDOWN_TIME, BandIdx))
					goto W56APDOWN;
				else
					ASSERT(BwChannel);
			}
		} else if (wlan_config_get_vht_bw(wdev) == VHT_BW_80) {

			if (pAd->CommonCfg.bCh144Enabled)
				GrpSize = V10_W56_VHT80_SIZE;
			else
				GrpSize = V10_W56_VHT80_SIZE - V10_W56_VHT80_C_SIZE;

			if (DfsV10CheckGrpChnlLeft(pAd, W56_UAB, GrpSize, BandIdx) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "56 Channel Left\n");

				if (pAd->CommonCfg.bCh144Enabled) {
					if (DfsV10W56APDownStart(pAd, pAutoChCtrl, V10_W56_APDOWN_TIME, BandIdx))
						goto W56APDOWN;
				} else {
				/* VHT80 -> VHT20 */
					wlan_config_set_ht_bw(wdev, HT_BW_20);
					wlan_config_set_vht_bw(wdev, VHT_BW_2040);
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
						"BW Switched to VHT20\n");
#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
					pAd->CommonCfg.MCastPhyMode.field.BW = HT_BW_20;
					pAd->CommonCfg.MCastPhyMode_5G.field.BW = HT_BW_20;
#else
					pAd->CommonCfg.mcastphymode.field.BW = HT_BW_20;
#endif /* MCAST_VENDOR10_CUSTOM_FEATURE */
#endif /* MCAST_RATE_SPECIFIC */
					SET_V10_W56_VHT80_SWITCH(pAd, TRUE);
					ApAutoChannelSkipListBuild(pAd, wdev);
				}
			}
		}
	}

	/* Perform Off Channel Scan to find channel */
	SET_V10_OFF_CHNL_TIME(pAd, V10_BGND_SCAN_TIME);
	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_NONE;
	BwChannel = MTAPAutoSelectChannel(pAd, wdev, ChannelAlgBusyTime, TRUE);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "BwChannel = %d\n", BwChannel);
	SET_V10_OFF_CHNL_TIME(pAd, V10_NORMAL_SCAN_TIME);
	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_SELECTED;

	/* Return to Original RADAR Hit Channel */
	/* Update channel of wdev as new channel */
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "calling AutoChSelUpdateChannel\n");
	AutoChSelUpdateChannel(pAd, CurCh, TRUE, wdev);

	/* Update primay channel */
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		"calling wlan_operate_set_prim_ch wdev->channel = %d\n", wdev->channel);
	wlan_operate_set_prim_ch(wdev, wdev->channel);

W56APDOWN:
	/* W56 Channel Exhausted : Ap Down for 30 Minutes */
	if (!BwChannel && IS_V10_W56_AP_DOWN_ENBLE(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"AP Down %ld\n", pDfsParam->gV10W56TrgrApDownTime);
		SET_V10_W56_AP_DOWN(pAd, FALSE);

		pDfsParam->DfsChBand[0] = FALSE;
		pDfsParam->DfsChBand[1] = FALSE;
		pDfsParam->RadarDetected[0] = FALSE;
		pDfsParam->RadarDetected[1] = FALSE;
		return;
	}

	pDfsParam->PrimBand = RDD_BAND0;
	pDfsParam->band_ch = pDfsParam->PrimCh = BwChannel & 0xFF;
	pDfsParam->band_bw = BwChannel >> 8;

	NextCh = pDfsParam->PrimCh;
	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdev = &pMbss->wdev;
		if (wdev->pHObj == NULL)
			continue;
		if (HcGetBandByWdev(wdev) != BandIdx)
			continue;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Update wdev of BssIdx %d\n",
				 BssIdx);
		/*Adjust Bw*/
#ifdef BACKGROUND_SCAN_SUPPORT
#ifdef ONDEMAND_DFS
		if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE
		&& hc_get_hw_band_idx(pAd) == pDfsParam->ZeroWaitBandidx) && (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd)) &&
			GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
#else
		if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE
		&& hc_get_hw_band_idx(pAd) == pDfsParam->ZeroWaitBandidx) && GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
#endif
			DfsAdjustBwSetting(pAd, wdev, pDfsParam->band_bw, pDfsParam->OutBandBw);
			NextBw = pDfsParam->OutBandBw;
		} else {
#else
		{
#endif /* BACKGROUND_SCAN_SUPPORT */

			DfsAdjustBwSetting(pAd, wdev, KeepBw, pDfsParam->band_bw);
			NextBw = pDfsParam->band_bw;
		}

		if (pDfsParam->Dot11_H.ChannelMode == CHAN_NORMAL_MODE) {
			pDfsParam->DfsChBand[0] = FALSE;
			pDfsParam->DfsChBand[1] = FALSE;
			pDfsParam->RadarDetected[0] = FALSE;
			pDfsParam->RadarDetected[1] = FALSE;

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				"\x1b[1;33m Normal Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
					 NextCh,
					 NextBw));
			rtmp_set_channel(pAd, wdev, NextCh);
		} else if (pDfsParam->Dot11_H.ChannelMode == CHAN_SILENCE_MODE) {
			pDfsParam->DfsChBand[0] = FALSE;
			pDfsParam->DfsChBand[1] = FALSE;
			pDfsParam->RadarDetected[0] = FALSE;
			pDfsParam->RadarDetected[1] = FALSE;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
					"\x1b[1;33m [%s]Silence Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
					 NextCh,
					 NextBw);
			SetChInfo |= NextCh;
			SetChInfo |= (BssIdx << 8);
			SetChInfo |= (BandIdx << 16);
			RTEnqueueInternalCmd(pAd, CMDTHRED_DFS_CAC_TIMEOUT, &SetChInfo, sizeof(UINT_32));
			RTMP_MLME_HANDLER(pAd);
		}
	}
}
#endif

USHORT DfsBwChQueryByDefault(/*Query current available BW & Channel list or select default*/
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Bw,
	IN PDFS_PARAM pDfsParam,
	IN UCHAR level,
	IN BOOLEAN bDefaultSelect,
	IN BOOLEAN SkipNonDfsCh,
	IN UCHAR band_idx)
{
	USHORT BwChannel = 0;
	UINT_8 ch = 0;
	UINT_8 ch_idx, SelectIdx;
	UINT_8 AvailableChCnt = 0;
	BOOLEAN nonWetherBandChExist = FALSE;
	BOOLEAN isSelectWetherBandCh = FALSE;
	PCHANNEL_CTRL pChCtrl = NULL;
	USHORT PhyMode = 0;
	BOOLEAN DefSelectCh = FALSE;
	UCHAR BandIdx = 0;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		 "Bw(%d), level(%d), bDefaultSelect(%d), SkipNonDfsCh(%d), band_idx(%d)\n",
		 Bw, level, bDefaultSelect, SkipNonDfsCh, band_idx);

	BandIdx = hc_get_hw_band_idx(pAd);
	pChCtrl = DfsGetChCtrl(pAd);
	PhyMode = HcGetRadioPhyModeByBandIdx(pAd);

	if ((pChCtrl->ChListNum > MAX_NUM_OF_CHANNELS) || (pChCtrl->ChListNum <= 0)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			 "Incorrect ChListNum(%d)\n", pChCtrl->ChListNum);
		return FALSE;
	}

	if (pDfsParam->bIEEE80211H == FALSE) {
		ch = pChCtrl->ChList[(UINT_8)(RandomByte(pAd)%pChCtrl->ChListNum)].Channel;
		BwChannel |= ch;
		BwChannel |= (Bw << 8);
		return BwChannel;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			 "pDfsParam->outbandch %d\n",
			 pDfsParam->OutBandCh);

	for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			 "Ch_idx(%d), Channel(%d), NonOccupancy(%d), NOPClrCnt(%d)\n",
			 ch_idx, pChCtrl->ChList[ch_idx].Channel, pChCtrl->ChList[ch_idx].NonOccupancy, pChCtrl->ChList[ch_idx].NOPClrCnt);

#ifdef CONFIG_AP_SUPPORT
		if (AutoChannelSkipListCheck(pAd, pChCtrl->ChList[ch_idx].Channel) == TRUE)
			continue;
#endif
		if ((SkipNonDfsCh == TRUE) && (!RadarChannelCheck(pAd, pChCtrl->ChList[ch_idx].Channel)))
			continue;

		if ((SkipNonDfsCh == TRUE) && (Bw < BW_160 && IS_CH_BETWEEN(pChCtrl->ChList[ch_idx].Channel, 36, 48)))
			continue;

		if (!IsChABand(PhyMode, pChCtrl->ChList[ch_idx].Channel))
			continue;

		if (ByPassChannelByBw(pChCtrl->ChList[ch_idx].Channel, Bw, pChCtrl))
			continue;

		if ((pChCtrl->ChList[ch_idx].NonOccupancy == 0)
		 && (pChCtrl->ChList[ch_idx].NOPClrCnt != 0)
		 && (pChCtrl->ChList[ch_idx].NOPSetByBw == Bw)
		)
			continue;

		if (DfsCheckBwGroupAllAvailable(ch_idx, Bw, pAd, band_idx) == FALSE)
			continue;

		if (DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, pDfsParam->band_bw) ==
			DfsPrimToCent(pDfsParam->band_ch, pDfsParam->band_bw)) {
			if (Bw < BW_160 && IS_CH_BETWEEN(pChCtrl->ChList[ch_idx].Channel, 36, 48))
				;
			else
				continue;
		}

		if ((level == DFS_BW_CH_QUERY_LEVEL1)
		&& ((pChCtrl->ChList[ch_idx].NonOccupancy == 0) && (pChCtrl->ChList[ch_idx].NOPClrCnt == 0)))
			pDfsParam->dfs_ch_grp.AvailableBwChIdx[Bw][AvailableChCnt++] = ch_idx;

		/* Avoid to select channel in NOP status */
		if ((level == DFS_BW_CH_QUERY_LEVEL2)
		&& ((pChCtrl->ChList[ch_idx].NonOccupancy == 0) && (pChCtrl->ChList[ch_idx].NOPClrCnt == 0)))
			pDfsParam->dfs_ch_grp.AvailableBwChIdx[Bw][AvailableChCnt++] = ch_idx;

		if (AvailableChCnt >= DFS_AVAILABLE_LIST_CH_NUM)
			break;
	}

	if (pChCtrl->ch_cfg.chan_prio_valid) {
		/* priority 0 channel won't be selected, if channel priority valid */
		UINT_8 max_prio = 1;
		UINT_8 new_available_cnt, available_ch_idx;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
			"Channel priority is valid\n");

		for (available_ch_idx = 0; available_ch_idx < AvailableChCnt; available_ch_idx++) {
			max_prio = pChCtrl->ChList[pDfsParam->dfs_ch_grp.AvailableBwChIdx[Bw][available_ch_idx]].Priority > max_prio ?
						pChCtrl->ChList[pDfsParam->dfs_ch_grp.AvailableBwChIdx[Bw][available_ch_idx]].Priority : max_prio;
		}

		new_available_cnt = 0;
		for (available_ch_idx = 0; available_ch_idx < AvailableChCnt; available_ch_idx++) {
			if (pChCtrl->ChList[pDfsParam->dfs_ch_grp.AvailableBwChIdx[Bw][available_ch_idx]].Priority == max_prio)
				pDfsParam->dfs_ch_grp.AvailableBwChIdx[Bw][new_available_cnt++] = pDfsParam->dfs_ch_grp.AvailableBwChIdx[Bw][available_ch_idx];
		}

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
				"Filter out channel by maximum priority(%d), cnt(%d) to new cnt(%d)\n",
				max_prio, AvailableChCnt, new_available_cnt);
		AvailableChCnt = new_available_cnt;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		 "AvailableChCnt(%d)\n", AvailableChCnt);

	if (AvailableChCnt > 0) {

		for (ch_idx = 0; ch_idx < AvailableChCnt; ch_idx++) {
			SelectIdx = pDfsParam->dfs_ch_grp.AvailableBwChIdx[Bw][ch_idx];
			if (pAd->CommonCfg.RDDurRegion == CE) {
				if (!DfsCacRestrictBand(pAd, Bw, pChCtrl->ChList[SelectIdx].Channel, 0))
					nonWetherBandChExist = TRUE;
				else {
					nonWetherBandChExist = FALSE;
					break;
				}

			} else {
				nonWetherBandChExist = TRUE;
				break;
			}
		}

		for (ch_idx = 0; ch_idx < AvailableChCnt; ch_idx++) {
			SelectIdx = pDfsParam->dfs_ch_grp.AvailableBwChIdx[Bw][ch_idx];

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"DefSelectCh-->%d, %d\n", pChCtrl->ChList[SelectIdx].Channel, Bw);

			if (pDfsParam->BW80DedicatedZWSupport &&
				IS_CH_BETWEEN(pChCtrl->ChList[SelectIdx].Channel, 100, 128) &&
				Bw == BW_80) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
					"DefSelectCh(%d)\n", pChCtrl->ChList[SelectIdx].Channel);
				DefSelectCh = TRUE;
				break;
			}
		}

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"DefSelectCh!!(%d), %d\n", pChCtrl->ChList[SelectIdx].Channel, Bw);

		if (!DefSelectCh) {
		/*randomly select a ch for this BW*/
			SelectIdx = pDfsParam->dfs_ch_grp.AvailableBwChIdx[Bw][(UINT_8)(RandomByte(pAd)%AvailableChCnt)];
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
				"Randomly select a ch: %d for bw: %d\n", pChCtrl->ChList[SelectIdx].Channel, Bw);
		}

		if ((pAd->CommonCfg.RDDurRegion == CE)
			&& DfsCacRestrictBand(pAd, Bw, pChCtrl->ChList[SelectIdx].Channel, 0))
			isSelectWetherBandCh = TRUE;
		while (isSelectWetherBandCh == TRUE && nonWetherBandChExist == TRUE) {
			SelectIdx = pDfsParam->dfs_ch_grp.AvailableBwChIdx[Bw][(UINT_8)(RandomByte(pAd)%AvailableChCnt)];

			if ((pAd->CommonCfg.RDDurRegion == CE)
				&& DfsCacRestrictBand(pAd, Bw, pChCtrl->ChList[SelectIdx].Channel, 0))
				isSelectWetherBandCh = TRUE;
			else
				isSelectWetherBandCh = FALSE;
		}
		BwChannel |= pChCtrl->ChList[SelectIdx].Channel;
		BwChannel |= (Bw << 8);
		return BwChannel;
	} else if (level == DFS_BW_CH_QUERY_LEVEL1)
		BwChannel = DfsBwChQueryByDefault(pAd, Bw, pDfsParam, DFS_BW_CH_QUERY_LEVEL2, bDefaultSelect, SkipNonDfsCh, band_idx);

	else if (level == DFS_BW_CH_QUERY_LEVEL2) {
		if (Bw > BW_20) {
			/*Clear NOP of the current BW*/
			for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
				if ((pChCtrl->ChList[ch_idx].NonOccupancy != 0) && (pChCtrl->ChList[ch_idx].NOPSetByBw == Bw)) {
					pChCtrl->ChList[ch_idx].NOPSaveForClear = pChCtrl->ChList[ch_idx].NonOccupancy;
					pChCtrl->ChList[ch_idx].NonOccupancy = 0;
					pChCtrl->ChList[ch_idx].NOPClrCnt++;
				}
			}
			/*reduce BW*/
			if (pDfsParam->DfsChSelPrefer == RadarDetectSelectWidestBWDFS) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
					"Don't reduce BW, return channel\n");
				BwChannel = Bw << 8;
			} else {
				if (pChCtrl->ch_cfg.bw40_forbid && Bw == BW_80) // avoid BW40
					BwChannel = DfsBwChQueryByDefault(pAd, Bw - 2, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, bDefaultSelect, SkipNonDfsCh, band_idx);
				else
					BwChannel = DfsBwChQueryByDefault(pAd, Bw - 1, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, bDefaultSelect, SkipNonDfsCh, band_idx);
			}
		} else
			;/*Will return BwChannel = 0*/
	} else
		;
	return BwChannel;

}

VOID DfsBwChQueryAllList(/*Query current All available BW & Channel list*/
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Bw,
	IN PDFS_PARAM pDfsParam,
	IN BOOLEAN SkipWorkingCh,
	IN UCHAR band_idx)
{
	UINT_8 ch_idx;
	UINT_8 AvailableChCnt = 0;
	PCHANNEL_CTRL pChCtrl = NULL;
	USHORT PhyMode = 0;

	if (pDfsParam->bIEEE80211H == FALSE)
		return ;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	PhyMode = HcGetRadioPhyModeByBandIdx(pAd);

	for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
#ifdef CONFIG_AP_SUPPORT
		if (AutoChannelSkipListCheck(pAd, pChCtrl->ChList[ch_idx].Channel) == TRUE)
			continue;
#endif
		if (!IsChABand(PhyMode, pChCtrl->ChList[ch_idx].Channel))
			continue;

		if (ByPassChannelByBw(pChCtrl->ChList[ch_idx].Channel, Bw, pChCtrl))
			continue;

		if ((pChCtrl->ChList[ch_idx].NonOccupancy == 0)
		 && (pChCtrl->ChList[ch_idx].NOPClrCnt != 0)
		 && (pChCtrl->ChList[ch_idx].NOPSetByBw <= Bw)
		)
			continue;

		if (DfsCheckBwGroupAllAvailable(ch_idx, Bw, pAd, band_idx) == FALSE)
			continue;

		if (SkipWorkingCh == TRUE) {
			if (DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, pDfsParam->band_bw) ==
				DfsPrimToCent(pDfsParam->band_ch, pDfsParam->band_bw))
				continue;

			if (DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, Bw) ==
				DfsPrimToCent(pDfsParam->band_ch, Bw))
				continue;
		}

		if (pChCtrl->ChList[ch_idx].NonOccupancy == 0) {
			pDfsParam->dfs_ch_grp.AvailableBwChIdx[Bw][AvailableChCnt++] = ch_idx;
		} else
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "NOP ! =0 (%d)\n",
					 pChCtrl->ChList[ch_idx].NonOccupancy);
	}

	if (Bw > BW_20) {
		/*Clear NOP of the current BW*/
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if ((pChCtrl->ChList[ch_idx].NonOccupancy != 0) && (pChCtrl->ChList[ch_idx].NOPSetByBw == Bw)) {
				pChCtrl->ChList[ch_idx].NOPSaveForClear = pChCtrl->ChList[ch_idx].NonOccupancy;
				pChCtrl->ChList[ch_idx].NonOccupancy = 0;
				pChCtrl->ChList[ch_idx].NOPClrCnt++;
			}
		}
		DfsBwChQueryAllList(pAd, Bw - 1, pDfsParam, SkipWorkingCh, band_idx);
	}

}

BOOLEAN DfsDedicatedCheckChBwValid(
	IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw, UCHAR BandIdx)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UINT_8 i, j, idx;
	PCHANNEL_CTRL pChCtrl = NULL;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	if (pDfsParam->bDedicatedZeroWaitSupport == FALSE || hc_get_hw_band_idx(pAd) != pDfsParam->ZeroWaitBandidx)
		return TRUE;

	for (i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++) {
		for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
			pDfsParam->dfs_ch_grp.AvailableBwChIdx[i][j] = 0xff;
	}

	DfsBwChQueryAllList(pAd, BW_160, pDfsParam, FALSE, BandIdx);

	for (i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++) {
		for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++) {
			if (pDfsParam->dfs_ch_grp.AvailableBwChIdx[i][j] != 0xff) {
				idx = pDfsParam->dfs_ch_grp.AvailableBwChIdx[i][j];

				if ((pChCtrl->ChList[idx].Channel == Channel)
				 && (Bw == i)) {
					return TRUE;
				}
			}
		}
	}
	return FALSE;

}

VOID DfsAdjustBwSetting(
	IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR CurrentBw, UCHAR NewBw)
{
	UCHAR HtBw;
	UCHAR VhtBw;
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	switch (NewBw) {
	case BW_20:
		HtBw = BW_20;
		VhtBw = VHT_BW_2040;
		break;
	case BW_40:
		HtBw = BW_40;
		VhtBw = VHT_BW_2040;
		break;
	case BW_80:
		HtBw = BW_40;
		VhtBw = VHT_BW_80;
		break;
	case BW_160:
		HtBw = BW_40;
		VhtBw = VHT_BW_160;
		break;
	default:
		return;
	}

	if (cfg) {
		cfg->ht_conf.ht_bw = HtBw;
		cfg->vht_conf.vht_bw = VhtBw;
	}

}

VOID DfsAdjustOpBwSetting(
	IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR CurrentBw, UCHAR NewBw)
{
	UCHAR HtBw;
	UCHAR VhtBw;

	switch (NewBw) {
	case BW_20:
		HtBw = BW_20;
		VhtBw = VHT_BW_2040;
		break;
	case BW_40:
		HtBw = BW_40;
		VhtBw = VHT_BW_2040;
		break;
	case BW_80:
		HtBw = BW_40;
		VhtBw = VHT_BW_80;
		break;
	case BW_160:
		HtBw = BW_40;
		VhtBw = VHT_BW_160;
		break;
	default:
		return;
	}
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
		"[Adjust Op bw]: CurrentBw: %d, NewBw %d\n",
		CurrentBw,
		NewBw);
	pAd->CommonCfg.DfsParameter.ZwAdjBw = NewBw;
	pAd->CommonCfg.DfsParameter.ZwAdjBwFlag = TRUE;
}

VOID WrapDfsRadarDetectStart(/*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev
)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	struct freq_oper oper;
	struct DOT11_H *pDot11h = NULL;
	UCHAR band_idx;
	UCHAR rddidx;

	if (wdev == NULL)
		return;

	if (!IsChABand(wdev->PhyMode, wdev->channel))
		return;

	if (hc_radio_query_by_wdev(wdev, &oper)) {
		return;
	}

	if (wdev) {
		pDot11h = wdev->pDot11_H;
	}

	if (pDot11h == NULL)
		return;

	band_idx = HcGetBandByWdev(wdev);
	rddidx = (band_idx == BAND1) ? RDD_INBAND_IDX_1 : RDD_INBAND_IDX_2;
	pDfsParam->DfsChBand[rddidx] = RadarChannelCheck(pAd, pDfsParam->band_ch);

#ifdef DOT11_VHT_AC

	if (pDfsParam->band_bw == BW_160)
		pDfsParam->DfsChBand[rddidx] = RadarChannelCheck(pAd, pDfsParam->band_ch);

	if ((pDfsParam->band_bw == BW_160) &&
		(pDfsParam->PrimCh >= GROUP1_LOWER && pDfsParam->PrimCh <= GROUP1_UPPER))
		pDfsParam->DfsChBand[rddidx] = TRUE;

#endif

#ifdef DFS_ADJ_BW_ZERO_WAIT
	if (IS_ADJ_BW_ZERO_WAIT(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState))
		pDfsParam->DfsChBand[HW_RDD1] = TRUE;
#endif

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "[RDM]: OutBandCh: %d, band_ch: %d\n",
		pDfsParam->OutBandCh,
		pDfsParam->band_ch);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
		"[RDM]: DfsChBand[0]: %d, DfsChBand[1]: %d, DfsChBand[2]: %d\n",
		pDfsParam->DfsChBand[HW_RDD0],
		pDfsParam->DfsChBand[HW_RDD1],
		pDfsParam->DfsChBand[HW_RDD2]);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
		"[RDM] BandIdx: 0, BW: %d, ChannelMode: %d\n",
		 pDfsParam->OutBandBw,
		 pDot11h->ChannelMode);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
		"[RDM] BandIdx: 1, BW: %d, ChannelMode: %d\n",
		 pDfsParam->band_bw,
		 pDot11h->ChannelMode);
	DfsRadarDetectStart(pAd, pDfsParam, wdev);
}

VOID DfsRadarDetectStart(/*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd,
	PDFS_PARAM pDfsParam,
	struct wifi_dev *wdev
)
{
	INT ret1 = TRUE;
	UCHAR BandIdx;
	UCHAR RddIdx;
	UCHAR rd_region = 0; /* Region of radar detection */
	struct DOT11_H *pDot11h = NULL;
	UCHAR cac_done = FALSE;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	BandIdx = HcGetBandByWdev(wdev);
	RddIdx = (BandIdx == BAND1) ? RDD_INBAND_IDX_1 : RDD_INBAND_IDX_2;
	rd_region = pAd->CommonCfg.RDDurRegion;

	if (pDot11h == NULL)
		return;

	if (scan_in_run_state(pAd, NULL) || (pDot11h->ChannelMode == CHAN_SWITCHING_MODE))
		return;

	if (pAd->CommonCfg.DfsParameter.CACMemoEn)
		cac_done = dfs_cac_op(pAd, wdev, CAC_DONE_CHECK, wdev->channel);

#ifdef MAP_R2
	if (pDot11h->ChannelMode == CHAN_SILENCE_MODE
		|| wdev->cac_not_required == TRUE
		|| cac_done == TRUE) {
#else
	if (pDot11h->ChannelMode == CHAN_SILENCE_MODE) {
#endif
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "[RDM]:ZeroWaitState:%d\n",
				 GET_MT_ZEROWAIT_DFS_STATE(pAd));

		if (pDfsParam->RadarDetectState[RddIdx] == FALSE) {
			if (pAd->CommonCfg.dbdc_mode) {
				/* DBDC mode */
				/* RddSel=0: Use band1/RX2 to detect radar */
				ret1 = mtRddControl(pAd, RDD_START, RddIdx, RXSEL_0, rd_region);
			}

#ifdef DOT11_VHT_AC
			else if (pDfsParam->band_bw == BW_160) {
				if ((pDfsParam->OutBandCh >= GROUP1_LOWER &&
					pDfsParam->OutBandCh <= GROUP1_UPPER)
#ifdef DFS_SDB_SUPPORT
					&& (pAd->CommonCfg.DfsParameter.bDfsSdbEnable == FALSE)
#endif /* DFS_SDB_SUPPORT */
					)
					;
				else
					ret1 = mtRddControl(pAd, RDD_START, RDD_DEDICATED_IDX, RXSEL_0, rd_region);

				ret1 = mtRddControl(pAd, RDD_START, RddIdx, RXSEL_0, rd_region);
			}

#endif
			else {
				ret1 = mtRddControl(pAd, RDD_START, RDD_DEDICATED_IDX, RXSEL_0, rd_region);
				ret1 = mtRddControl(pAd, RDD_START, RddIdx, RXSEL_0, rd_region);
			}
		}

		pDfsParam->RadarDetectState[RddIdx] = TRUE;
	} else if (DfsIsOutBandAvailable(pAd, wdev) && pDfsParam->bDedicatedZeroWaitSupport
	&& hc_get_hw_band_idx(pAd) == pDfsParam->ZeroWaitBandidx) {
		ret1 = mtRddControl(pAd, RDD_START, RddIdx, RXSEL_0, rd_region);
	}
#ifdef DFS_ADJ_BW_ZERO_WAIT
	else if (IS_ADJ_BW_ZERO_WAIT(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState))
	{
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "[RDM]: start RDD, BandIdx:%d\n", BandIdx);
		ret1 = mtRddControl(pAd, RDD_START, RddIdx, RXSEL_0, rd_region);
	}
#endif
}

VOID WrapDfsRadarDetectStop(/*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	DfsRadarDetectStop(pAd, pDfsParam);
}

VOID DfsRadarDetectStop(/*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam)
{
	INT ret = TRUE;
	UCHAR BandIdx = 0;
	UCHAR RddIdx = 0;

	BandIdx = hc_get_hw_band_idx(pAd);
	RddIdx = (BandIdx == BAND1) ? RDD_INBAND_IDX_1 : RDD_INBAND_IDX_2;

	pDfsParam->RadarDetectState[RddIdx] = FALSE;

	if (!pDfsParam->bDfsEnable)
		return;

	ret = mtRddControl(pAd, RDD_STOP, RddIdx, 0, 0);
}

VOID DfsDedicatedOutBandRDDStart(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR rd_region = pAd->CommonCfg.RDDurRegion; /* Region of radar detection */

	pDfsParam->RadarDetected[RDD_DEDICATED_IDX] = FALSE;
	pDfsParam->DfsChBand[RDD_DEDICATED_IDX] = RadarChannelCheck(pAd, pDfsParam->OutBandCh);

#ifdef DFS_ADJ_BW_ZERO_WAIT
	if (pDfsParam->BW160ZeroWaitSupport == TRUE)
		return;
#endif

	if (RadarChannelCheck(pAd, pDfsParam->OutBandCh) ||
		(IS_CH_BETWEEN(pAd->CommonCfg.DfsParameter.OutBandCh, 36, 48) &&
		(pAd->CommonCfg.DfsParameter.OutBandBw == BW_160)))
		pDfsParam->DfsChBand[RDD_DEDICATED_IDX] = TRUE;
	else
		pDfsParam->DfsChBand[RDD_DEDICATED_IDX] = FALSE;
	if (pDfsParam->DfsChBand[RDD_DEDICATED_IDX] || (pDfsParam->OutBandBw == BW_160 && IS_CH_BETWEEN(pDfsParam->OutBandCh, 36, 64))) {
		mtRddControl(pAd, RDD_START, RDD_DEDICATED_IDX, RXSEL_0, rd_region);
		if (pDfsParam->ucDisTm)
			mtRddControl(pAd, DISABLE_ZW_TM, RDD_DEDICATED_IDX, RXSEL_0, rd_region);

		DfsOutBandCacReset(pAd);

		if (pDfsParam->bOcacEnable != 0
			&& DfsCacRestrictBand(pAd, pDfsParam->OutBandBw, pDfsParam->OutBandCh, 0))
			pDfsParam->DedicatedOutBandCacTime = CAC_OCACENABLE_WEATHER_BAND;
		else if ((pAd->CommonCfg.RDDurRegion == CE)
		 && DfsCacRestrictBand(pAd, pDfsParam->OutBandBw, pDfsParam->OutBandCh, 0))
			pDfsParam->DedicatedOutBandCacTime = CAC_WEATHER_BAND;
		else if (pDfsParam->bOcacEnable != 0)
			pDfsParam->DedicatedOutBandCacTime = CAC_OCACENABLE_NON_WEATHER_BAND;
		else
			pDfsParam->DedicatedOutBandCacTime = CAC_NON_WEATHER_BAND;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
			"[RDM]: Dedicated CAC time: %d\n", pDfsParam->DedicatedOutBandCacTime);
	}
}

VOID DfsDedicatedOutBandRDDRunning(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	USHORT bw_ch = 0;
	UCHAR BandIdx = 0;
	UCHAR band_idx;
	USHORT bw_ch_band[CFG_WIFI_RAM_BAND_NUM];
	UCHAR bw_band[CFG_WIFI_RAM_BAND_NUM];
	UCHAR bw = 0;
	CHANNEL_CTRL *pChCtrl = NULL;
	UCHAR ch_idx = 0;
	BOOLEAN fg_in_band_use = FALSE;
	BOOLEAN fg_radar_detect = FALSE;
	UCHAR wdev_idx;
	struct wifi_dev *wdev = NULL;
	UCHAR dfs_range_change = DFS_RANGE_NULL;
#ifdef CONFIG_MAP_SUPPORT
	UCHAR map_outband_ch = 0;
	UCHAR map_outBand_BW = 0;
	UCHAR BssIdx;
	BSS_STRUCT *pMbss = NULL;
#endif

	if ((pDfsParam->bDedicatedZeroWaitDefault == FALSE && pDfsParam->bOcacEnable == 0)
	|| pDfsParam->BW160ZeroWaitSupport == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
			"DedicatedZeroWaitDefault is not enabled\n");
		return;
	}

	/* Find an active wdev on the phy */
	for (wdev_idx = 0; wdev_idx < WDEV_NUM_MAX; wdev_idx++) {
		if (pAd->wdev_list[wdev_idx]) {
			wdev = pAd->wdev_list[wdev_idx];
			break;
		}
	}

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"No any active wdev on the phy!\n");
		return;
	}

	for (band_idx = 0; band_idx < 2; band_idx++) {
		bw_ch_band[band_idx] = 0;
		bw_band[band_idx] = 0;
		if (pDfsParam->OutBandBw == BW_160 && pDfsParam->BW80DedicatedZWSupport == FALSE)
			bw = BW_160;
		else if (pDfsParam->BW80DedicatedZWSupport && (pDfsParam->OutBandBw == BW_160 || pDfsParam->OutBandBw == BW_80))
			bw = BW_80;
		else
			bw = wlan_config_get_bw(wdev);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
			"bw is %d\n", bw);

		BandIdx = hc_get_hw_band_idx(pAd);
		pChCtrl = DfsGetChCtrl(pAd);

		/* Check A band */
		if (pChCtrl->ChList[0].Channel < 36) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				"Not A-band, channel %d\n", pChCtrl->ChList[0].Channel);
			continue;
		}

		if (pDfsParam->bDfsDedicatedRxPreselectCh)
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
					"in-band channel %d, outband ch %d\n",
					pDfsParam->band_ch, pDfsParam->OutBandCh);

		/* Check NOP of current outband ch */
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (pChCtrl->ChList[ch_idx].Channel == pDfsParam->OutBandCh) {
				if ((pChCtrl->ChList[ch_idx].NonOccupancy == 0) &&
					(pChCtrl->ChList[ch_idx].NOPSaveForClear == 0)) {

					if (DfsPrimToCent(pDfsParam->OutBandCh, pDfsParam->OutBandBw) ==
						DfsPrimToCent(pDfsParam->band_ch, pDfsParam->band_bw) ||
						(pDfsParam->BW80DedicatedZWSupport && IS_CH_BETWEEN(pDfsParam->band_ch, 36, 64) &&
						pDfsParam->OutBandCh == 52 && pDfsParam->band_bw == BW_160)) {
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
								 "In-band %d is using this channel %d\n",
								  band_idx, pDfsParam->band_ch);

						fg_in_band_use = TRUE;
						break;
					}

					dfs_range_change = judge_dfs_range_change(pAd, wdev, pDfsParam->band_ch, pDfsParam->band_bw, pDfsParam->OutBandCh, pDfsParam->OutBandBw);

					if (dfs_range_change == DFS_RANGE_NO_CHANGE ||
						dfs_range_change == DFS_RANGE_NARROW) {
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
								"band_ch: %d, band_bw: %d -> OutBandCh: %d, OutBandBw: %d (dfs_range_change: %d)\n",
								pDfsParam->band_ch, pDfsParam->band_bw, pDfsParam->OutBandCh, pDfsParam->OutBandBw, dfs_range_change);


						fg_in_band_use = TRUE;
						break;
					}
				}
				else {
					fg_radar_detect = TRUE;
				}

				if (pDfsParam->OutBandBw == BW_160 && IS_CH_BETWEEN(pDfsParam->OutBandCh, 36, 64) &&
						(pChCtrl->ChList[4].NonOccupancy)) {
					fg_radar_detect = TRUE;
				}
			}
		}

		bw_ch_band[band_idx] = DfsBwChQueryByDefault(pAd, bw, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, TRUE, band_idx);

		bw_band[band_idx] = bw_ch_band[band_idx]>>8;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"bw_ch_band[%d] 0x%x\n", band_idx, bw_ch_band[band_idx]);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"bw_band[%d] 0x%x\n", band_idx, bw_band[band_idx]);

		bw_ch = bw_ch_band[band_idx];
	}

	/* no in-band use ch same as out-band, keep use out-band */
	if ((fg_in_band_use == FALSE) && (fg_radar_detect == FALSE))
	{
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
							 "NOP of Ch%d is clear, keep using this ch\n",
							  pDfsParam->OutBandCh);
		return;
	}

#ifdef CONFIG_MAP_SUPPORT
	map_outband_ch = bw_ch & 0xff;
	map_outBand_BW = bw_ch >> 8;
	if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {
		if (map_outband_ch != pDfsParam->OutBandCh) {
			band_idx = HW_RDD1;
			for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
				pMbss = &pAd->ApCfg.MBSSID[BssIdx];
				wdev = &pMbss->wdev;
				if (wdev->pHObj == NULL)
					continue;
				if (HcGetBandByWdev(wdev) != band_idx)
					continue;
				if (!wdev->if_dev)
					continue;
			}
			if (wdev->if_dev) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
						"CAC STOP to WAPP as outband ch: %d is of range of inband ch\n",
						pDfsParam->OutBandCh);
				wapp_send_cac_stop(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), pDfsParam->OutBandCh, TRUE);
			}
		}
	}
#endif

	if (pDfsParam->bOcacEnable != 1) {
		pDfsParam->OutBandCh = bw_ch & 0xff;
		pDfsParam->OutBandBw = bw_ch>>8;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
		"\x1b[1;33m OutBandCh %d, OutBandBw %d \x1b[m\n",
		pDfsParam->OutBandCh, pDfsParam->OutBandBw);
}

VOID DfsDedicatedOutBandRDDStop(
	IN PRTMP_ADAPTER pAd)
{
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "DfsDedicatedOutBandRDDStop start.\n");
#if RDD_PROJECT_TYPE_2
	mtRddControl(pAd, RDD_IRQ_OFF, RDD_DEDICATED_IDX, 0, 0);
#else
	mtRddControl(pAd, RDD_STOP, RDD_DEDICATED_IDX, 0, 0);
#endif

}

BOOLEAN DfsIsRadarHitReport(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	return pDfsParam->RadarHitReport == TRUE;
}

VOID DfsRadarHitReportReset(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	pDfsParam->RadarHitReport = FALSE;
}

VOID DfsReportCollision(
	IN PRTMP_ADAPTER pAd)
{
#ifdef BACKGROUND_SCAN_SUPPORT
	if (IS_SUPPORT_DEDICATED_ZEROWAIT_DFS(pAd)
	&& DfsIsRadarHitReport(pAd)) {
		ZeroWait_DFS_collision_report(pAd, HW_RDD0,
		GET_BGND_PARAM(pAd, ORI_INBAND_CH), GET_BGND_PARAM(pAd, ORI_INBAND_BW));
		DfsRadarHitReportReset(pAd);
	}
#endif
}

BOOLEAN DfsIsTargetChAvailable(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	if ((pDfsParam->targetCh != 0) && (pDfsParam->targetCacValue == 0))
		return TRUE;

	return FALSE;
}

BOOLEAN DfsIsOutBandAvailable(
	IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	BOOLEAN bAvailable;

	bAvailable = ((pDfsParam->bOutBandAvailable == TRUE) &&
		(pDfsParam->bSetInBandCacReStart == FALSE));

	return bAvailable;
}

VOID DfsOutBandCacReset(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	pDfsParam->DedicatedOutBandCacCount = 0;
	pDfsParam->bOutBandAvailable = FALSE;
}

VOID DfsSetCacRemainingTime(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;
	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;

	if (pDfsParam->bDedicatedZeroWaitSupport == TRUE
	&& hc_get_hw_band_idx(pAd) == pDfsParam->ZeroWaitBandidx) {
		if ((pDot11h->ChannelMode == CHAN_SILENCE_MODE)
			&& (pDfsParam->bSetInBandCacReStart == FALSE)) {
			pDot11h->RDCount = pDfsParam->DedicatedOutBandCacCount;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				"\x1b[1;33m [RDM] Remaining CAC time is %d \x1b[m \n",
			pDot11h->cac_time - pDot11h->RDCount);
		}
	}

	pDfsParam->bSetInBandCacReStart = FALSE;
	DfsOutBandCacReset(pAd);

}

VOID DfsOutBandCacCountUpdate(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR BandIdx;
	struct wifi_dev *wdev = NULL;
	UCHAR BssIdx;
#ifdef DFS_CAC_R2
	BSS_STRUCT *pMbss = NULL;
#endif
#ifdef BACKGROUND_SCAN_SUPPORT
	if (!GET_BGND_STATE(pAd, BGND_RDD_DETEC))
		return;
#endif

	BandIdx = hc_get_hw_band_idx(pAd);

	if ((pDfsParam->bDedicatedZeroWaitSupport == FALSE
	|| BandIdx != pDfsParam->ZeroWaitBandidx) && pDfsParam->bOcacEnable == 0
	&& pDfsParam->BW160ZeroWaitSupport == TRUE)
		return;

	if (pDfsParam->bOutBandAvailable != FALSE)
		return;

	/* detection mode is enabled */
	if (pDfsParam->bNoSwitchCh == TRUE)
		return;

	if (pDfsParam->RadarDetected[RDD_DEDICATED_IDX] == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "Radar is detected by dedicated RX.\n");
		return;
	}

	if (pDfsParam->OutBandCh != 0 &&
		pDfsParam->DedicatedOutBandCacCount++ > pDfsParam->DedicatedOutBandCacTime) {

		pDfsParam->bOutBandAvailable = TRUE;
		pDfsParam->DedicatedOutBandCacCount = 0;

		/*update CAC done*/
		if (pAd->CommonCfg.DfsParameter.CACMemoEn) {
			for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
				wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
				if (BandIdx == HcGetBandByWdev(wdev))
					break;
			}
			dfs_cac_op(pAd, wdev, CAC_DONE_UPDATE, pDfsParam->OutBandCh);
		}

		if (pDfsParam->bDfsDedicatedRxPreselectCh)
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
				"OutBand(SynB) CAC complete and is available now.\n");
#ifdef DFS_CAC_R2
		if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {
			/*add for non radar detected case by dedicated radio
			For Harrier we have done DFS by dedicated radio still send ifindex for 5G Radio*/
			for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
				pMbss = &pAd->ApCfg.MBSSID[BssIdx];
				wdev = &pMbss->wdev;
				if (wdev->pHObj == NULL)
					continue;
				if (!wdev->if_dev)
					continue;
				if (RtmpOSNetDevIsUp(wdev->if_dev)) {
					wapp_send_cac_stop(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), pDfsParam->OutBandCh, TRUE);
					break;
				}
			}
		}
#endif
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
		if (pDfsParam->bDedicatedZeroWaitDefault == TRUE || pDfsParam->bOcacEnable != 0) {
			if (!MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_OFF_CAC_END, 0, NULL, 0)) {
				ReleaseChannelOpChargeByBand(pAd, BandIdx, CH_OP_OWNER_DFS);
				return;
			}
			RTMP_MLME_HANDLER(pAd);
		}
#else
		if (DfsCacTimeOutCallBack) {
			DfsCacTimeOutCallBack(RDD_BAND1, pDfsParam->OutBandBw, pDfsParam->OutBandCh);
		}
#endif
	}
}

VOID DfsDedicatedExamineSetNewCh(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR Channel)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR InputCentCh = DfsPrimToCent(Channel, pDfsParam->band_bw);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "DfsDedicatedExamineSetNewCh start.\n");

	if (pDfsParam->bDedicatedZeroWaitSupport == FALSE
	|| hc_get_hw_band_idx(pAd) != pDfsParam->ZeroWaitBandidx)
		return;

	if (InputCentCh == DfsPrimToCent(pDfsParam->OutBandCh, pDfsParam->band_bw))
		pDfsParam->bSetInBandCacReStart = FALSE;
	else
		pDfsParam->bSetInBandCacReStart = TRUE;

#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
	/* Mark the set ch flag to ensure go to set channel flow */
	pDfsParam->SetChByCmd = TRUE;
	DfsDedicatedSetNewChStat(pAd, wdev, Channel);
#endif /* DFS_ZEROWAIT_DEFAULT_FLOW */

}

VOID DfsDedicatedSetNewChStat(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR Channel)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "DfsDedicatedSetNewChStat start.\n");
	if (!WMODE_CAP_5G(wdev->PhyMode)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "Not ABand, End---->.\n");
		return;
	}
	if (pDfsParam->bDedicatedZeroWaitDefault == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
			"ZW-DFS disabled, End---->.\n");
		return;
	}
	if (!RadarChannelCheck(pAd, wdev->channel) && !RadarChannelCheck(pAd, Channel)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
			"Wdev channel %d and Current channel %d not Radar channel, End---->.\n",
			wdev->channel, Channel);
		return;
	}

	if (RadarChannelCheck(pAd, Channel) &&
	    (*ch_stat == DFS_INB_DFS_OUTB_CH_CAC || *ch_stat == DFS_INB_DFS_OUTB_CH_CAC_DONE)) {
		if (pDfsParam->SetOutBandChStat == OUTB_SET_CH_DEFAULT) {
			if (*ch_stat == DFS_INB_DFS_OUTB_CH_CAC_DONE &&
				(Channel == pDfsParam->OutBandCh)) {
				/* The channel that has been set has already
				undergone a Channel Availability Check (CAC) on dedicataRX. */
				*ch_stat = DFS_INB_DFS_OUTB_CH_CAC;

				pDfsParam->SetOutBandChStat = OUTB_SET_CH_CAC_DONE;
			} else {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				"Set to DFS channel to  Outband\n");
				*ch_stat = DFS_INB_DFS_OUTB_CH_CAC;

				pDfsParam->SetOutBandChStat = OUTB_SET_CH_CAC;
			}
		}

		if (pDfsParam->SetOutBandChStat == OUTB_SET_CH_CAC_DONE) {
			pDfsParam->SetChByCmd = FALSE;
			pDfsParam->bSetInBandCacReStart = FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		"Set to DFS channel - initialize stat\n");
		*ch_stat = DFS_INB_CH_INIT;
		pDfsParam->OutBandCh = 0;
		pDfsParam->ZwChBootUp = TRUE;

		pDfsParam->SetChByCmd = FALSE;
	}

	pDfsParam->DedicatedOutBandCacCount = 0;
}


/*----------------------------------------------------------------------------*/
/*!
* \brief	 Configure (Enable/Disable) HW RDD and RDD wrapper module
*
* \param[in] ucRddCtrl
*			 ucRddIdex
*
*
* \return	 None
*/
/*----------------------------------------------------------------------------*/

INT mtRddControl(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR ucRddCtrl,
	IN UCHAR ucRddIdex,
	IN UCHAR ucRddRxSel,
	IN UCHAR ucSetVal)
{
	INT ret = FALSE;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
	"[mtRddControl]RddCtrl=%d, RddIdx=%d, RddRxSel=%d\n", ucRddCtrl, ucRddIdex, ucRddRxSel);
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		ret = UniCmdRddCtrl(pAd, ucRddCtrl, ucRddIdex, ucRddRxSel, ucSetVal);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		ret = MtCmdRddCtrl(pAd, ucRddCtrl, ucRddIdex, ucRddRxSel, ucSetVal);
	return ret;
}

UCHAR DfsGetCentCh(IN PRTMP_ADAPTER pAd, IN UCHAR Channel, IN UCHAR bw, struct wifi_dev *wdev)
{
	UCHAR CentCh = 0;

	if (bw == BW_20)
		CentCh = Channel;

#ifdef DOT11_N_SUPPORT
	else if ((bw == BW_40) && N_ChannelGroupCheck(pAd, Channel, wdev)) {
#ifdef A_BAND_SUPPORT

		if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
			(Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157))
			CentCh = Channel + 2;
		else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) ||
			(Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161))
			CentCh = Channel - 2;
#endif /* A_BAND_SUPPORT */
	}

#ifdef DOT11_VHT_AC
	else if (bw == BW_80) {
		if (vht80_channel_group(pAd, Channel, wdev))
			CentCh = vht_cent_ch_freq(Channel, VHT_BW_80, CMD_CH_BAND_5G);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "[RDM]Error!Unexpected Bw=%d!!\n",
				 bw);
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "[RDM]Control/Central Ch=%d/%d;Bw=%d\n",
			 Channel,
			 CentCh,
			 bw);
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	return CentCh;
}

#ifdef BACKGROUND_SCAN_SUPPORT
VOID DfsDedicatedScanStart(IN PRTMP_ADAPTER pAd) /*This function is not used*/
{
	UCHAR bw_band0, bw_band1, idx;
	USHORT bw_ch, bw_ch_band0, bw_ch_band1;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "[RDM] DfsDedicatedScanStart \n");

	if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) &&
		(pDfsParam->bDedicatedZeroWaitDefault == TRUE)) {

			bw_ch_band0 = DfsBwChQueryByDefault(pAd, BW_80, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, TRUE, RDD_BAND0);
			bw_ch_band1 = DfsBwChQueryByDefault(pAd, BW_80, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, TRUE, RDD_BAND1);

			bw_band0 = bw_ch_band0>>8;
			bw_band1 = bw_ch_band1>>8;

			if (bw_band0 > bw_band1)
				bw_ch = bw_band0;

			else if (bw_band0 < bw_band1)
				bw_ch = bw_band1;

			else {
				/* bw_band0 == bw_band1 */
				idx = RandomByte(pAd) % 2;
				bw_ch = (idx) ? bw_band0 : bw_band1;
			}

			pDfsParam->OutBandCh = bw_ch & 0xff;
			pDfsParam->OutBandBw = bw_ch>>8;


		if (pDfsParam->OutBandCh == 0) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "[RDM] No available Outband BW\n");
			return;
		}

		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_DEDICATE_RDD_REQ, 0, NULL, 0);
		RTMP_MLME_HANDLER(pAd);
	}
}

VOID DfsInitDedicatedScanStart(IN PRTMP_ADAPTER pAd) /*This function is not used*/
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	if (pDfsParam->bInitOutBandBranch == TRUE) {
		pDfsParam->bInitOutBandBranch = FALSE;
		DfsDedicatedScanStart(pAd);
	}
}

VOID DfsSetInitDediatedScanStart(IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	pDfsParam->bInitOutBandBranch = TRUE;
}

BOOLEAN DfsDedicatedInBandSetChannel(
	IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw, BOOLEAN doCAC, UCHAR ucRddIdx)
{
	UCHAR NextCh;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR BssIdx;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	struct DOT11_H *dot11h_param = &pAd->Dot11_H;
	UINT_32 SetChInfo = 0;
	UCHAR BandIdx = 0;
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
	UCHAR ret = 0;
	BOOLEAN IsSuccess = FALSE;

	BandIdx = hc_get_hw_band_idx(pAd);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
		"(caller:%pS)\n", OS_TRACE);

	if (!TakeChannelOpChargeByBand(pAd, BandIdx, CH_OP_OWNER_DFS, FALSE)) {
		pDfsParam->DedicatedOutBandCacCount = pDfsParam->DedicatedOutBandCacCount - 5;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				"%s: TakeChannelOpCharge fail for DFS!!, extend CAC time by 5s\n", __func__);
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
	"\x1b[1;33m[RDM] BandIdx: %d, ucRddIdx: %d, Channel: %d, Bw: %d, ChannelMode: %d \x1b[m\n",
	BandIdx, ucRddIdx, Channel, Bw, dot11h_param->ChannelMode);

	if ((pDfsParam->bDedicatedZeroWaitSupport == FALSE || hc_get_hw_band_idx(pAd) != pDfsParam->ZeroWaitBandidx)
	&& pDfsParam->BW160ZeroWaitSupport == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
		"[RDM] DedicatedZeroWaitSupport is not enabled on band:%d\n",
		hc_get_hw_band_idx(pAd));
		IsSuccess = FALSE;
		goto end;
	}

	if (!DfsDedicatedCheckChBwValid(pAd, Channel, Bw, BandIdx)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
		"\x1b[1;33m[RDM] Not a Valid InBand Channel. Fail. \x1b[m\n");
		IsSuccess = FALSE;
		goto end;
	}

	if (Channel == 0 ||
		((Channel == pDfsParam->OutBandCh) && (Bw == pDfsParam->OutBandBw))) {
		if (pDfsParam->BW80DedicatedZWSupport &&
			(Channel == 52 && Bw == BW_80)) {
			Channel = pDfsParam->OutBandOriCh;
			Bw = BW_160;
		} else if (pDfsParam->BW80DedicatedZWSupport && *ch_stat == DFS_INB_DFS_OUTB_CH_CAC_DONE)
			Bw = BW_160;
		else {
			Channel = pDfsParam->OutBandCh;
			Bw = pDfsParam->OutBandBw;
		}
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "InBand set to OutBand Channel %d, Bw :%d\n", Channel, Bw);
	} else {
		pDfsParam->bSetInBandCacReStart = TRUE;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "InBand set to non-OutBand Channel %d, Bw %d\n", Channel, Bw);
	}

#ifdef DFS_CAC_R2
		if (IS_MAP_TURNKEY_ENABLE(pAd)) {
		/*test -> avoid this setting of outband channel onto inband if cac time expires
		and no radar is detected , no need for this step at all .*/

			/* Still switching ch to inband if ChannelMode equal to NOP_MODE
			for Enable MAC Tx */
			if (dot11h_param->ChannelMode != CHAN_NOP_MODE) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
				"OUT band CAC end on ch %d, but avoid switching\n", pDfsParam->OutBandCh);
				*ch_stat = DFS_OUTB_CH_CAC;
				pAd->CommonCfg.DfsParameter.ZwAdjBw = Bw;
				pAd->CommonCfg.DfsParameter.ZwAdjBwFlag = FALSE;
				IsSuccess = TRUE;
				goto end;
			}
		}
#endif

	if ((Channel == pDfsParam->band_ch)  &&  (Bw == pDfsParam->band_bw) &&
		(dot11h_param->ChannelMode != CHAN_NOP_MODE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "\x1b[1;33m This is current Ch %d, Bw %d \x1b[m\n", Channel, Bw);
		if ((doCAC == FALSE) && (dot11h_param->ChannelMode == CHAN_SILENCE_MODE)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
			"\x1b[1;33m Enable beacon now \x1b[m\n");
			dot11h_param->RDCount = dot11h_param->cac_time;
		}
		IsSuccess = TRUE;
		goto end;
	}

	if (doCAC == FALSE) {
		pDfsParam->bSetInBandCacReStart = FALSE;
		pDfsParam->bOutBandAvailable = TRUE;
	}


	pDfsParam->OrigInBandCh = pDfsParam->band_ch;
	pDfsParam->OrigInBandBw = pDfsParam->band_bw;
	pDfsParam->band_ch = Channel;
	pDfsParam->PrimCh = pDfsParam->band_ch;
	pDfsParam->PrimBand = BandIdx;
	NextCh = pDfsParam->PrimCh;

	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdev = &pMbss->wdev;

		if (wdev == NULL || wdev->pHObj == NULL)
			continue;

		/*Adjust Bw*/
		DfsAdjustOpBwSetting(pAd, wdev, pDfsParam->band_bw, Bw);
	}

	/*choose RadioOn interface*/
	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdev = &pMbss->wdev;

		if (wdev->pHObj == NULL)
			continue;
		/*Need choose RadioOn interface, before enq cmd to set channel*/
		if (IsHcRadioCurStatOffByWdev(wdev))
			continue;

		if (HcGetBandByWdev(wdev) == BandIdx)
			break;
	}
	if (BssIdx == pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
			"no Radio On interface\n");
		IsSuccess = FALSE;
		goto end;
	}

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "[RDM]Update wdev of BssIdx %d\n",
				 BssIdx);

		if (dot11h_param->ChannelMode == CHAN_NORMAL_MODE) {
			pDfsParam->DfsChBand[ucRddIdx] = FALSE;
			pDfsParam->RadarDetected[ucRddIdx] = FALSE;

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "[RDM]\x1b[1;33m Normal Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
						 NextCh,
						 Bw);

			perform_channel_change(pAd, wdev, NextCh);

			// if disable csa, need release lock
			if (pAd->ApCfg.set_ch_async_flag == FALSE)
				ReleaseChannelOpChargeByBand(pAd, BandIdx, CH_OP_OWNER_DFS);
		} else if (dot11h_param->ChannelMode == CHAN_SILENCE_MODE ||
				   dot11h_param->ChannelMode == CHAN_NOP_MODE) {
			pDfsParam->DfsChBand[ucRddIdx] = FALSE;
			pDfsParam->RadarDetected[ucRddIdx] = FALSE;

			if (dot11h_param->ChannelMode == CHAN_SILENCE_MODE)
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
							"\x1b[1;33m [RDM]Silence Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
							NextCh, Bw);
			else if (dot11h_param->ChannelMode == CHAN_NOP_MODE) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
							"\x1b[1;33m [RDM]NOP Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
							NextCh, Bw);
				/* CAC done channel switch to inband */
				pAd->CommonCfg.DfsParameter.bBootUpInBandUnavailableCh = FALSE;
			}
			SetChInfo |= NextCh;
			SetChInfo |= (BssIdx << 8);
			SetChInfo |= (BandIdx << 16);

			ret = RTEnqueueInternalCmd(pAd, CMDTHRED_DFS_CAC_TIMEOUT, &SetChInfo, sizeof(UINT_32));
			if (ret == NDIS_STATUS_SUCCESS)
				RTMP_MLME_HANDLER(pAd);
			else {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
					"[RDM]\x1b[1;33m enque dfs cac timeout cmd failed.\x1b[m\n");
				IsSuccess = FALSE;
				goto end;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "[RDM]\x1b[1;33m Switching Mode!!Dedicated set channel failed.\x1b[m\n");
			IsSuccess = FALSE;
			goto end;
		}
	return TRUE;
end:
	ReleaseChannelOpChargeByBand(pAd, BandIdx, CH_OP_OWNER_DFS);
	return IsSuccess;
}

VOID DfsDedicatedOutBandSetChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel,
	IN UCHAR Bw,
	IN UCHAR band_idx)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR BandIdx = 0;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "[RDM] SynNum: %d, Channel: %d, Bw: %d\n", band_idx, Channel, Bw);

	if (pDfsParam->SetChByCmd == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
		"Set SetChByCmd to FALSE\n");
		pDfsParam->SetChByCmd = FALSE; // reset the set channel flag
	}

	if ((pDfsParam->bDedicatedZeroWaitSupport == FALSE
	|| hc_get_hw_band_idx(pAd) != pDfsParam->ZeroWaitBandidx) && pDfsParam->bOcacEnable == 0
	&& pDfsParam->BW160ZeroWaitSupport == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
		"[RDM] DedicatedZeroWaitSupport is not enabled on band:%d\n", hc_get_hw_band_idx(pAd));
		return;
	}

	BandIdx = hc_get_hw_band_idx(pAd);

	if (!(DfsDedicatedCheckChBwValid(pAd, Channel, Bw, BandIdx))
#if (RDD_2_SUPPORTED == 1)
		&& !(DfsDedicatedCheckChBwValid(pAd, Channel, Bw, RDD_BAND1))
#endif
		) {
		pDfsParam->OutBandBw = Bw;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Get new outband DFS channel\n");
		DfsDedicatedOutBandRDDRunning(pAd);

		if (pDfsParam->OutBandCh != 0) {
			Channel = pDfsParam->OutBandCh;
			Bw = pDfsParam->OutBandBw;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
			"\x1b[1;33m [RDM] No valid OutBand Channel. Fail. \x1b[m \n");
			return;
		}
	}
	if (!RadarChannelCheck(pAd, Channel) && !((Bw == BW_160) && IS_CH_BETWEEN(Channel, 36, 64))) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
		"\x1b[1;33m [RDM] This is Not a DFS Channel. No need for Radar Detection. \x1b[m \n");
		return;
	}

	if (Channel != 0) {
		pDfsParam->OutBandCh = Channel;
		pDfsParam->OutBandBw = Bw;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Pick OutBand Ch by internal Alogorithm\n");
	}

#ifdef MAP_R2
	if (IS_MAP_TURNKEY_ENABLE(pAd)) {
		if (GET_BGND_STATE(pAd, BGND_SCAN_IDLE) &&
			RadarChannelCheck(pAd, pDfsParam->band_ch) &&
			(DfsPrimToCent(Channel, Bw) == DfsPrimToCent(pDfsParam->band_ch, pDfsParam->band_bw)) &&
			pAd->Dot11_H.ChannelMode != CHAN_NOP_MODE) {
			/* Get a new outband DFS channel */
			/* if the channel is already being used in-band */
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
				"Get new outband DFS channel\n");
			DfsDedicatedOutBandRDDRunning(pAd);
		}
	}
#endif /* MAP_R2 */

	if (GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "Dedicated Running: OutBand set Channel to %d\n", Channel);
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_OUTBAND_SWITCH, 0, NULL, 0);
		RTMP_MLME_HANDLER(pAd);
	} else if (GET_BGND_STATE(pAd, BGND_SCAN_IDLE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "Dedicated Start: OutBand set Channel to %d\n", Channel);
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_DEDICATE_RDD_REQ, 0, NULL, 0);
		RTMP_MLME_HANDLER(pAd);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "Wrong state. OutBand Set Channel Fail\n");
	}
}

#if (RDD_2_SUPPORTED == 0)
VOID DfsDedicatedDynamicCtrl(IN PRTMP_ADAPTER pAd, UINT_32 DfsDedicatedOnOff)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "\x1b[1;33m [RDM] DfsDedicatedOnOff: %d \x1b[m \n",
		DfsDedicatedOnOff);

	if (DfsDedicatedOnOff == DYNAMIC_ZEROWAIT_OFF) {
		if (GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
			pDfsParam->OrigInBandCh = pDfsParam->PrimCh;
			pDfsParam->OrigInBandBw = pDfsParam->OutBandBw;
			DedicatedZeroWaitStop(pAd, FALSE);
			DfsOutBandCacReset(pAd);
			pDfsParam->RadarDetected[1] = FALSE;
		} else
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "\x1b[1;33m [RDM] Already in 4x4 mode \x1b[m \n");
	} else	{
		if (GET_BGND_STATE(pAd, BGND_RDD_DETEC))
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "\x1b[1;33m [RDM] Already in 2x2 mode \x1b[m \n");
		else if (pDfsParam->OutBandCh == 0)
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "\x1b[1;33m [RDM] No SynB Info Recorded. Fail. \x1b[m \n");
		else {

			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_DEDICATE_RDD_REQ, 0, NULL, 0);
			RTMP_MLME_HANDLER(pAd);
		}
	}
}
#endif /* RDD_2_SUPPORTED */
#endif /* BACKGROUND_SCAN_SUPPORT */

INT Set_ModifyChannelList_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Value;
	UCHAR Bw80Num = 4;
	UCHAR Bw40Num = 10;
	UCHAR Bw20Num = 11;

	DFS_REPORT_AVALABLE_CH_LIST Bw80AvailableChList[4]
	= {{116, 0}, {120, 0}, {124, 0}, {128, 0} };
	DFS_REPORT_AVALABLE_CH_LIST Bw40AvailableChList[10]
	= {{100, 0}, {104, 0}, {108, 0}, {112, 0}, {116, 0}, {120, 0}, {124, 0}, {128, 0}, {132, 0}, {136, 0} };
	DFS_REPORT_AVALABLE_CH_LIST Bw20AvailableChList[11]
	= {{100, 0}, {104, 0}, {108, 0}, {112, 0}, {116, 0}, {120, 0}, {124, 0}, {128, 0}, {132, 0}, {136, 0}, {140, 0} };

	Value = (UCHAR) simple_strtol(arg, 0, 10);

	ZeroWait_DFS_Initialize_Candidate_List(pAd,
	Bw80Num, &Bw80AvailableChList[0],
	Bw40Num, &Bw40AvailableChList[0],
	Bw20Num, &Bw20AvailableChList[0]);

	return TRUE;
}

INT Show_available_BwCh_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR band_idx;

	for (band_idx = 0; band_idx < CFG_WIFI_RAM_BAND_NUM; band_idx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "band_idx: %d\n", band_idx);
		DfsProvideAvailableChList(pAd, band_idx);
	}

	return TRUE;
}

INT Show_NOP_Of_ChList(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	union dfs_zero_wait_msg msg;
	UCHAR ch_idx = 0;
	os_zero_mem(&msg, sizeof(union dfs_zero_wait_msg));

	DfsProvideNopOfChList(pAd, &msg);

	MTWF_PRINT("[%s][RDM]\n", __func__);

	for (ch_idx = 0; ch_idx < msg.nop_of_channel_list_msg.NOPTotalChNum; ch_idx++) {
		MTWF_PRINT("NopReportChList[%d].Channel = %d, Bw = %d, NOP = %d\n",
		ch_idx,
		msg.nop_of_channel_list_msg.NopReportChList[ch_idx].Channel,
		msg.nop_of_channel_list_msg.NopReportChList[ch_idx].Bw,
		msg.nop_of_channel_list_msg.NopReportChList[ch_idx].NonOccupancy);
	}

	return TRUE;
}

INT Show_Target_Ch_Info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ZeroWait_DFS_Next_Target_Show(pAd, 1);
	return TRUE;
}

VOID ZeroWait_DFS_Initialize_Candidate_List(
	IN PRTMP_ADAPTER pAd,
	UCHAR Bw80Num, PDFS_REPORT_AVALABLE_CH_LIST pBw80AvailableChList,
	UCHAR Bw40Num, PDFS_REPORT_AVALABLE_CH_LIST pBw40AvailableChList,
	UCHAR Bw20Num, PDFS_REPORT_AVALABLE_CH_LIST pBw20AvailableChList)
{
	UINT_8 band_idx, i = 0, j = 0, k = 0;
	UINT_8 ChIdx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR SupportBwBitMap[MAX_NUM_OF_CHS] = {0};
	UCHAR OrigSupportBwBitMap[MAX_NUM_OF_CHS] = {0};
	PCHANNEL_CTRL pChCtrl = NULL;

	band_idx = hc_get_hw_band_idx(pAd);
	if (pAd->Dot11_H.ChannelMode == CHAN_SWITCHING_MODE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				"Channel list init fail during channel switch\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "band_idx: %d\n", band_idx);

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
		if (pBw80AvailableChList->Channel == pChCtrl->ChList[ChIdx].Channel) {
			SupportBwBitMap[ChIdx] |= 0x04;
			if (i++ < Bw80Num)
				pBw80AvailableChList++;
		}
		if (pBw40AvailableChList->Channel == pChCtrl->ChList[ChIdx].Channel) {
			SupportBwBitMap[ChIdx] |= 0x02;
			if (j++ < Bw40Num)
				pBw40AvailableChList++;
		}
		if (pBw20AvailableChList->Channel == pChCtrl->ChList[ChIdx].Channel) {
			SupportBwBitMap[ChIdx] |= 0x01;
			if (k++ < Bw20Num)
				pBw20AvailableChList++;
		}
	}

	for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
		OrigSupportBwBitMap[ChIdx] = pChCtrl->ChList[ChIdx].SupportBwBitMap;

		if (OrigSupportBwBitMap[ChIdx] >= 0x07) {
			if (SupportBwBitMap[ChIdx] == 0x07)
				;
			else if (SupportBwBitMap[ChIdx] == 0x03) {
				pChCtrl->ChList[ChIdx].NOPSetByBw = BW_80;
				pChCtrl->ChList[ChIdx].NOPClrCnt = 1;
				pChCtrl->ChList[ChIdx].NOPSaveForClear = 1800;
			} else if (SupportBwBitMap[ChIdx] == 0x01) {
				pChCtrl->ChList[ChIdx].NOPSetByBw = BW_40;
				pChCtrl->ChList[ChIdx].NOPClrCnt = 1;
				pChCtrl->ChList[ChIdx].NOPSaveForClear = 1800;
			} else if (SupportBwBitMap[ChIdx] == 0x0) {
				pChCtrl->ChList[ChIdx].NOPSetByBw = BW_20;
				pChCtrl->ChList[ChIdx].NOPClrCnt = 1;
				pChCtrl->ChList[ChIdx].NOPSaveForClear = 1800;
			} else
				;
		} else if (OrigSupportBwBitMap[ChIdx] == 0x03) {
			if (SupportBwBitMap[ChIdx] == 0x03)
				;
			else if (SupportBwBitMap[ChIdx] == 0x01) {
				pChCtrl->ChList[ChIdx].NOPSetByBw = BW_40;
				pChCtrl->ChList[ChIdx].NOPClrCnt = 1;
				pChCtrl->ChList[ChIdx].NOPSaveForClear = 1800;
			} else if (SupportBwBitMap[ChIdx] == 0x0) {
				pChCtrl->ChList[ChIdx].NOPSetByBw = BW_20;
				pChCtrl->ChList[ChIdx].NOPClrCnt = 1;
				pChCtrl->ChList[ChIdx].NOPSaveForClear = 1800;
			} else
				;
		} else if (OrigSupportBwBitMap[ChIdx] == 0x01) {
			if (SupportBwBitMap[ChIdx] == 0x01)
				;
			else if (SupportBwBitMap[ChIdx] == 0x0) {
				pChCtrl->ChList[ChIdx].NOPSetByBw = BW_20;
				pChCtrl->ChList[ChIdx].NOPClrCnt = 1;
				pChCtrl->ChList[ChIdx].NOPSaveForClear = 1800;
			} else
				;
		} else
			;
	}

	for (i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++) {
		for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
			pDfsParam->dfs_ch_grp.AvailableBwChIdx[i][j] = 0xff;
	}

	DfsBwChQueryAllList(pAd, BW_80, pDfsParam, TRUE, band_idx);

	for (i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++) {
#ifdef DFS_DBG_LOG_0
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Bw: %d\n", i);
#endif
		for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++) {
			if (pDfsParam->dfs_ch_grp.AvailableBwChIdx[i][j] != 0xff) {
#ifdef DFS_DBG_LOG_0
				ChIdx = pDfsParam->dfs_ch_grp.AvailableBwChIdx[i][j];
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
						"ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
						ChIdx,
						pChCtrl->ChList[ChIdx].Channel,
						pChCtrl->ChList[ChIdx].NOPClrCnt);
#endif
			}
		}
	}
}

VOID DfsProvideAvailableChList(
	IN PRTMP_ADAPTER pAd, IN UCHAR band_idx)
{
	UINT_8 bw_idx, ch_idx, idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	PCHANNEL_CTRL pChCtrl = NULL;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (bw_idx = 0; bw_idx < DFS_AVAILABLE_LIST_BW_NUM; bw_idx++) {
		for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++)
			pDfsParam->dfs_ch_grp.AvailableBwChIdx[bw_idx][ch_idx] = 0xff;
	}

	if (pAd->Dot11_H.ChannelMode == CHAN_SWITCHING_MODE)
		return;

	DfsBwChQueryAllList(pAd, BW_80, pDfsParam, TRUE, band_idx);

	for (bw_idx = 0; bw_idx < DFS_AVAILABLE_LIST_BW_NUM; bw_idx++) {

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Bw: %d\n", bw_idx);

		for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
			if (pDfsParam->dfs_ch_grp.AvailableBwChIdx[bw_idx][ch_idx] != 0xff) {
				idx = pDfsParam->dfs_ch_grp.AvailableBwChIdx[bw_idx][ch_idx];
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				"ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
				idx, pChCtrl->ChList[idx].Channel,
				pChCtrl->ChList[idx].NOPClrCnt);
			}
		}
	}
}

VOID DfsProvideNopOfChList(
	IN PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg)
{
	UINT_8 ch_idx;
	UINT_8 nop_ch_idx = 0;
	UINT_8 band_idx = 0;
	PCHANNEL_CTRL pChCtrl = NULL;

	NOP_REPORT_CH_LIST NopReportChList[DFS_AVAILABLE_LIST_CH_NUM];

	band_idx = hc_get_hw_band_idx(pAd);
	os_zero_mem(&NopReportChList, sizeof(NOP_REPORT_CH_LIST) * DFS_AVAILABLE_LIST_CH_NUM);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
		if (pChCtrl->ChList[ch_idx].NonOccupancy != 0) {
			NopReportChList[nop_ch_idx].Channel = pChCtrl->ChList[ch_idx].Channel;
			NopReportChList[nop_ch_idx].Bw = pChCtrl->ChList[ch_idx].NOPSetByBw;
			NopReportChList[nop_ch_idx].NonOccupancy = pChCtrl->ChList[ch_idx].NonOccupancy;
			nop_ch_idx++;
		} else if (pChCtrl->ChList[ch_idx].NOPSaveForClear != 0) {
			NopReportChList[nop_ch_idx].Channel = pChCtrl->ChList[ch_idx].Channel;
			NopReportChList[nop_ch_idx].Bw = pChCtrl->ChList[ch_idx].NOPSetByBw;
			NopReportChList[nop_ch_idx].NonOccupancy = pChCtrl->ChList[ch_idx].NOPSaveForClear;
			nop_ch_idx++;
		}
	}

	for (ch_idx = 0; ch_idx < nop_ch_idx; ch_idx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				"Local NopReportChList[%d].Channel = %d, Bw = %d, NOP = %d\n",
				ch_idx, NopReportChList[ch_idx].Channel, NopReportChList[ch_idx].Bw,
				NopReportChList[ch_idx].NonOccupancy);
	}

	msg->nop_of_channel_list_msg.NOPTotalChNum = nop_ch_idx;
	memcpy(&(msg->nop_of_channel_list_msg.NopReportChList[0]),
			NopReportChList,
			nop_ch_idx * sizeof(NOP_REPORT_CH_LIST));
}

VOID ZeroWait_DFS_set_NOP_to_Channel_List(
	IN PRTMP_ADAPTER pAd, IN UCHAR Channel, UCHAR Bw, USHORT NOPTime)
{
	UINT_8 ch_idx;
	PCHANNEL_CTRL pChCtrl = NULL;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "[RDM] Channel: %d, Bw: %d, NOP: %d\n",
		Channel, Bw, NOPTime);

	if (Bw > BW_80) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "[RDM] Not a valid BW for ZeroWait\n");
		return;
	}
	if (!RadarChannelCheck(pAd, Channel)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "[RDM] Ch %d is not a DFS channel. InValid\n",
			Channel);
		return;
	}

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	if (ByPassChannelByBw(Channel, Bw, pChCtrl)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "[RDM] Ch%d doesn't support BW %d\n",
				Channel, Bw);
		return;
	}

	for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
		if (Channel == pChCtrl->ChList[ch_idx].Channel) {
			pChCtrl->ChList[ch_idx].NOPSetByBw = Bw;
			pChCtrl->ChList[ch_idx].NOPClrCnt++;

			switch (Bw) {
				case BW_80:
				case BW_40:
					pChCtrl->ChList[ch_idx].NOPSaveForClear = NOPTime;
					break;

				case BW_20:
					pChCtrl->ChList[ch_idx].NonOccupancy = NOPTime;
					break;

				default:
					break;
			}
		}
	}
}

VOID ZeroWait_DFS_Pre_Assign_Next_Target_Channel(
	IN PRTMP_ADAPTER pAd, IN UCHAR Channel, IN UCHAR Bw, IN USHORT CacValue)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	pDfsParam->targetCh = Channel;
	pDfsParam->targetBw = Bw;
	pDfsParam->targetCacValue = CacValue;

}

VOID ZeroWait_DFS_Next_Target_Show(
	IN PRTMP_ADAPTER pAd, IN UCHAR mode)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if (mode != 0)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "\x1b[1;33m[RDM] Target Channel: %d, Target Bw: %d, Target CAC value:%d \x1b[m \n",
		pDfsParam->targetCh, pDfsParam->targetBw, pDfsParam->targetCacValue);

}

VOID ZeroWait_DFS_collision_report(
	IN PRTMP_ADAPTER pAd, IN UCHAR SynNum, IN UCHAR Channel, UCHAR Bw)
{
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "\x1b[1;33m[RDM] SynNum: %d, Channel: %d, Bw:%d \x1b[m \n",
		SynNum, Channel, Bw);

	if (radar_detected_callback_func) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
			"\x1b[1;33m[RDM] Call back func \x1b[m \n");

		radar_detected_callback_func(SynNum, Channel, Bw);
	}

}

VOID DfsZeroHandOffRecovery(IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	if (pDot11h) {
		if (pDot11h->ChannelMode == CHAN_SILENCE_MODE) {
			mtRddControl(pAd, RDD_RESUME_BF, HW_RDD0, 0, 0);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Resume BF.\n");
		}
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief		Mapping RDD index to DBDC index
* @param[in]	PRTMP_ADAPTER pAd
* @param[in]	rddidx: RDD index
* @return		bandIdx: DBDC index
*/
/*----------------------------------------------------------------------------*/
UCHAR dfs_rddidx_to_dbdc(IN PRTMP_ADAPTER pAd, IN UINT8 rddidx)
{
	UCHAR bandidx = rddidx;

#if (RDD_PROJECT_TYPE_1 == 1)
	/* Single PHY, DBDC, RDD0/RDD1 */
	if (IS_SUPPORT_SINGLE_PHY_DBDC_DUAL_RDD(pAd)) {
		PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
		if (pDfsParam->OutBandBw == BW_160)
			bandidx = DBDC_BAND0;
	}

#if (RDD_2_SUPPORTED == 1)
	if (IS_SUPPORT_RDD2_DEDICATED_RX(pAd)) {
		/*PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;*/

		switch (rddidx) {
		case HW_RDD0:
		case HW_RDD1:
			break;

		case HW_RDD2:
			bandidx = RDD_DEDICATED_RX;
			break;

		default:
			break;
		}
	}
#endif /* RDD_2_SUPPORTED */
#endif /* RDD_PROJECT_TYPE_1 */

	return bandidx;
}

VOID DfsSetNewChInit(IN PRTMP_ADAPTER pAd)
{
	UCHAR band_idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	for (band_idx = 0; band_idx < 1; band_idx++)
		pDfsParam->NeedSetNewChList = DFS_SET_NEWCH_INIT;
}

VOID get_dfs_range(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR ch, UCHAR bw, UCHAR *dfs_start_idx, UCHAR *dfs_end_idx)
{
	UCHAR band_idx = pAd->CommonCfg.BandSelBand;
	UCHAR ch_start = 0, ch_end = 0, ch_start_idx = 0, ch_end_idx = 0, ch_idx = 0;
	UCHAR cent_ch, cap_bw;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	*dfs_start_idx = 0xff;
	*dfs_end_idx = 0xff;

	if (pChCtrl == NULL)
		return;

	if (band_idx != BAND_SELECT_BAND_5G)
		return;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"ch=%d, bw=%d\n", ch, bw);

	/* adjust bw if need*/
	cap_bw = get_channel_bw_cap(wdev, ch);
	if (bw > cap_bw)
		bw = cap_bw;

	/* find support ch range*/
	cent_ch = DfsPrimToCent(ch, bw);

	if (bw == BW_160) {
		ch_start = cent_ch - 14;
		ch_end = cent_ch + 14;
	} else if (bw == BW_80) {
		ch_start = cent_ch - 6;
		ch_end = cent_ch + 6;
	} else if (bw == BW_40) {
		ch_start = cent_ch - 2;
		ch_end = cent_ch + 2;
	} else {
		ch_start = cent_ch;
		ch_end = cent_ch;
	}


	/* find first ch idx*/
	for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
		if (pChCtrl->ChList[ch_idx].Channel == ch_start) {
			ch_start_idx = ch_idx;
			break;
		}
	}

	/* find last ch idx*/
	for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
		if (pChCtrl->ChList[ch_idx].Channel == ch_end) {
			ch_end_idx = ch_idx;
			break;
		}
	}
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"ch_start=%d, ch_end=%d, ch_start_id=%d, ch_end_id=%d\n",
			ch_start, ch_end, ch_start_idx, ch_end_idx);

	/* find first dfs channel idx*/
	for (ch_idx = ch_start_idx; ch_idx <= ch_end_idx; ch_idx++) {
		if (pChCtrl->ChList[ch_idx].DfsReq == TRUE) {
			*dfs_start_idx = ch_idx;
			break;
		}
	}

	/* find last dfs channel idx*/
	for (ch_idx = ch_start_idx; ch_idx <= ch_end_idx; ch_idx++) {
		if (pChCtrl->ChList[ch_idx].DfsReq == TRUE)
			*dfs_end_idx = ch_idx;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"dfs_start_idx=%d, dfs_end_idx=%d\n", *dfs_start_idx, *dfs_end_idx);

	if (*dfs_end_idx < *dfs_start_idx) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				"dfs_idx is wrong\n");
		*dfs_start_idx = 0xff;
		*dfs_end_idx = 0xff;
	}
}

UCHAR judge_dfs_range_change(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR old_ch, UCHAR old_bw, UCHAR new_ch, UCHAR new_bw)
{
	UCHAR band_idx = pAd->CommonCfg.BandSelBand;
	struct wlan_config *cfg = wdev->wpf_cfg;
	UCHAR old_dfs_start_idx = 0xff, old_dfs_end_idx = 0xff;
	UCHAR new_dfs_start_idx = 0xff, new_dfs_end_idx = 0xff;
	UCHAR ret = DFS_RANGE_NO_CHANGE;

	if (band_idx != BAND_SELECT_BAND_5G || cfg == NULL)
		return DFS_RANGE_NO_CHANGE;


	get_dfs_range(pAd, wdev, old_ch, old_bw, &old_dfs_start_idx, &old_dfs_end_idx);
	get_dfs_range(pAd, wdev, new_ch, new_bw, &new_dfs_start_idx, &new_dfs_end_idx);

	if (new_dfs_start_idx == 0xff)
		ret = DFS_RANGE_NULL;
	else if (new_dfs_start_idx < old_dfs_start_idx
			|| new_dfs_end_idx > old_dfs_end_idx)
		ret = DFS_RANGE_EXPAND;
	else if (new_dfs_start_idx == old_dfs_start_idx
			&& new_dfs_end_idx == old_dfs_end_idx)
		ret = DFS_RANGE_NO_CHANGE;
	else
		ret = DFS_RANGE_NARROW;

	return ret;
}

VOID update_cac_ctrl_status(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR old_ch, UCHAR old_bw, UCHAR new_ch, UCHAR new_bw)
{
	UCHAR dfs_range_change = DFS_RANGE_NULL;
	struct DOT11_H *pDot11h = wdev->pDot11_H;

	if (pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport == TRUE &&
		pAd->CommonCfg.DfsParameter.SetOutBandChStat == OUTB_SET_CH_CAC_DONE &&
		pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"No need to do CAC if zero wait supported\n");

		pAd->CommonCfg.DfsParameter.CacCtrl = NO_NEED_CAC;
		return;
	}

	dfs_range_change = judge_dfs_range_change(pAd, wdev, old_ch, old_bw, new_ch, new_bw);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"dfs_range_change = %d\n", dfs_range_change);

	if (dfs_range_change == DFS_RANGE_NO_CHANGE
		|| dfs_range_change == DFS_RANGE_NARROW) {
		pAd->CommonCfg.DfsParameter.CacCtrl = KEEP_CAC_STATE;
		if (pDot11h && pDot11h->ChannelMode == NORMAL_MODE)
			pAd->CommonCfg.DfsParameter.CacCtrl = NO_NEED_CAC;
	} else if (dfs_range_change == DFS_RANGE_EXPAND)
		pAd->CommonCfg.DfsParameter.CacCtrl = NEED_RESTART;
	else
		pAd->CommonCfg.DfsParameter.CacCtrl = KEEP_CAC_STATE;
}
#endif /*MT_DFS_SUPPORT*/
