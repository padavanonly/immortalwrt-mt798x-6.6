/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2022 MediaTek Inc.
 * Author: Henry Yen <henry.yen@mediatek.com>
 */

#include <linux/regmap.h>
#include "mtk_eth_soc.h"
#include "mtk_eth_reset.h"

extern struct mtk_eth *g_eth;

static int mtk_rest_cnt;
int mtk_wifi_num;
u32 mtk_reset_flag = MTK_FE_START_RESET;
bool mtk_stop_fail;

DECLARE_COMPLETION(wait_ack_done);
DECLARE_COMPLETION(wait_ser_done);
DECLARE_COMPLETION(wait_tops_done);

bool (*mtk_check_wifi_busy)(u32 wdma_idx) = NULL;
EXPORT_SYMBOL(mtk_check_wifi_busy);

void mtk_set_pse_drop(u32 config)
{
	struct mtk_eth *eth = g_eth;

	if (!eth || mtk_is_netsys_v1(eth))
		return;

	mtk_w32(eth, config, PSE_PPE_DROP(0));
	if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSTCTRL_PPE1))
		mtk_w32(eth, config, PSE_PPE_DROP(1));
	if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSTCTRL_PPE2))
		mtk_w32(eth, config, PSE_PPE_DROP(2));
}
EXPORT_SYMBOL(mtk_set_pse_drop);

int mtk_eth_netdevice_event(struct notifier_block *n, unsigned long event, void *ptr)
{
	struct mtk_eth *eth = container_of(n, struct mtk_eth, reset.netdevice_notifier);

	switch (event) {
	case MTK_TOPS_DUMP_DONE:
		complete(&wait_tops_done);
		break;
	case MTK_WIFI_RESET_DONE:
	case MTK_FE_STOP_TRAFFIC_DONE:
		pr_info("%s rcv done event:%lx\n", __func__, event);
		mtk_rest_cnt--;
		if (!mtk_rest_cnt) {
			complete(&wait_ser_done);
			mtk_rest_cnt = mtk_wifi_num;
		}
		break;
	case MTK_WIFI_CHIP_ONLINE:
		mtk_wifi_num++;
		mtk_rest_cnt = mtk_wifi_num;
		break;
	case MTK_WIFI_CHIP_OFFLINE:
		mtk_wifi_num--;
		mtk_rest_cnt = mtk_wifi_num;
		break;
	case MTK_FE_STOP_TRAFFIC_DONE_FAIL:
		mtk_stop_fail = true;
		mtk_reset_flag = MTK_FE_START_RESET;
		pr_info("%s rcv done event:%lx\n", __func__, event);
		complete(&wait_ser_done);
		mtk_rest_cnt = mtk_wifi_num;
		break;
	case MTK_FE_START_RESET_INIT:
		pr_info("%s rcv fe start reset init event:%lx\n", __func__, event);
		if (!test_bit(MTK_RESETTING, &eth->state)) {
			mtk_reset_flag = MTK_FE_START_RESET;
			schedule_work(&eth->pending_work);
		}
		break;
	case MTK_WIFI_L1SER_DONE:
		pr_info("%s rcv wifi done ack :%lx\n", __func__, event);
		complete(&wait_ack_done);
		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}
