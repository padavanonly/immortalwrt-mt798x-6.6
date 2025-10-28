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
/***************************************************************************
 ***************************************************************************

*/

#ifndef __PEAK_ADJUST_H__
#define __PEAK_ADJUST_H__

#ifdef PEAK_ENHANCE
#define PEAK_TP_AVG_TX_PKT_LEN 700
#define PEAK_TP_AVG_RX_PKT_LEN 1000
#define PEAK_TP_WO_REPORT_TIME 0x6
#define PEAK_TP_TX_COUNT_TH 100000
#define PEAK_TP_MAX_BAND_NUM (CFG_WIFI_RAM_BAND_NUM+1)

#define UL_AVG_TX_PKT_LEN 256
#define UL_AVG_RX_PKT_LEN 1000
#define LEGACY_TPUT_THRESHOLD 500

#define ENABLE_TXOP_ADJUST 0x01
#define ENABLE_AIFS_ADJUST 0x02
#define ENABLE_MLO_ENTRY_ADJUST_ONLY 0x80

struct peak_enhance_ctrl {
	u8 enable_adjust;
	u8 aifsn;
	u16 txop;
	u32 avg_rx_pkt_len;
	u32 avg_tx_pkt_len;
	u32 rx_high_bound;
	u32 rx_low_bound;
	u32 sim_avg_rx_pkt_len;
	u32 sim_avg_tx_pkt_len;
	u32 tx_count_th[PEAK_TP_MAX_BAND_NUM];
	u32 sim_tx_count[PEAK_TP_MAX_BAND_NUM];
};

struct peak_rx_enhance_ctrl {
	u8 enable;
#ifdef WHNAT_SUPPORT
	u8 rx_drop;
#endif /* WHNAT_SUPPORT */
};
#endif /* PEAK_ENHANCE */

#endif /* __PEAK_ADJUST_H__ */
