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
	warp_hw.c
*/
#include <warp.h>
#include <warp_hw.h>
#include <warp_utility.h>
#include "wox_ap2wo_mcu_ccif4.h"
#include <mcu/warp_ccif.h>


void
warp_ccif_kickout(void *hw)
{
	struct ccif_entry *ccif = (struct ccif_entry *) hw;

	warp_io_write32((struct ccif_entry *) hw, WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_BUSY_ADDR,
		BIT(ccif->tx_ring->chnum));
	warp_io_write32((struct ccif_entry *) hw, WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_TCHNUM_ADDR,
		ccif->tx_ring->chnum);

	warp_dbg(WARP_DBG_LOU, "%s(): write tx_ch: %d & tx busy: %x\n", __func__,
		ccif->tx_ring->chnum, ccif->tx_ring->chnum);
}

void
warp_ccif_set_ack(void *hw)
{
	struct ccif_entry *ccif = (struct ccif_entry *) hw;

	warp_io_write32((struct ccif_entry *) hw, WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_ACK_ADDR,
		ccif->rx_ring->irq_mask);
	warp_dbg(WARP_DBG_LOU, "%s(): write ack: %d & rx ack: %x\n", __func__,
		ccif->rx_ring->irq_mask, ccif->rx_ring->irq_mask);
}

u32
warp_ccif_get_rxchnum(void *hw)
{
	u32 val;

	warp_io_read32((struct ccif_entry *) hw, WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_RCHNUM_ADDR, &val);
	val = SET_FIELD(WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_RCHNUM_RCHNUM, val);
	warp_dbg(WARP_DBG_LOU, "%s(): get rxch: %x\n", __func__, val);
	return val;
}

void
warp_ccif_clear_int(void *hw)
{
	u32 val = 0;
	struct ccif_entry *ccif = (struct ccif_entry *) hw;

	warp_io_write32((struct ccif_entry *) hw, WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_ACK_ADDR,
		ccif->rx_ring->irq_mask);
	warp_dbg(WARP_DBG_LOU, "%s(): clear_warp_CCIF\n", __func__);
	warp_io_read32((struct ccif_entry *) hw, WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_RCHNUM_ADDR, &val);
	val = SET_FIELD(WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_RCHNUM_RCHNUM, val);
	warp_dbg(WARP_DBG_LOU, "%s(): get rxch: %x\n", __func__, val);
}

void
warp_ccif_isr_ctrl(void *hw, bool enable)
{
	struct ccif_entry *ccif = (struct ccif_entry *) hw;

	if (enable)
		warp_io_write32((struct ccif_entry *) hw,
			WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ0_MASK_ADDR, ccif->rx_ring->irq_mask);
	else
		warp_io_write32((struct ccif_entry *) hw,
			WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_IRQ0_MASK_ADDR, 0);
}

static const u32 ccif_dummy_table[] = {
	WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY1_ADDR,
	WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY2_ADDR,
	WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY3_ADDR,
	WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY4_ADDR,
	WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY5_ADDR,
	WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY6_ADDR,
	WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY7_ADDR,
	WOX_AP2WO_MCU_CCIF4_CPUA_PCCIF_DUMMY8_ADDR
};

static const u32 ccif_shadow_table[] = {
	WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW1_ADDR,
	WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW2_ADDR,
	WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW3_ADDR,
	WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW4_ADDR,
	WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW5_ADDR,
	WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW6_ADDR,
	WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW7_ADDR,
	WOX_AP2WO_MCU_CCIF4_CPUB2CPUA_SHADOW8_ADDR
};

void
warp_ccif_tx_ring_get_hw(void *hw, struct warp_bus_ring *ring)
{
	struct ccif_entry *ccif = (struct ccif_entry *) hw;
	struct ccif_ring_ctrl *tx_ring = ccif->tx_ring;

	ring->hw_desc_base = ccif_dummy_table[tx_ring->base_addr_dnum];
	ring->hw_cnt_addr  = ccif_dummy_table[tx_ring->cnt_dnum];
	ring->hw_cidx_addr = ccif_dummy_table[tx_ring->cidx_dnum];
	ring->hw_didx_addr = ccif_shadow_table[tx_ring->didix_dnum];
}


void
warp_ccif_rx_ring_get_hw(void *hw, struct warp_bus_ring *ring)
{
	struct ccif_entry *ccif = (struct ccif_entry *) hw;
	struct ccif_ring_ctrl *rx_ring = ccif->rx_ring;

	ring->hw_desc_base = ccif_dummy_table[rx_ring->base_addr_dnum];
	ring->hw_cnt_addr  = ccif_dummy_table[rx_ring->cnt_dnum];
	ring->hw_cidx_addr = ccif_dummy_table[rx_ring->cidx_dnum];
	ring->hw_didx_addr = ccif_shadow_table[rx_ring->didix_dnum];
}
