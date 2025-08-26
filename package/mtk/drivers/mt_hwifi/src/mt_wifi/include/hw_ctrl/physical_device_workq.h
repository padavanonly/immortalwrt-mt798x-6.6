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
	physical_device_workq.h

*/

#ifndef __PHYSICAL_DEVICE_WORKQ_DEF_H__
#define __PHYSICAL_DEVICE_WORKQ_DEF_H__

struct physical_device_workq_item {
	struct physical_device_workq_item *pNext;
	struct _RTMP_ADAPTER *mac_ad;
	struct wifi_dev *wdev;
	u32 tag_id;
	u16 data_len;
	u8 data[0];
};

enum physical_device_workq_tag {
	MAIN_WORKQ_FIRST_TAG_ID = 0,
	MAIN_WORKQ_DEL_MAC_ENTRY_TAG = MAIN_WORKQ_FIRST_TAG_ID,
	MAIN_WORKQ_END_TAG_ID
};

void physical_device_workq_init(
	void *physical_dev);
void physical_device_workq_exit(
	void *physical_dev);
void physical_device_workq_add_job(
	struct _RTMP_ADAPTER *mac_ad,
	struct wifi_dev *wdev,
	u32 tag_id,
	uint8_t *data,
	uint32_t data_len,
	u8 schedule_now);
INT physical_device_workq_show_info(struct _RTMP_ADAPTER *mac_ad, RTMP_STRING *arg);

#endif  /* __PHYSICAL_DEVICE_WORKQ_DEF_H__ */

