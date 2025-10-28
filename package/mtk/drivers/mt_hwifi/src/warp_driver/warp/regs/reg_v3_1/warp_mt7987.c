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
	warp_mt7988.c
*/

#include <warp.h>
#include <warp_hw.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include "wed_hw.h"

void
warp_whole_chip_wo_reset(void)
{
	warp_dbg(WARP_DBG_INF, "%s:don't need whole chip reset in mt7990.\n", __func__);
}

int
warp_bus_msi_set(struct warp_entry *warp, struct warp_bus *bus, u8 enable)
{
	return 0;
}

#ifdef WED_HW_TX_SUPPORT
void
warp_wdma_int_sel(struct wed_entry *wed, int idx)
{
	u32 value = 0;

	/* WED_WDMA_SRC SEL */
	warp_io_read32(wed, WED_WDMA_INT_CTRL_ADDR, &value);
	value &= ~WED_WDMA_INT_CTRL_SRC_SEL_MASK;
	value |= SET_FIELD(WED_WDMA_INT_CTRL_SRC_SEL, idx);
	warp_io_write32(wed, WED_WDMA_INT_CTRL_ADDR, value);
}
#endif /* WED_HW_TX_SUPPORT */

#ifdef WED_RX_HW_RRO_3_0
inline u8
warp_get_min_rx_rro_data_ring_num(void)
{
	return 0;
}

inline u8
warp_get_min_rx_rro_page_ring_num(void)
{
	return 0;
}
#endif /* WED_RX_HW_RRO_3_0 */

u8
warp_get_pcie_slot(struct pci_dev *pdev)
{
	warp_dbg(WARP_DBG_OFF, "%s():domain_nr: %d\n", __func__, pdev->bus->domain_nr);
	return pdev->bus->domain_nr;
}

static int
warp_bus_set_cpu_mask(int idx, bool enable)
{
#define IRQ_MASK_APMCU	0x1000301c

	u32 mask_addr = IRQ_MASK_APMCU;
	u32 mask_value = BIT(idx);
	u32 value;
	void *addr;

	addr = ioremap(mask_addr, 4);
	value = readl(addr);

	if (enable)
		value |= mask_value;
	else
		value &= ~mask_value;

	writel(value, addr);
	iounmap(addr);
	return 0;
}

void
warp_bus_set_hw(struct wed_entry *wed, struct warp_bus *bus,
			int idx, bool msi_enable, u32 hif_type)
{
	u32 value = 0;

	if (msi_enable) {
		warp_io_write32(wed, WED_PCIE_CFG_ADDR_INTM_ADDR, bus->pcie_msim[idx]);
		warp_io_write32(wed, WED_PCIE_CFG_ADDR_INTS_ADDR, bus->pcie_msis[idx]);
		/* pcie interrupt status trigger register */
		warp_io_write32(wed, WED_PCIE_INTS_TRIG_ADDR, bus->pcie_msis_offset);
		warp_io_write32(wed, WED_PCIE_INTS_CLR_ADDR, bus->pcie_msis_offset);
	} else {
		warp_io_write32(wed, WED_PCIE_CFG_ADDR_INTM_ADDR, bus->pcie_intm[idx]);
		warp_io_write32(wed, WED_PCIE_CFG_ADDR_INTS_ADDR, bus->pcie_ints[idx]);
		/* pcie interrupt status trigger register */
		warp_io_write32(wed, WED_PCIE_INTS_TRIG_ADDR, bus->pcie_ints_offset);
		warp_io_write32(wed, WED_PCIE_INTS_CLR_ADDR, bus->pcie_ints_offset);
	}

	/*set mask apmcu*/
	warp_bus_set_cpu_mask(idx, true);

	/* src setting */
	warp_io_read32(wed, WED_PCIE_INT_CTRL_ADDR, &value);
	value |= SET_FIELD(WED_PCIE_INT_CTRL_SRC_SEL, idx);
	warp_io_write32(wed, WED_PCIE_INT_CTRL_ADDR, value);
}

void
warp_bus_reset_hw(struct wed_entry *wed, struct warp_bus *bus,
			int idx)
{
	warp_bus_set_cpu_mask(idx, false);
}

void
bus_setup(struct warp_bus *bus)
{
#define PCIE_BASE_ADDR0 0x11280000
	bus->pcie_base[0] = PCIE_BASE_ADDR0;

	bus->pcie_intm[0] = bus->pcie_base[0] | 0x180;
	bus->pcie_ints[0] = bus->pcie_base[0] | 0x184;

	bus->pcie_msim[0] = bus->pcie_base[0] | 0xC08;
	bus->pcie_msis[0] = bus->pcie_base[0] | 0xC04;

	bus->pcie_ints_offset = (1 << 24);
	bus->pcie_msis_offset = (1 << 8); // for func0 (wifi)
	bus->trig_flag = IRQF_TRIGGER_HIGH;
}

void
warp_eint_ctrl_hw(struct wed_entry *wed, u8 enable)
{
	u32 value = 0;

	if (enable)
		value = wed->ext_int_mask;

	warp_io_write32(wed, WED_EX_INT_MSK_ADDR, value);
}

int
warp_get_wdma_port(struct wdma_entry *wdma, u8 idx)
{
	int rx_port, tx_port;

	rx_port = hnat_get_wdma_rx_port(idx);
	if (rx_port < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): wdma_idx = %u, rx_port err = %d\n",
			__func__, idx, rx_port);
		return rx_port;
	}

	tx_port = hnat_get_wdma_tx_port(idx);
	if (tx_port < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): wdma_idx = %u, tx_port err = %d\n",
			__func__, idx, tx_port);
		return tx_port;
	}

	wdma->wdma_rx_port = rx_port;
	wdma->wdma_tx_port = tx_port;

	warp_dbg(WARP_DBG_INF,
		"%s(): wdma_idx = %u, rx_port = 0x%x, tx_port = 0x%x\n",
		__func__, idx, rx_port, tx_port);

	return 0;
}
