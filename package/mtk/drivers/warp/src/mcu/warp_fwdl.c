/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: wifi_offload
	warp_fwdl.c
*/
#include "warp.h"
#include "warp_hw.h"
#include <linux/firmware.h>

#define WOCPU_DATA_DEV_NODE "mediatek,wocpu_data"
#define WOCPU_BOOT_DEV_NODE "mediatek,wocpu_boot"
#define FW_DL_TIMEOUT		((3000 * HZ)/1000)	/* system ticks -- 100 ms*/

struct fw_dl_buf {
	u8 *img_ptr;
	u32 img_dest_addr;
	u32 img_size;
	u8 feature_set;
	/* decompress features are supported in CONNAC */
	u32 decomp_crc;
	u32 decomp_img_size;
	u32 decomp_block_size;
};


struct wo_fwdl_host_mode_ctrl *wo_host_exist = NULL;
EXPORT_SYMBOL(wo_host_exist);

static void img_get_8bit(u8 *dest, u8 **src, u32 cnt)
{
	u32 i;

	for (i = 0; i < cnt; i++) {
		dest[i] = *(*src + i);
	}

	*src += cnt;
}

static void img_get_32bit(u32 *dest, u8 **src, u32 cnt)
{
	u32 i;

	for (i = 0; i < cnt; i++) {
		dest[i] = (u32)((*(*src + (i * 4) + 3) << 24) |
						   (*(*src + (i * 4) + 2) << 16) |
						   (*(*src + (i * 4) + 1) <<  8) |
						   (*(*src + (i * 4))     <<  0));
	}

	*src += cnt * 4;
}

static int get_region_info(struct fwdl_region *region, const char *compat, u8 dts_idx)
{
	struct device_node *node;
	struct resource res;
	int rc;

	node = of_find_compatible_node(NULL, NULL, compat);
	if (!node) {
		warp_dbg(WARP_DBG_ERR,
		"%s(!node): wed_idx = %d, base_addr = 0x%p, size = 0x%x, "
		"res.start = 0x%llx, shared:%d\n",
		__func__, dts_idx, region->base_addr, region->size,
		region->base_addr_pa, region->shared);
		return -1;
	}
	rc = of_address_to_resource(node, dts_idx, &res);
	if (rc){
		warp_dbg(WARP_DBG_ERR,
		"%s(rc = %d): wed_idx = %d, base_addr = 0x%p, size = 0x%x, "
		"res.start = 0x%llx, shared:%d\n",
		__func__, rc, dts_idx, region->base_addr, region->size,
		region->base_addr_pa, region->shared);
		return -1;
	}
	region->base_addr = ioremap(res.start, resource_size(&res));
	region->base_addr_pa = res.start;
	region->size = resource_size(&res);
	of_property_read_u32_index(node, "shared", 0, &region->shared);

	warp_dbg(WARP_DBG_INF,
		"%s(): wed_idx = %d, base_addr = 0x%p, size = 0x%x, "
		"res.start = 0x%llx, shared:%d\n",
		__func__, dts_idx, region->base_addr, region->size,
		region->base_addr_pa, region->shared);

	return 0;
}

static int warp_fwdl_ctrl_init_mcu_mode(struct warp_fwdl_ctrl *ctrl, u8 wed_idx)
{
	int ret;
	u8 dts_idx = wed_idx;

	/* for backward compatible */
	warp_get_dts_idx(&dts_idx);

	ret = get_region_info(&ctrl->region[0], warp_get_wo_emi_node(wed_idx), 0);
	if (ret) {
		warp_dbg(WARP_DBG_OFF, "warp_get_wo_emi_node fail\n");
		return ret;
	}

	ret = get_region_info(&ctrl->region[1], warp_get_wo_ilm_node(wed_idx), dts_idx);
	if (ret) {
		warp_dbg(WARP_DBG_OFF, "warp_get_wo_ilm_node fail\n");
		return ret;
	}

	ret = get_region_info(&ctrl->region[2], WOCPU_DATA_DEV_NODE, 0);
	if (ret) {
		warp_dbg(WARP_DBG_OFF, "WOCPU_DATA_DEV_NODE fail\n");
		return ret;
	}

	ret = get_region_info(&ctrl->boot_setting, WOCPU_BOOT_DEV_NODE, wed_idx);
	if (ret)
		warp_dbg(WARP_DBG_OFF, "WOCPU_BOOT_DEV_NODE fail\n");

	return ret;
}

static int warp_fwdl_ctrl_init_host_mode(struct warp_fwdl_ctrl *ctrl, u8 wed_idx)
{
	ctrl->wo_host_exist = &wo_host_exist;
	return 0;
}

static int warp_fwdl_write_wed_idx_mcu_mode(struct warp_fwdl_ctrl *ctrl,
		struct wed_entry *wed, u8 wed_idx)
{
	warp_dummy_cr_set(wed, WED_DUMMY_CR_FWDL, wed_idx + 1);
	return 0;
}

static int warp_fwdl_write_wed_idx_host_mode(struct warp_fwdl_ctrl *ctrl,
		struct wed_entry *wed, u8 wed_idx)
{
	int ret = 0;
	struct wo_fwdl_host_mode_ctrl *wo_host_exist = NULL;;

	if (ctrl == NULL) {
		ret = -1;
		warp_dbg(WARP_DBG_OFF, "%s(): invalid ctrl pointer [%p]\n", __func__,
			 ctrl);
		goto err_out;
	}

	if (*ctrl->wo_host_exist == NULL) {
		ret = -1;
		warp_dbg(WARP_DBG_OFF, "%s(): invalid host pointer [%p]\n", __func__,
			 *ctrl->wo_host_exist);
		goto err_out;
	}

	wo_host_exist = *ctrl->wo_host_exist;
	wo_host_exist->wed_idx = wed_idx;
	warp_dbg(WARP_DBG_OFF, "%s(): wed_idx = %d\n", __func__,
		 wo_host_exist->wed_idx);

err_out:
	return ret;
}

static int warp_fw_init(struct warp_entry *warp)
{
	const struct firmware *fw_entry;
	int ret = -1;
	int print_ret = 0;
	struct platform_device *pdev = warp->wed.pdev;
	struct warp_fwdl_ctrl *ctrl = &warp->woif.fwdl_ctrl;
	char bin_name[100] = {0}, chip_name[7] = {0};

#ifdef WARP_CHIP_SET
	print_ret = snprintf(chip_name, strlen(WARP_CHIP_SET)+1, "%s", WARP_CHIP_SET);
	if (print_ret)
		warp_dbg(WARP_DBG_ERR, "%s:print error\n", __func__);

	if (warp->wed.ver == 2 && warp->wed.sub_ver == 0 ) {
		memcpy(bin_name, chip_name, strlen(WARP_CHIP_SET));
		print_ret = snprintf(&bin_name[strlen(WARP_CHIP_SET)],
				sizeof(bin_name)-strlen(WARP_CHIP_SET),
				"_WOCPU%d_RAM_CODE_release.bin", warp->idx);
		if (print_ret)
			warp_dbg(WARP_DBG_ERR, "%s:print error\n", __func__);
	} else {
		memcpy(bin_name, &chip_name[2], strlen(WARP_CHIP_SET)-2);
		print_ret = snprintf(&bin_name[(strlen(WARP_CHIP_SET)-2)],
				sizeof(bin_name)-strlen(WARP_CHIP_SET)-2,
				"_WOCPU%d_RAM_CODE_release.bin", warp->idx);
		if (print_ret)
			warp_dbg(WARP_DBG_ERR, "%s:print error\n", __func__);
	}
#else
	goto err_out;
#endif

	warp_dbg(WARP_DBG_INF,
		"loading %s from /lib/firmware/!\n", bin_name);

	/*  surpress warning message */
	if (request_firmware_direct(&fw_entry, bin_name, &pdev->dev) != 0) {
#ifdef CONFIG_WARP_WO_EMBEDDED_LOAD
		warp_dbg(WARP_DBG_OFF,
			"%s:fw not available(/lib/firmware/%s), loading embedded version!\n",
			__func__, bin_name);

		ctrl->bin_size = warp_get_wo_bin_size(warp->idx);
		ctrl->bin_mode = WO_FW_EMBEDDED;
#else
		warp_dbg(WARP_DBG_OFF,
				 "%s:fw not available(/lib/firmware/%s)!\n", __func__, bin_name);

		goto err_out;
#endif	/* CONFIG_WARP_WO_EMBEDDED_LOAD */
	} else {
		ctrl->bin_size = fw_entry->size;
		ctrl->bin_mode = WO_FW_BIN;
	}

	ctrl->bin_ptr = (u8 *)kmalloc(ctrl->bin_size, GFP_KERNEL);
	if (ctrl->bin_ptr) {
		memset(ctrl->bin_ptr, 0, ctrl->bin_size);

#ifdef CONFIG_WARP_WO_EMBEDDED_LOAD
		if (ctrl->bin_mode == WO_FW_EMBEDDED) {
			memcpy(ctrl->bin_ptr,
				warp_get_wo_bin_ptr(warp->idx), ctrl->bin_size);
		} else
#endif	/* CONFIG_WARP_WO_EMBEDDED_LOAD */
		{
			memcpy(ctrl->bin_ptr, fw_entry->data, ctrl->bin_size);
			release_firmware(fw_entry);
		}

		ret = 0;
	} else {
		warp_dbg(WARP_DBG_ERR,
			"%s:allocate memory for firmware failed!\n", __func__);
		goto err_out;
	}

err_out:
	return ret;
}

static int warp_fwdl_mcu_mode(struct warp_fwdl_ctrl *ctrl, struct wed_entry *wed, u8 wed_idx)
{
	u8 *img_ptr = NULL, fw_info_str[50] = {0};
	u8 *img_ptr_tail = NULL;
	struct fw_info_t *fw_info = &ctrl->fw_info;
	u32 num_of_region = 0, i, offset = 0;
	static u8 shared_dl_flag[MAX_REGION_SIZE] = {0};

	warp_dbg(WARP_DBG_INF, "%s(): mcu mode, need fwdl\n", __func__);

	if (warp_fw_init(wed->warp) == 0) {
		img_ptr_tail = ctrl->bin_ptr + ctrl->bin_size;

		img_ptr = img_ptr_tail - FW_V3_COMMON_TAILER_SIZE;

		img_get_8bit(&fw_info->chip_id, &img_ptr, sizeof(fw_info->chip_id));
		img_get_8bit(&fw_info->eco_ver, &img_ptr, sizeof(fw_info->eco_ver));
		img_get_8bit(&fw_info->num_of_region, &img_ptr, sizeof(fw_info->num_of_region));
		img_get_8bit(&fw_info->format_ver, &img_ptr, sizeof(fw_info->format_ver));
		img_get_8bit(&fw_info->format_flag, &img_ptr, sizeof(fw_info->format_flag));
		img_ptr += 2; /* bypass reserved field */
		img_get_8bit(fw_info->ram_ver, &img_ptr, sizeof(fw_info->ram_ver));
		img_get_8bit(fw_info->ram_built_date, &img_ptr, sizeof(fw_info->ram_built_date));
		img_get_32bit(&fw_info->crc, &img_ptr, 1);

		num_of_region = fw_info->num_of_region;

		warp_dbg(WARP_DBG_OFF, "########## WO Firmware ##########\n");
		warp_dbg(WARP_DBG_OFF, "Chip ID: 0x%04x\n", fw_info->chip_id);
		warp_dbg(WARP_DBG_OFF, "ECO version:%d\n", fw_info->eco_ver);
		fw_info_str[0] = '\0';
		for (i = 0 ; i < sizeof(fw_info->ram_ver) ; i++)
			strncat(fw_info_str, fw_info->ram_ver+i, sizeof(u8));
		warp_dbg(WARP_DBG_OFF, "Version: %s\n", fw_info_str);
		fw_info_str[0] = '\0';
		for (i = 0 ; i < sizeof(fw_info->ram_built_date) ; i++)
			strncat(fw_info_str, fw_info->ram_built_date+i, sizeof(u8));
		warp_dbg(WARP_DBG_OFF, "Build date: %s\n", fw_info_str);
		warp_dbg(WARP_DBG_OFF, "Total region:%d\n", fw_info->num_of_region);
		warp_dbg(WARP_DBG_OFF, "#################################\n");

		for (i = 0; i < num_of_region; i++) {
			struct fw_dl_buf region_info = {0};
			struct fwdl_region *region = NULL;
			u32 dl_addr = 0, dl_len = 0, decomp_crc= 0, decomp_len = 0, decomp_block_size = 0;
			u8 dl_feature_set = 0;
			u8 j = 0;

			warp_dbg(WARP_DBG_OFF, "Parsing tailer region %d\n", i);

			img_ptr = img_ptr_tail - FW_V3_COMMON_TAILER_SIZE - (num_of_region - i) * FW_V3_REGION_TAILER_SIZE;
			img_get_32bit(&decomp_crc, &img_ptr, 1);
			warp_dbg(WARP_DBG_INF, "\tDecomp crc: 0x%x\n", decomp_crc);

			img_get_32bit(&decomp_len, &img_ptr, 1);
			warp_dbg(WARP_DBG_INF, "\tDecomp size: 0x%x\n", decomp_len);

			img_get_32bit(&decomp_block_size, &img_ptr, 1);
			warp_dbg(WARP_DBG_INF, "\tDecomp block size: 0x%x\n", decomp_block_size);

			img_ptr += 4; /* bypass reserved field */

			img_get_32bit(&dl_addr, &img_ptr, 1);
			warp_dbg(WARP_DBG_OFF, "\tTarget address: 0x%x\n", dl_addr);

			img_get_32bit(&dl_len, &img_ptr, 1);
			warp_dbg(WARP_DBG_OFF, "\tDownload size: %d\n", dl_len);

			if (dl_len > MAX_FWDL_SIZE) {
				warp_dbg(WARP_DBG_ERR, "\terror:wrong fwdl size:%d\n", dl_len);
				return -1;
			}
			img_get_8bit(&dl_feature_set, &img_ptr, 1);

			for (j = 0; j < MAX_REGION_SIZE; j++) {
				region = &ctrl->region[j];

				if (!region->base_addr) {
					break;
				} else if (dl_addr == region->base_addr_pa) {
					if (!region->shared) {
						memcpy(region->base_addr, ctrl->bin_ptr + offset, dl_len);
					} else {
						if (!shared_dl_flag[i]) {
							memcpy(region->base_addr, ctrl->bin_ptr + offset, dl_len);
							shared_dl_flag[i] = true;
						}
					}
				}
			}

			region_info.img_dest_addr = dl_addr;
			region_info.img_size = dl_len;
			region_info.feature_set = dl_feature_set;
			region_info.decomp_crc = decomp_crc;
			region_info.decomp_img_size = decomp_len;
			region_info.decomp_block_size = decomp_block_size;
			region_info.img_ptr = ctrl->bin_ptr + offset;
			offset += dl_len;
		}

		warp_fwdl_write_start_address(ctrl, ctrl->region[0].base_addr_pa, wed_idx);
		warp_fwdl_reset(ctrl, 1, wed_idx);
	} else
		warp_dbg(WARP_DBG_ERR, "[%s]Prepare firmware failed!\n", __func__);
	
	return 0;
}

static int warp_fwdl_host_mode(struct warp_fwdl_ctrl *ctrl, struct wed_entry *wed, u8 wed_idx)
{
	warp_dbg(WARP_DBG_OFF, "%s(): ko mode, don't need fwdl\n", __func__);
	return 0;
}

static int warp_fwdl_ready_check_mcu_mode(struct warp_fwdl_ctrl *ctrl,
		struct wed_entry *wed)
{
	int32_t ret = 0;
	u32 v = 0;
	unsigned long timeout = jiffies+FW_DL_TIMEOUT;
	warp_dbg(WARP_DBG_INF, "%s(): waiting for wocpu\n", __func__);
	do {
		warp_dummy_cr_get(wed, WED_DUMMY_CR_FWDL, &v);
	} while (v != 0 && !time_after(jiffies, timeout));

	if (v != 0) {
		warp_dbg(WARP_DBG_OFF, "\033[0;32;31m%s(): WO FW DL failed!\033[m\n", __func__);
		ret = -1;
	} else {
		warp_dbg(WARP_DBG_OFF, "%s(): wocpu is ready\n", __func__);
	}

	if (ctrl->bin_ptr) {
		kfree(ctrl->bin_ptr);
		ctrl->bin_ptr = NULL;
	}
	/*unmap */
	if (ctrl->region[0].base_addr) {
		iounmap(ctrl->region[0].base_addr);
		ctrl->region[0].base_addr = NULL;
	}

	if (ctrl->region[1].base_addr) {
		iounmap(ctrl->region[1].base_addr);
		ctrl->region[1].base_addr = NULL;
	}

	if (ctrl->region[2].base_addr) {
		iounmap(ctrl->region[2].base_addr);
		ctrl->region[2].base_addr = NULL;
	}
	return ret;
}

static int warp_fwdl_ready_check_host_mode(struct warp_fwdl_ctrl *ctrl,
		struct wed_entry *wed)
{
	struct wo_fwdl_host_mode_ctrl *wo_host_exist = *ctrl->wo_host_exist;
	complete(&wo_host_exist->fwdl_warp_notify_wo);
	wait_for_completion(&wo_host_exist->fwdl_wo_notify_warp);
	if (wo_host_exist->wed_idx != UNDEFINE_WED_IDX) {
		warp_dbg(WARP_DBG_OFF, "%s(): wed_idx should be 0xff instead of %d\n", __func__,
			 wo_host_exist->wed_idx);
		return -1;
	}
	warp_dbg(WARP_DBG_OFF, "%s(): wo get wed_idx, fwdl ready\n", __func__);

	return 0;
}

static int warp_fwdl_ctrl_deinit_mcu_mode(struct warp_fwdl_ctrl *ctrl) {

	memset(ctrl, 0, sizeof(struct warp_fwdl_ctrl));
	return 0;
}


static int warp_fwdl_ctrl_deinit_host_mode(struct warp_fwdl_ctrl *ctrl) {

	ctrl->wo_host_exist = NULL;
	return 0;
}

static int warp_fwdl_done_check_mcu_mode(struct warp_fwdl_ctrl *ctrl) {

	return (ctrl->boot_setting.base_addr != NULL) ? WARP_ALREADY_DONE_STATUS : 0;
}


static int warp_fwdl_done_check_host_mode(struct warp_fwdl_ctrl *ctrl) {

	return (ctrl->wo_host_exist != NULL) ? WARP_ALREADY_DONE_STATUS : 0;
}

static int warp_fwdl_clear_mcu_mode(struct warp_fwdl_ctrl *ctrl) {
	if (ctrl->boot_setting.base_addr) {
		iounmap(ctrl->boot_setting.base_addr);
	}
	ctrl->boot_setting.base_addr = NULL;
	return 0;
}


static int warp_fwdl_clear_host_mode(struct warp_fwdl_ctrl *ctrl) {

	ctrl->wo_host_exist = NULL;
	return 0;
}





struct warp_fwdl_ops host_mode_fwdl_ops = {
	.init = warp_fwdl_ctrl_init_host_mode,
	.write_wed_idx = warp_fwdl_write_wed_idx_host_mode,
	.do_fwdl = warp_fwdl_host_mode,
	.fwdl_ready_check = warp_fwdl_ready_check_host_mode,
	.deinit = warp_fwdl_ctrl_deinit_host_mode,
	.fwdl_done_check = warp_fwdl_done_check_host_mode,
	.fwdl_clear = warp_fwdl_clear_host_mode,
};

struct warp_fwdl_ops mcu_mode_fwdl_ops = {
	.init = warp_fwdl_ctrl_init_mcu_mode,
	.write_wed_idx = warp_fwdl_write_wed_idx_mcu_mode,
	.do_fwdl = warp_fwdl_mcu_mode,
	.fwdl_ready_check = warp_fwdl_ready_check_mcu_mode,
	.deinit = warp_fwdl_ctrl_deinit_mcu_mode,
	.fwdl_done_check = warp_fwdl_done_check_mcu_mode,
	.fwdl_clear = warp_fwdl_clear_mcu_mode,
};

struct warp_fwdl_ops *fwdl_ops;



int warp_fwdl(struct warp_entry *warp)
{
	struct woif_entry *woif = &warp->woif;
	struct warp_fwdl_ctrl *ctrl = &woif->fwdl_ctrl;
	int ret = 0;

	if (IS_WO_HOST_MODE)
		fwdl_ops = &host_mode_fwdl_ops;
	else
		fwdl_ops = &mcu_mode_fwdl_ops;

	ret = fwdl_ops->fwdl_done_check(ctrl);
	if (ret != 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): fwdl ctrl done check, no need to do again\n", __func__);
		goto end;
	}

	ret = fwdl_ops->init(ctrl, warp->idx);
	if (ret != 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): fwdl ctrl init fail\n", __func__);
		goto end;
	}

	ret = fwdl_ops->write_wed_idx(ctrl, &warp->wed, warp->idx);
	if (ret != 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): write wed_idx fail\n", __func__);
		goto end;
	}

	ret = fwdl_ops->do_fwdl(ctrl, &warp->wed, warp->idx);
	if (ret != 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): fwdl fail\n", __func__);
		goto end;
	}

	ret = fwdl_ops->fwdl_ready_check(ctrl, &warp->wed);
	if (ret != 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): fwdl_ready_check fail\n", __func__);
		goto end;
	}
end:
	return ret;
}

int warp_fwdl_clear(struct warp_entry *warp)
{
	struct woif_entry *woif = &warp->woif;
	struct warp_fwdl_ctrl *ctrl = &woif->fwdl_ctrl;

	return fwdl_ops->fwdl_clear(ctrl);
}

