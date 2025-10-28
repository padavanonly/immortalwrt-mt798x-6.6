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
#include <os/hwifi_mac.h>
#include <wlan_tr.h>

static void tx_bytes_calculate(struct mtk_tk_entry *tk_entry,
							struct _TX_BLK *tx_blk)
{
	struct sk_buff *pkt_next = NULL;

	skb_queue_walk(&tk_entry->tx_q, pkt_next) {
		if (skb_queue_is_first(&tk_entry->tx_q, pkt_next))
			tx_blk->tx_bytes_len = MT_TXD_SIZE;

		tx_blk->tx_bytes_len += tx_blk->MpduHeaderLen +
					tx_blk->HdrPadLen +
					pkt_next->len;

		if (((GET_PACKET_PROTOCOL(tx_blk->pPacket) <= 1500) ? 0 : 1))
			tx_blk->tx_bytes_len += 8;

		if (GET_PACKET_VLAN(tx_blk->pPacket)) {
			/* TODO: add remove vlan */
			if (0)
				tx_blk->tx_bytes_len -= 4;
			else
				tx_blk->tx_bytes_len += 6;
		}

		if (!skb_queue_is_last(&tk_entry->tx_q, pkt_next)) {
			if (tx_blk->tx_bytes_len & 3)
				tx_blk->tx_bytes_len += (4 - (tx_blk->tx_bytes_len & 3));
		}
	}
}


static void
bmac_tx_free_notify_v0(struct mtk_bus_trans *trans, void *free_ptr)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct bmac_tx_free_v0 *free = (struct bmac_tx_free_v0 *)free_ptr;
	u8 msdu_cnt = 0, dw_idx = 0;
	u8 i = 0, ver = 0;
	u32 rx_bytes = 0;

	msdu_cnt = GET_FIELD(WF_TX_FREE_DONE_EVENT_V0_MSDU_ID_COUNT, le32_to_cpu(free->dw0));
	rx_bytes = GET_FIELD(WF_TX_FREE_DONE_EVENT_V0_RX_BYTE_COUNT, le32_to_cpu(free->dw0));

	rx_bytes -= sizeof(u32);
	ver = GET_FIELD(WF_TX_FREE_DONE_EVENT_VER, le32_to_cpu(free->dw1));
	rx_bytes -= sizeof(u32);

	/*
	 *if (0)
	 *	dump_free_notify_info(free->info);
	 */

	for (dw_idx = 0; i < msdu_cnt && rx_bytes; rx_bytes -= sizeof(u32), dw_idx++) {
		u16 token = 0;

		token = GET_FIELD(WF_TX_FREE_DONE_EVENT_V0_MSDU_ID0,
				le32_to_cpu(free->token[dw_idx]));

		if (token != WF_TX_FREE_DONE_EVENT_V0_MSDU_ID0_MASK) {
			i++;
			mtk_hwifi_free_tx(dev, token, 0);
		}

		if (i >= msdu_cnt)
			break;

		token = GET_FIELD(WF_TX_FREE_DONE_EVENT_V0_MSDU_ID1,
				le32_to_cpu(free->token[dw_idx]));

		if (token != WF_TX_FREE_DONE_EVENT_V0_MSDU_ID0_MASK) {
			i++;
			mtk_hwifi_free_tx(dev, token, 0);
		}

	}

	if (i < msdu_cnt) {
		dev_err(dev->dev, "%s(): token might leak! due to only %d token id freed, but should be %d\n",
				__func__, i, msdu_cnt);
		dbg_bus_free_notify_err_inc(trans);
	}
}

static void
bmac_tx_free_notify_v1(struct mtk_bus_trans *trans, void *free_ptr)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct bmac_tx_free_v0 *free = (struct bmac_tx_free_v0 *)free_ptr;
	u8 msdu_cnt = 0, dw_idx = 0;
	u8 i = 0, ver = 0;
	u32 rx_bytes = 0;

	msdu_cnt = GET_FIELD(WF_TX_FREE_DONE_EVENT_MSDU_ID_COUNT, le32_to_cpu(free->dw0));
	rx_bytes = GET_FIELD(WF_TX_FREE_DONE_EVENT_RX_BYTE_COUNT, le32_to_cpu(free->dw0));

	rx_bytes -= sizeof(u32);
	ver = GET_FIELD(WF_TX_FREE_DONE_EVENT_VER, le32_to_cpu(free->dw1));
	rx_bytes -= sizeof(u32);

	/*
	 *if (0)
	 *	dump_free_notify_info(free->info);
	 */

	for (dw_idx = 0; i < msdu_cnt && rx_bytes; rx_bytes -= sizeof(u32), dw_idx++) {
		u16 token = 0;

		token = GET_FIELD(WF_TX_FREE_DONE_EVENT_V1_V2_MSDU_ID0,
				le32_to_cpu(free->token[dw_idx]));

		if (token != WF_TX_FREE_DONE_EVENT_V1_V2_MSDU_ID0_MASK) {
			i++;
			mtk_hwifi_free_tx(dev, token, 0);
		}
	}

	if (i < msdu_cnt) {
		dev_err(dev->dev, "%s(): token might leak! due to only %d token id freed, but should be %d\n",
				__func__, i, msdu_cnt);
		dbg_bus_free_notify_err_inc(trans);
	}
}

static void
bmac_tx_free_notify_v4(struct mtk_bus_trans *trans, void *free_ptr)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct bmac_tx_free_v4 *free = (struct bmac_tx_free_v4 *)free_ptr;
	struct mtk_tx_status tx_sts = {0};
	u16 msdu_cnt = 0, dw_idx = 0;
	u16 wcid = 0, qid = 0;
	u16 ver = 0, i = 0;
	u32 rx_bytes = 0;

	msdu_cnt = GET_FIELD(WF_TX_FREE_DONE_EVENT_MSDU_ID_COUNT, le32_to_cpu(free->dw0));
	rx_bytes = GET_FIELD(WF_TX_FREE_DONE_EVENT_RX_BYTE_COUNT, le32_to_cpu(free->dw0));

	mtk_dbg_dump(MTK_RX, "TX_FREE_DONE ", (void *)free, rx_bytes);

	rx_bytes -= sizeof(u32);
	ver = GET_FIELD(WF_TX_FREE_DONE_EVENT_VER, le32_to_cpu(free->dw1));
	rx_bytes -= sizeof(u32);

	qid = GET_FIELD(WF_TX_FREE_DONE_EVENT_QID, le32_to_cpu(free->info));
	wcid = GET_FIELD(WF_TX_FREE_DONE_EVENT_WLAN_ID, le32_to_cpu(free->info));
	rx_bytes -= sizeof(u32);
	/*
	 *if (0)
	 *	dump_free_notify_info(free->info);
	 */

	for (dw_idx = 0; i < msdu_cnt && rx_bytes; rx_bytes -= sizeof(u32), dw_idx++) {
		bool is_msdu_dw = true;
		u16 token = 0;

		if (GET_FIELD(WF_TX_FREE_DONE_EVENT_H4, le32_to_cpu(free->token[dw_idx])) ||
			GET_FIELD(WF_TX_FREE_DONE_EVENT_P4, le32_to_cpu(free->token[dw_idx])))
			is_msdu_dw = false;

		if (is_msdu_dw) {
			token = GET_FIELD(WF_TX_FREE_DONE_EVENT_MSDU_ID0,
					le32_to_cpu(free->token[dw_idx]));

			if (token != WF_TX_FREE_DONE_EVENT_MSDU_ID0_MASK) {
				i++;
				mtk_hwifi_free_tx(dev, token, wcid);
			}

			token = GET_FIELD(WF_TX_FREE_DONE_EVENT_MSDU_ID1,
					le32_to_cpu(free->token[dw_idx]));

			if (token != WF_TX_FREE_DONE_EVENT_MSDU_ID0_MASK) {
				i++;
				mtk_hwifi_free_tx(dev, token, wcid);
			}
		} else {
			if (GET_FIELD(WF_TX_FREE_DONE_EVENT_P4, le32_to_cpu(free->token[dw_idx]))) {
				qid = GET_FIELD(WF_TX_FREE_DONE_EVENT_QID,
						le32_to_cpu(free->token[dw_idx]));
				wcid = GET_FIELD(WF_TX_FREE_DONE_EVENT_WLAN_ID,
						le32_to_cpu(free->token[dw_idx]));
			} else {
				tx_sts.wcid = wcid;
				tx_sts.qid = qid;
				tx_sts.stat = GET_FIELD(WF_TX_FREE_DONE_EVENT_STAT,
						le32_to_cpu(free->token[dw_idx]));
				tx_sts.cnt = GET_FIELD(WF_TX_FREE_DONE_EVENT_TX_COUNT,
						le32_to_cpu(free->token[dw_idx]));
				tx_sts.air_latency = GET_FIELD(WF_TX_FREE_DONE_EVENT_AIR_DELAY,
						le32_to_cpu(free->token[dw_idx]));
				tx_sts.mac_latency = GET_FIELD(WF_TX_FREE_DONE_EVENT_TRANSMIT_DELAY,
						le32_to_cpu(free->token[dw_idx]));
				mtk_hwifi_tx_status(dev, wcid, &tx_sts);
			}
		}
	}

	if (i < msdu_cnt) {
		dev_err(dev->dev, "%s(): token might leak! due to only %d token id freed, but should be %d\n",
				__func__, i, msdu_cnt);
		dbg_bus_free_notify_err_inc(trans);
	}
}

static void
bmac_tx_free_notify_v7(struct mtk_bus_trans *trans, void *free_ptr)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct bmac_tx_free_v7 *free = (struct bmac_tx_free_v7 *)free_ptr;
	struct mtk_tx_status tx_sts = {0};
	u16 msdu_cnt = 0, dw_idx = 0;
	u16 wcid = 0, qid = 0, stat = 0;
	u16 i = 0;
	u32 rx_bytes = 0;

	// DW0
	msdu_cnt = GET_FIELD(WF_TX_FREE_DONE_EVENT_MSDU_ID_COUNT, le32_to_cpu(free->dw0));
	rx_bytes = GET_FIELD(WF_TX_FREE_DONE_EVENT_RX_BYTE_COUNT, le32_to_cpu(free->dw0));

	mtk_dbg_dump(MTK_RX, "TX_FREE_DONE ", (void *)free, rx_bytes);

	rx_bytes -= sizeof(u32);

	// DW1
	rx_bytes -= sizeof(u32);

	// DW2
	qid = GET_FIELD(WF_TX_FREE_DONE_EVENT_QID, le32_to_cpu(free->dw2));
	wcid = GET_FIELD(WF_TX_FREE_DONE_EVENT_WLAN_ID, le32_to_cpu(free->dw2));
	rx_bytes -= sizeof(u32);

	// DW3
	rx_bytes -= sizeof(u32);

	// TOKEN
	for (dw_idx = 0; i < msdu_cnt && rx_bytes; rx_bytes -= sizeof(u32), dw_idx++) {
		bool is_msdu_dw = true;
		u16 token = 0;

		if (GET_FIELD(WF_TX_FREE_DONE_EVENT_H4, le32_to_cpu(free->token[dw_idx])) ||
			GET_FIELD(WF_TX_FREE_DONE_EVENT_P4, le32_to_cpu(free->token[dw_idx])))
			is_msdu_dw = false;

		if (is_msdu_dw) {
			token = GET_FIELD(WF_TX_FREE_DONE_EVENT_MSDU_ID0,
					le32_to_cpu(free->token[dw_idx]));

			if (token != WF_TX_FREE_DONE_EVENT_MSDU_ID0_MASK) {
				i++;
				mtk_hwifi_free_tx(dev, token, wcid);
			}

			token = GET_FIELD(WF_TX_FREE_DONE_EVENT_MSDU_ID1,
					le32_to_cpu(free->token[dw_idx]));

			if (token != WF_TX_FREE_DONE_EVENT_MSDU_ID0_MASK) {
				i++;
				mtk_hwifi_free_tx(dev, token, wcid);
			}
		} else {
			if (GET_FIELD(WF_TX_FREE_DONE_EVENT_P4, le32_to_cpu(free->token[dw_idx]))) {
				qid = GET_FIELD(WF_TX_FREE_DONE_EVENT_QID,
						le32_to_cpu(free->token[dw_idx]));
				wcid = GET_FIELD(WF_TX_FREE_DONE_EVENT_WLAN_ID,
						le32_to_cpu(free->token[dw_idx]));
			} else {
				stat = GET_FIELD(WF_TX_FREE_DONE_EVENT_STAT,
						le32_to_cpu(free->token[dw_idx]));
				if (stat <= 0x1) {
					tx_sts.wcid = wcid;
					tx_sts.qid = qid;
					tx_sts.stat = stat;
					tx_sts.cnt = GET_FIELD(WF_TX_FREE_DONE_EVENT_TX_COUNT,
							le32_to_cpu(free->token[dw_idx]));
					tx_sts.air_latency =
						GET_FIELD(WF_TX_FREE_DONE_EVENT_AIR_DELAY,
						le32_to_cpu(free->token[dw_idx]));
					tx_sts.mac_latency =
						GET_FIELD(WF_TX_FREE_DONE_EVENT_TRANSMIT_DELAY,
						le32_to_cpu(free->token[dw_idx]));
					mtk_hwifi_tx_status(dev, wcid, &tx_sts);
				}
			}
		}
	}

	if (i < msdu_cnt) {
		dev_err(dev->dev, "%s(): token might leak! due to only %d token id freed, but should be %d\n",
				__func__, i, msdu_cnt);
		dbg_bus_free_notify_err_inc(trans);
	}
}

void
bmac_tx_free_notify(struct mtk_bus_trans *trans, struct sk_buff *skb)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct bmac_tx_free_v0 *free = (struct bmac_tx_free_v0 *)skb->data;
	u8 ver = 0;

	ver = GET_FIELD(WF_TX_FREE_DONE_EVENT_VER, le32_to_cpu(free->dw1));

	switch (ver) {
	case FREE_NOTIFY_VERSION_0:
		bmac_tx_free_notify_v0(trans, (void *)free);
		break;
	case FREE_NOTIFY_VERSION_1:
	case FREE_NOTIFY_VERSION_2:
		bmac_tx_free_notify_v1(trans, (void *)free);
		break;
	case FREE_NOTIFY_VERSION_3:
		dev_err(dev->dev, "%s(): rx not supported free notify version: %d\n",
			__func__, ver);
		dbg_bus_free_notify_err_inc(trans);
		break;
	case FREE_NOTIFY_VERSION_4:
	case FREE_NOTIFY_VERSION_5:
		bmac_tx_free_notify_v4(trans, (void *)free);
		break;
	case FREE_NOTIFY_VERSION_7:
		bmac_tx_free_notify_v7(trans, (void *)free);
		break;
	default:
		dev_err(dev->dev, "%s(): rx wrong free notify version: %d\n",
			__func__, ver);
		dbg_bus_free_notify_err_inc(trans);
		break;
	}

	dbg_bus_free_notify_inc(trans);
	dev_kfree_skb(skb);
}

static int
bmac_drop_decision(struct mtk_hw_dev *dev, struct sk_buff *skb)
{
	struct mtk_bus_rx_info *rx_info = (struct mtk_bus_rx_info *)skb->cb;

	if (rx_info->wifi_frag)
		return 0;

	if (rx_info->repeat_pkt) {
		rx_info->drop = RXDMAD_INCMD_REASON_DROP;
		return -EINVAL;
	}

	if (rx_info->old_pkt &&
		(!rx_info->ip_frag || !(dev->limit & BIT(LIMIT_NO_DROP_IP_FRAG_OLD_PKT))) &&
		!(dev->limit & BIT(LIMIT_NO_DROP_OLD_PKT))) {
		rx_info->drop = RXDMAD_INCMD_REASON_DROP;
		return -EINVAL;
	}

	return 0;
}

static bool
bmac_need_hif_txd_legacy(struct mtk_hw_dev *dev, void *tx_pkt_info)
{
	return false;
}

static int bmac_write_buf(struct mtk_hw_dev *dev,
	__le32 *buf_info, __le32 *ml_info, u32 ml_shift,
	struct mtk_tk_entry *tk_entry, u32 max_buf_cnt)
{
	u8 i = 0;
	__le32 *_buf_info = buf_info;
	struct sk_buff *pkt_next;
	u32 buf_cnt = skb_queue_len(&tk_entry->tx_q);

	if (WARN_ON_ONCE(buf_cnt > max_buf_cnt))
		return -EINVAL;

	skb_queue_walk(&tk_entry->tx_q, pkt_next) {
		dma_addr_t pkt_dma_addr;

		pkt_dma_addr = dma_map_single(dev->dev,
			pkt_next->data, pkt_next->len, DMA_TO_DEVICE);

		if (unlikely(dma_mapping_error(dev->dev, pkt_dma_addr))) {
			dev_err(dev->dev, "%s(): dma map fail for tkid %d\n",
				__func__, tk_entry->sid.idx);
			goto err;
		}

		if (i % 2 == 0) {
			*_buf_info = SET_FIELD(HIF_TXP_BUF_PTR0_L, pkt_dma_addr);
			*_buf_info = cpu_to_le32(*_buf_info);

			_buf_info++;
			*_buf_info = SET_FIELD(HIF_TXP_BUF_LEN0, pkt_next->len);
#ifdef CONFIG_HWIFI_64BIT_SUPPORT
			*_buf_info |= SET_FIELD(HIF_TXP_BUF_PTR0_H,
					pkt_dma_addr >> DMA_ADDR_H_SHIFT);
#endif
		} else {
			*_buf_info |= SET_FIELD(HIF_TXP_BUF_LEN1, pkt_next->len);
#ifdef CONFIG_HWIFI_64BIT_SUPPORT
			*_buf_info |= SET_FIELD(HIF_TXP_BUF_PTR1_H,
					pkt_dma_addr >> DMA_ADDR_H_SHIFT);
#endif
			*_buf_info = cpu_to_le32(*_buf_info);

			_buf_info++;
			*_buf_info = SET_FIELD(HIF_TXP_BUF_PTR1_L, pkt_dma_addr);
			*_buf_info = cpu_to_le32(*_buf_info);

			_buf_info++;
		}

		*ml_info |= BIT(ml_shift + i);
		i++;
	}

	/* If odd number of pkt, loop above will not call cpu_to_le32 */
	if (buf_cnt % 2)
		*_buf_info = cpu_to_le32(*_buf_info);

	*ml_info =  cpu_to_le32(*ml_info);
	return 0;
err:
	bmac_skb_unmap(dev, buf_info, i);
	return -ENOMEM;
}

int
bmac_write_mac_txd(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
	struct mtk_tk_entry *tk_entry, void *tx_pkt_info)
{
/* TODO: move to txs control header */
#define NIC_TX_DESC_DRIVER_PID_MIN 1
#define NIC_TX_DESC_DRIVER_PID_MAX 127
	struct _TX_BLK *tx_blk = (struct _TX_BLK *)tx_pkt_info;
	struct sk_buff *skb = (struct sk_buff *)tx_blk->pPacket;
	u8 buf_cnt = skb_queue_len(&tk_entry->tx_q);
	__le32 *txd = (__le32 *)tk_entry->txd_ptr;
	struct mtk_mld_sta_entry *mld_sta_entry = NULL;
	struct mtk_hw_bss *bss = sta->bss;
	static uint8_t pid = NIC_TX_DESC_DRIVER_PID_MIN;
	struct mtk_chip_hw_cap *hw_cap = dev->chip_drv->hw_caps;
	bool om_map = false;
	u8 om_idx = bss->omac_idx;
	u8 band_idx = bss->hw_phy->band_idx;
	u8 q_idx;
	u32 pn1 = 0;
	u16 pn2 = 0;

	mld_sta_entry = mtk_wsys_mld_sta_entry_get(sta->mld_sta_idx);
	if (mld_sta_entry &&
	    !TX_BLK_TEST_FLAG(tx_blk, fTX_ForceLink)) {
		sta = tx_blk->UserPriority % 2 ?
			mld_sta_entry->secondary :
			mld_sta_entry->primary;
		bss = sta->bss;

		mld_sta_entry = mtk_wsys_mld_sta_entry_get(sta->mld_sta_idx);
		if (mld_sta_entry && mld_sta_entry->mld_bss->remap_id != OM_REMAP_IDX_NONE) {
			om_map = true;
			om_idx = mld_sta_entry->mld_bss->remap_id;
		} else {
			om_map = false;
			om_idx = bss->omac_idx;
		}
	}

	/* dw0 */
	if (buf_cnt <= 1) {
		txd[0] =
			SET_FIELD(WF_TX_DESCRIPTOR_TX_BYTE_COUNT,
					skb->len + MT_TXD_SIZE);
	} else {
		tx_bytes_calculate(tk_entry, tx_blk);

		txd[0] =
			SET_FIELD(WF_TX_DESCRIPTOR_TX_BYTE_COUNT,
						tx_blk->tx_bytes_len);
	}

	if (tx_blk->QueIdx < 4) {
		q_idx = bss->wmm_idx * MAX_WMM_SETS + tx_blk->QueIdx;
		txd[0] |=
			SET_FIELD(WF_TX_DESCRIPTOR_PKT_FT, MT_TX_TYPE_CT);
	} else if (tx_blk->dot11_subtype == SUBTYPE_BEACON) {
		q_idx = MT_LMAC_BCN0;
		txd[0] |=
			SET_FIELD(WF_TX_DESCRIPTOR_PKT_FT, MT_TX_TYPE_FW);
	} else {
		q_idx = MT_LMAC_ALTX0;
		txd[0] |=
			SET_FIELD(WF_TX_DESCRIPTOR_PKT_FT, MT_TX_TYPE_CT);
	}

	txd[0] |= SET_FIELD(WF_TX_DESCRIPTOR_Q_IDX, q_idx);

	txd[0] = cpu_to_le32(txd[0]);

	/* dw1 */
	txd[1] = SET_FIELD(WF_TX_DESCRIPTOR_MLD_ID, sta->link_wcid);

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_ForceLink) ||
	    (q_idx >= MT_LMAC_ALTX0 && q_idx <= MT_LMAC_BCN0))
		txd[1] |= SET_FIELD(WF_TX_DESCRIPTOR_TGID, band_idx);

	if (!TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS)) {
		txd[1] |=
			SET_FIELD(WF_TX_DESCRIPTOR_HF,
					MT_HDR_FORMAT_802_11) |
			SET_FIELD(WF_TX_DESCRIPTOR_HEADER_LENGTH,
					tx_blk->wifi_hdr_len >> 1);
	} else {
		txd[1] |=
			SET_FIELD(WF_TX_DESCRIPTOR_HF,
					MT_HDR_FORMAT_802_3) |
			SET_FIELD(WF_TX_DESCRIPTOR_ETYP,
					1);
	}

	/* TODO: BIP and Timing measurement */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bMMIE))
		txd[1] |= SET_FIELD(WF_TX_DESCRIPTOR_TID_MGMT_TYPE, 0x8);

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bAddBA)) {
		if (dev->limit & BIT(LIMIT_SET_ADDBA_SSN))
			txd[1] |= SET_FIELD(WF_TX_DESCRIPTOR_TID_MGMT_TYPE, 0);
		else
			txd[1] |= SET_FIELD(WF_TX_DESCRIPTOR_TID_MGMT_TYPE, 2);
	} else {
		txd[1] |= SET_FIELD(WF_TX_DESCRIPTOR_TID_MGMT_TYPE,
				tx_blk->UserPriority);
	}

	txd[1] |= SET_FIELD(WF_TX_DESCRIPTOR_OM, om_idx);

	/* TODO: check fixed rate condition again */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_ForceRate))
		txd[1] |= BIT(WF_TX_DESCRIPTOR_FR_SHIFT);

	txd[1] = cpu_to_le32(txd[1]);

	/* dw2 */
	txd[2] =
		SET_FIELD(WF_TX_DESCRIPTOR_SUBTYPE, tx_blk->dot11_subtype) |
		SET_FIELD(WF_TX_DESCRIPTOR_FTYPE, tx_blk->dot11_type);

	/* MAC TXD above V5 */
	if (om_map) {
		if (hw_cap->mac_txd_ver >= MAC_TXD_V5)
			txd[2] |= BIT(WF_TX_DESCRIPTOR_V5_OM_MAP_SHIFT);
		else
			txd[2] |= BIT(WF_TX_DESCRIPTOR_OM_MAP_SHIFT);
	}

	if (tx_blk->HdrPadLen) {
		if (!TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS))
			txd[2] |= SET_FIELD(WF_TX_DESCRIPTOR_HEADER_PADDING,
							MT_HDR_PAD_TAIL_PAD);
		else
			txd[2] |= SET_FIELD(WF_TX_DESCRIPTOR_HEADER_PADDING,
							MT_HDR_PAD_HEAD_PAD);
	}
	if (tx_blk->FragIdx)
		txd[2] |= SET_FIELD(WF_TX_DESCRIPTOR_FRAG, tx_blk->FragIdx);

	if (tx_blk->txpwr_offset)
		txd[2] |= (((tx_blk->txpwr_offset) & 0x3f) << WF_TX_DESCRIPTOR_POWER_OFFSET_SHIFT);

	if (tx_blk->ApplyRetryLimit)
		txd[2] |= SET_FIELD(WF_TX_DESCRIPTOR_REMAINING_TX_TIME, tx_blk->RemainingTxTime);

	txd[2] = cpu_to_le32(txd[2]);

	/* dw3 */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bAckRequired)) {
		txd[3] = SET_FIELD(WF_TX_DESCRIPTOR_REMAINING_TX_COUNT,
					MT_TX_LONG_RETRY);
		/* for testmode usage */
		if (TX_BLK_TEST_FLAG(tx_blk, fTX_bNoRetry)) {
			txd[3] = SET_FIELD(WF_TX_DESCRIPTOR_REMAINING_TX_COUNT,
						0x1);
			mtk_dbg(MTK_TXD, "fTX_bNoRetry\n");
		} else if (TX_BLK_TEST_FLAG(tx_blk, fTX_bRetryUnlimit)) {
			txd[3] = SET_FIELD(WF_TX_DESCRIPTOR_REMAINING_TX_COUNT,
						MT_TX_RETRY_UNLIMIT);
			mtk_dbg(MTK_TXD, "fTX_bRetryUnlimit\n");
		} else if (tx_blk->ApplyRetryLimit) {
			if (tx_blk->RemainingTxCount > MT_TX_RETRY_UNLIMIT)
				tx_blk->RemainingTxCount = MT_TX_RETRY_UNLIMIT;

			txd[3] = SET_FIELD(WF_TX_DESCRIPTOR_REMAINING_TX_COUNT,
				tx_blk->RemainingTxCount);
		}

	} else {
		txd[3] =
			BIT(WF_TX_DESCRIPTOR_NA_SHIFT) |
			SET_FIELD(WF_TX_DESCRIPTOR_REMAINING_TX_COUNT,
						MT_TX_RETRY_UNLIMIT);
	}

	if (TX_BLK_TEST_FLAG2(tx_blk, fTX_bSnVld)) {
		txd[3] |= SET_FIELD(WF_TX_DESCRIPTOR_SN, tx_blk->sn);
		txd[3] |= SET_FIELD(WF_TX_DESCRIPTOR_SN_VLD, 0x1);
	}

	if (!IS_CIPHER_NONE(tx_blk->CipherAlg))
		txd[3] |= BIT(WF_TX_DESCRIPTOR_PF_SHIFT);

	if (tx_blk->TxFrameType == TX_BMC_FRAME)
		txd[3] |= BIT(WF_TX_DESCRIPTOR_BM_SHIFT);

	if ((tx_blk->TxFrameType == TX_FRAG_FRAME) ||
		(tx_blk->ApplyBaPowerTxs && tx_blk->BaDisable))
		txd[3] |= BIT(WF_TX_DESCRIPTOR_BA_DIS_SHIFT);

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bSwPN)) {
		txd[3] |= BIT(WF_TX_DESCRIPTOR_PN_VLD_SHIFT);
		memcpy(&pn1, &tx_blk->sw_pn[0], sizeof(u32));
		memcpy(&pn2, &tx_blk->sw_pn[4], sizeof(u16));
	}

	if (is_amsdu_capable(skb) &&
			(tx_blk->TxFrameType == TX_AMSDU_FRAME))
		txd[3] |= BIT(WF_TX_DESCRIPTOR_HW_AMSDU_CAP_SHIFT);

	if (TX_BLK_TEST_FLAG2(tx_blk, fTX_bPsmBySw))
		txd[3] |= BIT(WF_TX_DESCRIPTOR_PM_SHIFT);

	txd[3] = cpu_to_le32(txd[3]);

	/* dw4 - pn[31:0] */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bSwPN))
		txd[4] = pn1;
	txd[4] = cpu_to_le32(txd[4]);

	/* dw5 */
	if (tx_blk->TxS2Host || tx_blk->ApplyBaPowerTxs) {
		txd[5] = SET_FIELD(WF_TX_DESCRIPTOR_PID, tx_blk->Pid);
	} else {
		txd[5] = SET_FIELD(WF_TX_DESCRIPTOR_PID, pid++);
		if (pid > NIC_TX_DESC_DRIVER_PID_MAX)
			pid = NIC_TX_DESC_DRIVER_PID_MIN;
	}

	if (tx_blk->TxS2Mcu)
		txd[5] |= BIT(WF_TX_DESCRIPTOR_TXS2M_SHIFT);

	if (tx_blk->TxS2Host)
		txd[5] |= BIT(WF_TX_DESCRIPTOR_TXS2H_SHIFT);

	if (tx_blk->TxSFormat)
		txd[5] |= BIT(WF_TX_DESCRIPTOR_TXSFM_SHIFT);

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_ForceLink))
		txd[5] |= BIT(WF_TX_DESCRIPTOR_FL_SHIFT);

	/* pn[47:32] */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bSwPN))
		txd[5] |= SET_FIELD(WF_TX_DESCRIPTOR_PN_47_32_, pn2);

	txd[5] = cpu_to_le32(txd[5]);

	/* dw6 */
	txd[6] = BIT(WF_TX_DESCRIPTOR_DAS_SHIFT);

	if (buf_cnt > 1)
		txd[6] |= BIT(WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB_SHIFT);

	/* For non-IGMP clone BMC frame */
	if ((tx_blk->TxFrameType == TX_BMC_FRAME) &&
		!TX_BLK_TEST_FLAG(tx_blk, fTX_MCAST_CLONE))
		txd[6] &= ~WF_TX_DESCRIPTOR_DAS_MASK;

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_ForceLink) ||
	    (q_idx >= MT_LMAC_ALTX0 && q_idx <= MT_LMAC_BCN0) ||
	    TX_BLK_TEST_FLAG2(tx_blk, fTX_bSwSTA_WithOutWTBL))
		txd[6] |= BIT(WF_TX_DESCRIPTOR_DIS_MAT_SHIFT);

	if ((tx_blk->dot11_type == FC_TYPE_MGMT) && (tx_blk->dot11_subtype == SUBTYPE_PROBE_RSP)) {
		if (hw_cap->mac_txd_ver >= MAC_TXD_V6) {
			txd[6] |= SET_FIELD(WF_TX_DESCRIPTOR_V6_TIMESTAMP_OFFSET_IDX, 0);
			txd[6] |= SET_FIELD(WF_TX_DESCRIPTOR_V6_TIMESTAMP_OFFSET_EN, 1);
		} else if (hw_cap->mac_txd_ver >= MAC_TXD_V2) {
			txd[6] |= SET_FIELD(WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_IDX, 0);
			txd[6] |= SET_FIELD(WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_EN, 1);
		}
	}

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bAddBA)) {
		if (hw_cap->mac_txd_ver >= MAC_TXD_V6)
			txd[6] |= SET_FIELD(WF_TX_DESCRIPTOR_V6_TID_ADDBA, tx_blk->UserPriority);
	}

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_ForceRate)) {
		txd[6] |= SET_FIELD(WF_TX_DESCRIPTOR_FIXED_RATE_IDX,
					 tx_blk->fr_tbl_idx);
		txd[6] |= SET_FIELD(WF_TX_DESCRIPTOR_BW, tx_blk->fr_bw);
	}

	if (hw_cap->mac_txd_ver >= MAC_TXD_V5) {
		if (tx_blk->dot11_type != FC_TYPE_MGMT) {
			txd[6] |= SET_FIELD(WF_TX_DESCRIPTOR_V5_MSDU_COUNT, buf_cnt);
		}
	} else {
		txd[6] |= SET_FIELD(WF_TX_DESCRIPTOR_MSDU_COUNT, buf_cnt);
	}

	txd[6] = cpu_to_le32(txd[6]);

	/* dw7 */
	txd[7] = SET_FIELD(WF_TX_DESCRIPTOR_TXD_LEN, MT_TXD_LEN_1_PAGE);

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bAteAgg)) {
		txd[7] |= SET_FIELD(WF_TX_DESCRIPTOR_SUBTYPE,
							 tx_blk->dot11_subtype);
		txd[7] |= SET_FIELD(WF_TX_DESCRIPTOR_FTYPE,
							 tx_blk->dot11_type);
	}

	if (dev->limit & BIT(LIMIT_SET_ADDBA_TID) &&
	    TX_BLK_TEST_FLAG(tx_blk, fTX_bAddBA)) {
		txd[7] |= BIT(WF_TX_DESCRIPTOR_HM_SHIFT);
		tk_entry->write_txp = dev->hw_ops.write_mac_txp;
		tk_entry->skb_unmap_txp = dev->hw_ops.skb_unmap_mac_txp;
	}

	txd[7] = cpu_to_le32(txd[7]);

#ifdef CONFIG_HWIFI_DBG
	bmac_dump_txd(txd);
#endif

	return 0;
}

int
bmac_write_hif_txd(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
	struct mtk_tk_entry *tk_entry, void *tx_pkt_info)
{
	__le32 *txd = (__le32 *)tk_entry->txd_ptr;
	struct _TX_BLK *tx_blk = (struct _TX_BLK *)tx_pkt_info;

	memset(tk_entry->txd_ptr, 0, MT_TXD_SIZE);

#ifdef CONFIG_HWIFI_DBG
	bmac_dump_txd((__le32 *)tk_entry->txd_ptr);
#endif

	if (TX_BLK_TEST_FLAG2(tx_blk, fTX_bSnVld)) {
		txd[3] |= SET_FIELD(WF_TX_DESCRIPTOR_SN, tx_blk->sn);
		txd[3] |= SET_FIELD(WF_TX_DESCRIPTOR_SN_VLD, 0x1);
	}

	txd[3] = cpu_to_le32(txd[3]);

	return 0;
}

int
bmac_write_hif_txd_v2(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
	struct mtk_tk_entry *tk_entry, void *tx_pkt_info)
{
	__le32 *txd = (__le32 *)tk_entry->txd_ptr;
	u8 buf_cnt = skb_queue_len(&tk_entry->tx_q);
	struct _TX_BLK *tx_blk = (struct _TX_BLK *)tx_pkt_info;

	memset((void *)txd, 0, MT_TXD_SIZE);

	/* only amsdu need to calculated by host */
	if (buf_cnt > 1) {
		tx_bytes_calculate(tk_entry, tx_blk);

		txd[0] =
			SET_FIELD(WF_TX_DESCRIPTOR_TX_BYTE_COUNT,
						tx_blk->tx_bytes_len);
	}

	txd[0] |= SET_FIELD(HIF_TXD_VERSION, 2);
	txd[0] = cpu_to_le32(txd[0]);

#ifdef CONFIG_HWIFI_DBG
	bmac_dump_txd(txd);
#endif

	return 0;
}

int
bmac_write_hif_txd_v3(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
	struct mtk_tk_entry *tk_entry, void *tx_pkt_info)
{
	__le32 *txd = (__le32 *)tk_entry->txd_ptr;
	u8 buf_cnt = skb_queue_len(&tk_entry->tx_q);
	struct _TX_BLK *tx_blk = (struct _TX_BLK *)tx_pkt_info;

	memset((void *)txd, 0, MT_TXD_SIZE);

	/* only amsdu need to calculated by host */
	if (buf_cnt > 1) {
		tx_bytes_calculate(tk_entry, tx_blk);

		txd[0] =
			SET_FIELD(WF_TX_DESCRIPTOR_TX_BYTE_COUNT,
						tx_blk->tx_bytes_len);
	}

	txd[0] |= SET_FIELD(HIF_TXD_VERSION, 3);
	txd[0] = cpu_to_le32(txd[0]);

#ifdef CONFIG_HWIFI_DBG
	bmac_dump_txd(txd);
#endif

	return 0;
}


int
bmac_write_mac_txp(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
				void *tx_pkt_info)
{
	__le32 *txp = (__le32 *)(tk_entry->txd_ptr + MT_TXD_SIZE);
	struct _TX_BLK *tx_blk = (struct _TX_BLK *)tx_pkt_info;
	struct sk_buff *skb = tk_entry->tx_q.next;

	memset(txp, 0, MAC_TXP_SIZE);

	tk_entry->pkt_pa = dma_map_single(dev->dev,
		skb->data, skb->len, DMA_TO_DEVICE);

	tk_entry->fbuf_dma_size += MAC_TXP_SIZE;

	if (unlikely(dma_mapping_error(dev->dev, tk_entry->pkt_pa))) {
		dev_err(dev->dev, "%s(): dma map fail for tkid %d\n",
			__func__, tk_entry->sid.idx);
		return -ENOMEM;
	}

	/* dw0 */
	txp[0] = SET_FIELD(MAC_TXP_TOKEN_ID0, tk_entry->sid.idx);
	txp[0] |= MAC_TXP_TOKEN_ID0_VALID_MASK;

	txp[0] = cpu_to_le32(txp[0]);

	/* dw1 */
	if (dev->limit & BIT(LIMIT_SET_ADDBA_TID) &&
	    TX_BLK_TEST_FLAG(tx_blk, fTX_bAddBA)) {
		txp[1] = SET_FIELD(MAC_TXP_TID_ADDBA, tx_blk->UserPriority);
		txp[1] = cpu_to_le32(txp[1]);
	}

	/* dw2 */
	txp[2] = cpu_to_le32(SET_FIELD(MAC_TXP_BUF_PTR0_L, tk_entry->pkt_pa));

	/* dw3 */
	txp[3] = SET_FIELD(MAC_TXP_BUF_LEN0, skb->len);
#ifdef CONFIG_HWIFI_64BIT_SUPPORT
	txp[3] |= SET_FIELD(MAC_TXP_BUF_PTR0_H, tk_entry->pkt_pa >> DMA_ADDR_H_SHIFT);
#endif
	txp[3] |= MAC_TXP_BUF_ML0_MASK;

	txp[3] = cpu_to_le32(txp[3]);

#ifdef CONFIG_HWIFI_DBG
	bmac_dump_mac_txp((__le32 *)txp);
#endif

	return 0;
}

int
bmac_write_mac_txp_pao(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
				void *tx_pkt_info)
{
	/* TODO: change to more page */
	tk_entry->fbuf_dma_size += MAC_TXP_SIZE;

	return 0;
}

int
bmac_write_hif_txp(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
				void *tx_pkt_info)
{
	struct hif_txp *txp;
	struct mtk_mld_sta_entry *mld_sta_entry = NULL;
	struct mtk_hw_sta *sta = tk_entry->sta;
	struct mtk_hw_bss *bss = sta->bss;
	struct _TX_BLK *tx_blk = (struct _TX_BLK *)tx_pkt_info;
	struct sk_buff *skb = tk_entry->tx_q.next;
	UINT32 index = 0, pl_cnt = 0, last_pl_len;

	txp = (struct hif_txp *)(tk_entry->txd_ptr + MT_TXD_SIZE);
	memset(txp, 0, sizeof(*txp));

	tk_entry->pkt_pa = dma_map_single(dev->dev,
		skb->data, skb->len, DMA_TO_DEVICE);

	tk_entry->fbuf_dma_size += HIF_TXP_SIZE;

	if (unlikely(dma_mapping_error(dev->dev, tk_entry->pkt_pa))) {
		dev_err(dev->dev, "%s(): dma map fail for tkid %d\n",
			__func__, tk_entry->sid.idx);
		return -ENOMEM;
	}

	/* for testmode, s/w generate a large msdu then separate it */
	pl_cnt = skb->len / HIF_TXP_BUF_LEN_MASK +
			 ((skb->len % HIF_TXP_BUF_LEN_MASK) ? 1 : 0);
	last_pl_len = ((skb->len % HIF_TXP_BUF_LEN_MASK) ?
		(skb->len % HIF_TXP_BUF_LEN_MASK) : HIF_TXP_BUF_LEN_MASK);

	for (index = 0; index < pl_cnt; index++) {
		txp->buf[index] = cpu_to_le32(SET_FIELD(HIF_TXP_BUF_PTR,
			tk_entry->pkt_pa+HIF_TXP_BUF_LEN_MASK*index));

		if (index == (pl_cnt - 1))
			txp->len[index] = SET_FIELD(HIF_TXP_BUF_LEN,
				last_pl_len);
		else
			txp->len[index] = SET_FIELD(HIF_TXP_BUF_LEN,
				HIF_TXP_BUF_LEN_MASK);

#ifdef CONFIG_HWIFI_64BIT_SUPPORT
		txp->len[index] |= SET_FIELD(HIF_TXP_BUF_PTR_H,
			tk_entry->pkt_pa >> DMA_ADDR_H_SHIFT);
#endif
		txp->len[index] = cpu_to_le16(txp->len[index]);
	}

	mld_sta_entry = mtk_wsys_mld_sta_entry_get(sta->mld_sta_idx);
	if (mld_sta_entry &&
	    !TX_BLK_TEST_FLAG(tx_blk, fTX_ForceLink)) {
		sta = tx_blk->UserPriority % 2 ?
			mld_sta_entry->secondary :
			mld_sta_entry->primary;
		bss = sta->bss;
	}

	txp->nbuf = pl_cnt;
	txp->token = cpu_to_le16(tk_entry->sid.idx);
	txp->bss_idx = bss->fw_idx;

	if ((tx_blk->TxFrameType == TX_BMC_FRAME) && (dev->limit & BIT(LIMIT_SET_BMC_WCID)))
		txp->rept_wds_wcid = 0xfff;
	else
		txp->rept_wds_wcid = sta->link_wcid;

	txp->rept_wds_wcid = cpu_to_le16(txp->rept_wds_wcid);

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_CT_WithTxD))
		txp->flags = MT_CT_INFO_APPLY_TXD;
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bClearEAPFrame))
		txp->flags |= MT_CT_INFO_NONE_CIPHER_FRAME;
	if (tx_blk->dot11_type == FC_TYPE_MGMT)
		txp->flags |= MT_CT_INFO_MGMT_FRAME;
	if (GET_PACKET_HS2_TX(skb))
		txp->flags |= MT_CT_INFO_HSR2_TX;
	txp->flags |= MT_CT_INFO_PKT_FR_HOST;
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_HIGH_PRIO))
		txp->flags |= HIF_TXP_PRIORITY_MASK;
	if (TX_BLK_TEST_FLAG2(tx_blk, fTX_bSnVld))
		txp->flags |= HIF_TXP_OVERRIDE_MASK;
	txp->flags = cpu_to_le16(txp->flags);

	if (GET_PACKET_HIGH_PRIO(skb))
		tk_entry->is_prior = 1;
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_ForceRate))
		tk_entry->is_fixed_rate = 1;
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_AmsduInAmpdu))
		tk_entry->amsdu_en = 1;

	tk_entry->tid = tx_blk->UserPriority;
	tk_entry->bss_idx = bss->fw_idx;
	tk_entry->wcid = sta->link_wcid;
	/* non-802.11 frame */
	tk_entry->hf = 0x0;

#ifdef CONFIG_HWIFI_DBG
	bmac_dump_hif_txp((__le32 *)txp);
#endif
	return 0;
}


int
bmac_write_hif_txp_v2(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
				void *tx_pkt_info)
{
	struct mtk_mld_sta_entry *mld_sta_entry = NULL;
	struct mtk_hw_sta *sta = tk_entry->sta;
	struct mtk_hw_bss *bss = sta->bss;
	__le32 *buf_info, *ml_info;
	__le32 *txp = (__le32 *)(tk_entry->txd_ptr + MT_TXD_SIZE);
	struct _TX_BLK *tx_blk = (struct _TX_BLK *)tx_pkt_info;
	int buf_cnt = skb_queue_len(&tk_entry->tx_q);
	struct sk_buff *fskb = tk_entry->tx_q.next;
	struct mtk_chip_hw_cap *hw_cap = dev->chip_drv->hw_caps;
	int ret;

	memset(txp, 0, HIF_TXP_V2_SIZE);
	tk_entry->fbuf_dma_size += HIF_TXP_V2_SIZE;

	mld_sta_entry = mtk_wsys_mld_sta_entry_get(sta->mld_sta_idx);
	if (mld_sta_entry &&
	    !TX_BLK_TEST_FLAG(tx_blk, fTX_ForceLink)) {
		sta = tx_blk->UserPriority % 2 ?
			mld_sta_entry->secondary :
			mld_sta_entry->primary;
		bss = sta->bss;
	}

	/* dw0 */
	if (GET_PACKET_HIGH_PRIO(fskb)) {
		if (hw_cap->hif_txd_ver == HIF_TXD_V2_0)
			txp[0] = HIF_TXP_V2_0_PRIORITY_MASK;
		else
			txp[0] = HIF_TXP_PRIORITY_MASK;

		tk_entry->is_prior = 1;
	}

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_ForceRate)) {
		txp[0] |= HIF_TXP_FIXED_RATE_MASK;
		tk_entry->is_fixed_rate = 1;
	}

	if (GET_PACKET_TCP(fskb))
		txp[0] = HIF_TXP_TCP_MASK;

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bClearEAPFrame))
		txp[0] |= HIF_TXP_NON_CIPHER_MASK;

	if (GET_PACKET_VLAN(fskb))
		txp[0] |= HIF_TXP_VLAN_MASK;

	if (tx_blk->TxFrameType == TX_BMC_FRAME) {
		if (fskb->pkt_type == PACKET_MULTICAST)
			txp[0] |= SET_FIELD(HIF_TXP_BC_MC_FLAG, 1);
		else if (fskb->pkt_type == PACKET_BROADCAST)
			txp[0] |= SET_FIELD(HIF_TXP_BC_MC_FLAG, 2);
	} else {
		txp[0] |= SET_FIELD(HIF_TXP_BC_MC_FLAG, 0);
	}

	txp[0] |= HIF_TXP_FR_HOST_MASK;

	if (ntohs(fskb->protocol) > 1500)
		txp[0] |= HIF_TXP_ETYPE_MASK;

	if (buf_cnt > 1)
		txp[0] |= HIF_TXP_AMSDU_MASK;

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_MCAST_CLONE))
		txp[0] |= HIF_TXP_MC_CLONE_MASK;

	txp[0] |= SET_FIELD(HIF_TXP_TOKEN_ID, tk_entry->sid.idx);

	txp[0] = cpu_to_le32(txp[0]);

	/* dw1 */
	txp[1] = SET_FIELD(HIF_TXP_BSS_IDX, bss->fw_idx);
	txp[1] |= SET_FIELD(HIF_TXP_USER_PRIORITY, tx_blk->UserPriority);

	/* TODO: buf_nums need to refine if scatter and gather support */
	txp[1] |= SET_FIELD(HIF_TXP_BUF_NUM, buf_cnt);

	txp[1] |= SET_FIELD(HIF_TXP_MSDU_CNT, buf_cnt);

	/* TODO: src */
	txp[1] = cpu_to_le32(txp[1]);

	/* dw2 */
	txp[2] |= SET_FIELD(HIF_TXP_ETH_TYPE, fskb->protocol);

	if ((tx_blk->TxFrameType == TX_BMC_FRAME) && (dev->limit & BIT(LIMIT_SET_BMC_WCID)))
		txp[2] |= SET_FIELD(HIF_TXP_WLAN_IDX, 0xfff);
	else
		txp[2] |= SET_FIELD(HIF_TXP_WLAN_IDX, sta->link_wcid);

	txp[2] = cpu_to_le32(txp[2]);

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_AmsduInAmpdu))
		tk_entry->amsdu_en = 1;

	tk_entry->tid = tx_blk->UserPriority;
	tk_entry->bss_idx = bss->fw_idx;
	tk_entry->wcid = sta->link_wcid;
	/* non-802.11 frame */
	tk_entry->hf = 0x0;

	buf_info = &txp[4];
	ml_info = &txp[23];

	ret = bmac_write_buf(dev, buf_info, ml_info, HIF_TXP_ML_SHIFT, tk_entry, 13);
	if (ret)
		goto end;

#ifdef CONFIG_HWIFI_DBG
	bmac_dump_hif_txp_v2(txp);
#endif
end:
	return ret;
}

int
bmac_write_hif_txp_v3(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
				void *tx_pkt_info)
{
	struct mtk_mld_sta_entry *mld_sta_entry = NULL;
	struct mtk_hw_sta *sta = tk_entry->sta;
	struct mtk_hw_bss *bss = sta->bss;
	__le32 *buf_info, *ml_info;
	__le32 *txp = (__le32 *)(tk_entry->txd_ptr + MT_TXD_SIZE);
	struct _TX_BLK *tx_blk = (struct _TX_BLK *)tx_pkt_info;
	int buf_cnt = skb_queue_len(&tk_entry->tx_q);
	struct sk_buff *fskb = tk_entry->tx_q.next;
	int ret;

	memset(txp, 0, HIF_TXP_V3_SIZE);
	tk_entry->fbuf_dma_size += HIF_TXP_V3_SIZE;

	mld_sta_entry = mtk_wsys_mld_sta_entry_get(sta->mld_sta_idx);
	if (mld_sta_entry &&
	    !TX_BLK_TEST_FLAG(tx_blk, fTX_ForceLink)) {
		sta = tx_blk->UserPriority % 2 ?
			mld_sta_entry->secondary :
			mld_sta_entry->primary;
		bss = sta->bss;
	}

	/* dw0 */
	if (GET_PACKET_HIGH_PRIO(fskb)) {
		txp[0] = HIF_TXP_PRIORITY_MASK;
		tk_entry->is_prior = 1;
	}

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_ForceRate)) {
		txp[0] |= HIF_TXP_FIXED_RATE_MASK;
		tk_entry->is_fixed_rate = 1;
	}

	if (GET_PACKET_TCP(fskb))
		txp[0] = HIF_TXP_TCP_MASK;

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bClearEAPFrame))
		txp[0] |= HIF_TXP_NON_CIPHER_MASK;

	if (GET_PACKET_VLAN(fskb))
		txp[0] |= HIF_TXP_VLAN_MASK;

	if (tx_blk->TxFrameType == TX_BMC_FRAME) {
		if (fskb->pkt_type == PACKET_MULTICAST)
			txp[0] |= SET_FIELD(HIF_TXP_BC_MC_FLAG, 1);
		else if (fskb->pkt_type == PACKET_BROADCAST)
			txp[0] |= SET_FIELD(HIF_TXP_BC_MC_FLAG, 2);
	} else {
		txp[0] |= SET_FIELD(HIF_TXP_BC_MC_FLAG, 0);
	}

	txp[0] |= HIF_TXP_FR_HOST_MASK;

	if (ntohs(fskb->protocol) > 1500)
		txp[0] |= HIF_TXP_ETYPE_MASK;

	if (buf_cnt > 1)
		txp[0] |= HIF_TXP_AMSDU_MASK;

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_MCAST_CLONE))
		txp[0] |= HIF_TXP_MC_CLONE_MASK;

	txp[0] |= SET_FIELD(HIF_TXP_TOKEN_ID, tk_entry->sid.idx);

	txp[0] = cpu_to_le32(txp[0]);

	/* dw1 */
	txp[1] = SET_FIELD(HIF_TXP_BSS_IDX, bss->fw_idx);
	txp[1] |= SET_FIELD(HIF_TXP_USER_PRIORITY, tx_blk->UserPriority);

	/* TODO: buf_nums need to refine if scatter and gather support */
	txp[1] |= SET_FIELD(HIF_TXP_BUF_NUM, buf_cnt);

	txp[1] |= SET_FIELD(HIF_TXP_MSDU_CNT, buf_cnt);

	/* TODO: src */
	txp[1] = cpu_to_le32(txp[1]);

	/* dw2 */
	txp[2] |= SET_FIELD(HIF_TXP_ETH_TYPE, fskb->protocol);

	if ((tx_blk->TxFrameType == TX_BMC_FRAME) && (dev->limit & BIT(LIMIT_SET_BMC_WCID)))
		txp[2] |= SET_FIELD(HIF_TXP_WLAN_IDX, 0xfff);
	else
		txp[2] |= SET_FIELD(HIF_TXP_WLAN_IDX, sta->link_wcid);

	txp[2] = cpu_to_le32(txp[2]);

	buf_info = &txp[5];
	ml_info = &txp[4];

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_AmsduInAmpdu))
		tk_entry->amsdu_en = 1;

	tk_entry->tid = tx_blk->UserPriority;
	tk_entry->bss_idx = bss->fw_idx;
	tk_entry->wcid = sta->link_wcid;
	/* non-802.11 frame */
	tk_entry->hf = 0x0;

	ret = bmac_write_buf(dev, buf_info, ml_info, HIF_TXP_V3_ML_SHIFT, tk_entry, 12);
	if (ret)
		goto end;

#ifdef CONFIG_HWIFI_DBG
	bmac_dump_hif_txp_v3(txp);
#endif
end:
	return ret;
}


int
bmac_write_txd(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
	struct mtk_tk_entry *tk_entry, void *tx_pkt_info)
{
	struct _TX_BLK *tx_blk = (struct _TX_BLK *)tx_pkt_info;

	tk_entry->fbuf_dma_size = MT_TXD_SIZE;

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_CT_WithTxD)) {
		tk_entry->write_txp = dev->hw_ops.write_hif_txp_legacy;
		tk_entry->skb_unmap_txp = dev->hw_ops.skb_unmap_hif_txp_legacy;
		return dev->hw_ops.write_mac_txd(dev, sta, tk_entry, tx_pkt_info);
	}

	if (bmac_need_hif_txd_legacy(dev, tx_pkt_info)) {
		tk_entry->write_txp = dev->hw_ops.write_hif_txp_legacy;
		tk_entry->skb_unmap_txp = dev->hw_ops.skb_unmap_hif_txp_legacy;
		return dev->hw_ops.write_hif_txd_legacy(dev, sta, tk_entry, tx_pkt_info);
	}

	tk_entry->write_txp = dev->hw_ops.write_hif_txp;
	tk_entry->skb_unmap_txp = dev->hw_ops.skb_unmap_hif_txp;
	return dev->hw_ops.write_hif_txd(dev, sta, tk_entry, tx_pkt_info);
}

int
bmac_write_txp(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
				void *tx_pkt_info)
{
	if (unlikely(!tk_entry->write_txp)) {
		dev_err(dev->dev, "%s(): Write TXP not found\n", __func__);
		return -EINVAL;
	}

	return tk_entry->write_txp(dev, tk_entry, tx_pkt_info);
}

static int
bmac_mac_rxd_radiotap_parser(struct mtk_hw_dev *dev, struct sk_buff *skb,
				struct _RX_BLK *rx_blk)
{
	struct bmac_rxd_g_0 *rxd_grp0 = NULL;
	struct bmac_rxd_g_1 *rxd_grp1 = NULL;
	struct bmac_rxd_g_2 *rxd_grp2 = NULL;
	struct bmac_rxd_g_3 *rxd_grp3 = NULL;
	struct bmac_rxd_g_4 *rxd_grp4 = NULL;
	struct bmac_rxd_g_5 *rxd_grp5 = NULL;
	struct IEEE80211_RADIOTAP_INFO radiotap_info = {0};
	u8 *rmac_info;
	u16 rxd_len = (RMAC_INFO_BASE_SIZE * 4);
	u8 BandIdx = 0;
	u32 u4AmpduRefNum = 0;
	u32 rxd0 = 0;
	u32 rxd1 = 0;
	u32 rxd2 = 0;
	u32 rxd3 = 0;
	u32 rxd16 = 0;
	u32 rxd20 = 0;
	u32 rxd24 = 0;
	u32 rxd25 = 0;
	u32 rxd26 = 0;
	u32 rxd28 = 0;
	u32 rxd29 = 0;
	u32 rxd30 = 0;
	u32 rxd33 = 0;
	u32 rxd36 = 0;
	u32 rxd37 = 0;
	u32 rxd41 = 0;

	if (rx_blk->sniffermode == 0)
		return 0;

	rmac_info =  (u8 *)skb->data;
	rxd_grp0 = (struct bmac_rxd_g_0 *)rmac_info;
	rxd0 = le32_to_cpu(rxd_grp0->rxd_0);
	rxd1 = le32_to_cpu(rxd_grp0->rxd_1);
	rxd2 = le32_to_cpu(rxd_grp0->rxd_2);
	rxd3 = le32_to_cpu(rxd_grp0->rxd_3);
	skb->len = GET_FIELD(WF_RX_DESCRIPTOR_RX_BYTE_COUNT, rxd0);

	/* Group4 */
	if (GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, rxd1) &
					BMAC_GROUP_VLD_4) {
		rxd_grp4 = (struct bmac_rxd_g_4 *)(rmac_info + rxd_len);
		rxd_len += (RMAC_INFO_GRP_4_SIZE * 4);
	}

	/* Group1 */
	if (GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, rxd1) &
					BMAC_GROUP_VLD_1) {
		rxd_grp1 = (struct bmac_rxd_g_1 *)(rmac_info + rxd_len);
		rxd_len += (RMAC_INFO_GRP_1_SIZE * 4);
	}

	/* Group2 */
	if (GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, rxd1) &
					BMAC_GROUP_VLD_2) {
		rxd_grp2 = (struct bmac_rxd_g_2 *)(rmac_info + rxd_len);
		rxd_len += (RMAC_INFO_GRP_2_SIZE * 4);
	}

	/* Group3 */
	if (GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, rxd1) &
					BMAC_GROUP_VLD_3) {
		rxd_grp3 = (struct bmac_rxd_g_3 *)(rmac_info + rxd_len);
		rxd_len += (RMAC_INFO_GRP_3_SIZE * 4);
	}

	/* Group5 */
	if (GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, rxd1) &
					BMAC_GROUP_VLD_5) {
		rxd_grp5 = (struct bmac_rxd_g_5 *)(rmac_info + rxd_len);
		rxd_len += (RMAC_INFO_GRP_5_SIZE * 4);
	}

	if ((rxd_grp2 == NULL) || (rxd_grp3 == NULL) || (rxd_grp5 == NULL))
		return 0;

	rxd16 = le32_to_cpu(rxd_grp2->rxd_16);
	rxd20 = le32_to_cpu(rxd_grp3->rxd_20);
	rxd24 = le32_to_cpu(rxd_grp5->rxd_24);
	rxd25 = le32_to_cpu(rxd_grp5->rxd_25);
	rxd26 = le32_to_cpu(rxd_grp5->rxd_26);
	rxd28 = le32_to_cpu(rxd_grp5->rxd_28);
	rxd29 = le32_to_cpu(rxd_grp5->rxd_29);
	rxd30 = le32_to_cpu(rxd_grp5->rxd_30);
	rxd33 = le32_to_cpu(rxd_grp5->rxd_33);
	rxd36 = le32_to_cpu(rxd_grp5->rxd_36);
	rxd37 = le32_to_cpu(rxd_grp5->rxd_37);
	rxd41 = le32_to_cpu(rxd_grp5->rxd_41);


	if (GET_FIELD(WF_RX_DESCRIPTOR_RXV_SN, rxd3) != 0)
		u4AmpduRefNum += 1;

	BandIdx = GET_FIELD(WF_RX_DESCRIPTOR_BN, rxd1);
	radiotap_info.u2VendorLen =
			rxd_len + (((rxd_grp0->rxd_2 & BMAC_RXD_HO_MASK) >> BMAC_RXD_HO_SHIFT) * 2);
	radiotap_info.ucSubNamespace = 3; /* MTK wireshark CONNAC3 id */
	radiotap_info.u4AmpduRefNum = u4AmpduRefNum;
	radiotap_info.u4Timestamp = GET_FIELD(WF_RX_DESCRIPTOR_TIMESTAMP, rxd16);
	radiotap_info.ucFcsErr = GET_FIELD(WF_RX_DESCRIPTOR_FC, rxd3);
	radiotap_info.ucFrag = GET_FIELD(WF_RX_DESCRIPTOR_FRAG, rxd2);
	radiotap_info.ucRfBand = GET_FIELD(WF_RX_DESCRIPTOR_BN, rxd1);
	radiotap_info.u2ChFrequency = GET_FIELD(WF_RX_DESCRIPTOR_CH_FREQUENCY, rxd3);
	radiotap_info.ucRxMode = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_RX_MODE, rxd24);
	radiotap_info.ucFrMode = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_FR_MODE, rxd24);
	radiotap_info.ucShortGI = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_SHORT_GI, rxd24);
	radiotap_info.ucSTBC = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_STBC, rxd24);
	radiotap_info.ucNess = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_NESS, rxd24);
	radiotap_info.ucLDPC = GET_FIELD(WF_RX_DESCRIPTOR_P_RXV_LDPC, rxd20);
	radiotap_info.ucMcs = GET_FIELD(WF_RX_DESCRIPTOR_P_RXV_RX_RATE, rxd20);
	radiotap_info.ucRcpi0 = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_RCPI0, rxd30);
	radiotap_info.ucRcpi1 = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_RCPI1, rxd30);
	radiotap_info.ucTxopPsNotAllow =
			GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_TXOP_PS_NOT_ALLOWED, rxd24);
	radiotap_info.ucLdpcExtraOfdmSym =
			GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_LDPC_EXTRA_OFDM_SYM, rxd24);
	radiotap_info.ucVhtGroupId =
		GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_GROUP_ID, rxd26);
	radiotap_info.ucNsts =
		GET_FIELD(WF_RX_DESCRIPTOR_P_RXV_NSTS, rxd20) + 1;
	radiotap_info.ucBeamFormed =
		GET_FIELD(WF_RX_DESCRIPTOR_P_RXV_BEAMFORMED, rxd20);
	if (radiotap_info.ucRxMode >= MODE_HE_SU) {
		radiotap_info.ucPeDisamb = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_PE_DIS_AMB, rxd25);
		radiotap_info.ucNumUser = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_NUM_USER, rxd24);
		radiotap_info.ucSigBRU0 = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_SIGB_RU0, rxd36);
		radiotap_info.ucSigBRU1 = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_SIGB_RU1, rxd36);
		radiotap_info.ucSigBRU2 = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_SIGB_RU2, rxd36);
		radiotap_info.ucSigBRU3 = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_SIGB_RU3, rxd36) |
			GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_SIGB_RU3_1, rxd37);
		radiotap_info.u2VhtPartialAid =
			GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_PART_AID, rxd28);
		radiotap_info.u2RuAllocation = GET_FIELD(WF_RX_DESCRIPTOR_P_RXV_RU_ALLOC, rxd20);
		radiotap_info.u2BssClr = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_BSS_COLOR, rxd29);
		radiotap_info.u2BeamChange = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_BEAM_CHANGE, rxd28);
		radiotap_info.u2UlDl = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_UL_DL, rxd25);
		radiotap_info.u2DataDcm = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_DCM, rxd28);
		radiotap_info.u2SpatialReuse1 =
			GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE1, rxd33);
		radiotap_info.u2SpatialReuse2 =
			GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE2, rxd33);
		radiotap_info.u2SpatialReuse3 =
			GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE3, rxd33);
		radiotap_info.u2SpatialReuse4 =
			GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE4, rxd33);
		radiotap_info.u2Ltf =
			GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_LTF, rxd24) + 1;
		radiotap_info.u2Doppler = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_DOPPLER, rxd29);
		radiotap_info.u2Txop = GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_TXOP, rxd29);
		radiotap_info.ucPpduTypeComp =
					GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_PPDU_TYPE_COMP, rxd25);
		radiotap_info.ucEhtSigMcs =
					GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_EHT_SIG_MCS, rxd41);
		radiotap_info.ucEhtLtfSymNum =
					GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_EHT_LTF_SYM_NUM, rxd41);
		radiotap_info.ucEhtSigSymNum =
					GET_FIELD(WF_RX_DESCRIPTOR_C_RXV_EHT_SIG_SYM_NUM, rxd41);

	}

	rx_blk->radiotap_info = radiotap_info;


	return 0;
}

static int
bmac_mac_rxd_parser(struct mtk_hw_dev *dev, struct sk_buff *skb,
				struct _RX_BLK *rx_blk)
{
	__le32 *rxd = (__le32 *)skb->data;
	u32 rxd0 = le32_to_cpu(rxd[0]);
	u32 rxd1 = le32_to_cpu(rxd[1]);
	u32 rxd2 = le32_to_cpu(rxd[2]);
	u32 rxd3 = le32_to_cpu(rxd[3]);
	u32 rxd4 = le32_to_cpu(rxd[4]);
	struct mtk_bus_rx_info *rx_info = (struct mtk_bus_rx_info *)skb->cb;
#ifdef CONFIG_HWIFI_DBG
	u16 sn;

	sn = GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, le32_to_cpu(rxd[1]))
			& BMAC_GROUP_VLD_4 ?
			GET_FIELD(WF_RX_DESCRIPTOR_SEQUENCE_NUMBER,
			le32_to_cpu(rxd[10])) : 65535;

	if (rx_info->check_sn &&
		GET_FIELD(WF_RX_DESCRIPTOR_H, le32_to_cpu(rxd[2])) &&
		sn != rx_info->sn) {
		dev_err(dev->dev, "%s(): i = %u, j = %u, mac_rxd sn = %u != ind_cmd sn = %u\n",
			__func__, rx_info->i, rx_info->j, sn, rx_info->sn);
	}
#endif

	rx_blk->pRxInfo =
		(struct _RXINFO_STRUC *)(&rx_blk->hw_rx_info[12]);
	rx_blk->rmac_info = (u8 *)rxd;

	if (rx_info->ip_frag && rx_info->old_pkt)
		RX_BLK_SET_ERR_FLAG(rx_blk, fRX_ERR_IP_FRAG_OLD_PKT);

	if (rx_info->old_pkt)
		RX_BLK_SET_ERR_FLAG(rx_blk, fRX_ERR_OLD_PKT);

	if (rx_info->pn_chk_fail)
		RX_BLK_SET_ERR_FLAG(rx_blk, fRX_ERR_PN_CHK_FAIL);

	/* Group0 DW0 */
	if (rx_info->hw_rro)
		rx_blk->MPDUtotalByteCnt = rx_info->len;
	else
		rx_blk->MPDUtotalByteCnt =
				GET_FIELD(WF_RX_DESCRIPTOR_RX_BYTE_COUNT, rxd0);

	/* Group0 DW1 */
	rx_blk->wcid = GET_FIELD(WF_RX_DESCRIPTOR_MLD_ID, rxd1);
	rx_blk->key_idx = GET_FIELD(WF_RX_DESCRIPTOR_KID, rxd1);

	if (GET_FIELD(WF_RX_DESCRIPTOR_CM, rxd1))
		RX_BLK_SET_FLAG(rx_blk, fRX_CM);

	if (GET_FIELD(WF_RX_DESCRIPTOR_CLM, rxd1))
		RX_BLK_SET_FLAG(rx_blk, fRX_CLM);

	if (GET_FIELD(WF_RX_DESCRIPTOR_I, rxd1))
		RX_BLK_SET_FLAG(rx_blk, fRX_ICV_ERR);

	if (GET_FIELD(WF_RX_DESCRIPTOR_T, rxd1))
		RX_BLK_SET_FLAG(rx_blk, fRX_TKIP_MIC_ERR);

	rx_blk->band = GET_FIELD(WF_RX_DESCRIPTOR_BN, rxd1);

	/* Group0 DW2 */
	rx_blk->bss_idx = GET_FIELD(WF_RX_DESCRIPTOR_BSSID, rxd2);

	if (GET_FIELD(WF_RX_DESCRIPTOR_H, rxd2))
		RX_BLK_SET_FLAG(rx_blk, fRX_HDR_TRANS);

	rx_blk->MPDUtotalByteCnt -=
		(GET_FIELD(WF_RX_DESCRIPTOR_HO, rxd2) << 1);

	rx_blk->sec_mode = GET_FIELD(WF_RX_DESCRIPTOR_SEC_MODE, rxd2);

	if (GET_FIELD(WF_RX_DESCRIPTOR_DAF, rxd2))
		rx_blk->DeAmsduFail = 1;

	if (GET_FIELD(WF_RX_DESCRIPTOR_FRAG, rxd2)) {
		rx_blk->pRxInfo->FRAG = 1;
		rx_info->wifi_frag = 1;
	}

	if (GET_FIELD(WF_RX_DESCRIPTOR_NUL, rxd2))
		rx_blk->pRxInfo->NULLDATA = 1;

	if (!GET_FIELD(WF_RX_DESCRIPTOR_NDATA, rxd2))
		rx_blk->pRxInfo->DATA = 1;

	if (!GET_FIELD(WF_RX_DESCRIPTOR_NAMP, rxd2))
		rx_blk->pRxInfo->AMPDU = 1;

	/* Group0 DW3 */
	rx_blk->channel_freq =
			GET_FIELD(WF_RX_DESCRIPTOR_CH_FREQUENCY, rxd3);

	switch (GET_FIELD(WF_RX_DESCRIPTOR_A1_TYPE, rxd3)) {
	case 0x1:
		rx_blk->pRxInfo->U2M = 1;
		break;
	case 0x2:
		rx_blk->pRxInfo->Mcast = 1;
		break;
	case 0x3:
		rx_blk->pRxInfo->Bcast = 1;
		break;
	}

	if (GET_FIELD(WF_RX_DESCRIPTOR_HTC, rxd3)) {
		RX_BLK_SET_FLAG(rx_blk, fRX_HTC);
		rx_blk->pRxInfo->HTC = 1;
	}

	if (GET_FIELD(WF_RX_DESCRIPTOR_FC, rxd3))
		rx_blk->pRxInfo->Crc = 1;

	/* Group0 DW4 */
	rx_blk->AmsduState = GET_FIELD(WF_RX_DESCRIPTOR_PF, rxd4);
	rx_blk->TID =
			GET_FIELD(WF_RX_DESCRIPTOR_TID, rxd4);

	rxd += RMAC_INFO_BASE_SIZE;
	rx_blk->MPDUtotalByteCnt -= (RMAC_INFO_BASE_SIZE << 2);

	/* Group4 */
	if (GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, rxd1) &
					BMAC_GROUP_VLD_4) {
		rx_blk->FC = (u8 *)rxd;

		if (RX_BLK_TEST_FLAG(rx_blk, fRX_HDR_TRANS)) {
			u16 fn_sn = le16_to_cpu(*((u16 *)(rx_blk->FC + 8)));

			rx_blk->FN = fn_sn & 0x000f;
			rx_blk->SN = (fn_sn & 0xfff0) >> 4;

			rx_blk->UserPriority =
				le16_to_cpu(*((u16 *)(rx_blk->FC + 10))) & 0xf;
		}

		rxd += RMAC_INFO_GRP_4_SIZE;
		rx_blk->MPDUtotalByteCnt -= (RMAC_INFO_GRP_4_SIZE << 2);
	}

	/* Group1 */
	if (GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, rxd1) &
					BMAC_GROUP_VLD_1) {
		rx_blk->CCMP_PN = (rxd[0] + ((u64)rxd[1] << 32));

		rxd += RMAC_INFO_GRP_1_SIZE;
		rx_blk->MPDUtotalByteCnt -= (RMAC_INFO_GRP_1_SIZE << 2);
	}

	/* Group2 */
	if (GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, rxd1) &
					BMAC_GROUP_VLD_2) {
		rxd += RMAC_INFO_GRP_2_SIZE;
		rx_blk->MPDUtotalByteCnt -= (RMAC_INFO_GRP_2_SIZE << 2);
	}

	/* Group3 */
	if (GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, rxd1) &
					BMAC_GROUP_VLD_3) {
		u8 i = 0;

		rx_blk->rcpi[0] = (rxd[3] & GENMASK(7, 0)) >> 0;
		rx_blk->rcpi[1] = (rxd[3] & GENMASK(15, 8)) >> 8;
		rx_blk->rcpi[2] = (rxd[3] & GENMASK(23, 16)) >> 16;
		rx_blk->rcpi[3] = (rxd[3] & GENMASK(31, 24)) >> 24;

		for (i = 0; i < MAX_ANTENNA_NUM; i++)
			rx_blk->rx_signal.raw_rssi[i] = RCPI_TO_RSSI(rx_blk->rcpi[i]);

		rx_blk->rx_rate.field.MCS = ((rxd[0] & GENMASK(6, 0)) >> 0);

		rxd += RMAC_INFO_GRP_3_SIZE;
		rx_blk->MPDUtotalByteCnt -= (RMAC_INFO_GRP_3_SIZE << 2);
	}

	/* Group5 */
	if (GET_FIELD(WF_RX_DESCRIPTOR_GROUP_VLD, rxd1) &
					BMAC_GROUP_VLD_5) {

		rx_blk->rx_rate.field.MODE = ((rxd[0] & GENMASK(7, 4)) >> 4);

		rxd += RMAC_INFO_GRP_5_SIZE;
		rx_blk->MPDUtotalByteCnt -= (RMAC_INFO_GRP_5_SIZE << 2);
		rx_blk->sniffermode = 1;
	} else
		rx_blk->sniffermode = 0;

	if (rx_info->hw_rro) {
		rx_blk->pData =
		(u8 *)(skb->data) + (rx_info->eth_hdr_ofst << 1);
		rx_blk->DataSize = rx_info->len - (rx_info->eth_hdr_ofst << 1);
	} else {
		rx_blk->pData =
		(u8 *)(rxd) + (GET_FIELD(WF_RX_DESCRIPTOR_HO, rxd2) << 1);
		rx_blk->DataSize = rx_blk->MPDUtotalByteCnt;
	}
	rx_blk->pRxPacket = skb;

	return 0;
}

void
bmac_rx_data(struct mtk_bus_trans *trans, struct sk_buff *skb)
{
	int ret = 0;
	struct _RX_BLK rx_blk = {0};
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct mtk_hw_phy *mlo_hw_phy, *link_hw_phy;
	struct sk_buff *pre_skb = NULL;
	struct sk_buff *tmp;
	u32 band_idx, mld_idx, mld_link_idx;
	u8 mld_link = 0;

	ret = bmac_mac_rxd_parser(dev, skb, &rx_blk);
	if (ret)
		goto err;

	ret = bmac_drop_decision(dev, skb);
	if (ret)
		goto err;

	ret = bmac_mac_rxd_radiotap_parser(dev, skb, &rx_blk);
	if (ret)
		goto err;

	skb->data = rx_blk.pData;
	skb->len = rx_blk.DataSize;
	band_idx = rx_blk.band;
	rx_blk.link_band = rx_blk.band;

	link_hw_phy = mtk_hwctrl_phy_entry_find(dev, band_idx);

	if (!link_hw_phy)
		goto err;

	mld_idx = rx_blk.wcid;
	rx_blk.link_wcid = rx_blk.wcid;
	mld_link_idx = rx_blk.wcid;

	mtk_dbg(MTK_RX, "link wcid = %d, band_idx = %d\n",
					rx_blk.link_wcid, band_idx);

	/*redirect mld rx data to master interface including hw_phy and mld_idx*/
	mlo_hw_phy = mtk_hdev_rx_get_phy(dev, &band_idx, &mld_idx, &mld_link_idx, &mld_link);
	rx_blk.wcid = mld_idx;
	rx_blk.band = band_idx;
	rx_blk.link_wcid = mld_link_idx;

	if (!mlo_hw_phy)
		goto err;

	mtk_dbg(MTK_RX, "mld_link = %d, mld_idx = %d, band_idx = %d\
				sn = %d, tid = %d, amsdu_state = %d\n",
				mld_link, mld_idx, band_idx, rx_blk.SN, rx_blk.TID, rx_blk.AmsduState);

	if (!mld_link) {
		mtk_mac_rx(link_hw_phy->mac_hw, NULL, skb, &rx_blk);
	} else if (test_bit(HWIFI_FLAG_IN_CHIP_RRO, &dev->flags) ||
		   test_bit(HWIFI_FLAG_BA_OFFLOAD, &dev->flags)) {
		mtk_mac_rx(mlo_hw_phy->mac_hw, NULL, skb, &rx_blk);
	} else {
		struct sk_buff_head *msdu_q = &link_hw_phy->msdu_q;
		struct _RX_BLK *msdu_blk = (struct _RX_BLK *)link_hw_phy->msdu_rx_blk;
		struct _RX_BLK *cur_blk = NULL;
		int i = 0;
		switch (rx_blk.AmsduState) {
		case FIRST_AMSDU_FORMAT:
	   	case MIDDLE_AMSDU_FORMAT:
			cur_blk = msdu_blk + skb_queue_len(msdu_q);
			*cur_blk = rx_blk;
			cur_blk->pRxInfo =
				(struct _RXINFO_STRUC *)(&cur_blk->hw_rx_info[12]);
			__skb_queue_tail(msdu_q, skb);
			break;
		case FINAL_AMSDU_FORMAT:
			spin_lock(&mlo_hw_phy->mlo_rx_lock);
			skb_queue_walk_safe(msdu_q, pre_skb, tmp) {
				__skb_unlink(pre_skb, msdu_q);
				mtk_mac_rx(mlo_hw_phy->mac_hw, NULL, pre_skb,
						msdu_blk + i);
				i++;
			}
			mtk_mac_rx(mlo_hw_phy->mac_hw, NULL, skb, &rx_blk);
			spin_unlock(&mlo_hw_phy->mlo_rx_lock);
			break;
		case MSDU_FORMAT:
			spin_lock(&mlo_hw_phy->mlo_rx_lock);
			mtk_mac_rx(mlo_hw_phy->mac_hw, NULL, skb, &rx_blk);
			spin_unlock(&mlo_hw_phy->mlo_rx_lock);
			break;
		default:
			goto err;
		}
	}
	return;
err:
	dbg_bus_rx_drop_inc(trans, (struct mtk_bus_rx_info *) skb->cb);
	dev_kfree_skb(skb);
}

void
bmac_rx_ics(struct mtk_bus_trans *trans, struct sk_buff *skb)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct mtk_hw_phy *hw_phy;

	hw_phy = mtk_hwctrl_phy_entry_find(dev, 0);

	if (!hw_phy)
		goto err;

	mtk_mac_rx_ics(hw_phy->mac_hw, skb);

	return;
err:
	dbg_bus_rx_err_inc(trans);
	dev_kfree_skb(skb);
}

void
bmac_rx_txs(struct mtk_bus_trans *trans, struct sk_buff *skb)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct bmac_txs *txs = (struct bmac_txs *)skb->data;
	struct txs_info_t txs_info = {0};
	u32 txs_dw0, txs_dw2, txs_dw3, tr_delay, rx_bytes = 0;
	u16 txs_cnt, dw_idx, pid, i;
	bool l_err, r_err, m_err;

	txs_cnt = GET_FIELD(WF_DS_AGG_TXS_DW00_TXS_CNT, le32_to_cpu(txs->dw0));
	rx_bytes = GET_FIELD(WF_DS_AGG_TXS_DW00_RX_BYTE_COUNT, le32_to_cpu(txs->dw0));

	mtk_dbg_dump(MTK_RX, "TXS ", (void *)txs, rx_bytes);

	rx_bytes -= (4 * sizeof(u32));

	for (dw_idx = 0, i = 0; i < txs_cnt && rx_bytes; i++) {
		txs_dw0 = le32_to_cpu(txs->info[dw_idx]);
		txs_dw2 = le32_to_cpu(txs->info[dw_idx + 2]);
		txs_dw3 = le32_to_cpu(txs->info[dw_idx + 3]);

		l_err = GET_FIELD(WF_DS_TXS_MPDU_DW00_LE, txs_dw0);
		r_err = GET_FIELD(WF_DS_TXS_MPDU_DW00_RE, txs_dw0);
		m_err = GET_FIELD(WF_DS_TXS_MPDU_DW00_ME, txs_dw0);
		tr_delay = GET_FIELD(WF_DS_TXS_MPDU_DW02_TRANSMISSION_DELAY, txs_dw2);
		pid = GET_FIELD(WF_DS_TXS_MPDU_DW03_PID, txs_dw3);

		txs_info.txs_sts = (l_err || r_err || m_err) ?
			TXS_STS_NG : TXS_STS_OK;
		txs_info.pid = pid;
		txs_info.tr_delay = tr_delay * BMAC_TXS_TR_DELAY_UNIT;

		mtk_mac_txs_handler(dev, txs_info);
		rx_bytes -= (8 * sizeof(u32));
		dw_idx += 8;
	}

	if (i < txs_cnt) {
		dev_err(dev->dev, "%s(): txs might leak! due to only %d txs report, but should be %d\n",
				__func__, i, txs_cnt);
		dbg_bus_txs_err_inc(trans);
	}

	dbg_bus_txs_inc(trans);
	dev_kfree_skb(skb);
}

void
bmac_rx_sdo_event(struct mtk_bus_trans *trans, struct sk_buff *skb)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	__le32 *sdo_event = (__le32 *)skb->data;
	u32 dw, rx_bytes;
	u16 wcid, dw_idx, i, event_cnt, dw_len, dw_num, event_id;
	u8 tid;

	/* Common Event */
	dw = le32_to_cpu(sdo_event[0]);
	event_cnt = GET_FIELD(WF_SDO_EVENT_EVENT_COUNT, dw);
	rx_bytes = GET_FIELD(WF_SDO_EVENT_RX_BYTE_COUNT, dw);

	mtk_dbg_dump(MTK_SDO, "SDO EVENT ", (void *)sdo_event, rx_bytes);

	rx_bytes -= SDO_EVENT_COMMON_DW_NUM * sizeof(u32);

	/* Event-Specific */
	for (dw_idx = SDO_EVENT_COMMON_DW_NUM, i = 0; i < event_cnt && rx_bytes; i++) {
		dw = le32_to_cpu(sdo_event[dw_idx]);

		dw_len = GET_FIELD(WF_SDO_EVENT_DW_LEN, dw);
		event_id = GET_FIELD(WF_SDO_EVENT_EVENT_ID, dw);

		if (event_id == SDO_BA_TRIG_EVENT_ID) { /* BA Trigger Event */
			tid = GET_FIELD(WF_SDO_EVENT_TID, dw);
			wcid = GET_FIELD(WF_SDO_EVENT_MLD_ID, dw);
			mtk_interface_ba_trig_event(dev, wcid, tid);
		}

		dw_num = dw_len + 1;
		rx_bytes -= dw_num * sizeof(u32);
		dw_idx += dw_num;
	}

	if (i < event_cnt) {
		dev_err(dev->dev, "%s(): only %d sdo event report, but should be %d\n",
			__func__, i, event_cnt);
		dbg_bus_sdo_event_err_inc(trans);
	}

	dbg_bus_sdo_event_inc(trans);
	dev_kfree_skb(skb);
}
