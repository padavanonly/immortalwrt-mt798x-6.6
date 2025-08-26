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
	physical_device_thread.h

*/

#ifndef __PHYSICAL_DEVICE_THREAD_DEF_H__
#define __PHYSICAL_DEVICE_THREAD_DEF_H__

typedef u32 (*handler_fn)(
	struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, u8 *data, u16 data_len);

enum physical_thread_tag {
	MAIN_THREAD_FIRST_TAG_ID = 0,
	MAIN_THREAD_SHOW_BCN_INFO_TAG = MAIN_THREAD_FIRST_TAG_ID,
	MAIN_THREAD_END_TAG_ID
};

struct physical_thread_data_item {
	struct physical_thread_data_item *pNext;
	void *mac_ad;
	void *wdev;
	u32 tag;
	u16 data_len;
	u8 data[0];
};

NDIS_STATUS physical_device_thread_init(
	void *physical_dev);
void physical_device_thread_exit(
	void *physical_dev);
void physical_device_thread_add_data(
	void *mac_ad_obj,
	void *wdev_obj,
	u32 tag_id,
	u8 *data,
	u32 data_len);
void physical_device_thread_queue_clear_by_mac_ad(
	struct _RTMP_ADAPTER *mac_ad);
void physical_device_thread_queue_clear_by_wdev(
	struct _RTMP_ADAPTER *mac_ad,
	struct wifi_dev *wdev);
#endif  /* __PHYSICAL_DEVICE_THREAD_DEF_H__ */

