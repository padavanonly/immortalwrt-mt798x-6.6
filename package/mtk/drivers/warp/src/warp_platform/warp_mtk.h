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
	warp_mtk.h
*/

#ifndef _WARP_MTK_H
#define _WARP_MTK_H

#include <net/ra_nat.h>

extern int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no);
extern int (*hnat_set_wdma_pse_port_state)(int wdma_idx, int up);
extern int (*hnat_get_wdma_rx_port)(int wdma_idx);
extern int (*hnat_get_wdma_tx_port)(int wdma_idx);

/* global definition */
#define WED_DEV_NODE "mediatek,wed"
#ifdef CONFIG_WARP_NEW_DTS_FORMAT
#define WED0_DEV_NODE "mediatek,wed0"
#define WED1_DEV_NODE "mediatek,wed1"
#define WED2_DEV_NODE "mediatek,wed2"
#else
#define WED0_DEV_NODE "mediatek,wed"
#define WED1_DEV_NODE "mediatek,wed2"
#define WED2_DEV_NODE "mediatek,wed3"
#endif
#define WDMA_DEV_NODE "mediatek,wed-wdma"

/*default usage, should get from pcie device*/
#define WPDMA_BASE_ADDR0 0x20004000
#define WPDMA_BASE_ADDR1 0x20104000

#endif
