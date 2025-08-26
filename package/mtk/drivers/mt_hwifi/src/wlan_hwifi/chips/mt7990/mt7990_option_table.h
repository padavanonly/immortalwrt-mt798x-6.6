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

#ifndef __MTK_CHIP_MT7990_OPTION_TABLE_H
#define __MTK_CHIP_MT7990_OPTION_TABLE_H

#define MT7990_ZERO_NUM	(~0)
#define MT7990_CHIP_OPT_AN7581 5

struct mtk_intr_option_set mt7990_intr_opt_set[] = {
	[0] = { /* INTx */
		.intr_opt_master = 0,
		.intr_opt_slave = 0,
	},
	[1] = { /* MSI 1msg */
		.intr_opt_master = 1,
		.intr_opt_slave = 1,
	}
};

struct mtk_intr_option_desc mt7990_intr_opt_tbl[] = {
	[0] = { /* INTx */
		.irq_type = PCI_IRQ_LEGACY,
		.vec_num = 1,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_LUMP_ALL,
			},
		},
	},
	[1] = { /* MSI 1msg */
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

struct mtk_intr_option_desc mt7991_intr_opt_tbl[] = {
	[0] = { /* INTx */
		.irq_type = PCI_IRQ_LEGACY,
		.vec_num = 1,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_PCIE1_LUMP_ALL,
			},
		},
	},
	[1] = { /* MSI 1msg */
		.irq_type = PCI_IRQ_MSI,
		.vec_num = 1,
		.vec_data = {
			[0] = {
				.attr = VEC_ATTR_LUMP,
				.int_ena_mask = IRQ_PCIE1_LUMP_ALL,
			},
		},
	},
};

struct mtk_chip_option_table mt7990_chip_opt_tbl[] = {
	[0] = {
		.option_info = "Jaguar (1PCIE 2+5+6)",
		.chip_desc = {
			.rx_path_type = 0,
			.rro_bypass_type = ALL_NOT_BYPASS,
			.txfreedone_path = TXFREEDONE_FROM_MAC,
			.force_rro_disable = false,
			.rro_disable_rro_bypass_type = ALL_BYPASS,
			.rro_disable_txfreedone_path = TXFREEDONE_FROM_WA,
		},
		.bus_desc_master = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND0) | BIT(BAND1),
			},
			[1] = {
				.band_idx_bmp = BIT(BAND2),
			},
			},
		},
	},
	[1] = {
		.option_info = "Jaguar (2PCIE 2+5/6)",
		.chip_desc = {
			.rx_path_type = 2,
			.rro_bypass_type = ALL_NOT_BYPASS,
			.txfreedone_path = TXFREEDONE_FROM_MAC,
			.force_rro_disable = false,
			.rro_disable_rro_bypass_type = ALL_BYPASS,
			.rro_disable_txfreedone_path = TXFREEDONE_FROM_WA,
			.max_ba_wsize_scene_mlo = 512,
		},
		.bus_desc_master = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND0) | BIT(BAND1) | BIT(BAND2),
				.q_type_bmp = BIT(Q_TYPE_PRIO),
			},
			[1] = {
				.band_idx_bmp = BIT(BAND0) | BIT(BAND1),
				.q_type_bmp = BIT(Q_TYPE_ACQ_BE) | BIT(Q_TYPE_ACQ_BK) |
					      BIT(Q_TYPE_ACQ_VI) | BIT(Q_TYPE_ACQ_VO),
			},
			},
		},
		.bus_desc_slave = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND2),
				.q_type_bmp = BIT(Q_TYPE_ACQ_BE) | BIT(Q_TYPE_ACQ_BK) |
					      BIT(Q_TYPE_ACQ_VI) | BIT(Q_TYPE_ACQ_VO),
			},
			[1] = {
				.band_idx_bmp = 0,
			},
			},
		},
	},
	[2] = {
		.option_info = "Jaguar (2PCIE 6/2+5)",
		.chip_desc = {
			.rx_path_type = 3,
			.rro_bypass_type = ALL_NOT_BYPASS,
			.txfreedone_path = TXFREEDONE_FROM_MAC,
			.force_rro_disable = false,
			.rro_disable_rro_bypass_type = ALL_BYPASS,
			.rro_disable_txfreedone_path = TXFREEDONE_FROM_WA,
			.max_ba_wsize_scene_mlo = 512,
		},
		.bus_desc_master = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND0) | BIT(BAND1) | BIT(BAND2),
				.q_type_bmp = BIT(Q_TYPE_PRIO),
			},
			[1] = {
				.band_idx_bmp = BIT(BAND2),
				.q_type_bmp = BIT(Q_TYPE_ACQ_BE) | BIT(Q_TYPE_ACQ_BK) |
					      BIT(Q_TYPE_ACQ_VI) | BIT(Q_TYPE_ACQ_VO),
			},
			},
		},
		.bus_desc_slave = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND0) | BIT(BAND1),
				.q_type_bmp = BIT(Q_TYPE_ACQ_BE) | BIT(Q_TYPE_ACQ_BK) |
					      BIT(Q_TYPE_ACQ_VI) | BIT(Q_TYPE_ACQ_VO),
			},
			[1] = {
				.band_idx_bmp = 0,
			},
			},
		},
	},
	[3] = {
		.option_info = "Jaguar (1PCIE 5+6)",
		.chip_desc = {
			.rx_path_type = 4,
			.rro_bypass_type = ALL_NOT_BYPASS,
			.txfreedone_path = TXFREEDONE_FROM_MAC,
			.force_rro_disable = false,
			.rro_disable_rro_bypass_type = ALL_BYPASS,
			.rro_disable_txfreedone_path = TXFREEDONE_FROM_WA,
			.max_ba_wsize_scene_mlo = 512,
		},
		.bus_desc_master = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND1),
			},
			[1] = {
				.band_idx_bmp = BIT(BAND2),
			},
			},
		},
	},
	[4] = {
		.option_info = "Zeta Tau Epsilon",
		.chip_desc = {
			.rx_path_type = 5,
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
				.band_idx_bmp = BIT(BAND2),
			},
			},
		},
		.bus_desc_slave = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND1),
			},
			[1] = {
				.band_idx_bmp = 0,
			},
			},
		},
	},
	[5] = {
		.option_info = "AN7581",
		.chip_desc = {
			.rx_path_type = 6,
			.rro_bypass_type = ALL_NOT_BYPASS,
			.mld_dest_type = 0,
			.force_rro_disable = false,
			.rro_disable_rro_bypass_type = ALL_BYPASS,
			.rro_disable_txfreedone_path = TXFREEDONE_FROM_WA,
		},
		.bus_desc_master = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND1) | BIT(BAND2),
			},
			[1] = {
				.band_idx_bmp = 0,
			},
			},
		},
		.bus_desc_slave = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND0),
			},
			[1] = {
				.band_idx_bmp = 0,
			},
			},
		},
	},
	[6] = {
		.option_info = "Bollinger",
		.chip_desc = {
			.rx_path_type = 2,
			.force_rro_disable = true,
			.rro_disable_rro_bypass_type = ALL_BYPASS,
			.rro_disable_txfreedone_path = TXFREEDONE_FROM_WA,
		},
		.bus_desc_master = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND0) | BIT(BAND1),
				.q_type_bmp = BIT(Q_TYPE_ACQ_BE) | BIT(Q_TYPE_ACQ_BK) |
					      BIT(Q_TYPE_ACQ_VI) | BIT(Q_TYPE_ACQ_VO),
			},
			[1] = {
				.band_idx_bmp = BIT(BAND0) | BIT(BAND1) | BIT(BAND2),
				.q_type_bmp = BIT(Q_TYPE_PRIO),
			},
			},
			.ext_tx_tk_nums_master = 8192,
			.ext_tx_tk_nums_slave = 8192,
		},
		.bus_desc_slave = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND2),
				.q_type_bmp = BIT(Q_TYPE_ACQ_BE) | BIT(Q_TYPE_ACQ_BK) |
					      BIT(Q_TYPE_ACQ_VI) | BIT(Q_TYPE_ACQ_VO),
			},
			[1] = {
				.band_idx_bmp = 0,
			},
			},
		},
	},
	[7] = {
		.option_info = "Jaguar (2PCIE 2+5/6 with 1WED)",
		.chip_desc = {
			.rx_path_type = 2,
			.rro_bypass_type = ALL_NOT_BYPASS,
			.txfreedone_path = TXFREEDONE_FROM_MAC,
			.force_rro_disable = false,
			.rro_disable_rro_bypass_type = ALL_BYPASS,
			.rro_disable_txfreedone_path = TXFREEDONE_FROM_WA,
			.max_ba_wsize_scene_mlo = 512,
		},
		.bus_desc_master = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND0) | BIT(BAND1),
			},
			[1] = {
				.band_idx_bmp = BIT(BAND2),
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
	[8] = {
		.option_info = "Griffin NPU (2PCIE 2+5/6 with 1WED)",
		.chip_desc = {
			.rx_path_type = 2,
			.rro_bypass_type = ALL_NOT_BYPASS,
			.txfreedone_path = TXFREEDONE_FROM_MAC,
			.force_rro_disable = false,
			.rro_disable_rro_bypass_type = ALL_BYPASS,
			.rro_disable_txfreedone_path = TXFREEDONE_FROM_WA,
			.max_ba_wsize_scene_mlo = 512,
		},
		.bus_desc_master = {
			.tx_ring_num = 2,
			.tx_ring_desc = {
			[0] = {
				.band_idx_bmp = BIT(BAND0) | BIT(BAND1),
			},
			[1] = {
				.band_idx_bmp = BIT(BAND2),
			},
			},
		},
	},
};
#endif

