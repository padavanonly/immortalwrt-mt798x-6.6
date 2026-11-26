/*
 * Copyright (c) [2020], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/
/****************************************************************************
 ***************************************************************************

	Module Name:
	physical_device.h

	Abstract:
	One physical device generic portion header file

*/
#ifndef __PHYSICAL_DEVICE_H__
#define __PHYSICAL_DEVICE_H__

#define MAC_MAC_ADAPTER_NUM	3

void *
physical_device_find_physical_dev(void *hw_dev);

u8
physical_device_add_main_device(void *hw_dev, u32 chip_id, unsigned long hw_flags, void *mac_cap_info);

void
physical_device_remove_main_device(void *physical_dev);

void
physical_device_show_information(void);

u8
physical_device_get_band_en(void *physical_dev, u8 band_idx);

void
physical_device_proc_handle_init(void *physical_dev);

void
physical_device_proc_handle_exit(void *physical_dev);

void
physical_device_init(void);

void
physical_device_exit(void);

u8
physical_device_add_mac_adapter(void *hw_dev, void *hw_obj);

u8
physical_device_remove_mac_adapter(void *hw_dev, void *mac_ad);

struct _RTMP_ADAPTER *
physical_device_get_mac_adapter_by_band(void *physical_dev, u8 band_idx);

u32
physical_device_get_mac_list_by_chip_hw_id(
	u32 chip_id, u32 hw_id, struct _RTMP_ADAPTER **ret_pAd_list);

struct _RTMP_ADAPTER *
physical_device_get_first_mac_adapter(void *physical_dev);

struct _RTMP_ADAPTER *
physical_device_get_first_up_mac_adapter(void *physical_dev);

void
physical_device_timer_list_add(
	void *ph_dev_obj, void *pRsc, char *timer_name);

void
physical_device_timer_list_release(void *ph_dev_obj, void *pRsc);

void
physical_device_show_timer_list(void *ph_dev_obj);

void
physical_device_all_timer_list_release(void *ph_dev_obj);

void
physical_device_init_timer(
	struct _RTMP_ADAPTER *mac_ad,
	void *ph_dev_obj,
	void *timer_obj,
	void *pTimerFunc,
	void *pData,
	u8 Repeat,
	char *timer_name);

#endif  /* __PHYSICAL_DEVICE_H__ */

