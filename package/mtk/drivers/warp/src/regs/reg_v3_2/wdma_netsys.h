/* SPDX-License-Identifier: <SPDX License Expression> */
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
	wdma_netsys.h
*/

#ifndef _WDMA_NETSYS_H
#define _WDMA_NETSYS_H

#include <warp.h>
#include <warp_hw.h>

int wdma_rx_ring_rdy_ck(struct wdma_entry *wdma, struct wdma_rx_ring_ctrl *ring_ctrl);
int wdma_tx_ring_init_hw(struct wdma_entry *wdma, struct wdma_tx_ring_ctrl *tx_ring_ctrl, int idx);
int wdma_rx_ring_init_hw(struct wdma_entry *wdma, struct wdma_rx_ring_ctrl *rx_ring_ctrl, int idx);
int wdma_dma_ctrl(struct wdma_entry *wdma, u8 txrx);
int wdma_int_ctrl(struct wdma_entry *wdma, u8 enable);
int reset_wdma_rx(struct wdma_entry *wdma);
int reset_wdma_tx(struct wdma_entry *wdma);
int stop_wdma_tx(struct wdma_entry *wdma);
int restore_wdma_tx(struct wdma_entry *wdma);
void dump_wdma_cfg_hw(struct wdma_entry *wdma, struct seq_file *seq);
void dump_wdma_rx_ring_hw(struct wdma_entry *wdma, struct seq_file *seq);
void dump_wdma_tx_ring_hw(struct wdma_entry *wdma, struct seq_file *seq);

#endif

