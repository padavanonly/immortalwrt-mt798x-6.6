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

#ifndef __MTK_CORE_H__
#define __MTK_CORE_H__

#include <linux/debugfs.h>
#include <linux/list.h>
#include <linux/notifier.h>
#include "os/mt_linux.h"
#include "config.h"
#include "debug.h"
#include "debugfs.h"
#include "mcu/mcu.h"
#include "hwres_mgmt.h"
#include "bus.h"
#include "chips.h"
#include "wsys.h"
#include "tk_mgmt.h"

/** struct mtk_hw_ops - Hardware callbacks for different generation arch.
 *
 * This callbacks defined the operations for control differet
 * generation architecture.
 *
 * @param query_wa: Query WA information
 * @param get_mem: Get client memory content
 * @param rx_pkt: Receive a data packet from bus
 * @param rx_event: Receive a event packet from bus
 * @param write_txd: Write the TXD content by tx_pkt_info
 * @param write_txp: Write the TXP content by tx_pkt_info
 * @param write_mcu_txd: Write the MCU TXD content by mcu_txblk
 * @param tx_skb_umap: Unmap a skb buffer from txd
 * @param fw_log_2_host: Receive a fwlog2 host event from BUS
 * @param set_wa: Set WA configure
 */
struct mtk_hw_ops {
	int (*query_wa)(struct mtk_hw_dev *dev, u32 ctrl);
	int (*get_mem)(struct mtk_hw_dev *dev, u32 addr, u8 *data);
	void (*rx_pkt)(struct mtk_hw_dev *dev, struct mtk_bus_trans *trans,
		struct sk_buff *skb);
	void (*rx_event)(struct mtk_hw_dev *dev, struct mtk_bus_trans *trans,
		struct sk_buff *skb, u32 mcu_type);
	int (*write_txd)(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
		struct mtk_tk_entry *tk_entry, void *tx_pkt_info);
	int (*write_mac_txd)(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
		struct mtk_tk_entry *tk_entry, void *tx_pkt_info);
	int (*write_hif_txd)(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
		struct mtk_tk_entry *tk_entry, void *tx_pkt_info);
	int (*write_hif_txd_legacy)(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
		struct mtk_tk_entry *tk_entry, void *tx_pkt_info);
	int (*write_txp)(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
		void *tx_pkt_info);
	int (*write_mac_txp)(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
		void *tx_pkt_info);
	int (*write_hif_txp)(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
		void *tx_pkt_info);
	int (*write_hif_txp_legacy)(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
		void *tx_pkt_info);
	void (*skb_unmap_txp)(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry);
	void (*skb_unmap_mac_txp)(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry);
	void (*skb_unmap_hif_txp)(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry);
	void (*skb_unmap_hif_txp_legacy)(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry);
	int (*write_mcu_txd)(struct mtk_hw_dev *dev, struct mtk_mcu_txblk *mcu_txblk);
	int (*write_mcu_txd_uni_cmd)(struct mtk_hw_dev *dev, struct mtk_mcu_txblk *mcu_txblk);
	int (*fw_log_2_host)(struct mtk_hw_dev *dev, u8 ctrl, int dest);
	int (*set_wa)(struct mtk_hw_dev *dev, u32 opt);
};

#define MAC_WATCHDOG_TIME		(HZ / 10)

struct mtk_mib_status {
	u16 ack_fail_cnt;
	u16 fcs_err_cnt;
	u16 rts_cnt;
	u16 rts_retries_cnt;
	u16 ba_miss_cnt;
};

enum {
	CHANNEL_STATE_ON,
	CHANNEL_STATE_SWITCH,
	CHANNEL_STATE_MAX
};

/** struct mtk_hw_phy - Hardware physical indicate a physical band
 *
 * This struct is used to maintain hardware related information for a band
 *
 * @param band_idx: Band index of this hw_phy
 * @param wmm_sta: WMM set index of STA mode for this hw_phy
 * @param wmm_ap: WMM set index of AP mode for this hw_phy
 * @param omac_mgmt: OWN MAC manager
 * @param rept_omac_mgmt: Repeater own MAC manager
 * @param ext_omac_mgmt: MBSS own MAC manager
 * @param hw_dev: Point to related hardware device
 * @param sys_idx: Unique mtk_idr_entry to hardware WiFi system
 * @param bus_res_ctl: Point to bus resource control for this band
 * @param tk_res_ctl: Point to token resource contorl for this band
 * @param mlo_rx_lock: Spinlock for receive mlo packet per band
 * @param msdu_q: MSDU packet queue
 * @param msdu_rx_blk: Point to MSDU rx blk
 * @param mac_hw: Point to upper layer MAC driver hardware struct
 */
struct mtk_hw_phy {
	u8 band_idx;
	u8 wmm_sta;
	u8 wmm_ap;
	struct mtk_idr_mgmt omac_mgmt;
	struct mtk_idr_mgmt rept_omac_mgmt;
	struct mtk_idr_mgmt ext_omac_mgmt;
	struct mtk_hw_dev *hw_dev;
	struct mtk_idr_entry sys_idx;
	void *bus_res_ctl;
	struct mtk_tk_res_ctl *tk_res_ctl;
	spinlock_t mlo_rx_lock;
	struct sk_buff_head msdu_q;
	void *msdu_rx_blk;
	void *mac_hw;
} __aligned(NETDEV_ALIGN);

struct mtk_hw_phy_mgmt {
	struct mtk_hw_phy *phys[MAX_BAND_NUM];
	struct mtk_idr_mgmt mgmt;
};

struct mtk_hw_ctrl {
	struct mtk_tk_mgmt tk_mgmt[TK_MGMT_MAX];
	struct mtk_rx_tk_mgmt rx_tk_mgmt;
	struct mtk_idr_mgmt uwtbl_mgmt;
	struct mtk_idr_mgmt group_mgmt;
	struct mtk_idr_mgmt bss_mgmt;
	struct mtk_idr_mgmt mld_remap_mgmt;
	struct mtk_mat_tb_mgmt mat_tb_mgmt;
	struct mtk_hw_phy_mgmt phy_mgmt;
};

enum {
	MTK_OTHER_BSS,
	MTK_FCSFAIL,
	MTK_CONTROL,
	MTK_MONITOR,
};

enum {
	MTK_STA_TYPE_NORMAL,
	MTK_STA_TYPE_GROUP,
	MTK_STA_TYPE_MLD,
};

/** struct mtk_hw_sta - Hardware STA indicat a link peer station
 *
 * This struct is used to maintain hardware related information for
 * a peer link STA
 *
 * @param sta_type: Type of this station
 * @param tx_info: TX information for this station
 * @param link_wcid: Index of LWTBL for this station
 * @param mld_sta_idx: Index of MLD STA for this station
 * @param bss: Point to a hw_bss this station associated
 * @param sys_idx: Unique mtk_idr_entry to hardware WiFi system
 * @param mac_sta: Point to a upper layer MAC interface station
 */
struct mtk_hw_sta {
	u32 sta_type;
	u32 tx_info;
	u32 link_wcid;
	u32 mld_sta_idx;
	struct mtk_hw_bss *bss;
	struct mtk_idr_entry sys_idx;
	void *mac_sta;
} __aligned(NETDEV_ALIGN);

#define NUM_ACS 4

#define PHY_MODE_A			BIT(0)
#define PHY_MODE_B			BIT(1)
#define PHY_MODE_G			BIT(2)
#define PHY_MODE_GN			BIT(3)
#define PHY_MODE_AN			BIT(4)
#define PHY_MODE_AC			BIT(5)
#define PHY_MODE_AX_24G		BIT(6)
#define PHY_MODE_AX_5G		BIT(7)
#define PHY_MODE_AX_6G		BIT(8)
#define PHY_MODE_BE_24G		BIT(9)
#define PHY_MODE_BE_5G		BIT(10)
#define PHY_MODE_BE_6G		BIT(11)



enum mac80211_phy_type {
	MT_PHY_TYPE_CCK,
	MT_PHY_TYPE_OFDM,
	MT_PHY_TYPE_HT,
	MT_PHY_TYPE_HT_GF,
	MT_PHY_TYPE_VHT,
	MT_PHY_TYPE_HE_SU = 8,
	MT_PHY_TYPE_HE_EXT_SU,
	MT_PHY_TYPE_HE_TB,
	MT_PHY_TYPE_HE_MU,
};

struct mtk_rx_blk {
	u16 wcid;
	u16 ch_freq;
	u8 band_idx;
	u32 unicast:1;
	u32 tkip_mic_err:1;
	u32 cm:1;
	u32 non_ampdu:1;
	u32 fcs_err:1;
	u32 len_err:1;
	u32 decrypted:1;
	u32 ldpc:1;
	u8 stbc;
	u8 gi;
	u8 tx_rate;
	u8 tx_mode;
	u8 nss;
	u8 bw;
	u8 bssidx;
	s8 signal;
	s8 chain_signal[4];
	u8 key_id;
	u8 he_ru;
	u8 he_gi;
	u8 he_dcm;
	unsigned long reorder_time;
	u16 seqno;
	u8 tid;
	u32 flag;
};

enum {
	MAC_TX_FLAG_PROTECT = BIT(0),
	MAC_TX_FLAG_MGMT = BIT(1),
};

struct mtk_hw_txq {
	void *txq;
	void *txq_trans;
	void *tx_pkt_info;
	struct mtk_hw_sta *sta;
	spinlock_t hw_txq_lock;
} __aligned(NETDEV_ALIGN);

/** struct mtk_bss_mld_cfg - MLD config
 *
 * This struct is used to maintain information for a software MLD group
 * in a Hardware BSS struct.
 *
 * @param type: Type of MLD group
 * @param mld_group_idx: Software index of MLD group
 * @param mld_addr: MLD address of MLD group
 */
struct mtk_bss_mld_cfg {
	u32 mld_type;
	u32 mld_group_idx;
	u8 mld_addr[MAC_ADDR_LEN];
};

/** struct mtk_hw_bss - Hardware BSS indicate a BSS entity
 *
 * This struct is used to maintain hardware related information for a BSS
 *
 * @param wmm_idx: Hardware index of WMM set for this BSS
 * @param type: Type of this BSS
 * @param mld_addr_idx: Hardware MLD address idx for this BSS
 * @param fw_idx: Software index of BSS for linking to firmware BSS structure
 * @param omac_idx: Hardware own MAC index for this BSS
 * @param if_addr: MAC address of Interface (BSS)
 * @param mld_cfg: MLD configuration of this BSS
 * @param group: Point to a hw_sta for BMC packet transmit in this BSS
 * @param hw_phy: Point to a hw_phy for this BSS
 * @param mld_bss_entry: Point to a MLD bss entry if BSS related to a MLD group
 * @param sys_idx: Unique mtk_idr_entry to hardware WiFi system
 * @param mac_bss: Point to a upper layer MAC interface BSS
 */

struct mtk_hw_bss {
	u8 wmm_idx;
	u8 type;
	u32 mld_addr_idx;
	u32 fw_idx;
	u32 omac_idx;
	u8 if_addr[MAC_ADDR_LEN];
	struct mtk_bss_mld_cfg mld_cfg;
	struct mtk_hw_sta *group;
	struct mtk_hw_phy *hw_phy;
	struct mtk_mld_bss_entry *mld_bss_entry;
	struct mtk_idr_entry sys_idx;
	void *mac_bss;
} __aligned(NETDEV_ALIGN);

enum {
	MTK_IF_TYPE_AP,
	MTK_IF_TYPE_STA,
	MTK_IF_TYPE_MESH,
	MTK_IF_TYPE_ADHOC,
	MTK_IF_TYPE_MONITOR,
	MTK_IF_TYPE_WDS,
	MTK_IF_TYPE_REPT,
	MTK_IF_TYPE_MAX
};

enum {
	MTK_MLD_TYPE_NONE,
	MTK_MLD_TYPE_SINGLE,
	MTK_MLD_TYPE_MULTI,
	MTK_MLD_TYPE_MAX,
};

enum {
	HWIFI_STATE_RUNNING,
	HWIFI_STATE_SCANNING,
	HWIFI_STATE_RESTART,
	HWIFI_STATE_RESET,
	HWIFI_STATE_RESET_FAILED,
	HWIFI_STATE_REMOVED,
	HWIFI_STATE_READING_STATS,
	HWIFI_STATE_MAX,
};

enum {
	HWIFI_FLAG_HW_AMSDU,
	HWIFI_FLAG_RXD_PARSER,
	HWIFI_FLAG_MCU_TXD,
	HWIFI_FLAG_BA_OFFLOAD,
	HWIFI_FLAG_RX_OFFLOAD,
	HWIFI_FLAG_MULTI_BUS,
	HWIFI_FLAG_CHIP_OPTION,
	HWIFI_FLAG_PARSE_TX_PAYLOAD,
	HWIFI_FLAG_EFUSE_READY,
	HWIFI_FLAG_IN_CHIP_RRO,
	HWIFI_FLAG_MAX,
};

/** struct mtk_dbg_ops - Debug callbacks for chips
 *
 * This struct defined the callbacks for debug chip
 *
 * @param rxd_info: Dump rxd information
 * @param txd_info: Dump rxd information
 * @param debufs_init: Init chip specific debugfs and handler
 * @param debufs_exit: Exit chip specific debugfs
 * @param debufs_reg: Debugfs for read register
 */
struct mtk_dbg_ops {
	void (*rxd_info)(u32 *data);
	void (*txd_info)(struct mtk_hw_dev *dev, u32 *data);
	void (*dbgfs_init)(struct mtk_hw_dev *dev);
	void (*dbgfs_exit)(struct mtk_hw_dev *dev);
	int (*dbgfs_alloc)(struct mtk_hw_dev *dev);
	void (*dbgfs_free)(struct mtk_hw_dev *dev);
#ifdef CONFIG_HWIFI_DBG_ISR
	int (*show_isr_info)(struct mtk_bus_trans *trans, u8 bus_id);
#endif
	u32 debugfs_reg;
};

/** struct mtk_hw_dev - Hardware device indicate a physical card
 *
 * This struct is used to maintain per card hardware related information.
 *
 * @param sys_idx: Unique mtk_idr_entry to hardware WiFi system
 * @param state: Indicate device state
 * @param flags: Indicate hardware featrue flags
 * @param limit: Indicate limitation flags
 * @param if_dev: Point to main mac interface device
 * @param dev: Kernel device
 * @param lock: Critical protect for this device
 * @param mutex: Non-critical protect for this device
 * @param chip_drv: Point to chip driver for this device
 * @param inf_ops: Point to MAC interface ops
 * @param mcu_ctrl: MCU control for this device
 * @param hw_ctrl: Hardware control for this device
 * @param hw_ops: Hardware operation for this device
 * @param dbg_ops: Point to debug framework related to device's chip
 * @param dev_ops: Point to MAC operations for MAC interface can use
 * @param bus_trans: Point to main bus trans head
 * @param chip_opt: The index used in chip option table
 * @param intr_opt: The index used in interrupt option set
 * @param rx_path_type: The rx data flow path settings in PP and RRO
 * @param rro_bypass_type: The RRO bypass settings in MDP
 * @param mld_dest_type: The rx data flow path settings for MLD in MDP
 * @param txfreedone_path: The TxFreeDone path to host (WA or MAC)
 * @param hif_txd_ver_sdo: HIF TXD version for SDO HW path
 * @param sdb_band_sel: The index used in band selection for SDB
 */
struct mtk_hw_dev {
	struct mtk_idr_entry sys_idx;
	unsigned long state;
	unsigned long flags;
	unsigned long limit;
	void *if_dev;
	struct device *dev;
	spinlock_t lock;
	struct mutex mutex;
	struct mtk_chip_drv *chip_drv;
	struct mtk_interface_ops *inf_ops;
	struct mtk_mcu_ctrl mcu_ctrl;
	struct mtk_hw_ctrl hw_ctrl;
	struct mtk_hw_ops hw_ops;
	struct mtk_dbg_ops *dbg_ops;
	/*for mac_if layer ops*/
	const struct mtk_mac_ops *dev_ops;
	struct mtk_bus_trans *bus_trans;
	u8 band_idx_bmp_sup;
	u8 band_idx_bmp_used;
	u8 band_num_sup;
	/* For option table */
	struct {
		u8 chip_opt;
		u8 intr_opt;
		u8 irq_type;
		u8 vec_num;
		u8 rro_mode;
		u8 alloc_vec_num;
		u8 sdb_band_sel;
		bool rss_enable;
		u16 max_ba_wsize_scene_mlo;
		u16 ext_tx_tk_nums_master;
		u16 ext_tx_tk_nums_slave;
		u16 ext_rx_tk_nums_master;
		u16 ext_rx_tk_nums_slave;
		u16 ext_tx_buf_nums_master;
		u16 ext_tx_buf_nums_slave;
	};
	/* For WM/WA */
	u32 hif_txd_ver_sdo;
	u8 rx_path_type;
	u8 rro_bypass_type;
	u8 mld_dest_type;
	u8 txfreedone_path;
#ifdef CONFIG_HWIFI_MD_COEX_SUPPORT
	struct notifier_block notifier;
	u32 last_pc_value;
	u8 same_pc_cnt;
#endif
};

static inline struct mtk_hw_sta *
mtk_hwctrl_sta_entry_find(struct mtk_hw_dev *dev, int mld_id)
{
	if (mld_id > dev->hw_ctrl.group_mgmt.low)
		return mtk_idr_entry_find(&dev->hw_ctrl.group_mgmt, mld_id);
	else
		return mtk_idr_entry_find(&dev->hw_ctrl.uwtbl_mgmt, mld_id);
}

static inline struct mtk_hw_phy *
mtk_hwctrl_phy_entry_find(struct mtk_hw_dev *dev, u32 band_idx)
{
	struct mtk_hw_ctrl *hw_ctrl = &dev->hw_ctrl;
	struct mtk_hw_phy_mgmt *phy_mgmt = &hw_ctrl->phy_mgmt;
	struct mtk_hw_phy *phy;

	if (band_idx >= MAX_BAND_NUM || !phy_mgmt->phys[band_idx])
		return NULL;

	phy = rcu_dereference(phy_mgmt->phys[band_idx]);

	return phy;
}

static inline u32
hw_bss_get_band_idx(struct mtk_hw_bss *bss)
{
	return bss->hw_phy->band_idx;
}

static inline u32
hw_sta_get_band_idx(struct mtk_hw_sta *sta)
{
	if (sta && sta->bss && sta->bss->hw_phy)
		return sta->bss->hw_phy->band_idx;
	else
		return MAX_BAND_NUM;
}
#endif
