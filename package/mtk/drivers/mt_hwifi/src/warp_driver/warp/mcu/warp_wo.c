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
#include "warp.h"
#include "warp_wo.h"
#include "warp_fwdl.h"
#include "warp_cmd.h"
#include "warp_hw.h"
#include "warp_woif.h"
#include "warp_meminfo_list.h"

#include <linux/version.h>
#if defined(CONFIG_MTK_AEE_FEATURE)
#include <mt-plat/aee.h>
#endif


extern void wed_wo_init(struct wo_entry **wo);

extern void wo_proc_init(struct warp_entry *warp);
extern void wo_exep_proc_init(struct warp_entry *warp);

extern void ccif_bus_register(struct woif_bus *bus);

struct bus_dmad {
	__le32 sdp0;
	__le32 ctrl;
	__le32 sdp1;
	__le32 info;
	__le32 reserved[4];
} __packed __aligned(32);

#define CMD_BUFFER_LEN 1504//change to 32 byte alignment 1500

struct profiling_statistic_t profiling_total[6] = {
	{1001, 0},
	{1501, 0},
	{3001, 0},
	{5001, 0},
	{10001, 0},
	{0xffffffff, 0}
};

struct profiling_statistic_t profiling_mod[6] = {
	{1001, 0},
	{1501, 0},
	{3001, 0},
	{5001, 0},
	{10001, 0},
	{0xffffffff, 0}
};

struct profiling_statistic_t profiling_rro[6] = {
	{1001, 0},
	{1501, 0},
	{3001, 0},
	{5001, 0},
	{10001, 0},
	{0xffffffff, 0}
};


static int tx_dma_cb_init(
	struct woif_bus *bus,
	struct warp_dma_buf *desc,
	u32 idx,
	struct warp_dma_cb *dma_cb)
{
	struct bus_dmad *txd;
	u32 ctrl = 0;
	struct platform_device *pdev = bus->pdev;

	dma_cb->pkt_size = bus->tx_ring.pkt_len;

#if (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
	dma_cb->pkt = (struct sk_buff *) page_frag_alloc(&bus->woif_tx_page, dma_cb->pkt_size, GFP_ATOMIC|GFP_DMA32);
#else
	dma_cb->pkt = (struct sk_buff *) __alloc_page_frag(&bus->woif_tx_page, dma_cb->pkt_size, GFP_ATOMIC|GFP_DMA32);
#endif
	if (unlikely(!dma_cb->pkt)) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate bus pkt\n", __func__);
		goto err;
	}
#ifdef WED_MEM_LEAK_DBG
	pkt_alloc_mem(dma_cb->pkt_size, (void *)dma_cb->pkt, __builtin_return_address(0));
#endif /* WED_MEM_LEAK_DBG */
	dma_cb->pkt_va = (unsigned char *)dma_cb->pkt;
	dma_cb->pkt_pa = dma_map_single(&pdev->dev, dma_cb->pkt_va, dma_cb->pkt_size, DMA_TO_DEVICE);

	if (unlikely(dma_mapping_error(&pdev->dev, dma_cb->pkt_pa))){
		warp_dbg(WARP_DBG_ERR, "%s(): dma map for bus fail!\n", __func__);
		goto err;
	}

	dma_cb->alloc_size = sizeof(struct bus_dmad);
	dma_cb->alloc_va = desc->alloc_va + (idx * dma_cb->alloc_size);
	dma_cb->alloc_pa = desc->alloc_pa + (idx * dma_cb->alloc_size);
	txd = (struct bus_dmad *) dma_cb->alloc_va;
	ctrl = (bus->tx_ring.pkt_len << WED_CTL_SD_LEN0_SHIFT) & WED_CTL_SD_LEN0;
	ctrl |= WED_CTL_LAST_SEC0;
	WRITE_ONCE(txd->sdp0, cpu_to_le32(dma_cb->pkt_pa));
	WRITE_ONCE(txd->ctrl, cpu_to_le32(ctrl));
	return 0;

err:
	return -1;
}


static void tx_dma_cb_exit(
	struct woif_bus *bus,
	struct warp_dma_cb *dma_cb)
{

	if (bus && dma_cb) {
		if (dma_cb->pkt_pa && dma_cb->pkt) {
#ifdef WED_MEM_LEAK_DBG
			pkt_free_mem((unsigned char *)dma_cb->pkt);
#endif	/* WED_MEM_LEAK_DBG */
			dma_unmap_single(&bus->pdev->dev, dma_cb->pkt_pa, dma_cb->pkt_size, DMA_TO_DEVICE);
			page_frag_free(dma_cb->pkt);
		}
		dma_cb->pkt = NULL;
		memset(dma_cb, 0, sizeof(struct warp_dma_cb));
	} else {
		warp_dbg(WARP_DBG_ERR, "%s(): invalid pointer(dma_cb:0x%p, bus:0x%p)\n",
			 __func__, dma_cb, bus);
	}
}


static int tx_ring_init(struct woif_bus *bus, struct wo_ring_ctrl *tx_ctrl)
{
	struct warp_bus_ring *tx_ring = &tx_ctrl->ring;
	u32 ring_len = WED_BUS_RING_SIZE;
	u32 txd_size = sizeof(struct bus_dmad);
	struct warp_dma_buf *desc = &tx_ring->desc;
	int len = txd_size * ring_len;
	int i;

	spin_lock_init(&tx_ctrl->lock);
	tx_ctrl->pkt_len = CMD_BUFFER_LEN;
	tx_ctrl->ring_len = ring_len;
	memset(desc, 0, sizeof(struct warp_dma_buf));

	if (warp_dma_buf_alloc(bus->pdev, desc, len) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): alloc desc fail, len=%d\n", __func__, len);
		return -1;
	}

	for (i = 0 ; i < ring_len; i++) {
		tx_dma_cb_init(bus, desc, i, &tx_ring->cell[i]);
	}
	return 0;
}


static void tx_ring_exit(
	struct woif_bus *bus,
	struct wo_ring_ctrl *tx_ctrl)
{
	u32 i;
	struct warp_bus_ring *tx_ring = &tx_ctrl->ring;
	u32 ring_len = tx_ctrl->ring_len;

	for (i = 0 ; i < ring_len ; i++) {
		tx_dma_cb_exit(bus, &tx_ring->cell[i]);
	}
	warp_dma_buf_free(bus->pdev, &tx_ring->desc);
}


static int rx_dma_cb_init(
	struct woif_bus *bus,
	struct warp_dma_buf *desc,
	u32 idx,
	struct warp_dma_cb *dma_cb)
{
	struct bus_dmad *rxd;
	u32 ctrl = 0;
#if CCIF_PAGE_ALLOC
	struct platform_device *pdev = bus->pdev;
#endif

	dma_cb->pkt_size = bus->rx_ring.pkt_len;
	//dma_cb->pkt = bus_pkt_allocate(bus, dma_cb->pkt_size, &dma_cb->pkt_va, &dma_cb->pkt_pa);

#if CCIF_PAGE_ALLOC
#if (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
	dma_cb->pkt = (struct sk_buff *) page_frag_alloc(&bus->woif_rx_page,
							dma_cb->pkt_size, GFP_ATOMIC | GFP_DMA32);
#else
	dma_cb->pkt = (struct sk_buff *) __alloc_page_frag(&bus->woif_rx_page,
							dma_cb->pkt_size, GFP_ATOMIC | GFP_DMA32);
#endif

	if (unlikely(!dma_cb->pkt)) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate bus pkt\n", __func__);
		goto err;
	}

#ifdef WED_MEM_LEAK_DBG
	pkt_alloc_mem(dma_cb->pkt_size, (void *)dma_cb->pkt, __builtin_return_address(0));
#endif	/* WED_MEM_LEAK_DBG */

	dma_cb->pkt_va = (unsigned char *)dma_cb->pkt;
	dma_cb->pkt_pa = dma_map_single(&pdev->dev, dma_cb->pkt_va, dma_cb->pkt_size, DMA_FROM_DEVICE);

	if (unlikely(dma_mapping_error(&pdev->dev, dma_cb->pkt_pa))){
		warp_dbg(WARP_DBG_ERR, "%s(): dma map for bus fail!\n", __func__);
		goto err;
	}
#else
	dma_cb->pkt_va = dma_alloc_coherent(&bus->pdev->dev, dma_cb->pkt_size, &dma_cb->pkt_pa, GFP_KERNEL);

	if (unlikely(!dma_cb->pkt_va)) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate bus pkt\n", __func__);
		goto err;
	} else
		dma_cb->pkt = (struct sk_buff *)dma_cb->pkt_va;
#endif

	dma_cb->alloc_size = sizeof(struct bus_dmad);
	dma_cb->alloc_va = desc->alloc_va + (idx * dma_cb->alloc_size);
	dma_cb->alloc_pa = desc->alloc_pa + (idx * dma_cb->alloc_size);
	rxd = (struct bus_dmad *) dma_cb->alloc_va;
	/* txd, ddone set to 0 */
	ctrl = (bus->rx_ring.pkt_len << WED_CTL_SD_LEN0_SHIFT) & WED_CTL_SD_LEN0;
	ctrl |= WED_CTL_LAST_SEC0;
	WRITE_ONCE(rxd->sdp0, cpu_to_le32(dma_cb->pkt_pa));
	WRITE_ONCE(rxd->ctrl, cpu_to_le32(ctrl));
	return 0;

err:
	return -1;
}


static void rx_dma_cb_exit(
	struct woif_bus *bus,
	struct warp_dma_cb *dma_cb)
{
	if (dma_cb && bus) {
		if (dma_cb->pkt_pa && dma_cb->pkt) {
#ifdef WED_MEM_LEAK_DBG
			pkt_free_mem((void *)dma_cb->pkt);
#endif /* WED_MEM_LEAK_DBG */
#if CCIF_PAGE_ALLOC
			dma_unmap_single(&bus->pdev->dev, dma_cb->pkt_pa, dma_cb->pkt_size, DMA_FROM_DEVICE);
			page_frag_free(dma_cb->pkt);
#else
			dma_free_coherent(&bus->pdev->dev, dma_cb->alloc_size, dma_cb->pkt_va, dma_cb->pkt_pa);
#endif
		}
		dma_cb->pkt = NULL;
		memset(dma_cb, 0 , sizeof(struct warp_dma_cb));
	} else {
		warp_dbg(WARP_DBG_ERR, "%s(): invalid pointer(dma_cb:0x%p, bus:0x%p)\n",
			 __func__, dma_cb, bus);
	}
}


static int rx_ring_init(struct woif_bus *bus, struct wo_ring_ctrl *rx_ctrl)
{
	struct warp_bus_ring *rx_ring = &rx_ctrl->ring;
	struct warp_dma_buf *desc = &rx_ring->desc;
	int rxd_len = sizeof(struct bus_dmad);
	int ring_len = WED_BUS_RING_SIZE;
	int len = rxd_len * ring_len;
	int i;

	spin_lock_init(&rx_ctrl->lock);
	rx_ctrl->pkt_len = CMD_BUFFER_LEN;
	rx_ctrl->ring_len = ring_len;

	memset(desc, 0, sizeof(struct warp_dma_buf));
	if (warp_dma_buf_alloc(bus->pdev, desc, len) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate desc fail, len = %d\n", __func__, len);
		return -1;
	}

	for (i = 0 ; i < ring_len; i++) {
		rx_dma_cb_init(bus, desc, i, &rx_ring->cell[i]);
	}
	return 0;
}


static void rx_ring_exit(
	struct woif_bus *bus,
	struct wo_ring_ctrl *rx_ctrl)
{
	u32 i;
	struct warp_bus_ring *rx_ring = &rx_ctrl->ring;
	int ring_len = rx_ctrl->ring_len;

	for (i = 0 ; i < ring_len ; i++) {
		rx_dma_cb_exit(bus, &rx_ring->cell[i]);
	}
	warp_dma_buf_free(bus->pdev, &rx_ring->desc);
}


static void bus_ring_init(struct woif_bus *bus)
{
	tx_ring_init(bus, &bus->tx_ring);
	rx_ring_init(bus, &bus->rx_ring);
}


static void bus_ring_exit(struct woif_bus *bus)
{
	struct page *page;

	tx_ring_exit(bus, &bus->tx_ring);
	rx_ring_exit(bus, &bus->rx_ring);

	if (bus->woif_tx_page.va) {
		page = virt_to_page(bus->woif_tx_page.va);
#if (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
		__page_frag_cache_drain(page, bus->woif_tx_page.pagecnt_bias);
#else
		atomic_sub(bus->woif_tx_page.pagecnt_bias - 1, &page->_count);
		__free_pages(page, compound_order(page));
#endif
		memset(&bus->woif_tx_page, 0, sizeof(bus->woif_tx_page));
	}
	if (bus->woif_rx_page.va) {
		page = virt_to_page(bus->woif_rx_page.va);
#if (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
		__page_frag_cache_drain(page, bus->woif_rx_page.pagecnt_bias);
#else
		atomic_sub(bus->virt_to_page.pagecnt_bias - 1, &page->_count);
		__free_pages(page, compound_order(page));
#endif
		memset(&bus->woif_rx_page, 0, sizeof(bus->woif_rx_page));
	}

}


/*
*
*/
static void bus_rx_task(unsigned long data)
{
	struct woif_entry *woif = (struct woif_entry *)data;
	struct warp_entry *warp = NULL;
	struct woif_bus *bus = &woif->bus;
	struct wo_ring_ctrl *rx_ctrl = &bus->rx_ring;
	struct warp_bus_ring *rx_ring = &rx_ctrl->ring;
	struct bus_dmad *rxd;
	struct warp_dma_cb *dma_cb;
	u8 *msg;
	u32 msg_len;
	u32 didx, cidx;
	u32 ctrl;
	bool reschedule = false;

	if (!bus->hw) {
		warp_dbg(WARP_DBG_INF, "%s(): hw unregister do nothing!\n", __func__);
		goto end;
	}

	if(bus->check_excpetion(bus) == true) {
		warp_dbg(WARP_DBG_ERR, "%s(): Error: wo excpetion\n", __func__);

		dma_unmap_single(&bus->pdev->dev, woif->wo_exp_ctrl.phys,
			woif->wo_exp_ctrl.log_size, DMA_FROM_DEVICE);
		if (woif->wo_exp_ctrl.log != NULL) {
			warp_dbg(WARP_DBG_ERR, "%s(): wo dump\n%s\n", __func__, (u8*) woif->wo_exp_ctrl.log);
		}
#if defined(CONFIG_MTK_AEE_FEATURE)
		aee_kernel_exception("wed_wo", "wed_wo exception happen\nDetail in SYS_WO_DUMP file\nCRDISPATCH_KEY:WED/WOCPU\n");
#endif

		/* notify wifi driver to do wifi reset */
		warp = warp_entry_get_by_idx(woif->idx);
		if (warp == NULL)
			return;

		if (warp->pdev) {
			warp->woif.wo_ctrl.cur_state = WO_STATE_WF_RESET;
			wifi_chip_reset(&warp->wifi);
			bus->clear_int(bus);
		}
		return;
	}

	spin_lock(&rx_ctrl->lock);

	warp_woif_bus_get_rx_res(bus, &didx, &cidx);

	while(didx != cidx) {
		cidx = (cidx + 1) % rx_ctrl->ring_len;
		dma_cb = &rx_ring->cell[cidx];
		rxd = (struct bus_dmad *) dma_cb->alloc_va;

		/* check cidx ddone is set or not,if ddone is 1 update cidx, if 0 reschedule. */
		if ((rxd->ctrl & WED_CTL_DMA_DONE) != 0) {
			ctrl = rxd->ctrl;

			/*unmap dma for coherence*/
			dma_unmap_single(&bus->pdev->dev, dma_cb->pkt_pa, dma_cb->pkt_size, DMA_FROM_DEVICE);
			msg = (u8 *) rx_ring->cell[cidx].pkt_va;
			msg_len = (ctrl & WED_CTL_SD_LEN0) >> WED_CTL_SD_LEN0_SHIFT;
			warp_cmd_process_wo_ring(woif->idx, msg, msg_len);

			/*clear ddone and description*/
			WRITE_ONCE(rxd->ctrl, 0);

			/* set cidx to wo/ccif*/
			warp_woif_bus_set_rx_res(bus, cidx);
			/* update dma_idx */
			warp_woif_bus_get_rx_res(bus, &didx, &cidx);

			bus->post_rx(bus);

			rx_ctrl->dbg_ddone_check = 0;
		} else if ((rxd->ctrl & WED_CTL_DMA_DONE) == 0) {
			warp_dbg(WARP_DBG_INF, "%s(): ddone is not done! rxd->ctrl:0x%x\n",
				 __func__, rxd->ctrl);
			reschedule = true;
			rx_ctrl->dbg_ddone_check++;

			if (rx_ctrl->dbg_ddone_check < 10) {
				/* do reschedule */
				goto end;
			} else {
				u8 sendmsg_buf[128] = {0};
				struct warp_msg_cmd cmd = {0};
				u8 *addr;
				u32 size;

				warp = warp_entry_get_by_idx(woif->idx);
				if (warp == NULL) {
					goto end;
				}

				cmd.param.cmd_id = WO_CMD_CCIF_RING_DUMP;
				cmd.msg = sendmsg_buf;
				cmd.param.to_id = MODULE_ID_WO;
				cmd.param.wait_type = WARP_MSG_WAIT_TYPE_NONE;
				cmd.param.timeout = 3000;
				cmd.param.rsp_hdlr = NULL;

				warp_msg_send_cmd(warp->idx , &cmd);
				warp_dbg(WARP_DBG_OFF,
						 "(%s) Host keep waiting cidx:%d ddone for 10 times! Acquire WO dump CCIF ring!\n",
						 __func__, cidx);
				warp_dbg(WARP_DBG_ERR, "%s(): didx:0x%x cidx:0x%xx\n", __func__, didx, cidx);
				warp_dbg(WARP_DBG_ERR, "=========cidx %d CCIF_RING=========\n", cidx);
				warp_dump_dmacb(&rx_ring->cell[cidx]);
				addr = (unsigned char *) rx_ring->cell[cidx].alloc_va;
				size = rx_ring->cell[cidx].alloc_size;
				warp_dump_raw("CCIF_RING", addr, size);
				warp_dbg(WARP_DBG_ERR, "=========cidx %d CCIF_RING=========\n", cidx);
				warp_dbg(WARP_DBG_ERR, "=========didx %d CCIF_RING=========\n", didx);
				warp_dump_dmacb(&rx_ring->cell[didx]);
				addr = (unsigned char *) rx_ring->cell[didx].alloc_va;
				size = rx_ring->cell[didx].alloc_size;
				warp_dump_raw("CCIF_RING", addr, size);
				warp_dbg(WARP_DBG_ERR, "=========didx %d CCIF_RING=========\n", didx);

				rx_ctrl->dbg_ddone_check = 0;
			}
			} else {
				warp_dbg(WARP_DBG_ERR, "wrong ddone bit 0x%x: %lu", cidx,
					 (rxd->ctrl & WED_CTL_DMA_DONE));
			}
		}

	if (didx != cidx) {
		warp_dbg(WARP_DBG_INF, "%s(): didx is updated, so re-schedule, dma_idx:%d->%d\n",
			__func__, cidx, didx);
		reschedule = true;
	}

end:
	spin_unlock(&rx_ctrl->lock);
	if (reschedule)
		tasklet_hi_schedule(&bus->rx_task);
}

/*
*
*/
static irqreturn_t woif_bus_isr(int irq, void *data)
{
	struct woif_bus *bus = (struct woif_bus *)data;

	warp_dbg(WARP_DBG_LOU, "%s(): receive msg from interrupt, irq:%d!\n", __func__, irq);

	bus->pre_rx(bus);


	tasklet_hi_schedule(&bus->rx_task);
	return IRQ_HANDLED;
}

static void woif_bus_init(struct woif_entry *woif,
	struct platform_device *pdev, unsigned long trig_flag)
{
	struct woif_bus *bus = &woif->bus;

	ccif_bus_register(bus);
	bus->pdev = pdev;
	bus->init(&woif->bus, woif->idx, (void *) woif_bus_isr, trig_flag);
	bus_ring_init(bus);
	warp_woif_bus_init_hw(bus);
	tasklet_init(&bus->rx_task, bus_rx_task, (unsigned long)woif);
}

static void woif_bus_exit(struct woif_entry *woif)
{
	struct woif_bus *bus = &woif->bus;

	if (bus->exit) {
		bus->exit(bus);
	}
	tasklet_kill(&bus->rx_task);
	bus_ring_exit(bus);
}

int woif_bus_tx(u8 idx, u8 *msg, u32 msg_len)
{
	struct warp_entry *warp = warp_entry_get_by_idx(idx);
	struct woif_bus *bus = &warp->woif.bus;
	struct wo_ring_ctrl *tx_ctrl = &bus->tx_ring;
	struct warp_bus_ring *tx_ring  = &tx_ctrl->ring;
	struct warp_dma_cb *dma_cb;
	struct bus_dmad *txd;
	u32 ctrl = 0;
	int res_idx;
	int ret = 0;

	spin_lock(&tx_ctrl->lock);
	res_idx = warp_woif_bus_get_tx_res(bus);
	if (res_idx < 0) {
		warp_dbg(WARP_DBG_INF, "%s(): no hw resouce for tx!\n", __func__);
		ret = -1;
		goto err;
	}

	dma_cb = &tx_ring->cell[res_idx];
	txd = (struct bus_dmad *)dma_cb->alloc_va;

	if (txd->ctrl & WED_CTL_DMA_DONE) {
		warp_dbg(WARP_DBG_INF, "%s(): ddone is set error, should be zero! ctrl: %x\n", __func__, txd->ctrl);
	}
	/* payload,
	 * Enable DMA access for cpu only at first, then do memcpy and enable
	 * DMA access for device at last
	 */
	dma_sync_single_for_cpu(&bus->pdev->dev, dma_cb->pkt_pa, dma_cb->pkt_size, DMA_TO_DEVICE);
	memcpy(dma_cb->pkt_va, msg, msg_len);
	dma_sync_single_for_device(&bus->pdev->dev, dma_cb->pkt_pa, dma_cb->pkt_size, DMA_TO_DEVICE);
	/*txd, ddone set to 0*/
	ctrl = (msg_len << WED_CTL_SD_LEN0_SHIFT) & WED_CTL_SD_LEN0;
	ctrl |= WED_CTL_LAST_SEC0;
	ctrl |= WED_CTL_DMA_DONE;
	WRITE_ONCE(txd->ctrl, cpu_to_le32(ctrl));
	/*kickout generic*/
	warp_woif_bus_kickout(bus, res_idx);
	/*bus specific*/
	bus->kickout(bus);
	warp_dbg(WARP_DBG_INF, "%s(): kick msg: idx is %d!\n", __func__, res_idx);
err:
	spin_unlock(&tx_ctrl->lock);
	return ret;
}

void wo_log_handle(char *msg, u16 msg_len, void *user_data)
{
	warp_dbg(WARP_DBG_OFF, " [WO LOG]: %s(): %s\n", __func__, msg);
}

void wo_profiling_statistic(struct profiling_statistic_t *record, u32 value, u8 lvl_bound)
{
	u8 lvl = 0;

	for (lvl = 0 ; lvl < lvl_bound ; lvl++)
	{
		if (value < record[lvl].bound) {
			record[lvl].record++;
			break;
		}
	}
}

void wo_profiling_dump_clr(struct profiling_statistic_t *report, u8 count)
{
	u8 loop = 0;

	for (loop = 0 ; loop < count ; loop++) {
		if (loop == 0) {
			warp_dbg(WARP_DBG_OFF, "\033[1;34m\t\t < %u ticks: %u packets\033[m\n",
				 report[loop].bound, report[loop].record);
		} else if (loop == (count-1)) {
			warp_dbg(WARP_DBG_OFF, "\033[1;34m\t\t > %u ticks: %u packets\033[m\n",
				 report[loop-1].bound-1, report[loop].record);
		} else {
			warp_dbg(WARP_DBG_OFF, "\033[1;34m\t\t %u ~ %u ticks: %u packets\033[m\n",
				 report[loop-1].bound, report[loop].bound-1, report[loop].record);
		}

		report[loop].record = 0;
	}
}

void wo_profiling_handle(char *msg, u16 msg_len, void *user_data)
{
	u8 report_cnt = 0, loop = 0;
	struct latency_record_t *record = NULL;

	report_cnt = msg_len/(sizeof(struct latency_record_t));
	record = (struct latency_record_t *)msg;
	warp_dbg(WARP_DBG_LOU, "\033[1;34m\t[WO Profiling]: %s(): %d report arrived!\033[m\n", __func__, report_cnt);
	for (loop = 0 ; loop < report_cnt ; loop++) {
		wo_profiling_statistic(profiling_total, record[loop].total, ARRAY_SIZE(profiling_total));
		wo_profiling_statistic(profiling_mod, record[loop].mod, ARRAY_SIZE(profiling_mod));
		wo_profiling_statistic(profiling_rro, record[loop].rro, ARRAY_SIZE(profiling_rro));
		warp_dbg(WARP_DBG_LOU, "\033[1;34m\t\t SN:%u with latency: total=%u, rro:%u, mod:%u\033[m\n",
			 record[loop].sn, record[loop].total, record[loop].rro, record[loop].mod);
	}
}

void wo_profiling_report(void)
{
	warp_dbg(WARP_DBG_OFF, "\033[1;34m\t[WO Profiling]: wo handling profiling:\033[m\n");
	warp_dbg(WARP_DBG_OFF, "\033[1;34m\t\t\t total handling statistics:\033[m\n");
	wo_profiling_dump_clr(profiling_total, ARRAY_SIZE(profiling_total));
	warp_dbg(WARP_DBG_OFF, "\n\033[1;34m\t\t\t rro handling statistics:\033[m\n");
	wo_profiling_dump_clr(profiling_rro, ARRAY_SIZE(profiling_rro));
	warp_dbg(WARP_DBG_OFF, "\n\033[1;34m\t\t\t mod handling statistics:\033[m\n");
	wo_profiling_dump_clr(profiling_mod, ARRAY_SIZE(profiling_mod));
}

void wo_rxcnt_update_handle(char *msg, u16 msg_len, void *user_data)
{
	struct warp_entry *warp = (struct warp_entry *)user_data;
	struct wo_cmd_rxcnt_t *r_cnt = NULL;
	u32 count = 0;
	u8 report_idx = 0;

	if (msg == NULL) {
		warp_dbg(WARP_DBG_ERR, "%s(): msg is NULL\n", __func__);
		return;
	}

	count = *(u32 *)msg;
	r_cnt = (struct wo_cmd_rxcnt_t *)((u32 *)msg+1);
	warp_dbg(WARP_DBG_LOU, "%s(): report count:%d\n", __func__, count);

	for (report_idx = 0 ; report_idx < count ; report_idx++)
		wifi_chip_update_wo_rxcnt(&warp->wifi, (void *)&r_cnt[report_idx]);

}

void wo_log_event_register(struct warp_entry *warp)
{
	warp_dbg(WARP_DBG_INF, "%s(): wo_log_event_register: idx is %d\n", __func__, warp->idx);
	register_wo_event_handler(warp->idx, WO_EVT_LOG_DUMP, wo_log_handle, warp);
	register_wo_event_handler(warp->idx, WO_EVT_PROFILING, wo_profiling_handle, warp);
	register_wo_event_handler(warp->idx, WO_EVT_RXCNT_INFO, wo_rxcnt_update_handle, warp);
}

static void wo_exception_init(struct woif_entry *woif)
{
	struct wo_exception_ctrl *exp_ctrl = &woif->wo_exp_ctrl;
	struct woif_bus *bus = &woif->bus;
	struct warp_msg_cmd cmd = {0};
	u8 sendmsg_buf[128] = {0};
	struct wo_cmd_query *query = (struct wo_cmd_query *)sendmsg_buf;

	//init dma buffer
	exp_ctrl->log_size = (32*1024*sizeof(char));
	warp_os_alloc_mem((unsigned char **)&exp_ctrl->log , exp_ctrl->log_size, GFP_ATOMIC);

	if (unlikely(!exp_ctrl->log)){
		warp_dbg(WARP_DBG_ERR, "%s(): alloc exp_ctrl->log error\n", __func__);
		return;
	}
	memset(exp_ctrl->log, 0, exp_ctrl->log_size);
	exp_ctrl->phys = dma_map_single(&bus->pdev->dev, exp_ctrl->log,
		exp_ctrl->log_size, DMA_FROM_DEVICE);
	if (unlikely(dma_mapping_error(&bus->pdev->dev, exp_ctrl->phys))) {
		warp_dbg(WARP_DBG_ERR, "%s(): dma map error\n", __func__);
		return;
	}

	//set hw
	cmd.param.cmd_id = WO_CMD_EXCEPTION_INIT;
	query->query_arg0 = (u32) exp_ctrl->phys;
	query->query_arg1 = (u32) exp_ctrl->log_size;
	cmd.msg = sendmsg_buf;
	cmd.msg_len = sizeof(struct wo_cmd_query);
	cmd.param.to_id = MODULE_ID_WO;
	cmd.param.wait_type = WARP_MSG_WAIT_TYPE_NONE;
	cmd.param.timeout = 3000;
	cmd.param.rsp_hdlr = NULL;
	warp_msg_send_cmd(woif->idx , &cmd);
	warp_dbg(WARP_DBG_INF, "%s(%d): exp log= 0x%p, phy_addr= %pad size= %d\n", __func__,
		woif->idx, exp_ctrl->log, &exp_ctrl->phys, exp_ctrl->log_size);
}

static void wo_exception_exit(struct woif_entry *woif)
{
	struct wo_exception_ctrl *exp_ctrl = &woif->wo_exp_ctrl;
	struct woif_bus *bus = &woif->bus;

	if (exp_ctrl->phys) {
		dma_unmap_single(&bus->pdev->dev, exp_ctrl->phys,
			exp_ctrl->log_size, DMA_FROM_DEVICE);
	}
	if(exp_ctrl->log) {
		warp_os_free_mem(exp_ctrl->log);
		exp_ctrl->log = NULL;
	}
	warp_dbg(WARP_DBG_OFF, "%s(%d): wo_exception_exit done\n", __func__, woif->idx);
}

int32_t woif_init(struct warp_entry *warp, struct warp_bus *bus)
{
	struct woif_entry *woif = &warp->woif;
	struct warp_fwdl_ctrl *ctrl = &woif->fwdl_ctrl;
	int ret = 0;

	woif->idx = warp->idx;

	/*initial AP side wo bus for communication before fwdl*/
	woif_bus_init(woif, warp->pdev, bus->trig_flag);
	/* wo fwdl */
	ret = warp_fwdl(warp);

	if (ret < 0) {
		ctrl->status = WO_FWDL_FAIL;
		return -1;
	} else if (ret == WARP_ALREADY_DONE_STATUS) {
		/* enable wo */
		warp_woctrl_enter_state(warp, WO_STATE_ENABLE);
	} else {
		/* init wo */
		warp_woctrl_init_state(warp);
	}

	wo_proc_init(warp);
	wo_exep_proc_init(warp);
	wo_log_event_register(warp);
	wo_exception_init(woif);

	return ret;
}

void woif_exit(struct woif_entry *woif)
{
	woif_bus_exit(woif);
	if (woif->fwdl_ctrl.status == WO_FWDL_FAIL) {
		warp_dbg(WARP_DBG_OFF, "%s WO FWDL fail:(status:%d):\n", __func__, WO_FWDL_FAIL);
	}
	else
		wo_exception_exit(woif);
}
