/*
 * Copyright (c) [2020] MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ""AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MTK_WED_H
#define __MTK_WED_H

#include <linux/interrupt.h>
#include <bus.h>
#include <chips.h>
#include "mtk_pci_dma.h"

#define WED_INVAL -1
#define WED_IDX_ERR 0xff

/*TBD: need to change warp driver interface and take hw as input*/
/* then we can use hw to call hw_ops to replace direct call and*/
/* export symbol in here*/
extern void
fmac_hif_txd_init(u8 *fbuf, u32 pkt_pa, u32 tkid);
extern void
fmac_hif_txd_v1_init(u8 *fbuf, dma_addr_t pkt_pa, u32 tkid, u8 src);

enum warp_ver_subver {
	WARP_V1_0,
	WARP_V2_0,
	WARP_V2_1,
	WARP_V2_2,
	WARP_V2_3,
	WARP_V3_0,
	WARP_V3_1,
};

enum src {
	MASTER_SRC = 0,
	SLAVE_SRC = 1,

};

/** struct wed_trans - WED virtual bus structure
 *
 * This struct is used for WED bus tunneling
 *
 * @param trans: Point to real bus data structure
 * @param irq_name: IRQ name for WED IRQ
 * @param bus_type: Type of real bus
 * @param irq: Interrupt request idx for WED
 * @param enable: WED enable or not
 * @param io_ops: Point to real bus io_ops
 * @param dma_ops: Point to real bus dma_ops
 */
struct wed_trans {
	union {
		struct pci_trans trans;
	} u;
	char irq_name[MAX_IRQ_NAME_LEN];
	u32 bus_type;
	u32 irq;
	bool enable;
	u8 wed_ver;
	u8 wed_sub_ver;
	const struct mtk_bus_io_ops *io_ops;
	const struct mtk_bus_dma_ops *dma_ops;
	struct wifi_hw *wifi_hw;
};

struct wed_driver {
	struct pci_driver pdriver;
};

#define to_wed_trans(_bus) ((struct wed_trans *) _bus)
#define to_wed_trans_dp(_bus) (*((struct wed_trans **) _bus))

/* struct wed_hif_txd_ver
 *
 * This struct indicates WED can support the HIF TXD version
 *
 * @param warp_ver: WED version
 * @param warp_subver: WED subversion
 * @param hif_txd_ver: HIF TXD version supported
 */

struct wed_hif_txd_info {
	enum warp_ver_subver warp_ver_subver;
	u8 warp_ver;
	u8 warp_subver;
	u8 hif_txd_ver;
};

struct wed_hif_txd_info wed_hif_txd_info_list[] = {
	{ WARP_V1_0, 1, 0, HIF_TXD_V0_0 },
	{ WARP_V2_0, 2, 0, HIF_TXD_V0_0 },
	{ WARP_V2_1, 2, 1, HIF_TXD_V0_0 },
	{ WARP_V2_2, 2, 2, HIF_TXD_V0_0 },
	{ WARP_V2_3, 2, 3, HIF_TXD_V0_1 },
	{ WARP_V3_0, 3, 0, HIF_TXD_V2_1 },
	{ WARP_V3_1, 3, 1, HIF_TXD_V2_1 },
};

#endif

