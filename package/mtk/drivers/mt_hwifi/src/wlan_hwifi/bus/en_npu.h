/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) [2023] Airoha Inc.
 */
#ifndef __EN_NPU_H
#define __EN_NPU_H
#include <linux/interrupt.h>
#include <bus.h>
#include <chips.h>
#include "mtk_pci_dma.h"
#include "mtk_wed.h"

#define NPU_SUPPORT_RRO 1
#define RRO_IN_NPU 1 //switch the control for RRO ring between NPU<->driver

union _2_hostapdt_vend_specific {
	struct {
		u32 more			: 1;
		u32 hw_rro			: 1;
		u32 ip_frag			: 1;
		u32 old_pkt			: 1;
		u32 drop			: 1;
		u32 len				: 12;
		u32 eth_hdr_ofst	: 7;
		u32 repeat_pkt		: 1;
		u32 wifi_frag		: 1;
		u32 pn_chk_fail		: 1;
		u32 rro_v31_token	: 1;
		u32 resv			: 4;
	} rx_info;
	u32 ctrl_word;
};
extern unsigned char glb_npu_en_node;
extern void (*hostdapt_registe_wifitask_hook)(unsigned int ringIdx, void *func);
extern struct sk_buff *(*fromHostadptPktHandle_hook)(unsigned int ringIdx,
	unsigned char *preschedule, unsigned int *vnd_spec);
extern void (*hostdapt_enable_int_hook)(unsigned int ringIdx);
extern void (*hostdapt_disable_int_hook)(unsigned int ringIdx);

/** struct npu_trans - NPU virtual bus structure
 *
 * This struct is used for NPU bus tunneling
 *
 * @param wed_trans: Point to wed bus data structure.
 * @param io_ops: Point to real bus io_ops
 * @param dma_ops: Point to real bus dma_ops
 */
struct npu_trans {
	union {
		struct wed_trans trans;
	} u;
	const struct mtk_bus_io_ops *io_ops;
	const struct mtk_bus_dma_ops *dma_ops;
};

#define to_npu_trans(_bus) ((struct npu_trans *) _bus)
#define to_npu_trans_dp(_bus) (*((struct npu_trans **) _bus))

#endif
