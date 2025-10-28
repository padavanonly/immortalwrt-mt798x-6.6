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
	warp_mt7622.c
*/

#include <linux/pci.h>
#include <warp.h>
#include "wed_hw.h"

void
warp_bus_set_hw(struct wed_entry *wed, struct warp_bus *bus,
			int idx, bool msi_enable, u32 hif_type)
{
	u32 value = bus->pcie_base[idx];

	warp_io_write32(wed, WED_PCIE_CFG_BASE, value);
	/*pcie interrupt agent control*/
#ifdef WED_WORK_AROUND_INT_POLL
	value = (PCIE_POLL_MODE_ALWAYS << WED_PCIE_INT_CTRL_FLD_POLL_EN);
	warp_io_write32(wed, WED_PCIE_INT_CTRL, value);
#endif /*WED_WORK_AROUND_INT_POLL*/
}

void
bus_setup(struct warp_bus *bus)
{
#define PCIE_BASE_ADDR0 0x1A143000
#define PCIE_BASE_ADDR1 0x1A145000
	bus->pcie_base[0] = PCIE_BASE_ADDR0;
	bus->pcie_base[1] = PCIE_BASE_ADDR1;
	bus->pcie_intm[0] = bus->pcie_base[0] | 0x420;
	bus->pcie_intm[1] = bus->pcie_base[1] | 0x420;
	bus->pcie_ints[0] = bus->pcie_base[0] | 0x424;
	bus->pcie_ints[1] = bus->pcie_base[1] | 0x424;
	/*MT7622 MSI PCIE setting need to check*/
	bus->pcie_msim[0] = 0;
	bus->pcie_msim[1] = 0;
	bus->pcie_msis[0] = 0;
	bus->pcie_msis[1] = 0;
	/*default valule will run time overlap it*/
	bus->wpdma_base[0] = WPDMA_BASE_ADDR0;
	bus->wpdma_base[1] = WPDMA_BASE_ADDR1;
	bus->pcie_ints_offset = (1 << 16);
	bus->pcie_msis_offset = (1 << 8);
	bus->trig_flag = IRQF_TRIGGER_LOW;
}

u8
warp_get_pcie_slot(struct pci_dev *pdev)
{
	return (pdev->bus->self->devfn >> 3) & 0x1f;
}

int
warp_get_wdma_port(struct wdma_entry *wdma, u8 idx)
{
	switch (idx) {
	case WARP_WED0:
		wdma->wdma_rx_port = WDMA_PORT3;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
