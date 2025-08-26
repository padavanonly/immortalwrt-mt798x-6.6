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

 pao_hw.c (pao hw hal)

*/

#include <warp.h>
#include <warp_hw.h>
#include <regs/reg_v3/pao_hw.h>
#include <warp_utility.h>

/* amsdu length per 256-bytes */
#define WED_PAO_LEN_SHIFT_256_bytes 8
#define WED_PAO_NUM_MAX_CAP 13
#define WED_PAO_LEN_MAX_CAP 16384

#define WED_PAO_NUM_LEN_SET_TO_DEFAULT 0
#define WED_PAO_NUM_LEN_SKIP 1
#define WED_PAO_VLAN_SET_TO_DEFAULT -1
#define WED_PAO_HDRT_SET_TO_DEFAULT -1
#define WED_PAO_VLAN_SKIP 2
#define WED_PAO_HDRT_SKIP 2

static int
warp_pao_hif_txd_init_base(struct wed_entry *wed)
{
	struct wed_hif_txd_ctrl *hif_txd_ctrl = &wed->res_ctrl.tx_ctrl.hif_txd_ctrl;
	u32 offset = WED_PAO_HIFTXD_BASE01_L_OFFSET - WED_PAO_HIFTXD_BASE00_L_OFFSET;
	u32 shft_offset = WED_PAO_HIFTXD_BASE_00_07_H_BASE00_SHFT -
			WED_PAO_HIFTXD_BASE_00_07_H_BASE01_SHFT;
	u32 segment_nums = hif_txd_ctrl->hif_txd_segment_nums;
	dma_addr_t hif_txd_pa;
	u32 value, i;

	if (segment_nums > 32) {
		warp_dbg(WARP_DBG_OFF, "%s(): Segment nums %u more than 32\n",
			__func__, segment_nums);
		return -EINVAL;
	}

	for (i = 0; i < segment_nums; i++) {
		hif_txd_pa = hif_txd_ctrl->hif_txd_addr_pa[i];

		value = SET_FIELD(WED_PAO_HIFTXD_BASE00_L_PTR, hif_txd_pa);
		warp_io_write32(wed, WED_PAO_HIFTXD_BASE00_L_OFFSET + i * offset, value);

#ifdef CONFIG_WARP_64BIT_SUPPORT
		warp_io_read32(wed, WED_PAO_HIFTXD_BASE_00_07_H_OFFSET + ((i >> 3) * offset),
			&value);

		/* Each BASE_H CR need to clear for first setup */
		value = ((i & 0x7) == 0) ? 0 : value;
		value |= ((hif_txd_pa >> 32) & WED_PAO_HIFTXD_BASE_00_07_H_BASE07_MASK) <<
			(WED_PAO_HIFTXD_BASE_00_07_H_BASE00_SHFT - ((i & 0x7) * shft_offset));
		warp_io_write32(wed, WED_PAO_HIFTXD_BASE_00_07_H_OFFSET + ((i >> 3) * offset),
			value);
#endif
	}

	return 0;
}

int
hif_txd_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	return warp_pao_hif_txd_init_base(wed);
}

int
warp_pao_all_sta_info_init(struct wed_entry *wed, u8 max_amsdu_nums,
		u32 max_amsdu_len, bool remove_vlan, bool hdrt_mode)
{
	u32 value, cnt = 0;
	int ret = 0;

	max_amsdu_len >>= WED_PAO_LEN_SHIFT_256_bytes;

	warp_io_read32(wed, WED_PAO_STA_INFO_INIT_OFFSET, &value);
	value &= ~WED_PAO_STA_INFO_INIT_MAX_AMSDU_NUM_MASK;
	value |= (max_amsdu_nums << WED_PAO_STA_INFO_INIT_MAX_AMSDU_NUM_SHFT);
	value &= ~WED_PAO_STA_INFO_INIT_MAX_AMSDU_LEN_MASK;
	value |= (max_amsdu_len << WED_PAO_STA_INFO_INIT_MAX_AMSDU_LEN_SHFT);
	value |= (remove_vlan << WED_PAO_STA_INFO_INIT_RMVL_SHFT);
	value |= (hdrt_mode << WED_PAO_STA_INFO_INIT_CFG_WTBL_HDRT_MODE_SHFT);
	warp_io_write32(wed, WED_PAO_STA_INFO_INIT_OFFSET, value);

	value = (1 << WED_PAO_STA_INFO_DO_INIT_SHFT);
	warp_io_write32(wed, WED_PAO_STA_INFO_OFFSET, value);

	warp_io_read32(wed, WED_PAO_STA_INFO_OFFSET, &value);

	while ((value & WED_PAO_STA_INFO_DO_INIT_MASK) && (cnt < WED_POLL_MAX)) {
		warp_io_read32(wed, WED_PAO_STA_INFO_OFFSET, &value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_OFF, "%s(): init pao_sta_info fail:", __func__);
		ret = -1;
	} else {
		warp_dbg(WARP_DBG_OFF, "%s(): init uccess:", __func__);
		ret = 0;
	}
	warp_dbg(WARP_DBG_OFF,
			"max_amsdu_nums/len/hdrt/rm_vlan = %d/%d/%d/%d\n",
			max_amsdu_nums, max_amsdu_len << WED_PAO_LEN_SHIFT_256_bytes,
			hdrt_mode, remove_vlan);

	return ret;
}

void
warp_pao_get_sta_info(struct wed_entry *wed, u16 wcid, u32 *pao_cr_value)
{
	u32 set_cr_value = 0;

	set_cr_value |= (wcid << WED_PAO_STA_INFO_SET_NUMBER_SHFT);
	warp_io_write32(wed, WED_PAO_STA_INFO_SET_OFFSET, set_cr_value);

	set_cr_value = 0;
	set_cr_value = (1 << WED_PAO_STA_INFO_DO_GET_SHFT);
	warp_io_write32(wed, WED_PAO_STA_INFO_OFFSET, set_cr_value);
	warp_io_read32(wed, WED_PAO_STA_INFO_STS_OFFSET, pao_cr_value);

}

int
warp_pao_set_sta_info(struct wed_entry *wed, u16 wcid,
		u8 max_amsdu_nums, u32 max_amsdu_len, int remove_vlan, int hdrt_mode)
{
	u32 value = 0, cnt = 0, pao_info_cr = 0;

	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wifi_entry *wifi;
	struct wifi_hw *hw;

	wifi = &warp->wifi;
	hw = &wifi->hw;

	warp_pao_get_sta_info(wed, wcid, &pao_info_cr);

	/*PAO AMSDU NUMS/LEN*/
	if (max_amsdu_nums == WED_PAO_NUM_LEN_SET_TO_DEFAULT ||
		max_amsdu_len == WED_PAO_NUM_LEN_SET_TO_DEFAULT ||
		max_amsdu_nums >= WED_PAO_NUM_MAX_CAP ||
		max_amsdu_len > WED_PAO_LEN_MAX_CAP) {
		max_amsdu_nums = hw->max_amsdu_nums;
		max_amsdu_len = hw->max_amsdu_len;
	} else if (max_amsdu_nums == WED_PAO_NUM_LEN_SKIP &&
				max_amsdu_len == WED_PAO_NUM_LEN_SKIP) {
		/*DOUT[20:17]: AMSDU_NUM*/
		max_amsdu_nums = (pao_info_cr & 0x001E0000) >> 17;
		/*DOUT[16:11]: AMSDU_LEN*/
		max_amsdu_len  = (pao_info_cr & 0x0001F800) >> 11;
	} else
		warp_dbg(WARP_DBG_INF, "%s():set mas_amsdu_nums/len set= %d/%d\n", __func__,
			max_amsdu_nums, max_amsdu_len);

	max_amsdu_len >>= WED_PAO_LEN_SHIFT_256_bytes;

	/*PAO VLAN*/
	if (remove_vlan <= WED_PAO_VLAN_SET_TO_DEFAULT)
		remove_vlan = hw->rm_vlan;
	else if (remove_vlan >= WED_PAO_VLAN_SKIP) {
		/*DOUT[10]: RMVL*/
		remove_vlan = (pao_info_cr & 0x00000400) >> 10;
	} else
		warp_dbg(WARP_DBG_INF, "%s():set remove_vlan set= %d\n", __func__, remove_vlan);

	/*PAO HDRT*/
	if (hdrt_mode <= WED_PAO_HDRT_SET_TO_DEFAULT)
		hdrt_mode = hw->hdtr_mode;
	else if (hdrt_mode >= WED_PAO_HDRT_SKIP) {
		/*DOUT[9]: HDRT*/
		hdrt_mode = (pao_info_cr & 0x00000200) >> 9;
	} else
		warp_dbg(WARP_DBG_INF, "%s():set hdrt_mode set= %d\n", __func__, hdrt_mode);

	warp_io_read32(wed, WED_PAO_STA_INFO_SET_OFFSET, &value);
	value &= ~WED_PAO_STA_INFO_SET_NUMBER_MASK;
	value |= (wcid << WED_PAO_STA_INFO_SET_NUMBER_SHFT);
	value &= ~WED_PAO_STA_INFO_SET_MAX_AMSDU_NUM_MASK;
	value |= (max_amsdu_nums << WED_PAO_STA_INFO_SET_MAX_AMSDU_NUM_SHFT);
	value &= ~WED_PAO_STA_INFO_SET_MAX_AMSDU_LEN_MASK;
	value |= (max_amsdu_len << WED_PAO_STA_INFO_SET_MAX_AMSDU_LEN_SHFT);
	value |= (hdrt_mode << WED_PAO_STA_INFO_INIT_CFG_WTBL_HDRT_MODE_SHFT);
	value |= (remove_vlan << WED_PAO_STA_INFO_INIT_RMVL_SHFT);
	warp_io_write32(wed, WED_PAO_STA_INFO_SET_OFFSET, value);

	/* do set pao cr*/
	value = (1 << WED_PAO_STA_INFO_DO_SET_SHFT);
	warp_io_write32(wed, WED_PAO_STA_INFO_OFFSET, value);

	warp_io_read32(wed, WED_PAO_STA_INFO_STS_OFFSET, &value);

	while ((value & WED_PAO_STA_INFO_STS_SET_BUSY_MASK) && (cnt < WED_POLL_MAX)) {
		warp_io_read32(wed, WED_PAO_STA_INFO_STS_OFFSET, &value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_OFF, "%s():set fail wcid: %d, amsdu_nums: %d, amsdu_len:%d, rm_vlan:%d, hdrt:%d\n",
				__func__, wcid, max_amsdu_nums,
				max_amsdu_len << WED_PAO_LEN_SHIFT_256_bytes, remove_vlan, hdrt_mode);
		return -1;
	} else {
		warp_dbg(WARP_DBG_INF, "%s():set success wcid: %d, amsdu_nums: %d, amsdu_len:%d, rm_vlan:%d, hdrt:%d\n",
				__func__, wcid, max_amsdu_nums,
				max_amsdu_len << WED_PAO_LEN_SHIFT_256_bytes, remove_vlan, hdrt_mode);
		return 0;
	}

}

static void
warp_pao_set_hiftxd_src(struct wed_entry *wed)
{
	struct wed_hif_txd_ctrl *hif_txd_ctrl = &wed->res_ctrl.tx_ctrl.hif_txd_ctrl;
	u32 value = 0;

	warp_io_read32(wed, WED_PAO_HIFTXD_OFFSET, &value);
	value &= ~WED_PAO_HIFTXD_SRC_MASK;
	value |= (hif_txd_ctrl->hif_txd_src << WED_PAO_HIFTXD_SRC_SHFT);
	warp_io_write32(wed, WED_PAO_HIFTXD_OFFSET, value);
}

static int
warp_pao_qmem_init(struct wed_entry *wed)
{
	u32 value, cnt = 0;

	warp_io_read32(wed, WED_PAO_PSE_OFFSET, &value);
	value |= (1 << WED_PAO_PSE_RESET_SHFT);
	warp_io_write32(wed, WED_PAO_PSE_OFFSET, value);

	warp_io_read32(wed, WED_PAO_MON_QMEM_STS1_OFFSET, &value);

	while ((value & (0x1 << 29)) && (cnt < WED_POLL_MAX)) {
		warp_io_read32(wed, WED_PAO_MON_QMEM_STS1_OFFSET, &value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_OFF, "%s(): init pao qmem fail\n", __func__);
		return -1;
	} else {
		warp_dbg(WARP_DBG_INF, "%s(): init pao qmem success\n", __func__);
		return 0;
	}
}

void
warp_pao_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value;
	struct wifi_hw *hw = &wifi->hw;

	/* init all sta parameter */
	warp_pao_all_sta_info_init(wed, hw->max_amsdu_nums,
			hw->max_amsdu_len, hw->rm_vlan, hw->hdtr_mode);

	/* init pao txd src */
	warp_pao_set_hiftxd_src(wed);

	/* init qmem */
	warp_pao_qmem_init(wed);

	/* Eagle E1 PCIE1 TxRing22 flow control issue */
	if (hw->chip_id == 0x7991) {
		warp_io_read32(wed, WED_PAO_AMSDU_FIFO_OFFSET, &value);
		value &= (~WED_PAO_AMSDU_FIFO_IS_PRIOR0_RING_MASK);
		warp_io_write32(wed, WED_PAO_AMSDU_FIFO_OFFSET, value);
	}

	warp_io_read32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WED_TX_PAO_EN);
	warp_io_write32(wed, WED_CTRL, value);
}

void
warp_pao_exit_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value;

	warp_io_read32(wed, WED_CTRL, &value);
	value &= ~(1 << WED_CTRL_FLD_WED_TX_PAO_EN);
	warp_io_write32(wed, WED_CTRL, value);

	value = 1 << WED_MOD_RST_FLD_TX_PAO;
	WHNAT_RESET(wed, WED_MOD_RST, value);
}

static void
warp_pao_amsdu_fifo_cnt(struct wed_entry *wed,
			struct seq_file *seq)
{
	u32 value;

	dump_string(seq, "=======WED PAO AMSDU FIFO=======\n");
	warp_io_read32(wed, WED_PAO_MON_AMSDU_FIFO_CNT0_OFFSET, &value);
	seq_printf(seq, "amsdu_fifo_dmad_cnt:%d\n", value);
}

static void
_warp_pao_amsdu_eng_cnt(struct wed_entry *wed,
			struct seq_file *seq, int i)
{
	u32 value;
	u32 offset = WED_PAO_MON_AMSDU_ENG1_CNT3_OFFSET -
		     WED_PAO_MON_AMSDU_ENG0_CNT3_OFFSET;

	dump_string(seq, "=======WED PAO AMSDU ENG%d=======\n", i);
	warp_io_read32(wed, WED_PAO_MON_AMSDU_ENG0_CNT3_OFFSET + i * offset,
			&value);
	seq_printf(seq, "eng%d_dmad_cnt:%d\n", i, value);

	warp_io_read32(wed, WED_PAO_MON_AMSDU_ENG0_CNT4_OFFSET + i * offset,
			&value);
	seq_printf(seq, "eng%d_qfpl_cnt:%d\n", i, value);

	warp_io_read32(wed, WED_PAO_MON_AMSDU_ENG0_CNT5_OFFSET + i * offset,
			&value);
	seq_printf(seq, "eng%d_qeni_cnt:%d\n", i, value);

	warp_io_read32(wed, WED_PAO_MON_AMSDU_ENG0_CNT6_OFFSET + i * offset,
			&value);
	seq_printf(seq, "eng%d_qeno_cnt:%d\n", i, value);

	warp_io_read32(wed, WED_PAO_MON_AMSDU_ENG0_CNT7_OFFSET + i * offset,
			&value);
	seq_printf(seq, "eng%d_merge_cnt:%d\n", i, value);

	warp_io_read32(wed, WED_PAO_MON_AMSDU_ENG0_CNT8_OFFSET + i * offset,
			&value);
	seq_printf(seq, "eng%d_max_search:%d\n", i,
			(value & WED_PAO_MON_AMSDU_ENG0_CNT8_MAX_QGPP_CNT_MASK)
			>> WED_PAO_MON_AMSDU_ENG0_CNT8_MAX_QGPP_CNT_SHFT);
	seq_printf(seq, "eng%d_max_entry:%d\n", i,
			(value & WED_PAO_MON_AMSDU_ENG0_CNT8_MAX_PL_CNT_MASK)
			 >> WED_PAO_MON_AMSDU_ENG0_CNT8_MAX_PL_CNT_SHFT);

	warp_io_read32(wed, WED_PAO_MON_AMSDU_ENG0_CNT9_OFFSET + i * offset,
			&value);
	seq_printf(seq, "eng%d_cur_entry:%d\n", i,
			(value & WED_PAO_MON_AMSDU_ENG0_CNT9_PORTQ_CNT_MASK)
			>> WED_PAO_MON_AMSDU_ENG0_CNT9_PORTQ_CNT_SHFT);
	seq_printf(seq, "eng%d_max_buf:%d\n", i,
			(value & WED_PAO_MON_AMSDU_ENG0_CNT9_MAX_BUF_NUM_MASK)
			>> WED_PAO_MON_AMSDU_ENG0_CNT9_MAX_BUF_NUM_SHFT);
	seq_printf(seq, "eng%d_max_msdu:%d\n", i,
			(value & WED_PAO_MON_AMSDU_ENG0_CNT9_MAX_MSDU_CNT_MASK)
			>> WED_PAO_MON_AMSDU_ENG0_CNT9_MAX_MSDU_CNT_SHFT);
}

static void
warp_pao_amsdu_eng_cnt(struct wed_entry *wed,
			struct seq_file *seq)
{
	int amsdu_eng_num = 9;
	int i;

	for (i = 0; i < amsdu_eng_num; i++)
		_warp_pao_amsdu_eng_cnt(wed, seq, i);
}

static void
warp_pao_qmem_cnt(struct wed_entry *wed,
			struct seq_file *seq)
{
	u32 value;

	dump_string(seq, "=======WED PAO QMEM=======\n");
	warp_io_read32(wed, WED_PAO_MON_QMEM_CNT0_OFFSET, &value);
	seq_printf(seq, "free_q entry cnt: %d\n",
			(value & WED_PAO_MON_QMEM_CNT0_FQ_PCNT_MASK)
			>> WED_PAO_MON_QMEM_CNT0_FQ_PCNT_SHFT);
	seq_printf(seq, "is_sp_q entry cnt: %d\n",
			(value & WED_PAO_MON_QMEM_CNT0_Q0_PCNT_MASK)
			>> WED_PAO_MON_QMEM_CNT0_Q0_PCNT_SHFT);
	
	warp_io_read32(wed, WED_PAO_MON_QMEM_CNT1_OFFSET, &value);
	seq_printf(seq, "tid0_q entry cnt: %d\n",
			(value & WED_PAO_MON_QMEM_CNT1_Q1_PCNT_MASK)
			>> WED_PAO_MON_QMEM_CNT1_Q1_PCNT_SHFT);
	seq_printf(seq, "tid1_q entry cnt: %d\n",
			(value & WED_PAO_MON_QMEM_CNT1_Q2_PCNT_MASK)
			>> WED_PAO_MON_QMEM_CNT1_Q2_PCNT_SHFT);

	warp_io_read32(wed, WED_PAO_MON_QMEM_CNT2_OFFSET, &value);
	seq_printf(seq, "tid2_q entry cnt: %d\n",
			(value & WED_PAO_MON_QMEM_CNT2_Q3_PCNT_MASK)
			>> WED_PAO_MON_QMEM_CNT2_Q3_PCNT_SHFT);
	seq_printf(seq, "tid3_q entry cnt: %d\n",
			(value & WED_PAO_MON_QMEM_CNT2_Q4_PCNT_MASK)
			>> WED_PAO_MON_QMEM_CNT2_Q4_PCNT_SHFT);

	warp_io_read32(wed, WED_PAO_MON_QMEM_CNT3_OFFSET, &value);
	seq_printf(seq, "tid4_q entry cnt: %d\n",
			(value & WED_PAO_MON_QMEM_CNT3_Q5_PCNT_MASK)
			>> WED_PAO_MON_QMEM_CNT3_Q5_PCNT_SHFT);
	seq_printf(seq, "tid5_q entry cnt: %d\n",
			(value & WED_PAO_MON_QMEM_CNT3_Q6_PCNT_MASK)
			>> WED_PAO_MON_QMEM_CNT3_Q6_PCNT_SHFT);

	warp_io_read32(wed, WED_PAO_MON_QMEM_CNT4_OFFSET, &value);
	seq_printf(seq, "tid6_q entry cnt: %d\n",
			(value & WED_PAO_MON_QMEM_CNT4_Q7_PCNT_MASK)
			>> WED_PAO_MON_QMEM_CNT4_Q7_PCNT_SHFT);
	seq_printf(seq, "tid7_q entry cnt: %d\n",
			(value & WED_PAO_MON_QMEM_CNT4_Q8_PCNT_MASK)
			>> WED_PAO_MON_QMEM_CNT4_Q8_PCNT_SHFT);
}

static void
warp_pao_qmem_head_cnt(struct wed_entry *wed,
			struct seq_file *seq)
{
	u32 value;

	dump_string(seq, "=======WED PAO QMEM HEAD=======\n");
	warp_io_read32(wed, WED_PAO_MON_QMEM_PTR0_OFFSET, &value);
	seq_printf(seq, "free_q head: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR0_FQ_START_MASK)
			>> WED_PAO_MON_QMEM_PTR0_FQ_START_SHFT);
	seq_printf(seq, "is_sp_q head: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR0_Q0_START_MASK)
			>> WED_PAO_MON_QMEM_PTR0_Q0_START_SHFT);

	warp_io_read32(wed, WED_PAO_MON_QMEM_PTR1_OFFSET, &value);
	seq_printf(seq, "tid0_q head: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR1_Q1_START_MASK)
			>> WED_PAO_MON_QMEM_PTR1_Q1_START_SHFT);
	seq_printf(seq, "tid1_q head: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR1_Q2_START_MASK)
			>> WED_PAO_MON_QMEM_PTR1_Q2_START_SHFT);

	warp_io_read32(wed, WED_PAO_MON_QMEM_PTR2_OFFSET, &value);
	seq_printf(seq, "tid2_q head: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR2_Q3_START_MASK)
			>> WED_PAO_MON_QMEM_PTR2_Q3_START_SHFT);
	seq_printf(seq, "tid3_q head: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR2_Q4_START_MASK)
			>> WED_PAO_MON_QMEM_PTR2_Q4_START_SHFT);

	warp_io_read32(wed, WED_PAO_MON_QMEM_PTR3_OFFSET, &value);
	seq_printf(seq, "tid4_q head: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR3_Q5_START_MASK)
			>> WED_PAO_MON_QMEM_PTR3_Q5_START_SHFT);
	seq_printf(seq, "tid5_q head: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR3_Q6_START_MASK)
			>> WED_PAO_MON_QMEM_PTR3_Q6_START_SHFT);

	warp_io_read32(wed, WED_PAO_MON_QMEM_PTR4_OFFSET, &value);
	seq_printf(seq, "tid6_q head: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR4_Q7_START_MASK)
			>> WED_PAO_MON_QMEM_PTR4_Q7_START_SHFT);
	seq_printf(seq, "tid7_q head: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR4_Q8_START_MASK)
			>> WED_PAO_MON_QMEM_PTR4_Q8_START_SHFT);
}

static void
warp_pao_qmem_tail_cnt(struct wed_entry *wed,
			struct seq_file *seq)
{
	u32 value;

	dump_string(seq, "=======WED PAO QMEM TAIL=======\n");
	warp_io_read32(wed, WED_PAO_MON_QMEM_PTR5_OFFSET, &value);
	seq_printf(seq, "free_q tail: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR5_FQ_END_MASK)
			>> WED_PAO_MON_QMEM_PTR5_FQ_END_SHFT);
	seq_printf(seq, "is_sp_q tail: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR5_Q0_END_MASK)
			>> WED_PAO_MON_QMEM_PTR5_Q0_END_SHFT);

	warp_io_read32(wed, WED_PAO_MON_QMEM_PTR6_OFFSET, &value);
	seq_printf(seq, "tid0_q tail: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR6_Q1_END_MASK)
			>> WED_PAO_MON_QMEM_PTR6_Q1_END_SHFT);
	seq_printf(seq, "tid1_q tail: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR6_Q2_END_MASK)
			>> WED_PAO_MON_QMEM_PTR6_Q2_END_SHFT);

	warp_io_read32(wed, WED_PAO_MON_QMEM_PTR7_OFFSET, &value);
	seq_printf(seq, "tid2_q tail: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR7_Q3_END_MASK)
			>> WED_PAO_MON_QMEM_PTR7_Q3_END_SHFT);
	seq_printf(seq, "tid3_q tail: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR7_Q4_END_MASK)
			>> WED_PAO_MON_QMEM_PTR7_Q4_END_SHFT);

	warp_io_read32(wed, WED_PAO_MON_QMEM_PTR8_OFFSET, &value);
	seq_printf(seq, "tid4_q tail: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR8_Q5_END_MASK)
			>> WED_PAO_MON_QMEM_PTR8_Q5_END_SHFT);
	seq_printf(seq, "tid5_q tail: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR8_Q6_END_MASK)
			>> WED_PAO_MON_QMEM_PTR8_Q6_END_SHFT);

	warp_io_read32(wed, WED_PAO_MON_QMEM_PTR9_OFFSET, &value);
	seq_printf(seq, "tid6 tail: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR9_Q7_END_MASK)
			>> WED_PAO_MON_QMEM_PTR9_Q7_END_SHFT);
	seq_printf(seq, "tid7 tail: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR9_Q8_END_MASK)
			>> WED_PAO_MON_QMEM_PTR9_Q8_END_SHFT);
}

static void
warp_pao_qmem_pre_cnt(struct wed_entry *wed,
			struct seq_file *seq)
{
	u32 value;

	dump_string(seq, "=======WED PAO QMEM PRE=======\n");
	warp_io_read32(wed, WED_PAO_MON_QMEM_PTR10_OFFSET, &value);
	seq_printf(seq, "is_sp_q pre: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR10_Q0_CPP_MASK)
			>> WED_PAO_MON_QMEM_PTR10_Q0_CPP_SHFT);

	warp_io_read32(wed, WED_PAO_MON_QMEM_PTR11_OFFSET, &value);
	seq_printf(seq, "tid0_q pre: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR11_Q1_CPP_MASK)
			>> WED_PAO_MON_QMEM_PTR11_Q1_CPP_SHFT);
	seq_printf(seq, "tid1_q pre: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR11_Q2_CPP_MASK)
			>> WED_PAO_MON_QMEM_PTR11_Q2_CPP_SHFT);

	warp_io_read32(wed, WED_PAO_MON_QMEM_PTR12_OFFSET, &value);
	seq_printf(seq, "tid2_q pre: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR12_Q3_CPP_MASK)
			>> WED_PAO_MON_QMEM_PTR12_Q3_CPP_SHFT);
	seq_printf(seq, "tid3_q pre: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR12_Q4_CPP_MASK)
			>> WED_PAO_MON_QMEM_PTR12_Q4_CPP_SHFT);

	warp_io_read32(wed, WED_PAO_MON_QMEM_PTR13_OFFSET, &value);
	seq_printf(seq, "tid4_q pre: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR13_Q5_CPP_MASK)
			>> WED_PAO_MON_QMEM_PTR13_Q5_CPP_SHFT);
	seq_printf(seq, "tid5_q pre: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR13_Q6_CPP_MASK)
			>> WED_PAO_MON_QMEM_PTR13_Q6_CPP_SHFT);

	warp_io_read32(wed, WED_PAO_MON_QMEM_PTR14_OFFSET, &value);
	seq_printf(seq, "tid6 pre: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR14_Q7_CPP_MASK)
			>> WED_PAO_MON_QMEM_PTR14_Q7_CPP_SHFT);
	seq_printf(seq, "tid7 pre: 0x%x\n",
			(value & WED_PAO_MON_QMEM_PTR14_Q8_CPP_MASK)
			>> WED_PAO_MON_QMEM_PTR14_Q8_CPP_SHFT);
}

static void
warp_pao_hiftxd_cnt(struct wed_entry *wed,
			struct seq_file *seq)
{
	u32 value;

	dump_string(seq, "=======WED PAO HIF_TXD=======\n");
	warp_io_read32(wed, WED_PAO_MON_HIFTXD_FETCH_CNT0_OFFSET, &value);
	seq_printf(seq, "amsdu_hiftxd_in_cnt: %d\n", value);
	warp_io_read32(wed, WED_PAO_MON_HIFTXD_FETCH_CNT1_OFFSET, &value);
	seq_printf(seq, "amsdu_hiftxd_out0_cnt: %d\n", value);
	warp_io_read32(wed, WED_PAO_MON_HIFTXD_FETCH_CNT2_OFFSET, &value);
	seq_printf(seq, "amsdu_hiftxd_out1_cnt: %d\n", value);
}

static void
warp_pao_hiftxd_buf_cnt(struct wed_entry *wed,
			struct seq_file *seq)
{
	int i = 1;
	u32 value;
	u32 addr = WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1_OFFSET;
	u32 offset = WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_2_OFFSET -
		     WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1_OFFSET;

	dump_string(seq, "=======WED PAO HIF_TXD BUF=======\n");
	while (addr <= WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_13_OFFSET) {
		warp_io_read32(wed, addr, &value);
		seq_printf(seq, "amsdu_buf_num_%d: %d\n", i, value);

		addr += offset;
		i++;
	}
}

static void
warp_pao_hiftxd_msdu_cnt(struct wed_entry *wed,
			struct seq_file *seq)
{
	int i = 1;
	u32 value;
	u32 addr = WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1_OFFSET;
	u32 offset = WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_2_OFFSET -
		     WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1_OFFSET;

	dump_string(seq, "=======WED PAO HIF_TXD MSDU=======\n");
	while (addr <= WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_13_OFFSET) {
		warp_io_read32(wed, addr, &value);
		seq_printf(seq, "amsdu_msdu_cnt_%d: %d\n", i, value);

		addr += offset;
		i++;
	}
}

int
warp_procinfo_dump_pao_hw(struct warp_entry *warp, struct seq_file *seq)
{
	struct wed_entry *wed = &warp->wed;

	warp_pao_amsdu_fifo_cnt(wed, seq);
	warp_pao_amsdu_eng_cnt(wed, seq);
	warp_pao_qmem_cnt(wed, seq);
	warp_pao_qmem_head_cnt(wed, seq);
	warp_pao_qmem_tail_cnt(wed, seq);
	warp_pao_qmem_pre_cnt(wed, seq);
	warp_pao_hiftxd_cnt(wed, seq);
	warp_pao_hiftxd_buf_cnt(wed, seq);
	warp_pao_hiftxd_msdu_cnt(wed, seq);

	return 0;
}

