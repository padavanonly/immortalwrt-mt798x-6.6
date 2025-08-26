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

#ifndef __MTK_MAC_OPS_H
#define __MTK_MAC_OPS_H

#include <linux/netdevice.h>
#include "core.h"


struct mtk_tx_cb {
	unsigned long jiffies;
	u16 mld_id;
	u8 pktid;
	u8 flags;
};

/**
 * struct mtk_mac_ops - MAC operations
 *
 * This struct provide handlers for interface driver can control and manage
 *  hardware ralated operations
 *
 * @param start starting a specific hw_dev
 * @param stop stop a specific hw_dev
 * @param add_interface add a new hw_bss for a specific hw_phy
 * @param remove_interface remove a specific hw_bss
 * @param add_mld add a new mld_bss_entry for a MLD group
 * @param remove_mld remove a new mld_bss_entry for a MLD group
 * @param mld_add_link add a hw_bss to a mld_bss_entry (MLD group)
 * @param mld_remove_link remove a hw_bss from a mld_bss_entry (MLD group)
 * @param add_sta add a new hw_sta for a hw_bss
 * @param remove_sta remove a specific hw_sta
 * @param tx_check_resource check a specific band or txq resource
 * @param tx_data tx frames for a specific txq
 * @param tx_kick real kick data to hardware
 * @param add_phy add a new hw_phy
 * @param remove_phy remove a specific hw_phy
 * @param init_txq indicate a txq to real hw txq
 * @param rx_get_phy get hw_phy for rx packet from rxd
 * @param update_interface runtime update hw_bss information for dynamic dbdc
 * @param muc_tx tx mcu cmd to specific mcu entry.
 * @param set_l05reset do system l05reset.
 * @param update_idrm runtime update idrm. low/high
 * @param set_fw_mode set fw mode config to use test mode fw or normal mode fw.
 * @param get_free_sta_pool_num get remaining free sta number from sta_mgmt of wsys.
 * @param change_setup_link_sta change setup link sta.
 */
struct mtk_mac_ops {
	int (*start)(struct mtk_hw_dev *dev);
	int (*stop)(struct mtk_hw_dev *dev);
	int (*add_interface)(struct mtk_hw_phy *phy, struct mtk_hw_bss *hw_bss,
		u8 *if_addr, struct mtk_bss_mld_cfg *mld_cfg);
	int (*remove_interface)(struct mtk_hw_phy *phy,
		struct mtk_hw_bss *hw_bss);
	int (*add_mld)(struct mtk_hw_dev *dev, struct mtk_bss_mld_cfg *mld_cfg);
	int (*remove_mld)(struct mtk_hw_dev *dev, u32 mld_group_idx);
	int (*mld_add_link)(struct mtk_hw_dev *dev, struct mtk_hw_bss *hw_bss, u32 mld_group_idx);
	int (*mld_remove_link)(struct mtk_hw_dev *dev, struct mtk_hw_bss *hw_bss);
	int (*add_sta)(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss,
		struct mtk_hw_sta *sta, u8 sta_type, u32 mld_sta_idx);
	int (*remove_sta)(struct mtk_hw_dev *dev,
		struct mtk_hw_bss *bss, struct mtk_hw_sta *sta);
	int (*tx_check_resource)(struct mtk_hw_phy *phy, struct mtk_hw_txq *txq);
	int (*tx_data)(struct mtk_hw_phy *phy, struct mtk_hw_txq *txq);
	int (*tx_kick)(struct mtk_hw_dev *dev);
	int (*add_phy)(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy);
	int (*remove_phy)(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy);
	int (*init_txq)(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss,
		struct mtk_hw_sta *sta, u8 tid, struct mtk_hw_txq *txq);
	struct mtk_hw_phy *(*rx_get_phy)(struct mtk_hw_dev *dev, u32 *band_idx,
						u32 *mld_idx, u32 *mld_link_idx, u8 *mld_link);
	int (*update_interface)(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss);
	/*cmd*/
	int (*mcu_tx)(struct mtk_hw_dev *dev, struct mtk_mcu_txblk *mcu_txblk);
	int (*set_dma_tk)(struct mtk_hw_phy *phy, bool enable);
	int (*set_dma_sw_rxq)(struct mtk_hw_phy *phy, bool enable);
	int (*hw_reset)(struct mtk_hw_dev *dev);
	int (*set_traffic)(struct mtk_hw_dev *dev, bool enable);
	int (*get_pc_value)(struct mtk_hw_dev *dev, u32 *pc_value);
	int (*get_idrm_high)(struct mtk_hw_dev *dev, struct mtk_idr_mgmt *idrm);
	int (*get_idrm_low)(struct mtk_hw_dev *dev, struct mtk_idr_mgmt *idrm);
	int (*update_idrm)(struct mtk_hw_dev *dev, struct mtk_idr_mgmt *idrm, u32 low, u32 high);
	struct mtk_hw_bss *(*get_hw_bss)(struct mtk_hw_dev *dev, u8 bss_idx);
	void (*get_fw_info)(struct mtk_hw_dev *dev, int mcu_type, char *fw_ver, char *build_date,
				char *fw_ver_long);
	void (*set_fw_mode)(struct mtk_hw_dev *dev, int mcu_type, unsigned char fw_mode);
	u32 (*get_free_sta_pool_num)(struct mtk_hw_dev *dev);
	int (*change_setup_link_sta)(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta);
	int (*get_mld_id)(struct mtk_hw_sta *hw_sta, u32 *pri_id, u32 *sec_id);
};

int
mtk_hdev_ops_init(struct mtk_hw_dev *dev);

static inline int
mtk_hdev_start(struct mtk_hw_dev *dev)
{
	if (dev->dev_ops->start)
		return dev->dev_ops->start(dev);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_stop(struct mtk_hw_dev *dev)
{
	if (dev->dev_ops->stop)
		return dev->dev_ops->stop(dev);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_add_interface(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy,
	struct mtk_hw_bss *bss, u8 *if_addr, struct mtk_bss_mld_cfg *mld_cfg)
{
	if (dev->dev_ops->add_interface)
		return dev->dev_ops->add_interface(hw_phy, bss, if_addr, mld_cfg);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_remove_interface(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy, struct mtk_hw_bss *bss)
{
	if (dev->dev_ops->remove_interface)
		return dev->dev_ops->remove_interface(hw_phy, bss);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_update_interface(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss)
{
	if (dev->dev_ops->update_interface)
		return dev->dev_ops->update_interface(dev, bss);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_add_mld(struct mtk_hw_dev *dev, struct mtk_bss_mld_cfg *mld_cfg)
{
	if (dev->dev_ops->add_mld)
		return dev->dev_ops->add_mld(dev, mld_cfg);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_remove_mld(struct mtk_hw_dev *dev, u32 mld_group_idx)
{
	if (dev->dev_ops->remove_mld)
		return dev->dev_ops->remove_mld(dev, mld_group_idx);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_mld_add_link(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss, u32 mld_group_idx)
{
	if (dev->dev_ops->mld_add_link)
		return dev->dev_ops->mld_add_link(dev, bss, mld_group_idx);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_mld_remove_link(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss)
{
	if (dev->dev_ops->mld_remove_link)
		return dev->dev_ops->mld_remove_link(dev, bss);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_remove_sta(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss, struct mtk_hw_sta *sta)
{
	if (dev->dev_ops->remove_sta)
		return dev->dev_ops->remove_sta(dev, bss, sta);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_add_sta(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss,
	struct mtk_hw_sta *sta, u8 sta_type, u32 mld_sta_idx)
{
	if (dev->dev_ops->add_sta)
		return dev->dev_ops->add_sta(dev, bss, sta, sta_type, mld_sta_idx);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_init_txq(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss, struct mtk_hw_sta *sta, u8 tid, struct mtk_hw_txq *txq)
{
	if (dev->dev_ops->init_txq)
		return dev->dev_ops->init_txq(dev, bss, sta, tid, txq);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_tx_check_resource(struct mtk_hw_phy *phy, struct mtk_hw_txq *txq)
{
	struct mtk_hw_dev *dev = phy->hw_dev;

	if (dev->dev_ops->tx_check_resource)
		return dev->dev_ops->tx_check_resource(phy, txq);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_tx_data(struct mtk_hw_phy *phy, struct mtk_hw_txq *txq)
{
	struct mtk_hw_dev *dev = phy->hw_dev;

	if (dev->dev_ops->tx_data)
		return dev->dev_ops->tx_data(phy, txq);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_tx_kick(struct mtk_hw_dev *dev)
{
	if (dev->dev_ops->tx_kick)
		return dev->dev_ops->tx_kick(dev);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_tx_cmd(struct mtk_hw_dev *dev, struct mtk_mcu_txblk *mcu_txblk)
{
	if (dev->dev_ops->mcu_tx)
		return dev->dev_ops->mcu_tx(dev, mcu_txblk);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_add_phy(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy)
{
	if (dev->dev_ops->add_phy)
		return dev->dev_ops->add_phy(dev, hw_phy);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_remove_phy(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy)
{
	if (dev->dev_ops->remove_phy)
		return dev->dev_ops->remove_phy(dev, hw_phy);
	return -EOPNOTSUPP;
}

static inline struct mtk_hw_phy *
mtk_hdev_rx_get_phy(struct mtk_hw_dev *dev, u32 *band_idx,
				u32 *mld_idx, u32 *mld_link_idx, u8 *mld_link)
{
	if (dev->dev_ops->rx_get_phy)
		return dev->dev_ops->rx_get_phy(dev, band_idx, mld_idx, mld_link_idx, mld_link);
	return NULL;
}

static inline int
mtk_hdev_set_dma_tk(struct mtk_hw_dev *dev,
	struct mtk_hw_phy *hw_phy, bool enable)
{
	if (dev->dev_ops->set_dma_tk)
		return dev->dev_ops->set_dma_tk(hw_phy, enable);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_set_dma_sw_rxq(struct mtk_hw_dev *dev,
	struct mtk_hw_phy *hw_phy, bool enable)
{
	if (dev->dev_ops->set_dma_sw_rxq)
		return dev->dev_ops->set_dma_sw_rxq(hw_phy, enable);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_hw_reset(struct mtk_hw_dev *dev)
{
	if (dev->dev_ops->hw_reset)
		return dev->dev_ops->hw_reset(dev);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_set_traffic(struct mtk_hw_dev *dev, bool enable)
{
	if (dev->dev_ops->set_traffic)
		return dev->dev_ops->set_traffic(dev, enable);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_get_pc_value(struct mtk_hw_dev *dev, u32 *pc_value)
{
	if (dev->dev_ops->get_pc_value)
		return dev->dev_ops->get_pc_value(dev, pc_value);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_get_idrm_high(struct mtk_hw_dev *dev, struct mtk_idr_mgmt *idrm)
{
	if (dev->dev_ops->get_idrm_high)
		return dev->dev_ops->get_idrm_high(dev, idrm);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_get_idrm_low(struct mtk_hw_dev *dev, struct mtk_idr_mgmt *idrm)
{
	if (dev->dev_ops->get_idrm_low)
		return dev->dev_ops->get_idrm_low(dev, idrm);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_update_idrm(struct mtk_hw_dev *dev, struct mtk_idr_mgmt *idrm, u32 low, u32 high)
{
	if (dev->dev_ops->update_idrm)
		return dev->dev_ops->update_idrm(dev, idrm, low, high);
	return -EOPNOTSUPP;
}

static inline struct mtk_hw_bss *
mtk_hdev_get_hw_bss(struct mtk_hw_dev *dev, u8 bss_idx)
{
	if (dev->dev_ops->get_hw_bss)
		return dev->dev_ops->get_hw_bss(dev, bss_idx);
	return NULL;
}

static inline void
mtk_hdev_get_fw_info(struct mtk_hw_dev *dev, int mcu_type, char *fw_ver, char *build_date,
			char *fw_ver_long)
{
	if (dev->dev_ops->get_fw_info)
		dev->dev_ops->get_fw_info(dev, mcu_type, fw_ver, build_date, fw_ver_long);
}


static inline void
mtk_hdev_set_fw_mode(struct mtk_hw_dev *dev, int mcu_type, u8 fw_mode)
{
	if (dev->dev_ops->set_fw_mode)
		dev->dev_ops->set_fw_mode(dev, mcu_type, fw_mode);
}

static inline u32
mtk_hdev_get_free_sta_pool_num(struct mtk_hw_dev *dev)
{
	if (dev->dev_ops->get_free_sta_pool_num)
		return dev->dev_ops->get_free_sta_pool_num(dev);

	return 0;
}

static inline int
mtk_hdev_change_setup_link_sta(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta)
{
	if (dev->dev_ops->change_setup_link_sta)
		return dev->dev_ops->change_setup_link_sta(dev, sta);
	return -EOPNOTSUPP;
}

static inline int
mtk_hdev_get_mld_id(struct mtk_hw_dev *dev, struct mtk_hw_sta *hw_sta, u32 *pri_id, u32 *sec_id)
{
	if (dev->dev_ops->get_mld_id)
		return dev->dev_ops->get_mld_id(hw_sta, pri_id, sec_id);

	return 0;
}

int mtk_ge_add_phy(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy);
int mtk_ge_remove_phy(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy);
int mtk_ge_tx_check_resource(struct mtk_hw_phy *phy, struct mtk_hw_txq *txq);
int mtk_ge_tx_data(struct mtk_hw_phy *phy, struct mtk_hw_txq *txq);
int mtk_ge_set_dma_tk(struct mtk_hw_phy *phy, bool enable);
int mtk_ge_set_dma_sw_rxq(struct mtk_hw_phy *phy, bool enable);
int mtk_ge_hw_reset(struct mtk_hw_dev *dev);
int mtk_ge_set_traffic(struct mtk_hw_dev *dev, bool enable);
int mtk_ge_get_pc_value(struct mtk_hw_dev *dev, u32 *pc_value);
struct mtk_hw_bss *mtk_ge_get_hw_bss(struct mtk_hw_dev *dev, u8 bss_idx);
#endif
