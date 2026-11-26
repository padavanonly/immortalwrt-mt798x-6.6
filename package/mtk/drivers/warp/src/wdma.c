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
	wdma.c
*/
#include "warp.h"
#include "wdma.h"
#include "warp_hw.h"
#include "warp_meminfo_list.h"


/*
*
*/
static void
dump_wdma_rx_ring(struct warp_ring *ring)
{
	warp_dbg(WARP_DBG_OFF, "hw_desc_base:\t%x\n", ring->hw_desc_base);
	warp_dbg(WARP_DBG_OFF, "hw_cnt_addr:\t%x\n", ring->hw_cnt_addr);
	warp_dbg(WARP_DBG_OFF, "hw_cidx_addr:\t%x\n", ring->hw_cidx_addr);
	warp_dbg(WARP_DBG_OFF, "hw_didx_addr:\t%x\n", ring->hw_didx_addr);
}
/*
*
*/
static void
dump_wdma_basic(struct wdma_entry *wdma)
{
	int i;
	struct wdma_rx_ring_ctrl *ring_ctrl = &wdma->res_ctrl.rx_ctrl.rx_ring_ctrl;

	warp_dbg(WARP_DBG_OFF, "base phy addr:\t%lx\n", wdma->base_phy_addr);
	warp_dbg(WARP_DBG_OFF, "base addr:\t%lx\n", wdma->base_addr);
	warp_dbg(WARP_DBG_OFF, "pdev:\t%p\n", wdma->pdev);
	warp_dbg(WARP_DBG_OFF, "proc:\t%p\n", wdma->proc);
	warp_dbg(WARP_DBG_OFF, "ring_num:\t%d\n", ring_ctrl->ring_num);
	warp_dbg(WARP_DBG_OFF, "ring_len:\t%d\n", ring_ctrl->ring_len);
	warp_dbg(WARP_DBG_OFF, "rxd_len:\t%d\n", ring_ctrl->rxd_len);

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		dump_wdma_rx_ring(&ring_ctrl->ring[i]);
		warp_dump_dmabuf(&ring_ctrl->desc[i]);
	}
}

/*
*
*/
static void
dump_rx_ring_raw(struct wdma_entry *wdma, u8 ring_id, u32 idx)
{
	struct warp_ring *ring = &wdma->res_ctrl.rx_ctrl.rx_ring_ctrl.ring[ring_id];
	u8 *addr;
	u32 size;
	/*WDMA Tx Ring content*/
	warp_dbg(WARP_DBG_OFF, "==========WDMA RX RING RAW (%d/0x%x)==========\n",
		 ring_id, idx);
	warp_dump_dmacb(&ring->cell[idx]);
	addr = (u8 *) ring->cell[idx].alloc_va;
	size = ring->cell[idx].alloc_size;
	warp_dump_raw("WDMA_RX_RING", addr, size);
}

/*
* assign tx description for tx ring entry
*/
static int
rx_dma_cb_init(struct wdma_entry *entry,
		   struct warp_dma_buf *desc,
	       u32 idx,
	       struct warp_dma_cb *dma_cb)
{
	dma_cb->pkt = NULL;

	/* Init Tx Ring Size, Va, Pa variables */
	if (entry->ver == 0x1)
		dma_cb->alloc_size = sizeof(struct WDMA_RXD);
	else
		dma_cb->alloc_size = sizeof(struct WDMA_RXD) * 2;

	dma_cb->alloc_va = desc->alloc_va + (idx * dma_cb->alloc_size);
	dma_cb->alloc_pa = desc->alloc_pa + (idx * dma_cb->alloc_size);
	return 0;
}

/*
* local function
*/
static void
rx_ring_exit(struct wdma_entry *entry, u8 idx,
	     struct wdma_rx_ring_ctrl *ring_ctrl)
{
	struct warp_dma_buf *desc = &ring_ctrl->desc[idx];

	warp_dma_buf_free(entry->pdev, desc);
}

/*
*
*/
static int
rx_ring_init(struct wdma_entry *entry, u8 idx,
	     struct wdma_rx_ring_ctrl *ring_ctrl)
{
	int i;
	unsigned int len = 0;
	struct warp_dma_buf *desc = &ring_ctrl->desc[idx];
	struct warp_ring *ring = &ring_ctrl->ring[idx];

	len = ring_ctrl->rxd_len * ring_ctrl->ring_len;
	warp_wdma_rx_ring_get_hw(ring, idx);
	if (warp_dma_buf_alloc(entry->pdev, desc, len) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate rx ring fail!\n", __func__);
		return -1;
	}

	for (i = 0; i < ring_ctrl->ring_len; i++)
		rx_dma_cb_init(entry, desc, i, &ring->cell[i]);

	return 0;
}

/*
*
*/
static void
wdma_rx_ring_exit(struct wdma_entry *wdma, struct wdma_rx_ring_ctrl *ring_ctrl)
{
	int i;

	for (i = 0; i < ring_ctrl->ring_num; i++)
		rx_ring_exit(wdma, i, ring_ctrl);

	if (ring_ctrl->ring)
		warp_os_free_mem(ring_ctrl->ring);

	if (ring_ctrl->desc)
		warp_os_free_mem(ring_ctrl->desc);

	memset(ring_ctrl, 0, sizeof(*ring_ctrl));
}

/*
*
*/
static int
wdma_rx_ring_init(struct wdma_entry *wdma, struct wdma_rx_ring_ctrl *ring_ctrl)
{
	unsigned int len;
	int i;

	len = sizeof(struct warp_dma_buf) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->desc , len, GFP_ATOMIC);
	if (!ring_ctrl->desc)
		goto err;

	memset(ring_ctrl->desc, 0, len);

	len = sizeof(struct warp_ring) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->ring , len, GFP_ATOMIC);
	if (!ring_ctrl->ring)
		goto err;

	memset(ring_ctrl->ring, 0, len);

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		if (rx_ring_init(wdma, i, ring_ctrl) < 0)
			goto err;
	}

	return 0;
err:
	warp_dbg(WARP_DBG_ERR, "%s(): rx ring init faild\n", __func__);
	wdma_rx_ring_exit(wdma, ring_ctrl);
	return -1;
}

#if 0	/* TBD, lack of caller */
static void
dump_wdma_tx_ring(struct warp_ring *ring)
{
	warp_dbg(WARP_DBG_OFF, "hw_desc_base:\t%x\n", ring->hw_desc_base);
	warp_dbg(WARP_DBG_OFF, "hw_cnt_addr:\t%x\n", ring->hw_cnt_addr);
	warp_dbg(WARP_DBG_OFF, "hw_cidx_addr:\t%x\n", ring->hw_cidx_addr);
	warp_dbg(WARP_DBG_OFF, "hw_didx_addr:\t%x\n", ring->hw_didx_addr);
}

static void
dump_tx_ring_raw(struct wdma_entry *wdma, u8 ring_id, u32 idx)
{
	struct warp_ring *ring = &wdma->res_ctrl.tx_ctrl.tx_ring_ctrl.ring[ring_id];
	u8 *addr;
	u32 size;
	/*WDMA Tx Ring content*/
	warp_dbg(WARP_DBG_OFF, "==========WDMA TX RING RAW (%d/0x%x)==========\n",
		 ring_id, idx);
	warp_dump_dmacb(&ring->cell[idx]);
	addr = (u8 *) ring->cell[idx].alloc_va;
	size = ring->cell[idx].alloc_size;
	warp_dump_raw("WDMA_TX_RING", addr, size);
}
#endif

#ifdef WED_HW_RX_SUPPORT
static int
tx_dma_cb_init(struct wdma_entry *entry,
		   struct warp_dma_buf *desc,
	       u32 idx,
	       struct warp_dma_cb *dma_cb)
{
	struct WDMA_TXD *txd;

	dma_cb->pkt = NULL;
	memset(dma_cb, 0, sizeof(struct warp_dma_cb));
	/* Init Tx Ring Size, Va, Pa variables */
	if (entry->ver == 0x1)
		dma_cb->alloc_size = sizeof(struct WDMA_TXD);
	else
		dma_cb->alloc_size = sizeof(struct WDMA_TXD) * 2;

	dma_cb->alloc_va = desc->alloc_va + (idx * dma_cb->alloc_size);
	dma_cb->alloc_pa = desc->alloc_pa + (idx * dma_cb->alloc_size);

	txd = (struct WDMA_TXD *)dma_cb->alloc_va;
	txd->ctrl = cpu_to_le32(WDMA_TXD0_CTL_DMA_DONE);
#ifdef CONFIG_WARP_V3
	txd->tx_info = cpu_to_le32(WDMA_TXD0_TX_INFO_DMA_DONE_DW1);
	/* 2nd TXD (As 128-bit size per TXD) */
	txd += 1;
	txd->ctrl = cpu_to_le32(WDMA_TXD1_CTL_DMA_DONE_DW2);
	txd->tx_info = cpu_to_le32(WDMA_TXD1_TX_INFO_DMA_DONE2);
#endif

	return 0;
}

static void
tx_ring_exit(struct wdma_entry *entry, u8 idx,
	     struct wdma_tx_ring_ctrl *ring_ctrl)
{
	struct warp_dma_buf *desc = &ring_ctrl->desc[idx];

	warp_dma_buf_free(entry->pdev, desc);

}

static int
tx_ring_init(struct wdma_entry *entry, u8 idx,
	     struct wdma_tx_ring_ctrl *ring_ctrl)
{
	int i;
	unsigned int len = 0;
	struct warp_dma_buf *desc = &ring_ctrl->desc[idx];
	struct warp_ring *ring = &ring_ctrl->ring[idx];

	len = ring_ctrl->txd_len * ring_ctrl->ring_len;
	warp_wdma_tx_ring_get_hw(ring, idx);
	if (warp_dma_buf_alloc(entry->pdev, desc, len) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate rx ring fail!\n", __func__);
		return -1;
	}

	for (i = 0; i < ring_ctrl->ring_len; i++)
		tx_dma_cb_init(entry, desc, i, &ring->cell[i]);

	return 0;
}

static int
tx_ring_reset(struct wdma_entry *entry, u32 idx,
	struct wdma_tx_ring_ctrl *ring_ctrl)
{
	u32 i = 0;
	struct warp_dma_buf *desc = &ring_ctrl->desc[idx];
	struct warp_ring *ring = &ring_ctrl->ring[idx];

	for (i = 0; i < ring_ctrl->ring_len; i++)
		tx_dma_cb_init(entry, desc, i, &ring->cell[i]);

	return 0;
}

int
wdma_tx_ring_reset(struct wdma_entry *entry)
{
	struct wdma_tx_ring_ctrl *ring_ctrl = &entry->res_ctrl.tx_ctrl.tx_ring_ctrl;
	u32 i = 0;

	for (i = 0; i < ring_ctrl->ring_num; i++)
		tx_ring_reset(entry, i, ring_ctrl);

	return 0;

}


static void
wdma_tx_ring_exit(struct wdma_entry *wdma, struct wdma_tx_ring_ctrl *ring_ctrl)
{
	int i;

	for (i = 0; i < ring_ctrl->ring_num; i++)
		tx_ring_exit(wdma, i, ring_ctrl);

	if (ring_ctrl->ring)
		warp_os_free_mem(ring_ctrl->ring);

	if (ring_ctrl->desc)
		warp_os_free_mem(ring_ctrl->desc);

	memset(ring_ctrl, 0, sizeof(*ring_ctrl));
}

static int
wdma_tx_ring_init(struct wdma_entry *wdma, struct wdma_tx_ring_ctrl *ring_ctrl)
{
	unsigned int len;
	int i;

	len = sizeof(struct warp_dma_buf) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->desc , len, GFP_ATOMIC);
	if (!ring_ctrl->desc)
		goto err;

	memset(ring_ctrl->desc, 0, len);

	len = sizeof(struct warp_ring) * ring_ctrl->ring_num;
	warp_os_alloc_mem((unsigned char **)&ring_ctrl->ring , len, GFP_ATOMIC);
	if (!ring_ctrl->ring)
		goto err;

	memset(ring_ctrl->ring, 0, len);

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		if (tx_ring_init(wdma, i, ring_ctrl) < 0)
			goto err;
	}

	return 0;
err:
	warp_dbg(WARP_DBG_ERR, "%s(): tx ring init faild\n", __func__);
	wdma_tx_ring_exit(wdma, ring_ctrl);
	return -1;

}
#endif

int
wdma_ring_init(struct wdma_entry *wdma)
{
	struct wdma_res_ctrl *res = &wdma->res_ctrl;
	struct wdma_tx_ctrl *tx_ctrl = &res->tx_ctrl;
	struct wdma_rx_ctrl *rx_ctrl = &res->rx_ctrl;
	struct sw_conf_t *sw_conf = wdma->sw_conf;

	/* initial default value */
	if (wdma->ver >= 2) {
		rx_ctrl->rx_ring_ctrl.rxd_len = sizeof(struct WDMA_RXD) * 2;
		rx_ctrl->rx_ring_ctrl.ring_len = sw_conf->rx_wdma_ring_depth;
		tx_ctrl->tx_ring_ctrl.txd_len = sizeof(struct WDMA_TXD) * 2;
		tx_ctrl->tx_ring_ctrl.ring_len = sw_conf->tx_wdma_ring_depth;
	} else {
		rx_ctrl->rx_ring_ctrl.rxd_len = sizeof(struct WDMA_RXD);
		rx_ctrl->rx_ring_ctrl.ring_len = WDMA_TX_BM_RING_SIZE;
		tx_ctrl->tx_ring_ctrl.txd_len = sizeof(struct WDMA_TXD);
		tx_ctrl->tx_ring_ctrl.ring_len = WDMA_RX_BM_RING_SIZE;
	}
	rx_ctrl->rx_ring_ctrl.ring_num = WDMA_RX_RING_NUM;
	tx_ctrl->tx_ring_ctrl.ring_num = WDMA_TX_RING_NUM;

	warp_dbg(WARP_DBG_INF, "%s(): tx_ctrl %p, rx_ctrl %p\n",
	__func__, tx_ctrl, rx_ctrl);

#ifdef WED_HW_RX_SUPPORT
	wdma_tx_ring_init(wdma, &tx_ctrl->tx_ring_ctrl);
#endif

#ifdef WED_HW_TX_SUPPORT
	wdma_rx_ring_init(wdma, &rx_ctrl->rx_ring_ctrl);
#endif
	return 0;
}

void
wdma_ring_exit(struct wdma_entry *wdma)
{
	struct wdma_res_ctrl *res = &wdma->res_ctrl;
	struct wdma_tx_ctrl *tx_ctrl = &res->tx_ctrl;
	struct wdma_rx_ctrl *rx_ctrl = &res->rx_ctrl;

	warp_dbg(WARP_DBG_INF, "%s(): tx_ctrl %p, rx_ctrl %p\n",
	__func__, tx_ctrl, rx_ctrl);

#ifdef WED_HW_RX_SUPPORT
	wdma_tx_ring_exit(wdma, &tx_ctrl->tx_ring_ctrl);
#endif

#ifdef WED_HW_TX_SUPPORT
	wdma_rx_ring_exit(wdma, &rx_ctrl->rx_ring_ctrl);
#endif
}

/*
*
*/
int
wdma_init(struct platform_device *pdev, u8 idx, struct wdma_entry *wdma, u8 ver)
{
	struct device_node *node = NULL;
	struct resource res;
	int ret = 0;

	node = of_find_compatible_node(NULL, NULL, WDMA_DEV_NODE);
	ret = of_address_to_resource(node, idx, &res);
	if (ret)
		return -ENODEV;

	warp_dbg(WARP_DBG_INF, "%s(): get node=%p\n", __func__, &node);

	/* get wdma physical addr and ioremap addr */
	wdma->base_phy_addr = res.start;
	wdma->base_addr = (unsigned long)of_iomap(node, idx);
	wdma->pdev = pdev;
	warp_dbg(WARP_DBG_INF, "%s(): wdma(%d) base_addr=0x%lx, base_phy_addr=0x%lx\n",
		__func__, idx, wdma->base_addr, wdma->base_phy_addr);

	wdma->ver = ver;


	ret = warp_get_wdma_port(wdma, idx);
	if (ret) {
		warp_dbg(WARP_DBG_ERR,
			"%s(): Unexpected error %d from getting wdma port with idx %u\n",
			__func__, ret, idx);
		return ret;
	}

	/* initial wdma ring resource */
	wdma_ring_init(wdma);

	return 0;
}

/*
*
*/
int
wdma_exit(struct platform_device *pdev, struct wdma_entry *wdma)
{
	if (wdma) {
		/*remove ring & token buffer*/
		wdma_ring_exit(wdma);

		iounmap((void *)wdma->base_addr);
		memset(wdma, 0, sizeof(*wdma));
	}

	return 0;
}

/*
*
*/
void
wdma_proc_handle(struct wdma_entry *wdma, char choice, char *arg)
{
	unsigned int i;
	char *str;
	char *end;
	unsigned char idx;

	switch (choice) {
	case WDMA_PROC_BASIC:
		dump_wdma_basic(wdma);
		break;

	case WDMA_PROC_RX_CELL: {
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
		dump_rx_ring_raw(wdma, idx, i);
	}
	break;

	default:
		break;
	}
}
