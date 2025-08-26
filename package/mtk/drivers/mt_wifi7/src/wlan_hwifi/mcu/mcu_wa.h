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

#ifndef __MTK_MCU_WA_H
#define __MTK_MCU_WA_H

extern const struct mtk_mcu_ops mtk_wa_ops;

/**
 * enum mcu_wa_opts - WA options
 *
 * These are wa mcu options
 */
enum mcu_wa_opts {
	WA_OPT_MAX,
};

/**
 * enum mcu_wa_extend_cmd - WA extend command index
 *
 * This partition's must accompanying with cid is "MCU_CMD_WA = 0xC4"
 *
 * @WA_EXT_CMD_ID_QUERY:
 * @WA_EXT_CMD_ID_SET:
 * @WA_EXT_CMD_ID_CAPABILITY:
 * @WA_EXT_CMD_ID_DEBUG:
 */
enum mcu_wa_extend_cmd_c4 {
	WA_EXT_CMD_ID_QUERY = 0,
	WA_EXT_CMD_ID_SET,
	WA_EXT_CMD_ID_CAPABILITY,
	WA_EXT_CMD_ID_DEBUG,
	WA_EXT_CMD_ID_MAX_NUM,
};

#endif
