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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/etherdevice.h>
#include "core.h"
#include "main.h"
#include "hwres_mgmt.h"
#include "hw_ops.h"
#include "bus.h"

/* main structrue for hwifi driver
 * should only this global structure
 */
static struct mtk_hwifi_drv hwifi_drv;

static int
mtk_hwifi_drv_register_debugfs(struct mtk_hwifi_drv *drv)
{
	struct dentry *dir;

	dir =  mt_debugfs_create_dir("mtk_hwifi", NULL);
	if (!dir)
		goto err;

	drv->debugfs_dir = dir;
	return 0;
err:
	return -ENOMEM;
}

static void
mtk_hwifi_drv_unregister_debugfs(struct mtk_hwifi_drv *drv)
{
	mt_debugfs_remove_recursive(drv->debugfs_dir);
}

static int
mtk_hwifi_hw_init(struct mtk_hw_dev *dev)
{
	int ret;

	/*reset whole chip*/
	mtk_hw_chip_reset(dev);
	/*reset wf_subsys*/
	mtk_hw_reset(dev);
	/*hw mac tx /rx ops, formate or cr distinct*/
	ret = mtk_hw_init(dev);
	if (ret)
		goto err;
	/*mcu hw ops install*/
	mtk_mcu_hw_ops_init(dev);
	/*mac ops install*/
	mtk_hdev_ops_init(dev);
	return 0;
err:
	return ret;
}

static void
mtk_hwifi_exit_device(struct mtk_hw_dev *dev)
{
	mtk_dbg_exit_device(dev);
	mtk_mcu_exit_device(dev);
	mtk_hwctrl_exit_device(dev);
}

static int
mtk_hwifi_init_device(struct mtk_hw_dev *dev)
{
	int ret;

	clear_bit(HWIFI_STATE_RUNNING, &dev->state);
	/*initial device ops*/
	ret = mtk_hwifi_hw_init(dev);
	if (ret)
		goto err_hw_init;
	/*initial chip mcu*/
	ret = mtk_mcu_init_device(dev);
	if (ret)
		goto err_mcu_init;

	/*initial hwctrl layer*/
	ret = mtk_hwctrl_init_device(dev);
	if (ret)
		goto err_hwctrl_init;

	/*initial internal debug module*/
	ret = mtk_dbg_init_device(dev);
	if (ret)
		goto err_dbg_init;
	return 0;
err_dbg_init:
	mtk_hwctrl_exit_device(dev);
err_hwctrl_init:
	mtk_mcu_exit_device(dev);
err_mcu_init:
err_hw_init:
	return ret;
}

/*
 *
 */
struct mtk_bus_mgmt *
mtk_hwifi_get_bus_mgmt(void)
{
	return &hwifi_drv.bus;
}

/*
 *
 */
struct mtk_chip_mgmt *
mtk_hwifi_get_chip_mgmt(void)
{
	return &hwifi_drv.chips;
}

/*
 *
 */
struct mtk_interface_mgmt *
mtk_hwifi_get_interface_mgmt(void)
{
	return &hwifi_drv.inf;
}

/*
 *
 */
struct mtk_wsys_mgmt *
mtk_hwifi_get_wsys_mgmt(void)
{
	return &hwifi_drv.wsys;
}

/*
 *
 */
int
mtk_hwifi_free_tx(
	struct mtk_hw_dev *dev,
	u32 token_id,
	u16 wcid)
{
	struct sk_buff_head *tx_q;
	struct sk_buff *skb, *tmp;
	struct mtk_tk_entry *tk_entry;
	int ret;

	tk_entry = mtk_tk_get_entry(dev, token_id);
	if (!tk_entry)
		return -EINVAL;

	ret = mtk_tk_unmap_tx_q(dev, tk_entry);
	if (ret) {
		dbg_tk_free_tkid_err_inc(tk_entry->tk_mgmt);
		return -EINVAL;
	}

	tx_q = mtk_tk_get_tx_q(dev, tk_entry);

	if (!tx_q) {
		dbg_tk_free_tkid_err_inc(tk_entry->tk_mgmt);
		return -EINVAL;
	}

	skb_queue_walk_safe(tx_q, skb, tmp) {
		__skb_unlink(skb, tx_q);
		dev_kfree_skb(skb);
	}

	dbg_tk_free_tkid_inc(tk_entry->tk_mgmt);

	mtk_interface_dequeue_by_free_notify(dev, tk_entry);
	ret = mtk_tk_release_entry(dev, tk_entry);

	if (ret)
		dbg_tk_free_tkid_err_inc(tk_entry->tk_mgmt);

	return ret;
}

int
mtk_hwifi_tx_status(struct mtk_hw_dev *dev, u16 wcid, void *tx_sts)
{
	struct mtk_hw_sta *hw_sta;
	int ret = -EINVAL;

	hw_sta = mtk_hwctrl_sta_entry_find(dev, wcid);
	if (hw_sta)
		ret = mtk_interface_tx_status(dev, hw_sta, tx_sts);

	return ret;
}

/*
 *
 */
int
mtk_hwifi_queue_rx_data(struct mtk_hw_dev *dev, struct sk_buff *skb)
{
	if (!test_bit(HWIFI_STATE_RUNNING, &dev->state))
		goto err;

	__skb_queue_tail(&dev->bus_trans->rx_skb, skb);
	return 0;
err:
	return -EPERM;
}

/*
 *
 */
struct mtk_bus_trans *
mtk_hwifi_alloc_device(struct device *pdev, struct mtk_chip *chip)
{
	struct mtk_hw_dev *dev;
	struct mtk_bus_trans *trans;

	/*interface related allocation*/
	dev = mtk_interface_alloc_device(pdev, chip);

	if (!dev)
		goto err;

	dev->chip_drv = chip->drv;
	dev->dev = pdev;
	dev->chip_opt = chip->drv->chip_opt;
	dev->intr_opt = chip->drv->intr_opt;
	dev->hif_txd_ver_sdo = chip->drv->hw_caps->hif_txd_ver;
	dev->irq_type = 1;
	dev->vec_num = 1;
	dev->rro_mode = chip->drv->rro_mode;
	dev->rss_enable = false;
	dev->sdb_band_sel = chip->drv->sdb_band_sel;
	/*assign bus trans to device*/
	trans = (struct mtk_bus_trans *) ((char *) dev + ALIGN(sizeof(*dev), NETDEV_ALIGN));
	trans->dev = dev;
	dev->bus_trans = trans;
	spin_lock_init(&dev->lock);
	mutex_init(&dev->mutex);

	return trans;
err:
	return NULL;
}

void
mtk_hwifi_free_device(struct mtk_bus_trans *trans)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);

	mtk_interface_free_device(dev);
}

int
mtk_hwifi_register_device(struct mtk_bus_trans *trans)
{
	int ret;
	struct mtk_hw_dev *dev = to_hw_dev(trans);

	ret = mtk_wsys_register_device(dev);
	if (ret)
		goto err_wsys_reg_dev;

	ret = mtk_hwifi_init_device(dev);
	if (ret)
		goto err_hwifi_init_dev;

	ret = mtk_interface_register_device(dev);
	if (ret)
		goto err_if_reg_dev;

	return 0;
err_if_reg_dev:
	mtk_hwifi_exit_device(dev);
err_hwifi_init_dev:
	mtk_wsys_unregister_device(dev);
err_wsys_reg_dev:
	return ret;
}

void
mtk_hwifi_unregister_device(struct mtk_bus_trans *trans)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	/*remove mac_if device*/
	mtk_interface_unregister_device(dev);
	/*remove hwifi device*/
	mtk_hwifi_exit_device(dev);
	/*remove debugfs dir for hwifi_dev*/
	mtk_wsys_unregister_device(dev);
}

static int __init mtk_hwifi_drv_init(void)
{
	struct mtk_hwifi_drv *drv = &hwifi_drv;
	int ret = 0;

	ret = mtk_hwifi_drv_register_debugfs(drv);
	if (ret)
		goto err;

	ret = mtk_bus_init(drv, &drv->bus);
	if (ret)
		goto err;

	ret = mtk_interface_init(drv, &drv->inf);
	if (ret)
		goto err;

	ret = mtk_chip_init(drv, &drv->chips);
	if (ret)
		goto err;

	ret = mtk_wsys_init(drv, &drv->wsys);
	if (ret)
		goto err;

	return 0;
err:
	mtk_chip_exit(&drv->chips);
	mtk_interface_exit(&drv->inf);
	mtk_bus_exit(&drv->bus);
	mtk_hwifi_drv_unregister_debugfs(drv);
	return ret;
}

static void __exit mtk_hwifi_drv_exit(void)
{
	struct mtk_hwifi_drv *drv = &hwifi_drv;

	mtk_wsys_exit(&drv->wsys);
	mtk_chip_exit(&drv->chips);
	mtk_interface_exit(&drv->inf);
	mtk_bus_exit(&drv->bus);
	mtk_hwifi_drv_unregister_debugfs(drv);
}

module_init(mtk_hwifi_drv_init);
module_exit(mtk_hwifi_drv_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Mediatek Hardware Wi-Fi Driver");

