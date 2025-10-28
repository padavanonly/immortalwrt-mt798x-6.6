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

#ifndef __MTK_CONNAC_H__
#define __MTK_CONNAC_H__

#ifdef CONFIG_HWIFI_MD_COEX_SUPPORT
#include "connac_if_md_coex.h"
#endif

enum WDEV_TYPE {
	WDEV_TYPE_AP = (1 << 0),
	WDEV_TYPE_STA = (1 << 1),
	WDEV_TYPE_ADHOC = (1 << 2),
	WDEV_TYPE_WDS = (1 << 3),
	WDEV_TYPE_MESH = (1 << 4),
	WDEV_TYPE_GO = (1 << 5),
	WDEV_TYPE_GC = (1 << 6),
	/* WDEV_TYPE_APCLI = (1 << 7), */
	WDEV_TYPE_APCLI = (1 << 7),
	WDEV_TYPE_REPEATER = (1 << 8),
	WDEV_TYPE_P2P_DEVICE = (1 << 9),
	WDEV_TYPE_TDLS = (1 << 10),
	WDEV_TYPE_SERVICE_TXC = (1 << 11),
	WDEV_TYPE_SERVICE_TXD = (1 << 12),
	WDEV_TYPE_ATE_AP = (1 << 13),	/* For TX with TXC */
	WDEV_TYPE_ATE_STA = (1 << 14)	/* For TX with TXD */
};

enum {
	PHY_ALLOC_SINGLE,
	PHY_ALLOC_MULTI,
	PHY_ALLOC_MAX,
};

struct connac_bss_wrap {
	struct mtk_hw_bss *cur_bss;
	struct mtk_hw_bss *bss[MAX_BAND_NUM];
} __aligned(NETDEV_ALIGN);

#endif
