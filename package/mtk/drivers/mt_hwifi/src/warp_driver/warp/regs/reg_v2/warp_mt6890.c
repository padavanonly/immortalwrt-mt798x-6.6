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
	warp_mt6890.c
*/
#include <warp.h>
#include <warp_hw.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include "../coda/reg_v2/wox_mcu_cfg_hs.h"
#include "../coda/reg_v2/wox_mcu_cfg_ls.h"
#ifdef CONFIG_WARP_WO_EMBEDDED_LOAD
#include "WO0_firmware.h"
#include "WO1_firmware.h"
#endif	/* CONFIG_WARP_WO_EMBEDDED_LOAD */


#define WOCPU0_EMI_DEV_NODE "mediatek,wocpu0_emi"
#define WOCPU1_EMI_DEV_NODE "mediatek,wocpu1_emi"
#define WOCPU_SYSRAM_DEV_NODE "mediatek,wocpu_sysram"
#define WOCPU_DLM_DEV_NODE "mediatek,wocpu_dlm"
#define WOCPU_MCU_VIEW_MIOD_BASE_ADDR	0x2000
#define WOCPU0_MCUSYS_APSRC_ADDR	0x15195070
#define WOCPU1_MCUSYS_APSRC_ADDR	0x15195074
#define WOCPU_MCUSYS_RESET_ADDR		0x15000034
#define WOCPU_MCUSYS_RESET_MASK		0x10

/*
*
*/
void
warp_whole_chip_wo_reset(void)
{
	unsigned long addr;
	u32 value = 0;

	warp_dbg(WARP_DBG_OFF, "%s: --->\n", __func__);

	addr = (unsigned long)ioremap(WOCPU_MCUSYS_RESET_ADDR, 4);
	value = readl((void *)addr);
	value |= WOCPU_MCUSYS_RESET_MASK;
	writel(value, (void *)addr);
	value &= ~WOCPU_MCUSYS_RESET_MASK;
	writel(value, (void *)addr);
	iounmap((void *)addr);

	warp_dbg(WARP_DBG_OFF, "%s: <---\n", __func__);
}

/*
*
*/
void
warp_wo_reset(u8 wed_idx)
{
	if (warp_get_ref() == 0)
		warp_whole_chip_wo_reset();
}

/*
*
*/
void
warp_wo_set_apsrc_idle(u8 wed_idx)
{
	unsigned long addr;
	u32 value = 0;

	warp_dbg(WARP_DBG_OFF, "%s: --->\n", __func__);

	addr = (wed_idx == WARP_WED0)
		? (unsigned long)ioremap(WOCPU0_MCUSYS_APSRC_ADDR, 4)
		: (unsigned long)ioremap(WOCPU1_MCUSYS_APSRC_ADDR, 4);
	value = readl((void *)addr);
	if (wed_idx == WARP_WED0) {
		value |= WOX_MCU_CFG_HS_WO0_APSRC_WO0_APSRC_DDREN_MASK;
		value |= WOX_MCU_CFG_HS_WO0_APSRC_WO0_APSRC_REQ_MASK;
		value |= WOX_MCU_CFG_HS_WO0_APSRC_WO0_APSRC_IDLE_MASK;
	} else {
		value |= WOX_MCU_CFG_HS_WO1_APSRC_WO1_APSRC_DDREN_MASK;
		value |= WOX_MCU_CFG_HS_WO1_APSRC_WO1_APSRC_REQ_MASK;
		value |= WOX_MCU_CFG_HS_WO1_APSRC_WO1_APSRC_IDLE_MASK;
	}
	writel(value, (void *)addr);
	iounmap((void *)addr);

	warp_dbg(WARP_DBG_OFF, "%s: <---\n", __func__);
}


#ifdef WED_HW_TX_SUPPORT
/*
*
*/
void
warp_wdma_int_sel(struct wed_entry *wed, int idx)
{
	u32 value = 0;

	/* WED_WDMA_SRC SEL */
	warp_io_read32(wed, WED_WDMA_INT_CTRL, &value);
	/* setting for wdma_int0->wdma0, wdma_int1->wdma1 */
	value &= ~(WED_WDMA_INT_CTRL_FLD_SRC_SEL_MASK << WED_WDMA_INT_CTRL_FLD_SRC_SEL);
	warp_io_write32(wed, WED_WDMA_INT_CTRL, value);
}
#endif /* WED_HW_TX_SUPPORT */

#ifdef WED_RX_D_SUPPORT
/*
*
*/
int
warp_wed_rro_init(struct wed_entry *wed)
{
	struct wed_rro_ctrl *rro_ctrl = &wed->res_ctrl.rx_ctrl.rro_ctrl;
	struct warp_entry *warp = wed->warp;
	struct wifi_hw *hw = &warp->wifi.hw;
	struct device_node *node = of_find_compatible_node(NULL, NULL, WOCPU_DLM_DEV_NODE);
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
#endif /* WED_RX_D_SUPPORT */

#ifdef WED_RX_HW_RRO_2_0
#ifdef CONFIG_WARP_WO_EMBEDDED_LOAD
/*
*
*/
u8 *
warp_get_wo_bin_ptr(u8 wed_idx)
{
	if (wed_idx == WARP_WED1)
		return WO1_FirmwareImage;
	else
		return WO0_FirmwareImage;
}

/*
*
*/
u32
warp_get_wo_bin_size(u8 wed_idx)
{
	if (wed_idx == WARP_WED1)
		return sizeof(WO1_FirmwareImage);
	else
		return sizeof(WO0_FirmwareImage);
}
#endif /* CONFIG_WARP_WO_EMBEDDED_LOAD */

/*
*
*/
void
warp_get_dts_idx(u8 *dts_idx)
{
	*dts_idx = WARP_WED0;
}

/*
*  WO PC and LR dump
*/
void
warp_wo_pc_lr_cr_dump(u8 wed_idx)
{
	return;
}

/*
*
*/
char *
warp_get_wo_emi_node(u8 wed_idx)
{
	return (wed_idx == WARP_WED0 ? WOCPU0_EMI_DEV_NODE : WOCPU1_EMI_DEV_NODE);
}

/*
*
*/
char *
warp_get_wo_ilm_node(u8 wed_idx)
{
	return WOCPU_SYSRAM_DEV_NODE;
}

/*
*
*/
void
warp_fwdl_reset(struct warp_fwdl_ctrl *ctrl, bool en, u8 wed_idx)
{
	u32 value = 0;
	u32 mask = (wed_idx == WARP_WED0) ?
		WOX_MCU_CFG_LS_WF_MCU_CFG_WM_WA_WM_CPU_RSTB_MASK : WOX_MCU_CFG_LS_WF_MCU_CFG_WM_WA_WA_CPU_RSTB_MASK;

	/* clear UART_DBG_STOP before WOCPU software reset release */
	fwdl_io_write32(ctrl, WOX_MCU_CFG_LS_WF_MCCR_CLR_ADDR, 0xc00);

	fwdl_io_read32(ctrl, WOX_MCU_CFG_LS_WF_MCU_CFG_WM_WA_ADDR, &value);
	if (en)
		value |= mask;
	else
		value &= ~mask;
	fwdl_io_write32(ctrl, WOX_MCU_CFG_LS_WF_MCU_CFG_WM_WA_ADDR, value);
}

/*
*
*/
void
warp_fwdl_write_start_address(struct warp_fwdl_ctrl *ctrl, u32 addr, u8 wed_idx)
{
	u32 boot_cr = (wed_idx == WARP_WED0) ?
		WOX_MCU_CFG_LS_WM_BOOT_ADDR_ADDR : WOX_MCU_CFG_LS_WA_BOOT_ADDR_ADDR;
	fwdl_io_write32(ctrl, boot_cr, addr >> 16);
}

/*
*
*/
void
warp_fwdl_get_wo_heartbeat(struct warp_fwdl_ctrl *ctrl, u32 *hb_val, u8 wed_idx)
{
	u32 addr = (wed_idx == WARP_WED0) ?
		WOX_MCU_CFG_LS_WF_COM_REG0_ADDR : WOX_MCU_CFG_LS_WF_COM_REG1_ADDR;
	fwdl_io_read32(ctrl, addr, hb_val);
}
#endif /* WED_RX_HW_RRO_2_0 */

/*
*
*/
u8
warp_get_pcie_slot(struct pci_dev *pdev)
{
	return (!strcmp(pdev->bus->name, "PCI Bus 0001:01")) ? 1 : 0;
}

/*
*  set MSI group# to WED
*/
int
warp_bus_msi_set(struct warp_entry *warp, struct warp_bus *bus, u8 enable)
{
	return 0;
}

/*
*
*/
void
warp_bus_set_hw(struct wed_entry *wed, struct warp_bus *bus,
			int idx, bool msi_enable, u32 hif_type)
{
#define INFRAM_DEVICE_NODE "mediatek,infracfg_ao"
#define INFRA_INT_MASK 0xc80
	struct device_node *node = of_find_compatible_node(NULL, NULL, INFRAM_DEVICE_NODE);
	struct infra_dts {
		unsigned long base_addr;
	};
	struct infra_dts infra;
	u32 value = 0;

	if (node == NULL)
		warp_dbg(WARP_DBG_ERR, "%s(): get device_node faill\n", __func__);

	/* bus interrupt control */
	warp_io_read32(wed, WED_WPDMA_INT_CTRL, &value);
	value |= (1 << WED_WPDMA_INT_CTRL_FLD_SUBRT_ADV);
	/* disable due to interrupt lost issue should not happen in ASIC */
	/* value |= (1 << WED_WPDMA_INT_CTRL_FLD_MSK_ON_DLY); */
	warp_io_write32(wed, WED_WPDMA_INT_CTRL, value);

	if (msi_enable) {
		warp_io_write32(wed, WED_PCIE_CFG_ADDR_INTM, bus->pcie_msim[idx]);
		warp_io_write32(wed, WED_PCIE_CFG_ADDR_INTS, bus->pcie_msis[idx]);
		/* pcie interrupt status trigger register */
		warp_io_write32(wed, WED_PCIE_INTS_TRIG, bus->pcie_msis_offset);
	} else {
		warp_io_write32(wed, WED_PCIE_CFG_ADDR_INTM, bus->pcie_intm[idx]);
		warp_io_write32(wed, WED_PCIE_CFG_ADDR_INTS, bus->pcie_ints[idx]);
		/* pcie interrupt status trigger register */
		warp_io_write32(wed, WED_PCIE_INTS_TRIG, bus->pcie_ints_offset);
	}

	/* pcie interrupt agent control */
#ifdef WED_WORK_AROUND_INT_POLL
	value = (PCIE_POLL_MODE_ALWAYS << WED_PCIE_INT_CTRL_FLD_POLL_EN);
	warp_io_write32(wed, WED_PCIE_INT_CTRL, value);
#endif /* WED_WORK_AROUND_INT_POLL */

	infra.base_addr = (unsigned long)of_iomap(node, 0);
	warp_io_read32(&infra, INFRA_INT_MASK, &value);
	value |= (1 << 2);
	value |= (1 << 3);
	warp_io_write32(&infra, INFRA_INT_MASK, value);

	/* pola setting */
	warp_io_read32(wed, WED_PCIE_INT_CTRL, &value);
	value |= (1 << WED_PCIE_INT_CTRL_FLD_MSK_EN_POLA);
	warp_io_write32(wed, WED_PCIE_INT_CTRL, value);

	return;
}

/*
 * Reset PCIE reg in relative to warp_bus_set_hw()
 */
void
warp_bus_reset_hw(struct wed_entry *wed, struct warp_bus *bus,
			int idx)
{
}

/*
*
*/
void
bus_setup(struct warp_bus *bus)
{
#define PCIE_BASE_ADDR0 0x11280000
#define PCIE_BASE_ADDR1 0x11288000
	bus->pcie_base[0] = PCIE_BASE_ADDR0;
	bus->pcie_base[1] = PCIE_BASE_ADDR1;

	bus->pcie_intm[0] = bus->pcie_base[0] | 0x180;
	bus->pcie_intm[1] = bus->pcie_base[1] | 0x180;
	bus->pcie_ints[0] = bus->pcie_base[0] | 0x184;
	bus->pcie_ints[1] = bus->pcie_base[1] | 0x184;

	bus->pcie_msim[0] = bus->pcie_base[0] | 0xC08;
	bus->pcie_msim[1] = bus->pcie_base[1] | 0xC08;
	bus->pcie_msis[0] = bus->pcie_base[0] | 0xC04;
	bus->pcie_msis[1] = bus->pcie_base[1] | 0xC04;

	/* default valule will run time overlap it */
	bus->wpdma_base[0] = WPDMA_BASE_ADDR0;
	bus->wpdma_base[1] = WPDMA_BASE_ADDR1;

	bus->pcie_ints_offset = (1 << 24);
	bus->pcie_msis_offset = (1 << 8);
	bus->trig_flag = IRQF_TRIGGER_HIGH;

	return;
}

/*
*
*/
void
warp_eint_ctrl_hw(struct wed_entry *wed, u8 enable)
{
	u32 value = 0;

	if (enable)
		value = wed->ext_int_mask;

	warp_io_write32(wed, WED_EX_INT_MSK, value);

#ifdef WED_RX_D_SUPPORT
	if (enable) {
		warp_io_write32(wed, WED_EX_INT_MSK1, wed->ext_int_mask1);
		warp_io_write32(wed, WED_EX_INT_MSK2, wed->ext_int_mask2);
	} else {
		warp_io_write32(wed, WED_EX_INT_MSK1, 0);
		warp_io_write32(wed, WED_EX_INT_MSK2, 0);
	}
#endif
}

int
warp_get_wdma_port(struct wdma_entry *wdma, u8 idx)
{
	switch (idx) {
	case WARP_WED0:
		wdma->wdma_tx_port = WDMA_PORT3;
		wdma->wdma_rx_port = WDMA_PORT8;
		break;
	case WARP_WED1:
		wdma->wdma_tx_port = WDMA_PORT4;
		wdma->wdma_rx_port = WDMA_PORT9;
		break;
	case WARP_WED2:
		wdma->wdma_tx_port = WDMA_PORT3;
		wdma->wdma_rx_port = WDMA_PORT13;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
