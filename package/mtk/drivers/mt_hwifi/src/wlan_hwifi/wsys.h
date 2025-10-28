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

#ifndef __MTK_WSYS_H__
#define __MTK_WSYS_H__

#include "utility.h"

struct mtk_dbg_ops;
struct mtk_hwifi_drv;
struct mtk_hw_dev;

struct mtk_hw_bss;
struct mtk_hw_sta;
struct mtk_hw_phy;

struct mtk_bss_mld_cfg;

#define MAC_ADDR_LEN 6
#define MLD_GROUP_NONE 0xff
#define MLD_STA_NONE 0xffff
#define MLD_LINK_MAX 3

/** struct mtk_mld_bss_entry - MLD bss group entry
 *
 * This struct used to maintain a MLD group information
 *
 * @param mld_type: Indicate MLD is single-link or multi-link
 * @param ref_cnt: Reference count
 * @param remap_id: Hardware idx for remap table
 * @param mat_idx: Hardware idx for MAT table
 * @param mld_addr: MAC address of this MLD group
 * @param mutux: Lock protect to maintain mld_bss_entry
 * @param mld_group_idx: Unique mtk_idr_entry to hardware WiFi system.
 * @param mld_bss: Aarry of point for all BSS join this MLD group
 */
struct mtk_mld_bss_entry {
	u8 mld_type;
	u8 ref_cnt;
	u32 remap_id;
	u32 mat_idx;
	u8 mld_addr[MAC_ADDR_LEN];
	struct mutex mutex;
	struct mtk_idr_entry mld_group_idx;
	struct mtk_hw_bss *mld_bss[MLD_LINK_MAX];
};

/** struct mtk_mld_sta_entry - MLD STA entry
 *
 * This struct used to maintain a MLD STA information
 *
 * @param ref_cnt: Reference count
 * @param links: Index array to maintain per link LWTBL for this MLD STA
 * @param rx_pkt_cnt: Receive packet counter per link
 * @param primary: Index for primary UWTBL
 * @param secondary: Index for secondary UWTBL
 * @param setup_wcid: LWTBL of setup link
 * @param setup_band: band of setup link
 * @param mld_sta_idr: Unique mtk_idr_entry to hardware WiFi system
 * @param mld_bss: Point to MLD BSS group entry
 * @param link_sta: Array of point for all Link STA in this MLD STA
 */
struct mtk_mld_sta_entry {
	u8 ref_cnt;
	u32 links[MLD_LINK_MAX];
	u32 rx_pkt_cnt[MLD_LINK_MAX];
	struct mtk_hw_sta *primary;
	struct mtk_hw_sta *secondary;
	u32 setup_wcid;
	u8 setup_band;
	struct mtk_idr_entry *sys_idx;
	struct mtk_mld_bss_entry *mld_bss;
	struct mtk_hw_sta *link_sta[MLD_LINK_MAX];
};

struct mtk_mld_sta_entry_ctrl {
	struct mutex lock;
	struct mtk_idr_entry sys_idx;
};

/*global system control*/
struct mtk_wsys_bss_mgmt {
	struct mtk_idr_mgmt base;
};

struct mtk_wsys_sta_mgmt {
	struct mtk_idr_mgmt base;
};

struct mtk_wsys_dev_mgmt {
	struct mtk_idr_mgmt base;
};

struct mtk_wsys_phy_mgmt {
	struct mtk_idr_mgmt base;
};

struct mtk_wsys_mld_bss_mgmt {
	struct mtk_idr_mgmt base;
	struct mtk_mld_bss_entry entries[WSYS_MLD_BSS_MAX];
};

struct mtk_wsys_mld_sta_mgmt {
	struct mtk_idr_mgmt base;
	struct mtk_mld_sta_entry_ctrl ctrl[WSYS_MLD_STA_MAX];
};

/** struct mtk_wsys_mgmt - Hardware WiFi system manager
 *
 * This struct is global used for managed Hardware WiFi system
 *
 * @param debugfs_dir: Root of debugfs forHardware WiFi system
 * @param dev_mgmt: Manager for hw_dev in hwifi_drv
 * @param bss_mgmt: Manager for hw_bss in hwifi_drv
 * @param sta_mgmt: Manager for hw_sta in hwifi_drv
 * @param phy_mgmt: Manager for hw_phy in hwifi_drv
 * @param mld_bss_mgmt: Manager for MLD group in hwifi_drv
 * @param mld_sta_mgmt: Manager for MLT STA in hwifi_drv
 */
struct mtk_wsys_mgmt {
	struct dentry *debugfs_dir;
	struct mtk_wsys_dev_mgmt dev_mgmt;
	struct mtk_wsys_bss_mgmt bss_mgmt;
	struct mtk_wsys_sta_mgmt sta_mgmt;
	struct mtk_wsys_phy_mgmt phy_mgmt;
	struct mtk_wsys_mld_bss_mgmt mld_bss_mgmt;
	struct mtk_wsys_mld_sta_mgmt mld_sta_mgmt;
};

int mtk_wsys_init(struct mtk_hwifi_drv *drv, struct mtk_wsys_mgmt *wsys);
void mtk_wsys_exit(struct mtk_wsys_mgmt *wsys);
int mtk_wsys_register_device(struct mtk_hw_dev *dev);
void mtk_wsys_unregister_device(struct mtk_hw_dev *dev);
int mtk_wsys_register_bss(struct mtk_hw_bss *hw_bss);
void mtk_wsys_unregister_bss(struct mtk_hw_bss *hw_bss);
int mtk_wsys_register_sta(struct mtk_hw_sta *hw_sta);
void mtk_wsys_unregister_sta(struct mtk_hw_sta *hw_sta);
int mtk_wsys_register_phy(struct mtk_hw_phy *hw_phy);
void mtk_wsys_unregister_phy(struct mtk_hw_phy *hw_phy);
int mtk_wsys_dev_debugfs_plugin(char *name, struct mtk_dbg_ops *ops);
void mtk_wsys_dev_debugfs_unplug(char *name);
int mtk_wsys_register_mld_bss_entry(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss);
int mtk_wsys_unregister_mld_bss_entry(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss);
int mtk_wsys_update_mld_bss_entry(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss);
int mtk_wsys_register_mld(struct mtk_hw_dev *dev, struct mtk_bss_mld_cfg *mld_cfg);
int mtk_wsys_unregister_mld(struct mtk_hw_dev *dev, u32 mld_group_idx);
int mtk_wsys_mld_register_bss(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss, u32 mld_group_idx);
int mtk_wsys_mld_unregister_bss(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss);
struct mtk_mld_sta_entry *mtk_wsys_mld_sta_entry_get(u32 id);
int mtk_wsys_mld_sta_entry_mld_id_get(struct mtk_hw_sta *hw_sta, u32 *pri_id, u32 *sec_id);
int mtk_wsys_mld_sta_entry_change_setup_link(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta);
int mtk_wsys_register_mld_sta_entry(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta);
int mtk_wsys_unregister_mld_sta_entry(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta);
struct mtk_hw_bss *mtk_wsys_bss_get(struct mtk_hw_dev *dev, u8 bss_idx);
u32 mtk_wsys_free_uwtbl_num_get(struct mtk_hw_dev *dev);

static inline void
dbg_mld_sta_rx_increase(struct mtk_mld_sta_entry *mld_sta, u32 link_idx)
{
	mld_sta->rx_pkt_cnt[link_idx]++;
}


#endif /*__MTK_WSYS_H__*/
