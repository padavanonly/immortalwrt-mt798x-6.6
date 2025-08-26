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
#include <linux/delay.h>
#include "mac_ops.h"
#include "bus.h"
#include "mtk_rro.h"

static void rro_init_rxdmad_c_queue(
	struct pci_trans *trans,
	struct pci_rx_queue *rq)
{
	struct pci_queue *q = &rq->q;
	struct pci_dma_buf *desc = &q->desc_ring;
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	struct rro_cidx_didx_emi *cidx = rro_cfg->cidx_va;
	u8 *desc_va;
	struct pci_dma_cb *cb;
	struct pci_dma_buf *buf;
	u16 q_sz = q->q_size;
	dma_addr_t desc_pa;
	int i;
	struct rxdmad_c *dmad;

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
		buf = &cb->dma_buf;
		buf->alloc_size = 0;

		dmad = (struct rxdmad_c *) desc_va;
		dmad->magic_cnt = MAX_RXDNAD_C_MAGIC_CNT - 1;

		desc_va = (unsigned char *) desc_va + q->desc_size;
		desc_pa += q->desc_size;
	}
	q->head = q->q_size - 1;
	q->tail = q->q_size;

	if (!test_bit(PCI_FLAG_WHNAT_RX, &trans->flags))
		set_bit(Q_FLAG_WRITE_EMI, &q->q_flags);
	q->emi_cidx_addr = &cidx->ring[q->emi_ring_idx].idx;
}

static int rro_refill_data_buf(struct pci_trans *trans,
	struct pci_queue *q, u32 idx, struct mtk_bus_rx_info *rx_info)
{
	struct pci_dma_cb *cb = &q->cb[idx];
	struct pci_dma_buf *dma_buf = &cb->dma_buf;
	struct pci_rx_queue *rq = (struct pci_rx_queue *) q;
	int id = -ENOMEM;
#ifdef CONFIG_RRO_PRELINK
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct hw_rro_cfg *rro_cfg = to_pci_trans(dev->bus_trans)->rro_cfg;
	struct pci_dma_desc *desc = cb->alloc_va;
#ifdef CONFIG_HWIFI_DBG
	struct hw_rro_dbg *rro_dbg = &rro_cfg->rro_dbg;
#endif
	int ret = 0;
	u32 next_id;
	u32 cur_id;
	u8 status = 0;

	if (trans->rro_mode == HW_RRO_V3_0_PRE_LINK) {
		cur_id = GET_FIELD(RXDMAD_TOKEN_ID, desc->dw2);

		spin_lock(&rro_cfg->lock);
		deq_id = rro_cfg->fp_head_sid;
#ifdef CONFIG_HWIFI_DBG
		rro_dbg->refill_cnt++;
#endif
		mtk_rx_tk_entry_get_state(to_rx_tk_mgmt(trans), deq_id, &status);

		if (status != TOKEN_STATE_IN_FREE_POOL)
			dev_info(to_device(trans),
				"%s() wrong fp head token_id %d status %d\n",
				__func__, deq_id, status);
		ret = mtk_rx_tk_dequeue_id_from_list(to_rx_tk_mgmt(trans), deq_id, &next_id);
		if (ret) {
#ifdef CONFIG_HWIFI_DBG
			rro_dbg->fp_full_cnt++;
#endif
			spin_unlock(&rro_cfg->lock);
			return ret;
		}
		rro_cfg->fp_head_sid = next_id;
		mtk_rx_tk_inset_next(to_rx_tk_mgmt(trans), deq_id, rq->tail_sid);
		rq->tail_sid = deq_id;
#ifdef CONFIG_HWIFI_DBG
		if (status == TOKEN_STATE_IN_FREE_POOL) {
			rro_dbg->rro_token_used++;

			if (rro_dbg->rro_token_used > rro_dbg->max_rro_token_used)
				rro_dbg->max_rro_token_used = rro_dbg->rro_token_used;
		}
#endif
		spin_unlock(&rro_cfg->lock);
	}
#endif

	/*rx refill*/
	dma_buf->alloc_va = NULL;
	cb->pkt = pci_dma_rx_pkt_alloc(to_device(trans),
		(struct pci_rx_queue *)q, dma_buf, true);
	if (cb->pkt == NULL)
		return -ENOMEM;

	if (trans->rro_mode != HW_RRO_V3_0_PRE_LINK)
		id = mtk_rx_tk_request_entry(to_rx_tk_mgmt(trans),
				cb->pkt, dma_buf->alloc_size, dma_buf->alloc_va,
				dma_buf->alloc_pa);

	if (id < 0) {
		dma_unmap_single(to_device(trans), cb->dma_buf.alloc_pa,
			cb->dma_buf.alloc_size, DMA_FROM_DEVICE);
		put_page(virt_to_head_page(cb->pkt));
		return -ENOMEM;
	}

#ifdef CONFIG_RRO_PRELINK
	/*update to token manager*/
	mtk_rx_tk_entry_update(to_rx_tk_mgmt(trans), deq_id,
	cb->pkt, dma_buf->alloc_size,
	dma_buf->alloc_va, dma_buf->alloc_pa, NULL);
	mtk_rx_tk_entry_update_state(to_rx_tk_mgmt(trans),
		deq_id, TOKEN_STATE_IN_RRO);
#endif

	/* Update magic_cnt if ring is used a round */
	if (idx == 0)
		rq->magic_cnt = (rq->magic_cnt + 1) % MAX_RXDMAD_MAGIC_CNT;

	clear_ddone(cb->alloc_va);
	fill_rxd(cb, id, rq->magic_cnt);

	return 0;
}


static void rro_init_ind_cmd_queue(
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
	struct ind_cmd *cmd;

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
		buf = &cb->dma_buf;
		buf->alloc_size = 0;

		cmd = (struct ind_cmd *) desc_va;
		cmd->magic_cnt = MAX_IND_CMD_MAGIC_CNT - 1;

		desc_va = (unsigned char *) desc_va + q->desc_size;
		desc_pa += q->desc_size;
	}
	q->head = q->q_size - 1;
	q->tail = q->q_size;
}

static int
ind_cmd_dequeue(struct pci_trans *trans,
	struct pci_queue *q, struct ind_cmd **cmd)
{
	u32 idx = (q->head + 1) % q->q_size;
	struct pci_dma_cb *cb = &q->cb[idx];
	struct pci_rx_queue *rq = (struct pci_rx_queue *)q;

	*cmd = cb->alloc_va;

	if ((*cmd)->magic_cnt != rq->magic_cnt) {
		mtk_dbg(MTK_RRO, "%s() magic_cnt %d %d\n",
			__func__, (*cmd)->magic_cnt, rq->magic_cnt);
		return -EINVAL;
	}

	q->head = idx;

	if (q->head == q->q_size - 1)
		rq->magic_cnt =
			(rq->magic_cnt + 1) % MAX_IND_CMD_MAGIC_CNT;

	mtk_dbg(MTK_RRO, "%s() idx %d, magic_cnt = %d, q->q_size %d\n",
		__func__, idx, rq->magic_cnt, q->q_size);

	/*update rx cidx*/
	if (trans->rro_mode != HW_RRO_V3_0_BUF_PG_DBG)
		pci_dma_kick_queue(trans, q);

	return 0;
}

struct addr_elem *get_addr_elem(
	struct pci_trans *trans,
	struct hw_rro_cfg *rro_cfg,
	u16 seid, u16 start_sn)
{
	u32 index;
	void *addr_elem;

	if (seid == rro_cfg->particular_se_id) {
		addr_elem = rro_cfg->particular_session_va;
		index = start_sn % rro_cfg->win_sz;
	} else {
		addr_elem = rro_cfg->addr_elem_alloc_va[seid / HW_RRO_SESSION_CNT_PER_CR];
		index = (seid % HW_RRO_SESSION_CNT_PER_CR) * rro_cfg->win_sz
			+ (start_sn % rro_cfg->win_sz);
	}
	return addr_elem + index * sizeof(struct addr_elem);
}
EXPORT_SYMBOL(get_addr_elem);

static int rro_update_ack_sn(
	struct pci_trans *trans,
	u16 seid, u16 sn)
{
	u32 value;
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
#ifdef CONFIG_HWIFI_DBG
	struct hw_rro_dbg *rro_dbg = &rro_cfg->rro_dbg;
#endif

	value = ((seid << rro_cfg->ack_sn_seid_shift) & rro_cfg->ack_sn_seid_mask) |
		((sn << rro_cfg->ack_sn_shift) & rro_cfg->ack_sn_mask);
	bus_write(to_bus_trans(trans), rro_cfg->ack_sn_addr, value);


	mtk_dbg(MTK_RRO, "%s() seid = %d, sn = %d, value = %x\n",
		__func__, seid, sn, value);
#ifdef CONFIG_HWIFI_DBG
	rro_dbg->last_ack_se_id = seid;
	rro_dbg->last_ack_sn = sn;
#endif
	return 0;
}

static u32 msdu_page_hash(dma_addr_t pa)
{
	u32 sum = 0;
	u16 i = 0;

	while (pa != 0) {
		sum += (u32) ((pa & 0xff) + i) % MSDU_PAGE_HASH_SIZE;
		pa >>= 8;
		i += 13;
	}

	return sum % MSDU_PAGE_HASH_SIZE;
}

static struct msdu_page_addr *rro_msdu_page_hash_search(
	struct pci_trans *trans,
	dma_addr_t pa)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	struct msdu_page_addr *pg_addr = NULL;
	struct list_head *p;
	u32 hash_idx =  msdu_page_hash(pa);
	u8 found = 0;

	spin_lock(&rro_cfg->lock);
	list_for_each(p, &rro_cfg->pg_hash_head[hash_idx]) {
		pg_addr = list_entry(p, struct msdu_page_addr, list);
		if (pg_addr->pa == pa) {
			list_del(&pg_addr->list);
			found = 1;
			break;
		}
	}
	spin_unlock(&rro_cfg->lock);

	return (found == 1) ? pg_addr : NULL;
}

static void*
rro_msdu_page_chk_owner(struct pci_trans *trans,
	struct msdu_page_addr *pg_addr, u16 idx)
{
	u32 read_cnt = 0;
	struct msdu_info_page *pg;

	do {
		dma_sync_single_for_cpu(
			to_device(trans),
			pg_addr->pa,
			sizeof(struct msdu_info_page),
			DMA_FROM_DEVICE);
		pg = (struct msdu_info_page *)
			pg_addr->va;

		/* read pg before checking owner */
		rmb();
		read_cnt++;

		if (pg->owner)
			udelay(1);
		else
			break;
	} while (read_cnt < 10);

	dma_unmap_single(to_device(trans), pg_addr->pa,
		sizeof(struct msdu_info_page),
		DMA_FROM_DEVICE);

	if (read_cnt > 1) {
#ifdef CONFIG_HWIFI_DBG
		struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
		struct hw_rro_dbg *rro_dbg = &rro_cfg->rro_dbg;

		rro_dbg->addition_read_cnt++;
		if (read_cnt > rro_dbg->max_read_cnt)
			rro_dbg->max_read_cnt = read_cnt;
#endif
		mtk_dbg(MTK_RRO_ERR, "%s(): msdu_pg[%u] Read more than once\n",
			__func__, idx);
	}

	return pg;
}

#ifdef CONFIG_RRO_PRELINK
static int rro_ind_cmd_process_prelink(struct pci_trans *trans,
	struct pci_queue *q, int budget)
{
	struct sk_buff *skb = NULL;
	struct pci_rx_queue *rq = (struct pci_rx_queue *)q;
	struct mtk_bus_rx_info rx_info = {0};
	struct mtk_bus_rx_info *info;
	int done = 0;
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	struct ind_cmd *cmd;
	struct addr_elem *elem;
	u32 token_id;
	u32 next_id;
	u16 i;
	u16 j;
	u32 sn;
	u8 status = 0;
#ifdef CONFIG_HWIFI_DBG
	struct hw_rro_dbg *rro_dbg = &rro_cfg->rro_dbg;
#endif

	while (done < budget) {
		if (ind_cmd_dequeue(trans, q, &cmd))
			break;

		mtk_dbg(MTK_RRO, "%s() seid = %d, ssn = %d, ind_cnt = %d\n",
			__func__, cmd->se_id, cmd->start_sn, cmd->ind_cnt);

#ifdef CONFIG_HWIFI_DBG
		rro_dbg->last_se_id = cmd->se_id;
		rro_dbg->last_ind_cnt = cmd->ind_cnt;
		rro_dbg->last_start_sn = cmd->start_sn;
		rro_dbg->last_ind_reason = cmd->ind_reason;
#endif
		for (i = 0; i < cmd->ind_cnt; i++) {
			sn = (cmd->start_sn + i) & RRO_MAX_SEQ;
			elem = get_addr_elem(trans, rro_cfg, cmd->se_id, sn);
			if (elem->signature != (sn / rro_cfg->win_sz)) {
				mtk_dbg(MTK_RRO,
				"%s() signature fail(%d!=%d), go next (sn, idx) = (%d %d)\n",
				__func__, elem->signature,
				(sn / rro_cfg->win_sz), sn, i);

				elem->signature = 0xff;
				goto update_ack_sn;
			}
			token_id = elem->head_pkt_addr_info_l;
#ifdef CONFIG_HWIFI_DBG
			rro_dbg->last_sn = sn;
			rro_dbg->last_seg_cnt = elem->seg_cnt;
#endif

			mtk_dbg(MTK_RRO,
				"%s() seg_cnt = %d, tkid = %d, signature = %d(expected: %d)\n",
				__func__, elem->seg_cnt, token_id,
				elem->signature, (sn / rro_cfg->win_sz));

			for (j = 0; j < elem->seg_cnt; j++) {
				memset(&rx_info, 0, sizeof(rx_info));
				spin_lock(&rro_cfg->lock);
				mtk_rx_tk_entry_get_state(
					to_rx_tk_mgmt(trans),
					token_id, &status);
				if (status != TOKEN_STATE_IN_RRO) {
					spin_unlock(&rro_cfg->lock);
					dev_err(to_device(trans),
						"%s() wrong token_id %d status %d\n",
						__func__, token_id, status);
					goto update_ack_sn;
				}
				mtk_rx_tk_dequeue_id_from_list(
					to_rx_tk_mgmt(trans),
					token_id, &next_id);
				mtk_rx_tk_inset_prev(to_rx_tk_mgmt(trans),
					token_id, rro_cfg->fp_head_sid);
				mtk_rx_tk_entry_update_state(
					to_rx_tk_mgmt(trans),
					token_id, TOKEN_STATE_IN_FREE_POOL);
				spin_unlock(&rro_cfg->lock);
				rx_info.pkt = mtk_rx_tk_release_entry(
					to_rx_tk_mgmt(trans), token_id);

				mtk_dbg(MTK_RRO,
				"%s() token_id = %d, next_id = %d, fp_head_sid %d\n",
				__func__, token_id,
				next_id, rro_cfg->fp_head_sid);
#ifdef CONFIG_HWIFI_DBG
				rro_dbg->last_tkid = token_id;
#endif
				token_id = next_id;

				if (!rx_info.pkt)
					continue;

				skb = build_skb(rx_info.pkt,
					rx_pkt_total_size(rq->rx_buf_size));

				skb_reserve(skb, SKB_HEADROOM_RSV);

				__skb_put(skb, rx_info.len);
				rx_info.skb = skb;

				if (cmd->ind_reason == 1
					|| cmd->ind_reason == 2) {
					rx_info.drop = 1;
					info =
					(struct mtk_bus_rx_info *)skb->cb;
					*info = rx_info;
				}

				mtk_bus_rx_process(
					(struct mtk_bus_trans *)trans,
					q->q_attr, skb);
			}
update_ack_sn:
			if ((i + 1) % rro_cfg->ack_sn_update_cnt == 0)
				rro_update_ack_sn(trans, cmd->se_id, sn);
		}

		/* update ack_sn for remaining addr_elem */
		if (i % rro_cfg->ack_sn_update_cnt != 0)
			rro_update_ack_sn(trans, cmd->se_id, sn);

		done++;
	}

	mtk_bus_rx_poll((struct mtk_bus_trans *)trans, &q->napi, ffs(q->band_idx_bmp) - 1);

	return done;
}
#endif
#ifdef CONFIG_HWIFI_DBG
static void msdu_page_dump(enum mtk_debug_mask dbg_mask,
	struct hif_rxd *rxd, u8 index)
{
	mtk_dbg(dbg_mask, "hif rxd[%u]->rx_blk_base_l = 0x%x\n",
		index, rxd->rx_blk_base_l);
	mtk_dbg(dbg_mask, "hif rxd[%u]->rx_blk_base_h = 0x%x\n",
		index, rxd->rx_blk_base_h);
	mtk_dbg(dbg_mask, "hif rxd[%u]->eth_hdr_ofst = 0x%x\n",
		index, rxd->eth_hdr_ofst);
	mtk_dbg(dbg_mask, "hif rxd[%u]->rsv = 0x%x\n", index, rxd->rsv);
	mtk_dbg(dbg_mask, "hif rxd[%u]->ring_no = 0x%x\n", index, rxd->ring_no);
	mtk_dbg(dbg_mask, "hif rxd[%u]->dst_sel = 0x%x\n", index, rxd->dst_sel);
	mtk_dbg(dbg_mask, "hif rxd[%u]->sdl = 0x%x\n", index, rxd->sdl);
	mtk_dbg(dbg_mask, "hif rxd[%u]->ls = 0x%x\n", index, rxd->ls);
	mtk_dbg(dbg_mask, "hif rxd[%u]->rsv2 = 0x%x\n", index, rxd->rsv2);
	mtk_dbg(dbg_mask, "hif rxd[%u]->pn_31_0 = 0x%x\n", index, rxd->pn_31_0);
	mtk_dbg(dbg_mask, "hif rxd[%u]->pn_47_32 = 0x%x\n",
		index, rxd->pn_47_32);
	mtk_dbg(dbg_mask, "hif rxd[%u]->cs_status = 0x%x\n",
		index, rxd->cs_status);
	mtk_dbg(dbg_mask, "hif rxd[%u]->cs_type = 0x%x\n", index, rxd->cs_type);
	mtk_dbg(dbg_mask, "hif rxd[%u]->c = 0x%x\n", index, rxd->c);
	mtk_dbg(dbg_mask, "hif rxd[%u]->f = 0x%x\n", index, rxd->f);
	mtk_dbg(dbg_mask, "hif rxd[%u]->un = 0x%x\n", index, rxd->un);
	mtk_dbg(dbg_mask, "hif rxd[%u]->rsv3 = 0x%x\n", index, rxd->rsv3);
	mtk_dbg(dbg_mask, "hif rxd[%u]->is_fc_data = 0x%x\n",
		index, rxd->is_fc_data);
	mtk_dbg(dbg_mask, "hif rxd[%u]->uc = 0x%x\n", index, rxd->uc);
	mtk_dbg(dbg_mask, "hif rxd[%u]->mc = 0x%x\n", index, rxd->mc);
	mtk_dbg(dbg_mask, "hif rxd[%u]->bc = 0x%x\n", index, rxd->bc);
	mtk_dbg(dbg_mask, "hif rxd[%u]->rx_token_id = 0x%x\n",
		index, rxd->rx_token_id);
	mtk_dbg(dbg_mask, "hif rxd[%u]->rsv4 = 0x%x\n", index, rxd->rsv4);
	mtk_dbg(dbg_mask, "hif rxd[%u]->rsv5 = 0x%x\n", index, rxd->rsv5);
}

static int rro_addr_elem_dbg_memcpy(struct pci_trans *trans,
	struct pci_queue *q, struct ind_cmd *cmd, u32 sign_ok, u32 sign_fail)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	struct addr_elem *elem_copy;
	u32 sign_fail_bef, sign_ok_bef, total_sign_bef;
	u32 sign_fail_aft, sign_ok_aft, total_sign_aft;
	u32 acksn_bef, acksn_aft, cr_state_machine;
	u32 cidx_to_didx, ack_sn, read_cnt, delay;
	struct pci_rx_queue *rq = (struct pci_rx_queue *)q;
	struct hw_rro_dbg *rro_dbg = &rro_cfg->rro_dbg;

	read_cnt = 0, delay = 0;
	elem_copy = (struct addr_elem *) rro_cfg->addr_elem_cp_va;

	memcpy(rro_cfg->addr_elem_cp_va,
	       (void *) get_addr_elem(trans, rro_cfg, cmd->se_id, 0),
	       rro_cfg->win_sz * sizeof(struct addr_elem));

	/* Make sure addr_elem are copy */
	wmb();

	/* Total addr_elem counter by warp */
	total_sign_bef = bus_read(to_bus_trans(trans), 0xD430C);
	/* Sign fail counter by warp */
	sign_fail_bef = bus_read(to_bus_trans(trans), 0xD4310);
	sign_fail_bef &= 0xffff;
	/* PG_CNT = 1 (Sign OK) counter by warp */
	sign_ok_bef = bus_read(to_bus_trans(trans), 0xD4314);
	sign_ok_bef &= 0xfffffff;


	/* ACKSN_LAST_CNT by warp */
	acksn_bef = bus_read(to_bus_trans(trans), 0xD4300);
	acksn_bef = (acksn_bef >> 16) & 0xfff;
	cidx_to_didx = ((q->head + 1) % q->q_size) |
		       ((rq->magic_cnt & 0x7) << 28) |
		       (1 << 31);

	mtk_bus_dma_rro_write_dma_idx(to_bus_trans(trans),
				      cidx_to_didx);

	do {
		/* ACKSN_LAST_CNT by warp */
		acksn_aft = bus_read(to_bus_trans(trans), 0xD4300);
		acksn_aft = (acksn_aft >> 16) & 0xfff;
		/* WED_RRO state machine by warp */
		cr_state_machine = bus_read(to_bus_trans(trans),
					    0xD4318);
		cr_state_machine &= 0x7;
		read_cnt++;
		delay = delay < 50 ? read_cnt * 10 : delay;

		if (acksn_bef == acksn_aft || cr_state_machine) {
			udelay(delay);
			continue;
		}

		/* WED_RRO state machine by warp */
		cr_state_machine = bus_read(to_bus_trans(trans),
					    0xD4318);
		cr_state_machine &= 0x7;

		if (cr_state_machine)
			udelay(delay);
		else
			break;
	} while (read_cnt < 10);

	if (read_cnt >= 10) {
		mtk_dbg(MTK_RRO_ERR, "%s(): WED_RRO too slow\n",
			__func__);
	}

	/* Total addr_elem counter by warp */
	total_sign_aft = bus_read(to_bus_trans(trans), 0xD430C);
	total_sign_aft -= total_sign_bef;
	/* Sign fail counter by warp */
	sign_fail_aft = bus_read(to_bus_trans(trans), 0xD4310);
	sign_fail_aft &= 0xffff;
	sign_fail_aft = (sign_fail_aft - sign_fail_bef) & 0xffff;
	/* PG_CNT = 1 (Sign OK) counter by warp */
	sign_ok_aft = bus_read(to_bus_trans(trans), 0xD4314);
	sign_ok_aft &= 0xfffffff;
	sign_fail_aft = (sign_ok_aft - sign_ok_bef) & 0xfffffff;

	if (sign_fail != sign_fail_aft ||
	    sign_ok != sign_ok_aft) {
		rro_dbg->stop_rx = 1;

		mtk_dbg(MTK_RRO_ERR, "%s(): SW sign_fail = %u, sign_ok = %u\n",
			__func__, sign_fail, sign_ok);
		mtk_dbg(MTK_RRO_ERR, "%s(): HW sign_fail = %u, pg_cnt1 (sign_ok) = %u, total_sign_aft = %u\n",
			__func__, sign_fail_aft, sign_ok_aft,
			total_sign_aft);
		mtk_dbg(MTK_RRO_ERR, "%s(): start_sn = %u, ind_cnt = %u, ind_reason = %u, se_id = %u\n",
			__func__, cmd->start_sn, cmd->ind_cnt,
			cmd->ind_reason, cmd->se_id);
		mtk_dbg(MTK_RRO_ERR, "%s(): q->head = 0x%x\n",
			__func__, q->head);
		mtk_dbg(MTK_RRO_ERR, "%s(): allocate PA for debug = 0x%llX\n",
			__func__, rro_cfg->addr_elem_cp_pa);
		return -EINVAL;
	}

	/* Unused CR in RRO */
	ack_sn = bus_read(to_bus_trans(trans), 0xDA130);
	bus_write(to_bus_trans(trans), rro_cfg->ack_sn_addr, ack_sn);

	return 0;
}

static void
rro_dump_raw(char *str, void *va, u32 size)
{
	pr_debug("%s: data len = %u\n", str, size);
	print_hex_dump(KERN_DEBUG, "\t", DUMP_PREFIX_OFFSET, 16, 1, va, size, 0);
}

static int rro_ind_cmd_process_buffer_page_dbg(struct pci_trans *trans,
	struct pci_queue *q, int budget)
{
	int done = 0;
	int ret = 0;
	u32 sn, sign_fail, sign_ok, i, ind_cnt;
	u32 token_id, cidx_to_didx, pg_num;
	u32 seg_idx, rxd_idx;
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	struct pci_rx_queue *rq = (struct pci_rx_queue *)q;
	struct ind_cmd *cmd;
	struct addr_elem *elem;
	struct msdu_info_page *pg, *last_err_pg;
	struct mtk_bus_rx_info rx_info = {0};
	struct hif_rxd *rxd;
	struct hw_rro_dbg *rro_dbg = &rro_cfg->rro_dbg;
	void *va;
	dma_addr_t pa, next_pa;

	if (unlikely(rro_dbg->stop_rx))
		return budget;

	while (done < budget) {
		if (ind_cmd_dequeue(trans, q, &cmd))
			break;

		sign_fail = 0;
		sign_ok = 0;
		rro_dbg->ind_cmd_cnt += 1;
		rro_dbg->ind_cnt += cmd->ind_cnt;

		if (cmd->ind_cnt > rro_cfg->win_sz) {
			rro_dbg->larger_winsize_cnt += 1;
			mtk_dbg(MTK_RRO, "%s() ind_cnt %u > win_sz %u\n",
				__func__, cmd->ind_cnt, rro_cfg->win_sz);
		}

		ind_cnt = cmd->ind_cnt < rro_cfg->win_sz ?
			  cmd->ind_cnt : rro_cfg->win_sz;

		for (i = 0; i < ind_cnt; i++) {
			pg_num = 1;
			last_err_pg = NULL;
			sn = (cmd->start_sn + i) & RRO_MAX_SEQ;
			elem = get_addr_elem(trans, rro_cfg, cmd->se_id, sn);

			if (elem->signature != (sn / rro_cfg->win_sz)) {
				sign_fail += 1;
				continue;
			}

			sign_ok += 1;
			rro_dbg->total_msdu_cnt += elem->seg_cnt;

			if (elem->seg_cnt > rro_dbg->max_msdu_num)
				rro_dbg->max_msdu_num = elem->seg_cnt;

			if (elem->seg_cnt < 1 || elem->seg_cnt > MAX_MSDU_CNT_ARR_SIZE)
				rro_dbg->invalid_msdu_cnt++;
			else
				rro_dbg->msdu_cnt[elem->seg_cnt - 1]++;

			pa = elem->head_pkt_addr_info_h;
			pa <<= DMA_ADDR_H_SHIFT;
			pa |= elem->head_pkt_addr_info_l;

			for (seg_idx = 0; seg_idx < elem->seg_cnt; seg_idx++) {
				memset(&rx_info, 0, sizeof(rx_info));
				next_pa = 0;

				dma_sync_single_for_cpu(
					to_device(trans),
					pa,
					sizeof(struct msdu_info_page),
					DMA_FROM_DEVICE);

				va = mtk_bus_dma_rro_page_hash_get(
					(struct mtk_bus_trans *)trans,
					pa);

				pg = (struct msdu_info_page *)va;

				if (pg == NULL) {
					dev_err(to_device(trans), "%s() pg_addr %pad search fail for seg_idx = %u\n",
						__func__, &pa, seg_idx);
					continue;
				}

				rxd_idx = seg_idx % MAX_HIF_RXD_CND_IN_PG;
				rxd = &pg->rxd[rxd_idx];

				rx_info.more = !rxd->ls;
				rx_info.len = rxd->sdl;
				token_id = rxd->rx_token_id;
				rro_dbg->last_tkid = token_id;

				if (rxd->rx_blk_base_l == 0 || rxd->sdl == 0) {
					pr_err("<----- START DUMP  ----->\n");
					pr_err("cmd: seid = %u, start_sn = %u, ind_cnt = %u, ind_reason = %u\n",
						cmd->se_id, cmd->start_sn,
						cmd->ind_cnt, cmd->ind_reason);
					pr_err("elem: seg_cnt = %u, sign = %u, sn = %u, winsz = %u, seg_idx = %u\n",
						elem->seg_cnt, elem->signature,
						sn, rro_cfg->win_sz, seg_idx);
					pr_err("msdu_pg: pa = %pad, rxd_idx = %u\n",
						&pa, rxd_idx);
					pr_err("rxd: rx_blk_base = 0x%x, token_id = %u, ls = %u, sdl = %u, pn_31_0 = 0x%x\n",
						rxd->rx_blk_base_l, rxd->rx_token_id,
						rxd->ls, rxd->sdl, rxd->pn_31_0);

					if (last_err_pg != pg) {
						rro_dump_raw("msdu_pg_raw", pg, sizeof(*pg));
						last_err_pg = pg;
					}

					rro_dump_raw("rxd_raw", rxd, sizeof(*rxd));

					pr_err("<----- END DUMP  ----->\n");
				}

				if ((seg_idx + 1) % MAX_HIF_RXD_CND_IN_PG == 0) {
					next_pa = pg->next_pg_h;
					next_pa <<= DMA_ADDR_H_SHIFT;
					next_pa |= pg->next_pg_l;

					if (next_pa)
						pg_num++;
				}

				dma_sync_single_for_device(to_device(trans), pa,
					sizeof(struct msdu_info_page),
					DMA_FROM_DEVICE);

				pa = next_pa ? next_pa : pa;
			}

			if (pg_num > rro_dbg->max_pg_num)
				rro_dbg->max_pg_num = pg_num;
		}

		rro_dbg->sign_ok_cnt += sign_ok;
		rro_dbg->sign_fail_cnt += sign_fail;

		if (test_bit(PCI_FLAG_WHNAT_RRO_DBG_MEMCPY, &trans->flags)) {
			ret = rro_addr_elem_dbg_memcpy(trans, q,
					cmd, sign_ok, sign_fail);
			if (ret)
				break;
		}

		done++;
	}

	if (test_bit(PCI_FLAG_WHNAT_RRO_DBG, &trans->flags)) {
		cidx_to_didx = ((q->head + 1) % q->q_size) |
			       ((rq->magic_cnt & 0x7) << 28) |
			       (1 << 31);

		mtk_bus_dma_rro_write_dma_idx((struct mtk_bus_trans *)trans,
					      cidx_to_didx);
	}

	return done;
}

static int
rroinfo_debugfs(struct seq_file *s, void *data)
{
	struct pci_trans *trans = to_pci_trans(dev_get_drvdata(s->private));
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	struct hw_rro_dbg *rro_dbg = &rro_cfg->rro_dbg;
	int i;

	seq_puts(s, "===================RRO counter==============\n");
	seq_printf(s, "rro_token_used = %d\n", rro_dbg->rro_token_used);
	seq_printf(s, "max_rro_token_used = %d (read clear)\n",
		rro_dbg->max_rro_token_used);
	rro_dbg->max_rro_token_used = 0;

	seq_printf(s, "seg_cnt = %u\n", rro_dbg->seg_cnt);
	seq_printf(s, "drop_cnt = %u\n", rro_dbg->drop_cnt);
	seq_printf(s, "err_cnt = %u\n", rro_dbg->err_cnt);
	seq_printf(s, "refill_cnt = %u\n", rro_dbg->refill_cnt);
	seq_printf(s, "fp_full_cnt = %u\n", rro_dbg->fp_full_cnt);
	seq_printf(s, "fp_tkid_start = %u\n", rro_dbg->fp_tkid_start);
	seq_printf(s, "fp_tkid_end = %u\n", rro_dbg->fp_tkid_end);
	if (rro_dbg->addition_read_cnt) {
		seq_printf(s, "addition_read_cnt = %u\n",
			rro_dbg->addition_read_cnt);
		seq_printf(s, "max_read_cnt = %u\n", rro_dbg->max_read_cnt);
	}
	seq_printf(s, "last_se_id = %u\n", rro_dbg->last_se_id);
	seq_printf(s, "last_start_sn = %u\n", rro_dbg->last_start_sn);
	seq_printf(s, "last_ind_cnt = %u\n", rro_dbg->last_ind_cnt);
	seq_printf(s, "last_ind_reason = %u\n", rro_dbg->last_ind_reason);
	seq_printf(s, "last_sn = %u\n", rro_dbg->last_sn);
	seq_printf(s, "last_seg_cnt = %u\n", rro_dbg->last_seg_cnt);
	seq_printf(s, "last_tkid = %u\n", rro_dbg->last_tkid);
	seq_printf(s, "last_ack_sn = %u\n", rro_dbg->last_ack_sn);
	seq_printf(s, "last_ack_se_id = %u\n", rro_dbg->last_ack_se_id);

	seq_puts(s, "=================RRO_DBG counter============\n");
	seq_printf(s, "ind_cmd_cnt: %u\n", rro_dbg->ind_cmd_cnt);
	seq_printf(s, "ind_cnt: %u\n", rro_dbg->ind_cnt);
	seq_printf(s, "sign_ok_cnt: %u\n", rro_dbg->sign_ok_cnt);
	seq_printf(s, "sign_fail_cnt: %u\n", rro_dbg->sign_fail_cnt);
	seq_printf(s, "larger_winsize_cnt: %u\n", rro_dbg->larger_winsize_cnt);
	seq_printf(s, "invalid_msdu_cnt: %u\n", rro_dbg->invalid_msdu_cnt);
	seq_printf(s, "total_msdu_cnt: %u\n", rro_dbg->total_msdu_cnt);
	seq_printf(s, "max_msdu_num: %u\n", rro_dbg->max_msdu_num);
	seq_printf(s, "max_pg_num: %u\n", rro_dbg->max_pg_num);

	for (i = 0; i < MAX_MSDU_CNT_ARR_SIZE; i++) {
		seq_printf(s, "msdu%02d_cnt = 0x%8x", i + 1, rro_dbg->msdu_cnt[i]);

		if ((i + 1) % 4 == 0 || (i + 1) == MAX_MSDU_CNT_ARR_SIZE)
			seq_puts(s, "\n");
		else
			seq_puts(s, ", ");
	}

	return 0;
}

#endif

static int rro_ind_cmd_process_buffer_page(struct pci_trans *trans,
	struct pci_queue *q, int budget)
{
	struct sk_buff *skb = NULL;
	struct pci_rx_queue *rq = (struct pci_rx_queue *)q;
	struct mtk_bus_rx_info rx_info = {0};
	struct mtk_bus_rx_info *info;
	int done = 0;
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	struct ind_cmd *cmd;
	struct addr_elem *elem;
	dma_addr_t msdu_pg_pa;
	struct msdu_page_addr *pg_addr = NULL;
	struct msdu_info_page *pg = NULL;
	struct hif_rxd *rxd;
	u8 rxd_idx;
	u16 i;
	u16 j;
	u32 sn;
#ifdef CONFIG_HWIFI_DBG
	struct hw_rro_dbg *rro_dbg = &rro_cfg->rro_dbg;
	u32 token_id;
	/* check rx_blk_base */
	dma_addr_t pa;
	dma_addr_t pa2;
#endif

	while (done < budget) {
		if (ind_cmd_dequeue(trans, q, &cmd))
			break;

		mtk_dbg(MTK_RRO,
			"%s() seid = %d, ssn = %d, ind_cnt = %d, reason = %d\n",
			__func__, cmd->se_id,
			cmd->start_sn, cmd->ind_cnt, cmd->ind_reason);

#ifdef CONFIG_HWIFI_DBG
		rro_dbg->last_se_id = cmd->se_id;
		rro_dbg->last_start_sn = cmd->start_sn;
		rro_dbg->last_ind_cnt = cmd->ind_cnt;
		rro_dbg->last_ind_reason = cmd->ind_reason;
#endif
		for (i = 0; i < cmd->ind_cnt; i++) {
			sn = (cmd->start_sn + i) & RRO_MAX_SEQ;
			elem = get_addr_elem(trans, rro_cfg, cmd->se_id, sn);
			if (elem->signature != (sn / rro_cfg->win_sz)) {
				mtk_dbg(MTK_RRO,
				"%s() signature fail(%d!=%d), go next (sn, idx) = (%d %d)\n",
				__func__, elem->signature,
				(sn / rro_cfg->win_sz), sn, i);

				elem->signature = 0xff;
				goto update_ack_sn;
			}
#ifdef CONFIG_HWIFI_DBG
			rro_dbg->last_sn = sn;
			rro_dbg->last_seg_cnt = elem->seg_cnt;
#endif
			msdu_pg_pa = elem->head_pkt_addr_info_l;
#ifdef CONFIG_HWIFI_64BIT_SUPPORT
			msdu_pg_pa |= ((dma_addr_t)elem->head_pkt_addr_info_h << DMA_ADDR_H_SHIFT);
#endif

			mtk_dbg(MTK_RRO,
				"%s() seg_cnt = %d, msdu_pg_pa = %llx, signature = %d(expected: %d)\n",
				__func__, elem->seg_cnt, msdu_pg_pa,
				elem->signature, (sn / rro_cfg->win_sz));

			for (j = 0; j < elem->seg_cnt; j++) {
				memset(&rx_info, 0, sizeof(rx_info));

				if (pg_addr == NULL) {
					pg_addr = rro_msdu_page_hash_search(
					trans, msdu_pg_pa);

					if (pg_addr == NULL) {
						dev_info(to_device(trans),
						"%s()  pg_addr search fail, msdu_pg_pa = %pad\n",
						__func__, &msdu_pg_pa);
						continue;
					}

					pg = rro_msdu_page_chk_owner(trans, pg_addr, j);
				}

				rxd_idx = j % MAX_HIF_RXD_CND_IN_PG;

				rxd = &pg->rxd[rxd_idx];

				rx_info.more = !rxd->ls;
				rx_info.len = rxd->sdl;
				rx_info.ip_frag = rxd->f;
#ifdef CONFIG_HWIFI_DBG
				token_id = rxd->rx_token_id;
				rro_dbg->last_tkid = token_id;

				mtk_dbg(MTK_RRO,
					"%s()  msdu_pg_pa = %llx va = %p, rxd_idx = %d, rx_token_id = %d more = %d %d, len %d %d\n",
					__func__, msdu_pg_pa, pg_addr->va,
					rxd_idx, rxd->rx_token_id,
					rx_info.more, rxd->ls,
					rx_info.len, rxd->sdl);
				msdu_page_dump(MTK_MSDUPG_DUMP, rxd, j);

				rro_dbg->rro_token_used--;
				rro_dbg->seg_cnt++;

				mtk_rx_tk_entry_query(
					to_rx_tk_mgmt(trans),
					rxd->rx_token_id, &pa);

				pa2 = rxd->rx_blk_base_h;
				pa2 <<= 32;
				pa2 |= rxd->rx_blk_base_l;

				if (pa != pa2) {
					mtk_dbg(MTK_RRO_ERR,
					"[Error]%s() wrong pa, token_id = %d,  pa = %llx/ %llx\n",
					__func__, rxd->rx_token_id,
					pa, pa2);
					goto update_ack_sn;
				}

#endif
				rx_info.pkt = mtk_rx_tk_release_entry(
					to_rx_tk_mgmt(trans), rxd->rx_token_id);

				mtk_dbg(MTK_RRO, "%s() token_id = %d\n",
					__func__, rxd->rx_token_id);

				if (!rx_info.pkt)
					goto next_page_chk;

				if (rq->rx_frag_head) {
					pci_dma_gather_frag(trans, rq, &rx_info);

					skb = rq->rx_frag_head;
					info = (struct mtk_bus_rx_info *) skb->cb;
					info->len += rxd->sdl;

					if (rx_info.more)
						goto next_page_chk;

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
					goto next_page_chk;
				}

				skb_reserve(skb, SKB_HEADROOM_RSV);

				__skb_put(skb, rx_info.len);
				rx_info.skb = skb;

				if (rxd->dst_sel == 1) {
					rx_info.hw_rro = 1;
					rx_info.eth_hdr_ofst
						= rxd->eth_hdr_ofst;
				}

				if (cmd->ind_reason == IND_REASON_PN_CHK_FAIL)
					rx_info.pn_chk_fail = true;

				/* Decide drop in mac rxd parser */
				if (cmd->ind_reason == IND_REASON_REPEAT)
					rx_info.repeat_pkt = 1;
				else if (cmd->ind_reason == IND_REASON_OLDPKT)
					rx_info.old_pkt = 1;

				info = (struct mtk_bus_rx_info *)skb->cb;
				*info = rx_info;

#ifdef CONFIG_HWIFI_DBG
				if (cmd->se_id != rro_cfg->max_rro_se_cnt
					&& trans->rro_mode < HW_RRO_V3_1_BUF_PG_EMUL) {
					info->check_sn = 1;
					info->sn = sn;
					info->i = i;
					info->j = j;
				}
#endif
				if (rx_info.more) {
					rq->rx_frag_head = skb;
					goto next_page_chk;
				}
process:
				mtk_bus_rx_process(
					(struct mtk_bus_trans *)trans,
					q->q_attr, skb);
next_page_chk:
				if ((j + 1) % MAX_HIF_RXD_CND_IN_PG == 0) {
					msdu_pg_pa = pg->next_pg_l;
#ifdef CONFIG_HWIFI_64BIT_SUPPORT
					msdu_pg_pa |= ((dma_addr_t)
							pg->next_pg_h << DMA_ADDR_H_SHIFT);
#endif
					put_page(virt_to_head_page(pg_addr->pkt));
					kfree(pg_addr);
					pg_addr = NULL;
				}
			}
update_ack_sn:
			if ((i + 1) % rro_cfg->ack_sn_update_cnt == 0)
				rro_update_ack_sn(trans, cmd->se_id, sn);
			if (pg_addr) {
				put_page(virt_to_head_page(pg_addr->pkt));
				kfree(pg_addr);
				pg_addr = NULL;
			}
		}

		/* update ack_sn for remaining addr_elem */
		if (i % rro_cfg->ack_sn_update_cnt != 0)
			rro_update_ack_sn(trans, cmd->se_id, sn);

		done++;
	}

	mtk_bus_rx_poll((struct mtk_bus_trans *)trans, &q->napi, ffs(q->band_idx_bmp) - 1);

	return done;
}

static int rro_ind_cmd_process(struct pci_trans *trans,
	struct pci_queue *q, int budget)
{
	if (trans->rro_mode == HW_RRO_V3_0_BUF_PG || trans->rro_mode == HW_RRO_V3_1_BUF_PG_EMUL)
		return rro_ind_cmd_process_buffer_page(trans, q, budget);
#ifdef CONFIG_HWIFI_DBG
	else if (trans->rro_mode == HW_RRO_V3_0_BUF_PG_DBG)
		return rro_ind_cmd_process_buffer_page_dbg(trans, q, budget);
#endif
	else
#ifdef CONFIG_RRO_PRELINK
		return rro_ind_cmd_process_prelink(trans, q, budget);
#else
		return -EINVAL;
#endif
}


static int
rxdmad_c_dequeue(struct pci_trans *trans,
	struct pci_queue *q, struct rxdmad_c **dmad)
{
	u32 idx = (q->head + 1) % q->q_size;
	struct pci_dma_cb *cb = &q->cb[idx];
	struct pci_rx_queue *rq = (struct pci_rx_queue *)q;

	*dmad = cb->alloc_va;

	if ((*dmad)->magic_cnt != rq->magic_cnt) {
		mtk_dbg(MTK_RRO,
			"%s() magic_cnt %d %d\n",
			__func__, (*dmad)->magic_cnt,
			rq->magic_cnt);
		return -EINVAL;
	}

	q->head = idx;

	if (q->head == q->q_size - 1)
		rq->magic_cnt =
			(rq->magic_cnt + 1) % MAX_RXDNAD_C_MAGIC_CNT;

	mtk_dbg(MTK_RRO,
			"%s() idx %d, rro_cfg->cur_rxdmadc_magic_cnt = %d, q->q_size %d\n",
			__func__, idx,
			rq->magic_cnt, q->q_size);

	/*update rx cidx*/
	*q->emi_cidx_addr = cpu_to_le16(q->head);

	return 0;
}

static int rro_rxdmad_c_process(struct pci_trans *trans,
	struct pci_queue *q, int budget)
{
	struct sk_buff *skb = NULL;
	struct pci_rx_queue *rq = (struct pci_rx_queue *)q;
	struct mtk_bus_rx_info rx_info = {0};
	struct mtk_bus_rx_info *info;
	struct rxdmad_c *dmad;
	int done = 0;
	u32 token_id;
#ifdef CONFIG_HWIFI_DBG
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	struct hw_rro_dbg *rro_dbg = &rro_cfg->rro_dbg;
#endif

	while (done < budget) {
		if (rxdmad_c_dequeue(trans, q, &dmad))
			break;

		memset(&rx_info, 0, sizeof(rx_info));
		token_id = dmad->rx_token_id;

#ifdef CONFIG_HWIFI_DBG
		mtk_dbg(MTK_RRO,
			"%s() tkid = %d, reason = %d, to_host = %d, wcid = %d, sdp0 = %x %x, len = %d, ls = %d\n",
			__func__, dmad->rx_token_id, dmad->ind_reason,
			dmad->to_host, dmad->wcid,
			dmad->sdp0_35_32, dmad->sdp0_31_0,
			dmad->sdl0, dmad->ls);
#endif
#ifdef CONFIG_HWIFI_DBG
		rro_dbg->rro_token_used--;
		rro_dbg->seg_cnt++;
#endif
		rx_info.pkt = mtk_rx_tk_release_entry(
			to_rx_tk_mgmt(trans), token_id);
		rx_info.len = dmad->sdl0;
		rx_info.more = !dmad->ls;

		if (!rx_info.pkt)
			goto next;

		if (rq->rx_frag_head) {
			pci_dma_gather_frag(trans, rq, &rx_info);

			skb = rq->rx_frag_head;
			info = (struct mtk_bus_rx_info *) skb->cb;
			info->len += dmad->sdl0;

			if (rx_info.more)
				continue;

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
			break;
		}

		skb_reserve(skb, SKB_HEADROOM_RSV);

		__skb_put(skb, rx_info.len);
		rx_info.skb = skb;
		if (dmad->dst_sel == 1) {
			rx_info.hw_rro = 1;
			rx_info.eth_hdr_ofst
				= dmad->header_ofst;
		}

		/* Decide drop in mac rxd parser */
		if (dmad->ind_reason == IND_REASON_REPEAT)
			rx_info.repeat_pkt = 1;
		else if (dmad->ind_reason == IND_REASON_OLDPKT)
			rx_info.old_pkt = 1;

		if (dmad->pn_chk_fail)
			rx_info.pn_chk_fail = true;

		info = (struct mtk_bus_rx_info *)skb->cb;
				*info = rx_info;

		if (rx_info.more) {
			rq->rx_frag_head = skb;
			continue;
		}

process:
		mtk_bus_rx_process((struct mtk_bus_trans *)trans,
						q->q_attr, skb);
next:
		done++;
	}

	mtk_bus_rx_poll((struct mtk_bus_trans *)trans, &q->napi, ffs(q->band_idx_bmp) - 1);

	return done;
}


static int rro_init_addr_elem(struct pci_trans *trans)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	struct addr_elem *ptr;
	u16 i;
	u16 j;

	rro_cfg->session_cnt =
		rro_cfg->max_rro_se_cnt + HW_RRO_PARTICULAR_SESSION_CNT;
	rro_cfg->win_sz = MAX_HW_RRO_WIN_SIZE;

	for (i = 0; i < rro_cfg->addr_elem_cr_cnt; i++) {
		rro_cfg->addr_elem_alloc_va[i] =
			dma_alloc_coherent(to_device(trans),
			HW_RRO_SESSION_CNT_PER_CR *
				rro_cfg->win_sz * sizeof(struct addr_elem),
			&rro_cfg->addr_elem_alloc_pa[i],
			GFP_KERNEL);

		if (!rro_cfg->addr_elem_alloc_va[i])
			goto err;

		memset(rro_cfg->addr_elem_alloc_va[i], 0,
			HW_RRO_SESSION_CNT_PER_CR *
				rro_cfg->win_sz * sizeof(struct addr_elem));

		ptr = (struct addr_elem *) rro_cfg->addr_elem_alloc_va[i];
		for (j = 0; j < HW_RRO_SESSION_CNT_PER_CR * rro_cfg->win_sz; j++) {
			ptr->signature = 0xff;
			ptr++;
		}
	}

	rro_cfg->particular_se_id = rro_cfg->max_rro_se_cnt;
	rro_cfg->particular_session_va = dma_alloc_coherent(to_device(trans),
			rro_cfg->win_sz * sizeof(struct addr_elem),
			&rro_cfg->particular_session_pa,
			GFP_KERNEL);
	ptr = (struct addr_elem *) rro_cfg->particular_session_va;
	for (j = 0; j < rro_cfg->win_sz; j++) {
		ptr->signature = 0xff;
		ptr++;
	}

#ifdef CONFIG_HWIFI_DBG
	rro_cfg->addr_elem_cp_va = dma_alloc_coherent(to_device(trans),
				   rro_cfg->win_sz * sizeof(struct addr_elem),
				   &rro_cfg->addr_elem_cp_pa, GFP_KERNEL);

	if (rro_cfg->addr_elem_cp_va == NULL) {
		dev_err(to_device(trans), "%s(): addr_elem_cp_va allocated failed\n",
			__func__);
		goto err;
	}
#endif
	return 0;
err:
	return -ENOMEM;
}

bool init_rro_addr_elem_by_seid(struct pci_trans *trans, u16 seid)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	struct addr_elem *elem;
	int i;

	for (i = 0; i < MAX_HW_RRO_WIN_SIZE; i++) {
		elem = get_addr_elem(trans, rro_cfg, seid, i);
		elem->signature = 0xFF;
	}

	return true;

}

u32 rro_get_se_id(u16 wcid, u8 part_id)
{
	u32 se_id;

	se_id = ((wcid & 0x7F) << 3) + part_id;
	return se_id;

}

int rro_get_mode(struct pci_trans *trans)
{
	switch (trans->rro_mode) {
	case HW_RRO_OFF:
		return HW_RRO_MODE_OFF;
	case HW_RRO_V3_0_PRE_LINK:
		return HW_RRO_MODE_PRE_LINK;
	case HW_RRO_V3_0_BUF_PG:
	case HW_RRO_V3_0_BUF_PG_DBG:
	case HW_RRO_V3_1_BUF_PG_EMUL:
		return HW_RRO_MODE_BUF_PG;
	default:
		dev_err(to_device(trans), "%s(): rro_mode = %u undefined\n",
			__func__, trans->rro_mode);
		return -EINVAL;
	}
}

static int rro_init_ba_bitmap_cache(struct pci_trans *trans)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	u16 i;

	for (i = 0; i < HW_RRO_BA_BITMAP_CR_CNT; i++) {
		rro_cfg->ba_bitmap_cache_va[i] =
			dma_alloc_coherent(to_device(trans),
			BA_BITMAP_CACHE_SIZE_PER_CR,
			&rro_cfg->ba_bitmap_cache_pa[i], GFP_KERNEL);
		if (!rro_cfg->ba_bitmap_cache_va[i])
			goto err;
	}

	return 0;
err:
	return -ENOMEM;
}


static int rro_init_msdu_page(struct pci_trans *trans)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	u16 i;

	for (i = 0; i < HW_RRO_MSDU_PG_CR_CNT; i++) {
		rro_cfg->msdu_pg_alloc_va[i] =
			dma_alloc_coherent(to_device(trans),
			HW_RRO_MSDU_PG_SIZE_PER_CR,
			&rro_cfg->msdu_pg_alloc_pa[i], GFP_KERNEL);
		if (!rro_cfg->msdu_pg_alloc_va[i])
			goto err;
	}

	return 0;
err:
	return -ENOMEM;
}

static int rro_init_cidx_didx_emi(struct pci_trans *trans)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;

	rro_cfg->cidx_va =
		dma_alloc_coherent(to_device(trans),
			sizeof(struct rro_cidx_didx_emi),
			&rro_cfg->cidx_pa, GFP_KERNEL);
	if (!rro_cfg->cidx_va)
			goto err;
	rro_cfg->didx_va =
		dma_alloc_coherent(to_device(trans),
			sizeof(struct rro_cidx_didx_emi),
			&rro_cfg->didx_pa, GFP_KERNEL);
	if (!rro_cfg->didx_va)
			goto err;

	return 0;
err:
	return -ENOMEM;
}


static int rro_init_ddr(struct pci_trans *trans)
{
	int ret;

	if (trans->rro_mode <= HW_RRO_V3_0_BUF_PG_DBG) {
		ret = rro_init_ba_bitmap_cache(trans);
		if (ret)
			goto err;
	}

	ret = rro_init_addr_elem(trans);
	if (ret)
		goto err;

	if (trans->rro_mode >= HW_RRO_V3_1_BUF_PG_EMUL) {
		ret = rro_init_msdu_page(trans);
		if (ret)
			goto err;

		ret = rro_init_cidx_didx_emi(trans);
		if (ret)
			goto err;
	}

	return 0;
err:
	return -EINVAL;
}

static int rro_exit_addr_elem(struct pci_trans *trans)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	u16 i;

	for (i = 0; i < HW_RRO_ADDR_ELEM_CR_CNT; i++) {
	     if (rro_cfg->addr_elem_alloc_va[i])
			dma_free_coherent(to_device(trans),
				HW_RRO_SESSION_CNT_PER_CR *
				rro_cfg->win_sz * sizeof(struct addr_elem),
				rro_cfg->addr_elem_alloc_va[i],
				rro_cfg->addr_elem_alloc_pa[i]);
	}

	if (rro_cfg->particular_session_va)
		dma_free_coherent(to_device(trans),
			rro_cfg->win_sz * sizeof(struct addr_elem),
			rro_cfg->particular_session_va,
			rro_cfg->particular_session_pa);

#ifdef CONFIG_HWIFI_DBG
	if (rro_cfg->addr_elem_cp_va)
		dma_free_coherent(to_device(trans),
			rro_cfg->win_sz * sizeof(struct addr_elem),
			rro_cfg->addr_elem_cp_va,
			rro_cfg->addr_elem_cp_pa);
#endif

	return 0;
}


static int rro_exit_ba_bitmap_cache(struct pci_trans *trans)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	u16 i;

	for (i = 0; i < HW_RRO_BA_BITMAP_CR_CNT; i++) {
		if (rro_cfg->ba_bitmap_cache_va[i])
			dma_free_coherent(to_device(trans),
			BA_BITMAP_CACHE_SIZE_PER_CR,
			rro_cfg->ba_bitmap_cache_va[i],
			rro_cfg->ba_bitmap_cache_pa[i]);
	}

	return 0;
}

static int rro_exit_msdu_page(struct pci_trans *trans)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	u16 i;

	for (i = 0; i < HW_RRO_MSDU_PG_CR_CNT; i++) {
		if (rro_cfg->msdu_pg_alloc_va[i])
			dma_free_coherent(to_device(trans),
			HW_RRO_MSDU_PG_SIZE_PER_CR,
			rro_cfg->msdu_pg_alloc_va[i],
			rro_cfg->msdu_pg_alloc_pa[i]);
	}

	return 0;
}

static int rro_exit_cidx_didx_emi(struct pci_trans *trans)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;

	if (rro_cfg->cidx_va)
		dma_free_coherent(to_device(trans),
			sizeof(struct rro_cidx_didx_emi),
			rro_cfg->cidx_va,
			rro_cfg->cidx_pa);

	if (rro_cfg->didx_va)
		dma_free_coherent(to_device(trans),
			sizeof(struct rro_cidx_didx_emi),
			rro_cfg->didx_va,
			rro_cfg->didx_pa);

	return 0;
}


static int rro_exit_ddr(struct pci_trans *trans)
{
	int ret;

	if (trans->rro_mode <= HW_RRO_V3_0_BUF_PG_DBG) {
		ret = rro_exit_ba_bitmap_cache(trans);
		if (ret)
			goto err;
	}

	ret = rro_exit_addr_elem(trans);
	if (ret)
		goto err;

	if (trans->rro_mode >= HW_RRO_V3_1_BUF_PG_EMUL) {
		ret = rro_exit_msdu_page(trans);
		if (ret)
			goto err;

		ret = rro_exit_cidx_didx_emi(trans);
		if (ret)
			goto err;
	}

	return 0;
err:
	return -EINVAL;
}

#ifdef CONFIG_RRO_PRELINK
static int rro_init_free_pool_tk(struct pci_trans *trans)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct mtk_hwres_cap *res_cap = dev->chip_drv->hw_caps->hwres;
	u16 i;
	u32 id = RX_TK_NEXT_ID_NULL;
	u32 token_start = RX_TK_NEXT_ID_NULL, token_end = RX_TK_NEXT_ID_NULL;
#ifdef CONFIG_HWIFI_DBG
	struct hw_rro_dbg *rro_dbg = &rro_cfg->rro_dbg;
#endif

	for (i = 0; i < res_cap->rro_free_pool_tk_nums; i++) {
		id = mtk_rx_tk_request_entry(
			to_rx_tk_mgmt(trans),
			NULL, 0, 0, 0, &id);
		mtk_rx_tk_entry_update_state(
			to_rx_tk_mgmt(trans),
			id, TOKEN_STATE_IN_FREE_POOL);
		if (i == 0)
			rro_cfg->fp_head_sid = id;
		if (token_start == RX_TK_NEXT_ID_NULL)
			token_start = id;
		else
			token_end = id;
	}
#ifdef CONFIG_HWIFI_DBG
	rro_dbg->fp_tkid_start = token_start;
	rro_dbg->fp_tkid_end = token_end;
#endif
	return 0;
}

static int rro_exit_free_pool_tk(struct pci_trans *trans)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	u32 deq_id = RX_TK_NEXT_ID_NULL, next_id;

	next_id = rro_cfg->fp_head_sid;

	while (deq_id != next_id) {
		spin_lock(&rro_cfg->lock);
		deq_id = next_id;
		mtk_rx_tk_dequeue_id_from_list(to_rx_tk_mgmt(trans), deq_id, &next_id);
		rro_cfg->fp_head_sid = next_id;
		spin_unlock(&rro_cfg->lock);
		mtk_rx_tk_free_entry(to_rx_tk_mgmt(trans), deq_id);
	}

	return 0;
}
#endif

int rro_msdu_page_hash_init(
	struct pci_trans *trans,
	void *pkt,
	void *va,
	dma_addr_t pa)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	struct msdu_page_addr *pg_addr;

	pg_addr = kzalloc(sizeof(*pg_addr), GFP_ATOMIC);
	if (!pg_addr)
		goto err;
	pg_addr->pkt = pkt;
	pg_addr->pa = pa;
	pg_addr->va = va;

	spin_lock(&rro_cfg->lock);
	list_add_tail(&pg_addr->list,
		&rro_cfg->pg_hash_head[msdu_page_hash(pa)]);
	spin_unlock(&rro_cfg->lock);

	return 0;
err:
	return -ENOMEM;
}


static int rro_init_msdu_page_list(struct pci_trans *trans)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	u16 i;

	for (i = 0; i < MSDU_PAGE_HASH_SIZE; i++)
		INIT_LIST_HEAD(&rro_cfg->pg_hash_head[i]);

	return 0;
}

static int rro_exit_msdu_page_hash(struct pci_trans *trans)
{
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	u16 i;
	struct list_head *p, *q;
	struct msdu_page_addr *pg_addr = NULL;

	spin_lock(&rro_cfg->lock);
	for (i = 0; i < MSDU_PAGE_HASH_SIZE; i++) {
		list_for_each_safe(p, q, &rro_cfg->pg_hash_head[i]) {
			pg_addr = list_entry(p, struct msdu_page_addr, list);
			list_del(&pg_addr->list);
			kfree(pg_addr);
		}
	}
	spin_unlock(&rro_cfg->lock);

	return 0;
}

static void
rro_sw_cfg_rro_queue(struct pci_trans *trans)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct pci_trans *parent = to_pci_trans(dev->bus_trans);
	struct pci_rx_queue *rxq;
	bool rro_disable = !parent->rro_mode;
	int i;

	for (i = 0; i < trans->rxq_num; i++) {
		rxq = &trans->rxq[i];

		switch (rxq->q.q_attr) {
		case Q_RX_DATA_RRO:
			rxq->q.disable = rro_disable;
			break;
		case Q_RX_DATA_MSDU_PG:
		case Q_RX_IND:
			rxq->q.disable = trans->rro_mode == HW_RRO_V3_1 ? true : rro_disable;
			break;
		case Q_RX_RXDMAD_C:
			rxq->q.disable = trans->rro_mode != HW_RRO_V3_1 ? true : rro_disable;
			if (!rxq->q.disable)
				rxq->q.disable = (dev->rss_enable ^ (rxq->q.isr_map_idx != 0));
			break;
		default:
			break;
		}
	}
}

static int
rro_init_dbg(struct pci_trans *trans)
{
#ifdef CONFIG_HWIFI_DBG
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct dentry *dir = dev->sys_idx.dir;
	struct dentry *new_dir;

	mt_debugfs_create_devm_seqfile(dev->dev, "rroinfo",
		dir, rroinfo_debugfs);

	new_dir = mt_debugfs_lookup("rroinfo", dir);

	if (IS_ERR_OR_NULL(new_dir))
		return -ENOMEM;

#endif
	return 0;
}

static void
rro_exit_dbg(struct pci_trans *trans)
{
#ifdef CONFIG_HWIFI_DBG
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct dentry *dir = dev->sys_idx.dir;
	struct dentry *new_dir;

	new_dir = mt_debugfs_lookup("rroinfo", dir);
	mt_debugfs_remove_recursive(new_dir);
	return;
#endif
}

int rro_sw_init(struct pci_trans *trans)
{
	int ret;

	rro_sw_cfg_rro_queue(trans);

	if (!trans->rro_mode)
		return 0;

	ret = rro_init_ddr(trans);
	if (ret)
		goto err_init_ddr;

#ifdef CONFIG_RRO_PRELINK
	if (trans->rro_mode == HW_RRO_V3_0_PRE_LINK) {
		ret = rro_init_free_pool_tk(trans);
		if (ret)
			goto err_init_free_pool;
	}
#endif

	ret = rro_init_msdu_page_list(trans);
	if (ret)
		goto err_init_msdu_page;

	ret = rro_init_dbg(trans);
	if (ret)
		goto err_init_dbg;

	return 0;
err_init_dbg:
	rro_exit_msdu_page(trans);
err_init_msdu_page:
#ifdef CONFIG_RRO_PRELINK
	rro_exit_free_pool_tk(trans);
err_init_free_pool:
#endif
	rro_exit_ddr(trans);
err_init_ddr:
	return ret;
}

int rro_sw_exit(struct pci_trans *trans)
{
	int ret;

	if (!trans->rro_mode)
		return 0;

#ifdef CONFIG_RRO_PRELINK
	if (trans->rro_mode == HW_RRO_V3_0_PRE_LINK) {
		ret = rro_exit_free_pool_tk(trans);
		if (ret)
			goto err;
	}
#endif
	ret = rro_exit_ddr(trans);
	if (ret)
		goto err;

	ret = rro_exit_msdu_page_hash(trans);
	if (ret)
		goto err;

	rro_exit_dbg(trans);

	return 0;
err:
	return -EINVAL;
}

int rro_alloc_resource(struct pci_trans *trans, struct rro_chip_profile *profile)
{
	struct hw_rro_cfg *rro_cfg;

	trans->rro_mode = profile->rro_mode;
	if (!trans->rro_mode)
		return 0;

	trans->rro_cfg = kzalloc(sizeof(struct hw_rro_cfg), GFP_KERNEL);
	if (!trans->rro_cfg)
		return -ENOMEM;

	rro_cfg = trans->rro_cfg;
	rro_cfg->sign_base_0_addr = profile->sign_base_0_addr;
	rro_cfg->sign_base_1_addr = profile->sign_base_1_addr;
	rro_cfg->sign_base_1_en_shift = profile->sign_base_1_en_shift;
	rro_cfg->debug_mode_rxd_addr = profile->debug_mode_rxd_addr;
	rro_cfg->ack_sn_addr = profile->ack_sn_addr;
	rro_cfg->ack_sn_mask = profile->ack_sn_mask;
	rro_cfg->ack_sn_seid_mask = profile->ack_sn_seid_mask;
	rro_cfg->ack_sn_shift = profile->ack_sn_shift;
	rro_cfg->ack_sn_seid_shift = profile->ack_sn_seid_shift;
	rro_cfg->ack_sn_update_cnt = profile->ack_sn_update_cnt;
	rro_cfg->dbg_rd_ctrl_addr = profile->dbg_rd_ctrl_addr;
	rro_cfg->dbg_rd_ctrl_mask = profile->dbg_rd_ctrl_mask;
	rro_cfg->dbg_rdat_dw0 = profile->dbg_rdat_dw0;
	rro_cfg->dbg_rdat_dw1 = profile->dbg_rdat_dw1;
	rro_cfg->dbg_rdat_dw2 = profile->dbg_rdat_dw2;
	rro_cfg->dbg_rdat_dw3 = profile->dbg_rdat_dw3;
	rro_cfg->max_rro_se_cnt = profile->max_rro_se_cnt;
	rro_cfg->addr_elem_cr_cnt = profile->addr_elem_cr_cnt;

	return 0;
}

inline void
rro_free_resource(struct pci_trans *trans)
{
	kfree(trans->rro_cfg);
	trans->rro_cfg = NULL;
}

struct pci_rxq_ops rro_rxq_ops = {
	.get_data = rro_refill_data_buf,
	.process = pci_dma_rx_process,
	.init = pci_dma_init_rx_data_queue,
	.exit = pci_dma_exit_rx_data_queue,
};

struct pci_rxq_ops ind_cmd_rxq_ops = {
	.get_data = NULL,
	.process = rro_ind_cmd_process,
	.init = rro_init_ind_cmd_queue,
	.exit = NULL,
};

struct pci_rxq_ops rxdmad_c_rxq_ops = {
	.get_data = NULL,
	.process = rro_rxdmad_c_process,
	.init = rro_init_rxdmad_c_queue,
	.exit = NULL,
};
