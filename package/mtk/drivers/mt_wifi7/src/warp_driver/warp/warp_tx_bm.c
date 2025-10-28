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

/*
*
*/
void
dump_pkt_info(struct wed_buf_res *res)
{
	struct wed_pkt_info *info = NULL;
	struct wed_bm_group_info *grp = NULL;
	struct list_head *grp_cur = NULL, *pkt_cur = NULL;

	list_for_each(grp_cur, &res->grp_head) {
		grp = list_entry(grp_cur, struct wed_bm_group_info, list);

		if (grp) {
				list_for_each(pkt_cur, &grp->pkt_head) {
				info = list_entry(pkt_cur, struct wed_pkt_info, list);

				if (info) {
					warp_dbg(WARP_DBG_OFF, "desc_len\t: %d\n", info->desc_len);
					warp_dbg(WARP_DBG_OFF, "desc_pa\t: %pad\n", &info->desc_pa);
					warp_dbg(WARP_DBG_OFF, "desc_va\t: 0x%p\n", info->desc_va);
					warp_dbg(WARP_DBG_OFF, "len\t: %d\n", info->len);
					warp_dbg(WARP_DBG_OFF, "pkt\t: 0x%p\n", info->pkt);
					warp_dbg(WARP_DBG_OFF, "pkt_pa\t: %pad\n", &info->pkt_pa);
					warp_dbg(WARP_DBG_OFF, "pkt_va\t: 0x%p\n", info->pkt_va);
					warp_dbg(WARP_DBG_OFF, "fd_len\t: %d\n", info->fd_len);
					warp_dbg(WARP_DBG_OFF, "fd_pa\t: %pad\n", &info->fdesc_pa);
					warp_dbg(WARP_DBG_OFF, "fd_va\t: 0x%p\n", info->fdesc_va);
					warp_dump_raw("WED_TX_DMAD", info->desc_va, info->desc_len);
					warp_dump_raw("WED_TX_BM", info->fdesc_va, info->fd_len);
				}
			}
		}
	}

#ifdef WED_DYNAMIC_TXBM_SUPPORT
	list_for_each(grp_cur, &res->budget_head) {
		grp = list_entry(grp_cur, struct wed_bm_group_info, list);

		if (grp) {
				list_for_each(pkt_cur, &grp->pkt_head) {
				info = list_entry(pkt_cur, struct wed_pkt_info, list);

				if (info) {
					warp_dbg(WARP_DBG_OFF, "desc_len\t: %d\n", info->desc_len);
					warp_dbg(WARP_DBG_OFF, "desc_pa\t: %pad\n", &info->desc_pa);
					warp_dbg(WARP_DBG_OFF, "desc_va\t: 0x%p\n", info->desc_va);
					warp_dbg(WARP_DBG_OFF, "len\t: %d\n", info->len);
					warp_dbg(WARP_DBG_OFF, "pkt\t: 0x%p\n", info->pkt);
					warp_dbg(WARP_DBG_OFF, "pkt_pa\t: %pad\n", &info->pkt_pa);
					warp_dbg(WARP_DBG_OFF, "pkt_va\t: 0x%p\n", info->pkt_va);
					warp_dbg(WARP_DBG_OFF, "fd_len\t: %d\n", info->fd_len);
					warp_dbg(WARP_DBG_OFF, "fd_pa\t: %pad\n", &info->fdesc_pa);
					warp_dbg(WARP_DBG_OFF, "fd_va\t: 0x%p\n", info->fdesc_va);
					warp_dump_raw("WED_TX_DMAD", info->desc_va, info->desc_len);
					warp_dump_raw("WED_TX_BM", info->fdesc_va, info->fd_len);
				}
			}
		}
	}
#endif	/* WED_DYNAMIC_TXBM_SUPPORT */
}

/*
*
*/
static int
grp_buf_init(
	struct wed_entry *wed,
	struct wed_buf_res *res,
	struct list_head *pkt_head)
{
	struct wed_pkt_info *info = NULL;
	struct list_head *cur = NULL, *next = NULL;

	list_for_each_safe(cur, next, pkt_head) {
		info = list_entry(cur, struct wed_pkt_info, list);

		if (info) {
			/*init firt buf with MAC TXD+CR4 TXP*/
			wed_fdesc_init(wed, info);
		}
	}

	return 0;
}

/*
*
*/
static void
grp_info_free(
	struct wed_entry *wed,
	struct wed_buf_res *res,
	struct list_head *head)
{
	struct wed_pkt_info *info = NULL;
	struct list_head *cur = NULL, *next = NULL;
	struct platform_device *pdev = NULL;

	if (wed && res) {
		pdev = wed->pdev;

		list_for_each_safe(cur, next, head) {
			info = list_entry(cur, struct wed_pkt_info, list);

			if (info) {
				if (info->pkt_pa && info->pkt) {
#ifdef WED_MEM_LEAK_DBG
					pkt_free_mem(((void *)info->pkt);
#endif	/* WED_MEM_LEAK_DBG */
					dma_unmap_single(&pdev->dev, info->pkt_pa, info->len, DMA_TO_DEVICE);
					page_frag_free(info->pkt);
				} else
					warp_dbg(WARP_DBG_ERR, "%s(): invalid pointer(pkt_pa:%pad, pkt:0x%p)\n",
						__func__, &info->pkt_pa, info->pkt);
				list_del(&info->list);
				memset(info, 0, sizeof(*info));
				warp_os_free_mem(info);
			} else {
				warp_dbg(WARP_DBG_ERR, "%s(): invalid pointer(info:0x%p)\n", __func__, info);
			}
		}
	} else {
		warp_dbg(WARP_DBG_ERR, "%s(): invalid pointer(entry:0x%p, res:0x%p)\n", __func__, wed, res);
	}
}

/*
*
*/
static int
grp_info_alloc(
	struct wed_entry *wed,
	struct wed_buf_res *res,
	u32 start,
	u32 size,
	struct list_head *head)
{
	u32 i;
	u32 end = start + size;
	struct wed_pkt_info *info;
	struct platform_device *pdev = wed->pdev;

	/*prepare info and add to list */
	for (i = start; i < end; i++) {
		/*allocate token info*/
		warp_os_alloc_mem((unsigned char **)&info, sizeof(struct wed_pkt_info), GFP_ATOMIC);

		if (info == NULL) {
			warp_dbg(WARP_DBG_ERR, "%s(): allocate token %d info fail!\n", __func__, i);
			goto err;
		}

		memset(info, 0, sizeof(struct wed_pkt_info));
		info->len = res->pkt_len;
		/*allocate skb*/

#if (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
		info->pkt = (struct sk_buff *) page_frag_alloc(&res->tx_page, info->len, GFP_ATOMIC);
#else
		info->pkt = (struct sk_buff *) __alloc_page_frag(&res->tx_page, info->len, GFP_ATOMIC);
#endif
		if (unlikely(!info->pkt)) {
			warp_dbg(WARP_DBG_ERR, "%s(): allocate pkt for token %d fail!\n", __func__, i);
			warp_os_free_mem(info);
			goto err;

		}
#ifdef WED_MEM_LEAK_DBG
		pkt_alloc_mem(info->len, (void *)info->pkt, __builtin_return_address(0));
#endif	/* WED_MEM_LEAK_DBG */

		info->pkt_va = (unsigned char *)info->pkt;
		info->pkt_pa = dma_map_single(&pdev->dev, info->pkt_va, info->len, DMA_TO_DEVICE);

		if (unlikely(dma_mapping_error(&pdev->dev, info->pkt_pa))){
			warp_dbg(WARP_DBG_ERR, "%s(): dma map for token %d fail!\n", __func__, i);
			warp_os_free_mem(info);
			goto err;
		}

		info->token_id = i;
		/*allocate txd*/
		info->desc_len = res->dmad_len;
		info->desc_pa = res->des_buf.alloc_pa + (i * (dma_addr_t)info->desc_len);
		info->desc_va = res->des_buf.alloc_va + (i * info->desc_len);
		/*allocate first buffer*/
		info->fd_len = res->fd_len;
		info->fdesc_pa = res->fbuf.alloc_pa + (i * (dma_addr_t)info->fd_len);
		info->fdesc_va = res->fbuf.alloc_va + (i * info->fd_len);
		/*insert to list*/
		list_add(&info->list, head);
	}

	return 0;

err:
	grp_info_free(wed, res, head);
	return -1;
}

static void tx_page_frag_cache_drain(struct page *page,
								unsigned int count)
{
#if (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
		__page_frag_cache_drain(page, count);
#else
		atomic_sub(count - 1, &page->_count);
		__free_pages(page, compound_order(page));
#endif
}


/*
*
*/
static void
bm_deinit_all(struct wed_entry *wed, struct wed_buf_res *res)
{
	struct list_head *cur = NULL, *next = NULL;
	struct platform_device *pdev = wed->pdev;
	struct page *page;

	if (!res->des_buf.alloc_va)
		return;

	list_for_each_safe(cur, next, &res->grp_head) {
		struct wed_bm_group_info *grp = list_entry(cur, struct wed_bm_group_info, list);

		if (grp) {
			grp_info_free(wed, res, &grp->pkt_head);
			list_del(&grp->list);
			memset(grp, 0, sizeof(*grp));
			warp_os_free_mem(grp);
		}
	}

#ifdef WED_DYNAMIC_TXBM_SUPPORT
	list_for_each_safe(cur, next, &res->budget_head) {
		struct wed_bm_group_info *grp = list_entry(cur, struct wed_bm_group_info, list);

		if (grp) {
			grp_info_free(wed, res, &grp->pkt_head);
			list_del(&grp->list);
			memset(grp, 0, sizeof(*grp));
			warp_os_free_mem(grp);
		}
	}
#endif	/* WED_DYNAMIC_TXBM_SUPPORT */

	if (res->tx_page.va) {
		page = virt_to_page(res->tx_page.va);
		tx_page_frag_cache_drain(page, res->tx_page.pagecnt_bias);
		memset(&res->tx_page, 0, sizeof(res->tx_page));
	}

	warp_dma_buf_free(pdev, &res->des_buf);
	warp_dma_buf_free(pdev, &res->fbuf);
}

/*
*
*/
static int
bm_init_all(struct wed_entry *wed, struct wed_buf_res *res)
{
	u32 len = 0, i = 0;
	struct list_head *cur = NULL, *next = NULL;

	/*allocate wed dmad descript buffer for H/W capability*/
	len = res->dmad_len * (res->bm_max_grp * res->bm_grp_sz);

	/*initial list */
	INIT_LIST_HEAD(&res->grp_head);

	if (warp_dma_buf_alloc(wed->pdev, &res->des_buf, len) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate txd buffer fail!\n", __func__);
		goto err;
	}

	/*allocate wed txd + txp buffer for H/W capability*/
	len = res->fd_len * (res->bm_max_grp * res->bm_grp_sz);

	if (warp_dma_buf_alloc(wed->pdev, &res->fbuf, len) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate txd buffer fail!\n", __func__);
		goto err;
	}

	/* fill descriptor for initial packets number as bm_vld_grp*group_sz */
	for (i = 0 ; i < res->bm_vld_grp ; i++) {
		struct wed_bm_group_info *grp_info = NULL;

		warp_os_alloc_mem((unsigned char **)&grp_info, sizeof(struct wed_bm_group_info), GFP_ATOMIC);

		if (grp_info == NULL) {
			warp_dbg(WARP_DBG_ERR, "%s(): allocate group %d info fail!\n", __func__, i);
			goto err;
		}

		grp_info->skb_id_start = i*res->bm_grp_sz;
		INIT_LIST_HEAD(&grp_info->pkt_head);

		if (grp_info_alloc(wed, res, grp_info->skb_id_start, res->bm_grp_sz, &grp_info->pkt_head) < 0)
			goto err;

		list_add(&grp_info->list, &res->grp_head);

		warp_dbg(WARP_DBG_INF, "%s(): Add grp of %u to initial pool!\n", __func__, grp_info->skb_id_start);
	}

	list_for_each_safe(cur, next, &res->grp_head) {
		struct wed_bm_group_info *grp_info = list_entry(cur, struct wed_bm_group_info, list);

		grp_buf_init(wed, res, &grp_info->pkt_head);
	}

	INIT_LIST_HEAD(&res->budget_head);
#ifdef WED_DYNAMIC_TXBM_SUPPORT
	if (IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_TXBM)) {
		for (; i < res->bm_vld_grp + wed->sw_conf->txbm.budget_limit; i++) {
			struct wed_bm_group_info *grp_info = NULL;

			warp_os_alloc_mem((unsigned char **)&grp_info, sizeof(struct wed_bm_group_info), GFP_ATOMIC);

			if (grp_info == NULL) {
				warp_dbg(WARP_DBG_ERR, "%s(): allocate group %d info fail!\n", __func__, i);
				goto err;
			}

			grp_info->skb_id_start = i*res->bm_grp_sz;
			INIT_LIST_HEAD(&grp_info->pkt_head);

			if (grp_info_alloc(wed, res, grp_info->skb_id_start, res->bm_grp_sz, &grp_info->pkt_head) < 0)
				goto err;

			list_add_tail(&grp_info->list, &res->budget_head);
			warp_dbg(WARP_DBG_INF, "%s(): Append grp of %u to budge!\n", __func__, grp_info->skb_id_start);

			res->budget_grp++;
		}
		res->pkt_num += res->budget_grp * res->bm_grp_sz;

		list_for_each_safe(cur, next, &res->budget_head) {
			struct wed_bm_group_info *grp_info = list_entry(cur, struct wed_bm_group_info, list);

			grp_buf_init(wed, res, &grp_info->pkt_head);
		}
	}
#endif	/* WED_DYNAMIC_TXBM_SUPPORT */

	return 0;

err:
	return -1;
}

#ifdef WED_DYNAMIC_TXBM_SUPPORT
/*
 *
 */
static void
tbudge_refill_handler(unsigned long data)
{
	u32 i = 0, quota = 0;
	struct wed_entry *wed = (struct wed_entry *)data;
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;
#ifdef WARP_DVT
	unsigned long now = sched_clock(), after = 0;
#endif	/* WARP_DVT */

	if (res->budget_grp < wed->sw_conf->txbm.budget_limit)
		quota = wed->sw_conf->txbm.budget_limit - res->budget_grp;
	else
		goto err;

	for (i = res->pkt_num/res->bm_grp_sz ;
		 (i < (res->pkt_num/res->bm_grp_sz) + quota) && (i < res->bm_max_grp);
		 i++) {
		struct wed_bm_group_info *grp_info = NULL;

		warp_os_alloc_mem((unsigned char **)&grp_info, sizeof(struct wed_bm_group_info), GFP_ATOMIC);

		if (grp_info == NULL) {
			warp_dbg(WARP_DBG_ERR, "%s(): allocate group %d info fail!\n", __func__, i);
		} else {
			grp_info->skb_id_start = i*res->bm_grp_sz;
			INIT_LIST_HEAD(&grp_info->pkt_head);

			if (grp_info_alloc(wed, res, grp_info->skb_id_start, res->bm_grp_sz, &grp_info->pkt_head) < 0) {
				res->dybm_stat.budget_refill_failed++;
				warp_dbg(WARP_DBG_ERR, "%s(): allocate packet %d info fail!\n", __func__, i);
			} else {
				grp_buf_init(wed, res, &grp_info->pkt_head);

				list_add_tail(&grp_info->list, &res->budget_head);
				warp_dbg(WARP_DBG_INF, "%s(): budge extend with group %u!\n", __func__, grp_info->skb_id_start);
				res->budget_grp++;
			}
		}
	}

err:
#ifdef WARP_DVT
	after = sched_clock();
#endif	/* WARP_DVT */
	if (quota) {
		res->dybm_stat.budget_refill += (i-res->pkt_num/res->bm_grp_sz);
		res->pkt_num += (i-res->pkt_num/res->bm_grp_sz)*res->bm_grp_sz;
		warp_dbg(WARP_DBG_INF, "%s(): Enlarge %d group(s)!\n", __func__, quota);
#ifdef WARP_DVT
		warp_dbg(WARP_DBG_OFF, "%s(): process time: %ld us(%ld-%ld)\n",
				 __func__, (after-now)/1000, after, now);
#endif	/* WARP_DVT */
	} else
		warp_dbg(WARP_DBG_INF, "%s(): Exceed max. group!(%d)\n",
				 __func__, res->bm_max_grp);
}

/*
 *
 */
static void
tbudge_release_handler(unsigned long data)
{
	u32 quota = 0;
	struct wed_entry *wed = (struct wed_entry *)data;
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;

	if (res->budget_grp > wed->sw_conf->txbm.budget_limit) {
		quota = res->budget_grp - wed->sw_conf->txbm.budget_limit;
		res->dybm_stat.budget_release += quota;
		res->pkt_num -= quota*res->bm_grp_sz;

		warp_dbg(WARP_DBG_INF, "%s(): Release %d group(s)!\n", __func__, quota);
	}

	while(quota) {
		struct wed_bm_group_info *grp_info = list_last_entry(&res->budget_head, struct wed_bm_group_info, list);

		if (grp_info) {
			list_del(&grp_info->list);
			grp_info_free(wed, res, &grp_info->pkt_head);
			warp_os_free_mem(grp_info);

			res->budget_grp--;
			quota--;
		} else
			break;
	}
}

/*
*
*/
static int
bm_buf_extend(struct wed_entry *wed, u32 grp_num)
{
	int ret = -1;
	struct wed_tx_ctrl *tx_ctrl = &wed->res_ctrl.tx_ctrl;
	struct wed_buf_res *res = &tx_ctrl->res;

	if (!list_empty(&res->budget_head)) {
		warp_dbg(WARP_DBG_INF, "%s(): Occupy %d group(s) from budge!\n", __func__, grp_num);

		while (grp_num) {
			struct wed_bm_group_info *grp_info = list_first_entry(&res->budget_head, struct wed_bm_group_info, list);

			if (grp_info) {
				list_move(&grp_info->list, &res->grp_head);
				grp_num--;
				res->bm_vld_grp++;
				res->budget_grp--;
			} else
				break;
		}

		if (res->bm_vld_grp > res->dybm_stat.max_vld_grp)
			res->dybm_stat.max_vld_grp = res->bm_vld_grp;

		wed->tbuf_alloc_times++;

		ret = 0;
	}

	return ret;
}

/*
*
*/
static void
bm_buf_reduce(struct wed_entry *wed, u32 grp_num)
{
	struct wed_tx_ctrl *tx_ctrl = &wed->res_ctrl.tx_ctrl;
	struct wed_buf_res *res = &tx_ctrl->res;

	warp_dbg(WARP_DBG_INF, "%s(): Recycle %d group(s) from budge!\n", __func__, grp_num);

	while (grp_num) {
		struct wed_bm_group_info *grp_info = list_first_entry(&res->grp_head, struct wed_bm_group_info, list);

		if (grp_info) {
			list_move(&grp_info->list, &res->budget_head);
			grp_num--;
			res->bm_vld_grp--;
			res->budget_grp++;

			warp_dbg(WARP_DBG_INF, "%s(): Recycle grp of %u to budge pool!\n", __func__, grp_info->skb_id_start);
		} else
			break;
	}

	if (res->dybm_stat.min_vld_grp == 0)
		res->dybm_stat.min_vld_grp = res->bm_vld_grp;
	else if (res->bm_vld_grp < res->dybm_stat.min_vld_grp)
		res->dybm_stat.min_vld_grp = res->bm_vld_grp;

	wed->tbuf_free_times++;
}

/*
*
*/
void
buf_free_task(unsigned long data)
{
	struct wed_entry *wed = (struct wed_entry *)data;
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;
	u32 reduce_grp = 0, value = 0;
#ifdef WARP_DVT
	unsigned long now = sched_clock(), after = 0;
#endif	/* WARP_DVT */

	if (wed == NULL) {
		warp_dbg(WARP_DBG_ERR, "%s(): invalid wed address! dismissed!\n", __func__);

		goto err;
	}

	value = warp_get_recycle_grp_idx(wed);

	if (((value+1) < res->bm_vld_grp) && ((value+1) >= res->bm_rsv_grp)) {
		reduce_grp = res->bm_vld_grp - (value+1);

		if (reduce_grp > wed->sw_conf->txbm.alt_quota)
			reduce_grp -= wed->sw_conf->txbm.alt_quota;
		else
			reduce_grp = 0;
	}

	warp_dbg(WARP_DBG_INF, "%s(): old packet num:%d\n", __func__, res->pkt_num);
	warp_dbg(WARP_DBG_INF, "%s(): reduce:%d packets(%d groups)\n", __func__, reduce_grp*res->bm_grp_sz, reduce_grp);

	if (reduce_grp) {
		warp_dbg(WARP_DBG_INF, "%s(): recycle index:0x%x, rsv_group:0x%x, vld_group:0x%x, reduce:%d\n",
				 __func__, value, res->bm_rsv_grp, res->bm_vld_grp, reduce_grp);
		bm_buf_reduce(wed, reduce_grp);
		warp_bfm_update_hw(wed, true);
		warp_dbg(WARP_DBG_INF, "%s(): update packet num:%d\n", __func__, res->pkt_num);

		if (res->budget_grp > wed->sw_conf->txbm.budget_limit) {
			struct dybm_dl_tasks *tasks = NULL;

			if (wed->dybm_dl_tasks) {
				tasks = (struct dybm_dl_tasks *)wed->dybm_dl_tasks;
				tasklet_hi_schedule(&tasks->tbudge_release_task);
			} else
				warp_dbg(WARP_DBG_ERR, "%s(): empty tasks! dismissed!\n", __func__);
		}

		res->dybm_stat.shk_times++;
	} else {
		warp_dbg(WARP_DBG_INF, "%s(): packet num remain:%d\n", __func__, res->pkt_num);

		res->dybm_stat.shk_unhanlded++;
	}

	dybm_eint_ctrl(wed, true, WARP_DYBM_EINT_BM_H);
#ifdef WARP_DVT
	after = sched_clock();
	warp_dbg(WARP_DBG_OFF, "%s(): process time: %ld us(%ld-%ld)\n",
			 __func__, (after-now)/1000, after, now);
#endif /* WARP_DVT */

err:
	return;
}

/*
*
*/
void
buf_alloc_task(unsigned long data)
{
	struct wed_entry *wed = (struct wed_entry *)data;
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;
	u32 tkn_quota = 0, value= 0;
#ifdef WARP_DVT
	unsigned long now = sched_clock(), after = 0;
#endif	/* WARP_DVT */

	value = warp_get_recycle_grp_idx(wed);
	if (res->recycle_grp_idx == value) {
		warp_dbg(WARP_DBG_INF, "%s(): recycle idx:%d is not changed! dismissed!\n", __func__, value);
		res->dybm_stat.ext_unhanlded++;
		goto done;
	} else
		res->recycle_grp_idx = value;

	res->dybm_stat.ext_times++;

	warp_dbg(WARP_DBG_INF, "%s(): old packet num:%d\n", __func__, res->pkt_num);

	if (res->budget_grp >= wed->sw_conf->txbm.alt_quota) {
		if (bm_buf_extend(wed, wed->sw_conf->txbm.alt_quota) == 0) {
			warp_bfm_update_hw(wed, false);

			tkn_quota = (res->token_num - (res->tkn_vld_grp*res->tkn_grp_sz));

			if (tkn_quota) {
				u32 tkn_ext_grp = 0;

				if (tkn_quota >= wed->sw_conf->txbm.alt_quota*res->tkn_grp_sz) {
					tkn_ext_grp = wed->sw_conf->txbm.alt_quota;
					tkn_quota = wed->sw_conf->txbm.alt_quota*res->tkn_grp_sz;
				} else if (tkn_quota) {
					tkn_ext_grp = (tkn_quota/res->tkn_grp_sz) + ((tkn_quota%res->tkn_grp_sz) ? 1 : 0);
				}

				res->tkn_vld_grp += tkn_ext_grp;
				warp_btkn_update_hw(wed, false);

				warp_dbg(WARP_DBG_INF, "%s(): update token num:%d(+%d tokens/%d group)\n",
						 __func__, res->tkn_vld_grp*res->tkn_grp_sz, tkn_quota, tkn_ext_grp);
			}
			warp_dbg(WARP_DBG_INF, "%s(): update packet num:%d(+%d packets/%d group)\n",
					 __func__, res->pkt_num, wed->sw_conf->txbm.alt_quota*res->bm_grp_sz, wed->sw_conf->txbm.alt_quota);

			if ((res->budget_grp < wed->sw_conf->txbm.budget_limit) &&
				(res->budget_grp <= wed->sw_conf->txbm.budget_refill_watermark)) {
				struct dybm_dl_tasks *tasks = NULL;

				if (wed->dybm_dl_tasks) {
					tasks = (struct dybm_dl_tasks *)wed->dybm_dl_tasks;
					tasklet_hi_schedule(&tasks->tbudge_refill_task);
				} else
					warp_dbg(WARP_DBG_ERR, "%s(): empty tasks! dismissed!\n", __func__);
			}
		} else {
			warp_dbg(WARP_DBG_ERR, "%s(): BM size remain:%d due to budge being exhausted!\n",
					 __func__, res->pkt_num);
			res->dybm_stat.ext_failed++;
		}
	} else
		warp_dbg(WARP_DBG_INF, "%s(): memory required exceed capability! dismissed!\n", __func__);

done:
	dybm_eint_ctrl(wed, true, WARP_DYBM_EINT_BM_L);
#ifdef WARP_DVT
	after = sched_clock();
	warp_dbg(WARP_DBG_OFF, "%s(): process time: %ldus(%ld-%ld)\n",
				 __func__, (after-now)/1000, after, now);
#endif	/* WARP_DVT */
}

/*
*
*/
static void
token_alloc_task(unsigned long data)
{
	struct wed_entry *wed = (struct wed_entry *)data;
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;
	u32 size = 0;
	u32 buf_lmt = 0, tkn_quota = 0;
	unsigned long now = jiffies;

	tkn_quota = (res->token_num - (res->tkn_vld_grp*res->tkn_grp_sz));

	if (tkn_quota) {
		u32 tkn_ext_grp = 0;

		if (tkn_quota >= wed->sw_conf->txbm.alt_quota*res->tkn_grp_sz) {
			tkn_ext_grp = wed->sw_conf->txbm.alt_quota;
			tkn_quota = wed->sw_conf->txbm.alt_quota*res->tkn_grp_sz;
		} else if (tkn_quota) {
			tkn_ext_grp = (tkn_quota/res->tkn_grp_sz) + ((tkn_quota%res->tkn_grp_sz) ? 1 : 0);
		}

		res->tkn_vld_grp += tkn_ext_grp;

		if (IS_WED_HW_CAP(wed, WED_HW_CAP_32K_TXBUF))
			buf_lmt = res->bm_max_grp*res->bm_grp_sz;
		else
			buf_lmt = 8192;

		size = res->pkt_num + (tkn_ext_grp * res->bm_grp_sz);

		warp_dbg(WARP_DBG_INF, "%s(): old packet num:%d\n", __func__, res->pkt_num);

		if (size <= buf_lmt) {
			if (bm_buf_extend(wed, wed->sw_conf->txbm.alt_quota) == 0) {
				warp_bfm_update_hw(wed, false);
				warp_dbg(WARP_DBG_INF, "%s(): update packet num:%d(+%d packets/%d group)\n",
						 __func__, res->pkt_num, wed->sw_conf->txbm.alt_quota*res->bm_grp_sz, wed->sw_conf->txbm.alt_quota);
				warp_btkn_update_hw(wed, false);
				warp_dbg(WARP_DBG_INF, "%s(): update token num:%d(+%d tokens/%d group)\n",
							 __func__, res->tkn_vld_grp*res->tkn_grp_sz, tkn_quota, tkn_ext_grp);
			} else
				warp_dbg(WARP_DBG_ERR, "%s(): token num remain:%d due to allocate memory failed!\n",
						 __func__, res->tkn_vld_grp*res->tkn_grp_sz);
		} else
			warp_dbg(WARP_DBG_INF, "%s(): memory required exceed capability! dismissed!\n", __func__);
	}

	warp_dbg(WARP_DBG_INF, "%s(): process time:%ld(%ld-%ld)\n",
				 __func__, jiffies-now, jiffies, now);

	dybm_eint_ctrl(wed, true, WARP_DYBM_EINT_TKID_L);
}

int unregist_dl_dybm_task(struct wed_entry *wed)
{
	if (wed->dybm_dl_tasks) {
		warp_os_free_mem(wed->dybm_dl_tasks);
		wed->dybm_dl_tasks = NULL;
	}

	return 0;
}

int regist_dl_dybm_task(struct wed_entry *wed)
{
	int ret = -1;
	struct dybm_dl_tasks *tasks = NULL;

	if (wed == NULL) {
		warp_dbg(WARP_DBG_ERR, "%s(): invalid wed address!\n", __func__);
		goto err;
	}

	if (warp_os_alloc_mem((u8 **)&wed->dybm_dl_tasks, sizeof(struct dybm_dl_tasks), GFP_ATOMIC) == 0) {
		tasks = (struct dybm_dl_tasks *)wed->dybm_dl_tasks;

		tasklet_init(&tasks->tbuf_alloc_task, buf_alloc_task, (unsigned long)wed);
		tasklet_init(&tasks->tbuf_free_task, buf_free_task, (unsigned long)wed);
		tasklet_init(&tasks->tkn_alloc_task, token_alloc_task, (unsigned long)wed);
		tasklet_init(&tasks->tbudge_refill_task, tbudge_refill_handler, (unsigned long)wed);
		tasklet_init(&tasks->tbudge_release_task, tbudge_release_handler, (unsigned long)wed);

		ret = 0;
	} else
		warp_dbg(WARP_DBG_ERR, "%s(): memory to store tasks aloocated failed!\n", __func__);

err:
	return ret;
}

int dybm_dl_int_disp(struct wed_entry *wed, u32 status)
{
	struct dybm_dl_tasks *tasks = (struct dybm_dl_tasks *)wed->dybm_dl_tasks;

	if (tasks) {
		switch(status) {
			case WARP_DYBM_EINT_BM_H:
				tasklet_hi_schedule(&tasks->tbuf_free_task);
				break;
			case WARP_DYBM_EINT_BM_L:
				tasklet_hi_schedule(&tasks->tbuf_alloc_task);
				break;
			default:
				warp_dbg(WARP_DBG_ERR, "%s(): unknown INT status! ignored!\n", __func__);
		}
	} else
		warp_dbg(WARP_DBG_ERR, "%s(): Empty tasks! ignored!\n", __func__);

	return 0;
}
#endif /*WED_DYNAMIC_TXBM_SUPPORT*/

/*
*
*/
void
wed_txbm_exit(struct wed_entry *wed)
{
	struct wed_tx_ctrl *tx_ctrl = &wed->res_ctrl.tx_ctrl;
	struct wed_buf_res *res = &tx_ctrl->res;
	/*tx resource free*/
	bm_deinit_all(wed, res);
}

/*
*
*/
int
wed_txbm_init(struct wed_entry *wed, struct wifi_hw *hw)
{
	struct wed_tx_ctrl *tx_ctrl = &wed->res_ctrl.tx_ctrl;
	struct wed_buf_res *res = &tx_ctrl->res;
	struct sw_conf_t *sw_conf = wed->sw_conf;
	u32 init_grp = 0;
	/*tx resource allocate*/
	/* H/W capability */
	res->tkn_max_grp = 0x40;	/* 64 groups, 8192 tokens */
	res->bm_grp_sz = 0x80;		/* 128 packets per group */
	res->tkn_grp_sz = 0x80;		/* 128 token per group */

#ifdef WED_WDMA_DUAL_RX_RING
	if (wifi_dbdc_support(wed->warp) == false)	/* dbdc_mode is false, shrink TXBM to support single wdma ring */
		init_grp = ceil(sw_conf->rx_wdma_ring_depth, res->bm_grp_sz);
	else
#endif
		init_grp = ceil(sw_conf->rx_wdma_ring_depth*WDMA_RX_RING_NUM, res->bm_grp_sz);

#ifdef WED_DYNAMIC_TXBM_SUPPORT
	if (sw_conf->txbm.enable) {
		wed->hw_cap |= WED_HW_CAP_DYN_TXBM;
		res->bm_rsv_grp = init_grp;
		res->bm_vld_grp = init_grp+sw_conf->txbm.alt_quota;
	} else
#endif	/* WED_DYNAMIC_TXBM_SUPPORT */
	{
		/* once DYBM disabled, prepare BM size with rule, fullfill WDMA RX ring plus cut through token number */
		res->bm_rsv_grp = 0;
		res->bm_vld_grp = res->tkn_max_grp;
	}

	if (wed->ver >= 2) {
#ifdef MEMORY_SHRINK
		if (sw_conf->txbm.enable == false)
			res->bm_max_grp = res->bm_vld_grp;	/* the groups of packets */
		else
#endif
			res->bm_max_grp = MAX_GROUP_SIZE;	/* 256 groups, 32768 packets */
	} else
		res->bm_max_grp = 0x40; 	/* 64 groups, 8192 packets */

	if (sw_conf->txbm.max_group) {
		/* sanity to prevent exceed H/W capability */
		if (sw_conf->txbm.max_group > MAX_GROUP_SIZE) {
			warp_dbg(WARP_DBG_ERR,
				"%s(): TXBM exceed BM H/W cap(%d), correct to H/W cap(%d)!\n",
				__func__, sw_conf->txbm.max_group, res->bm_max_grp);
			sw_conf->txbm.max_group = MAX_GROUP_SIZE;
		}
		warp_dbg(WARP_DBG_INF,"%s(): TXBM set BM H/W cap(%d), old H/W cap(%d)!\n",
			__func__, sw_conf->txbm.max_group, res->bm_max_grp);
		res->bm_max_grp = sw_conf->txbm.max_group;
	}

	if (sw_conf->txbm.vld_group) {
		/* sanity to prevent exceed H/W capability */
		if (sw_conf->txbm.vld_group > res->bm_max_grp) {
			warp_dbg(WARP_DBG_ERR,
					 "%s(): TXBM exceed H/W capability(%d), correct to H/W cap(%d)!\n",
					 __func__, sw_conf->txbm.vld_group, res->bm_max_grp);
			sw_conf->txbm.vld_group = res->bm_max_grp;
		}

		if (sw_conf->txbm.vld_group > init_grp) {
			if (sw_conf->txbm.enable)
				res->bm_rsv_grp = sw_conf->txbm.vld_group-sw_conf->txbm.alt_quota;
			else
				res->bm_rsv_grp = sw_conf->txbm.vld_group;

			res->bm_vld_grp = sw_conf->txbm.vld_group;
		}
	}

	res->token_num = hw->tx_token_nums - hw->sw_tx_token_nums;
	if (res->tkn_max_grp > ceil(res->token_num, res->tkn_grp_sz))
		res->tkn_max_grp = ceil(res->token_num, res->tkn_grp_sz);
	res->tkn_rsv_grp = (res->token_num / res->tkn_grp_sz);
	res->tkn_vld_grp = (res->token_num / res->tkn_grp_sz);

	res->token_start = 0;
	res->token_end = (res->token_num - 1);

	res->wed_token_cnt = res->token_num;

	if (wed->ver >= 2)
		res->pkt_num = res->bm_vld_grp * res->bm_grp_sz;
	else
		res->pkt_num = res->wed_token_cnt;

	res->dmad_len = hw->txd_size;
	res->fd_len = hw->fbuf_size;
	res->pkt_len = hw->tx_pkt_size;
	res->recycle_grp_idx = 0;

	if (bm_init_all(wed, res) < 0)
		goto err;

	return 0;
err:
	/*error handle*/
	bm_deinit_all(wed, res);
	return -1;
}

