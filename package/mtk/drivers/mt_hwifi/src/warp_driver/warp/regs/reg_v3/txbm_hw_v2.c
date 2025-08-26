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
#include <mcu/warp_fwdl.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include "mcu/warp_woif.h"
#include <warp_bm.h>

void
warp_bfm_get_tx_freecnt_hw(struct wed_entry *wed, u32 *cnt)
{
	struct wed_buf_res *buf_res = &wed->res_ctrl.tx_ctrl.res;

	u32 value = 0;

	warp_io_read32(wed, WED_TX_TKID_STS, &value);
	warp_dbg(WARP_DBG_OFF, "WED_TX_TKID_STS:(0x%x)\n", value);

	warp_dbg(WARP_DBG_OFF,
		 "CR: Free Cnt(%d), All Token Cnt(%d)\n",
		 value, buf_res->token_num);

}

static void
warp_txm_tkid_init_hw(struct wed_entry *wed)
{
	u32 value;
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;

	/*PASUE TKID BUF MGMT*/
	value = (1 << WED_TX_TKID_CTRL_FLD_PAUSE);
	/*should be set before WED_MOD_RST is invoked*/
	value |= (res->tkn_rsv_grp << WED_TX_TKID_CTRL_FLD_RSV_GRP_NUM);
	value |= (res->tkn_vld_grp << WED_TX_TKID_CTRL_FLD_VLD_GRP_NUM);
	warp_io_write32(wed, WED_TX_TKID_CTRL, value);

	/* return SKBID + SDP back to BM */
	warp_io_read32(wed, WED_TX_TKID_CTRL, &value);
	value |= (1 << WED_TX_TKID_CTRL_FLD_FREE_FORMAT);
	warp_io_write32(wed, WED_TX_TKID_CTRL, value);

	/* TX token cfg */
	value = ((res->token_start) << WED_TX_TKID_TKID_FLD_START_ID);
	value |= ((res->token_end) << WED_TX_TKID_TKID_FLD_END_ID);
	warp_io_write32(wed, WED_TX_TKID_TKID, value);
}

int
warp_tx_bm_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value = 0;
	struct wifi_hw *hw = &wifi->hw;
	u32 pkt_nums = wed->res_ctrl.tx_ctrl.tx_bm.pkt_num;
	struct warp_dma_cb *cb = &wed->res_ctrl.tx_ctrl.tx_bm.ring->cell[0];

	/* token setting */
	warp_txm_tkid_init_hw(wed);
	warp_io_read32(wed, WED_MOD_RST, &value);
	value |= (1 << WED_MOD_RST_FLD_TX_BM);
	warp_io_write32(wed, WED_MOD_RST, value);

	warp_io_read32(wed, WED_TX_BM_BLEN, &value);
	value |= (hw->tx_pkt_size << WED_TX_BM_BLEN_FLD_BYTE_LEN);
	warp_io_write32(wed, WED_TX_BM_BLEN, value);

	value = (cb->alloc_pa << WED_TX_BM_BASE_FLD_PTR);
	warp_io_write32(wed, WED_TX_BM_BASE, value);

	value = 0;
	value |= (pkt_nums << WED_TX_BM_INIT_PTR_FLD_SW_TAIL_IDX);
	value |= (1 << WED_TX_BM_INIT_PTR_FLD_INIT_SW_TAIL_IDX);
	warp_io_write32(wed, WED_TX_BM_INIT_PTR, value);

	/* TODO: dynamic tx bm */

	/* Change to new BM */
	warp_io_read32(wed, WED_TX_BM_CTRL, &value);
	value &= ~(1 << WED_TX_BM_CTRL_FLD_LEGACY_EN);
	warp_io_write32(wed, WED_TX_BM_CTRL, value);

	/* token setting */
	warp_txm_tkid_init_hw(wed);

	/* Enable TX_BM to fetch DMAD */
	warp_io_read32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WED_TX_BM_EN |
		  1 << WED_CTRL_FLD_WED_TX_FREE_AGT_EN |
		  1 << WED_CTRL_FLD_WED_TX_TKID_ALI_EN);
	warp_io_write32(wed, WED_CTRL, value);

	/* tkid */
	warp_io_read32(wed, WED_TX_TKID_CTRL, &value);
	value &= ~(1 << WED_TX_TKID_CTRL_FLD_PAUSE);
	warp_io_write32(wed, WED_TX_TKID_CTRL, value);

	return 0;
}
