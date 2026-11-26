/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2019, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/
#include "rt_config.h"
#include "ser_mngr.h"
#ifdef MT7990
#include "mt7990_ser.h"
#endif /* MT7990 */
#ifdef MT7992
#include "mt7992_ser.h"
#endif /* MT7992 */
#ifdef MT7993
#include "mt7993_ser.h"
#endif /* MT7993 */

#ifdef ERR_RECOVERY

static struct ser_tbl_t ser_tbl[] = {
#ifdef MT7990
	{
		.chip_id = 0x7990,
		.ser_1_0 = mt7990_ser_1_0_v1,
		.ser_0_5 = mt7990_ser_0_5_v1,
		.ser_0_0 = mt7990_ser_0_0_v1,
		.ser_dump_ser_stat = mt7990_dump_ser_stat,
#ifdef MTK_FE_RESET_RECOVER
		.fe_fe_reset = mt7990_fe_reset,
		.fe_fe_wdma_reset = mt7990_fe_wdma_reset,
#endif
		.ser_10_0 = mt7990_ser_10_0_v1,
	},
#endif /* MT7990 */
#ifdef MT7992
	{
		.chip_id = 0x7992,
		.ser_1_0 = mt7992_ser_1_0_v1,
		.ser_0_5 = mt7992_ser_0_5_v1,
		.ser_0_0 = mt7992_ser_0_0_v1,
		.ser_dump_ser_stat = mt7992_dump_ser_stat,
#ifdef MTK_FE_RESET_RECOVER
		.fe_fe_reset = mt7992_fe_reset,
		.fe_fe_wdma_reset = mt7992_fe_wdma_reset,
#endif
		.ser_10_0 = mt7992_ser_10_0_v1,
	},
#endif /* MT7992 */
#ifdef MT7993
	{
		.chip_id = 0x7993,
		.ser_1_0 = mt7993_ser_1_0_v1,
		.ser_0_5 = mt7993_ser_0_5_v1,
		.ser_0_0 = mt7993_ser_0_0_v1,
		.ser_dump_ser_stat = mt7993_dump_ser_stat,
#ifdef MTK_FE_RESET_RECOVER
		.fe_fe_reset = mt7993_fe_reset,
		.fe_fe_wdma_reset = mt7993_fe_wdma_reset,
#endif
		.ser_10_0 = mt7993_ser_10_0_v1,
	},
#endif /* MT7993 */
};

struct ser_ctrl_t ser_ctrl;

static struct ser_ctrl_t *ser_get_ser_ctrl(
	void)
{
	return &ser_ctrl;
}

int ser_event_enq(
	UINT32 chip_id,
	UINT32 ser_level,
	UINT32 ser_event,
	UINT32 hw_id)
{
	int status = NDIS_STATUS_SUCCESS;
	struct ser_ctrl_t *_ser_ctrl = &ser_ctrl;
	NDIS_SPIN_LOCK *lock = &_ser_ctrl->event_q_lock;
	unsigned long flags;
	struct ser_event_cb_t *event_cb = NULL;
	UINT32 ser_func_state = 0;

	NdisAcquireSpinLock(&_ser_ctrl->ser_lock);
	ser_func_state = _ser_ctrl->ser_func_state;
	NdisReleaseSpinLock(&_ser_ctrl->ser_lock);

	if (ser_func_state != RTMP_TASK_STAT_RUNNING)
		return NDIS_STATUS_FAILURE;

	status = os_alloc_mem(NULL, (UINT8 **)&event_cb, sizeof(struct ser_event_cb_t));
	if (!event_cb)
		return NDIS_STATUS_RESOURCES;

	NdisZeroMemory(event_cb, sizeof(struct ser_event_cb_t));

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	event_cb->chip_id = chip_id;
	event_cb->ser_level = ser_level;
	event_cb->ser_event = ser_event;
	event_cb->hw_id = hw_id;
	DlListAddTail(&_ser_ctrl->event_q, &event_cb->list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);

	RTCMDUp(&_ser_ctrl->ser_task);

	return status;
}

static struct ser_event_cb_t *ser_event_deq(
	struct ser_ctrl_t *_ser_ctrl)
{
	NDIS_SPIN_LOCK *lock = &_ser_ctrl->event_q_lock;
	unsigned long flags;
	struct ser_event_cb_t *event_cb = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	event_cb = DlListFirst(&_ser_ctrl->event_q, struct ser_event_cb_t, list);

	if (!event_cb) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
		return NULL;
	}

	_ser_ctrl->event_times++;
	DlListDel(&event_cb->list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
		"event_times=%d\n", _ser_ctrl->event_times);

	return event_cb;
}

static UINT8 ser_event_q_empty(
	struct ser_ctrl_t *_ser_ctrl)
{
	NDIS_SPIN_LOCK *lock = &_ser_ctrl->event_q_lock;
	unsigned long flags;
	UINT8 empty = 0;

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	empty = DlListEmpty(&_ser_ctrl->event_q) ? 1 : 0;
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);

	return empty;
}


static UINT8 ser_get_first_state(
	UINT8 level)
{
	UINT8 first_state = 0;

	if (level == SER_LV_1_0)
		first_state = SER_LV_1_0_STATE_STOP_IDLE;

	return first_state;
}

static void ser_create_ser_cb(
	struct ser_ctrl_t *_ser_ctrl,
	struct ser_event_cb_t *event_cb)
{
	NDIS_SPIN_LOCK *lock = &_ser_ctrl->cb_lsit_lock;
	unsigned long flags;
	struct ser_cb_t *ser_cb = NULL;
	struct ser_cb_t *ser_cb_new = NULL;
	UINT8 found = 0;
	UINT8 ser_cb_num = 0;

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	DlListForEach(ser_cb, &_ser_ctrl->ser_cb_list, struct ser_cb_t, list) {
		if ((ser_cb->chip_id == event_cb->chip_id) &&
			(ser_cb->hw_id == event_cb->hw_id) &&
			(ser_cb->ser_level == event_cb->ser_level)) {
			found = 1;
			break;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);

	if (!found) {
		os_alloc_mem(NULL, (UINT8 **)&ser_cb_new, sizeof(struct ser_cb_t));
		if (!ser_cb_new) {
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				"fail to allocate ser_cb_new\n");
			return;
		}
		os_zero_mem(ser_cb_new, sizeof(struct ser_cb_t));
	}

	/* create the 1st ser_cb and add to ser_cb_list  */
	if (!found) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
			"create ser_cb(0x%x,%d,%d)\n",
			event_cb->chip_id, event_cb->hw_id, event_cb->ser_level);

		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
		ser_cb_new->chip_id = event_cb->chip_id;
		ser_cb_new->ser_level = event_cb->ser_level;
		ser_cb_new->ser_state = ser_get_first_state(event_cb->ser_level);
		ser_cb_new->ser_event = event_cb->ser_event;
		ser_cb_new->hw_id = event_cb->hw_id;
#ifdef MTK_FE_RESET_RECOVER
		ser_cb_new->need_reset = FALSE;
#endif
		DlListAddTail(&_ser_ctrl->ser_cb_list, &ser_cb_new->list);
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
	}

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	ser_cb_num = DlListLen(&_ser_ctrl->ser_cb_list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);

	if (found) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
			"ser_cb(0x%x,%d,%d) exited, no need to create\n",
			event_cb->chip_id, event_cb->hw_id, event_cb->ser_level);
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
		"ser_cb_num=%d\n", ser_cb_num);
}

static void ser_free_ser_cb(
	struct ser_ctrl_t *_ser_ctrl)
{
	NDIS_SPIN_LOCK *lock = &_ser_ctrl->cb_lsit_lock;
	unsigned long flags;
	struct ser_cb_t *ser_cb = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	while (!DlListEmpty(&_ser_ctrl->ser_cb_list)) {
		ser_cb = DlListFirst(&_ser_ctrl->ser_cb_list, struct ser_cb_t, list);
		DlListDel(&ser_cb->list);
		os_free_mem(ser_cb);
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
}

UINT8 ser_get_ser_state(
	struct ser_cb_t *ser_cb)
{
	UINT8 ser_state = 0;
	struct ser_ctrl_t *_ser_ctrl = ser_get_ser_ctrl();
	NDIS_SPIN_LOCK *lock = &_ser_ctrl->cb_lsit_lock;
	unsigned long flags;

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	ser_state = ser_cb->ser_state;
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);

	return ser_state;
}

void ser_tans_ser_state(
	struct ser_cb_t *ser_cb,
	UINT8 ser_state)
{
	struct ser_ctrl_t *_ser_ctrl = ser_get_ser_ctrl();
	NDIS_SPIN_LOCK *lock = &_ser_ctrl->cb_lsit_lock;
	unsigned long flags;

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	ser_cb->ser_state = ser_state;
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
}

void ser_ack_event(
	struct ser_cb_t *ser_cb,
	UINT32 ack_event)
{
	UINT32 chip_id = ser_cb->chip_id;
	UINT32 hw_id = ser_cb->hw_id;
	RTMP_ADAPTER *pAd_list[MAX_NUM_OF_INF] = {NULL};
	RTMP_ADAPTER *main_pAd;
	int pAd_num;
#ifdef MTK_FE_RESET_RECOVER
	struct _RTMP_ADAPTER *pAd2 = NULL;
#endif

	pAd_num = ser_get_all_pAd_by_chipid_hwid(pAd_list, chip_id, hw_id);
	main_pAd = pAd_list[0];

	if (pAd_num == 0 || main_pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"pAd_num is 0 or main_pAd is NULL\n");
		return;
	}

	if (ack_event > BIT(10)) {
#ifdef MTK_FE_RESET_RECOVER
		pAd2 = ser_get_fe_pAd_by_mainpAd(main_pAd);
		rtnl_lock();
		if (ack_event == FE_WIFI_RESET_DONE)
			call_netdevice_notifiers(MTK_WIFI_RESET_DONE, pAd2->net_dev);
		if (ack_event == FE_WIFI_TARFFIC_DONE)
			call_netdevice_notifiers(MTK_FE_STOP_TRAFFIC_DONE, pAd2->net_dev);
		if (ack_event == FE_WIFI_TARFFIC_DONE_FAIL)
			call_netdevice_notifiers(MTK_FE_STOP_TRAFFIC_DONE_FAIL, pAd2->net_dev);
		if (ack_event == FE_WIFI_WDMA_FAIL_RESET)
			call_netdevice_notifiers(MTK_FE_START_RESET_INIT, pAd2->net_dev);
		rtnl_unlock();
#endif
	} else {
		if (asic_ser_handler(main_pAd, SER_ACTION_INT_TO_MCU, ack_event))
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				"SER ack event = %x failed\n",
				ack_event);
	}
}

UINT8 ser_get_curr_state(
	struct _RTMP_ADAPTER *ad)
{
	struct ser_ctrl_t *_ser_ctrl = ser_get_ser_ctrl();
	NDIS_SPIN_LOCK *lock = &_ser_ctrl->cb_lsit_lock;
	unsigned long flags;
	struct ser_cb_t *ser_cb = NULL;

	UINT8 found = 0;
	UINT8 ser_state = 0;
	UINT32 hw_id = 0;

	/* get hw_id of this ad */
	if (ser_get_hwid_by_pAd(ad, &hw_id)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"hw_id not found");
		return SER_LV_1_0_STATE_STOP_IDLE;
	}

	/* get ser state from ser_cb */
	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	DlListForEach(ser_cb, &_ser_ctrl->ser_cb_list, struct ser_cb_t, list) {
		if (ser_cb->hw_id == hw_id) {
			found = 1;
			ser_state = ser_cb->ser_state;
			break;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);

	return found ? ser_state : SER_LV_1_0_STATE_STOP_IDLE;
}

UINT8 ser_is_idle_state(
	struct _RTMP_ADAPTER *ad)
{
	return ser_get_curr_state(ad) == SER_LV_1_0_STATE_STOP_IDLE ? 1 : 0;
}

static struct ser_cb_t *ser_get_ser_cb(
	struct ser_ctrl_t *_ser_ctrl,
	struct ser_event_cb_t *event_cb)
{
	NDIS_SPIN_LOCK *lock = &_ser_ctrl->cb_lsit_lock;
	unsigned long flags;
	struct ser_cb_t *ser_cb = NULL;
	UINT8 found = 0;
	UINT32 pre_event = 0;
	UINT32 cur_event = 0;

	UINT32 chip_id = 0;
	UINT32 hw_id = 0;
	UINT8 level = 0;
	UINT8 state = 0;

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	DlListForEach(ser_cb, &_ser_ctrl->ser_cb_list, struct ser_cb_t, list) {
		if ((ser_cb->chip_id == event_cb->chip_id) &&
			(ser_cb->hw_id == event_cb->hw_id) &&
			(ser_cb->ser_level == event_cb->ser_level)) {
			pre_event = ser_cb->ser_event;
			ser_cb->ser_event = event_cb->ser_event;
			cur_event = ser_cb->ser_event;

			chip_id = ser_cb->chip_id;
			hw_id = ser_cb->hw_id;
			level = ser_cb->ser_level;
			state = ser_cb->ser_state;

			found = 1;
			break;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);

	if (!found) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"failed to find ser_cb, error!\n");
		return NULL;
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
		"chip_id=0x%x,hw_id=%d,level=%s,state=%d,event=0x%x->0x%x\n",
		chip_id,
		hw_id,
		((level == SER_LV_1_0) ? "SER_LV_1_0" : ((level == SER_LV_0_5) ? "SER_LV_0_5" : ((level == SER_LV_10_0) ? "SER_LV_10_0" : "SER_LV_0_0"))),
		state,
		pre_event,
		cur_event);

	return ser_cb;
}

static UINT8 ser_invalid_event(
	UINT8 level,
	UINT32 event)
{
	UINT8 invalid_event = 0;

	if ((level == SER_LV_1_0) && !(HAS_SER_LV_1_0_EVT(event)))
		invalid_event = 1;

	return invalid_event;
}

UINT32 ser_timediff(
	UINT32 time1,
	UINT32 time2)
{
	UINT32 timeDiff = 0;

	if (time1 > time2)
		timeDiff = (0xFFFFFFFF - time1 + 1) + time2;
	else
		timeDiff = time2 - time1;

	return timeDiff;
}

UINT32 ser_get_time(
	void)
{
	ULONG timestamp;
	UINT32 timestamp_l;

	NdisGetSystemUpTime(&timestamp);
	timestamp = timestamp * (1000 / OS_HZ);
	timestamp_l = 0xFFFFFFFF & timestamp;

	return timestamp_l;
}

UINT8 _ser_dump_timelog(
	struct ser_cb_t *ser_cb)
{
	UINT32 idx = 0;
	UINT32 *pSerTimes = NULL;
	UINT32 curr_time = ser_get_time();

	if (ser_cb == NULL)
		return 1;

	pSerTimes = &ser_cb->ser_times[0];

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
		"Last SER to now(ms)=%u\n",
		ser_timediff(pSerTimes[SER_TIME_ID_T0],
		curr_time));
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
		"Current Time(ms)=%u\n", curr_time);

	for (idx = SER_TIME_ID_T0; idx < SER_TIME_ID_END; idx++) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
			"Time[%d](ms)=%u\n", idx,
			pSerTimes[idx]);
	}

	for (idx = SER_TIME_ID_T0; idx < (SER_TIME_ID_END - 1); idx++) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
			"T%d - T%d(ms)=%u\n",
			idx + 1, idx, ser_timediff(pSerTimes[idx],
			pSerTimes[idx + 1]));
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
		"Total Time(ms)=%u\n",
		ser_timediff(pSerTimes[SER_TIME_ID_T0],
		pSerTimes[SER_TIME_ID_T7]));

	return 0;
}

UINT8 ser_get_hwid_by_pAd(
	struct _RTMP_ADAPTER *ad,
	UINT32 *hw_id)
{
#ifdef HWIFI_SUPPORT
	struct os_cookie *handle = NULL;
	struct mtk_mac_dev *mac_dev = NULL;
	struct mtk_mac_hw *mac_hw = NULL;

	if (ad == NULL)
		goto error;

	handle = ad->OS_Cookie;
	mac_dev = handle->mac_dev;
	mac_hw = &mac_dev->hw;
	*hw_id = mac_hw->hw_id;
	return 0;
error:
#endif
	return 1;
}

UINT8 ser_get_chip_by_pAd(
	struct _RTMP_ADAPTER *ad,
	UINT32 *chip_id)
{
#ifdef HWIFI_SUPPORT
	struct os_cookie *handle = NULL;
	struct mtk_mac_dev *mac_dev = NULL;
	struct mtk_mac_hw *mac_hw = NULL;

	if (ad == NULL)
		goto error;

	handle = ad->OS_Cookie;
	mac_dev = handle->mac_dev;
	mac_hw = &mac_dev->hw;
	*chip_id = mac_hw->chip_id;
	return 0;
error:
#endif
	return 1;
}

int ser_get_all_pAd_by_chipid_hwid(
	RTMP_ADAPTER **ret_pAd_list,
	UINT32 chip_id,
	UINT32 hw_id)
{
	int pAd_num = 0;

	pAd_num = physical_device_get_mac_list_by_chip_hw_id(chip_id, hw_id, NULL);
	if (pAd_num > MAX_NUM_OF_INF) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"pAd_num(%d) > MAX_NUM_OF_INF(%d)\n", pAd_num, MAX_NUM_OF_INF);
		return 0;
	}

	physical_device_get_mac_list_by_chip_hw_id(chip_id, hw_id, ret_pAd_list);

	return pAd_num;
}

#ifdef MTK_FE_RESET_RECOVER
struct _RTMP_ADAPTER *
ser_get_fe_pAd_by_mainpAd(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_ADAPTER *fe_pAd = NULL;
	struct physical_device *device;
	P_ERR_RECOVERY_CTRL_T pErrRecoveryCtrl;
	struct mtk_notifier_block *mtk_nb;

	device = ad->physical_dev;
	pErrRecoveryCtrl = &device->ErrRecoveryCtl;
	mtk_nb = &pErrRecoveryCtrl->mtk_nb;
	fe_pAd = mtk_nb->priv;

	return fe_pAd;
}
#endif

void ser_dump_ser_stat(
	struct _RTMP_ADAPTER *ad,
	UINT8 dump_all)
{
	UINT32 chip_id = 0;
	UINT8 ser_tbl_num = ARRAY_SIZE(ser_tbl);
	UINT8 i = 0;
	ser_dump_ser_stat_t ser_dump_ser_stat = NULL;

	if (ser_get_chip_by_pAd(ad, &chip_id)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"hw_id not found\n");
		return;
	}

	for (i = 0; i < ser_tbl_num; i++) {
		if (chip_id == ser_tbl[i].chip_id) {
			ser_dump_ser_stat = ser_tbl[i].ser_dump_ser_stat;
			break;
		}
	}

	if (ser_dump_ser_stat) {
		ser_dump_ser_stat(ad, dump_all);
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"error\n");
}

void ser_dump_timelog(
	struct _RTMP_ADAPTER *ad,
	UINT8 ser_lv)
{
	struct ser_ctrl_t *_ser_ctrl = ser_get_ser_ctrl();
	NDIS_SPIN_LOCK *lock = &_ser_ctrl->cb_lsit_lock;
	unsigned long flags;

	struct ser_cb_t *ser_cb = NULL;
	struct ser_cb_t *_ser_cb = NULL;
	UINT8 found = 0;
	UINT32 chip_id = 0;
	UINT32 hw_id = 0;

	if (ser_get_chip_by_pAd(ad, &chip_id)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"chip_id not found\n");
		return;
	}

	if (ser_get_hwid_by_pAd(ad, &hw_id)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"hw_id not found\n");
		return;
	}

	DlListForEach(ser_cb, &_ser_ctrl->ser_cb_list, struct ser_cb_t, list) {
		if ((ser_cb->chip_id == chip_id) &&
			(ser_cb->hw_id == hw_id) &&
			(ser_cb->ser_level == ser_lv)) {
			_ser_cb = ser_cb;
			found = 1;
			break;
		}
	}

	if (found) {
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
		_ser_dump_timelog(_ser_cb);
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
		"ser_cb not found\n");
}

VOID ser_l1_timeout(PVOID SystemSpecific1, PVOID FunctionContext,
			PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)FunctionContext;
	struct ser_ctrl_t *_ser_ctrl = ser_get_ser_ctrl();
	struct ser_cb_t *ser_cb = NULL;
	UINT32 chip_id = 0;
	UINT32 hw_id = 0;
	UINT32 last_t0 = 0;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"ser stuck check\n");
	if (ser_is_idle_state(ad))
		return;

	if (ser_get_chip_by_pAd(ad, &chip_id)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
			"chip_id not found\n");
		return;
	}

	if (ser_get_hwid_by_pAd(ad, &hw_id)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
			"hw_id not found\n");
		return;
	}

	DlListForEach(ser_cb, &_ser_ctrl->ser_cb_list, struct ser_cb_t, list) {
		if ((ser_cb->chip_id == chip_id) &&
			(ser_cb->hw_id == hw_id) &&
			(ser_cb->ser_level == SER_LV_1_0)) {
			last_t0 =  ser_cb->ser_times[0];
		}
	}

	if (last_t0 != 0)
		ShowSerProc2(ad, "");
}

UINT32 ser_get_t0(struct _RTMP_ADAPTER *ad)
{
	struct ser_ctrl_t *_ser_ctrl = ser_get_ser_ctrl();

	struct ser_cb_t *ser_cb = NULL;
	UINT32 chip_id = 0;
	UINT32 hw_id = 0;

	if (ser_get_chip_by_pAd(ad, &chip_id)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"chip_id not found\n");
		return 0;
	}

	if (ser_get_hwid_by_pAd(ad, &hw_id)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"hw_id not found\n");
		return 0;
	}

	DlListForEach(ser_cb, &_ser_ctrl->ser_cb_list, struct ser_cb_t, list) {
		if ((ser_cb->chip_id == chip_id) &&
			(ser_cb->hw_id == hw_id) &&
			(ser_cb->ser_level == SER_LV_1_0)) {
			return ser_cb->ser_times[0];
		}
	}
	return 0;
}

#ifdef WF_RESET_SUPPORT
INT ser_mngr_free_event_cb(
	void)
{
	struct ser_ctrl_t *_ser_ctrl = ser_get_ser_ctrl();
	struct ser_event_cb_t *event_cb = NULL;

	while (!ser_event_q_empty(_ser_ctrl)) {
		event_cb = ser_event_deq(_ser_ctrl);
		if (event_cb == NULL)
			continue;

		os_free_mem(event_cb);
	}

	return 0;
}

static void ser_free_event_cb(
	struct ser_ctrl_t *_ser_ctrl)
{
	struct ser_event_cb_t *event_cb = NULL;

	while (!ser_event_q_empty(_ser_ctrl)) {
		event_cb = ser_event_deq(_ser_ctrl);
		if (event_cb == NULL)
			continue;

		os_free_mem(event_cb);
	}
}
#endif


static int ser_handler(
	struct ser_ctrl_t *_ser_ctrl,
	struct ser_cb_t *ser_cb)
{
	NDIS_SPIN_LOCK *lock = &_ser_ctrl->cb_lsit_lock;
	unsigned long flags;
	UINT32 chip_id = 0;
	UINT32 hw_id = 0;
	UINT8 level = 0;
	UINT8 state = 0;
	UINT32 event = 0;
	UINT8 ser_tbl_num = ARRAY_SIZE(ser_tbl);
	UINT8 i = 0;
	ser_handler_t _ser_handler = NULL;

	if (!ser_cb) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"ser_cb=NULL, error\n");
		return NDIS_STATUS_FAILURE;
	}

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	chip_id = ser_cb->chip_id;
	hw_id = ser_cb->hw_id;
	level = ser_cb->ser_level;
	state = ser_cb->ser_state;
	event = ser_cb->ser_event;
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);

	if (level >= SER_LV_END) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"invalid level=%u, error\n", level);
		return NDIS_STATUS_FAILURE;
	}

	if (ser_invalid_event(level, event)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"invalid event=%u, error\n", event);
		return NDIS_STATUS_FAILURE;
	}

	for (i = 0; i < ser_tbl_num; i++) {
		if (chip_id == ser_tbl[i].chip_id) {
			if (level == SER_LV_0_0)
				_ser_handler = ser_tbl[i].ser_0_0;
			if (level == SER_LV_0_5) {
#ifdef WF_RESET_SUPPORT
				struct ser_ctrl_t *_ser_ctrl = ser_get_ser_ctrl();

				ser_free_event_cb(_ser_ctrl);
#endif
				_ser_handler = ser_tbl[i].ser_0_5;
			}
			if (level == SER_LV_1_0)
				_ser_handler = ser_tbl[i].ser_1_0;

			if (level == SER_LV_10_0)
				_ser_handler = ser_tbl[i].ser_10_0;
#ifdef MTK_FE_RESET_RECOVER
			if (level == FE_RESET)
				_ser_handler = ser_tbl[i].fe_fe_reset;
			if (level == FE_WDMA_RESET)
				_ser_handler = ser_tbl[i].fe_fe_wdma_reset;
#endif
			break;
		}
	}

	if (_ser_handler == NULL) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"chip=0x%x,level=%d failed to find handler, error\n",
			chip_id, level);
		return NDIS_STATUS_FAILURE;
	}

	if (_ser_handler)
		_ser_handler(ser_cb);

	return NDIS_STATUS_SUCCESS;
}

static int ser_action(
	struct ser_ctrl_t *_ser_ctrl)
{
	struct ser_event_cb_t *event_cb = NULL;
	struct ser_cb_t *ser_cb = NULL;
	int ret = NDIS_STATUS_SUCCESS;

	while (!ser_event_q_empty(_ser_ctrl)) {
		event_cb = ser_event_deq(_ser_ctrl);
		if (event_cb == NULL)
			continue;

		ser_create_ser_cb(_ser_ctrl, event_cb);
		ser_cb = ser_get_ser_cb(_ser_ctrl, event_cb);
		ser_handler(_ser_ctrl, ser_cb);

		os_free_mem(event_cb);
	}

	return ret;
}


static INT ser_mngr_ctrl_task(
	ULONG context)
{
	int status = NDIS_STATUS_FAILURE;
	struct ser_ctrl_t *_ser_ctrl = NULL;
	RTMP_OS_TASK *task;

	_ser_ctrl = (struct ser_ctrl_t *)context;
	if (!_ser_ctrl)
		return status;

	task = &_ser_ctrl->ser_task;
	if (!task)
		return status;

	RtmpOSTaskCustomize(task);

	NdisAcquireSpinLock(&_ser_ctrl->ser_lock);
	_ser_ctrl->ser_func_state = RTMP_TASK_STAT_RUNNING;
	NdisReleaseSpinLock(&_ser_ctrl->ser_lock);

	while (task && !RTMP_OS_TASK_IS_KILLED(task)) {
		if (RtmpOSTaskWait(NULL, task, &status) == FALSE)
			break;
		ser_action(_ser_ctrl);
	}

	NdisAcquireSpinLock(&_ser_ctrl->ser_lock);
	_ser_ctrl->ser_func_state = RTMP_TASK_STAT_UNKNOWN;
	NdisReleaseSpinLock(&_ser_ctrl->ser_lock);

	status = RtmpOSTaskNotifyToExit(task);

	return status;
}

INT ser_mngr_init(
	void)
{
	struct ser_ctrl_t *_ser_ctrl = ser_get_ser_ctrl();
	NDIS_STATUS status;
	RTMP_OS_TASK *task = NULL;

	MTWF_PRINT("%s()", __func__);

	os_zero_mem(_ser_ctrl, sizeof(struct ser_ctrl_t));

	NdisAllocateSpinLock(NULL, &_ser_ctrl->ser_lock);
	NdisAllocateSpinLock(NULL, &_ser_ctrl->event_q_lock);
	NdisAllocateSpinLock(NULL, &_ser_ctrl->cb_lsit_lock);

	DlListInit(&_ser_ctrl->event_q);
	DlListInit(&_ser_ctrl->ser_cb_list);

	_ser_ctrl->event_times = 0;
	NdisAcquireSpinLock(&_ser_ctrl->ser_lock);
	_ser_ctrl->ser_func_state = RTMP_TASK_STAT_INITED;
	NdisReleaseSpinLock(&_ser_ctrl->ser_lock);
	task = &_ser_ctrl->ser_task;
	//RTMP_OS_TASK_INIT(task, "ser_mngr", NULL);
	RtmpOSTaskInit(task, "ser_mngr", NULL, NULL, NULL);
	status = RtmpOSTaskAttach(task, ser_mngr_ctrl_task, (ULONG)_ser_ctrl);

	if (status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"unable to start ser_mngr\n");
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

INT ser_mngr_exit(
	void)
{
	struct ser_ctrl_t *_ser_ctrl = ser_get_ser_ctrl();
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_WARN,
		"cb_num=%d (before)\n", DlListLen(&_ser_ctrl->ser_cb_list));
	ser_free_ser_cb(_ser_ctrl);
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_WARN,
		"cb_num=%d (after)\n", DlListLen(&_ser_ctrl->ser_cb_list));

	/*kill task*/
	status = RtmpOSTaskKill(&_ser_ctrl->ser_task);
	NdisFreeSpinLock(&_ser_ctrl->ser_lock);
	NdisFreeSpinLock(&_ser_ctrl->event_q_lock);
	NdisFreeSpinLock(&_ser_ctrl->cb_lsit_lock);

	return status;
}
#endif /* ERR_RECOVERY */
