/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 Mediatek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _PCE_DEBUGFS_H_
#define _PCE_DEBUGFS_H_

#include <linux/platform_device.h>

int mtk_pce_debugfs_init(struct platform_device *pdev);
void mtk_pce_debugfs_deinit(struct platform_device *pdev);
#endif /* _PCE_DEBUGFS_H_ */
