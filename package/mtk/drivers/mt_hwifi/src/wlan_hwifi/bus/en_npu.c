// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) [2023] Airoha Inc.
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
#include <linux/pci.h>
#include "en_npu.h"
#include "mtk_pci_dma.h"
#include <ecnt_hook/ecnt_hook_pcie.h>
#include <modules/npu/wifi_mail.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include "mtk_rro.h"
#include "../../../npu/UNION_EN7563_MULTI_CHIP_KERNEL_5_4_demo/host/npu_rv32.h"

unsigned long *ba_reorder_va;

int pci_dma_rx_process_hostadpt(struct pci_trans *trans, struct pci_queue *q, int budget)
{
	struct sk_buff *skb = NULL;
	int done = 0;
	unsigned char bReschedule = 0;
	unsigned int vend_spec = 0;
	__le32 *rxd = NULL;
	unsigned char band_idx = q->band_idx_bmp;
	unsigned char hostadpt_ring_num = 0;
#if NPU_SUPPORT_RRO
	union _2_hostapdt_vend_specific *vnd_spe;
	struct mtk_bus_rx_info rx_info = {0};
	struct mtk_bus_rx_info *info;
#endif

	if (q->q_attr != Q_RX_RXDMAD_C)
		dev_info(to_device(trans), "%s()wrong q: %x !!!\n", __func__, q->hw_desc_base);

	//according to mt7990_wfdma_wa.h, BAND0_RX_PCIE0 for band0/1, BAND2_RX_PCIE0 for band 2
	//For band0/1, we use hostadpt ring0.
	//For band 2, we use hostadpt ring1.
	//Above is based on 2G+5G+6G are all on pcie0(option_type=0 in mt7990_option_table.h).
	if (band_idx == BIT(BAND2))
		hostadpt_ring_num = 1;
	else
		hostadpt_ring_num = 0;

	while (done < budget) {
		if (fromHostadptPktHandle_hook)
			skb = fromHostadptPktHandle_hook(hostadpt_ring_num,
							&bReschedule, &vend_spec);

		if (skb == NULL) {
			mtk_dbg(MTK_RRO, "%s()[%d] no pkt\n", __func__, __LINE__);
			break;
		}

		rxd = (__le32 *)skb->data;

#if NPU_SUPPORT_RRO
		vnd_spe = (union _2_hostapdt_vend_specific *)&vend_spec;

		memset(&rx_info, 0, sizeof(rx_info));
		rx_info.more		 =	vnd_spe->rx_info.more;
		rx_info.len			 =	vnd_spe->rx_info.len;
		rx_info.ip_frag		 =	vnd_spe->rx_info.ip_frag;
		rx_info.hw_rro		 =	vnd_spe->rx_info.hw_rro;
		rx_info.eth_hdr_ofst =	vnd_spe->rx_info.eth_hdr_ofst;
		rx_info.old_pkt		 =	vnd_spe->rx_info.old_pkt;
		rx_info.repeat_pkt	 =	vnd_spe->rx_info.repeat_pkt;
		rx_info.wifi_frag	 =	vnd_spe->rx_info.wifi_frag;

		if (vnd_spe->rx_info.pn_chk_fail)
			rx_info.pn_chk_fail = true;

		if (vnd_spe->rx_info.rro_v31_token)
			dev_dbg(to_device(trans), "\nthis is a leakage token!\n");

		info = (struct mtk_bus_rx_info *)skb->cb;
		*info = rx_info;
#endif

		mtk_bus_rx_process((struct mtk_bus_trans *)trans, Q_RX_DATA, skb);

		done++;
	}

	return done;
}

#if RRO_IN_NPU
int pci_dma_rx_poll_hostadpt_ring0(struct napi_struct *napi, int budget)
{
	struct pci_queue *q;
	struct pci_trans *trans;
	int done = 0, cur;
	struct pci_rx_queue *rq;

	trans = container_of(napi->dev, struct pci_trans, napi_dev);
	q = container_of(napi, struct pci_queue, napi);
	rq = (struct pci_rx_queue *)q;

	local_bh_disable();
	mt_rcu_read_lock();

	if (q->q_attr != Q_RX_RXDMAD_C)
		dev_info(to_device(trans), "%s()[%d] BUG. Wrong interrupt value:%d\n",
				__func__, __LINE__, q->hw_int_mask);

	//q->tail = bus_get_field(to_bus_trans(trans), q->hw_didx_addr, q->hw_didx_mask);
	do {
		cur = pci_dma_rx_process_hostadpt(trans, q, budget - done);
		done += cur;

		//sync sw idx
		//q->tail = bus_get_field(to_bus_trans(trans),
		//		q->hw_didx_addr, q->hw_didx_mask);
	} while (cur && done < budget);

	mt_rcu_read_unlock();
	local_bh_enable();

	if (done < budget) {
		// reschedule if ring is not empty
		//if (((q->head + 1) % q->q_size) != q->tail)
		//	return budget;

		napi_complete(napi);
		//atomic_set(&in_task_band01, 0); //set in_task_band01 to 0 since napi is completed.
		if (hostdapt_enable_int_hook)
			hostdapt_enable_int_hook(0);
	}
	return done;
}
#endif
static void
npu_dma_set_irq_mask(struct pci_irq_vector_data *vec_data, u32 addr,
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
npu_dma_irq_ena_clr(struct pci_trans *trans, u8 map_idx, u32 clr)
{
	bus_write(to_bus_trans(trans),
			trans->int_mask_map[map_idx].int_ena_clr_addr, clr);
}

static void
npu_dma_irq_disable(struct pci_irq_vector_data *vec_data, u32 mask)
{
	struct pci_trans *trans = to_pci_trans(vec_data->bus_trans);
	u8 map_idx = vec_data->isr_map_idx;

	if (map_idx)
		npu_dma_irq_ena_clr(trans, map_idx, mask);
	else
		npu_dma_set_irq_mask(vec_data,
			trans->int_mask_map[map_idx].int_ena_addr, mask, 0);
}


struct pci_trans *glb_npu_trans1;

void npu_unassign_trans(struct npu_trans *trans)
{
	dev_info(to_device(trans), "%s() npu unregister hostapdt hook func\n", __func__);
	//glb_npu_trans1 = NULL;
	//glb_npu_trans2 = NULL;
	if (hostdapt_registe_wifitask_hook) {
		hostdapt_registe_wifitask_hook(0, NULL);
		hostdapt_registe_wifitask_hook(1, NULL);
	}
}

void npu_pci_dma_sw_init_device(struct npu_trans *trans)
{
	int isMailBoxSuccess = 0, hw_pcie_port_type = -1;

#ifdef TCSUPPORT_CPU_AN7552
	hw_pcie_port_type = P0_P0;
#endif

	if (hw_pcie_port_type != -1) {
		isMailBoxSuccess = WIFI_MAIL_API_SET_WAIT_PCIE_PORT_TYPE(0, hw_pcie_port_type);
		if (isMailBoxSuccess != 1)
			dev_info(to_device(trans),
				"Error: WIFI_MAIL_API_SET_WAIT_PCIE_PORT_TYPE fail !!!\n");
	} else
		dev_info(to_device(trans), "[ERROR]ISnot AN7552 for NPU's pcie port type\n");
}

static u8
npu_dma_get_vec_id_by_int_mask(struct pci_trans *trans, u8 isr_map_idx, u32 mask)
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
npu_dma_queue_napi_init(struct pci_trans *trans,
	struct pci_queue *q, u8 res_id, void *cb)
{
	u8 map_id = ffs(q->hw_int_mask)-1;
	u8 vec_id = npu_dma_get_vec_id_by_int_mask(trans, q->isr_map_idx, 1 << map_id);

	if (map_id >= 32)
		return -EINVAL;

	q->vec_id = vec_id;
	q->resource_idx = res_id;
	trans->isr_map[q->isr_map_idx][map_id] = &q->napi;
	netif_napi_add(&trans->napi_dev, &q->napi, cb, NAPI_POLL_WEIGHT);
	napi_enable(&q->napi);
	/* avoid schedule loss */
	napi_schedule(trans->isr_map[q->isr_map_idx][map_id]);

	return 0;
}

static int
npu_dma_queue_napi_exit(struct pci_trans *trans, struct pci_queue *q)
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

int npu_pci_dma_sw_init_rx_queue(struct npu_trans *wtrans)
{
	struct pci_trans *trans = to_pci_trans(wtrans);
	struct pci_rx_queue *rq;
	struct pci_queue *q;
	int i;

	for (i = 0 ; i < trans->rxq_num; i++) {
		rq = &trans->rxq[i];
		q = (struct pci_queue *)rq;

		if (!rq->q.disable &&
			trans->rro_mode == HW_RRO_V3_1 && q->q_attr == Q_RX_RXDMAD_C) {

			if (!glb_npu_trans1)
				glb_npu_trans1 = trans;

			npu_dma_queue_napi_exit(trans, (struct pci_queue *)rq);
			npu_dma_queue_napi_init(trans, (struct pci_queue *)rq,
						i, pci_dma_rx_poll_hostadpt_ring0);
		}
	}
	return 0;
}

void tasklet_schedule_rx_data_done_hostadpt_ring0(void)
{
	struct pci_trans *trans;
	struct pci_irq_vector_data *vec_data;
	struct pci_rx_queue *rxq = NULL, *rq = NULL;
	struct pci_queue *q = NULL;
	u8 cur = 0, map_idx = 0, rn, idx;

	trans = glb_npu_trans1;
	if (trans == NULL) {
		dev_info(to_device(trans), "%s()[ERROR] unassigned trans ptr.\n", __func__);
		return;
	}

	rxq = trans->rxq;
	rn = trans->rxq_num;

	for (idx = 0 ; idx < rn; idx++) {
		rq = &rxq[idx];
		q = &rq->q;

		if (q->q_attr == Q_RX_RXDMAD_C)
			break;
	}

	vec_data = &trans->vec_data[q->vec_id];
	map_idx = vec_data->isr_map_idx;

	cur = ffs(q->hw_int_mask)-1;

	if (trans->isr_map[map_idx][cur])
		napi_schedule(trans->isr_map[map_idx][cur]);
}

static void npu_pci_dma_init_rx_data_queue(
	struct pci_trans *trans, struct pci_rx_queue *rq)
{
	struct pci_queue *q = &rq->q;
	struct pci_dma_buf *desc = &q->desc_ring;
	struct pci_dma_cb *cb;
	unsigned long pci_pa = 0;
	int isMailBoxSuccess = 0;
	npu_info_t get_RingBase;
	void *npu_alloc_va = NULL;
	dma_addr_t npu_alloc_pa;
	unsigned char msdu_offset = 0;

	if ((q->q_attr == Q_RX_DATA_RRO || q->q_attr == Q_RX_DATA_MSDU_PG)) {
		if (q->q_attr == Q_RX_DATA_MSDU_PG) {
			// This offset value should be carefully sync with NPU code
			msdu_offset = 5;
			if (trans->rro_mode >= HW_RRO_V3_1) {
				dev_info(to_device(trans),
					"%s() RRO mode:%d so return\n", __func__, trans->rro_mode);
				return;
			}
		}

		pci_pa = pci_resource_start(trans->pdev, 0);
		//printk("%s() send pcie pa to NPU. %lx %x %lx\n", __func__,
		//	pci_pa, q->hw_desc_base, (pci_pa + q->hw_desc_base));

		isMailBoxSuccess = WIFI_MAIL_API_SET_WAIT_PCIE_ADDR(
			(ffs(q->band_idx_bmp)-1) + msdu_offset,
			(pci_pa + q->hw_desc_base));
		if (isMailBoxSuccess != 1)
			dev_info(to_device(trans),
				"%s() Error: WIFI_MAIL_API_SET_WAIT_PCIE_ADDR fail\n", __func__);

		isMailBoxSuccess = WIFI_MAIL_API_SET_WAIT_DESC(
				(ffs(q->band_idx_bmp)-1) + msdu_offset, q->q_size);
		if (isMailBoxSuccess != 1)
			dev_info(to_device(trans),
					"%s() Error: WIFI_MAIL_API_SET_WAIT_DESC fail\n", __func__);

		if (q->q_attr == Q_RX_DATA_MSDU_PG) {
			// This offset value should be carefully sync with NPU code
			msdu_offset = 10;
		}

#ifdef REFILL_RING_IN_DRAM_ADDR
		WIFI_MAIL_API_SET_WAIT_TX_DESC_BASE(
					(ffs(q->band_idx_bmp)-1) + msdu_offset, desc->alloc_pa);
		npu_alloc_pa = desc->alloc_pa;
		npu_alloc_va = desc->alloc_va;
		dev_info(to_device(trans), "%s() q size:%d(%d*%d)\n", __func__,
						q->desc_ring.alloc_size, q->desc_size, q->q_size);
		dev_info(to_device(trans), "%s() desc:%d %lx %x\n", __func__, desc->alloc_size,
			(unsigned long)desc->alloc_va, desc->alloc_pa);

#else
		isMailBoxSuccess = WIFI_MAIL_API_GET_WAIT_RXDESC_BASE(
					(ffs(q->band_idx_bmp)-1) + msdu_offset, &get_RingBase);
		if (isMailBoxSuccess != 1)
			dev_info(to_device(trans), "Error: WIFI_MAIL_API_GET_WAIT_RXDESC_BASE fail\n");

		npu_alloc_pa = (dma_addr_t)get_RingBase.info;
		npu_alloc_va = ioremap((phys_addr_t)get_RingBase.info, 4);

		if (desc->alloc_va)
			dma_free_coherent(to_device(trans), desc->alloc_size,
					desc->alloc_va, desc->alloc_pa);

		desc->alloc_va = npu_alloc_va;
		desc->alloc_pa = npu_alloc_pa;
#endif


		rq->magic_cnt = 0;

		cb = &q->cb[0];
		cb->alloc_size = q->desc_size;
		cb->alloc_va = desc->alloc_va;
		cb->alloc_pa = desc->alloc_pa;
		q->head = q->q_size - 1;
		q->tail = q->q_size;
	}
}

static void npu_rro_init_rxdmad_c_queue(
	struct pci_trans *trans,
	struct pci_rx_queue *rq)
{
	struct pci_queue *q = &rq->q;
	struct pci_dma_buf *desc = &q->desc_ring;
	struct hw_rro_cfg *rro_cfg = trans->rro_cfg;
	struct rro_cidx_didx_emi *cidx = rro_cfg->cidx_va;
	struct pci_dma_cb *cb;
	unsigned long pci_pa = 0;
	int isMailBoxSuccess = 0;
	unsigned char rxdmad_c_offset = 9; //should be sync with NPU code,value = ind_cmd offset
	npu_info_t get_RingBase;
	dma_addr_t npu_alloc_pa;
	void *npu_alloc_va = NULL;

	pci_pa = pci_resource_start(trans->pdev, 0);

	isMailBoxSuccess = WIFI_MAIL_API_SET_WAIT_PCIE_ADDR(0 + rxdmad_c_offset,
							(pci_pa + q->hw_desc_base));
	if (isMailBoxSuccess != 1)
		dev_info(to_device(trans), "Error: WIFI_MAIL_API_SET_WAIT_PCIE_ADDR fail\n");

	isMailBoxSuccess = WIFI_MAIL_API_SET_WAIT_DESC(0 + rxdmad_c_offset, q->q_size);
	if (isMailBoxSuccess != 1)
		dev_info(to_device(trans), "Error: WIFI_MAIL_API_SET_WAIT_DESC fail\n");

	isMailBoxSuccess = WIFI_MAIL_API_GET_WAIT_RXDESC_BASE(0 + rxdmad_c_offset, &get_RingBase);
	if (isMailBoxSuccess != 1)
		dev_info(to_device(trans), "Error: WIFI_MAIL_API_GET_WAIT_RXDESC_BASE fail\n");

	npu_alloc_pa = (dma_addr_t)get_RingBase.info;
	npu_alloc_va = ioremap((phys_addr_t)get_RingBase.info, 4);


	if (desc->alloc_va)
		dma_free_coherent(to_device(trans), desc->alloc_size,
					desc->alloc_va, desc->alloc_pa);

	desc->alloc_va = npu_alloc_va;
	desc->alloc_pa = npu_alloc_pa;

	cb = &q->cb[0];
	cb->alloc_size = q->desc_size;
	cb->alloc_va = desc->alloc_va;
	cb->alloc_pa = desc->alloc_pa;

	q->head = q->q_size - 1;
	q->tail = q->q_size;

	set_bit(Q_FLAG_WRITE_EMI, &q->q_flags);
	q->emi_cidx_addr = &cidx->ring[q->emi_ring_idx].idx;

	dev_dbg(to_device(trans), "%s() pa of emi_cidx_addr: %x\n", __func__, rro_cfg->cidx_pa);
	isMailBoxSuccess = WIFI_MAIL_API_SET_WAIT_INODE_TXRX_REG_ADDR(5,
		0, (unsigned int)rro_cfg->cidx_pa, 0, 0);
	if (isMailBoxSuccess != 1)
		dev_info(to_device(trans), "Error: WIFI_MAIL_API_GET_WAIT_RXDESC_BASE fail\n");

}

void
npu_dma_exit_rx_queue(
	struct pci_trans *trans,
	struct pci_rx_queue *rq)
{
	dev_err(to_device(trans), "%s set rxq alloc_va to NULL!\n", __func__);
	iounmap((void *)(rq->q.desc_ring.alloc_va));
	rq->q.desc_ring.alloc_va = NULL;
}

struct pci_rxq_ops npu_rro_rxq_ops = {
	.get_data = NULL,
	.process = NULL,
	.init = npu_pci_dma_init_rx_data_queue,
	.exit = npu_dma_exit_rx_queue,
};

struct pci_rxq_ops npu_rxdmad_c_rxq_ops = {
	.get_data = NULL,
	.process = NULL,
	.init = npu_rro_init_rxdmad_c_queue,
	.exit = npu_dma_exit_rx_queue,
};


static int
npu_dma_set_queue_ops(void *hw)
{
	struct pci_trans *trans = to_pci_trans(hw);
	u8 idx, rxn = trans->rxq_num;

	for (idx = 0; idx < rxn; idx++) {
		struct pci_rx_queue *rq = &trans->rxq[idx];

		switch (rq->q.q_attr) {
		case Q_RX_DATA_RRO:
			rq->q_ops = &npu_rro_rxq_ops;
			rq->get_pkt_method = GET_PKT_DDONE;
			rq->magic_enable = false;
			/* there is no need to change outstanding, when method is DDONE*/
			trans->parent.dev->limit  &= ~(BIT(LIMIT_SET_OUTSTANDING));
			break;
		case Q_RX_RXDMAD_C:
			rq->q_ops = &npu_rxdmad_c_rxq_ops;
			break;
		default:
			break;
		}
	}
	dev_dbg(to_device(trans), "reset rxq ops!\n");

	return 0;
}

int npu_disable_rro_irq(struct npu_trans *wtrans)
{
	struct pci_trans *trans = to_pci_trans(wtrans);
	struct pci_rx_queue *rq;
	struct pci_queue *q;
	int i;

	for (i = 0 ; i < trans->rxq_num; i++) {
		rq = &trans->rxq[i];
		q = (struct pci_queue *)rq;

		if (!rq->q.disable &&
			(q->q_attr == Q_RX_DATA_RRO ||
			q->q_attr == Q_RX_DATA_MSDU_PG ||
			q->q_attr == Q_RX_RXDMAD_C))
			npu_dma_irq_disable(&trans->vec_data[q->vec_id],
						(1 << (ffs(q->hw_int_mask)-1)));
	}
	return 0;
}

void send_rro_done_msg_2_NPU(struct npu_trans *trans)
{
#if NPU_SUPPORT_RRO
	int isMailBoxSuccess = 0;

	isMailBoxSuccess = WIFI_MAIL_API_SET_WAIT_INODE_TXRX_REG_ADDR(2, 0, 0, 0, 0);
	if (isMailBoxSuccess != 1)
		dev_info(to_device(trans), "Error: WIFI_MAIL_API_SET_WAIT_DESC 2 fail\n");
#endif
}

//stop receiving packets on Refill/MSDU/ind_cmd ring when wifi reload
//to avoid rx buffer ID leak and msdu buffer ID leak problem.
void send_rro_stop_msg_2_NPU(struct pci_trans *trans)
{
#if NPU_SUPPORT_RRO
	int isMailBoxSuccess = 0;
	npu_info_t get_RingBase;
	unsigned int chk_NPU_is_stop;

	isMailBoxSuccess = WIFI_MAIL_API_SET_WAIT_INODE_TXRX_REG_ADDR(4, 0, 0, 0, 0);
	if (isMailBoxSuccess != 1)
		dev_info(to_device(trans), "Error: WIFI_MAIL_API_SET_WAIT_DESC 4 fail\n");

	while (1) {
		isMailBoxSuccess = WIFI_MAIL_API_GET_WAIT_NPU_INFO(3, &get_RingBase);
		if (isMailBoxSuccess != 1)
			dev_info(to_device(trans), "Error: WIFI_MAIL_API_GET_WAIT_NPU_INFO fail\n");

		chk_NPU_is_stop = (unsigned int)get_RingBase.info;
		if (!chk_NPU_is_stop) {
			dev_dbg(to_device(trans), "%s() NPU rx is stopped\n", __func__);
			break;
		}
	}

	isMailBoxSuccess = WIFI_MAIL_API_SET_WAIT_INODE_TXRX_REG_ADDR(6, 0, 0, 0, 0);
	if (isMailBoxSuccess != 1)
		dev_info(to_device(trans), "%s() Error: WIFI_MAIL_API_SET_WAIT_DESC fail\n",
			__func__);
#endif
}

static void
pci_dma_npu_stop(void *hw)
{
	struct pci_trans *trans = to_pci_trans(hw);

	if (glb_npu_en_node) {
		dev_dbg(to_device(trans), "send stop msg to NPU\n");
		send_rro_stop_msg_2_NPU(trans);
	}
}

static irqreturn_t
npu_irq_handler(int irq, void *data)
{
	struct npu_trans *trans = to_npu_trans_dp(data);

	return trans->dma_ops->irq_handler(irq, data);
}

static int
npu_alloc_resource(void *hw, void *profile)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->alloc_resource(hw, profile);
}

static void
npu_free_resource(void *hw)
{
	struct npu_trans *trans = to_npu_trans(hw);

	trans->dma_ops->free_resource(hw);
}

static int
npu_preinit_device(void *hw)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->preinit_device(hw);
}

static int
npu_init_device(void *hw)
{
	struct npu_trans *trans = to_npu_trans(hw);

	if (glb_npu_en_node)
		npu_dma_set_queue_ops(hw);

	trans->dma_ops->init_device(hw);

	if (glb_npu_en_node) {
		npu_pci_dma_sw_init_device(trans);
		npu_pci_dma_sw_init_rx_queue(trans);
		if (hostdapt_registe_wifitask_hook) {
			// hostadpt ring0 is set for band0/1 and ring1 is set for band2
			hostdapt_registe_wifitask_hook(0,
					tasklet_schedule_rx_data_done_hostadpt_ring0);
			hostdapt_registe_wifitask_hook(1, NULL);
		}
	}
	return 0;
}

static void
npu_exit_device(void *hw)
{
	struct npu_trans *trans = to_npu_trans(hw);

	trans->dma_ops->exit_device(hw);
}

static int
npu_init_after_fwdl(void *hw)
{
	struct npu_trans *trans = to_npu_trans(hw);
	int ret;

	ret = trans->dma_ops->init_after_fwdl(hw);
	if (ret)
		goto err;

	if (glb_npu_en_node) {
		npu_disable_rro_irq(trans);
		send_rro_done_msg_2_NPU(trans);
	}

err:
	return ret;
}

static int
npu_tx_mcu_queue(void *hw, struct sk_buff *skb, enum mtk_queue_attr q)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->tx_mcu_queue(hw, skb, q);
}

static bool
npu_request_tx(void *hw, void *txq)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->request_tx(hw, txq);
}

static int
npu_tx_data_queue(void *hw, struct mtk_bus_tx_info *tx_info)
{
	struct npu_trans *trans = to_npu_trans(tx_info->txq_trans);

	return trans->dma_ops->tx_data_queue(hw, tx_info);
}

static int
npu_tx_kick(void *hw)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->tx_kick(hw);
}

static int
npu_chip_attached(void *hw, void *bus_ops)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->chip_attached(hw, bus_ops);
}

static bool
npu_match(void *master, void *slave)
{
	struct npu_trans *trans = to_npu_trans(master);

	return trans->dma_ops->match(master, slave);
}

static void
npu_tx_free(void *hw, struct sk_buff *skb)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->tx_free(hw, skb);
}


static int
npu_start(void *hw)
{
	struct npu_trans *trans = to_npu_trans(hw);

	if (trans->dma_ops->start)
		return trans->dma_ops->start(hw);
	return 0;
}

static void
npu_stop(void *hw)
{
	struct npu_trans *trans = to_npu_trans(hw);

	//since NPU handles RRO related ring by reading dma index from reg.
	//we need to stop polling in the beginning to avoid polling dma index
	//when it is reset to 0.
	if (glb_npu_en_node)
		pci_dma_npu_stop(hw);

	if (trans->dma_ops->stop)
		trans->dma_ops->stop(hw);
}

static int
npu_start_traffic(void *hw)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->start_traffic(hw);
}

static int
npu_stop_traffic(void *hw)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->stop_traffic(hw);
}

static int
npu_get_txq(void *hw, u8 band_idx, u8 tid, void **txq, void **txq_trans)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->get_txq(hw, band_idx, tid, txq, txq_trans);
}

static int
npu_add_hw_rx(void *hw, struct mtk_bus_rx_info *rx_info)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->add_hw_rx(hw, rx_info);
}

/*dbugfs*/
static int
npu_queues_read(void *hw, struct seq_file *s)
{
	struct wed_trans *trans = to_wed_trans(hw);

	return trans->dma_ops->queues_read(trans, s);
}

static int
npu_get_rro_mode(void *hw)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->get_rro_mode(hw);
}

static int
npu_rro_write_dma_idx(void *hw, u32 value)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->rro_write_dma_idx(hw, value);
}


static void *
npu_page_hash_get(void *hw, dma_addr_t pa)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->rro_page_hash_get(hw, pa);
}


static int
npu_switch_node(void *hw_master, void *hw_slave)
{
	struct npu_trans *trans = to_npu_trans(hw_master);

	return trans->dma_ops->switch_node(hw_master, hw_slave);
}

static bool npu_init_rro_addr_elem_by_seid(void *hw, u16 seid)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->init_rro_addr_elem_by_seid(hw, seid);
}
static int
npu_int_to_mcu(void *hw, u32 value)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->int_to_mcu(hw, value);
}

static int
npu_set_pao_sta_info(void *hw, u16 wcid,
		u8 max_amsdu_nums, u32 max_amsdu_len, int remove_vlan, int hdrt_mode)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->set_pao_sta_info(hw, wcid, max_amsdu_nums,
					max_amsdu_len, remove_vlan, hdrt_mode);
}

static int
npu_set_pn_check(void *hw, u32 se_id, bool enable)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->set_pn_check(hw, se_id, enable);
}

static int
npu_set_particular_to_host(void *hw, bool enable)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->set_particular_to_host(hw, enable);
}

static int
npu_get_device(void *hw, void **device)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->get_device(hw, device);
}

static int
npu_get_tx_token_num(void *hw, u16 tx_token_num[], u8 max_src_num)
{
	struct npu_trans *trans = to_npu_trans(hw);

	return trans->dma_ops->get_tx_token_num(hw, tx_token_num, max_src_num);
}

static int npu_get_rro_sp_page_num(void *hw, u32 *page_num)
{
	struct npu_trans *trans = to_npu_trans(hw);
	u32 sp_num = 0, abnormal3_num = 0;

	if (counter_base[0] && counter_base[1]) {
		sp_num = NPU_COUNTER(counter_base[1], PN_CHECK_FAIL);
		abnormal3_num = NPU_COUNTER(counter_base[0], SKB_BUFID_STATE_ABNORMAL3);
		*page_num = sp_num > abnormal3_num ? sp_num - abnormal3_num : 0;
		dev_dbg(to_device(trans), "get rro page leakage:%d(%d-%d)\n",
				*page_num, sp_num, abnormal3_num);

		return 0;
	}

	if (trans->dma_ops->get_rro_sp_page_num)
		return trans->dma_ops->get_rro_sp_page_num(hw, page_num);

	return 0;
}

static struct mtk_bus_dma_ops npu_dma_ops = {
	.alloc_resource = npu_alloc_resource,
	.free_resource = npu_free_resource,
	.preinit_device = npu_preinit_device,
	.init_device = npu_init_device,
	.exit_device = npu_exit_device,
	.init_after_fwdl = npu_init_after_fwdl,
	.tx_mcu_queue = npu_tx_mcu_queue,
	.request_tx = npu_request_tx,
	.tx_data_queue = npu_tx_data_queue,
	.tx_kick = npu_tx_kick,
	.chip_attached = npu_chip_attached,
	.match = npu_match,
	.tx_free = npu_tx_free,
	.start = npu_start,
	.stop = npu_stop,
	.start_traffic = npu_start_traffic,
	.stop_traffic = npu_stop_traffic,
	.get_txq = npu_get_txq,
	.queues_read = npu_queues_read,
	.add_hw_rx = npu_add_hw_rx,
	.int_to_mcu = npu_int_to_mcu,
	.irq_handler = npu_irq_handler,
	.get_rro_mode = npu_get_rro_mode,
	.rro_write_dma_idx = npu_rro_write_dma_idx,
	.rro_page_hash_get = npu_page_hash_get,
	.switch_node = npu_switch_node,
	.init_rro_addr_elem_by_seid = npu_init_rro_addr_elem_by_seid,
	.set_pao_sta_info = npu_set_pao_sta_info,
	.set_pn_check = npu_set_pn_check,
	.set_particular_to_host = npu_set_particular_to_host,
	.get_device = npu_get_device,
	.get_tx_token_num = npu_get_tx_token_num,
	.get_rro_sp_page_num = npu_get_rro_sp_page_num,
};

static int
npu_drv_register(void *drv, void *priv_drv)
{
	struct mtk_bus_driver *bus_drv = (struct mtk_bus_driver *)drv;

	if (bus_drv->sub_drv)
		return bus_drv->sub_drv->ops.drv_register((void *) bus_drv->sub_drv, priv_drv);
	return 0;
}

static int
npu_drv_unregister(void *drv, void *priv_drv)
{
	struct mtk_bus_driver *bus_drv = (struct mtk_bus_driver *)drv;

	if (bus_drv->sub_drv)
		return bus_drv->sub_drv->ops.drv_unregister((void *)bus_drv->sub_drv, priv_drv);
	return 0;
}

static int
npu_drv_create_tunnel(void *drv, struct mtk_bus_trans *trans, u32 bus_type)
{
	struct npu_trans *npu_trans = (struct npu_trans *) trans;

	if (trans->io_ops) {
		npu_trans->io_ops = trans->io_ops;
		npu_trans->dma_ops = trans->dma_ops;
		trans->dma_ops = &npu_dma_ops;
	} else {
		dev_err(to_device(trans), "%s(): Create tunnel failed\n",
			__func__);
		return -EINVAL;
	}

	return 0;
}

static int
npu_drv_profile_dump(struct seq_file *s, void *profile)
{
	return 0;
}

static struct mtk_bus_driver npu_bus = {
	.bus_type = MTK_BUS_NPU,
#ifdef CONFIG_HWIFI_HIF_WED
	.bus_sub_type = MTK_BUS_WED,
#else
	.bus_sub_type = MTK_BUS_NUM,
#endif
	.ctl_size = sizeof(struct npu_trans),
	.ops = {
		.drv_register = npu_drv_register,
		.drv_unregister = npu_drv_unregister,
		.drv_profile_dump = npu_drv_profile_dump,
		.drv_create_tunnel = npu_drv_create_tunnel,
	},
};

static int __init en_npu_init(void)
{
	return mtk_bus_driver_register(&npu_bus);
}

static void __exit en_npu_exit(void)
{
	mtk_bus_driver_unregister(&npu_bus);
}

module_init(en_npu_init);
module_exit(en_npu_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Mediatek NPU BUS Driver");
