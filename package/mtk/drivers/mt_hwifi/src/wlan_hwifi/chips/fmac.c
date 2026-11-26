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

#include "core.h"
#include "mac.h"
#include "fmac.h"
#include "main.h"

#define to_rssi(field, rxv) ((field_get(field, rxv) - 220) / 2)

static int
fmac_rxd_basic_info(struct mtk_hw_dev *dev, struct sk_buff *skb)
{
	struct mtk_rx_blk *status = (struct mtk_rx_blk *) skb->cb;
	__le32 *rxd = (__le32 *)skb->data;
	u32 rxd1 = le32_to_cpu(rxd[1]);
	u32 rxd2 = le32_to_cpu(rxd[2]);
	u32 rxd3 = le32_to_cpu(rxd[3]);
	enum rx_pkt_type type;
	enum rx_pkt_type sw_type;

	type = field_get(MT_RXD0_PKT_TYPE, le32_to_cpu(rxd[0]));
	sw_type = field_get(RXD_SW_PKT_TYPE_MASK, le32_to_cpu(rxd[0]));
	if (type != PKT_TYPE_NORMAL &&
			((sw_type & RXD_SW_PKT_TYPE_BITMAP) !=
			RXD_SW_PKT_TYPE_FRAME))
		return -EINVAL;

	if (rxd1 & MT_RXD1_NORMAL_FCS_ERR)
		return -EINVAL;

	status->band_idx = field_get(MT_RXD1_NORMAL_BAND_IDX, rxd1);
	status->unicast =
		(field_get(MT_RXD3_NORMAL_ADDR_TYPE, rxd3) == MT_RXD3_NORMAL_U2M);
	status->wcid = field_get(MT_RXD1_NORMAL_WLAN_IDX, rxd1);
	status->bssidx = field_get(MT_RXD2_NORMAL_BSSID, rxd2);
	status->ch_freq = field_get(MT_RXD3_NORMAL_CH_FREQ, rxd3);
	return 0;
}

static int
fmac_rxd_parser(struct mtk_hw_dev *dev, struct sk_buff *skb)
{
	struct mtk_rx_blk *status = (struct mtk_rx_blk *) skb->cb;
	struct fmac_rxv rxv = {0};
	__le32 *rxd = (__le32 *)skb->data;
	__le32 *orig = rxd;
	u32 rxd1 = le32_to_cpu(rxd[1]);
	u32 rxd2 = le32_to_cpu(rxd[2]);
	u32 rxd3 = le32_to_cpu(rxd[3]);
	u8 remove_pad;

	if (field_get(MT_RXD0_PKT_TYPE, le32_to_cpu(rxd[0])) != PKT_TYPE_NORMAL)
		return -EINVAL;

	memset(status, 0, sizeof(*status));
	status->band_idx = field_get(MT_RXD1_NORMAL_BAND_IDX, rxd1);
	status->unicast =
		(field_get(MT_RXD3_NORMAL_ADDR_TYPE, rxd3) == MT_RXD3_NORMAL_U2M);
	status->wcid = field_get(MT_RXD1_NORMAL_WLAN_IDX, rxd1);
	status->bssidx = field_get(MT_RXD2_NORMAL_BSSID, rxd2);
	status->ch_freq = field_get(MT_RXD3_NORMAL_CH_FREQ, rxd3);

	if (rxd1 & MT_RXD1_NORMAL_FCS_ERR) {
		status->fcs_err = true;
		return -EINVAL;
	}

	if (rxd1 & MT_RXD1_NORMAL_TKIP_MIC_ERR)
		status->tkip_mic_err = true;

	if (field_get(MT_RXD1_NORMAL_SEC_MODE, rxd1) != 0 &&
	    !(rxd1 & (MT_RXD1_NORMAL_CLM | MT_RXD1_NORMAL_CM))) {
		status->cm = true;
	}

	if (rxd2 & MT_RXD2_NORMAL_NON_AMPDU)
		status->non_ampdu = true;

	remove_pad = field_get(MT_RXD2_NORMAL_HDR_OFFSET, rxd2);

	if (rxd2 & MT_RXD2_NORMAL_MAX_LEN_ERROR)
		return -EINVAL;

	rxd += 6;
	if (rxd1 & MT_RXD1_NORMAL_GROUP_4) {
		rxd += 4;
		if ((u8 *)rxd - skb->data >= skb->len)
			return -EINVAL;
	}

	if (rxd1 & MT_RXD1_NORMAL_GROUP_1) {

		rxd += 4;
		if ((u8 *)rxd - skb->data >= skb->len)
			return -EINVAL;
	}

	if (rxd1 & MT_RXD1_NORMAL_GROUP_2) {
		rxd += 2;
		if ((u8 *)rxd - skb->data >= skb->len)
			return -EINVAL;
	}

	/* RXD Group 3 - P-RXV */
	if (rxd1 & MT_RXD1_NORMAL_GROUP_3) {
		memcpy(rxv.v, rxd, sizeof(rxv.v));

		rxd += 2;
		if ((u8 *)rxd - skb->data >= skb->len)
			return -EINVAL;

		if (rxv.v[0] & MT_PRXV_HT_AD_CODE)
			status->ldpc = true;

		status->chain_signal[0] = to_rssi(MT_PRXV_RCPI0, rxv.v[1]);
		status->chain_signal[1] = to_rssi(MT_PRXV_RCPI1, rxv.v[1]);
		status->chain_signal[2] = to_rssi(MT_PRXV_RCPI2, rxv.v[1]);
		status->chain_signal[3] = to_rssi(MT_PRXV_RCPI3, rxv.v[1]);
		status->signal = status->chain_signal[0];

		/* RXD Group 5 - C-RXV */
		if (rxd1 & MT_RXD1_NORMAL_GROUP_5) {
			status->stbc = field_get(MT_CRXV_HT_STBC, rxv.v[2]);
			status->gi = field_get(MT_CRXV_HT_SHORT_GI, rxv.v[2]);

			rxd += 18;
			if ((u8 *)rxd - skb->data >= skb->len)
				return -EINVAL;

			status->tx_rate = field_get(MT_PRXV_TX_RATE, rxv.v[0]);
			status->tx_mode = field_get(MT_CRXV_TX_MODE, rxv.v[2]);
			status->nss = field_get(MT_PRXV_NSTS, rxv.v[0]) + 1;
			status->bw = field_get(MT_CRXV_FRAME_MODE, rxv.v[2]);
		}
	}

	skb_pull(skb, (u8 *)rxd - skb->data + 2 * remove_pad);

	status->key_id = field_get(MT_RXD1_NORMAL_KEY_ID, rxd1);
	if (0)
		dev->dbg_ops->rxd_info(orig);
	return 0;

}

static void
fmac_process_txs(struct mtk_hw_dev *dev, void *data)
{
	__le32 *txs_data = data;
	u32 txs;
	u16 wcidx;
	u8 pid;
	bool me, re, le, bae;
	bool fr, bipe, tsh, tsm, ampdu;
	u8 bw, txrate, tid, txsf, psf;


	txs = le32_to_cpu(txs_data[3]);
	pid = field_get(MT_TXS3_PID, txs);
	txs = le32_to_cpu(txs_data[2]);
	wcidx = field_get(MT_TXS2_WCID, txs);
	txs = le32_to_cpu(txs_data[0]);
	bae = txs & MT_TXS0_BA_ERROR;
	bipe = txs & MT_TXS0_BIP_ERROR;
	tsh = txs & MT_TXS0_TX_STATUS_HOST;
	tsm = txs & MT_TXS0_TX_STATUS_MCU;
	fr = txs & MT_TXS0_FIXED_RATE;
	bw = field_get(MT_TXS0_BW, txs);
	txrate = field_get(MT_TXS0_TX_RATE, txs);
	tid = field_get(MT_TXS0_TID, txs);
	txsf = field_get(MT_TXS0_TXS_FORMAT, txs);
	psf =  field_get(MT_TXS0_PS_FLAG, txs);
	me = txs & MT_TXS0_ACK_TIMEOUT;
	re = txs & MT_TXS0_RTS_TIMEOUT;
	le = txs & MT_TXS0_QUEUE_TIMEOUT;
	ampdu = txs & MT_TXS0_AMPDU;

	hwf_printk("%s(): pid: %d, wcid: %d, format: %d, toh: %d, tom: %d\n",
		__func__, pid, wcidx, txsf, tsh, tsm);

	hwf_printk("%s(): ba err: %d, bip err: %d, me: %d, re: %d, le: %d\n",
			__func__, bae, bipe, me, re, le);

	hwf_printk("%s(): fixrate: %d, txrate: %d, bw: %d, tid: %d, psf: %d, ampdu: %d\n",
		__func__, fr, txrate, bw, tid, psf, ampdu);

}

static inline void
dump_free_notify_info(__le32 info)
{
	u32 val = le32_to_cpu(info);
	u8 dcm, nsts, txmode, gi, rate, bw, pair;
	u16 wcid, qid;

	pair = field_get(MT_TX_FREE_PAIR, val);
	qid = field_get(MT_TX_FREE_QID, val);
	wcid = field_get(MT_TX_FREE_WLAN_ID, val);
	dcm = field_get(MT_TX_FREE_DCM, val);
	nsts = field_get(MT_TX_FREE_NSTS, val);
	txmode = field_get(MT_TX_FREE_TXMODE, val);
	gi = field_get(MT_TX_FREE_GI, val);
	rate = field_get(MT_TX_FREE_RATE, val);
	bw = field_get(MT_TX_FREE_BW, val);

	pr_info("pair: %d, qid: %d, wcid: %d, dcm: %d, nsts: %d, txmode: %d, gi: %d, rate: %d, bw: %d\n",
		pair, qid, wcid, dcm, nsts, txmode, gi, rate, bw);
}

static void
fmac_tx_free_notify_v0(struct mtk_hw_dev *dev, struct mtk_bus_trans *trans,
	struct sk_buff *skb)
{
	struct fmac_tx_free_v0 *free = (struct fmac_tx_free_v0 *)skb->data;
	u16 rx_len = field_get(MT_TX_FREE_RX_BTYE, le32_to_cpu(free->dw0));
	u16 count = field_get(MT_TX_FREE_MSDU_ID_CNT_V0, le32_to_cpu(free->dw0));
	u8 i;

	rx_len -= 8;
	if ((count * 2) != rx_len) {
		dev_err(dev->dev, "%s(): tokenCnt(%d) and rxByteCnt(%d) mismatch!\n", __func__, count, rx_len);
		goto end;
	}

	for (i = 0 ; i < count; i++) {
		u32 token;

		token = le16_to_cpu(free->token[i]);
		mtk_hwifi_free_tx(dev, token, WCID_MASK);
	}
end:
	return;
}

static void
fmac_tx_free_notify_v3(struct mtk_hw_dev *dev, struct mtk_bus_trans *trans,
	struct sk_buff *skb)
{
	struct fmac_tx_free *free = (struct fmac_tx_free *)skb->data;
	u8 count;
	u16 wcid, qid;
	u8 i;

	count = field_get(MT_TX_FREE_MSDU_ID_CNT, le32_to_cpu(free->dw0));

	qid = field_get(MT_TX_FREE_QID, le32_to_cpu(free->info));
	wcid = field_get(MT_TX_FREE_WLAN_ID, le32_to_cpu(free->info));
	if (0)
		dump_free_notify_info(free->info);

	for (i = 0; i < count; i++) {
		u32 token;

		token = field_get(MT_TX_FREE_TOKEN_ID, le32_to_cpu(free->token[i]));
		mtk_hwifi_free_tx(dev, token, wcid);
	}
}

static void
fmac_tx_free_notify(struct mtk_bus_trans *trans, struct sk_buff *skb)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct fmac_tx_free *free = (struct fmac_tx_free *)skb->data;
	u8 ver;

	ver = field_get(MT_TX_FREE_VER, le32_to_cpu(free->dw1));

	switch(ver) {
	case MT_TX_FREE_VER_0:
		fmac_tx_free_notify_v0(dev, trans, skb);
		break;
	case MT_TX_FREE_VER_3:
		fmac_tx_free_notify_v3(dev, trans, skb);
		break;
	default:
		dev_err(dev->dev, "%s(): rx wrong free notify version: %d\n", __func__, ver);
		dbg_bus_free_notify_err_inc(trans);
		break;
	}

	dbg_bus_free_notify_inc(trans);
	mtk_bus_dma_tx_free(trans, skb);
	dev_kfree_skb(skb);
}

static void
fmac_rx_txs(struct mtk_bus_trans *trans, struct sk_buff *skb)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	__le32 *rxd = (__le32 *)skb->data;
	__le32 *end = (__le32 *)&skb->data[skb->len];

	for (rxd += 2; rxd + 8 <= end; rxd += 8)
		fmac_process_txs(dev, rxd);
	dev_kfree_skb(skb);
}

static void
fmac_rx_skb(struct mtk_hw_dev *dev, struct mtk_bus_trans *trans,
	struct sk_buff *skb)
{
	struct mtk_bus_rx_info *rx_info = (struct mtk_bus_rx_info *)skb->cb;
	int ret = 0;

	if (test_bit(HWIFI_FLAG_RXD_PARSER, &dev->flags))
		ret = fmac_rxd_parser(dev, skb);
	else
		ret = fmac_rxd_basic_info(dev, skb);

	if (ret)
		goto err;

	ret = mtk_hwifi_queue_rx_data(dev, skb);
	if (ret)
		goto err;

	return;
err:
	dbg_bus_rx_drop_inc(trans, rx_info);
	dev_kfree_skb(skb);
}

static void
fmac_rx_event(struct mtk_hw_dev *dev, struct mtk_bus_trans *trans,
	struct sk_buff *skb, u32 mcu_type)
{
	enum rx_pkt_type type;
	enum rx_pkt_type sw_type;
	__le32 *rxd = (__le32 *)skb->data;
	struct mtk_bus_rx_info *rx_info = (struct mtk_bus_rx_info *)skb->cb;

	type = field_get(MT_RXD0_PKT_TYPE, le32_to_cpu(rxd[0]));
	sw_type = field_get(RXD_SW_PKT_TYPE_MASK, le32_to_cpu(rxd[0]));
	if ((sw_type & RXD_SW_PKT_TYPE_BITMAP) == RXD_SW_PKT_TYPE_FRAME)
		type = PKT_TYPE_NORMAL;

	switch (type) {
	case PKT_TYPE_TXRX_NOTIFY:
	case PKT_TYPE_TXRX_NOTIFY_V0:
		fmac_tx_free_notify(trans, skb);
		break;
	case PKT_TYPE_RX_EVENT:
		mtk_mcu_rx_event(dev, skb, mcu_type);
		break;
	case PKT_TYPE_TXS:
		fmac_rx_txs(trans, skb);
		break;
	case PKT_TYPE_NORMAL:
		fmac_rx_skb(dev, trans, skb);
		break;
	default:
		dbg_bus_rx_drop_inc(trans, rx_info);
		dev_kfree_skb(skb);
		break;
	}
}

static void
fmac_skb_unmap_txp(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry)
{

	struct hif_txp *txp = (struct hif_txp *)(tk_entry->txd_ptr + MT_TXD_SIZE);
	dma_addr_t dma_addr;
	u32 len;
	int i = 0;

	for (i = 0 ; i < txp->nbuf; i++) {
		dma_addr = le32_to_cpu(txp->buf[i]);
#ifdef CONFIG_HWIFI_64BIT_SUPPORT
		dma_addr |= ((dma_addr_t)GET_FIELD(HIF_TXP_BUF_PTR_H,
				le16_to_cpu(txp->len[i])) << DMA_ADDR_H_SHIFT);
#endif
		len = GET_FIELD(HIF_TXP_BUF_LEN, le16_to_cpu(txp->len[i]));
		dma_unmap_single(dev->dev, dma_addr, len, DMA_TO_DEVICE);
	}
}

/*TBD need to refine*/
static int
fmac_write_mcu_txd(struct mtk_hw_dev *dev, struct mtk_mcu_txblk *mcu_txblk)
{
	struct sk_buff *skb = mcu_txblk->skb;
	struct mcu_txd *mcu_txd;
	u8 pkt_fmt, qidx;
	__le32 *txd;
	u32 val;

	mcu_txd = (struct mcu_txd *)skb_push(skb, sizeof(*mcu_txd));
	qidx = MT_TX_MCU_PORT_RX_Q0;
	pkt_fmt = MT_TX_TYPE_CMD;

	txd = mcu_txd->txd;

	val = field_prep(MT_TXD0_TX_BYTES, skb->len) |
	      field_prep(MT_TXD0_PKT_FMT, pkt_fmt) |
	      field_prep(MT_TXD0_Q_IDX, qidx);
	txd[0] = cpu_to_le32(val);

	val = MT_TXD1_LONG_FORMAT |
	      field_prep(MT_TXD1_HDR_FORMAT, MT_HDR_FORMAT_CMD);
	txd[1] = cpu_to_le32(val);

	mcu_txd->len = cpu_to_le16(skb->len - sizeof(mcu_txd->txd));
	mcu_txd->pq_id = cpu_to_le16(MCU_PQ_ID(MT_TX_PORT_IDX_MCU, qidx));
	mcu_txd->pkt_type = MCU_PKT_ID;
	mcu_txd->seq = mcu_txblk->seq;
	mcu_txd->cid = mcu_txblk->cmd;
	mcu_txd->ext_cid = mcu_txblk->ext_cmd;
	mcu_txd->set_query = MCU_Q_NA;
	mcu_txd->ext_cid_ack = (mcu_txblk->ack || mcu_txblk->wait_resp) ? 1 : 0;

	if (mcu_txblk->cmd == MCU_CMD_EXT_CID || mcu_txblk->cmd == MCU_CMD_WA) {
		/* do not use Q_SET for efuse */
		if (mcu_txblk->action == MCU_ACT_QUERY)
			mcu_txd->set_query = MCU_Q_QUERY;
		else
			mcu_txd->set_query = MCU_Q_SET;
	}

	mcu_txd->s2d_index = mtk_mcu_dest_2_s2d(mcu_txblk->dest);

	mtk_dbg(MTK_MCU, "%s:len=%d,pid=0x%x,pt=0x%x,cid=0x%x,ext_cid=0x%x\n",
		__func__, mcu_txd->len, mcu_txd->pq_id, mcu_txd->pkt_type,
		mcu_txd->cid, mcu_txd->ext_cid);
	mtk_dbg(MTK_MCU, "%s:seq=%d,sq=%d,ack=%d,s2d=%d,path=%d\n",
		__func__, mcu_txd->seq, mcu_txd->set_query,
		mcu_txd->ext_cid_ack, mcu_txd->s2d_index, mcu_txblk->path);

	return 0;
}

static int
fmac_write_mcu_txd_uni_cmd(struct mtk_hw_dev *dev,
	struct mtk_mcu_txblk *mcu_txblk)
{
	struct sk_buff *skb = mcu_txblk->skb;
	struct mcu_txd_uni_cmd *mcu_txd;
	u8 pkt_fmt, qidx;
	__le32 *txd;
	u32 val;

	mcu_txd = (struct mcu_txd_uni_cmd *)skb_push(skb, sizeof(*mcu_txd));
	qidx = MT_TX_MCU_PORT_RX_Q0;
	pkt_fmt = MT_TX_TYPE_CMD;

	txd = mcu_txd->txd;

	val = field_prep(MT_TXD0_TX_BYTES, skb->len) |
	      field_prep(MT_TXD0_PKT_FMT, pkt_fmt) |
	      field_prep(MT_TXD0_Q_IDX, qidx);
	txd[0] = cpu_to_le32(val);

	val = MT_TXD1_LONG_FORMAT |
	      field_prep(MT_TXD1_HDR_FORMAT, MT_HDR_FORMAT_CMD);
	txd[1] = cpu_to_le32(val);

	mcu_txd->len = cpu_to_le16(skb->len - sizeof(mcu_txd->txd));
	mcu_txd->cid = mcu_txblk->cmd;
	mcu_txd->pkt_type = MCU_PKT_ID;
	mcu_txd->frag = (((mcu_txblk->frag_num & 0xf) << 4) |
		(mcu_txblk->frag_total_num & 0xf));
	mcu_txd->seq = mcu_txblk->seq;
	mcu_txd->check_sum = 0;
	mcu_txd->s2d_index = mtk_mcu_dest_2_s2d(mcu_txblk->dest);
	mcu_txd->option = 0;
	mcu_txd->option |= (mcu_txblk->uni_cmd ?
		MCU_UNI_CMD_OPT_BIT_1_UNI_CMD : 0);
	mcu_txd->option |= ((mcu_txblk->ack || mcu_txblk->wait_resp) ?
		MCU_UNI_CMD_OPT_BIT_0_ACK : 0);
	/* do not use Q_SET for efuse */
	mcu_txd->option |= (mcu_txblk->action == MCU_ACT_QUERY ?
		0 : MCU_UNI_CMD_OPT_BIT_2_SET_QUERY);

	dev_info(dev->dev, "%s:len=%d,pt=0x%x,cid=0x%x,seq=%d,frag=0x%x",
		__func__, mcu_txd->len, mcu_txd->pkt_type, mcu_txd->cid,
		mcu_txd->seq, mcu_txd->frag);
	dev_info(dev->dev, "%s:cs=%d,s2d=%d,%d,path=%d,option=0x%x\n",
		__func__, mcu_txd->check_sum, mcu_txd->s2d_index,
		mcu_txblk->dest, mcu_txblk->path, mcu_txd->option);

	return 0;
}

void
fmac_hif_txd_init(u8 *fbuf, u32 pkt_pa, u32 tkid)
{
	__le32 *txd = (__le32 *) fbuf;
	struct hif_txp *txp;
	u32 val;

	memset(txd, 0, MT_TXD_SIZE);
	val = field_prep(MT_TXD0_PKT_FMT, MT_TX_TYPE_CT);
	txd[0] = cpu_to_le32(val);
	val = MT_TXD1_LONG_FORMAT;
	txd[1] = cpu_to_le32(val);

	txp = (struct hif_txp *)(fbuf + MT_TXD_SIZE);
	memset(txp, 0, sizeof(*txp));
	txp->flags = 0;
	txp->nbuf = 1;
	txp->buf[0] = cpu_to_le32(SET_FIELD(HIF_TXP_BUF_PTR,pkt_pa));
	txp->len[0] = 0;
	txp->token = cpu_to_le16(tkid);

}
EXPORT_SYMBOL(fmac_hif_txd_init);

void
fmac_hif_txd_v1_init(u8 *fbuf, dma_addr_t pkt_pa, u32 tkid, u8 src)
{
	__le32 *txd = (__le32 *) fbuf;
	struct hif_txp *txp;
	u32 val;

	memset(txd, 0, MT_TXD_SIZE);
	val = field_prep(MT_TXD0_PKT_FMT, MT_TX_TYPE_CT);
	txd[0] = cpu_to_le32(val);
	val = MT_TXD1_LONG_FORMAT;
	txd[1] = cpu_to_le32(val);
	val = field_prep(MT_TXD6_SRC, src);
	txd[6] = cpu_to_le32(val);

	txp = (struct hif_txp *)(fbuf + MT_TXD_SIZE);
	memset(txp, 0, sizeof(*txp));
	txp->flags = 0;
	txp->nbuf = 1;
	txp->buf[0] = cpu_to_le32(SET_FIELD(HIF_TXP_BUF_PTR, pkt_pa));
	txp->len[0] = 0;
	txp->token = cpu_to_le16(tkid);

}
EXPORT_SYMBOL(fmac_hif_txd_v1_init);

/*
 *
 */
void
fmac_ops_init(struct mtk_hw_dev *dev)
{
	struct mtk_hw_ops *ops = &dev->hw_ops;

	ops->rx_pkt = fmac_rx_skb;
	ops->rx_event = fmac_rx_event;
	ops->write_txd = fmac_write_txd;
	ops->write_txp = fmac_write_txp;
	ops->write_mcu_txd = fmac_write_mcu_txd;
	ops->write_mcu_txd_uni_cmd = fmac_write_mcu_txd_uni_cmd;
	ops->skb_unmap_txp = fmac_skb_unmap_txp;
}
EXPORT_SYMBOL(fmac_ops_init);
