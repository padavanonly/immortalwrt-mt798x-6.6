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

#ifndef __MTK_CHIP_MT7992_H
#define __MTK_CHIP_MT7992_H

#include <chips.h>
#include <bus.h>
#include <core.h>
#include <bus/mtk_pci_dma.h>
#include <bus/mtk_wed.h>
#include <bus/mtk_rro.h>
#include "mt7992_wfdma_wa.h"
#include "mt7992_option_table.h"

#define MT7992_FIRMWARE_WA				"WIFI_MT7992_WACPU_RAM_CODE_1_1.bin"
#define MT7992_FIRMWARE_WM				"WIFI_RAM_CODE_MT7992_1_1.bin"
#define MT7992_FIRMWARE_WM_TESTMODE		"WIFI_RAM_CODE_MT7992_1_1_TESTMODE.bin"
#define MT7992_ROM_PATCH				"WIFI_MT7992_PATCH_MCU_1_1_hdr.bin"
#define MT7992_FIRMWARE_DSP				"WIFI_MT7992_PHY_RAM_CODE_1_1.bin"

#define MT7992_MAX_VEC_NUM	8
#define MT7992_MAX_BUS_NUM 2
#define MT7992_LEGACY_MAX_BIT_NUM 32
#define MT7992_RSS_MAX_BIT_NUM 4

#define SET_PCIE0_MSI_NUM(num) \
	((num) << WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pcie0_msi_num_SHFT)
#define CLEAR_PCIE0_MSI_NUM() \
	(~WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pcie0_msi_num_MASK)

#define SET_PCIE1_MSI_NUM(num) \
	((num) << WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pcie1_msi_num_SHFT)
#define CLEAR_PCIE1_MSI_NUM() \
	(~WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pcie1_msi_num_MASK)

#define DEFAULT_RID 0

#ifdef CONFIG_HWIFI_CHIP_MT7992_FPGA
#define MT7992_WMM_SET_END 0x1
#else
#define MT7992_WMM_SET_END 0x3
#endif

#define TOKEN_END(TOKEN_TYPE) \
	((TOKEN_TYPE##_TOKEN_NUM) ? \
	((TOKEN_TYPE##_TOKEN_START) + (TOKEN_TYPE##_TOKEN_NUM) - 1) : \
	(TOKEN_TYPE##_TOKEN_START)) \

#ifdef CONFIG_HWIFI_MEM_SHRINK
/* SW Tx token */
#define MT7992_SW_TX_TOKEN_NUM		12288
#define MT7992_SW_TX_TOKEN_NUM_PER_BAND	12288
/* SW Tx BMC token */
#define MT7992_SW_TX_BMC_TOKEN_NUM	1024
/* SW Rx token */
#define MT7992_SW_RX_TOKEN_NUM		800	/* 2 legacy RxRing */
#define MT7992_SW_RRO_RX_TOKEN_NUM	5200
#else
/* SW Tx token */
#define MT7992_SW_TX_TOKEN_NUM		16384
#define MT7992_SW_TX_TOKEN_NUM_PER_BAND	16384
/* SW Tx BMC token */
#define MT7992_SW_TX_BMC_TOKEN_NUM	1024
/* SW Rx token */
#define MT7992_SW_RX_TOKEN_NUM		3104	/* 4 legacy RxRing */
#define MT7992_SW_RRO_RX_TOKEN_NUM	32768
#endif

#define MT7992_SW_TX_RSV_TOKEN_NUM_PER_BAND	100

#define MT7992_SW_TX_TOKEN_START	0
#define MT7992_SW_TX_TOKEN_END		TOKEN_END(MT7992_SW_TX)
/* WA only support BMC token range in 16384 to 17407 (1K) */
#define MT7992_SW_TX_BMC_TOKEN_START	16384
#define MT7992_SW_TX_BMC_TOKEN_END	TOKEN_END(MT7992_SW_TX_BMC)
#define MT7992_SW_RX_TOKEN_START	0
#define MT7992_SW_RX_TOKEN_END		TOKEN_END(MT7992_SW_RX)

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

static inline u32
token_end(u32 token_start, u32 token_num)
{
	return token_num ? token_start + token_num - 1 : token_start;
}

#endif

