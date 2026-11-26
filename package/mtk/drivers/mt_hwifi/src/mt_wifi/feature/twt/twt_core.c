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
    twt_core.c

    Abstract:
    Support twt mlme

    Who             When            What
    --------------  ----------      --------------------------------------------

*/

#include "rt_config.h"
#include "action.h"

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT

/* TWT control*/
VOID twt_ctrl_init(struct _RTMP_ADAPTER *ad)
{
	struct TWT_CTRL_T *twt_ctrl = &ad->twt_ctrl;
	UINT8 sch_link_idx = 0;

	os_zero_mem(twt_ctrl, sizeof(struct TWT_CTRL_T));
	NdisAllocateSpinLock(NULL, &twt_ctrl->twt_rec_lock);
	/* twt sch/usch link init. */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++)
		DlListInit(&twt_ctrl->twt_link[sch_link_idx]);

	MTWF_PRINT("%s()\n", __func__);
}

VOID twt_ctrl_exit(struct _RTMP_ADAPTER *ad)
{
	struct TWT_CTRL_T *twt_ctrl = &ad->twt_ctrl;
	UINT8 sch_link_idx = 0;

	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++)
		DlListInit(&twt_ctrl->twt_link[sch_link_idx]);

	NdisFreeSpinLock(&twt_ctrl->twt_rec_lock);
	os_zero_mem(twt_ctrl, sizeof(struct TWT_CTRL_T));

	MTWF_PRINT("%s()\n", __func__);
}

NDIS_STATUS twt_ctrl_acquire_twt_node(
	struct wifi_dev *wdev,
	struct twt_link_node **twt_node,
	UINT8 type)
{
	INT ret = 0;
	struct _RTMP_ADAPTER *ad = NULL;
	struct twt_link_node *_twt_node = NULL;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	ret = os_alloc_mem(ad, (UINT8 **)&_twt_node, sizeof(struct twt_link_node));
	if (!ret) {
		os_zero_mem((UINT8 *)_twt_node, sizeof(struct twt_link_node));
		_twt_node->type = type;
		_twt_node->state = TWT_STATE_SW_OCCUPIED;
		*twt_node = _twt_node;
	} else
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s:fail to allocate twt node\n", __func__);
	return ret;
}

VOID twt_ctrl_release_twt_node(
	struct twt_link_node *twt_node)
{
	os_free_mem(twt_node);
}

INT twt_ctrl_acquire_twt_agrt(
	struct wifi_dev *wdev,
	struct twt_link_node *twt_node,
	struct TWT_AGRT_MGMT_T *agrt_mgmt)
{
	INT ret = -EINVAL;
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_AGRT_PARA_T agrt_para = {0};

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);

	if (!twt_node || !agrt_mgmt) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
		    "ret=%d,twt_node=%p,agrt_mgmt=%p\n",
		    ret, twt_node, agrt_mgmt);
		return ret;
	}

	twt_agrt_cmd_set(&agrt_para, twt_node, TWT_AGRT_CTRL_AGRT_ALLOC, CMD_TSF_TYPE_NA);
	mt_asic_twt_agrt_alloc(wdev, &agrt_para, agrt_mgmt);

	ret = (!agrt_mgmt->sts && (agrt_mgmt->wcid == agrt_para.peer_id_grp_id))
		? 0 : -EBUSY;

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, (ret == 0) ? DBG_LVL_INFO : DBG_LVL_ERROR,
	    "ret=%d,peer_grp_id=0x%x,res_id=%d,sts=%d\n",
	    ret, agrt_mgmt->wcid, agrt_mgmt->res_id, agrt_mgmt->sts);

	return ret;
}

/*
 *  Use case:
 *  1. allocated but not used: driver needs to free it
 *  2. allocated and used: fw will free it when driver teardowns agrt
 */
INT twt_ctrl_release_twt_agrt_by_node(
	struct wifi_dev *wdev,
	struct twt_link_node *twt_node)
{
	INT ret = 0;
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_AGRT_PARA_T agrt_para = {0};

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!twt_node) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s:twt_node=NULL,caller=%pS\n", __func__, OS_TRACE);
		return -EINVAL;
	}

	twt_agrt_cmd_set(&agrt_para, twt_node, TWT_AGRT_CTRL_AGRT_FREE, CMD_TSF_TYPE_NA);
	mt_asic_twt_agrt_free(wdev, &agrt_para);

	return ret;
}

INT twt_ctrl_release_twt_agrt_by_para(
	struct wifi_dev *wdev,
	struct TWT_AGRT_PARA_T *agrt_para,
	UINT8 agrt_ctrl_flag)
{
	INT ret = 0;
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_AGRT_PARA_T update_agrt_para = {0};

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!agrt_para) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s:twt_node=NULL,caller=%pS\n", __func__, OS_TRACE);
		return -EINVAL;
	}

	os_move_mem(&update_agrt_para, agrt_para, sizeof(update_agrt_para));
	update_agrt_para.agrt_ctrl_flag = agrt_ctrl_flag;
	update_agrt_para.agrt_sp_start_tsf_low = 0;
	update_agrt_para.agrt_sp_start_tsf_high = 0;
	update_agrt_para.agrt_sp_duration = 0;
	update_agrt_para.agrt_sp_wake_intvl_mantissa = 0;
	update_agrt_para.agrt_sp_wake_intvl_exponent = 0;
	update_agrt_para.agrt_para_bitmap = 0;
	update_agrt_para.persistence = 0;
	update_agrt_para.ntbtt_before_reject = 0;

	mt_asic_twt_agrt_free(wdev, &update_agrt_para);

	return ret;
}


INT twt_ctrl_btwt_dump(struct wifi_dev *wdev, UINT32 *btwt_id_bitmap)
{
	INT ret = 0;
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	UINT8 sch_link_idx = 0;
	struct twt_link_node *curr_twt_node = NULL;
#ifdef SW_CONNECT_SUPPORT
	STA_TR_ENTRY *tr_entry = NULL;
#endif /* SW_CONNECT_SUPPORT */
#ifdef DOT11_EHT_BE
	UINT16 i = 0, j = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct RTWT_INFO_T *pRTWTInfo = NULL;
#endif /* DOT11_EHT_BE */

	if (!wdev) {
		ret = -EINVAL;
		goto end;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!ad) {
		ret = -ENOENT;
		goto end;
	}

	twt_ctrl = &ad->twt_ctrl;

	MTWF_PRINT("\n** btwt status **\n");
#ifdef DOT11_EHT_BE
	MTWF_PRINT("%-5s%-8s%-4s%-4s%-4s%-5s%-7s%-8s%-10s%-11s%-13s%-14s%-4s%-5s\n",
		"band", "btwt_id", "sp", "man", "exp", "trig", "f_type", "protect", "setup_cmd",
		"peristence", "twt_info_dis", "wake_dur_unit", "rec", "rtwt");
#else
	MTWF_PRINT("%-5s%-8s%-4s%-4s%-4s%-5s%-7s%-8s%-10s%-11s%-13s%-14s%-4s\n",
		"band", "btwt_id", "sp", "man", "exp", "trig", "f_type", "protect", "setup_cmd",
		"peristence", "twt_info_dis", "wake_dur_unit", "rec");
#endif /* DOT11_EHT_BE */

	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(curr_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			if (curr_twt_node->type != TWT_TYPE_BTWT)
				continue;
			if (curr_twt_node->state != TWT_STATE_SW_OCCUPIED)
				continue;
			(*btwt_id_bitmap) |= (1 << GET_BTWT_ID(curr_twt_node));

			MTWF_PRINT("%-5d", curr_twt_node->band);
			MTWF_PRINT("%-8d", (UINT16)GET_BTWT_ID(curr_twt_node));
			MTWF_PRINT("%-4d", curr_twt_node->agrt_sp_duration);
			MTWF_PRINT("%-4d", curr_twt_node->agrt_sp_wake_intvl_mantissa);
			MTWF_PRINT("%-4d", curr_twt_node->agrt_sp_wake_intvl_exponent);
			MTWF_PRINT("%-5d", GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_TRIGGER));
			MTWF_PRINT("%-7d", GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE));
			MTWF_PRINT("%-8d", GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_PROTECT));
			MTWF_PRINT("%-10d", curr_twt_node->twt_setup_cmd);
			MTWF_PRINT("%-11d", curr_twt_node->persistence);
			MTWF_PRINT("%-13d", curr_twt_node->twt_info_frame_dis);
			MTWF_PRINT("%-14d", curr_twt_node->wake_dur_unit);
			MTWF_PRINT("%-4d", curr_twt_node->btwt_recommendation);
#ifdef DOT11_EHT_BE
			MTWF_PRINT("%-5d", IS_RTWT_ID(curr_twt_node));
#endif /* DOT11_EHT_BE */
			MTWF_PRINT("\n");
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

#ifdef DOT11_EHT_BE
	MTWF_PRINT("\n** rtwt entry status **\n");
	MTWF_PRINT("%-5s%-8s%-18s%-14s%-14s\n", "WCID", "btwt_id", "traffic_info_ctrl",
				"tid_dl_bitmap", "tid_ul_bitmap");

	for (i = 0; VALID_UCAST_ENTRY_WCID(ad, i); i++) {
		pEntry = entry_get(ad, i);

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

#ifdef SW_CONNECT_SUPPORT
			tr_entry = tr_entry_get(ad, i);
			if (IS_SW_STA(tr_entry))
				continue;
#endif /* SW_CONNECT_SUPPORT */

		for (j = 0; j < TWT_BTWT_ID_NUM; j++) {
			pRTWTInfo = &pEntry->twt_ctrl.rtwt_info[j];

			if (pRTWTInfo->rtwt_valid) {
				MTWF_PRINT("%-5d", pEntry->wcid);
				MTWF_PRINT("%-8d", pRTWTInfo->btwt_id);
				MTWF_PRINT("0x%-16x", pRTWTInfo->traffic_info_ctrl);
				MTWF_PRINT("0x%-12x", pRTWTInfo->tid_dl_bitmap);
				MTWF_PRINT("0x%-12x", pRTWTInfo->tid_ul_bitmap);
			}
		}
	}
#endif /* DOT11_EHT_BE */

end:
	return ret;
}

INT twt_ctrl_itwt_dump(struct wifi_dev *wdev)
{
	INT ret = 0;
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	UINT8 sch_link_idx = 0;
	struct twt_link_node *curr_twt_node = NULL;

	if (!wdev) {
		ret = -EINVAL;
		goto end;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!ad) {
		ret = -ENOENT;
		goto end;
	}

	twt_ctrl = &ad->twt_ctrl;

	/* twt link list status */
	MTWF_PRINT("\n** itwt status **\n");
	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		MTWF_PRINT("*** twt_link[%d],len=%d ***\n", sch_link_idx, DlListLen(&twt_ctrl->twt_link[sch_link_idx]));
		DlListForEach(curr_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			MTWF_PRINT("\t**twt_node:tbl_i=%d,s=%d,wcid=%d,spd=%d,f_i=%d,tsf_t=%d,tsf_sch=(%x,%x),sp=%d,align=%d,tsf_info=(%x,%x),tsf_wish(%x,%x)\n",
				curr_twt_node->agrt_tbl_idx,
				curr_twt_node->state,
				curr_twt_node->peer_id_grp_id,
				curr_twt_node->suspend,
				curr_twt_node->flow_id,
				curr_twt_node->tsf_type,
				(UINT32)(curr_twt_node->schedule_sp_start_tsf & 0xffffffff),
				(UINT32)(curr_twt_node->schedule_sp_start_tsf >> 32),
				curr_twt_node->agrt_sp_duration,
				(UINT32)(curr_twt_node->schedule_sp_start_tsf & 0xffffffff) % TWT_TSF_ALIGNMNET_UINT,
				(UINT32)(curr_twt_node->agrt_sp_info_tsf & 0xffffffff),
				(UINT32)(curr_twt_node->agrt_sp_info_tsf >> 32),
				(UINT32)(curr_twt_node->agrt_sp_start_tsf & 0xffffffff),
				(UINT32)(curr_twt_node->agrt_sp_start_tsf >> 32));
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

end:
	return ret;
}

static UINT8 twt_ctrl_get_token(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	UINT8 token = 0;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	twt_ctrl = &ad->twt_ctrl;

	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	token = twt_ctrl->token++;
	if (twt_ctrl->token > 64)
		twt_ctrl->token = 0;
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

	return token;
}

static BOOLEAN twt_agrt_exist(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN UINT8 flow_id)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	BOOLEAN twt_agrt_exist = FALSE;
	struct twt_link_node *curr_twt_node = NULL;
	UINT8 sch_link_idx = 0;

	if (!wdev) {
		MTWF_DBG(NULL,DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return FALSE;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"ad=NULL, please check\n");
		return FALSE;
	}

	twt_ctrl = &ad->twt_ctrl;

	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(curr_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			if (!curr_twt_node) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				return FALSE;
			}

			if ((curr_twt_node->peer_id_grp_id == wcid) &&
			    (curr_twt_node->flow_id == flow_id)) {
				twt_agrt_exist = TRUE;
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

	return twt_agrt_exist;
}

static UINT32 twt_align_duration(UINT32 sp_duration, UINT32 alignment)
{
	UINT32 sp_duration_alignment = 0;
	UINT32 m = sp_duration % alignment;

#if (TWT_TSF_ALIGNMENT_EN == 1)
	if (m == 0)
		sp_duration_alignment = sp_duration;
	else
		sp_duration_alignment = sp_duration + (alignment - m);
#else
	sp_duration_alignment = sp_duration;
#endif /* TWT_TSF_ALIGNMENT_EN */

	return sp_duration_alignment;
}

/*
    target wake time:
    1. Decide by AP when CMD=Request/Suggest and use force=FALSE
    2. Decide by STA when CMD=Demand and use force=TRUE
*/
static VOID twt_link_insert_node(
	IN struct wifi_dev *wdev,
	IN struct twt_link_node *twt_node)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	BOOLEAN found = FALSE;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *next_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct twt_link_node *head_twt_node = NULL;
	UINT16 sp_duration = 0;

	if (!wdev || !twt_node) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=%p, twt_node=%p, NULL, please check\n",
			wdev, twt_node);
		return;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"ad=NULL, please check\n");
		return;
	}

	twt_ctrl = &ad->twt_ctrl;

	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);

	if ((twt_node->type == TWT_TYPE_INDIVIDUAL) &&
		(twt_node->tsf_type == TSF_FROM_SETUP_CMD_DEMAND)) { /* iTWT: demand */
		/* insert to unschedule link list */
		DlListAddTail(&twt_ctrl->twt_link[USCH_LINK], &twt_node->list);
	}
#ifdef DOT11_EHT_BE
	else if ((twt_node->type == TWT_TYPE_BTWT) &&
			 (twt_node->tsf_type == TSF_FROM_SETUP_CMD_DEMAND)) { /* bTWT: demand */
		/* insert to unschedule link list */
		DlListAddTail(&twt_ctrl->twt_link[USCH_LINK], &twt_node->list);
	}
#endif /* DOT11_EHT_BE */
	else { /* iTWT: suggest/resuest, bTWT */
		/* insert to schedule link list */
		sp_duration = twt_node->agrt_sp_duration << 8;
		if (DlListLen(&twt_ctrl->twt_link[SCH_LINK]) == 0) {
			/* insert as the 1st node */
			twt_node->schedule_sp_start_tsf = 0;
			DlListAddTail(&twt_ctrl->twt_link[SCH_LINK], &twt_node->list);
		} else if (DlListLen(&twt_ctrl->twt_link[SCH_LINK]) == 1) {
			curr_twt_node = DlListFirst(&twt_ctrl->twt_link[SCH_LINK], struct twt_link_node, list);

			if (!curr_twt_node) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"NULL Pointer!\n");
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				return;
			}

			if (curr_twt_node->schedule_sp_start_tsf >= sp_duration) {
				/* insert before 1st node */
				twt_node->schedule_sp_start_tsf = 0;
				DlListAddTail(&curr_twt_node->list, &twt_node->list);
			} else {
				/* insert after 1st node */
				twt_node->schedule_sp_start_tsf = curr_twt_node->schedule_sp_start_tsf +
					twt_align_duration((curr_twt_node->agrt_sp_duration) << 8, TWT_TSF_ALIGNMNET_UINT);
				DlListAdd(&curr_twt_node->list, &twt_node->list);
			}
		} else {
			/* insert at proper place */
			head_twt_node = DlListFirst(&twt_ctrl->twt_link[SCH_LINK], struct twt_link_node, list);
			DlListForEachSafe(temp_twt_node, next_twt_node, &twt_ctrl->twt_link[SCH_LINK], struct twt_link_node, list) {
				curr_twt_node = temp_twt_node;
				/* space check before 1st node */
				if (curr_twt_node == head_twt_node) {
					if (curr_twt_node->schedule_sp_start_tsf >= sp_duration) {
						/* insert before head */
						twt_node->schedule_sp_start_tsf = 0;
						DlListAddTail(&curr_twt_node->list, &twt_node->list);
						found = TRUE;
						break;
					}
				}
				/* space check after 1st node if current node is not the last node */
				if ((&curr_twt_node->list)->Next != (&twt_ctrl->twt_link[SCH_LINK])) {
					if (next_twt_node->schedule_sp_start_tsf -
						(curr_twt_node->schedule_sp_start_tsf +
						twt_align_duration(curr_twt_node->agrt_sp_duration << 8, TWT_TSF_ALIGNMNET_UINT)) >= sp_duration) {
						twt_node->schedule_sp_start_tsf = curr_twt_node->schedule_sp_start_tsf +
							twt_align_duration(curr_twt_node->agrt_sp_duration << 8, TWT_TSF_ALIGNMNET_UINT);
						DlListAdd(&curr_twt_node->list, &twt_node->list);
						found = TRUE;
						break;
					}
				}
			}
			/* insert as the tail node */
			if (!found) {
				twt_node->schedule_sp_start_tsf = curr_twt_node->schedule_sp_start_tsf +
					twt_align_duration(curr_twt_node->agrt_sp_duration << 8, TWT_TSF_ALIGNMNET_UINT);
				DlListAddTail(&twt_ctrl->twt_link[SCH_LINK], &twt_node->list);
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
}

static VOID twt_link_remove_node(
	IN struct wifi_dev *wdev,
	IN struct twt_link_node *twt_node,
	IN BOOLEAN need_lock)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;

	if (!wdev || !twt_node) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL or ptwt_link_entry=NULL, please check\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"ad=NULL, please check\n");
		return;
	}

	twt_ctrl = &ad->twt_ctrl;

	if (need_lock)
		NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	DlListDel(&twt_node->list);
	if (need_lock)
		NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

}

static UINT8 twt_check_interval(
	UINT32 twt_mantissa,
	UINT32 twt_exp,
	UINT32 peer_mantissa,
	UINT32 peer_exp)
{
	UINT64 twt_interval = 0;
	UINT64 peer_interval = 0;

	twt_interval = twt_mantissa;
	twt_interval <<= twt_exp;

	peer_interval = peer_mantissa;
	peer_interval <<= peer_exp;

	return (twt_interval == peer_interval) ? TRUE : FALSE;
}

static UINT8 twt_get_free_flow_id(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid)
{
	UINT8 flow_id = 0;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct _RTMP_ADAPTER *ad = NULL;
	UINT8 i = 0;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	entry = entry_get(ad, wcid);

	for (i = 0; i < TWT_FLOW_ID_MAX_NUM; i++) {
		if ((GET_PEER_ITWT_FID_BITMAP(entry) & (1 << i)) == 0) {
			flow_id = i;
			break;
		}
	}

	return flow_id;
}

VOID twt_agrt_cmd_set(
	OUT struct TWT_AGRT_PARA_T *agrt_para,
	IN struct twt_link_node *curr_twt_node,
	IN UINT8 agrt_ctrl_flag,
	IN UINT8 cmd_tsf_type)
{
	UINT8 i = 0;

	agrt_para->agrt_tbl_idx = curr_twt_node->agrt_tbl_idx;
	agrt_para->agrt_tbl_idx_h = (UINT8)((curr_twt_node->agrt_tbl_idx & 0xff00) >> 8);
	agrt_para->agrt_ctrl_flag = agrt_ctrl_flag;
	agrt_para->own_mac_idx = curr_twt_node->own_mac_idx;
	agrt_para->flow_id = curr_twt_node->flow_id;
	agrt_para->peer_id_grp_id = curr_twt_node->peer_id_grp_id;
	agrt_para->agrt_sp_duration = curr_twt_node->agrt_sp_duration;
	agrt_para->bss_idx = curr_twt_node->bss_idx;
	agrt_para->agrt_sp_wake_intvl_mantissa = curr_twt_node->agrt_sp_wake_intvl_mantissa;
	agrt_para->agrt_sp_wake_intvl_exponent = curr_twt_node->agrt_sp_wake_intvl_exponent;
	agrt_para->agrt_para_bitmap = curr_twt_node->agrt_para_bitmap;
	if (IS_BTWT_ID(curr_twt_node)) {
		agrt_para->persistence = curr_twt_node->persistence;
		agrt_para->ntbtt_before_reject = NTBBT_BEFORE_REJECT;
	}

	if (cmd_tsf_type == CMD_TST_TYPE_SCHEDULE) {
		agrt_para->agrt_sp_start_tsf_low = (UINT32)(curr_twt_node->schedule_sp_start_tsf_abs & 0xffffffff);
		agrt_para->agrt_sp_start_tsf_high = (UINT32)(curr_twt_node->schedule_sp_start_tsf_abs >> 32);
	} else if (cmd_tsf_type == CMD_TSF_TYPE_REQUESTER) {
		agrt_para->agrt_sp_start_tsf_low = (UINT32)(curr_twt_node->agrt_sp_start_tsf & 0xffffffff);
		agrt_para->agrt_sp_start_tsf_high = (UINT32)(curr_twt_node->agrt_sp_start_tsf >> 32);
	} else if (cmd_tsf_type == CMD_TSF_TYPE_TWT_INFO) {
		agrt_para->agrt_sp_start_tsf_low = (UINT32)(curr_twt_node->agrt_sp_info_tsf & 0xffffffff);
		agrt_para->agrt_sp_start_tsf_high = (UINT32)(curr_twt_node->agrt_sp_info_tsf >> 32);
	} else if (cmd_tsf_type == CMD_TSF_TYPE_NA) {
		agrt_para->agrt_sp_start_tsf_low = 0;
		agrt_para->agrt_sp_start_tsf_high = 0;
		agrt_para->agrt_sp_duration = 0;
		agrt_para->agrt_sp_wake_intvl_mantissa = 0;
		agrt_para->agrt_sp_wake_intvl_exponent = 0;
		agrt_para->agrt_para_bitmap = 0;
		agrt_para->persistence = 0;
		agrt_para->ntbtt_before_reject = 0;
	} else if (cmd_tsf_type == CMD_TSF_TYPE_BTWT) {
		agrt_para->agrt_sp_start_tsf_low = (UINT32)(curr_twt_node->schedule_sp_start_tsf & 0xffffffff);
		agrt_para->agrt_sp_start_tsf_high = (UINT32)(curr_twt_node->schedule_sp_start_tsf >> 32);
#ifdef DOT11_EHT_BE
		agrt_para->tid_dl_bitmap = 0;
		agrt_para->tid_ul_bitmap = 0;
		if (curr_twt_node->rtwt_traffic_info_present) {
			if (GET_TRAFFIC_INFO_CTRL_DL_TID_BITMAP_VALID(curr_twt_node->traffic_info_ctrl))
				agrt_para->tid_dl_bitmap = curr_twt_node->tid_dl_bitmap;
			if (GET_TRAFFIC_INFO_CTRL_UL_TID_BITMAP_VALID(curr_twt_node->traffic_info_ctrl))
				agrt_para->tid_ul_bitmap = curr_twt_node->tid_ul_bitmap;
		}
#endif /* DOT11_EHT_BE */
	}

	agrt_para->is_role_ap = curr_twt_node->is_role_ap;
	agrt_para->grp_member_cnt = curr_twt_node->grp_member_cnt;
	if (agrt_para->grp_member_cnt) {
		for (i = 0; i < TWT_HW_BTWT_MAX_MEMBER_CNT; i++)
			agrt_para->sta_list[i] = curr_twt_node->sta_list[i];
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"caller=%pS\n", OS_TRACE);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"tbl=%d,ctl=%d,oi=%d,fid=%d,wcid=0x%.4x,dur=%d,bi=%d,tl=0x%.8x,th=0x%.8x,man=%d,exp=%d,rol=%d,para=0x%x,p=%d,n=%d\n",
		(agrt_para->agrt_tbl_idx_h << 8) + agrt_para->agrt_tbl_idx,
		agrt_para->agrt_ctrl_flag,
		agrt_para->own_mac_idx,
		agrt_para->flow_id,
		agrt_para->peer_id_grp_id,
		agrt_para->agrt_sp_duration,
		agrt_para->bss_idx,
		agrt_para->agrt_sp_start_tsf_low,
		agrt_para->agrt_sp_start_tsf_high,
		agrt_para->agrt_sp_wake_intvl_mantissa,
		agrt_para->agrt_sp_wake_intvl_exponent,
		agrt_para->is_role_ap,
		agrt_para->agrt_para_bitmap,
		agrt_para->persistence,
		agrt_para->ntbtt_before_reject);
	if (IS_BTWT_ID(agrt_para)) {
#ifdef DOT11_EHT_BE
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"tid_dl_bitmap = 0x%x, tid_ul_bitmap = 0x%x\n",
			agrt_para->tid_dl_bitmap, agrt_para->tid_ul_bitmap);
#endif /* DOT11_EHT_BE */
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"grp_member_cnt(%d)\n", agrt_para->grp_member_cnt);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"sta_list[%d,%d,%d,%d,%d,%d,%d,%d]\n",
			agrt_para->sta_list[0],
			agrt_para->sta_list[1],
			agrt_para->sta_list[2],
			agrt_para->sta_list[3],
			agrt_para->sta_list[4],
			agrt_para->sta_list[5],
			agrt_para->sta_list[6],
			agrt_para->sta_list[7]);
	}
}

static BOOLEAN twt_setup_frame_sanity(
	IN struct itwt_ie_t *twt_ie)
{
	UINT8 len = 0;
	BOOLEAN implicit = 0;

	if (twt_ie->elem_id != IE_TWT) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" elem_id!=%d incorrect, please check\n", IE_TWT);
		return FALSE;
	}

	len = sizeof(struct itwt_ie_t) - 2;
	if (twt_ie->len != len) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"twt ie len!=%d incorrect, please check\n", len);
		return FALSE;
	}

	implicit = GET_TWT_RT_IMPLICIT_LAST(twt_ie->req_type);
	if (implicit != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"implicit!=TRUE incorrect, please check\n");
		return FALSE;
	}
/*
	if (twt_ie->channel != 0) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" channel!=0 incorrect, please check\n");
		return FALSE;
	}
*/
	return TRUE;
}

#define ITWT_AGRT_PARA_BITMAP_CHECK (TWT_AGRT_PARA_BITMAP_WAKE_DUR_UINT | \
									 TWT_AGRT_PARA_BITMAP_IS_TRIGGER | \
									 TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE | \
									 TWT_AGRT_PARA_BITMAP_IS_PROTECT)
static BOOLEAN twt_itwt_agrt_exist(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN struct itwt_ie_t *twt_ie_in,
	OUT PUINT8 flow_id)
{
	BOOLEAN twt_agrt_exist = FALSE;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	UINT8 sch_link_idx = 0;
	UINT8 wake_dur_unit = 0;
	BOOLEAN trigger = 0;
	BOOLEAN flow_type = 0;
	BOOLEAN protection = 0;
	UINT8 exponent = 0;
	UINT8 agrt_para_bitmap = 0;

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
	    "wdev=NULL, please check!\n");
		return FALSE;
	}

	if (!twt_ie_in) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
	    "twt_ie_in=NULL, please check!\n");
		return FALSE;
	}

	twt_ctrl = &ad->twt_ctrl;
	wake_dur_unit = GET_TWT_CTRL_WAKE_DUR_UNIT(twt_ie_in->control);
	trigger = GET_TWT_RT_TRIGGER(twt_ie_in->req_type);
	flow_type = GET_TWT_RT_FLOW_TYPE(twt_ie_in->req_type);
	protection = GET_TWT_RT_PROTECTION(twt_ie_in->req_type);
	exponent = GET_TWT_RT_WAKE_INTVAL_EXP(twt_ie_in->req_type);

	if (wake_dur_unit)
		agrt_para_bitmap |= TWT_AGRT_PARA_BITMAP_WAKE_DUR_UINT;
	if (trigger)
		agrt_para_bitmap |= TWT_AGRT_PARA_BITMAP_IS_TRIGGER;
	if (!flow_type)
		agrt_para_bitmap |= TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE;
	if (protection)
		agrt_para_bitmap |= TWT_AGRT_PARA_BITMAP_IS_PROTECT;

	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(curr_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			if (!curr_twt_node) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check!\n");
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				return FALSE;
			}

			/* TWT parameter is defined in ln39, page 186, IEEE P802.11ax/D7.0,
			 * Check related info to find out duplicated twt request.
			 */
			if ((curr_twt_node->peer_id_grp_id == wcid) &&
				((curr_twt_node->agrt_para_bitmap & ITWT_AGRT_PARA_BITMAP_CHECK) == agrt_para_bitmap) &&
				(curr_twt_node->agrt_sp_duration == twt_ie_in->duration) &&
				(curr_twt_node->agrt_sp_wake_intvl_mantissa == twt_ie_in->mantissa) &&
				(curr_twt_node->agrt_sp_wake_intvl_exponent == exponent) &&
				(curr_twt_node->channel == twt_ie_in->channel)) {
				if (flow_id)
					*flow_id = curr_twt_node->flow_id;
				twt_agrt_exist = TRUE;
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
						"Duplicated twt mode is found!\n");
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

	return twt_agrt_exist;
}

static INT twt_itwt_request(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN struct itwt_ie_t *twt_ie_in,
	OUT PUINT16 agrt_tbl_idx,
	OUT PUINT8 setup_cmd,
	OUT PUINT32 tsf,
	OUT PUINT8 flow_id)
{
	/*
	* 1. check existed itwt session
	* 2. allocate twt_node
	* 3. alloc agrt tbl resource
	* 4. schedule this twt_node
	* 5. prepare twt cmd
	*/
	INT ret = 0;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct _RTMP_ADAPTER *ad = NULL;

	struct twt_link_node *twt_node = NULL;
	struct TWT_AGRT_MGMT_T agrt_mgmt = {0};
	struct TWT_TX_CMD_CTRL_T *twt_cmd = NULL;

	UINT16 control = 0;
	UINT8 twt_info_frame_dis = 0;
	UINT8 wake_dur_unit = 0;
	UINT16 req_type = 0;
	UINT8 steup_cmd_in = 0;
	BOOLEAN trigger = 0;
	BOOLEAN implicit = 0;
	BOOLEAN flow_type = 0;
	UINT8 flow_identifier = 0;
	UINT8 exponent = 0;
	BOOLEAN protection = 0;
	UINT8 duration = 0;
	UINT16 mantissa = 0;
	UINT8 channel = 0;

	UINT32 current_tsf[2];
	UINT64 twt_interval = 0;
	UINT64 twt_mod = 0;
	UINT64 twt_current_tsf = 0;
	UINT64 twt_assigned_tsf = 0;
	UINT64 agrt_tsf = 0;
	UINT64 temp = 0;

	if (!wdev) {
		ret = -EINVAL;
		goto end;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!ad) {
		ret = -EINVAL;
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"ad=NULL, please check!\n");
		goto end;
	}

	entry = entry_get(ad, wcid);
	if (!IS_VALID_ENTRY(entry)) {
		ret = -EINVAL;
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"entry is invalid, please check!\n");
		goto end;
	}

	/*** check existed itwt session ***/
	/* check if max itwt session */
	if (GET_PEER_ITWT_FID_BITMAP(entry) == 0xff) {
		ret = -EPERM;
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"iTWT flow id is full! reject it!\n");
		goto end;
	}

	/* check if itwt session exist */
	flow_identifier = twt_get_free_flow_id(wdev, wcid);
	if (twt_agrt_exist(wdev, wcid, flow_identifier)) {
		ret = -EEXIST;
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
				"iTWT flow id is already exist! Ignore it!\n");
		goto end;
	}

	*flow_id = flow_identifier;

	/* Dictate TWT support */
	if (twt_is_sp_duration_tolerance(ad, twt_ie_in) == FALSE) {
		*setup_cmd = TWT_SETUP_CMD_DICTATE;
		return 0;
	}

	/*** update twt node ***/
	/* allocate twt_node */
	if (twt_ctrl_acquire_twt_node(wdev, &twt_node, TWT_TYPE_INDIVIDUAL)) {
		ret = -ENOMEM;
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"No memory available for new twt node, please check!\n");
		goto end;
	}

	/* get peer twt control filed */
	control = twt_ie_in->control;
	twt_info_frame_dis = GET_TWT_CTRL_INFO_FRM_DIS(control);
	wake_dur_unit = GET_TWT_CTRL_WAKE_DUR_UNIT(control);

	/* get peer twt request type */
	req_type = twt_ie_in->req_type;
	steup_cmd_in = GET_TWT_RT_SETUP_CMD(req_type);
	trigger = GET_TWT_RT_TRIGGER(req_type);
	implicit = GET_TWT_RT_IMPLICIT_LAST(req_type);
	flow_type = GET_TWT_RT_FLOW_TYPE(req_type);
	exponent = GET_TWT_RT_WAKE_INTVAL_EXP(req_type);
	protection = GET_TWT_RT_PROTECTION(req_type);
	duration = twt_ie_in->duration;
	mantissa = twt_ie_in->mantissa;
	channel = twt_ie_in->channel;
	agrt_tsf = twt_ie_in->target_wake_time[1];
	agrt_tsf = (agrt_tsf << 32) + twt_ie_in->target_wake_time[0];

	/* update twt_node */
	twt_node->own_mac_idx = wdev->OmacIdx;
	twt_node->flow_id = flow_identifier;
	twt_node->peer_id_grp_id = wcid;
	twt_node->agrt_sp_duration = duration;
	twt_node->bss_idx = wdev->bss_info_argument.ucBssIndex;
	twt_node->agrt_sp_wake_intvl_mantissa = mantissa;
	twt_node->agrt_sp_wake_intvl_exponent = exponent;
	twt_node->agrt_sp_start_tsf = agrt_tsf;
	twt_node->channel = channel;
	twt_node->is_role_ap = TWT_ROLE_AP;
	twt_node->twt_info_frame_dis = twt_info_frame_dis;

	CLR_AGRT_PARA_BITMAP(twt_node);
	if (wake_dur_unit)
		SET_AGRT_PARA_BITMAP(twt_node, TWT_AGRT_PARA_BITMAP_WAKE_DUR_UINT);
	if (trigger)
		SET_AGRT_PARA_BITMAP(twt_node, TWT_AGRT_PARA_BITMAP_IS_TRIGGER);
	if (!flow_type)
		SET_AGRT_PARA_BITMAP(twt_node, TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE);
	if (protection)
		SET_AGRT_PARA_BITMAP(twt_node, TWT_AGRT_PARA_BITMAP_IS_PROTECT);

	if (steup_cmd_in == TWT_SETUP_CMD_REQUEST)
		twt_node->tsf_type = TSF_FROM_SETUP_CMD_REQUEST;
	else if (steup_cmd_in == TWT_SETUP_CMD_SUGGEST)
		twt_node->tsf_type = TSF_FROM_SETUP_CMD_SUGGEST;
	else if (steup_cmd_in == TWT_SETUP_CMD_DEMAND)
		twt_node->tsf_type = TSF_FROM_SETUP_CMD_DEMAND;
	else
		twt_node->tsf_type = TSF_FROM_SETUP_CMD_REQUEST;

	/* alloc agrt tbl resource */
	ret = twt_ctrl_acquire_twt_agrt(wdev, twt_node, &agrt_mgmt);
	if (ret)
		goto end;

	twt_node->agrt_tbl_idx = agrt_mgmt.res_id;
	*agrt_tbl_idx = agrt_mgmt.res_id;

	/* schedule this twt_node to reduce SP overlap */
	twt_link_insert_node(wdev, twt_node);

	/* handle tsf */
	if ((steup_cmd_in == TWT_SETUP_CMD_REQUEST) || (steup_cmd_in == TWT_SETUP_CMD_SUGGEST)) {
		twt_get_current_tsf(wdev, current_tsf);
		twt_current_tsf = current_tsf[0] + (((UINT64)current_tsf[1]) << 32);
		twt_interval = ((UINT64)(twt_node->agrt_sp_wake_intvl_mantissa)) << twt_node->agrt_sp_wake_intvl_exponent;
		temp = twt_current_tsf - twt_node->schedule_sp_start_tsf;
		twt_mod = mod_64bit(temp, twt_interval);
		twt_node->schedule_sp_start_tsf_abs = (twt_current_tsf + (twt_interval - twt_mod));
		twt_assigned_tsf = twt_node->schedule_sp_start_tsf_abs;
	} else if (steup_cmd_in == TWT_SETUP_CMD_DEMAND) {
		twt_assigned_tsf = twt_node->agrt_sp_start_tsf;
	}

	if (entry->twt_ctrl.twt_interval_max < twt_interval) {
		UINT64 temp2;
		temp = twt_interval;
		/* Converting twt_interval from microseconds to seconds. */
		temp2 = mod_64bit(temp, 1000000);
		entry->twt_ctrl.twt_interval_max = temp;
	}

	os_move_mem(tsf, &twt_assigned_tsf, sizeof(twt_assigned_tsf));

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
	    "steup_cmd=%d,%s current_tsf(%llu) and assigned_tsf(%llu),wcid=%d,tbl_i(%d)\n",
	    steup_cmd_in,
	    (steup_cmd_in == TWT_SETUP_CMD_DEMAND) ? "STA" : "AP",
	    twt_current_tsf,
	    twt_assigned_tsf,
	    twt_node->peer_id_grp_id,
	    twt_node->agrt_tbl_idx);

	*setup_cmd = TWT_SETUP_CMD_ACCEPT;

	/* prepare twt tx cmd */
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"prepare twt cmd\n");

	twt_cmd = &entry->twt_ctrl.twt_cmd;
	twt_cmd->token = twt_ctrl_get_token(wdev);
	twt_cmd->cmd_num = 1;
	twt_cmd->cmd[0].opt |= (OPT_ITWT_REQ | OPT_SETUP_CMD_ACCEPT);
	twt_cmd->cmd[0].twt_node = twt_node;
	twt_agrt_cmd_set(&twt_cmd->cmd[0].agrt_para,
		twt_node,
		TWT_AGRT_CTRL_ADD,
		(twt_node->tsf_type == TSF_FROM_SETUP_CMD_DEMAND) ? CMD_TSF_TYPE_REQUESTER : CMD_TST_TYPE_SCHEDULE);

	return ret;
end:
	/* free agrt tbl resource for successfully already occupied resource */
	if (twt_node &&
		(!agrt_mgmt.sts && (ret != -EBUSY)))
		twt_ctrl_release_twt_agrt_by_node(wdev, twt_node);
	/* free twt_node */
	if (twt_node)
		twt_ctrl_release_twt_node(twt_node);

	*setup_cmd = TWT_SETUP_CMD_REJECT;

	return ret;
}

static INT twt_build_twt_ie(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN struct itwt_ie_t *twt_ie_in,
	OUT struct itwt_ie_t *twt_ie_out)
{
	struct _RTMP_ADAPTER *ad = NULL;
	INT ret = 0;
	UINT16 req_type = 0;
	UINT8 setup_cmd = TWT_SETUP_CMD_ACCEPT;
	BOOLEAN trigger = 0;
	BOOLEAN implicit = 0;
	BOOLEAN flow_type = 0;
	UINT8 flow_identifier = 0;
	UINT8 exponent = 0;
	BOOLEAN protection = 0;
	UINT16 agrt_tbl_idx = 0;
	UINT8 duration = 0;
	UINT16 mantissa = 0;
	UINT8 channel = 0;
	UINT32 tsf[2] = {0};

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return -EINVAL;
	}

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (!twt_setup_frame_sanity(twt_ie_in))
		setup_cmd = TWT_SETUP_CMD_REJECT;

	/* twt agrt request and come out setup_cmd (accept, reject, alternate) */
	if (setup_cmd != TWT_SETUP_CMD_REJECT) {
		/* check if itwt session parameter already exist */
		if (twt_itwt_agrt_exist(ad, wdev, wcid, twt_ie_in, &flow_identifier)) {
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
					"itwt_agrt for wcid = %d, flow id = %d already exist!\n",
					wcid, flow_identifier);
			flow_identifier = twtGetRxSetupFlowId(twt_ie_in);
			setup_cmd = TWT_SETUP_CMD_REJECT;
		} else {
			ret = twt_itwt_request(wdev, wcid, twt_ie_in,
								&agrt_tbl_idx, &setup_cmd, tsf, &flow_identifier);
		}
	}

	/* get twt_ie_in parameters */
	req_type = twt_ie_in->req_type;
	trigger = GET_TWT_RT_TRIGGER(req_type);
	implicit = GET_TWT_RT_IMPLICIT_LAST(req_type);
	flow_type = GET_TWT_RT_FLOW_TYPE(req_type);
	exponent = GET_TWT_RT_WAKE_INTVAL_EXP(req_type);
	protection = GET_TWT_RT_PROTECTION(req_type);
	if ((setup_cmd == TWT_SETUP_CMD_DICTATE) && ad) {
		struct _RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(ad->hdev_ctrl);

		duration = pChipCap->twt_sp_duration_min_num;
	} else {
		duration = twt_ie_in->duration;
	}
	mantissa = twt_ie_in->mantissa;
	channel = twt_ie_in->channel;

	/* set twt_ie_out parameters */
	/* control */
	os_zero_mem(twt_ie_out, sizeof(struct itwt_ie_t));
	twt_ie_out->elem_id = IE_TWT;
	twt_ie_out->len = sizeof(struct itwt_ie_t) - 2;

	twt_ie_out->control &= ~TWT_CTRL_NDP_PAGING_INDICATOR;
	twt_ie_out->control &= ~TWT_CTRL_RESPONDER_PM_MODE;
	twt_ie_out->control |= SET_TWT_CTRL_NEGO_TYPE(GET_TWT_CTRL_NEGO_TYPE(twt_ie_in->control));
	twt_ie_out->control |= SET_TWT_CTRL_INFO_FRM_DIS(twt_get_twt_info_frame_support(wdev) ? 0 : 1);
	twt_ie_out->control |= SET_TWT_CTRL_WAKE_DUR_UNIT(GET_TWT_CTRL_WAKE_DUR_UNIT(twt_ie_in->control));
	twt_ie_out->control |= SET_TWT_CTRL_LINKID_BITMAP_PRESENT(GET_TWT_CTRL_LINKID_BITMAP_PRESENT(twt_ie_in->control));

	/* request type */
	twt_ie_out->req_type &= ~TWT_REQ_TYPE_TWT_REQUEST;
	twt_ie_out->req_type |= SET_TWT_RT_SETUP_CMD(setup_cmd);
	twt_ie_out->req_type |= SET_TWT_RT_TRIGGER(trigger);
	twt_ie_out->req_type |= SET_TWT_RT_IMPLICIT_LAST(implicit);
	twt_ie_out->req_type |= SET_TWT_RT_FLOW_TYPE(flow_type);
	twt_ie_out->req_type |= SET_TWT_RT_FLOW_ID(flow_identifier);
	twt_ie_out->req_type |= SET_TWT_RT_WAKE_INTVAL_EXP(exponent);
	twt_ie_out->req_type |= SET_TWT_RT_PROTECTION(protection);

	/* target wake time */
	twt_ie_out->target_wake_time[0] = cpu2le32(tsf[0]);
	twt_ie_out->target_wake_time[1] = cpu2le32(tsf[1]);

	/* duration */
	twt_ie_out->duration = duration;

	/* matissa */
	twt_ie_out->mantissa = mantissa;

	/* channel */
	twt_ie_out->channel = channel;

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"%s:ret=%d,reply wcid=%d, f_id=%d, setup_cmd=%d, tbl_i=%d\n",
		__func__, ret, wcid, flow_identifier, setup_cmd, agrt_tbl_idx);

	return ret;
}

static BOOLEAN twt_teardown_request(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN UINT8 twt_flow_id,
	IN UINT8 reason)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	UINT8 sch_link_idx = 0;
	BOOLEAN found = FALSE;
	BOOLEAN process_last_wcid = FALSE;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct TWT_AGRT_PARA_T agrt_para = {0};
	struct _MAC_TABLE_ENTRY *entry = NULL;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return found;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	entry = entry_get(ad, wcid);
	twt_ctrl = &ad->twt_ctrl;

	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	/*
	 * find twt database for peer wcid + twt flow id
	 * delete: in individial
	 * delete: in grp and grp_mem_cnt=1
	 * modify: in grp & grp_member>1
	 */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					" twt_entry=NULL, please check\n");
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				return found;
			}

			if ((curr_twt_node->peer_id_grp_id == wcid) &&
				(curr_twt_node->flow_id == twt_flow_id)) {
				found = TRUE;
				process_last_wcid = TRUE;
				twt_agrt_cmd_set(&agrt_para, curr_twt_node, TWT_AGRT_CTRL_DELETE, CMD_TSF_TYPE_NA);
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

	if (found) {
		if (GET_PEER_ITWT_FID_BITMAP(entry) == 0)
			entry->twt_ctrl.twt_interval_max = 0;

		if ((reason == ITWT_TEARDOWN_BY_STA) || (reason == ITWT_TEARDOWN_BY_LINKDOWN)) {
			mt_asic_twt_agrt_update(wdev, &agrt_para);
			NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
			if (process_last_wcid &&
				twt_is_link_node_valid(ad, curr_twt_node, FALSE)) {
				CLR_PEER_ITWT_FID(entry, twt_flow_id);
				twt_link_remove_node(wdev, curr_twt_node, FALSE);
				twt_ctrl_release_twt_node(curr_twt_node);
			}
			NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
		}
		if (reason == ITWT_TEARDOWN_BY_AP) {
			struct TWT_TX_CMD_CTRL_T *twt_cmd = &entry->twt_ctrl.twt_cmd;

			twt_cmd->cmd_num = 1;
			twt_cmd->token = twt_ctrl_get_token(wdev);
			twt_cmd->cmd[0].twt_node = curr_twt_node;
			twt_agrt_cmd_set(&twt_cmd->cmd[0].agrt_para, curr_twt_node, TWT_AGRT_CTRL_DELETE, CMD_TSF_TYPE_NA);
			twt_cmd->cmd[0].opt |= OPT_ITWT_TRAEDOWN_BY_AP;
		}
	} else
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" process=FALSE, wcid=%d,flow_id=%d\n",  wcid, twt_flow_id);

	return found;
}

static UINT8 twt_handle_one_peer_with_btwt_id_leave(
	IN struct wifi_dev *wdev,
	IN struct _MAC_TABLE_ENTRY *entry,
	IN UINT8 btwt_id,
	OUT struct TWT_AGRT_PARA_T *agrt_para)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	UINT8 sch_link_idx = 0;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	UINT8 band = 0;
	UINT8 found = FALSE;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return FALSE;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"ad=NULL, please check\n");
		return FALSE;
	}

	twt_ctrl = &ad->twt_ctrl;
	band = HcGetBandByWdev(wdev);

	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	/*
	 * btwt_id = remove wcid belongs to btwt_id
	 * teardown_all_twt = remove wcid belongs to twt_id=0~31
	 */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				return found;
			}

			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band) &&
				(GET_BTWT_ID(curr_twt_node) == btwt_id) &&
				twt_check_btwt_member(curr_twt_node->sta_list, entry->wcid)) {
				twt_remove_btwt_member(curr_twt_node, entry->wcid);
				twt_agrt_cmd_set(agrt_para, curr_twt_node, TWT_AGRT_CTRL_MODIFY, CMD_TSF_TYPE_BTWT);
				found = TRUE;
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

	return found;
}

static VOID twt_handle_all_peers_in_btwt_id_leave(
	IN struct wifi_dev *wdev,
	IN UINT8 btwt_id,
	OUT UINT16 *sta_list,
	OUT UINT8 *sta_cnt,
	OUT struct TWT_AGRT_PARA_T *agrt_para)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	UINT8 sch_link_idx = 0;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	UINT8 band = 0;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"ad=NULL, please check\n");
		return;
	}

	twt_ctrl = &ad->twt_ctrl;
	band = HcGetBandByWdev(wdev);

	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	/*
	 * btwt_id = remove wcid belongs to btwt_id
	 * teardown_all_twt = remove wcid belongs to twt_id=0~31
	 */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				return;
			}

			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band) &&
				(GET_BTWT_ID(curr_twt_node) == btwt_id) &&
				(curr_twt_node->grp_member_cnt > 0)) {
				os_move_mem(sta_list, curr_twt_node->sta_list, sizeof(UINT16) * TWT_HW_BTWT_MAX_MEMBER_CNT);
				os_zero_mem(curr_twt_node->sta_list, sizeof(UINT16) * TWT_HW_BTWT_MAX_MEMBER_CNT);
				*sta_cnt = curr_twt_node->grp_member_cnt;
				curr_twt_node->grp_member_cnt = 0;
				twt_agrt_cmd_set(agrt_para, curr_twt_node, TWT_AGRT_CTRL_MODIFY, CMD_TSF_TYPE_BTWT);
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
}

VOID twt_wait_ack_inform(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN UINT8 wait_ack_sts)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct mlme_twt_wait_ack_struct msg = {0};

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);

	/* assign message */
	msg.wcid = wcid;
	msg.wait_ack_sts = wait_ack_sts;

	/* enqueue message */
	MlmeEnqueueWithWdev(ad,
		ACTION_STATE_MACHINE,
		MT2_MLME_WAIT_ACK_STS,
		sizeof(struct mlme_twt_wait_ack_struct),
		(PVOID)&msg,
		0,
		wdev, FALSE, NULL);
	RTMP_MLME_HANDLER(ad);
}

static VOID twt_wait_ack_timeout_exec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	MAC_TABLE_ENTRY *pEntry = (PMAC_TABLE_ENTRY) FunctionContext;

	if (!pEntry)
		return;

#ifdef DOT11_EHT_BE
	if (pEntry->mlo.mlo_en) {
		struct TWT_PEER_CTRL_T *twt_ctrl = NULL;

		twt_ctrl = &pEntry->twt_ctrl;
		bss_mngr_mld_twt_wait_ack_inform(pEntry->wdev,
			twt_ctrl->mld_sta_idx,
			twt_ctrl->action_linkid,
			twt_ctrl->all_linkid_bitmap,
			TWT_WAIT_ACK_TO);
	} else
#endif /* DOT11_EHT_BE  */
		twt_wait_ack_inform(pEntry->wdev, pEntry->wcid, TWT_WAIT_ACK_TO);
}

DECLARE_TIMER_FUNCTION(twt_wait_ack_timeout_exec);
BUILD_TIMER_FUNCTION(twt_wait_ack_timeout_exec);

VOID mlme_twt_wait_ack_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	UINT8 i = 0;
	UINT8 wait_ack_sts = 0;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct TWT_TX_CMD_CTRL_T *twt_cmd = NULL;
	struct wifi_dev *wdev = NULL;
	struct twt_link_node *twt_node = NULL;
	struct mlme_twt_wait_ack_struct *msg = NULL;
	BOOLEAN cancelled;
	UINT16 opt = 0;
	UINT8 cmd_num = 0;
	struct TWT_AGRT_PARA_T *agrt_para = NULL;
	struct TWT_CTRL_T *twt_ctrl = &ad->twt_ctrl;
	BOOLEAN bNeedBcnUpdate = FALSE;

	msg = (struct mlme_twt_wait_ack_struct *)&elem->Msg;
	wdev = elem->wdev;
	if (!wdev || !msg)
		return;
	entry = entry_get(ad, msg->wcid);
	twt_cmd = &entry->twt_ctrl.twt_cmd;
	cmd_num = twt_cmd->cmd_num;
	wait_ack_sts = msg->wait_ack_sts;

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"ad=%p,wdev(%s),wcid=%d,wait_ack_sts=%d,token=%d,twt_cmd_num=%d\n",
		ad, wdev->if_dev->name, msg->wcid, wait_ack_sts, twt_cmd->token, twt_cmd->cmd_num);

	RTMPCancelTimer(&entry->twt_ctrl.act_wait_ack_timer, &cancelled);
	RTMPReleaseTimer(&entry->twt_ctrl.act_wait_ack_timer, &cancelled);

	if ((wait_ack_sts == TWT_WAIT_ACK_TO) || (wait_ack_sts == TWT_WAIT_ACK_NG)) {
		for (i = 0; i < cmd_num && i < TWT_CMD_MAX_NUM; i++) {
			twt_node = twt_cmd->cmd[i].twt_node;
			opt = twt_cmd->cmd[i].opt;
			agrt_para = &twt_cmd->cmd[i].agrt_para;
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"i=%d,twt_node=%p,opt=0x%x\n", i, twt_node, opt);

			/* ITWT */
			if ((opt & OPT_ITWT_REQ) && (opt & OPT_SETUP_CMD_ACCEPT)) {
				if (twt_node) {
					/* free agrt tbl resource */
					twt_ctrl_release_twt_agrt_by_para(wdev, agrt_para,
													TWT_AGRT_CTRL_AGRT_FREE);
					/* free twt_node */
					NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
					if (twt_is_link_node_valid(ad, twt_node, FALSE)) {
						twt_link_remove_node(wdev, twt_node, FALSE);
						CLR_PEER_ITWT_FID(entry, twt_node->flow_id);
						twt_ctrl_release_twt_node(twt_node);
					}
					NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				}
			}

			if ((opt & OPT_ITWT_TRAEDOWN_BY_AP)) {
				if (twt_node) {
					/* itwt teardown & agrt tbl resource freed by fw */
					mt_asic_twt_agrt_update(wdev, agrt_para);

					/* free twt_node */
					NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
					if (twt_is_link_node_valid(ad, twt_node, FALSE)) {
						twt_link_remove_node(wdev, twt_node, FALSE);
						CLR_PEER_ITWT_FID(entry, twt_node->flow_id);
						twt_ctrl_release_twt_node(twt_node);
					}
					NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				}
			}
		}
	} else if (wait_ack_sts == TWT_WAIT_ACK_OK) {
		for (i = 0; i < cmd_num && i < TWT_CMD_MAX_NUM; i++) {
			twt_node = twt_cmd->cmd[i].twt_node;
			opt = twt_cmd->cmd[i].opt;
			agrt_para = &twt_cmd->cmd[i].agrt_para;
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"i=%d,twt_node=%p,opt=0x%x\n", i, twt_node, opt);
			/* ITWT */
			if ((opt & OPT_ITWT_REQ) && (opt & OPT_SETUP_CMD_ACCEPT)) {
				if (twt_node) {
					mt_asic_twt_agrt_update(wdev, agrt_para);

					NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
					if (twt_is_link_node_valid(ad, twt_node, FALSE))
						SET_PEER_ITWT_FID(entry, twt_node->flow_id);
					NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				}
			}
			if (opt & OPT_ITWT_TRAEDOWN_BY_AP) {
				if (twt_node) {
					/* itwt teardown & agrt tbl resource freed by fw */
					mt_asic_twt_agrt_update(wdev, agrt_para);

					/* free twt_node */
					NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
					if (twt_is_link_node_valid(ad, twt_node, FALSE)) {
						twt_link_remove_node(wdev, twt_node, FALSE);
						CLR_PEER_ITWT_FID(entry, twt_node->flow_id);
						twt_ctrl_release_twt_node(twt_node);
					}
					NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				}
			}
			/* BTWT */
			/* join/leave by TWT Setup frame */
			if ((opt & OPT_BTWT_REQ) && (opt & OPT_SETUP_CMD_ACCEPT)
				&& !(opt & OPT_BTWT_TRAEDOWN)) {
				if (twt_node) {
					mt_asic_twt_agrt_update(wdev, agrt_para);

					NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
					if (twt_is_link_node_valid(ad, twt_node, FALSE)) {
						UINT32 btwt_id = GET_BTWT_ID(twt_node);

						SET_PEER_JOIN_BTWT_ID(entry, btwt_id);
#ifdef DOT11_EHT_BE
						if (IS_RTWT_ID(twt_node)) {
							if (btwt_id < TWT_BTWT_ID_NUM) {
								entry->twt_ctrl.rtwt_info[btwt_id].btwt_id = btwt_id;
								entry->twt_ctrl.rtwt_info[btwt_id].traffic_info_ctrl = twt_node->traffic_info_ctrl;
								entry->twt_ctrl.rtwt_info[btwt_id].tid_dl_bitmap = twt_node->tid_dl_bitmap;
								entry->twt_ctrl.rtwt_info[btwt_id].tid_ul_bitmap = twt_node->tid_ul_bitmap;
								entry->twt_ctrl.rtwt_info[btwt_id].rtwt_valid = TRUE;
							}
							if (twt_node->need_bcn_update) {
								bNeedBcnUpdate = TRUE;
								twt_node->need_bcn_update = FALSE;
							}
						}
#endif /* DOT11_EHT_BE */
					}
					NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				}
			}
		}
	}

	os_zero_mem(twt_cmd, sizeof(struct TWT_TX_CMD_CTRL_T));

	/* build btwt element for bcn/probe_rsp */
	if (bNeedBcnUpdate) {
		if (bcn_bpcc_op_lock(ad, wdev, TRUE, BCN_BPCC_BTWTSET) == FALSE) {
			MTWF_PRINT("%s: bcn_bpcc_op_lock fail!\n", __func__);
			if (in_interrupt()) {
				bcn_bpcc_ct_switch(ad, wdev, BCN_BPCC_BTWTSET);
				return;
			}
		}
		twt_build_btwt_ie(wdev, BCN_BPCC_BTWTSET);
	}
}

void twt_dump_btwt_setup_frame(
	struct frame_btwt_setup *btwt_frame_in,
	struct frame_btwt_setup *btwt_frame_out)
{
	struct btwt_para_set *btwt_para_in = NULL;
	struct btwt_para_set *btwt_para_out = NULL;
	struct btwt_para_set_fix *btwt_para_fix_in = NULL;
	struct btwt_para_set_fix *btwt_para_fix_out = NULL;

	if (!btwt_frame_in || !btwt_frame_out)
		return;

	btwt_para_in = &btwt_frame_in->btwt_para[0];
	btwt_para_out = &btwt_frame_out->btwt_para[0];

	if (!btwt_para_in || !btwt_para_out)
		return;

	btwt_para_fix_in = &btwt_para_in->btwt_para_fix;
	btwt_para_fix_out = &btwt_para_out->btwt_para_fix;

	/* in twt setup frame */
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"In(twt setup frame):\nA1=%pM,A2=%pM,A3=%pM\n",
		btwt_frame_in->hdr.Addr1, btwt_frame_in->hdr.Addr2, btwt_frame_in->hdr.Addr3);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"Cat(%d),S1G(%d),Token(%d),ElemID(%d),Len(%d)\n",
		btwt_frame_in->category,
		btwt_frame_in->s1g_action,
		btwt_frame_in->token,
		btwt_frame_in->elem_id,
		btwt_frame_in->len);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"NDP(%d),PM(%d),NT(%d),TWTInfoDis(%d),WakeDurUnit(%d)\n",
		(UINT8)GET_TWT_CTRL_NDP_PAGING_INDICATOR(btwt_frame_in->control),
		(UINT8)GET_TWT_CTRL_RESPONDER_PM_MODE(btwt_frame_in->control),
		(UINT8)GET_TWT_CTRL_NEGO_TYPE(btwt_frame_in->control),
		(UINT8)GET_TWT_CTRL_INFO_FRM_DIS(btwt_frame_in->control),
		(UINT8)GET_TWT_CTRL_WAKE_DUR_UNIT(btwt_frame_in->control));

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"TR(%d),SC(%d),T(%d),LBPS(%d),FT(%d),BTR(%d),TWIE(%d)\n",
		(UINT16)GET_TWT_RT_REQUEST(btwt_para_fix_in->req_type),
		(UINT16)GET_TWT_RT_SETUP_CMD(btwt_para_fix_in->req_type),
		(UINT16)GET_TWT_RT_TRIGGER(btwt_para_fix_in->req_type),
		(UINT16)GET_TWT_RT_IMPLICIT_LAST(btwt_para_fix_in->req_type),
		(UINT16)GET_TWT_RT_FLOW_TYPE(btwt_para_fix_in->req_type),
		(UINT16)GET_TWT_RT_BTWT_REC(btwt_para_fix_in->req_type),
		(UINT16)GET_TWT_RT_WAKE_INTVAL_EXP(btwt_para_fix_in->req_type));

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"TWT(0x%x),Duration(%d),Mantissa(%d)\n",
		btwt_para_fix_in->target_wake_time,
		btwt_para_fix_in->duration,
		btwt_para_fix_in->mantissa);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"bTWTId(%d),Persistence(%d), rTWT_Traffic_Info_Present(%d)\n",
		(UINT16)GET_BTWT_INFO_BTWT_ID(btwt_para_fix_in->btwt_info),
		(UINT16)GET_BTWT_INFO_BTWT_P(btwt_para_fix_in->btwt_info),
		(UINT16)GET_BTWT_INFO_RTWT_TRAFFIC_INFO_PRESENT(btwt_para_fix_in->btwt_info));

#ifdef DOT11_EHT_BE
	if (GET_BTWT_INFO_RTWT_TRAFFIC_INFO_PRESENT(btwt_para_fix_in->btwt_info)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"traffic_info_ctrl(0x%x),tid_dl_bitmap(0x%x),tid_ul_bitmap(0x%x)\n",
		btwt_para_in->rtwt_ext_set.traffic_info_ctrl,
		btwt_para_in->rtwt_ext_set.tid_dl_bitmap,
		btwt_para_in->rtwt_ext_set.tid_ul_bitmap);
	}
#endif /* DOT11_EHT_BE */

	/* out twt setup frame */
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"Out(twt setup frame):\nA1=%pM,A2=%pM,A3=%pM\n",
		btwt_frame_out->hdr.Addr1, btwt_frame_out->hdr.Addr2, btwt_frame_out->hdr.Addr3);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"Cat(%d),S1G(%d),Token(%d),ElemID(%d),Len(%d)\n",
		btwt_frame_out->category,
		btwt_frame_out->s1g_action,
		btwt_frame_out->token,
		btwt_frame_out->elem_id,
		btwt_frame_out->len);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"NDP(%d),PM(%d),NT(%d),TWTInfoDis(%d),WakeDurUnit(%d)\n",
		(UINT8)GET_TWT_CTRL_NDP_PAGING_INDICATOR(btwt_frame_out->control),
		(UINT8)GET_TWT_CTRL_RESPONDER_PM_MODE(btwt_frame_out->control),
		(UINT8)GET_TWT_CTRL_NEGO_TYPE(btwt_frame_out->control),
		(UINT8)GET_TWT_CTRL_INFO_FRM_DIS(btwt_frame_out->control),
		(UINT8)GET_TWT_CTRL_WAKE_DUR_UNIT(btwt_frame_out->control));

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"TR(%d),SC(%d),T(%d),LBPS(%d),FT(%d),BTR(%d),TWIE(%d)\n",
		(UINT16)GET_TWT_RT_REQUEST(btwt_para_fix_out->req_type),
		(UINT16)GET_TWT_RT_SETUP_CMD(btwt_para_fix_out->req_type),
		(UINT16)GET_TWT_RT_TRIGGER(btwt_para_fix_out->req_type),
		(UINT16)GET_TWT_RT_IMPLICIT_LAST(btwt_para_fix_out->req_type),
		(UINT16)GET_TWT_RT_FLOW_TYPE(btwt_para_fix_out->req_type),
		(UINT16)GET_TWT_RT_BTWT_REC(btwt_para_fix_out->req_type),
		(UINT16)GET_TWT_RT_WAKE_INTVAL_EXP(btwt_para_fix_out->req_type));

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"TWT(0x%x),Duration(%d),Mantissa(%d)\n",
		btwt_para_fix_out->target_wake_time,
		btwt_para_fix_out->duration,
		btwt_para_fix_out->mantissa);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"bTWTId(%d),Persistence(%d)\n",
		(UINT16)GET_BTWT_INFO_BTWT_ID(btwt_para_fix_out->btwt_info),
		(UINT16)GET_BTWT_INFO_BTWT_P(btwt_para_fix_out->btwt_info));

#ifdef DOT11_EHT_BE
	if (GET_BTWT_INFO_RTWT_TRAFFIC_INFO_PRESENT(btwt_para_fix_out->btwt_info)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"traffic_info_ctrl(0x%x),tid_dl_bitmap(0x%x),tid_ul_bitmap(0x%x)\n",
		btwt_para_out->rtwt_ext_set.traffic_info_ctrl,
		btwt_para_out->rtwt_ext_set.tid_dl_bitmap,
		btwt_para_out->rtwt_ext_set.tid_ul_bitmap);
	}
#endif /* DOT11_EHT_BE */
}

void twt_dump_btwt_mlme_teardown_frame(
	struct frame_btwt_teardown *mlme_teardown_frame)
{
	if (!mlme_teardown_frame)
		return;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"mlme_teardown_frame:\nA1=%pM,A2=%pM,A3=%pM\n",
		mlme_teardown_frame->hdr.Addr1, mlme_teardown_frame->hdr.Addr2, mlme_teardown_frame->hdr.Addr3);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"Cat(%d),S1G(%d),bTWTID(%d),NT(%d),AllTWT(%d)\n",
		mlme_teardown_frame->category,
		mlme_teardown_frame->s1g_action,
		(UINT8)GET_BTWT_FLOW_BTWT_ID(mlme_teardown_frame->twt_flow),
		(UINT8)GET_BTWT_FLOW_NEGO_TYPE(mlme_teardown_frame->twt_flow),
		(UINT8)GET_BTWT_FLOW_TEARDOWN_ALL_TWT(mlme_teardown_frame->twt_flow));
}

#ifdef TWT_TX_ACTION_BY_TXS
static int twt_tx_action_frame_txs_callback(
	void *arg,
	struct txs_info_t *txs_info)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct wifi_dev *wdev = NULL;
	struct TWT_TX_ACTION_REQ_ARG_T *callback_arg = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	int ret = 0;
	UINT16 u2Wcid = 0;
	UINT8 wait_ack_sts = TWT_WAIT_ACK_OK;

	if (!arg || !txs_info) {
		ret = -EINVAL;
		goto err;
	}
	callback_arg = arg;

	if (txs_info->txs_sts == TXS_STS_OK)
		wait_ack_sts = TWT_WAIT_ACK_OK;
	else if (txs_info->txs_sts == TXS_STS_TO)
		wait_ack_sts = TWT_WAIT_ACK_TO;
	else
		wait_ack_sts = TWT_WAIT_ACK_NG;

	wdev = callback_arg->wdev;
	u2Wcid = callback_arg->u2WlanIdx;

	if (!wdev) {
		ret = -EINVAL;
		goto err;
	}
	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);

	if (!VALID_UCAST_ENTRY_WCID(ad, u2Wcid)) {
		ret = -EINVAL;
		goto err;
	}

	entry = entry_get(ad, u2Wcid);
	if (!IS_VALID_ENTRY(entry)) {
		ret = -EINVAL;
		goto err;
	}

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		 "u2Wcid = %d, wait_ack_sts = %d\n", u2Wcid, wait_ack_sts);

#ifdef DOT11_EHT_BE
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"bss=%d,wcid=%d,mlo_en=%d,token=%d,mld_sta_idx=%d,action_linkid=%d,all_linkid_bitmap=0x%x\n",
		callback_arg->ucBssIndex,
		u2Wcid,
		callback_arg->ucMloEn,
		callback_arg->ucToken,
		callback_arg->u2MldStaIndx,
		callback_arg->ucActionLinkid,
		callback_arg->u2AllLinkidBitmap);

	if (callback_arg->ucMloEn)
		bss_mngr_mld_twt_wait_ack_inform(wdev,
			callback_arg->u2MldStaIndx,
			callback_arg->ucActionLinkid,
			callback_arg->u2AllLinkidBitmap,
			wait_ack_sts);
	else
#endif /* DOT11_EHT_BE */
		twt_wait_ack_inform(wdev, u2Wcid, wait_ack_sts);

	return ret;

err:
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
		 "err = %d\n", ret);
	return ret;
}

static VOID twt_tx_action_frame_by_txs(
	struct wifi_dev *wdev,
	UINT16 wcid, UINT8 *buf,
	UINT32 len,
	struct MLO_TWT_PRIV_T *priv)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	UINT8 mlo_en = 0;
	struct TWT_PEER_CTRL_T *twt_ctrl;
	struct txs_callback_info_t *callback = NULL;
	struct TWT_TX_ACTION_REQ_ARG_T *callback_arg = NULL;

	if (!wdev)
		return;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);

	hex_dump("twt_tx_action_frame_by_txs", buf, len);

	entry = entry_get(ad, wcid);
	if (!IS_VALID_ENTRY(entry))
		return;

	if (TxsInitCallbackInfo(ad,
		&callback,
		twt_tx_action_frame_txs_callback,
		(VOID **)&callback_arg,
		sizeof(struct TWT_TX_ACTION_REQ_ARG_T))
		== NDIS_STATUS_SUCCESS) {
		twt_ctrl = &entry->twt_ctrl;

#ifdef DOT11_EHT_BE
		if (entry->mlo.mlo_en &&
			priv &&
			(priv->state == STATE_TX_AGGREGATE_ACTION_FRAME)) {
			mlo_en = 1;
			twt_ctrl->mld_sta_idx = priv->mld_sta_idx;
			twt_ctrl->action_linkid = priv->action_linkid;
			twt_ctrl->all_linkid_bitmap = priv->all_linkid_bitmap;
		}
#endif /* DOT11_EHT_BE */
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"wdev(%s),BssIdx=%d,wcid=%d,token=%d,mlo_en=%d,mld_sta_idx=%d,action_linkid=%d,all_linkid_bitmap=0x%x\n",
				wdev->if_dev->name, wdev->bss_info_argument.ucBssIndex, wcid,
				twt_ctrl->twt_cmd.token, mlo_en, twt_ctrl->mld_sta_idx,
				twt_ctrl->action_linkid, twt_ctrl->all_linkid_bitmap);

		callback_arg->wdev = wdev;
		callback_arg->u2WlanIdx = wcid;
		callback_arg->ucBssIndex = wdev->bss_info_argument.ucBssIndex;
		callback_arg->ucToken = twt_ctrl->twt_cmd.token;
		callback_arg->ucMloEn = mlo_en;
		if (mlo_en) {
			callback_arg->u2MldStaIndx = twt_ctrl->mld_sta_idx;
			callback_arg->ucActionLinkid = twt_ctrl->action_linkid;
			callback_arg->u2AllLinkidBitmap = twt_ctrl->all_linkid_bitmap;
		} else {
			callback_arg->u2MldStaIndx = MLD_STA_NONE;
		}
		if (MiniportMMRequest(ad, QID_AC_BE,
			buf, len, callback) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"Can't send action frame successfully due to MiniportMMRequest fail!\n");
			TxsFreeCallbackInfo(callback);
		} else {
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					"Send TWT action frame successfully!\n");
		}
	} else {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"Can't send action frame successfully due to TxsInitCallbackInfo fail!\n");
	}
}
#endif /* TWT_TX_ACTION_BY_TXS */

static VOID twt_tx_action_frame(
	struct wifi_dev *wdev,
	UINT16 wcid, UINT8 *buf,
	UINT32 len,
	struct MLO_TWT_PRIV_T *priv,
	BOOLEAN offload)
{
#ifdef TWT_TX_ACTION_BY_TXS
	twt_tx_action_frame_by_txs(wdev, wcid, buf, len, priv);
#else
	struct _RTMP_ADAPTER *ad = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	UINT8 mlo_en = 0;
	struct TWT_PEER_CTRL_T *twt_ctrl;
	BOOLEAN cancelled;

	if (!wdev)
		return;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);

	hex_dump("twt_tx_action_frame", buf, len);

	entry = entry_get(ad, wcid);
	if (!IS_VALID_ENTRY(entry))
		return;

	/* timer is in action link peer entry */
	twt_ctrl = &entry->twt_ctrl;
	RTMPCancelTimer(&entry->twt_ctrl.act_wait_ack_timer, &cancelled);
	RTMPReleaseTimer(&entry->twt_ctrl.act_wait_ack_timer, &cancelled);
	os_zero_mem(&twt_ctrl->act_wait_ack_timer, sizeof(RALINK_TIMER_STRUCT));
	RTMPInitTimer(ad, &twt_ctrl->act_wait_ack_timer, GET_TIMER_FUNCTION(twt_wait_ack_timeout_exec), entry, FALSE);
	RTMPSetTimer(&twt_ctrl->act_wait_ack_timer, TWT_WAIT_ACT_TIME);

#ifdef DOT11_EHT_BE
	if (entry->mlo.mlo_en &&
		priv &&
		(priv->state == STATE_TX_AGGREGATE_ACTION_FRAME)) {
		mlo_en = 1;
		twt_ctrl->mld_sta_idx = priv->mld_sta_idx;
		twt_ctrl->action_linkid = priv->action_linkid;
		twt_ctrl->all_linkid_bitmap = priv->all_linkid_bitmap;
	}
#endif /* DOT11_EHT_BE */

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"wdev(%s),BssIdx=%d,wcid=%d,token=%d,mlo_en=%d,mld_sta_idx=%d,action_linkid=%d,all_linkid_bitmap=0x%x\n",
			wdev->if_dev->name, wdev->bss_info_argument.ucBssIndex, wcid,
			twt_ctrl->twt_cmd.token, mlo_en, twt_ctrl->mld_sta_idx,
			twt_ctrl->action_linkid, twt_ctrl->all_linkid_bitmap);

	if (offload) {
		mt_asic_twt_mgmt_frame_offload(wdev,
										wdev->bss_info_argument.ucBssIndex,
										wcid,
										twt_ctrl->twt_cmd.token,
										mlo_en,
										twt_ctrl->mld_sta_idx,
										twt_ctrl->action_linkid,
										twt_ctrl->all_linkid_bitmap,
										buf,
										len);
	} else {
		MiniportMMRequest(ad,
			(MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[QID_AC_VO]),
			buf, len, NULL);

		{
			INT ret = 0;
			UINT8 *data = NULL;
			UINT8 len = sizeof(struct UNI_EVENT_TWT_SYNC_T) + sizeof(struct UNI_EVENT_TWT_ACTION_TXS_INFO_T);

			ret = os_alloc_mem(ad, (UINT8 **)&data, len);
			if (!ret) {
				struct UNI_EVENT_TWT_SYNC_T *twt_event_commom = (struct UNI_EVENT_TWT_SYNC_T *)data;
				struct UNI_EVENT_TWT_ACTION_TXS_INFO_T *twt_event_body = (struct UNI_EVENT_TWT_ACTION_TXS_INFO_T *)twt_event_commom->aucTlvBuffer;

				os_zero_mem(data, len);
				twt_event_body->u2Tag = 1;
				twt_event_body->u2Length = sizeof(struct UNI_EVENT_TWT_ACTION_TXS_INFO_T);
				twt_event_body->u2WlanIdx = wcid;
				twt_event_body->ucBssIndex = wdev->bss_info_argument.ucBssIndex;
				twt_event_body->ucToken = twt_ctrl->twt_cmd.token;
				twt_event_body->ucMloEn = mlo_en;
				twt_event_body->u2MldStaIndx = twt_ctrl->mld_sta_idx;
				twt_event_body->ucActionLinkid = twt_ctrl->action_linkid;
				twt_event_body->u2AllLinkidBitmap = twt_ctrl->all_linkid_bitmap;
				twt_event_body->ucStatus = 0;
				UniEventTWTSyncHandler(ad->physical_dev, data, len, NULL);
				os_free_mem(data);
			}
		}
	}
#endif /* TWT_TX_ACTION_BY_TXS */
}

/* TWT action frame state machine management (for peer STA role) */
VOID peer_twt_setup_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct wifi_dev *wdev = elem->wdev;
	UINT8 nego_type = TWT_CTRL_NEGO_TYPE_ITWT;
	struct frame_twt_setup *frame = (struct frame_twt_setup *)&elem->Msg;

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	nego_type = GET_TWT_CTRL_NEGO_TYPE(frame->control);

	if (nego_type == TWT_CTRL_NEGO_TYPE_ITWT) {
		struct frame_itwt_setup *frame_in = (struct frame_itwt_setup *)&elem->Msg;
		struct frame_itwt_setup frame_out;
		struct itwt_ie_t *twt_ie_in = &frame_in->twt_ie;
		struct itwt_ie_t *twt_ie_out = &frame_out.twt_ie;

		if (!twt_ie_in) {
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				" twt_ie_in == NULL!!!!\n");
			return;
		}

#ifdef APCLI_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_STA) {
			UCHAR ucTWTFlowId;
			struct _STA_ADMIN_CONFIG *apcli_entry = NULL;

			apcli_entry = &ad->StaCfg[wdev->func_idx];
			ucTWTFlowId = twtGetRxSetupFlowId(twt_ie_in);

			twtParseTWTElement(twt_ie_in, &(apcli_entry->arTWTFlow[ucTWTFlowId].rTWTPeerParams));
			twtReqFsmRunEventRxSetup(ad, wdev, ucTWTFlowId);
		} else
#endif /* APCLI_SUPPORT */
		{
			UINT8 build_ie = 0;
			UINT8 tx_action = 0;
#ifdef DOT11_EHT_BE
			UINT8 breakdown_ie = 0;
			UINT8 agg_ie = 0;
			struct MLO_TWT_PRIV_T *mlo_twt_priv =
				(struct MLO_TWT_PRIV_T *)elem->Others;
#endif /* DOT11_EHT_BE */

			/* fill action frame content */
#ifdef DOT11_EHT_BE
			/* breakdown_ie->build_ie->agg_ie->tx_action when MLO iTWT */
			if (!mlo_twt_priv) {
				build_ie = 1;
				tx_action = 1;
			} else {
				if (mlo_twt_priv->state == STATE_NO_LINKID_BITMAP_PRESENT) {
					build_ie = 1;
					tx_action = 1;
				} else if (mlo_twt_priv->state == STATE_BREAKDOWN_ACTION_FRAME) {
					breakdown_ie = 1;
				} else if (mlo_twt_priv->state == STATE_ACTION_FRAME_HANDLE) {
					build_ie = 1;
					agg_ie = 1;
				} else if (mlo_twt_priv->state == STATE_TX_AGGREGATE_ACTION_FRAME) {
					tx_action = 1; // TX N->1 action frame
				} else
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						"fail. state=%d\n", mlo_twt_priv->state);
			}

			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"mlo_twt_priv=%p,breakdown_ie=%d,build_ie=%d,agg_ie=%d,tx_action=%d\n",
				mlo_twt_priv, breakdown_ie, build_ie, agg_ie, tx_action);

			/* this ad = action link and only action link ad does breakdown */
			if (breakdown_ie)
				bss_mngr_mld_twt_action_frm_breadown(ad, elem);
#else
			/* build_ie->tx_action when <= EHT_BE */
			build_ie = 1;
			tx_action = 1;
#endif /* DOT11_EHT_BE */

			/* each link ad MLME does building ie */
			if (build_ie) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					"frame_in: A1=%pM,A2=%pM,A3=%pM\n",
					frame_in->hdr.Addr1, frame_in->hdr.Addr2, frame_in->hdr.Addr3);
				os_zero_mem(&frame_out, sizeof(struct frame_itwt_setup));
				ActHeaderInit(ad, &frame_out.hdr, frame_in->hdr.Addr2, wdev->if_addr, wdev->bssid);
				frame_out.category = CATEGORY_UNPROT_S1G;
				frame_out.s1g_action = CATE_S1G_ACTION_TWT_SETUP;
				frame_out.token = frame_in->token;
				twt_build_twt_ie(wdev, elem->Wcid, twt_ie_in, twt_ie_out);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					"frame_out:A1=%pM,A2=%pM,A3=%pM\n",
					frame_out.hdr.Addr1, frame_out.hdr.Addr2, frame_out.hdr.Addr3);
			}
#ifdef DOT11_EHT_BE
			/* bss_mngr integrates per link action rsp to one action frame */
			if (agg_ie)
				bss_mngr_mld_twt_action_frm_agg(wdev, &frame_out, mlo_twt_priv);
#endif /* DOT11_EHT_BE */

			if (tx_action) {
#ifdef DOT11_EHT_BE
				/* mlo case: A1=peer_entry A2=A3=bssid assigned in bss_mngr */
				if (mlo_twt_priv) {
					if (mlo_twt_priv->state == STATE_NO_LINKID_BITMAP_PRESENT)
						twt_tx_action_frame(wdev, elem->Wcid, (UINT8 *)&frame_out, sizeof(frame_out), mlo_twt_priv, TRUE);
					else
						twt_tx_action_frame(wdev, elem->Wcid, (UINT8 *)&elem->Msg, elem->MsgLen, mlo_twt_priv, TRUE);
				} else
#endif /* DOT11_EHT_BE */
					twt_tx_action_frame(wdev, elem->Wcid, (UINT8 *)&frame_out, sizeof(frame_out), NULL, TRUE);
			}
		}
	} else if (nego_type == TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) {
		if (wdev->wdev_type == WDEV_TYPE_STA) {
		} else {
			struct frame_btwt_setup *btwt_frame_in = (struct frame_btwt_setup *)&elem->Msg;
			UCHAR *para_buf_next_in = (UCHAR *)&btwt_frame_in->btwt_para[0];
			struct btwt_para_set *btwt_para_in = (struct btwt_para_set *)para_buf_next_in;
			struct btwt_para_set_fix *btwt_para_fix_in = &btwt_para_in->btwt_para_fix;
			UINT8 btwt_element_num = 0;
			BOOLEAN last_element = TRUE;
			UINT8 btwt_frame_out_len = 0;
			PUCHAR out_buffer = NULL;
			struct frame_btwt_setup *btwt_frame_out = NULL;
			UCHAR *para_buf_next_out = NULL;
			struct btwt_para_set *btwt_para_out = NULL;
#ifdef DOT11_EHT_BE
			UINT8 rtwt_ext_len = 0;
#endif /* DOT11_EHT_BE */
			struct _MAC_TABLE_ENTRY *entry = NULL;
			struct TWT_TX_CMD_CTRL_T *twt_cmd = NULL;

			if (!TWT_SUPPORT_BTWT(wlan_config_get_he_twt_support(wdev))) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"wdev(type=%d,fun_idx=%d,wdev_idx=%d) not support btwt,return\n",
				wdev->wdev_type, wdev->func_idx, wdev->wdev_idx);
				return;
			}

			if (!VALID_UCAST_ENTRY_WCID(ad, elem->Wcid)) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"Wcid (%d) isn't valid unicast wcid!\n", elem->Wcid);
				return;
			}

			entry = entry_get(ad, elem->Wcid);
			twt_cmd = &entry->twt_ctrl.twt_cmd;

			if (twt_cmd->cmd_num != 0) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"Can't receive this TWT request tempotarily due to previous command is processing!\n");
				return;
			}

			if (MlmeAllocateMemory(ad, &out_buffer) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						" allocate memory failed, please check\n");
				return;
			}
			btwt_frame_out = (struct frame_btwt_setup *)out_buffer;
			para_buf_next_out = (UCHAR *)&btwt_frame_out->btwt_para[0];
			btwt_para_out = (struct btwt_para_set *)para_buf_next_out;

			/* Handle join request and fill action frame content */
			do {
				btwt_element_num++;
				btwt_para_fix_in = &btwt_para_in->btwt_para_fix;
				/*
					A broadcast TWT parameter set that has the Broadcast TWT Recommendation field value equal to 4
					is referred to as a restricted TWT parameter set.
				*/
#ifdef DOT11_EHT_BE
				if (wlan_config_get_eht_restricted_twt(wdev) &&
					(GET_TWT_RT_REQUEST(btwt_para_fix_in->req_type) == TWT_REQ_TYPE_TWT_REQUEST) &&
					(GET_TWT_RT_SETUP_CMD(btwt_para_fix_in->req_type) == TWT_SETUP_CMD_DEMAND) &&
					(GET_TWT_RT_BTWT_REC(btwt_para_fix_in->req_type) == BTWT_REQ_TYPE_RECOMMENDATION_FIELD_VALUE_RTWT))
					rtwt_handle_peer_join_btwt_id(wdev, elem->Wcid, btwt_frame_in->control,
												(btwt_element_num - 1), btwt_para_in, btwt_para_out);
				else
#endif /* DOT11_EHT_BE */
					twt_handle_peer_join_btwt_id(wdev, elem->Wcid, (btwt_element_num - 1),
												btwt_para_in, btwt_para_out);
				last_element = GET_TWT_RT_IMPLICIT_LAST(btwt_para_fix_in->req_type) ? TRUE : FALSE;
				if (GET_BTWT_INFO_RTWT_TRAFFIC_INFO_PRESENT(btwt_para_fix_in->btwt_info)) {
					para_buf_next_in += sizeof(*btwt_para_in);
					para_buf_next_out += sizeof(*btwt_para_out);
#ifdef DOT11_EHT_BE
					rtwt_ext_len += sizeof(struct rtwt_para_ext_set);
#endif /* DOT11_EHT_BE */
				} else {
					para_buf_next_in = &btwt_para_in->next_buf[0];
					para_buf_next_out = &btwt_para_out->next_buf[0];
				}
				btwt_para_in = (struct btwt_para_set *)para_buf_next_in;
				btwt_para_out = (struct btwt_para_set *)para_buf_next_out;
			} while (!last_element);

			ActHeaderInit(ad, &btwt_frame_out->hdr, btwt_frame_in->hdr.Addr2, wdev->if_addr, wdev->bssid);
			btwt_frame_out->category = CATEGORY_UNPROT_S1G;
			btwt_frame_out->s1g_action = CATE_S1G_ACTION_TWT_SETUP;
			btwt_frame_out->token = btwt_frame_in->token;
			btwt_frame_out->elem_id = IE_TWT;
			btwt_frame_out->control = btwt_frame_in->control;
			/* 1=control, control is counted in frame_btwt_setup */
			btwt_frame_out->len = 1 + sizeof(struct btwt_para_set_fix) * btwt_element_num;
			btwt_frame_out_len = (sizeof(struct frame_btwt_setup) + sizeof(struct btwt_para_set_fix) * btwt_element_num);
#ifdef DOT11_EHT_BE
			btwt_frame_out->len += rtwt_ext_len;
			btwt_frame_out_len += rtwt_ext_len;
#endif /* DOT11_EHT_BE */
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"btwt_frame_out_len = %d, btwt_element_num = %d\n",
				btwt_frame_out_len, btwt_element_num);
			twt_dump_btwt_setup_frame(btwt_frame_in, btwt_frame_out);
			twt_tx_action_frame(wdev, elem->Wcid, out_buffer, btwt_frame_out_len, NULL, FALSE);
			MlmeFreeMemory(out_buffer);
		}
	}
}

static VOID peer_twt_teardown_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct wifi_dev *wdev = elem->wdev;
	UINT16 wcid = elem->Wcid;
	UINT8 nego_type = TWT_CTRL_NEGO_TYPE_ITWT;
	struct frame_twt_teardown *frame = (struct frame_twt_teardown *)&elem->Msg;

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	nego_type = GET_TEARDWON_FRAME_NEGO_TYPE(frame->twt_flow);

	if (nego_type == TWT_CTRL_NEGO_TYPE_ITWT) {
		struct frame_itwt_teardown *frame_in = NULL;

		frame_in = (struct frame_itwt_teardown *)&elem->Msg;

		/* handle twt_entry, twt_link_entry, twt cmd-event */
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
		    "wcid=%d,flow_id=%d\n", wcid, frame_in->twt_flow_id);

#ifdef APCLI_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_STA)
			twtReqFsmRunEventRxTeardown(
				ad, wdev, frame_in->twt_flow_id);
		else
#endif /* APCLI_SUPPORT */
			twt_teardown_request(wdev, wcid, frame_in->twt_flow_id, ITWT_TEARDOWN_BY_STA);
	} else if (nego_type == TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) {
		struct frame_btwt_teardown *btwt_tardown_frame = (struct frame_btwt_teardown *)&elem->Msg;

		twt_handle_peer_leave_btwt_id(wdev,
			elem->Wcid,
			GET_BTWT_FLOW_BTWT_ID(btwt_tardown_frame->twt_flow));
	}
}

/* handle peer twt information frame request */
BOOLEAN twt_get_twt_info_frame_support(
	struct wifi_dev *wdev)
{
	return wlan_config_get_he_twt_info_frame(wdev) ? TRUE : FALSE;
}

VOID peer_twt_info_frame_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct wifi_dev *wdev = elem->wdev;
	UINT16 wcid = elem->Wcid;
	BOOLEAN found = FALSE;
	UINT8 found_num = 0;
	UINT8 sch_link_idx = 0;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	struct frame_twt_information *frame_in = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct TWT_AGRT_PARA_T agrt_para = {0};
	UINT8 agrt_ctrl_flag = 0;
	UINT8 twt_flow_id = 0;
	UINT8 next_twt_subfield_size = 0;
	UINT8 all_twt = 0;
	UINT32 tsf[2] = {0};
	UINT64 tsf_64 = 0;

	if (!wdev) {
		MTWF_DBG(ad,DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	if (!twt_get_twt_info_frame_support(wdev)) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"Not support\n");
		return;
	}

	twt_ctrl = &ad->twt_ctrl;
	frame_in = (struct frame_twt_information *)&elem->Msg;

	/* get twt inforamtion parameters */
	twt_flow_id = GET_TWT_INFO_FLOW_ID(frame_in->twt_info);
	next_twt_subfield_size = GET_TWT_INFO_NEXT_TWT_SUBFIELD_SIZE(frame_in->twt_info);
	if (next_twt_subfield_size)
		os_move_mem(&tsf, &frame_in->next_twt, sizeof(tsf));
	all_twt = GET_TWT_INFO_ALL_TWT(frame_in->twt_info);

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
		"wcid(%d),fid(%d),subfield_size(%d),all_twt(%d),next_twt(msb:0x%.8x,lsb:0x%.8x)\n",
		wcid, twt_flow_id, next_twt_subfield_size, all_twt, frame_in->next_twt[1], frame_in->next_twt[0]);

	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	/* find the twt agrt with flow id */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL,please check\n");
				return;
			}

			found = FALSE;
			if (all_twt) {
				if (curr_twt_node->peer_id_grp_id == wcid) {
					found_num++;
					found = TRUE;
				}
			} else {
				if ((curr_twt_node->peer_id_grp_id == wcid) &&
					(curr_twt_node->flow_id == twt_flow_id)) {
					found_num++;
					found = TRUE;
				}
			}
			/* handle suspend or suspend_resume */
			if (found) {
				curr_twt_node->suspend = TRUE;
				CLR_AGRT_PARA_BITMAP(curr_twt_node);
				if (all_twt)
					SET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_ALL_TWT);

				if (next_twt_subfield_size) {
					/* suspend_resume */
					tsf_64 = tsf[1];
					tsf_64 = (tsf_64 << 32) + tsf[0];

					if (next_twt_subfield_size == 1) {
						tsf_64 = tsf_64 & 0x00000000ffffffff;
						SET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_NEXT_TWT_32_BITS);
					} else if (next_twt_subfield_size == 2) {
						tsf_64 = tsf_64 & 0x0000ffffffffffff;
						SET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_NEXT_TWT_48_BITS);
					} else if (next_twt_subfield_size == 3) {
						SET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_NEXT_TWT_64_BITS);
					}

					curr_twt_node->agrt_sp_info_tsf = tsf_64;
					agrt_ctrl_flag = TWT_AGRT_CTRL_SUSPEND_RESUME;
				} else {
					/* suspend */
					curr_twt_node->agrt_sp_info_tsf = 0;
					agrt_ctrl_flag = TWT_AGRT_CTRL_SUSPEND;
				}

				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					"found wcid=%d,fid=%d agrt with all_twt(%d)\n",
					wcid, twt_flow_id, all_twt);

				twt_agrt_cmd_set(&agrt_para, curr_twt_node, agrt_ctrl_flag, CMD_TSF_TYPE_TWT_INFO);
				mt_asic_twt_agrt_update_by_cmd_task(wdev, &agrt_para);
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

	if (found_num == 0) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"fail to find wcid=%d,fid=%d agrt with all_twt(%d)\n",
			 wcid, twt_flow_id, all_twt);
	}
}

VOID twt_get_resume_event(
	IN struct wifi_dev *wdev,
	IN struct twt_resume_info *resume_info)
{
	struct _RTMP_ADAPTER *ad = NULL;

	if (!wdev)
		return;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);

	if (!ad)
		return;
	if (!resume_info)
		return;
	if (!VALID_UCAST_ENTRY_WCID(ad, resume_info->wcid))
		return;

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"bss=%d,wcid=%d,flow_id=%d,idle=%d\n",
		resume_info->bssinfo_idx,
		resume_info->wcid,
		resume_info->flow_id,
		resume_info->idle);

	MlmeEnqueueWithWdev(ad,
		ACTION_STATE_MACHINE,
		MT2_MLME_TWT_RESUME_INFO,
		sizeof(struct twt_resume_info),
		(PVOID)resume_info,
		0,
		wdev, FALSE, NULL);
	RTMP_MLME_HANDLER(ad);
}

VOID mlme_twt_resume_info_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct wifi_dev *wdev = elem->wdev;
	UINT8 sch_link_idx = 0;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct twt_resume_info *resume_info = NULL;

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	if (!twt_get_twt_info_frame_support(wdev)) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"Not support\n");
		return;
	}

	twt_ctrl = &ad->twt_ctrl;
	resume_info = (struct twt_resume_info *)&elem->Msg;

	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	/* update suspend status */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL,please check\n");
				return;
			}

			if ((curr_twt_node->type == TWT_TYPE_INDIVIDUAL) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->bss_idx == resume_info->bssinfo_idx) &&
				(curr_twt_node->peer_id_grp_id == resume_info->wcid) &&
				(curr_twt_node->flow_id == resume_info->flow_id)) {

				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					"bss=%d,wcid=%d,flow_id=%d,suspend=%d->0\n",
					resume_info->bssinfo_idx,
					resume_info->wcid,
					resume_info->flow_id,
					curr_twt_node->suspend);

				curr_twt_node->suspend = FALSE;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
}


VOID peer_twt_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	PFRAME_802_11 frame = (PFRAME_802_11)&elem->Msg;
	/* Bypass HTC len 4 bytes */
	UINT8 htc_len = (frame->Hdr.FC.Order) ? 4 : 0;
	UINT8 action = 0;
	struct _MAC_TABLE_ENTRY *entry = NULL;
#ifdef DOT11_EHT_BE
	struct MLO_TWT_PRIV_T *mlo_twt_priv = (struct MLO_TWT_PRIV_T *)elem->Others;
#endif /* DOT11_EHT_BE */

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"wcid=%d\n", elem->Wcid);

	/* get entry in master link */
	if (!VALID_UCAST_ENTRY_WCID(ad, elem->Wcid))
		return;

	/* Legacy: ad=action link, wcid=action link, pEntry=action link */
	/* MLO: ad=master link, wcid=master link, pEntry=master link */
	entry = entry_get(ad, elem->Wcid);
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"pEnrty=%p\n", entry);

	if (!entry || !entry->wdev)
		return;

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"A1=%pM,A2=%pM,A3=%pM\n", frame->Hdr.Addr1, frame->Hdr.Addr2, frame->Hdr.Addr3);
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"MLME is for %s BSSID=%pM, len=%d\n",
		entry->wdev->if_dev->name, entry->wdev->bssid, (UINT16)elem->MsgLen);

#ifdef DOT11_EHT_BE
	/* forward action frame from master link to action link */
	if (entry->mlo.mlo_en && !mlo_twt_priv) {
		bss_mngr_mld_twt_action_frm_forward(ad, elem);
		return;
	}

	if (!MAC_ADDR_EQUAL(entry->wdev->bssid, frame->Hdr.Addr3)) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"fail: entry bssid=%pM, frame A1=%pM\n", entry->wdev->bssid, frame->Hdr.Addr1);
		return;
	}
#endif /* DOT11_EHT_BE */

	if (IS_HE_STA(entry->cap.modes) ? FALSE : TRUE) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"fail: Non-HE STA, MaxCap=%s\n",
			get_phymode_str(entry->MaxHTPhyMode.field.MODE));
		return;
	}

	if (htc_len) {
		if ((elem->MsgLen > htc_len) && (elem->MsgLen <= (MAX_MGMT_PKT_LEN - LENGTH_802_11)))
			NdisMoveMemory((void *)(elem->Msg+LENGTH_802_11),
				(void *)(elem->Msg+LENGTH_802_11+htc_len),
				(elem->MsgLen-htc_len));
		else
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"Not have enough buffer size for elem->Msg!\n");
	}

	/* Get S1G Action */
	action = elem->Msg[LENGTH_802_11 + 1];
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"get twt action=%d\n", action);

	switch (action) {
	case CATE_S1G_ACTION_TWT_SETUP:
		peer_twt_setup_action(ad, elem);
		break;

	case CATE_S1G_ACTION_TWT_TEARDOWN:
		peer_twt_teardown_action(ad, elem);
		break;

	case CATE_S1G_ACTION_TWT_INFO:
		peer_twt_info_frame_action(ad, elem);
		break;
	default:
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"unexpected action=%d, please check\n", action);
		break;
	}
}

/* TWT action frame trigger (for AP role) */
VOID twt_teardown_itwt(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN UINT8 twt_flow_id)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct mlme_twt_tear_down_req_struct msg = {0};
	struct _MAC_TABLE_ENTRY *entry = NULL;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	entry = entry_get(ad, wcid);

	/* assign message */
	msg.wdev = wdev;
	msg.nego_type = TWT_CTRL_NEGO_TYPE_ITWT;
	msg.wcid = wcid;
	os_move_mem(&msg.peer_addr[0], entry->Addr, MAC_ADDR_LEN);
	msg.twt_flow_id = twt_flow_id;

	/* enqueue message */
	MlmeEnqueueWithWdev(ad,
		ACTION_STATE_MACHINE,
		MT2_MLME_TWT_TEARDOWN_TWT,
		sizeof(struct mlme_twt_tear_down_req_struct),
		(PVOID)&msg,
		0,
		wdev, FALSE, NULL);
	RTMP_MLME_HANDLER(ad);

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"wcid(%d),flow_id(%d)\n",
		wcid,
		twt_flow_id);
}

static VOID twt_remove_btwt_ie(
	struct wifi_dev *wdev,
	UINT8 btwt_id)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	UINT8 sch_link_idx = 0;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	UINT8 band = 0;
	UINT8 found = FALSE;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"ad=NULL, please check\n");
		return;
	}

	twt_ctrl = &ad->twt_ctrl;
	band = HcGetBandByWdev(wdev);

	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				return;
			}

			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band) &&
				(GET_BTWT_ID(curr_twt_node) == btwt_id)) {
				curr_twt_node->present = FALSE;
				found = TRUE;
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

	if (found)
		UpdateBeaconHandler(ad, wdev, BCN_REASON(BCN_UPDATE_BTWT_IE));
}

VOID mlme_twt_teradown_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct mlme_twt_tear_down_req_struct *msg = NULL;
	struct frame_twt_teardown frame_out;
	struct wifi_dev *wdev = NULL;
	PUCHAR out_buffer = NULL;
	ULONG frame_len = 0;
	UINT8 nego_type = 0;
	UINT8 i = 0, j = 0;
	UINT16 wcid = 0;
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	BOOLEAN found = FALSE;
	struct _MAC_TABLE_ENTRY *entry = NULL;
#ifdef DOT11_EHT_BE
	struct MLO_TWT_PRIV_T _priv = {0};
	struct MLO_TWT_PRIV_T *priv = NULL;
	struct mld_entry_t *mld_entry = NULL;
#endif /* DOT11_EHT_BE */

	/* get an unused nonpaged memory */
	status = os_alloc_mem(ad, &out_buffer, MAX_MGMT_PKT_LEN);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"allocate memory failed, please check\n");
		return;
	}

	msg = (struct mlme_twt_tear_down_req_struct *)&elem->Msg;
	wdev = msg->wdev;
	wcid = msg->wcid;
	nego_type = msg->nego_type;

	if (nego_type == TWT_CTRL_NEGO_TYPE_ITWT) {
		struct frame_itwt_teardown *itwt_frame = (struct frame_itwt_teardown *)&frame_out;

#ifdef DOT11_EHT_BE
		/* extra info needed by pEnrty which is mlo_en=1 */
		entry = entry_get(ad, wcid);
		mt_rcu_read_lock();
		mld_entry = rcu_dereference(entry->mld_entry);

		if (entry->mlo.mlo_en && mld_entry) {
			_priv.mld_sta_idx = mld_entry->mld_sta_idx;
			_priv.action_linkid = entry->mlo.link_info.link_id;
			_priv.all_linkid_bitmap = 1 << entry->mlo.link_info.link_id;
			_priv.state = STATE_TX_AGGREGATE_ACTION_FRAME;
			priv = &_priv;
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
				"mlo pEntry: mld_sta_idx=%d,action_linkid=%d,all_linkid_bitmap=0x%x\n",
				_priv.mld_sta_idx,
				_priv.action_linkid,
				_priv.all_linkid_bitmap);
		}
		mt_rcu_read_unlock();
#endif /* DOT11_EHT_BE */

		/* find the twt session by wdev, wcid, twt_flow_id */
		found = twt_teardown_request(wdev, wcid, msg->twt_flow_id, ITWT_TEARDOWN_BY_AP);

		/* send action frame to peer sta to teardown twt session */
		if (found) {
			os_zero_mem(itwt_frame, sizeof(struct frame_itwt_teardown));
			ActHeaderInit(ad, &itwt_frame->hdr, msg->peer_addr, wdev->if_addr, wdev->bssid);
			itwt_frame->category = CATEGORY_UNPROT_S1G;
			itwt_frame->s1g_action = CATE_S1G_ACTION_TWT_TEARDOWN;
			itwt_frame->twt_flow_id = msg->twt_flow_id;

			MakeOutgoingFrame(out_buffer, &frame_len,
				sizeof(struct frame_itwt_teardown), itwt_frame,
				END_OF_ARGS);
#ifdef DOT11_EHT_BE
			if (entry->mlo.mlo_en && priv)
				twt_tx_action_frame(wdev, wcid, out_buffer, frame_len, priv, TRUE);
			else
#endif /* DOT11_EHT_BE */
				twt_tx_action_frame(wdev, wcid, out_buffer, frame_len, NULL, TRUE);
		}

		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"wdev(%s): wcid(%d), flow_id(%d) %s\n",
			wdev->if_dev->name,
			wcid,
			msg->twt_flow_id,
			found ? "is handled" : "not found!");
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"A1=%pM,A2=%pM,A3=%pM\n",
			itwt_frame->hdr.Addr1,
			itwt_frame->hdr.Addr2,
			itwt_frame->hdr.Addr3);
	} else if (nego_type == TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) {
		struct frame_btwt_teardown *btwt_frame = (struct frame_btwt_teardown *)&frame_out;
		UINT16 sta_list[TWT_HW_BTWT_MAX_MEMBER_CNT] = {0};
		UINT8 sta_cnt = 0;
		UINT8 btwt_id = msg->btwt_id;
		BOOLEAN teardown_all_twt = msg->teardown_all_twt;
		struct TWT_AGRT_PARA_T agrt_para = {0};
		struct wifi_dev_ops *ops;
		BOOLEAN action_frame_send = FALSE;

		if (!MAC_ADDR_EQUAL(msg->peer_addr, ZERO_MAC_ADDR)) {
			ops = wdev->wdev_ops;
			ops->mac_entry_lookup(ad, msg->peer_addr, wdev, &entry);

			if (!entry) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"Fail to find pEntry with %pM, return\n", msg->peer_addr);
				os_free_mem(out_buffer);
				return;
			}

			/* teardown with action frame only btwt_id > 0 */
			for (i = 1; i < TWT_BTWT_ID_NUM; i++) {
				if (!teardown_all_twt && btwt_id != i)
					continue;

				/* send action frame to peer sta */
				if (!VALID_UCAST_ENTRY_WCID(ad, entry->wcid))
					continue;

				if (!GET_PEER_JOIN_BTWT_ID(entry, i))
					continue;

				SET_PEER_LEAVE_BTWT_ID(entry, i);

#ifdef DOT11_EHT_BE
				os_zero_mem(&entry->twt_ctrl.rtwt_info[i], sizeof(struct RTWT_INFO_T));
#endif /* DOT11_EHT_BE */

				if (!action_frame_send) {
					os_zero_mem(btwt_frame, sizeof(struct frame_btwt_teardown));
					ActHeaderInit(ad, &btwt_frame->hdr, entry->Addr, wdev->if_addr, wdev->bssid);
					btwt_frame->category = CATEGORY_UNPROT_S1G;
					btwt_frame->s1g_action = CATE_S1G_ACTION_TWT_TEARDOWN;
					btwt_frame->twt_flow |= SET_BTWT_FLOW_BTWT_ID(teardown_all_twt ? 0 : i) |
						SET_BTWT_FLOW_NEGO_TYPE(TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) |
						SET_BTWT_FLOW_TEARDOWN_ALL_TWT(teardown_all_twt);

					MakeOutgoingFrame(out_buffer, &frame_len,
						sizeof(struct frame_btwt_teardown), btwt_frame,
						END_OF_ARGS);

					MiniportMMRequest(ad,
						(MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[QID_AC_VO]),
						out_buffer, frame_len, NULL);

					twt_dump_btwt_mlme_teardown_frame((struct frame_btwt_teardown *)out_buffer);

					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"OK to remove btwt_id=%d,wcid=%d with teardown_all_twt=%d\n",
						i, wcid, teardown_all_twt);
					action_frame_send = TRUE;
				}

				/* twt cmd-event(modify) */
				if (twt_handle_one_peer_with_btwt_id_leave(wdev, entry, i, &agrt_para))
					mt_asic_twt_agrt_update(wdev, &agrt_para);
			}
		} else {
			/* teardown with action frame only btwt_id > 0 */
			for (i = 1; i < TWT_BTWT_ID_NUM; i++) {
				/* AP teardown specific btwt_id (not btwt_id_0) */
				if (!teardown_all_twt && btwt_id != i)
					continue;

				os_zero_mem(sta_list, sizeof(sta_list));
				os_zero_mem(&agrt_para, sizeof(struct TWT_AGRT_PARA_T));
				sta_cnt = 0;

				/* handle twt_node, twt_link_lsit */
				twt_handle_all_peers_in_btwt_id_leave(wdev, i, sta_list, &sta_cnt, &agrt_para);

				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					"btwt_id=%d,teardown_all_twt=%d,sta_cnt=%d,sta_list[%d,%d,%d,%d,%d,%d,%d,%d],sts=%s\n",
					i, teardown_all_twt, sta_cnt,
					sta_list[0], sta_list[1], sta_list[2], sta_list[3], sta_list[4], sta_list[5], sta_list[6], sta_list[7],
					sta_cnt ? "DONE" : "NA");

				if (sta_cnt > 0) {
					/* send action frame to peer sta */
					for (j = 0; j < sta_cnt; j++) {
						wcid = sta_list[j];

						if (!VALID_UCAST_ENTRY_WCID(ad, wcid))
							continue;

						entry = entry_get(ad, wcid);
						if (!GET_PEER_JOIN_BTWT_ID(entry, i))
							continue;

						SET_PEER_LEAVE_BTWT_ID(entry, i);

						if (teardown_all_twt) {
							SET_PEER_LEAVE_ALL_BTWT(entry);
#ifdef DOT11_EHT_BE
							os_zero_mem(entry->twt_ctrl.rtwt_info,
										(sizeof(struct RTWT_INFO_T) * TWT_BTWT_ID_NUM));
#endif /* DOT11_EHT_BE */
						}
#ifdef DOT11_EHT_BE
						else
							os_zero_mem(&entry->twt_ctrl.rtwt_info[i], sizeof(struct RTWT_INFO_T));
#endif /* DOT11_EHT_BE */

						os_zero_mem(btwt_frame, sizeof(struct frame_btwt_teardown));
						ActHeaderInit(ad, &btwt_frame->hdr, entry->Addr, wdev->if_addr, wdev->bssid);
						btwt_frame->category = CATEGORY_UNPROT_S1G;
						btwt_frame->s1g_action = CATE_S1G_ACTION_TWT_TEARDOWN;
						btwt_frame->twt_flow |= SET_BTWT_FLOW_BTWT_ID(i) |
							SET_BTWT_FLOW_NEGO_TYPE(TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) |
							SET_BTWT_FLOW_TEARDOWN_ALL_TWT(teardown_all_twt);

						MakeOutgoingFrame(out_buffer, &frame_len,
							sizeof(struct frame_btwt_teardown), btwt_frame,
							END_OF_ARGS);

						MiniportMMRequest(ad,
							(MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[QID_AC_VO]),
							out_buffer, frame_len, NULL);

						twt_dump_btwt_mlme_teardown_frame((struct frame_btwt_teardown *)out_buffer);

						MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
							"(ID):send teardown action frame btwt_id=%d,wcid=%d,teardown_all_twt=%d\n",
							i, wcid, teardown_all_twt);
					}
					/* twt cmd-event(modify) */
					mt_asic_twt_agrt_update(wdev, &agrt_para);
				}
			}
		}
	}

	os_free_mem(out_buffer);
}

/* AP wants to exclude btwt_id ie in BCN/ProbeRsp */
VOID twt_remove_btwt_resouce(
	struct wifi_dev *wdev,
	UINT8 btwt_id)
{
	struct frame_twt_teardown frame_out;
	struct frame_btwt_teardown *btwt_frame = (struct frame_btwt_teardown *)&frame_out;
	PUCHAR out_buffer = NULL;
	ULONG frame_len = 0;
	UINT8 i = btwt_id, j = 0;
	UINT16 wcid = 0;
	UINT16 sta_list[TWT_HW_BTWT_MAX_MEMBER_CNT] = {0};
	UINT8 sta_cnt = 0;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct TWT_AGRT_PARA_T agrt_para = {0};
	NDIS_STATUS status = NDIS_STATUS_FAILURE;

	/* get an unused nonpaged memory */
	status = os_alloc_mem(ad, &out_buffer, MAX_MGMT_PKT_LEN);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
		"allocate memory failed, please check\n");
		return;
	}

	os_zero_mem(sta_list, sizeof(sta_list));
	os_zero_mem(&agrt_para, sizeof(struct TWT_AGRT_PARA_T));
	sta_cnt = 0;

	/* handle twt_node, twt_link_lsit */
	twt_handle_all_peers_in_btwt_id_leave(wdev, i, sta_list, &sta_cnt, &agrt_para);

	/* remove btwt ie from BCN/ProbeRsp */
	twt_remove_btwt_ie(wdev, i);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"btwt_id=%d,sta_cnt=%d,sta_list[%d,%d,%d,%d,%d,%d,%d,%d],sts=%s\n",
		i, sta_cnt,
		sta_list[0], sta_list[1], sta_list[2], sta_list[3], sta_list[4], sta_list[5], sta_list[6], sta_list[7],
		sta_cnt ? "DONE" : "NA");

	/* only btwt_id > 0 apply teadrdown action frame */
	if ((sta_cnt > 0) && (i > 0)) {
		/* send action frame to peer sta */
		for (j = 0; j < sta_cnt; j++) {
			wcid = sta_list[j];

			if (!VALID_UCAST_ENTRY_WCID(ad, wcid))
				continue;

			entry = entry_get(ad, wcid);
			if (!GET_PEER_JOIN_BTWT_ID(entry, i))
				continue;

			SET_PEER_LEAVE_BTWT_ID(entry, btwt_id);

#ifdef DOT11_EHT_BE
			os_zero_mem(&entry->twt_ctrl.rtwt_info[btwt_id], sizeof(struct RTWT_INFO_T));
#endif /* DOT11_EHT_BE */

			os_zero_mem(btwt_frame, sizeof(struct frame_btwt_teardown));
			ActHeaderInit(ad, &btwt_frame->hdr, entry->Addr, wdev->if_addr, wdev->bssid);
			btwt_frame->category = CATEGORY_UNPROT_S1G;
			btwt_frame->s1g_action = CATE_S1G_ACTION_TWT_TEARDOWN;
			btwt_frame->twt_flow |= SET_BTWT_FLOW_BTWT_ID(i) |
				SET_BTWT_FLOW_NEGO_TYPE(TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) |
				SET_BTWT_FLOW_TEARDOWN_ALL_TWT(0);

			MakeOutgoingFrame(out_buffer, &frame_len,
				sizeof(struct frame_btwt_teardown), btwt_frame,
				END_OF_ARGS);

			MiniportMMRequest(ad,
				(MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[QID_AC_VO]),
				out_buffer, frame_len, NULL);

			twt_dump_btwt_mlme_teardown_frame((struct frame_btwt_teardown *)out_buffer);

			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"OK to remove wcid=%d from btwt_id=%d\n",
				wcid, i);
		}
	}
	/* twt cmd-event(modify) */
	if (sta_cnt > 0)
		mt_asic_twt_agrt_update(wdev, &agrt_para);

	/* remove btwt node in linklist and twt cmd-event(del) */
	twt_release_btwt_node(wdev, i);

	os_free_mem(out_buffer);
}

/* Peer STA link down twt management */
VOID twt_resource_release_at_link_down(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid)
{
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct _RTMP_ADAPTER *ad = NULL;
	UINT8 i = 0;
	BOOLEAN cancelled;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	entry = entry_get(ad, wcid);

	RTMPCancelTimer(&entry->twt_ctrl.act_wait_ack_timer, &cancelled);
	RTMPReleaseTimer(&entry->twt_ctrl.act_wait_ack_timer, &cancelled);
	/* handle twt_node, twt_link_list, twt cmd-event */
	if (GET_PEER_ITWT_FID_BITMAP(entry) != 0) {
		for (i = 0; i < TWT_FLOW_ID_MAX_NUM; i++) {
			if (GET_PEER_ITWT_FID_BITMAP(entry) & (1 << i)) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					" wcid(%d), flow_id(%d)\n", wcid, i);
				twt_teardown_request(wdev, wcid, i, ITWT_TEARDOWN_BY_LINKDOWN);
			}
		}
	}

	if (GET_PEER_BTWT_ID_BITMAP(entry) != 0) {
		for (i = 0; i < TWT_BTWT_ID_NUM; i++) {
			if (GET_PEER_BTWT_ID_BITMAP(entry) & (1 << i)) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					" wcid(%d) in btwt_id(%d)\n",
					wcid, i);
				twt_handle_peer_leave_btwt_id(wdev, wcid, i);
			}
		}
	}
}

VOID twt_dump_resource(
	IN struct wifi_dev *wdev)
{
	INT ret = 0;

	ret = twt_ctrl_itwt_dump(wdev);
	if (ret)
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s: error=%d\n", __func__, ret);
}

VOID twt_get_current_tsf(
	struct wifi_dev *wdev,
	PUINT32 current_tsf)
{
	if (HW_GET_TSF(wdev, current_tsf) != NDIS_STATUS_SUCCESS)
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"fail\n");
}

/* bTWT*/
VOID twt_handle_peer_join_btwt_id_0(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN UINT16 wcid
)
{
	struct TWT_CTRL_T *twt_ctrl = NULL;
	UINT8 band = 0;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	UINT8 sch_link_idx = 0;
	struct TWT_AGRT_PARA_T agrt_para = {0};

	if (!ad)
		return;

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	if (!VALID_UCAST_ENTRY_WCID(ad, wcid))
		return;

	twt_ctrl = &ad->twt_ctrl;
	band = HcGetBandByWdev(wdev);
	entry = entry_get(ad, wcid);

	/* i have btwt_id=0 and let peer STA with btwt cap. join btwt id=0 */
	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					" twt_entry=NULL, please check\n");
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				return;
			}
			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band)) {
				if (GET_BTWT_ID(curr_twt_node) == 0) {
					if (twt_add_btwt_member(curr_twt_node, wcid)) {
						SET_PEER_JOIN_BTWT_ID(entry, 0);
						twt_agrt_cmd_set(&agrt_para,
							curr_twt_node,
							TWT_AGRT_CTRL_MODIFY,
							CMD_TSF_TYPE_BTWT);

						//mt_asic_twt_agrt_update(wdev, &agrt_para);
						mt_asic_twt_agrt_update_by_cmd_task(wdev, &agrt_para);

						MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
							"OK to add wcid=%d to btwt_id=%d\n",
							wcid, (UINT16)GET_BTWT_ID(curr_twt_node));
					}

					break;
				}
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
}

VOID mlme_twt_handle_btwt_join_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct mlme_twt_join_btwt_req_struct *msg = NULL;
	struct wifi_dev *wdev = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	UINT8 sch_link_idx = 0;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct TWT_AGRT_PARA_T agrt_para = {0};
	UINT8 band = 0;

	msg = (struct mlme_twt_join_btwt_req_struct *)&elem->Msg;
	wdev = msg->wdev;

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	if (!ad)
		return;

	if (!VALID_UCAST_ENTRY_WCID(ad, msg->wcid))
		return;

	twt_ctrl = &ad->twt_ctrl;
	band = HcGetBandByWdev(wdev);
	entry = entry_get(ad, msg->wcid);

	/* i have btwt_id=0 and let peer STA with btwt cap. join btwt id=0 */
	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					" twt_entry=NULL, please check\n");
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				return;
			}
			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band)) {
				if (GET_BTWT_ID(curr_twt_node) == msg->btwt_id) {
					if (twt_add_btwt_member(curr_twt_node, msg->wcid)) {
						SET_PEER_JOIN_BTWT_ID(entry, msg->btwt_id);
						twt_agrt_cmd_set(&agrt_para,
							curr_twt_node,
							TWT_AGRT_CTRL_MODIFY,
							CMD_TSF_TYPE_BTWT);

						mt_asic_twt_agrt_update_by_cmd_task(wdev, &agrt_para);

						MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
							"OK to add wcid=%d to btwt_id=%d\n",
							msg->wcid, (UINT16)GET_BTWT_ID(curr_twt_node));
					}

					break;
				}
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
}

VOID twt_peer_join_btwt_id_0(
	struct wifi_dev *wdev,
	VOID *enrty)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *ad = NULL;
	struct BTWT_BUF_STRUCT *btwt = NULL;
	struct _MAC_TABLE_ENTRY *_entry = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return;
	}

	_entry = (struct _MAC_TABLE_ENTRY *)enrty;
	if (!_entry)
		return;

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (!ad)
		return;

	if (!VALID_UCAST_ENTRY_WCID(ad, _entry->wcid))
		return;

	if (wlan_config_get_asic_twt_caps(wdev) &&
		TWT_SUPPORT_BTWT(wlan_config_get_he_twt_support(wdev)) &&
		IS_STA_SUPPORT_BTWT(_entry)) {
		twt_ctrl = &ad->twt_ctrl;
		btwt = &ad->ApCfg.btwt;
		NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
		if (btwt->support_btwt_id_0) {
			NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
			twt_handle_peer_join_btwt_id_0(ad, wdev, _entry->wcid);
			return;
		}
		NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
	}
#endif /* CONFIG_AP_SUPPORT */
}

VOID twt_tardown_btwt(
	struct wifi_dev *wdev,
	UCHAR *peer_addr,
	UINT8 nego_type,
	UINT8 btwt_id,
	BOOLEAN teardown_all_twt)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct mlme_twt_tear_down_req_struct msg = {0};

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (!ad)
		return;

	if (nego_type == TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) {
		if (wlan_config_get_asic_twt_caps(wdev) &&
			TWT_SUPPORT_BTWT(wlan_config_get_he_twt_support(wdev))) {
			msg.wdev = wdev;
			if (peer_addr != NULL)
				COPY_MAC_ADDR(&msg.peer_addr, peer_addr);
			msg.nego_type = nego_type;
			msg.btwt_id = btwt_id;
			msg.teardown_all_twt = teardown_all_twt;
			MlmeEnqueueWithWdev(ad,
				ACTION_STATE_MACHINE,
				MT2_MLME_TWT_TEARDOWN_TWT,
				sizeof(struct mlme_twt_tear_down_req_struct),
				(PVOID)&msg,
				0,
				wdev, FALSE, NULL);
		}
	}
}

VOID twt_build_btwt_ie(
	struct wifi_dev *wdev,
	UINT32 ie_type)
{
	struct _RTMP_ADAPTER *ad = NULL;
	UINT8 sch_link_idx = 0;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	UINT8 btwt_element_num = 0;
	struct btwt_ie *btwt_element = NULL;
	struct btwt_para_set *btwt_para = NULL;
	struct btwt_para_set_fix *btwt_para_fix = NULL;
	UINT8 band = 0;
	struct BTWT_BUF_STRUCT *btwt = NULL;
	UCHAR *pParaBufNext = NULL, *pParaBufEnd = NULL;
#ifdef DOT11_EHT_BE
	UINT8 rtwt_ext_len = 0;
#endif /* DOT11_EHT_BE */

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"ad=NULL, please check\n");
		return;
	}

	twt_ctrl = &ad->twt_ctrl;

	NdisAcquireSpinLock(&ad->ApCfg.btwt_ie_lock);
	band = HcGetBandByWdev(wdev);
	btwt = &ad->ApCfg.btwt;
	os_zero_mem(btwt, sizeof(struct BTWT_BUF_STRUCT));
	btwt_element = (struct btwt_ie *)&btwt->btwt_element;
	pParaBufNext = (UCHAR *)&btwt_element->btwt_para[btwt_element_num];
	pParaBufEnd = ((UCHAR *)&btwt->btwt_element[0] + sizeof(struct btwt_ie));

	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check!\n");
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				NdisReleaseSpinLock(&ad->ApCfg.btwt_ie_lock);
				return;
			}
			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band) &&
				curr_twt_node->present) {
				if (pParaBufNext >= pParaBufEnd)
					break;
				btwt_para = (struct btwt_para_set *)pParaBufNext;
				if (!btwt_para) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
							"btwt_para=NULL, please check!\n");
					break;
				}
				btwt_para_fix = &btwt_para->btwt_para_fix;

				/* request type */
				btwt_para_fix->req_type &= ~TWT_REQ_TYPE_TWT_REQUEST;
				btwt_para_fix->req_type &= ~TWT_REQ_TYPE_TWT_SETUP_COMMAND;
				btwt_para_fix->req_type |= (SET_TWT_RT_SETUP_CMD(TWT_SETUP_CMD_ACCEPT) |
						SET_TWT_RT_TRIGGER(GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_TRIGGER)) |
						SET_TWT_RT_FLOW_TYPE(GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE) ? 0 : 1) |
						SET_TWT_RT_BTWT_REC(curr_twt_node->btwt_recommendation) |
						SET_TWT_RT_WAKE_INTVAL_EXP(curr_twt_node->agrt_sp_wake_intvl_exponent) |
						SET_TWT_RT_PROTECTION(GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_PROTECT)));
				/* target wake  time: unit=TU */
				btwt_para_fix->target_wake_time = (UINT16)(curr_twt_node->schedule_sp_start_tsf >> 10);
				btwt->schedule_sp_start_tsf[btwt_element_num] = (UINT32)(curr_twt_node->schedule_sp_start_tsf);
				/* nominal minimum twt wake duration */
				btwt_para_fix->duration = curr_twt_node->agrt_sp_duration;
				/* twt wake inetrval mantissa */
				btwt_para_fix->mantissa = curr_twt_node->agrt_sp_wake_intvl_mantissa;
				/* broadcast twt info */
				btwt_para_fix->btwt_info |= (SET_BTWT_INFO_BTWT_ID(GET_BTWT_ID(curr_twt_node)) |
											SET_BTWT_INFO_BTWT_P(curr_twt_node->persistence));

				if (GET_BTWT_ID(curr_twt_node) == 0)
					btwt->support_btwt_id_0 = TRUE;
#ifdef DOT11_EHT_BE
				/* broadcast twt info for rTWT */
				if (wlan_config_get_eht_restricted_twt(wdev) &&
					IS_RTWT_ID(curr_twt_node)) {
					btwt_para_fix->btwt_info |= SET_BTWT_INFO_RTWT_SCHEDULE_INFO(1);
					pParaBufNext = &btwt_para->next_buf[0];
				} else
#endif /* DOT11_EHT_BE */
					pParaBufNext = &btwt_para->next_buf[0];

				btwt_element_num++;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

	if (btwt_element_num != 0) {
		btwt_element->elem_id = IE_TWT;
		btwt_element->len = 1 + (sizeof(struct btwt_para_set_fix) * btwt_element_num); /*1=ctl*/
#ifdef DOT11_EHT_BE
		btwt_element->len += rtwt_ext_len;
#endif /* DOT11_EHT_BE */

		/* control */
		btwt_element->control |= (SET_TWT_CTRL_NEGO_TYPE(TWT_CTRL_NEGO_TYPE_BTWT_ANNOUNCE) |
								SET_TWT_CTRL_INFO_FRM_DIS(curr_twt_node->twt_info_frame_dis) |
								SET_TWT_CTRL_WAKE_DUR_UNIT(curr_twt_node->wake_dur_unit));
		/* assign last element bit */
		if (btwt_para_fix)
			btwt_para_fix->req_type |= SET_TWT_RT_IMPLICIT_LAST(1);
		btwt->btwt_element_exist = TRUE;
		btwt->btwt_element_num = btwt_element_num;
		btwt->btwt_bcn_offset = 0;
		btwt->btwt_probe_rsp_offset = 0;
	}
	NdisReleaseSpinLock(&ad->ApCfg.btwt_ie_lock);

	UpdateBeaconHandler_BPCC(ad, wdev, BCN_REASON(BCN_UPDATE_IE_CHG), ie_type, TRUE);
}

VOID twt_update_btwt_twt(
	struct wifi_dev *wdev,
	UINT32 *current_tsf)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct BTWT_BUF_STRUCT *btwt = NULL;
	struct btwt_ie *btwt_element = NULL;
	UINT8 i = 0;
	struct btwt_para_set *btwt_para = NULL;
	UINT64 twt_current_tsf = 0;
	UINT16 mantissa = 0;
	UINT8 exponent = 0;
	UINT64 twt_interval = 0;
	UINT64 twt_mod = 0;
	UINT64 temp = 0;
	UINT64 future_target_wake_time = 0;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, error\n");
		return;
	}

	if (current_tsf[0] == 0 && current_tsf[1] == 0) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"tsf=0, error\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	btwt = &ad->ApCfg.btwt;
	btwt_element = (struct btwt_ie *)&btwt->btwt_element;
	twt_current_tsf = current_tsf[0] + (((UINT64)current_tsf[1]) << 32);

	for (i = 0; i < btwt->btwt_element_num; i++) {
		if (i == 0)
			btwt_para = &btwt_element->btwt_para[i];
		else
			btwt_para = (struct btwt_para_set *)btwt_para->next_buf;
		mantissa = btwt_para->btwt_para_fix.mantissa;
		exponent = (UINT8)GET_TWT_RT_WAKE_INTVAL_EXP(btwt_para->btwt_para_fix.req_type);
		twt_interval = ((UINT64)mantissa) << exponent;
		if (twt_interval != 0) {
			temp = twt_current_tsf - btwt->schedule_sp_start_tsf[i];
			twt_mod = mod_64bit(temp, twt_interval);
			future_target_wake_time = twt_current_tsf + (twt_interval - twt_mod);
			btwt_para->btwt_para_fix.target_wake_time = (UINT16)((future_target_wake_time >> 10) & 0xffff);
		}
	}
}

VOID twt_update_btwt_action_frame_tsf(
	struct wifi_dev *wdev,
	UINT64 schedule_sp_start_tsf,
	struct btwt_para_set_fix *btwt_para_fix)
{
	UINT32 current_tsf[2] = {0};
	UINT64 twt_current_tsf = 0;
	UINT16 mantissa = 0;
	UINT8 exponent = 0;
	UINT64 twt_interval = 0;
	UINT64 twt_mod = 0;
	UINT64 temp = 0;
	UINT64 future_target_wake_time = 0;

	if ((wdev == NULL) ||
		(btwt_para_fix == NULL)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL or btwt_para_fix=NULL, please check\n");
		return;
	}

	twt_get_current_tsf(wdev, current_tsf);

	if (current_tsf[0] == 0 && current_tsf[1] == 0) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"tsf=0, error\n");
		return;
	}

	twt_current_tsf = current_tsf[0] + (((UINT64)current_tsf[1]) << 32);
	mantissa = btwt_para_fix->mantissa;
	exponent = (UINT8)GET_TWT_RT_WAKE_INTVAL_EXP(btwt_para_fix->req_type);
	twt_interval = ((UINT64)mantissa) << exponent;
	if (twt_interval != 0) {
		temp = twt_current_tsf - schedule_sp_start_tsf;
		twt_mod = mod_64bit(temp, twt_interval);
		future_target_wake_time = twt_current_tsf + (twt_interval - twt_mod);
		btwt_para_fix->target_wake_time = (UINT16)((future_target_wake_time >> 10) & 0xffff);
	}
}

INT twt_acquire_btwt_node(
	struct wifi_dev *wdev,
	struct twt_ctrl_btwt *btwt_ctrl_para)
{
	INT ret = 0;
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	UINT8 sch_link_idx = 0;
	BOOLEAN found = FALSE;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct TWT_AGRT_PARA_T agrt_para = {0};
	struct TWT_AGRT_MGMT_T agrt_mgmt = {0};
	struct twt_link_node *twt_node = NULL;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		ret = -EINVAL;
		goto end;
	}

	if (!TWT_SUPPORT_BTWT(wlan_config_get_he_twt_support(wdev))) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
		"wdev(type=%d,fun_idx=%d,wdev_idx=%d) not support btwt,return\n",
		wdev->wdev_type, wdev->func_idx, wdev->wdev_idx);
		ret = -EINVAL;
		goto end;
	}

	if (wdev->btwt_id & (1 << btwt_ctrl_para->btwt_id)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
		"btwt_id=%d existed, please release it brefore new acquire\n",
		btwt_ctrl_para->btwt_id);
		ret = -EINVAL;
		goto end;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"ad=NULL, please check\n");
		ret = -EINVAL;
		goto end;
	}

	twt_ctrl = &ad->twt_ctrl;

	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	/* reject duplicate band/btwt_id request */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				ret = -EINVAL;
				goto end;
			}

			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == btwt_ctrl_para->band) &&
				(GET_BTWT_ID(curr_twt_node) == btwt_ctrl_para->btwt_id)) {
				found = TRUE;
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

	if (!found) {
		/* allocate twt_node */
		twt_ctrl_acquire_twt_node(wdev, &twt_node, TWT_TYPE_BTWT);
		if (twt_node) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"Success.Done added btwt element\n");
			twt_node->band = btwt_ctrl_para->band;
			SET_BTWT_ID(twt_node, btwt_ctrl_para->btwt_id);
			twt_node->agrt_sp_duration = btwt_ctrl_para->agrt_sp_duration;
			twt_node->bss_idx = wdev->bss_info_argument.ucBssIndex;
			twt_node->agrt_sp_wake_intvl_mantissa = btwt_ctrl_para->agrt_sp_wake_intvl_mantissa;
			twt_node->agrt_sp_wake_intvl_exponent = btwt_ctrl_para->agrt_sp_wake_intvl_exponent;
			twt_node->is_role_ap = TWT_ROLE_AP;
			twt_node->agrt_para_bitmap = btwt_ctrl_para->agrt_para_bitmap;
			twt_node->twt_setup_cmd	= btwt_ctrl_para->twt_setup_cmd;
			twt_node->persistence = btwt_ctrl_para->persistence;
			twt_node->twt_info_frame_dis = btwt_ctrl_para->twt_info_frame_dis;
			twt_node->wake_dur_unit = btwt_ctrl_para->wake_dur_unit;
			twt_node->btwt_recommendation = btwt_ctrl_para->btwt_recommendation;
			twt_node->present = TRUE;

			/* alloc agrt tbl resource */
			ret = twt_ctrl_acquire_twt_agrt(wdev, twt_node, &agrt_mgmt);
			if (ret)
				goto end;

			twt_node->agrt_tbl_idx = agrt_mgmt.res_id;

			/* schedule this twt_node */
			twt_link_insert_node(wdev, twt_node);

			/* twt cmd */
			twt_agrt_cmd_set(&agrt_para, twt_node, TWT_AGRT_CTRL_ADD, CMD_TSF_TYPE_BTWT);

			mt_asic_twt_agrt_update(wdev, &agrt_para);
			wdev->btwt_id |= (1 << btwt_ctrl_para->btwt_id);

			/* build btwt element for bcn/probe_rsp */
			if (bcn_bpcc_op_lock(ad, wdev, TRUE, BCN_BPCC_BTWTSET) == FALSE) {
				MTWF_PRINT("%s: bcn_bpcc_op_lock fail!\n", __func__);
			} else {
				twt_build_btwt_ie(wdev, BCN_BPCC_BTWTSET);
			}
		} else {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"Fail.No free btwt resource available!\n");
		}
	}

	return ret;
end:
	/* free twt_node */
	if (twt_node)
		twt_ctrl_release_twt_node(twt_node);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
		"%s: error=%d\n", __func__, ret);
	return ret;
}

VOID twt_release_btwt_node(
	struct wifi_dev *wdev,
	UINT8 btwt_id)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	UINT8 sch_link_idx = 0;
	UINT8 band = 0;
	UINT8 found = FALSE;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct TWT_AGRT_PARA_T agrt_para = {0};

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return;
	}

	if (!TWT_SUPPORT_BTWT(wlan_config_get_he_twt_support(wdev))) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"wdev(type=%d,fun_idx=%d,wdev_idx=%d) not support btwt,return\n",
		wdev->wdev_type, wdev->func_idx, wdev->wdev_idx);
		return;
	}

	if (wdev->btwt_id == 0) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"wdev not support btwt and return\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"ad=NULL, please check\n");
		return;
	}

	twt_ctrl = &ad->twt_ctrl;
	band = HcGetBandByWdev(wdev);

	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	/* find twt database for band+btwt_id */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				return;
			}

			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band) &&
				(GET_BTWT_ID(curr_twt_node) == btwt_id)) {
				found = TRUE;
				twt_agrt_cmd_set(&agrt_para, curr_twt_node, TWT_AGRT_CTRL_DELETE, CMD_TSF_TYPE_NA);
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

	if (found) {
		wdev->btwt_id &= ~(1 << btwt_id);

		mt_asic_twt_agrt_update(wdev, &agrt_para);

		NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
		if (twt_is_link_node_valid(ad, curr_twt_node, FALSE)) {
			twt_link_remove_node(wdev, curr_twt_node, FALSE);
			twt_ctrl_release_twt_node(curr_twt_node);
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"Success.Found band=%d,btwt_id=%d and release btwt resource\n",
				band, btwt_id);
		}
		NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
	} else {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"Fail.Failed to find band=%d,btwt_id=%d\n",
			band, btwt_id);
	}

	/* build btwt element for bcn/probe_rsp */
	if (bcn_bpcc_op_lock(ad, wdev, TRUE, BCN_BPCC_BTWT) == FALSE) {
		MTWF_PRINT("%s(%d): bcn_bpcc_op_lock fail!\n", __func__, __LINE__);
		if (in_interrupt()) {
			bcn_bpcc_ct_switch(ad, wdev, BCN_BPCC_BTWT);
			return;
		}
	}
	twt_build_btwt_ie(wdev, BCN_BPCC_BTWT);
}


VOID twt_release_btwt_resource(
	struct wifi_dev *wdev)
{
	UINT8 i = 0;

	if (!wdev)
		return;

	if (!TWT_SUPPORT_BTWT(wlan_config_get_he_twt_support(wdev)))
		return;

	if (wdev->btwt_id == 0)
		return;

	for (i = 0; i < TWT_BTWT_ID_NUM; i++) {
		if (wdev->btwt_id & (1 << i))
			twt_remove_btwt_resouce(wdev, i);
	}
}

BOOLEAN twt_check_btwt_member(
	UINT16 *sta_list,
	UINT16 wcid)
{
	UINT8 i = 0;
	BOOLEAN found = FALSE;

	for (i = 0; i < TWT_HW_BTWT_MAX_MEMBER_CNT; i++) {
		if (sta_list[i] == wcid) {
			found = TRUE;
			break;
		}
	}

	return found;
}

BOOLEAN twt_add_btwt_member(
	IN struct twt_link_node *curr_twt_node,
	IN UINT16 wcid)
{
	UINT8 i = 0;
	BOOLEAN done = FALSE;

	for (i = 0 ; i < TWT_HW_BTWT_MAX_MEMBER_CNT; i++) {
		if (curr_twt_node->sta_list[i] == 0) {
			curr_twt_node->sta_list[i] = wcid;
			curr_twt_node->grp_member_cnt++;
			done = TRUE;
			break;
		}
	}

	return done;
}

VOID twt_remove_btwt_member(
	IN struct twt_link_node *curr_twt_node,
	IN UINT16 wcid)
{
	UINT8 i = 0;
	UINT8 j = 0;
	UINT16 temp_wcid[TWT_HW_BTWT_MAX_MEMBER_CNT] = {0};

	for (i = 0; i < TWT_HW_BTWT_MAX_MEMBER_CNT; i++) {
		if (curr_twt_node->sta_list[i] == wcid) {
			curr_twt_node->sta_list[i] = 0;
			curr_twt_node->grp_member_cnt--;
			break;
		}
	}

	for (i = 0; i < TWT_HW_BTWT_MAX_MEMBER_CNT; i++) {
		if (curr_twt_node->sta_list[i] != 0)
			temp_wcid[j++] = curr_twt_node->sta_list[i];
	}

	for (i = 0; i < TWT_HW_BTWT_MAX_MEMBER_CNT; i++)
		curr_twt_node->sta_list[i] = temp_wcid[i];
}

VOID twt_handle_peer_join_btwt_id(
	struct wifi_dev *wdev,
	UINT16 wcid,
	UINT8 twt_cmd_idx,
	struct btwt_para_set *btwt_para_in,
	struct btwt_para_set *btwt_para_out)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	UINT8 sch_link_idx = 0;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	UINT8 band = 0;
	struct TWT_TX_CMD_CTRL_T *twt_cmd = NULL;
	BOOLEAN join = FALSE;
	struct btwt_para_set_fix *btwt_para_fix_in = NULL;
	struct btwt_para_set_fix *btwt_para_fix_out = NULL;

	if (!btwt_para_in)
		return;

	btwt_para_fix_in = &btwt_para_in->btwt_para_fix;
	btwt_para_fix_out = &btwt_para_out->btwt_para_fix;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s: ad=NULL, please check\n", __func__);
		return;
	}

	if (!VALID_UCAST_ENTRY_WCID(ad, wcid)) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"Wcid (%d) isn't valid unicast wcid! \n", wcid);
		return;
	}

	if (twt_cmd_idx >= TWT_CMD_MAX_NUM) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"twt_cmd_idx(%d) is overflow twt_cmd max cmd array!\n",
				twt_cmd_idx);
		return;
	}

	entry = entry_get(ad, wcid);
	twt_ctrl = &ad->twt_ctrl;
	band = HcGetBandByWdev(wdev);

	/* i have this btwt_id & have free space available & peer btwt para meet my config */
	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				return;
			}
			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band)) {
				/* step 5 */
				/* e.i */
				if (GET_BTWT_ID(curr_twt_node) != GET_BTWT_INFO_BTWT_ID(btwt_para_fix_in->btwt_info))
					continue;
				/* a.i */
				if (GET_TWT_RT_REQUEST(btwt_para_fix_in->req_type) != 1) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, twt request=1,%d\n", (UINT16)GET_TWT_RT_REQUEST(btwt_para_fix_in->req_type));
					continue;
				}
				/* a.iii */
				if (GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_TRIGGER) != GET_TWT_RT_TRIGGER(btwt_para_fix_in->req_type)) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, trigger=%d,%d\n",
						GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_TRIGGER),
						(UINT16)GET_TWT_RT_REQUEST(btwt_para_fix_in->req_type));
					continue;
				}
				/* a.v */
				if (GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE) != (GET_TWT_RT_FLOW_TYPE(btwt_para_fix_in->req_type) ? 0 : 1)) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, announce=%d,%d\n",
						GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE),
						(GET_TWT_RT_FLOW_TYPE(btwt_para_fix_in->req_type) ? 1 : 0));
					continue;
				}
				/* a.vi */
				if (curr_twt_node->btwt_recommendation != GET_TWT_RT_BTWT_REC(btwt_para_fix_in->req_type)) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, recommendation=%d,%d\n",
						curr_twt_node->btwt_recommendation,
						(UINT16)GET_TWT_RT_BTWT_REC(btwt_para_fix_in->req_type));
					continue;
				}
				/* a.vii & d */
				if (!twt_check_interval(
					curr_twt_node->agrt_sp_wake_intvl_mantissa,
					curr_twt_node->agrt_sp_wake_intvl_exponent,
					btwt_para_fix_in->mantissa,
					GET_TWT_RT_WAKE_INTVAL_EXP(btwt_para_fix_in->req_type))) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, man/exp=%d,%d,man/exp=%d,%d\n",
						curr_twt_node->agrt_sp_wake_intvl_mantissa,
						curr_twt_node->agrt_sp_wake_intvl_exponent,
						btwt_para_fix_in->mantissa,
						(UINT16)GET_TWT_RT_WAKE_INTVAL_EXP(btwt_para_fix_in->req_type));
					continue;
				}
				/* c */
				if (curr_twt_node->agrt_sp_duration != btwt_para_fix_in->duration) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, sp_duration=%d,%d\n",
						curr_twt_node->agrt_sp_duration,
						btwt_para_fix_in->duration);
					continue;
				}
				if (curr_twt_node->grp_member_cnt >= TWT_HW_BTWT_MAX_MEMBER_CNT) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, grp_member_cnt=%d,%d\n",
						TWT_HW_BTWT_MAX_MEMBER_CNT,
						curr_twt_node->grp_member_cnt);
					continue;
				}

				if (twt_check_btwt_member(curr_twt_node->sta_list, wcid)) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, wcid=%d already in btwt_id=%d\n",
						wcid,
						(UINT16)GET_BTWT_ID(curr_twt_node));
					continue;
				}

				join = twt_add_btwt_member(curr_twt_node, wcid);
				if (join)
					break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

	/* Handle out frame */
	os_move_mem(btwt_para_out, btwt_para_in, sizeof(struct btwt_para_set));
	/* step 6 */
	/* a.i */
	btwt_para_fix_out->req_type &= ~TWT_REQ_TYPE_TWT_REQUEST;
	/* a.ii */
	btwt_para_fix_out->req_type &= ~TWT_REQ_TYPE_TWT_SETUP_COMMAND;
	btwt_para_fix_out->req_type |= (join ? SET_TWT_RT_SETUP_CMD(TWT_SETUP_CMD_ACCEPT) :
										SET_TWT_RT_SETUP_CMD(TWT_SETUP_CMD_REJECT));
	/* b */
	if (join && curr_twt_node)
		twt_update_btwt_action_frame_tsf(wdev, curr_twt_node->schedule_sp_start_tsf, btwt_para_fix_out);
	/* e.ii */
	btwt_para_fix_out->btwt_info &= ~BTWT_INFO_BTWT_P;
	if (join && curr_twt_node)
		btwt_para_fix_out->btwt_info |= SET_BTWT_INFO_BTWT_P(curr_twt_node->persistence);
	/* prepare twt cmd */
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"%s:prepare twt cmd, idx=%d\n", __func__, twt_cmd_idx);
	twt_cmd = &entry->twt_ctrl.twt_cmd;
	twt_cmd->cmd[twt_cmd_idx].opt |= (join ? (OPT_BTWT_REQ | OPT_SETUP_CMD_ACCEPT) : 0);
	twt_cmd->cmd[twt_cmd_idx].twt_node = (join ? curr_twt_node : NULL);
	if (join && curr_twt_node)
		twt_agrt_cmd_set(&twt_cmd->cmd[twt_cmd_idx].agrt_para,
			curr_twt_node,
			TWT_AGRT_CTRL_MODIFY,
			CMD_TSF_TYPE_BTWT);

	if (twt_cmd->cmd_num == 0)
		twt_cmd->token = twt_ctrl_get_token(wdev);
	twt_cmd->cmd_num++;

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"%s to add wcid=%d to btwt_id=%d\n",
		join ? "OK" : "Reject", wcid, (UINT16)GET_BTWT_INFO_BTWT_ID(btwt_para_fix_in->btwt_info));
}

#ifdef DOT11_EHT_BE
VOID rtwt_handle_peer_join_btwt_id(
	struct wifi_dev *wdev,
	UINT16 wcid,
	UINT8 control_in,
	UINT8 twt_cmd_idx,
	struct btwt_para_set *btwt_para_in,
	struct btwt_para_set *btwt_para_out)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	UINT8 sch_link_idx = 0;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct twt_link_node *twt_node = NULL;
	UINT8 band = 0;
	BOOLEAN join = FALSE;
	struct btwt_para_set_fix *btwt_para_fix_in = NULL;
	struct btwt_para_set_fix *btwt_para_fix_out = NULL;
	struct TWT_TX_CMD_CTRL_T *twt_cmd = NULL;

	if (!btwt_para_in || !btwt_para_out) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s: btwt_para_in=NULL or btwt_para_out=NULL, please check\n", __func__);
		return;
	}
	os_move_mem(btwt_para_out, btwt_para_in, sizeof(struct btwt_para_set));

	btwt_para_fix_in = &btwt_para_in->btwt_para_fix;
	btwt_para_fix_out = &btwt_para_out->btwt_para_fix;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s: wdev=NULL, please check\n", __func__);
		return;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s: ad=NULL, please check\n", __func__);
		return;
	}

	if (!VALID_UCAST_ENTRY_WCID(ad, wcid)) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"Wcid (%d) isn't valid unicast wcid! \n", wcid);
		return;
	}

	if (twt_cmd_idx >= TWT_CMD_MAX_NUM) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"twt_cmd_idx(%d) is overflow twt_cmd max cmd array!\n",
				twt_cmd_idx);
		return;
	}

	entry = entry_get(ad, wcid);
	twt_ctrl = &ad->twt_ctrl;
	band = HcGetBandByWdev(wdev);

	/* i have this btwt_id & have free space available & peer btwt para meet current config */
	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				return;
			}
			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band)) {
				/* step 5 */
				/* e.i */
				if (GET_BTWT_ID(curr_twt_node) != GET_BTWT_INFO_BTWT_ID(btwt_para_fix_in->btwt_info))
					continue;
				/* a.i */
				if (GET_TWT_RT_REQUEST(btwt_para_fix_in->req_type) != 1) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, twt request != 1, %d\n", (UINT16)GET_TWT_RT_REQUEST(btwt_para_fix_in->req_type));
					continue;
				}
				/* a.iii */
				if (GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_TRIGGER) != GET_TWT_RT_TRIGGER(btwt_para_fix_in->req_type)) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, trigger=%d, %d\n",
						GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_TRIGGER),
						(UINT16)GET_TWT_RT_REQUEST(btwt_para_fix_in->req_type));
					continue;
				}
				/* a.v */
				if (GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE) != (GET_TWT_RT_FLOW_TYPE(btwt_para_fix_in->req_type) ? 0 : 1)) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, announce=%d, %d\n",
						GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE),
						(GET_TWT_RT_FLOW_TYPE(btwt_para_fix_in->req_type) ? 1 : 0));
					continue;
				}
				if (GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_PROTECT) != (GET_TWT_RT_PROTECTION(btwt_para_fix_in->req_type))) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, protect=%d, %d\n",
						GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_PROTECT),
						(UINT16)GET_TWT_RT_FLOW_TYPE(btwt_para_fix_in->req_type));
					continue;
				}
				/* a.vi */
				if (curr_twt_node->btwt_recommendation != GET_TWT_RT_BTWT_REC(btwt_para_fix_in->req_type)) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, recommendation=%d, %d\n",
						curr_twt_node->btwt_recommendation,
						(UINT16)GET_TWT_RT_BTWT_REC(btwt_para_fix_in->req_type));
					continue;
				}
				/* a.vii & d */
				if (!twt_check_interval(
					curr_twt_node->agrt_sp_wake_intvl_mantissa,
					curr_twt_node->agrt_sp_wake_intvl_exponent,
					btwt_para_fix_in->mantissa,
					GET_TWT_RT_WAKE_INTVAL_EXP(btwt_para_fix_in->req_type))) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, man/exp=%d,%d,man/exp=%d, %d\n",
						curr_twt_node->agrt_sp_wake_intvl_mantissa,
						curr_twt_node->agrt_sp_wake_intvl_exponent,
						btwt_para_fix_in->mantissa,
						(UINT16)GET_TWT_RT_WAKE_INTVAL_EXP(btwt_para_fix_in->req_type));
					continue;
				}
				/* c */
				if (curr_twt_node->agrt_sp_duration != btwt_para_fix_in->duration) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, sp_duration=%d, %d\n",
						curr_twt_node->agrt_sp_duration,
						btwt_para_fix_in->duration);
					continue;
				}

				if (curr_twt_node->grp_member_cnt >= TWT_HW_BTWT_MAX_MEMBER_CNT) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, grp_member_cnt=%d, %d\n",
						TWT_HW_BTWT_MAX_MEMBER_CNT,
						curr_twt_node->grp_member_cnt);
					continue;
				}
				if (twt_check_btwt_member(curr_twt_node->sta_list, wcid)) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Error, wcid=%d already in btwt_id=%d\n",
						wcid,
						(UINT16)GET_BTWT_ID(curr_twt_node));
					continue;
				}

				join = twt_add_btwt_member(curr_twt_node, wcid);
				if (join) {
					/* Update rTWT traffic info from peer */
					curr_twt_node->rtwt_traffic_info_present = GET_BTWT_INFO_RTWT_TRAFFIC_INFO_PRESENT(btwt_para_fix_in->btwt_info);
					curr_twt_node->rtwt_schedule_info = GET_BTWT_INFO_RTWT_SCHEDULE_INFO(btwt_para_fix_in->btwt_info);
					if (GET_BTWT_INFO_RTWT_TRAFFIC_INFO_PRESENT(btwt_para_fix_in->btwt_info)) {
						curr_twt_node->traffic_info_ctrl = btwt_para_in->rtwt_ext_set.traffic_info_ctrl;
						curr_twt_node->tid_dl_bitmap = btwt_para_in->rtwt_ext_set.tid_dl_bitmap;
						curr_twt_node->tid_ul_bitmap = btwt_para_in->rtwt_ext_set.tid_ul_bitmap;
					} else {
						curr_twt_node->traffic_info_ctrl = 0;
						curr_twt_node->tid_dl_bitmap = 0;
						curr_twt_node->tid_ul_bitmap = 0;
					}
					twt_update_btwt_action_frame_tsf(wdev, curr_twt_node->schedule_sp_start_tsf, btwt_para_fix_out);

					/* prepare twt cmd */
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
							"Prepare twt cmd, idx=%d\n", twt_cmd_idx);
					twt_cmd = &entry->twt_ctrl.twt_cmd;
					if (twt_cmd->cmd_num == 0)
						twt_cmd->token = twt_ctrl_get_token(wdev);
					twt_cmd->cmd_num++;
					twt_cmd->cmd[twt_cmd_idx].opt |= (OPT_BTWT_REQ | OPT_SETUP_CMD_ACCEPT);
					twt_cmd->cmd[twt_cmd_idx].twt_node = curr_twt_node;
					twt_agrt_cmd_set(&twt_cmd->cmd[twt_cmd_idx].agrt_para,
									curr_twt_node,
									TWT_AGRT_CTRL_MODIFY,
									CMD_TSF_TYPE_BTWT);
					NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
					goto out;
				}
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

	if (!join) {
		INT ret = 0;
		struct twt_ctrl_btwt btwt_ctrl_para = {0};
		struct TWT_AGRT_MGMT_T agrt_mgmt = {0};

		btwt_ctrl_para.band = band;
		btwt_ctrl_para.btwt_id = GET_BTWT_INFO_BTWT_ID(btwt_para_fix_in->btwt_info);
		btwt_ctrl_para.agrt_sp_duration = btwt_para_fix_in->duration;
		btwt_ctrl_para.agrt_sp_wake_intvl_mantissa = btwt_para_fix_in->mantissa;
		btwt_ctrl_para.agrt_sp_wake_intvl_exponent = GET_TWT_RT_WAKE_INTVAL_EXP(btwt_para_fix_in->req_type);
		if (GET_TWT_CTRL_WAKE_DUR_UNIT(control_in))
			btwt_ctrl_para.agrt_para_bitmap |= TWT_AGRT_PARA_BITMAP_WAKE_DUR_UINT;
		if (GET_TWT_RT_TRIGGER(btwt_para_fix_in->req_type))
			btwt_ctrl_para.agrt_para_bitmap |= TWT_AGRT_PARA_BITMAP_IS_TRIGGER;
		if (!GET_TWT_RT_FLOW_TYPE(btwt_para_fix_in->req_type))
			btwt_ctrl_para.agrt_para_bitmap |= TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE;
		if (GET_TWT_RT_PROTECTION(btwt_para_fix_in->req_type))
			btwt_ctrl_para.agrt_para_bitmap |= TWT_AGRT_PARA_BITMAP_IS_PROTECT;
		btwt_ctrl_para.twt_setup_cmd = GET_TWT_RT_SETUP_CMD(btwt_para_fix_in->req_type);
		btwt_ctrl_para.persistence = GET_BTWT_INFO_BTWT_P(btwt_para_fix_in->btwt_info);
		btwt_ctrl_para.twt_info_frame_dis = GET_TWT_CTRL_INFO_FRM_DIS(control_in);
		btwt_ctrl_para.wake_dur_unit = GET_TWT_CTRL_WAKE_DUR_UNIT(control_in);
		btwt_ctrl_para.btwt_recommendation = GET_TWT_RT_BTWT_REC(btwt_para_fix_in->req_type);
		btwt_ctrl_para.rtwt_traffic_info_present = GET_BTWT_INFO_RTWT_TRAFFIC_INFO_PRESENT(btwt_para_fix_in->btwt_info);
		btwt_ctrl_para.rtwt_schedule_info = GET_BTWT_INFO_RTWT_SCHEDULE_INFO(btwt_para_fix_in->btwt_info);
		if (btwt_ctrl_para.rtwt_traffic_info_present) {
			btwt_ctrl_para.traffic_info_ctrl = btwt_para_in->rtwt_ext_set.traffic_info_ctrl;
			btwt_ctrl_para.tid_dl_bitmap = btwt_para_in->rtwt_ext_set.tid_dl_bitmap;
			btwt_ctrl_para.tid_ul_bitmap = btwt_para_in->rtwt_ext_set.tid_ul_bitmap;
		}
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"band=%d,btwt_id=%d,agrt_sp_duration=%d,bss_idx=%d,agrt_sp_wake_intvl_mantissa=%d,agrt_sp_wake_intvl_exponent=%d\n",
				btwt_ctrl_para.band,
				btwt_ctrl_para.btwt_id,
				btwt_ctrl_para.agrt_sp_duration,
				wdev->bss_info_argument.ucBssIndex,
				btwt_ctrl_para.agrt_sp_wake_intvl_mantissa,
				btwt_ctrl_para.agrt_sp_wake_intvl_exponent);

		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"agrt_para_bitmap=0x%x,twt_setup_cmd=%d,persistence=%d,twt_info_frame_dis=%d,wake_dur_unit=%d,btwt_recommendation=%d\n",
				btwt_ctrl_para.agrt_para_bitmap,
				btwt_ctrl_para.twt_setup_cmd,
				btwt_ctrl_para.persistence,
				btwt_ctrl_para.twt_info_frame_dis,
				btwt_ctrl_para.wake_dur_unit,
				btwt_ctrl_para.btwt_recommendation);

		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"rtwt_traffic_info_present=%d,rtwt_schedule_info=%d,rtwt_traffic_info_present=%d,traffic_info_ctrl=0x%x,tid_dl_bitmap=0x%x,tid_ul_bitmap=0x%x\n",
				btwt_ctrl_para.rtwt_traffic_info_present,
				btwt_ctrl_para.rtwt_schedule_info,
				btwt_ctrl_para.rtwt_traffic_info_present,
				btwt_ctrl_para.traffic_info_ctrl,
				btwt_ctrl_para.tid_dl_bitmap,
				btwt_ctrl_para.tid_ul_bitmap);

		/* allocate twt_node */
		twt_ctrl_acquire_twt_node(wdev, &twt_node, TWT_TYPE_BTWT);
		if (twt_node) {
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"Success. Done added btwt element\n");
			twt_node->band = btwt_ctrl_para.band;
			SET_RTWT_ID(twt_node, btwt_ctrl_para.btwt_id);
			twt_node->agrt_sp_duration = btwt_ctrl_para.agrt_sp_duration;
			twt_node->bss_idx = wdev->bss_info_argument.ucBssIndex;
			twt_node->agrt_sp_wake_intvl_mantissa = btwt_ctrl_para.agrt_sp_wake_intvl_mantissa;
			twt_node->agrt_sp_wake_intvl_exponent = btwt_ctrl_para.agrt_sp_wake_intvl_exponent;
			twt_node->is_role_ap = TWT_ROLE_AP;
			twt_node->agrt_para_bitmap = btwt_ctrl_para.agrt_para_bitmap;
			twt_node->twt_setup_cmd	= btwt_ctrl_para.twt_setup_cmd;
			twt_node->persistence = btwt_ctrl_para.persistence;
			twt_node->twt_info_frame_dis = btwt_ctrl_para.twt_info_frame_dis;
			twt_node->wake_dur_unit = btwt_ctrl_para.wake_dur_unit;
			twt_node->btwt_recommendation = btwt_ctrl_para.btwt_recommendation;
			twt_node->present = TRUE;
			if (twt_node->twt_setup_cmd == TWT_SETUP_CMD_REQUEST) {
				twt_node->tsf_type = TSF_FROM_SETUP_CMD_REQUEST;
			} else if (twt_node->twt_setup_cmd == TWT_SETUP_CMD_SUGGEST) {
				twt_node->tsf_type = TSF_FROM_SETUP_CMD_SUGGEST;
			} else if (twt_node->twt_setup_cmd == TWT_SETUP_CMD_DEMAND) {
				twt_node->tsf_type = TSF_FROM_SETUP_CMD_DEMAND;
				twt_node->schedule_sp_start_tsf = (((UINT64)btwt_para_fix_in->target_wake_time) << 10);
			} else {
				twt_node->tsf_type = TSF_FROM_SETUP_CMD_REQUEST;
			}
			twt_node->rtwt_traffic_info_present = btwt_ctrl_para.rtwt_traffic_info_present;
			twt_node->rtwt_schedule_info = btwt_ctrl_para.rtwt_schedule_info;
			twt_node->traffic_info_ctrl = btwt_ctrl_para.traffic_info_ctrl;
			twt_node->tid_dl_bitmap = btwt_ctrl_para.tid_dl_bitmap;
			twt_node->tid_ul_bitmap = btwt_ctrl_para.tid_ul_bitmap;
			join = twt_add_btwt_member(twt_node, wcid);
			if (!join) {
				twt_ctrl_release_twt_node(twt_node);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						"Fail to do twt_add_btwt_member!\n");
				goto out;
			}

			/* alloc agrt tbl resource */
			ret = twt_ctrl_acquire_twt_agrt(wdev, twt_node, &agrt_mgmt);
			if (ret) {
				twt_ctrl_release_twt_node(twt_node);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						"Fail to do twt_ctrl_acquire_twt_agrt! ret = %d\n", ret);
				join = FALSE;
				goto out;
			}
			twt_node->agrt_tbl_idx = agrt_mgmt.res_id;

			/* schedule this twt_node */
			twt_link_insert_node(wdev, twt_node);

			join = TRUE;

			/* Update btwt id */
			wdev->btwt_id |= (1 << btwt_ctrl_para.btwt_id);

			/* Need to update beacon */
			twt_node->need_bcn_update = TRUE;
		} else {
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"Fail.No free btwt resource available!\n");
			join = FALSE;
			goto out;
		}
	}

	/* prepare twt cmd */
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"Prepare twt cmd, idx=%d\n", twt_cmd_idx);
	twt_cmd = &entry->twt_ctrl.twt_cmd;
	if (twt_cmd->cmd_num == 0)
		twt_cmd->token = twt_ctrl_get_token(wdev);
	twt_cmd->cmd_num++;
	twt_cmd->cmd[twt_cmd_idx].opt |= (OPT_BTWT_REQ | OPT_SETUP_CMD_ACCEPT);
	twt_cmd->cmd[twt_cmd_idx].twt_node = twt_node;
	twt_agrt_cmd_set(&twt_cmd->cmd[twt_cmd_idx].agrt_para,
					twt_node,
					TWT_AGRT_CTRL_ADD,
					CMD_TSF_TYPE_BTWT);
out:
	if (join)
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"OK to add wcid = %d to btwt_id = %d\n",
				wcid, (UINT16)GET_BTWT_INFO_BTWT_ID(btwt_para_fix_in->btwt_info));

	/* Handle out frame */
	/* step 6 */
	/* a.i */
	btwt_para_fix_out->req_type &= ~TWT_REQ_TYPE_TWT_REQUEST;
	/* a.ii */
	btwt_para_fix_out->req_type &= ~TWT_REQ_TYPE_TWT_SETUP_COMMAND;
	btwt_para_fix_out->req_type |= (join ? SET_TWT_RT_SETUP_CMD(TWT_SETUP_CMD_ACCEPT) :
										SET_TWT_RT_SETUP_CMD(TWT_SETUP_CMD_REJECT));
}
#endif /* DOT11_EHT_BE */

/* peer send action frame to AP to leave btwt_id */
VOID twt_handle_peer_leave_btwt_id(
	struct wifi_dev *wdev,
	UINT16 wcid,
	UINT16 btwt_id)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct TWT_CTRL_T *twt_ctrl = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	UINT8 sch_link_idx = 0;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct TWT_AGRT_PARA_T agrt_para = {0};
	UINT8 band = 0;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);

	if (!ad)
		return;

	if (!VALID_UCAST_ENTRY_WCID(ad, wcid))
		return;

	entry = entry_get(ad, wcid);
	twt_ctrl = &ad->twt_ctrl;
	band = HcGetBandByWdev(wdev);

	/* i have this btwt_id & have free space available & peer btwt para meet my config */
	NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEachSafe(curr_twt_node, temp_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				return;
			}
			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band)) {
				if (GET_BTWT_ID(curr_twt_node) != btwt_id)
					continue;

				if (twt_check_btwt_member(curr_twt_node->sta_list, wcid)) {
					SET_PEER_LEAVE_BTWT_ID(entry, btwt_id);
#ifdef DOT11_EHT_BE
					os_zero_mem(&entry->twt_ctrl.rtwt_info[btwt_id], sizeof(struct RTWT_INFO_T));
#endif /* DOT11_EHT_BE */

					twt_remove_btwt_member(curr_twt_node, wcid);

					twt_agrt_cmd_set(&agrt_para,
									curr_twt_node,
									TWT_AGRT_CTRL_MODIFY,
									CMD_TSF_TYPE_BTWT);
					mt_asic_twt_agrt_update_by_cmd_task(wdev, &agrt_para);
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"OK to remove wcid=%d from btwt_id=%d\n",
						wcid, btwt_id);
				} else {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Fail. wcid=%d is not in btwt_id=%d\n",
						wcid, btwt_id);
				}

				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
}

VOID twt_dump_btwt_elem(
	struct wifi_dev *wdev,
	UINT32 *btwt_id_bitmap)
{
	INT ret = 0;

	ret = twt_ctrl_btwt_dump(wdev, btwt_id_bitmap);
	if (ret)
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s: error=%d\n", __func__, ret);
}

BOOLEAN twt_is_sp_duration_tolerance(
	IN struct _RTMP_ADAPTER *ad,
	IN struct itwt_ie_t *twt_ie_in)
{
	struct _RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(ad->hdev_ctrl);

	if (twt_ie_in->duration < pChipCap->twt_sp_duration_min_num)
		return FALSE;

	return TRUE;
}

BOOLEAN twt_is_link_node_valid(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct twt_link_node *twt_node,
	IN BOOLEAN need_lock)
{
	struct TWT_CTRL_T *twt_ctrl = NULL;
	UINT8 sch_link_idx = 0;
	BOOLEAN found = FALSE;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;

	if (!twt_node) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"twt_node=NULL, please check\n");
		return FALSE;
	}
	twt_ctrl = &pAd->twt_ctrl;

	if (need_lock)
		NdisAcquireSpinLock(&twt_ctrl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctrl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				if (need_lock)
					NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);
				return found;
			}

			if (curr_twt_node == twt_node) {
				found = TRUE;
				break;
			}
		}
	}
	if (need_lock)
		NdisReleaseSpinLock(&twt_ctrl->twt_rec_lock);

	return found;
}

#ifdef APCLI_SUPPORT
VOID twtParseTWTElement(
	struct itwt_ie_t *prTWTIE,
	struct twt_params_t *prTWTParams)
{
	UINT16 u2ReqType;

	u2ReqType = le2cpu16(prTWTIE->req_type);

	prTWTParams->fgReq = GET_TWT_RT_REQUEST(u2ReqType);
	prTWTParams->ucSetupCmd = GET_TWT_RT_SETUP_CMD(u2ReqType);
	prTWTParams->fgTrigger = GET_TWT_RT_TRIGGER(u2ReqType);
	prTWTParams->fgUnannounced = GET_TWT_RT_FLOW_TYPE(u2ReqType);
	prTWTParams->ucWakeIntvalExponent = GET_TWT_RT_WAKE_INTVAL_EXP(u2ReqType);
	prTWTParams->fgProtect = GET_TWT_RT_PROTECTION(u2ReqType);

	prTWTParams->u8TWT = le2cpu32(prTWTIE->target_wake_time[0]) |
		(((UINT64)(le2cpu32(prTWTIE->target_wake_time[1]))) << 32);

	prTWTParams->ucMinWakeDur = prTWTIE->duration;
	prTWTParams->u2WakeIntvalMantiss = le2cpu16(prTWTIE->mantissa);
}

UINT8 twtGetRxSetupFlowId(
	struct itwt_ie_t *prTWTIE)
{
	UINT16 u2ReqType;

	ASSERT(prTWTIE);

	if (prTWTIE == NULL)
		return 0;

	u2ReqType = le2cpu16(prTWTIE->req_type);
	return GET_TWT_RT_FLOW_ID(u2ReqType);
}

VOID twtReqFsmRunEventRxSetup(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId
	)
{

	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;

	apcli_entry = &pAd->StaCfg[wdev->func_idx];
	if (!apcli_entry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" apcli_entry=NULL, please check\n");
		return;
	}

	switch (apcli_entry->aeTWTReqState) {
	case TWT_REQ_STATE_WAIT_RSP:
		/* transition to the IDLE state */
		twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_IDLE,
			ucTWTFlowId, NULL);
		break;

	default:
		break;		/* Ignore other cases */
	}

}

VOID twtReqFsmRunEventRxTeardown(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId
)
{
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" apcli_entry=NULL, please check\n");
		return;
	}

	switch (apcli_entry->aeTWTReqState) {
	case TWT_REQ_STATE_IDLE:
		/* transition to the RX TEARDOWN state */
		twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_RX_TEARDOWN,
			ucTWTFlowId, NULL);
		break;

	default:
		break;		/* Ignore other cases */
	}
}

static VOID twtFillTWTElement(
	struct itwt_ie_t *prTWTBuf,
	UINT8 ucTWTFlowId,
	struct twt_params_t *prTWTParams)
{
	/* Add TWT element */
	prTWTBuf->elem_id = IE_TWT;
	prTWTBuf->len = sizeof(struct itwt_ie_t) - 2;

	/* Request Type */
	prTWTBuf->req_type |= SET_TWT_RT_REQUEST(prTWTParams->fgReq) |
		SET_TWT_RT_SETUP_CMD(prTWTParams->ucSetupCmd) |
		SET_TWT_RT_TRIGGER(prTWTParams->fgTrigger) |
		TWT_REQ_TYPE_TWT_IMPLICIT_LAST_BCAST_PARAM |
		SET_TWT_RT_FLOW_TYPE(prTWTParams->fgUnannounced) |
		SET_TWT_RT_FLOW_ID(ucTWTFlowId) |
		SET_TWT_RT_WAKE_INTVAL_EXP(prTWTParams->ucWakeIntvalExponent) |
		SET_TWT_RT_PROTECTION(prTWTParams->fgProtect);
	prTWTBuf->target_wake_time[0] = (cpu2le64(prTWTParams->u8TWT) & 0xFFFFFFFF);
	prTWTBuf->target_wake_time[1] = (cpu2le64(prTWTParams->u8TWT & 0xffffffff00000000) >> 32);
	prTWTBuf->duration = prTWTParams->ucMinWakeDur;
	prTWTBuf->mantissa =
		cpu2le16(prTWTParams->u2WakeIntvalMantiss);
}

UINT32 twtSendSetupFrame(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId,
	IN struct twt_params_t *prTWTParams)
{
	struct frame_itwt_setup frame_out;
	struct itwt_ie_t *twt_ie_out = &frame_out.twt_ie;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	PUCHAR out_buffer = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	ULONG frame_len = 0;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" apcli_entry=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" pEntry=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	if (os_alloc_mem(pAd, &out_buffer, MAX_MGMT_PKT_LEN) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
		" allocate memory failed, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	os_zero_mem(&frame_out, sizeof(struct frame_itwt_setup));
	ActHeaderInit(pAd, &frame_out.hdr, pEntry->Addr, wdev->if_addr, wdev->bssid);
	frame_out.category = CATEGORY_UNPROT_S1G;
	frame_out.s1g_action = CATE_S1G_ACTION_TWT_SETUP;

	twtFillTWTElement(twt_ie_out, ucTWTFlowId, prTWTParams);

	/* send action frame to peer sta */
	MakeOutgoingFrame(out_buffer, &frame_len,
		sizeof(struct frame_itwt_setup), &frame_out,
		END_OF_ARGS);

	MiniportMMRequest(pAd, QID_AC_BE, out_buffer, frame_len, NULL);
	os_free_mem(out_buffer);

	return NDIS_STATUS_SUCCESS;
}

UINT32 twtSendTeardownFrame(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId
	)
{
	struct frame_itwt_teardown frame_out;
	PUCHAR out_buffer = NULL;
	ULONG frame_len = 0;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (!wdev) {
		MTWF_DBG(pAd,DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_DBG(pAd,DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" apcli_entry=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry) {
		MTWF_DBG(pAd,DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" pEntry=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	/* send action frame to peer sta */
	if (os_alloc_mem(pAd, &out_buffer, MAX_MGMT_PKT_LEN) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd,DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
		" allocate memory failed, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	os_zero_mem(&frame_out, sizeof(struct frame_itwt_teardown));
	ActHeaderInit(pAd, &frame_out.hdr, pEntry->Addr, wdev->if_addr, wdev->bssid);
	frame_out.category = CATEGORY_UNPROT_S1G;
	frame_out.s1g_action = CATE_S1G_ACTION_TWT_TEARDOWN;
	frame_out.twt_flow_id = ucTWTFlowId;

	MakeOutgoingFrame(out_buffer,
		&frame_len,
		sizeof(struct frame_itwt_teardown),
		&frame_out,
		END_OF_ARGS);
	MiniportMMRequest(pAd,
		(MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[QID_AC_VO]),
		out_buffer, frame_len, NULL);

	os_free_mem(out_buffer);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
		" wcid(%d), flow_id(%d)\n",
		pEntry->wcid,
		ucTWTFlowId);

	return NDIS_STATUS_SUCCESS;
}

static UINT8 twtPlannerDrvAgrtFind(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId
	)
{
	UINT8 i;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	struct twt_planner_t *prTWTPlanner = NULL;
	struct twt_agrt_t *prTWTAgrt = NULL;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	prTWTPlanner = &(apcli_entry->rTWTPlanner);

	for (i = 0; i < TWT_AGRT_MAX_NUM; i++) {

		prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[i]);

		if (prTWTAgrt->fgValid == TRUE &&
			prTWTAgrt->ucFlowId == ucTWTFlowId &&
			prTWTAgrt->ucBssIdx == wdev->bss_info_argument.ucBssIndex)
			break;
	}

	return i;
}

static UINT32 twtPlannerDrvAgrtDel(
	IN struct twt_planner_t *prTWTPlanner,
	IN UINT8 ucIdx
	)
{
	struct twt_agrt_t *prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[ucIdx]);

	NdisZeroMemory(prTWTAgrt, sizeof(struct twt_agrt_t));

	return NDIS_STATUS_SUCCESS;
}

static UINT32 twtPlannerDelAgrtTbl(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId,
	IN UINT8 fgDelDrvEntry)
{
	UINT8 ucAgrtTblIdx;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	NDIS_STATUS status = NDIS_STATUS_FAILURE;
	struct twt_planner_t *prTWTPlanner = NULL;
	struct TWT_AGRT_PARA_T agrt_para = {0};

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" apcli_entry=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" pEntry=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	prTWTPlanner = &(apcli_entry->rTWTPlanner);

	/* Find and delete the agreement entry in the driver */
	ucAgrtTblIdx = twtPlannerDrvAgrtFind(pAd, wdev, ucTWTFlowId);

	if (ucAgrtTblIdx >= TWT_AGRT_MAX_NUM)
		return NDIS_STATUS_FAILURE;

	if (fgDelDrvEntry)
		twtPlannerDrvAgrtDel(prTWTPlanner, ucAgrtTblIdx);

	agrt_para.agrt_tbl_idx = ucAgrtTblIdx;
	agrt_para.agrt_ctrl_flag = TWT_AGRT_CTRL_DELETE;
	agrt_para.own_mac_idx = wdev->OmacIdx;
	agrt_para.flow_id = ucTWTFlowId;
	agrt_para.peer_id_grp_id = pEntry->wcid;
	agrt_para.bss_idx = wdev->bss_info_argument.ucBssIndex;
	/* apcli */
	agrt_para.is_role_ap = TWT_ROLE_APCLI;
	mt_asic_twt_agrt_update(wdev, &agrt_para);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
		"del twt agrt to FW,wcid=%d,flow_id=%d,tbl_idx=%d\n",
		 pEntry->wcid, ucTWTFlowId, ucAgrtTblIdx);

	return status;
}

UINT32 twtPlannerTeardownDone(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId
	)
{
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"apcli_entry=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" pEntry=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	/* Delete driver & FW TWT agreement entry */
	twtPlannerDelAgrtTbl(pAd, wdev, ucTWTFlowId, TRUE);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		" wcid(%d), flow_id(%d)\n",
		pEntry->wcid, ucTWTFlowId);

	return NDIS_STATUS_SUCCESS;
}

VOID twtPlannerSetParams(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN struct twt_ctrl_t rtwtCtrl)
{
	struct twt_ctrl_t rTWTCtrl, *prTWTCtrl = &rTWTCtrl;
	UINT_8 ucBssIdx, ucFlowId;
	NDIS_STATUS status = NDIS_STATUS_FAILURE;
	struct twt_get_tsf_context_t *prGetTsfCtxt = NULL;

	NdisCopyMemory(prTWTCtrl, &rtwtCtrl, sizeof(rTWTCtrl));

	ucBssIdx = prTWTCtrl->ucBssIdx;
	pAd = (struct _RTMP_ADAPTER *)(wdev->sys_handle);

	if (IS_TWT_PARAM_ACTION_ADD_BYPASS(prTWTCtrl->ucCtrlAction) ||
		IS_TWT_PARAM_ACTION_ADD(prTWTCtrl->ucCtrlAction)) {

		status = os_alloc_mem(pAd, (UCHAR **)&prGetTsfCtxt, sizeof(struct twt_get_tsf_context_t));

		if (prGetTsfCtxt == NULL)
			return;

		if (IS_TWT_PARAM_ACTION_ADD_BYPASS(prTWTCtrl->ucCtrlAction))
			prGetTsfCtxt->ucReason = TWT_GET_TSF_FOR_ADD_AGRT_BYPASS;
		else
			prGetTsfCtxt->ucReason = TWT_GET_TSF_FOR_ADD_AGRT;

		prGetTsfCtxt->ucBssIdx = ucBssIdx;
		prGetTsfCtxt->ucTWTFlowId = prTWTCtrl->ucTWTFlowId;
		NdisCopyMemory(&(prGetTsfCtxt->rTWTParams),
				&(prTWTCtrl->rTWTParams),
				sizeof(struct twt_params_t));
		twtPlannerGetCurrentTSF(pAd,
			wdev, prGetTsfCtxt, sizeof(*prGetTsfCtxt));



		if (prGetTsfCtxt)
			os_free_mem(prGetTsfCtxt);

	}

	ucFlowId = prTWTCtrl->ucTWTFlowId;

	switch (prTWTCtrl->ucCtrlAction) {
	case TWT_PARAM_ACTION_DEL:
		if (twtPlannerDrvAgrtFind(pAd,
			wdev, ucFlowId) < TWT_AGRT_MAX_NUM) {
			/* Start the process to tear down this TWT agreement */
			twtReqFsmSendEvent(pAd, wdev,
				ucFlowId, MID_TWT_REQ_FSM_TEARDOWN);
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"BSS %u TWT flow %u doesn't exist\n\n", ucBssIdx, ucFlowId);
		}
		break;
	default:
		break;
	}
}

VOID twtPlannerRxNegoResult(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId)
{
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct twt_flow_t *prTWTFlow;
	struct twt_params_t *prTWTResult;
	struct twt_params_t *prTWTParams;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" apcli_entry=NULL, please check\n");
		return;
	}

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" pEntry=NULL, please check\n");
		return;
	}

	prTWTFlow = &(apcli_entry->arTWTFlow[ucTWTFlowId]);
	prTWTResult = &(prTWTFlow->rTWTPeerParams);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"ucSetupCmd=%d\n", prTWTResult->ucSetupCmd);

	switch (prTWTResult->ucSetupCmd) {
	case TWT_SETUP_CMD_ACCEPT:
		/* Update agreement table */
		twtPlannerAddAgrtTbl(pAd, wdev, pEntry,
			prTWTResult, ucTWTFlowId);
		break;

	case TWT_SETUP_CMD_ALTERNATE:
	case TWT_SETUP_CMD_DICTATE:
		/* Use AP's suggestions */
		prTWTParams = &(prTWTFlow->rTWTParams);
		NdisCopyMemory(prTWTParams, prTWTResult, sizeof(struct twt_params_t));
		prTWTParams->ucSetupCmd = TWT_SETUP_CMD_SUGGEST;
		prTWTParams->fgReq = 1;
		twtReqFsmSendEvent(pAd, wdev, ucTWTFlowId, MID_TWT_REQ_FSM_START);
		break;

	case TWT_SETUP_CMD_REJECT:
		/* Clear TWT flow in StaRec */
		break;

	default:
		ASSERT(0);
		break;
	}
}

VOID twtReqFsmSendEvent(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId,
	IN UINT8 eMsgId)
{
	struct msg_twt_fsm_t rTWTReqFsmMsg = {0};

	rTWTReqFsmMsg.eMsgId = eMsgId;
	rTWTReqFsmMsg.wdev = wdev;
	rTWTReqFsmMsg.ucTWTFlowId = ucTWTFlowId;

	MlmeEnqueueWithWdev(pAd,
		ACTION_STATE_MACHINE,
		MT2_MLME_S1G_CATE_TWT_SETUP,
		sizeof(struct msg_twt_fsm_t),
		(PVOID)&rTWTReqFsmMsg,
		0,
		wdev, FALSE, NULL);

	RTMP_MLME_HANDLER(pAd);
}

VOID twtReqFsmSteps(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN enum ENUM_TWT_REQUESTER_STATE_T eNextState,
	IN UINT8 ucTWTFlowId,
	IN void *pParam)
{
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	enum ENUM_TWT_REQUESTER_STATE_T ePreState;
	UINT8 fgIsTransition;
	NDIS_STATUS rStatus = NDIS_STATUS_SUCCESS;

	if (wdev == NULL)
		return;

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	do {
		ePreState = apcli_entry->aeTWTReqState;
		apcli_entry->aeTWTReqState = eNextState;
		fgIsTransition = (UINT8) FALSE;

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"twtReqFsmSteps ePreState=%d, eNextState=%d\n",
				ePreState, eNextState);

		switch (apcli_entry->aeTWTReqState) {
		case TWT_REQ_STATE_IDLE:
			/* Notify TWT Planner of the negotiation result */
			if (ePreState == TWT_REQ_STATE_WAIT_RSP) {
				twtReqFsmSendEvent(pAd, wdev,
					ucTWTFlowId, MID_TWT_REQ_IND_RESULT);
				/* TODO: how to handle failures */
			} else if (ePreState == TWT_REQ_STATE_TEARING_DOWN) {
				twtReqFsmSendEvent(pAd, wdev,
					ucTWTFlowId,
					MID_TWT_REQ_IND_TEARDOWN_DONE);
			} else if (ePreState == TWT_REQ_STATE_RESUMING) {
				twtReqFsmSendEvent(pAd, wdev,
					ucTWTFlowId,
					MID_TWT_REQ_IND_RESUME_DONE);
			}
			break;
		case TWT_REQ_STATE_REQTX:
			{
				struct twt_params_t *prTWTParams =
					(struct twt_params_t *)pParam;
				if (unlikely(prTWTParams == NULL)) {
					eNextState = TWT_REQ_STATE_IDLE;
					fgIsTransition = TRUE;
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
								"prTWTParams is NULL, TWT_REQ TX Failure!\n");
					ASSERT(prTWTParams);
					break;
				}
				rStatus = twtSendSetupFrame(
					pAd, wdev, ucTWTFlowId,
					prTWTParams);
				if (rStatus != NDIS_STATUS_SUCCESS) {
					eNextState = TWT_REQ_STATE_IDLE;
					fgIsTransition = TRUE;
				}
				break;
			}
		case TWT_REQ_STATE_WAIT_RSP:
			break;
		case TWT_REQ_STATE_TEARING_DOWN:
			rStatus = twtSendTeardownFrame(
				pAd, wdev, ucTWTFlowId);
			if (rStatus != NDIS_STATUS_SUCCESS) {
				eNextState = TWT_REQ_STATE_IDLE;
				fgIsTransition = TRUE;
			}
			break;
		case TWT_REQ_STATE_RX_TEARDOWN:
			twtReqFsmSendEvent(pAd, wdev,
				ucTWTFlowId, MID_TWT_REQ_IND_TEARDOWN_DONE);
			break;
		default:
			/* nothing to do */
			break;
		}
	} while (fgIsTransition);
}

UINT32 twtPlannerDrvAgrtInsert(
	IN struct twt_planner_t *prTWTPlanner,
	IN UINT8 ucBssIdx,
	IN UINT8 ucFlowId,
	IN struct twt_params_t *prTWTParams,
	IN UINT8 ucIdx)
{
	struct twt_agrt_t *prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[ucIdx]);

	prTWTAgrt->fgValid = TRUE;
	prTWTAgrt->ucBssIdx = ucBssIdx;
	prTWTAgrt->ucFlowId = ucFlowId;
	prTWTAgrt->ucAgrtTblIdx = ucIdx;
	NdisCopyMemory(&(prTWTAgrt->rTWTAgrt), prTWTParams,
		sizeof(struct twt_params_t));

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		" ucIdx=%d, ucBssIdx=%d, ucFlowId=%d\n",
			ucIdx, ucBssIdx, ucFlowId);

	return NDIS_STATUS_SUCCESS;
}

UINT32 twtPlannerDrvAgrtAdd(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucFlowId,
	IN struct twt_params_t *prTWTParams,
	IN UINT8 *pucIdx)
{
	UINT8 ucIdx;
	UINT32 rStatus = NDIS_STATUS_FAILURE;
	struct twt_planner_t *prTWTPlanner = NULL;
	struct twt_agrt_t *prTWTAgrt = NULL;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];
	prTWTPlanner = &(apcli_entry->rTWTPlanner);

	for (ucIdx = 0; ucIdx < TWT_AGRT_MAX_NUM; ucIdx++) {
		prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[ucIdx]);
		if (prTWTAgrt->fgValid == FALSE)
			break;
	}

	if (ucIdx < TWT_AGRT_MAX_NUM) {
		twtPlannerDrvAgrtInsert(prTWTPlanner, wdev->bss_info_argument.ucBssIndex,
			ucFlowId, prTWTParams, ucIdx);
		*pucIdx = ucIdx;
		rStatus = NDIS_STATUS_SUCCESS;
	}

	return rStatus;
}

UINT32 twtPlannerAddAgrtTbl(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN VOID *pEntry,
	IN struct twt_params_t *prTWTParams,
	IN UINT8 ucFlowId)
{
	UINT8 ucAgrtTblIdx;
	UINT32 rWlanStatus = NDIS_STATUS_SUCCESS;
	struct TWT_AGRT_PARA_T agrt_para = {0};
	struct _MAC_TABLE_ENTRY *_pEntry = NULL;

	twtPlannerDbgPrintVal(pAd, prTWTParams);

	if (pEntry == NULL)
		return NDIS_STATUS_FAILURE;

	_pEntry = (struct _MAC_TABLE_ENTRY *)pEntry;

	if (wdev == NULL)
		return NDIS_STATUS_FAILURE;

	rWlanStatus = twtPlannerDrvAgrtAdd(pAd, wdev,
		ucFlowId, prTWTParams, &ucAgrtTblIdx);

	if (rWlanStatus) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
				" Agreement table is full\n");
		return NDIS_STATUS_FAILURE;
	}

	agrt_para.agrt_tbl_idx = ucAgrtTblIdx;
	agrt_para.agrt_ctrl_flag = TWT_AGRT_CTRL_ADD;
	agrt_para.own_mac_idx = wdev->OmacIdx;
	agrt_para.flow_id = ucFlowId;
	agrt_para.peer_id_grp_id = _pEntry->wcid;
	agrt_para.agrt_sp_duration = prTWTParams->ucMinWakeDur;
	agrt_para.bss_idx = wdev->bss_info_argument.ucBssIndex;
	agrt_para.agrt_sp_start_tsf_low = prTWTParams->u8TWT & 0xFFFFFFFF;
	agrt_para.agrt_sp_start_tsf_high = (UINT32)(prTWTParams->u8TWT >> 32);
	agrt_para.agrt_sp_wake_intvl_mantissa = prTWTParams->u2WakeIntvalMantiss;
	agrt_para.agrt_sp_wake_intvl_exponent = prTWTParams->ucWakeIntvalExponent;
	/* TODO: aplci might need to use ap role */
	agrt_para.is_role_ap = TWT_ROLE_APCLI;
	agrt_para.agrt_para_bitmap =
		((prTWTParams->fgProtect << TWT_AGRT_PARA_BITMAP_PROTECT_OFFSET) |
		((!prTWTParams->fgUnannounced) << TWT_AGRT_PARA_BITMAP_ANNCE_OFFSET) |
		(prTWTParams->fgTrigger << TWT_AGRT_PARA_BITMAP_TRIGGER_OFFSET));

	agrt_para.grp_member_cnt = 0;

	mt_asic_twt_agrt_update(wdev, &agrt_para);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
		" set twt agrt to FW,wcid=%d,flow_id=%d,tbl_idx=%d\n",
		 _pEntry->wcid, ucFlowId, ucAgrtTblIdx);

	return NDIS_STATUS_SUCCESS;
}

static struct twt_flow_t *twtPlannerFlowFindById(
	IN struct _STA_ADMIN_CONFIG *prStaCfg,
	IN UINT8 ucFlowId)
{
	struct twt_flow_t *prTWTFlow = NULL;

	ASSERT(prStaCfg);

	if (ucFlowId >= TWT_MAX_FLOW_NUM) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"Invalid TWT flow id %u\n",
				ucFlowId);
		return NULL;
	}

	prTWTFlow = &(prStaCfg->arTWTFlow[ucFlowId]);

	return prTWTFlow;
}

UINT32 twtPlannerGetCurrentTSF(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN struct twt_get_tsf_context_t *prGetTsfCtxt,
	IN UINT32 u4SetBufferLen)
{
	UINT32 current_tsf[2] = {0};
	UINT64 u8CurTsf;
	UINT8 ucBssIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;

	if (wdev == NULL)
		return NDIS_STATUS_FAILURE;

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (apcli_entry == NULL)
		return NDIS_STATUS_FAILURE;

	twt_get_current_tsf(wdev, current_tsf);

	if (current_tsf[0] == 0 && current_tsf[1] == 0)
		return NDIS_STATUS_FAILURE;

	u8CurTsf = le2cpu32(current_tsf[0]) |
		(((UINT64)(le2cpu32(current_tsf[1]))) << 32);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		" u8CurTsf=%llu, Reason=%d\n",
		u8CurTsf, prGetTsfCtxt->ucReason);

	ucBssIdx = prGetTsfCtxt->ucBssIdx;

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (pEntry == NULL)
		return NDIS_STATUS_FAILURE;

	switch (prGetTsfCtxt->ucReason) {
	case TWT_GET_TSF_FOR_ADD_AGRT_BYPASS:
		prGetTsfCtxt->rTWTParams.u8TWT = u8CurTsf + TSF_OFFSET_FOR_EMU;
		twtPlannerAddAgrtTbl(pAd, wdev,
			pEntry, &(prGetTsfCtxt->rTWTParams),
			prGetTsfCtxt->ucTWTFlowId);
		break;

	case TWT_GET_TSF_FOR_ADD_AGRT:
	{
		struct twt_params_t *prTWTParams;
		struct twt_flow_t *prTWTFlow = twtPlannerFlowFindById(apcli_entry,
			prGetTsfCtxt->ucTWTFlowId);

		if (!prTWTFlow)	{
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"NULL Pointer!\n");
			return NDIS_STATUS_FAILURE;
		}


		prGetTsfCtxt->rTWTParams.u8TWT =
			u8CurTsf + TSF_OFFSET_FOR_AGRT_ADD;
		prTWTParams = &(prTWTFlow->rTWTParams);
		NdisCopyMemory(prTWTParams, &(prGetTsfCtxt->rTWTParams),
			sizeof(struct twt_params_t));
		/* Start the process to nego for a new agreement */
		twtReqFsmSendEvent(pAd,
			wdev, prGetTsfCtxt->ucTWTFlowId, MID_TWT_REQ_FSM_START);

		break;
	}
	default:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"Unknown reason to get TSF %u\n", prGetTsfCtxt->ucReason);
		break;
	}

	return NDIS_STATUS_SUCCESS;
}

VOID twtTxDoneCheckSetupFrame(
	IN struct _RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pkt)
{
	HEADER_802_11 *pHead;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	UCHAR wdev_idx = 0;
	PFRAME_ACTION_HDR pActHdr;
	struct wifi_dev *wdev = NULL;
	struct frame_itwt_setup frame_setup;
	struct frame_itwt_teardown frame_tear_down;
	struct itwt_ie_t *twt_ie_out;
	UCHAR ucTWTFlowId;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;

	wdev_idx = RTMP_GET_PACKET_WDEV(pkt);
	wdev = pAd->wdev_list[wdev_idx];

	if (wdev == NULL || (wdev->wdev_type != WDEV_TYPE_STA))
		return;

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	pHead = (PHEADER_802_11)(GET_OS_PKT_DATAPTR(pkt) + tx_hw_hdr_len);

	if (pHead->FC.Type == FC_TYPE_MGMT && pHead->FC.SubType == SUBTYPE_ACTION) {
		pActHdr = (PFRAME_ACTION_HDR)pHead;

		if (pActHdr->Category != CATEGORY_UNPROT_S1G)
			return;

		switch (pActHdr->Action) {
		case CATE_S1G_ACTION_TWT_SETUP:
			if (apcli_entry->aeTWTReqState == TWT_REQ_STATE_REQTX) {
				NdisCopyMemory(&frame_setup, pHead, sizeof(struct frame_itwt_setup));
				twt_ie_out = &frame_setup.twt_ie;
				ucTWTFlowId = twtGetRxSetupFlowId(twt_ie_out);
				twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_WAIT_RSP, ucTWTFlowId, NULL);
			}
			break;

		case CATE_S1G_ACTION_TWT_TEARDOWN:
			if (apcli_entry->aeTWTReqState == TWT_REQ_STATE_TEARING_DOWN) {
				NdisCopyMemory(&frame_tear_down, pHead, sizeof(struct frame_itwt_teardown));
				ucTWTFlowId = twtGetTxTeardownFlowId(&frame_tear_down);
				twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_IDLE, ucTWTFlowId, NULL);
			}
			break;

		default:
			break;
		} /* End of switch */
	}
}

VOID twtPlannerDbgPrintVal(
	IN struct _RTMP_ADAPTER *ad,
	IN struct twt_params_t *prTWTParams)
{
	MTWF_PRINT("prTWTParams->fgReq = %d\n", prTWTParams->fgReq);
	MTWF_PRINT("prTWTParams->ucSetupCmd = %d\n", prTWTParams->ucSetupCmd);
	MTWF_PRINT("prTWTParams->fgTrigger = %d\n", prTWTParams->fgTrigger);
	MTWF_PRINT("prTWTParams->fgUnannounced = %d\n", prTWTParams->fgUnannounced);
	MTWF_PRINT("prTWTParams->ucWakeIntvalExponent = %d\n", prTWTParams->ucWakeIntvalExponent);
	MTWF_PRINT("prTWTParams->tsf_low = 0x%.8x\n", cpu2le32(prTWTParams->u8TWT & 0xFFFFFFFF));
	MTWF_PRINT("prTWTParams->tsf_high = 0x%.8x\n", cpu2le32((UINT32)(prTWTParams->u8TWT >> 32)));
	MTWF_PRINT("prTWTParams->fgProtect = %d\n", prTWTParams->fgProtect);
	MTWF_PRINT("prTWTParams->ucMinWakeDur = %d\n", prTWTParams->ucMinWakeDur);
	MTWF_PRINT("prTWTParams->u2WakeIntvalMantiss = %d\n", prTWTParams->u2WakeIntvalMantiss);
}

UINT32 twtGetTxTeardownFlowId(
	IN struct frame_itwt_teardown *pframe_tear_down)
{
	UINT8 ucFlowId;

	ucFlowId = (pframe_tear_down->twt_flow_id & TWT_TEARDOWN_FLOW_ID);
	return ucFlowId;
}

BOOLEAN twtPlannerIsRunning(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _STA_ADMIN_CONFIG *prStaCfg)
{
	struct twt_planner_t *prTWTPlanner = NULL;
	struct twt_agrt_t *prTWTAgrt = NULL;
	UINT8 ucIdx;
	BOOLEAN bFound = FALSE;

	ASSERT(prStaCfg);
	if (!prStaCfg) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"prStaCfg is NULL, please check\n");
		return bFound;
	}

	prTWTPlanner = &prStaCfg->rTWTPlanner;

	for (ucIdx = 0; ucIdx < TWT_AGRT_MAX_NUM; ucIdx++) {
		prTWTAgrt = &prTWTPlanner->arTWTAgrtTbl[ucIdx];
		if ((prTWTAgrt != NULL) && (prTWTAgrt->fgValid == TRUE)) {
			bFound = TRUE;
			break;
		}
	}

	return bFound;
}

VOID twtPlannerDrvAgrtDelAll(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _STA_ADMIN_CONFIG *prStaCfg)
{
	UINT8 i;
	struct twt_planner_t *prTWTPlanner = NULL;
	struct twt_agrt_t *prTWTAgrt = NULL;
	struct wifi_dev *wdev = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct TWT_AGRT_PARA_T agrt_para = {0};

	ASSERT(prStaCfg);
	if (!prStaCfg) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"prStaCfg is NULL, please check\n");
		return;
	}

	pEntry = (MAC_TABLE_ENTRY *)prStaCfg->pAssociatedAPEntry;
	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" pEntry=NULL, please check\n");
		return;
	}

	wdev = &prStaCfg->wdev;
	prTWTPlanner = &prStaCfg->rTWTPlanner;
	for (i = 0; i < TWT_AGRT_MAX_NUM; i++) {
		prTWTAgrt = &prTWTPlanner->arTWTAgrtTbl[i];

		if (prTWTAgrt &&
			(prTWTAgrt->fgValid == TRUE) &&
			(prTWTAgrt->ucBssIdx == wdev->bss_info_argument.ucBssIndex)) {
			agrt_para.agrt_tbl_idx = i;
			agrt_para.agrt_ctrl_flag = TWT_AGRT_CTRL_DELETE;
			agrt_para.own_mac_idx = wdev->OmacIdx;
			agrt_para.flow_id = prTWTAgrt->ucFlowId;
			agrt_para.peer_id_grp_id = pEntry->wcid;
			agrt_para.bss_idx = wdev->bss_info_argument.ucBssIndex;
			/* apcli */
			agrt_para.is_role_ap = TWT_ROLE_APCLI;
			mt_asic_twt_agrt_update(wdev, &agrt_para);

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"del twt agrt to FW,wcid=%d,flow_id=%d,tbl_idx=%d\n",
				 pEntry->wcid, prTWTAgrt->ucFlowId, i);

			twtPlannerDrvAgrtDel(prTWTPlanner, i);
		}
	}
}

VOID twtMlmeSetupAction(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct msg_twt_fsm_t *msg = NULL;
	struct wifi_dev *wdev = NULL;
	UINT8 ucFlowId;
	UCHAR eMsgId;
	struct twt_flow_t *prTWTFlow = NULL;
	STA_ADMIN_CONFIG *apcli_entry = NULL;
	struct twt_params_t *prTWTParams;

	msg = (struct msg_twt_fsm_t *)&elem->Msg;
	wdev = msg->wdev;
	ucFlowId = msg->ucTWTFlowId;
	eMsgId = msg->eMsgId;

	if (wdev == NULL)
		return;

	apcli_entry = &pAd->StaCfg[wdev->func_idx];
	prTWTFlow = twtPlannerFlowFindById(apcli_entry, ucFlowId);

	if (prTWTFlow == NULL)
		return;

	prTWTParams = &(prTWTFlow->rTWTParams);

	switch (eMsgId) {
	case MID_TWT_PARAMS_SET:
		twtPlannerSetParams(pAd, wdev, msg->rtwtCtrl);
		break;
	case MID_TWT_REQ_FSM_START:
		twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_REQTX, ucFlowId, prTWTParams);
		twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_WAIT_RSP, ucFlowId, prTWTParams);
		break;
	case MID_TWT_REQ_FSM_TEARDOWN:
		twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_TEARING_DOWN, ucFlowId, prTWTParams);
		twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_IDLE, ucFlowId, prTWTParams);
		break;
	case MID_TWT_REQ_IND_TEARDOWN_DONE:
		twtPlannerTeardownDone(pAd, wdev, ucFlowId);
		break;
	case MID_TWT_REQ_IND_RESULT:
		twtPlannerRxNegoResult(pAd, wdev, ucFlowId);
		break;
	default:
		/* nothing to do */
		break;
	}
}
#endif /* APCLI_SUPPORT */
#endif /* WIFI_TWT_SUPPORT */
#endif /* #ifdef DOT11_HE_AX */
