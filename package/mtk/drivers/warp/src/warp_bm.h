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
	warp.h
*/

#ifndef _WARP_BM_H
#define _WARP_BM_H

#include "warp_wifi.h"
#include <linux/interrupt.h>

#define PARTIAL_TXDMAD_SDP0_L_MASK		0xFFFFFFFF
#define PARTIAL_TXDMAD_SDP0_L_SHIFT		0
#define PARTIAL_TXDMAD_SDP0_H_MASK		0x0000FFFF
#define PARTIAL_TXDMAD_SDP0_H_SHIFT		0
#define PARTIAL_TXDMAD_TOKEN_ID_MASK		0xFFFF0000
#define PARTIAL_TXDMAD_TOKEN_ID_SHIFT		16

struct warp_bm_txdmad {
	__le32 sdp0;
	__le32 token;
} __packed __aligned(4);

#define PARTIAL_RXDMAD_SDP0_L_MASK		0xFFFFFFFF
#define PARTIAL_RXDMAD_SDP0_L_SHIFT		0
#define PARTIAL_RXDMAD_SDP0_H_MASK		0x0000FFFF
#define PARTIAL_RXDMAD_SDP0_H_SHIFT		0
#define PARTIAL_RXDMAD_TOKEN_ID_MASK		0xFFFF0000
#define PARTIAL_RXDMAD_TOKEN_ID_SHIFT		16

struct warp_bm_rxdmad {
	/* Partial RXDMAD DW0 */
	__le32 sdp0;
	/* Partial RXDMAD DW1 */
	__le32 token;
} __packed __aligned(4);

struct warp_tkid_rxdmad {
	__le32 sdp0;
	__le32 sdp0_h	:15;
	__le32 source	:1;
	__le32 tkid	:16;
} __packed __aligned(4);

enum thrd_operation {
	THRD_INC_ALL = 0,
	THRD_DEC_ALL,
	THRD_INC_L,
	THRD_INC_H,
	THRD_DEC_L,
	THRD_DEC_H,
	THRD_OP_MAX
};

enum {
	WARP_DYBM_EINT_ALL,
	WARP_DYBM_EINT_BM_H,
	WARP_DYBM_EINT_BM_L,
	WARP_DYBM_EINT_TKID_H,
	WARP_DYBM_EINT_TKID_L,
	WARP_DYBM_EINT_RXBM_H,
	WARP_DYBM_EINT_RXBM_L,
	WARP_DYBM_EINT_RXBM_HL,
	WARP_DYBM_EINT_MAX
};

#define DYBM_RX_EXT_POLLING_CNT	(0)//(50) /*50 times of polling approximately cost 5us */

struct dybm_dl_tasks {
	struct tasklet_struct tbuf_alloc_task;
	struct tasklet_struct tbuf_free_task;
	struct tasklet_struct tkn_alloc_task;
	struct tasklet_struct tbudge_refill_task;
	struct tasklet_struct tbudge_release_task;
};

struct dybm_ul_tasks {
	struct tasklet_struct rbudge_release_task;
	struct tasklet_struct rbudge_refill_task;
	struct tasklet_struct rbuf_alloc_task;
	struct tasklet_struct rbuf_free_task;
};

int regist_dl_dybm_task(struct wed_entry *wed);
int unregist_dl_dybm_task(struct wed_entry *wed);
int regist_ul_dybm_task(struct wed_entry *wed);
int unregist_ul_dybm_task(struct wed_entry *wed);

void buf_free_task(unsigned long data);
void buf_alloc_task(unsigned long data);

void dump_pkt_info(struct wed_buf_res *res);

int wed_txbm_init(struct wed_entry *wed, struct wifi_hw *hw);
void wed_txbm_exit(struct wed_entry *wed);

int dybm_dl_int_disp(struct wed_entry *wed, u32 status);

void rxbm_recycle_handler(unsigned long data);
void rxbm_alloc_handler(unsigned long data);

int wed_rx_bm_init(struct wed_entry *wed, struct wifi_hw *hw);
void wed_rx_bm_exit(struct wed_entry *wed);
int wed_rx_page_bm_init(struct wed_entry *wed, struct wifi_hw *hw);
void *wed_page_hash_search(struct wed_entry *wed, dma_addr_t pa);

void wed_rx_page_bm_exit(struct wed_entry *wed);

int dybm_ul_int_disp(struct wed_entry *wed, u32 status);

#endif /*_WARP_BM_H*/
