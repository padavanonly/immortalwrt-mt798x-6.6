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
#include "warp_utility.h"
#include <linux/version.h>
#include <linux/pci.h>

static u32
wed_msdu_page_hash(dma_addr_t pa)
{
	u32 sum = 0;
	u16 i = 0;

	while (pa != 0) {
		sum += (u32) ((pa & 0xff) + i) % WED_MSDU_PAGE_HASH_SIZE;
		pa >>= 8;
		i += 13;
	}

	return sum % WED_MSDU_PAGE_HASH_SIZE;
}

static int
wed_rx_page_bm_dma_cb_init(struct platform_device *pdev,
			struct wed_rx_page_bm *bm,
			struct warp_dma_buf *desc,
			u32 idx,
			struct warp_dma_cb *dma_cb)
{
	struct warp_bm_rxdmad *rxd;
	struct pci_dev *wdev = NULL;
	void *pkt = NULL;
	u32 value = 0;
#ifdef WED_RX_HW_RRO_3_0_DBG
	void *pkt_pg_test = NULL;
#endif

	if (bm->hif_dev) {
		wdev = bm->hif_dev;

		dma_cb->alloc_size = bm->rxd_len;
		dma_cb->alloc_va = desc->alloc_va + (idx * dma_cb->alloc_size);
		dma_cb->alloc_pa = desc->alloc_pa + (idx * dma_cb->alloc_size);
		dma_cb->pkt_size = bm->rx_page_size;
		dma_cb->next = NULL;

		if (idx < bm->page_num) {
			pkt = page_frag_alloc(&bm->rx_page, dma_cb->pkt_size, GFP_ATOMIC);
#ifdef WED_MEM_LEAK_DBG
			pkt_alloc_mem((dma_cb->pkt_size), (void *)pkt, __builtin_return_address(0));
#endif	/* WED_MEM_LEAK_DBG */

			dma_cb->pkt = (struct sk_buff *)(pkt);
			dma_cb->pkt_va = (unsigned char *)pkt;
			dma_cb->pkt_pa = dma_map_single(&wdev->dev, dma_cb->pkt_va,
						dma_cb->pkt_size, DMA_FROM_DEVICE);
			rxd = (struct warp_bm_rxdmad *)dma_cb->alloc_va;
			/* Set SDP0_L to DW0 */
			WRITE_ONCE(rxd->sdp0, (dma_cb->pkt_pa & PARTIAL_RXDMAD_SDP0_L_MASK));

#ifdef CONFIG_WARP_64BIT_SUPPORT
			/* Set SDP0_H to DW1 */
			value = (dma_cb->pkt_pa >> WARP_DMA_ADDR_H_SHIFT) &
				PARTIAL_RXDMAD_SDP0_H_MASK;
#endif
			WRITE_ONCE(rxd->token, value);

#ifdef WED_RX_HW_RRO_3_0_DBG
			pkt_pg_test = kzalloc(sizeof(*rxd), GFP_ATOMIC);
			spin_lock(&bm->lock);
			list_add_tail(&dma_cb->list,
				&bm->wed_page_hash[wed_msdu_page_hash(dma_cb->pkt_pa)]);
			spin_unlock(&bm->lock);
#endif
		}

		return 0;
	} else {
		warp_dbg(WARP_DBG_ERR, "%s(): invalid hif_dev address!\n", __func__);
		return -1;
	}
}

void *
wed_page_hash_search(struct wed_entry *wed, dma_addr_t pa)
{

	struct wed_res_ctrl *res_ctrl = &wed->res_ctrl;
	struct wed_rx_ctrl *rx_ctrl = &res_ctrl->rx_ctrl;
	struct wed_rx_page_bm *bm = &rx_ctrl->page_bm;

	struct warp_dma_cb *pg_addr = NULL;
	struct list_head *p;
	u32 hash_idx =  wed_msdu_page_hash(pa);
	u8 found = 0;

	spin_lock(&bm->lock);
	list_for_each(p, &bm->wed_page_hash[hash_idx]) {
		pg_addr = list_entry(p, struct warp_dma_cb, list);
		if (pg_addr->pkt_pa == pa) {
			found = 1;
			break;
		}
	}
	spin_unlock(&bm->lock);

	return (found == 1) ? pg_addr->pkt_va : NULL;
}

static int
wed_init_msdu_page_hash(struct wed_rx_page_bm *bm)
{
#ifdef WED_RX_HW_RRO_3_0_DBG
	u16 i;

	for (i = 0; i < WED_MSDU_PAGE_HASH_SIZE; i++)
		INIT_LIST_HEAD(&bm->wed_page_hash[i]);

	return 0;
#else
	return 0;
#endif
}

static int
wed_exit_msdu_page_hash(struct wed_rx_page_bm *bm)
{
#ifdef WED_RX_HW_RRO_3_0_DBG

	u16 i;
	struct list_head *p, *q;
	struct warp_dma_cb *rxd = NULL;

	spin_lock(&bm->lock);
	for (i = 0; i < WED_MSDU_PAGE_HASH_SIZE; i++) {
		list_for_each_safe(p, q, &bm->wed_page_hash[i]) {
			rxd = list_entry(p, struct warp_dma_cb, list);
			list_del(&rxd->list);
		}
	}
	spin_unlock(&bm->lock);

	return 0;
#else
	return 0;
#endif

}


static int
wed_rx_page_bm_ring_init(
	struct wed_entry *wed,
	struct wed_rx_page_bm *bm)
{
	struct warp_rx_ring *ring = bm->ring;
	struct warp_dma_buf *desc = bm->desc;
	u32 i = 0, len = 0;

	len = bm->rxd_len * bm->ring_len;

	/* allocate dmad fifo ring */
	if (warp_dma_buf_alloc(wed->pdev, desc, len) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate desc fail, len=%d\n", __func__, len);
		return -1;
	}

	/* allocat dma cb for store hif_rxd page location */
	len = sizeof(struct warp_dma_cb) * bm->ring_len;

	ring->cell = vmalloc(len);
	if (!ring->cell) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate ring->cell faild\n", __func__);
		return -ENOMEM;
	}

	wed_init_msdu_page_hash(bm);

	for (i = 0; i < bm->ring_len; i++)
		wed_rx_page_bm_dma_cb_init(wed->pdev, bm, desc, i, &ring->cell[i]);

	return 0;
}

static inline void rx_page_bm_page_frag_cache_drain(struct page *page,
							unsigned int count)
{
	__page_frag_cache_drain(page, count);
}

static void
wed_rx_page_bm_dma_cb_exit(struct wed_entry *wed, struct wed_rx_page_bm *bm,
				u32 idx, struct warp_dma_cb *dma_cb)
{
	struct pci_dev *wdev = bm->hif_dev;

	if (idx < bm->page_num) {
#ifdef WED_MEM_LEAK_DBG
		pkt_free_mem((void *)dma_cb->pkt);
#endif	/* WED_MEM_LEAK_DBG */
		dma_unmap_single(&wdev->dev, dma_cb->pkt_pa,
				dma_cb->pkt_size, DMA_FROM_DEVICE);

		put_page(virt_to_head_page(dma_cb->pkt));
	}

	memset(dma_cb, 0, sizeof(struct warp_dma_cb));
}

static void
wed_rx_page_bm_ring_exit(
	struct wed_entry *wed,
	struct warp_rx_ring *ring,
	u32 ring_len,
	struct warp_dma_buf *desc)
{
	struct wed_res_ctrl *res_ctrl = &wed->res_ctrl;
	struct wed_rx_ctrl *rx_ctrl = &res_ctrl->rx_ctrl;
	struct wed_rx_page_bm *bm = &rx_ctrl->page_bm;
	u32 i;

	wed_exit_msdu_page_hash(bm);

	for (i = 0; i < ring_len; i++)
		wed_rx_page_bm_dma_cb_exit(wed, bm, i, &ring->cell[i]);


	if (bm->rx_page.va) {
		struct page *page;
		page = virt_to_page(bm->rx_page.va);
		rx_page_bm_page_frag_cache_drain(page, bm->rx_page.pagecnt_bias);
		memset(&bm->rx_page, 0, sizeof(bm->rx_page));
	}

	vfree(ring->cell);
	warp_dma_buf_free(wed->pdev, desc);
}

void
wed_rx_page_bm_exit(struct wed_entry *wed)
{
	struct wed_res_ctrl *res_ctrl = &wed->res_ctrl;
	struct wed_rx_ctrl *rx_ctrl = &res_ctrl->rx_ctrl;
	struct wed_rx_page_bm *bm = &rx_ctrl->page_bm;

	if (bm->ring) {
		wed_rx_page_bm_ring_exit(wed, bm->ring, bm->ring_len, bm->desc);

		warp_os_free_mem(bm->ring);
		bm->ring = NULL;
	}

	if (bm->desc) {
		warp_os_free_mem(bm->desc);
		bm->desc = NULL;
	}
}

int
wed_rx_page_bm_init(struct wed_entry *wed, struct wifi_hw *hw)
{
	struct wed_res_ctrl *res_ctrl = &wed->res_ctrl;
	struct wed_rx_ctrl *rx_ctrl = &res_ctrl->rx_ctrl;
	struct wed_rx_page_bm *bm = &rx_ctrl->page_bm;

	bm->ring_len = 65536;
	bm->rxd_len = sizeof(struct warp_bm_rxdmad);
	bm->page_num = 8192;
	bm->rx_page_size = 0x80; // 128 bytes
	bm->hif_dev = hw->hif_dev;

	warp_os_alloc_mem((unsigned char **)&bm->desc,
		sizeof(struct warp_dma_buf), GFP_ATOMIC);

	if (!bm->desc) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate rx page bm desc faild\n", __func__);
		goto err;
	}

	memset(bm->desc, 0, sizeof(struct warp_dma_buf));

	warp_os_alloc_mem((unsigned char **)&bm->ring,
		sizeof(struct warp_rx_ring), GFP_ATOMIC);

	if (!bm->ring) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate rx page bm ring faild\n", __func__);
		goto err;
	}

	memset(bm->ring, 0, sizeof(struct warp_rx_ring));

	if (wed_rx_page_bm_ring_init(wed, bm) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): init rx page bm ring faild\n", __func__);
		goto err;
	}

	return 0;

err:
	wed_rx_page_bm_exit(wed);
	return -1;
}
