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

#ifndef __MTK_MAIN_H
#define __MTK_MAIN_H

#include "config.h"
#include "bus.h"
#include "mac_if.h"
#include "chips.h"
#include "wsys.h"

/**
 * struct mtk_hwifi_drv - main data structrue for hwifi core
 *
 * @param inf interface manager
 * @param bus bus manager
 * @param chips chip manager
 * @param wsys hardware wifi system manager
 * @param debugfs_dir debug framework root
 */
struct mtk_hwifi_drv {
	struct mtk_interface_mgmt inf;
	struct mtk_bus_mgmt bus;
	struct mtk_chip_mgmt chips;
	struct mtk_wsys_mgmt wsys;
	struct dentry *debugfs_dir;
};

struct mtk_bus_mgmt *
mtk_hwifi_get_bus_mgmt(void);

struct mtk_chip_mgmt *
mtk_hwifi_get_chip_mgmt(void);

struct mtk_interface_mgmt *
mtk_hwifi_get_interface_mgmt(void);

struct mtk_wsys_mgmt *
mtk_hwifi_get_wsys_mgmt(void);

struct mtk_bus_trans *
mtk_hwifi_alloc_device(struct device *pdev, struct mtk_chip *chip);

void
mtk_hwifi_free_device(struct mtk_bus_trans *trans);

int
mtk_hwifi_register_device(struct mtk_bus_trans *trans);

void
mtk_hwifi_unregister_device(struct mtk_bus_trans *trans);

int
mtk_hwifi_free_tx(struct mtk_hw_dev *dev, u32 token_id, u16 wcid);

int
mtk_hwifi_tx_status(struct mtk_hw_dev *dev, u16 wcid, void *tx_sts);

int
mtk_hwifi_queue_rx_data(struct mtk_hw_dev *dev, struct sk_buff *skb);
#endif
