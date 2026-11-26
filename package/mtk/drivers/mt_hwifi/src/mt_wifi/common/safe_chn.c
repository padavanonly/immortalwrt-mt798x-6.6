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
    safe_chn.c
*/

#ifdef WIFI_MD_COEX_SUPPORT
#include "rt_config.h"
#include "hdev/hdev.h"
#include "config_internal.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

typedef enum _ENUM_LTE_SAFE_CH_OP_T {
	LTE_SAFE_CH_OP_NO_CHNG = 0,
	LTE_SAFE_CH_OP_TURN_INTO_ALL_UNSAFE = 1,
	LTE_SAFE_CH_OP_RECOVER_FROM_ALL_UNSAFE = 2,
	LTE_SAFE_CH_OP_UNSAFE_SWITCH = 3
} ENUM_LTE_SAFE_CH_OP_T;

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

static int cmm_utl_is_bit_set(UINT_64 *flags, UCHAR bit)
{
	UINT_64	mask;

	if (bit > 63) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_ERROR, "error !!!!\n");
		return 0;
	}

	mask = 1UL << (bit);
	return ((mask & *flags) != 0);
}

static void cmm_utl_clear_bit(UINT_64 *flags, UCHAR bit)
{
	UINT_64	mask;

	if (bit > 63) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_ERROR, "error !!!!\n");
		return;
	}

	mask = 1UL << (bit);
	*flags &= (~mask);
}

static void cmm_utl_set_bit(UINT_64 *flags, UCHAR bit)
{
	UINT_64	mask;

	if (bit > 63) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_ERROR, "error !!!!\n");
		return;
	}

	mask = 1UL << (bit);
	*flags |= mask;
}

static void cmm_utl_cal_max_min_bit_num(UCHAR *min_bit, UCHAR *max_bit, UCHAR check_bit)
{
	UCHAR step = 4;
	UCHAR offset = 0;

	/*
	 * (check_bit == 13(MAX_80M_CHECK_BIT - 1)):
	 *    When the channel with 80M bandwidth is 215,
	 *    if the 13th bit (channel:215) is unsafe,
	 *    the channel {209, 213, 217, 221, 225, 229, 233}
	 *    with 20M bandwidth are all unsafe.
	 *
	*/
	if (check_bit == (MAX_80M_CHECK_BIT - 1))
		offset = 3;

	*min_bit = check_bit * step;
	*max_bit = *min_bit + step + offset;
}

static int is_24g_channel(UCHAR channel)
{
	return channel <= SAFE_CHN_END_IDX_2G4;
}

static int is_5g0_channel(UCHAR channel)
{
	return channel <= SAFE_CHN_END_IDX_5G0 &&
		channel >= SAFE_CHN_START_IDX_5G0;
}

static int is_5g1_channel(UCHAR channel)
{
	return channel <= SAFE_CHN_END_IDX_5G1 &&
		channel >= SAFE_CHN_START_IDX_5G1;
}

static int is_6g_channel(UCHAR channel)
{
	return channel <= SAFE_CHN_END_IDX_6G &&
		channel >= SAFE_CHN_START_IDX_6G;
}

static BOOLEAN get_chn_bitmask_pos(UCHAR chn_type, UCHAR channel,
						UCHAR *ch_type_idx, UCHAR *ch_bit_pos)
{
	UCHAR ch_step = 1, ch_idx_start = 0;

	if (chn_type == SAFE_CHN_TYPE_2G4) {
		ch_step = 1;
		if (is_24g_channel(channel)) {
			ch_idx_start = SAFE_CHN_START_IDX_2G4;
			*ch_type_idx = SAFE_CHN_MASK_BAND_2G4;
		} else {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_ERROR,
					"Channel(%d) out of range!!\n", channel);
			return FALSE;
		}
	} else if (chn_type == SAFE_CHN_TYPE_5G) {
		ch_step = 4;
		if (is_5g0_channel(channel)) {
			ch_idx_start = SAFE_CHN_START_IDX_5G0;
			*ch_type_idx = SAFE_CHN_MASK_BAND_5G_0;
		} else if (is_5g1_channel(channel)) {
			ch_idx_start = SAFE_CHN_START_IDX_5G1;
			*ch_type_idx = SAFE_CHN_MASK_BAND_5G_1;
		} else {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_ERROR,
					"Channel(%d) out of range!!\n", channel);
			return FALSE;
		}
	} else if (chn_type == SAFE_CHN_TYPE_6G) {
		ch_step = 4;
		if (is_6g_channel(channel)) {
			ch_idx_start = SAFE_CHN_START_IDX_6G;
			*ch_type_idx = SAFE_CHN_MASK_BAND_6G;
		} else {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_ERROR,
					"Channel(%d) out of range!!\n", channel);
			return FALSE;
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_ERROR,
				"Channel(%d) out of range!!\n", channel);
		return FALSE;
	}

	*ch_bit_pos = (channel - ch_idx_start) / ch_step;
	return TRUE;
}

static VOID convert_chn_6g_80M_to_20M(UINT32 *channel_80M, UINT64 *channel_20M)
{
	UCHAR i = 0;
	UCHAR j = 0;
	UCHAR min_bit = 0;
	UCHAR max_bit = 0;

	for (i = 0; i < MAX_80M_CHECK_BIT; i++) {
		cmm_utl_cal_max_min_bit_num(&min_bit, &max_bit, i);
		/* the specified bit of the channel with 80M bandwidth is 0 */
		if (!cmm_utl_is_bit_set((UINT64 *)channel_80M, i)) {
			for (j = min_bit; j < max_bit; j++)
				cmm_utl_clear_bit(channel_20M, j);
		} else {
			/* the specified bit of the channel with 80M bandwidth is 1 */
			for (j = min_bit; j < max_bit; j++)
				cmm_utl_set_bit(channel_20M, j);
		}
	}
}

static UCHAR get_channel_type_by_band(RTMP_ADAPTER *pAd)
{
	struct wifi_dev *wdev = NULL;

	wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	if (wdev) {
		if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G)
			return SAFE_CHN_TYPE_5G;
		else if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G)
			return SAFE_CHN_TYPE_2G4;
		else if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_6G)
			return SAFE_CHN_TYPE_6G;
	}
	return SAFE_CHN_TYPE_NONE;
}

static VOID safe_chn_list_to_bitmask(PRTMP_ADAPTER pAd, UCHAR *ch_list, UCHAR ch_cnt, UINT64 *bitmask)
{
	UCHAR i, ch_bit_pos, ch_type_idx, chn_type;

	chn_type = get_channel_type_by_band(pAd);

	for (i = 0; i < ch_cnt; i++) {
		if (get_chn_bitmask_pos(chn_type, ch_list[i], &ch_type_idx,
							&ch_bit_pos))
			cmm_utl_set_bit(&bitmask[ch_type_idx], ch_bit_pos);
	}
}

#ifndef MT_DFS_SUPPORT
static UCHAR bit_pos_to_channel(UCHAR ch_type_idx, UCHAR ch_bit_pos)
{
	UCHAR ch_step = 1, ch_idx_start = 0;
	UCHAR channel;

	if (ch_type_idx == SAFE_CHN_MASK_BAND_2G4) {
		ch_step = 1;
		ch_idx_start = SAFE_CHN_START_IDX_2G4;
	} else if (ch_type_idx == SAFE_CHN_MASK_BAND_6G) {
		ch_step = 4;
		ch_idx_start = SAFE_CHN_START_IDX_6G;
	} else {
		ch_step = 4;
		if (ch_type_idx == SAFE_CHN_MASK_BAND_5G_0)
			ch_idx_start = SAFE_CHN_START_IDX_5G0;
		else if (ch_type_idx == SAFE_CHN_MASK_BAND_5G_1)
			ch_idx_start = SAFE_CHN_START_IDX_5G1;
		else
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_ERROR,
					"ch_type_idx(%d) out of range!!\n",
					ch_type_idx);
	}

	channel = ch_bit_pos * ch_step + ch_idx_start;
	if (ch_type_idx == SAFE_CHN_MASK_BAND_2G4)
		channel = is_24g_channel(channel) ? channel : 0;
	else if (ch_type_idx == SAFE_CHN_MASK_BAND_6G)
		channel = is_6g_channel(channel) ? channel : 0;
	else if (ch_type_idx == SAFE_CHN_MASK_BAND_5G_0)
		channel = is_5g0_channel(channel) ? channel : 0;
	else if (ch_type_idx == SAFE_CHN_MASK_BAND_5G_1)
		channel = is_5g1_channel(channel) ? channel : 0;
	else
		channel = 0;
	return channel;
}

static UCHAR get_first_safe_channel_number(PRTMP_ADAPTER pAd, UINT64 *safe_chn)
{
	UCHAR ch_type_idx, ch_bit_pos = 0, ch_type;

	ch_type = get_channel_type_by_band(pAd);
	if (ch_type == SAFE_CHN_TYPE_2G4) {
		ch_type_idx = SAFE_CHN_MASK_BAND_2G4;
		ch_bit_pos = ffsll(safe_chn[ch_type_idx]);
		ch_bit_pos = ch_bit_pos ? (ch_bit_pos - 1) : ch_bit_pos;
	} else if (ch_type == SAFE_CHN_TYPE_6G) {
		ch_type_idx = SAFE_CHN_MASK_BAND_6G;
		ch_bit_pos = ffsll(safe_chn[ch_type_idx]);
		ch_bit_pos = ch_bit_pos ? (ch_bit_pos - 1) : ch_bit_pos;
	} else {
		for (ch_type_idx = SAFE_CHN_MASK_BAND_5G_0; ch_type_idx < SAFE_CHN_MASK_BAND_6G; ch_type_idx++) {
			ch_bit_pos = ffsll(safe_chn[ch_type_idx]);
			ch_bit_pos = ch_bit_pos ? (ch_bit_pos - 1) : ch_bit_pos;
			if (ch_bit_pos)
				break;
		}
	}

	return bit_pos_to_channel(ch_type_idx, ch_bit_pos);

}

static UCHAR get_first_safe_channel(PRTMP_ADAPTER pAd, UCHAR band_idx)
{
	UCHAR i;
	UCHAR chn = 0;
	UCHAR ch_type_idx;
	struct wifi_dev *wdev;
	BOOLEAN bFoundWdev = FALSE;
	UINT64 available_safe_chn_bitmask[SAFE_CHN_MASK_IDX_NUM];

	/* Get any proper wdev on the band;  */
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];
		if (wdev != NULL) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
					"type=%d, state=%d, band_idx=%d\n",
				wdev->wdev_type, wdev->if_up_down_state, HcGetBandByWdev(wdev));
			if ((wdev->wdev_type == WDEV_TYPE_AP)
				&& (wdev->if_up_down_state == TRUE)) {
				if (band_idx == HcGetBandByWdev(wdev)) {
					bFoundWdev = TRUE;
					break;
				}
			}
		}
	}

	if (!bFoundWdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_WARN,
				"No any active wdev (band_idx:%d).\n",
				band_idx);
		return STATUS_SUCCESS;
	}

	NdisZeroMemory(available_safe_chn_bitmask, sizeof(available_safe_chn_bitmask));
	/* try to find channel from (SafeChnBitmask & AvaChnBitmask) */
	for (ch_type_idx = SAFE_CHN_MASK_BAND_2G4; ch_type_idx < SAFE_CHN_MASK_IDX_NUM; ch_type_idx++)
		available_safe_chn_bitmask[ch_type_idx] = pAd->LteSafeChCtrl.SafeChnBitmask[ch_type_idx] &
												pAd->LteSafeChCtrl.AvaChnBitmask[ch_type_idx];
	chn = get_first_safe_channel_number(pAd, available_safe_chn_bitmask);

	/* There is no safe channel, try to find channel from (PwrChnBitmask & AvaChnBitmask) */
	if (!chn) {
		/* If the current channel is pwr backoff channel and
		 * there is no safe channel, stay in the current channel
		 */
		if (IsPwrChannelSafe(pAd, wdev->channel))
			chn = wdev->channel;
		else {
			/* get channel from power backoff safe channel list*/
			NdisZeroMemory(available_safe_chn_bitmask, sizeof(available_safe_chn_bitmask));
			for (ch_type_idx = SAFE_CHN_MASK_BAND_2G4; ch_type_idx < SAFE_CHN_MASK_IDX_NUM; ch_type_idx++)
				available_safe_chn_bitmask[ch_type_idx] = pAd->LteSafeChCtrl.PwrChnBitmask[ch_type_idx] &
														pAd->LteSafeChCtrl.AvaChnBitmask[ch_type_idx];
			chn = get_first_safe_channel_number(pAd, available_safe_chn_bitmask);
		}
	}
	return chn;
}
#endif

static UCHAR select_safe_channel(PRTMP_ADAPTER pAd)
{
	UCHAR new_chn = 0;
	UCHAR band_idx = hc_get_hw_band_idx(pAd);

#ifdef MT_DFS_SUPPORT
	new_chn = WrapDfsRandomSelectChannel(pAd, 0, band_idx);
#else
	new_chn = get_first_safe_channel(pAd, band_idx);
#endif

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_INFO,
			"band_idx=%d, new_chn=%d\n",
			band_idx, new_chn);
	return new_chn;
}

static BOOLEAN update_safe_chn_db(RTMP_ADAPTER *pAd,
	UINT32 *safe_channel_bit_mask, UINT32 *pwr_channel_bit_mask,
	UCHAR *safe_chan_change_type, UCHAR *pwr_chan_change_type)
{
	UCHAR idx;
	BOOLEAN bChanged = FALSE;

	*safe_chan_change_type = SAFE_CHN_TYPE_NONE;
	*pwr_chan_change_type = SAFE_CHN_TYPE_NONE;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
		"(change before) SafeChnBitmask=%llx-%llx-%llx-%llx\n",
		pAd->LteSafeChCtrl.SafeChnBitmask[SAFE_CHN_MASK_BAND_2G4],
		pAd->LteSafeChCtrl.SafeChnBitmask[SAFE_CHN_MASK_BAND_5G_0],
		pAd->LteSafeChCtrl.SafeChnBitmask[SAFE_CHN_MASK_BAND_5G_1],
		pAd->LteSafeChCtrl.SafeChnBitmask[SAFE_CHN_MASK_BAND_6G]);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
		"(change before) PwrChnBitmask=%llx-%llx-%llx-%llx\n",
		pAd->LteSafeChCtrl.PwrChnBitmask[SAFE_CHN_MASK_BAND_2G4],
		pAd->LteSafeChCtrl.PwrChnBitmask[SAFE_CHN_MASK_BAND_5G_0],
		pAd->LteSafeChCtrl.PwrChnBitmask[SAFE_CHN_MASK_BAND_5G_1],
		pAd->LteSafeChCtrl.PwrChnBitmask[SAFE_CHN_MASK_BAND_6G]);

	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	for (idx = SAFE_CHN_MASK_BAND_2G4; idx < SAFE_CHN_MASK_IDX_NUM; idx++) {
		if (idx == SAFE_CHN_MASK_BAND_6G) {
			if (pAd->LteSafeChCtrl.SafeChn6GBitmask != safe_channel_bit_mask[idx]) {
				pAd->LteSafeChCtrl.SafeChn6GBitmask = safe_channel_bit_mask[idx];
				convert_chn_6g_80M_to_20M(&(safe_channel_bit_mask[idx]),
							&(pAd->LteSafeChCtrl.SafeChnBitmask[idx]));
				*safe_chan_change_type |= SAFE_CHN_TYPE_6G;
			}

			if (pAd->LteSafeChCtrl.PwrChn6GBitmask != pwr_channel_bit_mask[idx]) {
				pAd->LteSafeChCtrl.PwrChn6GBitmask = pwr_channel_bit_mask[idx];
				convert_chn_6g_80M_to_20M(&(pwr_channel_bit_mask[idx]),
							&(pAd->LteSafeChCtrl.PwrChnBitmask[idx]));
				*pwr_chan_change_type |= SAFE_CHN_TYPE_6G;
			}
		} else {
			if (pAd->LteSafeChCtrl.SafeChnBitmask[idx] != safe_channel_bit_mask[idx]) {
				pAd->LteSafeChCtrl.SafeChnBitmask[idx] = safe_channel_bit_mask[idx];

				if (idx == SAFE_CHN_MASK_BAND_2G4)
					*safe_chan_change_type |= SAFE_CHN_TYPE_2G4;
				else
					*safe_chan_change_type |= SAFE_CHN_TYPE_5G;
			}

			if (pAd->LteSafeChCtrl.PwrChnBitmask[idx] != pwr_channel_bit_mask[idx]) {
				pAd->LteSafeChCtrl.PwrChnBitmask[idx] = pwr_channel_bit_mask[idx];

				if (idx == SAFE_CHN_MASK_BAND_2G4)
					*pwr_chan_change_type |= SAFE_CHN_TYPE_2G4;
				else
					*pwr_chan_change_type |= SAFE_CHN_TYPE_5G;
			}
		}
	}
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);

	bChanged = (*safe_chan_change_type | *pwr_chan_change_type) ? TRUE : FALSE;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
		"(change after) SafeChnBitmask=%llx-%llx-%llx-%llx, bChanged=%d(%d:%d)\n",
		pAd->LteSafeChCtrl.SafeChnBitmask[SAFE_CHN_MASK_BAND_2G4],
		pAd->LteSafeChCtrl.SafeChnBitmask[SAFE_CHN_MASK_BAND_5G_0],
		pAd->LteSafeChCtrl.SafeChnBitmask[SAFE_CHN_MASK_BAND_5G_1],
		pAd->LteSafeChCtrl.SafeChnBitmask[SAFE_CHN_MASK_BAND_6G],
		bChanged, *safe_chan_change_type, *pwr_chan_change_type);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
		"(change after) PwrChnBitmask=%llx-%llx-%llx-%llx, bChanged=%d(%d:%d)\n",
		pAd->LteSafeChCtrl.PwrChnBitmask[SAFE_CHN_MASK_BAND_2G4],
		pAd->LteSafeChCtrl.PwrChnBitmask[SAFE_CHN_MASK_BAND_5G_0],
		pAd->LteSafeChCtrl.PwrChnBitmask[SAFE_CHN_MASK_BAND_5G_1],
		pAd->LteSafeChCtrl.PwrChnBitmask[SAFE_CHN_MASK_BAND_6G],
		bChanged, *safe_chan_change_type, *pwr_chan_change_type);

	return bChanged;
}

static VOID set_safe_chn_chg_flag(RTMP_ADAPTER *pAd)
{
	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	pAd->LteSafeChCtrl.WaitForSafeChOpCnt++;
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
}

static VOID set_pwr_chn_chg_flag(RTMP_ADAPTER *pAd)
{
	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	pAd->LteSafeChCtrl.WaitForPwrChOpCnt++;
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
}

static VOID switch_unsafe_channel(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	UCHAR new_channel;
	bool pwr_backoff = FALSE;

	if (IsPwrChannelSafe(pAd, wdev->channel))
		pwr_backoff = TRUE;

	new_channel = select_safe_channel(pAd);
	if (new_channel == 0)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_WARN,
				"stay in current %s channel %d.\n",
				pwr_backoff ? "power backoff" : "unsafe",
				wdev->channel);
	else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_WARN,
				"Current channel %d is %s! Switch to channel %d.\n",
				wdev->channel,
				pwr_backoff ? "power backoff" : "unsafe",
				new_channel);
		if (!rtmp_set_channel(pAd, wdev, new_channel))
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_ERROR,
					"rtmp_set_channel failed: new_channel(%d).\n",
					new_channel);
		else
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_INFO,
					"rtmp_set_channel succeed: new_channel(%d).\n",
					new_channel);
		pAd->LteSafeChCtrl.ChnSwitchCnt++;
	}
}


static VOID band_recover_from_all_unsafe(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	BSS_STRUCT *pMbss = NULL;

	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];

	if (!pMbss) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_ERROR, "Invalid pMbss.\n");
		return;
	}

	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	pAd->LteSafeChCtrl.bAllUnsafe = FALSE;
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);

	if (!IsChannelSafe(pAd, wdev->channel))
		switch_unsafe_channel(pAd, wdev);
	else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_WARN,
				"Current channel %d is free run channel.\n", wdev->channel);
	pAd->LteSafeChCtrl.BandUpCnt++;
}

static VOID band_turn_into_all_unsafe(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	BSS_STRUCT *pMbss = NULL;

	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	if (!pMbss) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_ERROR, "Invalid pMbss.\n");
		return;
	}

	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	pAd->LteSafeChCtrl.bAllUnsafe = TRUE;
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_WARN,
			"All channels are unsafe now! Stay in Current Channel %d.\n", wdev->channel);
	pAd->LteSafeChCtrl.BandBanCnt++;
}

static BOOLEAN is_all_available_chn_unsafe(PRTMP_ADAPTER pAd)
{
	BOOLEAN b_all_unsafe = TRUE;
	UINT64 available_safe_chn_bitmask[SAFE_CHN_MASK_IDX_NUM];
	UCHAR ch_type_idx, ch_type;

	ch_type = get_channel_type_by_band(pAd);

	for (ch_type_idx = SAFE_CHN_MASK_BAND_2G4; ch_type_idx < SAFE_CHN_MASK_IDX_NUM; ch_type_idx++)
		available_safe_chn_bitmask[ch_type_idx] = (pAd->LteSafeChCtrl.SafeChnBitmask[ch_type_idx] |
												pAd->LteSafeChCtrl.PwrChnBitmask[ch_type_idx]) &
												pAd->LteSafeChCtrl.AvaChnBitmask[ch_type_idx];

	if (ch_type == SAFE_CHN_TYPE_2G4) {
		if (available_safe_chn_bitmask[SAFE_CHN_MASK_BAND_2G4] != 0)
			b_all_unsafe = FALSE;
	} else if (ch_type == SAFE_CHN_TYPE_6G) {
		if (available_safe_chn_bitmask[SAFE_CHN_MASK_BAND_6G] != 0)
			b_all_unsafe = FALSE;
	} else if (ch_type == SAFE_CHN_TYPE_5G) {
		for (ch_type_idx = SAFE_CHN_MASK_BAND_5G_0; ch_type_idx < SAFE_CHN_MASK_BAND_6G; ch_type_idx++) {
			if (available_safe_chn_bitmask[ch_type_idx] != 0)
				b_all_unsafe = FALSE;
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
			"b_all_unsafe=%d\n", b_all_unsafe);
	return b_all_unsafe;
}

static BOOLEAN is_chn_op_state_idle(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	struct DOT11_H *pDot11h = NULL;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG, "\n");
	if (scan_in_run_state(pAd, wdev)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG, "Scan is running now!\n");
		return FALSE;
	}

	pDot11h = wdev->pDot11_H;
	if (pDot11h) {
		if (pDot11h->ChannelMode != CHAN_NORMAL_MODE) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_INFO,
				"ChannelMode=%d!\n", pDot11h->ChannelMode);
			return FALSE;
		}
	}

	return TRUE;
}

VOID band_safe_chn_event_process(RTMP_ADAPTER *pAd, UCHAR band_idx)
{
	BOOLEAN need_enqueue = FALSE;
	UINT_32	WaitForSafeChOpCnt, WaitForPwrChOpCnt;

	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	WaitForSafeChOpCnt = pAd->LteSafeChCtrl.WaitForSafeChOpCnt;
	WaitForPwrChOpCnt = pAd->LteSafeChCtrl.WaitForPwrChOpCnt;
	if (WaitForSafeChOpCnt || WaitForPwrChOpCnt) {
		need_enqueue = TRUE;
		pAd->LteSafeChCtrl.WaitForSafeChOpCnt = (WaitForSafeChOpCnt) ? 0 : WaitForSafeChOpCnt;
		pAd->LteSafeChCtrl.WaitForPwrChOpCnt = (WaitForPwrChOpCnt) ? 0 : WaitForPwrChOpCnt;
	}
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);

	if (need_enqueue) {
		RTEnqueueInternalCmd(pAd, CMDTHREAD_LTE_SAFE_CHN_CHG, &band_idx, sizeof(band_idx));
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
				"Enqueue cmd for band(%d).\n", band_idx);
	}
}

/**
* LteSafeChannelInit - Init LTE safe channel.
* @pAd: pointer of the RTMP_ADAPTER
**/
VOID LteSafeChannelInit(IN PRTMP_ADAPTER	pAd)
{
	P_LTE_SAFE_CH_CTRL pSafeChCtl = &pAd->LteSafeChCtrl;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_INFO, "\n");

	pSafeChCtl->bEnabled = TRUE;
	pSafeChCtl->bQueryLteDone = FALSE;
	pSafeChCtl->SafeChnProcIntvl = UNSAFE_CHN_FIRST_PROC_TIME;	/* Delay unsafe channel active switch at wifi_init phase */
	pSafeChCtl->RcvLteEventCnt = 0;
	pSafeChCtl->SafeChnChgCnt = 0;
	pSafeChCtl->PwrChnChgCnt = 0;
	pSafeChCtl->TriggerEventIntvl = 0;
	NdisFillMemory(&pSafeChCtl->SafeChn6GBitmask, sizeof(pSafeChCtl->SafeChn6GBitmask), 0xFF);
	NdisFillMemory(&pSafeChCtl->PwrChn6GBitmask, sizeof(pSafeChCtl->PwrChn6GBitmask), 0xFF);
	NdisFillMemory(pSafeChCtl->PwrChnBitmask, sizeof(pSafeChCtl->PwrChnBitmask), 0xFF);
	NdisFillMemory(pSafeChCtl->SafeChnBitmask, sizeof(pSafeChCtl->SafeChnBitmask), 0xFF);

	NdisAcquireSpinLock(&pSafeChCtl->SafeChDbLock);
	pSafeChCtl->bAllUnsafe = FALSE;
	NdisReleaseSpinLock(&pSafeChCtl->SafeChDbLock);
	pSafeChCtl->WaitForSafeChOpCnt = 0;
	pSafeChCtl->ChnSwitchCnt = 0;
	pSafeChCtl->BandBanCnt = 0;
	pSafeChCtl->BandUpCnt = 0;
	pSafeChCtl->FailCnt = 0;
}

/**
* LteSafeChannelDeinit - Deinit LTE safe channel.
* @pAd: pointer of the RTMP_ADAPTER
**/
VOID LteSafeChannelDeinit(IN PRTMP_ADAPTER pAd)
{
	P_LTE_SAFE_CH_CTRL pSafeChCtl = &pAd->LteSafeChCtrl;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_INFO, "\n");

	pSafeChCtl->bEnabled = TRUE;
	pSafeChCtl->bQueryLteDone = FALSE;
	pSafeChCtl->SafeChnProcIntvl = UNSAFE_CHN_FIRST_PROC_TIME;	/* Delay unsafe channel active switch at wifi_init phase */
	pSafeChCtl->RcvLteEventCnt = 0;
	pSafeChCtl->SafeChnChgCnt = 0;
	pSafeChCtl->PwrChnChgCnt = 0;
	pSafeChCtl->TriggerEventIntvl = 0;
	NdisFillMemory(&pSafeChCtl->SafeChn6GBitmask, sizeof(pSafeChCtl->SafeChn6GBitmask), 0xFF);
	NdisFillMemory(&pSafeChCtl->PwrChn6GBitmask, sizeof(pSafeChCtl->PwrChn6GBitmask), 0xFF);
	NdisFillMemory(pSafeChCtl->SafeChnBitmask, sizeof(pSafeChCtl->SafeChnBitmask), 0xFF);
	NdisFillMemory(pSafeChCtl->PwrChnBitmask, sizeof(pSafeChCtl->PwrChnBitmask), 0xFF);

	NdisAcquireSpinLock(&pSafeChCtl->SafeChDbLock);
	pSafeChCtl->bAllUnsafe = FALSE;
	NdisReleaseSpinLock(&pSafeChCtl->SafeChDbLock);
	pSafeChCtl->WaitForSafeChOpCnt = 0;
	pSafeChCtl->ChnSwitchCnt = 0;
	pSafeChCtl->BandBanCnt = 0;
	pSafeChCtl->BandUpCnt = 0;
	pSafeChCtl->FailCnt = 0;
}

/**
* CheckSafeChannelChange - Check and enqueue unsafe channel change.
* @pAd: pointer of the RTMP_ADAPTER
*
**/
VOID CheckSafeChannelChange(RTMP_ADAPTER *pAd)
{
	UCHAR band_idx = hc_get_hw_band_idx(pAd);

	band_safe_chn_event_process(pAd, band_idx);
	pAd->LteSafeChCtrl.SafeChnProcIntvl = UNSAFE_CHN_PROC_INTVL;
}

/**
* LteSafeBuildChnBitmask - Build available channel bit mask.
* @pAd: pointer of the RTMP_ADAPTER
*
* This function translate basic available channel list to bitmask, for safe channel process use.
*
**/
VOID LteSafeBuildChnBitmask(PRTMP_ADAPTER pAd)
{
	UCHAR ch_list[MAX_NUM_OF_CHANNELS], ch_num, idx, ch, ch_idx = 0;
	CHANNEL_CTRL *pChCtrl = NULL;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	if (!pChCtrl) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_ERROR,
				"hc_get_channel_ctrlfailed!!\n");
		return;
	}

	NdisZeroMemory(ch_list, sizeof(ch_list));
	for (idx = 0; idx < MAX_NUM_OF_CHANNELS; idx++) {
		ch = pChCtrl->ChList[idx].Channel;
		if (ch != 0) {
			ch_list[ch_idx] = ch;
			ch_idx++;
		}
	}
	ch_num = ch_idx;

	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	NdisZeroMemory(pAd->LteSafeChCtrl.AvaChnBitmask, sizeof(pAd->LteSafeChCtrl.AvaChnBitmask));
	safe_chn_list_to_bitmask(pAd, ch_list, ch_num, pAd->LteSafeChCtrl.AvaChnBitmask);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_INFO,
			"AvaChnBitmask=%llx-%llx-%llx-%llx\n",
			pAd->LteSafeChCtrl.AvaChnBitmask[SAFE_CHN_MASK_BAND_2G4],
			pAd->LteSafeChCtrl.AvaChnBitmask[SAFE_CHN_MASK_BAND_5G_0],
			pAd->LteSafeChCtrl.AvaChnBitmask[SAFE_CHN_MASK_BAND_5G_1],
			pAd->LteSafeChCtrl.AvaChnBitmask[SAFE_CHN_MASK_BAND_6G]);
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
}

/**
* IsChannelSafe - Check whether the input channel is safe or not.
* @pAd: pointer of the RTMP_ADAPTER
* @channel: the channel number
*
* The return value is - TRUE if safe, FALSE if unsafe.
**/
BOOLEAN IsChannelSafe(PRTMP_ADAPTER pAd, UCHAR channel)
{
	BOOLEAN bSafe = FALSE;
	UCHAR ch_bit_pos, ch_type_idx, chn_type;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
			"channel=%d\n", channel);

	chn_type = get_channel_type_by_band(pAd);
	if (!get_chn_bitmask_pos(chn_type, channel, &ch_type_idx, &ch_bit_pos)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_ERROR,
				"Channel(%d) out of range!!\n", channel);
		return TRUE;
	}

	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	if (cmm_utl_is_bit_set(&(pAd->LteSafeChCtrl.SafeChnBitmask[ch_type_idx]), ch_bit_pos))
		bSafe = TRUE;
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_INFO,
			"Channel(%d) ch_type_idx(%d) ch_bit_pos(%d) is %s.\n",
			channel, ch_type_idx, ch_bit_pos, bSafe?"Safe":"Unsafe");

	return bSafe;
}

/**
* IsPwrChannelSafe - Check whether the input channel is power backoff safe or not.
* @pAd: pointer of the RTMP_ADAPTER
* @channel: the channel number
*
* The return value is - TRUE if safe, FALSE if unsafe.
**/
BOOLEAN IsPwrChannelSafe(PRTMP_ADAPTER pAd, UCHAR channel)
{
	BOOLEAN bSafe = FALSE;
	UCHAR ch_bit_pos, ch_type_idx, chn_type;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
			"channel=%d\n", channel);

	chn_type = get_channel_type_by_band(pAd);
	if (!get_chn_bitmask_pos(chn_type, channel, &ch_type_idx, &ch_bit_pos)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_ERROR,
				"Channel(%d) out of range!!\n", channel);
		return TRUE;
	}

	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	if (cmm_utl_is_bit_set(&(pAd->LteSafeChCtrl.PwrChnBitmask[ch_type_idx]), ch_bit_pos))
		bSafe = TRUE;
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_INFO,
			"Channel(%d) ch_type_idx(%d) ch_bit_pos(%d) is %s.\n",
			channel, ch_type_idx, ch_bit_pos, bSafe?"Safe":"Unsafe");

	return bSafe;
}

/**
* LteSafeChnEventHandle - Handle LTE safe channel event.
* @pAd: pointer of the RTMP_ADAPTER
* @safe_channel_bit_mask: the safe channel bit mask
* @pwr_channel_bit_mask: the power backoff channel bit mask
*
**/
VOID LteSafeChnEventHandle(RTMP_ADAPTER *pAd,
		UINT32 *safe_channel_bit_mask, UINT32 *pwr_channel_bit_mask)
{
	BOOLEAN bChanged;
	UCHAR band_chn_type;
	UCHAR safe_chan_change_type, pwr_chan_change_type;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
			"FW_event: safe_channel_bit_mask=%x-%x-%x-%x, pwr_channel_bit_mask=%x-%x-%x-%x\n",
			safe_channel_bit_mask[SAFE_CHN_MASK_BAND_2G4],
			safe_channel_bit_mask[SAFE_CHN_MASK_BAND_5G_0],
			safe_channel_bit_mask[SAFE_CHN_MASK_BAND_5G_1],
			safe_channel_bit_mask[SAFE_CHN_MASK_BAND_6G],
			pwr_channel_bit_mask[SAFE_CHN_MASK_BAND_2G4],
			pwr_channel_bit_mask[SAFE_CHN_MASK_BAND_5G_0],
			pwr_channel_bit_mask[SAFE_CHN_MASK_BAND_5G_1],
			pwr_channel_bit_mask[SAFE_CHN_MASK_BAND_6G]);

	pAd->LteSafeChCtrl.RcvLteEventCnt++;
	bChanged = update_safe_chn_db(pAd, safe_channel_bit_mask, pwr_channel_bit_mask,
				&safe_chan_change_type, &pwr_chan_change_type);
	if (!bChanged)
		return;

	if (safe_chan_change_type)
		pAd->LteSafeChCtrl.SafeChnChgCnt++;
	if (pwr_chan_change_type)
		pAd->LteSafeChCtrl.PwrChnChgCnt++;
	band_chn_type = get_channel_type_by_band(pAd);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
		"band_chn_type=%d, safe_chan_change_type=%d, pwr_chan_change_type = %d\n",
		band_chn_type, safe_chan_change_type, pwr_chan_change_type);
	if (safe_chan_change_type & band_chn_type)
		set_safe_chn_chg_flag(pAd);
	if (pwr_chan_change_type & band_chn_type)
		set_pwr_chn_chg_flag(pAd);
}

/**
* LteSafeChannelChangeProcess - Process lte safe channel change event after dequeue from cmd queue.
* @pAd: pointer of the RTMP_ADAPTER
* @CMDQelmt: band index
*
**/
NTSTATUS LteSafeChannelChangeProcess(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	BOOLEAN bUnsafe = TRUE, bOpDone = FALSE;
	BOOLEAN bFoundWdev = FALSE;
	UCHAR i, band_idx;
	struct wifi_dev *wdev;
#ifdef CONFIG_MAP_SUPPORT
	UCHAR idx = 0;
#endif
	ENUM_LTE_SAFE_CH_OP_T safe_ch_op = LTE_SAFE_CH_OP_NO_CHNG;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG, "\n");

	if (!pAd || !CMDQelmt) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_ERROR, "Invalid input!\n");
		return STATUS_UNSUCCESSFUL;
   }

	NdisMoveMemory(&band_idx, CMDQelmt->buffer, sizeof(UCHAR));

	/* Get any proper wdev on the band;  */
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];
		if (wdev != NULL) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
					"type=%d, state=%d, band_idx=%d\n",
					wdev->wdev_type, wdev->if_up_down_state, HcGetBandByWdev(wdev));
			if ((wdev->wdev_type == WDEV_TYPE_AP)
				&& (wdev->if_up_down_state == TRUE)) {
				if (band_idx == HcGetBandByWdev(wdev)) {
					bFoundWdev = TRUE;
					break;
				}
			}
		}
	}

	if (!bFoundWdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_WARN, "No any active wdev.\n");
		return STATUS_SUCCESS;
	}

	/* Check whether init ready */
	if (WDEV_BSS_STATE(wdev) < BSS_READY) {
		if (!pAd->LteSafeChCtrl.bAllUnsafe) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
					 "wdev(%d) bss not ready (state:%d)!!\n",
					 wdev->wdev_idx, WDEV_BSS_STATE(wdev));
			goto end;
		} else
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_WARN,
					 "In all unsafe case\n");
	}

	/* Check related features state, only do unsafe channel check/switch when all other featues' state is idle. */
	if (!is_chn_op_state_idle(pAd, wdev) || pAd->IoctlHandleFlag || pAd->ApCfg.iwpriv_event_flag) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
				 "Other channel op is running on wdev(%d), IoctlHandleFlag=%d, iwpriv_event_flag=%d!!\n",
				 wdev->wdev_idx, pAd->IoctlHandleFlag, pAd->ApCfg.iwpriv_event_flag);
		goto end;
	}

	if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_LTE_SAFE_CHN, FALSE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_INFO, "TakeChannelOpCharge fail for safe channel!!\n");
		goto end;
	}

	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);

	/* Classify the operation type */
	bUnsafe = is_all_available_chn_unsafe(pAd);
	if (pAd->LteSafeChCtrl.bAllUnsafe && !bUnsafe)
		safe_ch_op = LTE_SAFE_CH_OP_RECOVER_FROM_ALL_UNSAFE;
	else if (!pAd->LteSafeChCtrl.bAllUnsafe && bUnsafe)
		safe_ch_op = LTE_SAFE_CH_OP_TURN_INTO_ALL_UNSAFE;
	else if (!pAd->LteSafeChCtrl.bAllUnsafe && !bUnsafe)
		safe_ch_op = LTE_SAFE_CH_OP_UNSAFE_SWITCH;

	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);

	/* Process unsafe: 1. All turns into unsafe; 2. Recover from all unsafe; 3. Switch to safe if current channel is unsafe; */
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
			"Channel operation type on wdev(%d) is %d, bAllUnsafe = %d\n",
			wdev->wdev_idx, safe_ch_op, pAd->LteSafeChCtrl.bAllUnsafe);

	switch (safe_ch_op) {
	case LTE_SAFE_CH_OP_RECOVER_FROM_ALL_UNSAFE:
		band_recover_from_all_unsafe(pAd, wdev);
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {
			wapp_send_lte_safe_chn_event(pAd, pAd->LteSafeChCtrl.SafeChnBitmask);
			wapp_send_band_status_event(pAd, wdev, TRUE);
		}
#endif
		break;

	case LTE_SAFE_CH_OP_TURN_INTO_ALL_UNSAFE:
#ifdef CONFIG_MAP_SUPPORT
	/*let AP stop happen without apcli disconnect at AP stop*/
		if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {
			int j;
			struct wifi_dev *sta_wdev = NULL;
			for (j = 0; j < MAX_APCLI_NUM; j++) {
				sta_wdev = &pAd->StaCfg[j].wdev;

				if (sta_wdev->channel == wdev->channel)
					pAd->StaCfg[j].ApcliInfStat.Enable = FALSE;
			}
		}
#endif
		band_turn_into_all_unsafe(pAd, wdev);
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {
			wapp_send_lte_safe_chn_event(pAd, pAd->LteSafeChCtrl.SafeChnBitmask);
			wapp_send_band_status_event(pAd, wdev, FALSE);

			for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
				if (HcGetBandByWdev(&pAd->StaCfg[idx].wdev) == band_idx) {
					pAd->StaCfg[idx].ApcliInfStat.Enable = FALSE;
					ApCliIfDown(pAd);
				}
			}
		}
#endif
		break;

	case LTE_SAFE_CH_OP_UNSAFE_SWITCH:

		/* Notify wapp */
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {
			wapp_send_lte_safe_chn_event(pAd, pAd->LteSafeChCtrl.SafeChnBitmask);
			wapp_send_band_status_event(pAd, wdev, TRUE);
		}
#endif
		if (!IsChannelSafe(pAd, wdev->channel)) {

#ifdef CONFIG_MAP_SUPPORT
			/* Notify wapp about the current channel switch */
			if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {
				int j;
				struct wifi_dev *sta_wdev = NULL;
				wdev->map_lte_unsafe_ch_detect = 1;
				for (j = 0; j < MAX_APCLI_NUM; j++) {
					sta_wdev = &pAd->StaCfg[j].wdev;

					if (sta_wdev->channel == wdev->channel)
						pAd->StaCfg[j].ApcliInfStat.Enable = FALSE;
				}
			}
#endif

			switch_unsafe_channel(pAd, wdev);
		}
		break;

	default:
		break;
	}

	bOpDone = TRUE;

end:
	/* Not done yet, enqueue again; */
	if (!bOpDone) {
		set_safe_chn_chg_flag(pAd);
		pAd->LteSafeChCtrl.FailCnt++;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
				"Not processed, line up again.\n");
	} else
		ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_LTE_SAFE_CHN);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG, "end.\n");

	return STATUS_SUCCESS;
}

/**
* Set_UnsafeChannel_State - Enable or disable unsafe channel switch.
* @pAd: pointer of the RTMP_ADAPTER
* @arg: state (0: disable; 1: enable)
*
**/
INT Set_UnsafeChannel_State(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN	bEnable;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_INFO, "\n");
	if (arg == NULL || strlen(arg) == 0)
		return FALSE;

	bEnable = os_str_tol(arg, 0, 10);

	if (pAd->LteSafeChCtrl.bEnabled != bEnable) {
		pAd->LteSafeChCtrl.bEnabled = bEnable;
		if (bEnable) {
			HW_QUERY_LTE_SAFE_CHANNEL(pAd);
			pAd->LteSafeChCtrl.bQueryLteDone = TRUE;
		} else {
			/* clear all unsafe record; */
			NdisFillMemory(&(pAd->LteSafeChCtrl.SafeChn6GBitmask),
							sizeof(pAd->LteSafeChCtrl.SafeChn6GBitmask), 0xFF);
			NdisFillMemory(&(pAd->LteSafeChCtrl.PwrChn6GBitmask),
							sizeof(pAd->LteSafeChCtrl.PwrChn6GBitmask), 0xFF);
			NdisFillMemory(pAd->LteSafeChCtrl.SafeChnBitmask,
							sizeof(pAd->LteSafeChCtrl.SafeChnBitmask), 0xFF);
			NdisFillMemory(pAd->LteSafeChCtrl.PwrChnBitmask,
							sizeof(pAd->LteSafeChCtrl.PwrChnBitmask), 0xFF);
			pAd->LteSafeChCtrl.bQueryLteDone = FALSE;
		}

		MTWF_PRINT("%s unsafe channel switch.\n", bEnable?"Enable":"Disable");
	}

	return TRUE;
}

/**
* Set_UnsafeChannel_Proc - Configure unsafe channel list.
* @pAd: pointer of the RTMP_ADAPTER
* @arg: safe/power-backoff channel list (ex. 0x1:0x2:0x3:0x4-0x11:0x22:0x33:0x44)
*
* This function is for feature debug
*
**/
INT Set_UnsafeChannel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int i, buf_size;
	char *tmp = NULL;
	char *tmpFree = NULL;
	char *safeChan = NULL;
	char *pwrChan = NULL;
	UINT8 safeChCnt = 0;
	UINT8 pwrChCnt = 0;
	EVENT_LTE_SAFE_CHN_T EventLteSafeChn;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_INFO, "\n");
	if (arg == NULL || strlen(arg) == 0)
		return FALSE;

	buf_size = strlen(arg) * sizeof(RTMP_STRING);
	os_alloc_mem(NULL, (UCHAR **)&safeChan, buf_size);
	os_alloc_mem(NULL, (UCHAR **)&pwrChan, buf_size);
	if (!safeChan || !pwrChan) {
		MTWF_PRINT("fail to allocate memory for safe_chn/pwr_chn\n");
		if (safeChan)
			os_free_mem(safeChan);

		if (pwrChan)
			os_free_mem(pwrChan);
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, tmp = rstrtok(arg, "-"); tmp; tmp = rstrtok(NULL, "-"), i++) {
		switch (i) {
		case 0:
			strlcpy(safeChan, tmp, buf_size);
			while (strsep(&tmp, ":"))
				safeChCnt++;
			break;
		case 1:
			strlcpy(pwrChan, tmp, buf_size);
			while (strsep(&tmp, ":"))
				pwrChCnt++;
			break;
		default:
			MTWF_PRINT("Number of parameters exceed expectation, %d!!\n", i);
			return FALSE;
		}
	}

	MTWF_PRINT("Length of the safe channel list : %d\n", safeChCnt);
	MTWF_PRINT("Length of the pwr channel list : %d\n", pwrChCnt);
	NdisZeroMemory(&EventLteSafeChn, sizeof(EventLteSafeChn));

	tmpFree = safeChan;
	for (i = 0; i < safeChCnt; i++) {
		tmp = strsep(&safeChan, ":");
		if (tmp) {
			EventLteSafeChn.u4SafeChannelBitmask[i] = (UINT32)os_str_tol(tmp, 0, 16);
			MTWF_PRINT("safeChanBitmask[%d] = %x\n", i, EventLteSafeChn.u4SafeChannelBitmask[i]);
		}
	}
	os_free_mem(tmpFree);

	tmpFree = pwrChan;
	for (i = 0; i < pwrChCnt; i++) {
		tmp = strsep(&pwrChan, ":");
		if (tmp) {
			EventLteSafeChn.u4PwrChannelBitmask[i] = (UINT32)os_str_tol(tmp, 0, 16);
			MTWF_PRINT("pwrChanBitmask[%d] = %x\n", i, EventLteSafeChn.u4PwrChannelBitmask[i]);
		}
	}
	os_free_mem(tmpFree);

	EventLteSafeChn.ucVersion = FW_IDC_V3_VERSION;
	EventLteSafeChnHandler(pAd, (UINT8 *)&EventLteSafeChn, sizeof(EventLteSafeChn));
	return TRUE;
}

/**
* Show_UnsafeChannel_Info - Display unsafe channel info
* @pAd: pointer of the RTMP_ADAPTER
* @arg: Null
*
* This function is for feature debug
*
**/
INT Show_UnsafeChannel_Info(PRTMP_ADAPTER	 pAd, RTMP_STRING *arg)
{
	UCHAR chn_type = get_channel_type_by_band(pAd);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_INFO, "\n");
	MTWF_PRINT("LTE unsafe channel Info\n---------\n");

	MTWF_PRINT("bEnabled=%d, bQueryLteDone=%d, SafeChnProcIntvl=%d\n",
			pAd->LteSafeChCtrl.bEnabled, pAd->LteSafeChCtrl.bQueryLteDone, pAd->LteSafeChCtrl.SafeChnProcIntvl);
	MTWF_PRINT("RcvLteEventCnt=%d, SafeChnChgCnt=%d, PwrChnChgCnt =%d\n",
			pAd->LteSafeChCtrl.RcvLteEventCnt, pAd->LteSafeChCtrl.SafeChnChgCnt, pAd->LteSafeChCtrl.PwrChnChgCnt);

	MTWF_PRINT("bAllUnsafe=%d\n", pAd->LteSafeChCtrl.bAllUnsafe);
	MTWF_PRINT("WaitForSafeChOpCnt=%d\n",
		pAd->LteSafeChCtrl.WaitForSafeChOpCnt);
	MTWF_PRINT("WaitForPwrChOpCnt=%d\n",
		pAd->LteSafeChCtrl.WaitForPwrChOpCnt);
	MTWF_PRINT("ChnSwitchCnt=%d\n", pAd->LteSafeChCtrl.ChnSwitchCnt);
	MTWF_PRINT("BandBanCnt=%d\n", pAd->LteSafeChCtrl.BandBanCnt);
	MTWF_PRINT("BandUpCnt=%d\n", pAd->LteSafeChCtrl.BandUpCnt);

	MTWF_PRINT("TriggerEventIntvl=%d\n", pAd->LteSafeChCtrl.TriggerEventIntvl);

	if (chn_type == SAFE_CHN_TYPE_6G) {
		MTWF_PRINT("SafeChn6GBitmask(80M)=%x\n",
					pAd->LteSafeChCtrl.SafeChn6GBitmask);
		MTWF_PRINT("PwrChn6GBitmask(80M)=%x\n",
					pAd->LteSafeChCtrl.PwrChn6GBitmask);
	}

	MTWF_PRINT("SafeChnBitmask=%llx-%llx-%llx-%llx\n",
				pAd->LteSafeChCtrl.SafeChnBitmask[SAFE_CHN_MASK_BAND_2G4],
				pAd->LteSafeChCtrl.SafeChnBitmask[SAFE_CHN_MASK_BAND_5G_0],
				pAd->LteSafeChCtrl.SafeChnBitmask[SAFE_CHN_MASK_BAND_5G_1],
				pAd->LteSafeChCtrl.SafeChnBitmask[SAFE_CHN_MASK_BAND_6G]);
	MTWF_PRINT("PwrChnBitmask=%llx-%llx-%llx-%llx\n",
				pAd->LteSafeChCtrl.PwrChnBitmask[SAFE_CHN_MASK_BAND_2G4],
				pAd->LteSafeChCtrl.PwrChnBitmask[SAFE_CHN_MASK_BAND_5G_0],
				pAd->LteSafeChCtrl.PwrChnBitmask[SAFE_CHN_MASK_BAND_5G_1],
				pAd->LteSafeChCtrl.PwrChnBitmask[SAFE_CHN_MASK_BAND_6G]);
	MTWF_PRINT("AvaChnBitmask=%llx-%llx-%llx-%llx\n",
				pAd->LteSafeChCtrl.AvaChnBitmask[SAFE_CHN_MASK_BAND_2G4],
				pAd->LteSafeChCtrl.AvaChnBitmask[SAFE_CHN_MASK_BAND_5G_0],
				pAd->LteSafeChCtrl.AvaChnBitmask[SAFE_CHN_MASK_BAND_5G_1],
				pAd->LteSafeChCtrl.AvaChnBitmask[SAFE_CHN_MASK_BAND_6G]);

	return TRUE;
}

/**
* Trigger_UnsafeChannel_Event - Trigger unsafe channel event.
* @pAd: pointer of the RTMP_ADAPTER
* @arg: event interval (0: not send event, >0: send event interval in msecs.)
*
* This function is for feature debug
*
**/
INT Trigger_UnsafeChannel_Event(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 interval;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_INFO, "\n");
	if (arg == NULL || strlen(arg) == 0)
		return FALSE;

	interval = (UINT32) os_str_tol(arg, 0, 10);

	pAd->LteSafeChCtrl.TriggerEventIntvl = interval;
	if (interval == 0) {
		MTWF_PRINT("Shut down unsafe channel event trigger.\n");
	} else {
		MTWF_PRINT("Trigger unsafe channel event per %d msecs.\n", interval);
	}

	return TRUE;
}

/**
* MakeUpSafeChannelEvent - Make up unsafe channel event.
* @pAd: pointer of the RTMP_ADAPTER
*
* This function is for feature debug
*
**/
VOID MakeUpSafeChannelEvent(RTMP_ADAPTER *pAd)
{
	EVENT_LTE_SAFE_CHN_T EventLteSafeChn;
	UINT32 safe_bitmask_2g, safe_bitmask_5g_1, safe_bitmask_5g_2, safe_bitmask_6G;
	UINT32 pwr_bitmask_2g, pwr_bitmask_5g_1, pwr_bitmask_5g_2, pwr_bitmask_6G;

	safe_bitmask_2g = MtRandom32() & 0x7FFE;
	safe_bitmask_5g_1 = MtRandom32();
	safe_bitmask_5g_2 = MtRandom32() & 0x1FF;
	safe_bitmask_6G = MtRandom32() & 0x3FFF;

	pwr_bitmask_2g = MtRandom32() & 0x7FFE;
	pwr_bitmask_5g_1 = MtRandom32();
	pwr_bitmask_5g_2 = MtRandom32() & 0x1FF;
	pwr_bitmask_6G = MtRandom32() & 0x3FFF;

	NdisZeroMemory(&EventLteSafeChn, sizeof(EventLteSafeChn));
	EventLteSafeChn.u4SafeChannelBitmask[SAFE_CHN_MASK_BAND_2G4] = safe_bitmask_2g;
	EventLteSafeChn.u4SafeChannelBitmask[SAFE_CHN_MASK_BAND_5G_0] = safe_bitmask_5g_1;
	EventLteSafeChn.u4SafeChannelBitmask[SAFE_CHN_MASK_BAND_5G_1] = safe_bitmask_5g_2;
	EventLteSafeChn.u4SafeChannelBitmask[SAFE_CHN_MASK_BAND_6G] = safe_bitmask_6G;

	EventLteSafeChn.u4PwrChannelBitmask[SAFE_CHN_MASK_BAND_2G4] = pwr_bitmask_2g;
	EventLteSafeChn.u4PwrChannelBitmask[SAFE_CHN_MASK_BAND_5G_0] = pwr_bitmask_5g_1;
	EventLteSafeChn.u4PwrChannelBitmask[SAFE_CHN_MASK_BAND_5G_1] = pwr_bitmask_5g_2;
	EventLteSafeChn.u4PwrChannelBitmask[SAFE_CHN_MASK_BAND_6G] = pwr_bitmask_6G;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_UNSAFE, DBG_LVL_DEBUG,
			"safe_channel_bit_mask=%x-%x-%x-%x, pwr_channel_bit_mask=%x-%x-%x-%x\n",
			EventLteSafeChn.u4SafeChannelBitmask[SAFE_CHN_MASK_BAND_2G4],
			EventLteSafeChn.u4SafeChannelBitmask[SAFE_CHN_MASK_BAND_5G_0],
			EventLteSafeChn.u4SafeChannelBitmask[SAFE_CHN_MASK_BAND_5G_1],
			EventLteSafeChn.u4SafeChannelBitmask[SAFE_CHN_MASK_BAND_6G],
			EventLteSafeChn.u4PwrChannelBitmask[SAFE_CHN_MASK_BAND_2G4],
			EventLteSafeChn.u4PwrChannelBitmask[SAFE_CHN_MASK_BAND_5G_0],
			EventLteSafeChn.u4PwrChannelBitmask[SAFE_CHN_MASK_BAND_5G_1],
			EventLteSafeChn.u4PwrChannelBitmask[SAFE_CHN_MASK_BAND_6G]);

	EventLteSafeChn.ucVersion = FW_IDC_V3_VERSION;
	EventLteSafeChnHandler(pAd, (UINT8 *)&EventLteSafeChn, sizeof(EventLteSafeChn));
}

#endif

