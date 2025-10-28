/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	net_adaption_jedi.c
*/
#include "rt_config.h"
#include "net_adaption.h"

#define TEST_MODE_FR_TABLE_INDEX 	17

extern s_int32 net_ad_open_inf(struct wifi_dev *wdev);
extern s_int32 net_ad_close_inf(struct wifi_dev *wdev);
extern s_int32 net_ad_enqueue_mlme_pkt(
	RTMP_ADAPTER *ad,
	void *pkt,
	struct wifi_dev *wdev,
	u_char q_idx,
	boolean is_data_queue);
extern s_int32 net_ad_tx_pkt_handle(
	RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	struct _TX_BLK *tx_blk);
extern s_int32 net_ad_tx(
	RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	struct _TX_BLK *tx_blk);

static struct test_thread_cb g_test_thread[MAX_SERV_THREAD_NUM];
struct test_ant_map test_ant_to_spe_idx_map[] = {
	/* All */
	{0x0, 0},
	{0xF, 0},
	/* 1 Ant */
	{0x1, 0},	/* Tx0 */
	{0x2, 1},	/* Tx1 */
	{0x4, 3},	/* Tx2 */
	{0x8, 9},	/* Tx3 */
	/* 2 Ant */
	{0x3, 0},
	{0x5, 2},
	{0x9, 8},
	{0x6, 4},
	{0xA, 6},
	{0xC, 16},
	/* 3 Ant */
	{0x7, 0},	/* 0_1_2 */
	{0xB, 10},	/* 0_1_3 */
	{0xD, 12},	/* 0_2_3 */
	{0xE, 18},	/* 1_2_3 */
};

/*****************************************************************************
 *	Internal functions
 *****************************************************************************/
s_int32 net_ad_concurrent_status(struct test_wlan_info *winfos, u_int32 in_op_mode, u_int32 *is_concurrent)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct _RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	*is_concurrent = 0;
	if ((in_op_mode & TESTMODE_GET_PARAM(ad, TESTMODE_BAND0, op_mode)) &&
			(in_op_mode & TESTMODE_GET_PARAM(ad, TESTMODE_BAND1, op_mode)))
			*is_concurrent = 1;

	return ret;
}

static s_int32 net_ad_mps_check_stat(
	struct test_wlan_info *winfos,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_mps_cb *mps_cb;
	struct test_mps_setting *mps_setting;
	u_int32 tx_cnt, txed_cnt;

	mps_cb = &configs->mps_cb;
	mps_setting = mps_cb->mps_setting;
	if (!mps_setting || !mps_cb->mps_cnt) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"mps_cb/mps_setting NULL %p/%p\n",
			mps_cb, mps_setting);

		ret = SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;
		return ret;
	}

	tx_cnt = configs->tx_stat.tx_cnt;
	txed_cnt = configs->tx_stat.txed_cnt;

	SERV_OS_SEM_LOCK(&mps_cb->lock);
	if ((mps_cb->stat & TEST_MPS_ITEM_RUNNING)
		&& (txed_cnt >= tx_cnt)) {
		mps_cb->stat = 0;
		SERV_OS_SEM_UNLOCK(&mps_cb->lock);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"mps_idx finished idx=%d, mps_cnt=%d\n",
			mps_cb->ref_idx, mps_cb->mps_cnt);
		if (mps_cb->ref_idx > mps_cb->mps_cnt) {
			configs->op_mode &= ~fTEST_MPS;
			mps_cb->setting_inuse = FALSE;
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"mps all finished idx=%d, mps_cnt=%d\n",
				mps_cb->ref_idx, mps_cb->mps_cnt);

			ret = net_ad_mps_tx_operation(winfos, configs, FALSE);
		}
	} else {
		SERV_OS_SEM_UNLOCK(&mps_cb->lock);
	}

	return ret;
}

static s_int32 net_ad_mps_dump_setting(
	struct test_configuration *configs,
	u_int16 idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_mps_cb *mps_cb;
	struct test_mps_setting *mps_setting;

	mps_cb = &configs->mps_cb;
	mps_setting = mps_cb->mps_setting;
	if (!mps_setting || !mps_cb->mps_cnt) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"mps_cb/mps_setting NULL %p/%p\n",
			mps_cb, mps_setting);

		ret = SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;
		return ret;
	}

	if (idx == 0xFFFF) {
		for (idx = 1; idx <= mps_cb->mps_cnt; idx++) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"item_idx=%d, phy_mode=%d, ",
				idx, mps_setting[idx].tx_mode);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"tx_ant=0x%x, mcs=%d, ",
				mps_setting[idx].tx_ant,
				mps_setting[idx].mcs);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"pkt_len=%d, pkt_cnt=%d, ",
				mps_setting[idx].pkt_len,
				mps_setting[idx].pkt_cnt);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"pwr=%d, nss=%d, pkt_bw=%d\n",
				mps_setting[idx].pwr,
				mps_setting[idx].nss,
				mps_setting[idx].pkt_bw);

		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"item_idx=%d, phy_mode=%d, ",
			idx, mps_setting[idx].tx_mode);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"tx_ant=0x%x, mcs=%d, ",
			mps_setting[idx].tx_ant,
			mps_setting[idx].mcs);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"pkt_len=%d, pkt_cnt=%d, ",
			mps_setting[idx].pkt_len,
			mps_setting[idx].pkt_cnt);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"pwr=%d, nss=%d, pkt_bw=%d\n",
			mps_setting[idx].pwr,
			mps_setting[idx].nss,
			mps_setting[idx].pkt_bw);
	}

	return ret;
}

static s_int32 net_ad_mps_load_setting(
	struct test_wlan_info *winfos,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_mps_cb *mps_cb = NULL;
	struct test_mps_setting *mps_setting = NULL;
	u_char *test_pkt;
	u_int32 idx, tx_mode, tx_ant, mcs, pwr;
	struct service_test *serv_test = NULL;
	RTMP_ADAPTER *ad = NULL;
	u_int32 pkt_len, pkt_cnt, nss, pkt_bw;
	struct test_tx_stack *stack = NULL;

	/* TODO: factor out here for tx power */

	test_pkt = configs->test_pkt;
	mps_cb = &configs->mps_cb;
	stack = &configs->stack;
	if (mps_cb != NULL)
		mps_setting = mps_cb->mps_setting;
	if (!mps_cb || !mps_setting)
		goto err0;

	SERV_OS_SEM_LOCK(&mps_cb->lock);

	if (mps_cb->stat & TEST_MPS_ITEM_RUNNING)
		goto err1;

	mps_cb->stat |= TEST_MPS_ITEM_RUNNING;
	idx = mps_cb->ref_idx;

	if (idx > mps_cb->mps_cnt)
		goto err2;

	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	serv_test = (struct service_test *)ad->serv.serv_handle;

	tx_mode = mps_setting[idx].tx_mode;
	tx_ant = mps_setting[idx].tx_ant;
	mcs = mps_setting[idx].mcs;
	pwr = mps_setting[idx].pwr;
	pkt_len = mps_setting[idx].pkt_len;
	pkt_cnt = mps_setting[idx].pkt_cnt;
	nss = mps_setting[idx].nss;
	pkt_bw = mps_setting[idx].pkt_bw;
	configs->pwr_param.power = mps_setting[idx].pwr;
	configs->tx_mode = tx_mode;
	configs->tx_ant = tx_ant;
	configs->mcs = mcs;
	configs->nss = nss;
	configs->per_pkt_bw = pkt_bw;
	configs->tx_len = pkt_len;
	configs->tx_stat.tx_cnt = pkt_cnt;
	configs->tx_stat.tx_done_cnt = 0;
	configs->tx_stat.txed_cnt = 0;
	if (tx_mode == 8 && pkt_bw > 0)
		configs->ldpc = 1;
	else
		configs->ldpc = 0;

	/* TODO: factor out here for tx power */

	SERV_OS_SEM_UNLOCK(&mps_cb->lock);

	/* TODO: factor out here for tx power */

	/* Here means no need to fill packet first time */
	if (mps_cb->ref_idx != 1) {

		ret = mt_serv_tx_power_operation(serv_test,
			SERV_TEST_TXPWR_SET_PWR);
		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"set tx power fail\n");
		}
	}

	ret = net_ad_mps_dump_setting(configs, mps_cb->ref_idx);
	mps_cb->ref_idx++;

	return ret;

err2:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"mps_cb->ref_idx=%d, mps_cnt=%d\n",
		mps_cb->ref_idx, mps_cb->mps_cnt);
err1:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"item=%d is running\n",
		mps_cb->ref_idx - 1);
	SERV_OS_SEM_UNLOCK(&mps_cb->lock);
	return ret;
err0:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"mps_cb/mps_setting NULL %p/%p\n",
		mps_cb, mps_setting);
	return ret;
}

static VOID net_ad_thread_set_service(
	struct test_wlan_info *winfos,
	u_char band_idx, u_int8 *stat)
{
	u_int8 mask = 0;

	if (band_idx <= TEST_BAND2)
		mask = 1 << band_idx;
	else
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"fail, band_idx = 0x%x\n",
			band_idx);

	*stat |= mask;
}

VOID net_ad_thread_proceed_tx(
	struct test_wlan_info *winfos, u_char band_idx)
{
	u_char thread_idx = winfos->thread_idx;

	OS_SEM_LOCK(&g_test_thread[thread_idx].lock);
	net_ad_thread_set_service(winfos,
				  band_idx,
				  &g_test_thread[thread_idx].service_stat);
	OS_SEM_UNLOCK(&g_test_thread[thread_idx].lock);
	sys_ad_wakeup_os_task(&g_test_thread[thread_idx].task);
}

VOID net_ad_thread_stop_tx(struct test_wlan_info *winfos)
{
	return;
}

static s_int32 net_ad_thread_handler(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	PQUEUE_HEADER mgmt_swq = NULL, mgmt_post_swq = NULL;
	struct test_configuration *test_config;
	struct test_tx_stack *stack = NULL;
	struct ipg_param *ipg_param;
	struct tx_time_param *tx_time_param = NULL;
	struct tx_mpdu_info *mpdu_info = NULL;
	struct wifi_dev *wdev;
	struct _RTMP_CHIP_CAP *chip_cap = NULL;
	u_int32 op_mode, txed_cnt = 0, tx_cnt = 0, pkt_tx_time, ipg;
	s_int32 dequeue_size, multi_users = 0;
	u_short q_idx;
	u_int8 need_ampdu;
	u_char hwq_idx;
	u_int32 mgmt_swq_lmt = 0, is_concurrent_tx = 0;
	u_int32 band0_gen = 0, band1_gen = 0;
	u_int32 ple_limit_pkt_quota = 0, mgmt_swq_num = 0;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	if (PD_GET_QM_PARM(ad->physical_dev)) {
		struct fp_qm *qm_parm = (struct fp_qm *)PD_GET_QM_PARM(ad->physical_dev);

		mgmt_swq_lmt = qm_parm->max_mgmt_que_num;
	} else
		mgmt_swq_lmt = MGMT_QUE_MAX_NUMS;

	if (winfos->chip_cap.swq_per_band) {
		mgmt_swq = PD_GET_QM_MGMT_SWQ(ad->physical_dev, band_idx);
		mgmt_post_swq = PD_GET_QM_MGMT_POST_SWQ(ad->physical_dev, band_idx);
	}
	else {
		mgmt_swq = PD_GET_QM_MGMT_SWQ(ad->physical_dev, 0);
		mgmt_post_swq = PD_GET_QM_MGMT_POST_SWQ(ad->physical_dev, 0);
	}
	test_config = configs;
	stack = &test_config->stack;
	op_mode = test_config->op_mode;
	q_idx = test_config->ac_idx;
	txed_cnt = test_config->tx_stat.txed_cnt;
	tx_cnt = test_config->tx_stat.tx_cnt;
	ipg_param = &test_config->ipg_param;
	tx_time_param = &test_config->tx_time_param;
	mpdu_info = &test_config->stack.tx_info[stack->q_idx].mpdu_info;
	pkt_tx_time = tx_time_param->pkt_tx_time;
	need_ampdu = mpdu_info->need_ampdu;
	ipg = ipg_param->ipg;
	wdev = (struct wifi_dev *)test_config->stack.virtual_device[0];
	hwq_idx = hif_get_resource_idx(ad->hdev_ctrl, wdev, TX_MGMT, q_idx);
	dequeue_size = g_test_thread[winfos->thread_idx].deq_cnt;

	chip_cap = hc_get_chip_cap(ad->hdev_ctrl);
	/* limit pkt quota per band in the ple */
	ple_limit_pkt_quota = chip_cap->hif_group_page_size/3;

	do {
		u_long free_num;

test_thread_dequeue:
		free_num = hif_get_tx_resource_free_num(ad->hdev_ctrl, hwq_idx);

		if (op_mode & OP_MODE_STOP)
			break;

		if (!(op_mode & OP_MODE_TXFRAME))
			break;

		if (!free_num)
			break;

		net_ad_concurrent_status(winfos, OP_MODE_TXFRAME, &is_concurrent_tx);

round_tx:
		mgmt_swq_num = mgmt_swq->Number + mgmt_post_swq->Number;

		if (((pkt_tx_time > 0) || (ipg > 0)) &&
			(mgmt_swq_num >= mgmt_swq_lmt))
			break;

		/* For service thread tx packet counter control */
		if (tx_cnt <= txed_cnt)
			break;

		if ((pkt_tx_time > 0) && need_ampdu)
			q_idx = WMM_AC_BE;
		else
			q_idx = WMM_AC_BK;

		ret = net_ad_enq_pkt(winfos,
				     q_idx,
				     stack->virtual_wtbl[stack->q_idx],
				     stack->virtual_device[stack->q_idx],
				     stack->pkt_skb[stack->q_idx]);
		if (ret)
			break;

		txed_cnt++;
		stack->q_idx++;

		if (stack->q_idx == stack->index)
			stack->q_idx = 0;

		if (((pkt_tx_time > 0) && need_ampdu) || (ipg > 0)) {
			if (!(IS_BELLWETHER(ad))) {
				PKT_TOKEN_CB *cb = NULL;
				struct token_tx_pkt_queue *que = NULL;
				u_int32 free_token_cnt, pkt_tx_token_id_max;

				cb = hc_get_ct_cb(winfos->hdev_ctrl);
				if (winfos->chip_cap.swq_per_band)
					que = token_tx_get_queue_by_band(cb, band_idx);
				else
					que = token_tx_get_queue_by_band(cb, 0);
				free_token_cnt =
					token_tx_get_free_cnt(que);
				pkt_tx_token_id_max = que->pkt_tkid_cnt;
				free_num = hif_get_tx_resource_free_num(ad->hdev_ctrl,
									hwq_idx);

				/* DBDC mode always runs this part for dbdc tx duty stable */
				if (is_concurrent_tx || IS_TEST_DBDC(winfos)) {
#ifdef WHNAT_SUPPORT
					if (chip_cap->qm == FAST_PATH_QM &&
						IS_ASIC_CAP(ad, fASIC_CAP_WHNAT) &&
						PD_GET_WHNAT_ENABLE(ad->physical_dev) &&
							(IS_MT7986(ad) || IS_MT7916(ad)
							|| IS_MT7981(ad) || IS_BELLWETHER(ad))) {

						ple_limit_pkt_quota = pkt_tx_token_id_max/3;
						mgmt_swq_num = mgmt_swq->Number + mgmt_post_swq->Number;

						if (band_idx == TESTMODE_BAND0) {
							if ((que->total_enq_cnt_per_band[TESTMODE_BAND0] < ple_limit_pkt_quota)
								&& (mgmt_swq_num <= mgmt_swq_lmt - 1) && ((mgmt_swq_num +
									que->total_enq_cnt_per_band[TESTMODE_BAND0]) <
										(ple_limit_pkt_quota + 150))) {
									band0_gen++;
									goto round_tx;
							}
						}
						else if (band_idx == TESTMODE_BAND1) {
							if ((que->total_enq_cnt_per_band[TESTMODE_BAND1] < (ple_limit_pkt_quota * 2)
								&& (mgmt_swq_num <= mgmt_swq_lmt - 1) && ((mgmt_swq_num +
									que->total_enq_cnt_per_band[TESTMODE_BAND1]) <
										(ple_limit_pkt_quota * 2 + 150)))) {
									band1_gen++;
									goto round_tx;
							}
						}
					}
					else
#endif
					{
						if (band_idx == TESTMODE_BAND0) {
							/* ple group page size 1/3 for band0 packet cnt */
							if (((pkt_tx_token_id_max - free_token_cnt) < ple_limit_pkt_quota)
								&& (free_num > 0)
								&& (mgmt_swq->Number < mgmt_swq_lmt)) {
									band0_gen++;
									goto round_tx;
							}
						}
						else if (band_idx == TESTMODE_BAND1) {
							/* ple group page size 2/3 for band1 packet cnt */
							if (((pkt_tx_token_id_max - free_token_cnt) < (ple_limit_pkt_quota << 1))
								&& (free_num > 0)
								&& (mgmt_swq->Number < mgmt_swq_lmt)) {
									band1_gen++;
									goto round_tx;
							}
						}
					}
				} else {
					if ((free_token_cnt
						> (pkt_tx_token_id_max - TEST_ENQ_PKT_NUM))
						&& (free_num > 0)
						&& (mgmt_swq->Number < mgmt_swq_lmt)) {
							(band_idx == 0)? band0_gen++: band1_gen++;
							goto round_tx;
					}
				}
			} else {
				if (mgmt_swq->Number < mgmt_swq_lmt)
					goto round_tx;
			}
		}

		dequeue_size--;
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
			"band_idx=%u, tx_cnt=%u, txed_cnt=%u, ",
			band_idx, tx_cnt, txed_cnt);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
			"dequeue=%d, multi_user=%d, free=%lu\n",
			dequeue_size, multi_users, free_num);

		if (!dequeue_size) {
			multi_users--;
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
				"dequeue %d finish, multi_user=%d\n",
				dequeue_size, multi_users);
		} else
			goto test_thread_dequeue;
	} while (multi_users > 0);

	test_config->tx_stat.txed_cnt = txed_cnt;
	test_config->tx_stat.tx_cnt = tx_cnt;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
		"band_idx=%d, tx_cnt=%u, txed_cnt=%u, dequeue=%d\n",
		band_idx, tx_cnt, txed_cnt, dequeue_size);

	return ret;
}

static s_int32 net_ad_thread_get_band_idx(
	struct test_wlan_info *winfos,
	u_int8 *stat)
{
	u_int8 mask = 0;

	mask = 1 << TEST_BAND0;

	if (*stat & mask) {
		*stat &= ~mask;
		return TEST_BAND0;
	}

	mask = 1 << TEST_BAND1;

	if (*stat & mask) {
		*stat &= ~mask;
		return TEST_BAND1;
	}

	mask = 1 << TEST_BAND2;

	if (*stat & mask) {
		*stat &= ~mask;
		return TEST_BAND2;
	}

	return -1;
}

static s_int32 net_ad_thread(u_long context)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct serv_os_task *task = (SERV_OS_TASK *) context;
	struct test_wlan_info *winfos = NULL;
	struct test_configuration *configs = NULL;
	struct test_configuration *MPSconfigs = NULL;
	RTMP_ADAPTER *ad = NULL;
	u_char thread_idx = 0;
	s_int32 status;
	s_int32 band_idx = 0;
	u_char service_stat = 0;

	if (!task)
		goto err;

	winfos = (struct test_wlan_info *)SERV_OS_TASK_GET_WINFOS(task);
	configs = (struct test_configuration *)SERV_OS_TASK_GET_CONFIGS(task);
	thread_idx = winfos->thread_idx;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		goto err;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
		"init thread_idx %u for band %d\n",
		thread_idx, band_idx);
	SERV_OS_COMPLETE(&g_test_thread[thread_idx].cmd_done);

	while (!SERV_OS_TASK_IS_KILLED(task)) {
		if (sys_ad_wait_os_task(NULL, task, &status) == FALSE) {
			RTMP_SET_FLAG(ad, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		SERV_OS_SEM_LOCK(&g_test_thread[thread_idx].lock);
		service_stat = g_test_thread[thread_idx].service_stat;

		do {
			if (!service_stat)
				break;

			band_idx = net_ad_thread_get_band_idx(winfos,
								&service_stat);

			if (band_idx == -1)
				break;

			net_ad_thread_handler(
					winfos, configs, (u_char)band_idx);
		} while (1);

		g_test_thread[thread_idx].service_stat = service_stat;
		SERV_OS_SEM_UNLOCK(&g_test_thread[thread_idx].lock);

		if (band_idx == -1)
			goto err;

		MPSconfigs = configs;
		if (MPSconfigs->op_mode & fTEST_MPS) {

			ret = net_ad_mps_check_stat(winfos, MPSconfigs);
			if (ret)
				break;

			ret = net_ad_mps_load_setting(winfos, MPSconfigs);
			if (ret)
				break;
		}

		schedule();

	}

err:
	if (ad) {
		RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_HALT_IN_PROGRESS);
	}

	g_test_thread[thread_idx].is_init = FALSE;

	if (ret)
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			 "\x1b[41m abnormal leave err=0x%08x\x1b[m\n",
			  ret);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"leave\n");

	return ret;
}

static s_int32 net_ad_init_payload(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char *packet, u_int32 len)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_int32 policy, pl_len, pos;
	u_char *payload;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	policy = configs->fixed_payload;
	payload = configs->payload;
	pl_len = configs->pl_len;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"len=%d, pl_len=%u, policy=%x\n",
		len, pl_len, policy);

	if (policy == TEST_RANDOM_PAYLOAD) {
		for (pos = 0; pos < len; pos++)
			packet[pos] = RandomByte(ad);

		return ret;
	}

	if (!payload)
		return SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"payload=%x\n", payload[0]);

	if (pl_len == 0) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"payload length can't be 0!!\n");

		return SERV_STATUS_OSAL_NET_INVALID_LEN;
	}

	if (policy == TEST_USER_PAYLOAD) {
		sys_ad_zero_mem(packet, len);
		sys_ad_move_mem(packet, payload, pl_len);
	} else if (policy == TEST_FIXED_PAYLOAD) {
		for (pos = 0; pos < len; pos += pl_len)
			sys_ad_move_mem(&packet[pos], payload, pl_len);
	}

	return ret;
}

static s_int32 net_ad_fill_tmac_info(
	RTMP_ADAPTER *ad,
	TMAC_INFO *tmac_info,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct service_test *serv_test;
	struct test_configuration *configs;
	struct tx_time_param *tx_time_param = NULL;
	struct tx_mpdu_info *mpdu_info = NULL;
	struct wifi_dev *wdev = NULL;
	u_char *addr1 = NULL;
	u_char tx_mode, mcs, vht_nss, wmm_idx;
	u_int32 ant_sel, pkt_tx_time;
	u_int8 need_qos, need_amsdu, need_ampdu;
	boolean fgspe;

	/* Note: shall not use ad here */
	serv_test = (struct service_test *)ad->serv.serv_handle;
	configs = &serv_test->test_config;
	wdev = &ad->ate_wdev;
	tx_time_param = &configs->tx_time_param;
	pkt_tx_time = tx_time_param->pkt_tx_time;
	mpdu_info = &configs->stack.tx_info[0].mpdu_info;
	need_qos = mpdu_info->need_qos;
	need_amsdu = mpdu_info->need_amsdu;
	need_ampdu = mpdu_info->need_ampdu;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"cannot get wdev\n");

		return SERV_STATUS_OSAL_NET_INVALID_PARAM;
	}

	configs->hdr_len = LENGTH_802_11;
	addr1 = configs->addr1[0];
	ant_sel = configs->tx_ant;
	tx_mode = configs->tx_mode;
	mcs = configs->mcs;
	vht_nss = configs->nss;
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"addr1=%02x:%02x:%02x:%02x:%02x:%02x\n",
		SERV_PRINT_MAC(addr1));

	/* Fill TMAC_INFO */
	sys_ad_zero_mem(tmac_info, sizeof(*tmac_info));
	tmac_info->LongFmt = TRUE;

	if (pkt_tx_time > 0) {
		tmac_info->WifiHdrLen = (u_int8)mpdu_info->hdr_len;
		tmac_info->PktLen = (u_int16)mpdu_info->msdu_len;
		tmac_info->NeedTrans = FALSE;

		if (need_qos | need_amsdu | need_ampdu) {
			tmac_info->HdrPad = 2;
			tmac_info->BmcPkt = FALSE;
			tmac_info->UserPriority = 0;
		} else {
			tmac_info->HdrPad = 0;
			tmac_info->BmcPkt = IS_BM_MAC_ADDR(addr1);
			tmac_info->UserPriority = 0;
		}
	} else {
		tmac_info->WifiHdrLen = (u_int8)configs->hdr_len;
		tmac_info->HdrPad = 0;
		tmac_info->PktLen = (u_int16)configs->tx_len;
		tmac_info->BmcPkt = IS_BM_MAC_ADDR(addr1);
	}

	/* no ack */
	if ((pkt_tx_time > 0) && (need_ampdu))
		tmac_info->bAckRequired = 1;
	else
		tmac_info->bAckRequired = 0;

	tmac_info->FrmType = FC_TYPE_DATA;
	tmac_info->SubType = SUBTYPE_QDATA;
	tmac_info->OwnMacIdx = wdev->OmacIdx;

	/* no frag */
	tmac_info->FragIdx = 0;

	/* no protection */
	tmac_info->CipherAlg = 0;

	/* tx path setting */
	tmac_info->VhtNss = vht_nss ? vht_nss : 1;
	tmac_info->AntPri = 0;
	tmac_info->SpeEn = 0;

	/* timing measure setting */
	if ((ad->pTmrCtrlStruct != NULL)
		&& (ad->pTmrCtrlStruct->TmrEnable == TMR_INITIATOR))
		tmac_info->TimingMeasure = 1;

	/* band_idx for tx ring choose */
	tmac_info->band_idx = band_idx;

	switch (ant_sel) {
	case 0: /* both */
		tmac_info->AntPri = 0;
		tmac_info->SpeEn = 1;
		break;

	case 1: /* tx0 */
		tmac_info->AntPri = 0;
		tmac_info->SpeEn = 0;
		break;

	case 2: /* tx1 */
		tmac_info->AntPri = 2; /* b'010 */
		tmac_info->SpeEn = 0;
		break;
	}


	/* Need to modify the way of getting wmm_idx */
	wmm_idx = configs->wmm_idx;
	tmac_info->WmmSet = wmm_idx;

	if (ant_sel & TEST_ANT_USER_SEL) {
		ant_sel &= ~TEST_ANT_USER_SEL;
		tmac_info->AntPri = ant_sel;
	} else {
		s_int32 map_idx;

		for (map_idx = 0;
			map_idx < SERV_ARRAY_SIZE(test_ant_to_spe_idx_map);
			map_idx++) {
			if (ant_sel ==
				test_ant_to_spe_idx_map[map_idx].ant_sel)
				break;
		}

		if (map_idx == SERV_ARRAY_SIZE(test_ant_to_spe_idx_map))
			tmac_info->AntPri = 0;
		else
			tmac_info->AntPri
				= test_ant_to_spe_idx_map[map_idx].spe_idx;
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"ant_sel=%x, ant_pri=%x, vht_nss=%x, TxD.VhtNss=%x\n",
		ant_sel, tmac_info->AntPri,
		vht_nss, tmac_info->VhtNss);

	/* Fill transmit setting */
	tmac_info->TxRadioSet.RateCode = mcs;
	tmac_info->TxRadioSet.PhyMode = tx_mode;
	tmac_info->TxRadioSet.CurrentPerPktBW = configs->per_pkt_bw;
	tmac_info->TxRadioSet.ShortGI =	configs->sgi;
	tmac_info->TxRadioSet.Stbc = configs->stbc;
	tmac_info->TxRadioSet.Ldpc = configs->ldpc;

	tmac_info->QueIdx =
		asic_get_hwq_from_ac(ad, tmac_info->WmmSet, configs->ac_idx);

	if ((pkt_tx_time > 0) && (need_ampdu)) {
		tmac_info->Wcid = configs->wcid_ref;
		tmac_info->FixRate = 0;
		tmac_info->BaDisable = FALSE;
		tmac_info->RemainTxCnt = 1;
	} else {
		tmac_info->Wcid = 0;
		tmac_info->FixRate = 1;
		tmac_info->BaDisable = TRUE;
		tmac_info->RemainTxCnt = 15;
	}

	if (configs->txs_enable) {
		tmac_info->TxS2Host = TRUE;
		tmac_info->TxS2Mcu = FALSE;
		tmac_info->TxSFmt = 1;
	}

	if (tx_mode == TEST_MODE_CCK) {
		tmac_info->TxRadioSet.Premable = LONG_PREAMBLE;

		if (mcs == MCS_9) {
			tmac_info->TxRadioSet.RateCode = 0;
			tmac_info->TxRadioSet.Premable = SHORT_PREAMBLE;
		} else if (mcs == MCS_10) {
			tmac_info->TxRadioSet.RateCode = 1;
			tmac_info->TxRadioSet.Premable = SHORT_PREAMBLE;
		} else if (mcs == MCS_11) {
			tmac_info->TxRadioSet.RateCode = 2;
			tmac_info->TxRadioSet.Premable = SHORT_PREAMBLE;
		}
	}

	tmac_info->Wcid = configs->wcid_ref;
	if (tmac_info->AntPri >= 24)
		fgspe = TRUE;
	else
		fgspe = FALSE;

	if ((pkt_tx_time > 0) && (need_ampdu)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"tmac_info->Wcid/Wmmset/QueIdx=%d/%d/%d\n",
			tmac_info->Wcid,
			tmac_info->WmmSet, tmac_info->QueIdx);
	}

	return ret;
}

static s_int32 net_ad_fill_non_offload_tx_blk(
	RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	void *tx_blk)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	void *packet;
	TX_BLK *txblk = (TX_BLK *)tx_blk;

	packet = txblk->pPacket;
	txblk->Wcid = RTMP_GET_PACKET_WCID(packet);
	RTMP_QueryPacketInfo(
		packet, txblk->pSrcBufHeader, txblk->SrcBufLen);

	TX_BLK_SET_FLAG(txblk, fTX_CT_WithTxD);

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(packet))
		TX_BLK_SET_FLAG(txblk, fTX_bClearEAPFrame);

	if (IS_ASIC_CAP(ad, fASIC_CAP_TX_HDR_TRANS)) {
		if ((txblk->TxFrameType == TX_LEGACY_FRAME)
			|| (txblk->TxFrameType == TX_AMSDU_FRAME)
			|| (txblk->TxFrameType == TX_BMC_FRAME))
			TX_BLK_SET_FLAG(txblk, fTX_HDR_TRANS);
	}

	txblk->pSrcBufData = txblk->pSrcBufHeader;

	return ret;
}

static boolean net_ad_fill_offload_tx_blk(
	RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	void *tx_blk,
	boolean retry)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	PNDIS_PACKET pPacket;
	TX_BLK *txblk = (TX_BLK *)tx_blk;

	pPacket = txblk->pPacket;
	txblk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	RTMP_QueryPacketInfo(pPacket,
				 txblk->pSrcBufHeader,
				 txblk->SrcBufLen);
	txblk->pSrcBufHeader += cap->TXWISize;
	/* Due to testmode allocate size include TXWISize */
	txblk->SrcBufLen -= cap->TXWISize;

	TX_BLK_SET_FLAG(txblk, fTX_CT_WithTxD);

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pPacket))
		TX_BLK_SET_FLAG(txblk, fTX_bClearEAPFrame);

	/* testmode data does not support fTX_HDR_TRANS yet
	if (IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) {
		if ((txblk->TxFrameType == TX_LEGACY_FRAME)
			|| (txblk->TxFrameType == TX_AMSDU_FRAME)
			|| (txblk->TxFrameType == TX_BMC_FRAME))
			TX_BLK_SET_FLAG(tx_blk, fTX_HDR_TRANS);
	}
	*/

	txblk->pSrcBufData = txblk->pSrcBufHeader;
	txblk->wmm_set = HcGetWmmIdx(ad, wdev);

	if (retry)
		TX_BLK_SET_FLAG(txblk, fTX_bRetryUnlimit);
	else
		TX_BLK_SET_FLAG(txblk, fTX_bNoRetry);

	txblk->UserPriority = 0;

	/*	no frag */
	txblk->FragIdx = 0;
	/* no protection */
	SET_CIPHER_NONE(txblk->CipherAlg);
	return TRUE;
}

/*****************************************************************************
 *	Extern functions
 *****************************************************************************/
struct service_test *net_ad_wrap_service(void *adapter)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)adapter;
	struct service *serv = &ad->serv;

	return (struct service_test *)serv->serv_handle;
}

struct wifi_dev_ops serv_wdev_ops = {
	.open = net_ad_open_inf,
	.close = net_ad_close_inf,
	.send_mlme_pkt = net_ad_enqueue_mlme_pkt,
	.tx_pkt_handle = net_ad_tx_pkt_handle,
	.ate_tx = net_ad_tx,
	.disconn_act = wifi_sys_disconn_act
};

s_int32 net_ad_open_inf(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;


	if (wifi_sys_open(wdev) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"open fail!!!\n");
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"inf up for ra_%x(func_idx) OmacIdx=%d\n",
		wdev->func_idx, wdev->OmacIdx);

	MlmeRadioOn(ad, wdev);

	wdev->bAllowBeaconing = FALSE;


	return TRUE;
}

s_int32 net_ad_close_inf(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;


	if (ad == NULL)
		return FALSE;

	if (wifi_sys_close(wdev) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"close fail!!!\n");
		return FALSE;
	}


	return TRUE;
}

s_int32 net_ad_enqueue_mlme_pkt(
	RTMP_ADAPTER *ad,
	void *pkt,
	struct wifi_dev *wdev,
	u_char q_idx,
	boolean is_data_queue)
{
	s_int32 ret;
	struct qm_ops *qm_ops = PD_GET_QM_OPS(ad->physical_dev);

	RTMP_SET_PACKET_MGMT_PKT(pkt, 1);

	ret = qm_ops->enq_mgmtq_pkt(ad, wdev, pkt);

	return ret;
}

s_int32 net_ad_tx_pkt_handle(
	RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	struct _TX_BLK *tx_blk)
{
	struct wifi_dev_ops *ops = NULL;
	s_int32 ret = NDIS_STATUS_SUCCESS;

	if (!wdev) {
		RELEASE_NDIS_PACKET(ad, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	ops = wdev->wdev_ops;
	ret = ops->ate_tx(ad, wdev, tx_blk);

	return ret;
}

s_int32 net_ad_tx(
	RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	struct _TX_BLK *tx_blk)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	TMAC_INFO tmac_info;
	PQUEUE_ENTRY q_entry;
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);
	u_int32 band_idx = HcGetBandByWdev(wdev);

	q_entry = RemoveHeadQueue(&tx_blk->TxPacketList);
	tx_blk->pPacket = QUEUE_ENTRY_TO_PACKET(q_entry);
	RTMP_SET_PACKET_WCID(tx_blk->pPacket, 0);

	/* Fill tx blk for test mode */
	ret = net_ad_fill_non_offload_tx_blk(ad, wdev, tx_blk);
	if (ret != SERV_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"fill tx blk fail, err:0x%08x\n", ret);
	}

	/* TMAC_INFO setup for test mode */
	ret = net_ad_fill_tmac_info(ad, &tmac_info, band_idx);
	if (ret)
		return ret;

	return arch_ops->ate_hw_tx(ad, &tmac_info, tx_blk);
}

s_int32 net_ad_tx_v2(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	struct _TX_BLK *tx_blk)
{
	u_int8 band_idx = 0, need_amsdu = 0, stack_idx = 0;
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);
	struct service_test *serv_test = NULL;
	struct test_configuration *configs = NULL;
	struct tx_mpdu_info *mpdu_info = NULL;

	serv_test = (struct service_test *)ad->serv.serv_handle;

	band_idx = HcGetBandByWdev(wdev);
	configs = &serv_test->test_config;

#ifdef TESTMODE_V1
	ret = mt_engine_search_stack(configs,
				   RTMP_GET_PACKET_WCID(tx_blk->pPacket),
				   &stack_idx,
				   (void **)&tx_blk->pMacEntry);
	if (ret != SERV_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"wcid:%d is in-valid in stack!\n"
			RTMP_GET_PACKET_WCID(tx_blk->pPacket));

		return SERV_STATUS_OSAL_NET_FAIL;
	}
#endif
	mpdu_info = &configs->stack.tx_info[stack_idx].mpdu_info;
	need_amsdu = mpdu_info->need_amsdu;

	if (mpdu_info->need_qos) {
		tx_blk->wifi_hdr_len = (UINT8) mpdu_info->hdr_len;
		tx_blk->MpduHeaderLen = (UINT8) mpdu_info->hdr_len;
	} else {
		tx_blk->wifi_hdr_len = (UINT8) LENGTH_802_11;
		tx_blk->MpduHeaderLen = (UINT8) LENGTH_802_11;
	}

	if (mpdu_info->need_ampdu) {
		TX_BLK_CLEAR_FLAG(tx_blk, fTX_ForceRate);
		TX_BLK_SET_FLAG(tx_blk, fTX_bAckRequired);
		TX_BLK_SET_FLAG(tx_blk, fTX_bAteAgg);
		tx_blk->HdrPadLen = 2;
	} else {
		TX_BLK_SET_FLAG(tx_blk, fTX_ForceRate);
		TX_BLK_CLEAR_FLAG(tx_blk, fTX_bAckRequired);
		tx_blk->HdrPadLen = 0;
	}

	/* Fill TX blk for ATE mode */
	if ((configs->tx_mode == TEST_MODE_HE_MU
		|| configs->tx_mode == TEST_MODE_VHT_MIMO)
		&& configs->retry)
		ret = net_ad_fill_offload_tx_blk(ad, wdev, tx_blk, TRUE);
	else
		ret = net_ad_fill_offload_tx_blk(ad, wdev, tx_blk, FALSE);

	tx_blk->QueIdx = configs->ac_idx;

#if defined(CONFIG_AP_SUPPORT)
	if (tx_blk->pMacEntry)
		tx_blk->pMbss = MBSS_GET(tx_blk->pMacEntry->pMbss);
#endif
	if (configs->txs_enable)
		TX_BLK_SET_FLAG(tx_blk, fTX_bAteTxsRequired);

	return arch_ops->hw_tx(ad, tx_blk);
}

s_int32 net_ad_tx_v3(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	struct _TX_BLK *tx_blk)
{
	u_int8 band_idx = 0, need_amsdu = 0, stack_idx = 0;
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);
	struct service_test *serv_test = NULL;
	struct test_configuration *configs = NULL;
	struct tx_mpdu_info *mpdu_info = NULL;
	struct serv_hdr_802_11 *phdr = NULL;

	serv_test = (struct service_test *)ad->serv.serv_handle;

	band_idx = HcGetBandByWdev(wdev);
	configs = &serv_test->test_config;

#ifdef TESTMODE_V1
	ret = mt_engine_search_stack(configs,
				   RTMP_GET_PACKET_WCID(tx_blk->pPacket),
				   &stack_idx,
				   (void **)&tx_blk->pMacEntry);
	if (ret != SERV_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"wcid:%d is in-valid in stack!\n",
			RTMP_GET_PACKET_WCID(tx_blk->pPacket));

		return SERV_STATUS_OSAL_NET_FAIL;
	}
#endif
	mpdu_info = &configs->stack.tx_info[stack_idx].mpdu_info;
	need_amsdu = mpdu_info->need_amsdu;

	phdr = (struct serv_hdr_802_11 *)configs->template_frame;

	tx_blk->phy_info = &tx_blk->pMacEntry->phy_param;

	//testmode frTable (fixed rate table)
	tx_blk->fr_tbl_idx = TEST_MODE_FR_TABLE_INDEX;
	tx_blk->fr_bw = tx_blk->phy_info->bw;

	if (mpdu_info->need_qos) {
		tx_blk->wifi_hdr_len = (uint8_t) mpdu_info->hdr_len;
		tx_blk->MpduHeaderLen = (uint8_t) mpdu_info->hdr_len;
		tx_blk->dot11_type = FC_TYPE_DATA;
		tx_blk->dot11_subtype = SUBTYPE_QDATA;
	} else {
		tx_blk->wifi_hdr_len = (uint8_t) LENGTH_802_11;
		tx_blk->MpduHeaderLen = (uint8_t) LENGTH_802_11;
		tx_blk->dot11_type = phdr->fc.type;
		tx_blk->dot11_subtype = phdr->fc.sub_type;
	}

	if (mpdu_info->need_ampdu) {
		TX_BLK_CLEAR_FLAG(tx_blk, fTX_ForceRate);
		TX_BLK_SET_FLAG(tx_blk, fTX_bAckRequired);
		TX_BLK_SET_FLAG(tx_blk, fTX_bAteAgg);
		tx_blk->HdrPadLen = 2;
	} else {
		TX_BLK_SET_FLAG(tx_blk, fTX_ForceRate);
		TX_BLK_CLEAR_FLAG(tx_blk, fTX_bAckRequired);
		tx_blk->HdrPadLen = 0;
	}

	/* Fill TX blk for ATE mode */
	if ((configs->tx_mode == TEST_MODE_HE_MU
		|| configs->tx_mode == TEST_MODE_VHT_MIMO
		|| configs->tx_mode == TEST_MODE_EHT_MU_DL_OFDMA)
		&& configs->retry)
		ret = net_ad_fill_offload_tx_blk(ad, wdev, tx_blk, TRUE);
	else
		ret = net_ad_fill_offload_tx_blk(ad, wdev, tx_blk, FALSE);

	tx_blk->QueIdx = configs->ac_idx;

#if defined(CONFIG_AP_SUPPORT)
	if (tx_blk->pMacEntry)
		tx_blk->pMbss = MBSS_GET(tx_blk->pMacEntry->pMbss);
#endif
	if (configs->txs_enable)
		TX_BLK_SET_FLAG(tx_blk, fTX_bAteTxsRequired);

	return arch_ops->hw_tx(ad, tx_blk);
}

s_int32 net_ad_init_thread(
	struct test_wlan_info *winfos,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_char thread_idx = 0;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	for (thread_idx = 0; thread_idx < MAX_SERV_THREAD_NUM; thread_idx++) {
		if (!g_test_thread[thread_idx].is_init)
			break;
		}
	if (thread_idx == MAX_SERV_THREAD_NUM) {

		ret = SERV_STATUS_OSAL_SYS_FAIL;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"tx thread num upto maxnum!!\n");
		goto err;
	}
		winfos->thread_idx = thread_idx;

	/* Init test_thread_cb */
		g_test_thread[thread_idx].deq_cnt = 1;
		g_test_thread[thread_idx].cmd_expire = RTMPMsecsToJiffies(3000);
		SERV_OS_INIT_COMPLETION(&g_test_thread[thread_idx].cmd_done);

		if (!g_test_thread[thread_idx].is_init) {
			ret = sys_ad_init_os_task(
					&g_test_thread[thread_idx].task,
					"serv_thread_tx",
					(VOID *)winfos,
					(VOID *)configs);
			if (ret)
				goto err;

			NdisAllocateSpinLock(
				ad, &g_test_thread[thread_idx].lock);

			ret = sys_ad_attach_os_task(
				&g_test_thread[thread_idx].task,
				net_ad_thread,
				(ULONG)&g_test_thread[thread_idx].task);

			if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT
				(&g_test_thread[thread_idx].cmd_done,
				g_test_thread[thread_idx].cmd_expire))
				goto err;

			if (ret)
				goto err;

			g_test_thread[thread_idx].is_init = TRUE;
		}

		SERV_OS_SEM_LOCK(&g_test_thread[thread_idx].lock);
		g_test_thread[thread_idx].service_stat = 0;
		SERV_OS_SEM_UNLOCK(&g_test_thread[thread_idx].lock);


	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"initialize thread_idx=%d\n", thread_idx);

	return ret;

err:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"tx thread init fail err=0x%08x\n", ret);

	return ret;
}

s_int32 net_ad_release_thread(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char thread_idx = winfos->thread_idx;

	ret = sys_ad_kill_os_task(&g_test_thread[thread_idx].task);

	if (ret != SERV_STATUS_SUCCESS)
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			 "kill test mode tx task failed!\n");
	else
		g_test_thread[thread_idx].is_init = FALSE;

	NdisFreeSpinLock(&g_test_thread[thread_idx].lock);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		 "release thread_idx=%d\n", thread_idx);

	return ret;
}

s_int32 net_ad_backup_cr(
	struct test_wlan_info *winfos,
	struct test_bk_cr *test_bkcr,
	u_long offset, enum test_bk_cr_type type)
{
	struct test_bk_cr *entry = NULL;
	RTMP_ADAPTER *ad = NULL;
	u_int32 entry_idx;

	if ((type >= SERV_TEST_BKCR_TYPE_NUM)
		|| (type == SERV_TEST_EMPTY_BKCR))
		return SERV_STATUS_OSAL_NET_FAIL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	for (entry_idx = 0; entry_idx < TEST_MAX_BKCR_NUM; entry_idx++) {
		struct test_bk_cr *tmp = &test_bkcr[entry_idx];

		if ((tmp->type == SERV_TEST_EMPTY_BKCR) && (entry == NULL)) {
			entry = tmp;
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"find empty bk entry %d\n",
				entry_idx);
		} else if ((tmp->type == type) && (tmp->offset == offset)) {
			entry = tmp;
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"update bk entry %d\n",
				entry_idx);
			break;
		}
	}

	if (!entry)
		return SERV_STATUS_OSAL_NET_FAIL;

	entry->type = type;
	entry->offset = offset;

	switch (type) {
	case SERV_TEST_MAC_BKCR:
		MAC_IO_READ32(ad->hdev_ctrl, offset, &entry->val);
		break;

	case SERV_TEST_HIF_BKCR:
		HIF_IO_READ32(ad->hdev_ctrl, offset, &entry->val);
		break;

	case SERV_TEST_PHY_BKCR:
		PHY_IO_READ32(ad->hdev_ctrl, offset, &entry->val);
		break;

	case SERV_TEST_HW_BKCR:
		HW_IO_READ32(ad->hdev_ctrl, offset, &entry->val);
		break;

	case SERV_TEST_MCU_BKCR:
		MCU_IO_READ32(ad->hdev_ctrl, offset, &entry->val);
		break;

	default:
		break;
	}

	return SERV_STATUS_SUCCESS;
}

s_int32 net_ad_restore_cr(
	struct test_wlan_info *winfos,
	struct test_bk_cr *test_bkcr,
	u_long offset)
{
	struct test_bk_cr *entry = NULL;
	RTMP_ADAPTER *ad = NULL;
	u_int32 entry_idx;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	for (entry_idx = 0; entry_idx < TEST_MAX_BKCR_NUM; entry_idx++) {
		struct test_bk_cr *tmp = &test_bkcr[entry_idx];

		if (tmp->offset == offset) {
			entry = tmp;
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"find entry %d\n", entry_idx);
			break;
		}
	}

	if (!entry)
		return SERV_STATUS_OSAL_NET_FAIL;

	switch (entry->type) {
	case SERV_TEST_MAC_BKCR:
		MAC_IO_WRITE32(ad->hdev_ctrl, offset, entry->val);
		break;

	case SERV_TEST_HIF_BKCR:
		HIF_IO_WRITE32(ad->hdev_ctrl, offset, entry->val);
		break;

	case SERV_TEST_PHY_BKCR:
		PHY_IO_WRITE32(ad->hdev_ctrl, offset, entry->val);
		break;

	case SERV_TEST_HW_BKCR:
		HW_IO_WRITE32(ad->hdev_ctrl, offset, entry->val);
		break;

	case SERV_TEST_MCU_BKCR:
		MCU_IO_WRITE32(ad->hdev_ctrl, offset, entry->val);
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_WARN,
			"bk-type not supported\n");
		entry->type = SERV_TEST_EMPTY_BKCR;
		entry->offset = 0;
		break;
	}

	entry->type = SERV_TEST_EMPTY_BKCR;
	entry->offset = 0;
	entry->val = 0;

	return SERV_STATUS_SUCCESS;
}

s_int32 net_ad_cfg_queue(
	struct test_wlan_info *winfos, boolean enable)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
#ifdef CONFIG_AP_SUPPORT
	s_int32 bss_id, max_num_bss;
#endif /* CONFIG_AP_SUPPROT */

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

#ifdef CONFIG_AP_SUPPORT
	max_num_bss = ad->ApCfg.BssidNum;
#endif

	if (enable) {
		/* Start to deq sw queue */
		RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

		/* Start tcp/ip layer queue */
		RTMP_OS_NETDEV_START_QUEUE(ad->net_dev);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(ad) {
			if (max_num_bss > MAX_MBSSID_NUM(ad))
				max_num_bss = MAX_MBSSID_NUM(ad);

			for (bss_id = FIRST_MBSSID;
				bss_id < MAX_MBSSID_NUM(ad); bss_id++) {
				if (ad->ApCfg.MBSSID[bss_id].wdev.if_dev)
					RTMP_OS_NETDEV_START_QUEUE(
					ad->ApCfg.MBSSID[bss_id].wdev.if_dev);
			}
		}
#endif /* CONFIG_AP_SUPPROT */
	} else {
		/* Stop tcp/ip layer queue */
		RTMP_OS_NETDEV_STOP_QUEUE(ad->net_dev);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(ad) {
			if (max_num_bss > MAX_MBSSID_NUM(ad))
				max_num_bss = MAX_MBSSID_NUM(ad);

			for (bss_id = FIRST_MBSSID;
				bss_id < MAX_MBSSID_NUM(ad); bss_id++) {
				if (ad->ApCfg.MBSSID[bss_id].wdev.if_dev)
					RTMP_OS_NETDEV_STOP_QUEUE(
					ad->ApCfg.MBSSID[bss_id].wdev.if_dev);
			}
		}
#endif /* CONFIG_AP_SUPPROT */
		/* Stop to deq sw queue */
		RTMP_SET_FLAG(ad, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	}

	return ret;
}

s_int32 net_ad_startup_ap(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *mbss = NULL;
#endif /* CONFIG_AP_SUPPROT */

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

#ifdef CONFIG_AP_SUPPORT
	mbss = &ad->ApCfg.MBSSID[MAIN_MBSSID];
#endif /* CONFIG_AP_SUPPROT */

	ret = NICInitializeAdapter(ad);
	if (ret != NDIS_STATUS_SUCCESS) {
		return SERV_STATUS_OSAL_NET_FAIL;
	}

	RTMPSetTimer(&ad->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);

#ifdef CONFIG_AP_SUPPORT
	APStartUp(ad, mbss, AP_BSS_OPER_ALL);
#endif /* CONFIG_AP_SUPPROT */

	/* Start tx path queues */
	ret = net_ad_cfg_queue(winfos, TRUE);

	return ret;
}

s_int32 net_ad_stop_ap(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	boolean cancelled;
#ifdef CONFIG_AP_SUPPORT
	s_int32 bss_id, max_num_bss;
	BSS_STRUCT *mbss = NULL;
#endif

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

#ifdef CONFIG_AP_SUPPORT
	max_num_bss = ad->ApCfg.BssidNum;
	mbss = &ad->ApCfg.MBSSID[MAIN_MBSSID];
#endif

	/* Stop tx path queues */
	ret = net_ad_cfg_queue(winfos, FALSE);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(ad) {
		if (max_num_bss > MAX_MBSSID_NUM(ad))
			max_num_bss = MAX_MBSSID_NUM(ad);

		/* First IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (bss_id = FIRST_MBSSID;
			bss_id < MAX_MBSSID_NUM(ad); bss_id++) {
			if (ad->ApCfg.MBSSID[bss_id].wdev.if_dev) {
				ad->ApCfg.MBSSID[bss_id].wdev.protection = 0;
			}
		}

#ifdef CONFIG_6G_SUPPORT
		bss_id = MAIN_MBSSID;
		for (bss_id = MAIN_MBSSID;
			bss_id < MAX_MBSSID_NUM(ad); bss_id++) {
			struct wifi_dev *wdev = NULL;

			if (ad->ApCfg.MBSSID[bss_id].wdev.if_dev) {
				wdev = &ad->ApCfg.MBSSID[bss_id].wdev;
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"\033[1;33m wdev=%d, bandidx:%d\033[0m\n",
					IS_BSSID_11V_NON_TRANS(&ad->ApCfg.MBSSID[bss_id].mbss_11v),
					HcGetBandByWdev(wdev));

				in_band_discovery_update_oper(wdev,
					UNSOLICIT_TX_DISABLE,
					0, UNSOLICIT_TXMODE_NON_HT);
			}
		}
#endif /* CONFIG_6G_SUPPORT */
	}
#endif
#ifdef CONFIG_AP_SUPPORT
	APStop(ad, mbss, AP_BSS_OPER_ALL);
#endif /* CONFIG_AP_SUPPORT */
	RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	RTMPCancelTimer(&ad->Mlme.PeriodicTimer, &cancelled);
	RTMP_SET_FLAG(ad, fRTMP_ADAPTER_SYSEM_READY);

	return ret;
}

s_int32 net_ad_enter_normal(
	struct test_wlan_info *winfos,
	struct test_backup_params *bak)
{
	RTMP_ADAPTER *ad = NULL;
	u_int8 band_seq = 0;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	ad->CommonCfg.bEnableTxBurst = bak->en_tx_burst;
/* FIXME: bcn_prd */
	for (band_seq = 0; band_seq < TEST_BAND_NUM; band_seq++)
		ad->CommonCfg.BeaconPeriod = bak->bcn_prd[band_seq];
	/* restore no beacon status */
	ad->BcnCheckInfo.nobcncnt = 0;

	if (bak->premable)
		OPSTATUS_SET_FLAG(ad, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	else
		OPSTATUS_CLEAR_FLAG(ad, fOP_STATUS_SHORT_PREAMBLE_INUSED);

#if defined(GREENAP_SUPPORT)
	greenap_set_capability(ad, bak->greenap);
#endif
	/* resotre stream path */
	sys_ad_move_mem(&ad->Antenna, &bak->antenna, sizeof(ad->Antenna));

	net_ad_startup_ap(winfos);
	ad->CommonCfg.bBssCoexEnable = bak->en_bss_coex;

	return SERV_STATUS_SUCCESS;
}

s_int32 net_ad_exit_normal(
	struct test_wlan_info *winfos,
	struct test_backup_params *bak)
{
	RTMP_ADAPTER *ad = NULL;
	u_int8 i;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	net_ad_stop_ap(winfos);

	bak->en_tx_burst = ad->CommonCfg.bEnableTxBurst;
	ad->CommonCfg.bEnableTxBurst = FALSE;
	bak->en_bss_coex = ad->CommonCfg.bBssCoexEnable;
	/* To prevent BSS scan occupy execution time */
	ad->CommonCfg.bBssCoexEnable = FALSE;
/* FIXME bcn_prd*/
	for (i = 0; i < TEST_BAND_NUM; i++)
		bak->bcn_prd[i] = ad->CommonCfg.BeaconPeriod;
	/* To disable TBTT interrupt */
	ad->CommonCfg.BeaconPeriod = 0;

#if defined(GREENAP_SUPPORT)
	bak->greenap = greenap_get_capability(ad);
	greenap_set_capability(ad, FALSE);
#endif
	/* backup stream path */
	sys_ad_move_mem(&bak->antenna, &ad->Antenna, sizeof(ad->Antenna));

	if (OPSTATUS_TEST_FLAG(ad, fOP_STATUS_SHORT_PREAMBLE_INUSED))
		bak->premable = TRUE;
	else
		bak->premable = FALSE;

	return SERV_STATUS_SUCCESS;
}

s_int32 net_ad_update_wdev(
	u_int8 band_idx,
	struct test_wlan_info *winfos,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct wifi_dev *wdev = NULL;
#if defined(DOT11_HE_AX)
	struct wifi_dev *wdev_txc = NULL;
#endif
	RTMP_ADAPTER *ad = NULL;
	struct serv_chip_cap *cap = NULL;
	u_int8 ch_band, tx_stream_num = 0, ant_loop = 0, max_path;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	/* To update wdev setting according to ch_band */
#if defined(DOT11_HE_AX)
	wdev_txc = &ad->ate_wdev;

	if (!wdev_txc)
		goto err;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"wdev_idx(txd)=%d, ch=%d\n",
		wdev_txc->wdev_idx, wdev_txc->channel);

#endif /* DOT11_HE_AX */
	wdev = &ad->ate_wdev;

	if (!wdev)
		goto err;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"wdev_idx=%d, ch=%d\n",
		wdev->wdev_idx, wdev->channel);

	cap = &winfos->chip_cap;
	/* TX stream number required to be most true bit of tx_ant bimap,
	   due to Base band initial H/W only consider this parameter as total stream number.
	   For nss=1 WF1/2/3 TX, it is required to initialize as 2 streams/3 streams/4 streams.

	   But it is not adaptive that initial as maximum stream number because of iBF matrix
	   apply or not depend on the real stream number configured.
	 */
	max_path = max(GET_MAX_PATH(MCS_NSS_CAP(ad), band_idx, 0),
					GET_MAX_PATH(MCS_NSS_CAP(ad), band_idx, 1));
	for (ant_loop = 0 ; ant_loop < max_path ; ant_loop++) {
		if (configs->tx_ant & BIT(ant_loop))
			tx_stream_num = ant_loop+1;
	}


	ad->Antenna.field.RxPath = configs->rx_ant;
	ad->Antenna.field.TxPath = tx_stream_num;

	ch_band = configs->ch_band;
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"ch_band=%d\n", ch_band);

	if (ch_band == 0)
		wdev->PhyMode = TEST_WMODE_CAP_24G;
	else if (ch_band == 1)
		wdev->PhyMode = TEST_WMODE_CAP_5G;
	else
		wdev->PhyMode = TEST_WMODE_CAP_6G;
#if defined(DOT11_HE_AX)
	if (ch_band == 0)
		wdev_txc->PhyMode = TEST_WMODE_CAP_24G;
	else if (ch_band == 1)
		wdev_txc->PhyMode = TEST_WMODE_CAP_5G;
	else
		wdev_txc->PhyMode = TEST_WMODE_CAP_6G;
#endif
	/*
	 * QA mode used central ch,
	 * thus wdev ch set as qa mode's control ch of relating bw setting
	 */
	wdev->channel = configs->ctrl_ch;
	wlan_config_set_ch_band(wdev, wdev->PhyMode);
	wlan_config_set_tx_stream(wdev, configs->tx_ant);
	wlan_config_set_rx_stream(wdev, configs->rx_ant);
	wlan_config_set_ht_bw(wdev,
		((configs->bw > TEST_BW_20) ? HT_BW_40 : HT_BW_20));
	wlan_config_set_ext_cha(wdev, configs->ch_offset);
	wlan_config_set_cen_ch_2(wdev, configs->channel_2nd);

	//convert QA tool BW to VHT BW
	if (configs->bw < TEST_BW_5)
		// BW20 ~ BW 80
		wlan_config_set_vht_bw(wdev,
			((configs->bw > TEST_BW_40) ?
			(VHT_BW_80+(configs->bw-BW_80)) : VHT_BW_2040));
	else if (configs->bw < TEST_BW_160NC)
		//BW160 , BW80+80
		wlan_config_set_vht_bw(wdev,
				(VHT_BW_80+(configs->bw-TEST_BW_5)));
	else
		//BW320
		wlan_config_set_vht_bw(wdev,
				(VHT_BW_320+(configs->bw-TEST_BW_320)));

#if defined(DOT11_HE_AX)
	if (configs->per_pkt_bw > TEST_BW_80)
		wlan_config_set_ap_bw(wdev, VHT_BW_160);
	else
		wlan_config_set_ap_bw(wdev, configs->per_pkt_bw);
	wlan_config_set_ap_cen(wdev, configs->channel-configs->ch_offset);
#endif /* DOT11_HE_AX */

	if (wdev_attr_update(ad, wdev) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"error to update wdev\n");
		goto err;
	}

	if (wdev_edca_acquire(ad, wdev) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"error to acquire edca\n");
		goto err;
	}

#if defined(DOT11_HE_AX)
	wdev_txc->channel = configs->ctrl_ch;

	wlan_config_set_ch_band(wdev_txc, wdev_txc->PhyMode);
	wlan_config_set_tx_stream(wdev_txc, configs->tx_ant);
	wlan_config_set_rx_stream(wdev_txc, configs->rx_ant);
	wlan_config_set_ht_bw(wdev_txc,
		((configs->bw > TEST_BW_20) ? HT_BW_40 : HT_BW_20));
	wlan_config_set_ext_cha(wdev_txc, configs->ch_offset);
	wlan_config_set_cen_ch_2(wdev_txc, configs->channel_2nd);
	if (configs->bw > TEST_BW_5)
		wlan_config_set_vht_bw(wdev_txc,
			(VHT_BW_80+(configs->bw-TEST_BW_5)));
	else
		wlan_config_set_vht_bw(wdev_txc,
			((configs->bw > TEST_BW_40) ?
			(VHT_BW_80+(configs->bw-BW_80)) : VHT_BW_2040));
	if (configs->per_pkt_bw > TEST_BW_80)
		wlan_config_set_ap_bw(wdev_txc, VHT_BW_160);
	else
		wlan_config_set_ap_bw(wdev_txc, configs->per_pkt_bw);
	wlan_config_set_ap_cen(wdev_txc, configs->channel-configs->ch_offset);

	if (wdev_attr_update(ad, wdev_txc) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"error to update wdev\n");
		goto err;
	}

	if (wdev_edca_acquire(ad, wdev_txc) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"error to acquire edca\n");
		goto err;
	}
#endif

	return ret;

err:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"updats wdev failed!\n");

	return SERV_STATUS_OSAL_NET_FAIL_UPDATE_WDEV;
}

s_int32 net_ad_init_wdev(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct wifi_dev *wdev = NULL;
	RTMP_ADAPTER *ad = NULL;
	u_char *own_mac_addr = NULL, *bssid = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	/* To init wdev */
	wdev = &ad->ate_wdev;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"wdev is null\n");
		return SERV_STATUS_OSAL_NET_INVALID_PARAM;
	}

	if (wdev_init(ad, wdev, WDEV_TYPE_SERVICE_TXD,
			ad->wdev_list[0]->if_dev,
			band_idx, NULL, (void *)ad) != TRUE)
		goto err;

	if (IS_MT7915(ad) || IS_MT7986(ad) || IS_MT7916(ad) ||
		IS_MT7981(ad))
		serv_wdev_ops.ate_tx = net_ad_tx_v2;

	if (IS_BELLWETHER(ad))
		serv_wdev_ops.ate_tx = net_ad_tx_v3;

	if (wdev_ops_register(wdev, WDEV_TYPE_SERVICE_TXD, &serv_wdev_ops, 0)
		!= TRUE)
		goto err;

	configs->wdev_idx = wdev->wdev_idx;
	wdev->channel = configs->channel;
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"wdev_idx=%d, channel=%d\n",
		wdev->wdev_idx, wdev->channel);

	if (wdev->channel > 14)
		wdev->PhyMode = TEST_WMODE_CAP_5G;
	else
		wdev->PhyMode = TEST_WMODE_CAP_24G;
	wlan_config_set_ch_band(wdev, wdev->PhyMode);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(ad)
		own_mac_addr = (u_char *)&configs->addr3[0];

#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(ad)
		own_mac_addr = (u_char *)&configs->addr2[0];
#endif
	if (own_mac_addr != NULL)
		sys_ad_move_mem(wdev->if_addr, own_mac_addr, SERV_MAC_ADDR_LEN);
	else
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"own_mac_addr is NULL\n");

	if (wdev_do_open(wdev) != TRUE)
		goto err;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(ad)
		bssid = (u_char *)&configs->addr2[0];

#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(ad)
		bssid = (u_char *)&configs->addr1[0];
#endif
	if (bssid != NULL)
		sys_ad_move_mem(wdev->bssid, bssid, SERV_MAC_ADDR_LEN);
	else
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"bssid is NULL\n");

	if (wifi_sys_linkup(wdev, NULL) != TRUE)
		goto err;

	return ret;

err:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"inits wdev failed!\n");

	return SERV_STATUS_OSAL_NET_FAIL_INIT_WDEV;
}

s_int32 net_ad_release_wdev(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct wifi_dev *wdev = NULL;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"\n");

	/* To release wdev */
	wdev = &ad->ate_wdev;

	if (wifi_sys_linkdown(wdev) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"linkdown failed!\n");
		goto err;
	}

	if (wdev_do_close(wdev) != TRUE)
		goto err;

	if (wdev_deinit(ad, wdev) != TRUE)
		goto err;

	return ret;

err:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"releases wdev failed!\n");

	return SERV_STATUS_OSAL_NET_FAIL_RELEASE_WDEV;
}

s_int32 net_ad_alloc_wtbl(
	struct test_wlan_info *winfos,
	u_char *da,
	void *virtual_device,
	void **virtual_wtbl,
	struct test_tx_info *tx_info)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct wifi_dev *wdev = (struct wifi_dev *)virtual_device;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"invalid adapter!\n");
		ret = SERV_STATUS_OSAL_NET_INVALID_PAD;
		goto err_out;
	}

	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"invalid wdev!\n");
		ret = SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;
		goto err_out;
	}

	mac_entry_lookup(ad, da, wdev, (MAC_TABLE_ENTRY **)virtual_wtbl);
	if (*virtual_wtbl) {
		entry = (struct _MAC_TABLE_ENTRY *)*virtual_wtbl;
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			 "[Reused]\n");
	} else {
		*virtual_wtbl = (void *)MacTableInsertEntry(ad,
							    da,
							    wdev,
							    ENTRY_ATE,
							    OPMODE_ATE,
							    TRUE,
							    MLD_STA_NONE,
							    NULL);
		entry = (struct _MAC_TABLE_ENTRY *)*virtual_wtbl;
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			 "[Create]\n");
	}

	if (*virtual_wtbl == NULL) {
		ret = SERV_STATUS_OSAL_NET_FAIL;
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"err: *virtual_wtbl == NULL\n");
	}
	else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			 "wcid[%d] occupied\n", entry->wcid);
		if (tx_info && tx_info->aid) {
			u_int16 bak_aid = 0;

			bak_aid = entry->Aid;
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				 "AID forced from %d to %d\n",
				  bak_aid, tx_info->aid);
			entry->Aid = tx_info->aid;
			tx_info->aid = bak_aid;
		} else
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				 "AID allocated as %d\n",
				  entry->Aid);
	}

err_out:
	return ret;
}

s_int32 net_ad_free_wtbl(
	struct test_wlan_info *winfos,
	u_char *da,
	void *virtual_wtbl,
	struct test_tx_info *tx_info)
{
	RTMP_ADAPTER *ad = NULL;
	struct _MAC_TABLE_ENTRY *mac_tbl_entry = NULL;
	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	if (virtual_wtbl) {
		mac_tbl_entry = (struct _MAC_TABLE_ENTRY *)virtual_wtbl;

		if (tx_info && tx_info->aid) {
			mac_tbl_entry->Aid = tx_info->aid;
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				 "AID restored as %d\n",
				  tx_info->aid);
		} else
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				 "AID remain as %d\n",
				  mac_tbl_entry->Aid);
		MacTableDeleteEntry(ad, mac_tbl_entry->wcid, da);
	} else
		return SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;

	return SERV_STATUS_SUCCESS;
}

s_int32 net_ad_apply_wtbl(
	struct test_wlan_info *winfos,
	void *virtual_dev,
	void *virtual_wtbl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct serv_chip_cap *chip_cap = NULL;
	struct caps_info *cap = NULL;
	struct ampdu_caps *ampdu = NULL;
	u_short bw_winsiz = 0, tid_idx = 0;
	struct wifi_dev *wdev = (struct wifi_dev *)virtual_dev;
	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	chip_cap = &winfos->chip_cap;

	if (virtual_wtbl) {
		struct _MAC_TABLE_ENTRY *entry = NULL;
		struct phy_params *phy_info = NULL;

		entry = (struct _MAC_TABLE_ENTRY *)virtual_wtbl;
		phy_info = &entry->phy_param;
		cap = &entry->cap;
		ampdu = &entry->cap.ampdu;

		if (phy_info->phy_mode > MODE_VHT)
			entry->MaxHTPhyMode.field.MODE = MODE_VHT;
		else
			entry->MaxHTPhyMode.field.MODE = phy_info->phy_mode;

		if (phy_info->phy_mode > MODE_OFDM) {
			entry->MaxRAmpduFactor = chip_cap->ht_ampdu_exp;
			ampdu->max_ht_ampdu_len_exp = chip_cap->ht_ampdu_exp;
		}
		if (phy_info->phy_mode > MODE_HTGREENFIELD) {
			entry->MaxRAmpduFactor = chip_cap->vht_ampdu_exp;
			ampdu->max_mpdu_len = chip_cap->max_mpdu_len;
			ampdu->max_vht_ampdu_len_exp = chip_cap->vht_ampdu_exp;
		}
#if defined(DOT11_HE_AX)
		if (phy_info->phy_mode > MODE_VHT) {
			cap->modes |= (HE_24G_SUPPORT | HE_5G_SUPPORT);
			cap->he_mac_cap |= HE_AMSDU_IN_ACK_EN_AMPDU;
			ampdu->max_he_ampdu_len_exp = chip_cap->he_ampdu_exp;
			ampdu->max_vht_ampdu_len_exp = chip_cap->vht_ampdu_exp;
		}
#endif
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_WMM_CAPABLE);
		cap->ch_bw.he_ch_width = BW_80;

		if ((entry->EntryState != ENTRY_STATE_SYNC) &&
			(!wifi_sys_conn_act(wdev, virtual_wtbl))) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"connect action fail!\n");
		}

#if defined(DOT11_VHT_AC)
		if (IS_HIF_TYPE(ad, HIF_MT))
			RAInit(ad, entry);
#endif

		if (phy_info->phy_mode <= MODE_VHT)
			bw_winsiz = chip_cap->non_he_tx_ba_wsize;
#if defined(DOT11_HE_AX)
		else
			bw_winsiz = chip_cap->he_tx_ba_wsize;
#endif /* DIT11_HE_AX */

		for (tid_idx = 0; tid_idx < 8 ; tid_idx++)
			AsicUpdateBASession(ad,
					    entry->wcid,
					    tid_idx,
					    0,
					    bw_winsiz,
					    TRUE,
					    BA_SESSION_ORI,
					    0);

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		if (winfos->chip_cap.ra_offload == TRUE) {
			CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;
			RA_PHY_CFG_T *rate_cfg = NULL;
			u_int8 gi_type = phy_info->gi_type;
			u_int8 ltf_type = phy_info->ltf_type;

			entry->bAutoTxRateSwitch = FALSE;
			sys_ad_zero_mem(&rRaParam,
					sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
			rate_cfg = &rRaParam.FixedRateCfg;
			rate_cfg->MODE = phy_info->phy_mode;
			rate_cfg->STBC = phy_info->stbc;
			if (phy_info->phy_mode < MODE_HE_SU) {
				if (phy_info->gi_type)
					rate_cfg->ShortGI = BIT(phy_info->bw);
				else
					rate_cfg->ShortGI = 0;
			}
#if defined(DOT11_HE_AX)
			else {
				switch (phy_info->bw) {
				case BW_40:
					rate_cfg->ShortGI = (gi_type << 2);
					rate_cfg->he_ltf = (ltf_type << 2);
					break;
				case BW_80:
					rate_cfg->ShortGI = (gi_type << 4);
					rate_cfg->he_ltf = (ltf_type << 4);
					break;
				case BW_160:
					rate_cfg->ShortGI = (gi_type << 6);
					rate_cfg->he_ltf = (ltf_type << 6);
					break;
				default:
					rate_cfg->ShortGI = gi_type;
					rate_cfg->he_ltf = ltf_type;
				}
			}
#endif /* DOT11_HE_AX */
			rate_cfg->BW = phy_info->bw;
			if (phy_info->ldpc) {
				switch (phy_info->phy_mode) {
				case MODE_HTMIX:
				case MODE_HTGREENFIELD:
					rate_cfg->ldpc = 1;
					break;
				case MODE_VHT:
					rate_cfg->ldpc = 2;
					break;
#if defined(DOT11_HE_AX)
				case MODE_HE_SU:
				case MODE_HE_EXT_SU:
				case MODE_HE_TRIG:
				case MODE_HE_MU:
					rate_cfg->ldpc = 4;
					break;
#endif /* DOT11_HE_AX */
				default:/* should not happen */
					rate_cfg->ldpc = 0;
				}
			}
			rate_cfg->MCS = phy_info->rate;
			if (phy_info->phy_mode == TEST_MODE_HE_MU ||
				phy_info->phy_mode == TEST_MODE_VHT_MIMO) {
				/* work-around to fix OFDM 54M preventing TX CCK
				 * while 5GHz band due to fixrate
				 * not support HEMU/VHT MIMO
				 */
				rate_cfg->MODE = TEST_MODE_OFDM;
				rate_cfg->MCS = 7;
			}
			if (phy_info->dcm)
				rate_cfg->MCS |= BIT(4);
			if (phy_info->su_ext_tone)
				rate_cfg->MCS |= BIT(5);
			rate_cfg->VhtNss = phy_info->vht_nss;
			rRaParam.u4Field = RA_PARAM_FIXED_RATE;
			RAParamUpdate(ad, entry, &rRaParam);
		}
#endif
	} else
		ret = SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;

	return ret;
}

s_int32 net_ad_match_wtbl(
	void *virtual_wtbl,
	u_int16 wcid)
{
	s_int32 ret = SERV_STATUS_OSAL_NET_FAIL;
	struct _MAC_TABLE_ENTRY *mac_tbl_entry = NULL;
	/* Get adapter from jedi driver first */

	mac_tbl_entry = (struct _MAC_TABLE_ENTRY *)virtual_wtbl;

	if (mac_tbl_entry) {
		if (mac_tbl_entry->wcid == wcid)
			ret = SERV_STATUS_SUCCESS;
		else
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE,
				 DBG_LVL_NOTICE,
				 "Not match(%d:%d)!\n",
				  mac_tbl_entry->wcid, wcid);
	} else {
		ret = SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE,
			 DBG_LVL_ERROR,
			 "Invalid virtual wtbl!\n");
	}

	return ret;
}

s_int32 net_ad_get_wmm_idx(
	IN void *virtual_device,
	OUT u_int8 *wmm_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct wifi_dev *wdev = (struct wifi_dev *)virtual_device;

	if (wdev == NULL) {
		ret = SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;
		goto err_out;
	}

	*wmm_idx = HcGetWmmIdx(NULL, wdev);

err_out:
	return ret;
}

s_int32 net_ad_get_band_idx(
	IN void *virtual_device,
	OUT u_char *band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	if (virtual_device == NULL)
		ret = SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;
	else {
		struct wifi_dev *wdev = (struct wifi_dev *)virtual_device;

		*band_idx = HcGetBandByWdev(wdev);
	}

	return ret;
}

s_int32 net_ad_get_omac_idx(
	IN struct test_wlan_info *winfos,
	IN void *virtual_device,
	OUT u_char *omac_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL) {
		ret = SERV_STATUS_OSAL_NET_INVALID_PAD;
		goto err_out;
	}

	if (virtual_device == NULL)
		ret = SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;
	else {
		struct wifi_dev *wdev = (struct wifi_dev *)virtual_device;

		*omac_idx = HcGetOmacIdx(ad, wdev);
	}

err_out:
	return ret;
}

s_int32 net_ad_fill_phy_info(
	void *virtual_wtbl,
	struct test_tx_info *tx_info)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct phy_params *phy_info = NULL;

	entry = (struct _MAC_TABLE_ENTRY *)virtual_wtbl;
	phy_info = &entry->phy_param;

	sys_ad_zero_mem(phy_info, sizeof(*phy_info));

	phy_info->phy_mode = tx_info->tx_mode;
#ifdef TXBF_SUPPORT
	phy_info->tx_ibf = tx_info->ibf;
	phy_info->tx_ebf = tx_info->ebf;
#endif
	phy_info->stbc = tx_info->stbc;
	phy_info->ldpc = tx_info->ldpc;
	phy_info->bw = tx_info->bw;
	phy_info->vht_nss = tx_info->nss;
	phy_info->gi_type = tx_info->gi;
	phy_info->ltf_type = tx_info->ltf;
	phy_info->ucTxFrTblRate = tx_info->fixed_rate;

#if defined(DOT11_HE_AX)
	if (phy_info->phy_mode > TEST_MODE_VHT) {
		phy_info->rate = (tx_info->mcs & 0xf);
		/* b'4 for DCM */
		phy_info->dcm = (tx_info->mcs & BIT(4)) ? TRUE : FALSE;

		if (phy_info->phy_mode == TEST_MODE_HE_ER) {
			/* b'5 for tone*/
			if (tx_info->mcs & BIT(5))
				phy_info->su_ext_tone = TRUE;
			else
				phy_info->su_ext_tone = FALSE;
		}
	} else
#endif /* DOT11_HE_AX */
	{
		phy_info->rate = (tx_info->mcs & 0x1f);

		if (phy_info->phy_mode == TEST_MODE_CCK) {
			phy_info->rate = tx_info->mcs & TEST_CCK_RATE_MASK;
			if ((tx_info->mcs & TEST_CCK_SHORT_PREAMBLE)
			     && phy_info->rate)
				phy_info->rate--;

			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE,
				 DBG_LVL_NOTICE,
				 "CCK MCS%d\n",
				  phy_info->rate);
		}
	}

	return ret;
}

s_int32 net_ad_get_speidx(
	IN struct test_wlan_info *winfos,
	IN u_int16 ant_sel,
	OUT u_int8 *spe_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_int8 map_idx = 0;
	struct serv_spe_map *spe_map = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL) {
		ret = SERV_STATUS_OSAL_NET_INVALID_PAD;
		goto err_out;
	}

	*spe_idx = 0;
	spe_map = winfos->chip_cap.spe_map_list.spe_map;
	for (map_idx = 0;
		map_idx < winfos->chip_cap.spe_map_list.size;
		map_idx++) {
		if (ant_sel == spe_map[map_idx].ant_sel) {
			*spe_idx = spe_map[map_idx].spe_idx;
			break;
		}
	}
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"spe idx=%d(map_idx:%d/%d)\n",
		*spe_idx, map_idx,
		winfos->chip_cap.spe_map_list.size);

err_out:
	return ret;
}


s_int32 net_ad_fill_spe_antid(
	struct test_wlan_info *winfos,
	void *virtual_wtbl,
	u_int8 spe_idx,
	u_int8 ant_pri)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct phy_params *phy_info = NULL;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;
#endif	/* RACTRL_FW_OFFLOAD_SUPPORT */

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL) {
		ret = SERV_STATUS_OSAL_NET_INVALID_PAD;
		goto err_out;
	}

	if (virtual_wtbl == NULL) {
		ret = SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;
		goto err_out;
	}

	entry = (struct _MAC_TABLE_ENTRY *)virtual_wtbl;
	phy_info = &entry->phy_param;

	phy_info->spe_idx = spe_idx;
	phy_info->ant_pri = ant_pri;

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	sys_ad_zero_mem(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));

	rRaParam.ucSpeEn = spe_idx;
	rRaParam.u4Field = RA_PARAM_SPE_UPDATE;
	RAParamUpdate(ad, entry, &rRaParam);
#endif	/* RACTRL_FW_OFFLOAD_SUPPORT */

err_out:
	return ret;
}


s_int32 net_ad_compose_pkt(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	s_int32 sta_idx, u_char *buf,
	u_int32 txlen, u_int32 hlen)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_char *tmac_info, *pheader, *payload;
	u_char *addr1, *addr2, *addr3, *template;
	u_int8 tx_hw_hdr_len;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	tx_hw_hdr_len = winfos->chip_cap.tx_wi_size;
	addr1 = configs->addr1[sta_idx];
	addr2 = configs->addr2[sta_idx];
	addr3 = configs->addr3[sta_idx];
	template = configs->template_frame;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"(wcid:%d):: DA: %02x:%02x:%02x:%02x:%02x:%02x\n\t",
		configs->wcid_ref, SERV_PRINT_MAC(addr1));
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"SA: %02x:%02x:%02x:%02x:%02x:%02x\n\t",
		SERV_PRINT_MAC(addr2));
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"BSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",
		SERV_PRINT_MAC(addr3));

	/* Error check for txlen */
	if (txlen == 0) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"tx length can't be 0!!\n");

		return SERV_STATUS_OSAL_NET_INVALID_LEN;
	}

	tmac_info = buf;
	pheader = (buf + tx_hw_hdr_len);
	payload = (pheader + hlen);
	sys_ad_zero_mem(buf, TEST_PKT_LEN);
	sys_ad_move_mem(pheader, template, hlen);
	sys_ad_move_mem(pheader + 4, addr1, SERV_MAC_ADDR_LEN);
	sys_ad_move_mem(pheader + 10, addr2, SERV_MAC_ADDR_LEN);
	sys_ad_move_mem(pheader + 16, addr3, SERV_MAC_ADDR_LEN);

	ret = net_ad_init_payload(winfos, configs, payload, txlen - hlen);
	if (ret)
		return ret;

	/* TODO: factor out here for log dump */

#ifdef CFG_BIG_ENDIAN
	RTMPFrameEndianChange(ad, (pu_char) pheader, DIR_WRITE, FALSE);
#ifdef MT_MAC
	if (IS_HIF_TYPE(ad, HIF_MT))
		MTMacInfoEndianChange(ad, tmac_info, TYPE_TMACINFO,
					sizeof(TMAC_TXD_L));
#endif
#endif

	return ret;
}

s_int32 net_ad_alloc_pkt(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_int32 mpdu_length,
	void **pkt_skb)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_char *src_buff;
	u_short qid;
	u_int8 tx_hw_hdr_len;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	src_buff = configs->test_pkt;
	qid = configs->ac_idx;
	tx_hw_hdr_len = winfos->chip_cap.tx_hw_hdr_len;

	if (!src_buff) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"invalid test_pkt\n");
		goto err_out;
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"test_pkt=%p, ring idx=%u\n", src_buff, qid);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"txlen=%d, tx_hw_hdr_len=%d, total=%d\n",
			mpdu_length, tx_hw_hdr_len,
			mpdu_length + tx_hw_hdr_len);

		/* txd is handled by hwifi, therefore skip tx_hw_hdr_len */
		ret = RTMPAllocateNdisPacket(ad,
				     pkt_skb,
				     NULL,
				     0,
				     src_buff + tx_hw_hdr_len,
				     mpdu_length);

	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"AllocateNdisPacket fail\n");
		goto err_out;
	}

err_out:
	return ret;
}

s_int32 net_ad_free_pkt(
	struct test_wlan_info *winfos,
	void *pkt_skb)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	RTMPFreeNdisPacket(ad, pkt_skb);

	return ret;
}

s_int32 net_ad_enq_pkt(
	struct test_wlan_info *winfos,
	u_short q_idx,
	void *virtual_wtbl,
	void *virtual_device,
	void *pkt_skb)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	PNDIS_PACKET pkt = NULL;
	struct sk_buff *skb = NULL, *skb2 = NULL;
	struct wifi_dev *wdev = (struct wifi_dev *)virtual_device;
	struct _MAC_TABLE_ENTRY *entry = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL) {
		ret = SERV_STATUS_OSAL_NET_INVALID_PAD;
		goto err_out;
	}

	if (virtual_device)
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
			"wdev_idx=%d, q_idx=%d, pkt_va=%p\n",
			wdev->wdev_idx, q_idx, pkt_skb);
	else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"invalid wdev(%p)!\n", wdev);
		ret = SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;
		goto err_out;
	}

	if (virtual_wtbl)
		entry = (struct _MAC_TABLE_ENTRY *)virtual_wtbl;
	else {
		ret = SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;
		goto err_out;
	}

	if (pkt_skb) {
		skb = (struct sk_buff *)pkt_skb;
		SERV_OS_PKT_CLONE(skb, skb2, GFP_ATOMIC);

		if (skb2 == NULL) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"clone packet fail\n");
			ret = SERV_STATUS_OSAL_NET_ALLOC_FAIL;
			goto err_out;
		}  else {
			pkt = (PNDIS_PACKET)skb2;
			RTMP_SET_PACKET_WCID(pkt, entry->wcid);
			RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
			RTMP_SET_BAND_IDX(pkt, HcGetBandByWdev(wdev));

			RTMP_SET_PACKET_TXTYPE(pkt, TX_ATE_FRAME);

			if (q_idx > 0) {
				RTMP_SET_PACKET_QUEIDX(pkt, QID_AC_BE);
				RTMP_SET_PACKET_TYPE(pkt, TX_DATA);
			} else {
				RTMP_SET_PACKET_QUEIDX(pkt, 0);
				RTMP_SET_PACKET_TYPE(pkt, TX_MGMT);
			}
		}

		ret = send_mlme_pkt(ad, pkt, wdev, q_idx, FALSE);
	}

	if (ret)
		ret = SERV_STATUS_OSAL_NET_FAIL;

err_out:
	return ret;
}

s_int32 net_ad_post_tx(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_int8 band_idx,
	void *pkt)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_int32 txdone_cnt = 0, tx_cnt = 0, op_mode = 0;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	if (band_idx >= TEST_BAND_NUM) {
		ret = SERV_STATUS_OSAL_NET_INVALID_BANDIDX;
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"wrong band_idx %u, ret=0x%08x\n",
			band_idx, ret);
		goto done;
	}

	tx_cnt = configs->tx_stat.tx_cnt;
	op_mode = configs->op_mode;

	/* Do not count in packet number when tx is not in start stage */
	if (!(op_mode & OP_MODE_TXFRAME))
		goto done;

	/* Triggered when RX tasklet free token */
	if (pkt) {
		ad->RalinkCounters.KickTxCount++;
		txdone_cnt++;
	}

	if (configs->tx_strategy == TEST_TX_STRA_THREAD)
		net_ad_thread_proceed_tx(winfos, band_idx);
	else if (configs->tx_strategy == TEST_TX_STRA_TASKLET) {
		if ((op_mode & OP_MODE_TXFRAME) && (txdone_cnt < tx_cnt))
			ret = net_ad_enq_pkt(winfos,
					     configs->ac_idx,
					     configs->stack.virtual_wtbl[0],
					     configs->stack.virtual_device[0],
					     configs->stack.pkt_skb[0]);
		else if ((op_mode & OP_MODE_TXFRAME)
			&& (txdone_cnt == tx_cnt)) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"all tx is done\n");

			if (op_mode & fTEST_MPS) {
				SERV_OS_COMPLETION *tx_wait;

				tx_wait = &configs->tx_wait;
				SERV_OS_COMPLETE(tx_wait);
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE,
					DBG_LVL_NOTICE,
					"finish one MPS item\n");
			}

			/* Tx status enters idle mode */
			configs->tx_status = 0;
		} else if (!(op_mode & OP_MODE_TXFRAME)) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"stop tx bottom is pressed\n");

			if (op_mode & fTEST_MPS) {
				SERV_OS_COMPLETION *tx_wait;

				tx_wait = &configs->tx_wait;
				op_mode &= ~fTEST_MPS;
				configs->op_mode = op_mode;
				SERV_OS_COMPLETE(tx_wait);
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE,
					DBG_LVL_NOTICE,
					"MPS stop\n");
			}
		} else {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_WARN,
				"do not match any condition, ");
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_WARN,
				"op_mode:0x%x, tx_cnt:%u, txdone_cnt:%u\n",
				op_mode, tx_cnt, txdone_cnt);
		}
	} else {
		ret = SERV_STATUS_OSAL_NET_INVALID_PARAM;
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"wrong tx strategy=%d, ret=0x%08x\n",
			configs->tx_strategy, ret);
		goto done;
	}

done:
	return ret;
}

s_int32 net_ad_rx_done_handle(
	struct test_wlan_info *winfos,
	void *rx_blk)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct service_test *serv_test;
	RX_BLK *rxblk = (RX_BLK *)rx_blk;
	u_char band_idx;
	u_int32 chfreq0 = 0, chfreq1 = 0;
	u_int32 bn0_cr_addr = RMAC_CHFREQ0;
	u_int32 bn1_cr_addr = RMAC_CHFREQ1;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	if (IS_MT7915(ad) || IS_MT7986(ad) || IS_MT7916(ad) || IS_MT7981(ad) ||
		IS_MT7990(ad) || IS_MT7992(ad) || IS_MT7993(ad))
		band_idx = rxblk->band;
	else {
		MAC_IO_READ32(ad->hdev_ctrl, bn0_cr_addr, &chfreq0);
		MAC_IO_READ32(ad->hdev_ctrl, bn1_cr_addr, &chfreq1);

		/* Note: shall not use ad here */
		serv_test = (struct service_test *)ad->serv.serv_handle;

		/* RX packet counter calculate by chfreq of RXD */
		if (rxblk->channel_freq == chfreq0)
			band_idx = TEST_BAND0;
		else if (rxblk->channel_freq == chfreq1)
			band_idx = TEST_BAND1;
		else {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"wrong chfreq!!\n"
				 "\tRXD.ch_freq=%u, chfreq0=%u, chfreq1=%u\n",
				rxblk->channel_freq,
				chfreq0, chfreq1);
			return SERV_STATUS_OSAL_NET_INVALID_PARAM;
		}
	}

	return ret;
}

s_int32 net_ad_get_chip_info(
	struct test_wlan_info *winfos,
	struct _net_testmode_chip_info *ptestmode_chip_info)
{
	RTMP_ADAPTER *ad = NULL;
	struct _RTMP_CHIP_OP *chip_ops = NULL;
	u_int8 band_num = 0, band_idx = 0;

	struct _chip_testmode_chip_info testmode_chip_info;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);

	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	if (ptestmode_chip_info == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	sys_ad_zero_mem(ptestmode_chip_info,
		sizeof(struct _net_testmode_chip_info));

	sys_ad_zero_mem(&testmode_chip_info,
		sizeof(struct _chip_testmode_chip_info));

	chip_ops = hc_get_chip_ops(ad->hdev_ctrl);
	if (chip_ops == NULL)
		return SERV_STATUS_HAL_MAC_INVALID_CHIPOPS;

	testmode_chip_info.u4ChipId = winfos->chip_id;
	if (chip_ops->get_testmode_chip_info)
		chip_ops->get_testmode_chip_info(ad, &testmode_chip_info);

	ptestmode_chip_info->u1chip_band_num = PD_GET_BAND_NUM(ad->physical_dev);
	ptestmode_chip_info->band_used_num = PD_GET_BAND_NUM(ad->physical_dev);
	ptestmode_chip_info->pAd_band_idx = hc_get_hw_band_idx(ad);

	ptestmode_chip_info->u4ChipId = testmode_chip_info.u4ChipId;
	band_num = ptestmode_chip_info->u1chip_band_num;
	ptestmode_chip_info->u1testmode_sup_mlo = testmode_chip_info.u1testmode_sup_mlo;

	for (band_idx = 0; band_idx < band_num; band_idx++) {
		if (physical_device_get_mac_adapter_by_band(ad->physical_dev, band_idx))
			ptestmode_chip_info->au1used_band[band_idx] = 1;
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
			"band_num:%d, band_used_num:%d, %d:%d:%d\n",
			ptestmode_chip_info->u1chip_band_num,
			ptestmode_chip_info->band_used_num,
			ptestmode_chip_info->au1used_band[TEST_BAND0],
			ptestmode_chip_info->au1used_band[TEST_BAND1],
			ptestmode_chip_info->au1used_band[TEST_BAND2]);

	return SERV_STATUS_SUCCESS;
}

s_int32 net_ad_get_band_mode(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 *band_type)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct _net_testmode_chip_info chip_info;
	u_int16 band_sel[] = {TEST_BAND_TYPE_UNUSE,
		TEST_BAND_TYPE_2_4G, TEST_BAND_TYPE_5G, TEST_BAND_TYPE_6G,
		TEST_BAND_TYPE_5G, TEST_BAND_TYPE_5G, TEST_BAND_TYPE_6G,
		TEST_BAND_TYPE_6G};

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	sys_ad_zero_mem(&chip_info, sizeof(struct _net_testmode_chip_info));

	ret = net_ad_get_chip_info(winfos, &chip_info);
	if (ret)
		return ret;

	if (IS_MT7915(ad) || IS_MT7986(ad) || IS_MT7916(ad) || IS_MT7981(ad)) {
		if (band_idx > TEST_BAND1) {
			*band_type = TEST_BAND_TYPE_UNUSE;
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"band_idx=%d, band_type=%d\n",
				band_idx, *band_type);
			return ret;
		}
	} else if (IS_BELLWETHER(ad)) {
		if (band_idx == TEST_BAND0)
			*band_type = TEST_BAND_TYPE_5G_6G;
		else if (band_idx == TEST_BAND2 &&
				chip_info.au1used_band[TEST_BAND2])
			*band_type = TEST_BAND_TYPE_2_4G_5G;
		else
			*band_type = TEST_BAND_TYPE_UNUSE;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"BELLWETHER band_idx=%d, band_type=%d\n",
				band_idx, *band_type);
	} else if (IS_MT7990(ad)) {
		ad = net_ad_multi_inf_get_pAd_by_Band(winfos, band_idx);
		if (ad != NULL)
			*band_type = band_sel[ad->BandSel];
		else
			*band_type = TEST_BAND_TYPE_UNUSE;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"MT7990 band_idx=%d, band_type=%d\n",
				band_idx, *band_type);
	} else if (IS_MT7992(ad)) {
#if defined(MT7992_SKU_BE3600SDB)
		if (band_idx == TEST_BAND0)
			*band_type = TEST_BAND_TYPE_2_4G_5G;
		else if (band_idx == TEST_BAND1)
			*band_type = TEST_BAND_TYPE_5G_6G;
		else
			*band_type = TEST_BAND_TYPE_UNUSE;
#else
		if (band_idx == TEST_BAND0)
			*band_type = TEST_BAND_TYPE_2_4G;
		else if (band_idx == TEST_BAND1)
			*band_type = TEST_BAND_TYPE_5G_6G;
		else
			*band_type = TEST_BAND_TYPE_UNUSE;
#endif
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"MT7992 band_idx=%d, band_type=%d\n",
				band_idx, *band_type);
	} else if (IS_MT7993(ad)) {
		if (band_idx == TEST_BAND0)
			*band_type = TEST_BAND_TYPE_2_4G;
		else if (band_idx == TEST_BAND1)
			*band_type = TEST_BAND_TYPE_5G_6G;
		else
			*band_type = TEST_BAND_TYPE_UNUSE;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"MT7993 band_idx=%d, band_type=%d\n",
				band_idx, *band_type);
	}

	return ret;
}

s_int32 net_ad_set_band_mode(
	struct test_wlan_info *winfos,
	struct test_band_state *band_state)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	if (band_state->band_mode == TEST_BAND_MODE_SINGLE) {
		if (band_state->band_type == TEST_BAND_TYPE_5G)
			Set_WirelessMode_Proc(ad, "14");
		else if (band_state->band_type == TEST_BAND_TYPE_2_4G)
			Set_WirelessMode_Proc(ad, "9");
		else
			ret = SERV_STATUS_OSAL_NET_INVALID_PARAM;
	}

	return ret;
}

s_int32 net_ad_set_txpwr_sku(
	struct test_wlan_info *winfos,
	u_char sku_ctrl, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	ret = MtCmdTxPowerSKUCtrl(ad, sku_ctrl, band_idx);
	if (ret)
		ret = SERV_STATUS_OSAL_NET_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 net_ad_set_txpwr_power_drop(
	struct test_wlan_info *winfos,
	u_char power_drop, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	ret = MtCmdTxPowerDropCtrl(ad, power_drop, band_idx);
	if (ret)
		ret = SERV_STATUS_OSAL_NET_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 net_ad_set_txpwr_percentage(
	struct test_wlan_info *winfos,
	u_char percentage_ctrl, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	ret = MtCmdTxPowerPercentCtrl(ad, percentage_ctrl, band_idx);
	if (ret)
		ret = SERV_STATUS_OSAL_NET_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 net_ad_set_txpwr_backoff(
	struct test_wlan_info *winfos,
	u_char backoff_ctrl, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	ret = MtCmdTxBfBackoffCtrl(ad, backoff_ctrl, band_idx);
	if (ret)
		ret = SERV_STATUS_OSAL_NET_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 net_ad_init_txpwr(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	if (band_idx >= TEST_BAND_NUM)
		return SERV_STATUS_OSAL_NET_INVALID_BANDIDX;

	/* Disable tx power related function for test mode */
	ret = net_ad_set_txpwr_sku(winfos, configs->tx_pwr_sku_en, band_idx);
	if (ret)
		goto error;

	ret = net_ad_set_txpwr_power_drop(
			winfos, configs->tx_pwr_percentage_level, band_idx);
	if (ret)
		goto error;

	ret = net_ad_set_txpwr_percentage(
			winfos, configs->tx_pwr_percentage_en, band_idx);
	if (ret)
		goto error;

	ret = net_ad_set_txpwr_backoff(
			winfos, configs->tx_pwr_backoff_en, band_idx);
	if (ret)
		goto error;

	return ret;

error:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"engine init tx power fail, err=0x%08x\n", ret);

	return ret;
}

s_int32 net_ad_handle_mcs32(
	struct test_wlan_info *winfos,
	void *virtual_wtbl, u_int8 bw)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	union WTBL_DW5 wtbl_txcap;
	u_int32 dw_mask = 0;
	struct _MAC_TABLE_ENTRY *entry = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	entry = (struct _MAC_TABLE_ENTRY *)virtual_wtbl;
	if (entry == NULL) {
		ret = SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;
		goto err_out;
	}

	dw_mask = ~(3 << 12);	/* only update fcap bit[13:12] */
	wtbl_txcap.field.fcap = bw;

	/* WTBLDW5 */
	ret = WtblDwSet(ad, entry->wcid, 1, 5, dw_mask, wtbl_txcap.word);
	if (ret)
		ret = SERV_STATUS_OSAL_NET_FAIL_SEND_FWCMD;

err_out:
	return ret;
}

s_int32 net_ad_cfg_wtbl(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	struct test_tx_info *tx_info
)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct _RTMP_CHIP_CAP *cap = NULL;
#ifndef WIFI_UNIFIED_COMMAND
	struct _EXT_CMD_ATE_TEST_MODE_T param;
#else
	struct _UNI_CMD_TEST_TR_PARAM_CTRL_T param;
	uint64_t u8TestTrParamFeature;
	uint8_t ucParamNum = 1;
#endif /*WIFI_UNIFIED_COMMAND*/

	P_HT_CAP_T   wtbl_ht_cap;
	P_VHT_CAP_T  wtbl_vht_cap;
	P_ANT_CAP_T  wtbl_ant_cap;
	P_BA_CAP_T   wtbl_ba_cap;
	P_RATE_CAP_T wtbl_rate_cap;
	u_int8 need_qos, need_ampdu;
	u_char tx_mode, mcs, nss, bw, sgi, stbc, ldpc, preamble, u4Stbc;
	u_int32 ant_sel = 0;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	cap = hc_get_chip_cap(ad->hdev_ctrl);
	wtbl_ht_cap   = &param.Data.rAteSetAmpduWtbl.rWtblHt;
	wtbl_vht_cap  = &param.Data.rAteSetAmpduWtbl.rWtblVht;
	wtbl_ant_cap  = &param.Data.rAteSetAmpduWtbl.rWtblAnt;
	wtbl_ba_cap   = &param.Data.rAteSetAmpduWtbl.rWtblBa;
	wtbl_rate_cap = &param.Data.rAteSetAmpduWtbl.rWtblRate;

	need_qos = tx_info->mpdu_info.need_qos;
	need_ampdu = tx_info->mpdu_info.need_ampdu;
	tx_mode = tx_info->tx_mode;
	mcs = tx_info->mcs;
	nss = tx_info->nss;
	bw = tx_info->bw;
	sgi = configs->sgi;
	stbc = configs->stbc;
	ldpc = configs->ldpc;
	ant_sel = configs->tx_ant;
	preamble = configs->preamble;

	sys_ad_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = TRUE;

#ifndef WIFI_UNIFIED_COMMAND
	param.ucAteIdx = ENUM_ATE_SET_AMPDU_WTBL;
#else
	param.ucAction = ENUM_TR_PARAM_SET;
	u8TestTrParamFeature = UNI_ATE_AMPDU_WTBL_FEATURE;
#endif /*WIFI_UNIFIED_COMMAND*/

	switch (tx_mode) {
	case TXD_TX_MODE_HTMIX:
	case TXD_TX_MODE_HTGREENFIELD:
		wtbl_ht_cap->fgIsHT = TRUE;
		wtbl_ht_cap->fgLDPC = ldpc;

		if (cap)
			wtbl_ht_cap->ucAmpduFactor
				= winfos->chip_cap.ht_ampdu_exp;
		else
			wtbl_ht_cap->ucAmpduFactor = 3;

		break;

	case TXD_TX_MODE_VHT:
		wtbl_ht_cap->fgIsHT = 1;
		wtbl_ht_cap->fgLDPC = ldpc;
		wtbl_vht_cap->fgIsVHT = 1;
		wtbl_vht_cap->fgVhtLDPC = ldpc;

		if (cap)
			wtbl_ht_cap->ucAmpduFactor
				= winfos->chip_cap.vht_ampdu_exp;
		else
			wtbl_ht_cap->ucAmpduFactor = 7;

		break;
	case TXD_TX_MODE_HE_SU:
		wtbl_ht_cap->fgIsHT = 1;
		wtbl_ht_cap->fgLDPC = ldpc;
		wtbl_vht_cap->fgIsVHT = 1;
		wtbl_vht_cap->fgVhtLDPC = ldpc;

		if (cap)
			wtbl_ht_cap->ucAmpduFactor
				= winfos->chip_cap.vht_ampdu_exp;
		else
			wtbl_ht_cap->ucAmpduFactor = 7;

		break;
	case TXD_TX_MODE_EHT_MU:
		wtbl_ht_cap->fgIsHT = 1;
		wtbl_ht_cap->fgLDPC = ldpc;
		wtbl_vht_cap->fgIsVHT = 1;
		wtbl_vht_cap->fgVhtLDPC = ldpc;

		if (cap)
			wtbl_ht_cap->ucAmpduFactor
				= winfos->chip_cap.vht_ampdu_exp;
		else
			wtbl_ht_cap->ucAmpduFactor = 7;

		break;

	default:
		wtbl_ht_cap->fgIsHT = 0;
		wtbl_vht_cap->fgIsVHT = 0;
		break;
	}

	if (need_ampdu) {
		if (ant_sel & TEST_ANT_USER_SEL) {
			ant_sel &= ~TEST_ANT_USER_SEL;
		} else {
			s_int32 map_idx = 0;
			s_int32 map_idx_len = sizeof(test_ant_to_spe_idx_map)
					/ sizeof(test_ant_to_spe_idx_map[0]);

			for (map_idx = 0; map_idx < map_idx_len; map_idx++) {
				if (ant_sel ==
				test_ant_to_spe_idx_map[map_idx].ant_sel)
					break;
			}
			if (map_idx == map_idx_len)
				ant_sel = 0;
			else
				ant_sel
				= test_ant_to_spe_idx_map[map_idx].spe_idx;
		}

		wtbl_ant_cap->ucSpe = (ant_sel & 0x1F);
		wtbl_ant_cap->AntIDConfig.ucANTIDSts0 = ant_sel;
		wtbl_ant_cap->AntIDConfig.ucANTIDSts1 = ant_sel;
		wtbl_ant_cap->AntIDConfig.ucANTIDSts2 = ant_sel;
		wtbl_ant_cap->AntIDConfig.ucANTIDSts3 = ant_sel;

		wtbl_ba_cap->ucBaEn = 1;
		wtbl_ba_cap->ucBaSize = 7;
		param.Data.rAteSetAmpduWtbl.ucIPsm = 1;
	}

	wtbl_rate_cap->ucFcap = bw;

	if (sgi) {
		switch (bw) {
		case TEST_BW_20:
			wtbl_rate_cap->fgG2 = TRUE;
			break;

		case TEST_BW_40:
			wtbl_rate_cap->fgG4 = TRUE;
			break;

		case TEST_BW_80:
			wtbl_rate_cap->fgG8 = TRUE;
			break;
		case TEST_TXD_BW160:
		case TEST_BW_160C:
			wtbl_rate_cap->fgG16 = TRUE;
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"can't find such bw, use default\n");
			break;
		}
	}

	u4Stbc = raStbcSettingCheck(stbc, tx_mode, mcs, nss, 0, 0);

	wtbl_rate_cap->ucStbc = u4Stbc;
	wtbl_rate_cap->ucMode = tx_mode;
	if (tx_mode == TXD_TX_MODE_EHT_MU)
		wtbl_rate_cap->ucSgi = (tx_info->gi << 4) | tx_info->ltf;
	else
		wtbl_rate_cap->ucSgi = sgi;
	wtbl_rate_cap->ucBw = bw;
	wtbl_rate_cap->ucNss = nss;
	wtbl_rate_cap->ucPreamble = preamble;
	wtbl_rate_cap->ucLdpc = ldpc;
	wtbl_rate_cap->au2RateCode = mcs;

	if (need_qos)
		param.Data.rAteSetAmpduWtbl.ucQos = 1;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"wtbl ucBw=%d ucSgi=%d\n",
				wtbl_rate_cap->ucBw, wtbl_rate_cap->ucSgi);

#ifdef CONFIG_HW_HAL_OFFLOAD
#ifndef WIFI_UNIFIED_COMMAND
	ret = MtCmdATETest(ad, &param);
#else

	ret = UniCmdATETest(ad, &param, u8TestTrParamFeature, ucParamNum, NULL);
#endif /*WIFI_UNIFIED_COMMAND*/
#endif /*CONFIG_HW_HAL_OFFLOAD*/

	return ret;
}

s_int32 net_ad_set_wmm_param_by_qid(
	u_char wmm_idx,
	u_int8 q_idx,
	struct test_wlan_info *winfos,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct ipg_param *ipg_param;
	u_int16 slot_time, sifs_time, cw;
	u_int8 ac_num, aifsn;
#ifdef WIFI_UNIFIED_COMMAND
	struct wifi_dev *wdev;
#endif /* WIFI_UNIFIED_COMMAND */

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	ipg_param = &configs->ipg_param;

	if (wmm_idx > 3) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"invalid wmm_idx=%d, ",
			wmm_idx);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"reset to 0xff!\n");
		wmm_idx = 0xFF;
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"wmm_idx=%d\n", wmm_idx);

	if ((q_idx != QID_AC_BE)
		&& (q_idx != TxQ_IDX_ALTX0)
		&& (q_idx != TxQ_IDX_ALTX1)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"impossible!\n");
		return SERV_STATUS_OSAL_NET_INVALID_PARAM;
	}

	slot_time = ipg_param->slot_time;
	sifs_time = ipg_param->sifs_time;
	ac_num = q_idx;
	aifsn = ipg_param->aifsn;
	cw = ipg_param->cw;
#ifdef WIFI_UNIFIED_COMMAND
	wdev = (struct wifi_dev *)configs->stack.virtual_device[0];
	ret = AsicSetWmmParam(ad, wdev, wmm_idx,
				(u_int32) ac_num, WMM_PARAM_AIFSN,
				(u_int32) aifsn);
	if (ret)
		return ret;

	ret = AsicSetWmmParam(ad, wdev, wmm_idx,
				(u_int32) ac_num, WMM_PARAM_CWMIN,
				(u_int32) cw);
	if (ret)
		return ret;

	ret = AsicSetWmmParam(ad, wdev, wmm_idx,
				(u_int32) ac_num, WMM_PARAM_CWMAX,
				(u_int32) cw);
#else
	ret = AsicSetWmmParam(ad, wmm_idx,
				(u_int32) ac_num, WMM_PARAM_AIFSN,
				(u_int32) aifsn);
	if (ret)
		return ret;

	ret = AsicSetWmmParam(ad, wmm_idx,
				(u_int32) ac_num, WMM_PARAM_CWMIN,
				(u_int32) cw);
	if (ret)
		return ret;

	ret = AsicSetWmmParam(ad, wmm_idx,
				(u_int32) ac_num, WMM_PARAM_CWMAX,
				(u_int32) cw);
#endif /* WIFI_UNIFIED_COMMAND */
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"qid=%d, slot_time=%d, sifs_time=%d, ",
		q_idx, slot_time, sifs_time);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"ac_num=%d, aifsn=%d, cw=%d\n", ac_num, aifsn, cw);

	return ret;
}

s_int32 net_ad_clean_sta_q(
	struct test_wlan_info *winfos, u_char band_idx, u_char wcid)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct qm_ops *qm_ops = NULL;
	struct wifi_dev *wdev = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	wdev = &ad->ate_wdev;

	qm_ops = PD_GET_QM_OPS(ad->physical_dev);

	if (qm_ops->sta_clean_queue) {
		ret = qm_ops->sta_clean_queue(ad, wcid);
		if (ret)
			ret = SERV_STATUS_OSAL_NET_FAIL_SEND_FWCMD;
	}

	if (qm_ops->bss_clean_queue)
		qm_ops->bss_clean_queue(ad, wdev);


	return ret;
}

s_int32 net_ad_set_auto_resp(
	struct test_wlan_info *winfos,
	struct test_operation *ops,
	struct test_configuration *configs,
	u_char band_idx, u_char mode)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = ops->op_set_tr_mac(winfos, SERV_TEST_MAC_TX, TRUE, band_idx);

	return ret;
}

s_int32 net_ad_set_device_info(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char *addr,
	u_char band_idx,
	u_char mode)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	/* u_char *sa = NULL; */
	u_char omac_idx = 0;
	struct wifi_dev *wdev = NULL;
	struct _RTMP_CHIP_OP *chip_ops = NULL;
	struct _chip_testmode_chip_info testmode_chip_info;

	sys_ad_zero_mem(&testmode_chip_info,
		sizeof(struct _chip_testmode_chip_info));

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_MAC_INVALID_PAD;

	chip_ops = hc_get_chip_ops(ad->hdev_ctrl);
	if (chip_ops == NULL)
		return SERV_STATUS_HAL_MAC_INVALID_CHIPOPS;

	if (chip_ops->get_testmode_chip_info)
		chip_ops->get_testmode_chip_info(ad, &testmode_chip_info);

	wdev = &ad->ate_wdev;
	omac_idx = HcGetOmacIdx(ad, wdev);

	if (mode) {
		sys_ad_move_mem((u_int8 *)&configs->own_mac[0],
					(u_int8 *)addr,
					SERV_MAC_ADDR_LEN);

		AsicDevInfoUpdate(ad, omac_idx, addr,
				band_idx, TRUE, DEVINFO_ACTIVE_FEATURE);
	} else {
		AsicDevInfoUpdate(ad, omac_idx,
				(u_int8 *)ad->CurrentAddress,
				band_idx, TRUE, DEVINFO_ACTIVE_FEATURE);
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
	"ownmac ID : %d, ownmac addr = %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
		omac_idx,
			configs->own_mac[0],
			configs->own_mac[1],
			configs->own_mac[2],
			configs->own_mac[3],
			configs->own_mac[4],
			configs->own_mac[5]);

	if (testmode_chip_info.u1testmode_sup_mlo == FALSE &&
		chip_ops->testmode_disable_mlo) {

		chip_ops->testmode_disable_mlo(ad);
	}

	return ret;
}

#ifdef TXBF_SUPPORT
s_int32 net_ad_set_bss_info(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char crl_band_idx,
	u_char *pBssid)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	BSS_INFO_ARGUMENT_T bss_info_argument;
	RTMP_ADAPTER *ad = NULL;
	u_char  omac_idx = 0;
	struct wifi_dev *wdev = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	wdev = &ad->ate_wdev;

	sys_ad_move_mem(&bss_info_argument, &wdev->bss_info_argument,
			sizeof(BSS_INFO_ARGUMENT_T));

	omac_idx = HcGetOmacIdx(ad, wdev);

	bss_info_argument.OwnMacIdx = omac_idx;
	bss_info_argument.ucBssIndex = wdev->bss_info_argument.ucBssIndex;
	os_move_mem(bss_info_argument.Bssid, pBssid, MAC_ADDR_LEN);
	bss_info_argument.bmc_wlan_idx = 1;
	bss_info_argument.NetworkType = NETWORK_INFRA;
	bss_info_argument.u4ConnectionType = CONNECTION_INFRA_AP;
	bss_info_argument.CipherSuit = CIPHER_SUIT_NONE;
	bss_info_argument.bss_state = BSS_ACTIVE;
	bss_info_argument.ucBandIdx = crl_band_idx;
	bss_info_argument.u8BssInfoFeature = BSS_INFO_OWN_MAC_FEATURE |
						BSS_INFO_BASIC_FEATURE;

	bss_info_argument.priv = wdev;

	if (AsicBssInfoUpdate(ad, &bss_info_argument) != STATUS_TRUE)
		ret = SERV_STATUS_AGENT_FAIL;

	return ret;
}

s_int32 mt_ad_set_txbf_tx_apply(
	struct test_wlan_info *winfos,
	u_char *arg)
{
	RTMP_ADAPTER *ad = NULL;
	u_char  wlan_idx;
	boolean fg_ebf, fg_ibf, fg_mu, fg_phase_cal;
	s_int32 ret = SERV_STATUS_SUCCESS;


	wlan_idx = arg[0];
	fg_ebf   = arg[1];
	fg_ibf   = arg[2];
	fg_mu    = arg[3];
	fg_phase_cal = arg[4];

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);

	CmdTxBfTxApplyCtrl(ad,
			wlan_idx,
			fg_ebf,
			fg_ibf,
			fg_mu,
			fg_phase_cal);

	return ret;
}


s_int32 net_ad_set_txbf_lna_gain(
	struct test_wlan_info *winfos,
	u_char lna_gain)
{
	RTMP_ADAPTER *ad = NULL;
	s_int32 ret = SERV_STATUS_SUCCESS;
	UINT8	u1BandIdx;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	u1BandIdx = TESTMODE_GET_BAND_IDX(ad);


	ret = CmdTxBfLnaGain(ad, lna_gain, u1BandIdx);

	return ret;
}


s_int32 net_ad_set_ibf_phase_comp(
	struct test_wlan_info *winfos,
	u_char bw, u_char band, u_char dbdc_band_idx, u_char group_idx,
	boolean fg_read_from_e2p, boolean fg_dis_comp)
{
	RTMP_ADAPTER *ad = NULL;
	s_int32 ret = SERV_STATUS_SUCCESS;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);

	ret = CmdITxBfPhaseComp(
				ad,
				bw,
				band,
				dbdc_band_idx,
				group_idx,
				fg_read_from_e2p,
				fg_dis_comp);

	return ret;
}


s_int32 net_ad_set_txbf_profile_tag_read(
	struct test_wlan_info *winfos,
	u_char pf_idx, boolean fg_bfer)
{
	RTMP_ADAPTER *ad = NULL;
	s_int32 ret = SERV_STATUS_SUCCESS;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);

	ret = TxBfProfileTagRead(ad, pf_idx, fg_bfer);

	return ret;
}


s_int32 net_ad_set_txbf_profile_tag_mcs_thrd(
	struct test_wlan_info *winfos,
	u_char *mcs_lss, u_char *mcs_sss)
{
	RTMP_ADAPTER *ad = NULL;
	s_int32 ret = SERV_STATUS_SUCCESS;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);

#ifndef DOT11_HE_AX
	TxBfProfileTag_McsThd(&ad->rPfmuTag2,
				mcs_lss,
				mcs_sss);
#endif
	return ret;
}


s_int32 net_ad_set_sta_rec_bf_update(
	struct test_wlan_info *winfos,
	u_char *arg)
{
	RTMP_ADAPTER *ad = NULL;
	s_int32 ret = SERV_STATUS_SUCCESS;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);

	ret = Set_StaRecBfUpdate(ad, arg);

	return ret;
}


s_int32 net_ad_set_sta_rec_bf_read(
	struct test_wlan_info *winfos,
	u_char *arg)
{
	RTMP_ADAPTER *ad = NULL;
	s_int32 ret = SERV_STATUS_SUCCESS;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);

	ret = Set_StaRecBfRead(ad, arg);

	return ret;
}


s_int32 mt_ad_set_ibf_inst_cal(
	struct test_wlan_info *winfos,
	u_char group_idx,
	u_char group_l_m_h,
	u_char fg_sx2,
	u_char phase_cal,
	u_char phase_lna_gain_level,
	u_char phase_out_revision)
{
	RTMP_ADAPTER *ad = NULL;
	s_int32 ret = SERV_STATUS_SUCCESS;
	UINT8 u1BandIdx;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	u1BandIdx = TESTMODE_GET_BAND_IDX(ad);

	ret = CmdITxBfPhaseCal(ad,
						   group_idx,
						   group_l_m_h,
						   fg_sx2,
						   phase_cal,
						   phase_lna_gain_level,
						   u1BandIdx,
						   phase_out_revision);

	return ret;
}


s_int32 mt_ad_set_txbf_profile_data_write_20m_all(
	struct test_wlan_info *winfos,
	u_char profile_idx,
	u_char *data)
{
	PFMU_HALF_DATA profile_data[64];
	RTMP_ADAPTER *ad = NULL;
	s_int32 ret = SERV_STATUS_SUCCESS;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);

	if (TxBfProfileDataFormatTranslate(ad, data, profile_data) == FALSE)
		ret = SERV_STATUS_AGENT_FAIL;

	if (CmdETxBfPfmuProfileDataWrite20MAll(ad,
				profile_idx,
				(u_char *)&profile_data[0]) !=
				NDIS_STATUS_SUCCESS)
		ret = SERV_STATUS_AGENT_FAIL;
	return ret;
}


s_int32 mt_ad_set_txbf_profile_data_write_20m_all_5x5(
	struct test_wlan_info *winfos,
	u_char profile_idx,
	u_char *data)
{
	struct _PFMU_HALF_DATA_5X5 profile_data[64];
	RTMP_ADAPTER *ad = NULL;
	s_int32 ret = SERV_STATUS_SUCCESS;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);

	if (TxBfProfileDataFormatTranslate_5x5(ad, data, profile_data) == FALSE)
		ret = SERV_STATUS_AGENT_FAIL;

	if (CmdETxBfPfmuProfileDataWrite20MAll_5x5(ad,
				profile_idx,
				(u_char *)&profile_data[0]) !=
				NDIS_STATUS_SUCCESS)
		ret = SERV_STATUS_AGENT_FAIL;
	return ret;
}

#endif /* TXBF_SUPPORT */

s_int32 net_ad_set_low_power(
	struct test_wlan_info *winfos, u_int32 control)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	if (control)
		MlmeLpEnter(ad);
	else
		MlmeLpExit(ad);

	return ret;
}

s_int32 net_ad_read_mac_bbp_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"cr_addr=0x%08x\n",
		regs->cr_addr);

#ifdef WIFI_UNIFIED_COMMAND
	/* WF_PHY_DSP CR via WM */
	if ((regs->cr_addr & 0x86000000) == 0x86000000) {
		RTMP_REG_PAIR dsp_reg;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"dsp_addr=0x%08x\n", regs->cr_addr);
		dsp_reg.Register = regs->cr_addr;
		*regs->cr_val = 0;
		UniCmdMultipleMacRegAccessRead(ad, &dsp_reg, 1);
		*regs->cr_val = dsp_reg.Value;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"dsp_addr=0x%08x, cr_val=0x%08x\n",
			regs->cr_addr, *regs->cr_val);
	} else
#endif
		RTMP_IO_READ32(ad->hdev_ctrl, regs->cr_addr, regs->cr_val);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"cr_addr=0x%08x, cr_val=0x%08x\n",
		regs->cr_addr, *regs->cr_val);

	return ret;
}

s_int32 net_ad_write_mac_bbp_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

#ifdef WIFI_UNIFIED_COMMAND
	/* WF_PHY_DSP CR via WM */
	if ((regs->cr_addr & 0x86000000) == 0x86000000) {
		RTMP_REG_PAIR dsp_reg;

		dsp_reg.Register = regs->cr_addr;
		dsp_reg.Value = *regs->cr_val;

		UniCmdMultipleMacRegAccessWrite(ad, &dsp_reg, 1);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"dsp_addr=0x%08x, val=0x%08x\n",
			 regs->cr_addr, *regs->cr_val);
	} else
#endif
		RTMP_IO_WRITE32(ad->hdev_ctrl, regs->cr_addr, *regs->cr_val);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"cr_addr=0x%08x, cr_val=0x%08x\n",
		regs->cr_addr, *regs->cr_val);

	return ret;
}

s_int32 net_ad_read_bulk_mac_bbp_reg(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	struct test_register *regs)
{
#define REG_BLOCK_SIZE 128
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_int32 reg_seq, addr, reg_total, value = 0;
	u_char offset_byte = 0x4;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	addr = regs->cr_addr;
	reg_total = regs->cr_num;

	for (reg_seq = 0; reg_seq < reg_total; reg_seq++) {
#ifdef WIFI_UNIFIED_COMMAND
		/* WF_PHY_DSP CR via WM */
		if ((regs->cr_addr & 0x86000000) == 0x86000000) {
			RTMP_REG_PAIR dsp_reg;

			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"dsp_addr=0x%08x\n", addr);
			dsp_reg.Register = addr;
			*regs->cr_val = 0;
			UniCmdMultipleMacRegAccessRead(ad, &dsp_reg, 1);
			value = dsp_reg.Value;
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"dsp_addr=0x%08x, val=0x%08x\n",
				 regs->cr_addr, value);
		} else
#endif
			RTMP_IO_READ32(ad->hdev_ctrl,
					addr,
					&value);

		sys_ad_move_mem(regs->cr_val+reg_seq,
				&value, sizeof(value));
		addr += offset_byte;
	}

	return ret;
}

s_int32 net_ad_read_bulk_rf_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_int32 idx, addr, value = 0;
	u_int32 *dst;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	for (idx = 0; idx < regs->cr_num; idx++) {
		addr = regs->cr_addr + (idx << 2);
		dst = regs->cr_val + idx;
#ifndef WIFI_UNIFIED_COMMAND
		ret = MtCmdRFRegAccessRead(ad, regs->wf_sel, addr, &value);
#else
		ret = UniCmdRFRegAccessRead(ad, regs->wf_sel, addr, &value);
#endif
		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"wf_sel=%d, cr_addr=0x%08x, ",
				regs->wf_sel, addr);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"cr_val=0x%08x fail\n", value);
			break;
		}

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"wf_sel=%d, cr_addr=0x%08x, cr_val=0x%08x\n",
			regs->wf_sel, addr, value);

		sys_ad_move_mem(dst, &value, sizeof(value));
	}

	return ret;
}

s_int32 net_ad_write_bulk_rf_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_int32 idx, addr, value;
	u_int32 *src;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	for (idx = 0; idx < regs->cr_num; idx++) {
		addr = regs->cr_addr + (idx << 2);
		src = regs->cr_val + idx;
		sys_ad_move_mem(&value, src, sizeof(value));

#ifndef WIFI_UNIFIED_COMMAND
		ret = MtCmdRFRegAccessWrite(ad, regs->wf_sel, addr, value);
#else
		ret = UniCmdRFRegAccessWrite(ad, regs->wf_sel, addr, value);
#endif
		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"wf_sel=%d, cr_addr=0x%08x, ",
				regs->wf_sel, addr);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"cr_val=0x%08x fail\n", value);
			break;
		}

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"wf_sel=%d, cr_addr=0x%08x, cr_val=0x%08x\n",
			regs->wf_sel, addr, value);
	}

	return ret;
}

void net_ad_read_ca53_reg(struct test_register *regs)
{
	u_long offset;

	regs->cr_addr = (u_long)ioremap(regs->cr_addr, CA53_GPIO_REMAP_SIZE);
	sys_ad_move_mem((u_char *)&offset, (u_char *)&regs->cr_addr,
			sizeof(u_long));
	RTMP_SYS_IO_READ32(offset, regs->cr_val);
	iounmap((void *)offset);
}

void net_ad_write_ca53_reg(struct test_register *regs)
{
	u_long offset;

	regs->cr_addr = (u_long)ioremap(regs->cr_addr, CA53_GPIO_REMAP_SIZE);
	sys_ad_move_mem((u_char *)&offset, (u_char *)&regs->cr_addr,
			sizeof(u_long));
	RTMP_SYS_IO_WRITE32(offset, *regs->cr_val);
	iounmap((void *)offset);
}

s_int32 net_ad_read_write_eeprom(
	struct test_wlan_info *winfos,
	struct test_eeprom *eprms,
	boolean is_read)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_int16 value = 0;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	if (is_read) {
		RT28xx_EEPROM_READ16(ad, eprms->offset, value);
		sys_ad_move_mem(eprms->value, &value, sizeof(value));
	} else
		RT28xx_EEPROM_WRITE16(ad, eprms->offset, *eprms->value);

	return ret;
}

s_int32 net_ad_read_write_bulk_eeprom(
	struct test_wlan_info *winfos,
	struct test_eeprom *eprms,
	boolean is_read)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_int16 value = 0;
	u_int32 offset = 0, length  = 0, eeprom_size = 0, word_seq = 0;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	eeprom_size = winfos->chip_cap.efuse_size;
	offset = eprms->offset;
	length = eprms->length;

	if (is_read) {
		RTMP_OS_NETDEV_STOP_QUEUE(ad->net_dev);
		for (word_seq = 0 ; word_seq < (length >> 1) ; word_seq++) {
			RT28xx_EEPROM_READ16(ad, offset+(word_seq << 1), value);
			eprms->value[word_seq] = value;
		}

		RTMP_OS_NETDEV_START_QUEUE(ad->net_dev);
	} else {
		u_int16 val_seq = 0;

		for (val_seq = 0;
				val_seq < (length >> 1);
				val_seq++) {
			value = eprms->value[val_seq];
			RT28xx_EEPROM_WRITE16(ad,
					offset+(val_seq << 1),
					value);
		}
	}

	return ret;
}

s_int32 net_ad_get_free_efuse_block(
	IN struct test_wlan_info *winfos,
	OUT struct test_eeprom *eprms)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct _RTMP_CHIP_CAP *cap = NULL;
	u_int8 i = 0;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	cap = hc_get_chip_cap(ad->hdev_ctrl);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"\n");

	eFuseGetFreeBlockCount(ad, &eprms->efuse_free_block);

	if (cap->efuse_free_block_ver == EFUSE_FREE_BLOCK_FORMAT_V2) {
		if (cap->efuse_total_bank_num > TEST_FREE_BLOCK_SIZE)
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"error, free buffer %d less than needed %d\n",
				TEST_FREE_BLOCK_SIZE, cap->efuse_total_bank_num);

		eprms->free_block_format_ver = cap->efuse_free_block_ver;
		eprms->efuse_total_bank_num = cap->efuse_total_bank_num;

		for (i = 0; i < cap->efuse_total_bank_num; i++) {
			eprms->free_block_v2[i] = ad->physical_dev->EfuseBlkInfo.blk_info[i].free_block;
			eprms->efuse_num_v2[i] = ad->physical_dev->EfuseBlkInfo.blk_info[i].efuse_num;
			eprms->total_block_v2[i] = ad->physical_dev->EfuseBlkInfo.blk_info[i].total_block;
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"bankIdx:%d Total:%d Free:%d eFuseNum:%u\n",
				i, eprms->total_block_v2[i], eprms->free_block_v2[i],
				 eprms->efuse_num_v2[i]);
		}
	}

	return ret;
}

s_int32 net_ad_w_cali_2_efuse(
	struct test_wlan_info *winfos,
	u_int8 *data)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	rtmp_ee_write_to_efuse(ad);

	return ret;
}

s_int32 net_ad_w_cali_2_bin(
	struct test_wlan_info *winfos,
	u_int8 *data)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	rtmp_ee_write_to_bin(ad);

	return ret;
}

s_int32 net_ad_w_cali_2_i2c_eeprom(
	struct test_wlan_info *winfos,
	u_int8 *data)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	rtmp_ee_write_to_i2c_eeprom(ad);

	return ret;
}

s_int32 net_ad_mps_tx_operation(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	boolean is_start_tx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_mps_cb *mps_cb;
	struct test_mps_setting *mps_setting;

	mps_cb = &configs->mps_cb;
	mps_setting = mps_cb->mps_setting;
	if (!mps_setting || !mps_cb->mps_cnt) {
		ret = SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;
		goto err;
	}

	if (is_start_tx) {
		if (configs->op_mode & OP_MODE_MPS) {
			ret = SERV_STATUS_OSAL_NET_FAIL;
			goto err;
		}

		if (mps_cb->setting_inuse) {
			ret = SERV_STATUS_OSAL_NET_FAIL;
			goto err;
		}

		configs->op_mode |= fTEST_MPS;
		mps_cb->ref_idx = 1;
		mps_cb->setting_inuse = TRUE;
		ret = net_ad_mps_load_setting(winfos, configs);
		if (ret)
			goto err;

		ret = net_ad_mps_dump_setting(configs, 0xFFFF);
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"op_mode=0x%x, inuse=0x%x, setting_addr=%p\n",
			configs->op_mode, mps_cb->setting_inuse,
			mps_setting);

		if (!(configs->op_mode & OP_MODE_MPS)
			&& !mps_cb->setting_inuse) {
			struct test_mps_setting **setting_addr =
						&(mps_cb->mps_setting);
			mps_cb->mps_cnt = 0;
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"before free mem=%p\n",
				mps_setting);
			sys_ad_free_mem(*setting_addr);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"after free mem=%p\n",
				mps_setting);
			*setting_addr = NULL;
		}
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"error, mps_cnt=%d, mps_setting=%p\n",
		mps_cb->mps_cnt, mps_setting);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"error, op_mode=0x%x, setting_inuse=0x%x\n",
		configs->op_mode, mps_cb->setting_inuse);

	return ret;
}

s_int32 net_ad_set_tmr(
	struct test_wlan_info *winfos,
	struct test_tmr_info *tmr_info)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	s_char tmr_setting[8], tmr_hw_version[8];

	INT sRet = 0;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	if (tmr_info->version == TMR_HW_VER_100)
		tmr_info->version = TMR_VER_1_0;
	else if (tmr_info->version == TMR_HW_VER_150)
		tmr_info->version = TMR_VER_1_5;
	else if (tmr_info->version == TMR_HW_VER_200)
		tmr_info->version = TMR_VER_2_0;
	else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"wrong version %d!!\n",
			tmr_info->version);
		return SERV_STATUS_OSAL_NET_INVALID_PARAM;
	}

	sRet = snprintf(tmr_setting, sizeof(tmr_setting), "%d", tmr_info->setting);
	if (os_snprintf_error(sizeof(tmr_setting), sRet))
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR, "snprintf error!\n");
	sRet = snprintf(tmr_hw_version, sizeof(tmr_hw_version), "%d", tmr_info->version);
	if (os_snprintf_error(sizeof(tmr_hw_version), sRet))
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR, "snprintf error!\n");

	ret = TmrUpdateParameter(ad, tmr_info->through_hold, tmr_info->iter);
	if (ret)
		return SERV_STATUS_OSAL_NET_FAIL;
	ret = setTmrVerProc(ad, tmr_hw_version);
	if (ret)
		return SERV_STATUS_OSAL_NET_FAIL;
	ret = setTmrEnableProc(ad, tmr_setting);

	return ret;
}

s_int32 net_ad_get_rxv_cnt(
	IN struct test_wlan_info *winfos,
	IN u_char ctrl_band_idx,
	OUT u_int32 *byte_cnt)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	chip_get_rxv_cnt(ad, ctrl_band_idx, byte_cnt);

	return ret;
}

s_int32 net_ad_get_rxv_content(
	IN struct test_wlan_info *winfos,
	IN u_char ctrl_band_idx,
	OUT void *content)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	chip_get_rxv_content(ad, ctrl_band_idx, content);

	return ret;
}

s_int32 net_ad_rf_test_cb(
	struct test_wlan_info *winfos,
	struct test_log_dump_cb *test_log_dump,
	u_int32 en_log,
	u_int8 *test_result,
	u_int32 length)
{

	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	EXT_EVENT_RF_TEST_RESULT_T *result =
		(EXT_EVENT_RF_TEST_RESULT_T *)test_result;
	EXT_EVENT_RF_TEST_DATA_T *data =
		(EXT_EVENT_RF_TEST_DATA_T *)result->aucEvent;
	struct test_log_dump_cb *log_dump = NULL;
	static u_int32 evt_type;
	static u_int32 recal_type;
	static int total;
	boolean test_done = FALSE;
	struct _RTMP_CHIP_CAP *cap;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
		"\n");

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	if (IS_MT7986(ad) ||
		IS_MT7990(ad) ||
		IS_MT7992(ad) ||
		IS_MT7993(ad))
		cap = hc_get_chip_cap(ad->hdev_ctrl);
	else
		cap = NULL;
	result->u4FuncIndex = le2cpu32(result->u4FuncIndex);
	evt_type = result->u4FuncIndex;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"evt_type %d\n", evt_type);

	switch (evt_type) {
	case RDD_TEST_MODE:
		if (en_log & fTEST_LOG_RDD) {
		struct test_rdd_log unit;
		struct _EVENT_WIFI_RDD_TEST_T *log =
			(struct _EVENT_WIFI_RDD_TEST_T *)test_result;
		u_int64 *data = (u_int64 *)log->aucBuffer;
		u_int32 idx = 0, len = 0;

		log_dump = &test_log_dump[TEST_LOG_RDD - 1];

		log->u4FuncLength = le2cpu32(log->u4FuncLength);
		log->u4Prefix = le2cpu32(log->u4Prefix);
		log->u4Count = le2cpu32(log->u4Count);

		len = (log->u4FuncLength -
			sizeof(struct _EVENT_WIFI_RDD_TEST_T)
			+ sizeof(log->u4FuncIndex)
			+ sizeof(log->u4FuncIndex))>>3;

		sys_ad_zero_mem(&unit, sizeof(unit));
		unit.prefix = log->u4Prefix;
		unit.cnt = log->u4Count;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"log->u4Count %d\n", log->u4Count);

		for (idx = 0; idx < len; idx++) {
			sys_ad_move_mem(unit.buffer, data++, TEST_RDD_LOG_SIZE);
			net_ad_insert_test_log(winfos,
				log_dump,
				(UCHAR *)&unit, fTEST_LOG_RDD,
				sizeof(unit));
			/* byPass is used @logDump,
			 * if the same event, don't dump same message
			 */
			unit.by_pass = TRUE;
		}
	}
	break;

#ifdef INTERNAL_CAPTURE_SUPPORT
	case GET_ICAP_CAPTURE_STATUS:
		break;

	case GET_ICAP_RAW_DATA:
		RTEnqueueInternalCmd(ad,
			CMDTHRED_ICAP_DUMP_RAW_DATA,
			(VOID *)test_result, length);
		break;
#endif/* INTERNAL_CAPTURE_SUPPORT */

#ifdef PHY_ICS_SUPPORT
	case RF_AT_EXT_FUNCID_GET_PHY_ICS_DATA:
		RTEnqueueInternalCmd(ad,
			CMDTHRED_PHY_ICS_DUMP_RAW_DATA,
			(VOID *)test_result, length);
		break;
#endif /* PHY_ICS_SUPPORT */

	case RE_CALIBRATION:
	if (data) {
		struct test_recal_log re_cal;
		s_int32 i = 0;
		u_int32 cal_idx = 0;
		u_int32 cal_type = 0;
		u_int32 len = 0;
		u_int32 *dump_tmp = (u_int32 *)data->aucData;
		u_int32 *cal_log = NULL;
		s_int32 ret = SERV_STATUS_SUCCESS;

		if (IS_MT7915(ad) || IS_MT7916(ad) || IS_MT7986(ad) ||
			IS_MT7981(ad) || IS_MT7990(ad) || IS_MT7992(ad) ||
			IS_MT7993(ad)) {
			ret = sys_ad_alloc_mem((UCHAR **)&cal_log, 6000);
			if (ret) {
						MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
							"allocated memory fail!\n");
					}
		}

		data->u4CalIndex = le2cpu32(data->u4CalIndex);
		data->u4CalType = le2cpu32(data->u4CalType);
		result->u4PayloadLength = le2cpu32(result->u4PayloadLength);
		cal_idx = data->u4CalIndex;
		cal_type = data->u4CalType;
		len = result->u4PayloadLength;
		len = (len - sizeof(EXT_EVENT_RF_TEST_DATA_T)) >> 2;
		log_dump = &test_log_dump[TEST_LOG_RE_CAL - 1];
		/* MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, */
		/* "CalType:%x\n", cal_type); */
		re_cal.cal_idx = cal_idx;
		re_cal.cal_type = cal_type;

		if (total == 0) {
			recal_type = cal_type;
			log_dump->recal_curr_type = recal_type;
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
				"[Recal][%08x][START]\n", recal_type);
		}

		total += result->u4PayloadLength;

		if ((cal_type == CAL_ALL) && (total == CAL_ALL_LEN))
			test_done = TRUE;

		if (((IS_MT7915(ad) || IS_MT7916(ad) || IS_MT7986(ad) ||
			IS_MT7981(ad) || IS_MT7990(ad) || IS_MT7992(ad) ||
			IS_MT7993(ad))) &&
			((cal_type == TX_DNL_CAL) ||
			 (cal_type == TX_TSSI_CAL_2G) ||
			 (cal_type == TX_TSSI_CAL_5G) ||
			 (cal_type == PRE_CAL) ||
			 ((cal_type & TX_DPD_FLATNESS_CAL) ==
			TX_DPD_FLATNESS_CAL) ||
			 (cal_type == RX_GAIN_CAL))) {
			for (i = 0; i < len; i++) {
				dump_tmp[i] = le2cpu32(dump_tmp[i]);
				/* tempory enable for verify*/
				if (cal_type == RX_GAIN_CAL) {
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
					"[Recal][%08x][%08x]\n",
					cal_type, dump_tmp[i]);
				}
				if ((cal_type & TX_DPD_FLATNESS_CAL) == TX_DPD_FLATNESS_CAL) {
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
					"[Recal][%08x][%08x]\n",
					cal_type, dump_tmp[i]);
				}
				re_cal.cr_val = dump_tmp[i];
				cal_log[i] = dump_tmp[i];
			}
		} else {
			for (i = 0; i < len; i++) {
				dump_tmp[i] = le2cpu32(dump_tmp[i]);
				if (i & 0x1) {
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE,
						DBG_LVL_INFO,
						"%08x\n", dump_tmp[i]);
					re_cal.cr_val = dump_tmp[i];
					if (en_log & fTEST_LOG_RE_CAL) {
						net_ad_insert_test_log(winfos,
							log_dump,
							(UCHAR *)&re_cal,
							fTEST_LOG_RE_CAL,
							sizeof(re_cal));
					}
				} else {
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE,
						DBG_LVL_INFO,
						"[Recal][%08x][%08x]",
						cal_type, dump_tmp[i]);
					re_cal.cr_addr = dump_tmp[i];
				}
			}
		}

		if (IS_MT7915(ad) || IS_MT7916(ad) || IS_MT7986(ad) ||
			IS_MT7981(ad) || IS_MT7990(ad) || IS_MT7992(ad) ||
			IS_MT7993(ad)) {
			if (cal_type == TX_DNL_CAL) {
				u_int16 tmp_len = len * sizeof(u_int32);

				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
				"[cal_type][DNL]Ofst = 0x%x, len=%d\n\n",
				ad->DnlCalOfst, tmp_len);

				memcpy(ad->TxDnlCal + ad->DnlCalOfst,
					cal_log, tmp_len);
				ad->DnlCalOfst += tmp_len;
			} else if (cal_type == TX_TSSI_CAL_2G) {
				u_int16 tmp_len = len * sizeof(u_int32);

				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
				"[cal_type][TSSI-2G]Ofst = 0x%x, len=%d\n\n",
				ad->TssiCal2GOfst, tmp_len);

				memcpy(ad->TssiCal2G + ad->TssiCal2GOfst,
					cal_log, tmp_len);
				ad->TssiCal2GOfst += tmp_len;
			} else if (cal_type == TX_TSSI_CAL_5G) {
				u_int16 tmp_len = len * sizeof(u_int32);

				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
				"[cal_type][TSSI-5G]Ofst = 0x%x, len=%d\n\n",
				ad->TssiCal5GOfst, tmp_len);

				memcpy(ad->TssiCal5G + ad->TssiCal5GOfst,
					cal_log, tmp_len);
				ad->TssiCal5GOfst += tmp_len;
			} else if (cal_type == RX_GAIN_CAL) {
				u_int16 tmp_len = len * sizeof(u_int32);

				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
				"[cal_type][RXGainCal]Ofst = 0x%x, len=%d\n\n",
				ad->RXGainCalOfst, tmp_len);

#if defined(MT7990) || defined(MT7992) || defined(MT7993)
				if (IS_MT7990(ad) || IS_MT7992(ad) || IS_MT7993(ad)) {
					/*Ignore idle EEprom */
					if (ad->RXGainCalOfst == 320) { //max data from fw to driver
						u_int16 ofst = RX_GAIN_CAL_EERPOM_6G_WF0_GP10_WF1_GP_13_OFSET + 1;
						u_int16 ofst2 = RX_GAIN_CAL_EERPOM_6G_WF1_GP14_WF3_GP_13_OFSET + 1;
						u_int16 ofst_IDLE = RX_GAIN_CAL_EERPOM_IDLE_OFSET + 1;
						/* ofst 0x1970~ 0x19DF*/
						memcpy(ad->RXGainCal + ad->RXGainCalOfst,
						cal_log, ofst);
						/* ofst 0x1AAF~ 0x1A0F*/
						memcpy(ad->RXGainCal +
						ad->RXGainCalOfst +
						ofst + ofst_IDLE,
						((u_int8 *)cal_log + ofst + ofst_IDLE), ofst2);
					} else {
						memcpy(ad->RXGainCal + ad->RXGainCalOfst,
						cal_log, tmp_len);
					}
				} else
#endif
				memcpy(ad->RXGainCal + ad->RXGainCalOfst,
						cal_log, tmp_len);
				ad->RXGainCalOfst += tmp_len;
				}
//#ifdef PRE_CAL_MT7915_SUPPORT
#if defined(PRE_CAL_MT7915_SUPPORT) || defined(PRE_CAL_MT7986_SUPPORT) || \
	defined(PRE_CAL_MT7916_SUPPORT) || defined(PRE_CAL_MT7981_SUPPORT) || \
	defined(PRE_CAL_MT7990_SUPPORT) || defined(PRE_CAL_MT7992_SUPPORT) || \
	defined(PRE_CAL_MT7993_SUPPORT)
		if (ad->bPreCalMode) {
			if (cal_type == PRE_CAL) {
			u_int16 tmp_len = len * sizeof(u_int32);

			memcpy(ad->PreCalImage + ad->PreCalOfst,
			cal_log, tmp_len);
			ad->PreCalOfst += tmp_len;

			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE,
			DBG_LVL_INFO,
			"[cal_type][GROUP][0x%p] GroupCalOfst=%d,cal_log=%x, len=%d\n",
			ad->PreCalImage, ad->PreCalOfst, *cal_log, tmp_len);

			memcpy(ad->PreCalImageInfo, &(ad->PreCalOfst),
			sizeof(UINT32));
			} else if ((cal_type & TX_DPD_FLATNESS_CAL) ==
			TX_DPD_FLATNESS_CAL) {
			u_int16 tmp_len = len * sizeof(u_int32);
#if defined(PRE_CAL_MT7986_SUPPORT) || defined(PRE_CAL_MT7981_SUPPORT) || \
	defined(PRE_CAL_MT7990_SUPPORT) || defined(PRE_CAL_MT7992_SUPPORT) || \
	defined(PRE_CAL_MT7993_SUPPORT)
			u_int32 tmp_eeprom_ofst = 0;
			struct _prek_ee_info *chip_prek = NULL;

			chip_prek = &(cap->prek_ee_info);

			if (IS_MT7981(ad) ||
				IS_MT7986(ad) ||
				IS_MT7990(ad) ||
				IS_MT7992(ad) ||
				IS_MT7993(ad)) {
				if (cal_type ==
				TX_DPD_FLATNESS_CAL) {
					tmp_eeprom_ofst =
					chip_prek->dpd_flash_offset_g_begin;
				} else if ((cal_type ==
				TX_DPD_FLATNESS_CAL_A5) || (cal_type ==
				TX_DPD_FLATNESS_CAL_A5_MEM)) {
					tmp_eeprom_ofst =
					chip_prek->dpd_flash_offset_a5_begin;
				} else if ((cal_type ==
				TX_DPD_FLATNESS_CAL_A6) || (cal_type ==
				TX_DPD_FLATNESS_CAL_A6_MEM)) {
					tmp_eeprom_ofst =
					chip_prek->dpd_flash_offset_a6_begin;
				}
				memcpy(ad->TxDPDImage +
				tmp_eeprom_ofst + ad->TxDPDOfst,
				cal_log, tmp_len);
				ad->TxDPDOfst += tmp_len;
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE,
				DBG_LVL_INFO,
			"[cal_type][%d][0x%p] TxDPDofset=%x, cal_log=%x, len=%d\n",
			cal_type, ad->TxDPDImage, ad->TxDPDOfst, *cal_log, tmp_len);
			} else
#endif /* defined(PRE_CAL_MT7986_SUPPORT) || defined(PRE_CAL_MT7981_SUPPORT) || */
/* defined(PRE_CAL_MT7990_SUPPORT) || defined(PRE_CAL_MT7992_SUPPORT) || defined(PRE_CAL_MT7993_SUPPORT) */
			{
				memcpy(ad->TxDPDImage + ad->TxDPDOfst,
				cal_log, tmp_len);
				ad->TxDPDOfst += tmp_len;
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE,
				DBG_LVL_INFO,
			"[cal_type][TX_DPD][0x%p] TxDPDofset=%x, len=%d\n",
			ad->TxDPDImage, ad->TxDPDOfst, tmp_len);
			}
			}
		}
#endif
			sys_ad_free_mem(cal_log);
		}
	}
	break;

	case CALIBRATION_BYPASS:
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"No RF Test Event %x Dump\n",
			result->u4FuncIndex);
	break;
	}

	total = 0;
	evt_type = 0;
	recal_type = 0;

	return ret;
}

s_int32 net_ad_insert_test_log(
	struct test_wlan_info *winfos,
	struct test_log_dump_cb *log_cb,
	u_int8 *log,
	u_int32 log_type,
	u_int32 len)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	s_int32 idx = 0;
	u_char is_dumping = 0;

	s_int32 ((*insert_func)
		(struct test_log_dump_entry *entry,
		u_int8 *data,
		u_int32 len)) = NULL;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"\n");

	switch (log_type) {
	case fTEST_LOG_RDD:
		insert_func = net_ad_insert_rdd_log;
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"Unknown log type %08x\n",
			log_type);
			break;
	}

	if (!insert_func)
		goto err0;

	idx = log_cb->idx;
	SERV_OS_SEM_LOCK(&log_cb->lock);
	is_dumping = log_cb->is_dumping;
	SERV_OS_SEM_UNLOCK(&log_cb->lock);

	if (is_dumping)
		goto err1;

	if ((log_cb->idx + 1) == log_cb->len) {
		if (!log_cb->overwritable)
			goto err0;
		else
			log_cb->is_overwritten = TRUE;
	}

	SERV_OS_SEM_LOCK(&log_cb->lock);
	if (idx < log_cb->len) {
		if (!log_cb->entry) {
			SERV_OS_SEM_UNLOCK(&log_cb->lock);
			goto err0;
		}
	} else {
		SERV_OS_SEM_UNLOCK(&log_cb->lock);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"idx is invalid %d.\n", idx);
		return SERV_STATUS_OSAL_NET_INVALID_LEN;
	}

	ret = insert_func(&log_cb->entry[idx], log, len);
	SERV_OS_SEM_UNLOCK(&log_cb->lock);

	if (ret)
		goto err0;

	INC_RING_INDEX(log_cb->idx, log_cb->len);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"idx:%d, log_cb->idx:%d, log_type:%08x\n",
			  idx, log_cb->idx, log_type);
	return ret;

err0:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"[WARN] overwritable:%x, log_type:%08x\n",
			  (log_cb) ? log_cb->overwritable:0xff,
			  log_type);
err1:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"Log dumping\n");
	return SERV_STATUS_OSAL_NET_INVALID_PAD;
}


s_int32 net_ad_insert_rdd_log(
	struct test_log_dump_entry *entry,
	u_int8 *data,
	u_int32 len)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_rdd_log *rdd;
	u_int32 *pulse;

	if (!entry)
		goto err0;

	if (!data)
		goto err0;

	sys_ad_zero_mem(entry, sizeof(*entry));
	entry->log_type = fTEST_LOG_RDD;
	entry->un_dumped = TRUE;

	if (len > sizeof(entry->log.rdd))
		len = sizeof(entry->log.rdd);

	sys_ad_move_mem((UCHAR *)&entry->log.rdd, data, len);

	rdd = &entry->log.rdd;
	pulse = (UINT32 *)rdd->buffer;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"[RDD]0x%08x %08x\n", pulse[0], pulse[1]);

	return ret;

err0:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"NULL entry %p, data %p\n",
		entry, data);
	return SERV_STATUS_OSAL_NET_INVALID_PAD;
}

s_int32 net_ad_set_preamble(
	struct test_wlan_info *winfos,
	boolean preamble)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL) {
		ret = SERV_STATUS_OSAL_NET_INVALID_PAD;
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"NULL adapter\n");
		return ret;
	}

	if (preamble)
		OPSTATUS_SET_FLAG(ad, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	else
		OPSTATUS_CLEAR_FLAG(ad, fOP_STATUS_SHORT_PREAMBLE_INUSED);

	return ret;
}

s_int32 net_ad_get_virtual_dev(
	IN struct test_wlan_info *winfos,
	IN u_int8 band_idx,
	IN u_int8 wmm_idx,
	OUT void **virtual_device)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		ret = SERV_STATUS_OSAL_NET_INVALID_PAD;

	*virtual_device = &ad->ate_wdev;

	return ret;
}

s_int32 net_ad_check_txv(
	IN struct test_wlan_info *winfos,
	IN u_int8 band_idx,
	IN struct test_configuration *configs,
	IN void* virtual_wtbl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct test_configuration *test_config = configs;
	struct _MAC_TABLE_ENTRY *entry = (struct _MAC_TABLE_ENTRY *)virtual_wtbl;
	struct _RTMP_CHIP_DBG *chip_dbg = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL) {
		ret = SERV_STATUS_OSAL_NET_INVALID_PAD;
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"NULL adapter, ret:0x%8x\n", ret);
		return ret;
	}

	chip_dbg = hc_get_chip_dbg(ad->hdev_ctrl);

	if (chip_dbg->check_txv && entry) {
		UINT8 phy_mode = entry->phy_param.phy_mode;
		UINT8 ofdm_rate[8] = {0xb, 0xf, 0xa, 0xe, 0x9, 0xd, 0x8, 0xc};

		chip_dbg->check_txv(ad->hdev_ctrl, "spe idx", entry->phy_param.spe_idx, band_idx);
		chip_dbg->check_txv(ad->hdev_ctrl, "phy mode", entry->phy_param.phy_mode, band_idx);
		chip_dbg->check_txv(ad->hdev_ctrl, "dbw", entry->phy_param.bw, band_idx);
		chip_dbg->check_txv(ad->hdev_ctrl, "ER-106T", entry->phy_param.su_ext_tone, band_idx);
		chip_dbg->check_txv(ad->hdev_ctrl, "DCM", entry->phy_param.dcm, band_idx);

		if (phy_mode == MODE_CCK) {
			if (entry->phy_param.rate > MCS_8)
				chip_dbg->check_txv(ad->hdev_ctrl, "Rate", entry->phy_param.rate+5, band_idx);
			else
				chip_dbg->check_txv(ad->hdev_ctrl, "Rate", entry->phy_param.rate, band_idx);
		} else if (phy_mode == MODE_OFDM && entry->phy_param.rate < ARRAY_SIZE(ofdm_rate)) {
			chip_dbg->check_txv(ad->hdev_ctrl, "Rate", ofdm_rate[entry->phy_param.rate], band_idx);
		} else
			chip_dbg->check_txv(ad->hdev_ctrl, "Rate", entry->phy_param.rate, band_idx);

		if (phy_mode < MODE_HTMIX)
			chip_dbg->check_txv(ad->hdev_ctrl, "NSTS", 0, band_idx);
		else if (phy_mode < MODE_VHT)
			chip_dbg->check_txv(ad->hdev_ctrl, "NSTS", (entry->phy_param.rate >> 3), band_idx);
		else
			chip_dbg->check_txv(ad->hdev_ctrl, "NSTS", entry->phy_param.vht_nss-1, band_idx);

		chip_dbg->check_txv(ad->hdev_ctrl, "HE LTF", entry->phy_param.ltf_type, band_idx);
		chip_dbg->check_txv(ad->hdev_ctrl, "HE GI", entry->phy_param.gi_type, band_idx);
		chip_dbg->check_txv(ad->hdev_ctrl, "stbc", entry->phy_param.stbc, band_idx);
		chip_dbg->check_txv(ad->hdev_ctrl, "FEC Coding", entry->phy_param.ldpc, band_idx);

		if (entry->phy_param.phy_mode > MODE_VHT)
			chip_dbg->check_txv(ad->hdev_ctrl, "Max TPE", test_config->max_pkt_ext, band_idx);
		else
			chip_dbg->check_txv(ad->hdev_ctrl, "Max TPE", 0, band_idx);

		chip_dbg->check_txv(ad->hdev_ctrl, "TX LEN",
				test_config->tx_len + ((phy_mode > MODE_OFDM) ? 8 : 4), band_idx);
				/* 4 bytes of FCS and 4 bytes of AMPDU delimiter*/
	}

	return ret;
}

s_int32 net_ad_get_chip_capability(
	struct test_wlan_info *winfos,
	struct test_capability *chip_cap)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct _RTMP_CHIP_OP *chip_ops = NULL;

	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	chip_ops = hc_get_chip_ops(ad->hdev_ctrl);
	if (chip_ops == NULL)
		return SERV_STATUS_HAL_MAC_INVALID_CHIPOPS;

	if (chip_ops->get_chip_capability)
		ret = chip_ops->get_chip_capability(ad, chip_cap);

	return ret;
}

s_int32 net_ad_get_rf_type_capability(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int32 *tx_ant,
	u_int32 *rx_ant)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	/* the dbdc mode of single phy don't support this func */
	if (IS_MT7915(ad))
		return SERV_STATUS_ENGINE_NOT_SUPPORTED;

	*rx_ant = ad->Antenna.field.RxPath;
	*tx_ant = ad->Antenna.field.TxPath;
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		 "single band rx_ant:%d, tx_ant:%d\n", *rx_ant, *tx_ant);

	return ret;
}

s_int32 net_ad_set_test_mode_dnlk(
	struct test_wlan_info *winfos,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct _ATE_CTRL *ATECtrl = NULL;
	struct _RTMP_CHIP_OP *chip_ops = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	chip_ops = hc_get_chip_ops(ad->hdev_ctrl);
	if (chip_ops == NULL)
		return SERV_STATUS_HAL_MAC_INVALID_CHIPOPS;

	ATECtrl = &(ad->ATECtrl);

	ad->DnlCalOfst = 0;

	if (band_idx == 0)
		ad->TssiCal2GOfst = 0;
	else
		ad->TssiCal5GOfst = 0;

	if (chip_ops->test_mode_dnlk)
		ret = chip_ops->test_mode_dnlk(ad, band_idx);

	ATECtrl->op_mode &= ~fATE_IN_RFTEST;

	return ret;
}

s_int32 net_ad_do_cal_item(
	struct test_wlan_info *winfos,
	u_int32 item,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	if (item == 0) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"Wrong input [%d] ! Check !\n", item);
		return SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
	}

	if (IS_ATE_DBDC(ad) == FALSE) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"MIMO item[0x%x]\n", item);
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"DBDC item[0x%x]\n", item);
	}

	if (item == (1<<25)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"2G DNL+TSSI\n");
		ret = net_ad_set_test_mode_dnlk(winfos, band_idx);
	} else if (item == (1<<26)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"5G DNL+TSSI\n");
		ret = net_ad_set_test_mode_dnlk(winfos, band_idx);
	} else {
		ret = MtCmdDoCalibration(ad, 0x1, item, band_idx);
	}

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;

}

s_int32 net_ad_group_prek(
	struct test_wlan_info *winfos,
	u_char op)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct _RTMP_CHIP_OP *chip_ops = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	chip_ops = hc_get_chip_ops(ad->hdev_ctrl);
	if (chip_ops == NULL)
		return SERV_STATUS_HAL_MAC_INVALID_CHIPOPS;

	if (op >= PREK_GROUP_OP_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"Invalid op\n");
		return SERV_STATUS_HAL_OP_FAIL;
	}

	if (chip_ops->ate_group_prek) {
		ret = chip_ops->ate_group_prek(ad, op);
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"PRE_CAL not enable!\n");
	}

	/* MtATE_Group_Pre_Cal_Store_Proc retrun 1 if success, */
	/* so flip the bit */
	ret = ret ^ 1;
	return ret;
}


s_int32 net_ad_dpd_prek(
	struct test_wlan_info *winfos,
	u_char op)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct _RTMP_CHIP_OP *chip_ops = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	chip_ops = hc_get_chip_ops(ad->hdev_ctrl);
	if (chip_ops == NULL)
		return SERV_STATUS_HAL_MAC_INVALID_CHIPOPS;

	if (op >= PREK_DPD_OP_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"Invalid op\n");
		return SERV_STATUS_HAL_OP_FAIL;
	}

	if (chip_ops->ate_dpd_prek) {
		ret = chip_ops->ate_dpd_prek(ad, op);
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"PRE_CAL not enable!\n");
	}

	/* MtATE_DPD_Cal_Store_Proc retrun 1 if success, so flip the bit */
	ret = ret ^ 1;
	return ret;
}

s_int32 net_ad_set(
	struct test_wlan_info *winfos,
	u_int32 id, u_int32 data)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);

	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	ret = MtCmdSetTestEngine(ad, id, data);
	if (ret)
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"ret: 0x%x, Data: 0x%x\n", ret, data);

	return ret;
}

s_int32 net_ad_get(
	struct test_wlan_info *winfos,
	u_int32 id, u_int32 data, u_int32 *rsp_data)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);

	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	ret = MtCmdGetTestEngine(ad, id, data, rsp_data);
	if (ret)
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"ret: 0x%x, Data: 0x%x\n", ret, data);

	return ret;
}

void *net_ad_multi_inf_get_pAd_by_Band(
	struct test_wlan_info *winfos,
	u_int8 band_idx)
{
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);

	ad = physical_device_get_mac_adapter_by_band(ad->physical_dev, band_idx);

	return ad;
}
