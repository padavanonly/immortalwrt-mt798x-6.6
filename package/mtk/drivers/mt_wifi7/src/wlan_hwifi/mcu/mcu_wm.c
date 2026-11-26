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
mcu_wm_patch_sem_ctrl(struct mtk_hw_dev *dev, bool get)
{
	int ret;
	u32 val = 0;

	struct mtk_mcu_resp resp = {
		.rx_data = &val,
		.rx_len = sizeof(val),
	};

	struct {
		__le32 op;
	} req = {
		.op = cpu_to_le32(get ? PATCH_SEM_GET : PATCH_SEM_RELEASE),
	};

	struct mtk_mcu_txblk mcu_txblk = {
		.fwdl = true,
		.cmd = MCU_CMD_PATCH_SEM_CONTROL,
		.data = &req,
		.len = sizeof(req),
		.wait_resp = true,
		.dest = MCU_DEST_WM,
		.resp = &resp,
	};

	ret = mtk_mcu_tx_nocheck(dev, &mcu_txblk);

	if (ret)
		goto err;

	return val;
err:
	return ret;
}

/*
 * mcu_wm_query_emi_info() for query wm umac buffer

static int
mcu_wm_query_emi_info(struct mtk_hw_dev *dev)
{
	int ret;
	u32 val = 0;
	struct mtk_mcu_resp resp = {
		.rx_data = &val,
		.rx_len = sizeof(val),
	};

	struct {
		__le32 op;
	} req = {
		.op = cpu_to_le32(1),
	};

	struct mtk_mcu_txblk mcu_txblk = {
		.fwdl = true,
		.data = &req,
		.len = sizeof(req),
		.wait_resp = true,
		.dest = MCU_DEST_WM,
		.resp = &resp,
		.cmd = MCU_CMD_QUERY_INFO,
	};

	ret = mtk_mcu_tx_nocheck(dev, &mcu_txblk);

	if (ret)
		goto err;

	return val;
err:
	return ret;
}
*/

static int
mcu_emi_start_cmd(struct mtk_hw_dev *dev, u32 addr, u32 len, u32 last)
{
	struct {
		__le32 len;
		__le32 last;
	} req = {
		.len = cpu_to_le32(len),
		.last = cpu_to_le32(last),
	};

	struct mtk_mcu_txblk mcu_txblk = {
		.fwdl = true,
		.data = &req,
		.len = sizeof(req),
		.wait_resp = true,
		.dest = MCU_DEST_WM,
		.resp = NULL,
		.cmd = MCU_CMD_EMI_FW_TRIGGER_AXI_DMA,
	};

	return mtk_mcu_tx_nocheck(dev, &mcu_txblk);
}

static int
mcu_emi_config_pda(struct mtk_hw_dev *dev, u32 addr, u32 len, u32 mode)
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
		.cmd = MCU_CMD_EMI_FW_DOWNLOAD_CONFIG,
	};

	return mtk_mcu_tx_nocheck(dev, &mcu_txblk);
}

static int
mcu_wm_init_download(struct mtk_hw_dev *dev, u32 addr,
	   u32 len, u32 mode, enum mcu_fwdl_type type)
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

	switch (type) {
	case MCU_FWDL_TYPE_ROM_PATCH:
		mcu_txblk.cmd = MCU_CMD_PATCH_START_REQ;
		break;
	case MCU_FWDL_TYPE_RAM:
		mcu_txblk.cmd = MCU_CMD_TARGET_ADDRESS_LEN_REQ;
		break;
	default:
		return -EINVAL;
	}

	return mtk_mcu_tx_nocheck(dev, &mcu_txblk);
}

static int
mcu_wm_send_firmware(struct mtk_hw_dev *dev, const void *data,
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
mcu_wm_start_patch(struct mtk_hw_dev *dev)
{
	struct {
		u8 check_crc;
		u8 reserved[3];
	} req = {
		.check_crc = 0,
	};

	struct mtk_mcu_txblk mcu_txblk = {
		.fwdl = true,
		.cmd = MCU_CMD_PATCH_FINISH_REQ,
		.data = &req,
		.len = sizeof(req),
		.wait_resp = true,
		.dest = MCU_DEST_WM,
		.resp = NULL,
	};

	return mtk_mcu_tx_nocheck(dev, &mcu_txblk);
}

static u32 mcu_wm_gen_patch_dl_mode(u32 sec_info)
{
	u32 mode = 0;

	mode |= DL_MODE_NEED_RSP;

	if (sec_info == PATCH_SECINFO_NOT_SUPPORT)
		goto exit;

	switch (field_get(PATCH_SECINFO_ENC_TYPE_MASK, sec_info)) {
	case PATCH_SECINFO_ENC_TYPE_PLAIN:
		break;
	case PATCH_SECINFO_ENC_TYPE_AES:
		mode |= DL_MODE_ENCRYPT;
		mode |= field_prep(DL_MODE_KEY_IDX,
			(sec_info & PATCH_SECINFO_ENC_AES_KEY_MASK));
		mode |= DL_MODE_RESET_SEC_IV;
		break;
	case PATCH_SECINFO_ENC_TYPE_SCRAMBLE:
		mode |= DL_MODE_ENCRYPT;
		mode |= DL_MODE_ENCRY_MODE_SEL;
		mode |= field_prep(DL_MODE_KEY_IDX,
			(sec_info & PATCH_SECINFO_ENC_SCRAMBLE_KEY_MASK));
		mode |= DL_MODE_RESET_SEC_IV;
		break;
	default:
		break;
	}

exit:
	return mode;
}

static int
mcu_wm_load_patch(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	const struct mcu_patch_hdr *hdr;
	const struct firmware *fw = NULL;
	int i, sem;
	int ret = 0;
	const char *data_ptr = NULL;
	u32 n_region = 0;

	sem = mcu_wm_patch_sem_ctrl(dev, 1);

	switch (sem) {
	case PATCH_IS_DL:
		return 0;
	case PATCH_NOT_DL_SEM_SUCCESS:
		break;
	default:
		dev_err(dev->dev, "Failed to get patch semaphore %d\n", sem);
		return -EAGAIN;
	}

	if (mcu_entry->applied_method == BIN_METHOD) {
		ret = request_firmware(&fw, mcu_entry->fw_patch, dev->dev);
		if (ret)
			goto out;

		if (!fw || !fw->data || fw->size < sizeof(*hdr)) {
			dev_err(dev->dev, "Invalid firmware\n");
			ret = -EINVAL;
			goto out;
		}

		hdr = (const struct mcu_patch_hdr *)(fw->data);
		data_ptr = fw->data;
	} else {
		hdr = (const struct mcu_patch_hdr *)(mcu_entry->fw_patch);
		data_ptr = mcu_entry->fw_patch;
		if (!hdr) {
			dev_err(dev->dev, "Invalid firmware\n");
			ret = -EINVAL;
			goto out;
		}
	}

	dev_err(dev->dev, "HW/SW Version: 0x%x, Build Time: %.16s\n",
		 be32_to_cpu(hdr->hw_sw_ver), hdr->build_date);

	n_region = be32_to_cpu(hdr->desc.n_region);
	if (n_region > PACH_MAX_N_SECTION) {
		dev_err(dev->dev, "n_region error\n");
		ret = -EINVAL;
		goto out;
	}

	for (i = 0; i < n_region; i++) {
		struct mcu_patch_sec *sec;
		const u8 *dl;
		u32 len, addr, sec_info;

		sec = (struct mcu_patch_sec *)(data_ptr + sizeof(*hdr) +
						  i * sizeof(*sec));
		if ((be32_to_cpu(sec->type) & PATCH_SEC_TYPE_MASK) !=
		    PATCH_SEC_TYPE_INFO) {
			ret = -EINVAL;
			goto out;
		}

		addr = be32_to_cpu(sec->info.addr);
		len = be32_to_cpu(sec->info.len);
		if (len > PACH_MAX_LEN) {
			dev_err(dev->dev, "len error\n");
			ret = -EINVAL;
			goto out;
		}

		sec_info = be32_to_cpu(sec->info.sec_info);
		dl = data_ptr + be32_to_cpu(sec->offs);

		ret = mcu_wm_init_download(dev, addr, len,
			mcu_wm_gen_patch_dl_mode(sec_info),
			MCU_FWDL_TYPE_ROM_PATCH);
		if (ret) {
			dev_err(dev->dev, "Download request failed\n");
			goto out;
		}

		ret = mcu_wm_send_firmware(dev, dl, len);
		if (ret) {
			dev_err(dev->dev, "Failed to send patch\n");
			goto out;
		}
	}

	ret = mcu_wm_start_patch(dev);
	if (ret)
		dev_err(dev->dev, "Failed to start patch\n");

out:
	sem = mcu_wm_patch_sem_ctrl(dev, 0);
	switch (sem) {
	case PATCH_REL_SEM_SUCCESS:
		break;
	default:
		ret = -EAGAIN;
		dev_err(dev->dev, "Failed to release patch semaphore\n");
		goto out;
	}
	if (mcu_entry->applied_method == BIN_METHOD)
		release_firmware(fw);
	return ret;
}

static u32
mcu_wm_gen_dl_mode(u8 feature_set, bool is_wa)
{
	u32 ret = 0;

	ret |= (feature_set & FW_FEATURE_SET_ENCRYPT) ?
		   (DL_MODE_ENCRYPT | DL_MODE_RESET_SEC_IV) : 0;

	ret |= (feature_set | DL_MODE_ENCRY_MODE_EMI);

	ret |= field_prep(DL_MODE_KEY_IDX,
			  field_get(FW_FEATURE_SET_KEY_IDX, feature_set));
	ret |= DL_MODE_NEED_RSP;
	ret |= is_wa ? DL_MODE_WORKING_PDA_CR4 : 0;

	return ret;
}

static int
mcu_wm_start_firmware(struct mtk_hw_dev *dev, u32 addr,
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
mcu_wm_send_ram_firmware(struct mtk_hw_dev *dev,
			     const struct mcu_fw_trailer *hdr,
			     const u8 *data, bool is_wa)
{
	int i = 0, offset = 0;
	u32 override = 0, option = 0;
	u32 err = 0, umac_size;

	u32 reg_start, reg_end, len_cnt = 0;

	for (i = 0; i < hdr->n_region; i++) {
		const struct mcu_fw_region *region;
		u32 len = 0, addr = 0, mode = 0;

		region = (const struct mcu_fw_region *)((const u8 *)hdr -
			 (hdr->n_region - i) * sizeof(*region));
		mode = mcu_wm_gen_dl_mode(region->feature_set, is_wa);
		len = le32_to_cpu(region->len);
		addr = le32_to_cpu(region->addr);

		if (region->feature_set & FW_FEATURE_OVERRIDE_ADDR)
			override = addr;

		if (region->feature_set & DL_MODE_ENCRY_MODE_EMI) {

			reg_start = addr + len_cnt;
			reg_end = reg_start + len;
			/* query wm umac buffer.
			 * umac_size = mcu_wm_query_emi_info(dev)
			 * default use 8 *1024
			 */
			umac_size = 8 * 1024;
			while (reg_start != reg_end) {
				u32 sec_start, sec_end;
				u32 sec_len;

				sec_start = reg_start;
				sec_end = sec_start + umac_size > reg_end ? reg_end : sec_start
											+ umac_size;
				sec_len = sec_end - sec_start;

				err = mcu_emi_config_pda(dev, addr, sec_len, mode);
				if (err) {
					dev_err(dev->dev, "EMI config PDA failed: %d\n", err);
					return err;
				}
				err = mcu_wm_send_firmware(dev, data + offset, sec_len);
				if (err) {
					dev_err(dev->dev, "Send to EMI fail\n");
					return err;
				}
				err = mcu_emi_start_cmd(dev, addr, sec_len,
							(sec_end == reg_end) ? 1 : 0);
				if (err) {
					dev_err(dev->dev, "Send cmd to trigger AXI DMA\n");
					return err;
				}
				offset += sec_len;
				reg_start += sec_len;
				len_cnt += sec_len;
			}

		} else {
			err = mcu_wm_init_download(dev, addr, len, mode, MCU_FWDL_TYPE_RAM);
			if (err) {
				dev_err(dev->dev, "Download request failed\n");
				return err;
			}

			err = mcu_wm_send_firmware(dev, data + offset, len);
			if (err) {
				dev_err(dev->dev, "WM Failed to send firmware\n");
				return err;
			}
			offset += len;
		}
	}

	if (override)
		option |= FW_START_OVERRIDE;

	if (is_wa)
		option |= FW_START_WORKING_PDA_CR4;

	return mcu_wm_start_firmware(dev, override, option);
}


static int
mcu_wm_load_ram(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
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

	dev_info(dev->dev, "WM Firmware Version: %.10s, Build Time: %.15s, FW Mode: %d\n",
		 hdr->fw_ver, hdr->build_date, mcu_entry->fw_mode);

	mcu_info = &dev->chip_drv->hw_caps->mcu_infos[MCU_WM];
	memcpy(mcu_info->fw_ver, hdr->fw_ver, sizeof(mcu_info->fw_ver));
	memcpy(mcu_info->build_date, hdr->build_date,
					sizeof(mcu_info->build_date));

	ret = mcu_wm_send_ram_firmware(dev, hdr, data_ptr, false);
	if (ret) {
		dev_err(dev->dev, "Failed to start WM firmware\n");
		goto out;
	}

	if (mcu_entry->option & BIT(WM_OPT_BYPASS_WA)) {
		struct mcu_fw_trailer null_hdr;

		memset(&null_hdr, 0, sizeof(struct mcu_fw_trailer));
		ret = mcu_wm_send_ram_firmware(dev, &null_hdr, NULL, true);
		if (ret) {
			dev_err(dev->dev, "Failed to send fake WA start command\n");
			goto out;
		}
	}
out:
	if (mcu_entry->applied_method == BIN_METHOD)
		release_firmware(fw);
end:
	return ret;
}

static int
mcu_wm_restart(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	struct {
		u8 power_ctrl;
		u8 reserve[3];
	} req = {
		.power_ctrl = 1,
	};

	/* this is normal ram code cmd not fwdl cmd */
	struct mtk_mcu_txblk mcu_txblk = {
		.fwdl = false,
		.cmd = MCU_CMD_NIC_POWER_CTRL,
		.data = &req,
		.len = sizeof(req),
		.wait_resp = false,
		.dest = MCU_DEST_WM,
		.resp = NULL,
	};

	return mtk_mcu_tx_nocheck(dev, &mcu_txblk);
}

static int
mcu_wm_restart_uni_cmd(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	int ret = 0;
	struct uni_cmd_power_ctrl *req;
	struct uni_cmd_power_off tag;
	u32 size = sizeof(struct uni_cmd_power_ctrl) +
		sizeof(struct uni_cmd_power_off);
	struct mtk_mcu_txblk mcu_txblk;

	req = kmalloc(size, GFP_KERNEL);
	if (!req)
		return -ENOMEM;

	memset(&tag, 0, sizeof(tag));
	tag.tag = UNI_CMD_POWER_OFF;
	tag.length = sizeof(tag);
	tag.power_mode = 1;

	memset(req, 0, sizeof(size));
	memcpy(req->buf, &tag, sizeof(tag));

	/* this is normal ram code cmd not fwdl cmd */
	memset(&mcu_txblk, 0, sizeof(struct mtk_mcu_txblk));
	mcu_txblk.fwdl = false;
	mcu_txblk.cmd = UNI_CMD_ID_POWER_CTRL;
	mcu_txblk.data = req;
	mcu_txblk.len = size;
	mcu_txblk.wait_resp = false;
	mcu_txblk.dest = MCU_DEST_WM;
	mcu_txblk.resp = NULL;
	mcu_txblk.uni_cmd = 1;

	ret = mtk_mcu_tx_nocheck(dev, &mcu_txblk);
	kfree(req);

	return ret;
}

static int
mcu_wm_rx_unsolicited_event(struct mtk_hw_dev *dev, struct sk_buff *skb)
{
	struct mcu_rxd *rxd = (struct mcu_rxd *)skb->data;

	switch (rxd->eid) {
	case MCU_EVENT_EXT:
		mtk_interface_rx_event(dev, skb);
		break;
	default:
		break;
	}
	dev_kfree_skb(skb);
	return 0;
}

static int
mcu_wm_rx_uni_unsolicited_event(struct mtk_hw_dev *dev, struct sk_buff *skb)
{
	mtk_interface_rx_event(dev, skb);
	dev_kfree_skb(skb);
	return 0;
}

static int
mcu_wm_fwdl_patch(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	int ret;

	if (!mcu_entry->need_patch)
		goto end;

	ret = mtk_hw_check_fwdl_state(dev, FW_STATE_FW_DOWNLOAD);

	if (ret)
		goto err;

	ret = mcu_wm_load_patch(dev, mcu_entry);

	if (ret)
		goto err;
end:
	return 0;
err:
	return -EPERM;
}

static int
mcu_wm_start(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	int ret;

	if (!mcu_entry->need_patch) {
		ret = mtk_hw_check_fwdl_state(dev, FW_STATE_FW_DOWNLOAD);
		if (ret)
			goto err;
	}

	ret =  mcu_wm_load_ram(dev, mcu_entry);
	if (ret)
		goto err;

	if (mcu_entry->option & BIT(WM_OPT_CK_FWDL_STA)) {
		ret = mtk_hw_check_fwdl_state(dev, FW_STATE_NORMAL_TRX);
		if (ret)
			goto err;
	}

err:
	return ret;
}

static int
mcu_wm_stop(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	int ret;

	if (mcu_entry->option & BIT(WM_OPT_UNICMD_NOT_SUPP))
		ret = mcu_wm_restart(dev, mcu_entry);
	else
		ret = mcu_wm_restart_uni_cmd(dev, mcu_entry);
	if (ret)
		goto end;

	ret = mtk_hw_check_fwdl_state(dev, FW_STATE_FW_DOWNLOAD);

	skb_queue_purge(&mcu_entry->res_q);
end:
	return ret;
}

static int
mcu_wm_suspense(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	return 0;
}

static int
mcu_wm_reset(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	return 0;
}

static int
mcu_wm_tx(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, struct mtk_mcu_txblk *mcu_txblk)
{
	u8 q;

	/*FW scatter should be in fwdl q*/
	if (mcu_txblk->cmd == MCU_CMD_FW_SCATTER)
		q = Q_TX_FWDL;
	else
		q = Q_TX_CMD_WM;

	if (q != Q_TX_FWDL)
		mtk_hw_write_mcu_txd(dev, mcu_txblk);

	return mtk_bus_dma_tx_mcu_queue(dev->bus_trans, mcu_txblk->skb, q);
}

static int
mcu_wm_rx(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, struct mtk_mcu_rx_data *rx_data)
{
	struct mcu_rxd *rxd = (struct mcu_rxd *) rx_data->skb->data;

	mtk_dbg(MTK_MCU, "%s:len=%d,pt=0x%x,eid=0x%x,ext_eid=0x%x,seq=%d"
		",option=0x%x,s2d=%d\n",
		__func__, rxd->len, rxd->pkt_type_id, rxd->eid, rxd->ext_eid,
		rxd->seq, rxd->option, rxd->s2d_index);

	/* unsolicited event through wm */
	if (rxd->option & MCU_UNI_CMD_OPT_BIT_1_UNI_EVENT) {
		if (rxd->option & MCU_UNI_CMD_OPT_BIT_2_UNSOLICITED_EVENT)
			return mcu_wm_rx_uni_unsolicited_event(dev, rx_data->skb);
		/*back to framework*/
		return -EOPNOTSUPP;
	} else {
		if (rxd->ext_eid == MCU_EXT_EVENT_THERMAL_PROTECT ||
			rxd->ext_eid == MCU_EXT_EVENT_FW_LOG_2_HOST ||
			rxd->ext_eid == MCU_EXT_EVENT_ASSERT_DUMP ||
			rxd->ext_eid == MCU_EXT_EVENT_PS_SYNC ||
			rxd->ext_eid == MCU_EXT_EVENT_HE_RA_CTRL ||
			!rxd->seq)
			return mcu_wm_rx_unsolicited_event(dev, rx_data->skb);
		/*back to framework*/
		return -EOPNOTSUPP;
	}
}

static int
mcu_wm_rx_prepare(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, struct sk_buff *skb,
	struct mtk_mcu_resp *rx_info)
{
	struct mcu_rxd *rxd = (struct mcu_rxd *)skb->data;

	rx_info->seq = rxd->seq;

	/*special case, patch sem using mcu_rxd as data*/
	switch (rxd->eid) {
	case MCU_EVENT_MT_PATCH_SEM:
		rx_info->rx_data = &rxd->ext_eid;
		rx_info->rx_len = sizeof(rxd->ext_eid);
		break;
	default:
		skb_pull(skb, mcu_entry->mcu_rxd_sz);
		rx_info->rx_data = skb->data;
		rx_info->rx_len = skb->len;
		break;
	}
	return 0;
}

static int
mcu_wm_init(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	struct mtk_chip_drv *drv = dev->chip_drv;

	mcu_entry->need_patch = (mcu_entry->option & BIT(WM_OPT_PATCH_DL)) ? true : false;
	mcu_entry->mcu_rxd_sz = sizeof(struct mcu_rxd);
	mcu_entry->rx_ignore_sz = drv->hw_caps->mac_rxd_grp_0_sz;
	return 0;
}

static int
mcu_wm_exit(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry)
{
	return 0;
}

static void
mcu_wm_get_fw_info(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, char *fw_ver,
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

static void
mcu_wm_set_fw_mode(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, u8 testmode_en)
{
	struct mtk_chip_mcu_info *mcu_info =
			&dev->chip_drv->hw_caps->mcu_infos[MCU_WM];

	if (testmode_en >= MCU_FW_MODE_MAX)
		return;

	if (!mcu_entry->fw_ram[testmode_en])
		return;

	mcu_info->fw_mode = testmode_en;
	mcu_entry->fw_mode = testmode_en;
}

const struct mtk_mcu_ops mtk_wm_ops = {
	.fwdl_patch = mcu_wm_fwdl_patch,
	.start = mcu_wm_start,
	.stop = mcu_wm_stop,
	.suspense = mcu_wm_suspense,
	.reset = mcu_wm_reset,
	.tx = mcu_wm_tx,
	.rx = mcu_wm_rx,
	.rx_prepare = mcu_wm_rx_prepare,
	.init = mcu_wm_init,
	.exit = mcu_wm_exit,
	.get_fw_info = mcu_wm_get_fw_info,
	.set_fw_mode = mcu_wm_set_fw_mode,
};
