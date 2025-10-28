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

*/

#include "wed.h"
#include "warp_bm.h"

int
wed_rx_tkid_free_buf(struct wed_entry *wed, u32 tkid)
{
	struct wed_res_ctrl *res_ctrl = &wed->res_ctrl;
	struct wed_rx_ctrl *rx_ctrl = &res_ctrl->rx_ctrl;
	struct wed_rx_tkid *rx_tkid = &rx_ctrl->rx_tkid;
	struct warp_dma_buf *desc = rx_tkid->desc;
	struct warp_tkid_rxdmad *dmad = desc->alloc_va;

	if (tkid >= rx_tkid->token_num)
		return -EINVAL;

	dmad = dmad + tkid;

	/* TODO: Call BMU API */

	return 0;
}

static int
wed_rx_tkid_desc_init(struct wed_entry *wed, struct wed_rx_tkid *rx_tkid)
{
	struct warp_dma_buf *desc = rx_tkid->desc;
	u32 len = rx_tkid->rxd_len * rx_tkid->ring_len;

	/* allocate dmad fifo ring */
	if (warp_dma_buf_alloc(wed->pdev, desc, len) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): Allocate desc failed, len = %u\n",
			__func__, len);
		return -ENOMEM;
	}

	return 0;
}

static void
wed_rx_tkid_desc_exit(struct wed_entry *wed, struct wed_rx_tkid *rx_tkid)
{
	struct warp_dma_buf *desc = rx_tkid->desc;

	warp_dma_buf_free(wed->pdev, desc);
}

void
wed_rx_tkid_exit(struct wed_entry *wed)
{
	struct wed_res_ctrl *res_ctrl = &wed->res_ctrl;
	struct wed_rx_ctrl *rx_ctrl = &res_ctrl->rx_ctrl;
	struct wed_rx_tkid *rx_tkid = &rx_ctrl->rx_tkid;

	if (rx_tkid->desc) {
		wed_rx_tkid_desc_exit(wed, rx_tkid);

		warp_os_free_mem(rx_tkid->desc);
		rx_tkid->desc = NULL;
	}
}

int
wed_rx_tkid_init(struct wed_entry *wed, struct wifi_hw *hw)
{
	struct wed_res_ctrl *res_ctrl = &wed->res_ctrl;
	struct wed_rx_ctrl *rx_ctrl = &res_ctrl->rx_ctrl;
	struct wed_rx_tkid *rx_tkid = &rx_ctrl->rx_tkid;

	rx_tkid->tkn_grp_sz = 0x80;	/* 128 token per group */
	rx_tkid->token_num = hw->hw_rx_token_num;
	rx_tkid->tkn_vld_grp = (rx_tkid->token_num / rx_tkid->tkn_grp_sz);

	rx_tkid->ring_len = rx_tkid->token_num;
	rx_tkid->rxd_len = sizeof(struct warp_tkid_rxdmad);
	rx_tkid->hif_dev = hw->hif_dev;

	warp_os_alloc_mem((unsigned char **)&rx_tkid->desc,
		sizeof(struct warp_dma_buf), GFP_ATOMIC);

	if (!rx_tkid->desc) {
		warp_dbg(WARP_DBG_ERR, "%s(): Allocate rx_tkid desc failed\n", __func__);
		goto err;
	}

	memset(rx_tkid->desc, 0, sizeof(struct warp_dma_buf));

	if (wed_rx_tkid_desc_init(wed, rx_tkid) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): Init rx_tkid desc failed\n", __func__);
		goto err;
	}

	return 0;

err:
	wed_rx_tkid_exit(wed);
	return -1;
}
