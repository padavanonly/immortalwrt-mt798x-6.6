// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 Mediatek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <linux/device.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_platform.h>

#include "pce/internal.h"
#include "pce/tport_map.h"

int mtk_pce_tport_map_ts_config_read(enum ts_config_entry entry,
				    struct tsc_desc *ts_cfg)
{
	struct fe_mem_msg msg;
	int ret = 0;

	if (!ts_cfg)
		return -EINVAL;

	if (!(TS_CONFIG_MASK & BIT(entry))) {
		PCE_ERR("invalid ts config entry: %u\n", entry);
		return -EPERM;
	}

	mtk_pce_fe_mem_msg_config(&msg, FE_MEM_CMD_READ, FE_MEM_TYPE_TS_CONFIG, entry);
	memset(&msg.raw, 0, sizeof(msg.raw));

	ret = mtk_pce_fe_mem_msg_send(&msg);
	if (ret) {
		PCE_NOTICE("fe_mem send msg failed: %d\n", ret);
		return ret;
	}

	memcpy(ts_cfg, &msg.tdesc, sizeof(*ts_cfg));

	return ret;
}

int mtk_pce_tport_map_ts_config_write(enum ts_config_entry entry,
				    struct tsc_desc *ts_cfg)
{
	struct fe_mem_msg msg;
	int ret = 0;

	if (!ts_cfg)
		return -EINVAL;

	if (!(TS_CONFIG_MASK & BIT(entry))) {
		PCE_NOTICE("invalid ts config entry: %u\n", entry);
		return -EPERM;
	}

	mtk_pce_fe_mem_msg_config(&msg, FE_MEM_CMD_WRITE, FE_MEM_TYPE_TS_CONFIG, entry);
	memset(&msg.raw, 0, sizeof(msg.raw));
	memcpy(&msg.raw, ts_cfg, sizeof(struct tsc_desc));

	ret = mtk_pce_fe_mem_msg_send(&msg);
	if (ret) {
		PCE_NOTICE("fe_mem send msg failed: %d\n", ret);
		return ret;
	}

	return ret;
}

int mtk_pce_tport_map_ppe_read(enum pse_port ppe, u64 *map)
{
	if (!map)
		return -EINVAL;

	if (!(PSE_PORT_PPE_MASK & BIT(ppe))) {
		PCE_NOTICE("invalid ppe index: %u\n", ppe);
		return -EPERM;
	}

	*map = mtk_pce_ppe_read(ppe, PPE_TPORT_TBL_0);
	*map |= ((u64)mtk_pce_ppe_read(ppe, PPE_TPORT_TBL_1)) << 32;

	return 0;
}

static int mtk_pce_tport_map_update_ts_config(enum ts_config_entry entry,
					      u32 tport_idx,
					      enum pse_port target)
{
	u32 shift = (tport_idx % TPORT_IDX_PER_REG) * PSE_PER_PORT_BITS;
	u32 val = (target & PSE_PER_PORT_MASK) << shift;
	u32 mask = PSE_PER_PORT_MASK << shift;
	struct tsc_desc ts_cfg;
	int ret = 0;

	ret = mtk_pce_tport_map_ts_config_read(entry, &ts_cfg);
	if (ret)
		return ret;

	if (tport_idx < TPORT_IDX_MAX / 2) {
		ts_cfg.tport_map_lower &= ~mask;
		ts_cfg.tport_map_lower |= val;
	} else {
		ts_cfg.tport_map_upper &= ~mask;
		ts_cfg.tport_map_upper |= val;
	}

	ret = mtk_pce_tport_map_ts_config_write(entry, &ts_cfg);
	if (ret)
		return ret;

	return ret;
}

static int mtk_pce_tport_map_update_ppe(enum pse_port ppe,
					u32 tport_idx,
					enum pse_port target)
{
	u32 shift = (tport_idx % TPORT_IDX_PER_REG) * PSE_PER_PORT_BITS;
	u32 val = (target & PSE_PER_PORT_MASK) << shift;
	u32 mask = PSE_PER_PORT_MASK << shift;

	if (tport_idx < TPORT_IDX_MAX / 2)
		mtk_pce_ppe_rmw(ppe, PPE_TPORT_TBL_0, mask, val);
	else
		mtk_pce_ppe_rmw(ppe, PPE_TPORT_TBL_1, mask, val);

	return 0;
}

/*
 * update tport idx mapping
 * pse_port: the pse port idx that is going to be modified
 * tport_idx: the tport idx that is going to be modified
 * target: the next port for packet when the packet is at pse_port with tport_idx
 */
int mtk_pce_tport_map_pse_port_update(enum pse_port pse_port,
				      u32 tport_idx,
				      enum pse_port target)
{
	int ret = 0;

	if (pse_port >= __PSE_PORT_MAX || target >= __PSE_PORT_MAX) {
		PCE_NOTICE("invalid pse_port: %u, target: %u\n", pse_port, target);
		return -EPERM;
	}

	if (tport_idx >= TPORT_IDX_MAX) {
		PCE_NOTICE("invalid tport_idx: %u\n", tport_idx);
		return -EPERM;
	}

	if (TS_CONFIG_MASK & BIT(pse_port))
		ret = mtk_pce_tport_map_update_ts_config((enum ts_config_entry)pse_port,
							 tport_idx, target);
	else if (PSE_PORT_PPE_MASK & BIT(pse_port))
		ret = mtk_pce_tport_map_update_ppe(pse_port, tport_idx, target);
	else
		ret = -EINVAL;

	if (ret)
		PCE_ERR("update tport map failed: %d\n", ret);

	return ret;
}
