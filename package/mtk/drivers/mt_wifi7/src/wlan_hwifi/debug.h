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

#ifndef __MTK_DEBUG_H__
#define __MTK_DEBUG_H__

#define MAX_BUF_LEN 256

enum mtk_dbg_path {
	MTK_IOCTL_PATH,
	MTK_DBGFS_PATH
};

enum mtk_debug_mask {
	MTK_CORE = BIT(0),
	MTK_MCU = BIT(1),
	MTK_BUS = BIT(2),
	MTK_MAC = BIT(3),
	MTK_TX = BIT(4),
	MTK_RX = BIT(5),
	MTK_TXD = BIT(6),
	MTK_TXD_DUMP = BIT(7),
	MTK_RXD = BIT(8),
	MTK_RXD_DUMP = BIT(9),
	MTK_TK_TX = BIT(10),
	MTK_TK_RX = BIT(11),
	MTK_MSDUPG_DUMP = BIT(12),
	MTK_RRO = BIT(13),
	MTK_RRO_ERR = BIT(14),
	MTK_SDO = BIT(15),
	MTK_ANY = 0xffffffff,
};

struct mtk_cmm_dbg {
	char *name;
	int (*proc)(enum mtk_dbg_path path, void *data, char *arg);
};

int mtk_dbg_info(enum mtk_dbg_path path, void *data, char *arg);

#ifdef CONFIG_HWIFI_DBG
extern unsigned int mtk_dbg_mask;

void mtk_dbg(enum mtk_debug_mask dbg_mask,
			const char *fmt, ...);
void mtk_dbg_dump(enum mtk_debug_mask dbg_mask,
				const char *msg,
				const void *buf, size_t len);
#else
static inline void mtk_dbg(enum mtk_debug_mask dbg_mask,
				const char *fmt, ...)
{
}
static inline void mtk_dbg_dump(enum mtk_debug_mask dbg_mask,
					const char *msg,
					const void *buf, size_t len)
{
}
#endif

#endif
