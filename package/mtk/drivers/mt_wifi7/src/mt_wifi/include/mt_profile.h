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

#ifndef __MT_PROFILE_H__
#define __MT_PROFILE_H__

UCHAR *get_dev_l2profile(RTMP_ADAPTER *pAd);
UCHAR *get_dev_profile_path(void *ph_dev_obj);
INT get_dev_config_idx(RTMP_ADAPTER *pAd);
UCHAR *get_dev_name_prefix(RTMP_ADAPTER *pAd, INT dev_type);
UCHAR *get_single_sku_path(RTMP_ADAPTER *pAd);
UCHAR *get_bf_sku_path(RTMP_ADAPTER *pAd);
UCHAR *get_dev_eeprom_default_bin_path(RTMP_ADAPTER *pvAd);
int ShowL1profile(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT RTMPGetKeyParameter(
	IN RTMP_STRING *key,
	OUT RTMP_STRING *dest,
	IN INT destsize,
	IN RTMP_STRING *buffer,
	IN BOOLEAN bTrimSpace);

#ifdef DBG
NTSTATUS get_dbg_setting_by_profile(RTMP_STRING *dbg_level, RTMP_STRING *dbg_option);
#endif /* DBG */

#ifndef CONFIG_PROFILE_OFF
INT RTMPSetKeyParameter(
	IN RTMP_STRING *key,
	OUT CHAR *value,
	IN INT destsize,
	IN RTMP_STRING *buffer,
	IN BOOLEAN bTrimSpace);

INT RTMPAddKeyParameter(
	IN RTMP_STRING *key,
	IN CHAR *value,
	IN INT destsize,
	IN RTMP_STRING *buffer);

INT RTMPGetKeyParameterWithOffset(
	IN  RTMP_STRING *key,
	OUT RTMP_STRING *dest,
	OUT PUINT end_offset,
	IN  INT     destsize,
	IN  RTMP_STRING *buffer,
	IN	BOOLEAN	bTrimSpace);

#ifdef WSC_INCLUDED
void rtmp_read_wsc_user_parms_from_file(
	IN PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer);
#endif/*WSC_INCLUDED*/

#ifdef FW_LOG_DUMP
void rtmp_read_fw_log_dump_parms_from_file(
	RTMP_ADAPTER *pAd,
	CHAR *tmpbuf,
	CHAR *buffer);
#endif /* FW_LOG_DUMP */

VOID ReadWPAParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer);

VOID ReadSecurityParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer);

NDIS_STATUS	RTMPReadParametersHook(RTMP_ADAPTER *pAd);
NDIS_STATUS	RTMPSetProfileParameters(RTMP_ADAPTER *pAd, RTMP_STRING *pBuffer);
NDIS_STATUS	RTMPPreReadProfile(RTMP_ADAPTER *pAd);
NDIS_STATUS	RTMPSetPreProfileParameters(RTMP_ADAPTER *pAd, RTMP_STRING *pBuffer);

void rtmp_read_cp_parms_from_file(
	IN	PRTMP_ADAPTER pAd,
	char *tmpbuf,
	char *buffer);

void rtmp_read_multi_cli_nums_eap_th_parms_from_file(
	IN	PRTMP_ADAPTER pAd,
	char *tmpbuf,
	char *buffer);

#ifdef CONFIG_AP_SUPPORT
void rtmp_read_acl_parms_from_file(
	RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer);
void rtmp_read_ap_wmm_parms_from_file(
	RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer);
#ifdef VOW_SUPPORT
void rtmp_read_vow_parms_from_file(IN PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer);
#endif /* VOW_SUPPORT */

#ifdef DOT11_EHT_BE
void read_ap_eht_config_from_file(
	IN PRTMP_ADAPTER pAd,
	IN char *tmpbuf,
	IN char *pBuffer);
#endif /* DOT11_EHT_BE */

#ifdef IGMP_SNOOP_SUPPORT
void rtmp_read_igmp_snoop_from_file(
	IN  PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer);
#endif /* IGMP_SNOOP_SUPPORT */

#ifdef CONFIG_MAP_SUPPORT
INT ReadSRMeshUlModeParameterFromFile(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *pBuffer);

INT ReadMapParameterFromFile(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *pBuffer);
#endif /* CONFIG_MAP_SUPPORT */

#ifdef MBO_SUPPORT
MBO_ERR_CODE ReadMboParameterFromFile(
    PRTMP_ADAPTER pAd,
    RTMP_STRING *tmpbuf,
    RTMP_STRING *pBuffer);
#endif /* MBO_SUPPORT */

#ifdef MWDS
VOID rtmp_read_MWDS_from_file(
	IN  PRTMP_ADAPTER pAd,
	PSTRING tmpbuf,
	PSTRING buffer);
#endif /* MWDS */

#ifdef CFG_RED_SUPPORT
void read_red_parms_from_file(
	struct physical_device *ph_dev, char *tmpbuf, char *buffer);
#endif /* CFG_RED_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
void RRM_ReadParametersFromFile(
	IN PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer);
#endif /* DOT11K_RRM_SUPPORT */
#ifdef CONFIG_DOT11V_WNM
void WNM_ReadParametersFromFile(
        IN PRTMP_ADAPTER pAd,
        RTMP_STRING *tmpbuf,
        RTMP_STRING *buffer);
#endif /* CONFIG_DOT11V_WNM */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11_EHT_BE
void read_sta_eht_config_from_file(
	IN PRTMP_ADAPTER pAd,
	IN char *tmpbuf,
	IN char *pBuffer);
#endif /* DOT11_EHT_BE */
#endif /* CONFIG_STA_SUPPORT */

NDIS_STATUS load_physical_device_profile(void *ph_dev_obj);
NDIS_STATUS rtmp_read_physical_device_parms(
	void *ph_dev_obj,
	RTMP_STRING *pBuffer);

#endif /* !CONFIG_PROFILE_OFF */
#endif /* __MT_PROFILE_H__ */
