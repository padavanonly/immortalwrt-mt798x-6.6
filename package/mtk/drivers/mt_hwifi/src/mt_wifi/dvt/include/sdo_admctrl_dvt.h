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
    sdo_admctrl_dvt.h

*/

#ifndef _SDO_ADMCTRL_H_
#define _SDO_ADMCTRL_H_

#include "rt_config.h"
#include "framework_dvt.h"
#include <net/ip.h>
#include <linux/wait.h>

#define BAND_NUM RAM_BAND_NUM
#define PKT_TYPE_RED_DVT	6
#define MAX_ADM_DVT_STA 128
#define MAX_ADM_DVT_BITMAP_DW	((MAX_ADM_DVT_STA + 31)/32)
#define MAX_ADM_PKT_LEN 1514
#define DEFAULT_ADM_PKT_LEN 256
#define MAX_ADM_DBG_CNT 32
enum {
	ADM_TEST_CASE_0 = 0,
	ADM_TEST_CASE_1 = 1,
	ADM_TEST_CASE_2 = 2,
	ADM_TEST_CASE_3 = 3,
	ADM_TEST_CASE_4 = 4,
	ADM_TEST_CASE_5 = 5,
	ADM_TEST_CASE_6 = 6,
	ADM_TEST_CASE_7 = 7,
	ADM_TEST_CASE_8 = 8,
	ADM_TEST_CASE_9 = 9,
	ADM_TEST_CASE_10 = 10,
	ADM_TEST_CASE_11 = 11,
	ADM_TEST_CASE_12 = 12,
	ADM_TEST_CASE_13 = 13,
	ADM_TEST_CASE_14 = 14,
	ADM_TEST_CASE_15 = 15,
	ADM_TEST_CASE_16 = 16,
	ADM_TEST_CASE_17 = 17,
	ADM_TEST_CASE_18 = 18,
	ADM_TEST_CASE_19 = 19,
	MAX_ADM_DVT_TEST_CASE
};

enum {
	VHT_HIGH_RATE = 1,
	VHT_LOW_RATE,
	HT_HIGH_RATE,
	HT_LOW_RATE,
	HE_5G_HIGH_RATE,
	HE_5G_LOW_RATE,
	EHT_5G_HIGH_RATE,
	EHT_5G_LOW_RATE,
	HE_2G_HIGH_RATE,
	HE_2G_LOW_RATE,
	EHT_2G_HIGH_RATE,
	EHT_2G_LOW_RATE
};

enum {
	VO_SHORT_HEAVY_PKT = 1,
	VI_SHORT_HEAVY_PKT,
	BE_SHORT_HEAVY_PKT,
	BK_SHORT_HEAVY_PKT,
	VO_LONG_HEAVY_PKT,
	VI_LONG_HEAVY_PKT,
	BE_LONG_HEAVY_PKT,
	BK_LONG_HEAVY_PKT,
	BE_SHORT_LIGHT_PKT,
	BE_LONG_DVT_PKT,
	BE_LONG_DVT_TC4_1_PKT,
	VO_LONG_DVT_TC4_1_PKT,
	BE_SHORT_DVT_PKT,
	BK_SHORT_DVT_PKT,
	BE_LONG_DVT_PRI_PKT,
	VO_LONG_DVT_PKT,
	VI_LONG_DVT_PKT,
	MAX_ADM_DVT_PACKET_SETTING
};

#define MAX_ADM_DVT_PHY_SETTING		8
#define MAX_ADM_DVT_GROUP	4

struct adm_dvt_phy_setting {
	u32 type;
	u32 txmode;
	u8 bw;
	u8 mcs;
	u8 nss;
	//u8 band_idx;
};

struct adm_dvt_packet_setting {
	u32 type;
	u8 qid;
	u8 wmm_set;
	u16 ba_winsize;
	u16 pktlen;
	u32 pktcnt;
	u32 pri_pkt_start;
};

struct adm_dvt_group_setting {
	u8 band_idx;
	u32 phy_type;
	u32 packet_type;
	u32 bitmap[MAX_ADM_DVT_BITMAP_DW];
};

struct adm_dvt_test_case {
	u32 test_case_id;
	u8 group_num;
	int (*check_result)(u32 tc_id);
	struct adm_dvt_group_setting adm_dvt_group[MAX_ADM_DVT_GROUP];
};

struct adm_sta_t {
	struct sk_buff *skb;
	struct dvt_seudo_sta pseudo_sta;
	struct _MAC_TABLE_ENTRY *pEntry;
	struct wifi_dev *wdev;
	u32 packet_cnt;
	u32 pri_packet_start;
	u16 pktlen;
	u16 wcid;
	u8  qid;
	u8  band_idx;
};


struct adm_dvt_var_t {
	struct adm_sta_t adm_sta[MAX_ADM_DVT_STA];
	struct _RTMP_ADAPTER *ad[BAND_NUM];
	u16 pseudo_sta_cnt_per_band[BAND_NUM];
	u16 pseudo_sta_cnt;
	u32 tot_dvt_pkt_cnt;
	u32 txdone_pkt_cnt;
	u32 curr_tc_id;
	wait_queue_head_t kthread_q;
	bool kthread_running;
	u32 timer_cnt;
	u32 pktcnt;
	u32 ple_tot_usage;
	u32 collect_data;
	u32 ple_pause;
	u32 timeout_cnt;
	u32 ple_usage_level_cnt[40];
	u32 dbgcnt[MAX_ADM_DBG_CNT];
};

extern struct adm_dvt_var_t adm_dvt_var;
extern int adm_dvt_thread(void *data);
extern struct task_struct *adm_dvt_task;
extern int adm_sta_packet_cnt[MAX_ADM_DVT_STA][4];
extern int adm_sta_drop_cnt[MAX_ADM_DVT_STA][4];


int adm_dvt_test_init(u32 tc_id);
int adm_dvt_test_deinit(void);
int adm_dvt_destroy_starec(u16 idx, u8 band_idx);
int adm_dvt_create_starec(u16 idx, u8 band_idx);
int adm_dvt_set_rate(u16 idx, struct adm_dvt_phy_setting *phy_setting);
int adm_dvt_set_packet(u16 idx, struct adm_dvt_packet_setting *packet_setting);
int adm_dvt_update_starec(u16 idx, u8 band_idx);
INT set_adm_dvt_cmd(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
int adm_dvt_timer_init(struct _RTMP_ADAPTER *ad);
int adm_dvt_test_start(void);
int adm_dvt_thread(void *data);
int fp_adm_dvt_tx_pkt(struct _RTMP_ADAPTER *ad, NDIS_PACKET *pkt, u16 idx);
VOID adm_dvt_ple_usage_dump(struct _RTMP_ADAPTER *ad, bool reset);
int adm_dvt_timer_init(struct _RTMP_ADAPTER *ad);
int adm_dvt_timer_deinit(struct _RTMP_ADAPTER *ad);
int adm_dvt_tx_status(void *param);
#endif /* _SDO_ADMCTRL_H_ */



