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

#ifndef __MTK_TK_MGMT_H_
#define __MTK_TK_MGMT_H_

#include "wsys.h"

struct mtk_hw_dev;
struct mtk_rx_tk_mgmt;

enum mtk_tk_mgmt_type {
	TK_MGMT_CMM,
	TK_MGMT_BMC,
	TK_MGMT_MAX,
};

/** struct mtk_tk_entry - TX token related information
 *
 * This struct is used to maintin TXD/TXP relationship for
 * cut through/PAO architecture.
 *
 * @param list: Linking list for free pool
 * @param skb: Packet point mapping to TXD
 * @param dma_addr: Physical DMA address for txd
 * @param pkt_pa: Physical address for packet buffer point
 * @param txd_ptr: Virtual address for TXD
 * @param txd_size: Size of TXD
 * @param sta: Point to a hw_sta related to this txd
 * @param sid: Unique mtk_idr_entry for this token entry
 * @param hw_dev: Point to a hw_dev related to this token entry
 * @param band_idx: Index of band related to this token entry
 * @param rdy: this token entry is valid or not
 */
struct mtk_tk_entry {
	struct list_head list;
	struct sk_buff_head tx_q;
	struct mtk_idr_entry sid;
	dma_addr_t dma_addr;
	dma_addr_t pkt_pa;
	int (*write_txp)(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
		void *tx_pkt_info);
	void (*skb_unmap_txp)(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry);
	u8 *txd_ptr;
	struct mtk_tk_mgmt *tk_mgmt;
	struct mtk_hw_sta *sta;
	struct mtk_hw_dev *hw_dev;
	u16 fbuf_size;
	u16 fbuf_dma_size;
	u16 wcid;
	u8 bss_idx;
	u8 band_idx	:4;
	u8 tid		:4;
	u8 hf		:2;
	u8 rdy		:1;
	u8 is_fixed_rate:1;
	u8 is_prior	:1;
	u8 is_sp	:1;
	u8 amsdu_en	:1;
	u8 is_rsv	:1;
};

struct mtk_tk_dbg {
	u32 request_cnt;
	u32 release_cnt;
	u32 free_tkid_cnt;
	u32 request_err_cnt;
	u32 release_err_cnt;
	u32 free_tkid_err_cnt;
	u32 free_tkid_oor_cnt;
	u32 tk_entry_pkt_null;
};

struct mtk_rx_tk_dbg {
	u32 request_cnt;
	u32 release_cnt;
	u32 request_err_cnt;
};

struct mtk_tk_res {
	u16 max_nums;
	atomic_t used;
	spinlock_t lock;
	struct list_head cache;
};

struct mtk_tk_res_ctl {
	u16 max_nums;
	atomic_t used;
	struct mtk_tk_res rsv_res;
};

/** struct mtk_tk_mgmt - TX token manager
 *
 * This sturct is used for managing token entry
 *
 * @param base: mtk_idr manager for token index
 * @param tx_cache: List head for free token entry pool
 * @param fbuf_size: Size of TXD (+ 802.11 header + llc if needed)
 * @param max_tx_tk_nums: Maximum tx token number
 * @param pdev: Linux device for DMA coherence operation.
 * @param tk_dbg: Debugfs for token manager
 */
struct mtk_tk_mgmt {
	struct mtk_idr_mgmt base;
	struct list_head tx_cache;
	u16 fbuf_size;
	u16 max_tx_tk_nums;
	struct device *pdev;
	struct mtk_tk_dbg tk_dbg;
};

/** struct mtk_rx_tk_entry - RX token entry
 *
 * This struct is used to indicate a RXD and related information
 *
 * @param pkt:
 * @param pkt_size:
 * @param pkt_pa:
 * @param pkt_va:
 * @param sid:
 * @param prev: pointer to prev tk_entry
 * @param next: pointer to next tk_entry
 */
struct mtk_rx_tk_entry {
	struct list_head list;
	void *pkt;
	u32 pkt_size;
	dma_addr_t pkt_pa;
	dma_addr_t pkt_pa_bk;
	void *pkt_va;
	struct mtk_idr_entry sid;
} ____cacheline_aligned;

/** struct mtk_rx_tk_ops - RX token manager callbacks
 *
 * These callbacks used for selected driver (mtk_wed) can callback to rx manager
 *
 * @param find: Find a rx_tk_entry by index
 * @param update: Update information to rx_tk_entry by index
 * @param request: Request a new index for a rx_tk_entry
 * @param update_state: update token state
 * @param get_state: query token state
 * @param release: Release an index for a rx_tk_entry
 * @param free: Free an rx_tk_entry by index
 * @param dequeue: remove entry from double linked list
 * @param insert_prev: insert to prev of target entry
 * @param insert_next: insert to next of target entry
 */
struct mtk_rx_tk_ops {
	int (*find)(struct mtk_rx_tk_mgmt *tk,
		u32 id, void **pkt, void **va, dma_addr_t *pa);
	int (*query)(struct mtk_rx_tk_mgmt *tk, u32 id, dma_addr_t *pa);
	int (*request)(struct mtk_rx_tk_mgmt *tk, void *pkt,
		u32 len, void *va, dma_addr_t pa);
	void *(*release)(struct mtk_rx_tk_mgmt *tk, u32 id);
};

/** struct mtk_rx_tk_mgmt - Rx token manager
 *
 * This struct is used to manage rx token
 *
 * @param base: mtk_idr_mgmt for rx token index
 * @param pdev: Linux device for DMA coherence operations
 * @param ops: Point to callbacks for rx token manager
 */
struct mtk_rx_tk_mgmt {
	struct mtk_idr_mgmt base;
	struct list_head rx_cache;
	struct device *pdev;
	struct mtk_rx_tk_ops *ops;
	struct mtk_rx_tk_dbg rx_tk_dbg;
};

static inline void
dbg_tk_request_inc(struct mtk_tk_mgmt *tk)
{
	tk->tk_dbg.request_cnt++;
}

static inline void
dbg_tk_release_inc(struct mtk_tk_mgmt *tk)
{
	tk->tk_dbg.release_cnt++;
}

static inline void
dbg_tk_free_tkid_inc(struct mtk_tk_mgmt *tk)
{
	tk->tk_dbg.free_tkid_cnt++;
}

static inline void
dbg_tk_request_err_inc(struct mtk_tk_mgmt *tk)
{
	tk->tk_dbg.request_err_cnt++;
}

static inline void
dbg_tk_release_err_inc(struct mtk_tk_mgmt *tk)
{
	tk->tk_dbg.release_err_cnt++;
}

static inline void
dbg_tk_free_tkid_err_inc(struct mtk_tk_mgmt *tk)
{
	tk->tk_dbg.free_tkid_err_cnt++;
}

static inline void
dbg_tk_free_tkid_oor_inc(struct mtk_tk_mgmt *tk)
{
	tk->tk_dbg.free_tkid_oor_cnt++;
}

static inline void
dbg_tk_entry_pkt_null_err_inc(struct mtk_tk_mgmt *tk)
{
	tk->tk_dbg.tk_entry_pkt_null++;
}

static inline void
dbg_rx_tk_request_inc(struct mtk_rx_tk_mgmt *tk)
{
	tk->rx_tk_dbg.request_cnt++;
}

static inline void
dbg_rx_tk_release_inc(struct mtk_rx_tk_mgmt *tk)
{
	tk->rx_tk_dbg.release_cnt++;
}

static inline void
dbg_rx_tk_request_err_inc(struct mtk_rx_tk_mgmt *tk)
{
	tk->rx_tk_dbg.request_err_cnt++;
}

static inline int
mtk_rx_tk_request_entry(struct mtk_rx_tk_mgmt *tk,
	void *pkt, u32 len, void *va, dma_addr_t pa)
{
	if (tk->ops->request)
		return tk->ops->request(tk, pkt, len, va, pa);
	return -EOPNOTSUPP;
}

static inline void *
mtk_rx_tk_release_entry(struct mtk_rx_tk_mgmt *tk, u32 id)
{
	if (tk->ops->release)
		return tk->ops->release(tk, id);
	return NULL;
}

static inline int
mtk_rx_tk_entry_find(struct mtk_rx_tk_mgmt *tk,
		u32 id, void **pkt, void **va, dma_addr_t *pa)
{
	if (tk->ops->find)
		return tk->ops->find(tk, id, pkt, va, pa);
	return -EOPNOTSUPP;
}

static inline int
mtk_rx_tk_entry_query(struct mtk_rx_tk_mgmt *tk,
	u32 id, dma_addr_t *pa)
{
	if (tk->ops->query)
		return tk->ops->query(tk, id, pa);
	return -EOPNOTSUPP;
}

int
mtk_tk_init(struct mtk_hw_dev *dev);

void
mtk_tk_exit(struct mtk_hw_dev *dev);

int
mtk_tk_mgmt_init(struct mtk_hw_dev *dev);

void
mtk_tk_mgmt_exit(struct mtk_hw_dev *dev);

int
mtk_tk_res_ctl_alloc(struct mtk_hw_phy *phy, struct mtk_hwres_radio_cap *radio_cap);

int
mtk_tk_res_ctl_free(struct mtk_hw_phy *phy);

int
mtk_tk_res_ctl_req(struct mtk_hw_phy *phy, u16 max_nums, u16 rsv_nums);

int
mtk_tk_check_resource(struct mtk_hw_phy *phy);

int
mtk_tk_bmc_check_resource(struct mtk_hw_phy *phy);

int
mtk_tk_rsv_check_resource(struct mtk_hw_phy *phy);

struct mtk_tk_entry *
mtk_tk_request_entry(struct mtk_hw_dev *dev);

struct mtk_tk_entry *
mtk_tk_bmc_request_entry(struct mtk_hw_dev *dev);

struct mtk_tk_entry *
mtk_tk_rsv_request_entry(struct mtk_hw_phy *phy);


struct mtk_tk_entry *
mtk_tk_get_entry(struct mtk_hw_dev *dev, u32 id);

int
mtk_tk_unmap_tx_q(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry);

struct sk_buff_head *
mtk_tk_get_tx_q(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry);

int
mtk_tk_release_entry(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry);

#define mtk_tk_entry_get_id(_tk_entry) \
	(_tk_entry->sid.idx)

#endif
