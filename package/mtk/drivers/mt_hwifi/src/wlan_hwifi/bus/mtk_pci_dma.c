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

#include <linux/interrupt.h>
#include <linux/version.h>
#include "mac_ops.h"
#include "bus.h"
#include "mtk_pci_dma.h"
#include "mtk_rro.h"


static u32
pci_io_read(void *hw, u32 offset)
{
	u32 val;

	val = readl(bus_to_regs(hw) + offset);

	return val;
}
static void
pci_io_write(void *hw, u32 offset, u32 val)
{
	writel(val, bus_to_regs(hw) + offset);
}

static void
pci_io_write_range (void *hw, u32 offset, void *buf, u32 size)
{
	memcpy_toio((void *)(bus_to_regs(hw) + offset), (void *) buf, size);
}

static void
pci_io_read_range (void *hw, u32 offset, void *buf, u32 size)
{
	memcpy_fromio((void *) buf, (void *)(bus_to_regs(hw) + offset), size);
}

inline void
fill_rxd(struct pci_dma_cb *cb, u32 id, u32 magic_cnt)
{
	struct pci_dma_desc *desc = cb->alloc_va;

	WRITE_ONCE(desc->dw0,
		cpu_to_le32(SET_FIELD(RXDMAD_SDP0, cb->dma_buf.alloc_pa)));

	WRITE_ONCE(desc->dw2,
#ifdef CONFIG_HWIFI_64BIT_SUPPORT
		cpu_to_le32(SET_FIELD(RXDMAD_SDP0_H,
			cb->dma_buf.alloc_pa >> DMA_ADDR_H_SHIFT)) |
#endif
		cpu_to_le32(SET_FIELD(RXDMAD_TOKEN_ID, id)));

	WRITE_ONCE(desc->dw1,
		cpu_to_le32(SET_FIELD(RXDMAD_SDL0, cb->dma_buf.alloc_size)) |
		cpu_to_le32(BIT(RXDMAD_TO_HOST_SHIFT)));

	WRITE_ONCE(desc->dw3,
		cpu_to_le32(SET_FIELD(RXDMAD_MAGIC_CNT, magic_cnt)));
}

static void
pci_dma_set_irq_mask(struct pci_irq_vector_data *vec_data, u32 addr,
		       u32 clear, u32 set)
{
	struct pci_trans *trans = to_pci_trans(vec_data->bus_trans);
	unsigned long flags;

	spin_lock_irqsave(&trans->irq_lock, flags);
	vec_data->int_ena_mask &= ~clear;
	vec_data->int_ena_mask |= set;
	trans->int_mask_map[vec_data->isr_map_idx].int_ena_mask &= ~clear;
	trans->int_mask_map[vec_data->isr_map_idx].int_ena_mask |= set;
	bus_write(to_bus_trans(trans), addr,
			trans->int_mask_map[vec_data->isr_map_idx].int_ena_mask);
	vec_data->int_dis_mask |= clear;
	vec_data->int_dis_mask &= ~set;
	trans->int_mask_map[vec_data->isr_map_idx].int_dis_mask |= clear;
	trans->int_mask_map[vec_data->isr_map_idx].int_dis_mask &= ~set;
	spin_unlock_irqrestore(&trans->irq_lock, flags);
}

static void
pci_dma_irq_ena_set(struct pci_trans *trans, u8 map_idx, u32 set)
{
	bus_write(to_bus_trans(trans),
			trans->int_mask_map[map_idx].int_ena_set_addr, set);
}

static void
pci_dma_irq_ena_clr(struct pci_trans *trans, u8 map_idx, u32 clr)
{
	bus_write(to_bus_trans(trans),
			trans->int_mask_map[map_idx].int_ena_clr_addr, clr);
}

static void
pci_dma_irq_enable(struct pci_irq_vector_data *vec_data, u32 mask)
{
	struct pci_trans *trans = to_pci_trans(vec_data->bus_trans);
	u8 map_idx = vec_data->isr_map_idx;

	if (map_idx)
		pci_dma_irq_ena_set(trans, map_idx, mask);
	else
		pci_dma_set_irq_mask(vec_data,
				trans->int_mask_map[map_idx].int_ena_addr, 0, mask);
}

static void
pci_dma_irq_disable(struct pci_irq_vector_data *vec_data, u32 mask)
{
	struct pci_trans *trans = to_pci_trans(vec_data->bus_trans);
	u8 map_idx = vec_data->isr_map_idx;

	if (map_idx)
		pci_dma_irq_ena_clr(trans, map_idx, mask);
	else
		pci_dma_set_irq_mask(vec_data,
			trans->int_mask_map[map_idx].int_ena_addr, mask, 0);
}

static inline s8
pci_dma_get_txq_by_attr(struct pci_trans *trans, u32 attr, u8 band_idx, u8 qid)
{
	u8 i;

	for (i = 0 ; i < trans->txq_num; i++) {
		if (trans->txq[i].q.q_attr == attr)
			break;
	}

	return i == trans->txq_num ? -1 : i;
}

static int
pci_dma_set_queue_common(struct pci_queue *q, const struct pci_queue_desc *cmm)
{
	q->hw_desc_base = cmm->hw_desc_base;
	q->hw_int_mask = cmm->hw_int_mask;
	q->hw_didx_mask = cmm->hw_didx_mask;
	q->hw_magic_ena_mask = cmm->hw_magic_ena_mask;
	q->hw_attr_ena_mask = cmm->hw_attr_ena_mask;
	q->q_attr = cmm->q_attr;
	q->q_size = cmm->q_size;
	q->band_idx_bmp = cmm->band_idx_bmp;
	q->isr_map_idx = cmm->isr_map_idx;
	q->emi_ring_idx = cmm->emi_ring_idx;
	q->q_info = cmm->q_info;
	q->desc_size = cmm->desc_size;
	q->disable = cmm->disable;
	return 0;
}

static int
pci_dma_set_tx_queue_desc(struct pci_tx_queue *tq, const struct pci_tx_queue_desc *desc)
{
	struct pci_queue_mp_attr *q_mp_attr = &tq->q_mp_attr;

	q_mp_attr->ring_no = desc->mp_attr.ring_no;
	q_mp_attr->handled_by = desc->mp_attr.handled_by;
	q_mp_attr->handled_by_wed_mode = desc->mp_attr.handled_by_wed_mode;

	return 0;
}

static int
pci_dma_tx_alloc_cb(struct pci_trans *trans, struct pci_tx_queue *txq, u8 txn)
{
	u8 idx;
	struct pci_tx_queue *tq;
	struct pci_queue *q;
	struct pci_dma_buf *desc;

	for (idx = 0 ; idx < txn; idx++) {
		tq = &txq[idx];
		q = (struct pci_queue *) tq;
		desc = &q->desc_ring;
		desc->alloc_size = q->q_size * q->desc_size;
		desc->alloc_va = dma_alloc_coherent(to_device(trans),
			desc->alloc_size, &desc->alloc_pa, GFP_KERNEL);
		if (!desc->alloc_va)
			goto err;

		q->cb = kcalloc(q->q_size, sizeof(struct pci_dma_cb), GFP_KERNEL);
		if (!q->cb)
			goto err;

	}
	return 0;
err:
	return -ENOMEM;
}

static void
pci_dma_tx_free_cb(struct pci_trans *trans, struct pci_tx_queue *txq, u8 txn)
{
	u8 idx;
	struct pci_tx_queue *tq;
	struct pci_queue *q;
	struct pci_dma_buf *desc;

	for (idx = 0 ; idx < txn; idx++) {
		tq = &txq[idx];
		q = (struct pci_queue *) tq;
		desc = &q->desc_ring;

		kfree(q->cb);

		if (desc->alloc_va)
			dma_free_coherent(to_device(trans), desc->alloc_size,
				desc->alloc_va, desc->alloc_pa);
	}
}

static int
pci_dma_alloc_tx_queue(struct pci_trans *trans, u8 txn,
	const struct pci_tx_queue_desc *txd)
{
	u8 idx;
	struct pci_tx_queue *txq = kzalloc(txn * sizeof(*txq), GFP_KERNEL);

	if (!txq)
		goto err;

	for (idx = 0 ; idx < txn; idx++) {
		const struct pci_tx_queue_desc *desc = &txd[idx];
		struct pci_tx_queue *tq = &txq[idx];
		struct pci_queue *q = (struct pci_queue *) tq;

		spin_lock_init(&q->lock);
		spin_lock_init(&tq->done_lock);

		pci_dma_set_queue_common(q, &desc->cmm);
		pci_dma_set_tx_queue_desc(tq, desc);
	}
	trans->txq = txq;
	return 0;
err:
	return -ENOMEM;
}

static void
pci_dma_free_tx_queue(struct pci_trans *trans)
{
	struct pci_tx_queue *txq = trans->txq;

	if (txq) {
		trans->txq = NULL;
		kfree(txq);
	}
}

static int
pci_dma_set_rx_queue_desc(struct pci_rx_queue *rq, const struct pci_rx_queue_desc *desc)
{
	struct pci_queue_mp_attr *q_mp_attr = &rq->q_mp_attr;

	rq->rx_buf_size = desc->rx_buf_size;
	rq->free_done_handled_by = desc->free_done_handled_by;
	rq->txfreedone_path = desc->txfreedone_path;
	rq->magic_enable = desc->magic_enable;
	rq->attr_enable = desc->attr_enable;

	q_mp_attr->ring_no = desc->mp_attr.ring_no;
	q_mp_attr->handled_by = desc->mp_attr.handled_by;
	q_mp_attr->handled_by_wed_mode = desc->mp_attr.handled_by_wed_mode;

#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
	rq->rx_offload.kfifo_size = desc->rx_offload.kfifo_size;
	rq->rx_offload.kfifo_enable = desc->rx_offload.kfifo_enable;
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */

	return 0;
}

static int
pci_dma_set_rx_queue_ops(struct pci_rx_queue *rq, unsigned long *flags, u8 dma_size)
{
	rq->gfp_flags = GFP_ATOMIC | GFP_DMA32;

	switch (rq->q.q_attr) {
	case Q_RX_DATA_MSDU_PG:
		rq->q_ops = &data_rxq_ops;
		rq->get_pkt_method = GET_PKT_IO;
		rq->buf_flags = BUF_DEFAULT;
		rq->buf_debug = 0;
		rq->need_init_after_fwdl = 1;
		if (dma_size > 32)
			rq->gfp_flags = GFP_ATOMIC;
		break;
	case Q_RX_DATA:
	case Q_RX_DATA_WED:
		rq->q_ops = test_bit(RQ_FLAG_RX_DATA_COPY, &rq->rq_flags) ?
				&data_cp_rxq_ops : &data_rxq_ops;
		rq->get_pkt_method = GET_PKT_DDONE;
		rq->buf_flags = BUF_DEFAULT;
		rq->buf_debug = 0;
		if (dma_size > 32)
			rq->gfp_flags = GFP_ATOMIC;
		break;
	case Q_RX_DATA_RRO:
		rq->q_ops = &rro_rxq_ops;
		set_bit(RQ_FLAG_RX_DATA_TKID, &rq->rq_flags);
		rq->get_pkt_method = GET_PKT_IO;
		rq->buf_flags = BUF_DEFAULT;
		rq->buf_debug = 0;
		if (dma_size > 32)
			rq->gfp_flags = GFP_ATOMIC;
		break;
	case Q_RX_IND:
		rq->q_ops = &ind_cmd_rxq_ops;
		rq->get_pkt_method = GET_PKT_DDONE;
		rq->buf_flags = BUF_DEFAULT;
		rq->buf_debug = 0;
		rq->need_init_after_fwdl = 1;
		break;
	case Q_RX_RXDMAD_C:
		rq->q_ops = &rxdmad_c_rxq_ops;
		rq->get_pkt_method = GET_PKT_DDONE;
		rq->buf_flags = BUF_DEFAULT;
		rq->buf_debug = 0;
		rq->need_init_after_fwdl = 1;
		break;
	case Q_RX_EVENT_WM:
	case Q_RX_EVENT_WA:
	case Q_RX_EVENT_TX_FREE_DONE:
	case Q_RX_EVENT_SDO:
		rq->q_ops = &evt_rxq_ops;
		rq->get_pkt_method = GET_PKT_DDONE;
		rq->buf_flags = BUF_DEFAULT;
		rq->buf_debug = 0;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int
pci_dma_set_queue_ops(void *hw)
{
	struct pci_trans *trans = to_pci_trans(hw);
	u8 idx, rxn = trans->rxq_num;
	int ret = 0;

	for (idx = 0; idx < rxn; idx++) {
		struct pci_rx_queue *rq = &trans->rxq[idx];

		ret = pci_dma_set_rx_queue_ops(rq, &trans->flags,
			trans->non_coherent_dma_addr_size);
	}

	return ret;
}

static int
pci_dma_set_free_notify_ver(void *hw, u8 version)
{
	struct pci_trans *trans = to_pci_trans(hw);

	pci_chip_set_free_notify_ver(trans, version);
	return 0;
}

static int
pci_dma_rx_alloc_cb(struct pci_trans *trans, struct pci_rx_queue *rxq, u8 rxn)
{
	u8 idx;
	struct pci_rx_queue *rq;
	struct pci_queue *q;
	struct pci_dma_buf *desc;

	for (idx = 0 ; idx < rxn; idx++) {
		rq = &rxq[idx];
		q = &rq->q;
		desc = &q->desc_ring;
		desc->alloc_size = q->q_size * q->desc_size;
		desc->alloc_va = dma_alloc_coherent(to_device(trans),
			desc->alloc_size, &desc->alloc_pa, GFP_KERNEL);
		if (!desc->alloc_va)
			goto err;

		q->cb = kcalloc(q->q_size, sizeof(struct pci_dma_cb), GFP_KERNEL);
		if (!q->cb)
			goto err;
	}
	return 0;
err:
	return -ENOMEM;
}

static void
pci_dma_rx_free_cb(struct pci_trans *trans, struct pci_rx_queue *rxq, u8 rxn)
{
	u8 idx;
	struct pci_rx_queue *rq;
	struct pci_queue *q;
	struct pci_dma_buf *desc;

	for (idx = 0 ; idx < rxn; idx++) {
		rq = &rxq[idx];
		q = &rq->q;
		desc = &q->desc_ring;

		kfree(q->cb);

		if (desc->alloc_va)
			dma_free_coherent(to_device(trans), desc->alloc_size,
				desc->alloc_va, desc->alloc_pa);
	}
}

static int
pci_dma_alloc_rx_queue(struct pci_trans *trans,
	u8 rn, const struct pci_rx_queue_desc *rxd)
{
	u8 idx;
	struct pci_rx_queue *rxq = kzalloc(rn * sizeof(*rxq), GFP_KERNEL);

	if (!rxq)
		goto err;

	for (idx = 0 ; idx < rn; idx++) {
		struct pci_rx_queue *rq = &rxq[idx];
		struct pci_queue *q = &rq->q;
		const struct pci_rx_queue_desc *desc = &rxd[idx];

		spin_lock_init(&q->lock);
		pci_dma_set_queue_common(q, &desc->cmm);
		pci_dma_set_rx_queue_desc(rq, desc);
		pci_dma_set_rx_queue_ops(rq, &trans->flags, trans->non_coherent_dma_addr_size);
	}

	trans->rxq = rxq;
	return 0;
err:
	return -ENOMEM;
}

static void
pci_dma_free_rx_queue(struct pci_trans *trans)
{
	struct pci_rx_queue *rxq = trans->rxq;

	if (rxq) {
		trans->rxq = NULL;
		kfree(rxq);
	}
}

static int
pci_dma_alloc_queue(struct pci_trans *trans, struct pci_queue_layout *layout)
{
	int ret;
	u8 txn = trans->txq_num;
	u8 rxn = trans->rxq_num;
	/*tx related*/
	ret = pci_dma_alloc_tx_queue(trans, txn, layout->tx_queue_layout);
	if (ret)
		goto err;

	ret = pci_dma_tx_alloc_cb(trans, trans->txq, txn);
	if (ret)
		goto err;

	/*rx related*/
	ret = pci_dma_alloc_rx_queue(trans, rxn, layout->rx_queue_layout);
	if (ret)
		goto err;

	ret = pci_dma_rx_alloc_cb(trans, trans->rxq, rxn);
	if (ret)
		goto err;

	return 0;
err:
	return ret;
}

static int
pci_dma_alloc_resource(void *hw, void *profile)
{
	struct pci_chip_profile *p = profile;
	struct pci_trans *trans = to_pci_trans(hw);
	int ni = 0;
	int ret;

	for (; ni < MAX_MAP_IDX; ni++) {
		trans->int_mask_map[ni].int_sts_addr = p->int_mask_map[ni].int_sts_addr;
		trans->int_mask_map[ni].int_ena_addr = p->int_mask_map[ni].int_ena_addr;
		trans->int_mask_map[ni].int_ena_set_addr = p->int_mask_map[ni].int_ena_set_addr;
		trans->int_mask_map[ni].int_ena_clr_addr = p->int_mask_map[ni].int_ena_clr_addr;
		trans->int_mask_map[ni].int_ena_mask = p->int_mask_map[ni].int_ena_mask;
		trans->int_mask_map[ni].int_dis_mask = 0;
	}

	trans->vec_data[0].bus_trans = to_bus_trans(hw);
	trans->vec_data[0].int_ena_mask = p->int_mask_map[0].int_ena_mask;
	trans->vec_data[0].int_dis_mask = 0;

	trans->lp_fw_own_reg_addr = p->lp_fw_own_reg_addr;
	trans->tx_dma_glo_reg_addr = p->tx_dma_glo_reg_addr;
	trans->rx_dma_glo_reg_addr = p->rx_dma_glo_reg_addr;
	trans->int_ser_reg_addr = p->int_ser_reg_addr;
	trans->int_ser_mask = p->int_ser_mask;
	trans->pause_trxq_reg_addr = p->pause_trxq_reg_addr;
	trans->pause_trxq_mask = p->pause_trxq_mask;
	trans->txq_num = p->txq_num;
	trans->rxq_num = p->rxq_num;
	trans->int_enable_swi_mask = p->int_enable_swi_mask;
	trans->mcu2host_sw_int_isr_map_idx = p->mcu2host_sw_int_isr_map_idx;
	trans->mcu2host_sw_int_ena_addr = p->mcu2host_sw_int_ena_addr;
	trans->mcu2host_sw_int_sta_addr = p->mcu2host_sw_int_sta_addr;
	trans->mcu2host_sw_int_ser_mask = p->mcu2host_sw_int_ser_mask;
	trans->mcu2host_sw_wdt_ser_mask = p->mcu2host_sw_wdt_ser_mask;
	trans->coherent_dma_addr_size = p->coherent_dma_addr_size;
	trans->non_coherent_dma_addr_size = p->non_coherent_dma_addr_size;
	spin_lock_init(&trans->irq_lock);
	ret = pci_dma_alloc_queue(trans, &p->queue_layout);
	if (ret)
		goto err;

	ret = rro_alloc_resource(trans, &p->rro_profile);
	if (ret)
		goto err;

	return 0;
err:
	return ret;
}

static void
pci_dma_free_queue(struct pci_trans *trans)
{
	u8 txn = trans->txq_num;
	u8 rxn = trans->rxq_num;

	/*tx related*/
	pci_dma_tx_free_cb(trans, trans->txq, txn);
	pci_dma_free_tx_queue(trans);

	/*rx related*/
	pci_dma_rx_free_cb(trans, trans->rxq, rxn);
	pci_dma_free_rx_queue(trans);
}

static void
pci_dma_free_resource(void *hw)
{
	struct pci_trans *trans = to_pci_trans(hw);

	pci_dma_free_queue(trans);
	rro_free_resource(trans);
}

static void
pci_dma_init_tx_data_queue(
	struct pci_trans *trans, struct pci_tx_queue *tq)
{
	struct pci_queue *q = (struct pci_queue *) tq;
	struct pci_dma_buf *desc = &q->desc_ring;
	void *desc_va;
	struct pci_dma_cb *cb;
	u16 q_sz = q->q_size;
	dma_addr_t desc_pa;
	int i;

	q->band_num_sup = hweight8(q->band_idx_bmp);
	memset(desc->alloc_va, 0, desc->alloc_size);
	desc_va = desc->alloc_va;
	desc_pa = desc->alloc_pa;

	for (i = 0 ; i < q_sz; i++) {
		cb = &q->cb[i];
		cb->pkt = NULL;
		cb->next_pkt = NULL;
		cb->alloc_size = q->desc_size;
		cb->alloc_va = desc_va;
		cb->alloc_pa = desc_pa;

		desc_va = (unsigned char *) desc_va + q->desc_size;
		desc_pa += q->desc_size;
	}
	q->head = 0;
	q->tail = 0;
}

inline size_t
rx_pkt_total_size(size_t dma_buf_size)
{
	size_t total_size = 0;

	total_size = SKB_DATA_ALIGN(SKB_HEADROOM_RSV + dma_buf_size) +
				SKB_DATA_ALIGN(sizeof(struct skb_shared_info));

	return total_size;
}

void *
pci_dma_rx_pkt_alloc(struct device *dev,
	struct pci_rx_queue *rq, struct pci_dma_buf *buf, bool map)
{
	void *pkt = NULL;
	dma_addr_t addr;

#if (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
	pkt = page_frag_alloc(&rq->rx_page,
			rx_pkt_total_size(buf->alloc_size),
			rq->gfp_flags);
#else
	pkt = __alloc_page_frag(&rq->rx_page,
			rx_pkt_total_size(buf->alloc_size),
			rq->gfp_flags);
#endif

	if (!pkt) {
		dev_err(dev, "%s: frag alloc fail\n", __func__);
		return NULL;
	}

	if (rq->buf_flags & BUF_ZERO)
		memset((void *)((u8 *)pkt + SKB_HEADROOM_RSV), 0, buf->alloc_size);

	memset((void *)(u8 *)pkt, 0, SKB_HEADROOM_RSV);

	buf->alloc_va = (void *)((u8 *)pkt + SKB_HEADROOM_RSV);

	if (map) {
		addr = dma_map_single(dev,
			buf->alloc_va, buf->alloc_size, DMA_FROM_DEVICE);

		if (unlikely(dma_mapping_error(dev, addr))) {
			put_page(virt_to_head_page(pkt));
			dev_err(dev, "%s: dma mapping err\n", __func__);
			return NULL;
		}

		buf->alloc_pa = addr;
	} else {
		buf->alloc_pa = 0;
	}

	mtk_dbg(MTK_BUS, "%s: alloc_size = %zu, rx_pkt_total_size = %zu\n",
		__func__, buf->alloc_size, rx_pkt_total_size(buf->alloc_size));
	mtk_dbg(MTK_BUS, "%s: pkt: %p\n", __func__, pkt);
	mtk_dbg(MTK_BUS, "%s: dma va: %p\n", __func__, buf->alloc_va);
	mtk_dbg(MTK_BUS, "%s: dma pa: %pad\n", __func__, &buf->alloc_pa);

	return pkt;
}

static void
pci_dma_init_rx_event_queue(
	struct pci_trans *trans,
	struct pci_rx_queue *rq)
{
	struct pci_queue *q = &rq->q;
	struct pci_dma_buf *desc = &q->desc_ring;
	u8 *desc_va;
	struct pci_dma_cb *cb;
	struct pci_dma_buf *buf;
	u16 q_sz = q->q_size;
	dma_addr_t desc_pa;
	int i;

	rq->magic_cnt = 0;
	desc_va = desc->alloc_va;
	desc_pa = desc->alloc_pa;
	for (i = 0 ; i < q_sz; i++) {
		cb = &q->cb[i];
		cb->pkt = NULL;
		cb->next_pkt = NULL;
		cb->alloc_size = q->desc_size;
		cb->alloc_va = desc_va;
		cb->alloc_pa = desc_pa;

		clear_ddone(cb->alloc_va);
		buf = &cb->dma_buf;
		buf->alloc_size = rq->rx_buf_size;
		cb->pkt = pci_dma_rx_pkt_alloc(to_device(trans), rq, buf, true);

		/*write rxd*/
		fill_rxd(cb, 0, 0);

		desc_va = (unsigned char *) desc_va + q->desc_size;
		desc_pa += q->desc_size;
	}
	q->head = q->q_size - 1;
	q->tail = q->q_size;
}

#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
void
pci_dma_rx_process_work(struct work_struct *w)
{
	struct delayed_work *d = container_of(w, struct delayed_work, work);
	struct pci_rx_queue *rq =
				container_of(d, struct pci_rx_queue, bus_rx_process_work);
	struct mtk_rx_process rx_process = {0};
	int len = 0;

	len = kfifo_len(&rq->bus_rx_process_fifo);
	while (len > 0) {
		if (kfifo_get(&rq->bus_rx_process_fifo, &rx_process)) {
			mtk_bus_rx_process(rx_process.trans, rx_process.q_attr, rx_process.skb);
			len--;
		} else
			break;
	}
}

int
pci_dma_init_rx_process_work(struct pci_rx_queue *rq)
{
	int ret = 0,  rx_data_fifo_size = 0, last_online_cpu = 0;

	mt_get_online_cpus();
	last_online_cpu = cpumask_last(cpu_online_mask);
	mt_put_online_cpus();
	rq->work_on_cpu = last_online_cpu;
	rx_data_fifo_size = roundup_pow_of_two(rq->rx_offload.kfifo_size);
	INIT_DELAYED_WORK(&rq->bus_rx_process_work, pci_dma_rx_process_work);
	rq->bus_rx_process_wq =
		mt_alloc_workqueue("bus_rx_process_wq", WQ_HIGHPRI | __WQ_ORDERED, 0);
	ret = kfifo_alloc(&rq->bus_rx_process_fifo, rx_data_fifo_size, GFP_KERNEL);
	mtk_dbg(MTK_BUS, "rx fifo size=%d, work_on_cpu=%d\n", rx_data_fifo_size, last_online_cpu);
	return ret;
}

void
pci_dma_exit_rx_process_work(struct pci_rx_queue *rq)
{
	flush_delayed_work(&rq->bus_rx_process_work);
	cancel_delayed_work_sync(&rq->bus_rx_process_work);
	mt_destroy_workqueue(rq->bus_rx_process_wq);
	kfifo_free(&rq->bus_rx_process_fifo);
}
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */

void
pci_dma_init_rx_data_queue(
	struct pci_trans *trans,
	struct pci_rx_queue *rq)
{
	struct pci_queue *q = &rq->q;
	struct pci_dma_buf *desc = &q->desc_ring;
	u8 *desc_va;
	struct pci_dma_cb *cb;
	struct pci_dma_buf *buf;
	u16 q_sz = q->q_size;
	dma_addr_t desc_pa;
	int i;
	u32 id = 0;

	rq->magic_cnt = 0;
	desc_va = desc->alloc_va;
	desc_pa = desc->alloc_pa;
	for (i = 0 ; i < q_sz; i++) {
		cb = &q->cb[i];
		cb->pkt = NULL;
		cb->next_pkt = NULL;
		cb->alloc_size = q->desc_size;
		cb->alloc_va = desc_va;
		cb->alloc_pa = desc_pa;

		if ((test_bit(PCI_FLAG_WHNAT_RX, &trans->flags) &&
		    (q->q_attr == Q_RX_DATA_RRO ||  q->q_attr == Q_RX_DATA_MSDU_PG)))
			goto update_desc;

		clear_ddone(cb->alloc_va);
		buf = &cb->dma_buf;
		buf->alloc_size = rq->rx_buf_size;
		cb->pkt = pci_dma_rx_pkt_alloc(to_device(trans), rq, buf, true);

		if (rq->q.q_attr == Q_RX_DATA_MSDU_PG) {
			rro_msdu_page_hash_init(trans, cb->pkt, buf->alloc_va,
				buf->alloc_pa);
		} else if (test_bit(RQ_FLAG_RX_DATA_TKID, &rq->rq_flags)) {
			id = mtk_rx_tk_request_entry(to_rx_tk_mgmt(trans),
				cb->pkt, buf->alloc_size, buf->alloc_va,
				buf->alloc_pa);
		}

		/*write rxd*/
		fill_rxd(cb, id, rq->magic_cnt);
update_desc:
		desc_va = (unsigned char *) desc_va + q->desc_size;
		desc_pa += q->desc_size;
	}
	if (rq->q.q_attr == Q_RX_DATA_RRO)
		rq->tail_sid = id;
	q->head = q->q_size - 1;
	q->tail = q->q_size;

#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
	if (rq->rx_offload.kfifo_enable)
		pci_dma_init_rx_process_work(rq);
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */
}

#if (KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE)
void __page_frag_drain(struct page *page, unsigned int order,
						unsigned int count)
{
#if (KERNEL_VERSION(4, 7, 0) > LINUX_VERSION_CODE)
	atomic_sub(count - 1, &page->_count);
#else
	atomic_sub(count - 1, &page->_refcount);
#endif
	__free_pages(page, order);
}
#endif

static void
pci_rx_queue_page_drain(struct pci_rx_queue *rq)
{
	if (rq->rx_page.va) {
		struct page *page;

		page = virt_to_page(rq->rx_page.va);
#if (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
		__page_frag_cache_drain(page, rq->rx_page.pagecnt_bias);
#elif (KERNEL_VERSION(4, 3, 0) < LINUX_VERSION_CODE)
		__page_frag_drain(page, compound_order(page),
				rq->rx_page.pagecnt_bias);
#endif
	}

	memset(&rq->rx_page, 0x00, sizeof(rq->rx_page));
}

static void
pci_dma_exit_rx_event_queue(
	struct pci_trans *trans,
	struct pci_rx_queue *rq)
{
	struct pci_dma_cb *cb;
	struct pci_queue *q = &rq->q;
	u16 q_sz = q->q_size;
	int i;

	for (i = 0 ; i < q_sz; i++) {
		cb = &q->cb[i];
		if (cb->pkt) {
			dma_unmap_single(to_device(trans),
			cb->dma_buf.alloc_pa,
			cb->dma_buf.alloc_size, DMA_FROM_DEVICE);
			put_page(virt_to_head_page(cb->pkt));
			cb->pkt = NULL;
		}
	}

	pci_rx_queue_page_drain(rq);
}

void
pci_dma_exit_rx_data_queue(
	struct pci_trans *trans,
	struct pci_rx_queue *rq)
{
	struct pci_dma_cb *cb;
	struct pci_queue *q = &rq->q;
	u16 q_sz = q->q_size;
	struct pci_dma_desc *desc;
	u32 dw2;
	int i;
	u32 id;

#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
	if (rq->rx_offload.kfifo_enable)
		pci_dma_exit_rx_process_work(rq);
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */

	for (i = 0 ; i < q_sz; i++) {
		cb = &q->cb[i];
		if (cb->pkt) {
			desc = cb->alloc_va;
			dw2 = le32_to_cpu(READ_ONCE(desc->dw2));
			id = GET_FIELD(RXDMAD_TOKEN_ID, dw2);

			if (q->q_attr == Q_RX_DATA &&
			    !test_bit(RQ_FLAG_RX_DATA_TKID, &rq->rq_flags)) {
				dma_unmap_single(to_device(trans), cb->dma_buf.alloc_pa,
						cb->dma_buf.alloc_size, DMA_FROM_DEVICE);
				put_page(virt_to_head_page(cb->pkt));
			}

			cb->pkt = NULL;
		}
	}

	pci_rx_queue_page_drain(rq);
}

static void
pci_dma_hw_init_tx_queue(struct pci_trans *trans, struct pci_tx_queue *tq)
{
}

static void
pci_dma_hw_init_rx_queue(struct pci_trans *trans, struct pci_rx_queue *rq)
{
	struct pci_queue *q = &rq->q;

	if (rq->magic_enable)
		bus_set(to_bus_trans(trans), q->hw_cnt_addr,
			q->hw_magic_ena_mask);

	if (rq->attr_enable && !test_bit(PCI_FLAG_WHNAT_RX, &trans->flags))
		bus_set(to_bus_trans(trans), q->hw_cnt_addr,
			q->hw_attr_ena_mask);
}


static void
pci_dma_hw_init_queue_cmm(struct pci_trans *trans, struct pci_queue *q)
{
	q->hw_cnt_addr = q->hw_desc_base + 0x4;
	q->hw_cidx_addr = q->hw_desc_base + 0x8;
	q->hw_didx_addr = q->hw_desc_base + 0xC;

	bus_write(to_bus_trans(trans), q->hw_desc_base,
		SET_FIELD(BASE_PTR, q->cb[0].alloc_pa));

	bus_write(to_bus_trans(trans), q->hw_cnt_addr,
#ifdef CONFIG_HWIFI_64BIT_SUPPORT
		SET_FIELD(BASE_PTR_EXT, q->cb[0].alloc_pa >> DMA_ADDR_H_SHIFT) |
#endif
		q->q_size);

	if (!test_bit(PCI_FLAG_WHNAT_RX, &trans->flags) ||
	    (q->q_attr != Q_RX_DATA_RRO && q->q_attr != Q_RX_DATA_MSDU_PG)) {
		if (test_bit(Q_FLAG_WRITE_EMI, &q->q_flags))
			*q->emi_cidx_addr = cpu_to_le16(q->head);
		else
			bus_write(to_bus_trans(trans), q->hw_cidx_addr, q->head);

		bus_write(to_bus_trans(trans), q->hw_didx_addr, 0);
	}
}

static void
pci_dma_hw_exit_queue_cmm(struct pci_trans *trans, struct pci_queue *q)
{
	bus_write(to_bus_trans(trans), q->hw_desc_base, 0);
	bus_write(to_bus_trans(trans), q->hw_cnt_addr, q->q_size);

	if (test_bit(Q_FLAG_WRITE_EMI, &q->q_flags))
		*q->emi_cidx_addr = cpu_to_le16(0);
	else
		bus_write(to_bus_trans(trans), q->hw_cidx_addr, 0);

	bus_write(to_bus_trans(trans), q->hw_didx_addr, 0);
}

/**
 * @brief initial hw dma queues configure CR
 *
 * @param *trans initial this trans dma queues setting
 */
static void
pci_dma_hw_init_queues(struct pci_trans *trans)
{
	int i;
	struct pci_tx_queue *txq;
	struct pci_rx_queue *rxq;

	for (i = 0 ; i < trans->txq_num; i++) {
		txq = &trans->txq[i];

		if (txq->q.disable)
			continue;

		pci_dma_hw_init_queue_cmm(trans, &txq->q);
		pci_dma_hw_init_tx_queue(trans, txq);
	}

	for (i = 0 ; i < trans->rxq_num; i++) {
		rxq = &trans->rxq[i];

		if (rxq->q.disable || rxq->need_init_after_fwdl)
			continue;

		pci_dma_hw_init_queue_cmm(trans, &rxq->q);
		pci_dma_hw_init_rx_queue(trans, rxq);
	}
}

/**
 * @brief initial hw dma queues configure CR
 *
 * @param *trans initial this trans dma queues setting
 */
static void
pci_dma_hw_init_queues_after_fwdl(struct pci_trans *trans)
{
	struct pci_rx_queue *rxq;
	int i;

	for (i = 0 ; i < trans->rxq_num; i++) {
		rxq = &trans->rxq[i];

		if (rxq->q.disable || !rxq->need_init_after_fwdl)
			continue;

		pci_dma_hw_init_queue_cmm(trans, &rxq->q);
		pci_dma_hw_init_rx_queue(trans, rxq);
		set_bit(RQ_FLAG_INIT_AFTER_FWDL_CPLT, &rxq->rq_flags);
	}
}

static void
pci_dma_hw_exit_queues(struct pci_trans *trans)
{
	int i;
	struct pci_tx_queue *txq;
	struct pci_rx_queue *rxq;

	for (i = 0 ; i < trans->txq_num; i++) {
		txq = &trans->txq[i];

		if (txq->q.disable)
			continue;

		pci_dma_hw_exit_queue_cmm(trans, &txq->q);
	}

	for (i = 0 ; i < trans->rxq_num; i++) {
		rxq = &trans->rxq[i];

		if (rxq->q.disable)
			continue;

		if (rxq->need_init_after_fwdl) {
			if (!test_bit(RQ_FLAG_INIT_AFTER_FWDL_CPLT, &rxq->rq_flags))
				continue;

			clear_bit(RQ_FLAG_INIT_AFTER_FWDL_CPLT, &rxq->rq_flags);
		}

		pci_dma_hw_exit_queue_cmm(trans, &rxq->q);
	}
}


static irqreturn_t
pci_irq_handler(int irq, void *data)
{
	struct pci_irq_vector_data *vec_data =
		container_of(data, struct pci_irq_vector_data, bus_trans);
	struct mtk_bus_trans *bus_trans = vec_data->bus_trans;
	struct pci_trans *trans = to_pci_trans(bus_trans);
	u8 map_idx = vec_data->isr_map_idx;
	u32 ints, cur;

#ifdef CONFIG_HWIFI_DBG_ISR
	atomic_inc(&bus_trans->bus_dbg.irq_cnt);
#endif
	ints = bus_read(bus_trans, trans->int_mask_map[map_idx].int_sts_addr);
	ints &= vec_data->int_ena_mask;
	bus_write(bus_trans, trans->int_mask_map[map_idx].int_sts_addr, ints);

	if (test_bit(PCI_STATE_ISR_PEDING, &trans->state))
		goto end;

	while (ints) {
		cur = ffs(ints)-1;
		pci_dma_irq_disable(vec_data, 1 << cur);

		if (trans->isr_map[map_idx][cur]) {
#ifdef CONFIG_HWIFI_DBG_ISR
			if (map_idx == LEGACY_IDX && bus_trans->bus_dbg.legacy_ring_irq_cnt)
				atomic_inc(&bus_trans->bus_dbg.legacy_ring_irq_cnt[cur]);
			else if (map_idx == RSS_IDX && bus_trans->bus_dbg.rss_ring_irq_cnt)
				atomic_inc(&bus_trans->bus_dbg.rss_ring_irq_cnt[cur]);
#endif
			napi_schedule(trans->isr_map[map_idx][cur]);
		}

		ints &= ints-1;
	}
end:
	return IRQ_HANDLED;
}

inline void
pci_dma_kick_queue(struct pci_trans *trans, struct pci_queue *q)
{
	spin_lock(&q->lock);
	bus_write(to_bus_trans(trans), q->hw_cidx_addr, q->head);
	spin_unlock(&q->lock);
}

static int
pci_dma_tx_tail_sync(struct pci_trans *trans,
				struct pci_queue *q, int budget)
{
	struct pci_dma_cb *cb;
	struct pci_dma_desc *desc;
	struct sk_buff *skb;
	int done = 0;

	while (done < budget) {
		cb = &q->cb[q->tail];
		desc = cb->alloc_va;

		if (q->tail == q->head ||
		    !(desc->dw1 & cpu_to_le32(BIT(TXDMAD_DDONE_SHIFT))))
			return done;

		if (q->q_attr != Q_TX_DATA && cb->pkt) {
			skb = (struct sk_buff *)cb->pkt;

			dma_unmap_single(to_device(trans),
				cb->pkt_pa, skb->len, DMA_TO_DEVICE);
			dev_kfree_skb(skb);
			cb->pkt = NULL;
		}

		q->tail = (q->tail+1) % q->q_size;
		done++;
	}

	return done;
}

static int
pci_dma_tx_done_poll(struct napi_struct *napi, int budget)
{
	struct pci_trans *trans;
	struct pci_queue *q;
	struct pci_irq_vector_data *vec_data;
	int done;

	trans = container_of(napi->dev, struct pci_trans, napi_dev);
	q = container_of(napi, struct pci_queue, napi);
	vec_data = &trans->vec_data[q->vec_id];

	local_bh_disable();
	mt_rcu_read_lock();
	done = pci_dma_tx_tail_sync(trans, q, budget);
	mt_rcu_read_unlock();
	local_bh_enable();

	if (done < budget) {
		napi_complete(napi);
		pci_dma_irq_enable(vec_data, q->hw_int_mask);
	}

	return done;
}

static int
pci_dma_swi_ser(
	struct mtk_hw_dev *dev,
	u32 chip_id,
	u32 ser_level,
	u32 ser_event,
	u32 hw_id)
{
	return mtk_bus_rx_ser_event(
		dev,
		chip_id,
		ser_level,
		ser_event,
		hw_id);
}

static int
pci_dma_wdt_ser(
	struct mtk_hw_dev *dev,
	u32 chip_id,
	u32 ser_level,
	u32 ser_event,
	u32 hw_id)
{
	return mtk_bus_rx_ser_event(
		dev,
		chip_id,
		ser_level,
		ser_event,
		hw_id);
}

static int
pci_dma_swi_handler(struct napi_struct *napi, int budget)
{
	struct pci_trans *trans;
	struct mtk_bus_trans *bus_trans;
	struct mtk_hw_dev *dev;
	struct pci_irq_vector_data *vec_data;
	u32 swi_sta = 0, swi_ena_wdt = 0;

	trans = container_of(napi->dev, struct pci_trans, napi_dev);
	bus_trans = to_bus_trans(trans);
	dev = to_hw_dev(trans);
	vec_data = &trans->vec_data[trans->mcu2host_sw_int_vec_id];

	local_bh_disable();
	mt_rcu_read_lock();

	swi_sta = bus_read(bus_trans, trans->mcu2host_sw_int_sta_addr);
	if (swi_sta & trans->mcu2host_sw_int_ser_mask)
		pci_dma_swi_ser(
			dev,
			dev->chip_drv->device_id,
			HW_SER_LV_1_0,
			swi_sta & trans->mcu2host_sw_int_ser_mask,
			dev->sys_idx.idx);

	if (swi_sta & trans->mcu2host_sw_wdt_ser_mask) {
		pci_dma_wdt_ser(
			dev,
			dev->chip_drv->device_id,
			HW_SER_LV_0_5,
			swi_sta & trans->mcu2host_sw_wdt_ser_mask,
			dev->sys_idx.idx);

	/* disable wdt_irq_setting ,to stop wa wdt timeout always trigger irq*/
		swi_ena_wdt =
			bus_read(bus_trans, trans->mcu2host_sw_int_ena_addr);
		swi_ena_wdt &=
			~(trans->mcu2host_sw_wdt_ser_mask);
		bus_write(bus_trans,
			trans->mcu2host_sw_int_ena_addr, swi_ena_wdt);
	}

	bus_write(bus_trans, trans->mcu2host_sw_int_sta_addr, swi_sta);

	mt_rcu_read_unlock();
	local_bh_enable();

	napi_complete_done(napi, 0);
	pci_dma_irq_enable(vec_data, trans->int_enable_swi_mask);

	return 0;
}

static inline void
pci_tx_queue_read(struct seq_file *s, struct pci_trans *trans, struct pci_tx_queue *txq)
{
	struct pci_queue *q = &txq->q;
	u32 base, cnt, cidx, didx;

	seq_printf(s, "%s:\n", q->q_info);

	if (q->disable) {
		seq_puts(s, "\tSkip disabled queue\n");
		return;
	}

	base = bus_read(to_bus_trans(trans), q->hw_desc_base);
	cnt = bus_read(to_bus_trans(trans), q->hw_cnt_addr);
	/* Tx queue should read didx first */
	didx = bus_read(to_bus_trans(trans), q->hw_didx_addr);
	cidx = bus_read(to_bus_trans(trans), q->hw_cidx_addr);

	seq_printf(s, "\tbase: 0x%x, cpuidx: 0x%x, dmaidx: 0x%x, cnt: 0x%x\n",
		base, cidx, didx, cnt);
	seq_printf(s, "\thead: 0x%x, tail: 0x%x\n", q->head, q->tail);
}

static inline void
pci_rx_queue_read(struct seq_file *s, struct pci_trans *trans, struct pci_rx_queue *rxq)
{
	struct pci_queue *q = &rxq->q;
	u32 base, cnt, cidx, didx;

	seq_printf(s, "%s:\n", q->q_info);

	if (q->disable) {
		seq_puts(s, "\tSkip disabled queue\n");
		return;
	}

	base = bus_read(to_bus_trans(trans), q->hw_desc_base);
	cnt = bus_read(to_bus_trans(trans), q->hw_cnt_addr);
	/* Rx queue should read cidx first */
	cidx = bus_read(to_bus_trans(trans), q->hw_cidx_addr);
	didx = bus_read(to_bus_trans(trans), q->hw_didx_addr);

	seq_printf(s, "\tbase: 0x%x, cpuidx: 0x%x, dmaidx: 0x%x, cnt: 0x%x\n",
		base, cidx, didx, cnt);
	seq_printf(s, "\thead: 0x%x, tail: 0x%x, magic_cnt(en = %u): 0x%x\n",
		q->head, q->tail, rxq->magic_enable, rxq->magic_cnt);
}

static int
pci_queues_read(void *hw, struct seq_file *s)
{
	struct pci_trans *trans = to_pci_trans(hw);
	struct mtk_bus_trans *bus_trans = to_bus_trans(trans);
	bool master = test_bit(BUS_TRANS_FLAG_MASTER, &bus_trans->flag) ? true : false;
	int i;

	seq_printf(s, "%s\n", master ? "master" : "slave");
	seq_puts(s, "txq:\n");

	for (i = 0 ; i < trans->txq_num; i++)
		pci_tx_queue_read(s, trans, &trans->txq[i]);

	seq_puts(s, "rxq:\n");

	for (i = 0 ; i < trans->rxq_num; i++)
		pci_rx_queue_read(s, trans, &trans->rxq[i]);

	return 0;
}

static int
pci_dma_rxdmad_parser(struct pci_trans *trans,
	struct pci_dma_desc *desc, struct mtk_bus_rx_info *rx_info)
{
	u32 dw0 = le32_to_cpu(READ_ONCE(desc->dw0));
	u32 dw1 = le32_to_cpu(READ_ONCE(desc->dw1));
	u32 dw2 = le32_to_cpu(READ_ONCE(desc->dw2));
	u32 dw3 = le32_to_cpu(READ_ONCE(desc->dw3));

	rx_info->sdp0 = GET_FIELD(RXDMAD_SDP0, dw0);
#ifdef CONFIG_HWIFI_64BIT_SUPPORT
	rx_info->sdp0 |= (GET_FIELD(RXDMAD_SDP0_H, dw2) << DMA_ADDR_H_SHIFT);
#endif
	rx_info->id = GET_FIELD(RXDMAD_TOKEN_ID, dw2);
	rx_info->len = GET_FIELD(RXDMAD_SDL0, dw1);
	rx_info->more = !GET_FIELD(RXDMAD_LS0, dw1);
	rx_info->ip_frag = GET_FIELD(RXDMAD_F, dw3);

	if (dw3 & RXDMAD_PPE_VLD_MASK) {
		rx_info->ppe_entry = GET_FIELD(RXDMAD_PPE_ENTRY, dw3);
		rx_info->csrn = GET_FIELD(RXDMAD_CSRN, dw3);

		if (GET_FIELD(RXDMAD_IND_REASON, dw2) != IND_REASON_CANNOT_DO_REORDER)
			rx_info->hw_path = true;
	}

	if (dw1 & RXDMAD_VER_MASK) {
		if (GET_FIELD(RXDMAD_PN_CHK_FAIL, dw1) ||
		    GET_FIELD(RXDMAD_IND_REASON, dw2) == IND_REASON_PN_CHK_FAIL)
			rx_info->pn_chk_fail = true;

		/* Decide drop in mac rxd parser */
		if (GET_FIELD(RXDMAD_IND_REASON, dw2) == IND_REASON_REPEAT)
			rx_info->repeat_pkt = 1;
		else if (GET_FIELD(RXDMAD_IND_REASON, dw2) == IND_REASON_OLDPKT)
			rx_info->old_pkt = 1;
	} else {
		rx_info->drop = dw1 & (RXDMAD_RXD_DROP_MASK | RXDMAD_TO_HOST_A_MASK) ?
				RXDMAD_VER_ZREO_DROP : RXDMAD_NON_DROP;
	}


	mtk_dbg(MTK_BUS, "%s: rxdmad_version = %d\n", __func__,
						GET_FIELD(RXDMAD_VER, dw1));
	mtk_dbg(MTK_BUS, "%s: id = %d\n", __func__,
						rx_info->id);
	mtk_dbg(MTK_BUS, "%s: len = %d\n", __func__,
						rx_info->len);
	mtk_dbg(MTK_BUS, "%s: more = %d\n", __func__,
						rx_info->more);
	mtk_dbg(MTK_BUS, "%s: hw_path = %d\n", __func__,
						rx_info->hw_path);
	mtk_dbg(MTK_BUS, "%s: ppe_entry = %d\n", __func__,
						rx_info->ppe_entry);
	mtk_dbg(MTK_BUS, "%s: csrn = %d\n", __func__,
						rx_info->csrn);
	mtk_dbg(MTK_BUS, "%s: drop = %d\n", __func__,
						rx_info->drop);

	return 0;
}

static int
pci_dma_get_event_buf(struct pci_trans *trans,
	struct pci_queue *q, u32 idx, struct mtk_bus_rx_info *rx_info)
{
	struct pci_dma_cb *cb = &q->cb[idx];
	struct pci_dma_desc *desc = cb->alloc_va;
	struct pci_dma_buf *dma_buf = &cb->dma_buf;
	void *new_pkt;
	struct pci_dma_buf new_dma_buf;

	pci_dma_rxdmad_parser(trans, desc, rx_info);

	new_dma_buf.alloc_size = dma_buf->alloc_size;
	new_dma_buf.alloc_va = 0;
	new_dma_buf.alloc_pa = 0;
	new_pkt = pci_dma_rx_pkt_alloc(to_device(trans),
		(struct pci_rx_queue *)q, &new_dma_buf, true);
	if (new_pkt == NULL)
		return -ENOMEM;

	dma_unmap_single(to_device(trans),
		dma_buf->alloc_pa, dma_buf->alloc_size, DMA_FROM_DEVICE);
	/*event through cb base manager*/
	rx_info->pkt = cb->pkt;

	/*rx refill*/
	cb->pkt = new_pkt;
	dma_buf->alloc_va = new_dma_buf.alloc_va;
	dma_buf->alloc_pa = new_dma_buf.alloc_pa;

	clear_ddone(cb->alloc_va);
	fill_rxd(cb, 0, 0);
	return 0;
}

static int
pci_dma_get_data_buf(struct pci_trans *trans,
	struct pci_queue *q, u32 idx, struct mtk_bus_rx_info *rx_info)
{
	struct pci_dma_cb *cb = &q->cb[idx];
	struct pci_dma_desc *desc = cb->alloc_va;
	struct pci_dma_buf *dma_buf = &cb->dma_buf;
	struct pci_rx_queue *rq = (struct pci_rx_queue *) q;
	void *new_pkt;
	struct pci_dma_buf new_dma_buf;

	pci_dma_rxdmad_parser(trans, desc, rx_info);

	new_dma_buf.alloc_size = dma_buf->alloc_size;
	new_dma_buf.alloc_va = 0;
	new_dma_buf.alloc_pa = 0;
	new_pkt = pci_dma_rx_pkt_alloc(to_device(trans),
		(struct pci_rx_queue *)q, &new_dma_buf, true);
	if (new_pkt == NULL)
		return -ENOMEM;

	if (q->q_attr == Q_RX_DATA) {
		dma_unmap_single(to_device(trans), dma_buf->alloc_pa,
			dma_buf->alloc_size, DMA_FROM_DEVICE);
		rx_info->pkt = cb->pkt;
	}

	/*rx refill*/
	cb->pkt = new_pkt;
	dma_buf->alloc_va = new_dma_buf.alloc_va;
	dma_buf->alloc_pa = new_dma_buf.alloc_pa;

	/*update to token manager*/
	if (rq->q.q_attr == Q_RX_DATA_MSDU_PG) {
		rro_msdu_page_hash_init(trans, cb->pkt, dma_buf->alloc_va,
			dma_buf->alloc_pa);

		if (idx == 0)
			rq->magic_cnt = (rq->magic_cnt + 1) %
					MAX_RXDMAD_MAGIC_CNT;
	}

	clear_ddone(cb->alloc_va);
	fill_rxd(cb, rx_info->id, rq->magic_cnt);

	return 0;
}

static int
pci_dma_get_data_buf_by_copy(struct pci_trans *trans,
	struct pci_queue *q, u32 idx, struct mtk_bus_rx_info *rx_info)
{
	struct pci_dma_cb *cb = &q->cb[idx];
	struct pci_dma_desc *desc = cb->alloc_va;
	struct pci_dma_buf *dma_buf = &cb->dma_buf;
	struct pci_rx_queue *rq = (struct pci_rx_queue *) q;
	void *pkt_va, *new_pkt;
	dma_addr_t pkt_pa;
	struct pci_dma_buf new_dma_buf;
	int offset, ret;

	pci_dma_rxdmad_parser(trans, desc, rx_info);

	ret = mtk_rx_tk_entry_find(to_rx_tk_mgmt(trans), rx_info->id,
				   &rx_info->pkt, &pkt_va, &pkt_pa);
	if (ret)
		return ret;

	new_dma_buf.alloc_size = dma_buf->alloc_size;
	new_dma_buf.alloc_va = NULL;
	new_dma_buf.alloc_pa = 0;
	new_pkt = pci_dma_rx_pkt_alloc(to_device(trans), rq, &new_dma_buf, false);

	if (new_pkt == NULL)
		return -ENOMEM;

	offset = SKB_HEADROOM_RSV - (pkt_va - rx_info->pkt);

	if (unlikely(offset < 0)) {
		dev_err_once(to_device(trans),
			"%s(): idx = %u, tkid = %u, offset = %d should not negative\n",
			__func__, idx, rx_info->id, offset);
		goto err;
	}

	if (unlikely(rx_info->sdp0 != pkt_pa)) {
		dev_err_once(to_device(trans),
			"%s(): idx = %u, tkid = %u, SDP0 = %pad and PKT_PA = %pad not match\n",
			__func__, idx, rx_info->id, &rx_info->sdp0, &pkt_pa);
		dev_err_once(to_device(trans),
			"%s(): RXDMAD DW0: 0x%08x DW1: 0x%08x DW2: 0x%08x DW3: 0x%08x\n",
			__func__, desc->dw0, desc->dw1, desc->dw2, desc->dw3);
		goto err;
	}

	dma_sync_single_for_cpu(to_device(trans), pkt_pa,
		dma_buf->alloc_size, DMA_FROM_DEVICE);

	memcpy(new_pkt + offset, rx_info->pkt, rx_pkt_total_size(dma_buf->alloc_size - offset));
	rx_info->pkt = new_pkt;
	dma_buf->alloc_va = pkt_va;
	dma_buf->alloc_pa = pkt_pa;

	dma_sync_single_for_device(to_device(trans), pkt_pa,
		dma_buf->alloc_size, DMA_FROM_DEVICE);

	clear_ddone(cb->alloc_va);
	fill_rxd(cb, rx_info->id, rq->magic_cnt);

	return 0;
err:
	put_page(virt_to_head_page(new_pkt));
	return -EINVAL;
}


static inline int
pci_dma_get_buf(struct pci_trans *trans,
	struct pci_queue *q, u32 idx, struct mtk_bus_rx_info *rx_info)
{
	int ret = 0;
	struct pci_rx_queue *rq = (struct pci_rx_queue *) q;

	if (rq->q_ops->get_data)
		ret = rq->q_ops->get_data(trans, q, idx, rx_info);

	if (ret)
		return ret;

	q->head = idx;

	return ret;
}

static int
pci_dma_pause_trxq(void *hw, bool enable)
{
	struct pci_trans *trans = to_pci_trans(hw);
	u32 mask = enable ? trans->pause_trxq_mask : 0;

	bus_write(to_bus_trans(trans), trans->pause_trxq_reg_addr, mask);
	return 0;
}

static int
pci_dma_dequeue(struct pci_trans *trans,
	struct pci_queue *q, bool flush, struct mtk_bus_rx_info *rx_info)
{
	u32 idx = (q->head + 1) % q->q_size;
	struct pci_dma_desc *desc = q->cb[idx].alloc_va;
	struct pci_rx_queue *rq = (struct pci_rx_queue *)q;

	if (flush) {
		desc->dw1 |= cpu_to_le32(BIT(RXDMAD_DDONE_SHIFT));
	} else if (rq->get_pkt_method == GET_PKT_IO) {
		if (idx == q->tail)
			return -EINVAL;
	} else if (test_bit(PCI_FLAG_WHNAT_TX, &trans->flags) &&
			q->q_attr == Q_RX_EVENT_TX_FREE_DONE) {
		if (!(desc->dw1 & cpu_to_le32(BIT(RXDMAD_DDONE_SHIFT))) ||
			!(desc->dw1 & cpu_to_le32(BIT(RXDMAD_M_DONE_SHIFT))))
			return -EINVAL;
	} else if (!(desc->dw1 & cpu_to_le32(BIT(RXDMAD_DDONE_SHIFT)))) {
		return -EINVAL;
	}

	return pci_dma_get_buf(trans, q, idx, rx_info);
}

void
pci_dma_gather_frag(struct pci_trans *trans, struct pci_rx_queue *rq,
				struct mtk_bus_rx_info *rx_info)
{
	struct sk_buff *skb = rq->rx_frag_head;
	struct skb_shared_info *shinfo = skb_shinfo(skb);
	int nr_frags = shinfo->nr_frags;

	if (nr_frags < ARRAY_SIZE(shinfo->frags)) {
		struct page *page = virt_to_head_page(rx_info->pkt);
		int offset = rx_info->pkt - page_address(page) + SKB_HEADROOM_RSV;

		skb_add_rx_frag(skb, nr_frags,
				page, offset, rx_info->len,
				rx_pkt_total_size(rq->rx_buf_size));
	}
}

int
pci_dma_rx_process(struct pci_trans *trans, struct pci_queue *q, int budget)
{
	struct sk_buff *skb = NULL;
	struct pci_rx_queue *rq = (struct pci_rx_queue *)q;
	struct mtk_bus_rx_info rx_info = {0};
	struct mtk_bus_rx_info *skb_rx_info;
	int done = 0;
#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct mtk_rx_process rx_process = {0};

	rx_process.trans = (struct mtk_bus_trans *)trans;
	rx_process.q_attr = q->q_attr;
#endif

	while (done < budget) {

#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
		if (rq->rx_offload.kfifo_enable && kfifo_is_full(&rq->bus_rx_process_fifo)) {
			dbg_bus_kfifo_full_inc(to_bus_trans(trans));
			mdelay(5);
			continue;
		}
#endif

		memset(&rx_info, 0, sizeof(rx_info));

		if (pci_dma_dequeue(trans, q, false, &rx_info)) {
			/*update rx cidx*/
			pci_dma_kick_queue(trans, q);
			break;
		}

		if (!rx_info.pkt)
			continue;

		if (q->q_attr == Q_RX_DATA_RRO ||
			q->q_attr == Q_RX_DATA_MSDU_PG) {
			done++;
			continue;
		}

		if (rq->rx_frag_head) {
			pci_dma_gather_frag(trans, rq, &rx_info);

			if (rx_info.more)
				continue;

			skb = rq->rx_frag_head;
			rq->rx_frag_head = NULL;
			goto process;
		}

		skb = build_skb(rx_info.pkt,
						rx_pkt_total_size(rq->rx_buf_size));

		/* TODO: Need to fix if fragment packet. */
		if (unlikely(!skb)) {
			put_page(virt_to_head_page(rx_info.pkt));
			dev_err(to_device(trans), "%s(): build skb failed\n",
				__func__);
			done++;
			continue;
		}

		skb_reserve(skb, SKB_HEADROOM_RSV);

		__skb_put(skb, rx_info.len);
		rx_info.skb = skb;

		skb_rx_info = (struct mtk_bus_rx_info *)skb->cb;
		*skb_rx_info = rx_info;

		if (rx_info.hw_path) {
			SET_PACKET_TYPE(rx_info.skb, RX_PPE_VALID);
		}

		if (rx_info.more) {
			rq->rx_frag_head = skb;
			continue;
		}

process:

#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
		if (likely(rq->rx_offload.kfifo_enable)) {
			rx_process.skb = skb;
			if (unlikely(!kfifo_put(&rq->bus_rx_process_fifo, rx_process))) {
				dbg_bus_kfifo_full_drop_by_hwifi_inc(to_bus_trans(trans));
				dev_err(dev->dev, "%s(): rx_data fifo full\n\n", __func__);
				dev_kfree_skb(skb);
				break;
			}
		} else {
			mtk_bus_rx_process((struct mtk_bus_trans *)trans,
							q->q_attr, skb);
		}
#else
		mtk_bus_rx_process((struct mtk_bus_trans *)trans,
							q->q_attr, skb);
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */

		done++;
	}

#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
	if (rq->rx_offload.kfifo_enable)
		queue_delayed_work_on(rq->work_on_cpu, rq->bus_rx_process_wq,
								&rq->bus_rx_process_work, 0);
#else
	mtk_bus_rx_poll((struct mtk_bus_trans *)trans, &q->napi, ffs(q->band_idx_bmp) - 1);
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */

	return done;
}

static int
pci_dma_rx_poll(struct napi_struct *napi, int budget)
{
	struct pci_queue *q;
	struct pci_trans *trans;
	struct pci_irq_vector_data *vec_data;
	int done = 0, cur;
	struct pci_rx_queue *rq;

	trans = container_of(napi->dev, struct pci_trans, napi_dev);
	q = container_of(napi, struct pci_queue, napi);
	vec_data = &trans->vec_data[q->vec_id];
	rq = (struct pci_rx_queue *)q;

	q->tail = bus_get_field(to_bus_trans(trans),
			q->hw_didx_addr, q->hw_didx_mask);

	mt_rcu_read_lock();

	do {
		cur = rq->q_ops->process(trans, q, budget - done);
		done += cur;
	} while (cur && done < budget);

	mt_rcu_read_unlock();

	if (done < budget) {
		/*sync sw idx*/
		q->tail = bus_get_field(to_bus_trans(trans),
				q->hw_didx_addr, q->hw_didx_mask);

		/* reschedule if ring is not empty */
		if (((q->head + 1) % q->q_size) != q->tail)
			return budget;

		napi_complete(napi);
		pci_dma_irq_enable(vec_data, q->hw_int_mask);
	}
	return done;
}

static u8
pci_dma_get_vec_id_by_int_mask(struct pci_trans *trans, u8 isr_map_idx, u32 mask)
{
	int nr = 0;
	u8 vec_id = 0;
	struct mtk_hw_dev *dev = to_hw_dev(trans);

	for (; nr < dev->vec_num; nr++) {
		if ((mask & trans->vec_data[nr].int_ena_mask) &&
				(isr_map_idx == trans->vec_data[nr].isr_map_idx)) {
			vec_id = nr;
			break;
		}
	}

	return vec_id;
}

static int
pci_dma_queue_napi_init(struct pci_trans *trans,
	struct pci_queue *q, u8 res_id, void *cb)
{
	u8 map_id = ffs(q->hw_int_mask)-1;
	u8 vec_id = pci_dma_get_vec_id_by_int_mask(trans, q->isr_map_idx, 1 << map_id);

	if (map_id >= 32)
		return -EINVAL;

	q->vec_id = vec_id;
	q->resource_idx = res_id;
	trans->isr_map[q->isr_map_idx][map_id] = &q->napi;
	netif_napi_add_weight(&trans->napi_dev, &q->napi, cb, NAPI_POLL_WEIGHT);
	napi_enable(&q->napi);
	return 0;
}

static int
pci_dma_queue_napi_exit(struct pci_trans *trans, struct pci_queue *q)
{
	u8 map_id = ffs(q->hw_int_mask)-1;

	if (map_id >= 32)
		return -EINVAL;

	if (trans->isr_map[q->isr_map_idx][map_id]) {
		trans->isr_map[q->isr_map_idx][map_id] = NULL;
		napi_disable(&q->napi);
		netif_napi_del(&q->napi);
	}

	return 0;
}

static void
pci_dma_exit_tx_queue(struct pci_trans *trans, struct pci_tx_queue *tq)
{
	struct pci_dma_cb *cb;
	struct pci_queue *q = &tq->q;
	u16 q_sz = q->q_size;
	int i;

	for (i = 0 ; i < q_sz; i++) {
		cb = &q->cb[i];

		if (q->q_attr != Q_TX_DATA) {
			struct sk_buff *skb = (struct sk_buff *)cb->pkt;

			if (skb) {
				dma_unmap_single(to_device(trans),
					cb->pkt_pa, skb->len, DMA_TO_DEVICE);
				dev_kfree_skb(skb);
				cb->pkt = NULL;
			}
		}
	}
}

static int
pci_dma_sw_init_tx_queue(struct pci_trans *trans)
{
	struct pci_tx_queue *tq;
	u8 tn = trans->txq_num;
	int i;
	int ret = 0;

	for (i = 0 ; i < tn; i++) {
		tq = &trans->txq[i];

		if (tq->q.disable)
			continue;

		pci_dma_init_tx_data_queue(trans, tq);
		ret = pci_dma_queue_napi_init(trans,
			(struct pci_queue *) tq, i, pci_dma_tx_done_poll);
		if (ret)
			goto end;
	}
end:
	return ret;
}

static void
pci_dma_sw_exit_tx_queue(struct pci_trans *trans)
{
	struct pci_tx_queue *tq;
	u8 tn = trans->txq_num;
	int i;

	/*txq*/
	for (i = 0 ; i < tn; i++) {
		tq = &trans->txq[i];

		if (tq->q.disable)
			continue;

		pci_dma_queue_napi_exit(trans, (struct pci_queue *)tq);
		pci_dma_exit_tx_queue(trans, tq);
	}
}

static int
pci_dma_sw_init_mcu2host_swi(struct pci_trans *trans)
{
	u8 map_id = ffs(trans->int_enable_swi_mask)-1;
	u8 isr_map_idx = trans->mcu2host_sw_int_isr_map_idx;
	u8 vec_id = pci_dma_get_vec_id_by_int_mask(trans,
			isr_map_idx, trans->int_enable_swi_mask);

	if (map_id >= 32)
		return -EINVAL;

	trans->mcu2host_sw_int_vec_id = vec_id;
	trans->isr_map[isr_map_idx][map_id] = &trans->swi_napi;
	netif_napi_add_weight(&trans->napi_dev, &trans->swi_napi,
			pci_dma_swi_handler, NAPI_POLL_WEIGHT);
	napi_enable(&trans->swi_napi);
	return 0;
}

static int
pci_dma_sw_exit_mcu2host_swi(struct pci_trans *trans)
{
	u8 map_id = ffs(trans->int_enable_swi_mask)-1;
	u8 isr_map_idx = trans->mcu2host_sw_int_isr_map_idx;

	if (map_id >= 32)
		return -EINVAL;

	trans->isr_map[isr_map_idx][map_id] = NULL;
	napi_disable(&trans->swi_napi);
	netif_napi_del(&trans->swi_napi);
	return 0;
}

static inline void
pci_dma_init_rx_queue(struct pci_trans *trans, struct pci_rx_queue *rq)
{
	if (rq->q_ops->init)
		rq->q_ops->init(trans, rq);
}

static int
pci_dma_sw_init_rx_queue(struct pci_trans *trans)
{
	struct pci_rx_queue *rq;
	u8 rn = trans->rxq_num;
	int i;
	int ret = 0;

	for (i = 0 ; i < rn; i++) {
		rq = &trans->rxq[i];

		if (rq->q.disable)
			continue;

		pci_dma_init_rx_queue(trans, rq);
		ret = pci_dma_queue_napi_init(trans,
			(struct pci_queue *) rq, i, pci_dma_rx_poll);
		if (ret)
			goto end;
	}
end:
	return ret;
}

static inline void
pci_dma_exit_rx_queue(struct pci_trans *trans, struct pci_rx_queue *rq)
{
	if (rq->q_ops->exit)
		rq->q_ops->exit(trans, rq);
}

static int
pci_dma_sw_exit_rx_queue(struct pci_trans *trans)
{
	struct pci_rx_queue *rq;
	u8 rn = trans->rxq_num;
	int i;

	for (i = 0 ; i < rn; i++) {
		rq = &trans->rxq[i];

		if (rq->q.disable)
			continue;

		pci_dma_queue_napi_exit(trans, (struct pci_queue *) rq);
		pci_dma_exit_rx_queue(trans, rq);
	}
	return 0;
}

static void
pci_dma_sw_preinit_irq(struct pci_trans *trans)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	int nr;

	for (nr = 0; nr < MAX_MAP_IDX; nr++) {
		trans->int_mask_map[nr].int_ena_mask |=
			trans->int_mask_map[nr].int_dis_mask;
		trans->int_mask_map[nr].int_dis_mask = 0;
	}

	for (nr = 0; nr < dev->vec_num; nr++) {
		trans->vec_data[nr].int_ena_mask |= trans->vec_data[nr].int_dis_mask;
		trans->vec_data[nr].int_dis_mask = 0;
	}
}

static int
pci_dma_sw_init_host_int(struct pci_trans *trans)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	int nv = 0;

	for (; nv < dev->vec_num; nv++) {
		u8 map_id = trans->vec_data[nv].isr_map_idx;

		trans->int_mask_map[map_id].int_ena_mask |=
			trans->vec_data[nv].int_ena_mask;
	}

	return 0;
}

static int
pci_dma_sw_init_irq(struct pci_trans *trans)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	char *name;
	int irq, nvec;
	int nr = 0, i = 0;
	int ret = 0;
	u8 alloc_vec_num = dev->alloc_vec_num ?
			   dev->alloc_vec_num : dev->vec_num;

	nvec = pci_alloc_irq_vectors(trans->pdev, alloc_vec_num,
			alloc_vec_num, dev->irq_type);
	if (nvec != alloc_vec_num)  {
		dev_err(to_device(trans),
			"%s(): Allocate %u vector(s) failed\n",
			__func__, alloc_vec_num);
		return nvec;
	}

	for (; nr < dev->vec_num; nr++) {
		name = trans->vec_data[nr].irq_name;
		irq = pci_irq_vector(trans->pdev, nr);
		trans->vec_data[nr].irq = irq;

		ret = snprintf(name, MAX_IRQ_NAME_LEN, "%s-vec_data%d", to_name(trans), nr);
		if (ret < 0) {
			dev_err(to_device(trans), "%s(): Name error\n", __func__);
			goto err;
		}

		ret = request_irq(irq, mtk_bus_dma_irq_handler,
			IRQF_SHARED, name, &trans->vec_data[nr]);
		if (ret)
			goto err;
	}

	return 0;
err:
	for (; i < nr; i++) {
		irq = pci_irq_vector(trans->pdev, i);
		free_irq(irq, &trans->vec_data[i]);
	}
	pci_free_irq_vectors(trans->pdev);
	return ret;
}

static void
pci_dma_sw_exit_irq(struct pci_trans *trans)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	int nr, irq;

	for (nr = 0; nr < dev->vec_num; nr++) {
		irq = pci_irq_vector(trans->pdev, nr);
		free_irq(irq, &trans->vec_data[nr]);
	}
	pci_free_irq_vectors(trans->pdev);
}

static int
pci_dma_sw_preinit_device(struct pci_trans *trans)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	int ret;

	if (test_bit(HWIFI_STATE_RUNNING, &dev->state))
		goto preinit_irq;

	ret = pci_chip_option_init(trans);
	if (ret)
		goto err;

preinit_irq:
	pci_dma_sw_preinit_irq(trans);
	ret = rro_sw_init(trans);

err:
	return ret;
}

static int
pci_dma_hw_preinit_device(struct pci_trans *trans)
{
	return 0;
}

static int
pci_dma_preinit_device(void *hw)
{
	struct pci_trans *trans = to_pci_trans(hw);
	int ret;

	ret = pci_dma_sw_preinit_device(trans);
	if (ret)
		goto err;

	ret = pci_dma_hw_preinit_device(trans);
	if (ret)
		goto err;

	return 0;
err:
	return ret;
}

static int
pci_dma_sw_init_device(struct pci_trans *trans)
{
	int ret;

	mt_init_dummy_netdev(&trans->napi_dev);

	ret = pci_dma_sw_init_tx_queue(trans);
	if (ret) {
		dev_err(to_device(trans), "%s(): TXQ init failed, ret = %d\n",
			__func__, ret);
		goto err;
	}

	ret = pci_dma_sw_init_rx_queue(trans);
	if (ret) {
		dev_err(to_device(trans), "%s(): RXQ init failed, ret = %d\n",
			__func__, ret);
		goto err;
	}

	ret = pci_dma_sw_init_mcu2host_swi(trans);
	if (ret) {
		dev_err(to_device(trans), "%s(): MCU2HOST init failed, ret = %d\n",
			__func__, ret);
		goto err;
	}

	ret = pci_dma_sw_init_host_int(trans);
	if (ret) {
		dev_err(to_device(trans), "%s(): Host interrupt init failed, ret = %d\n",
			__func__, ret);
		goto err;
	}
	/*interrupt sw initial*/
	ret = pci_dma_sw_init_irq(trans);
	if (ret) {
		dev_err(to_device(trans), "%s(): IRQ init failed, ret = %d\n",
			__func__, ret);
		goto err;
	}

	ret = pci_dma_sw_init_dbg(trans);
	if (ret) {
		dev_err(to_device(trans), "%s(): debug init failed, ret = %d\n",
			__func__, ret);
		goto err;
	}

	ret = pci_dma_sw_init_dmy(trans);
	if (ret) {
		dev_err(to_device(trans), "%s(): dmy init failed, ret = %d\n",
			__func__, ret);
		goto err;
	}

	return 0;
err:
	return -EINVAL;
}

static int
pci_dma_hw_init_device(struct pci_trans *trans)
{
	pci_chip_dma_schdl_init(trans);
	pci_chip_dma_init(trans);
	pci_dma_hw_init_queues(trans);

	return 0;
}

static int
pci_dma_init_device(void *hw)
{
	struct pci_trans *trans = to_pci_trans(hw);
	int ret;

	/*init txrx queue*/
	ret = pci_dma_sw_init_device(trans);
	if (ret)
		goto err;

	ret = pci_dma_hw_init_device(trans);
	if (ret)
		goto err;

	return 0;
err:
	return ret;
}

static int
pci_dma_init_after_fwdl(void *hw)
{
	struct pci_trans *trans = to_pci_trans(hw);

	if (trans->rro_mode)
		pci_chip_rro_hw_init(trans); /* ddr related cr setting */
	pci_dma_hw_init_queues_after_fwdl(trans);
	return 0;
}

static int
pci_dma_start(void *hw)
{
	struct pci_trans *trans = to_pci_trans(hw);

	return pci_chip_dma_enable(trans);
}

static void
pci_dma_stop(void *hw)
{
	struct pci_trans *trans = to_pci_trans(hw);

	pci_chip_dma_disable(trans);
}

static void
pci_dma_hw_exit_device(struct pci_trans *trans)
{
	pci_dma_hw_exit_queues(trans);
}

static void
pci_dma_sw_exit_device(struct pci_trans *trans)
{
	pci_dma_sw_exit_irq(trans);
	pci_dma_sw_exit_tx_queue(trans);
	pci_dma_sw_exit_rx_queue(trans);
	pci_dma_sw_exit_mcu2host_swi(trans);
	rro_sw_exit(trans);
	pci_dma_sw_exit_dbg(trans);
	pci_dma_sw_exit_dmy(trans);
}

static void
pci_dma_exit_device(void *hw)
{
	struct pci_trans *trans = to_pci_trans(hw);

	pci_dma_hw_exit_device(trans);
	pci_dma_sw_exit_device(trans);
}

static int
pci_dma_add_buf(struct pci_queue *q,
		 struct pci_queue_buf *buf, int nbufs, u32 dw3,
		 struct sk_buff *skb)
{
	struct pci_dma_desc *desc;
	u32 dw1;
	int i, idx = -1;

	for (i = 0; i < nbufs; i += 2) {
		u32 dw0 = 0, dw2 = 0;

		/* sdp0/sdp0_h/sdl0 */
		dw0 = SET_FIELD(TXDMAD_SDP0, buf[i].addr);
#ifdef CONFIG_HWIFI_64BIT_SUPPORT
		dw3 |= SET_FIELD(TXDMAD_SDP0_H,
				buf[i].addr >> DMA_ADDR_H_SHIFT);
#endif
		dw1 = SET_FIELD(TXDMAD_SDL0, buf[i].len);

		if (i < (nbufs - 1)) {
			/* sdp1/sdp1_h/sdl1 */
			dw2 = SET_FIELD(TXDMAD_SDP1, buf[i + 1].addr);
#ifdef CONFIG_HWIFI_64BIT_SUPPORT
			dw3 |= SET_FIELD(TXDMAD_SDP1_H,
				buf[i + 1].addr >> DMA_ADDR_H_SHIFT);
#endif
			dw1 |= SET_FIELD(TXDMAD_SDL1, buf[i + 1].len);
		}

		/* ls0/ls1 */
		if (i == (nbufs - 1))
			dw1 |= BIT(TXDMAD_LS0_SHIFT);
		else if (i == (nbufs - 2))
			dw1 |= BIT(TXDMAD_LS1_SHIFT);

		idx = q->head;
		desc = q->cb[idx].alloc_va;

		if (skb) {
			q->cb[idx].pkt = skb;
			q->cb[idx].pkt_pa = buf[i].addr;
		}

		/* Please keep order of writing TXDMAD */
		WRITE_ONCE(desc->dw0, cpu_to_le32(dw0));
		WRITE_ONCE(desc->dw2, cpu_to_le32(dw2));
		WRITE_ONCE(desc->dw3, cpu_to_le32(dw3));
		WRITE_ONCE(desc->dw1, cpu_to_le32(dw1));

		/* DMAD need update before q->head */
		smp_wmb();
		q->head = (q->head + 1) % q->q_size;

		mtk_dbg(MTK_BUS, "txd_dw0: 0x%x\n", desc->dw0);
		mtk_dbg(MTK_BUS, "txd_dw1: 0x%x\n", desc->dw1);
		mtk_dbg(MTK_BUS, "txd_dw2: 0x%x\n", desc->dw2);
		mtk_dbg(MTK_BUS, "txd_dw3: 0x%x\n", desc->dw3);
	}

	return idx;
}

static int
pci_dma_chip_ops_sanity(struct pci_chip_ops *chip_ops)
{
	if (!chip_ops->dma_enable ||
		!chip_ops->dma_disable ||
		!chip_ops->dma_init) {
		return -EINVAL;
	}
	return 0;
}

static int
pci_dma_chip_attached(void *hw, void *chip_ops)
{
	struct pci_trans *trans = to_pci_trans(hw);
	int ret;

	ret = pci_dma_chip_ops_sanity(chip_ops);

	if (ret)
		goto err;

	trans->chip_ops = (struct pci_chip_ops *) chip_ops;
	return 0;
err:
	return ret;
}

static bool
pci_dma_match(void *master, void *slave)
{
	struct pci_trans *m_trans = to_pci_trans(master);
	struct pci_trans *s_trans = to_pci_trans(slave);

	if (!pci_chip_dma_match(m_trans, s_trans))
		return false;

	return true;
}

static int
pci_dma_tx_queue_skb_raw(struct pci_trans *trans, u8 qid,
			  struct sk_buff *skb, u32 tx_info)
{
	struct pci_queue *q = (struct pci_queue *) &trans->txq[qid];
	struct pci_queue_buf buf;
	dma_addr_t addr;

	if (((q->head + 1) % q->q_size) == q->tail)
		goto err;

	mtk_dbg_dump(MTK_BUS, "pci_dma_tx_queue_skb_raw",
					skb->data, skb->len);

	addr = dma_map_single(to_device(trans), skb->data, skb->len,
			      DMA_TO_DEVICE);
	if (unlikely(dma_mapping_error(to_device(trans), addr)))
		goto err;

	buf.addr = addr;
	buf.len = skb->len;

	mtk_dbg(MTK_BUS, "%s(): va = %p\n", __func__, skb->data);
	mtk_dbg(MTK_BUS, "%s(): pa = 0x%llx\n", __func__,
				(unsigned long long)virt_to_phys(skb->data));
	mtk_dbg(MTK_BUS, "%s(): bus = %pad\n", __func__, &addr);

	spin_lock(&q->lock);
	pci_dma_add_buf(q, &buf, 1, tx_info, skb);
	spin_unlock(&q->lock);

	pci_dma_kick_queue(trans, q);
	return 0;
err:
	return -ENOMEM;
}

static int
pci_dma_tx_mcu_queue(void *hw,
	struct sk_buff *skb, enum mtk_queue_attr q)
{
	struct pci_trans *trans = to_pci_trans(hw);
	s8 txq;

	txq = pci_dma_get_txq_by_attr(trans, q, 0, 0);
	if (txq == -1)
		return -EINVAL;

	return pci_dma_tx_queue_skb_raw(trans, txq, skb, 0);
}

static bool
pci_dma_request_tx(void *hw, void *txq)
{
	struct pci_queue *q = (struct pci_queue *) txq;
	u32 res;
	u8 min_res = q->band_num_sup ? q->band_num_sup : 1;

	if (q->head >= q->tail)
		res = q->q_size - q->head + q->tail;
	else
		res = q->tail - q->head;

	return res > min_res;
}

static inline struct pci_dma_cb *
pci_dma_get_txcb(struct pci_queue *q)
{
	return &q->cb[q->head];
}

static int
pci_dma_tx_data_queue(void *hw, struct mtk_bus_tx_info *tx_info)
{
	struct pci_trans *trans = to_pci_trans(tx_info->txq_trans);
	struct pci_queue *q;
	struct pci_tx_queue *tq;
	struct pci_queue_buf buf[2] = {0};
	struct mtk_tk_entry *tk_entry = tx_info->tk_entries;
	struct mtk_hw_dev *dev;
	struct mtk_chip_drv *chip_drv;

	q = (struct pci_queue *)tx_info->txq;
	tq = (struct pci_tx_queue *)q;
	dev = to_hw_dev(trans);
	chip_drv = dev->chip_drv;

	spin_lock(&q->lock);

	if (chip_drv->hw_caps->mac_cap & BIT(CAP_OFFLOAD_TXD)) {
		buf[0].addr = tk_entry->dma_addr;
		buf[0].len = tk_entry->fbuf_dma_size;
		if (tk_entry->pkt_pa) {
			buf[1].addr = tk_entry->pkt_pa;
			buf[1].len = MT_CT_PARSE_LEN;
			pci_dma_add_buf(q, buf, 2, 0, NULL);
		} else {
			pci_dma_add_buf(q, buf, 1, 0, NULL);
		}
	} else {
		buf[0].addr = tk_entry->dma_addr;
		buf[0].len = tk_entry->fbuf_dma_size;
		buf[1].addr = 0;
		buf[1].len = 0;
		pci_dma_add_buf(q, buf, 1, 0, NULL);
	}

	tk_entry->rdy = true;
	dbg_bus_tx_inc(to_bus_trans(trans));
	trans->kick_txq_bitmap |= (1 << q->resource_idx);
	spin_unlock(&q->lock);

	return 0;
}

static int
pci_dma_tx_kick(void *hw)
{
	struct pci_trans *trans = to_pci_trans(hw);
	struct pci_tx_queue *tq;
	u8 i = 0;
	u16 txq_bitmap = trans->kick_txq_bitmap;

	while (txq_bitmap) {
		if (txq_bitmap & 0x1) {
			tq = &trans->txq[i];
			pci_dma_kick_queue(trans, (struct pci_queue *)tq);
		}

		txq_bitmap >>= 1;
		i++;
	}

	trans->kick_txq_bitmap = 0;

	return 0;
}

static void
pci_dma_tx_free(void *hw, struct sk_buff *skb)
{
}

/* TID 0~15 used in spec, 16 is reserved for driver used */
static u8 tid_to_acq[17] = {
	1, 0, 0, 1, 2, 2, 3, 3,
	1, 0, 0, 1, 2, 2, 3, 3,
	Q_TYPE_PRIO
};

static int
pci_dma_get_txq(void *hw, u8 band_idx, u8 tid, void **txq, void **txq_trans)
{
	struct pci_trans *trans = to_pci_trans(hw);
	struct pci_tx_queue *tq;
	s8 acq = tid < ARRAY_SIZE(tid_to_acq) ? tid_to_acq[tid] : -1;
	u8 i;

	if (band_idx >= MAX_BAND_NUM || acq == -1)
		return -EINVAL;

	for (i = 0; i < trans->txq_num; i++) {
		tq = &trans->txq[i];

		if (tq->q.disable)
			continue;

		if (tq->q.q_attr == Q_TX_DATA &&
		    tq->q.band_idx_bmp & BIT(band_idx) &&
		    (!tq->q.q_type_bmp || tq->q.q_type_bmp & BIT(acq))) {
			*txq = tq;
			*txq_trans = trans;
			return 0;
		}
	}

	return -EINVAL;
}

static int
pci_dma_int_to_mcu(void *hw, u32 status)
{
	struct pci_trans *trans = to_pci_trans(hw);

	bus_write(to_bus_trans(trans), trans->int_ser_reg_addr, status);
	return 0;
}

static int
pci_dma_sw_init_rx_queue_from_bus(void *hw)
{
	struct pci_trans *trans = to_pci_trans(hw);

	return pci_dma_sw_init_rx_queue(trans);
}

static int
pci_dma_sw_exit_rx_queue_from_bus(void *hw)
{
	struct pci_trans *trans = to_pci_trans(hw);

	return pci_dma_sw_exit_rx_queue(trans);
}

static int
pci_dma_switch_node(void *master, void *slave)
{
	struct mtk_hw_dev *dev = to_hw_dev(master);
	struct mtk_bus_cfg *bus_cfg = &dev->chip_drv->bus_cfg;
	struct pci_trans *pci_master = to_pci_trans(master);
	struct pci_trans *pci_slave = to_pci_trans(slave);
	struct pci_tx_queue *tq_master, *tq_slave;
	struct pci_rx_queue *rq_master, *rq_slave;
	u32 ring_no_master, ring_no_slave;
	u32 int_mask_unmap_master = 0, int_mask_unmap_slave = 0;
	int i, j;
	bool wed_mode = (bus_cfg->bus_type == MTK_BUS_WED);
	u8 handled_by;

	for (i = 0; i < pci_master->txq_num; i++) {
		tq_master = &pci_master->txq[i];
		ring_no_master = tq_master->q_mp_attr.ring_no;

		if (!ring_no_master)
			continue;

		for (j = 0; j < pci_slave->txq_num; j++) {
			tq_slave = &pci_slave->txq[j];
			ring_no_slave = tq_slave->q_mp_attr.ring_no;

			if (!ring_no_slave)
				continue;

			if (ring_no_master == ring_no_slave) {
				handled_by = wed_mode && tq_master->q_mp_attr.handled_by_wed_mode ?
					tq_master->q_mp_attr.handled_by_wed_mode :
					tq_master->q_mp_attr.handled_by;

				switch (handled_by) {
				case MASTER_NODE:
					tq_master->q.hw_desc_base = tq_slave->q.hw_desc_base;
					tq_slave->q.disable = true;
					int_mask_unmap_slave |= tq_slave->q.hw_int_mask;
					break;
				case SLAVE_NODE:
					tq_slave->q.hw_desc_base = tq_master->q.hw_desc_base;
					tq_master->q.disable = true;
					int_mask_unmap_master |= tq_master->q.hw_int_mask;
					break;
				case DISABLE_NODE:
					tq_master->q.disable = true;
					int_mask_unmap_master |= tq_master->q.hw_int_mask;
					break;
				case SHRINK_NODE:
					tq_master->q.q_size = 16;
					break;
				default:
					dev_err(to_device(master),
						"%s(): %s handled_by value = %u is not valid\n",
						__func__, tq_master->q.q_info,
						tq_master->q_mp_attr.handled_by);
				}
			}
		}
	}

	for (i = 0; i < pci_master->rxq_num; i++) {
		rq_master = &pci_master->rxq[i];
		ring_no_master = rq_master->q_mp_attr.ring_no;

		if (!ring_no_master)
			continue;

		for (j = 0; j < pci_slave->rxq_num; j++) {
			rq_slave = &pci_slave->rxq[j];
			ring_no_slave = rq_slave->q_mp_attr.ring_no;

			if (!ring_no_slave)
				continue;

			if (ring_no_master == ring_no_slave) {
				handled_by = wed_mode && rq_master->q_mp_attr.handled_by_wed_mode ?
					rq_master->q_mp_attr.handled_by_wed_mode :
					rq_master->q_mp_attr.handled_by;

				switch (handled_by) {
				case MASTER_NODE:
					rq_master->q.hw_desc_base = rq_slave->q.hw_desc_base;
					rq_slave->q.disable = true;
					int_mask_unmap_slave |= rq_slave->q.hw_int_mask;
					break;
				case SLAVE_NODE:
					rq_slave->q.hw_desc_base = rq_master->q.hw_desc_base;
					rq_master->q.disable = true;
					int_mask_unmap_master |= rq_master->q.hw_int_mask;
					break;
				case DISABLE_NODE:
					rq_master->q.disable = true;
					int_mask_unmap_master |= rq_master->q.hw_int_mask;
					break;
				case SHRINK_NODE:
					rq_master->q.q_size = 16;
					rq_master->attr_enable = !rq_slave->attr_enable;
					break;
				default:
					dev_err(to_device(master),
						"%s(): %s handled_by value = %u is not valid\n",
						__func__, rq_master->q.q_info,
						rq_master->q_mp_attr.handled_by);
				}
			}
		}
	}

	/* Unmap interrupt bit if corresponding ring disable */
	pci_master->vec_data[0].int_ena_mask &= (~int_mask_unmap_master);
	pci_master->int_mask_map[0].int_ena_mask &= (~int_mask_unmap_master);
	pci_slave->vec_data[0].int_ena_mask &= (~int_mask_unmap_slave);
	pci_slave->int_mask_map[0].int_ena_mask &= (~int_mask_unmap_slave);

	return 0;
}

static int
pci_get_rro_mode(void *hw)
{
	struct pci_trans *trans = to_pci_trans(hw);

	return rro_get_mode(trans);
}

static bool
pci_init_rro_addr_elem_by_seid(void *hw, u16 seid)
{
	struct pci_trans *trans = to_pci_trans(hw);

	return init_rro_addr_elem_by_seid(trans, seid);
}


static int
pci_set_pao_sta_info(void *hw, u16 wcid,
		u8 max_amsdu_nums, u32 max_amsdu_len, int remove_vlan, int hdrt_mode)
{
	return 0;
}

static int
pci_set_pn_check(void *hw, u32 se_id, bool enable)
{
	return 0;
}

static int
pci_dma_get_device(void *hw, void **device)
{
	struct pci_trans *trans = to_pci_trans(hw);

	*device = trans->pdev;
	return 0;
}

static int
pci_dma_get_tx_token_num(void *hw, u16 tx_token_num[], u8 max_src_num)
{
	struct pci_trans *trans = to_pci_trans(hw);

	return pci_chip_get_tx_token_num(trans, tx_token_num, max_src_num);
}

struct mtk_bus_io_ops pci_mmio_ops = {
	.rr = pci_io_read,
	.wr = pci_io_write,
	.rr_dbg = pci_io_read,
	.wr_dbg = pci_io_write,
	.wr_range = pci_io_write_range,
	.rr_range = pci_io_read_range,
};

struct mtk_bus_dma_ops pci_dma_ops = {
	.alloc_resource = pci_dma_alloc_resource,
	.free_resource = pci_dma_free_resource,
	.preinit_device = pci_dma_preinit_device,
	.init_device = pci_dma_init_device,
	.exit_device = pci_dma_exit_device,
	.init_after_fwdl = pci_dma_init_after_fwdl,
	.tx_mcu_queue = pci_dma_tx_mcu_queue,
	.request_tx = pci_dma_request_tx,
	.tx_data_queue = pci_dma_tx_data_queue,
	.tx_kick = pci_dma_tx_kick,
	.chip_attached = pci_dma_chip_attached,
	.match = pci_dma_match,
	.queues_read = pci_queues_read,
	.tx_free = pci_dma_tx_free,
	.get_txq = pci_dma_get_txq,
	.irq_handler = pci_irq_handler,
	.get_rro_mode = pci_get_rro_mode,
	.start = pci_dma_start,
	.stop = pci_dma_stop,
	.int_to_mcu = pci_dma_int_to_mcu,
	.pause_trxq = pci_dma_pause_trxq,
	.init_sw_rxq = pci_dma_sw_init_rx_queue_from_bus,
	.exit_sw_rxq = pci_dma_sw_exit_rx_queue_from_bus,
	.switch_node = pci_dma_switch_node,
	.set_queue_ops = pci_dma_set_queue_ops,
	.set_free_notify_version = pci_dma_set_free_notify_ver,
	.init_rro_addr_elem_by_seid = pci_init_rro_addr_elem_by_seid,
	.set_pao_sta_info = pci_set_pao_sta_info,
	.set_pn_check = pci_set_pn_check,
	.get_device = pci_dma_get_device,
	.get_tx_token_num = pci_dma_get_tx_token_num,
};

struct pci_rxq_ops data_rxq_ops = {
	.get_data = pci_dma_get_data_buf,
	.process = pci_dma_rx_process,
	.init = pci_dma_init_rx_data_queue,
	.exit = pci_dma_exit_rx_data_queue,
};

struct pci_rxq_ops data_cp_rxq_ops = {
	.get_data = pci_dma_get_data_buf_by_copy,
	.process = pci_dma_rx_process,
	.init = pci_dma_init_rx_data_queue,
	.exit = pci_dma_exit_rx_data_queue,
};

struct pci_rxq_ops evt_rxq_ops = {
	.get_data = pci_dma_get_event_buf,
	.process = pci_dma_rx_process,
	.init = pci_dma_init_rx_event_queue,
	.exit = pci_dma_exit_rx_event_queue,
};
