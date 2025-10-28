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

	Module Name: warp
	warp_dbg.c
*/

#include "warp_utility.h"
#include "warp.h"
#include "warp_hw.h"

int warp_get_ring_status(void *hw, u32 host_stat[], u8 host_stat_num,
	u32 wdma_stat[], u8 wdma_stat_num)
{
	struct warp_entry *warp = NULL;
	struct wed_entry *wed = NULL;
	struct wdma_entry *wdma = NULL;
	struct wifi_entry *wifi = NULL;
	struct wifi_hw *wifi_hw = NULL;
	struct wdma_rx_ring_ctrl *rx_ring_ctrl = NULL;
	struct wdma_tx_ring_ctrl *tx_ring_ctrl = NULL;
	u32 base = 0, cidx = 0, didx = 0, qcnt = 0, tcnt = 0, i = 0;

	if (!check_and_update_warp(&warp, &wed, &wdma, &wifi, hw))
		return 0;
	/* WED */
	wifi_hw = &wifi->hw;
	/* WDMA */
	rx_ring_ctrl = &wdma->res_ctrl.rx_ctrl.rx_ring_ctrl;
	tx_ring_ctrl = &wdma->res_ctrl.tx_ctrl.tx_ring_ctrl;

	if (wifi_hw->tx_ring_num + wifi_hw->rx_ring_num > host_stat_num ||
		rx_ring_ctrl->ring_num + tx_ring_ctrl->ring_num > wdma_stat_num)
		return -EINVAL;

#ifdef WED_TX_SUPPORT
	for (i = 0; i < wifi_hw->tx_ring_num; i++) {
		warp_wed_tx_ring_get_idx(wed, i, &base, &tcnt, &cidx, &didx);

		if (base > 0)
			qcnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + tcnt);
		else
			qcnt = 0;
		*(host_stat + i) = (qcnt << 16) | didx;

		warp_dbg(WARP_DBG_INF, "%s(): WED_TX%d Qcnt=%d tcnt=%d cidx=%d didx=%d\n",
			__func__, i, qcnt, tcnt, cidx, didx);
	}
#endif

#ifdef WED_RX_SUPPORT
	/* skip txfree done */
	for (i = 0; i < wifi_hw->rx_ring_num; i++) {
		warp_wed_rx_ring_get_idx(wed, i, &base, &tcnt, &cidx, &didx);

		if (base > 0)
			qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
		else
			qcnt = 0;
		*(host_stat + wifi_hw->tx_ring_num + i) = (qcnt << 16) | didx;

		warp_dbg(WARP_DBG_INF, "%s(): WED_RX%d Qcnt=%d tcnt=%d cidx=%d didx=%d\n",
				__func__, i, qcnt, tcnt, cidx, didx);
	}
#endif

#ifdef WED_HW_TX_SUPPORT
	for (i = 0; i < rx_ring_ctrl->ring_num; i++) {
		warp_wdma_rx_ring_get_idx(wdma, i, &base, &tcnt, &cidx, &didx);

		if (base > 0)
			qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
		else
			qcnt = 0;
		*(wdma_stat + i) = (qcnt << 16) | didx;

		warp_dbg(WARP_DBG_INF, "%s(): WDMA RX%d Qcnt=%d tcnt=%d cidx=%d didx=%d\n",
				__func__, i, qcnt, tcnt, cidx, didx);
	}
#endif

#ifdef WED_HW_RX_SUPPORT
	for (i = 0; i < tx_ring_ctrl->ring_num; i++) {
		warp_wdma_tx_ring_get_idx(wdma, i, &base, &tcnt, &cidx, &didx);

		if (base > 0)
			qcnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + tcnt);
		else
			qcnt = 0;
		*(wdma_stat + rx_ring_ctrl->ring_num + i) = (qcnt << 16) | didx;

		warp_dbg(WARP_DBG_INF, "%s(): WDMA TX%d Qcnt=%d tcnt=%d cidx=%d didx=%d\n",
				__func__, i, qcnt, tcnt, cidx, didx);
	}
#endif

	return 0;
}
EXPORT_SYMBOL(warp_get_ring_status);

