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

#ifndef __MTK_HW_OPS_H__
#define __MTK_HW_OPS_H__

#include "config.h"
#include "core.h"
#include "chips.h"
#include "mac_if.h"

struct mtk_tk_entry;

static inline int
mtk_hw_query_wa(struct mtk_hw_dev *dev, u32 ctrl)
{
	if (dev->hw_ops.query_wa)
		return dev->hw_ops.query_wa(dev, ctrl);
	return -EOPNOTSUPP;
}

static inline int
mtk_hw_get_mem(struct mtk_hw_dev *dev, u32 addr, u8 *data)
{
	if (dev->hw_ops.get_mem)
		return dev->hw_ops.get_mem(dev, addr, data);
	return -EOPNOTSUPP;
}

static inline int
mtk_hw_set_wa(struct mtk_hw_dev *dev, u32 opt)
{
	if (dev->hw_ops.set_wa)
		return dev->hw_ops.set_wa(dev, opt);
	return -EOPNOTSUPP;
}

static inline void
mtk_hw_rx_pkt(struct mtk_hw_dev *dev, struct mtk_bus_trans *trans,
	struct sk_buff *skb)
{
	if (dev->hw_ops.rx_pkt)
		dev->hw_ops.rx_pkt(dev, trans, skb);
}

static inline void
mtk_hw_rx_event(struct mtk_hw_dev *dev, struct mtk_bus_trans *trans,
	struct sk_buff *skb, u32 mcu_type)
{
	if (dev->hw_ops.rx_event)
		dev->hw_ops.rx_event(dev, trans, skb, mcu_type);
}

static inline int
mtk_hw_write_txd(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
	void *txd, void *tx_pkt_info)
{
	if (dev->hw_ops.write_txd)
		return dev->hw_ops.write_txd(dev, sta, txd, tx_pkt_info);
	return -EOPNOTSUPP;
}

static inline int
mtk_hw_write_txp(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
	void *tx_pkt_info)
{
	if (dev->hw_ops.write_txp)
		return dev->hw_ops.write_txp(dev, tk_entry, tx_pkt_info);
	return -EOPNOTSUPP;
}

static inline int
mtk_hw_write_mcu_txd(struct mtk_hw_dev *dev, struct mtk_mcu_txblk *mcu_txblk)
{
	if (mcu_txblk->uni_cmd) {
		if (dev->hw_ops.write_mcu_txd_uni_cmd)
			return dev->hw_ops.write_mcu_txd_uni_cmd(dev, mcu_txblk);
	} else {
		if (dev->hw_ops.write_mcu_txd)
			return dev->hw_ops.write_mcu_txd(dev, mcu_txblk);
	}
	return -EOPNOTSUPP;
}

static inline void
mtk_hw_tx_skb_unmap(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry)
{
	if (dev->hw_ops.skb_unmap_txp)
		dev->hw_ops.skb_unmap_txp(dev, tk_entry);
}

static inline int
mtk_hw_set_driver_own(struct mtk_hw_dev *dev, struct mtk_bus_trans *trans, bool drv_own)
{
	if (dev->chip_drv->ctl_ops->driver_own)
		return dev->chip_drv->ctl_ops->driver_own(dev, trans, drv_own);
	return -EOPNOTSUPP;
}

static inline int
mtk_hw_check_fwdl_state(struct mtk_hw_dev *dev, u32 state)
{
	if (dev->chip_drv->ctl_ops->check_fwdl_state)
		return dev->chip_drv->ctl_ops->check_fwdl_state(dev, state);
	return -EOPNOTSUPP;
}

static inline void
mtk_hw_set_filter(struct mtk_hw_dev *dev, u8 band, unsigned long flag)
{
	if (dev->chip_drv->ctl_ops->set_filter)
		dev->chip_drv->ctl_ops->set_filter(dev, band, flag);
}

static inline int
mtk_hw_init(struct mtk_hw_dev *dev)
{
	if (dev->chip_drv->ctl_ops->hw_init)
		return dev->chip_drv->ctl_ops->hw_init(dev);
	return -EOPNOTSUPP;
}

static inline int
mtk_hw_reset(struct mtk_hw_dev *dev)
{
	if (dev->chip_drv->ctl_ops->hw_reset)
		return dev->chip_drv->ctl_ops->hw_reset(dev);
	return -EOPNOTSUPP;
}

static inline int
mtk_hw_chip_reset(struct mtk_hw_dev *dev)
{
	if (dev->chip_drv->ctl_ops->hw_chip_reset)
		return dev->chip_drv->ctl_ops->hw_chip_reset(dev);
	return -EOPNOTSUPP;
}

static inline int
mtk_hw_get_pc_value(struct mtk_hw_dev *dev, u32 *value)
{
	if (dev->chip_drv->ctl_ops->get_pc_value)
		return dev->chip_drv->ctl_ops->get_pc_value(dev, value);
	return -EOPNOTSUPP;
}
#endif
