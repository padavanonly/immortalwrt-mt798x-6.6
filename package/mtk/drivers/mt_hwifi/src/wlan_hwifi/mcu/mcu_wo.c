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

#include <linux/kernel.h>
#include <linux/firmware.h>
#include "mcu.h"
#include "hw_ops.h"
#include "mac_if.h"

static int
mcu_wo_start(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	/*not need to set wa support v0 free notify*/
	return 0;
}

static int
mcu_wo_stop(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	return 0;
}

static int
mcu_wo_suspense(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	/*TBD*/
	return 0;
}

static int
mcu_wo_reset(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	/*TBD*/
	return 0;
}

static int
mcu_wo_fwdl_patch(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	/*wa no need to do fw patch currently*/
	return 0;
}

static int
mcu_wo_rx_prepare(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, struct sk_buff *skb,
	struct mtk_mcu_resp *rx_info)
{
	return 0;
}

static u32 wm_to_wo_cmd_wrap(struct mtk_mcu_txblk *mcu_txblk)
{
	u32 idx = WO_CMD_WED_END;
	if (mcu_txblk->uni_cmd) {
		switch (mcu_txblk->cmd) {
		case 0x01:
			idx = WO_CMD_DEV_INFO;
			break;
		case 0x02:
			idx = WO_CMD_BSS_INFO;
			break;
		case 0x03:
			idx = WO_CMD_STA_REC;
			break;
		case 0xB:
			idx = WO_CMD_FW_LOG_CTRL;
			break;
		}
	} else {
		switch (mcu_txblk->ext_cmd) {
		case 0x2a:
			idx = WO_CMD_DEV_INFO;
			break;
		case 0x25:
			idx = WO_CMD_STA_REC;
			break;
		case 0x26:
			idx = WO_CMD_BSS_INFO;
			break;
		case 0x13:
			idx = WO_CMD_FW_LOG_CTRL;
			break;
		}
	}

	return idx;
}

static int
mcu_wo_write_txd(struct mtk_hw_dev *dev, struct mtk_mcu_txblk *mcu_txblk)
{
	struct sk_buff *skb = mcu_txblk->skb;
	struct wo_txd *txd;

	txd = (struct wo_txd *)skb_push(skb, sizeof(*txd));

	txd->cmd_id = wm_to_wo_cmd_wrap(mcu_txblk);
	txd->len = mcu_txblk->len;

	return 0;
}

static int
mcu_wo_tx(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, struct mtk_mcu_txblk *mcu_txblk)
{
	int ret = -EINVAL;
	struct mtk_bus_trans *trans = dev->bus_trans;

	while (trans) {
		mcu_wo_write_txd(dev, mcu_txblk);
		ret = mtk_bus_dma_tx_mcu_queue(trans, mcu_txblk->skb, Q_TX_CMD_WO);
		if (ret)
			break;
		trans = trans->next;
	}
	return ret;
}

static int
mcu_wo_rx(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, struct mtk_mcu_rx_data *rx_data)
{
	printk("%s(): rx data %p\n", __func__, rx_data->skb);
	return 0;
}

static int
mcu_wo_init(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	struct mtk_chip_drv *drv = dev->chip_drv;

	mcu_entry->need_patch = false;
	mcu_entry->mcu_rxd_sz = sizeof(struct mcu_rxd);
	mcu_entry->rx_ignore_sz = drv->hw_caps->mac_rxd_grp_0_sz;
	return 0;
}

static int
mcu_wo_exit(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	/*do nothing, through by wm*/
	return 0;
}

const struct mtk_mcu_ops mtk_wo_ops = {
	.fwdl_patch = mcu_wo_fwdl_patch,
	.start = mcu_wo_start,
	.stop = mcu_wo_stop,
	.suspense = mcu_wo_suspense,
	.reset = mcu_wo_reset,
	.tx = mcu_wo_tx,
	.rx = mcu_wo_rx,
	.rx_prepare = mcu_wo_rx_prepare,
	.init = mcu_wo_init,
	.exit = mcu_wo_exit,
};
