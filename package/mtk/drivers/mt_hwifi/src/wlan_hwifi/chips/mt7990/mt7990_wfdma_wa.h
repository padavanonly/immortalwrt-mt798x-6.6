/*
* Copyright (c) [2021] MediaTek Inc.
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

#ifndef __MTK_CHIP_MT7990_WFDMA_WA_H
#define __MTK_CHIP_MT7990_WFDMA_WA_H

#include "bus/mtk_pci_dma.h"
#include "wf_wfdma_host_dma0.h"
#include "wf_wfdma_host_dma0_pcie1.h"
#include "wf_wfdma_mcu_dma0.h"
#include "wf_wfdma_ext_wrap_csr.h"
#include "conn_host_csr_top.h"
#include "wf_rro_top.h"

#define TXD_SIZE 16
#define RXD_SIZE 16

#define RX_EVENT_BUF_SIZE 2048
#define RX_BUF_SIZE 1792

#define RX_BUF_SIZE_DATA RX_BUF_SIZE
#define RX_PROC_FIFO_SIZE	4096

enum mt7990_irq_mask {
	IRQ_TX_RING16 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_16_MASK,
	IRQ_TX_RING17 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_17_MASK,
	IRQ_TX_RING18 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_18_MASK,
	IRQ_TX_RING19 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_19_MASK,
	IRQ_TX_RING20 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_20_MASK,
	IRQ_RX_RING0 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_0_MASK,
	IRQ_RX_RING1 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_1_MASK,
	IRQ_RX_RING2 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_2_MASK,
	IRQ_RX_RING3 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_3_MASK,
	IRQ_RX_RING4 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_4_MASK,
	IRQ_RX_RING5 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_5_MASK,
	IRQ_RX_RING6 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_6_MASK,
	IRQ_RX_RING7 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_7_MASK,
	IRQ_RX_RING8 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_8_MASK,
	IRQ_RX_RING9 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_9_MASK,
	IRQ_RX_RING10 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_10_MASK,
	IRQ_RX_RING11 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_11_MASK,
	IRQ_RX_RING12 = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_12_MASK,
	IRQ_LUMP_ALL = (
		IRQ_TX_RING16 | IRQ_TX_RING17 | IRQ_TX_RING18 | IRQ_TX_RING19 |
		IRQ_TX_RING20 | IRQ_RX_RING0 | IRQ_RX_RING1 | IRQ_RX_RING2 |
		IRQ_RX_RING3 | IRQ_RX_RING4 | IRQ_RX_RING5 | IRQ_RX_RING6 |
		IRQ_RX_RING7 | IRQ_RX_RING8 | IRQ_RX_RING9 | IRQ_RX_RING10 |
		IRQ_RX_RING11 | IRQ_RX_RING12 |
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_wf_rro_irq_sts_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_coherent_int_sts_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_coherent_int_sts_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_mcu2host_sw_int_sts_MASK),
};

enum mt7991_irq_mask {
	IRQ_PCIE1_TX_RING21 = WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_tx_done_int_sts_21_MASK,
	IRQ_PCIE1_TX_RING22 = WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_tx_done_int_sts_22_MASK,
	IRQ_PCIE1_RX_RING3 = WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_rx_done_int_sts_3_MASK,
	IRQ_PCIE1_RX_RING4 = WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_rx_done_int_sts_4_MASK,
	IRQ_PCIE1_RX_RING5 = WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_rx_done_int_sts_5_MASK,
	IRQ_PCIE1_RX_RING7 = WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_rx_done_int_sts_7_MASK,
	IRQ_PCIE1_RX_RING9 = WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_rx_done_int_sts_9_MASK,
	IRQ_PCIE1_LUMP_ALL = (
		IRQ_PCIE1_TX_RING21 | IRQ_PCIE1_TX_RING22 | IRQ_PCIE1_RX_RING3 |
		IRQ_PCIE1_RX_RING4 | IRQ_PCIE1_RX_RING5 | IRQ_PCIE1_RX_RING7 | IRQ_PCIE1_RX_RING9 |
		WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_tx_coherent_int_sts_MASK |
		WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_rx_coherent_int_sts_MASK |
		WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_mcu2host_sw_int_sts_MASK),
};

const struct pci_tx_queue_desc txq_wa_layout[] = {
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING16_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_16_MASK,
			.q_size = 768,
			.q_attr = Q_TX_FWDL,
			.desc_size = TXD_SIZE,
			.q_info = "FWDL"
		},
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING17_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_17_MASK,
			.q_size = 256,
			.q_attr = Q_TX_CMD_WM,
			.desc_size = TXD_SIZE,
			.q_info = "cmd to WM"
		},
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING18_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_18_MASK,
			.q_size = 2048,
			.q_attr = Q_TX_DATA,
			.band_idx_bmp = BIT(BAND0),
			.desc_size = TXD_SIZE,
			.q_info = "band0 TXD"
		},
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING19_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_19_MASK,
			.q_size = 2048,
			.q_attr = Q_TX_DATA,
			.band_idx_bmp = BIT(BAND1),
			.desc_size = TXD_SIZE,
			.q_info = "band1 TXD"
		},
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING20_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_20_MASK,
			.q_size = 256,
			.q_attr = Q_TX_CMD,
			.desc_size = TXD_SIZE,
			.q_info = "cmd to WA"
		},
	},
#ifdef CONFIG_HWIFI_USE_PCIE0_TXRING21
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING21_CTRL0_ADDR,
			.hw_int_mask = 0x0,
			.q_size = 2048,
			.q_attr = Q_TX_DATA,
			.band_idx_bmp = BIT(BAND2),
			.desc_size = TXD_SIZE,
			.q_info = "band2 TXD"
		},
		.mp_attr = {
			.ring_no = TX_RING21,
			.handled_by = SLAVE_NODE,
		},
	}
#endif
};

const struct pci_rx_queue_desc rxq_wa_layout[] = {
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_0_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL3_DMA_IDX_MASK,
			.q_size = 512,
			.q_attr = Q_RX_EVENT_WM,
			.desc_size = RXD_SIZE,
			.q_info = "event from WM",
		},
		.rx_buf_size = RX_EVENT_BUF_SIZE,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING1_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_1_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING1_CTRL3_DMA_IDX_MASK,
			.q_size = 1024,
			.q_attr = Q_RX_EVENT_WA,
			.desc_size = RXD_SIZE,
			.q_info = "event from WA",
		},
		.rx_buf_size = RX_EVENT_BUF_SIZE,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_2_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_CTRL3_DMA_IDX_MASK,
			.q_size = 512,
			.q_attr = Q_RX_EVENT_TX_FREE_DONE,
			.desc_size = RXD_SIZE,
			.q_info = "TX free done from WA (src 0)",
		},
		.rx_buf_size = RX_BUF_SIZE,
		.free_done_handled_by = MASTER_HANDLER,
		.txfreedone_path = TXFREEDONE_FROM_WA,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING3_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_3_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING3_CTRL3_DMA_IDX_MASK,
			.q_size = 512,
			.q_attr = Q_RX_EVENT_TX_FREE_DONE,
			.desc_size = RXD_SIZE,
			.q_info = "TX free done from WA (src 1)",
		},
		.mp_attr = {
			.ring_no = RX_RING3,
			.handled_by = SLAVE_NODE,
		},
		.rx_buf_size = RX_BUF_SIZE,
		.free_done_handled_by = SLAVE_HANDLER,
		.txfreedone_path = TXFREEDONE_FROM_WA,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_4_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_CTRL3_DMA_IDX_MASK,
			.hw_attr_ena_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_CTRL1_ATTR_MASK,
			.q_size = 1536,
			.q_attr = Q_RX_DATA,
			.desc_size = RXD_SIZE,
			.band_idx_bmp = BIT(BAND0),
			.q_info = "band0/1 RX data (Legacy)"
		},
#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
		.rx_offload = {
			.kfifo_size = RX_PROC_FIFO_SIZE,
			.kfifo_enable = true,
		},
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */
		.rx_buf_size = RX_BUF_SIZE_DATA,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_5_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_CTRL3_DMA_IDX_MASK,
			.hw_attr_ena_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_CTRL1_ATTR_MASK,
			.q_size = 1536,
			.q_attr = Q_RX_DATA,
			.desc_size = RXD_SIZE,
			.band_idx_bmp = BIT(BAND2),
			.q_info = "band2 RX data (Legacy)"
		},
		.mp_attr = {
			.ring_no = RX_RING5,
			.handled_by = SHRINK_NODE,
		},
#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
		.rx_offload = {
			.kfifo_size = RX_PROC_FIFO_SIZE,
			.kfifo_enable = true,
		},
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */
		.rx_buf_size = RX_BUF_SIZE_DATA,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING6_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_6_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING6_CTRL3_DMA_IDX_MASK,
			.hw_magic_ena_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING6_CTRL1_MGC_ENA_MASK,
			.q_size = 1536,
			.q_attr = Q_RX_DATA_RRO,
			.desc_size = RXD_SIZE,
			.band_idx_bmp = BIT(BAND2),
			.q_info = "RRO Refill (band 2)"
		},
		.mp_attr = {
			.ring_no = RX_RING6,
			.handled_by = MASTER_NODE,
		},
		.rx_buf_size = RX_BUF_SIZE_DATA,
		.magic_enable = true,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING7_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_7_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING7_CTRL3_DMA_IDX_MASK,
			.q_size = 512,
			.q_attr = Q_RX_EVENT_TX_FREE_DONE,
			.desc_size = RXD_SIZE,
			.q_info = "TX free done from MAC (src 1)",
		},
		.mp_attr = {
			.ring_no = RX_RING7,
			.handled_by = DISABLE_NODE,
		},
		.rx_buf_size = RX_BUF_SIZE,
		.free_done_handled_by = SLAVE_HANDLER,
		.txfreedone_path = TXFREEDONE_FROM_MAC,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING8_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_8_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING8_CTRL3_DMA_IDX_MASK,
			.hw_magic_ena_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING8_CTRL1_MGC_ENA_MASK,
			.q_size = 1536,
			.q_attr = Q_RX_DATA_RRO,
			.desc_size = RXD_SIZE,
			.band_idx_bmp = BIT(BAND0),
			.q_info = "RRO Refill (band 0/1)"
		},
		.rx_buf_size = RX_BUF_SIZE_DATA,
		.magic_enable = true,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING9_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_9_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING9_CTRL3_DMA_IDX_MASK,
			.q_size = 512,
			.q_attr = Q_RX_EVENT_TX_FREE_DONE,
			.desc_size = RXD_SIZE,
			.q_info = "TX free done from MAC (src 0)",
		},
		.rx_buf_size = RX_BUF_SIZE,
		.free_done_handled_by = MASTER_HANDLER,
		.txfreedone_path = TXFREEDONE_FROM_MAC,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING10_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_10_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING10_CTRL3_DMA_IDX_MASK,
			.hw_magic_ena_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING10_CTRL1_MGC_ENA_MASK,
			.q_size = 1536,
			.q_attr = Q_RX_DATA_MSDU_PG,
			.desc_size = RXD_SIZE,
			.band_idx_bmp = BIT(BAND0),
			.q_info = "MSDU Page Ring (band 0)"
		},
		.rx_buf_size = 128,
		.magic_enable = true,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING11_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_11_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING11_CTRL3_DMA_IDX_MASK,
			.hw_magic_ena_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING11_CTRL1_MGC_ENA_MASK,
			.q_size = 1536,
			.q_attr = Q_RX_DATA_MSDU_PG,
			.desc_size = RXD_SIZE,
			.band_idx_bmp = BIT(BAND1),
			.q_info = "MSDU Page Ring (band 1)"
		},
		.rx_buf_size = 128,
		.magic_enable = true,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING12_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_12_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING12_CTRL3_DMA_IDX_MASK,
			.hw_magic_ena_mask = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING12_CTRL1_MGC_ENA_MASK,
			.q_size = 1536,
			.q_attr = Q_RX_DATA_MSDU_PG,
			.desc_size = RXD_SIZE,
			.band_idx_bmp = BIT(BAND2),
			.q_info = "MSDU Page Ring (band 2)"
		},
		.rx_buf_size = 128,
		.magic_enable = true,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_0_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL3_DMA_IDX_MASK,
			.q_size = 1536,
			.disable = true,
			.q_attr = Q_RX_DATA_WED,
			.desc_size = RXD_SIZE,
			.q_info = "WED Rx Data Ring"
		},
		.rx_buf_size = RX_BUF_SIZE_DATA,
	},
	/* indicate cmd */
	{
		.cmm = {
			.hw_desc_base = WF_RRO_TOP_IND_CMD_0_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_STA_wf_rro_irq_sts_MASK,
			.hw_didx_mask = WF_RRO_TOP_IND_CMD_0_CTRL3_DMA_IDX_MASK,
			.q_size = 1536,
			.q_attr = Q_RX_IND,
			.desc_size = 8, /* IND_CMD_SIZE */
			.band_idx_bmp = BIT(BAND0),
			.q_info = "IND CMD"
		},
		.rx_buf_size = RX_BUF_SIZE_DATA,
	}
};

struct pci_chip_profile mt7990_pci_wa_profile = {
	.int_mask_map = {
		/* map_idx 0: Legacy interrupt bitmap */
		[0] = {
			.int_sts_addr = WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR,
			.int_ena_addr = WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR,
		},
	},
	.int_ser_reg_addr = WF_WFDMA_MCU_DMA0_HOST2MCU_SW_INT_SET_ADDR,
	.pause_trxq_reg_addr = WF_WFDMA_HOST_DMA0_WPDMA_PAUSE0_ADDR,
	.pause_trxq_mask = (
		WF_WFDMA_HOST_DMA0_WPDMA_PAUSE0_RX_RING_PAUSE_ALL_MASK |
		WF_WFDMA_HOST_DMA0_WPDMA_PAUSE0_TX_RING_PAUSE_MASK),
	.lp_fw_own_reg_addr = CONN_HOST_CSR_TOP_WF_BAND0_LPCTL_WF_B0_AP_HOST_OWNER_STATE_SYNC_ADDR,
	/*mcu2host swi*/
	.int_enable_swi_mask =
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_mcu2host_sw_int_ena_MASK,
	.mcu2host_sw_int_ena_addr = WF_WFDMA_HOST_DMA0_MCU2HOST_SW_INT_ENA_ADDR,
	.mcu2host_sw_int_sta_addr = WF_WFDMA_HOST_DMA0_MCU2HOST_SW_INT_STA_ADDR,
	.mcu2host_sw_int_ser_mask = (
		WF_WFDMA_HOST_DMA0_MCU2HOST_SW_INT_STA_mcu2host_sw_int_2_MASK |
		WF_WFDMA_HOST_DMA0_MCU2HOST_SW_INT_STA_mcu2host_sw_int_3_MASK |
		WF_WFDMA_HOST_DMA0_MCU2HOST_SW_INT_STA_mcu2host_sw_int_4_MASK |
		WF_WFDMA_HOST_DMA0_MCU2HOST_SW_INT_STA_mcu2host_sw_int_5_MASK),
	.mcu2host_sw_wdt_ser_mask = (
		WF_WFDMA_HOST_DMA0_MCU2HOST_SW_INT_STA_mcu2host_wacpu_wdt_MASK |
		WF_WFDMA_HOST_DMA0_MCU2HOST_SW_INT_STA_mcu2host_wmcpu_wdt_MASK),
	.tx_dma_glo_reg_addr = WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR,
	.rx_dma_glo_reg_addr = WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR,
	.txq_num = ARRAY_SIZE(txq_wa_layout),
	.rxq_num = ARRAY_SIZE(rxq_wa_layout),
	.queue_layout = {
		.tx_queue_layout = txq_wa_layout,
		.rx_queue_layout = rxq_wa_layout,
	},
	.coherent_dma_addr_size = 32,
	.non_coherent_dma_addr_size = 36,
	/* rro related */
	.rro_profile = {
		.rro_mode = HW_RRO_V3_0_BUF_PG,
		.sign_base_0_addr = WF_RRO_TOP_IND_CMD_SIGNATURE_BASE_0_ADDR,
		.sign_base_1_addr = WF_RRO_TOP_IND_CMD_SIGNATURE_BASE_1_ADDR,
		.sign_base_1_en_shift = WF_RRO_TOP_IND_CMD_SIGNATURE_BASE_1_EN_SHFT,
		.debug_mode_rxd_addr = WF_RRO_TOP_HIF_RXD_DST_SEL_MAP_0_ADDR,
		.ack_sn_addr = WF_RRO_TOP_ACK_SN_CTRL_ADDR,
		.ack_sn_mask = WF_RRO_TOP_ACK_SN_CTRL_ACK_SN_MASK,
		.ack_sn_shift = WF_RRO_TOP_ACK_SN_CTRL_ACK_SN_SHFT,
		.ack_sn_seid_mask = WF_RRO_TOP_ACK_SN_CTRL_SESSION_ID_MASK,
		.ack_sn_seid_shift = WF_RRO_TOP_ACK_SN_CTRL_SESSION_ID_SHFT,
		.ack_sn_update_cnt = 4,
		.dbg_rd_ctrl_mask = WF_RRO_TOP_DBG_RD_CTRL_DBG_RD_EXEC_MASK,
		.dbg_rd_ctrl_addr = WF_RRO_TOP_DBG_RD_CTRL_ADDR,
		.dbg_rdat_dw0 = WF_RRO_TOP_DBG_RDAT_DW0_ADDR,
		.dbg_rdat_dw1 = WF_RRO_TOP_DBG_RDAT_DW1_ADDR,
		.dbg_rdat_dw2 = WF_RRO_TOP_DBG_RDAT_DW2_ADDR,
		.dbg_rdat_dw3 = WF_RRO_TOP_DBG_RDAT_DW3_ADDR,
		.max_rro_se_cnt = 1024,
		.addr_elem_cr_cnt = 128,
	}
};

const struct pci_tx_queue_desc txq_wa_layout_pcie1[] = {
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING21_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_tx_done_int_sts_21_MASK,
			.q_size = 2048,
			.q_attr = Q_TX_DATA,
			.desc_size = TXD_SIZE,
			.band_idx_bmp = BIT(BAND2),
			.q_info = "band2 TXD"
		},
		.mp_attr = {
			.ring_no = TX_RING21,
		},
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING22_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_tx_done_int_sts_22_MASK,
			.q_size = 2048,
			.q_attr = Q_TX_DATA,
			.desc_size = TXD_SIZE,
			.band_idx_bmp = 0,
			.q_info = "band? TXD (Unused)"
		}
	}
};

const struct pci_rx_queue_desc rxq_wa_layout_pcie1[] = {
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING3_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_rx_done_int_sts_3_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING3_CTRL3_DMA_IDX_MASK,
			.q_size = 512,
			.q_attr = Q_RX_EVENT_TX_FREE_DONE,
			.desc_size = RXD_SIZE,
			.q_info = "TX free done from WA (src 1)",
		},
		.mp_attr = {
			.ring_no = RX_RING3,
		},
		.rx_buf_size = RX_BUF_SIZE,
		.free_done_handled_by = SLAVE_HANDLER,
		.txfreedone_path = TXFREEDONE_FROM_WA,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING4_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_rx_done_int_sts_4_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING4_CTRL3_DMA_IDX_MASK,
			.hw_attr_ena_mask = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING4_CTRL1_ATTR_MASK,
			.q_size = 16,
			.q_attr = Q_RX_DATA,
			.desc_size = RXD_SIZE,
			.band_idx_bmp = 0,
			.q_info = "RX data (Unused)"
		},
		.rx_buf_size = RX_BUF_SIZE_DATA,
		.attr_enable = true,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING5_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_rx_done_int_sts_5_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING5_CTRL3_DMA_IDX_MASK,
			.hw_attr_ena_mask = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING5_CTRL1_ATTR_MASK,
			.q_size = 1536,
			.q_attr = Q_RX_DATA,
			.desc_size = RXD_SIZE,
			.band_idx_bmp = BIT(BAND2),
			.q_info = "band2 RX data (Legacy)"
		},
		.mp_attr = {
			.ring_no = RX_RING5,
		},
#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
		.rx_offload = {
			.kfifo_size = RX_PROC_FIFO_SIZE,
			.kfifo_enable = true,
		},
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */
		.rx_buf_size = RX_BUF_SIZE_DATA,
		.attr_enable = false,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING6_CTRL0_ADDR,
			.hw_int_mask = 0x0,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING6_CTRL3_DMA_IDX_MASK,
			.hw_magic_ena_mask = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING6_CTRL1_MGC_ENA_MASK,
			.q_size = 1536,
			.q_attr = Q_RX_DATA_RRO,
			.desc_size = RXD_SIZE,
			.band_idx_bmp = BIT(BAND2),
			.q_info = "RRO Refill (band 2)"
		},
		.mp_attr = {
			.ring_no = RX_RING6,
		},
		.rx_buf_size = RX_BUF_SIZE_DATA,
		.magic_enable = true,
	},
	{
		.cmm = {
			.hw_desc_base = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING7_CTRL0_ADDR,
			.hw_int_mask = WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_rx_done_int_sts_7_MASK,
			.hw_didx_mask = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING7_CTRL3_DMA_IDX_MASK,
			.q_size = 512,
			.q_attr = Q_RX_EVENT_TX_FREE_DONE,
			.desc_size = RXD_SIZE,
			.q_info = "TX free done from MAC (src 1)",
		},
		.mp_attr = {
			.ring_no = RX_RING7,
		},
		.rx_buf_size = RX_BUF_SIZE,
		.free_done_handled_by = SLAVE_HANDLER,
		.txfreedone_path = TXFREEDONE_FROM_MAC,
	}
};

struct pci_chip_profile mt7991_pci_wa_profile = {
	.int_mask_map = {
		/* map_idx 0: Legacy interrupt bitmap */
		[0] = {
			.int_sts_addr = WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_EXT_ADDR,
			.int_ena_addr = WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_ENA_EXT_ADDR,
		},
	},
	.pause_trxq_reg_addr = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_PAUSE0_ADDR,
	.pause_trxq_mask = (
		WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_PAUSE0_RX_RING_PAUSE_ALL_MASK |
		WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_PAUSE0_TX_RING_PAUSE_MASK),
	/*mcu2host swi*/
	.int_enable_swi_mask =
		WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_ENA_EXT_mcu2host_sw_int_ena_MASK,
	.mcu2host_sw_int_ena_addr = WF_WFDMA_HOST_DMA0_PCIE1_MCU2HOST_SW_INT_ENA_ADDR,
	.mcu2host_sw_int_sta_addr = WF_WFDMA_HOST_DMA0_PCIE1_MCU2HOST_SW_INT_STA_ADDR,
	.mcu2host_sw_int_ser_mask = (
		WF_WFDMA_HOST_DMA0_PCIE1_MCU2HOST_SW_INT_STA_mcu2host_sw_int_2_MASK |
		WF_WFDMA_HOST_DMA0_PCIE1_MCU2HOST_SW_INT_STA_mcu2host_sw_int_3_MASK |
		WF_WFDMA_HOST_DMA0_PCIE1_MCU2HOST_SW_INT_STA_mcu2host_sw_int_4_MASK |
		WF_WFDMA_HOST_DMA0_PCIE1_MCU2HOST_SW_INT_STA_mcu2host_sw_int_5_MASK),
	.mcu2host_sw_wdt_ser_mask = (
		WF_WFDMA_HOST_DMA0_PCIE1_MCU2HOST_SW_INT_STA_mcu2host_wacpu_wdt_MASK |
		WF_WFDMA_HOST_DMA0_PCIE1_MCU2HOST_SW_INT_STA_mcu2host_wmcpu_wdt_MASK),
	.tx_dma_glo_reg_addr = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_ADDR,
	.rx_dma_glo_reg_addr = WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_ADDR,
	.txq_num = ARRAY_SIZE(txq_wa_layout_pcie1),
	.rxq_num = ARRAY_SIZE(rxq_wa_layout_pcie1),
	.queue_layout = {
		.tx_queue_layout = txq_wa_layout_pcie1,
		.rx_queue_layout = rxq_wa_layout_pcie1,
	},
	.coherent_dma_addr_size = 32,
	.non_coherent_dma_addr_size = 36,
};
#endif
