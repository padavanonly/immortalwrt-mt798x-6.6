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

#ifndef __MTK_PCI_DMA_H
#define __MTK_PCI_DMA_H

#include "config.h"
#include "chips.h"
#include "mtk_pci.h"
#include <wlan_tr.h>

#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
#include <linux/workqueue.h>
#include <linux/kfifo.h>
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */

/* txdmad definition */
/* dw0 */
#define TXDMAD_SDP0_MASK		0xffffffff
#define TXDMAD_SDP0_SHIFT		0

/* dw1 */
#define TXDMAD_SDL1_MASK		0x00003fff
#define TXDMAD_SDL1_SHIFT		0
#define TXDMAD_LS1_SHIFT		14
#define TXDMAD_SDL0_MASK		0x3fff0000
#define TXDMAD_SDL0_SHIFT		16
#define TXDMAD_LS0_SHIFT		30
#define TXDMAD_DDONE_SHIFT		31

/* dw2 */
#define TXDMAD_SDP1_MASK		0xffffffff
#define TXDMAD_SDP1_SHIFT		0

/* dw3 */
#define TXDMAD_SDP0_H_MASK		0x0000ffff
#define TXDMAD_SDP0_H_SHIFT		0
#define TXDMAD_SDP1_H_MASK		0xffff0000
#define TXDMAD_SDP1_H_SHIFT		16


/* rxdmad definition */
/* dw0 */
#define RXDMAD_SDP0_MASK		GENMASK(31, 0)
#define RXDMAD_SDP0_SHIFT		0

/* dw1 */
#define RXDMAD_VER_MASK			BIT(7)
#define RXDMAD_VER_SHIFT		7
#define RXDMAD_TO_HOST_MASK		BIT(8)
#define RXDMAD_TO_HOST_SHIFT		8
#define RXDMAD_SDL0_MASK		GENMASK(29, 16)
#define RXDMAD_SDL0_SHIFT		16
#define RXDMAD_LS0_MASK			BIT(30)
#define RXDMAD_LS0_SHIFT		30
#define RXDMAD_DDONE_MASK		BIT(31)
#define RXDMAD_DDONE_SHIFT		31
/* RRO 2.X */
#define RXDMAD_V0_RING_INFO_MASK	BIT(9)
#define RXDMAD_V0_RING_INFO_SHIFT	9
#define RXDMAD_TO_HOST_A_MASK		BIT(12)
#define RXDMAD_TO_HOST_A_SHITF		12
#define RXDMAD_RXD_ERR_MASK		BIT(13)
#define RXDMAD_RXD_ERR_SHIFT		13
#define RXDMAD_RXD_DROP_MASK		BIT(14)
#define RXDMAD_RXD_DROP_SHIFT		14
#define RXDMAD_M_DONE_MASK		BIT(15)
#define RXDMAD_M_DONE_SHIFT		15
/* RRO 3.X */
#define RXDMAD_V1_RING_INFO_MASK	GENMASK(10, 9)
#define RXDMAD_V1_RING_INFO_SHIFT	9
#define RXDMAD_DST_SEL_MASK		GENMASK(12, 11)
#define RXDMAD_DST_SEL_SHIFT		11
#define RXDMAD_PN_CHK_FAIL_MASK		BIT(13)
#define RXDMAD_PN_CHK_FAIL_SHIFT	13
#define RXDMAD_HEAD_MSDU_MASK		BIT(14)
#define RXDMAD_HEAD_MSDU_SHIFT		14

/* dw2 */
#define RXDMAD_SDP0_H_MASK		GENMASK(3, 0)
#define RXDMAD_SDP0_H_SHIFT		0
#define RXDMAD_TOKEN_ID_MASK		GENMASK(31, 16)
#define RXDMAD_TOKEN_ID_SHIFT		16
/* RRO 3.X */
#define RXDMAD_IND_REASON_MASK		GENMASK(15, 12)
#define RXDMAD_IND_REASON_SHIFT		12

/* dw3 */
#define RXDMAD_RX_INFO_MASK		GENMASK(31, 0)
#define RXDMAD_RX_INFO_SHIFT		0
#define RXDMAD_CS_STATUS_MASK		GENMASK(3, 0)
#define RXDMAD_CS_STATUS_SHIFT		0
#define RXDMAD_CS_TYPE_MASK		GENMASK(7, 4)
#define RXDMAD_CS_TYPE_SHIFT		4
#define RXDMAD_C_MASK			BIT(8)
#define RXDMAD_C_SHIFT			8
#define RXDMAD_F_MASK			BIT(9)
#define RXDMAD_F_SHIFT			9
#define RXDMAD_UN_MASK			BIT(10)
#define RXDMAD_UN_SHIFT			10
#define RXDMAD_CSRN_MASK		GENMASK(15, 11)
#define RXDMAD_CSRN_SHIFT		11
#define RXDMAD_PPE_ENTRY_MASK		GENMASK(30, 16)
#define RXDMAD_PPE_ENTRY_SHIFT		16
#define RXDMAD_PPE_VLD_MASK		BIT(31)
#define RXDMAD_PPE_VLD_SHIFT		31
/* RRO 3.X */
#define RXDMAD_MAGIC_CNT_MASK		GENMASK(31, 28)
#define RXDMAD_MAGIC_CNT_SHIFT		28

struct pci_dma_desc {
	__le32 dw0;
	__le32 dw1;
	__le32 dw2;
	__le32 dw3;
} __packed __aligned(4);

#define BASE_PTR_MASK		0xffffffff
#define BASE_PTR_SHIFT		0
#define BASE_PTR_EXT_MASK	0xf0000
#define BASE_PTR_EXT_SHIFT	16
#define DMA_ADDR_H_SHIFT	32
#define MT_CT_PARSE_LEN		72
#define MT_CT_DMA_BUF_NUM	2
#define MAX_IRQ_NAME_LEN	32
#define MAX_ISR_NUM 32
/* WDMA RX RING need 32 bytes alignment */
#define MT_TX_BUF_LEN 1920

/* TODO: check other fields, it set/clear other fields to zero */
#define clear_ddone(_va) (((struct pci_dma_desc *)_va)->dw1 = cpu_to_le32(0))
#define set_ddone(_va) (\
	((struct pci_dma_desc *)_va)->dw1 =\
			cpu_to_le32(BIT(TXDMAD_DDONE_SHIFT)))

enum {
	PCI_FLAG_MSI,
	PCI_FLAG_WHNAT_TX,
	PCI_FLAG_WHNAT_RX,
	PCI_FLAG_CTXD,
	PCI_FLAG_TWO_PCIE,
	PCI_FLAG_PREFETCH_MANUAL,
	PCI_FLAG_DELAY_INTR,
	PCI_FLAG_CT,
	PCI_FLAG_WHNAT_RRO_DBG,
	PCI_FLAG_WHNAT_RRO_DBG_MEMCPY,
	PCI_FLAG_MAX
};

enum Q_FLAG {
	Q_FLAG_WRITE_EMI,
};

enum RQ_FLAG {
	RQ_FLAG_INIT_AFTER_FWDL_CPLT,
	RQ_FLAG_RX_DATA_COPY,
	RQ_FLAG_RX_DATA_TKID,
};

enum {
	PCI_STATE_ISR_START,
	PCI_STATE_ISR_PEDING,
	PCI_STATE_MAX
};

enum {
	LEGACY_IDX,
	RSS_IDX,
	MAX_MAP_IDX
};

/* sync with Logan driver: hif_base.h */
enum PACKET_TYPE {
	TX_DATA,
	TX_DATA_HIGH_PRIO,
	TX_MGMT,
	TX_ALTX,
	TX_CMD,
	TX_FW_DL,
	TX_DATA_PS,
	RX_PPE_VALID,
	PACKET_TYPE_NUM,
};

struct pci_queue_desc {
	enum mtk_queue_attr q_attr;
	u32 hw_desc_base;
	u32 hw_int_mask;
	u32 hw_didx_mask;
	u32 hw_attr_ena_mask;
	u32 hw_magic_ena_mask;
	u32 desc_size;
	u16 q_size;
	bool disable;
	u8 band_idx_bmp;
	u8 isr_map_idx;
	u8 emi_ring_idx;
	char *const q_info;
};

struct pci_queue_mp_attr_desc {
	/* Multi-port attribute */
	u32 ring_no;
	u8 handled_by;
	u8 handled_by_wed_mode;
};

#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
struct rx_offload_desc {
	u16 kfifo_size;
	bool kfifo_enable;
};
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */

struct pci_tx_queue_desc {
	struct pci_queue_desc cmm;
	struct pci_queue_mp_attr_desc mp_attr;
	/*tx specific*/
};

struct pci_rx_queue_desc {
	struct pci_queue_desc cmm;
	struct pci_queue_mp_attr_desc mp_attr;
	u16 rx_buf_size;
	bool magic_enable;
	bool attr_enable;
	enum bus_handler free_done_handled_by;
	enum txfreedone_path txfreedone_path;
#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
	struct rx_offload_desc rx_offload;
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */
};

struct pci_queue_layout {
	const struct pci_tx_queue_desc *tx_queue_layout;
	const struct pci_rx_queue_desc *rx_queue_layout;
};

struct rro_chip_profile {
	u32 ack_sn_addr;
	u32 ack_sn_mask;
	u32 ack_sn_seid_mask;
	u8 ack_sn_shift;
	u8 ack_sn_seid_shift;
	u8 ack_sn_update_cnt;
	u8 rro_mode;
	u32 sign_base_0_addr;
	u32 sign_base_1_addr;
	u32 sign_base_1_en_shift;
	u32 debug_mode_rxd_addr;
	u16 msdu_page_fp_cnt;
	u32 dbg_rd_ctrl_mask;
	u32 dbg_rd_ctrl_addr;
	u32 dbg_rdat_dw0;
	u32 dbg_rdat_dw1;
	u32 dbg_rdat_dw2;
	u32 dbg_rdat_dw3;
	u16 max_rro_se_cnt;
	u16 addr_elem_cr_cnt;
};

struct wed_v1_compatible_config {
	u32 en_addr;
	u32 en_msk;
	u32 tx0_addr;
	u32 tx0_msk;
	u32 tx0_id;
	u32 tx1_addr;
	u32 tx1_msk;
	u32 tx1_id;
	u32 rx1_addr;
	u32 rx1_msk;
	u32 rx1_id;
};

struct pci_dma_buf {
	size_t alloc_size;
	void *alloc_va;
	dma_addr_t alloc_pa;
};

struct pci_queue_buf {
	dma_addr_t addr;
	int len;
};

struct pci_dma_cb {
	size_t alloc_size;
	void *alloc_va;
	dma_addr_t alloc_pa;
	dma_addr_t pkt_pa;
	void *pkt;
	void *next_pkt;
	struct pci_dma_buf dma_buf;
} ____cacheline_aligned;

struct pci_flow_ctrl {
	u32 tx_ring_low_water_mark;
	u32 tx_ring_high_water_mark;
	u32 tx_ring_full_cnt;
};

struct pci_queue {
	enum mtk_queue_attr q_attr;
	u32 hw_didx_addr;
	u32 hw_cidx_addr;
	u32 hw_desc_base;
	u32 hw_cnt_addr;
	u32 hw_int_mask;
	u32 hw_didx_mask;
	u32 hw_attr_ena_mask;
	u32 hw_magic_ena_mask;
	u32 head;
	u32 tail;
	u32 desc_size;
	struct pci_dma_cb *cb;
	struct pci_dma_buf desc_ring;
	spinlock_t lock;
	u16 q_size;
	bool disable;
	u8 resource_idx;
	u8 band_idx_bmp;
	u8 band_num_sup;
	u8 q_type_bmp;
	u8 isr_map_idx;
	u8 vec_id;
	u8 emi_ring_idx;
	u16 *emi_cidx_addr;
	unsigned long q_flags;
	struct napi_struct napi;
	char *q_info;
};

struct pci_queue_mp_attr {
	u32 ring_no;
	u8 handled_by;
	u8 handled_by_wed_mode;
};

struct pci_tx_queue {
	/*inher pci queue, need to the same as.*/
	struct pci_queue q;
	struct pci_queue_mp_attr q_mp_attr;
	/*txq specific start in here*/
	unsigned long long tx_ring_state;
	struct pci_flow_ctrl flow_ctrl;
	spinlock_t done_lock;
	u8 cur_txd_cnt;
} ____cacheline_aligned;

#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
struct mtk_rx_process {
	struct mtk_bus_trans *trans;
	struct sk_buff *skb;
	u8 q_attr;
};
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */

struct pci_rx_queue {
	/*inher pci queue, need to the same as.*/
	struct pci_queue q;
	struct pci_queue_mp_attr q_mp_attr;
	/*rxq specific start in here*/
	struct page_frag_cache rx_page;
	unsigned long rq_flags;
	u8 buf_flags;
	u8 get_pkt_method;
	u16 cur_free_buf_len;
	u16 rx_buf_size;
	u8 buf_debug;
	struct sk_buff *rx_frag_head;
	u32 tail_sid;
	bool attr_enable;
	bool magic_enable;
	u8 magic_cnt;
	u8 free_done_handled_by;
	u8 txfreedone_path;
	u8 need_init_after_fwdl;
	struct pci_rxq_ops *q_ops;
	gfp_t gfp_flags;
#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
	struct rx_offload_desc rx_offload;
	struct workqueue_struct *bus_rx_process_wq;
	struct delayed_work bus_rx_process_work;
	DECLARE_KFIFO_PTR(bus_rx_process_fifo, struct mtk_rx_process);
	int work_on_cpu;
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */
} ____cacheline_aligned;

struct pci_irq_vector_data {
	struct mtk_bus_trans *bus_trans;
	char irq_name[MAX_IRQ_NAME_LEN];
	u8 attr;
	union attr_info info;
	u8 isr_map_idx;
	u8 ring_num;
	bool repeat;
	u32 ring_id;
	u32 irq;
	u32 int_ena_mask;
	u32 int_dis_mask;
};

struct pci_int_mask_map {
	u32 int_ena_addr;
	u32 int_sts_addr;
	u32 int_ena_set_addr;
	u32 int_ena_clr_addr;
	u32 int_ena_mask;
	u32 int_dis_mask;
};

struct pci_chip_profile {
	struct pci_queue_layout queue_layout;
	struct pci_int_mask_map int_mask_map[MAX_MAP_IDX];
	u32 lp_fw_own_reg_addr;
	/*for wed request cr*/
	u32 tx_dma_glo_reg_addr;
	u32 rx_dma_glo_reg_addr;
	u32 int_ser_reg_addr;
	u32 int_ser_mask;
	u32 pause_trxq_reg_addr;
	u32 pause_trxq_mask;
	/* swi mcu2host */
	u32 int_enable_swi_mask;
	u32 mcu2host_sw_int_ena_addr;
	u32 mcu2host_sw_int_sta_addr;
	u32 mcu2host_sw_int_ser_mask;
	u32 mcu2host_sw_wdt_ser_mask;
	u32 dma_addr;
	u8 mcu2host_sw_int_isr_map_idx;
	u8 txq_num;
	u8 rxq_num;
	u8 coherent_dma_addr_size;
	u8 non_coherent_dma_addr_size;
	struct rro_chip_profile rro_profile;
	struct wed_v1_compatible_config compatible_config;

};

/** struct pci_trans - PCI bus specific data structure
 *
 * This struct is indicate a PCI port and be allocated with bus_trans
 * used for pci specific operations
 *
 * @param parant: Bus trans must be the first member
 * @param napi_dev: Used for created NAPI task for interrupt handler
 * @param pdev: Kernel PCI device for pci related operations
 * @param regs: PCIE I/O mapping address
 * @param int_enable_mask: Enable value of interrupt mask
 * @param irq: Interrupt request id
 * @param id: Chip id for this HIF port
 * @param int_ena_reg_addr: Address of interrupt mask
 * @param int_src_reg_addr: address of Interrupt status
 * @param int_disable_mask: Value of current disabled interrupt mask
 * @param lp_fw_own_reg_addr: Address of fw own control register for low power
 * @param tx_dma_glo_reg_addr: Address of TX DMA_GLO_CFG
 * @param rx_dma_glo_reg_addr: Address of RX DMA_GLO_CFG
 * @param int_ser_reg_addr: Address of SER register
 * @param int_ser_mask: Mask of SER control value
 * @param pause_trxq_reg_addr: Address of TX/RX queue pause register
 * @param pause_trxq_mask: Mask of TX/RX queue pause
 * @param dma_addr: Base Address of DMA
 * @param kick_txq_bitmap: TX ring bitmap for tx control
 * @param txq_num: Number of TX ring
 * @param rxq_num: Number of RX ring
 * @param irq_lock: Lock for interrupt handle
 * @param rxq: RX ring information
 * @param txq: TX ring information
 * @param chip_ops: Chip specific pdma control
 * @param flags: PDMA/PCI related support flags
 * @param state: Status of PDMA/PCI
 * @param isr_map: NAPI table for per interrupt status bit
 * @param data_txq: TX ring array for mapping per band/AC
 * @param int_enable_swi_mask: Value of sw defined mask register
 * @param mcu2host_sw_int_ena_addr: Address of sw define mask register
 * @param mcu2host_sw_int_sta_addr: Address of sw define status register
 * @param mcu2host_sw_int_ser_mask: Value of sw define SER status
 * @param mcu2host_sw_wdt_ser_mask: Value of WDT define SER status
 * @param swi_napi: NAPI for sw interrupt handler
 */
struct pci_trans {
	struct mtk_bus_trans parent;
	struct net_device napi_dev;
	struct pci_dev *pdev;
	struct pci_irq_vector_data vec_data[MAX_VEC_NUM];
	struct pci_int_mask_map int_mask_map[MAX_MAP_IDX];
	void __iomem *regs;
	u32 id;
	u32 lp_fw_own_reg_addr;
	u32 tx_dma_glo_reg_addr;
	u32 rx_dma_glo_reg_addr;
	u32 int_ser_reg_addr;
	u32 int_ser_mask;
	u32 pause_trxq_reg_addr;
	u32 pause_trxq_mask;
	u32 dma_addr;
	u16 kick_txq_bitmap;
	u8 txq_num;
	u8 rxq_num;
	u8 coherent_dma_addr_size;
	u8 non_coherent_dma_addr_size;
	bool offload_disable;
	spinlock_t irq_lock;
	struct pci_rx_queue *rxq;
	struct pci_tx_queue *txq;
	struct pci_chip_ops *chip_ops;
	unsigned long flags;
	unsigned long state;
	struct napi_struct *isr_map[MAX_MAP_IDX][MAX_ISR_NUM];
	struct {
		u8 rro_mode;
		void *rro_cfg;
	};
	struct {
		u8 mcu2host_sw_int_isr_map_idx;
		u8 mcu2host_sw_int_vec_id;
		u32 int_enable_swi_mask;
		u32 mcu2host_sw_int_ena_addr;
		u32 mcu2host_sw_int_sta_addr;
		u32 mcu2host_sw_int_ser_mask;
		u32 mcu2host_sw_wdt_ser_mask;
		struct napi_struct swi_napi;
	};
	enum pcie_gen pci_gen;
	enum pcie_link_width pci_lane;
};

struct pci_rxq_ops {
	int (*get_data)(struct pci_trans *trans,
	struct pci_queue *q, u32 idx, struct mtk_bus_rx_info *rx_info);
	int (*process)(struct pci_trans *trans,
		struct pci_queue *q, int budget);
	void (*init)(struct pci_trans *trans, struct pci_rx_queue *rq);
	void (*exit)(struct pci_trans *trans, struct pci_rx_queue *rq);
};

struct pci_chip_ops {
	int (*dma_init)(struct pci_trans *trans);
	int (*dma_enable)(struct pci_trans *trans);
	int (*dma_disable)(struct pci_trans *trans);
	bool (*dma_match)(struct pci_trans *master, struct pci_trans *slave);
	int (*dma_schdl_init)(struct pci_trans *trans);
	int (*rro_hw_init)(struct pci_trans *trans);
	int (*set_free_notify_ver)(struct pci_trans *trans, u8 version);
	int (*option_init)(struct pci_trans *trans);
	int (*get_tx_token_num)(struct pci_trans *trans, u16 tx_token_num[], u8 max_src_num);
	int (*dbg_init)(struct pci_trans *ptrans);
	void (*dbg_exit)(struct pci_trans *ptrans);
	int (*dmy_init)(struct pci_trans *ptrans);
	void (*dmy_exit)(struct pci_trans *ptrans);
};

static inline int
pci_chip_dma_enable(struct pci_trans *trans)
{
	if (trans->chip_ops->dma_enable)
		return trans->chip_ops->dma_enable(trans);
	return -EOPNOTSUPP;
}

static inline int
pci_chip_dma_disable(struct pci_trans *trans)
{
	if (trans->chip_ops->dma_disable)
		return trans->chip_ops->dma_disable(trans);
	return -EOPNOTSUPP;
}

static inline int
pci_chip_dma_init(struct pci_trans *trans)
{
	if (trans->chip_ops->dma_init)
		return trans->chip_ops->dma_init(trans);
	return -EOPNOTSUPP;
}

static inline int
pci_dma_sw_init_dbg(struct pci_trans *trans)
{
	if (trans->chip_ops->dbg_init)
		return trans->chip_ops->dbg_init(trans);

	return 0;
}

static inline void
pci_dma_sw_exit_dbg(struct pci_trans *trans)
{
	if (trans->chip_ops->dbg_exit)
		trans->chip_ops->dbg_exit(trans);
}

static inline int
pci_dma_sw_init_dmy(struct pci_trans *trans)
{
	if (trans->chip_ops->dmy_init)
		return trans->chip_ops->dmy_init(trans);

	return 0;
}

static inline void
pci_dma_sw_exit_dmy(struct pci_trans *trans)
{
	if (trans->chip_ops->dmy_exit)
		trans->chip_ops->dmy_exit(trans);
}

static inline int
pci_chip_rro_hw_init(struct pci_trans *trans)
{
	if (trans->chip_ops->rro_hw_init)
		return trans->chip_ops->rro_hw_init(trans);
	return -EOPNOTSUPP;
}

static inline int
pci_chip_set_free_notify_ver(struct pci_trans *trans, u8 version)
{
	if (trans->chip_ops->set_free_notify_ver)
		return trans->chip_ops->set_free_notify_ver(trans, version);
	return -EOPNOTSUPP;
}

static inline int
pci_chip_option_init(struct pci_trans *trans)
{
	if (trans->chip_ops->option_init)
		return trans->chip_ops->option_init(trans);
	return 0;
}

static inline int
pci_chip_get_tx_token_num(struct pci_trans *trans, u16 tx_token_num[], u8 max_src_num)
{
	if (trans->chip_ops->get_tx_token_num)
		return trans->chip_ops->get_tx_token_num(trans, tx_token_num, max_src_num);
	return -EOPNOTSUPP;
}

static inline bool
pci_chip_dma_match(struct pci_trans *master, struct pci_trans *slave)
{
	if (master->chip_ops->dma_match)
		return master->chip_ops->dma_match(master, slave);
	return false;
}

static inline int
pci_chip_dma_schdl_init(struct pci_trans *trans)
{
	if (trans->chip_ops->dma_schdl_init)
		trans->chip_ops->dma_schdl_init(trans);
	return -EOPNOTSUPP;
}

#define to_pci_trans(_bus) ((struct pci_trans *) _bus)
#define to_rx_tk_mgmt(_bus) (&(to_hw_dev(_bus))->hw_ctrl.rx_tk_mgmt)
#define bus_to_regs(_bus) (((struct pci_trans *)(to_pci_trans(_bus)))->regs)

extern struct pci_rxq_ops data_rxq_ops;
extern struct pci_rxq_ops data_cp_rxq_ops;
extern struct pci_rxq_ops evt_rxq_ops;

extern struct mtk_bus_dma_ops pci_dma_ops;
extern struct mtk_bus_io_ops pci_mmio_ops;

extern void *pci_dma_rx_pkt_alloc(
	struct device *dev, struct pci_rx_queue *rq,
	struct pci_dma_buf *buf, bool map);
extern inline void fill_rxd(struct pci_dma_cb *cb,
			    u32 id, u32 magic_cnt);
extern inline void pci_dma_kick_queue(
	struct pci_trans *trans, struct pci_queue *q);
extern int pci_dma_rx_process(struct pci_trans *trans,
	struct pci_queue *q, int budget);
extern void pci_dma_init_rx_data_queue(
	struct pci_trans *trans, struct pci_rx_queue *rq);
extern void pci_dma_exit_rx_data_queue(
	struct pci_trans *trans, struct pci_rx_queue *rq);
extern void pci_dma_gather_frag(
	struct pci_trans *trans, struct pci_rx_queue *rq,
	struct mtk_bus_rx_info *rx_info);
extern inline size_t rx_pkt_total_size(size_t dma_buf_size);

#endif
