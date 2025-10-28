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
	wed.c
*/
#include <linux/pci.h>
#include "wed.h"
#include "warp.h"
#include "warp_bm.h"
#include "warp_hw.h"
#include "warp_utility.h"

/*
*define local function
*/
/*include from warp_ser.c*/
extern int wed_ser_init(struct wed_entry *wed);
extern void wed_ser_exit(struct wed_entry *wed);

/*
*
*/
static void
dump_tx_ring(struct warp_ring *ring)
{
	warp_dbg(WARP_DBG_OFF, "hw_desc_base\t: 0x%x\n", ring->hw_desc_base);
	warp_dbg(WARP_DBG_OFF, "hw_cidx_addr\t: 0x%x\n", ring->hw_cidx_addr);
	warp_dbg(WARP_DBG_OFF, "hw_didx_addr\t: 0x%x\n", ring->hw_didx_addr);
	warp_dbg(WARP_DBG_OFF, "hw_cnt_addr\t: 0x%X\n", ring->hw_cnt_addr);
	warp_dbg(WARP_DBG_OFF, "ring_lens\t: %d\n", ring->ring_lens);
}

/*
*
*/
static void
dump_buf_res(struct wed_buf_res *res)
{
	warp_dbg(WARP_DBG_OFF, "token_num: %d\n", res->token_num);
	warp_dbg(WARP_DBG_OFF, "pkt_num\t: %d\n", res->pkt_num);
	warp_dbg(WARP_DBG_OFF, "des_buf\n");
	warp_dump_dmabuf(&res->des_buf);
	warp_dbg(WARP_DBG_OFF, "dma_len\t: %d\n", res->dmad_len);
	warp_dbg(WARP_DBG_OFF, "fbuf\n");
	warp_dump_dmabuf(&res->fbuf);
	warp_dbg(WARP_DBG_OFF, "buf_len\t: %d\n", res->fd_len);
	warp_dbg(WARP_DBG_OFF, "pkt_len\t: %d\n", res->pkt_len);
}

/*
*
*/
static void
dump_tx_ring_ctrl_basic(struct wed_tx_ring_ctrl *ring_ctrl)
{
	warp_dbg(WARP_DBG_OFF, "txd_len\t: %d\n", ring_ctrl->txd_len);
	warp_dbg(WARP_DBG_OFF, "ring_num\t: %d\n", ring_ctrl->ring_num);
	warp_dbg(WARP_DBG_OFF, "desc\t:\n");
	warp_dump_dmabuf(ring_ctrl->desc);
}

/*
*
*/
static void
dump_tx_ring_ctrl(struct wed_tx_ring_ctrl *ring_ctrl)
{
	u8 i;

	dump_tx_ring_ctrl_basic(ring_ctrl);

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		warp_dbg(WARP_DBG_OFF, "ring\t: %d\n", i);
		dump_tx_ring(&ring_ctrl->ring[i]);
	}
}

/*
*
*/
static void
dump_tx_ctrl(struct wed_tx_ctrl *tx_ctrl)
{
	struct wed_buf_res *res = &tx_ctrl->res;
	struct wed_tx_ring_ctrl *ring_ctrl = &tx_ctrl->ring_ctrl;

	warp_dbg(WARP_DBG_OFF, "buf_res\n");
	dump_buf_res(res);
	warp_dbg(WARP_DBG_OFF, "ring_ctrl\n");
	dump_tx_ring_ctrl(ring_ctrl);
}

/*
*
*/
static void
dump_res_ctrl(struct wed_res_ctrl *res_ctrl)
{
	struct wed_tx_ctrl *tx_ctrl = &res_ctrl->tx_ctrl;

	warp_dbg(WARP_DBG_OFF, "tx_ctrl\n");
	dump_tx_ctrl(tx_ctrl);
}

/*
*
*/
static void
tx_dma_cb_exit(struct wed_entry *entry, struct warp_dma_cb *dma_cb)
{
	memset(dma_cb, 0, sizeof(struct warp_dma_cb));
}

/*
* assign tx description for tx ring entry
*/
static int
tx_dma_cb_init(struct wifi_entry *wifi,
	       struct warp_dma_buf *desc,
	       u32 idx,
	       struct warp_dma_cb *dma_cb)
{
	struct warp_txdmad *txd;
	struct wifi_hw *hw = &wifi->hw;

	/* Init Tx Ring Size, Va, Pa variables */
	dma_cb->alloc_size = hw->txd_size;
	dma_cb->alloc_va = desc->alloc_va + (idx * dma_cb->alloc_size);
	dma_cb->alloc_pa = desc->alloc_pa + (idx * dma_cb->alloc_size);
	/* advance to next ring descriptor address */
	txd = (struct warp_txdmad *) dma_cb->alloc_va;
	txd->ctrl = cpu_to_le32(WED_CTL_DMA_DONE);

	return 0;
}

/*
*
*/
static void
tx_ring_exit(
	struct wed_entry *entry,
	struct warp_ring *ring,
	struct warp_dma_buf *desc)
{
	u32 i;

	for (i = 0; i < WED_TX_RING_SIZE; i++)
		tx_dma_cb_exit(entry, &ring->cell[i]);

	warp_dma_buf_free(entry->pdev, desc);
}

/*
*
*/
static int
tx_ring_init(
	struct wed_entry *entry,
	u8 idx,
	struct wed_tx_ring_ctrl *ring_ctrl)
{
	u32 i;
	u32 len;
	struct warp_entry *warp = entry->warp;
	struct warp_ring *ring = &ring_ctrl->ring[idx];
	struct warp_dma_buf *desc = &ring_ctrl->desc[idx];
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	struct ring_ctrl *hw_ring_ctrl = &hw->tx[idx];

	ring->ring_lens = hw_ring_ctrl->lens;
	len = ring_ctrl->txd_len * ring->ring_lens;

	if (warp_dma_buf_alloc(entry->pdev, desc, len) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate desc fail, len=%d\n", __func__, len);
		return -1;
	}

	/*should find wifi cr & control it*/
	warp_tx_ring_get_hw(warp, ring, idx);

	for (i = 0; i < ring->ring_lens; i++)
		tx_dma_cb_init(&warp->wifi, desc, i, &ring->cell[i]);

	return 0;
}

/*
*
*/
static int
tx_ring_reset(
	struct wed_entry *entry,
	u8 idx,
	struct wed_tx_ring_ctrl *ring_ctrl)
{
	u32 i;
	struct warp_entry *warp = entry->warp;
	struct warp_ring *ring = &ring_ctrl->ring[idx];
	struct warp_dma_buf *desc = &ring_ctrl->desc[idx];
	struct warp_dma_cb *dma_cb;

	for (i = 0; i < ring->ring_lens; i++) {
		dma_cb = &ring->cell[i];

		tx_dma_cb_exit(entry, dma_cb);
		tx_dma_cb_init(&warp->wifi, desc, i, dma_cb);
	}

	return 0;
}


/*
*
*/
static void
wed_tx_ring_exit(struct wed_entry *entry, struct wed_tx_ctrl *tx_ctrl)
{
	struct wed_tx_ring_ctrl *ring_ctrl = &tx_ctrl->ring_ctrl;
	u32 len;
	u8 i;

	len = sizeof(struct warp_ring) * ring_ctrl->ring_num;

	/*free skb in ring*/
	for (i = 0; i < ring_ctrl->ring_num; i++)
		tx_ring_exit(entry, &ring_ctrl->ring[i], &ring_ctrl->desc[i]);

	/*free wed tx ring*/
	warp_os_free_mem(ring_ctrl->ring);
	/*free desc*/
	warp_os_free_mem(ring_ctrl->desc);
}

/*
*
*/
static int
wed_tx_ring_init(struct wed_entry *entry, struct wed_tx_ctrl *tx_ctrl)
{
	struct wed_tx_ring_ctrl *ring_ctrl = &tx_ctrl->ring_ctrl;
	struct warp_entry *warp = (struct warp_entry *)entry->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	u32 len;
	u8 i;

	ring_ctrl->ring_num = hw->tx_ring_num;
	ring_ctrl->txd_len = hw->txd_size;
	/* allocate wed tx descript for original chip ring */
	len = sizeof(struct warp_dma_buf) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->desc , len, GFP_ATOMIC);

	if (!ring_ctrl->desc) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate tx desc faild\n", __func__);
		goto err;
	}
	memset(ring_ctrl->desc, 0, len);

	/*allocate wed tx ring, assign initial value */
	len = sizeof(struct warp_ring) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->ring , len, GFP_ATOMIC);

	if (!ring_ctrl->ring) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate tx ring faild\n", __func__);
		goto err;
	}
	memset(ring_ctrl->ring, 0, len);

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		/*initial for "PDMA" TX ring*/
		if (tx_ring_init(entry, i, ring_ctrl) < 0) {
			warp_dbg(WARP_DBG_ERR, "%s(): init tx ring faild\n", __func__);
			goto err;
		}
	}

	warp_dbg(WARP_DBG_INF, "%s():done, ring_num/txd_len: %d/%d\n", __func__,
			ring_ctrl->ring_num, ring_ctrl->txd_len);

	return 0;
err:
	warp_dbg(WARP_DBG_ERR, "%s(): init fail, do ring exit\n", __func__);
	wed_tx_ring_exit(entry, tx_ctrl);
	return -1;
}

int wed_tx_ring_reset(struct wed_entry *entry)
{
	struct wed_res_ctrl *res_ctrl = &entry->res_ctrl;
	struct wed_tx_ctrl *tx_ctrl = &res_ctrl->tx_ctrl;
	u32 i = 0;
	struct wed_tx_ring_ctrl *ring_ctrl = &tx_ctrl->ring_ctrl;

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		/*reset for "PDMA" TX ring*/
		tx_ring_reset(entry, i, ring_ctrl);
	}

	return 0;
}

#ifdef WED_RX_D_SUPPORT
static void
rx_ring_hw_desc_base_spare_exit(struct wed_entry *entry, struct warp_rx_ring *ring)
{
	struct platform_device *pdev = entry->pdev;
	u32 size = WED_CR_SIZE * 4;
#ifdef WED_MEM_LEAK_DBG
	struct _MEM_INFO_LIST_ENTRY *delEntry;

	delEntry = MIListRemove(&MemInfoList, ring->hw_desc_base_va);
	if (!delEntry) {
		warp_dbg(WARP_DBG_ERR, "the memory has not been allocated\n");
		warp_dbg(WARP_DBG_ERR, "mem addr = %p, caller is %pS\n",
				ring->hw_desc_base_va, __builtin_return_address(0));
	}

#endif	/* WED_MEM_LEAK_DBG */

	dma_free_coherent(&pdev->dev, size, ring->hw_desc_base_va, ring->hw_desc_base_pa);
}

static int
rx_ring_hw_desc_base_spare_init(struct wed_entry *entry, struct warp_rx_ring *ring)
{
	struct platform_device *pdev = entry->pdev;
	u32 size = WED_CR_SIZE * 4;

	ring->hw_desc_base_va =
		dma_alloc_coherent(&pdev->dev, size, &ring->hw_desc_base_pa,
				   GFP_KERNEL | GFP_DMA32);

	if (!ring->hw_desc_base_va) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate failed\n", __func__);
		return -1;
	}

	memset(ring->hw_desc_base_va, 0, size);
	ring->hw_desc_base = (u32)ring->hw_desc_base_pa;
	ring->hw_cnt_addr = ring->hw_desc_base + WED_CR_SIZE;
	ring->hw_cidx_addr = ring->hw_cnt_addr + WED_CR_SIZE;
	ring->hw_didx_addr = ring->hw_cidx_addr + WED_CR_SIZE;

#ifdef WED_MEM_LEAK_DBG
	MIListAddHead(&MemInfoList, size, ring->hw_desc_base_va, __builtin_return_address(0));
#endif	/* WED_MEM_LEAK_DBG */

	return 0;
}

int wed_acquire_rx_token(
	struct wed_entry *wed,
	struct warp_dma_cb *cb,
	u32 *token_id)
{
	int ret = -1;
	struct warp_entry *warp = NULL;
	struct wifi_entry *wifi = NULL;
	struct wifi_ops *ops = NULL;

	if (!wed || !wed->warp)
		return ret;

	warp = wed->warp;
	wifi = &warp->wifi;
	ops = wifi->ops;

	if (!ops || !ops->token_rx_dmad_init)
		return ret;

	*token_id = ops->token_rx_dmad_init(wifi->hw.priv,
					    cb->pkt, cb->pkt_size,
					    cb->pkt_va, cb->pkt_pa);

	return 0;
}

int wed_release_rx_token(
	struct wed_entry *wed,
	u32 token_id)
{
	int ret = -1;
	return ret;
}
#endif

#ifdef WED_RX_HW_RRO_2_0
static void
rx_dma_cb_exit(struct wed_entry *entry, struct warp_dma_cb *dma_cb)
{
	/*
	 * packet release by WED_RX_BM SW module, so no need to free here
	 * always assign pNdisPacket as NULL after clear
	 */
	dma_cb->pkt = NULL;
	memset(dma_cb, 0, sizeof(struct warp_dma_cb));
}

static int
rx_dma_cb_init(struct wifi_entry *wifi,
	       struct warp_dma_buf *desc,
	       u32 idx,
	       struct warp_dma_cb *dma_cb)
{
	struct wifi_hw *hw = &wifi->hw;
	struct warp_rxdmad *rxd;

	dma_cb->pkt = NULL;
	/* Init RX Ring Size, Va, Pa variables */
	dma_cb->alloc_size = hw->rxd_size;
	dma_cb->alloc_va = desc->alloc_va + (idx * dma_cb->alloc_size);
	dma_cb->alloc_pa = desc->alloc_pa + (idx * dma_cb->alloc_size);
	rxd = (struct warp_rxdmad *)dma_cb->alloc_va;
	rxd->sdl0 |= (1 << TO_HOST_SHIFT);

	return 0;
}

static void
rx_ring_exit(
	struct wed_entry *entry,
	struct warp_rx_ring *ring,
	u32 ring_len,
	struct warp_dma_buf *desc)
{
	u32 i;

	for (i = 0; i < ring_len; i++)
		rx_dma_cb_exit(entry, &ring->cell[i]);

	warp_os_free_mem(ring->cell);
	warp_dma_buf_free(entry->pdev, desc);

	if (ring->spare)
		rx_ring_hw_desc_base_spare_exit(entry, ring);
}

static int
rx_ring_init(
	struct wed_entry *entry,
	u8 idx,
	struct wed_rx_ring_ctrl *ring_ctrl)
{
	u32 i;
	u32 len = 0;
	struct warp_entry *warp = NULL;
	struct warp_rx_ring *ring = NULL;
	struct warp_dma_buf *desc = NULL;
	struct wifi_entry *wifi = NULL;
	struct wifi_hw *hw = NULL;
	struct ring_ctrl *hw_ring_ctrl = NULL;

	if (!entry || !ring_ctrl) {
		warp_dbg(WARP_DBG_ERR, "%s(): %s null pointer\n", __func__,
			entry ? "ring_ctrl" : "entry");
		goto err_null;
	}

	warp = (struct warp_entry *)entry->warp;
	wifi = &warp->wifi;
	hw =  &wifi->hw;
	hw_ring_ctrl = &hw->rx[idx];

	ring = &ring_ctrl->ring[idx];
	desc = &ring_ctrl->desc[idx];

	if (idx >= hw->rx_ring_num) {
		ring->spare = true;
		ring->ring_lens = WED_MIN_RX_RING_SIZE;

		if (rx_ring_hw_desc_base_spare_init(entry, ring)) {
			warp_dbg(WARP_DBG_ERR, "%s(): allocate base failed\n", __func__);
			goto err_spare_init;
		}
	} else {
		ring->ring_lens = hw_ring_ctrl->lens;
		/*should find wifi cr & control it*/
		warp_rx_data_ring_get_hw(warp, ring, idx);
	}

	len = ring_ctrl->rxd_len * ring->ring_lens;
	if (warp_dma_buf_alloc(entry->pdev, desc, len)) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate desc failed, len=%d\n", __func__, len);
		goto err_dma_buf_alloc;
	}

	len = sizeof(struct warp_dma_cb) * ring->ring_lens;
	warp_os_alloc_mem((unsigned char **)&ring->cell, len, GFP_KERNEL);
	if (!ring->cell) {
		warp_dbg(WARP_DBG_ERR, "%s(): alloc mem for ring failed.\n", __func__);
		goto err_ring_cell_alloc;
	}

	for (i = 0; i < ring->ring_lens; i++)
		rx_dma_cb_init(&warp->wifi, desc, i, &ring->cell[i]);

	return 0;
err_ring_cell_alloc:
	warp_dma_buf_free(entry->pdev, desc);
err_dma_buf_alloc:
	if (ring->spare)
		rx_ring_hw_desc_base_spare_exit(entry, ring);
err_spare_init:
err_null:
	return -1;
}

static int
rx_ring_reset(
	struct wed_entry *entry,
	u8 idx,
	struct wed_rx_ring_ctrl *ring_ctrl)
{
	u32 i;
	struct warp_entry *warp = entry->warp;
	struct warp_rx_ring *ring = &ring_ctrl->ring[idx];
	struct warp_dma_buf *desc = &ring_ctrl->desc[idx];
	struct warp_dma_cb *dma_cb;

	for (i = 0; i < ring->ring_lens; i++) {
		dma_cb = &ring->cell[i];

		rx_dma_cb_exit(entry, dma_cb);
		rx_dma_cb_init(&warp->wifi, desc, i, dma_cb);
	}

	return 0;
}

static void
wed_rx_ring_exit(struct wed_entry *entry, struct wed_rx_ctrl *rx_ctrl)
{
	struct wed_rx_ring_ctrl *ring_ctrl = &rx_ctrl->ring_ctrl;
	u8 i;

	for (i = 0; i < ring_ctrl->ring_num; i++)
		rx_ring_exit(entry, &ring_ctrl->ring[i],
			    ring_ctrl->ring[i].ring_lens, &ring_ctrl->desc[i]);

	/*free wed rx ring*/
	warp_os_free_mem(ring_ctrl->ring);
	/*free desc*/
	warp_os_free_mem(ring_ctrl->desc);
}

static int
wed_rx_ring_init(struct wed_entry *entry, struct wed_rx_ctrl *rx_ctrl)
{
	struct wed_rx_ring_ctrl *ring_ctrl = &rx_ctrl->ring_ctrl;
	struct warp_entry *warp = (struct warp_entry *)entry->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	u8 min_ring_num = warp_get_min_rx_data_ring_num();
	u32 len;
	u8 i;

	ring_ctrl->ring_num = hw->rx_ring_num > min_ring_num ?
		hw->rx_ring_num : min_ring_num;
	ring_ctrl->rxd_len = hw->rxd_size;

	/* allocate wed rx descriptor for chip ring */
	len = sizeof(struct warp_dma_buf) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->desc , len, GFP_ATOMIC);

	if (!ring_ctrl->desc) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate rx desc faild\n", __func__);
		goto err;
	}
	memset(ring_ctrl->desc, 0, len);

	/*allocate wed rx ring, assign initial value */
	len = sizeof(struct warp_rx_ring) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->ring , len, GFP_ATOMIC);

	if (!ring_ctrl->ring) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate rx ring faild\n", __func__);
		goto err;
	}
	memset(ring_ctrl->ring, 0, len);

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		if (rx_ring_init(entry, i, ring_ctrl) < 0) {
			warp_dbg(WARP_DBG_ERR, "%s(): init rx ring faild\n", __func__);
			goto err;
		}
	}

	warp_dbg(WARP_DBG_INF, "%s():done, ring_num/rxd_len: %d/%d\n", __func__,
			ring_ctrl->ring_num, ring_ctrl->rxd_len);

	return 0;
err:
	warp_dbg(WARP_DBG_ERR, "%s(): init fail, do ring exit\n", __func__);
	wed_rx_ring_exit(entry, rx_ctrl);
	return -1;
}

int wed_rx_ring_reset(struct wed_entry *entry)
{
	struct wed_res_ctrl *res_ctrl = &entry->res_ctrl;
	struct wed_rx_ctrl *rx_ctrl = &res_ctrl->rx_ctrl;
	struct wed_rx_ring_ctrl *ring_ctrl = &rx_ctrl->ring_ctrl;
	u32 i;

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		/* reset for "WFDMA" RX ring */
		rx_ring_reset(entry, i, ring_ctrl);
	}

	return 0;
}

static int
wed_rro_exit(struct wed_entry *wed)
{
	struct wed_rro_ctrl *rro_ctrl = &wed->res_ctrl.rx_ctrl.rro_ctrl;

	warp_dma_buf_free(wed->pdev, &rro_ctrl->rro_que);

	return 0;
}
#endif /* WED_RX_HW_RRO_2_0 */

#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
static void
wed_rx_rro_data_ring_exit(struct wed_entry *entry, struct wed_rx_ctrl *rx_ctrl)
{
	struct wed_rx_ring_ctrl *ring_ctrl = &rx_ctrl->rro_data_ring_ctrl;
	struct warp_rx_ring *ring;
	struct warp_dma_buf *desc;
	int i;

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ring = &ring_ctrl->ring[i];
		desc = &ring_ctrl->desc[i];

		if (ring->spare) {
			warp_dma_buf_free(entry->pdev, desc);
			rx_ring_hw_desc_base_spare_exit(entry, ring);
		}
	}

	warp_os_free_mem(ring_ctrl->ring);
	warp_os_free_mem(ring_ctrl->desc);
}

int
wed_rx_rro_data_ring_init(struct wed_entry *entry,
				struct wed_rx_ctrl *rx_ctrl)
{
	struct wed_rx_ring_ctrl *ring_ctrl = &rx_ctrl->rro_data_ring_ctrl;
	struct warp_entry *warp = (struct warp_entry *)entry->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	struct warp_rx_ring *ring;
	struct warp_dma_buf *desc;
	u8 min_ring_num = warp_get_min_rx_rro_data_ring_num();
	u32 len;
	int i;

	ring_ctrl->ring_num = hw->rx_rro_data_ring_num > min_ring_num ?
		hw->rx_rro_data_ring_num : min_ring_num;
	ring_ctrl->rxd_len = hw->rxd_size;

	/* allocate wed rx descriptor for chip ring */
	len = sizeof(struct warp_dma_buf) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->desc, len, GFP_ATOMIC);

	if (!ring_ctrl->desc) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate rx desc failed\n", __func__);
		goto err_desc_alloc;
	}
	memset(ring_ctrl->desc, 0, len);

	/*allocate wed rx ring, assign initial value */
	len = sizeof(struct warp_rx_ring) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->ring , len, GFP_ATOMIC);

	if (!ring_ctrl->ring) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate rx rro data ring failed\n", __func__);
		goto err_ring_alloc;
	}
	memset(ring_ctrl->ring, 0, len);

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ring = &ring_ctrl->ring[i];
		desc = &ring_ctrl->desc[i];

		if (i >= hw->rx_rro_data_ring_num) {
			ring->spare = true;
			ring->ring_lens = WED_MIN_RX_RING_SIZE;

			if (rx_ring_hw_desc_base_spare_init(entry, ring)) {
				warp_dbg(WARP_DBG_ERR, "%s(): allocate base failed\n", __func__);
				goto err_spare_init;
			}

			len = ring_ctrl->rxd_len * ring->ring_lens;
			if (warp_dma_buf_alloc(entry->pdev, desc, len)) {
				warp_dbg(WARP_DBG_ERR, "%s(): allocate desc failed, len=%d\n",
					__func__, len);
				rx_ring_hw_desc_base_spare_exit(entry, ring);
				goto err_dma_buf_alloc;
			}

			ring->cb_alloc_pa = desc->alloc_pa;
		} else {
			ring = &ring_ctrl->ring[i];
			ring->hw_desc_base = hw->rx_rro[i].base;
			ring->hw_cidx_addr = hw->rx_rro[i].cidx;
			ring->hw_didx_addr = hw->rx_rro[i].didx;
			ring->hw_cnt_addr = hw->rx_rro[i].cnt;
			ring->cb_alloc_pa = hw->rx_rro[i].cb_alloc_pa;
			ring->ring_lens = hw->rx_rro[i].lens;
		}
	}

	warp_dbg(WARP_DBG_INF, "%s():done, ring_num/rxd_len: %d/%d\n", __func__,
		ring_ctrl->ring_num, ring_ctrl->rxd_len);

	return 0;
err_dma_buf_alloc:
err_spare_init:
	for (i -= 1; i >= hw->rx_rro_data_ring_num; i--) {
		ring = &ring_ctrl->ring[i];
		desc = &ring_ctrl->desc[i];

		warp_dma_buf_free(entry->pdev, desc);
		rx_ring_hw_desc_base_spare_exit(entry, ring);
	}

	warp_os_free_mem(ring_ctrl->ring);
err_ring_alloc:
	warp_os_free_mem(ring_ctrl->desc);
err_desc_alloc:
	return -1;
}
#endif


#ifdef WED_RX_HW_RRO_3_0
static void
wed_rx_rro_page_ring_exit(struct wed_entry *entry, struct wed_rx_ctrl *rx_ctrl)
{
	struct wed_rx_ring_ctrl *ring_ctrl = &rx_ctrl->rro_page_ring_ctrl;
	struct warp_rx_ring *ring;
	struct warp_dma_buf *desc;
	int i;

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ring = &ring_ctrl->ring[i];
		desc = &ring_ctrl->desc[i];

		if (ring->spare) {
			warp_dma_buf_free(entry->pdev, desc);
			rx_ring_hw_desc_base_spare_exit(entry, ring);
		}
	}

	warp_os_free_mem(ring_ctrl->ring);
	warp_os_free_mem(ring_ctrl->desc);
}

int
wed_rx_rro_page_ring_init(struct wed_entry *entry,
				struct wed_rx_ctrl *rx_ctrl)
{
	struct wed_rx_ring_ctrl *ring_ctrl = &rx_ctrl->rro_page_ring_ctrl;
	struct warp_entry *warp = (struct warp_entry *)entry->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	struct warp_rx_ring *ring;
	struct warp_dma_buf *desc;
	u8 min_ring_num = warp_get_min_rx_rro_page_ring_num();
	u32 len;
	int i;

	ring_ctrl->ring_num = hw->rx_rro_page_ring_num > min_ring_num ?
		hw->rx_rro_page_ring_num : min_ring_num;
	ring_ctrl->rxd_len = hw->rxd_size;

	/* allocate wed rx descriptor for chip ring */
	len = sizeof(struct warp_dma_buf) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->desc, len, GFP_ATOMIC);

	if (!ring_ctrl->desc) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate rx desc failed\n", __func__);
		goto err_desc_alloc;
	}
	memset(ring_ctrl->desc, 0, len);

	/*allocate wed rx ring, assign initial value */
	len = sizeof(struct warp_rx_ring) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->ring, len, GFP_KERNEL);
	if (!ring_ctrl->ring) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate rx rro data ring failed\n", __func__);
		goto err_ring_alloc;
	}

	memset(ring_ctrl->ring, 0, len);

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ring = &ring_ctrl->ring[i];
		desc = &ring_ctrl->desc[i];

		if (i >= hw->rx_rro_page_ring_num) {
			ring->spare = true;
			ring->ring_lens = WED_MIN_RX_RING_SIZE;

			if (rx_ring_hw_desc_base_spare_init(entry, ring)) {
				warp_dbg(WARP_DBG_ERR, "%s(): allocate base failed\n", __func__);
				goto err_spare_init;
			}

			len = ring_ctrl->rxd_len * ring->ring_lens;
			if (warp_dma_buf_alloc(entry->pdev, desc, len)) {
				warp_dbg(WARP_DBG_ERR, "%s(): allocate desc failed, len=%d\n",
					__func__, len);
				rx_ring_hw_desc_base_spare_exit(entry, ring);
				goto err_dma_buf_alloc;
			}

			ring->cb_alloc_pa = desc->alloc_pa;
		} else {
			ring->hw_desc_base = hw->rx_rro_pg[i].base;
			ring->hw_cidx_addr = hw->rx_rro_pg[i].cidx;
			ring->hw_didx_addr = hw->rx_rro_pg[i].didx;
			ring->hw_cnt_addr = hw->rx_rro_pg[i].cnt;
			ring->cb_alloc_pa = hw->rx_rro_pg[i].cb_alloc_pa;
			ring->ring_lens = hw->rx_rro_pg[i].lens;
		}
	}

	warp_dbg(WARP_DBG_INF, "%s():done, ring_num/rxd_len: %d/%d\n", __func__,
			ring_ctrl->ring_num, ring_ctrl->rxd_len);

	return 0;
err_dma_buf_alloc:
err_spare_init:
	for (i -= 1; i >= hw->rx_rro_page_ring_num; i--) {
		ring = &ring_ctrl->ring[i];
		desc = &ring_ctrl->desc[i];

		warp_dma_buf_free(entry->pdev, desc);
		rx_ring_hw_desc_base_spare_exit(entry, ring);
	}

	warp_os_free_mem(ring_ctrl->ring);
err_ring_alloc:
	warp_os_free_mem(ring_ctrl->desc);
err_desc_alloc:
	return -1;
}

static void
wed_rx_rro_ind_cmd_ring_exit(struct wed_entry *entry, struct wed_rx_ctrl *rx_ctrl)
{
	struct wed_rx_ring_ctrl *ring_ctrl = &rx_ctrl->rro_ind_cmd_ring_ctrl;

	warp_os_free_mem(ring_ctrl->ring);
}

int
wed_rx_rro_ind_cmd_ring_init(struct wed_entry *entry,
				struct wed_rx_ctrl *rx_ctrl)
{
	struct wed_rx_ring_ctrl *ring_ctrl = &rx_ctrl->rro_ind_cmd_ring_ctrl;
	struct warp_entry *warp = (struct warp_entry *)entry->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	struct warp_rx_ring *ring;
	u32 len;
	u8 i;

	ring_ctrl->ring_num = hw->rx_rro_ind_cmd_ring_num;
	ring_ctrl->rxd_len = hw->rxd_size;

	/*allocate wed rx ring, assign initial value */
	len = sizeof(struct warp_rx_ring) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->ring , len, GFP_ATOMIC);

	if (!ring_ctrl->ring) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate rx rro data ring faild\n", __func__);
		goto err;
	}
	memset(ring_ctrl->ring, 0, len);

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ring = &ring_ctrl->ring[i];
		ring->hw_desc_base = hw->rx_rro_ind_cmd.base;
		ring->hw_cidx_addr = hw->rx_rro_ind_cmd.cidx;
		ring->hw_didx_addr = hw->rx_rro_ind_cmd.didx;
		ring->hw_cnt_addr = hw->rx_rro_ind_cmd.cnt;
		ring->cb_alloc_pa = hw->rx_rro_ind_cmd.cb_alloc_pa;
		ring->ring_lens = hw->rx_rro_ind_cmd.lens;
	}

	warp_dbg(WARP_DBG_INF, "%s():done, ring_num/rxd_len: %d/%d\n", __func__,
			ring_ctrl->ring_num, ring_ctrl->rxd_len);

	return 0;
err:
	warp_dbg(WARP_DBG_ERR, "%s(): init fail, do ring exit\n", __func__);
	wed_rx_rro_ind_cmd_ring_exit(entry, rx_ctrl);
	return -1;


}
#endif /* WED_RX_HW_RRO_3_0 */

#ifdef WED_RX_HW_RRO_3_1
static void
wed_rx_rro_3_1_ring_exit(struct wed_entry *entry, struct wed_rx_ctrl *rx_ctrl)
{
	struct wed_rx_ring_ctrl *ring_ctrl = &rx_ctrl->rro_3_1_ring_ctrl;

	warp_os_free_mem(ring_ctrl->ring);
	warp_os_free_mem(ring_ctrl->desc);
}

int
wed_rx_rro_3_1_ring_init(struct wed_entry *entry,
				struct wed_rx_ctrl *rx_ctrl)
{
	struct wed_rx_ring_ctrl *ring_ctrl = &rx_ctrl->rro_3_1_ring_ctrl;
	struct warp_entry *warp = (struct warp_entry *)entry->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	struct warp_rx_ring *ring;
	u32 len, i;

	ring_ctrl->ring_num = hw->rx_rro_3_1_ring_num;
	ring_ctrl->rxd_len = hw->rxd_size;

	/* allocate wed rx descriptor for chip ring */
	len = sizeof(struct warp_dma_buf) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->desc, len, GFP_ATOMIC);

	if (!ring_ctrl->desc) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate rx desc failed\n", __func__);
		goto err_desc_alloc;
	}
	memset(ring_ctrl->desc, 0, len);

	/*allocate wed rx ring, assign initial value */
	len = sizeof(struct warp_rx_ring) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->ring, len, GFP_ATOMIC);

	if (!ring_ctrl->ring) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate rx rro data ring failed\n", __func__);
		goto err_ring_alloc;
	}
	memset(ring_ctrl->ring, 0, len);

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ring = &ring_ctrl->ring[i];

		ring->hw_desc_base = hw->rx_rro_3_1.base;
		ring->hw_cidx_addr = hw->rx_rro_3_1.cidx;
		ring->hw_didx_addr = hw->rx_rro_3_1.didx;
		ring->hw_cnt_addr = hw->rx_rro_3_1.cnt;
		ring->cb_alloc_pa = hw->rx_rro_3_1.cb_alloc_pa;
		ring->ring_lens = hw->rx_rro_3_1.lens;
	}

	warp_dbg(WARP_DBG_INF, "%s():done, ring_num/rxd_len: %d/%d\n", __func__,
		ring_ctrl->ring_num, ring_ctrl->rxd_len);

	return 0;
err_ring_alloc:
	warp_os_free_mem(ring_ctrl->desc);
err_desc_alloc:
	return -ENOMEM;
}
#endif /* WED_RX_HW_RRO_3_1 */

#ifdef WED_HIF_TXD_SUPPORT
int
hif_txd_init(struct wed_entry *wed)
{
	struct wed_hif_txd_ctrl *hif_txd_ctrl = &wed->res_ctrl.tx_ctrl.hif_txd_ctrl;
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	struct pci_dev *wdev = hw->hif_dev;
	int i = 0;

	/* TODO: set segment_num */
	hif_txd_ctrl->hif_txd_segment_nums = 32;
	hif_txd_ctrl->hif_txd_src = hw->src;

	for (i = 0; i < hif_txd_ctrl->hif_txd_segment_nums; i++) {
		/* each segment is 64K */
		hif_txd_ctrl->hif_txd_addr[i] =
			(void *)__get_free_pages(GFP_ATOMIC | __GFP_ZERO, 4);

		if (!hif_txd_ctrl->hif_txd_addr[i])
			goto err_alloc;

#ifdef WED_MEM_LEAK_DBG
		pkt_alloc_mem(64*1024, hif_txd_ctrl->hif_txd_addr[i], __builtin_return_address(0));
#endif/* WED_MEM_LEAK_DBG */

		hif_txd_ctrl->hif_txd_addr_pa[i] =
			dma_map_single(&wdev->dev, hif_txd_ctrl->hif_txd_addr[i],
					64 * 1024, DMA_TO_DEVICE);

		if (unlikely(dma_mapping_error(&wdev->dev, hif_txd_ctrl->hif_txd_addr_pa[i]))) {
			warp_dbg(WARP_DBG_ERR, "%s(): dma map fail\n", __func__);
			goto err_dma_map;
		}
	}

	hif_txd_ctrl->hif_txd_segment_nums = i;

	return 0;

err_dma_map:
#ifdef WED_MEM_LEAK_DBG
	pkt_free_mem(hif_txd_ctrl->hif_txd_addr[i]);
#endif
	free_pages((unsigned long)hif_txd_ctrl->hif_txd_addr[i], 4);
err_alloc:
	for (i -= 1; i >= 0; i--) {
		dma_unmap_single(&wdev->dev, hif_txd_ctrl->hif_txd_addr_pa[i],
				64 * 1024, DMA_TO_DEVICE);
#ifdef WED_MEM_LEAK_DBG
		pkt_free_mem(hif_txd_ctrl->hif_txd_addr[i]);
#endif
		free_pages((unsigned long)hif_txd_ctrl->hif_txd_addr[i], 4);
	}

	return -ENOMEM;
}

void
hif_txd_exit(struct wed_entry *wed)
{
	int i;
	struct wed_hif_txd_ctrl *hif_txd_ctrl = &wed->res_ctrl.tx_ctrl.hif_txd_ctrl;
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	struct pci_dev *wdev = hw->hif_dev;

	for (i = 0; i < hif_txd_ctrl->hif_txd_segment_nums; i++) {

		dma_unmap_single(&wdev->dev, hif_txd_ctrl->hif_txd_addr_pa[i],
				64 * 1024, DMA_TO_DEVICE);
#ifdef WED_MEM_LEAK_DBG
		pkt_free_mem(hif_txd_ctrl->hif_txd_addr[i]);
#endif
		free_pages((unsigned long)hif_txd_ctrl->hif_txd_addr[i], 4);
	}
};

#endif

/*
*
*/
static int
wed_ring_init(struct wed_entry *entry)
{
	struct wed_res_ctrl *res_ctrl = &entry->res_ctrl;
	struct wed_tx_ctrl *tx_ctrl = &res_ctrl->tx_ctrl;
#ifdef WED_RX_D_SUPPORT
	struct wed_rx_ctrl *rx_ctrl = &res_ctrl->rx_ctrl;
#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	struct warp_entry *warp = entry->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	bool wifi_cap_rro_3_0 = hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_0);
	bool wifi_cap_rro_3_1 = hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_1);
#endif /* WED_RX_HW_RRO_3_0 || WED_RX_HW_RRO_3_1 */
#endif /* WED_RX_D_SUPPORT */
	int ret = 0;

	wed_tx_ring_init(entry, tx_ctrl);
#ifdef WED_RX_D_SUPPORT
#ifdef WED_RX_HW_RRO_2_0
	wed_rx_ring_init(entry, rx_ctrl);
#endif
#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	if (wifi_cap_rro_3_0 || wifi_cap_rro_3_1)
		wed_rx_rro_data_ring_init(entry, rx_ctrl);
#endif
#ifdef WED_RX_HW_RRO_3_0
	if (wifi_cap_rro_3_0) {
		wed_rx_rro_page_ring_init(entry, rx_ctrl);
		wed_rx_rro_ind_cmd_ring_init(entry, rx_ctrl);
	}
#endif
#ifdef WED_RX_HW_RRO_3_1
	if (wifi_cap_rro_3_1)
		wed_rx_rro_3_1_ring_init(entry, rx_ctrl);
#endif /* WED_RX_HW_RRO_3_1 */
#endif /* WED_RX_D_SUPPORT */

	return ret;
}

/*
*
*/
static void
wed_ring_exit(struct wed_entry *entry)
{
	struct wed_res_ctrl *res_ctrl = &entry->res_ctrl;
	struct wed_tx_ctrl *tx_ctrl = &res_ctrl->tx_ctrl;
#ifdef WED_RX_D_SUPPORT
	struct wed_rx_ctrl *rx_ctrl = &res_ctrl->rx_ctrl;
#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	struct warp_entry *warp = entry->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	bool wifi_cap_rro_3_0 = hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_0);
	bool wifi_cap_rro_3_1 = hw->hw_cap & BIT(WIFI_HW_CAP_RRO_3_1);
#endif /* WED_RX_HW_RRO_3_0 || WED_RX_HW_RRO_3_1 */
#endif /* WED_RX_D_SUPPORT */

	wed_tx_ring_exit(entry, tx_ctrl);
#ifdef WED_RX_D_SUPPORT
#ifdef WED_RX_HW_RRO_2_0
	wed_rx_ring_exit(entry, rx_ctrl);
#endif

#if defined(WED_RX_HW_RRO_3_0) || defined(WED_RX_HW_RRO_3_1)
	if (wifi_cap_rro_3_0 || wifi_cap_rro_3_1)
		wed_rx_rro_data_ring_exit(entry, rx_ctrl);
#endif
#ifdef WED_RX_HW_RRO_3_0
	if (wifi_cap_rro_3_0) {
		wed_rx_rro_page_ring_exit(entry, rx_ctrl);
		wed_rx_rro_ind_cmd_ring_exit(entry, rx_ctrl);
	}
#endif
#ifdef WED_RX_HW_RRO_3_1
	if (wifi_cap_rro_3_1)
		wed_rx_rro_3_1_ring_exit(entry, rx_ctrl);
#endif
#endif /* WED_RX_D_SUPPORT */
}

int wed_fdesc_init(
	struct wed_entry *wed,
	struct wed_pkt_info *info)
{
	struct warp_entry *warp = NULL;
	struct wifi_entry *wifi = NULL;
	struct warp_txdmad *txdma = NULL;
	int ret = -1;
#ifdef CONFIG_WARP_64BIT_SUPPORT
	u32 phy_add_h = 0;
#endif
	u32 phy_add_l = 0;

	if (wed)
		warp = (struct warp_entry *)wed->warp;
	else
		goto err;

	if (warp && info) {
		u32 ctrl = 0;

		wifi = &warp->wifi;

		txdma = (struct warp_txdmad *)info->desc_va;

		ctrl = (info->fd_len << WED_CTL_SD_LEN0_SHIFT) & WED_CTL_SD_LEN0;
		ctrl |= info->len & WED_CTL_SD_LEN1;
		if (wed->ver >= 2) {
			ctrl |= WED_CTL_LAST_SEC0;
		} else {
			ctrl &= ~WED_CTL_LAST_SEC0;
			ctrl |= WED_CTL_LAST_SEC1;
			ctrl &= ~WED_CTL_DMA_DONE;
			ctrl &= ~WED_CTL_BURST;
		}
		WRITE_ONCE(txdma->ctrl, cpu_to_le32(ctrl));

		/* Set SDP0_L to wfdma_txdmad dw0 */
		phy_add_l = info->fdesc_pa & WARP_TXDMAD_SDP0_L_MASK;
		WRITE_ONCE(txdma->sdp0, cpu_to_le32(phy_add_l));

		/* Set SDP1_L to wfdma_txdmad dw0 */
		phy_add_l = info->pkt_pa & WARP_TXDMAD_SDP1_L_MASK;
		WRITE_ONCE(txdma->sdp1, cpu_to_le32(phy_add_l));

#ifdef CONFIG_WARP_64BIT_SUPPORT
		/* Set SDP0_H and SDP1_H to wfdma_txdmad dw3 */
		phy_add_h = (info->fdesc_pa >> WARP_DMA_ADDR_H_SHIFT) & WARP_TXDMAD_SDP0_H_MASK;
		phy_add_h |= ((info->pkt_pa >> WARP_DMA_ADDR_H_SHIFT) &
				WARP_TXDMAD_SDP1_H_MASK) << WARP_TXDMAD_SDP1_H_SHIFT;
		WRITE_ONCE(txdma->info, cpu_to_le32(phy_add_h));
#endif
		if (wed->ver >= 2)
			wifi->ops->fbuf_init((unsigned char *)info->fdesc_va, txdma->sdp1, 0);
		else
			wifi->ops->fbuf_v1_init((unsigned char *)info->fdesc_va, txdma->sdp1, info->token_id, 0);

		warp_dbg(WARP_DBG_INF, "%s(): txdmad DW0: 0x%x\n", __func__, txdma->sdp0);
		warp_dbg(WARP_DBG_INF, "%s(): txdmad DW1: 0x%x\n", __func__, txdma->ctrl);
		warp_dbg(WARP_DBG_INF, "%s(): txdmad DW2: 0x%x\n", __func__, txdma->sdp1);
		warp_dbg(WARP_DBG_INF, "%s(): txdmad DW3: 0x%x\n", __func__, txdma->info);
		ret = 0;
	}

err:
	return ret;
}

/*
*
*/
int
wed_init(struct platform_device *pdev, u8 idx, struct wed_entry *wed)
{
	/*assign to pdev*/
	int ret = -1;
	struct resource *res;
	u32 irq;
	unsigned long base_addr;
	struct warp_entry *warp = NULL;

	res = platform_get_resource(pdev, IORESOURCE_MEM, TO_RES_IDX(idx));
	if (!res) {
		warp_dbg(WARP_DBG_ERR, "%s(): resource idx = %u not found\n",
			__func__, idx);
		return -EINVAL;
	}

	irq = platform_get_irq(pdev, TO_RES_IDX(idx));
	base_addr = (unsigned long)devm_ioremap(&pdev->dev, res->start,
						resource_size(res));
	warp_dbg(WARP_DBG_OFF,
		 "%s(%d): res_start = %pa[p], size = %x, irq=%d,base_addr=0x%lx\n",
		 __func__, idx, &res->start, (u32)resource_size(res), irq, base_addr);
	wed->base_addr = base_addr;
	wed->pdev = pdev;
	wed->irq = irq;
	wed->cr_base_addr = res->start;
	/*initial wed hw cap for related decision*/
	warp_conf_hwcap(wed);
	/*allocate ring first*/
	wed_ring_init(wed);
	/*assign tx ring to ad*/
	if (wed->warp)
		warp = (struct warp_entry *)wed->warp;
	else
		goto err;

#ifdef WED_RX_D_SUPPORT
#ifdef WED_RX_HW_RRO_2_0
	warp_wed_rro_init(wed);
#endif
#endif

#ifdef WED_DYNAMIC_TXBM_SUPPORT
	if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_TXBM))
		regist_dl_dybm_task(wed);
#endif	/* WED_DYNAMIC_TXBM_SUPPORT */
	wed_ser_init(wed);
	ret = 0;

err:
	return ret;
}

/*
*
*/
void
wed_exit(struct platform_device *pdev, struct wed_entry *wed)
{
	wed_ser_exit(wed);
#ifdef WED_TX_SUPPORT
	wed_ring_exit(wed);
#endif

#ifdef WED_RX_D_SUPPORT
#ifdef WED_RX_HW_RRO_2_0
	wed_rro_exit(wed);
#endif
#endif

#ifdef WED_DYNAMIC_TXBM_SUPPORT
	if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_TXBM))
		unregist_dl_dybm_task(wed);
#endif	/* WED_DYNAMIC_TXBM_SUPPORT */

	/*assign to pdev*/
	devm_iounmap(&pdev->dev, (void *)wed->base_addr);
	memset(wed, 0, sizeof(*wed));
}

/*
*
*/
u32
wed_get_entry_num(void)
{
	struct device_node *node = NULL;
	u32 num = 0;

	node = of_find_compatible_node(NULL, NULL, WED_DEV_NODE);

	if (of_property_read_u32_index(node, "wed_num", 0, &num)) {
		warp_dbg(WARP_DBG_ERR, "%s(): get WED number from DTS fail!!\n", __func__);
		return 0;
	}

	return num;
}

/*
*
*/
u8
wed_get_slot_map(unsigned int idx)
{
	struct device_node *node = NULL;
	u32 num = 0;

	node = of_find_compatible_node(NULL, NULL, TO_NODE(idx));

	if (of_property_read_u32_index(node, "pci_slot_map", TO_RES_IDX(idx), &num)) {
		warp_dbg(WARP_DBG_OFF, "%s(): get wed slot from DTS fail!!\n", __func__);
		warp_dbg(WARP_DBG_OFF, "%s(): assign default value: (0->1), (1->2)!!\n",
			 __func__);
		num = (idx == 0) ? 1 : 2;
	}
	warp_dbg(WARP_DBG_OFF,
		 "%s(): assign slot_id:%d for entry: %d!\n", __func__, num, idx);
	return (u8)num;
}

/*
*
*/
void
dump_wed_basic(struct wed_entry *wed)
{
	warp_dbg(WARP_DBG_OFF, "virtual addr\t: 0x%lx\n", wed->base_addr);
	warp_dbg(WARP_DBG_OFF, "irq\t: %d\n", wed->irq);
	dump_res_ctrl(&wed->res_ctrl);
}

/*
*
*/
static void
dump_tx_ring_raw(struct wed_entry *wed, u8 ring_id, u32 idx)
{
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct warp_ring *ring = &wed->res_ctrl.tx_ctrl.ring_ctrl.ring[ring_id];
	u8 *addr;
	u32 size;

	warp_dbg(WARP_DBG_OFF, "==========WED TX RING RAW (%d/0x%x)==========\n",
		 ring_id, idx);
	wifi_tx_ring_info_dump(wifi, ring_id, idx);
	/*WED_WPDMA Tx Ring content*/
	warp_dbg(WARP_DBG_OFF, "==========WPDAM TX RING RAW (%d/0x%x)==========\n",
		 ring_id, idx);
	warp_dump_dmacb(&ring->cell[idx]);
	addr = (unsigned char *) ring->cell[idx].alloc_va;
	size = ring->cell[idx].alloc_size;
	warp_dump_raw("WPDMA_TX_RING", addr, size);
}

/*
*  Debug purpose: dump ser debug cnt
*/
void
dump_wed_ser_dbg_cnt(struct wed_entry *wed)
{
	struct wed_ser_ctrl *ser_ctrl = &wed->ser_ctrl;
	struct wed_ser_moudle_busy_cnt *busy_cnt;

	busy_cnt = &ser_ctrl->ser_busy_cnt;

	warp_dbg(WARP_DBG_OFF, "==========WED TX SER CNT==========\n");
	warp_dbg(WARP_DBG_OFF, "reset_wed_tx_dma\t: %d\n", busy_cnt->reset_wed_tx_dma);
	warp_dbg(WARP_DBG_OFF, "reset_wed_wdma_rx_drv\t: %d\n", busy_cnt->reset_wed_wdma_rx_drv);
	warp_dbg(WARP_DBG_OFF, "reset_wdma_rx\t: %d\n", busy_cnt->reset_wdma_rx);
	warp_dbg(WARP_DBG_OFF, "reset_wed_tx_bm\t: %d\n", busy_cnt->reset_wed_tx_bm);
	warp_dbg(WARP_DBG_OFF, "reset_wed_wpdma_tx_drv\t: %d\n", busy_cnt->reset_wed_wpdma_tx_drv);
	warp_dbg(WARP_DBG_OFF, "reset_wed_rx_drv\t: %d\n", busy_cnt->reset_wed_rx_drv);
	warp_dbg(WARP_DBG_OFF, "==========WED RX SER CNT==========\n");
	warp_dbg(WARP_DBG_OFF, "reset_wed_wpdma_rx_d_drv\t: %d\n", busy_cnt->reset_wed_wpdma_rx_d_drv);
	warp_dbg(WARP_DBG_OFF, "reset_wed_rx_rro_qm\t: %d\n", busy_cnt->reset_wed_rx_rro_qm);
	warp_dbg(WARP_DBG_OFF, "reset_wed_rx_route_qm\t: %d\n", busy_cnt->reset_wed_rx_route_qm);
	warp_dbg(WARP_DBG_OFF, "reset_wdma_tx\t: %d\n", busy_cnt->reset_wdma_tx);
	warp_dbg(WARP_DBG_OFF, "reset_wdma_tx_drv\t: %d\n", busy_cnt->reset_wdma_tx_drv);
	warp_dbg(WARP_DBG_OFF, "reset_wed_rx_dma\t: %d\n", busy_cnt->reset_wed_rx_dma);
	warp_dbg(WARP_DBG_OFF, "reset_wed_rx_bm\t: %d\n", busy_cnt->reset_wed_rx_bm);
	warp_dbg(WARP_DBG_OFF, "==========WED SER DETECT CNT==========\n");
	warp_dbg(WARP_DBG_OFF, "wpdma_ser_cnt\t: %d\n", ser_ctrl->wpdma_ser_cnt);
	warp_dbg(WARP_DBG_OFF, "tx_bm_ser_cnt\t: %d\n", ser_ctrl->tx_bm_ser_cnt);

}

/*
*
*/
void
wed_proc_handle(struct wed_entry *wed, char choice, char *arg)
{
	u32 i;
	char *str;
	char *end;
	u8 idx = 0;
#ifdef WED_PAO_SUPPORT
	u8 amsdu_num = 0;
	u16 wcid = 0;
	u32 pao_cr_value = 0, amsdu_len = 0;
	int vlan, hdrt = 0;
#endif
#ifdef WED_RX_HW_RRO_3_0
	u16 se_id;
#endif
	struct wed_res_ctrl *res_ctrl = &wed->res_ctrl;
	struct wed_tx_ctrl *tx_ctrl = &res_ctrl->tx_ctrl;
	struct wed_buf_res *buf_res = &tx_ctrl->res;

	switch (choice) {
	case WED_PROC_TX_RING_BASIC:
		dump_wed_basic(wed);
		break;

	case WED_PROC_TX_BUF_BASIC:
		dump_buf_res(buf_res);
		break;
#ifndef WED_BMU_SUPPORT
	case WED_PROC_TX_BUF_INFO:
		dump_pkt_info(buf_res);
		break;
#endif
	case WED_PROC_TX_RING_CELL: {
		str = strsep(&arg, " ");
		if (str == NULL)
			break;

		str = strsep(&arg, " ");
		if (str == NULL)
			break;
		idx =  warp_str_tol(str, &end, 10);

		str = strsep(&arg, " ");
		if (str == NULL)
			break;
		i =  warp_str_tol(str, &end, 16);
		dump_tx_ring_raw(wed, idx, i);
	}
	break;

	case WED_PROC_DBG_INFO:
		warp_dbginfo_dump_wed_hw(wed);
		break;
#ifdef WED_DYNAMIC_TXBM_SUPPORT
	case WED_PROC_TX_DYNAMIC_ALLOC: {
		if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_TXBM))
			buf_alloc_task((unsigned long)wed);
		else
			warp_dbg(WARP_DBG_ERR, "DYBM is not enabled!\n");
	}
	break;

	case WED_PROC_TX_DYNAMIC_FREE: {
		if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_TXBM))
			buf_free_task((unsigned long)wed);
		else
			warp_dbg(WARP_DBG_ERR, "DYBM is not enabled!\n");
	}
	break;
#endif	/* WED_DYNAMIC_TXBM_SUPPORT */
#ifdef WED_DYNAMIC_RXBM_SUPPORT
	case WED_PROC_RX_DYNAMIC_ALLOC: {
		if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_RXBM))
			rxbm_alloc_handler((unsigned long)wed);
		else
			warp_dbg(WARP_DBG_ERR, "DYBM is not enabled!\n");
	}
	break;

	case WED_PROC_RX_DYNAMIC_FREE: {
		if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_RXBM))
			rxbm_recycle_handler((unsigned long)wed);
		else
			warp_dbg(WARP_DBG_ERR, "DYBM is not enabled!\n");
	}
	break;
#endif	/* WED_DYNAMIC_TXBM_SUPPORT */

#ifdef WED_HW_TX_SUPPORT
	case WED_PROC_TX_FREE_CNT:
		warp_bfm_get_tx_freecnt_hw(wed, &i);
		warp_dbg(WARP_DBG_ERR, "WED_PROC_TX_FREE_CNT enabled!\n");
		break;
#endif	/* WED_HW_TX_SUPPORT */
	case WED_PROC_TX_RESET:
	case WED_PROC_RX_RESET:
		warp_reset_hw(wed, WARP_RESET_IDX_ONLY);
		break;

#if defined(CONFIG_WARP_HW_DDR_PROF)
#if defined(WED_HW_TX_SUPPORT)
	case WED_PROC_TX_DDR_PROF:
		wed_dram_prof(wed, WED_DRAM_PROF_TX);
		break;
#endif	/* WED_HW_TX_SUPPORT */
#if defined(WED_HW_RX_SUPPORT)
	case WED_PROC_RX_DDR_PROF:
		wed_dram_prof(wed, WED_DRAM_PROF_RX);
		break;
#endif	/* WED_PROC_TX_DDR_PROF */
	case WED_PROC_OFF_DDR_PROF:
		wed_dram_prof(wed, WED_DRAM_PROF_OFF);
		break;
#endif	/* CONFIG_WARP_HW_DDR_PROF */
#if defined(WED_DYNAMIC_TXBM_SUPPORT) || defined(WED_DYNAMIC_RXBM_SUPPORT)
	case WED_PROC_TXBM_INFO:
		if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_TXBM)) {
			struct list_head *cur = NULL, *next = NULL;
			u32 idx = 0;

			warp_dbg(WARP_DBG_OFF, "Dynamic TXBM is enablded!");
			warp_dbg(WARP_DBG_OFF, "\tOccupied:%d Budget:%d Total:%d\n",
						wed->res_ctrl.tx_ctrl.res.bm_vld_grp, wed->res_ctrl.tx_ctrl.res.budget_grp,
						wed->res_ctrl.tx_ctrl.res.pkt_num/wed->res_ctrl.tx_ctrl.res.bm_grp_sz);
			warp_dbg(WARP_DBG_OFF, "\tBM group info:\n");
			list_for_each_safe(cur, next, &buf_res->grp_head) {
				struct wed_bm_group_info *grp_info = list_entry(cur, struct wed_bm_group_info, list);

				warp_dbg(WARP_DBG_OFF, "\t\tGroup[%d] skb start:%u\n", idx, grp_info->skb_id_start);
				idx++;
			}
			warp_dbg(WARP_DBG_OFF, "\tMax. groups:%d, Min. groups:%d\n",
						wed->res_ctrl.tx_ctrl.res.dybm_stat.max_vld_grp,
						wed->res_ctrl.tx_ctrl.res.dybm_stat.min_vld_grp);
			warp_dbg(WARP_DBG_OFF, "\tExtend failed due to exhausted budge:%d\n",
						wed->res_ctrl.tx_ctrl.res.dybm_stat.ext_failed);

			idx = 0;

			warp_dbg(WARP_DBG_OFF, "\tBudget group info:\n");
			list_for_each_safe(cur, next, &buf_res->budget_head) {
				struct wed_bm_group_info *grp_info = list_entry(cur, struct wed_bm_group_info, list);

				warp_dbg(WARP_DBG_OFF, "\t\tGroup[%d] skb start:%u\n", idx, grp_info->skb_id_start);
				idx++;
			}
			warp_dbg(WARP_DBG_OFF, "\tRefill groups:%d, Release groups:%d\n",
						wed->res_ctrl.tx_ctrl.res.dybm_stat.budget_refill,
						wed->res_ctrl.tx_ctrl.res.dybm_stat.budget_release);
			warp_dbg(WARP_DBG_OFF, "\tRefill failed:%d\n",
						wed->res_ctrl.tx_ctrl.res.dybm_stat.budget_refill_failed);
			warp_dbg(WARP_DBG_OFF, "\n\tLow water mark handled:%d\n",
						wed->res_ctrl.tx_ctrl.res.dybm_stat.ext_times);
			warp_dbg(WARP_DBG_OFF, "\tLow water mark dismissed:%d\n",
						wed->res_ctrl.tx_ctrl.res.dybm_stat.ext_unhanlded);
			warp_dbg(WARP_DBG_OFF, "\tHigh water mark handled:%d\n",
						wed->res_ctrl.tx_ctrl.res.dybm_stat.shk_times);
			warp_dbg(WARP_DBG_OFF, "\tHigh water mark dismissed:%d\n",
						wed->res_ctrl.tx_ctrl.res.dybm_stat.shk_unhanlded);
			memset(&wed->res_ctrl.tx_ctrl.res.dybm_stat, 0 ,sizeof(wed->res_ctrl.tx_ctrl.res.dybm_stat));
		} else
			warp_dbg(WARP_DBG_OFF, "Dynamic TXBM is disabled!\n");
		break;
	case WED_PROC_RXBM_INFO:
		if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_RXBM)) {
			struct warp_bm_rxdmad *dmad = NULL;
			struct sw_conf_t *conf = wed->sw_conf;
			struct wed_rx_ctrl *rx_ctrl = &res_ctrl->rx_ctrl;
			struct wed_rx_bm_res *rx_bm_res = &rx_ctrl->res;

			warp_dbg(WARP_DBG_OFF, "Dynamic RXBM is enablded!");
			warp_dbg(WARP_DBG_OFF, "\tOccupied:%d Budget:%d Total:%d\n",
						rx_bm_res->pkt_num/128+rx_ctrl->budget_head_idx*conf->rxbm.alt_quota,
						rx_bm_res->budget_grp,
						rx_bm_res->pkt_num/128+rx_bm_res->budget_grp);
			warp_dbg(WARP_DBG_OFF, "\tBM group info:\n");
			dmad = (struct warp_bm_rxdmad *)rx_ctrl->res.desc[0].alloc_va;
			warp_dbg(WARP_DBG_OFF, "\t\tRing[%d] token start:%u\n", idx, dmad->token >> TOKEN_ID_SHIFT);
			warp_dbg(WARP_DBG_OFF, "\tBudget rings info:\n");
			for (idx = rx_ctrl->budget_head_idx ; idx < rx_ctrl->budget_tail_idx ; idx++) {
				dmad = (struct warp_bm_rxdmad *) rx_ctrl->extra.desc[idx].alloc_va;

				if (rx_ctrl->extra.ring[idx].recycled) {
					warp_dbg(WARP_DBG_OFF, "\t\tRing[%d] token start:%u [R]\n", idx, dmad->token >> TOKEN_ID_SHIFT);
				} else {
					warp_dbg(WARP_DBG_OFF, "\t\tRing[%d] token start:%u\n", idx, dmad->token >> TOKEN_ID_SHIFT);
				}
			}
			warp_dbg(WARP_DBG_OFF, "\tMax. groups:%d, Min. groups:%d\n",
						rx_bm_res->dybm_stat.max_vld_grp,
						rx_bm_res->dybm_stat.min_vld_grp);
			warp_dbg(WARP_DBG_OFF, "\tExtend failed due to exhausted budge:%d\n",
						rx_bm_res->dybm_stat.ext_failed);
			warp_dbg(WARP_DBG_OFF, "\tRefill groups:%d, Release groups:%d\n",
						rx_bm_res->dybm_stat.budget_refill,
						rx_bm_res->dybm_stat.budget_release);
			warp_dbg(WARP_DBG_OFF, "\tLow water mark detected:%d\n",
						rx_bm_res->dybm_stat.ext_times);
			warp_dbg(WARP_DBG_OFF, "\tHigh water mark detected:%d\n",
						rx_bm_res->dybm_stat.shk_times);
			warp_dbg(WARP_DBG_OFF, "\tExtend failed due to insufficient memory:%d\n",
						rx_bm_res->dybm_stat.ext_failed);
			memset(&rx_bm_res->dybm_stat, 0 ,sizeof(rx_bm_res->dybm_stat));
		} else
			warp_dbg(WARP_DBG_OFF, "Dynamic RXBM is disabled!\n");
		break;

#endif	/* WED_DYNAMIC_TXBM_SUPPORT */
	case WED_PROC_SER_ERR_CNT:
		dump_wed_ser_dbg_cnt(wed);
		break;

#ifdef WED_PAO_SUPPORT
	case WED_PROC_PAO_WCID_STAT:

		str = strsep(&arg, " ");
		if (str == NULL)
			break;

		str = strsep(&arg, " ");
		if (str == NULL)
			break;

		wcid =  warp_str_tol(str, &end, 10);

		warp_dbg(WARP_DBG_OFF, "WED_PROC_PAO_WCID_STAT!: wcid:%d\n", wcid);
		warp_pao_get_sta_info(wed, wcid, &pao_cr_value);
		warp_dbg(WARP_DBG_OFF, "%s():wcid = %d\n", __func__, wcid);
		warp_dbg(WARP_DBG_OFF, "%s():amsdu_nums/lens = %d/%d\n", __func__,
				(pao_cr_value & 0x001E0000) >> 17,
				((pao_cr_value & 0x0001F800) >> 11) << 8);
		warp_dbg(WARP_DBG_OFF, "%s():rm_vlan/hdrt = %d/%d\n", __func__,
				(pao_cr_value & 0x00000400) >> 10, (pao_cr_value & 0x00000200) >> 9);
		break;

	case WED_PROC_PAO_SET_WCID_STAT:

			str = strsep(&arg, " ");
			if (str == NULL)
				break;

			str = strsep(&arg, " ");
			if (str == NULL)
				break;
			wcid =	warp_str_tol(str, &end, 10);

			str = strsep(&arg, " ");
			if (str == NULL)
				break;
			amsdu_num =  warp_str_tol(str, &end, 10);

			str = strsep(&arg, " ");
			if (str == NULL)
				break;
			amsdu_len =  warp_str_tol(str, &end, 10);

			str = strsep(&arg, " ");
			if (str == NULL)
				break;
			vlan =  warp_str_tol(str, &end, 10);

			str = strsep(&arg, " ");
			if (str == NULL)
				break;
			hdrt =	warp_str_tol(str, &end, 10);

			warp_dbg(WARP_DBG_OFF, "WED_PROC_PAO_WCID_STAT!: wcid:%d\n", wcid);
			warp_pao_set_sta_info(wed, wcid, amsdu_num, amsdu_len, vlan, hdrt);
			break;
#endif	/* WED_PAO_SUPPORT */

#ifdef WED_MEM_LEAK_DBG
	case WED_PROC_MEM_INFO:
		warp_dump_mem();
		break;
#endif /*WED_MEM_LEAK_DBG*/
#ifdef WED_RX_HW_RRO_3_0
	case WED_PROC_RRO_SE_GET_PN:
		str = strsep(&arg, " ");
		if (str == NULL)
			break;

		str = strsep(&arg, " ");
		if (str == NULL)
			break;

		se_id =	warp_str_tol(str, &end, 10);
		warp_rx_pn_chk_get_hw(wed, se_id);
		break;
#endif
	default:
		break;
	}
}

/*
*
*/
void wed_procinfo_dump(struct wed_entry *wed, struct seq_file *seq, int idx)
{
	int ret = 0;
	struct wed_res_ctrl *res = &wed->res_ctrl;
	struct wed_rx_ctrl *rx_ctrl = &res->rx_ctrl;
	struct wed_rx_bm_res *rxbm_res = &rx_ctrl->res;
	struct warp_bm_rxdmad *rxd;
	u8 *addr;
	u32 size;
	int x;
	char linebuf[128] = "";
	u32 len = 0, buf_len = sizeof(linebuf);
	struct warp_dma_cb *cb;

	dump_string(seq, "===== WED RXBM info =====\n");
	dump_string(seq, "wed_rx_bm_res Va: %p\n", &rxbm_res);
	dump_string(seq, "ring_num: %d\n", rxbm_res->ring_num);
	dump_string(seq, "ring_len: %d\n", rxbm_res->ring_len);
	dump_string(seq, "pkt_num: %d\n", rxbm_res->pkt_num);
	dump_string(seq, "rxd_len: %d\n", rxbm_res->rxd_len);

	dump_string(seq, "Desc AllocPA\t: %pad\n", &rxbm_res->desc->alloc_pa);
	dump_string(seq, "Desc AllocVa\t: 0x%p\n", &rxbm_res->desc->alloc_va);
	dump_string(seq, "Desc Size\t: %lu\n", rxbm_res->desc->alloc_size);
	dump_string(seq, "Ring VA\t: AllocVa\t: 0x%p\n", &rxbm_res->ring->cell);

	if (idx >= 0 && idx < rxbm_res->ring_len) {
		cb = &rxbm_res->ring->cell[idx];
		dump_string(seq, "===== WED RXBM PKT [%d] info =====\n", idx);
		dump_string(seq, "[%d] CB AllocPA\t: %pad\n", idx, &cb->alloc_pa);
		dump_string(seq, "[%d] CB AllocVA\t: 0x%p\n", idx, &cb->alloc_va);
		dump_string(seq, "[%d] CB Size\t:  %lu\n", idx, cb->alloc_size);
		dump_string(seq, "[%d] PKT pkt_size\t:  %d\n", idx, cb->pkt_size);
		dump_string(seq, "[%d] PKT AllocPA\t: %pad\n", idx, &cb->pkt_pa);
		dump_string(seq, "[%d] PKT AllocVA\t: 0x%p\n", idx, &cb->pkt_va);
		rxd = (struct warp_bm_rxdmad *) &cb->alloc_va;
		dump_string(seq, "[%d] RXDMAD sdp0\t: 0x%08X\n", idx, rxd->sdp0);
		dump_string(seq, "[%d] RXDMAD token\t: 0x%08X\n", idx, rxd->token);

		addr = (u8 *) rxbm_res->ring->cell[idx].alloc_va;
		size = rxbm_res->ring->cell[idx].alloc_size;

		dump_string(seq, "===== WED RXBM PKT [%d] RAW: SIZE = %d =====\n", idx, size);
		for (x = 0; x < size; x++) {
			if (x % 32 == 0) {
				memset(linebuf, 0, 128);
				ret = snprintf(linebuf, buf_len, "\n0x%04x : ", x);
				if (ret < 0)
					return;
				len = strlen(linebuf);
			}
			ret = snprintf(linebuf + len, (buf_len - len), "%02x ",
					((unsigned char)addr[x]));
			if (ret < 0)
				return;
			len = strlen(linebuf);
			if ((x+1) % 32 == 0) { //print a line
				dump_string(seq, "%s\n", linebuf);
			}
		}
		if (size % 32 != 0) dump_string(seq, "%s\n", linebuf);
		dump_string(seq, "===== WED RXBM PKT [%d] RAW END =====\n", idx);
	} else
		dump_string(seq, "===== %d is invalid due to depth is %d =====\n", idx, rxbm_res->ring_len);

	if (rx_ctrl->budget_head_idx > 0) {
		u32 ext_idx = 0, pkt_idx = 0, dump_lmt = 0;
		struct wed_rx_bm_res *ext_rxbm_res = &rx_ctrl->extra;
		struct warp_rx_ring *ext_ring = &rx_ctrl->extra.ring[0];

		for (ext_idx = 0 ; ext_idx < rx_ctrl->budget_head_idx ; ext_idx++) {
			if (idx >= ext_rxbm_res->ring_len)
				dump_lmt = ext_rxbm_res->ring_len-1;
			else
				pkt_idx = idx;

			dump_string(seq, "===== WED RXBM EXT[%d] PKT [%d] info =====\n", ext_idx, idx);
			for ( ; pkt_idx <= dump_lmt ; pkt_idx++) {
				cb = &ext_ring[ext_idx].cell[pkt_idx];
				dump_string(seq, "[%d] CB AllocPA\t: %pad\n", pkt_idx, &cb->alloc_pa);
				dump_string(seq, "[%d] CB AllocVA\t: 0x%p\n", pkt_idx, &cb->alloc_va);
				dump_string(seq, "[%d] CB Size\t:  %lu\n", pkt_idx, cb->alloc_size);
				dump_string(seq, "[%d] PKT pkt_size\t:  %d\n", pkt_idx, cb->pkt_size);
				dump_string(seq, "[%d] PKT AllocPA\t: %pad\n", pkt_idx, &cb->pkt_pa);
				dump_string(seq, "[%d] PKT AllocVA\t: 0x%p\n", pkt_idx, &cb->pkt_va);
				rxd = (struct warp_bm_rxdmad *) &cb->alloc_va;
				dump_string(seq, "[%d] RXDMAD sdp0\t: 0x%08X\n", pkt_idx, rxd->sdp0);
				dump_string(seq, "[%d] RXDMAD token\t: 0x%08X\n", pkt_idx, rxd->token);

				addr = (u8 *) ext_rxbm_res->ring->cell[pkt_idx].alloc_va;
				size = ext_rxbm_res->ring->cell[pkt_idx].alloc_size;

				dump_string(seq, "===== WED RXBM EXT PKT [%d] RAW: SIZE = %d =====\n", pkt_idx, size);
				for (x = 0; x < size; x++) {
					if (x % 32 == 0) {
						memset(linebuf, 0, 128);
						ret = snprintf(linebuf, buf_len, "\n0x%04x : ", x);
						if (ret < 0)
							return;
						len = strlen(linebuf);
					}
					ret = snprintf(linebuf + len, (buf_len - len), "%02x ",
							((unsigned char)addr[x]));
					if (ret < 0)
						return;
					len = strlen(linebuf);
					if ((x+1) % 32 == 0) { //print a line
						dump_string(seq, "%s\n", linebuf);
					}
				}
				if (size % 32 != 0) dump_string(seq, "%s\n", linebuf);
			}
			dump_string(seq, "===== WED RXBM EXT PKT [%d] RAW END =====\n", ext_idx);
		}
	}
}

