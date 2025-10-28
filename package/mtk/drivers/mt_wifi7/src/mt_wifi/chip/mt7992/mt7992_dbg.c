/*
 * Copyright (c) [2022], MediaTek Inc. All rights reserved.
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

#ifdef MT7992

#include "rt_config.h"
#include "chip/mt7992_cr.h"
#include "hdev/hdev.h"
#include "mac_mt/bmac/mt_bmac.h"

#define EXP_DUMP_PUTTING_DONE 2
#define CURRENT_PC_DUMP_COUNT 1 //for fw_dbg_info dump
#define CMD_TIMEOUT_CURRENT_PC_DUMP_COUNT 10 //for cmd_timeout_info


#if (KERNEL_VERSION(4, 1, 0) > LINUX_VERSION_CODE)
#define FILE_WRITE_METHOD(file_w, msg, size) \
	do { \
		if (file_w->f_op->write) \
			file_w->f_op->write(file_w, msg, size, &file_w->f_pos); \
	else \
		MTWF_PRINT("no file write method\n"); \
	} while (0)
#elif (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
#define FILE_WRITE_METHOD(file_w, msg, size) \
	kernel_write(file_w, msg, size, &file_w->f_pos)
#else
#define FILE_WRITE_METHOD(file_w, msg, size) \
	__vfs_write(file_w, msg, size, &file_w->f_pos)
#endif

static struct bmac_empty_queue_info ple_queue_empty_info[] = {
	{"CPU Q0",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_0, 0},
	{"CPU Q1",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_1, 0},
	{"CPU Q2",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_2, 0},
	{"CPU Q3",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_3, 0},
	{"ALTX Q0", ENUM_UMAC_LMAC_PORT_2,    0x10, 0},
	{"BMC Q0",  ENUM_UMAC_LMAC_PORT_2,    0x11, 0},
	{"BCN Q0",  ENUM_UMAC_LMAC_PORT_2,    0x12, 0},
	{"PSMP Q0", ENUM_UMAC_LMAC_PORT_2,    0x13, 0},
	{"ALTX Q1", ENUM_UMAC_LMAC_PORT_2,    0x10, 1},
	{"BMC Q1",  ENUM_UMAC_LMAC_PORT_2,    0x11, 1},
	{"BCN Q1",  ENUM_UMAC_LMAC_PORT_2,    0x12, 1},
	{"PSMP Q1", ENUM_UMAC_LMAC_PORT_2,    0x13, 1},
	{"ALTX Q2", ENUM_UMAC_LMAC_PORT_2,    0x10, 2},
	{"BMC Q2",  ENUM_UMAC_LMAC_PORT_2,    0x11, 2},
	{"BCN Q2",  ENUM_UMAC_LMAC_PORT_2,    0x12, 2},
	{"PSMP Q2", ENUM_UMAC_LMAC_PORT_2,    0x13, 2},
	{"NAF Q",   ENUM_UMAC_LMAC_PORT_2,    0x18, 0},
	{"NBCN Q",  ENUM_UMAC_LMAC_PORT_2,    0x19, 0},
	{NULL, 0, 0, 0}, {NULL, 0, 0, 0}, /* 18, 19 not defined */
	{"FIXFID Q", ENUM_UMAC_LMAC_PORT_2, 0x1a, 0},
	{NULL, 0, 0, 0}, {NULL, 0, 0, 0}, {NULL, 0, 0, 0}, {NULL, 0, 0, 0}, {NULL, 0, 0, 0},
	{NULL, 0, 0, 0}, {NULL, 0, 0, 0},
	{"RLS4 Q",   ENUM_PLE_CTRL_PSE_PORT_3, 0x7c, 0},
	{"RLS3 Q",   ENUM_PLE_CTRL_PSE_PORT_3, 0x7d, 0},
	{"RLS2 Q",   ENUM_PLE_CTRL_PSE_PORT_3, 0x7e, 0},
	{"RLS Q",  ENUM_PLE_CTRL_PSE_PORT_3, 0x7f, 0}
};

static EMPTY_QUEUE_INFO_T ple_txcmd_queue_empty_info[2][32] = {
	{{"AC00Q", ENUM_UMAC_LMAC_PORT_2, 0x40},
	{"AC01Q", ENUM_UMAC_LMAC_PORT_2, 0x41},
	{"AC02Q", ENUM_UMAC_LMAC_PORT_2, 0x42},
	{"AC03Q", ENUM_UMAC_LMAC_PORT_2, 0x43},
	{"AC10Q", ENUM_UMAC_LMAC_PORT_2, 0x44},
	{"AC11Q", ENUM_UMAC_LMAC_PORT_2, 0x45},
	{"AC12Q", ENUM_UMAC_LMAC_PORT_2, 0x46},
	{"AC13Q", ENUM_UMAC_LMAC_PORT_2, 0x47},
	{"AC20Q", ENUM_UMAC_LMAC_PORT_2, 0x48},
	{"AC21Q", ENUM_UMAC_LMAC_PORT_2, 0x49},
	{"AC22Q", ENUM_UMAC_LMAC_PORT_2, 0x4a},
	{"AC23Q", ENUM_UMAC_LMAC_PORT_2, 0x4b},
	{"AC30Q", ENUM_UMAC_LMAC_PORT_2, 0x4c},
	{"AC31Q", ENUM_UMAC_LMAC_PORT_2, 0x4d},
	{"AC32Q", ENUM_UMAC_LMAC_PORT_2, 0x4e},
	{"AC33Q", ENUM_UMAC_LMAC_PORT_2, 0x4f},
	{"ALTX Q0", ENUM_UMAC_LMAC_PORT_2, 0x70},
	{"TF Q0", ENUM_UMAC_LMAC_PORT_2, 0x71},
	{"TWT TSF-TF Q0", ENUM_UMAC_LMAC_PORT_2, 0x72},
	{"TWT DL Q0", ENUM_UMAC_LMAC_PORT_2, 0x73},
	{"TWT UL Q0", ENUM_UMAC_LMAC_PORT_2, 0x74},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},},

	{{"AC00Q", ENUM_UMAC_LMAC_PORT_2, 0x50},
	{"AC01Q", ENUM_UMAC_LMAC_PORT_2, 0x51},
	{"AC02Q", ENUM_UMAC_LMAC_PORT_2, 0x52},
	{"AC03Q", ENUM_UMAC_LMAC_PORT_2, 0x53},
	{"AC10Q", ENUM_UMAC_LMAC_PORT_2, 0x54},
	{"AC11Q", ENUM_UMAC_LMAC_PORT_2, 0x55},
	{"AC12Q", ENUM_UMAC_LMAC_PORT_2, 0x56},
	{"AC13Q", ENUM_UMAC_LMAC_PORT_2, 0x57},
	{"AC20Q", ENUM_UMAC_LMAC_PORT_2, 0x58},
	{"AC21Q", ENUM_UMAC_LMAC_PORT_2, 0x59},
	{"AC22Q", ENUM_UMAC_LMAC_PORT_2, 0x5a},
	{"AC23Q", ENUM_UMAC_LMAC_PORT_2, 0x5b},
	{"AC30Q", ENUM_UMAC_LMAC_PORT_2, 0x5c},
	{"AC31Q", ENUM_UMAC_LMAC_PORT_2, 0x5d},
	{"AC32Q", ENUM_UMAC_LMAC_PORT_2, 0x5e},
	{"AC33Q", ENUM_UMAC_LMAC_PORT_2, 0x5f},
	{"ALTX Q0", ENUM_UMAC_LMAC_PORT_2, 0x75},
	{"TF Q0", ENUM_UMAC_LMAC_PORT_2, 0x76},
	{"TWT TSF-TF Q0", ENUM_UMAC_LMAC_PORT_2, 0x77},
	{"TWT DL Q0", ENUM_UMAC_LMAC_PORT_2, 0x78},
	{"TWT UL Q0", ENUM_UMAC_LMAC_PORT_2, 0x79},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},}
};


static EMPTY_QUEUE_INFO_T pse_queue_empty_info[] = {
	{"CPU Q0",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_0},
	{"CPU Q1",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_1},
	{"CPU Q2",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_2},
	{"CPU Q3",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_3},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, /* 4~7 not defined */
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},
	{NULL, 0, 0}, {NULL, 0, 0},  /* 14~15 not defined */
	{"LMAC Q",  ENUM_UMAC_LMAC_PORT_2,    0},
	{"MDP TX Q0", ENUM_UMAC_LMAC_PORT_2, 1},
	{"MDP RX Q", ENUM_UMAC_LMAC_PORT_2, 2},
	{"SEC TX Q0", ENUM_UMAC_LMAC_PORT_2, 3},
	{"SEC RX Q", ENUM_UMAC_LMAC_PORT_2, 4},
	{"SFD_PARK Q", ENUM_UMAC_LMAC_PORT_2, 5},
	{"MDP_TXIOC Q0", ENUM_UMAC_LMAC_PORT_2, 6},
	{"MDP_RXIOC Q0", ENUM_UMAC_LMAC_PORT_2, 7},
	{"MDP TX Q1", ENUM_UMAC_LMAC_PORT_2, 0x11},
	{"SEC TX Q1", ENUM_UMAC_LMAC_PORT_2, 0x13},
	{"MDP_TXIOC Q1", ENUM_UMAC_LMAC_PORT_2, 0x16},
	{"MDP_RXIOC Q1", ENUM_UMAC_LMAC_PORT_2, 0x17},
	{"CPU Q3",  ENUM_UMAC_CPU_PORT_1,     4},
	{NULL, 0, 0}, {NULL, 0, 0},
	{"RLS Q",  ENUM_PLE_CTRL_PSE_PORT_3, ENUM_UMAC_PLE_CTRL_P3_Q_0X1F}
};

static EMPTY_QUEUE_INFO_T pse_queue_empty2_info[] = {
	{"MDP_TDPIOC Q0", ENUM_UMAC_LMAC_PORT_2, 0x8},
	{"MDP_RDPIOC Q0", ENUM_UMAC_LMAC_PORT_2, 0x9},
	{"MDP_TDPIOC Q1", ENUM_UMAC_LMAC_PORT_2, 0x18},
	{"MDP_RDPIOC Q1", ENUM_UMAC_LMAC_PORT_2, 0x19},
	{"MDP_TDPIOC Q2", ENUM_UMAC_LMAC_PORT_2, 0x28},
	{"MDP_RDPIOC Q2", ENUM_UMAC_LMAC_PORT_2, 0x29},
	{NULL, 0, 0},
	{"MDP_RDPIOC Q3", ENUM_UMAC_LMAC_PORT_2, 0x39},
	{"MDP TX Q2", ENUM_UMAC_LMAC_PORT_2, 0x21},
	{"SEC TX Q2", ENUM_UMAC_LMAC_PORT_2, 0x23},
	{"MDP_TXIOC Q2", ENUM_UMAC_LMAC_PORT_2, 0x26},
	{"MDP_RXIOC Q2", ENUM_UMAC_LMAC_PORT_2, 0x27},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},
	{"MDP_RXIOC Q3", ENUM_UMAC_LMAC_PORT_2, 0x37},
	{"HIF Q0", ENUM_UMAC_HIF_PORT_0,    0},
	{"HIF Q1", ENUM_UMAC_HIF_PORT_0,    1},
	{"HIF Q2", ENUM_UMAC_HIF_PORT_0,    2},
	{"HIF Q3", ENUM_UMAC_HIF_PORT_0,    3},
	{"HIF Q4", ENUM_UMAC_HIF_PORT_0,    4},
	{"HIF Q5", ENUM_UMAC_HIF_PORT_0,    5},
	{"HIF Q6", ENUM_UMAC_HIF_PORT_0,    6},
	{"HIF Q7", ENUM_UMAC_HIF_PORT_0,    7},
	{"HIF Q8", ENUM_UMAC_HIF_PORT_0,    8},
	{"HIF Q9", ENUM_UMAC_HIF_PORT_0,    9},
	{"HIF Q10", ENUM_UMAC_HIF_PORT_0,    10},
	{"HIF Q11", ENUM_UMAC_HIF_PORT_0,    11},
	{"HIF Q12", ENUM_UMAC_HIF_PORT_0,    12},
	{"HIF Q13", ENUM_UMAC_HIF_PORT_0,    13},
	{NULL, 0, 0}, {NULL, 0, 0}
};


static PCHAR sta_ctrl_reg[] = {"ENABLE", "DISABLE", "PAUSE", "TWT_PAUSE"};

#ifdef CONFIG_ATE
static uint8_t ltf_sym_code[] = {
	0, 0, 1, 2, 2, 3, 3, 4, 4	/* SS 1~8 */
};

static UINT32 tbrx_phy_ctrl[2][3] = {
	{0x83082110, 0x83082114, 0x83082118},
	{0x83082018, 0x8308201c, 0x83082020}
};
#endif

static INT32 chip_show_tmac_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	UINT32 Value = 0;
	RTMP_ADAPTER *pAd = ctrl->priv;

	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_TCR, &Value);
	MTWF_PRINT("TX Stream = %d\n", GET_TMAC_TCR_TX_STREAM_NUM(Value) + 1);
	MTWF_PRINT("TX RIFS Enable = %d\n", GET_TX_RIFS_EN(Value));
	MTWF_PRINT("RX RIFS Mode = %d\n", GET_RX_RIFS_MODE(Value));
	MTWF_PRINT("TXOP TBTT Control = %d\n", GET_TXOP_TBTT_CONTROL(Value));
	MTWF_PRINT("TXOP TBTT Stop Control = %d\n", GET_TBTT_TX_STOP_CONTROL(Value));
	MTWF_PRINT("TXOP Burst Stop = %d\n", GET_TXOP_BURST_STOP(Value));
	MTWF_PRINT("RDG Mode = %d\n", GET_RDG_RA_MODE(Value));
	MTWF_PRINT("RDG Responser Enable = %d\n", GET_RDG_RESP_EN(Value));
	MTWF_PRINT("Smoothing = %d\n", GET_SMOOTHING(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_PSCR, &Value);
	MTWF_PRINT("AP Power Save RXPE Off Time(unit 2us) = %d\n",
			 GET_APS_OFF_TIME(Value));
	MTWF_PRINT("AP Power Save RXPE On Time(unit 2us) = %d\n", APS_ON_TIME(Value));
	MTWF_PRINT("AP Power Save Halt Time (unit 32us) = %d\n",
			 GET_APS_HALT_TIME(Value));
	MTWF_PRINT("AP Power Enable = %d\n", GET_APS_EN(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR1, &Value);
	MTWF_PRINT("AC0 TXOP = 0x%x (unit: 32us)\n", GET_AC0LIMIT(Value));
	MTWF_PRINT("AC1 TXOP = 0x%x (unit: 32us)\n", GET_AC1LIMIT(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR0, &Value);
	MTWF_PRINT("AC2 TXOP = 0x%x (unit: 32us)\n", GET_AC2LIMIT(Value));
	MTWF_PRINT("AC3 TXOP = 0x%x (unit: 32us)\n", GET_AC3LIMIT(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR3, &Value);
	MTWF_PRINT("AC10 TXOP = 0x%x (unit: 32us)\n", GET_AC10LIMIT(Value));
	MTWF_PRINT("AC11 TXOP = 0x%x (unit: 32us)\n", GET_AC11LIMIT(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR2, &Value);
	MTWF_PRINT("AC12 TXOP = 0x%x (unit: 32us)\n", GET_AC12LIMIT(Value));
	MTWF_PRINT("AC13 TXOP = 0x%x (unit: 32us)\n", GET_AC13LIMIT(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ICR_BAND_0, &Value);
	MTWF_PRINT("EIFS Time, Band0 (unit: 1us) = %d\n", GET_ICR_EIFS_TIME(Value));
	MTWF_PRINT("RIFS Time, Band0 (unit: 1us) = %d\n", GET_ICR_RIFS_TIME(Value));
	MTWF_PRINT("SIFS Time, Band0 (unit: 1us) = %d\n", GET_ICR_SIFS_TIME(Value));
	MTWF_PRINT("SLOT Time, Band0 (unit: 1us) = %d\n", GET_ICR_SLOT_TIME(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ICR_BAND_1, &Value);
	MTWF_PRINT("EIFS Time, Band1 (unit: 1us) = %d\n", GET_ICR_EIFS_TIME(Value));
	MTWF_PRINT("RIFS Time, Band1 (unit: 1us) = %d\n", GET_ICR_RIFS_TIME(Value));
	MTWF_PRINT("SIFS Time, Band1 (unit: 1us) = %d\n", GET_ICR_SIFS_TIME(Value));
	MTWF_PRINT("SLOT Time, Band1 (unit: 1us) = %d\n", GET_ICR_SLOT_TIME(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ATCR, &Value);
	MTWF_PRINT("Aggregation Timeout (unit: 50ns) = 0x%x\n", GET_AGG_TOUT(Value));
	return 0;
}

static INT32 chip_show_agg_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
#ifndef CONFIG_AP_SUPPORT
	struct _RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	UINT64 total_burst = 0, total_ampdu = 0, ampdu_cnt[16] = {};
	UINT32 value = 0, idx = 0, row_idx = 0, col_idx = 0, start_range = 0,
	       agg_rang_sel[16] = {}, burst_cnt[16] = {},
	       band_offset = 0;
	uint8_t band_idx = 0, readFW = 1,
		partial_str[16] = {}, full_str[64] = {};
	int ret;

	if (arg != NULL && strlen(arg) > 0)
		readFW = 0;

#ifndef CONFIG_AP_SUPPORT
	for (band_idx = 0; band_idx < pChipCap->band_cnt; band_idx++) {
#else
	band_idx = hc_get_hw_band_idx(pAd);
#endif
		switch (band_idx) {
		case 0:
			band_offset = 0;
			break;
		case 1:
			band_offset = BN1_WF_AGG_TOP_BASE - BN0_WF_AGG_TOP_BASE;
			break;
		default:
			return 0;
		}

		MTWF_PRINT("Band %d AGG Status\n", band_idx);
		MTWF_PRINT("===============================\n");
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR0_ADDR + band_offset, &value);
		MTWF_PRINT("AC00 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR0_AC00_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR0_AC00_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC01 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR0_AC01_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR0_AC01_AGG_LIMIT_SHFT);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR1_ADDR + band_offset, &value);
		MTWF_PRINT("AC02 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR1_AC02_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR1_AC02_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC03 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR1_AC03_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR1_AC03_AGG_LIMIT_SHFT);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR2_ADDR + band_offset, &value);
		MTWF_PRINT("AC10 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR2_AC10_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR2_AC10_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC11 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR2_AC11_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR2_AC11_AGG_LIMIT_SHFT);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR3_ADDR + band_offset, &value);
		MTWF_PRINT("AC12 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR3_AC12_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR3_AC12_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC13 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR3_AC13_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR3_AC13_AGG_LIMIT_SHFT);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR4_ADDR + band_offset, &value);
		MTWF_PRINT("AC20 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR4_AC20_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR4_AC20_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC21 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR4_AC21_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR4_AC21_AGG_LIMIT_SHFT);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR5_ADDR + band_offset, &value);
		MTWF_PRINT("AC22 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR5_AC22_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR5_AC22_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC23 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR5_AC23_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR5_AC23_AGG_LIMIT_SHFT);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR6_ADDR + band_offset, &value);
		MTWF_PRINT("AC30 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR6_AC30_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR6_AC30_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC31 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR6_AC31_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR6_AC31_AGG_LIMIT_SHFT);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR7_ADDR + band_offset, &value);
		MTWF_PRINT("AC32 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR7_AC32_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR7_AC32_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC33 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR7_AC33_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR7_AC33_AGG_LIMIT_SHFT);

		switch (band_idx) {
		case 0:
			band_offset = 0;
			break;
		case 1:
			band_offset = BN1_WF_MIB_TOP_BASE - BN0_WF_MIB_TOP_BASE;
			break;
		default:
			return 0;
		}

		MTWF_PRINT("===AMPDU Related Counters===\n");

		{
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRARC0_ADDR + band_offset, &value);
			agg_rang_sel[0] = (value & BN0_WF_MIB_TOP_TRARC0_AGG_RANG_SEL_0_MASK) >> BN0_WF_MIB_TOP_TRARC0_AGG_RANG_SEL_0_SHFT;
			agg_rang_sel[1] = (value & BN0_WF_MIB_TOP_TRARC0_AGG_RANG_SEL_1_MASK) >> BN0_WF_MIB_TOP_TRARC0_AGG_RANG_SEL_1_SHFT;
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRARC1_ADDR + band_offset, &value);
			agg_rang_sel[2] = (value & BN0_WF_MIB_TOP_TRARC1_AGG_RANG_SEL_2_MASK) >> BN0_WF_MIB_TOP_TRARC1_AGG_RANG_SEL_2_SHFT;
			agg_rang_sel[3] = (value & BN0_WF_MIB_TOP_TRARC1_AGG_RANG_SEL_3_MASK) >> BN0_WF_MIB_TOP_TRARC1_AGG_RANG_SEL_3_SHFT;
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRARC2_ADDR + band_offset, &value);
			agg_rang_sel[4] = (value & BN0_WF_MIB_TOP_TRARC2_AGG_RANG_SEL_4_MASK) >> BN0_WF_MIB_TOP_TRARC2_AGG_RANG_SEL_4_SHFT;
			agg_rang_sel[5] = (value & BN0_WF_MIB_TOP_TRARC2_AGG_RANG_SEL_5_MASK) >> BN0_WF_MIB_TOP_TRARC2_AGG_RANG_SEL_5_SHFT;
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRARC3_ADDR + band_offset, &value);
			agg_rang_sel[6] = (value & BN0_WF_MIB_TOP_TRARC3_AGG_RANG_SEL_6_MASK) >> BN0_WF_MIB_TOP_TRARC3_AGG_RANG_SEL_6_SHFT;
			agg_rang_sel[7] = (value & BN0_WF_MIB_TOP_TRARC3_AGG_RANG_SEL_7_MASK) >> BN0_WF_MIB_TOP_TRARC3_AGG_RANG_SEL_7_SHFT;
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRARC4_ADDR + band_offset, &value);
			agg_rang_sel[8] = (value & BN0_WF_MIB_TOP_TRARC4_AGG_RANG_SEL_8_MASK) >> BN0_WF_MIB_TOP_TRARC4_AGG_RANG_SEL_8_SHFT;
			agg_rang_sel[9] = (value & BN0_WF_MIB_TOP_TRARC4_AGG_RANG_SEL_9_MASK) >> BN0_WF_MIB_TOP_TRARC4_AGG_RANG_SEL_9_SHFT;
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRARC5_ADDR + band_offset, &value);
			agg_rang_sel[10] = (value & BN0_WF_MIB_TOP_TRARC5_AGG_RANG_SEL_10_MASK) >> BN0_WF_MIB_TOP_TRARC5_AGG_RANG_SEL_10_SHFT;
			agg_rang_sel[11] = (value & BN0_WF_MIB_TOP_TRARC5_AGG_RANG_SEL_11_MASK) >> BN0_WF_MIB_TOP_TRARC5_AGG_RANG_SEL_11_SHFT;
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRARC6_ADDR + band_offset, &value);
			agg_rang_sel[12] = (value & BN0_WF_MIB_TOP_TRARC6_AGG_RANG_SEL_12_MASK) >> BN0_WF_MIB_TOP_TRARC6_AGG_RANG_SEL_12_SHFT;
			agg_rang_sel[13] = (value & BN0_WF_MIB_TOP_TRARC6_AGG_RANG_SEL_13_MASK) >> BN0_WF_MIB_TOP_TRARC6_AGG_RANG_SEL_13_SHFT;
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRARC7_ADDR + band_offset, &value);
			agg_rang_sel[14] = (value & BN0_WF_MIB_TOP_TRARC7_AGG_RANG_SEL_14_MASK) >> BN0_WF_MIB_TOP_TRARC7_AGG_RANG_SEL_14_SHFT;

			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR0_ADDR + band_offset, &burst_cnt[0]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR1_ADDR + band_offset, &burst_cnt[1]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR2_ADDR + band_offset, &burst_cnt[2]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR3_ADDR + band_offset, &burst_cnt[3]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR4_ADDR + band_offset, &burst_cnt[4]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR5_ADDR + band_offset, &burst_cnt[5]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR6_ADDR + band_offset, &burst_cnt[6]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR7_ADDR + band_offset, &burst_cnt[7]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR8_ADDR + band_offset, &burst_cnt[8]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR9_ADDR + band_offset, &burst_cnt[9]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR10_ADDR + band_offset, &burst_cnt[10]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR11_ADDR + band_offset, &burst_cnt[11]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR12_ADDR + band_offset, &burst_cnt[12]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR13_ADDR + band_offset, &burst_cnt[13]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR14_ADDR + band_offset, &burst_cnt[14]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_TRDR15_ADDR + band_offset, &burst_cnt[15]);
		}

		start_range = 1;
		total_burst = 0;
		total_ampdu = 0;
		agg_rang_sel[15] = 1023;

		/* Need to add 1 after read from AGG_RANG_SEL CR */
		for (idx = 0; idx < 16; idx++) {
			agg_rang_sel[idx]++;
			total_burst += burst_cnt[idx];

			if (start_range == agg_rang_sel[idx])
				ampdu_cnt[idx] = (UINT64) start_range * burst_cnt[idx];
			else
				ampdu_cnt[idx] = (UINT64) ((start_range + agg_rang_sel[idx]) >> 1) * burst_cnt[idx];

			start_range = agg_rang_sel[idx] + 1;
			total_ampdu += ampdu_cnt[idx];
		}

		start_range = 1;
		ret = snprintf(full_str, sizeof(full_str), "%13s ", "Tx Agg Range:");
		if (os_snprintf_error(sizeof(full_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "snprintf error!\n");
			return 1;
		}

		for (row_idx = 0; row_idx < 4; row_idx++) {
			for (col_idx = 0; col_idx < 4; col_idx++, idx++) {
				idx = 4 * row_idx + col_idx;

				if (start_range == agg_rang_sel[idx]) {
					ret = snprintf(partial_str, sizeof(partial_str), "%d", agg_rang_sel[idx]);
					if (os_snprintf_error(sizeof(partial_str), ret)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "snprintf error!\n");
						return 1;
					}
				} else {
					ret = snprintf(partial_str, sizeof(partial_str), "%d~%d", start_range, agg_rang_sel[idx]);
					if (os_snprintf_error(sizeof(partial_str), ret)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "snprintf error!\n");
						return 1;
					}
				}

				start_range = agg_rang_sel[idx] + 1;
				ret = snprintf(full_str + strlen(full_str), sizeof(full_str) - strlen(full_str), "%-11s ", partial_str);
				if (ret < 0 || strlen(full_str) >= sizeof(full_str)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "snprintf error!\n");
					return 1;
				}
			}

			idx = 4 * row_idx;

			MTWF_PRINT("%s\n", full_str);
			MTWF_PRINT("%13s 0x%-9x 0x%-9x 0x%-9x 0x%-9x\n",
				row_idx ? "" : "Burst count:",
				burst_cnt[idx], burst_cnt[idx + 1],
				burst_cnt[idx + 2], burst_cnt[idx + 3]);

			if (total_burst != 0) {
				if (row_idx == 0) {
					ret = snprintf(full_str, sizeof(full_str), "%13s ",
						"Burst ratio:");
					if (os_snprintf_error(sizeof(full_str), ret)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "snprintf error!\n");
						return 1;
					}
				} else {
					ret = snprintf(full_str, sizeof(full_str), "%13s ", "");
					if (os_snprintf_error(sizeof(full_str), ret)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "snprintf error!\n");
						return 1;
					}
				}

				for (col_idx = 0; col_idx < 4; col_idx++) {
					UINT64 count = (UINT64) burst_cnt[idx + col_idx] * 100;

					ret = snprintf(partial_str, sizeof(partial_str), "(%llu%%)",
						div64_u64(count, total_burst));
					if (os_snprintf_error(sizeof(partial_str), ret)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "snprintf error!\n");
						return 1;
					}
					ret = snprintf(full_str + strlen(full_str), sizeof(full_str) - strlen(full_str),
						"%-11s ", partial_str);
					if (ret < 0 || strlen(full_str) >= sizeof(full_str)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "snprintf error!\n");
						return 1;
					}
				}

				MTWF_PRINT("%s\n", full_str);

				if (row_idx == 0) {
					ret = snprintf(full_str, sizeof(full_str), "%13s ",
						"MDPU ratio:");
					if (os_snprintf_error(sizeof(full_str), ret)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "snprintf error!\n");
						return 1;
					}
				} else {
					ret = snprintf(full_str, sizeof(full_str), "%13s ", "");
					if (os_snprintf_error(sizeof(full_str), ret)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "snprintf error!\n");
						return 1;
					}
				}

				for (col_idx = 0; col_idx < 4; col_idx++) {
					UINT64 count = ampdu_cnt[idx + col_idx] * 100;

					ret = snprintf(partial_str, sizeof(partial_str), "(%llu%%)",
						div64_u64(count, total_ampdu));
					if (os_snprintf_error(sizeof(partial_str), ret)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "snprintf error!\n");
						return 1;
					}
					ret = snprintf(full_str + strlen(full_str), sizeof(full_str) - strlen(full_str),
						"%-11s ", partial_str);
					if (ret < 0 || strlen(full_str) >= sizeof(full_str)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "snprintf error!\n");
						return 1;
					}
				}

				MTWF_PRINT("%s\n", full_str);
			}

			ret = snprintf(full_str, sizeof(full_str), "%13s ", "");
			if (os_snprintf_error(sizeof(full_str), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "snprintf error!\n");
				return 1;
			}
		}
#ifndef CONFIG_AP_SUPPORT
	}
#endif
	return 0;
}

static INT32 chip_dump_mib_info(
	struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 bss_nums = pChipCap->BssNums;
	UINT32 idx = 0;
	UINT32 mac_val = 0, band_idx = 0, band_offset = 0, band_offset_umib = 0;
	UINT32 msdr6 = 0, msdr9 = 0, msdr18 = 0;
	UINT32 rvsr0 = 0, rscr26 = 0, rscr35 = 0, mctr5 = 0, mctr6 = 0, msr0 = 0,
		   msr1 = 0, msr2 = 0;
	UINT32 tbcr0 = 0, tbcr1 = 0, tbcr2 = 0, tbcr3 = 0;
	UINT32 btscr[7] = {};
	UINT32 tdrcr[5] = {};
	UINT32 mbtocr[16] = {}, mbtbcr[16] = {}, mbrocr[16] = {}, mbrbcr[16] = {};
	UINT32 btcr = 0, btbcr = 0, brocr = 0, brbcr = 0, btdcr = 0, brdcr = 0;
	UINT32 mu_cnt[5] = {};
	UINT32 ampdu_cnt[3] = {};
	ULONG per = 0;
#ifdef CONFIG_AP_SUPPORT
	PBCN_CHECK_INFO_STRUC pBcnCheckInfo = NULL;
#endif /* CONFIG_AP_SUPPORT */

	band_idx = hc_get_hw_band_idx(pAd);
#ifdef CONFIG_AP_SUPPORT
	pBcnCheckInfo = &pAd->BcnCheckInfo;
#endif /* CONFIG_AP_SUPPORT */
	switch (band_idx) {
	case 0:
		band_offset = 0;
		band_offset_umib = 0;
		break;
	case 1:
		band_offset = BN1_WF_MIB_TOP_BASE - BN0_WF_MIB_TOP_BASE;
		band_offset_umib = WF_UMIB_TOP_B1BROCR_ADDR - WF_UMIB_TOP_B0BROCR_ADDR;
		break;
	default:
		return TRUE;
	}

	MTWF_PRINT("Band %d MIB Status\n", band_idx);
	MTWF_PRINT("===============================\n");
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_M0SCR0_ADDR + band_offset, &mac_val);
	MTWF_PRINT("MIB Status Control=0x%x\n", mac_val);

	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_M0SDR6_ADDR + band_offset, &msdr6);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_RVSR0_ADDR + band_offset, &rvsr0);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_RSCR35_ADDR + band_offset, &rscr35);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_M0SDR9_ADDR + band_offset, &msdr9);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_RSCR26_ADDR + band_offset, &rscr26);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_MCTR5_ADDR + band_offset, &mctr5);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_MCTR6_ADDR + band_offset, &mctr6);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_M0SDR18_ADDR + band_offset, &msdr18);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_MSR0_ADDR + band_offset, &msr0);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_MSR1_ADDR + band_offset, &msr1);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_MSR2_ADDR + band_offset, &msr2);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_TSCR0_ADDR + band_offset, &ampdu_cnt[0]);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_TSCR3_ADDR + band_offset, &ampdu_cnt[1]);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_TSCR4_ADDR + band_offset, &ampdu_cnt[2]);
	ampdu_cnt[1] &= BN0_WF_MIB_TOP_TSCR3_AMPDU_MPDU_COUNT_MASK;
	ampdu_cnt[2] &= BN0_WF_MIB_TOP_TSCR4_AMPDU_ACKED_COUNT_MASK;

	MTWF_PRINT("===Phy/Timing Related Counters===\n");
	MTWF_PRINT("\tChannelIdleCnt=0x%x\n",
		msdr6 & BN0_WF_MIB_TOP_M0SDR6_CHANNEL_IDLE_COUNT_MASK);
	MTWF_PRINT("\tCCA_NAV_Tx_Time=0x%x\n",
		msdr9 & BN0_WF_MIB_TOP_M0SDR9_CCA_NAV_TX_TIME_MASK);
	MTWF_PRINT("\tRx_MDRDY_CNT=0x%x\n",
		rscr26 & BN0_WF_MIB_TOP_RSCR26_RX_MDRDY_COUNT_MASK);
	MTWF_PRINT("\tCCK_MDRDY_TIME=0x%x, OFDM_MDRDY_TIME=0x%x",
		msr0 & BN0_WF_MIB_TOP_MSR0_CCK_MDRDY_TIME_MASK,
		msr1 & BN0_WF_MIB_TOP_MSR1_OFDM_LG_MIXED_VHT_MDRDY_TIME_MASK);
	MTWF_PRINT(", OFDM_GREEN_MDRDY_TIME=0x%x\n",
		msr2 & BN0_WF_MIB_TOP_MSR2_OFDM_GREEN_MDRDY_TIME_MASK);
	MTWF_PRINT("\tPrim CCA Time=0x%x\n",
		mctr5 & BN0_WF_MIB_TOP_MCTR5_P_CCA_TIME_MASK);
	MTWF_PRINT("\tSec CCA Time=0x%x\n",
		mctr6 & BN0_WF_MIB_TOP_MCTR6_S_CCA_TIME_MASK);
	MTWF_PRINT("\tPrim ED Time=0x%x\n",
		msdr18 & BN0_WF_MIB_TOP_M0SDR18_P_ED_TIME_MASK);

	MTWF_PRINT("===Tx Related Counters(Generic)===\n");
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_TSCR18_ADDR + band_offset, &mac_val);
#ifdef CONFIG_AP_SUPPORT
	pBcnCheckInfo->totalbcncnt +=
		(mac_val & BN0_WF_MIB_TOP_TSCR18_BEACONTXCOUNT_MASK);
	MTWF_PRINT("\tBeaconTxCnt=0x%x\n", pBcnCheckInfo->totalbcncnt);
	pBcnCheckInfo->totalbcncnt = 0;
#else
	MTWF_PRINT("\tBeaconTxCnt=0x%x\n", mac_val);
#endif
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_TBCR0_ADDR + band_offset, &tbcr0);
	MTWF_PRINT("\tTx 20MHz Cnt=0x%x\n",
		tbcr0 & BN0_WF_MIB_TOP_TBCR0_TX_20MHZ_CNT_MASK);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_TBCR1_ADDR + band_offset, &tbcr1);
	MTWF_PRINT("\tTx 40MHz Cnt=0x%x\n",
		tbcr1 & BN0_WF_MIB_TOP_TBCR1_TX_40MHZ_CNT_MASK);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_TBCR2_ADDR + band_offset, &tbcr2);
	MTWF_PRINT("\tTx 80MHz Cnt=0x%x\n",
		tbcr2 & BN0_WF_MIB_TOP_TBCR2_TX_80MHZ_CNT_MASK);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_TBCR3_ADDR + band_offset, &tbcr3);
	MTWF_PRINT("\tTx 160MHz Cnt=0x%x\n",
		tbcr3 & BN0_WF_MIB_TOP_TBCR3_TX_160MHZ_CNT_MASK);
	MTWF_PRINT("\tAMPDU Cnt=0x%x\n", ampdu_cnt[0]);
	MTWF_PRINT("\tAMPDU MPDU Cnt=0x%x\n", ampdu_cnt[1]);
	MTWF_PRINT("\tAMPDU MPDU Ack Cnt=0x%x\n", ampdu_cnt[2]);
	per = (ampdu_cnt[2] == 0 ?
		0 : 1000 * (ampdu_cnt[1] - ampdu_cnt[2]) / ampdu_cnt[1]);
	MTWF_PRINT("\tAMPDU MPDU PER=%ld.%1ld%%\n", per / 10, per % 10);

	MTWF_PRINT("===MU Related Counters===\n");
	MAC_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_BSCR2_ADDR + band_offset, &mu_cnt[0]);
	MAC_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_TSCR5_ADDR + band_offset, &mu_cnt[1]);
	MAC_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_TSCR6_ADDR + band_offset, &mu_cnt[2]);
	MAC_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_TSCR8_ADDR + band_offset, &mu_cnt[3]);
	MAC_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_TSCR7_ADDR + band_offset, &mu_cnt[4]);

	MTWF_PRINT("\tMUBF_TX_COUNT=0x%x\n",
		mu_cnt[0] & BN0_WF_MIB_TOP_BSCR2_MUBF_TX_COUNT_MASK);
	MTWF_PRINT("\tMU_TX_MPDU_COUNT(Ok+Fail)=0x%x\n", mu_cnt[1]);
	MTWF_PRINT("\tMU_TX_OK_MPDU_COUNT=0x%x\n", mu_cnt[2]);
	MTWF_PRINT("\tMU_TO_MU_FAIL_PPDU_COUNT=0x%x\n", mu_cnt[3]);
	MTWF_PRINT("\tSU_TX_OK_MPDU_COUNT=0x%x\n", mu_cnt[4]);

	MTWF_PRINT("===Rx Related Counters(Generic)===\n");
	MTWF_PRINT("\tVector Mismacth Cnt=0x%x\n",
		rvsr0 & BN0_WF_MIB_TOP_RVSR0_VEC_MISS_COUNT_MASK);
	MTWF_PRINT("\tDelimiter Fail Cnt=0x%x\n",
		rscr35 & BN0_WF_MIB_TOP_RSCR35_DELIMITER_FAIL_COUNT_MASK);

	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_RSCR1_ADDR + band_offset, &mac_val);
	MTWF_PRINT("\tRxFCSErrCnt=0x%x\n",
		(mac_val & BN0_WF_MIB_TOP_RSCR1_RX_FCS_ERROR_COUNT_MASK));
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_RSCR33_ADDR + band_offset, &mac_val);
	MTWF_PRINT("\tRxFifoFullCnt=0x%x\n",
		(mac_val & BN0_WF_MIB_TOP_RSCR33_RX_FIFO_FULL_COUNT_MASK));
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_RSCR36_ADDR + band_offset, &mac_val);
	MTWF_PRINT("\tRxLenMismatch=0x%x\n",
		(mac_val & BN0_WF_MIB_TOP_RSCR36_RX_LEN_MISMATCH_MASK));
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_RSCR31_ADDR + band_offset, &mac_val);
	MTWF_PRINT("\tRxMPDUCnt=0x%x\n",
		(mac_val & BN0_WF_MIB_TOP_RSCR31_RX_MPDU_COUNT_MASK));
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_RSCR27_ADDR + band_offset, &mac_val);
	MTWF_PRINT("\tRx AMPDU Cnt=0x%x\n", mac_val);
	RTMP_IO_READ32(pAd->hdev_ctrl,
		BN0_WF_MIB_TOP_RSCR28_ADDR + band_offset, &mac_val);
	MTWF_PRINT("\tRx Total ByteCnt=0x%x\n", mac_val);


	/* Per-BSS T/RX Counters */
	MTWF_PRINT("===Per-BSS Related Tx/Rx Counters===\n");
	MTWF_PRINT("BSS Idx TxCnt/DataCnt TxByteCnt RxOkCnt/DataCnt RxByteCnt\n");
	for (idx = 0; idx < bss_nums; idx++) {
		RTMP_IO_READ32(pAd->hdev_ctrl
			, BN0_WF_MIB_TOP_BTCR_ADDR + band_offset + idx * 4
			, &btcr);
		RTMP_IO_READ32(pAd->hdev_ctrl
			, BN0_WF_MIB_TOP_BTDCR_ADDR + band_offset + idx * 4
			, &btdcr);
		RTMP_IO_READ32(pAd->hdev_ctrl
			, BN0_WF_MIB_TOP_BTBCR_ADDR + band_offset + idx * 4
			, &btbcr);

		RTMP_IO_READ32(pAd->hdev_ctrl
			, WF_UMIB_TOP_B0BROCR_ADDR + band_offset_umib + idx * 4
			, &brocr);
		RTMP_IO_READ32(pAd->hdev_ctrl
			, WF_UMIB_TOP_B0BRDCR_ADDR + band_offset_umib + idx * 4
			, &brdcr);
		RTMP_IO_READ32(pAd->hdev_ctrl
			, WF_UMIB_TOP_B0BRBCR_ADDR + band_offset_umib + idx * 4
			, &brbcr);

		MTWF_PRINT("%d\t 0x%x/0x%x\t 0x%x \t 0x%x/0x%x \t 0x%x\n",
			idx, btcr, btdcr, btbcr, brocr, brdcr, brbcr);
	}

	MTWF_PRINT("===Per-BSS Related MIB Counters===\n");
	MTWF_PRINT("BSS Idx RTSTx/RetryCnt BAMissCnt AckFailCnt FrmRetry1/2/3Cnt\n");

	/* Per-BSS TX Status */
	for (idx = 0; idx < bss_nums; idx++) {
		RTMP_IO_READ32(pAd->hdev_ctrl
			, BN0_WF_MIB_TOP_BTSCR5_ADDR + band_offset + idx * 4
			, &btscr[0]);
		RTMP_IO_READ32(pAd->hdev_ctrl
			, BN0_WF_MIB_TOP_BTSCR6_ADDR + band_offset + idx * 4
			, &btscr[1]);
		RTMP_IO_READ32(pAd->hdev_ctrl
			, BN0_WF_MIB_TOP_BTSCR0_ADDR + band_offset + idx * 4
			, &btscr[2]);
		RTMP_IO_READ32(pAd->hdev_ctrl
			, BN0_WF_MIB_TOP_BTSCR1_ADDR + band_offset + idx * 4
			, &btscr[3]);
		RTMP_IO_READ32(pAd->hdev_ctrl
			, BN0_WF_MIB_TOP_BTSCR2_ADDR + band_offset + idx * 4
			, &btscr[4]);
		RTMP_IO_READ32(pAd->hdev_ctrl
			, BN0_WF_MIB_TOP_BTSCR3_ADDR + band_offset + idx * 4
			, &btscr[5]);
		RTMP_IO_READ32(pAd->hdev_ctrl
			, BN0_WF_MIB_TOP_BTSCR4_ADDR + band_offset + idx * 4
			, &btscr[6]);

		MTWF_PRINT("%d:\t0x%x/0x%x  0x%x \t 0x%x \t  0x%x/0x%x/0x%x\n",
			idx, (btscr[0] & BN0_WF_MIB_TOP_BTSCR5_RTSTXCOUNTn_MASK),
			(btscr[1] & BN0_WF_MIB_TOP_BTSCR6_RTSRETRYCOUNTn_MASK),
			(btscr[2] & BN0_WF_MIB_TOP_BTSCR0_BAMISSCOUNTn_MASK),
			(btscr[3] & BN0_WF_MIB_TOP_BTSCR1_ACKFAILCOUNTn_MASK),
			(btscr[4] & BN0_WF_MIB_TOP_BTSCR2_FRAMERETRYCOUNTn_MASK),
			(btscr[5] & BN0_WF_MIB_TOP_BTSCR3_FRAMERETRY2COUNTn_MASK),
			(btscr[6] & BN0_WF_MIB_TOP_BTSCR4_FRAMERETRY3COUNTn_MASK));
	}

	/* Dummy delimiter insertion result */
	MTWF_PRINT("===Dummy delimiter insertion result===\n");
	RTMP_IO_READ32(pAd->hdev_ctrl
		, BN0_WF_MIB_TOP_TDRCR0_ADDR + band_offset, &tdrcr[0]);
	RTMP_IO_READ32(pAd->hdev_ctrl
		, BN0_WF_MIB_TOP_TDRCR1_ADDR + band_offset, &tdrcr[1]);
	RTMP_IO_READ32(pAd->hdev_ctrl
		, BN0_WF_MIB_TOP_TDRCR2_ADDR + band_offset, &tdrcr[2]);
	RTMP_IO_READ32(pAd->hdev_ctrl
		, BN0_WF_MIB_TOP_TDRCR3_ADDR + band_offset, &tdrcr[3]);
	RTMP_IO_READ32(pAd->hdev_ctrl
		, BN0_WF_MIB_TOP_TDRCR4_ADDR + band_offset, &tdrcr[4]);

	MTWF_PRINT("Range0 = %d\t Range1 = %d\t Range2 = %d\t Range3 = %d\t Range4 = %d\n",
		tdrcr[0],
		tdrcr[1],
		tdrcr[2],
		tdrcr[3],
		tdrcr[4]);

	/* Per-MBSS T/RX Counters */
	MTWF_PRINT("===Per-MBSS Related Tx/Rx Counters===\n");
	MTWF_PRINT("MBSSIdx   TxOkCnt  TxByteCnt  RxOkCnt  RxByteCnt\n");

	for (idx = 0; idx < 16; idx++) {
		RTMP_IO_READ32(pAd->hdev_ctrl
			, BN0_WF_MIB_TOP_BTOCR_ADDR + band_offset + (bss_nums + idx) * 4
			, &mbtocr[idx]);
		RTMP_IO_READ32(pAd->hdev_ctrl
			, BN0_WF_MIB_TOP_BTBCR_ADDR + band_offset + (bss_nums + idx) * 4
			, &mbtbcr[idx]);

		RTMP_IO_READ32(pAd->hdev_ctrl
			, WF_UMIB_TOP_B0BROCR_ADDR + band_offset_umib + (bss_nums + idx) * 4
			, &mbrocr[idx]);
		RTMP_IO_READ32(pAd->hdev_ctrl
			, WF_UMIB_TOP_B0BRBCR_ADDR + band_offset_umib + (bss_nums + idx) * 4
			, &mbrbcr[idx]);
	}

	for (idx = 0; idx < 16; idx++) {
		MTWF_PRINT("%d\t 0x%x\t 0x%x \t 0x%x \t 0x%x\n",
			idx, mbtocr[idx], mbtbcr[idx], mbrocr[idx], mbrbcr[idx]);
	}
#ifdef TRACELOG_TCP_PKT
	MTWF_PRINT("TCP RxAck = %d\t TxData = %d",
		pAd->u4TcpRxAckCnt, pAd->u4TcpTxDataCnt);
	pAd->u4TcpRxAckCnt = 0;
	pAd->u4TcpTxDataCnt = 0;
#endif /* TRACELOG_TCP_PKT */
	return TRUE;
}

static INT32 chip_show_pse_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 pse_buf_ctrl = 0, pg_sz = 0, pg_num = 0;
	UINT32 pse_stat[2] = {}, pg_flow_ctrl[28] = {0};
	UINT32 fpg_cnt = 0, ffa_cnt = 0, fpg_head = 0, fpg_tail = 0;
	UINT32 max_q = 0, min_q = 0, rsv_pg = 0, used_pg = 0;
	INT32 i = 0;

	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PBUF_CTRL_ADDR, &pse_buf_ctrl);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_QUEUE_EMPTY_ADDR, &pse_stat[0]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_QUEUE_EMPTY_1_ADDR, &pse_stat[1]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_FREEPG_CNT_ADDR, &pg_flow_ctrl[0]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_FREEPG_HEAD_TAIL_ADDR, &pg_flow_ctrl[1]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_HIF0_GROUP_ADDR, &pg_flow_ctrl[2]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_HIF0_PG_INFO_ADDR, &pg_flow_ctrl[3]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_HIF1_GROUP_ADDR, &pg_flow_ctrl[4]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_HIF1_PG_INFO_ADDR, &pg_flow_ctrl[5]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_CPU_GROUP_ADDR, &pg_flow_ctrl[6]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_CPU_PG_INFO_ADDR, &pg_flow_ctrl[7]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_LMAC0_GROUP_ADDR, &pg_flow_ctrl[8]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_LMAC0_PG_INFO_ADDR, &pg_flow_ctrl[9]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_LMAC1_GROUP_ADDR, &pg_flow_ctrl[10]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_LMAC1_PG_INFO_ADDR, &pg_flow_ctrl[11]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_LMAC2_GROUP_ADDR, &pg_flow_ctrl[12]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_LMAC2_PG_INFO_ADDR, &pg_flow_ctrl[13]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_PLE_GROUP_ADDR, &pg_flow_ctrl[14]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PLE_PG_INFO_ADDR, &pg_flow_ctrl[15]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_LMAC3_GROUP_ADDR, &pg_flow_ctrl[16]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_LMAC3_PG_INFO_ADDR, &pg_flow_ctrl[17]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_MDP_GROUP_ADDR, &pg_flow_ctrl[18]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_MDP_PG_INFO_ADDR, &pg_flow_ctrl[19]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_PLE1_GROUP_ADDR, &pg_flow_ctrl[20]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PLE1_PG_INFO_ADDR, &pg_flow_ctrl[21]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_MDP2_GROUP_ADDR, &pg_flow_ctrl[22]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_MDP2_PG_INFO_ADDR, &pg_flow_ctrl[23]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_HIF2_GROUP_ADDR, &pg_flow_ctrl[26]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_HIF2_PG_INFO_ADDR, &pg_flow_ctrl[27]);
	/* Configuration Info */
	MTWF_PRINT("PSE Configuration Info:\n");
	MTWF_PRINT("\tPacket Buffer Control: 0x%08x\n", pse_buf_ctrl);
	pg_sz = (pse_buf_ctrl & WF_PSE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_MASK) >> WF_PSE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_SHFT;
	MTWF_PRINT("\t\tPage Size=%d(%d bytes per page)\n", pg_sz, (pg_sz == 1 ? 256 : 128));
	MTWF_PRINT("\t\tPage Offset=%d(in unit of 64KB)\n",
			 (pse_buf_ctrl & WF_PSE_TOP_PBUF_CTRL_PBUF_OFFSET_MASK) >> WF_PSE_TOP_PBUF_CTRL_PBUF_OFFSET_SHFT);
	pg_num = (pse_buf_ctrl & WF_PSE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_MASK) >> WF_PSE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_SHFT;
	MTWF_PRINT("\t\tTotal page numbers=%d pages\n", pg_num);
	/* Page Flow Control */
	MTWF_PRINT("PSE Page Flow Control:\n");
	MTWF_PRINT("\tFree page counter: 0x%08x\n", pg_flow_ctrl[0]);
	fpg_cnt = (pg_flow_ctrl[0] & WF_PSE_TOP_FREEPG_CNT_FREEPG_CNT_MASK) >> WF_PSE_TOP_FREEPG_CNT_FREEPG_CNT_SHFT;
	MTWF_PRINT("\t\tThe toal page number of free=0x%03x\n", fpg_cnt);
	ffa_cnt = (pg_flow_ctrl[0] & WF_PSE_TOP_FREEPG_CNT_FFA_CNT_MASK) >> WF_PSE_TOP_FREEPG_CNT_FFA_CNT_SHFT;
	MTWF_PRINT("\t\tThe free page numbers of free for all=0x%03x\n", ffa_cnt);
	MTWF_PRINT("\tFree page head and tail: 0x%08x\n", pg_flow_ctrl[1]);
	fpg_head = (pg_flow_ctrl[1] & WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_MASK) >> WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_SHFT;
	fpg_tail = (pg_flow_ctrl[1] & WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_MASK) >> WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_SHFT;
	MTWF_PRINT("\t\tThe tail/head page of free page list=0x%03x/0x%03x\n", fpg_tail, fpg_head);
	MTWF_PRINT("\tReserved page counter of HIF0 group: 0x%08x\n", pg_flow_ctrl[2]);
	MTWF_PRINT("\tHIF0 group page status: 0x%08x\n", pg_flow_ctrl[3]);
	min_q = (pg_flow_ctrl[2] & WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[2] & WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of HIF0 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[3] & WF_PSE_TOP_HIF0_PG_INFO_HIF0_RSV_CNT_MASK) >> WF_PSE_TOP_HIF0_PG_INFO_HIF0_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[3] & WF_PSE_TOP_HIF0_PG_INFO_HIF0_SRC_CNT_MASK) >> WF_PSE_TOP_HIF0_PG_INFO_HIF0_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of HIF0 group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	MTWF_PRINT("\tReserved page counter of HIF1 group: 0x%08x\n", pg_flow_ctrl[4]);
	MTWF_PRINT("\tHIF1 group page status: 0x%08x\n", pg_flow_ctrl[5]);
	min_q = (pg_flow_ctrl[4] & WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[4] & WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of HIF1 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[5] & WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[5] & WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of HIF1 group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	MTWF_PRINT("\tReserved page counter of HIF2 group: 0x%08x\n", pg_flow_ctrl[26]);
	MTWF_PRINT("\tHIF2 group page status: 0x%08x\n", pg_flow_ctrl[27]);
	min_q = (pg_flow_ctrl[26] & WF_PSE_TOP_PG_HIF2_GROUP_HIF2_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF2_GROUP_HIF2_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[26] & WF_PSE_TOP_PG_HIF2_GROUP_HIF2_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF2_GROUP_HIF2_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of HIF2 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[27] & WF_PSE_TOP_HIF2_PG_INFO_HIF2_RSV_CNT_MASK) >> WF_PSE_TOP_HIF2_PG_INFO_HIF2_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[27] & WF_PSE_TOP_HIF2_PG_INFO_HIF2_SRC_CNT_MASK) >> WF_PSE_TOP_HIF2_PG_INFO_HIF2_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of HIF2 group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	MTWF_PRINT("\tReserved page counter of CPU group: 0x%08x\n", pg_flow_ctrl[6]);
	MTWF_PRINT("\tCPU group page status: 0x%08x\n", pg_flow_ctrl[7]);
	min_q = (pg_flow_ctrl[6] & WF_PSE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[6] & WF_PSE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of CPU group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[7] & WF_PSE_TOP_CPU_PG_INFO_CPU_RSV_CNT_MASK) >> WF_PSE_TOP_CPU_PG_INFO_CPU_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[7] & WF_PSE_TOP_CPU_PG_INFO_CPU_SRC_CNT_MASK) >> WF_PSE_TOP_CPU_PG_INFO_CPU_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of CPU group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	MTWF_PRINT("\tReserved page counter of LMAC0 group: 0x%08x\n", pg_flow_ctrl[8]);
	MTWF_PRINT("\tLMAC0 group page status: 0x%08x\n", pg_flow_ctrl[9]);
	min_q = (pg_flow_ctrl[8] & WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[8] & WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of LMAC0 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[9] & WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[9] & WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of LMAC0 group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	MTWF_PRINT("\tReserved page counter of LMAC1 group: 0x%08x\n", pg_flow_ctrl[10]);
	MTWF_PRINT("\tLMAC1 group page status: 0x%08x\n", pg_flow_ctrl[11]);
	min_q = (pg_flow_ctrl[10] & WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[10] & WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of LMAC1 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[11] & WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[11] & WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of LMAC1 group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	MTWF_PRINT("\tReserved page counter of LMAC2 group: 0x%08x\n", pg_flow_ctrl[11]);
	MTWF_PRINT("\tLMAC2 group page status: 0x%08x\n", pg_flow_ctrl[12]);
	min_q = (pg_flow_ctrl[12] & WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[12] & WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of LMAC2 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[13] & WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[13] & WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of LMAC2 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	MTWF_PRINT("\tReserved page counter of LMAC3 group: 0x%08x\n", pg_flow_ctrl[16]);
	MTWF_PRINT("\tLMAC3 group page status: 0x%08x\n", pg_flow_ctrl[17]);
	min_q = (pg_flow_ctrl[16] & WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[16] & WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of LMAC3 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[17] & WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[17] & WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of LMAC3 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	MTWF_PRINT("\tReserved page counter of PLE group: 0x%08x\n", pg_flow_ctrl[14]);
	MTWF_PRINT("\tPLE group page status: 0x%08x\n", pg_flow_ctrl[15]);
	min_q = (pg_flow_ctrl[14] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[14] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of PLE group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[15] & WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[15] & WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of PLE group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	MTWF_PRINT("\tReserved page counter of PLE1 group: 0x%08x\n", pg_flow_ctrl[14]);
	MTWF_PRINT("\tPLE1 group page status: 0x%08x\n", pg_flow_ctrl[15]);
	min_q = (pg_flow_ctrl[20] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[20] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of PLE1 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[21] & WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[21] & WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of PLE1 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	MTWF_PRINT("\tReserved page counter of MDP group: 0x%08x\n", pg_flow_ctrl[18]);
	MTWF_PRINT("\tMDP group page status: 0x%08x\n", pg_flow_ctrl[19]);
	min_q = (pg_flow_ctrl[18] & WF_PSE_TOP_PG_MDP_GROUP_MDP_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP_GROUP_MDP_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[18] & WF_PSE_TOP_PG_MDP_GROUP_MDP_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP_GROUP_MDP_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of MDP group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[19] & WF_PSE_TOP_MDP_PG_INFO_MDP_RSV_CNT_MASK) >> WF_PSE_TOP_MDP_PG_INFO_MDP_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[19] & WF_PSE_TOP_MDP_PG_INFO_MDP_SRC_CNT_MASK) >> WF_PSE_TOP_MDP_PG_INFO_MDP_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of MDP group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	MTWF_PRINT("\tReserved page counter of MDP2 group: 0x%08x\n", pg_flow_ctrl[22]);
	MTWF_PRINT("\tMDP2 group page status: 0x%08x\n", pg_flow_ctrl[23]);
	min_q = (pg_flow_ctrl[22] & WF_PSE_TOP_PG_MDP2_GROUP_MDP2_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP2_GROUP_MDP2_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[22] & WF_PSE_TOP_PG_MDP2_GROUP_MDP2_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP2_GROUP_MDP2_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of MDP2 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[23] & WF_PSE_TOP_MDP2_PG_INFO_MDP2_RSV_CNT_MASK) >> WF_PSE_TOP_MDP2_PG_INFO_MDP2_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[23] & WF_PSE_TOP_MDP2_PG_INFO_MDP2_SRC_CNT_MASK) >> WF_PSE_TOP_MDP2_PG_INFO_MDP2_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of MDP2 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	/* Queue Empty Status */
	MTWF_PRINT("PSE Queue Empty Status:\n");
	MTWF_PRINT("\tQUEUE_EMPTY: 0x%08x, QUEUE_EMPTY2: 0x%08x\n", pse_stat[0], pse_stat[1]);
	MTWF_PRINT("\t\tCPU Q0/1/2/3/4 empty=%d/%d/%d/%d/%d\n",
			  (pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q0_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q0_EMPTY_SHFT,
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q1_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q1_EMPTY_SHFT),
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q2_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q2_EMPTY_SHFT),
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q3_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q3_EMPTY_SHFT),
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q4_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q4_EMPTY_SHFT));
	MTWF_PRINT("\t\tHIF Q0/1/2/3/4/5/6/7/8 empty=%d/%d/%d/%d/%d/%d/%d/%d/%d\n",
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_0_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_0_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_1_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_1_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_2_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_2_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_3_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_3_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_4_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_4_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_5_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_5_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_6_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_6_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_7_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_7_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_8_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_8_EMPTY_SHFT));
	MTWF_PRINT("\t\tHIF Q9/10/11/12/13/14/15/16 empty=%d/%d/%d/%d/%d/%d/%d/%d\n",
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_9_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_9_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_10_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_10_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_11_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_11_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_12_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_12_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_13_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_13_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_14_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_14_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_15_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_15_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_16_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_16_EMPTY_SHFT));
	MTWF_PRINT("\t\tLMAC TX Q empty=%d\n",
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_LMAC_TX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_LMAC_TX_QUEUE_EMPTY_SHFT));
	MTWF_PRINT("\t\tMDP TX Q0/Q1/Q2/RX Q empty=%d/%d/%d/%d\n",
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_MDP_TX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_TX_QUEUE_EMPTY_SHFT),
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_MDP_TX1_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_TX1_QUEUE_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_MDP_TX2_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_MDP_TX2_QUEUE_EMPTY_SHFT),
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_MDP_RX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_RX_QUEUE_EMPTY_SHFT));
	MTWF_PRINT("\t\tSEC TX Q0/Q1/Q2/RX Q empty=%d/%d/%d/%d\n",
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_SEC_TX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SEC_TX_QUEUE_EMPTY_SHFT),
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_SEC_TX1_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SEC_TX1_QUEUE_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_SEC_TX2_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_SEC_TX2_QUEUE_EMPTY_SHFT),
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_SEC_RX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SEC_RX_QUEUE_EMPTY_SHFT));
	MTWF_PRINT("\t\tSFD PARK Q empty=%d\n",
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_SFD_PARK_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SFD_PARK_QUEUE_EMPTY_SHFT));
	MTWF_PRINT("\t\tMDP TXIOC Q0/Q1/Q2 empty=%d/%d/%d\n",
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_MDP_TXIOC_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_TXIOC_QUEUE_EMPTY_SHFT),
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_MDP_TXIOC1_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_TXIOC1_QUEUE_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_MDP_TXIOC2_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_MDP_TXIOC2_QUEUE_EMPTY_SHFT));
	MTWF_PRINT("\t\tMDP RXIOC Q0/Q1/Q2/Q3 empty=%d/%d/%d/%d\n",
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_MDP_RXIOC_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_RXIOC_QUEUE_EMPTY_SHFT),
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_MDP_RXIOC1_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_RXIOC1_QUEUE_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_MDP_RXIOC2_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_MDP_RXIOC2_QUEUE_EMPTY_SHFT),
			  ((pse_stat[1] & WF_PSE_TOP_QUEUE_EMPTY_1_MDP_RXIOC3_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_MDP_RXIOC3_QUEUE_EMPTY_SHFT));
	MTWF_PRINT("\t\tRLS Q empty=%d\n",
			  ((pse_stat[0] & WF_PSE_TOP_QUEUE_EMPTY_RLS_Q_EMTPY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_RLS_Q_EMTPY_SHFT));
	MTWF_PRINT("Nonempty Q info:\n");

	for (i = 0; i < 31; i++) {
		if (((pse_stat[0] & (0x1 << i)) >> i) == 0) {
			UINT32 hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (pse_queue_empty_info[i].QueueName != NULL) {
				MTWF_PRINT("\t%s: ", pse_queue_empty_info[i].QueueName);
				fl_que_ctrl[0] |= WF_PSE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (pse_queue_empty_info[i].Portid << WF_PSE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (pse_queue_empty_info[i].Queueid << WF_PSE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
			} else
				continue;

			fl_que_ctrl[0] |= (0x1 << 31);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PSE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_FL_QUE_CTRL_2_ADDR, &fl_que_ctrl[1]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_FL_QUE_CTRL_3_ADDR, &fl_que_ctrl[2]);
			hfid = (fl_que_ctrl[1] & WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >> WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
			tfid = (fl_que_ctrl[1] & WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >> WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
			pktcnt = (fl_que_ctrl[2] & WF_PSE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >> WF_PSE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
			MTWF_PRINT("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x\n",
					  tfid, hfid, pktcnt);
		}
	}

	for (i = 0; i < 31; i++) {
		if (((pse_stat[1] & (0x1 << i)) >> i) == 0) {
			UINT32 hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (pse_queue_empty2_info[i].QueueName != NULL) {
				MTWF_PRINT("\t%s: ", pse_queue_empty2_info[i].QueueName);
				fl_que_ctrl[0] |= WF_PSE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (pse_queue_empty2_info[i].Portid << WF_PSE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (pse_queue_empty2_info[i].Queueid << WF_PSE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
			} else
				continue;

			fl_que_ctrl[0] |= (0x1 << 31);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PSE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_FL_QUE_CTRL_2_ADDR, &fl_que_ctrl[1]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_FL_QUE_CTRL_3_ADDR, &fl_que_ctrl[2]);
			hfid = (fl_que_ctrl[1] & WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >> WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
			tfid = (fl_que_ctrl[1] & WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >> WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
			pktcnt = (fl_que_ctrl[2] & WF_PSE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >> WF_PSE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
			MTWF_PRINT("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x\n",
					  tfid, hfid, pktcnt);
		}
	}

	return TRUE;
}

static INT32 chip_show_protect_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	UINT32 val = 0;
	RTMP_ADAPTER *pAd = ctrl->priv;

	MTWF_PRINT(" -Proetction\n");
	RTMP_IO_READ32(pAd->hdev_ctrl, AGG_PCR, &val);
	MTWF_PRINT("  > AGG_PCR 0x%08x\n", val);
	MTWF_PRINT(" -RTS Threshold\n");
	RTMP_IO_READ32(pAd->hdev_ctrl, AGG_PCR1, &val);
	MTWF_PRINT("  > AGG_PCR1 0x%08x\n", val);
	return TRUE;
}

static INT32 chip_show_cca_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	UINT32 val = 0;
	RTMP_ADAPTER *pAd = ctrl->priv;

	MAC_IO_READ32(pAd->hdev_ctrl, RMAC_DEBUG_CR, &val);
	val |= (1 << 31); /* For Band0 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_DEBUG_CR, val);
	/* Debug CR */
	MAC_IO_WRITE32(pAd->hdev_ctrl, (WF_CFG_OFF_BASE + 0x2c), 0xf);
	MAC_IO_WRITE32(pAd->hdev_ctrl, (WF_CFG_BASE + 0x14), 0x1f);
	MAC_IO_WRITE32(pAd->hdev_ctrl, (WF_CFG_BASE + 0x18), 0x06060606);
	MAC_IO_WRITE32(pAd->hdev_ctrl, (WF_CFG_BASE + 0x4c), 0x1c1c1d1d);
	MAC_IO_READ32(pAd->hdev_ctrl, (WF_CFG_BASE + 0x24), &val);
	MTWF_PRINT("CCA for BAND0 info:\n");
	MTWF_PRINT("-- CCA Prim: %d, SE20: %d, SEC40: %d\n",
			  ((val & (1 << 14)) >> 14), ((val & (1 << 6)) >> 6),
			  ((val & (1 << 5)) >> 5));
	MAC_IO_READ32(pAd->hdev_ctrl, RMAC_DEBUG_CR, &val);
	val &= ~(1 << 31); /* For Band1 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_DEBUG_CR, val);
	MAC_IO_READ32(pAd->hdev_ctrl, (WF_CFG_BASE + 0x24), &val);
	MTWF_PRINT("CCA for BAND1 info:\n");
	MTWF_PRINT("-- CCA Prim: %d, SE20: %d, SEC40: %d\n",
			  ((val & (1 << 14)) >> 14), ((val & (1 << 6)) >> 6),
			  ((val & (1 << 5)) >> 5));
	return 0;
}

static INT32 chip_set_cca_en(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	uint8_t enable;
	UINT32 val = 0;
	RTMP_ADAPTER *pAd = ctrl->priv;

	enable = os_str_tol(arg, 0, 10);
	MTWF_PRINT("Enable CCA on Band0 SEC40: %s\n", (enable) ? "ON" : "OFF");
	/* RF CR for BAND0 CCA */
	PHY_IO_READ32(pAd->hdev_ctrl, PHY_BAND0_PHY_CCA, &val);
	val |= ((1 << 18) | (1 << 2));
	MTWF_PRINT("-- Force Mode: %d, Force CCA SEC40: %d [0x%08x]\n",
			  ((val & (1 << 18)) >> 18), ((val & (1 << 2)) >> 2), val);
	PHY_IO_WRITE32(pAd->hdev_ctrl, PHY_BAND0_PHY_CCA, val);
	/* TMAC_TCR for the normal Tx BW */
	MAC_IO_READ32(pAd->hdev_ctrl, TMAC_TCR, &val);
	val &= ~(PRE_RTS_IDLE_DET_DIS);
	val |= DCH_DET_DIS;
	MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_TCR, val);
	return TRUE;
}

static INT32 chip_show_dmasch_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 value = 0;
	UINT32 ple_pkt_max_sz;
	UINT32 pse_pkt_max_sz;
	UINT32 max_quota;
	UINT32 min_quota;
	UINT32 rsv_cnt;
	UINT32 src_cnt;
	UINT32 pse_rsv_cnt = 0;
	UINT32 pse_src_cnt = 0;
	UINT32 odd_group_pktin_cnt = 0;
	UINT32 odd_group_ask_cnt = 0;
	UINT32 pktin_cnt;
	UINT32 ask_cnt;
	UINT32 total_src_cnt = 0;
	UINT32 total_rsv_cnt = 0;
	UINT32 ffa_cnt;
	UINT32 free_pg_cnt;
	UINT32 Group_Mapping_Q[16] = {0};
	UINT32 qmapping_addr = WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_ADDR;
	UINT32 status_addr = WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_ADDR;
	UINT32 quota_addr = WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_ADDR;
	UINT32 pkt_cnt_addr = WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_ADDR;
	UINT32 mapping_mask = 0xf;
	UINT32 mapping_offset = 0;
	UINT32 mapping_qidx;
	UINT32 groupidx = 0;
	uint8_t idx = 0;
	uint8_t pktin_int_refill_ena;
	uint8_t	pdma_add_int_refill_ena;
	uint8_t	ple_add_int_refill_ena;
	uint8_t	ple_sub_ena;
	uint8_t	hif_ask_sub_ena;
	uint8_t	wacpu_mode_en;
	UINT32 ple_rpg_hif;
	UINT32 ple_upg_hif;
	UINT32 pse_rpg_hif = 0;
	UINT32 pse_upg_hif = 0;
	UCHAR is_mismatch = FALSE;

	for (mapping_qidx = 0; mapping_qidx < 32; mapping_qidx++) {
		UINT32 mapping_group;

		idx = 0;

		if (mapping_qidx == 0) {
			qmapping_addr = WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_ADDR;
			mapping_mask = 0xf;
			mapping_offset = 0;
		} else if ((mapping_qidx % 8) == 0) {
			qmapping_addr += 0x4;
			mapping_mask = 0xf;
			mapping_offset = 0;
		} else {
			mapping_offset += 4;
			mapping_mask = 0xf << mapping_offset;
		}

		HW_IO_READ32(pAd->hdev_ctrl, qmapping_addr, &value);
		mapping_group = (value & mapping_mask) >> mapping_offset;
		Group_Mapping_Q[mapping_group] |= 1 << mapping_qidx;
	}

	MTWF_PRINT("Dma scheduler info:\n");
	HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_ADDR, &value);
	pktin_int_refill_ena = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_PKTIN_INT_REFILL_ENA_MASK) ? TRUE : FALSE;
	pdma_add_int_refill_ena = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_PDMA_ADD_INT_REFILL_ENA_MASK) ? TRUE : FALSE;
	ple_add_int_refill_ena = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_PLE_ADD_INT_REFILL_ENA_MASK) ? TRUE : FALSE;
	ple_sub_ena = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_PLE_SUB_ENA_MASK) ? TRUE : FALSE;
	hif_ask_sub_ena = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_HIF_ASK_SUB_ENA_MASK) ? TRUE : FALSE;
	wacpu_mode_en = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_WACPU_MODE_EN_MASK) ? TRUE : FALSE;
	MTWF_PRINT("DMASHDL Ctrl Signal(0x5000A018): 0x%08x\n", value);
	MTWF_PRINT("\twacpu mode en(BIT0) = %d\n", wacpu_mode_en);
	MTWF_PRINT("\thif_ask_sub_ena(BIT16) = %d\n", hif_ask_sub_ena);
	MTWF_PRINT("\tple_sub_ena(BIT17) = %d\n", ple_sub_ena);
	MTWF_PRINT("\tple_add_int_refill_ena(BIT29) = %d\n", ple_add_int_refill_ena);
	MTWF_PRINT("\tpdma_add_int_refill_ena(BIT30) = %d\n", pdma_add_int_refill_ena);
	MTWF_PRINT("\tpktin_int_refill(BIT31)_ena = %d\n", pktin_int_refill_ena);
	HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, &value);
	ple_pkt_max_sz = (value & WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_MASK)
				>> WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_SHFT;
	pse_pkt_max_sz = (value & WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_MASK)
				>> WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_SHFT;
	MTWF_PRINT("DMASHDL Packet_max_size(0x5000A01c): 0x%08x\n", value);
	MTWF_PRINT("PLE/PSE packet max size=0x%03x/0x%03x\n",
			  ple_pkt_max_sz, pse_pkt_max_sz);
	HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_ERROR_FLAG_CTRL_ADDR, &value);
	MTWF_PRINT("DMASHDL ERR FLAG CTRL(0x5000A09c): 0x%08x\n", value);
	HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_STATUS_RD_ADDR, &value);
	ffa_cnt = (value & WF_HIF_DMASHDL_TOP_STATUS_RD_FFA_CNT_MASK) >> WF_HIF_DMASHDL_TOP_STATUS_RD_FFA_CNT_SHFT;
	free_pg_cnt = (value & WF_HIF_DMASHDL_TOP_STATUS_RD_FREE_PAGE_CNT_MASK) >> WF_HIF_DMASHDL_TOP_STATUS_RD_FREE_PAGE_CNT_SHFT;
	MTWF_PRINT("DMASHDL Status_RD(0x5000A100): 0x%08x\n", value);
	MTWF_PRINT("free page cnt = 0x%03x, ffa cnt = 0x%03x\n", free_pg_cnt, ffa_cnt);

	for (groupidx = 0; groupidx < 16; groupidx++) {
		idx = 0;
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "Group %d info:", groupidx);
		HW_IO_READ32(pAd->hdev_ctrl, status_addr, &value);
		rsv_cnt = (value & WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_RSV_CNT_MASK)
				>> WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_RSV_CNT_SHFT;
		src_cnt = (value & WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_SRC_CNT_MASK)
				>> WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_SRC_CNT_SHFT;
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"\tDMASHDL Status_RD_GP%d(0x%08x): 0x%08x\n", groupidx, status_addr, value);
		HW_IO_READ32(pAd->hdev_ctrl, quota_addr, &value);
		max_quota = (value & WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MAX_QUOTA_MASK)
				>> WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MAX_QUOTA_SHFT;
		min_quota = (value & WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MIN_QUOTA_MASK)
				>> WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MIN_QUOTA_SHFT;
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"\tDMASHDL Group%d control(0x%08x): 0x%08x\n", groupidx, quota_addr, value);

		if ((groupidx & 0x1) == 0) {
			HW_IO_READ32(pAd->hdev_ctrl, pkt_cnt_addr, &value);
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
				"\tDMASHDL RD_group_pkt_cnt_%d(0x%08x): 0x%08x\n", groupidx / 2, pkt_cnt_addr, value);
			odd_group_pktin_cnt = (value & WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP1_PKTIN_CNT_MASK)
					>> WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP1_PKTIN_CNT_SHFT;
			odd_group_ask_cnt = (value & WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP1_ASK_CNT_MASK)
					>> WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP1_ASK_CNT_SHFT;
			pktin_cnt = (value & WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP0_PKTIN_CNT_MASK)
					>> WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP0_PKTIN_CNT_SHFT;
			ask_cnt = (value & WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP0_ASK_CNT_MASK)
					>> WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP0_ASK_CNT_SHFT;
		} else {
			pktin_cnt = odd_group_pktin_cnt;
			ask_cnt = odd_group_ask_cnt;
		}

		MTWF_PRINT("\trsv_cnt = 0x%03x, src_cnt = 0x%03x\n", rsv_cnt, src_cnt);
		MTWF_PRINT("\tmax/min quota = 0x%03x/ 0x%03x\n", max_quota, min_quota);
		MTWF_PRINT("\tpktin_cnt = 0x%02x, ask_cnt = 0x%02x", pktin_cnt, ask_cnt);

		if (hif_ask_sub_ena && pktin_cnt != ask_cnt) {
			MTWF_PRINT(", mismatch!");
			is_mismatch = TRUE;
		}

		MTWF_PRINT("\n");

		if (groupidx == 15 && Group_Mapping_Q[groupidx] == 0) { /* Group15 is for PSE */
			pse_src_cnt = src_cnt;
			pse_rsv_cnt = rsv_cnt;
			break;
		}

		MTWF_PRINT("\tMapping Qidx:");

		while (Group_Mapping_Q[groupidx] != 0) {
			if (Group_Mapping_Q[groupidx] & 0x1)
				MTWF_PRINT("Q%d ", idx);

			Group_Mapping_Q[groupidx] >>= 1;
			idx++;
		}

		MTWF_PRINT("\n");
		total_src_cnt += src_cnt;
		total_rsv_cnt += rsv_cnt;
		status_addr = status_addr + 4;
		quota_addr = quota_addr + 4;

		if (groupidx & 0x1)
			pkt_cnt_addr = pkt_cnt_addr + 4;
	}

	MTWF_PRINT("\nCounter Check:\n");
	MAC_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_HIF_PG_INFO_ADDR, &value);
	ple_rpg_hif = (value & WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_SHFT;
	ple_upg_hif = (value & WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_SHFT;
	MTWF_PRINT("PLE:\n\tThe used/reserved pages of PLE HIF group=0x%03x/0x%03x\n",
			  ple_upg_hif, ple_rpg_hif);
	MAC_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_HIF1_PG_INFO_ADDR, &value);
	pse_rpg_hif = (value & WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_SHFT;
	pse_upg_hif = (value & WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_SHFT;
	MTWF_PRINT("PSE:\n\tThe used/reserved pages of PSE HIF group=0x%03x/0x%03x\n",
			  pse_upg_hif, pse_rpg_hif);
	MTWF_PRINT("DMASHDL:\n\tThe total used pages of group0~14=0x%03x",
			  total_src_cnt);

	if (ple_upg_hif != total_src_cnt) {
		MTWF_PRINT(", mismatch!");
		is_mismatch = TRUE;
	}

	MTWF_PRINT("\n");
	MTWF_PRINT("\tThe total reserved pages of group0~14=0x%03x\n",
			  total_rsv_cnt);
	MTWF_PRINT("\tThe total ffa pages of group0~14=0x%03x\n",
			  ffa_cnt);
	MTWF_PRINT("\tThe total free pages of group0~14=0x%03x",
			  free_pg_cnt);

	if (free_pg_cnt != total_rsv_cnt + ffa_cnt) {
		MTWF_PRINT(", mismatch(total_rsv_cnt + ffa_cnt in DMASHDL)");
		is_mismatch = TRUE;
	}

	if (free_pg_cnt != ple_rpg_hif) {
		MTWF_PRINT(", mismatch(reserved pages in PLE)");
		is_mismatch = TRUE;
	}

	MTWF_PRINT("\n");
	MTWF_PRINT("\tThe used pages of group15=0x%03x", pse_src_cnt);

	if (pse_upg_hif != pse_src_cnt) {
		MTWF_PRINT(", mismatch!");
		is_mismatch = TRUE;
	}

	MTWF_PRINT("\n");
	MTWF_PRINT("\tThe reserved pages of group15=0x%03x", pse_rsv_cnt);

	if (pse_rpg_hif != pse_rsv_cnt) {
		MTWF_PRINT(", mismatch!");
		is_mismatch = TRUE;
	}

	MTWF_PRINT("\n");

	if (!is_mismatch)
		MTWF_PRINT("DMASHDL: no counter mismatch\n");

	return TRUE;
}

#ifdef MT7992_FPGA
static INT32 chip_show_txv_info(struct hdev_ctrl *ctrl, void *arg)
{
	MTWF_PRINT("failed");
	return 0;
}
#endif /*MT7992_FPGA*/


#ifdef RANDOM_PKT_GEN
INT32 RandomTxCtrl;
UINT32 Qidmapping[16] = {0};
UINT32 pause_period;
uint8_t random_drop = FALSE;

INT chip_set_txctrl_proc(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 testcase = 0;
	RTMP_STRING *pfp  = NULL;
	UINT32 tmp_value;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pfp = strsep(&arg, ":");

	if (pfp == NULL)
		return FALSE;

	RandomTxCtrl = os_str_tol(pfp, 0, 10);

	if (arg != NULL)
		testcase = os_str_toul(arg, 0, 16);

	MTWF_PRINT("%s(): (RandomTxCtrl = %d) testcase: 0x%x\n",
		__func__, RandomTxCtrl, testcase);
	if (RandomTxCtrl == 0) {
		UINT i;

		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_ADDR, 0x42104210);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_QUEUE_MAPPING1_ADDR, 0x42104210);

		for (i = 0; i < cap->qos.WmmHwNum; i++) {
			Qidmapping[i] = 0;
			Qidmapping[i + 1] = 1;
			Qidmapping[i + 2] = 2;
			Qidmapping[i + 4] = 4;
		}
	} else if (RandomTxCtrl == 1) {
		UINT i;

		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_ADDR, 0x7654b210);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_QUEUE_MAPPING1_ADDR, 0xb210ba98);

		for (i = 0; i < cap->qos.WmmHwNum * 4; i++) {
			Qidmapping[i] = i % 12;

			if (Qidmapping[i] == 3)
				Qidmapping[i] = 11;
		}
	} else if (RandomTxCtrl == 2) {
		UINT i;

		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_ADDR, 0x89ab0124);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_QUEUE_MAPPING1_ADDR, 0x01244567);

		for (i = 0; i < cap->qos.WmmHwNum * 4; i++) {
			Qidmapping[i] = (15 - i) % 12;

			if (Qidmapping[i] == 3)
				Qidmapping[i] = 4;
		}
	}

	if (testcase & BIT0) {
		/* default setting */
		HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_ADDR, &tmp_value);
		tmp_value |= WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_HIF_ASK_SUB_ENA_MASK;
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_ADDR, tmp_value);
		HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, &tmp_value);
		tmp_value &= ~(WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_MASK | WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_MASK);
		tmp_value |= (0x1 << WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_SHFT);
		tmp_value |= (0x8 << WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_SHFT);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, tmp_value);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_ADDR, 0x6012345f);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING1_ADDR, 0xedcba987);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_ADDR, 0x76543210);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING1_ADDR, 0xfedcba98);
		HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PAGE_SETTING_ADDR, &tmp_value);
		tmp_value |= WF_HIF_DMASHDL_TOP_PAGE_SETTING_GROUP_SEQUENCE_ORDER_TYPE_MASK;
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PAGE_SETTING_ADDR, tmp_value);
		pause_period = 0;
		random_drop = 0;
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE0_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE1_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE2_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE3_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE4_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE5_ADDR, 0);
	}

	if (testcase & BIT1) {
		/* disable cr_hif_ask_sub_ena, ple_packet_max_size = 6 */
		HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_ADDR, &tmp_value);
		tmp_value &= ~WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_HIF_ASK_SUB_ENA_MASK;
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_ADDR, tmp_value);
		HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, &tmp_value);
		tmp_value &= ~WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_MASK;
		tmp_value |= (0x8 << WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_SHFT);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, tmp_value);
	}

	if (testcase & BIT2) {
		/* modify schedular priority(0x5000a0b0, 0x5000a0b4, 0x5000a0c4, 0x5000a0c8) */
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_ADDR, 0x6012345f);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING1_ADDR, 0xedcba987);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_ADDR, 0x6012345f);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING1_ADDR, 0xedcba987);
	}

	if (testcase & BIT3) {
		/* disable User program group sequence type control (0x5000a00c[16]) */
		HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PAGE_SETTING_ADDR, &tmp_value);
		tmp_value &= ~WF_HIF_DMASHDL_TOP_PAGE_SETTING_GROUP_SEQUENCE_ORDER_TYPE_MASK;
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PAGE_SETTING_ADDR, tmp_value);
	}

	if (testcase & BIT4) {
		if (pause_period == 0)
			pause_period = 120;
		else {
			pause_period = 0;
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE0_ADDR, 0);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE1_ADDR, 0);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE2_ADDR, 0);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE3_ADDR, 0);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE4_ADDR, 0);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE5_ADDR, 0);
		}
	}

	if (testcase & BIT5)
		random_drop = (random_drop == 1) ? 0 : 1;

	if (testcase & BIT6)
		random_drop = (random_drop == 2) ? 0 : 2;

	return TRUE;
}

VOID chip_regular_pause_umac(struct hdev_ctrl *ctrl)
{
	RTMP_ADAPTER *pAd = ctrl->priv;

	if (pause_period == 0)
		return;

	if ((pAd->Mlme.PeriodicRound % (pause_period * 2)) == 0) {
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE0_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE1_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE2_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE3_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE4_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE5_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE6_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE7_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE8_ADDR, 0);
	} else if ((pAd->Mlme.PeriodicRound % pause_period) == 0) {
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE0_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE1_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE2_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE3_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE4_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE5_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE6_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE7_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE8_ADDR, 0xffffffff);

	} else if (random_drop
			   && ((pAd->Mlme.PeriodicRound + pause_period / 2) % (pause_period * 2)) == 0) {
		UINT32 ple_stat[4] = {0};
		INT32 i, j;
		UINT32 hfid;
		UINT32 deq_fid;

		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC0_QUEUE_EMPTY0_ADDR, &ple_stat[0]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC1_QUEUE_EMPTY0_ADDR, &ple_stat[1]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC2_QUEUE_EMPTY0_ADDR, &ple_stat[2]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC3_QUEUE_EMPTY0_ADDR, &ple_stat[3]);

		for (j = 0; j < 4; j++) {
			for (i = 0; i < 32; i++) {
				if (((ple_stat[j] & (0x1 << i)) >> i) == 0) {
					UINT32 fl_que_ctrl[4] = {0};

					fl_que_ctrl[0] |= (0x1 << WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_SHFT);
					fl_que_ctrl[0] |= (0x2 << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
					fl_que_ctrl[0] |= (j << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
					fl_que_ctrl[0] |= (i << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_WLANID_SHFT);
					HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
					HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_2_ADDR, &fl_que_ctrl[1]);
					HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_3_ADDR, &fl_que_ctrl[2]);
					hfid = fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK;

					if (hfid == 0xfff)
						continue;

					fl_que_ctrl[0] |= (0x2 << WF_PLE_TOP_C_DE_QUEUE_0_DEQ_SUB_TYPE_SHFT);

					if (random_drop == 2)
						fl_que_ctrl[0] |= (0x9 << WF_PLE_TOP_C_DE_QUEUE_0_ENQ_SUB_TYPE_SHFT);

					fl_que_ctrl[1] = (hfid << WF_PLE_TOP_C_DE_QUEUE_1_CUR_LIST_FID_END_SHFT)
								| (hfid << WF_PLE_TOP_C_DE_QUEUE_1_CUR_LIST_FID_START_SHFT);
					HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_C_DE_QUEUE_1_ADDR, fl_que_ctrl[1]);

					if (random_drop == 2) {
						/* fl_que_ctrl[3] = 0x3 << 30; */
						fl_que_ctrl[3] |= 0x1f << WF_PLE_TOP_C_DE_QUEUE_2_DEQ_ENQ_DST_QID_SHFT;
						HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_C_DE_QUEUE_2_ADDR, fl_que_ctrl[3]);
					}

					HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_C_DE_QUEUE_0_ADDR, fl_que_ctrl[0]);
					HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_C_DE_QUEUE_3_ADDR, &fl_que_ctrl[2]);
					deq_fid = fl_que_ctrl[2] & WF_PLE_TOP_C_DE_QUEUE_3_DEQ_HEAD_FID_MASK;

					if (deq_fid == 0xfff || random_drop == 2)
						continue;

					fl_que_ctrl[0] = WF_PLE_TOP_C_EN_QUEUE_0_EXECUTE_MASK;
					fl_que_ctrl[0] |= (0x1 << WF_PLE_TOP_C_EN_QUEUE_0_SUB_TYPE_SHFT);
					fl_que_ctrl[0] |= (0x3 << WF_PLE_TOP_C_EN_QUEUE_0_DST_PID_SHFT);
					fl_que_ctrl[0] |= (0x1f << WF_PLE_TOP_C_EN_QUEUE_0_ENQ_DST_QID_SHFT);
					fl_que_ctrl[1] = (deq_fid << WF_PLE_TOP_C_EN_QUEUE_1_CUR_LIST_FID_END_SHFT)
							| (deq_fid << WF_PLE_TOP_C_EN_QUEUE_1_CUR_LIST_FID_START_SHFT);
					HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_C_EN_QUEUE_1_ADDR, fl_que_ctrl[1]);
					HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_C_EN_QUEUE_0_ADDR, fl_que_ctrl[0]);
				}
			}
		}
	}
}
#endif


static VOID chip_show_bcn_info(struct hdev_ctrl *ctrl, UCHAR bandidx)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 mac_val = 0;
	UINT32 idx;
	UINT32 band_offset = 0x10000 * bandidx;

	Show_Mib_Info_Proc(pAd, "");
	MTWF_PRINT("\n");

	show_mib_proc(pAd, (bandidx == DBDC_BAND0) ? "0" : "1");
	MTWF_PRINT("\n");

	show_trinfo_proc(pAd, "");
	MTWF_PRINT("\n");

	ShowPLEInfo(pAd, NULL);
	MTWF_PRINT("\n");

#ifdef ERR_RECOVERY
	ShowSerProc2(pAd, "");
	MTWF_PRINT("\n");
#endif

	ShowPseInfo(pAd, NULL);
	MTWF_PRINT("\n");

	show_tpinfo_proc(pAd, "0");
	MTWF_PRINT("\n");

	Show_MibBucket_Proc(pAd, "");
	MTWF_PRINT("\n");

	MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_ARB_TOP_SCR_ADDR + band_offset, &mac_val);
	MTWF_PRINT("ARB_SCR=0x%08x\n", mac_val);

	for (idx = 0; idx < 10; idx++) {
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_ARB_TOP_BFCR_ADDR + band_offset, &mac_val);
		MTWF_PRINT("ARB_BFCR=0x%08x (loop %d)\n", mac_val, idx);
	}

	MTWF_PRINT("\n");
}

static UINT32 chip_show_sta_acq_info(RTMP_ADAPTER *pAd, UINT32 *ple_stat,
				   UINT32 *sta_pause, UINT32 *dis_sta_map,
				   UINT32 dumptxd)
{
	int i, j;
	UINT32 total_nonempty_cnt = 0;

	for (j = 0; j < DRR_MAX_DW_ALL_AC(pAd); j++) { /* show AC Q info */
		for (i = 0; i < 32; i++) {
			if (((ple_stat[j + 1] & (0x1 << i)) >> i) == 0) {
				UINT32 hfid, tfid, pktcnt, ac_num = j / DRR_MAX_DW_PER_AC(pAd), ctrl = 0;
				UINT32 sta_num = i + (j % DRR_MAX_DW_PER_AC(pAd)) * 32, fl_que_ctrl[3] = {0};
				struct wifi_dev *wdev = wdev_search_by_wcid(pAd, sta_num);
				UINT32 wmmidx = 0;

				if (wdev)
					wmmidx = HcGetWmmIdx(pAd, wdev);

				MTWF_PRINT("\tSTA%d AC%d: ", sta_num, ac_num);

				fl_que_ctrl[0] |= WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (ENUM_UMAC_LMAC_PORT_2 << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (ac_num << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
				fl_que_ctrl[0] |= (sta_num << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_WLANID_SHFT);
				HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
				HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_2_ADDR, &fl_que_ctrl[1]);
				HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_3_ADDR, &fl_que_ctrl[2]);
				hfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
				tfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
				pktcnt = (fl_que_ctrl[2] & WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
				MTWF_PRINT("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x",
						  tfid, hfid, pktcnt);

				if (((sta_pause[j % DRR_MAX_DW_PER_AC(pAd)] & 0x1 << i) >> i) == 1)
					ctrl = 2;

				if (((dis_sta_map[j % DRR_MAX_DW_PER_AC(pAd)] & 0x1 << i) >> i) == 1)
					ctrl = 1;

				MTWF_PRINT(" ctrl = %s", sta_ctrl_reg[ctrl]);
				MTWF_PRINT(" (wmmidx=%d)\n", wmmidx);

				total_nonempty_cnt++;

				if (pktcnt > 0 && dumptxd > 0)
					ShowTXDInfo(pAd, hfid);
			}
		}
	}

	return total_nonempty_cnt;
}

static UINT32 chip_show_sta_acq_info_new(RTMP_ADAPTER *pAd, UINT32 *ple_stat,
				   struct pause_bitmap_ *ppause_bitmap, UINT32 dumptxd)
{
	int i, j;
	UINT32 total_nonempty_cnt = 0;

	if (!ppause_bitmap || !ppause_bitmap->pause || !ppause_bitmap->twt_pause)
		return total_nonempty_cnt;

	for (j = 0; j < DRR_MAX_DW_ALL_AC(pAd); j++) { /* show AC Q info */
		for (i = 0; i < 32; i++) {
			if (((ple_stat[j + 1] & (0x1 << i)) >> i) == 0) {
				UINT32 hfid, tfid, pktcnt, ac_num = j / DRR_MAX_DW_PER_AC(pAd), ctrl = 0;
				UINT32 sta_num = i + (j % DRR_MAX_DW_PER_AC(pAd)) * 32, fl_que_ctrl[3] = {0};
				struct wifi_dev *wdev = wdev_search_by_wcid(pAd, sta_num);
				UINT32 wmmidx = 0;
				UINT8 band_idx = 0;

				if (wdev) {
					wmmidx = HcGetWmmIdx(pAd, wdev);
					band_idx = HcGetBandByWdev(wdev);
				}

				MTWF_PRINT("\tSTA%d AC%d: ", sta_num, ac_num);

				fl_que_ctrl[0] |= WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (ENUM_UMAC_LMAC_PORT_2 << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (ac_num << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
				fl_que_ctrl[0] |= (sta_num << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_WLANID_SHFT);
				HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
				HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_2_ADDR, &fl_que_ctrl[1]);
				HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_3_ADDR, &fl_que_ctrl[2]);
				hfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
				tfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
				pktcnt = (fl_que_ctrl[2] & WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
				MTWF_PRINT("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x",
						  tfid, hfid, pktcnt);

				MTWF_PRINT(" BN%d", band_idx);
				if (ppause_bitmap->pause[band_idx] &&
					(((ppause_bitmap->pause[band_idx][j % DRR_MAX_DW_PER_AC(pAd)] & 0x1 << i) >> i) == 1)) {
					ctrl = 2;
				}

				/* TBD : no disable */
				MTWF_PRINT(" ctrl = %s", sta_ctrl_reg[ctrl]);

				if (ppause_bitmap->twt_pause[band_idx]
					&& (j < DRR_MAX_DW_TWT(pAd))
					&& (((ppause_bitmap->twt_pause[band_idx][j % DRR_MAX_DW_TWT(pAd)] & 0x1 << i) >> i) == 1)) {
					ctrl = 3;
					MTWF_PRINT(" ctrl2 = %s", sta_ctrl_reg[ctrl]);
				}

				MTWF_PRINT(" (wmmidx=%d)\n", wmmidx);

				total_nonempty_cnt++;

				if (pktcnt > 0 && dumptxd > 0)
					ShowTXDInfo(pAd, hfid);
			}
		}
	}

	return total_nonempty_cnt;
}

static VOID chip_show_nonempty_txcmdq_info(RTMP_ADAPTER *pAd,
										   UINT32 ple_txcmd_stat,
										   UINT32 band,
										   UINT32 dumptxd)
{
	int i;

	if (band >= PD_GET_BAND_NUM(pAd->physical_dev))
		return;

	for (i = 0; i < 32 ; i++) {
		if (((ple_txcmd_stat & (0x1 << i)) >> i) == 0) {
			UINT32 hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (ple_txcmd_queue_empty_info[band][i].QueueName != NULL) {
				MTWF_PRINT("\t%s: ", ple_txcmd_queue_empty_info[band][i].QueueName);
				fl_que_ctrl[0] |= WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (ple_txcmd_queue_empty_info[band][i].Portid <<
							WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (ple_txcmd_queue_empty_info[band][i].Queueid <<
							WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
			} else
				continue;

			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_2_ADDR, &fl_que_ctrl[1]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_3_ADDR, &fl_que_ctrl[2]);
			hfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
			tfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
			pktcnt = (fl_que_ctrl[2] & WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
			MTWF_PRINT("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x\n",
					  tfid, hfid, pktcnt);
			if (pktcnt > 0 && dumptxd > 0)
				ShowTXDInfo(pAd, hfid);
		}
	}
}

static VOID chip_show_txcmdq_info(RTMP_ADAPTER *pAd,
								  UINT32 ple_txcmd_stat,
								  UINT32 dumptxd)
{
	int band_idx;
	UINT32 ple_native_txcmd_stat = 0;
	UINT32 TXCMD_STAT_ADDR = 0, NATIVE_TXCMD_STAT_ADDR = 0;

	for (band_idx = 0; band_idx < PD_GET_BAND_NUM(pAd->physical_dev) ; band_idx++) {
		switch (band_idx) {
		case 0:
			TXCMD_STAT_ADDR = WF_PLE_TOP_TXCMD_QUEUE_EMPTY_ADDR;
			NATIVE_TXCMD_STAT_ADDR = WF_PLE_TOP_NATIVE_TXCMD_QUEUE_EMPTY_ADDR;
			break;
		case 1:
			TXCMD_STAT_ADDR = WF_PLE_TOP_BN1_TXCMD_QUEUE_EMPTY_ADDR;
			NATIVE_TXCMD_STAT_ADDR = WF_PLE_TOP_BN1_NATIVE_TXCMD_QUEUE_EMPTY_ADDR;
			break;
		default:
			return;
		}

		HW_IO_READ32(pAd->hdev_ctrl, TXCMD_STAT_ADDR, &ple_txcmd_stat);
		HW_IO_READ32(pAd->hdev_ctrl, NATIVE_TXCMD_STAT_ADDR, &ple_native_txcmd_stat);

		MTWF_PRINT("BN%d non-native/native txcmd queue empty = %x/%x\n",
				band_idx, ple_txcmd_stat,
				ple_native_txcmd_stat);

		chip_show_nonempty_txcmdq_info(
			pAd, ple_native_txcmd_stat, band_idx, dumptxd);
	}
}

static VOID chip_get_ple_acq_stat(RTMP_ADAPTER *pAd, UINT32 *ple_stat)
{
	INT32 j = 0;
	UINT32 addr = 0;

	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_QUEUE_EMPTY_ADDR, &ple_stat[0]);

	addr = WF_PLE_TOP_AC0_QUEUE_EMPTY0_ADDR;
	for (j = 0; j < DRR_MAX_DW_PER_AC(pAd); j++, addr += 4) {
		if (j < DRR_MAX_DW_PER_AC(pAd) - 1)
			HW_IO_READ32(pAd->hdev_ctrl, addr, &ple_stat[j + 1]);
		else {
			/* not continuous address, 0x0680 */
			addr = WF_PLE_TOP_AC0_QUEUE_EMPTY_EXT0_ADDR;
			HW_IO_READ32(pAd->hdev_ctrl, addr, &ple_stat[j + 1]);
		}
	}

	addr = WF_PLE_TOP_AC1_QUEUE_EMPTY0_ADDR;
	for (; j < DRR_MAX_DW_PER_AC(pAd) * 2; j++, addr += 4) {
		if (j < (DRR_MAX_DW_PER_AC(pAd) * 2 - 1))
			HW_IO_READ32(pAd->hdev_ctrl, addr, &ple_stat[j + 1]);
		else {
			/* not continuous address, 0x0780 */
			addr = WF_PLE_TOP_AC1_QUEUE_EMPTY_EXT0_ADDR;
			HW_IO_READ32(pAd->hdev_ctrl, addr, &ple_stat[j + 1]);
		}
	}

	addr = WF_PLE_TOP_AC2_QUEUE_EMPTY0_ADDR;
	for (; j < DRR_MAX_DW_PER_AC(pAd) * 3; j++, addr += 4) {
		if (j < (DRR_MAX_DW_PER_AC(pAd) * 3 - 1))
			HW_IO_READ32(pAd->hdev_ctrl, addr, &ple_stat[j + 1]);
		else {
			/* not continuous address, 0x0880 */
			addr = WF_PLE_TOP_AC2_QUEUE_EMPTY_EXT0_ADDR;
			HW_IO_READ32(pAd->hdev_ctrl, addr, &ple_stat[j + 1]);
		}
	}

	addr = WF_PLE_TOP_AC3_QUEUE_EMPTY0_ADDR;
	for (; j < DRR_MAX_DW_PER_AC(pAd) * 4; j++, addr += 4) {
		if (j < (DRR_MAX_DW_PER_AC(pAd) * 4 - 1))
			HW_IO_READ32(pAd->hdev_ctrl, addr, &ple_stat[j + 1]);
		else {
			/* not continuous address, 0x0980 */
			addr = WF_PLE_TOP_AC3_QUEUE_EMPTY_EXT0_ADDR;
			HW_IO_READ32(pAd->hdev_ctrl, addr, &ple_stat[j + 1]);
		}
	}
}

static VOID chip_get_ple_txcmd_stat(RTMP_ADAPTER *pAd, UINT32 *ple_txcmd_stat)
{
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_NATIVE_TXCMD_QUEUE_EMPTY_ADDR, ple_txcmd_stat);
}

static VOID chip_get_dis_sta_map(RTMP_ADAPTER *pAd, UINT32 *dis_sta_map)
{
}

static VOID chip_get_sta_pause_by_band(RTMP_ADAPTER *pAd, UINT32 *sta_pause, UINT32 *twt_pause, UINT8 band_idx)
{
	INT32 j = 0;
	UINT32 addr = 0;

	/* switch to target band */
	HW_IO_WRITE32(pAd->hdev_ctrl, WF_DRR_TOP_SBRR_ADDR,
		((WF_DRR_TOP_SBRR_TARGET_BAND_MASK & band_idx) << WF_DRR_TOP_SBRR_TARGET_BAND_SHFT));

	/* Legacy */
	addr = WF_DRR_TOP_AC0_STATION_PAUSE00_ADDR;
	for (j = 0; j < DRR_MAX_DW_PER_AC(pAd); j++, addr += 4) {
		if (j < DRR_MAX_DW_PER_AC(pAd) - 1)
			HW_IO_READ32(pAd->hdev_ctrl, addr, &sta_pause[j]);
		else {
			/* not continuous address, 0x0280 */
			addr = WF_DRR_TOP_AC0_STATION_PAUSE_EXT_00_ADDR;
			HW_IO_READ32(pAd->hdev_ctrl, addr, &sta_pause[j]);
		}
	}

	addr = WF_DRR_TOP_AC1_STATION_PAUSE00_ADDR;
	for (; j < DRR_MAX_DW_PER_AC(pAd) * 2; j++, addr += 4) {
		if (j < (DRR_MAX_DW_PER_AC(pAd) * 2 - 1))
			HW_IO_READ32(pAd->hdev_ctrl, addr, &sta_pause[j]);
		else {
			/* not continuous address, 0x0380 */
			addr = WF_DRR_TOP_AC1_STATION_PAUSE_EXT_00_ADDR;
			HW_IO_READ32(pAd->hdev_ctrl, addr, &sta_pause[j]);
		}
	}

	addr = WF_DRR_TOP_AC2_STATION_PAUSE00_ADDR;
	for (; j < DRR_MAX_DW_PER_AC(pAd) * 3; j++, addr += 4) {
		if (j < (DRR_MAX_DW_PER_AC(pAd) * 3 - 1))
			HW_IO_READ32(pAd->hdev_ctrl, addr, &sta_pause[j]);
		else {
			/* not continuous address, 0x0480 */
			addr = WF_DRR_TOP_AC2_STATION_PAUSE_EXT_00_ADDR;
			HW_IO_READ32(pAd->hdev_ctrl, addr, &sta_pause[j]);
		}
	}

	addr = WF_DRR_TOP_AC3_STATION_PAUSE00_ADDR;
	for (; j < DRR_MAX_DW_PER_AC(pAd) * 4; j++, addr += 4) {
		if (j < (DRR_MAX_DW_PER_AC(pAd) * 4 - 1))
			HW_IO_READ32(pAd->hdev_ctrl, addr, &sta_pause[j]);
		else {
			/* not continuous address, 0x0580 */
			addr = WF_DRR_TOP_AC3_STATION_PAUSE_EXT_00_ADDR;
			HW_IO_READ32(pAd->hdev_ctrl, addr, &sta_pause[j]);
		}
	}

	/* TWT */
	addr = WF_DRR_TOP_TWT_STA_MAP00_ADDR;
	for (j = 0; j < DRR_MAX_DW_TWT(pAd); j++, addr += 4) {
		if (j < DRR_MAX_DW_TWT(pAd) - 1)
			HW_IO_READ32(pAd->hdev_ctrl, addr, &twt_pause[j]);
		else {
			/* not continuous address, 0x0480 */
			addr = WF_DRR_TOP_TWT_STA_MAP_EXT_00_ADDR;
			HW_IO_READ32(pAd->hdev_ctrl, addr, &twt_pause[j]);
		}
	}

}

static VOID chip_get_sta_pause(RTMP_ADAPTER *pAd, UINT32 *sta_pause)
{
}

static INT32 chip_show_drr_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	return TRUE;
}

static INT32 get_pause_by_band(struct hdev_ctrl *ctrl, RTMP_STRING *arg, UINT8 band_idx)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	struct pause_bitmap_ *ppause_bitmap = &(pAd->physical_dev->pause_bitmap);

	if (ppause_bitmap && ppause_bitmap->pause && ppause_bitmap->twt_pause) {
		chip_get_sta_pause_by_band(pAd, &(ppause_bitmap->pause[band_idx][0]), &(ppause_bitmap->twt_pause[band_idx][0]), 0);
		return TRUE;
	} else
		return FALSE;
}

static INT32 chip_show_ple_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 ple_buf_ctrl = 0, pg_sz, pg_num;
	UINT32 *ple_stat = NULL;
	UINT32 pg_flow_ctrl[10] = {0};
	UINT32 ple_native_txcmd_stat = 0;
	UINT32 fpg_cnt, ffa_cnt, fpg_head, fpg_tail, hif_max_q, hif_min_q;
	UINT32 rpg_hif, upg_hif, cpu_max_q, cpu_min_q, rpg_cpu, upg_cpu;
	INT32 i, j;
	UINT32 dumptxd = 0;
	UINT32 QueryCR = 0;
	CHAR *param = NULL;
	struct pause_bitmap_ *ppause_bitmap = &(pAd->physical_dev->pause_bitmap);

	/* iwpriv ra0 show pleinfo=[dumptxd]-[QueryCR] */

	param = rstrtok(arg, "-");

	if (param) {
		dumptxd = os_str_toul(arg, 0, 16);
		param = rstrtok(NULL, "-");

		if (param)
			QueryCR = os_str_toul(param, 0, 10);
	} else if (arg != NULL)
		dumptxd = os_str_toul(arg, 0, 16);

	os_alloc_mem(NULL, (UCHAR **)&ple_stat, ((DRR_MAX_DW_ALL_AC(pAd) + 1) * 4));
	if (!ple_stat)
		goto end;
	os_zero_mem(ple_stat, ((DRR_MAX_DW_ALL_AC(pAd) + 1) * 4));

	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_PBUF_CTRL_ADDR, &ple_buf_ctrl);
	chip_get_ple_acq_stat(pAd, ple_stat);
	chip_get_ple_txcmd_stat(pAd, &ple_native_txcmd_stat);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FREEPG_CNT_ADDR, &pg_flow_ctrl[0]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FREEPG_HEAD_TAIL_ADDR, &pg_flow_ctrl[1]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_PG_HIF_GROUP_ADDR, &pg_flow_ctrl[2]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_HIF_PG_INFO_ADDR, &pg_flow_ctrl[3]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_PG_CPU_GROUP_ADDR, &pg_flow_ctrl[4]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_CPU_PG_INFO_ADDR, &pg_flow_ctrl[5]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_PG_HIF_TXCMD_GROUP_ADDR, &pg_flow_ctrl[6]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_HIF_TXCMD_PG_INFO_ADDR, &pg_flow_ctrl[7]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_PG_HIF_WMTXD_GROUP_ADDR, &pg_flow_ctrl[8]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_HIF_WMTXD_PG_INFO_ADDR, &pg_flow_ctrl[9]);

	/* TBD */
	/* chip_get_dis_sta_map(pAd, dis_sta_map); */
	if (DRR_QUERY_PAUSE_BY_CMD(pAd) && (!QueryCR))
		get_sta_pause_by_cmd(pAd);
	else {
		if (ppause_bitmap && ppause_bitmap->pause && ppause_bitmap->twt_pause) {
			chip_get_sta_pause_by_band(pAd, &(ppause_bitmap->pause[0][0]), &(ppause_bitmap->twt_pause[0][0]), 0);
			chip_get_sta_pause_by_band(pAd, &(ppause_bitmap->pause[1][0]), &(ppause_bitmap->twt_pause[1][0]), 1);
		}
	}

	/* Configuration Info */
	MTWF_PRINT("PLE Configuration Info:\n");
	MTWF_PRINT("\tPacket Buffer Control(0x82060014): 0x%08x\n", ple_buf_ctrl);
	pg_sz = (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_MASK) >> WF_PLE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_SHFT;
	MTWF_PRINT("\t\tPage Size=%d(%d bytes per page)\n", pg_sz, (pg_sz == 1 ? 128 : 64));
	MTWF_PRINT("\t\tPage Offset=%d(in unit of 2KB)\n",
			 (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_PBUF_OFFSET_MASK) >> WF_PLE_TOP_PBUF_CTRL_PBUF_OFFSET_SHFT);
	pg_num = (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_MASK) >> WF_PLE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_SHFT;
	MTWF_PRINT("\t\tTotal Page=%d pages\n", pg_num);
	/* Page Flow Control */
	MTWF_PRINT("PLE Page Flow Control:\n");
	MTWF_PRINT("\tFree page counter: 0x%08x\n", pg_flow_ctrl[0]);
	fpg_cnt = (pg_flow_ctrl[0] & WF_PLE_TOP_FREEPG_CNT_FREEPG_CNT_MASK) >> WF_PLE_TOP_FREEPG_CNT_FREEPG_CNT_SHFT;
	MTWF_PRINT("\t\tThe toal page number of free=0x%03x\n", fpg_cnt);
	ffa_cnt = (pg_flow_ctrl[0] & WF_PLE_TOP_FREEPG_CNT_FFA_CNT_MASK) >> WF_PLE_TOP_FREEPG_CNT_FFA_CNT_SHFT;
	MTWF_PRINT("\t\tThe free page numbers of free for all=0x%03x\n", ffa_cnt);
	MTWF_PRINT("\tFree page head and tail: 0x%08x\n", pg_flow_ctrl[1]);
	fpg_head = (pg_flow_ctrl[1] & WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_MASK) >> WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_SHFT;
	fpg_tail = (pg_flow_ctrl[1] & WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_MASK) >> WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_SHFT;
	MTWF_PRINT("\t\tThe tail/head page of free page list=0x%03x/0x%03x\n", fpg_tail, fpg_head);
	MTWF_PRINT("\tReserved page counter of HIF group: 0x%08x\n", pg_flow_ctrl[2]);
	MTWF_PRINT("\tHIF group page status: 0x%08x\n", pg_flow_ctrl[3]);
	hif_min_q = (pg_flow_ctrl[2] & WF_PLE_TOP_PG_HIF_GROUP_HIF_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_GROUP_HIF_MIN_QUOTA_SHFT;
	hif_max_q = (pg_flow_ctrl[2] & WF_PLE_TOP_PG_HIF_GROUP_HIF_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_GROUP_HIF_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of HIF group=0x%03x/0x%03x\n", hif_max_q, hif_min_q);
	rpg_hif = (pg_flow_ctrl[3] & WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_SHFT;
	upg_hif = (pg_flow_ctrl[3] & WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of HIF group=0x%03x/0x%03x\n", upg_hif, rpg_hif);

	MTWF_PRINT("\tReserved page counter of WMTXD group: 0x%08x\n", pg_flow_ctrl[8]);
	MTWF_PRINT("\tWMTXD group page status: 0x%08x\n", pg_flow_ctrl[9]);
	cpu_min_q = (pg_flow_ctrl[8] & WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MIN_QUOTA_SHFT;
	cpu_max_q = (pg_flow_ctrl[8] & WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of WMTXD group=0x%03x/0x%03x\n", cpu_max_q, cpu_min_q);
	rpg_cpu = (pg_flow_ctrl[9] & WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_RSV_CNT_MASK) >> WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_RSV_CNT_SHFT;
	upg_cpu = (pg_flow_ctrl[9] & WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_SRC_CNT_MASK) >> WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of WMTXD group=0x%03x/0x%03x\n", upg_cpu, rpg_cpu);

	MTWF_PRINT("\tReserved page counter of HIF_TXCMD group: 0x%08x\n", pg_flow_ctrl[6]);
	MTWF_PRINT("\tHIF_TXCMD group page status: 0x%08x\n", pg_flow_ctrl[7]);
	cpu_min_q = (pg_flow_ctrl[6] & WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MIN_QUOTA_SHFT;
	cpu_max_q = (pg_flow_ctrl[6] & WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of HIF_TXCMD group=0x%03x/0x%03x\n", cpu_max_q, cpu_min_q);
	rpg_cpu = (pg_flow_ctrl[7] & WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_RSV_CNT_MASK) >> WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_RSV_CNT_SHFT;
	upg_cpu = (pg_flow_ctrl[7] & WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_SRC_CNT_MASK) >> WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of HIF_TXCMD group=0x%03x/0x%03x\n", upg_cpu, rpg_cpu);

	MTWF_PRINT("\tReserved page counter of CPU group(0x820c0150): 0x%08x\n", pg_flow_ctrl[4]);
	MTWF_PRINT("\tCPU group page status(0x820c0154): 0x%08x\n", pg_flow_ctrl[5]);
	cpu_min_q = (pg_flow_ctrl[4] & WF_PLE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_SHFT;
	cpu_max_q = (pg_flow_ctrl[4] & WF_PLE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of CPU group=0x%03x/0x%03x\n", cpu_max_q, cpu_min_q);
	rpg_cpu = (pg_flow_ctrl[5] & WF_PLE_TOP_CPU_PG_INFO_CPU_RSV_CNT_MASK) >> WF_PLE_TOP_CPU_PG_INFO_CPU_RSV_CNT_SHFT;
	upg_cpu = (pg_flow_ctrl[5] & WF_PLE_TOP_CPU_PG_INFO_CPU_SRC_CNT_MASK) >> WF_PLE_TOP_CPU_PG_INFO_CPU_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of CPU group=0x%03x/0x%03x\n", upg_cpu, rpg_cpu);

	if ((ple_stat[0] & WF_PLE_TOP_QUEUE_EMPTY_ALL_AC_EMPTY_MASK) == 0) {
		for (j = 0; j < DRR_MAX_DW_ALL_AC(pAd); j++) {
			if (j % DRR_MAX_DW_PER_AC(pAd) == 0) {
				MTWF_PRINT("\n\tNonempty AC%d Q of STA#: ", j / DRR_MAX_DW_PER_AC(pAd));
			}

			for (i = 0; i < 32; i++) {
				if (((ple_stat[j + 1] & (0x1 << i)) >> i) == 0) {
					MTWF_PRINT("%d ", i + (j % DRR_MAX_DW_PER_AC(pAd)) * 32);
				}
			}
		}

		MTWF_PRINT("\n");
	}

	MTWF_PRINT("Nonempty Q info:\n");

	for (i = 0; i < 32; i++) {
		if (((ple_stat[0] & (0x1 << i)) >> i) == 0) {
			UINT32 hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (ple_queue_empty_info[i].QueueName != NULL) {
				MTWF_PRINT("\t%s: ", ple_queue_empty_info[i].QueueName);
				fl_que_ctrl[0] |= WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (ple_queue_empty_info[i].Portid << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (ple_queue_empty_info[i].tgid << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_TGID_SHFT);
				fl_que_ctrl[0] |= (ple_queue_empty_info[i].Queueid << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
			} else
				continue;

			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_2_ADDR, &fl_que_ctrl[1]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_3_ADDR, &fl_que_ctrl[2]);
			hfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >> WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
			tfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >> WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
			pktcnt = (fl_que_ctrl[2] & WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >> WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
			MTWF_PRINT("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x\n",
					  tfid, hfid, pktcnt);

			if (pktcnt > 0 && dumptxd > 0)
				ShowTXDInfo(pAd, hfid);
		}
	}

	chip_show_sta_acq_info_new(pAd, ple_stat, ppause_bitmap, dumptxd);
	chip_show_txcmdq_info(pAd, ple_native_txcmd_stat, dumptxd);

end:
	if (ple_stat)
		os_free_mem(ple_stat);

	return TRUE;
}

static INT32 chip_show_amsdu_info(struct hdev_ctrl *ctrl)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 ple_stat[8] = {0}, total_amsdu = 0;
	UCHAR i;

	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_1_MSDU_CNT_ADDR, &ple_stat[0]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_2_MSDU_CNT_ADDR, &ple_stat[1]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_3_MSDU_CNT_ADDR, &ple_stat[2]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_4_MSDU_CNT_ADDR, &ple_stat[3]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_5_MSDU_CNT_ADDR, &ple_stat[4]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_6_MSDU_CNT_ADDR, &ple_stat[5]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_7_MSDU_CNT_ADDR, &ple_stat[6]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_8_MSDU_CNT_ADDR, &ple_stat[7]);

	MTWF_PRINT("TXD counter status of MSDU:\n");

	for (i = 0; i < 8; i++)
		total_amsdu += ple_stat[i];

	for (i = 0; i < 8; i++) {
		MTWF_PRINT("AMSDU pack count of %d MSDU in TXD: 0x%x ", i+1, ple_stat[i]);
		if (total_amsdu != 0)
			MTWF_PRINT("(%d%%)\n", ple_stat[i] * 100 / total_amsdu);
		else
			MTWF_PRINT("\n");
	}

	return TRUE;
}

#define NO_SHIFT_DEFINE 0xFFFFFFFF

static UINT32
io_r_32(RTMP_ADAPTER *pAd, UINT32 addr)
{
	UINT32 value = 0;

	RTMP_IO_READ32(pAd->hdev_ctrl, addr, &value);

	return value;
}

UINT32
halWtblWriteRaw(
	RTMP_ADAPTER *pAd,
	UINT_16  u2EntryIdx,
	ENUM_WTBL_TYPE_T  eType,
	UINT_16  u2DW,
	UINT_32  u4Value
)
{
	UINT32 u4WtblVmAddr = 0;

	if (eType == WTBL_TYPE_LMAC) {
		LWTBL_CONFIG(u2EntryIdx);
		u4WtblVmAddr = LWTBL_IDX2BASE(u2EntryIdx, u2DW);
	} else if (eType == WTBL_TYPE_UMAC) {
		UWTBL_CONFIG(u2EntryIdx);
		u4WtblVmAddr = UWTBL_IDX2BASE(u2EntryIdx, u2DW);
	} else if (eType == WTBL_TYPE_KEY) {
		KEYTBL_CONFIG(u2EntryIdx);
		u4WtblVmAddr = KEYTBL_IDX2BASE(u2EntryIdx, u2DW);
	} else {
		/*TODO:*/
	}

	IO_W_32(u4WtblVmAddr, u4Value);

	return 0;
}

UINT32
halWtblReadRaw(
	RTMP_ADAPTER *pAd,
	UINT_16  u2EntryIdx,
	ENUM_WTBL_TYPE_T  eType,
	UINT_16  u2StartDW,
	UINT_16  u2LenInDW,
	PVOID    pBuffer
)
{
	UINT_32 *dest_cpy = (UINT_32 *)pBuffer;
	UINT_32 sizeInDW = u2LenInDW;
	UINT_32 u4SrcAddr = 0;

	if (pBuffer == NULL)
		return 0xFF;

	if (eType == WTBL_TYPE_LMAC) {
		LWTBL_CONFIG(u2EntryIdx);
		u4SrcAddr = LWTBL_IDX2BASE(u2EntryIdx, u2StartDW);
	} else if (eType == WTBL_TYPE_UMAC) {
		UWTBL_CONFIG(u2EntryIdx);
		u4SrcAddr = UWTBL_IDX2BASE(u2EntryIdx, u2StartDW);
	} else if (eType == WTBL_TYPE_KEY) {
		KEYTBL_CONFIG(u2EntryIdx);
		u4SrcAddr = KEYTBL_IDX2BASE(u2EntryIdx, u2StartDW);
	} else {
		/* TODO: */
	}

	while (sizeInDW--) {
		*dest_cpy++ = IO_R_32(u4SrcAddr);
		u4SrcAddr += 4;
	}

	return 0;
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW0[] = {
	{"MUAR_IDX",    WF_LWTBL_MUAR_MASK, WF_LWTBL_MUAR_SHIFT,	FALSE},
	{"RCA1",        WF_LWTBL_RCA1_MASK, NO_SHIFT_DEFINE,	FALSE},
	{"KID",         WF_LWTBL_KID_MASK,  WF_LWTBL_KID_SHIFT,	FALSE},
	{"RCID",        WF_LWTBL_RCID_MASK, NO_SHIFT_DEFINE,	FALSE},
	{"BAND",        WF_LWTBL_BAND_MASK, WF_LWTBL_BAND_SHIFT,	FALSE},
	{"RV",          WF_LWTBL_RV_MASK,   NO_SHIFT_DEFINE,	FALSE},
	{"RCA2",        WF_LWTBL_RCA2_MASK, NO_SHIFT_DEFINE,	FALSE},
	{"WPI_FLAG",    WF_LWTBL_WPI_FLAG_MASK, NO_SHIFT_DEFINE,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW0_1(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	MTWF_PRINT("\t\n");
	MTWF_PRINT("LinkAddr: %02x:%02x:%02x:%02x:%02x:%02x(D0[B0~15], D1[B0~31])\n",
		lwtbl[4], lwtbl[5], lwtbl[6], lwtbl[7], lwtbl[0], lwtbl[1]);

	/* LMAC WTBL DW 0 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 0/1\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_PEER_INFO_DW_0*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW0[i].name) {

		if (WTBL_LMAC_DW0[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW0[i].name,
					 (dw_value & WTBL_LMAC_DW0[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW0[i].name,
					  (dw_value & WTBL_LMAC_DW0[i].mask) >> WTBL_LMAC_DW0[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW2[] = {
	{"AID",                 WF_LWTBL_AID_MASK,              WF_LWTBL_AID_SHIFT,	FALSE},
	{"GID_SU",              WF_LWTBL_GID_SU_MASK,           NO_SHIFT_DEFINE,	FALSE},
	{"DUAL_PTEC_EN",        WF_LWTBL_DUAL_PTEC_EN_MASK,     NO_SHIFT_DEFINE,	FALSE},
	{"DUAL_CTS_CAP",        WF_LWTBL_DUAL_CTS_CAP_MASK,     NO_SHIFT_DEFINE,	FALSE},
	{"CIPHER_PGTK",         WF_LWTBL_CIPHER_SUIT_PGTK_MASK, WF_LWTBL_CIPHER_SUIT_PGTK_SHIFT,	TRUE},
	{"FROM_DS",             WF_LWTBL_FD_MASK,               NO_SHIFT_DEFINE,	FALSE},
	{"TO_DS",               WF_LWTBL_TD_MASK,               NO_SHIFT_DEFINE,	FALSE},
	{"SW",                  WF_LWTBL_SW_MASK,               NO_SHIFT_DEFINE,	FALSE},
	{"UL",                  WF_LWTBL_UL_MASK,               NO_SHIFT_DEFINE,	FALSE},
	{"TX_POWER_SAVE",       WF_LWTBL_TX_PS_MASK,            NO_SHIFT_DEFINE,	TRUE},
	{"QOS",                 WF_LWTBL_QOS_MASK,              NO_SHIFT_DEFINE,	FALSE},
	{"HT",                  WF_LWTBL_HT_MASK,               NO_SHIFT_DEFINE,	FALSE},
	{"VHT",                 WF_LWTBL_VHT_MASK,              NO_SHIFT_DEFINE,	FALSE},
	{"HE",                  WF_LWTBL_HE_MASK,               NO_SHIFT_DEFINE,	FALSE},
	{"EHT",                 WF_LWTBL_EHT_MASK,              NO_SHIFT_DEFINE,	FALSE},
	{"MESH",                WF_LWTBL_MESH_MASK,             NO_SHIFT_DEFINE,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW2(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 2 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 2\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_2*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW2[i].name) {

		if (WTBL_LMAC_DW2[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW2[i].name,
					 (dw_value & WTBL_LMAC_DW2[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW2[i].name,
					  (dw_value & WTBL_LMAC_DW2[i].mask) >> WTBL_LMAC_DW2[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW3[] = {
	{"WMM_Q",           WF_LWTBL_WMM_Q_MASK,                WF_LWTBL_WMM_Q_SHIFT,	FALSE},
	{"EHT_SIG_MCS",     WF_LWTBL_EHT_SIG_MCS_MASK,          WF_LWTBL_EHT_SIG_MCS_SHIFT,	FALSE},
	{"HDRT_MODE",       WF_LWTBL_HDRT_MODE_MASK,            NO_SHIFT_DEFINE,	FALSE},
	{"BEAM_CHG",        WF_LWTBL_BEAM_CHG_MASK,             NO_SHIFT_DEFINE,	FALSE},
	{"EHT_LTF_SYM_NUM", WF_LWTBL_EHT_LTF_SYM_NUM_OPT_MASK,  WF_LWTBL_EHT_LTF_SYM_NUM_OPT_SHIFT,	TRUE},
	{"PFMU_IDX",        WF_LWTBL_PFMU_IDX_MASK,             WF_LWTBL_PFMU_IDX_SHIFT,	FALSE},
	{"ULPF_IDX",        WF_LWTBL_ULPF_IDX_MASK,             WF_LWTBL_ULPF_IDX_SHIFT,	FALSE},
	{"RIBF",            WF_LWTBL_RIBF_MASK,                 NO_SHIFT_DEFINE,	FALSE},
	{"BYPASS_TXSMM",    WF_LWTBL_BYPASS_TXSMM_MASK,         NO_SHIFT_DEFINE,	TRUE},
	{"TBF_HT",          WF_LWTBL_TBF_HT_MASK,               NO_SHIFT_DEFINE,	FALSE},
	{"TBF_HT",          WF_LWTBL_TBF_HT_MASK,               NO_SHIFT_DEFINE,	FALSE},
	{"TBF_VHT",         WF_LWTBL_TBF_VHT_MASK,              NO_SHIFT_DEFINE,	FALSE},
	{"TBF_HE",          WF_LWTBL_TBF_HE_MASK,               NO_SHIFT_DEFINE,	FALSE},
	{"TBF_EHT",         WF_LWTBL_TBF_EHT_MASK,              NO_SHIFT_DEFINE,	FALSE},
	{"IGN_FBK",         WF_LWTBL_IGN_FBK_MASK,              NO_SHIFT_DEFINE,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW3(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 3 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 3\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_3*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW3[i].name) {

		if (WTBL_LMAC_DW3[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW3[i].name,
					 (dw_value & WTBL_LMAC_DW3[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW3[i].name,
					  (dw_value & WTBL_LMAC_DW3[i].mask) >> WTBL_LMAC_DW3[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW4[] = {
	{"PE",              WF_LWTBL_PE_MASK,           WF_LWTBL_PE_SHIFT,	FALSE},
	{"DIS_RHTR",        WF_LWTBL_DIS_RHTR_MASK,     NO_SHIFT_DEFINE,	FALSE},
	{"LDPC_HT",         WF_LWTBL_LDPC_HT_MASK,      NO_SHIFT_DEFINE,	FALSE},
	{"LDPC_VHT",        WF_LWTBL_LDPC_VHT_MASK,     NO_SHIFT_DEFINE,	FALSE},
	{"LDPC_HE",         WF_LWTBL_LDPC_HE_MASK,      NO_SHIFT_DEFINE,	FALSE},
	{"LDPC_EHT",	    WF_LWTBL_LDPC_EHT_MASK,	NO_SHIFT_DEFINE,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW4(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 4 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 4\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_4*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW4[i].name) {
		if (WTBL_LMAC_DW4[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW4[i].name,
					 (dw_value & WTBL_LMAC_DW4[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW4[i].name,
					  (dw_value & WTBL_LMAC_DW4[i].mask) >> WTBL_LMAC_DW4[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW5[] = {
	{"AF",                  WF_LWTBL_AF_MASK,           WF_LWTBL_AF_SHIFT,		FALSE},
	{"RTS",                 WF_LWTBL_RTS_MASK,          NO_SHIFT_DEFINE,	FALSE},
	{"SMPS",                WF_LWTBL_SMPS_MASK,         NO_SHIFT_DEFINE,	FALSE},
	{"DYN_BW",              WF_LWTBL_DYN_BW_MASK,       NO_SHIFT_DEFINE,	TRUE},
	{"MMSS",                WF_LWTBL_MMSS_MASK,         WF_LWTBL_MMSS_SHIFT,	FALSE},
	{"USR",                 WF_LWTBL_USR_MASK,          NO_SHIFT_DEFINE,	FALSE},
	{"SR_RATE",             WF_LWTBL_SR_R_MASK,         WF_LWTBL_SR_R_SHIFT,	FALSE},
	{"SR_ABORT",            WF_LWTBL_SR_ABORT_MASK,     NO_SHIFT_DEFINE,	TRUE},
	{"TX_POWER_OFFSET",     WF_LWTBL_TX_POWER_OFFSET_MASK,  WF_LWTBL_TX_POWER_OFFSET_SHIFT,	FALSE},
	{"LTF_EHT",		WF_LWTBL_LTF_EHT_MASK,      WF_LWTBL_LTF_EHT_SHIFT, FALSE},
	{"GI_EHT",		WF_LWTBL_GI_EHT_MASK,       WF_LWTBL_GI_EHT_SHIFT, FALSE},
	{"DOPPL",               WF_LWTBL_DOPPL_MASK,        NO_SHIFT_DEFINE,	FALSE},
	{"TXOP_PS_CAP",         WF_LWTBL_TXOP_PS_CAP_MASK,  NO_SHIFT_DEFINE,	FALSE},
	{"DONOT_UPDATE_I_PSM",  WF_LWTBL_DU_I_PSM_MASK,     NO_SHIFT_DEFINE,	TRUE},
	{"I_PSM",               WF_LWTBL_I_PSM_MASK,        NO_SHIFT_DEFINE,	FALSE},
	{"PSM",                 WF_LWTBL_PSM_MASK,          NO_SHIFT_DEFINE,	FALSE},
	{"SKIP_TX",             WF_LWTBL_SKIP_TX_MASK,      NO_SHIFT_DEFINE,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW5(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 5 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 5\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_5*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW5[i].name) {
		if (WTBL_LMAC_DW5[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW5[i].name,
					 (dw_value & WTBL_LMAC_DW5[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW5[i].name,
					  (dw_value & WTBL_LMAC_DW5[i].mask) >> WTBL_LMAC_DW5[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW6[] = {
	{"CBRN",        WF_LWTBL_CBRN_MASK,	    WF_LWTBL_CBRN_SHIFT,	FALSE},
	{"DBNSS_EN",    WF_LWTBL_DBNSS_EN_MASK, NO_SHIFT_DEFINE,	FALSE},
	{"BAF_EN",      WF_LWTBL_BAF_EN_MASK,   NO_SHIFT_DEFINE,	FALSE},
	{"RDGBA",       WF_LWTBL_RDGBA_MASK,    NO_SHIFT_DEFINE,	FALSE},
	{"RDG",         WF_LWTBL_R_MASK,        NO_SHIFT_DEFINE,	FALSE},
	{"SPE_IDX",     WF_LWTBL_SPE_IDX_MASK,  WF_LWTBL_SPE_IDX_SHIFT,	TRUE},
	{"G2",          WF_LWTBL_G2_MASK,       NO_SHIFT_DEFINE,	FALSE},
	{"G4",          WF_LWTBL_G4_MASK,       NO_SHIFT_DEFINE,	FALSE},
	{"G8",          WF_LWTBL_G8_MASK,       NO_SHIFT_DEFINE,	FALSE},
	{"G16",         WF_LWTBL_G16_MASK,      NO_SHIFT_DEFINE,	TRUE},
	{"G2_LTF",      WF_LWTBL_G2_LTF_MASK,   WF_LWTBL_G2_LTF_SHIFT,	FALSE},
	{"G4_LTF",      WF_LWTBL_G4_LTF_MASK,   WF_LWTBL_G4_LTF_SHIFT,	FALSE},
	{"G8_LTF",      WF_LWTBL_G8_LTF_MASK,   WF_LWTBL_G8_LTF_SHIFT,	FALSE},
	{"G16_LTF",     WF_LWTBL_G16_LTF_MASK,  WF_LWTBL_G16_LTF_SHIFT,	TRUE},
	{"G2_HE",       WF_LWTBL_G2_HE_MASK,    WF_LWTBL_G2_HE_SHIFT,	FALSE},
	{"G4_HE",       WF_LWTBL_G4_HE_MASK,    WF_LWTBL_G4_HE_SHIFT,	FALSE},
	{"G8_HE",       WF_LWTBL_G8_HE_MASK,    WF_LWTBL_G8_HE_SHIFT,	FALSE},
	{"G16_HE",      WF_LWTBL_G16_HE_MASK,   WF_LWTBL_G16_HE_SHIFT,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW6(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 6 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 6\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_6*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW6[i].name) {
		if (WTBL_LMAC_DW6[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW6[i].name,
					 (dw_value & WTBL_LMAC_DW6[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW6[i].name,
					  (dw_value & WTBL_LMAC_DW6[i].mask) >> WTBL_LMAC_DW6[i].shift);
		i++;
	}
}

static VOID parse_fmac_lwtbl_DW7(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	int i = 0;

	/* LMAC WTBL DW 7 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 7\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_7*4]);
	dw_value = *addr;

	for (i = 0; i < 8; i++) {
		MTWF_PRINT("\tBA_WIN_SIZE%u:%lu\n", i, ((dw_value & BITS(i*4, i*4+3)) >> i*4));
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW8[] = {
	{"RTS_FAIL_CNT_AC0",    WF_LWTBL_AC0_RTS_FAIL_CNT_MASK,	WF_LWTBL_AC0_RTS_FAIL_CNT_SHIFT,	FALSE},
	{"RTS_FAIL_CNT_AC1",    WF_LWTBL_AC1_RTS_FAIL_CNT_MASK,	WF_LWTBL_AC1_RTS_FAIL_CNT_SHIFT,	FALSE},
	{"RTS_FAIL_CNT_AC2",    WF_LWTBL_AC2_RTS_FAIL_CNT_MASK,	WF_LWTBL_AC2_RTS_FAIL_CNT_SHIFT,	FALSE},
	{"RTS_FAIL_CNT_AC3",    WF_LWTBL_AC3_RTS_FAIL_CNT_MASK,	WF_LWTBL_AC3_RTS_FAIL_CNT_SHIFT,	TRUE},
	{"PARTIAL_AID",         WF_LWTBL_PARTIAL_AID_MASK,      WF_LWTBL_PARTIAL_AID_SHIFT,	FALSE},
	{"CHK_PER",             WF_LWTBL_CHK_PER_MASK,          NO_SHIFT_DEFINE,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW8(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 8 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 8\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_8*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW8[i].name) {
		if (WTBL_LMAC_DW8[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW8[i].name,
					 (dw_value & WTBL_LMAC_DW8[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW8[i].name,
					  (dw_value & WTBL_LMAC_DW8[i].mask) >> WTBL_LMAC_DW8[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW9[] = {
	{"RX_AVG_MPDU_SIZE",    WF_LWTBL_RX_AVG_MPDU_SIZE_MASK,    WF_LWTBL_RX_AVG_MPDU_SIZE_SHIFT,	FALSE},
	{"PRITX_SW_MODE",       WF_LWTBL_PRITX_SW_MODE_MASK,       NO_SHIFT_DEFINE,	FALSE},
	{"PRITX_SW_MODE",       WF_LWTBL_PRITX_SW_MODE_MASK,       NO_SHIFT_DEFINE,	FALSE},
	{"PRITX_ERSU",	        WF_LWTBL_PRITX_ERSU_MASK,	   NO_SHIFT_DEFINE,	FALSE},
	{"PRITX_PLR",           WF_LWTBL_PRITX_PLR_MASK,           NO_SHIFT_DEFINE,	TRUE},
	{"PRITX_DCM",           WF_LWTBL_PRITX_DCM_MASK,           NO_SHIFT_DEFINE,	FALSE},
	{"PRITX_ER106T",        WF_LWTBL_PRITX_ER106T_MASK,        NO_SHIFT_DEFINE,	TRUE},
	{"FCAP",                WF_LWTBL_FCAP_MASK,                WF_LWTBL_FCAP_SHIFT,	FALSE},
	/*     {"FCAP(0:20 1:~40)",    WTBL_FCAP_20_TO_160_MHZ,
	WTBL_FCAP_20_TO_160_MHZ_OFFSET}, */
	{"MPDU_FAIL_CNT",       WF_LWTBL_MPDU_FAIL_CNT_MASK,       WF_LWTBL_MPDU_FAIL_CNT_SHIFT,	FALSE},
	{"MPDU_OK_CNT",         WF_LWTBL_MPDU_OK_CNT_MASK,         WF_LWTBL_MPDU_OK_CNT_SHIFT,	FALSE},
	{"RATE_IDX",            WF_LWTBL_RATE_IDX_MASK,            WF_LWTBL_RATE_IDX_SHIFT,	TRUE},
	{NULL,}
};

RTMP_STRING *fcap_name[] = {"20MHz", "20/40MHz", "20/40/80MHz", "20/40/80/160/80+80MHz", "20/40/80/160/80+80/320MHz"};

static VOID parse_fmac_lwtbl_DW9(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 9 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 9\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_9*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW9[i].name) {
		if (WTBL_LMAC_DW9[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW9[i].name,
					 (dw_value & WTBL_LMAC_DW9[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW9[i].name,
					  (dw_value & WTBL_LMAC_DW9[i].mask) >> WTBL_LMAC_DW9[i].shift);
		i++;
	}

	/* FCAP parser */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("FCAP:%s\n", fcap_name[(dw_value & WF_LWTBL_FCAP_MASK) >> WF_LWTBL_FCAP_SHIFT]);
}

#define HW_TX_RATE_TO_MODE(_x)			(((_x) & WTBL_RATE_TX_MODE_MASK) >> WTBL_RATE_TX_MODE_OFFSET)
#define HW_TX_RATE_TO_MCS(_x, _mode)		((_x) & WTBL_RATE_TX_RATE_MASK >> WTBL_RATE_TX_RATE_OFFSET)
#define HW_TX_RATE_TO_NSS(_x)			(((_x) & WTBL_RATE_NSTS_MASK) >> WTBL_RATE_NSTS_OFFSET)
#define HW_TX_RATE_TO_STBC(_x)			(((_x) & WTBL_RATE_STBC_MASK) >> WTBL_RATE_STBC_OFFSET)

#define MAX_TX_MODE 16
static char *HW_TX_MODE_STR[] = {"CCK", "OFDM", "HT-Mix", "HT-GF", "VHT",
				 "N/A", "N/A", "N/A",
				 "HE_SU", "HE_EXT_SU", "HE_TRIG", "HE_MU",
				 "N/A",
				 "EHT_EXT_SU", "EHT_TRIG", "EHT_MU",
				 "N/A"};
static char *HW_TX_RATE_CCK_STR[] = {"1M", "2Mlong", "5.5Mlong", "11Mlong", "N/A", "2Mshort", "5.5Mshort", "11Mshort", "N/A"};
static char *HW_TX_RATE_OFDM_STR[] = {"6M", "9M", "12M", "18M", "24M", "36M", "48M", "54M", "N/A"};

static char *hw_rate_ofdm_str(uint16_t ofdm_idx)
{
	switch (ofdm_idx) {
	case 11: /* 6M */
		return HW_TX_RATE_OFDM_STR[0];

	case 15: /* 9M */
		return HW_TX_RATE_OFDM_STR[1];

	case 10: /* 12M */
		return HW_TX_RATE_OFDM_STR[2];

	case 14: /* 18M */
		return HW_TX_RATE_OFDM_STR[3];

	case 9: /* 24M */
		return HW_TX_RATE_OFDM_STR[4];

	case 13: /* 36M */
		return HW_TX_RATE_OFDM_STR[5];

	case 8: /* 48M */
		return HW_TX_RATE_OFDM_STR[6];

	case 12: /* 54M */
		return HW_TX_RATE_OFDM_STR[7];

	default:
		return HW_TX_RATE_OFDM_STR[8];
	}
}

static char *hw_rate_str(uint8_t mode, uint16_t rate_idx)
{
	if (mode == 0)
		return rate_idx < 8 ? HW_TX_RATE_CCK_STR[rate_idx] : HW_TX_RATE_CCK_STR[8];
	else if (mode == 1)
		return hw_rate_ofdm_str(rate_idx);
	else
		return "MCS";
}

static VOID parse_rate(RTMP_ADAPTER *pAd, uint16_t rate_idx, uint16_t txrate)
{
	uint16_t txmode, mcs, nss, stbc;

	txmode = HW_TX_RATE_TO_MODE(txrate);
	mcs = HW_TX_RATE_TO_MCS(txrate, txmode);
	nss = HW_TX_RATE_TO_NSS(txrate);
	stbc = HW_TX_RATE_TO_STBC(txrate);

	MTWF_PRINT("\tRate%d(0x%x):TxMode=%d(%s), TxRate=%d(%s), Nsts=%d, STBC=%d\n",
			  rate_idx + 1, txrate,
			  txmode, (txmode < MAX_TX_MODE ? HW_TX_MODE_STR[txmode] : HW_TX_MODE_STR[MAX_TX_MODE]),
			  mcs, hw_rate_str(txmode, mcs), nss, stbc);
}


static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
} WTBL_LMAC_DW10[] = {
	{"RATE1",       WF_LWTBL_RATE1_MASK,        WF_LWTBL_RATE1_SHIFT},
	{"RATE2",       WF_LWTBL_RATE2_MASK,        WF_LWTBL_RATE2_SHIFT},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW10(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 10 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 10\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_AUTO_RATE_1_2*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW10[i].name) {
		parse_rate(pAd, i, (dw_value & WTBL_LMAC_DW10[i].mask) >> WTBL_LMAC_DW10[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
} WTBL_LMAC_DW11[] = {
	{"RATE3",       WF_LWTBL_RATE3_MASK,        WF_LWTBL_RATE3_SHIFT},
	{"RATE4",       WF_LWTBL_RATE4_MASK,        WF_LWTBL_RATE4_SHIFT},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW11(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 11 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 11\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_AUTO_RATE_3_4*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW11[i].name) {
		parse_rate(pAd, i+2, (dw_value & WTBL_LMAC_DW11[i].mask) >> WTBL_LMAC_DW11[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
} WTBL_LMAC_DW12[] = {
	{"RATE5",       WF_LWTBL_RATE5_MASK,        WF_LWTBL_RATE5_SHIFT},
	{"RATE6",       WF_LWTBL_RATE6_MASK,        WF_LWTBL_RATE6_SHIFT},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW12(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 12 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 12\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_AUTO_RATE_5_6*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW12[i].name) {
		parse_rate(pAd, i+4, (dw_value & WTBL_LMAC_DW12[i].mask) >> WTBL_LMAC_DW12[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
} WTBL_LMAC_DW13[] = {
	{"RATE7",       WF_LWTBL_RATE7_MASK,        WF_LWTBL_RATE7_SHIFT},
	{"RATE8",       WF_LWTBL_RATE8_MASK,        WF_LWTBL_RATE8_SHIFT},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW13(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 13 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 13\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_AUTO_RATE_7_8*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW13[i].name) {
		parse_rate(pAd, i+6, (dw_value & WTBL_LMAC_DW13[i].mask) >> WTBL_LMAC_DW13[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW14_BMC[] = {
	{"CIPHER_IGTK",         WF_LWTBL_CIPHER_SUIT_IGTK_MASK,    WF_LWTBL_CIPHER_SUIT_IGTK_SHIFT,		FALSE},
	{"CIPHER_BIGTK",        WF_LWTBL_CIPHER_SUIT_BIGTK_MASK,   WF_LWTBL_CIPHER_SUIT_BIGTK_SHIFT,	TRUE},
	{NULL,}
};

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW14[] = {
	{"RATE1_TX_CNT",      WF_LWTBL_RATE1_TX_CNT_MASK,     WF_LWTBL_RATE1_TX_CNT_SHIFT,   FALSE},
	{"RATE1_FAIL_CNT",    WF_LWTBL_RATE1_FAIL_CNT_MASK,   WF_LWTBL_RATE1_FAIL_CNT_SHIFT, TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW14(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr, *muar_addr = 0;
	UINT_32 dw_value, muar_dw_value = 0;
	UINT_16 i = 0;

	/* DUMP DW14 for BMC entry only */
	muar_addr = (UINT_32 *)&(lwtbl[WF_LWTBL_MUAR_DW*4]);
	muar_dw_value = *muar_addr;
	if (((muar_dw_value & WF_LWTBL_MUAR_MASK) >> WF_LWTBL_MUAR_SHIFT)
		== MUAR_INDEX_OWN_MAC_ADDR_BC_MC) {
		/* LMAC WTBL DW 14 */
		MTWF_PRINT("\t\n");
		MTWF_PRINT("LWTBL DW 14_BMC\n");
		addr = (UINT_32 *)&(lwtbl[WF_LWTBL_CIPHER_SUIT_IGTK_DW*4]);
		dw_value = *addr;

		while (WTBL_LMAC_DW14_BMC[i].name) {
			if (WTBL_LMAC_DW14_BMC[i].shift == NO_SHIFT_DEFINE)
				MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW14_BMC[i].name,
					(dw_value & WTBL_LMAC_DW14_BMC[i].mask) ? 1 : 0);
			else
				MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW14_BMC[i].name,
					(dw_value & WTBL_LMAC_DW14_BMC[i].mask) >> WTBL_LMAC_DW14_BMC[i].shift);
			i++;
		}
	} else {
		MTWF_PRINT("\t\n");
		MTWF_PRINT("LWTBL DW 14\n");
		addr = (UINT_32 *)&(lwtbl[WF_LWTBL_CIPHER_SUIT_IGTK_DW*4]);
		dw_value = *addr;

		while (WTBL_LMAC_DW14[i].name) {
			if (WTBL_LMAC_DW14[i].shift == NO_SHIFT_DEFINE)
				MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW14[i].name,
					(dw_value & WTBL_LMAC_DW14[i].mask) ? 1 : 0);
			else
				MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW14[i].name,
					(dw_value & WTBL_LMAC_DW14[i].mask) >> WTBL_LMAC_DW14[i].shift);
			i++;
		}
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW28[] = {
	{"RELATED_IDX0",	WF_LWTBL_RELATED_IDX0_MASK,		WF_LWTBL_RELATED_IDX0_SHIFT,	FALSE},
	{"RELATED_BAND0",	WF_LWTBL_RELATED_BAND0_MASK,		WF_LWTBL_RELATED_BAND0_SHIFT,	FALSE},
	{"PRI_MLD_BAND",    WF_LWTBL_PRIMARY_MLD_BAND_MASK,		WF_LWTBL_PRIMARY_MLD_BAND_SHIFT,	TRUE},
	{"RELATED_IDX1",	WF_LWTBL_RELATED_IDX1_MASK,		WF_LWTBL_RELATED_IDX1_SHIFT,	FALSE},
	{"RELATED_BAND1",   WF_LWTBL_RELATED_BAND1_MASK,		WF_LWTBL_RELATED_BAND1_SHIFT,	FALSE},
	{"SEC_MLD_BAND",	WF_LWTBL_SECONDARY_MLD_BAND_MASK,	WF_LWTBL_SECONDARY_MLD_BAND_SHIFT,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW28(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 28 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 28\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_MLO_INFO_LINE_1*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW28[i].name) {
		if (WTBL_LMAC_DW28[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW28[i].name,
				(dw_value & WTBL_LMAC_DW28[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW28[i].name,
				(dw_value & WTBL_LMAC_DW28[i].mask) >>
					WTBL_LMAC_DW28[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW29[] = {
	{"DISPATCH_POLICY0", WF_LWTBL_DISPATCH_POLICY0_MASK,    WF_LWTBL_DISPATCH_POLICY0_SHIFT,	FALSE},
	{"DISPATCH_POLICY1", WF_LWTBL_DISPATCH_POLICY1_MASK,    WF_LWTBL_DISPATCH_POLICY1_SHIFT,	FALSE},
	{"DISPATCH_POLICY2", WF_LWTBL_DISPATCH_POLICY2_MASK,    WF_LWTBL_DISPATCH_POLICY2_SHIFT,	FALSE},
	{"DISPATCH_POLICY3", WF_LWTBL_DISPATCH_POLICY3_MASK,    WF_LWTBL_DISPATCH_POLICY3_SHIFT,	TRUE},
	{"DISPATCH_POLICY4", WF_LWTBL_DISPATCH_POLICY4_MASK,    WF_LWTBL_DISPATCH_POLICY4_SHIFT,	FALSE},
	{"DISPATCH_POLICY5", WF_LWTBL_DISPATCH_POLICY5_MASK,    WF_LWTBL_DISPATCH_POLICY5_SHIFT,	FALSE},
	{"DISPATCH_POLICY6", WF_LWTBL_DISPATCH_POLICY6_MASK,    WF_LWTBL_DISPATCH_POLICY6_SHIFT,	FALSE},
	{"DISPATCH_POLICY7", WF_LWTBL_DISPATCH_POLICY7_MASK,    WF_LWTBL_DISPATCH_POLICY7_SHIFT,	TRUE},
	{"OWN_MLD_ID",       WF_LWTBL_OWN_MLD_ID_MASK,          WF_LWTBL_OWN_MLD_ID_SHIFT,	FALSE},
	{"EMLSR0",		WF_LWTBL_EMLSR0_MASK,		NO_SHIFT_DEFINE,	FALSE},
	{"EMLMR0",		WF_LWTBL_EMLMR0_MASK,		NO_SHIFT_DEFINE,	FALSE},
	{"EMLSR1",		WF_LWTBL_EMLSR1_MASK,		NO_SHIFT_DEFINE,	FALSE},
	{"EMLMR1",		WF_LWTBL_EMLMR1_MASK,		NO_SHIFT_DEFINE,	TRUE},
	{"EMLSR2",		WF_LWTBL_EMLSR2_MASK,		NO_SHIFT_DEFINE,	FALSE},
	{"EMLMR2",		WF_LWTBL_EMLMR2_MASK,		NO_SHIFT_DEFINE,	FALSE},
	{"STR_BITMAP",	WF_LWTBL_STR_BITMAP_MASK,	WF_LWTBL_STR_BITMAP_SHIFT,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW29(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 29 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 29\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_MLO_INFO_LINE_2*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW29[i].name) {
		if (WTBL_LMAC_DW29[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW29[i].name,
				(dw_value & WTBL_LMAC_DW29[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW29[i].name,
				(dw_value & WTBL_LMAC_DW29[i].mask) >>
					WTBL_LMAC_DW29[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW30[] = {
	{"DISPATCH_ORDER",	WF_LWTBL_DISPATCH_ORDER_MASK,	WF_LWTBL_DISPATCH_ORDER_SHIFT,	FALSE},
	{"DISPATCH_RATIO",	WF_LWTBL_DISPATCH_RATIO_MASK,	WF_LWTBL_DISPATCH_RATIO_SHIFT,	FALSE},
	{"LINK_MGF",		WF_LWTBL_LINK_MGF_MASK,		WF_LWTBL_LINK_MGF_SHIFT,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW30(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 30 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 30\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_MLO_INFO_LINE_3*4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW30[i].name) {
		if (WTBL_LMAC_DW30[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW30[i].name,
				(dw_value & WTBL_LMAC_DW30[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW30[i].name,
				(dw_value & WTBL_LMAC_DW30[i].mask) >> WTBL_LMAC_DW30[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW31[] = {
	{"NEGO_WINSIZE0", WF_LWTBL_NEGOTIATED_WINSIZE0_MASK,  WF_LWTBL_NEGOTIATED_WINSIZE0_SHIFT,    FALSE},
	{"WINSIZE1",      WF_LWTBL_NEGOTIATED_WINSIZE1_MASK,  WF_LWTBL_NEGOTIATED_WINSIZE1_SHIFT,    FALSE},
	{"WINSIZE2",      WF_LWTBL_NEGOTIATED_WINSIZE2_MASK,  WF_LWTBL_NEGOTIATED_WINSIZE2_SHIFT,    FALSE},
	{"WINSIZE3",      WF_LWTBL_NEGOTIATED_WINSIZE3_MASK,  WF_LWTBL_NEGOTIATED_WINSIZE3_SHIFT,    TRUE},
	{"WINSIZE4",      WF_LWTBL_NEGOTIATED_WINSIZE4_MASK,  WF_LWTBL_NEGOTIATED_WINSIZE4_SHIFT,    FALSE},
	{"WINSIZE5",      WF_LWTBL_NEGOTIATED_WINSIZE5_MASK,  WF_LWTBL_NEGOTIATED_WINSIZE5_SHIFT,    FALSE},
	{"WINSIZE6",      WF_LWTBL_NEGOTIATED_WINSIZE6_MASK,  WF_LWTBL_NEGOTIATED_WINSIZE6_SHIFT,    FALSE},
	{"WINSIZE7",      WF_LWTBL_NEGOTIATED_WINSIZE7_MASK,  WF_LWTBL_NEGOTIATED_WINSIZE7_SHIFT,    TRUE},
	{"DROP",          WF_LWTBL_DROP_MASK,                 NO_SHIFT_DEFINE,    FALSE},
	{"CASCAD",        WF_LWTBL_CASCAD_MASK,               NO_SHIFT_DEFINE,    FALSE},
	{"ALL_ACK",       WF_LWTBL_ALL_ACK_MASK,              NO_SHIFT_DEFINE,    FALSE},
	{"MPDU_SIZE",     WF_LWTBL_MPDU_SIZE_MASK,            WF_LWTBL_MPDU_SIZE_SHIFT,  FALSE},
	{"BA_MODE",       WF_LWTBL_BA_MODE_MASK,              WF_LWTBL_BA_MODE_SHIFT,  TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW31(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 31 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 31\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RESP_INFO_DW_31*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW31[i].name) {
		if (WTBL_LMAC_DW31[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW31[i].name,
				(dw_value & WTBL_LMAC_DW31[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW31[i].name,
				(dw_value & WTBL_LMAC_DW31[i].mask) >>
					WTBL_LMAC_DW31[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW32[] = {
	{"OM_INFO",             WF_LWTBL_OM_INFO_MASK,             WF_LWTBL_OM_INFO_SHIFT,      FALSE},
	{"OM_INFO_EHT",         WF_LWTBL_OM_INFO_EHT_MASK,         WF_LWTBL_OM_INFO_EHT_SHIFT,  FALSE},
	{"RXD_DUP_FOR_OM_CHG",  WF_LWTBL_RXD_DUP_FOR_OM_CHG_MASK,  NO_SHIFT_DEFINE,             FALSE},
	{"RXD_DUP_WHITE_LIST",  WF_LWTBL_RXD_DUP_WHITE_LIST_MASK,  WF_LWTBL_RXD_DUP_WHITE_LIST_SHIFT,	FALSE},
	{"RXD_DUP_MODE",        WF_LWTBL_RXD_DUP_MODE_MASK,        WF_LWTBL_RXD_DUP_MODE_SHIFT,	FALSE},
	{"ACK_EN",              WF_LWTBL_ACK_EN_MASK,              NO_SHIFT_DEFINE,             TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW32(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 32 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 32\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RX_DUP_INFO_DW_32*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW32[i].name) {
		if (WTBL_LMAC_DW32[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW32[i].name,
				(dw_value & WTBL_LMAC_DW32[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW32[i].name,
				(dw_value & WTBL_LMAC_DW32[i].mask) >>
					WTBL_LMAC_DW32[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW33[] = {
	{"USER_RSSI",                   WF_LWTBL_USER_RSSI_MASK,            WF_LWTBL_USER_RSSI_SHIFT,	FALSE},
	{"USER_SNR",                    WF_LWTBL_USER_SNR_MASK,             WF_LWTBL_USER_SNR_SHIFT,	FALSE},
	{"RAPID_REACTION_RATE",         WF_LWTBL_RAPID_REACTION_RATE_MASK,  WF_LWTBL_RAPID_REACTION_RATE_SHIFT,	TRUE},
	{"HT_AMSDU(Read Only)",         WF_LWTBL_HT_AMSDU_MASK,             NO_SHIFT_DEFINE,	FALSE},
	{"AMSDU_CROSS_LG(Read Only)",   WF_LWTBL_AMSDU_CROSS_LG_MASK,       NO_SHIFT_DEFINE,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW33(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 33 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 33\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_1*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW33[i].name) {
		if (WTBL_LMAC_DW33[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW33[i].name,
				(dw_value & WTBL_LMAC_DW33[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW33[i].name,
				(dw_value & WTBL_LMAC_DW33[i].mask) >>
					WTBL_LMAC_DW33[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW34[] = {
	{"RESP_RCPI0",	WF_LWTBL_RESP_RCPI0_MASK,	WF_LWTBL_RESP_RCPI0_SHIFT,	FALSE},
	{"RCPI1",	WF_LWTBL_RESP_RCPI1_MASK,	WF_LWTBL_RESP_RCPI1_SHIFT,	FALSE},
	{"RCPI2",	WF_LWTBL_RESP_RCPI2_MASK,	WF_LWTBL_RESP_RCPI2_SHIFT,	FALSE},
	{"RCPI3",	WF_LWTBL_RESP_RCPI3_MASK,	WF_LWTBL_RESP_RCPI3_SHIFT,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW34(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 34 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 34\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_2*4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW34[i].name) {
		if (WTBL_LMAC_DW34[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW34[i].name,
				(dw_value & WTBL_LMAC_DW34[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW34[i].name,
				(dw_value & WTBL_LMAC_DW34[i].mask) >>
					WTBL_LMAC_DW34[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW35[] = {
	{"SNR 0",	WF_LWTBL_SNR_RX0_MASK,		WF_LWTBL_SNR_RX0_SHIFT,	FALSE},
	{"SNR 1",	WF_LWTBL_SNR_RX1_MASK,		WF_LWTBL_SNR_RX1_SHIFT,	FALSE},
	{"SNR 2",	WF_LWTBL_SNR_RX2_MASK,		WF_LWTBL_SNR_RX2_SHIFT,	FALSE},
	{"SNR 3",	WF_LWTBL_SNR_RX3_MASK,		WF_LWTBL_SNR_RX3_SHIFT,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW35(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 35 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("LWTBL DW 35\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_3*4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW35[i].name) {
		if (WTBL_LMAC_DW35[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_LMAC_DW35[i].name,
				(dw_value & WTBL_LMAC_DW35[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_LMAC_DW35[i].name,
				(dw_value & WTBL_LMAC_DW35[i].mask) >>
					WTBL_LMAC_DW35[i].shift);
		i++;
	}
}

static VOID parse_fmac_lwtbl_rx_stats(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	parse_fmac_lwtbl_DW33(pAd, lwtbl);
	parse_fmac_lwtbl_DW34(pAd, lwtbl);
	parse_fmac_lwtbl_DW35(pAd, lwtbl);
}

static VOID parse_fmac_lwtbl_mlo_info(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	parse_fmac_lwtbl_DW28(pAd, lwtbl);
	parse_fmac_lwtbl_DW29(pAd, lwtbl);
	parse_fmac_lwtbl_DW30(pAd, lwtbl);
}


static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_UMAC_DW9[] = {
	{"RELATED_IDX0",    WF_UWTBL_RELATED_IDX0_MASK,         WF_UWTBL_RELATED_IDX0_SHIFT,       FALSE},
	{"RELATED_BAND0",   WF_UWTBL_RELATED_BAND0_MASK,        WF_UWTBL_RELATED_BAND0_SHIFT,      FALSE},
	{"PRI_MLD_BAND",    WF_UWTBL_PRIMARY_MLD_BAND_MASK,     WF_UWTBL_PRIMARY_MLD_BAND_SHIFT,   TRUE},
	{"RELATED_IDX1",    WF_UWTBL_RELATED_IDX1_MASK,         WF_UWTBL_RELATED_IDX1_SHIFT,       FALSE},
	{"RELATED_BAND1",   WF_UWTBL_RELATED_BAND1_MASK,        WF_UWTBL_RELATED_BAND1_SHIFT,      FALSE},
	{"SEC_MLD_BAND",    WF_UWTBL_SECONDARY_MLD_BAND_MASK,   WF_UWTBL_SECONDARY_MLD_BAND_SHIFT, TRUE},
	{NULL,}
};

static VOID parse_fmac_uwtbl_mlo_info(RTMP_ADAPTER *pAd, uint8_t *uwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	MTWF_PRINT("\t\n");
	MTWF_PRINT("MldAddr: %02x:%02x:%02x:%02x:%02x:%02x(D0[B0~15], D1[B0~31])\n",
		uwtbl[4], uwtbl[5], uwtbl[6], uwtbl[7], uwtbl[0], uwtbl[1]);

	/* UMAC WTBL DW 0 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("UWTBL DW 0\n");
	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_OWN_MLD_ID_DW*4]);
	dw_value = *addr;

	MTWF_PRINT("\t%s:%u\n", "OMLD_ID",
		(dw_value & WF_UWTBL_OWN_MLD_ID_MASK) >> WF_UWTBL_OWN_MLD_ID_SHIFT);

	/* UMAC WTBL DW 9 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("UWTBL DW 9\n");
	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_RELATED_IDX0_DW*4]);
	dw_value = *addr;

	while (WTBL_UMAC_DW9[i].name) {

		if (WTBL_UMAC_DW9[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_UMAC_DW9[i].name,
				(dw_value & WTBL_UMAC_DW9[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_UMAC_DW9[i].name,
				 (dw_value & WTBL_UMAC_DW9[i].mask) >>
					WTBL_UMAC_DW9[i].shift);
		i++;
	}
}

static bool is_wtbl_bigtk_exist(RTMP_ADAPTER *pAd, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;

	addr = (UINT_32 *)&(lwtbl[WF_LWTBL_MUAR_DW*4]);
	dw_value = *addr;
	if (((dw_value & WF_LWTBL_MUAR_MASK) >> WF_LWTBL_MUAR_SHIFT) ==
					MUAR_INDEX_OWN_MAC_ADDR_BC_MC) {
		addr = (UINT_32 *)&(lwtbl[WF_LWTBL_CIPHER_SUIT_BIGTK_DW*4]);
		dw_value = *addr;
		if (((dw_value & WF_LWTBL_CIPHER_SUIT_BIGTK_MASK) >>
			WF_LWTBL_CIPHER_SUIT_BIGTK_SHIFT) != IGTK_CIPHER_SUIT_NONE)
			return TRUE;
	}

	return FALSE;
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_UMAC_DW2[] = {
	{"PN[31:0]",  WF_UWTBL_PN_31_0__MASK,  WF_UWTBL_PN_31_0__SHIFT,  TRUE},
	{NULL,}
};

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_UMAC_DW3[] = {
	{"PN[47:32]",  WF_UWTBL_PN_47_32__MASK,  WF_UWTBL_PN_47_32__SHIFT,  FALSE},
	{"COM_SN",     WF_UWTBL_COM_SN_MASK,     WF_UWTBL_COM_SN_SHIFT,	TRUE},
	{NULL,}
};

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_UMAC_DW4_BIPN[] = {
	{"BIPN[31:0]",	WF_UWTBL_RX_BIPN_31_0__MASK,	WF_UWTBL_RX_BIPN_31_0__SHIFT,	TRUE},
	{NULL,}
};

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_UMAC_DW5_BIPN[] = {
	{"BIPN[47:32]",	WF_UWTBL_RX_BIPN_47_32__MASK,	WF_UWTBL_RX_BIPN_47_32__SHIFT,	FALSE},
	{NULL,}
};

static VOID parse_fmac_uwtbl_sn(RTMP_ADAPTER *pAd, UINT8 *uwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 u2SN = 0;

	/* UMAC WTBL DW SN part */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("UWTBL SN\n");

	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_TID0_SN_DW*4]);
	u2SN = ((*addr) & WF_UWTBL_TID0_SN_MASK) >> WF_UWTBL_TID0_SN_SHIFT;
	MTWF_PRINT("\t%s:%u\n", "TID0_AC0_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_TID1_SN_DW*4]);
	u2SN = ((*addr) & WF_UWTBL_TID1_SN_MASK) >> WF_UWTBL_TID1_SN_SHIFT;
	MTWF_PRINT("\t%s:%u\n", "TID1_AC1_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_TID2_SN_7_0__DW*4]);
	u2SN = ((*addr) & WF_UWTBL_TID2_SN_7_0__MASK) >>
				WF_UWTBL_TID2_SN_7_0__SHIFT;
	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_TID2_SN_11_8__DW*4]);
	u2SN |= (((*addr) & WF_UWTBL_TID2_SN_11_8__MASK) >>
			WF_UWTBL_TID2_SN_11_8__SHIFT) << 8;
	MTWF_PRINT("\t%s:%u\n", "TID2_AC2_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_TID3_SN_DW*4]);
	u2SN = ((*addr) & WF_UWTBL_TID3_SN_MASK) >> WF_UWTBL_TID3_SN_SHIFT;
	MTWF_PRINT("\t%s:%u\n", "TID3_AC3_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_TID4_SN_DW*4]);
	u2SN = ((*addr) & WF_UWTBL_TID4_SN_MASK) >> WF_UWTBL_TID4_SN_SHIFT;
	MTWF_PRINT("\t%s:%u\n", "TID4_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_TID5_SN_3_0__DW*4]);
	u2SN = ((*addr) & WF_UWTBL_TID5_SN_3_0__MASK) >>
				WF_UWTBL_TID5_SN_3_0__SHIFT;
	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_TID5_SN_11_4__DW*4]);
	u2SN |= (((*addr) & WF_UWTBL_TID5_SN_11_4__MASK) >>
				WF_UWTBL_TID5_SN_11_4__SHIFT) << 4;
	MTWF_PRINT("\t%s:%u\n", "TID5_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_TID6_SN_DW*4]);
	u2SN = ((*addr) & WF_UWTBL_TID6_SN_MASK) >> WF_UWTBL_TID6_SN_SHIFT;
	MTWF_PRINT("\t%s:%u\n", "TID6_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_TID7_SN_DW*4]);
	u2SN = ((*addr) & WF_UWTBL_TID7_SN_MASK) >> WF_UWTBL_TID7_SN_SHIFT;
	MTWF_PRINT("\t%s:%u\n", "TID7_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_COM_SN_DW*4]);
	u2SN = ((*addr) & WF_UWTBL_COM_SN_MASK) >> WF_UWTBL_COM_SN_SHIFT;
	MTWF_PRINT("\t%s:%u\n", "COM_SN", u2SN);
}

static VOID parse_fmac_uwtbl_pn_sn(RTMP_ADAPTER *pAd, UINT8 *uwtbl, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	MTWF_PRINT("\t\n");
	MTWF_PRINT("UWTBL PN\n");

	/* UMAC WTBL DW 2/3 */
	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_PN_31_0__DW*4]);
	dw_value = *addr;

	while (WTBL_UMAC_DW2[i].name) {
		MTWF_PRINT("\t%s:%u\n", WTBL_UMAC_DW2[i].name,
			(dw_value & WTBL_UMAC_DW2[i].mask) >>
				WTBL_UMAC_DW2[i].shift);
		i++;
	}

	i = 0;
	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_PN_47_32__DW*4]);
	dw_value = *addr;

	while (WTBL_UMAC_DW3[i].name) {
		MTWF_PRINT("\t%s:%u\n", WTBL_UMAC_DW3[i].name,
			 (dw_value & WTBL_UMAC_DW3[i].mask) >>
			WTBL_UMAC_DW3[i].shift);
		i++;
	}


	/* UMAC WTBL DW 4/5 for BIGTK */
	if (is_wtbl_bigtk_exist(pAd, lwtbl) == TRUE) {
		i = 0;
		addr = (UINT_32 *)&(uwtbl[WF_UWTBL_RX_BIPN_31_0__DW*4]);
		dw_value = *addr;

		while (WTBL_UMAC_DW4_BIPN[i].name) {
			MTWF_PRINT("\t%s:%u\n", WTBL_UMAC_DW4_BIPN[i].name,
				(dw_value & WTBL_UMAC_DW4_BIPN[i].mask) >>
					WTBL_UMAC_DW4_BIPN[i].shift);
			i++;
		}

		i = 0;
		addr = (UINT_32 *)&(uwtbl[WF_UWTBL_RX_BIPN_47_32__DW*4]);
		dw_value = *addr;

		while (WTBL_UMAC_DW5_BIPN[i].name) {
			MTWF_PRINT("\t%s:%u\n", WTBL_UMAC_DW5_BIPN[i].name,
				(dw_value & WTBL_UMAC_DW5_BIPN[i].mask) >>
				WTBL_UMAC_DW5_BIPN[i].shift);
			i++;
		}
	} else
		parse_fmac_uwtbl_sn(pAd, uwtbl);
}

static VOID dump_key_table(
	RTMP_ADAPTER *pAd,
	UINT_16 keyloc0,
	UINT_16 keyloc1,
	UINT_16 keyloc2
)
{
	UINT8 keytbl[ONE_KEY_ENTRY_LEN_IN_DW*4] = {0};
	UINT_16 x;

	MTWF_PRINT("\t\n");
	MTWF_PRINT("\t%s:%d\n", "keyloc0", keyloc0);
	if (keyloc0 != INVALID_KEY_ENTRY) {

		/* Don't swap below two lines, halWtblReadRaw will
		* write new value WF_WTBLON_TOP_WDUCR_ADDR
		*/
		halWtblReadRaw(pAd, keyloc0,
			WTBL_TYPE_KEY, 0, ONE_KEY_ENTRY_LEN_IN_DW, keytbl);
		MTWF_PRINT("\t\tKEY WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
			WF_UWTBL_TOP_WDUCR_ADDR,
			IO_R_32(WF_UWTBL_TOP_WDUCR_ADDR),
			KEYTBL_IDX2BASE(keyloc0, 0));
		for (x = 0; x < ONE_KEY_ENTRY_LEN_IN_DW; x++) {
			MTWF_PRINT("\t\tDW%02d: %02x %02x %02x %02x\n",
				x,
				keytbl[x * 4 + 3],
				keytbl[x * 4 + 2],
				keytbl[x * 4 + 1],
				keytbl[x * 4]);
		}
	}

	MTWF_PRINT("\t%s:%d\n", "keyloc1", keyloc1);
	if (keyloc1 != INVALID_KEY_ENTRY) {
		/* Don't swap below two lines, halWtblReadRaw will
		* write new value WF_WTBLON_TOP_WDUCR_ADDR
		*/
		halWtblReadRaw(pAd, keyloc1,
			WTBL_TYPE_KEY, 0, ONE_KEY_ENTRY_LEN_IN_DW, keytbl);
		MTWF_PRINT("\t\tKEY WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
			WF_UWTBL_TOP_WDUCR_ADDR,
			IO_R_32(WF_UWTBL_TOP_WDUCR_ADDR),
			KEYTBL_IDX2BASE(keyloc1, 0));
		for (x = 0; x < ONE_KEY_ENTRY_LEN_IN_DW; x++) {
			MTWF_PRINT("\t\tDW%02d: %02x %02x %02x %02x\n",
				x,
				keytbl[x * 4 + 3],
				keytbl[x * 4 + 2],
				keytbl[x * 4 + 1],
				keytbl[x * 4]);
		}
	}

	MTWF_PRINT("\t%s:%d\n", "keyloc2", keyloc2);
	if (keyloc2 != INVALID_KEY_ENTRY) {
		/* Don't swap below two lines, halWtblReadRaw will
		* write new value WF_WTBLON_TOP_WDUCR_ADDR
		*/
		halWtblReadRaw(pAd, keyloc2,
			WTBL_TYPE_KEY, 0, ONE_KEY_ENTRY_LEN_IN_DW, keytbl);
		MTWF_PRINT("\t\tKEY WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
			WF_UWTBL_TOP_WDUCR_ADDR,
			IO_R_32(WF_UWTBL_TOP_WDUCR_ADDR),
			KEYTBL_IDX2BASE(keyloc2, 0));
		for (x = 0; x < ONE_KEY_ENTRY_LEN_IN_DW; x++) {
			MTWF_PRINT("\t\tDW%02d: %02x %02x %02x %02x\n",
				x,
				keytbl[x * 4 + 3],
				keytbl[x * 4 + 2],
				keytbl[x * 4 + 1],
				keytbl[x * 4]);
		}
	}
}

static VOID parse_fmac_uwtbl_key_info(RTMP_ADAPTER *pAd, uint8_t *uwtbl, uint8_t *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	uint16_t keyloc0 = INVALID_KEY_ENTRY;
	uint16_t keyloc1 = INVALID_KEY_ENTRY;
	uint16_t keyloc2 = INVALID_KEY_ENTRY;

	/* UMAC WTBL DW 7 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("UWTBL key info\n");

	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_KEY_LOC0_DW*4]);
	dw_value = *addr;
	keyloc0 = (dw_value & WF_UWTBL_KEY_LOC0_MASK) >> WF_UWTBL_KEY_LOC0_SHIFT;
	keyloc1 = (dw_value & WF_UWTBL_KEY_LOC1_MASK) >> WF_UWTBL_KEY_LOC1_SHIFT;

	MTWF_PRINT("\t%s:%u/%u\n", "Key Loc 0/1", keyloc0, keyloc1);

	/* UMAC WTBL DW 6 for BIGTK */
	if (is_wtbl_bigtk_exist(pAd, lwtbl) == TRUE) {
		addr = (UINT_32 *)&(uwtbl[WF_UWTBL_KEY_LOC2_DW*4]);
		dw_value = *addr;
		keyloc2 = (dw_value & WF_UWTBL_KEY_LOC2_MASK) >>
			WF_UWTBL_KEY_LOC2_SHIFT;
		MTWF_PRINT("\t%s:%u\n", "Key Loc 2", keyloc2);
	}

	/* Parse KEY link */
	dump_key_table(pAd, keyloc0, keyloc1, keyloc2);
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_UMAC_DW8[] = {
	{"UWTBL_WMM_Q",		WF_UWTBL_WMM_Q_MASK,		WF_UWTBL_WMM_Q_SHIFT,	FALSE},
	{"UWTBL_QOS",		WF_UWTBL_QOS_MASK,		NO_SHIFT_DEFINE,	FALSE},
	{"UWTBL_HT_VHT_HE",	WF_UWTBL_HT_MASK,		NO_SHIFT_DEFINE,	FALSE},
	{"UWTBL_HDRT_MODE",	WF_UWTBL_HDRT_MODE_MASK,	NO_SHIFT_DEFINE,	TRUE},
	{"UWTBL_AAD_OM",	WF_UWTBL_AAD_OM_MASK,	WF_UWTBL_AAD_OM_SHIFT,	TRUE},
	{NULL,}
};

static VOID parse_fmac_uwtbl_msdu_info(RTMP_ADAPTER *pAd, uint8_t *uwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_32 amsdu_len = 0;
	UINT_16 i = 0;

	/* UMAC WTBL DW 8 */
	MTWF_PRINT("\t\n");
	MTWF_PRINT("UWTBL DW8\n");

	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_AMSDU_CFG_DW*4]);
	dw_value = *addr;

	while (WTBL_UMAC_DW8[i].name) {

		if (WTBL_UMAC_DW8[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("\t%s:%d\n", WTBL_UMAC_DW8[i].name,
				(dw_value & WTBL_UMAC_DW8[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("\t%s:%u\n", WTBL_UMAC_DW8[i].name,
				(dw_value & WTBL_UMAC_DW8[i].mask) >>
					WTBL_UMAC_DW8[i].shift);
		i++;
	}

	/* UMAC WTBL DW 8 - SEC_ADDR_MODE */
	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_SEC_ADDR_MODE_DW*4]);
	dw_value = *addr;
	MTWF_PRINT("\t%s:%lu\n", "SEC_ADDR_MODE",
		(dw_value & WTBL_SEC_ADDR_MODE_MASK) >> WTBL_SEC_ADDR_MODE_OFFSET);

	/* UMAC WTBL DW 8 - AMSDU_CFG */
	MTWF_PRINT("\t%s:%d\n", "HW AMSDU Enable",
				(dw_value & WTBL_AMSDU_EN_MASK) ? 1 : 0);

	amsdu_len = (dw_value & WTBL_AMSDU_LEN_MASK) >> WTBL_AMSDU_LEN_OFFSET;
	if (amsdu_len == 0)
		MTWF_PRINT("\t%s:invalid (WTBL value=0x%x)\n", "HW AMSDU Len",
			amsdu_len);
	else if (amsdu_len == 1)
		MTWF_PRINT("\t%s:%d~%d (WTBL value=0x%x)\n", "HW AMSDU Len",
			1,
			255,
			amsdu_len);
	else if (amsdu_len == 2)
		MTWF_PRINT("\t%s:%d~%d (WTBL value=0x%x)\n", "HW AMSDU Len",
			256,
			511,
			amsdu_len);
	else if (amsdu_len == 3)
		MTWF_PRINT("\t%s:%d~%d (WTBL value=0x%x)\n", "HW AMSDU Len",
			512,
			767,
			amsdu_len);
	else
		MTWF_PRINT("\t%s:%d~%d (WTBL value=0x%x)\n", "HW AMSDU Len",
			256 * (amsdu_len - 1),
			256 * (amsdu_len - 1) + 255,
			amsdu_len);

	MTWF_PRINT("\t%s:%lu (WTBL value=0x%lx)\n", "HW AMSDU Num",
		((dw_value & WTBL_AMSDU_NUM_MASK) >> WTBL_AMSDU_NUM_OFFSET) + 1,
		(dw_value & WTBL_AMSDU_NUM_MASK) >> WTBL_AMSDU_NUM_OFFSET);
}

static VOID dump_fmac_wtbl_info(RTMP_ADAPTER *pAd, uint8_t *lwtbl, uint8_t *uwtbl)
{
	/* Parse LWTBL */
	parse_fmac_lwtbl_DW0_1(pAd, lwtbl);
	parse_fmac_lwtbl_DW2(pAd, lwtbl);
	parse_fmac_lwtbl_DW3(pAd, lwtbl);
	parse_fmac_lwtbl_DW4(pAd, lwtbl);
	parse_fmac_lwtbl_DW5(pAd, lwtbl);
	parse_fmac_lwtbl_DW6(pAd, lwtbl);
	parse_fmac_lwtbl_DW7(pAd, lwtbl);
	parse_fmac_lwtbl_DW8(pAd, lwtbl);
	parse_fmac_lwtbl_DW9(pAd, lwtbl);
	parse_fmac_lwtbl_DW10(pAd, lwtbl);
	parse_fmac_lwtbl_DW11(pAd, lwtbl);
	parse_fmac_lwtbl_DW12(pAd, lwtbl);
	parse_fmac_lwtbl_DW13(pAd, lwtbl);
	parse_fmac_lwtbl_DW14(pAd, lwtbl);
	parse_fmac_lwtbl_mlo_info(pAd, lwtbl);
	parse_fmac_lwtbl_DW31(pAd, lwtbl);
	parse_fmac_lwtbl_DW32(pAd, lwtbl);
	parse_fmac_lwtbl_rx_stats(pAd, lwtbl);

	/* Parse UWTBL */
	parse_fmac_uwtbl_mlo_info(pAd, uwtbl);
	parse_fmac_uwtbl_pn_sn(pAd, uwtbl, lwtbl);
	parse_fmac_uwtbl_key_info(pAd, uwtbl, lwtbl);
	parse_fmac_uwtbl_msdu_info(pAd, uwtbl);
}

static VOID chip_dump_wtbl_base_info(RTMP_ADAPTER *pAd)
{
	MTWF_PRINT("WTBL Basic Info:\n");
}

static VOID chip_dump_wtbl_info(RTMP_ADAPTER *pAd, uint16_t wtbl_idx)
{
	uint8_t lwtbl[LWTBL_LEN_IN_DW*4] = {0};
	uint8_t uwtbl[UWTBL_LEN_IN_DW*4] = {0};
	int x;
	uint8_t real_lwtbl_size = 0;

	real_lwtbl_size = LWTBL_LEN_IN_DW;

	/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
	halWtblReadRaw(pAd, wtbl_idx, WTBL_TYPE_LMAC, 0, real_lwtbl_size, lwtbl);
	MTWF_PRINT("Dump WTBL info of WLAN_IDX:%d\n", wtbl_idx);
	MTWF_PRINT("LMAC WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
								WF_WTBLON_TOP_WDUCR_ADDR,
								IO_R_32(WF_WTBLON_TOP_WDUCR_ADDR),
								LWTBL_IDX2BASE(wtbl_idx, 0));
	for (x = 0; x < real_lwtbl_size; x++) {
		MTWF_PRINT("DW%02d: %02x %02x %02x %02x\n",
									x,
									lwtbl[x * 4 + 3],
									lwtbl[x * 4 + 2],
									lwtbl[x * 4 + 1],
									lwtbl[x * 4]);
	}

	/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
	halWtblReadRaw(pAd, wtbl_idx, WTBL_TYPE_UMAC, 0, UWTBL_LEN_IN_DW, uwtbl);
	MTWF_PRINT("UMAC WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
								WF_UWTBL_TOP_WDUCR_ADDR,
								IO_R_32(WF_UWTBL_TOP_WDUCR_ADDR),
								UWTBL_IDX2BASE(wtbl_idx, 0));
	for (x = 0; x < UWTBL_LEN_IN_DW; x++) {
		MTWF_PRINT("DW%02d: %02x %02x %02x %02x\n",
									x,
									uwtbl[x * 4 + 3],
									uwtbl[x * 4 + 2],
									uwtbl[x * 4 + 1],
									uwtbl[x * 4]);
	}

	dump_fmac_wtbl_info(pAd, lwtbl, uwtbl);
}

static VOID chip_dump_wtbl_mlo_omac(RTMP_ADAPTER *pAd, uint16_t wtbl_idx)
{
	uint8_t uwtbl[UWTBL_LEN_IN_DW*4] = {0};
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT32 OMLD_ID = 0;
	UINT32 reg_val1 = 0;
	UINT32 reg_val2 = 0;
	uint8_t *reg_p1, *reg_p2;

	/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
	halWtblReadRaw(pAd, wtbl_idx, WTBL_TYPE_UMAC, 0, UWTBL_LEN_IN_DW, uwtbl);

	MTWF_PRINT("UWTBL DW 0\n");
	addr = (UINT_32 *)&(uwtbl[WF_UWTBL_OWN_MLD_ID_DW*4]);
	dw_value = *addr;

	OMLD_ID = (dw_value & WF_UWTBL_OWN_MLD_ID_MASK) >> WF_UWTBL_OWN_MLD_ID_SHIFT;

	MTWF_PRINT("\t%s:%u\n", "OMLD_ID",
		OMLD_ID);

	/* DUMP UWTBL.Own MLD Addr via OMLD_ID */
	reg_val1 = (WF_UWTBL_TOP_ITCR_EXECUTE_MASK | ((OMLD_ID & WF_UWTBL_TOP_ITCR_INDEX_MASK) << WF_UWTBL_TOP_ITCR_INDEX_SHFT));
	IO_W_32(WF_UWTBL_TOP_ITCR_ADDR, reg_val1);
	reg_val1 = IO_R_32(WF_UWTBL_TOP_ITDR0_ADDR);
	reg_val2 = IO_R_32(WF_UWTBL_TOP_ITDR1_ADDR);
	reg_p1 = (uint8_t *)&reg_val1;
	reg_p2 = (uint8_t *)&reg_val2;

	MTWF_PRINT("\t\n");
	MTWF_PRINT("MLD Own Addr: %02x:%02x:%02x:%02x:%02x:%02x(D0[B0~15], D1[B0~31])\n",
		reg_p1[0], reg_p1[1], reg_p1[2], reg_p1[3], reg_p2[0], reg_p2[1]);

}

static VOID chip_dump_wtbl_mac(RTMP_ADAPTER *pAd, uint16_t wtbl_idx)
{
	uint8_t lwtbl[LWTBL_LEN_IN_DW*4] = {0};
	uint8_t real_lwtbl_size = 0;

	real_lwtbl_size = LWTBL_LEN_IN_DW;

	/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
	halWtblReadRaw(pAd, wtbl_idx, WTBL_TYPE_LMAC, 0, real_lwtbl_size, lwtbl);

	MTWF_PRINT("WLAN_IDX: %d Mac Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
			  wtbl_idx, lwtbl[4], lwtbl[5], lwtbl[6], lwtbl[7], lwtbl[0], lwtbl[1]);
}

static INT32 chip_set_ba_winsize(RTMP_ADAPTER *pAd, UINT16 wtbl_idx, UCHAR tid, UINT32 win_size)
{
	UINT32 wtblDW7 = 0, curWinSize = 0;

	halWtblReadRaw(pAd, wtbl_idx, WTBL_TYPE_LMAC, 7, 1, &wtblDW7);
	/*
	* wtblDW7 -- 32bit
	* bit[31-28] [27-24] [23-20] [19-16] [15-12] [11-8] [7-4] [3-0]
	*     TID7    TID6    TID5    TID4    TID3    TID2   TID1  TID0
	*/
	curWinSize = (wtblDW7 >> (tid * 4)) & 0x0F;

	if (win_size < 0xF && win_size != curWinSize) {
		wtblDW7 &= ~(0x0F << (tid * 4));
		wtblDW7 |= win_size << (tid * 4);
		halWtblWriteRaw(pAd, wtbl_idx, WTBL_TYPE_LMAC, 7, wtblDW7);
	}

	return TRUE;
}

static INT32 chip_clear_keyloc(RTMP_ADAPTER *pAd, UINT16 wtbl_idx, UINT32 key_loc)
{
	UINT32 wtblDW = 0;

	if (key_loc == 0) {
		halWtblReadRaw(pAd, wtbl_idx, WTBL_TYPE_UMAC, WF_UWTBL_KEY_LOC0_DW, 1, &wtblDW);
		wtblDW =  ((wtblDW & ~WF_UWTBL_KEY_LOC0_MASK)  | (WF_UWTBL_KEY_LOC0_MASK << WF_UWTBL_KEY_LOC0_SHIFT));
		halWtblWriteRaw(pAd, wtbl_idx, WTBL_TYPE_UMAC, WF_UWTBL_KEY_LOC0_DW, wtblDW);
	} else if (key_loc == 1) {
		halWtblReadRaw(pAd, wtbl_idx, WTBL_TYPE_UMAC, WF_UWTBL_KEY_LOC1_DW, 1, &wtblDW);
		wtblDW =  ((wtblDW & ~WF_UWTBL_KEY_LOC1_MASK)  | (WF_UWTBL_KEY_LOC1_MASK << WF_UWTBL_KEY_LOC1_SHIFT));
		halWtblWriteRaw(pAd, wtbl_idx, WTBL_TYPE_UMAC, WF_UWTBL_KEY_LOC1_DW, wtblDW);
	} else if (key_loc == 2) {
		halWtblReadRaw(pAd, wtbl_idx, WTBL_TYPE_UMAC, WF_UWTBL_KEY_LOC2_DW, 1, &wtblDW);
		wtblDW =  ((wtblDW & ~WF_UWTBL_KEY_LOC2_MASK)  | (WF_UWTBL_KEY_LOC2_MASK << WF_UWTBL_KEY_LOC2_SHIFT));
		halWtblWriteRaw(pAd, wtbl_idx, WTBL_TYPE_UMAC, WF_UWTBL_KEY_LOC2_DW, wtblDW);
	}

	return TRUE;
}

static VOID chip_set_hw_amsdu(RTMP_ADAPTER *pAd, UINT32 wcid, uint8_t num, UINT32 len)
{
	UINT32 hw_amsdu_cfg = 0;

	halWtblReadRaw(pAd, wcid, WTBL_TYPE_UMAC, UWTBL_HW_AMSDU_DW, 1, &hw_amsdu_cfg);

	if (len) {
		hw_amsdu_cfg &= ~WTBL_AMSDU_LEN_MASK;
		hw_amsdu_cfg |= (len << WTBL_AMSDU_LEN_OFFSET);
	}

	hw_amsdu_cfg &= ~WTBL_AMSDU_NUM_MASK;
	hw_amsdu_cfg |= (num << WTBL_AMSDU_NUM_OFFSET);

	halWtblWriteRaw(pAd, wcid, WTBL_TYPE_UMAC, UWTBL_HW_AMSDU_DW, hw_amsdu_cfg);
}

static VOID chip_set_header_translation(RTMP_ADAPTER *pAd, UINT32 wcid, uint8_t on)
{
	UINT32 dw = 0;

	halWtblReadRaw(pAd, wcid, WTBL_TYPE_LMAC, 4, 1, &dw);

	if (on) {
		dw &= ~WTBL_DIS_RHTR;
	} else {
		dw |= WTBL_DIS_RHTR;
	}

	halWtblWriteRaw(pAd, wcid, WTBL_TYPE_LMAC, 4, dw);
}

static VOID dump_dma_tx_ring_info(struct hdev_ctrl *ctrl, char *s1,
				  char *s2, UINT32 ring_base)
{
	UINT32 base = 0, cnt = 0, cidx = 0, didx = 0, queue_cnt = 0;

	/* use RTMP_IO because addr need to be lookup */
	RTMP_IO_READ32(ctrl, ring_base, &base);
	RTMP_IO_READ32(ctrl, ring_base + 4, &cnt);
	RTMP_IO_READ32(ctrl, ring_base + 12, &didx);
	RTMP_IO_READ32(ctrl, ring_base + 8, &cidx);

	if (base)
		queue_cnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + cnt);

	MTWF_PRINT("%20s %6s %10x %15x %10x %10x %10x\n",
		s1, s2, base, cnt, cidx, didx, queue_cnt);

}

static VOID dump_dma_rx_ring_info(struct hdev_ctrl *ctrl, char *s1,
				  char *s2, UINT32 ring_base)
{
	UINT32 base = 0, ctrl1 = 0, cnt = 0, cidx = 0, didx = 0, queue_cnt = 0;

	/* use RTMP_IO because addr need to be lookup */
	RTMP_IO_READ32(ctrl, ring_base, &base);
	RTMP_IO_READ32(ctrl, ring_base + 4, &ctrl1);
	RTMP_IO_READ32(ctrl, ring_base + 8, &cidx);
	RTMP_IO_READ32(ctrl, ring_base + 12, &didx);
	cnt = ctrl1 & 0xFFF;
	cidx &= 0xFFF;
	didx &= 0xFFF;

	if (base)
		queue_cnt = (didx > cidx) ? (didx - cidx - 1) : (didx - cidx + cnt - 1);

	MTWF_PRINT("%20s %6s %10x %10x(%3x) %10x %10x %10x\n",
		s1, s2, base, ctrl1, cnt, cidx, didx, queue_cnt);
}

static VOID chip_show_dma_info(struct hdev_ctrl *ctrl)
{
	struct _RTMP_ADAPTER *pAd = hc_get_hdev_privdata(ctrl);
	struct _RTMP_CHIP_CAP *chip_cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);

	if (IS_RBUS_INF(pAd) || IS_PCI_INF(pAd) || IS_HWIFI_INF(pAd)) {
		UINT32 sys_ctrl[10] = {0};

		/* HOST DMA information */
		RTMP_IO_READ32(ctrl,
			WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR, &sys_ctrl[0]);
		RTMP_IO_READ32(ctrl,
			WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR, &sys_ctrl[1]);
		RTMP_IO_READ32(ctrl,
			WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, &sys_ctrl[2]);

		MTWF_PRINT("HOST_DMA0 Configuration\n");
		MTWF_PRINT("%10s %10s %10s %10s %10s %10s\n",
			"DMA", "IntCSR", "IntMask", "Glocfg", "Tx/RxEn", "Tx/RxBusy");
		MTWF_PRINT("%10s %10x %10x %10x %4x/%5x %4x/%5x\n",
			"DMA0", sys_ctrl[0], sys_ctrl[1], sys_ctrl[2],
			(sys_ctrl[2] & WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_MASK)
				>> WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_SHFT,
			(sys_ctrl[2] & WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK)
				>> WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_SHFT,
			(sys_ctrl[2] & WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK)
				>> WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_BUSY_SHFT,
			(sys_ctrl[2] & WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK)
				>> WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_BUSY_SHFT);

		RTMP_IO_READ32(ctrl,
			WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_ADDR, &sys_ctrl[0]);
		RTMP_IO_READ32(ctrl,
			WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_ENA_EXT_ADDR, &sys_ctrl[1]);
		RTMP_IO_READ32(ctrl,
			WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_ADDR, &sys_ctrl[2]);

		MTWF_PRINT("%10s %10x %10x %10x %4x/%5x %4x/%5x\n",
			"DMA0P1", sys_ctrl[0], sys_ctrl[1], sys_ctrl[2],
			(sys_ctrl[2] & WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_MASK)
				>> WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_SHFT,
			(sys_ctrl[2] & WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_MASK)
				>> WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_SHFT,
			(sys_ctrl[2] & WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK)
				>> WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_TX_DMA_BUSY_SHFT,
			(sys_ctrl[2] & WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK)
				>> WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_RX_DMA_BUSY_SHFT);

		MTWF_PRINT("HOST_DMA0 Ring Configuration\n");
		MTWF_PRINT("%20s %6s %10s %15s %10s %10s %10s\n",
			"Name", "Used", "Base", "Ctrl1(Cnt)", "CIDX", "DIDX", "QCnt");
		dump_dma_tx_ring_info(ctrl, "T16:FWDL", "Both",
			WF_WFDMA_HOST_DMA0_WPDMA_TX_RING16_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T17:Cmd(H2WM)", "Both",
			WF_WFDMA_HOST_DMA0_WPDMA_TX_RING17_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T18:TXD0(H2WA)", "AP",
			WF_WFDMA_HOST_DMA0_WPDMA_TX_RING18_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T19:TXD1(H2WA)", "AP",
			WF_WFDMA_HOST_DMA0_WPDMA_TX_RING19_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T20:Cmd(H2WA)", "AP",
			WF_WFDMA_HOST_DMA0_WPDMA_TX_RING20_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T21:TXD2(H2WA)", "AP",
			WF_WFDMA_HOST_DMA0_WPDMA_TX_RING21_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T22:TXD?(H2WA)", "AP",
			WF_WFDMA_HOST_DMA0_WPDMA_TX_RING22_CTRL0_ADDR);

		dump_dma_rx_ring_info(ctrl, "R0:Event(WM2H)", "Both",
			WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R1:Event(WA2H)", "AP",
			WF_WFDMA_HOST_DMA0_WPDMA_RX_RING1_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R2:TxDone0(WA2H)", "AP",
			WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R3:TxDone1(WA2H)", "AP",
			WF_WFDMA_HOST_DMA0_WPDMA_RX_RING3_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R4:Data0(MAC2H)", "Both",
			WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R5:Data1(MAC2H)", "Both",
			WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_CTRL0_ADDR);
		if (chip_cap->asic_caps & fASIC_CAP_HW_RRO) {
			dump_dma_rx_ring_info(ctrl, "R6:TxDone0(MAC2H)", "Both",
				WF_WFDMA_HOST_DMA0_WPDMA_RX_RING6_CTRL0_ADDR);
			dump_dma_rx_ring_info(ctrl, "R7:TxDone1(MAC2H)", "Both",
				WF_WFDMA_HOST_DMA0_WPDMA_RX_RING7_CTRL0_ADDR);
			dump_dma_rx_ring_info(ctrl, "R8:BUF0(MAC2H)", "Both",
				WF_WFDMA_HOST_DMA0_WPDMA_RX_RING8_CTRL0_ADDR);
			dump_dma_rx_ring_info(ctrl, "R9:BUF1(MAC2H)", "Both",
				WF_WFDMA_HOST_DMA0_WPDMA_RX_RING9_CTRL0_ADDR);
			dump_dma_rx_ring_info(ctrl, "R10:MSDU_PG(MAC2H)", "Both",
				WF_WFDMA_HOST_DMA0_WPDMA_RX_RING10_CTRL0_ADDR);
			dump_dma_rx_ring_info(ctrl, "RRO:IND_CMD(MAC2H)", "Both",
				WF_RRO_TOP_IND_CMD_0_CTRL0_ADDR);
			dump_dma_rx_ring_info(ctrl, "RRO:Data0(MAC2H)", "Both",
				WF_RRO_TOP_RX_RING_AP_0_CTRL0_ADDR);
			dump_dma_rx_ring_info(ctrl, "RRO:Data1(MAC2H)", "Both",
				WF_RRO_TOP_RX_RING_AP_1_CTRL0_ADDR);
			dump_dma_rx_ring_info(ctrl, "RRO:Data2(MAC2H)", "Both",
				WF_RRO_TOP_RX_RING_AP_2_CTRL0_ADDR);
			dump_dma_rx_ring_info(ctrl, "RRO:Data3(MAC2H)", "Both",
				WF_RRO_TOP_RX_RING_AP_3_CTRL0_ADDR);
		}

		MTWF_PRINT("HOST_DMA0 PCIe1 Ring Configuration\n");
		MTWF_PRINT("%20s %6s %10s %15s %10s %10s %10s\n",
			"Name", "Used", "Base", "Cnt", "CIDX", "DIDX", "QCnt");
		dump_dma_tx_ring_info(ctrl, "T21:TXD1(H2WA)", "AP",
			WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING21_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T22:TXD?(H2WA)", "AP",
			WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING22_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R3:TxDone1(WA2H)", "AP",
			WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING3_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R5:Data1(MAC2H)", "Both",
			WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING5_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R7:TxDone1(MAC2H)", "Both",
			WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING7_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R9:BUF1(MAC2H)", "Both",
			WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING9_CTRL0_ADDR);

		/* MCU DMA information */
		RTMP_IO_READ32(ctrl,
			WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_ADDR, &sys_ctrl[0]);
		RTMP_IO_READ32(ctrl,
			WF_WFDMA_MCU_DMA0_HOST_INT_STA_ADDR, &sys_ctrl[1]);
		RTMP_IO_READ32(ctrl,
			WF_WFDMA_MCU_DMA0_HOST_INT_ENA_ADDR, &sys_ctrl[2]);

		MTWF_PRINT("MCU_DMA Configuration\n");
		MTWF_PRINT("%10s %10s %10s %10s %10s %10s\n",
			"DMA", "IntCSR", "IntMask", "Glocfg", "Tx/RxEn", "Tx/RxBusy");
		MTWF_PRINT("%10s %10x %10x %10x %4x/%5x %4x/%5x\n",
			"DMA0", sys_ctrl[1], sys_ctrl[2], sys_ctrl[0],
			(sys_ctrl[0] & WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_MASK)
				>> WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_SHFT,
			(sys_ctrl[0] & WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK)
				>> WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_SHFT,
			(sys_ctrl[0] & WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK)
				>> WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_TX_DMA_BUSY_SHFT,
			(sys_ctrl[0] & WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK)
				>> WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_RX_DMA_BUSY_SHFT);

		MTWF_PRINT("MCU_DMA0 Ring Configuration\n");
		MTWF_PRINT("%20s %6s %10s %15s %10s %10s %10s\n",
			"Name", "Used", "Base", "Cnt", "CIDX", "DIDX", "QCnt");
		dump_dma_tx_ring_info(ctrl, "T0:Event(WM2H)", "Both",
			WF_WFDMA_MCU_DMA0_WPDMA_TX_RING0_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T1:Event(WA2H)", "AP",
			WF_WFDMA_MCU_DMA0_WPDMA_TX_RING1_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T2:TxDone0(WA2H)", "AP",
			WF_WFDMA_MCU_DMA0_WPDMA_TX_RING2_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T3:TxDone1(WA2H)", "AP",
			WF_WFDMA_MCU_DMA0_WPDMA_TX_RING3_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T4:TXD(WM2MAC)", "Both",
			WF_WFDMA_MCU_DMA0_WPDMA_TX_RING4_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T5:TXCMD(WM2MAC)", "Both",
			WF_WFDMA_MCU_DMA0_WPDMA_TX_RING5_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T6:TXD(WA2MAC)", "AP",
			WF_WFDMA_MCU_DMA0_WPDMA_TX_RING6_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R0:FWDL", "Both",
			WF_WFDMA_MCU_DMA0_WPDMA_RX_RING0_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R1:Cmd(H2WM)", "Both",
			WF_WFDMA_MCU_DMA0_WPDMA_RX_RING1_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R2:TXD0(H2WA)", "AP",
			WF_WFDMA_MCU_DMA0_WPDMA_RX_RING2_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R3:TXD1(H2WA)", "AP",
			WF_WFDMA_MCU_DMA0_WPDMA_RX_RING3_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R4:Cmd(H2WA)", "AP",
			WF_WFDMA_MCU_DMA0_WPDMA_RX_RING4_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R5:Data0(MAC2WM)", "Both",
			WF_WFDMA_MCU_DMA0_WPDMA_RX_RING5_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R6:TxDone(MAC2WM)", "Both",
			WF_WFDMA_MCU_DMA0_WPDMA_RX_RING6_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R7:SPL/RPT(MAC2WM)", "Both",
			WF_WFDMA_MCU_DMA0_WPDMA_RX_RING7_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R8:TxDone(MAC2WA)", "AP",
			WF_WFDMA_MCU_DMA0_WPDMA_RX_RING8_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R9:Data1(MAC2WM)", "Both",
			WF_WFDMA_MCU_DMA0_WPDMA_RX_RING9_CTRL0_ADDR);

		/* MEM DMA information */
		RTMP_IO_READ32(ctrl,
			WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_ADDR, &sys_ctrl[0]);
		RTMP_IO_READ32(ctrl,
			WF_WFDMA_MEM_DMA_HOST_INT_STA_ADDR, &sys_ctrl[1]);
		RTMP_IO_READ32(ctrl,
			WF_WFDMA_MEM_DMA_HOST_INT_ENA_ADDR, &sys_ctrl[2]);

		MTWF_PRINT("MEM_DMA Configuration\n");
		MTWF_PRINT("%10s %10s %10s %10s %10s %10s\n",
			"DMA", "IntCSR", "IntMask", "Glocfg", "Tx/RxEn", "Tx/RxBusy");
		MTWF_PRINT("%10s %10x %10x %10x %4x/%5x %4x/%5x\n",
			"MEM", sys_ctrl[1], sys_ctrl[2], sys_ctrl[0],
			(sys_ctrl[0] & WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_TX_DMA_EN_MASK)
				>> WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_TX_DMA_EN_SHFT,
			(sys_ctrl[0] & WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_RX_DMA_EN_MASK)
				>> WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_RX_DMA_EN_SHFT,
			(sys_ctrl[0] & WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK)
				>> WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_TX_DMA_BUSY_SHFT,
			(sys_ctrl[0] & WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK)
				>> WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_RX_DMA_BUSY_SHFT);

		MTWF_PRINT("MEM_DMA Ring Configuration\n");
		MTWF_PRINT("%20s %6s %10s %15s %10s %10s %10s\n",
			"Name", "Used", "Base", "Cnt", "CIDX", "DIDX", "QCnt");
		dump_dma_tx_ring_info(ctrl, "T0:CmdEvent(WM2WA)", "AP",
			WF_WFDMA_MEM_DMA_WPDMA_TX_RING0_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T1:CmdEvent(WA2WM)", "AP",
			WF_WFDMA_MEM_DMA_WPDMA_TX_RING1_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R0:CmdEvent(WM2WA)", "AP",
			WF_WFDMA_MEM_DMA_WPDMA_RX_RING0_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R1:CmdEvent(WA2WM)", "AP",
			WF_WFDMA_MEM_DMA_WPDMA_RX_RING1_CTRL0_ADDR);

	}
}

UINT32 chip_get_lpon_frcr(RTMP_ADAPTER *pAd)
{
	UINT32 free_cnt = 0;

	HW_IO_READ32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_FRCR_ADDR, &free_cnt);
	return free_cnt;
}

#ifdef VOW_SUPPORT
UINT32 chip_get_sta_airtime(RTMP_ADAPTER *pAd, uint16_t sta, uint16_t ac, uint8_t tx)
{
	UINT32 airtime = 0;
	UINT32 wtbl_offset = 20;

	if (tx)
		halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, wtbl_offset + (ac << 1), 1, &airtime);
	else
		halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, wtbl_offset + (ac << 1) + 1, 1, &airtime);

	return airtime;
}

UINT32 chip_get_sta_addr(RTMP_ADAPTER *pAd, UINT32 sta)
{
	UINT32 addr = 0;

	halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, 0, 1, &addr);
	return addr;
}

UINT32 chip_get_sta_rate(RTMP_ADAPTER *pAd, UINT32 sta)
{
	UINT32 rate = 0;

	halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, 10, 1, &rate);
	return rate;
}

UINT32 chip_get_sta_tx_cnt(RTMP_ADAPTER *pAd, UINT32 sta, UINT32 bw)
{
	UINT32 tx_cnt = 0;
	UINT32 wtbl_offset = 16;

	halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, wtbl_offset + bw, 1, &tx_cnt);
	return tx_cnt;
}

UINT32 chip_get_sta_rx_rcpi(RTMP_ADAPTER *pAd, UINT32 sta)
{
	UINT32 rcpi = 0;
	UINT32 wtbl_offset = 34;

	halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, wtbl_offset, 1, &rcpi);
	return rcpi;
}

INT32 chip_set_sta_psm(RTMP_ADAPTER *pAd, UINT32 sta, UINT32 psm)
{
	UINT32 reg_val = 0;
	UINT32 mask = WTBL_PSM;
	UINT32 field = 0;
	UINT32 cnt = 0;
	uint8_t dw = WTBL_GROUP_TRX_CAP_DW_5;

	if (psm)
		field = mask;

	IO_W_32(WF_WTBLON_TOP_WMUMR_ADDR, mask);
	IO_W_32(WF_WTBLON_TOP_WMUDR_ADDR, field);
	/* Set WTBL ID */
	reg_val = (sta << WF_WTBLON_TOP_WIUCR_WLAN_IDX_SHFT) & WF_WTBLON_TOP_WIUCR_WLAN_IDX_MASK;
	/* Set DW */
	reg_val |= (dw << WF_WTBLON_TOP_WIUCR_DW_SHFT) & WF_WTBLON_TOP_WIUCR_DW_MASK;
	/* Trigger it */
	reg_val |= WF_WTBLON_TOP_WIUCR_MASK_UPDATE_MASK;
	IO_W_32(WF_WTBLON_TOP_WIUCR_ADDR, reg_val);

	/* Wait access complete */
	do {
		reg_val = IO_R_32(WF_WTBLON_TOP_WIUCR_ADDR);
		cnt++;

		if (cnt > WTBL_MASK_UPDATE_MAX_RETRY)
			break;

		RtmpusecDelay(1);
	} while (reg_val & WF_WTBLON_TOP_WIUCR_IU_BUSY_MASK);

	if (cnt > WTBL_MASK_UPDATE_MAX_RETRY)
		return FALSE;

	return TRUE;
}

static VOID chip_get_obss_nonwifi_airtime(RTMP_ADAPTER *pAd, UINT32 *at_info)
{
	HW_IO_READ32(pAd->hdev_ctrl, BN0_WF_RMAC_TOP_AIRTIME13_RX0_AIRTIME_NONWIFI_ADDR, &at_info[0]);
	HW_IO_READ32(pAd->hdev_ctrl, BN0_WF_RMAC_TOP_AIRTIME13_RX0_AIRTIME_NONWIFI_ADDR + (1 << 16), &at_info[1]);
	HW_IO_READ32(pAd->hdev_ctrl, BN0_WF_RMAC_TOP_AIRTIME14_RX0_AIRTIME_OBSS_ADDR, &at_info[2]);
	HW_IO_READ32(pAd->hdev_ctrl, BN0_WF_RMAC_TOP_AIRTIME14_RX0_AIRTIME_OBSS_ADDR + (1 << 16), &at_info[3]);
}
#endif	/* VOW_SUPPORT */

static INT32 chip_check_txv(
	IN struct hdev_ctrl *ctrl, IN UCHAR *name, IN UINT32 value, UINT8 band_idx)
{
	return 0;
}


#ifdef CONFIG_ATE
static INT32 chip_ctrl_manual_hetb_tx(
	struct _RTMP_ADAPTER *ad,
	uint8_t band_idx,
	uint8_t ctrl,
	uint8_t bw,
	uint8_t ltf_gi,
	uint8_t stbc,
	struct _ATE_RU_STA *ru_sta)
{
	UINT32 cr_value = 0;
	union hetb_rx_cmm cmm;
	union hetb_tx_usr usr;
	UINT32 nss;

	if (ctrl == HETB_TX_CFG) {
		if (ru_sta == NULL) {
			MTWF_PRINT("%s[%d]: invalid input\n", __func__, __LINE__);

			goto err_out;
		}
		/* setup MAC start */
		/* step 1, common info of TF */
		os_zero_mem(&cmm, sizeof(cmm));
		cmm.field.sig_a_reserved = 0x1ff;
		cmm.field.ul_length = ru_sta->l_len;
		cmm.field.t_pe = (ru_sta->afactor_init & 0x3) | ((ru_sta->pe_disamb & 0x1) << 2);
		cmm.field.ldpc_extra_sym = ru_sta->ldpc_extr_sym;
		nss = (ru_sta->ru_mu_nss > ru_sta->nss) ? ru_sta->ru_mu_nss : ru_sta->nss;
		if (ru_sta->ru_mu_nss > ru_sta->nss)
			cmm.field.mimo_ltf = 1;
		if (stbc && nss == 1)
			cmm.field.ltf_sym_midiam = ltf_sym_code[nss+1];
		else
			cmm.field.ltf_sym_midiam = ltf_sym_code[nss];
		cmm.field.gi_ltf = ltf_gi;
		cmm.field.ul_bw = bw;
		cmm.field.stbc = stbc;
		cr_value = (cmm.cmm_info & 0xffffffff);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR0_TF_COMINFO_B31B0_ADDR+(0x10000*band_idx), cr_value);
		cr_value = ((cmm.cmm_info & 0xffffffff00000000) >> 32);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR1_TF_COMINFO_B63B32_ADDR+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s: Step1: [CMM][%x][0x%llx]\n", __func__, BN0_WF_TMAC_TOP_TTRCR0_TF_COMINFO_B31B0_ADDR+(0x10000*band_idx), cmm.cmm_info);
		/* step 1, users info */
		usr.field.aid = 0x1;
		usr.field.allocation = ru_sta->ru_index;
		usr.field.coding = ru_sta->ldpc;
		usr.field.mcs = ru_sta->rate & ~BIT5;
		usr.field.dcm = (ru_sta->rate & BIT5) >> 4;
		usr.field.ss_allocation = ((nss-1) << 3) | (ru_sta->start_sp_st & 0x7);
		cr_value = usr.usr_info;
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR2_TF_USRINFO_B31B0_ADDR+(0x10000*band_idx), cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR3_TF_USRINFO_B39B32_ADDR+(0x10000*band_idx), 0xef);
		MTWF_PRINT("%s: Step1: [USR][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR2_TF_USRINFO_B31B0_ADDR+(0x10000*band_idx), usr.usr_info);
		MTWF_PRINT("%s:        [USR][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR3_TF_USRINFO_B39B32_ADDR+(0x10000*band_idx), 0xef);
		/*  step 2, rssi report*/
		cr_value = 0xffffffff;
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR4_TF_RX_RSSI_20M_B26B0_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR4_TF_RX_RSSI_20M_B26B0_MASK);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR5_TF_RX_RSSI_20M_B53B27_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR5_TF_RX_RSSI_20M_B53B27_MASK);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR6_TF_RX_RSSI_20M_B71B54_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR6_TF_RX_RSSI_20M_B71B54_MASK);
		MTWF_PRINT("%s: Step2: [RSSI][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR4_TF_RX_RSSI_20M_B26B0_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR4_TF_RX_RSSI_20M_B26B0_MASK);
		MTWF_PRINT("%s:        [RSSI][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR5_TF_RX_RSSI_20M_B53B27_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR5_TF_RX_RSSI_20M_B53B27_MASK);
		MTWF_PRINT("%s:        [RSSI][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR6_TF_RX_RSSI_20M_B71B54_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR6_TF_RX_RSSI_20M_B71B54_MASK);
		cr_value = 0xffffffff;
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR6_TF_RX_BWD_20M_ADDR+(0x10000*band_idx), cr_value | BN0_WF_TMAC_TOP_TTRCR6_TF_RX_BWD_20M_MASK);
		MTWF_PRINT("%s:        [BWD][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR6_TF_RX_BWD_20M_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR6_TF_RX_BWD_20M_MASK);
		/* step 3, channel information */
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_ADDR+(0x10000*band_idx), &cr_value);
		cr_value &= ~BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_MASK;
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_ADDR+(0x10000*band_idx), cr_value & ~BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_MASK);
		MTWF_PRINT("%s: Step3: [CBW Mode][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_ADDR+(0x10000*band_idx), (cr_value & BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_MASK));
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_CBW_160NC_IND_ADDR+(0x10000*band_idx), &cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_CBW_160NC_IND_ADDR+(0x10000*band_idx), (cr_value & ~BN0_WF_TMAC_TOP_TFCR0_CBW_160NC_IND_MASK));
		MTWF_PRINT("%s:        [CBW 160NC IND][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TFCR0_CBW_160NC_IND_ADDR+(0x10000*band_idx), (cr_value & BN0_WF_TMAC_TOP_TFCR0_CBW_160NC_IND_MASK));
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_ADDR+(0x10000*band_idx), &cr_value);
		cr_value |= ((TESTMODE_GET_PARAM(ad, band_idx, pri_sel) << BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_SHFT) & BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_MASK);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_ADDR+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s:        [CBW PRIM20 CH][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_ADDR+(0x10000*band_idx), (cr_value & BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_MASK));
		/* setup MAC end */
	} else if (ctrl == HETB_TX_START) {
		/*  step 6. Set 1 to TTRCR3.TF_RESP_TEST_MODE*/
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), &cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), cr_value | BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_MASK);
		MTWF_PRINT("%s: Step6: [TF_RESP_TEST_MODE][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), cr_value | BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_MASK);
	} else {
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), &cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), cr_value & ~(BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_MASK | BN0_WF_TMAC_TOP_TTRCR3_TF_USRINFO_B39B32_MASK));
		MTWF_PRINT("%s: [Proactive HETB TX turned off][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), cr_value & ~(BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_MASK | BN0_WF_TMAC_TOP_TTRCR3_TF_USRINFO_B39B32_MASK));
	}

err_out:
	return 0;
}


static INT32 chip_ctrl_manual_hetb_rx(
	struct _RTMP_ADAPTER *ad,
	uint8_t band_idx,
	uint8_t start,
	uint8_t bw,
	uint8_t gi_ltf,
	uint8_t stbc,
	ULONGLONG csd,
	struct _ATE_RU_STA *pri_sta,
	struct _ATE_RU_STA *sta_list)
{
	ULONGLONG mac_cr_value = 0;
	UINT32 cr_value = 0, usr_grp_idx = 0, sta_idx = 0, nss;
	union hetb_rx_cmm cmm;
	union hetb_rx_usr usr;
	UINT32 *phy_rx_ctrl = NULL;

	phy_rx_ctrl = tbrx_phy_ctrl[1];

	if (start) {
		/* setup MAC start */
		cr_value = 0;
		for (sta_idx = 0 ; sta_idx < 16 ; sta_idx++) {
			ULONGLONG ru_mac_coding = 0;
			UINT32 ru_idx = 0;

			if (sta_list[sta_idx].valid) {
				ru_idx = (sta_list[sta_idx].ru_index >> 1);

				if (ru_idx < 69) {
					if (ru_idx < 37)
						ru_mac_coding = 0x0;
					else if (ru_idx < 53)
						ru_mac_coding = 0x1;
					else if (ru_idx < 61)
						ru_mac_coding = 0x2;
					else if (ru_idx < 65)
						ru_mac_coding = 0x3;
					else if (ru_idx < 67)
						ru_mac_coding = 0x4;
					else if (ru_idx < 68)
						ru_mac_coding = 0x5;
					else
						ru_mac_coding = 0x6;
				} else
					ru_mac_coding = 0x7;
			} else
				ru_mac_coding = 0x0;

			mac_cr_value |= (ru_mac_coding << (sta_idx*3));
		}
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_RMAC_TOP_TF_USERTONE1_ADDR+(0x10000*band_idx), (((mac_cr_value & 0xffff00000000) >> 32) | BIT31));
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_RMAC_TOP_TF_USERTONE0_ADDR+(0x10000*band_idx), (mac_cr_value & 0xffffffff));
		MTWF_PRINT("%s: [MAC]0x%x=0x%llx\n", __func__, BN0_WF_RMAC_TOP_TF_USERTONE0_ADDR+(0x10000*band_idx), mac_cr_value);
		/* end MAC start */
		/* setup PHY start */
		/* cycle 0: start manual hetb rx (without TF) */
		RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), &cr_value);
		cr_value |= 0x1;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s: [Start]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		cr_value |= 0x2;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s:        0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		/* cycle 1:CSD part */
		cr_value = (csd & 0xffffffff);
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[2]+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s: [CSD_H]0x%x=0x%x\n", __func__, phy_rx_ctrl[2]+(0x10000*band_idx), cr_value);
		cr_value = (csd & 0xffffffff00000000) >> 32;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[1]+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s: [CSD_L]0x%x=0x%x\n", __func__, phy_rx_ctrl[1]+(0x10000*band_idx), cr_value);

		RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), &cr_value);
		cr_value |= 0x8;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s: [Assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		cr_value &= 0xfffffff3;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s: [De-assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		/* cycle 2:common info of TF */
		os_zero_mem(&cmm, sizeof(cmm));
		cmm.field.sig_a_reserved = 0x1ff;
		cmm.field.ul_length = pri_sta->l_len;
		cmm.field.t_pe = (pri_sta->afactor_init & 0x3) | ((pri_sta->pe_disamb & 0x1) << 2);
		cmm.field.ldpc_extra_sym = pri_sta->ldpc_extr_sym;
		nss = (pri_sta->ru_mu_nss > pri_sta->nss) ? pri_sta->ru_mu_nss : pri_sta->nss;
		cmm.field.ltf_sym_midiam = ltf_sym_code[nss];
		cmm.field.gi_ltf = gi_ltf;
		cmm.field.ul_bw = bw;
		cmm.field.stbc = stbc;
		cr_value = cmm.cmm_info & 0xffffffff;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[2]+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s: [CMM_H]0x%x=0x%x\n", __func__, phy_rx_ctrl[2]+(0x10000*band_idx), cr_value);
		cr_value = (cmm.cmm_info >> 32);
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[1]+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s: [CMM_L]0x%x=0x%x\n", __func__, phy_rx_ctrl[1]+(0x10000*band_idx), cr_value);

		RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), &cr_value);
		cr_value |= 0x8;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s: [Assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		cr_value &= 0xfffffff3;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s: [De-assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		/* cycle 3:users info */
		for (usr_grp_idx = 0 ; usr_grp_idx < 8 ; usr_grp_idx++) {
			os_zero_mem(&usr, sizeof(usr));

			if (sta_list[usr_grp_idx*2].valid) {
				usr.field.uid = usr_grp_idx*2;
				if (sta_list[usr_grp_idx*2].ru_mu_nss > sta_list[usr_grp_idx*2].nss)
					usr.field.nss = sta_list[usr_grp_idx*2].start_sp_st;	/* OFDMA:0, 1~7:MIMO; */
				else
					usr.field.nss = 0;	/* OFDMA:0, 1~7:MIMO; */
				usr.field.allocation = sta_list[usr_grp_idx*2].ru_index;
				usr.field.coding = sta_list[usr_grp_idx*2].ldpc;
				usr.field.mcs = sta_list[usr_grp_idx*2].rate & ~BIT5;
				usr.field.dcm = (sta_list[usr_grp_idx*2].rate & BIT5) >> 5;
				usr.field.ss_allocation = ((sta_list[usr_grp_idx*2].nss-1) << 3) | (sta_list[usr_grp_idx*2].start_sp_st & 0x7);
			} else
				usr.usr_info = 0xffffffff;
			RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[2]+(0x10000*band_idx), usr.usr_info);
			MTWF_PRINT("%s: [USR%d]0x%x=[0x%x]\n", __func__, usr_grp_idx*2, phy_rx_ctrl[2]+(0x10000*band_idx), usr.usr_info);

			os_zero_mem(&usr, sizeof(usr));
			if (sta_list[usr_grp_idx*2+1].valid) {
				usr.field.uid = usr_grp_idx*2+1;
				if (sta_list[usr_grp_idx*2+1].ru_mu_nss > sta_list[usr_grp_idx*2+1].nss)
					usr.field.nss = sta_list[usr_grp_idx*2+1].start_sp_st;	/* OFDMA:0, 1~7:MIMO; */
				else
					usr.field.nss = 0;	/* OFDMA:0, 1~7:MIMO; */
				usr.field.allocation = sta_list[usr_grp_idx*2+1].ru_index;
				usr.field.coding = sta_list[usr_grp_idx*2+1].ldpc;
				usr.field.mcs = sta_list[usr_grp_idx*2+1].rate & ~BIT5;
				usr.field.dcm = (sta_list[usr_grp_idx*2+1].rate & BIT5) >> 5;
				usr.field.ss_allocation = ((sta_list[usr_grp_idx*2+1].nss-1) << 3) | (sta_list[usr_grp_idx*2+1].start_sp_st & 0x7);
			} else
				usr.usr_info = 0xffffffff;
			RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[1]+(0x10000*band_idx), usr.usr_info);
			MTWF_PRINT("%s: [USR%d]0x%x=[0x%x]\n", __func__, usr_grp_idx*2+1, phy_rx_ctrl[1]+(0x10000*band_idx), usr.usr_info);

			RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), &cr_value);
			cr_value |= 0x8;
			RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
			MTWF_PRINT("%s: [Assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
			cr_value &= 0xfffffff3;
			RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
			MTWF_PRINT("%s: [De-assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		}

		RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), &cr_value);
		cr_value |= 0x4;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s: [Submit]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		/* setup PHY end */
	} else {
		RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), &cr_value);
		cr_value &= 0xfffffff0;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[1]+(0x10000*band_idx), 0);
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[2]+(0x10000*band_idx), 0);
		MTWF_PRINT("%s: [Stop]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s:       0x%x=0x%x\n", __func__, phy_rx_ctrl[1]+(0x10000*band_idx), 0);
		MTWF_PRINT("%s:       0x%x=0x%x\n", __func__, phy_rx_ctrl[2]+(0x10000*band_idx), 0);
	}

	return 0;
}

static INT32 chip_ctrl_asic_spe(RTMP_ADAPTER *ad,
											   uint8_t band_idx,
											   uint8_t tx_mode,
											   uint8_t spe_idx)
{
	UINT32 cr_value = 0;

	if (tx_mode == MODE_HE_TRIG) {
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_ADDR+(0x10000*band_idx), &cr_value);
		cr_value &= ~BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_MASK;
		cr_value |= ((spe_idx << BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_SHFT) & BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_MASK);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_ADDR+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s: Ste3.1:[SPE index][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_ADDR+(0x10000*band_idx), (cr_value & BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_MASK));
	} else {
		MTWF_PRINT("mt7992: %s, check correct cr settings\n",
			__func__);
	}

	return 0;
}
#endif

static UINT32 chip_show_asic_rx_stat(RTMP_ADAPTER *ad, UINT type)
{
	UINT32 value = 0;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"mt7992: Type(%d)\n", type);
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"Type(%d):%x\n", type, value);
	return value;
}

static INT32 chip_show_ple_info_by_idx(struct hdev_ctrl *ctrl, uint16_t wtbl_idx)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 *ple_stat = NULL;
	UINT32 *sta_pause = NULL;
	UINT32 *dis_sta_map = NULL;
	struct wifi_dev *wdev = NULL;
	INT32 i, j;
	UINT32 wmmidx = 0;

	os_alloc_mem(NULL, (UCHAR **)&ple_stat, ((DRR_MAX_DW_ALL_AC(pAd) + 1) * 4));
	if (!ple_stat)
		goto end;
	os_zero_mem(ple_stat, ((DRR_MAX_DW_ALL_AC(pAd) + 1) * 4));

	os_alloc_mem(NULL, (UCHAR **)&sta_pause, (DRR_MAX_DW_PER_AC(pAd) * 4));
	if (!sta_pause)
		goto end;
	os_zero_mem(sta_pause, (DRR_MAX_DW_PER_AC(pAd) * 4));

	os_alloc_mem(NULL, (UCHAR **)&dis_sta_map, (DRR_MAX_DW_PER_AC(pAd) * 4));
	if (!dis_sta_map)
		goto end;
	os_zero_mem(dis_sta_map, (DRR_MAX_DW_PER_AC(pAd) * 4));

	chip_get_ple_acq_stat(pAd, ple_stat);
	chip_get_dis_sta_map(pAd, dis_sta_map);
	chip_get_sta_pause(pAd, sta_pause);

	for (j = 0; j < DRR_MAX_DW_ALL_AC(pAd); j++) { /* show AC Q info */
		for (i = 0; i < 32; i++) {
			if (((ple_stat[j + 1] & (0x1 << i)) >> i) == 0) {
				UINT32 hfid, tfid, pktcnt, ac_num = j / DRR_MAX_DW_PER_AC(pAd), ctrl = 0;
				UINT32 sta_num = i + (j % DRR_MAX_DW_PER_AC(pAd)) * 32, fl_que_ctrl[3] = {0};

				if (sta_num != wtbl_idx)
					continue;

				wdev = wdev_search_by_wcid(pAd, sta_num);

				if (wdev)
					wmmidx = HcGetWmmIdx(pAd, wdev);

				MTWF_PRINT("\tSTA%d AC%d: ", sta_num, ac_num);

				fl_que_ctrl[0] |= WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (ENUM_UMAC_LMAC_PORT_2 << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (ac_num << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
				fl_que_ctrl[0] |= (sta_num << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_WLANID_SHFT);
				HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
				HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_2_ADDR, &fl_que_ctrl[1]);
				HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_3_ADDR, &fl_que_ctrl[2]);
				hfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
				tfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
				pktcnt = (fl_que_ctrl[2] & WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
				MTWF_PRINT("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x",
						  tfid, hfid, pktcnt);

				if (((sta_pause[j % DRR_MAX_DW_PER_AC(pAd)] & 0x1 << i) >> i) == 1)
					ctrl = 2;

				if (((dis_sta_map[j % DRR_MAX_DW_PER_AC(pAd)] & 0x1 << i) >> i) == 1)
					ctrl = 1;

				MTWF_PRINT(" ctrl = %s", sta_ctrl_reg[ctrl]);
				MTWF_PRINT(" (wmmidx=%d)\n", wmmidx);
			}
		}
	}

end:
	if (ple_stat)
		os_free_mem(ple_stat);
	if (sta_pause)
		os_free_mem(sta_pause);
	if (dis_sta_map)
		os_free_mem(dis_sta_map);

	return TRUE;
}

#ifdef CONFIG_ATE
static UINT32 chip_get_tx_mibinfo(struct _RTMP_ADAPTER *ad, uint8_t band_idx, uint8_t tx_mode, uint8_t dbw)
{
	UINT32 mac_val = 0;
	return mac_val;
}
#endif /* CONFIG_ATE */

static INT chip_chk_exception_type(RTMP_ADAPTER *pAd)
{
	UINT32 macVal = 0;
	UINT32 exp_assert_proc_entry_cnt = 0;
	UINT32 macAddr = 0;

	macVal = 0;

	/* MT7992 TODO */
	macAddr = 0x220842c;
	HW_IO_READ32(pAd->hdev_ctrl, macAddr, &macVal);
	if (macVal == 0xdeadbeef)
		return 0;

	exp_assert_proc_entry_cnt = macVal;
	if (exp_assert_proc_entry_cnt == 0)
		return 0;
	else
		return 1;
}

VOID ShowFwDbgCnt(RTMP_ADAPTER *pAd)
{
}

VOID ShowCpuUtilSum(RTMP_ADAPTER *pAd)
{
	UINT32 busy_perc = 0;
	UINT32 peak_busy_perc = 0;
	UINT32 idle_cnt = 0;
	UINT32 peak_idle_cnt = 0;

	/* MT7992 TODO: check correct cr setting */
	HW_IO_READ32(pAd->hdev_ctrl, 0x41F030, &busy_perc);
	HW_IO_READ32(pAd->hdev_ctrl, 0x41F034, &peak_busy_perc);
	HW_IO_READ32(pAd->hdev_ctrl, 0x41F038, &idle_cnt);
	HW_IO_READ32(pAd->hdev_ctrl, 0x41F03c, &peak_idle_cnt);

	MTWF_PRINT("\n\n       cpu ultility\n");
	MTWF_PRINT("       Busy:%d%% Peak:%d%%\n",
				busy_perc, peak_busy_perc);
	MTWF_PRINT("       IDLE:%d Peak_ILDE:%d\n",
				idle_cnt, peak_idle_cnt);
}

#define SYSIRQ_INTERRUPT_HISTORY_NUM 10
VOID ShowIrqHistory(RTMP_ADAPTER *pAd)
{
	UINT32 macVal = 0;
	UINT32 i = 0;
	UINT32 start = 0;
	UINT32 idx = 0;
	uint8_t ucIrqDisIdx = 0;
	uint8_t ucIrqResIdx = 0;
	UINT32 irq_dis_time[SYSIRQ_INTERRUPT_HISTORY_NUM];
	UINT32 irq_dis_lp[SYSIRQ_INTERRUPT_HISTORY_NUM];
	UINT32 irq_res_time[SYSIRQ_INTERRUPT_HISTORY_NUM];
	UINT32 irq_res_lp[SYSIRQ_INTERRUPT_HISTORY_NUM];

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, 0x2170BC, &macVal);
	ucIrqResIdx = (macVal & 0xff);
	ucIrqDisIdx = ((macVal >> 8) & 0xff);

	MTWF_PRINT("\n\n\n       Irq Idx (Dis=%d Res=%d):\n",
		ucIrqDisIdx, ucIrqResIdx);

	HW_IO_READ32(pAd->hdev_ctrl, 0x2170B8, &start);

	for (i = 0; i < SYSIRQ_INTERRUPT_HISTORY_NUM; i++) {
		HW_IO_READ32(pAd->hdev_ctrl, (start + (i * 8)), &macVal);
		irq_dis_time[i] = macVal;
		HW_IO_READ32(pAd->hdev_ctrl, (start + (i * 8) + 4), &macVal);
		irq_dis_lp[i] = macVal;
	}

	HW_IO_READ32(pAd->hdev_ctrl, 0x2170B4, &start);

	for (i = 0; i < SYSIRQ_INTERRUPT_HISTORY_NUM; i++) {
		HW_IO_READ32(pAd->hdev_ctrl, (start + (i * 8)), &macVal);
		irq_res_time[i] = macVal;
		HW_IO_READ32(pAd->hdev_ctrl, (start + (i * 8) + 4), &macVal);
		irq_res_lp[i] = macVal;
	}

	MTWF_PRINT("\n       Dis Irq history (from old to new):\n");

	for (i = 0; i < SYSIRQ_INTERRUPT_HISTORY_NUM; i++) {
		idx = (i + ucIrqDisIdx) % SYSIRQ_INTERRUPT_HISTORY_NUM;
		MTWF_PRINT("      [%d].LP = 0x%x   time=%u\n",
			idx, irq_dis_lp[idx], irq_dis_time[idx]);
	}

	MTWF_PRINT("\n       Restore Irq history (from old to new):\n");

	for (i = 0; i < SYSIRQ_INTERRUPT_HISTORY_NUM; i++) {
		idx = (i + ucIrqResIdx) % SYSIRQ_INTERRUPT_HISTORY_NUM;
		MTWF_PRINT("      [%d].LP = 0x%x   time=%u\n",
			idx, irq_res_lp[idx], irq_res_time[idx]);
	}

}

VOID ShowLpHistory(RTMP_ADAPTER *pAd, uint8_t fgIsExp)
{
	UINT32 macVal = 0;
	UINT32 gpr_log_idx = 0;
	UINT32 oldest_idx = 0;
	UINT32 idx = 0;
	UINT32 i = 0;

	if (!fgIsExp) {
		/* disable LP recored */
		HW_IO_READ32(pAd->hdev_ctrl, 0x89050200, &macVal);
		macVal &= (~0x1);
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x89050200, macVal);
		udelay(100);
	}

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, 0x89050200, &macVal);
	gpr_log_idx = ((macVal >> 16) & 0x1f);
	oldest_idx = gpr_log_idx + 2;

	MTWF_PRINT("       lp history (from old to new):\n");

	for (i = 0; i < 16; i++) {
		idx = ((oldest_idx + 2*i + 1)%32);
		HW_IO_READ32(pAd->hdev_ctrl, (0x89050204 + idx*4), &macVal);
		MTWF_PRINT("       %d: 0x%x\n", i, macVal);
	}

	if (!fgIsExp) {
		/* enable LP recored */
		HW_IO_READ32(pAd->hdev_ctrl, 0x89050200, &macVal);
		macVal |= 0x1;
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x89050200, macVal);
	}
}

/* MT7992 TODO: check correct cr setting */
#define WF_MCU_WM_SW_DEF_CR_MSG_TRACE_PTR_ADDR             0x41F054
#define WF_MCU_WM_SW_DEF_CR_MSG_TRACE_NUM_ADDR             0x41F058
#define WF_MCU_WM_SW_DEF_CR_MSG_TRACE_IDX_ADDR             0x41F05C
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R0_PTR_ADDR             0x41F060
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R0_NUM_ADDR             0x41F064
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R1_PTR_ADDR             0x41F068
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R1_NUM_ADDR             0x41F06C
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R2_PTR_ADDR             0x41F070
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R2_NUM_ADDR             0x41F074
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R3_PTR_ADDR             0x41F078
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R3_NUM_ADDR             0x41F07C
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R4_PTR_ADDR             0x41F080
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R4_NUM_ADDR             0x41F084

#define MSG_HISTORY_NUM 64
#define MAX_TASK_NUM 3

#define WM_SW_DEF_PROGRAM_TRACE_BASE_ADDR                 (0x41F0E0)
#define WM_SW_DEF_PROGRAM_TRACE_TRACE_PTR_ADDR                (WM_SW_DEF_PROGRAM_TRACE_BASE_ADDR + 0x00)
#define WM_SW_DEF_PROGRAM_TRACE_TRACE_NUM_ADDR                (WM_SW_DEF_PROGRAM_TRACE_BASE_ADDR + 0x04)
#define WM_SW_DEF_PROGRAM_TRACE_TRACE_IDX_ADDR                (WM_SW_DEF_PROGRAM_TRACE_BASE_ADDR + 0x08)

VOID MemSectionRead(RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 length, UINT32 addr)
{
	UINT32 idx = 0;
	void *ptr = buf;

	while (idx < length) {
		HW_IO_READ32(pAd->hdev_ctrl, (addr + idx), ptr);
		idx += 4;
		ptr += 4;
	}
}

VOID ShowSchduleTrace(RTMP_ADAPTER *pAd)
{
	task_info_struct  task_info_g[MAX_TASK_NUM];
	UINT32 length = 0;
	UINT32 idx = 0;
	UINT32 km_total_time = 0;
	UINT32 addr = 0;
	cos_task_type tcb;
	cos_task_type *tcb_ptr;
	CHAR   name[3][15] = {
		"WMT   ", "WIFI   ", "WIFI2   "
	};

	length = MAX_TASK_NUM * sizeof(task_info_struct);
	MemSectionRead(pAd, (UCHAR *)&(task_info_g[0]), length, 0x215400);

	HW_IO_READ32(pAd->hdev_ctrl, 0x219838, &km_total_time);
	if (km_total_time == 0) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR, "km_total_time zero!\n");
		return;
	}

	MTWF_PRINT("\n\n\n       TASK    XTIME    RATIO    PREMPT CNT\n");

	for (idx = 0 ;  idx < MAX_TASK_NUM ; idx++) {
		addr = task_info_g[idx].task_id;
		MemSectionRead(pAd, (UCHAR *)&(tcb), sizeof(cos_task_type), addr);
		tcb_ptr = &(tcb);

		if (tcb_ptr) {
			MTWF_PRINT("       %s    %d    %d       %d\n",
				name[idx],
				tcb_ptr->tc_exe_time,
				(tcb_ptr->tc_exe_time*100/km_total_time),
				tcb_ptr->tc_pcount);
		}
	}

}

#define PROGRAM_TRACE_HISTORY_NUM 32
VOID ShowProgTrace(RTMP_ADAPTER *pAd)
{
	cos_program_trace_t *cos_program_trace_ptr = NULL;
	UINT32 trace_ptr = 0;
	UINT32 idx = 0;
	UINT32 old_idx = 0;
	UINT32 old_idx_addr = 0;
	UINT32 prev_idx = 0;
	UINT32 prev_time = 0;
	UINT32 curr_time = 0;
	UINT32 diff = 0;
	UINT32 length = 0;

	os_alloc_mem(pAd, (UCHAR **)&cos_program_trace_ptr, PROGRAM_TRACE_HISTORY_NUM * sizeof(cos_program_trace_t));
	if (!cos_program_trace_ptr) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			"can not allocate cos_program_trace_ptr memory\n");
		return;
	}
	os_zero_mem(cos_program_trace_ptr, PROGRAM_TRACE_HISTORY_NUM * sizeof(cos_program_trace_t));
	HW_IO_READ32(pAd->hdev_ctrl, WM_SW_DEF_PROGRAM_TRACE_TRACE_PTR_ADDR, &trace_ptr);
	HW_IO_READ32(pAd->hdev_ctrl, WM_SW_DEF_PROGRAM_TRACE_TRACE_IDX_ADDR, &old_idx_addr);
	HW_IO_READ32(pAd->hdev_ctrl, old_idx_addr, &old_idx);

	old_idx = old_idx & 0xff;

	length = PROGRAM_TRACE_HISTORY_NUM * sizeof(cos_program_trace_t);
	MemSectionRead(pAd, (UCHAR *)cos_program_trace_ptr, length, trace_ptr);

	MTWF_PRINT("\n");
	MTWF_PRINT("       prog trace:\n");

	for (idx = 0 ; idx < PROGRAM_TRACE_HISTORY_NUM ; idx++) {

		prev_idx = ((old_idx + 32 - 1) % 32);

		MTWF_PRINT("       (p_%d)t_id=%x/%d, m_id=%d, LP=0x%x, name=%s, ts4=%d, ",
			old_idx,
			cos_program_trace_ptr[old_idx].dest_id,
			cos_program_trace_ptr[old_idx].msg_sn,
			cos_program_trace_ptr[old_idx].msg_id,
			cos_program_trace_ptr[old_idx].LP,
			cos_program_trace_ptr[old_idx].name,
			cos_program_trace_ptr[old_idx].ts_gpt4);

		MTWF_PRINT("ts2=%d, ", cos_program_trace_ptr[old_idx].ts_gpt2);

		/* diff for gpt4 */
		prev_time = cos_program_trace_ptr[prev_idx].ts_gpt4;
		curr_time = cos_program_trace_ptr[old_idx].ts_gpt4;

		if (prev_time) {
			if ((cos_program_trace_ptr[prev_idx].dest_id == cos_program_trace_ptr[old_idx].dest_id) &&
				(cos_program_trace_ptr[prev_idx].msg_sn == cos_program_trace_ptr[old_idx].msg_sn)) {
				if (curr_time > prev_time)
					diff = curr_time - prev_time;
				else
					diff = 0xFFFFFFFF - prev_time + curr_time + 1;
			} else
				diff = 0xFFFFFFFF;
		} else
			diff = 0xFFFFFFFF;

		if (diff == 0xFFFFFFFF)
			MTWF_PRINT("diff4=NA, ");
		else
			MTWF_PRINT("diff4=%8d, ", diff);

		/* diff for gpt2 */
		prev_time = cos_program_trace_ptr[prev_idx].ts_gpt2;
		curr_time = cos_program_trace_ptr[old_idx].ts_gpt2;

		if (prev_time) {
			if ((cos_program_trace_ptr[prev_idx].dest_id == cos_program_trace_ptr[old_idx].dest_id) &&
				(cos_program_trace_ptr[prev_idx].msg_sn == cos_program_trace_ptr[old_idx].msg_sn)) {
				if (curr_time > prev_time)
					diff = curr_time - prev_time;
				else
					diff = 0xFFFFFFFF - prev_time + curr_time + 1;
			} else
				diff = 0xFFFFFFFF;
		} else
			diff = 0xFFFFFFFF;

		if (diff == 0xFFFFFFFF)
			MTWF_PRINT("diff2=NA\n");
		else
			MTWF_PRINT("diff2=%8d\n", diff);

		old_idx++;

		if (old_idx >= 32)
			old_idx = 0;
	}
	if (cos_program_trace_ptr)
		os_free_mem(cos_program_trace_ptr);
}

VOID ShowPcLpHistory(RTMP_ADAPTER *pAd, uint8_t fgIsExp, UINT mcu_type, UINT count)
{
	UINT32 i;
	UINT32 macVal = 0;

	if (mcu_type == WA_LOG_OUTPUT)
		macVal = WA_LOG_OUTPUT;
	else
		macVal = WM_LOG_OUTPUT;

	MTWF_PRINT("\x1b[32m%s log output\x1b[0m\n", (mcu_type == WA_LOG_OUTPUT) ? "WA" : "WM");

	//0:WM_LOG_OUTPUT 1:WA_LOG_OUTPUT
	HW_IO_WRITE32(pAd->hdev_ctrl,
	CONN_DBG_CTL_WF_MCU_DBGOUT_SEL_WF_MCU_DBGOUT_SEL_ADDR, macVal);

	macVal = 0x3f;
	HW_IO_WRITE32(pAd->hdev_ctrl,
	CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_SEL_WF_MCU_DBG_PC_LOG_SEL_ADDR, macVal);
	HW_IO_READ32(pAd->hdev_ctrl,
	CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_WF_MCU_DBG_PC_LOG_ADDR, &macVal);
	MTWF_PRINT("Current PC=%x\n", macVal);

	macVal = 0x20;
	HW_IO_WRITE32(pAd->hdev_ctrl,
	CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_SEL_WF_MCU_DBG_PC_LOG_SEL_ADDR, macVal);
	HW_IO_READ32(pAd->hdev_ctrl,
	CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_WF_MCU_DBG_PC_LOG_ADDR, &macVal);
	macVal = (macVal & 0x003E0000) >> 17;
	MTWF_PRINT("latest PC index = 0x%x\n", macVal);

	for (i = 0; i < 32; i++) {
		HW_IO_WRITE32(pAd->hdev_ctrl,
		CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_SEL_WF_MCU_DBG_PC_LOG_SEL_ADDR, i);
		HW_IO_READ32(pAd->hdev_ctrl,
		CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_WF_MCU_DBG_PC_LOG_ADDR, &macVal);
		MTWF_PRINT("PC log(%d)=0x%x\n", i, macVal);
	}

	macVal = 0x20;
	HW_IO_WRITE32(pAd->hdev_ctrl,
	CONN_DBG_CTL_WF_MCU_DBG_GPR_LOG_SEL_WF_MCU_DBG_GPR_LOG_SEL_ADDR, macVal);
	HW_IO_READ32(pAd->hdev_ctrl,
	CONN_DBG_CTL_WF_MCU_GPR_BUS_DBGOUT_LOG_WF_MCU_GPR_BUS_DBGOUT_LOG_ADDR, &macVal);
	macVal = (macVal & 0x003E0000) >> 17;
	MTWF_PRINT("latest LR index = 0x%x\n", macVal);

	for (i = 0; i < 32; i++) {
		HW_IO_WRITE32(pAd->hdev_ctrl,
		CONN_DBG_CTL_WF_MCU_DBG_GPR_LOG_SEL_WF_MCU_DBG_GPR_LOG_SEL_ADDR, i);
		HW_IO_READ32(pAd->hdev_ctrl,
		CONN_DBG_CTL_WF_MCU_GPR_BUS_DBGOUT_LOG_WF_MCU_GPR_BUS_DBGOUT_LOG_ADDR, &macVal);
		MTWF_PRINT("LR log(%d)=0x%x\n", i, macVal);
	}
}

static INT32 chip_show_fw_debg_info(struct _RTMP_ADAPTER *pAd, UINT cpu_type)
{
	UINT8 fgIsExp = FALSE;

	MTWF_PRINT("======[ShowPcLpHistory]======\n");
	ShowPcLpHistory(pAd, fgIsExp, cpu_type, CURRENT_PC_DUMP_COUNT);
	MTWF_PRINT("======[End ShowPcLpHistory]==\n");

	return 0;
}

static void write_to_file(struct file *file_w, void *msg, size_t size)
{
	FILE_WRITE_METHOD(file_w, msg, size);
}

static void wfdma_block_dump(struct _RTMP_ADAPTER *pAd, UINT32 start_address, UINT32 end_address, struct file *file_w)
{
	UCHAR msg[512];
	UINT16 row_size = 4, total_cnt = 0, row_cnt = 0, seq = 0, i = 0,
	total_dump_cnt = 0, dump_cnt = 0;
	UINT32 address = 0, macVal[4] = {0};
	int ret;

	NdisZeroMemory(msg, 512);

	total_cnt = (end_address - start_address) / 4 + 1;
	row_cnt = total_cnt / row_size + 1;

	for (seq = 0; seq < row_cnt; seq++) {
		address = start_address + seq * row_size * 4;
		if (total_cnt -  total_dump_cnt < 4)
			dump_cnt = total_cnt -  total_dump_cnt;
		else
			dump_cnt = 4;

		for (i = 0; i < dump_cnt; i++)
			HW_IO_READ32(pAd->hdev_ctrl, address + (i * 4), &macVal[i]);
		total_dump_cnt += dump_cnt;

		ret = snprintf(msg, sizeof(msg),
			"WFDMA, 151, %u, %u, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
				seq+1, row_cnt, address, macVal[0], macVal[1], macVal[2], macVal[3]);

		if (os_snprintf_error(sizeof(msg), ret) != 0) {
			MTWF_PRINT("%s(%d):snprintf error, check!\n", __func__, __LINE__);
			break;
		}

		write_to_file(file_w, msg, strlen(msg));
	}
}

static INT32 chip_show_wfdma_info(struct _RTMP_ADAPTER *pAd)
{
#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	mm_segment_t orig_fs;
#endif
	UCHAR fileName[64];
	struct file *file_w;
	INT ret;

#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	orig_fs = get_fs();
	set_fs(KERNEL_DS);
#endif

	ret = snprintf(fileName, sizeof(fileName), "/etc/wfdma_dump.log");

	if (os_snprintf_error(sizeof(fileName), ret) != 0) {
		MTWF_PRINT("%s(%d):snprintf error, check!\n",
			__func__, __LINE__);
		return false;
	}

	/* open file */
	file_w = filp_open(fileName, O_WRONLY | O_CREAT, 0);

	if (IS_ERR(file_w)) {
		MTWF_PRINT("-->2) %s: Error %ld opening %s\n", __func__,
		-PTR_ERR(file_w), fileName);
	} else {
		if (file_w->f_op)
			file_w->f_pos = 0;
		else
			goto  done;

	//HOST_DMA
	wfdma_block_dump(pAd, WF_WFDMA_HOST_DMA0_CBTOP_TXDMAD_0_ADDR,
	WF_WFDMA_HOST_DMA0_WPDMA_RX_RING13_PKT_MIB_ADDR, file_w);

	//MCU DMA
	wfdma_block_dump(pAd, WF_WFDMA_MCU_DMA0_CBTOP_TXDMAD_0_ADDR,
	WF_WFDMA_MCU_DMA0_WPDMA_RX_RING11_PKT_MIB_ADDR, file_w);

	//MEM_DMA
	wfdma_block_dump(pAd, WF_WFDMA_MEM_DMA_HOST_IF_TX_DONE_STS_ADDR,
	WF_WFDMA_MEM_DMA_WPDMA_RX_RING3_BKRS_CTRL2_ADDR, file_w);

	MTWF_PRINT("%s write done\n", fileName);
	}

done:
#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	set_fs(orig_fs);
#endif
	return true;
}

struct _COREDUMP_INFO {
	PCHAR Name;
	UINT32 StartAddr;
	UINT32 DumpSize;
};

static struct _COREDUMP_INFO WM_COREDUMP_INFO[] = {
{"ULM0_DUMP.bin", 0x00800000, 0x0004C000},
{"ULM1_DUMP.bin", 0x00900000, 0x00036000},
{"ULM2_DUMP.bin", 0x02200000, 0x00040000},
{"fw_dump_wmcpu.cmm", 0x00400000, 0x00028000},
{"CRAM_DUMP.bin", 0xE0000000, 0x00228000},
{"CONN_INFRA_DUMP.bin", 0x7C050000, 0x00008000},

{NULL,}
};

static struct _COREDUMP_INFO WA_COREDUMP_INFO[] = {
{"WACPU_CRAM.bin", 0xE0000000, 0x00010000},
{"fw_dump_wacpu.cmm", 0xE0010000, 0x00011800},
{"WACPU_ILM.bin", 0x10000000, 0x0001C000},
{"WACPU_DLM.bin", 0x10200000, 0x00064000},
{NULL,}
};

static INT32 chip_show_coredump_proc(struct _RTMP_ADAPTER *pAd, UINT mcu_type)
{

	RTMP_STRING *msg;
	UCHAR fileName[64];
	struct file *file_w;
#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	mm_segment_t orig_fs;
#endif
	UINT32 addr = 0;
	UINT32 end_addr = 0;
	UINT32 macVal = 0, ctrl_info_ram_addr = 0, coredump_occur = 0;
	UINT32 i = 0;
	int ret;
	struct _COREDUMP_INFO *coredump_array;

	if (mcu_type == WA_LOG_OUTPUT) {
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x7C056480, &ctrl_info_ram_addr);
		RTMP_IO_READ32(pAd->hdev_ctrl, ctrl_info_ram_addr + 4, &coredump_occur);
		if (coredump_occur != EXP_DUMP_PUTTING_DONE) {
			MTWF_PRINT("WA EXP_DUMP_STATE=%d\n", coredump_occur);
			return TRUE;
		}
		coredump_array = WA_COREDUMP_INFO;
	} else {
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x7C0564A8, &ctrl_info_ram_addr);
		RTMP_IO_READ32(pAd->hdev_ctrl, ctrl_info_ram_addr + 4, &coredump_occur);
		if (coredump_occur != EXP_DUMP_PUTTING_DONE) {
			MTWF_PRINT("WM EXP_DUMP_STATE=%d\n", coredump_occur);
			return TRUE;
		}
		coredump_array = WM_COREDUMP_INFO;
	}
	os_alloc_mem(NULL, (UCHAR **)&msg, 4);

	if (!msg)
		return TRUE;

	NdisZeroMemory(msg, 4);

#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	orig_fs = get_fs();
	set_fs(KERNEL_DS);
#endif

	while (coredump_array[i].Name != NULL) {
		ret = snprintf(fileName, sizeof(fileName), "/etc/%s", coredump_array[i].Name);

		if (os_snprintf_error(sizeof(fileName), ret) != 0) {
			MTWF_PRINT("%s(%d):snprintf error, check!\n",
				__func__, __LINE__);
			break;
		}

		/* open file */
		file_w = filp_open(fileName, O_WRONLY | O_CREAT, 0);

		if (IS_ERR(file_w)) {
			MTWF_PRINT("-->2) %s: Error %ld opening %s\n", __func__,
			-PTR_ERR(file_w), fileName);
		} else {
			if (file_w->f_op)
				file_w->f_pos = 0;
			else
				goto  done;

			addr = coredump_array[i].StartAddr;
			end_addr = addr + coredump_array[i].DumpSize - 1;
			MTWF_PRINT("%s open success  addr:0x%x\n", fileName, addr);

			while (addr <= end_addr) {
				HW_IO_READ32(pAd->hdev_ctrl, addr, &macVal);
				NdisCopyMemory(msg, &macVal, 4);
				addr += 4;
#if (KERNEL_VERSION(4, 1, 0) > LINUX_VERSION_CODE)
				if (file_w->f_op->write)
					file_w->f_op->write(file_w, msg, 4, &file_w->f_pos);
				else
					MTWF_PRINT("no file write method\n");
#elif (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
						kernel_write(file_w, msg, 4, &file_w->f_pos);
#else
						__vfs_write(file_w, msg, 4, &file_w->f_pos);
#endif
			}

			filp_close(file_w, NULL);
			MTWF_PRINT("%s write done\n", fileName);
		}
		i++;
	}

done:
#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	set_fs(orig_fs);
#endif
	os_free_mem(msg);

	return TRUE;
}

static INT32 chip_show_cmd_timeout_info(struct _RTMP_ADAPTER *pAd)
{
	UINT32 coredump_occur = 0,  macVal = 0, addr = 0x00400000, end_addr  = 0, dump_size =
	0, ctrl_info_ram_addr = 0;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	RTMP_IO_READ32(pAd->hdev_ctrl, 0x7C0564A8, &ctrl_info_ram_addr);
	RTMP_IO_READ32(pAd->hdev_ctrl, ctrl_info_ram_addr + 4, &coredump_occur);
	if (coredump_occur == EXP_DUMP_PUTTING_DONE && pAd->bIsBeenDumped == FALSE) {
		pAd->bIsBeenDumped = TRUE;
		//First step: show trinfo
		show_trinfo_proc(pAd, NULL);
		//Second step:show fw_dump_wmcpu.cmm
		RTMP_IO_READ32(pAd->hdev_ctrl, ctrl_info_ram_addr + 0x10, &dump_size);
		MTWF_PRINT("dump size= 0x%08x\n", dump_size);
		if (dump_size == 0) {
			dump_size = 0xA622;
			MTWF_PRINT("dump size(default)= 0x%08x\n", dump_size);
		}
		end_addr =  0x00400000 + dump_size - 1;
		while (addr <= end_addr) {
			HW_IO_READ32(pAd->hdev_ctrl, addr, &macVal);
			MTWF_PRINT("%c%c%c%c", (macVal & 0x000000FF), (macVal &
				0x0000FF00) >> 8, (macVal & 0x00FF0000) >> 16, (macVal &
				0xFF000000) >> 24);
			addr += 4;
		}
		MTWF_PRINT("\n");
		//step3:show current PC/pc &LR log
		ShowPcLpHistory(pAd, FALSE, 0, CMD_TIMEOUT_CURRENT_PC_DUMP_COUNT);
		//step4: save file
		chip_dbg->show_coredump_proc(pAd, 0);
		//step5: dump wfdma
		chip_dbg->show_wfdma_info(pAd);
	}
	return true;
}

static INT chip_set_fw_cp_util_en(RTMP_ADAPTER *pAd, UINT en)
{
	UINT32 macVal = 0;

	HW_IO_READ32(pAd->hdev_ctrl, 0x41F04c, &macVal);
	macVal &= ~(BIT(0));
	HW_IO_WRITE32(pAd->hdev_ctrl, 0x41F04c, macVal);

	if (en > 0) {
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x41F010, 0x2004);
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x89010080, 0x20);
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x890100c0, 0x20);
	} else {
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x41F010, 0x2005);
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x89010080, 0x20);
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x890100c0, 0x20);
	}

	return 0;
}

VOID mt7992_chip_dbg_init(struct _RTMP_CHIP_DBG *dbg_ops)
{
	dbg_ops->dump_ps_table = NULL;
	dbg_ops->dump_mib_info = chip_dump_mib_info;
	dbg_ops->show_tmac_info = chip_show_tmac_info;
	dbg_ops->show_agg_info = chip_show_agg_info;
	dbg_ops->show_dmasch_info = chip_show_dmasch_info;
	dbg_ops->show_pse_info = chip_show_pse_info;
	dbg_ops->show_pse_data = NULL; /* read PSE data from host is not supported */
	dbg_ops->show_protect_info = chip_show_protect_info;
	dbg_ops->show_cca_info = chip_show_cca_info;
	dbg_ops->set_cca_en = chip_set_cca_en;
	dbg_ops->show_txv_info = NULL;
	dbg_ops->check_txv = chip_check_txv;
	dbg_ops->show_bcn_info = chip_show_bcn_info;
	dbg_ops->show_ple_info = chip_show_ple_info;
	dbg_ops->get_pause_by_band = get_pause_by_band;
	dbg_ops->show_drr_info = chip_show_drr_info;
	dbg_ops->dump_wtbl_info = chip_dump_wtbl_info;
	dbg_ops->dump_wtbl_mlo_omac = chip_dump_wtbl_mlo_omac;
	dbg_ops->dump_wtbl_mac = chip_dump_wtbl_mac;
	dbg_ops->dump_wtbl_base_info = chip_dump_wtbl_base_info;
	dbg_ops->dump_ple_amsdu_count_info = chip_show_amsdu_info;
	dbg_ops->set_ba_winsize = chip_set_ba_winsize;
	dbg_ops->clear_keyloc = chip_clear_keyloc;
	dbg_ops->set_hw_amsdu = chip_set_hw_amsdu;
	dbg_ops->set_header_translation = chip_set_header_translation;
	dbg_ops->show_dma_info = chip_show_dma_info;
#ifdef RANDOM_PKT_GEN
	dbg_ops->set_txctrl_proc = chip_set_txctrl_proc;
	dbg_ops->regular_pause_umac = chip_regular_pause_umac;
#endif /* RANDOM_PKT_GEN */
	dbg_ops->get_lpon_frcr = chip_get_lpon_frcr;
#ifdef VOW_SUPPORT
	dbg_ops->show_sta_acq_info = chip_show_sta_acq_info;
	dbg_ops->show_txcmdq_info = chip_show_txcmdq_info;
	dbg_ops->get_ple_acq_stat = chip_get_ple_acq_stat;
	dbg_ops->get_ple_txcmd_stat = chip_get_ple_txcmd_stat;
	dbg_ops->get_dis_sta_map = chip_get_dis_sta_map;
	dbg_ops->get_sta_pause = chip_get_sta_pause;
	dbg_ops->get_obss_nonwifi_airtime = chip_get_obss_nonwifi_airtime;
	dbg_ops->get_sta_airtime = chip_get_sta_airtime;
	dbg_ops->get_sta_addr = chip_get_sta_addr;
	dbg_ops->get_sta_rate = chip_get_sta_rate;
	dbg_ops->get_sta_tx_cnt = chip_get_sta_tx_cnt;
	dbg_ops->set_sta_psm = chip_set_sta_psm;
	dbg_ops->get_sta_rx_rcpi = chip_get_sta_rx_rcpi;
#endif	/* VOW_SUPPORT */
#ifdef MT7992_FPGA
	dbg_ops->show_txv_info = chip_show_txv_info;
#endif /*MT7992_FPGA*/
#ifdef CONFIG_ATE
	dbg_ops->ctrl_manual_hetb_tx = chip_ctrl_manual_hetb_tx;
	dbg_ops->ctrl_manual_hetb_rx = chip_ctrl_manual_hetb_rx;
	dbg_ops->chip_ctrl_spe = chip_ctrl_asic_spe;
	dbg_ops->get_tx_mibinfo = chip_get_tx_mibinfo;
#endif
	dbg_ops->show_asic_rx_stat = chip_show_asic_rx_stat;
	dbg_ops->show_ple_info_by_idx = chip_show_ple_info_by_idx;
	dbg_ops->show_fw_dbg_info = chip_show_fw_debg_info;
	dbg_ops->show_wfdma_info = chip_show_wfdma_info;
	dbg_ops->show_coredump_proc = chip_show_coredump_proc;
	dbg_ops->show_cmd_timeout_info = chip_show_cmd_timeout_info;
	dbg_ops->set_cpu_util_en = chip_set_fw_cp_util_en;
	dbg_ops->chk_exception_type = chip_chk_exception_type;
}

#endif /* MT7992 */
