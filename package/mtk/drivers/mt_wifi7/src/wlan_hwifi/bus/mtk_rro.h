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

#ifndef __MTK_RRO_H
#define __MTK_RRO_H

#include <hw_ctrl/rro_cmm.h>
#include <bus/mtk_pci_dma.h>

#define MAX_HW_RRO_SESSION_CNT 1024
#define HW_RRO_PARTICULAR_SESSION_CNT 1
#define MAX_HW_RRO_WIN_SIZE 1024
#define HW_RRO_ADDR_ELEM_CR_CNT 128
#define HW_RRO_BA_BITMAP_CR_CNT 2
#define HW_RRO_SESSION_CNT_PER_CR (MAX_HW_RRO_SESSION_CNT / HW_RRO_ADDR_ELEM_CR_CNT)
#define BA_BITMAP_CACHE_SIZE_PER_SESSION 128
#define BA_BITMAP_CACHE_SIZE_PER_CR ((MAX_HW_RRO_SESSION_CNT * BA_BITMAP_CACHE_SIZE_PER_SESSION) / HW_RRO_BA_BITMAP_CR_CNT)
#define RRO_MAX_SEQ 0xfff

/* RRO 3.1 */
#define HW_RRO_MSDU_PG_CR_CNT 8
#define HW_RRO_MSDU_PG_SIZE_PER_CR 0x10000


#define TOKEN_STATE_NONE 0
#define TOKEN_STATE_IN_RRO 1
#define TOKEN_STATE_IN_FREE_POOL 2

enum {
	HW_RRO_OFF = 0,
	HW_RRO_V3_0_PRE_LINK = 1,
	HW_RRO_V3_0_BUF_PG = 2,
	HW_RRO_V3_0_BUF_PG_DBG = 3,
	HW_RRO_V3_1_BUF_PG_EMUL = 4,
	HW_RRO_V3_1 = 5,
};

enum IND_REASON {
	IND_REASON_STEP_ONE = 0,
	IND_REASON_REPEAT = 1,
	IND_REASON_OLDPKT = 2,
	IND_REASON_WITHIN = 3,
	IND_REASON_SURPASS = 4,
	IND_REASON_SURPASS_BY_BAR = 5,
	IND_REASON_SURPASS_BIG_SN_GAP = 6,
	IND_REASON_DISCONNECT = 7,
	IND_REASON_CANNOT_DO_REORDER = 8,
	IND_REASON_TIMEOUT_STEP_ONE = 9,
	IND_REASON_TIMEOUT_FLUSH_ALL = 10,
	IND_REASON_BUF_RUN_OUT = 11,
	IND_REASON_MSDU_PG_RUN_OUT = 12,
	IND_REASON_PN_CHK_FAIL = 13,
	IND_REASON_MAX
};

struct addr_elem {
	u32 head_pkt_addr_info_l;
	u32 head_pkt_addr_info_h   : 4;
	u32 seg_cnt                :11;
	u32 out_of_range           : 1;
	u32 rsv                    : 8;
	u32 signature              : 8;
};

struct ind_cmd {
	u32 se_id                  :12;
	u32 rsv                    : 4;
	u32 start_sn               :12;
	u32 ind_reason             : 4;
	u32 ind_cnt                :13;
	u32 win_sz                 : 3;
	u32 rsv2                   :13;
	u32 magic_cnt              : 3;
};

struct hif_rxd {
	u32 rx_blk_base_l;
	u32 rx_blk_base_h          : 4;
	u32 eth_hdr_ofst           : 7;
	u32 rsv                    : 1;
	u32 ring_no                : 2;
	u32 dst_sel                : 2;
	u32 sdl                    :14;
	u32 ls                     : 1;
	u32 rsv2                   : 1;
	u32 pn_31_0;
	u32 pn_47_32               :16;
	u32 cs_status              : 4;
	u32 cs_type                : 4;
	u32 c                      : 1;
	u32 f                      : 1;
	u32 un                     : 1;
	u32 rsv3                   : 1;
	u32 is_fc_data             : 1;
	u32 uc                     : 1;
	u32 mc                     : 1;
	u32 bc                     : 1;
	u16 rx_token_id;
	u16 rsv4;
	u32 rsv5;
};

struct msdu_info_page {
	struct hif_rxd rxd[5];
	u32 next_pg_l;
	u32 next_pg_h              : 4;
	u32 rsv                    :27;
	u32 owner		   : 1;
};

struct ba_session_tbl {
	u32 ack_sn         :12;
	u32 win_sz         :3;
	u32 bn             :1;
	u32 last_in_sn     :12;
	u32 bc             :1;
	u32 bd             :1;
	u32 sat            :1;
	u32 cn             :1;
	u32 within_cnt     :12;
	u32 to_sel         :3;
	u32 rsv            :1;
	u32 last_in_rxtime :12;
};

struct rxdmad_c {
	u32 sdp0_31_0;
	u32 header_ofst     :7;
	u32 ver             :1;
	u32 to_host         :1;
	u32 ring_info       :2;
	u32 dst_sel         :2;
	u32 pn_chk_fail     :1;
	u32 rsv             :2;
	u32 sdl0            :14;
	u32 ls              :1;
	u32 rsv2            :1;
	u32 sdp0_35_32      :4;
	u32 rsv3            :2;
	u32 sca_gat         :1;
	u32 par_se          :1;
	u32 rss_hash        :4;
	u32 ind_reason      :4;
	u32 rx_token_id     :16;
	u32 cs_status       :4;
	u32 cs_type         :4;
	u32 c               :1;
	u32 f               :1;
	u32 un              :1;
	u32 is_fc_data      :1;
	u32 uc              :1;
	u32 mc              :1;
	u32 bc              :1;
	u32 rsv4            :1;
	u32 wcid            :12;
	u32 magic_cnt       :4;
};

#define MAX_RXDNAD_C_MAGIC_CNT 16
#define MAX_IND_CMD_MAGIC_CNT 8
#define MAX_RXDMAD_MAGIC_CNT 16
#define MSDU_PAGE_HASH_SIZE 127
#define MAX_HIF_RXD_CND_IN_PG 5
#define MAX_RRO_RRS_RING 4
#define MAX_MSDU_CNT_ARR_SIZE 32

struct rro_cidx_didx_emi {
	struct {
		u16 idx;
		u16 rsv;
	} ring[MAX_RRO_RRS_RING];
};

struct hw_rro_dbg {
	int rro_token_used;
	int max_rro_token_used;
	u32 seg_cnt;
	u32 drop_cnt;
	u32 err_cnt;
	u32 refill_cnt;
	u32 fp_full_cnt;
	u32 fp_tkid_start;
	u32 fp_tkid_end;
	u32 addition_read_cnt;
	u32 max_read_cnt;
	u16 last_se_id;
	u16 last_start_sn;
	u16 last_ind_cnt;
	u16 last_sn;
	u16 last_tkid;
	u16 last_ack_sn;
	u16 last_ack_se_id;
	u8 last_ind_reason;
	u8 last_seg_cnt;
	/* rro debug related */
	u32 stop_rx;
	u32 ind_cmd_cnt;
	u32 ind_cnt;
	u32 larger_winsize_cnt;
	u32 max_msdu_num;
	u32 max_pg_num;
	u32 total_msdu_cnt;
	u32 invalid_msdu_cnt;
	u32 msdu_cnt[MAX_MSDU_CNT_ARR_SIZE];
	u32 sign_ok_cnt;
	u32 sign_fail_cnt;
};

struct hw_rro_cfg {
	u32 ind_signature;
	void *ba_bitmap_cache_va[HW_RRO_BA_BITMAP_CR_CNT];
	void *addr_elem_alloc_va[HW_RRO_ADDR_ELEM_CR_CNT];
	void *msdu_pg_alloc_va[HW_RRO_MSDU_PG_CR_CNT];
	void *particular_session_va;
	void *cidx_va;
	void *didx_va;
	void *dmy_va;
	u32 particular_se_id;
	dma_addr_t ba_bitmap_cache_pa[HW_RRO_BA_BITMAP_CR_CNT];
	dma_addr_t addr_elem_alloc_pa[HW_RRO_ADDR_ELEM_CR_CNT];
	dma_addr_t msdu_pg_alloc_pa[HW_RRO_MSDU_PG_CR_CNT];
	dma_addr_t particular_session_pa;
	dma_addr_t cidx_pa;
	dma_addr_t didx_pa;
	dma_addr_t dmy_pa;
	u16 session_cnt;
	u16 win_sz;
	u32 fp_head_sid;
	struct {
		u32 ack_sn_addr;
		u32 ack_sn_mask;
		u32 ack_sn_seid_mask;
		u32 dbg_rd_ctrl_mask;
		u32 dbg_rd_ctrl_addr;
		u32 dbg_rdat_dw0;
		u32 dbg_rdat_dw1;
		u32 dbg_rdat_dw2;
		u32 dbg_rdat_dw3;
		u32 sign_base_0_addr;
		u32 sign_base_1_addr;
		u32 sign_base_1_en_shift;
		u32 debug_mode_rxd_addr;
		u16 max_rro_se_cnt;
		u16 addr_elem_cr_cnt;
		u8 ack_sn_shift;
		u8 ack_sn_seid_shift;
		u8 ack_sn_update_cnt;
	};
	spinlock_t lock;
	struct list_head pg_hash_head[MSDU_PAGE_HASH_SIZE];
#ifdef CONFIG_HWIFI_DBG
	struct hw_rro_dbg rro_dbg;
	void *addr_elem_cp_va;
	dma_addr_t addr_elem_cp_pa;
#endif

};

struct msdu_page_addr {
	struct list_head list;
	void *pkt;
	dma_addr_t pa;
	void *va;
};

struct addr_elem_info {
	u16 seid;
	u16 start_sn;
	u16 size;
};

extern struct pci_rxq_ops rro_rxq_ops;
extern struct pci_rxq_ops ind_cmd_rxq_ops;
extern struct pci_rxq_ops rxdmad_c_rxq_ops;

int rro_get_mode(struct pci_trans *trans);
bool init_rro_addr_elem_by_seid(struct pci_trans *trans, u16 seid);
u32 rro_get_se_id(u16 wcid, u8 part_id);

int rro_msdu_page_hash_init(
	struct pci_trans *trans,
	void *pkt,
	void *va,
	dma_addr_t pa);
int rro_sw_init(struct pci_trans *trans);
int rro_sw_exit(struct pci_trans *trans);
int rro_alloc_resource(struct pci_trans *trans, struct rro_chip_profile *profile);
inline void rro_free_resource(struct pci_trans *trans);

extern struct addr_elem *get_addr_elem(
	struct pci_trans *trans,
	struct hw_rro_cfg *rro_cfg,
	u16 seid, u16 start_sn);
#endif
