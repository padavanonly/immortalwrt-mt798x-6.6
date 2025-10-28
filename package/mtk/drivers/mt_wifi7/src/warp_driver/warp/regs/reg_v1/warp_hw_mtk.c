// SPDX-License-Identifier: <SPDX License Expression>
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
	warp_hw_mtk.c
*/

#include <warp.h>
#include <warp_hw.h>
#include "warp_hw_mtk.h"

static void
wdma_dma_ctrl(struct wdma_entry *wdma, u8 txrx)
{
	u32 wdma_cfg = 0;

	warp_io_read32(wdma, WDMA_GLO_CFG, &wdma_cfg);
	warp_dbg(WARP_DBG_OFF, "%s(): WDMA_GLO_CFG=%x\n",  __func__, wdma_cfg);

	if (txrx) {
		/*reset wdma*/
		wdma_cfg |= (1 << WDMA_GLO_CFG_RX_INFO1_PRERESERVE);
		wdma_cfg |= (1 << WDMA_GLO_CFG_RX_INFO2_PRERESERVE);
		wdma_cfg |= (1 << WDMA_GLO_CFG_RX_INFO3_PRERESERVE);
		warp_io_write32(wdma, WDMA_GLO_CFG, wdma_cfg);
	} else {
		wdma_cfg &= ~(1 << WDMA_GLO_CFG_RX_INFO1_PRERESERVE);
		wdma_cfg &= ~(1 << WDMA_GLO_CFG_RX_INFO2_PRERESERVE);
		warp_io_write32(wdma, WDMA_GLO_CFG, wdma_cfg);
	}
}

void
warp_wed_init_hw(struct wed_entry *wed, struct wdma_entry *wdma)
{
	u32 wed_wdma_cfg;

	/*cfg wdma recycle*/
	warp_io_read32(wed, WED_WDMA_GLO_CFG, &wed_wdma_cfg);
	wed_wdma_cfg &= ~(WED_WDMA_GLO_CFG_FLD_WDMA_BT_SIZE_MASK <<
			  WED_WDMA_GLO_CFG_FLD_WDMA_BT_SIZE);
	wed_wdma_cfg &= ~((1 << WED_WDMA_GLO_CFG_FLD_IDLE_STATE_DMAD_SUPPLY_EN) |
			  (1 << WED_WDMA_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_PREPARE) |
			  (1 << WED_WDMA_GLO_CFG_FLD_DYNAMIC_DMAD_RECYCLE));
	/*disable auto idle*/
	wed_wdma_cfg &= ~(1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_DISABLE_FSM_AUTO_IDLE);
	/*Set to 16 DWORD for 64bytes*/
	wed_wdma_cfg |= (0x2 << WED_WDMA_GLO_CFG_FLD_WDMA_BT_SIZE);
	/*enable skip state for fix dma busy issue*/
	wed_wdma_cfg |= ((1 << WED_WDMA_GLO_CFG_FLD_IDLE_STATE_DMAD_SUPPLY_EN) |
			 (1 << WED_WDMA_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_PREPARE));
#ifdef WED_HW_TX_SUPPORT
	warp_io_write32(wed, WED_WDMA_GLO_CFG, wed_wdma_cfg);
#endif /*WED_HW_TX_SUPPORT*/
}

void
warp_wdma_init_hw(struct wed_entry *wed, struct wdma_entry *wdma, int idx)
{
	u32 value = 0;

	/*Apply WDMA  related setting*/
	wdma_dma_ctrl(wdma, WARP_DMA_TXRX);
	/*offset 0*/
	value = (idx) ? WDMA1_OFST0 : WDMA0_OFST0;
	warp_io_write32(wed, WED_WDMA_OFST0, value);
	/*offset 1*/
	value = (idx) ? WDMA1_OFST1 : WDMA0_OFST1;
	warp_io_write32(wed, WED_WDMA_OFST1, value);
}
