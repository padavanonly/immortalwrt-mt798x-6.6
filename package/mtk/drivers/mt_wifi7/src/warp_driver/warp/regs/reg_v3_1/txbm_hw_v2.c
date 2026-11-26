// SPDX-License-Identifier: <SPDX License Expression>
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

 txbm_v2_hw.c (txbm hal for warp3.0)

*/
#include <warp.h>
#include <warp_hw.h>
#include <warp_utility.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include <warp_bm.h>
#include "wed_hw.h"

void
warp_bfm_get_tx_freecnt_hw(struct wed_entry *wed, u32 *cnt)
{
	struct wed_buf_res *buf_res = &wed->res_ctrl.tx_ctrl.res;

	u32 value = 0;

	warp_io_read32(wed, WED_TX_TKID_STS_ADDR, &value);
	warp_dbg(WARP_DBG_OFF, "WED_TX_TKID_STS:(0x%x)\n", value);

	warp_dbg(WARP_DBG_OFF,
		 "CR: Free Cnt(%d), All Token Cnt(%d)\n",
		 value, buf_res->token_num);

}

static void
warp_tkid_init_hw(struct wed_entry *wed)
{
	u32 value;
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;

	value = SET_FIELD(WED_TX_TKID_CTRL_RSV_GRP_NUM, res->tkn_rsv_grp);
	value |= SET_FIELD(WED_TX_TKID_CTRL_VLD_GRP_NUM, res->tkn_vld_grp);
	warp_io_write32(wed, WED_TX_TKID_CTRL_ADDR, value);

	/* TX token cfg */
	value = SET_FIELD(WED_TX_TKID_TKID_START_ID, res->token_start);
	value |= SET_FIELD(WED_TX_TKID_TKID_END_ID, res->token_end);
	warp_io_write32(wed, WED_TX_TKID_TKID_ADDR, value);

	/* Set the threshold that will not interrupt to host */
	value = SET_FIELD(WED_TX_TKID_DYN_TH_LOW_GRP_NUM, 0);
	value |= SET_FIELD(WED_TX_TKID_DYN_TH_HI_GRP_NUM, res->tkn_vld_grp);
	warp_io_write32(wed, WED_TX_TKID_DYN_TH_ADDR, value);
}

int
warp_tx_bm_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value = 0;
	struct wifi_hw *hw = &wifi->hw;
	u32 pkt_nums = wed->res_ctrl.tx_ctrl.tx_bm.pkt_num;
	struct warp_dma_cb *cb = &wed->res_ctrl.tx_ctrl.tx_bm.ring->cell[0];

	/* token setting */
	warp_tkid_init_hw(wed);

	value = WED_MOD_RST_TX_BM_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);

	if (cb->alloc_pa & 0xFFF) {
		warp_dbg(WARP_DBG_ERR, "%s(): address %pad is not 4KB alignment\n",
			__func__, &cb->alloc_pa);
		return -EINVAL;
	}

	value = SET_FIELD(WED_TX_BM_BASE_PTR, cb->alloc_pa);
	warp_io_write32(wed, WED_TX_BM_BASE_ADDR, value);

	value = SET_FIELD(WED_TX_BM_BLEN_BYTE_LEN, hw->tx_pkt_size);
#ifdef CONFIG_WARP_64BIT_SUPPORT
	value |= SET_FIELD(WED_TX_BM_BLEN_BASE_PTR_H, cb->alloc_pa >> 32);
#endif
	warp_io_write32(wed, WED_TX_BM_BLEN_ADDR, value);

	value = SET_FIELD(WED_TX_BM_RANGE_CFG_SW_CFG_BUF_IDX, pkt_nums);
	warp_io_write32(wed, WED_TX_BM_RANGE_CFG_ADDR, value);

	value = SET_FIELD(WED_TX_BM_INIT_PTR_SW_TAIL_IDX, pkt_nums);
	value |= WED_TX_BM_INIT_PTR_INIT_SW_TAIL_HEAD_IDX_MASK;
	warp_io_write32(wed, WED_TX_BM_INIT_PTR_ADDR, value);

	/* Enable TX_BM to fetch DMAD */
	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value |= (WED_CTRL_WED_TX_BM_EN_MASK |
		  WED_CTRL_WED_TX_FREE_AGT_EN_MASK |
		  WED_CTRL_WED_TX_TKID_ALI_EN_MASK);
	warp_io_write32(wed, WED_CTRL_ADDR, value);

	return 0;
}
