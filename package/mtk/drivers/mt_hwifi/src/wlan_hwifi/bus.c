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

#include "bus.h"
#include "main.h"
#include "hw_ops.h"

static const char bus_type_str[][8] = {
	"pci",
	"axi",
	"usb",
	"sdio"
};

static int
mtk_bus_driver_sanity(struct mtk_bus_driver *bus_drv)
{
	if (bus_drv->bus_type > MTK_BUS_NUM)
		goto err;
	if (!bus_drv->ops.drv_register)
		goto err;
	if (!bus_drv->ops.drv_unregister)
		goto err;
	return 0;
err:
	return -EINVAL;
}

static int
mtk_bus_debugfs(struct seq_file *s, void *data)
{
	struct mtk_idr_entry *sys_idx = (struct mtk_idr_entry *)s->private;
	struct mtk_bus_driver *bus_drv = (struct mtk_bus_driver *)sys_idx->data;

	seq_printf(s, "type: %s\n", bus_type_str[bus_drv->bus_type]);
	seq_printf(s, "trans size: %zu\n", bus_drv->ctl_size);
	return 0;
}

static int
mtk_bus_trans_debugfs(struct seq_file *s, void *data)
{
	struct mtk_idr_entry *sys_idx = (struct mtk_idr_entry *)s->private;
	struct mtk_bus_trans *trans = (struct mtk_bus_trans *)sys_idx->data;

	seq_printf(s, "======= trans id %d =============\n", trans->sid.idx);
	seq_printf(s, "name\t\t: %s\n", trans->name);
	seq_printf(s, "self\t\t: %p\n", trans);
	seq_printf(s, "master\t\t: %p\n", trans->master);
	seq_printf(s, "flag\t\t: %lx\n", trans->flag);
	seq_printf(s, "io_ops\t\t: %p\n", trans->io_ops);
	seq_printf(s, "next\t\t: %p\n", trans->next);
	seq_printf(s, "pdev\t\t: %p\n", trans->pdev);
	seq_printf(s, "rx_skb head\t: %p\n", &trans->rx_skb);
	seq_printf(s, "rx_skb len\t: %d\n", trans->rx_skb.qlen);
	return 0;
}

static inline struct mtk_bus_driver *
mtk_bus_driver_get(u8 type)
{
	struct mtk_bus_mgmt *bus_mgmt = mtk_hwifi_get_bus_mgmt();
	struct mtk_bus_driver *tmp, *bus_drv = NULL;
	u32 id;

	mtk_idr_for_each_entry(&bus_mgmt->driver, tmp, id) {
		if (tmp->bus_type == type) {
			bus_drv = tmp;
			break;
		}
	}
	return bus_drv;
}

int
mtk_bus_preinit_device(struct mtk_bus_trans *trans)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	int ret = -EINVAL;

	while (trans) {
		ret = mtk_bus_dma_preinit_device(trans);
		if (ret) {
			dev_err(to_device(trans), "%s(): Preinit failed, ret = %d\n",
				__func__, ret);
			goto end;
		}

		trans = trans->next;
	}

	if (test_bit(HWIFI_FLAG_MULTI_BUS, &dev->flags)) {
		struct mtk_bus_trans *master, *slave;

		master = dev->bus_trans;
		slave = master->next;
		ret = mtk_bus_dma_switch_node(master, slave);
	}

end:
	return ret;
}

int
mtk_bus_init_device(struct mtk_bus_trans *trans)
{
	int ret = -EINVAL;

	while (trans) {
		/*per bus port need to do driver own before use it!*/
		if (test_bit(BUS_TRANS_FLAG_MASTER, &trans->flag))
			ret = mtk_hw_set_driver_own(trans->dev, trans, true);
		if (ret) {
			dev_err(to_device(trans), "%s(): Driver own failed, ret = %d\n",
				__func__, ret);
			break;
		}
		ret = mtk_bus_dma_init_device(trans);
		if (ret) {
			dev_err(to_device(trans), "%s(): Init failed, ret = %d\n",
				__func__, ret);
			break;
		}
		set_bit(BUS_TRANS_FLAG_READY, &trans->flag);
		trans = trans->next;
	}
	return ret;
}

int
mtk_bus_set_pao_sta_info(struct mtk_bus_trans *trans, u16 wcid,
			u8 max_amsdu_nums, u32 max_amsdu_len, int remove_vlan, int hdrt_mode)
{
	int ret = -EINVAL;

	while (trans) {
		ret = mtk_bus_dma_set_pao_sta_info(trans, wcid,
			max_amsdu_nums, max_amsdu_len, remove_vlan, hdrt_mode);
		if (ret)
			break;
		trans = trans->next;
	}
	return ret;
}
EXPORT_SYMBOL(mtk_bus_set_pao_sta_info);

int
mtk_bus_set_pn_check(struct mtk_bus_trans *trans,
			u16 wcid, u16 se_id, bool enable)
{
	int ret = -EINVAL;

	while (trans) {
		ret = mtk_bus_dma_set_pn_check(trans, se_id, enable);
		if (ret)
			break;
		trans = trans->next;
	}

	return ret;
}
EXPORT_SYMBOL(mtk_bus_set_pn_check);

int
mtk_bus_set_particular_to_host(
	struct mtk_bus_trans *trans, bool enable)
{
	int ret = -EINVAL;

	while (trans) {
		ret = mtk_bus_dma_set_particular_to_host(trans, enable);
		if (ret)
			break;
		trans = trans->next;
	}

	return ret;
}
EXPORT_SYMBOL(mtk_bus_set_particular_to_host);

int
mtk_bus_get_tx_token_num(struct mtk_bus_trans *trans, u16 tx_token_num[], u8 max_src_num)
{
	int ret = -EINVAL;

	if (trans)
		ret = mtk_bus_dma_get_tx_token_num(trans, tx_token_num, max_src_num);

	return ret;
}
EXPORT_SYMBOL(mtk_bus_get_tx_token_num);

int mtk_bus_get_rro_sp_page_num(struct mtk_bus_trans *trans, u32 *page_num)
{
	int ret = -EINVAL;

	if (trans)
		ret = mtk_bus_dma_get_rro_sp_page_num(trans, page_num);

	return ret;
}
EXPORT_SYMBOL(mtk_bus_get_rro_sp_page_num);

int
mtk_bus_init_after_fwdl(struct mtk_bus_trans *trans)
{
	int ret = 0;

	while (trans) {
		ret = mtk_bus_dma_init_after_fwdl(trans);
		if (ret) {
			dev_err(to_device(trans), "%s(): Init after FWDL failed, ret = %d\n",
				__func__, ret);
			break;
		}
		trans = trans->next;
	}
	return ret;
}

void
mtk_bus_exit_device(struct mtk_bus_trans *trans)
{
	int ret;

	while (trans) {
		if (!test_bit(BUS_TRANS_FLAG_READY, &trans->flag))
			goto next;

		clear_bit(BUS_TRANS_FLAG_READY, &trans->flag);
		mtk_bus_dma_exit_device(trans);

		/*per bus port need to do fw own when will not use it*/
		if (test_bit(BUS_TRANS_FLAG_MASTER, &trans->flag)) {
			ret = mtk_hw_set_driver_own(trans->dev, trans, false);
			if (ret)
				break;
		}
next:
		trans = trans->next;
	}
}

int
mtk_bus_start_device(struct mtk_bus_trans *trans)
{
	int ret;

	while (trans) {
		ret = mtk_bus_dma_start(trans);
		if (ret) {
			dev_err(to_device(trans), "%s(): Bus start failed, ret = %d\n",
				__func__, ret);
			break;
		}
		set_bit(BUS_TRANS_FLAG_START, &trans->flag);
		trans = trans->next;
	}

	return ret;
}

void
mtk_bus_stop_device(struct mtk_bus_trans *trans)
{
	while (trans) {
		if (!test_bit(BUS_TRANS_FLAG_START, &trans->flag))
			goto next;

		clear_bit(BUS_TRANS_FLAG_START, &trans->flag);
		mtk_bus_dma_stop(trans);
next:
		trans = trans->next;
	}
}

int
mtk_bus_start_traffic(struct mtk_bus_trans *trans)
{
	int ret = 0;

	while (trans) {
		ret = mtk_bus_dma_start_traffic(trans);
		if (ret)
			break;

		trans = trans->next;
	}
	return ret;
}

int
mtk_bus_stop_traffic(struct mtk_bus_trans *trans)
{
	int ret = 0;

	while (trans) {
		ret = mtk_bus_dma_stop_traffic(trans);
		if (ret)
			break;

		trans = trans->next;
	}
	return ret;
}

int
mtk_bus_get_txq(struct mtk_bus_trans *trans, u8 band_idx, u8 tid,
		void **txq, void **txq_trans)
{
	int ret = -EINVAL;
	struct mtk_bus_trans *bus_trans = trans;

	while (bus_trans) {
		ret = mtk_bus_dma_get_txq(bus_trans, band_idx, tid,
					  txq, txq_trans);
		bus_trans = bus_trans->next;

		if (ret == 0)
			break;
	}
	return ret;
}

int
mtk_bus_tx_kick(struct mtk_bus_trans *trans)
{
	int ret = 0;
	struct mtk_bus_trans *bus_trans = trans;

	while (bus_trans) {
		ret = mtk_bus_dma_tx_kick(bus_trans);
		bus_trans = bus_trans->next;
	}

	return ret;
}

int
mtk_bus_queues_read(struct seq_file *s, void *data)
{
	int ret = 0;
	struct mtk_bus_trans *bus_trans = dev_get_drvdata(s->private);

	while (bus_trans) {
		ret = mtk_bus_dma_queues_read(bus_trans, s);
		bus_trans = bus_trans->next;
	}

	return ret;
}

static struct mtk_bus_trans*
__mtk_bus_alloc_trans(struct device *pdev, struct mtk_chip *chip)
{
	struct mtk_bus_trans *bus_trans;
	int sz = chip->bus_drv->ctl_size;

	bus_trans = kzalloc(sz, GFP_KERNEL);
	return bus_trans;
}

static inline struct mtk_bus_trans*
_mtk_bus_alloc_trans(struct device *pdev,
	struct mtk_chip *chip, struct mtk_bus_cfg *bus_cfg)
{
	struct mtk_bus_trans *trans;

	if (bus_cfg->ms_type == CHIP_TYPE_MASTER) {
		trans = mtk_hwifi_alloc_device(pdev, chip);
		if (trans)
			set_bit(BUS_TRANS_FLAG_MASTER, &trans->flag);
	} else
		trans = __mtk_bus_alloc_trans(pdev, chip);

	return trans;
}

static inline void
_mtk_bus_free_trans(struct mtk_bus_trans *trans)
{
	if (test_bit(BUS_TRANS_FLAG_MASTER, &trans->flag))
		mtk_hwifi_free_device(trans);
	else
		kfree(trans);
}

static inline void
_mtk_bus_trans_group(struct mtk_bus_trans *master, struct mtk_bus_trans *slave)
{
	struct mtk_bus_trans *cur = master;
	struct mtk_hw_dev *dev = to_hw_dev(master);

	while (cur->next)
		cur = cur->next;
	cur->next = slave;
	slave->master = master;
	slave->dev = master->dev;
	set_bit(HWIFI_FLAG_MULTI_BUS, &dev->flags);
}

static struct mtk_bus_trans *
mtk_bus_trans_group_master(struct mtk_bus_trans *master)
{
	struct mtk_bus_mgmt *mgmt = mtk_hwifi_get_bus_mgmt();
	struct mtk_bus_trans *slave;
	u32 id;
	int ret;

	/*traversal all slave & join to group*/
	ret = mtk_idr_entry_register(&mgmt->trans, &master->sid, master);

	if (ret)
		goto err_idr_reg;

	/*dbgfs*/
	ret = mtk_idr_entry_register_debugfs(&mgmt->trans, &master->sid,
		"trans", mtk_bus_trans_debugfs);

	if (ret)
		goto err_dbgfs_reg;

	master->master = master;
	mtk_idr_for_each_entry(&mgmt->trans, slave, id) {
		if (test_bit(BUS_TRANS_FLAG_MASTER, &slave->flag))
			continue;
		if (mtk_bus_dma_match(master, slave))
			_mtk_bus_trans_group(master, slave);
	}
	return master;
err_dbgfs_reg:
	mtk_idr_entry_unregister(&mgmt->trans, &master->sid);
err_idr_reg:
	return NULL;
}

static struct mtk_bus_trans *
mtk_bus_trans_group_slave(struct mtk_bus_trans *slave)
{
	struct mtk_bus_mgmt *mgmt = mtk_hwifi_get_bus_mgmt();
	struct mtk_bus_trans *cur;
	u32 id;
	int ret;

	ret = mtk_idr_entry_register(&mgmt->trans, &slave->sid, slave);
	if (ret)
		goto err;

	/*dbgfs*/
	ret = mtk_idr_entry_register_debugfs(&mgmt->trans, &slave->sid,
		"trans", mtk_bus_trans_debugfs);

	if (ret)
		goto err;

	mtk_idr_for_each_entry(&mgmt->trans, cur, id) {
		if (!test_bit(BUS_TRANS_FLAG_MASTER, &cur->flag))
			continue;
		if (mtk_bus_dma_match(cur, slave)) {
			_mtk_bus_trans_group(cur, slave);
			break;
		}
	}
err:
	return NULL;
}

static void
_mtk_bus_trans_degroup(struct mtk_bus_trans *trans)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);

	clear_bit(HWIFI_FLAG_MULTI_BUS, &dev->flags);
}

static void
mtk_bus_trans_degroup_master(struct mtk_bus_trans *master)
{
	struct mtk_bus_mgmt *mgmt = mtk_hwifi_get_bus_mgmt();
	struct mtk_bus_trans *cur = master->next;

	mutex_lock(&mgmt->mutex);
	while (cur) {
		struct mtk_bus_trans *tmp = cur->next;

		cur->master = NULL;
		cur->next = NULL;
		cur = tmp;
	}
	_mtk_bus_trans_degroup(master);
	mutex_unlock(&mgmt->mutex);

	mtk_idr_entry_unregister(&mgmt->trans, &master->sid);
}

static void
mtk_bus_trans_degroup_slave(struct mtk_bus_trans *slave)
{
	struct mtk_bus_mgmt *mgmt = mtk_hwifi_get_bus_mgmt();
	struct mtk_bus_trans *master = slave->master;
	struct mtk_bus_trans *cur;

	/*remove slave from master, else do nothing*/
	if (master) {
		mutex_lock(&mgmt->mutex);
		cur = master;
		while (cur->next) {
			if (cur->next == slave) {
				cur->next = slave->next;
				slave->next = NULL;
				slave->master = NULL;
				break;
			}
		}
		_mtk_bus_trans_degroup(master);
		mutex_unlock(&mgmt->mutex);
	}

	mtk_idr_entry_unregister(&mgmt->trans, &slave->sid);
}

static struct mtk_bus_trans *
mtk_bus_trans_group(struct mtk_bus_trans *trans)
{
	struct mtk_bus_trans *master;

	if (test_bit(BUS_TRANS_FLAG_MASTER, &trans->flag))
		master = mtk_bus_trans_group_master(trans);
	else
		master = mtk_bus_trans_group_slave(trans);

	return master;
}

static void
mtk_bus_trans_degroup(struct mtk_bus_trans *trans)
{
	if (test_bit(BUS_TRANS_FLAG_MASTER, &trans->flag))
		mtk_bus_trans_degroup_master(trans);
	else
		mtk_bus_trans_degroup_slave(trans);
}

static void
mtk_bus_rx_pkt(struct mtk_bus_trans *trans, struct sk_buff *skb)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct mtk_bus_rx_info *rx_info = (struct mtk_bus_rx_info *)skb->cb;

	/* Check usage size can't more than skb->cb size */
	BUILD_BUG_ON(sizeof(struct mtk_bus_rx_info) > sizeof(skb->cb));

	/*apply to hw path due to rx info indicate ppe entry vaild*/
	if (rx_info->hw_path)
		mtk_bus_dma_add_hw_rx(trans, rx_info);

	/*drop packet due to rx info indicate drop*/
	if (rx_info->drop) {
		dbg_bus_rx_drop_inc(trans, rx_info);
		dev_kfree_skb(skb);
		goto end;
	}
	mtk_hw_rx_pkt(dev, trans, skb);
end:
	return;
}

int
mtk_bus_rx_poll(struct mtk_bus_trans *trans, struct napi_struct *napi, u8 band_idx)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);

	if (trans->rx_skb.qlen)
		return mtk_interface_rx_data(dev, napi, band_idx);
	else
		return 0;
}
EXPORT_SYMBOL(mtk_bus_rx_poll);

int
mtk_bus_rx_process(struct mtk_bus_trans *trans, u8 q_attr,
				struct sk_buff *skb)
{
	struct mtk_hw_dev *dev = to_hw_dev(trans);

	switch (q_attr) {
	case Q_RX_DATA:
	case Q_RX_IND:
	case Q_RX_RXDMAD_C:
	case Q_RX_DATA_WED:
		mtk_bus_rx_pkt(trans, skb);
		break;
	case Q_RX_EVENT_WM:
	case Q_RX_EVENT_TX_FREE_DONE:
	case Q_RX_EVENT_SDO:
		mtk_hw_rx_event(dev, trans, skb, MCU_WM);
		break;
	case Q_RX_EVENT_WA:
		mtk_hw_rx_event(dev, trans, skb, MCU_WA);
		break;
	default:
		dbg_bus_rx_err_inc(trans);
		dev_kfree_skb(skb);
		return -EINVAL;
	}
	dbg_bus_rx_inc(trans);
	return 0;
}
EXPORT_SYMBOL(mtk_bus_rx_process);

int
mtk_bus_rx_ser_event(
	struct mtk_hw_dev *dev,
	u32 chip_id,
	u32 ser_level,
	u32 ser_event,
	u32 hw_id)
{
	if (dev && (ser_level == HW_SER_LV_1_0))
		return mtk_interface_rx_swi_ser_event(
			dev,
			chip_id,
			ser_event,
			hw_id);
	else if (dev && (ser_level == HW_SER_LV_0_5))
		return mtk_interface_rx_wdt_ser_event(
			dev,
			chip_id,
			ser_event,
			hw_id);
	else if (dev && (ser_level == HW_SER_LV_10_0))
		return mtk_interface_host_detect_ser_event(
			dev,
			chip_id,
			ser_event,
			hw_id);
	else
		return 0;
}
EXPORT_SYMBOL(mtk_bus_rx_ser_event);

/**
 * @brief alloc a trans and related device/mac_if resouce
 *
 * @param *pdev hif device
 * @param *driver hif driver
 * @param *io_ops specific bus io operations
 * @param **dma_ops specific bus dma operations
 * @retval 0 register success
 * @retval !0 register fail
 */
void*
mtk_bus_alloc_trans(struct device *pdev, void *driver,
	struct mtk_bus_io_ops *io_ops, struct mtk_bus_dma_ops *dma_ops)
{
	struct mtk_chip *chip = container_of(driver, struct mtk_chip, priv_drv);
	struct mtk_bus_cfg *bus_cfg = &chip->drv->bus_cfg;
	struct mtk_bus_mgmt *bus_mgmt = mtk_hwifi_get_bus_mgmt();
	struct mtk_bus_trans *bus_trans;
	struct mtk_bus_driver *bus_drv = chip->bus_drv;
	struct mtk_bus_driver **bus_drv_list;
	int i = 0, ret = 0;

	/*allocate memory resouce by type & chip*/
	bus_trans = _mtk_bus_alloc_trans(pdev, chip, bus_cfg);

	if (!bus_trans)
		goto err_alloc_trans;

	/*support sub driver*/
	bus_trans->io_ops = io_ops;
	bus_trans->dma_ops = dma_ops;
	bus_trans->pdev = pdev;
	bus_trans->name = bus_cfg->name;
	bus_trans->next = NULL;
	bus_trans->bus_cfg = bus_cfg;
	__skb_queue_head_init(&bus_trans->rx_skb);
	spin_lock_init(&bus_trans->rx_lock);

	/*support tunneling*/
	bus_drv_list =
		kzalloc((bus_mgmt->driver.cnt * sizeof(struct mtk_bus_driver *)), GFP_KERNEL);
	if (!bus_drv_list)
		goto err_bus_drv_list_alloc;

	while (bus_drv) {
		bus_drv_list[i++] = bus_drv;
		bus_drv = bus_drv->sub_drv;
	}

	for (i -= 1; i >= 0; i--) {
		bus_drv = bus_drv_list[i];

		if (!bus_drv->ops.drv_create_tunnel)
			continue;

		bus_drv->ops.drv_create_tunnel(driver, bus_trans, bus_cfg->bus_sub_type);
	}

	kfree(bus_drv_list);

	ret = mtk_bus_dma_chip_attached(bus_trans, bus_cfg->bus_ops);
	if (ret)
		goto err_dma_chip_attach;

	/*bus related allocation*/
	ret = mtk_bus_dma_alloc_resource(bus_trans, bus_cfg->profile);
	if (ret)
		goto err_dma_alloc_resource;

	return (void *) bus_trans;
err_dma_alloc_resource:
err_dma_chip_attach:
err_bus_drv_list_alloc:
	_mtk_bus_free_trans(bus_trans);
err_alloc_trans:
	dev_err(pdev, "%s(): err ret=%d\n", __func__, ret);
	return NULL;
}
EXPORT_SYMBOL(mtk_bus_alloc_trans);

/**
 * @brief free resouce of a private trans and related deivce
 *
 * @param *trans a master private trans (such as pci_trans)
 */
int
mtk_bus_free_trans(void *trans)
{
	struct mtk_bus_trans *bus_trans = to_bus_trans(trans);

	mtk_bus_dma_free_resource(bus_trans);
	_mtk_bus_free_trans(bus_trans);
	return 0;
}
EXPORT_SYMBOL(mtk_bus_free_trans);

/**
 * @brief register a master trans and related device/mac_if layer
 *
 * pci_trans->bus_trans->hwifi_dev->ieee80211_hw->...
 * @param *trans a master private trans (such as pci_trans)
 * @retval 0 register success
 * @retval !0 register fail
 */
int
mtk_bus_register_trans(void *trans)
{
	struct mtk_bus_trans *bus_trans = to_bus_trans(trans);
	struct mtk_bus_trans *master = NULL;
	int ret;

	master = mtk_bus_trans_group(bus_trans);
	if (!master)
		return 0;

	ret = mtk_hwifi_register_device(master);
	if (ret)
		goto err_hwifi_reg_dev;

	return 0;
err_hwifi_reg_dev:
	mtk_bus_trans_degroup(bus_trans);
	return ret;
}
EXPORT_SYMBOL(mtk_bus_register_trans);

/**
 * @brief unregister a master trans and related device
 *
 * pci_trans->bus_trans->hwifi_dev->ieee80211_hw->...
 * @param *trans a master private trans (such as pci_trans)
 */
int
mtk_bus_unregister_trans(void *trans)
{
	struct mtk_bus_trans *bus_trans = to_bus_trans(trans);
	struct mtk_bus_trans *master = bus_trans->master;

	if (master)
		mtk_hwifi_unregister_device(master);

	mtk_bus_trans_degroup(trans);
	return 0;
}
EXPORT_SYMBOL(mtk_bus_unregister_trans);

/**
 *
Export function for register a specific bus drvier to
 * bus generic layer interface
 *
 * @param *bus_drv specific bus driver
 * @retval 0 initial success
 * @retval -EINVAL invalid bus driver
 */
int
mtk_bus_driver_register(struct mtk_bus_driver *bus_drv)
{
	struct mtk_bus_mgmt *bus_mgmt = mtk_hwifi_get_bus_mgmt();
	int ret;

	ret = mtk_bus_driver_sanity(bus_drv);
	if (ret)
		goto err_drv_sanity;

	ret = mtk_idr_entry_register(&bus_mgmt->driver, &bus_drv->sid, bus_drv);
	if (ret)
		goto err_idr_reg;

	ret = mtk_idr_entry_register_debugfs(&bus_mgmt->driver, &bus_drv->sid,
		"bus", mtk_bus_debugfs);
	if (ret)
		goto err_dbgfs_reg;
	return 0;
err_dbgfs_reg:
	mtk_idr_entry_unregister(&bus_mgmt->driver, &bus_drv->sid);
err_idr_reg:
err_drv_sanity:
	return ret;
}
EXPORT_SYMBOL(mtk_bus_driver_register);

/**
 *
Export function for unregister specific bus driver
 * bus generic layer interface
 *
 * @param *bus_drv specific bus driver
 */
int
mtk_bus_driver_unregister(struct mtk_bus_driver *bus_drv)
{
	struct mtk_bus_mgmt *bus_mgmt = mtk_hwifi_get_bus_mgmt();
	int ret;

	ret = mtk_bus_driver_sanity(bus_drv);
	if (ret)
		goto err;

	mtk_idr_entry_unregister(&bus_mgmt->driver, &bus_drv->sid);
	return 0;
err:
	return -EINVAL;
}
EXPORT_SYMBOL(mtk_bus_driver_unregister);

int
mtk_bus_register_chip(struct mtk_chip *chip)
{
	struct mtk_bus_cfg *bus_cfg = &chip->drv->bus_cfg;
	struct mtk_bus_driver *bus_drv = mtk_bus_driver_get(bus_cfg->bus_type);
	struct mtk_bus_driver *sub_drv;
	int ret;

	if (!bus_drv)
		goto err;

	chip->bus_drv = bus_drv;

	while (bus_drv) {
		sub_drv = mtk_bus_driver_get(bus_drv->bus_sub_type);
		bus_drv->sub_drv = sub_drv;

		if (!sub_drv)
			break;

		bus_drv = sub_drv;
	}

	sub_drv = mtk_bus_driver_get(bus_cfg->bus_sub_type);
	bus_drv->sub_drv = sub_drv;

	ret = chip->bus_drv->ops.drv_register((void *) chip->bus_drv, (void *) chip->priv_drv);
	if (ret)
		goto err;
	return 0;
err:
	return -EINVAL;
}

int
mtk_bus_unregister_chip(struct mtk_chip *chip)
{
	struct mtk_bus_cfg *bus_cfg = &chip->drv->bus_cfg;
	struct mtk_bus_driver *bus_drv = mtk_bus_driver_get(bus_cfg->bus_type);

	if (!bus_drv)
		return -EINVAL;

	bus_drv->ops.drv_unregister((void *) chip->bus_drv, (void *) chip->priv_drv);
	chip->bus_drv = NULL;
	return 0;
}

/**
 * BUS structure init
 *
 * @param *drv gloabl hwifi drv structure
 * @param *bus_mgmt bus management structure
 * @retval 0 initial success
 * @retval -ENOMEM allocate debugfs fail
 */
int
mtk_bus_init(struct mtk_hwifi_drv *drv, struct mtk_bus_mgmt *bus_mgmt)
{
	int ret;

	ret = mtk_idrm_init(&bus_mgmt->driver, "bus", drv->debugfs_dir, 0, MTK_BUS_NUM);
	if (ret)
		goto err;

	/*rid initial value should be 1, 0 is default value*/
	ret = mtk_idrm_init(&bus_mgmt->trans, "bus_trans", drv->debugfs_dir, 1, MAX_TRANS_NUM);
	if (ret)
		goto err;

	mutex_init(&bus_mgmt->mutex);
	return 0;
err:
	mtk_idrm_exit(&bus_mgmt->driver);
	return -ENOMEM;
}

/**
 * BUS structure deinit
 *
 * @param *bus_mgmt bus management structure
 */
int
mtk_bus_exit(struct mtk_bus_mgmt *bus_mgmt)
{
	mtk_idrm_exit(&bus_mgmt->trans);
	mtk_idrm_exit(&bus_mgmt->driver);
	mutex_init(&bus_mgmt->mutex);
	return 0;
}
