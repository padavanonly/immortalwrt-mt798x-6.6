/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: wifi_offload
	wed.h
*/

#ifndef _WED_H
#define _WED_H

#include "warp_utility.h"

/*WED Token setting*/
#define WED_TOKEN_STATUS_INIT	1
#define WED_TOKEN_UNIT			128
#define WED_TOKEN_STATUS_UNIT	32
#define WED_TOKEN_RSV			0
/*buffer mgmet initial token cnt*/
/*max cr4 support token id, mt7622 can support up to 8192*/

#ifdef WED_DYNAMIC_TXBM_SUPPORT
#define WED_DYNBM_LOW_GRP			2
#define WED_DYNBM_HIGH_GRP			(2*WED_DYNBM_LOW_GRP-1)
#endif /*WED_DYNAMIC_TXBM_SUPPORT*/

#define WED_PKT_NUM_GET(_wed) (_wed->res_ctrl.tx_ctrl.res.pkt_num)
#define WED_TOKEN_NUM_GET(_wed) (_wed->res_ctrl.tx_ctrl.res.token_num)
#define WED_PKT_GRPNUM_GET(_wed) (_wed->res_ctrl.tx_ctrl.res.bm_vld_grp)
#define WED_TOKEN_GRPNUM_GET(_wed) (_wed->res_ctrl.tx_ctrl.res.tkn_vld_grp)
#define WED_TOKEN_CNT_GET(_wed) (_wed->res_ctrl.tx_ctrl.res.wed_token_cnt)
#define WED_DLY_INT_VALUE 0xC014C014
#define WED_RING_OFFSET 0x10
#define WED_MIN_RX_RING_SIZE 4
#define WED_CR_SIZE 4

#define WED_CTL_SD_LEN1		GENMASK(13, 0)
#define WED_CTL_LAST_SEC1	BIT(14)
#define WED_CTL_BURST		BIT(15)
#define WED_CTL_SD_LEN0_SHIFT 16
#define WED_CTL_SD_LEN0		GENMASK(29, 16)
#define WED_CTL_LAST_SEC0	BIT(30)
#define WED_CTL_DMA_DONE	BIT(31)
#define WED_INFO_WINFO		GENMASK(15, 0)

#define SKB_BUF_HEADROOM_RSV (NET_SKB_PAD)
#define SKB_BUF_TAILROOM_RSV (sizeof(struct skb_shared_info))

#define MAX_GROUP_SIZE 0x100

#define ceil(_a, _b) (((_a%_b) > 0) ? ((_a/_b)+1) : (_a/_b))

#define WARP_DMA_ADDR_L_SHIFT	0
#define WARP_DMA_ADDR_H_SHIFT	32

struct dybm_conf_t {
	bool enable;
	u8 recycle_postponed;
	u32 alt_quota;
	u32 buf_low;
	u32 buf_high;
	u32 vld_group;
	u32 max_group;
	u32 budget_limit;
	u32 budget_refill_watermark;
};

struct sw_conf_t {
	u32 rx_wdma_ring_depth;
	u32 tx_wdma_ring_depth;
	struct dybm_conf_t txbm;
	struct dybm_conf_t rxbm;
};

#define WARP_TXDMAD_SDP0_L_MASK		(0xffffffff)
#define WARP_TXDMAD_SDP1_L_MASK		(0xffffffff)

#define WARP_TXDMAD_SDP0_H_MASK		(0xf)
#define WARP_TXDMAD_SDP0_H_SHIFT	0
#define WARP_TXDMAD_SDP1_H_MASK		(0xf)
#define WARP_TXDMAD_SDP1_H_SHIFT	16

struct warp_txdmad {
	/* DW0 */
	__le32 sdp0;
	/* DW1 */
	__le32 ctrl;
	/* DW2  */
	__le32 sdp1;
	/* DW3 */
	__le32 info;
} __packed __aligned(4);

struct wed_bm_group_info {
	u32 skb_id_start;
	struct list_head list;
	struct list_head pkt_head;
};

/*
 * token_id: used as token id or skb id
 */
struct wed_pkt_info {
	u32 token_id;
	u32 len;
	u32 desc_len;
	u32 fd_len;
	dma_addr_t pkt_pa;
	dma_addr_t desc_pa;
	dma_addr_t fdesc_pa;
	void *pkt_va;
	void *desc_va;
	void *fdesc_va;
	struct sk_buff *pkt;
	struct list_head list;
};

struct wed_dybm_stat_t {
	u32 budget_refill;
	u32 budget_refill_failed;
	u32 budget_release;
	u32 ext_failed;
	u32 ext_times;
	u32 ext_unhanlded;
	u32 shk_times;
	u32 shk_unhanlded;
	u32 max_vld_grp;
	u32 min_vld_grp;
};

struct wed_buf_res {
	u32 token_num;
	u32 token_start;
	u32 token_end;
	u32 wed_token_cnt;
	u32 dmad_len;
	u32 fd_len;
	u32 pkt_len;
	u32 pkt_num;
	u32 bm_rsv_grp;
	u32 bm_vld_grp;
	u32 bm_max_grp;
	u32 bm_grp_sz;
	u32 tkn_rsv_grp;
	u32 tkn_vld_grp;
	u32 tkn_max_grp;
	u32 tkn_grp_sz;
	struct wed_dybm_stat_t dybm_stat;
	struct warp_dma_buf fbuf;
	struct warp_dma_buf des_buf;
	struct list_head grp_head;
	struct list_head budget_head;
	u32 budget_grp;
	u32 recycle_grp_idx;

	struct page_frag_cache tx_page;
};

struct wed_tx_bm {
	void *hif_dev;
	u32 ring_len;
	u32 pkt_num;
	u16 pkt_size;
	u16 txd_len;
	struct warp_dma_buf *desc;
	struct warp_tx_ring *ring;
	struct page_frag_cache tx_page;
	struct idr id;
};

struct wed_tx_ring_ctrl {
	u32 ring_num;
	u32 txd_len;
	struct warp_dma_buf *desc;
	struct warp_ring *ring;
};

struct wed_hif_txd_ctrl {
	u32 hif_txd_segment_nums;
	u8 hif_txd_src;
	void *hif_txd_addr[32];
	dma_addr_t hif_txd_addr_pa[32];
};

struct wed_tx_ctrl {
	struct wed_buf_res res; // v1
	struct wed_tx_bm tx_bm; // v2
	struct wed_tx_ring_ctrl ring_ctrl;
	struct wed_hif_txd_ctrl hif_txd_ctrl;
};

#define TOKEN_ID_MASK		(0xffff << 16)
#define TOKEN_ID_SHIFT		16
#define TO_HOST_SHIFT			8

struct warp_rxdmad {
	/* DW0 */
	__le32 sdp0;
	/* DW1 */
	__le32 sdl0;
	/* DW2 */
	__le32 token;
	/* DW3 */
	__le32 info;
} __packed __aligned(4);

struct wed_rx_ring_ctrl {
	u32 ring_num;
	u32 rxd_len;
	struct warp_dma_buf *desc;
	struct warp_rx_ring *ring;
};

struct wed_rx_bm_res {
	void *hif_dev;
	u32 ring_num;
	u32 ring_len;
	u32 pkt_num;
	u32 pkt_len;
	u32 rxd_len;
	u32 budget_grp;
	struct wed_dybm_stat_t dybm_stat;
	struct warp_dma_buf *desc;
	struct warp_rx_ring *ring;
	struct page_frag_cache rx_page[2];
	struct task_struct *monitor_task;
	struct timer_list extend_monitor;
	bool add_check;
};

#define WED_MSDU_PAGE_HASH_SIZE 127

struct wed_rx_page_bm {
	void *hif_dev;
	u32 ring_len;
	u32 page_num;
	u16 rx_page_size;
	u16 rxd_len;
	struct warp_dma_buf *desc;
	struct warp_rx_ring *ring;
	spinlock_t lock;
	struct list_head wed_page_hash[WED_MSDU_PAGE_HASH_SIZE];
	struct page_frag_cache rx_page;
};

struct wed_rx_tkid {
	void *hif_dev;
	u32 tkn_grp_sz;
	u32 token_num;
	u32 tkn_vld_grp;
	u32 ring_len;
	u16 rxd_len;
	struct warp_dma_buf *desc;
};

struct wed_rro_ctrl {
	u16 miod_cnt;
	u16 mid_size;
	u16 mod_size;
	u16 miod_entry_size;
	u32 miod_desc_base_mcu_view;
	struct warp_dma_buf miod_desc;
	void *miod_desc_base;
	dma_addr_t miod_desc_base_pa;
	u16 fdbk_cnt;
	struct warp_dma_buf fdbk_desc;
	void *fdbk_desc_base;
	dma_addr_t fdbk_desc_base_pa;
	u16 rro_que_cnt;
	struct warp_dma_buf rro_que;
	void *rro_que_base;
	dma_addr_t rro_que_base_pa;
};

struct wed_rx_ctrl {
	struct wed_rx_bm_res res;
	struct wed_rx_ring_ctrl ring_ctrl;
	struct wed_rx_ring_ctrl rro_data_ring_ctrl;
	struct wed_rx_ring_ctrl rro_page_ring_ctrl;
	struct wed_rx_ring_ctrl rro_ind_cmd_ring_ctrl;
	struct wed_rx_ring_ctrl rro_3_1_ring_ctrl;
	struct wed_rro_ctrl rro_ctrl;
	struct wed_rx_page_bm page_bm;
	struct wed_rx_tkid rx_tkid;
	u32 budget_head_idx;
	u32 budget_tail_idx;
	struct wed_rx_bm_res extra;
	u8 recycle_wait;
#ifdef EXTEND_POLLING
	spinlock_t recy_lock;
#endif
	u8 extend_polling_cnt;
};

struct wed_res_ctrl {
	struct wed_tx_ctrl tx_ctrl;
	struct wed_rx_ctrl rx_ctrl;
	void *proc_rxbm;

};

struct wed_ser_state {
	/*WED_TX_DMA*/
	u32 tx_dma_stat;
	u32 tx0_mib;
	u32 tx1_mib;
	u32 tx0_cidx;
	u32 tx0_didx;
	u32 tx1_cidx;
	u32 tx1_didx;
	/*WED_WDMA*/
	u32 wdma_stat;
	u32 wdma_rx0_mib;
	u32 wdma_rx1_mib;
	u32 wdma_rx0_recycle_mib;
	u32 wdma_rx1_recycle_mib;
	/*WED_WPDMA*/
	u32 wpdma_stat;
	u32 wpdma_tx0_mib;
	u32 wpdma_tx1_mib;
	/*WED_BM*/
	u32 bm_tx_stat;
	u32 txfree_to_bm_mib;
	u32 txbm_to_wdma_mib;
};

struct wed_ser_moudle_busy_cnt {
	/* TX */
	u32 reset_wed_tx_dma;
	u32 reset_wed_wdma_rx_drv;
	u32 reset_wdma_rx;
	u32 reset_wed_tx_bm;
	u32 reset_wed_wpdma_tx_drv;
	u32 reset_wed_rx_drv; /* tx free done */
	u32 reset_pao;
	/* RX */
	u32 reset_wed_wpdma_rx_d_drv;
	u32 reset_wed_rx_rro_data_3_1_drv;
	u32 reset_wed_rx_rro;
	u32 reset_wed_rx_rro_qm;
	u32 reset_wed_rx_rro_drv;
	u32 reset_wed_rx_route_qm;
	u32 reset_wdma_tx;
	u32 reset_wdma_tx_drv;
	u32 reset_wed_rx_dma;
	u32 reset_wed_rx_bm;
	u32 reset_wed_rx_page_bm;
};

struct wed_ser_ctrl {
	struct task_struct *ser_task;
	struct wed_ser_state state;
	struct wed_ser_moudle_busy_cnt ser_busy_cnt;
	u32 tx_dma_err_cnt;
	u32 tx_dma_ser_cnt;
	u32 wdma_err_cnt;
	u32 tx_bm_err_cnt;
	u32 wdma_ser_cnt;
	u32 wpdma_idle_cnt;
	u32 wpdma_ser_cnt;
	u32 tx_bm_ser_cnt;
	u32 period_time;
	u8 recovery;
	u8 wo_no_response;
	u8 wo_check_countdown;
	spinlock_t ser_lock;
};

struct wed_entry {
	u8 ver;
	u8 sub_ver;
	u8 branch;
	u8 eco;
	struct platform_device *pdev;
	unsigned long base_addr;
	u32 cr_base_addr;
	u32 hw_cap;
	u32 irq;
	u32 ext_int_mask;
#ifdef WED_RX_D_SUPPORT
	u32 ext_int_mask1;
	u32 ext_int_mask2;
	u32 ext_int_mask3;
#endif
#ifdef WED_DYNAMIC_TXBM_SUPPORT
	void *dybm_dl_tasks;
	u32 tbuf_alloc_times;
	u32 tbuf_free_times;
	u32 tkn_alloc_times;
#endif /*WED_DYNAMIC_TXBM_SUPPORT*/
#ifdef WED_DYNAMIC_RXBM_SUPPORT
	void *dybm_ul_tasks;
	u32 rbuf_alloc_times;
	u32 rbuf_free_times;
#endif /*WED_DYNAMIC_RXBM_SUPPORT*/
	struct sw_conf_t *sw_conf;
	struct wed_ser_ctrl ser_ctrl;
	struct wed_res_ctrl res_ctrl;
	void *proc;
	void *warp;
};

enum {
	WED_HW_CAP_32K_TXBUF = BIT(0),
	WED_HW_CAP_RX_OFFLOAD = BIT(1),
	WED_HW_CAP_RX_BYPASS = BIT(2),
	WED_HW_CAP_RX_KO = BIT(3),
	WED_HW_CAP_RX_WOCPU = BIT(4),
	WED_HW_CAP_TXD_PADDING = BIT(5),
	WED_HW_CAP_DYN_TXBM = BIT(6),
	WED_HW_CAP_DYN_RXBM = BIT(7),
	WED_HW_CAP_PASSIVE_INT = BIT(8),
	WED_HW_CAP_RRO_DBG_MODE = BIT(9),
	WED_HW_CAP_RRO_DBG_MEMCPY_MODE = BIT(10),
};

#define IS_WED_HW_CAP(_wed, _cap) (_cap & _wed->hw_cap)

enum {
	WED_PROC_TX_RING_BASIC = 0,
	WED_PROC_TX_BUF_BASIC = 1,
	WED_PROC_TX_BUF_INFO = 2,
	WED_PROC_TX_RING_CELL = 3,
	WED_PROC_TX_RING_RAW = 4,
	WED_PROC_DBG_INFO = 5,
	WED_PROC_TX_DYNAMIC_FREE = 6,
	WED_PROC_TX_DYNAMIC_ALLOC = 7,
	WED_PROC_TX_FREE_CNT = 8,
	WED_PROC_TX_RESET = 9,
	WED_PROC_RX_RESET = 10,
	WED_PROC_TX_DDR_PROF = 11,
	WED_PROC_RX_DDR_PROF = 12,
	WED_PROC_OFF_DDR_PROF = 13,
	WED_PROC_TXBM_INFO = 14,
	WED_PROC_RXBM_INFO = 15,
	WED_PROC_RX_DYNAMIC_FREE = 16,
	WED_PROC_RX_DYNAMIC_ALLOC = 17,
	WED_PROC_SER_ERR_CNT = 18,
	WED_PROC_PAO_WCID_STAT = 19,
	WED_PROC_PAO_SET_WCID_STAT = 20,
	WED_PROC_MEM_INFO = 21,
	WED_PROC_RRO_SE_GET_PN = 22,
	WED_PROC_END
};

int wed_init(struct platform_device *pdev, u8 idx, struct wed_entry *wed);
void wed_exit(struct platform_device *pdev, struct wed_entry *wed);
int wed_tx_ring_reset(struct wed_entry *entry);
int wed_rx_ring_reset(struct wed_entry *entry);
void wed_proc_handle(struct wed_entry *entry, char choice, char *arg);
#ifdef WED_DYNAMIC_TXBM_SUPPORT
int wed_token_buf_expend(struct wed_entry *entry, u32 grp_num);
void wed_token_buf_reduce(struct wed_entry *entry, u32 grp_num);
#endif /*WED_DYNAMIC_TXBM_SUPPORT*/
u32 wed_get_entry_num(void);
u8 wed_get_slot_map(u32 idx);
void wed_ser_dump(struct wed_entry *wed);
void wed_procinfo_dump(struct wed_entry *wed, struct seq_file *seq, int idx);
int wed_fdesc_init(struct wed_entry *wed, struct wed_pkt_info *info);
int wed_acquire_rx_token(struct wed_entry *wed, struct warp_dma_cb *cb, u32 *token_id);
int wed_release_rx_token(struct wed_entry *wed, u32 token_id);
int hif_txd_init(struct wed_entry *wed);
void hif_txd_exit(struct wed_entry *wed);
#endif /*_WED_H*/
