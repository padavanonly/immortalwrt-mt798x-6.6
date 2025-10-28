/*
 * Copyright (c) [2023] MediaTek Inc.
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

#ifndef __MTK_CONNAC_MD_COEX_H__
#define __MTK_CONNAC_MD_COEX_H__

#include <linux/kernel.h>
#include <os/hwifi_mac.h>
#include "core.h"
#include "bus.h"
#include "mac_ops.h"

/*
 * These DEFINE below should have a common header with MD COEX driver,
 * discuss with owner to improve the design in the future.
 */
#define APCCCI_DRIVER_FW		0x0100
#define FW_DRIVER_APCCCI		0x0200
#define REGISTER_WIFI_MD_DTB		0x0300
#define UNREGISTER_WIFI_MD_DTB		0x0400
#define QUERY_FW_STATUS			0x0500
#define COEX_DEBUG_LEVEL_MSG		0x0600

struct _COEX_FW2APCCCI_MSG_T {
	void *device;
	u16 len;
	u8 pci_num;
	u8 pci_slot_id;
	u8 card_type;
	u8 data[0];
};

int connac_if_md_coex_register_info(struct mtk_hw_dev *hw_dev);
void connac_if_md_coex_unregister_info(struct mtk_hw_dev *hw_dev);
int connac_if_md_coex_register_notifier(struct mtk_hw_dev *hw_dev);
void connac_if_md_coex_unregister_notifier(struct mtk_hw_dev *hw_dev);
extern int register_wifi_md_coex_notifier(struct notifier_block *nb);
extern int unregister_wifi_md_coex_notifier(struct notifier_block *nb);
extern int call_wifi_md_coex_notifier(unsigned long event, void *msg);

#endif
