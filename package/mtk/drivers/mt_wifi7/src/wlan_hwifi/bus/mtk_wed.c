/*
 * Copyright (c) [2020] MediaTek Inc.
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
#include <warp.h>
#include "mtk_wed.h"
#include "mtk_rro.h"

#ifndef WARP_VERSION_CODE
#define WARP_VERSION_CODE		0x0000
#define WARP_VERSION(VER, SUB_VER)	(((VER) << 8) + (SUB_VER))
#endif

/*initial function*/
static u32
wed_io_read(void *hw, u32 offset)
{
	struct wed_trans *trans = to_wed_trans(hw);
	int ret;
	u32 val;

	ret = warp_proxy_read(hw, offset, &val);

	if (ret == WED_INVAL)
		val = trans->io_ops->rr(hw, offset);

	return val;
}

static void
wed_io_write(void *hw, u32 offset, u32 val)
{
	struct wed_trans *trans = to_wed_trans(hw);
	int ret;

	ret = warp_proxy_write(hw, offset, val);

	if (ret == WED_INVAL)
		trans->io_ops->wr(hw, offset, val);
}

static u32
wed_io_dbg_read(void *hw, u32 offset)
{
	struct wed_trans *trans = to_wed_trans(hw);

	return trans->io_ops->rr(hw, offset);
}

static void
wed_io_dbg_write(void *hw, u32 offset, u32 val)
{
	struct wed_trans *trans = to_wed_trans(hw);

	trans->io_ops->wr(hw, offset, val);
}

static const struct mtk_bus_io_ops wed_io_ops = {
	.rr = wed_io_read,
	.wr = wed_io_write,
	.rr_dbg = wed_io_dbg_read,
	.wr_dbg = wed_io_dbg_write,
};

/*wed callback functions*/
static void
wed_config_atc(void *priv_data, bool enable)
{
	struct mtk_bus_trans *trans = to_bus_trans(priv_data);
	struct wed_trans *wed = to_wed_trans(priv_data);

	if (enable)
		trans->io_ops = wed->io_ops;
	else
		trans->io_ops = &wed_io_ops;
}

static void
wed_request_irq(void *priv_data, u32 irq)
{
	struct wed_trans *wed = to_wed_trans(priv_data);
	struct pci_trans *trans = to_pci_trans(priv_data);
	char *name;
	int ret;

	wed->irq = irq;
	name = wed->irq_name;

	ret = snprintf(name, MAX_IRQ_NAME_LEN, "%s-wed", to_name(trans));
	if (ret < 0) {
		dev_err(to_device(trans), "%s(): Name error\n", __func__);
		goto end;
	}

	ret = request_irq(irq, mtk_bus_dma_irq_handler,
		IRQF_SHARED, name, &trans->vec_data[0]);
end:
	if (ret)
		dev_err(to_device(trans), "%s(): Request wed irq failed\n", __func__);
}

static void
wed_free_irq(void *priv_data, u32 irq)
{
	struct wed_trans *wed = to_wed_trans(priv_data);
	struct pci_trans *trans = to_pci_trans(priv_data);

	if (irq == wed->irq)
		free_irq(wed->irq, &trans->vec_data[0]);
	else
		mtk_dbg(MTK_BUS, "warp_irq(%d) != wed->irq(%d)\n", __func__);
}

static void
wed_txinfo_wrapper(u8 *tx_info, struct wlan_tx_info *info)
{
	struct mtk_bus_tx_info *bus_tx = (struct mtk_bus_tx_info *)tx_info;
	struct wed_trans *trans = to_wed_trans(bus_tx->txq_trans);
	struct mtk_tk_entry *tk_entry = (struct mtk_tk_entry *)bus_tx->tk_entries;

	info->pkt = (u8 *)tk_entry->tx_q.next;
	info->bssidx = tk_entry->bss_idx;
	/* TODO: Warp decides from hw_bss->hw_phy->band_idx */
	if (trans->wed_ver == 1)
		info->ringidx = tk_entry->band_idx;
	else
		info->ringidx = 0;
	info->wcid = tk_entry->wcid;
	info->usr_info = 0x0;
	info->tid = tk_entry->tid;
	info->is_fixedrate = tk_entry->is_fixed_rate;
	info->is_prior = tk_entry->is_prior;
	info->is_sp = tk_entry->is_sp;
	info->hf = tk_entry->hf;

	info->amsdu_en = tk_entry->amsdu_en;

	mtk_dbg(MTK_BUS, "%s(): pkt %p, bssidx %d, ringidx %d, wcid %d\n", __func__,
		info->pkt, info->bssidx, info->ringidx, info->wcid);

	mtk_dbg(MTK_BUS, "usr_info = %d, tid = %d, is_fixedrate = %d,\
		is_prior = %d, is_sp = %d, hf = %d, amsdu_en = %d\n",
		info->usr_info, info->tid, info->is_fixedrate,
		info->is_prior, info->is_sp, info->hf, info->amsdu_en);
}

static void
wed_txinfo_set_drop(u8 *tx_info)
{
	struct mtk_bus_tx_info *bus_tx = (struct mtk_bus_tx_info *) tx_info;

	set_bit(BUS_TX_DROP, &bus_tx->flags);

	mtk_dbg(MTK_BUS, "%s(): %d\n", __func__, __LINE__);
}

static bool
wed_hw_tx_allow(u8 *tx_info)
{
	struct mtk_bus_tx_info *bus_tx = (struct mtk_bus_tx_info *) tx_info;
	struct mtk_tk_entry *tk_entry = (struct mtk_tk_entry *)bus_tx->tk_entries;
	struct wed_trans *trans = to_wed_trans(bus_tx->txq_trans);
	struct wifi_hw *hw = (struct wifi_hw *) trans->wifi_hw;

	if ((test_bit(BUS_TX_SW, &bus_tx->flags)) || (tk_entry->wcid >= hw->max_wcid_nums))
		return false;

	mtk_dbg(MTK_BUS, "%s(): %d\n", __func__, __LINE__);
	return true;
}

static void
dump_raw(char *str, u8 *va, u32 size)
{
	u8 *pt;
	char buf[512] = "";
	u32 len = 0;
	int append_len = 0;
	int x;

	pt = va;
	pr_info("%s: %p, len = %d\n", str, va, size);

	for (x = 0; x < size; x++) {
		if (x % 16 == 0) {
			append_len = snprintf(buf + len, sizeof(buf) - len, "\n0x%04x : ", x);
			if (append_len > 0)
				len = strlen(buf);
		}

		append_len = snprintf(buf + len, sizeof(buf) - len, "%02x ", ((u8)pt[x]));
		if (append_len > 0)
			len = strlen(buf);
	}

	pr_info("%s\n", buf);
}

static void
wed_tx_ring_info_dump(void *priv_data, u8 ring_id, u32 idx)
{
	struct pci_trans *trans = to_pci_trans(priv_data);
	struct pci_queue *q = NULL;
	struct pci_dma_cb *cb;
	int i = 0;

	for (i = 0 ; i< trans->txq_num; i++) {
		struct pci_queue *tq = &trans->txq[i].q;

		if (tq->q_attr == Q_TX_DATA && tq->band_idx_bmp & BIT(ring_id)) {
			q = tq;
			break;
		}
	}

	if (!q || idx >= q->desc_size)
		return;

	cb = &q->cb[idx];

	pr_info("AllocPA\t: %pad\n", &cb->alloc_pa);
	pr_info("AllocVa\t: %p\n", cb->alloc_va);
	pr_info("Size\t: %zu\n", cb->alloc_size);
	pr_info("PktPtr\t: %p\n", cb->pkt);
	dump_raw("WED_TX_RING", cb->alloc_va, cb->alloc_size);
}

static int
wed_get_hif_txd_ver(struct mtk_hw_dev *dev, u8 ver, u8 subver)
{
	u32 i;
	u32 list_size = ARRAY_SIZE(wed_hif_txd_info_list);

	for (i = 0; i < list_size; i++) {
		u8 entry_ver = wed_hif_txd_info_list[i].warp_ver;
		u8 entry_subver = wed_hif_txd_info_list[i].warp_subver;

		if (ver == entry_ver && subver == entry_subver) {
			/* TODO: Cross check final version between platform and nic */
			dev->hif_txd_ver_sdo = wed_hif_txd_info_list[i].hif_txd_ver;
			return 0;
		}
	}

	dev_err(dev->dev, "%s(): WED ver = %u, subver = %u not found\n",
		__func__, ver, subver);
	return -EINVAL;
}

static void
wed_chk_hif_txd_ver(struct mtk_hw_dev *dev)
{
	if (dev->hif_txd_ver_sdo >= HIF_TXD_V2_0)
		set_bit(HWIFI_FLAG_PARSE_TX_PAYLOAD, &dev->flags);
}

static void
wed_warp_rx_data_cfg(struct pci_trans *ptrans)
{
	u32 i;

	for (i = 0; i < ptrans->rxq_num; i++) {
		struct pci_rx_queue *rxq = &ptrans->rxq[i];
		struct pci_queue *q = &rxq->q;

		if (q->disable)
			continue;

		if (q->q_attr == Q_RX_DATA) {
			set_bit(RQ_FLAG_RX_DATA_COPY, &rxq->rq_flags);
			set_bit(RQ_FLAG_RX_DATA_TKID, &rxq->rq_flags);
		}
	}
}


static void
wed_warp_rx_data_wed_cfg(struct wifi_hw *wed_hw,
		struct pci_trans *ptrans, bool copy_mode)
{
	u32 i;

	for (i = 0; i < ptrans->rxq_num; i++) {
		struct pci_rx_queue *rxq = &ptrans->rxq[i];
		struct pci_queue *q = &rxq->q;

		if (q->q_attr == Q_RX_DATA_WED) {
			q->disable = false;
			ptrans->vec_data[q->vec_id].int_ena_mask |= q->hw_int_mask;

			if (copy_mode) {
				set_bit(RQ_FLAG_RX_DATA_COPY, &rxq->rq_flags);
				set_bit(RQ_FLAG_RX_DATA_TKID, &rxq->rq_flags);
			}
		}
	}
}

static void
wed_warp_ver_notify(void *priv_data, u8 ver, u8 warp_sub_ver,
		u8 warp_ver_brnach, int hw_cap)
{
	struct mtk_hw_dev *dev = to_hw_dev(priv_data);
	struct pci_trans *trans = to_pci_trans(priv_data);
	struct wed_trans *wed_trans = to_wed_trans(trans);

	/* run in wed start, will update to notify mac driver*/
	mtk_dbg(MTK_BUS, "%s(): ver %d\n", __func__, ver);

	wed_trans->wed_ver = ver;
	wed_trans->wed_sub_ver = warp_sub_ver;

	if (hw_cap & WED_HW_CAP_RRO_DBG_MODE)
		set_bit(PCI_FLAG_WHNAT_RRO_DBG, &trans->flags);
	if (hw_cap & WED_HW_CAP_RRO_DBG_MEMCPY_MODE)
		set_bit(PCI_FLAG_WHNAT_RRO_DBG_MEMCPY, &trans->flags);
	if (hw_cap & WED_HW_CAP_RX_OFFLOAD)
		set_bit(HWIFI_FLAG_RX_OFFLOAD, &dev->flags);
	if (hw_cap & WED_HW_CAP_RX_WOCPU)
		set_bit(HWIFI_FLAG_BA_OFFLOAD, &dev->flags);
}

static u32
wed_token_rx_dmad_init(void *priv_data, void *pkt,
	unsigned long len, void *va,
	dma_addr_t pa)
{
	struct mtk_hw_dev *dev = to_hw_dev(priv_data);
	struct mtk_rx_tk_mgmt *tk = &dev->hw_ctrl.rx_tk_mgmt;
	u32 id;

	id = mtk_rx_tk_request_entry(tk, pkt,
		len, va, pa);

	return id;
}

static int
wed_token_rx_dmad_lookup(void *priv_data, u32 id,
	void **pkt, void **va, dma_addr_t *pa)
{
	struct mtk_hw_dev *dev = to_hw_dev(priv_data);
	struct mtk_rx_tk_mgmt *tk = &dev->hw_ctrl.rx_tk_mgmt;

	return mtk_rx_tk_entry_find(tk, id, pkt, va, pa);
}

static void
wed_rxinfo_wrapper(u8 *rx_info, struct wlan_rx_info *wlan_info)
{
	struct mtk_bus_rx_info *_rx_info = (struct mtk_bus_rx_info *) rx_info;

	wlan_info->pkt = (u8 *)_rx_info->skb;
	wlan_info->ppe_entry = _rx_info->ppe_entry;
	wlan_info->csrn = _rx_info->csrn;

	mtk_dbg(MTK_BUS, "%s(): pkt %p, ppe_entry %d, csrn %d\n",
		__func__, wlan_info->pkt, wlan_info->ppe_entry, wlan_info->csrn);
}

static void
wed_do_wifi_reset(void *priv_data)
{
	mtk_dbg(MTK_BUS, "%s(): %d\n", __func__, __LINE__);
}

static void wed_update_wo_rxcnt(void *priv_data, void *wo_rxcnt)
{
	mtk_dbg(MTK_BUS, "%s(): %d\n", __func__, __LINE__);
}

static void
wed_hb_check_notify(void *priv_data)
{
	struct mtk_hw_dev *dev = to_hw_dev(priv_data);

	mtk_bus_rx_ser_event(
		dev,
		dev->chip_drv->device_id,
		HW_SER_LV_0_5,
		0,
		dev->sys_idx.idx);
}

static void
wed_fbuf_init(u8 *fbuf, u32 pkt_pa, u32 tkid)
{
	if (fbuf)
		fmac_hif_txd_init(fbuf, pkt_pa, tkid);
}

static void
wed_fbuf_v1_init(u8 *fbuf, dma_addr_t pkt_pa, u32 tkid, u8 src)
{
	if (fbuf)
		fmac_hif_txd_v1_init(fbuf, pkt_pa, tkid, src);
}

static struct wifi_ops wed_ops = {
	.config_atc = wed_config_atc,
	.fbuf_init = wed_fbuf_init,
	.txinfo_wrapper = wed_txinfo_wrapper,
	.txinfo_set_drop = wed_txinfo_set_drop,
	.hw_tx_allow = wed_hw_tx_allow,
	.tx_ring_info_dump = wed_tx_ring_info_dump,
	.warp_ver_notify = wed_warp_ver_notify,
	.token_rx_dmad_init = wed_token_rx_dmad_init,
	.token_rx_dmad_lookup = wed_token_rx_dmad_lookup,
	.rxinfo_wrapper = wed_rxinfo_wrapper,
	.do_wifi_reset = wed_do_wifi_reset,
	.update_wo_rxcnt = wed_update_wo_rxcnt,
	.request_irq = wed_request_irq,
	.free_irq = wed_free_irq,
	.hb_check_notify = wed_hb_check_notify,
	.fbuf_v1_init = wed_fbuf_v1_init,
};

/*TODO: here only support pci need to more flexity to support*/
/*multi-type HIF*/
static int
wed_pci_host_get(struct pci_trans *ptrans, struct wifi_hw *hw)
{
	struct pci_dev *pdev = ptrans->pdev;

	hw->hif_type = BUS_TYPE_PCIE;
	hw->base_addr = (unsigned long) ptrans->regs;
	hw->base_phy_addr = pci_resource_start(pdev, 0);
	hw->wpdma_base = hw->base_phy_addr | hw->dma_offset;
	return 0;
}

static void
wed_pci_queue_set(struct pci_queue *q, struct ring_ctrl *ring)
{
	ring->base = q->hw_desc_base;
	ring->cnt = q->hw_desc_base + 0x4;
	ring->cidx = q->hw_desc_base + 0x8;
	ring->didx = q->hw_desc_base + 0xC;
	ring->lens = q->q_size;
}

static void
wed_pci_rxq_desc_get(struct pci_queue *q,  struct ring_ctrl *ring)
{
	ring->cb_alloc_pa = q->desc_ring.alloc_pa;
}

static void
wed_pci_rx_queue_set(struct pci_rx_queue *rxq, struct ring_ctrl *ring)
{
	ring->attr_enable = rxq->attr_enable;
	ring->attr_mask = rxq->q.hw_attr_ena_mask;
}

static int
wed_pci_txq_get(struct pci_trans *ptrans, struct wifi_hw *hw)
{
	u32 i;
	u32 cnt = 0;

	for (i = 0; i < ptrans->txq_num; i++) {
		struct pci_queue *q = &ptrans->txq[i].q;

		if (q->q_attr != Q_TX_DATA || q->disable)
			continue;

		hw->txd_size = q->desc_size;
		/* support WED2.x ring size control */
		hw->tx_ring_size = q->q_size;
		/* WED3.x support pert ring control ring size */
		wed_pci_queue_set(q, &hw->tx[cnt]);

		if (cnt == 0)
			hw->wfdma_tx_done_trig0_bit = ffs(q->hw_int_mask)-1;
		else
			hw->wfdma_tx_done_trig1_bit = ffs(q->hw_int_mask)-1;

		cnt++;
	}
	hw->tx_ring_num = cnt;
	return 0;
}

static int
wed_pci_rxq_get(struct pci_trans *ptrans, struct wifi_hw *hw)
{
	struct mtk_hw_dev *hw_dev = to_hw_dev(ptrans);
	struct mtk_bus_trans *bus_trans = to_bus_trans(ptrans);
	u32 i, cnt = 0;
	bool is_free_notify_set = false;
	bool is_master = test_bit(BUS_TRANS_FLAG_MASTER, &bus_trans->flag);

	for (i = 0; i < ptrans->rxq_num; i++) {
		struct pci_rx_queue *rxq = &ptrans->rxq[i];
		struct pci_queue *q = &rxq->q;

		if (q->disable)
			continue;

		if (q->q_attr == Q_RX_DATA) {
			/* support WED2.x ring size control */
			hw->rx_data_ring_size = q->q_size;
			hw->rxd_size = q->desc_size;
			hw->rx_pkt_size = rxq->rx_buf_size;
			/* WED3.x support pert ring control ring size in queue_set() */
			wed_pci_queue_set(q, &hw->rx[cnt]);
			wed_pci_rx_queue_set(rxq, &hw->rx[cnt]);

			if (cnt == 0)
				hw->wfdma_rx_done_trig0_bit = ffs(q->hw_int_mask)-1;
			else
				hw->wfdma_rx_done_trig1_bit = ffs(q->hw_int_mask)-1;

			cnt++;
		} else if (q->q_attr == Q_RX_EVENT_TX_FREE_DONE) {
			if (hw_dev->txfreedone_path != rxq->txfreedone_path) {
				dev_info(to_device(ptrans),
					"%s(): hw_dev->txfreedone_path (%u) != rxq->txfreedone_path (%u), skip\n",
					__func__, hw_dev->txfreedone_path, rxq->txfreedone_path);
				continue;
			} else if (is_free_notify_set) {
				dev_err(to_device(ptrans), "%s(): More than one TxFreeDone ring\n",
					__func__);
				continue;
			} else if ((is_master && rxq->free_done_handled_by == SLAVE_HANDLER) ||
				   (!is_master && rxq->free_done_handled_by == MASTER_HANDLER)) {
				dev_info(to_device(ptrans), "%s(): Skip %s ring since handler mismatch\n",
					__func__, rxq->q.q_info);
				continue;
			}

			/* support WED2.x ring size control */
			hw->rx_ring_size  = q->q_size;
			/* event ring for tx free notify */
			wed_pci_queue_set(q, &hw->event);
			hw->wfdma_tx_done_free_notify_trig_bit = ffs(q->hw_int_mask)-1;
			is_free_notify_set = true;
		}
	}

	if (cnt == 0 || is_free_notify_set == false) {
		dev_err(to_device(ptrans), "%s(): [Value(s) not expected] cnt = %u, is_free_notify_set = %s\n",
			__func__, cnt, is_free_notify_set ? "true" : "false");
		return -EINVAL;
	}

	hw->rx_ring_num = cnt;
	return 0;
}

#if (WARP_VERSION(3, 1) <= WARP_VERSION_CODE)
static int
wed_pci_wed_rxq_get(struct pci_trans *ptrans, struct wifi_hw *hw)
{
	u32 i, cnt = 0;

	for (i = 0; i < ptrans->rxq_num; i++) {
		struct pci_rx_queue *rxq = &ptrans->rxq[i];
		struct pci_queue *q = &rxq->q;

		if (q->q_attr == Q_RX_DATA_WED) {
			wed_pci_queue_set(q, &hw->rx_wed[cnt]);

			if (cnt == 0)
				hw->rx_wed_done_trig0_bit = ffs(q->hw_int_mask)-1;
			else
				hw->rx_wed_done_trig1_bit = ffs(q->hw_int_mask)-1;

			cnt++;
		}
	}

	hw->rx_wed_ring_num = cnt;
	return 0;
}

static int
wed_pci_rxdmad_c_q_get(struct pci_trans *ptrans, struct wifi_hw *hw)
{
	u32 i;
	u32 cnt = 0;

	for (i = 0; i < ptrans->rxq_num; i++) {
		struct pci_rx_queue *rxq = &ptrans->rxq[i];
		struct pci_queue *q = &rxq->q;

		if (q->disable)
			continue;

		if (q->q_attr == Q_RX_RXDMAD_C) {
			wed_pci_queue_set(q, &hw->rx_rro_3_1);
			wed_pci_rxq_desc_get(q, &hw->rx_rro_3_1);

			if (cnt == 0)
				hw->wfdma_rro_rx_3_1_trig_bit = ffs(q->hw_int_mask)-1;

			cnt++;
		}
	}
	hw->rx_rro_3_1_ring_num = cnt;
	return 0;
}
#endif

static void
wed_v1_compatible_mode_para_get(struct wifi_hw *hw, struct pci_chip_profile *p)
{
	hw->wed_v1_compatible_en_addr = p->compatible_config.en_addr;
	hw->wed_v1_compatible_en_msk = p->compatible_config.en_msk;
	hw->wed_v1_compatible_tx0_addr = p->compatible_config.tx0_addr;
	hw->wed_v1_compatible_tx0_msk = p->compatible_config.tx0_msk;
	hw->wed_v1_compatible_tx0_id = p->compatible_config.tx0_id;
	hw->wed_v1_compatible_tx1_addr = p->compatible_config.tx1_addr;
	hw->wed_v1_compatible_tx1_msk = p->compatible_config.tx1_msk;
	hw->wed_v1_compatible_tx1_id = p->compatible_config.tx1_id;
	hw->wed_v1_compatible_rx1_addr = p->compatible_config.rx1_addr;
	hw->wed_v1_compatible_rx1_msk = p->compatible_config.rx1_msk;
	hw->wed_v1_compatible_rx1_id = p->compatible_config.rx1_id;
}

static void
wed_set_free_notify_io_mode(void *hw)
{
	struct pci_trans *trans = to_pci_trans(hw);
	u8 idx, rxn = trans->rxq_num;

	for (idx = 0; idx < rxn; idx++) {
		struct pci_rx_queue *rq = &trans->rxq[idx];

		if (rq->q.q_attr == Q_RX_EVENT_TX_FREE_DONE)
			rq->get_pkt_method = GET_PKT_IO;
	}
}

static int
wed_pci_rro_data_q_get(struct pci_trans *ptrans, struct wifi_hw *hw)
{
	u32 i;
	u32 cnt = 0;

	for (i = 0; i < ptrans->rxq_num; i++) {
		struct pci_rx_queue *rxq = &ptrans->rxq[i];
		struct pci_queue *q = &rxq->q;

		if (q->disable)
			continue;

		if (q->q_attr == Q_RX_DATA_RRO) {
			wed_pci_queue_set(q, &hw->rx_rro[cnt]);
			wed_pci_rxq_desc_get(q, &hw->rx_rro[cnt]);

			if (cnt == 0)
				hw->wfdma_rro_rx_done_trig0_bit = ffs(q->hw_int_mask)-1;
			else
				hw->wfdma_rro_rx_done_trig1_bit = ffs(q->hw_int_mask)-1;

			cnt++;
		}
	}
	hw->rx_rro_data_ring_num = cnt;
	return 0;
}

static int
wed_pci_rro_page_q_get(struct pci_trans *ptrans, struct wifi_hw *hw)
{
	u32 i;
	u32 cnt = 0;

	for (i = 0; i < ptrans->rxq_num; i++) {
		struct pci_rx_queue *rxq = &ptrans->rxq[i];
		struct pci_queue *q = &rxq->q;

		if (q->disable)
			continue;

		if (q->q_attr == Q_RX_DATA_MSDU_PG) {
			wed_pci_queue_set(q, &hw->rx_rro_pg[cnt]);
			wed_pci_rxq_desc_get(q, &hw->rx_rro_pg[cnt]);

			if (cnt == 0x0)
				hw->wfdma_rro_rx_pg_trig0_bit = ffs(q->hw_int_mask)-1;
			else if (cnt == 0x1)
				hw->wfdma_rro_rx_pg_trig1_bit = ffs(q->hw_int_mask)-1;
			else
				hw->wfdma_rro_rx_pg_trig2_bit = ffs(q->hw_int_mask)-1;

			hw->rx_page_size = rxq->rx_buf_size;
			cnt++;
		}
	}
	hw->rx_rro_page_ring_num = cnt;
	return 0;
}

static int
wed_pci_ind_cmd_q_get(struct pci_trans *ptrans, struct wifi_hw *hw)
{
	u32 i;
	u32 cnt = 0;

	for (i = 0; i < ptrans->rxq_num; i++) {
		struct pci_rx_queue *rxq = &ptrans->rxq[i];
		struct pci_queue *q = &rxq->q;

		if (q->disable)
			continue;

		if (q->q_attr == Q_RX_IND) {
			wed_pci_queue_set(q, &hw->rx_rro_ind_cmd);
			wed_pci_rxq_desc_get(q, &hw->rx_rro_ind_cmd);

			cnt++;
		}
	}
	hw->rx_rro_ind_cmd_ring_num = cnt;
	return 0;
}

static int
wed_rro_info_get(struct pci_trans *ptrans, struct wifi_hw *hw)
{
	struct hw_rro_cfg *rro_cfg = ptrans->rro_cfg;
	int i;

	hw->rro_ctl.max_win_sz = rro_cfg->win_sz;
	hw->rro_ctl.ack_sn_cr = rro_cfg->ack_sn_addr;
	hw->rro_ctl.particular_se_id = rro_cfg->particular_se_id;
	hw->rro_ctl.particular_se_base = rro_cfg->particular_session_pa;
	hw->rro_ctl.se_group_num = HW_RRO_ADDR_ELEM_CR_CNT;

	for (i = 0; i < hw->rro_ctl.se_group_num; i++)
		hw->rro_ctl.se_base[i] = rro_cfg->addr_elem_alloc_pa[i];

	return 0;
}

static int
wed_pci_cap_get(struct pci_trans *ptrans, struct mtk_hw_dev *hw_dev,
					struct wifi_hw *hw)
{
	struct mtk_bus_trans *bus_trans = to_bus_trans(ptrans);
	struct mtk_chip_drv *chip_drv = hw_dev->chip_drv;
	struct mtk_bus_cfg *bus_cfg = &chip_drv->bus_cfg;
	struct pci_chip_profile *p = (struct pci_chip_profile *)bus_cfg->profile;

	/*basic information*/
	hw->chip_id = ptrans->id;
	hw->hif_dev = ptrans->pdev;
	hw->coherent_dma_addr_size = p->coherent_dma_addr_size;
	hw->non_coherent_dma_addr_size = p->non_coherent_dma_addr_size;
	hw->msi_enable = hw_dev->irq_type == PCI_IRQ_MSI ? true : false;
	/* others */
	hw->p_int_mask = &ptrans->int_mask_map[0].int_ena_mask;
	hw->int_sta = ptrans->int_mask_map[0].int_sts_addr;
	hw->int_mask = ptrans->int_mask_map[0].int_ena_addr;
	hw->tx_dma_glo_cfg = ptrans->tx_dma_glo_reg_addr;
	hw->int_ser = ptrans->int_ser_reg_addr;
	hw->int_ser_value = ptrans->int_ser_mask;
	hw->rx_dma_glo_cfg = ptrans->rx_dma_glo_reg_addr;
	hw->dma_offset = 0xd7000;
	hw->ring_offset = 0x10;
	hw->max_amsdu_nums = 8;
	hw->max_amsdu_len = 1536;
	hw->rm_vlan = true;
	hw->hdtr_mode = true;
	hw->tx_pkt_size = MT_TX_BUF_LEN;
	hw->fbuf_size = 128;
	/* get hif specific profile */
	wed_pci_txq_get(ptrans, hw);
	wed_pci_rxq_get(ptrans, hw);
#if (WARP_VERSION(3, 1) <= WARP_VERSION_CODE)
	wed_pci_wed_rxq_get(ptrans, hw);
#endif
	wed_v1_compatible_mode_para_get(hw, p);

	if (chip_drv->hw_caps->mac_cap & BIT(CAP_RRO) &&
	    test_bit(BUS_TRANS_FLAG_MASTER, &bus_trans->flag)) {
		wed_pci_rro_data_q_get(ptrans, hw);
		wed_pci_rro_page_q_get(ptrans, hw);
		wed_pci_ind_cmd_q_get(ptrans, hw);
#if (WARP_VERSION(3, 1) <= WARP_VERSION_CODE)
		wed_pci_rxdmad_c_q_get(ptrans, hw);
#endif
		wed_rro_info_get(ptrans, hw);

		hw->hw_cap &= ~BIT(WIFI_HW_CAP_RRO);
#if (WARP_VERSION(3, 1) <= WARP_VERSION_CODE)
		hw->hw_cap &= ~BIT(WIFI_HW_CAP_RRO_3_1);
#endif
		switch (ptrans->rro_mode) {
		case HW_RRO_V3_0_BUF_PG:
		case HW_RRO_V3_0_BUF_PG_DBG:
		case HW_RRO_V3_1_BUF_PG_EMUL:
			hw->hw_cap |= BIT(WIFI_HW_CAP_RRO);
			break;
		case HW_RRO_V3_1:
#if (WARP_VERSION(3, 1) <= WARP_VERSION_CODE)
			hw->hw_cap |= BIT(WIFI_HW_CAP_RRO_3_1);
#endif
			break;
		}
	}

	wed_pci_host_get(ptrans, hw);
	return 0;
}

static int
wed_set_device(struct wifi_hw *wed_hw, void *hw)
{
	struct mtk_hw_dev *dev = to_hw_dev(hw);
	struct wed_trans *wed_trans = to_wed_trans(hw);
	struct pci_trans *ptrans = to_pci_trans(hw);
	u8 ver = wed_trans->wed_ver;
	u8 sub_ver = wed_trans->wed_sub_ver;

	switch (ver) {
	case 1:
		wed_set_free_notify_io_mode(hw);
		wed_hw->max_wcid_nums = 256;
		set_bit(PCI_FLAG_WHNAT_TX, &ptrans->flags);
		break;
	case 2:
		wed_warp_rx_data_cfg(ptrans);
		wed_hw->max_wcid_nums = 1024;
		set_bit(PCI_FLAG_WHNAT_TX, &ptrans->flags);
		set_bit(PCI_FLAG_WHNAT_RX, &ptrans->flags);
		wed_trans->dma_ops->set_queue_ops(hw);
		break;
	case 3:
		if (sub_ver == 0)
			wed_warp_rx_data_cfg(ptrans);
		else if (sub_ver == 1)
			wed_warp_rx_data_wed_cfg(wed_hw, ptrans, true);

		wed_hw->max_wcid_nums = 1024;
		set_bit(PCI_FLAG_WHNAT_TX, &ptrans->flags);
		set_bit(PCI_FLAG_WHNAT_RX, &ptrans->flags);
		wed_trans->dma_ops->set_queue_ops(hw);
		break;
	}

	if (wed_get_hif_txd_ver(dev, ver, sub_ver) == 0)
		wed_chk_hif_txd_ver(dev);

	return 0;
}

/* TODO: hwifi not support ser and suspend yet.*/
/*
client_ser_handler(ad, priv);
warp_suspend_handler(ad);
warp_resume_handler(ad);
*/

static irqreturn_t
wed_irq_handler(int irq, void *data)
{
	struct wed_trans *trans = to_wed_trans_dp(data);

	if (trans->enable)
		warp_isr_handler(trans);

	return trans->dma_ops->irq_handler(irq, data);
}

static int
wed_alloc_resource(void *hw, void *profile)
{
	struct wed_trans *trans = to_wed_trans(hw);

	return trans->dma_ops->alloc_resource(hw, profile);
}

static void
wed_free_resource(void *hw)
{
	struct wed_trans *trans = to_wed_trans(hw);

	trans->dma_ops->free_resource(hw);
}

static int
wed_preinit_device(void *hw)
{
	struct wed_trans *trans = to_wed_trans(hw);

	return trans->dma_ops->preinit_device(hw);
}

static int
wed_init_device(void *hw)
{
	struct wed_trans *trans = to_wed_trans(hw);
	struct pci_trans *ptrans = to_pci_trans(hw);
	struct mtk_bus_trans *bus_trans = to_bus_trans(hw);
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct mtk_chip_hw_cap *hw_cap = dev->chip_drv->hw_caps;
	struct wifi_hw *wed_hw;
	bool is_master = test_bit(BUS_TRANS_FLAG_MASTER, &bus_trans->flag);
	int ret = 0;

	/*initial as false*/
	trans->enable = false;

	if (ptrans->offload_disable)
		goto end;

	/*allocate a wed hw*/
	wed_hw = warp_alloc_client(ptrans->id, trans->bus_type,
				   0, 0, ptrans->pdev);

	trans->wifi_hw = wed_hw;

	mtk_dbg(MTK_BUS, "%s(): wed_hw %p\n", __func__, wed_hw);
	if (!wed_hw)
		goto end;

	wed_hw->priv = hw;

	wed_hw->tx_buf_nums = is_master ?
		dev->ext_tx_buf_nums_master :
		dev->ext_tx_buf_nums_slave;
	wed_hw->tx_token_nums = is_master ?
		dev->ext_tx_tk_nums_master :
		dev->ext_tx_tk_nums_slave;
	wed_hw->hw_rx_token_num = is_master ?
		dev->ext_rx_tk_nums_master :
		dev->ext_rx_tk_nums_slave;

	wed_hw->tx_free_done_ver = hw_cap->hwres->tx_free_done_ver;
	wed_hw->max_rxd_size = hw_cap->mrxd_sz;
	wed_hw->mac_ver = hw_cap->mac_type;

	if (is_master)
		wed_hw->src = MASTER_SRC;
	else
		wed_hw->src = SLAVE_SRC;

	wed_pci_cap_get(ptrans, dev, wed_hw);
	if (!test_bit(HWIFI_STATE_RESET, &dev->state))
		ret = warp_register_client(wed_hw, &wed_ops);

	trans->enable = (ret == WED_INVAL) ? false : true;
	if (trans->enable)
		wed_set_device(wed_hw, hw);

end:
	return trans->dma_ops->init_device(hw);
}

static void
wed_exit_device(void *hw)
{
	struct wed_trans *trans = to_wed_trans(hw);
	struct mtk_hw_dev *dev = to_hw_dev(trans);

	trans->dma_ops->exit_device(hw);

	if (trans->enable && !test_bit(HWIFI_STATE_RESET, &dev->state))
		warp_client_remove(hw);
}

static void
wed_set_rro_sign_addr(void *hw)
{
	struct wed_trans *trans = to_wed_trans(hw);
	struct pci_trans *ptrans = to_pci_trans(hw);
	struct mtk_bus_trans *bus_trans = to_bus_trans(hw);
	struct hw_rro_cfg *rro_cfg = ptrans->rro_cfg;
	u32 cr_value = 0;

	if (trans->wifi_hw->hw_cap & BIT(WIFI_HW_CAP_RRO)) {
		bus_write(bus_trans, rro_cfg->sign_base_0_addr, trans->wifi_hw->sign_base_cr);
		bus_read(bus_trans, rro_cfg->sign_base_1_addr);
		cr_value |= (1 << rro_cfg->sign_base_1_en_shift);
		bus_write(bus_trans, rro_cfg->sign_base_1_addr, cr_value);

		if (test_bit(PCI_FLAG_WHNAT_RRO_DBG, &ptrans->flags))
			/* Set all to packet to host */
			bus_write(bus_trans, rro_cfg->debug_mode_rxd_addr, 0xffffffff);
	}
}

static int
wed_init_after_fwdl(void *hw)
{
	struct wed_trans *trans = to_wed_trans(hw);
	int ret;

	ret = trans->dma_ops->init_after_fwdl(hw);
	if (ret)
		goto err;

	if (trans->enable) {
		wed_set_rro_sign_addr(hw);

		if (trans->wed_ver == 1) {
			ret = trans->dma_ops->set_free_notify_version(hw, FREE_NOTIFY_VERSION_0);
			if (ret)
				goto err;
		}

		warp_dma_handler_after_fwdl(hw, WARP_DMA_TXRX);
	}
err:
	return ret;
}

static void
wed_cmd_resp_handler(char *msg, u16 msg_len, void *user_data)
{
	mtk_dbg(MTK_BUS, "%s(): %d\n", __func__, __LINE__);
}

static int
_wed_tx_mcu_queue(void *hw, struct sk_buff *skb)
{
	struct wed_trans *trans = to_wed_trans(hw);
	struct warp_msg_cmd cmd = {0};
	struct wo_txd *wo_txd = (struct wo_txd *) skb->data;
	u32 wed_idx;
	void *msg;

	mtk_dbg(MTK_BUS, "%s(): cmd %d, len %d\n", __func__,
		wo_txd->cmd_id, wo_txd->len);

	if (!trans->enable)
		return 0;

	wed_idx = warp_get_wed_idx(hw);

	if (wed_idx == WED_IDX_ERR)
		return -EINVAL;

	msg = skb_pull(skb, sizeof(*wo_txd));
	cmd.param.cmd_id = wo_txd->cmd_id;
	cmd.param.to_id = MODULE_ID_WO;
	cmd.param.wait_type = WARP_MSG_WAIT_TYPE_RSP_STATUS;
	cmd.param.timeout = WARP_MSG_TIMEOUT_DEFAULT;
	cmd.param.rsp_hdlr = wed_cmd_resp_handler;
	cmd.param.user_data = (void *)&wo_txd->cmd_id;
	cmd.msg = msg;
	cmd.msg_len = wo_txd->len;

	mtk_dbg(MTK_BUS, "%s(): txd %p, txd_sz %ld, msg %p, cmd_id %d, len %d\n",
		__func__, wo_txd, sizeof(*wo_txd), msg, wo_txd->cmd_id, wo_txd->len);

	return warp_msg_send_cmd_handler(wed_idx, &cmd);
}

static int
wed_tx_mcu_queue(void *hw, struct sk_buff *skb, enum mtk_queue_attr q)
{
	struct wed_trans *trans = to_wed_trans(hw);

	if (q == Q_TX_CMD_WO)
		return _wed_tx_mcu_queue(hw, skb);
	else
		return trans->dma_ops->tx_mcu_queue(hw, skb, q);
}

static bool
wed_request_tx(void *hw, void *txq)
{
	struct wed_trans *trans = to_wed_trans(hw);

	return trans->dma_ops->request_tx(hw, txq);
}

static int
wed_tx_data_queue(void *hw, struct mtk_bus_tx_info *tx_info)
{
	int ret = 0;
	struct wed_trans *trans = to_wed_trans(tx_info->txq_trans);

	if (trans->enable && test_bit(BUS_TX_DATA, &tx_info->flags))
		ret = warp_wlan_tx((void *)trans, (u8 *)tx_info);

	if (!ret)
		ret = trans->dma_ops->tx_data_queue(hw, tx_info);

	return ret;
}

static int
wed_tx_kick(void *hw)
{
	struct wed_trans *trans = to_wed_trans(hw);

	return trans->dma_ops->tx_kick(hw);
}

static int
wed_chip_attached(void *hw, void *bus_ops)
{
	struct wed_trans *trans = to_wed_trans(hw);

	return trans->dma_ops->chip_attached(hw, bus_ops);
}

static bool
wed_match(void *master, void *slave)
{
	struct wed_trans *trans = to_wed_trans(master);

	return trans->dma_ops->match(master, slave);
}

static void
wed_tx_free(void *hw, struct sk_buff *skb)
{
	struct wed_trans *trans = to_wed_trans(hw);

	return trans->dma_ops->tx_free(hw, skb);
}

static void
wed_v1_compatible_mode_config(struct mtk_bus_trans *bus_trans, struct wifi_hw *wed_hw)
{
	bus_set_field(bus_trans, wed_hw->wed_v1_compatible_en_addr,
		wed_hw->wed_v1_compatible_en_msk, 0x1);
	bus_set_field(bus_trans, wed_hw->wed_v1_compatible_tx0_addr,
		wed_hw->wed_v1_compatible_tx0_msk, wed_hw->wed_v1_compatible_tx0_id);
	bus_set_field(bus_trans, wed_hw->wed_v1_compatible_tx1_addr,
		wed_hw->wed_v1_compatible_tx1_msk, wed_hw->wed_v1_compatible_tx1_id);
	bus_set_field(bus_trans, wed_hw->wed_v1_compatible_rx1_addr,
		wed_hw->wed_v1_compatible_rx1_msk, wed_hw->wed_v1_compatible_rx1_id);
}

static int
wed_start(void *hw)
{
	struct wed_trans *trans = to_wed_trans(hw);


	if (trans->enable) {
		if (trans->wed_ver == 1)
			wed_v1_compatible_mode_config(to_bus_trans(hw), trans->wifi_hw);
		warp_ring_init(hw);
		warp_dma_handler(hw, WARP_DMA_TXRX);
	}

	if (trans->dma_ops->start)
		return trans->dma_ops->start(hw);
	return 0;
}

static void
wed_stop(void *hw)
{
	struct wed_trans *trans = to_wed_trans(hw);
	struct mtk_hw_dev *dev = to_hw_dev(trans);

	if (trans->enable) {
		if (!test_bit(HWIFI_STATE_RESET, &dev->state)) {
			warp_dma_handler(hw, WARP_DMA_DISABLE);
			warp_ring_exit(hw);
		} else {
#if (WARP_VERSION(3, 0) <= WARP_VERSION_CODE)
			if (warp_reset_handler(hw, WIFI_ERR_RECOV_STOP_PDMA0))
				set_bit(HWIFI_STATE_RESET_FAILED, &dev->state);
#else
			warp_ser_handler(hw, WIFI_ERR_RECOV_STOP_PDMA0);
#endif
		}
	}

	if (trans->dma_ops->stop)
		trans->dma_ops->stop(hw);
}

static int
wed_start_traffic(void *hw)
{
	struct wed_trans *trans = to_wed_trans(hw);

	if (trans->enable)
		warp_reset_handler(hw, WIFI_ETH_ERR_START_WED_RX_TRAFFIC);

	return 0;
}

static int
wed_stop_traffic(void *hw)
{
	struct wed_trans *trans = to_wed_trans(hw);

	if (trans->enable)
		return warp_reset_handler(hw, WIFI_ETH_ERR_STOP_WED_RX_TRAFFIC);

	return 0;
}

static int
wed_get_txq(void *hw, u8 band_idx, u8 tid, void **txq, void **txq_trans)
{
	struct wed_trans *trans = to_wed_trans(hw);

	return trans->dma_ops->get_txq(hw, band_idx, tid, txq, txq_trans);
}

static int
wed_add_hw_rx(void *hw, struct mtk_bus_rx_info *rx_info)
{
	struct wed_trans *trans = to_wed_trans(hw);

	mtk_dbg(MTK_BUS, "%s(): hw %p, rx_info %p, entry: %d, csrn: %d\n", __func__,
		hw, rx_info, rx_info->ppe_entry, rx_info->csrn);

	if (trans && trans->enable)
		warp_wlan_rx(hw, (void *)rx_info);
	return 0;
}

/*dbugfs*/
static int
wed_queues_read(void *hw, struct seq_file *s)
{
	struct wed_trans *trans = to_wed_trans(hw);

	return trans->dma_ops->queues_read(hw, s);
}

static int
wed_get_rro_mode(void *hw)
{
	struct wed_trans *trans = to_wed_trans(hw);

	return trans->dma_ops->get_rro_mode(hw);
}

static int
wed_rro_write_dma_idx(void *hw, u32 value)
{
	return warp_rro_write_dma_idx(hw, value);
}


static void *
wed_page_hash_get(void *hw, dma_addr_t pa)
{
	return warp_page_hash_get(hw, pa);
}


static int
wed_switch_node(void *hw_master, void *hw_slave)
{
	struct wed_trans *trans = to_wed_trans(hw_master);

	return trans->dma_ops->switch_node(hw_master, hw_slave);
}

static bool wed_init_rro_addr_elem_by_seid(void *hw, u16 seid)
{
	struct wed_trans *trans = to_wed_trans(hw);

	return trans->dma_ops->init_rro_addr_elem_by_seid(hw, seid);
}
static int
wed_int_to_mcu(void *hw, u32 value)
{
	struct wed_trans *trans = to_wed_trans(hw);

	return trans->dma_ops->int_to_mcu(hw, value);
}

static int
wed_set_pao_sta_info(void *hw, u16 wcid,
		u8 max_amsdu_nums, u32 max_amsdu_len, int remove_vlan, int hdrt_mode)
{
	struct wed_trans *trans = to_wed_trans(hw);

	if (trans->enable)
		return warp_set_pao_sta_info(hw, wcid, max_amsdu_nums,
					max_amsdu_len, remove_vlan, hdrt_mode);
	return 0;
}

static int
wed_set_pn_check(void *hw, u32 se_id, bool enable)
{
	struct wed_trans *trans = to_wed_trans(hw);
	struct mtk_bus_trans *bus_trans = to_bus_trans(hw);
	struct mtk_hw_dev *hw_dev = bus_trans->dev;
	struct mtk_chip_drv *chip_drv = hw_dev->chip_drv;
	bool pn_chk_in_rro_chip = chip_drv->hw_caps->mac_cap & BIT(CAP_PN_CHK);

	if (trans->enable && !pn_chk_in_rro_chip)
		return warp_set_pn_check(hw, se_id, enable);
	return 0;
}

static int
wed_set_particular_to_host(void *hw, bool enable)
{
#if (WARP_VERSION(3, 1) <= WARP_VERSION_CODE)
	return warp_set_particular_to_host(hw, enable);
#else
	return 0;
#endif
}

static int
wed_get_device(void *hw, void **device)
{
	struct wed_trans *trans = to_wed_trans(hw);

	return trans->dma_ops->get_device(hw, device);
}

static int
wed_get_tx_token_num(void *hw, u16 tx_token_num[], u8 max_src_num)
{
	struct wed_trans *trans = to_wed_trans(hw);
	struct mtk_hw_dev *hw_dev = to_hw_dev(trans);
	u8 idx;

	if (trans->enable) {
		for (idx = 0; idx < max_src_num; idx++)
			tx_token_num[idx] = hw_dev->ext_tx_tk_nums_master;
		tx_token_num[1] = hw_dev->ext_tx_tk_nums_slave;
	} else {
		return trans->dma_ops->get_tx_token_num(hw, tx_token_num, max_src_num);
	}

	return 0;
}

static struct mtk_bus_dma_ops wed_dma_ops = {
	.alloc_resource = wed_alloc_resource,
	.free_resource = wed_free_resource,
	.preinit_device = wed_preinit_device,
	.init_device = wed_init_device,
	.exit_device = wed_exit_device,
	.init_after_fwdl = wed_init_after_fwdl,
	.tx_mcu_queue = wed_tx_mcu_queue,
	.request_tx = wed_request_tx,
	.tx_data_queue = wed_tx_data_queue,
	.tx_kick = wed_tx_kick,
	.chip_attached = wed_chip_attached,
	.match = wed_match,
	.tx_free = wed_tx_free,
	.start = wed_start,
	.stop = wed_stop,
	.start_traffic = wed_start_traffic,
	.stop_traffic = wed_stop_traffic,
	.get_txq = wed_get_txq,
	.queues_read = wed_queues_read,
	.add_hw_rx = wed_add_hw_rx,
	.int_to_mcu = wed_int_to_mcu,
	.irq_handler = wed_irq_handler,
	.get_rro_mode = wed_get_rro_mode,
	.rro_write_dma_idx = wed_rro_write_dma_idx,
	.rro_page_hash_get = wed_page_hash_get,
	.switch_node = wed_switch_node,
	.init_rro_addr_elem_by_seid = wed_init_rro_addr_elem_by_seid,
	.set_pao_sta_info = wed_set_pao_sta_info,
	.set_pn_check = wed_set_pn_check,
	.set_particular_to_host = wed_set_particular_to_host,
	.get_device = wed_get_device,
	.get_tx_token_num = wed_get_tx_token_num,
};

static int
wed_drv_register(void *drv, void *priv_drv)
{
	struct mtk_bus_driver *bus_drv = (struct mtk_bus_driver *)drv;
	struct mtk_chip *chip = container_of(priv_drv, struct mtk_chip, priv_drv);
	struct mtk_bus_cfg *bus_cfg = &chip->drv->bus_cfg;

	if (bus_cfg->ms_type == CHIP_TYPE_MASTER && warp_mcu_wo_support())
		chip->drv->hw_caps->fw_flags |= BIT(FW_FLAG_WO);

	if (bus_drv->sub_drv)
		return bus_drv->sub_drv->ops.drv_register(bus_drv->sub_drv, priv_drv);
	return 0;
}

static int
wed_drv_unregister(void *drv, void *priv_drv)
{
	struct mtk_bus_driver *bus_drv = (struct mtk_bus_driver *)drv;

	if (bus_drv->sub_drv)
		return bus_drv->sub_drv->ops.drv_unregister((void *)bus_drv->sub_drv, priv_drv);
	return 0;
}

static int
wed_drv_create_tunnel(void *drv, struct mtk_bus_trans *trans, u32 bus_type)
{
	struct wed_trans *wed_trans = (struct wed_trans *) trans;

	if (trans->io_ops) {
		wed_trans->io_ops = trans->io_ops;
		wed_trans->dma_ops = trans->dma_ops;
		wed_trans->bus_type = (bus_type == MTK_BUS_PCI) ?
				      BUS_TYPE_PCIE : BUS_TYPE_AXI;
		trans->dma_ops = &wed_dma_ops;
	} else {
		dev_err(to_device(trans), "%s(): Create tunnel failed\n",
			__func__);
		return -EINVAL;
	}

	return 0;
}

static int
wed_drv_profile_dump(struct seq_file *s, void *profile)
{
	return 0;
}

static struct mtk_bus_driver wed_bus = {
	.bus_type = MTK_BUS_WED,
	.bus_sub_type = MTK_BUS_NUM,
	.ctl_size = sizeof(struct wed_trans),
	.ops = {
		.drv_register = wed_drv_register,
		.drv_unregister = wed_drv_unregister,
		.drv_profile_dump = wed_drv_profile_dump,
		.drv_create_tunnel = wed_drv_create_tunnel,
	},
};

static int __init mtk_wed_init(void)
{
	return mtk_bus_driver_register(&wed_bus);
}

static void __exit mtk_wed_exit(void)
{
	mtk_bus_driver_unregister(&wed_bus);
}

module_init(mtk_wed_init);
module_exit(mtk_wed_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Mediatek WED BUS Driver");
