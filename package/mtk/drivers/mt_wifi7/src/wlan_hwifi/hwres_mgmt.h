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

#ifndef __MTK_HRM_H__
#define __MTK_HRM_H__

#include "config.h"
#include "wsys.h"
#include "utility.h"

#define OM_REMAP_IDX_NONE 0xff

struct mtk_rx_tid {
	struct rcu_head rcu_head;
	struct mtk_hw_dev *dev;
	struct mtk_hw_phy *phy;
	spinlock_t lock;
	struct delayed_work reorder_work;
	u16 head;
	u16 size;
	u16 nframes;
	u8 num;
	u8 started:1, stopped:1, timer_pending:1;
	struct sk_buff *reorder_buf[];
};

struct mtk_ampdu_param {
	u16 tid;
	u16 ssn;
	u16 buf_size;
	bool amsdu;
	u16 timeout;
	u8 action;
};

struct mtk_hw_dev;
struct mtk_hw_sta;
struct mtk_hw_bss;


enum phy_freq_type {
	HWIFI_PHY_2G,
	HWIFI_PHY_5G,
	HWIFI_PHY_6G,
};

enum phy_cap {
	HWIFI_PHY_LEGACY = 1 << 0,
	HWIFI_PHY_N = 1 << 1,
	HWIFI_PHY_VHT = 1 << 2,
	HWIFI_PHY_HE = 1 << 3,
	HWIFI_PHY_BE = 1 << 4,
};

/** struct mtk_hwres_radio_info - Radio freqency and rate information
 *
 * @param type Indicate frequency type for this information
 * @param cap Indicate rate capabilities
 */
struct mtk_hwres_radio_info {
	enum phy_freq_type type; /*2G/5G/6G*/
	enum phy_cap cap;
};

/** struct mtk_hwres_radio_cap - Radio capabilities
 *
 * @param band_idx Hardware idx for this band
 * @param radio_info_size Size of info field
 * @param omac Range of own MAC resource
 * @param ext_omac Range of extended own MAC resource
 * @param rept_omac Range of repeater own MAC resource
 * @param wmm_set Range of WMM set
 * @param info Indicate radio frequency and rate capabilities
 * @param max_tx_tk_nums Max tx token number for this band
 */
struct mtk_hwres_radio_cap {
	u8 band_idx;
	u8 radio_info_size;
	struct mtk_range omac;
	struct mtk_range ext_omac;
	struct mtk_range rept_omac;
	struct mtk_range wmm_set;
	struct mtk_hwres_radio_info *info;
	u16 rsv_tx_tk_nums;
	u16 max_tx_tk_nums;
};

/** struct mtk_hwres_cap - Hardware resource capabilities
 *
 * @param uwtbl Range of uwtbl resource
 * @param group Range of group wtbl resource
 * @param tx_token Range of SW TX token resource
 * @param rx_token Range of RX token resource, may include HW path
 * @param mld_addr Range of MLD address table resource
 * @param link_addr Range of Link address table resource
 * @param mld_remap Range of MLD remap table resource
 * @param bss Range of FW BSS idx resource
 * @param radio_num Number of radio in this chip
 * @param tx_free_done_ver of this chip
 * @param ext_rx_tk_nums_master Number of extended rx token idx for Master
 * @param ext_rx_tk_nums_slave Number of extended rx token idx for Slave
 * @pram radio_cap Capabilities of each radio
 */
struct mtk_hwres_cap {
	struct mtk_range uwtbl;
	struct mtk_range group;
	struct mtk_range tx_token;
	struct mtk_range tx_token_bmc;
	struct mtk_range rx_token;
	struct mtk_range mld_addr;
	struct mtk_range link_addr;
	struct mtk_range mld_remap;
	struct mtk_range bss;
	u8 radio_num;
	u8 tx_free_done_ver;
	u16 rro_free_pool_tk_nums;
	u16 ext_tx_tk_nums_master;
	u16 ext_tx_tk_nums_slave;
	u16 ext_rx_tk_nums_master;
	u16 ext_rx_tk_nums_slave;
	u16 ext_tx_buf_nums_master;
	u16 ext_tx_buf_nums_slave;
	struct mtk_hwres_radio_cap *radio_cap;
};

enum MAT_MAC_ADDR_TYPE {
	MAT_ADDR_TYPE_MLD,
	MAT_ADDR_TYPE_NON_MLD
};

/** struct mtk_mat_tb_entry - Hardware MAT table entry
 *
 * @param mat_tb_idx MAT table index
 * @param addr_type Indicate address type of mac_addr
 * @param mac_addr MAC address contained in MAT table indexed by mat_tb_idx
 * @param hw_bss_idx Indicate mat_tb_idx is assigned to which hw_bss_idx
 * @param mld_group_idx Indicate mat_tb_idx is assigned to which mld_group_idx
 */
struct mtk_mat_tb_entry {
	u8 mat_tb_idx;
	u8 addr_type;
	u8 mac_addr[MAC_ADDR_LEN];
	union {
		u8 hw_bss_idx;
		u8 mld_group_idx;
	};
};

/** struct mtk_mat_tb_mgmt - Hardware MAT table manager
 *
 * @param mld_addr_mgmt Manager for MLD MAC address on MAT table
 * @param link_addr_mgmt Manager for Link MAC address on MAT table
 * @param entries MAT table entries
 */
struct mtk_mat_tb_mgmt {
	struct mtk_idr_mgmt mld_addr_mgmt;
	struct mtk_idr_mgmt link_addr_mgmt;
	struct mtk_mat_tb_entry *entries;
};

int
mtk_hwctrl_mat_tb_alloc(struct mtk_hw_dev *dev, u8 *mac_addr, u8 addr_type, u32 *mat_idx);

int
mtk_hwctrl_mat_tb_free(struct mtk_hw_dev *dev, u32 mat_idx);

int
mtk_hwctrl_om_remap_alloc(struct mtk_hw_dev *dev, struct mtk_mld_bss_entry *mld_bss_entry);

int
mtk_hwctrl_om_remap_free(struct mtk_hw_dev *dev, struct mtk_mld_bss_entry *mld_bss_entry);

int
mtk_hwctrl_sta_free(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta);

int
mtk_hwctrl_sta_alloc(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta, u8 sta_type);

int
mtk_hwctrl_bss_alloc(struct mtk_hw_dev *dev, struct mtk_hw_bss *hw_bss);

int
mtk_hwctrl_bss_free(struct mtk_hw_dev *dev, struct mtk_hw_bss *hw_bss);

int
mtk_hwctrl_phy_alloc(struct mtk_hw_dev *dev, struct mtk_hw_phy *phy);

int
mtk_hwctrl_phy_free(struct mtk_hw_dev *dev, struct mtk_hw_phy *phy);

int
mtk_hwctrl_init_device(struct mtk_hw_dev *dev);

void
mtk_hwctrl_exit_device(struct mtk_hw_dev *dev);


#endif
