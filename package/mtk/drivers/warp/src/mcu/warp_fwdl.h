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
	warp_fwdl.h
*/

#ifndef _WARP_FWDL_H_
#define _WARP_FWDL_H_

#include "warp_utility.h"

struct warp_entry;

#define IS_WO_HOST_MODE (wo_host_exist != NULL)
#define UNDEFINE_WED_IDX (0xff)
#define MAX_REGION_SIZE 3
#define MAX_FWDL_SIZE 0xfffffff


struct fw_info_t {
	u8 chip_id;
	u8 eco_ver;
	u8 num_of_region;
	u8 format_ver;
	u8 format_flag;
	u8 ram_ver[10];
	u8 ram_built_date[15];
	u32 crc;
};

struct fwdl_region {
	void *base_addr;
	resource_size_t base_addr_pa;
	u32 size;
	u32 shared;
};

#define WO_FWDL_SUCCESS 0
#define WO_FWDL_FAIL 1

struct warp_fwdl_ctrl {
	struct fw_info_t fw_info;
	struct fwdl_region region[MAX_REGION_SIZE];
	struct fwdl_region boot_setting;
	struct wo_fwdl_host_mode_ctrl **wo_host_exist;
	u8 *bin_ptr;
	u32 bin_size;
	u8 bin_mode;
	u8 status;
};

struct wo_fwdl_host_mode_ctrl {
	u8 wed_idx;
	struct completion fwdl_warp_notify_wo;
	struct completion fwdl_wo_notify_warp;
};

struct warp_fwdl_ops {
	int (*init)(struct warp_fwdl_ctrl *mcu_ctrl, u8 wed_idx);
	int (*write_wed_idx)(struct warp_fwdl_ctrl *mcu_ctrl, struct wed_entry *wed,
			     u8 wed_idx);
	int (*do_fwdl)(struct warp_fwdl_ctrl *mcu_ctrl, struct wed_entry *wed,
			     u8 wed_idx);
	int (*fwdl_ready_check)(struct warp_fwdl_ctrl *mcu_ctrl, struct wed_entry *wed);
	int (*deinit)(struct warp_fwdl_ctrl *mcu_ctrl);
	int (*fwdl_done_check)(struct warp_fwdl_ctrl *mcu_ctrl);
	int (*fwdl_clear)(struct warp_fwdl_ctrl *mcu_ctrl);
};

#define FW_V3_COMMON_TAILER_SIZE 36
#define FW_V3_REGION_TAILER_SIZE 40

int warp_fwdl(struct warp_entry *warp);
int warp_fwdl_clear(struct warp_entry *warp);
void warp_fwdl_reset(struct warp_fwdl_ctrl *ctrl, bool en, u8 wed_idx);
void warp_fwdl_write_start_address(struct warp_fwdl_ctrl *ctrl, u32 addr, u8 wed_idx);
void warp_fwdl_get_wo_heartbeat(struct warp_fwdl_ctrl *ctrl, u32 *hb_val, u8 wed_idx);
#endif /*_WARP_FWDL_H_*/
