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

#ifndef __MTK_CHIP_MT7990_H
#define __MTK_CHIP_MT7990_H

#include <chips.h>
#include <bus.h>
#include <core.h>
#include <bus/mtk_pci_dma.h>
#include <bus/mtk_wed.h>
#include <bus/mtk_rro.h>
#include "mt7990_wfdma_wa.h"
#include "mt7990_option_table.h"

#define MT7990_FIRMWARE_WA				"WIFI_MT7990_WACPU_RAM_CODE_1_1.bin"
#define MT7990_FIRMWARE_WM				"WIFI_RAM_CODE_MT7990_1_1.bin"
#define MT7990_FIRMWARE_WM_TESTMODE			"WIFI_RAM_CODE_MT7990_1_1_TESTMODE.bin"
#define MT7990_ROM_PATCH				"WIFI_MT7990_PATCH_MCU_1_1_hdr.bin"
#define MT7990_FIRMWARE_DSP				"WIFI_MT7990_PHY_RAM_CODE_1_1.bin"

#define MT7990_MAX_VEC_NUM	1
#define MT7990_MAX_BUS_NUM 2
#define MT7990_LEGACY_MAX_BIT_NUM 32

#define L1_REMAP_ADDR		0x155024
#define L1_REMAP_MASK		0xFFFF0000
#define L1_REMAP_SHFT		16
#define L1_REMAP_BASE_ADDR	0x130000
#define L1_REMAP_BASE_SHFT	16
#define L1_REMAP_BASE_MASK	0xFFFF
#define GET_L1_REMAP_BASE(addr) ((addr) >> L1_REMAP_BASE_SHFT)
#define GET_L1_REMAP_OFFSET(addr) ((addr) & L1_REMAP_BASE_MASK)
#define L2_REMAP_ADDR		0x000120
#define L2_REMAP_MASK		0xFFFFFFFF
#define L2_REMAP_SHFT		0
#define L2_REMAP_BASE_ADDR	0x18500000
#define L2_REMAP_BASE_SHFT	0
#define GET_L2_REMAP_BASE(addr) ((addr) >> L2_REMAP_BASE_SHFT)

#define DEFAULT_RID 0

#ifdef CONFIG_HWIFI_CHIP_MT7990_FPGA
#define MT7990_WMM_SET_END 0x1
#else
#define MT7990_WMM_SET_END 0x3
#endif

#define MT7990_SW_TX_RSV_TOKEN_NUM_PER_BAND 100

static inline u32
get_rid_value(struct pci_trans *trans)
{
	u32 rid;

	rid = bus_read(to_bus_trans(trans), WF_WFDMA_EXT_WRAP_CSR_WFDMA_PCIE_RECOG_ADDR);

	dev_info(to_device(trans), "%s(): get rid = %u\n", __func__, rid);
	return ((rid & WF_WFDMA_EXT_WRAP_CSR_WFDMA_PCIE_RECOG_PCIE_RECOG_ID_MASK) >>
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_PCIE_RECOG_PCIE_RECOG_ID_SHFT);
}

static inline void
set_rid_value(struct pci_trans *trans, u32 rid)
{
	u32 value = WF_WFDMA_EXT_WRAP_CSR_WFDMA_PCIE_RECOG_PCIE_RECOG_SEM_MASK | rid;

	dev_info(to_device(trans), "%s(): set rid = %u\n", __func__, rid);
	bus_write(to_bus_trans(trans), WF_WFDMA_EXT_WRAP_CSR_WFDMA_PCIE_RECOG_ADDR, value);
}
#endif
