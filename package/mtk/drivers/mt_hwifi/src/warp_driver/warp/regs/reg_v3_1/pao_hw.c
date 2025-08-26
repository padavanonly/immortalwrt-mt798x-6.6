// SPDX-License-Identifier: <SPDX License Expression>
/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemptt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

 pao_hw.c (pao hw hal)

*/

#include <warp.h>
#include <warp_hw.h>
#include <warp_utility.h>
#include "wed_hw.h"
#include "pao_hw.h"

static void
warp_pao_set_hiftxd_src(struct wed_entry *wed)
{
	struct wed_hif_txd_ctrl *hif_txd_ctrl = &wed->res_ctrl.tx_ctrl.hif_txd_ctrl;
	u32 value;

	warp_io_read32(wed, WED_PAO_HIFTXD_ADDR, &value);
	value |= SET_FIELD(WED_PAO_HIFTXD_SRC, hif_txd_ctrl->hif_txd_src);
	warp_io_write32(wed, WED_PAO_HIFTXD_ADDR, value);
}

void
warp_pao_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value;
	struct wifi_hw *hw = &wifi->hw;

	/* init pao txd src */
	warp_pao_set_hiftxd_src(wed);

	/* Eagle E1 PCIE1 TxRing22 flow control issue */
	if (hw->chip_id == 0x7991) {
		warp_io_read32(wed, WED_PAO_AMSDU_FIFO_ADDR, &value);
		value &= (~WED_PAO_AMSDU_FIFO_IS_PRIOR0_RING_MASK);
		warp_io_write32(wed, WED_PAO_AMSDU_FIFO_ADDR, value);
	}

	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value |= WED_CTRL_WED_TX_PAO_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);
}

void
warp_pao_exit_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value;

	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value &= ~WED_CTRL_WED_TX_PAO_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);

	/* Reset counter */
	warp_io_read32(wed, WED_PAO_DBG_CFG_ADDR, &value);
	value |= WED_PAO_DBG_CFG_EN_MASK;
	warp_io_write32(wed, WED_PAO_DBG_CFG_ADDR, value);

	warp_io_read32(wed, WED_PAO_DBG_CNT_ADDR, &value);
	value |= WED_PAO_DBG_CNT_CLR_MASK;
	warp_io_write32(wed, WED_PAO_DBG_CNT_ADDR, value);

	warp_io_read32(wed, WED_PAO_DBG_CFG_ADDR, &value);
	value &= ~WED_PAO_DBG_CFG_EN_MASK;
	warp_io_write32(wed, WED_PAO_DBG_CFG_ADDR, value);
}

void
warp_pao_get_sta_info(struct wed_entry *wed, u16 wcid, u32 *pao_cr_value)
{
}

int
warp_pao_set_sta_info(struct wed_entry *wed, u16 wcid,
	u8 max_amsdu_nums, u32 max_amsdu_len, int remove_vlan, int hdrt_mode)
{
	return 0;
}

static void
warp_pao_amsdu_fifo_cnt(struct wed_entry *wed,
			struct seq_file *seq)
{
	dump_string(seq, "=======WED PAO AMSDU_FIFO======\n");
	dump_io(seq, wed, "DMAD_CNT", WED_PAO_MON_AMSDU_FIFO_CNT0_ADDR);
	dump_io(seq, wed, "MSDU_CNT (All)", WED_PAO_MON_AMSDU_FIFO_CNT7_ADDR);
	dump_io(seq, wed, "MSDU_CNT (S&G)", WED_PAO_MON_AMSDU_FIFO_CNT1_ADDR);
	dump_io(seq, wed, "DROP_MSDU_CNT", WED_PAO_MON_AMSDU_FIFO_CNT9_ADDR);
}

static void
warp_pao_hiftxd_cnt(struct wed_entry *wed,
			struct seq_file *seq)
{
	u32 addr, offset, value;
	int i;

	dump_string(seq, "=======WED PAO HIFTXD=========\n");
	dump_io(seq, wed, "HIFTXD_IN_CNT", WED_PAO_MON_HIFTXD_FETCH_CNT0_ADDR);
	dump_io(seq, wed, "HIFTXD_OU1_CNT", WED_PAO_MON_HIFTXD_FETCH_CNT1_ADDR);
	dump_io(seq, wed, "HIFTXD_OU2_CNT", WED_PAO_MON_HIFTXD_FETCH_CNT2_ADDR);

	i = 1;
	addr = WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1_ADDR;
	offset = WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_2_ADDR -
		 WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1_ADDR;

	dump_string(seq, "=======WED PAO HIFTXD BUF_NUM=========\n");
	while (addr <= WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_13_ADDR) {
		warp_io_read32(wed, addr, &value);
		seq_printf(seq, "HIFTXD_BUF_NUM%d_CNT\t: 0x%x\n", i, value);
		addr += offset;
		i++;
	}

	i = 1;
	addr = WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1_ADDR;
	offset = WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_2_ADDR -
		 WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1_ADDR;

	dump_string(seq, "=======WED PAO HIFTXD MSDU_NUM=========\n");
	while (addr <= WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_13_ADDR) {
		warp_io_read32(wed, addr, &value);
		seq_printf(seq, "HIFTXD_MSDU_NUM%d_CNT\t: 0x%x\n", i, value);
		addr += offset;
		i++;
	}
}

static void warp_pao_err_flag(struct wed_entry *wed,
			struct seq_file *seq)
{
	u32 value;

	warp_io_read32(wed, WED_PAO_AMSDU_FIFO_ERR_ADDR, &value);
	if (!value)
		return;

	dump_string(seq, "=======WED PAO ERR=========\n");
	seq_printf(seq, "EXCEED_MAX_BUF_NUM_ERR\t: 0x%x\n",
		GET_FIELD(WED_PAO_AMSDU_FIFO_ERR_JUMBO_BUF_NUM_ERR, value));
	seq_printf(seq, "ILLEGAL_TID_ERR\t: 0x%x\n",
		GET_FIELD(WED_PAO_AMSDU_FIFO_ERR_Illegal_TID_ERR, value));
}
int
warp_procinfo_dump_pao_hw(struct warp_entry *warp, struct seq_file *seq)
{
	struct wed_entry *wed = &warp->wed;

	warp_pao_amsdu_fifo_cnt(wed, seq);
	warp_pao_hiftxd_cnt(wed, seq);
	warp_pao_err_flag(wed, seq);

	return 0;
}

