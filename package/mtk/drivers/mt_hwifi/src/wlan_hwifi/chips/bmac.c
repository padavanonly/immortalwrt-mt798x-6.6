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
#include "bmac.h"
#include "main.h"

static void
bmac_rx_event(struct mtk_hw_dev *dev, struct mtk_bus_trans *trans,
	struct sk_buff *skb, u32 mcu_type)
{
	enum rx_pkt_type type;
	enum rx_pkt_type sw_type;
	__le32 *rxd = (__le32 *)skb->data;

	type = GET_FIELD(WF_RX_DESCRIPTOR_PACKET_TYPE,
					le32_to_cpu(rxd[0]));
	sw_type = field_get(RXD_SW_PKT_TYPE_MASK, le32_to_cpu(rxd[0]));
	if ((sw_type & RXD_SW_PKT_TYPE_BITMAP) == RXD_SW_PKT_TYPE_FRAME)
		type = PKT_TYPE_NORMAL;

	switch (type) {
	case PKT_TYPE_TXRX_NOTIFY:
	case PKT_TYPE_TXRX_NOTIFY_V0:
		bmac_tx_free_notify(trans, skb);
		break;
	case PKT_TYPE_RX_EVENT:
		mtk_mcu_rx_event(dev, skb, mcu_type);
		break;
	case PKT_TYPE_TXS:
		bmac_rx_txs(trans, skb);
		break;
	case PKT_TYPE_NORMAL:
		bmac_rx_data(trans, skb);
		break;
	case PKT_TYPE_RX_SDO_EVENT:
		bmac_rx_sdo_event(trans, skb);
		break;
	default:
		dbg_bus_rx_err_inc(trans);
		dev_kfree_skb(skb);
		break;
	}
}

void
bmac_rx_skb(struct mtk_hw_dev *dev, struct mtk_bus_trans *trans,
			struct sk_buff *skb)
{
	enum rx_pkt_type type;
	__le32 *rxd = (__le32 *)skb->data;

#ifdef CONFIG_HWIFI_DBG
	bmac_dump_rxd(rxd);
#endif

	type = GET_FIELD(WF_RX_DESCRIPTOR_PACKET_TYPE,
					le32_to_cpu(rxd[0]));

	switch (type) {
	case PKT_TYPE_NORMAL:
	case PKT_TYPE_RX_DUP_RFB:
		bmac_rx_data(trans, skb);
		break;
	case PKT_TYPE_RX_ICS:
		bmac_rx_ics(trans, skb);
		break;
	default:
		dbg_bus_rx_err_inc(trans);
		dev_kfree_skb(skb);
		break;
	}
}

static int
bmac_write_mcu_txd(struct mtk_hw_dev *dev, struct mtk_mcu_txblk *mcu_txblk)
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

	val = SET_FIELD(WF_TX_DESCRIPTOR_TX_BYTE_COUNT, skb->len) |
		SET_FIELD(WF_TX_DESCRIPTOR_PKT_FT, pkt_fmt) |
		SET_FIELD(WF_TX_DESCRIPTOR_Q_IDX, qidx);
	txd[0] = cpu_to_le32(val);

	val = SET_FIELD(WF_TX_DESCRIPTOR_HF, MT_HDR_FORMAT_CMD);
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

	mtk_dbg(MTK_MCU, "%s:len=%d,pid=0x%x,pt=0x%x,cid=0x%x,ext_cid=0x%x,\n",
		__func__, mcu_txd->len, mcu_txd->pq_id, mcu_txd->pkt_type,
		mcu_txd->cid, mcu_txd->ext_cid);
	mtk_dbg(MTK_MCU, "%s:seq=%d,sq=%d,ack=%d,s2d=%d,path=%d\n",
		__func__, mcu_txd->seq, mcu_txd->set_query,
		mcu_txd->ext_cid_ack, mcu_txd->s2d_index, mcu_txblk->path);

	return 0;
}

static int
bmac_write_mcu_txd_uni_cmd(struct mtk_hw_dev *dev, struct mtk_mcu_txblk *mcu_txblk)
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

	val = SET_FIELD(WF_TX_DESCRIPTOR_TX_BYTE_COUNT, skb->len) |
		SET_FIELD(WF_TX_DESCRIPTOR_PKT_FT, pkt_fmt) |
		SET_FIELD(WF_TX_DESCRIPTOR_Q_IDX, qidx);
	txd[0] = cpu_to_le32(val);

	val = SET_FIELD(WF_TX_DESCRIPTOR_HF, MT_HDR_FORMAT_CMD);
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

	mtk_dbg(MTK_MCU, "%s:len=%d,pt=0x%x,cid=0x%x,seq=%d,frag=0x%x",
		__func__, mcu_txd->len, mcu_txd->pkt_type, mcu_txd->cid,
		mcu_txd->seq, mcu_txd->frag);
	mtk_dbg(MTK_MCU, "%s:cs=%d,s2d=%d,%d,path=%d,option=0x%x\n",
		__func__, mcu_txd->check_sum, mcu_txd->s2d_index,
		mcu_txblk->dest, mcu_txblk->path, mcu_txd->option);

	return 0;
}

void
bmac_skb_unmap(struct mtk_hw_dev *dev, __le32 *buf_info, u8 buf_cnt)
{
	__le32 *_buf_info = buf_info;
	dma_addr_t dma_addr;
	u32 len;
	u8 i;

	for (i = 0; i < buf_cnt; i++) {
		dma_addr = 0;

		if (i % 2 == 0) {
			dma_addr |= ((dma_addr_t)GET_FIELD(HIF_TXP_BUF_PTR0_L,
					le32_to_cpu(*_buf_info)));
			_buf_info++;
			len = GET_FIELD(HIF_TXP_BUF_LEN0, le32_to_cpu(*_buf_info));
#ifdef CONFIG_HWIFI_64BIT_SUPPORT
			dma_addr |= ((dma_addr_t)GET_FIELD(HIF_TXP_BUF_PTR0_H,
					le32_to_cpu(*_buf_info)) << DMA_ADDR_H_SHIFT);
#endif
			dma_unmap_single(dev->dev, dma_addr, len, DMA_TO_DEVICE);
		} else {
			len = GET_FIELD(HIF_TXP_BUF_LEN1, le32_to_cpu(*_buf_info));
#ifdef CONFIG_HWIFI_64BIT_SUPPORT
			dma_addr |= ((dma_addr_t)GET_FIELD(HIF_TXP_BUF_PTR1_H,
					le32_to_cpu(*_buf_info)) << DMA_ADDR_H_SHIFT);
#endif
			_buf_info++;
			dma_addr |= ((dma_addr_t)GET_FIELD(HIF_TXP_BUF_PTR1_L,
					le32_to_cpu(*_buf_info)));
			dma_unmap_single(dev->dev, dma_addr, len, DMA_TO_DEVICE);
			_buf_info++;
		}
	}
}

static void
bmac_skb_unmap_mac_txp(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry)
{
	__le32 *txp = (__le32 *)(tk_entry->txd_ptr + MT_TXD_SIZE);
	dma_addr_t dma_addr;
	u32 len;


	dma_addr = le32_to_cpu(txp[2]);
#ifdef CONFIG_HWIFI_64BIT_SUPPORT
	dma_addr |= ((dma_addr_t)GET_FIELD(MAC_TXP_BUF_PTR0_H,
			le32_to_cpu(txp[3])) << DMA_ADDR_H_SHIFT);
#endif
	len = GET_FIELD(MAC_TXP_BUF_LEN0, le32_to_cpu(txp[3]));
	dma_unmap_single(dev->dev, dma_addr, len, DMA_TO_DEVICE);
}

static void
bmac_skb_unmap_hif_txp(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry)
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

static void
bmac_skb_unmap_hif_txp_v2(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry)
{
	__le32 *txp = (__le32 *)(tk_entry->txd_ptr + MT_TXD_SIZE);
	__le32 *buf_info;
	u8 buf_cnt;

	buf_cnt = GET_FIELD(HIF_TXP_BUF_NUM, le32_to_cpu(txp[1]));
	buf_info = &txp[4];
	bmac_skb_unmap(dev, buf_info, buf_cnt);
}

static void
bmac_skb_unmap_hif_txp_v3(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry)
{
	__le32 *txp = (__le32 *)(tk_entry->txd_ptr + MT_TXD_SIZE);
	__le32 *buf_info;
	u8 buf_cnt;

	buf_cnt = GET_FIELD(HIF_TXP_BUF_NUM, le32_to_cpu(txp[1]));
	buf_info = &txp[5];
	bmac_skb_unmap(dev, buf_info, buf_cnt);
}

static inline void
bmac_skb_unmap_txp(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry)
{
	if (unlikely(!tk_entry->skb_unmap_txp)) {
		dev_err(dev->dev, "%s(): Unmap not exist\n", __func__);
		return;
	}

	tk_entry->skb_unmap_txp(dev, tk_entry);
}

/*
 *
 */
void bmac_ops_init(struct mtk_hw_dev *dev)
{
	struct mtk_hw_ops *ops = &dev->hw_ops;
	struct mtk_chip_drv *chip_drv = dev->chip_drv;

	ops->rx_pkt = bmac_rx_skb;
	ops->rx_event = bmac_rx_event;

	if (chip_drv->hw_caps->mac_cap & BIT(CAP_OFFLOAD_TXD)) {
		ops->write_txd = bmac_write_txd;
		ops->write_txp = bmac_write_txp;
		ops->skb_unmap_txp = bmac_skb_unmap_txp;

		ops->write_mac_txd = bmac_write_mac_txd;
		ops->write_mac_txp = bmac_write_mac_txp;
		ops->skb_unmap_mac_txp = bmac_skb_unmap_mac_txp;

		switch (chip_drv->hw_caps->hif_txd_ver) {
		case HIF_TXD_V3_0:
			ops->write_hif_txd = bmac_write_hif_txd_v3;
			ops->write_hif_txp = bmac_write_hif_txp_v3;
			ops->skb_unmap_hif_txp = bmac_skb_unmap_hif_txp_v3;
			break;
		case HIF_TXD_V2_1:
		case HIF_TXD_V2_0:
			ops->write_hif_txd = bmac_write_hif_txd_v2;
			ops->write_hif_txp = bmac_write_hif_txp_v2;
			ops->skb_unmap_hif_txp = bmac_skb_unmap_hif_txp_v2;
			break;
		default:
			ops->write_hif_txd = bmac_write_hif_txd;
			ops->write_hif_txp = bmac_write_hif_txp;
			ops->skb_unmap_hif_txp = bmac_skb_unmap_hif_txp;
		}

		ops->write_hif_txd_legacy = bmac_write_hif_txd;
		ops->write_hif_txp_legacy = bmac_write_hif_txp;
		ops->skb_unmap_hif_txp_legacy = bmac_skb_unmap_hif_txp;
	} else {
		ops->write_txd = bmac_write_mac_txd;

		if (chip_drv->hw_caps->mac_cap & BIT(CAP_PAO))
			ops->write_txp = bmac_write_mac_txp_pao;
		else
			ops->write_txp = bmac_write_mac_txp;

		ops->skb_unmap_txp = bmac_skb_unmap_mac_txp;
	}

	ops->write_mcu_txd = bmac_write_mcu_txd;
	ops->write_mcu_txd_uni_cmd = bmac_write_mcu_txd_uni_cmd;
}
EXPORT_SYMBOL(bmac_ops_init);

#ifdef CONFIG_HWIFI_DBG
void bmac_dump_txd(__le32 *txd)
{
	u32 dw;

	mtk_dbg_dump(MTK_TXD_DUMP, "txd",
					txd, MT_TXD_SIZE);
	/* dw0 */
	dw = le32_to_cpu(txd[0]);
	mtk_dbg(MTK_TXD, "TX_BYTE_COUNT = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_TX_BYTE_COUNT, dw));
	mtk_dbg(MTK_TXD, "ETHER_TYPE_OFFSET(word) = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_ETHER_TYPE_OFFSET, dw));
	mtk_dbg(MTK_TXD, "PKT_FT = %d%s%s%s%s\n",
			GET_FIELD(WF_TX_DESCRIPTOR_PKT_FT, dw),
			GET_FIELD(WF_TX_DESCRIPTOR_PKT_FT, dw) == 0 ? "(ct)" : "",
			GET_FIELD(WF_TX_DESCRIPTOR_PKT_FT, dw) == 1 ? "(s&f)" : "",
			GET_FIELD(WF_TX_DESCRIPTOR_PKT_FT, dw) == 2 ? "(cmd)" : "",
			GET_FIELD(WF_TX_DESCRIPTOR_PKT_FT, dw) == 3 ? "(redirect)" : "");
	mtk_dbg(MTK_TXD, "Q_IDX = %d%s%s%s\n",
			GET_FIELD(WF_TX_DESCRIPTOR_Q_IDX, dw),
			GET_FIELD(WF_TX_DESCRIPTOR_Q_IDX, dw) == 0x10 ? "(ALTX)" : "",
			GET_FIELD(WF_TX_DESCRIPTOR_Q_IDX, dw) == 0x11 ? "(BMC)" : "",
			GET_FIELD(WF_TX_DESCRIPTOR_Q_IDX, dw) == 0x12 ? "(BCN)" : "");

	/* dw1 */
	dw = le32_to_cpu(txd[1]);
	mtk_dbg(MTK_TXD, "MLD_ID = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_MLD_ID, dw));
	mtk_dbg(MTK_TXD, "TGID = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_TGID, dw));
	mtk_dbg(MTK_TXD, "HF = %d%s%s%s%s\n",
			GET_FIELD(WF_TX_DESCRIPTOR_HF, dw),
			GET_FIELD(WF_TX_DESCRIPTOR_HF, dw) == 0 ? "(eth/802.3)" : "",
			GET_FIELD(WF_TX_DESCRIPTOR_HF, dw) == 1 ? "(cmd)" : "",
			GET_FIELD(WF_TX_DESCRIPTOR_HF, dw) == 2 ? "(802.11)" : "",
			GET_FIELD(WF_TX_DESCRIPTOR_HF, dw) == 3 ? "(802.11 enhanced" : "");
	mtk_dbg(MTK_TXD, "802.11 HEADER_LENGTH = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_HF, dw) == 2 ?
			GET_FIELD(WF_TX_DESCRIPTOR_HEADER_LENGTH, dw) : 0);
	mtk_dbg(MTK_TXD, "MRD = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_HF, dw) == 0 ?
			GET_FIELD(WF_TX_DESCRIPTOR_MRD, dw) : 0);
	mtk_dbg(MTK_TXD, "EOSP = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_HF, dw) == 0 ?
			GET_FIELD(WF_TX_DESCRIPTOR_EOSP, dw) : 0);
	mtk_dbg(MTK_TXD, "AMS = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_HF, dw) == 3 ?
			GET_FIELD(WF_TX_DESCRIPTOR_AMS, dw) : 0);
	mtk_dbg(MTK_TXD, "RMVL = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_HF, dw) == 0 ?
			GET_FIELD(WF_TX_DESCRIPTOR_RMVL, dw) : 0);
	mtk_dbg(MTK_TXD, "VLAN = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_HF, dw) == 0 ?
			GET_FIELD(WF_TX_DESCRIPTOR_VLAN, dw) : 0);
	mtk_dbg(MTK_TXD, "ETYP = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_HF, dw) == 0 ?
			GET_FIELD(WF_TX_DESCRIPTOR_ETYP, dw) : 0);
	mtk_dbg(MTK_TXD, "TID_MGMT_TYPE = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_TID_MGMT_TYPE, dw));
	mtk_dbg(MTK_TXD, "OM = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_OM, dw));
	mtk_dbg(MTK_TXD, "FR = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_FR, dw));

	/* dw2 */
	dw = le32_to_cpu(txd[2]);
	mtk_dbg(MTK_TXD, "SUBTYPE = %d%s%s%s%s\n",
			GET_FIELD(WF_TX_DESCRIPTOR_SUBTYPE, dw),
			(GET_FIELD(WF_TX_DESCRIPTOR_FTYPE, dw) == 0) &&
			(GET_FIELD(WF_TX_DESCRIPTOR_SUBTYPE, dw) == 13) ?
			"(action)" : "",
			(GET_FIELD(WF_TX_DESCRIPTOR_FTYPE, dw) == 1) &&
			(GET_FIELD(WF_TX_DESCRIPTOR_SUBTYPE, dw) == 8) ?
			"(bar)" : "",
			(GET_FIELD(WF_TX_DESCRIPTOR_FTYPE, dw) == 2) &&
			(GET_FIELD(WF_TX_DESCRIPTOR_SUBTYPE, dw) == 4) ?
			"(null)" : "",
			(GET_FIELD(WF_TX_DESCRIPTOR_FTYPE, dw) == 2) &&
			(GET_FIELD(WF_TX_DESCRIPTOR_SUBTYPE, dw) == 12) ?
			"(qos null)" : "");

	mtk_dbg(MTK_TXD, "FTYPE = %d%s\n",
			GET_FIELD(WF_TX_DESCRIPTOR_FTYPE, dw),
			GET_FIELD(WF_TX_DESCRIPTOR_FTYPE, dw) == 0 ? "(mgmt)" : "",
			GET_FIELD(WF_TX_DESCRIPTOR_FTYPE, dw) == 1 ? "(ctl)" : "",
			GET_FIELD(WF_TX_DESCRIPTOR_FTYPE, dw) == 2 ? "(data)" : "");
	mtk_dbg(MTK_TXD, "BF_TYPE = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_BF_TYPE, dw));
	mtk_dbg(MTK_TXD, "OM_MAP = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_OM_MAP, dw));
	mtk_dbg(MTK_TXD, "RTS = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_RTS, dw));
	mtk_dbg(MTK_TXD, "HEADER_PADDING = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_HEADER_PADDING, dw));
	mtk_dbg(MTK_TXD, "DU = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_DU, dw));
	mtk_dbg(MTK_TXD, "HE = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_HE, dw));
	mtk_dbg(MTK_TXD, "FRAG = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_FRAG, dw));
	mtk_dbg(MTK_TXD, "REMAINING_TX_TIME = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_REMAINING_TX_TIME, dw));
	mtk_dbg(MTK_TXD, "POWER_OFFSET = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_POWER_OFFSET, dw));

	/* dw3 */
	dw = le32_to_cpu(txd[3]);
	mtk_dbg(MTK_TXD, "NA = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_NA, dw));
	mtk_dbg(MTK_TXD, "PF = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_PF, dw));
	mtk_dbg(MTK_TXD, "EMRD = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_EMRD, dw));
	mtk_dbg(MTK_TXD, "EEOSP = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_EEOSP, dw));
	mtk_dbg(MTK_TXD, "BM = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_BM, dw));
	mtk_dbg(MTK_TXD, "HW_AMSDU_CAP = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_HW_AMSDU_CAP, dw));
	mtk_dbg(MTK_TXD, "TX_COUNT = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_TX_COUNT, dw));
	mtk_dbg(MTK_TXD, "REMAINING_TX_COUNT = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_REMAINING_TX_COUNT, dw));
	mtk_dbg(MTK_TXD, "SN = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_SN, dw));
	mtk_dbg(MTK_TXD, "BA_DIS = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_BA_DIS, dw));
	mtk_dbg(MTK_TXD, "PM = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_PM, dw));
	mtk_dbg(MTK_TXD, "PN_VLD = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_PN_VLD, dw));
	mtk_dbg(MTK_TXD, "SN_VLD = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_SN_VLD, dw));

	/* dw4 */
	dw = le32_to_cpu(txd[4]);
	mtk_dbg(MTK_TXD, "PN_31_0 = 0x%x\n",
			GET_FIELD(WF_TX_DESCRIPTOR_PN_31_0_, dw));

	/* dw5 */
	dw = le32_to_cpu(txd[5]);
	mtk_dbg(MTK_TXD, "PID = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_PID, dw));
	mtk_dbg(MTK_TXD, "TXSFM = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_TXSFM, dw));
	mtk_dbg(MTK_TXD, "TXS2M = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_TXS2M, dw));
	mtk_dbg(MTK_TXD, "TXS2H = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_TXS2H, dw));
	mtk_dbg(MTK_TXD, "FBCZ = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_FBCZ, dw));
	mtk_dbg(MTK_TXD, "BYPASS_RBB = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_BYPASS_RBB, dw));

	mtk_dbg(MTK_TXD, "FL = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_FL, dw));
	mtk_dbg(MTK_TXD, "PN_47_32 = 0x%x\n",
			GET_FIELD(WF_TX_DESCRIPTOR_PN_47_32_, dw));

	/* dw6 */
	dw = le32_to_cpu(txd[6]);
	mtk_dbg(MTK_TXD, "AMSDU_CAP_UTXB = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB, dw));
	mtk_dbg(MTK_TXD, "DAS = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_DAS, dw));
	mtk_dbg(MTK_TXD, "DIS_MAT = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_DIS_MAT, dw));
	mtk_dbg(MTK_TXD, "MSDU_COUNT = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_MSDU_COUNT, dw));
	mtk_dbg(MTK_TXD, "TIMESTAMP_OFFSET = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_IDX, dw));
	mtk_dbg(MTK_TXD, "FIXED_RATE_IDX = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_FIXED_RATE_IDX, dw));
	mtk_dbg(MTK_TXD, "BW = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_BW, dw));
	mtk_dbg(MTK_TXD, "VTA = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_VTA, dw));
	mtk_dbg(MTK_TXD, "SRC = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_SRC, dw));

	/* dw7 */
	dw = le32_to_cpu(txd[7]);
	mtk_dbg(MTK_TXD, "SW_TX_TIME(unit:65536ns) = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_SW_TX_TIME, dw));
	mtk_dbg(MTK_TXD, "UT = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_UT, dw));
	mtk_dbg(MTK_TXD, "CTXD_CNT = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_CTXD_CNT, dw));
	mtk_dbg(MTK_TXD, "HM = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_HM, dw));
	mtk_dbg(MTK_TXD, "DP = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_DP, dw));
	mtk_dbg(MTK_TXD, "IP = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_IP, dw));
	mtk_dbg(MTK_TXD, "TXD_LEN = %d\n",
			GET_FIELD(WF_TX_DESCRIPTOR_TXD_LEN, dw));
}

void bmac_dump_mac_txp(__le32 *txp)
{
	int i, j;
	u32 dw;

	mtk_dbg_dump(MTK_TXD_DUMP, "txp",
			txp, MAC_TXP_SIZE);

	/* dw0 */
	dw = le32_to_cpu(txp[0]);
	mtk_dbg(MTK_TXD, "TOKEN_ID0 = %d\n",
			GET_FIELD(MAC_TXP_TOKEN_ID0, dw));
	mtk_dbg(MTK_TXD, "TOKEN_ID0_VALID = %d\n",
			GET_FIELD(MAC_TXP_TOKEN_ID0_VALID, dw));
	mtk_dbg(MTK_TXD, "TOKEN_ID1 = %d\n",
			GET_FIELD(MAC_TXP_TOKEN_ID1, dw));
	mtk_dbg(MTK_TXD, "TOKEN_ID1_VALID = %d\n",
			GET_FIELD(MAC_TXP_TOKEN_ID1_VALID, dw));

	/* dw1 */
	dw = le32_to_cpu(txp[1]);
	mtk_dbg(MTK_TXD, "TOKEN_ID2 = %d\n",
			GET_FIELD(MAC_TXP_TOKEN_ID0, dw));
	mtk_dbg(MTK_TXD, "TOKEN_ID2_VALID = %d\n",
			GET_FIELD(MAC_TXP_TOKEN_ID0_VALID, dw));
	mtk_dbg(MTK_TXD, "TOKEN_ID3 = %d\n",
			GET_FIELD(MAC_TXP_TOKEN_ID1, dw));
	mtk_dbg(MTK_TXD, "TOKEN_ID3_VALID = %d\n",
			GET_FIELD(MAC_TXP_TOKEN_ID1_VALID, dw));
	mtk_dbg(MTK_TXD, "TID_ADDBA = %d\n",
			GET_FIELD(MAC_TXP_TID_ADDBA, dw));

	/* dw2 */
	j = 0;
	for (i = 2; i < 8; i++) {
		dw = le32_to_cpu(txp[i]);

		if (i % 3 == 0) {
			mtk_dbg(MTK_TXD, "MAC_TXP_BUF_LEN%d = %d\n",
				j - 1, GET_FIELD(MAC_TXP_BUF_LEN0, dw));
			mtk_dbg(MTK_TXD, "MAC_TXP_BUF_PTR%d_H = 0x%x\n",
				j - 1, GET_FIELD(MAC_TXP_BUF_PTR0_H, dw));
			mtk_dbg(MTK_TXD, "MAC_TXP_BUF_SRC%d = %d\n",
				j - 1, GET_FIELD(MAC_TXP_BUF_SRC0, dw));
			mtk_dbg(MTK_TXD, "MAC_TXP_BUF_ML%d = %d\n",
				j - 1, GET_FIELD(MAC_TXP_BUF_ML0, dw));
			mtk_dbg(MTK_TXD, "MAC_TXP_BUF_LEN%d = %d\n",
				j, GET_FIELD(MAC_TXP_BUF_LEN0, dw));
			mtk_dbg(MTK_TXD, "MAC_TXP_BUF_PTR%d_H = 0x%x\n",
				j, GET_FIELD(MAC_TXP_BUF_PTR0_H, dw));
			mtk_dbg(MTK_TXD, "MAC_TXP_BUF_SRC%d = %d\n",
				j, GET_FIELD(MAC_TXP_BUF_SRC0, dw));
			mtk_dbg(MTK_TXD, "MAC_TXP_BUF_ML%d = %d\n",
				j, GET_FIELD(MAC_TXP_BUF_ML0, dw));
		} else {
			mtk_dbg(MTK_TXD, "MAC_TXP_BUF_PTR%d_L = 0x0x\n",
				j, GET_FIELD(MAC_TXP_BUF_PTR0_L, dw));
			j++;
		}
	}
}

void bmac_dump_hif_txp(__le32 *txp)
{
	int i;
	u32 dw;

	mtk_dbg_dump(MTK_TXD_DUMP, "txp",
			txp, HIF_TXP_SIZE);

	/* dw0 */
	dw = le32_to_cpu(txp[0]);
	mtk_dbg(MTK_TXD, "HIF_TXP_APPLY_TXD = %d\n",
			dw & MT_CT_INFO_APPLY_TXD ? 1 : 0);
	mtk_dbg(MTK_TXD, "HIF_TXP_MGMT_FRAME = %d\n",
			dw & MT_CT_INFO_MGMT_FRAME ? 1 : 0);
	mtk_dbg(MTK_TXD, "HIF_TXP_NONE_CIPHER_FRAME = %d\n",
			dw & MT_CT_INFO_NONE_CIPHER_FRAME ? 1 : 0);
	mtk_dbg(MTK_TXD, "HIF_TXP_HSR2_TX = %d\n",
			dw & MT_CT_INFO_HSR2_TX ? 1 : 0);
	mtk_dbg(MTK_TXD, "HIF_TXP_PKT_FR_HOST = %d\n",
			dw & MT_CT_INFO_PKT_FR_HOST ? 1 : 0);
	mtk_dbg(MTK_TXD, "HIF_TXP_TOKEN_ID = %d\n",
			GET_FIELD(HIF_TXP_TOKEN_ID, dw));

	/* dw1 */
	dw = le32_to_cpu(txp[1]);
	mtk_dbg(MTK_TXD, "HIF_TXP_BSS_IDX = %d\n",
			GET_FIELD(HIF_TXP_BSS_IDX, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_WLAN_IDX = %d\n",
			GET_FIELD(HIF_TXP_V1_WLAN_IDX, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_BUF_NUM = %d\n",
			GET_FIELD(HIF_TXP_V1_BUF_NUM, dw));

	/* dw2 */
	for (i = 0; i < 6; i++)
		mtk_dbg(MTK_TXD, "HIF_TXP_BUF_PTR%d_L = 0x%x\n", i,
			GET_FIELD(HIF_TXP_BUF_PTR0_L, le32_to_cpu(txp[2 + i])));

	for (i = 0; i < 6; i++) {
		mtk_dbg(MTK_TXD, "HIF_TXP_BUF_LEN%d = %u\n", i,
			i % 2 == 0 ?
			GET_FIELD(HIF_TXP_BUF_LEN0, le32_to_cpu(txp[8 + i])) :
			GET_FIELD(HIF_TXP_BUF_LEN1, le32_to_cpu(txp[8 + i])));
		mtk_dbg(MTK_TXD, "HIF_TXP_BUF_PTR%d_H = 0x%x\n", i,
			i % 2 == 0 ?
			GET_FIELD(HIF_TXP_BUF_PTR0_H, le32_to_cpu(txp[8 + i])) :
			GET_FIELD(HIF_TXP_BUF_PTR1_H, le32_to_cpu(txp[8 + i])));
	}
}


void bmac_dump_hif_txp_v2(__le32 *txp)
{
	int i, j = 0;
	u32 dw;

	mtk_dbg_dump(MTK_TXD_DUMP, "txp",
					txp, HIF_TXP_V2_SIZE);

	/* dw0 */
	dw = le32_to_cpu(txp[0]);
	mtk_dbg(MTK_TXD, "HIF_TXP_STRICT_PRIOR = %d\n",
			GET_FIELD(HIF_TXP_STRICT_PRIOR, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_FIXED_RATE = %d\n",
			GET_FIELD(HIF_TXP_FIXED_RATE, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_TCP = %d\n",
			GET_FIELD(HIF_TXP_TCP, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_NON_CIPHER = %d\n",
			GET_FIELD(HIF_TXP_NON_CIPHER, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_VLAN = %d\n",
			GET_FIELD(HIF_TXP_VLAN, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_BC_MC_FLAG = %d\n",
			GET_FIELD(HIF_TXP_BC_MC_FLAG, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_FR_HOST = %d\n",
			GET_FIELD(HIF_TXP_FR_HOST, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_ETYPE = %d\n",
			GET_FIELD(HIF_TXP_ETYPE, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_TXP_AMSDU = %d\n",
			GET_FIELD(HIF_TXP_AMSDU, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_TXP_MC_CLONE = %d\n",
			GET_FIELD(HIF_TXP_MC_CLONE, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_PRIORITY = %d\n",
			GET_FIELD(HIF_TXP_PRIORITY, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_TOKEN_ID = %d\n",
			GET_FIELD(HIF_TXP_TOKEN_ID, dw));

	/* dw1 */
	dw = le32_to_cpu(txp[1]);
	mtk_dbg(MTK_TXD, "HIF_TXP_BSS_IDX = %d\n",
			GET_FIELD(HIF_TXP_BSS_IDX, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_USER_PRIORITY = %d\n",
			GET_FIELD(HIF_TXP_USER_PRIORITY, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_BUF_NUM = %d\n",
			GET_FIELD(HIF_TXP_BUF_NUM, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_MSDU_CNT = %d\n",
			GET_FIELD(HIF_TXP_MSDU_CNT, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_SRC = %d\n",
			GET_FIELD(HIF_TXP_SRC, dw));

	/* dw2 */
	dw = le32_to_cpu(txp[2]);
	mtk_dbg(MTK_TXD, "HIF_TXP_ETH_TYPE(network-endian) = 0x%x\n",
			GET_FIELD(HIF_TXP_ETH_TYPE, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_WLAN_IDX = %d\n",
			GET_FIELD(HIF_TXP_WLAN_IDX, dw));

	/* dw3 */
	dw = le32_to_cpu(txp[3]);
	mtk_dbg(MTK_TXD, "HIF_TXP_PPE_INFO = 0x%x\n",
			GET_FIELD(HIF_TXP_PPE_INFO, dw));

	for (i = 0; i < 13; i++) {
		if (i % 2 == 0) {
			mtk_dbg(MTK_TXD, "HIF_TXP_BUF_PTR%d_L = 0x%x\n", i,
				GET_FIELD(HIF_TXP_BUF_PTR0_L, le32_to_cpu(txp[4 + j])));
			j++;
			mtk_dbg(MTK_TXD, "HIF_TXP_BUF_LEN%d = %d\n", i,
				GET_FIELD(HIF_TXP_BUF_LEN0, le32_to_cpu(txp[4 + j])));
			mtk_dbg(MTK_TXD, "HIF_TXP_BUF_PTR%d_H = 0x%x\n", i,
				GET_FIELD(HIF_TXP_BUF_PTR0_H, le32_to_cpu(txp[4 + j])));
			if (i <= 10) {
				mtk_dbg(MTK_TXD, "HIF_TXP_BUF_LEN%d = %d\n", i + 1,
					GET_FIELD(HIF_TXP_BUF_LEN1, le32_to_cpu(txp[4 + j])));
				mtk_dbg(MTK_TXD, "HIF_TXP_BUF_PTR%d_H = 0x%x\n", i + 1,
					GET_FIELD(HIF_TXP_BUF_PTR1_H, le32_to_cpu(txp[4 + j])));
			}
			j++;
		} else {
			mtk_dbg(MTK_TXD, "HIF_TXP_BUF_PTR%d_L = 0x%x\n",
				i, GET_FIELD(HIF_TXP_BUF_PTR1_L,
				le32_to_cpu(txp[4 + j])));
			j++;
		}
	}

	mtk_dbg(MTK_TXD, "ml = 0x%x\n",
			GET_FIELD(HIF_TXP_ML, le32_to_cpu(txp[23])));
}

void bmac_dump_hif_txp_v3(__le32 *txp)
{
	int i, j = 0;
	u32 dw;

	mtk_dbg_dump(MTK_TXD_DUMP, "txp",
					txp, HIF_TXP_V3_SIZE);

	/* dw0 */
	dw = le32_to_cpu(txp[0]);
	mtk_dbg(MTK_TXD, "HIF_TXP_STRICT_PRIOR = %d\n",
			GET_FIELD(HIF_TXP_STRICT_PRIOR, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_FIXED_RATE = %d\n",
			GET_FIELD(HIF_TXP_FIXED_RATE, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_TCP = %d\n",
			GET_FIELD(HIF_TXP_TCP, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_NON_CIPHER = %d\n",
			GET_FIELD(HIF_TXP_NON_CIPHER, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_VLAN = %d\n",
			GET_FIELD(HIF_TXP_VLAN, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_BC_MC_FLAG = %d\n",
			GET_FIELD(HIF_TXP_BC_MC_FLAG, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_FR_HOST = %d\n",
			GET_FIELD(HIF_TXP_FR_HOST, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_ETYPE = %d\n",
			GET_FIELD(HIF_TXP_ETYPE, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_TXP_AMSDU = %d\n",
			GET_FIELD(HIF_TXP_AMSDU, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_TXP_MC_CLONE = %d\n",
			GET_FIELD(HIF_TXP_MC_CLONE, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_PRIORITY = %d\n",
			GET_FIELD(HIF_TXP_PRIORITY, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_TOKEN_ID = %d\n",
			GET_FIELD(HIF_TXP_TOKEN_ID, dw));

	/* dw1 */
	dw = le32_to_cpu(txp[1]);
	mtk_dbg(MTK_TXD, "HIF_TXP_BSS_IDX = %d\n",
			GET_FIELD(HIF_TXP_BSS_IDX, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_USER_PRIORITY = %d\n",
			GET_FIELD(HIF_TXP_USER_PRIORITY, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_BUF_NUM = %d\n",
			GET_FIELD(HIF_TXP_BUF_NUM, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_MSDU_CNT = %d\n",
			GET_FIELD(HIF_TXP_MSDU_CNT, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_SRC = %d\n",
			GET_FIELD(HIF_TXP_SRC, dw));

	/* dw2 */
	dw = le32_to_cpu(txp[2]);
	mtk_dbg(MTK_TXD, "HIF_TXP_ETH_TYPE(network-endian) = 0x%x\n",
			GET_FIELD(HIF_TXP_ETH_TYPE, dw));
	mtk_dbg(MTK_TXD, "HIF_TXP_WLAN_IDX = %d\n",
			GET_FIELD(HIF_TXP_WLAN_IDX, dw));

	/* dw3 */
	dw = le32_to_cpu(txp[3]);
	mtk_dbg(MTK_TXD, "HIF_TXP_PPE_INFO = 0x%x\n",
			GET_FIELD(HIF_TXP_PPE_INFO, dw));

	/* dw4 */
	mtk_dbg(MTK_TXD, "ml = 0x%x\n",
			GET_FIELD(HIF_TXP_V3_ML, le32_to_cpu(txp[4])));

	for (i = 0; i < 12; i++) {
		if (i % 2 == 0) {
			mtk_dbg(MTK_TXD, "HIF_TXP_BUF_PTR%d_L = 0x%x\n", i,
				GET_FIELD(HIF_TXP_BUF_PTR0_L, le32_to_cpu(txp[5 + j])));
			j++;
			mtk_dbg(MTK_TXD, "HIF_TXP_BUF_LEN%d = %d\n", i,
				GET_FIELD(HIF_TXP_BUF_LEN0, le32_to_cpu(txp[5 + j])));
			mtk_dbg(MTK_TXD, "HIF_TXP_BUF_PTR%d_H = 0x%x\n", i,
				GET_FIELD(HIF_TXP_BUF_PTR0_H, le32_to_cpu(txp[5 + j])));
			if (i <= 10) {
				mtk_dbg(MTK_TXD, "HIF_TXP_BUF_LEN%d = %d\n", i + 1,
					GET_FIELD(HIF_TXP_BUF_LEN1, le32_to_cpu(txp[5 + j])));
				mtk_dbg(MTK_TXD, "HIF_TXP_BUF_PTR%d_H = 0x%x\n", i + 1,
					GET_FIELD(HIF_TXP_BUF_PTR1_H, le32_to_cpu(txp[5 + j])));
			}
			j++;
		} else {
			mtk_dbg(MTK_TXD, "HIF_TXP_BUF_PTR%d_L = 0x%x\n",
				i, GET_FIELD(HIF_TXP_BUF_PTR1_L,
				le32_to_cpu(txp[5 + j])));
			j++;
		}
	}
}

void bmac_dump_rxd(__le32 *rxd)
{
	mtk_dbg_dump(MTK_RXD_DUMP, "rxd", rxd, 96);

	/* group0 */
	/* dw0 */
	mtk_dbg(MTK_RXD, "RX_BYTE_COUNT = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_RX_BYTE_COUNT, le32_to_cpu(rxd[0])));
	mtk_dbg(MTK_RXD, "PACKET_TYPE = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_PACKET_TYPE, le32_to_cpu(rxd[0])));

	/* dw1 */
	mtk_dbg(MTK_RXD, "MLD_ID = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_MLD_ID, le32_to_cpu(rxd[1])));
	mtk_dbg(MTK_RXD, "GROUP_VLD = 0x%x%s%s%s%s%s\n",
			GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, le32_to_cpu(rxd[1])),
			GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, le32_to_cpu(rxd[1]))
			& BMAC_GROUP_VLD_1 ? "[group1]" : "",
			GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, le32_to_cpu(rxd[1]))
			& BMAC_GROUP_VLD_2 ? "[group2]" : "",
			GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, le32_to_cpu(rxd[1]))
			& BMAC_GROUP_VLD_3 ? "[group3]" : "",
			GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, le32_to_cpu(rxd[1]))
			& BMAC_GROUP_VLD_4 ? "[group4]" : "",
			GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, le32_to_cpu(rxd[1]))
			& BMAC_GROUP_VLD_5 ? "[group5]" : "");
	mtk_dbg(MTK_RXD, "KID = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_KID, le32_to_cpu(rxd[1])));
	mtk_dbg(MTK_RXD, "CM = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_CM, le32_to_cpu(rxd[1])));
	mtk_dbg(MTK_RXD, "CLM = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_CLM, le32_to_cpu(rxd[1])));
	mtk_dbg(MTK_RXD, "I = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_I, le32_to_cpu(rxd[1])));
	mtk_dbg(MTK_RXD, "T = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_T, le32_to_cpu(rxd[1])));
	mtk_dbg(MTK_RXD, "BN = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_BN, le32_to_cpu(rxd[1])));
	mtk_dbg(MTK_RXD, "BIPN_FAIL = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_BIPN_FAIL, le32_to_cpu(rxd[1])));

	/* dw2 */
	mtk_dbg(MTK_RXD, "BSSID = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_BSSID, le32_to_cpu(rxd[2])));
	mtk_dbg(MTK_RXD, "H = %d%s\n",
			GET_FIELD(WF_RX_DESCRIPTOR_H, le32_to_cpu(rxd[2])),
			GET_FIELD(WF_RX_DESCRIPTOR_H, le32_to_cpu(rxd[2])) == 0 ?
			"802.11 frame" : "eth/802.3 frame");
	mtk_dbg(MTK_RXD, "HEADER_LENGTH(word) = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_HEADER_LENGTH, le32_to_cpu(rxd[2])));
	mtk_dbg(MTK_RXD, "HO(word) = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_HO, le32_to_cpu(rxd[2])));
	mtk_dbg(MTK_RXD, "SEC_MODE = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_SEC_MODE, le32_to_cpu(rxd[2])));
	mtk_dbg(MTK_RXD, "MUBAR = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_MUBAR, le32_to_cpu(rxd[2])));
	mtk_dbg(MTK_RXD, "SWBIT = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_SWBIT, le32_to_cpu(rxd[2])));
	mtk_dbg(MTK_RXD, "DAF = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_DAF, le32_to_cpu(rxd[2])));
	mtk_dbg(MTK_RXD, "EL = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_EL, le32_to_cpu(rxd[2])));
	mtk_dbg(MTK_RXD, "HTF = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_HTF, le32_to_cpu(rxd[2])));
	mtk_dbg(MTK_RXD, "INTF = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_INTF, le32_to_cpu(rxd[2])));
	mtk_dbg(MTK_RXD, "FRAG = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_FRAG, le32_to_cpu(rxd[2])));
	mtk_dbg(MTK_RXD, "NUL = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_NUL, le32_to_cpu(rxd[2])));
	mtk_dbg(MTK_RXD, "NDATA = %d%s\n",
			GET_FIELD(WF_RX_DESCRIPTOR_NDATA, le32_to_cpu(rxd[2])),
			GET_FIELD(WF_RX_DESCRIPTOR_NDATA, le32_to_cpu(rxd[2])) == 0 ?
			"[data frame]" : "[mgmt/ctl frame]");
	mtk_dbg(MTK_RXD, "NAMP = %d%s\n",
			GET_FIELD(WF_RX_DESCRIPTOR_NAMP, le32_to_cpu(rxd[2])),
			GET_FIELD(WF_RX_DESCRIPTOR_NAMP, le32_to_cpu(rxd[2])) == 0 ?
			"[ampdu frame]" : "[mpdu frame]");
	mtk_dbg(MTK_RXD, "BF_RPT = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_BF_RPT, le32_to_cpu(rxd[2])));

	/* dw3 */
	mtk_dbg(MTK_RXD, "RXV_SN = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_RXV_SN, le32_to_cpu(rxd[3])));
	mtk_dbg(MTK_RXD, "CH_FREQUENCY = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_CH_FREQUENCY, le32_to_cpu(rxd[3])));
	mtk_dbg(MTK_RXD, "A1_TYPE = %d%s%s%s%s\n",
			GET_FIELD(WF_RX_DESCRIPTOR_A1_TYPE, le32_to_cpu(rxd[3])),
			GET_FIELD(WF_RX_DESCRIPTOR_A1_TYPE, le32_to_cpu(rxd[3])) == 0 ?
			"[reserved]" : "",
			GET_FIELD(WF_RX_DESCRIPTOR_A1_TYPE, le32_to_cpu(rxd[3])) == 1 ?
			"[uc2me]" : "",
			GET_FIELD(WF_RX_DESCRIPTOR_A1_TYPE, le32_to_cpu(rxd[3])) == 2 ?
			"[mc]" : "",
			GET_FIELD(WF_RX_DESCRIPTOR_A1_TYPE, le32_to_cpu(rxd[3])) == 3 ?
			"[bc]" : "");
	mtk_dbg(MTK_RXD, "HTC = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_HTC, le32_to_cpu(rxd[3])));
	mtk_dbg(MTK_RXD, "TCL = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_TCL, le32_to_cpu(rxd[3])));
	mtk_dbg(MTK_RXD, "BBM = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_BBM, le32_to_cpu(rxd[3])));
	mtk_dbg(MTK_RXD, "BU = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_BU, le32_to_cpu(rxd[3])));
	mtk_dbg(MTK_RXD, "CO_ANT = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_CO_ANT, le32_to_cpu(rxd[3])));
	mtk_dbg(MTK_RXD, "BF_CQI = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_BF_CQI, le32_to_cpu(rxd[3])));
	mtk_dbg(MTK_RXD, "FC = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_FC, le32_to_cpu(rxd[3])));
	mtk_dbg(MTK_RXD, "VLAN = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_VLAN, le32_to_cpu(rxd[3])));

	/* dw4 */
	mtk_dbg(MTK_RXD, "PF = %d%s%s%s%s\n",
			GET_FIELD(WF_RX_DESCRIPTOR_PF, le32_to_cpu(rxd[4])),
			GET_FIELD(WF_RX_DESCRIPTOR_PF, le32_to_cpu(rxd[4])) == 0 ?
			"[msdu]" : "",
			GET_FIELD(WF_RX_DESCRIPTOR_PF, le32_to_cpu(rxd[4])) == 1 ?
			"[final amsdu]" : "",
			GET_FIELD(WF_RX_DESCRIPTOR_PF, le32_to_cpu(rxd[4])) == 2 ?
			"[middle amsdu]" : "",
			GET_FIELD(WF_RX_DESCRIPTOR_PF, le32_to_cpu(rxd[4])) == 3 ?
			"[first amsdu]" : "");
	mtk_dbg(MTK_RXD, "MAC = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_MAC, le32_to_cpu(rxd[4])));
	mtk_dbg(MTK_RXD, "TID = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_TID, le32_to_cpu(rxd[4])));
	mtk_dbg(MTK_RXD, "ETHER_TYPE_OFFSET = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_ETHER_TYPE_OFFSET, le32_to_cpu(rxd[4])));
	mtk_dbg(MTK_RXD, "IP = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_IP, le32_to_cpu(rxd[4])));
	mtk_dbg(MTK_RXD, "UT = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_UT, le32_to_cpu(rxd[4])));
	mtk_dbg(MTK_RXD, "PSE_FID = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_PSE_FID, le32_to_cpu(rxd[4])));

	/* group4 */
	/* dw0 */
	mtk_dbg(MTK_RXD, "FRAME_CONTROL_FIELD = 0x%x\n",
			GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, le32_to_cpu(rxd[1]))
			& BMAC_GROUP_VLD_4 ?
			GET_FIELD(WF_RX_DESCRIPTOR_FRAME_CONTROL_FIELD, le32_to_cpu(rxd[8])) : 0);
	mtk_dbg(MTK_RXD, "PEER_MLD_ADDRESS_15_0 = 0x%x\n",
			GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, le32_to_cpu(rxd[1]))
			& BMAC_GROUP_VLD_4 ?
			GET_FIELD(WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_15_0_,
			le32_to_cpu(rxd[8])) : 0);

	/* dw1 */
	mtk_dbg(MTK_RXD, "PEER_MLD_ADDRESS_47_16 = 0x%x\n",
			GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, le32_to_cpu(rxd[1]))
			& BMAC_GROUP_VLD_4 ?
			GET_FIELD(WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_47_16_,
			le32_to_cpu(rxd[9])) : 0);

	/* dw2 */
	mtk_dbg(MTK_RXD, "FRAGMENT_NUMBER = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, le32_to_cpu(rxd[1]))
			& BMAC_GROUP_VLD_4 ?
			GET_FIELD(WF_RX_DESCRIPTOR_FRAGMENT_NUMBER,
			le32_to_cpu(rxd[10])) : 0);
	mtk_dbg(MTK_RXD, "SEQUENCE_NUMBER = %d\n",
			GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, le32_to_cpu(rxd[1]))
			& BMAC_GROUP_VLD_4 ?
			GET_FIELD(WF_RX_DESCRIPTOR_SEQUENCE_NUMBER,
			le32_to_cpu(rxd[10])) : 0);
	mtk_dbg(MTK_RXD, "QOS_CONTROL_FIELD = 0x%x\n",
			GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, le32_to_cpu(rxd[1]))
			& BMAC_GROUP_VLD_4 ?
			GET_FIELD(WF_RX_DESCRIPTOR_QOS_CONTROL_FIELD,
			le32_to_cpu(rxd[10])) : 0);

	/* dw3 */
	mtk_dbg(MTK_RXD, "HT_CONTROL_FIELD = 0x%x\n",
			GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, le32_to_cpu(rxd[1]))
			& BMAC_GROUP_VLD_4 ?
			GET_FIELD(WF_RX_DESCRIPTOR_HT_CONTROL_FIELD,
			le32_to_cpu(rxd[11])) : 0);
}
#endif
