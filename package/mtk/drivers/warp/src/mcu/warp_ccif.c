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

	Module Name: warp_ccif
	warp_ccif.c
*/

#include "warp_utility.h"
#include "warp.h"
#include "warp_wo.h"
#include "warp_ccif.h"
#include "warp_meminfo_list.h"

#define AP2WOCCIF_DEV_NODE "mediatek,ap2woccif"

extern void warp_ccif_tx_ring_get_hw(void *hw, struct warp_bus_ring *ring);
extern void warp_ccif_rx_ring_get_hw(void *hw, struct warp_bus_ring *ring);
extern void warp_ccif_kickout(void *hw);
extern void warp_ccif_isr_ctrl(void *hw, bool enable);
extern void warp_ccif_set_ack(void *hw);
extern u32 warp_ccif_get_rxchnum(void *hw);
extern void warp_ccif_clear_int(void *hw);



#define AP2WO_TXCH_NUM 0
#define AP2WO_RXCH_NUM 1
#define AP2WO_RXCH_NUM_WO_EXCEPTION 7

static struct ccif_ring_ctrl ccif_tx_ring = {
	.chnum = AP2WO_TXCH_NUM,
	.base_addr_dnum = 0,
	.cnt_dnum = 1,
	.cidx_dnum = 2,
	.didix_dnum = 3,
	.irq_mask = 0,
};

static struct ccif_ring_ctrl ccif_rx_ring = {
	.chnum = AP2WO_RXCH_NUM,
	.base_addr_dnum = 4,
	.cnt_dnum = 5,
	.cidx_dnum = 6,
	.didix_dnum = 7,
	.irq_mask = (1 << AP2WO_RXCH_NUM | (1 << AP2WO_RXCH_NUM_WO_EXCEPTION)),
};


/*
*
*/
static int ccif_init(struct woif_bus *bus, u8 idx,
			void *isr, unsigned long trig_flag)
{
	struct device_node *node = NULL;
	struct ccif_entry *ccif = NULL;
	warp_os_alloc_mem((unsigned char **)&ccif, sizeof(struct ccif_entry), GFP_ATOMIC);

	if (ccif == NULL) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate ccif entry fail!\n", __func__);
		goto err;
	}
	memset(ccif, 0, sizeof(struct ccif_entry));
	bus->hw = (void*)ccif;
	ccif->tx_ring = &ccif_tx_ring;
	ccif->rx_ring = &ccif_rx_ring;

	node = of_find_compatible_node(NULL, NULL, AP2WOCCIF_DEV_NODE);
	if (!node) {
		warp_dbg(WARP_DBG_ERR, "%s(): ccif device tree not defined!\n", __func__);
		goto err;
	}

	ccif->base_addr = (unsigned long)of_iomap(node, idx);
	ccif->irq = irq_of_parse_and_map(node, idx);

	warp_dbg(WARP_DBG_INF, "%s(): ccif: %p, addr: %lx, irq:%d, idx:%d!\n",
		__func__, ccif, ccif->base_addr, ccif->irq, idx);

	/*initial virtual ring*/
	warp_ccif_tx_ring_get_hw(ccif, bus_to_tx_ring(bus));
	warp_ccif_rx_ring_get_hw(ccif, bus_to_rx_ring(bus));
	/*assign interrupt*/
	if (request_irq(ccif->irq, (irq_handler_t)isr,
		trig_flag, "ccif_wo_isr", bus)) {
		warp_dbg(WARP_DBG_ERR, "%s(): register irq: %d for ccif fail!\n",
			 __func__, ccif->irq);
	}

	warp_dbg(WARP_DBG_INF, "%s(): enable isr!\n", __func__);
	warp_ccif_isr_ctrl(ccif, true);

	return 0;
err:
	if (ccif) {
		warp_os_free_mem(ccif);
		bus->hw = NULL;
	}
	return -1;
}


/*
*
*/
static int ccif_pre_rx(struct woif_bus *bus)
{
	warp_ccif_isr_ctrl(bus->hw, false);
	return 0;
}


/*
*
*/
static int ccif_post_rx(struct woif_bus *bus)
{
	warp_ccif_set_ack(bus->hw);
	warp_ccif_isr_ctrl(bus->hw, true);
	return 0;
}

/*
*
*/
static void ccif_kickout(struct woif_bus *bus)
{
	warp_ccif_kickout(bus->hw);
}

/*
*
*/
static void ccif_exit(struct woif_bus *bus)
{
	struct ccif_entry *ccif = (struct ccif_entry *) bus->hw;

	if (ccif) {
		warp_ccif_isr_ctrl(ccif, false);
		disable_irq(ccif->irq);
		free_irq(ccif->irq, bus);
		if (ccif->base_addr)
			iounmap((void *)ccif->base_addr);
		warp_os_free_mem(ccif);
	}

	bus->hw = NULL;
}

static bool ccif_wo_exception(struct woif_bus *bus)
{
	u32 value;

	value =  warp_ccif_get_rxchnum(bus->hw);
	if( value  & (1 << AP2WO_RXCH_NUM_WO_EXCEPTION) ){
		warp_dbg(WARP_DBG_OFF, "%s(): wo excpetion happen %d\n", __func__, value);
		return true;
	}
	return false;
}

static void ccif_clear_int(struct woif_bus *bus)
{
	warp_ccif_clear_int(bus->hw);
}

/*
*
*/
void ccif_bus_register(struct woif_bus *bus)
{
	bus->init = ccif_init;
	bus->kickout = ccif_kickout;
	bus->exit = ccif_exit;
	bus->pre_rx = ccif_pre_rx;
	bus->post_rx = ccif_post_rx;
	bus->check_excpetion = ccif_wo_exception;
	bus->clear_int = ccif_clear_int;
}

