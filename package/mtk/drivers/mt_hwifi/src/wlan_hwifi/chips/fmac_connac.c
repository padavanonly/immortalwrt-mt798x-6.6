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
#include <wlan_tr.h>

int
fmac_write_txp(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
				void *tx_pkt_info)
{
	struct hif_txp *txp;
	struct mtk_hw_bss *bss = tk_entry->sta->bss;
	struct _TX_BLK *tx_blk = (struct _TX_BLK *)tx_pkt_info;
	struct sk_buff *skb = tk_entry->tx_q.next;

	txp = (struct hif_txp *)(tk_entry->txd_ptr + MT_TXD_SIZE);
	memset(txp, 0, sizeof(*txp));

	tk_entry->fbuf_dma_size += HIF_TXP_SIZE;
	tk_entry->pkt_pa = dma_map_single(dev->dev,
					skb->data, skb->len, DMA_TO_DEVICE);

	if (unlikely(dma_mapping_error(dev->dev, tk_entry->pkt_pa))) {
			dev_err(dev->dev, "%s(): dma map fail for tkid %d\n",
					__func__, tk_entry->sid.idx);
		return -ENOMEM;
	}

	txp->buf[0] = cpu_to_le32(SET_FIELD(HIF_TXP_BUF_PTR, tk_entry->pkt_pa));
	txp->len[0] = SET_FIELD(HIF_TXP_BUF_LEN, skb->len);
#ifdef CONFIG_HWIFI_64BIT_SUPPORT
	txp->len[0] |= SET_FIELD(HIF_TXP_BUF_PTR_H,
			tk_entry->pkt_pa >> DMA_ADDR_H_SHIFT);
#endif
	txp->len[0] = cpu_to_le16(txp->len[0]);
	txp->nbuf = 1;
	txp->token = cpu_to_le16(tk_entry->sid.idx);

	txp->bss_idx = bss->fw_idx;
	txp->rept_wds_wcid = cpu_to_le16(0x3ff);
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_CT_WithTxD))
		txp->flags = MT_CT_INFO_APPLY_TXD;
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bClearEAPFrame))
		txp->flags |= MT_CT_INFO_NONE_CIPHER_FRAME;
	if (tx_blk->dot11_type == FC_TYPE_MGMT)
		txp->flags |= MT_CT_INFO_MGMT_FRAME;
	if (GET_PACKET_HS2_TX(skb))
		txp->flags |= MT_CT_INFO_HSR2_TX;
	txp->flags |= MT_CT_INFO_PKT_FR_HOST;

	txp->flags = cpu_to_le16(txp->flags);

	return 0;
}

int
fmac_write_fix_txd(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
	void *_txd, void *tx_pkt_info)
{
	u8 q_idx, p_fmt;
	__le32 *txd = (__le32 *) _txd;
	u32 val;
	int tx_count = 8;
	struct mtk_hw_bss *bss = sta->bss;
	struct _TX_BLK *tx_blk = (struct _TX_BLK *)tx_pkt_info;
	struct sk_buff *skb = (struct sk_buff *)tx_blk->pPacket;

	if (tx_blk->QueIdx < 4) {
		q_idx = bss->wmm_idx * MAX_WMM_SETS + tx_blk->QueIdx;
		p_fmt = MT_TX_TYPE_CT;
	} else if (tx_blk->dot11_subtype == SUBTYPE_BEACON) {
		q_idx = MT_LMAC_BCN0;
		p_fmt = MT_TX_TYPE_FW;
	} else {
		q_idx = MT_LMAC_ALTX0;
		p_fmt = MT_TX_TYPE_CT;
	}

	val = field_prep(MT_TXD0_TX_BYTES, skb->len + MT_TXD_SIZE) |
	field_prep(MT_TXD0_PKT_FMT, p_fmt) |
	field_prep(MT_TXD0_Q_IDX, q_idx);
	txd[0] = cpu_to_le32(val);

	val = MT_TXD1_LONG_FORMAT |
	field_prep(MT_TXD1_WLAN_IDX, sta->link_wcid) |
	field_prep(MT_TXD1_HDR_FORMAT, MT_HDR_FORMAT_802_11) |
	field_prep(MT_TXD1_HDR_INFO, tx_blk->wifi_hdr_len >> 1) |
	field_prep(MT_TXD1_TID, tx_blk->UserPriority) |
	field_prep(MT_TXD1_OWN_MAC, bss->omac_idx);

	if (bss->hw_phy->band_idx != 0 &&
		q_idx >= MT_LMAC_ALTX0 &&
		q_idx <= MT_LMAC_BCN0) {
		val |= cpu_to_le32(MT_TXD1_TGID);
	}
	txd[1] = cpu_to_le32(val);

	val = field_prep(MT_TXD2_FRAME_TYPE, tx_blk->dot11_type) |
	      field_prep(MT_TXD2_SUB_TYPE, tx_blk->dot11_subtype) |
	      field_prep(MT_TXD2_MULTICAST,
			(tx_blk->TxFrameType == TX_BMC_FRAME) ? 1 : 0);

	if (!IS_CIPHER_NONE(tx_blk->CipherAlg)) {
		txd[3] = cpu_to_le32(MT_TXD3_PROTECT_FRAME);
	} else {
		txd[3] = 0;
	}
	txd[2] = cpu_to_le32(val);
	txd[4] = 0;
	txd[5] = 0;
	txd[6] = 0;

	if ((tx_blk->QueIdx >= 4) || (tx_blk->TxFrameType == TX_BMC_FRAME)) {
		u16 rateval = 0x4b;

		/* H/W won't add HTC for mgmt/ctrl frame */
		txd[2] |= cpu_to_le32(MT_TXD2_FIX_RATE | MT_TXD2_HTC_VLD);

		val = MT_TXD6_FIXED_BW |
			  field_prep(MT_TXD6_TX_RATE, rateval);
		txd[6] |= cpu_to_le32(val);
		txd[3] |= cpu_to_le32(MT_TXD3_BA_DISABLE);

		if (!(tx_blk->dot11_subtype == SUBTYPE_BEACON)) {
			val = MT_TXD5_TX_STATUS_HOST |
				  field_prep(MT_TXD5_PID, tx_blk->Pid);
			txd[5] |= cpu_to_le32(val);
			txd[3] |= cpu_to_le32(MT_TXD3_SW_POWER_MGMT);
		} else {
			tx_count = 0x1f;
		}
	} else {
		txd[3] |= cpu_to_le32(MT_TXD3_SW_POWER_MGMT);
	}

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bAckRequired))
		txd[3] |= cpu_to_le32(MT_TXD3_NO_ACK);

	val = field_prep(MT_TXD7_TYPE, tx_blk->dot11_type) |
		  field_prep(MT_TXD7_SUB_TYPE, tx_blk->dot11_subtype);
	txd[7] = cpu_to_le32(val);

	val = field_prep(MT_TXD3_REM_TX_COUNT, tx_count);

	if (test_bit(HWIFI_FLAG_HW_AMSDU, &dev->flags)) {
		if (is_amsdu_capable(skb))
			txd[7] |= MT_TXD7_HW_AMSDU;
	} else {
		val |= MT_TXD3_SN_VALID;
	}
	txd[3] |= cpu_to_le32(val);

	if (0)
		dev->dbg_ops->txd_info(dev, txd);
	return 0;
}

static int
fmac_write_offload_txd(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
	void *_txd, void *tx_pkt_info)
{
	__le32 *txd = (__le32 *) _txd;
	u32 val;

	memset(_txd, 0, MT_TXD_SIZE);
	val = field_prep(MT_TXD0_PKT_FMT, MT_TX_TYPE_CT);
	txd[0] = cpu_to_le32(val);

	val = MT_TXD1_LONG_FORMAT;
	txd[1] = cpu_to_le32(val);
	return 0;
}

static int
fmac_write_mac_txd(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
	void *_txd, void *tx_pkt_info)
{
	dev_info(dev->dev, "%s()\n", __func__);
	return -EINVAL;
}

int
fmac_write_txd(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
	struct mtk_tk_entry *tk_entry, void *tx_pkt_info)
{
	struct _TX_BLK *tx_blk = (struct _TX_BLK *)tx_pkt_info;

	tk_entry->fbuf_dma_size = MT_TXD_SIZE;

	if (test_bit(HWIFI_FLAG_MCU_TXD, &dev->flags)) {
		if (TX_BLK_TEST_FLAG(tx_blk, fTX_CT_WithTxD))
			return fmac_write_fix_txd(dev, sta, tk_entry->txd_ptr, tx_pkt_info);
		else
			return fmac_write_offload_txd(dev, sta, tk_entry->txd_ptr, tx_pkt_info);
	} else {
		return fmac_write_mac_txd(dev, sta, tk_entry->txd_ptr, tx_pkt_info);
	}
	return -EINVAL;
}

