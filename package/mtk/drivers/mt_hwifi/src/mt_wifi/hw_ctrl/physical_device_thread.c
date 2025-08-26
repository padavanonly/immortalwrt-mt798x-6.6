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
	physical_device_thread.c

	Abstract:
	One physical device task thread handle

*/
#include	"rt_config.h"

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

#define MAX_HANDLE_COUNT 5

/**
 * Struct: physical_device_thread_handler
 *
 * This structure represents a thread handler for a physical device.
 *
 * Members:
 *
 * - **tag_id**: An identifier for the physical device that this thread handler is associated with.
 * - **handler**: The function that will be called when the thread is executed.
 */
struct physical_device_thread_handler {
	u32 tag_id;
	handler_fn handler;
};

/*******************************************************************************
 *                P R I V A T E    F U N C T I O N S
 *******************************************************************************
 */

/**
 * @brief Handles the command to show beacon information for a specific band.
 *
 * @param ad The adapter structure pointer.
 * @param wdev The wireless device structure pointer.
 * @param data The input data buffer.
 * @param data_len The length of the input data buffer.
 *
 * @return NDIS_STATUS_SUCCESS if successful, NDIS_STATUS_FAILURE otherwise.
 */
static u32 physical_device_thread_show_bcn_info_hdlr(
	struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, u8 *data, u16 data_len)
{
	u8 band_idx;

	if (ad && data) {
		struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(ad->hdev_ctrl);

		band_idx = data[0];
		MTWF_PRINT("%s(): show beacon info for band%d\n",
			 __func__, band_idx);
		MTWF_PRINT("-------------------------------\n");
		if (chip_dbg->show_bcn_info)
			chip_dbg->show_bcn_info(ad->hdev_ctrl, band_idx);
		MTWF_PRINT("-------------------------------\n");
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}

static struct physical_device_thread_handler handle_table[] = {
	{MAIN_THREAD_SHOW_BCN_INFO_TAG, physical_device_thread_show_bcn_info_hdlr},
};

/**
 * @brief Triggers firmware time synchronization on a physical device.
 *
 * @param mac_ad pointer to a structure representing the MAC adapter
 * @param cap pointer to a structure containing chip capabilities
 *
 * @return NDIS_STATUS_SUCCESS if successful, otherwise an error status
 */
static NTSTATUS physical_device_trigger_fw_time_sync(
	struct _RTMP_ADAPTER *mac_ad, struct _RTMP_CHIP_CAP *cap)
{
#ifdef LINUX
	UINT32 second = 0;
	UINT32 usecond = 0;
	struct tm current_tm;

#if (KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE)
	struct timespec64 time;

	ktime_get_real_ts64(&time);
	second = (unsigned int)time.tv_sec;
	usecond = (unsigned int)time.tv_nsec/1000;
#else
	struct timeval time;

	do_gettimeofday(&time);
	second = (unsigned int)time.tv_sec;
	usecond = (unsigned int)time.tv_usec;
#endif
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		MtUniCmdFwTimeSync(mac_ad, second, usecond);
#endif

	time64_to_tm((time64_t) time.tv_sec, 0, &current_tm);
	MTWF_PRINT("%ld-%02d-%02d %02d:%02d:%02d.%u UTC\n",
	current_tm.tm_year + 1900, current_tm.tm_mon + 1, current_tm.tm_mday,
	current_tm.tm_hour, current_tm.tm_min, current_tm.tm_sec, usecond);
#endif /* LINUX */
	return NDIS_STATUS_SUCCESS;
}

/**
 * @brief Retrieves the data item from the given data list.
 *
 * This function locks the thread data lock of the physical device,
 * retrieves the head of the data list, removes the head item from the list,
 * and unlocks the thread data lock.
 *
 * @param ph_dev The physical device associated with the data list.
 * @param data_list The list containing the data items.
 * @return The retrieved data item.
 */
static RT_LIST_ENTRY *physical_device_thread_get_data(
	struct physical_device *ph_dev,
	PLIST_HEADER data_list)
{
	RT_LIST_ENTRY *data_item = NULL;

	OS_SEM_LOCK(&ph_dev->pd_thread_data_lock);
	data_item = data_list->pHead;
	removeHeadList(data_list);
	OS_SEM_UNLOCK(&ph_dev->pd_thread_data_lock);
	return data_item;
}

/**
 * @brief Handles the thread data of a physical device.
 *
 * This function handles the thread data of a physical device by processing each item
 * in the data list.
 * It calls the appropriate handler function for each item based on its tag value,
 * and updates the periodic round counter for every 10 seconds. Additionally,
 * it performs other tasks such as triggering firmware time sync, heart beat check,
 * maintaining PMKID cache, and querying BSS ACQ packet number.
 *
 * @param ph_dev The pointer to the physical device structure.
 * @param data_list The pointer to the list header of the thread data items.
 */
static void physical_device_thread_data_handle(
	struct physical_device *ph_dev,
	PLIST_HEADER data_list)
{
	struct physical_thread_data_item *data = NULL;
	RT_LIST_ENTRY *data_item = NULL;
	struct wifi_dev *wdev = NULL;
	u32 count = 0;
	struct _RTMP_ADAPTER *mac_ad = physical_device_get_first_mac_adapter(ph_dev);
	struct _RTMP_CHIP_CAP *cap = &ph_dev->chip_cap;
#ifdef WF_RESET_SUPPORT
	struct _RTMP_ARCH_OP *arch_ops = NULL;
#endif
	PFW_LOG_CTRL FwLogCtrl = &(mac_ad->physical_dev->fw_log_ctrl);
	UINT32 LogType;

	while (((data_item = physical_device_thread_get_data(ph_dev, data_list)) != NULL)
		&& (count < MAX_HANDLE_COUNT)) {
		data = (struct physical_thread_data_item *)data_item;
		wdev = (struct wifi_dev *)data->wdev;
		if (data->tag < MAIN_THREAD_END_TAG_ID) {
			if (handle_table[data->tag].handler) {
				handle_table[data->tag].handler(
					(struct _RTMP_ADAPTER *)data->mac_ad,
					(struct wifi_dev *)data->wdev,
					data->data,
					data->data_len);
			} else
				MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG,
					"%s(): no handler for this tag id(=%d)\n",
					__func__, data->tag);
		} else
			MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG,
				"%s(): invalid tag id(=%d)\n", __func__, data->tag);
		os_free_mem(data_item);
		count++;
	}

	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG,
		"periodic_round = %lu\n", ph_dev->periodic_round);
	if (mac_ad && RTMP_TEST_FLAG(ph_dev, PH_DEV_SYSTEM_START_UP)) {
#ifdef WF_RESET_SUPPORT
		arch_ops = hc_get_arch_ops(mac_ad->hdev_ctrl);
#endif
#ifdef PEAK_ENHANCE
#ifdef WHNAT_SUPPORT
		peak_adjust_rx_drop_handle(ph_dev);
#endif /* WHNAT_SUPPORT */
#endif /* PEAK_ENHANCE */
		/* 10 seconds update */
		LogType = FwLogCtrl->wmcpu_log_type;
		if ((LogType == 40) || (LogType == 41) ||
			(LogType == 96) || (LogType == 97)) {
			if ((ph_dev->periodic_round % 10) == 0)
				physical_device_trigger_fw_time_sync(mac_ad, cap);
		}
#ifdef WF_RESET_SUPPORT
		if (arch_ops) {
			if (arch_ops->heart_beat_check)
				arch_ops->heart_beat_check(mac_ad);

		}
#endif
#ifndef RT_CFG80211_SUPPORT
		RTMPMaintainPMKIDCache(ph_dev); /*hostapd will set/del pmkid cache*/
#endif
	}

#ifdef PKT_BUDGET_CTRL_SUPPORT

	mac_ad = physical_device_get_first_up_mac_adapter(ph_dev);

	if ((mac_ad) && (IS_ASIC_CAP(mac_ad, fASIC_CAP_WMM_PKTDETECT_OFFLOAD)))
		HW_WA_QUERY(mac_ad, CR4_QUERY_OPTION_GET_BSS_ACQ_PKT_NUM, CR4_GET_BSS_ACQ_PKT_NUM_CMD_DEFAULT, 0);
#endif
}

/**
 * @brief Physical device main thread function.
 *
 * This function serves as the entry point for the physical device thread.
 * It initializes the thread data structure,
 * customizes the task, and starts the loop that handles the device events.
 *
 * @param Context pointer to the RTMP_OS_TASK structure that represents this thread.
 *
 * @return 0.
 */
static INT physical_device_main_thread(
	ULONG Context)
{
	RTMP_OS_TASK *pTask;
	struct physical_device *ph_dev;
	int	status = 0;

	pTask = (RTMP_OS_TASK *)Context;
	if (pTask == NULL) {
		MTWF_PRINT("%s(): pTask == NULL\n", __func__);
		return 0;
	}

	ph_dev = (struct physical_device *)RTMP_OS_TASK_DATA_GET(pTask);
	if (ph_dev == NULL) {
		MTWF_PRINT("%s(): device_entry == NULL\n", __func__);
		return 0;
	}

	RtmpOSTaskCustomize(pTask);

	while (!RTMP_OS_TASK_IS_KILLED(pTask)) {
		if (RtmpOSTaskWait(NULL, pTask, &status) == FALSE)
			break;

		physical_device_thread_data_handle(ph_dev, &ph_dev->pd_thread_data_list);
	}

	MTWF_PRINT("<--- %s()\n", __func__);
	RtmpOSTaskNotifyToExit(pTask);
	return 0;
}

/**
 * @brief Initializes the thread queue for a physical device.
 *
 * This function initializes the spin lock and list data structures used by the
 * physical device thread.
 *
 * @param ph_dev A pointer to the physical device structure.
 */
void physical_device_thread_queue_init(
	struct physical_device *ph_dev)
{
	MTWF_PRINT("---> %s()\n", __func__);
	NdisAllocateSpinLock(NULL, &ph_dev->pd_thread_data_lock);
	initList(&ph_dev->pd_thread_data_list);
	MTWF_PRINT("<--- %s()\n", __func__);
}

/**
* @brief Clears all items from a thread-safe queue.
*
* This function clears all items from a thread-safe queue by iterating
* through the list and freeing each item.
* It then sets the `pHead` pointer of the list to `NULL`.
*
* @param data_list The thread-safe queue to clear.
*/
static void physical_device_thread_queue_clear(
	PLIST_HEADER data_list)
{
	RT_LIST_ENTRY *data_item = data_list->pHead;

	while (data_item != NULL) {
		removeHeadList(data_list);
		os_free_mem(data_item);
		data_item = data_list->pHead;
	}
}

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/**
 * @brief Initialize the thread for the physical device.
 *
 * This function initializes the thread for the physical device, including creating a new task
 * and attaching it to the main thread.
 *
 * @param physical_dev A pointer to the physical device structure.
 *
 * @return NDIS_STATUS The status of the operation.
 */
NDIS_STATUS physical_device_thread_init(
	void *physical_dev)
{
	NDIS_STATUS status;
	RTMP_OS_TASK *pTask;
	struct physical_device *ph_dev = (struct physical_device *)physical_dev;
	char task_name[TASK_NAME_LEN] = {0};
	int ret = 0;

	MTWF_PRINT("---> %s()\n", __func__);
	physical_device_thread_queue_init(ph_dev);
	pTask = &ph_dev->pd_thread_task;
	ret = snprintf(task_name, TASK_NAME_LEN,
		"DeviceTask_%d", ph_dev->device_idx);
	if (os_snprintf_error(TASK_NAME_LEN, ret))
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
			"snprintf error!\n");
	MTWF_PRINT("%s(%d): task_name is %s\n", __func__, __LINE__, task_name);
	RtmpOSTaskInit(pTask, task_name, ph_dev, NULL, NULL);
	status = RtmpOSTaskAttach(pTask, physical_device_main_thread, (ULONG)pTask);
	MTWF_PRINT("<--- %s(): status=%d!\n", __func__, status);
	return status;
}

/**
* @brief Exit function for physical device thread.
*
* This function is used to exit the physical device thread gracefully.
* It releases all the resources allocated by the thread and clears the queue of pending data.
*
* @param physical_dev A pointer to the struct physical_device representing the physical device.
*/
void physical_device_thread_exit(void *physical_dev)
{
	struct physical_device *ph_dev = (struct physical_device *)physical_dev;
	int ret;

	if (!ph_dev) {
		MTWF_PRINT("%s(): ph_dev is NULL!\n", __func__);
		return;
	}
	ret = RtmpOSTaskKill(&ph_dev->pd_thread_task);
	if (ret != NDIS_STATUS_SUCCESS)
		MTWF_PRINT("%s(): kill physical device thread failed!\n", __func__);
	OS_SEM_LOCK(&ph_dev->pd_thread_data_lock);
	physical_device_thread_queue_clear(&ph_dev->pd_thread_data_list);
	OS_SEM_UNLOCK(&ph_dev->pd_thread_data_lock);
	NdisFreeSpinLock(&ph_dev->pd_thread_data_lock);
}

/**
 * @brief Add data to a physical device thread.
 *
 * @param mac_ad_obj Pointer to a MAC adapter object.
 * @param wdev_obj Pointer to a wireless device object.
 * @param tag_id Thread ID.
 * @param data Data to be added.
 * @param data_len Length of the data.
 */
void physical_device_thread_add_data(
	void *mac_ad_obj,
	void *wdev_obj,
	u32 tag_id,
	u8 *data,
	u32 data_len)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)mac_ad_obj;
	struct physical_device *ph_dev = ad->physical_dev;
	struct physical_thread_data_item *data_item;
	uint32_t data_item_size;

	data_item_size = sizeof(struct physical_thread_data_item) + data_len;
	os_alloc_mem(NULL, (UCHAR **)&data_item, data_item_size);
	if (data_item) {
		os_zero_mem(data_item, data_item_size);
		if (data && data_len)
			os_move_mem(&data_item->data[0], data, data_len);
		data_item->data_len = data_len;
		data_item->mac_ad = mac_ad_obj;
		data_item->wdev = wdev_obj;
		data_item->tag = tag_id;
		OS_SEM_LOCK(&ph_dev->pd_thread_data_lock);
		insertTailList(&ph_dev->pd_thread_data_list, (RT_LIST_ENTRY *)data_item);
		OS_SEM_UNLOCK(&ph_dev->pd_thread_data_lock);
	}
}

/**
 * @brief Clears the thread queue of a physical device by MAC address.
 *
 * @param mac_ad The pointer to the MAC address of the physical device.
 */
void physical_device_thread_queue_clear_by_mac_ad(
	struct _RTMP_ADAPTER *mac_ad)
{
	PLIST_HEADER data_list;
	RT_LIST_ENTRY *data_item = NULL;
	struct physical_device *ph_dev;
	struct physical_thread_data_item *data;

	if (mac_ad) {
		ph_dev = mac_ad->physical_dev;
		data_list = &ph_dev->pd_thread_data_list;
		data_item = data_list->pHead;

		while (data_item != NULL) {
			data = (struct physical_thread_data_item *)data_item;
			if (data->mac_ad == mac_ad) {
				OS_SEM_LOCK(&ph_dev->pd_thread_data_lock);
				removeHeadList(data_list);
				OS_SEM_UNLOCK(&ph_dev->pd_thread_data_lock);
				os_free_mem(data_item);
			}
			data_item = data_list->pHead;
		}
	}
}

/**
 * @brief Clears the thread queue of a specific wireless device on a physical device.
 *
 * @param mac_ad The pointer to the MAC adapter structure.
 * @param wdev The pointer to the wireless device structure.
 */
void physical_device_thread_queue_clear_by_wdev(
	struct _RTMP_ADAPTER *mac_ad,
	struct wifi_dev *wdev)
{
	PLIST_HEADER data_list;
	RT_LIST_ENTRY *data_item = NULL;
	struct physical_device *ph_dev;
	struct physical_thread_data_item *data;

	if (mac_ad && wdev) {
		ph_dev = mac_ad->physical_dev;
		data_list = &ph_dev->pd_thread_data_list;
		data_item = data_list->pHead;

		while (data_item != NULL) {
			data = (struct physical_thread_data_item *)data_item;
			if (data->wdev == wdev) {
				OS_SEM_LOCK(&ph_dev->pd_thread_data_lock);
				removeHeadList(data_list);
				OS_SEM_UNLOCK(&ph_dev->pd_thread_data_lock);
				os_free_mem(data_item);
			}
			data_item = data_list->pHead;
		}
	}
}

