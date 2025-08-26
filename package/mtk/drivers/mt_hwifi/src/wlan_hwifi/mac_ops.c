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
#include "main.h"
#include "mac_ops.h"
#include "hw_ops.h"
#include "mac_if.h"

static struct mtk_hw_phy *
mtk_ge_rx_get_phy(struct mtk_hw_dev *dev,
	u32 *band_idx, u32 *mld_idx, u32 *mld_link_idx, u8 *mld_link)
{
	struct mtk_hw_phy *hw_phy = NULL;
	struct mtk_hw_sta *hw_sta = NULL;
	struct mtk_mld_sta_entry *mld_sta_entry = NULL;

	hw_sta = mtk_hwctrl_sta_entry_find(dev, *mld_idx);

	if (hw_sta) {
		mld_sta_entry = mtk_wsys_mld_sta_entry_get(hw_sta->mld_sta_idx);
		if (mld_sta_entry) {
			struct mtk_mld_bss_entry *mld_bss_entry =
				mld_sta_entry->mld_bss;
			struct mtk_hw_sta *link_sta =
				mld_sta_entry->link_sta[*band_idx];

			if (!mld_bss_entry || !link_sta)
				return NULL;

			dbg_mld_sta_rx_increase(mld_sta_entry, *band_idx);
			*mld_link_idx = link_sta->link_wcid;
			/*apply to master mld sta*/
			*band_idx = mld_sta_entry->setup_band;
			*mld_idx = mld_sta_entry->setup_wcid;
			*mld_link = true;
		}
	}

	/*input hw_phy used when queue can direct mapping to link*/
	hw_phy = mtk_hwctrl_phy_entry_find(dev, *band_idx);
	return hw_phy;
}

static int
mtk_ge_add_sta(struct mtk_hw_dev *dev, struct mtk_hw_bss *hw_bss,
	struct mtk_hw_sta *hw_sta, u8 sta_type, u32 mld_sta_idx)
{
	int ret = -EINVAL;

	if (!hw_bss || !hw_sta)
		goto err;

	hw_sta->bss = hw_bss;
	ret = mtk_wsys_register_sta(hw_sta);
	if (ret)
		goto err;

	hw_sta->sta_type = sta_type;
	hw_sta->mld_sta_idx = mld_sta_idx;
	ret = mtk_hwctrl_sta_alloc(dev, hw_sta, sta_type);
	if (ret)
		goto err;

	return 0;
err:
	dev_err(dev->dev, "%s(): hw_bss=%p, hw_sta=%p\n",
		__func__, hw_bss, hw_sta);
	return ret;
}

static int
mtk_ge_tx_kick(struct mtk_hw_dev *dev)
{
	int ret;

	ret = mtk_bus_tx_kick(dev->bus_trans);

	return ret;
}

static int
mtk_ge_remove_sta(struct mtk_hw_dev *dev,
	struct mtk_hw_bss *bss, struct mtk_hw_sta *sta)
{
	int ret = 0;

	ret = mtk_hwctrl_sta_free(dev, sta);
	if (!ret)
		mtk_wsys_unregister_sta(sta);

	return ret;
}

static int
mtk_ge_add_interface(struct mtk_hw_phy *phy, struct mtk_hw_bss *hw_bss,
	u8 *if_addr, struct mtk_bss_mld_cfg *mld_cfg)
{
	int ret;
	struct mtk_hw_dev *dev = phy->hw_dev;

	mutex_lock(&dev->mutex);
	/*enable group wcid */
	hw_bss->hw_phy = phy;
	memcpy(hw_bss->if_addr, if_addr, MAC_ADDR_LEN);
	memcpy(&hw_bss->mld_cfg, mld_cfg, sizeof(*mld_cfg));

	ret = mtk_wsys_register_bss(hw_bss);
	if (ret)
		goto out;

	ret = mtk_hwctrl_bss_alloc(dev, hw_bss);
	if (ret)
		goto out;

out:
	mutex_unlock(&dev->mutex);
	return ret;
}

static int
mtk_ge_remove_interface(struct mtk_hw_phy *phy, struct mtk_hw_bss *hw_bss)
{
	struct mtk_hw_dev *dev = phy->hw_dev;

	mutex_lock(&dev->mutex);
	mtk_hwctrl_bss_free(dev, hw_bss);
	mtk_wsys_unregister_bss(hw_bss);
	mutex_unlock(&dev->mutex);
	return 0;
}

static int
mtk_ge_update_interface(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss)
{
	return mtk_wsys_update_mld_bss_entry(dev, bss);
}

static int
mtk_ge_add_mld(struct mtk_hw_dev *dev, struct mtk_bss_mld_cfg *mld_cfg)
{
	int ret = -EINVAL;

	mutex_lock(&dev->mutex);

	if (mld_cfg->mld_group_idx != MLD_GROUP_NONE)
		ret = mtk_wsys_register_mld(dev, mld_cfg);

	mutex_unlock(&dev->mutex);
	return ret;
}

static int
mtk_ge_remove_mld(struct mtk_hw_dev *dev, u32 mld_group_idx)
{
	int ret = -EINVAL;

	mutex_lock(&dev->mutex);

	if (mld_group_idx != MLD_GROUP_NONE)
		ret = mtk_wsys_unregister_mld(dev, mld_group_idx);

	mutex_unlock(&dev->mutex);
	return ret;
}

static int
mtk_ge_mld_add_link(struct mtk_hw_dev *dev, struct mtk_hw_bss *hw_bss, u32 mld_group_idx)
{
	int ret = -EINVAL;

	mutex_lock(&dev->mutex);

	if (mld_group_idx != MLD_GROUP_NONE)
		ret = mtk_wsys_mld_register_bss(dev, hw_bss, mld_group_idx);

	mutex_unlock(&dev->mutex);
	return ret;
}

static int
mtk_ge_mld_remove_link(struct mtk_hw_dev *dev, struct mtk_hw_bss *hw_bss)
{
	int ret = -EINVAL;

	mutex_lock(&dev->mutex);

	if (hw_bss->mld_bss_entry)
		ret = mtk_wsys_mld_unregister_bss(dev, hw_bss);

	mutex_unlock(&dev->mutex);
	return ret;
}

static int
mtk_ge_start(struct mtk_hw_dev *dev)
{
	int ret = 0;

	if (!test_bit(HWIFI_STATE_RUNNING, &dev->state)) {
		mtk_bus_stop_device(dev->bus_trans);
		ret = mtk_tk_init(dev);
		if (ret)
			goto err_tk_init;

		ret = mtk_bus_preinit_device(dev->bus_trans);
		if (ret)
			goto err_bus_preinit;

		ret = mtk_bus_init_device(dev->bus_trans);
		if (ret)
			goto err_bus_init;

		ret = mtk_bus_start_device(dev->bus_trans);
		if (ret)
			goto err_bus_start;

		ret = mtk_mcu_start_device(dev);
		if (ret)
			goto err_mcu_start;

		ret = mtk_bus_init_after_fwdl(dev->bus_trans);
		if (ret)
			goto err_init_after_fwdl;

		mutex_lock(&dev->mutex);
		set_bit(HWIFI_STATE_RUNNING, &dev->state);
		mutex_unlock(&dev->mutex);
	}

	return ret;
err_init_after_fwdl:
	mtk_mcu_stop_device(dev);
err_mcu_start:
	mtk_bus_stop_device(dev->bus_trans);
err_bus_start:
	mtk_bus_exit_device(dev->bus_trans);
err_bus_init:
err_bus_preinit:
	mtk_tk_exit(dev);
err_tk_init:
	return ret;
}

static int
mtk_ge_stop(struct mtk_hw_dev *dev)
{
	mutex_lock(&dev->mutex);
	clear_bit(HWIFI_STATE_RUNNING, &dev->state);
	mutex_unlock(&dev->mutex);
	mtk_mcu_stop_device(dev);
	mtk_bus_stop_device(dev->bus_trans);
	mtk_bus_exit_device(dev->bus_trans);
	mtk_tk_exit(dev);
	return 0;
}

static int
mtk_ge_init_txq(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss,
	struct mtk_hw_sta *sta, u8 tid, struct mtk_hw_txq *txq)
{
	txq->sta = sta;
	return mtk_bus_get_txq(dev->bus_trans, bss->hw_phy->band_idx, tid,
			       &txq->txq, &txq->txq_trans);
}

static int
mtk_ge_mcu_tx(struct mtk_hw_dev *dev, struct mtk_mcu_txblk *mcu_txblk)
{
	return mtk_mcu_tx(dev, mcu_txblk);
}

static void
mtk_ge_mcu_get_fw_info(struct mtk_hw_dev *dev, int mcu_type, char *fw_ver, char *build_date,
			char *fw_ver_long)
{
	mtk_mcu_get_fw_info(dev, mcu_type, fw_ver, build_date, fw_ver_long);
}

static void
mtk_ge_mcu_set_fw_mode(struct mtk_hw_dev *dev, int mcu_type, u8 fw_mode)
{
	mtk_mcu_set_fw_mode(dev, mcu_type, fw_mode);
}

static int
mtk_ge_get_idrm_high(struct mtk_hw_dev *dev, struct mtk_idr_mgmt *idrm)
{
	int ret;

	spin_lock_bh(&idrm->lock);
	ret = idrm->high;
	spin_unlock_bh(&idrm->lock);

	return ret;
}

static int
mtk_ge_get_idrm_low(struct mtk_hw_dev *dev, struct mtk_idr_mgmt *idrm)
{
	int ret;

	spin_lock_bh(&idrm->lock);
	ret = idrm->low;
	spin_unlock_bh(&idrm->lock);

	return ret;
}

static int
mtk_ge_update_idrm(struct mtk_hw_dev *dev, struct mtk_idr_mgmt *idrm, u32 low, u32 high)
{
	spin_lock_bh(&idrm->lock);
	idrm->low  = low;
	idrm->high  = high;
	spin_unlock_bh(&idrm->lock);
	return 0;
}

static u32
mtk_ge_get_free_sta_pool_num(struct mtk_hw_dev *dev)
{
	return mtk_wsys_free_uwtbl_num_get(dev);
}

static int
mtk_ge_change_setup_link_sta(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta)
{
	int ret = 0;

	ret = mtk_wsys_mld_sta_entry_change_setup_link(dev, sta);
	if (ret)
		goto err;

	return ret;
err:
	dev_err(dev->dev, "%s(): change setup link sta err %d\n", __func__, ret);
	return ret;
}

static int
mtk_ge_get_mld_id(struct mtk_hw_sta *hw_sta, u32 *pri_id, u32 *sec_id)
{
	return mtk_wsys_mld_sta_entry_mld_id_get(hw_sta, pri_id, sec_id);
}

static const struct mtk_mac_ops mac_ops = {
	.start = mtk_ge_start,
	.stop = mtk_ge_stop,
	.add_interface = mtk_ge_add_interface,
	.remove_interface = mtk_ge_remove_interface,
	.add_mld = mtk_ge_add_mld,
	.remove_mld = mtk_ge_remove_mld,
	.mld_add_link = mtk_ge_mld_add_link,
	.mld_remove_link = mtk_ge_mld_remove_link,
	.add_sta = mtk_ge_add_sta,
	.tx_check_resource = mtk_ge_tx_check_resource,	/* connac */
	.tx_data = mtk_ge_tx_data,			/* connac */
	.tx_kick = mtk_ge_tx_kick,
	.remove_sta = mtk_ge_remove_sta,
	.mcu_tx = mtk_ge_mcu_tx,
	.add_phy = mtk_ge_add_phy,
	.remove_phy = mtk_ge_remove_phy,
	.init_txq = mtk_ge_init_txq,
	.rx_get_phy = mtk_ge_rx_get_phy,
	.update_interface = mtk_ge_update_interface,
	.set_dma_tk = mtk_ge_set_dma_tk,		/* connac */
	.set_dma_sw_rxq = mtk_ge_set_dma_sw_rxq,	/* connac */
	.hw_reset = mtk_ge_hw_reset,			/* connac */
	.set_traffic = mtk_ge_set_traffic,		/* connac */
	.get_pc_value = mtk_ge_get_pc_value,		/* connac */
	.get_idrm_high = mtk_ge_get_idrm_high,
	.get_idrm_low = mtk_ge_get_idrm_low,
	.update_idrm = mtk_ge_update_idrm,
	.get_hw_bss = mtk_ge_get_hw_bss,		/* connac */
	.get_fw_info = mtk_ge_mcu_get_fw_info,
	.set_fw_mode = mtk_ge_mcu_set_fw_mode,
	.get_free_sta_pool_num = mtk_ge_get_free_sta_pool_num,
	.change_setup_link_sta = mtk_ge_change_setup_link_sta,
	.get_mld_id = mtk_ge_get_mld_id,
};

int
mtk_hdev_ops_init(struct mtk_hw_dev *dev)
{
	dev->dev_ops = &mac_ops;
	return 0;
}
