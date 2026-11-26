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
#include "core.h"

static inline void
mcu_state_trans(struct mtk_mcu_entry *mcu_entry, enum mtk_mcu_state state)
{
	mutex_lock(&mcu_entry->mutex);
	mcu_entry->state = state;
	mutex_unlock(&mcu_entry->mutex);
}

static struct sk_buff *
mcu_get_response(struct mtk_hw_dev *dev,
	struct mtk_mcu_entry *mcu_entry, unsigned long expires)
{
	unsigned long timeout;

	if (!time_is_after_jiffies(expires))
		return NULL;

	timeout = expires - jiffies;
	wait_event_timeout(mcu_entry->wait,
			   (!skb_queue_empty(&mcu_entry->res_q)),
			   timeout);
	return skb_dequeue(&mcu_entry->res_q);
}

static int
mcu_write_back_resp(struct mtk_mcu_resp *rx_info, struct mtk_mcu_resp *resp)
{
	if (resp && resp->rx_data) {
		resp->rx_len = rx_info->rx_len;
		resp->seq = rx_info->seq;
		memcpy(resp->rx_data, rx_info->rx_data, rx_info->rx_len);
	}
	return 0;
}

static int
mcu_parse_response(struct mtk_hw_dev *dev,
	struct mtk_mcu_entry *mcu_entry, struct mtk_mcu_txblk *mcu_txblk, struct sk_buff *skb)
{
	int ret;
	struct mtk_mcu_resp rx_info = {0};
	struct uni_event_cmd_result *result = NULL;

	if (!mcu_entry->ops->rx_prepare)
		return -EOPNOTSUPP;

	/*handle specific mcu header*/
	ret = mcu_entry->ops->rx_prepare(dev, mcu_entry, skb, &rx_info);
	if (ret)
		return ret;

	mtk_dbg(MTK_MCU, "%s:get seq=%d cmd's event, exp_seq=%d\n",
		__func__, rx_info.seq, mcu_txblk->seq);

	if (mcu_txblk->seq != rx_info.seq)
		return -EAGAIN;

	result = (struct uni_event_cmd_result *)rx_info.rx_data;
	if (result &&
		mcu_txblk->uni_cmd &&
		mcu_txblk->action == MCU_ACT_SET &&
		mcu_txblk->ack &&
		mcu_txblk->cmd != result->cid)
		return -EAGAIN;

	if (mcu_write_back_resp(&rx_info, mcu_txblk->resp))
		return -EINVAL;

	return ret;
}

static int
mcu_wait_response(struct mtk_hw_dev *dev,
	struct mtk_mcu_entry *mcu_entry, struct mtk_mcu_txblk *mcu_txblk)
{
	unsigned long expires = jiffies + 20 * HZ;
	struct sk_buff *skb;
	int ret = 0;

	mtk_dbg(MTK_MCU, "%s:wait seq=%d cmd's event\n",
		__func__, mcu_txblk->seq);

	if (mcu_txblk->timeout != 0)
		expires = jiffies + mcu_txblk->timeout * HZ;

	while (true) {
		skb = mcu_get_response(dev, mcu_entry, expires);
		if (!skb) {
			mtk_dbg(MTK_MCU, "%s:cmd=%d (seq %d) timeout\n",
				__func__, mcu_txblk->cmd, mcu_txblk->seq);
			return -ETIMEDOUT;
		}

		ret = mcu_parse_response(dev, mcu_entry, mcu_txblk, skb);
		dev_kfree_skb(skb);

		if (ret != -EAGAIN)
			break;
	}

	return ret;
}

static struct sk_buff *
_mcu_msg_alloc(const void *data, int head_len,
		   int data_len, int tail_len)
{
	int length = head_len + data_len + tail_len;
	struct sk_buff *skb;

	skb = dev_alloc_skb(length);
	if (!skb)
		return NULL;

	memset(skb->head, 0, length);
	skb_reserve(skb, head_len);

	if (data && data_len) {
		void *tmp = skb_put(skb, data_len);

		memcpy(tmp, data, data_len);
	}
	return skb;
}

static int
_mcu_msg_send(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, struct mtk_mcu_txblk *mcu_txblk)
{
	if (mcu_txblk->frag_total_num == 0 ||
		(mcu_txblk->frag_total_num > 0 && mcu_txblk->frag_num == 0)) {
		mcu_txblk->seq = ++mcu_entry->msg_seq & 0xf;
		if (!mcu_txblk->seq)
			mcu_txblk->seq = ++mcu_entry->msg_seq & 0xf;
	} else {
		mcu_txblk->seq = mcu_entry->msg_seq;
	}

	return mcu_entry->ops->tx(dev, mcu_entry, mcu_txblk);
}

static int
mcu_send_message(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, struct mtk_mcu_txblk *mcu_txblk)
{
	int ret;

	mutex_lock(&mcu_entry->mutex);
	ret = _mcu_msg_send(dev, mcu_entry, mcu_txblk);
	if (ret) {
		dev_kfree_skb(mcu_txblk->skb);
		mcu_txblk->skb = NULL;
		goto out;
	}

	if (mcu_txblk->wait_resp)
		ret = mcu_wait_response(dev, mcu_entry, mcu_txblk);
out:
	mutex_unlock(&mcu_entry->mutex);
	return ret;
}

#define mcu_msg_alloc(_data, _len) (\
	_mcu_msg_alloc(_data, sizeof(struct mcu_txd), _len, 0))

static struct mtk_mcu_entry *
mcu_get_entry_by_path(struct mtk_hw_dev *dev, int path)
{
	u32 mcu_type = MCU_MAX;

	switch (path) {
	case MCU_PATH_WM:
		mcu_type = MCU_WM;
		break;
	case MCU_PATH_WA:
		mcu_type = (dev->mcu_ctrl.mcu_support & BIT(MCU_WA)) ?
			MCU_WA : MCU_WM;
		break;
	case MCU_PATH_WO:
		mcu_type = MCU_WO;
		break;
	case MCU_PATH_DSP:
		mcu_type = MCU_DSP;
		break;
	}

	return mcu_get_entry_by_type(&dev->mcu_ctrl, mcu_type);
}

static void
_mcu_rx_event(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, struct sk_buff *skb)
{
	skb_queue_tail(&mcu_entry->res_q, skb);
	wake_up(&mcu_entry->wait);
}

int
mtk_mcu_rx_event(struct mtk_hw_dev *dev, struct sk_buff *skb, u32 mcu_type)
{
	int ret;
	struct mtk_mcu_entry *mcu_entry = mcu_get_entry_by_type(&dev->mcu_ctrl, mcu_type);
	struct mtk_mcu_rx_data rx_data = {0};

	if (!mcu_entry || mcu_entry->state == MCU_STATE_NONE)
		return -EINVAL;

	skb_pull(skb, mcu_entry->rx_ignore_sz);
	rx_data.skb = skb;


	/* unsolicited event */
	ret = mcu_entry->ops->rx(dev, mcu_entry, &rx_data);

	/* solicited event: rx event send to wait queue */
	if (ret)
		_mcu_rx_event(dev, mcu_entry, skb);
	return 0;
}

static int
mcu_get_mcu_path(struct mtk_hw_dev *dev, bool fwdl, u8 dest, u8 *mcu_path)
{
	struct mtk_chip_drv *chip_drv = dev->chip_drv;

	if (!chip_drv)
		return -EINVAL;
	if (chip_drv->dest_2_path) {
		*mcu_path = fwdl ? MCU_PATH_WM : chip_drv->dest_2_path[dest].path;
		return 0;
	} else
		return -EOPNOTSUPP;
}

int
mtk_mcu_tx_nocheck(struct mtk_hw_dev *dev, struct mtk_mcu_txblk *mcu_txblk)
{
	int ret = 0;
	struct mtk_mcu_entry *mcu_entry = NULL;
	struct sk_buff *skb = NULL;

	ret = mcu_get_mcu_path(dev, mcu_txblk->fwdl, mcu_txblk->dest, &mcu_txblk->path);
	if (ret)
		return ret;

	mcu_entry = mcu_get_entry_by_path(dev, mcu_txblk->path);
	if (!mcu_entry)
		return -EINVAL;

	skb = mcu_msg_alloc(mcu_txblk->data, mcu_txblk->len);
	if (!skb)
		return -ENOMEM;

	/*update txblk*/
	mcu_txblk->skb = skb;
	return mcu_send_message(dev, mcu_entry, mcu_txblk);
}

static int
mtk_mcu_tx_wo(struct mtk_hw_dev *dev, struct mtk_mcu_txblk *mcu_txblk)
{
	u32 dest = mcu_txblk->dest;
	bool wait = mcu_txblk->wait_resp;

	if (mcu_txblk->uni_cmd) {
		switch (mcu_txblk->cmd) {
		case 0x01: /* UNI_CMD_ID_DEVINFO */
		case 0x02: /* UNI_CMD_ID_BSSINFO */
		case 0x03: /* UNI_CMD_ID_STAREC_INFO */
		case 0x0B: /* UNI_CMD_ID_WSYS_CONFIG - LOG_TO_HOST */
			mcu_txblk->dest = MCU_DEST_WO;
			mcu_txblk->wait_resp = false;
			mtk_mcu_tx_nocheck(dev, mcu_txblk);
			if (mcu_txblk->skb)
				dev_kfree_skb(mcu_txblk->skb);
		}
	} else {
		switch (mcu_txblk->ext_cmd) {
		case 0x2a:
		case 0x25:
		case 0x26:
		case 0x13:
			mcu_txblk->dest = MCU_DEST_WO;
			mcu_txblk->wait_resp = false;
			mtk_mcu_tx_nocheck(dev, mcu_txblk);
			if (mcu_txblk->skb)
				dev_kfree_skb(mcu_txblk->skb);
		}
	}
	mcu_txblk->skb = NULL;
	mcu_txblk->dest = dest;
	mcu_txblk->wait_resp = wait;
	return 0;
}


int
mtk_mcu_tx(struct mtk_hw_dev *dev, struct mtk_mcu_txblk *mcu_txblk)
{
	int ret = 0;
	struct mtk_mcu_entry *mcu_entry = NULL;

	ret = mcu_get_mcu_path(dev, mcu_txblk->fwdl, mcu_txblk->dest, &mcu_txblk->path);
	if (ret)
		return ret;

	mcu_entry = mcu_get_entry_by_path(dev, mcu_txblk->path);
	if (mcu_entry->state != MCU_STATE_START)
		return -EINVAL;

	if (dev->mcu_ctrl.mcu_support & BIT(MCU_WO))
		mtk_mcu_tx_wo(dev, mcu_txblk);

	return mtk_mcu_tx_nocheck(dev, mcu_txblk);
}

static int
mcu_entry_init(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, enum mtk_mcu_type type)
{
	const struct mtk_mcu_ops *mcu_ops = NULL;
	struct mtk_chip_mcu_info *mcu_info =
		&dev->chip_drv->hw_caps->mcu_infos[type];
	int ret;
	unsigned int srclen;

	/*check if need to restart mcu*/
	if (mcu_entry->state != MCU_STATE_NONE) {
		dev_err(dev->dev, "mcu %s not at initial state\n", mcu_entry->name);
		return -EINVAL;
	}

	memset(mcu_entry, 0, sizeof(*mcu_entry));

	switch (type) {
	case MCU_WM:
		mcu_ops = &mtk_wm_ops;
		srclen = sizeof("wm");
		strncpy(mcu_entry->name, "wm", srclen);
		break;
	case MCU_WA:
		mcu_ops = &mtk_wa_ops;
		srclen = sizeof("wa");
		strncpy(mcu_entry->name, "wa", srclen);
		break;
	case MCU_WO:
		mcu_ops = &mtk_wo_ops;
		srclen = sizeof("wo");
		strncpy(mcu_entry->name, "wo", srclen);
		break;
	case MCU_DSP:
		mcu_ops = &mtk_dsp_ops;
		srclen = sizeof("dsp");
		strncpy(mcu_entry->name, "dsp", srclen);
		break;
	default:
		return -EINVAL;
	}
	/*initial common part*/
	mcu_entry->type = type;
	mcu_entry->ops = mcu_ops;
	mcu_entry->msg_seq = 0;
	mcu_entry->applied_method = mcu_info->applied_method;
	if (mcu_entry->applied_method == HEADER_METHOD) {
		mcu_entry->fw_ram[MCU_FW_MODE_NORMALMODE] =
			mcu_info->fw_hdr[MCU_FW_MODE_NORMALMODE];
		mcu_entry->fw_ram_hdr_len[MCU_FW_MODE_NORMALMODE] =
			mcu_info->fw_hdr_len[MCU_FW_MODE_NORMALMODE];
		mcu_entry->fw_ram[MCU_FW_MODE_TESTMODE] =
			mcu_info->fw_hdr[MCU_FW_MODE_TESTMODE];
		mcu_entry->fw_ram_hdr_len[MCU_FW_MODE_TESTMODE] =
			mcu_info->fw_hdr_len[MCU_FW_MODE_TESTMODE];
		mcu_entry->fw_patch = mcu_info->rom_patch_hdr;
	} else {
		mcu_entry->fw_ram[MCU_FW_MODE_NORMALMODE] =
			mcu_info->fw[MCU_FW_MODE_NORMALMODE];
		mcu_entry->fw_ram[MCU_FW_MODE_TESTMODE] =
			mcu_info->fw[MCU_FW_MODE_TESTMODE];
		mcu_entry->fw_patch = mcu_info->rom_patch;
	}
	mcu_entry->fw_mode = mcu_info->fw_mode;
	mcu_entry->option = mcu_info->opt;
	mutex_init(&mcu_entry->mutex);
	skb_queue_head_init(&mcu_entry->res_q);
	init_waitqueue_head(&mcu_entry->wait);

	/*initial specific part*/
	ret = mcu_ops->init(dev, mcu_entry);
	if (ret)
		return ret;

	/*change state*/
	mcu_state_trans(mcu_entry, MCU_STATE_INIT);
	return 0;
}

static int
mcu_entry_exit(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	int ret = 0;

	if (mcu_entry->state != MCU_STATE_INIT)
		goto end;

	ret = mcu_entry->ops->exit(dev, mcu_entry);
	if (ret)
		dev_err(dev->dev, "%s(): Error ret = %d\n", __func__, ret);

end:
	mcu_state_trans(mcu_entry, MCU_STATE_NONE);
	skb_queue_purge(&mcu_entry->res_q);
	return ret;
}

static int
mcu_entry_start(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	int ret = -EINVAL;

	if (mcu_entry->state != MCU_STATE_INIT)
		goto err;

	ret = mcu_entry->ops->fwdl_patch(dev, mcu_entry);
	if (ret)
		goto err;

	mcu_state_trans(mcu_entry, MCU_STATE_PATCH);

	ret = mcu_entry->ops->start(dev, mcu_entry);

	if (ret)
		goto err;

	mcu_state_trans(mcu_entry, MCU_STATE_START);
	return 0;
err:
	mcu_state_trans(mcu_entry, MCU_STATE_INIT);
	return ret;
}

static int
mcu_entry_stop(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	int ret = 0;

	if (mcu_entry->state != MCU_STATE_START)
		goto end;

	ret = mcu_entry->ops->stop(dev, mcu_entry);
	if (ret)
		dev_err(dev->dev, "%s(): Error ret = %d\n", __func__, ret);

	mcu_state_trans(mcu_entry, MCU_STATE_INIT);
end:
	return ret;
}

int
mtk_mcu_init_device(struct mtk_hw_dev *dev)
{
	struct mtk_mcu_ctrl *mcu_ctrl = &dev->mcu_ctrl;
	struct mtk_chip_drv *chip_drv = dev->chip_drv;
	unsigned long fw_flags = chip_drv->hw_caps->fw_flags;
	int ret;
	int i;

	for (i = 0 ; i < FW_FLAG_MAX; i++) {
		if (test_bit(i, &fw_flags)) {
			mcu_ctrl->mcu_support |= BIT(i);
			ret = mcu_entry_init(dev, &mcu_ctrl->entries[i], i);
			if (ret)
				goto err;
		}
	}

	dev_dbg(dev->dev, "Firmware init done\n");
	return 0;
err:
	return ret;
}

int
mtk_mcu_exit_device(struct mtk_hw_dev *dev)
{
	struct mtk_mcu_ctrl *mcu_ctrl = &dev->mcu_ctrl;
	int i;

	for (i = 0 ; i < FW_FLAG_MAX; i++) {
		if (mcu_ctrl->mcu_support & BIT(i))
			mcu_entry_exit(dev, &mcu_ctrl->entries[i]);
	}
	return 0;
}

int
mtk_mcu_start_device(struct mtk_hw_dev *dev)
{
	struct mtk_mcu_ctrl *mcu_ctrl = &dev->mcu_ctrl;
	int ret = 0;
	int i;

	for (i = 0 ; i < FW_FLAG_MAX; i++) {
		if (mcu_ctrl->mcu_support & BIT(i)) {
			ret = mcu_entry_start(dev, &mcu_ctrl->entries[i]);
			if (ret) {
				dev_err(dev->dev, "%s(): MCU[%d] start failed, ret = %d\n",
					__func__, i, ret);
				goto err;
			}
		}
	}
	return 0;
err:
	for (i -= 1; i >= FW_FLAG_WM; i--) {
		if (mcu_ctrl->mcu_support & BIT(i))
			mcu_entry_stop(dev, &mcu_ctrl->entries[i]);
	}
	return ret;
}

int
mtk_mcu_stop_device(struct mtk_hw_dev *dev)
{
	struct mtk_mcu_ctrl *mcu_ctrl = &dev->mcu_ctrl;
	int i;

	for (i = 0 ; i < FW_FLAG_MAX; i++) {
		if (mcu_ctrl->mcu_support & BIT(i))
			mcu_entry_stop(dev, &mcu_ctrl->entries[i]);
	}
	return 0;
}

u8
mtk_mcu_dest_2_s2d(u8 dest)
{
	u8 s2d_index = MCU_S2D_H2N;

	if (dest == MCU_DEST_WM)
		s2d_index = MCU_S2D_H2N;
	else if (dest == MCU_DEST_WA)
		s2d_index = MCU_S2D_H2C;
	else if (dest == MCU_DEST_WA_WM)
		s2d_index = MCU_S2D_H2CN;

	return s2d_index;
}

const char *
mtk_mcu_mem_find(const char *src, size_t src_len, const char *target, size_t target_len)
{
	const char *cur;
	const char *last;

	if (src == NULL)
		return NULL;

	if (target == NULL)
		return NULL;

	if (target_len <= 1)
		return NULL;

	last = src + src_len - target_len;

	for (cur = src; cur <= last; ++cur) {
		if (cur[0] == target[0] && memcmp(cur, target, target_len) == 0)
			return cur;
	}

	return NULL;
}

void
mtk_mcu_get_fw_info(struct mtk_hw_dev *dev, int mcu_type, char *fw_ver, char *build_date,
			char *fw_ver_long)
{
	struct mtk_mcu_ctrl *mcu_ctrl = &dev->mcu_ctrl;
	struct mtk_mcu_entry *mcu_entry = &mcu_ctrl->entries[mcu_type];

	if (!mcu_entry->ops || !mcu_entry->ops->get_fw_info)
		return;

	mcu_entry->ops->get_fw_info(dev, mcu_entry, fw_ver, build_date, fw_ver_long);
}

void
mtk_mcu_set_fw_mode(struct mtk_hw_dev *dev, int mcu_type, u8 fw_mode)
{
	struct mtk_mcu_ctrl *mcu_ctrl = &dev->mcu_ctrl;
	struct mtk_mcu_entry *mcu_entry = &mcu_ctrl->entries[mcu_type];

	if (!mcu_entry->ops->set_fw_mode)
		return;

	mcu_entry->ops->set_fw_mode(dev, mcu_entry, fw_mode);
}
