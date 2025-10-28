/*
 * Copyright (c) [2021], MediaTek Inc. All rights reserved.
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

#include "rt_config.h"
#ifdef WIFI_UNIFIED_COMMAND
#include "sdo_admctrl_dvt.h"

struct adm_dvt_phy_setting adm_dvt_phy[MAX_ADM_DVT_PHY_SETTING] = {
	{VHT_HIGH_RATE,		MODE_VHT,	2, 9, 4},
	{VHT_LOW_RATE,		MODE_VHT,	1, 2, 1},
	{HT_HIGH_RATE,		MODE_HTMIX,	1, 7, 4},
	{HT_LOW_RATE,		MODE_HTMIX,	0, 2, 1},
	{HE_5G_HIGH_RATE,	MODE_HE,	2, 11, 4},
	{HE_5G_LOW_RATE,	MODE_HE,	1, 2, 1},
	{EHT_5G_HIGH_RATE,	MODE_EHT,	3, 13, 4},
	{EHT_5G_LOW_RATE,	MODE_EHT,	0, 2, 1}
};

struct adm_dvt_packet_setting
adm_dvt_packet[MAX_ADM_DVT_PACKET_SETTING] = {
	{VO_SHORT_HEAVY_PKT,	QID_AC_VO,	0, 256, 512, 5000, 0},
	{VI_SHORT_HEAVY_PKT,	QID_AC_VI,	0, 256, 512, 5000, 0},
	{BE_SHORT_HEAVY_PKT,	QID_AC_BE,	0, 256, 512, 5000, 0},
	{BK_SHORT_HEAVY_PKT,	QID_AC_BK,	0, 256, 512, 5000, 0},
	{VO_LONG_HEAVY_PKT,	QID_AC_VO,	0, 256, 1510, 3000*10000, 0},
	{VI_LONG_HEAVY_PKT,	QID_AC_VI,	0, 256, 1510, 3000*10000, 0},
	{BE_LONG_HEAVY_PKT,	QID_AC_BE,	0, 256, 1510, 3000*10000, 0},
	{BK_LONG_HEAVY_PKT,	QID_AC_BK,	0, 256, 1510, 3000*10000, 0},
	{BE_SHORT_LIGHT_PKT,	QID_AC_BE,	0, 64, 1510, 300, 0},
	{BE_LONG_DVT_PKT,	QID_AC_BE,	0, 64, 1510, 160+100, 0},
	{BE_LONG_DVT_TC4_1_PKT,	QID_AC_BE,	0, 64, 1510, 100+100, 0},
	{VO_LONG_DVT_TC4_1_PKT,	QID_AC_VO,	0, 64, 1510, 200, 0},
	{BE_SHORT_DVT_PKT,	QID_AC_BE,	0, 64, 64, 260, 0},
	{BE_LONG_DVT_PRI_PKT,	QID_AC_BE,	0, 64, 1510, 260+40, 260},
	{VO_LONG_DVT_PKT,	QID_AC_VO,	0, 64, 1510, 260, 0},
	{VI_LONG_DVT_PKT,	QID_AC_VI,	0, 64, 1510, 260, 0},
};

static int adm_dvt_check_result_tc(u32 tc_id);

struct adm_dvt_test_case adm_dvt_tc[MAX_ADM_DVT_TEST_CASE] = {
	{
		ADM_TEST_CASE_0, 1,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE, VO_SHORT_HEAVY_PKT, {0x1, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} } }
	},
	{
		ADM_TEST_CASE_1, 2,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE,	VO_LONG_HEAVY_PKT, {0xf, 0, 0, 0} },
		{1, HT_LOW_RATE,	VO_LONG_HEAVY_PKT, {0xf0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} } }
	},
	{
		ADM_TEST_CASE_2, 2,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE,	VO_LONG_DVT_TC4_1_PKT, {0x1, 0, 0, 0} },
		{1, VHT_HIGH_RATE,	VO_LONG_DVT_TC4_1_PKT, {0x2, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} } }
	},
	{
		ADM_TEST_CASE_3, 2,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE,	VO_SHORT_HEAVY_PKT, {0x1, 0, 0, 0} },
		{1, HT_HIGH_RATE,	VO_SHORT_HEAVY_PKT, {0x2, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} }, {0, 0, 0, {0, 0, 0, 0} } }
	},
	{
		ADM_TEST_CASE_4, 2,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE,	VI_SHORT_HEAVY_PKT, {0x3, 0, 0, 0} },
		{1, HT_HIGH_RATE,	VI_SHORT_HEAVY_PKT, {0xc, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} }, {0, 0, 0, {0, 0, 0, 0} } }
	},

	{
		ADM_TEST_CASE_5, 1,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE, BE_LONG_HEAVY_PKT, {0xf, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} } }
	},
	{
		ADM_TEST_CASE_6, 1,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE, BE_SHORT_LIGHT_PKT, {0xffffffff, 0xffffffff, 0x0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} } }
	},
	{
		ADM_TEST_CASE_7, 2,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE,	VO_SHORT_HEAVY_PKT, {0x7, 0, 0, 0} },
		{1, HT_HIGH_RATE,	VO_SHORT_HEAVY_PKT, {0x38, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} }, {0, 0, 0, {0, 0, 0, 0} } }
	},
	{
		ADM_TEST_CASE_8, 2,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE,	VI_SHORT_HEAVY_PKT, {0x1, 0, 0, 0} },
		{1, HT_HIGH_RATE,	VI_SHORT_HEAVY_PKT, {0x2, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} }, {0, 0, 0, {0, 0, 0, 0} } }
	},
	{
		ADM_TEST_CASE_9, 4,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE, VO_LONG_HEAVY_PKT, {0x1, 0, 0, 0} },
		{0, VHT_HIGH_RATE, VI_LONG_HEAVY_PKT, {0x2, 0, 0, 0} },
		{0, VHT_HIGH_RATE, BE_LONG_HEAVY_PKT, {0x4, 0, 0, 0} },
		{0, VHT_HIGH_RATE, BK_LONG_HEAVY_PKT, {0x8, 0, 0, 0} },
		}
	},
	{
		ADM_TEST_CASE_10, 1,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE, VI_LONG_HEAVY_PKT, {0x3, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} } }
	},

	{
		ADM_TEST_CASE_11, 1,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE, BE_LONG_DVT_PKT, {0x1, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} } }
	},

	{
		ADM_TEST_CASE_12, 1,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE, BE_LONG_DVT_TC4_1_PKT, {0x1, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} } }
	},

	{
		ADM_TEST_CASE_13, 2,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE, BE_LONG_DVT_TC4_1_PKT, {0x1, 0, 0, 0} },
		{0, VHT_HIGH_RATE, VO_LONG_DVT_TC4_1_PKT, {0x2, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} } },
	},
	{
		ADM_TEST_CASE_14, 2,
		adm_dvt_check_result_tc,
		{{0, EHT_5G_HIGH_RATE,	BE_SHORT_DVT_PKT, {0x1, 0, 0, 0} },
		{1, EHT_5G_HIGH_RATE,	BE_SHORT_DVT_PKT, {0x2, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} }, {0, 0, 0, {0, 0, 0, 0} } }
	},
	{
		ADM_TEST_CASE_15, 1,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE, BE_LONG_DVT_PRI_PKT, {0x1, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} } }
	},
	{
		ADM_TEST_CASE_16, 1,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE, BE_SHORT_DVT_PKT, {0x1, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} } }
	},
	{
		ADM_TEST_CASE_17, 1,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE, VO_LONG_DVT_PKT, {0x1, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} } }
	},
	{
		ADM_TEST_CASE_18, 1,
		adm_dvt_check_result_tc,
		{{0, VHT_HIGH_RATE, VI_LONG_DVT_PKT, {0x1, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} } }
	},
	{
		ADM_TEST_CASE_19, 1,
		adm_dvt_check_result_tc,
		{{0, EHT_5G_HIGH_RATE, BE_LONG_DVT_PKT, {0x1, 0, 0, 0} },
		{0, EHT_5G_HIGH_RATE, BE_LONG_DVT_PKT, {0x1, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} },
		{0, 0, 0, {0, 0, 0, 0} } }
	},

};

struct adm_dvt_phy_setting *search_phy_setting(u32 type)
{
	int i;
	struct adm_dvt_phy_setting *phy_setting = NULL;

	for (i = 0 ; i < MAX_ADM_DVT_PHY_SETTING; i++) {
		if (type != adm_dvt_phy[i].type)
			continue;
		phy_setting = &adm_dvt_phy[i];
		break;
	}

	return phy_setting;
}


struct adm_dvt_packet_setting *search_packet_setting(u32 type)
{
	int i;
	struct adm_dvt_packet_setting *packet_setting = NULL;

	for (i = 0 ; i < MAX_ADM_DVT_PACKET_SETTING; i++) {
		if (type != adm_dvt_packet[i].type)
			continue;
		packet_setting = &adm_dvt_packet[i];
		break;
	}

	return packet_setting;
}

int adm_dvt_phy_config(u16 idx, u32 type)
{
	int ret = 1;
	struct adm_dvt_phy_setting *phy_setting;

	phy_setting = search_phy_setting(type);
	ret = adm_dvt_set_rate(idx, phy_setting);

	return ret;
}

int adm_dvt_packet_config(u16 idx, u32 type)
{
	int ret = 1;
	struct adm_dvt_packet_setting *packet_setting;

	packet_setting = search_packet_setting(type);
	ret = adm_dvt_set_packet(idx, packet_setting);

	return ret;
}

int adm_dvt_create_sta(u32 tc_id)
{
	u8 group, i, j;
	u8 band_idx;
	u8 group_num;
	u16 idx = 0;
	u32 *bitmap;
	int ret = 1;

	idx = 0;
	group_num = adm_dvt_tc[tc_id].group_num;

	for (group = 0; group < group_num; group++) {
		bitmap = &adm_dvt_tc[tc_id].adm_dvt_group[group].bitmap[0];
		for (i = 0; i < MAX_ADM_DVT_BITMAP_DW; i++) {
			for (j = 0; j < 32; j++) {
				if (!(bitmap[i] & (1 << j)))
					continue;

				idx = (i << 2) + j;
				band_idx = adm_dvt_tc[tc_id].adm_dvt_group[group].band_idx;
				ret = adm_dvt_create_starec(idx, band_idx);
				if (ret < 0)
					break;

				ret = adm_dvt_phy_config(idx, adm_dvt_tc[tc_id].adm_dvt_group[group].phy_type);
				if (ret < 0)
					break;

				ret = adm_dvt_update_starec(idx, band_idx);
				if (ret < 0)
					break;

				ret = adm_dvt_packet_config(idx, adm_dvt_tc[tc_id].adm_dvt_group[group].packet_type);
				if (ret < 0)
					break;
				}
		}
	}


	return ret;
}

int adm_dvt_test_deinit(void)
{
	int i;
	struct adm_sta_t *adm_sta;
	struct _RTMP_ADAPTER *ad;
	/* unsigned int value[3] = {0};*/

	ad = adm_dvt_var.ad[0];

	if (adm_dvt_var.kthread_running)
		adm_dvt_var.kthread_running = FALSE;

	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
		"adm_dvt_task=%p, stop\n", adm_dvt_task);

	adm_dvt_check_result_tc(adm_dvt_var.curr_tc_id);
/*
	HW_IO_READ32(ad->hdev_ctrl, 0x820C03a8, &value[0]);
	HW_IO_READ32(ad->hdev_ctrl, 0x820C0470, &value[1]);
	HW_IO_READ32(ad->hdev_ctrl, 0x820C0478, &value[2]);

	MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"%s: [%08X]=%08X,[%08X]=%08X,[%08X]=%08X\n", __func__,
	0x820C0470,value[1], 0x820C0478,value[2], 0x820C03a8, value[0]);
*/
	for (i = 0; i < MAX_ADM_DVT_STA; i++) {
		adm_sta = &adm_dvt_var.adm_sta[i];
		if (adm_sta->skb)
			kfree_skb(adm_sta->skb);
		adm_dvt_destroy_starec(i, 0);
	}

	adm_dvt_var.curr_tc_id = 0;

	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
		"release pseudo allocated skb\n");

	return 1;
}

int adm_dvt_test_init(u32 tc_id)
{
	int ret = 1;
	struct _RTMP_ADAPTER *ad;

	ad = adm_dvt_var.ad[0];

	adm_dvt_task = NULL;

	ret = adm_dvt_create_sta(tc_id);
	if (ret == 0)
		goto error;

	adm_dvt_var.curr_tc_id = tc_id;
	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR, "tc:%u\n", tc_id);
error:
	return 1;
}

int adm_dvt_test_start(void)
{
	struct _RTMP_ADAPTER *ad;

	ad = adm_dvt_var.ad[0];

	init_waitqueue_head(&(adm_dvt_var.kthread_q));
	adm_dvt_var.kthread_running = TRUE;
	adm_dvt_task = kthread_run(adm_dvt_thread, &adm_dvt_var, "adm_dvt");
	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
		"adm_dvt_task=%p ple_pause=%u\n", adm_dvt_task, adm_dvt_var.ple_pause);
	return 1;
}

static int adm_dvt_check_result_tc(u32 tc_id)
{
	int sta_cnt, group_sta_cnt;
	int i, j, idx, group;
	u32 packet_type;
	u16 wcid;
	u8 qid;
	struct adm_dvt_packet_setting *packet_setting;
	struct adm_sta_t *adm_sta;
	u32 *bitmap;
	u8 group_num;
	struct _RTMP_ADAPTER *ad;

	ad = adm_dvt_var.ad[0];

	idx = 0;
	sta_cnt = 0;
	group_num = adm_dvt_tc[tc_id].group_num;

	for (group = 0; group < group_num; group++) {
		packet_type = adm_dvt_tc[tc_id].adm_dvt_group[group].packet_type;
		packet_setting = search_packet_setting(packet_type);
		qid = packet_setting->qid;
		group_sta_cnt = 0;
		bitmap = &adm_dvt_tc[tc_id].adm_dvt_group[group].bitmap[0];
		for (i = 0; i < MAX_ADM_DVT_BITMAP_DW; i++) {
			for (j = 0; j < 32; j++) {
				if (!(bitmap[i] & (1 << j)))
					continue;
				idx = (i << 2) + j;
				adm_sta = &adm_dvt_var.adm_sta[idx];
				wcid = adm_sta->wcid;

				MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
					"TC%u group%u STA%u idx:%u QID%u pktcnt:%u[%u],drop:%u\n",
					tc_id, group, wcid, idx, qid,
					adm_sta_packet_cnt[wcid][qid%4],
					packet_setting->pktcnt,
					adm_sta_drop_cnt[wcid][qid%4]);
				group_sta_cnt++;
			}
		}
		sta_cnt += group_sta_cnt;
	}

	return 1;
}
#endif /* WIFI_UNIFIED_COMMAND */
