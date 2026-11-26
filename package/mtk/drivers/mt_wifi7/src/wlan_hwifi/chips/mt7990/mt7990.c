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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/firmware.h>
#include "mt7990.h"
#include "mac_if.h"
#include "core.h"
#include "chips/bmac.h"
#ifdef CONFIG_HWIFI_FW_HDR_SUPPORT
#include "mcu_hdr/WIFI_MT7990_PATCH_MCU_1_1_hdr.h"
#include "mcu_hdr/WIFI_RAM_CODE_MT7990_1_1.h"
#include "mcu_hdr/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.h"
#include "mcu_hdr/WIFI_MT7990_WACPU_RAM_CODE_1_1.h"
#endif /* CONFIG_HWIFI_FW_HDR_SUPPORT */

/*#include "firmware/mt7990_e2p_ePAeLNA.h"*/

/*TBD: not ready*/
static int mac_type = MTK_INTERFACE_CONNAC;
module_param(mac_type, int, 0644);
MODULE_PARM_DESC(mac_type, "mac driver type");

static u8 option_type = 1;
module_param(option_type, byte, 0644);
MODULE_PARM_DESC(option_type, "Option Type");

static u8 intr_option_set;
module_param(intr_option_set, byte, 0644);
MODULE_PARM_DESC(intr_option_set, "Interrupt Option Set");

static bool rro_enable = true;
module_param(rro_enable, bool, 0644);
MODULE_PARM_DESC(rro_enable, "RRO Enable");

#define MT7990_CHIP_ID 0x7990

/**
 * @defgroup eagle_wfdma_hal eagle wfdma hal
 * @ingroup wfdma
 * @{
 */

static void
dump_chip_info(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);
	u32 chipid = MT7990_CHIP_ID;

	dev_info(to_device(trans), "%s(): chip id: %x\n",
		__func__, chipid);
}

static void
_mt7990_pdma_irq_clear_int_sta(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);
	unsigned long flags;

	spin_lock_irqsave(&pci_hw->irq_lock, flags);
	bus_write(trans, pci_hw->mcu2host_sw_int_sta_addr, 0xffffffff);
	bus_write(trans, pci_hw->int_mask_map[0].int_sts_addr, 0xffffffff);
	spin_unlock_irqrestore(&pci_hw->irq_lock, flags);
}

static void
_mt7990_pdma_irq_init(struct pci_trans *pci_hw)
{
	_mt7990_pdma_irq_clear_int_sta(pci_hw);

	/* TODO: Move to suitable function */
	if (test_bit(PCI_FLAG_WHNAT_RRO_DBG, &pci_hw->flags)) {
		pci_hw->rro_mode = HW_RRO_V3_0_BUF_PG_DBG;
	} else if (test_bit(PCI_FLAG_WHNAT_RX, &pci_hw->flags)) {
		pci_hw->int_mask_map[0].int_ena_mask &=
			~WF_WFDMA_HOST_DMA0_HOST_INT_ENA_WF_RRO_IRQ_ENA_MASK;
		pci_hw->vec_data[0].int_ena_mask &=
			~WF_WFDMA_HOST_DMA0_HOST_INT_ENA_WF_RRO_IRQ_ENA_MASK;
	}
}

static void
_mt7990_pdma_irq_disable(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);
	unsigned long flags;

	spin_lock_irqsave(&pci_hw->irq_lock, flags);
	bus_write(trans, pci_hw->mcu2host_sw_int_ena_addr, 0);
	bus_write(trans, pci_hw->int_mask_map[0].int_ena_addr, 0);
	spin_unlock_irqrestore(&pci_hw->irq_lock, flags);
	clear_bit(PCI_STATE_ISR_START, &pci_hw->state);
}

static void
_mt7990_pdma_irq_enable(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);
	unsigned long flags;

	spin_lock_irqsave(&pci_hw->irq_lock, flags);
	bus_write(trans, pci_hw->int_mask_map[0].int_ena_addr,
		pci_hw->int_mask_map[0].int_ena_mask);
	bus_write(trans, pci_hw->mcu2host_sw_int_ena_addr,
		pci_hw->mcu2host_sw_int_ser_mask |
		pci_hw->mcu2host_sw_wdt_ser_mask);
	spin_unlock_irqrestore(&pci_hw->irq_lock, flags);
	clear_bit(PCI_STATE_ISR_PEDING, &pci_hw->state);
	set_bit(PCI_STATE_ISR_START, &pci_hw->state);
}

static void
_mt7990_pdma_config(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);
	struct mtk_hw_dev *hw_dev = to_hw_dev(trans);
	struct mtk_chip_drv *chip_drv = hw_dev->chip_drv;

	/* GLO_CFG */
	bus_set(trans, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR,
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_OMIT_TX_INFO_MASK |
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_PDMA_ADDR_EXT_EN_MASK);
	bus_clear(trans, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR,
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_OMIT_RX_INFO_MASK);

	/* GLO_CFG_EXT0 */
	bus_set(trans, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT0_ADDR,
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT0_CSR_RX_WB_RXD_MASK |
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT0_CSR_WED_MERGE_MODE_MASK);

	/* GLO_CFG_EXT1 */
	bus_set(trans, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT1_ADDR,
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT1_CSR_TX_FCTRL_MODE_MASK);

	if (chip_drv->chip_opt == MT7990_CHIP_OPT_AN7581) {
		bus_set(trans, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT0_CSR_WED_MERGE_MODE_ADDR,
			WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT0_CSR_WED_MERGE_MODE_MASK);
		bus_clear(trans, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_bn2_pcie_sel_ADDR,
			WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_bn2_pcie_sel_MASK);
		bus_clear(trans, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_bn1_pcie_sel_ADDR,
			WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_bn1_pcie_sel_MASK);
		bus_set(trans, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_bn0_pcie_sel_ADDR,
			WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_bn0_pcie_sel_MASK);
		bus_set(trans, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pdma_per_band_ADDR,
			WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pdma_per_band_MASK);
	}
}

static void
_mt7990_pdma_interrupt_select(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);
	struct mtk_hw_dev *hw_dev = to_hw_dev(trans);
	struct mtk_chip_drv *chip_drv = hw_dev->chip_drv;

#define WF_WFDMA_HOST_DMA0_HOST_TX_INT_PCIE_SEL_ADDR_TX_RING_21_MASK BIT(21)
#define WF_WFDMA_HOST_DMA0_HOST_TX_INT_PCIE_SEL_ADDR_TX_RING_22_MASK BIT(22)
#define WF_WFDMA_HOST_DMA0_HOST_RX_INT_PCIE_SEL_ADDR_RX_RING_3_MASK BIT(3)
#define WF_WFDMA_HOST_DMA0_HOST_RX_INT_PCIE_SEL_ADDR_RX_RING_5_MASK BIT(5)
#define WF_WFDMA_HOST_DMA0_HOST_RX_INT_PCIE_SEL_ADDR_RX_RING_6_MASK BIT(6)
#define WF_WFDMA_HOST_DMA0_HOST_RX_INT_PCIE_SEL_ADDR_RX_RING_7_MASK BIT(7)

	/* Redirect interrupt to PCIE1 */
	if (test_bit(HWIFI_FLAG_MULTI_BUS, &hw_dev->flags)) {
		bus_set(trans, WF_WFDMA_HOST_DMA0_HOST_TX_INT_PCIE_SEL_ADDR,
			WF_WFDMA_HOST_DMA0_HOST_TX_INT_PCIE_SEL_ADDR_TX_RING_21_MASK);
		bus_set(trans, WF_WFDMA_HOST_DMA0_HOST_RX_INT_PCIE_SEL_ADDR,
			WF_WFDMA_HOST_DMA0_HOST_RX_INT_PCIE_SEL_ADDR_RX_RING_3_MASK);
	}
	if (chip_drv->chip_opt == MT7990_CHIP_OPT_AN7581) {
		bus_set(trans, WF_WFDMA_HOST_DMA0_HOST_TX_INT_PCIE_SEL_ADDR,
			WF_WFDMA_HOST_DMA0_HOST_TX_INT_PCIE_SEL_ADDR_TX_RING_22_MASK);
		bus_set(trans, WF_WFDMA_HOST_DMA0_HOST_RX_INT_PCIE_SEL_ADDR,
			WF_WFDMA_HOST_DMA0_HOST_RX_INT_PCIE_SEL_ADDR_RX_RING_5_MASK |
			WF_WFDMA_HOST_DMA0_HOST_RX_INT_PCIE_SEL_ADDR_RX_RING_6_MASK |
			WF_WFDMA_HOST_DMA0_HOST_RX_INT_PCIE_SEL_ADDR_RX_RING_7_MASK);
	}
}

static void
_mt7990_pdma_rx_threshold(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);

	/* RRO related, pause PP if lower than DMAD threshold */
	/* WFDMA */
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH54_ADDR,
		  0xc000c);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH76_ADDR,
		  0x10008);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH98_ADDR,
		  0x10008);

	/* RRO */
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RRO_Q_ADDR,
		  0x20);
}

static void
_mt7990_pdma_prefetch_config(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);

	/* Rx */
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_EXT_CTRL_ADDR, 0x00000002);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING1_EXT_CTRL_ADDR, 0x00200002);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_EXT_CTRL_ADDR, 0x00400002);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING3_EXT_CTRL_ADDR, 0x00600002);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_EXT_CTRL_ADDR, 0x00800010);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_EXT_CTRL_ADDR, 0x01800010);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING6_EXT_CTRL_ADDR, 0x02800010);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING7_EXT_CTRL_ADDR, 0x03800004);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING8_EXT_CTRL_ADDR, 0x03C00010);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING9_EXT_CTRL_ADDR, 0x04C00004);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING10_EXT_CTRL_ADDR, 0x05000004);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING11_EXT_CTRL_ADDR, 0x05400004);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING12_EXT_CTRL_ADDR, 0x05800004);

	/* Tx */
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_EXT_CTRL_ADDR, 0x00000000);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING1_EXT_CTRL_ADDR, 0x00000000);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING2_EXT_CTRL_ADDR, 0x00000000);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING3_EXT_CTRL_ADDR, 0x00000000);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING4_EXT_CTRL_ADDR, 0x00000000);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING5_EXT_CTRL_ADDR, 0x00000000);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING6_EXT_CTRL_ADDR, 0x00000000);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING16_EXT_CTRL_ADDR, 0x05E00002);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING17_EXT_CTRL_ADDR, 0x05C00002);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING18_EXT_CTRL_ADDR, 0x05E00008);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING19_EXT_CTRL_ADDR, 0x06600008);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING20_EXT_CTRL_ADDR, 0x06E00002);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING21_EXT_CTRL_ADDR, 0x07000008);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING22_EXT_CTRL_ADDR, 0x07800008);
}

static void
_mt7990_pdma_reset_didx(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);

	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_RST_DTX_PTR_ADDR, 0xffffffff);
	bus_write(trans, WF_WFDMA_HOST_DMA0_WPDMA_RST_DRX_PTR_ADDR, 0xffffffff);
}

static void
_mt7990_pdma_init(struct pci_trans *pci_hw)
{
	_mt7990_pdma_config(pci_hw);
	_mt7990_pdma_interrupt_select(pci_hw);
	_mt7990_pdma_rx_threshold(pci_hw);
	_mt7990_pdma_prefetch_config(pci_hw);
	_mt7990_pdma_reset_didx(pci_hw);
}

static void
_mt7990_pdma_unpause(struct pci_trans *pci_hw)
{
	bus_write(to_bus_trans(pci_hw), pci_hw->pause_trxq_reg_addr, 0);
}

static void
_mt7990_pdma_pause(struct pci_trans *pci_hw)
{
}

static bool
_mt7990_pdma_poll(struct pci_trans *pci_hw, u32 round, u32 wait_us)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);
	u32 val;
	u8 i = 0;
	bool idle = false;

	bus_set(trans, WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_ADDR,
		WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_conn_hif_rxfifo0_busy_enable_MASK |
		WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_conn_hif_rxfifo1_busy_enable_MASK |
		WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_conn_hif_rxfifo2_busy_enable_MASK |
		WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_conn_hif_txfifo0_busy_enable_MASK |
		WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_conn_hif_txfifo1_busy_enable_MASK);

	do {
		val = bus_read(trans, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HIF_MISC_ADDR);
		if ((val & WF_WFDMA_EXT_WRAP_CSR_WFDMA_HIF_MISC_HIF_BUSY_MASK) == 0) {
			idle = true;
			break;
		}

		udelay(wait_us);
	} while ((i++) < round);

	return !idle;
}

static void
_mt7990_pdma_enable(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);

	bus_set(trans, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR,
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_MASK |
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK);
}

static void
_mt7990_pdma_disable(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);

	bus_clear(trans, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR,
			WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_MASK |
			WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK);
}

static int
mt7990_pdma_enable(struct pci_trans *trans)
{
	_mt7990_pdma_irq_enable(trans);
	_mt7990_pdma_unpause(trans);
	_mt7990_pdma_enable(trans);

	return 0;
}

static int
mt7990_pdma_disable(struct pci_trans *trans)
{
#define PDMA_POLL_ROUND 100
#define PDMA_POLL_US 1000
	int ret;

	_mt7990_pdma_disable(trans);
	_mt7990_pdma_pause(trans);
	ret = _mt7990_pdma_poll(trans, PDMA_POLL_ROUND, PDMA_POLL_US);
	if (ret)
		dev_err(to_device(trans), "%s(): Polling to idle failed\n", __func__);

	_mt7990_pdma_irq_disable(trans);

	return ret;
}

static int
mt7990_pdma_init(struct pci_trans *trans)
{
	dump_chip_info(trans);
	_mt7990_pdma_init(trans);
	_mt7990_pdma_irq_init(trans);

	return 0;
}

/** @} */

static int
mt7990_rro_hw_init(struct pci_trans *trans)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	u32 value = 0;
	u16 i;
	u32 cr_addr = WF_RRO_TOP_ADDR_ELEM_SEG_ADDR_0_ADDR;

	/* TODO: remove line after WM has set */
	bus_clear(to_bus_trans(trans),
		  WF_RRO_TOP_AXI_MST_CFG_ADDR,
		  WF_RRO_TOP_AXI_MST_CFG_DIDX_WAIT_BRSP_OK_MASK);

	/* setup BA bitmap cache address */
	bus_write(to_bus_trans(trans),
		WF_RRO_TOP_BA_BITMAP_BASE_0_ADDR,
		rro_cfg->ba_bitmap_cache_pa[0]);
	bus_write(to_bus_trans(trans),
		WF_RRO_TOP_BA_BITMAP_BASE_1_ADDR, 0);
	bus_write(to_bus_trans(trans),
		WF_RRO_TOP_BA_BITMAP_BASE_EXT0_ADDR,
		rro_cfg->ba_bitmap_cache_pa[1]);
	bus_write(to_bus_trans(trans),
		WF_RRO_TOP_BA_BITMAP_BASE_EXT1_ADDR, 0);

	/* setup Address element address */
	for (i = 0; i < rro_cfg->addr_elem_cr_cnt; i++) {
		bus_write(to_bus_trans(trans), cr_addr,
			rro_cfg->addr_elem_alloc_pa[i] >> 4);
		cr_addr += 4;
	}

	/* setup Address element address - separate address segment mode */
	bus_set(to_bus_trans(trans),
		WF_RRO_TOP_ADDR_ARRAY_BASE_1_ADDR,
		WF_RRO_TOP_ADDR_ARRAY_BASE_1_ELEM_ADDR_SEG_MODE_MASK);

	bus_write(to_bus_trans(trans),
		WF_RRO_TOP_IND_CMD_SIGNATURE_BASE_0_ADDR,
		0);
	bus_write(to_bus_trans(trans),
		WF_RRO_TOP_IND_CMD_SIGNATURE_BASE_1_ADDR,
		0);

	/* particular session configure */
	/* use max session idx + 1 as particular session id */
	bus_write(to_bus_trans(trans),
		WF_RRO_TOP_PARTICULAR_CFG_0_ADDR,
		rro_cfg->particular_session_pa);

	value = rro_cfg->max_rro_se_cnt
		<< WF_RRO_TOP_PARTICULAR_CFG_1_PARTICULAR_SESSION_ID_SHFT;
	value |= WF_RRO_TOP_PARTICULAR_CFG_1_PARTICULAR_CONFG_EN_MASK;
	bus_write(to_bus_trans(trans),
		WF_RRO_TOP_PARTICULAR_CFG_1_ADDR, value);

	/* interrupt enable */
	bus_set(to_bus_trans(trans),
		WF_RRO_TOP_HOST_INT_ENA_ADDR,
		WF_RRO_TOP_HOST_INT_ENA_HOST_RRO_DONE_ENA_MASK);

	return 0;
}

static bool
mt7990_pdma_match(struct pci_trans *master, struct pci_trans *slave)
{
	u32 master_rid = 0;
	u32 slave_rid = 0;
	u32 i;

	if (!master || !slave)
		return false;

	master_rid = get_rid_value(master);

	if (master_rid == DEFAULT_RID) {
		master_rid = master->parent.sid.idx;
		set_rid_value(master, master->parent.sid.idx);
		/* make sure rid value is written before read */
		wmb();
	}

	for (i = 0; i < 5; i++) {
		slave_rid = get_rid_value(slave);

		if (slave_rid != master_rid)
			udelay(1);
		else
			break;
	}

	return slave_rid == master_rid;
}

static u32
mt7990_chip_option_set_nums(u32 src_nums, u32 default_nums)
{
	if (src_nums)
		return (src_nums == MT7990_ZERO_NUM) ? 0 : src_nums;
	return default_nums;
}

static void
mt7990_chip_option_set_txrx_nums(struct mtk_hw_dev *hw_dev,
	struct mtk_bus_desc *bus_desc)
{
	struct mtk_hwres_cap *hwres = hw_dev->chip_drv->hw_caps->hwres;
	u32 src_nums, default_nums;

	src_nums = bus_desc->ext_tx_tk_nums_master;
	default_nums = hwres->ext_tx_tk_nums_master;
	hw_dev->ext_tx_tk_nums_master =
		mt7990_chip_option_set_nums(src_nums, default_nums);

	src_nums = bus_desc->ext_tx_tk_nums_slave;
	default_nums = hwres->ext_tx_tk_nums_slave;
	hw_dev->ext_tx_tk_nums_slave =
		mt7990_chip_option_set_nums(src_nums, default_nums);

	src_nums = bus_desc->ext_rx_tk_nums_master;
	default_nums = hwres->ext_rx_tk_nums_master;
	hw_dev->ext_rx_tk_nums_master =
		mt7990_chip_option_set_nums(src_nums, default_nums);

	src_nums = bus_desc->ext_rx_tk_nums_slave;
	default_nums = hwres->ext_rx_tk_nums_slave;
	hw_dev->ext_rx_tk_nums_slave =
		mt7990_chip_option_set_nums(src_nums, default_nums);

	src_nums = bus_desc->ext_tx_buf_nums_master;
	default_nums = hwres->ext_tx_buf_nums_master;
	hw_dev->ext_tx_buf_nums_master =
		mt7990_chip_option_set_nums(src_nums, default_nums);

	src_nums = bus_desc->ext_tx_buf_nums_slave;
	default_nums = hwres->ext_tx_buf_nums_slave;
	hw_dev->ext_tx_buf_nums_slave =
		mt7990_chip_option_set_nums(src_nums, default_nums);
}

static int
mt7990_chip_option_check(struct pci_trans *trans)
{
	struct mtk_hw_dev *hw_dev = to_hw_dev(trans);
	struct mtk_chip_drv *chip_drv = hw_dev->chip_drv;
	struct mtk_chip_option_table *chip_opt_desc =
		&chip_drv->chip_opt_table[hw_dev->chip_opt];

	if (!chip_opt_desc->bus_desc_master.tx_ring_num) {
		dev_err(to_device(trans),
			"%s(): TxRing number in bus descriptor is 0\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int
mt7990_chip_option_set(struct pci_trans *trans)
{
	struct mtk_hw_dev *hw_dev = to_hw_dev(trans);
	struct mtk_chip_drv *chip_drv = hw_dev->chip_drv;
	struct mtk_chip_option_table *chip_opt_desc =
		&chip_drv->chip_opt_table[hw_dev->chip_opt];
	struct mtk_chip_desc *chip_desc = &chip_opt_desc->chip_desc;
	struct mtk_bus_desc *bus_desc = &chip_opt_desc->bus_desc_master;
	bool is_rro_enable = hw_dev->rro_mode ? true : false;
	u8 txr_idx = 0, txq_idx = 0;

	for (; txr_idx < bus_desc->tx_ring_num; txr_idx++) {
		struct mtk_bus_tx_ring_desc *tx_ring_desc =
			&bus_desc->tx_ring_desc[txr_idx];
		bool is_used = false;

		while (txq_idx < trans->txq_num) {
			struct pci_queue *q = &trans->txq[txq_idx++].q;

			if (q->q_attr == Q_TX_DATA) {
				q->band_idx_bmp = tx_ring_desc->band_idx_bmp;
				q->q_type_bmp = tx_ring_desc->q_type_bmp;
				is_used = true;
				break;
			}
		}

		if (!is_used)
			dev_err(to_device(trans), "%s(): TxRing descriptor[%u] isn't used\n",
				__func__, txr_idx);
	}

	trans->offload_disable = bus_desc->offload_disable;
	mt7990_chip_option_set_txrx_nums(hw_dev, bus_desc);

	set_bit(HWIFI_FLAG_CHIP_OPTION, &hw_dev->flags);
	hw_dev->rx_path_type = chip_desc->rx_path_type;
	hw_dev->max_ba_wsize_scene_mlo = chip_desc->max_ba_wsize_scene_mlo;

	if (!is_rro_enable || chip_desc->force_rro_disable) {
		hw_dev->rro_bypass_type = chip_desc->rro_disable_rro_bypass_type;
		hw_dev->mld_dest_type = chip_desc->rro_disable_mld_dest_type;
		hw_dev->txfreedone_path = chip_desc->rro_disable_txfreedone_path;
	} else {
		hw_dev->rro_bypass_type = chip_desc->rro_bypass_type;
		hw_dev->mld_dest_type = chip_desc->mld_dest_type;
		hw_dev->txfreedone_path = chip_desc->txfreedone_path;
		set_bit(HWIFI_FLAG_IN_CHIP_RRO, &hw_dev->flags);
	}

	return 0;
}

static int
mt7990_intr_option_check(struct pci_trans *trans)
{
	struct mtk_hw_dev *hw_dev = to_hw_dev(trans);
	struct mtk_chip_drv *chip_drv = hw_dev->chip_drv;
	struct mtk_bus_trans *bus_trans = to_bus_trans(trans);
	struct mtk_bus_cfg *bus_cfg = bus_trans->bus_cfg;
	struct mtk_intr_option_desc *intr_opt_desc;
	u8 intr_opt;

	intr_opt = chip_drv->intr_opt_set[hw_dev->intr_opt].intr_opt_master;
	intr_opt_desc = (struct mtk_intr_option_desc *)
		bus_cfg->intr_opt_table + intr_opt;

	if (!intr_opt_desc->vec_num) {
		dev_err(to_device(trans),
			"%s(): Vector number in interrupt descriptor is 0\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int
mt7990_intr_option_set(struct pci_trans *trans)
{
	struct mtk_hw_dev *hw_dev = to_hw_dev(trans);
	struct mtk_chip_drv *chip_drv = hw_dev->chip_drv;
	struct mtk_bus_trans *bus_trans = to_bus_trans(trans);
	struct mtk_bus_cfg *bus_cfg = bus_trans->bus_cfg;
	struct mtk_intr_option_desc *intr_opt_desc;
	struct vector_data *vec_data;
	u8 intr_opt, nr = 0;

	intr_opt = chip_drv->intr_opt_set[hw_dev->intr_opt].intr_opt_master;
	intr_opt_desc = (struct mtk_intr_option_desc *)
		bus_cfg->intr_opt_table + intr_opt;
	vec_data = intr_opt_desc->vec_data;

	hw_dev->irq_type = intr_opt_desc->irq_type;
	hw_dev->vec_num = intr_opt_desc->vec_num;
	hw_dev->alloc_vec_num = intr_opt_desc->alloc_vec_num;

	if (hw_dev->vec_num > MT7990_MAX_VEC_NUM)
		return -EINVAL;

	for (; nr < hw_dev->vec_num; nr++) {
		trans->vec_data[nr].bus_trans = to_bus_trans(trans);
		trans->vec_data[nr].attr = vec_data[nr].attr;
		trans->vec_data[nr].int_ena_mask = vec_data[nr].int_ena_mask;

		if (vec_data[nr].attr == VEC_ATTR_LUMP)
			continue;

		trans->vec_data[nr].ring_num = vec_data[nr].ring_num;
		trans->vec_data[nr].ring_id = vec_data[nr].ring_id;
		trans->vec_data[nr].info = vec_data[nr].info;
	}

	return 0;
}

static int
mt7990_rro_mode_set(struct pci_trans *trans)
{
	struct mtk_bus_trans *bus_trans = to_bus_trans(trans);
	struct mtk_hw_dev *dev = bus_trans->dev;

	trans->rro_mode = dev->rro_mode;

	return 0;
}

static int
mt7990_option_init(struct pci_trans *trans)
{
	int ret;

	ret = mt7990_rro_mode_set(trans);
	if (ret)
		goto end;

	ret = mt7990_chip_option_check(trans);
	if (ret)
		goto end;

	ret = mt7990_chip_option_set(trans);
	if (ret)
		goto end;

	ret = mt7990_intr_option_check(trans);
	if (ret)
		goto end;

	ret = mt7990_intr_option_set(trans);
end:
	return ret;
}

static int
mt7990_get_tx_token_num(struct pci_trans *trans, u16 tx_token_num[], u8 max_src_num)
{
	struct mtk_hw_dev *hw_dev = to_hw_dev(trans);
	struct mtk_tk_mgmt *tk_mgmt = &hw_dev->hw_ctrl.tk_mgmt[TK_MGMT_CMM];
	u8 idx;

	for (idx = 0; idx < max_src_num; idx++)
		tx_token_num[idx] = tk_mgmt->max_tx_tk_nums;

	return 0;
}

#ifdef CONFIG_HWIFI_DBG_ISR
static int mt7990_dbg_isr_init(struct pci_trans *ptrans)
{
	struct mtk_bus_trans *trans = to_bus_trans(ptrans);

	trans->bus_dbg.legacy_ring_irq_cnt =
		kzalloc(sizeof(atomic_t) * MT7990_LEGACY_MAX_BIT_NUM, GFP_ATOMIC);

	if (!trans->bus_dbg.legacy_ring_irq_cnt) {
		dev_err(to_device(trans), "%s(): allocate memory error: legacy_ring_irq_cnt\n",
			__func__);
		return -ENOMEM;
	}

	return 0;
}

static void mt7990_dbg_isr_exit(struct pci_trans *ptrans)
{
	struct mtk_bus_trans *trans = to_bus_trans(ptrans);
	void *tmp;

	tmp = trans->bus_dbg.legacy_ring_irq_cnt;
	trans->bus_dbg.legacy_ring_irq_cnt = NULL;
	kfree(tmp);
}
#endif

static int mt7990_dbg_init(struct pci_trans *ptrans)
{
	int ret = 0;

#ifdef CONFIG_HWIFI_DBG_ISR
	ret = mt7990_dbg_isr_init(ptrans);
	if (ret)
		return ret;
#endif

	return ret;
}

static void mt7990_dbg_exit(struct pci_trans *ptrans)
{
#ifdef CONFIG_HWIFI_DBG_ISR
	mt7990_dbg_isr_exit(ptrans);
#endif
}

static int
mt7990_driver_own(struct mtk_hw_dev *dev, struct mtk_bus_trans *trans, bool drv_own)
{
	struct pci_trans *pci_hw = to_pci_trans(trans);
	u32 reg = pci_hw->lp_fw_own_reg_addr;
	u32 target_state = (drv_own ? 0 : 1)
			<< CONN_HOST_CSR_TOP_WF_BAND0_LPCTL_WF_B0_AP_HOST_OWNER_STATE_SYNC_SHFT;

	if (drv_own)
		bus_write(trans, reg,
		CONN_HOST_CSR_TOP_WF_BAND0_LPCTL_WF_B0_AP_HOST_CLR_FW_OWN_HS_PULSE_MASK);
	else
		bus_write(trans, reg,
		CONN_HOST_CSR_TOP_WF_BAND0_LPCTL_WF_B0_AP_HOST_SET_FW_OWN_HS_PULSE_MASK);

	if (!mtk_bus_poll_msec(trans, reg,
		CONN_HOST_CSR_TOP_WF_BAND0_LPCTL_WF_B0_AP_HOST_OWNER_STATE_SYNC_MASK,
		target_state, 500)) {
		dev_err(trans->pdev, "Timeout for %s\n",
			drv_own ? "driver own" : "fw own");

		return -EIO;
	}

	return 0;
}

static int
mt7990_check_fwdl_state(struct mtk_hw_dev *dev, u32 state)
{
	struct mtk_bus_trans *trans = dev->bus_trans;

	dev_info(trans->pdev, "%s(): check target stat=%d, cur state=%d\n",
		__func__, state,
		bus_read(trans, CONN_HOST_CSR_TOP_CONN_ON_MISC_ADDR) &
		CONN_HOST_CSR_TOP_CONN_ON_MISC_DRV_FW_STAT_SYNC_MASK);
	if (!mtk_bus_poll_msec(trans, CONN_HOST_CSR_TOP_CONN_ON_MISC_ADDR,
			CONN_HOST_CSR_TOP_CONN_ON_MISC_DRV_FW_STAT_SYNC_MASK,
			field_prep(CONN_HOST_CSR_TOP_CONN_ON_MISC_DRV_FW_STAT_SYNC_MASK,
			   state), 500)) {
		dev_err(trans->pdev,
			"Timeout for initializing firmware, %x\n",
			bus_read(trans,
			CONN_HOST_CSR_TOP_CONN_ON_MISC_DRV_FW_STAT_SYNC_ADDR));
		return -EIO;
	}
	return 0;
}

static int
mt7990_fwmode_conf(struct mtk_hw_dev *dev)
{
	u32 val;
	struct mtk_bus_trans *trans = dev->bus_trans;

#define SWDEF_MODE_ADDR 0x009143C
#define FW_NORMAL_MODE 0
	/*set as normal mode*/
	val = bus_read(trans, SWDEF_MODE_ADDR);
	bus_write(trans, SWDEF_MODE_ADDR, FW_NORMAL_MODE);
	return 0;
}

static u32
mt7990_l1_remap_read(struct pci_trans *trans, u32 offset)
{
	u32 bk_val, tmp_val;

	bk_val = bus_read(to_bus_trans(trans), L1_REMAP_ADDR);
	tmp_val = bk_val & ~L1_REMAP_MASK;
	tmp_val |= GET_L1_REMAP_BASE(offset) << L1_REMAP_SHFT;
	bus_write(to_bus_trans(trans), L1_REMAP_ADDR, tmp_val);
	tmp_val = GET_L1_REMAP_OFFSET(offset);
	tmp_val = bus_read(to_bus_trans(trans), L1_REMAP_BASE_ADDR + tmp_val);
	bus_write(to_bus_trans(trans), L1_REMAP_ADDR, bk_val);

	return tmp_val;
}

static void
mt7990_l1_remap_write(struct pci_trans *trans, u32 offset, u32 value)
{
	u32 bk_val, tmp_val;

	bk_val = bus_read(to_bus_trans(trans), L1_REMAP_ADDR);
	tmp_val = bk_val & ~L1_REMAP_MASK;
	tmp_val |= GET_L1_REMAP_BASE(offset) << L1_REMAP_SHFT;
	bus_write(to_bus_trans(trans), L1_REMAP_ADDR, tmp_val);
	tmp_val = GET_L1_REMAP_OFFSET(offset);
	bus_write(to_bus_trans(trans), L1_REMAP_BASE_ADDR + tmp_val, value);
	bus_write(to_bus_trans(trans), L1_REMAP_ADDR, bk_val);
}

static u32
mt7990_l2_remap_read(struct pci_trans *trans, u32 offset)
{
	u32 bk_val, tmp_val;

	bk_val = bus_read(to_bus_trans(trans), L2_REMAP_ADDR);
	tmp_val = bk_val & ~L2_REMAP_MASK;
	tmp_val |= GET_L2_REMAP_BASE(offset) << L2_REMAP_SHFT;
	bus_write(to_bus_trans(trans), L2_REMAP_ADDR, tmp_val);
	tmp_val = mt7990_l1_remap_read(trans, L2_REMAP_BASE_ADDR);
	bus_write(to_bus_trans(trans), L2_REMAP_ADDR, bk_val);

	return tmp_val;
}

static void
mt7990_hw_caps_init(struct mtk_hw_dev *dev)
{
	struct pci_trans *trans = to_pci_trans(dev->bus_trans);
	u32 val;

#define HW_VERSION_ADDR 0x70010204
	val = mt7990_l1_remap_read(trans, HW_VERSION_ADDR);
	if (val == 0x8A00) {
		dev->limit |= BIT(LIMIT_NO_DROP_IP_FRAG_OLD_PKT);
		dev->limit |= BIT(LIMIT_SET_ADDBA_SSN);
	} else {
		dev->limit |= BIT(LIMIT_SET_ADDBA_TID);
	}

	dev->limit |= BIT(LIMIT_SET_BMC_WCID) | BIT(LIMIT_SET_OUTSTANDING);

#define CBTOP_GPIO_ADDR			0x700056F0
#define CBTOP_GPIO_ADDR_GPIO1_MASK	BIT(16)
#define CBTOP_GPIO_ADDR_GPIO2_MASK	BIT(17)
#define CBTOP_GPIO_ADDR_GPIO4_MASK	BIT(19)

	val = mt7990_l1_remap_read(trans, CBTOP_GPIO_ADDR);

	if (val & CBTOP_GPIO_ADDR_GPIO2_MASK) {
		dev->band_idx_bmp_sup = BIT(BAND2);
	} else if (val & CBTOP_GPIO_ADDR_GPIO4_MASK) {
		dev->band_idx_bmp_sup = GENMASK(BAND2, BAND0);
	} else {
		if (val & CBTOP_GPIO_ADDR_GPIO1_MASK)
			dev->band_idx_bmp_sup = BIT(BAND0) | BIT(BAND2);
		else
			dev->band_idx_bmp_sup = GENMASK(BAND2, BAND0);
	}

	dev->band_num_sup = hweight8(dev->band_idx_bmp_sup);
}

static ssize_t
mt7990_chip_opt_read_debugfs(struct file *file, char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct mtk_hw_dev *dev = file->private_data;
	struct mtk_bus_trans *bus_trans = dev->bus_trans;
	char buf[256] = {0};
	int len = 0;
	u8 i;

	len += scnprintf(buf + len, sizeof(buf) - len,
			"chip option = %u\n", dev->chip_opt);

	for (i = 0; bus_trans; i++) {
		struct pci_trans *trans = to_pci_trans(bus_trans);

		len += scnprintf(buf + len, sizeof(buf) - len,
			"bus%u, pcie speed = 0x%x, pcie width = %u\n", i,
			trans->pci_gen, trans->pci_lane);
		bus_trans = bus_trans->next;
	}

	return simple_read_from_buffer(ubuf, count, ppos, buf, len);
}

static ssize_t
mt7990_chip_opt_write_debugfs(struct file *file, const char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct mtk_hw_dev *dev = file->private_data;
	u8 chip_opt;
	char buf[256] = {0};
	int ret, rc;

	rc = simple_write_to_buffer(buf, sizeof(buf) - 1, ppos, ubuf, count);

	if (rc < 0) {
		ret = rc;
		goto done;
	}

	buf[rc] = '\0';

	ret = kstrtou8(buf, 10, &chip_opt);

	if (ret) {
		ret = -EINVAL;
		goto done;
	}

	if (chip_opt >= dev->chip_drv->chip_opt_table_sz) {
		ret = -EINVAL;
		dev_err(dev->dev, "chip option = %u larger than table size = %u\n",
			chip_opt, dev->chip_drv->chip_opt_table_sz);
		goto done;
	}

	dev->chip_opt = chip_opt;
	ret = count;
done:
	return ret;
}

static ssize_t
mt7990_intr_opt_read_debugfs(struct file *file, char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct mtk_hw_dev *dev = file->private_data;
	char buf[256] = {0};
	int len = 0;

	len += scnprintf(buf + len, sizeof(buf) - len,
			"interrupt option = %u\n", dev->intr_opt);

	return simple_read_from_buffer(ubuf, count, ppos, buf, len);
}

static ssize_t
mt7990_intr_opt_write_debugfs(struct file *file, const char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct mtk_hw_dev *dev = file->private_data;
	u8 intr_opt;
	char buf[256] = {0};
	int ret, rc;

	rc = simple_write_to_buffer(buf, sizeof(buf) - 1, ppos, ubuf, count);

	if (rc < 0) {
		ret = rc;
		goto done;
	}

	buf[rc] = '\0';

	ret = kstrtou8(buf, 10, &intr_opt);

	if (ret) {
		ret = -EINVAL;
		goto done;
	}

	if (intr_opt >= dev->chip_drv->intr_opt_set_sz) {
		ret = -EINVAL;
		dev_err(dev->dev, "interrupt option = %u larger than table size = %u\n",
			intr_opt, dev->chip_drv->intr_opt_set_sz);
		goto done;
	}

	dev->intr_opt = intr_opt;
	ret = count;
done:
	return ret;
}

static ssize_t
mt7990_rro_enable_read_debugfs(struct file *file, char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct mtk_hw_dev *dev = file->private_data;
	char buf[256] = {0};
	int len = 0;

	len += scnprintf(buf + len, sizeof(buf) - len,
			"rro_enable = %u\n", dev->rro_mode ? true : false);

	return simple_read_from_buffer(ubuf, count, ppos, buf, len);
}

static ssize_t
mt7990_rro_enable_write_debugfs(struct file *file, const char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct mtk_hw_dev *dev = file->private_data;
	bool rro_enable;
	char buf[256] = {0};
	int ret, rc;

	rc = simple_write_to_buffer(buf, sizeof(buf) - 1, ppos, ubuf, count);

	if (rc < 0) {
		ret = rc;
		goto done;
	}

	buf[rc] = '\0';

	ret = kstrtobool(buf, &rro_enable);

	if (ret) {
		ret = -EINVAL;
		goto done;
	}

	dev->rro_mode = rro_enable ? HW_RRO_V3_0_BUF_PG : HW_RRO_OFF;
	ret = count;
done:
	return ret;
}


static const struct file_operations mt7990_fops_chip_opt = {
	.read = mt7990_chip_opt_read_debugfs,
	.write = mt7990_chip_opt_write_debugfs,
	.open = simple_open,
};

static const struct file_operations mt7990_fops_intr_opt = {
	.read = mt7990_intr_opt_read_debugfs,
	.write = mt7990_intr_opt_write_debugfs,
	.open = simple_open,
};

static const struct file_operations mt7990_fops_rro_enable = {
	.read = mt7990_rro_enable_read_debugfs,
	.write = mt7990_rro_enable_write_debugfs,
	.open = simple_open,
};

static int
mt7990_hw_debugfs_init(struct mtk_hw_dev *dev)
{
	struct mtk_idr_entry *idr_entry = &dev->sys_idx;
	struct dentry *dir = idr_entry->dir;

	mt_debugfs_create_file("chip_option", 0644, dir, dev,
				&mt7990_fops_chip_opt);

	mt_debugfs_create_file("intr_option", 0644, dir, dev,
				&mt7990_fops_intr_opt);

	mt_debugfs_create_file("rro_enable", 0644, dir, dev,
				&mt7990_fops_rro_enable);

	return 0;
}

static int
mt7990_hw_init(struct mtk_hw_dev *dev)
{
	bmac_ops_init(dev);
	mt7990_hw_caps_init(dev);
	mt7990_hw_debugfs_init(dev);
	return mt7990_fwmode_conf(dev);
}

static int
mt7990_get_pc_value(struct mtk_hw_dev *dev, u32 *value)
{
	struct pci_trans *trans = to_pci_trans(dev->bus_trans);

#define WM_PC_ADDR 0x89050104
	*value = mt7990_l2_remap_read(trans, WM_PC_ADDR);
	return 0;
}

static int
mt7990_hw_chip_reset(struct mtk_hw_dev *dev)
{
	u32 val;
	struct pci_trans *trans = to_pci_trans(dev->bus_trans);
	u32 chip_id = MT7990_CHIP_ID;

#define ADDR_GPIO_RESET_MODE 0x7000532c
#define VAL_GPIO_RESET_MODE 0x90000000

	/* set reset-gpio to reset mode */
	val = mt7990_l1_remap_read(trans, ADDR_GPIO_RESET_MODE);
	mt7990_l1_remap_write(trans, ADDR_GPIO_RESET_MODE, VAL_GPIO_RESET_MODE);
	/* call chip_reset func */
	if (mtk_interface_chip_reset(dev, chip_id))
		mt7990_l1_remap_write(trans, ADDR_GPIO_RESET_MODE, val); /* failure, roll-back*/

	return 0;
}

static int
mt7990_hw_reset(struct mtk_hw_dev *dev)
{
	struct pci_trans *trans = to_pci_trans(dev->bus_trans);

#define WF_SUBSYS_RST 0x1F8600

	bus_write(to_bus_trans(trans), WF_SUBSYS_RST, 0x1);
	udelay(1000);
	bus_write(to_bus_trans(trans), WF_SUBSYS_RST, 0x0);

	return 0;
}

static struct pci_device_id mt7990_pci_device_table[] = {
	{ PCI_DEVICE(0x14c3, MT7990_CHIP_ID) },
	{ },
};

static struct mtk_chip_ctrl_ops mt7990_chip_ops = {
	.driver_own = mt7990_driver_own,
	.check_fwdl_state = mt7990_check_fwdl_state,
	.set_filter = NULL,
	.mac_mib_update = NULL,
	.hw_init = mt7990_hw_init,
	.hw_reset = mt7990_hw_reset,
	.hw_chip_reset = mt7990_hw_chip_reset,
	.get_pc_value = mt7990_get_pc_value,
};

static struct mtk_hwres_radio_info mt7990_band0[] = {
	{
		.type = HWIFI_PHY_2G,
		.cap = HWIFI_PHY_LEGACY |
			HWIFI_PHY_N |
			HWIFI_PHY_HE |
			HWIFI_PHY_BE
	},
	{
		.type = HWIFI_PHY_5G,
		.cap = HWIFI_PHY_LEGACY |
			HWIFI_PHY_N |
			HWIFI_PHY_VHT |
			HWIFI_PHY_HE |
			HWIFI_PHY_BE
	}
};

static struct mtk_hwres_radio_info mt7990_band1[] = {
	{
		.type = HWIFI_PHY_2G,
		.cap = HWIFI_PHY_LEGACY |
			HWIFI_PHY_N |
			HWIFI_PHY_HE |
			HWIFI_PHY_BE
	},
	{
		.type = HWIFI_PHY_5G,
		.cap = HWIFI_PHY_LEGACY |
			HWIFI_PHY_N |
			HWIFI_PHY_VHT |
			HWIFI_PHY_HE |
			HWIFI_PHY_BE
	}
};

static struct mtk_hwres_radio_info mt7990_band2[] = {
	{
		.type = HWIFI_PHY_2G,
		.cap = HWIFI_PHY_LEGACY |
			HWIFI_PHY_N |
			HWIFI_PHY_HE |
			HWIFI_PHY_BE
	},
	{
		.type = HWIFI_PHY_5G,
		.cap = HWIFI_PHY_LEGACY |
			HWIFI_PHY_N |
			HWIFI_PHY_VHT |
			HWIFI_PHY_HE |
			HWIFI_PHY_BE
	}
};

#ifdef CONFIG_HWIFI_REBB_SUPPORT
static struct mtk_mcu_dest_2_path mt7990_rebb_dest_2_path[MCU_DEST_MAX_NUM] = {
	{MCU_DEST_WM,		MCU_PATH_WA},
	{MCU_DEST_RVD,		MCU_PATH_WA},
	{MCU_DEST_WA,		MCU_PATH_WA},
	{MCU_DEST_WA_WM,	MCU_PATH_WA},
	{MCU_DEST_WO,		MCU_PATH_WO},
};

static struct mtk_hwres_radio_cap mt7990_rebb_hw_radio_cap[] = {
	{
		/*band_idx 0*/
		.radio_info_size = ARRAY_SIZE(mt7990_band0),
		.info = mt7990_band0,
		.band_idx = 0,
		.wmm_set = {
			.start = 0,
			.end = MT7990_WMM_SET_END
		},
		.omac = {
			.start = 0,
			.end = 0x4
		},
		.ext_omac = {
			.start = 0x11,
			.end = 0x1f
		},
		.rept_omac = {
			.start = 0x20,
			.end = 0x3f
		},
		.max_tx_tk_nums = 16384,
		.rsv_tx_tk_nums = MT7990_SW_TX_RSV_TOKEN_NUM_PER_BAND,
	},
	{
		/*band_idx 1*/
		.radio_info_size = ARRAY_SIZE(mt7990_band1),
		.info = mt7990_band1,
		.band_idx = 1,
		.wmm_set = {
			.start = 0,
			.end = MT7990_WMM_SET_END
		},
		.omac = {
			.start = 0,
			.end = 0x4
		},
		.ext_omac = {
			.start = 0x11,
			.end = 0x1f
		},
		.rept_omac = {
			.start = 0x20,
			.end = 0x3f
		},
		.max_tx_tk_nums = 16384,
		.rsv_tx_tk_nums = MT7990_SW_TX_RSV_TOKEN_NUM_PER_BAND,
	},
	{
		/*band_idx 2*/
		.radio_info_size = ARRAY_SIZE(mt7990_band2),
		.info = mt7990_band2,
		.band_idx = 2,
		.wmm_set = {
			.start = 0,
			.end = MT7990_WMM_SET_END
		},
		.omac = {
			.start = 0,
			.end = 0x4
		},
		.ext_omac = {
			.start = 0x11,
			.end = 0x1f
		},
		.rept_omac = {
			.start = 0x20,
			.end = 0x3f
		},
		.max_tx_tk_nums = 16384,
		.rsv_tx_tk_nums = MT7990_SW_TX_RSV_TOKEN_NUM_PER_BAND,
	},
};

static struct mtk_hwres_cap mt7990_rebb_hw_res = {
	.uwtbl = {
		.start = 1,
		.end = 512,
	},
	.group = {
		.start = 513,
		.end = 576,
	},
	.tx_token = {
		.start = 0,
		.end = 16383,
	},
	.rx_token = {
		.start = 0,
		.end = 12287,
	},
	.bss = {
		.start = 0,
		.end = 63,
	},
	.mld_addr = {
		.start = 0,
		.end = 15,
	},
	.link_addr = {
		.start = 16,
		.end = 63
	},
	.mld_remap = {
		.start = 0,
		.end = 15,
	},
	.rro_free_pool_tk_nums = 32768,
	.radio_num = ARRAY_SIZE(mt7990_rebb_hw_radio_cap),
	.radio_cap = mt7990_rebb_hw_radio_cap,
	.tx_free_done_ver = FREE_NOTIFY_VERSION_5,
};

static struct mtk_chip_hw_cap mt7990_rebb_chip_cap = {
	.dev_size = sizeof(struct mtk_hw_dev),
	.fw_flags = BIT(FW_FLAG_WM) | BIT(FW_FLAG_WA) | BIT(FW_FLAG_DSP),
	.mac_type = MAC_TYPE_BMAC,
	.mtxd_sz = 148,
	.mrxd_sz = 0x60,
	.mac_rxd_grp_0_sz = 8 * 4,
	.mac_txd_ver = MAC_TXD_V3,
	.hwres = &mt7990_rebb_hw_res,
	.mcu_infos = {
		[MCU_WM] = {
			.fw = {
				[MCU_FW_MODE_NORMALMODE] = MT7990_FIRMWARE_WM,
				[MCU_FW_MODE_TESTMODE] = MT7990_FIRMWARE_WM_TESTMODE,
			},
			.fw_mode = MCU_FW_MODE_NORMALMODE,
			.rom_patch = MT7990_ROM_PATCH,
			.opt = BIT(WM_OPT_PATCH_DL)
		},
		[MCU_WA] = {
			.fw = {
				[MCU_FW_MODE_NORMALMODE] = MT7990_FIRMWARE_WA,
				[MCU_FW_MODE_TESTMODE] = NULL,
			},
			.fw_mode = MCU_FW_MODE_NORMALMODE,
			.rom_patch = NULL,
			.opt = 0
		},
		[MCU_DSP] = {
			.fw = {
				[MCU_FW_MODE_NORMALMODE] = MT7990_FIRMWARE_DSP,
				[MCU_FW_MODE_TESTMODE] = NULL,
			},
			.fw_mode = MCU_FW_MODE_NORMALMODE,
			.rom_patch = NULL,
			.opt = 0
		},
	},
	.cap_info = {
		.sw_max_amsdu_num = 13,
		.sw_max_amsdu_len = 1536,
	},
#ifdef CONFIG_HWIFI_SW_AMSDU
	.mac_cap = BIT(CAP_PAO) | BIT(CAP_RRO) | BIT(CAP_OFFLOAD_TXD),
	.hif_txd_ver = HIF_TXD_V2_1,
#else
	.mac_cap = BIT(CAP_RRO) | BIT(CAP_OFFLOAD_TXD),
	.hif_txd_ver = HIF_TXD_V1_0,
#endif
};

static struct pci_chip_ops mt7990_rebb_pci_ops = {
	.dma_enable = mt7990_pdma_enable,
	.dma_disable = mt7990_pdma_disable,
	.dma_init = mt7990_pdma_init,
	.dma_match = mt7990_pdma_match,
	.dma_schdl_init = NULL,
	.rro_hw_init = mt7990_rro_hw_init,
	.option_init = mt7990_option_init,
	.get_tx_token_num = mt7990_get_tx_token_num,
	.dbg_init = mt7990_dbg_init,
	.dbg_exit = mt7990_dbg_exit,
};

static struct mtk_chip_drv mt7990_rebb_drv = {
	.bus_cfg = {
		.bus_type = MTK_BUS_PCI,
		.id_table = mt7990_pci_device_table,
		.name = KBUILD_MODNAME,
		.profile = &mt7990_pci_wa_profile,
		.ms_type = CHIP_TYPE_MASTER,
		.bus_ops = &mt7990_rebb_pci_ops,
		.bus_drv_sz = sizeof(struct pci_driver),
		.intr_opt_table = mt7990_intr_opt_tbl,
		.intr_opt_table_sz = ARRAY_SIZE(mt7990_intr_opt_tbl),
	},
	.ctl_ops = &mt7990_chip_ops,
	.hw_caps = &mt7990_rebb_chip_cap,
	.device_id = MT7990_CHIP_ID,
	.dest_2_path = mt7990_rebb_dest_2_path,
	.chip_opt_table = mt7990_chip_opt_tbl,
	.chip_opt_table_sz = ARRAY_SIZE(mt7990_chip_opt_tbl),
	.intr_opt_set = mt7990_intr_opt_set,
	.intr_opt_set_sz = ARRAY_SIZE(mt7990_intr_opt_set),
};
#endif /*CONFIG_HWIFI_REBB_SUPPORT*/

#ifdef CONFIG_HWIFI_FW_HDR_SUPPORT
static void mt7990_fw_hdr_prepare(struct mtk_chip_drv *drv)
{
	struct mtk_chip_hw_cap *hw_caps = drv->hw_caps;
	struct mtk_chip_mcu_info *mcu_info;

	/* WM */
	mcu_info = &hw_caps->mcu_infos[MCU_WM];
	mcu_info->fw_hdr[MCU_FW_MODE_NORMALMODE] = mt7990_FirmwareImage_1_1;
	mcu_info->fw_hdr_len[MCU_FW_MODE_NORMALMODE] = sizeof(mt7990_FirmwareImage_1_1);
	mcu_info->fw_hdr[MCU_FW_MODE_TESTMODE] = mt7990_FirmwareImage_Testmode_1_1;
	mcu_info->fw_hdr_len[MCU_FW_MODE_TESTMODE] = sizeof(mt7990_FirmwareImage_Testmode_1_1);
	mcu_info->rom_patch_hdr = mt7990_rom_patch_1_1;
	mcu_info->applied_method = HEADER_METHOD;

	/* WA */
	mcu_info = &hw_caps->mcu_infos[MCU_WA];
	mcu_info->fw_hdr[MCU_FW_MODE_NORMALMODE] = mt7990_WA_FirmwareImage_1_1;
	mcu_info->fw_hdr_len[MCU_FW_MODE_NORMALMODE] = sizeof(mt7990_WA_FirmwareImage_1_1);
	mcu_info->fw_hdr[MCU_FW_MODE_TESTMODE] = NULL;
	mcu_info->fw_hdr_len[MCU_FW_MODE_TESTMODE] = 0;
	mcu_info->rom_patch_hdr = NULL;
	mcu_info->applied_method = HEADER_METHOD;
}
#endif /* CONFIG_HWIFI_FW_HDR_SUPPORT */

static int
mt7990_drv_sanity_check(struct mtk_chip_drv *drv)
{
	if (drv->chip_opt >= drv->chip_opt_table_sz) {
		pr_err("%s(): chip option = %u larger than table size = %u\n",
			__func__, drv->chip_opt, drv->chip_opt_table_sz);
		return -EINVAL;
	}
	if (drv->intr_opt >= drv->intr_opt_set_sz) {
		pr_err("%s(): interrupt option = %u larger than table size = %u\n",
			__func__, drv->intr_opt, drv->intr_opt_set_sz);
		return -EINVAL;
	}

	return 0;
}

static int __init mtk_chip_mt7990_init(void)
{
	static struct mtk_chip_drv *drv;
	int ret;

#ifdef CONFIG_HWIFI_REBB_SUPPORT
	drv = &mt7990_rebb_drv;
#endif
	drv->interface_type = mac_type;
	drv->chip_opt = option_type;
	drv->rro_mode = rro_enable ? HW_RRO_V3_0_BUF_PG : HW_RRO_OFF;
	drv->intr_opt = intr_option_set;
#ifdef CONFIG_HWIFI_HIF_WED
	drv->bus_cfg.bus_type = MTK_BUS_WED;
	drv->bus_cfg.bus_sub_type = MTK_BUS_PCI;
	drv->bus_cfg.bus_drv_sz = sizeof(struct wed_driver);
	mt7990_rebb_hw_res.tx_token.start = 16384;
	mt7990_rebb_hw_res.tx_token.end = 31743;
	mt7990_rebb_hw_res.rro_free_pool_tk_nums = 0;
	mt7990_rebb_hw_res.ext_tx_tk_nums_master = 16384; /* TKID num */
	mt7990_rebb_hw_res.ext_tx_tk_nums_slave = 16384;
	mt7990_rebb_hw_res.ext_rx_tk_nums_master = 24576; /* RXBM buffer num */
	mt7990_rebb_hw_res.ext_rx_tk_nums_slave = 8192;
	mt7990_rebb_hw_res.ext_tx_buf_nums_master = 32768; /* TXBM buffer num */
	mt7990_rebb_hw_res.ext_tx_buf_nums_slave = 32768;
	mt7990_rebb_hw_radio_cap[0].max_tx_tk_nums = 5120;
	mt7990_rebb_hw_radio_cap[1].max_tx_tk_nums = 5120;
	mt7990_rebb_hw_radio_cap[2].max_tx_tk_nums = 5120;

	mt7990_rebb_hw_res.rx_token.end += mt7990_rebb_hw_res.ext_rx_tk_nums_master +
					   mt7990_rebb_hw_res.ext_rx_tk_nums_slave;
#endif
	mt7990_rebb_hw_res.rx_token.end += mt7990_rebb_hw_res.rro_free_pool_tk_nums;

#ifdef CONFIG_HWIFI_FW_HDR_SUPPORT
	mt7990_fw_hdr_prepare(drv);
#endif /* CONFIG_HWIFI_FW_HDR_SUPPORT */
	ret = mt7990_drv_sanity_check(drv);
	if (ret)
		return ret;

	return mtk_chip_register(drv);
}

static void __exit mtk_chip_mt7990_exit(void)
{
	static struct mtk_chip_drv *drv;

#ifdef CONFIG_HWIFI_REBB_SUPPORT
	drv = &mt7990_rebb_drv;
#endif

	mtk_chip_unregister(drv);
}

module_init(mtk_chip_mt7990_init);
module_exit(mtk_chip_mt7990_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Mediatek Hardware Wi-Fi Module");
