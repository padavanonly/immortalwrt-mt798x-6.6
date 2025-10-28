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

#include <net/mtk/mtk_fe_dma.h>
#include "wdma_netsys.h"

static u8 hwid_list[] = { WED0_ID, WED1_ID };

int
wdma_rx_ring_rdy_ck(struct wdma_entry *wdma,
	struct wdma_rx_ring_ctrl *ring_ctrl)
{
	struct warp_entry *warp = wdma->warp;
	struct warp_ring *ring;
	int i, j;
	u32 value;

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		j = 0;

		while (j < 3) {
			mdelay(CHK_DELAY_MS);
			ring = &ring_ctrl->ring[i];
			value = get_rx_cidx(hwid_list[warp->idx], i);

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

int
wdma_tx_ring_init_hw(struct wdma_entry *wdma,
	struct wdma_tx_ring_ctrl *tx_ring_ctrl, int idx)
{
	struct warp_entry *warp = wdma->warp;
	u8 hw_id = hwid_list[warp->idx];

	if (idx == 0)
		dma_tx_reset(hw_id);

	dma_tx_init(hw_id);
	return 0;
}

int
wdma_rx_ring_init_hw(struct wdma_entry *wdma,
	struct wdma_rx_ring_ctrl *rx_ring_ctrl, int idx)
{
	struct warp_entry *warp = wdma->warp;
	u8 hw_id = hwid_list[warp->idx];

	if (idx == 0)
		dma_rx_reset(hw_id);

	dma_rx_init(hw_id, idx);
	return 0;
}

int
wdma_dma_ctrl(struct wdma_entry *wdma, u8 txrx)
{
	struct warp_entry *warp = wdma->warp;
	u8 hw_id = hwid_list[warp->idx];
	u32 ret = 0;

	switch (txrx) {
	case WARP_DMA_TX:
		dma_rx_enable(hw_id);
		break;
	case WARP_DMA_RX:
		dma_tx_enable(hw_id);
		break;
	case WARP_DMA_TXRX:
		dma_rx_enable(hw_id);
		dma_tx_enable(hw_id);
		break;
	case WARP_DMA_TX_DISABLE:
		dma_rx_disable(hw_id);
		break;
	case WARP_DMA_RX_DISABLE:
		dma_tx_disable(hw_id);
		break;
	case WARP_DMA_DISABLE:
		dma_rx_disable(hw_id);
		dma_tx_disable(hw_id);
		break;
	default:
		warp_dbg(WARP_DBG_INF, "%s(): Unknown DMA control (%d).\n",
			 __func__, txrx);
		break;
	}

	return ret;
}

static inline void
wdma_tx_int_ctrl(struct wdma_entry *wdma, u8 enable, u8 hw_id)
{
	if (enable)
		dma_tx_interrupt(hw_id);
	else
		set_tx_irq_mask(hw_id, 0);
}

static inline void
wdma_rx_int_ctrl(struct wdma_entry *wdma, u8 enable, u8 hw_id)
{
	struct wdma_rx_ring_ctrl *rx_ring_ctrl = &wdma->res_ctrl.rx_ctrl.rx_ring_ctrl;
	int i;

	if (enable) {
		for (i = 0; i < rx_ring_ctrl->ring_num; i++)
			dma_rx_interrupt(hw_id, i);
	} else {
		set_tx_irq_mask(hw_id, 0);
	}
}

int
wdma_int_ctrl(struct wdma_entry *wdma, u8 enable)
{
	struct warp_entry *warp = wdma->warp;
	u8 hw_id = hwid_list[warp->idx];

	wdma_tx_int_ctrl(wdma, enable, hw_id);
	wdma_rx_int_ctrl(wdma, enable, hw_id);
	return 0;
}

int
reset_wdma_tx(struct wdma_entry *wdma)
{
	struct warp_entry *warp = wdma->warp;
	u8 hw_id = hwid_list[warp->idx];

	dma_tx_disable(hw_id);
	dma_tx_reset(hw_id);
	return 0;
}

int
reset_wdma_rx(struct wdma_entry *wdma)
{
	struct warp_entry *warp = wdma->warp;
	u8 hw_id = hwid_list[warp->idx];

	dma_rx_disable(hw_id);
	dma_rx_reset(hw_id);
	return 0;
}

int
stop_wdma_tx(struct wdma_entry *wdma)
{
	wdma_dma_ctrl(wdma, WARP_DMA_RX_DISABLE);

	return 0;
}

int
restore_wdma_tx(struct wdma_entry *wdma)
{
	wdma_dma_ctrl(wdma, WARP_DMA_RX);

	return 0;
}

static inline void
wdma_tx_ring_get_hw(struct warp_ring *ring, u8 ring_idx)
{
	ring->hw_desc_base = dma_tx_offset();
	ring->hw_cnt_addr  = ring->hw_desc_base + 0x4;
	ring->hw_cidx_addr = ring->hw_desc_base + 0x8;
	ring->hw_didx_addr = ring->hw_desc_base + 0xC;
}

static inline void
wdma_rx_ring_get_hw(struct warp_ring *ring, u8 ring_idx)
{
	ring->hw_desc_base = dma_rx_offset(ring_idx);
	ring->hw_cnt_addr  = ring->hw_desc_base + 0x4;
	ring->hw_cidx_addr = ring->hw_desc_base + 0x8;
	ring->hw_didx_addr = ring->hw_desc_base + 0xC;
}

static inline int
wdma_tx_ring_get_int_mask(struct wdma_tx_ring_ctrl *ring_ctrl,
				u8 hw_id, u8 ring_idx)
{
	u32 tx_mask = dma_tx_int_mask_status();

	if (!tx_mask)
		return -EINVAL;

	ring_ctrl->int_sts_mask |= tx_mask;
	return 0;
}

static inline int
wdma_rx_ring_get_int_mask(struct wdma_rx_ring_ctrl *ring_ctrl,
				u8 hw_id, u8 ring_idx)
{
	u32 rx_mask = dma_rx_int_mask_status(ring_idx);

	if (!rx_mask)
		return -EINVAL;

	ring_ctrl->int_sts_mask |= rx_mask;
	return 0;
}

static int
tx_ring_init(struct wdma_entry *entry, struct wdma_tx_ring_ctrl *ring_ctrl,
		u8 hw_id, u8 ring_idx)
{
	struct warp_ring *ring = &ring_ctrl->ring[ring_idx];
	struct warp_dma_cb *cell = &ring->cell[0];

	wdma_tx_ring_get_hw(ring, ring_idx);

	cell->alloc_pa = get_tx_ring_phys(hw_id);
	if (!cell->alloc_pa) {
		warp_dbg(WARP_DBG_ERR, "%s(): Get ring_idx = %u DMAD address failed\n",
			__func__, ring_idx);
		return -ENOMEM;
	}

	return 0;
}

static int
rx_ring_init(struct wdma_entry *entry, struct wdma_rx_ring_ctrl *ring_ctrl,
		u8 hw_id, u8 ring_idx)
{
	struct warp_ring *ring = &ring_ctrl->ring[ring_idx];
	struct warp_dma_cb *cell = &ring->cell[0];

	wdma_rx_ring_get_hw(ring, ring_idx);

	cell->alloc_pa = get_rx_ring_phys(hw_id, ring_idx);
	if (!cell->alloc_pa) {
		warp_dbg(WARP_DBG_ERR, "%s(): Get ring_idx = %u DMAD address failed\n",
			__func__, ring_idx);
		return -ENOMEM;
	}

	return 0;
}

static void
wdma_tx_ring_exit(struct wdma_entry *wdma, struct wdma_tx_ring_ctrl *ring_ctrl)
{
	if (ring_ctrl->ring)
		warp_os_free_mem(ring_ctrl->ring);

	memset(ring_ctrl, 0, sizeof(*ring_ctrl));
}

static void
wdma_rx_ring_exit(struct wdma_entry *wdma, struct wdma_rx_ring_ctrl *ring_ctrl)
{
	if (ring_ctrl->ring)
		warp_os_free_mem(ring_ctrl->ring);

	memset(ring_ctrl, 0, sizeof(*ring_ctrl));
}

static int
wdma_tx_ring_init(struct wdma_entry *wdma, struct wdma_tx_ring_ctrl *ring_ctrl, u8 hw_id)
{
	unsigned int len;
	int i, ret;

	len = sizeof(struct warp_ring) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->ring, len, GFP_ATOMIC);
	if (!ring_ctrl->ring) {
		ret = -ENOMEM;
		warp_dbg(WARP_DBG_ERR, "%s(): Alloc ring's memory failed\n", __func__);
		goto err;
	}

	memset(ring_ctrl->ring, 0, len);

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ret = tx_ring_init(wdma, ring_ctrl, hw_id, i);
		if (ret)
			goto err;

		ret = wdma_tx_ring_get_int_mask(ring_ctrl, hw_id, i);
		if (ret)
			goto err;
	}

	return 0;
err:
	wdma_tx_ring_exit(wdma, ring_ctrl);
	return ret;
}

static int
wdma_rx_ring_init(struct wdma_entry *wdma, struct wdma_rx_ring_ctrl *ring_ctrl, u8 hw_id)
{
	unsigned int len;
	int i, ret;

	len = sizeof(struct warp_ring) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->ring, len, GFP_ATOMIC);
	if (!ring_ctrl->ring) {
		ret = -ENOMEM;
		warp_dbg(WARP_DBG_ERR, "%s(): Alloc ring's memory failed\n", __func__);
		goto err;
	}

	memset(ring_ctrl->ring, 0, len);

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ret = rx_ring_init(wdma, ring_ctrl, hw_id, i);
		if (ret)
			goto err;

		ret = wdma_rx_ring_get_int_mask(ring_ctrl, hw_id, i);
		if (ret)
			goto err;
	}

	return 0;
err:
	wdma_rx_ring_exit(wdma, ring_ctrl);
	return ret;
}

int
wdma_tx_ring_reset(struct wdma_entry *entry)
{
	return 0;
}

void
wdma_ring_exit(struct wdma_entry *wdma)
{
	struct wdma_res_ctrl *res = &wdma->res_ctrl;
	struct wdma_tx_ctrl *tx_ctrl = &res->tx_ctrl;
	struct wdma_rx_ctrl *rx_ctrl = &res->rx_ctrl;

	wdma_tx_ring_exit(wdma, &tx_ctrl->tx_ring_ctrl);
	wdma_rx_ring_exit(wdma, &rx_ctrl->rx_ring_ctrl);
}

static int
wdma_ring_init(struct wdma_entry *wdma, u8 hw_id)
{
	struct wdma_res_ctrl *res = &wdma->res_ctrl;
	struct wdma_tx_ctrl *tx_ctrl = &res->tx_ctrl;
	struct wdma_rx_ctrl *rx_ctrl = &res->rx_ctrl;
	u8 ring_idx = 0;
	int ret;

	/* Get DMA_BASE/GLO_CFG/INT_STS info */
	rx_ctrl->base_phy_addr = get_rx_base_address(hw_id);
	tx_ctrl->base_phy_addr = get_tx_base_address(hw_id);
	rx_ctrl->glo_cfg_offset = dma_glo_offset();
	tx_ctrl->glo_cfg_offset = dma_glo_offset();
	rx_ctrl->int_sts_offset = dma_int_offset();
	tx_ctrl->int_sts_offset = dma_int_offset();

	/* Initial default value */
	rx_ctrl->rx_ring_ctrl.rxd_len = sizeof(struct WDMA_RXD) * 2;
	tx_ctrl->tx_ring_ctrl.txd_len = sizeof(struct WDMA_TXD) * 2;
	rx_ctrl->rx_ring_ctrl.ring_len = dma_rx_max_cnt(hw_id, ring_idx);
	tx_ctrl->tx_ring_ctrl.ring_len = dma_tx_max_cnt(hw_id);
	rx_ctrl->rx_ring_ctrl.ring_num = WDMA_RX_RING_NUM;
	tx_ctrl->tx_ring_ctrl.ring_num = WDMA_TX_RING_NUM;

	ret = wdma_rx_ring_init(wdma, &rx_ctrl->rx_ring_ctrl, hw_id);
	if (ret) {
		warp_dbg(WARP_DBG_ERR, "%s(): Init rx_ring failed\n", __func__);
		goto err_wdma_rx;
	}

	ret = wdma_tx_ring_init(wdma, &tx_ctrl->tx_ring_ctrl, hw_id);
	if (ret) {
		warp_dbg(WARP_DBG_ERR, "%s(): Init tx_ring failed\n", __func__);
		goto err_wdma_tx;
	}

	return 0;
err_wdma_tx:
	wdma_rx_ring_exit(wdma, &rx_ctrl->rx_ring_ctrl);
err_wdma_rx:
	return ret;
}

int
wdma_exit(struct platform_device *pdev, struct wdma_entry *wdma)
{
	wdma_ring_exit(wdma);
	return 0;
}

int
wdma_init(struct platform_device *pdev, u8 idx, struct wdma_entry *wdma, u8 ver)
{
	int ret;

	wdma->pdev = pdev;
	wdma->ver = ver;

	if (idx >= ARRAY_SIZE(hwid_list)) {
		warp_dbg(WARP_DBG_ERR, "%s(): idx = %u larger than array length\n",
			__func__, idx);
		goto err;
	}

	ret = warp_get_wdma_port(wdma, idx);
	if (ret)
		goto err;

	ret = wdma_ring_init(wdma, hwid_list[idx]);
	if (ret)
		goto err;

	return 0;

err:
	warp_dbg(WARP_DBG_ERR, "%s() wdma%u init failed\n",
		__func__, idx);
	return ret;
}

void
wdma_proc_handle(struct wdma_entry *wdma, char choice, char *arg)
{
	warp_dbg(WARP_DBG_ERR, "%s(): Unsupported features\n", __func__);
}

void
dump_wdma_cfg_hw(struct wdma_entry *wdma, struct seq_file *seq)
{
}

void
dump_wdma_rx_ring_hw(struct wdma_entry *wdma, struct seq_file *output)
{
	struct warp_entry *warp = wdma->warp;
	u8 hw_id = hwid_list[warp->idx];
	u32 tcnt, cidx, didx, qcnt;
	dma_addr_t base;
	int i;

	dump_string(output, "==========WDMA RX ring info===========\n");
	for (i = 0; i < 1; i++) {
		base = get_rx_ring_phys(hw_id, i);
		tcnt = dma_rx_max_cnt(hw_id, i);
		cidx = get_rx_cidx(hw_id, i);
		didx = get_rx_didx(hw_id, i);
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
	struct warp_entry *warp = wdma->warp;
	u8 hw_id = hwid_list[warp->idx];
	u32 tcnt, cidx, didx, qcnt;
	dma_addr_t base;
	int i;

	dump_string(output, "==========WDMA TX ring info===========\n");
	for (i = 0; i < 1; i++) {
		base = get_tx_ring_phys(hw_id);
		tcnt = dma_tx_max_cnt(hw_id);
		didx = get_tx_didx(hw_id);
		cidx = get_tx_cidx(hw_id);
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

