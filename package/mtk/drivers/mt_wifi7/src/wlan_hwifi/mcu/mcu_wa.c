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

static u32
mcu_wa_gen_dl_mode(u8 feature_set)
{
	u32 ret = 0;

	ret |= (feature_set & FW_FEATURE_SET_ENCRYPT) ?
	       (DL_MODE_ENCRYPT | DL_MODE_RESET_SEC_IV) : 0;
	ret |= field_prep(DL_MODE_KEY_IDX,
			  field_get(FW_FEATURE_SET_KEY_IDX, feature_set));
	ret |= DL_MODE_NEED_RSP;
	ret |= DL_MODE_WORKING_PDA_CR4;

	return ret;
}

static int
mcu_wa_init_download(struct mtk_hw_dev *dev, u32 addr,
				    u32 len, u32 mode)
{
	struct {
		__le32 addr;
		__le32 len;
		__le32 mode;
	} req = {
		.addr = cpu_to_le32(addr),
		.len = cpu_to_le32(len),
		.mode = cpu_to_le32(mode),
	};

	struct mtk_mcu_txblk mcu_txblk = {
		.fwdl = true,
		.data = &req,
		.len = sizeof(req),
		.wait_resp = true,
		.dest = MCU_DEST_WM,
		.resp = NULL,
	};

	if (req.addr == MCU_PATCH_ADDRESS)
		mcu_txblk.cmd = MCU_CMD_PATCH_START_REQ;
	else
		mcu_txblk.cmd = MCU_CMD_TARGET_ADDRESS_LEN_REQ;

	/*WA download through by WM*/
	return mtk_mcu_tx_nocheck(dev, &mcu_txblk);
}

static int
mcu_wa_send_firmware(struct mtk_hw_dev *dev, const void *data,
				    int len)
{
	int ret = 0, cur_len;
	struct mtk_mcu_txblk mcu_txblk = {
		.fwdl = true,
		.cmd = MCU_CMD_FW_SCATTER,
		.wait_resp = false,
		.dest = MCU_DEST_WM,
		.resp = NULL,
	};

	while (len > 0) {
		cur_len = min_t(int, 4096 - sizeof(struct mcu_txd),
				len);

		/*FW download through by WM*/
		mcu_txblk.data = data;
		mcu_txblk.len = cur_len;
		ret = mtk_mcu_tx_nocheck(dev, &mcu_txblk);
		if (ret)
			break;

		data += cur_len;
		len -= cur_len;
	}

	return ret;
}

static int
mcu_wa_start_firmware(struct mtk_hw_dev *dev, u32 addr,
				     u32 option)
{
	struct {
		__le32 option;
		__le32 addr;
	} req = {
		.option = cpu_to_le32(option),
		.addr = cpu_to_le32(addr),
	};

	struct mtk_mcu_txblk mcu_txblk = {
		.fwdl = true,
		.cmd = MCU_CMD_FW_START_REQ,
		.data = &req,
		.len = sizeof(req),
		.wait_resp = true,
		.dest = MCU_DEST_WM,
		.resp = NULL,
	};
	return mtk_mcu_tx_nocheck(dev, &mcu_txblk);
}

static int
mcu_wa_send_ram_firmware(struct mtk_hw_dev *dev,
			     const struct mcu_fw_trailer *hdr,
			     const u8 *data)
{
	int i, offset = 0;
	u32 override = 0, option = 0;

	for (i = 0; i < hdr->n_region; i++) {
		const struct mcu_fw_region *region;
		int err;
		u32 len, addr, mode;

		region = (const struct mcu_fw_region *)((const u8 *)hdr -
			 (hdr->n_region - i) * sizeof(*region));
		mode = mcu_wa_gen_dl_mode(region->feature_set);
		len = le32_to_cpu(region->len);
		addr = le32_to_cpu(region->addr);

		if (region->feature_set & FW_FEATURE_OVERRIDE_ADDR)
			override = addr;

		err = mcu_wa_init_download(dev, addr, len, mode);
		if (err) {
			dev_err(dev->dev, "Download request failed\n");
			return err;
		}

		err = mcu_wa_send_firmware(dev, data + offset, len);
		if (err) {
			dev_err(dev->dev, "WA Failed to send firmware.\n");
			return err;
		}

		offset += len;
	}

	if (override)
		option |= FW_START_OVERRIDE;

	option |= FW_START_WORKING_PDA_CR4;

	return mcu_wa_start_firmware(dev, override, option);
}

static int
mcu_wa_load_ram(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	const struct mcu_fw_trailer *hdr;
	const struct firmware *fw;
	struct mtk_chip_mcu_info *mcu_info;
	int ret = 0;
	const char *data_ptr = NULL;

	if (mcu_entry->applied_method == BIN_METHOD) {
		ret = request_firmware(&fw, mcu_entry->fw_ram[mcu_entry->fw_mode], dev->dev);
		if (ret)
			goto end;

		if (!fw || !fw->data || fw->size < sizeof(*hdr)) {
			dev_err(dev->dev, "Invalid firmware\n");
			ret = -EINVAL;
			goto out;
		}

		hdr = (const struct mcu_fw_trailer *)(fw->data + fw->size -
						sizeof(*hdr));
		data_ptr = fw->data;
	} else {
		data_ptr = mcu_entry->fw_ram[mcu_entry->fw_mode];
		if (!data_ptr) {
			dev_err(dev->dev, "Invalid firmware\n");
			ret = -EINVAL;
			goto out;
		}
		hdr = (const struct mcu_fw_trailer *)
			(data_ptr + mcu_entry->fw_ram_hdr_len[mcu_entry->fw_mode] - sizeof(*hdr));
	}

	dev_info(dev->dev, "WA Firmware Version: %.10s, Build Time: %.15s\n",
		 hdr->fw_ver, hdr->build_date);

	mcu_info = &dev->chip_drv->hw_caps->mcu_infos[MCU_WA];
	memcpy(mcu_info->fw_ver, hdr->fw_ver, sizeof(mcu_info->fw_ver));
	memcpy(mcu_info->build_date, hdr->build_date,
					sizeof(mcu_info->build_date));

	ret = mcu_wa_send_ram_firmware(dev, hdr, data_ptr);
	if (ret) {
		dev_err(dev->dev, "Failed to start WA firmware\n");
		goto out;
	}
out:
	if (mcu_entry->applied_method == BIN_METHOD)
		release_firmware(fw);
end:
	return ret;
}

static int
mcu_wa_fwdl_patch(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	/*wa no need to do fw patch currently*/
	return 0;
}

static int
mcu_wa_start(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	int ret;

	ret = mcu_wa_load_ram(dev, mcu_entry);

	if (ret)
		return ret;

	ret = mtk_hw_check_fwdl_state(dev, FW_STATE_WACPU_RDY);
	if (ret)
		return ret;

	set_bit(HWIFI_FLAG_MCU_TXD, &dev->flags);
	return 0;
}

static int
mcu_wa_stop(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	/*do nothing, stop by wm*/
	clear_bit(HWIFI_FLAG_MCU_TXD, &dev->flags);
	return 0;
}

static int
mcu_wa_suspense(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	/*TBD*/
	return 0;
}

static int
mcu_wa_reset(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	/*TBD*/
	return 0;
}

static int
mcu_wa_tx(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, struct mtk_mcu_txblk *mcu_txblk)
{
	mtk_hw_write_mcu_txd(dev, mcu_txblk);
	return mtk_bus_dma_tx_mcu_queue(dev->bus_trans, mcu_txblk->skb, Q_TX_CMD);
}

static int
mcu_wa_rx(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, struct mtk_mcu_rx_data *rx_data)
{
	struct mtk_mcu_entry *wm_entry = mcu_get_entry_by_type(&dev->mcu_ctrl, MCU_WM);

	if (!wm_entry)
		return -EOPNOTSUPP;
	return wm_entry->ops->rx(dev, mcu_entry, rx_data);
}

static int
mcu_wa_rx_prepare(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, struct sk_buff *skb,
	struct mtk_mcu_resp *rx_info)
{
	struct mcu_rxd *rxd = (struct mcu_rxd *)skb->data;

	skb_pull(skb, mcu_entry->mcu_rxd_sz);
	rx_info->seq = rxd->seq;
	rx_info->rx_data = skb->data;
	rx_info->rx_len = skb->len;
	return 0;
}

static int
mcu_wa_init(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	struct mtk_chip_drv *drv = dev->chip_drv;

	mcu_entry->need_patch = false;
	mcu_entry->mcu_rxd_sz = sizeof(struct mcu_rxd);
	mcu_entry->rx_ignore_sz = drv->hw_caps->mac_rxd_grp_0_sz;
	return 0;
}

static int
mcu_wa_exit(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	/*do nothing, through by wm*/
	return 0;
}

static void
mcu_wa_get_fw_info(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, char *fw_ver,
			char *build_date, char *fw_ver_long)
{
	const struct firmware *fw;
	const struct mcu_fw_trailer *hdr;
	int ret = 0;
	char tmpbuf[640] = {0};
	char *key = "t-neptune";
	const char *ram_ver;
	const char *data_ptr = NULL;
	u32 data_size;

	if (mcu_entry->applied_method == BIN_METHOD) {
		ret = request_firmware(&fw, mcu_entry->fw_ram[mcu_entry->fw_mode], dev->dev);
		if (ret)
			return;

		if (!fw || !fw->data || fw->size < sizeof(*hdr)) {
			dev_err(dev->dev, "Invalid firmware\n");
			release_firmware(fw);
			return;
		}

		hdr = (const struct mcu_fw_trailer *)(fw->data + fw->size - sizeof(*hdr));
		data_ptr = fw->data;
		data_size = fw->size;
	} else {
		data_ptr = mcu_entry->fw_ram[mcu_entry->fw_mode];
		if (!data_ptr) {
			dev_err(dev->dev, "Invalid firmware\n");
			ret = -EINVAL;
			return;
		}
		hdr = (const struct mcu_fw_trailer *)
			(data_ptr + mcu_entry->fw_ram_hdr_len[mcu_entry->fw_mode] - sizeof(*hdr));
		data_size = mcu_entry->fw_ram_hdr_len[mcu_entry->fw_mode];
	}

	ret = snprintf(fw_ver, 10, "%s", hdr->fw_ver);
	if (ret < 0) {
		dev_err(dev->dev, "%s(): fw_ver parse error\n", __func__);
		fw_ver[0] = '\0';
	}
	ret = snprintf(build_date, 15, "%s", hdr->build_date);
	if (ret < 0) {
		dev_err(dev->dev, "%s(): build_date parse error\n", __func__);
		build_date[0] = '\0';
	}

	memcpy(tmpbuf, data_ptr + data_size - sizeof(tmpbuf), sizeof(tmpbuf));
	ram_ver = mtk_mcu_mem_find(tmpbuf, sizeof(tmpbuf), key, strlen(key));
	if (ram_ver) {
		ret = snprintf(fw_ver_long, strlen(ram_ver), "%s", ram_ver);
		if (ret < 0) {
			dev_err(dev->dev, "%s(): fw_ver_long parse error\n", __func__);
			fw_ver_long[0] = '\0';
		}
	}

	if (mcu_entry->applied_method == BIN_METHOD)
		release_firmware(fw);
}


const struct mtk_mcu_ops mtk_wa_ops = {
	.fwdl_patch = mcu_wa_fwdl_patch,
	.start = mcu_wa_start,
	.stop = mcu_wa_stop,
	.suspense = mcu_wa_suspense,
	.reset = mcu_wa_reset,
	.tx = mcu_wa_tx,
	.rx = mcu_wa_rx,
	.rx_prepare = mcu_wa_rx_prepare,
	.init = mcu_wa_init,
	.exit = mcu_wa_exit,
	.get_fw_info = mcu_wa_get_fw_info,
};
