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

#include "chips.h"
#include "bus.h"
#include "main.h"

#define range_dump(_s, _name, _r) { \
	if (strlen(_name) < 8) \
		seq_printf(_s, "%s\t\t: %d - %d\n", _name, _r.start, _r.end);\
	else \
		seq_printf(_s, "%s\t: %d - %d\n", _name, _r.start, _r.end);\
}

static inline int
mtk_chip_hwres_radio_cap_dump(struct seq_file *s, struct mtk_hwres_radio_cap *radio_cap)
{
	seq_printf(s, "phy type\t: %x\n", radio_cap->info->type);
	seq_printf(s, "phy cap\t\t: %d\n", radio_cap->info->cap);
	range_dump(s, "omac", radio_cap->omac);
	range_dump(s, "ext_omac", radio_cap->ext_omac);
	range_dump(s, "rept_omac", radio_cap->rept_omac);
	range_dump(s, "wmm_set", radio_cap->wmm_set);
	return 0;
}

static inline int
mtk_chip_hwres_cap_dump(struct seq_file *s, struct mtk_hwres_cap *hwres)
{
	int i = 0;

	seq_puts(s, "==========hwres cap==========\n");
	range_dump(s, "uwtbl", hwres->uwtbl);
	range_dump(s, "group", hwres->group);
	range_dump(s, "tx_token", hwres->tx_token);
	range_dump(s, "rx_token", hwres->rx_token);
	range_dump(s, "bss", hwres->bss);
	range_dump(s, "mld_addr", hwres->mld_addr);
	range_dump(s, "link_addr", hwres->link_addr);
	range_dump(s, "mld_remap", hwres->mld_remap);

	seq_printf(s, "radio num\t: %d\n", hwres->radio_num);

	for (i = 0; i < hwres->radio_num; i++) {
		struct mtk_hwres_radio_cap *radio_cap = &hwres->radio_cap[i];

		seq_printf(s, "==========link idx %d ==========\n", radio_cap->band_idx);
		mtk_chip_hwres_radio_cap_dump(s, radio_cap);
	}

	return 0;
}

static int
mtk_chip_bus_cfg_dump(struct seq_file *s,
	struct mtk_bus_driver *bus_drv, struct mtk_bus_cfg *bus_cfg)
{
	seq_puts(s, "==========chip bus config==========\n");
	seq_printf(s, "name\t\t: %s\n", bus_cfg->name);
	seq_printf(s, "type\t\t: %d\n", bus_cfg->bus_type);
	seq_printf(s, "drv_size\t: %d\n", bus_cfg->bus_drv_sz);
	seq_printf(s, "master\t\t: %s\n", bus_cfg->ms_type ? "SLAVE" : "MASTER");
	seq_printf(s, "id_table\t: %p\n", bus_cfg->id_table);
	seq_printf(s, "bus_ops\t\t: %p\n", bus_cfg->bus_ops);
	seq_printf(s, "profile\t\t: %p\n", bus_cfg->profile);
	mtk_bus_dump_profile(bus_drv, bus_cfg->profile, s);
	return 0;
}

static inline int
mtk_chip_mcu_info_dump(struct seq_file *s, struct mtk_chip_mcu_info *mcu_infos)
{
	int i = 0;

	seq_puts(s, "==========mcu info==========\n");

	for (i = 0; i < MCU_MAX; i++) {
		struct mtk_chip_mcu_info *info = &mcu_infos[i];

		seq_printf(s, "index\t\t: %d\n", i);
		seq_printf(s, "fw\t\t: %s\n", info->fw[info->fw_mode]);
		seq_printf(s, "fw_mode\t\t: %u\n", info->fw_mode);
		seq_printf(s, "build_time\t: %.15s\n", info->build_date);
		seq_printf(s, "rom_patch\t: %s\n", info->rom_patch);
		seq_printf(s, "opt\t\t: 0x%x\n", info->opt);

	}

	return 0;
}

static int
mtk_chip_debugfs(struct seq_file *s, void *data)
{
	struct mtk_idr_entry *sys_idx = (struct mtk_idr_entry *)s->private;
	struct mtk_chip *chip = (struct mtk_chip *)sys_idx->data;
	struct mtk_chip_drv *chip_drv = chip->drv;
	struct mtk_chip_hw_cap *hw_cap = chip_drv->hw_caps;

	seq_puts(s, "==========chip info==========\n");

	seq_printf(s, "chip id\t\t: %d\n", chip->sid.idx);
	seq_printf(s, "device id\t: %x\n", chip_drv->device_id);
	seq_printf(s, "interface_type\t: %d\n", chip_drv->interface_type);
	mtk_chip_bus_cfg_dump(s, chip->bus_drv, &chip_drv->bus_cfg);

	if (!hw_cap)
		goto end;

	seq_puts(s, "==========hw_cap==========\n");
	seq_printf(s, "dev_size\t: %zd\n", hw_cap->dev_size);
	seq_printf(s, "eeprom_size\t: %d\n", hw_cap->eeprom_size);
	seq_printf(s, "mtxd_size\t: %d\n", hw_cap->mtxd_sz);
	seq_printf(s, "mac_cap\t: 0x%lx\n", hw_cap->mac_cap);
	mtk_chip_mcu_info_dump(s, hw_cap->mcu_infos);
	mtk_chip_hwres_cap_dump(s, hw_cap->hwres);
end:
	return 0;
}

/**
 * Sanity check for chip driver
 *
 * @param *chip_drv chip driver
 * @retval 0 valid chip drv
 * @retval -EINVAL invalid chip drv
 */
static int
mtk_chip_drv_sanity(struct mtk_chip_drv *chip_drv)
{
	if (!chip_drv->bus_cfg.id_table)
		goto err;
	if (!chip_drv->bus_cfg.bus_ops)
		goto err;
	if (!chip_drv->bus_cfg.profile)
		goto err;

	return 0;
err:
	return -EINVAL;
}

static void
mtk_chip_clear(struct mtk_chip_mgmt *chip_mgmt, struct mtk_chip *chip)
{
	if (chip) {
		mtk_bus_unregister_chip(chip);
		mtk_idr_entry_unregister(&chip_mgmt->idrm, &chip->sid);
		kfree(chip);
	}
}

/**
 * Export function for register chip driver
 *
 * @param *chip_drv chip driver
 * @retval 0 valid chip drv
 * @retval -EINVAL invalid chip drv
 */
int
mtk_chip_register(struct mtk_chip_drv *chip_drv)
{
	struct mtk_chip_mgmt *chip_mgmt = mtk_hwifi_get_chip_mgmt();
	struct mtk_chip *chip = NULL;
	struct mtk_bus_cfg *bus_cfg = &chip_drv->bus_cfg;
	int sz;
	int ret;

	/*sanity check*/
	ret = mtk_chip_drv_sanity(chip_drv);
	if (ret)
		goto err;

	/*allocate chip + bus driver (pci_driver/usb_driver...)*/
	sz = ALIGN(sizeof(*chip), NETDEV_ALIGN) + bus_cfg->bus_drv_sz;
	/*allocate chip memory*/
	chip = kzalloc(sz, GFP_KERNEL);

	if (!chip)
		goto err;

	/*assign chip value*/
	chip->drv = chip_drv;

	/*add to chip list*/
	ret = mtk_idr_entry_register(&chip_mgmt->idrm, &chip->sid, chip);
	if (ret)
		goto err;
	/*dbgfs*/
	ret = mtk_idr_entry_register_debugfs(&chip_mgmt->idrm, &chip->sid,
		"chip", mtk_chip_debugfs);

	if (ret)
		goto err;

	/*get bus driver by bus type*/
	ret = mtk_bus_register_chip(chip);
	if (ret)
		goto err;
	return 0;
err:
	mtk_chip_clear(chip_mgmt, chip);
	return -ENOMEM;
}
EXPORT_SYMBOL(mtk_chip_register);

/**
 * Export function for unregister chip driver
 *
 * @param *chip_drv chip driver
 */
void
mtk_chip_unregister(struct mtk_chip_drv *chip_drv)
{
	struct mtk_chip_mgmt *chip_mgmt = mtk_hwifi_get_chip_mgmt();
	struct mtk_chip *cur, *chip = NULL;
	u32 id;

	mutex_lock(&chip_mgmt->mutex);
	/*search chip*/
	mtk_idr_for_each_entry(&chip_mgmt->idrm, cur, id) {
		if (cur->drv == chip_drv)
			chip = cur;
	}
	mutex_unlock(&chip_mgmt->mutex);

	if (chip)
		mtk_chip_clear(chip_mgmt, chip);
}
EXPORT_SYMBOL(mtk_chip_unregister);

/**
 * @brief Initial chip sub module
 *  prepare chip_mgmt
 *
 * @param *drv global hwifi driver
 * @param *chip_mgmt chip mgmt
 * @retval 0 init success
 * @retval -ENOMEM alloc resouce fail
 */
int
mtk_chip_init(struct mtk_hwifi_drv *drv, struct mtk_chip_mgmt *chip_mgmt)
{
	int ret;

	ret = mtk_idrm_init(&chip_mgmt->idrm, "chips", drv->debugfs_dir, 0, WSYS_CAR_MAX);
	if (ret)
		return -ENOMEM;

	mutex_init(&chip_mgmt->mutex);
	return 0;
}

/**
 * function for chip submodule exit
 *
 * @param *chip_mgmt chip mgmt
 */
void
mtk_chip_exit(struct mtk_chip_mgmt *chip_mgmt)
{
	struct mtk_chip *chip;
	u32 id;

	mtk_idr_for_each_entry(&chip_mgmt->idrm, chip, id) {
		mtk_chip_clear(chip_mgmt, chip);
	}

	mtk_idrm_exit(&chip_mgmt->idrm);
	mutex_init(&chip_mgmt->mutex);
}
