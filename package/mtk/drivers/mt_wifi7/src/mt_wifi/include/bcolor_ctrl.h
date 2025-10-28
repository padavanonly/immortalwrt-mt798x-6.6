/*
 * Copyright (c) [2024], MediaTek Inc. All rights reserved.
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
/***************************************************************************
 ***************************************************************************

*/


#ifndef _BCOLOR_CTRL_H_
#define _BCOLOR_CTRL_H_

#define BSS_COLOR_VALUE_MIN	1
#define BSS_COLOR_VALUE_MAX	63

#ifdef DOT11_HE_AX
struct bss_color_table {
	ULONG last_detected_time[BSS_COLOR_VALUE_MAX];
	NDIS_SPIN_LOCK bss_color_lock;
};

/* BSS color table contrl */
void bss_color_table_init(struct _RTMP_ADAPTER *ad);
void bss_color_table_deinit(struct _RTMP_ADAPTER *ad);
UINT8 bcolor_acquire_entry(struct _RTMP_ADAPTER *ad);
void bcolor_release_entry(struct _RTMP_ADAPTER *ad, uint8_t color);
void bcolor_occupy_entry(struct _RTMP_ADAPTER *ad, uint8_t color);
BOOLEAN bcolor_entry_is_occupied(struct _RTMP_ADAPTER *ad, uint8_t color);
void bcolor_entry_ageout(struct _RTMP_ADAPTER *ad, uint8_t sec);
void bcolor_get_bitmap(struct _RTMP_ADAPTER *ad, uint8_t *bitmap);
void bcolor_update_by_bitmap(struct _RTMP_ADAPTER *ad, uint8_t *bitmap);
#endif

#endif
