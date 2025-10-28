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
#include <linux/msi.h>
#include "wed_hw.h"
#include "pcie_mac.h"
#ifdef WED_RX_HW_RRO_2_0
#include "wox_mcu_cfg_hs.h"
#include "wox_mcu_cfg_ls.h"
#include "wox_mcu_cfg_on.h"
#ifdef CONFIG_WARP_WO_EMBEDDED_LOAD
#include "WO0_firmware.h"
#endif

#define WOCPU0_EMI_DEV_NODE			"mediatek,wocpu0_emi"
#define WOCPU0_ILM_DEV_NODE			"mediatek,wocpu_ilm"
#define WOCPU0_DLM_DEV_NODE			"mediatek,wocpu_dlm"
//MIOD uses WO DLM memory
#define WOCPU_MCU_VIEW_MIOD_BASE_ADDR		0x8000
#endif /* WED_RX_HW_RRO_2_0 */

#define PCIE_DEV_NODE	"mediatek,mt6988-pcie"

void
warp_whole_chip_wo_reset(void)
{
	warp_dbg(WARP_DBG_INF, "%s:don't need whole chip reset in mt7990.\n", __func__);
}

/*
 *  WO PC and LR dump
 */
void
warp_wo_pc_lr_cr_dump(u8 wed_idx)
{
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

#ifdef WED_RX_HW_RRO_2_0
inline u8
warp_get_min_rx_data_ring_num(void)
{
	return 2;
}

void
warp_wo_reset(u8 wed_idx)
{
	unsigned long addr = 0;
	u32 value = 0;

	switch (wed_idx) {
	case WARP_WED0:
		addr = (unsigned long)ioremap(WOX_MCU_CFG_ON_BASE +
			WOX_MCU_CFG_ON_WOX_MCU_CFG_WO0_WO1_ADDR, 4);
		value = readl((void *)addr);
		value |= WOX_MCU_CFG_ON_WOX_MCU_CFG_WO0_WO1_WO0_CPU_RSTB_MASK;
		writel(value, (void *)addr);
		value &= ~WOX_MCU_CFG_ON_WOX_MCU_CFG_WO0_WO1_WO0_CPU_RSTB_MASK;
		writel(value, (void *)addr);
		iounmap((void *)addr);
		break;
	default:
		warp_dbg(WARP_DBG_ERR, "%s(): wrong wed_idx=%d!\n", __func__, wed_idx);
	}
}

void
warp_wo_set_apsrc_idle(u8 wed_idx)
{
	unsigned long addr, addr_remap;
	u32 value = 0;

	switch (wed_idx) {
	case WARP_WED0:
		addr = (WOX_MCU_CFG_LS_BASE + WOX_MCU_CFG_LS_WO0_APSRC_ADDR);
		addr_remap = (unsigned long)ioremap(addr, 4);
		value = readl((void *)addr_remap);
		value |= WOX_MCU_CFG_LS_WO0_APSRC_WO0_APSRC_DDREN_MASK;
		value |= WOX_MCU_CFG_LS_WO0_APSRC_WO0_APSRC_REQ_MASK;
		value |= WOX_MCU_CFG_LS_WO0_APSRC_WO0_APSRC_IDLE_MASK;
		writel(value, (void *)addr_remap);
		iounmap((void *)addr_remap);
		break;
	default:
		warp_dbg(WARP_DBG_ERR, "%s(): wrong wed_idx=%d!\n", __func__, wed_idx);
	}
}

/*
 *
 */
int
warp_wed_rro_init(struct wed_entry *wed)
{
	struct wed_rro_ctrl *rro_ctrl = &wed->res_ctrl.rx_ctrl.rro_ctrl;
	struct warp_entry *warp = wed->warp;
	struct wifi_hw *hw = &warp->wifi.hw;
	struct device_node *node = of_find_compatible_node(NULL, NULL, WOCPU0_DLM_DEV_NODE);
	struct resource res;
	int rc = 0;
	u32 len = 0;

	rro_ctrl->miod_cnt = MIOD_CNT;
	rro_ctrl->mod_size = 16;
	rro_ctrl->mid_size = (rro_ctrl->mod_size + hw->max_rxd_size);
	rro_ctrl->miod_entry_size = 128;
	rro_ctrl->fdbk_cnt = FB_CMD_CNT;
	rro_ctrl->rro_que_cnt = RRO_QUE_CNT;
	rro_ctrl->miod_desc_base_mcu_view = WOCPU_MCU_VIEW_MIOD_BASE_ADDR;

	/* get MID/MOD from device tree */
	rc = of_address_to_resource(node, warp->idx, &res);
	if (rc)
		goto err0;

	rro_ctrl->miod_desc_base_pa = res.start;
	len = rro_ctrl->miod_entry_size * rro_ctrl->miod_cnt;
	rro_ctrl->fdbk_desc_base_pa = rro_ctrl->miod_desc_base_pa + len;

	/* allocate RRO queue */
	len = hw->rxd_size * rro_ctrl->rro_que_cnt;
	if (warp_dma_buf_alloc(wed->pdev, &rro_ctrl->rro_que, len) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): allocate rro queue fail!\n", __func__);
		goto err0;
	}

	rro_ctrl->rro_que_base = rro_ctrl->rro_que.alloc_va;
	rro_ctrl->rro_que_base_pa = rro_ctrl->rro_que.alloc_pa;

	return 0;

err0:
	return -1;
}

#ifdef CONFIG_WARP_WO_EMBEDDED_LOAD
u8 *
warp_get_wo_bin_ptr(u8 wed_idx)
{
	if (wed_idx == WARP_WED0)
		return WO0_FirmwareImage;
	else
		return NULL;
}

u32
warp_get_wo_bin_size(u8 wed_idx)
{
	if (wed_idx == WARP_WED0)
		return sizeof(WO0_FirmwareImage);
	else
		return 0;
}
#endif /* CONFIG_WARP_WO_EMBEDDED_LOAD */

void
warp_get_dts_idx(u8 *dts_idx)
{
	*dts_idx = WARP_WED0;
}

char *
warp_get_wo_emi_node(u8 wed_idx)
{
	if (wed_idx == WARP_WED0)
		return WOCPU0_EMI_DEV_NODE;
	else
		return NULL;
}

char *
warp_get_wo_ilm_node(u8 wed_idx)
{
	if (wed_idx == WARP_WED0)
		return WOCPU0_ILM_DEV_NODE;
	else
		return NULL;
}

void
warp_fwdl_reset(struct warp_fwdl_ctrl *ctrl, bool en, u8 wed_idx)
{
	u32 value = 0;
	unsigned long addr = 0;

	/* clear UART_DBG_STOP before WOCPU software reset release */
	addr = (unsigned long)ioremap(WOX_MCU_CFG_ON_BASE +
		WOX_MCU_CFG_ON_WOX_MCCR_CLR_ADDR, 4);

	writel(0xc00, (void *)addr);
	iounmap((void *)addr);

	addr = (unsigned long)ioremap(WOX_MCU_CFG_ON_BASE +
		WOX_MCU_CFG_ON_WOX_MCU_CFG_WO0_WO1_ADDR, 4);
	value = readl((void *)addr);
	if (en)
		value |= WOX_MCU_CFG_ON_WOX_MCU_CFG_WO0_WO1_WO0_CPU_RSTB_MASK;
	else
		value &= ~WOX_MCU_CFG_ON_WOX_MCU_CFG_WO0_WO1_WO0_CPU_RSTB_MASK;
	writel(value, (void *)addr);
	iounmap((void *)addr);

}

void
warp_fwdl_write_start_address(struct warp_fwdl_ctrl *ctrl, u32 addr, u8 wed_idx)
{
	fwdl_io_write32(ctrl, WOX_MCU_CFG_HS_WO0_BOOT_ADDR_ADDR, addr >> 16);
}

void
warp_fwdl_get_wo_heartbeat(struct warp_fwdl_ctrl *ctrl, u32 *hb_val, u8 wed_idx)
{
	u32 value = 0;
	unsigned long addr = 0;

	addr = (unsigned long)ioremap(WOX_MCU_CFG_LS_BASE + WOX_MCU_CFG_LS_WOX_COM_REG0_ADDR, 4);

	value = readl((void *)addr);
	*hb_val = value;
	iounmap((void *)addr);
}
#endif /* WED_RX_HW_RRO_2_0 */

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

static struct msi_msg *
warp_bus_get_msi_msg(u32 irq)
{
	struct irq_data *irq_data;
	struct msi_desc *msi_desc;

	irq_data = irq_get_irq_data(irq);
	if (!irq_data) {
		warp_dbg(WARP_DBG_ERR, "%s(): Failed to get irq_data for irq_num %u\n",
			__func__, irq);
		return NULL;
	}

	msi_desc = irq_data_get_msi_desc(irq_data);
	if (!msi_desc) {
		warp_dbg(WARP_DBG_ERR, "%s(): Failed to get msi_desc for irq_num %u\n",
			__func__, irq);
		return NULL;
	}

	return &msi_desc->msg;
}

static int
warp_bus_set_msi_grp_mask(struct warp_bus *bus, u8 idx, u8 enable)
{
	void *grp1_va;
	u32 grp1_value;
	u32 grp1_pa = bus->pcie_msim[idx];

	grp1_va = ioremap(grp1_pa, 4);
	if (!grp1_va) {
		warp_dbg(WARP_DBG_ERR, "%s(): grp1_va is NULL from address 0x%08x\n",
			__func__, grp1_pa);
		return -EINVAL;
	}

	grp1_value = readl(grp1_va);

	if (enable)
		grp1_value |= bus->pcie_msis_offset;
	else
		grp1_value &= ~bus->pcie_msis_offset;

	writel(grp1_value, grp1_va);

	iounmap(grp1_va);

	return 0;
}

int
warp_bus_msi_set(struct warp_entry *warp, struct warp_bus *bus, u8 enable)
{
	struct wifi_hw *wifi = &warp->wifi.hw;
	struct msi_msg *msi_msg;
	u8 idx = warp->idx;
	/* PCIE MSI status and mask address for WED/host access */
	u32 sts_addr = PCIE_MAC_ISTATUS_MSI_F0_ADDR;
	u32 msk_grp0_addr = PCIE_MAC_IMASK_MSI_F0_ADDR;
	u32 msk_grp1_addr = PCIE_MAC_IMASK_MSI_GRP1_F0_ADDR;
	/* Setup for finding which PCIE MSI func address from irq */
	u32 base_addr;
	u32 min_base_addr = PCIE_MAC_IMSI_LO_ADDR_F0_ADDR;
	u32 max_base_addr = PCIE_MAC_IMSI_LO_ADDR_F7_ADDR;
	u32 cmp_addr = min_base_addr;
	/* The offset between PCIE MSI func0 and func1 address */
	u32 offset = PCIE_MAC_IMSI_LO_ADDR_F1_ADDR - min_base_addr;

	if (!enable)
		goto set_grp_mask;

	msi_msg = warp_bus_get_msi_msg(wifi->irq);
	if (!msi_msg)
		return -EINVAL;

	base_addr = msi_msg->address_lo & GENMASK(11, 0);

	/* Find which PCIE MSI func address currently used */
	while (cmp_addr <= max_base_addr) {
		if (base_addr == cmp_addr)
			break;

		cmp_addr += offset;
		sts_addr += offset;
		msk_grp1_addr += offset;
		msk_grp0_addr += offset;

		if (cmp_addr > max_base_addr) {
			warp_dbg(WARP_DBG_ERR,
				"%s(): idx = %u failed to find MSI function base address\n",
				__func__, idx);
			return -EINVAL;
		}
	}

	bus->pcie_msim[idx] = bus->pcie_base[idx] | msk_grp1_addr;
	bus->pcie_msis[idx] = bus->pcie_base[idx] | sts_addr;
	bus->pcie_msis_offset =	BIT(msi_msg->data);

set_grp_mask:
	return warp_bus_set_msi_grp_mask(bus, idx, enable);
}

u8
warp_get_pcie_slot(struct pci_dev *pdev)
{
	warp_dbg(WARP_DBG_OFF, "%s():domain_nr: %d\n", __func__, pdev->bus->domain_nr);
	return pdev->bus->domain_nr;
}

static int
warp_bus_set_cpu_mask(int idx, bool enable)
{
#define PCIE0_IRQ_MASK_APMCU_ADDR	0x106A0040
#define PCIE0_IRQ_MASK_APMCU_MASK	BIT(1)
#define PCIE1_IRQ_MASK_APMCU_ADDR	0x10790040
#define PCIE1_IRQ_MASK_APMCU_MASK	BIT(30)

	u32 mask_addr, mask_value, value;
	void *addr;

	switch (idx) {
	case 0:
		mask_addr = PCIE0_IRQ_MASK_APMCU_ADDR;
		mask_value = PCIE0_IRQ_MASK_APMCU_MASK;
		break;
	case 1:
		mask_addr = PCIE1_IRQ_MASK_APMCU_ADDR;
		mask_value = PCIE1_IRQ_MASK_APMCU_MASK;
		break;
	default:
		warp_dbg(WARP_DBG_ERR, "%s(): idx = %d unsupported\n",
				__func__, idx);
		return -EINVAL;
	};

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
	u32 value;

	if (msi_enable) {
		warp_io_write32(wed, WED_PCIE_CFG_ADDR_INTM_ADDR, bus->pcie_msim[idx]);
		warp_io_write32(wed, WED_PCIE_CFG_ADDR_INTS_ADDR, bus->pcie_msis[idx]);
		/* pcie interrupt status trigger register */
		warp_io_write32(wed, WED_PCIE_INTS_TRIG_ADDR, bus->pcie_msis_offset);
		warp_io_write32(wed, WED_PCIE_INTS_CLR_ADDR, bus->pcie_msis_offset);

		warp_io_read32(wed, WED_PCIE_INT_CTRL_ADDR, &value);
		value |= WED_PCIE_INT_CTRL_IRQ_MSI_SEL_MASK;
		warp_io_write32(wed, WED_PCIE_INT_CTRL_ADDR, value);
	} else {
		warp_io_write32(wed, WED_PCIE_CFG_ADDR_INTM_ADDR, bus->pcie_intm[idx]);
		warp_io_write32(wed, WED_PCIE_CFG_ADDR_INTS_ADDR, bus->pcie_ints[idx]);
		/* pcie interrupt status trigger register */
		warp_io_write32(wed, WED_PCIE_INTS_TRIG_ADDR, bus->pcie_ints_offset);
		warp_io_write32(wed, WED_PCIE_INTS_CLR_ADDR, bus->pcie_ints_offset);

		warp_io_read32(wed, WED_PCIE_INT_CTRL_ADDR, &value);
		value &= ~WED_PCIE_INT_CTRL_IRQ_MSI_SEL_MASK;
		warp_io_write32(wed, WED_PCIE_INT_CTRL_ADDR, value);
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
	struct device_node *pcie_node;
	struct resource res;
	u32 domain;
	int i, ret;
	bool is_found;

	for (i = 0; i < bus->warp_num; i++) {
		pcie_node = NULL;
		is_found = false;

		for_each_compatible_node(pcie_node, NULL, PCIE_DEV_NODE) {
			of_property_read_u32(pcie_node, "linux,pci-domain", &domain);

			if (i == domain) {
				is_found = true;
				break;
			}
		}

		if (!is_found) {
			warp_dbg(WARP_DBG_ERR, "%s(): PCIE dev %s idx = %d not found\n",
				__func__, PCIE_DEV_NODE, i);
			continue;
		}

		ret = of_address_to_resource(pcie_node, 0, &res);
		if (ret) {
			warp_dbg(WARP_DBG_ERR, "%s(): PCIE dev %s idx = %d resource not found\n",
				__func__, PCIE_DEV_NODE, i);
			continue;
		}

		bus->base_addr[i] = of_iomap(pcie_node, 0);
		bus->pcie_base[i] = res.start;
		bus->pcie_intm[i] = bus->pcie_base[i] | PCIE_MAC_IMASK_LOCAL_ADDR;
		bus->pcie_ints[i] = bus->pcie_base[i] | PCIE_MAC_ISTATUS_LOCAL_ADDR;
	}

	bus->pcie_ints_offset = BIT(24);
	bus->trig_flag = IRQF_TRIGGER_HIGH;
}

void
warp_eint_ctrl_hw(struct wed_entry *wed, u8 enable)
{
	u32 value = 0;

	if (enable)
		value = wed->ext_int_mask;

	warp_io_write32(wed, WED_EX_INT_MSK_ADDR, value);

#ifdef WED_RX_HW_RRO_2_0
	if (enable)
		warp_io_write32(wed, WED_EX_INT_MSK1_ADDR, wed->ext_int_mask1);
	else
		warp_io_write32(wed, WED_EX_INT_MSK1_ADDR, 0);
#endif
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
