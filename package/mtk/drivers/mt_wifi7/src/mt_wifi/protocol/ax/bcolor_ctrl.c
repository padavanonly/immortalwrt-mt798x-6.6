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
/***************************************************************************
 ***************************************************************************

*/

#ifdef DOT11_HE_AX

#include "rt_config.h"
#include "bcolor_ctrl.h"

void bss_color_table_init(struct _RTMP_ADAPTER *ad)
{
	struct bss_color_table *table = &ad->color_tbl;

	os_zero_mem(table, sizeof(struct bss_color_table));
	OS_NdisAllocateSpinLock(&table->bss_color_lock);
}

void bss_color_table_deinit(struct _RTMP_ADAPTER *ad)
{
	struct bss_color_table *table;

	table = &ad->color_tbl;
	NdisFreeSpinLock(&table->bss_color_lock);
	os_zero_mem(table, sizeof(struct bss_color_table));
}

UINT8 bcolor_acquire_entry(struct _RTMP_ADAPTER *ad)
{
	struct bss_color_table *table;
	ULONG current_time;
	UINT8 new_color;
	UINT8 start_idx, stop_idx;

	table = &ad->color_tbl;

	NdisGetSystemUpTime(&current_time);
	start_idx = (UINT8)(current_time % BSS_COLOR_VALUE_MAX);
	stop_idx = start_idx + BSS_COLOR_VALUE_MAX;
	NdisAcquireSpinLock(&table->bss_color_lock);

	while (start_idx < stop_idx) {
		new_color = (start_idx < BSS_COLOR_VALUE_MAX) ? start_idx : (start_idx % BSS_COLOR_VALUE_MAX);
		if (table->last_detected_time[new_color] == 0) {
			table->last_detected_time[new_color] = current_time;
			break;
		} else {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO,
				"new_color = %u, last detect time = %lu, current time = %lu\n",
				new_color, table->last_detected_time[new_color], current_time);
		}
		start_idx++;
	}

	NdisReleaseSpinLock(&table->bss_color_lock);

	return (start_idx == stop_idx) ? 0 : (new_color + 1);
}

void bcolor_release_entry(struct _RTMP_ADAPTER *ad, uint8_t color)
{
	struct bss_color_table *table;

	if (color < BSS_COLOR_VALUE_MIN || color > BSS_COLOR_VALUE_MAX)
		return;

	table = &ad->color_tbl;
	NdisAcquireSpinLock(&table->bss_color_lock);
	table->last_detected_time[color - 1] = 0;
	NdisReleaseSpinLock(&table->bss_color_lock);
}

void bcolor_occupy_entry(struct _RTMP_ADAPTER *ad, uint8_t color)
{
	struct bss_color_table *table;
	ULONG current_time;

	if (color < BSS_COLOR_VALUE_MIN || color > BSS_COLOR_VALUE_MAX)
		return;

	table = &ad->color_tbl;
	NdisGetSystemUpTime(&current_time);
	NdisAcquireSpinLock(&table->bss_color_lock);
	table->last_detected_time[color - 1] = current_time;
	NdisReleaseSpinLock(&table->bss_color_lock);
}

uint8_t bcolor_entry_is_occupied(struct _RTMP_ADAPTER *ad, uint8_t color)
{
	struct bss_color_table *table;
	uint8_t ret;

	if (color < BSS_COLOR_VALUE_MIN || color > BSS_COLOR_VALUE_MAX)
		return FALSE;

	table = &ad->color_tbl;
	NdisAcquireSpinLock(&table->bss_color_lock);

	if (table->last_detected_time[color - 1])
		ret = TRUE;
	else
		ret = FALSE;

	NdisReleaseSpinLock(&table->bss_color_lock);
	return ret;
}

void bcolor_entry_ageout(struct _RTMP_ADAPTER *ad, uint8_t sec)
{
	struct bss_color_table *table;
	UCHAR i;
	ULONG current_time;

	table = &ad->color_tbl;
	NdisGetSystemUpTime(&current_time);
	NdisAcquireSpinLock(&table->bss_color_lock);

	for (i = 0; i < BSS_COLOR_VALUE_MAX; i++) {
		if (RTMP_TIME_AFTER(current_time, table->last_detected_time[i] + (sec * OS_HZ)))
			table->last_detected_time[i] = 0;
	}

	NdisReleaseSpinLock(&table->bss_color_lock);
}

void bcolor_get_bitmap(struct _RTMP_ADAPTER *ad, uint8_t *bitmap)
{
	struct bss_color_table *table;
	UCHAR i;
	uint8_t maps[8];

	table = &ad->color_tbl;

	os_zero_mem(maps, sizeof(maps));
	NdisAcquireSpinLock(&table->bss_color_lock);

	for (i = 0; i < BSS_COLOR_VALUE_MAX; i++) {
		if (table->last_detected_time[i])
			maps[(i + 1) / 8] |= 1 << ((i + 1) % 8);
	}
	NdisReleaseSpinLock(&table->bss_color_lock);
	os_move_mem(bitmap, maps, sizeof(maps));
}

void bcolor_update_by_bitmap(struct _RTMP_ADAPTER *ad, uint8_t *bitmap)
{
	struct bss_color_table *table;
	UCHAR i;
	ULONG current_time;
	uint8_t maps[8];

	table = &ad->color_tbl;
	NdisGetSystemUpTime(&current_time);

	os_move_mem(maps, bitmap, sizeof(maps));
	NdisAcquireSpinLock(&table->bss_color_lock);

	for (i = 0; i < BSS_COLOR_VALUE_MAX; i++) {
		if (maps[(i + 1) / 8] & (1 << ((i+1) % 8)))
			table->last_detected_time[i] = current_time;
	}

	NdisReleaseSpinLock(&table->bss_color_lock);
}
#endif
