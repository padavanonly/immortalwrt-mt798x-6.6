/* SPDX-License-Identifier: <SPDX License Expression> */
/*
 * Copyright (c) [2022] MediaTek Inc.
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

#ifndef __MTK_CHIP_MT7992_OPTION_TABLE_H
#define __MTK_CHIP_MT7992_OPTION_TABLE_H

#define MT7992_ZERO_NUM	(~0)

#define MT7992_SDB_BAND_SEL_DEFAULT 1
#define MT7992_SDB_BAND_SEL_NB 3

enum mt7992_chip_opt {
	MT7992_CHIP_OPT_PCIE0_2G_5G	  = 0,
	MT7992_CHIP_OPT_PCIE0_2G_PCIE1_5G = 1,
	MT7992_CHIP_OPT_MT7622		  = 2,
	MT7992_CHIP_OPT_BOLLINGER	  = 3,
	MT7992_CHIP_OPT_PCIE0_5G_PCIE1_2G = 4,
	MT7992_CHIP_OPT_GRIFFIN_NPU	  = 5,
};

struct mtk_intr_option_set mt7992_intr_opt_set[] = {
	[0] = { /* 1-PCIe INTx */
		.intr_opt_master = 0,
	},
	[1] = { /* 1-PCIe MSI 1msg */
		.intr_opt_master = 1,
	},
	[2] = { /* 1-PCIe MSI 8msg */
		.intr_opt_master = 2,
	},
	[3] = { /* 1-PCIe MSI 8msg w/ RSS */
		.intr_opt_master = 3,
	},
	[4] = { /* 2-PCIe INTx */
		.intr_opt_master = 4,
		.intr_opt_slave = 0,
	},
	[5] = { /* 2-PCIe MSI 1msg */
		.intr_opt_master = 5,
		.intr_opt_slave = 1,
	},
	[6] = { /* 2-PCIe MSI 8msg */
		.intr_opt_master = 6,
		.intr_opt_slave = 2,
	},
	[7] = { /* 2-PCIe MSI 8msg w/ RSS */
		.intr_opt_master = 7,
		.intr_opt_slave = 2,
	},
	[8] = { /* 2-PCIe INTx for MT7622 */
		.intr_opt_master = 8,
		.intr_opt_slave = 3,
	},
	[9] = { /* 1-PCIe MSI 1msg for Bollinger */
		.intr_opt_master = 9,
	},
};

struct mtk_intr_option_desc mt7992_intr_opt_tbl[] = {
	[0] = { /* 1-PCIe INTx */
		.irq_type = PCI_IRQ_LEGACY,
		.vec_num = 1,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_LUMP_ALL,
			},
		},
	},
	[1] = { /* 1-PCIe MSI 1-msg */
		.irq_type = PCI_IRQ_MSI,
		.vec_num = 1,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_LUMP_ALL,
			},
		},
	},
	[2] = { /* 1-PCIe MSI 8-msg */
		.irq_type = PCI_IRQ_MSI,
		.vec_num = 8,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_TXD_RING,
				.info = {
					.band = BIT(BAND0),
				},
				.ring_num = 1,
				.ring_id = TX_RING18,
				.int_ena_mask = IRQ_TX_RING18,
			},
			[1] = {
				.attr = VEC_ATTR_TXD_RING,
				.info = {
					.band = BIT(BAND1),
				},
				.ring_num = 1,
				.ring_id = TX_RING19,
				.int_ena_mask = IRQ_TX_RING19,
			},
			[2] = {
				.attr = VEC_ATTR_RX_TFD_RING,
				.info = {
					.band = BIT(BAND0),
				},
				.ring_num = 1,
				.ring_id = RX_RING2,
				.int_ena_mask = IRQ_RX_RING2,
				.repeat = true,
			},
			[3] = {
				.attr = VEC_ATTR_RXD_RING,
				.info = {
					.band = BIT(BAND0),
				},
				.ring_num = 1,
				.ring_id = RX_RING8,
				.int_ena_mask = IRQ_RX_RING8,
			},
			[4] = {
				.attr = VEC_ATTR_RXD_RING,
				.info = {
					.band = BIT(BAND1),
				},
				.ring_num = 1,
				.ring_id = RX_RING9,
				.int_ena_mask = IRQ_RX_RING9,
			},
			[5] = {
				.attr = VEC_ATTR_RX_EVT_RING,
				.info = {
					.mcu = BIT(MCU_WM) | BIT(MCU_WA),
				},
				.ring_num = 2,
				.ring_id = (RX_RING0 | RX_RING1),
				.int_ena_mask = (IRQ_RX_RING0 | IRQ_RX_RING1),
			},
			[6] = {
				.attr = VEC_ATTR_TX_CMD_RING,
				.info = {
					.mcu = BIT(MCU_WM) | BIT(MCU_WA),
				},
				.ring_num = 2,
				.ring_id = (TX_RING17 | TX_RING20),
				.int_ena_mask = (IRQ_TX_RING17 | IRQ_TX_RING20),
			},
			[7] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_MSI8_LUMP,
			},
		},
	},
	[3] = { /* 1-PCIe MSI 8-msg w/ RSS */
		.irq_type = PCI_IRQ_MSI,
		.is_rss = 1,
		.vec_num = 8,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_RRO_RSS,
				.int_ena_mask = IRQ_AP_RSS0,
			},
			[1] = {
				.attr = VEC_ATTR_RRO_RSS,
				.int_ena_mask = IRQ_AP_RSS1,
			},
			[2] = {
				.attr = VEC_ATTR_RX_TFD_RING,
				.info = {
					.band = BIT(BAND0),
				},
				.ring_num = 1,
				.ring_id = RX_RING2,
				.int_ena_mask = IRQ_RX_RING2,
				.repeat = true,
			},
			[3] = {
				.attr = VEC_ATTR_RXD_RING,
				.info = {
					.band = BIT(BAND0),
				},
				.ring_num = 1,
				.ring_id = RX_RING8,
				.int_ena_mask = IRQ_RX_RING8,
			},
			[4] = {
				.attr = VEC_ATTR_RXD_RING,
				.info = {
					.band = BIT(BAND1),
				},
				.ring_num = 1,
				.ring_id = RX_RING9,
				.int_ena_mask = IRQ_RX_RING9,
			},
			[5] = {
				.attr = VEC_ATTR_RRO_RSS,
				.int_ena_mask = IRQ_AP_RSS2,
			},
			[6] = {
				.attr = VEC_ATTR_RRO_RSS,
				.int_ena_mask = IRQ_AP_RSS3,
			},
			[7] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_MSI8_RSS_LUMP,
			},
		},
	},
	[4] = { /* 2-PCIe PCIe0 INTx */
		.irq_type = PCI_IRQ_LEGACY,
		.vec_num = 1,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_LUMP_ALL,
			},
		},
	},
	[5] = { /* 2-PCIe PCIe0 MSI 1-msg */
		.irq_type = PCI_IRQ_MSI,
		.vec_num = 1,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_LUMP_ALL,
			},
		},
	},
	[6] = { /* 2-PCIe PCIe0 MSI 8-msg */
		.irq_type = PCI_IRQ_MSI,
		.vec_num = 8,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_TXD_RING,
				.info = {
					.band = BIT(BAND0),
				},
				.ring_num = 1,
				.ring_id = TX_RING18,
				.int_ena_mask = IRQ_TX_RING18,
			},
			[1] = {
				.attr = VEC_ATTR_RX_TFD_RING,
				.info = {
					.band = BIT(BAND0),
				},
				.ring_num = 1,
				.ring_id = RX_RING2,
				.int_ena_mask = IRQ_RX_RING2,
			},
			[2] = {
				.attr = VEC_ATTR_RXD_RING,
				.info = {
					.band = BIT(BAND0),
				},
				.ring_num = 1,
				.ring_id = RX_RING8,
				.int_ena_mask = IRQ_RX_RING8,
			},
			[3] = {
				.attr = VEC_ATTR_RX_EVT_RING,
				.info = {
					.mcu = BIT(MCU_WM) | BIT(MCU_WA),
				},
				.ring_num = 2,
				.ring_id = (RX_RING0 | RX_RING1),
				.int_ena_mask = (IRQ_RX_RING0 | IRQ_RX_RING1),
			},
			[4] = {
				.attr = VEC_ATTR_TX_CMD_RING,
				.info = {
					.mcu = BIT(MCU_WM) | BIT(MCU_WA),
				},
				.ring_num = 2,
				.ring_id = (TX_RING17 | TX_RING20),
				.int_ena_mask = (IRQ_TX_RING17 | IRQ_TX_RING20),
			},
			[5] = {
				.attr = VEC_ATTR_TX_FWDL_RING,
				.ring_num = 1,
				.ring_id = TX_RING16,
				.int_ena_mask = IRQ_TX_RING16,
			},
			[6] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_PCIE0_MSI8_LUMP,
			},
		},
	},
	[7] = { /* 2-PCIe PCIe0 MSI 8-msg w/ RSS */
		.irq_type = PCI_IRQ_MSI,
		.is_rss = 1,
		.vec_num = 8,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_RRO_RSS,
				.int_ena_mask = IRQ_AP_RSS0,
			},
			[1] = {
				.attr = VEC_ATTR_RX_TFD_RING,
				.info = {
					.band = BIT(BAND0),
				},
				.ring_num = 1,
				.ring_id = RX_RING2,
				.int_ena_mask = IRQ_RX_RING2,
			},
			[2] = {
				.attr = VEC_ATTR_RXD_RING,
				.info = {
					.band = BIT(BAND0),
				},
				.ring_num = 1,
				.ring_id = RX_RING8,
				.int_ena_mask = IRQ_RX_RING8,
			},
			[3] = {
				.attr = VEC_ATTR_RRO_RSS,
				.int_ena_mask = IRQ_AP_RSS2,
			},
			[4] = {
				.attr = VEC_ATTR_RRO_RSS,
				.int_ena_mask = IRQ_AP_RSS3,
			},
			[5] = {
				.attr = VEC_ATTR_RRO_RSS,
				.int_ena_mask = IRQ_AP_RSS1,
			},
			[6] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_PCIE0_MSI8_RSS_LUMP,
			},
		},
	},
	[8] = { /* 2-PCIe PCIe0 INTx for MT7622 */
		.irq_type = PCI_IRQ_LEGACY,
		.vec_num = 1,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_WED_LUMP_ALL,
			},
		},
	},
	[9] = { /* 1-PCIe MSI 1msg for Bollinger */
		.irq_type = PCI_IRQ_MSI,
		.vec_num = 1,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_LUMP_ALL,
			},
		},
		.alloc_vec_num = 32,
	},
};

struct mtk_intr_option_desc mt799a_intr_opt_tbl[] = {
	[0] = { /* 2-PCIe PCIe1 INTx */
		.irq_type = PCI_IRQ_LEGACY,
		.vec_num = 1,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_PCIE1_LUMP_ALL,
			},
		},
	},
	[1] = { /* 2-PCIe PCIe1 MSI 1-msg */
		.irq_type = PCI_IRQ_MSI,
		.vec_num = 1,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_PCIE1_LUMP_ALL,
			},
		},
	},
	[2] = { /* 2-PCIe PCIe1 MSI 8-msg (both for RSS & non RSS) */
		.irq_type = PCI_IRQ_MSI,
		.vec_num = 8,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_TXD_RING,
				.info = {
					.band = BIT(BAND1),
				},
				.ring_num = 1,
				.ring_id = TX_RING21,
				.int_ena_mask = IRQ_PCIE1_TX_RING21,
			},
			[1] = {
				.attr = VEC_ATTR_RX_TFD_RING,
				.info = {
					.band = BIT(BAND1),
				},
				.ring_num = 1,
				.ring_id = RX_RING3,
				.int_ena_mask = IRQ_PCIE1_RX_RING3,
			},
			[2] = {
				.attr = VEC_ATTR_RXD_RING,
				.info = {
					.band = BIT(BAND1),
				},
				.ring_num = 1,
				.ring_id = RX_RING9,
				.int_ena_mask = IRQ_PCIE1_RX_RING9,
			},
			[3] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_PCIE1_MSI8_LUMP,
			},
		},
	},
	[3] = { /* 2-PCIe PCIe1 INTx */
		.irq_type = PCI_IRQ_LEGACY,
		.vec_num = 1,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_WED_PCIE1_LUMP_ALL,
			},
		},
	},
};

struct mtk_chip_option_table mt7992_chip_opt_tbl[] = {
	[MT7992_CHIP_OPT_PCIE0_2G_5G] = {
		.option_info = "Kite (1PCIE 2+5)",
		.chip_desc = {
			.rx_path_type = 0,
			.rro_bypass_type = BMC_BYPASS_UC_BASED_ON_WTBL,
			.mld_dest_type = 0,
			.txfreedone_path = TXFREEDONE_FROM_WA,
			.force_rro_disable = false,
			.rro_disable_rro_bypass_type = ALL_BYPASS,
			.rro_disable_mld_dest_type = 0,
			.rro_disable_txfreedone_path = TXFREEDONE_FROM_WA,
		},
		.bus_desc_master = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND0),
			},
			[1] = {
				.band_idx_bmp = BIT(BAND1),
			},
			},
		},
	},
	[MT7992_CHIP_OPT_PCIE0_2G_PCIE1_5G] = {
		.option_info = "Kite (2PCIE 2/5)",
		.chip_desc = {
			.rx_path_type = 7,
			.rro_bypass_type = BMC_BYPASS_UC_BASED_ON_WTBL,
			.mld_dest_type = 0,
			.txfreedone_path = TXFREEDONE_FROM_WA,
			.force_rro_disable = false,
			.rro_disable_rro_bypass_type = ALL_BYPASS,
			.rro_disable_mld_dest_type = 0,
			.rro_disable_txfreedone_path = TXFREEDONE_FROM_WA,
		},
		.bus_desc_master = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND0),
				.q_type_bmp = BIT(Q_TYPE_PRIO),
			},
			[1] = {
				.band_idx_bmp = BIT(BAND0),
				.q_type_bmp = BIT(Q_TYPE_ACQ_BE) | BIT(Q_TYPE_ACQ_BK) |
					      BIT(Q_TYPE_ACQ_VI) | BIT(Q_TYPE_ACQ_VO),
			},
			},
		},
		.bus_desc_slave = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND1),
				.q_type_bmp = BIT(Q_TYPE_PRIO),
			},
			[1] = {
				.band_idx_bmp = BIT(BAND1),
				.q_type_bmp = BIT(Q_TYPE_ACQ_BE) | BIT(Q_TYPE_ACQ_BK) |
					      BIT(Q_TYPE_ACQ_VI) | BIT(Q_TYPE_ACQ_VO),
			},
			},
		},
	},
	[MT7992_CHIP_OPT_MT7622] = {
		.option_info = "MT7622 Kite (2PCIE 2/5)",
		.chip_desc = {
			.rx_path_type = 7,
			.rro_bypass_type = BMC_BYPASS_UC_BASED_ON_WTBL,
			.mld_dest_type = 0,
			.txfreedone_path = TXFREEDONE_FROM_WA,
			.force_rro_disable = false,
			.rro_disable_rro_bypass_type = ALL_BYPASS,
			.rro_disable_mld_dest_type = 0,
			.rro_disable_txfreedone_path = TXFREEDONE_FROM_WA,
		},
		.bus_desc_master = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND0),
			},
			[1] = {
				.band_idx_bmp = BIT(BAND1),
			},
			},
		},
		.bus_desc_slave = {
			.offload_disable = true,
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = 0,
			},
			[1] = {
				.band_idx_bmp = 0,
			},
			},
		},
	},
	[MT7992_CHIP_OPT_BOLLINGER] = {
		.option_info = "Bollinger Kite (1PCIE 2+5)",
		.chip_desc = {
			.rx_path_type = 0,
			.force_rro_disable = true,
			.rro_disable_rro_bypass_type = ALL_BYPASS,
			.rro_disable_txfreedone_path = TXFREEDONE_FROM_WA,
		},
		.bus_desc_master = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND0),
			},
			[1] = {
				.band_idx_bmp = BIT(BAND1),
			},
			},
			.ext_tx_tk_nums_master = 8192,
			.ext_tx_tk_nums_slave = 8192,
		},
	},
	[MT7992_CHIP_OPT_PCIE0_5G_PCIE1_2G] = {
		.option_info = "Kite (2PCIE 5/2)",
		.chip_desc = {
			.rx_path_type = 8,
			.rro_bypass_type = BMC_BYPASS_UC_BASED_ON_WTBL,
			.mld_dest_type = 0,
			.txfreedone_path = TXFREEDONE_FROM_WA,
			.force_rro_disable = false,
			.rro_disable_rro_bypass_type = ALL_BYPASS,
			.rro_disable_mld_dest_type = 0,
			.rro_disable_txfreedone_path = TXFREEDONE_FROM_WA,
		},
		.bus_desc_master = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND1),
				.q_type_bmp = BIT(Q_TYPE_PRIO),
			},
			[1] = {
				.band_idx_bmp = BIT(BAND1),
				.q_type_bmp = BIT(Q_TYPE_ACQ_BE) | BIT(Q_TYPE_ACQ_BK) |
					      BIT(Q_TYPE_ACQ_VI) | BIT(Q_TYPE_ACQ_VO),
			},
			},
		},
		.bus_desc_slave = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND0),
				.q_type_bmp = BIT(Q_TYPE_PRIO),
			},
			[1] = {
				.band_idx_bmp = BIT(BAND0),
				.q_type_bmp = BIT(Q_TYPE_ACQ_BE) | BIT(Q_TYPE_ACQ_BK) |
					      BIT(Q_TYPE_ACQ_VI) | BIT(Q_TYPE_ACQ_VO),
			},
			},
		},
	},
	[MT7992_CHIP_OPT_GRIFFIN_NPU] = {
		.option_info = "Griffin NPU (2PCIE 2/5 with 1WED)",
		.chip_desc = {
			.rx_path_type = 7,
			.rro_bypass_type = BMC_BYPASS_UC_BASED_ON_WTBL,
			.mld_dest_type = 0,
			.txfreedone_path = TXFREEDONE_FROM_WA,
			.force_rro_disable = false,
			.rro_disable_rro_bypass_type = ALL_BYPASS,
			.rro_disable_mld_dest_type = 0,
			.rro_disable_txfreedone_path = TXFREEDONE_FROM_WA,
		},
		.bus_desc_master = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND0),
			},
			[1] = {
				.band_idx_bmp = BIT(BAND1),
			},
			},
		},
	},
};

#endif
