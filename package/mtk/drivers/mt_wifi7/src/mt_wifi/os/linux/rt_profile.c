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
 ****************************************************************************

    Module Name:
	rt_profile.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#include "rt_config.h"
#include "l1profile.h"

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
#include "phy/rlm_cal_cache.h"
#endif /* RLM_CAL_CACHE_SUPPORT */


#ifdef CONFIG_FAST_NAT_SUPPORT
#include <net/ra_nat.h>
#endif /*CONFIG_FAST_NAT_SUPPORT*/

#ifdef MT_FMAC
#include "mac_mt/fmac/mt_fmac.h"
#endif /* MT_FMAC */

#define BSSID_WCID_TO_REMOVE 1

struct l1profile_info_t {
	BOOL valid;
	RTMP_STRING profile_index[L1PROFILE_INDEX_LEN];
	RTMP_STRING profile_path[L2PROFILE_PATH_LEN];
	eeprom_flash_info ee_info;
	struct dev_type_name_map_t dev_name_map[MAX_INT_TYPES + 1];
	RTMP_STRING single_sku_path[L2PROFILE_PATH_LEN];
	RTMP_STRING bf_sku_path[L2PROFILE_PATH_LEN];
};

struct l1profile_attribute_t {
	RTMP_STRING name[L1PROFILE_ATTRNAME_LEN];
	UINT_32	extra;
	INT (*handler)(struct physical_device *ph_dev,
		INT dev_idx, UINT_32 extra, RTMP_STRING *value);
};

#if defined(RT_CFG80211_SUPPORT)
#ifdef CONFIG_SUPPORT_OPENWRT
#define SECOND_INF_MAIN_DEV_NAME		"rai0"
#define SECOND_INF_MBSSID_DEV_NAME	"rai"
#else
#define SECOND_INF_MAIN_DEV_NAME		"wlani"
#define SECOND_INF_MBSSID_DEV_NAME	"wlani"
#endif
#else
#define SECOND_INF_MAIN_DEV_NAME		"rai0"
#define SECOND_INF_MBSSID_DEV_NAME	"rai"
#endif
#define SECOND_INF_WDS_DEV_NAME		"wdsi"
#define SECOND_INF_APCLI_DEV_NAME	"apclii"
#define SECOND_INF_MESH_DEV_NAME		"meshi"
#define SECOND_INF_P2P_DEV_NAME		"p2pi"
#define SECOND_INF_MONITOR_DEV_NAME		"moni"
#define SECOND_INF_MSTA_DEV_NAME    "rai"
#ifdef ACTION_MONITOR_SUPPORT
#define SECOND_INF_ACT_MONITOR_DEV_NAME		"moni"
#endif

#if defined(RT_CFG80211_SUPPORT)
#define THIRD_INF_MAIN_DEV_NAME		"wlane"
#define THIRD_INF_MBSSID_DEV_NAME	"wlane"
#else
#define THIRD_INF_MAIN_DEV_NAME		"rae0"
#define THIRD_INF_MBSSID_DEV_NAME	"rae"
#endif
#define THIRD_INF_WDS_DEV_NAME		"wdse"
#define THIRD_INF_APCLI_DEV_NAME	"apclie"
#define THIRD_INF_MESH_DEV_NAME		"meshe"
#define THIRD_INF_P2P_DEV_NAME		"p2pe"
#define THIRD_INF_MONITOR_DEV_NAME		"mone"
#define THIRD_INF_MSTA_DEV_NAME    "rae"

#ifdef ACTION_MONITOR_SUPPORT
#define THIRD_INF_ACT_MONITOR_DEV_NAME		"monx"
#endif

#define xdef_to_str(s)   def_to_str(s)
#define def_to_str(s)    #s

#define FIRST_EEPROM_FILE_PATH	"/etc_ro/Wireless/RT2860/"
#define FIRST_AP_PROFILE_PATH		"/etc/Wireless/RT2860AP/RT2860AP.dat"
#define FIRST_STA_PROFILE_PATH      "/etc/Wireless/RT2860/RT2860.dat"
#define FIRST_CHIP_ID	xdef_to_str(CONFIG_RT_FIRST_CARD)

#define SECOND_EEPROM_FILE_PATH	"/etc_ro/Wireless/iNIC/"
#define SECOND_AP_PROFILE_PATH	"/etc/Wireless/iNIC/iNIC_ap.dat"
#define SECOND_STA_PROFILE_PATH "/etc/Wireless/iNIC/iNIC_sta.dat"

#define SECOND_CHIP_ID	xdef_to_str(CONFIG_RT_SECOND_CARD)

#define THIRD_EEPROM_FILE_PATH	"/etc_ro/Wireless/WIFI3/"
#define THIRD_AP_PROFILE_PATH	"/etc/Wireless/WIFI3/RT2870AP.dat"
#define THIRD_STA_PROFILE_PATH "/etc/Wireless/WIFI3/RT2870AP.dat"

#define THIRD_CHIP_ID	xdef_to_str(CONFIG_RT_THIRD_CARD)

#ifndef CONFIG_RT_FIRST_IF_RF_OFFSET
#define CONFIG_RT_FIRST_IF_RF_OFFSET DEFAULT_RF_OFFSET
#endif

#ifndef CONFIG_RT_SECOND_IF_RF_OFFSET
#define CONFIG_RT_SECOND_IF_RF_OFFSET DEFAULT_RF_OFFSET
#endif

#ifndef CONFIG_RT_THIRD_IF_RF_OFFSET
#define CONFIG_RT_THIRD_IF_RF_OFFSET DEFAULT_RF_OFFSET
#endif

static struct l1profile_info_t l1profile[MAX_MULTI_CARD_NUM];
static struct l1profile_info_t l1profile_template[MAX_MULTI_CARD_NUM] = {
	{
		FALSE,
		{0},
		FIRST_AP_PROFILE_PATH,
		{CONFIG_RT_FIRST_IF_RF_OFFSET, EEPROM_SIZE},
		{	{INT_MAIN, {INF_MAIN_DEV_NAME, SECOND_INF_MAIN_DEV_NAME, ""}},
			{INT_MBSSID, {INF_MBSSID_DEV_NAME, SECOND_INF_MBSSID_DEV_NAME, ""}},
			{INT_WDS, {INF_WDS_DEV_NAME, SECOND_INF_WDS_DEV_NAME, ""}},
			{INT_APCLI, {INF_APCLI_DEV_NAME, SECOND_INF_APCLI_DEV_NAME, ""}},
			{INT_P2P, {INF_P2P_DEV_NAME, SECOND_INF_P2P_DEV_NAME, ""}},
			{INT_MONITOR, {INF_MONITOR_DEV_NAME, SECOND_INF_MONITOR_DEV_NAME, ""}},
			{INT_MSTA, {INF_MSTA_DEV_NAME, SECOND_INF_MSTA_DEV_NAME, ""}},
#ifdef ACTION_MONITOR_SUPPORT
			{INT_ACT_MONITOR, {INF_ACT_MONITOR_DEV_NAME, SECOND_INF_ACT_MONITOR_DEV_NAME, THIRD_INF_ACT_MONITOR_DEV_NAME}},
#endif
			{INT_DISABLE, {INF_DISABLE_DEV_NAME, "", ""}},
			{0}
		},
		{SINGLE_SKU_TABLE_FILE_NAME},
		{BF_SKU_TABLE_FILE_NAME}
	},
	{
		FALSE,
		{0},
		SECOND_AP_PROFILE_PATH,
		{CONFIG_RT_SECOND_IF_RF_OFFSET, EEPROM_SIZE},
		{	{INT_MAIN, {SECOND_INF_MAIN_DEV_NAME, "", ""}},
			{INT_MBSSID, {SECOND_INF_MBSSID_DEV_NAME, "", ""}},
			{INT_WDS, {SECOND_INF_WDS_DEV_NAME, "", ""}},
			{INT_APCLI, {SECOND_INF_APCLI_DEV_NAME, "", ""}},
			{INT_P2P, {SECOND_INF_P2P_DEV_NAME, "", ""}},
			{INT_MONITOR, {SECOND_INF_MONITOR_DEV_NAME, "", ""}},
			{INT_MSTA, {SECOND_INF_MSTA_DEV_NAME, "", ""}},
#ifdef ACTION_MONITOR_SUPPORT
			{INT_ACT_MONITOR, {INF_ACT_MONITOR_DEV_NAME, SECOND_INF_ACT_MONITOR_DEV_NAME, THIRD_INF_ACT_MONITOR_DEV_NAME}},
#endif
			{INT_DISABLE, {INF_DISABLE_DEV_NAME, "", ""}},
			{0}
		},
		{SINGLE_SKU_TABLE_FILE_NAME},
		{BF_SKU_TABLE_FILE_NAME}
	},
	{
		FALSE,
		{0},
		THIRD_AP_PROFILE_PATH,
		{CONFIG_RT_THIRD_IF_RF_OFFSET, EEPROM_SIZE},
		{	{INT_MAIN, {THIRD_INF_MAIN_DEV_NAME, "", ""}},
			{INT_MBSSID, {THIRD_INF_MBSSID_DEV_NAME, "", ""}},
			{INT_WDS, {THIRD_INF_WDS_DEV_NAME, "", ""}},
			{INT_APCLI, {THIRD_INF_APCLI_DEV_NAME, "", ""}},
			{INT_P2P, {THIRD_INF_P2P_DEV_NAME, "", ""}},
			{INT_MONITOR, {THIRD_INF_MONITOR_DEV_NAME, "", ""}},
			{INT_MSTA, {THIRD_INF_MSTA_DEV_NAME, "", ""}},
#ifdef ACTION_MONITOR_SUPPORT
			{INT_ACT_MONITOR, {INF_ACT_MONITOR_DEV_NAME, SECOND_INF_ACT_MONITOR_DEV_NAME, THIRD_INF_ACT_MONITOR_DEV_NAME}},
#endif
			{INT_DISABLE, {INF_DISABLE_DEV_NAME, "", ""}},
			{0}
		},
		{SINGLE_SKU_TABLE_FILE_NAME},
		{BF_SKU_TABLE_FILE_NAME}
	},
};


static NDIS_STATUS l1set_profile_path(
	struct physical_device *ph_dev, INT dev_idx, UINT_32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	INT ret;
	RTMP_STRING *target = NULL;
	UINT8 str_len;

	if (dev_idx < 0 || dev_idx >= MAX_MULTI_CARD_NUM) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"%s: invalid dev_idx:%d\n", __func__, dev_idx);
		return NDIS_STATUS_INVALID_DATA;
	}

	target = l1profile[dev_idx].profile_path;
	str_len = strlen(value);
	if (strcmp(target, value) && (str_len < L2PROFILE_PATH_LEN)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"profile update from %s to %s\n", target, value);

		ret = snprintf(target, L2PROFILE_PATH_LEN, "%s", value);
		if (os_snprintf_error(L2PROFILE_PATH_LEN, ret) != 0)
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
				"snprintf error\n");
	} else
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"profile remain %s\n", target);

	return retVal;
}


static NDIS_STATUS l1set_eeprom_bin(
	struct physical_device *ph_dev, INT dev_idx, UINT_32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	INT ret;
	RTMP_STRING *target = NULL;
	UINT8 str_len;

	if (dev_idx < 0 || dev_idx >= MAX_MULTI_CARD_NUM) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"%s: invalid dev_idx:%d\n", __func__, dev_idx);
		return NDIS_STATUS_INVALID_DATA;
	}

	target = l1profile[dev_idx].ee_info.bin_name;
	str_len = strlen(value);
	if (strcmp(target, value) && (str_len < L1PROFILE_ATTRNAME_LEN)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"eeprom binary update from %s to %s\n", target, value);

		ret = snprintf(target, L1PROFILE_ATTRNAME_LEN, "%s", value);
		if (os_snprintf_error(L1PROFILE_ATTRNAME_LEN, ret) != 0)
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
				"snprintf error\n");
		*(target+str_len) = '\0';
	} else
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"eeprom binary remain %s\n", target);

	return retVal;
}

static NDIS_STATUS l1set_eeprom_offset(
	struct physical_device *ph_dev, INT dev_idx, UINT_32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	eeprom_flash_info *target = NULL;
	UINT	int_value = 0;

	if (dev_idx < 0 || dev_idx >= MAX_MULTI_CARD_NUM) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"%s: invalid dev_idx:%d\n", __func__, dev_idx);
		return NDIS_STATUS_INVALID_DATA;
	}

	target = (eeprom_flash_info *)(&(l1profile[dev_idx].ee_info));
	int_value = os_str_tol(value, 0, 0);

	if (target->offset != int_value) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"eeprom offset update from 0x%x to 0x%x\n", target->offset, int_value);
		target->offset = int_value;
	} else
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"eeprom offset remain 0x%x\n", target->offset);

	return retVal;
}

static NDIS_STATUS l1set_eeprom_size(
	struct physical_device *ph_dev, INT dev_idx, UINT_32 extra, RTMP_STRING *value)
{
	UINT int_value;
	INT retVal = NDIS_STATUS_SUCCESS;
	eeprom_flash_info *target = NULL;

	if (dev_idx < 0 || dev_idx >= MAX_MULTI_CARD_NUM) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"%s: invalid dev_idx:%d\n", __func__, dev_idx);
		return NDIS_STATUS_INVALID_DATA;
	}

	target = (eeprom_flash_info *)(&(l1profile[dev_idx].ee_info));
	int_value = os_str_tol(value, 0, 0);

	if (target->size != int_value) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"eeprom size update from 0x%x to 0x%x\n", target->size, int_value);
		target->size = int_value;
	} else
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"eeprom size remain 0x%x\n", target->size);
	return retVal;
}

static NDIS_STATUS l1set_eeprom_default_bin_path(
	struct physical_device *ph_dev, INT dev_idx, UINT_32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	INT ret;
	RTMP_STRING *target = NULL;
	UINT8 str_len;

	if (dev_idx < 0 || dev_idx >= MAX_MULTI_CARD_NUM) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"%s: invalid dev_idx:%d\n", __func__, dev_idx);
		return NDIS_STATUS_INVALID_DATA;
	}

	target = l1profile[dev_idx].ee_info.eeprom_default_bin_path;
	str_len = strlen(value);
	if (strcmp(target, value) && (str_len < L2PROFILE_PATH_LEN)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"eeprom binary update from %s to %s, len:%d\n", target, value, str_len);

		ret = snprintf(target, L2PROFILE_PATH_LEN, "%s", value);
		if (os_snprintf_error(L2PROFILE_PATH_LEN, ret) != 0)
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"%s: snprintf error\n", __func__);
		*(target+str_len) = '\0';
	} else
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			"eeprom default bin path remain %s\n", target);

	return retVal;
}

static NDIS_STATUS l1set_ifname(
	struct physical_device *ph_dev, INT dev_idx, UINT_32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	RTMP_STRING *target = NULL;
	struct dev_type_name_map_t *map;
	INT type_idx = 0;
	INT ret, i;
	char *macptr;

	if (dev_idx < 0 || dev_idx >= MAX_MULTI_CARD_NUM) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"%s: invalid dev_idx:%d\n", __func__, dev_idx);
		return NDIS_STATUS_INVALID_DATA;
	}

	do {
		map = &(l1profile[dev_idx].dev_name_map[type_idx]);
		if (map->type == extra) {
			for (i = 0, macptr = rstrtok(value, ";");
					macptr; macptr = rstrtok(NULL, ";"), i++) {
				if (i >= CFG_WIFI_RAM_BAND_NUM)
					break;
				target = map->prefix[i];
				os_zero_mem(target, IFNAMSIZ);
				ret = snprintf(target, IFNAMSIZ, "%s", macptr);
				if (os_snprintf_error(IFNAMSIZ, ret) != 0)
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"snprintf error(=%d)\n", ret);
				else
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_DEBUG,
						"ifname(=%s)\n", target);
			}
			break;
		}

		type_idx++;
	} while (l1profile[dev_idx].dev_name_map[type_idx].type != 0);

	return retVal;
}

static NDIS_STATUS l1set_single_sku_path(
	struct physical_device *ph_dev, INT dev_idx, UINT_32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	RTMP_STRING *target = NULL;

	if (dev_idx < 0 || dev_idx >= MAX_MULTI_CARD_NUM) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"%s: invalid dev_idx:%d\n", __func__, dev_idx);
		return NDIS_STATUS_INVALID_DATA;
	}

	target = l1profile[dev_idx].single_sku_path;
	if (strcmp(target, value)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"sku path update from %s to %s\n", target, value);
		strncpy(target, value, L2PROFILE_PATH_LEN - 1);
	} else
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"profile remain %s\n", target);
	return retVal;
}

static NDIS_STATUS l1set_bf_sku_path(
	struct physical_device *ph_dev, INT dev_idx, UINT_32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	RTMP_STRING *target = NULL;

	if (dev_idx < 0 || dev_idx >= MAX_MULTI_CARD_NUM) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"%s: invalid dev_idx:%d\n", __func__, dev_idx);
		return NDIS_STATUS_INVALID_DATA;
	}

	target = l1profile[dev_idx].bf_sku_path;
	if (strcmp(target, value)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"BF sku path update from %s to %s\n", target, value);
		strncpy(target, value, L2PROFILE_PATH_LEN - 1);
	} else
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"profile remain %s\n", target);
	return retVal;
}

static struct l1profile_attribute_t l1profile_attributes[] = {
	{ {"init_path"},    0,           l1set_profile_path},
	{ {"EEPROM_name"},     0,           l1set_eeprom_bin},
	{ {"EEPROM_offset"},   0,           l1set_eeprom_offset},
	{ {"EEPROM_size"},     0,           l1set_eeprom_size},
	{ {"EEPROM_def_bin_path"},   0,     l1set_eeprom_default_bin_path},
	{ {"main_ifname"},     INT_MAIN,    l1set_ifname},
	{ {"ext_ifname"},      INT_MBSSID,  l1set_ifname},
#ifdef WDS_SUPPORT
	{ {"wds_ifname"},      INT_WDS,     l1set_ifname},
#endif	/* WDS_SUPPORT */
	{ {"apcli_ifname"},    INT_APCLI,   l1set_ifname},
#ifdef SNIFFER_SUPPORT
	{ {"monitor_ifname"},  INT_MONITOR, l1set_ifname},
#endif	/* monitor_ifname */
	{ {"single_sku_path"}, 0,           l1set_single_sku_path},
	{ {"bf_sku_path"},     0,           l1set_bf_sku_path},
};

struct dev_id_name_map {
	INT chip_id;
	RTMP_STRING *chip_name;
};

static const struct dev_id_name_map id_name_list[] = {
	{7610, "7610, 7610e 7610u"},

};

UCHAR *get_single_sku_path(RTMP_ADAPTER *pAd)
{
	UCHAR *src = NULL;

	if (!pAd->physical_dev) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"physical_dev is null!\n");
		return NULL;
	}

	src = l1profile[PD_GET_DEVICE_IDX(pAd->physical_dev)].single_sku_path;
	return src;
}

UCHAR *get_bf_sku_path(RTMP_ADAPTER *pAd)
{
	UCHAR *src = NULL;

	if (!pAd->physical_dev) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"physical_dev is null!\n");
		return NULL;
	}

	src = l1profile[PD_GET_DEVICE_IDX(pAd->physical_dev)].bf_sku_path;
	return src;
}

INT get_dev_config_idx(RTMP_ADAPTER *pAd)
{
	return pAd->dev_idx;
}

UCHAR *get_dev_eeprom_default_bin_path(RTMP_ADAPTER *pAd)
{
	if (!pAd->physical_dev) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"physical_dev is null!\n");
		return NULL;
	}

	return l1profile[PD_GET_DEVICE_IDX(pAd->physical_dev)].ee_info.eeprom_default_bin_path;
}

RTMP_STRING *get_dev_eeprom_binary(VOID *pvAd)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pvAd;

	if (!pAd->physical_dev) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"physical_dev is null!\n");
		return NULL;
	}

	return l1profile[PD_GET_DEVICE_IDX(pAd->physical_dev)].ee_info.bin_name;
}


UINT32 get_dev_eeprom_offset(VOID *pvAd)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pvAd;

	if (!pAd->physical_dev) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"physical_dev is null!\n");
		return 0;
	}

	return l1profile[PD_GET_DEVICE_IDX(pAd->physical_dev)].ee_info.offset;
}


UINT32 get_dev_eeprom_size(VOID *pvAd)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pvAd;

	if (!pAd->physical_dev) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"physical_dev is null!\n");
		return 0;
	}

	return l1profile[PD_GET_DEVICE_IDX(pAd->physical_dev)].ee_info.size;
}


UCHAR *get_dev_name_prefix(RTMP_ADAPTER *pAd, INT dev_type)
{
	struct dev_type_name_map_t *map;
	INT type_idx = 0, dev_idx;
	u8 band_idx = hc_get_hw_band_idx(pAd);
	u8 mac_ad_idx = get_dev_config_idx(pAd);

	if (!pAd->physical_dev) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"%s(): physical_dev is null!\n",
			__func__);
		return NULL;
	}
	dev_idx = PD_GET_DEVICE_IDX(pAd->physical_dev);
	if (dev_idx < 0 || dev_idx >= MAX_NUM_OF_INF) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"%s(): invalid dev_idx(%d)!\n",
			__func__, dev_idx);
		return NULL;
	}

	/*
	 * Set dev_type to be INT_DISABLE when
	 * 1. Band is disabled.
	 * 2. Invalid band index.
	 */
	if (physical_device_get_band_en(pAd->physical_dev, band_idx) == FALSE)
		dev_type = INT_DISABLE;

	do {
		map = &(l1profile[dev_idx].dev_name_map[type_idx]);

		if (map->type == dev_type) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_DEBUG,
				"%s(): dev_idx = %d, mac_ad_idx = %d, dev_name_prefix=%s\n",
				__func__, dev_idx, mac_ad_idx, map->prefix[mac_ad_idx]);
			return map->prefix[mac_ad_idx];
		}

		type_idx++;
	} while (l1profile[dev_idx].dev_name_map[type_idx].type != 0);

	return NULL;
}

UCHAR *get_dev_l2profile(RTMP_ADAPTER *pAd)
{
	UCHAR *src = NULL;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);
	struct _RTMP_CHIP_CAP *chip_cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);

	if (band_idx >= chip_cap->hw_band_num)
		MTWF_PRINT("%s: invalid band index(=%d), check.\n",
			__func__, band_idx);
	else
		src = PD_GET_BAND_PROFILE_PATH(pAd->physical_dev, band_idx);

	/* use per band l1profile */
	if (!src) {
		INT dev_idx = get_dev_config_idx(pAd);

		if (dev_idx >= 0 && dev_idx < MAX_MULTI_CARD_NUM)
			src = l1profile[dev_idx].profile_path;
	}

	if (src == NULL) {
		MTWF_PRINT("%s: suggest to use l1profile. use default profile path.\n",
			__func__);
		switch (band_idx) {
		case BAND1:
		case BAND2:
			src = DEFAULT_PROFILE_PATH1;
			break;
		case BAND0:
		default:
			src = DEFAULT_PROFILE_PATH0;
			break;
		}
	}

	MTWF_PRINT("%s: Band%d: profile path=%s\n",
			__func__, band_idx, src);
	return src;
}

UCHAR *get_dev_profile_path(void *ph_dev_obj)
{
	u8 dev_idx;

	dev_idx = PD_GET_DEVICE_IDX(ph_dev_obj);
	if (dev_idx >= MAX_MULTI_CARD_NUM)
		return NULL;

	if (l1profile[dev_idx].valid == TRUE)
		return l1profile[dev_idx].profile_path;
	else
		return NULL;
}

INT ResetL1profile(INT idx)
{
	if ((idx < 0) || (idx >= MAX_MULTI_CARD_NUM))
		return -1;

	memcpy(&l1profile[idx], &l1profile_template[idx],
		sizeof(struct l1profile_info_t));
	return 0;
}

int ShowL1profile(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;

	MTWF_PRINT("===== L1 profile settings =====\n");
	for (i = 0; i < MAX_MULTI_CARD_NUM; i++) {
		if (l1profile[i].valid == FALSE)
			continue;
		MTWF_PRINT("profile %d:\n", i);
		MTWF_PRINT("%s = %s\n",
			l1profile_attributes[0].name, l1profile[i].profile_path);
		MTWF_PRINT("%s = 0x%x\n",
			l1profile_attributes[2].name, l1profile[i].ee_info.offset);
		MTWF_PRINT("%s = 0x%x\n",
			l1profile_attributes[3].name, l1profile[i].ee_info.size);
		MTWF_PRINT("%s = %s\n",
			l1profile_attributes[4].name, l1profile[i].ee_info.eeprom_default_bin_path);
		MTWF_PRINT("%s = %s\n",
			l1profile_attributes[5].name, l1profile[i].dev_name_map[0].prefix[0]);
	}

	return TRUE;
}

static struct {
	UINT32 id;
	RTMP_STRING *name;
} chipname_table[] = {
	{ 0x7902, "MT7902"},
	{ 0x7990, "MT7990"},
	{ 0x7992, "MT7992"},
	{ 0x7993, "MT7993"}, };

static RTMP_STRING *get_chipname(UINT32 chip_id)
{
	INT i;

	for (i = 0; i < ARRAY_SIZE(chipname_table); i++) {
		if (chipname_table[i].id == chip_id)
			return chipname_table[i].name;
	}
	return NULL;
}

static NDIS_STATUS llget_profile_setting(
	struct physical_device *ph_dev,
	RTMP_STRING *buffer)
{
	INT dev_idx = 0;
	INT ret;
	UINT32 attr_index = 0;
	struct l1profile_attribute_t *l1attr = NULL;
	RTMP_STRING key[30] = {'\0'};
	RTMP_STRING *tmpbuf = NULL;
	RTMP_STRING *chip_name;

	if (ph_dev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG,
			CATCFG_PROFILE, DBG_LVL_ERROR,
			"invalid phsical device\n");
		return NDIS_STATUS_FAILURE;
	}

	dev_idx = PD_GET_DEVICE_IDX(ph_dev);
	if ((dev_idx < 0) || (dev_idx >= MAX_MULTI_CARD_NUM)) {
		MTWF_DBG(NULL, DBG_CAT_CFG,
			CATCFG_PROFILE, DBG_LVL_ERROR,
			"invalid device idx:%d\n", dev_idx);
		return NDIS_STATUS_FAILURE;
	}

	chip_name = get_chipname(ph_dev->chip_id);
	if (chip_name == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG,
			CATCFG_PROFILE, DBG_LVL_ERROR,
			"not support chip id:0x%x\n", ph_dev->chip_id);
		return NDIS_STATUS_FAILURE;
	}

	MTWF_PRINT("%s: dev_idx = %d!\n", __func__, dev_idx);

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (tmpbuf == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			"memory not available\n");
		return NDIS_STATUS_FAILURE;
	}

	ret = snprintf(key, sizeof(key), "INDEX%d", dev_idx);
	if (os_snprintf_error(sizeof(key), ret) != 0) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			"%s: snprintf error\n", __func__);
		goto err1;
	}

	if (RTMPGetKeyParameter(
		key, tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) == FALSE) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			"value of %s not found\n", key);
		goto err1;
	}

	if ((strncmp(tmpbuf, chip_name, strlen(chip_name)) != 0) ||
	    (strlen(chip_name) != strlen(tmpbuf))) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			"chip %s not found\n", chip_name);
		goto err1;
	}

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_DEBUG,
		"%s found as %s\n", chip_name, key);

	for (attr_index = 0;
		attr_index < ARRAY_SIZE(l1profile_attributes);
		attr_index++) {
		l1attr = &l1profile_attributes[attr_index];
		ret = snprintf(key, sizeof(key), "INDEX%d_%s",
			dev_idx, l1attr->name);
		if (os_snprintf_error(sizeof(key), ret) != 0) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
				"snprintf error!\n");
			goto err2;
		}

		if (RTMPGetKeyParameter(
			key, tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) == FALSE) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
				"value of %s not found\n", key);
			continue;
		}

		MTWF_DBG(NULL, DBG_CAT_CFG,
			CATCFG_PROFILE, DBG_LVL_INFO,
			"%s=%s\n", l1attr->name, tmpbuf);

		if (!l1attr->handler) {
			MTWF_DBG(NULL, DBG_CAT_CFG,
				CATCFG_PROFILE, DBG_LVL_ERROR,
				"unknown handler for %s, ignored!\n",
				l1attr->name);
			goto err2;
		}

		if (l1attr->handler(ph_dev, dev_idx,
			l1attr->extra, tmpbuf) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_CFG,
				CATCFG_PROFILE, DBG_LVL_ERROR,
				"handler for %s failed\n",
				l1attr->name);
			goto err2;
		}
	}
	l1profile[dev_idx].valid = TRUE;
	os_free_mem(tmpbuf);

	return NDIS_STATUS_SUCCESS;
err2:
	ResetL1profile(dev_idx);
err1:
	if (tmpbuf)
		os_free_mem(tmpbuf);

	return NDIS_STATUS_FAILURE;
}

void reset_dev_l1profile(void *ph_dev_obj)
{
	struct physical_device *ph_dev = (struct physical_device *)ph_dev_obj;

	if (!ph_dev)
		return;

	ResetL1profile(PD_GET_DEVICE_IDX(ph_dev));
}

NDIS_STATUS load_dev_l1profile(void *ph_dev_obj)
{
	RTMP_STRING *buffer = NULL;
	INT retval = NDIS_STATUS_SUCCESS;
	UINT32 buf_size = 0;
	struct physical_device *ph_dev = (struct physical_device *)ph_dev_obj;

	os_load_code_from_bin(NULL, (unsigned char **)&buffer, L1_PROFILE_PATH, &buf_size);
	if (buffer == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_WARN,
			"Open file \"%s\" failed, try embedded default!\n", L1_PROFILE_PATH);
		buffer = l1profile_default;
	}

	retval = llget_profile_setting(ph_dev, buffer);

	if (buffer && (buffer != ((RTMP_STRING *)l1profile_default)))
		os_free_mem(buffer);

	return retval;
}

INT rt_profile_init(void)
{
	memcpy(l1profile, l1profile_template, sizeof(l1profile));

	return 0;
}

void rt_profile_deinit(void)
{
}

