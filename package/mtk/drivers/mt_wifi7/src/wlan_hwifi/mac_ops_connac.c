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
#include <wlan_tr.h>
#include "mac_ops.h"
#include "hw_ops.h"
#include "mac_if.h"

int
mtk_ge_add_phy(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy)
{
	int ret;

	hw_phy->hw_dev = dev;
	ret = mtk_hwctrl_phy_alloc(dev, hw_phy);
	if (ret)
		return -EINVAL;

	hw_phy->msdu_rx_blk = kmalloc(sizeof(struct _RX_BLK) * 32, GFP_ATOMIC);
	return mtk_wsys_register_phy(hw_phy);

}

int
mtk_ge_remove_phy(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy)
{
	kfree(hw_phy->msdu_rx_blk);
	mtk_wsys_unregister_phy(hw_phy);
	mtk_hwctrl_phy_free(dev, hw_phy);
	return 0;
}

int
mtk_ge_tx_check_resource(struct mtk_hw_phy *phy, struct mtk_hw_txq *txq)
{
	struct mtk_bus_trans *trans = to_bus_trans(txq->txq_trans);
	enum TX_RESOURCE_TYPE tx_resource_type = *(enum TX_RESOURCE_TYPE *)txq->tx_pkt_info;
	int ret;

	/* check bus resource */
	if (!mtk_bus_dma_request_tx(trans, txq->txq)) {
		dbg_bus_tx_err_inc(trans);
		return 0;
	}

	/* check token */
	if (tx_resource_type == TX_BMC_RESOURCE) {
		/* Return value meaning
		 * -> Above 0, check BMC tk_mgmt's resource
		 * -> Otherwise, BMC tk_mgmt not supported or full, and need use CMM tk_mgmt
		 */
		ret =  mtk_tk_bmc_check_resource(phy);
		if (ret > 0)
			return ret;
	} else if (tx_resource_type == TX_HIGHPRIO_RESOURCE) {
		/* Return value meaning
		 * -> Above 0, use rsv resource
		 * -> Otherwise, rsv resource not supported or full, and need use CMM tk_mgmt
		 */
		ret = mtk_tk_rsv_check_resource(phy);
		if (ret > 0)
			return ret;
	}

	return mtk_tk_check_resource(phy);
}

static struct mtk_tk_entry *
mtk_ge_request_txres(struct mtk_hw_phy *phy, struct _TX_BLK *tx_blk)
{
	struct mtk_hw_dev *dev = phy->hw_dev;
	struct mtk_tk_entry *tk_entry;
	struct mtk_tk_res_ctl *tk_res_ctl = phy->tk_res_ctl;
	struct _QUEUE_ENTRY *c_entry = tx_blk->TxPacketList.Head;
	struct _QUEUE_ENTRY *n_entry;

	if (tx_blk->TxResourceType == TX_BMC_RESOURCE) {
		tk_entry = mtk_tk_bmc_request_entry(dev);
		if (!IS_ERR_OR_NULL(tk_entry))
			goto req_done;
	} else if (tx_blk->TxResourceType == TX_HIGHPRIO_RESOURCE) {
		tk_entry = mtk_tk_rsv_request_entry(phy);
		if (!IS_ERR_OR_NULL(tk_entry))
			goto req_done;
	}

	tk_entry = mtk_tk_request_entry(dev);

req_done:
	if (!tk_entry)
		return NULL;

	atomic_inc(&tk_res_ctl->used);
	tk_entry->band_idx = phy->band_idx;

	do {
		n_entry = QUEUE_GET_NEXT_ENTRY(c_entry);
		__skb_queue_tail(&tk_entry->tx_q,
				(struct sk_buff *)c_entry);
		c_entry = n_entry;
	} while (n_entry);

	return tk_entry;
}

/**
 * mtk_ge_tx_data - TX data frame from hw_txq to bus
 *
 * This function do tx frame procedure if tx fail the free skb
 * in here.
 *
 * @dev:
 * @txq
 * Return: 0 success, otherwise erro code
 */
int
mtk_ge_tx_data(struct mtk_hw_phy *phy, struct mtk_hw_txq *txq)
{
	struct mtk_hw_dev *dev = phy->hw_dev;
	struct mtk_tk_entry *tk_entry = NULL;
	struct mtk_bus_tx_info _tx_bus_info = {0}, *tx_bus_info = &_tx_bus_info;
	struct _TX_BLK *tx_blk;
	int ret = -ENOMEM;

	if (!txq || !txq->sta) {
		dev_err(dev->dev, "%s(): txq or sta err!\n", __func__);
		ret = -EINVAL;
		goto err_null;
	}

	tx_blk = (struct _TX_BLK *)txq->tx_pkt_info;

	tk_entry = mtk_ge_request_txres(phy, tx_blk);
	if (!tk_entry)
		goto err_no_txres;

	tk_entry->sta = txq->sta;

	/*cache invalidate*/
	dma_sync_single_for_cpu(dev->dev, tk_entry->dma_addr,
		tk_entry->fbuf_size, DMA_TO_DEVICE);

	mt_rcu_read_lock();
	mtk_hw_write_txd(dev, txq->sta, tk_entry, txq->tx_pkt_info);
	ret = mtk_hw_write_txp(dev, tk_entry, txq->tx_pkt_info);
	mt_rcu_read_unlock();

	if (ret) {
		tk_entry->rdy = true;
		goto err_write_txp;
	}

	/*cache flush*/
	dma_sync_single_for_device(dev->dev, tk_entry->dma_addr,
		tk_entry->fbuf_size, DMA_TO_DEVICE);

	tx_bus_info->tk_entries = tk_entry;
	tx_bus_info->txq = txq->txq;
	tx_bus_info->txq_trans = txq->txq_trans;
	tx_bus_info->cnt = 1;

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS))
		set_bit(BUS_TX_DATA, &tx_bus_info->flags);

	if (TX_BLK_TEST_FLAG2(tx_blk, fTX_SW_PATH))
		set_bit(BUS_TX_SW, &tx_bus_info->flags);

	ret = mtk_bus_dma_tx_data_queue(dev->bus_trans, tx_bus_info);
	if (ret) {
		dev_err(dev->dev, "%s():ret=%d, tx bus error, free tkid %d!\n",
			__func__, ret, tk_entry->sid.idx);
		dbg_tk_entry_pkt_null_err_inc(tk_entry->tk_mgmt);
		goto err_tx_data_queue;
	}

	return 0;
err_tx_data_queue:
err_write_txp:
	mtk_tk_release_entry(dev, tk_entry);
err_no_txres:
err_null:
	return ret;

}
int
mtk_ge_set_dma_tk(struct mtk_hw_phy *phy, bool enable)
{
	struct mtk_hw_dev *dev = phy->hw_dev;
	int ret = 0;

	if (enable) {
		mtk_bus_stop_device(dev->bus_trans);
		ret = mtk_tk_init(dev);
		if (ret)
			goto err_tk_init;

		ret = mtk_bus_preinit_device(dev->bus_trans);
		if (ret)
			goto err_bus_preinit;

		ret = mtk_bus_init_device(dev->bus_trans);
		if (ret)
			goto err_bus_init;

		ret = mtk_bus_start_device(dev->bus_trans);
		if (ret)
			goto err_bus_start;

		ret = mtk_bus_init_after_fwdl(dev->bus_trans);
		if (ret)
			goto err_bus_init_after_fwdl;
	} else {
		mtk_bus_stop_device(dev->bus_trans);
		mtk_bus_exit_device(dev->bus_trans);
		mtk_tk_exit(dev);
	}

	return ret;
err_bus_init_after_fwdl:
	mtk_bus_stop_device(dev->bus_trans);
err_bus_start:
	mtk_bus_exit_device(dev->bus_trans);
err_bus_init:
err_bus_preinit:
	mtk_tk_exit(dev);
err_tk_init:
	return ret;
}

int
mtk_ge_set_dma_sw_rxq(struct mtk_hw_phy *phy, bool enable)
{
	struct mtk_hw_dev *dev = phy->hw_dev;

	if (enable)
		return mtk_bus_dma_init_sw_rxq(dev->bus_trans);
	return mtk_bus_dma_exit_sw_rxq(dev->bus_trans);
}

int
mtk_ge_hw_reset(struct mtk_hw_dev *dev)
{
	return mtk_hw_reset(dev);
}

int
mtk_ge_set_traffic(struct mtk_hw_dev *dev, bool enable)
{
	int ret = 0;

	if (enable)
		ret = mtk_bus_start_traffic(dev->bus_trans);
	else
		ret = mtk_bus_stop_traffic(dev->bus_trans);

	return ret;
}

int
mtk_ge_get_pc_value(struct mtk_hw_dev *dev, u32 *pc_value)
{
	return mtk_hw_get_pc_value(dev, pc_value);
}

struct mtk_hw_bss *
mtk_ge_get_hw_bss(struct mtk_hw_dev *dev, u8 bss_idx)
{
	return mtk_wsys_bss_get(dev, bss_idx);
}
