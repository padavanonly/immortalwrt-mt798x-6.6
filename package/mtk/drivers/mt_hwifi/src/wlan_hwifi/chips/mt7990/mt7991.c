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
#include "mt7990_option_table.h"

/*#include "firmware/mt7991_e2p_ePAeLNA.h"*/

/*TBD: not ready*/
static int mac_type = MTK_INTERFACE_CONNAC;
module_param(mac_type, int, 0644);
MODULE_PARM_DESC(mac_type, "mac driver type");

#define MT7991_CHIP_ID 0x7991

static void
dump_chip_info(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);
	u32 chipid = MT7991_CHIP_ID;

	dev_info(to_device(trans), "%s(): chip id: %x\n",
		__func__, chipid);
}

static void
_mt7991_pdma_irq_clear_int_sta(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);
	unsigned long flags;

	spin_lock_irqsave(&pci_hw->irq_lock, flags);
	bus_write(trans, pci_hw->mcu2host_sw_int_sta_addr, 0xffffffff);
	bus_write(trans, pci_hw->int_mask_map[0].int_sts_addr, 0xffffffff);
	spin_unlock_irqrestore(&pci_hw->irq_lock, flags);
}

static void
_mt7991_pdma_irq_init(struct pci_trans *pci_hw)
{
	_mt7991_pdma_irq_clear_int_sta(pci_hw);
}

static void
_mt7991_pdma_irq_disable(struct pci_trans *pci_hw)
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
_mt7991_pdma_irq_enable(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);
	unsigned long flags;

	spin_lock_irqsave(&pci_hw->irq_lock, flags);
	bus_write(trans, pci_hw->int_mask_map[0].int_ena_addr,
		pci_hw->int_mask_map[0].int_ena_mask);
	bus_write(trans, pci_hw->mcu2host_sw_int_ena_addr,
		pci_hw->mcu2host_sw_int_ser_mask);
	spin_unlock_irqrestore(&pci_hw->irq_lock, flags);
	clear_bit(PCI_STATE_ISR_PEDING, &pci_hw->state);
	set_bit(PCI_STATE_ISR_START, &pci_hw->state);
}

static void
_mt7991_pdma_config(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);
	struct mtk_hw_dev *hw_dev = to_hw_dev(trans);

	/* GLO_CFG */
	bus_set(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_ADDR,
		WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_OMIT_TX_INFO_MASK |
		WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_PDMA_ADDR_EXT_EN_MASK);
	bus_clear(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_ADDR,
		WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_OMIT_RX_INFO_MASK);

	/* GLO_CFG_EXT0 */
	bus_set(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_EXT0_ADDR,
		WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_EXT0_CSR_RX_WB_RXD_MASK |
		WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_EXT0_CSR_WED_MERGE_MODE_MASK);

	/* GLO_CFG_EXT1 */
	bus_set(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_EXT1_ADDR,
		WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_EXT1_CSR_TX_FCTRL_MODE_MASK);

	/* WRAP_CSR_HOST_CONFIG */
	bus_set(trans, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_ADDR,
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pdma_per_band_MASK);
	bus_clear(trans, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_ADDR,
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_bn0_pcie_sel_MASK |
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_bn1_pcie_sel_MASK |
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_bn2_pcie_sel_MASK);

	if (hw_dev->chip_opt == 2) {
		/* Set BN0/1 payload transfer through PCIE1 */
		bus_set(trans, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_ADDR,
			WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_bn0_pcie_sel_MASK |
			WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_bn1_pcie_sel_MASK);
	} else if (hw_dev->chip_opt == MT7990_CHIP_OPT_AN7581) {
		/* Set BN0 payload transfer through PCIE1 */
		bus_set(trans, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_ADDR,
			WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_bn0_pcie_sel_MASK);
	} else {
		/* Set BN2 payload transfer through PCIE1 */
		bus_set(trans, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_ADDR,
			WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_bn2_pcie_sel_MASK);
	}

	if (!(trans->dev->limit & BIT(LIMIT_SET_OUTSTANDING)))
		return;

	if (pci_hw->pci_gen < PCIE_GEN3 ||
	    (pci_hw->pci_gen == PCIE_GEN3 && pci_hw->pci_lane < 2)) {
		bus_set_field(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_EXT0_ADDR,
			WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_EXT0_CSR_AXI_AW_OUTSTANDING_NUM_MASK,
			0x3);
	}
}

static void
_mt7991_pdma_interrupt_select(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);

#define WF_WFDMA_HOST_DMA0_PCIE1_HOST_RX_INT_PCIE_SEL_ADDR_RX_RING_6_MASK BIT(6)

	/* Redirect RxRing6 interrupt to PCIE0 */
	bus_set(trans, WF_WFDMA_HOST_DMA0_PCIE1_HOST_RX_INT_PCIE_SEL_ADDR,
		WF_WFDMA_HOST_DMA0_PCIE1_HOST_RX_INT_PCIE_SEL_ADDR_RX_RING_6_MASK);
}

static void
_mt7991_pdma_rx_threshold(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);

	/* RRO related, pause PP if lower than DMAD threshold */
	/* WFDMA */
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_PAUSE_RX_Q_TH54_ADDR,
		  0xc000c);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_PAUSE_RX_Q_TH76_ADDR,
		  0x10008);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_PAUSE_RX_Q_TH98_ADDR,
		  0x10008);

	/* RRO */
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_PAUSE_RRO_Q_ADDR,
		  0x20);
}

static void
_mt7991_pdma_prefetch_config(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);

	/* Rx */
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING0_EXT_CTRL_ADDR, 0x00000002);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING1_EXT_CTRL_ADDR, 0x00200002);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING2_EXT_CTRL_ADDR, 0x00400002);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING3_EXT_CTRL_ADDR, 0x00600002);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING4_EXT_CTRL_ADDR, 0x00800010);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING5_EXT_CTRL_ADDR, 0x01800010);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING6_EXT_CTRL_ADDR, 0x02800010);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING7_EXT_CTRL_ADDR, 0x03800004);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING8_EXT_CTRL_ADDR, 0x03C00010);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING9_EXT_CTRL_ADDR, 0x04C00004);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING10_EXT_CTRL_ADDR, 0x05000004);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING11_EXT_CTRL_ADDR, 0x05400004);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING12_EXT_CTRL_ADDR, 0x05800004);

	/* Tx */
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING0_EXT_CTRL_ADDR, 0x00000000);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING1_EXT_CTRL_ADDR, 0x00000000);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING2_EXT_CTRL_ADDR, 0x00000000);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING3_EXT_CTRL_ADDR, 0x00000000);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING4_EXT_CTRL_ADDR, 0x00000000);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING5_EXT_CTRL_ADDR, 0x00000000);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING6_EXT_CTRL_ADDR, 0x00000000);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING16_EXT_CTRL_ADDR, 0x05E00002);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING17_EXT_CTRL_ADDR, 0x05C00002);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING18_EXT_CTRL_ADDR, 0x05E00008);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING19_EXT_CTRL_ADDR, 0x06600008);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING20_EXT_CTRL_ADDR, 0x06E00002);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING21_EXT_CTRL_ADDR, 0x07000008);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING22_EXT_CTRL_ADDR, 0x07800008);
}

static void
_mt7991_pdma_reset_didx(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);

	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RST_DTX_PTR_ADDR, 0xffffffff);
	bus_write(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RST_DRX_PTR_ADDR, 0xffffffff);
}

static void
_mt7991_pdma_outstanding_config(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);

	bus_set_field(trans, WF_WFDMA_EXT_WRAP_CSR_WFDMA_AXI0_R2A_CTRL_0_ADDR,
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_AXI0_R2A_CTRL_0_RD_OUTSTAND_NUM_MASK, 0x14);

	if (!(trans->dev->limit & BIT(LIMIT_SET_OUTSTANDING)))
		return;

	if (pci_hw->pci_gen < PCIE_GEN3 ||
	    (pci_hw->pci_gen == PCIE_GEN3 && pci_hw->pci_lane < 2)) {
		bus_set_field(trans, WF_WFDMA_EXT_WRAP_CSR_WFDMA_AXI0_R2A_CTRL_2_ADDR,
			WF_WFDMA_EXT_WRAP_CSR_WFDMA_AXI0_R2A_CTRL_2_WR_OUTSTAND_NUM_MASK,
			0x3);
	}
}

static void
_mt7991_pdma_init(struct pci_trans *pci_hw)
{
	_mt7991_pdma_config(pci_hw);
	_mt7991_pdma_interrupt_select(pci_hw);
	_mt7991_pdma_rx_threshold(pci_hw);
	_mt7991_pdma_prefetch_config(pci_hw);
	_mt7991_pdma_reset_didx(pci_hw);
	_mt7991_pdma_outstanding_config(pci_hw);
}

static void
_mt7991_pdma_unpause(struct pci_trans *pci_hw)
{
	bus_write(to_bus_trans(pci_hw), pci_hw->pause_trxq_reg_addr, 0);
}

static void
_mt7991_pdma_pause(struct pci_trans *pci_hw)
{
}

static bool
_mt7991_pdma_poll(struct pci_trans *pci_hw, u32 round, u32 wait_us)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);
	u32 val;
	u8 i = 0;
	bool idle = false;

	bus_set(trans, WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_ADDR,
		WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_rxfifo0_busy_enable_MASK |
		WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_rxfifo1_busy_enable_MASK |
		WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_rxfifo2_busy_enable_MASK |
		WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_txfifo0_busy_enable_MASK |
		WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_txfifo1_busy_enable_MASK);

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
_mt7991_pdma_enable(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);

	bus_set(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_ADDR,
		WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_MASK |
		WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_MASK);
}

static void
_mt7991_pdma_disable(struct pci_trans *pci_hw)
{
	struct mtk_bus_trans *trans = to_bus_trans(pci_hw);

	bus_clear(trans, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_ADDR,
			WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_MASK |
			WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_MASK);
}

static int
mt7991_pdma_enable(struct pci_trans *trans)
{
	_mt7991_pdma_irq_enable(trans);
	_mt7991_pdma_unpause(trans);
	_mt7991_pdma_enable(trans);

	return 0;
}

static int
mt7991_pdma_disable(struct pci_trans *trans)
{
#define PDMA_POLL_ROUND 100
#define PDMA_POLL_US 1000
	int ret;

	_mt7991_pdma_disable(trans);
	_mt7991_pdma_pause(trans);
	ret = _mt7991_pdma_poll(trans, PDMA_POLL_ROUND, PDMA_POLL_US);
	if (ret)
		dev_err(to_device(trans), "%s(): Polling to idle failed\n", __func__);

	_mt7991_pdma_irq_disable(trans);

	return ret;
}

static int
mt7991_pdma_init(struct pci_trans *trans)
{
	dump_chip_info(trans);
	_mt7991_pdma_init(trans);
	_mt7991_pdma_irq_init(trans);

	return 0;
}

static bool
mt7991_pdma_match(struct pci_trans *master, struct pci_trans *slave)
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

static int
mt7991_chip_option_check(struct pci_trans *trans)
{
	struct mtk_hw_dev *hw_dev = to_hw_dev(trans);
	struct mtk_chip_drv *chip_drv = hw_dev->chip_drv;
	struct mtk_chip_option_table *chip_opt_desc =
		&chip_drv->chip_opt_table[hw_dev->chip_opt];

	if (!chip_opt_desc->bus_desc_slave.tx_ring_num) {
		dev_err(to_device(trans),
			"%s(): TxRing number in bus descriptor is 0\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int
mt7991_chip_option_set(struct pci_trans *trans)
{
	struct mtk_hw_dev *hw_dev = to_hw_dev(trans);
	struct mtk_chip_drv *chip_drv = hw_dev->chip_drv;
	struct mtk_chip_option_table *chip_opt_desc =
		&chip_drv->chip_opt_table[hw_dev->chip_opt];
	struct mtk_bus_desc *bus_desc = &chip_opt_desc->bus_desc_slave;
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

	return 0;
}

static int
mt7991_intr_option_check(struct pci_trans *trans)
{
	struct mtk_hw_dev *hw_dev = to_hw_dev(trans);
	struct mtk_chip_drv *chip_drv = hw_dev->chip_drv;
	struct mtk_bus_trans *bus_trans = to_bus_trans(trans);
	struct mtk_bus_cfg *bus_cfg = bus_trans->bus_cfg;
	struct mtk_intr_option_desc *intr_opt_desc;
	u8 intr_opt;

	intr_opt = chip_drv->intr_opt_set[hw_dev->intr_opt].intr_opt_slave;
	intr_opt_desc = (struct mtk_intr_option_desc *)
		bus_cfg->intr_opt_table + intr_opt;

	if (hw_dev->irq_type != intr_opt_desc->irq_type) {
		dev_err(to_device(trans),
			"%s(): Device's irq type (%u) isn't match descriptor's irq type (%u)\n",
			__func__, hw_dev->irq_type, intr_opt_desc->irq_type);
		return -EINVAL;
	}

	if (!intr_opt_desc->vec_num) {
		dev_err(to_device(trans),
			"%s(): Vector number in interrupt descriptor is 0\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int
mt7991_intr_option_set(struct pci_trans *trans)
{
	struct mtk_hw_dev *hw_dev = to_hw_dev(trans);
	struct mtk_chip_drv *chip_drv = hw_dev->chip_drv;
	struct mtk_bus_trans *bus_trans = to_bus_trans(trans);
	struct mtk_bus_cfg *bus_cfg = bus_trans->bus_cfg;
	struct mtk_intr_option_desc *intr_opt_desc;
	struct vector_data *vec_data;
	u8 intr_opt, nr = 0;

	intr_opt = chip_drv->intr_opt_set[chip_drv->intr_opt].intr_opt_slave;
	intr_opt_desc = (struct mtk_intr_option_desc *)
		bus_cfg->intr_opt_table + intr_opt;
	vec_data = intr_opt_desc->vec_data;

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
mt7991_option_init(struct pci_trans *trans)
{
	int ret;

	ret = mt7991_chip_option_check(trans);
	if (ret)
		goto end;

	ret = mt7991_chip_option_set(trans);
	if (ret)
		goto end;

	ret = mt7991_intr_option_check(trans);
	if (ret)
		goto end;

	ret = mt7991_intr_option_set(trans);
end:
	return ret;
}

#ifdef CONFIG_HWIFI_DBG_ISR
static int mt7991_dbg_isr_init(struct pci_trans *ptrans)
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

static void mt7991_dbg_isr_exit(struct pci_trans *ptrans)
{
	struct mtk_bus_trans *trans = to_bus_trans(ptrans);
	void *tmp;

	tmp = trans->bus_dbg.legacy_ring_irq_cnt;
	trans->bus_dbg.legacy_ring_irq_cnt = NULL;
	kfree(tmp);
}
#endif

static int mt7991_dbg_init(struct pci_trans *ptrans)
{
	int ret = 0;

#ifdef CONFIG_HWIFI_DBG_ISR
	ret = mt7991_dbg_isr_init(ptrans);
	if (ret)
		return ret;
#endif

	return ret;
}

static void mt7991_dbg_exit(struct pci_trans *ptrans)
{
#ifdef CONFIG_HWIFI_DBG_ISR
	mt7991_dbg_isr_exit(ptrans);
#endif
}

static struct pci_device_id mt7991_pci_device_table[] = {
	{ PCI_DEVICE(0x14c3, MT7991_CHIP_ID) },
	{ },
};

#ifdef CONFIG_HWIFI_REBB_SUPPORT
static struct mtk_mcu_dest_2_path mt7991_rebb_dest_2_path[MCU_DEST_MAX_NUM] = {
	{MCU_DEST_WM,		MCU_PATH_WA},
	{MCU_DEST_RVD,		MCU_PATH_WA},
	{MCU_DEST_WA,		MCU_PATH_WA},
	{MCU_DEST_WA_WM,	MCU_PATH_WA},
	{MCU_DEST_WO,		MCU_PATH_WO},
};

static struct pci_chip_ops mt7991_rebb_pci_ops = {
	.dma_enable = mt7991_pdma_enable,
	.dma_disable = mt7991_pdma_disable,
	.dma_init = mt7991_pdma_init,
	.dma_match = mt7991_pdma_match,
	.dma_schdl_init = NULL,
	.rro_hw_init = NULL,
	.option_init = mt7991_option_init,
	.dbg_init = mt7991_dbg_init,
	.dbg_exit = mt7991_dbg_exit,
};

static struct mtk_chip_drv mt7991_rebb_drv = {
	.bus_cfg = {
		.bus_type = MTK_BUS_PCI,
		.id_table = mt7991_pci_device_table,
		.name = KBUILD_MODNAME,
		.profile = &mt7991_pci_wa_profile,
		.ms_type = CHIP_TYPE_SLAVE,
		.bus_ops = &mt7991_rebb_pci_ops,
		.bus_drv_sz = sizeof(struct pci_driver),
		.intr_opt_table = mt7991_intr_opt_tbl,
		.intr_opt_table_sz = ARRAY_SIZE(mt7991_intr_opt_tbl),
	},
	.ctl_ops = NULL,
	.hw_caps = NULL,
	.device_id = MT7991_CHIP_ID,
	.dest_2_path = mt7991_rebb_dest_2_path,
};
#endif /*CONFIG_HWIFI_REBB_SUPPORT*/

static int __init mtk_chip_mt7991_init(void)
{
	static struct mtk_chip_drv *drv;

#ifdef CONFIG_HWIFI_REBB_SUPPORT
	drv = &mt7991_rebb_drv;
#endif
	drv->interface_type = mac_type;
#ifdef CONFIG_HWIFI_HIF_WED
	drv->bus_cfg.bus_type = MTK_BUS_WED;
	drv->bus_cfg.bus_sub_type = MTK_BUS_PCI;
	drv->bus_cfg.bus_drv_sz = sizeof(struct wed_driver);
#endif
	return mtk_chip_register(drv);
}

static void __exit mtk_chip_mt7991_exit(void)
{
	static struct mtk_chip_drv *drv;

#ifdef CONFIG_HWIFI_REBB_SUPPORT
	drv = &mt7991_rebb_drv;
#endif

	mtk_chip_unregister(drv);
}

module_init(mtk_chip_mt7991_init);
module_exit(mtk_chip_mt7991_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Mediatek Hardware Wi-Fi Module");
