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

#ifndef __MTK_INTERFACE_H
#define __MTK_INTERFACE_H

#include <linux/netdevice.h>
#include "core.h"
#include "mac_ops.h"

enum mtk_interface_type {
	MTK_INTERFACE_MAC80211, /**< MAC80211_IF*/
	MTK_INTERFACE_CONNAC, /**< CONNAC_IF*/
	MTK_INTERFACE_NUM, /**< Max MAC interface number*/
};

/**
 * struct mtk_interface_ops - interface callback functions used for hwifi
 * core callback to interface driver.
 *
 * @param alloc_device request to alloc a new device
 * @param free_device request to free a indicate device
 * @param register_device request to register a device
 * @param unregister_device request to unregister a indicate device
 * @param rx_pkt receive frames from a specific device and band
 * @param rx_indicate_pkt receive frames from a specific device and band
 * @param tx_status receive a tx done event and response to interface driver
 * @param rx_event receive a event and response to interface driver
 * @param rx_uni_event receive a unify event and response to interface driver
 * @param add_phy request to creat a new band
 */
struct mtk_interface_ops {
	struct mtk_hw_dev *(*alloc_device)(struct mtk_chip_drv *drv,
		struct device *pdev, size_t size);
	int (*free_device)(struct mtk_hw_dev *dev);
	int (*register_device)(struct mtk_hw_dev *dev);
	int (*unregister_device)(struct mtk_hw_dev *dev);
	int (*rx_pkt)(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy,
			struct sk_buff_head *frames, struct napi_struct *napi);
	int (*rx_indicate_pkt)(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy,
		struct sk_buff_head *frames, struct napi_struct *napi);
	int (*tx_status)(struct mtk_hw_dev *dev, struct mtk_hw_sta *hw_sta,
					void *tx_sts);
	int (*rx_event)(struct mtk_hw_dev *dev, struct sk_buff *skb);
	int (*rx_uni_event)(struct mtk_hw_dev *dev, struct sk_buff *skb);
	int (*add_phy)(struct mtk_hw_dev *dev, struct mtk_hw_phy **hw_phy);
	int (*rx_ser_event)(u32 chip_id, u32 ser_level, u32 ser_event,
		u32 hw_id);
	int (*dequeue_by_tk_free)(struct mtk_hw_dev *dev,
			struct mtk_tk_entry *tk_entry);
	int (*ba_trig_event)(struct mtk_hw_dev *dev, u16 wcid, u8 tid);
	int (*chip_reset)(unsigned int chip_id);
};

/**
 * struct mtk_interface - a MAC interface information
 *
 * @param type MAC interface type
 * @param ops MAC interface callback functions see @mtk_interface_ops.
 * @param debugfs_dir MAC interface related debugfs
 */
struct mtk_interface {
	u8 type;
	struct mtk_interface_ops ops;
	struct dentry *debugfs_dir;
};

/**
 * struct mtk_interface_mgmt - MAC interface manager
 *
 * @param interfaces supported MAC interface for each type
 * @param debugfs_dir debugfs for MAC interface manager
 * @param mutex mutex lock for managing MAC interface
 */
struct mtk_interface_mgmt {
	struct mtk_interface *interfaces[MTK_INTERFACE_NUM];
	struct dentry *debugfs_dir;
	struct mutex mutex;
};

int mtk_interface_init(struct mtk_hwifi_drv *drv,
	struct mtk_interface_mgmt *if_mgmt);
void mtk_interface_exit(struct mtk_interface_mgmt *if_mgmt);
int mtk_interface_register(struct mtk_interface *inf);
void mtk_interface_unregister(struct mtk_interface *inf);
struct mtk_hw_dev *mtk_interface_alloc_device(struct device *pdev, struct mtk_chip *chip);


/*
 *
 */
static inline int
mtk_interface_rx_event(struct mtk_hw_dev *dev, struct sk_buff *skb)
{
	struct mcu_rxd *rxd = (struct mcu_rxd *)skb->data;

	if (rxd->option & MCU_UNI_CMD_OPT_BIT_1_UNI_EVENT) {
		if (dev->inf_ops->rx_uni_event)
			return dev->inf_ops->rx_uni_event(dev, skb);
		return -EOPNOTSUPP;
	} else {
		if (dev->inf_ops->rx_event)
			return dev->inf_ops->rx_event(dev, skb);
		return -EOPNOTSUPP;
	}
}

static inline int
mtk_interface_host_detect_ser_event(
	struct mtk_hw_dev *dev,
	u32 chip_id,
	u32 ser_event,
	u32 hw_id)
{
	if (dev->inf_ops->rx_ser_event)
		return dev->inf_ops->rx_ser_event(
			chip_id,
			HW_SER_LV_10_0,
			ser_event,
			hw_id);
	return -EOPNOTSUPP;
}

static inline int
mtk_interface_rx_swi_ser_event(
	struct mtk_hw_dev *dev,
	u32 chip_id,
	u32 ser_event,
	u32 hw_id)
{
	if (dev->inf_ops->rx_ser_event)
		return dev->inf_ops->rx_ser_event(
			chip_id,
			HW_SER_LV_1_0,
			ser_event,
			hw_id);
	return -EOPNOTSUPP;
}

static inline int
mtk_interface_rx_wdt_ser_event(
	struct mtk_hw_dev *dev,
	u32 chip_id,
	u32 ser_event,
	u32 hw_id)
{
	if (dev->inf_ops->rx_ser_event)
		return dev->inf_ops->rx_ser_event(
			chip_id,
			HW_SER_LV_0_5,
			ser_event,
			hw_id);
	return -EOPNOTSUPP;
}

static inline int
mtk_interface_rx_indicate(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy,
	struct sk_buff_head *frames, struct napi_struct *napi)
{
	if (dev->inf_ops->rx_indicate_pkt)
		return dev->inf_ops->rx_indicate_pkt(dev, hw_phy, frames, napi);
	return -EOPNOTSUPP;
}

/*
 *
 */
static inline int
mtk_interface_rx_data(struct mtk_hw_dev *dev, struct napi_struct *napi, u8 band_idx)
{
	struct mtk_hw_phy *hw_phy = mtk_hwctrl_phy_entry_find(dev, band_idx);

	if (!hw_phy)
		return -EINVAL;

	if (dev->inf_ops->rx_pkt)
		return dev->inf_ops->rx_pkt(dev, hw_phy, &dev->bus_trans->rx_skb, napi);
	return -EOPNOTSUPP;
}

/*
 *
 */
static inline int
mtk_interface_chip_reset(struct mtk_hw_dev *dev, unsigned int chip_id)
{
	if (dev->inf_ops->chip_reset)
		return dev->inf_ops->chip_reset(chip_id);
	return -EOPNOTSUPP;
}

/*
 *
 */
static inline int
mtk_interface_dequeue_by_free_notify(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry)
{
	if (dev->inf_ops->dequeue_by_tk_free)
		return dev->inf_ops->dequeue_by_tk_free(dev, tk_entry);
	return -EOPNOTSUPP;
}

/*
 *
 */
static inline int
mtk_interface_ba_trig_event(struct mtk_hw_dev *dev, u16 wcid, u8 tid)
{
	if (dev->inf_ops->ba_trig_event)
		return dev->inf_ops->ba_trig_event(dev, wcid, tid);
	return -EOPNOTSUPP;
}

/*
 *
 */
static inline int
mtk_interface_tx_status(struct mtk_hw_dev *dev, struct mtk_hw_sta *hw_sta,
		void *tx_sts)
{
	if (dev->inf_ops->tx_status)
		return dev->inf_ops->tx_status(dev, hw_sta, tx_sts);
	return -EOPNOTSUPP;
}

/*
 *
 */
static inline int
mtk_interface_add_phy(struct mtk_hw_dev *dev, struct mtk_hw_phy **hw_phy)
{
	if (dev->inf_ops->add_phy)
		return dev->inf_ops->add_phy(dev, hw_phy);
	return -EOPNOTSUPP;
}

/*
 *
 */
static inline int
mtk_interface_free_device(struct mtk_hw_dev *dev)
{
	if (dev->inf_ops->free_device)
		return dev->inf_ops->free_device(dev);
	return -EOPNOTSUPP;
}

/*
 *
 */
static inline int
mtk_interface_register_device(struct mtk_hw_dev *dev)
{
	int ret = 0;

	if (!dev->inf_ops->register_device)
		goto err;

	ret = dev->inf_ops->register_device(dev);
	if (ret)
		goto err;

	return 0;
err:
	return -EOPNOTSUPP;
}

/*
 *
 */
static inline int
mtk_interface_unregister_device(struct mtk_hw_dev *dev)
{
	if (dev->inf_ops->unregister_device)
		return dev->inf_ops->unregister_device(dev);

	return -EOPNOTSUPP;
}

#endif
