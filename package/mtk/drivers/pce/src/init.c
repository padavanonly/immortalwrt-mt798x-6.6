// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 Mediatek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <linux/device.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

#include "pce/cdrt.h"
#include "pce/cls.h"
#include "pce/debugfs.h"
#include "pce/dipfilter.h"
#include "pce/internal.h"
#include "pce/netsys.h"
#include "pce/pce.h"

struct device *pce_dev;

static int mtk_pce_probe(struct platform_device *pdev)
{
	int ret = 0;

	pce_dev = &pdev->dev;

	ret = mtk_pce_netsys_init(pdev);
	if (ret)
		return ret;

	ret = mtk_pce_cls_init(pdev);
	if (ret)
		goto netsys_deinit;

	ret = mtk_pce_dipfilter_init(pdev);
	if (ret)
		goto cls_deinit;

	ret = mtk_pce_cdrt_init(pdev);
	if (ret)
		goto dipfilter_deinit;

	ret = mtk_pce_debugfs_init(pdev);
	if (ret)
		goto cdrt_deinit;

	ret = mtk_pce_enable();
	if (ret)
		goto debugfs_deinit;

	return ret;

debugfs_deinit:
	mtk_pce_debugfs_deinit(pdev);

cdrt_deinit:
	mtk_pce_cdrt_deinit(pdev);

dipfilter_deinit:
	mtk_pce_dipfilter_deinit(pdev);

cls_deinit:
	mtk_pce_cls_deinit(pdev);

netsys_deinit:
	mtk_pce_netsys_deinit(pdev);

	return ret;
}

static int mtk_pce_remove(struct platform_device *pdev)
{
	mtk_pce_disable();

	mtk_pce_debugfs_deinit(pdev);

	mtk_pce_cdrt_deinit(pdev);

	mtk_pce_dipfilter_deinit(pdev);

	mtk_pce_cls_deinit(pdev);

	mtk_pce_netsys_deinit(pdev);

	return 0;
}

static const struct of_device_id mtk_pce_match[] = {
	{ .compatible = "mediatek,pce", },
	{ },
};
MODULE_DEVICE_TABLE(of, mtk_pce_match);

static struct platform_driver mtk_pce_driver = {
	.probe = mtk_pce_probe,
	.remove = mtk_pce_remove,
	.driver = {
		.name = "mediatek,pce",
		.owner = THIS_MODULE,
		.of_match_table = mtk_pce_match,
	},
};

static int __init mtk_pce_init(void)
{
	return platform_driver_register(&mtk_pce_driver);
}

static void __exit mtk_pce_exit(void)
{
	platform_driver_unregister(&mtk_pce_driver);
}

module_init(mtk_pce_init);
module_exit(mtk_pce_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek PCE Driver");
MODULE_AUTHOR("Ren-Ting Wang <ren-ting.wang@mediatek.com>");
