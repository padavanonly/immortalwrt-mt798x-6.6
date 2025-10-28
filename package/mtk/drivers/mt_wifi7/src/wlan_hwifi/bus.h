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

#ifndef __MTK_BUS_H
#define __MTK_BUS_H

#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/types.h>
#include "utility.h"

#define SKB_HEADROOM_RSV	192 /* max radiotap length 128 => 192 for EHT TLV*/
#define DESC_TX_MAX		2
#define DESC_RX_MAX		2

struct mtk_hwifi_drv;
struct mtk_chip;
struct mtk_tk_entry;

enum mtk_bus_type {
	MTK_BUS_WED = 0,
	MTK_BUS_PCI,
	MTK_BUS_AXI,
	MTK_BUS_USB,
	MTK_BUS_SDIO,
	MTK_BUS_NPU,
	MTK_BUS_NUM,
};

enum mtk_queue_attr {
	Q_TX_DATA,
	Q_TX_CMD,
	Q_TX_CMD_WM,
	Q_TX_CMD_WO,
	Q_TX_FWDL,
	Q_RX_DATA,
	Q_RX_DATA_WED,
	Q_RX_DATA_RRO,
	Q_RX_DATA_MSDU_PG,
	Q_RX_IND,
	Q_RX_RXDMAD_C,
	Q_RX_EVENT_TX_FREE_DONE,
	Q_RX_EVENT_WM,
	Q_RX_EVENT_WA,
	Q_RX_EVENT_SDO,
	Q_ATTR_NUM
};

enum mtk_queue_type {
	Q_TYPE_ACQ_BE = 0,
	Q_TYPE_ACQ_BK = 1,
	Q_TYPE_ACQ_VI = 2,
	Q_TYPE_ACQ_VO = 3,
	Q_TYPE_PRIO = 4,
	Q_TYPE_NUM,
};

enum {
	GET_PKT_DDONE,
	GET_PKT_IO,
	GET_PKT_METHOD_NUMS
};

enum buf_alloc_flags {
	BUF_DEFAULT = (1 << 0),
	BUF_ZERO = (1 << 1),
	BUF_DEBUG = (1 << 2),
};

enum buf_alloc_type {
	DYNAMIC_PAGE_ALLOC,
	DYNAMIC_SLAB_ALLOC,
	PRE_SLAB_ALLOC,
	DYNAMIC_PAGE_ALLOC_DEBUG,
	PKT_ALLOC_TYPE_NUMS
};

enum band_idx {
	BAND0 = 0,
	BAND1 = 1,
	BAND2 = 2,
};

enum tx_ring_bit {
	TX_RING16 = BIT(16),
	TX_RING17 = BIT(17),
	TX_RING18 = BIT(18),
	TX_RING19 = BIT(19),
	TX_RING20 = BIT(20),
	TX_RING21 = BIT(21),
	TX_RING22 = BIT(22),
};

enum rx_ring_bit {
	RX_RING0 = BIT(0),
	RX_RING1 = BIT(1),
	RX_RING2 = BIT(2),
	RX_RING3 = BIT(3),
	RX_RING4 = BIT(4),
	RX_RING5 = BIT(5),
	RX_RING6 = BIT(6),
	RX_RING7 = BIT(7),
	RX_RING8 = BIT(8),
	RX_RING9 = BIT(9),
};

enum node_type {
	MASTER_NODE = BIT(0),
	SLAVE_NODE = BIT(1),
	DISABLE_NODE = BIT(2),
	SHRINK_NODE = BIT(3),
};

enum bus_handler {
	MASTER_HANDLER = BIT(0),
	SLAVE_HANDLER = BIT(1),
};

enum {
	BUS_TX_DATA,
	BUS_TX_DROP,
	BUS_TX_SW,
};

enum rxdmad_drop {
	RXDMAD_NON_DROP = 0,
	RXDMAD_VER_ZREO_DROP = 1,
	RXDMAD_INCMD_REASON_DROP = 2,
};


struct mtk_bus_data {
	u32 value;
	u32 reg;
};

/** struct mtk_bus_io_ops - I/O operation for a specific BUS
 *
 * @param rr: Read Register
 * @param wr: Write Register
 */
struct mtk_bus_io_ops {
	u32 (*rr)(void *hw, u32 offset);
	void (*wr)(void *hw, u32 offset, u32 val);
	u32 (*rr_dbg)(void *hw, u32 offset);
	void (*wr_dbg)(void *hw, u32 offset, u32 val);
	void (*rr_range)(void *hw, u32 offset, void *buf, u32 size);
	void (*wr_range)(void *hw, u32 offset, void *buf, u32 size);
};

struct mtk_bus_tx_info {
	struct mtk_tk_entry *tk_entries;
	void *txq;
	void *txq_trans;
	u8 cnt;
	unsigned long flags;
};

/* need to small than NET_SKB_PAD */
struct mtk_bus_rx_info {
	void *pkt;
	union {
		dma_addr_t sdp0;
		struct sk_buff *skb;
	};
	u32 id;
	u16 len;
	u16 ppe_entry;
	u8 eth_hdr_ofst;
	u8 csrn;
	u8 drop		:4;
	u8 more		:1;
	u8 hw_path	:1;
	u8 hw_rro	:1;
	u8 ip_frag	:1;
	u8 wifi_frag	:1;
	u8 repeat_pkt	:1;
	u8 old_pkt	:1;
	u8 pn_chk_fail	:1;
#ifdef CONFIG_HWIFI_DBG
	u8 check_sn	:1;
	u16 sn;
	u16 i		:10;
	u16 j		:6;
#endif
};

/** struct mtk_bus_dma_ops - DMA operations for a specific bus
 *
 * @param alloc_resource: Allocate dma resource depende on profile
 * @param free_resource: Free dma resource for the trans
 * @param preinit_device: Preinit hardware device from the trans
 * @param init_device: Init hardware device from the trans
 * @param exit_device: Exit hardware device from the trans
 * @param tx_mcu_queue: Transmit a mcu packet to DMA mcu queue
 * @param request_rx: Check DMA resource for the txq
 * @param tx_data_queue: Transmit a MSUD packet to a DMA data queue
 * @param tx_kick: Trigger DMA to transmit packet
 * @param chip_attached: Attached the chip ops to bus_ops for
 *        chip base DMA operations
 * @param match: Match function for multiple bus
 * @param tx_free: DMA transmit free handler
 * @param start: DMA start
 * @param stop: DMA stop
 * @param get_txq: Get a DMA txq by band and tid
 * @param add_hw_rx: Add hardware offload session for Receive path
 * @param irq_handler: Interrupt handler
 * @param rro_write_dma_idx: write dma idx for warp hw
 * @param get_rro_mode: Get rro cap
 * @param queue_read: Read all DMA queues information
 * @param queue_dump: Dump all DMA information
 * @param set_pao_sta_info: set PAO setting
 * @param set_pn_check: set PN enable or not
 * @param get_tx_token_num: get tx token number from chip_ops
 */
struct mtk_bus_dma_ops {
	int (*alloc_resource)(void *trans, void *profile);
	void (*free_resource)(void *trans);
	int (*preinit_device)(void *trans);
	int (*init_device)(void *trans);
	int (*init_after_fwdl)(void *trans);
	void (*exit_device)(void *trans);
	int (*tx_mcu_queue)(void *trans, struct sk_buff *skb, enum mtk_queue_attr q);
	bool (*request_tx)(void *trans, void *txq);
	int (*tx_data_queue)(void *trans, struct mtk_bus_tx_info *tx_info);
	int (*tx_kick)(void *trans);
	int (*chip_attached)(void *trans, void *bus_ops);
	bool (*match)(void *master, void *slave);
	void (*tx_free)(void *trans, struct sk_buff *skb);
	int (*start)(void *trans);
	void (*stop)(void *trans);
	int (*start_traffic)(void *trans);
	int (*stop_traffic)(void *trans);
	int (*get_txq)(void *trans, u8 band_idx, u8 tid,
		       void **txq, void **txq_trans);
	int (*add_hw_rx)(void *trans, struct mtk_bus_rx_info *rx_info);
	int (*int_to_mcu)(void *trans, u32 status);
	int (*pause_trxq)(void *trans, bool enable);
	int (*init_sw_rxq)(void *trans);
	int (*exit_sw_rxq)(void *trans);
	irqreturn_t (*irq_handler)(int irq, void *data);
	int (*rro_write_dma_idx)(void *transc, u32 value);
	void * (*rro_page_hash_get)(void *trans, dma_addr_t pa);
	int (*get_rro_mode)(void *trans);
	int (*switch_node)(void *master, void *slave);
	int (*set_queue_ops)(void *trans);
	int (*set_free_notify_version)(void *trans, u8 version);
	int (*set_pao_sta_info)(void *trans, u16 wcid,
		u8 max_amsdu_nums, u32 max_amsdu_len, int remove_vlan, int hdrt_mode);
	int (*get_device)(void *trans, void **device);
	/*debugfs*/
	int (*queues_read)(void *trans, struct seq_file *s);
	bool (*init_rro_addr_elem_by_seid)(void *hw, u16 seid);
	int (*set_pn_check)(void *hw, u32 se_id, bool enable);
	int (*set_particular_to_host)(void *hw, bool enable);
	int (*get_tx_token_num)(void *trans, u16 tx_token_num[], u8 max_src_num);
	int (*get_rro_sp_page_num)(void *trans, u32 *page_num);
};

enum {
	BUS_TRANS_FLAG_MASTER,
	BUS_TRANS_FLAG_READY,
	BUS_TRANS_FLAG_START,
	BUS_TRANS_FLAG_BA_OFFLOAD,
	BUS_TRANS_FLAG_TX_OFFLOAD,
	BUS_TRANS_FLAG_RX_OFFLOAD,
	BUS_TRANS_MAX,
};

struct mtk_bus_dbg {
	u32 tx_cnt;
	u32 tx_err_cnt;
	u32 rx_cnt;
	u32 rx_err_cnt;
	u32 free_notify_cnt;
	u32 free_notify_err_cnt;
	u32 txs_cnt;
	u32 txs_err_cnt;
	u32 sdo_event_cnt;
	u32 sdo_event_err_cnt;
	u32 rx_leagcy_drop_cnt;
	u32 rx_pn_check_drop_cnt;
	u32 rx_incmd_reason_drop_cnt;
#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
	u32 kfifo_full_cnt;
	u32 kfifo_full_drop_by_hwifi_cnt;
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */
#ifdef CONFIG_HWIFI_DBG_ISR
	atomic_t *legacy_ring_irq_cnt;
	atomic_t *rss_ring_irq_cnt;
	atomic_t irq_cnt;
#endif
};

/** struct mtk_bus_cfg - Bus configuration for a specific chip
 *
 * @param id_table Chip id table for a specific bus type
 * @param profile Bus profile for a specific bus type
 * @param bus_ops Bus operation for a specific bus type
 * @param name Bus driver name
 * @param intr_opt_table Interrupt option type for bus
 * @param bus_drv_sz Size of bus driver data structure
 * @param bus_type Indicate master type for this bus configuration
 * @param bus_sub_type Indicate sub type for this bus, then master
 *        type is a tunnels
 * @param ms_type Indicate as a master bus or not for multi-bus
 * @param inter_opt_table_sz Interrupt option type table size
 */
struct mtk_bus_cfg {
	void *id_table;
	void *profile;
	void *bus_ops;
	char *name;
	void *intr_opt_table;
	u32 bus_drv_sz;
	u8 bus_type;
	u8 bus_sub_type;
	u8 ms_type;
	u8 intr_opt_table_sz;
};

/** struct mtk_bus_trans - Bus entity for bus framework
 *
 * This struct is used in bus framework indicate a HIF port.
 *
 * @param sid: System index for bus manager can manage.
 * @param name: Name of bus for debugfs can match
 * @param flag: Bus flags
 * @param pdev: Kernel device for coherence operations.
 * @param dev: Point to a hw_dev
 * @param next: Point to next slave bus_trans
 * @param master: Point to master bus_trans
 * @param bus_cfg: Point to bus configuration
 * @param io_ops: Point to bus driver io callbacks
 * @param dma_ops: Point to bus driver dma callbacks
 * @param rx_skb: Queue for receiving packets
 * @param rx_lock: Lock for protect receiving packets
 * @param bus_dbg: Bus debug information
 * @param ae_info: Input info of debugfs for querying addr_elem
 */
struct mtk_bus_trans {
	struct mtk_idr_entry sid;
	char *name;
	unsigned long flag;
	struct device *pdev;
	struct mtk_hw_dev *dev;
	struct mtk_bus_trans *next; /*multi bus support*/
	struct mtk_bus_trans *master; /*master bus_trans*/
	struct mtk_bus_cfg *bus_cfg;
	const struct mtk_bus_io_ops *io_ops;
	const struct mtk_bus_dma_ops *dma_ops;
	struct sk_buff_head rx_skb;
	spinlock_t rx_lock;
	struct mtk_bus_dbg bus_dbg;
	/* For rro_debug */
	struct addr_elem_info *ae_info;
};

/** struct mtk_bus_ops - Specific bus operations
 *
 * @param drv_register Register a new bus driver for a chip
 * @param drv_unregister Unregister a specific bus driver for a chip
 * @param drv_profile_dump Dump bus driver profile
 * @param drv_create_tunnel Create a tunnel for sub type BUS
 */
struct mtk_bus_ops {
	int (*drv_register)(void *bus_drv, void *priv_drv);
	int (*drv_unregister)(void *bus_drv, void *priv_drv);
	int (*drv_profile_dump)(struct seq_file *s, void *profile);
	int (*drv_create_tunnel)(void *priv_drv, struct mtk_bus_trans *trans, u32 bus_type);
};

/** struct mtk_bus_driver - Bus driver prepared information
 *
 * @param bus_type Define this bus driver's type
 * @param bus_sub_type Indicate sub type for this bus
 * @param ctl_size Size of specific bus_trans
 * @param ops Specific bus operations
 * @param sid System idx for this bus type
 */
struct mtk_bus_driver {
	u8 bus_type;
	u8 bus_sub_type;
	size_t ctl_size;
	struct mtk_bus_ops ops;
	struct mtk_idr_entry sid;
	struct mtk_bus_driver *sub_drv;
};

/** struct mtk_bus_mgmt - Bus manager
 *
 * @param driver Bus driver manager
 * @param trans Bus trans manager
 * @param mutex Bus manage protect
 */

struct mtk_bus_mgmt {
	struct mtk_idr_mgmt driver;
	struct mtk_idr_mgmt trans;
	struct mutex mutex;
};

/** mtk_bus_tx_ring_desc - The description of relation in TxRing
 * @param band_idx_bmp TxRing binds which band(s) in bitmap format
 * @param q_type_bmp TxRing type (AC or priority queue)
 */
struct mtk_bus_tx_ring_desc {
	u8 band_idx_bmp;
	u8 q_type_bmp;
};

/** struct mtk_bus_desc - The desciption of each bus
 * @param tx_ring_num The number of TxRing in the bus
 * @param mtk_bus_tx_ring_desc The description of relation in TxRing
 */
struct mtk_bus_desc {
	bool offload_disable;
	u8 tx_ring_num;
	struct mtk_bus_tx_ring_desc tx_ring_desc[DESC_TX_MAX];
	u32 ext_tx_tk_nums_master;
	u32 ext_tx_tk_nums_slave;
	u32 ext_rx_tk_nums_master;
	u32 ext_rx_tk_nums_slave;
	u32 ext_tx_buf_nums_master;
	u32 ext_tx_buf_nums_slave;
};

#define to_device(_trans) (((struct mtk_bus_trans *) _trans)->pdev)
#define to_name(_trans) (((struct mtk_bus_trans *) _trans)->name)
#define to_id(_trans) (((struct mtk_bus_trans *) _trans)->id)
#define to_hw_dev(_trans) (((struct mtk_bus_trans *) _trans)->dev)
#define to_flags(_trans) (&((struct mtk_bus_trans *) _trans)->flags)
#define to_bus_trans(_bus) ((struct mtk_bus_trans *) _bus)
#define to_bus_trans_dp(_bus) (*((struct mtk_bus_trans **) _bus))

static inline void
dbg_bus_tx_inc(struct mtk_bus_trans *trans)
{
	trans->bus_dbg.tx_cnt++;
}

static inline void
dbg_bus_tx_err_inc(struct mtk_bus_trans *trans)
{
	trans->bus_dbg.tx_err_cnt++;
}

static inline void
dbg_bus_rx_inc(struct mtk_bus_trans *trans)
{
	trans->bus_dbg.rx_cnt++;
}

static inline void
dbg_bus_rx_err_inc(struct mtk_bus_trans *trans)
{
	trans->bus_dbg.rx_err_cnt++;
}

static inline void
dbg_bus_free_notify_inc(struct mtk_bus_trans *trans)
{
	trans->bus_dbg.free_notify_cnt++;
}

static inline void
dbg_bus_free_notify_err_inc(struct mtk_bus_trans *trans)
{
	trans->bus_dbg.free_notify_err_cnt++;
}

static inline void
dbg_bus_txs_inc(struct mtk_bus_trans *trans)
{
	trans->bus_dbg.txs_cnt++;
}

static inline void
dbg_bus_txs_err_inc(struct mtk_bus_trans *trans)
{
	trans->bus_dbg.txs_err_cnt++;
}

static inline void
dbg_bus_sdo_event_inc(struct mtk_bus_trans *trans)
{
	trans->bus_dbg.sdo_event_cnt++;
}

static inline void
dbg_bus_sdo_event_err_inc(struct mtk_bus_trans *trans)
{
	trans->bus_dbg.sdo_event_err_cnt++;
}

static inline void
dbg_bus_rx_drop_inc(struct mtk_bus_trans *trans,
	struct mtk_bus_rx_info *rx_info)
{
	switch (rx_info->drop) {
	case RXDMAD_VER_ZREO_DROP:
		trans->bus_dbg.rx_leagcy_drop_cnt++;
		break;

	case RXDMAD_INCMD_REASON_DROP:
		trans->bus_dbg.rx_incmd_reason_drop_cnt++;
		break;

	default:
		trans->bus_dbg.rx_leagcy_drop_cnt++;
		break;
	}
}

#ifdef CONFIG_HWIFI_RX_PROCESS_WORKQUEUE
static inline void
dbg_bus_kfifo_full_inc(struct mtk_bus_trans *trans)
{
	trans->bus_dbg.kfifo_full_cnt++;
}

static inline void
dbg_bus_kfifo_full_drop_by_hwifi_inc(struct mtk_bus_trans *trans)
{
	trans->bus_dbg.kfifo_full_drop_by_hwifi_cnt++;
}
#endif /* CONFIG_HWIFI_RX_PROCESS_WORKQUEUE */

/*bus sub module initial*/
int
mtk_bus_init(struct mtk_hwifi_drv *drv, struct mtk_bus_mgmt *bus_mgmt);

int
mtk_bus_exit(struct mtk_bus_mgmt *bus_mgmt);

/*specific bus driver register*/
int
mtk_bus_driver_register(struct mtk_bus_driver *bus_driver);

int
mtk_bus_driver_unregister(struct mtk_bus_driver *bus_driver);

static inline int
mtk_bus_dump_profile(struct mtk_bus_driver *bus_driver, void *profile, struct seq_file *s)
{
	if (bus_driver->ops.drv_profile_dump)
		return bus_driver->ops.drv_profile_dump(s, profile);
	return -EINVAL;
}

/*a chip probe allocate resource*/
void *
mtk_bus_alloc_trans(struct device *pdev, void *driver,
	struct mtk_bus_io_ops *io_ops, struct mtk_bus_dma_ops *dma_ops);

int
mtk_bus_free_trans(void *trans);

/*a chip probe register to hwifi_dev*/
int
mtk_bus_register_trans(void *trans);

int
mtk_bus_unregister_trans(void *trans);

/*a chip register to bus*/
int
mtk_bus_register_chip(struct mtk_chip *chip);

int
mtk_bus_unregister_chip(struct mtk_chip *chip);

int
mtk_bus_rx_process(struct mtk_bus_trans *trans, u8 q_attr,
					struct sk_buff *skb);

int
mtk_bus_rx_poll(struct mtk_bus_trans *trans, struct napi_struct *napi, u8 band_idx);

int
mtk_bus_rx_ser_event(
	struct mtk_hw_dev *dev,
	u32 chip_id,
	u32 ser_level,
	u32 ser_event,
	u32 hw_id);

/*io ops*/
static inline u32
bus_read(struct mtk_bus_trans *bus, u32 offset)
{
	if (bus->io_ops->rr)
		return bus->io_ops->rr(bus, offset);
	return 0;
}

static inline void
bus_write(struct mtk_bus_trans *bus, u32 offset, u32 val)
{
	if (bus->io_ops->wr)
		bus->io_ops->wr(bus, offset, val);
}

static inline u32
bus_dbg_read(struct mtk_bus_trans *bus, u32 offset)
{
	if (bus->io_ops->rr_dbg)
		return bus->io_ops->rr_dbg(bus, offset);
	return 0;
}

static inline void
bus_dbg_write(struct mtk_bus_trans *bus, u32 offset, u32 val)
{
	if (bus->io_ops->wr_dbg)
		bus->io_ops->wr_dbg(bus, offset, val);
}

static inline u32
bus_rw(struct mtk_bus_trans *bus, u32 offset, u32 mask, u32 val)
{
	val |= bus_read(bus, offset) & ~mask;
	bus_write(bus, offset, val);
	return val;
}

static inline void
bus_set(struct mtk_bus_trans *bus, u32 offset, u32 val)
{
	bus_rw(bus, offset, 0, val);
}

static inline void
bus_clear(struct mtk_bus_trans *bus, u32 offset, u32 val)
{
	bus_rw(bus, offset, val, 0);
}

static inline void
bus_set_field(struct mtk_bus_trans *bus, u32 offset, u32 field, u32 val)
{
	bus_rw(bus, offset, field, field_prep(field, val));
}


static inline u32
bus_get_field(struct mtk_bus_trans *bus, u32 offset, u32 field)
{
	return field_get(field, bus_read(bus, offset));
}

static inline void
bus_read_range(struct mtk_bus_trans *bus, u32 offset, u8 *buf, u32 size)
{
	if (bus->io_ops->rr_range)
		bus->io_ops->rr_range(bus, offset, buf, size);
}

static inline void
bus_write_range(struct mtk_bus_trans *bus, u32 offset, u8 *buf, u32 size)
{
	if (bus->io_ops->wr_range)
		bus->io_ops->wr_range(bus, offset, buf, size);
}

/*dma operation*/
static inline int
mtk_bus_dma_alloc_resource(struct mtk_bus_trans *trans, void *profile)
{
	if (trans->dma_ops->alloc_resource)
		return trans->dma_ops->alloc_resource(trans, profile);
	return -EOPNOTSUPP;
}

static inline void
mtk_bus_dma_free_resource(struct mtk_bus_trans *trans)
{
	if (trans->dma_ops->free_resource)
		trans->dma_ops->free_resource(trans);
}

static inline int
mtk_bus_dma_preinit_device(struct mtk_bus_trans *trans)
{
	if (trans->dma_ops->preinit_device)
		return trans->dma_ops->preinit_device(trans);
	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_init_device(struct mtk_bus_trans *trans)
{
	if (trans->dma_ops->init_device)
		return trans->dma_ops->init_device(trans);
	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_init_after_fwdl(struct mtk_bus_trans *trans)
{
	if (trans->dma_ops->init_after_fwdl)
		return trans->dma_ops->init_after_fwdl(trans);
	return -EOPNOTSUPP;
}


static inline void
mtk_bus_dma_exit_device(struct mtk_bus_trans *trans)
{
	if (trans->dma_ops->exit_device)
		trans->dma_ops->exit_device(trans);
}

static inline int
mtk_bus_dma_chip_attached(struct mtk_bus_trans *trans, void *bus_ops)
{
	if (trans->dma_ops->chip_attached)
		return trans->dma_ops->chip_attached(trans, bus_ops);
	return -EOPNOTSUPP;
}

static inline bool
mtk_bus_dma_match(struct mtk_bus_trans *master, struct mtk_bus_trans *slave)
{
	if (master->dma_ops->match)
		return master->dma_ops->match(master, slave);
	return false;
}

static inline int
mtk_bus_dma_tx_mcu_queue(struct mtk_bus_trans *trans,
	struct sk_buff *skb, enum mtk_queue_attr q)
{
	if (trans->dma_ops->tx_mcu_queue)
		return trans->dma_ops->tx_mcu_queue(trans, skb, q);
	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_tx_data_queue(struct mtk_bus_trans *trans, struct mtk_bus_tx_info *tx_info)
{
	if (trans->dma_ops->tx_data_queue)
		return trans->dma_ops->tx_data_queue(trans, tx_info);
	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_tx_kick(struct mtk_bus_trans *trans)
{
	if (trans->dma_ops->tx_kick)
		return trans->dma_ops->tx_kick(trans);
	return -EOPNOTSUPP;
}

static inline void
mtk_bus_dma_tx_free(struct mtk_bus_trans *trans, struct sk_buff *skb)
{
	if (trans->dma_ops->tx_free)
		trans->dma_ops->tx_free(trans, skb);
}

static inline int
mtk_bus_dma_start(struct mtk_bus_trans *trans)
{
	if (trans->dma_ops->start)
		return trans->dma_ops->start(trans);
	return -EOPNOTSUPP;
}

static inline void
mtk_bus_dma_stop(struct mtk_bus_trans *trans)
{
	if (trans->dma_ops->stop)
		trans->dma_ops->stop(trans);
}

static inline int
mtk_bus_dma_start_traffic(struct mtk_bus_trans *trans)
{
	if (trans->dma_ops->start_traffic)
		return trans->dma_ops->start_traffic(trans);
	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_stop_traffic(struct mtk_bus_trans *trans)
{
	if (trans->dma_ops->stop_traffic)
		return trans->dma_ops->stop_traffic(trans);
	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_add_hw_rx(struct mtk_bus_trans *trans,
	struct mtk_bus_rx_info *rx_info)
{
	if (trans->dma_ops->add_hw_rx)
		return trans->dma_ops->add_hw_rx(trans, rx_info);
	return -EOPNOTSUPP;
}

/*dbugfs*/
static inline int
mtk_bus_dma_queues_read(struct mtk_bus_trans *trans, struct seq_file *s)
{
	if (trans->dma_ops->queues_read)
		return trans->dma_ops->queues_read(trans, s);
	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_get_txq(struct mtk_bus_trans *trans, u8 band_idx, u8 tid,
		    void **txq, void **txq_trans)
{
	if (trans->dma_ops->get_txq)
		return trans->dma_ops->get_txq(trans, band_idx, tid,
					       txq, txq_trans);
	return -EOPNOTSUPP;
}

static inline bool
mtk_bus_poll_msec(struct mtk_bus_trans *trans, u32 offset, u32 mask, u32 val,
		      int timeout)
{
	u32 cur;

	timeout /= 10;
	do {
		cur = bus_read(trans, offset) & mask;
		if (cur == val)
			return true;

		usleep_range(10000, 20000);
	} while (timeout-- > 0);

	return false;
}

static inline bool
mtk_bus_dma_request_tx(struct mtk_bus_trans *trans, void *txq)
{
	if (trans->dma_ops->request_tx)
		return trans->dma_ops->request_tx(trans, txq);
	return -EOPNOTSUPP;
}

static inline irqreturn_t
mtk_bus_dma_irq_handler(int irq, void *data)
{
	struct mtk_bus_trans *trans = to_bus_trans_dp(data);

	if (trans->dma_ops->irq_handler)
		return trans->dma_ops->irq_handler(irq, data);
	return 0;
}

static inline int
mtk_bus_dma_int_to_mcu(struct mtk_bus_trans *trans, u32 status)
{
	if (trans->dma_ops->int_to_mcu)
		return trans->dma_ops->int_to_mcu(trans, status);
	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_pause_trxq(struct mtk_bus_trans *trans, bool enable)
{
	if (trans->dma_ops->pause_trxq)
		return trans->dma_ops->pause_trxq(trans, enable);
	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_init_sw_rxq(struct mtk_bus_trans *trans)
{
	if (trans->dma_ops->init_sw_rxq)
		return trans->dma_ops->init_sw_rxq(trans);
	return -EOPNOTSUPP;
}


static inline int
mtk_bus_dma_exit_sw_rxq(struct mtk_bus_trans *trans)
{
	if (trans->dma_ops->exit_sw_rxq)
		return trans->dma_ops->exit_sw_rxq(trans);
	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_rro_write_dma_idx(struct mtk_bus_trans *trans, u32 value)
{
	if (trans->dma_ops->rro_write_dma_idx)
		return trans->dma_ops->rro_write_dma_idx(trans, value);
	return -EOPNOTSUPP;
}

static inline void *
mtk_bus_dma_rro_page_hash_get(struct mtk_bus_trans *trans, dma_addr_t pa)
{
	if (trans->dma_ops->rro_page_hash_get)
		return trans->dma_ops->rro_page_hash_get(trans, pa);
	return NULL;
}

static inline int
mtk_bus_dma_switch_node(struct mtk_bus_trans *master,
		struct mtk_bus_trans *slave)
{
	if (master->dma_ops->switch_node)
		return master->dma_ops->switch_node(master, slave);

	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_get_rro_mode(struct mtk_bus_trans *trans)
{
	if (trans->dma_ops->get_rro_mode)
		return trans->dma_ops->get_rro_mode(trans);

	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_set_pao_sta_info(struct mtk_bus_trans *trans, u16 wcid,
		u8 max_amsdu_nums, u32 max_amsdu_len, int remove_vlan, int hdrt_mode)
{
	if (trans->dma_ops->set_pao_sta_info)
		return trans->dma_ops->set_pao_sta_info(trans, wcid,
			max_amsdu_nums, max_amsdu_len, remove_vlan, hdrt_mode);

	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_set_pn_check(struct mtk_bus_trans *trans, u32 se_id, bool enable)
{
	if (trans->dma_ops->set_pn_check)
		return trans->dma_ops->set_pn_check(trans, se_id, enable);
	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_set_particular_to_host(struct mtk_bus_trans *trans, bool enable)
{
	if (trans->dma_ops->set_particular_to_host)
		return trans->dma_ops->set_particular_to_host(trans, enable);
	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_get_device(struct mtk_bus_trans *trans, void **device)
{
	if (trans->dma_ops->get_device)
		return trans->dma_ops->get_device(trans, device);
	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_get_tx_token_num(struct mtk_bus_trans *trans, u16 tx_token_num[], u8 max_src_num)
{
	if (trans->dma_ops->get_tx_token_num)
		return trans->dma_ops->get_tx_token_num(trans, tx_token_num, max_src_num);
	return -EOPNOTSUPP;
}

static inline int
mtk_bus_dma_get_rro_sp_page_num(struct mtk_bus_trans *trans, u32 *page_num)
{
	if (trans->dma_ops->get_rro_sp_page_num)
		return trans->dma_ops->get_rro_sp_page_num(trans, page_num);
	return -EOPNOTSUPP;
}

int
mtk_bus_get_rro_sp_page_num(struct mtk_bus_trans *trans, u32 *page_num);

int
mtk_bus_get_tx_token_num(struct mtk_bus_trans *trans, u16 tx_token_num[], u8 max_src_num);

int
mtk_bus_set_pn_check(struct mtk_bus_trans *trans, u16 wcid, u16 se_id, bool enable);

int
mtk_bus_set_particular_to_host(struct mtk_bus_trans *trans, bool enable);

int
mtk_bus_set_pao_sta_info(struct mtk_bus_trans *trans, u16 wcid,
		u8 max_amsdu_nums, u32 max_amsdu_len, int remove_vlan, int hdrt_mode);

int
mtk_bus_preinit_device(struct mtk_bus_trans *trans);

int
mtk_bus_init_device(struct mtk_bus_trans *trans);

int
mtk_bus_init_after_fwdl(struct mtk_bus_trans *trans);

void
mtk_bus_exit_device(struct mtk_bus_trans *trans);

int
mtk_bus_start_device(struct mtk_bus_trans *trans);

void
mtk_bus_stop_device(struct mtk_bus_trans *trans);

int
mtk_bus_start_traffic(struct mtk_bus_trans *trans);

int
mtk_bus_stop_traffic(struct mtk_bus_trans *trans);

int
mtk_bus_get_txq(struct mtk_bus_trans *trans, u8 band_idx, u8 tid,
		void **txq, void **txq_trans);

int
mtk_bus_tx_kick(struct mtk_bus_trans *trans);

int
mtk_bus_queues_read(struct seq_file *s, void *data);

#define mtk_bus_get_device mtk_bus_dma_get_device
#endif
