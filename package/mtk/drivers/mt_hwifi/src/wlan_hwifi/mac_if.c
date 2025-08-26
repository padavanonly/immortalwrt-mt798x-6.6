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

#include "mac_if.h"
#include "main.h"
#include "hw_ops.h"

static int
mtk_interface_register_debugfs(struct mtk_interface_mgmt *info,
	struct mtk_interface *inf)
{
	struct dentry *dir;

	static const char interface_type_str[][16] = {
		"mac80211_if",
		"connac_if",
	};

	dir = mt_debugfs_create_dir(interface_type_str[inf->type],
		info->debugfs_dir);

	if (!dir)
		goto err;

	inf->debugfs_dir = dir;
	return 0;
err:
	return -ENOMEM;
}

static void
mtk_interface_unregister_debugfs(struct mtk_interface *inf)
{
	mt_debugfs_remove_recursive(inf->debugfs_dir);
}

static int
mtk_interface_sanity(struct mtk_interface *inf)
{
	struct mtk_interface_ops *ops = &inf->ops;

	if (inf->type >= MTK_INTERFACE_NUM)
		goto err;
	if (!ops->register_device)
		goto err;
	if (!ops->alloc_device)
		goto err;
	if (!ops->unregister_device)
		goto err;
	if (!ops->free_device)
		goto err;
	return 0;
err:
	return -EINVAL;
}

int
mtk_interface_register(struct mtk_interface *inf)
{
	struct mtk_interface_mgmt *if_mgmt = mtk_hwifi_get_interface_mgmt();
	int ret;

	ret = mtk_interface_sanity(inf);

	if (ret)
		goto err;

	mutex_lock(&if_mgmt->mutex);
	if_mgmt->interfaces[inf->type] = inf;
	mutex_unlock(&if_mgmt->mutex);
	mtk_interface_register_debugfs(if_mgmt, inf);
	return 0;
err:
	return -EINVAL;
}
EXPORT_SYMBOL(mtk_interface_register);

void
mtk_interface_unregister(struct mtk_interface *inf)
{
	struct mtk_interface_mgmt *if_mgmt = mtk_hwifi_get_interface_mgmt();
	int ret;

	ret = mtk_interface_sanity(inf);

	if (ret)
		goto err;

	if (!if_mgmt->interfaces[inf->type])
		goto err;

	mutex_lock(&if_mgmt->mutex);
	if_mgmt->interfaces[inf->type] = NULL;
	mutex_unlock(&if_mgmt->mutex);
	mtk_interface_unregister_debugfs(inf);
err:
	return;
}
EXPORT_SYMBOL(mtk_interface_unregister);

/**
 * @brief Initial mac interface sub moudle
 *  prepare if_mgmt
 *
 * @param *drv hwifi driver
 * @param *if_mgmt interface management
 * @retval 0 success
 * @retval -ENOMEM create dir fail
 */
int
mtk_interface_init(struct mtk_hwifi_drv *drv,
	struct mtk_interface_mgmt *if_mgmt)
{
	struct dentry *dir;
	int i;

	dir = mt_debugfs_create_dir("interface", drv->debugfs_dir);
	if (!dir)
		goto err;

	if_mgmt->debugfs_dir = dir;

	for (i = 0; i < MTK_INTERFACE_NUM; i++)
		if_mgmt->interfaces[i] = NULL;

	mutex_init(&if_mgmt->mutex);
	return 0;
err:
	return -ENOMEM;
}

void
mtk_interface_exit(struct mtk_interface_mgmt *if_mgmt)
{
	int i;
	struct mtk_interface *inf;

	mutex_lock(&if_mgmt->mutex);

	for (i = 0 ; i < MTK_INTERFACE_NUM; i++) {
		inf = if_mgmt->interfaces[i];
		if (inf)
			mtk_interface_unregister(inf);
	}

	mt_debugfs_remove_recursive(if_mgmt->debugfs_dir);

	mutex_unlock(&if_mgmt->mutex);
	mutex_init(&if_mgmt->mutex);
}

static inline struct mtk_interface *
get_interface(u8 interface_type)
{
	struct mtk_interface_mgmt *if_mgmt = mtk_hwifi_get_interface_mgmt();

	if (interface_type >= MTK_INTERFACE_NUM)
		goto err;

	return if_mgmt->interfaces[interface_type];
err:
	return NULL;
}

struct mtk_hw_dev*
mtk_interface_alloc_device(struct device *pdev, struct mtk_chip *chip)
{
	struct mtk_chip_drv *drv = chip->drv;
	struct mtk_interface *inf = get_interface(drv->interface_type);
	struct mtk_hw_dev *dev;
	int sz;

	sz = ALIGN(drv->hw_caps->dev_size,
		NETDEV_ALIGN) + chip->bus_drv->ctl_size;

	if (!inf)
		goto err;

	dev = inf->ops.alloc_device(drv, pdev, sz);
	if (!dev)
		goto err;

	dev->inf_ops = &inf->ops;
	return dev;
err:
	return NULL;
}
