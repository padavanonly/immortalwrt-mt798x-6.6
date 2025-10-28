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

	Module Name: wo interface definitions
	warp_wo.h
*/

#ifndef _WARP_WO_H_
#define _WARP_WO_H_

#include "warp_utility.h"
#include "warp_fwdl.h"
#include "warp_woctrl.h"

struct warp_entry;
struct woif_entry;

struct wo_ops {
	void (*mid_mod_schedule)(int warp_idx);
};

struct wo_entry {
	struct wo_ops ops;
};

struct wo_ring_ctrl {
	struct warp_bus_ring ring;
	u32 ring_len;
	u32 pkt_len;
	spinlock_t lock;
	u8 dbg_ddone_check;
};

struct wo_exception_ctrl {
	void* log;
	int log_size;
	dma_addr_t phys;
};

struct woif_bus {
	void *hw;
	struct platform_device *pdev;
	struct wo_ring_ctrl tx_ring;
	struct wo_ring_ctrl rx_ring;
	struct tasklet_struct rx_task;
	int (*init) (struct woif_bus *bus, u8 idx, void *isr, unsigned long trig_flag);
	void (*exit) (struct woif_bus *bus);
	void (*kickout) (struct woif_bus *bus);
	int (*pre_rx) (struct woif_bus *bus);
	int (*post_rx) (struct woif_bus *bus);
	bool (*check_excpetion) (struct woif_bus *bus);
	void (*clear_int) (struct woif_bus *bus);
	struct page_frag_cache woif_tx_page;
	struct page_frag_cache woif_rx_page;
};

struct woif_entry {
	u8 idx;
	u32 prev_heartbeat;
	struct warp_fwdl_ctrl fwdl_ctrl;
	struct woif_bus bus;
	struct wo_entry *wo;
	struct warp_wo_ctrl wo_ctrl;
	struct wo_exception_ctrl wo_exp_ctrl;
};

#define bus_to_tx_ring(_bus) (&(_bus->tx_ring.ring))
#define bus_to_rx_ring(_bus) (&(_bus->rx_ring.ring))
#define bus_to_tx_ring_len(_bus) (_bus->tx_ring.ring_len)
#define bus_to_rx_ring_len(_bus) (_bus->rx_ring.ring_len)
#define bus_to_hw(_bus) (_bus->hw)

/*export function*/
int32_t woif_init(struct warp_entry *warp, struct warp_bus *bus);
void woif_exit(struct woif_entry *woif);
int woif_bus_tx(u8 idx, u8 *msg, u32 msg_len);
void wo_profiling_report(void);
void wo_rxcnt_update_handle(char *msg, u16 msg_len, void *user_data);

#define CCIF_PAGE_ALLOC 1

#endif /*_WARP_WO_H_*/
