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
	physical_device.c

	Abstract:
	One physical device generic portion variable/functions

*/
#include	"rt_config.h"

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
static LIST_HEADER physical_device_list;

#define PD_MAIN_TIMER_NAME "main_timer"

DEFINE_SPINLOCK(physical_device_list_lock);
struct physical_device_node {
	struct physical_device_node *pNext;
	struct physical_device device_entry;
};

struct mac_ad_node {
	struct mac_ad_node *pNext;
	struct _RTMP_ADAPTER *ad;
};

struct chip_handle {
	u32 chip_id;
	void (*chip_cap_init)(void *ph_dev_obj);
};

struct chip_handle chip_handle_proc[] = {
#ifdef MT7990
	{NIC7990_PCIe_DEVICE_ID, mt7990_init_default},
#endif /* MT7990 */
#ifdef MT7992
	{NIC7992_PCIe_DEVICE_ID, mt7992_init_default},
#endif /* MT7992 */
#ifdef MT7993
	{NIC7993_PCIe_DEVICE_ID, mt7993_init_default},
#endif /* MT7993 */
	{0, NULL}
};


#define hw_to_mac_dev(_hw) container_of(_hw, struct mtk_mac_dev, hw)

/*******************************************************************************
 *                P R I V A T E    F U N C T I O N S
 *******************************************************************************
 */

/**
 * @brief Find a specific physical device in a linked list.
 *
 * This function searches for a physical device with a matching hardware device pointer
 * in a linked list of physical devices.
 *
 * @param hw_dev The hardware device pointer to search for.
 * @return A pointer to the physical_device_node structure containing the matching device,
 *         or NULL if no matching device is found.
 */
static struct physical_device_node *
physical_device_find_master_dev(void *hw_dev)
{
	struct physical_device_node *device_node;
	struct physical_device *device;
	BOOLEAN found = FALSE;

	/* need handle lock outside by caller!! */
	device_node = (struct physical_device_node *)physical_device_list.pHead;

	while (device_node) {
		device = &device_node->device_entry;
		if (device->hw_dev == hw_dev) {
			found = TRUE;
			break;
		}

		device_node = (struct physical_device_node *)device_node->pNext;
	}

	if (found == FALSE)
		device_node = NULL;

	return device_node;
}

/**
 * @brief Find the device node that contains a specific physical device structure.
 *
 * This function searches for a device node in a linked list of physical devices
 * that contains a matching physical device structure.
 *
 * @param ph_dev Pointer to the physical device structure that is being searched for.
 * @return A pointer to the physical_device_node structure containing the matching device,
 *         or NULL if no matching device is found.
 */
static struct physical_device_node *
physical_device_find_device_node(struct physical_device *ph_dev)
{
	struct physical_device_node *device_node;

	OS_SEM_LOCK(&physical_device_list_lock);

	for (device_node = (struct physical_device_node *)physical_device_list.pHead;
		device_node != NULL; device_node = device_node->pNext) {
		struct physical_device *device = &device_node->device_entry;

		if (device == ph_dev) {
			OS_SEM_UNLOCK(&physical_device_list_lock);
			return device_node;
		}
	}

	OS_SEM_UNLOCK(&physical_device_list_lock);
	return NULL;
}

/**
 * @brief Free the MAC address list associated with a physical device.
 *
 * This function frees the memory allocated for the MAC address list of a physical device.
 * It iterates through the list and frees each entry one by one.
 *
 * @param mac_ad_list Pointer to the list header of the MAC address list.
 * @return None.
 */
static void
physical_device_free_mac_ad_list(LIST_HEADER *mac_ad_list)
{
	while (mac_ad_list->pHead != NULL) {
		RT_LIST_ENTRY *entry = mac_ad_list->pHead;

		removeHeadList(mac_ad_list);
		os_free_mem(entry);
	}
}

/**
 * @brief Initialize the chip capabilities for a physical device.
 *
 * This function initializes the chip capabilities for a physical device based on its chip ID.
 * It iterates through the array of chip handles
 * and calls the corresponding chip capability initialization function
 * if a matching chip ID is found.
 *
 * @param ph_dev Pointer to the physical device structure.
 * @return None.
 */
static void
physical_device_chip_init(struct physical_device *ph_dev)
{
	struct chip_handle *handle;

	for (handle = chip_handle_proc; handle->chip_cap_init; handle++) {
		if (ph_dev->chip_id == handle->chip_id) {
			handle->chip_cap_init(ph_dev);
			return;
		}
	}
	MTWF_PRINT("%s(): cannot find chip init handle for this chip(=0x%x).\n",
		__func__, ph_dev->chip_id);
}

/**
 * @brief Load default settings for a physical device.
 *
 * This function initializes the default settings for a physical device.
 * It sets the default values for txpower, E2PAccessMode, band_num, profile_path,
 * pmkid_cache, rro_timeout_profile_set, and other configurations.
 *
 * @param ph_dev Pointer to the physical device structure.
 * @return None.
 */
static void
physical_device_load_default_setting(
	struct physical_device *ph_dev)
{
	u32 i = 0;
	struct txpwr_cfg *txpwr = &ph_dev->txpwr_cfg;

	/* txpower init */
	txpwr->local_max_txpwr_bw20 = DEFAULT_LOCAL_MAX_TXPWR;
	txpwr->local_max_txpwr_bw40 = DEFAULT_LOCAL_MAX_TXPWR;
	txpwr->local_max_txpwr_bw80 = DEFAULT_LOCAL_MAX_TXPWR;
	txpwr->local_max_txpwr_bw160 = DEFAULT_LOCAL_MAX_TXPWR;
	txpwr->local_max_txpwr_bw320 = DEFAULT_LOCAL_MAX_TXPWR;
	for (i = 0; i < MAX_TXPWR_PSD_SIZE; i++)
		txpwr->max_txpwr_psd[i] = DEFAULT_MAX_TXPWR_PSD;
	ph_dev->E2pAccessMode = E2P_FLASH_MODE;
	ph_dev->band_num = ph_dev->chip_cap.hw_band_num;
	for (i = 0; i < CFG_WIFI_RAM_BAND_NUM; i++)
		ph_dev->profile_path[i] = NULL;
#ifdef DOT11_EHT_BE
	eht_global_cfg_init(ph_dev);
#endif /* DOT11_EHT_BE */
#if (defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)) && !defined(RT_CFG80211_SUPPORT)
	sae_cfg_init(ph_dev);
#endif /* DOT11_SAE_SUPPORT */
	/* PMK cache setting*/
	NdisZeroMemory(&ph_dev->pmkid_cache, sizeof(NDIS_AP_802_11_PMKID));
	OS_NdisAllocateSpinLock(&ph_dev->pmkid_cache.pmkid_lock);
	ph_dev->rro_timeout_profile_set = FALSE;
#ifdef PEAK_ENHANCE
	peak_adjust_init_ph_dev(ph_dev);
#endif /* PEAK_ENHANCE */


	/* default : dup beacon by host */
	PD_SET_11V_BCN_DUP(ph_dev, TRUE);
#ifdef CCN67_BS_SUPPORT
	PD_SET_BB_STA_THRESHOLD(ph_dev, 20);
	PD_SET_MESH_ENABLED(ph_dev, 0);
#endif
}

/**
 * @brief Main timer function for a physical device.
 *
 * This function is the main timer function that is called periodically for a physical device.
 * It increments the periodic round counter and calls RTCMDUp to trigger the device task.
 *
 * @param SystemSpecific1 Pointer to system-specific data 1 (unused).
 * @param FunctionContext Pointer to the physical device structure.
 * @param SystemSpecific2 Pointer to system-specific data 2 (unused).
 * @param SystemSpecific3 Pointer to system-specific data 3 (unused).
 * @return None.
 */
static void physical_device_main_timer(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct physical_device *ph_dev = (struct physical_device *)FunctionContext;
	unsigned long timeout = mt_msecs_to_jiffies(500); /* 500ms */

	if (ph_dev) {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_DEBUG,
			"enter\n");
		ph_dev->periodic_round++;
		OS_SEM_LOCK(&ph_dev->ph_dev_wq_lock);
		if (ph_dev->ph_dev_work_list.size)
			mt_queue_delayed_work(ph_dev->ph_dev_wq, &ph_dev->ph_dev_work, timeout);
		OS_SEM_UNLOCK(&ph_dev->ph_dev_wq_lock);
		RTCMDUp(PD_GET_DEVICE_TASK_PTR(ph_dev));

		WaitTxsTOPeriodicHandler(ph_dev);
	}
}

DECLARE_TIMER_FUNCTION(physical_device_main_timer);
BUILD_TIMER_FUNCTION(physical_device_main_timer);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/**
 * @brief Get the MAC adapter associated with a specific band index from a physical device.
 *
 * This function retrieves the MAC adapter that is associated
 * with the given band index from a physical device.
 * It iterates through the MAC address list of the device
 * and compares the band index of each MAC adapter
 * with the given band index. If a match is found, it returns the corresponding MAC adapter.
 *
 * @param physical_dev Pointer to the physical device structure.
 * @param band_idx The band index to search for.
 * @return Pointer to the MAC adapter associated with the specified band index.
 *         Returns NULL if not found.
 */
struct _RTMP_ADAPTER *
physical_device_get_mac_adapter_by_band(VOID *physical_dev, u8 band_idx)
{
	struct physical_device *device_entry = (struct physical_device *)physical_dev;
	LIST_HEADER *mac_ad_list;
	struct mac_ad_node *mac_ad_node;
	struct _RTMP_ADAPTER *pAd = NULL;

	OS_SEM_LOCK(&physical_device_list_lock);
	mac_ad_list = &device_entry->mac_ad_list;
	mac_ad_node = (struct mac_ad_node *)mac_ad_list->pHead;

	while (mac_ad_node) {
		if (hc_get_hw_band_idx(mac_ad_node->ad) == band_idx) {
			pAd = mac_ad_node->ad;
			break;
		}

		mac_ad_node = (struct mac_ad_node *)mac_ad_node->pNext;
	}
	OS_SEM_UNLOCK(&physical_device_list_lock);

	return pAd;
}

/**
 * @brief Get the MAC adapter associated with a specific device index from a physical device.
 *
 * This function retrieves the MAC adapter that is associated with the given device index
 * from a physical device.
 * It iterates through the MAC address list of the device
 * and compares the device index of each MAC adapter
 * with the given device index. If a match is found, it returns the corresponding MAC adapter.
 *
 * @param physical_dev Pointer to the physical device structure.
 * @param dev_idx The device index to search for.
 * @return Pointer to the MAC adapter associated with the specified device index.
 *         Returns NULL if not found.
 */
struct _RTMP_ADAPTER *
physical_device_get_mac_adapter_by_dev_idx(VOID *physical_dev, u8 dev_idx)
{
	struct physical_device *device_entry = (struct physical_device *)physical_dev;
	LIST_HEADER *mac_ad_list;
	struct mac_ad_node *mac_ad_node;
	struct _RTMP_ADAPTER *pAd = NULL;

	OS_SEM_LOCK(&physical_device_list_lock);
	mac_ad_list = &device_entry->mac_ad_list;
	mac_ad_node = (struct mac_ad_node *)mac_ad_list->pHead;

	while (mac_ad_node) {
		pAd = mac_ad_node->ad;
		if (pAd->dev_idx == dev_idx)
			break;

		mac_ad_node = (struct mac_ad_node *)mac_ad_node->pNext;
	}
	OS_SEM_UNLOCK(&physical_device_list_lock);

	return pAd;
}

/**
 * @brief Get the first MAC adapter from a physical device.
 *
 * This function retrieves the first MAC adapter from a physical device.
 * It returns the MAC adapter associated with the head node of the MAC address list of the device.
 *
 * @param physical_dev Pointer to the physical device structure.
 * @return Pointer to the first MAC adapter in the device.
 *         Returns NULL if the MAC address list is empty.
 */
struct _RTMP_ADAPTER *
physical_device_get_first_mac_adapter(VOID *physical_dev)
{
	struct physical_device *device_entry = (struct physical_device *)physical_dev;
	LIST_HEADER *mac_ad_list;
	struct mac_ad_node *mac_ad_node;
	struct _RTMP_ADAPTER *pAd = NULL;

	OS_SEM_LOCK(&physical_device_list_lock);
	mac_ad_list = &device_entry->mac_ad_list;
	mac_ad_node = (struct mac_ad_node *)mac_ad_list->pHead;

	if (mac_ad_node)
		pAd = mac_ad_node->ad;

	OS_SEM_UNLOCK(&physical_device_list_lock);

	return pAd;
}

/**
 * @brief Get the first up MAC adapter from a physical device.
 *
 * This function retrieves the first up MAC adapter from a physical device.
 * It returns the MAC adapter associated with the head node of the MAC address list of the device
 * that has a non-zero virtual interface number.
 *
 * @param physical_dev Pointer to the physical device structure.
 * @return Pointer to the first up MAC adapter in the device.
 *         Returns NULL if no up MAC adapter is found.
 */
struct _RTMP_ADAPTER *
physical_device_get_first_up_mac_adapter(VOID *physical_dev)
{
	struct physical_device *device_entry = (struct physical_device *)physical_dev;
	LIST_HEADER *mac_ad_list;
	struct mac_ad_node *mac_ad_node;
	struct _RTMP_ADAPTER *pAd = NULL;

	OS_SEM_LOCK(&physical_device_list_lock);
	mac_ad_list = &device_entry->mac_ad_list;
	mac_ad_node = (struct mac_ad_node *)mac_ad_list->pHead;

	while (mac_ad_node) {
		pAd = mac_ad_node->ad;
		if (VIRTUAL_IF_NUM(pAd) > 0)
			break;

		mac_ad_node = (struct mac_ad_node *)mac_ad_node->pNext;
	}

	OS_SEM_UNLOCK(&physical_device_list_lock);

	return pAd;
}

/**
 * @brief Get the list of MAC adapters associated with a specific chip ID and hardware ID.
 *
 * This function retrieves the list of MAC adapters
 * that are associated with the given chip ID and hardware ID.
 * It iterates through the physical device list and checks each device's chip ID and hardware ID.
 * If a device matches the specified chip ID and hardware ID,
 * it retrieves the MAC adapter list from the device.
 * The number of MAC adapters found is returned as well.
 *
 * @param chip_id The chip ID to search for.
 * @param hw_id The hardware ID to search for.
 * @param ret_pAd_list Pointer to the array that will contain the MAC adapters (optional).
 * @return The number of MAC adapters associated with the specified chip ID and hardware ID.
 */
u32
physical_device_get_mac_list_by_chip_hw_id(
	u32 chip_id, u32 hw_id, struct _RTMP_ADAPTER **ret_pAd_list)
{
	struct physical_device_node *device_node;
	struct physical_device *device;
	struct mac_ad_node *mac_ad_node;
	u32 mac_num = 0, i;

	OS_SEM_LOCK(&physical_device_list_lock);

	/* Iterate through the physical device list */
	for (device_node = (struct physical_device_node *)physical_device_list.pHead;
		device_node; device_node = (struct physical_device_node *)device_node->pNext) {
		device = &device_node->device_entry;
		if (device->chip_id == chip_id && device->hw_id == hw_id) {
			mac_num = device->mac_ad_list.size;
			if (mac_num > CFG_WIFI_RAM_BAND_NUM) {
				MTWF_PRINT("%s: impossible(mac_num=%d)!!\n", __func__, mac_num);
				break;
			}

			if (ret_pAd_list) {
				mac_ad_node = (struct mac_ad_node *)device->mac_ad_list.pHead;
				for (i = 0; i < mac_num && mac_ad_node;
					i++, mac_ad_node =
						(struct mac_ad_node *)mac_ad_node->pNext) {
					ret_pAd_list[i] = mac_ad_node->ad;
				}
			}

			break;
		}
	}

	OS_SEM_UNLOCK(&physical_device_list_lock);
	return mac_num;
}

/**
 * @brief Display information about the physical devices in the system.
 *
 * This function displays information about the physical devices in the system.
 * It iterates through the physical device list and prints details such as chip ID,
 * hardware device, hardware ID, software band number, device index, MAC table,
 * MAC adapter list, and profile list for each device.
 * Additionally, it prints the value of CFG_WIFI_RAM_BAND_NUM.
 */
void
physical_device_show_information(void)
{
	struct physical_device_node *device_node = NULL;
	struct physical_device *device_entry = NULL;
	struct mac_ad_node *mac_ad_node = NULL;
	int i;

	OS_SEM_LOCK(&physical_device_list_lock);
	MTWF_PRINT("\nPhysical Device List Size:%d\n", physical_device_list.size);
	MTWF_PRINT("=====================================\n");

	device_node = (struct physical_device_node *)physical_device_list.pHead;
	while (device_node) {
		device_entry = &device_node->device_entry;
		MTWF_PRINT("chip id: 0x%x\n", device_entry->chip_id);
		MTWF_PRINT("dev: %p\n", device_entry->hw_dev);
		MTWF_PRINT("dev - hw_id: %d\n", device_entry->hw_id);
		MTWF_PRINT("dev - sw band number: %d\n", device_entry->band_num);
		MTWF_PRINT("dev - device_idx: %d\n", device_entry->device_idx);
		MTWF_PRINT("mac_table: %p\n", &device_entry->mac_table);
		MTWF_PRINT("mac_table_lock: %p\n", &device_entry->mac_table_lock);
		MTWF_PRINT("mac adapter list size:%d\n", device_entry->mac_ad_list.size);
		mac_ad_node = (struct mac_ad_node *)device_entry->mac_ad_list.pHead;
		while (mac_ad_node) {
			MTWF_PRINT("\tmac adapter: %p\n", mac_ad_node->ad);
			MTWF_PRINT("\tmac adapter - dev_idx: %d\n", mac_ad_node->ad->dev_idx);
			MTWF_PRINT("\tmac adapter - MacTab: %p\n", mac_ad_node->ad->MacTab);
			MTWF_PRINT("\tmac adapter - MacTabLock: %p\n", mac_ad_node->ad->MacTabLock);
			mac_ad_node = mac_ad_node->pNext;
		}
		MTWF_PRINT("Profile list:\n");
		for (i = 0; i < CFG_WIFI_RAM_BAND_NUM; i++) {
			if (device_entry->profile_path[i])
				MTWF_PRINT("\tBand[%d] Profile Path: %s\n",
					i, device_entry->profile_path[i]);
		}
		MTWF_PRINT("=====================================\n");
		device_node = device_node->pNext;
	}
	MTWF_PRINT("CFG_WIFI_RAM_BAND_NUM = %d\n", CFG_WIFI_RAM_BAND_NUM);
	MTWF_PRINT("=====================================\n");
	OS_SEM_UNLOCK(&physical_device_list_lock);
}

/**
 * @brief Get the band enable status for a physical device.
 *
 * This function checks the band enable status
 * for a physical device based on the specified band index.
 * It returns TRUE if the specified band is enabled, and FALSE otherwise.
 *
 * @param[in] physical_dev Pointer to the physical device.
 * @param[in] band_idx     Band index to check the enable status (BAND0, BAND1, BAND2).
 *
 * @return TRUE if the specified band is enabled, FALSE otherwise.
 */
u8 physical_device_get_band_en(void *physical_dev, u8 band_idx)
{
	struct physical_device *device_entry = (struct physical_device *)physical_dev;

	switch (band_idx) {
	case BAND0:
		if ((PD_GET_BAND_EN(device_entry) & BAND0_ENABLED) == 0) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"disabled band_idx(=%d)!\n", band_idx);
			return FALSE;
		}
		break;
	case BAND1:
		if ((PD_GET_BAND_EN(device_entry) & BAND1_ENABLED) == 0) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"disabled band_idx(=%d)!\n", band_idx);
			return FALSE;
		}
		break;
	case BAND2:
		if ((PD_GET_BAND_EN(device_entry) & BAND2_ENABLED) == 0) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"disabled band_idx(=%d)!\n", band_idx);
			return FALSE;
		}
		break;
	default:
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"invalid band_idx(%d)!\n", band_idx);
		return FALSE;
	}
	return TRUE;
}

/**
 * @brief Initialize the physical device procedure handler.
 *
 * This function initializes the physical device procedure handler.
 *
 * @param[in] physical_dev Pointer to the physical device.
 */
void physical_device_proc_handle_init(
	void *physical_dev)
{
	struct physical_device *ph_dev = (struct physical_device *)physical_dev;

	/* Init physical device thread */
	physical_device_thread_init(ph_dev);
	/* Init physical device workq */
	physical_device_workq_init(ph_dev);
	/* Allocate Timer Lock */
	NdisAllocateSpinLock(NULL, &ph_dev->pd_main_timer_lock);
	ph_dev->pd_main_timer.timer_lock = &ph_dev->pd_main_timer_lock;
	/* Init physical device main periodic timer*/
	physical_device_init_timer(
		NULL,
		ph_dev,
		&ph_dev->pd_main_timer,
		GET_TIMER_FUNCTION(physical_device_main_timer),
		ph_dev,
		TRUE,
		PD_MAIN_TIMER_NAME);
	/* Set physical device main periodic timer*/
	RTMPSetTimer(&ph_dev->pd_main_timer, TIMER_EXEC_INTV);

	TxsCbListInit(ph_dev);
}

/**
 * @brief Exit and clean up the physical device procedure handler.
 *
 * This function exits and cleans up the physical device procedure handler.
 *
 * @param[in] physical_dev Pointer to the physical device.
 */
void physical_device_proc_handle_exit(
	void *physical_dev)
{
	struct physical_device *ph_dev = (struct physical_device *)physical_dev;
	BOOLEAN Cancelled;

	RTMP_CLEAR_FLAG(ph_dev, PH_DEV_SYSTEM_START_UP);
	RTMPCancelTimer(&ph_dev->pd_main_timer, &Cancelled);
	RTMPReleaseTimer(&ph_dev->pd_main_timer, &Cancelled);
	NdisFreeSpinLock(&ph_dev->pd_main_timer_lock);
	ph_dev->periodic_round = 0;
	TxsCbListDestroy(ph_dev);
	physical_device_thread_exit(ph_dev);
	physical_device_workq_exit(ph_dev);
}

/**
 * @brief Initialize the physical device.
 *
 * This function initializes the physical device.
 *
 */
void
physical_device_init(void)
{
	MTWF_PRINT("%s!\n", __func__);
	initList(&physical_device_list);
	OS_NdisAllocateSpinLock(&physical_device_list_lock);
}

/**
 * @brief Exit and clean up the physical device.
 *
 * This function exits and cleans up the physical device.
 *
 */
void
physical_device_exit(void)
{
	RT_LIST_ENTRY *entry = NULL;
	struct physical_device_node *device_node = NULL;

	OS_SEM_LOCK(&physical_device_list_lock);
	entry = physical_device_list.pHead;

	while (entry) {
		device_node = (struct physical_device_node *)entry;
		physical_device_free_mac_ad_list(&device_node->device_entry.mac_ad_list);
		removeHeadList(&physical_device_list);
		os_free_mem(entry);
		entry = physical_device_list.pHead;
	}
	OS_SEM_UNLOCK(&physical_device_list_lock);
	NdisFreeSpinLock(&physical_device_list_lock);
}

/**
 * @brief Initialize a physical device.
 *
 * This function initializes a physical device.
 *
 * @param[in] device Pointer to the physical device struct.
 * @param[in] hw_dev Pointer to the hardware device.
 * @param[in] chip_id The chip ID of the device.
 * @param[in] hw_flags Hardware flags for the device.
 * @param[in] mac_cap_info Pointer to the MAC capability information.
 */
static void
physical_device_init_device(
	struct physical_device *device, void *hw_dev, u32 chip_id, unsigned long hw_flags, void *mac_cap_info)
{
	/* init physical device */
	device->hw_dev = hw_dev;
	device->chip_id = chip_id;

	wifi_sys_init(&device->wifi_sys_info);
	/* device irq_lock init */
	OS_NdisAllocateSpinLock(&device->irq_lock);

	/* device mlme_lock init */
	OS_NdisAllocateSpinLock(&device->mlme_lock);

	/* device bcn_check_lock init */
	OS_NdisAllocateSpinLock(&device->bcn_check_lock);

	/* mac_table init */
	InitializeQueueHeader(&device->mac_table.McastPsQueue);
	initList(&device->mac_ad_list);
	OS_NdisAllocateSpinLock(&device->mac_table_lock);

	/* timers list lock init */
	OS_NdisAllocateSpinLock(&device->TimerSemLock);
	/* timers list init */
	initList(&device->RscTimerCreateList);

	/* chip cap init */
	physical_device_chip_init(device);
	hwifi_update_hw_cap(device, hw_flags, mac_cap_info);

	/* QM init */
	qm_init(device);

#ifdef MAT_SUPPORT
	MATEngineInit(device);
#endif /* MAT_SUPPORT */

#ifdef CFG_RED_SUPPORT
	red_setting_init(device);
#endif /* CFG_RED_SUPPORT */

	/* Set default setting */
	physical_device_load_default_setting(device);

	/* Profile loading */
	/* load_dev_l1profile should prior to RTMPPreReadProfile */
	if (load_dev_l1profile(device) == NDIS_STATUS_SUCCESS)
		MTWF_PRINT("load l1profile succeed!\n");
	else {
		MTWF_PRINT("load l1profile failed!\n");
		MTWF_PRINT("Please add l1profile.dat to /etc/wireless/.\n");
	}
#ifndef CONFIG_PROFILE_OFF
	load_physical_device_profile(device);
#endif /* !CONFIG_PROFILE_OFF */
	ba_ctl_init(device);
	entry_table_init(device);
	pause_bitmap_init(device);
}

/**
 * @brief Exit and clean up a physical device.
 *
 * This function exits and cleans up the specified physical device.
 *
 * @param[in] device Pointer to the physical device struct.
 */
void
physical_device_exit_device(struct physical_device *device)
{
	int i;

#ifndef CONFIG_PROFILE_OFF
	reset_dev_l1profile(device);
#endif /* !CONFIG_PROFILE_OFF */
#if (defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)) && !defined(RT_CFG80211_SUPPORT)
	sae_cfg_deinit(device);
#endif /* DOT11_SAE_SUPPORT */
	qm_exit(device);
#ifdef MAT_SUPPORT
	MATEngineExit(device);
#endif /* MAT_SUPPORT */
	ba_ctl_exit(device);

	physical_device_free_mac_ad_list(&device->mac_ad_list);

	/* mac_table de-init */
	entry_table_deinit(device);
	pause_bitmap_deinit(device);

	for (i = 0; i < CFG_WIFI_RAM_BAND_NUM; i++) {
		if (device->profile_path[i]) {
			os_free_mem(device->profile_path[i]);
			device->profile_path[i] = NULL;
		}
	}
}

/**
 * @brief Find the physical device associated with the given hardware device.
 *
 * This function finds and returns the physical device associated
 * with the specified hardware device.
 *
 * @param[in] hw_dev Pointer to the hardware device.
 *
 * @return Pointer to the physical device entry if found, otherwise NULL.
 */
void *
physical_device_find_physical_dev(void *hw_dev)
{
	struct physical_device_node *device_node = NULL;
	VOID *physical_dev = NULL;

	OS_SEM_LOCK(&physical_device_list_lock);
	device_node = physical_device_find_master_dev(hw_dev);
	if (device_node)
		physical_dev = (VOID *)&device_node->device_entry;
	OS_SEM_UNLOCK(&physical_device_list_lock);

	return physical_dev;
}
EXPORT_SYMBOL(physical_device_find_physical_dev);

/**
 * @brief Add a main physical device to the device list.
 *
 * This function adds a main physical device to the device list.
 *
 * @param[in] hw_dev Pointer to the hardware device.
 * @param[in] chip_id The chip ID of the device.
 * @param[in] hw_flags Hardware flags for the device.
 * @param[in] mac_cap_info Pointer to the MAC capability information.
 *
 * @return TRUE if the physical device is successfully added, FALSE otherwise.
 */
u8
physical_device_add_main_device(void *hw_dev, u32 chip_id, unsigned long hw_flags, void *mac_cap_info)
{
	LIST_HEADER *device_list = &physical_device_list;
	struct physical_device_node *device_node = NULL;
	struct physical_device *device = NULL;
	int ret;

	OS_SEM_LOCK(&physical_device_list_lock);
	device_node = physical_device_find_master_dev(hw_dev);
	if (device_node) {
		OS_SEM_UNLOCK(&physical_device_list_lock);
		MTWF_PRINT("%s: Master device(=%p) exists!\n", __func__, hw_dev);
		return TRUE;
	}
	OS_SEM_UNLOCK(&physical_device_list_lock);

	/* allocate a physical device entry */
	device_node = (struct physical_device_node *)vmalloc(sizeof(struct physical_device_node));
	if (!device_node) {
		MTWF_PRINT("%s: alloc physical device obj fail!\n", __func__);
		return FALSE;
	}

	device = &device_node->device_entry;
	os_zero_mem(device, sizeof(struct physical_device));

	OS_SEM_LOCK(&physical_device_list_lock);
	device->device_idx = device_list->size;
	os_zero_mem(&device->device_name[0], IFNAMSIZ);
	ret = snprintf(&device->device_name[0], IFNAMSIZ, "dev-%d", device->device_idx);
	if (os_snprintf_error(IFNAMSIZ, ret) != 0)
		MTWF_PRINT("%s: snprintf error\n", __func__);
	insertTailList(device_list, (RT_LIST_ENTRY *)device_node);
	OS_SEM_UNLOCK(&physical_device_list_lock);

	physical_device_init_device(device, hw_dev, chip_id, hw_flags, mac_cap_info);

#ifdef WF_RESET_SUPPORT
	wf_reset_init(device);
#endif /* WF_RESET_SUPPORT */

	MTWF_PRINT("%s: add physical device obj %p, device(=%p), index(=%d), chip=(0x%x)!\n",
		__func__, device_node, device->hw_dev, device->device_idx, device->chip_id);
	return TRUE;
}
EXPORT_SYMBOL(physical_device_add_main_device);

/**
 * @brief Remove a main physical device from the device list.
 *
 * This function removes a main physical device from the device list.
 *
 * @param[in] physical_dev Pointer to the physical device to be removed.
 */
void
physical_device_remove_main_device(void *physical_dev)
{
	LIST_HEADER *device_list = &physical_device_list;
	struct physical_device_node *device_node;
	struct physical_device *device = (struct physical_device *)physical_dev;

	device_node = physical_device_find_device_node(device);
	if (!device_node)
		return;

	OS_SEM_LOCK(&physical_device_list_lock);
	if (device_node->device_entry.mac_ad_list.size) {
		MTWF_PRINT("%s: Still has mac_ad, cannot remove device(=%p) now!\n",
			__func__, device_node);
		OS_SEM_UNLOCK(&physical_device_list_lock);
		return;
	}
	OS_SEM_UNLOCK(&physical_device_list_lock);

#ifdef WF_RESET_SUPPORT
	wf_reset_exit(&device_node->device_entry);
#endif
	physical_device_exit_device(&device_node->device_entry);

	MTWF_PRINT("%s: remove physical device obj %p!\n", __func__, device_node);
	OS_SEM_LOCK(&physical_device_list_lock);
	NdisFreeSpinLock(&device_node->device_entry.mac_table_lock);
	NdisFreeSpinLock(&device_node->device_entry.irq_lock);
	NdisFreeSpinLock(&device_node->device_entry.mlme_lock);
	NdisFreeSpinLock(&device_node->device_entry.bcn_check_lock);
	os_zero_mem(&device_node->device_entry, sizeof(struct physical_device));
	delEntryList(device_list, (RT_LIST_ENTRY *)device_node);
	OS_SEM_UNLOCK(&physical_device_list_lock);
	vfree(device_node);
	return;
}

/**
 * @brief Add a MAC adapter to the device's MAC adapter list.
 *
 * This function adds a MAC adapter to the device's MAC adapter list.
 *
 * @param[in] hw_dev Pointer to the hardware device.
 * @param[in] mac_hw Pointer to the MAC hardware.
 *
 * @return TRUE if the MAC adapter is successfully added, FALSE otherwise.
 */
u8
physical_device_add_mac_adapter(void *hw_dev, void *mac_hw)
{
	struct mtk_mac_hw *hw = (struct mtk_mac_hw *)mac_hw;
	struct mtk_mac_dev *mac_dev = hw_to_mac_dev(hw);
	struct _RTMP_ADAPTER *ad = mac_dev->ad;
	struct physical_device_node *device_node;
	struct physical_device *device;
	struct mac_ad_node *mac_ad_node;
	LIST_HEADER *mac_ad_list;
	u8 found = FALSE;

	OS_SEM_LOCK(&physical_device_list_lock);
	device_node = physical_device_find_master_dev(hw_dev);
	if (!device_node) {
		OS_SEM_UNLOCK(&physical_device_list_lock);
		MTWF_PRINT("%s: cannot find this master device(=%p), check!\n",
			__func__, hw_dev);
		return FALSE;
	}

	device = &device_node->device_entry;
	mac_ad_list = &device_node->device_entry.mac_ad_list;

	mac_ad_node = (struct mac_ad_node *)mac_ad_list->pHead;
	while (mac_ad_node) {
		if (mac_ad_node->ad == ad) {
			found = TRUE;
			break;
		}

		mac_ad_node = (struct mac_ad_node *)mac_ad_node->pNext;
	}

	if (!found) {
		/* allocate a mac adapter entry */
		os_alloc_mem(NULL, (UCHAR **) &(mac_ad_node), sizeof(struct mac_ad_node));

		if (!mac_ad_node) {
			OS_SEM_UNLOCK(&physical_device_list_lock);
			MTWF_PRINT("%s: alloc mac adapter obj fail!\n", __func__);
			return FALSE;
		}
		ad->physical_dev = &device_node->device_entry;
		mac_ad_node->ad = ad;
		ad->MacTab = &device_node->device_entry.mac_table;
		ad->MacTabLock = &device_node->device_entry.mac_table_lock;
		ad->dev_idx = mac_ad_list->size;
		/* first mac adapter, update device hw id to master device */
		if (ad->dev_idx == 0)
			device_node->device_entry.hw_id = hw->hw_id;
		insertTailList(mac_ad_list, (RT_LIST_ENTRY *)mac_ad_node);
		ad->WifiSysInfo = &device->wifi_sys_info;
	}
	MTWF_PRINT("%s(): device(=%p, index=%d), mac_ad index(=%d)!\n",
		__func__, device->hw_dev, device->device_idx, ad->dev_idx);
	OS_SEM_UNLOCK(&physical_device_list_lock);
	return TRUE;
}

/**
 * @brief Remove a MAC adapter from the device's MAC adapter list.
 *
 * This function removes a MAC adapter from the device's MAC adapter list.
 *
 * @param[in] hw_dev Pointer to the hardware device.
 * @param[in] mac_ad Pointer to the MAC adapter.
 *
 * @return TRUE if the MAC adapter is successfully removed, FALSE otherwise.
 */
u8
physical_device_remove_mac_adapter(void *hw_dev, void *mac_ad)
{
	struct physical_device_node *device_node;
	struct physical_device *device;
	struct mac_ad_node *mac_ad_node;
	LIST_HEADER *mac_ad_list;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)mac_ad;

	OS_SEM_LOCK(&physical_device_list_lock);
	device_node = physical_device_find_master_dev(hw_dev);
	if (!device_node) {
		OS_SEM_UNLOCK(&physical_device_list_lock);
		MTWF_PRINT("%s: cannot find this master device(=%p), check!\n",
			__func__, hw_dev);
		return FALSE;
	}

	device = &device_node->device_entry;
	mac_ad_list = &device_node->device_entry.mac_ad_list;

	mac_ad_node = (struct mac_ad_node *)mac_ad_list->pHead;
	while (mac_ad_node) {
		if (mac_ad_node->ad == ad) {
			MTWF_PRINT("%s(): device(=%p, index=%d), mac_ad index(=%d)!\n",
				__func__, device->hw_dev, device->device_idx, ad->dev_idx);
			delEntryList(mac_ad_list, (RT_LIST_ENTRY *)mac_ad_node);
			os_free_mem(mac_ad_node);
			break;
		}

		mac_ad_node = (struct mac_ad_node *)mac_ad_node->pNext;
	}
	OS_SEM_UNLOCK(&physical_device_list_lock);
	return TRUE;
}

/**
 * @brief Add a timer object to the device's timer resource create list.
 *
 * This function adds a timer object to the device's timer resource create list.
 *
 * @param[in] ph_dev_obj Pointer to the physical device object.
 * @param[in] pRsc Pointer to the resource object.
 * @param[in] timer_name Name of the timer.
 */
void physical_device_timer_list_add(
	void *ph_dev_obj, void *pRsc, char *timer_name)
{
	struct physical_device *ph_dev = (struct physical_device *)ph_dev_obj;
	LIST_HEADER *pRscList = &ph_dev->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	/* try to find old entry */
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);

	while (1) {
		if (pObj == NULL)
			break;

		if ((ULONG)(pObj->pRscObj) == (ULONG)pRsc)
			return;

		pObj = pObj->pNext;
	}

	/* allocate a timer record entry */
	os_alloc_mem(NULL, (UCHAR **) &(pObj), sizeof(LIST_RESOURCE_OBJ_ENTRY));

	if (pObj == NULL) {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
			"alloc timer obj fail!\n");
		return;
	}
	pObj->pRscObj = pRsc;
	pObj->timer_name = timer_name;
	insertTailList(pRscList, (RT_LIST_ENTRY *)pObj);
	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG,
		"add timer obj %lx!\n", (ULONG)pRsc);
}

/**
 * @brief Release a timer object from the device's timer resource create list.
 *
 * This function releases a timer object from the device's timer resource create list.
 *
 * @param[in] ph_dev_obj Pointer to the physical device object.
 * @param[in] pRsc Pointer to the resource object.
 */
void physical_device_timer_list_release(
	void *ph_dev_obj, void *pRsc)
{
	struct physical_device *ph_dev = (struct physical_device *)ph_dev_obj;
	LIST_HEADER *pRscList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	RT_LIST_ENTRY *pListEntry;

	if (!ph_dev) {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
			"ph_dev is NULL!\n");
		return;
	}

	pRscList = &ph_dev->RscTimerCreateList;
	if (pRscList == NULL || pRscList->pHead == NULL)
		return;

	pListEntry = pRscList->pHead;
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)pListEntry;

	while (pObj) {
		if ((ULONG)(pObj->pRscObj) == (ULONG)pRsc) {
			pListEntry = (RT_LIST_ENTRY *)pObj;
			break;
		}

		pListEntry = pListEntry->pNext;
		pObj = (LIST_RESOURCE_OBJ_ENTRY *)pListEntry;
	}

	if (pListEntry) {
		delEntryList(pRscList, pListEntry);
		/* free a timer record entry */
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG,
			"release timer obj %lx!\n", (ULONG)pRsc);
		if (pObj == (LIST_RESOURCE_OBJ_ENTRY *)pListEntry)
			os_free_mem(pObj);
		else
			MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
					"Pointers mismatch\n");

	}
}

/**
 * @brief Show the timer list of a physical device.
 *
 * This function shows the timer list of a physical device.
 *
 * @param[in] ph_dev_obj Pointer to the physical device object.
 */
void physical_device_show_timer_list(void *ph_dev_obj)
{
	struct physical_device *ph_dev = (struct physical_device *)ph_dev_obj;
	LIST_HEADER *pRscList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	RALINK_TIMER_STRUCT *pTimer;
	unsigned long flags = 0;

	RTMP_SPIN_LOCK_IRQSAVE(&ph_dev->TimerSemLock, &flags);
	pRscList = &ph_dev->RscTimerCreateList;
	/* try to find old entry */
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);
	MTWF_PRINT("Timer List Size:%d\n", pRscList->size);
	MTWF_PRINT("=====================================\n");

	while (1) {
		if (pObj == NULL)
			break;

		pTimer = (RALINK_TIMER_STRUCT *)pObj->pRscObj;
		pObj = pObj->pNext;
		MTWF_PRINT("Valid:%d\n", pTimer->Valid);
		MTWF_PRINT("pObj:%lx\n", (ULONG)pTimer);
		MTWF_PRINT("PeriodicType:%d\n", pTimer->PeriodicType);
		MTWF_PRINT("Repeat:%d\n", pTimer->Repeat);
		MTWF_PRINT("State:%d\n", pTimer->State);
		MTWF_PRINT("TimerValue:%ld\n", pTimer->TimerValue);
		MTWF_PRINT("timer_lock:%lx\n", (ULONG)pTimer->timer_lock);
		MTWF_PRINT("pCaller:%pS\n", pTimer->pCaller);
		MTWF_PRINT("=====================================\n");
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&ph_dev->TimerSemLock, &flags);
}

/**
 * @brief Release all timer objects in the device's timer resource create list.
 *
 * This function releases all timer objects in the device's timer resource create list.
 *
 * @param[in] ph_dev_obj Pointer to the physical device object.
 */
void physical_device_all_timer_list_release(void *ph_dev_obj)
{
	struct physical_device *ph_dev = (struct physical_device *)ph_dev_obj;
	LIST_HEADER *pRscList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	BOOLEAN Cancel;
	RALINK_TIMER_STRUCT *pTimer;
	unsigned long flags = 0;

	if (!ph_dev) {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
			"ph_dev is NULL!\n");
		return;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&ph_dev->TimerSemLock, &flags);

	pRscList = &ph_dev->RscTimerCreateList;
	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_INFO, "Size=%d\n", pRscList->size);
	while (1) {
		pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);
		if (pObj == NULL)
			break;

		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG,
			"Cancel timer obj %lx, name: %s!\n",
			(ULONG)(pObj->pRscObj), (char *)(pObj->timer_name));

		pTimer = (RALINK_TIMER_STRUCT *)pObj->pRscObj;

		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG,
			"Timer is allocated by %pS,Valid:%d,Lock:%lx,State:%d\n",
			pTimer->pCaller, pTimer->Valid, (ULONG)pTimer->timer_lock, pTimer->State);

		if (pTimer->timer_lock != &ph_dev->TimerSemLock) {
			MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
				"invalid pTimer lock:%p\n", pTimer);
			break;
		}

		if (pTimer->Valid == FALSE) {
			MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
				"invalid pTimer status:%p\n", pTimer);
			break;
		}

		RTMP_OS_Del_Timer(&pTimer->TimerObj, &Cancel);
		if (Cancel == TRUE)
			pTimer->State = TRUE;

		pTimer->Valid = FALSE;
		physical_device_timer_list_release(ph_dev, pTimer);
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG, "%lx\n", (ULONG)pTimer);
	}
	/* reset TimerList */
	initList(pRscList);
	RTMP_SPIN_UNLOCK_IRQRESTORE(&ph_dev->TimerSemLock, &flags);
}

/**
 * @brief Initialize a timer object for the physical device.
 *
 * This function initializes a timer object for the physical device.
 *
 * @param[in] mac_ad Pointer to the RTMP_ADAPTER structure.
 * @param[in] ph_dev_obj Pointer to the physical device object.
 * @param[in] timer_obj Pointer to the timer object.
 * @param[in] pTimerFunc Pointer to the timer function.
 * @param[in] pData Pointer to additional data.
 * @param[in] Repeat Value indicating the repeat type.
 * @param[in] timer_name Pointer to the name of the timer.
 */
void physical_device_init_timer(
	struct _RTMP_ADAPTER *mac_ad,
	void *ph_dev_obj,
	void *timer_obj,
	void *pTimerFunc,
	void *pData,
	u8 Repeat,
	char *timer_name)
{
	unsigned long flags = 0;
	struct _RALINK_TIMER_STRUCT *pTimer = (struct _RALINK_TIMER_STRUCT *)timer_obj;
	struct physical_device *ph_dev = (struct physical_device *)ph_dev_obj;

	if (!ph_dev) {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG, "%lx\n", (ULONG)pTimer);
		return;
	}
	pTimer->timer_lock = &ph_dev->TimerSemLock;
	if (pTimer->timer_lock == NULL)
		return;
	RTMP_SPIN_LOCK_IRQSAVE(pTimer->timer_lock, &flags);
	physical_device_timer_list_add(ph_dev, pTimer, timer_name);
	/* Set Valid to TRUE for later used.*/
	/* It will crash if we cancel a timer or set a timer */
	/* that we haven't initialize before.*/
	pTimer->Valid      = TRUE;
	pTimer->PeriodicType = Repeat;
	pTimer->State      = FALSE;
	pTimer->cookie = (ULONG) pData;
	pTimer->pAd = mac_ad;
	pTimer->ph_dev = ph_dev;
	pTimer->pCaller = (VOID *)OS_TRACE;
	RTMP_OS_Init_Timer(
		ph_dev, &pTimer->TimerObj, pTimerFunc, (PVOID) pTimer);
	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG, "%lx\n", (ULONG)pTimer);
	RTMP_SPIN_UNLOCK_IRQRESTORE(pTimer->timer_lock, &flags);
}

