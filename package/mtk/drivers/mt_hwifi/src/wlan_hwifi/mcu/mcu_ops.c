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
#include "mcu.h"
#include "hw_ops.h"

static int
mcu_cmd_wa_query(struct mtk_hw_dev *dev, u32 ctrl)
{
	struct {
		u32 opt0;
		u32 opt1;
		u32 opt2;
	} data = {
		.opt0 = cpu_to_le32(ctrl)
	};

	struct mtk_mcu_txblk mcu_txblk = {
		.cmd = MCU_CMD_WA,
		.ext_cmd = WA_EXT_CMD_ID_QUERY,
		.data = &data,
		.len = sizeof(data),
		.wait_resp = true,
		.dest = MCU_DEST_WA,
		.resp = NULL,
	};

	return mtk_mcu_tx(dev, &mcu_txblk);
}

static int
mcu_cmd_set_wa(struct mtk_hw_dev *dev, u32 opt)
{
	struct req_hdr {
		__le32 opt;
	} __packed req = {
		.opt = cpu_to_le32(opt),
	};

	struct mtk_mcu_txblk mcu_txblk = {
		.cmd = MCU_CMD_WA,
		.ext_cmd = WA_EXT_CMD_ID_CAPABILITY,
		.data = &req,
		.len = sizeof(req),
		.wait_resp = true,
		.dest = MCU_DEST_WA,
		.resp = NULL,
	};

	return mtk_mcu_tx(dev, &mcu_txblk);
}


static int
mcu_cmd_get_mem(struct mtk_hw_dev *dev, u32 addr, u8 *dest)
{
	int ret;

	struct {
		u32 addr;
		u8 data[64];
	} data = {
		.addr = cpu_to_le32(addr),
		.data = {0},
	};

	struct mtk_mcu_resp resp = {
		.rx_data = data.data,
		.rx_len = sizeof(data.data),
	};

	struct mtk_mcu_txblk mcu_txblk = {
		.cmd = MCU_EXT_CMD_DUMP_MEM,
		.data = &data,
		.len = sizeof(data),
		.wait_resp = true,
		.dest = MCU_DEST_WM,
		.resp = &resp,
	};

	ret = mtk_mcu_tx(dev, &mcu_txblk);

	if (ret)
		goto end;

	memcpy(dest, data.data, 64);
end:
	return ret;
}

static int
mcu_cmd_fw_log_2_host(struct mtk_hw_dev *dev, u8 ctrl, int dest)
{
	struct {
		u8 ctrl_val;
		u8 pad[3];
	} data = {
		.ctrl_val = ctrl
	};

	struct mtk_mcu_txblk mcu_txblk = {
		.cmd = MCU_EXT_CMD_FW_LOG_2_HOST,
		.data = &data,
		.len = sizeof(data),
		.wait_resp = true,
		.dest = dest,
		.resp = NULL,
	};

	return mtk_mcu_tx(dev, &mcu_txblk);
}

/*
 *
 */
void
mtk_mcu_hw_ops_init(struct mtk_hw_dev *dev)
{
	struct mtk_hw_ops *ops = &dev->hw_ops;

	ops->query_wa = mcu_cmd_wa_query;
	ops->get_mem = mcu_cmd_get_mem;
	ops->fw_log_2_host = mcu_cmd_fw_log_2_host;
	ops->set_wa = mcu_cmd_set_wa;
}
