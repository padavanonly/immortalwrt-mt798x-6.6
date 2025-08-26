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
	wdma.h
*/

#ifndef _WDMA_H_
#define _WDMA_H_

#include "warp_utility.h"

#ifdef CONFIG_WARP_V1
#define WDMA_TX_BM_RING_SIZE 2048
#else
#define WDMA_TX_BM_RING_SIZE 4095
#endif
#define WDMA_RX_BM_RING_SIZE 512
#define WDMA_RING_OFFSET 0x10
#define WDMA_TX_RING_NUM 1
#ifdef WED_WDMA_DUAL_RX_RING
#define WDMA_RX_RING_NUM 2
#else
#define WDMA_RX_RING_NUM 1
#endif
#define WDMA_PORT3 0x3
#define WDMA_PORT4 0x4
#define WDMA_PORT8 0x8
#define WDMA_PORT9 0x9
#define WDMA_PORT12 0xc
#define WDMA_PORT13 0xd

struct wdma_rx_ring_ctrl {
	u32 ring_num;
	u32 ring_len;
	u32 rxd_len;
	u32 int_sts_mask;
	struct warp_dma_buf *desc;
	struct warp_ring *ring;
};

struct wdma_rx_ctrl {
	struct wdma_rx_ring_ctrl rx_ring_ctrl;
	unsigned long base_phy_addr;
	u32 glo_cfg_offset;
	u32 int_sts_offset;
};

struct wdma_tx_ring_ctrl {
	u32 ring_num;
	u32 ring_len;
	u32 txd_len;
	u32 int_sts_mask;
	struct warp_dma_buf *desc;
	struct warp_ring *ring;
};

struct wdma_tx_ctrl {
	struct wdma_tx_ring_ctrl tx_ring_ctrl;
	unsigned long base_phy_addr;
	u32 glo_cfg_offset;
	u32 int_sts_offset;
};

struct wdma_res_ctrl {
	struct wdma_tx_ctrl tx_ctrl;
	struct wdma_rx_ctrl rx_ctrl;
};

struct wdma_entry {
	struct platform_device *pdev;
	unsigned long base_phy_addr;
	unsigned long base_addr;
	struct wdma_res_ctrl res_ctrl;
	struct sw_conf_t *sw_conf;
	void *proc;
	void *warp;
	u8 ver;
	u32 wdma_tx_port;
	u32 wdma_rx_port;
};

#define WDMA_CTL_RSV			GENMASK(15, 0)
#define WDMA_CTL_SD_LEN0		GENMASK(29, 16)
#define WDMA_CTL_LAST_SEC0		BIT(30)
#define WDMA_CTL_DMA_DONE		BIT(31)
#define WDMA_TXD0_CTL_DMA_DONE		BIT(31)
#define WDMA_TXD0_TX_INFO_DMA_DONE_DW1	BIT(29)
#define WDMA_TXD1_CTL_DMA_DONE_DW2	BIT(31)
#define WDMA_TXD1_TX_INFO_DMA_DONE2	BIT(31)

struct WDMA_TXD {
	__le32 sdp0;
	__le32 ctrl;
	__le32 sdp1;
	__le32 tx_info;
} __packed __aligned(4);

struct WDMA_RXD {
	__le32 sdp0;
	__le32 ctrl;
	__le32 tokenId;
	__le32 rx_info;
} __packed __aligned(4);

enum {
	WDMA_PROC_BASIC = 0,
	WDMA_PROC_RX_CELL = 1,
	WDMA_PROC_END
};

int wdma_init(struct platform_device *pdev, u8 idx, struct wdma_entry *wdma,
	      u8 ver);
int wdma_exit(struct platform_device *pdev, struct wdma_entry *wdma);
void wdma_dump_entry(struct wdma_entry *wdma);
int wdma_tx_ring_reset(struct wdma_entry *entry);
void wdma_proc_handle(struct wdma_entry *wdma, char choice, char *value);

#endif /*_WDMA_H_*/
