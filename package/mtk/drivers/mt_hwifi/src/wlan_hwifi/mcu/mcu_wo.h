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

#ifndef __MTK_MCU_WO_H
#define __MTK_MCU_WO_H

extern const struct mtk_mcu_ops mtk_wo_ops;

struct wo_txd {
	u32 cmd_id;
	u32 len;
};

enum WO_CMD_ID {
	WO_CMD_WED_START = 0x0000,
	WO_CMD_WED_CFG  = WO_CMD_WED_START,
	WO_CMD_WED_RX_STAT = 0x0001,
	WO_CMD_RRO_SER = 0x0002,
	WO_CMD_DBG_INFO = 0x0003,
	WO_CMD_DEV_INFO = 0x0004,
	WO_CMD_BSS_INFO = 0x0005,
	WO_CMD_STA_REC = 0x0006,
	WO_CMD_DEV_INFO_DUMP = 0x0007,
	WO_CMD_BSS_INFO_DUMP = 0x0008,
	WO_CMD_STA_REC_DUMP = 0x0009,
	WO_CMD_BA_INFO_DUMP = 0x000A,
	WO_CMD_FBCMD_Q_DUMP = 0x000B,
	WO_CMD_FW_LOG_CTRL = 0x000C,
	WO_CMD_LOG_FLUSH = 0x000D,
	WO_CMD_CHANGE_STATE = 0x000E,
	WO_CMD_WED_END
};

#endif
