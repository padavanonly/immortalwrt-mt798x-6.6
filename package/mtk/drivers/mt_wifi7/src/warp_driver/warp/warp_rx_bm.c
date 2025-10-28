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

static void
wed_rx_bm_dma_cb_exit(struct wed_entry *wed, struct warp_dma_cb *dma_cb)
{
	memset(dma_cb, 0, sizeof(struct warp_dma_cb));
}

#if (KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE)
static inline void __rx_bm_page_frag_drain(struct page *page,
								unsigned int order,
								unsigned int count)
{
	atomic_sub(count - 1, &page->_count);
	__free_pages(page, order);

}
#endif

static inline void rx_bm_page_frag_cache_drain(struct page *page,
								unsigned int count)
{
#if (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
		__page_frag_cache_drain(page, count);
#else
		__rx_bm_page_frag_drain(page, compound_order(page), count);
#endif
}

static void
wed_rx_bm_ring_exit(
	struct wed_entry *wed,
	struct warp_rx_ring *ring,
	u32 ring_len,
	struct warp_dma_buf *desc)
{
	struct wed_res_ctrl *res_ctrl = &wed->res_ctrl;
	struct wed_rx_ctrl *rx_ctrl = &res_ctrl->rx_ctrl;
	struct wed_rx_bm_res *res = &rx_ctrl->res;
	u32 i;

	if (!ring->cell) {
		warp_dbg(WARP_DBG_ERR, "%s(): ring->cell is null return\n", __func__);
		return;
	}

	for (i = 0; i < ring_len; i++) {
#ifdef WED_MEM_LEAK_DBG
		if (i < res->pkt_num)
			pkt_free_mem(ring->cell[i].pkt);
#endif /* WED_MEM_LEAK_DBG */
		wed_rx_bm_dma_cb_exit(wed, &ring->cell[i]);
	}

	for (i = 0; i < 2; i++) {
		if (res->rx_page[i].va) {
			struct page *page;
			page = virt_to_page(res->rx_page[i].va);
			rx_bm_page_frag_cache_drain(page, res->rx_page[i].pagecnt_bias);
			memset(&res->rx_page[i], 0, sizeof(res->rx_page[i]));
		}
	}

	vfree(ring->cell);
	warp_dma_buf_free(wed->pdev, desc);
}

static int
wed_rx_bm_dma_cb_init(struct platform_device *pdev,
		      struct wed_rx_bm_res *res,
		      struct warp_dma_buf *desc,
		      u32 idx,
		      struct warp_dma_cb *dma_cb)
{
	int ret = -1;
	struct warp_bm_rxdmad *rxd;
	struct pci_dev *wdev = NULL;
	void *pkt = NULL;
	u32 value = 0;
	u8 i;

	if (res->hif_dev) {
		wdev = res->hif_dev;

		/* Init RX Ring Size, Va, Pa variables */
		dma_cb->alloc_size = res->rxd_len;
		dma_cb->alloc_va = desc->alloc_va + (idx * dma_cb->alloc_size);
		dma_cb->alloc_pa = desc->alloc_pa + (idx * dma_cb->alloc_size);
		dma_cb->pkt_size = res->pkt_len;
		dma_cb->next = NULL;

		if (idx < res->pkt_num) {
			/* allocate Wi-Fi RXD + RXP buffer */

			if (idx < res->pkt_num / 2)
				i = 0;
			else
				i = 1;

#if (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
			pkt = page_frag_alloc(&res->rx_page[i],
						SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + dma_cb->pkt_size)
						+ SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV), GFP_ATOMIC);
#else
			pkt = __alloc_page_frag(&res->rx_page[i],
						SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + dma_cb->pkt_size)
						+ SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV), GFP_ATOMIC);
#endif
			dma_cb->pkt = (struct sk_buff *)(pkt);
#ifdef WED_MEM_LEAK_DBG
			pkt_alloc_mem((SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + dma_cb->pkt_size)
						+ SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV)),
						(void *)pkt, __builtin_return_address(0));
#endif	/* WED_MEM_LEAK_DBG */

			dma_cb->pkt_va = ((unsigned char *)pkt + SKB_BUF_HEADROOM_RSV);

			dma_cb->pkt_pa = dma_map_single(&wdev->dev, dma_cb->pkt_va, dma_cb->pkt_size,
							DMA_FROM_DEVICE);

			/* advance to next ring descriptor address */
			rxd = (struct warp_bm_rxdmad *)dma_cb->alloc_va;
			/* Set SDP0_L to DW0 */
			WRITE_ONCE(rxd->sdp0, (dma_cb->pkt_pa & PARTIAL_RXDMAD_SDP0_L_MASK));

#ifdef CONFIG_WARP_64BIT_SUPPORT
			/* Set SDP0_H to DW1 */
			value = (dma_cb->pkt_pa >> WARP_DMA_ADDR_H_SHIFT) &
				PARTIAL_RXDMAD_SDP0_H_MASK;
#endif
			WRITE_ONCE(rxd->token, value);

			warp_dbg(WARP_DBG_INF, "%s(): rxd->sdp0: 0x%x!\n", __func__, rxd->sdp0);
			warp_dbg(WARP_DBG_INF, "%s(): rxd->token: 0x%x!\n", __func__, rxd->token);
		}

		ret = 0;
	} else
		warp_dbg(WARP_DBG_ERR, "%s(): invalid hif_dev address!\n", __func__);

	return ret;
}

static int
wed_rx_bm_ring_init(
	struct wed_entry *wed,
	u8 idx,
	struct wed_rx_bm_res *res)
{
	int ret = -1;
	u32 i = 0, len = 0;
	struct warp_rx_ring *ring = &res->ring[idx];
	struct warp_dma_buf *desc = &res->desc[idx];

	len = res->rxd_len * res->ring_len;

	if (warp_dma_buf_alloc(wed->pdev, desc, len) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate desc fail, len=%d\n", __func__, len);
		return ret;
	}

	len =  sizeof(struct warp_dma_cb) * res->ring_len;
	ring->cell = (struct warp_dma_cb *)vmalloc(len);

	if (!ring->cell) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate ring->cell faild\n", __func__);
		return ret;
	}

	for (i = 0; i < res->ring_len; i++)
		wed_rx_bm_dma_cb_init(wed->pdev, res, desc, i, &ring->cell[i]);

	ring->head = NULL;

	ret = 0;

	return ret;
}

int
wed_rx_bm_token_deinit(
	struct wed_entry *wed,
	struct warp_rx_ring *ring,
	u32 number)
{
	int ret = 0;
	u32 pkt_idx = 0;
	struct warp_dma_cb *cb = NULL;

	cb = &ring->cell[0];

	for (pkt_idx = 0 ; pkt_idx < number ; pkt_idx++) {
		u32 tkn_rx_id = 0;
		struct warp_bm_rxdmad *dmad = NULL;

		dmad = (struct warp_bm_rxdmad *)cb[pkt_idx].alloc_va;
		tkn_rx_id = (dmad->token >> TOKEN_ID_SHIFT);
		/* register packet information once switch DMAD bwtween host/warp */
		if (wed_release_rx_token(wed, tkn_rx_id) == 0) {
#ifdef WARP_DVT
			if (pkt_idx == 0)
				warp_dbg(WARP_DBG_OFF, "%s(): deinit bm rx ring start w/ token:%d\n",
							__func__, tkn_rx_id);

			if (pkt_idx == (number-1))
				warp_dbg(WARP_DBG_OFF, "%s(): deinit bm rx ring end w/ token:%d\n",
							__func__, tkn_rx_id);
#endif
		} else
			warp_dbg(WARP_DBG_LOU, "%s(): deinit bm rx ring pkt[%d] failed!\n",
							__func__, pkt_idx);
	}

	return ret;
}

int
wed_rx_bm_token_init(
	struct wed_entry *wed,
	u8 ring_idx,
	struct wed_rx_bm_res *res,
	u32 number)
{
	int ret = 0;
	u32 pkt_idx = 0;
	u32 rxdmad_dw2 = 0;
	struct warp_dma_cb *cb = NULL;

	cb = &res->ring[ring_idx].cell[0];

	for (pkt_idx = 0 ; pkt_idx < number ; pkt_idx++) {
		u32 tkn_rx_id = 0;
		struct warp_bm_rxdmad *dmad = NULL;

		dmad = (struct warp_bm_rxdmad *)cb[pkt_idx].alloc_va;
		/* register packet information once switch DMAD bwtween host/warp */
		if (wed_acquire_rx_token(wed, &cb[pkt_idx], &tkn_rx_id) == 0) {
			rxdmad_dw2 = READ_ONCE(dmad->token) | (tkn_rx_id << TOKEN_ID_SHIFT);
			WRITE_ONCE(dmad->token, rxdmad_dw2);
#ifdef WARP_DVT
			if (pkt_idx == 0)
				warp_dbg(WARP_DBG_OFF, "%s(): init bm rx ring[%d][%d] start w/ token:%d\n",
							__func__, ring_idx, pkt_idx, tkn_rx_id);

			if (pkt_idx == (number-1))
				warp_dbg(WARP_DBG_OFF, "%s(): init bm rx ring[%d][%d] end w/ token:%d\n",
							__func__, ring_idx, pkt_idx, tkn_rx_id);
#endif
		} else
			warp_dbg(WARP_DBG_LOU, "%s(): init bm rx ring[%d] pkt[%d] w/o token!\n",
							__func__, ring_idx, pkt_idx);
	}

	return ret;
}

#ifdef WED_DYNAMIC_RXBM_SUPPORT
static void
wed_rx_budget_ring_exit(
	struct wed_entry *wed,
	struct warp_rx_ring *ring,
	u32 ring_len,
	struct warp_dma_buf *desc)
{
	struct wed_res_ctrl *res_ctrl = &wed->res_ctrl;
	struct wed_rx_ctrl *rx_ctrl = &res_ctrl->rx_ctrl;
	struct wed_rx_bm_res *extra_res = &rx_ctrl->extra;
	u32 i;

	for (i = 0; i < ring_len; i++)
		wed_rx_bm_dma_cb_exit(wed, &ring->cell[i]);

	dma_unmap_single(&wed->pdev->dev, desc->alloc_pa, desc->alloc_size, PCI_DMA_FROMDEVICE);
	warp_os_free_mem(desc->alloc_va);
	memset(desc, 0, sizeof(*desc));

	warp_os_free_mem(ring->cell);
	memset(ring, 0, sizeof(*ring));
}

static int
wed_rx_budget_ring_init(
	struct wed_entry *wed,
	u8 idx,
	struct wed_rx_bm_res *res)
{
	int ret = -1;
	u32 i = 0, len = 0;
	struct warp_rx_ring *ring = &res->ring[idx];
	struct warp_dma_buf *desc = &res->desc[idx];

	desc->alloc_size = res->rxd_len * res->ring_len;

	if (warp_os_alloc_mem((u8 **)&res->desc[idx].alloc_va, desc->alloc_size, GFP_ATOMIC) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate desc fail, len=%ld\n", __func__, desc->alloc_size);
		ret = -1;
		goto err;
	}
	memset((u8 *)desc->alloc_va, 0, desc->alloc_size);

	len =  sizeof(struct warp_dma_cb) * res->ring_len;
	warp_os_alloc_mem((unsigned char **)&ring->cell, len, GFP_ATOMIC);

	if (!ring->cell) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate ring->cell faild\n", __func__);
		return -1;
	}

	for (i = 0; i < res->ring_len; i++)
		wed_rx_bm_dma_cb_init(wed->pdev, res, desc, i, &ring->cell[i]);

	ring->head = NULL;

	wed_rx_bm_token_init(wed, idx, res, res->ring_len);

	desc->alloc_pa = dma_map_single(&wed->pdev->dev,
					desc->alloc_va,
					desc->alloc_size, DMA_TO_DEVICE);
	if (dma_mapping_error(&wed->pdev->dev, desc->alloc_pa)) {
		warp_dbg(WARP_DBG_ERR, "%s(): FATAL, DMA map ERROR, 0x%p\n", __func__, desc->alloc_va);
		warp_os_free_mem(res->desc[idx].alloc_va);
		ret = -1;
		goto err;
	}

	ret = 0;

err:
	return ret;
}

#ifdef EXTEND_POLLING
static void
rxbm_free_cache_handler(struct timer_list *monitor)
{
	struct wed_rx_bm_res *res = container_of(monitor, struct wed_rx_bm_res, extend_monitor);
	struct wed_rx_ctrl *rx_ctrl = container_of(res, struct wed_rx_ctrl, res);
	struct wed_res_ctrl *res_ctrl = container_of(rx_ctrl, struct wed_res_ctrl, rx_ctrl);
	struct wed_entry *wed = container_of(res_ctrl, struct wed_entry, res_ctrl);

	if (!warp_rx_dybm_addsub_acked(wed)) {
		rx_ctrl->extend_polling_cnt++;
		mod_timer(&res->extend_monitor, jiffies+1);
	} else {
		spin_lock_bh(&rx_ctrl->recy_lock);
		warp_rx_dybm_mod_thrd(wed, THRD_INC_L, wed->sw_conf->rxbm.alt_quota*128);
		rx_ctrl->recycle_wait = wed->sw_conf->rxbm.recycle_postponed;
		res->add_check = false;
		spin_unlock_bh(&rx_ctrl->recy_lock);
#ifdef WARP_DVT
		warp_dbg(WARP_DBG_OFF, "%s(): recycle wait=%d! (polling:%d)\n",
				 __func__, rx_ctrl->recycle_wait, rx_ctrl->extend_polling_cnt);
#endif	/* WARP_DVT */
		rx_ctrl->extend_polling_cnt = 0;
		dybm_eint_ctrl(wed, true, WARP_DYBM_EINT_RXBM_HL);
	}

	return;
}
#endif	/* EXTEND_POLLING */

/*
 *
 */
static void
rbudge_refill_handler(unsigned long data)
{
	u32 i = 0, quota = 0, refill_cnt = 0;
	struct wed_entry *wed = (struct wed_entry *)data;
	struct dybm_conf_t *sw_conf = &wed->sw_conf->rxbm;
	struct wed_rx_bm_res *res = &wed->res_ctrl.rx_ctrl.res;
	struct wed_rx_bm_res *extra_res = &wed->res_ctrl.rx_ctrl.extra;
#ifdef WARP_DVT
	unsigned long now = sched_clock(), after = 0;
#endif	/* WARP_DVT */

	quota = sw_conf->budget_limit - res->budget_grp;
	quota /= sw_conf->alt_quota;	/* each budget entry include wed->sw_conf->rxbm.alt_quota groups */

	for (i = 0 ; i < quota && (wed->res_ctrl.rx_ctrl.budget_tail_idx < extra_res->ring_num); i++) {
		if (wed_rx_budget_ring_init(wed, wed->res_ctrl.rx_ctrl.budget_tail_idx, extra_res) < 0) {
			warp_dbg(WARP_DBG_ERR, "%s(): init rx extra bm rx ring faild\n", __func__);
			goto err;
		}
#ifdef WARP_DVT
		warp_dbg(WARP_DBG_OFF, "%s(): quota:%d refill %d budget w/ extra ring[%d]! (budget_grp:%d)\n",
				 __func__, quota, i, wed->res_ctrl.rx_ctrl.budget_tail_idx, (res->budget_grp + (refill_cnt+1)*sw_conf->alt_quota));
#endif	/* WARP_DVT */
		wed->res_ctrl.rx_ctrl.budget_tail_idx++;
		refill_cnt++;
	}

err:
#ifdef WARP_DVT
	after = sched_clock();
#endif /* WARP_DVT */
	if (i) {
		res->dybm_stat.budget_refill += refill_cnt*sw_conf->alt_quota;
		res->budget_grp += refill_cnt*sw_conf->alt_quota;
#ifdef WARP_DVT
		warp_dbg(WARP_DBG_OFF, "%s(): Enlarge %d group(s)!\n", __func__,
					refill_cnt*sw_conf->alt_quota);
		warp_dbg(WARP_DBG_OFF, "%s(): process time: %ld us(%ld-%ld)\n",
				 __func__, (after-now)/1000, after, now);
#endif	/* WARP_DVT */
	} else
		warp_dbg(WARP_DBG_ERR, "%s(): extra rings been run out(%d:%d)!\n", __func__,
					wed->res_ctrl.rx_ctrl.budget_tail_idx, extra_res->ring_num);
}

void
rxbm_free_handler(unsigned long data)
{
	struct wed_entry *wed = (struct wed_entry *)data;
	struct wed_rx_bm_res *res = NULL;
	struct wed_rx_bm_res *extra_res = NULL;
	struct wed_rx_ctrl *rx_ctrl = NULL;
	u32 free_cnt = 0, i = 0;

	if (wed) {
		u32 budget_head = 0;
		struct dybm_conf_t *sw_conf = &wed->sw_conf->rxbm;

		rx_ctrl = &wed->res_ctrl.rx_ctrl;
		extra_res = &rx_ctrl->extra;
		res = &rx_ctrl->res;

		budget_head = rx_ctrl->budget_head_idx;

		free_cnt = (rx_ctrl->budget_tail_idx - budget_head);
		free_cnt *= sw_conf->alt_quota;
		free_cnt -= sw_conf->budget_limit;
		free_cnt /= sw_conf->alt_quota;

		for (i = 0 ; i < free_cnt ; i++) {
#ifdef WARP_DVT
			warp_dbg(WARP_DBG_OFF,
					 "%s(): free_cnt:%d Budget ring[%d] is capcable to be freed! due to ((%d-%d)*%d > %d) (budget_grp:%d)\n",
					 __func__, free_cnt, rx_ctrl->budget_tail_idx-1, rx_ctrl->budget_tail_idx,
					 budget_head, sw_conf->alt_quota,
					 sw_conf->budget_limit, res->budget_grp - sw_conf->alt_quota);
#endif	/* WARP_DVT */
			if (extra_res->ring[rx_ctrl->budget_tail_idx-1].cell) {
				wed_rx_bm_token_deinit(wed, &extra_res->ring[rx_ctrl->budget_tail_idx-1], extra_res->ring_len);

				wed_rx_budget_ring_exit(wed, &extra_res->ring[rx_ctrl->budget_tail_idx-1],
										extra_res->ring_len,
										&extra_res->desc[rx_ctrl->budget_tail_idx-1]);
				rx_ctrl->budget_tail_idx--;
				res->budget_grp -= sw_conf->alt_quota;

				res->dybm_stat.budget_release += sw_conf->alt_quota;
			}
#ifdef WARP_DVT
			else
				warp_dbg(WARP_DBG_OFF,
						 "%s(): free_cnt:%d Budget ring[%d] is freed! dismissed! \n",
						 __func__, free_cnt, rx_ctrl->budget_tail_idx-1);
#endif	/* WARP_DVT */
		}
	}
}


void
rxbm_recycle_handler(unsigned long data)
{
	u32 value = 0, fifo_left = 0;
	struct wed_entry *wed = (struct wed_entry *)data;
	struct dybm_conf_t *sw_conf = &wed->sw_conf->rxbm;
	struct wed_rx_ctrl *rx_ctrl = &wed->res_ctrl.rx_ctrl;
	struct wed_rx_bm_res *res = &wed->res_ctrl.rx_ctrl.res;
	struct wed_rx_bm_res *extra_res = &wed->res_ctrl.rx_ctrl.extra;
	struct dybm_ul_tasks *tasks = NULL;
#ifdef WARP_DVT
	unsigned long now = sched_clock(), after = 0;
#endif	/* WARP_DVT */
#ifdef EXTEND_POLLING
	bool locked = false;
#endif	/* EXTEND_POLLING */

#if !defined(EXTEND_POLLING)
	if (!warp_rx_dybm_addsub_acked(wed)) {
#ifdef WARP_DVT
		warp_dbg(WARP_DBG_OFF, "%s(): Extend cache not drained! dismissed!\n", __func__);
#endif	/* WARP_DVT */
		goto err_out;
	}
#else
	locked = spin_trylock(&rx_ctrl->recy_lock);

	if (locked)
#endif	/* !EXTEND_POLLING */
	{
#if !defined(EXTEND_POLLING)
		if (res->add_check == true) {
			warp_dbg(WARP_DBG_OFF, "%s(): Extend operation is not finised but not locked!\n", __func__);

			goto err_out;
		}
#endif	/* EXTEND_POLLING */
		if (wed->dybm_ul_tasks) {
			tasks = (struct dybm_ul_tasks *)wed->dybm_ul_tasks;
		} else {
			warp_dbg(WARP_DBG_ERR, "%s(): empty tasks! dismissed!\n", __func__);
			goto err_out;
		}

		warp_io_read32(wed, WED_RX_BM_TOTAL_DMAD_IDX, &value);
		if (value <= sw_conf->vld_group) {
			warp_dbg(WARP_DBG_ERR, "%s(): Ignore event due to buffer number(%d) less then %d!\n",
					 __func__, value, sw_conf->vld_group);
			goto err_out;
		}

		fifo_left = warp_rxbm_left_query(wed);
#ifdef WARP_DVT
		warp_dbg(WARP_DBG_OFF, "%s(): shrink INT triggered, total:%d! used:%04x left:%d\n",
				 __func__, value, value - fifo_left, fifo_left);
#endif	/* WARP_DVT */

		if (rx_ctrl->recycle_wait > 0) {
			rx_ctrl->recycle_wait--;
#ifdef WARP_DVT
			warp_dbg(WARP_DBG_OFF, "%s(): tend to recycle!\n", __func__);
#endif	/* WARP_DVT */
		}

		if (rx_ctrl->recycle_wait == 0) {
			u8 recycle_cnt = sw_conf->recycle_postponed, i = 0;

			rx_ctrl->recycle_wait = sw_conf->recycle_postponed;

			if (recycle_cnt <= rx_ctrl->budget_head_idx &&
				fifo_left >= extra_res->ring_len*recycle_cnt) {
				u32 stop_idx = 0;

				/* control head_idx prior to access DDR for latency prevention */
				warp_rx_dybm_fifo_jump(wed, recycle_cnt, &stop_idx);
				for (i = 0 ; i < recycle_cnt ; i++)
					warp_rx_dybm_mod_thrd(wed, THRD_DEC_ALL, sw_conf->alt_quota*128);

				for (i = 0 ; i < recycle_cnt ; i++) {
					res->dybm_stat.shk_times++;
					/* recycle DMADs content to budget head */
#ifdef WARP_DVT
					warp_dbg(WARP_DBG_OFF, "%s(): Recyle %d/%d DMADs to 0x%lu(ring:%d)! (budget_grp:%d)\n",
							 __func__, fifo_left, extra_res->ring_len,
							 extra_res->desc[rx_ctrl->budget_head_idx-1].alloc_va,
							 rx_ctrl->budget_head_idx-1, (res->budget_grp + sw_conf->alt_quota));
#endif	/* WARP_DVT */
#ifdef WARP_DVT
					now = sched_clock();
#endif	/* WARP_DVT */
					warp_rx_dybm_r_fifo(wed, &extra_res->desc[rx_ctrl->budget_head_idx-1], stop_idx);
#ifdef WARP_DVT
					after = sched_clock();
					warp_dbg(WARP_DBG_OFF, "%s(): Recyle %d packets to extra ring[%d]!\n", __func__,
								rx_ctrl->extra.ring_len, rx_ctrl->budget_head_idx-1);
					warp_dbg(WARP_DBG_OFF, "%s(): process time: %ld us(%ld-%ld)\n",
							 __func__, (after-now)/1000, after, now);
#endif	/* WARP_DVT */
					extra_res->ring[rx_ctrl->budget_head_idx-1].recycled = true;
					rx_ctrl->budget_head_idx--;
					res->budget_grp += sw_conf->alt_quota;

					stop_idx += extra_res->ring_len;
					stop_idx %= res->ring_len;
					fifo_left -= extra_res->ring_len;
				}
			}
#ifdef WARP_DVT
			else {
				if (recycle_cnt > rx_ctrl->budget_head_idx)
					warp_dbg(WARP_DBG_OFF, "%s(): %d is more than budget ring index(%d) , dismissed!\n", __func__,
						 sw_conf->recycle_postponed, rx_ctrl->budget_head_idx);
				else
					warp_dbg(WARP_DBG_OFF, "%s(): FIFO left(%d) less than quota*%d(%d), dismissed!\n", __func__,
						 fifo_left, sw_conf->recycle_postponed, extra_res->ring_len*sw_conf->recycle_postponed);
			}
#endif	/* WARP_DVT */

			if ((rx_ctrl->budget_tail_idx - rx_ctrl->budget_head_idx)*sw_conf->alt_quota
				 > sw_conf->budget_limit)
				 tasklet_schedule(&tasks->rbudge_release_task);
		}
#ifdef WARP_DVT
		else
			warp_dbg(WARP_DBG_OFF, "%s(): postponed!\n", __func__);
#endif	/* WARP_DVT */
	}
#ifdef EXTEND_POLLING
	else {
		if (res->add_check == true)
			warp_dbg(WARP_DBG_INF, "%s(): Locked by extend operation! dismissed!\n", __func__);
		else
			warp_dbg(WARP_DBG_INF, "%s(): Suspicious locked! dismissed!\n", __func__);
	}
#endif	/* EXTEND_POLLING */

err_out:
	dybm_eint_ctrl(wed, true, WARP_DYBM_EINT_RXBM_HL);

#ifdef EXTEND_POLLING
	if (locked)
		spin_unlock(&rx_ctrl->recy_lock);
#endif	/* EXTEND_POLLING */

	return;
}

/*
*
*/
void
rxbm_alloc_handler(unsigned long data)
{
	u32 dma_pa = 0;
	struct wed_entry *wed = (struct wed_entry *)data;
	struct dybm_conf_t *sw_conf = &wed->sw_conf->rxbm;
	struct wed_rx_ctrl *rx_ctrl = &wed->res_ctrl.rx_ctrl;
	struct wed_rx_bm_res *res = &wed->res_ctrl.rx_ctrl.res;
	struct wed_rx_bm_res *extra_res = &wed->res_ctrl.rx_ctrl.extra;
#ifdef WARP_DVT
	unsigned long now = sched_clock(), after = 0;
#endif	/* WARP_DVT */
	struct dybm_ul_tasks *tasks = NULL;

#if !defined(EXTEND_POLLING)
	if (!warp_rx_dybm_addsub_acked(wed) && is_warp_rx_dybm_add_op(wed)) {
#ifdef WARP_DVT
		warp_dbg(WARP_DBG_ERR, "%s(): Extend cache not drained! dismissed!\n", __func__);
#endif	/* WARP_DVT */
		dybm_eint_ctrl(wed, true, WARP_DYBM_EINT_RXBM_HL);
		goto err_out;
	}
#endif	/* !EXTEND_POLLING */

	if (wed->dybm_ul_tasks) {
		tasks = (struct dybm_ul_tasks *)wed->dybm_ul_tasks;
	} else {
		warp_dbg(WARP_DBG_ERR, "%s(): empty tasks! dismissed!\n", __func__);
		goto err_out;
	}

#ifdef WARP_DVT
	warp_dbg(WARP_DBG_OFF, "%s(): extend INT triggered, FIFO left:%d!\n",
			 __func__, warp_rxbm_left_query(wed));
#endif

	if (res->budget_grp) {
		if (extra_res->ring[rx_ctrl->budget_head_idx].recycled) {
#ifdef WARP_DVT
			warp_dbg(WARP_DBG_OFF, "%s(): Extend BM by ring(%d), recycled! (budget_grp:%d)\n",
					 __func__, rx_ctrl->budget_head_idx, (res->budget_grp - wed->sw_conf->rxbm.alt_quota));
#endif
			extra_res->ring[rx_ctrl->budget_head_idx].recycled = false;
		} else {
#ifdef WARP_DVT
			warp_dbg(WARP_DBG_OFF, "%s(): Extend BM by ring(%d) (budget_grp:%d)!\n",
					 __func__, rx_ctrl->budget_head_idx, (res->budget_grp - wed->sw_conf->rxbm.alt_quota));
#endif
		}
		res->budget_grp -= wed->sw_conf->rxbm.alt_quota;
		dma_pa = extra_res->desc[rx_ctrl->budget_head_idx].alloc_pa,

		rx_ctrl->budget_head_idx++;

		res->dybm_stat.ext_times++;
	} else {
		warp_dbg(WARP_DBG_ERR,
				 "%s(): Extend BM operation dimissed! Due to budget is exhausted!\n",
				 __func__);
		res->dybm_stat.ext_failed++;
	}

	/* req h/w */
	if (dma_pa) {
#ifdef EXTEND_POLLING
		u8 polling = DYBM_RX_EXT_POLLING_CNT;	/* polling 50 times proximately wait 5us */
		u32 polling_start = 0, polling_end = 0;
#endif	/* EXTEND_POLLING */
		warp_rx_dybm_mod_thrd(wed, THRD_INC_H, sw_conf->alt_quota*128);
		warp_rx_dybm_w_cache(wed, dma_pa);

#ifdef EXTEND_POLLING
		polling_start = sched_clock();
		while (!warp_rx_dybm_addsub_acked(wed) && polling)
			polling--;
		polling_end = sched_clock();
		if (polling == 0) {
			res->add_check = true;
			rx_ctrl->extend_polling_cnt++;
			mod_timer(&res->extend_monitor, jiffies+1);

#ifdef WARP_DVT
			warp_dbg(WARP_DBG_OFF, "%s(): Polling %d times but not cousumed!(%dus, %ld-%ld)\n",
					 __func__, DYBM_RX_EXT_POLLING_CNT, (polling_end-polling_start)/1000, polling_end, polling_start);
#endif	/* WARP_DVT */
		} else
#endif	/* EXTEND_POLLING */
		{
			rx_ctrl->recycle_wait = sw_conf->recycle_postponed;
#ifdef WARP_DVT
			warp_dbg(WARP_DBG_OFF, "%s(): recycle wait=%d!\n", __func__, rx_ctrl->recycle_wait);
#endif	/* WARP_DVT */
			if ((res->dybm_stat.ext_times % sw_conf->recycle_postponed) == 0)
				warp_rx_dybm_mod_thrd(wed, THRD_INC_L, sw_conf->recycle_postponed*sw_conf->alt_quota*128);
			dybm_eint_ctrl(wed, true, WARP_DYBM_EINT_RXBM_HL);
#if defined(EXTEND_POLLING) && defined(WARP_DVT)
			warp_dbg(WARP_DBG_OFF, "%s(): Polling %d times and cousumed!(%dus, %ld-%ld)\n",
					 __func__, polling, (polling_end-polling_start)/1000, polling_end, polling_start);
#endif	/* WARP_DVT */
		}

		if (((res->pkt_num/128) + rx_ctrl->budget_head_idx*sw_conf->alt_quota > res->dybm_stat.max_vld_grp))
			res->dybm_stat.max_vld_grp = (res->pkt_num/128) + rx_ctrl->budget_head_idx*sw_conf->alt_quota;
	}

#ifdef WARP_DVT
	after = sched_clock();

	warp_dbg(WARP_DBG_OFF, "%s(): process time:%ld us(%ld-%ld)\n",
				 __func__, (after-now)/1000, after, now);
#endif	/* WARP_DVT */

	if ((sw_conf->budget_limit > res->budget_grp) &&
		(res->budget_grp <= sw_conf->budget_refill_watermark))
			tasklet_schedule(&tasks->rbudge_refill_task);

err_out:
	return;
}

int unregist_ul_dybm_task(struct wed_entry *wed)
{
	if (wed->dybm_ul_tasks) {
		warp_os_free_mem(wed->dybm_ul_tasks);
		wed->dybm_ul_tasks = NULL;
	}

	return 0;
}

int regist_ul_dybm_task(struct wed_entry *wed)
{
	int ret = -1;
	struct dybm_ul_tasks *tasks = NULL;

	if(wed == NULL) {
		warp_dbg(WARP_DBG_ERR, "%s(): invalid wed address!\n", __func__);
		goto err;
	}

	if (warp_os_alloc_mem((u8 **)&wed->dybm_ul_tasks, sizeof(struct dybm_ul_tasks), GFP_ATOMIC) == 0) {
		tasks = (struct dybm_ul_tasks *)wed->dybm_ul_tasks;

		tasklet_init(&tasks->rbudge_refill_task, rbudge_refill_handler, (unsigned long)wed);
		tasklet_init(&tasks->rbuf_alloc_task, rxbm_alloc_handler, (unsigned long)wed);
		tasklet_init(&tasks->rbuf_free_task, rxbm_recycle_handler, (unsigned long)wed);
		tasklet_init(&tasks->rbudge_release_task, rxbm_free_handler, (unsigned long)wed);

		ret = 0;
	} else
		warp_dbg(WARP_DBG_ERR, "%s(): memory to store tasks allocated failed!\n", __func__);

err:
	return ret;
}

int dybm_ul_int_disp(struct wed_entry *wed, u32 status)
{
	struct dybm_ul_tasks *tasks = NULL;

	if (wed) {
		if (wed->dybm_ul_tasks) {
			tasks = (struct dybm_ul_tasks *)wed->dybm_ul_tasks;

			switch(status) {
				case WARP_DYBM_EINT_RXBM_H:
					tasklet_schedule(&tasks->rbuf_free_task);
					break;
				case WARP_DYBM_EINT_RXBM_L:
					tasklet_schedule(&tasks->rbuf_alloc_task);
					break;
				default:
					warp_dbg(WARP_DBG_ERR, "%s(): unknown INT status! ignored!\n", __func__);
			}
		} else
			warp_dbg(WARP_DBG_ERR, "%s(): Empty tasks! ignored!\n", __func__);
	} else
		warp_dbg(WARP_DBG_ERR, "%s(): invalid wed address! ignored!\n", __func__);

	return 0;
}
#endif	/* WED_DYNAMIC_RXBM_SUPPORT */

void
wed_rx_bm_exit(struct wed_entry *wed)
{
	struct wed_res_ctrl *res_ctrl = &wed->res_ctrl;
	struct wed_rx_ctrl *rx_ctrl = &res_ctrl->rx_ctrl;
	struct wed_rx_bm_res *res = &rx_ctrl->res;
	u32 len;
	u8 i;

	len = sizeof(struct warp_rx_ring) * res->ring_num;
	if (res->ring) {
		for (i = 0; i < res->ring_num; i++)
			wed_rx_bm_ring_exit(wed, &res->ring[i], res->ring_len, &res->desc[i]);

		/*free wed rx ring*/
		warp_os_free_mem(res->ring);
		res->ring = NULL;
	}
	if (res->desc) {
		/*free desc*/
		warp_os_free_mem(res->desc);
		res->desc = NULL;
	}

#ifdef WED_DYNAMIC_RXBM_SUPPORT
	if (IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_RXBM)) {
		res = &rx_ctrl->extra;
		if (res->ring) {
			for (i = 0; i < (rx_ctrl->budget_tail_idx-1); i++) {
				wed_rx_budget_ring_exit(wed, &res->ring[i], res->ring_len, &res->desc[i]);
			}

			for (i = 0; i < 2; i++) {
				if (res->rx_page[i].va) {
					struct page *page;
#ifdef WED_MEM_LEAK_DBG
					pkt_free_mem(res->rx_page[i].va);
#endif	/* WED_MEM_LEAK_DBG */
					page = virt_to_page(res->rx_page[i].va);
					rx_bm_page_frag_cache_drain(page, res->rx_page[i].pagecnt_bias);
					memset(&res->rx_page[i], 0, sizeof(res->rx_page[i]));
				}
			}

			/*free wed rx ring*/
			warp_os_free_mem(res->ring);
			res->ring = NULL;
		}

		if (res->desc) {
			/*free desc*/
			warp_os_free_mem(res->desc);
			res->desc = NULL;
		}

		/*free wed rx ring*/
		warp_os_free_mem(res->ring);
		/*free desc*/
		warp_os_free_mem(res->desc);
	}
#endif	/* WED_DYNAMIC_RXBM_SUPPORT */
}

int
wed_rx_bm_init(struct wed_entry *wed, struct wifi_hw *hw)
{
	struct wed_res_ctrl *res_ctrl = &wed->res_ctrl;
#if defined(WED_DYNAMIC_RXBM_SUPPORT)
	struct dybm_conf_t *sw_conf = &wed->sw_conf->rxbm;
#endif	/* WED_DYNAMIC_RXBM_SUPPORT */
	struct wed_rx_ctrl *rx_ctrl = &res_ctrl->rx_ctrl;
	struct wed_rx_bm_res *res = &rx_ctrl->res;
	struct wed_rx_bm_res *extra_res = &rx_ctrl->extra;
	u32 len;

	if (hw) {
		res->ring_num = 1;
		res->ring_len = 65536;
		res->rxd_len = sizeof(struct warp_bm_rxdmad);
		extra_res->rxd_len = sizeof(struct warp_bm_rxdmad);
		res->pkt_num = hw->hw_rx_token_num;
		res->pkt_len = hw->rx_pkt_size;
		res->hif_dev = hw->hif_dev;
		extra_res->hif_dev = hw->hif_dev;
		extra_res->pkt_len = hw->rx_pkt_size;

#if defined(WED_DYNAMIC_RXBM_SUPPORT)
		if (sw_conf->vld_group) {
			if (sw_conf->vld_group*128 <= hw->hw_rx_token_num) {
				res->pkt_num = sw_conf->vld_group * 128;
			} else {
				warp_dbg(WARP_DBG_ERR, "%s(): RXBM exceed maximum! Forced as %d!\n",
						 __func__, hw->hw_rx_token_num);
			}
		}

		if ((wed->sub_ver == 2) && (sw_conf->enable))
			wed->hw_cap |= WED_HW_CAP_DYN_RXBM;

		if (IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_RXBM)) {
			u32 rx_bm_max = 0;

			if (sw_conf->max_group) {
				if (sw_conf->max_group*128 <= hw->hw_rx_token_num)
					rx_bm_max = sw_conf->max_group*128;
				else {
					rx_bm_max = hw->hw_rx_token_num;
					warp_dbg(WARP_DBG_ERR, "%s(): Max. RXBM exceed maximum:%d! Change to be %d\n",
							 __func__, hw->hw_rx_token_num, hw->hw_rx_token_num);
					sw_conf->max_group = rx_bm_max/128;
				}
			} else
				rx_bm_max = hw->hw_rx_token_num;

			if (sw_conf->vld_group) {
				res->pkt_num = sw_conf->vld_group * 128;

				if ((sw_conf->buf_high == 0)
					|| (sw_conf->buf_high > sw_conf->vld_group))
					sw_conf->buf_high = sw_conf->vld_group;
			} else {
				if (sw_conf->buf_high)
					res->pkt_num = (sw_conf->buf_high + 1) * 128;
				else {
					res->pkt_num = (ceil(hw->rx_ring_size * hw->rx_ring_num, 128) + sw_conf->alt_quota) * 128;
					sw_conf->buf_high = (ceil(hw->rx_ring_size * hw->rx_ring_num, 128) + sw_conf->alt_quota);
					warp_dbg(WARP_DBG_OFF, "%s(): Both valid & buffer high threshold are not specified!\n", __func__);
					warp_dbg(WARP_DBG_OFF, "%s(): Set valid as:%d, buffer low threshold as:%d\n", __func__,
						res->pkt_num / 128,
						ceil(hw->rx_ring_size * hw->rx_ring_num, 128));
				}
			}

			if (sw_conf->buf_high)
				sw_conf->buf_low = (sw_conf->buf_high - 1) * 128 + 1;

			extra_res->ring_num = (rx_bm_max - res->pkt_num)/(sw_conf->alt_quota * 128);
			extra_res->ring_len = (sw_conf->alt_quota * 128);
			rx_ctrl->budget_head_idx = 0;
			warp_dbg(WARP_DBG_OFF, "%s(): prepare %d rxbm virtual rings\n", __func__, extra_res->ring_num);
		}
#endif	/* WED_DYNAMIC_RXBM_SUPPORT */
		warp_dbg(WARP_DBG_INF, "%s(): allocate %d-of-dmad RX BM!\n", __func__, res->pkt_num);

		/*allocate wed rx descript for original chip ring*/
		len = sizeof(struct warp_dma_buf) * res->ring_num;
		warp_os_alloc_mem((unsigned char **)&res->desc, len, GFP_ATOMIC);

		if (!res->desc) {
			warp_dbg(WARP_DBG_ERR, "%s(): allocate rx bm desc faild\n", __func__);
			goto err;
		}
		memset(res->desc, 0, len);

		/*allocate wed rx ring, assign initial value */
		len = sizeof(struct warp_rx_ring) * res->ring_num;
		warp_os_alloc_mem((unsigned char **)&res->ring, len, GFP_ATOMIC);

		if (!res->ring) {
			warp_dbg(WARP_DBG_ERR, "%s(): allocate rx bm rx ring faild\n", __func__);
			goto err;
		}
		memset(res->ring, 0, len);

		if (wed_rx_bm_ring_init(wed, 0, res) < 0) {
			warp_dbg(WARP_DBG_ERR, "%s(): init rx bm rx ring faild\n", __func__);
			goto err;
		}
		wed_rx_bm_token_init(wed, 0, res, res->pkt_num);

#if defined(WED_DYNAMIC_RXBM_SUPPORT)
		if (IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_RXBM)) {
			u32 budget_idx = 0;

			/*allocate wed rx descript for extended chip ring*/
			len = sizeof(struct warp_dma_buf) * extra_res->ring_num;

			if (warp_os_alloc_mem((unsigned char **)&extra_res->desc, len, GFP_ATOMIC)) {
				warp_dbg(WARP_DBG_ERR, "%s(): allocate rx extra bm desc faild\n", __func__);
				goto err;
			}
			memset(extra_res->desc, 0, len);

			/*allocate wed rx ring, assign initial value */
			len = sizeof(struct warp_rx_ring) * extra_res->ring_num;

			if (warp_os_alloc_mem((unsigned char **)&extra_res->ring, len, GFP_ATOMIC)) {
				warp_dbg(WARP_DBG_ERR, "%s(): allocate rx extra bm rx ring faild\n", __func__);
				goto err;
			}
			memset(extra_res->ring, 0, len);

			extra_res->pkt_num = extra_res->ring_len;

			res->budget_grp = sw_conf->budget_limit;

			for (budget_idx = 0 ; budget_idx < res->budget_grp/sw_conf->alt_quota ; budget_idx++) {
				if (wed_rx_budget_ring_init(wed, budget_idx, extra_res) < 0) {
					warp_dbg(WARP_DBG_ERR, "%s(): init rx extra bm rx ring faild\n", __func__);
					goto err;
				}
#ifdef WARP_DVT
				warp_dbg(WARP_DBG_OFF, "%s(): pre-allocate budget w/ extra ring[%d]!\n",
						 __func__, budget_idx);
#endif	/* WARP_DVT */
			}

			rx_ctrl->budget_tail_idx += res->budget_grp/sw_conf->alt_quota;
#ifdef EXTEND_POLLING
			spin_lock_init(&rx_ctrl->recy_lock);
			timer_setup(&res->extend_monitor, rxbm_free_cache_handler, 0);
#endif	/* EXTEND_POLLING */
		}
#endif	/* WED_DYNAMIC_RXBM_SUPPORT */
	} else
		warp_dbg(WARP_DBG_ERR, "%s(): Invalid wifi_hw address!\n", __func__);

	return 0;

err:
	wed_rx_bm_exit(wed);
	return -1;
}

