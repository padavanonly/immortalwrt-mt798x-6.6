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
#include "warp_hw.h"
#include "wdma.h"
#include <linux/version.h>
#include <linux/pci.h>

void
dump_pkt_info(struct wed_buf_res *res)
{
}

static int
wed_tx_bm_dma_cb_init(struct platform_device *pdev,
			struct wed_tx_bm *bm,
			struct warp_dma_buf *desc,
			u32 idx,
			struct warp_dma_cb *dma_cb)
{
	struct warp_bm_txdmad *txd;
	struct pci_dev *wdev = NULL;
	void *pkt = NULL;
	int skb_id;
	u32 value;

	if (bm->hif_dev) {
		wdev = bm->hif_dev;

		dma_cb->alloc_size = bm->txd_len;
		dma_cb->alloc_va = desc->alloc_va + (idx * dma_cb->alloc_size);
		dma_cb->alloc_pa = desc->alloc_pa + (idx * dma_cb->alloc_size);
		dma_cb->pkt_size = bm->pkt_size;
		dma_cb->next = NULL;

		if (idx < bm->pkt_num) {
			pkt = page_frag_alloc(&bm->tx_page, dma_cb->pkt_size, GFP_ATOMIC);
			if (!pkt) {
				warp_dbg(WARP_DBG_ERR, "%s(): allocate pkt fail!\n", __func__);
				return -1;
			}
#ifdef WED_MEM_LEAK_DBG
			pkt_alloc_mem(dma_cb->pkt_size, (void *)pkt, __builtin_return_address(0));
#endif	/* WED_MEM_LEAK_DBG */
			skb_id = idr_alloc(&bm->id, pkt, 0, bm->pkt_num, GFP_ATOMIC);

			dma_cb->pkt = (struct sk_buff *)(pkt);
			dma_cb->pkt_va = (unsigned char *)pkt;
			dma_cb->pkt_pa = dma_map_single(&wdev->dev, dma_cb->pkt_va,
						dma_cb->pkt_size, DMA_TO_DEVICE);

			txd = (struct warp_bm_txdmad *)dma_cb->alloc_va;
			WRITE_ONCE(txd->sdp0, (dma_cb->pkt_pa & PARTIAL_TXDMAD_SDP0_L_MASK));

			value = (skb_id << PARTIAL_TXDMAD_TOKEN_ID_SHIFT) &
				PARTIAL_TXDMAD_TOKEN_ID_MASK;
#ifdef CONFIG_WARP_64BIT_SUPPORT
			/* Set SDP0_H to DW1 */
			value |= (dma_cb->pkt_pa >> WARP_DMA_ADDR_H_SHIFT) &
				 PARTIAL_TXDMAD_SDP0_H_MASK;
#endif
			WRITE_ONCE(txd->token, value);
		}

		return 0;
	} else {
		warp_dbg(WARP_DBG_ERR, "%s(): invalid hif_dev address!\n", __func__);
		return -1;
	}
}

static int
wed_tx_bm_ring_init(
	struct wed_entry *wed,
	struct wed_tx_bm *bm)
{
	struct warp_tx_ring *ring = bm->ring;
	struct warp_dma_buf *desc = bm->desc;
	u32 i = 0, len = 0;

	len = bm->txd_len * bm->ring_len;

	/* allocate dmad fifo ring */
	if (warp_dma_buf_alloc(wed->pdev, desc, len) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate desc fail, len=%d\n", __func__, len);
		return -1;
	}

	len = sizeof(struct warp_dma_cb) * bm->ring_len;

	ring->cell = vmalloc(len);
	if (!ring->cell) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate ring->cell faild\n", __func__);
		return -ENOMEM;
	}

	for (i = 0; i < bm->ring_len; i++) {
		if (wed_tx_bm_dma_cb_init(wed->pdev, bm, desc, i, &ring->cell[i]) < 0) {
			warp_dbg(WARP_DBG_ERR, "%s(): init tx bm dma cb failed\n", __func__);
			return -1;
		}
	}
	return 0;
}

static inline void tx_bm_page_frag_cache_drain(struct page *page,
						unsigned int count)
{
	__page_frag_cache_drain(page, count);
}

static void
wed_tx_bm_dma_cb_exit(struct wed_entry *wed, struct wed_tx_bm *bm, u32 idx,
			struct warp_dma_cb *dma_cb)
{
	struct pci_dev *wdev = bm->hif_dev;

	if (idx < bm->pkt_num) {
		void *pkt = idr_find(&bm->id, idx);
		if (pkt) {
#ifdef WED_MEM_LEAK_DBG
			pkt_free_mem(pkt);
#endif	/* WED_MEM_LEAK_DBG */
			idr_remove(&bm->id, idx);
			dma_unmap_single(&wdev->dev, dma_cb->pkt_pa,
					dma_cb->pkt_size, DMA_TO_DEVICE);
			put_page(virt_to_head_page(pkt));
		}
	}
	memset(dma_cb, 0, sizeof(struct warp_dma_cb));
}

static void
wed_tx_bm_ring_exit(
	struct wed_entry *wed,
	struct warp_tx_ring *ring,
	u32 ring_len,
	struct warp_dma_buf *desc)
{
	struct wed_res_ctrl *res_ctrl = &wed->res_ctrl;
	struct wed_tx_ctrl *tx_ctrl = &res_ctrl->tx_ctrl;
	struct wed_tx_bm *bm = &tx_ctrl->tx_bm;
	u32 i;

	if (ring->cell) {
		for (i = 0; i < ring_len; i++)
			wed_tx_bm_dma_cb_exit(wed, bm, i, &ring->cell[i]);
	}

	if (bm->tx_page.va) {
		struct page *page;
		page = virt_to_page(bm->tx_page.va);
		tx_bm_page_frag_cache_drain(page, bm->tx_page.pagecnt_bias);
		memset(&bm->tx_page, 0, sizeof(bm->tx_page));
	}

	if (ring->cell)
		vfree(ring->cell);
	warp_dma_buf_free(wed->pdev, desc);
}

void
wed_txbm_exit(struct wed_entry *wed)
{
	struct wed_res_ctrl *res_ctrl = &wed->res_ctrl;
	struct wed_tx_ctrl *tx_ctrl = &res_ctrl->tx_ctrl;
	struct wed_tx_bm *bm = &tx_ctrl->tx_bm;

	if (bm->ring) {
		wed_tx_bm_ring_exit(wed, bm->ring, bm->ring_len, bm->desc);

		warp_os_free_mem(bm->ring);
		bm->ring = NULL;
	}

	if (bm->desc) {
		warp_os_free_mem(bm->desc);
		bm->desc = NULL;
	}

	idr_destroy(&bm->id);
}

int
wed_txbm_init(struct wed_entry *wed, struct wifi_hw *hw)
{
	struct wed_res_ctrl *res_ctrl = &wed->res_ctrl;
	struct wed_tx_ctrl *tx_ctrl = &res_ctrl->tx_ctrl;
	struct wed_buf_res *res = &tx_ctrl->res;
	struct wed_tx_bm *bm = &tx_ctrl->tx_bm;

	res->tkn_grp_sz = 0x80;	/* 128 token per group */
	res->token_num = hw->tx_token_nums - hw->sw_tx_token_nums;
	res->tkn_rsv_grp = (res->token_num / res->tkn_grp_sz);
	res->tkn_vld_grp = (res->token_num / res->tkn_grp_sz);
	res->token_start = 0;
	res->token_end = (res->token_start + res->token_num - 1);

	bm->ring_len = 65536;
	bm->txd_len = sizeof(struct warp_bm_txdmad);
	bm->pkt_num = hw->tx_buf_nums;
	bm->pkt_size = hw->tx_pkt_size;
	bm->hif_dev = hw->hif_dev;

	idr_init(&bm->id);

	warp_os_alloc_mem((unsigned char **)&bm->desc,
		sizeof(struct warp_dma_buf), GFP_ATOMIC);

	if (!bm->desc) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate tx bm desc faild\n", __func__);
		goto err;
	}

	memset(bm->desc, 0, sizeof(struct warp_dma_buf));

	warp_os_alloc_mem((unsigned char **)&bm->ring,
		sizeof(struct warp_tx_ring), GFP_ATOMIC);

	if (!bm->ring) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate tx bm ring faild\n", __func__);
		goto err;
	}

	memset(bm->ring, 0, sizeof(struct warp_tx_ring));

	if (wed_tx_bm_ring_init(wed, bm) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): init tx bm ring faild\n", __func__);
		goto err;
	}

	return 0;

err:
	wed_txbm_exit(wed);
	return -1;
}
