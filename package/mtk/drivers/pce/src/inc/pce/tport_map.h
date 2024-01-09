/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 Mediatek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _PCE_TPORT_MAP_H_
#define _PCE_TPORT_MAP_H_

#include "pce/netsys.h"
#include "pce/pce.h"

enum ts_config_entry {
	TS_CONFIG_NONE = 0,
	TS_CONFIG_GDM1,
	TS_CONFIG_GDM2,
	TS_CONFIG_CDM2 = 6,
	TS_CONFIG_CDM3 = 8,
	TS_CONFIG_CDM4,
	TS_CONFIG_CDM6 = 10,
	TS_CONFIG_CDM5 = 13,
	TS_CONFIG_CDM7,
	TS_CONFIG_GDM3 = 15,

	__TS_CONFIG_MAX,
};

int mtk_pce_tport_map_ts_config_read(enum ts_config_entry entry,
				    struct tsc_desc *ts_cfg);
int mtk_pce_tport_map_ts_config_write(enum ts_config_entry entry,
				    struct tsc_desc *ts_cfg);
int mtk_pce_tport_map_ppe_read(enum pse_port pse_port, u64 *map);
int mtk_pce_tport_map_pse_port_update(enum pse_port pse_port,
				      u32 tport_idx,
				      enum pse_port target);
#endif /* _PCE_TPORT_MAP_H_ */
