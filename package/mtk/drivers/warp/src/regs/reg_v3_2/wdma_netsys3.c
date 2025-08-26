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

#include "wdma_netsys.h"
#include "wdma_hw.h"

static inline int
wdma_busy_chk(struct wdma_entry *wdma, u32 addr, u32 busy_mask)
{
	u32 cnt = 0;
	u32 value;

	warp_io_read32(wdma, addr, &value);

	while ((value & busy_mask) && cnt < WED_POLL_MAX) {
		udelay(CHK_DELAY_US);
		warp_io_read32(wdma, addr, &value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_WAR, "%s(): addr 0x%x busy mask 0x%x fail!!\n",
			__func__, addr, busy_mask);
		return -EBUSY;
	}

	return 0;
}

static inline int
wdma_rdy_chk(struct wdma_entry *wdma, u32 addr, u32 rdy_mask)
{
	u32 cnt = 0;
	u32 value;

	warp_io_read32(wdma, addr, &value);

	while (((value & rdy_mask) != rdy_mask) && cnt < WED_POLL_MAX) {
		udelay(CHK_DELAY_US);
		warp_io_read32(wdma, addr, &value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_WAR, "%s(): addr 0x%x ready mask 0x%x fail!!\n",
			__func__, addr, rdy_mask);
		return -EBUSY;
	}

	return 0;
}

int
wdma_rx_ring_rdy_ck(struct wdma_entry *wdma,
	struct wdma_rx_ring_ctrl *ring_ctrl)
{
	int i, j;
	u32 value;
	struct warp_ring *ring;

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		j = 0;

		while (j < 3) {
			mdelay(CHK_DELAY_MS);
			ring = &ring_ctrl->ring[i];
			warp_io_read32(wdma, ring->hw_cidx_addr, &value);

			if (value == (ring_ctrl->ring_len - 1))
				break;

			j++;
		}

		if (j >= 3) {
			warp_dbg(WARP_DBG_WAR, "%s(): cidx = 0x%x not equal to 0x%x!!\n",
				__func__, value, (ring_ctrl->ring_len - 1));
			return -EBUSY;
		}
	}

	return 0;
}

static bool
wdma_tx_ring_reset_state(struct wdma_entry *wdma,
	struct wdma_tx_ring_ctrl *ring_ctrl)
{
	int i;
	u32 base_value, cnt_value, cidx_value, didx_value;
	struct warp_ring *ring;

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ring = &ring_ctrl->ring[i];
		warp_io_read32(wdma, ring->hw_desc_base, &base_value);
		warp_io_read32(wdma, ring->hw_cnt_addr, &cnt_value);
		/* TxRing read DIDX first */
		warp_io_read32(wdma, ring->hw_didx_addr, &didx_value);
		warp_io_read32(wdma, ring->hw_cidx_addr, &cidx_value);

		if (base_value || cnt_value || didx_value || cidx_value)
			return false;
	}

	return true;
}

static bool
wdma_rx_ring_reset_state(struct wdma_entry *wdma,
	struct wdma_rx_ring_ctrl *ring_ctrl)
{
	int i;
	u32 base_value, cnt_value, cidx_value, didx_value;
	struct warp_ring *ring;

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ring = &ring_ctrl->ring[i];
		warp_io_read32(wdma, ring->hw_desc_base, &base_value);
		warp_io_read32(wdma, ring->hw_cnt_addr, &cnt_value);
		warp_io_read32(wdma, ring->hw_cidx_addr, &cidx_value);
		warp_io_read32(wdma, ring->hw_didx_addr, &didx_value);

		if (base_value || cnt_value || cidx_value || didx_value)
			return false;
	}

	return true;
}

static int
wdma_tx_ring_poll_to_idle(struct wdma_entry *wdma,
	struct wdma_tx_ring_ctrl *ring_ctrl)
{
	int i, j;
	u32 cidx_value, didx_value;
	struct warp_ring *ring;

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		j = 0;

		while (j < 3) {
			mdelay(CHK_DELAY_MS);
			ring = &ring_ctrl->ring[i];
			/* TxRing read DIDX first */
			warp_io_read32(wdma, ring->hw_didx_addr, &didx_value);
			warp_io_read32(wdma, ring->hw_cidx_addr, &cidx_value);

			if (cidx_value == didx_value)
				break;

			j++;
		}

		if (j >= 3) {
			warp_dbg(WARP_DBG_WAR,
				"%s(): ring_idx = %d, cidx = 0x%x not equal to didx = 0x%x\n",
				__func__, i, cidx_value, didx_value);
			return -EBUSY;
		}
	}

	return 0;
}

static int
wdma_rx_ring_poll_to_idle(struct wdma_entry *wdma,
	struct wdma_rx_ring_ctrl *ring_ctrl)
{
	int i, j;
	u32 didx_value, prev_didx, unchange_cnt;
	struct warp_ring *ring;

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		prev_didx = ring_ctrl->ring_len;
		unchange_cnt = 0;
		j = 0;

		while (j < 3) {
			mdelay(CHK_DELAY_MS);
			ring = &ring_ctrl->ring[i];
			warp_io_read32(wdma, ring->hw_didx_addr, &didx_value);

			if (prev_didx == didx_value) {
				if (++unchange_cnt >= 10)
					break;
				continue;
			}

			prev_didx = didx_value;
			unchange_cnt = 0;
			j++;
		}

		if (j >= 3) {
			warp_dbg(WARP_DBG_WAR, "%s(): ring idx = %d can't remain constant\n",
				__func__, i);
			return -EBUSY;
		}
	}

	return 0;
}

static int
wdma_tx_pref_busy_check(struct wdma_entry *wdma)
{
	int ret;

	ret = wdma_busy_chk(wdma, WDMA_PREF_TX_CFG_ADDR, WDMA_PREF_TX_CFG_BUSY_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): Polling TX failed", __func__);

	return ret;
}

static int
wdma_rx_pref_busy_check(struct wdma_entry *wdma)
{
	int ret;

	ret = wdma_busy_chk(wdma, WDMA_PREF_RX_CFG_ADDR, WDMA_PREF_RX_CFG_BUSY_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): Polling RX failed", __func__);

	return ret;
}

int
wdma_tx_ring_init_hw(struct wdma_entry *wdma,
	struct wdma_tx_ring_ctrl *tx_ring_ctrl, int idx)
{
	struct warp_ring *tx_ring = &tx_ring_ctrl->ring[idx];
	u32 value;

	value = SET_FIELD(WDMA_TX_BASE_PTR_0_TX_BASE_PTR, tx_ring->cell[0].alloc_pa);
	warp_io_write32(wdma, tx_ring->hw_desc_base, value);

	value = SET_FIELD(WDMA_TX_MAX_CNT_0_TX_MAX_CNT, tx_ring_ctrl->ring_len);
	warp_io_write32(wdma, tx_ring->hw_cnt_addr, value);
	warp_io_write32(wdma, tx_ring->hw_cidx_addr, 0);
	warp_io_write32(wdma, tx_ring->hw_didx_addr, 0);

	return 0;
}

int
wdma_rx_ring_init_hw(struct wdma_entry *wdma,
	struct wdma_rx_ring_ctrl *rx_ring_ctrl, int idx)
{
	struct warp_ring *rx_ring = &rx_ring_ctrl->ring[idx];
	u32 value;

	value = SET_FIELD(WDMA_RX_BASE_PTR_0_RX_BASE_PTR, rx_ring->cell[0].alloc_pa);
	warp_io_write32(wdma, rx_ring->hw_desc_base, value);

	value = SET_FIELD(WDMA_RX_MAX_CNT_0_RX_MAX_CNT, rx_ring_ctrl->ring_len);
	warp_io_write32(wdma, rx_ring->hw_cnt_addr, value);
	warp_io_write32(wdma, rx_ring->hw_cidx_addr, 0);

	return 0;
}

static int
_wdma_pref_ctrl(struct wdma_entry *wdma, u8 txrx)
{
	u32 ret = 0;
	u32 wdma_tx_pref_cfg, wdma_rx_pref_cfg = 0;

	warp_io_read32(wdma, WDMA_PREF_TX_CFG_ADDR, &wdma_tx_pref_cfg);
	warp_io_read32(wdma, WDMA_PREF_RX_CFG_ADDR, &wdma_rx_pref_cfg);

	switch (txrx) {
	case WARP_DMA_TX:
		wdma_rx_pref_cfg |= WDMA_PREF_RX_CFG_PREF_EN_MASK;
		warp_io_write32(wdma, WDMA_PREF_RX_CFG_ADDR, wdma_rx_pref_cfg);
		break;
	case WARP_DMA_RX:
		wdma_tx_pref_cfg |= WDMA_PREF_TX_CFG_PREF_EN_MASK;
		warp_io_write32(wdma, WDMA_PREF_TX_CFG_ADDR, wdma_tx_pref_cfg);
		break;
	case WARP_DMA_TXRX:
		wdma_tx_pref_cfg |= WDMA_PREF_TX_CFG_PREF_EN_MASK;
		wdma_rx_pref_cfg |= WDMA_PREF_RX_CFG_PREF_EN_MASK;
		warp_io_write32(wdma, WDMA_PREF_TX_CFG_ADDR, wdma_tx_pref_cfg);
		warp_io_write32(wdma, WDMA_PREF_RX_CFG_ADDR, wdma_rx_pref_cfg);
		break;
	case WARP_DMA_TX_DISABLE:
		wdma_rx_pref_cfg &= ~WDMA_PREF_RX_CFG_PREF_EN_MASK;
		warp_io_write32(wdma, WDMA_PREF_RX_CFG_ADDR, wdma_rx_pref_cfg);
		ret = wdma_rx_pref_busy_check(wdma);
		break;
	case WARP_DMA_RX_DISABLE:
		wdma_tx_pref_cfg &= ~WDMA_PREF_TX_CFG_PREF_EN_MASK;
		warp_io_write32(wdma, WDMA_PREF_TX_CFG_ADDR, wdma_tx_pref_cfg);
		ret = wdma_tx_pref_busy_check(wdma);
		break;
	case WARP_DMA_DISABLE:
		wdma_rx_pref_cfg &= ~WDMA_PREF_RX_CFG_PREF_EN_MASK;
		wdma_tx_pref_cfg &= ~WDMA_PREF_TX_CFG_PREF_EN_MASK;
		warp_io_write32(wdma, WDMA_PREF_RX_CFG_ADDR, wdma_rx_pref_cfg);
		warp_io_write32(wdma, WDMA_PREF_TX_CFG_ADDR, wdma_tx_pref_cfg);
		ret = wdma_rx_pref_busy_check(wdma) | wdma_tx_pref_busy_check(wdma);
		break;
	default:
		warp_dbg(WARP_DBG_INF, "%s(): Unknown DMA control (%d).\n",
			 __func__, txrx);
		break;
	}
	return ret;
}

static int
wdma_tx_glo_busy_check(struct wdma_entry *wdma)
{
	int ret;

	ret = wdma_busy_chk(wdma, WDMA_GLO_CFG0_ADDR, WDMA_GLO_CFG0_TX_DMA_BUSY_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): Polling TX failed", __func__);

	return ret;
}

static int
wdma_rx_glo_busy_check(struct wdma_entry *wdma)
{
	int ret;

	ret = wdma_busy_chk(wdma, WDMA_GLO_CFG0_ADDR, WDMA_GLO_CFG0_RX_DMA_BUSY_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): Polling RX failed", __func__);

	return ret;
}

static int
_wdma_dma_ctrl(struct wdma_entry *wdma, u8 txrx)
{
	u32 wdma_glo_cfg;
	u32 ret = 0;

	warp_io_read32(wdma, WDMA_GLO_CFG0_ADDR, &wdma_glo_cfg);
	warp_dbg(WARP_DBG_OFF, "%s(): WDMA_GLO_CFG0=%x, txrx = %d\n",
		__func__, wdma_glo_cfg, txrx);

	switch (txrx) {
	case WARP_DMA_TX:
		wdma_glo_cfg |= WDMA_GLO_CFG0_RX_DMA_EN_MASK;
		warp_io_write32(wdma, WDMA_GLO_CFG0_ADDR, wdma_glo_cfg);
		break;
	case WARP_DMA_RX:
		wdma_glo_cfg |= WDMA_GLO_CFG0_TX_DMA_EN_MASK;
		warp_io_write32(wdma, WDMA_GLO_CFG0_ADDR, wdma_glo_cfg);
		break;
	case WARP_DMA_TXRX:
		wdma_glo_cfg |= WDMA_GLO_CFG0_TX_DMA_EN_MASK;
		wdma_glo_cfg |= WDMA_GLO_CFG0_RX_DMA_EN_MASK;
		warp_io_write32(wdma, WDMA_GLO_CFG0_ADDR, wdma_glo_cfg);
		break;
	case WARP_DMA_TX_DISABLE:
		wdma_glo_cfg &= ~WDMA_GLO_CFG0_RX_DMA_EN_MASK;
		warp_io_write32(wdma, WDMA_GLO_CFG0_ADDR, wdma_glo_cfg);
		ret = wdma_rx_glo_busy_check(wdma);
		break;
	case WARP_DMA_RX_DISABLE:
		wdma_glo_cfg &= ~WDMA_GLO_CFG0_TX_DMA_EN_MASK;
		warp_io_write32(wdma, WDMA_GLO_CFG0_ADDR, wdma_glo_cfg);
		ret = wdma_tx_glo_busy_check(wdma);
		break;
	case WARP_DMA_DISABLE:
		wdma_glo_cfg &= ~(WDMA_GLO_CFG0_TX_DMA_EN_MASK |
				  WDMA_GLO_CFG0_RX_DMA_EN_MASK);
		warp_io_write32(wdma, WDMA_GLO_CFG0_ADDR, wdma_glo_cfg);
		ret = wdma_tx_glo_busy_check(wdma) | wdma_rx_glo_busy_check(wdma);
		break;
	default:
		warp_dbg(WARP_DBG_INF, "%s(): Unknown DMA control (%d).\n",
			 __func__, txrx);
		break;
	}
	return ret;
}

static int
wdma_tx_wrbk_busy_check(struct wdma_entry *wdma)
{
	int ret;

	ret = wdma_busy_chk(wdma, WDMA_WRBK_TX_CFG_ADDR, WDMA_WRBK_TX_CFG_BUSY_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): Polling TX failed", __func__);

	return ret;
}

static int
wdma_rx_wrbk_busy_check(struct wdma_entry *wdma)
{
	int ret;

	ret = wdma_busy_chk(wdma, WDMA_WRBK_RX_CFG_ADDR, WDMA_WRBK_RX_CFG_BUSY_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): Polling RX failed", __func__);

	return ret;
}


static int
_wdma_wrbk_ctrl(struct wdma_entry *wdma, u8 txrx)
{
	u32 wdma_tx_wrbk_cfg, wdma_rx_wrbk_cfg = 0;
	u32 ret = 0;

	warp_io_read32(wdma, WDMA_WRBK_TX_CFG_ADDR, &wdma_tx_wrbk_cfg);
	warp_io_read32(wdma, WDMA_WRBK_RX_CFG_ADDR, &wdma_rx_wrbk_cfg);

	switch (txrx) {
	case WARP_DMA_TX:
		wdma_rx_wrbk_cfg |= WDMA_WRBK_RX_CFG_WRBK_EN_MASK;
		warp_io_write32(wdma, WDMA_WRBK_RX_CFG_ADDR, wdma_rx_wrbk_cfg);
		break;
	case WARP_DMA_RX:
		wdma_tx_wrbk_cfg |= WDMA_WRBK_TX_CFG_WRBK_EN_MASK;
		warp_io_write32(wdma, WDMA_WRBK_TX_CFG_ADDR, wdma_tx_wrbk_cfg);
		break;
	case WARP_DMA_TXRX:
		wdma_rx_wrbk_cfg |= WDMA_WRBK_RX_CFG_WRBK_EN_MASK;
		wdma_tx_wrbk_cfg |= WDMA_WRBK_TX_CFG_WRBK_EN_MASK;
		warp_io_write32(wdma, WDMA_WRBK_RX_CFG_ADDR, wdma_rx_wrbk_cfg);
		warp_io_write32(wdma, WDMA_WRBK_TX_CFG_ADDR, wdma_tx_wrbk_cfg);
		break;
	case WARP_DMA_TX_DISABLE:
		wdma_rx_wrbk_cfg &= ~WDMA_WRBK_RX_CFG_WRBK_EN_MASK;
		warp_io_write32(wdma, WDMA_WRBK_RX_CFG_ADDR, wdma_rx_wrbk_cfg);
		ret = wdma_rx_wrbk_busy_check(wdma);
		break;
	case WARP_DMA_RX_DISABLE:
		wdma_tx_wrbk_cfg &= ~WDMA_WRBK_TX_CFG_WRBK_EN_MASK;
		warp_io_write32(wdma, WDMA_WRBK_TX_CFG_ADDR, wdma_tx_wrbk_cfg);
		ret = wdma_tx_wrbk_busy_check(wdma);
		break;
	case WARP_DMA_DISABLE:
		wdma_tx_wrbk_cfg &= ~WDMA_WRBK_TX_CFG_WRBK_EN_MASK;
		wdma_rx_wrbk_cfg &= ~WDMA_WRBK_RX_CFG_WRBK_EN_MASK;
		warp_io_write32(wdma, WDMA_WRBK_TX_CFG_ADDR, wdma_tx_wrbk_cfg);
		warp_io_write32(wdma, WDMA_WRBK_RX_CFG_ADDR, wdma_rx_wrbk_cfg);
		ret = wdma_rx_wrbk_busy_check(wdma) | wdma_tx_wrbk_busy_check(wdma);
		break;
	default:
		warp_dbg(WARP_DBG_INF, "%s(): Unknown DMA control (%d).\n",
			 __func__, txrx);
		break;
	}
	return ret;
}

int
wdma_dma_ctrl(struct wdma_entry *wdma, u8 txrx)
{
	_wdma_pref_ctrl(wdma, txrx);
	_wdma_dma_ctrl(wdma, txrx);
	_wdma_wrbk_ctrl(wdma, txrx);

	return 0;
}

int
wdma_int_ctrl(struct wdma_entry *wdma, u8 enable)
{
	u32 value = 0;

	if (enable) {
		value |= WDMA_INT_MASK_RX_DONE_INT0_MASK;
		value |= WDMA_INT_MASK_TX_DONE_INT0_MASK;
	}

	warp_io_write32(wdma, WDMA_INT_MASK_ADDR, value);
	warp_io_write32(wdma, WDMA_INT_GRP2_ADDR, value);

	return 0;
}

static void
reset_wdma_rx_fifo(struct wdma_entry *wdma)
{
	u32 value;

	/* Prefetch FIFO */
	value = (WDMA_PREF_RX_FIFO_CFG0_RING0_CLEAR_MASK |
		 WDMA_PREF_RX_FIFO_CFG0_RING1_CLEAR_MASK);
	warp_io_write32(wdma, WDMA_PREF_RX_FIFO_CFG0_ADDR, value);

	warp_io_read32(wdma, WDMA_PREF_RX_FIFO_CFG0_ADDR, &value);
	value &= ~(WDMA_PREF_RX_FIFO_CFG0_RING0_CLEAR_MASK |
		   WDMA_PREF_RX_FIFO_CFG0_RING1_CLEAR_MASK);
	warp_io_write32(wdma, WDMA_PREF_RX_FIFO_CFG0_ADDR, value);

	/* Core FIFO */
	value = (WDMA_RX_XDMA_FIFO_CFG0_RX_PAR_FIFO_CLEAR_MASK |
		 WDMA_RX_XDMA_FIFO_CFG0_RX_CMD_FIFO_CLEAR_MASK |
		 WDMA_RX_XDMA_FIFO_CFG0_RX_DMAD_FIFO_CLEAR_MASK |
		 WDMA_RX_XDMA_FIFO_CFG0_RX_ARR_FIFO_CLEAR_MASK |
		 WDMA_RX_XDMA_FIFO_CFG0_RX_LEN_FIFO_CLEAR_MASK |
		 WDMA_RX_XDMA_FIFO_CFG0_RX_WID_FIFO_CLEAR_MASK |
		 WDMA_RX_XDMA_FIFO_CFG0_RX_BID_FIFO_CLEAR_MASK);
	warp_io_write32(wdma, WDMA_RX_XDMA_FIFO_CFG0_ADDR, value);

	warp_io_read32(wdma, WDMA_RX_XDMA_FIFO_CFG0_ADDR, &value);
	value &= ~(WDMA_RX_XDMA_FIFO_CFG0_RX_PAR_FIFO_CLEAR_MASK |
		   WDMA_RX_XDMA_FIFO_CFG0_RX_CMD_FIFO_CLEAR_MASK |
		   WDMA_RX_XDMA_FIFO_CFG0_RX_DMAD_FIFO_CLEAR_MASK |
		   WDMA_RX_XDMA_FIFO_CFG0_RX_ARR_FIFO_CLEAR_MASK |
		   WDMA_RX_XDMA_FIFO_CFG0_RX_LEN_FIFO_CLEAR_MASK |
		   WDMA_RX_XDMA_FIFO_CFG0_RX_WID_FIFO_CLEAR_MASK |
		   WDMA_RX_XDMA_FIFO_CFG0_RX_BID_FIFO_CLEAR_MASK);
	warp_io_write32(wdma, WDMA_RX_XDMA_FIFO_CFG0_ADDR, value);

	/* Writeback FIFO */
	value = WDMA_WRBK_RX_FIFO_CFG0_RING0_CLEAR_MASK;
	warp_io_write32(wdma, WDMA_WRBK_RX_FIFO_CFG0_ADDR, value);
	value = WDMA_WRBK_RX_FIFO_CFG1_RING1_CLEAR_MASK;
	warp_io_write32(wdma, WDMA_WRBK_RX_FIFO_CFG1_ADDR, value);

	warp_io_read32(wdma, WDMA_WRBK_RX_FIFO_CFG0_ADDR, &value);
	value &= ~WDMA_WRBK_RX_FIFO_CFG0_RING0_CLEAR_MASK;
	warp_io_write32(wdma, WDMA_WRBK_RX_FIFO_CFG0_ADDR, value);
	warp_io_read32(wdma, WDMA_WRBK_RX_FIFO_CFG1_ADDR, &value);
	value &= ~WDMA_WRBK_RX_FIFO_CFG1_RING1_CLEAR_MASK;
	warp_io_write32(wdma, WDMA_WRBK_RX_FIFO_CFG1_ADDR, value);
}

static void
reset_wdma_rx_idx(struct wdma_entry *wdma)
{
	u32 value;

	/* Ring status */
	value = (WDMA_RST_IDX_RST_DRX_IDX0_MASK |
		 WDMA_RST_IDX_RST_DRX_IDX1_MASK);
	warp_io_write32(wdma, WDMA_RST_IDX_ADDR, value);

	/* Prefetch ring status */
	value = (WDMA_PREF_SIDX_CFG_RX_RING0_SIDX_CLR_MASK |
		 WDMA_PREF_SIDX_CFG_RX_RING1_SIDX_CLR_MASK);
	warp_io_write32(wdma, WDMA_PREF_SIDX_CFG_ADDR, value);

	warp_io_read32(wdma, WDMA_PREF_SIDX_CFG_ADDR, &value);
	value &= ~(WDMA_PREF_SIDX_CFG_RX_RING0_SIDX_CLR_MASK |
		   WDMA_PREF_SIDX_CFG_RX_RING1_SIDX_CLR_MASK);
	warp_io_write32(wdma, WDMA_PREF_SIDX_CFG_ADDR, value);

	/* Writeback ring status */
	value = (WDMA_WRBK_SIDX_CFG_RX_RING0_SIDX_CLR_MASK |
		 WDMA_WRBK_SIDX_CFG_RX_RING1_SIDX_CLR_MASK);
	warp_io_write32(wdma, WDMA_WRBK_SIDX_CFG_ADDR, value);

	warp_io_read32(wdma, WDMA_WRBK_SIDX_CFG_ADDR, &value);
	value &= ~(WDMA_WRBK_SIDX_CFG_RX_RING0_SIDX_CLR_MASK |
		   WDMA_WRBK_SIDX_CFG_RX_RING1_SIDX_CLR_MASK);
	warp_io_write32(wdma, WDMA_WRBK_SIDX_CFG_ADDR, value);
}

int
reset_wdma_rx(struct wdma_entry *wdma)
{
	struct wdma_res_ctrl *res = &wdma->res_ctrl;
	struct wdma_rx_ctrl *rx_ctrl = &res->rx_ctrl;
	int ret = 0;

	/* Check if WDMA has been reset */
	if (wdma_rx_ring_reset_state(wdma, &rx_ctrl->rx_ring_ctrl) &&
	    !wdma_rx_glo_busy_check(wdma))
		return 0;

	if (wdma_rx_ring_poll_to_idle(wdma, &rx_ctrl->rx_ring_ctrl)) {
		warp_dbg(WARP_DBG_ERR, "%s(): polling wdma rx ring timeout\n", __func__);
		ret = -EIO;
	}

	if (_wdma_pref_ctrl(wdma, WARP_DMA_TX_DISABLE))
		ret = -EIO;

	if (_wdma_dma_ctrl(wdma, WARP_DMA_TX_DISABLE))
		ret = -EIO;

	if (_wdma_wrbk_ctrl(wdma, WARP_DMA_TX_DISABLE))
		ret = -EIO;

	reset_wdma_rx_fifo(wdma);
	reset_wdma_rx_idx(wdma);

	return ret;
}

static void
reset_wdma_tx_fifo(struct wdma_entry *wdma)
{
	u32 value;

	/* Prefetch FIFO */
	value = (WDMA_PREF_TX_FIFO_CFG0_RING0_CLEAR_MASK |
		 WDMA_PREF_TX_FIFO_CFG0_RING1_CLEAR_MASK);
	warp_io_write32(wdma, WDMA_PREF_TX_FIFO_CFG0_ADDR, value);

	warp_io_read32(wdma, WDMA_PREF_TX_FIFO_CFG0_ADDR, &value);
	value &= ~(WDMA_PREF_TX_FIFO_CFG0_RING0_CLEAR_MASK |
		   WDMA_PREF_TX_FIFO_CFG0_RING1_CLEAR_MASK);
	warp_io_write32(wdma, WDMA_PREF_TX_FIFO_CFG0_ADDR, value);

	/* Core FIFO */
	value = (WDMA_TX_XDMA_FIFO_CFG0_TX_PAR_FIFO_CLEAR_MASK |
		 WDMA_TX_XDMA_FIFO_CFG0_TX_CMD_FIFO_CLEAR_MASK |
		 WDMA_TX_XDMA_FIFO_CFG0_TX_DMAD_FIFO_CLEAR_MASK |
		 WDMA_TX_XDMA_FIFO_CFG0_TX_ARR_FIFO_CLEAR_MASK);
	warp_io_write32(wdma, WDMA_TX_XDMA_FIFO_CFG0_ADDR, value);

	warp_io_read32(wdma, WDMA_TX_XDMA_FIFO_CFG0_ADDR, &value);
	value &= ~(WDMA_TX_XDMA_FIFO_CFG0_TX_PAR_FIFO_CLEAR_MASK |
		   WDMA_TX_XDMA_FIFO_CFG0_TX_CMD_FIFO_CLEAR_MASK |
		   WDMA_TX_XDMA_FIFO_CFG0_TX_DMAD_FIFO_CLEAR_MASK |
		   WDMA_TX_XDMA_FIFO_CFG0_TX_ARR_FIFO_CLEAR_MASK);
	warp_io_write32(wdma, WDMA_TX_XDMA_FIFO_CFG0_ADDR, value);

	/* Writeback FIFO */
	value = WDMA_WRBK_TX_FIFO_CFG0_RING0_CLEAR_MASK;
	warp_io_write32(wdma, WDMA_WRBK_TX_FIFO_CFG0_ADDR, value);
	value = WDMA_WRBK_TX_FIFO_CFG1_RING1_CLEAR_MASK;
	warp_io_write32(wdma, WDMA_WRBK_TX_FIFO_CFG1_ADDR, value);

	warp_io_read32(wdma, WDMA_WRBK_TX_FIFO_CFG0_ADDR, &value);
	value &= ~WDMA_WRBK_TX_FIFO_CFG0_RING0_CLEAR_MASK;
	warp_io_write32(wdma, WDMA_WRBK_TX_FIFO_CFG0_ADDR, value);
	warp_io_read32(wdma, WDMA_WRBK_TX_FIFO_CFG1_ADDR, &value);
	value &= ~WDMA_WRBK_TX_FIFO_CFG1_RING1_CLEAR_MASK;
	warp_io_write32(wdma, WDMA_WRBK_TX_FIFO_CFG1_ADDR, value);
}

static void
reset_wdma_tx_idx(struct wdma_entry *wdma)
{
	u32 value;

	/* Ring status */
	value = (WDMA_RST_IDX_RST_DTX_IDX0_MASK |
		 WDMA_RST_IDX_RST_DTX_IDX1_MASK);
	warp_io_write32(wdma, WDMA_RST_IDX_ADDR, value);

	/* Prefetch ring status */
	value = (WDMA_PREF_SIDX_CFG_TX_RING0_SIDX_CLR_MASK |
		 WDMA_PREF_SIDX_CFG_TX_RING1_SIDX_CLR_MASK);
	warp_io_write32(wdma, WDMA_PREF_SIDX_CFG_ADDR, value);

	warp_io_read32(wdma, WDMA_PREF_SIDX_CFG_ADDR, &value);
	value &= ~(WDMA_PREF_SIDX_CFG_TX_RING0_SIDX_CLR_MASK |
		   WDMA_PREF_SIDX_CFG_TX_RING1_SIDX_CLR_MASK);
	warp_io_write32(wdma, WDMA_PREF_SIDX_CFG_ADDR, value);

	/* Writeback ring status */
	value = (WDMA_WRBK_SIDX_CFG_TX_RING0_SIDX_CLR_MASK |
		 WDMA_WRBK_SIDX_CFG_TX_RING1_SIDX_CLR_MASK);
	warp_io_write32(wdma, WDMA_WRBK_SIDX_CFG_ADDR, value);

	warp_io_read32(wdma, WDMA_WRBK_SIDX_CFG_ADDR, &value);
	value &= ~(WDMA_WRBK_SIDX_CFG_TX_RING0_SIDX_CLR_MASK |
		   WDMA_WRBK_SIDX_CFG_TX_RING1_SIDX_CLR_MASK);
	warp_io_write32(wdma, WDMA_WRBK_SIDX_CFG_ADDR, value);
}

int
reset_wdma_tx(struct wdma_entry *wdma)
{
	int ret = 0;
	struct wdma_res_ctrl *res = &wdma->res_ctrl;
	struct wdma_tx_ctrl *tx_ctrl = &res->tx_ctrl;

	/* Check if WDMA has been reset */
	if (wdma_tx_ring_reset_state(wdma, &tx_ctrl->tx_ring_ctrl) &&
	    !wdma_tx_glo_busy_check(wdma))
		return 0;

	/*check WDMA TX status*/
	if (wdma_tx_ring_poll_to_idle(wdma, &tx_ctrl->tx_ring_ctrl)) {
		warp_dbg(WARP_DBG_ERR, "%s(): polling wdma tx ring timeout\n", __func__);
		ret = -EIO;
	}

	if (_wdma_pref_ctrl(wdma, WARP_DMA_RX_DISABLE))
		ret = -EIO;

	if (_wdma_dma_ctrl(wdma, WARP_DMA_RX_DISABLE))
		ret = -EIO;

	if (_wdma_wrbk_ctrl(wdma, WARP_DMA_RX_DISABLE))
		ret = -EIO;

	reset_wdma_tx_fifo(wdma);
	reset_wdma_tx_idx(wdma);

	return ret;
}

int
stop_wdma_tx(struct wdma_entry *wdma)
{
	struct wdma_res_ctrl *res = &wdma->res_ctrl;
	struct wdma_tx_ctrl *tx_ctrl = &res->tx_ctrl;
	struct wdma_rx_ctrl *rx_ctrl = &res->rx_ctrl;
	int ret;

	/* Check WDMA status */
	ret = wdma_tx_ring_poll_to_idle(wdma, &tx_ctrl->tx_ring_ctrl);
	if (ret) {
		warp_dbg(WARP_DBG_ERR, "%s(): Polling wdma tx ring timeout\n", __func__);
		goto err;
	}

	ret = wdma_tx_glo_busy_check(wdma);
	if (ret) {
		warp_dbg(WARP_DBG_ERR, "%s(): Polling wdma tx idle timeout\n", __func__);
		goto err;
	}

	ret = wdma_rx_ring_poll_to_idle(wdma, &rx_ctrl->rx_ring_ctrl);
	if (ret) {
		warp_dbg(WARP_DBG_ERR, "%s(): Polling wdma rx ring timeout\n", __func__);
		goto err;
	}

	ret = wdma_rx_glo_busy_check(wdma);
	if (ret) {
		warp_dbg(WARP_DBG_ERR, "%s(): Polling wdma rx idle timeout\n", __func__);
		goto err;
	}

	/* Disable WDMA */
	_wdma_pref_ctrl(wdma, WARP_DMA_RX_DISABLE);
	_wdma_dma_ctrl(wdma, WARP_DMA_RX_DISABLE);
	_wdma_wrbk_ctrl(wdma, WARP_DMA_RX_DISABLE);

	return 0;
err:
	return ret;
}

int
restore_wdma_tx(struct wdma_entry *wdma)
{
	_wdma_pref_ctrl(wdma, WARP_DMA_RX);
	_wdma_dma_ctrl(wdma, WARP_DMA_RX);
	_wdma_wrbk_ctrl(wdma, WARP_DMA_RX);

	return 0;
}

void
warp_wdma_rx_ring_get_hw(struct warp_ring *ring, u8 idx)
{
	u32 offset = idx * (WDMA_RX_BASE_PTR_1_ADDR - WDMA_RX_BASE_PTR_0_ADDR);

	ring->hw_desc_base = WDMA_RX_BASE_PTR_0_ADDR + offset;
	ring->hw_cnt_addr  = WDMA_RX_MAX_CNT_0_ADDR + offset;
	ring->hw_cidx_addr = WDMA_RX_CRX_IDX_0_ADDR + offset;
	ring->hw_didx_addr = WDMA_RX_DRX_IDX_0_ADDR + offset;
}

void
warp_wdma_tx_ring_get_hw(struct warp_ring *ring, u8 idx)
{
	u32 offset = idx * (WDMA_TX_BASE_PTR_1_ADDR - WDMA_TX_BASE_PTR_0_ADDR);

	ring->hw_desc_base = WDMA_TX_BASE_PTR_0_ADDR + offset;
	ring->hw_cnt_addr  = WDMA_TX_MAX_CNT_0_ADDR + offset;
	ring->hw_cidx_addr = WDMA_TX_CTX_IDX_0_ADDR + offset;
	ring->hw_didx_addr = WDMA_TX_DTX_IDX_0_ADDR + offset;
}

void
dump_wdma_cfg_hw(struct wdma_entry *wdma, struct seq_file *seq)
{
	dump_string(seq,  "==========WDMA basic info:==========\n");
	dump_io(seq, wdma, "WDMA_GLO_CFG0", WDMA_GLO_CFG0_ADDR);
	dump_io(seq, wdma, "WDMA_INT_MSK", WDMA_INT_MASK_ADDR);
	dump_io(seq, wdma, "WDMA_INT_STA", WDMA_INT_STATUS_ADDR);
	dump_io(seq, wdma, "WDMA_INFO", WDMA_INFO_ADDR);
	dump_io(seq, wdma, "WDMA_FREEQ_THRES", WDMA_FREEQ_THRES_ADDR);
	dump_io(seq, wdma, "WDMA_INT_STS_GRP0", WDMA_INT_STS_GRP0_ADDR);
	dump_io(seq, wdma, "WDMA_INT_STS_GRP1", WDMA_INT_STS_GRP1_ADDR);
	dump_io(seq, wdma, "WDMA_INT_STS_GRP2", WDMA_INT_STS_GRP2_ADDR);
	dump_io(seq, wdma, "WDMA_INT_GRP1", WDMA_INT_GRP1_ADDR);
	dump_io(seq, wdma, "WDMA_INT_GRP2", WDMA_INT_GRP2_ADDR);
	dump_io(seq, wdma, "WDMA_SCH_Q01_CFG", WDMA_SCH_Q01_CFG_ADDR);
	dump_io(seq, wdma, "WDMA_SCH_Q23_CFG", WDMA_SCH_Q23_CFG_ADDR);
}

void
dump_wdma_rx_ring_hw(struct wdma_entry *wdma, struct seq_file *output)
{
	int i;
	u32 ctrl0, ctrl1, ctrl2, ctrl3;
	u32 tcnt, cidx, didx, qcnt;
	dma_addr_t base;

	dump_string(output, "==========WDMA RX ring info===========\n");
	for (i = 0; i < 1; i++) {
		u32 ring_offset = WDMA_RX_BASE_PTR_1_ADDR - WDMA_RX_BASE_PTR_0_ADDR;

		warp_io_read32(wdma, WDMA_RX_BASE_PTR_0_ADDR + i * ring_offset, &ctrl0);
		warp_io_read32(wdma, WDMA_RX_MAX_CNT_0_ADDR + i * ring_offset, &ctrl1);
		warp_io_read32(wdma, WDMA_RX_CRX_IDX_0_ADDR + i * ring_offset, &ctrl2);
		warp_io_read32(wdma, WDMA_RX_DRX_IDX_0_ADDR + i * ring_offset, &ctrl3);

		base = ctrl0;
		tcnt = GET_FIELD(WDMA_RX_MAX_CNT_0_RX_MAX_CNT, ctrl1);
		cidx = GET_FIELD(WDMA_RX_CRX_IDX_0_RX_CRX_IDX, ctrl2);
		didx = GET_FIELD(WDMA_RX_DRX_IDX_0_RX_DRX_IDX, ctrl3);
		if (base)
			qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
		else
			qcnt = 0;

		seq_printf(output, "WMDA_RX%d_BASE\t: %pad\n", i, &base);
		seq_printf(output, "WMDA_RX%d_MAX\t: 0x%x\n", i, tcnt);
		seq_printf(output, "WMDA_RX%d_CIDX\t: 0x%x\n", i, cidx);
		seq_printf(output, "WMDA_RX%d_DIDX\t: 0x%x\n", i, didx);
		seq_printf(output, "WMDA_RX%d Qcnt\t: 0x%x\n", i, qcnt);
	}
}

void
dump_wdma_tx_ring_hw(struct wdma_entry *wdma, struct seq_file *output)
{
	int i;
	u32 ctrl0, ctrl1, ctrl2, ctrl3;
	u32 tcnt, cidx, didx, qcnt;
	dma_addr_t base;

	dump_string(output, "==========WDMA TX ring info===========\n");
	for (i = 0; i < 1; i++) {
		warp_io_read32(wdma, WDMA_TX_BASE_PTR_0_ADDR, &ctrl0);
		warp_io_read32(wdma, WDMA_TX_MAX_CNT_0_ADDR, &ctrl1);
		warp_io_read32(wdma, WDMA_TX_DTX_IDX_0_ADDR, &ctrl3);
		warp_io_read32(wdma, WDMA_TX_CTX_IDX_0_ADDR, &ctrl2);

		base = ctrl0;
		tcnt = GET_FIELD(WDMA_TX_MAX_CNT_0_TX_MAX_CNT, ctrl1);
		cidx = GET_FIELD(WDMA_TX_CTX_IDX_0_TX_CTX_IDX, ctrl2);
		didx = GET_FIELD(WDMA_TX_DTX_IDX_0_TX_DTX_IDX, ctrl3);
		if (base)
			qcnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + tcnt);
		else
			qcnt = 0;

		seq_printf(output, "WDMA_TX%d_BASE\t: %pad\n", i, &base);
		seq_printf(output, "WDMA_TX%d_MAX\t: 0x%x\n", i, tcnt);
		seq_printf(output, "WDMA_TX%d_CIDX\t: 0x%x\n", i, cidx);
		seq_printf(output, "WDMA_TX%d_DIDX\t: 0x%x\n", i, didx);
		seq_printf(output, "WDMA_TX%d Qcnt\t: 0x%x\n", i, qcnt);
	}
}

