/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 Mediatek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _PCE_INTERNAL_H_
#define _PCE_INTERNAL_H_

#include <linux/device.h>
#include <linux/io.h>

extern struct device *pce_dev;

#define PCE_DBG(fmt, ...)		dev_dbg(pce_dev, fmt, ##__VA_ARGS__)
#define PCE_INFO(fmt, ...)		dev_info(pce_dev, fmt, ##__VA_ARGS__)
#define PCE_NOTICE(fmt, ...)		dev_notice(pce_dev, fmt, ##__VA_ARGS__)
#define PCE_WARN(fmt, ...)		dev_warn(pce_dev, fmt, ##__VA_ARGS__)
#define PCE_ERR(fmt, ...)		dev_err(pce_dev, fmt, ##__VA_ARGS__)
#endif /* _PCE_INTERNAL_H_ */
