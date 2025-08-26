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

#include "connac_if_md_coex.h"

int connac_if_md_coex_set_info(struct mtk_hw_dev *hw_dev,
		struct _COEX_FW2APCCCI_MSG_T *md_coex_msg)
{
	struct mtk_bus_trans *bus_trans = hw_dev->bus_trans;
	int ret;

	ret = mtk_bus_get_device(bus_trans, &md_coex_msg->device);
	if (ret) {
		dev_err(hw_dev->dev, "%s(): Get base_addr failed ret = %d\n",
			__func__, ret);
		goto err;
	}

	md_coex_msg->card_type = 0;
	md_coex_msg->len = 0;
	md_coex_msg->pci_num = 1;
	md_coex_msg->pci_slot_id = 0;

err:
	return ret;
}

int connac_if_md_coex_register_info(struct mtk_hw_dev *hw_dev)
{
	struct _COEX_FW2APCCCI_MSG_T md_coex_msg;
	int ret;

	ret = connac_if_md_coex_set_info(hw_dev, &md_coex_msg);
	if (ret) {
		dev_err(hw_dev->dev, "%s(): Set info failed ret = %d\n",
			__func__, ret);
		goto err;
	}

	ret = call_wifi_md_coex_notifier(REGISTER_WIFI_MD_DTB, &md_coex_msg);
err:
	return ret;
}

void connac_if_md_coex_unregister_info(struct mtk_hw_dev *hw_dev)
{
	struct _COEX_FW2APCCCI_MSG_T md_coex_msg;

	connac_if_md_coex_set_info(hw_dev, &md_coex_msg);
	call_wifi_md_coex_notifier(UNREGISTER_WIFI_MD_DTB, &md_coex_msg);
}

int connac_if_md_coex_tx_event(struct notifier_block *nb,
		unsigned long event, void *msg)
{
	struct mtk_hw_dev *hw_dev = container_of(nb, struct mtk_hw_dev, notifier);
	u32 pc_value;
	int ret = 0;

	switch (event) {
	case QUERY_FW_STATUS:
		ret = mtk_hdev_get_pc_value(hw_dev, &pc_value);
		if (!ret) {
			hw_dev->same_pc_cnt = hw_dev->last_pc_value == pc_value ?
					      hw_dev->same_pc_cnt + 1 : 0;
			hw_dev->last_pc_value = pc_value;

			if (hw_dev->same_pc_cnt == 10) {
				dev_err(hw_dev->dev, "%s(): Same PC cnt 10 times\n",
					__func__);
				connac_if_md_coex_unregister_info(hw_dev);
				hw_dev->same_pc_cnt = 0;
			}
		}

		break;
	default:
		ret = -EOPNOTSUPP;
	}

	return ret;
}

int connac_if_md_coex_register_notifier(struct mtk_hw_dev *hw_dev)
{
	hw_dev->notifier.notifier_call = connac_if_md_coex_tx_event;
	return register_wifi_md_coex_notifier(&hw_dev->notifier);
}

void connac_if_md_coex_unregister_notifier(struct mtk_hw_dev *hw_dev)
{
	unregister_wifi_md_coex_notifier(&hw_dev->notifier);
	hw_dev->notifier.notifier_call = NULL;
}
