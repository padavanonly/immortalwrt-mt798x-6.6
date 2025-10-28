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
	physical_device_workq.c

	Abstract:
	One physical device task workq handle

*/
#include "rt_config.h"

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

#define MAX_HANDLE_COUNT 10

struct physical_device_workq_handler {
	u32 tag_id;
	char *tag_string;
	handler_fn handler;
};

/*******************************************************************************
 *                P R I V A T E    F U N C T I O N S
 *******************************************************************************
 */

/**
 * @brief Delete an entry from the physical device work queue.
 *
 * This function deletes an entry
 * from the physical device work queue based on the given parameters.
 *
 * @param ad The pointer to the RTMP_ADAPTER structure.
 * @param wdev The pointer to the wifi_dev structure.
 * @param data The pointer to the data buffer.
 * @param data_len The length of the data buffer.
 *
 * @return If the operation is successful, the function returns NDIS_STATUS_SUCCESS.
 *         Otherwise, it returns NDIS_STATUS_FAILURE.
 */
static u32 physical_device_workq_delete_entry(
	struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, u8 *data, u16 data_len)
{
	if (ad && data) {
		UINT16 wcid = 0;

		wcid = *((UINT16 *)data);
		MacTableDeleteEntry(ad, wcid, (data + 2));
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}

/**
 * @struct physical_device_workq_handler
 * Structure to hold the handler information for a work queue operation.
 */
static struct physical_device_workq_handler workq_handle_table[] = {
	{MAIN_WORKQ_DEL_MAC_ENTRY_TAG, "Delete Entry", physical_device_workq_delete_entry},
};

/**
 * @brief Retrieves the data from the physical device work queue
 *
 * This function retrieves the data from the given \p data_list,
 * which is a linked list representing a work queue.
 * It removes the head of the list and returns
 * it as a pointer to a `physical_device_workq_item` structure.
 *
 * @param ph_dev Pointer to the physical device object
 * @param data_list Pointer to the linked list representing the work queue
 * @return Pointer to the retrieved `physical_device_workq_item` structure
 */
static struct physical_device_workq_item *physical_device_workq_get_data(
	struct physical_device *ph_dev,
	PLIST_HEADER data_list)
{
	RT_LIST_ENTRY *data_item = NULL;

	OS_SEM_LOCK(&ph_dev->ph_dev_wq_lock);
	data_item = data_list->pHead;
	removeHeadList(data_list);
	OS_SEM_UNLOCK(&ph_dev->ph_dev_wq_lock);
	return (struct physical_device_workq_item *)data_item;
}

/**
 * @brief Handles the work queue items of a physical device
 *
 * This function handles the work queue items of a physical device.
 * It takes in a pointer to a `work_struct` structure,
 * which represents the work to be handled.
 *
 * @param work Pointer to the `work_struct` structure representing the work to be handled
 */
static void physical_device_workq_handle(
	struct work_struct *work)
{
	struct physical_device *ph_dev = container_of(
			work, struct physical_device, ph_dev_work.work);
	struct physical_device_workq_item *data;
	u32 count = 0;

	while ((count < MAX_HANDLE_COUNT) &&
		((data = physical_device_workq_get_data(ph_dev, &ph_dev->ph_dev_work_list)) != NULL)) {
		if (data->tag_id < MAIN_WORKQ_END_TAG_ID) {
			if (workq_handle_table[data->tag_id].handler) {
				workq_handle_table[data->tag_id].handler(
					(struct _RTMP_ADAPTER *)data->mac_ad,
					(struct wifi_dev *)data->wdev,
					data->data,
					data->data_len);
			} else
				MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG,
					"%s(): no handler for this tag id(=%d)\n",
					__func__, data->tag_id);
		} else
			MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG,
				"%s(): invalid tag id(=%d)\n", __func__, data->tag_id);
		os_free_mem(data);
		count++;
	}
}

/**
 * @brief Clears the given work queue list
 *
 * This function clears the content of the given work queue list indicated by `work_list`.
 * It starts by assigning the head of the list to a pointer called `work_item`.
 *
 * @param work_list Pointer to the work queue list to be cleared
 */
static void physical_device_workq_clear_list(
	IN  PLIST_HEADER work_list)
{
	RT_LIST_ENTRY *work_item = NULL;

	work_item = work_list->pHead;

	while (work_item != NULL) {
		removeHeadList(work_list);
		os_free_mem(work_item);
		work_item = work_list->pHead;
	}
}

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/**
 * @brief Initializes the work queue for a physical device
 *
 * This function initializes the work queue for a physical device.
 * It takes a pointer to a `physical_device` structure,
 * represented by `physical_dev`, as input.
 *
 * @param physical_dev Pointer to the physical device structure to initialize the work queue
 */
void physical_device_workq_init(
	void *physical_dev)
{
	struct physical_device *ph_dev = (struct physical_device *)physical_dev;

	ph_dev->ph_dev_wq = mt_create_workqueue("ph_dev_wq");
	if (ph_dev->ph_dev_wq == NULL) {
		MTWF_PRINT("%s(): No memory for workqueue\n", __func__);
		return;
	}
	NdisAllocateSpinLock(ph_dev, &ph_dev->ph_dev_wq_lock);
	OS_SEM_LOCK(&ph_dev->ph_dev_wq_lock);
	initList(&ph_dev->ph_dev_work_list);
	OS_SEM_UNLOCK(&ph_dev->ph_dev_wq_lock);
	mt_init_delayed_work(&ph_dev->ph_dev_work, physical_device_workq_handle);
}

/**
 * @brief Cleans up and exits the work queue for a physical device
 *
 * This function performs cleanup and exits the work queue for a physical device.
 * It takes a pointer to a `physical_device`
 * structure, represented by `physical_dev`, as input.
 *
 * @param physical_dev Pointer to the physical device structure to exit the work queue
 */
void physical_device_workq_exit(
	void *physical_dev)
{
	struct physical_device *ph_dev = (struct physical_device *)physical_dev;

	if (ph_dev->ph_dev_wq) {
		mt_flush_workqueue(ph_dev->ph_dev_wq);
		mt_destroy_workqueue(ph_dev->ph_dev_wq);
		ph_dev->ph_dev_wq = NULL;
		OS_SEM_LOCK(&ph_dev->ph_dev_wq_lock);
		physical_device_workq_clear_list(&ph_dev->ph_dev_work_list);
		OS_SEM_UNLOCK(&ph_dev->ph_dev_wq_lock);
		NdisFreeSpinLock(&ph_dev->ph_dev_wq_lock);
	}
}

/**
 * @brief Adds a job to the work queue of a physical device
 *
 * This function adds a job to the work queue of a physical device. It takes several parameters:
 *   - `mac_ad`: Pointer to a `_RTMP_ADAPTER` structure representing the MAC adapter.
 *   - `wdev`: Pointer to a `wifi_dev` structure representing the WiFi device.
 *   - `tag_id`: The ID associated with the job.
 *   - `data`: Pointer to a byte array containing the data for the job.
 *   - `data_len`: The length of the data in bytes.
 *   - `schedule_now`: Flag indicating whether to schedule the job immediately.
 *
 * The function starts by casting the `mac_ad->physical_dev` pointer
 * to the appropriate structure type and assigns it to `ph_dev`.
 *
 * Next, it determines if the work queue (`ph_dev->ph_dev_wq`) exists. If it doesn't,
 * a debug message is printed, and the function returns.
 *
 * The function then calculates the size required for the `item`,
 * including both the `struct physical_device_workq_item`
 * structure size and the size of the data. It allocates memory for `item` using `os_alloc_mem`.
 *
 * If memory allocation for `item` is successful,
 * the function proceeds by performing the following steps:
 *   - Fills the allocated memory with zeros using `os_zero_mem`.
 *   - Copies the data from `data` to the `item` structure using `os_move_mem`,
 *     only if `data` is not NULL and `data_len` is non-zero.
 *   - Sets the `item` structure's `data_len`, `mac_ad`, `wdev`, and `tag_id` fields.
 *   - Locks the work queue spin lock using `OS_SEM_LOCK`.
 *   - Inserts `item` at the tail of the work queue list using `insertTailList`.
 *   - Unlocks the work queue spin lock using `OS_SEM_UNLOCK`.
 *
 * Finally, if `schedule_now` is set to TRUE,
 * the function schedules the delayed work item `ph_dev->ph_dev_work`
 * on the work queue `ph_dev->ph_dev_wq` using `mt_queue_delayed_work`.
 *
 * @param mac_ad Pointer to the _RTMP_ADAPTER structure representing the MAC adapter
 * @param wdev Pointer to the wifi_dev structure representing the WiFi device
 * @param tag_id The ID associated with the job
 * @param data Pointer to the data for the job
 * @param data_len The length of the data in bytes
 * @param schedule_now Flag indicating whether to schedule the job immediately
 */
void physical_device_workq_add_job(
	struct _RTMP_ADAPTER *mac_ad,
	struct wifi_dev *wdev,
	u32 tag_id,
	uint8_t *data,
	uint32_t data_len,
	u8 schedule_now)
{
	struct physical_device *ph_dev = (struct physical_device *)mac_ad->physical_dev;
	struct physical_device_workq_item *item;
	u32 item_size;

	if (ph_dev->ph_dev_wq == NULL) {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_INFO,
				 "no workqueue.....\n");
		return;
	}

	item_size = sizeof(struct physical_device_workq_item) + data_len;
	os_alloc_mem(NULL, (UCHAR **)&item, item_size);
	if (item) {
		os_zero_mem(item, item_size);
		if (data && data_len)
			os_move_mem(&item->data[0], data, data_len);
		item->data_len = data_len;
		item->mac_ad = mac_ad;
		item->wdev = wdev;
		item->tag_id = tag_id;
		OS_SEM_LOCK(&ph_dev->ph_dev_wq_lock);
		insertTailList(&ph_dev->ph_dev_work_list, (RT_LIST_ENTRY *)item);
		OS_SEM_UNLOCK(&ph_dev->ph_dev_wq_lock);
	}
	if (schedule_now == TRUE)
		mt_queue_delayed_work(ph_dev->ph_dev_wq, &ph_dev->ph_dev_work, 0);
}

/**
 * @brief Shows information about the work queue of a physical device
 *
 * This function displays information about the work queue of a physical device.
 * It takes two parameters:
 *   - `mac_ad`: Pointer to a `_RTMP_ADAPTER` structure representing the MAC adapter.
 *   - `arg`: Pointer to a character string argument (not used in the function).
 *
 * The function starts by casting the `mac_ad->physical_dev` pointer
 * to the appropriate structure type and assigns it to `ph_dev`.
 *
 * Next, it locks the work queue spin lock using `OS_SEM_LOCK`.
 *
 * Then, using a while loop, the function iterates over each item in the work queue.
 *   - Prints the tag ID and corresponding tag string for each item
 *     in the work queue using `MTWF_PRINT`.
 *   - Updates the `item` pointer to the next item in the work queue.
 *
 * After iterating through all the items in the work queue,
 * the function prints the total count of items in the work queue using `MTWF_PRINT`.
 *
 * Finally, it unlocks the work queue spin lock using `OS_SEM_UNLOCK` and returns TRUE.
 *
 * @param mac_ad Pointer to the _RTMP_ADAPTER structure representing the MAC adapter
 * @param arg Pointer to a character string argument (not used in the function)
 * @return TRUE indicating the function execution completed successfully
 */
INT physical_device_workq_show_info(struct _RTMP_ADAPTER *mac_ad, RTMP_STRING *arg)
{
	struct physical_device *ph_dev = (struct physical_device *)mac_ad->physical_dev;
	struct physical_device_workq_item *item;

	OS_SEM_LOCK(&ph_dev->ph_dev_wq_lock);
	item = (struct physical_device_workq_item *)ph_dev->ph_dev_work_list.pHead;
	while (item) {
		if (item->tag_id < MAIN_WORKQ_END_TAG_ID)
			MTWF_PRINT("TagID: %d (%s)\n",
				item->tag_id, workq_handle_table[item->tag_id].tag_string);
		else
			MTWF_PRINT("TagID: %d (unknown id)\n",
				item->tag_id);
		item = item->pNext;
	}
	MTWF_PRINT("Total count: %d\n", ph_dev->ph_dev_work_list.size);
	OS_SEM_UNLOCK(&ph_dev->ph_dev_wq_lock);
	return TRUE;
}

