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

	Module Name: wifi_offload
	warp_hw.c
*/
#include <warp.h>
#include <warp_hw.h>
#include <warp_utility.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include <warp_bm.h>
#include "wed_hw.h"
#include "wdma_hw.h"

/*Local function*/
static inline int
wed_busy_chk(struct wed_entry *wed, u32 addr, u32 busy_mask)
{
	u32 cnt = 0;
	u32 value;

	warp_io_read32(wed, addr, &value);

	while ((value & busy_mask) && cnt < WED_POLL_MAX) {
		udelay(CHK_DELAY_US);
		warp_io_read32(wed, addr, &value);
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
wed_rdy_chk(struct wed_entry *wed, u32 addr, u32 rdy_mask)
{
	u32 cnt = 0;
	u32 value;

	warp_io_read32(wed, addr, &value);

	while (((value & rdy_mask) != rdy_mask) && cnt < WED_POLL_MAX) {
		udelay(CHK_DELAY_US);
		warp_io_read32(wed, addr, &value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_WAR, "%s(): addr 0x%x ready mask 0x%x fail!!\n",
			__func__, addr, rdy_mask);
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

static int
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

static int
wdma_all_rx_ring_rdy_ck(struct wed_entry *wed,
			struct wdma_entry *wdma)
{
	struct wdma_res_ctrl *res = &wdma->res_ctrl;
	struct wdma_rx_ctrl *rx_ctrl = &res->rx_ctrl;

	if (wdma_rx_ring_rdy_ck(wdma, &rx_ctrl->rx_ring_ctrl)) {
		warp_dbg(WARP_DBG_ERR, "%s(): wdma rx data init fail\n",
			 __func__);
		return -1;
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
wifi_rx_ring_rdy_ck(struct wed_rx_ring_ctrl *ring_ctrl,
			struct wifi_entry *wifi)
{
	int i, j;
	u32 value;
	struct warp_rx_ring *ring;

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		j = 0;

		while (j < 3) {
			mdelay(CHK_DELAY_MS);
			ring = &ring_ctrl->ring[i];
			warp_io_read32(wifi, ring->hw_cidx_addr, &value);

			if (value == (ring->ring_lens - 1))
				break;

			j++;
		}

		if (j >= 3) {
			warp_dbg(WARP_DBG_WAR, "%s(): cidx = 0x%x not equal to 0x%x!!\n",
				__func__, value, (ring->ring_lens - 1));
			return -EBUSY;
		}
	}

	return 0;
}

static int
wifi_rro_rx_ring_rdy_ck_en(struct wed_entry *wed,
				struct wifi_entry *wifi)
{
	struct wifi_hw *hw = &wifi->hw;
	struct wed_rx_ring_ctrl *rro_data_ring_ctrl =
				&wed->res_ctrl.rx_ctrl.rro_data_ring_ctrl;
	struct wed_rx_ring_ctrl *rro_page_ring_ctrl =
				&wed->res_ctrl.rx_ctrl.rro_page_ring_ctrl;
#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	bool wifi_cap_rro_3_0 = hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_0);
	bool wifi_cap_rro_3_1 = hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_1);

	if (wifi_cap_rro_3_0 || wifi_cap_rro_3_1) {
		if (wifi_rx_ring_rdy_ck(rro_data_ring_ctrl, wifi)) {
			warp_dbg(WARP_DBG_ERR,
				"%s():chip:0x%x, wfdma rx rro data init fail\n",
				__func__, hw->chip_id);
			return -1;
		}
	}
#endif
#ifdef WED_RX_HW_RRO_3_0
	if (wifi_cap_rro_3_0) {
		if (wifi_rx_ring_rdy_ck(rro_page_ring_ctrl, wifi)) {
			warp_dbg(WARP_DBG_ERR,
				"%s():chip:0x%x, wfdma rx rro page init fail\n",
				__func__, hw->chip_id);
			return -1;
		}
	}
#endif
	return 0;
}

static void
wed_dma_ctrl(struct wed_entry *wed, u8 txrx)
{
	u32 wed_cfg;
	u32 wed_wdma_cfg;
	u32 wed_wpdma_cfg;
#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	bool wifi_cap_rro_3_0 = hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_0);
	bool wifi_cap_rro_3_1 = hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_1);
	u32 rro_data_drv_cfg;
#endif

	/*reset wed*/
	warp_io_read32(wed, WED_GLO_CFG_ADDR, &wed_cfg);
	warp_io_read32(wed, WED_WPDMA_GLO_CFG_ADDR, &wed_wpdma_cfg);
	warp_io_read32(wed, WED_WDMA_GLO_CFG_ADDR, &wed_wdma_cfg);

#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	if (wifi_cap_rro_3_0 || wifi_cap_rro_3_1)
		warp_io_read32(wed, RRO_RX_D_RING_CFG_ADDR_2_ADDR, &rro_data_drv_cfg);
#endif

	switch (txrx) {
	case WARP_DMA_TX:
		wed_cfg |= WED_GLO_CFG_TX_DMA_EN_MASK;
		wed_wpdma_cfg |= WED_WPDMA_GLO_CFG_TX_DRV_EN_MASK;
		wed_wdma_cfg |= WED_WDMA_GLO_CFG_RX_DRV_EN_MASK;

		warp_io_write32(wed, WED_GLO_CFG_ADDR, wed_cfg);
		warp_io_write32(wed, WED_WPDMA_GLO_CFG_ADDR, wed_wpdma_cfg);
#ifdef WED_HW_TX_SUPPORT
		warp_io_write32(wed, WED_WDMA_GLO_CFG_ADDR, wed_wdma_cfg);
#endif /*WED_HW_TX_SUPPORT*/

		break;
	case WARP_DMA_RX:
		wed_cfg |= WED_GLO_CFG_RX_DMA_EN_MASK;
		wed_wpdma_cfg |= WED_WPDMA_GLO_CFG_RX_DRV_EN_MASK;
		wed_wdma_cfg |= WED_WDMA_GLO_CFG_TX_DRV_EN_MASK;

#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
		if (wifi_cap_rro_3_0 || wifi_cap_rro_3_1)
			rro_data_drv_cfg |= RRO_RX_D_RING_CFG_ADDR_2_DRV_EN_MASK;
#endif

		warp_io_write32(wed, WED_GLO_CFG_ADDR, wed_cfg);
		warp_io_write32(wed, WED_WPDMA_GLO_CFG_ADDR, wed_wpdma_cfg);
#ifdef WED_HW_TX_SUPPORT
		warp_io_write32(wed, WED_WDMA_GLO_CFG_ADDR, wed_wdma_cfg);
#endif /*WED_HW_TX_SUPPORT*/
#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
		if (wifi_cap_rro_3_0 || wifi_cap_rro_3_1)
			warp_io_write32(wed, RRO_RX_D_RING_CFG_ADDR_2_ADDR, rro_data_drv_cfg);
#endif
		break;
	case WARP_DMA_TXRX:
		warp_dbg(WARP_DBG_INF, "%s(): %s DMA TXRX.\n", __func__,
			 (txrx ? "ENABLE" : "DISABLE"));
		wed_cfg |= (WED_GLO_CFG_TX_DMA_EN_MASK |
			    WED_GLO_CFG_RX_DMA_EN_MASK);
		wed_wpdma_cfg |= (WED_WPDMA_GLO_CFG_TX_DRV_EN_MASK |
				  WED_WPDMA_GLO_CFG_RX_DRV_EN_MASK);
		wed_wdma_cfg |= (WED_WDMA_GLO_CFG_TX_DRV_EN_MASK |
				 WED_WDMA_GLO_CFG_RX_DRV_EN_MASK);

#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
		if (wifi_cap_rro_3_0 || wifi_cap_rro_3_1)
			rro_data_drv_cfg |= RRO_RX_D_RING_CFG_ADDR_2_DRV_EN_MASK;
#endif

		warp_io_write32(wed, WED_GLO_CFG_ADDR, wed_cfg);
		warp_io_write32(wed, WED_WPDMA_GLO_CFG_ADDR, wed_wpdma_cfg);
#ifdef WED_HW_TX_SUPPORT
		warp_io_write32(wed, WED_WDMA_GLO_CFG_ADDR, wed_wdma_cfg);
#endif /*WED_HW_TX_SUPPORT*/
#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
		if (wifi_cap_rro_3_0 || wifi_cap_rro_3_1)
			warp_io_write32(wed, RRO_RX_D_RING_CFG_ADDR_2_ADDR, rro_data_drv_cfg);
#endif
		break;
	default:
		warp_dbg(WARP_DBG_INF,
			 "%s(): Unknown DMA control (%d).\n", __func__, txrx);
		break;
	}
}

/*
 *  DMA control after WM FWDL completed
 */
static void
wed_dma_ctrl_after_fwdl(struct wed_entry *wed, u8 txrx)
{
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	u32 rro_page_drv_cfg;
#ifdef WED_RX_HW_RRO_3_1
	u32 rro_3_1_drv_cfg;
#endif

	switch (txrx) {
	case WARP_DMA_TXRX:
		if (hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_0)) {
			warp_io_read32(wed, RRO_MSDU_PG_RING2_CFG1_ADDR, &rro_page_drv_cfg);

			/*
			 * RRO_MSDU_PG_RING2_CFG1_DRV_EN should be enabled after
			 * WM FWDL completed, otherwise system may malfunction
			 */
			if (rro_page_drv_cfg & RRO_MSDU_PG_RING2_CFG1_DRV_EN_MASK)
				warp_dbg(WARP_DBG_ERR,
					"%s(): RRO_MSDU_PG_RING2_CFG1_DRV_EN shouldn't be enabled before\n",
					__func__);

			rro_page_drv_cfg |= RRO_MSDU_PG_RING2_CFG1_DRV_EN_MASK;
			warp_io_write32(wed, RRO_MSDU_PG_RING2_CFG1_ADDR, rro_page_drv_cfg);
		}
#ifdef WED_RX_HW_RRO_3_1
		if (hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_1)) {
			warp_io_read32(wed, WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR, &rro_3_1_drv_cfg);

			/*
			 * WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RX_DRV_EN_MASK should be enabled after
			 * WM FWDL completed, otherwise system may malfunction
			 */
			if (rro_3_1_drv_cfg & WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RX_DRV_EN_MASK)
				warp_dbg(WARP_DBG_ERR,
					"%s(): WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RX_DRV_EN_MASK shouldn't be enabled before\n",
					__func__);

			rro_3_1_drv_cfg |= WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RX_DRV_EN_MASK;
			warp_io_write32(wed, WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR, rro_3_1_drv_cfg);
		}
#endif
		wifi_rro_rx_ring_rdy_ck_en(wed, wifi);
		break;
	default:
		warp_dbg(WARP_DBG_INF, "%s(): Unknown DMA control (%d).\n",
			 __func__, txrx);
		break;
	}
}

static int
reset_wed_tx_dma(struct wed_entry *wed, u32 reset_type)
{
#ifdef WED_TX_SUPPORT
	u32 value;
	int ret;

	warp_io_read32(wed, WED_GLO_CFG_ADDR, &value);
	value &= ~WED_GLO_CFG_TX_DMA_EN_MASK;
	warp_io_write32(wed, WED_GLO_CFG_ADDR, value);

	ret = wed_busy_chk(wed, WED_GLO_CFG_ADDR, WED_GLO_CFG_TX_DMA_BUSY_MASK);
	if (ret) {
		warp_dbg(WARP_DBG_ERR, "%s(): wed tx dma reset fail!\n", __func__);
		warp_dbg(WARP_DBG_ERR, "%s(): Reset whole WED Tx DMA module\n", __func__);
	}

	value = WED_MOD_RST_WED_TX_DMA_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);
#endif
	return 0;
}

static int wdma_tx_drv_fifo_check(struct wed_entry *wed)
{
	u32 rd_cnt, wr_cnt;
	u32 cnt = 0;

	do {
		warp_io_read32(wed, WED_WDMA_TX0_MIB_0_ADDR, &wr_cnt);
		warp_io_read32(wed, WED_WDMA_TX0_MIB_1_ADDR, &rd_cnt);

		if (rd_cnt == wr_cnt)
			break;

		udelay(CHK_DELAY_US);
	} while (cnt++ < WED_POLL_MAX);

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_ERR, "%s(): Polling FIFO empty failed", __func__);
		return -EBUSY;
	}

	if (wed_busy_chk(wed, WED_WDMA_ST_ADDR, WED_WDMA_ST_TX_DRV_ST_MASK)) {
		warp_dbg(WARP_DBG_ERR, "%s(): Polling drv idle failed", __func__);
		return -EBUSY;
	}

	return 0;
}

static int wdma_tx_pref_busy_check(struct wdma_entry *wdma)
{
	int ret;

	ret = wdma_busy_chk(wdma, WDMA_PREF_TX_CFG_ADDR, WDMA_PREF_TX_CFG_BUSY_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): Polling TX failed", __func__);

	return ret;
}

static int wdma_rx_pref_busy_check(struct wdma_entry *wdma)
{
	int ret;

	ret = wdma_busy_chk(wdma, WDMA_PREF_RX_CFG_ADDR, WDMA_PREF_RX_CFG_BUSY_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): Polling RX failed", __func__);

	return ret;
}

static int
wdma_pref_ctrl(struct wdma_entry *wdma, u8 txrx)
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

static int wdma_tx_glo_busy_check(struct wdma_entry *wdma)
{
	int ret;

	ret = wdma_busy_chk(wdma, WDMA_GLO_CFG0_ADDR, WDMA_GLO_CFG0_TX_DMA_BUSY_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): Polling TX failed", __func__);

	return ret;
}

static int wdma_rx_glo_busy_check(struct wdma_entry *wdma)
{
	int ret;

	ret = wdma_busy_chk(wdma, WDMA_GLO_CFG0_ADDR, WDMA_GLO_CFG0_RX_DMA_BUSY_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): Polling RX failed", __func__);

	return ret;
}

static int
wdma_dma_ctrl(struct wdma_entry *wdma, u8 txrx)
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

static int wdma_tx_wrbk_busy_check(struct wdma_entry *wdma)
{
	int ret;

	ret = wdma_busy_chk(wdma, WDMA_WRBK_TX_CFG_ADDR, WDMA_WRBK_TX_CFG_BUSY_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): Polling TX failed", __func__);

	return ret;
}

static int wdma_rx_wrbk_busy_check(struct wdma_entry *wdma)
{
	int ret;

	ret = wdma_busy_chk(wdma, WDMA_WRBK_RX_CFG_ADDR, WDMA_WRBK_RX_CFG_BUSY_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): Polling RX failed", __func__);

	return ret;
}


static int
wdma_wrbk_ctrl(struct wdma_entry *wdma, u8 txrx)
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

static void
wdma_rx_drv_skip_dmad_prepare(struct wed_entry *wed, bool enable)
{
	u32 wed_wdma_cfg;

	warp_io_read32(wed, WED_WDMA_GLO_CFG_ADDR, &wed_wdma_cfg);
	if (enable) {
		wed_wdma_cfg |= WED_WDMA_GLO_CFG_DYNAMIC_SKIP_DMAD_PREPARE_MASK;
		wed_wdma_cfg &= ~WED_WDMA_GLO_CFG_DYNAMIC_SKIP_DMAD_THRES_REACH_METHOD_MASK;
	} else {
		wed_wdma_cfg &= ~WED_WDMA_GLO_CFG_DYNAMIC_SKIP_DMAD_PREPARE_MASK;
		wed_wdma_cfg |= WED_WDMA_GLO_CFG_DYNAMIC_SKIP_DMAD_THRES_REACH_METHOD_MASK;
	}
	warp_io_write32(wed, WED_WDMA_GLO_CFG_ADDR, wed_wdma_cfg);
}

#ifdef WED_HW_TX_SUPPORT
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

static int
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

	if (wdma_pref_ctrl(wdma, WARP_DMA_TX_DISABLE))
		ret = -EIO;

	if (wdma_dma_ctrl(wdma, WARP_DMA_TX_DISABLE))
		ret = -EIO;

	if (wdma_wrbk_ctrl(wdma, WARP_DMA_TX_DISABLE))
		ret = -EIO;

	reset_wdma_rx_fifo(wdma);
	reset_wdma_rx_idx(wdma);

	return ret;
}

static int
reset_wdma_rx_drv(struct wed_entry *wed, struct wdma_entry *wdma, u32 reset_type)
{
	struct wdma_res_ctrl *res = &wdma->res_ctrl;
	struct wdma_rx_ctrl *rx_ctrl = &res->rx_ctrl;
	u32 value = 0;
	u32 ret = 0;

	/*Stop WED_WDMA Rx Driver Engine*/
	warp_io_read32(wed, WED_WDMA_GLO_CFG_ADDR, &value);
	value |= WED_WDMA_GLO_CFG_FSM_RETURN_IDLE_MASK;
	value &= ~WED_WDMA_GLO_CFG_RX_DRV_EN_MASK;
	warp_io_write32(wed, WED_WDMA_GLO_CFG_ADDR, value);

	value = WED_WDMA_GLO_CFG_RX_DRV_BUSY_MASK;
	if (wed_busy_chk(wed, WED_WDMA_GLO_CFG_ADDR, value)) {
		warp_dbg(WARP_DBG_ERR, "%s(): wed wdma rx drv can't return idle state!\n",
			__func__);
		ret = -EBUSY;
	}

	value = WED_WDMA_RX_PREF_CFG_BUSY_MASK;
	if (wed_busy_chk(wed, WED_WDMA_RX_PREF_CFG_ADDR, value)) {
		warp_dbg(WARP_DBG_ERR, "%s(): wed wdma rx pref can't return idle state!\n",
			__func__);
		ret = -EBUSY;
	}

	/* Recycle mode settings prepare */
	warp_io_read32(wed, WED_WDMA_GLO_CFG_ADDR, &value);
	value &= ~WED_WDMA_GLO_CFG_FSM_RETURN_IDLE_MASK;
	value &= ~WED_WDMA_GLO_CFG_RECYCLE_DESCRIPTOR_USE_TX_BM_EN_MASK;
	warp_io_write32(wed, WED_WDMA_GLO_CFG_ADDR, value);

	wdma_rx_drv_skip_dmad_prepare(wed, true);

	/* Enable WDMA_RX_DRV recycle mode */
	warp_io_read32(wed, WED_WDMA_GLO_CFG_ADDR, &value);
	value |= WED_WDMA_GLO_CFG_DYNAMIC_DMAD_RECYCLE_MASK;
	value |= WED_WDMA_GLO_CFG_RX_DRV_EN_MASK;
	warp_io_write32(wed, WED_WDMA_GLO_CFG_ADDR, value);

	if (wdma_rx_ring_poll_to_idle(wdma, &rx_ctrl->rx_ring_ctrl))
		warp_dbg(WARP_DBG_WAR, "%s(): polling wdma rx ring timeout\n", __func__);

	value = WDMA_GLO_CFG0_TX_DMA_BUSY_MASK;
	value |= WDMA_GLO_CFG0_RX_DMA_BUSY_MASK;
	if (wdma_busy_chk(wdma, WDMA_GLO_CFG0_ADDR, value))
		warp_dbg(WARP_DBG_ERR, "%s(): wed wdma rx drv with recycle mode failed\n",
			__func__);

	warp_io_read32(wed, WED_WDMA_GLO_CFG_ADDR, &value);
	value &= ~WED_WDMA_GLO_CFG_RX_DRV_EN_MASK;
	warp_io_write32(wed, WED_WDMA_GLO_CFG_ADDR, value);

	value = WED_WDMA_GLO_CFG_RX_DRV_BUSY_MASK;
	if (wed_busy_chk(wed, WED_WDMA_GLO_CFG_ADDR, value)) {
		warp_dbg(WARP_DBG_ERR, "%s(): wed wdma rx drv can't return idle state after recycle mode!\n",
			__func__);
		ret = -EBUSY;
	}

	value = WED_WDMA_RX_PREF_CFG_BUSY_MASK;
	if (wed_busy_chk(wed, WED_WDMA_RX_PREF_CFG_ADDR, value)) {
		warp_dbg(WARP_DBG_ERR, "%s(): wed wdma rx pref can't return idle state after recycle mode!\n",
			__func__);
		ret = -EBUSY;
	}

	/* Disable WDMA_RX_DRV recycle mode */
	warp_io_read32(wed, WED_WDMA_GLO_CFG_ADDR, &value);
	value &= ~WED_WDMA_GLO_CFG_DYNAMIC_DMAD_RECYCLE_MASK;
	value |= WED_WDMA_GLO_CFG_RECYCLE_DESCRIPTOR_USE_TX_BM_EN_MASK;
	warp_io_write32(wed, WED_WDMA_GLO_CFG_ADDR, value);

	wdma_rx_drv_skip_dmad_prepare(wed, false);

	/*Reset WDMA Interrupt Agent*/
	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value &= ~WED_CTRL_WDMA_INT_AGT_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);

	if (wed_busy_chk(wed, WED_CTRL_ADDR, WED_CTRL_WDMA_INT_AGT_BUSY_MASK))
		warp_dbg(WARP_DBG_ERR, "%s(): wed wdma int agent can't return idle state!\n",
			__func__);

	value = WED_MOD_RST_WDMA_INT_AGT_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);

	/*Reset WED RX Driver Engine*/
	value = WED_MOD_RST_WDMA_RX_DRV_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);

	return 0;
}
#endif

#ifdef WED_HW_RX_SUPPORT
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

static int
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

	if (wdma_pref_ctrl(wdma, WARP_DMA_RX_DISABLE))
		ret = -EIO;

	if (wdma_dma_ctrl(wdma, WARP_DMA_RX_DISABLE))
		ret = -EIO;

	if (wdma_wrbk_ctrl(wdma, WARP_DMA_RX_DISABLE))
		ret = -EIO;

	reset_wdma_tx_fifo(wdma);
	reset_wdma_tx_idx(wdma);

	return ret;
}

static int
reset_wdma_tx_drv(struct wed_entry *wed, u32 reset_type)
{
	u32 value;
	int ret;

	warp_io_read32(wed, WED_WDMA_GLO_CFG_ADDR, &value);
	value &= ~WED_WDMA_GLO_CFG_TX_DRV_EN_MASK;
	warp_io_write32(wed, WED_WDMA_GLO_CFG_ADDR, value);

	ret = wed_busy_chk(wed, WED_WDMA_ST_ADDR, WED_WDMA_ST_TX_DRV_ST_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): wed_wdma_tx_drv busy check failed\n",
			__func__);

	value = WED_MOD_RST_WDMA_TX_DRV_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);

	return 0;
}
#endif

static int reset_wed_tx_bm(struct wed_entry *wed)
{
#ifdef WED_HW_TX_SUPPORT
	u32 value;
	u32 cnt = 0;

	/*Tx Free Agent Reset*/
	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value &= ~WED_CTRL_WED_TX_FREE_AGT_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);

	if (wed_busy_chk(wed, WED_CTRL_ADDR, WED_CTRL_WED_TX_FREE_AGT_BUSY_MASK))
		warp_dbg(WARP_DBG_ERR, "%s(): tx free agent busy check failed\n", __func__);

	warp_io_read32(wed, WED_TX_TKID_INTF_ADDR, &value);
	value = GET_FIELD(WED_TX_TKID_INTF_FREE_TKFIFO_FDEP, value);

	while (value != 0x200 && cnt < WED_POLL_MAX) {
		warp_io_read32(wed, WED_TX_TKID_INTF_ADDR, &value);
		value = GET_FIELD(WED_TX_TKID_INTF_FREE_TKFIFO_FDEP, value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX)
		warp_dbg(WARP_DBG_ERR, "%s(): tx free agent fifo busy check failed\n", __func__);

	value = WED_MOD_RST_TX_FREE_AGT_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);

	/*Reset TX Buffer manager*/
	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value &= ~WED_CTRL_WED_TX_BM_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);

	value = WED_MOD_RST_TX_BM_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);
#endif /*WED_HW_TX_SUPPORT*/
	return 0;
}

static int
reset_wed_tx_drv(struct wed_entry *wed, u32 reset_type)
{
#ifdef WED_TX_SUPPORT
	u32 value;
	int ret;

	/*Disable TX driver*/
	warp_io_read32(wed, WED_WPDMA_GLO_CFG_ADDR, &value);
	value &= ~WED_WPDMA_GLO_CFG_TX_DRV_EN_MASK;
	warp_io_write32(wed, WED_WPDMA_GLO_CFG_ADDR, value);

	value = WED_WPDMA_GLO_CFG_TX_DRV_BUSY_MASK;
	ret = wed_busy_chk(wed, WED_WPDMA_GLO_CFG_ADDR, value);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): wed wpdma tx drv can't return idle!\n", __func__);

	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value &= ~WED_CTRL_WPDMA_INT_AGT_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);

	if (wed_busy_chk(wed, WED_CTRL_ADDR, WED_CTRL_WPDMA_INT_AGT_BUSY_MASK))
		warp_dbg(WARP_DBG_ERR, "%s(): wed wpdma int. agent can't return idle!\n",
			__func__);

	value = WED_MOD_RST_WPDMA_INT_AGT_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);

	value = WED_MOD_RST_WPDMA_TX_DRV_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);
#endif /*WED_TX_SUPPORT*/
	return 0;
}

#ifdef WED_PAO_SUPPORT
static int
reset_wed_pao(struct wed_entry *wed, u32 reset_type)
{
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wifi_entry *wifi = &warp->wifi;
	u32 value;

	warp_pao_exit_hw(wed, wifi);

	value = WED_MOD_RST_TX_PAO_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);

	return 0;
}
#endif
static int
reset_tx_traffic(struct wed_entry *wed, u32 reset_type)
{
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wdma_entry *wdma = &warp->wdma;
	struct wed_ser_ctrl *ser_ctrl = &wed->ser_ctrl;
	struct wed_ser_moudle_busy_cnt *busy_cnt = &ser_ctrl->ser_busy_cnt;
	int ret = 0;

	/* Disable wdma pse port */
	wdma_pse_port_config_state(warp->idx, false);

	/* host tx dma */
	if (reset_wed_tx_dma(wed, reset_type))
		busy_cnt->reset_wed_tx_dma++;

	/* wed_wdma_rx driver */
	if (reset_wdma_rx_drv(wed, wdma, reset_type))
		busy_cnt->reset_wed_wdma_rx_drv++;

	/* wdma rx dma */
	if (reset_wdma_rx(wdma)) {
		warp_dbg(WARP_DBG_ERR, "%s(): wdma_rx reset fail\n", __func__);
		busy_cnt->reset_wdma_rx++;
		ret = -EIO;
	}

	/* wed txbm */
	if (reset_wed_tx_bm(wed))
		busy_cnt->reset_wed_tx_bm++;

	/* wpdma tx driver */
	if (reset_wed_tx_drv(wed, reset_type))
		busy_cnt->reset_wed_wpdma_tx_drv++;

#ifdef WED_PAO_SUPPORT
	/* reset pao */
	if (reset_wed_pao(wed, reset_type))
		busy_cnt->reset_pao++;
#endif

	return ret;
}

static int
reset_rx_traffic(struct wed_entry *wed, u32 reset_type)
{
#ifdef WED_RX_SUPPORT
	u32 value;
	int ret;

	/*disable WPDMA RX Driver Engine*/
	warp_io_read32(wed, WED_WPDMA_GLO_CFG_ADDR, &value);
	value &= ~WED_WPDMA_GLO_CFG_RX_DRV_EN_MASK;
	warp_io_write32(wed, WED_WPDMA_GLO_CFG_ADDR, value);

	value = WED_WPDMA_GLO_CFG_RX_DRV_BUSY_MASK;
	ret = wed_busy_chk(wed, WED_WPDMA_GLO_CFG_ADDR, value);
	if (ret) {
		warp_dbg(WARP_DBG_ERR, "%s(): wed wpdma rx drv can't return idle state!\n",
			__func__);
		warp_dbg(WARP_DBG_ERR, "%s(): Reset whole WED WPDMA TxFreeDoneEvent Driver!\n",
			__func__);
	}

	/*WPDMA  interrupt agent*/
	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value &= ~WED_CTRL_WPDMA_INT_AGT_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);

	if (wed_busy_chk(wed, WED_CTRL_ADDR, WED_CTRL_WPDMA_INT_AGT_BUSY_MASK))
		warp_dbg(WARP_DBG_ERR, "%s(): wed wpdma int can't return idle state!\n",
			__func__);

	value = WED_MOD_RST_WPDMA_INT_AGT_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);
	/*WPDMA RX Driver Engine*/
	value = WED_MOD_RST_WPDMA_RX_DRV_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);
#endif /*WED_RX_SUPPORT*/
	return 0;
}

#ifdef WED_RX_D_SUPPORT
static int reset_wed_rx_dma(struct wed_entry *wed, u32 reset_type)
{
	u32 value;
	int ret;

	warp_io_read32(wed, WED_GLO_CFG_ADDR, &value);
	value &= ~WED_GLO_CFG_RX_DMA_EN_MASK;
	warp_io_write32(wed, WED_GLO_CFG_ADDR, value);

	value = WED_GLO_CFG_RX_DMA_BUSY_MASK;
	ret = wed_busy_chk(wed, WED_GLO_CFG_ADDR, value);

	value = WED_MOD_RST_WED_RX_DMA_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);

	return 0;
}

static int reset_wed_rx_bm(struct wed_entry *wed, u32 reset_type)
{
	int ret;
	u32 value;

	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value &= ~WED_CTRL_WED_RX_BM_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);

	ret = wed_busy_chk(wed, WED_CTRL_ADDR, WED_CTRL_WED_RX_BM_BUSY_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): wed_rx_bm busy check fail\n", __func__);

	value = WED_MOD_RST_RX_BM_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);

	return 0;
}
#endif

#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
static int reset_wed_rx_rro_data_drv(struct wed_entry *wed, u32 reset_type)
{
	int ret;
	u32 value;

	/* Disable RRO Data Drv */
	warp_io_read32(wed, RRO_RX_D_RING_CFG_ADDR_2_ADDR, &value);
	value &= ~RRO_RX_D_RING_CFG_ADDR_2_DRV_EN_MASK;
	warp_io_write32(wed, RRO_RX_D_RING_CFG_ADDR_2_ADDR, value);

	/* Reset RRO Data Drv */
	value = RRO_RX_D_RING_CFG_ADDR_2_DRV_CLR_MASK;
	warp_io_write32(wed, RRO_RX_D_RING_CFG_ADDR_2_ADDR, value);

	ret = wed_busy_chk(wed, RRO_RX_D_RING_CFG_ADDR_2_ADDR, value);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): RRO Data Drv reset failed\n",
			__func__);

	return 0;
}

static int reset_wed_rx_route_qm(struct wed_entry *wed, u32 reset_type)
{
	int ret;
	u32 value;

	/* wait for there is no traffic from ppe/rro */
	ret = wed_busy_chk(wed, WED_CTRL_ADDR, WED_CTRL_WED_RX_ROUTE_QM_BUSY_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): route qm can't return idle!\n", __func__);

	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value &= ~WED_CTRL_WED_RX_ROUTE_QM_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);

	value = WED_MOD_RST_RX_ROUTE_QM_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);

	return 0;
}

#endif

#ifdef WED_RX_HW_RRO_3_0
static int reset_wed_rx_rro_page_drv(struct wed_entry *wed, u32 reset_type)
{
	int ret;
	u32 value;

	/* Disable RRO MSDU Page Drv */
	warp_io_read32(wed, RRO_MSDU_PG_RING2_CFG1_ADDR, &value);
	value &= ~RRO_MSDU_PG_RING2_CFG1_DRV_EN_MASK;
	warp_io_write32(wed, RRO_MSDU_PG_RING2_CFG1_ADDR, value);

	/* Reset RRO MSDU Page Drv */
	value = RRO_MSDU_PG_RING2_CFG1_DRV_CLR_MASK;
	warp_io_write32(wed, RRO_MSDU_PG_RING2_CFG1_ADDR, value);

	ret = wed_busy_chk(wed, RRO_MSDU_PG_RING2_CFG1_ADDR, value);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): RRO Page Drv reset failed\n",
			__func__);

	return 0;
}

static int reset_wed_rx_rro_ind_cmd(struct wed_entry *wed, u32 reset_type)
{
	int ret;
	u32 value;

	/* Disable IND_CMD */
	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value &= ~WED_CTRL_WED_RX_IND_CMD_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);

	value = GENMASK(31, 0);
	ret = wed_busy_chk(wed, WED_RRO_RX_HW_STS_ADDR, value);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): IND_CMD disable failed\n", __func__);

	/* Reset counter */
	warp_io_read32(wed, WED_RX_IND_CMD_CNT0_ADDR, &value);
	value |= WED_RX_IND_CMD_CNT0_all_dbg_cnt_rst_MASK;
	warp_io_write32(wed, WED_RX_IND_CMD_CNT0_ADDR, value);

	warp_io_read32(wed, WED_RX_IND_CMD_CNT0_ADDR, &value);
	value &= ~WED_RX_IND_CMD_CNT0_all_dbg_cnt_rst_MASK;
	warp_io_write32(wed, WED_RX_IND_CMD_CNT0_ADDR, value);

	/* Reset IND_CMD/INFO_PAGE/PN_CHK */
	value = WED_MOD_RST_RRO_RX_TO_PG_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);

	return 0;
}

static int reset_wed_rx_page_bm(struct wed_entry *wed, u32 reset_type)
{
	int ret;
	u32 value;

	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value &= ~WED_CTRL_WED_RX_PG_BM_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);

	ret = wed_busy_chk(wed, WED_CTRL_ADDR, WED_CTRL_WED_RX_PG_BM_BUSY_MASK);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): wed_rx_pg_bm busy check fail\n", __func__);

	value = WED_MOD_RST_RX_PG_BM_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);

	return 0;
}
#endif

#ifdef WED_RX_HW_RRO_3_1
static int reset_wed_rx_rro_3_1_drv(struct wed_entry *wed, u32 reset_type)
{
	int ret = 0;
	u32 value;

	/* Disable RRO 3.1 Drv */
	warp_io_read32(wed, WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR, &value);
	value &= ~WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RX_DRV_EN_MASK;
	warp_io_write32(wed, WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR, value);

	value = WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RX_DRV_BUSY_MASK;
	if (wed_busy_chk(wed, WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR, value)) {
		warp_dbg(WARP_DBG_ERR, "%s(): RRO 3.1 Drv disable failed\n", __func__);
		ret = -EBUSY;
	}

	/* Disable RRO 3.1 Prefetch */
	warp_io_read32(wed, WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR, &value);
	value &= ~WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ENABLE_MASK;
	warp_io_write32(wed, WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR, value);

	value = WED_WPDMA_RRO3_1_RX_D_PREF_CFG_BUSY_MASK;
	if (wed_busy_chk(wed, WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR, value)) {
		warp_dbg(WARP_DBG_ERR, "%s(): RRO 3.1 Prefetch disable failed\n", __func__);
		ret = -EBUSY;
	}

	/* Reset RRO 3.1 Drv */
	value = WED_MOD_RST_WPDMA_RRO3_1_RX_D_DRV_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);

	return 0;
}
#endif

static int
reset_rx_rro_traffic(struct wed_entry *wed, u32 reset_type)
{
	int ret = 0;
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wdma_entry *wdma = &warp->wdma;
	struct wifi_entry *wifi = &warp->wifi;
	struct wed_ser_ctrl *ser_ctrl = &wed->ser_ctrl;
	struct wed_ser_moudle_busy_cnt *busy_cnt = &ser_ctrl->ser_busy_cnt;
#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	struct wifi_hw *hw = &wifi->hw;
	bool wifi_cap_rro_3_0 = hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_0);
	bool wifi_cap_rro_3_1 = hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_1);
#endif

#ifdef WED_RX_HW_RRO_3_1
	if (wifi_cap_rro_3_1) {
		if (reset_wed_rx_rro_3_1_drv(wed, reset_type))
			busy_cnt->reset_wed_rx_rro_data_3_1_drv++;
	}
#endif
#ifdef WED_RX_HW_RRO_3_0
	if (wifi_cap_rro_3_0) {
		if (reset_wed_rx_rro_ind_cmd(wed, reset_type))
			busy_cnt->reset_wed_rx_rro++;

		if (reset_wed_rx_rro_page_drv(wed, reset_type))
			busy_cnt->reset_wed_rx_rro_drv++;

		if (reset_wed_rx_page_bm(wed, reset_type))
			busy_cnt->reset_wed_rx_page_bm++;
	}
#endif
#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	if (wifi_cap_rro_3_0 || wifi_cap_rro_3_1) {
		if (reset_wed_rx_rro_data_drv(wed, reset_type))
			busy_cnt->reset_wed_rx_rro_drv++;
	}
#endif

	if (reset_wed_rx_route_qm(wed, reset_type))
		busy_cnt->reset_wed_rx_route_qm++;

	if (reset_wdma_tx_drv(wed, reset_type))
		busy_cnt->reset_wdma_tx_drv++;

	if (reset_wdma_tx(wdma)) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wdma_tx failed\n", __func__);
		busy_cnt->reset_wdma_tx++;
		ret = -EIO;
	}

	if (reset_wed_rx_dma(wed, reset_type))
		busy_cnt->reset_wed_rx_dma++;

	if (reset_wed_rx_bm(wed, reset_type))
		busy_cnt->reset_wed_rx_bm++;

	return ret;
}

static int
rtqm_ppe_feedback_ctrl(struct wed_entry *wed, bool ctrl)
{
	u32 dbg_value = 0;

	warp_io_read32(wed, WED_RTQM_DBG_CFG_ADDR, &dbg_value);

	if (ctrl)
		dbg_value |= WED_RTQM_DBG_CFG_PPE_FDBK_DROP_MASK;
	else
		dbg_value &= ~WED_RTQM_DBG_CFG_PPE_FDBK_DROP_MASK;

	warp_io_write32(wed, WED_RTQM_DBG_CFG_ADDR, dbg_value);
	return 0;
}

static int
rtqm_igrs_ctrl(struct wed_entry *wed, bool ctrl)
{
	u32 glo_value = 0;

	warp_io_read32(wed, WED_RTQM_GLO_CFG_ADDR, &glo_value);

	if (ctrl)
		/* ENABLE[7:5] = IGRS 3~1 */
		glo_value |= SET_FIELD(WED_RTQM_GLO_CFG_ENABLE, GENMASK(7, 5));
	else
		glo_value &= ~SET_FIELD(WED_RTQM_GLO_CFG_ENABLE, GENMASK(7, 5));

	warp_io_write32(wed, WED_RTQM_GLO_CFG_ADDR, glo_value);
	return 0;
}

static void
rtqm_dmad_mod_ctrl(struct wed_entry *wed, bool ctrl)
{
	u32 value;

	if (ctrl) {
		warp_io_read32(wed, WED_RTQM_IGRS0_CFG0_ADDR, &value);
		/* DMAD_MOD_EN[0] = PPE_VALID */
		value |= SET_FIELD(WED_RTQM_IGRS0_CFG0_DMAD_MOD_EN, BIT(0));
		warp_io_write32(wed, WED_RTQM_IGRS0_CFG0_ADDR, value);

		warp_io_read32(wed, WED_RTQM_IGRS0_CFG1_ADDR, &value);
		value &= ~WED_RTQM_IGRS0_CFG1_DMAD_MOD_PPE_VLD_MASK;
		warp_io_write32(wed, WED_RTQM_IGRS0_CFG1_ADDR, value);
	} else {
		warp_io_read32(wed, WED_RTQM_IGRS0_CFG0_ADDR, &value);
		value &= ~SET_FIELD(WED_RTQM_IGRS0_CFG0_DMAD_MOD_EN, BIT(0));
		warp_io_write32(wed, WED_RTQM_IGRS0_CFG0_ADDR, value);
	}
}

static void
rtqm_age_out_ctrl(struct wed_entry *wed, bool ctrl)
{
	u32 value;

	if (ctrl) {
		/* Configure ageout duration as 0 */
		warp_io_write32(wed, WED_RTQM_AGE_CFG1_ADDR, 0);

		/* Configure PPE feedback TO_HOST as true */
		warp_io_read32(wed, WED_RTQM_AGE_CFG0_ADDR, &value);
		value |= WED_RTQM_AGE_CFG0_DFDBK_TO_HOST_MASK;
		warp_io_write32(wed, WED_RTQM_AGE_CFG0_ADDR, value);

		/* Enable ageout */
		/* ENABLE[8] = ageout */
		warp_io_read32(wed, WED_RTQM_GLO_CFG_ADDR, &value);
		value |= SET_FIELD(WED_RTQM_GLO_CFG_ENABLE, BIT(8));
		warp_io_write32(wed, WED_RTQM_GLO_CFG_ADDR, value);
	} else {
		/* Disable ageout */
		warp_io_read32(wed, WED_RTQM_GLO_CFG_ADDR, &value);
		value &= ~SET_FIELD(WED_RTQM_GLO_CFG_ENABLE, BIT(8));
		warp_io_write32(wed, WED_RTQM_GLO_CFG_ADDR, value);
	}
}

static int
rtqm_flush_pkt(struct wed_entry *wed)
{
	u32 value;
	int ret;

	/* Manual flushout */
	warp_io_read32(wed, WED_RTQM_AGE_CFG0_ADDR, &value);
	value |= WED_RTQM_AGE_CFG0_FLUSH_EN_MASK;
	warp_io_write32(wed, WED_RTQM_AGE_CFG0_ADDR, value);

	/* Wait fifo queue empty */
	value = WED_RTQM_AGE_CFG0_FLUSH_EN_MASK;
	ret = wed_busy_chk(wed, WED_RTQM_AGE_CFG0_ADDR, value);
	if (ret) {
		warp_dbg(WARP_DBG_ERR, "%s(): FIFO queue empty timeout!\n", __func__);
		goto end;
	}

	/* Wait ageout queue empty */
	value = WED_RTQM_PFDBK_FIFO_CFG_A2Q_EMPTY_MASK;
	ret = wed_rdy_chk(wed, WED_RTQM_PFDBK_FIFO_CFG_ADDR, value);
	if (ret) {
		warp_dbg(WARP_DBG_ERR, "%s(): Dequeue age out packet timeout!\n", __func__);
		goto end;
	}

	/* Wait rtqm queue empty*/
	value = WED_RTQM_GLO_CFG_STATUS_QUEUE_NOT_EMPTY_MASK;
	ret = wed_busy_chk(wed, WED_RTQM_GLO_CFG_ADDR, value);
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): RTQM queue empty timeout!\n", __func__);

end:
	return ret;
}

static int
stop_tx_traffic(struct wed_entry *wed, struct wdma_entry *wdma)
{
	return 0;
}


static void
restore_tx_traffic(struct wed_entry *wed, struct wdma_entry *wdma)
{

}

static int
stop_rx_traffic(struct wed_entry *wed, struct wdma_entry *wdma)
{
	struct wdma_res_ctrl *res = &wdma->res_ctrl;
	struct wdma_tx_ctrl *tx_ctrl = &res->tx_ctrl;
	struct wdma_rx_ctrl *rx_ctrl = &res->rx_ctrl;
	u32 value;
	int ret;

	/* Disable IGRS from wifi */
	rtqm_igrs_ctrl(wed, 0);

	/* Check if drv's fifo is empty */
	ret = wdma_tx_drv_fifo_check(wed);
	if (ret)
		goto err_fifo;

	/* Disable drv */
	warp_io_read32(wed, WED_WDMA_GLO_CFG_ADDR, &value);
	value &= ~WED_WDMA_GLO_CFG_TX_DRV_EN_MASK;
	warp_io_write32(wed, WED_WDMA_GLO_CFG_ADDR, value);

	/* Check WDMA status */
	ret = wdma_tx_ring_poll_to_idle(wdma, &tx_ctrl->tx_ring_ctrl);
	if (ret) {
		warp_dbg(WARP_DBG_ERR, "%s(): Polling wdma tx ring timeout\n", __func__);
		goto err_wdma;
	}

	ret = wdma_tx_glo_busy_check(wdma);
	if (ret) {
		warp_dbg(WARP_DBG_ERR, "%s(): Polling wdma tx idle timeout\n", __func__);
		goto err_wdma;
	}

	ret = wdma_rx_ring_poll_to_idle(wdma, &rx_ctrl->rx_ring_ctrl);
	if (ret) {
		warp_dbg(WARP_DBG_ERR, "%s(): Polling wdma rx ring timeout\n", __func__);
		goto err_wdma;
	}

	ret = wdma_rx_glo_busy_check(wdma);
	if (ret) {
		warp_dbg(WARP_DBG_ERR, "%s(): Polling wdma rx idle timeout\n", __func__);
		goto err_wdma;
	}

	/* Disable WDMA */
	wdma_pref_ctrl(wdma, WARP_DMA_RX_DISABLE);
	wdma_dma_ctrl(wdma, WARP_DMA_RX_DISABLE);
	wdma_wrbk_ctrl(wdma, WARP_DMA_RX_DISABLE);

	/* Configure RTQM */
	rtqm_ppe_feedback_ctrl(wed, 1);
	rtqm_dmad_mod_ctrl(wed, 1);
	rtqm_age_out_ctrl(wed, 1);

	/* Flush RTQM */
	ret = rtqm_flush_pkt(wed);
	if (ret)
		goto err_flush;

	return 0;
err_flush:
	rtqm_age_out_ctrl(wed, 0);
	rtqm_dmad_mod_ctrl(wed, 0);
	rtqm_ppe_feedback_ctrl(wed, 0);
err_wdma:
err_fifo:
	rtqm_igrs_ctrl(wed, 1);
	return ret;
}

static void
restore_rx_traffic(struct wed_entry *wed, struct wdma_entry *wdma)
{
	u32 value;

	/* Restore RTQM */
	rtqm_age_out_ctrl(wed, 0);
	rtqm_dmad_mod_ctrl(wed, 0);
	rtqm_ppe_feedback_ctrl(wed, 0);

	/* Enable WDMA */
	wdma_pref_ctrl(wdma, WARP_DMA_RX);
	wdma_dma_ctrl(wdma, WARP_DMA_RX);
	wdma_wrbk_ctrl(wdma, WARP_DMA_RX);

	/* Enable drv */
	warp_io_read32(wed, WED_WDMA_GLO_CFG_ADDR, &value);
	value |= WED_WDMA_GLO_CFG_TX_DRV_EN_MASK;
	warp_io_write32(wed, WED_WDMA_GLO_CFG_ADDR, value);

	/* Enable IGRS from wifi */
	rtqm_igrs_ctrl(wed, 1);
}

static void
wed_ctr_intr_set(struct wed_entry *wed, enum wed_int_agent int_agent,
		 unsigned char enable)
{
	u32 value = 0;

	warp_io_read32(wed, WED_CTRL_ADDR, &value);

	if (enable) {
		value |= WED_CTRL_WPDMA_INT_AGT_EN_MASK;
		value |= WED_CTRL_WDMA_INT_AGT_EN_MASK;
	} else {
		value &= ~WED_CTRL_WPDMA_INT_AGT_EN_MASK;
		value &= ~WED_CTRL_WDMA_INT_AGT_EN_MASK;
	}

	warp_io_write32(wed, WED_CTRL_ADDR, value);

#ifdef WED_DELAY_INT_SUPPORT
	warp_io_write32(wed, WED_DLY_INT_CFG_ADDR, WED_DLY_INT_VALUE);
#endif /*WED_DELAY_INT_SUPPORT*/
}

static u32
wifi_cr_get(struct warp_entry *entry, u32 cr)
{
	u32 i = 0;

	while (entry->mtbl[i].wifi_cr != 0) {
		if (entry->mtbl[i].warp_cr == cr)
			return entry->mtbl[i].wifi_cr;

		i++;
	}

	warp_dbg(WARP_DBG_ERR, "%s(): can't get wifi cr from warp cr %x\n", __func__,
		 cr);
	return 0;
}

static void
wed_wpdma_inter_tx_init(struct wed_entry *wed, struct wifi_hw *hw)
{
	u32 value = 0;

#ifdef WED_TX_SUPPORT
	switch (hw->tx_ring_num) {
	case 2:
		/*TX1*/
		value |= WED_WPDMA_INT_CTRL_TX_TX_DONE_EN1_MASK;
		value |= WED_WPDMA_INT_CTRL_TX_TX_DONE_CLR1_MASK;
		value |= SET_FIELD(WED_WPDMA_INT_CTRL_TX_TX_DONE_TRIG1,
				   hw->wfdma_tx_done_trig1_bit);
		fallthrough;
	case 1:
		/*TX0*/
		value |= WED_WPDMA_INT_CTRL_TX_TX_DONE_EN0_MASK;
		value |= WED_WPDMA_INT_CTRL_TX_TX_DONE_CLR0_MASK;
		value |= SET_FIELD(WED_WPDMA_INT_CTRL_TX_TX_DONE_TRIG0,
				   hw->wfdma_tx_done_trig0_bit);
		break;
	default:
		warp_dbg(WARP_DBG_ERR, "%s(): tx_ring_num = %u is not valid\n",
			__func__, hw->tx_ring_num);
	}
	warp_io_write32(wed, WED_WPDMA_INT_CTRL_TX_ADDR, value);
#endif /*WED_TX_SUPPORT*/
#ifdef WED_RX_SUPPORT
	/*free notify*/
	value = 0;
	value |= WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_EN0_MASK;
	value |= WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_CLR0_MASK;
	value |= SET_FIELD(WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_TRIG0,
			   hw->wfdma_tx_done_free_notify_trig_bit);
	warp_io_write32(wed, WED_WPDMA_INT_CTRL_TX_FREE_ADDR, value);
#endif /*WED_RX_SUPPORT*/
}

static void
wed_wpdma_inter_rx_rro_data_init(struct wed_entry *wed, struct wifi_hw *hw)
{
	u32 value = 0;

	switch (hw->rx_rro_data_ring_num) {
	case 2:
		/* RRO RX1 */
		value |= WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_EN1_MASK;
		value |= WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_CLR1_MASK;
		value |= SET_FIELD(WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_TRIG1,
				   hw->wfdma_rro_rx_done_trig1_bit);
		fallthrough;
	case 1:
		/* RRO RX0 */
		value |= WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_EN0_MASK;
		value |= WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_CLR0_MASK;
		value |= SET_FIELD(WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_TRIG0,
				   hw->wfdma_rro_rx_done_trig0_bit);
		break;
	default:
		warp_dbg(WARP_DBG_ERR, "%s(): rx_rro_data_ring_num = %u is not valid\n",
			__func__, hw->rx_rro_data_ring_num);
	}
	warp_io_write32(wed, WED_WPDMA_INT_CTRL_RRO_RX_ADDR, value);

}

static void
wed_wpdma_inter_rx_rro_page_init(struct wed_entry *wed, struct wifi_hw *hw)
{
	u32 value = 0;

	switch (hw->rx_rro_page_ring_num) {
	case 3:
		/* RRO PAGE RX2 */
		value |= WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_EN2_MASK;
		value |= WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_CLR2_MASK;
		value |= SET_FIELD(WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_TRIG2,
				   hw->wfdma_rro_rx_pg_trig2_bit);
		fallthrough;
	case 2:
		/* RRO PAGE RX1 */
		value |= WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_EN1_MASK;
		value |= WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_CLR1_MASK;
		value |= SET_FIELD(WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_TRIG1,
				   hw->wfdma_rro_rx_pg_trig1_bit);
		fallthrough;
	case 1:
		/* RRO PAGE RX0 */
		value |= WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_EN0_MASK;
		value |= WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_CLR0_MASK;
		value |= SET_FIELD(WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_TRIG0,
				   hw->wfdma_rro_rx_pg_trig0_bit);
		break;
	default:
		warp_dbg(WARP_DBG_ERR, "%s(): rx_rro_page_ring_num = %u is not valid\n",
			__func__, hw->rx_rro_page_ring_num);
	}
	warp_io_write32(wed, WED_WPDMA_INT_CTRL_RRO_MSDU_PG_ADDR, value);
}

#ifdef WED_RX_HW_RRO_3_1
static void
wed_wpdma_inter_rx_rro_3_1_init(struct wed_entry *wed, struct wifi_hw *hw)
{
	u32 value;

	warp_io_read32(wed, WED_WPDMA_INT_CTRL_RX_ADDR, &value);

	switch (hw->rx_rro_3_1_ring_num) {
	case 1:
		value |= WED_WPDMA_INT_CTRL_RX_RX_RRO3_1_DONE_EN_MASK;
		value |= WED_WPDMA_INT_CTRL_RX_RX_RRO3_1_DONE_CLR_MASK;
		value |= SET_FIELD(WED_WPDMA_INT_CTRL_RX_RX_RRO3_1_DONE_TRIG,
				   hw->wfdma_rro_rx_3_1_trig_bit);
		break;
	default:
		warp_dbg(WARP_DBG_ERR, "%s(): rx_rro_3_1_ring_num = %u is not valid\n",
			__func__, hw->rx_rro_3_1_ring_num);
	}

	warp_io_write32(wed, WED_WPDMA_INT_CTRL_RX_ADDR, value);
}
#endif

static void
wed_inter_rx_wed_init(struct wed_entry *wed, struct wifi_hw *hw)
{
	u32 value = 0;

	/* WED to Host RxRing */
	switch (hw->rx_wed_ring_num) {
	case 2:
		value |= SET_FIELD(WED_INT_CTRL_RX_DONE_ASSERT1, hw->rx_wed_done_trig1_bit);
		fallthrough;
	case 1:
		value |= SET_FIELD(WED_INT_CTRL_RX_DONE_ASSERT0, hw->rx_wed_done_trig0_bit);
		break;
	default:
		warp_dbg(WARP_DBG_ERR, "%s(): rx_wed_ring_num = %u is not valid\n",
			__func__, hw->rx_wed_ring_num);
	}

	warp_io_write32(wed, WED_INT_CTRL_ADDR, value);
}

void
warp_int_ctrl_hw(struct wed_entry *wed, struct wifi_entry *wifi,
	struct wdma_entry *wdma, u32 int_agent,
	u8 enable, u32 pcie_ints_offset, int idx)
{
	struct wifi_hw *hw = &wifi->hw;
	u32 value = 0;
#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	bool wifi_cap_rro_3_0 = hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_0);
	bool wifi_cap_rro_3_1 = hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_1);
#endif

	/*wed control cr set*/
	wed_ctr_intr_set(wed, int_agent, enable);

	/* initail tx interrupt trigger */
	wed_wpdma_inter_tx_init(wed, hw);

#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	if (wifi_cap_rro_3_0 || wifi_cap_rro_3_1)
		wed_wpdma_inter_rx_rro_data_init(wed, hw);
#endif
#ifdef WED_RX_HW_RRO_3_0
	if (wifi_cap_rro_3_0)
		wed_wpdma_inter_rx_rro_page_init(wed, hw);
#endif
#ifdef WED_RX_HW_RRO_3_1
	if (wifi_cap_rro_3_1)
		wed_wpdma_inter_rx_rro_3_1_init(wed, hw);
#endif

	wed_inter_rx_wed_init(wed, hw);

	{
		/*WED_WDMA Interrupt agent */
		value = 0;

		if (enable) {
			value |= WDMA_INT_MASK_RX_DONE_INT0_MASK;
			value |= WDMA_INT_MASK_TX_DONE_INT0_MASK;
		}

		warp_io_write32(wed, WED_WDMA_INT_TRIG_ADDR, value);

		warp_io_read32(wed, WED_WDMA_INT_CLR_ADDR, &value);

		if (enable) {
			value |= WDMA_INT_MASK_RX_DONE_INT0_MASK;
			value |= WDMA_INT_MASK_TX_DONE_INT0_MASK;
		} else {
			value &= ~WDMA_INT_MASK_RX_DONE_INT1_MASK;
			value &= ~WDMA_INT_MASK_RX_DONE_INT0_MASK;
		}

		warp_io_write32(wed, WED_WDMA_INT_CLR_ADDR, value);

		warp_wdma_int_sel(wed, idx);

		/*WDMA interrupt enable*/
		value = 0;

		if (enable) {
			value |= WDMA_INT_MASK_RX_DONE_INT0_MASK;
			value |= WDMA_INT_MASK_TX_DONE_INT0_MASK;
		}

		warp_io_write32(wdma, WDMA_INT_MASK_ADDR, value);
		warp_io_write32(wdma, WDMA_INT_GRP2_ADDR, value);
	}

	if (enable && hw->p_int_mask) {
		warp_pdma_mask_set_hw(wed, *hw->p_int_mask);
		warp_io_write32(wifi, hw->int_mask, *hw->p_int_mask);
	} else {
		warp_pdma_mask_set_hw(wed, 0);
		warp_io_write32(wifi, hw->int_mask, 0);
	}
}

void
warp_eint_init_hw(struct wed_entry *wed)
{
	u32 value;

	value = WED_EX_INT_STA_RX_DRV_COHERENT_MASK;

	/* TODO:
	 * Commented since irq storm occurs without recovery,
	 * Uncomment if recovery has been implemented
	 */
	//value |= WED_EX_INT_STA_ERR_MON_MASK;

	wed->ext_int_mask = value;
}

void
warp_eint_get_stat_hw(struct wed_entry *wed, u32 *state, u32 *err_state)
{
	/*read stat*/
	warp_io_read32(wed, WED_EX_INT_STA_ADDR, state);


	/*write 1 clear*/
	warp_io_write32(wed, WED_EX_INT_STA_ADDR, *state);

	/* TODO:
	 * Move the func after below mask if recovery has been implemented
	 */
	if (*state & WED_EX_INT_STA_ERR_MON_MASK) {
		warp_io_read32(wed, WED_ERR_MON_ADDR, err_state);
		warp_io_write32(wed, WED_ERR_MON_ADDR, *err_state);
	}

	*state &= wed->ext_int_mask;
}

void
warp_dma_ctrl_hw(struct wed_entry *wed, u8 txrx)
{
	int ret = 0;
#ifdef WED_HW_RX_SUPPORT
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wdma_entry *wdma = &warp->wdma;
#endif

	wed_dma_ctrl(wed, txrx);
#ifdef WED_HW_RX_SUPPORT
	switch (txrx) {
	case WARP_DMA_TX:
	case WARP_DMA_TXRX:
		ret = wdma_all_rx_ring_rdy_ck(wed, wdma);
		if (ret) {
			warp_dbg(WARP_DBG_ERR, "%s(): WDMA rx ring rdy fail, pls check txbm\n",
				__func__);
			return;
		}

		/* Enable wdma pse port */
		wdma_pse_port_config_state(warp->idx, true);

		fallthrough;
	case WARP_DMA_RX:
		wdma_pref_ctrl(wdma, txrx);
		wdma_dma_ctrl(wdma, txrx);
		wdma_wrbk_ctrl(wdma, txrx);
	}
#endif
}

void
warp_dma_ctrl_hw_after_fwdl(struct wed_entry *wed, u8 txrx)
{
	wed_dma_ctrl_after_fwdl(wed, txrx);
}

void
warp_wed_ver(struct wed_entry *wed)
{
	u32 version = 0;

	warp_io_read32(wed, WED_REV_ID_ADDR, &version);
	wed->ver = GET_FIELD(WED_REV_ID_MAJOR, version);
	wed->sub_ver = GET_FIELD(WED_REV_ID_MINOR, version);
	wed->branch = GET_FIELD(WED_REV_ID_BRANCH, version);
	wed->eco = GET_FIELD(WED_REV_ID_ECO, version);
}

void
warp_conf_hwcap(struct wed_entry *wed)
{
#ifdef WED_RX_HW_RRO_3_0_DBG
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
#endif
	warp_wed_ver(wed);

	wed->hw_cap = WED_HW_CAP_32K_TXBUF;
#ifdef WED_RX_HW_RRO_3_0
	wed->hw_cap |= WED_HW_CAP_RX_OFFLOAD;
#endif
#ifdef WED_RX_HW_RRO_3_0_DBG
	if (hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_0))
		wed->hw_cap |= WED_HW_CAP_RRO_DBG_MODE;
#endif
#ifdef WED_RX_HW_RRO_3_0_DBG_MEMCPY
	if (hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_0))
		wed->hw_cap |= WED_HW_CAP_RRO_DBG_MEMCPY_MODE;
#endif
}

void
warp_wed_512_support_hw(struct wed_entry *wed, u8 *enable)
{
}

void
warp_wed_init_hw(struct wed_entry *wed, struct wdma_entry *wdma)
{
	u32 wed_wpdma_cfg;

	warp_io_read32(wed, WED_WPDMA_GLO_CFG_ADDR, &wed_wpdma_cfg);

	wed_wpdma_cfg |= WED_WPDMA_GLO_CFG_RX_DDONE2_WR_MASK;

	/* Enable TX_DDONE and LAST DMAD check */
	wed_wpdma_cfg |= WED_WPDMA_GLO_CFG_TX_DDONE_CHK_MASK;
	wed_wpdma_cfg |= WED_WPDMA_GLO_CFG_TX_DDONE_CHK_LAST_MASK;

	/* Auto detect mode, ver > 4 will parse as ver = 4 */
	wed_wpdma_cfg |= WED_WPDMA_GLO_CFG_RX_DRV_EVENT_PKT_FMT_CHK_MASK;
	wed_wpdma_cfg |= WED_WPDMA_GLO_CFG_RX_DRV_UNS_VER_FORCE_4_MASK;

	/* TX_DRV use HIF TXD V0.1 (For > 32 bits platform and PAO disabled) */
	wed_wpdma_cfg |= WED_WPDMA_GLO_CFG_TXD_VER_MASK;

	/* New TXBM, not required to keep token id */
	wed_wpdma_cfg &= ~WED_WPDMA_GLO_CFG_TX_TKID_KEEP_MASK;

	warp_io_write32(wed, WED_WPDMA_GLO_CFG_ADDR, wed_wpdma_cfg);
}

void
warp_wdma_init_hw(struct wed_entry *wed, struct wdma_entry *wdma, int idx)
{
#ifdef WED_HW_TX_SUPPORT
	u32 value;
	u32 wed_wdma_cfg, wed_wdma_pref;

	/* WDMA base physical address */
	warp_io_write32(wed, WED_WDMA_CFG_BASE_ADDR, wdma->base_phy_addr);

	/* WDMA GLO_CFG and INT_STA offset */
	value = SET_FIELD(WED_WDMA_OFST0_GLO_CFG, WDMA_GLO_CFG0_ADDR);
	value |= SET_FIELD(WED_WDMA_OFST0_INTS, WDMA_INT_STATUS_ADDR);
	warp_io_write32(wed, WED_WDMA_OFST0_ADDR, value);

	/* WDMA RX0 and TX0 offset */
	value = SET_FIELD(WED_WDMA_OFST1_RX0_CTRL, WDMA_RX_BASE_PTR_0_ADDR);
	value |= SET_FIELD(WED_WDMA_OFST1_TX0_CTRL, WDMA_TX_BASE_PTR_0_ADDR);
	warp_io_write32(wed, WED_WDMA_OFST1_ADDR, value);

	/* WED_WDMA_GLO_CFG */
	warp_io_read32(wed, WED_WDMA_GLO_CFG_ADDR, &wed_wdma_cfg);
	wed_wdma_cfg |= WED_WDMA_GLO_CFG_TX_DDONE_CHK_MASK;
	/* Cannot move DRV_IDX if TXBM no buffer */
	wed_wdma_cfg &= ~WED_WDMA_GLO_CFG_DYNAMIC_SKIP_DMAD_PREPARE_MASK;
	wed_wdma_cfg |= WED_WDMA_GLO_CFG_DYNAMIC_SKIP_DMAD_THRES_REACH_METHOD_MASK;
	warp_io_write32(wed, WED_WDMA_GLO_CFG_ADDR, wed_wdma_cfg);

	/* WED_WDMA_RX_PREF_CFG */
	warp_io_read32(wed, WED_WDMA_RX_PREF_CFG_ADDR, &wed_wdma_pref);
	wed_wdma_pref |= WED_WDMA_RX_PREF_CFG_DDONE2_EN_MASK;
	wed_wdma_pref |= WED_WDMA_RX_PREF_CFG_ENABLE_MASK;
	warp_io_write32(wed, WED_WDMA_RX_PREF_CFG_ADDR, wed_wdma_pref);

	/* Disable delay writing CIDX */
	warp_io_read32(wed, WED_WDMA_RX_CIDX_WR_CFG_ADDR, &value);
	value &= ~WED_WDMA_RX_CIDX_WR_CFG_CIDX_DLY_CNT_MAX_MASK;
	warp_io_write32(wed, WED_WDMA_RX_CIDX_WR_CFG_ADDR, value);
#endif /*WED_HW_TX_SUPPORT*/
}

#ifdef WED_HW_TX_SUPPORT
static void
warp_wdma_rx_ring_init_hw(struct wed_entry *wed, struct wdma_entry *wdma)
{
	struct wdma_rx_ring_ctrl *rx_ring_ctrl = &wdma->res_ctrl.rx_ctrl.rx_ring_ctrl;
	struct warp_ring *rx_ring;
	u32 offset = WED_WDMA_RX1_BASE_ADDR - WED_WDMA_RX0_BASE_ADDR;
	u32 value;
	int i;

	for (i = 0; i < rx_ring_ctrl->ring_num; i++) {
		rx_ring = &rx_ring_ctrl->ring[i];

		warp_dbg(WARP_DBG_INF, "%s(): wed = 0x%p, wdma = 0x%p, ring_no = %d\n",
			__func__, wed, wdma, i);
		warp_dbg(WARP_DBG_INF, "%s(): hw_desc_base 0x%x = %pad, hw_cnt_base 0x%x = %u\n",
			__func__, rx_ring->hw_desc_base, &rx_ring->cell[0].alloc_pa,
			rx_ring->hw_cnt_addr, rx_ring_ctrl->ring_len);

		/*WDMA*/
		value = SET_FIELD(WDMA_RX_BASE_PTR_0_RX_BASE_PTR, rx_ring->cell[0].alloc_pa);
		warp_io_write32(wdma, rx_ring->hw_desc_base, value);

		value = SET_FIELD(WDMA_RX_MAX_CNT_0_RX_MAX_CNT, rx_ring_ctrl->ring_len);
		warp_io_write32(wdma, rx_ring->hw_cnt_addr, value);
		warp_io_write32(wdma, rx_ring->hw_cidx_addr, 0);

		/*WED_WDMA*/
		value = SET_FIELD(WED_WDMA_RX0_BASE_PTR, rx_ring->cell[0].alloc_pa);
		warp_io_write32(wed, WED_WDMA_RX0_BASE_ADDR + i * offset, value);

		value = SET_FIELD(WED_WDMA_RX0_CNT_MAX, rx_ring_ctrl->ring_len);
		warp_io_write32(wed, WED_WDMA_RX0_CNT_ADDR + i * offset, value);
	}

	/* Reset Index */
	warp_io_read32(wed, WED_WDMA_RST_IDX_ADDR, &value);
	value |= WED_WDMA_RST_IDX_CRX_IDX0_MASK;
	value |= WED_WDMA_RST_IDX_CRX_IDX1_MASK;
	value |= WED_WDMA_RST_IDX_DRV_IDX0_MASK;
	value |= WED_WDMA_RST_IDX_DRV_IDX1_MASK;
	value |= WED_WDMA_RST_IDX_DRX_IDX_ALL_MASK;
	warp_io_write32(wed, WED_WDMA_RST_IDX_ADDR, value);

	warp_io_read32(wed, WED_WDMA_RST_IDX_ADDR, &value);
	value &= ~WED_WDMA_RST_IDX_CRX_IDX0_MASK;
	value &= ~WED_WDMA_RST_IDX_CRX_IDX1_MASK;
	value &= ~WED_WDMA_RST_IDX_DRV_IDX0_MASK;
	value &= ~WED_WDMA_RST_IDX_DRV_IDX1_MASK;
	value &= ~WED_WDMA_RST_IDX_DRX_IDX_ALL_MASK;
	warp_io_write32(wed, WED_WDMA_RST_IDX_ADDR, value);

	/* Reset Prefetch Index */
	warp_io_read32(wed, WED_WDMA_RX_PREF_CFG_ADDR, &value);
	value |= WED_WDMA_RX_PREF_CFG_RX0_SIDX_CLR_MASK;
	value |= WED_WDMA_RX_PREF_CFG_RX1_SIDX_CLR_MASK;
	warp_io_write32(wed, WED_WDMA_RX_PREF_CFG_ADDR, value);

	warp_io_read32(wed, WED_WDMA_RX_PREF_CFG_ADDR, &value);
	value &= ~WED_WDMA_RX_PREF_CFG_RX0_SIDX_CLR_MASK;
	value &= ~WED_WDMA_RX_PREF_CFG_RX1_SIDX_CLR_MASK;
	warp_io_write32(wed, WED_WDMA_RX_PREF_CFG_ADDR, value);

	/* Reset Prefetch FIFO */
	warp_io_read32(wed, WED_WDMA_RX_PREF_FIFO_CFG_ADDR, &value);
	value |= WED_WDMA_RX_PREF_FIFO_CFG_RING0_CLEAR_MASK;
	value |= WED_WDMA_RX_PREF_FIFO_CFG_RING1_CLEAR_MASK;
	warp_io_write32(wed, WED_WDMA_RX_PREF_FIFO_CFG_ADDR, value);

	warp_io_read32(wed, WED_WDMA_RX_PREF_FIFO_CFG_ADDR, &value);
	value &= ~WED_WDMA_RX_PREF_FIFO_CFG_RING0_CLEAR_MASK;
	value &= ~WED_WDMA_RX_PREF_FIFO_CFG_RING1_CLEAR_MASK;
	warp_io_write32(wed, WED_WDMA_RX_PREF_FIFO_CFG_ADDR, value);
}
#endif
#ifdef WED_HW_RX_SUPPORT
static void
warp_wdma_tx_ring_init_hw(struct wed_entry *wed, struct wdma_entry *wdma)
{
	struct wdma_tx_ring_ctrl *tx_ring_ctrl = &wdma->res_ctrl.tx_ctrl.tx_ring_ctrl;
	struct warp_ring *tx_ring = &tx_ring_ctrl->ring[0];
	u32 value;

	warp_dbg(WARP_DBG_INF, "%s(): wed = 0x%p, wdma = 0x%p\n",
		__func__, wed, wdma);
	warp_dbg(WARP_DBG_INF, "%s(): hw_desc_base 0x%x = %pad, hw_cnt_base 0x%x = %u\n",
		__func__, tx_ring->hw_desc_base, &tx_ring->cell[0].alloc_pa,
		tx_ring->hw_cnt_addr, tx_ring_ctrl->ring_len);

	/*WDMA*/
	value = SET_FIELD(WDMA_TX_BASE_PTR_0_TX_BASE_PTR, tx_ring->cell[0].alloc_pa);
	warp_io_write32(wdma, tx_ring->hw_desc_base, value);

	value = SET_FIELD(WDMA_TX_MAX_CNT_0_TX_MAX_CNT, tx_ring_ctrl->ring_len);
	warp_io_write32(wdma, tx_ring->hw_cnt_addr, value);
	warp_io_write32(wdma, tx_ring->hw_cidx_addr, 0);
	warp_io_write32(wdma, tx_ring->hw_didx_addr, 0);

	/*WED_WDMA*/
	value = SET_FIELD(WED_WDMA_TX0_BASE_PTR, tx_ring->cell[0].alloc_pa);
	warp_io_write32(wed, WED_WDMA_TX0_BASE_ADDR, value);

	value = SET_FIELD(WED_WDMA_TX0_CNT_MAX, tx_ring_ctrl->ring_len);
	warp_io_write32(wed, WED_WDMA_TX0_CNT_ADDR, value);
	warp_io_write32(wed, WED_WDMA_TX0_CTX_IDX_ADDR, 0);
	warp_io_write32(wed, WED_WDMA_TX0_DTX_IDX_ADDR, 0);
}
#endif

void
warp_wdma_ring_init_hw(struct wed_entry *wed, struct wdma_entry *wdma)
{
#ifdef WED_HW_TX_SUPPORT
	warp_wdma_rx_ring_init_hw(wed, wdma);
#endif
#ifdef WED_HW_RX_SUPPORT
	warp_wdma_tx_ring_init_hw(wed, wdma);
#endif
}


#ifdef WED_TX_SUPPORT
int
warp_tx_ring_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	struct wed_tx_ring_ctrl *ring_ctrl = &wed->res_ctrl.tx_ctrl.ring_ctrl;
	struct warp_ring *ring;
	u32 max_ring_num = 2;
	u32 offset = WED_WPDMA_TX1_CTRL0_ADDR - WED_WPDMA_TX0_CTRL0_ADDR;
	u32 value;
	int i;

	if (ring_ctrl->ring_num > max_ring_num) {
		warp_dbg(WARP_DBG_ERR, "%s(): ring_num %u more than %u\n",
			__func__, ring_ctrl->ring_num, max_ring_num);
		return -EINVAL;
	}

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ring = &ring_ctrl->ring[i];

		warp_dbg(WARP_DBG_INF, "%s(): tx ring %d - wed:%p wifi:%p: %x=%lx,%x=%d,%x=%d\n",
			 __func__,
			 i, wed, wifi,
			 ring->hw_desc_base, (unsigned long)ring->cell[0].alloc_pa,
			 ring->hw_cnt_addr, ring->ring_lens,
			 ring->hw_cidx_addr, 0);

		value = SET_FIELD(WED_WPDMA_TX0_CTRL1_MAX_CNT, ring->ring_lens);
#ifdef CONFIG_WARP_64BIT_SUPPORT
		value |= SET_FIELD(WED_WPDMA_TX0_CTRL1_BASE_PTR_H, ring->cell[0].alloc_pa >> 32);
#endif

		/*WPDMA*/
		warp_io_write32(wifi, ring->hw_desc_base, ring->cell[0].alloc_pa);
		warp_io_write32(wifi, ring->hw_cnt_addr, value);
		warp_io_write32(wifi, ring->hw_cidx_addr, 0);
		/*WED_WPDMA*/
		warp_io_write32(wed, WED_WPDMA_TX0_CTRL0_ADDR + i * offset, ring->cell[0].alloc_pa);
		warp_io_write32(wed, WED_WPDMA_TX0_CTRL1_ADDR + i * offset, value);
		warp_io_write32(wed, WED_WPDMA_TX0_CTRL2_ADDR + i * offset, 0);
	}
	return 0;
}
#endif /*WED_TX_SUPPORT*/

#ifdef WED_RX_SUPPORT
int
warp_rx_ring_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value;
	struct wifi_hw *hw = &wifi->hw;

	/*Rx Ring base */
	warp_io_read32(wed, WED_RX0_CTRL0_ADDR, &value);
	warp_io_write32(wed, WED_WPDMA_RX0_CTRL0_ADDR, value);
	warp_io_write32(wifi, hw->event.base, value);
	/*Rx CNT*/
	warp_io_read32(wed, WED_RX0_CTRL1_ADDR, &value);
	warp_io_write32(wed, WED_WPDMA_RX0_CTRL1_ADDR, value);
	warp_io_write32(wifi, hw->event.cnt, value);
	/*cpu idx*/
	warp_io_read32(wed, WED_RX0_CTRL2_ADDR, &value);
	warp_io_write32(wed, WED_WPDMA_RX0_CTRL2_ADDR, value);
	warp_io_write32(wifi, hw->event.cidx, value);
	return 0;
}
#endif

#ifdef WED_RX_D_SUPPORT
int
warp_rx_bm_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value = 0;
	struct wifi_hw *hw = &wifi->hw;
	u32 pkt_num = wed->res_ctrl.rx_ctrl.res.pkt_num;
	struct warp_dma_cb *cb = &wed->res_ctrl.rx_ctrl.res.ring->cell[0];

	if (cb->alloc_pa & 0xFFF) {
		warp_dbg(WARP_DBG_ERR, "%s(): address %pad is not 4KB alignment\n",
			__func__, &cb->alloc_pa);
		return -EINVAL;
	}

	value = SET_FIELD(WED_RX_BM_BASE_PTR, cb->alloc_pa);
	warp_io_write32(wed, WED_RX_BM_BASE_ADDR, value);

	value = SET_FIELD(WED_RX_BM_RX_DMAD_SDL0, hw->rx_pkt_size);
#ifdef CONFIG_WARP_64BIT_SUPPORT
	value |= SET_FIELD(WED_RX_BM_RX_DMAD_BASE_PTR_H, cb->alloc_pa >> 32);
#endif
	warp_io_write32(wed, WED_RX_BM_RX_DMAD_ADDR, value);

	value = SET_FIELD(WED_RX_BM_RANGE_CFG_SW_CFG_BUF_IDX, pkt_num);
	warp_io_write32(wed, WED_RX_BM_RANGE_CFG_ADDR, value);

	value = SET_FIELD(WED_RX_BM_INIT_PTR_SW_TAIL_IDX, pkt_num);
	value |= WED_RX_BM_INIT_PTR_INIT_SW_TAIL_HEAD_IDX_MASK;
	warp_io_write32(wed, WED_RX_BM_INIT_PTR_ADDR, value);

	/* Enable RX_BM to fetch DMAD */
	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value |= WED_CTRL_WED_RX_BM_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);

	return 0;
}

u32 warp_rxbm_left_query(struct wed_entry *wed)
{
	u32 left = 0;
	u32 value = 0, head_idx = 0, tail_idx = 0;
	struct wed_rx_ctrl *rx_ctrl = &wed->res_ctrl.rx_ctrl;

	warp_io_read32(wed, WED_RX_BM_PTR_ADDR, &value);
	head_idx = GET_FIELD(WED_RX_BM_PTR_HEAD_IDX, value);
	tail_idx = GET_FIELD(WED_RX_BM_PTR_TAIL_IDX, value);

	if (tail_idx != head_idx) {
		left = (tail_idx > head_idx) ?
					(tail_idx - head_idx + 1) :
					(rx_ctrl->res.ring_len - head_idx + tail_idx);
	} else
		left = 0;

#ifdef WARP_DVT
	warp_dbg(WARP_DBG_OFF, "%s(): head:0x%04x tail:0x%04x\n", __func__, head_idx, tail_idx);
#endif	/* WARP_DVT */

	return left;
}
#endif

#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
int
warp_rx_rro_data_ring_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	int i;
	u32 value = 0;
	u32 max_ring_num = 2;
	u32 offset = RRO_RX_D_RX1_BASE_ADDR - RRO_RX_D_RX0_BASE_ADDR;
	struct wed_rx_ring_ctrl *ring_ctrl =
			&wed->res_ctrl.rx_ctrl.rro_data_ring_ctrl;
	struct warp_rx_ring *ring;

	if (ring_ctrl->ring_num > max_ring_num) {
		warp_dbg(WARP_DBG_ERR, "%s(): ring_num %u more than %u\n",
			__func__, ring_ctrl->ring_num, max_ring_num);
		return -EINVAL;
	}

	/* WED RRO DRV Ring */
	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ring = &ring_ctrl->ring[i];

		if (ring->cb_alloc_pa & 0xFFF) {
			warp_dbg(WARP_DBG_ERR, "%s(): idx = %d, address %pad is not 4KB alignment\n",
				__func__, i, &ring->cb_alloc_pa);
			return -EINVAL;
		}

		/* desc base */
		value = SET_FIELD(RRO_RX_D_RX0_BASE_PTR, ring->cb_alloc_pa);
		warp_io_write32(wed, RRO_RX_D_RX0_BASE_ADDR + i * offset, value);

		/* max cnt */
		value = SET_FIELD(RRO_RX_D_RX0_CNT_MAX, ring->ring_lens);
#ifdef CONFIG_WARP_64BIT_SUPPORT
		value |= SET_FIELD(RRO_RX_D_RX0_CNT_PTR_H, ring->cb_alloc_pa >> 32);
#endif
		warp_io_write32(wed, RRO_RX_D_RX0_CNT_ADDR + i * offset, value);
	}

	/* reset cidx/didx */
	warp_io_read32(wed, RRO_RX_D_RING_CFG_ADDR_2_ADDR, &value);
	value |= RRO_RX_D_RING_CFG_ADDR_2_DRV_CLR_MASK;
	warp_io_write32(wed, RRO_RX_D_RING_CFG_ADDR_2_ADDR, value);

	return 0;
}

int
warp_rx_route_qm_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wdma_entry *wdma = &warp->wdma;
	u32 value;

	/* Reset RX_ROUTE_QM */
	value = WED_MOD_RST_RX_ROUTE_QM_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);

	/* Configure RX_ROUTE_QM */
	warp_io_read32(wed, WED_RTQM_ENQ_CFG0_ADDR, &value);
	value &= ~WED_RTQM_ENQ_CFG0_TXDMAD_FPORT_MASK;
	value |= SET_FIELD(WED_RTQM_ENQ_CFG0_TXDMAD_FPORT, wdma->wdma_tx_port);
	warp_io_write32(wed, WED_RTQM_ENQ_CFG0_ADDR, value);

	/* Enable RX_ROUTE_QM */
	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value |= WED_CTRL_WED_RX_ROUTE_QM_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);
	return 0;
}

static void
_warp_rtqm_igrs_cnt(struct wed_entry *wed,
			struct seq_file *seq, int i)
{
	u32 value;
	u32 offset = WED_RTQM_IGRS1_I2HW_DMAD_CNT_ADDR -
		     WED_RTQM_IGRS0_I2HW_DMAD_CNT_ADDR;
	char igrs_type[4][10] = { "RTQM_QUE", "WO", "RRO", "DBG" };
	char igrs_dest[4][10] = { "NETSYS_HW", "RTQM_QUE", "RTQM_QUE", "RTQM_QUE" };

	dump_string(seq, "=======WED RTQM IGRS%d (From %s)=========\n", i, igrs_type[i]);
	warp_io_read32(wed, WED_RTQM_IGRS0_I2HW_DMAD_CNT_ADDR + i * offset,
			&value);
	seq_printf(seq, "To %s DMAD counter\t: 0x%x\n", igrs_dest[i], value);

	warp_io_read32(wed, WED_RTQM_IGRS0_I2H0_DMAD_CNT_ADDR + i * offset,
			&value);
	seq_printf(seq, "To Host Ring0 DMAD counter\t: 0x%x\n", value);

	warp_io_read32(wed, WED_RTQM_IGRS0_I2H1_DMAD_CNT_ADDR + i * offset,
			&value);
	seq_printf(seq, "To Host Ring1 DMAD counter\t: 0x%x\n", value);

	warp_io_read32(wed, WED_RTQM_IGRS0_I2HW_PKT_CNT_ADDR + i * offset,
			&value);
	seq_printf(seq, "To %s packet counter\t: 0x%x\n", igrs_dest[i], value);

	warp_io_read32(wed, WED_RTQM_IGRS0_I2H0_PKT_CNT_ADDR + i * offset,
			&value);
	seq_printf(seq, "To Host Ring0 packet counter\t: 0x%x\n", value);

	warp_io_read32(wed, WED_RTQM_IGRS0_I2H1_PKT_CNT_ADDR + i * offset,
			&value);
	seq_printf(seq, "To Host Ring1 packet counter\t: 0x%x\n", value);

	warp_io_read32(wed, WED_RTQM_IGRS0_FDROP_CNT_ADDR + i * offset,
			&value);
	seq_printf(seq, "Force Drop DMAD counter\t\t: 0x%x\n", value);
}

static void
warp_rtqm_igrs_cnt(struct wed_entry *wed, struct seq_file *seq)
{
	int igrs_num = 4;
	int i;

	for (i = 0; i < igrs_num; i++)
		_warp_rtqm_igrs_cnt(wed, seq, i);
}

static void
warp_rtqm_enq_cnt(struct wed_entry *wed, struct seq_file *seq)
{
	u32 value;

	dump_string(seq, "=======WED RTQM ENQ============\n");
	dump_io(seq, wed, "Enqueue to SRAM DMAD counter",
		WED_RTQM_ENQ_I2Q_DMAD_CNT_ADDR);
	dump_io(seq, wed, "Enqueue to NETSYS DMAD counter",
		WED_RTQM_ENQ_I2N_DMAD_CNT_ADDR);
	dump_io(seq, wed, "Enqueue to SRAM packet counter",
		WED_RTQM_ENQ_I2Q_PKT_CNT_ADDR);
	dump_io(seq, wed, "Enqueue to NETSYS pkt counter",
		WED_RTQM_ENQ_I2N_PKT_CNT_ADDR);

	warp_io_read32(wed, WED_RTQM_ENQ_USED_ENTRY_CNT_ADDR, &value);
	seq_printf(seq, "Enqueue HW used entry counter\t: 0x%x (prefetch use 1)\n",
		value);

	dump_io(seq, wed, "Enqueue error DMAD counter",
		WED_RTQM_ENQ_ERR_CNT_ADDR);
}

static void
warp_rtqm_deq_cnt(struct wed_entry *wed, struct seq_file *seq)
{
	dump_string(seq, "=======WED RTQM DEQ============\n");
	dump_io(seq, wed, "Dequeue from SRAM DMAD counter",
		WED_RTQM_DEQ_DQ_DMAD_CNT_ADDR);
	dump_io(seq, wed, "Dequeue from IGRS DMAD counter",
		WED_RTQM_DEQ_Q2I_DMAD_CNT_ADDR);
	dump_io(seq, wed, "Dequeue from SRAM pkt counter",
		WED_RTQM_DEQ_DQ_PKT_CNT_ADDR);
	dump_io(seq, wed, "Dequeue from IGRS pkt counter",
		WED_RTQM_DEQ_Q2I_PKT_CNT_ADDR);
	dump_io(seq, wed, "Dequeue HW used entry counter",
		WED_RTQM_DEQ_USED_PFDBK_CNT_ADDR);
	dump_io(seq, wed, "Dequeue error DMAD counter",
		WED_RTQM_DEQ_ERR_CNT_ADDR);
}

static void
warp_rtqm_que_cnt(struct wed_entry *wed, struct seq_file *seq)
{
	u32 value;

	dump_string(seq, "=======WED RTQM QUE============\n");
	dump_io(seq, wed, "Inqueue HW used entry number",
		WED_RTQM_QUEUE_CNT_ADDR);

	warp_io_read32(wed, WED_RTQM_PFDBK_FIFO_CFG_ADDR, &value);
	value = GET_FIELD(WED_RTQM_PFDBK_FIFO_CFG_P2Q_CNT, value);
	seq_printf(seq, "Inqueue PPE FDBK entry number\t: 0x%x\n", value);
}

int warp_procinfo_dump_rtqm_hw(struct warp_entry *warp, struct seq_file *seq)
{
	struct wed_entry *wed = &warp->wed;

	warp_rtqm_igrs_cnt(wed, seq);
	warp_rtqm_enq_cnt(wed, seq);
	warp_rtqm_deq_cnt(wed, seq);
	warp_rtqm_que_cnt(wed, seq);

	return 0;
}
#endif

#ifdef WED_RX_HW_RRO_3_0
int
warp_rx_rro_page_ring_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	int i;
	u32 value = 0;
	u32 max_ring_num = 3;
	u32 offset = RRO_MSDU_PG_1_CTRL0_ADDR - RRO_MSDU_PG_0_CTRL0_ADDR;
	struct wed_rx_ring_ctrl *ring_ctrl =
			&wed->res_ctrl.rx_ctrl.rro_page_ring_ctrl;
	struct warp_rx_ring *ring;

	if (ring_ctrl->ring_num > max_ring_num) {
		warp_dbg(WARP_DBG_ERR, "%s(): ring_num %u more than %u\n",
			__func__, ring_ctrl->ring_num, max_ring_num);
		return -EINVAL;
	}

	/* WFDMA RRO Page DRV Ring */
	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ring = &ring_ctrl->ring[i];

		if (ring->cb_alloc_pa & 0xFFF) {
			warp_dbg(WARP_DBG_ERR, "%s(): idx = %d, address %pad is not 4KB alignment\n",
				__func__, i, &ring->cb_alloc_pa);
			return -EINVAL;
		}

		/* desc base */
		value = SET_FIELD(RRO_MSDU_PG_0_CTRL0_BASE_PTR_L, ring->cb_alloc_pa);
		warp_io_write32(wed, RRO_MSDU_PG_0_CTRL0_ADDR + i * offset, value);

		/* max cnt */
		value = SET_FIELD(RRO_MSDU_PG_0_CTRL1_MAX_CNT, ring->ring_lens);
#ifdef CONFIG_WARP_64BIT_SUPPORT
		value |= SET_FIELD(RRO_MSDU_PG_0_CTRL1_BASE_PTR_M, ring->cb_alloc_pa >> 32);
#endif
		warp_io_write32(wed, RRO_MSDU_PG_0_CTRL1_ADDR + i * offset, value);
	}

	/* reset cidx/didx */
	warp_io_read32(wed, RRO_MSDU_PG_RING2_CFG1_ADDR, &value);
	value |= RRO_MSDU_PG_RING2_CFG1_DRV_CLR_MASK;
	warp_io_write32(wed, RRO_MSDU_PG_RING2_CFG1_ADDR, value);

	return 0;
}

static int
warp_rx_ind_cmd_conf_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	int i = 0, j = 0;
	u32 value = 0, win_sz = 32, max_win_sz = 1024;
	u32 max_ring_num = 1;
	struct wed_rx_ring_ctrl *ring_ctrl =
			&wed->res_ctrl.rx_ctrl.rro_ind_cmd_ring_ctrl;
	struct wifi_hw *hw = &wifi->hw;
	struct warp_rx_ring *ring;
	struct rro_ctrl *rro_ctl = &hw->rro_ctl;

	if (ring_ctrl->ring_num > max_ring_num) {
		warp_dbg(WARP_DBG_ERR, "%s(): ring_num %u more than %u\n",
			__func__, ring_ctrl->ring_num, max_ring_num);
		return -EINVAL;
	}

	/* WED Ind Cmd Ring */
	ring = &ring_ctrl->ring[i];

	if (ring->cb_alloc_pa & 0xF) {
		warp_dbg(WARP_DBG_ERR, "%s(): address %pad is not 16B alignment\n",
			 __func__, &ring->cb_alloc_pa);
		return -EINVAL;
	}
	if (ring->ring_lens & 0xF) {
		warp_dbg(WARP_DBG_ERR, "%s(): ring_len %u is not multiples of 16\n",
			 __func__, ring->ring_lens);
		return -EINVAL;
	}

	/* desc base */
	value = SET_FIELD(IND_CMD_0_CTRL_1_RRO_IND_CMD_BASE_L, ring->cb_alloc_pa >>
			IND_CMD_0_CTRL_1_RRO_IND_CMD_BASE_L_SHFT);
	warp_io_write32(wed, IND_CMD_0_CTRL_1_ADDR, value);

	/* max cnt */
	value = SET_FIELD(IND_CMD_0_CTRL_2_MAX_CNT_11_4, ring->ring_lens >>
			IND_CMD_0_CTRL_2_MAX_CNT_11_4_SHFT);
#ifdef CONFIG_WARP_64BIT_SUPPORT
	value |= SET_FIELD(IND_CMD_0_CTRL_2_RRO_IND_CMD_BASE_M, ring->cb_alloc_pa >> 32);
#endif
	warp_io_write32(wed, IND_CMD_0_CTRL_2_ADDR, value);

	/* rro conf */
	/* ack sn cr */
	if (IS_WED_HW_CAP(wed, WED_HW_CAP_RRO_DBG_MEMCPY_MODE)) {
		/* Unused CR in Connsys RRO */
		warp_io_write32(wed, RRO_CONF_0_ADDR, hw->base_phy_addr + 0xDA130);

		warp_dbg(WARP_DBG_INF, "%s(): WED write ACK_SN to 0xDA130 in Connsys\n",
			 __func__);
	} else {
		warp_io_write32(wed, RRO_CONF_0_ADDR, hw->base_phy_addr + rro_ctl->ack_sn_cr);
	}

	value = 0xFF;
	while (win_sz <= max_win_sz) {
		/* 32/64/128/256/512/1024 win_sz */
		if (rro_ctl->max_win_sz == win_sz) {
			value = j;
			break;
		}
		win_sz <<= 1;
		j++;
	}

	if (value == 0xFF) {
		warp_dbg(WARP_DBG_ERR, "%s(): RRO winsize %u isn't compatibility to WED\n",
			 __func__, rro_ctl->max_win_sz);
		return -EINVAL;
	}

	/* max win sz & particular_se_id */
	value = SET_FIELD(RRO_CONF_1_MAX_WIN_SZ, value);
	value |= SET_FIELD(RRO_CONF_1_PARTICULAR_SE_ID, rro_ctl->particular_se_id);
#ifdef CONFIG_WARP_64BIT_SUPPORT
	value |= SET_FIELD(RRO_CONF_1_ACK_SN_BASE_0_M, hw->base_phy_addr >> 32);
#endif
	warp_io_write32(wed, RRO_CONF_1_ADDR, value);

	return 0;
}

static int
warp_rx_addr_elem_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	int i;
	u32 value = 0;
	struct wifi_hw *hw = &wifi->hw;
	struct rro_ctrl *rro_ctl = &hw->rro_ctl;

	/* particular session addr element */
	value = SET_FIELD(ADDR_ELEM_CONF_0_PARTICULAR_SE_ID_ADDR_BASE_L,
			rro_ctl->particular_se_base);
	warp_io_write32(wed, ADDR_ELEM_CONF_0_ADDR, value);

#ifdef CONFIG_WARP_64BIT_SUPPORT
	warp_io_read32(wed, ADDR_ELEM_CONF_1_ADDR, &value);
	value &= ~ADDR_ELEM_CONF_1_PARTICULAR_SE_ID_ADDR_BASE_M_MASK;
	value |= SET_FIELD(ADDR_ELEM_CONF_1_PARTICULAR_SE_ID_ADDR_BASE_M,
			rro_ctl->particular_se_base >> 32);
	warp_io_write32(wed, ADDR_ELEM_CONF_1_ADDR, value);
#endif

	/* ba session addr element */
	for (i = 0; i < rro_ctl->se_group_num; i++) {

		if (rro_ctl->se_base[i] & 0xF) {
			warp_dbg(WARP_DBG_ERR, "%s(): idx = %d, address %pad is not 16B alignment\n",
				__func__, i, &rro_ctl->se_base[i]);
			return -EINVAL;
		}

		value = SET_FIELD(ADDR_ELEM_BASE_TBL_WDATA_W_DATA, rro_ctl->se_base[i] >> 4);
		warp_io_write32(wed, ADDR_ELEM_BASE_TBL_WDATA_ADDR, value);

		/* session group id */
		value = SET_FIELD(ADDR_ELEM_BASE_TBL_CONF_BASE_TBL_OFST, i);
		value |= ADDR_ELEM_BASE_TBL_CONF_WR_MASK;
		warp_io_write32(wed, ADDR_ELEM_BASE_TBL_CONF_ADDR, value);

		/* check if write done */
		value = ADDR_ELEM_BASE_TBL_CONF_WR_RDY_MASK;
		if (wed_rdy_chk(wed, ADDR_ELEM_BASE_TBL_CONF_ADDR, value)) {
			warp_dbg(WARP_DBG_ERR, "%s(): idx = %d, write addr_elem failed!\n",
				__func__, i);
			return -EBUSY;
		}
	}

	return 0;
}

static int
warp_rx_pn_chk_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value = 0;
	u32 session_id = 0;

	for (session_id = 0; session_id < 1024; session_id++) {
		value = PN_CONF_WDATA_M_IS_FIRST_MASK;
		warp_io_write32(wed, PN_CONF_WDATA_M_ADDR, value);

		value = SET_FIELD(PN_CONF_0_SE_ID, session_id);
		value |= PN_CONF_0_PN_WR_MASK;
		warp_io_write32(wed, PN_CONF_0_ADDR, value);

		value = PN_CONF_0_PN_WR_RDY_MASK;
		if (wed_rdy_chk(wed, PN_CONF_0_ADDR, value)) {
			warp_dbg(WARP_DBG_ERR, "%s(): seid = %u failed to init!\n",
				__func__, session_id);
			return -EBUSY;
		}
	}

	return 0;
}

int
_warp_rx_pn_chk_set_hw(struct wed_entry *wed, u32 se_id, bool pn_chk)
{
	u32 value;

	if (se_id > 1024) {
		warp_dbg(WARP_DBG_ERR, "%s(): se_id = %u invalid\n", __func__, se_id);
		return -EINVAL;
	}

	value = PN_CONF_WDATA_M_IS_FIRST_MASK;
	value |= SET_FIELD(PN_CONF_WDATA_M_NEED_CHECK_PN,
			   pn_chk);
	warp_io_write32(wed, PN_CONF_WDATA_M_ADDR, value);

	value = SET_FIELD(PN_CONF_0_SE_ID, se_id);
	value |= PN_CONF_0_PN_WR_MASK;
	warp_io_write32(wed, PN_CONF_0_ADDR, value);

	if (wed_rdy_chk(wed, PN_CONF_0_ADDR, PN_CONF_0_PN_WR_RDY_MASK)) {
		warp_dbg(WARP_DBG_ERR, "%s(): se_id = %u failed to set\n", __func__, se_id);
		return -EIO;
	}

	return 0;
}

int
_warp_rx_pn_chk_get_hw(struct wed_entry *wed, u32 se_id, bool *pn_chk, u64 *pn)
{
	u32 value;

	if (se_id > 1024) {
		warp_dbg(WARP_DBG_ERR, "%s(): se_id = %u invalid\n", __func__, se_id);
		return -EINVAL;
	}

	value = SET_FIELD(PN_CONF_0_SE_ID, se_id);
	value |= PN_CONF_0_PN_RD_MASK;
	warp_io_write32(wed, PN_CONF_0_ADDR, value);

	if (wed_rdy_chk(wed, PN_CONF_0_ADDR, PN_CONF_0_PN_RD_RDY_MASK)) {
		warp_dbg(WARP_DBG_ERR, "%s(): se_id = %u failed to read\n", __func__, se_id);
		return -EIO;
	}

	warp_io_read32(wed, PN_CONF_RDATA_L_ADDR, &value);
	*pn = GET_FIELD(PN_CONF_RDATA_L_PN_31_0, value);

	warp_io_read32(wed, PN_CONF_RDATA_M_ADDR, &value);
	*pn_chk = GET_FIELD(PN_CONF_RDATA_M_NEED_CHECK_PN, value);
	*pn |= (u64)GET_FIELD(PN_CONF_RDATA_M_PN_47_32, value) << 32;

	return 0;
}

int
warp_rx_pn_chk_set_hw(struct wed_entry *wed, u32 se_id, bool pn_chk)
{
	u64 pn;
	bool pn_chk_read;
	int ret, i = 0, count = 3;

set_pn_chk:
	ret = _warp_rx_pn_chk_set_hw(wed, se_id, pn_chk);
	if (ret)
		goto end;

	ret = _warp_rx_pn_chk_get_hw(wed, se_id, &pn_chk_read, &pn);
	if (ret)
		goto end;

	ret = pn_chk ^ pn_chk_read;
	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): Set PN_CHK %s for se_id = %u failed\n",
			__func__, pn_chk ? "on" : "off", se_id);
end:
	if (ret && i < count) {
		i++;
		goto set_pn_chk;
	}

	return ret;
}

int warp_rx_pn_chk_get_hw(struct wed_entry *wed, u16 se_id)
{
	bool pn_chk;
	u64 pn;
	int ret;

	ret = _warp_rx_pn_chk_get_hw(wed, se_id, &pn_chk, &pn);
	if (ret)
		goto end;

	warp_dbg(WARP_DBG_OFF, "%s(): se_id = %u, pn_chk = %s, pn = 0x%llx\n",
		__func__, se_id, pn_chk ? "true" : "false", pn);

end:
	return ret;
}

int
warp_rx_particular_to_host_set_hw(struct wed_entry *wed, bool to_host)
{
	u32 value;

	warp_io_read32(wed, PN_CONF_1_ADDR, &value);

	if (to_host)
		value |= PN_CONF_1_paticular_id_force_to_host_en_MASK;
	else
		value &= ~PN_CONF_1_paticular_id_force_to_host_en_MASK;

	warp_io_write32(wed, PN_CONF_1_ADDR, value);

	return 0;
}

static inline void
warp_rx_ind_cmd_set_sign_base_addr(struct wed_entry *wed, struct wifi_entry *wifi)
{
	struct wifi_hw *hw = &wifi->hw;

	/* fill sign_base_addr */
	hw->sign_base_cr = wed->cr_base_addr + RRO_IND_CMD_0_SIGNATURE_ADDR;
#ifdef WED_RX_HW_RRO_3_0_DBG
	hw->sign_base_cr = wed->cr_base_addr + WED_SCR7_ADDR;
#endif
}

int
warp_rx_ind_cmd_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value;

	warp_rx_ind_cmd_conf_init_hw(wed, wifi);
	warp_rx_addr_elem_init_hw(wed, wifi);
	warp_rx_pn_chk_init_hw(wed, wifi);
	warp_rx_ind_cmd_set_sign_base_addr(wed, wifi);

	warp_io_read32(wed, WED_RX_IND_CMD_CNT0_ADDR, &value);
	value = WED_RX_IND_CMD_CNT0_dbg_cnt_en_MASK;
	warp_io_write32(wed, WED_RX_IND_CMD_CNT0_ADDR, value);

	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value |= WED_CTRL_WED_RX_IND_CMD_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);

	return 0;
}

int
warp_rx_ind_cmd_exit_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value;

	/* Disable Indicate cmd handle */
	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value &= ~WED_CTRL_WED_RX_IND_CMD_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);

	if (wed_busy_chk(wed, WED_RRO_RX_HW_STS_ADDR, GENMASK(31, 0)))
		warp_dbg(WARP_DBG_ERR, "%s(): IND_CMD disable failed\n", __func__);

	/* Reset IND_CMD/INFO_PAGE/PN_CHK */
	value = WED_MOD_RST_RRO_RX_TO_PG_MASK;
	WHNAT_RESET(wed, WED_MOD_RST_ADDR, value);

	return 0;
}

int
warp_rx_page_bm_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value = 0;
	struct wifi_hw *hw = &wifi->hw;
	u32 pg_nums = wed->res_ctrl.rx_ctrl.page_bm.page_num;
	struct warp_dma_cb *cb = &wed->res_ctrl.rx_ctrl.page_bm.ring->cell[0];

	if (cb->alloc_pa & 0xFFF) {
		warp_dbg(WARP_DBG_ERR, "%s(): address %pad is not 4KB alignment\n",
			__func__, &cb->alloc_pa);
		return -EINVAL;
	}

	value = SET_FIELD(RRO_PG_BM_BASE_PTR, cb->alloc_pa);
	warp_io_write32(wed, RRO_PG_BM_BASE_ADDR, value);

	value = SET_FIELD(RRO_PG_BM_RX_DMAD_SDL0, hw->rx_page_size);
#ifdef CONFIG_WARP_64BIT_SUPPORT
	value |= SET_FIELD(RRO_PG_BM_RX_DMAD_BASE_PTR_H, cb->alloc_pa >> 32);
#endif
	warp_io_write32(wed, RRO_PG_BM_RX_DMAD_ADDR, value);

	value = SET_FIELD(RRO_PG_BM_RANGE_CFG_SW_CFG_BUF_IDX, pg_nums);
	warp_io_write32(wed, RRO_PG_BM_RANGE_CFG_ADDR, value);

	value = SET_FIELD(RRO_PG_BM_INIT_PTR_SW_TAIL_IDX, pg_nums);
	value |= RRO_PG_BM_INIT_PTR_INIT_SW_TAIL_HEAD_IDX_MASK;
	warp_io_write32(wed, RRO_PG_BM_INIT_PTR_ADDR, value);

	/* Enable RX_PG_BM to fetch DMAD */
	warp_io_read32(wed, WED_CTRL_ADDR, &value);
	value |= WED_CTRL_WED_RX_PG_BM_EN_MASK;
	warp_io_write32(wed, WED_CTRL_ADDR, value);

	return 0;
}

int
warp_rro_write_dma_idx_hw(struct wed_entry *wed,
			  struct wifi_entry *wifi,
			  u32 val)
{
	warp_io_write32(wed, RRO_IND_CMD_0_SIGNATURE_ADDR, val);
	return 0;
}

void warp_rro_ind_cmd_cnt(struct wed_entry *wed, struct seq_file *seq)
{
	int ind_cmd_fetch_num = 8;
	int pg_cnt_num = 5;
	u32 value;
	int i;

	dump_string(seq, "=======WED RRO IND_CMD=========\n");
	warp_io_read32(wed, WED_RX_IND_CMD_CNT0_ADDR, &value);
	seq_printf(seq, "total_ind_cmd_cnt\t: 0x%x\n",
		GET_FIELD(WED_RX_IND_CMD_CNT0_total_ind_cmd_cnt, value));

	for (i = 0; i < ind_cmd_fetch_num; i++) {
		u32 offset = WED_RX_IND_CMD_CNT2_ADDR - WED_RX_IND_CMD_CNT1_ADDR;

		warp_io_read32(wed, WED_RX_IND_CMD_CNT1_ADDR + i * offset, &value);
		seq_printf(seq, "ind_cmd_fetch%u_cnt\t: 0x%x\n", i + 1,
			GET_FIELD(WED_RX_IND_CMD_CNT1_ind_cmd_fetch1_cnt, value));
	}

	warp_io_read32(wed, WED_RX_IND_CMD_CNT9_ADDR, &value);
	seq_printf(seq, "magic_cnt_fail_cnt\t: 0x%x\n",
		GET_FIELD(WED_RX_IND_CMD_CNT9_magic_cnt_fail_cnt, value));

	warp_io_read32(wed, WED_RX_ADDR_ELEM_CNT0_ADDR, &value);
	seq_printf(seq, "total_addr_elem_cnt\t: 0x%x\n",
		GET_FIELD(WED_RX_ADDR_ELEM_CNT0_total_addr_elem_cnt, value));

	warp_io_read32(wed, WED_RX_ADDR_ELEM_CNT1_ADDR, &value);
	seq_printf(seq, "total_1st_sig_fail_cnt\t: 0x%x\n",
		GET_FIELD(WED_RX_ADDR_ELEM_CNT1_total_1st_sig_fail_cnt, value));
	seq_printf(seq, "total_sig_fail_cnt\t: 0x%x\n",
		GET_FIELD(WED_RX_ADDR_ELEM_CNT1_total_sig_fail_cnt, value));

	warp_io_read32(wed, WED_RX_ADDR_ELEM_CNT2_ADDR, &value);
	seq_printf(seq, "clr_addr_cnt\t: 0x%x\n",
		GET_FIELD(WED_RX_ADDR_ELEM_CNT2_clr_addr_cnt, value));

	warp_io_read32(wed, WED_RX_ADDR_ELEM_CNT3_ADDR, &value);
	seq_printf(seq, "acksn_cnt\t: 0x%x\n",
		GET_FIELD(WED_RX_ADDR_ELEM_CNT3_acksn_cnt, value));

	for (i = 0; i < pg_cnt_num; i++) {
		u32 offset = WED_RX_MSDU_PG_CNT2_ADDR - WED_RX_MSDU_PG_CNT1_ADDR;

		warp_io_read32(wed, WED_RX_MSDU_PG_CNT1_ADDR + i * offset, &value);
		seq_printf(seq, "pg_cnt%u_cnt\t: 0x%x\n", i + 1,
			GET_FIELD(WED_RX_MSDU_PG_CNT1_pg_cnt1_cnt, value));
	}

	warp_io_read32(wed, WED_RX_PN_CHK_CNT0_ADDR, &value);
	seq_printf(seq, "pn_chk_fail_cnt\t: 0x%x\n",
		GET_FIELD(WED_RX_PN_CHK_CNT0_pn_chk_fail_cnt, value));
}

int warp_procinfo_dump_rro_hw(struct warp_entry *warp, struct seq_file *seq)
{
	struct wed_entry *wed = &warp->wed;

	warp_rro_ind_cmd_cnt(wed, seq);
	return 0;
}
#endif

#ifdef WED_RX_HW_RRO_3_1
int
warp_rx_rro_3_1_ring_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	int i = 0;
	u32 value = 0;
	u32 max_ring_num = 1;
	struct wed_rx_ring_ctrl *ring_ctrl =
			&wed->res_ctrl.rx_ctrl.rro_3_1_ring_ctrl;
	struct warp_rx_ring *ring;

	if (ring_ctrl->ring_num > max_ring_num) {
		warp_dbg(WARP_DBG_ERR, "%s(): ring_num %u more than %u\n",
			__func__, ring_ctrl->ring_num, max_ring_num);
		return -EINVAL;
	}

	/* WED RRO 3.1 DRV Ring */
	ring = &ring_ctrl->ring[i];

	if (ring->ring_lens & 0x1) {
		warp_dbg(WARP_DBG_ERR, "%s(): ring_len %u is not multiples of 2\n",
			 __func__, ring->ring_lens);
		return -EINVAL;
	}

	/* desc base */
	value = SET_FIELD(WED_WPDMA_RRO3_1_RX_D_RX0_BASE_PTR, ring->cb_alloc_pa);
	warp_io_write32(wed, WED_WPDMA_RRO3_1_RX_D_RX0_BASE_ADDR, value);

	/* max cnt */
	value = SET_FIELD(WED_WPDMA_RRO3_1_RX_D_RX0_CNT_MAX, ring->ring_lens);
#ifdef CONFIG_WARP_64BIT_SUPPORT
	value |= SET_FIELD(WED_WPDMA_RRO3_1_RX_D_RX0_CNT_BASE_PTR_H, ring->cb_alloc_pa >> 32);
#endif
	warp_io_write32(wed, WED_WPDMA_RRO3_1_RX_D_RX0_CNT_ADDR, value);

	return 0;
}

int
warp_rx_rro_3_1_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value;

	/* Set WED_RRO use RRO 3.1 */
	value = WED_RRO_CTL_RRO_VER_MASK;
	warp_io_write32(wed, WED_RRO_CTL_ADDR, value);

	/* Enable RRO 3.1 prefetch */
	warp_io_read32(wed, WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR, &value);
	value |= WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ENABLE_MASK;
	warp_io_write32(wed, WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR, value);

	return 0;
}

void
warp_rx_rro_3_1_exit_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value;

	/* Disable RRO 3.1 prefetch */
	warp_io_read32(wed, WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR, &value);
	value &= ~WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ENABLE_MASK;
	warp_io_write32(wed, WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR, value);

	/* Clear WED_RRO use RRO 3.1 */
	warp_io_write32(wed, WED_RRO_CTL_ADDR, 0);
}
#endif

static int
warp_hif_txd_init_base(struct wed_entry *wed)
{

	struct wed_hif_txd_ctrl *hif_txd_ctrl = &wed->res_ctrl.tx_ctrl.hif_txd_ctrl;
	u32 offset = WED_HIFTXD_BASE01_L_ADDR - WED_HIFTXD_BASE00_L_ADDR;
	u32 shft_offset = WED_HIFTXD_BASE_00_07_H_BASE00_SHFT -
			WED_HIFTXD_BASE_00_07_H_BASE01_SHFT;
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

		value = SET_FIELD(WED_HIFTXD_BASE00_L_PTR, hif_txd_pa);
		warp_io_write32(wed, WED_HIFTXD_BASE00_L_ADDR + i * offset, value);

#ifdef CONFIG_WARP_64BIT_SUPPORT
		warp_io_read32(wed, WED_HIFTXD_BASE_00_07_H_ADDR + ((i >> 3) * offset),
			&value);

		/* Each BASE_H CR need to clear for first setup */
		value = ((i & 0x7) == 0) ? 0 : value;
		value |= ((hif_txd_pa >> 32) & WED_HIFTXD_BASE_00_07_H_BASE07_MASK) <<
			(WED_HIFTXD_BASE_00_07_H_BASE00_SHFT - ((i & 0x7) * shft_offset));
		warp_io_write32(wed, WED_HIFTXD_BASE_00_07_H_ADDR + ((i >> 3) * offset),
			value);
#endif
	}

	return 0;
}

int
hif_txd_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	return warp_hif_txd_init_base(wed);
}

int
warp_reset_hw(struct wed_entry *wed, u32 reset_type)
{
	int ret = 0;

	switch (reset_type) {
	case WARP_RESET_IDX_ONLY:
	case WARP_RESET_INTERFACE:
		/* TX data offload path reset */
		if (reset_tx_traffic(wed, reset_type)) {
			warp_dbg(WARP_DBG_ERR, "%s():reset_tx_traffic fail\n", __func__);
			ret = -EIO;
		}

		/* TX free done reset - rx path */
		if (reset_rx_traffic(wed, reset_type))
			warp_dbg(WARP_DBG_ERR, "%s():reset_rx_traffic fail\n", __func__);

		if (reset_rx_rro_traffic(wed, reset_type)) {
			warp_dbg(WARP_DBG_ERR, "%s():reset_rx_rro_traffic fail\n", __func__);
			ret = -EIO;
		}

		break;
	default:
		warp_dbg(WARP_DBG_ERR, "%s(): Unknown reset_type = %u", __func__, reset_type);
	}

	return ret;
}

int
warp_stop_hw(struct wed_entry *wed, struct wdma_entry *wdma, u8 stop_type)
{
	int ret = 0;

	switch (stop_type) {
	case WARP_STOP_RX_TRAFFIC:
		ret = stop_rx_traffic(wed, wdma);
		if (ret)
			goto end;

		break;
	case WARP_STOP_TX_TRAFFIC:
		ret = stop_tx_traffic(wed, wdma);
		if (ret)
			goto end;

		break;
	case WARP_STOP_TXRX_TRAFFIC:
		ret = stop_rx_traffic(wed, wdma);
		if (ret)
			goto end;

		ret = stop_tx_traffic(wed, wdma);
		if (ret)
			goto end;

		break;
	default:
		warp_dbg(WARP_DBG_ERR, "%s(): Unknown type = 0x%x\n",
			__func__, stop_type);
	}

end:
	return ret;
}

void
warp_restore_hw(struct wed_entry *wed, struct wdma_entry *wdma, u8 restore_type)
{
	switch (restore_type) {
	case WARP_RESTORE_RX_TRAFFIC:
		restore_rx_traffic(wed, wdma);
		break;
	case WARP_RESTORE_TX_TRAFFIC:
		restore_tx_traffic(wed, wdma);
		break;
	case WARP_RESTORE_TXRX_TRAFFIC:
		restore_rx_traffic(wed, wdma);
		restore_tx_traffic(wed, wdma);
		break;
	default:
		warp_dbg(WARP_DBG_ERR, "%s(): Unknown type = 0x%x\n",
			__func__, restore_type);
	}
}

void
warp_wifi_set_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	struct wifi_hw *hw = &wifi->hw;
#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	bool wifi_cap_rro_3_0 = hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_0);
	bool wifi_cap_rro_3_1 = hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_1);
#endif

	/* Interrupt status and mask */
	warp_io_write32(wed, WED_WPDMA_CFG_ADDR_INTS_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->int_sta);
	warp_io_write32(wed, WED_WPDMA_CFG_ADDR_INTM_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->int_mask);

	/* Tx data ring */
	warp_io_write32(wed, WED_WPDMA_CFG_CIDX_ADDR_TX0_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->tx[0].cidx);
	warp_io_write32(wed, WED_WPDMA_CFG_DIDX_ADDR_TX0_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->tx[0].didx);
	warp_io_write32(wed, WED_WPDMA_CFG_CIDX_ADDR_TX1_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->tx[1].cidx);
	warp_io_write32(wed, WED_WPDMA_CFG_DIDX_ADDR_TX1_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->tx[1].didx);

	/* Tx free done event ring */
	warp_io_write32(wed, WED_WPDMA_CFG_CIDX_ADDR_TX0_FREE_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->event.cidx);
	warp_io_write32(wed, WED_WPDMA_CFG_DIDX_ADDR_TX0_FREE_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->event.didx);

#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	if (wifi_cap_rro_3_0 || wifi_cap_rro_3_1) {
		/* GLO_CFG */
		warp_io_write32(wed, WED_WPDMA_RRO3_1_RX_D_GLO_CFG_BASE_ADDR_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->rx_dma_glo_cfg);

		/* RRO data ring */
		warp_io_write32(wed, RRO_RX_D_RING_CFG_CIDX_ADDR_0_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->rx_rro[0].cidx);
		warp_io_write32(wed, RRO_RX_D_RING_CFG_DIDX_ADDR_0_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->rx_rro[0].didx);
		warp_io_write32(wed, RRO_RX_D_RING_CFG_CIDX_ADDR_1_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->rx_rro[1].cidx);
		warp_io_write32(wed, RRO_RX_D_RING_CFG_DIDX_ADDR_1_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->rx_rro[1].didx);
	}
#endif
#ifdef WED_RX_HW_RRO_3_0
	if (wifi_cap_rro_3_0) {

		/* RRO MSDU_PG ring */
		warp_io_write32(wed, RRO_MSDU_PG_RING0_CIDX_CFG0_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->rx_rro_pg[0].cidx);
		warp_io_write32(wed, RRO_MSDU_PG_RING0_DIDX_CFG0_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->rx_rro_pg[0].didx);
		warp_io_write32(wed, RRO_MSDU_PG_RING1_CIDX_CFG0_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->rx_rro_pg[1].cidx);
		warp_io_write32(wed, RRO_MSDU_PG_RING1_DIDX_CFG0_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->rx_rro_pg[1].didx);
		warp_io_write32(wed, RRO_MSDU_PG_RING2_CIDX_CFG0_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->rx_rro_pg[2].cidx);
		warp_io_write32(wed, RRO_MSDU_PG_RING2_DIDX_CFG0_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->rx_rro_pg[2].didx);
	}
#endif
#ifdef WED_RX_HW_RRO_3_1
	if (wifi_cap_rro_3_1) {
		/* RRO 3.1 ring */
		warp_io_write32(wed, WED_WPDMA_RRO3_1_RX_D_RING0_CFG_CIDX_ADDR_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->rx_rro_3_1.cidx);
		warp_io_write32(wed, WED_WPDMA_RRO3_1_RX_D_RING0_CFG_DIDX_ADDR_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->rx_rro_3_1.didx);
	}
#endif
}

void
warp_mtable_build_hw(struct warp_entry *warp)
{
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	struct warp_wifi_cr_map *m = &warp->mtbl[0];
	int i;
#define MTBL_ADD(_wifi_cr, _warp_cr)\
	do {\
		m->warp_cr = _warp_cr,\
		m->wifi_cr = _wifi_cr;\
		m++;\
	} while (0)

	/*insert hw*/
	MTBL_ADD(hw->int_sta,    WED_INT_STA_ADDR);
	MTBL_ADD(hw->int_mask,   WED_INT_MSK_ADDR);
#ifdef WED_RX_SUPPORT
	MTBL_ADD(hw->event.base, WED_RX0_CTRL0_ADDR);
	MTBL_ADD(hw->event.cnt,  WED_RX0_CTRL1_ADDR);
	MTBL_ADD(hw->event.cidx, WED_RX0_CTRL2_ADDR);
	MTBL_ADD(hw->event.didx, WED_RX0_CTRL3_ADDR);
#endif /*RX*/
#ifdef WED_TX_SUPPORT
	for (i = 0; i < hw->tx_ring_num; i++) {
		if (i == 0) {
			MTBL_ADD(hw->tx[0].base, WED_TX0_CTRL0_ADDR);
			MTBL_ADD(hw->tx[0].cnt,  WED_TX0_CTRL1_ADDR);
			MTBL_ADD(hw->tx[0].cidx, WED_TX0_CTRL2_ADDR);
			MTBL_ADD(hw->tx[0].didx, WED_TX0_CTRL3_ADDR);
		} else if (i == 1) {
			MTBL_ADD(hw->tx[1].base, WED_TX1_CTRL0_ADDR);
			MTBL_ADD(hw->tx[1].cnt,  WED_TX1_CTRL1_ADDR);
			MTBL_ADD(hw->tx[1].cidx, WED_TX1_CTRL2_ADDR);
			MTBL_ADD(hw->tx[1].didx, WED_TX1_CTRL3_ADDR);
		} else {
			warp_dbg(WARP_DBG_OFF, "%s: tx ring(%d) for wed %d is invalid",
				__func__, i, warp->idx);
		}
	}
#endif
#ifdef WED_RX_D_SUPPORT
	for (i = 0; i < hw->rx_wed_ring_num; i++) {
		if (i == 0) {
			MTBL_ADD(hw->rx_wed[0].base, WED_RX_BASE_PTR_0_ADDR);
			MTBL_ADD(hw->rx_wed[0].cnt,  WED_RX_MAX_CNT_0_ADDR);
			MTBL_ADD(hw->rx_wed[0].cidx, WED_RX_CRX_IDX_0_ADDR);
			MTBL_ADD(hw->rx_wed[0].didx, WED_RX_DRX_IDX_0_ADDR);
		} else if (i == 1) {
			MTBL_ADD(hw->rx_wed[1].base, WED_RX_BASE_PTR_1_ADDR);
			MTBL_ADD(hw->rx_wed[1].cnt,  WED_RX_MAX_CNT_1_ADDR);
			MTBL_ADD(hw->rx_wed[1].cidx, WED_RX_CRX_IDX_1_ADDR);
			MTBL_ADD(hw->rx_wed[1].didx, WED_RX_DRX_IDX_1_ADDR);
		} else {
			warp_dbg(WARP_DBG_OFF, "%s: rx ring(%d) for wed %d is invalid",
				__func__, i, warp->idx);
		}
	}
#endif
#ifdef WED_RX_HW_RRO_3_0_DBG_MEMCPY
	if (hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_0)) {
		/* ACK_SN_CNT */
		MTBL_ADD(0xD4300, WED_RRO_RX_DBG1_ADDR);
		/* Total ind_cmd counter */
		MTBL_ADD(0xD4304, WED_RX_IND_CMD_CNT0_ADDR);
		/* ind_cmd magic_cnt fail counter */
		MTBL_ADD(0xD4308, WED_RX_IND_CMD_CNT9_ADDR);
		/* Total addr_elem counter */
		MTBL_ADD(0xD430C, WED_RX_ADDR_ELEM_CNT0_ADDR);
		/* Sign fail counter */
		MTBL_ADD(0xD4310, WED_RX_ADDR_ELEM_CNT1_ADDR);
		/* Sign OK counter */
		MTBL_ADD(0xD4314, WED_RX_MSDU_PG_CNT1_ADDR);
		/* WED_RRO state machine */
		MTBL_ADD(0xD4318, WED_RRO_RX_HW_STS_ADDR);
	}
#endif
	MTBL_ADD(0, 0);

#undef MTBL_ADD
}

void
warp_pdma_mask_set_hw(struct wed_entry *wed, u32 int_mask)
{
	warp_io_write32(wed, WED_WPDMA_INT_MSK_ADDR, int_mask);
}

void
warp_ser_trigger_hw(struct wifi_entry *wifi)
{
	struct wifi_hw *hw = &wifi->hw;

	warp_io_write32(wifi, hw->int_ser, hw->int_ser_value);
}

void
warp_ser_update_hw(struct wed_entry *wed, struct wed_ser_state *state)
{
	u16 head_idx, tail_idx;

	/*WED_TX_DMA*/
	warp_io_read32(wed, WED_ST_ADDR, &state->tx_dma_stat);
	state->tx_dma_stat = GET_FIELD(WED_ST_TX_ST, state->tx_dma_stat);
	warp_io_read32(wed, WED_TX0_MIB_ADDR, &state->tx0_mib);
	warp_io_read32(wed, WED_TX1_MIB_ADDR, &state->tx1_mib);
	warp_io_read32(wed, WED_TX0_CTRL2_ADDR, &state->tx0_cidx);
	warp_io_read32(wed, WED_TX1_CTRL2_ADDR, &state->tx1_cidx);
	warp_io_read32(wed, WED_TX0_CTRL3_ADDR, &state->tx0_didx);
	warp_io_read32(wed, WED_TX1_CTRL3_ADDR, &state->tx1_didx);
	/*WED_WDMA*/
	warp_io_read32(wed, WED_WDMA_ST_ADDR, &state->wdma_stat);
	state->wdma_stat = GET_FIELD(WED_WDMA_ST_RX_DRV_ST, state->wdma_stat);
	warp_io_read32(wed, WED_WDMA_RX0_MIB_ADDR, &state->wdma_rx0_mib);
	warp_io_read32(wed, WED_WDMA_RX1_MIB_ADDR, &state->wdma_rx1_mib);
	warp_io_read32(wed, WED_WDMA_RX0_RECYCLE_MIB_ADDR, &state->wdma_rx0_recycle_mib);
	warp_io_read32(wed, WED_WDMA_RX1_RECYCLE_MIB_ADDR, &state->wdma_rx1_recycle_mib);
	/*WED_WPDMA*/
	warp_io_read32(wed, WED_WPDMA_ST_ADDR, &state->wpdma_stat);
	state->wpdma_stat = GET_FIELD(WED_WPDMA_ST_TX_DRV_ST, state->wpdma_stat);
	warp_io_read32(wed, WED_WPDMA_TX0_MIB_ADDR, &state->wpdma_tx0_mib);
	warp_io_read32(wed, WED_WPDMA_TX1_MIB_ADDR, &state->wpdma_tx1_mib);
	/*WED_BM*/
	warp_io_read32(wed, WED_TX_BM_PTR_ADDR, &state->bm_tx_stat);
	head_idx = GET_FIELD(WED_TX_BM_PTR_HEAD_IDX, state->bm_tx_stat);
	tail_idx = GET_FIELD(WED_TX_BM_PTR_TAIL_IDX, state->bm_tx_stat);
	state->bm_tx_stat = ((tail_idx - head_idx) & 0xFFFF);
	warp_io_read32(wed, WED_TX_FREE_TO_TX_TKID_TKID_MIB_ADDR, &state->txfree_to_bm_mib);
	warp_io_read32(wed, WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB_ADDR,
		       &state->txbm_to_wdma_mib);
}

void
warp_procinfo_dump_cfg_hw(struct warp_entry *warp, struct seq_file *seq)
{
	struct wed_entry *wed = &warp->wed;
	struct wifi_entry *wifi = &warp->wifi;
	struct wdma_entry *wdma = &warp->wdma;
	struct wifi_hw *hw = &wifi->hw;
	u32 value;

	dump_string(seq, "==========WED basic info:==========\n");
	dump_io(seq, wed, "WED_REV", WED_REV_ID_ADDR);
	dump_io(seq, wed, "WED_CTRL", WED_CTRL_ADDR);
	dump_io(seq, wed, "WED_CTRL2", WED_CTRL2_ADDR);
	dump_io(seq, wed, "WED_EX_INT_STA", WED_EX_INT_STA_ADDR);
	dump_io(seq, wed, "WED_EX_INT_MSK", WED_EX_INT_MSK_ADDR);
	dump_io(seq, wed, "WED_ST", WED_ST_ADDR);
	dump_io(seq, wed, "WED_GLO_CFG", WED_GLO_CFG_ADDR);
	dump_io(seq, wed, "WED_INT_STA", WED_INT_STA_ADDR);
	dump_io(seq, wed, "WED_INT_MSK", WED_INT_MSK_ADDR);
	dump_io(seq, wed, "WED_AXI_CTRL", WED_AXI_CTRL_ADDR);
	dump_string(seq, "==========WED TX buf info:==========\n");
	dump_io(seq, wed, "WED_BM_ST", WED_BM_ST_ADDR);
	dump_io(seq, wed, "WED_TX_BM_BASE", WED_TX_BM_BASE_ADDR);
	dump_io(seq, wed, "WED_TX_BM_STS", WED_TX_BM_STS_ADDR);
	dump_io(seq, wed, "WED_TX_TKID_CTRL", WED_TX_TKID_CTRL_ADDR);
	dump_io(seq, wed, "WED_TX_TKID_TKID", WED_TX_TKID_TKID_ADDR);
	dump_io(seq, wed, "WED_TX_TKID_DYN_TH", WED_TX_TKID_DYN_TH_ADDR);
	dump_io(seq, wed, "WED_TX_TKID_INTF", WED_TX_TKID_INTF_ADDR);
	dump_io(seq, wed, "WED_TX_TKID_RECYC", WED_TX_TKID_RECYC_ADDR);
	dump_io(seq, wed, "WED_TX_FREE_TO_TX_BM_TKID_MIB_ADDR",
			WED_TX_FREE_TO_TX_TKID_TKID_MIB_ADDR);
	dump_io(seq, wed, "WED_TX_BM_TO_WDMA_RX_DRV_TKID_MIB_ADDR",
			WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB_ADDR);
	dump_io(seq, wed, "WED_TX_TKID_STS_ADDR",
			WED_TX_TKID_STS_ADDR);
	dump_string(seq, "==========WED RX BM info:==========\n");
	dump_io(seq, wed, "WED_RX_BM_RX_DMAD", WED_RX_BM_RX_DMAD_ADDR);
	dump_io(seq, wed, "WED_RX_BM_BASE", WED_RX_BM_BASE_ADDR);
	dump_io(seq, wed, "WED_RX_BM_INIT_PTR", WED_RX_BM_INIT_PTR_ADDR);
	dump_io(seq, wed, "WED_RX_BM_PTR", WED_RX_BM_PTR_ADDR);
	warp_io_read32(wed, WED_RX_BM_RX_DMAD_ADDR, &value);
	value = GET_FIELD(WED_RX_BM_RX_DMAD_SDL0, value);
	seq_printf(seq, "%s\t: 0x%x\n", "WED_RX_BM_BLEN", value);
	dump_io(seq, wed, "WED_RX_BM_STS", WED_RX_BM_STS_ADDR);
	dump_io(seq, wed, "WED_RX_BM_INTF2", WED_RX_BM_INTF2_ADDR);
	dump_io(seq, wed, "WED_RX_BM_INTF", WED_RX_BM_INTF_ADDR);
	dump_io(seq, wed, "WED_RX_BM_ERR_STS", WED_RX_BM_ERR_STS_ADDR);
	dump_string(seq,  "==========WED PCI Host Control:==========\n");
	dump_io(seq, wed, "WED_PCIE_CFG_ADDR_INTS", WED_PCIE_CFG_ADDR_INTS_ADDR);
	dump_io(seq, wed, "WED_PCIE_CFG_ADDR_INTM", WED_PCIE_CFG_ADDR_INTM_ADDR);
	dump_io(seq, wed, "WED_PCIE_INTS_TRIG", WED_PCIE_INTS_TRIG_ADDR);
	dump_io(seq, wed, "WED_PCIE_INTS_REC", WED_PCIE_INTS_REC_ADDR);
	dump_io(seq, wed, "WED_PCIE_INTM_REC", WED_PCIE_INTM_REC_ADDR);
	dump_io(seq, wed, "WED_PCIE_INT_CTRL", WED_PCIE_INT_CTRL_ADDR);
	dump_string(seq,  "==========WED_WPDMA basic info:==========\n");
	dump_io(seq, wed, "WED_WPDMA_ST", WED_WPDMA_ST_ADDR);
	dump_io(seq, wed, "WED_WPDMA_INT_STA_REC", WED_WPDMA_INT_STA_REC_ADDR);
	dump_io(seq, wed, "WED_WPDMA_GLO_CFG", WED_WPDMA_GLO_CFG_ADDR);
	dump_io(seq, wed, "WED_WPDMA_CFG_ADDR_INTS", WED_WPDMA_CFG_ADDR_INTS_ADDR);
	dump_io(seq, wed, "WED_WPDMA_CFG_ADDR_INTM", WED_WPDMA_CFG_ADDR_INTM_ADDR);
	dump_io(seq, wed, "WED_WPDMA_CFG_ADDR_TX", WED_WPDMA_CFG_CIDX_ADDR_TX0_ADDR);
	dump_io(seq, wed, "WED_WPDMA_CFG_ADDR_TX_FREE", WED_WPDMA_CFG_CIDX_ADDR_TX0_FREE_ADDR);
	dump_io(seq, wed, "WED_WPDAM_CTRL", WED_WPDMA_CTRL_ADDR);
	dump_string(seq, "==========WED_WDMA basic info:==========\n");
	dump_io(seq, wed, "WED_WDMA_ST", WED_WDMA_ST_ADDR);
	dump_io(seq, wed, "WED_WDMA_INFO", WED_WDMA_INFO_ADDR);
	dump_io(seq, wed, "WED_WDMA_GLO_CFG", WED_WDMA_GLO_CFG_ADDR);
	dump_io(seq, wed, "WED_WDMA_RST_IDX", WED_WDMA_RST_IDX_ADDR);
	dump_io(seq, wed, "WED_WDMA_LOAD_DRV_IDX", WED_WDMA_LOAD_DRV_IDX_ADDR);
	dump_io(seq, wed, "WED_WDMA_LOAD_CRX_IDX", WED_WDMA_LOAD_CRX_IDX_ADDR);
	dump_io(seq, wed, "WED_WDMA_SPR", WED_WDMA_SPR_ADDR);
	dump_io(seq, wed, "WED_WDMA_INT_STA_REC", WED_WDMA_INT_STA_REC_ADDR);
	dump_io(seq, wed, "WED_WDMA_INT_TRIG", WED_WDMA_INT_TRIG_ADDR);
	dump_io(seq, wed, "WED_WDMA_INT_CTRL", WED_WDMA_INT_CTRL_ADDR);
	dump_io(seq, wed, "WED_WDMA_INT_CLR", WED_WDMA_INT_CLR_ADDR);
	dump_io(seq, wed, "WED_WDMA_CFG_BASE", WED_WDMA_CFG_BASE_ADDR);
	dump_io(seq, wed, "WED_WDMA_OFST0", WED_WDMA_OFST0_ADDR);
	dump_io(seq, wed, "WED_WDMA_OFST1", WED_WDMA_OFST1_ADDR);
	/*other part setting*/
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
	dump_string(seq,  "==========WPDMA basic info:==========\n");
	dump_io(seq, wifi, "WPDMA_TX_GLO_CFG_ADDR", hw->tx_dma_glo_cfg);
	dump_io(seq, wifi, "WPDMA_RX_GLO_CFG_ADDR", hw->rx_dma_glo_cfg);
	dump_io(seq, wifi, "WPDMA_INT_MSK_ADDR", hw->int_mask);
	dump_io(seq, wifi, "WPDMA_INT_STA_ADDR", hw->int_sta);
}

void
warp_procinfo_dump_txinfo_hw(struct warp_entry *warp, struct seq_file *output)
{
	struct wed_entry *wed = &warp->wed;
	struct wifi_entry *wifi = &warp->wifi;
	struct wdma_entry *wdma = &warp->wdma;
	struct wifi_hw *hw = &wifi->hw;
	u32 ctrl0, ctrl1, ctrl2, ctrl3;
	u32 tcnt, cidx, didx, qcnt, mib;
	dma_addr_t base;
	int i;

	dump_string(output, "==========WED TX ring info===========\n");
	for (i = 0; i < 2; i++) {
		u32 mib_offset = WED_TX1_MIB_ADDR - WED_TX0_MIB_ADDR;
		u32 ring_offset = WED_TX1_CTRL0_ADDR - WED_TX0_CTRL0_ADDR;

		warp_io_read32(wed, WED_TX0_MIB_ADDR + i * mib_offset, &mib);
		warp_io_read32(wed, WED_TX0_CTRL0_ADDR + i * ring_offset, &ctrl0);
		warp_io_read32(wed, WED_TX0_CTRL1_ADDR + i * ring_offset, &ctrl1);
		warp_io_read32(wed, WED_TX0_CTRL3_ADDR + i * ring_offset, &ctrl3);
		warp_io_read32(wed, WED_TX0_CTRL2_ADDR + i * ring_offset, &ctrl2);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(WED_TX0_CTRL1_BASE_PTR_H, ctrl1) << 32;
#endif
		tcnt = GET_FIELD(WED_TX0_CTRL1_MAX_CNT, ctrl1);
		cidx = GET_FIELD(WED_TX0_CTRL2_CPU_IDX, ctrl2);
		didx = GET_FIELD(WED_TX0_CTRL3_DMA_IDX, ctrl3);
		if (base)
			qcnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + tcnt);
		else
			qcnt = 0;

		seq_printf(output, "WED_TX%d_MIB\t: 0x%x\n", i, mib);
		seq_printf(output, "WED_TX%d_BASE\t: %pad\n", i, &base);
		seq_printf(output, "WED_TX%d_MAX\t: 0x%x\n", i, tcnt);
		seq_printf(output, "WED_TX%d_CIDX\t: 0x%x\n", i, cidx);
		seq_printf(output, "WED_TX%d_DIDX\t: 0x%x\n", i, didx);
		seq_printf(output, "WED_TX%d Qcnt\t: 0x%x\n", i, qcnt);
	}

	dump_string(output, "==========WED WPDMA TX ring info===========\n");
	for (i = 0; i < 2; i++) {
		u32 mib_offset = WED_WPDMA_TX1_MIB_ADDR - WED_WPDMA_TX0_MIB_ADDR;
		u32 ring_offset = WED_WPDMA_TX1_CTRL0_ADDR - WED_WPDMA_TX0_CTRL0_ADDR;

		warp_io_read32(wed, WED_WPDMA_TX0_MIB_ADDR + i * mib_offset, &mib);
		warp_io_read32(wed, WED_WPDMA_TX0_CTRL0_ADDR + i * ring_offset, &ctrl0);
		warp_io_read32(wed, WED_WPDMA_TX0_CTRL1_ADDR + i * ring_offset, &ctrl1);
		warp_io_read32(wed, WED_WPDMA_TX0_CTRL3_ADDR + i * ring_offset, &ctrl3);
		warp_io_read32(wed, WED_WPDMA_TX0_CTRL2_ADDR + i * ring_offset, &ctrl2);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(WED_WPDMA_TX0_CTRL1_BASE_PTR_H, ctrl1) << 32;
#endif
		tcnt = GET_FIELD(WED_WPDMA_TX0_CTRL1_MAX_CNT, ctrl1);
		cidx = GET_FIELD(WED_WPDMA_TX0_CTRL2_CPU_IDX, ctrl2);
		didx = GET_FIELD(WED_WPDMA_TX0_CTRL3_DMA_IDX_MIRO, ctrl3);
		if (base)
			qcnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + tcnt);
		else
			qcnt = 0;

		seq_printf(output, "WED_WPDMA_TX%d_MIB\t: 0x%x\n", i, mib);
		seq_printf(output, "WED_WPDMA_TX%d_BASE\t: %pad\n", i, &base);
		seq_printf(output, "WED_WPDMA_TX%d_MAX\t: 0x%x\n", i, tcnt);
		seq_printf(output, "WED_WPDMA_TX%d_CIDX\t: 0x%x\n", i, cidx);
		seq_printf(output, "WED_WPDMA_TX%d_DIDX\t: 0x%x\n", i, didx);
		seq_printf(output, "WED_WPDMA_TX%d Qcnt\t: 0x%x\n", i, qcnt);
	}

	dump_string(output, "==========WPDMA TX ring info===========\n");
	for (i = 0; i < 2; i++) {
		u32 ring_offset = hw->tx[1].base - hw->tx[0].base;

		warp_io_read32(wifi, hw->tx[0].base + i * ring_offset, &ctrl0);
		warp_io_read32(wifi, hw->tx[0].cnt + i * ring_offset, &ctrl1);
		warp_io_read32(wifi, hw->tx[0].didx + i * ring_offset, &ctrl3);
		warp_io_read32(wifi, hw->tx[0].cidx + i * ring_offset, &ctrl2);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(WED_WPDMA_TX0_CTRL1_BASE_PTR_H, ctrl1) << 32;
#endif
		tcnt = GET_FIELD(WED_WPDMA_TX0_CTRL1_MAX_CNT, ctrl1);
		cidx = GET_FIELD(WED_WPDMA_TX0_CTRL2_CPU_IDX, ctrl2);
		didx = GET_FIELD(WED_WPDMA_TX0_CTRL3_DMA_IDX_MIRO, ctrl3);
		if (base)
			qcnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + tcnt);
		else
			qcnt = 0;

		seq_printf(output, "WPDMA_TX%d_BASE\t: %pad\n", i, &base);
		seq_printf(output, "WPDMA_TX%d_MAX\t: 0x%x\n", i, tcnt);
		seq_printf(output, "WPDMA_TX%d_CIDX\t: 0x%x\n", i, cidx);
		seq_printf(output, "WPDMA_TX%d_DIDX\t: 0x%x\n", i, didx);
		seq_printf(output, "WPDMA_TX%d Qcnt\t: 0x%x\n", i, qcnt);
	}

	/* WPDMA TX Free Done Event Ring */
	dump_string(output, "==========WPDMA RX ring info(free done)===========\n");
	for (i = 0; i < 1; i++) {
		warp_io_read32(wifi, hw->event.base, &ctrl0);
		warp_io_read32(wifi, hw->event.cnt, &ctrl1);
		warp_io_read32(wifi, hw->event.cidx, &ctrl2);
		warp_io_read32(wifi, hw->event.didx, &ctrl3);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(WED_WPDMA_RX0_CTRL1_BASE_PTR_H, ctrl1) << 32;
#endif
		tcnt = GET_FIELD(WED_WPDMA_RX0_CTRL1_MAX_CNT, ctrl1);
		cidx = GET_FIELD(WED_WPDMA_RX0_CTRL2_CPU_IDX, ctrl2);
		didx = GET_FIELD(WED_WPDMA_RX0_CTRL3_DMA_IDX_MIRO, ctrl3);
		if (base)
			qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
		else
			qcnt = 0;

		seq_printf(output, "WPDMA_RX%d_BASE\t: %pad\n", i, &base);
		seq_printf(output, "WPDMA_RX%d_MAX\t: 0x%x\n", i, tcnt);
		seq_printf(output, "WPDMA_RX%d_CIDX\t: 0x%x\n", i, cidx);
		seq_printf(output, "WPDMA_RX%d_DIDX\t: 0x%x\n", i, didx);
		seq_printf(output, "WPDMA_RX%d Qcnt\t: 0x%x\n", i, qcnt);
	}

	/*WPDMA status from WED*/
	dump_string(output, "==========WED WPDMA RX ring info(free done)===========\n");
	dump_io(output, wed, "WED_WPDMA_RX_FREE_TKID_MIB", WED_WPDMA_RX_EXTC_FREE_TKID_MIB_ADDR);
	for (i = 0; i < 1; i++) {
		u32 mib_offset = WED_WPDMA_RX1_MIB_ADDR - WED_WPDMA_RX0_MIB_ADDR;
		u32 ring_offset = WED_WPDMA_RX1_CTRL0_ADDR - WED_WPDMA_RX0_CTRL0_ADDR;

		warp_io_read32(wed, WED_WPDMA_RX0_MIB_ADDR + i * mib_offset, &mib);
		warp_io_read32(wed, WED_WPDMA_RX0_CTRL0_ADDR + i * ring_offset, &ctrl0);
		warp_io_read32(wed, WED_WPDMA_RX0_CTRL1_ADDR + i * ring_offset, &ctrl1);
		warp_io_read32(wed, WED_WPDMA_RX0_CTRL2_ADDR + i * ring_offset, &ctrl2);
		warp_io_read32(wed, WED_WPDMA_RX0_CTRL3_ADDR + i * ring_offset, &ctrl3);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(WED_WPDMA_RX0_CTRL1_BASE_PTR_H, ctrl1) << 32;
#endif
		tcnt = GET_FIELD(WED_WPDMA_RX0_CTRL1_MAX_CNT, ctrl1);
		cidx = GET_FIELD(WED_WPDMA_RX0_CTRL2_CPU_IDX, ctrl2);
		didx = GET_FIELD(WED_WPDMA_RX0_CTRL3_DMA_IDX_MIRO, ctrl3);
		if (base)
			qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
		else
			qcnt = 0;

		seq_printf(output, "WED_WPDMA_RX%d_MIB\t: 0x%x\n", i, mib);
		seq_printf(output, "WED_WPDMA_RX%d_BASE\t: %pad\n", i, &base);
		seq_printf(output, "WED_WPDMA_RX%d_MAX\t: 0x%x\n", i, tcnt);
		seq_printf(output, "WED_WPDMA_RX%d_CIDX\t: 0x%x\n", i, cidx);
		seq_printf(output, "WED_WPDMA_RX%d_DIDX\t: 0x%x\n", i, didx);
		seq_printf(output, "WED_WPDMA_RX%d Qcnt\t: 0x%x\n", i, qcnt);
	}

	dump_string(output, "==========WED RX ring info(free done)===========\n");
	for (i = 0; i < 1; i++) {
		u32 mib_offset = WED_RX1_MIB_ADDR - WED_RX0_MIB_ADDR;
		u32 ring_offset = WED_RX1_CTRL0_ADDR - WED_RX0_CTRL0_ADDR;

		warp_io_read32(wed, WED_RX0_MIB_ADDR + i * mib_offset, &mib);
		warp_io_read32(wed, WED_RX0_CTRL0_ADDR + i * ring_offset, &ctrl0);
		warp_io_read32(wed, WED_RX0_CTRL1_ADDR + i * ring_offset, &ctrl1);
		warp_io_read32(wed, WED_RX0_CTRL2_ADDR + i * ring_offset, &ctrl2);
		warp_io_read32(wed, WED_RX0_CTRL3_ADDR + i * ring_offset, &ctrl3);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(WED_RX0_CTRL1_BASE_PTR_H, ctrl1) << 32;
#endif
		tcnt = GET_FIELD(WED_RX0_CTRL1_MAX_CNT, ctrl1);
		cidx = GET_FIELD(WED_RX0_CTRL2_CPU_IDX, ctrl2);
		didx = GET_FIELD(WED_RX0_CTRL3_DMA_IDX, ctrl3);
		if (base)
			qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
		else
			qcnt = 0;

		seq_printf(output, "WED_RX%d_MIB\t: 0x%x\n", i, mib);
		seq_printf(output, "WED_RX%d_BASE\t: %pad\n", i, &base);
		seq_printf(output, "WED_RX%d_MAX\t: 0x%x\n", i, tcnt);
		seq_printf(output, "WED_RX%d_CIDX\t: 0x%x\n", i, cidx);
		seq_printf(output, "WED_RX%d_DIDX\t: 0x%x\n", i, didx);
		seq_printf(output, "WED_RX%d Qcnt\t: 0x%x\n", i, qcnt);
	}

	dump_string(output, "==========WED WDMA RX ring info===========\n");
	for (i = 0; i < 1; i++) {
		u32 mib_offset = WED_WDMA_RX1_PROCESSED_MIB_ADDR - WED_WDMA_RX0_PROCESSED_MIB_ADDR;
		u32 ring_offset = WED_WDMA_RX1_BASE_ADDR - WED_WDMA_RX0_BASE_ADDR;

		warp_io_read32(wed, WED_WDMA_RX0_PROCESSED_MIB_ADDR + i * mib_offset, &mib);
		warp_io_read32(wed, WED_WDMA_RX0_BASE_ADDR + i * ring_offset, &ctrl0);
		warp_io_read32(wed, WED_WDMA_RX0_CNT_ADDR + i * ring_offset, &ctrl1);
		warp_io_read32(wed, WED_WDMA_RX0_CRX_IDX_ADDR + i * ring_offset, &ctrl2);
		warp_io_read32(wed, WED_WDMA_RX0_DRX_IDX_ADDR + i * ring_offset, &ctrl3);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(WED_WDMA_RX0_CNT_BASE_PTR_H, ctrl1) << 32;
#endif
		tcnt = GET_FIELD(WED_WDMA_RX0_CNT_MAX, ctrl1);
		cidx = GET_FIELD(WED_WDMA_RX0_CRX_IDX_CRX_IDX, ctrl2);
		didx = GET_FIELD(WED_WDMA_RX0_DRX_IDX_DRX_IDX_MIRO, ctrl3);
		if (base)
			qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
		else
			qcnt = 0;

		seq_printf(output, "WED_WMDA_RX%d_MIB\t: 0x%x\n", i, mib);
		seq_printf(output, "WED_WMDA_RX%d_BASE\t: %pad\n", i, &base);
		seq_printf(output, "WED_WMDA_RX%d_MAX\t: 0x%x\n", i, tcnt);
		seq_printf(output, "WED_WMDA_RX%d_CIDX\t: 0x%x\n", i, cidx);
		seq_printf(output, "WED_WMDA_RX%d_DIDX\t: 0x%x\n", i, didx);
		seq_printf(output, "WED_WMDA_RX%d Qcnt\t: 0x%x\n", i, qcnt);
	}

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

	dump_string(output, "=====WED TXBM debug info=====\n");
	{
		warp_io_read32(wed, WED_TX_BM_BASE_ADDR, &ctrl0);
		warp_io_read32(wed, WED_TX_BM_BLEN_ADDR, &ctrl1);
		warp_io_read32(wed, WED_TX_BM_PTR_ADDR, &ctrl2);
		warp_io_read32(wed, WED_TX_BM_RANGE_CFG_ADDR, &ctrl3);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(WED_TX_BM_BLEN_BASE_PTR_H, ctrl1) << 32;
#endif
		tcnt = GET_FIELD(WED_TX_BM_RANGE_CFG_SW_CFG_BUF_IDX, ctrl3);
		cidx = GET_FIELD(WED_TX_BM_PTR_HEAD_IDX, ctrl2);
		didx = GET_FIELD(WED_TX_BM_PTR_TAIL_IDX, ctrl2);
		if (base)
			qcnt = (didx >= cidx) ? (didx - cidx) : (didx - cidx + tcnt + 1);
		else
			qcnt = 0;

		seq_printf(output, "WED_TXBM_BASE\t: %pad\n", &base);
		seq_printf(output, "WED_TXBM_MAX\t: 0x%x\n", tcnt);
		seq_printf(output, "WED_TXBM_HEAD\t: 0x%x\n", cidx);
		seq_printf(output, "WED_TXBM_TAIL\t: 0x%x\n", didx);
		seq_printf(output, "WED_TXBM Left\t: 0x%x\n", qcnt);
	}

	dump_string(output, "=====WED TKID debug info=====\n");
	{
		warp_io_read32(wed, WED_TX_TKID_STS_ADDR, &ctrl0);
		qcnt = GET_FIELD(WED_TX_TKID_STS_FREE_NUM, ctrl0);
		seq_printf(output, "WED_TKID Left\t: 0x%x\n", qcnt);
	}

}

void
warp_procinfo_dump_rxinfo_hw(struct warp_entry *warp, struct seq_file *output)
{
	struct wed_entry *wed = &warp->wed;
	struct wdma_entry *wdma = &warp->wdma;
	u32 ctrl0, ctrl1, ctrl2, ctrl3;
	u32 tcnt, cidx, didx, mcnt, qcnt, mib;
	dma_addr_t base;
	int i;

	dump_string(output, "==========WED RX INT info===========\n");
	dump_io(output, wed, "WED_PCIE_INT_CTRL", WED_PCIE_INT_CTRL_ADDR);
	dump_io(output, wed, "WED_PCIE_INTS_REC", WED_PCIE_INTS_REC_ADDR);
	dump_io(output, wed, "WED_WPDMA_INT_STA_REC", WED_WPDMA_INT_STA_REC_ADDR);
	dump_io(output, wed, "WED_WPDMA_INT_MON", WED_WPDMA_INT_MON_ADDR);
	dump_io(output, wed, "WED_WPDMA_INT_CTRL", WED_WPDMA_INT_CTRL_ADDR);
	dump_io(output, wed, "WED_WPDMA_INT_CTRL_TX", WED_WPDMA_INT_CTRL_TX_ADDR);
	dump_io(output, wed, "WED_WPDMA_INT_CTRL_RX", WED_WPDMA_INT_CTRL_RX_ADDR);
	dump_io(output, wed, "WED_WPDMA_INT_CTRL_TX_FREE", WED_WPDMA_INT_CTRL_TX_FREE_ADDR);
	dump_io(output, wed, "WED_WPDMA_ST", WED_WPDMA_ST_ADDR);

	dump_string(output, "==========WED RX Data ring info===========\n");
	for (i = 0; i < 1; i++) {
		u32 ring_offset = WED_RX_BASE_PTR_1_ADDR - WED_RX_BASE_PTR_0_ADDR;

		warp_io_read32(wed, WED_RX_BASE_PTR_0_ADDR + i * ring_offset, &ctrl0);
		warp_io_read32(wed, WED_RX_MAX_CNT_0_ADDR + i * ring_offset, &ctrl1);
		warp_io_read32(wed, WED_RX_CRX_IDX_0_ADDR + i * ring_offset, &ctrl2);
		warp_io_read32(wed, WED_RX_DRX_IDX_0_ADDR + i * ring_offset, &ctrl3);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(WED_RX_MAX_CNT_0_BASE_PTR_H, ctrl1) << 32;
#endif
		tcnt = GET_FIELD(WED_RX_MAX_CNT_0_MAX_CNT, ctrl1);
		cidx = GET_FIELD(WED_RX_CRX_IDX_0_CRX_IDX, ctrl2);
		didx = GET_FIELD(WED_RX_DRX_IDX_0_DRX_IDX, ctrl3);
		if (base)
			qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
		else
			qcnt = 0;

		seq_printf(output, "WED_RX%d_BASE\t: %pad\n", i, &base);
		seq_printf(output, "WED_RX%d_MAX\t: 0x%x\n", i, tcnt);
		seq_printf(output, "WED_RX%d_CIDX\t: 0x%x\n", i, cidx);
		seq_printf(output, "WED_RX%d_DIDX\t: 0x%x\n", i, didx);
		seq_printf(output, "WED_RX%d Qcnt\t: 0x%x\n", i, qcnt);
	}

	dump_string(output, "==========WED RX RRO Data ring info===========\n");
	for (i = 0; i < 2; i++) {
		u32 ring_offset = RRO_RX_D_RX1_BASE_ADDR - RRO_RX_D_RX0_BASE_ADDR;

		warp_io_read32(wed, RRO_RX_D_RX0_BASE_ADDR + i * ring_offset, &ctrl0);
		warp_io_read32(wed, RRO_RX_D_RX0_CNT_ADDR + i * ring_offset, &ctrl1);
		warp_io_read32(wed, RRO_RX_D_RX0_CRX_IDX_ADDR + i * ring_offset, &ctrl2);
		warp_io_read32(wed, RRO_RX_D_RX0_DRX_IDX_ADDR + i * ring_offset, &ctrl3);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(RRO_RX_D_RX0_CNT_PTR_H, ctrl1) << 32;
#endif
		tcnt = GET_FIELD(RRO_RX_D_RX0_CNT_MAX, ctrl1);
		mcnt = GET_FIELD(RRO_RX_D_RX0_CNT_MAGIC_CNT, ctrl1);
		cidx = GET_FIELD(RRO_RX_D_RX0_CRX_IDX_CRX_IDX, ctrl2);
		didx = GET_FIELD(RRO_RX_D_RX0_DRX_IDX_DRX_IDX_MIRO, ctrl3);
		if (base)
			qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
		else
			qcnt = 0;

		seq_printf(output, "RRO_RX_D_RX%d_BASE\t: %pad\n", i, &base);
		seq_printf(output, "RRO_RX_D_RX%d_MAX\t: 0x%x\n", i, tcnt);
		seq_printf(output, "RRO_RX_D_RX%d_MGC\t: 0x%x\n", i, mcnt);
		seq_printf(output, "RRO_RX_D_RX%d_CIDX\t: 0x%x\n", i, cidx);
		seq_printf(output, "RRO_RX_D_RX%d_DIDX\t: 0x%x\n", i, didx);
		seq_printf(output, "RRO_RX_D_RX%d Qcnt\t: 0x%x\n", i, qcnt);
	}

	dump_string(output, "==========WED MSDU Page ring info===========\n");
	for (i = 0; i < 3; i++) {
		u32 ring_offset = RRO_MSDU_PG_1_CTRL0_ADDR - RRO_MSDU_PG_0_CTRL0_ADDR;

		warp_io_read32(wed, RRO_MSDU_PG_0_CTRL0_ADDR + i * ring_offset, &ctrl0);
		warp_io_read32(wed, RRO_MSDU_PG_0_CTRL1_ADDR + i * ring_offset, &ctrl1);
		warp_io_read32(wed, RRO_MSDU_PG_0_CTRL2_ADDR + i * ring_offset, &ctrl2);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(RRO_MSDU_PG_0_CTRL1_BASE_PTR_M, ctrl1) << 32;
#endif
		tcnt = GET_FIELD(RRO_MSDU_PG_0_CTRL1_MAX_CNT, ctrl1);
		mcnt = GET_FIELD(RRO_MSDU_PG_0_CTRL1_MAGIC_CNT, ctrl1);
		cidx = GET_FIELD(RRO_MSDU_PG_0_CTRL2_CPU_IDX, ctrl2);
		didx = GET_FIELD(RRO_MSDU_PG_0_CTRL2_DMA_IDX_MIRO, ctrl2);
		if (base)
			qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
		else
			qcnt = 0;

		seq_printf(output, "RRO_MSDU_PG_RX%d_BASE\t: %pad\n", i, &base);
		seq_printf(output, "RRO_MSDU_PG_RX%d_MAX\t: 0x%x\n", i, tcnt);
		seq_printf(output, "RRO_MSDU_PG_RX%d_MGC\t: 0x%x\n", i, mcnt);
		seq_printf(output, "RRO_MSDU_PG_RX%d_CIDX\t: 0x%x\n", i, cidx);
		seq_printf(output, "RRO_MSDU_PG_RX%d_DIDX\t: 0x%x\n", i, didx);
		seq_printf(output, "RRO_MSDU_PG_RX%d Qcnt\t: 0x%x\n", i, qcnt);
	}

	dump_string(output, "==========WED IND_CMD ring info==========\n");
	for (i = 0; i < 1; i++) {
		warp_io_read32(wed, IND_CMD_0_CTRL_0_ADDR, &ctrl0);
		warp_io_read32(wed, IND_CMD_0_CTRL_1_ADDR, &ctrl1);
		warp_io_read32(wed, IND_CMD_0_CTRL_2_ADDR, &ctrl2);
		warp_io_read32(wed, RRO_IND_CMD_0_SIGNATURE_ADDR, &ctrl3);

		base = ctrl1;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(IND_CMD_0_CTRL_2_RRO_IND_CMD_BASE_M, ctrl2) << 32;
#endif
		tcnt = GET_FIELD(IND_CMD_0_CTRL_2_MAX_CNT_11_4, ctrl2) << 4;
		mcnt = GET_FIELD(RRO_IND_CMD_0_SIGNATURE_MAGIC_CNT, ctrl3);
		cidx = GET_FIELD(IND_CMD_0_CTRL_0_PROC_IDX, ctrl0);
		didx = GET_FIELD(RRO_IND_CMD_0_SIGNATURE_DMA_IDX, ctrl3);
		if (base)
			qcnt = (didx >= cidx) ? (didx - cidx) : (didx - cidx + tcnt);
		else
			qcnt = 0;

		seq_printf(output, "RRO_IND_CMD_%d_BASE\t: %pad\n", i, &base);
		seq_printf(output, "RRO_IND_CMD_%d_MAX\t: 0x%x\n", i, tcnt);
		seq_printf(output, "RRO_IND_CMD_%d_MGC\t: 0x%x\n", i, mcnt);
		seq_printf(output, "RRO_IND_CMD_%d_PROC_IDX\t: 0x%x\n", i, cidx);
		seq_printf(output, "RRO_IND_CMD_%d_DIDX\t: 0x%x\n", i, didx);
		seq_printf(output, "RRO_IND_CMD_%d Qcnt\t: 0x%x\n", i, qcnt);
	}

	dump_string(output, "==========WED RRO ACK_SN info==========\n");
	{
		warp_io_read32(wed, RRO_CONF_0_ADDR, &ctrl0);
		warp_io_read32(wed, RRO_CONF_1_ADDR, &ctrl1);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(RRO_CONF_1_ACK_SN_BASE_0_M, ctrl1) << 32;
#endif

		seq_printf(output, "%s\t: %pad\n", "RRO_CONF_ACK_SN_BASE", &base);
		seq_printf(output, "%s\t: 0x%x\n", "RRO_CONF_MAX_WIN_SZ",
					GET_FIELD(RRO_CONF_1_MAX_WIN_SZ, ctrl1));
		seq_printf(output, "%s\t: 0x%x\n", "RRO_CONF_PAR_SE_ID",
					GET_FIELD(RRO_CONF_1_PARTICULAR_SE_ID, ctrl1));
	}

	dump_string(output, "==========WED ADDR_ELEM info==========\n");
	{
		warp_io_read32(wed, ADDR_ELEM_CONF_0_ADDR, &ctrl0);
		warp_io_read32(wed, ADDR_ELEM_CONF_0_ADDR, &ctrl1);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)
			GET_FIELD(ADDR_ELEM_CONF_1_PARTICULAR_SE_ID_ADDR_BASE_M, ctrl1) << 32;
#endif

		seq_printf(output, "%s\t: %pad\n", "ADDR_ELEM_CONF_PAR_SE_ID_BASE", &base);
		seq_printf(output, "%s\t: 0x%x\n", "ADDR_ELEM_CONF_PREF_FREE_CNT",
				GET_FIELD(ADDR_ELEM_CONF_1_PREFETCH_ADDR_ELEM_FREE_CNT, ctrl1));

	}

	dump_string(output, "==========WED WPDMA RRO 3.1 ring info===========\n");
	for (i = 0; i < 1; i++) {
		warp_io_read32(wed, WED_WPDMA_RRO3_1_RX_D_RX0_PROCESSED_MIB_ADDR, &mib);
		warp_io_read32(wed, WED_WPDMA_RRO3_1_RX_D_RX0_BASE_ADDR, &ctrl0);
		warp_io_read32(wed, WED_WPDMA_RRO3_1_RX_D_RX0_CNT_ADDR, &ctrl1);
		warp_io_read32(wed, WED_WPDMA_RRO3_1_RX_D_RX0_CRX_IDX_ADDR, &ctrl2);
		warp_io_read32(wed, WED_WPDMA_RRO3_1_RX_D_RX0_DRX_IDX_ADDR, &ctrl3);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)
			GET_FIELD(WED_WPDMA_RRO3_1_RX_D_RX0_CNT_BASE_PTR_H, ctrl1) << 32;
#endif
		tcnt = GET_FIELD(WED_WPDMA_RRO3_1_RX_D_RX0_CNT_MAX, ctrl1);
		cidx = GET_FIELD(WED_WPDMA_RRO3_1_RX_D_RX0_CRX_IDX_CRX_IDX, ctrl2);
		didx = GET_FIELD(WED_WPDMA_RRO3_1_RX_D_RX0_DRX_IDX_DRX_IDX_MIRO, ctrl3);
		if (base)
			qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
		else
			qcnt = 0;

		seq_printf(output, "WED_RRO3_1_RX%d_MIB\t: 0x%x\n", i, mib);
		seq_printf(output, "WED_RRO3_1_RX%d_BASE\t: %pad\n", i, &base);
		seq_printf(output, "WED_RRO3_1_RX%d_MAX\t: 0x%x\n", i, tcnt);
		seq_printf(output, "WED_RRO3_1_RX%d_CIDX\t: 0x%x\n", i, cidx);
		seq_printf(output, "WED_RRO3_1_RX%d_DIDX\t: 0x%x\n", i, didx);
		seq_printf(output, "WED_RRO3_1_RX%d Qcnt\t: 0x%x\n", i, qcnt);
	}

	dump_string(output, "==========WED WDMA TX ring info===========\n");
	for (i = 0; i < 1; i++) {
		warp_io_read32(wed, WED_WDMA_TX0_MIB_0_ADDR, &mib);
		warp_io_read32(wed, WED_WDMA_TX0_BASE_ADDR, &ctrl0);
		warp_io_read32(wed, WED_WDMA_TX0_CNT_ADDR, &ctrl1);
		warp_io_read32(wed, WED_WDMA_TX0_DTX_IDX_ADDR, &ctrl3);
		warp_io_read32(wed, WED_WDMA_TX0_CTX_IDX_ADDR, &ctrl2);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(WED_WDMA_TX0_CNT_BASE_PTR_H, ctrl1) << 32;
#endif
		tcnt = GET_FIELD(WED_WDMA_TX0_CNT_MAX, ctrl1);
		cidx = GET_FIELD(WED_WDMA_TX0_CTX_IDX_CTX_IDX, ctrl2);
		didx = GET_FIELD(WED_WDMA_TX0_DTX_IDX_DTX_IDX, ctrl3);
		if (base)
			qcnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + tcnt);
		else
			qcnt = 0;

		seq_printf(output, "WED_WDMA_TX%d_MIB\t: 0x%x\n", i, mib);
		seq_printf(output, "WED_WDMA_TX%d_BASE\t: %pad\n", i, &base);
		seq_printf(output, "WED_WDMA_TX%d_MAX\t: 0x%x\n", i, tcnt);
		seq_printf(output, "WED_WDMA_TX%d_CIDX\t: 0x%x\n", i, cidx);
		seq_printf(output, "WED_WDMA_TX%d_DIDX\t: 0x%x\n", i, didx);
		seq_printf(output, "WED_WDMA_TX%d Qcnt\t: 0x%x\n", i, qcnt);
	}


	/*WDMA*/
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

	dump_string(output, "=====WED RXBM debug info=====\n");
	{
		warp_io_read32(wed, WED_RX_BM_BASE_ADDR, &ctrl0);
		warp_io_read32(wed, WED_RX_BM_RX_DMAD_ADDR, &ctrl1);
		warp_io_read32(wed, WED_RX_BM_PTR_ADDR, &ctrl2);
		warp_io_read32(wed, WED_RX_BM_RANGE_CFG_ADDR, &ctrl3);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(WED_RX_BM_RX_DMAD_BASE_PTR_H, ctrl1) << 32;
#endif
		tcnt = GET_FIELD(WED_RX_BM_RANGE_CFG_SW_CFG_BUF_IDX, ctrl3);
		cidx = GET_FIELD(WED_RX_BM_PTR_HEAD_IDX, ctrl2);
		didx = GET_FIELD(WED_RX_BM_PTR_TAIL_IDX, ctrl2);
		if (base)
			qcnt = (didx >= cidx) ? (didx - cidx) : (didx - cidx + tcnt + 1);
		else
			qcnt = 0;

		seq_printf(output, "WED_RXBM_BASE\t: %pad\n", &base);
		seq_printf(output, "WED_RXBM_MAX\t: 0x%x\n", tcnt);
		seq_printf(output, "WED_RXBM_HEAD\t: 0x%x\n", cidx);
		seq_printf(output, "WED_RXBM_TAIL\t: 0x%x\n", didx);
		seq_printf(output, "WED_RXBM Left\t: 0x%x\n", qcnt);
	}

	dump_string(output, "===== WED PGBM debug info =====\n");
	{
		warp_io_read32(wed, RRO_PG_BM_BASE_ADDR, &ctrl0);
		warp_io_read32(wed, RRO_PG_BM_RX_DMAD_ADDR, &ctrl1);
		warp_io_read32(wed, RRO_PG_BM_PTR_ADDR, &ctrl2);
		warp_io_read32(wed, RRO_PG_BM_RANGE_CFG_ADDR, &ctrl3);

		base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		base |= (dma_addr_t)GET_FIELD(RRO_PG_BM_RX_DMAD_BASE_PTR_H, ctrl1) << 32;
#endif
		tcnt = GET_FIELD(RRO_PG_BM_RANGE_CFG_SW_CFG_BUF_IDX, ctrl3);
		cidx = GET_FIELD(RRO_PG_BM_PTR_HEAD_IDX, ctrl2);
		didx = GET_FIELD(RRO_PG_BM_PTR_TAIL_IDX, ctrl2);
		if (base)
			qcnt = (didx >= cidx) ? (didx - cidx) : (didx - cidx + tcnt + 1);
		else
			qcnt = 0;

		seq_printf(output, "WED_PGBM_BASE\t: %pad\n", &base);
		seq_printf(output, "WED_PGBM_MAX\t: 0x%x\n", tcnt);
		seq_printf(output, "WED_PGBM_HEAD\t: 0x%x\n", cidx);
		seq_printf(output, "WED_PGBM_TAIL\t: 0x%x\n", didx);
		seq_printf(output, "WED_PGBM Left\t: 0x%x\n", qcnt);
	}
}

void
warp_dbginfo_dump_wed_hw(struct wed_entry *wed)
{
	warp_dbg(WARP_DBG_OFF, "==========WED DEBUG INFO:==========\n");
	dump_addr_value(wed, "WED_IRQ_MON", WED_IRQ_MON_ADDR);
	dump_addr_value(wed, "WED_WDMA_INT_MON", WED_WDMA_INT_MON_ADDR);
	dump_addr_value(wed, "WED_WPDMA_INT_CLR", WED_WPDMA_INT_CLR_ADDR);
	dump_addr_value(wed, "WED_WPDMA_INT_CTRL", WED_WPDMA_INT_CTRL_ADDR);
	dump_addr_value(wed, "WED_WPDMA_INT_MSK", WED_WPDMA_INT_MSK_ADDR);
	dump_addr_value(wed, "WED_WPDMA_INT_MON", WED_WPDMA_INT_MON_ADDR);
	dump_addr_value(wed, "WED_WPDMA_SPR", WED_WPDMA_SPR_ADDR);
	dump_addr_value(wed, "WED_PCIE_CFG_ADDR_INTS", WED_PCIE_CFG_ADDR_INTS_ADDR);
	dump_addr_value(wed, "WED_PCIE_CFG_ADDR_INTM", WED_PCIE_CFG_ADDR_INTM_ADDR);
	dump_addr_value(wed, "WED_PCIE_INTM_REC", WED_PCIE_INTM_REC_ADDR);
	dump_addr_value(wed, "WED_PCIE_INT_CTRL", WED_PCIE_INT_CTRL_ADDR);
	dump_addr_value(wed, "WED_TXD_DW0", WED_TXD_DW0_ADDR);
	dump_addr_value(wed, "WED_TXD_DW1", WED_TXD_DW1_ADDR);
	dump_addr_value(wed, "WED_TXD_DW2", WED_TXD_DW2_ADDR);
	dump_addr_value(wed, "WED_TXD_DW3", WED_TXD_DW3_ADDR);
	dump_addr_value(wed, "WED_TXD_DW4", WED_TXD_DW4_ADDR);
	dump_addr_value(wed, "WED_TXD_DW5", WED_TXD_DW5_ADDR);
	dump_addr_value(wed, "WED_TXD_DW6", WED_TXD_DW6_ADDR);
	dump_addr_value(wed, "WED_TXD_DW7", WED_TXD_DW7_ADDR);
	dump_addr_value(wed, "WED_TXP_DW0", WED_TXP_DW0_ADDR);
	dump_addr_value(wed, "WED_TXP_DW1", WED_TXP_DW1_ADDR);
	dump_addr_value(wed, "WED_DBG_CTRL", WED_DBG_CTRL_ADDR);
	dump_addr_value(wed, "WED_DBG_PRB0", WED_DBG_PRB0_ADDR);
	dump_addr_value(wed, "WED_DBG_PRB1", WED_DBG_PRB1_ADDR);
	dump_addr_value(wed, "WED_DBG_PRB2", WED_DBG_PRB2_ADDR);
	dump_addr_value(wed, "WED_DBG_PRB3", WED_DBG_PRB3_ADDR);
	dump_addr_value(wed, "WED_TX_COHERENT_MIB", WED_TX_COHERENT_MIB_ADDR);
	dump_addr_value(wed, "WED_TXP_DW0", WED_TXP_DW0_ADDR);
}

void
warp_tx_ring_get_hw(struct warp_entry *warp, struct warp_ring *ring, u8 idx)
{
	u32 offset = idx * (WED_TX1_CTRL0_ADDR - WED_TX0_CTRL0_ADDR);

	ring->hw_desc_base = wifi_cr_get(warp, WED_TX0_CTRL0_ADDR + offset);
	ring->hw_cnt_addr  = wifi_cr_get(warp, WED_TX0_CTRL1_ADDR + offset);
	ring->hw_cidx_addr = wifi_cr_get(warp, WED_TX0_CTRL2_ADDR + offset);
	ring->hw_didx_addr = wifi_cr_get(warp, WED_TX0_CTRL3_ADDR + offset);
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
warp_wed_tx_ring_get_idx(struct wed_entry *wed, u32 idx,
	u32 *base, u32 *tcnt, u32 *cidx, u32 *didx)
{
	u32 offset = idx * (WED_TX1_CTRL0_ADDR - WED_TX0_CTRL0_ADDR);

	warp_io_read32(wed, WED_TX0_CTRL0_ADDR + offset, base);
	warp_io_read32(wed, WED_TX0_CTRL1_ADDR + offset, tcnt);
	warp_io_read32(wed, WED_TX0_CTRL2_ADDR + offset, cidx);
	warp_io_read32(wed, WED_TX0_CTRL3_ADDR + offset, didx);
}

void
warp_wed_rx_ring_get_idx(struct wed_entry *wed, u32 idx,
	u32 *base, u32 *tcnt, u32 *cidx, u32 *didx)
{
	u32 offset = idx * (WED_RX1_CTRL0_ADDR - WED_RX0_CTRL0_ADDR);

	warp_io_read32(wed, WED_RX_BASE_PTR_0_ADDR + offset, base);
	warp_io_read32(wed, WED_RX_MAX_CNT_0_ADDR + offset, tcnt);
	warp_io_read32(wed, WED_RX_CRX_IDX_0_ADDR + offset, cidx);
	warp_io_read32(wed, WED_RX_DRX_IDX_0_ADDR + offset, didx);
}

void
warp_wdma_tx_ring_get_idx(struct wdma_entry *wdma, u32 idx,
	u32 *base, u32 *tcnt, u32 *cidx, u32 *didx)
{
	u32 offset = idx * (WDMA_TX_BASE_PTR_1_ADDR - WDMA_TX_BASE_PTR_0_ADDR);

	warp_io_read32(wdma, WDMA_TX_BASE_PTR_0_ADDR + offset, base);
	warp_io_read32(wdma, WDMA_TX_MAX_CNT_0_ADDR + offset, tcnt);
	warp_io_read32(wdma, WDMA_TX_CTX_IDX_0_ADDR + offset, cidx);
	warp_io_read32(wdma, WDMA_TX_DTX_IDX_0_ADDR + offset, didx);
}

void
warp_wdma_rx_ring_get_idx(struct wdma_entry *wdma, u32 idx,
	u32 *base, u32 *tcnt, u32 *cidx, u32 *didx)
{
	u32 offset = idx * (WDMA_RX_BASE_PTR_1_ADDR - WDMA_RX_BASE_PTR_0_ADDR);

	warp_io_read32(wdma, WDMA_RX_BASE_PTR_0_ADDR + offset, base);
	warp_io_read32(wdma, WDMA_RX_MAX_CNT_0_ADDR + offset, tcnt);
	warp_io_read32(wdma, WDMA_RX_CRX_IDX_0_ADDR + offset, cidx);
	warp_io_read32(wdma, WDMA_RX_DRX_IDX_0_ADDR + offset, didx);
}

void
warp_wifi_tx_ring_get_idx(struct wifi_entry *wifi, u32 idx,
	u32 *base, u32 *tcnt, u32 *cidx, u32 *didx)
{
	struct wifi_hw *hw = &wifi->hw;

	warp_io_read32(wifi, hw->tx[idx].base, base);
	warp_io_read32(wifi, hw->tx[idx].cnt, tcnt);
	warp_io_read32(wifi, hw->tx[idx].cidx, cidx);
	warp_io_read32(wifi, hw->tx[idx].didx, didx);
}

static int
warp_wifi_tx_ring_get_info(struct wed_entry *wed, struct warp_info *info)
{
	struct warp_entry *warp = wed->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct wed_tx_ring_ctrl *ring_ctrl = &wed->res_ctrl.tx_ctrl.ring_ctrl;
	struct warp_ring_ *ring;
	u32 ctrl0, ctrl1, ctrl2, ctrl3;
	u32 i;

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		if (i >= info->ring_info.ring_num)
			return -ENOMEM;

		warp_wifi_tx_ring_get_idx(wifi, i,
			&ctrl0, &ctrl1, &ctrl2, &ctrl3);

		ring = info->ring_info.ring;

		ring[i].base = ctrl0;
#ifdef CONFIG_WARP_64BIT_SUPPORT
		ring[i].base |= (dma_addr_t)
			GET_FIELD(WED_WPDMA_TX0_CTRL1_BASE_PTR_H, ctrl1) << 32;
#endif
		ring[i].max_cnt = GET_FIELD(WED_WPDMA_TX0_CTRL1_MAX_CNT, ctrl1);
		ring[i].cidx = GET_FIELD(WED_WPDMA_TX0_CTRL2_CPU_IDX, ctrl2);
		ring[i].didx = GET_FIELD(WED_WPDMA_TX0_CTRL3_DMA_IDX_MIRO, ctrl3);

		if (ring[i].base)
			ring[i].q_cnt = (ring[i].cidx >= ring[i].didx) ?
					(ring[i].cidx - ring[i].didx) :
					(ring[i].cidx - ring[i].didx + ring[i].max_cnt);
		else
			ring[i].q_cnt = 0;
	}

	info->ring_info.ring_num = i;
	return 0;
}

int
warp_wed_get_info(struct wed_entry *wed, struct warp_info *info)
{
	switch (info->type) {
	case WARP_INFO_WED_TX_DATA:
		return warp_wifi_tx_ring_get_info(wed, info);
	default:
		return -EINVAL;
	}
}

int
warp_dummy_cr_set(struct wed_entry *wed, u8 index, u32 value)
{
	if (index > 7) /* WED_SCR0 ~ WED_SCR7)*/
		return -1;

	warp_io_write32(wed, WED_SCR0_ADDR + 4 * index, value);

	return 0;
}

int
warp_dummy_cr_get(struct wed_entry *wed, u8 index, u32 *value)
{
	if (index > 7) /* WED_SCR0 ~ WED_SCR7)*/
		return -1;

	warp_io_read32(wed, WED_SCR0_ADDR + 4 * index, value);

	return 0;
}

void
warp_eint_work_hw(struct wed_entry *wed, u32 status, u32 err_status)
{

	if (err_status & WED_ERR_MON_RX_BM_FREE_AT_EMPTY_MASK)
		warp_dbg(WARP_DBG_ERR, "%s(): rx bm free at empty!\n", __func__);

	if (err_status & WED_ERR_MON_RX_BM_DMAD_RD_ERR_MASK)
		warp_dbg(WARP_DBG_ERR, "%s(): rx bm dmad rd err!\n", __func__);

	if (err_status & WED_ERR_MON_TF_LEN_ERR_MASK)
		warp_dbg(WARP_DBG_ERR, "%s(): tx free notify len error!\n", __func__);

	if (err_status & WED_ERR_MON_TX_DMA_W_RESP_ERR_MASK)
		warp_dbg(WARP_DBG_ERR, "%s(): tx dma write resp err!\n", __func__);

	if (err_status & WED_ERR_MON_TX_DMA_R_RESP_ERR_MASK)
		warp_dbg(WARP_DBG_ERR, "%s(): tx dma read resp err!\n", __func__);

	if (err_status & WED_ERR_MON_RX_DRV_INTI_WDMA_ENABLED_MASK)
		warp_dbg(WARP_DBG_ERR, "%s(): rx drv inti wdma enable!\n", __func__);

	if (status & WED_EX_INT_STA_RX_DRV_COHERENT_MASK)
		warp_dbg(WARP_DBG_LOU, "%s(): rx drv coherent!\n", __func__);

	if (err_status & WED_ERR_MON_RX_DRV_W_RESP_ERR_MASK)
		warp_dbg(WARP_DBG_ERR, "%s(): rx drv write resp err!\n", __func__);

	if (err_status & WED_ERR_MON_RX_DRV_R_RESP_ERR_MASK)
		warp_dbg(WARP_DBG_ERR, "%s(): rx drv read resp err!\n", __func__);

	if (err_status & WED_ERR_MON_TF_TKID_FIFO_INVLD_MASK)
		warp_dbg(WARP_DBG_ERR, "%s(): tx free token id is invalid!\n", __func__);

	if (err_status & WED_ERR_MON_RX_DRV_BM_DMAD_COHERENT_MASK)
		warp_dbg(WARP_DBG_ERR, "%s(): rx drv buffer mgmt dmad coherent!\n", __func__);
}
